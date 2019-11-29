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
#include "rio_ecosystem.h"
#include "tok_parse.h"
#include "libcli.h"
#include "rio_mport_lib.h"

#include "src/RXS_RT.c"

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

static void rxs_init_mock_rt(rio_rt_state_t *rt)
{
	memset(rt, 0xFF, sizeof(rio_rt_state_t));
}

static void rxs_chk_and_corr_rtv_success_test_x(uint32_t imp_spec)
{
	rio_rt_uc_info_t tst;
	uint32_t idx;
	uint32_t bools;
	uint32_t tst_rte;

	// Verify all valid ports are allowed
	for (idx = 0; idx < NUM_RXS_PORTS(&mock_dev_info);  idx++) {
		for (bools = 0; bools < 4; bools++) {
			bool dom = (bools & 2) ? true : false;
			bool dflt = (bools & 1) ? true : false;

			tst_rte = imp_spec | RIO_RTV_PORT(idx);
			tst.rte_val = tst_rte;
			tst.changed = false;

			rxs_chk_and_corr_rtv(&mock_dev_info, &tst, dom, dflt);
			assert_false(tst.changed);
			assert_int_equal(tst.rte_val, tst_rte);
		}
	}

	// Verify all invalid ports are detected
	for (idx = NUM_RXS_PORTS(&mock_dev_info); idx <= 0xFF; idx++) {
		for (bools = 0; bools < 4; bools++) {
			bool dom = (bools & 2) ? true : false;
			bool dflt = (bools & 1) ? true : false;

			tst_rte = imp_spec | RIO_RTV_PORT(idx);
			tst.rte_val = tst_rte;
			tst.changed = false;
			rxs_chk_and_corr_rtv(&mock_dev_info, &tst, dom, dflt);
			assert_int_equal(tst.rte_val, RIO_RTE_DROP);
			assert_true(tst.changed);
		}
	}

	// Verify all multicst masks are allowed
	for (idx = 0; idx < RXS2448_MC_MASK_CNT;  idx++) {
		for (bools = 0; bools < 4; bools++) {
			bool dom = (bools & 2) ? true : false;
			bool dflt = (bools & 1) ? true : false;

			tst_rte = imp_spec | RIO_RTV_MC_MSK(idx);
			tst.rte_val = tst_rte;
			tst.changed = false;
			rxs_chk_and_corr_rtv(&mock_dev_info, &tst, dom, dflt);
			assert_int_equal(tst.rte_val, tst_rte);
			assert_false(tst.changed);
		}
	}

	// Verify RIO_RTE_LVL_G0 is allowed for domain values only
	tst.rte_val = RIO_RTE_LVL_G0 | imp_spec;
	tst.changed = false;
	rxs_chk_and_corr_rtv(&mock_dev_info, &tst, false, false);
	assert_int_equal(tst.rte_val, RIO_RTE_DROP);
	assert_true(tst.changed);

	tst.rte_val = RIO_RTE_LVL_G0 | imp_spec;
	tst.changed = false;
	rxs_chk_and_corr_rtv(&mock_dev_info, &tst, false, true);
	assert_int_equal(tst.rte_val, RIO_RTE_DROP);
	assert_true(tst.changed);

	tst.rte_val = RIO_RTE_LVL_G0 | imp_spec;
	tst.changed = false;
	rxs_chk_and_corr_rtv(&mock_dev_info, &tst, true, false);
	assert_int_equal(tst.rte_val, RIO_RTE_LVL_G0 | imp_spec);
	assert_false(tst.changed);

	tst.rte_val = RIO_RTE_LVL_G0 | imp_spec;
	tst.changed = false;
	rxs_chk_and_corr_rtv(&mock_dev_info, &tst, true, true);
	assert_int_equal(tst.rte_val, RIO_RTE_DROP);
	assert_true(tst.changed);

	// Verify RIO_RTE_LVL_G1 to last are not allowed
	for (idx = 1; idx < 0xFF;  idx++) {
		for (bools = 0; bools < 4; bools++) {
			bool dom = (bools & 2) ? true : false;
			bool dflt = (bools & 1) ? true : false;

			tst.rte_val = RIO_RTV_LVL_GRP(idx) | imp_spec;
			tst.changed = false;
			rxs_chk_and_corr_rtv(&mock_dev_info, &tst, dom, dflt);
			assert_int_equal(tst.rte_val, RIO_RTE_DROP);
			assert_true(tst.changed);
		}
	}

	// Verify RIO_RTE_DROP is allowed for all values
	for (bools = 0; bools < 4; bools++) {
		bool dom = (bools & 2) ? true : false;
		bool dflt = (bools & 1) ? true : false;

		tst.rte_val = RIO_RTE_DROP;
		tst.changed = false;
		rxs_chk_and_corr_rtv(&mock_dev_info, &tst, dom, dflt);
		assert_int_equal(tst.rte_val, RIO_RTE_DROP);
		assert_false(tst.changed);
	}

	// Verify RIO_RTE_DFLT_PORT is not allowed for the default port itself
	for (bools = 0; bools < 4; bools++) {
		bool dom = (bools & 2) ? true : false;
		bool dflt = (bools & 1) ? true : false;

		tst.rte_val = RIO_RTE_DFLT_PORT;
		tst.changed = false;
		rxs_chk_and_corr_rtv(&mock_dev_info, &tst, dom, dflt);
		if (dflt) {
			assert_int_equal(tst.rte_val, RIO_RTE_DROP);
			assert_true(tst.changed);
		} else {
			assert_int_equal(tst.rte_val, RIO_RTE_DFLT_PORT);
			assert_false(tst.changed);
		}
	}
}

static void rxs_chk_and_corr_rtv_success_test(void **state)
{
	uint64_t imp_spec;
	for (imp_spec = 0; imp_spec <= RIO_RTE_IMP_SPEC; imp_spec += 0x10000000)
	{
		rxs_chk_and_corr_rtv_success_test_x(imp_spec);
	}

	(void)state;
}

static void rxs_chk_and_corr_rtv_error_test(void **state)
{
	uint32_t tst_vals[] = {0x302, 0x1000, 0x20234, 0x33333, 0xFFFFFFFF};
	uint32_t num_tsts = sizeof(tst_vals)/sizeof(tst_vals[0]);

	uint32_t idx;
	uint32_t bools;
	rio_rt_uc_info_t tst;

	for (idx = 0; idx < num_tsts; idx++) {
		for (bools = 0; bools < 4; bools++) {
			bool dom = (bools & 2) ? true : false;
			bool dflt = (bools & 1) ? true : false;

			tst.rte_val = tst_vals[idx];
			tst.changed = false;
			rxs_chk_and_corr_rtv(&mock_dev_info, &tst, dom, dflt);
			assert_int_equal(tst.rte_val, RIO_RTE_DROP);
			assert_true(tst.changed);
		}
	}

	(void)state;
}

static void rxs_reg_dev_dom(uint32_t port, uint32_t rte_num,
			    uint32_t *dom_out, uint32_t *dev_out)
{
	uint32_t dev_rte_base, dom_rte_base;

	if (RIO_ALL_PORTS == port) {
		dev_rte_base = RXS_BC_L2_GX_ENTRYY_CSR(0, 0);
		dom_rte_base = RXS_BC_L1_GX_ENTRYY_CSR(0, 0);
	} else {
		dev_rte_base = RXS_SPX_L2_GY_ENTRYZ_CSR(port, 0, 0);
		dom_rte_base = RXS_SPX_L1_GY_ENTRYZ_CSR(port, 0, 0);
	}

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, DOM_RTE_ADDR(dom_rte_base, rte_num), dom_out));
	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, DEV_RTE_ADDR(dev_rte_base, rte_num), dev_out));
}

static void rxs_reg_mc_mask(uint32_t port, uint32_t mc_mask_num,
		uint32_t *mc_mask_out)
{
	uint32_t base_mask_addr;

	if (RIO_ALL_PORTS == port) {
		base_mask_addr = RXS_SPX_MC_Y_S_CSR(0, 0);
	} else {
		base_mask_addr = RXS_SPX_MC_Y_S_CSR(port, 0);
	}

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, MC_MASK_ADDR(base_mask_addr, mc_mask_num), mc_mask_out));
}

static void check_init_rt_regs_port(
		uint32_t chk_on_port,
		uint32_t chk_dflt_val,
		uint32_t chk_rt_val,
		uint32_t chk_mask,
		uint32_t chk_first_dom_val)
{
	uint32_t rt_num, temp, s_rt_num = 0;
	uint32_t dom_out, dev_out, mask_num, mc_mask_out;

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, RXS_ROUTE_DFLT_PORT, &temp));
	assert_int_equal(temp, chk_dflt_val);

	rxs_reg_dev_dom(chk_on_port, s_rt_num, &dom_out, &dev_out);
	assert_int_equal(chk_first_dom_val, dom_out);
	assert_int_equal(chk_rt_val, dev_out);
	s_rt_num++;
	for (rt_num = s_rt_num; rt_num < RIO_RT_GRP_SZ; rt_num++) {
		rxs_reg_dev_dom(chk_on_port, rt_num, &dom_out, &dev_out);
		assert_int_equal(chk_rt_val, dom_out);
		assert_int_equal(chk_rt_val, dev_out);
	}

	// Mask regs are always always set to 0...
	for (mask_num = 0; mask_num < RXS2448_MC_MASK_CNT; mask_num++) {
		rxs_reg_mc_mask(chk_on_port, mask_num, &mc_mask_out);
		assert_int_equal(chk_mask, mc_mask_out);
	}
}

static void check_init_rt_regs(void **state, uint32_t port, bool hw,
		rio_rt_initialize_in_t *mock_init_in)
{
	uint32_t st_p = port;
	uint32_t end_p = port;
	uint32_t chk_rt_val = (hw)?mock_init_in->default_route_table_port:0;
	uint32_t chk_dflt_val = (hw)?mock_init_in->default_route:0;
	uint32_t chk_first_idx_dom_val = RIO_RTE_LVL_G0;
	uint32_t chk_mask = 0;
	RXS_test_state_t *RXS = *(RXS_test_state_t **)state;

	// When running on real hardware, and the hardware has not been
	// updated, it is not possible to check the register values...
	if (RXS->real_hw && !hw) {
		return;
	}
	if (port == RIO_ALL_PORTS) {
		st_p = 0;
		end_p = NUM_RXS_PORTS(&mock_dev_info) - 1;
	}

	for (port = st_p; port <= end_p; port++) {
		check_init_rt_regs_port(port,
				chk_dflt_val, chk_rt_val,
				chk_mask, chk_first_idx_dom_val);
	}
}

static void check_init_struct(rio_rt_initialize_in_t *mock_init_in)
{
	uint32_t idx;

	assert_int_equal(mock_init_in->default_route,
		mock_init_in->rt->default_route);

	for (idx = 0; idx < RIO_RT_GRP_SZ; idx++) {
		assert_int_equal(mock_init_in->default_route_table_port,
				mock_init_in->rt->dev_table[idx].rte_val);
		if (mock_init_in->update_hw) {
			assert_false(mock_init_in->rt->dev_table[idx].changed);
		} else {
			assert_true(mock_init_in->rt->dev_table[idx].changed);
		}
	}

	for (idx = 1; idx < RIO_RT_GRP_SZ; idx++) {
		assert_int_equal(mock_init_in->default_route_table_port,
				mock_init_in->default_route_table_port);
		if (mock_init_in->update_hw) {
			assert_false(mock_init_in->rt->dom_table[idx].changed);
		} else {
			assert_true(mock_init_in->rt->dom_table[idx].changed);
		}
	}

	for (idx = 0; idx < RXS2448_MC_MASK_CNT; idx++) {
		assert_int_equal(0, mock_init_in->rt->mc_masks[idx].mc_destID);
		assert_int_equal(tt_dev8, mock_init_in->rt->mc_masks[idx].tt);
		assert_int_equal(0, mock_init_in->rt->mc_masks[idx].mc_mask);
		assert_false(mock_init_in->rt->mc_masks[idx].in_use);
		assert_false(mock_init_in->rt->mc_masks[idx].allocd);

		if (mock_init_in->update_hw) {
			assert_false(mock_init_in->rt->mc_masks[idx].changed);
		} else {
			assert_true(mock_init_in->rt->mc_masks[idx].changed);
		}
	}
}

static void rxs_init_rt_test_success_val(void **state, bool hw,
			rio_port_t port, pe_rt_val val)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_state_t rt;

	init_mock_rxs_reg(state);
	rxs_init_mock_rt(&rt);
	if (RIO_ALL_PORTS == port) {
		init_in.default_route = RIO_RTE_DROP;
	} else {
		init_in.default_route = port;
	}
	init_in.set_on_port = port;
	init_in.default_route_table_port = val;
	init_in.update_hw = hw;
	init_in.rt = &rt;
	memset(&rt, 0, sizeof(rt));

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	check_init_struct(&init_in);
	check_init_rt_regs(state, init_in.set_on_port,
			init_in.update_hw, &init_in);
}

// Verify complete init of routing table structure and hardware
// for all valid values.
static void rxs_init_rt_test_success(void **state)
{
	bool hw = true;
	bool no_hw = !hw;

	// Test full range of ports
	rxs_init_rt_test_success_val(state, no_hw, 0, RIO_RTV_PORT(0));
	rxs_init_rt_test_success_val(state, no_hw, 12, RIO_RTV_PORT(12));
	rxs_init_rt_test_success_val(state, no_hw,
			NUM_RXS_PORTS(&mock_dev_info) - 1,
			RIO_RTV_PORT((NUM_RXS_PORTS(&mock_dev_info) - 1)));
	rxs_init_rt_test_success_val(state, no_hw,
					RIO_ALL_PORTS, RIO_RTV_PORT(12));

	// Test init with a multicast mask
	rxs_init_rt_test_success_val(state, no_hw, 0, RIO_RTE_MC_0);
	rxs_init_rt_test_success_val(state, no_hw, 0, RIO_RTE_MC_LAST);

	// Test init with RIO_RTE_DROP
	rxs_init_rt_test_success_val(state, no_hw, RIO_ALL_PORTS, RIO_RTE_DROP);

	// Test init with RIO_RTE_DROP, on hardware
	rxs_init_rt_test_success_val(state, hw, RIO_ALL_PORTS, RIO_RTE_DROP);

	(void)state; // unused
}

// Verify that the initialization succeeds when the routing table pointer
// passed in is null, whether or not hardware is updated.
static void rxs_init_rt_null_test_success(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	uint32_t temp;
	uint8_t update_hw;
	RXS_test_state_t *RXS = *(RXS_test_state_t **)state;

	for (update_hw = 0; update_hw < 2; update_hw++) {
		init_mock_rxs_reg(state);
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_ROUTE_DFLT_PORT,
								&temp));
		init_in.set_on_port = 3;
		init_in.default_route = temp;
		init_in.default_route_table_port = RIO_RTE_DROP;
		init_in.update_hw = false;
		init_in.rt = NULL;

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_initialize(&mock_dev_info, &init_in,
					&init_out));
		assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

		//Check initialze values
		// It is not possible to predict the default route value
		// on real hardware.
		if (!RXS->real_hw) {
			assert_int_equal(0, init_in.default_route);
		}
		assert_null(init_in.rt);

		if (update_hw) {
			check_init_rt_regs(state, init_in.set_on_port,
					init_in.update_hw, &init_in);
		}
	}
}

pe_rt_val bad_default_route_array[] = {
	RIO_RTE_DFLT_PORT,
	RIO_RTE_LVL_G0,
	RXS2448_MAX_PORTS,
	0x0B00F008
};
static uint32_t num_bad_default_routes = sizeof(bad_default_route_array) /
					sizeof(bad_default_route_array[0]);

static void rxs_init_rt_test_bad_default_route(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_state_t rt;
	uint32_t idx;

	init_mock_rxs_reg(state);
	rxs_init_mock_rt(&rt);

	for (idx = 0; idx < num_bad_default_routes; idx++) {
		// Default route is DEFAULT PORT
		init_in.set_on_port = 12;
		init_in.default_route = bad_default_route_array[idx];
		init_in.default_route_table_port = RIO_RTE_DROP;
		init_in.update_hw = false;
		init_in.rt = &rt;
		init_out.imp_rc = RIO_SUCCESS;

		assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_rt_initialize(&mock_dev_info,
							&init_in, &init_out));
		assert_int_not_equal(RIO_SUCCESS, init_out.imp_rc);
	}

	(void)state; // unused
}

static void rxs_init_rt_test_bad_default_route_table(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_state_t rt;

	init_mock_rxs_reg(state);
	rxs_init_mock_rt(&rt);

	// Initialize all routing entries to LEVEL GROUP - FAIL
	init_in.set_on_port = 0;
	init_in.default_route = 0;
	init_in.default_route_table_port = RIO_RTE_LVL_G0;
	init_in.update_hw = false;
	init_in.rt = &rt;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_not_equal(RIO_SUCCESS, init_out.imp_rc);

	// Initialize all routing entries to ILLEGAL PORT NUMBER - FAIL
	init_in.set_on_port = 0;
	init_in.default_route = 0;
	init_in.default_route_table_port = NUM_RXS_PORTS(&mock_dev_info);
	init_in.update_hw = false;
	init_in.rt = &rt;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_not_equal(RIO_SUCCESS, init_out.imp_rc);

	// Initialize all routing entries to SOMETHING MANGLED - FAIL
	init_in.set_on_port = 0;
	init_in.default_route = 0;
	init_in.default_route_table_port = 0x0DEADC00;
	init_in.update_hw = false;
	init_in.rt = &rt;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_not_equal(RIO_SUCCESS, init_out.imp_rc);


	(void)state; // unused
}

// Check that initializing on a bad port number is detected
static void rxs_init_rt_test_bad_port(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_state_t rt;

	rxs_init_mock_rt(&rt);
	init_in.set_on_port = NUM_RXS_PORTS(&mock_dev_info);
	init_in.default_route = 0;
	init_in.default_route_table_port = RIO_RTE_DROP;
	init_in.update_hw = false;
	init_in.rt = &rt;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_not_equal(RIO_SUCCESS, init_out.imp_rc);

	assert_int_equal(NUM_RXS_PORTS(&mock_dev_info),
					init_in.set_on_port);

	(void)state; // unused
}

static void rxs_change_rte_rt_test_success_val(rio_port_t port,
			uint32_t idx, bool dom, pe_rt_val new_val)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_change_rte_in_t chg_in;
	rio_rt_change_rte_out_t chg_out;
	rio_rt_state_t rt;
	rio_rt_uc_info_t *rte;

	rxs_init_mock_rt(&rt);

	init_in.set_on_port = port;
	init_in.default_route = RIO_RTE_DROP;
	init_in.default_route_table_port = RIO_RTE_DROP;
	init_in.update_hw = true;
	init_in.rt = &rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// All rt entries should be unchanged at this point.
	// Change the selected entry.
	chg_in.dom_entry = dom;
	chg_in.idx = idx;
	chg_in.rte_value = new_val;
	chg_in.rt = init_in.rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(RIO_SUCCESS, chg_out.imp_rc);

	if (dom) {
		rte = &rt.dom_table[idx];
	} else {
		rte = &rt.dev_table[idx];
	}

	// Verify that the entry has been changed (or not) as appropriate
	if (RIO_RTE_DROP == new_val) {
		assert_int_equal(rte->rte_val, RIO_RTE_DROP);
		assert_false(rte->changed);
	} else {
		assert_int_equal(rte->rte_val, new_val);
		assert_true(rte->changed);
	}
}

static void rxs_change_rte_rt_test_success(void **state)
{
	const rio_port_t pmax = NUM_RXS_PORTS(&mock_dev_info) - 1;
	uint32_t imax = RIO_RT_GRP_SZ - 1;
	rio_port_t port;
	rio_port_t port_array[] = {0, 4, pmax, RIO_ALL_PORTS};
	rio_port_t num_ports = sizeof(port_array) / sizeof(port_array[0]);
	uint32_t port_idx, dom_idx;
	bool dom;

	for (port_idx = 0; port_idx < num_ports; port_idx++) {
		port = port_array[port_idx];
		for (dom_idx = 0; dom_idx < 2; dom_idx++) {
			dom = dom_idx;

			rxs_change_rte_rt_test_success_val(
					port, dom_idx, dom, RIO_RTE_PT_0);
			rxs_change_rte_rt_test_success_val(
					port, imax, dom, pmax);

			// Note, do not test multicast mask numbers as they
			// will fail.

			// with level groups, for domain routing table only...
			if (dom) {
				rxs_change_rte_rt_test_success_val(
					port, dom_idx, dom, RIO_RTE_LVL_G0);
				rxs_change_rte_rt_test_success_val(
					port, imax, dom, RIO_RTE_LVL_G0);
			}

			// with default port...
			rxs_change_rte_rt_test_success_val(
					port, dom_idx, dom, RIO_RTE_DFLT_PORT);
			rxs_change_rte_rt_test_success_val(
					port, imax, dom, RIO_RTE_DFLT_PORT);

			// with drop...
			rxs_change_rte_rt_test_success_val(
					port, dom_idx, dom, RIO_RTE_DROP);
			rxs_change_rte_rt_test_success_val(
					port, imax, dom, RIO_RTE_DROP);
		}
	}
	(void)state; // unused
}

static void rxs_change_rte_rt_null_test(void **state)
{
	rio_rt_change_rte_in_t chg_in;
	rio_rt_change_rte_out_t chg_out;

	chg_in.dom_entry = true;
	chg_in.idx = 1;
	chg_in.rte_value = 0;
	chg_in.rt = NULL;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_not_equal(RIO_SUCCESS, chg_out.imp_rc);

	(void)state; // unused
}

static void rxs_change_rte_rt_bad_rte_test(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_change_rte_in_t chg_in;
	rio_rt_change_rte_out_t chg_out;
	rio_rt_state_t rt;

	rxs_init_mock_rt(&rt);

	init_in.set_on_port = 0;
	init_in.default_route = RIO_RTV_PORT(0);
	init_in.default_route_table_port = RIO_RTV_PORT(0);
	init_in.update_hw = false;
	init_in.rt = &rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Bad port number
	chg_in.dom_entry = true;
	chg_in.idx = 1;
	chg_in.rte_value = RIO_RTV_PORT(NUM_RXS_PORTS(&mock_dev_info));
	chg_in.rt = init_in.rt;
	chg_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_not_equal(RIO_SUCCESS, chg_out.imp_rc);

	// Bad level group
	chg_in.dom_entry = true;
	chg_in.idx = 1;
	chg_in.rte_value = RIO_RTV_LVL_GRP(1);
	chg_in.rt = init_in.rt;
	chg_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_not_equal(RIO_SUCCESS, chg_out.imp_rc);

	// Mangled value
	chg_in.dom_entry = true;
	chg_in.idx = 1;
	chg_in.rte_value = RIO_RTE_DFLT_PORT + 1;
	chg_in.rt = init_in.rt;
	chg_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_not_equal(RIO_SUCCESS, chg_out.imp_rc);

	// Attempt to change domain entry 0...
	chg_in.dom_entry = true;
	chg_in.idx = 0;
	chg_in.rte_value = RIO_RTV_PORT(0);
	chg_in.rt = init_in.rt;
	chg_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_not_equal(RIO_SUCCESS, chg_out.imp_rc);

	(void)state; // unused
}

static void rxs_check_alloc_mc_rt_change(uint32_t mc_idx,
		rio_rt_alloc_mc_mask_in_t *mock_alloc_in)
{
	uint32_t idx;

	for (idx = 0; idx <= mc_idx; idx++) {
		assert_true(!mock_alloc_in->rt->mc_masks[idx].in_use);
		assert_true(mock_alloc_in->rt->mc_masks[idx].allocd);
		assert_true(!mock_alloc_in->rt->mc_masks[idx].changed);
	}

	for (idx = mc_idx + 1; idx < RXS2448_MC_MASK_CNT; ++idx) {
		assert_true(!mock_alloc_in->rt->mc_masks[idx].in_use);
		assert_true(!mock_alloc_in->rt->mc_masks[idx].allocd);
		assert_true(!mock_alloc_in->rt->mc_masks[idx].changed);
	}
}

static void rxs_alloc_mc_rt_test_success(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_alloc_mc_mask_in_t alloc_in;
	rio_rt_alloc_mc_mask_out_t alloc_out;
	rio_rt_state_t rt;
	rio_port_t port;
	pe_rt_val mc_idx;

	for (port = 0; port <= NUM_RXS_PORTS(&mock_dev_info); port++) {
		rxs_init_mock_rt(&rt);

		if (NUM_RXS_PORTS(&mock_dev_info) == port) {
			init_in.set_on_port = RIO_ALL_PORTS;
			init_in.default_route =
			RIO_RTE_DROP;
			init_in.default_route_table_port =
			RIO_RTE_DFLT_PORT;
		} else {
			init_in.set_on_port = port;
			init_in.default_route = port;
			init_in.default_route_table_port = port;
		}
		init_in.update_hw = true;
		init_in.rt = &rt;
		memset(&rt, 0, sizeof(rt));

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_initialize(&mock_dev_info,
						&init_in, &init_out));
		assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

		// Allocate all multicast masks, check that they are
		// correctly allocated.
		alloc_in.rt = init_in.rt;
		for (mc_idx = 0; mc_idx < RXS2448_MC_MASK_CNT; mc_idx++) {
			assert_int_equal(RIO_SUCCESS,
				rio_rt_alloc_mc_mask(&mock_dev_info,
							&alloc_in,
							&alloc_out));
			assert_int_equal(RIO_SUCCESS, alloc_out.imp_rc);
			assert_int_equal(RIO_RTV_MC_MSK(mc_idx),
					alloc_out.mc_mask_rte);

			rxs_check_alloc_mc_rt_change(mc_idx, &alloc_in);
		}

		// Check that it is not possible to allocate another
		// multicast mask.
		assert_int_not_equal(RIO_SUCCESS,
			rio_rt_alloc_mc_mask(&mock_dev_info,
						&alloc_in,
						&alloc_out));
		assert_int_not_equal(RIO_SUCCESS, alloc_out.imp_rc);
		assert_int_equal(RIO_RTE_BAD, alloc_out.mc_mask_rte);

		rxs_check_alloc_mc_rt_change(RXS2448_MC_MASK_CNT - 1,
								&alloc_in);
	}

	(void)state; // unused
}

static void rxs_alloc_mc_rt_null_test(void **state)
{
	rio_rt_alloc_mc_mask_in_t alloc_in;
	rio_rt_alloc_mc_mask_out_t alloc_out;

	alloc_in.rt = NULL;

	assert_int_not_equal(RIO_SUCCESS,
			rio_rt_alloc_mc_mask(&mock_dev_info, &alloc_in,
					&alloc_out));
	assert_int_not_equal(RIO_SUCCESS, alloc_out.imp_rc);

	(void)state; // unused
}

static void rxs_alloc_mc_rt_bad_in_use_allocd_test(void **state)
{
	rio_rt_alloc_mc_mask_in_t alloc_in;
	rio_rt_alloc_mc_mask_out_t alloc_out;
	rio_rt_state_t rt;
	uint32_t idx;

	rxs_init_mock_rt(&rt);
	alloc_in.rt = &rt;
	memset(&rt, 0, sizeof(rt));

	// Cannot allocate another mask when all masks are in use
	for (idx = 0; idx < RXS2448_MC_MASK_CNT; idx++) {
		alloc_in.rt->mc_masks[idx].in_use = true;
	}

	assert_int_not_equal(RIO_SUCCESS,
			rio_rt_alloc_mc_mask(&mock_dev_info, &alloc_in,
					&alloc_out));
	assert_int_not_equal(RIO_SUCCESS, alloc_out.imp_rc);
	assert_int_equal(RIO_RTE_BAD, alloc_out.mc_mask_rte);

	rxs_init_mock_rt(&rt);
	alloc_in.rt = &rt;
	memset(&rt, 0, sizeof(rt));

	// Cannot allocate another mask when all masks are already alloc'ed
	for (idx = 0; idx < RXS2448_MC_MASK_CNT; idx++) {
		alloc_in.rt->mc_masks[idx].allocd = true;
	}

	assert_int_not_equal(RIO_SUCCESS,
			rio_rt_alloc_mc_mask(&mock_dev_info, &alloc_in,
					&alloc_out));
	assert_int_not_equal(RIO_SUCCESS, alloc_out.imp_rc);
	assert_int_equal(RIO_RTE_BAD, alloc_out.mc_mask_rte);

	rxs_init_mock_rt(&rt);
	alloc_in.rt = &rt;
	memset(&rt, 0, sizeof(rt));

	// Cannot allocate another mask when all masks are alloc'ed && in use
	for (idx = 0; idx < RXS2448_MC_MASK_CNT; idx++) {
		alloc_in.rt->mc_masks[idx].in_use = true;
		alloc_in.rt->mc_masks[idx].allocd = true;
	}

	assert_int_not_equal(RIO_SUCCESS,
			rio_rt_alloc_mc_mask(&mock_dev_info, &alloc_in,
					&alloc_out));
	assert_int_not_equal(RIO_SUCCESS, alloc_out.imp_rc);
	assert_int_equal(RIO_RTE_BAD, alloc_out.mc_mask_rte);

	(void)state; // unused
}

static void rxs_check_change_mc_rt_change(uint32_t mc_idx,
		rio_rt_change_mc_mask_in_t *mc_chg_in)
{
	uint32_t idx = RIO_RTV_GET_MC_MSK(mc_chg_in->mc_mask_rte);

	assert_int_equal(idx, mc_idx);

	// Verify multicast info is updated correctly
	assert_int_equal(mc_chg_in->mc_info.mc_destID,
			mc_chg_in->rt->mc_masks[idx].mc_destID);
	assert_int_equal(mc_chg_in->mc_info.tt,
			mc_chg_in->rt->mc_masks[idx].tt);
	assert_int_equal(mc_chg_in->mc_info.mc_mask,
			mc_chg_in->rt->mc_masks[idx].mc_mask);
	assert_true(mc_chg_in->rt->mc_masks[idx].in_use);
	assert_true(mc_chg_in->rt->mc_masks[idx].changed);
	assert_true(mc_chg_in->rt->mc_masks[idx].allocd);

	//Check dom_table
	if ((mc_chg_in->mc_info.tt == tt_dev16) && idx) {
		assert_int_equal(mc_chg_in->mc_mask_rte,
			mc_chg_in->rt->dom_table[idx].rte_val);
		assert_true(mc_chg_in->rt->dom_table[idx].changed);
	}

	if (mc_chg_in->mc_info.tt == tt_dev8) {
		assert_int_equal(mc_chg_in->mc_mask_rte,
				mc_chg_in->rt->dev_table[idx].rte_val);
		assert_true(mc_chg_in->rt->dev_table[idx].changed);
	}
}

static void rxs_change_mc_rt_test_success(bool dom)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_change_mc_mask_in_t mc_chg_in;
	rio_rt_change_mc_mask_out_t mc_chg_out;
	rio_rt_state_t rt;
	uint32_t port, mc_idx, mc_mask_val;

	for (port = 0; port <= NUM_RXS_PORTS(&mock_dev_info); port++) {
		if (NUM_RXS_PORTS(&mock_dev_info) == port) {
			init_in.set_on_port = RIO_ALL_PORTS;
			init_in.default_route = RIO_RTE_DROP;
			init_in.default_route_table_port = RIO_RTE_DFLT_PORT;
			mc_mask_val = 0x1234;
		} else {
			init_in.set_on_port = port;
			init_in.default_route = port;
			init_in.default_route_table_port = port;
			mc_mask_val = port + 1;
		}
		init_in.update_hw = true;
		init_in.rt = &rt;
		memset(&rt, 0, sizeof(rt));

		// Initialize routing table for the current port
		assert_int_equal(RIO_SUCCESS,
				rxs_rio_rt_initialize(&mock_dev_info,
						&init_in, &init_out));
		assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

		// Change all multicast masks without allocating them.
		// Should change corresponding dev/dom routing table entry.
		for (mc_idx = 0; mc_idx < RXS2448_MC_MASK_CNT; mc_idx++) {
			mc_chg_in.mc_mask_rte = RIO_RTV_MC_MSK(mc_idx);
			mc_chg_in.mc_info.mc_mask = mc_mask_val;
			if (dom) {
				mc_chg_in.mc_info.tt = tt_dev16;
				mc_chg_in.mc_info.mc_destID = (mc_idx << 8);
			} else {
				mc_chg_in.mc_info.tt = tt_dev8;
				mc_chg_in.mc_info.mc_destID = mc_idx;
			}
			mc_chg_in.mc_info.in_use = true;
			mc_chg_in.mc_info.allocd = true;
			mc_chg_in.mc_info.changed = false;
			mc_chg_in.rt = init_in.rt;

			assert_int_equal(RIO_SUCCESS,
				rxs_rio_rt_change_mc_mask(&mock_dev_info,
							&mc_chg_in,
							&mc_chg_out));
			assert_int_equal(RIO_SUCCESS, mc_chg_out.imp_rc);

			rxs_check_change_mc_rt_change(mc_idx, &mc_chg_in);
		}
	}
}

static void rxs_change_mc_rt_dom_inuse_hw_test_success(void **state)
{
	rxs_change_mc_rt_test_success(true);
	(void)state; // unused
}

static void rxs_change_mc_rt_dev_inuse_hw_test_success(void **state)
{
	rxs_change_mc_rt_test_success(false);
	(void)state; // unused
}

static void rxs_check_change_allocd_mc_rt_change(
		rio_rt_change_mc_mask_in_t *mc_chg_in1,
		rio_rt_change_mc_mask_in_t *mc_chg_in2)
{
	uint32_t idx;

	idx = RIO_RTV_GET_MC_MSK(mc_chg_in1->mc_mask_rte);

	// Check that the multicast mask was updated correctly.
	assert_int_equal(mc_chg_in1->mc_info.mc_destID,
			mc_chg_in1->rt->mc_masks[idx].mc_destID);
	assert_int_equal(mc_chg_in1->mc_info.tt,
			mc_chg_in1->rt->mc_masks[idx].tt);
	assert_int_equal(mc_chg_in2->mc_info.mc_mask,
			mc_chg_in2->rt->mc_masks[idx].mc_mask);
	assert_true(mc_chg_in1->rt->mc_masks[idx].in_use);
	assert_true(mc_chg_in1->rt->mc_masks[idx].changed);
	assert_true(mc_chg_in1->rt->mc_masks[idx].allocd);

	// Check the device and domain tables are correct

	assert_int_equal(mc_chg_in1->rt->dev_table[idx].rte_val,
			mc_chg_in1->mc_mask_rte);
	assert_true(mc_chg_in1->rt->dev_table[idx].changed);

	if (idx) {
		assert_int_equal(mc_chg_in2->mc_mask_rte,
				mc_chg_in2->rt->dom_table[idx].rte_val);
		assert_true(mc_chg_in2->rt->dom_table[idx].changed);
	} else {
		// Treat domain table entry 0 specially
		assert_int_equal(RIO_RTE_LVL_G0,
				mc_chg_in1->rt->dom_table[0].rte_val);
		assert_false(mc_chg_in1->rt->dom_table[0].changed);
	}
}

static void rxs_change_allocd_mc_rt_test_success(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_change_mc_mask_in_t mc_chg_in1;
	rio_rt_change_mc_mask_out_t mc_chg_out1;
	rio_rt_change_mc_mask_in_t mc_chg_in2;
	rio_rt_change_mc_mask_out_t mc_chg_out2;
	rio_rt_alloc_mc_mask_in_t alloc_in;
	rio_rt_alloc_mc_mask_out_t alloc_out;
	rio_rt_state_t rt;
	uint32_t mc_idx;
	rio_port_t port;
	const rio_port_t pmax = NUM_RXS_PORTS(&mock_dev_info) - 1;
	rio_port_t port_array[] = {0, 12, pmax, RIO_ALL_PORTS};
	uint32_t num_ports = sizeof(port_array) / sizeof(port_array[0]);
	uint32_t port_idx;

	for (port_idx = 0; port_idx < num_ports; port_idx++) {
		init_mock_rxs_reg(state);

		port = port_array[port_idx];

		if (RIO_ALL_PORTS == port) {
			init_in.set_on_port = RIO_ALL_PORTS;
			init_in.default_route =
			RIO_RTE_DROP;
			init_in.default_route_table_port =
			RIO_RTE_DFLT_PORT;
		} else {
			init_in.set_on_port = port;
			init_in.default_route = port;
			init_in.default_route_table_port = port;
		}
		init_in.update_hw = true;
		init_in.rt = &rt;

		// Initialize routing table
		assert_int_equal(RIO_SUCCESS,
				rxs_rio_rt_initialize(&mock_dev_info,
						&init_in, &init_out));
		assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

		alloc_in.rt = init_in.rt;
		for (mc_idx = 0; mc_idx < RXS2448_MC_MASK_CNT; mc_idx++) {
			// Allocate multicast mask
			assert_int_equal(RIO_SUCCESS,
					rio_rt_alloc_mc_mask(&mock_dev_info,
							&alloc_in,
							&alloc_out));
			assert_int_equal(RIO_SUCCESS, alloc_out.imp_rc);

			// Change the allocated multicast mask,
			// modifying the associated dev08 routing table entry
			mc_chg_in1.mc_mask_rte = alloc_out.mc_mask_rte;
			mc_chg_in1.mc_info.mc_mask = mc_idx + 1;
			mc_chg_in1.mc_info.mc_destID = mc_idx;
			mc_chg_in1.mc_info.tt = tt_dev8;
			mc_chg_in1.mc_info.in_use = true;
			mc_chg_in1.mc_info.allocd = false;
			mc_chg_in1.mc_info.changed = false;
			mc_chg_in1.rt = init_in.rt;
			mc_chg_out1.imp_rc = 0x12345678;

			assert_int_equal(RIO_SUCCESS,
					rxs_rio_rt_change_mc_mask(
							&mock_dev_info,
							&mc_chg_in1,
							&mc_chg_out1));
			assert_int_equal(RIO_SUCCESS, mc_chg_out1.imp_rc);

			// Now modify the dev16 routing table entry
			if (mc_idx) {
				mc_chg_in2.mc_mask_rte = alloc_out.mc_mask_rte;
				mc_chg_in2.mc_info.mc_mask = mc_idx + 2;
				mc_chg_in2.mc_info.mc_destID = (mc_idx << 8);
				mc_chg_in2.mc_info.tt = tt_dev16;
				mc_chg_in2.mc_info.in_use = true;
				mc_chg_in2.mc_info.allocd = true;
				mc_chg_in2.mc_info.changed = false;
				mc_chg_in2.rt = init_in.rt;
				mc_chg_out2.imp_rc = 0x12345678;

				assert_int_equal(RIO_SUCCESS,
					rxs_rio_rt_change_mc_mask(
							&mock_dev_info,
							&mc_chg_in2,
							&mc_chg_out2));
				assert_int_equal(RIO_SUCCESS,
							mc_chg_out2.imp_rc);
				rxs_check_change_allocd_mc_rt_change(&mc_chg_in1, &mc_chg_in2);
				continue;
			}

			// Dev16 entry 0 is a special case, so only check for dev08 changes
			rxs_check_change_allocd_mc_rt_change(&mc_chg_in1, &mc_chg_in1);
		}
	}
}

static void rxs_change_mc_rt_test_bad_parms(void **state)
{
	rio_rt_change_mc_mask_in_t chg_in;
	rio_rt_change_mc_mask_out_t chg_out;
	rio_rt_state_t rt;

	// Try RT = NULL
	chg_in.mc_mask_rte = RIO_RTE_MC_0;
	chg_in.mc_info.mc_mask = 0x0000FFFF;
	chg_in.mc_info.mc_destID = 0x01;
	chg_in.mc_info.tt = tt_dev8;
	chg_in.mc_info.in_use = true;
	chg_in.mc_info.allocd = false;
	chg_in.mc_info.changed = false;
	chg_in.rt = NULL;
	chg_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_change_mc_mask(&mock_dev_info, &chg_in, &chg_out));
	assert_int_not_equal(RIO_SUCCESS, chg_out.imp_rc);

	// Try bad mc_destID
	chg_in.rt = &rt;
	chg_in.mc_info.mc_destID = RIO_LAST_DEV16 + 1;
	chg_in.mc_info.tt = tt_dev16;
	chg_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_change_mc_mask(&mock_dev_info, &chg_in, &chg_out));
	assert_int_not_equal(RIO_SUCCESS, chg_out.imp_rc);

	// Try bad mc_mask
	chg_in.mc_info.mc_destID = RIO_LAST_DEV16;
	chg_in.mc_info.mc_mask = 1 << NUM_RXS_PORTS(&mock_dev_info);
	chg_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_change_mc_mask(&mock_dev_info, &chg_in, &chg_out));
	assert_int_not_equal(RIO_SUCCESS, chg_out.imp_rc);

	// Try bad mc_mask_rte
	chg_in.mc_info.mc_mask = 1;
	chg_in.mc_mask_rte = RIO_RTE_MC_LAST + 1;
	chg_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_change_mc_mask(&mock_dev_info, &chg_in, &chg_out));
	assert_int_not_equal(RIO_SUCCESS, chg_out.imp_rc);

	// Try bad multicast mask destID tt
	chg_in.mc_mask_rte = RIO_RTE_MC_LAST;
	chg_in.mc_info.tt = (tt_t)3;
	chg_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_change_mc_mask(&mock_dev_info, &chg_in, &chg_out));
	assert_int_not_equal(RIO_SUCCESS, chg_out.imp_rc);

	(void)state; // unused
}

// Demonstrate deallocation for unallocated multicast masks
static void rxs_dealloc_mc_rt_test_unalloced_success(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_dealloc_mc_mask_in_t dall_in;
	rio_rt_dealloc_mc_mask_out_t dall_out;
	rio_rt_state_t rt, chk_rt;
	pe_rt_val mc_idx;

	// Initialize routing table
	init_in.set_on_port = RIO_RTV_PORT(0);
	init_in.default_route = RIO_RTV_PORT(0);
	init_in.default_route_table_port = RIO_RTV_PORT(0);
	init_in.update_hw = false;
	init_in.rt = &rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Preserve freshly initialized rt
	memcpy(&chk_rt, &rt, sizeof(chk_rt));

	// Deallocate each unallocated multicast mask
	dall_in.rt = init_in.rt;
	for (mc_idx = 0; mc_idx < RXS2448_MC_MASK_CNT; mc_idx++) {
		dall_in.mc_mask_rte = RIO_RTV_MC_MSK(mc_idx);
		dall_out.imp_rc = 0xFFFFFFFF;

		assert_int_equal(RIO_SUCCESS,
			rio_rt_dealloc_mc_mask(
				&mock_dev_info, &dall_in, &dall_out));
		assert_int_equal(RIO_SUCCESS, dall_out.imp_rc);
	}

	// Test that the routing table is unchanged from its freshly
	// initialized state.
	assert_int_equal(0, memcmp(&rt, &chk_rt, sizeof(rt)));

	(void)state; // unused
}

// Demonstrate repeated allocation and deallocation for all multicast masks
static void rxs_dealloc_mc_rt_test_success(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_alloc_mc_mask_in_t all_in;
	rio_rt_alloc_mc_mask_out_t all_out;
	rio_rt_dealloc_mc_mask_in_t dall_in;
	rio_rt_dealloc_mc_mask_out_t dall_out;
	rio_rt_state_t rt, chk_rt;
	pe_rt_val mc_idx;
	uint8_t i;

	// Initialize routing table
	init_in.set_on_port = RIO_ALL_PORTS;
	init_in.default_route = RIO_RTE_DROP;
	init_in.default_route_table_port = RIO_RTE_DROP;
	init_in.update_hw = true;
	init_in.rt = &rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Preserve freshly initialized rt
	memcpy(&chk_rt, &rt, sizeof(chk_rt));

	// Try three times to alloc/deallocate all multicast masks
	for (i = 0; i < 3; i++) {
		// Allocate all multicast masks
		all_in.rt = init_in.rt;
		for (mc_idx = 0; mc_idx < RXS2448_MC_MASK_CNT; mc_idx++) {
			assert_int_equal(RIO_SUCCESS,
				rio_rt_alloc_mc_mask(
					&mock_dev_info, &all_in, &all_out));
				assert_int_equal(RIO_SUCCESS, all_out.imp_rc);
		}

		// Deallocate all multicast masks
		dall_in.rt = all_in.rt;
		for (mc_idx = 0; mc_idx < RXS2448_MC_MASK_CNT; mc_idx++) {
			dall_in.mc_mask_rte = RIO_RTV_MC_MSK(mc_idx);

			assert_int_equal(RIO_SUCCESS,
				DSF_rio_rt_dealloc_mc_mask(
					&mock_dev_info, &dall_in, &dall_out));
			assert_int_equal(RIO_SUCCESS, dall_out.imp_rc);
		}
	}

	// Test that the routing table is unchanged from its freshly
	// initialized state.
	assert_int_equal(0, memcmp(&rt, &chk_rt, sizeof(rt)));

	(void)state; // unused
}

static void rxs_check_dealloc_change_mc_rt_change(rio_rt_state_t *rt, bool chg_mc)
{
	uint32_t idx;

	// All multicast masks should be allocated, in use and changed
	// Always program a dev08 multicast mask first.
	// Multicast mask value should always be from the second
	// change request.
	for (idx = 0; idx < RXS2448_MC_MASK_CNT; ++idx) {
		assert_true(rt->mc_masks[idx].allocd);
		assert_int_equal(chg_mc, rt->mc_masks[idx].changed);
		assert_int_equal(chg_mc, rt->mc_masks[idx].in_use);
		assert_int_equal(tt_dev8, rt->mc_masks[idx].tt);
		if (chg_mc) {
			assert_int_equal(idx, rt->mc_masks[idx].mc_destID);
			assert_int_equal(idx + 2, rt->mc_masks[idx].mc_mask);
		} else {
			assert_int_equal(0, rt->mc_masks[idx].mc_destID);
			assert_int_equal(0, rt->mc_masks[idx].mc_mask);
		}
	}

	//Check dom_table
	for (idx = 0; idx < RIO_RT_GRP_SZ; idx++) {
		// Domain entry 0 is special, and should be unchanged.
		if (!idx) {
			assert_int_equal(RIO_RTE_LVL_G0,
				rt->dom_table[idx].rte_val);
			assert_false(rt->dom_table[idx].changed);
			continue;
		}

		assert_int_equal(chg_mc, rt->dom_table[idx].changed);
		if (chg_mc) {
			assert_int_equal(RIO_RTV_MC_MSK(idx),
				rt->dom_table[idx].rte_val);
		} else {
			assert_int_equal(RIO_RTE_DROP,
				rt->dom_table[idx].rte_val);
		}
	}

	//Check dev_table
	for (idx = 0; idx < RIO_RT_GRP_SZ; idx++) {
		assert_int_equal(chg_mc, rt->dev_table[idx].changed);
		if (chg_mc) {
			assert_int_equal(RIO_RTV_MC_MSK(idx),
				rt->dev_table[idx].rte_val);
		} else {
			assert_int_equal(RIO_RTE_DROP,
				rt->dev_table[idx].rte_val);
		}
	}
}

// Demonstrate routing table using rio_rt_change_mc_mask for multicast masks
static void rxs_dealloc_change_mc_rt_test_success(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_alloc_mc_mask_in_t alloc_in;
	rio_rt_alloc_mc_mask_out_t alloc_out;
	rio_rt_change_mc_mask_in_t chg_in;
	rio_rt_change_mc_mask_out_t chg_out;
	rio_rt_dealloc_mc_mask_in_t dealloc_in;
	rio_rt_dealloc_mc_mask_out_t dealloc_out;
	rio_rt_state_t rt, chk_rt;
	uint32_t mc_idx;
	uint8_t i;

	// Initialize routing table
	memset(&rt, 0, sizeof(rt));
	init_in.set_on_port = RIO_ALL_PORTS;
	init_in.default_route = RIO_RTE_DROP;
	init_in.default_route_table_port = RIO_RTE_DROP;
	init_in.update_hw = false;
	init_in.rt = &rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Preserve freshly initialized rt, with change indications
	// Clear change indication for dom_table[0], as it should not
	// be changed by the manipulations below.
	memcpy(&chk_rt, &rt, sizeof(chk_rt));
	chk_rt.dom_table[0].changed = false;

	// Reinitialize and clear changed indication
	init_in.update_hw = true;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	for (i = 0; i < 2; i++) {
		// For each multicast mask,
		// - allocate
		// - create dev8 routing table entry
		// - create dev16 routing table entry
		for (mc_idx = 0; mc_idx < RXS2448_MC_MASK_CNT; mc_idx++) {
			alloc_in.rt = &rt;
			assert_int_equal(RIO_SUCCESS,
				rio_rt_alloc_mc_mask(
					&mock_dev_info, &alloc_in, &alloc_out));
			assert_int_equal(RIO_SUCCESS, alloc_out.imp_rc);

			// Program device routing table entry
			chg_in.mc_mask_rte = alloc_out.mc_mask_rte;
			chg_in.mc_info.mc_mask = mc_idx + 1;

			chg_in.mc_info.mc_destID = mc_idx;
			chg_in.mc_info.tt = tt_dev8;
			chg_in.mc_info.in_use = true;
			chg_in.mc_info.allocd = true;
			chg_in.mc_info.changed = false;
			chg_in.rt = init_in.rt;
			chg_out.imp_rc = 0x12345678;

			assert_int_equal(RIO_SUCCESS,
				rxs_rio_rt_change_mc_mask(&mock_dev_info,
								&chg_in,
								&chg_out));
			assert_int_equal(RIO_SUCCESS, chg_out.imp_rc);

			// Program domain routing table entry and
			// change multicast mask.
			chg_in.mc_mask_rte = alloc_out.mc_mask_rte;
			chg_in.mc_info.mc_mask = mc_idx + 2;

			chg_in.mc_info.mc_destID = (mc_idx << 8);
			chg_in.mc_info.tt = tt_dev16;
			chg_in.mc_info.in_use = true;
			chg_in.mc_info.allocd = true;
			chg_in.mc_info.changed = false;
			chg_in.rt = init_in.rt;

			assert_int_equal(RIO_SUCCESS,
				rxs_rio_rt_change_mc_mask(&mock_dev_info,
								&chg_in,
								&chg_out));
			assert_int_equal(RIO_SUCCESS, chg_out.imp_rc);
		}

		// Check routing table and mulicast mask state
		rxs_check_dealloc_change_mc_rt_change(&rt, true);

		// Dealloc all multicast masks
		dealloc_in.rt = init_in.rt;
		for (mc_idx = 0; mc_idx < RXS2448_MC_MASK_CNT; mc_idx++) {
			dealloc_in.mc_mask_rte = RIO_RTV_MC_MSK(mc_idx);
			assert_int_equal(RIO_SUCCESS,
				DSF_rio_rt_dealloc_mc_mask( &mock_dev_info,
								&dealloc_in,
								&dealloc_out));
			assert_int_equal(RIO_SUCCESS, dealloc_out.imp_rc);
		}
	}

	// Test that the routing table is unchanged from its freshly
	// initialized state.
	assert_int_equal(0, memcmp(&rt, &chk_rt, sizeof(rt)));

	(void)state; // unused
}

// Demonstrate routing table using rio_rt_change_rte for multicast masks
static void rxs_dealloc_change_rt_rt_test_success(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_alloc_mc_mask_in_t alloc_in;
	rio_rt_alloc_mc_mask_out_t alloc_out;
	rio_rt_change_rte_in_t chg_in;
	rio_rt_change_rte_out_t chg_out;
	rio_rt_dealloc_mc_mask_in_t dealloc_in;
	rio_rt_dealloc_mc_mask_out_t dealloc_out;
	rio_rt_state_t rt, chk_rt;
	uint32_t mc_idx;
	uint8_t i;

	// Initialize routing table
	memset(&rt, 0, sizeof(rt));
	init_in.set_on_port = RIO_ALL_PORTS;
	init_in.default_route = RIO_RTE_DROP;
	init_in.default_route_table_port = RIO_RTE_DROP;
	init_in.update_hw = true;
	init_in.rt = &rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Preserve freshly initialized rt, no entries changed
	memcpy(&chk_rt, &rt, sizeof(chk_rt));

	for (i = 0; i < 2; i++) {
		// For each multicast mask,
		// - allocate
		// - create dev8 routing table entry
		// - create dev16 routing table entry
		for (mc_idx = 0; mc_idx < RXS2448_MC_MASK_CNT; mc_idx++) {

			// Allocate multicast mask
			alloc_in.rt = &rt;
			assert_int_equal(RIO_SUCCESS,
				rio_rt_alloc_mc_mask(
					&mock_dev_info, &alloc_in, &alloc_out));
			assert_int_equal(RIO_SUCCESS, alloc_out.imp_rc);

			// Program device dev8 routing table entry,
			// should fail as the multicast mask hasn't been set
			chg_in.dom_entry = false;
			chg_in.idx = mc_idx;
			chg_in.rte_value = alloc_out.mc_mask_rte;
			chg_in.rt = alloc_in.rt;

			assert_int_not_equal(RIO_SUCCESS,
				rxs_rio_rt_change_rte(&mock_dev_info,
								&chg_in,
								&chg_out));
			assert_int_not_equal(RIO_SUCCESS, chg_out.imp_rc);

			// Program device dev16 routing table entry,
			// should fail as the multicast mask hasn't been updated
			if (mc_idx) {
				chg_in.dom_entry = true;
				chg_in.idx = mc_idx;
				chg_in.rte_value = alloc_out.mc_mask_rte;
				chg_in.rt = alloc_in.rt;

				assert_int_not_equal(RIO_SUCCESS,
					rxs_rio_rt_change_rte(&mock_dev_info,
								&chg_in,
								&chg_out));
				assert_int_not_equal(RIO_SUCCESS, chg_out.imp_rc);
			}

		}

		// Check routing table and mulicast mask state
		rxs_check_dealloc_change_mc_rt_change(&rt, false);

		// Dealloc all multicast masks
		dealloc_in.rt = init_in.rt;
		for (mc_idx = 0; mc_idx < RXS2448_MC_MASK_CNT; mc_idx++) {
			dealloc_in.mc_mask_rte = RIO_RTV_MC_MSK(mc_idx);
			assert_int_equal(RIO_SUCCESS,
				DSF_rio_rt_dealloc_mc_mask( &mock_dev_info,
								&dealloc_in,
								&dealloc_out));
			assert_int_equal(RIO_SUCCESS, dealloc_out.imp_rc);
		}
	}

	// Test that the routing table is unchanged from its freshly
	// initialized state.
	assert_int_equal(0, memcmp(&rt, &chk_rt, sizeof(rt)));

	(void)state; // unused
}

static void rxs_dealloc_mc_rt_bad_parms(void **state)
{
	rio_rt_dealloc_mc_mask_in_t dealloc_in;
	rio_rt_dealloc_mc_mask_out_t dealloc_out;
	rio_rt_state_t rt;

	// Null routing table pointer
	dealloc_in.mc_mask_rte = RIO_RTE_MC_0;
	dealloc_in.rt = NULL;

	assert_int_not_equal(RIO_SUCCESS,
			DSF_rio_rt_dealloc_mc_mask(&mock_dev_info,
					&dealloc_in, &dealloc_out));
	assert_int_not_equal(RIO_SUCCESS, dealloc_out.imp_rc);

	// Bad multicast mask selection
	// Way out of range...
	rxs_init_mock_rt(&rt);
	dealloc_in.mc_mask_rte = RIO_RTE_BAD;
	dealloc_in.rt = &rt;

	assert_int_not_equal(RIO_SUCCESS,
			DSF_rio_rt_dealloc_mc_mask(&mock_dev_info,
					&dealloc_in, &dealloc_out));
	assert_int_not_equal(RIO_SUCCESS, dealloc_out.imp_rc);

	// One below range
	dealloc_in.mc_mask_rte = RIO_RTE_MC_0 - 1;
	dealloc_in.rt = &rt;

	assert_int_not_equal(RIO_SUCCESS,
			DSF_rio_rt_dealloc_mc_mask(&mock_dev_info,
					&dealloc_in, &dealloc_out));
	assert_int_not_equal(RIO_SUCCESS, dealloc_out.imp_rc);

	// One above range
	dealloc_in.mc_mask_rte = RIO_RTE_MC_LAST + 1;
	dealloc_in.rt = &rt;

	assert_int_not_equal(RIO_SUCCESS,
			DSF_rio_rt_dealloc_mc_mask(&mock_dev_info,
					&dealloc_in, &dealloc_out));
	assert_int_not_equal(RIO_SUCCESS, dealloc_out.imp_rc);

	(void)state; // unused
}

static void rxs_check_set_rt_regs(rio_rt_set_all_in_t *set_in)
{
	uint32_t dev_out, dom_out, mc_mask_out;
	uint32_t idx;
	rio_port_t port;
	rio_port_t st_port = set_in->set_on_port;
	rio_port_t end_port = set_in->set_on_port;

	if (RIO_ALL_PORTS == set_in->set_on_port) {
		st_port = 0;
		end_port = NUM_RXS_PORTS(&mock_dev_info) - 1;
	}

	for (port = st_port; port <= end_port; port++) {
		for (idx = 0; idx < RIO_RT_GRP_SZ; idx++) {
			rxs_reg_dev_dom(port, idx, &dom_out, &dev_out);

			assert_int_equal(dom_out,
					set_in->rt->dom_table[idx].rte_val);
			assert_false(set_in->rt->dom_table[idx].changed);
			assert_int_equal(dev_out,
					set_in->rt->dev_table[idx].rte_val);
			assert_false(set_in->rt->dev_table[idx].changed);
		}

		// Confirm no multicast values changed.
                for (idx = 0; idx < RXS2448_MC_MASK_CNT; idx++) {
			rxs_reg_mc_mask(port, idx, &mc_mask_out);
			assert_int_equal(mc_mask_out,
				set_in->rt->mc_masks[idx].mc_mask);
			assert_false(set_in->rt->mc_masks[idx].changed);
                }
	}
}

static void rxs_set_changed_rt_test_success_val(void **state, rio_port_t port,
				pe_rt_val dflt_rt, pe_rt_val dflt_val,
				pe_rt_val chg_val)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_change_rte_in_t chg_in;
	rio_rt_change_rte_out_t chg_out;
	rio_rt_change_mc_mask_in_t mc_in;
	rio_rt_change_mc_mask_out_t mc_out;
	rio_rt_set_changed_in_t set_in;
	rio_rt_set_changed_out_t set_out;
	rio_rt_state_t rt;
	rio_rt_state_t chk_rt;

	init_mock_rxs_reg(state);

	init_in.set_on_port = port;
	init_in.default_route = dflt_rt;
	init_in.default_route_table_port = dflt_val;

	// Initialize routing table
	init_in.update_hw = true;
	init_in.rt = &rt;

	memset(&rt, 0, sizeof(rt));

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Change a routing table entries the conventional way
	// at the limits of each table.  Note: Cannot change domain entry 0.
	chg_in.dom_entry = true;
	chg_in.idx = RIO_RT_GRP_SZ - 1;
	chg_in.rte_value = chg_val;
	chg_in.rt = init_in.rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(RIO_SUCCESS, chg_out.imp_rc);

	chg_in.dom_entry = false;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(RIO_SUCCESS, chg_out.imp_rc);

	chg_in.idx = 0;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(RIO_SUCCESS, chg_out.imp_rc);

	// Change the multicast masks at the limits of the multicast table
	// in the conventional way.

	mc_in.mc_mask_rte = RIO_RTV_MC_MSK(0);
	memcpy(&mc_in.mc_info, &rt.mc_masks[0], sizeof(mc_in.mc_info));
	mc_in.mc_info.mc_mask = 0xFFFFF;
	mc_in.mc_info.in_use = true;
	mc_in.rt = init_in.rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_mc_mask(&mock_dev_info, &mc_in, &mc_out));
	assert_int_equal(RIO_SUCCESS, mc_out.imp_rc);

	mc_in.mc_mask_rte = RIO_RTV_MC_MSK(RXS2448_MAX_MC_MASK);
	memcpy(&mc_in.mc_info, &rt.mc_masks[RXS2448_MAX_MC_MASK],
						sizeof(mc_in.mc_info));
	mc_in.mc_info.mc_mask = 0xFFFFF;
	mc_in.mc_info.in_use = true;
	mc_in.rt = init_in.rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_mc_mask(&mock_dev_info, &mc_in, &mc_out));
	assert_int_equal(RIO_SUCCESS, mc_out.imp_rc);

	// Preserve the "changed" rt values before doing direct manipulations.
	// Clear "changed" indications to allow checking to pass.
	memcpy(&chk_rt, &rt, sizeof(chk_rt));
	chk_rt.dev_table[0].changed = false;
	chk_rt.dev_table[RIO_RT_GRP_SZ - 1].changed = false;
	chk_rt.dom_table[RIO_RT_GRP_SZ - 1].changed = false;
	chk_rt.mc_masks[0].changed = false;
	chk_rt.mc_masks[RXS2448_MAX_MC_MASK].changed = false;

	// Change change domain routing table entry 0 through direct
	// manipulation.  This will not update the "changed" indication.
	//
	// Do the same thing for two more entries.

	rt.dom_table[0].rte_val = chg_val;
	rt.dom_table[99].rte_val = chg_val;
	rt.dev_table[99].rte_val = chg_val;

	// Change multicast masks directly.
	// This does not update the "changed" indication.

	rt.mc_masks[99].mc_mask = 0xF00F;
	rt.mc_masks[233].mc_mask = 0xF00F;

	// Set all the routing table entries...
	set_in.set_on_port = port;
	set_in.rt = chg_in.rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_set_changed(&mock_dev_info, &set_in, &set_out));
	assert_int_equal(RIO_SUCCESS, set_out.imp_rc);

	// Check that the hardware matches what is in the
	// routing table state structure.
	set_in.rt = &chk_rt;
	rxs_check_set_rt_regs(&set_in);
}

static void rxs_set_changed_rt_test_success(void **state)
{
	rio_port_t max = NUM_RXS_PORTS(&mock_dev_info) - 1;

	// Try setting all on port 0, on the maximum available port, and
	// on all ports.
	rxs_set_changed_rt_test_success_val(state,
		0, RIO_RTV_PORT(0), RIO_RTV_PORT(0), RIO_RTV_PORT(1));
	rxs_set_changed_rt_test_success_val(state,
		max, RIO_RTV_PORT(max), RIO_RTV_PORT(max), RIO_RTV_PORT(0));
	rxs_set_changed_rt_test_success_val(state,
		RIO_ALL_PORTS,
		RIO_RTE_DROP, RIO_RTE_DFLT_PORT, RIO_RTE_DROP);
}

static void rxs_set_changed_rt_null_test(void **state)
{
	rio_rt_set_changed_in_t set_in;
	rio_rt_set_changed_out_t set_out;

	set_in.set_on_port = 0;
	set_in.rt = NULL;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_set_changed(&mock_dev_info, &set_in, &set_out));
	assert_int_not_equal(RIO_SUCCESS, set_out.imp_rc);

	set_in.set_on_port = RIO_ALL_PORTS;
	set_in.rt = NULL;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_set_changed(&mock_dev_info, &set_in, &set_out));
	assert_int_not_equal(RIO_SUCCESS, set_out.imp_rc);

	(void)state; // unused
}

static void rxs_set_changed_rt_bad_port_test(void **state)
{
	rio_rt_set_changed_in_t set_in;
	rio_rt_set_changed_out_t set_out;
	rio_rt_state_t rt;

	init_mock_rxs_reg(state);
	set_in.set_on_port = NUM_RXS_PORTS(&mock_dev_info);
	set_in.rt = &rt;
	set_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_set_changed(&mock_dev_info, &set_in, &set_out));
	assert_int_not_equal(RIO_SUCCESS, set_out.imp_rc);
}

static void rxs_set_changed_rt_dev_hw_bad_default_route_test(void **state)
{
	rio_rt_set_changed_in_t set_in;
	rio_rt_set_changed_out_t set_out;
	rio_rt_state_t rt;
	uint32_t idx;

	init_mock_rxs_reg(state);

	for (idx = 0; idx < num_bad_default_routes; idx++) {
		set_in.set_on_port = 0;
		set_in.rt = &rt;
		set_in.rt->default_route = bad_default_route_array[idx];
		set_out.imp_rc = RIO_SUCCESS;

		assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_rt_set_changed(&mock_dev_info,
							&set_in, &set_out));
		assert_int_not_equal(RIO_SUCCESS, set_out.imp_rc);
	}
}

static void rxs_set_all_rt_test_success_val(void **state, rio_port_t port,
				pe_rt_val dflt_rt, pe_rt_val dflt_val,
				pe_rt_val chg_val)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_change_rte_in_t chg_in;
	rio_rt_change_rte_out_t chg_out;
	rio_rt_change_mc_mask_in_t mc_in;
	rio_rt_change_mc_mask_out_t mc_out;
	rio_rt_set_all_in_t set_in;
	rio_rt_set_all_out_t set_out;
	rio_rt_state_t rt;

	init_mock_rxs_reg(state);

	init_in.set_on_port = port;
	init_in.default_route = dflt_rt;
	init_in.default_route_table_port = dflt_val;

	// Initialize routing table
	init_in.update_hw = true;
	init_in.rt = &rt;
	memset(&rt, 0, sizeof(rt));

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in,
			&init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Change a routing table entries the conventional way
	// at the limits of each table.  Note: Cannot change domain entry 0.
	chg_in.dom_entry = true;
	chg_in.idx = RIO_RT_GRP_SZ - 1;
	chg_in.rte_value = chg_val;
	chg_in.rt = init_in.rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(RIO_SUCCESS, chg_out.imp_rc);

	chg_in.dom_entry = false;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(RIO_SUCCESS, chg_out.imp_rc);

	chg_in.idx = 0;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(RIO_SUCCESS, chg_out.imp_rc);

	// Change change domain routing table entry 0 through direct
	// manipulation.  This will not update the "changed" indication.
	//
	// Do the same thing for two more entries.

	rt.dom_table[0].rte_val = chg_val;
	rt.dom_table[99].rte_val = chg_val;
	rt.dev_table[99].rte_val = chg_val;

	// Change the multicast masks at the limits of the multicast table
	// in the conventional way.

	mc_in.mc_mask_rte = RIO_RTV_MC_MSK(0);
	memcpy(&mc_in.mc_info, &rt.mc_masks[0], sizeof(mc_in.mc_info));
	mc_in.mc_info.mc_mask = 0xFFFFF;
	mc_in.mc_info.in_use = true;
	mc_in.rt = init_in.rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_mc_mask(&mock_dev_info, &mc_in, &mc_out));
	assert_int_equal(RIO_SUCCESS, mc_out.imp_rc);

	mc_in.mc_mask_rte = RIO_RTV_MC_MSK(RXS2448_MAX_MC_MASK);
	memcpy(&mc_in.mc_info, &rt.mc_masks[RXS2448_MAX_MC_MASK],
						sizeof(mc_in.mc_info));
	mc_in.mc_info.mc_mask = 0xFFFFF;
	mc_in.mc_info.in_use = true;
	mc_in.rt = init_in.rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_mc_mask(&mock_dev_info, &mc_in, &mc_out));
	assert_int_equal(RIO_SUCCESS, mc_out.imp_rc);

	// Then set a few directly.
	// This does not update the "changed" indication.

	rt.mc_masks[99].mc_mask = 0xF00F;
	rt.mc_masks[233].mc_mask = 0xF00F;

	// Set all the routing table entries...
	set_in.set_on_port = port;
	set_in.rt = chg_in.rt;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_set_all(&mock_dev_info, &set_in, &set_out));
	assert_int_equal(RIO_SUCCESS, set_out.imp_rc);

	// Check that the hardware matches what is in the
	// routing table state structure.
	rxs_check_set_rt_regs(&set_in);
}

static void rxs_set_all_rt_test_success(void **state)
{
	rio_port_t max = NUM_RXS_PORTS(&mock_dev_info) - 1;

	// Try setting all on port 0, on the maximum available port, and
	// on all ports.
	rxs_set_all_rt_test_success_val(state,
		0, RIO_RTV_PORT(0), RIO_RTV_PORT(0), RIO_RTV_PORT(1));
	rxs_set_all_rt_test_success_val(state,
		max, RIO_RTV_PORT(max), RIO_RTV_PORT(max), RIO_RTV_PORT(0));
	rxs_set_all_rt_test_success_val(state,
		RIO_ALL_PORTS,
		RIO_RTE_DROP, RIO_RTE_DFLT_PORT, RIO_RTE_DROP);
}

static void rxs_set_all_rt_null_test(void **state)
{
	rio_rt_set_all_in_t set_in;
	rio_rt_set_all_out_t set_out;

	set_in.set_on_port = 0;
	set_in.rt = NULL;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_set_all(&mock_dev_info,
					&set_in, &set_out));
	assert_int_not_equal(RIO_SUCCESS, set_out.imp_rc);

	set_in.set_on_port = RIO_ALL_PORTS;
	set_in.rt = NULL;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_set_all(&mock_dev_info, &set_in, &set_out));
	assert_int_not_equal(RIO_SUCCESS, set_out.imp_rc);

	(void)state; // unused
}

static void rxs_set_all_rt_bad_port_test(void **state)
{
	rio_rt_set_all_in_t set_in;
	rio_rt_set_all_out_t set_out;
	rio_rt_state_t rt;

	init_mock_rxs_reg(state);
	set_in.set_on_port = NUM_RXS_PORTS(&mock_dev_info);
	set_in.rt = &rt;
	set_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_set_changed(&mock_dev_info, &set_in, &set_out));
	assert_int_not_equal(RIO_SUCCESS, set_out.imp_rc);
}

static void rxs_set_all_rt_dev_hw_bad_default_route_test(void **state)
{
	rio_rt_set_all_in_t set_in;
	rio_rt_set_all_out_t set_out;
	rio_rt_state_t rt;
	uint32_t idx;

	init_mock_rxs_reg(state);

	for (idx = 0; idx < num_bad_default_routes; idx++) {
		set_in.set_on_port = 0;
		set_in.rt = &rt;
		set_in.rt->default_route = bad_default_route_array[idx];
		set_out.imp_rc = RIO_SUCCESS;

		assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_rt_set_changed(&mock_dev_info,
							&set_in, &set_out));
		assert_int_not_equal(RIO_SUCCESS, set_out.imp_rc);
	}
}

static void rxs_rio_rt_probe_all_success_test(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_change_rte_in_t chg_in;
	rio_rt_change_rte_out_t chg_out;
	rio_rt_set_all_in_t set_in;
	rio_rt_set_all_out_t set_out;
	rio_rt_alloc_mc_mask_in_t alloc_in;
	rio_rt_alloc_mc_mask_out_t alloc_out;
	rio_rt_change_mc_mask_in_t mc_chg_in;
	rio_rt_change_mc_mask_out_t mc_chg_out;
	rio_rt_probe_all_in_t pr_all_in;
	rio_rt_probe_all_out_t pr_all_out;

	rio_rt_state_t rt;
	rio_rt_state_t pr_rt;

	rio_port_t port;
	uint32_t port_idx, rte_idx;
	const rio_port_t pmax = NUM_RXS_PORTS(&mock_dev_info) - 1;
	rio_port_t port_array[] = {0, 12, pmax, RIO_ALL_PORTS};
	uint32_t num_ports = sizeof(port_array) / sizeof(port_array[0]);

	port_array[2] = NUM_RXS_PORTS(&mock_dev_info) - 1;

	for (port_idx = 0; port_idx < num_ports; port_idx++) {
		init_mock_rxs_reg(state);

		port = port_array[port_idx];

		// Initialize routing table
		init_in.set_on_port = port;
		init_in.default_route = RIO_RTE_DROP;
		if (RIO_ALL_PORTS == port) {
			init_in.default_route_table_port = RIO_RTE_DROP;
		} else {
			init_in.default_route_table_port = RIO_RTV_PORT(port);
		}
		init_in.update_hw = false;
		init_in.rt = &rt;
		memset(&rt, 0, sizeof(rt));

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_initialize(&mock_dev_info, &init_in,
							&init_out));
		assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

		// Modify every routing table and multicast mask entry
		for (rte_idx = 0; rte_idx < RIO_MAX_MC_MASKS; rte_idx++) {
			uint32_t mc_idx;

			// Allocate multicast mask
			alloc_in.rt = &rt;
			assert_int_equal(RIO_SUCCESS,
				rio_rt_alloc_mc_mask(&mock_dev_info,
							&alloc_in, &alloc_out));
			assert_int_equal(0, alloc_out.imp_rc);

			// Modify dev8 and dev16 routing table entries to
			// use the multicast mask
			mc_chg_in.mc_mask_rte = alloc_out.mc_mask_rte;
			mc_idx = RIO_RTV_GET_MC_MSK(alloc_out.mc_mask_rte);
			memcpy(&mc_chg_in.mc_info, &rt.mc_masks[mc_idx],
				sizeof(mc_chg_in.mc_info));
			mc_chg_in.mc_info.in_use = true;
			mc_chg_in.mc_info.tt = tt_dev8;
			mc_chg_in.mc_info.mc_destID = rte_idx;
			mc_chg_in.mc_info.mc_mask = (rte_idx << 8) | (rte_idx);
			mc_chg_in.rt = &rt;
			assert_int_equal(RIO_SUCCESS,
				rxs_rio_rt_change_mc_mask(&mock_dev_info,
					&mc_chg_in, &mc_chg_out));
			assert_int_equal(0, mc_chg_out.imp_rc);

			// Set Dev16 routing table entry...
			if (rte_idx) {
				chg_in.dom_entry = true;
				chg_in.idx = rte_idx;
				chg_in.rte_value = alloc_out.mc_mask_rte;
				chg_in.rt = &rt;
				assert_int_equal(RIO_SUCCESS,
					rxs_rio_rt_change_rte(&mock_dev_info,
							&chg_in, &chg_out));
				assert_int_equal(0, chg_out.imp_rc);
			}
		}

		// Program the routing table
		set_in.set_on_port = port;
		set_in.rt = &rt;

		assert_int_equal(RIO_SUCCESS,
			rio_rt_set_all(&mock_dev_info, &set_in, &set_out));
		assert_int_equal(0, set_out.imp_rc);

		// Probe the routing table
		pr_all_in.probe_on_port = port;
		pr_all_in.rt = &pr_rt;
		assert_int_equal(RIO_SUCCESS,
			rio_rt_probe_all(&mock_dev_info,
					&pr_all_in, &pr_all_out));
		assert_int_equal(0, pr_all_out.imp_rc);

		// Confirm programmed routing table matches probed routing table

		for (rte_idx = 0; rte_idx < RIO_MAX_MC_MASKS; rte_idx++) {
			assert_int_equal(pr_rt.dev_table[rte_idx].rte_val,
					rt.dev_table[rte_idx].rte_val);
			assert_int_equal(pr_rt.dev_table[rte_idx].rte_val,
					RIO_RTV_MC_MSK(rte_idx));
			assert_int_equal(pr_rt.dev_table[rte_idx].changed,
					rt.dev_table[rte_idx].changed);
			assert_false(pr_rt.dev_table[rte_idx].changed);

			// Do not check index 0 of domain table, it must always
			// be "USE_DEV_TABLE"
			if (!rte_idx) {
				continue;
			}
			assert_int_equal(pr_rt.dom_table[rte_idx].rte_val,
					RIO_RTV_MC_MSK(rte_idx));
			assert_int_equal(pr_rt.dom_table[rte_idx].rte_val,
					rt.dom_table[rte_idx].rte_val);
			assert_int_equal(pr_rt.dom_table[rte_idx].changed,
					rt.dom_table[rte_idx].changed);
			assert_false(pr_rt.dom_table[rte_idx].changed);
		}
		// Do not check the multicast mask destdid or tt, as they may
		// differ from what was programmed.

		for (rte_idx = 0; rte_idx < RIO_MAX_MC_MASKS; rte_idx++) {
			assert_int_equal(pr_rt.mc_masks[rte_idx].mc_mask,
					rt.mc_masks[rte_idx].mc_mask);
			assert_true(pr_rt.mc_masks[rte_idx].in_use);
			assert_true(pr_rt.mc_masks[rte_idx].allocd);
		}
	}
}

static void rxs_rio_rt_probe_all_bad_parms_test(void **state)
{
	rio_rt_probe_all_in_t pr_all_in;
	rio_rt_probe_all_out_t pr_all_out;
	rio_rt_state_t probe_rt;

	// Bad port number
	pr_all_in.probe_on_port = NUM_RXS_PORTS(&mock_dev_info);
	pr_all_in.rt = &probe_rt;
	assert_int_not_equal(RIO_SUCCESS,
		rio_rt_probe_all(&mock_dev_info, &pr_all_in, &pr_all_out));
	assert_int_not_equal(0, pr_all_out.imp_rc);

	// Null routing table pointer
	pr_all_in.probe_on_port = 0;
	pr_all_in.rt = NULL;
	assert_int_not_equal(RIO_SUCCESS,
		rio_rt_probe_all(&mock_dev_info, &pr_all_in, &pr_all_out));
	assert_int_not_equal(0, pr_all_out.imp_rc);

	(void)state;
}

static void rxs_rio_rt_probe_bad_parms_test(void **state)
{
	rio_rt_probe_in_t pr_in;
	rio_rt_probe_out_t pr_out;
	rio_rt_state_t rt;

	// Test bad port
	pr_in.probe_on_port = NUM_RXS_PORTS(&mock_dev_info);
	pr_in.tt = tt_dev16;
	pr_in.destID = 0;
	pr_in.rt = &rt;
	pr_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
	assert_int_not_equal(RIO_SUCCESS, pr_out.imp_rc);

	// Test bad tt value
	pr_in.probe_on_port = 0;
	pr_in.tt = tt_t(3);
	pr_in.destID = 0;
	pr_in.rt = &rt;
	pr_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
	assert_int_not_equal(RIO_SUCCESS, pr_out.imp_rc);

	// Test bad destID
	pr_in.probe_on_port = 0;
	pr_in.tt = tt_dev8;
	pr_in.destID = 0x1FF;
	pr_in.rt = &rt;
	pr_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
	assert_int_not_equal(RIO_SUCCESS, pr_out.imp_rc);

	// Test NULL routing table pointer
	pr_in.probe_on_port = 0;
	pr_in.tt = tt_dev8;
	pr_in.destID = 0x1FF;
	pr_in.rt = NULL;
	pr_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
	assert_int_not_equal(RIO_SUCCESS, pr_out.imp_rc);

	(void)state;
}

static void rxs_rio_rt_probe_success_port_test(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_alloc_mc_mask_in_t alloc_in;
	rio_rt_alloc_mc_mask_out_t alloc_out;
	rio_rt_change_mc_mask_in_t mc_chg_in;
	rio_rt_change_mc_mask_out_t mc_chg_out;
	rio_rt_change_rte_in_t chg_in;
	rio_rt_change_rte_out_t chg_out;
	rio_rt_probe_in_t pr_in;
	rio_rt_probe_out_t pr_out;
	rio_rt_probe_out_t pr_out2;
	rio_rt_state_t rt;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Multicast mask for port 2, 4, 5, 9, and 12...
	uint32_t mc_mask = 0x1234;
	rio_port_t pt;
	did_reg_t did_reg_val;

	// Set all ports to "perfect"
	set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, RIO_ALL_PORTS);

	// Initialize routing table
	init_in.set_on_port = 0;
	init_in.default_route = RIO_RTV_PORT(0);
	init_in.default_route_table_port = RIO_RTV_PORT(1);
	init_in.update_hw = false;
	init_in.rt = &rt;
	memset(&rt, 0, sizeof(rt));

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Allocate multicast mask
	alloc_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rio_rt_alloc_mc_mask(&mock_dev_info, &alloc_in, &alloc_out));
	assert_int_equal(0, alloc_out.imp_rc);

	// Modify dev8 routing table entry 1 to
	// use the multicast mask
	mc_chg_in.mc_mask_rte = alloc_out.mc_mask_rte;
	mc_chg_in.mc_info.in_use = true;
	mc_chg_in.mc_info.tt = tt_dev8;
	mc_chg_in.mc_info.mc_destID = 1;
	mc_chg_in.mc_info.mc_mask = mc_mask;
	mc_chg_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_mc_mask(&mock_dev_info,
					&mc_chg_in, &mc_chg_out));
	assert_int_equal(0, mc_chg_out.imp_rc);

	// Modify Dev16 routing table entry 2 (destIDs 0x0200 through 0x02FF)
	// use the multicast mask
	chg_in.dom_entry = true;
	chg_in.idx = 2;
	chg_in.rte_value = alloc_out.mc_mask_rte;
	chg_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(0, chg_out.imp_rc);

	// Modify dev8 routing table entry 3 to use the default port
	chg_in.dom_entry = false;
	chg_in.idx = 3;
	chg_in.rte_value = RIO_RTE_DFLT_PORT;
	chg_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(0, chg_out.imp_rc);

	// Modify dev16 routing table entry 4 (destIDs 0x0400 through 0x04FF)
	// to use the device routing table
	chg_in.dom_entry = true;
	chg_in.idx = 4;
	chg_in.rte_value = RIO_RTV_LVL_GRP(0);
	chg_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(0, chg_out.imp_rc);

	// Check routing for all destIDs of the form 0xXX, 0x00XX and 0x04XX

	for (did_reg_val = 0; did_reg_val <= RIO_LAST_DEV8; did_reg_val++) {
		// Check destID 0xXX
		pr_in.probe_on_port = 0;
		pr_in.tt = tt_dev8;
		pr_in.destID = did_reg_val;
		pr_in.rt = &rt;
		memset(&pr_out, 0, sizeof(pr_out));

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));

		assert_int_equal(RIO_SUCCESS, pr_out.imp_rc);
		assert_true(pr_out.valid_route);

		switch (did_reg_val) {
		case 1:
			assert_int_equal(pr_out.routing_table_value,
							alloc_out.mc_mask_rte);
			for (pt = 0; pt < NUM_RXS_PORTS(&mock_dev_info); pt++) {
				if (mc_mask & (1 << pt)) {
					assert_true(pr_out.mcast_ports[pt]);
				} else {
					assert_false(pr_out.mcast_ports[pt]);
				}
			}
			break;
		case 3:
			assert_int_equal(pr_out.routing_table_value,
							RIO_RTE_DFLT_PORT);
			assert_int_equal(pr_out.default_route,
					init_in.default_route);
			break;
		default:
			assert_int_equal(pr_out.routing_table_value,
					init_in.default_route_table_port);
			break;
		}
		// Check destID 0x00XX matches destID 0xXX
		pr_in.probe_on_port = 0;
		pr_in.tt = tt_dev16;
		pr_in.destID = did_reg_val;
		pr_in.rt = &rt;
		memset(&pr_out2, 0, sizeof(pr_out2));

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out2));
		assert_int_equal(0, memcmp(&pr_out, &pr_out2, sizeof(pr_out)));

		// Check destID 0x04XX matches destID 0xXX
		pr_in.probe_on_port = 0;
		pr_in.tt = tt_dev16;
		pr_in.destID = 0x0400 + did_reg_val;
		pr_in.rt = &rt;
		memset(&pr_out2, 0, sizeof(pr_out2));

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out2));
		assert_int_equal(0, memcmp(&pr_out, &pr_out2, sizeof(pr_out)));
	}

	// Check routing for all dev16 destIDs of the form 0x02XX

	for (did_reg_val = 0; did_reg_val <= RIO_LAST_DEV8; did_reg_val++) {
		pr_in.probe_on_port = 0;
		pr_in.tt = tt_dev16;
		pr_in.destID = 0x0200 + did_reg_val;
		pr_in.rt = &rt;
		memset(&pr_out, 0, sizeof(pr_out));

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));

		assert_int_equal(RIO_SUCCESS, pr_out.imp_rc);
		assert_true(pr_out.valid_route);

		assert_int_equal(pr_out.routing_table_value,
							alloc_out.mc_mask_rte);
		for (pt = 0; pt < NUM_RXS_PORTS(&mock_dev_info); pt++) {
			if (mc_mask & (1 << pt)) {
				assert_true(pr_out.mcast_ports[pt]);
			} else {
				assert_false(pr_out.mcast_ports[pt]);
			}
		}
	}

	(void)state;
}

static void rxs_rio_rt_probe_success_mc_port_test(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_alloc_mc_mask_in_t alloc_in;
	rio_rt_alloc_mc_mask_out_t alloc_out;
	rio_rt_change_mc_mask_in_t mc_chg_in;
	rio_rt_change_mc_mask_out_t mc_chg_out;
	rio_rt_probe_in_t pr_in;
	rio_rt_probe_out_t pr_out;
	rio_rt_state_t rt;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Set all ports to "perfect"
	set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, RIO_ALL_PORTS);

	// Multicast mask for all ports
	uint32_t mc_mask = (1 << NUM_RXS_PORTS(&mock_dev_info)) - 1;
	rio_port_t pt, chk_pt;
	did_reg_t did_reg_val = 1;

	// Initialize routing table
	init_in.set_on_port = 0;
	init_in.default_route = RIO_RTV_PORT(0);
	init_in.default_route_table_port = RIO_RTV_PORT(1);
	init_in.update_hw = false;
	init_in.rt = &rt;
	memset(&rt, 0, sizeof(rt));

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Allocate multicast mask
	alloc_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rio_rt_alloc_mc_mask(&mock_dev_info, &alloc_in, &alloc_out));
	assert_int_equal(0, alloc_out.imp_rc);

	// Modify dev8 routing table entry 1 to
	// use the multicast mask
	mc_chg_in.mc_mask_rte = alloc_out.mc_mask_rte;
	mc_chg_in.mc_info.in_use = true;
	mc_chg_in.mc_info.tt = tt_dev8;
	mc_chg_in.mc_info.mc_destID = did_reg_val;
	mc_chg_in.mc_info.mc_mask = mc_mask;
	mc_chg_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_mc_mask(&mock_dev_info,
					&mc_chg_in, &mc_chg_out));
	assert_int_equal(0, mc_chg_out.imp_rc);

	// Check multicast routing excludes the probed on port

	for (pt = 0; pt <= NUM_RXS_PORTS(&mock_dev_info); pt++) {
		// Check destID 0xXX
		if (pt == NUM_RXS_PORTS(&mock_dev_info)) {
			pr_in.probe_on_port = RIO_ALL_PORTS;
		} else {
			pr_in.probe_on_port = pt;
		}
		pr_in.tt = tt_dev8;
		pr_in.destID = did_reg_val;
		pr_in.rt = &rt;
		memset(&pr_out, 0, sizeof(pr_out));

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));

		assert_int_equal(RIO_SUCCESS, pr_out.imp_rc);
		assert_true(pr_out.valid_route);

		assert_int_equal(pr_out.routing_table_value,
							alloc_out.mc_mask_rte);
		for (chk_pt = 0; chk_pt < NUM_RXS_PORTS(&mock_dev_info);
								chk_pt++) {
			if ((mc_mask & (1 << chk_pt)) && (pt != chk_pt)) {
				assert_true(pr_out.mcast_ports[chk_pt]);
			} else {
				assert_false(pr_out.mcast_ports[chk_pt]);
			}
		}
	}

	(void)state;
}

static void rxs_rio_rt_probe_discard_rt_port_test(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_change_rte_in_t chg_in;
	rio_rt_change_rte_out_t chg_out;
	rio_rt_probe_in_t pr_in;
	rio_rt_probe_out_t pr_out;
	rio_rt_probe_out_t pr_out2;
	rio_rt_state_t rt;

	did_reg_t did_reg_val;

	// Initialize routing table
	init_in.set_on_port = RIO_ALL_PORTS;
	init_in.default_route = RIO_RTE_DROP;
	init_in.default_route_table_port = RIO_RTE_DROP;
	init_in.update_hw = false;
	init_in.rt = &rt;
	memset(&rt, 0, sizeof(rt));

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Modify dev8 routing table entry 3 to use the default port
	chg_in.dom_entry = false;
	chg_in.idx = 3;
	chg_in.rte_value = RIO_RTE_DFLT_PORT;
	chg_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(0, chg_out.imp_rc);

	// Modify dev16 routing table entry 0xFF00 to use the default port
	chg_in.dom_entry = true;
	chg_in.idx = RIO_RT_GRP_SZ - 1;
	chg_in.rte_value = RIO_RTE_DFLT_PORT;
	chg_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(0, chg_out.imp_rc);

	// Check routing for all destIDs of the form 0xXX, 0x00XX and 0xffXX

	for (did_reg_val = 0; did_reg_val <= RIO_LAST_DEV8; did_reg_val++) {
		// Check destID 0xXX
		pr_in.probe_on_port = 0;
		pr_in.tt = tt_dev8;
		pr_in.destID = did_reg_val;
		pr_in.rt = &rt;
		memset(&pr_out, 0, sizeof(pr_out));

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));

		assert_int_equal(RIO_SUCCESS, pr_out.imp_rc);
		assert_false(pr_out.valid_route);

		if (3 == did_reg_val) {
			assert_int_equal(pr_out.routing_table_value,
							RIO_RTE_DFLT_PORT);
			assert_int_equal(pr_out.default_route, RIO_RTE_DROP);
			assert_int_equal(pr_out.default_route,
					rt.default_route);
			assert_int_equal(pr_out.reason_for_discard,
					rio_rt_disc_dflt_pt_deliberately);
		} else {
			assert_int_equal(pr_out.default_route,
					rt.default_route);
			assert_int_equal(pr_out.routing_table_value,
					RIO_RTE_DROP);
			assert_int_equal(pr_out.reason_for_discard,
					rio_rt_disc_deliberately);
		}
		// Check destID 0x00XX matches destID 0xXX
		pr_in.probe_on_port = 0;
		pr_in.tt = tt_dev16;
		pr_in.destID = did_reg_val;
		pr_in.rt = &rt;
		memset(&pr_out2, 0, sizeof(pr_out2));

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out2));
		assert_int_equal(0, memcmp(&pr_out, &pr_out2, sizeof(pr_out)));

	}

	for (did_reg_val = 0; did_reg_val <= RIO_LAST_DEV8; did_reg_val++) {
		// Check destID 0xffXX matches destID 0xXX
		pr_in.probe_on_port = 0;
		pr_in.tt = tt_dev16;
		pr_in.destID = 0xFF00 + did_reg_val;
		pr_in.rt = &rt;
		memset(&pr_out, 0, sizeof(pr_out));

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
		assert_int_equal(pr_out.routing_table_value, RIO_RTE_DFLT_PORT);
		assert_int_equal(pr_out.default_route, RIO_RTE_DROP);
		assert_int_equal(pr_out.reason_for_discard,
				rio_rt_disc_dflt_pt_deliberately);
	}

	(void)state;
}

static void rxs_rio_rt_probe_discard_rt_mc_test(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_alloc_mc_mask_in_t alloc_in;
	rio_rt_alloc_mc_mask_out_t alloc_out;
	rio_rt_change_mc_mask_in_t mc_chg_in;
	rio_rt_change_mc_mask_out_t mc_chg_out;
	rio_rt_probe_in_t pr_in;
	rio_rt_probe_out_t pr_out;
	rio_rt_state_t rt;

	uint32_t mc_mask;
	rio_port_t pt, c_pt;
	did_reg_t did_reg_val = 1;

	// Set all ports to "perfect"
	set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, 0);

	// Initialize routing table
	init_in.set_on_port = 0;
	init_in.default_route = RIO_RTE_DROP;
	init_in.default_route_table_port = RIO_RTE_DROP;
	init_in.update_hw = false;
	init_in.rt = &rt;
	memset(&rt, 0, sizeof(rt));

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Allocate multicast mask
	alloc_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rio_rt_alloc_mc_mask(&mock_dev_info, &alloc_in, &alloc_out));
	assert_int_equal(0, alloc_out.imp_rc);

	for (pt = 0; pt < NUM_RXS_PORTS(&mock_dev_info); pt++) {
		// Multicast mask has a single bit set, equal to port number
		mc_mask = 1 << pt;

		// Modify dev8 routing table entry 1 to
		// use the multicast mask, and set multicast mask
		mc_chg_in.mc_mask_rte = alloc_out.mc_mask_rte;
		mc_chg_in.mc_info.in_use = true;
		mc_chg_in.mc_info.tt = tt_dev8;
		mc_chg_in.mc_info.mc_destID = did_reg_val;
		mc_chg_in.mc_info.mc_mask = mc_mask;
		mc_chg_in.rt = &rt;
		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_change_mc_mask(&mock_dev_info,
						&mc_chg_in, &mc_chg_out));
		assert_int_equal(0, mc_chg_out.imp_rc);

		for (c_pt = 0; c_pt < NUM_RXS_PORTS(&mock_dev_info); c_pt++) {
			// Check destID 0xXX
			pr_in.probe_on_port = c_pt;
			pr_in.tt = tt_dev8;
			pr_in.destID = did_reg_val;
			pr_in.rt = &rt;
			memset(&pr_out, 0, sizeof(pr_out));

			assert_int_equal(RIO_SUCCESS,
				rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));

			assert_int_equal(RIO_SUCCESS, pr_out.imp_rc);
			assert_int_equal(pr_out.routing_table_value,
							alloc_out.mc_mask_rte);
			if (pt == c_pt) {
				assert_false(pr_out.valid_route);
				assert_int_equal(pr_out.reason_for_discard,
							rio_rt_disc_mc_one_bit);
				continue;
			}
			assert_true(pr_out.valid_route);
			for (pt = 0; pt < NUM_RXS_PORTS(&mock_dev_info); pt++) {
				if (mc_mask & (1 << pt)) {
					assert_true(pr_out.mcast_ports[pt]);
				} else {
					assert_false(pr_out.mcast_ports[pt]);
				}
			}
		}
	}

	// Test when multicast mask has no bits set...
	mc_mask = 0;

	// Modify dev8 routing table entry 1 to
	// use the multicast mask, and set multicast mask
	mc_chg_in.mc_mask_rte = alloc_out.mc_mask_rte;
	mc_chg_in.mc_info.in_use = true;
	mc_chg_in.mc_info.tt = tt_dev8;
	mc_chg_in.mc_info.mc_destID = 1;
	mc_chg_in.mc_info.mc_mask = mc_mask;
	mc_chg_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_mc_mask(&mock_dev_info,
						&mc_chg_in, &mc_chg_out));
	assert_int_equal(0, mc_chg_out.imp_rc);

	for (c_pt = 0; c_pt <= NUM_RXS_PORTS(&mock_dev_info); c_pt++) {
		// Check destID 0xXX
		pr_in.probe_on_port = 0;
		pr_in.tt = tt_dev8;
		pr_in.destID = did_reg_val;
		pr_in.rt = &rt;
		memset(&pr_out, 0, sizeof(pr_out));

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));

		assert_int_equal(RIO_SUCCESS, pr_out.imp_rc);
		assert_int_equal(pr_out.routing_table_value,
						alloc_out.mc_mask_rte);
		assert_false(pr_out.valid_route);
		assert_int_equal(pr_out.reason_for_discard,
						rio_rt_disc_mc_empty);
	}
	(void)state;
}

static void rxs_rio_rt_probe_discard_pt_fail_test_did(rio_rt_state_t *rt,
					tt_t tt,
					uint32_t did,
					rio_port_t probe_on_port,
					rio_port_t cfg_port)
{
	rio_rt_probe_in_t pr_in;
	rio_rt_probe_out_t pr_out;

	uint32_t lpbk_mask = RXS_PLM_SPX_IMP_SPEC_CTL_DLB_EN |
				RXS_PLM_SPX_IMP_SPEC_CTL_LLB_EN;
	uint32_t temp;

	set_all_port_config(cfg_unavl, NO_TTL, NO_FILT, cfg_port);
	pr_in.probe_on_port = probe_on_port;
	pr_in.tt = tt;
	pr_in.destID = did;
	pr_in.rt = rt;
	memset(&pr_out, 0, sizeof(pr_out));

	assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
	assert_false(pr_out.valid_route);
	assert_false(pr_out.trace_function_active);
	assert_false(pr_out.time_to_live_active);
	assert_false(pr_out.filter_function_active);
	assert_int_equal(pr_out.reason_for_discard, rio_rt_disc_port_unavail);

	set_all_port_config(cfg_avl_pwdn, NO_TTL, NO_FILT, cfg_port);
	pr_in.probe_on_port = probe_on_port;
	pr_in.tt = tt;
	pr_in.destID = did;
	pr_in.rt = rt;
	memset(&pr_out, 0, sizeof(pr_out));

	assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
	assert_false(pr_out.valid_route);
	assert_false(pr_out.trace_function_active);
	assert_false(pr_out.time_to_live_active);
	assert_false(pr_out.filter_function_active);
	assert_int_equal(pr_out.reason_for_discard, rio_rt_disc_port_pwdn);

	set_all_port_config(cfg_pwup_txdis, NO_TTL, NO_FILT, cfg_port);
	pr_in.probe_on_port = probe_on_port;
	pr_in.tt = tt;
	pr_in.destID = did;
	pr_in.rt = rt;
	memset(&pr_out, 0, sizeof(pr_out));

	assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
	assert_false(pr_out.valid_route);
	assert_false(pr_out.trace_function_active);
	assert_false(pr_out.time_to_live_active);
	assert_false(pr_out.filter_function_active);
	assert_int_equal(pr_out.reason_for_discard,
						rio_rt_disc_port_lkout_or_dis);

	set_all_port_config(cfg_txen_no_lp, NO_TTL, NO_FILT, cfg_port);
	pr_in.probe_on_port = probe_on_port;
	pr_in.tt = tt;
	pr_in.destID = did;
	pr_in.rt = rt;
	memset(&pr_out, 0, sizeof(pr_out));

	assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
	assert_false(pr_out.valid_route);
	assert_false(pr_out.trace_function_active);
	assert_false(pr_out.time_to_live_active);
	assert_false(pr_out.filter_function_active);
	assert_int_equal(pr_out.reason_for_discard, rio_rt_disc_port_no_lp);

	set_all_port_config(cfg_txen_lp_perr, NO_TTL, NO_FILT, cfg_port);
	pr_in.probe_on_port = probe_on_port;
	pr_in.tt = tt;
	pr_in.destID = did;
	pr_in.rt = rt;
	memset(&pr_out, 0, sizeof(pr_out));

	assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
	assert_false(pr_out.valid_route);
	assert_false(pr_out.trace_function_active);
	assert_false(pr_out.time_to_live_active);
	assert_false(pr_out.filter_function_active);
	assert_int_equal(pr_out.reason_for_discard, rio_rt_disc_port_fail);

	set_all_port_config(cfg_lp_lkout, NO_TTL, NO_FILT, cfg_port);
	pr_in.probe_on_port = probe_on_port;
	pr_in.tt = tt;
	pr_in.destID = did;
	pr_in.rt = rt;
	memset(&pr_out, 0, sizeof(pr_out));

	assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
	assert_false(pr_out.valid_route);
	assert_false(pr_out.trace_function_active);
	assert_false(pr_out.time_to_live_active);
	assert_false(pr_out.filter_function_active);
	assert_int_equal(pr_out.reason_for_discard,
						rio_rt_disc_port_lkout_or_dis);

	set_all_port_config(cfg_lp_nmtc_dis, YES_TTL, NO_FILT, cfg_port);
	pr_in.probe_on_port = probe_on_port;
	pr_in.tt = tt;
	pr_in.destID = did;
	pr_in.rt = rt;
	memset(&pr_out, 0, sizeof(pr_out));

	assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
	assert_false(pr_out.valid_route);
	assert_false(pr_out.trace_function_active);
	assert_true(pr_out.time_to_live_active);
	assert_false(pr_out.filter_function_active);
	assert_int_equal(pr_out.reason_for_discard,
						rio_rt_disc_port_in_out_dis);

	set_all_port_config(cfg_lp_lpbk, NO_TTL, YES_FILT, cfg_port);
	pr_in.probe_on_port = probe_on_port;
	pr_in.tt = tt;
	pr_in.destID = did;
	pr_in.rt = rt;
	memset(&pr_out, 0, sizeof(pr_out));

	assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));

	assert_int_equal(RIO_SUCCESS,
		RXSReadReg(&mock_dev_info, RXS_PLM_SPX_IMP_SPEC_CTL(cfg_port),
						&temp));
	assert_int_equal(temp & lpbk_mask, lpbk_mask);
	assert_int_equal(pr_out.reason_for_discard, rio_rt_disc_imp_spec);
	assert_false(pr_out.valid_route);
	assert_false(pr_out.trace_function_active);
	assert_false(pr_out.time_to_live_active);
	assert_true(pr_out.filter_function_active);

	set_all_port_config(cfg_lp_ecc, YES_TTL, YES_FILT, cfg_port);
	pr_in.probe_on_port = probe_on_port;
	pr_in.tt = tt;
	pr_in.destID = did;
	pr_in.rt = rt;
	memset(&pr_out, 0, sizeof(pr_out));

	assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
	assert_false(pr_out.valid_route);
	assert_false(pr_out.trace_function_active);
	assert_true(pr_out.time_to_live_active);
	assert_true(pr_out.filter_function_active);
	assert_int_equal(pr_out.reason_for_discard, rio_rt_disc_imp_spec);

	set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, cfg_port);
}

static void rxs_rio_rt_probe_discard_pt_fail_test(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_change_rte_in_t chg_in;
	rio_rt_change_rte_out_t chg_out;
	rio_rt_state_t rt;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Initialize routing table
	init_in.set_on_port = 0;
	init_in.default_route = RIO_RTE_DROP;
	init_in.default_route_table_port = RIO_RTE_DROP;
	init_in.update_hw = false;
	init_in.rt = &rt;
	memset(&rt, 0, sizeof(rt));

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Modify dev8 routing table entry 3 to select port 2
	chg_in.dom_entry = false;
	chg_in.idx = 3;
	chg_in.rte_value = RIO_RTV_PORT(2);
	chg_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(0, chg_out.imp_rc);

	// Modify dev16 routing table entry 7 (destID 0x0700 to 0x07FF)
	// to select port 2
	chg_in.dom_entry = true;
	chg_in.idx = 7;
	chg_in.rte_value = RIO_RTV_PORT(2);
	chg_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(0, chg_out.imp_rc);

	// Modify dev16 routing table entry 0xFF (destID 0xff00 to 0xffFF)
	// to use the device routing table
	chg_in.dom_entry = true;
	chg_in.idx = RIO_LAST_DEV8;
	chg_in.rte_value = RIO_RTV_LVL_GRP(0);
	chg_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_rte(&mock_dev_info, &chg_in, &chg_out));
	assert_int_equal(0, chg_out.imp_rc);

	// Perform the same packet discard tests with the configured
	// destination IDs, with full range of ports

	if (DEBUG_PRINTF) {
		printf("\ndev08 3 probe 0 config 2\n");
	}
	rxs_rio_rt_probe_discard_pt_fail_test_did(&rt, tt_dev8, 3, 0, 2);
	if (DEBUG_PRINTF) {
		printf("\ndev16 0x799 probe 6 config 2\n");
	}
	rxs_rio_rt_probe_discard_pt_fail_test_did(&rt, tt_dev16, 0x0799, 6, 2);
	if (DEBUG_PRINTF) {
		printf("\ndev16 0x0700 probe 14 config 2\n");
	}
	rxs_rio_rt_probe_discard_pt_fail_test_did(&rt, tt_dev16, 0x0700, 14, 2);
	if (DEBUG_PRINTF) {
		printf("\ndev16 0xFF03 23 or 15 config 2\n");
	}
	rxs_rio_rt_probe_discard_pt_fail_test_did(&rt, tt_dev16, 0xFF03,
					NUM_RXS_PORTS(&mock_dev_info) - 1, 2);

	(void)state;
}

static void rxs_rio_rt_probe_discard_mc_fail_test(void **state)
{
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;
	rio_rt_probe_in_t pr_in;
	rio_rt_probe_out_t pr_out;
	rio_rt_alloc_mc_mask_in_t alloc_in;
	rio_rt_alloc_mc_mask_out_t alloc_out;
	rio_rt_change_mc_mask_in_t mc_chg_in;
	rio_rt_change_mc_mask_out_t mc_chg_out;
	rio_rt_state_t rt;
	uint32_t mc_mask;
	rio_port_t port;
	rio_port_t test;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Initialize routing table
	init_in.set_on_port = 0;
	init_in.default_route = RIO_RTE_DROP;
	init_in.default_route_table_port = RIO_RTE_DROP;
	init_in.update_hw = false;
	init_in.rt = &rt;
	memset(&rt, 0, sizeof(rt));

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_initialize(&mock_dev_info, &init_in, &init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Allocate multicast mask
	alloc_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rio_rt_alloc_mc_mask(&mock_dev_info, &alloc_in, &alloc_out));
	assert_int_equal(0, alloc_out.imp_rc);

	// Multicast mask has all bits set.
	mc_mask = (1 << NUM_RXS_PORTS(&mock_dev_info)) - 1;

	// Modify dev8 routing table entry 1 to
	// use the multicast mask, and set multicast mask
	mc_chg_in.mc_mask_rte = alloc_out.mc_mask_rte;
	mc_chg_in.mc_info.in_use = true;
	mc_chg_in.mc_info.tt = tt_dev8;
	mc_chg_in.mc_info.mc_destID = 1;
	mc_chg_in.mc_info.mc_mask = mc_mask;
	mc_chg_in.rt = &rt;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_rt_change_mc_mask(&mock_dev_info,
						&mc_chg_in, &mc_chg_out));
	assert_int_equal(0, mc_chg_out.imp_rc);

	// By default, all ports are powered down and unavailable.
	// Set each port to perfect, and confirm that the port is available
	// in the reported multicast mask.
	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		// Set port to perfect
		set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, port);

		// Successful probe port
		pr_in.probe_on_port = RIO_ALL_PORTS;
		pr_in.tt = tt_dev8;
		pr_in.destID = 1;
		pr_in.rt = &rt;
		pr_out.imp_rc = RIO_SUCCESS;

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_rt_probe(&mock_dev_info, &pr_in, &pr_out));
		assert_int_equal(RIO_SUCCESS, pr_out.imp_rc);

		assert_true(pr_out.valid_route);
		assert_int_equal(pr_out.routing_table_value, alloc_out.mc_mask_rte);
		assert_false(pr_out.filter_function_active);
		assert_false(pr_out.trace_function_active);
		assert_false(pr_out.time_to_live_active);

		// Check that all ports up to the current one are present
		for (test = 0; test < NUM_RXS_PORTS(&mock_dev_info); test++) {
			if (test <= port) {
				assert_true(pr_out.mcast_ports[test]);
			} else {
				assert_false(pr_out.mcast_ports[test]);
			}
		}
	}

	(void)state;
}

int main(int argc, char** argv)
{
	const struct CMUnitTest tests[] = {
			cmocka_unit_test_setup_teardown(
					rxs_chk_and_corr_rtv_success_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_chk_and_corr_rtv_error_test,
					setup,
					NULL),

			cmocka_unit_test_setup_teardown(
					rxs_init_rt_test_success,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_init_rt_null_test_success,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_init_rt_test_bad_default_route,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_init_rt_test_bad_default_route_table,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_init_rt_test_bad_port,
					setup,
					NULL),

			cmocka_unit_test_setup_teardown(
					rxs_change_rte_rt_test_success,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_change_rte_rt_null_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_change_rte_rt_bad_rte_test,
					setup,
					NULL),

			cmocka_unit_test_setup_teardown(
					rxs_alloc_mc_rt_test_success,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_alloc_mc_rt_null_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_alloc_mc_rt_bad_in_use_allocd_test,
					setup,
					NULL),

			cmocka_unit_test_setup_teardown(
					rxs_change_allocd_mc_rt_test_success,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_change_mc_rt_test_bad_parms,
					setup,
					NULL),

			cmocka_unit_test_setup_teardown(
					rxs_dealloc_mc_rt_test_unalloced_success,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_dealloc_mc_rt_test_success,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_dealloc_change_mc_rt_test_success,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_dealloc_change_rt_rt_test_success,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_dealloc_mc_rt_bad_parms,
					setup,
					NULL),

			cmocka_unit_test_setup_teardown(
					rxs_change_mc_rt_dev_inuse_hw_test_success,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_change_mc_rt_dom_inuse_hw_test_success,
					setup,
					NULL),

			cmocka_unit_test_setup_teardown(
					rxs_set_changed_rt_test_success,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_set_changed_rt_null_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_set_changed_rt_bad_port_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_set_changed_rt_dev_hw_bad_default_route_test,
					setup,
					NULL),

			cmocka_unit_test_setup_teardown(
					rxs_set_all_rt_test_success,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_set_all_rt_null_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_set_all_rt_bad_port_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_set_all_rt_dev_hw_bad_default_route_test,
					setup,
					NULL),

			cmocka_unit_test_setup_teardown(
					rxs_rio_rt_probe_all_success_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_rio_rt_probe_all_bad_parms_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_rio_rt_probe_success_port_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_rio_rt_probe_success_mc_port_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_rio_rt_probe_discard_rt_mc_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_rio_rt_probe_discard_rt_port_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_rio_rt_probe_discard_pt_fail_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_rio_rt_probe_discard_mc_fail_test,
					setup,
					NULL),
			cmocka_unit_test_setup_teardown(
					rxs_rio_rt_probe_bad_parms_test,
					setup,
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
