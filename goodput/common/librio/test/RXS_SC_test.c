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
#include "RXS2448.h"
#include "src/RXS_DeviceDriver.c"
#include "src/RXS_RT.c"
#include "rio_ecosystem.h"
#include "tok_parse.h"
#include "libcli.h"
#include "rio_mport_lib.h"

#define DEBUG_PRINTF 0
#define DEBUG_REGTRACE 0

#include "common_src/RXS_cmdline.c"
#include "common_src/RXS_reg_emulation.c"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RXS_DAR_WANTED

static void rxs_not_supported_test(void **state)
{
	(void)state; // not used
}

int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++;// not used

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(rxs_not_supported_test)};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif /* RXS_DAR_WANTED */

#ifdef RXS_DAR_WANTED

static void rxs_init_ctrs(rio_sc_init_dev_ctrs_in_t *parms_in)
{
	uint8_t pnum;

	parms_in->ptl.num_ports = RIO_ALL_PORTS;
	for (pnum = 0; pnum < RXS2448_MAX_PORTS; pnum++) {
		parms_in->ptl.pnums[pnum] = 0x00;
	}

	parms_in->dev_ctrs = mock_dev_ctrs;
	parms_in->dev_ctrs->p_ctrs = pp_ctrs;
	parms_in->dev_ctrs->num_p_ctrs = RXS2448_MAX_PORTS;
}

static void rxs_init_dev_ctrs_test_success(void **state)
{
	rio_sc_init_dev_ctrs_in_t mock_sc_in;
	rio_sc_init_dev_ctrs_out_t mock_sc_out;
	int i, j;

	// Success case, all ports
	rxs_init_ctrs(&mock_sc_in);

	assert_int_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	assert_int_equal(24, mock_sc_in.dev_ctrs->valid_p_ctrs);
	for (i = 0; i < 24; i++) {
		assert_int_equal(i, mock_sc_in.dev_ctrs->p_ctrs[i].pnum);
		assert_int_equal(RXS2448_MAX_SC,
				mock_sc_in.dev_ctrs->p_ctrs[i].ctrs_cnt);
		for (j = 0; j < RXS2448_MAX_SC; j++) {
			assert_int_equal(0,
					mock_sc_in.dev_ctrs->p_ctrs[i].ctrs[j].total);
			assert_int_equal(0,
					mock_sc_in.dev_ctrs->p_ctrs[i].ctrs[j].last_inc);
			assert_int_equal(rio_sc_disabled,
					mock_sc_in.dev_ctrs->p_ctrs[i].ctrs[j].sc);
			assert_int_equal(false,
					mock_sc_in.dev_ctrs->p_ctrs[i].ctrs[j].tx);
			assert_int_equal(true,
					mock_sc_in.dev_ctrs->p_ctrs[i].ctrs[j].srio);
		}
	}
	(void)state; // unused
}

static void rxs_init_dev_ctrs_test_bad_ptrs(void **state)
{
	rio_sc_init_dev_ctrs_in_t mock_sc_in;
	rio_sc_init_dev_ctrs_out_t mock_sc_out;

	// Test invalid dev_ctrs pointer
	rxs_init_ctrs(&mock_sc_in);
	mock_sc_in.dev_ctrs = NULL;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	// Test invalid dev_ctrs->p_ctrs pointer
	rxs_init_ctrs(&mock_sc_in);
	mock_sc_in.dev_ctrs->p_ctrs = NULL;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	(void)state; // unused
}

static void rxs_init_dev_ctrs_test_bad_p_ctrs(void **state)
{
	rio_sc_init_dev_ctrs_in_t mock_sc_in;
	rio_sc_init_dev_ctrs_out_t mock_sc_out;

	// Test invalid number of p_ctrs
	rxs_init_ctrs(&mock_sc_in);
	mock_sc_in.dev_ctrs->num_p_ctrs = 0;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	rxs_init_ctrs(&mock_sc_in);
	mock_sc_in.dev_ctrs->num_p_ctrs = RXS2448_MAX_PORTS + 1;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	rxs_init_ctrs(&mock_sc_in);
	mock_sc_in.dev_ctrs->valid_p_ctrs = RXS2448_MAX_PORTS + 1;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	(void)state; // unused
}

static void rxs_init_dev_ctrs_test_bad_ptl_1(void **state)
{
	rio_sc_init_dev_ctrs_in_t mock_sc_in;
	rio_sc_init_dev_ctrs_out_t mock_sc_out;

	// Test that a bad Port list is reported correctly.
	rxs_init_ctrs(&mock_sc_in);
	mock_sc_in.ptl.num_ports = 3;
	mock_sc_in.ptl.pnums[0] = 1;
	mock_sc_in.ptl.pnums[1] = 3;
	mock_sc_in.ptl.pnums[2] = 24;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	(void)state; // unused
}

static void rxs_init_dev_ctrs_test_bad_ptl_2(void **state)
{
	rio_sc_init_dev_ctrs_in_t mock_sc_in;
	rio_sc_init_dev_ctrs_out_t mock_sc_out;

	rxs_init_ctrs(&mock_sc_in);
	mock_sc_in.ptl.num_ports = 3;
	mock_sc_in.ptl.pnums[0] = -1;
	mock_sc_in.ptl.pnums[1] = 3;
	mock_sc_in.ptl.pnums[2] = 5;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	(void)state; // unused
}

static void rxs_init_dev_ctrs_test_bad_ptl_3(void **state)
{
	rio_sc_init_dev_ctrs_in_t mock_sc_in;
	rio_sc_init_dev_ctrs_out_t mock_sc_out;

	rxs_init_ctrs(&mock_sc_in);
	mock_sc_in.ptl.num_ports = 3;
	mock_sc_in.ptl.pnums[0] = 5;
	mock_sc_in.ptl.pnums[1] = 3;
	mock_sc_in.ptl.pnums[2] = 5;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	(void)state; // unused
}

static void rxs_init_dev_ctrs_test_good_ptl(void **state)
{
	rio_sc_init_dev_ctrs_in_t mock_sc_in;
	rio_sc_init_dev_ctrs_out_t mock_sc_out;
	int i, j;

	// Test Port list with a few good entries...
	rxs_init_ctrs(&mock_sc_in);
	mock_sc_in.ptl.num_ports = 3;
	mock_sc_in.ptl.pnums[0] = 1;
	mock_sc_in.ptl.pnums[1] = 3;
	mock_sc_in.ptl.pnums[2] = 23;
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	assert_int_equal(3, mock_sc_in.dev_ctrs->valid_p_ctrs);
	assert_int_equal(1, mock_sc_in.dev_ctrs->p_ctrs[0].pnum);
	assert_int_equal(RXS2448_MAX_SC,
			mock_sc_in.dev_ctrs->p_ctrs[0].ctrs_cnt);
	assert_int_equal(3, mock_sc_in.dev_ctrs->p_ctrs[1].pnum);
	assert_int_equal(RXS2448_MAX_SC,
			mock_sc_in.dev_ctrs->p_ctrs[1].ctrs_cnt);
	assert_int_equal(23, mock_sc_in.dev_ctrs->p_ctrs[2].pnum);
	assert_int_equal(RXS2448_MAX_SC,
			mock_sc_in.dev_ctrs->p_ctrs[2].ctrs_cnt);

	for (i = 0; i < 3; i++) {
		for (j = 0; j < RXS2448_MAX_SC; j++) {
			assert_int_equal(0,
					mock_sc_in.dev_ctrs->p_ctrs[i].ctrs[j].total);
			assert_int_equal(0,
					mock_sc_in.dev_ctrs->p_ctrs[i].ctrs[j].last_inc);
			assert_int_equal(rio_sc_disabled,
					mock_sc_in.dev_ctrs->p_ctrs[i].ctrs[j].sc);
			assert_int_equal(false,
					mock_sc_in.dev_ctrs->p_ctrs[i].ctrs[j].tx);
			assert_int_equal(true,
					mock_sc_in.dev_ctrs->p_ctrs[i].ctrs[j].srio);
		}
	}

	(void)state; // unused
}

#define MAX_SC_CFG_VAL 21

static void test_rxs_cfg_dev_ctr(void **state, rio_sc_cfg_rxs_ctr_in_t *mock_sc_in,
		int sc_cfg)
{
	bool tx = true;
	uint32_t reg_val = 0;
	int ctr_idx;
	rio_sc_cfg_rxs_ctr_out_t mock_sc_out;
	rio_port_t st_pt, end_pt, port;
	bool srio = true;
	bool expect_fail = false;
	uint32_t cdata;
	RXS_test_state_t *RXS = *(RXS_test_state_t **)state;
	bool check_ctr = true;

	// Pick out a test value.  Test values cover all valid request
	// parameters, and 3 invalid combinations.
	//
	// Note: The first 8 values are the default configuration for packet
	// counters, used by other packet counter tests...
	mock_sc_in->tx = tx;
	switch (sc_cfg) {
	case 0:
		mock_sc_in->ctr_type = rio_sc_pkt;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_RIO_PKT;
		break;
	case 1:
		mock_sc_in->ctr_type = rio_sc_pkt;
		mock_sc_in->tx = !mock_sc_in->tx;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_RIO_PKT;
		break;
	case 2:
		mock_sc_in->ctr_type = rio_sc_fab_pkt;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_FAB_PKT;
		srio = false;
		break;
	case 3:
		mock_sc_in->ctr_type = rio_sc_fab_pkt;
		mock_sc_in->tx = !mock_sc_in->tx;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_FAB_PKT;
		srio = false;
		break;
	case 4:
		mock_sc_in->ctr_type = rio_sc_rio_pload;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_RIO_PAYLOAD;
		break;
	case 5:
		mock_sc_in->ctr_type = rio_sc_rio_pload;
		mock_sc_in->tx = !mock_sc_in->tx;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_RIO_PAYLOAD;
		break;
	case 6:
		mock_sc_in->ctr_type = rio_sc_rio_bwidth;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_RIO_TTL_PKTCNTR;
		// RIO BANDWIDTH is a free running counter, incremented on
		// every TX clock cycle.  The counter will never be 0 on
		// hardware.
		check_ctr = !RXS->real_hw;
		break;
	case 7:
		mock_sc_in->ctr_type = rio_sc_disabled;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_DISABLED;
		break;
	case 8:
		mock_sc_in->ctr_type = rio_sc_fab_pload;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_FAB_PAYLOAD;
		srio = false;
		break;
	case 9:
		mock_sc_in->ctr_type = rio_sc_fab_pload;
		mock_sc_in->tx = !mock_sc_in->tx;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_FAB_PAYLOAD;
		srio = false;
		break;
	case 10:
		mock_sc_in->ctr_type = rio_sc_retries;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_RETRIES;
		break;
	case 11:
		mock_sc_in->ctr_type = rio_sc_retries;
		mock_sc_in->tx = !mock_sc_in->tx;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_RETRIES;
		break;
	case 12:
		mock_sc_in->ctr_type = rio_sc_pna;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_PNA;
		break;
	case 13:
		mock_sc_in->ctr_type = rio_sc_pna;
		mock_sc_in->tx = !mock_sc_in->tx;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_PNA;
		break;
	case 14:
		mock_sc_in->ctr_type = rio_sc_pkt_drop;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_PKT_DROP;
		break;
	case 15:
		mock_sc_in->ctr_type = rio_sc_pkt_drop;
		mock_sc_in->tx = !mock_sc_in->tx;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_PKT_DROP;
		break;
	case 16:
		mock_sc_in->ctr_type = rio_sc_fab_pkt;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_FAB_PKT;
		mock_sc_in->prio_mask = 0;
		expect_fail = true;
		break;
	case 17:
		mock_sc_in->ctr_type = rio_sc_rio_bwidth;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_FAB_PAYLOAD;
		mock_sc_in->tx = !mock_sc_in->tx;
		mock_sc_in->prio_mask = 0;
		expect_fail = true;
		break;
	case 18:
		mock_sc_in->ctr_type = rio_sc_fab_pload;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_FAB_PAYLOAD;
		mock_sc_in->prio_mask = 0;
		expect_fail = true;
		break;
	case 19:
		mock_sc_in->ctr_type = rio_sc_pkt_drop;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_PKT_DROP;
		mock_sc_in->prio_mask = 0;
		expect_fail = true;
		break;
	case 20:
		mock_sc_in->ctr_type = rio_sc_uc_req_pkts;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_DISABLED;
		expect_fail = true;
		break;
	case MAX_SC_CFG_VAL:
		mock_sc_in->ctr_type = rio_sc_pkt_drop_ttl;
		reg_val = RXS_SPX_PCNTR_CTL_SEL_DISABLED;
		expect_fail = true;
		break;
	}
	// Get port range to check
	if (RIO_ALL_PORTS == mock_sc_in->ptl.num_ports) {
		st_pt = 0;
		end_pt = RXS2448_MAX_PORTS - 1;
	} else {
		st_pt = end_pt = mock_sc_in->ptl.pnums[0];
	}
	ctr_idx = mock_sc_in->ctr_idx;

	// Initialize test register values for all ports 
	for (port = st_pt; port <= end_pt; port++) {
		// Zero control register for the port
		assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info, RXS_SPX_PCNTR_EN(port), 0));

		// Set invalid control value
		assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info, RXS_SPX_PCNTR_CTL(port, ctr_idx), RXS_SPX_PCNTR_CTL_SEL_DISABLED));

		// Set non-zero counter value for the port
		assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info, RXS_SPX_PCNTR_CNT(port, ctr_idx), 0x12345678));
	}

	// If something is expected to fail, do not do any more checking.
	if (expect_fail) {
		mock_sc_out.imp_rc = RIO_SUCCESS;

		assert_int_not_equal(RIO_SUCCESS,
				rio_sc_cfg_rxs_ctr(&mock_dev_info, mock_sc_in,
						&mock_sc_out));
		assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
		return;
	}

	// If something is expected to work, do exhaustive checking
	mock_sc_out.imp_rc = !RIO_SUCCESS;

	assert_int_equal(RIO_SUCCESS,
			rio_sc_cfg_rxs_ctr(&mock_dev_info, mock_sc_in,
					&mock_sc_out));
	assert_int_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	for (port = st_pt; port <= end_pt; port++) {
		uint32_t reg_val_temp;
		uint32_t mask_temp;

		// Check counter data structure
		assert_int_equal(port, mock_sc_in->dev_ctrs->p_ctrs[port].pnum);
		assert_int_equal(
				mock_sc_in->dev_ctrs->p_ctrs[port].ctrs[ctr_idx].sc,
				mock_sc_in->ctr_type);
		assert_int_equal(0,
				mock_sc_in->dev_ctrs->p_ctrs[port].ctrs[ctr_idx].total);
		assert_int_equal(0,
				mock_sc_in->dev_ctrs->p_ctrs[port].ctrs[ctr_idx].last_inc);
		assert_int_equal(mock_sc_in->tx,
				mock_sc_in->dev_ctrs->p_ctrs[port].ctrs[ctr_idx].tx);
		assert_int_equal(srio,
				mock_sc_in->dev_ctrs->p_ctrs[port].ctrs[ctr_idx].srio);

		// Check register values.
		assert_int_equal(RIO_SUCCESS,
				DARRegRead(&mock_dev_info, RXS_SPX_PCNTR_EN(port), &cdata));
		assert_int_equal(cdata, RXS_SPX_PCNTR_EN_ENABLE);

		// Check control value
		assert_int_equal(RIO_SUCCESS,
				DARRegRead(&mock_dev_info, RXS_SPX_PCNTR_CTL(port, ctr_idx), &cdata));
		mask_temp = mock_sc_in->prio_mask << 8;
		mask_temp &= RXS_SPC_PCNTR_CTL_PRIO;
		reg_val_temp = reg_val;
		reg_val_temp |= mock_sc_in->tx ? RXS_SPX_PCNTR_CTL_TX : 0;
		reg_val_temp |= mask_temp;
		assert_int_equal(cdata, reg_val_temp);

		// Check counter value
		if ((port != RXS->conn_port) && (check_ctr)) {
			assert_int_equal(RIO_SUCCESS,
					DARRegRead(&mock_dev_info, RXS_SPX_PCNTR_CNT(port, ctr_idx), &cdata));
			assert_int_equal(cdata, 0);
		}
	}
}

static void rxs_cfg_dev_ctrs_test_per_port(void **state)
{
	int val;
	rio_sc_init_dev_ctrs_in_t init_in;
	rio_sc_init_dev_ctrs_out_t init_out;
	rio_sc_cfg_rxs_ctr_in_t mock_sc_in;

	// Initialize counters for all ports...
	rxs_init_ctrs(&init_in);

	assert_int_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &init_in,
					&init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Loop through each counter on each port, and each value,
	// and check counter values and registers...
	mock_sc_in.ptl.num_ports = 1;
	mock_sc_in.dev_ctrs = mock_dev_ctrs;
	mock_sc_in.ctr_en = true;

	for (mock_sc_in.ptl.pnums[0] = 0;
			mock_sc_in.ptl.pnums[0] < RXS2448_MAX_PORTS;
			mock_sc_in.ptl.pnums[0]++) {
		for (mock_sc_in.ctr_idx = 0;
				mock_sc_in.ctr_idx < RXS2448_MAX_SC;
				++mock_sc_in.ctr_idx) {
			for (val = 0; val < MAX_SC_CFG_VAL; val++) {
				mock_sc_in.prio_mask = FIRST_BYTE_MASK;
				test_rxs_cfg_dev_ctr(state, &mock_sc_in, val);
			}
		}
	}
	(void)state; // unused
}

static void rxs_cfg_dev_ctrs_test_all_ports(void **state)
{
	int val;
	rio_sc_init_dev_ctrs_in_t init_in;
	rio_sc_init_dev_ctrs_out_t init_out;
	rio_sc_cfg_rxs_ctr_in_t mock_sc_in;

	// Initialize counters for all ports...
	rxs_init_ctrs(&init_in);
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &init_in,
					&init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Loop through each counter on each port, and each value,
	// and check counter values and registers...
	mock_sc_in.ptl.num_ports = RIO_ALL_PORTS;
	mock_sc_in.dev_ctrs = mock_dev_ctrs;
	mock_sc_in.ctr_en = true;

	for (mock_sc_in.ctr_idx = 0; mock_sc_in.ctr_idx < RXS2448_MAX_SC;
			++mock_sc_in.ctr_idx) {
		for (val = 0; val < MAX_SC_CFG_VAL; val++) {
			mock_sc_in.prio_mask = FIRST_BYTE_MASK;
			test_rxs_cfg_dev_ctr(state, &mock_sc_in, val);
		}
	}
	(void)state; // unused
}

// Program counters to default configuration.
//
static void rxs_cfg_dev_ctrs_test_default(void **state)
{
	rio_sc_init_dev_ctrs_in_t init_in;
	rio_sc_init_dev_ctrs_out_t init_out;
	rio_sc_cfg_rxs_ctr_in_t mock_sc_in;

	// Initialize counters for all ports...
	rxs_init_ctrs(&init_in);
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &init_in,
					&init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	mock_sc_in.ptl.num_ports = RIO_ALL_PORTS;
	mock_sc_in.dev_ctrs = mock_dev_ctrs;
	mock_sc_in.ctr_en = true;

	for (mock_sc_in.ctr_idx = 0; mock_sc_in.ctr_idx < RXS2448_MAX_SC;
			++mock_sc_in.ctr_idx) {
		mock_sc_in.prio_mask = FIRST_BYTE_MASK;
		test_rxs_cfg_dev_ctr(state, &mock_sc_in, mock_sc_in.ctr_idx);
	}
	(void)state; // unused
}

static void rxs_init_read_ctrs(void **state, rio_sc_read_ctrs_in_t *parms_in)
{
	uint8_t val;
	rio_sc_cfg_rxs_ctr_in_t mock_sc_in;

	parms_in->ptl.num_ports = RIO_ALL_PORTS;
	parms_in->dev_ctrs = mock_dev_ctrs;

	// Loop through each counter on each port, 
	// programming the counter control value.
	mock_sc_in.ptl.num_ports = RIO_ALL_PORTS;
	mock_sc_in.dev_ctrs = parms_in->dev_ctrs;
	mock_sc_in.ctr_en = true;

	for (mock_sc_in.ctr_idx = 0; mock_sc_in.ctr_idx < RXS2448_MAX_SC;
			++mock_sc_in.ctr_idx) {
		for (val = 0; val < MAX_SC_CFG_VAL; val++) {
			mock_sc_in.prio_mask = FIRST_BYTE_MASK;
			test_rxs_cfg_dev_ctr(state, &mock_sc_in, val);
		}
	}
}

static void rxs_read_dev_ctrs_test(void **state)
{
	rio_sc_read_ctrs_in_t mock_sc_in;
	rio_sc_read_ctrs_out_t mock_sc_out;
	rio_sc_init_dev_ctrs_in_t init_in;
	rio_sc_init_dev_ctrs_out_t init_out;
	unsigned int ctr_idx, port;
	uint32_t cdata;
	uint32_t rval;
	uint32_t st_val = 0x10;
	RXS_test_state_t *RXS = *(RXS_test_state_t **)state;

	// Initialize counters structure
	rxs_init_ctrs(&init_in);
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &init_in,
					&init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Set up counters 
	rxs_init_read_ctrs(state, &mock_sc_in);

	// Set up counter registers
	for (port = 0; port < RXS2448_MAX_PORTS; port++) {
		for (ctr_idx = 0; ctr_idx < RXS2448_MAX_SC; ctr_idx++) {
			// Set non-zero counter value for the port
			rval = st_val + (port * RXS2448_MAX_SC) + ctr_idx;
			assert_int_equal(RIO_SUCCESS,
					DARRegWrite( &mock_dev_info, RXS_SPX_PCNTR_CNT(port, ctr_idx), rval));
		}
	}

	// Check for successfull reads...
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	// Check counter values... 
	for (port = 0; port < RXS2448_MAX_PORTS; port++) {
		// Counters on the port that is used to access the switch
		// cannot be checked due to actions of the unit test.
		if (RXS->real_hw && (port == RXS->conn_port)) {
			continue;
		}
		for (ctr_idx = 0; ctr_idx < RXS2448_MAX_SC; ctr_idx++) {
			// Check the counter value for the port...
			assert_int_equal(RIO_SUCCESS,
					DARRegRead( &mock_dev_info, RXS_SPX_PCNTR_CNT(port, ctr_idx), &cdata));
			// Do not read disabled counters, they should always
			// be zero.
			rval = st_val + (port * RXS2448_MAX_SC) + ctr_idx;
			if (rio_sc_disabled == pp_ctrs[port].ctrs[ctr_idx].sc) {
				rval = 0;
			}
			assert_int_equal(rval,
					pp_ctrs[port].ctrs[ctr_idx].last_inc);
			assert_int_equal(rval,
					pp_ctrs[port].ctrs[ctr_idx].total);
			if (rval) {
				assert_int_equal(rval, cdata);
			}
		}
	}

	// Increment counter registers
	for (port = 0; port < RXS2448_MAX_PORTS; port++) {
		for (ctr_idx = 0; ctr_idx < RXS2448_MAX_SC; ctr_idx++) {
			// Set non-zero counter value for the port
			rval = st_val + (port * RXS2448_MAX_SC) + ctr_idx;
			rval = rval * 3;
			assert_int_equal(RIO_SUCCESS,
					DARRegWrite( &mock_dev_info, RXS_SPX_PCNTR_CNT(port, ctr_idx), rval));
		}
	}

	// Check for successfull reads...
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	// Check counter values... 
	for (port = 0; port < RXS2448_MAX_PORTS; port++) {
		// Counters on the port that is used to access the switch
		// cannot be checked due to actions of the unit test.
		if (RXS->real_hw && (port == RXS->conn_port)) {
			continue;
		}
		for (ctr_idx = 0; ctr_idx < RXS2448_MAX_SC; ctr_idx++) {
			// Check the counter value for the port...
			rval = st_val + (port * RXS2448_MAX_SC) + ctr_idx;
			assert_int_equal(RIO_SUCCESS,
					DARRegRead( &mock_dev_info, RXS_SPX_PCNTR_CNT(port, ctr_idx), &cdata));
			// Do not read disabled counters, they should always
			// be zero.
			if (rio_sc_disabled == pp_ctrs[port].ctrs[ctr_idx].sc) {
				rval = 0;
			}
			assert_int_equal(3 * rval,
					pp_ctrs[port].ctrs[ctr_idx].total);
			assert_int_equal(2 * rval,
					pp_ctrs[port].ctrs[ctr_idx].last_inc);
			if (rval) {
				assert_int_equal(3 * rval, cdata);
			}
		}
	}

	// Decrement counter registers, check for wrap around handling...
	for (port = 0; port < RXS2448_MAX_PORTS; port++) {
		// Counters on the port that is used to access the switch
		// cannot be checked due to actions of the unit test.
		if (RXS->real_hw && (port == RXS->conn_port)) {
			continue;
		}
		for (ctr_idx = 0; ctr_idx < RXS2448_MAX_SC; ctr_idx++) {
			// Set non-zero counter value for the port
			rval = st_val + (port * RXS2448_MAX_SC) + ctr_idx;
			assert_int_equal(RIO_SUCCESS,
					DARRegWrite( &mock_dev_info, RXS_SPX_PCNTR_CNT(port, ctr_idx), rval));
		}
	}

	// Check for successfull reads...
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	// Check counter values... 
	for (port = 0; port < RXS2448_MAX_PORTS; port++) {
		// Counters on the port that is used to access the switch
		// cannot be checked due to actions of the unit test.
		if (RXS->real_hw && (port == RXS->conn_port)) {
			continue;
		}
		for (ctr_idx = 0; ctr_idx < RXS2448_MAX_SC; ctr_idx++) {
			uint64_t base = (uint64_t)0x100000000;
			// Check the counter value for the port...
			rval = st_val + (port * RXS2448_MAX_SC) + ctr_idx;
			assert_int_equal(RIO_SUCCESS,
					DARRegRead( &mock_dev_info, RXS_SPX_PCNTR_CNT(port, ctr_idx), &cdata));
			// Do not read disabled counters, they should always
			// be zero.
			if (rio_sc_disabled == pp_ctrs[port].ctrs[ctr_idx].sc) {
				rval = 0;
				base = 0;
			}
			assert_int_equal(base + rval,
					pp_ctrs[port].ctrs[ctr_idx].total);
			assert_int_equal(base - (2 * rval),
					pp_ctrs[port].ctrs[ctr_idx].last_inc);
			if (rval) {
				assert_int_equal(rval, cdata);
			}
		}
	}
	(void)state; // unused
}

static void rxs_read_dev_ctrs_test_bad_parms1(void **state)
{
	rio_sc_read_ctrs_in_t mock_sc_in;
	rio_sc_read_ctrs_out_t mock_sc_out;
	rio_sc_init_dev_ctrs_in_t init_in;
	rio_sc_init_dev_ctrs_out_t init_out;

	// Initialize counters structure
	rxs_init_ctrs(&init_in);
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &init_in,
					&init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Set up counters 
	rxs_init_read_ctrs(state, &mock_sc_in);

	// Now try some bad parameters/failure test cases
	mock_sc_in.ptl.num_ports = RXS2448_MAX_PORTS + 1;
	mock_sc_out.imp_rc = RIO_SUCCESS;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	mock_sc_in.ptl.num_ports = 1;
	mock_sc_in.ptl.pnums[0] = RXS2448_MAX_PORTS + 1;
	mock_sc_out.imp_rc = RIO_SUCCESS;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	mock_sc_in.ptl.num_ports = RIO_ALL_PORTS;
	mock_sc_in.dev_ctrs->p_ctrs = NULL;
	mock_sc_out.imp_rc = RIO_SUCCESS;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	mock_sc_in.dev_ctrs = NULL;
	mock_sc_out.imp_rc = RIO_SUCCESS;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	(void)state; // unused
}

static void rxs_read_dev_ctrs_test_bad_parms2(void **state)
{
	rio_sc_read_ctrs_in_t mock_sc_in;
	rio_sc_read_ctrs_out_t mock_sc_out;
	rio_sc_init_dev_ctrs_in_t init_in;
	rio_sc_init_dev_ctrs_out_t init_out;

	// Try to read a port that is not in the port list.
	rxs_init_ctrs(&init_in);
	init_in.ptl.num_ports = 3;
	init_in.ptl.pnums[0] = 1;
	init_in.ptl.pnums[1] = 2;
	init_in.ptl.pnums[2] = 3;
	init_in.dev_ctrs = mock_dev_ctrs;
	init_in.dev_ctrs->p_ctrs = pp_ctrs;

	assert_int_equal(RIO_SUCCESS,
			rxs_rio_sc_init_dev_ctrs(&mock_dev_info, &init_in,
					&init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	mock_sc_in.ptl.num_ports = 1;
	mock_sc_in.ptl.pnums[0] = 5;
	mock_sc_in.dev_ctrs = mock_dev_ctrs;
	mock_sc_out.imp_rc = RIO_SUCCESS;
	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	(void)state; // unused
}

static void rxs_init_read_dev_ctrs_test(void **state)
{
	rxs_init_dev_ctrs_test_success(state);
	rxs_read_dev_ctrs_test(state);

	(void)state; // unused
}

static void rxs_init_cfg_read_dev_ctrs_test(void **state)
{
	rxs_init_dev_ctrs_test_success(state);
	rxs_cfg_dev_ctrs_test_default(state);
	rxs_read_dev_ctrs_test(state);

	(void)state; // unused
}

int main(int argc, char** argv)
{
	const struct CMUnitTest tests[] = {
			cmocka_unit_test_setup_teardown(
					rxs_init_dev_ctrs_test_success, setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_init_dev_ctrs_test_bad_ptrs, setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_init_dev_ctrs_test_bad_p_ctrs,
					setup, NULL),
			cmocka_unit_test_setup_teardown(
					rxs_init_dev_ctrs_test_bad_ptl_1, setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_init_dev_ctrs_test_bad_ptl_2, setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_init_dev_ctrs_test_bad_ptl_3, setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_init_dev_ctrs_test_good_ptl, setup,
					NULL),

			cmocka_unit_test_setup_teardown(
					rxs_cfg_dev_ctrs_test_per_port, setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_cfg_dev_ctrs_test_all_ports, setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_cfg_dev_ctrs_test_default, setup,
					NULL),

			cmocka_unit_test_setup_teardown(rxs_read_dev_ctrs_test, setup, NULL),
			cmocka_unit_test_setup_teardown(
					rxs_read_dev_ctrs_test_bad_parms1,
					setup, NULL),
			cmocka_unit_test_setup_teardown(
					rxs_read_dev_ctrs_test_bad_parms2,
					setup, NULL),
			cmocka_unit_test_setup_teardown(
					rxs_init_read_dev_ctrs_test, setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_init_cfg_read_dev_ctrs_test, setup,
					NULL)
	};

	memset(&st, 0, sizeof(st));
	st.argc = argc;
	st.argv = argv;

	return cmocka_run_group_tests(tests, grp_setup, grp_teardown);
}

#endif /* RXS_DAR_WANTED */

#ifdef __cplusplus
}
#endif
