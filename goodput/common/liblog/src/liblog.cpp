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

/* System includes */
#include <sys/time.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <errno.h>
#include <semaphore.h>

/* C++ standard library */
#include <string>

#include <cstdio>
#include <cstring>
#include <cstdarg>

#include "circ_buf.h"
#include "liblog.h"

using std::string;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RDMA_LL
extern unsigned RDMA_LL;
#endif

unsigned g_level = RDMA_LL; /* Default log level from build */
unsigned g_disp_level = RDMA_LL_CRIT; /* Default log level from build */

static circ_buf<string, NUM_LOG_LINES> log_buf;
static unsigned circ_buf_en = 0;
static sem_t log_buf_sem;

static FILE* log_file = NULL;

int rdma_log_init(const char *log_filename, unsigned circ_buf_en)
{
	/* Semaphore for protecting access to log_buf */
	if (sem_init(&log_buf_sem, 0, 1) == -1) {
		perror("rdma_log_init: sem_init()");
		return -1;
	}

	::circ_buf_en = circ_buf_en;

	if (circ_buf_en && (NULL == log_filename)) {
		log_file = NULL;
		return 0;
	}

	/* Directory name */
	string filename(DEFAULT_LOG_DIR);

	/* Create log directory if not already present on system */
	struct stat st;
	if (stat(DEFAULT_LOG_DIR, &st) < 0) {
		string create_dir(filename);
		create_dir.insert(0, "mkdir ");
		if (system(create_dir.c_str()) < 0) {
			fprintf(stderr, "Failed to create '%s'\n",
					filename.c_str());
			return -ENOENT;
		}
	}

	/* Open log file */
	filename.append(log_filename);
	log_file = fopen(filename.c_str(), "ae");
	if (!log_file) {
		perror("rdma_log_init: fopen()");
		return -ENOENT;
	}

	return 0;
} /* rdma_log_init() */

void rdma_log_close()
{
	if (log_file) {
		fclose(log_file);
		log_file = NULL;
	} else {
		puts("rdma_log_close(): log_file is NULL");
	}
} /* rdma_log_close() */

void rdma_log_dump()
{
	log_buf.dump();
} /* rdma_log_dump() */

int rdma_log(unsigned level, const char *level_str, const char *file,
		int line_num, const char *func, const char *format, ...)
{
	char buffer[LOG_LINE_SIZE] = {0};
	va_list args;
	int n;
	int p;
	time_t cur_time;
	struct timeval tv;
	char asc_time[26] = {0};

	char *oneline_fmt = (char *)"%4s %s.%06ldus tid=%ld %s:%4d %s(): ";

	/* Prefix with level_str, timestamp, filename, line no., and func */
	time(&cur_time);
	ctime_r(&cur_time, asc_time);
	asc_time[strlen(asc_time) - 1] = '\0';
	gettimeofday(&tv, NULL);
	n = snprintf(buffer, sizeof(buffer), (const char *)(oneline_fmt),
			level_str, asc_time, tv.tv_usec, syscall(SYS_gettid),
			file, line_num, func);
	buffer[sizeof(buffer) - 1] = '\0';

	/* Handle format and variable arguments */
	va_start(args, format);
	p = vsnprintf(buffer + n, sizeof(buffer) - n, format, args);
	va_end(args);

	/* Push log line into circular log buffer and log file */
	string log_line(buffer);
	sem_wait(&log_buf_sem);
	if (circ_buf_en) {
		log_buf.push_back(log_line);
	}
	if (log_file) {
		fputs(log_line.c_str(), log_file);
		fflush(log_file);
	}
	if (level <= g_disp_level) {
		fflush(stderr);
		fflush(stdout);
		fprintf(stdout, "%s", log_line.c_str());
		if ('\n' != buffer[n + p - 1]) {
			fprintf(stdout, "\n");
		}
		fflush(stdout);
	}
	sem_post(&log_buf_sem);

	/* Return 0 if there is no error */
	return (n < 0) ? n : 0;
} /* rdma_log() */

#ifdef __cplusplus
}
#endif

