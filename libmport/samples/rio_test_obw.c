/*
 * Copyright 2014, 2015 Integrated Device Technology, Inc.
 *
 * This software is available to you under a choice of one of two licenses.
 * You may choose to be licensed under the terms of the GNU General Public
 * License(GPL) Version 2, or the BSD-3 Clause license below:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#include <rio_mport_lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialization patterns. All bytes in the source buffer has bit 7
 * set, all bytes in the destination buffer has bit 7 cleared.
 *
 * Bit 6 is set for all bytes which are to be copied by the DMA
 * engine. Bit 5 is set for all bytes which are to be overwritten by
 * the DMA engine.
 *
 * The remaining bits are the inverse of a counter which increments by
 * one for each byte address.
 */
#define PATTERN_SRC		0x80
#define PATTERN_DST		0x00
#define PATTERN_COPY		0x40
#define PATTERN_OVERWRITE	0x20
#define PATTERN_COUNT_MASK	0x1f

/* Maximum number of mismatched bytes in buffer to print */
#define MAX_ERROR_COUNT		32

#define TEST_BUF_SIZE (256 * 1024)
#define DEFAULT_IBWIN_SIZE (2 * 1024 * 1024)

#define U8P uint8_t*

static int fd;
static uint32_t tgt_destid;
static uint64_t tgt_addr;
static uint32_t offset;
static int align;
static uint32_t copy_size = TEST_BUF_SIZE;
static uint32_t ibwin_size;
static uint32_t tbuf_size = TEST_BUF_SIZE;
static int debug;

static struct timespec timediff(struct timespec start, struct timespec end)
{
	struct timespec temp;

	if ((end.tv_nsec - start.tv_nsec) < 0) {
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	return temp;
}

static void dmatest_init_srcs(uint8_t *buf, unsigned int start,
			      unsigned int len, unsigned int buf_size)
{
	unsigned int i;

	for (i = 0; i < start; i++)
		buf[i] = PATTERN_SRC | (~i & PATTERN_COUNT_MASK);
	for (; i < start + len; i++)
		buf[i] = PATTERN_SRC | PATTERN_COPY
			| (~i & PATTERN_COUNT_MASK);
	for (; i < buf_size; i++)
		buf[i] = PATTERN_SRC | (~i & PATTERN_COUNT_MASK);
}

static void dmatest_init_dsts(uint8_t *buf, unsigned int start,
			      unsigned int len, unsigned int buf_size)
{
	unsigned int i;

	for (i = 0; i < start; i++)
		buf[i] = PATTERN_DST | (~i & PATTERN_COUNT_MASK);
	for (; i < start + len; i++)
		buf[i] = PATTERN_DST | PATTERN_OVERWRITE
			| (~i & PATTERN_COUNT_MASK);
	for (; i < buf_size; i++)
		buf[i] = PATTERN_DST | (~i & PATTERN_COUNT_MASK);
}

static void dmatest_mismatch(uint8_t actual, uint8_t pattern,
			     unsigned int index, unsigned int counter,
			     int is_srcbuf)
{
	uint8_t diff = actual ^ pattern;
	uint8_t expected = pattern | (~counter & PATTERN_COUNT_MASK);

	if (is_srcbuf)
		printf("srcbuf[0x%x] overwritten! Expected %02x, got %02x\n",
			index, expected, actual);
	else if ((pattern & PATTERN_COPY)
			&& (diff & (PATTERN_COPY | PATTERN_OVERWRITE)))
		printf("dstbuf[0x%x] not copied! Expected %02x, got %02x\n",
			index, expected, actual);
	else if (diff & PATTERN_SRC)
		printf("dstbuf[0x%x] was copied! Expected %02x, got %02x\n",
			index, expected, actual);
	else
		printf("dstbuf[0x%x] mismatch! Expected %02x, got %02x\n",
			index, expected, actual);
}

static unsigned int dmatest_verify(uint8_t *buf, unsigned int start,
		unsigned int end, unsigned int counter, uint8_t pattern,
		int is_srcbuf)
{
	unsigned int i;
	unsigned int error_count = 0;
	uint8_t actual;
	uint8_t expected;
	unsigned int counter_orig = counter;

	counter = counter_orig;
	for (i = start; i < end; i++) {
		actual = buf[i];
		expected = pattern | (~counter & PATTERN_COUNT_MASK);
		if (actual != expected) {
			if (error_count < MAX_ERROR_COUNT)
				dmatest_mismatch(actual, pattern, i,
							counter, is_srcbuf);
			error_count++;
		}
		counter++;
	}

	if (error_count > MAX_ERROR_COUNT)
		printf("%u errors suppressed\n", error_count - MAX_ERROR_COUNT);

	return error_count;
}

static void *obwtest_buf_alloc(uint32_t size)
{
	void *buf_ptr = NULL;

	buf_ptr = malloc(size);
	if (!buf_ptr)
		perror("malloc");

	return buf_ptr;
}

static void obwtest_buf_free(void *buf)
{
	free(buf);
}

/*
 * This function is called by main() if Inbound Target Memory mode was
 * specified.
 *
 * rio_base - Base RapidIO address for inbound window
 * ib_size - Inbound window and buffer size in bytes
 * loc_addr - Physical address in reserved memory range
 * verify - Flag to enable/disable data verification on exit
 *
 * Returns: 0 if successful or error code returned by mport API.
 */
static int do_ibwin_test(uint64_t rio_base, uint32_t ib_size, uint64_t loc_addr,
		int verify)
{
	int ret;
	uint64_t ib_handle = loc_addr;
	void *ibmap;

	ret = rio_ibwin_map(fd, &rio_base, ib_size, &ib_handle);
	if (ret) {
		printf("Failed to allocate/map IB buffer err=%d\n", ret);
		return ret;
	}

	ibmap = mmap(NULL, ib_size, PROT_READ | PROT_WRITE, MAP_SHARED,
		     fd, ib_handle);
	if (ibmap == MAP_FAILED) {
		perror("mmap");
		goto out;
	}

	memset(ibmap, 0, ib_size);

	printf("\tSuccessfully allocated/mapped IB buffer (rio_base=0x%x_%x)\n",
	       (uint32_t)(rio_base >> 32), (uint32_t)(rio_base & 0xffffffff));

	if (debug)
		printf("\t(h=0x%x_%x, loc=%p)\n", (uint32_t)(ib_handle >> 32),
			(uint32_t)(ib_handle & 0xffffffff), ibmap);
	printf("\t.... press Enter key to exit ....\n");

	/* Pause until a user presses Enter key */
	getchar();

	if (verify)
		dmatest_verify((U8P)ibmap, 0, ib_size, 0,
				PATTERN_SRC | PATTERN_COPY, 0);

	if (munmap(ibmap, ib_size))
		perror("munmap");
out:
	ret = rio_ibwin_free(fd, &ib_handle);
	if (ret)
		printf("Failed to release IB buffer err=%d\n", ret);

	return 0;
}

/*
 * This function performs MEMIO write and read back to/from remote RapidIO
 * target device.
 *
 * Called by main() if MEMIO Master mode was specified.
 *
 * random - If non-zero, enables using random transfer size and offsets
 *           within source and destination buffers.
 * verify - Flag to enable/disable data verification for each write-read cycle
 * loop_count - Number of write-read cycles to perform
 *
 * Returns: 0 if successful or error code returned by mport API.
 */
int do_obwin_test(int random, int verify, int loop_count)
{
	void *buf_src = NULL;
	void *buf_dst = NULL;
	void *obw_ptr = NULL;
	unsigned int src_off, dst_off, len;
	uint64_t obw_handle = 0;
	int i, ret = 0;
	struct timespec wr_starttime, wr_endtime;
	struct timespec rd_starttime, rd_endtime;
	float totaltime;

	/* check specified DMA block size */
	if (copy_size > tbuf_size || copy_size == 0) {
		printf("ERR: invalid transfer size parameter\n");
		printf("     max allowed copy size: %d bytes\n", tbuf_size);
		return -1;
	}

	if (random) {
		printf("\tRANDOM mode is selected for size/offset "
			"combination\n");
		printf("\t\tmax data transfer size: %d bytes\n", copy_size);
		srand(time(NULL));
	} else if (copy_size + offset > tbuf_size) {
		printf("ERR: invalid transfer size/offset "
			"combination\n");
		return -1;
	}

	printf("\tcopy_size=%d offset=0x%x\n", copy_size, offset);

	/* Allocate source and destination buffers */
	buf_src = obwtest_buf_alloc(tbuf_size);
	if (!buf_src) {
		printf("DMA Test: error allocating SRC buffer\n");
		ret = -1;
		goto out;
	}

	buf_dst = obwtest_buf_alloc(tbuf_size);
	if (!buf_dst) {
		printf("DMA Test: error allocating DST buffer\n");
		ret = -1;
		goto out;
	}

	ret = rio_obwin_map(fd, tgt_destid, tgt_addr, tbuf_size, &obw_handle);
	if (ret) {
		printf("rio_obwin_map failed err=%d\n", ret);
		goto out;
	}

	printf("OBW handle 0x%x_%08x\n", (uint32_t)(obw_handle >> 32),
		(uint32_t)(obw_handle & 0xffffffff));

	obw_ptr = mmap(NULL, tbuf_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, obw_handle);
	if (obw_ptr == MAP_FAILED) {
		perror("mmap");
		obw_ptr = NULL;
		goto out_unmap;
	}

	for (i = 1; i <= loop_count; i++) {
		struct timespec time, rd_time;

		if (random) {
			len = rand() % copy_size + 1;
			len = (len >> align) << align;
			if (!len)
				len = 1 << align;
			src_off = rand() % (tbuf_size - len + 1);
			dst_off = rand() % (tbuf_size - len + 1);

			src_off = (src_off >> align) << align;
			dst_off = (dst_off >> align) << align;
		} else {
			len = copy_size;
			src_off = offset;
			dst_off = offset;
		}

		printf("<%d>: len=0x%x src_off=0x%x dst_off=0x%x\n",
			i, len, src_off, dst_off);

		/*
		 * If data verification is requested, fill src and dst buffers
		 * with predefined data
		 */
		if (verify) {
			dmatest_init_srcs((U8P)buf_src, src_off,
					  len, tbuf_size);
			dmatest_init_dsts((U8P)buf_dst, dst_off,
					  len, tbuf_size);
		}

		if (debug)
			printf("\tWrite %d bytes from src offset 0x%x\n",
				len, src_off);
		clock_gettime(CLOCK_MONOTONIC, &wr_starttime);

		/* Write data from the local source buffer to
		 * the remote target inbound buffer */
		memcpy(obw_ptr, (U8P)buf_src + src_off, len);
		clock_gettime(CLOCK_MONOTONIC, &wr_endtime);
		if (debug)
			printf("\tRead %d bytes to dst offset 0x%x\n",
				len, dst_off);

		clock_gettime(CLOCK_MONOTONIC, &rd_starttime);

		/*
		 * Read back data from remote target inbound buffer into
		 * the local destination buffer
		 */
		memcpy((U8P)buf_dst + dst_off, obw_ptr, len);
		clock_gettime(CLOCK_MONOTONIC, &rd_endtime);

		rd_time = timediff(rd_starttime, rd_endtime);

		if (verify) {
			unsigned int error_count;

			if (debug)
				printf("\tVerifying source buffer...\n");
			error_count = dmatest_verify((U8P)buf_src, 0, src_off,
					0, PATTERN_SRC, 1);
			error_count += dmatest_verify((U8P)buf_src, src_off,
					src_off + len, src_off,
					PATTERN_SRC | PATTERN_COPY, 1);
			error_count += dmatest_verify((U8P)buf_src,
					src_off + len, tbuf_size, src_off + len,
					PATTERN_SRC, 1);

			if (debug)
				printf("\tVerifying destination buffer...\n");
			error_count += dmatest_verify((U8P)buf_dst, 0, dst_off,
					0, PATTERN_DST, 0);
			error_count += dmatest_verify((U8P)buf_dst, dst_off,
					dst_off + len, src_off,
					PATTERN_SRC | PATTERN_COPY, 0);
			error_count += dmatest_verify((U8P)buf_dst,
					dst_off + len, tbuf_size, dst_off + len,
					PATTERN_DST, 0);
			if (error_count) {
				printf("\tBuffer verification failed with %d "
					"errors\n", error_count);
				break;
			}

			printf("\tBuffer verification OK!\n");
		} else
			printf("\tBuffer verification is turned off!\n");

		time = timediff(wr_starttime, wr_endtime);
		totaltime = ((double) time.tv_sec +
					(time.tv_nsec / 1000000000.0));
		printf("\t\tWR time: %4f s @ %4.2f MB/s\n",
			totaltime, (len/totaltime)/(1024*1024));
		totaltime = ((double) rd_time.tv_sec +
					(rd_time.tv_nsec / 1000000000.0));
		printf("\t\tRD time: %4f s @ %4.2f MB/s\n",
			totaltime, (len/totaltime)/(1024*1024));
		time = timediff(wr_starttime, rd_endtime);
		totaltime = ((double) time.tv_sec +
					(time.tv_nsec / 1000000000.0));
		printf("\t\tFull Cycle time: %4f s\n", totaltime);
	}

	if (munmap(obw_ptr, tbuf_size))
		perror("munmap");
out_unmap:
	ret = rio_obwin_free(fd, &obw_handle);
	if (ret)
		printf("Failed to release OB window err=%d\n", ret);
out:
	if (buf_src)
		obwtest_buf_free(buf_src);
	if (buf_dst)
		obwtest_buf_free(buf_dst);
	return ret;
}

static void display_help(char *program)
{
	printf("%s - test MEMIO data transfers to/from RapidIO device\n",
		program);
	printf("Usage:\n");
	printf("  %s [options]\n", program);
	printf("options are:\n");
	printf("Common:\n");
	printf("  -M mport_id\n");
	printf("  --mport mport_id\n");
	printf("    local mport device index (default=0)\n");
	printf("  -v turn off buffer data verification\n");
	printf("  --debug (or -d)\n");
	printf("  --help (or -h)\n");
	printf("OBW mapping test mode only:\n");
	printf("  -D xxxx\n");
	printf("  --destid xxxx\n");
	printf("    destination ID of target RapidIO device\n");
	printf("  -A xxxx\n");
	printf("  --taddr xxxx\n");
	printf("    memory address in target RapidIO device\n");
	printf("  -S xxxx\n");
	printf("  --size xxxx\n");
	printf("    data transfer size in bytes (default 0x100)\n");
	printf("  -O xxxx\n");
	printf("  --offset xxxx\n");
	printf("    offset in local data src/dst buffers (default=0)\n");
	printf("  -a n\n");
	printf("  --align n\n");
	printf("    data buffer address alignment\n");
	printf("  -T n\n");
	printf("  --repeat n\n");
	printf("    repeat test n times (default=1)\n");
	printf("  -B xxxx size of test buffer and OBW aperture (in MB, e.g -B2)\n");
	printf("  -r use random size and local buffer offset values\n");
	printf("Inbound Window mode only:\n");
	printf("  -i\n");
	printf("    allocate and map inbound window (memory) using default parameters\n");
	printf("  -I xxxx\n");
	printf("  --ibwin xxxx\n");
	printf("    inbound window (memory) size in bytes\n");
	printf("  -R xxxx\n");
	printf("  --ibbase xxxx\n");
	printf("    inbound window base address in RapidIO address space\n");
	printf("  -L xxxx\n");
	printf("  --laddr xxxx\n");
	printf("    physical address of reserved local memory to use\n");
	printf("\n");
}

int main(int argc, char **argv)
{
	uint32_t mport_id = 0;
	int option;
	int do_rand = 0;
	int verify = 1;
	unsigned int repeat = 1;
	uint64_t rio_base = RIO_MAP_ANY_ADDR;
	uint64_t loc_addr = RIO_MAP_ANY_ADDR;
	static const struct option options[] = {
		{ "destid", required_argument, NULL, 'D' },
		{ "taddr",  required_argument, NULL, 'A' },
		{ "size",   required_argument, NULL, 'S' },
		{ "offset", required_argument, NULL, 'O' },
		{ "align",  required_argument, NULL, 'a' },
		{ "repeat", required_argument, NULL, 'T' },
		{ "ibwin",  required_argument, NULL, 'I' },
		{ "ibbase", required_argument, NULL, 'R' },
		{ "mport",  required_argument, NULL, 'M' },
		{ "laddr", required_argument, NULL, 'L'},
		{ "faf",    no_argument, NULL, 'F' },
		{ "async",  no_argument, NULL, 'Y' },
		{ "debug",  no_argument, NULL, 'd' },
		{ "help",   no_argument, NULL, 'h' },
		{ }
	};
	char *program = argv[0];
	struct rio_mport_properties prop;
	int rc = EXIT_SUCCESS;

	while (1) {
		option = getopt_long_only(argc, argv,
				"rvwdhika:A:D:I:O:M:R:S:T:B:L:", options, NULL);
		if (option == -1)
			break;
		switch (option) {
		case 'A':
			tgt_addr = strtoull(optarg, NULL, 0);
			break;
		case 'a':
			align = strtol(optarg, NULL, 0);
			break;
		case 'D':
			tgt_destid = strtol(optarg, NULL, 0);
			break;
		case 'O':
			offset = strtol(optarg, NULL, 0);
			break;
		case 'S':
			copy_size = strtol(optarg, NULL, 0);
			break;
		case 'T':
			repeat = strtol(optarg, NULL, 0);
			break;
		case 'B':
			tbuf_size = strtol(optarg, NULL, 0) * 1024 * 1024;
			break;
		case 'r':
			do_rand = 1;
			break;
		case 'I':
			ibwin_size = strtol(optarg, NULL, 0);
			break;
		case 'i':
			ibwin_size = DEFAULT_IBWIN_SIZE;
			break;
		case 'R':
			rio_base = strtoull(optarg, NULL, 0);
			break;
		case 'L':
			loc_addr = strtoull(optarg, NULL, 0);
			break;
		case 'M':
			mport_id = strtol(optarg, NULL, 0);
			break;
		case 'v':
			verify = 0;
			break;
		case 'd':
			debug = 1;
			break;
		case 'h':
		default:
			display_help(program);
			exit(EXIT_SUCCESS);
		}
	}

	fd = rio_mport_open(mport_id, 0);
	if (fd < 0) {
		printf("OBW Test: unable to open mport%d device err=%d\n",
			mport_id, errno);
		exit(EXIT_FAILURE);
	}

	if (!rio_query_mport(fd, &prop)) {
		display_mport_info(&prop);

		if (prop.link_speed == 0) {
			printf("SRIO link is down. Test aborted.\n");
			rc = EXIT_FAILURE;
			goto out;
		}
	} else {
		printf("Failed to obtain mport information\n");
		printf("Using default configuration\n\n");
	}

	if (ibwin_size) {
		printf("+++ RapidIO Inbound Window Mode +++\n");
		printf("\tmport%d ib_size=0x%x PID:%d\n",
			mport_id, ibwin_size, (int)getpid());
		if (loc_addr != RIO_MAP_ANY_ADDR)
			printf("\tloc_addr=0x%llx\n",
				(unsigned long long)loc_addr);

		do_ibwin_test(rio_base, ibwin_size, loc_addr, verify);
	} else {
		printf("+++ RapidIO Outbound Window Mapping Test +++\n");
		printf("\tmport%d destID=%d rio_addr=0x%llx repeat=%d PID:%d\n",
			mport_id, tgt_destid, (unsigned long long)tgt_addr,
			repeat, (int)getpid());
		printf("\tbuf_size=0x%x\n", tbuf_size);

		do_obwin_test(do_rand, verify, repeat);
	}

out:
	close(fd);
	exit(rc);
}

#ifdef __cplusplus
}
#endif
