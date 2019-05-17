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
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include "libcli.h"
#include "string_util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct rem_sess_parms {
	pthread_t rem_sess_thr;
	char thr_name[16];
	int sess_num;
	struct cli_env *e;
};

void *remote_session(void *parms)
{
	struct rem_sess_parms *p;

	p = (struct rem_sess_parms *)parms;

	pthread_setname_np(p->rem_sess_thr, p->thr_name);
	pthread_detach(p->rem_sess_thr);

	splashScreen(p->e, p->thr_name);
	cli_terminal(p->e);

	close(p->e->sess_socket);
	free(p->e);
	free(parms);
	pthread_exit(NULL);
}

pthread_t remote_login_thread;

void *remote_login(void *remote_login_parm)
{
	int *status;
	int sockfd;
	int portno;
	socklen_t clilen;
	struct sockaddr_in s_addr, cli_addr;
	int one = 1;
	int session_num = 0;
	char my_name[16] = {0};
	struct remote_login_parms *parms =
			(struct remote_login_parms *)remote_login_parm;
	struct cli_env *env = NULL;
	struct rem_sess_parms *sess = NULL;

	snprintf(my_name, 15, "%s", parms->thr_name);
	pthread_setname_np(remote_login_thread, my_name);
	pthread_detach(remote_login_thread);

	portno = parms->portno;
	status = parms->status;
	free(remote_login_parm);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		goto fail;
	}

	memset((char *)&s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_addr.sin_port = htons(portno);
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

	if (bind(sockfd, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0) {
		goto fail;
	}

	if (NULL != status) {
		*status = 1;
	}

	while (1) {
		int rc;

		env = (struct cli_env *)malloc(sizeof(struct cli_env));
		if (NULL == env) {
			break;
		}

		env->sess_socket = 0;

		//@sonar:off - c:S3584 Allocated memory not released
		sess = (struct rem_sess_parms *)malloc(
				sizeof(struct rem_sess_parms));

		if (NULL == sess) {
			break;
		}
		//@sonar:on

		init_cli_env(env);
		snprintf(sess->thr_name, sizeof(sess->thr_name), "%s%d",
				my_name, session_num);
		snprintf(env->prompt, sizeof(env->prompt), "%s> ",
				sess->thr_name);
		sess->e = env;
		sess->sess_num = session_num;

		listen(sockfd, 5);
		clilen = sizeof(cli_addr);
		sess->e->sess_socket = accept(sockfd,
				(struct sockaddr *)&cli_addr, &clilen);
		if (sess->e->sess_socket < 0) {
			goto fail;
		}

		rc = pthread_create(&sess->rem_sess_thr, NULL, remote_session,
				(void *)sess);
		if (rc) {
			break;
		}
		session_num++;
	}

fail:
	if (NULL != status) {
		*status = 0;
	}

	if ((NULL != env) && (env->sess_socket > 0)) {
		close(env->sess_socket);
	}

	free(env);
	free(sess);
	close(sockfd);
	pthread_exit(NULL);
	return NULL;
}

#ifdef __cplusplus
}
#endif

