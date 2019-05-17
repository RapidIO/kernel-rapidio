/*
 ****************************************************************************
 Copyright (c) 2015, Integrated Device Technology Inc.
 Copyright (c) 2015, RapidIO Trade Association
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 3. Neither the name of the copyright holder nor the names of its contributors
 may be used to endorse or promote products derived from this software without
 specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "tok_parse.h"
#include "string_util.h"
#include "rio_misc.h"
#include "libcli.h"
#include "rrmap_config.h"

#ifdef __cplusplus
extern "C" {
#endif

struct connect_rx_parms {
	struct cli_env env;
	int sockfd;
	pthread_t conn_rx_thr;
	char conn_rx_name[16];
};

void *conn_rx(void *parms)
{
	struct connect_rx_parms *p = (struct connect_rx_parms *)parms;
	char rx_buffer[1024];
	int n;

	pthread_setname_np(p->conn_rx_thr, p->conn_rx_name);
	pthread_detach(p->conn_rx_thr);

	memset(rx_buffer, 0, sizeof(rx_buffer));
	n = read(p->sockfd, rx_buffer, sizeof(rx_buffer));
	while (n > 0) {
		LOGMSG((&(p->env)), "%s", rx_buffer);
		memset(rx_buffer, 0, sizeof(rx_buffer));
		n = read(p->sockfd, rx_buffer, sizeof(rx_buffer));
	}

	close(p->sockfd);
	free(p);
	pthread_exit(NULL);
}

int CLIConnectCmd(struct cli_env *env, int argc, char **argv)
{
	int sockfd, n;
	uint16_t sock_num;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char tx_buffer[1024];
	int one = 1;
	uint bytes_sent;
	int rc;
	struct connect_rx_parms *p = NULL;

	// Avoid compiler warning for unused parameter.
	if (0) {
		argv[0][0] = argc;
	}

	server = gethostbyname(argv[0]);
	if (server == NULL) {
		LOGMSG(env, "ERROR, host \"%s\" does not exit.\n", argv[0]);
		goto exit;
	}

	if (tok_parse_socket(argv[1], &sock_num, 0)) {
		LOGMSG(env, TOK_ERR_SOCKET_MSG_FMT, "Socket number");
		goto exit;
	}

	LOGMSG(env, "\nAttempting connection to host \"%s\" socket %d.\n",
			argv[0], sock_num);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		LOGMSG(env, "ERROR opening socket\n");
		goto exit;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memmove((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr,
			server->h_length);

	serv_addr.sin_port = htons(sock_num);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))
			< 0) {
		LOGMSG(env, "ERROR connecting\n");
		goto cleanup;
	}
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

	p = (struct connect_rx_parms *)malloc(sizeof(struct connect_rx_parms));
	if (NULL == p) {
		LOGMSG(env,
				"Could not allocate memory for connection parameters\n");
		goto cleanup;
	}

	memcpy(&p->env, env, sizeof(p->env));
	p->sockfd = sockfd;
	snprintf(p->conn_rx_name, sizeof(p->conn_rx_name), "%s_%s", argv[0],
			argv[1]);

	rc = pthread_create(&p->conn_rx_thr, NULL, conn_rx, (void *)p);
	if (rc) {
		LOGMSG(env, "\nCould not create conn_rx, code was %d %d %s\n",
				rc, errno, strerror(errno));
		goto cleanup;
	}

	// Since the thread started, freeing p is now the responsibility
	// of the thread.  Change p to null to prevent a double free.
	p = NULL;
	memset((char *)&tx_buffer, 0, sizeof(tx_buffer));
	while (1) {
		memset((char *)&tx_buffer, 0, sizeof(tx_buffer));
		if (NULL == fgets(tx_buffer, sizeof(tx_buffer) - 1, stdin)) {
			LOGMSG(env, "\nEnd of file\n");
			goto cleanup;
		}

		// Allow "done" or "quit" to kill the remote login session.
		//
		// NOTE: This does not allow a user to "quit" and application
		// using a remote connection.
		if (!strncmp(tx_buffer, "done", 4)
				|| !strncmp(tx_buffer, "quit", 4)) {
			goto cleanup;
		}

		n = write(sockfd, tx_buffer, strlen(tx_buffer));
		bytes_sent = n;
		if (n < 0) {
			LOGMSG(env, "\nError transmitting\n");
			goto cleanup;
		}

		while ((n >= 0) && (bytes_sent < strlen(tx_buffer))) {
			n = write(sockfd, &tx_buffer[bytes_sent],
					strlen(&tx_buffer[bytes_sent]));
			if (n < 0) {
				goto cleanup;
			}
			bytes_sent += n;
		}
	}

cleanup:
	LOGMSG(env, "\nClosing session\n");
	shutdown(sockfd, SHUT_WR);
	close(sockfd);
	free(p);

exit:
	return 0;
}

struct cli_cmd CLIConnect =
		{"connect", 3, 2,
				"Connect to an RRMAP processes command line interpreter",
				"connect <addr> <skt>\n"
						"<addr>: host address, either an IP address or translatable node name\n"
						"<skt> : socket number to connect to.  Default numbers are:\n"
						"        " STR(FMD_DFLT_CLI_SKT) " : Fabric Management Daemon (FMD)\n"
				"        " STR(RDMA_DFLT_CLI_SKT) " : Remote Direct Memory Access Daemon (RDMAD)\n"
				"        " STR(RSKT_DFLT_CLI_SKT) " : RDMA Socket Daemon (RSKT)\n"
				"        " STR(FXFR_DFLT_CLI_SKT) " : File Transfer Server (FXFR)\n"
				"        " STR(RSKTS_DFLT_CLI_SKT) " : RSKT Test server (RSKTS)\n",
				CLIConnectCmd,
				ATTR_NONE};

#ifdef __cplusplus
}
#endif

