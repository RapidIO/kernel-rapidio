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

/*
 * riodp_test_dma.c
 * Test DMA data transfers to/from RapidIO device.
 *
 * This program can be invoked in two modes:
 *
 * 1. DMA transfer initiator (master).
 *
 * 2. Target inbound memory allocator.
 *
 * The program starts in Inbound Target Memory mode when option -i or -I
 * is specified. To avoid DMA transfer error messages target inbound memory
 * must be created first. When started in inbound memory mode, riodp_test_dma
 * program will display RapidIO base address assigned to the inbound window.
 * This address should be used to define target address of DMA data transfers.
 */

#include <stdio.h>
#include <stdbool.h>
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
#include <ctype.h>
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

struct dma_async_wait_param {
	uint32_t token; /* DMA transaction ID token */
	int err; /* error code returned to caller */
};

#define U8P uint8_t*

/*
 * Max data block length that can be transferred by DMA channel
 * by default configured for 32 scatter/gather entries of 4K.
 * Size returned by mport driver should be when available.
 * Shall less or eq. TEST_BUF_SIZE.
 */
static uint32_t max_sge = 32;
static uint32_t max_size = 4096;

/* Maximum number of mismatched bytes in buffer to print */
#define MAX_ERROR_COUNT 32
/* Default size of source and destination data buffers */
#define TEST_BUF_SIZE (2 * 1024 * 1024)
/* Default size of RapidIO target inbound window */
#define DEFAULT_IBWIN_SIZE (2 * 1024 * 1024)

static int fd;
static uint32_t tgt_destid;
static uint64_t tgt_addr;
static uint32_t offset;
static int align;
static uint32_t dma_size;
static uint32_t ibwin_size;
static int debug;
static uint32_t tbuf_size = TEST_BUF_SIZE;


static void usage(char *program)
{
	printf("%s - test DMA data transfers to/from RapidIO device\n",
			program);
	printf("Usage:\n");
	printf("  %s [options]\n", program);
	printf("options are:\n");
	printf("Common:\n");
	printf("  --help (or -h)\n");
	printf("  -M mport_id\n");
	printf("  --mport mport_id\n");
	printf("    local mport device index (default = 0)\n");
	printf("  -L xxxx\n");
	printf("  --laddr xxxx\n");
	printf("    physical address of reserved local memory to use (default = auto)\n");
	printf("  -v turn off buffer data verification\n");
	printf("  --debug (or -d)\n");
	printf("DMA test mode only:\n");
	printf("  -D xxxx\n");
	printf("  --destid xxxx\n");
	printf("    destination ID of target RapidIO device (default = 0)\n");
	printf("  -A xxxx\n");
	printf("  --taddr xxxx\n");
	printf("    memory address in target RapidIO device (default = 0)\n");
	printf("  -S xxxx\n");
	printf("  --size xxxx\n");
	printf("    data transfer size in bytes (default 0x100)\n");
	printf("  -B xxxx\n");
	printf("    data buffer size (SRC and DST) in bytes (default = %u)\n",
			TEST_BUF_SIZE);
	printf("  -O xxxx\n");
	printf("  --offset xxxx\n");
	printf("    offset in local data src/dst buffers (default = 0)\n");
	printf("  -a n\n");
	printf("  --align n\n");
	printf("    data buffer address alignment (default 0)\n");
	printf("  -T n\n");
	printf("  --repeat n\n");
	printf("    repeat test n times (default 1)\n");
	printf("  -k use coherent kernel space buffer allocation\n");
	printf("  -r use random size and local buffer offset values\n");
	printf("  --faf use FAF DMA transfer mode (default SYNC)\n");
	printf("  --async use ASYNC DMA transfer mode (default SYNC)\n");
	printf("  -n xxxx[,yyyy]\n");
	printf("  --sinterleave xxxx[,yyyy]\n");
	printf("    source interleave setting size and distance\n");
	printf("  -N xxxx[,yyyy]\n");
	printf("  --dinterleave xxxx[,yyyy]\n");
	printf("    destination interleave setting size and distance\n");
	printf("Inbound Window mode only:\n");
	printf("  -i\n");
	printf("    allocate and map inbound window (memory) using default parameters\n");
	printf("  -I xxxx\n");
	printf("  --ibwin xxxx\n");
	printf("    inbound window (memory) size in bytes (default = %u)\n",
			DEFAULT_IBWIN_SIZE);
	printf("  -R xxxx\n");
	printf("  --ibbase xxxx\n");
	printf("    inbound window base address in RapidIO address space (default = auto)\n");
	printf("\n");
}

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
		buf[i] = PATTERN_SRC | PATTERN_COPY | (~i & PATTERN_COUNT_MASK);
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

static void dmatest_dump_mem(uint8_t *buf, unsigned int buf_size)
{
	unsigned int i;
	unsigned int dumpLen;
	uint8_t *addr = buf;

	dumpLen = buf_size;
	if (dumpLen > 0x200)
		dumpLen = 0x200;

	for (i = 0; i < dumpLen; i++) {
		if (i % 16 == 0) {
			printf("\n%p: %02x ", addr, buf[i]);
			addr += 16;
		} else
			printf("%02x ", buf[i]);
	}
	printf("\n");
}

static void dmatest_mismatch(uint8_t actual, uint8_t pattern,
		unsigned int index, unsigned int counter, int is_srcbuf)
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

/*
 * Verify the given buffer for the expected contents.
 *
 * Loop through buffer from start to end comparing against expected pattern
 * for the type of segment.  Depending on which part of the buffer was written,
 * The original contents should be there or the contents from the DMA.
 *
 * buf       - Buffer to check
 * start     - Start offset
 * end       - End offset
 * counter   - Cumulative 'offset' of the byte being examined (for error reporting)
 * pattern   - Expected pattern
 * is_srcbuf - Flag indicating if buffer is source buffer (for error reporting)
 *
 * Return: Count of the number of verification failures found.
 */
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
				dmatest_mismatch(actual, pattern, i, counter,
						is_srcbuf);
			error_count++;
		}
		counter++;
	}

	if (error_count > MAX_ERROR_COUNT)
		printf("%u errors suppressed\n", error_count - MAX_ERROR_COUNT);
	return error_count;
}

/*
 * This function is called by do_dma_test() to allocate source and
 * destination buffers in DMA Master mode.
 *
 * If parameter handle is a valid pointer (not NULL), this function will allocate
 * a kernel-space contiguous data buffer and return its physical address into
 * variable specified by the handle. The physical address of the buffer will be mapped
 * into process address space and returned to a caller.

 * If handle == NULL, this function allocates a paged user-space data buffer.
 *
 * fd     - descriptor of mport device to use
 * size   - Buffer size in bytes
 * handle - Physical memory address of the memory segment
 *
 * Return: A user-space pointer to the allocated buffer or NULL on failure.
 */
void *dmatest_buf_alloc(int fd, uint32_t size, uint64_t *handle)
{
	void *buf_ptr = NULL;
	uint64_t h;
	int ret;

	if (handle) {
		h = *handle;

		ret = rio_dbuf_alloc(fd, size, &h);
		if (ret) {
			if (debug)
				printf("rio_dbuf_alloc() failed err=%d\n", ret);
			return NULL;
		}

		buf_ptr = mmap(NULL, size,
				PROT_READ | PROT_WRITE, MAP_SHARED, fd, h);
		if (buf_ptr == MAP_FAILED) {
			perror("mmap");
			buf_ptr = NULL;
			ret = rio_dbuf_free(fd, handle);
			if (ret && debug)
				printf("rio_dbuf_free failed err=%d\n", ret);
		} else
			*handle = h;
	} else {
		buf_ptr = malloc(size);
		if (buf_ptr == NULL)
			perror("malloc");
	}

	return buf_ptr;
}

/*
 * This function is called by do_dma_test() to free source and
 * destination buffers allocated in DMA Master mode.
 *
 * If parameter handle is a valid pointer (not NULL), this function will unmap
 * and free a kernel-space contiguous data buffer, otherwise: user-space buffer.
 *
 * fd     - Descriptor of mport device to use
 * buf    - User-space buffer address
 * size   - Buffer size in bytes
 * handle - Physical memory address of the memory segment
 *
 * Return: None
 */
static void dmatest_buf_free(int fd, void *buf, uint32_t size, uint64_t *handle)
{
	if (handle && *handle) {
		int ret;

		if (munmap(buf, size))
			perror("munmap");

		ret = rio_dbuf_free(fd, handle);
		if (ret)
			printf("rio_dbuf_free failed err=%d\n", ret);
	} else if (buf)
		free(buf);
}

/*
 * This function is called by main() if Inbound Target Memory mode was
 * specified.
 *
 * rio_base - Base RapidIO address for inbound window
 * ib_size  - Inbound window and buffer size in bytes
 * loc_addr - Physical address in reserved memory range
 * verify   - Flag to enable/disable data verification on exit
 *
 * Return: 0 if successful or error code returned by mport API.
 */
static int do_ibwin_test(uint64_t rio_base, uint32_t ib_size, uint64_t loc_addr,
		int verify)
{
	int ret;
	uint64_t ib_handle = loc_addr;
	void *ibmap;

	/* Request mport's inbound window mapping */
	ret = rio_ibwin_map(fd, &rio_base, ib_size, &ib_handle);
	if (ret) {
		printf("Failed to allocate/map IB buffer err=%d\n", ret);
		close(fd);
		return ret;
	}

	ibmap = mmap(NULL, ib_size, PROT_READ | PROT_WRITE,
		     MAP_SHARED, fd, ib_handle);
	if (ibmap == MAP_FAILED) {
		perror("mmap");
		goto out;
	}

	memset(ibmap, 0, ib_size);

	printf("\tAllocated/mapped IB buffer (rio_base=0x%x_%08x)\n",
		(uint32_t)(rio_base >> 32), (uint32_t)(rio_base & 0xffffffff));

	if (debug)
		printf("\t(h=0x%x_%x, loc=%p)\n", (uint32_t)(ib_handle >> 32),
			(uint32_t)(ib_handle & 0xffffffff), ibmap);

	/* Pause until a user presses Enter key */
	printf("\t.... press Enter key to exit ....\n");
	getchar();

	/* Verify data before exit (if requested) */
	if (verify)
		dmatest_verify((U8P)ibmap, 0, ib_size, 0,
				PATTERN_SRC | PATTERN_COPY, 0);

	/* Unmap kernel-space data buffer */
	if (munmap(ibmap, ib_size))
		perror("munmap");
out:
	ret = rio_ibwin_free(fd, &ib_handle);
	if (ret)
		printf("Failed to release IB buffer err=%d\n", ret);

	return 0;
}

static void *dma_async_wait(void *arg)
{
	struct dma_async_wait_param *param = (struct dma_async_wait_param *)arg;
	int ret;

	ret = rio_wait_async(fd, param->token, 3000);
	param->err = ret;
	return &param->err;
}

/*
 * This function performs DMA write and read back to/from remote RapidIO
 * target device.
 *
 * Called by main() if DMA Master mode was specified.
 *
 * random - If non-zero, enables using random transfer size and offsets
 *          within source and destination buffers.
 * kbuf_mode - If non-zero, use kernel-space contiguous buffers
 * verify - Flag to enable/disable data verification for each write-read cycle
 * loop_count - Number of write-read cycles to perform
 * sync - DMA transfer synchronization mode
 * loc_addr - Physical address in reserved memory range
 *
 * Return: 0 if successful or error code returned by mport API.
 */
static int do_dma_test(int random, int kbuf_mode, int verify, int loop_count,
		enum rio_transfer_sync sync, uint64_t loc_addr)
{
	void *buf_src = NULL;
	void *buf_dst = NULL;
	unsigned int src_off, dst_off, len;
	uint64_t src_handle = RIO_MAP_ANY_ADDR;
	uint64_t dst_handle = RIO_MAP_ANY_ADDR;
	int i, ret = 0;
	uint32_t max_hw_size; /* max DMA transfer size supported by HW */
	enum rio_transfer_sync rd_sync;
	struct timespec wr_starttime, wr_endtime;
	struct timespec rd_starttime, rd_endtime;
	float totaltime;

	if (kbuf_mode)
		max_hw_size = max_size;
	else
		max_hw_size = (!max_sge) ?
				tbuf_size : (max_sge * getpagesize());

	/* check specified DMA block size */
	if (dma_size > 0 && (dma_size > tbuf_size || dma_size > max_hw_size)) {
		printf("ERR: invalid buffer size parameter\n");
		printf("     max allowed buffer size: %d bytes\n",
			(max_hw_size > tbuf_size) ? tbuf_size : max_hw_size);
		return -1;
	}

	if (random) {
		/* If not specified, set max supported dma_size */
		if (!dma_size)
			dma_size = (max_hw_size < tbuf_size) ?
						max_hw_size : tbuf_size;

		printf("\tRANDOM mode is selected for size and offset\n");
		printf("\t\tmax data transfer size: %d bytes\n", dma_size);
		srand(time(NULL));
	} else if (dma_size + offset > tbuf_size || dma_size == 0) {
		printf("ERR: invalid transfer size/offset combination\n");
		return -1;
	} else
		printf("\tdma_size=%d offset=0x%x\n", dma_size, offset);

	/*
	 * Allocate source and destination buffers in specified memory
	 * space (kernel or user)
	 */

	if (kbuf_mode && loc_addr != RIO_MAP_ANY_ADDR) {
		src_handle = loc_addr;
		dst_handle = loc_addr + tbuf_size;
	}

	buf_src = dmatest_buf_alloc(fd, tbuf_size,
				    kbuf_mode ? &src_handle : NULL);
	if (buf_src == NULL) {
		printf("DMA Test: error allocating SRC buffer\n");
		ret = -1;
		goto out_src;
	}

	buf_dst = dmatest_buf_alloc(fd, tbuf_size,
				    kbuf_mode ? &dst_handle : NULL);
	if (buf_dst == NULL) {
		printf("DMA Test: error allocating DST buffer\n");
		ret = -1;
		goto out_dst;
	}

	 /* Enter write-read cycle */
	for (i = 1; i <= loop_count; i++) {
		struct timespec time, rd_time;
		pthread_t wr_thr, rd_thr;
		struct dma_async_wait_param wr_wait, rd_wait;

		if (random) {
			len = rand() % dma_size + 1;
			len = (len >> align) << align;
			if (!len)
				len = 1 << align;
			src_off = rand() % (tbuf_size - len + 1);
			dst_off = rand() % (tbuf_size - len + 1);

			src_off = (src_off >> align) << align;
			dst_off = (dst_off >> align) << align;
		} else {
			len = dma_size;
			src_off = offset;
			dst_off = offset;
		}

		printf("<%d>: len=0x%x src_off=0x%x dst_off=0x%x\n", i, len,
			src_off, dst_off);

		/*
		 * If data verification is requested, fill src and dst buffers
		 * with predefined data
		 */
		if (verify) {
			dmatest_init_srcs((U8P)buf_src, src_off, len,
					tbuf_size);
			dmatest_init_dsts((U8P)buf_dst, dst_off, len,
					tbuf_size);
			if (debug) {
				printf("Buffers before xfer:\n");
				printf("\nSRC:");
				dmatest_dump_mem((U8P)buf_src, tbuf_size);
				printf("\nDST:");
				dmatest_dump_mem((U8P)buf_dst, tbuf_size);
			}
		}

		if (debug)
			printf("\tWrite %d bytes from src offset 0x%x\n",
				len, src_off);

		clock_gettime(CLOCK_MONOTONIC, &wr_starttime);

		/*
		 * Write data from a local source buffer to remote target
		 * inbound buffer
		 */
		if (kbuf_mode)
			ret = rio_dma_write_d(fd, tgt_destid, tgt_addr,
					src_handle, src_off, len,
					RIO_EXCHANGE_NWRITE_R, sync, NULL);
		else
			ret = rio_dma_write(fd, tgt_destid, tgt_addr,
					(U8P)buf_src + src_off, len,
					RIO_EXCHANGE_NWRITE_R, sync, NULL);

		/* If in ASYNC DMA transfer mode, create a wait thread */
		if ((sync == RIO_TRANSFER_ASYNC) && (ret >= 0)) {
			wr_wait.token = ret;
			wr_wait.err = -1;
			ret = pthread_create(&wr_thr, NULL, dma_async_wait,
					     (void *)&wr_wait);
			if (ret)
				printf("\tERR: Failed to create WR wait thread err=%d\n",
					ret);
		}

		clock_gettime(CLOCK_MONOTONIC, &wr_endtime);
		if (ret) {
			printf("DMA Test: write DMA transfer failed err=%d\n",
				ret);
			goto out;
		}

		if (debug)
			printf("\tRead %d bytes to dst offset 0x%x\n", len,
				dst_off);

		/*
		 * RIO_TRANSFER_FAF is not available for read operations.
		 * Force SYNC instead.
		 */
		rd_sync = (sync == RIO_TRANSFER_FAF) ? RIO_TRANSFER_SYNC : sync;

		clock_gettime(CLOCK_MONOTONIC, &rd_starttime);

		/*
		 * Read back data from remote target inbound buffer into
		 * a local destination buffer
		 */
		if (kbuf_mode)
			ret = rio_dma_read_d(fd, tgt_destid, tgt_addr,
					dst_handle, dst_off, len, rd_sync,
					NULL);
		else
			ret = rio_dma_read(fd, tgt_destid, tgt_addr,
					(U8P)buf_dst + dst_off, len, rd_sync,
					NULL);

		/* If in ASYNC DMA transfer mode, create waiting thread */
		if ((sync == RIO_TRANSFER_ASYNC) && (ret >= 0)) {
			rd_wait.token = ret;
			rd_wait.err = -1;
			ret = pthread_create(&rd_thr, NULL, dma_async_wait,
					     (void *)&rd_wait);
			if (ret)
				printf("\tERR: Failed to create RD wait thread err=%d\n",
					ret);
		}

		clock_gettime(CLOCK_MONOTONIC, &rd_endtime);
		if (ret) {
			printf("DMA Test: read DMA transfer failed err=%d\n",
				ret);
			goto out;
		}

		rd_time = timediff(rd_starttime, rd_endtime);

		/*
		 * If in ASYNC DMA transfer mode,
		 * wait for notification threads completion
		 */
		if (sync == RIO_TRANSFER_ASYNC) {
			pthread_join(wr_thr, NULL);
			pthread_join(rd_thr, NULL);
			clock_gettime(CLOCK_MONOTONIC, &rd_endtime);

			if (wr_wait.err)
				printf("Wait for DMA_WR: err=%d\n",
					wr_wait.err);
			if (rd_wait.err)
				printf("Wait for DMA_RD: err=%d\n",
					rd_wait.err);
			if (wr_wait.err || rd_wait.err)
				goto out;
		}

		/*
		 * If data verification is requested,
		 * verify data transfer results
		 */
		if (verify) {
			unsigned int error_count;

			if (debug) {
				printf("Buffer contents after transfer:\n");
				printf("\nSRC:");
				dmatest_dump_mem((U8P)buf_src, tbuf_size);
				printf("\nDST:");
				dmatest_dump_mem((U8P)buf_dst, tbuf_size);
			}

			if (debug)
				printf("\tVerifying source buffer...\n");
			error_count = dmatest_verify((U8P)buf_src, 0, src_off,
						0, PATTERN_SRC, 1);
			error_count += dmatest_verify((U8P)buf_src, src_off,
						src_off + len, src_off,
						PATTERN_SRC | PATTERN_COPY, 1);
			error_count += dmatest_verify((U8P)buf_src,
						src_off + len,
						tbuf_size, src_off + len,
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
				printf("\tBuffer verification failed with %d errors\n",
					error_count);
				break;
			}

			printf("\tBuffer verification OK!\n");
		} else
			printf("\tBuffer verification is turned off!\n");

		time = timediff(wr_starttime, wr_endtime);
		totaltime =  ((double)time.tv_sec
				+ (time.tv_nsec / 1000000000.0));
		if (sync != RIO_TRANSFER_SYNC)
			printf("\t\tWR time: %4f s\n", totaltime);
		else
			printf("\t\tWR time: %4f s @ %4.2f MB/s\n", totaltime,
				(len / totaltime) / (1024 * 1024));

		totaltime = ((double)rd_time.tv_sec
				+ (rd_time.tv_nsec / 1000000000.0));

		if (sync != RIO_TRANSFER_SYNC)
			printf("\t\tRD time: %4f s\n", totaltime);
		else
			printf("\t\tRD time: %4f s @ %4.2f MB/s\n", totaltime,
				(len / totaltime) / (1024 * 1024));

		time = timediff(wr_starttime, rd_endtime);
		totaltime = ((double)time.tv_sec
				+ (time.tv_nsec / 1000000000.0));
		printf("\t\tFull Cycle time: %4f s\n", totaltime);
	}

out:
	/* Free source and destination buffers */
	dmatest_buf_free(fd, buf_dst, tbuf_size,
			kbuf_mode ? &dst_handle : NULL);

out_dst:
	dmatest_buf_free(fd, buf_src, tbuf_size,
			kbuf_mode ? &src_handle : NULL);

out_src:
	return ret;

}

/*
 * Starting point for the test program
 *
 * Returns: 0 if success
 */
int main(int argc, char **argv)
{
	char *program = argv[0];
	uint32_t mport_id = 0;
	int option;
	int do_rand = 0;
	int kbuf_mode = 0; /* 0 - user-space, 1 - kernel */
	int verify = 1;
	uint32_t repeat = 1;
	uint64_t rio_base = RIO_MAP_ANY_ADDR;
	uint64_t loc_addr = RIO_MAP_ANY_ADDR;
	enum rio_transfer_sync sync = RIO_TRANSFER_SYNC;
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
			{ "laddr",  required_argument, NULL, 'L' },
			{ "faf",    no_argument, NULL, 'F' },
			{ "async",  no_argument, NULL, 'Y' },
			{ "debug",  no_argument, NULL, 'd' },
			{ "help",   no_argument, NULL, 'h' },
	};

	struct rio_mport_properties prop;
	int has_dma = 1;
	int rc = EXIT_SUCCESS;

	/* Parse command line options, if any */
	while ((option = getopt_long_only(argc, argv,
			"rvdhikaFY:A:D:I:O:M:R:S:T:B:L:",
			options, NULL))) {
		if (option == -1)
			break;
		switch (option) {
		case 'A':
			tgt_addr = strtoull(optarg, NULL, 0);
			break;
		case 'L':
			loc_addr = strtoull(optarg, NULL, 0);
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
			dma_size = strtol(optarg, NULL, 0);
			break;
		case 'B':
			tbuf_size = strtol(optarg, NULL, 0);
			break;
		case 'T':
			repeat = strtol(optarg, NULL, 0);
			break;
		case 'k':
			kbuf_mode = 1;
			break;
		case 'r':
			do_rand = 1;
			break;
		case 'F':
			sync = RIO_TRANSFER_FAF;
			break;
		case 'Y':
			sync = RIO_TRANSFER_ASYNC;
			break;
			/* Inbound Memory (window) Mode options */
		case 'I':
			ibwin_size = strtol(optarg, NULL, 0);
			break;
		case 'i':
			ibwin_size = DEFAULT_IBWIN_SIZE;
			break;
		case 'R':
			rio_base = strtoull(optarg, NULL, 0);
			break;
			/* Options common for all modes */
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
			usage(program);
			exit(EXIT_SUCCESS);
			break;
		default:
			printf("ERROR: Invalid command line option\n");
			usage(program);
			exit(EXIT_FAILURE);
		}
	}

	fd = rio_mport_open(mport_id, 0);
	if (fd < 0) {
		printf("DMA Test: unable to open mport%d device err=%d\n",
			mport_id, errno);
		exit(EXIT_FAILURE);
	}

	if (!rio_query_mport(fd, &prop)) {
		display_mport_info(&prop);

		if (prop.flags & RIO_MPORT_DMA) {
			align = prop.dma_align;
			max_sge = prop.dma_max_sge;
			max_size = prop.dma_max_size;
		} else
			has_dma = 0;

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
		printf("\tmport%d ib_size=0x%x PID:%d\n", mport_id, ibwin_size,
			(int)getpid());
		if (loc_addr != RIO_MAP_ANY_ADDR)
			printf("\tloc_addr=0x%llx\n",
				(unsigned long long)loc_addr);

		do_ibwin_test(rio_base, ibwin_size, loc_addr, verify);
	} else if (has_dma) {
		printf("+++ RapidIO DMA Test +++\n");
		printf("\tmport%d destID=%d rio_addr=0x%llx align=%d repeat=%d PID:%d\n",
			mport_id, tgt_destid, (unsigned long long)tgt_addr,
			align, repeat, (int)getpid());

		printf("\tsync=%d (%s)\n", sync,
			(sync == RIO_TRANSFER_SYNC) ? "SYNC" :
				(sync == RIO_TRANSFER_FAF) ? "FAF" : "ASYNC");

		if (loc_addr != RIO_MAP_ANY_ADDR)
			printf("\tloc_addr=0x%llx\n",
					(unsigned long long)loc_addr);

		if (do_dma_test(do_rand, kbuf_mode, verify, repeat,
				sync, loc_addr)) {
			printf("DMA test FAILED\n\n");
			rc = EXIT_FAILURE;
		}
	} else {
		printf("No DMA support. Test aborted.\n\n");
		rc = EXIT_FAILURE;
	}

out:
	close(fd);
	exit(rc);
}

#ifdef __cplusplus
}
#endif
