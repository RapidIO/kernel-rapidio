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
 * Test register access operations to/from RapidIO device
 *
 * The program performs access to registers of local (mport) and
 * remote RapidIO devices.
 * If -q option is specified, it displays attribute/status information for
 * selected local mport device.
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

#include <rio_mport_lib.h>

#ifdef __cplusplus
extern "C" {
#endif

static int debug;

static void display_help(char *program)
{
	printf("%s - test register access operations to/from RapidIO device\n",
			program);
	printf("Usage:\n");
	printf("  %s [options]\n", program);
	printf("Options are:\n");
	printf("  --help (or -h)\n");
	printf("  -M mport_id\n");
	printf("  --mport mport_id\n");
	printf("    local mport device index (default 0)\n");
	printf("  --debug (or -d)\n");
	printf("  -D xxxx\n");
	printf("  --destid xxxx\n");
	printf("    destination ID of target RapidIO device (default any)\n");
	printf("    If not specified access to local mport registers\n");
	printf("  -H xxxx\n");
	printf("  --hop xxxx\n");
	printf("    hop count target RapidIO device (default 0xff)\n");
	printf("  -S xxxx\n");
	printf("  --size xxxx\n");
	printf("    data transfer size in bytes (default 4)\n");
	printf("  -O xxxx\n");
	printf("  --offset xxxx\n");
	printf("    offset in register space (default 0)\n");
	printf("  -w\n");
	printf("    perform write operation\n");
	printf("  -V xxxx\n");
	printf("  --data xxxx\n");
	printf("    32-bit value to write into the device register (default 0)\n");
	printf("  -q\n");
	printf("    query mport attributes\n");
	printf("\n");
}

int main(int argc, char **argv)
{
	char *program = argv[0];
	int fd;
	uint32_t mport_id = 0;
	uint32_t tgt_destid = 0;
	uint32_t tgt_hc = 0xff;
	uint32_t tgt_remote = 0, tgt_write = 0, do_query = 0;
	uint32_t op_size = sizeof(uint32_t);
	uint32_t offset = 0;
	uint32_t data = 0;

	static const struct option options[] = {
			{"mport",  required_argument, NULL, 'M'},
			{"destid", required_argument, NULL, 'D'},
			{"hop",    required_argument, NULL, 'H'},
			{"offset", required_argument, NULL, 'O'},
			{"size",   required_argument, NULL, 'S'},
			{"data",   required_argument, NULL, 'V'},
			{"debug",  no_argument, NULL, 'd'},
			{"help",   no_argument, NULL, 'h'},
	};

	int option;
	struct rio_mport_properties prop;
	int rc = EXIT_SUCCESS;

	/* Parse command line options, if any */
	while (1) {
		option = getopt_long_only(argc, argv,
				"wdhqH:D:O:M:S:V:", options, NULL);
		if (option == -1)
			break;
		switch (option) {
		case 'D':
			tgt_destid = strtol(optarg, NULL, 0);
			tgt_remote = 1;
			break;
		case 'H':
			tgt_hc = strtol(optarg, NULL, 0);
			break;
		case 'O':
			offset = strtol(optarg, NULL, 0);
			break;
		case 'S':
			op_size = strtol(optarg, NULL, 0);
			break;
		case 'M':
			mport_id = strtol(optarg, NULL, 0);
			break;
		case 'V':
			data = strtoul(optarg, NULL, 0);
			break;
		case 'w':
			tgt_write = 1;
			break;
		case 'd':
			debug = 1;
			break;
		case 'q':
			do_query = 1;
			break;
		case 'h':
		default:
			display_help(program);
			exit(EXIT_SUCCESS);
		}
	}

	fd = rio_mport_open(mport_id, 0);
	if (fd < 0) {
		printf("Unable to open mport%d device err=%d\n",
			mport_id, errno);
		exit(EXIT_FAILURE);
	}

	if (do_query) {
		rc = rio_query_mport(fd, &prop);
		if (!rc) {
			display_mport_info(&prop);
			if (prop.link_speed == 0)
				printf("SRIO link is down. Test aborted.\n");
		}

		goto out;
	}

	if (tgt_remote) {
		/* For remote target execute maintenance transaction */
		if (tgt_write) {
			if (debug)
				printf("Write to dest=0x%x hc=0x%x offset=0x%x data=0x%08x\n",
					tgt_destid, tgt_hc, offset, data);
			rc = rio_maint_write(fd, tgt_destid, tgt_hc, offset,
						op_size, data);
		} else {
			if (debug)
				printf("Read from dest=0x%x hc=0x%x offset=0x%x\n",
					tgt_destid, tgt_hc, offset);
			rc = rio_maint_read(fd, tgt_destid, tgt_hc, offset,
						op_size, &data);
			if (!rc)
				printf("\tdata = 0x%08x\n", data);
		}
	} else {
		/* For local target execute register access operation */
		if (tgt_write) {
			if (debug)
				printf("Write to local offset=0x%x data=0x%08x\n",
					offset, data);
			rc = rio_lcfg_write(fd, offset, op_size, data);
		} else {
			if (debug)
				printf("Read from local offset=0x%x\n", offset);
			rc = rio_lcfg_read(fd, offset, op_size, &data);
			if (!rc)
				printf("\tdata = 0x%08x\n", data);
		}
	}

out:
	if (rc) {
		printf("ERR %d\n", rc);
		rc = EXIT_FAILURE;
	} else
		rc = EXIT_SUCCESS;

	/* Close the mport fd */
	close(fd);
	exit(rc);
}

#ifdef __cplusplus
}
#endif
