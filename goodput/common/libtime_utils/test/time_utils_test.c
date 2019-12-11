/*
 * ****************************************************************************
 * Copyright (c) 2014, Integrated Device Technology Inc.
 * Copyright (c) 2014, RapidIO Trade Association
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * *************************************************************************
 * */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <stdarg.h>
#include <setjmp.h>
#include "cmocka.h"

#include "libtime_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RETURNED_SUCCESS 0
#define RETURNED_FAILURE 1

// initialize the seq_ts structure with a known pattern
static void set_seq_ts(struct seq_ts *ts)
{
	ts->max_idx = 0xca;
	ts->ts_idx = 0xfe;
	memset(ts->ts_mkr, 0xdead, sizeof ts->ts_mkr);
	memset(ts->ts_val, 0xbeef, sizeof ts->ts_mkr);
}

/**
 * @brief Verify the internal structures and defined constants
 */
static void assumptions(void **state)
{
	assert_int_equal(4096, MAX_TIMESTAMPS);

	// although it appears a little bogus, duplicate the internal
	// structures for testing. Ensure they remain the same and other
	// assumptions in subsequent tests are valid
	struct dup_seq_ts {
		int max_idx;
		int ts_idx;
		struct timespec ts_val[MAX_TIMESTAMPS];
		int ts_mkr[MAX_TIMESTAMPS];
	};
	assert_int_equal(sizeof(struct dup_seq_ts), sizeof(struct seq_ts));

	(void)state; // not used
}

/** @brief Test handling of null parameters for init_seq_ts
 */
static void init_seq_ts_null_parm_test(void **state)
{

	// null ts
	assert_int_equal(RETURNED_FAILURE, init_seq_ts(NULL, 1));

	(void)state; // not used
}

/** @brief Test correct operation of init_seq_ts
 */
static void init_seq_ts_test(void **state)
{
	struct seq_ts ts;

	// min-1 ts
	set_seq_ts(&ts);
	assert_int_equal(0xca, ts.max_idx);
	assert_int_equal(0xfe, ts.ts_idx);
	assert_int_equal(RETURNED_FAILURE, init_seq_ts(&ts, -1));
	assert_int_equal(0xca, ts.max_idx);
	assert_int_equal(0xfe, ts.ts_idx);

	// min ts
	set_seq_ts(&ts);
	assert_int_equal(0xca, ts.max_idx);
	assert_int_equal(0xfe, ts.ts_idx);
	assert_int_equal(RETURNED_SUCCESS, init_seq_ts(&ts, 0));
	assert_int_equal(0, ts.max_idx);
	assert_int_equal(0, ts.ts_idx);

	// min ts+1
	set_seq_ts(&ts);
	assert_int_equal(0xca, ts.max_idx);
	assert_int_equal(0xfe, ts.ts_idx);
	assert_int_equal(RETURNED_SUCCESS, init_seq_ts(&ts, 1));
	assert_int_equal(1, ts.max_idx);
	assert_int_equal(0, ts.ts_idx);

	// mid range
	set_seq_ts(&ts);
	assert_int_equal(RETURNED_SUCCESS, init_seq_ts(&ts, MAX_TIMESTAMPS/2));
	assert_int_equal(MAX_TIMESTAMPS/2, ts.max_idx);
	assert_int_equal(0, ts.ts_idx);

	// max_ts -1
	set_seq_ts(&ts);
	assert_int_equal(RETURNED_SUCCESS, init_seq_ts(&ts, MAX_TIMESTAMPS-1));
	assert_int_equal(MAX_TIMESTAMPS-1, ts.max_idx);
	assert_int_equal(0, ts.ts_idx);

	// max_ts
	set_seq_ts(&ts);
	assert_int_equal(RETURNED_SUCCESS, init_seq_ts(&ts, MAX_TIMESTAMPS));
	assert_int_equal(MAX_TIMESTAMPS, ts.max_idx);
	assert_int_equal(0, ts.ts_idx);

	// max_ts + 1
	set_seq_ts(&ts);
	assert_int_equal(0xca, ts.max_idx);
	assert_int_equal(0xfe, ts.ts_idx);
	assert_int_equal(RETURNED_FAILURE, init_seq_ts(&ts, MAX_TIMESTAMPS+1));
	assert_int_equal(0xca, ts.max_idx);
	assert_int_equal(0xfe, ts.ts_idx);

	(void)state; // not used
}

/** @brief Test handling of null parameters for ts_now
 */
static void ts_now_null_parm_test(void **state)
{
	// just make sure it doesn't blow chunks, no rc
	ts_now(NULL);

	(void)state; // not used
}

/** @brief Test correct operation of ts_now
 */
static void ts_now_test(void **state)
{
	struct seq_ts ts;

	// initialize with known values
	set_seq_ts(&ts);
	assert_int_equal(0xca, ts.max_idx);
	assert_int_equal(0xfe, ts.ts_idx);

	// room for three timestamps
	assert_int_equal(RETURNED_SUCCESS, init_seq_ts(&ts, 3));
	assert_int_equal(0, ts.ts_idx);
	assert_int_equal(0, ts.ts_mkr[0]);
	assert_int_equal(0, ts.ts_mkr[1]);
	assert_int_equal(0, ts.ts_mkr[2]);

	// use first
	ts_now(&ts);
	assert_int_equal(1, ts.ts_idx);
	assert_int_equal(0, ts.ts_mkr[0]);
	assert_int_equal(0, ts.ts_mkr[1]);
	assert_int_equal(0, ts.ts_mkr[2]);

	// use second
	ts_now(&ts);
	assert_int_equal(2, ts.ts_idx);
	assert_int_equal(0, ts.ts_mkr[0]);
	assert_int_equal(0, ts.ts_mkr[1]);
	assert_int_equal(0, ts.ts_mkr[2]);

	// use third
	ts_now(&ts);
	assert_int_equal(3, ts.ts_idx);
	assert_int_equal(0, ts.ts_mkr[0]);
	assert_int_equal(0, ts.ts_mkr[1]);
	assert_int_equal(0, ts.ts_mkr[2]);

	// stil at third, out of room
	ts_now(&ts);
	assert_int_equal(3, ts.ts_idx);
	assert_int_equal(0, ts.ts_mkr[0]);
	assert_int_equal(0, ts.ts_mkr[1]);
	assert_int_equal(0, ts.ts_mkr[2]);

	(void)state; // not used
}

/** @brief Test handling of null parameters for ts_now_mark
 */
static void ts_now_mark_null_parm_test(void **state)
{
	// just make sure it doesn't blow chunks, no rc
	ts_now_mark(NULL, 0xdead);

	(void)state; // not used
}

/** @brief Test correct operation of ts_now_mark
 */
static void ts_now_mark_test(void **state)
{
	struct seq_ts ts;

	// initialize with known values
	set_seq_ts(&ts);
	assert_int_equal(0xca, ts.max_idx);
	assert_int_equal(0xfe, ts.ts_idx);

	// room for four timestamps
	assert_int_equal(RETURNED_SUCCESS, init_seq_ts(&ts, 4));
	assert_int_equal(0, ts.ts_idx);
	assert_int_equal(0, ts.ts_mkr[0]);
	assert_int_equal(0, ts.ts_mkr[1]);
	assert_int_equal(0, ts.ts_mkr[2]);
	assert_int_equal(0, ts.ts_mkr[3]);

	ts_now_mark(&ts, 0xdead);
	assert_int_equal(1, ts.ts_idx);
	assert_int_equal(0xdead, ts.ts_mkr[0]);
	assert_int_equal(0, ts.ts_mkr[1]);
	assert_int_equal(0, ts.ts_mkr[2]);
	assert_int_equal(0, ts.ts_mkr[3]);

	ts_now_mark(&ts, 0xbeef);
	assert_int_equal(2, ts.ts_idx);
	assert_int_equal(0xdead, ts.ts_mkr[0]);
	assert_int_equal(0xbeef, ts.ts_mkr[1]);
	assert_int_equal(0, ts.ts_mkr[2]);
	assert_int_equal(0, ts.ts_mkr[3]);

	ts_now_mark(&ts, 0xcafe);
	assert_int_equal(3, ts.ts_idx);
	assert_int_equal(0xdead, ts.ts_mkr[0]);
	assert_int_equal(0xbeef, ts.ts_mkr[1]);
	assert_int_equal(0xcafe, ts.ts_mkr[2]);
	assert_int_equal(0, ts.ts_mkr[3]);

	ts_now_mark(&ts, 0xbabe);
	assert_int_equal(4, ts.ts_idx);
	assert_int_equal(0xdead, ts.ts_mkr[0]);
	assert_int_equal(0xbeef, ts.ts_mkr[1]);
	assert_int_equal(0xcafe, ts.ts_mkr[2]);
	assert_int_equal(0xbabe, ts.ts_mkr[3]);

	ts_now_mark(&ts, 0xcaca);
	assert_int_equal(4, ts.ts_idx);
	assert_int_equal(0xdead, ts.ts_mkr[0]);
	assert_int_equal(0xbeef, ts.ts_mkr[1]);
	assert_int_equal(0xcafe, ts.ts_mkr[2]);
	assert_int_equal(0xbabe, ts.ts_mkr[3]);

	(void)state; // not used
}

/** @brief Test correct operation of time_difference
 */
static void time_difference_test(void **state)
{
	struct timespec start_ts, end_ts, result_ts;

	start_ts.tv_sec = 1;
	start_ts.tv_nsec = 1;
	end_ts.tv_sec = 2;
	end_ts.tv_nsec = 2;
	result_ts = time_difference(start_ts, end_ts);
	assert_int_equal(1, result_ts.tv_sec);
	assert_int_equal(1, result_ts.tv_nsec);

	start_ts.tv_sec = 1000;
	start_ts.tv_nsec = 500000000;
	end_ts.tv_sec = 2000;
	end_ts.tv_nsec = 499999999;
	result_ts = time_difference(start_ts, end_ts);
	assert_int_equal(999, result_ts.tv_sec);
	assert_int_equal(999999999, result_ts.tv_nsec);

	start_ts.tv_sec = 1000;
	start_ts.tv_nsec = 999999999;
	end_ts.tv_sec = 2000;
	end_ts.tv_nsec = 999999999;
	result_ts = time_difference(start_ts, end_ts);
	assert_int_equal(1000, result_ts.tv_sec);
	assert_int_equal(0, result_ts.tv_nsec);

	start_ts.tv_sec = 0;
	start_ts.tv_nsec = 999999999;
	end_ts.tv_sec = 2000;
	end_ts.tv_nsec = 999999999;
	result_ts = time_difference(start_ts, end_ts);
	assert_int_equal(2000, result_ts.tv_sec);
	assert_int_equal(0, result_ts.tv_nsec);

	start_ts.tv_sec = 0;
	start_ts.tv_nsec = 0;
	end_ts.tv_sec = 0;
	end_ts.tv_nsec = 0;
	result_ts = time_difference(start_ts, end_ts);
	assert_int_equal(0, result_ts.tv_sec);
	assert_int_equal(0, result_ts.tv_nsec);

	(void)state; // not used
}

/** @brief Test correct operation of time_add
 */
static void time_add_test(void **state)
{
	struct timespec start_ts, end_ts, result_ts;

	start_ts.tv_sec = 1;
	start_ts.tv_nsec = 1;
	end_ts.tv_sec = 2;
	end_ts.tv_nsec = 2;
	result_ts = time_add(start_ts, end_ts);
	assert_int_equal(3, result_ts.tv_sec);
	assert_int_equal(3, result_ts.tv_nsec);

	start_ts.tv_sec = 1000;
	start_ts.tv_nsec = 500000000;
	end_ts.tv_sec = 2000;
	end_ts.tv_nsec = 750000000;
	result_ts = time_add(start_ts, end_ts);
	assert_int_equal(3001, result_ts.tv_sec);
	assert_int_equal(250000000, result_ts.tv_nsec);

	start_ts.tv_sec = 1000;
	start_ts.tv_nsec = 500000000;
	end_ts.tv_sec = 2000;
	end_ts.tv_nsec = 500000000;
	result_ts = time_add(start_ts, end_ts);
	assert_int_equal(3001, result_ts.tv_sec);
	assert_int_equal(0, result_ts.tv_nsec);

	start_ts.tv_sec = 1000;
	start_ts.tv_nsec = 499999999;
	end_ts.tv_sec = 2000;
	end_ts.tv_nsec = 500000000;
	result_ts = time_add(start_ts, end_ts);
	assert_int_equal(3000, result_ts.tv_sec);
	assert_int_equal(999999999, result_ts.tv_nsec);

	(void)state; // not used
}

/** @brief Test correct operation of time_div
 */
static void time_div_test(void **state)
{
	struct timespec start_ts, result_ts;

	start_ts.tv_sec = 1;
	start_ts.tv_nsec = 1;
	result_ts = time_div(start_ts, 1);
	assert_int_equal(1, result_ts.tv_sec);
	assert_int_equal(1, result_ts.tv_nsec);

	start_ts.tv_sec = 10000;
	start_ts.tv_nsec = 500000000;
	result_ts = time_div(start_ts, 5000);
	assert_int_equal(2, result_ts.tv_sec);
	assert_int_equal(100000, result_ts.tv_nsec);

	start_ts.tv_sec = 1000;
	start_ts.tv_nsec = 500000000;
	result_ts = time_div(start_ts, 5000);
	assert_int_equal(0, result_ts.tv_sec);
	assert_int_equal(200100000, result_ts.tv_nsec);

	start_ts.tv_sec = 1000;
	start_ts.tv_nsec = 499999999;
	result_ts = time_div(start_ts, 1000);
	assert_int_equal(1, result_ts.tv_sec);
	assert_int_equal(499999, result_ts.tv_nsec);

	(void)state; // not used
}

/** @brief Test correct operation of time_track
 */
static void time_track_test(void **state)
{
	struct timespec start_ts, end_ts, totaltime, mintime, maxtime;
	struct timespec test_total_time = {0, 0}, test_mintime, test_maxtime;
	struct timespec start_offset = {0, 4999};
	int i;
	int start_idx = 1, max_idx = 10;

	end_ts = start_offset;

	for (i = start_idx; i < max_idx; i++) {
		start_ts = end_ts;
		end_ts.tv_sec += i * i;
		end_ts.tv_nsec += i * i;

		test_total_time.tv_sec += i * i;
		test_total_time.tv_nsec += i * i;

		time_track(i - start_idx, start_ts, end_ts, &totaltime,
				&mintime, &maxtime);
	}

	test_mintime.tv_sec = start_idx * start_idx;
	test_mintime.tv_nsec = start_idx * start_idx;
	max_idx--;
	test_maxtime.tv_sec = max_idx * max_idx;
	test_maxtime.tv_nsec = max_idx * max_idx;

	assert_int_equal(test_total_time.tv_sec, totaltime.tv_sec);
	assert_int_equal(test_total_time.tv_nsec, totaltime.tv_nsec);
	assert_int_equal(test_mintime.tv_sec, mintime.tv_sec);
	assert_int_equal(test_mintime.tv_nsec, mintime.tv_nsec);
	assert_int_equal(test_maxtime.tv_sec, maxtime.tv_sec);
	assert_int_equal(test_maxtime.tv_nsec, maxtime.tv_nsec);

	(void)state; // not used
}

/** @brief Test correct operation of time_track_lim
 */
static void time_track_lim_test(void **state)
{
	struct timespec start_ts, end_ts, lim;
	struct timespec test_total_time = {0, 0}, test_mintime, test_maxtime;

	// Test start condition, end ts < start ts
	start_ts = (struct timespec){1, 0};
	end_ts = (struct timespec){0, 500000};
	lim = (struct timespec){0, 3000};
	test_mintime = (struct timespec){5, 5};
	test_maxtime = (struct timespec){6, 6};
	test_total_time = (struct timespec){7, 7};

	time_track_lim(0, &lim, &start_ts, &end_ts, &test_total_time,
			&test_mintime, &test_maxtime);

	assert_int_equal(0, test_total_time.tv_sec);
	assert_int_equal(0, test_total_time.tv_nsec);
	assert_int_equal(0xFFFFFFFF, test_mintime.tv_sec);
	assert_int_equal(0xFFFFFFFF, test_mintime.tv_nsec);
	assert_int_equal(0, test_maxtime.tv_sec);
	assert_int_equal(0, test_maxtime.tv_nsec);

	// Test start condition, delta > limit condition 1
	end_ts = (struct timespec){1, 0};
	start_ts = (struct timespec){0, 500000};
	lim = (struct timespec){0, 3000};
	test_mintime = (struct timespec){5, 5};
	test_maxtime = (struct timespec){6, 6};
	test_total_time = (struct timespec){7, 7};

	time_track_lim(0, &lim, &start_ts, &end_ts, &test_total_time,
			&test_mintime, &test_maxtime);

	assert_int_equal(0, test_total_time.tv_sec);
	assert_int_equal(0, test_total_time.tv_nsec);
	assert_int_equal(0xFFFFFFFF, test_mintime.tv_sec);
	assert_int_equal(0xFFFFFFFF, test_mintime.tv_nsec);
	assert_int_equal(0, test_maxtime.tv_sec);
	assert_int_equal(0, test_maxtime.tv_nsec);

	// Test start condition, delta > limit condition 2
	end_ts = (struct timespec){2, 0};
	start_ts = (struct timespec){0, 500000};
	lim = (struct timespec){0, 3000};
	test_mintime = (struct timespec){5, 5};
	test_maxtime = (struct timespec){6, 6};
	test_total_time = (struct timespec){7, 7};

	time_track_lim(0, &lim, &start_ts, &end_ts, &test_total_time,
			&test_mintime, &test_maxtime);

	assert_int_equal(0, test_total_time.tv_sec);
	assert_int_equal(0, test_total_time.tv_nsec);
	assert_int_equal(0xFFFFFFFF, test_mintime.tv_sec);
	assert_int_equal(0xFFFFFFFF, test_mintime.tv_nsec);
	assert_int_equal(0, test_maxtime.tv_sec);
	assert_int_equal(0, test_maxtime.tv_nsec);

	// Test start condition, success
	end_ts = (struct timespec){1, 0};
	start_ts = (struct timespec){0, 500000000};
	lim = (struct timespec){0, 500000000};
	test_mintime = (struct timespec){5, 5};
	test_maxtime = (struct timespec){6, 6};
	test_total_time = (struct timespec){7, 7};

	time_track_lim(0, &lim, &start_ts, &end_ts, &test_total_time,
			&test_mintime, &test_maxtime);

	assert_int_equal(0, test_total_time.tv_sec);
	assert_int_equal(500000000, test_total_time.tv_nsec);
	assert_int_equal(0, test_mintime.tv_sec);
	assert_int_equal(500000000, test_mintime.tv_nsec);
	assert_int_equal(0, test_maxtime.tv_sec);
	assert_int_equal(500000000, test_maxtime.tv_nsec);

	// Test tracking
	end_ts = (struct timespec){0, 500000001};
	start_ts = (struct timespec){0, 500000000};
	lim = (struct timespec){0, 500000000};

	time_track_lim(1, &lim, &start_ts, &end_ts, &test_total_time,
			&test_mintime, &test_maxtime);

	assert_int_equal(0, test_total_time.tv_sec);
	assert_int_equal(500000001, test_total_time.tv_nsec);
	assert_int_equal(0, test_mintime.tv_sec);
	assert_int_equal(1, test_mintime.tv_nsec);
	assert_int_equal(0, test_maxtime.tv_sec);
	assert_int_equal(500000000, test_maxtime.tv_nsec);

	(void)state; // not used
}

int main(int argc, char *argv[])
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
	cmocka_unit_test(assumptions),
	cmocka_unit_test(init_seq_ts_null_parm_test),
	cmocka_unit_test(init_seq_ts_test),
	cmocka_unit_test(ts_now_null_parm_test),
	cmocka_unit_test(ts_now_test),
	cmocka_unit_test(ts_now_mark_null_parm_test),
	cmocka_unit_test(ts_now_mark_test),
	cmocka_unit_test(time_difference_test),
	cmocka_unit_test(time_add_test),
	cmocka_unit_test(time_div_test),
	cmocka_unit_test(time_track_test),
	cmocka_unit_test(time_track_lim_test), };

	return cmocka_run_group_tests(tests, NULL, NULL);
}

#ifdef __cplusplus
}
#endif
