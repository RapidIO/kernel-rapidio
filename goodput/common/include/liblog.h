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

#ifndef __LIBLOG_H__
#define __LIBLOG_H__

#include <stdio.h>
#include "rrmap_config.h"

#define NUM_LOG_LINES	100
#define LOG_LINE_SIZE	512

/* Log levels */
#define RDMA_LL_OFF	 1
#define RDMA_LL_CRIT	 2
#define RDMA_LL_ERR	 3
#define RDMA_LL_WARN	 4
#define RDMA_LL_HIGH	 5
#define RDMA_LL_INFO	 6
#define RDMA_LL_DBG	 7

/* If 'level' is not specified in the build generate an error */
#ifndef RDMA_LL
#error RDMA_LL not defined. Please specify in Makefile
#endif

#if RDMA_LL >= RDMA_LL_DBG
#define DBG(format, ...) if (RDMA_LL_DBG <= g_level) { \
		RDMA_LOG_FUNC(RDMA_LL_DBG, "DBG", format, ## __VA_ARGS__); \
}
#else
#define DBG(format, ...) if (0) fprintf(stderr, format, ## __VA_ARGS__)
#endif

#if RDMA_LL >= RDMA_LL_INFO
#define INFO(format, ...) if (RDMA_LL_INFO <= g_level) { \
		RDMA_LOG_FUNC(RDMA_LL_INFO, "INFO", format, ## __VA_ARGS__); \
}
#else
#define INFO(format, ...) if (0) fprintf(stderr, format, ## __VA_ARGS__)
#endif

#if RDMA_LL >= RDMA_LL_HIGH
#define HIGH(format, ...) if (RDMA_LL_HIGH <= g_level) { \
		RDMA_LOG_FUNC(RDMA_LL_HIGH, "HIGH", format, ## __VA_ARGS__); \
}
#else
#define HIGH(format, ...) if (0) fprintf(stderr, format, ## __VA_ARGS__)
#endif

#if RDMA_LL >= RDMA_LL_WARN
#define WARN(format, ...) if (RDMA_LL_WARN <= g_level) { \
		RDMA_LOG_FUNC(RDMA_LL_WARN, "WARN", format, ## __VA_ARGS__); \
}
#else
#define WARN(format, ...) if (0) fprintf(stderr, format, ## __VA_ARGS__)
#endif

#if RDMA_LL >= RDMA_LL_ERR
#define ERR(format, ...) if (RDMA_LL_ERR <= g_level) { \
		RDMA_LOG_FUNC(RDMA_LL_ERR, "ERR", format, ## __VA_ARGS__); \
}
#else
#define ERR(format, ...) if (0) fprintf(stderr, format, ## __VA_ARGS__)
#endif

#if RDMA_LL >= RDMA_LL_CRITICAL
#define CRIT(format, ...) if (RDMA_LL_CRIT <= g_level) { \
		RDMA_LOG_FUNC(RDMA_LL_CRIT, "CRIT", format, ## __VA_ARGS__); \
}
#else
#define CRIT(format, ...) if (0) fprintf(stderr, format, ## __VA_ARGS__)
#endif

#ifdef __cplusplus
extern "C" {
#endif

int rdma_log_init(const char *log_filename, unsigned circ_buf_en);

int rdma_log(unsigned level, const char *level_str, const char *file,
		int line_num, const char *func, const char *format, ...);

void rdma_log_dump();

void rdma_log_close();

int liblog_bind_cli_cmds();

extern unsigned g_level;
extern unsigned g_disp_level;

#define LOG_FMT_DEFAULT 1
#define LOG_FMT_ONE_LINE 1

extern unsigned log_fmt_sel; /* If non-zero, logs are formatted as one line */

#ifdef __cplusplus
}
#endif

#define RDMA_LOG_FUNC(level, level_str, format, ...) rdma_log(level, level_str, \
			 __FILE__, __LINE__, __func__, format, ## __VA_ARGS__)

#endif /* __LIBLOG_H__ */
