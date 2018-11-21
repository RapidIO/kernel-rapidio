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

#include <rio_mport_lib.h>

#ifdef __cplusplus
extern "C" {
#endif

static int debug;

static volatile sig_atomic_t rcv_exit;
static volatile sig_atomic_t report_status;

static void db_sig_handler(int signum)
{
	switch (signum) {
	case SIGTERM:
		rcv_exit = 1;
		break;
	case SIGINT:
		rcv_exit = 1;
		break;
	case SIGUSR1:
		report_status = 1;
		break;
	}
}

static int do_dbrcv_test(int fd, uint32_t rioid, uint16_t start, uint16_t end)
{
	int ret;
	struct rio_event evt;

	ret = rio_dbrange_enable(fd, rioid, start, end);
	if (ret) {
		printf("Failed to enable DB range, err=%d\n", ret);
		return ret;
	}

	while (!rcv_exit) {

		ret = read(fd, &evt, sizeof(struct rio_event));
		if (ret < 0) {
			if (errno == EAGAIN)
				continue;
			else {
				printf("Failed to read event, err=%d\n", errno);
				break;
			}
		}

		if (evt.header == RIO_DOORBELL)
			printf("\tDB 0x%04x from destID %d\n",
				evt.u.doorbell.payload, evt.u.doorbell.rioid);
		else
			printf("\tIgnoring event type %d)\n", evt.header);
	}

	ret = rio_dbrange_disable(fd, rioid, start, end);
	if (ret) {
		printf("Failed to disable DB range, err=%d\n", ret);
		return ret;
	}

	return 0;
}

static int do_dbsnd_test(int fd, uint32_t rioid, uint16_t dbval)
{
	struct rio_event evt;
	int ret = 0;

	evt.header = RIO_DOORBELL;
	evt.u.doorbell.rioid = rioid;
	evt.u.doorbell.payload = dbval;

	ret = write(fd, &evt, sizeof(evt));
	if (ret < 0)
		printf("Write DB event failed, err=%d\n", errno);
	else
		ret = 0;

	return ret;
}

static void display_help(char *program)
{
	printf("%s - test RapidIO DoorBell exchange\n",	program);
	printf("Usage:\n");
	printf("  %s [options]\n", program);
	printf("options are:\n");
	printf("Common:\n");
	printf("  -D xxxx\n");
	printf("  --destid xxxx\n");
	printf("    destination ID of sending/receiving RapidIO device\n");
	printf("    (defaults Rx:any, Tx:none)\n");
	printf("  --help (or -h)\n");
	/*printf("  --debug (or -d)\n");*/
	printf("Sender:\n");
	printf("  -M mport_id\n");
	printf("  --mport mport_id\n");
	printf("    local mport device index (default=0)\n");
	printf("  -I xxxx\n");
	printf("    DoorBell Info field value (default 0x5a5a)\n");
	printf("Receiver:\n");
	printf("  -r run in DB recever mode\n");
	printf("  -n run recever in non-blocking mode\n");
	printf("  -S xxxx\n");
	printf("    start of doorbell range (default 0x5a5a)\n");
	printf("  -E xxxx\n");
	printf("    end of doorbell range (default 0x5a5a)\n");
	printf("\n");
}

int main(int argc, char **argv)
{
	uint32_t mport_id = 0;
	uint32_t rio_destid = 0xffffffff;
	uint32_t db_info = 0x5a5a;
	uint32_t db_start = 0x5a5a;
	uint32_t db_end = 0x5a5a;
	int fd;
	int flags = 0;
	int option;
	int do_dbrecv = 0;
	static const struct option options[] = {
		{ "destid", required_argument, NULL, 'D' },
		{ "mport",  required_argument, NULL, 'M' },
		{ "debug",  no_argument, NULL, 'd' },
		{ "help",   no_argument, NULL, 'h' },
		{ }
	};
	char *program = argv[0];
	struct rio_mport_properties prop;
	int rc = EXIT_SUCCESS;

	while (1) {
		option = getopt_long_only(argc, argv,
				"rdhnD:I:M:S:E:", options, NULL);
		if (option == -1)
			break;
		switch (option) {
		case 'D':
			rio_destid = strtol(optarg, NULL, 0);
			break;
		case 'r':
			do_dbrecv = 1;
			break;
		case 'n':
			flags = O_NONBLOCK;
			break;
		case 'I':
			db_info = strtol(optarg, NULL, 0);
			break;
		case 'M':
			mport_id = strtol(optarg, NULL, 0);
			break;
		case 'S':
			db_start = strtol(optarg, NULL, 0);
			break;
		case 'E':
			db_end = strtol(optarg, NULL, 0);
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

	if (!do_dbrecv && rio_destid == 0xffffffff) {
		printf("\tPlease specify RIO Target DestID to Send a DB\n");
		exit(EXIT_SUCCESS);
	}

	fd = rio_mport_open(mport_id, flags);
	if (fd < 0) {
		printf("DB Test: unable to open mport%d device err=%d\n",
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

	/* Trap signals that we expect to receive */
	signal(SIGINT, db_sig_handler);
	signal(SIGTERM, db_sig_handler);
	signal(SIGUSR1, db_sig_handler);

	rio_set_event_mask(fd, RIO_DOORBELL);

	if (do_dbrecv) {
		printf("+++ RapidIO Doorbell Receive Mode +++\n");
		printf("\tmport%d PID:%d\n", mport_id, (int)getpid());
		printf("\tfilter: destid=%x start=%x end=%x\n",
			rio_destid, db_start, db_end);

		do_dbrcv_test(fd, rio_destid, db_start, db_end);
	} else {
		printf("+++ RapidIO Doorbell Send +++\n");
		printf("\tmport%d destID=%d db_info=0x%x\n",
			mport_id, rio_destid, db_info);

		do_dbsnd_test(fd, rio_destid, db_info);
	}

out:
	close(fd);
	exit(rc);
}

#ifdef __cplusplus
}
#endif
