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

#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#include "libtime_utils.h"

#ifdef UNIT_TESTING
#include <stdarg.h>
#include <setjmp.h>
#include "cmocka.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes timestamp sequence index and array
 *
 * @param[in] ts Time stamp tracking structure
 * @return 0 for success, 1 for failure
 * @retval 1 max_ts was > MAX_TIMESTAMPS, or max_ts was < 0
 */
int init_seq_ts(struct seq_ts *ts, int max_ts)
{
	if ((NULL == ts) || (max_ts > MAX_TIMESTAMPS) || (max_ts < 0)) {
		return 1;
	}

	ts->max_idx = max_ts;
	ts->ts_idx = 0;
	memset(&ts->ts_val, 0, sizeof(ts->ts_val));
	memset(&ts->ts_mkr, 0, sizeof(ts->ts_mkr));

	return 0;
}

/**
 * @brief If a free entry exists, get another timestamp.
 *
 * @param[in] ts Time stamp tracking structure
 * @retval None
 */

void ts_now(struct seq_ts *ts)
{
	if (NULL == ts) {
		return;
	}

	if (ts->ts_idx < ts->max_idx) {
		clock_gettime(CLOCK_MONOTONIC, &ts->ts_val[ts->ts_idx++]);
	}
}

/**
 * @brief If a free entry exists, get another timestamp and set marker value
 *
 * @param[in] ts Time stamp tracking structure
 * @param[in] marker Time stamp entry tracking structure
 * @return None
 */

void ts_now_mark(struct seq_ts *ts, int marker)
{
	if (NULL == ts) {
		return;
	}

	if (ts->ts_idx < ts->max_idx) {
		ts->ts_mkr[ts->ts_idx] = marker;
		clock_gettime(CLOCK_MONOTONIC, &ts->ts_val[ts->ts_idx++]);
	}
}

/**
 * @brief Given two timespec structs, subtract them and return a timespec 
 * containing the difference
 *
 * @param[in] start start time
 * @param[in] end end time
 * @return timespec difference between start and end.
 */
struct timespec time_difference(struct timespec start, struct timespec end)
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

/**
 * @brief Given two timespec structs, add them together and return 
 * a timespec containing the sum.
 *
 * @param[in] start start time
 * @param[in] end end time
 * @return timespec sum of start and end timespecs.
 */
struct timespec time_add(struct timespec start, struct timespec end)
{
	struct timespec temp;
	temp.tv_nsec = start.tv_nsec + end.tv_nsec;
	temp.tv_sec = start.tv_sec + end.tv_sec;
	if (temp.tv_nsec >= 1000000000) {
		temp.tv_sec++;
		temp.tv_nsec -= 1000000000;
	}
	return temp;
}

/**
 * @brief Divides the time in a timespec by a divisor, and returns 
 * a timespec containing the quotient.
 *
 * @param[in] time total time
 * @param[in] divisor factor to divide by
 * @return timespec time divided by divisor
 */
struct timespec time_div(struct timespec time, uint32_t divisor)
{
	uint64_t time_nsec;
	struct timespec temp;
	uint64_t one_bln = 1000000000;

	time_nsec = (uint64_t)time.tv_nsec + (uint64_t)(time.tv_sec) * one_bln;
	time_nsec = time_nsec / divisor;

	temp.tv_sec = time_nsec / one_bln;
	temp.tv_nsec = time_nsec % one_bln;
	return temp;
}

/**
 * @brief Given a start time and end time, track minimum, maximum and total time
 *        Limit the maximum time difference to lim.
 *
 * @param[in] i 0 - initialize totaltime/mintime/maxtime, 1 - don't init
 * @param[in] limit - Maximum legal time difference expected. 
 * @param[in] starttime - starting time for time interval to track
 * @param[in] endtime - end time for time interval to track
 * @param[inout] totaltime - on return, totaltime += endtime-starttime
 * @param[inout] mintime - on return, updated if endtime-starttime < mintime
 * @param[inout] maxtime - on return, updated if endtime-starttime > maxtime
 *
 */
const struct timespec no_lim = {0xFFFFFFFF, 0xFFFFFFFF};

void time_track_lim(int i, const struct timespec *lim,
		struct timespec *starttime, struct timespec *endtime,
		struct timespec *totaltime, struct timespec *mintime,
		struct timespec *maxtime)
{
	struct timespec dta = time_difference(*starttime, *endtime);

	if (!i) {
		totaltime->tv_nsec = totaltime->tv_sec = 0;
		mintime->tv_nsec = mintime->tv_sec = 0xFFFFFFFF;
		maxtime->tv_nsec = maxtime->tv_sec = 0;
	}

	if ((dta.tv_sec < 0) || (dta.tv_nsec < 0)) {
		return;
	}

	if ((dta.tv_sec > lim->tv_sec)
			|| ((dta.tv_sec == lim->tv_sec)
					&& (dta.tv_nsec > lim->tv_nsec))) {
		return;
	}

	if (i) {
		*totaltime = time_add(*totaltime, dta);

		if ((mintime->tv_sec > dta.tv_sec)
				|| ((mintime->tv_sec == dta.tv_sec)
						&& (mintime->tv_nsec
								> dta.tv_nsec))) {
			*mintime = dta;
		}

		if ((maxtime->tv_sec < dta.tv_sec)
				|| ((maxtime->tv_sec == dta.tv_sec)
						&& (maxtime->tv_nsec
								< dta.tv_nsec))) {
			*maxtime = dta;
		}
	} else {
		*totaltime = *mintime = *maxtime = dta;
	}
}

/**
 * @brief Given a start time and end time, track minimum, maximum and total time
 *        Do not limit the maximum time difference tracked.
 *
 * @param[in] i  0 - initialize totaltime/mintime/maxtime, 1 - don't init
 * @param[in] starttime - starting time for time interval to track
 * @param[in] endtime - end time for time interval to track
 * @param[inout] totaltime - on return, totaltime += endtime-starttime
 * @param[inout] mintime - on return, updated if endtime-starttime < mintime
 * @param[inout] maxtime - on return, updated if endtime-starttime > maxtime
 *
 */
void time_track(int i, struct timespec starttime, struct timespec endtime,
		struct timespec *totaltime, struct timespec *mintime,
		struct timespec *maxtime)
{
	time_track_lim(i, &no_lim, &starttime, &endtime, totaltime, mintime,
			maxtime);
}

void time_sleep(const struct timespec *delay)
{
	struct timespec dly;
	struct timespec rem;
	int rc;

	dly = *delay;
	do {
		errno = 0;
		rc = nanosleep(&dly, &rem);
		dly = rem;
	} while (rc && (errno == EINTR));
}
#ifdef __cplusplus
}
#endif
