/*
 ****************************************************************************
 Copyright (c) 2014, Integrated Device Technology Inc.
 Copyright (c) 2014, RapidIO Trade Association
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

#ifndef __LIBTIME_UTILS_H__
#define __LIBTIME_UTILS_H__

#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TIMESTAMPS 4096

struct seq_ts {
	int max_idx;
	int ts_idx;
	struct timespec ts_val[MAX_TIMESTAMPS];
	int ts_mkr[MAX_TIMESTAMPS];
};

int init_seq_ts(struct seq_ts *ts, int max_ts);
void ts_now(struct seq_ts *ts);
void ts_now_mark(struct seq_ts *ts, int marker);

struct timespec time_difference(struct timespec start, struct timespec end);

struct timespec time_add(struct timespec start, struct timespec end);

struct timespec time_div(struct timespec time, uint32_t divisor);

void time_track(int i, struct timespec starttime, struct timespec enddtime,
		struct timespec *totaltime, struct timespec *mintime,
		struct timespec *maxtime);

void time_track_lim(int i, const struct timespec *limit,
		struct timespec *starttime, struct timespec *enddtime,
		struct timespec *totaltime, struct timespec *mintime,
		struct timespec *maxtime);

void time_sleep(const struct timespec *delay);

#ifdef __cplusplus
}
#endif

#endif /* __LIBTIME_UTILS_H__ */
