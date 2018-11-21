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
 * riodp_test_client.c
 * Client part of client/server test program for RapidIO channelized messaging.
 *
 * Sends a message to target RapidIO device, receives an echo response from it
 * and verifies response.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <rio_mport_lib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct args {
	uint32_t mport_id;
	uint16_t remote_destid;
	uint16_t remote_channel;
	uint32_t repeat;
};

static void usage(char *program)
{
	printf("%s - API library test client\n", program);
	printf("Usage:\n");
	printf("  %s [options]\n", program);
	printf("Options are:\n");
	printf("    <mport> local mport device index\n");
	printf("    <destid> target RapidIO device destination ID\n");
	printf("    <channel> channel number on remote RapidIO device\n");
	printf("    <repetitions> number of repetitions (default 1)\n");
	printf("\n");
}

/*
 * Display available devices
 */
void show_rio_devs(void)
{
	uint32_t *mport_list = NULL;
	uint32_t *ep_list = NULL;
	uint32_t *list_ptr;
	uint32_t number_of_eps = 0;
	uint8_t number_of_mports = RIO_MAX_MPORTS;
	uint32_t ep;
	int i;
	int mport_id;
	int ret = 0;

	ret = rio_mport_get_mport_list(&mport_list, &number_of_mports);
	if (ret) {
		printf("ERR: rio_mport_get_mport_list() ERR %d\n", ret);
		return;
	}

	printf("\nAvailable %d local mport(s):\n", number_of_mports);
	if (number_of_mports > RIO_MAX_MPORTS) {
		printf("WARNING: Only %d out of %d have been retrieved\n",
		RIO_MAX_MPORTS, number_of_mports);
	}

	/* for each local mport display list of remote RapidIO devices */
	list_ptr = mport_list;
	for (i = 0; i < number_of_mports; i++, list_ptr++) {
		mport_id = *list_ptr >> 16;
		printf("+++ mport_id: %u dest_id: %u\n", mport_id,
			*list_ptr & 0xffff);

		/* Display EPs for this MPORT */

		ret = rio_mport_get_ep_list(mport_id, &ep_list,	&number_of_eps);
		if (ret) {
			printf("ERR: rio_mport_get_ep_list() ERR %d\n", ret);
			break;
		}

		printf("\t%u Endpoints (dest_ID): ", number_of_eps);
		for (ep = 0; ep < number_of_eps; ep++)
			printf("%u ", *(ep_list + ep));
		printf("\n");

		ret = rio_mport_free_ep_list(&ep_list);
		if (ret)
			printf("ERR: rio_mport_free_ep_list() ERR %d\n", ret);
	}

	printf("\n");

	ret = rio_mport_free_mport_list(&mport_list);
	if (ret)
		printf("ERR: rio_mport_free_mport_list() ERR %d\n", ret);
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

/*
 *  Starting point for the program
 *
 * When running without expected number of arguments displays list of available local
 * and remote devices.
 */
int main(int argc, char **argv)
{
	int ret = 0;
	uint32_t ep = 0;
	uint32_t number_of_eps = 0;
	uint32_t *ep_list = NULL;
	int ep_found = 0;
	void *msg_rx = NULL;
	void *msg_tx = NULL;
	struct args arg;
	rio_mailbox_t mailbox;
	rio_socket_t socket;
	uint32_t i;
	struct timespec starttime;
	struct timespec endtime;
	struct timespec time;
	float totaltime;
	float mean;
	int tmp;

	/* Parse console arguments */
	if (argc < 5) {
		usage(argv[0]);
		show_rio_devs();
		exit(EXIT_FAILURE);
	}
	arg.mport_id       = strtoul(argv[1], NULL, 10);
	arg.remote_destid  = strtoul(argv[2], NULL, 10);
	arg.remote_channel = strtoul(argv[3], NULL, 10);
	arg.repeat         = strtoul(argv[4], NULL, 10);

	printf("Start CM_CLIENT (PID %d)\n", (int)getpid());

	/* Verify existence of remote RapidIO Endpoint */
	ret = rio_mport_get_ep_list(arg.mport_id, &ep_list, &number_of_eps);
	if (ret) {
		printf("rio_mport_get_ep_list error: %d\n", ret);
		exit(1);
	}

	for (ep = 0; ep < number_of_eps; ep++)
		if (ep_list[ep] == arg.remote_destid)
			ep_found = 1;

	ret = rio_mport_free_ep_list(&ep_list);
	if (ret) {
		printf("ERROR: rio_mport_free_ep_list error: %d\n", ret);
		exit(1);
	}

	if (!ep_found) {
		printf("CM_CLIENT(%d) invalid remote destID %d\n",
				(int)getpid(), arg.remote_destid);
		exit(1);
	}

	/* Create rapidio_mport_mailbox control structure */
	ret = rio_mbox_create_handle(arg.mport_id, 0, &mailbox);
	if (ret) {
		printf("rio_mbox_create_handle error: %d\n", ret);
		exit(1);
	}

	/* Create a socket structure associated with given mailbox */
	ret = rio_socket_socket(mailbox, &socket);
	if (ret) {
		printf("rio_socket_socket error: %d\n", ret);
		rio_mbox_destroy_handle(&mailbox);
		exit(1);
	}

	ret = rio_socket_connect(socket, arg.remote_destid, arg.remote_channel,
				NULL);

	if (ret) {
		if (ret == EADDRINUSE)
			printf("Requested channel was in use, reusing...\n");
		else {
			printf("rio_socket_connect error: %d\n", ret);
			goto out;
		}
	}

	ret = rio_socket_request_send_buffer(socket, &msg_tx);
	if (ret) {
		printf("rio_socket_request_send_buffer error: %d\n", ret);
		goto out;
	}

	msg_rx = malloc(RIO_SOCKET_MSG_SIZE);
	if (msg_rx == NULL) {
		printf("CM_CLIENT(%d): error allocating rx buffer\n",
				(int)getpid());
		rio_socket_release_send_buffer(socket, msg_tx);
		goto out;
	}

	clock_gettime(CLOCK_MONOTONIC, &starttime);

	for (i = 1; i <= arg.repeat; i++) {
		/** - Place message into buffer */
		sprintf((char *)((char *)msg_tx + 20),
			"%d:%d\n", i, (int)getpid());

		/** - Send message to the destination */
		ret = rio_socket_send(socket, msg_tx, RIO_SOCKET_MSG_SIZE);
		if (ret) {
			printf("CM_CLIENT(%d): rio_socket_send() ERR %d\n",
				(int)getpid(), ret);
			break;
		}

		/* Get echo response from the server (blocking call, no TO) */
		ret = rio_socket_receive(socket, &msg_rx, 0, NULL);
		if (ret) {
			printf("CM_CLIENT(%d): riomp_sock_receive() ERR %d on roundtrip %d\n",
				(int)getpid(), ret, i);
			break;
		}

		if (strcmp((char *)msg_tx + 20, (char *)msg_rx + 20)) {
			printf("CM_CLIENT(%d): MSG TRANSFER ERROR: data corruption detected @ %d\n",
				(int)getpid(), i);
			printf("CM_CLIENT(%d): MSG OUT: %s\n",
				(int)getpid(), (char *)msg_tx + 20);
			printf("CM_CLIENT(%d): MSG IN: %s\n",
				(int)getpid(), (char *)msg_rx + 20);
			rio_socket_release_receive_buffer(socket, msg_rx);
			ret = -1;
			break;
		}
	}

	/* free the rx and tx buffers */
	rio_socket_release_receive_buffer(socket, msg_rx);
	rio_socket_release_send_buffer(socket, msg_tx);

	clock_gettime(CLOCK_MONOTONIC, &endtime);

	if (ret)
		printf("CM_CLIENT(%d) ERROR.\n", (int)getpid());
	else
		printf("CM_CLIENT(%d) Test finished.\n", (int)getpid());

	/* getchar(); */
	time = timediff(starttime, endtime);
	totaltime = ((double)time.tv_sec + (time.tv_nsec / 1000000000.0));
	mean = totaltime / arg.repeat * 1000.0; /* mean in us */

	printf("Total time:\t\t\t\t%4f s\n"
		"Mean time per message roundtrip:\t%4f us\n"
		"Data throughput:\t\t\t%4f MB/s\n", totaltime, mean,
		((4096 * i) / totaltime) / (1024 * 1024));

out:
	/* Close messaging channel */
	tmp = rio_socket_close(&socket);
	if (tmp)
		printf("rio_socket_close error: %d\n", tmp);

	/* free the mailbox */
	tmp = rio_mbox_destroy_handle(&mailbox);
	if (tmp)
		printf("rio_mbox_destroy_handle error: %d\n", tmp);
	return ret;
}

#ifdef __cplusplus
}
#endif
