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

#include "RapidIO_Device_Access_Routines_API.h"
#include "RapidIO_Error_Management_API.h"
#include "src/RapidIO_Error_Management_API.c"
#include "DAR_DB_Private.h"
#include "rio_standard.h"
#include "rio_ecosystem.h"
#include "libcli.h"

#ifdef __cplusplus
extern "C" {
#endif

static DAR_DEV_INFO_t mock_dev_info;

// These tests should never try to access real hardware.
// Bind in Read/Write/Delay routines that always fail.

static uint32_t TestReadReg(DAR_DEV_INFO_t *dev_info,
			uint32_t  offset, uint32_t *readdata)
{
	if ((NULL == dev_info) || offset || (NULL == readdata)) {
		return 2;
	}
	return 1;
}

static uint32_t TestWriteReg(DAR_DEV_INFO_t *dev_info,
			uint32_t  offset, uint32_t writedata)
{
	if ((NULL == dev_info) || offset || writedata) {
		return 2;
	}
	return 1;
}

static void TestWaitSec(uint32_t delay_nsec, uint32_t delay_sec)
{
	uint64_t counter = delay_nsec + ((uint64_t)delay_sec * 1000000000);
	for ( ; counter; counter--);
}

static void test_setup(void)
{
	uint8_t idx;

	mock_dev_info.privateData = 0x0;
	mock_dev_info.accessInfo = 0x0;
	strcpy(mock_dev_info.name, "FASTER_EP");
	mock_dev_info.dsf_h = 0x00380000;
	mock_dev_info.extFPtrForPort = 0x100;
	mock_dev_info.extFPtrPortType = 0x1000;
	mock_dev_info.extFPtrForLane = 0x2000;
	mock_dev_info.extFPtrForErr = 0x3000;
	mock_dev_info.extFPtrForVC = 0;
	mock_dev_info.extFPtrForVOQ = 0;
	mock_dev_info.devID = 0x80AB0038;
	mock_dev_info.driver_family = RIO_UNKNOWN_DEVICE;
	mock_dev_info.devInfo = 0;
	mock_dev_info.assyInfo = 0;
	mock_dev_info.features = 0xC000003F;
	mock_dev_info.swPortInfo = 0x00000100;
	mock_dev_info.swRtInfo = 0;
	mock_dev_info.srcOps = 0x0000FC04;
	mock_dev_info.dstOps = 0x0000FC04;
	mock_dev_info.swMcastInfo = 0;
        mock_dev_info.poregs_max = 0;
        mock_dev_info.poreg_cnt = 0;
        mock_dev_info.poregs = NULL;
	for (idx = 0; idx < RIO_MAX_PORTS; idx++) {
		mock_dev_info.ctl1_reg[idx] = 0;
	}

	for (idx = 0; idx < MAX_DAR_SCRPAD_IDX; idx++) {
		mock_dev_info.scratchpad[idx] = 0;
	}
	DAR_proc_ptr_init(TestReadReg, TestWriteReg, TestWaitSec);
}

static void assumptions_test(void **state)
{
	(void)state; // unused
}

static void macros_test(void **state)
{
	assert_string_equal("FLossOfSig", EVENT_NAME_STR(rio_em_f_los));
	assert_string_equal("FPortErr", EVENT_NAME_STR(rio_em_f_port_err));
	assert_string_equal("F2ManyReTx", EVENT_NAME_STR(rio_em_f_2many_retx));
	assert_string_equal("F2ManyPNA", EVENT_NAME_STR(rio_em_f_2many_pna));
	assert_string_equal("FErrRate", EVENT_NAME_STR(rio_em_f_err_rate));
	assert_string_equal("DropTTL", EVENT_NAME_STR(rio_em_d_ttl));
	assert_string_equal("DropOnPurp", EVENT_NAME_STR(rio_em_d_rte));
	assert_string_equal("DropLogErr", EVENT_NAME_STR(rio_em_d_log));
	assert_string_equal("ISigDet", EVENT_NAME_STR(rio_em_i_sig_det));
	assert_string_equal("IRstReq", EVENT_NAME_STR(rio_em_i_rst_req));
	assert_string_equal("IInitFail", EVENT_NAME_STR(rio_em_i_init_fail));
	assert_string_equal("AClrPwPnd", EVENT_NAME_STR(rio_em_a_clr_pwpnd));
	assert_string_equal("ANoEvent", EVENT_NAME_STR(rio_em_a_no_event));
	assert_string_equal("OORange", EVENT_NAME_STR(rio_em_last));

	assert_string_equal("NtfnNone", NOTFN_CTL_STR(rio_em_notfn_none));
	assert_string_equal("NtfnInt", NOTFN_CTL_STR(rio_em_notfn_int));
	assert_string_equal("NtfnPW", NOTFN_CTL_STR(rio_em_notfn_pw));
	assert_string_equal("NtfnBoth", NOTFN_CTL_STR(rio_em_notfn_both));
	assert_string_equal("NtfnNoChg", NOTFN_CTL_STR(rio_em_notfn_0delta));
	assert_string_equal("OORange", NOTFN_CTL_STR(rio_em_notfn_last));

	assert_string_equal("DetOff", DETECT_CTL_STR(rio_em_detect_off));
	assert_string_equal("DetOn", DETECT_CTL_STR(rio_em_detect_on));
	assert_string_equal("DetNoChg", DETECT_CTL_STR(rio_em_detect_0delta));
	assert_string_equal("OORange", DETECT_CTL_STR(rio_em_detect_last));

	(void)state; // unused
}

typedef struct err_rate_checks_t_TAG {
	uint32_t spx_rate_en;
	uint32_t spx_err_rate;
	uint32_t spx_err_thresh;
} err_rate_checks_t;

static void rio_em_compute_f_err_rate_info_test(void **state)
{
	uint32_t spx_rate_en;
	uint32_t spx_err_rate;
	uint32_t spx_err_thresh;
	uint32_t info;

	err_rate_checks_t event[] = {
		{RIO_EMHS_SPX_RATE_EN_LINK_TO,
			RIO_EMHS_SPX_RATE_RB_NONE | RIO_EMHS_SPX_RATE_RR_LIM_2,
			0x01000000},
		{RIO_EMHS_SPX_RATE_EN_CS_ACK_ILL,
			RIO_EMHS_SPX_RATE_RB_1_MS |
				RIO_EMHS_SPX_RATE_RR_LIM_4,
			0x10999999},
		{RIO_EMHS_SPX_RATE_EN_DELIN_ERR,
			RIO_EMHS_SPX_RATE_RB_10_MS |
				RIO_EMHS_SPX_RATE_RR_LIM_16,
			0xFFAAAAAA},
		{RIO_EMHS_SPX_RATE_EN_PROT_ERR,
			RIO_EMHS_SPX_RATE_RB_100_MS |
				RIO_EMHS_SPX_RATE_RR_LIM_NONE,
			0x30AAAAAA},
		{RIO_EMHS_SPX_RATE_EN_LR_ACKID_ILL,
			RIO_EMHS_SPX_RATE_RB_1_SEC |
				RIO_EMHS_SPX_RATE_RR_LIM_NONE,
			0x40BBBBBB},
		{RIO_EMHS_SPX_RATE_EN_PKT_ILL_SIZE,
			RIO_EMHS_SPX_RATE_RB_NONE |
				RIO_EMHS_SPX_RATE_RR_LIM_NONE,
			0x01000000},
		{RIO_EMHS_SPX_RATE_EN_PKT_CRC_ERR,
			RIO_EMHS_SPX_RATE_RB_NONE |
				RIO_EMHS_SPX_RATE_RR_LIM_NONE,
			0x010CCCCC},
		{RIO_EMHS_SPX_RATE_EN_CS_NOT_ACC,
			RIO_EMHS_SPX_RATE_RB_NONE |
				RIO_EMHS_SPX_RATE_RR_LIM_NONE,
			0x010DDDDD},
		{RIO_EMHS_SPX_RATE_EN_CS_ILL_ID,
			RIO_EMHS_SPX_RATE_RB_NONE |
				RIO_EMHS_SPX_RATE_RR_LIM_NONE,
			0x010DDDDD},
		{RIO_EMHS_SPX_RATE_EN_CS_CRC_ERR,
			RIO_EMHS_SPX_RATE_RB_NONE |
				RIO_EMHS_SPX_RATE_RR_LIM_NONE,
			0x010DDDDD},
		{RIO_EMHS_SPX_RATE_EN_LINK_TO |
				RIO_EMHS_SPX_RATE_EN_CS_ACK_ILL |
				RIO_EMHS_SPX_RATE_EN_DELIN_ERR |
				RIO_EMHS_SPX_RATE_EN_PROT_ERR |
				RIO_EMHS_SPX_RATE_EN_LR_ACKID_ILL |
				RIO_EMHS_SPX_RATE_EN_CS_NOT_ACC |
				RIO_EMHS_SPX_RATE_EN_PKT_ILL_SIZE |
				RIO_EMHS_SPX_RATE_EN_PKT_CRC_ERR |
				RIO_EMHS_SPX_RATE_EN_PKT_ILL_ACKID |
				RIO_EMHS_SPX_RATE_EN_CS_NOT_ACC |
				RIO_EMHS_SPX_RATE_EN_CS_ILL_ID |
				RIO_EMHS_SPX_RATE_EN_CS_CRC_ERR,
			RIO_EMHS_SPX_RATE_RB_NONE |
				RIO_EMHS_SPX_RATE_RR_LIM_NONE,
			0x010DDDDD},
	};

	unsigned int num_events = sizeof(event)/sizeof(event[0]);
	unsigned int i;

	for (i = 0; i < num_events; i++) {
		assert_int_equal(RIO_SUCCESS,
			rio_em_compute_f_err_rate_info( event[i].spx_rate_en,
				event[i].spx_err_rate, event[i].spx_err_thresh,
        			&info));

		assert_int_equal(RIO_SUCCESS,
			rio_em_get_f_err_rate_info(info, &spx_rate_en,
				&spx_err_rate, &spx_err_thresh));

		assert_int_equal(spx_rate_en, event[i].spx_rate_en);
		assert_int_equal(spx_err_rate, event[i].spx_err_rate);
		assert_int_equal(spx_err_thresh,
			event[i].spx_err_thresh & RIO_EMHS_SPX_THRESH_FAIL);
	}

	(void)state; // unused
}

static void rio_em_compute_f_err_rate_unsup_test(void **state)
{
	uint32_t info;

	err_rate_checks_t event[] = {
		{RIO_EMHS_SPX_RATE_EN_BAD_OS, 
			RIO_EMHS_SPX_RATE_RB_10_SEC |
				RIO_EMHS_SPX_RATE_RR_LIM_NONE,
			0x50000000},
		{RIO_EMHS_SPX_RATE_EN_LODS, 
			RIO_EMHS_SPX_RATE_RB_100_SEC |
				RIO_EMHS_SPX_RATE_RR_LIM_NONE,
			0x01000000},
		{RIO_EMHS_SPX_RATE_EN_BAD_IDLE1, 
			RIO_EMHS_SPX_RATE_RB_1000_SEC |
				RIO_EMHS_SPX_RATE_RR_LIM_NONE,
			0x01000000},
		{RIO_EMHS_SPX_RATE_EN_BAD_CHAR, 
			RIO_EMHS_SPX_RATE_RB_10000_SEC |
				RIO_EMHS_SPX_RATE_RR_LIM_NONE,
			0x01000000},
		{RIO_EMHS_SPX_RATE_EN_IMP_SPEC_ERR, 
			RIO_EMHS_SPX_RATE_RB_NONE |
				RIO_EMHS_SPX_RATE_RR_LIM_NONE,
			0x010DDDDD},
	};
	unsigned int num_events = sizeof(event)/sizeof(event[0]);
	unsigned int i;

	for (i = 0; i < num_events; i++) {
		assert_int_not_equal(RIO_SUCCESS,
			rio_em_compute_f_err_rate_info( event[i].spx_rate_en,
				event[i].spx_err_rate, event[i].spx_err_thresh,
        			&info));

	}

	(void)state; // unused
}
int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(assumptions_test),
		cmocka_unit_test(macros_test),
		cmocka_unit_test(rio_em_compute_f_err_rate_info_test),
		cmocka_unit_test(rio_em_compute_f_err_rate_unsup_test),
	};

	test_setup();
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#ifdef __cplusplus
}
#endif
