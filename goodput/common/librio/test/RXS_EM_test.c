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

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <stdarg.h>
#include <setjmp.h>
#include "cmocka.h"

#include "src/RXS_DeviceDriver.h"
#include "rio_standard.h"
#include "rio_ecosystem.h"
#include "tok_parse.h"
#include "libcli.h"
#include "rio_mport_lib.h"

#include "RXS2448.h"
#include "src/RXS_RT.c"
#include "src/RXS_EM.c"

#define DEBUG_PRINTF 0
#define DEBUG_REGTRACE 0

#include "common_src/RXS_cmdline.c"
#include "common_src/RXS_reg_emulation.c"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RXS_DAR_WANTED

static void rsx_not_supported_test(void **state)
{
	(void)state; // not used
}

int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++;// not used

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(rsx_not_supported_test)};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif /* RXS_DAR_WANTED */

#ifdef RXS_DAR_WANTED

#define COMPUTE_RXS_PW_RETX(x) \
	((((x * PORT_WRITE_RE_TX_NSEC) + RXS_PW_CTL_PW_TMR_NSEC - 1) / \
			RXS_PW_CTL_PW_TMR_NSEC) << 8)

static void rxs_em_cfg_pw_success_test(void **state)
{
	rio_em_cfg_pw_in_t in_parms;
	rio_em_cfg_pw_out_t out_parms;
	did_reg_t targ_id = 0x1234;
	uint32_t retx = 1000000; // 352 msec
	uint32_t retx_reg = COMPUTE_RXS_PW_RETX(retx);
	uint32_t chkdata;
	rio_port_t dflt_port;
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_state_t rt;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	// Power up and enable all ports...
	if (l_st->real_hw) {
		dflt_port = l_st->conn_port;
	} else {
		dflt_port = 5;
	}

	// RXS routes port-writes according to port bit-vector.
	// RXS EM support computes this bit vector based on current routing
	// table values, so set up the routing table...

	init_in.set_on_port = RIO_ALL_PORTS;
	init_in.default_route = RIO_RTV_PORT(dflt_port);
	init_in.default_route_table_port = RIO_RTV_PORT(dflt_port);
	init_in.update_hw = true;
	init_in.rt = &rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));

	// Power up and enable all ports...
	if (!l_st->real_hw) {
		set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);
	}

	// Test for dev16 destIDs...
	in_parms.imp_rc = 0xFFFFFFFF;
	in_parms.deviceID_tt = tt_dev16;
	in_parms.port_write_destID = targ_id;
	in_parms.srcID_valid = false;
	in_parms.port_write_srcID = 0;
	in_parms.priority = 1;
	in_parms.CRF = false;
	in_parms.port_write_re_tx = retx; // 1 msec
	memset(&out_parms, 0, sizeof(out_parms));
	in_parms.imp_rc = 0xFFFFFFFF;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_em_cfg_pw(&mock_dev_info, &in_parms, &out_parms));
	assert_int_equal(0, out_parms.imp_rc);
	assert_int_equal(tt_dev16, out_parms.deviceID_tt);
	assert_int_equal(targ_id, out_parms.port_write_destID);
	assert_false(out_parms.srcID_valid);
	assert_int_equal(0, out_parms.port_write_srcID);
	assert_int_equal(3, out_parms.priority);
	assert_true(out_parms.CRF);
	assert_int_equal(retx, in_parms.port_write_re_tx);

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_PW_TGT_ID, &chkdata));
	assert_int_equal(chkdata, (targ_id << 16) | RXS_PW_TGT_ID_DEV16);

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_PW_CTL, &chkdata));
	assert_int_equal(chkdata, retx_reg);

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_PW_ROUTE, &chkdata));
	assert_int_equal(chkdata, 1 << dflt_port);

	// Test for dev8 destIDs...
	in_parms.imp_rc = 0xFFFFFFFF;
	in_parms.deviceID_tt = tt_dev8;
	in_parms.port_write_destID = targ_id & 0xFF;
	in_parms.srcID_valid = false;
	in_parms.port_write_srcID = 0;
	in_parms.priority = 1;
	in_parms.CRF = false;
	in_parms.port_write_re_tx = retx;
	memset(&out_parms, 0, sizeof(out_parms));
	in_parms.imp_rc = 0xFFFFFFFF;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_em_cfg_pw(&mock_dev_info, &in_parms, &out_parms));
	assert_int_equal(0, out_parms.imp_rc);
	assert_int_equal(tt_dev8, out_parms.deviceID_tt);
	assert_int_equal((did_reg_t)(targ_id & 0xFF), out_parms.port_write_destID);
	assert_false(out_parms.srcID_valid);
	assert_int_equal((did_reg_t)0, out_parms.port_write_srcID);
	assert_int_equal(3, out_parms.priority);
	assert_true(out_parms.CRF);
	assert_int_equal(retx, out_parms.port_write_re_tx);

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_PW_TGT_ID, &chkdata));
	assert_int_equal(chkdata, (targ_id << 16) & RXS_PW_TGT_ID_PW_TGT_ID);

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_PW_CTL, &chkdata));
	assert_int_equal(chkdata, retx_reg);

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_PW_ROUTE, &chkdata));
	assert_int_equal(chkdata, 1 << dflt_port);
	(void)state; // unused
}

static void rxs_em_cfg_pw_bad_parms_test(void **state)
{
	rio_em_cfg_pw_in_t in_parms;
	rio_em_cfg_pw_out_t out_parms;
	did_reg_t targ_id = 0x12345678;

	// Test for dev16 destIDs...
	in_parms.imp_rc = 0xFFFFFFFF;
	in_parms.deviceID_tt = tt_dev16;
	in_parms.port_write_destID = targ_id;
	in_parms.srcID_valid = false;
	in_parms.port_write_srcID = 0;
	in_parms.priority = 1;
	in_parms.CRF = false;
	in_parms.port_write_re_tx = 5555;
	memset(&out_parms, 0, sizeof(out_parms));

	in_parms.deviceID_tt = tt_dev16;
	in_parms.priority = 4;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_cfg_pw(&mock_dev_info, &in_parms,
					&out_parms));
	assert_int_not_equal(0, out_parms.imp_rc);

	(void)state; // unused
}

typedef struct rxs_pw_retx_info_t_TAG {
	uint32_t timer_val_in;
	uint32_t timer_val_out;
	uint32_t reg_val_out;
} rxs_pw_retx_info_t;

// Verify the RXS exact values for port-write retransmission
static void rxs_rio_em_cfg_pw_retx_compute_test(void **state)
{
	rio_em_cfg_pw_in_t in_p;
	rio_em_cfg_pw_out_t out_p;
	did_reg_t targ_id = 0x1234;
	uint32_t chkdata;
	const rxs_pw_retx_info_t tests[] = {
		{ 1000, 1000, COMPUTE_RXS_PW_RETX(1000)},
		{ 9999, 9999, COMPUTE_RXS_PW_RETX(9999)},
		{ 1234, 1234, COMPUTE_RXS_PW_RETX(1234)},
		{ 1, 1, COMPUTE_RXS_PW_RETX(1)}
	};
	const int num_tests = sizeof(tests) / sizeof(tests[0]);
	int i;
	rio_port_t dflt_port = 5;
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_state_t rt;

	// RXS routes port-writes according to port bit-vector.
	// RXS EM support computes this bit vector based on current routing
	// table values, so set up the routing table...

	init_in.set_on_port = RIO_ALL_PORTS;
	init_in.default_route = RIO_RTV_PORT(dflt_port);
	init_in.default_route_table_port = RIO_RTV_PORT(dflt_port);
	init_in.update_hw = true;
	init_in.rt = &rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));

	// Power up and enable all ports...
	set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);

	// Test for dev16 destIDs...
	in_p.imp_rc = 0xFFFFFFFF;
	in_p.deviceID_tt = tt_dev16;
	in_p.port_write_destID = targ_id;
	in_p.srcID_valid = false;
	in_p.port_write_srcID = 0;
	in_p.priority = 1;
	in_p.CRF = false;
	memset(&out_p, 0, sizeof(out_p));
	in_p.imp_rc = 0xFFFFFFFF;

	for (i = 0; i < num_tests; i++) {
		in_p.port_write_re_tx = tests[i].timer_val_in;
		assert_int_equal(RIO_SUCCESS,
			rxs_rio_em_cfg_pw(&mock_dev_info, &in_p, &out_p));

		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(tt_dev16, out_p.deviceID_tt);
		assert_int_equal(targ_id, out_p.port_write_destID);
		assert_false(out_p.srcID_valid);
		assert_int_equal((did_reg_t)0, out_p.port_write_srcID);
		assert_int_equal(3, out_p.priority);
		assert_true(out_p.CRF);
		assert_int_equal(tests[i].timer_val_out,
				out_p.port_write_re_tx);

		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_PW_CTL, &chkdata));
		assert_int_equal(chkdata, tests[i].reg_val_out);
	}

	(void)state; // unused
}

static void chk_plm_event_enables(rio_em_notfn_ctl_t ntfn,
				uint32_t event_mask,
				rio_port_t port)
{
	uint32_t int_en;
	uint32_t pw_en;
	bool ints;
	bool pws;

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_PLM_SPX_INT_EN(port), &int_en));
	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_PLM_SPX_PW_EN(port), &pw_en));

	pws = pw_en & event_mask;
	ints = int_en & event_mask;

	switch (ntfn) {
	case rio_em_notfn_none:
		assert_false(ints);
		assert_false(pws);
		break;
	case rio_em_notfn_int:
		assert_true(ints);
		assert_false(pws);
		break;
	case rio_em_notfn_pw:
		assert_false(ints);
		assert_true(pws);
		break;
	case rio_em_notfn_both:
		assert_true(ints);
		assert_true(pws);
		break;
	case rio_em_notfn_0delta:
	case rio_em_notfn_last:
	default:
		assert_true(false);
		break;
	}
}

static void chk_regs_los(rio_em_cfg_t *event, rio_port_t port)
{
	uint32_t ctl;
	uint32_t plm_ctl;
	uint32_t dlt_to;
	uint32_t ctl_mask = RXS_SPX_RATE_EN_OK_TO_UNINIT |
				RXS_SPX_RATE_EN_DLT;
	uint32_t plm_ctl_mask = RXS_PLM_SPX_IMP_SPEC_CTL_OK2U_FATAL |
				RXS_PLM_SPX_IMP_SPEC_CTL_DLT_FATAL |
				RXS_PLM_SPX_IMP_SPEC_CTL_DWNGD_FATAL;
	uint64_t time;

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_SPX_RATE_EN(port), &ctl));

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_SPX_DLT_CSR(port), &dlt_to));

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_PLM_SPX_IMP_SPEC_CTL(port),
								&plm_ctl));

	ctl &= ctl_mask;
	time = (dlt_to >> 8);
	time *= RXS_SPX_DLT_CSR_TIMEOUT_NSEC;
	if (time > UINT32_MAX) {
		time = UINT32_MAX;
	}
	dlt_to = time;
	plm_ctl &= plm_ctl_mask;

	switch (event->em_detect) {
	case rio_em_detect_off:
		assert_int_equal(0, ctl & ctl_mask);
		assert_int_equal(0, dlt_to);
		assert_int_equal(0, plm_ctl & plm_ctl_mask);
		break;

	case rio_em_detect_on:
		assert_int_equal(plm_ctl & plm_ctl_mask, plm_ctl_mask);
		assert_int_equal(dlt_to, event->em_info);
		if (event->em_info) {
			assert_int_equal(ctl, RXS_SPX_RATE_EN_DLT);
		} else {
			assert_int_equal(ctl, RXS_SPX_RATE_EN_OK_TO_UNINIT);
		}
		break;
	case rio_em_detect_0delta:
	case rio_em_detect_last:
	default:
		assert_true(false);
	}
}

static void chk_regs_2many_retx(rio_em_cfg_t *event, rio_port_t port)
{
	uint32_t plm_denial_ctl;
	uint32_t mask = RXS_PLM_SPX_DENIAL_CTL_CNT_RTY;
	uint32_t thresh;

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_PLM_SPX_DENIAL_CTL(port),
							&plm_denial_ctl));
	thresh = plm_denial_ctl & RXS_PLM_SPX_DENIAL_CTL_DENIAL_THRESH;

	switch (event->em_detect) {
	case rio_em_detect_off:
		assert_int_equal(0, plm_denial_ctl & mask);
		break;
	case rio_em_detect_on:
		assert_int_equal(thresh, event->em_info);
		assert_int_equal(mask, plm_denial_ctl & mask);
		break;
	case rio_em_detect_0delta:
	case rio_em_detect_last:
	default:
		assert_true(false);
	}
}

static void chk_regs_2many_pna(rio_em_cfg_t *event, rio_port_t port)
{
	uint32_t ctl;
	uint32_t mask = RXS_PLM_SPX_DENIAL_CTL_CNT_PNA;
	uint32_t thresh;

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_PLM_SPX_DENIAL_CTL(port), &ctl));
	thresh = ctl & RXS_PLM_SPX_DENIAL_CTL_DENIAL_THRESH;

	switch (event->em_detect) {
	case rio_em_detect_off:
		assert_int_equal(0, ctl & mask);
		break;
	case rio_em_detect_on:
		assert_int_equal(thresh, event->em_info);
		assert_int_equal(mask, ctl & mask);
		break;
	case rio_em_detect_0delta:
	case rio_em_detect_last:
	default:
		assert_true(false);
	}
}

static void chk_em_int_pw_regs(rio_em_cfg_t *event, rio_em_notfn_ctl_t ntfn)
{
	uint32_t int_mask = 0;
	uint32_t int_en;
	uint32_t pw_mask = 0;
	uint32_t pw_en;

	switch(event->em_event) {
	case rio_em_d_log:
			int_mask = RXS_EM_INT_EN_LOG;
			pw_mask = RXS_EM_PW_EN_LOG;
			break;
	case rio_em_i_init_fail:
			int_mask = RXS_EM_INT_EN_EXTERNAL_I2C;
			pw_mask = RXS_EM_PW_EN_EXTERNAL_I2C;
			break;
	default:
		assert_true(false);
	}

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_EM_INT_EN, &int_en));
	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_EM_PW_EN, &pw_en));

	int_en &= int_mask;
	pw_en &= pw_mask;

	switch (ntfn) {
	case rio_em_notfn_none:
		assert_false(int_en);
		assert_false(pw_en);
		break;
	case rio_em_notfn_int:
		assert_true(int_en);
		assert_false(pw_en);
		break;
	case rio_em_notfn_pw:
		assert_false(int_en);
		assert_true(pw_en);
		break;
	case rio_em_notfn_both:
		assert_true(int_en);
		assert_true(pw_en);
		break;
	case rio_em_notfn_0delta:
	case rio_em_notfn_last:
	default:
		assert_true(false);
		break;
	}
}

static void chk_regs_log(rio_em_cfg_t *event, rio_em_notfn_ctl_t ntfn)
{
	uint32_t err_en;
	uint32_t mask = RXS_ERR_EN_ILL_TYPE_EN |
			RXS_ERR_EN_UNS_RSP_EN |
			RXS_ERR_EN_ILL_ID_EN;

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_ERR_EN, &err_en));

	if (rio_em_detect_on == event->em_detect) {
		assert_int_equal(event->em_info, err_en & mask);
	} else {
		assert_int_equal(0, err_en & mask);
	}
	chk_em_int_pw_regs(event, ntfn);
}

static void chk_regs_sig_det(rio_em_cfg_t *event, rio_port_t port)
{
	uint32_t rate_en;
	uint32_t mask = RXS_SPX_ERR_DET_LINK_INIT;

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_SPX_RATE_EN(port), &rate_en));

	rate_en &= mask;
	if (rio_em_detect_on == event->em_detect) {
		assert_int_equal(mask, rate_en);
	} else {
		assert_int_equal(0, rate_en);
	}
}

static void chk_regs_rst_req(rio_em_cfg_t *event, rio_port_t port)
{
	uint32_t plm_ctl;
	uint32_t mask = RXS_PLM_SPX_IMP_SPEC_CTL_SELF_RST |
			RXS_PLM_SPX_IMP_SPEC_CTL_PORT_SELF_RST;

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_PLM_SPX_IMP_SPEC_CTL(port),
								&plm_ctl));

	// An event can be detected as long as at least one of the self
	// reset bits is cleared.
	if (rio_em_detect_on == event->em_detect) {
		assert_int_not_equal(mask, plm_ctl & mask);
	} else {
		assert_int_equal(mask, plm_ctl & mask);
	}
}

static void chk_regs_init_fail(rio_em_cfg_t *event, rio_em_notfn_ctl_t ntfn)
{
	uint32_t i2c_i_en;
	uint32_t mask = I2C_INT_ENABLE_BL_FAIL;

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, I2C_INT_ENABLE, &i2c_i_en));

	i2c_i_en &= mask;
	if (rio_em_detect_on == event->em_detect) {
		assert_int_equal(mask, i2c_i_en);
	} else {
		assert_int_equal(0, i2c_i_en);
	}

	chk_em_int_pw_regs(event, ntfn);
}

static void chk_regs_ttl(rio_em_cfg_t *event,
			rio_port_t port,
			rio_em_notfn_ctl_t ntfn)
{
	uint32_t ttl_tmr;
	uint32_t int_en;
	uint32_t pw_en;
	uint32_t i_mask = RXS_PBM_SPX_INT_EN_EG_TTL_EXPIRED;
	uint32_t p_mask = RXS_PBM_SPX_PW_EN_EG_TTL_EXPIRED;
	uint32_t tval = (((event->em_info + 5999) / 6000) -1 ) * 4;

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_PKT_TIME_LIVE, &ttl_tmr));
	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_PBM_SPX_INT_EN(port), &int_en));
	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_PBM_SPX_PW_EN(port), &pw_en));

	if (!tval) {
		tval = 1;
	}
	if (tval > 0xFFFC) {
		tval = 0xFFFC;
	}

	if (rio_em_detect_on == event->em_detect) {
		assert_int_equal(tval << 16, ttl_tmr);
	} else {
		assert_int_equal(0, ttl_tmr);
	}

	int_en &= i_mask;
	pw_en &= p_mask;

	switch (ntfn) {
	case rio_em_notfn_none:
		assert_false(int_en);
		assert_false(pw_en);
		break;
	case rio_em_notfn_int:
		assert_true(int_en);
		assert_false(pw_en);
		break;
	case rio_em_notfn_pw:
		assert_false(int_en);
		assert_true(pw_en);
		break;
	case rio_em_notfn_both:
		assert_true(int_en);
		assert_true(pw_en);
		break;
	case rio_em_notfn_0delta:
	case rio_em_notfn_last:
	default:
		assert_true(false);
		break;
	}
}

static void chk_regs_rte(rio_port_t port,
			rio_em_notfn_ctl_t ntfn)
{
	uint32_t int_en;
	uint32_t pw_en;
	uint32_t i_mask = RXS_TLM_SPX_INT_EN_LUT_DISCARD;
	uint32_t p_mask = RXS_TLM_SPX_PW_EN_LUT_DISCARD;

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_TLM_SPX_INT_EN(port), &int_en));
	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_TLM_SPX_PW_EN(port), &pw_en));

	int_en &= i_mask;
	pw_en &= p_mask;

	switch (ntfn) {
	case rio_em_notfn_none:
		assert_false(int_en);
		assert_false(pw_en);
		break;
	case rio_em_notfn_int:
		assert_true(int_en);
		assert_false(pw_en);
		break;
	case rio_em_notfn_pw:
		assert_false(int_en);
		assert_true(pw_en);
		break;
	case rio_em_notfn_both:
		assert_true(int_en);
		assert_true(pw_en);
		break;
	case rio_em_notfn_0delta:
	case rio_em_notfn_last:
	default:
		assert_true(false);
		break;
	}
}

static void rxs_rio_em_cfg_set_get_chk_regs(rio_em_cfg_t *event,
						rio_em_notfn_ctl_t ntfn,
						rio_port_t port)
{
	bool chk_plm_en = false;
	uint32_t plm_mask = 0;

	assert_in_range(event->em_event, rio_em_f_los, rio_em_a_no_event);

	switch (event->em_detect) {
	case rio_em_detect_off:
		ntfn = rio_em_notfn_none;
		break;
	case rio_em_detect_on:
		break;
	case rio_em_detect_0delta:
	case rio_em_detect_last:
	default:
		assert_true(false);
	}

	switch (event->em_event) {
	case rio_em_f_los:
		chk_regs_los(event, port);
		chk_plm_en = true;
		plm_mask = RXS_PLM_SPX_STAT_DWNGD |
			RXS_PLM_SPX_STAT_OK_TO_UNINIT |
			RXS_PLM_SPX_STAT_DLT;
		break;
	case rio_em_f_port_err:
		chk_plm_en = true;
		plm_mask = RXS_PLM_SPX_STAT_PORT_ERR;
		break;
	case rio_em_f_2many_retx:
		chk_regs_2many_retx(event, port);
		chk_plm_en = true;
		plm_mask = RXS_PLM_SPX_STAT_MAX_DENIAL;
		break;
	case rio_em_f_2many_pna:
		chk_regs_2many_pna(event, port);
		chk_plm_en = true;
		plm_mask = RXS_PLM_SPX_STAT_MAX_DENIAL;
		break;
	case rio_em_f_err_rate:
		chk_plm_en = true;
		plm_mask = RXS_PLM_SPX_STAT_PBM_FATAL;
		break;
	case rio_em_d_ttl:
		chk_regs_ttl(event, port, ntfn);
		break;
	case rio_em_d_rte:
		chk_regs_rte(port, ntfn);
		break;
	case rio_em_d_log:
		chk_regs_log(event, ntfn);
		break;
	case rio_em_i_sig_det:
		chk_regs_sig_det(event, port);
		chk_plm_en = true;
		plm_mask = RXS_PLM_SPX_STAT_LINK_INIT;
		break;
	case rio_em_i_rst_req:
		chk_regs_rst_req(event, port);
		break;
	case rio_em_i_init_fail:
		chk_regs_init_fail(event, ntfn);
		break;
	case rio_em_a_clr_pwpnd:
		break;
	case rio_em_a_no_event:
		break;
	default:
		assert_true(false);
	}

	if (chk_plm_en) {
		chk_plm_event_enables(ntfn, plm_mask, port);
	}
}

void set_plm_imp_spec_ctl_rst(rio_port_t port, rio_em_detect_t det)
{
	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = RXS_PLM_SPX_IMP_SPEC_CTL_SELF_RST |
			RXS_PLM_SPX_IMP_SPEC_CTL_PORT_SELF_RST;

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_PLM_SPX_IMP_SPEC_CTL(port),
							&plm_imp_spec_ctl));
	if (rio_em_detect_off == det) {
		plm_imp_spec_ctl |= t_mask;
	} else {
		plm_imp_spec_ctl &= ~t_mask;
	}
	assert_int_equal(RIO_SUCCESS,
		DARRegWrite(&mock_dev_info, RXS_PLM_SPX_IMP_SPEC_CTL(port),
							plm_imp_spec_ctl));
}

typedef struct port_fail_checks_t_TAG {
	uint32_t sp_rate_en;
	uint32_t sp_rate;
	uint32_t sp_thresh;
	rio_em_cfg_t event;
} port_fail_checks_t;

// Verify various values for successfully configuring a Tsi721 event.
// Note that this test will not configure interrupt notofication if
// it is not supported.

static void rxs_rio_em_cfg_set_success_em_info_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_cfg_get_in_t get_cfg_in;
	rio_em_cfg_get_out_t get_cfg_out;
	rio_em_notfn_ctl_t set_chk_notfn = rio_em_notfn_both;
	uint32_t chk_em_info;
	rio_em_events_t get_cfg_event_req;
	rio_em_cfg_t get_cfg_event_info;
	rio_port_t port;

	rio_em_cfg_t events[] = {
		{rio_em_f_los, rio_em_detect_on, 0 * 0x253 },  // 0
		{rio_em_f_los, rio_em_detect_on, 1 * 0x253 },
		{rio_em_f_los, rio_em_detect_on, 2 * 0x253 },
		{rio_em_f_los, rio_em_detect_on, 3 * 0x253 },
		{rio_em_f_los, rio_em_detect_on, 4 * 0x253 },
		{rio_em_f_los, rio_em_detect_on, 5 * 0x253 }, // 5
		{rio_em_f_los, rio_em_detect_on, 6 * 0x253 },
		{rio_em_f_los, rio_em_detect_on, 7 * 0x253 },
		{rio_em_f_los, rio_em_detect_on, 9 * 0x253 },
		{rio_em_f_los, rio_em_detect_on, 100 * 0x253 * 1000},
		{rio_em_f_los, rio_em_detect_on, 200 * 0x253 * 1000}, // 10
		{rio_em_f_los, rio_em_detect_on, 300 * 0x253 * 1000},
		{rio_em_f_port_err, rio_em_detect_on, 0},
		{rio_em_f_2many_retx, rio_em_detect_on, 0xffff},
		{rio_em_f_2many_retx, rio_em_detect_on, 0x0001},
		{rio_em_f_2many_retx, rio_em_detect_on, 0x8888}, // 15
		{rio_em_f_2many_pna, rio_em_detect_on, 0x0001},
		{rio_em_f_2many_pna, rio_em_detect_on, 0xFFFE},
		{rio_em_f_2many_pna, rio_em_detect_on, 0xFFFF},
		{rio_em_f_2many_pna, rio_em_detect_on, 0x809E},
		{rio_em_d_log, rio_em_detect_on,
					RXS_ERR_EN_ILL_TYPE_EN | // 20
					RXS_ERR_EN_UNS_RSP_EN |
					RXS_ERR_EN_ILL_ID_EN},
		{rio_em_d_log, rio_em_detect_on, RXS_ERR_EN_ILL_TYPE_EN},
		{rio_em_d_log, rio_em_detect_on, RXS_ERR_EN_UNS_RSP_EN},
		{rio_em_d_log, rio_em_detect_on, RXS_ERR_EN_ILL_ID_EN},
		{rio_em_i_sig_det, rio_em_detect_on, 0},
		{rio_em_i_rst_req, rio_em_detect_on, 0}, // 25
		{rio_em_i_init_fail, rio_em_detect_on, 0},
		{rio_em_f_los, rio_em_detect_off,	      0},
		{rio_em_f_los, rio_em_detect_off, 1 * 0x253 * 1000},
		{rio_em_f_los, rio_em_detect_off, 2 * 0x253 * 1000},
		{rio_em_f_los, rio_em_detect_off, 3 * 0x253 * 1000}, // 30
		{rio_em_f_los, rio_em_detect_off, 4 * 0x253 * 1000},
		{rio_em_f_los, rio_em_detect_off, 5 * 0x253 * 1000},
		{rio_em_f_los, rio_em_detect_off, 6 * 0x253 * 1000},
		{rio_em_f_los, rio_em_detect_off, 7 * 0x253 * 1000},
		{rio_em_f_los, rio_em_detect_off, 9 * 0x253 * 1000}, // 35
		{rio_em_f_los, rio_em_detect_off, 100 * 0x253 * 1000},
		{rio_em_f_los, rio_em_detect_off, 200 * 0x253 * 1000},
		{rio_em_f_los, rio_em_detect_off, 300 * 0x253 * 1000},
		{rio_em_f_port_err, rio_em_detect_off, 0},
		{rio_em_f_2many_retx, rio_em_detect_off, 0x0000}, // 40
		{rio_em_f_2many_retx, rio_em_detect_off, 0x0000},
		{rio_em_f_2many_retx, rio_em_detect_off, 0x0000},
		{rio_em_f_2many_pna, rio_em_detect_off, 0x0000},
		{rio_em_f_2many_pna, rio_em_detect_off, 0x0000},
		{rio_em_f_2many_pna, rio_em_detect_off, 0x0000}, // 45
		{rio_em_f_2many_pna, rio_em_detect_off, 0x0000},
		{rio_em_d_log, rio_em_detect_off, 0},
		{rio_em_i_sig_det, rio_em_detect_off, 0},
		{rio_em_i_rst_req, rio_em_detect_off, 0},
		{rio_em_a_clr_pwpnd, rio_em_detect_off, 0}, // 50
		{rio_em_d_ttl, rio_em_detect_on, 1 * 6000},
		{rio_em_d_ttl, rio_em_detect_on, 2 * 6000},
		{rio_em_d_ttl, rio_em_detect_on, 3 * 6000},
		{rio_em_d_ttl, rio_em_detect_on, 4 * 6000},
		{rio_em_d_ttl, rio_em_detect_on, 5 * 6000}, // 55
		{rio_em_d_ttl, rio_em_detect_on, 0x0800 * 6000},
		{rio_em_d_ttl, rio_em_detect_on, 0x1000 * 6000},
		{rio_em_d_ttl, rio_em_detect_on, 0x2000 * 6000},
		{rio_em_d_ttl, rio_em_detect_on, 0x4000 * 6000},
		{rio_em_d_ttl, rio_em_detect_off, 0}, // 60
		{rio_em_d_rte, rio_em_detect_on, 0},
		{rio_em_d_rte, rio_em_detect_off, 0},
		{rio_em_a_no_event, rio_em_detect_off, 0},
		{rio_em_i_init_fail, rio_em_detect_off, 0},
	};

	unsigned int num_events = sizeof(events) / sizeof(events[0]);
	unsigned int i;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	// Power up and enable all ports...
	if (!l_st->real_hw) {
		set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);
	}

	// For each event in the prodigous list above
	for (i = 0; i < num_events; i++) {
		if (l_st->real_hw) {
			port = l_st->conn_port;
		} else {
			port = i % NUM_RXS_PORTS(&mock_dev_info);
		}
		if (DEBUG_PRINTF) {
			printf(
			"\nevent idx %d event %d port %d onoff %d info 0x%x\n",
			i, events[i].em_event, port,
			events[i].em_detect,
			events[i].em_info);
		}
		if (rio_em_i_rst_req == events[i].em_event) {
			// If we're testing disabling the Reset Request
			// event, do the real disable since this events
			// detection is actually controlled by Port Config
			// functionality.

			set_plm_imp_spec_ctl_rst(port, events[i].em_detect);
		}

		// Set the event configuration specified
		set_chk_notfn = rio_em_notfn_both;
		set_chk_notfn = (events[i].em_detect == rio_em_detect_on) ?
				set_chk_notfn : rio_em_notfn_none;

		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = port;
		set_cfg_in.notfn = (events[i].em_detect == rio_em_detect_on) ?
						rio_em_notfn_both :
						rio_em_notfn_none;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &events[i];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		// Check the returned configuration
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(set_chk_notfn, set_cfg_out.notfn);

		// Check the register values for the event
		rxs_rio_em_cfg_set_get_chk_regs(&events[i], set_chk_notfn, port);

		// Get the event configuration
		get_cfg_in.port_num = port;
		get_cfg_in.num_events = 1;
		get_cfg_in.event_list = &get_cfg_event_req;
		get_cfg_event_req = events[i].em_event;
		get_cfg_in.events = &get_cfg_event_info;

		get_cfg_out.imp_rc = 0xFFFFFFFF;
		get_cfg_out.fail_idx = 0xFF;
		get_cfg_out.notfn = (rio_em_notfn_ctl_t)0xFF;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_cfg_get(&mock_dev_info,
						&get_cfg_in, &get_cfg_out));
		assert_int_equal(0, get_cfg_out.imp_rc);
		assert_int_equal(rio_em_last, get_cfg_out.fail_idx);
		assert_int_equal(set_chk_notfn, get_cfg_out.notfn);

		// Check the returned configuration
		if (rio_em_detect_on == events[i].em_detect) {
			chk_em_info = events[i].em_info;
		} else {
			chk_em_info = 0;
		}
		assert_int_equal(events[i].em_event,
			get_cfg_in.events[0].em_event);
		assert_int_equal(events[i].em_detect,
			get_cfg_in.events[0].em_detect);
		assert_int_equal(chk_em_info, get_cfg_in.events[0].em_info);
	}
	(void)state;
}
// Check that some events that should be ignored can be successfully
// configured.

static void rxs_rio_em_cfg_set_ignore_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_cfg_get_in_t get_cfg_in;
	rio_em_cfg_get_out_t get_cfg_out;
	rio_em_notfn_ctl_t set_chk_notfn = rio_em_notfn_both;
	rio_em_events_t get_cfg_event_req;
	rio_em_cfg_t get_cfg_event_info;

	rio_em_cfg_t events[] = {
		{rio_em_a_no_event, rio_em_detect_on, 0},
		{rio_em_a_no_event, rio_em_detect_off, 0},
	};

	unsigned int num_events = sizeof(events) / sizeof(events[0]);
	unsigned int i;
	rio_port_t port = 0;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	assert_in_range(num_events, 0, NUM_RXS_PORTS(&mock_dev_info) - 1);

	// Power up and enable all ports...
	if (!l_st->real_hw) {
		set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);
	}

	for (i = 0; i < num_events; i++) {
		if (DEBUG_PRINTF) {
			printf("\nevent idx %d\n",  i);
		}
		if (l_st->real_hw) {
			port = l_st->conn_port;
		} else {
			port = i;
		}
		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = port;
		set_cfg_in.notfn = rio_em_notfn_both;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &events[i];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(set_chk_notfn, set_cfg_out.notfn);

		get_cfg_in.port_num = port;
		get_cfg_in.num_events = 1;
		get_cfg_event_req = events[i].em_event;
		get_cfg_in.event_list = &get_cfg_event_req;
		get_cfg_in.events = &get_cfg_event_info;

		get_cfg_out.imp_rc = 0xFFFFFFFF;
		get_cfg_out.fail_idx = 0xFF;
		get_cfg_out.notfn = (rio_em_notfn_ctl_t)0xFF;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_cfg_get(&mock_dev_info,
						&get_cfg_in, &get_cfg_out));
		assert_int_equal(0, get_cfg_out.imp_rc);
		assert_int_equal(rio_em_last, get_cfg_out.fail_idx);
		assert_int_equal(set_chk_notfn, get_cfg_out.notfn);

		assert_int_equal(events[i].em_event,
				get_cfg_in.events[0].em_event);
		assert_int_equal(rio_em_detect_off,
				get_cfg_in.events[0].em_detect);
		assert_int_equal(events[i].em_info,
				get_cfg_in.events[0].em_info);
	}
	(void)state;
}

typedef struct err_rate_checks_t_TAG {
	uint32_t sp_rate_en;
	uint32_t sp_rate;
	uint32_t sp_thresh;
	rio_em_cfg_t event;
} err_rate_checks_t;

// Test various illegal (generally 0) em_info parameters are detected
// and reported.

static void rxs_rio_em_cfg_set_fail_em_info_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;

	rio_em_cfg_t events[] = {
		{rio_em_f_2many_retx, rio_em_detect_on, 0},
		{rio_em_f_2many_pna, rio_em_detect_on, 0},
		{rio_em_d_log, rio_em_detect_on, 0},
		{rio_em_d_ttl, rio_em_detect_on, 0},
		{rio_em_last, rio_em_detect_on, 0},
	};
	rio_em_cfg_t pass_events[2];
	rio_port_t port;
	rio_port_t chk_port;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	unsigned int num_events = sizeof(events) / sizeof(events[0]);
	unsigned int i;

	// Power up and enable all ports...
	if (!l_st->real_hw) {
		set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);
	}

	pass_events[0].em_event = rio_em_a_no_event;
	pass_events[0].em_detect = rio_em_detect_0delta;
	pass_events[0].em_info = 0;

	for (i = 0; i < num_events; i++) {
		if (DEBUG_PRINTF) {
			printf("\nevent idx %d\n",  i);
		}
		if (l_st->real_hw) {
			port = l_st->conn_port;
		} else {
			port = i;
		}
		if ((rio_em_d_log == events[i].em_event) ||
		    (rio_em_d_ttl == events[i].em_event)) {
			chk_port = RIO_ALL_PORTS;
		} else {
			chk_port = port;
		}
		memcpy(&pass_events[1], &events[i], sizeof(pass_events[0]));
		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = port;
		set_cfg_in.notfn = rio_em_notfn_both;
		set_cfg_in.num_events = 2;
		set_cfg_in.events = &pass_events[0];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xFF;

		assert_int_not_equal(RIO_SUCCESS,
				rxs_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_not_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(chk_port, set_cfg_out.fail_port_num);
		assert_int_equal(1, set_cfg_out.fail_idx);
	}
	(void)state;
}

typedef struct em_info_diff_checks_t_TAG {
	uint32_t chk_info;
	rio_em_cfg_t event;
} em_info_diff_checks_t;

// Test various illegal (generally 0) em_info parameters are detected
// and reported.

static void rxs_rio_em_cfg_set_roundup_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_cfg_get_in_t get_cfg_in;
	rio_em_cfg_get_out_t get_cfg_out;
	rio_em_notfn_ctl_t set_chk_notfn = rio_em_notfn_both;
	rio_em_events_t get_cfg_event_req;
	rio_em_cfg_t get_cfg_event_info;

	em_info_diff_checks_t events[] = {
		{0x0000FFFF, {rio_em_f_2many_retx, rio_em_detect_on, 0x10000}},
		{0x0000FFFF, {rio_em_f_2many_retx, rio_em_detect_on, 0xFFFFF}},
		{0x0000FFFF, {rio_em_f_2many_pna, rio_em_detect_on, 0x10000}},
		{0x0000FFFF, {rio_em_f_2many_pna, rio_em_detect_on, 0xFFFFF}},
		{0x253, {rio_em_f_los, rio_em_detect_on, 1}},
		{0x253, {rio_em_f_los, rio_em_detect_on, 0x252}},
		{0x253, {rio_em_f_los, rio_em_detect_on, 0x253}},
		{0x4A6, {rio_em_f_los, rio_em_detect_on, 0x254}},
		{0x4A6, {rio_em_f_los, rio_em_detect_on, 0x4a5}},
		{0x4A6, {rio_em_f_los, rio_em_detect_on, 0x4a6}},
		{0x6F9, {rio_em_f_los, rio_em_detect_on, 0x4a7}}, // 10
		{0xFFFFFF00, {rio_em_f_los, rio_em_detect_on, 0xFFFFFF00}},
		{0xFFFFFFFF, {rio_em_f_los, rio_em_detect_on, 0xFFFFFF01}},
		{0xFFFFFFFF, {rio_em_f_los, rio_em_detect_on, 0xFFFFFFFF}},
		{6000, {rio_em_d_ttl, rio_em_detect_on, 1}},
		{6000, {rio_em_d_ttl, rio_em_detect_on, 5999}},
		{6000, {rio_em_d_ttl, rio_em_detect_on, 6000}},
		{12000, {rio_em_d_ttl, rio_em_detect_on, 6001}},
		{0x4000 * 6000, {rio_em_d_ttl, rio_em_detect_on, 0x20000000}},
		{0x4000 * 6000, {rio_em_d_ttl, rio_em_detect_on, 0xFFFFFFFF}},
	};

	unsigned int num_events = sizeof(events) / sizeof(events[0]);
	unsigned int i;
	rio_port_t port;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	// Power up and enable all ports...
	if (!l_st->real_hw) {
		set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);
	}

	for (i = 0; i < num_events; i++) {
		if (l_st->real_hw) {
			port = l_st->conn_port;
		} else {
			port = i & NUM_RXS_PORTS(&mock_dev_info);;
		}
		if (DEBUG_PRINTF) {
			printf("\nevent idx %d port %d\n",  i, port);
		}
		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = port;
		set_cfg_in.notfn = rio_em_notfn_both;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &events[i].event;

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xFF;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(set_chk_notfn, set_cfg_out.notfn);

		events[i].event.em_info = events[i].chk_info;
		rxs_rio_em_cfg_set_get_chk_regs(&events[i].event,
				set_chk_notfn, port);

		get_cfg_in.port_num = port;
		get_cfg_in.num_events = 1;
		get_cfg_in.event_list = &get_cfg_event_req;
		get_cfg_event_req = events[i].event.em_event;
		get_cfg_in.events = &get_cfg_event_info;
		get_cfg_out.imp_rc = 0xFFFFFFFF;
		get_cfg_out.fail_idx = 0xFF;
		get_cfg_out.notfn = (rio_em_notfn_ctl_t)0xFF;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_cfg_get(&mock_dev_info,
						&get_cfg_in, &get_cfg_out));
		assert_int_equal(0, get_cfg_out.imp_rc);
		assert_int_equal(rio_em_last, get_cfg_out.fail_idx);
		assert_int_equal(set_chk_notfn, get_cfg_out.notfn);

		assert_int_equal(events[i].event.em_event,
				get_cfg_in.events[0].em_event);
		assert_int_equal(events[i].event.em_detect,
				get_cfg_in.events[0].em_detect);
		assert_int_equal(events[i].event.em_info,
				get_cfg_in.events[0].em_info);
	}
	(void)state;
}

// Test bad parameter values are detected and reported.

static void rxs_rio_em_cfg_get_bad_parms_test(void **state)
{
	rio_em_cfg_get_in_t p_in;
	rio_em_cfg_get_out_t p_out;
	rio_em_events_t get_cfg_event_req;
	rio_em_cfg_t get_cfg_event_info;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	// Power up and enable all ports...
	if (!l_st->real_hw) {
		set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);
	}

	// Bad port number
	p_in.port_num = NUM_RXS_PORTS(&mock_dev_info) + 1;
	p_in.num_events = 1;
	p_in.event_list = &get_cfg_event_req;
	get_cfg_event_req = rio_em_f_2many_pna;
	p_in.events = &get_cfg_event_info;

	p_out.imp_rc = 0;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_cfg_get(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(rio_em_last, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	// Bad number of events
	p_in.port_num = 0;
	p_in.num_events = 0;

	p_out.imp_rc = 0;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_cfg_get(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(rio_em_last, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	// NULL event list pointer
	p_in.num_events = 1;
	p_in.event_list = NULL;

	p_out.imp_rc = 0;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_cfg_get(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(rio_em_last, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	// NULL events pointer
	p_in.num_events = 1;
	p_in.event_list = &get_cfg_event_req;
	p_in.events = NULL;

	p_out.imp_rc = 0;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_cfg_get(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(rio_em_last, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	// Bad event in list
	p_in.num_events = 1;
	p_in.event_list = &get_cfg_event_req;
	get_cfg_event_req = rio_em_last;
	p_in.events = &get_cfg_event_info;

	p_out.imp_rc = 0;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_cfg_get(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(0, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	(void)state;
}

// Test bad parameter values are detected and reported.

static void rxs_rio_em_cfg_set_bad_parms_test(void **state)
{
	rio_em_cfg_set_in_t p_in;
	rio_em_cfg_set_out_t p_out;
	rio_em_cfg_t events = {rio_em_f_2many_pna, rio_em_detect_on, 0x0100};

	// Bad number of ports
	p_in.ptl.num_ports = NUM_RXS_PORTS(&mock_dev_info) + 1;
	p_in.ptl.pnums[0] = 0;
	p_in.ptl.pnums[1] = 0;
	p_in.num_events = 1;
	p_in.events = &events;

	p_out.imp_rc = 0;
	p_out.fail_port_num = 0x99;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_cfg_set(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(RIO_ALL_PORTS, p_out.fail_port_num);
	assert_int_equal(rio_em_last, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	// Bad pnum
	p_in.ptl.num_ports = 1;
	p_in.ptl.pnums[0] = NUM_RXS_PORTS(&mock_dev_info);
	p_in.num_events = 1;
	p_in.events = &events;

	p_out.imp_rc = 0;
	p_out.fail_port_num = 0x99;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_cfg_set(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(RIO_ALL_PORTS, p_out.fail_port_num);
	assert_int_equal(rio_em_last, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	// Bad notification value
	p_in.ptl.num_ports = 1;
	p_in.ptl.pnums[0] = 0;
	p_in.notfn = rio_em_notfn_last;
	p_in.num_events = 1;
	p_in.events = &events;

	p_out.imp_rc = 0;
	p_out.fail_port_num = 0x99;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_cfg_set(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(RIO_ALL_PORTS, p_out.imp_rc);
	assert_int_equal(RIO_ALL_PORTS, p_out.fail_port_num);
	assert_int_equal(rio_em_last, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	// Bad number of events
	p_in.ptl.num_ports = 1;
	p_in.ptl.pnums[0] = 0;
	p_in.notfn = rio_em_notfn_none;
	p_in.num_events = 0;
	p_in.events = &events;

	p_out.imp_rc = 0;
	p_out.fail_port_num = 0x99;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_cfg_set(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(RIO_ALL_PORTS, p_out.fail_port_num);
	assert_int_equal(rio_em_last, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	// Bad event pointer
	p_in.ptl.num_ports = 1;
	p_in.ptl.pnums[0] = 0;
	p_in.notfn = rio_em_notfn_last;
	p_in.num_events = 1;
	p_in.events = NULL;

	p_out.imp_rc = 0;
	p_out.fail_port_num = 0x99;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_cfg_set(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(RIO_ALL_PORTS, p_out.fail_port_num);
	assert_int_equal(rio_em_last, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	(void)state;
}

static void rxs_rio_em_dev_rpt_ctl_chk_regs_port(rio_em_notfn_ctl_t chk_notfn,
					rio_port_t port)
{
	const uint32_t p_pw_mask = RXS_SPX_ERR_STAT_PORT_W_DIS;
	const uint32_t p_int_mask = RXS_PLM_SPX_ALL_INT_EN_IRQ_EN;
	uint32_t err_st;
	uint32_t int_en;

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_SPX_ERR_STAT(port), &err_st));
	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_PLM_SPX_ALL_INT_EN(port),
								&int_en));
	err_st &= p_pw_mask;
	int_en &= p_int_mask;
	switch (chk_notfn) {
	case rio_em_notfn_none:
		assert_int_equal(0, int_en);
		assert_int_equal(p_pw_mask, err_st);
		break;

	case rio_em_notfn_int:
		assert_int_equal(p_int_mask, int_en);
		assert_int_equal(p_pw_mask, err_st);
		break;

	case rio_em_notfn_pw:
		assert_int_equal(0, int_en);
		assert_int_equal(0, err_st);
		break;

	case rio_em_notfn_both:
		assert_int_equal(p_int_mask, int_en);
		assert_int_equal(0, err_st);
		break;

	default:
		assert_true(false);
		break;
	}
}

static void rxs_rio_em_dev_rpt_ctl_chk_regs(void **state,
		rio_em_notfn_ctl_t chk_notfn,
		struct DAR_ptl *ptl)
{
	const uint32_t pw_mask = RXS_PW_TRAN_CTL_PW_DIS;
	const uint32_t int_mask = RXS_EM_DEV_INT_EN_INT_EN;
	struct DAR_ptl good_ptl;
	unsigned int port_idx;
	rxs_rpt_ctl_regs_t regs;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	assert_int_equal(RIO_SUCCESS,
		DARrioGetPortList(&mock_dev_info, ptl, &good_ptl));
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_em_dev_rpt_ctl_reg_read(&mock_dev_info, &regs));

	switch (chk_notfn) {
	case rio_em_notfn_none:
		assert_int_equal(0, regs.dev_int_en);
		assert_int_equal(pw_mask, pw_mask & regs.pw_trans_dis);
		break;

	case rio_em_notfn_int:
		assert_int_equal(int_mask, int_mask & regs.dev_int_en);
		assert_int_equal(pw_mask, pw_mask & regs.pw_trans_dis);
		break;

	case rio_em_notfn_pw:
		assert_int_equal(0, regs.dev_int_en);
		assert_int_equal(0, regs.pw_trans_dis);
		break;

	case rio_em_notfn_both:
		assert_int_equal(int_mask, int_mask & regs.dev_int_en);
		assert_int_equal(0, regs.pw_trans_dis);
		break;

	default:
		assert_true(false);
		break;
	}

	// Cannot check per-port registers on real hardware.
	if (l_st->real_hw) {
		return;
	}

	for (port_idx = 0; port_idx < good_ptl.num_ports; port_idx++) {
		if (DEBUG_PRINTF) {
			printf("\nNotfn %d Port %d\n", chk_notfn,
						good_ptl.pnums[port_idx]);
		}
		rxs_rio_em_dev_rpt_ctl_chk_regs_port(chk_notfn,
						good_ptl.pnums[port_idx]);
	}
}

// Test that port-write/interrupt reporting control is working.
// Updates status for all ports.

static void rxs_rio_em_dev_rpt_ctl_success_test(void **state)
{
	rio_em_dev_rpt_ctl_in_t in_p;
	rio_em_dev_rpt_ctl_out_t out_p;
	rio_em_notfn_ctl_t notfn, chk_notfn;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	// Power up and enable all ports...
	if (!l_st->real_hw) {
		set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);
	}

	in_p.ptl.num_ports = RIO_ALL_PORTS;
	for (notfn = rio_em_notfn_none; notfn < rio_em_notfn_0delta; notfn =
			(rio_em_notfn_ctl_t)(notfn + 1)) {
		if (DEBUG_PRINTF) {
			printf("\nnotfn %d\n", notfn);
		}
		in_p.notfn = notfn;
		chk_notfn = notfn;
		out_p.imp_rc = 0xFFFFFFFF;
		out_p.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_em_dev_rpt_ctl(&mock_dev_info, &in_p, &out_p));

		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(chk_notfn, out_p.notfn);

		rxs_rio_em_dev_rpt_ctl_chk_regs(state, chk_notfn, &in_p.ptl);

		// Repeat check with 0delta
		if (DEBUG_PRINTF) {
			printf("\nnotfn %d 0delta\n", notfn);
		}
		in_p.notfn = rio_em_notfn_0delta;
		out_p.imp_rc = 0xFFFFFFFF;
		out_p.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_dev_rpt_ctl(&mock_dev_info, &in_p,
						&out_p));
		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(chk_notfn, out_p.notfn);

		rxs_rio_em_dev_rpt_ctl_chk_regs(state, chk_notfn, &in_p.ptl);
	}
	(void)state;
}

// Test that port-write/interrupt reporting control is working.
// Updates status for half the ports.
// Checks that other half is not changed.

static void rxs_rio_em_dev_rpt_ctl_oddport_test(void **state)
{
	rio_em_dev_rpt_ctl_in_t in_p;
	rio_em_dev_rpt_ctl_out_t out_p;
	rio_em_notfn_ctl_t notfn, chk_notfn;
	rio_port_t port;
	unsigned int port_idx;
	struct DAR_ptl other_ptl;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Power up and enable all ports...
	set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);

	// Set all ports to "both" int and pw notification
	in_p.ptl.num_ports = RIO_ALL_PORTS;
	in_p.notfn = rio_em_notfn_both;
	out_p.imp_rc = 0xFFFFFFFF;
	out_p.notfn = (rio_em_notfn_ctl_t)0xF;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_em_dev_rpt_ctl(&mock_dev_info, &in_p, &out_p));

	assert_int_equal(0, out_p.imp_rc);
	assert_int_equal(rio_em_notfn_both, out_p.notfn);

	// Divide ports in half, some are updated, others not so much...
	in_p.ptl.num_ports = 0;
	other_ptl.num_ports = 0;
	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		if (port & 1) {
			in_p.ptl.pnums[in_p.ptl.num_ports++] = port;
		} else {
			other_ptl.pnums[other_ptl.num_ports++] = port;
		}
	}

	for (notfn = rio_em_notfn_none; notfn < rio_em_notfn_0delta; notfn =
			(rio_em_notfn_ctl_t)(notfn + 1)) {
		in_p.notfn = notfn;
		chk_notfn = notfn;
		out_p.imp_rc = 0xFFFFFFFF;
		out_p.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_dev_rpt_ctl(&mock_dev_info, &in_p,
						&out_p));

		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(chk_notfn, out_p.notfn);

		rxs_rio_em_dev_rpt_ctl_chk_regs(state, chk_notfn, &in_p.ptl);

		for (port_idx = 0; port_idx < other_ptl.num_ports; port_idx++) {
			rxs_rio_em_dev_rpt_ctl_chk_regs_port(rio_em_notfn_both,
					other_ptl.pnums[port_idx]);
		}
	}
	(void)state;
}

// Test bad parameter values are detected and reported.

static void rxs_rio_em_dev_rpt_ctl_bad_parms_test(void **state)
{
	rio_em_dev_rpt_ctl_in_t in_p;
	rio_em_dev_rpt_ctl_out_t out_p;

	// Bad number of ports
	in_p.ptl.num_ports = NUM_RXS_PORTS(&mock_dev_info) + 1;
	in_p.notfn = rio_em_notfn_last;

	out_p.imp_rc = 0;
	out_p.notfn = rio_em_notfn_last;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_dev_rpt_ctl(&mock_dev_info, &in_p, &out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(rio_em_notfn_last, out_p.notfn);

	// Bad port number
	in_p.ptl.num_ports = 1;
	in_p.ptl.pnums[0] = NUM_RXS_PORTS(&mock_dev_info);
	in_p.notfn = rio_em_notfn_none;

	out_p.imp_rc = 0;
	out_p.notfn = rio_em_notfn_last;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_dev_rpt_ctl(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(rio_em_notfn_last, out_p.notfn);

	// Bad notification type
	in_p.ptl.num_ports = RIO_ALL_PORTS;
	in_p.notfn = rio_em_notfn_last;

	out_p.imp_rc = 0;
	out_p.notfn = rio_em_notfn_last;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_dev_rpt_ctl(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(rio_em_notfn_last, out_p.notfn);

	// Another bad notification type
	in_p.ptl.num_ports = RIO_ALL_PORTS;
	in_p.notfn = (rio_em_notfn_ctl_t)((uint8_t)rio_em_notfn_last + 1);

	out_p.imp_rc = 0;
	out_p.notfn = rio_em_notfn_last;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_dev_rpt_ctl(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(rio_em_notfn_last, out_p.notfn);

	(void)state;
}

// Test that a port-write with no events is parsed correctly.

static void rxs_rio_em_parse_pw_no_events_test(void **state)
{
	rio_em_parse_pw_in_t in_p;
	rio_em_parse_pw_out_t out_p;
	rio_em_event_n_loc_t events[(int)rio_em_last];

	memset(in_p.pw, 0, sizeof(in_p.pw));
	in_p.num_events = (int)rio_em_last;
	memset(events, 0, sizeof(events));
	in_p.events = events;

	out_p.imp_rc = 0xFFFFFFFF;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_equal(RIO_SUCCESS,
			rxs_rio_em_parse_pw(&mock_dev_info, &in_p, &out_p));

	assert_int_equal(0, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	(void)state;
}

typedef struct parse_pw_info_t_TAG {
	uint32_t pw[4];
	rio_em_event_n_loc_t event;
} parse_pw_info_t;

// Test that a port-write with one event is parsed correctly.

static void rxs_rio_em_parse_pw_all_events_test(void **state)
{
	unsigned int i;
	rio_em_parse_pw_in_t in_p;
	rio_em_parse_pw_out_t out_p;
	const unsigned int max_events = 50;
	rio_em_event_n_loc_t events[max_events];
	parse_pw_info_t pws[] = {
		{{	0,
			RXS_SPX_ERR_DET_DLT | RXS_SPX_ERR_DET_OK_TO_UNINIT,
			1 | RXS_PW_OK_TO_UNINIT | RXS_PW_DLT | RXS_PW_DWNGD,
			0},
			{1, rio_em_f_los}},
		{{	0,
			0,
			2 | RXS_PW_PORT_ERR,
			0},
			{2, rio_em_f_port_err}},
		{{	0,
			0,
			3 | RXS_PW_MAX_DENIAL,
			0},
			{3, rio_em_f_2many_retx}},
		{{	0,
			0,
			4 | RXS_PW_PBM_FATAL,
			0},
			{4, rio_em_f_err_rate}},
		{{	0,
			0,
			5 | RXS_PW_PBM_PW,
			0},
			{5, rio_em_d_ttl}},
		{{	0,
			0,
			6 | RXS_PW_TLM_PW,
			0},
			{6, rio_em_d_rte}},
		{{	0,
			0,
			7,
			RXS_ERR_DET_ILL_TYPE |
			RXS_ERR_DET_UNS_RSP |
			RXS_ERR_DET_ILL_ID},
			{RIO_ALL_PORTS, rio_em_d_log}},
		{{	0,
			RXS_SPX_ERR_DET_LINK_INIT,
			8 | RXS_PW_LINK_INIT,
			0},
			{8, rio_em_i_sig_det}},
		{{	0,
			0,
			9 | RXS_PW_RST_REQ | RXS_PW_PRST_REQ,
			0},
			{9, rio_em_i_rst_req}},
		{{	0,
			0,
			10 | RXS_PW_DEV_RCS,
			0},
			{RIO_ALL_PORTS, rio_em_i_rst_req}},
		{{	0,
			0,
			23 | RXS_PW_INIT_FAIL,
			0},
			{RIO_ALL_PORTS, rio_em_i_init_fail}},
	};
	unsigned int num_pws = sizeof(pws) / sizeof(pws[0]);

	// Checking values for "all events" test
	unsigned int num_chk_ev = 0;
	rio_em_event_n_loc_t chk_ev[max_events];
	bool found_it;
	unsigned int j;
	const uint32_t IMP_SPEC_IDX = RIO_EMHS_PW_IMP_SPEC_IDX;
	const uint32_t IMP_SPEC_0_PORT = ~RIO_EMHS_PW_IMP_SPEC_PORT;

	// Make sure the random value chosen for event size is correct.
	assert_in_range(num_pws + 2, 0, max_events);

	// Try each event individually...
	for (i = 0; i < num_pws; i++) {
		if (DEBUG_PRINTF) {
			printf("\ni %d\n", i);
		}
		memcpy(in_p.pw, pws[i].pw, sizeof(in_p.pw));
		in_p.num_events = max_events;
		memset(events, 0, sizeof(events));
		in_p.events = events;

		out_p.imp_rc = 0xFFFFFFFF;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_em_parse_pw(&mock_dev_info, &in_p, &out_p));

		assert_int_equal(0, out_p.imp_rc);
		if (rio_em_f_2many_retx == pws[i].event.event) {
			// Creation of a 2many_pna event also creates an
			// 2many_retx event.
			assert_int_equal(2, out_p.num_events);
			assert_int_equal(pws[i].event.event,
							in_p.events[0].event);
			assert_int_equal(pws[i].event.port_num,
						in_p.events[0].port_num);
			assert_int_equal(rio_em_f_2many_pna,
						in_p.events[1].event);
			// Yes, index for events port_num should be 'i', not 1
			assert_int_equal(pws[i].event.port_num,
						in_p.events[1].port_num);
			assert_false(out_p.too_many);
			assert_false(out_p.other_events);
		} else {
			assert_int_equal(1, out_p.num_events);
			assert_int_equal(pws[i].event.event, in_p.events[0].event);
			assert_int_equal(pws[i].event.port_num,
					in_p.events[0].port_num);
			assert_false(out_p.too_many);
			assert_false(out_p.other_events);
			continue;
		}
	}

	// Formulate port-write with all possible events...
	memcpy(in_p.pw, pws[0].pw, sizeof(in_p.pw));
	memcpy(&chk_ev[num_chk_ev], &pws[0].event, sizeof(chk_ev[0]));
	num_chk_ev++;
	for (i = 1; i < num_pws; i++) {
		for (j = 0; j < RIO_EMHS_PW_WORDS; j++) {
			if (IMP_SPEC_IDX == j) {
				in_p.pw[j] |= pws[i].pw[j] & IMP_SPEC_0_PORT;
			} else {
				in_p.pw[j] |= pws[i].pw[j];
			}
		}
		memcpy(&chk_ev[num_chk_ev], &pws[i].event, sizeof(chk_ev[i]));
		if (pws[i].event.port_num != RIO_ALL_PORTS) {
			chk_ev[num_chk_ev].port_num = chk_ev[0].port_num;
		}
		num_chk_ev++;
		if (rio_em_f_2many_retx == pws[i].event.event) {
			chk_ev[num_chk_ev].port_num = pws[0].event.port_num;
			chk_ev[num_chk_ev].event = rio_em_f_2many_pna;
			num_chk_ev++;
		}
	}

	// Parse Murphy's own port write
	in_p.num_events = max_events;
	memset(events, 0, sizeof(events));
	in_p.events = events;

	out_p.imp_rc = 0xFFFFFFFF;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_em_parse_pw(&mock_dev_info, &in_p, &out_p));

	assert_int_equal(0, out_p.imp_rc);

	assert_int_equal(out_p.num_events, num_chk_ev);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	// Check that every chk_ev event exists in the events list
	for (i = 0; i < num_chk_ev; i++) {
		if (DEBUG_PRINTF) {
			printf("\ni %d event %d port %d\n",
				i, chk_ev[i].event,chk_ev[i].port_num);
		}
		found_it = false;
		for (j = 0; !found_it && (j < out_p.num_events); j++) {
			found_it = ((chk_ev[i].port_num == events[j].port_num)
				&& (chk_ev[i].event == events[j].event));
			if (DEBUG_PRINTF) {
				if (found_it) {
					printf("\n	j %d\n", j);
				}
			}
		}
		assert_true(found_it);
	}

	(void)state;
}

// Test that a port-write with other, unsupported events is parsed correctly

static void rxs_rio_em_parse_pw_oth_events_test(void **state)
{
	rio_em_parse_pw_in_t in_p;
	rio_em_parse_pw_out_t out_p;
	rio_em_event_n_loc_t events[(int)rio_em_last];
	parse_pw_info_t pws[] = {
	{{0, 0, RXS_PW_ZERO, 0}, {0, rio_em_f_los}},
	{{0, 0, RXS_PW_MULTIPORT, 0}, {0, rio_em_f_los}},
	{{0, 0, RXS_PW_FAB_OR_DEL, 0}, {0, rio_em_f_los}},
	{{0, 0, RXS_PW_DEV_ECC, 0}, {0, rio_em_f_los}},
	{{0, 0, RXS_PW_EL_INTA, 0}, {0, rio_em_f_los}},
	{{0, 0, RXS_PW_EL_INTB, 0}, {0, rio_em_f_los}},
	{{0, 0, RXS_PW_II_CHG_0, 0}, {0, rio_em_f_los}},
	{{0, 0, RXS_PW_II_CHG_1, 0}, {0, rio_em_f_los}},
	{{0, 0, RXS_PW_II_CHG_2, 0}, {0, rio_em_f_los}},
	{{0, 0, RXS_PW_II_CHG_3, 0}, {0, rio_em_f_los}},
	{{0, 0, RXS_PW_PCAP, 0}, {0, rio_em_f_los}},
	};
	unsigned int num_pws = sizeof(pws) / sizeof(pws[0]);
	unsigned int i;

	// Should not have any events, but should have "other" events
	for (i = 0; i < num_pws; i++) {
		if (DEBUG_PRINTF) {
			printf("\ni %d\n", i);
		}
		memcpy(in_p.pw, pws[i].pw, sizeof(in_p.pw));
		in_p.num_events = (int)rio_em_last;
		memset(events, 0, sizeof(events));
		in_p.events = events;

		out_p.imp_rc = 0xFFFFFFFF;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_parse_pw(&mock_dev_info, &in_p,
						&out_p));

		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(0, out_p.num_events);
		assert_int_equal(pws[i].event.event, in_p.events[0].event);
		assert_int_equal(pws[i].event.port_num,
				in_p.events[0].port_num);
		assert_false(out_p.too_many);
		assert_true(out_p.other_events);
	}

	(void)state;
}

// Test bad parameter values are detected and reported.

static void rxs_rio_em_parse_pw_bad_parms_test(void **state)
{
	rio_em_parse_pw_in_t in_p;
	rio_em_parse_pw_out_t out_p;
	rio_em_event_n_loc_t events[(int)rio_em_last];

	// Null "events" pointer
	memset(in_p.pw, 0, sizeof(in_p.pw));
	in_p.num_events = (uint8_t)rio_em_last;
	memset(events, 0, sizeof(events));
	in_p.events = NULL;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_parse_pw(&mock_dev_info, &in_p, &out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	// No events allowed
	in_p.num_events = 0;
	in_p.events = events;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_parse_pw(&mock_dev_info, &in_p, &out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	// Illegal port value in port write
	in_p.num_events = (uint8_t)rio_em_last;
	in_p.pw[RIO_EM_PW_IMP_SPEC_IDX] = RIO_EM_PW_IMP_SPEC_PORT_MASK;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_parse_pw(&mock_dev_info, &in_p, &out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	(void)state;
}

// Test that bad parameters are detected and reported
// Also tests that "no_event" is ignored when creating events.

static void rxs_rio_em_create_events_bad_parms_test(void **state)
{
	rio_em_create_events_in_t in_p;
	rio_em_create_events_out_t out_p;
	rio_em_event_n_loc_t events[(uint8_t)rio_em_last];

	// Bad number of events
	in_p.num_events = 0;
	in_p.events = events;

	out_p.imp_rc = 0;
	out_p.failure_idx = 0xff;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_create_events(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0xFF, out_p.failure_idx);

	// NULL event pointer
	in_p.num_events = 1;
	in_p.events = NULL;

	out_p.imp_rc = 0;
	out_p.failure_idx = 0xff;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_create_events(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0xFF, out_p.failure_idx);

	// Illegal port in event
	in_p.num_events = 2;
	in_p.events = events;
	events[0].port_num = 0;
	events[0].event = rio_em_a_no_event;
	events[1].port_num = NUM_RXS_PORTS(&mock_dev_info);
	events[1].event = rio_em_f_los;

	out_p.imp_rc = 0;
	out_p.failure_idx = 0xff;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_create_events(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(1, out_p.failure_idx);

	// Illegal event type in event
	in_p.num_events = 2;
	in_p.events = events;
	events[0].port_num = 0;
	events[0].event = rio_em_a_no_event;
	events[1].port_num = 1;
	events[1].event = rio_em_last;

	out_p.imp_rc = 0;
	out_p.failure_idx = 0xff;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_create_events(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(1, out_p.failure_idx);

	(void)state;
}

// Test that each individual event can be created.
//
// NOte: This does not check "top level" interrupt/port-write status,
//       as this would assume that the event has been enabled.
//	Event enable/top level interrupt testing is done by the
//	em_int/pw_status tests.

typedef struct addr_and_value_t_TAG {
	uint32_t addr;
	uint32_t val;
} addr_and_value_t;

typedef struct offset_value_event_t_TAG {
	rio_em_event_n_loc_t event;
	addr_and_value_t chk[2];
} offset_value_event_t;

static void rxs_rio_em_create_events_success_test(void **state)
{
	rio_em_create_events_in_t in_p;
	rio_em_create_events_out_t out_p;
	rio_em_event_n_loc_t events[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;

	offset_value_event_t tests[] = {
		{{0, rio_em_f_los},
			{{RXS_PLM_SPX_STAT(0),
				RXS_PLM_SPX_STAT_OK_TO_UNINIT |
				RXS_PLM_SPX_STAT_DLT |
				RXS_PLM_SPX_STAT_DWNGD},
			{RXS_SPX_ERR_DET(0),
				RXS_PLM_SPX_STAT_DLT |
				RXS_SPX_ERR_DET_OK_TO_UNINIT}}},
		{{1, rio_em_f_port_err},
			{{RXS_PLM_SPX_STAT(1),
				RXS_PLM_SPX_STAT_PORT_ERR},
			{0, 0}}},
		{{2, rio_em_f_2many_pna},
			{{RXS_PLM_SPX_STAT(2),
				RXS_PLM_SPX_STAT_MAX_DENIAL},
			{RXS_PLM_SPX_PNA_CAP(2),
				RXS_PLM_SPX_PNA_CAP_VALID}}},
		{{3, rio_em_f_2many_retx},
			{{RXS_PLM_SPX_STAT(3),
				RXS_PLM_SPX_STAT_MAX_DENIAL},
			{0, 0}}},
		{{4, rio_em_f_err_rate},
			{{RXS_PBM_SPX_STAT(4),
				RXS_PBM_SPX_STAT_EG_DNFL_FATAL |
				RXS_PBM_SPX_STAT_EG_DOH_FATAL |
				RXS_PBM_SPX_STAT_EG_DATA_UNCOR},
			{RXS_PLM_SPX_STAT(4),
				RXS_PLM_SPX_STAT_PBM_FATAL}}},
		{{5, rio_em_d_ttl},
			{{RXS_PBM_SPX_STAT(5),
				RXS_PBM_SPX_STAT_EG_TTL_EXPIRED},
			{0, 0}}},
		{{6, rio_em_d_rte},
			{{RXS_TLM_SPX_STAT(6),
				RXS_TLM_SPX_STAT_LUT_DISCARD},
			{0, 0}}},
		{{RIO_ALL_PORTS, rio_em_d_log},
			{{RXS_ERR_DET,
				RXS_ERR_DET_ILL_TYPE |
				RXS_ERR_DET_UNS_RSP |
				RXS_ERR_DET_ILL_ID},
			{0, 0}}},
		{{8, rio_em_i_sig_det},
			{{RXS_PLM_SPX_STAT(8),
				RXS_PLM_SPX_STAT_LINK_INIT},
			{RXS_SPX_ERR_DET(8),
				RXS_PLM_SPX_STAT_LINK_INIT}}},
		{{9, rio_em_i_rst_req},
			{{RXS_PLM_SPX_STAT(9),
				RXS_PLM_SPX_STAT_RST_REQ |
				RXS_PLM_SPX_STAT_PRST_REQ},
			{0, 0}}},
		{{RIO_ALL_PORTS, rio_em_i_init_fail},
			{{I2C_INT_STAT, I2C_INT_STAT_BL_FAIL},
			{0, 0}}},
	};
	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i;
	unsigned int j;
	uint32_t chk_val;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Power up and enable all ports...
	set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);

	for (i = 0; i < test_cnt; i++) {
		if (DEBUG_PRINTF) {
			printf("\ni %d event %d port %d\n", i,
				tests[i].event.event, tests[i].event.port_num);
		}
		in_p.num_events = 1;
		in_p.events = events;
		events[0].port_num = tests[i].event.port_num;
		events[0].event = tests[i].event.event;

		out_p.imp_rc = 0xFFFFFF;
		out_p.failure_idx = 0xff;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_create_events(dev_info, &in_p,
						&out_p));

		assert_int_equal(RIO_SUCCESS, out_p.imp_rc);
		assert_int_equal(0, out_p.failure_idx);
		for (j = 0; j < 2; j++) {
			if (!tests[i].chk[j].addr) {
				continue;
			}
			if (DEBUG_PRINTF) {
				printf("\nj %d addr 0x%x port 0x%x\n", j,
					tests[i].chk[j].addr,
					tests[i].chk[j].val);
			}
			assert_int_equal(RIO_SUCCESS,
				DARRegRead(dev_info, tests[i].chk[j].addr,
						&chk_val));
			assert_int_equal(chk_val & tests[i].chk[j].val,
					tests[i].chk[j].val);
		}
	}

	(void)state;
}

// Test that events which should be ignored are successfull.

static void rxs_rio_em_create_ignored_events_test(void **state)
{
	rio_em_create_events_in_t in_p;
	rio_em_create_events_out_t out_p;
	rio_em_event_n_loc_t events[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;

	rio_em_events_t tests[] = {rio_em_a_clr_pwpnd, rio_em_a_no_event};
	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Power up and enable all ports...
	set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);

	for (i = 0; i < test_cnt; i++) {
		in_p.num_events = 1;
		in_p.events = events;
		events[0].port_num = 0;
		events[0].event = tests[i];

		out_p.imp_rc = 0xFFFFFF;
		out_p.failure_idx = 0xff;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_create_events(dev_info, &in_p,
						&out_p));

		assert_int_equal(RIO_SUCCESS, out_p.imp_rc);
		assert_int_equal(0, out_p.failure_idx);
	}

	(void)state;
}

// Test bad parameter detection.

static void rxs_rio_em_get_int_stat_bad_parms_test(void **state)
{
	rio_em_get_int_stat_in_t in_p;
	rio_em_get_int_stat_out_t out_p;
	rio_em_event_n_loc_t events[(uint8_t)rio_em_last];

	// Illegal number of ports
	in_p.ptl.num_ports = NUM_RXS_PORTS(&mock_dev_info) + 1;
	in_p.ptl.pnums[0] = 0;
	in_p.ptl.pnums[1] = 0;
	in_p.num_events = (uint8_t)rio_em_last;
	in_p.events = events;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_get_int_stat(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	// Illegal port number
	in_p.ptl.num_ports = 1;
	in_p.ptl.pnums[0] = NUM_RXS_PORTS(&mock_dev_info);

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_get_int_stat(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	// Illegal number of events
	in_p.ptl.num_ports = 1;
	in_p.ptl.pnums[0] = 0;
	in_p.num_events = 0;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_get_int_stat(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	// Null events pointer
	in_p.num_events = (uint8_t)rio_em_last;
	in_p.events = NULL;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_get_int_stat(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	(void)state;
}

// NOTE: A 2many_pna event also causes a 2many_retx event.
// For this reason, "2many_pna" must be the last test,
// and 2many_retx must occur before 2many_pna.
// Some tests depend on 2many_pna and 2many_retx occurring one after the other.
rio_em_cfg_t int_tests[] = {
	{rio_em_f_los, rio_em_detect_on, 0x253 * 1000},
	{rio_em_f_port_err, rio_em_detect_on, 0},
	{rio_em_f_err_rate, rio_em_detect_on, 0},
	{rio_em_d_ttl, rio_em_detect_on, 0x0000FFFF},
	{rio_em_d_rte, rio_em_detect_on, 0},
	{rio_em_d_log, rio_em_detect_on,
			RXS_ERR_DET_ILL_TYPE |
			RXS_ERR_DET_UNS_RSP |
			RXS_ERR_DET_ILL_ID},
	{rio_em_i_sig_det, rio_em_detect_on, 0},
	{rio_em_i_rst_req, rio_em_detect_on, 0},
	{rio_em_i_init_fail, rio_em_detect_on, 0},
	{rio_em_f_2many_retx, rio_em_detect_on, 0x0010},
	{rio_em_f_2many_pna, rio_em_detect_on, 0x0010}
};

const unsigned int int_test_cnt = sizeof(int_tests) / sizeof(int_tests[0]);

// Test the interrupt status is correctly determined for all events

static void rxs_rio_em_get_int_stat_success_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_create_events_in_t c_in;
	rio_em_create_events_out_t c_out;
	rio_em_get_int_stat_in_t in_p;
	rio_em_get_int_stat_out_t out_p;
	rio_em_event_n_loc_t c_e[1];
	rio_em_event_n_loc_t stat_e[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;

	unsigned int i, chk_i, srch_i;
	rio_port_t port = 0;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Power up and enable all ports...
	set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);

	for (i = 0; i < int_test_cnt; i++) {
		if (DEBUG_PRINTF) {
			printf("\ni %d event %d\n", i, int_tests[i].em_event);
		}

		// Enable detection of each event.
		if (rio_em_i_rst_req == int_tests[i].em_event) {
			set_plm_imp_spec_ctl_rst(port, rio_em_detect_on);
		}

		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = port;
		set_cfg_in.notfn = rio_em_notfn_int;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &int_tests[i];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(rio_em_notfn_int, set_cfg_out.notfn);
		rxs_rio_em_cfg_set_get_chk_regs(
					&int_tests[i], rio_em_notfn_int, port);

		// Create the event
		c_in.num_events = 1;
		c_in.events = c_e;
		if ((rio_em_d_log == int_tests[i].em_event) ||
			(rio_em_i_init_fail == int_tests[i].em_event)) {
			c_e[0].port_num = RIO_ALL_PORTS;
		} else {
			c_e[0].port_num = port;
		}
		c_e[0].event = int_tests[i].em_event;

		c_out.imp_rc = 0xFFFFFF;
		c_out.failure_idx = 0xff;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_create_events(dev_info, &c_in,
						&c_out));

		assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
		assert_int_equal(0, c_out.failure_idx);

		// Query the event interrupt status
		in_p.ptl.num_ports = 1;
		in_p.ptl.pnums[0] = port;
		in_p.num_events = (uint8_t)rio_em_last;
		in_p.events = stat_e;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_p.imp_rc = 0;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_get_int_stat(&mock_dev_info,
						&in_p, &out_p));
		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(i + 1, out_p.num_events);
		assert_false(out_p.too_many);
		assert_false(out_p.other_events);

		// Check that all events created to date are all found...
		for (chk_i = 0; chk_i <= i; chk_i++) {
			bool found = false;
			for (srch_i = 0; !found && (srch_i <= i); srch_i++) {
				if (int_tests[chk_i].em_event
						== stat_e[srch_i].event) {
					found = true;
				}
			}
			if (!found && DEBUG_PRINTF) {
				printf("i %d event_cnt %d chk_i %d event %d", i,
						out_p.num_events, chk_i,
						int_tests[chk_i].em_event);
			}
			assert_true(found);
		}

		// Query the event interrupt status again, and trigger the
		// "too many events" flag.  Must have at least 2 events to
		// trigger "too many events".
		if (!i) {
			continue;
		}
		in_p.ptl.num_ports = 1;
		in_p.ptl.pnums[0] = port;
		in_p.num_events = i;
		in_p.events = stat_e;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_p.imp_rc = 0;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_get_int_stat(&mock_dev_info,
						&in_p, &out_p));
		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(i, out_p.num_events);
		assert_true(out_p.too_many);
		assert_false(out_p.other_events);
	}

	(void)state;
}

// Test that if one event is configured with interrupt
// notification and all other events are disabled, that
// the "other events" field behaves correctly.

static void rxs_rio_em_get_int_stat_other_events_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_create_events_in_t c_in;
	rio_em_create_events_out_t c_out;
	rio_em_get_int_stat_in_t in_p;
	rio_em_get_int_stat_out_t out_p;
	rio_em_event_n_loc_t c_e[2];
	rio_em_event_n_loc_t stat_e[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;
	unsigned int i, t;
	RXS_test_state_t *l_st = (RXS_test_state_t *)*state;
	rio_port_t port = 1;

	if (l_st->real_hw) {
		return;
	}

	// Use test_cnt - 1 here to avoid trying for rio_em_f_2many_pna
	// without also setting rio_em_f_2many_retx.
	for (i = 0; i < int_test_cnt - 1; i++) {
		for (t = 0; t < int_test_cnt; t++) {
			// Must have two different events for this test.
			if (i == t) {
				continue;
			}
			// Reset request interrupts cannot be masked at the
			// port level, so skip 'em...
			if (rio_em_i_rst_req == int_tests[t].em_event) {
				continue;
			}
			// Can't tell the difference between a 2many_retx and
			// a 2many_pna, so skip this case...
			if ((rio_em_f_2many_retx == int_tests[i].em_event)
			 && (rio_em_f_2many_pna == int_tests[t].em_event)) {
				continue;
			}
			// This test requires a clean slate at the beginning
			// of each attempt
			setup(state);

			// Power up and enable all ports...
			set_all_port_config(cfg_perfect, false, false,
								RIO_ALL_PORTS);

			if (DEBUG_PRINTF) {
				printf("\ni %d ev %d t %d ev %d\n",
					i, int_tests[i].em_event,
					t, int_tests[t].em_event);
			}

			// Enable detection of the current event.
			if (rio_em_i_rst_req == int_tests[i].em_event) {
				set_plm_imp_spec_ctl_rst(port,
							int_tests[i].em_detect);
			}

			// Enable the i'th test
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = port;
			set_cfg_in.notfn = rio_em_notfn_int;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &int_tests[i];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_cfg_set(&mock_dev_info,
							&set_cfg_in,
							&set_cfg_out));
			assert_int_equal(0, set_cfg_out.imp_rc);
			assert_int_equal(RIO_ALL_PORTS,
					set_cfg_out.fail_port_num);
			assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
			assert_int_equal(rio_em_notfn_int, set_cfg_out.notfn);
			rxs_rio_em_cfg_set_get_chk_regs(
					&int_tests[i], rio_em_notfn_int, port);

			// Create the i'th and t'th event
			c_in.num_events = 2;
			c_in.events = c_e;
			c_e[0].event = int_tests[i].em_event;
			c_e[1].event = int_tests[t].em_event;
			if ((rio_em_d_log == int_tests[i].em_event) ||
				(rio_em_i_init_fail == int_tests[i].em_event)) {
				c_e[0].port_num = RIO_ALL_PORTS;
			} else {
				c_e[0].port_num = port;
			}
			if ((rio_em_d_log == int_tests[t].em_event) ||
				(rio_em_i_init_fail == int_tests[t].em_event)) {
				c_e[1].port_num = RIO_ALL_PORTS;
			} else {
				c_e[1].port_num = port;
			}

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event interrupt status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = port;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = true;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_get_int_stat(&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(1, out_p.num_events);
			assert_false(out_p.too_many);
			assert_true(out_p.other_events);

			// Check that the event created was found
			assert_int_equal(int_tests[i].em_event,
							stat_e[0].event);
		}
	}

	(void)state;
}

// Test bad parameter values are correctly detected and reported.

static void rxs_rio_em_get_pw_stat_bad_parms_test(void **state)
{
	rio_em_get_pw_stat_in_t in_p;
	rio_em_get_pw_stat_out_t out_p;
	rio_em_event_n_loc_t events[(uint8_t)rio_em_last];

	// Illegal number of ports
	in_p.ptl.num_ports = NUM_RXS_PORTS(&mock_dev_info) + 1;
	in_p.ptl.pnums[0] = 0;
	in_p.ptl.pnums[1] = 0;
	in_p.num_events = (uint8_t)rio_em_last;
	in_p.events = events;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_get_pw_stat(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	// Illegal port number
	in_p.ptl.num_ports = 1;
	in_p.ptl.pnums[0] = NUM_RXS_PORTS(&mock_dev_info);

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_get_pw_stat(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	// Illegal number of events
	in_p.ptl.num_ports = 1;
	in_p.ptl.pnums[0] = 0;
	in_p.num_events = 0;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_get_pw_stat(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	// Null events pointer
	in_p.num_events = (uint8_t)rio_em_last;
	in_p.events = NULL;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_get_pw_stat(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	(void)state;
}

// Test port-write status is correctly determined

static void rxs_rio_em_get_pw_stat_success_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_create_events_in_t c_in;
	rio_em_create_events_out_t c_out;
	rio_em_get_pw_stat_in_t in_p;
	rio_em_get_pw_stat_out_t out_p;
	rio_em_event_n_loc_t c_e[1];
	rio_em_event_n_loc_t stat_e[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;
	unsigned int i, chk_i, srch_i;
	rio_port_t port = 2;
	RXS_test_state_t *l_st = (RXS_test_state_t *)*state;

	if (l_st->real_hw) {
		return;
	}

	// Power up and enable all ports...
	set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);

	for (i = 0; i < int_test_cnt; i++) {
		// Enable detection of each event.
		if (rio_em_i_rst_req == int_tests[i].em_event) {
			// If we're testing disabling the Reset Request
			// event, do the real disable since this events
			// detection is actually controlled by Port Config
			// functionality.
			set_plm_imp_spec_ctl_rst(port, rio_em_detect_on);
		}

		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = port;
		set_cfg_in.notfn = rio_em_notfn_pw;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &int_tests[i];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(rio_em_notfn_pw, set_cfg_out.notfn);
		rxs_rio_em_cfg_set_get_chk_regs(
					&int_tests[i], rio_em_notfn_pw, port);

		// Create the event
		c_in.num_events = 1;
		c_in.events = c_e;
		c_e[0].event = int_tests[i].em_event;
		if ((rio_em_d_log == int_tests[i].em_event) ||
			(rio_em_i_init_fail == int_tests[i].em_event)) {
			c_e[0].port_num = RIO_ALL_PORTS;
		} else {
			c_e[0].port_num = port;
		}

		c_out.imp_rc = 0xFFFFFF;
		c_out.failure_idx = 0xff;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_create_events(dev_info, &c_in,
						&c_out));

		assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
		assert_int_equal(0, c_out.failure_idx);

		// Query the event port-write status
		in_p.ptl.num_ports = 1;
		in_p.ptl.pnums[0] = port;
		in_p.pw_port_num = RIO_ALL_PORTS;
		in_p.num_events = (uint8_t)rio_em_last;
		in_p.events = stat_e;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_p.imp_rc = 0;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_get_pw_stat(&mock_dev_info, &in_p,
						&out_p));
		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(i + 1, out_p.num_events);
		assert_false(out_p.too_many);
		assert_false(out_p.other_events);

		// Check that all events created to date are all found...
		for (chk_i = 0; chk_i <= i; chk_i++) {
			bool found = false;
			for (srch_i = 0; !found && (srch_i <= i); srch_i++) {
				if (int_tests[chk_i].em_event
						== stat_e[srch_i].event) {
					found = true;
				}
			}
			if (!found && DEBUG_PRINTF) {
				printf("i %d event_cnt %d chk_i %d event %d", i,
						out_p.num_events, chk_i,
						int_tests[chk_i].em_event);
			}
			assert_true(found);
		}

		// Query the event interrupt status again, and trigger the
		// "too many events" flag.
		if (!i) {
			continue;
		}
		in_p.ptl.num_ports = 1;
		in_p.ptl.pnums[0] = port;
		in_p.num_events = i;
		in_p.events = stat_e;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_p.imp_rc = 0;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_get_pw_stat(&mock_dev_info, &in_p,
						&out_p));
		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(i, out_p.num_events);
		assert_true(out_p.too_many);
		assert_false(out_p.other_events);
	}

	(void)state;
}

// Test that if one event is configured with port-write
// notification and all other events are disabled, that
// the "other events" fields behave correctly.
//
// This test is skipped on real hardware.

static void rxs_rio_em_get_pw_stat_other_events_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_create_events_in_t c_in;
	rio_em_create_events_out_t c_out;
	rio_em_get_pw_stat_in_t in_p;
	rio_em_get_pw_stat_out_t out_p;
	rio_em_event_n_loc_t c_e[2];
	rio_em_event_n_loc_t stat_e[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;

	rio_port_t port = 3;

	unsigned int i, t;
	RXS_test_state_t *l_st = (RXS_test_state_t *)*state;

	if (l_st->real_hw) {
		return;
	}

	// Use test_cnt - 1 here to avoid trying for rio_em_f_2many_pna
	// without also setting rio_em_f_2many_retx.
	for (i = 0; i < int_test_cnt - 1; i++) {
		for (t = 0; t < int_test_cnt; t++) {
			// Must have two different events for this test.
			if (i == t) {
				continue;
			}
			// Reset request port-writes cannot be masked at the
			// port level, so skip 'em...
			if (rio_em_i_rst_req == int_tests[t].em_event) {
				continue;
			}
			// Can't tell the difference between a 2many_retx and
			// a 2many_pna, so skip this case...
			if ((rio_em_f_2many_retx == int_tests[i].em_event)
			 && (rio_em_f_2many_pna == int_tests[t].em_event)) {
				continue;
			}
			if (DEBUG_PRINTF) {
				printf("\ni %d event %d t %d ev %d\n",
					i, int_tests[i].em_event,
					t, int_tests[t].em_event);
			}

			// This test requires a clean slate at the beginning
			// of each attempt
			setup(state);

			// Power up and enable all ports...
			set_all_port_config(cfg_perfect, false, false,
								RIO_ALL_PORTS);

			// Enable detection of the current event.
			if (rio_em_i_rst_req == int_tests[i].em_event) {
				set_plm_imp_spec_ctl_rst(port,
							int_tests[i].em_detect);
			}

			// Enable the i'th test
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = port;
			set_cfg_in.notfn = rio_em_notfn_pw;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &int_tests[i];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_cfg_set(&mock_dev_info,
							&set_cfg_in,
							&set_cfg_out));
			assert_int_equal(0, set_cfg_out.imp_rc);
			assert_int_equal(RIO_ALL_PORTS,
					set_cfg_out.fail_port_num);
			assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
			assert_int_equal(rio_em_notfn_pw, set_cfg_out.notfn);

			// Create the i'th and t'th event
			c_in.num_events = 2;
			c_in.events = c_e;
			c_e[0].event = int_tests[i].em_event;
			c_e[1].event = int_tests[t].em_event;
			if ((rio_em_d_log == int_tests[i].em_event) ||
				(rio_em_i_init_fail == int_tests[i].em_event)) {
				c_e[0].port_num = RIO_ALL_PORTS;
			} else {
				c_e[0].port_num = port;
			}
			if ((rio_em_d_log == int_tests[t].em_event) ||
				(rio_em_i_init_fail == int_tests[t].em_event)) {
				c_e[1].port_num = RIO_ALL_PORTS;
			} else {
				c_e[1].port_num = port;
			}

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = port;
			in_p.pw_port_num = RIO_ALL_PORTS;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = true;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(1, out_p.num_events);
			assert_false(out_p.too_many);
			assert_int_equal(int_tests[i].em_event, stat_e[0].event);
			// Check that the other events were found, EXCEPT
			// when the two events are 2many_retx & 2many_pna
			if ((rio_em_f_2many_retx == int_tests[i].em_event) &&
				(rio_em_f_2many_pna == int_tests[t].em_event)) {
				continue;
			}
			assert_true(out_p.other_events);
		}
	}

	(void)state;
}

// Test that if one event is configured with interrupt notification, and another
// is configured with port-write notification, that the "other events" fields
// for interrupt and port-write status indicate that another event is present.

static void rxs_rio_em_get_int_pw_stat_other_events_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_create_events_in_t c_in;
	rio_em_create_events_out_t c_out;
	rio_em_get_pw_stat_in_t in_p;
	rio_em_get_pw_stat_out_t out_p;
	rio_em_get_int_stat_in_t in_i;
	rio_em_get_int_stat_out_t out_i;
	rio_em_event_n_loc_t c_e[2];
	rio_em_event_n_loc_t stat_e[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;

	rio_port_t port = 4;
	unsigned int i, p;
	RXS_test_state_t *l_st = (RXS_test_state_t *)*state;
	const rio_em_notfn_ctl_t i_notfn = rio_em_notfn_int;
	const rio_em_notfn_ctl_t p_notfn = rio_em_notfn_pw;

	if (l_st->real_hw) {
		return;
	}

	for (i = 0; i < int_test_cnt; i++) {
		// Reset request events cannot be masked,
		// so just skip this case.
		if (rio_em_i_rst_req == int_tests[i].em_event) {
			continue;
		}
		for (p = 0; p < int_test_cnt; p++) {
			// Must have two different events for this test.
			if (i == p) {
				continue;
			}
			// Reset request events cannot be masked,
			// so just skip this case.
			if (rio_em_i_rst_req == int_tests[p].em_event) {
				continue;
			}
			// 2many_retx and 2many_pna cause the same event,
			// so skip ahead if they're both selected...
			if ((rio_em_f_2many_retx == int_tests[i].em_event) &&
				(rio_em_f_2many_pna == int_tests[p].em_event)) {
				continue;
			}
			if ((rio_em_f_2many_retx == int_tests[p].em_event) &&
				(rio_em_f_2many_pna == int_tests[i].em_event)) {
				continue;
			}
			if (DEBUG_PRINTF) {
				printf("\ni = %d p = %d event = %d %d\n", i, p,
						int_tests[i].em_event,
						int_tests[p].em_event);
			}
			if (DEBUG_PRINTF) {
				printf("\ni %d event %d p %d event %d\n",
					i, int_tests[i].em_event,
					p, int_tests[p].em_event);
			}

			// This test requires a clean slate at the beginning
			// of each attempt
			setup(state);

			// Power up and enable all ports...
			set_all_port_config(cfg_perfect, false, false,
								RIO_ALL_PORTS);

			// Enable detection of the current event.
			// NOTE: Only one of int_tests[i].em_event and
			// int_tests[p].em_event can be true at any one time.
			if (rio_em_i_rst_req == int_tests[i].em_event) {
				set_plm_imp_spec_ctl_rst(port,
							int_tests[i].em_detect);
			}
			if (rio_em_i_rst_req == int_tests[p].em_event) {
				set_plm_imp_spec_ctl_rst(port,
							int_tests[p].em_detect);
			}

			// Enable event with interrupt notification
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = port;
			set_cfg_in.notfn = i_notfn;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &int_tests[i];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_cfg_set(&mock_dev_info,
							&set_cfg_in,
							&set_cfg_out));
			assert_int_equal(0, set_cfg_out.imp_rc);
			assert_int_equal(RIO_ALL_PORTS,
					set_cfg_out.fail_port_num);
			assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
			assert_int_equal(i_notfn, set_cfg_out.notfn);
			rxs_rio_em_cfg_set_get_chk_regs(
					&int_tests[i], rio_em_notfn_int, port);

			// Enable event with port-write notification
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = port;
			set_cfg_in.notfn = p_notfn;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &int_tests[p];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_cfg_set(&mock_dev_info,
							&set_cfg_in,
							&set_cfg_out));
			assert_int_equal(0, set_cfg_out.imp_rc);
			assert_int_equal(RIO_ALL_PORTS,
					set_cfg_out.fail_port_num);
			assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
			assert_int_equal(p_notfn, set_cfg_out.notfn);
			rxs_rio_em_cfg_set_get_chk_regs(
					&int_tests[p], rio_em_notfn_pw, port);

			// Create the i'th and p'th events
			c_in.num_events = 2;
			c_in.events = c_e;
			c_e[0].event = int_tests[i].em_event;
			c_e[1].event = int_tests[p].em_event;

			if ((rio_em_d_log == int_tests[i].em_event) ||
				(rio_em_i_init_fail == int_tests[i].em_event)) {
				c_e[0].port_num = RIO_ALL_PORTS;
			} else {
				c_e[0].port_num = port;
			}

			if ((rio_em_d_log == int_tests[p].em_event) ||
				(rio_em_i_init_fail == int_tests[p].em_event)) {
				c_e[1].port_num = RIO_ALL_PORTS;
			} else {
				c_e[1].port_num = port;
			}

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event interrupt status
			in_i.ptl.num_ports = 1;
			in_i.ptl.pnums[0] = port;
			in_p.pw_port_num = RIO_ALL_PORTS;
			in_i.num_events = (uint8_t)rio_em_last;
			in_i.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_i.imp_rc = 0;
			out_i.num_events = 0xFF;
			out_i.too_many = false;
			out_i.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(1, out_i.num_events);
			assert_false(out_i.too_many);
			assert_true(out_i.other_events);
			assert_int_equal(stat_e[0].port_num, c_e[0].port_num);
			assert_int_equal(stat_e[0].event,int_tests[i].em_event);

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = port;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = false;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(1, out_p.num_events);
			assert_false(out_p.too_many);
			assert_true(out_p.other_events);
			assert_int_equal(stat_e[0].port_num, c_e[1].port_num);
			assert_int_equal(stat_e[0].event,int_tests[p].em_event);
		}
	}

	(void)state;
}

// Test that if two events are configured with both interrupt and
// port-write notification, that the interrupt and port-write status is
// correct.
void find_event(rio_em_event_n_loc_t *tgt_ev, rio_em_event_n_loc_t *ev_list,
			unsigned int max_ev_list)
{
	bool found = false;
	unsigned int idx;

	for (idx = 0; !found && (idx < max_ev_list); idx++) {
		found = (tgt_ev->port_num == ev_list[idx].port_num) &&
			(tgt_ev->event == ev_list[idx].event);
	}
	assert_true(found);
}

static void rxs_rio_em_get_int_pw_stat_both_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_create_events_in_t c_in;
	rio_em_create_events_out_t c_out;
	rio_em_get_pw_stat_in_t in_p;
	rio_em_get_pw_stat_out_t out_p;
	rio_em_get_int_stat_in_t in_i;
	rio_em_get_int_stat_out_t out_i;
	rio_em_event_n_loc_t c_e[2];
	rio_em_event_n_loc_t stat_e[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;
	rio_em_cfg_t tests_in[2];

	rio_port_t port = 5;

	unsigned int i, p;
	RXS_test_state_t *l_st = (RXS_test_state_t *)*state;

	if (l_st->real_hw) {
		return;
	}

	// Use test_cnt - 1 here to avoid trying for rio_em_f_2many_pna
	// without also setting rio_em_f_2many_retx.
	for (i = 0; i < int_test_cnt - 1; i++) {
		for (p = 0; p < int_test_cnt; p++) {
			// Must have two different events for this test.
			if (i == p) {
				continue;
			}
			// This test requires a clean slate at the beginning
			// of each attempt
			setup(state);

			// Power up and enable all ports...
			set_all_port_config(cfg_perfect, false, false,
								RIO_ALL_PORTS);

			// Enable detection of the current event.
			// NOTE: Only one of int_tests[i].em_event and
			// int_tests[p].em_event can be true at any one time.
			if (rio_em_i_rst_req == int_tests[i].em_event) {
				set_plm_imp_spec_ctl_rst(port,
						int_tests[i].em_detect);
			}
			if (rio_em_i_rst_req == int_tests[p].em_event) {
				set_plm_imp_spec_ctl_rst(port,
						int_tests[p].em_detect);
			}

			if (DEBUG_PRINTF) {
				printf("\ni %d event %d p %d event %d\n",
					i, int_tests[i].em_event,
					p, int_tests[p].em_event);
			}
			// Configure the i'th and p'th event
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = port;
			set_cfg_in.notfn = rio_em_notfn_both;
			set_cfg_in.num_events = 2;
			memcpy(&tests_in[0], &int_tests[i], sizeof(tests_in[0]));
			memcpy(&tests_in[1], &int_tests[p], sizeof(tests_in[1]));
			set_cfg_in.events = tests_in;

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_cfg_set(&mock_dev_info,
							&set_cfg_in,
							&set_cfg_out));
			assert_int_equal(0, set_cfg_out.imp_rc);
			assert_int_equal(RIO_ALL_PORTS,
					set_cfg_out.fail_port_num);
			assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
			assert_int_equal(rio_em_notfn_both, set_cfg_out.notfn);
			rxs_rio_em_cfg_set_get_chk_regs(
					&int_tests[i], rio_em_notfn_both, port);
			rxs_rio_em_cfg_set_get_chk_regs(
					&int_tests[p], rio_em_notfn_both, port);

			// Create the i'th and p'th events
			c_in.num_events = 2;
			c_in.events = c_e;
			c_e[0].event = int_tests[i].em_event;
			c_e[1].event = int_tests[p].em_event;

			if ((rio_em_d_log == int_tests[i].em_event) ||
				(rio_em_i_init_fail == int_tests[i].em_event)) {
				c_e[0].port_num = RIO_ALL_PORTS;
			} else {
				c_e[0].port_num = port;
			}

			if ((rio_em_d_log == int_tests[p].em_event) ||
				(rio_em_i_init_fail == int_tests[p].em_event)) {
				c_e[1].port_num = RIO_ALL_PORTS;
			} else {
				c_e[1].port_num = port;
			}

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event interrupt status
			in_i.ptl.num_ports = 1;
			in_i.ptl.pnums[0] = port;
			in_i.num_events = (uint8_t)rio_em_last;
			in_i.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_i.imp_rc = 0;
			out_i.num_events = 0xFF;
			out_i.too_many = true;
			out_i.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(2, out_i.num_events);
			assert_false(out_i.too_many);
			assert_false(out_i.other_events);

			// Check that events created are found...
			find_event(&c_e[0], stat_e, 2);
			find_event(&c_e[1], stat_e, 2);

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = port;
			in_p.pw_port_num = RIO_ALL_PORTS;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = true;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(2, out_p.num_events);
			assert_false(out_p.too_many);
			assert_false(out_p.other_events);

			// Check that events created are found...
			find_event(&c_e[0], stat_e, 2);
			find_event(&c_e[1], stat_e, 2);
		}
	}

	(void)state;
}

// Test that bad parameter values are detected and reported.

static void rxs_rio_em_clr_events_bad_parms_test(void **state)
{
	rio_em_clr_events_in_t in_c;
	rio_em_clr_events_out_t out_c;
	rio_em_event_n_loc_t events[(uint8_t)rio_em_last];

	// Illegal number of events
	in_c.num_events = 0;
	in_c.events = events;

	out_c.imp_rc = 0;
	out_c.failure_idx = 0xFF;
	out_c.pw_events_remain = true;
	out_c.int_events_remain = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_clr_events(&mock_dev_info, &in_c,
					&out_c));
	assert_int_not_equal(0, out_c.imp_rc);
	assert_int_equal(0xFF, out_c.failure_idx);
	assert_true(out_c.pw_events_remain);
	assert_true(out_c.int_events_remain);

	// Null events pointer
	in_c.num_events = 1;
	in_c.events = NULL;

	out_c.imp_rc = 0;
	out_c.failure_idx = 0xFF;
	out_c.pw_events_remain = true;
	out_c.int_events_remain = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_clr_events(&mock_dev_info, &in_c,
					&out_c));
	assert_int_not_equal(0, out_c.imp_rc);
	assert_int_equal(0xFF, out_c.failure_idx);
	assert_true(out_c.pw_events_remain);
	assert_true(out_c.int_events_remain);

	// Illegal port in clear port-write pending event
	in_c.num_events = 1;
	in_c.events = events;
	events[0].port_num = NUM_RXS_PORTS(&mock_dev_info);
	events[0].event = rio_em_a_clr_pwpnd;

	out_c.imp_rc = 0;
	out_c.failure_idx = 0xFF;
	out_c.pw_events_remain = true;
	out_c.int_events_remain = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_clr_events(&mock_dev_info, &in_c,
					&out_c));
	assert_int_not_equal(0, out_c.imp_rc);
	assert_int_equal(0, out_c.failure_idx);
	assert_false(out_c.pw_events_remain);
	assert_false(out_c.int_events_remain);

	// Illegal port in clear 2many_pna event
	in_c.num_events = 1;
	in_c.events = events;
	events[0].port_num = NUM_RXS_PORTS(&mock_dev_info);
	events[0].event = rio_em_f_2many_pna;

	out_c.imp_rc = 0;
	out_c.failure_idx = 0xFF;
	out_c.pw_events_remain = true;
	out_c.int_events_remain = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_clr_events(&mock_dev_info, &in_c,
					&out_c));
	assert_int_not_equal(0, out_c.imp_rc);
	assert_int_equal(0, out_c.failure_idx);
	assert_false(out_c.pw_events_remain);
	assert_false(out_c.int_events_remain);

	// Illegal port in clear TTL event
	in_c.num_events = 1;
	in_c.events = events;
	events[0].port_num = NUM_RXS_PORTS(&mock_dev_info);
	events[0].event = rio_em_d_ttl;

	out_c.imp_rc = 0;
	out_c.failure_idx = 0xFF;
	out_c.pw_events_remain = true;
	out_c.int_events_remain = true;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_em_clr_events(&mock_dev_info, &in_c,
					&out_c));
	assert_int_not_equal(0, out_c.imp_rc);
	assert_int_equal(0, out_c.failure_idx);
	assert_false(out_c.pw_events_remain);
	assert_false(out_c.int_events_remain);

	(void)state;
}

// Verify that each interrupt event can be cleared.

static void rxs_rio_em_clr_int_events_success_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_create_events_in_t c_in;
	rio_em_create_events_out_t c_out;
	rio_em_get_int_stat_in_t in_i;
	rio_em_get_int_stat_out_t out_i;
	rio_em_clr_events_in_t in_c;
	rio_em_clr_events_out_t out_c;

	rio_em_event_n_loc_t c_e[(uint8_t)rio_em_last];
	rio_em_event_n_loc_t stat_e[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;

	rio_port_t port = 6;

	unsigned int i, chk_i, j;
	RXS_test_state_t *l_st = (RXS_test_state_t *)*state;

	if (l_st->real_hw) {
		return;
	}

	// Power up and enable all ports...
	set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);

	for (i = 0; i < int_test_cnt; i++) {
		if (DEBUG_PRINTF) {
			printf("\ni = %d ev = %d\n", i, int_tests[i].em_event);
		 }
		// Enable detection of each event.
		if (rio_em_i_rst_req == int_tests[i].em_event) {
			set_plm_imp_spec_ctl_rst(port, int_tests[i].em_detect);
		}

		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = port;
		set_cfg_in.notfn = rio_em_notfn_both;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &int_tests[i];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(rio_em_notfn_both, set_cfg_out.notfn);

		// Create all events
		c_in.num_events = i + 1;
		c_in.events = c_e;
		for (j = 0; j <= i; j++) {
			if ((rio_em_d_log == int_tests[j].em_event) ||
				(rio_em_i_init_fail == int_tests[j].em_event)) {
				c_e[j].port_num = RIO_ALL_PORTS;
			} else {
				c_e[j].port_num = port;
			}
			c_e[j].event = int_tests[j].em_event;
		}

		c_out.imp_rc = 0xFFFFFF;
		c_out.failure_idx = 0xff;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_create_events(dev_info, &c_in,
						&c_out));

		assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
		assert_int_equal(0, c_out.failure_idx);

		// Query the event interrupt status
		in_i.ptl.num_ports = 1;
		in_i.ptl.pnums[0] = port;
		in_i.num_events = (uint8_t)rio_em_last;
		in_i.events = stat_e;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_i.imp_rc = 0;
		out_i.num_events = 0xFF;
		out_i.too_many = true;
		out_i.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_get_int_stat(&mock_dev_info,
						&in_i, &out_i));
		assert_int_equal(0, out_i.imp_rc);
		assert_int_equal(i + 1, out_i.num_events);
		assert_false(out_i.too_many);
		assert_false(out_i.other_events);

		// Check that all events created to date are all found...
		for (chk_i = 0; chk_i <= i; chk_i++) {
			find_event(&c_e[chk_i], stat_e, i + 1);
		}

		// Clear all interrupt events...
		in_c.num_events = out_i.num_events;
		in_c.events = in_i.events;

		out_c.imp_rc = 0xFFFF;
		out_c.failure_idx = 0xFF;
		out_c.pw_events_remain = true;
		out_c.int_events_remain = true;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_clr_events(&mock_dev_info, &in_c,
						&out_c));
		assert_int_equal(0, out_c.imp_rc);
		assert_int_equal(0, out_c.failure_idx);
		assert_false(out_c.pw_events_remain);
		assert_false(out_c.int_events_remain);

		// Query the event interrupt status, ensure all interrupts
		// are gone...
		in_i.ptl.num_ports = 1;
		in_i.ptl.pnums[0] = 0;
		in_i.num_events = (uint8_t)rio_em_last;
		in_i.events = stat_e;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_i.imp_rc = 0;
		out_i.num_events = 0xFF;
		out_i.too_many = true;
		out_i.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_get_int_stat(&mock_dev_info,
						&in_i, &out_i));
		assert_int_equal(0, out_i.imp_rc);
		assert_int_equal(0, out_i.num_events);
		assert_false(out_i.too_many);
		assert_false(out_i.other_events);
	}

	(void)state;
}

// Verify that each port-write event can be cleared.

static void rxs_rio_em_clr_pw_events_success_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_create_events_in_t c_in;
	rio_em_create_events_out_t c_out;
	rio_em_get_pw_stat_in_t in_p;
	rio_em_get_pw_stat_out_t out_p;
	rio_em_clr_events_in_t in_c;
	rio_em_clr_events_out_t out_c;

	rio_em_event_n_loc_t c_e[(uint8_t)rio_em_last];
	rio_em_event_n_loc_t stat_e[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;
	RXS_test_state_t *l_st = (RXS_test_state_t *)*state;

	if (l_st->real_hw) {
		return;
	}

	rio_port_t port = 7;

	unsigned int i, chk_i, j;

	// Power up and enable all ports...
	set_all_port_config(cfg_perfect, false, false, RIO_ALL_PORTS);

	// Grind through creating and clearing all events.

	for (i = 0; i < int_test_cnt; i++) {
		if (DEBUG_PRINTF) {
			printf("\ni = %d event = %d\n",
						i, int_tests[i].em_event);
		}
		// Enable detection of each event.
		if (rio_em_i_rst_req == int_tests[i].em_event) {
			set_plm_imp_spec_ctl_rst(port, int_tests[i].em_detect);
		}

		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = port;
		set_cfg_in.notfn = rio_em_notfn_pw;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &int_tests[i];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(rio_em_notfn_pw, set_cfg_out.notfn);

		// Create the event
		c_in.num_events = i + 1;
		c_in.events = c_e;
		for (j = 0; j <= i; j++) {
			if ((rio_em_d_log == int_tests[j].em_event) ||
				(rio_em_i_init_fail == int_tests[j].em_event)) {
				c_e[j].port_num = RIO_ALL_PORTS;
			} else {
				c_e[j].port_num = port;
			}
			c_e[j].event = int_tests[j].em_event;
		}

		c_out.imp_rc = 0xFFFFFF;
		c_out.failure_idx = 0xff;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_create_events(dev_info, &c_in,
						&c_out));

		assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
		assert_int_equal(0, c_out.failure_idx);

		// Query the event port-write status
		in_p.ptl.num_ports = 1;
		in_p.ptl.pnums[0] = port;
		in_p.num_events = (uint8_t)rio_em_last;
		in_p.events = stat_e;
		in_p.pw_port_num = RIO_ALL_PORTS;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_p.imp_rc = 0;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_get_pw_stat(&mock_dev_info, &in_p,
						&out_p));
		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(i + 1, out_p.num_events);
		assert_false(out_p.too_many);
		assert_false(out_p.other_events);

		// Check that all events created to date are all found...
		for (chk_i = 0; chk_i <= i; chk_i++) {
			find_event(&c_e[chk_i], stat_e, i + 1);
		}

		// Clear all port-write events...
		in_c.num_events = out_p.num_events;
		in_c.events = in_p.events;

		out_c.imp_rc = 0xFFFF;
		out_c.failure_idx = 0xFF;
		out_c.pw_events_remain = true;
		out_c.int_events_remain = true;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_clr_events(&mock_dev_info, &in_c,
						&out_c));
		assert_int_equal(0, out_c.imp_rc);
		assert_int_equal(0, out_c.failure_idx);
		assert_false(out_c.pw_events_remain);
		assert_false(out_c.int_events_remain);

		// Query the event port-write status, check all events are gone
		in_p.ptl.num_ports = 1;
		in_p.ptl.pnums[0] = port;
		in_p.num_events = (uint8_t)rio_em_last;
		in_p.events = stat_e;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_p.imp_rc = 0;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				rxs_rio_em_get_pw_stat(&mock_dev_info, &in_p,
						&out_p));
		assert_int_equal(0, out_p.imp_rc);
		if (out_p.num_events && DEBUG_PRINTF) {
			printf("\n%d events, first is %d\n", out_p.num_events,
					stat_e[0].event);
		}
		assert_int_equal(0, out_p.num_events);
		assert_false(out_p.too_many);
		assert_false(out_p.other_events);
	}

	(void)state;
}

// Test that if one event is configured with interrupt
// notification and all other events are disabled, that when the port-write
// event is cleared the "other events" fields behave correctly.

static void rxs_rio_em_clr_int_events_other_events_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_create_events_in_t c_in;
	rio_em_create_events_out_t c_out;
	rio_em_get_int_stat_in_t in_i;
	rio_em_get_int_stat_out_t out_i;
	rio_em_clr_events_in_t in_c;
	rio_em_clr_events_out_t out_c;

	rio_em_event_n_loc_t c_e[2];
	rio_em_event_n_loc_t stat_e[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;

	rio_port_t port = 8;
	bool events_remain;

	unsigned int i, t;
	RXS_test_state_t *l_st = (RXS_test_state_t *)*state;

	if (l_st->real_hw) {
		return;
	}

	// Use test_cnt - 1 here to avoid trying for rio_em_f_2many_pna
	// without also setting rio_em_f_err_rate.
	for (i = 0; i < int_test_cnt - 1; i++) {
		// Reset request events cannot be masked at the port
		// level, so skip them for this test.
		if (rio_em_i_rst_req == int_tests[i].em_event) {
			continue;
		}
		for (t = 0; t < int_test_cnt; t++) {
			// Must have two different events for this test.
			if (i == t) {
				continue;
			}
			// Reset request events cannot be masked at the port
			// level, so skip them for this test.
			if (rio_em_i_rst_req == int_tests[t].em_event) {
				continue;
			}
			// 2many_retx and 2many_pna cause the same event,
			// so skip ahead if they're both selected...
			if ((rio_em_f_2many_retx == int_tests[i].em_event) &&
				(rio_em_f_2many_pna == int_tests[t].em_event)) {
				continue;
			}
			if ((rio_em_f_2many_retx == int_tests[t].em_event) &&
				(rio_em_f_2many_pna == int_tests[i].em_event)) {
				continue;
			}
			if (DEBUG_PRINTF) {
				printf("\ni = %d ev = %d t = %d ev = %d\n",
						i, int_tests[i].em_event,
						t, int_tests[t].em_event);
			}
			// This test requires a clean slate at the beginning
			// of each attempt
			setup(state);

			// Power up and enable all ports...
			set_all_port_config(cfg_perfect, false, false,
								RIO_ALL_PORTS);

			// Enable detection of the current event.
			if (rio_em_i_rst_req == int_tests[i].em_event) {
				set_plm_imp_spec_ctl_rst(port,
							int_tests[i].em_detect);
			}

			// Enable the i'th test
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = port;
			set_cfg_in.notfn = rio_em_notfn_int;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &int_tests[i];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_cfg_set(&mock_dev_info,
							&set_cfg_in,
							&set_cfg_out));
			assert_int_equal(0, set_cfg_out.imp_rc);
			assert_int_equal(RIO_ALL_PORTS,
					set_cfg_out.fail_port_num);
			assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
			assert_int_equal(rio_em_notfn_int, set_cfg_out.notfn);

			// Create the i'th and t'th event
			c_in.num_events = 2;
			c_in.events = c_e;
			c_e[0].event = int_tests[i].em_event;
			c_e[1].event = int_tests[t].em_event;

			if ((rio_em_d_log == int_tests[i].em_event) ||
				(rio_em_i_init_fail == int_tests[i].em_event)) {
				c_e[0].port_num = RIO_ALL_PORTS;
			} else {
				c_e[0].port_num = port;
			}
			if ((rio_em_d_log == int_tests[t].em_event) ||
				(rio_em_i_init_fail == int_tests[t].em_event)) {
				c_e[1].port_num = RIO_ALL_PORTS;
			} else {
				c_e[1].port_num = port;
			}

			// The events_remain computation is a pain.
			// If the interrupt event created is cleared by a
			// reset, and the other event does not remain after
			// resetting that port, then events remain should be
			// false; the only events that remain after resetting
			// a port are the d_log and i_init fail, both
			// conveniently flagged by the above clause to have
			// port_num == RIO_ALL_PORTS.
			//
			// Also, if the first event is d_log/i_init_fail, and
			// the second event is NOT d_log/i_init_fail, then
			// no events remain as the per-port events won't be
			// queried.
			events_remain = true;
			if (((rio_em_f_los == c_e[0].event) ||
				(rio_em_f_port_err == c_e[0].event) ||
				(rio_em_f_2many_retx == c_e[0].event) ||
				(rio_em_f_2many_pna == c_e[0].event) ||
				(rio_em_f_err_rate == c_e[0].event)) &&
				(RIO_ALL_PORTS != c_e[1].port_num)) {
					events_remain = false;
			}
			if ((RIO_ALL_PORTS == c_e[0].port_num) &&
					(RIO_ALL_PORTS != c_e[1].port_num)) {
				events_remain = false;
			}

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event interrupt status
			in_i.ptl.num_ports = 1;
			in_i.ptl.pnums[0] = port;
			in_i.num_events = (uint8_t)rio_em_last;
			in_i.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_i.imp_rc = 0;
			out_i.num_events = 0xFF;
			out_i.too_many = true;
			out_i.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(1, out_i.num_events);
			assert_false(out_i.too_many);
			assert_true(out_i.other_events);

			// Check that the event created was found
			assert_int_equal(c_e[0].event, stat_e[0].event);
			assert_int_equal(c_e[0].port_num, stat_e[0].port_num);

			// Clear the interrupt events...
			in_c.num_events = out_i.num_events;
			in_c.events = in_i.events;

			out_c.imp_rc = 0xFFFF;
			out_c.failure_idx = 0xFF;
			out_c.pw_events_remain = true;
			out_c.int_events_remain = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_clr_events(&mock_dev_info,
							&in_c, &out_c));
			assert_int_equal(0, out_c.imp_rc);
			assert_int_equal(0, out_c.failure_idx);

			assert_int_equal(events_remain, out_c.pw_events_remain);
			assert_int_equal(events_remain,out_c.int_events_remain);

			// Query the event interrupt status, confirm that
			// port-write events remain...
			in_i.ptl.num_ports = 1;
			in_i.ptl.pnums[0] = port;
			in_i.num_events = (uint8_t)rio_em_last;
			in_i.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_i.imp_rc = 0;
			out_i.num_events = 0xFF;
			out_i.too_many = true;
			out_i.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(0, out_i.num_events);
			assert_false(out_i.too_many);
			// The events_remain computation is a pain.
			// Redo the events_remain computation, eliminating
			// the second condition from above, since this query
			// will be done against the port number.
			events_remain = true;
			if (((rio_em_f_los == c_e[0].event) ||
				(rio_em_f_port_err == c_e[0].event) ||
				(rio_em_f_2many_retx == c_e[0].event) ||
				(rio_em_f_2many_pna == c_e[0].event) ||
				(rio_em_f_err_rate == c_e[0].event)) &&
				(RIO_ALL_PORTS != c_e[1].port_num)) {
					events_remain = false;
			}
			assert_int_equal(events_remain, out_i.other_events);
		}
	}

	(void)state;
}

// Test that if one event is configured with port-write
// notification and all other events are disabled, that when the port-write
// event is cleared the "other events" fields behave correctly.
//
// This test is skipped on real hardware.

static void rxs_rio_em_clr_pw_events_other_events_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_create_events_in_t c_in;
	rio_em_create_events_out_t c_out;
	rio_em_get_pw_stat_in_t in_p;
	rio_em_get_pw_stat_out_t out_p;
	rio_em_clr_events_in_t in_c;
	rio_em_clr_events_out_t out_c;

	rio_em_event_n_loc_t c_e[2];
	rio_em_event_n_loc_t stat_e[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;

	rio_port_t port = 10;
	bool events_remain;

	unsigned int i, t;
	RXS_test_state_t *l_st = (RXS_test_state_t *)*state;

	if (l_st->real_hw) {
		return;
	}

	// Use test_cnt - 1 here to avoid trying for rio_em_f_2many_pna
	// without also setting rio_em_f_err_rate.
	for (i = 0; i < int_test_cnt - 1; i++) {
		// Reset requests can't be masked at the port level, so
		// they can't be used for the first event of this test.
		if (rio_em_i_rst_req == int_tests[i].em_event) {
			continue;
		}
		for (t = 0; t < int_test_cnt; t++) {
			// Must have two different events for this test.
			if (i == t) {
				continue;
			}
			// 2many_retx and 2many_pna cause the same event,
			// so skip ahead if they're both selected...
			if ((rio_em_f_2many_retx == int_tests[i].em_event) &&
				(rio_em_f_2many_pna == int_tests[t].em_event)) {
				continue;
			}
			if ((rio_em_f_2many_retx == int_tests[t].em_event) &&
				(rio_em_f_2many_pna == int_tests[i].em_event)) {
				continue;
			}
			// Reset requests can't be masked at the port level, so
			// they can't be used for the first event of this test.
			if (rio_em_i_rst_req == int_tests[t].em_event) {
				continue;
			}
			if (DEBUG_PRINTF) {
				printf("\ni = %d ev = %d t = %d ev %d\n",
						i, int_tests[i].em_event,
						t, int_tests[t].em_event);
			}
			// This test requires a clean slate at the beginning
			// of each attempt
			setup(state);

			// Power up and enable all ports...
			set_all_port_config(cfg_perfect, false, false,
								RIO_ALL_PORTS);

			// Enable the i'th test
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = port;
			set_cfg_in.notfn = rio_em_notfn_pw;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &int_tests[i];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_cfg_set(&mock_dev_info,
							&set_cfg_in,
							&set_cfg_out));
			assert_int_equal(0, set_cfg_out.imp_rc);
			assert_int_equal(RIO_ALL_PORTS,
					set_cfg_out.fail_port_num);
			assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
			assert_int_equal(rio_em_notfn_pw, set_cfg_out.notfn);

			// Create the i'th and t'th event
			c_in.num_events = 2;
			c_in.events = c_e;
			c_e[0].event = int_tests[i].em_event;
			c_e[1].event = int_tests[t].em_event;
			if ((rio_em_d_log == int_tests[i].em_event) ||
				(rio_em_i_init_fail == int_tests[i].em_event)) {
				c_e[0].port_num = RIO_ALL_PORTS;
			} else {
				c_e[0].port_num = port;
			}
			if ((rio_em_d_log == int_tests[t].em_event) ||
				(rio_em_i_init_fail == int_tests[t].em_event)) {
				c_e[1].port_num = RIO_ALL_PORTS;
			} else {
				c_e[1].port_num = port;
			}
			// The events_remain computation is a pain.
			// If the interrupt event created is cleared by a
			// reset, and the other event does not remain after
			// resetting that port, then events remain should be
			// false; the only events that remain after resetting
			// a port are the d_log and i_init fail, both
			// conveniently flagged by the above clause to have
			// port_num == RIO_ALL_PORTS.
			//
			// Also, if the first event is d_log/i_init_fail, and
			// the second event is NOT d_log/i_init_fail, then
			// no events remain as the per-port events won't be
			// queried.
			events_remain = true;
			if (((rio_em_f_los == c_e[0].event) ||
				(rio_em_f_port_err == c_e[0].event) ||
				(rio_em_f_2many_retx == c_e[0].event) ||
				(rio_em_f_2many_pna == c_e[0].event) ||
				(rio_em_f_err_rate == c_e[0].event)) &&
				(RIO_ALL_PORTS != c_e[1].port_num)) {
					events_remain = false;
			}
			if ((RIO_ALL_PORTS == c_e[0].port_num) &&
					(RIO_ALL_PORTS != c_e[1].port_num)) {
				events_remain = false;
			}

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = port;
			in_p.pw_port_num = RIO_ALL_PORTS;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = true;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(1, out_p.num_events);
			assert_false(out_p.too_many);
			assert_true(out_p.other_events);

			// Check that the event created was found
			assert_int_equal(int_tests[i].em_event,
							stat_e[0].event);

			// Clear all port-write events...
			in_c.num_events = out_p.num_events;
			in_c.events = in_p.events;

			out_c.imp_rc = 0xFFFF;
			out_c.failure_idx = 0xFF;
			out_c.pw_events_remain = true;
			out_c.int_events_remain = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_clr_events(&mock_dev_info,
							&in_c, &out_c));
			assert_int_equal(0, out_c.imp_rc);
			assert_int_equal(0, out_c.failure_idx);
			assert_int_equal(events_remain, out_c.pw_events_remain);
			assert_int_equal(events_remain,out_c.int_events_remain);

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = port;
			in_p.pw_port_num = RIO_ALL_PORTS;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = true;
			out_p.other_events = true;
			// The events_remain computation is a pain.
			// Redo the events_remain computation, eliminating
			// the second condition from above, since this query
			// will be done against the port number.
			events_remain = true;
			if (((rio_em_f_los == c_e[0].event) ||
				(rio_em_f_port_err == c_e[0].event) ||
				(rio_em_f_2many_retx == c_e[0].event) ||
				(rio_em_f_2many_pna == c_e[0].event) ||
				(rio_em_f_err_rate == c_e[0].event)) &&
				(RIO_ALL_PORTS != c_e[1].port_num)) {
					events_remain = false;
			}

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(0, out_p.num_events);
			assert_false(out_p.too_many);
			assert_int_equal(events_remain, out_p.other_events);
		}
	}

	(void)state;
}

// Test that if one event is configured with port-write
// notification and another is configured with interrupt notification,
// that when the events are created and cleared the
// "other events" fields behave correctly.

static void rxs_rio_em_clr_int_pw_events_other_events_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_create_events_in_t c_in;
	rio_em_create_events_out_t c_out;
	rio_em_get_pw_stat_in_t in_p;
	rio_em_get_pw_stat_out_t out_p;
	rio_em_get_int_stat_in_t in_i;
	rio_em_get_int_stat_out_t out_i;
	rio_em_clr_events_in_t in_c;
	rio_em_clr_events_out_t out_c;

	rio_em_event_n_loc_t c_e[2];
	rio_em_event_n_loc_t stat_i[(uint8_t)rio_em_last];
	rio_em_event_n_loc_t stat_p[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;

	rio_port_t port = 11;
	bool events_remain;

	unsigned int i, p;
	RXS_test_state_t *l_st = (RXS_test_state_t *)*state;
	rio_em_notfn_ctl_t i_notfn, p_notfn;

	if (l_st->real_hw) {
		return;
	}

	for (i = 0; i < int_test_cnt; i++) {
		// Reset requests can't be masked at the port level, so
		// they can't be used for the first event of this test.
		if (rio_em_i_rst_req == int_tests[i].em_event) {
			continue;
		}
		for (p = 0; p < int_test_cnt; p++) {
			// Must have two different events for this test.
			if (i == p) {
				continue;
			}
			// Reset requests can't be masked at the port level, so
			// they can't be used for the first event of this test.
			if (rio_em_i_rst_req == int_tests[p].em_event) {
				continue;
			}
			// 2many_retx & 2many_pna both create the same event,
			// so skip this test configuration.
			if ((rio_em_f_2many_retx == int_tests[i].em_event) &&
				(rio_em_f_2many_pna == int_tests[p].em_event)) {
				continue;
			}
			if ((rio_em_f_2many_retx == int_tests[p].em_event) &&
				(rio_em_f_2many_pna == int_tests[i].em_event)) {
				continue;
			}
			if (DEBUG_PRINTF) {
				printf("\ni = %d ev = %d p = %d ev = %d\n",
						i, int_tests[i].em_event,
						p, int_tests[p].em_event);
			}
			// This test requires a clean slate at the beginning
			// of each attempt
			setup(state);

			// Power up and enable all ports...
			set_all_port_config(cfg_perfect, false, false,
								RIO_ALL_PORTS);

			// Enable the i'th and p'th event
			// Special notification case for f_err_rate and
			// 2many_pna: Both are port_fail events, so notification
			// must be "both".
			i_notfn = rio_em_notfn_int;
			p_notfn = rio_em_notfn_pw;

			// Enable event with interrupt notification
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = port;
			set_cfg_in.notfn = i_notfn;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &int_tests[i];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_cfg_set(&mock_dev_info,
							&set_cfg_in,
							&set_cfg_out));
			assert_int_equal(0, set_cfg_out.imp_rc);
			assert_int_equal(RIO_ALL_PORTS,
					set_cfg_out.fail_port_num);
			assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
			assert_int_equal(i_notfn, set_cfg_out.notfn);

			// Enable event with port-write notification
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = port;
			set_cfg_in.notfn = p_notfn;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &int_tests[p];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_cfg_set(&mock_dev_info,
							&set_cfg_in,
							&set_cfg_out));
			assert_int_equal(0, set_cfg_out.imp_rc);
			assert_int_equal(RIO_ALL_PORTS,
					set_cfg_out.fail_port_num);
			assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
			assert_int_equal(p_notfn, set_cfg_out.notfn);

			// Create the i'th and p'th events
			c_in.num_events = 2;
			c_in.events = c_e;
			c_e[0].event = int_tests[i].em_event;
			c_e[1].event = int_tests[p].em_event;
			if ((rio_em_d_log == int_tests[i].em_event) ||
				(rio_em_i_init_fail == int_tests[i].em_event)) {
				c_e[0].port_num = RIO_ALL_PORTS;
			} else {
				c_e[0].port_num = port;
			}
			if ((rio_em_d_log == int_tests[p].em_event) ||
				(rio_em_i_init_fail == int_tests[p].em_event)) {
				c_e[1].port_num = RIO_ALL_PORTS;
			} else {
				c_e[1].port_num = port;
			}

			// The events_remain computation is a pain.
			// If the interrupt event created is cleared by a
			// reset, and the other event does not remain after
			// resetting that port, then events remain should be
			// false; the only events that remain after resetting
			// a port are the d_log and i_init fail, both
			// conveniently flagged by the above clause to have
			// port_num == RIO_ALL_PORTS.
			//
			// Also, if the first event is d_log/i_init_fail, and
			// the second event is NOT d_log/i_init_fail, then
			// no events remain as the per-port events won't be
			// queried.
			events_remain = true;
			if (((rio_em_f_los == c_e[0].event) ||
				(rio_em_f_port_err == c_e[0].event) ||
				(rio_em_f_2many_retx == c_e[0].event) ||
				(rio_em_f_2many_pna == c_e[0].event) ||
				(rio_em_f_err_rate == c_e[0].event)) &&
				(RIO_ALL_PORTS != c_e[1].port_num)) {
					events_remain = false;
			}
			if ((RIO_ALL_PORTS == c_e[0].port_num) &&
					(RIO_ALL_PORTS != c_e[1].port_num)) {
				events_remain = false;
			}

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event interrupt status
			in_i.ptl.num_ports = 1;
			in_i.ptl.pnums[0] = port;
			in_i.num_events = (uint8_t)rio_em_last;
			in_i.events = stat_i;
			memset(stat_i, 0xFF, sizeof(stat_i));

			out_i.imp_rc = 0;
			out_i.num_events = 0xFF;
			out_i.too_many = false;
			out_i.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(1, out_i.num_events);
			assert_false(out_i.too_many);
			assert_true(out_i.other_events);
			assert_int_equal(stat_i[0].port_num, c_e[0].port_num);
			assert_int_equal(stat_i[0].event,
						int_tests[i].em_event);

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = port;
			in_p.pw_port_num = RIO_ALL_PORTS;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_p;
			memset(stat_p, 0xFF, sizeof(stat_p));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = false;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(1, out_p.num_events);
			assert_false(out_p.too_many);
			assert_true(out_p.other_events);
			assert_int_equal(stat_p[0].port_num, c_e[1].port_num);
			assert_int_equal(stat_p[0].event,int_tests[p].em_event);

			// Clear all interrupt events...
			in_c.num_events = out_i.num_events;
			in_c.events = in_i.events;

			out_c.imp_rc = 0xFFFF;
			out_c.failure_idx = 0xFF;
			out_c.pw_events_remain = true;
			out_c.int_events_remain = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_clr_events(&mock_dev_info,
							&in_c, &out_c));
			assert_int_equal(0, out_c.imp_rc);
			assert_int_equal(0, out_c.failure_idx);

			assert_int_equal(events_remain, out_c.pw_events_remain);
			assert_int_equal(events_remain,out_c.int_events_remain);

			// Clear all port-write events...
			in_c.num_events = out_p.num_events;
			in_c.events = in_p.events;

			out_c.imp_rc = 0xFFFF;
			out_c.failure_idx = 0xFF;
			out_c.pw_events_remain = true;
			out_c.int_events_remain = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_clr_events(&mock_dev_info,
							&in_c, &out_c));
			assert_int_equal(0, out_c.imp_rc);
			assert_int_equal(0, out_c.failure_idx);
			assert_false(out_c.pw_events_remain);
			assert_false(out_c.int_events_remain);

			// Query the event interrupt status, confirm they're
			// gone.
			in_i.ptl.num_ports = 1;
			in_i.ptl.pnums[0] = port;
			in_i.num_events = (uint8_t)rio_em_last;
			in_i.events = stat_i;
			memset(stat_i, 0xFF, sizeof(stat_i));

			out_i.imp_rc = 0;
			out_i.num_events = 0xFF;
			out_i.too_many = false;
			out_i.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(0, out_i.num_events);
			assert_false(out_i.too_many);
			assert_false(out_i.other_events);

			// Query port-write events, confirm they're gone
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = port;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_p;
			memset(stat_p, 0xFF, sizeof(stat_p));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = false;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(0, out_p.num_events);
			assert_false(out_p.too_many);
			assert_false(out_p.other_events);
		}
	}

	(void)state;
}

// Test that when two events are configured with both port-write
// and interrupt notification, that when the events are created and
// cleared the "other events" fields behave correctly.

static void rxs_rio_em_clr_int_pw_events_both_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_create_events_in_t c_in;
	rio_em_create_events_out_t c_out;
	rio_em_get_pw_stat_in_t in_p;
	rio_em_get_pw_stat_out_t out_p;
	rio_em_get_int_stat_in_t in_i;
	rio_em_get_int_stat_out_t out_i;
	rio_em_clr_events_in_t in_c;
	rio_em_clr_events_out_t out_c;

	rio_em_event_n_loc_t c_e[2];
	rio_em_event_n_loc_t stat_i[(uint8_t)rio_em_last];
	rio_em_event_n_loc_t stat_p[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;
	rio_em_cfg_t tests_in[2];

	rio_port_t port = 15;

	unsigned int i, p;
	RXS_test_state_t *l_st = (RXS_test_state_t *)*state;

	if (l_st->real_hw) {
		return;
	}

	// Use test_cnt - 1 here to avoid trying for rio_em_f_2many_pna
	// without also setting rio_em_f_err_rate.
	for (i = 0; i < int_test_cnt - 1; i++) {
		for (p = 0; p < int_test_cnt; p++) {
			// Must have two different events for this test.
			if (i == p) {
				continue;
			}
			if (DEBUG_PRINTF) {
				printf("\ni = %d ev %d p = %d ev = %d\n",
						i, int_tests[i].em_event,
						p, int_tests[p].em_event);
			}
			// This test requires a clean slate at the beginning
			// of each attempt
			setup(state);

			// Power up and enable all ports...
			set_all_port_config(cfg_perfect, false, false,
								RIO_ALL_PORTS);

			// Enable detection of the current event.
			// NOTE: Only one of int_tests[i].em_event and
			// int_tests[p].em_event can be true at any one time.
			if (rio_em_i_rst_req == int_tests[i].em_event) {
				set_plm_imp_spec_ctl_rst(port,
							int_tests[i].em_detect);
			}

			if (rio_em_i_rst_req == int_tests[p].em_event) {
				set_plm_imp_spec_ctl_rst(port,
							int_tests[p].em_detect);
			}

			// Configure the i'th and p'th event
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = port;
			set_cfg_in.notfn = rio_em_notfn_both;
			set_cfg_in.num_events = 2;
			memcpy(&tests_in[0], &int_tests[i], sizeof(tests_in[0]));
			memcpy(&tests_in[1], &int_tests[p], sizeof(tests_in[1]));
			set_cfg_in.events = tests_in;

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_cfg_set(&mock_dev_info,
							&set_cfg_in,
							&set_cfg_out));
			assert_int_equal(0, set_cfg_out.imp_rc);
			assert_int_equal(RIO_ALL_PORTS,
					set_cfg_out.fail_port_num);
			assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
			assert_int_equal(rio_em_notfn_both, set_cfg_out.notfn);

			// Create the i'th and p'th events
			c_in.num_events = 2;
			c_in.events = c_e;
			c_e[0].event = int_tests[i].em_event;
			c_e[1].event = int_tests[p].em_event;
			if ((rio_em_d_log == int_tests[i].em_event) ||
				(rio_em_i_init_fail == int_tests[i].em_event)) {
				c_e[0].port_num = RIO_ALL_PORTS;
			} else {
				c_e[0].port_num = port;
			}
			if ((rio_em_d_log == int_tests[p].em_event) ||
				(rio_em_i_init_fail == int_tests[p].em_event)) {
				c_e[1].port_num = RIO_ALL_PORTS;
			} else {
				c_e[1].port_num = port;
			}

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event interrupt status
			in_i.ptl.num_ports = 1;
			in_i.ptl.pnums[0] = port;
			in_i.num_events = (uint8_t)rio_em_last;
			in_i.events = stat_i;
			memset(stat_i, 0xFF, sizeof(stat_i));

			out_i.imp_rc = 0;
			out_i.num_events = 0xFF;
			out_i.too_many = true;
			out_i.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(2, out_i.num_events);
			assert_false(out_i.too_many);
			assert_false(out_i.other_events);

			// Check that events created are found...
			find_event(&c_e[0], stat_i, 2);
			find_event(&c_e[1], stat_i, 2);

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = port;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_p;
			memset(stat_p, 0xFF, sizeof(stat_p));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = true;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(2, out_p.num_events);
			assert_false(out_p.too_many);
			assert_false(out_p.other_events);

			// Check that events created are found...
			find_event(&c_e[0], stat_p, 2);
			find_event(&c_e[1], stat_p, 2);

			// Clear all port-write events...
			in_c.num_events = out_p.num_events;
			in_c.events = in_p.events;

			out_c.imp_rc = 0xFFFF;
			out_c.failure_idx = 0xFF;
			out_c.pw_events_remain = true;
			out_c.int_events_remain = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_clr_events(&mock_dev_info,
							&in_c, &out_c));
			assert_int_equal(0, out_c.imp_rc);
			assert_int_equal(0, out_c.failure_idx);
			assert_false(out_c.pw_events_remain);
			assert_false(out_c.int_events_remain);

			// Query the event interrupt status, confirm they're
			// gone.
			in_i.ptl.num_ports = 1;
			in_i.ptl.pnums[0] = port;
			in_i.num_events = (uint8_t)rio_em_last;
			in_i.events = stat_i;
			memset(stat_i, 0xFF, sizeof(stat_i));

			out_i.imp_rc = 0;
			out_i.num_events = 0xFF;
			out_i.too_many = false;
			out_i.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(0, out_i.num_events);
			assert_false(out_i.too_many);
			assert_false(out_i.other_events);

			// Query port-write events, confirm they're gone
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = port;
			in_p.pw_port_num = RIO_ALL_PORTS;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_p;
			memset(stat_p, 0xFF, sizeof(stat_p));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = false;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(0, out_p.num_events);
			assert_false(out_p.too_many);
			assert_false(out_p.other_events);
		}
	}

	(void)state;
}

int main(int argc, char** argv)
{
	memset(&st, 0, sizeof(st));
	st.argc = argc;
	st.argv = argv;

	const struct CMUnitTest tests[] = {
	cmocka_unit_test_setup(
			rxs_rio_em_dev_rpt_ctl_success_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_dev_rpt_ctl_oddport_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_dev_rpt_ctl_bad_parms_test,
			setup),

	cmocka_unit_test_setup(
			rxs_em_cfg_pw_success_test,
			setup),
	cmocka_unit_test_setup(
			rxs_em_cfg_pw_bad_parms_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_cfg_pw_retx_compute_test,
			setup),

	cmocka_unit_test_setup(
			rxs_rio_em_cfg_set_success_em_info_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_cfg_set_roundup_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_cfg_set_ignore_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_cfg_set_fail_em_info_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_cfg_set_bad_parms_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_cfg_get_bad_parms_test,
			setup),

	cmocka_unit_test_setup(
			rxs_rio_em_parse_pw_no_events_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_parse_pw_all_events_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_parse_pw_oth_events_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_parse_pw_bad_parms_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_create_events_bad_parms_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_create_events_success_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_create_ignored_events_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_get_int_stat_bad_parms_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_get_int_stat_success_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_get_int_stat_other_events_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_get_pw_stat_bad_parms_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_get_pw_stat_success_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_get_pw_stat_other_events_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_get_int_pw_stat_other_events_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_get_int_pw_stat_both_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_clr_int_events_success_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_clr_pw_events_success_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_clr_int_events_other_events_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_clr_pw_events_other_events_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_clr_int_pw_events_other_events_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_clr_events_bad_parms_test,
			setup),
	cmocka_unit_test_setup(
			rxs_rio_em_clr_int_pw_events_both_test,
			setup),
	};

	return cmocka_run_group_tests(tests, grp_setup, grp_teardown);
}

#endif /* RXS_DAR_WANTED */

#ifdef __cplusplus
}
#endif
