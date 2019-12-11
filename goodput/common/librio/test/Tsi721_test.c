/*
 ************************************************************************ 
 Copyright (c) 2016, Integrated Device Technology Inc.
 Copyright (c) 2016, RapidIO Trade Association
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 l of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this l of conditions and the following disclaimer in the documentation
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
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <stdarg.h>
#include <setjmp.h>
#include "cmocka.h"

#include "Tsi721.h"
#include "src/Tsi721_SC.c"
#include "src/Tsi721_EM.c"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TSI721_DAR_WANTED

static void tsi721_not_supported_test(void **state)
{
	(void)state; // not used
}

int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
	cmocka_unit_test(tsi721_not_supported_test)};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif /* TSI721_DAR_WANTED */

#ifdef TSI721_DAR_WANTED

static void tsi721_assumptions_test(void **state)
{
	// Verify constants
	assert_int_equal(1, TSI721_MAX_PORTS);
	assert_int_equal(4, TSI721_MAX_LANES);

	// Verify assumptions in the code
	assert_int_equal(0xFFFF0000, TSI721_ODB_CNTX_ODB_TOT_CNT);
	assert_int_equal(0xFFFF0000, TSI721_NWR_CNT_NW_TOT_CNT);
	assert_int_equal(0xFFFF0000, TSI721_MWR_CNT_MW_TOT_CNT);
	assert_int_equal(0x0000FFFF, TSI721_ODB_CNTX_ODB_OK_CNT);
	assert_int_equal(0x0000FFFF, TSI721_NWR_CNT_NW_OK_CNT);
	assert_int_equal(0x0000FFFF, TSI721_MWR_CNT_MW_OK_CNT);
	assert_int_equal(TSI721_PLM_STATUS_MAX_DENIAL,
			TSI721_PLM_INT_ENABLE_MAX_DENIAL);
	assert_int_equal(TSI721_PLM_STATUS_MAX_DENIAL,
			TSI721_PLM_PW_ENABLE_MAX_DENIAL);
	assert_int_equal(TSI721_PLM_INT_ENABLE_PORT_ERR,
			TSI721_PLM_STATUS_PORT_ERR);
	assert_int_equal(TSI721_PLM_PW_ENABLE_PORT_ERR,
			TSI721_PLM_STATUS_PORT_ERR);
	assert_int_equal(TSI721_PLM_INT_ENABLE_DLT, TSI721_PLM_STATUS_DLT);
	assert_int_equal(TSI721_PLM_PW_ENABLE_DLT, TSI721_PLM_STATUS_DLT);
	assert_int_equal(TSI721_BASE_ID, RIO_DEVID);

	// Verify ranges are sane
	assert_in_range(TSI721_NUM_PERF_CTRS, 0, RIO_MAX_SC);

	// Verify that PKT_CRC is not part of the error rate exclusions
	assert_int_equal(0, TSI721_SP_ERR_DET_PKT_CRC_ERR &
			TSI721_ERR_RATE_EVENT_EXCLUSIONS);

	(void)state;// unused
}

static void tsi721_macros_test(void **state)
{
	// Verify assumptions in the code
	assert_int_equal(256, TSI721_PLM_IMP_SPEC_CTL_DLT_TICK_USEC);
	assert_int_equal(256000, TSI721_PLM_DLT_TICK_NSEC);
	assert_int_equal(1, TSI721_NSEC_TO_DLT(256000));
	assert_int_equal(2, TSI721_NSEC_TO_DLT(256001));
	assert_int_equal(3907, TSI721_NSEC_TO_DLT(1000000000));
	assert_int_equal(256000, TSI721_DLT_TO_NSEC(1));
	assert_int_equal(512000, TSI721_DLT_TO_NSEC(2));
	assert_int_equal(1000192000, TSI721_DLT_TO_NSEC(3907));

	assert_int_equal(188, TSI721_SR_RSP_TO_TICK_NSEC);
	assert_int_equal(1, TSI721_NSEC_TO_RTO(1));
	assert_int_equal(100, TSI721_NSEC_TO_RTO(18800));
	assert_int_equal(101, TSI721_NSEC_TO_RTO(18801));
	(void)state;// unused
}
int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(tsi721_assumptions_test),
		cmocka_unit_test(tsi721_macros_test),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif /* TSI721_DAR_WANTED */

#ifdef __cplusplus
}
#endif

