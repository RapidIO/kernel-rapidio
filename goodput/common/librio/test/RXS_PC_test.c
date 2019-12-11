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
#include "src/RXS_PC.c"
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

static void rxs_rio_pc_macros_test(void **state)
{
	assert_int_equal(RIO_SPX_CTL_PTW_MAX_LANES(0x40000000), 4);
	assert_int_equal(RIO_SPX_CTL_PTW_MAX_LANES(0xC0000000), 4);
	assert_int_equal(RIO_SPX_CTL_PTW_MAX_LANES(0x80000000), 2);
	assert_int_equal(RIO_SPX_CTL_PTW_MAX_LANES(0x00000000), 1);

	(void)state;
}

typedef struct clk_pd_tests_t_TAG {
	uint32_t ps; // Prescalar
	uint32_t cfgsig0; // Ref clock, clock config
	uint32_t clk_pd; // Expected clock period
} clk_pd_tests_t;

#define LO_LAT     RXS_MPM_CFGSIG0_CORECLK_SELECT_LO_LAT
#define LO_RSVD    RXS_MPM_CFGSIG0_CORECLK_SELECT_RSVD
#define LO_PWR_12G RXS_MPM_CFGSIG0_CORECLK_SELECT_LO_PWR_12G
#define LO_PWR_10G RXS_MPM_CFGSIG0_CORECLK_SELECT_LO_PWR_10G
#define MHZ_100    RXS_MPM_CFGSIG0_REFCLK_SELECT_100MHZ
#define MHZ_156    RXS_MPM_CFGSIG0_REFCLK_SELECT_156P25MHZ

static clk_pd_tests_t clk_pd_pass[] = {
	{ 42, LO_LAT | MHZ_100, 1001},
	{ 42, LO_LAT | MHZ_156, 1000},
	{ 38, LO_PWR_12G | MHZ_156, 998},
	{ 37, LO_PWR_12G | MHZ_100, 992},
	{ 31, LO_PWR_10G | MHZ_156, 992},
	{ 31, LO_PWR_10G | MHZ_100, 992}
};

static void rxs_rio_pc_clk_pd_success_test(void **state)
{
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;
	const int num_tests = sizeof(clk_pd_pass) / sizeof(clk_pd_pass[0]);
	uint32_t srv_pd;
	uint32_t i;

	// On real hardware, it is disasterous to mess with clocking
	// parameters, so just check that the current setup passes

	if (l_st->real_hw) {
		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_clk_pd(&mock_dev_info, &srv_pd));

		return;
	}

	for (i = 0; i < num_tests; i++) {
		if (DEBUG_PRINTF) {
			printf("\ni = %d\n", i);
		}
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info, RXS_PRESCALAR_SRV_CLK,
			clk_pd_pass[i].ps));
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info, RXS_MPM_CFGSIG0,
			clk_pd_pass[i].cfgsig0));
		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_clk_pd(&mock_dev_info, &srv_pd));
		assert_int_equal(srv_pd, clk_pd_pass[i].clk_pd);
	}
}

static clk_pd_tests_t clk_pd_fail[] = {
	{ 0x1FF, 0, 0},
	{ 0, 0, 0},
	{ 42, LO_RSVD | MHZ_156, 0},
	{ 42, LO_RSVD | MHZ_100, 0},
	{ 41, LO_LAT | MHZ_100, 1000},
	{ 41, LO_LAT | MHZ_156, 1001},
	{ 37, LO_PWR_12G | MHZ_156, 998},
	{ 36, LO_PWR_12G | MHZ_100, 992},
	{ 30, LO_PWR_10G | MHZ_156, 992},
	{ 32, LO_PWR_10G | MHZ_100, 992}
};

static void rxs_rio_pc_clk_pd_fail_test(void **state)
{
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;
	const int num_tests = sizeof(clk_pd_fail) / sizeof(clk_pd_fail[0]);
	uint32_t srv_pd;
	uint32_t i;

	// On real hardware, it is disasterous to mess with clocking
	// parameters, so just check that the current setup passes

	if (l_st->real_hw) {
		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_clk_pd(&mock_dev_info, &srv_pd));

		return;
	}

	for (i = 0; i < num_tests; i++) {
		if (DEBUG_PRINTF) {
			printf("\ni = %d\n", i);
		}
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info, RXS_PRESCALAR_SRV_CLK,
			clk_pd_fail[i].ps));
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info, RXS_MPM_CFGSIG0,
			clk_pd_fail[i].cfgsig0));
		assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_pc_clk_pd(&mock_dev_info, &srv_pd));
		assert_int_equal(srv_pd, 0);
	}
}

static void compare_pc(rio_pc_one_port_config_t *pc,
			rio_pc_one_port_config_t *chk_pc)
{
	uint32_t lane;

	assert_int_equal(pc->pnum, chk_pc->pnum);
	assert_int_equal(pc->port_available, chk_pc->port_available);
	assert_int_equal(pc->powered_up, chk_pc->powered_up);
	assert_int_equal(pc->pw, chk_pc->pw);
	assert_int_equal(pc->ls, chk_pc->ls);
	assert_int_equal(pc->fc, chk_pc->fc);
	assert_int_equal(pc->iseq, chk_pc->iseq);
	assert_int_equal(pc->xmitter_disable, chk_pc->xmitter_disable);
	assert_int_equal(pc->port_lockout, chk_pc->port_lockout);
	assert_int_equal(pc->nmtc_xfer_enable, chk_pc->nmtc_xfer_enable);
	assert_int_equal(pc->tx_lswap, chk_pc->tx_lswap);
	assert_int_equal(pc->rx_lswap, chk_pc->rx_lswap);
	for (lane = 0; lane < RIO_MAX_PORT_LANES; lane++) {
		assert_int_equal(pc->tx_linvert[lane],
				chk_pc->tx_linvert[lane]);
		assert_int_equal(pc->rx_linvert[lane],
				chk_pc->rx_linvert[lane]);
	}
}
static void rxs_rio_pc_get_config_success(void **state)
{
	rio_pc_get_config_in_t pc_in;
	rio_pc_get_config_out_t pc_out;
	rio_pc_one_port_config_t curr_pc;

	uint32_t err_stat = RXS_SPX_ERR_STAT_DFLT;
	uint32_t plm_ctl = RXS_PLM_SPX_IMP_SPEC_CTL_DFLT;
	uint32_t pwdn = RXS_PLM_SPX_PWDN_CTL_DFLT;
	uint32_t pol = RXS_PLM_SPX_POL_CTL_DFLT;
	uint32_t lrto = 0;

	rio_port_t port;
	uint32_t lane;
	uint32_t l_vec;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Default register values have all ports unavailable
	curr_pc.port_available = false;
	curr_pc.powered_up = false;
	curr_pc.pw = rio_pc_pw_last;
	curr_pc.ls = rio_pc_ls_last;
	curr_pc.fc = rio_pc_fc_last;
	curr_pc.iseq = rio_pc_is_last;
	curr_pc.xmitter_disable = false;
	curr_pc.port_lockout = false;
	curr_pc.nmtc_xfer_enable = false;
	curr_pc.tx_lswap = rio_lswap_none;
	curr_pc.rx_lswap = rio_lswap_none;
	for (lane = 0; lane < RIO_MAX_PORT_LANES; lane++) {
		curr_pc.tx_linvert[lane] = false;
		curr_pc.rx_linvert[lane] = false;
	}

	// Check that the default register values line up with the above.
	pc_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
	assert_int_equal(NUM_RXS_PORTS(&mock_dev_info), pc_out.num_ports);
	assert_int_equal(lrto, pc_out.lrto);
	assert_int_equal(0, pc_out.log_rto);

	for (port = 0; port < pc_out.num_ports; port++) {
		curr_pc.pnum = port;
		compare_pc(&pc_out.pc[port], &curr_pc);
	}

	if (DEBUG_PRINTF) {
		printf("\nconfig 1\n");
	}
	// Make port available and powered down
	// Doesn't work this way on real hardware
	err_stat &= ~RXS_SPX_ERR_STAT_PORT_UNAVL;
	pwdn |= RXS_PLM_SPX_PWDN_CTL_PWDN_PORT;
	curr_pc.port_available = true;

	set_all_port_config(cfg_avl_pwdn, NO_TTL, NO_FILT, RIO_ALL_PORTS);

	// Also update LRTO value
	pc_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
	assert_int_equal(NUM_RXS_PORTS(&mock_dev_info), pc_out.num_ports);
	assert_int_equal(lrto, pc_out.lrto);
	assert_int_equal(0, pc_out.log_rto);

	for (port = 0; port < pc_out.num_ports; port++) {
		curr_pc.pnum = port;
		compare_pc(&pc_out.pc[port], &curr_pc);
	}

	if (DEBUG_PRINTF) {
		printf("\nconfig 2\n");
	}

	// Power up the port, with transmitter disabled
	// Doesn't work this way on real hardware
	curr_pc.powered_up = true;
	curr_pc.xmitter_disable = true;
	curr_pc.pw = rio_pc_pw_4x;
	curr_pc.ls = rio_pc_ls_5p0;
	curr_pc.iseq = rio_pc_is_one;
	curr_pc.fc = rio_pc_fc_rx;

	set_all_port_config(cfg_pwup_txdis, NO_TTL, NO_FILT, RIO_ALL_PORTS);

	pc_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
	assert_int_equal(NUM_RXS_PORTS(&mock_dev_info), pc_out.num_ports);
	assert_int_equal(lrto, pc_out.lrto);
	assert_int_equal(0, pc_out.log_rto);

	for (port = 0; port < pc_out.num_ports; port++) {
		curr_pc.pnum = port;
		if (DEBUG_PRINTF) {
			printf("\n	Port %d\n", port);
		}
		if (port & 1) {
			curr_pc.pw = rio_pc_pw_2x;
		} else {
			curr_pc.pw = rio_pc_pw_4x;
		}
		compare_pc(&pc_out.pc[port], &curr_pc);
	}

	if (DEBUG_PRINTF) {
		printf("\nconfig 3\n");
	}
	// Enable transmitter, set lockout, disable non-maintenance
	// Doesn't work this way on real hardware
	curr_pc.xmitter_disable = false;
	curr_pc.port_lockout = true;

	set_all_port_config(cfg_lp_lkout, NO_TTL, NO_FILT, RIO_ALL_PORTS);

	pc_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
	assert_int_equal(NUM_RXS_PORTS(&mock_dev_info), pc_out.num_ports);
	assert_int_equal(lrto, pc_out.lrto);
	assert_int_equal(0, pc_out.log_rto);

	for (port = 0; port < pc_out.num_ports; port++) {
		curr_pc.pnum = port;
		if (port & 1) {
			curr_pc.pw = rio_pc_pw_2x;
		} else {
			curr_pc.pw = rio_pc_pw_4x;
		}
		compare_pc(&pc_out.pc[port], &curr_pc);
	}

	if (DEBUG_PRINTF) {
		printf("\nconfig 4\n");
	}
	// Clear lockout, enable non-maintenance
	// Doesn't work this way on real hardware
	curr_pc.port_lockout = false;
	curr_pc.nmtc_xfer_enable = true;

	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		if (port & 1) {
			set_all_port_config(cfg_perfect_2x, NO_TTL, NO_FILT, port);
		} else {
			set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, port);
		}
	}

	pc_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
	assert_int_equal(NUM_RXS_PORTS(&mock_dev_info), pc_out.num_ports);
	assert_int_equal(lrto, pc_out.lrto);
	assert_int_equal(0, pc_out.log_rto);

	for (port = 0; port < pc_out.num_ports; port++) {
		curr_pc.pnum = port;
		if (port & 1) {
			curr_pc.pw = rio_pc_pw_2x;
		} else {
			curr_pc.pw = rio_pc_pw_4x;
		}
		compare_pc(&pc_out.pc[port], &curr_pc);
	}

	if (DEBUG_PRINTF) {
		printf("\nconfig 5\n");
	}
	// Try various combinations of Tx/Rx swap, and lane inversion
	// l_vec is a bit mask of conditions for tx & rx swap,
	// and various lane inversions...
	for (l_vec = 0; l_vec < 1 << RIO_MAX_PORT_LANES; l_vec++) {
		plm_ctl &= ~RXS_PLM_SPX_IMP_SPEC_CTL_SWAP_RX;
		plm_ctl &= ~RXS_PLM_SPX_IMP_SPEC_CTL_SWAP_TX;
		curr_pc.tx_lswap = rio_lswap_none;
		curr_pc.rx_lswap = rio_lswap_none;

		plm_ctl |= (l_vec & 0x3) << 16;
		curr_pc.rx_lswap = lswap(l_vec & 0x3);
		plm_ctl |= (l_vec & 0xC) << 16;
		curr_pc.tx_lswap =lswap((l_vec & 0xC) >> 2);
		pol = l_vec;
		pol |= ~(l_vec << 16) & RXS_PLM_SPX_POL_CTL_TX_ALL_POL;
		for (lane = 0; lane < RIO_MAX_PORT_LANES; lane++) {
			curr_pc.rx_linvert[lane] = l_vec & (1 << lane);
			curr_pc.tx_linvert[lane] = !(l_vec & (1 << lane));
		}
		for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
			assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info,
					RXS_PLM_SPX_IMP_SPEC_CTL(port),
					plm_ctl));
			assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info,
					RXS_PLM_SPX_POL_CTL(port), pol));
		}
		pc_in.ptl.num_ports = RIO_ALL_PORTS;
		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_config(&mock_dev_info, &pc_in, &pc_out));
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
		assert_int_equal(NUM_RXS_PORTS(&mock_dev_info),
					pc_out.num_ports);
		assert_int_equal(lrto, pc_out.lrto);
		assert_int_equal(0, pc_out.log_rto);

		for (port = 0; port < pc_out.num_ports; port++) {
			curr_pc.pnum = port;
			if (port & 1) {
				curr_pc.pw = rio_pc_pw_2x;
			} else {
				curr_pc.pw = rio_pc_pw_4x;
			}
			compare_pc(&pc_out.pc[port], &curr_pc);
		}
	}

	(void)state;
}

static void rxs_rio_pc_get_config_bad_parms(void **state)
{
	rio_pc_get_config_in_t pc_in;
	rio_pc_get_config_out_t pc_out;

	// Bad number of ports...
	pc_in.ptl.num_ports = NUM_RXS_PORTS(&mock_dev_info) + 1;
	pc_out.imp_rc = RIO_SUCCESS;
	pc_out.lrto = 0xFFFF;
	pc_out.log_rto = 0xFFFF;
	pc_out.num_ports = NUM_RXS_PORTS(&mock_dev_info) + 1;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_pc_get_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_not_equal(RIO_SUCCESS, pc_out.imp_rc);
	assert_int_equal(0, pc_out.lrto);
	assert_int_equal(0, pc_out.log_rto);
	assert_int_equal(0, pc_out.num_ports);

	// Bad port number
	pc_in.ptl.num_ports = 1;
	pc_in.ptl.pnums[0] = NUM_RXS_PORTS(&mock_dev_info);
	pc_out.imp_rc = RIO_SUCCESS;
	pc_out.lrto = 0xFFFF;
	pc_out.log_rto = 0xFFFF;
	pc_out.num_ports = NUM_RXS_PORTS(&mock_dev_info) + 1;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_pc_get_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_not_equal(RIO_SUCCESS, pc_out.imp_rc);
	assert_int_equal(0, pc_out.lrto);
	assert_int_equal(0, pc_out.log_rto);
	assert_int_equal(0, pc_out.num_ports);
	(void)state;
}

static void adjust_ps_for_port(rio_pc_one_port_status_t *curr_ps,
				rio_port_t port)
{
	curr_ps->pnum = port;

	if (port & 1) {
		curr_ps->pw = rio_pc_pw_2x;
		curr_ps->num_lanes = 2;
		curr_ps->first_lane = ((port / 2) * 4) + 2;
	} else {
		curr_ps->pw = rio_pc_pw_4x;
		curr_ps->num_lanes = 4;
		curr_ps->first_lane = ((port / 2) * 4);
	}
}

static void compare_ps(rio_pc_one_port_status_t *ps,
			rio_pc_one_port_status_t *chk_ps)
{
	assert_int_equal(ps->pnum, chk_ps->pnum);
	assert_int_equal(ps->pw, chk_ps->pw);
	assert_int_equal(ps->fc, chk_ps->fc);
	assert_int_equal(ps->iseq, chk_ps->iseq);
	assert_int_equal(ps->port_error, chk_ps->port_error);
	assert_int_equal(ps->input_stopped, chk_ps->input_stopped);
	assert_int_equal(ps->output_stopped, chk_ps->output_stopped);
	assert_int_equal(ps->num_lanes, chk_ps->num_lanes);
	assert_int_equal(ps->first_lane, chk_ps->first_lane);
}

static void rxs_rio_pc_get_status_success(void **state)
{
	rio_pc_get_status_in_t ps_in;
	rio_pc_get_status_out_t ps_out;
	rio_pc_one_port_status_t curr_ps;

	rio_port_t port;
	uint32_t pw;
	uint32_t lane_adj = 0;

	uint32_t err_stat = RXS_SPX_ERR_STAT_DFLT;
	uint32_t ctl = RXS_SPX_CTL_DFLT;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Default register values have all ports uninitialized
	curr_ps.port_ok = false;
	curr_ps.pw = rio_pc_pw_last;
	curr_ps.fc = rio_pc_fc_last;
	curr_ps.iseq = rio_pc_is_last;
	curr_ps.port_error = false;
	curr_ps.input_stopped = false;
	curr_ps.output_stopped = false;
	curr_ps.num_lanes = 0;
	curr_ps.first_lane = 0;

	// Check that the default register values line up with the above.
	ps_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_status(&mock_dev_info, &ps_in, &ps_out));
	assert_int_equal(RIO_SUCCESS, ps_out.imp_rc);
	assert_int_equal(NUM_RXS_PORTS(&mock_dev_info), ps_out.num_ports);

	for (port = 0; port < ps_out.num_ports; port++) {
		curr_ps.pnum = port;
		compare_ps(&ps_out.ps[port], &curr_ps);
	}

	if (DEBUG_PRINTF) {
		printf("\nstatus 1\n");
	}
	// Make link uninitialized
	// Doesn't work this way on real hardware
	// Must read/modify/write err_stat to preserve odd & even
	// port initialization differences
	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		set_all_port_config(cfg_txen_no_lp, NO_TTL, NO_FILT, port);
	}

	ps_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_status(&mock_dev_info, &ps_in, &ps_out));
	assert_int_equal(RIO_SUCCESS, ps_out.imp_rc);
	assert_int_equal(NUM_RXS_PORTS(&mock_dev_info), ps_out.num_ports);

	for (port = 0; port < ps_out.num_ports; port++) {
		curr_ps.pnum = port;
		compare_ps(&ps_out.ps[port], &curr_ps);
	}

	if (DEBUG_PRINTF) {
		printf("\nstatus 2\n");
	}
	// Make port initialized, with a port error
	// Doesn't work this way on real hardware
	curr_ps.port_ok = true;
	curr_ps.port_error = true;
	curr_ps.fc = rio_pc_fc_rx;
	curr_ps.iseq = rio_pc_is_one;

	set_all_port_config(cfg_txen_lp_perr, NO_TTL, NO_FILT, RIO_ALL_PORTS);

	ps_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_status(&mock_dev_info, &ps_in, &ps_out));
	assert_int_equal(RIO_SUCCESS, ps_out.imp_rc);
	assert_int_equal(NUM_RXS_PORTS(&mock_dev_info), ps_out.num_ports);

	for (port = 0; port < ps_out.num_ports; port++) {
		adjust_ps_for_port(&curr_ps, port);
		compare_ps(&ps_out.ps[port], &curr_ps);
	}

	if (DEBUG_PRINTF) {
		printf("\nstatus 3\n");
	}
	// Clear port error, add input error
	// Doesn't work this way on real hardware
	curr_ps.port_error = false;
	curr_ps.input_stopped = true;

	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
				RXS_SPX_ERR_STAT(port), &err_stat));
		err_stat &= ~RXS_SPX_ERR_STAT_PORT_ERR;
		err_stat |= RXS_SPX_ERR_STAT_INPUT_ERR_STOP;
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info,
				RXS_SPX_ERR_STAT(port), err_stat));
	}

	ps_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_status(&mock_dev_info, &ps_in, &ps_out));
	assert_int_equal(RIO_SUCCESS, ps_out.imp_rc);
	assert_int_equal(NUM_RXS_PORTS(&mock_dev_info), ps_out.num_ports);

	for (port = 0; port < ps_out.num_ports; port++) {
		adjust_ps_for_port(&curr_ps, port);
		compare_ps(&ps_out.ps[port], &curr_ps);
	}

	if (DEBUG_PRINTF) {
		printf("\nstatus 4\n");
	}
	// Clear input error, add output error
	// Doesn't work this way on real hardware
	curr_ps.input_stopped = false;
	curr_ps.output_stopped = true;

	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
				RXS_SPX_ERR_STAT(port), &err_stat));
		err_stat &= ~RXS_SPX_ERR_STAT_INPUT_ERR_STOP;
		err_stat |= RXS_SPX_ERR_STAT_OUTPUT_ERR_STOP;
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info,
				RXS_SPX_ERR_STAT(port), err_stat));
	}

	ps_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_status(&mock_dev_info, &ps_in, &ps_out));
	assert_int_equal(RIO_SUCCESS, ps_out.imp_rc);
	assert_int_equal(NUM_RXS_PORTS(&mock_dev_info), ps_out.num_ports);

	for (port = 0; port < ps_out.num_ports; port++) {
		adjust_ps_for_port(&curr_ps, port);
		compare_ps(&ps_out.ps[port], &curr_ps);
	}

	if (DEBUG_PRINTF) {
		printf("\nstatus 5\n");
	}
	// Add input error and port fail (port error)
	// Doesn't work this way on real hardware
	curr_ps.input_stopped = true;
	curr_ps.output_stopped = true;
	curr_ps.port_error = true;

	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
				RXS_SPX_ERR_STAT(port), &err_stat));
		err_stat |= RXS_SPX_ERR_STAT_INPUT_ERR_STOP;
		err_stat |= RXS_SPX_ERR_STAT_OUTPUT_ERR_STOP;
		err_stat |= RXS_SPX_ERR_STAT_OUTPUT_FAIL;
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info,
				RXS_SPX_ERR_STAT(port), err_stat));
		// Must read/modify/write ctl to avoid complexity with which
		// port widths are enabled...
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
				RXS_SPX_CTL(port), &ctl));
		ctl |= RXS_SPX_CTL_STOP_FAIL_EN;
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info,
				RXS_SPX_CTL(port), ctl));
	}

	ps_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_status(&mock_dev_info, &ps_in, &ps_out));
	assert_int_equal(RIO_SUCCESS, ps_out.imp_rc);
	assert_int_equal(NUM_RXS_PORTS(&mock_dev_info), ps_out.num_ports);

	for (port = 0; port < ps_out.num_ports; port++) {
		adjust_ps_for_port(&curr_ps, port);
		compare_ps(&ps_out.ps[port], &curr_ps);
	}

	if (DEBUG_PRINTF) {
		printf("\nstatus 6\n");
	}
	// Remove input/output error and port fail (port error)
	// Doesn't work this way on real hardware
	curr_ps.input_stopped = false;
	curr_ps.output_stopped = false;
	curr_ps.port_error = false;

	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
				RXS_SPX_ERR_STAT(port), &err_stat));
		err_stat &= ~RXS_SPX_ERR_STAT_INPUT_ERR_STOP;
		err_stat &= ~RXS_SPX_ERR_STAT_OUTPUT_ERR_STOP;
		err_stat &= ~RXS_SPX_ERR_STAT_OUTPUT_FAIL;
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info,
				RXS_SPX_ERR_STAT(port), err_stat));
		// Must read/modify/write ctl to avoid complexity with which
		// port widths are enabled...
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
				RXS_SPX_CTL(port), &ctl));
		ctl |= RXS_SPX_CTL_STOP_FAIL_EN;
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info,
				RXS_SPX_CTL(port), ctl));
	}

	ps_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_status(&mock_dev_info, &ps_in, &ps_out));
	assert_int_equal(RIO_SUCCESS, ps_out.imp_rc);
	assert_int_equal(NUM_RXS_PORTS(&mock_dev_info), ps_out.num_ports);

	for (port = 0; port < ps_out.num_ports; port++) {
		adjust_ps_for_port(&curr_ps, port);
		compare_ps(&ps_out.ps[port], &curr_ps);
	}

	if (DEBUG_PRINTF) {
		printf("\nstatus 7\n");
	}
	// Try various link initialization values for the even (4x) ports

	for (pw = 0; pw < 6; pw++) {
		for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port += 2)
		{
			assert_int_equal(RIO_SUCCESS,
				DARRegRead(&mock_dev_info,
					RXS_SPX_CTL(port), &ctl));

			// Ensure no port width overrides are active.
			ctl &= ~RXS_SPX_CTL_OVER_PWIDTH;
			switch (pw) {
			case 0:
				lane_adj = 0;
				curr_ps.pw = rio_pc_pw_2x;
				curr_ps.num_lanes = 2;
				ctl |= RIO_SPX_CTL_PTW_OVER_2X_NO_4X;
				break;
			case 1:
				lane_adj = 2;
				curr_ps.pw = rio_pc_pw_1x_l2;
				curr_ps.num_lanes = 1;
				ctl |= RIO_SPX_CTL_PTW_OVER_1X_LR;
				break;
			case 2:
				lane_adj = 0;
				curr_ps.pw = rio_pc_pw_1x_l0;
				curr_ps.num_lanes = 1;
				ctl |= RIO_SPX_CTL_PTW_OVER_1X_L0;
				break;
			case 3:
				lane_adj = 0;
				curr_ps.pw = rio_pc_pw_2x;
				curr_ps.num_lanes = 2;
				ctl &= ~RIO_SPX_CTL_PTW_MAX_4X;
				break;
			case 4:
				lane_adj = 1;
				curr_ps.pw = rio_pc_pw_1x_l1;
				curr_ps.num_lanes = 1;
				ctl &= ~RIO_SPX_CTL_PTW_MAX_4X;
				ctl |= RIO_SPX_CTL_PTW_OVER_1X_LR;
				break;
			case 5:
				lane_adj = 0;
				curr_ps.pw = rio_pc_pw_1x_l0;
				curr_ps.num_lanes = 1;
				ctl &= ~RIO_SPX_CTL_PTW_MAX_2X;
				break;
			default:
				assert_false(true);
			}
			assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info,
					RXS_SPX_CTL(port), ctl));
		}
		ps_in.ptl.num_ports = RIO_ALL_PORTS;
		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_status(&mock_dev_info, &ps_in, &ps_out));
		assert_int_equal(RIO_SUCCESS, ps_out.imp_rc);
		assert_int_equal(NUM_RXS_PORTS(&mock_dev_info),
							ps_out.num_ports);

		// Only check even ports
		for (port = 0; port < ps_out.num_ports; port += 2) {
			curr_ps.pnum = port;
			curr_ps.first_lane = (port * 4) + lane_adj;
			compare_ps(&ps_out.ps[port], &curr_ps);
		}
	}
	// Try various link initialization values for the odd (2x) ports

	if (DEBUG_PRINTF) {
		printf("\nstatus 8\n");
	}
	for (pw = 0; pw < 2; pw++) {
		for (port = 1; port < NUM_RXS_PORTS(&mock_dev_info); port += 2)
		{
			assert_int_equal(RIO_SUCCESS,
				DARRegRead(&mock_dev_info,
					RXS_SPX_CTL(port), &ctl));
			ctl &= ~RXS_SPX_CTL_OVER_PWIDTH;
			ctl &= ~RXS_SPX_CTL_INIT_PWIDTH;
			switch (pw) {
			case 0:
				curr_ps.pw = rio_pc_pw_1x_l1;
				curr_ps.num_lanes = 1;
				ctl |= RIO_SPX_CTL_PTW_MAX_2X;
				ctl &= ~RIO_SPX_CTL_PTW_MAX_4X;
				ctl |= RIO_SPX_CTL_PTW_OVER_1X_LR;
				break;
			case 1:
				curr_ps.pw = rio_pc_pw_1x_l0;
				curr_ps.num_lanes = 1;
				ctl |= RIO_SPX_CTL_PTW_MAX_2X;
				ctl &= ~RIO_SPX_CTL_PTW_MAX_4X;
				ctl |= RIO_SPX_CTL_PTW_OVER_1X_L0;
				break;
			default:
				assert_false(true);
			}
			assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info,
					RXS_SPX_CTL(port), ctl));
		}
		ps_in.ptl.num_ports = RIO_ALL_PORTS;
		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_get_status(&mock_dev_info, &ps_in, &ps_out));
		assert_int_equal(RIO_SUCCESS, ps_out.imp_rc);
		assert_int_equal(NUM_RXS_PORTS(&mock_dev_info),
							ps_out.num_ports);

		// Only check odd ports
		for (port = 1; port < ps_out.num_ports; port += 2) {
			curr_ps.pnum = port;
			compare_ps(&ps_out.ps[port], &curr_ps);
		}
	}

	(void)state;
}

static void rxs_rio_pc_get_status_bad_parms(void **state)
{
	rio_pc_get_status_in_t ps_in;
	rio_pc_get_status_out_t ps_out;

	// Bad number of ports...
	ps_in.ptl.num_ports = NUM_RXS_PORTS(&mock_dev_info) + 1;
	ps_out.imp_rc = RIO_SUCCESS;
	ps_out.num_ports = NUM_RXS_PORTS(&mock_dev_info) + 1;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_pc_get_status(&mock_dev_info, &ps_in, &ps_out));
	assert_int_not_equal(RIO_SUCCESS, ps_out.imp_rc);
	assert_int_equal(0, ps_out.num_ports);

	// Bad port number
	ps_in.ptl.num_ports = 1;
	ps_in.ptl.pnums[0] = NUM_RXS_PORTS(&mock_dev_info);
	ps_out.imp_rc = RIO_SUCCESS;
	ps_out.num_ports = NUM_RXS_PORTS(&mock_dev_info) + 1;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_pc_get_status(&mock_dev_info, &ps_in, &ps_out));
	assert_int_not_equal(RIO_SUCCESS, ps_out.imp_rc);
	assert_int_equal(0, ps_out.num_ports);
	(void)state;
}

static void rxs_rio_pc_reset_port_bad_parms(void **state)
{
	rio_pc_reset_port_in_t pc_in;
	rio_pc_reset_port_out_t pc_out;

	// Bad port number...
	pc_in.port_num = NUM_RXS_PORTS(&mock_dev_info);
	pc_in.oob_reg_acc = true;
	pc_in.reg_acc_port = 0;
	pc_in.reset_lp = true;
	pc_in.preserve_config = true;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_pc_reset_port(&mock_dev_info, &pc_in, &pc_out));
	assert_int_not_equal(RIO_SUCCESS, pc_out.imp_rc);

	(void)state;
}

// Test that the register access port is never reset.
static void rxs_rio_pc_reset_port_exclude_reg_acc(void **state)
{
	rio_pc_reset_port_in_t pc_in;
	rio_pc_reset_port_out_t pc_out;
	uint32_t chk_data;
	uint32_t all_1 = 0xFFFFFFFF;
	rio_port_t port, t_pt;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Test on one port at a time.
	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		// Power up the port so it can be reset
		set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, port);

		// Set some status on the register access port
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info,
						RXS_SPX_ERR_DET(port), all_1));
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_ERR_DET(port),
								&chk_data));
		assert_int_not_equal(0, chk_data);

		// Clear link management response status
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_LM_RESP(port),
								&chk_data));

		// Try to reset the register access port
		pc_in.port_num = port;
		pc_in.oob_reg_acc = false;
		pc_in.reg_acc_port = port;
		pc_in.reset_lp = true;
		pc_in.preserve_config = true;
		pc_out.imp_rc = RIO_SUCCESS;

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_reset_port(&mock_dev_info, &pc_in, &pc_out));
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

		// Check that the port was not reset, by confirming that the
		// status bits set remain set.
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_ERR_DET(port),
								&chk_data));
		assert_int_not_equal(0, chk_data);

		// Check that link partner was not reset
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_LM_RESP(port),
								&chk_data));
		assert_int_equal(0, chk_data);
	}

	// All ports now have err detect bits set.  Try resetting all ports
	// and verify that the register access port is preserved.

	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		// Try to reset the register access port
		pc_in.port_num = RIO_ALL_PORTS;
		pc_in.oob_reg_acc = false;
		pc_in.reg_acc_port = port;
		pc_in.reset_lp = true;
		pc_in.preserve_config = true;
		pc_out.imp_rc = RIO_SUCCESS;

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_reset_port(&mock_dev_info, &pc_in, &pc_out));
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

		for (t_pt = 0; t_pt < NUM_RXS_PORTS(&mock_dev_info); t_pt++) {
			assert_int_equal(RIO_SUCCESS,
				DARRegRead(&mock_dev_info,
					RXS_SPX_ERR_DET(t_pt), &chk_data));
			if (t_pt == port) {
				// Verify status is preserved on register
				// access port
				assert_int_not_equal(0, chk_data);

				// Check that a reset was not sent to
				// the link partner.
				assert_int_equal(RIO_SUCCESS,
					DARRegRead(&mock_dev_info,
						RXS_SPX_LM_RESP(t_pt),
						&chk_data));
				assert_int_equal(0, chk_data);
				continue;
			}
			// Verify status is cleared on all other ports, and that
			// a reset was sent to the link partner,
			// and then make the status non-zero in preparation for
			// the next test loop.
			assert_int_equal(0, chk_data);
			assert_int_equal(RIO_SUCCESS,
				DARRegRead(&mock_dev_info,
					RXS_SPX_LM_RESP(t_pt), &chk_data));
			assert_int_equal(RIO_SPX_LM_RESP_VLD, chk_data);

			assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info,
					RXS_SPX_ERR_DET(t_pt), all_1));
		}
	}

	// All ports still have the status bits set.
	// Try resetting all ports and verify that if oob register access is
	// used, that all ports are reset.
	pc_in.port_num = RIO_ALL_PORTS;
	pc_in.oob_reg_acc = true;
	pc_in.reg_acc_port = 0;
	pc_in.reset_lp = false;
	pc_in.preserve_config = true;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_pc_reset_port(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
				RXS_SPX_ERR_DET(port), &chk_data));
		// Verify status is cleared on all ports,
		// and that a reset was not sent to the link partner.
		assert_int_equal(0, chk_data);
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
					RXS_SPX_LM_RESP(port), &chk_data));
		assert_int_equal(0, chk_data);
	}

	(void)state;
}

static void rxs_rio_pc_reset_link_partner_bad_parms(void **state)
{
	rio_pc_reset_link_partner_in_t pc_in;
	rio_pc_reset_link_partner_out_t pc_out;

	// Bad port number...
	pc_in.port_num = NUM_RXS_PORTS(&mock_dev_info);
	pc_in.resync_ackids = true;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_reset_link_partner(&mock_dev_info, &pc_in, &pc_out));
	assert_int_not_equal(RIO_SUCCESS, pc_out.imp_rc);

	(void)state;
}

// Test that the link partner is reset, and the local port is reset,
// according to the input parameters.
static void rxs_rio_pc_reset_link_partner_success(void **state)
{
	rio_pc_reset_link_partner_in_t pc_in;
	rio_pc_reset_link_partner_out_t pc_out;
	uint32_t chk_data;
	uint32_t all_1 = 0xFFFFFFFF;
	rio_port_t port;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Test on one port at a time.
	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		// Power up the port so it can be reset
		set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, port);

		// Set some status on the port under test
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info,
						RXS_SPX_ERR_DET(port), all_1));
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_ERR_DET(port),
								&chk_data));
		assert_int_not_equal(0, chk_data);

		// Clear link management response status
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_LM_RESP(port),
								&chk_data));
		// Reset the link partner and resync_ackids
		pc_in.port_num = port;
		pc_in.resync_ackids = true;
		pc_out.imp_rc = RIO_SUCCESS;

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_reset_link_partner(
					&mock_dev_info, &pc_in, &pc_out));
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

		// Check that the port was reset,
		// by confirming that the status bits are cleared.
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_ERR_DET(port),
								&chk_data));
		assert_int_equal(0, chk_data);

		// Check that link partner was reset
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_LM_RESP(port),
								&chk_data));
		assert_int_equal(RIO_SPX_LM_RESP_VLD, chk_data);
	}

	// Try again, verifying that the local port was no reset.

	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		// Set some status on the port under test
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info,
						RXS_SPX_ERR_DET(port), all_1));
		pc_in.port_num = port;
		pc_in.resync_ackids = false;
		pc_out.imp_rc = RIO_SUCCESS;

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_reset_link_partner(
					&mock_dev_info, &pc_in, &pc_out));
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

		// Verify the local port was not reset.
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
					RXS_SPX_ERR_DET(port), &chk_data));
		assert_int_not_equal(0, chk_data);

		// Check that a reset was sent to the link partner.
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
					RXS_SPX_LM_RESP(port), &chk_data));
		assert_int_equal(RIO_SPX_LM_RESP_VLD, chk_data);
	}
	(void)state;
}

static void rxs_rio_pc_clr_errs_bad_parms(void **state)
{
	rio_pc_clr_errs_in_t pc_in;
	rio_pc_clr_errs_out_t pc_out;

	// Bad port number...
	memset(&pc_in, 0, sizeof(pc_in));
	pc_in.port_num = NUM_RXS_PORTS(&mock_dev_info);
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_clr_errs(&mock_dev_info, &pc_in, &pc_out));
	assert_int_not_equal(RIO_SUCCESS, pc_out.imp_rc);

	// Clear link partner errors without link partner information
	memset(&pc_in, 0, sizeof(pc_in));
	pc_in.port_num = 0;
	pc_in.clr_lp_port_err = true;
	pc_in.lp_dev_info = NULL;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_clr_errs(&mock_dev_info, &pc_in, &pc_out));
	assert_int_not_equal(RIO_SUCCESS, pc_out.imp_rc);

	// Clear link partner errors without link partner port hints
	memset(&pc_in, 0, sizeof(pc_in));
	pc_in.port_num = 0;
	pc_in.clr_lp_port_err = true;
	pc_in.lp_dev_info = &mock_dev_info;
	pc_in.num_lp_ports = 0;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_clr_errs(&mock_dev_info, &pc_in, &pc_out));
	assert_int_not_equal(RIO_SUCCESS, pc_out.imp_rc);

	(void)state;
}

// Test that the local port is reset, and no ackID synchronization is performed
static void rxs_rio_pc_clr_errs_success(void **state)
{
	rio_pc_clr_errs_in_t pc_in;
	rio_pc_clr_errs_out_t pc_out;
	uint32_t chk_data;
	uint32_t all_1 = 0xFFFFFFFF;
	rio_port_t port;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Test on one port at a time.
	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		// Power up the port
		set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, port);

		// Set some status on the port under test
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info,
						RXS_SPX_ERR_DET(port), all_1));
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_ERR_DET(port),
								&chk_data));
		assert_int_not_equal(0, chk_data);

		// Clear link management response status
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_LM_RESP(port),
								&chk_data));
		// Reset the this port, do not clear link partner errors
		pc_in.port_num = port;
		pc_in.clr_lp_port_err = false;
		pc_out.imp_rc = RIO_SUCCESS;

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_clr_errs(
					&mock_dev_info, &pc_in, &pc_out));
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

		// Check that the port was reset,
		// by confirming that the status bits are cleared.
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_ERR_DET(port),
								&chk_data));
		assert_int_equal(0, chk_data);

		// Check that link partner was not touched...
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_LM_RESP(port),
								&chk_data));
		assert_int_equal(0, chk_data);
	}

	(void)state;
}

// Test that the local port is reset, and ackID synchronization is performed
static void rxs_rio_pc_clr_errs_resync_ackids(void **state)
{
	rio_pc_clr_errs_in_t pc_in;
	rio_pc_clr_errs_out_t pc_out;
	uint32_t chk_data;
	uint32_t all_1 = 0xFFFFFFFF;
	rio_port_t port;
	uint32_t rc;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Test on one port at a time.
	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		if (DEBUG_PRINTF) {
			printf("\nport = %d\n", port);
		}

		// Power up the port
		set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, port);

		// Set some status on the port under test
		assert_int_equal(RIO_SUCCESS,
			DARRegWrite(&mock_dev_info,
						RXS_SPX_ERR_DET(port), all_1));
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_ERR_DET(port),
								&chk_data));
		assert_int_not_equal(0, chk_data);

		// Clear link management response status
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_LM_RESP(port),
								&chk_data));
		// Reset the this port, clear link partner errors
		pc_in.port_num = port;
		pc_in.clr_lp_port_err = true;
		pc_in.lp_dev_info = &mock_lp_dev_info;
		pc_in.num_lp_ports = 1;
		pc_in.lp_port_list[0] = 0;
		pc_out.imp_rc = RIO_SUCCESS;

		rc = rxs_rio_pc_clr_errs(&mock_dev_info, &pc_in, &pc_out);
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
		assert_int_equal(RIO_SUCCESS, rc);

		// Check that the port was reset,
		// by confirming that the status bits are cleared.
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_ERR_DET(port),
								&chk_data));
		assert_int_equal(0, chk_data);

		// Check that inbound and outbound ackIDs have been resynced
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_IN_ACKID_CSR(port),
								&chk_data));
		assert_int_equal(ACKID_CAP_BASE + port, chk_data);
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_OUT_ACKID_CSR(port),
								&chk_data));
		assert_int_equal(0x8001f01f, chk_data);
	}

	(void)state;
}

static void rxs_rio_pc_secure_port_bad_parms(void **state)
{
	rio_pc_secure_port_in_t pc_in;
	rio_pc_secure_port_out_t pc_out;

	// Bad number of ports
	pc_in.ptl.num_ports = NUM_RXS_PORTS(&mock_dev_info) + 1;
	pc_in.mtc_pkts_allowed = true;
	pc_in.MECS_participant = true;
	pc_in.MECS_acceptance = true;
	pc_in.rst = rio_pc_rst_device;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_secure_port(&mock_dev_info, &pc_in, &pc_out));
	assert_int_not_equal(RIO_SUCCESS, pc_out.imp_rc);

	// Bad port number
	pc_in.ptl.num_ports = 1;
	pc_in.ptl.pnums[0] = NUM_RXS_PORTS(&mock_dev_info);
	pc_in.mtc_pkts_allowed = true;
	pc_in.MECS_participant = true;
	pc_in.MECS_acceptance = true;
	pc_in.rst = rio_pc_rst_device;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_secure_port(&mock_dev_info, &pc_in, &pc_out));
	assert_int_not_equal(RIO_SUCCESS, pc_out.imp_rc);

	// Bad device reset configuration
	pc_in.ptl.num_ports = NUM_RXS_PORTS(&mock_dev_info);
	pc_in.mtc_pkts_allowed = true;
	pc_in.MECS_participant = true;
	pc_in.MECS_acceptance = true;
	pc_in.rst = rio_pc_rst_last;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_secure_port(&mock_dev_info, &pc_in, &pc_out));
	assert_int_not_equal(RIO_SUCCESS, pc_out.imp_rc);

	(void)state;
}

// Test register settings for each control value.
static void rxs_rio_pc_secure_port_success(void **state)
{
	rio_pc_secure_port_in_t pc_in;
	rio_pc_secure_port_out_t pc_out;
	uint32_t temp;
	rio_port_t port;
	uint32_t plm_ctl_mask = RXS_PLM_SPX_IMP_SPEC_CTL_SELF_RST |
				RXS_PLM_SPX_IMP_SPEC_CTL_PORT_SELF_RST;
	uint32_t ctl_mask = RXS_SPX_CTL_MULT_CS;
	uint32_t filt_mask = RXS_TLM_SPX_FTYPE_FILT_MTC;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Test on one port at a time.
	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		// Power up the port
		set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, port);

		// Turn all items off

		pc_in.ptl.num_ports = 1;
		pc_in.ptl.pnums[0] = port;
		pc_in.mtc_pkts_allowed = false;
		pc_in.MECS_participant = false;
		pc_in.MECS_acceptance = false;
		pc_in.rst = rio_pc_rst_device;
		pc_out.imp_rc = RIO_SUCCESS;

		if (DEBUG_PRINTF) {
			printf("\nrst = %d %s\n",
				pc_in.rst, rst_to_str[pc_in.rst]);
		}
		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_secure_port(
					&mock_dev_info, &pc_in, &pc_out));
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

		assert_false(pc_out.bc_mtc_pkts_allowed);
		assert_false(pc_out.MECS_participant);
		assert_true(pc_out.MECS_acceptance);
		assert_int_equal(rio_pc_rst_device, pc_out.rst);

		// Check register values for MECS, filter, and reset...

		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_CTL(port), &temp));
		assert_int_equal(0, temp & ctl_mask);

		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_TLM_SPX_FTYPE_FILT(port),
									&temp));
		assert_int_equal(filt_mask, temp & filt_mask);

		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
				RXS_PLM_SPX_IMP_SPEC_CTL(port), &temp));
		assert_int_equal(plm_ctl_mask, temp & plm_ctl_mask);

		// Turn all items on

		pc_in.ptl.num_ports = 1;
		pc_in.ptl.pnums[0] = port;
		pc_in.mtc_pkts_allowed = true;
		pc_in.MECS_participant = true;
		pc_in.MECS_acceptance = true;
		pc_in.rst = rio_pc_rst_device;
		pc_out.imp_rc = RIO_SUCCESS;

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_secure_port(
					&mock_dev_info, &pc_in, &pc_out));
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

		assert_true(pc_out.bc_mtc_pkts_allowed);
		assert_true(pc_out.MECS_participant);
		assert_true(pc_out.MECS_acceptance);
		assert_int_equal(rio_pc_rst_device, pc_out.rst);

		// Check register values for MECS, filter, and reset...

		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_SPX_CTL(port), &temp));
		assert_int_equal(ctl_mask, temp & ctl_mask);

		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
				RXS_TLM_SPX_FTYPE_FILT(port), &temp));
		assert_int_equal(0, temp & filt_mask);

		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
				RXS_PLM_SPX_IMP_SPEC_CTL(port), &temp));
		assert_int_equal(plm_ctl_mask, temp & plm_ctl_mask);
	}

	(void)state;
}

// Test register settings for each reset control value
static void rxs_rio_pc_secure_port_rst_cfg(void **state)
{
	rio_pc_secure_port_in_t pc_in;
	rio_pc_secure_port_out_t pc_out;
	uint32_t temp;
	rio_port_t port;
	uint32_t plm_ctl_mask = RXS_PLM_SPX_IMP_SPEC_CTL_SELF_RST |
				RXS_PLM_SPX_IMP_SPEC_CTL_PORT_SELF_RST;
	uint32_t port_mask;
	uint32_t exp_plm_ctl = 0;
	uint32_t exp_em_int = 0;
	uint32_t exp_em_pw = 0;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Test on one port at a time.
	// Change reset configuration based on port number
	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		// Power up the port
		set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, port);

		// Test reset port configuration
		pc_in.ptl.num_ports = 1;
		pc_in.ptl.pnums[0] = port;
		pc_in.mtc_pkts_allowed = false;
		pc_in.MECS_participant = false;
		pc_in.MECS_acceptance = false;
		pc_in.rst = (rio_pc_rst_handling)(port % rio_pc_rst_last);
		pc_out.imp_rc = RIO_SUCCESS;

		if (DEBUG_PRINTF) {
			printf("\nport %d rst = %d %s\n",
				port, pc_in.rst, rst_to_str[pc_in.rst]);
		}
		port_mask = 1 << port;
		switch (pc_in.rst) {
		case rio_pc_rst_device:
			exp_plm_ctl = plm_ctl_mask;
			exp_em_int = 0;
			exp_em_pw = 0;
			break;

		case rio_pc_rst_port:
			exp_plm_ctl = RXS_PLM_SPX_IMP_SPEC_CTL_PORT_SELF_RST;
			exp_em_int = 0;
			exp_em_pw = 0;
			break;

		case rio_pc_rst_int:
			exp_plm_ctl = 0;
			exp_em_int = port_mask;
			exp_em_pw = 0;
			break;

		case rio_pc_rst_pw:
			exp_plm_ctl = 0;
			exp_em_int = 0;
			exp_em_pw = port_mask;
			break;

		case rio_pc_rst_ignore:
			exp_plm_ctl = 0;
			exp_em_int = 0;
			exp_em_pw = 0;
			break;

		default:
			assert_true(false);
		}

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_secure_port(
					&mock_dev_info, &pc_in, &pc_out));
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

		assert_false(pc_out.bc_mtc_pkts_allowed);
		assert_false(pc_out.MECS_participant);
		assert_true(pc_out.MECS_acceptance);
		assert_int_equal(pc_in.rst, pc_out.rst);

		// Check register values for reset...

		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info,
				RXS_PLM_SPX_IMP_SPEC_CTL(port), &temp));
		assert_int_equal(exp_plm_ctl, temp & plm_ctl_mask);
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_EM_RST_INT_EN, &temp));
		assert_int_equal(exp_em_int, temp & port_mask);
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_EM_RST_PW_EN, &temp));
		assert_int_equal(exp_em_pw, temp & port_mask);

	}

	(void)state;
}

static void rxs_rio_pc_dev_reset_config_bad_parms(void **state)
{
	rio_pc_dev_reset_config_in_t pc_in;
	rio_pc_dev_reset_config_out_t pc_out;

	// Bad device reset configuration
	pc_in.rst = rio_pc_rst_last;
	pc_out.imp_rc = RIO_SUCCESS;
	pc_out.rst = rio_pc_rst_last;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_dev_reset_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_not_equal(RIO_SUCCESS, pc_out.imp_rc);

	(void)state;
}

#define INCR_RST(x) ((rio_pc_rst_handling)((unsigned int)(x) + 1))
// Test register settings for each reset control value
static void rxs_rio_pc_dev_reset_config_rst_cfg(void **state)
{
	rio_pc_dev_reset_config_in_t pc_in;
	rio_pc_dev_reset_config_out_t pc_out;
	uint32_t temp;
	rio_port_t port;
	uint32_t plm_ctl_mask = RXS_PLM_SPX_IMP_SPEC_CTL_SELF_RST |
				RXS_PLM_SPX_IMP_SPEC_CTL_PORT_SELF_RST;
	uint32_t port_mask = (1 << NUM_RXS_PORTS(&mock_dev_info)) - 1;
	uint32_t exp_plm_ctl;
	uint32_t exp_em_int;
	uint32_t exp_em_pw;
	rio_pc_rst_handling rst;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Power up all ports
	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, port);
	}

	// Test each configuration
	for (rst = rio_pc_rst_device; rst < rio_pc_rst_last; rst = INCR_RST(rst)) {
		if (DEBUG_PRINTF) {
			printf("\nrst = %d %s\n", rst, rst_to_str[rst]);
		}
		// Test reset port configuration
		pc_in.rst = rst;
		pc_out.imp_rc = RIO_SUCCESS;
		pc_out.rst = rio_pc_rst_last;

		switch (pc_in.rst) {
		case rio_pc_rst_device:
			exp_plm_ctl = plm_ctl_mask;
			exp_em_int = 0;
			exp_em_pw = 0;
			break;

		case rio_pc_rst_port:
			exp_plm_ctl = RXS_PLM_SPX_IMP_SPEC_CTL_PORT_SELF_RST;
			exp_em_int = 0;
			exp_em_pw = 0;
			break;

		case rio_pc_rst_int:
			exp_plm_ctl = 0;
			exp_em_int = port_mask;
			exp_em_pw = 0;
			break;

		case rio_pc_rst_pw:
			exp_plm_ctl = 0;
			exp_em_int = 0;
			exp_em_pw = port_mask;
			break;

		case rio_pc_rst_ignore:
			exp_plm_ctl = 0;
			exp_em_int = 0;
			exp_em_pw = 0;
			break;

		default:
			assert_true(false);
		}

		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_dev_reset_config(
					&mock_dev_info, &pc_in, &pc_out));
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
		assert_int_equal(pc_in.rst, pc_out.rst);

		// Check register values for reset...

		for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
			assert_int_equal(RIO_SUCCESS,
				DARRegRead(&mock_dev_info,
					RXS_PLM_SPX_IMP_SPEC_CTL(port), &temp));
			assert_int_equal(exp_plm_ctl, temp & plm_ctl_mask);
		}
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_EM_RST_INT_EN, &temp));
		assert_int_equal(exp_em_int, temp);
		assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, RXS_EM_RST_PW_EN, &temp));
		assert_int_equal(exp_em_pw, temp);
	}

	(void)state;
}

static void rxs_rio_pc_set_config_check_parms_test(void **state)
{
	rio_pc_set_config_in_t pc_in;
	rio_pc_set_config_out_t pc_out;
	rio_pc_ls_t ls;

	assert_int_not_equal(PC_SET_CONFIG(0x00), RIO_SUCCESS);

	// Test out of range number of ports
	pc_in.lrto = 0x1000;
	pc_in.log_rto = 0x1000;
	pc_in.oob_reg_acc = true;
	pc_in.reg_acc_port = 0;
	pc_in.num_ports = RIO_ALL_PORTS;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x01), pc_out.imp_rc);

	// Test out of range number of ports
	pc_in.lrto = 0x1000;
	pc_in.log_rto = 0x1000;
	pc_in.oob_reg_acc = true;
	pc_in.reg_acc_port = 0;
	pc_in.num_ports = 0;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x01), pc_out.imp_rc);

	// Test out of range register access port
	pc_in.lrto = 0x1000;
	pc_in.log_rto = 0x1000;
	pc_in.oob_reg_acc = false;
	pc_in.reg_acc_port = NUM_RXS_PORTS(&mock_dev_info);
	pc_in.num_ports = 1;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x02), pc_out.imp_rc);

	// Test out of range register access port
	pc_in.lrto = 0x1000;
	pc_in.log_rto = 0x1000;
	pc_in.oob_reg_acc = false;
	pc_in.reg_acc_port = RIO_ALL_PORTS;
	pc_in.num_ports = 1;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x02), pc_out.imp_rc);

	// Test out of range config port number...
	pc_in.reg_acc_port = 1;
	pc_in.pc[0].pnum = NUM_RXS_PORTS(&mock_dev_info);
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x03), pc_out.imp_rc);

	// Test out of range port width value
	pc_out.imp_rc = RIO_SUCCESS;

	pc_in.pc[0].pnum = 4;
	pc_in.pc[0].port_available = true;
	pc_in.pc[0].powered_up = true;
        pc_in.pc[0].pw = rio_pc_pw_last;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x05), pc_out.imp_rc);

	// Test illegal port width values for odd ports
	pc_in.pc[0].pnum = 5;
        pc_in.pc[0].pw = rio_pc_pw_4x;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x07), pc_out.imp_rc);

	// Test illegal port width values for odd ports
	pc_in.pc[0].pnum = 5;
        pc_in.pc[0].pw = rio_pc_pw_1x_l2;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x07), pc_out.imp_rc);

	// Test out of range flow control value
	pc_in.pc[0].pnum = 4;
        pc_in.pc[0].pw = rio_pc_pw_4x;
        pc_in.pc[0].ls = rio_pc_ls_2p5;
        pc_in.pc[0].fc = rio_pc_fc_last;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x09), pc_out.imp_rc);

	// Test illegal flow control value
	pc_in.pc[0].pnum = 4;
        pc_in.pc[0].pw = rio_pc_pw_4x;
        pc_in.pc[0].fc = rio_pc_fc_tx;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x09), pc_out.imp_rc);

	// Test unsupported lane speed value
        pc_in.pc[0].fc = rio_pc_fc_rx;
        pc_in.pc[0].ls = rio_pc_ls_1p25;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x0B), pc_out.imp_rc);

	// Test out of range lane speed value
        pc_in.pc[0].fc = rio_pc_fc_rx;
        pc_in.pc[0].ls = rio_pc_ls_last;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x0D), pc_out.imp_rc);

	// Test out of range tx lane swap value
        pc_in.pc[0].fc = rio_pc_fc_rx;
        pc_in.pc[0].ls = rio_pc_ls_2p5;
        pc_in.pc[0].tx_lswap = rio_lswap_last;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x0F), pc_out.imp_rc);

	// Test out of range rx lane swap value
        pc_in.pc[0].fc = rio_pc_fc_rx;
        pc_in.pc[0].ls = rio_pc_ls_2p5;
        pc_in.pc[0].tx_lswap = rio_lswap_none;
        pc_in.pc[0].rx_lswap = rio_lswap_last;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x11), pc_out.imp_rc);

	// Test illegal lane speed/idle sequence combinations
        pc_in.pc[0].rx_lswap = rio_lswap_none;
        pc_in.pc[0].iseq = rio_pc_is_one;
#define INC_LS(x) (x = (rio_pc_ls_t)(int(x) + 1))
	for (ls = rio_pc_ls_6p25; ls <= rio_pc_ls_12p5; INC_LS(ls)) {
		pc_in.pc[0].ls = ls;
		pc_out.imp_rc = RIO_SUCCESS;

		assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_pc_set_config(&mock_dev_info,
						&pc_in, &pc_out));
		assert_int_equal(PC_SET_CONFIG(0x13), pc_out.imp_rc);
	}

        pc_in.pc[0].iseq = rio_pc_is_two;
	for (ls = rio_pc_ls_10p3; ls <= rio_pc_ls_12p5; INC_LS(ls)) {
		pc_in.pc[0].ls = ls;
		pc_out.imp_rc = RIO_SUCCESS;

		assert_int_not_equal(RIO_SUCCESS,
			rxs_rio_pc_set_config(&mock_dev_info,
						&pc_in, &pc_out));
		assert_int_equal(PC_SET_CONFIG(0x15), pc_out.imp_rc);
	}

	// Test out of range idle sequence
        pc_in.pc[0].iseq = rio_pc_is_last;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x17), pc_out.imp_rc);

	(void)state;
}

static void rxs_rio_pc_set_config_check_conflicts_test(void **state)
{
	rio_pc_set_config_in_t pc_in;
	rio_pc_set_config_out_t pc_out;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Setup for illegal port configuration test...
	// Port 0 - 2x
	// Port 1 - 2x
	// Port 2 - 4x
	// port 3 - unavailable
	set_all_port_config(cfg_perfect_2x, NO_TTL, NO_FILT, 0);
	set_all_port_config(cfg_perfect_2x, NO_TTL, NO_FILT, 1);
	set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, 2);
	set_all_port_config(cfg_unavl, NO_TTL, NO_FILT, 3);

	// Test illegal port configurations for "check both"
	// Port 0 config as 4x fails, cannot reconfigure PATH.
	pc_in.lrto = 0x1000;
	pc_in.log_rto = 0x1000;
	pc_in.oob_reg_acc = false;
	pc_in.reg_acc_port = 5;
	pc_in.num_ports = 2;
	pc_out.imp_rc = RIO_SUCCESS;

	pc_in.pc[0].pnum = 0;
        pc_in.pc[0].port_available = true;
        pc_in.pc[0].powered_up = true;
        pc_in.pc[0].pw = rio_pc_pw_4x;
        pc_in.pc[0].ls = rio_pc_ls_5p0;
        pc_in.pc[0].fc = rio_pc_fc_rx;
        pc_in.pc[0].iseq = rio_pc_is_one;
        pc_in.pc[0].xmitter_disable = false;
        pc_in.pc[0].port_lockout = false;
        pc_in.pc[0].nmtc_xfer_enable = true;
        pc_in.pc[0].tx_lswap = rio_lswap_none;
        pc_in.pc[0].tx_linvert[0] = false;
        pc_in.pc[0].tx_linvert[1] = false;
        pc_in.pc[0].tx_linvert[2] = false;
        pc_in.pc[0].tx_linvert[3] = false;
        pc_in.pc[0].rx_lswap = rio_lswap_none;
        pc_in.pc[0].rx_linvert[0] = false;
        pc_in.pc[0].rx_linvert[1] = false;
        pc_in.pc[0].rx_linvert[2] = false;
        pc_in.pc[0].rx_linvert[3] = false;

	pc_in.pc[1].pnum = 1;
        pc_in.pc[1].port_available = true;
        pc_in.pc[1].powered_up = true;
        pc_in.pc[1].pw = rio_pc_pw_2x;
        pc_in.pc[1].ls = rio_pc_ls_5p0;
        pc_in.pc[1].fc = rio_pc_fc_rx;
        pc_in.pc[1].iseq = rio_pc_is_one;
        pc_in.pc[1].xmitter_disable = false;
        pc_in.pc[1].port_lockout = false;
        pc_in.pc[1].nmtc_xfer_enable = true;
        pc_in.pc[1].tx_lswap = rio_lswap_none;
        pc_in.pc[1].tx_linvert[0] = false;
        pc_in.pc[1].tx_linvert[1] = false;
        pc_in.pc[1].tx_linvert[2] = false;
        pc_in.pc[1].tx_linvert[3] = false;
        pc_in.pc[1].rx_lswap = rio_lswap_none;
        pc_in.pc[1].rx_linvert[0] = false;
        pc_in.pc[1].rx_linvert[1] = false;
        pc_in.pc[1].rx_linvert[2] = false;
        pc_in.pc[1].rx_linvert[3] = false;
	pc_out.imp_rc = RIO_SUCCESS;

	// check_both_conf tests
	//
	// Check if the request is for a 4x port and 2x port
	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x32), pc_out.imp_rc);

	// Test illegal port configurations...
	// Port 0 as downgraded 4x fails, cannot reconfigure PATH.
        pc_in.pc[0].pw = rio_pc_pw_1x_l2;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x32), pc_out.imp_rc);

	// Check if the current configuration supports a 4x port,
	// cannot configure a 2x port.
	pc_in.pc[0].pnum = 2;
        pc_in.pc[0].pw = rio_pc_pw_2x;
	pc_in.pc[1].pnum = 3;
        pc_in.pc[1].pw = rio_pc_pw_2x;
	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x30), pc_out.imp_rc);

	// Check if want to make odd port unavailable,
	// cannot configure a 2x port.
	pc_in.pc[0].pnum = 0;
	pc_in.pc[1].pnum = 1;
	pc_in.pc[1].port_available = false;
	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x34), pc_out.imp_rc);

	// Lane speed conflict between port 0 & 1 - 10.3 & 2.5
	pc_in.pc[1].port_available = true;
        pc_in.pc[0].ls = rio_pc_ls_2p5;
        pc_in.pc[1].ls = rio_pc_ls_10p3;
        pc_in.pc[1].iseq = rio_pc_is_three;

	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x36), pc_out.imp_rc);

	// Lane speed conflict between port 0 & 1 - 10.3 & 5.0
        pc_in.pc[0].ls = rio_pc_ls_5p0;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x36), pc_out.imp_rc);

	// Lane speed conflict between port 0 & 1 - 2.5 and 10.3
        pc_in.pc[0].ls = rio_pc_ls_10p3;
        pc_in.pc[0].iseq = rio_pc_is_three;
        pc_in.pc[1].ls = rio_pc_ls_2p5;
        pc_in.pc[1].iseq = rio_pc_is_one;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x36), pc_out.imp_rc);

	// Lane speed conflict between port 0 & 1 - 5.0 and 10.3
        pc_in.pc[0].ls = rio_pc_ls_10p3;
        pc_in.pc[1].ls = rio_pc_ls_5p0;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x36), pc_out.imp_rc);

	// check_even_conf tests
	//
	// If configuring an even port only, refuse if the odd port is available
	pc_in.num_ports = 1;
	pc_out.imp_rc = RIO_SUCCESS;

	pc_in.pc[0].pnum = 0;
        pc_in.pc[0].pw = rio_pc_pw_4x;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x3C), pc_out.imp_rc);

	// If configuring an even port only, refuse if the odd port is available
	pc_in.num_ports = 1;
	pc_out.imp_rc = RIO_SUCCESS;

        pc_in.pc[0].pw = rio_pc_pw_1x_l2;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x3C), pc_out.imp_rc);

	// Check for lane speed conflicts.
        pc_in.pc[0].pw = rio_pc_pw_2x;
        pc_in.pc[0].ls = rio_pc_ls_10p3;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x40), pc_out.imp_rc);

	// check_odd_conf tests
	//
	// If configuring an odd port only, refuse if the even port is 4x or
	// the odd port is currently unavailable.
	pc_in.num_ports = 1;
	pc_out.imp_rc = RIO_SUCCESS;

	pc_in.pc[0].pnum = 3;
        pc_in.pc[0].pw = rio_pc_pw_2x;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x44), pc_out.imp_rc);

        pc_in.pc[0].pw = rio_pc_pw_1x;
	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x44), pc_out.imp_rc);

	// If configuring an odd port only,
	// refuse to make the odd port unavailable
	pc_in.pc[0].pnum = 1;
	pc_in.pc[0].port_available = false;
	pc_out.imp_rc = RIO_SUCCESS;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x46), pc_out.imp_rc);

	// Check for lane speed conflicts.
	pc_in.pc[0].port_available = true;
        pc_in.pc[0].ls = rio_pc_ls_10p3;

	assert_int_not_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(PC_SET_CONFIG(0x48), pc_out.imp_rc);

	(void)state;
}

static void rxs_rio_pc_set_config_success_two_2x_test(void **state)
{
	rio_pc_set_config_in_t pc_in;
	rio_pc_set_config_out_t pc_out;
	const rio_pc_pw_t twox_pws[] = {rio_pc_pw_1x_l0,
				rio_pc_pw_1x_l1,
				rio_pc_pw_2x};
	const unsigned int num_pws = sizeof(twox_pws) / sizeof (twox_pws[0]);
	unsigned int i, j;
	uint32_t rc;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Check all legal configurations of two 2x wide ports.
	// Port 0 - 2x
	// Port 1 - 2x
	set_all_port_config(cfg_perfect_2x, NO_TTL, NO_FILT, 0);
	set_all_port_config(cfg_perfect_2x, NO_TTL, NO_FILT, 1);

	// Test all lane speeds, port 1 always uses IDLE3, port 0 uses default
	// idle sequences.
	pc_in.lrto = 0x1000;
	pc_in.log_rto = 0x1000;
	pc_in.oob_reg_acc = false;
	pc_in.reg_acc_port = 5;
	pc_in.num_ports = 2;
	pc_out.imp_rc = RIO_SUCCESS;

	pc_in.pc[0].pnum = 0;
        pc_in.pc[0].port_available = true;
        pc_in.pc[0].powered_up = true;
        pc_in.pc[0].pw = rio_pc_pw_2x;
        pc_in.pc[0].ls = rio_pc_ls_2p5;
        pc_in.pc[0].fc = rio_pc_fc_rx;
        pc_in.pc[0].iseq = rio_pc_is_one;
        pc_in.pc[0].xmitter_disable = false;
        pc_in.pc[0].port_lockout = false;
        pc_in.pc[0].nmtc_xfer_enable = true;
        pc_in.pc[0].tx_lswap = rio_lswap_none;
        pc_in.pc[0].tx_linvert[0] = false;
        pc_in.pc[0].tx_linvert[1] = false;
        pc_in.pc[0].tx_linvert[2] = false;
        pc_in.pc[0].tx_linvert[3] = false;
        pc_in.pc[0].rx_lswap = rio_lswap_none;
        pc_in.pc[0].rx_linvert[0] = false;
        pc_in.pc[0].rx_linvert[1] = false;
        pc_in.pc[0].rx_linvert[2] = false;
        pc_in.pc[0].rx_linvert[3] = false;

	pc_in.pc[1].pnum = 1;
        pc_in.pc[1].port_available = true;
        pc_in.pc[1].powered_up = true;
        pc_in.pc[1].pw = rio_pc_pw_1x_l0;
        pc_in.pc[1].ls =rio_pc_ls_2p5;
        pc_in.pc[1].fc = rio_pc_fc_rx;
        pc_in.pc[1].iseq = rio_pc_is_three;
        pc_in.pc[1].xmitter_disable = false;
        pc_in.pc[1].port_lockout = false;
        pc_in.pc[1].nmtc_xfer_enable = true;
        pc_in.pc[1].tx_lswap = rio_lswap_none;
        pc_in.pc[1].tx_linvert[0] = false;
        pc_in.pc[1].tx_linvert[1] = false;
        pc_in.pc[1].tx_linvert[2] = false;
        pc_in.pc[1].tx_linvert[3] = false;
        pc_in.pc[1].rx_lswap = rio_lswap_none;
        pc_in.pc[1].rx_linvert[0] = false;
        pc_in.pc[1].rx_linvert[1] = false;
        pc_in.pc[1].rx_linvert[2] = false;
        pc_in.pc[1].rx_linvert[3] = false;
	pc_out.imp_rc = RIO_SUCCESS;

	for (i = (unsigned int)rio_pc_ls_2p5; i < rio_pc_ls_last; i++) {
		pc_in.pc[0].ls = (rio_pc_ls_t)i;
		pc_in.pc[1].ls = (rio_pc_ls_t)i;
		pc_out.imp_rc = 0xFFFFFFFF;

		switch((rio_pc_ls_t)i) {
		case rio_pc_ls_2p5:
		case rio_pc_ls_3p125:
		case rio_pc_ls_5p0:
			pc_in.pc[0].iseq = rio_pc_is_one;
			break;
		case rio_pc_ls_6p25:
			pc_in.pc[0].iseq = rio_pc_is_two;
			break;
		case rio_pc_ls_10p3:
		case rio_pc_ls_12p5:
			pc_in.pc[0].iseq = rio_pc_is_three;
			break;
		case rio_pc_ls_1p25:
		case rio_pc_ls_last:
		default:
			// Should never get here...
			assert_true(false);
		}
		if (DEBUG_PRINTF) {
			printf("\nport 0 ls %d %s iseq %d %s\n",
				i, ls_to_str[i],
				pc_in.pc[0].iseq, is_to_str[pc_in.pc[0].iseq]);
		}
		rc = rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out);
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
		assert_int_equal(RIO_SUCCESS, rc);

		compare_pc(&pc_in.pc[0], &pc_out.pc[0]);
		compare_pc(&pc_in.pc[1], &pc_out.pc[1]);
	}

	// Check all 2x port width values
	for (i = 0; i < num_pws; i++) {
		pc_in.pc[0].pw = twox_pws[i];
		for (j = 0; j < num_pws; j++) {
			pc_in.pc[1].pw = twox_pws[j];

			if (DEBUG_PRINTF) {
				printf("\npw %d %d port 0 %d %s port 1 %d %s\n",
					i, j,
					pc_in.pc[0].pw,
					pw_to_str[pc_in.pc[0].pw],
					pc_in.pc[1].pw,
					pw_to_str[pc_in.pc[1].pw]);
			}
			assert_int_equal(RIO_SUCCESS,
				rxs_rio_pc_set_config(
					&mock_dev_info, &pc_in, &pc_out));
			assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

			compare_pc(&pc_in.pc[0], &pc_out.pc[0]);
			compare_pc(&pc_in.pc[1], &pc_out.pc[1]);
		}
	}

	// Power down both ports...
	if (DEBUG_PRINTF) {
		printf("\nPower Down\n");
	}
	pc_in.pc[0].powered_up = false;
	pc_in.pc[1].powered_up = false;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

        pc_in.pc[0].pw = rio_pc_pw_last;
        pc_in.pc[0].ls = rio_pc_ls_last;
        pc_in.pc[0].fc = rio_pc_fc_last;
        pc_in.pc[0].iseq = rio_pc_is_last;
        pc_in.pc[0].xmitter_disable = false;
        pc_in.pc[0].port_lockout = false;
        pc_in.pc[0].nmtc_xfer_enable = false;

        pc_in.pc[1].pw = rio_pc_pw_last;
        pc_in.pc[1].ls = rio_pc_ls_last;
        pc_in.pc[1].fc = rio_pc_fc_last;
        pc_in.pc[1].iseq = rio_pc_is_last;
        pc_in.pc[1].xmitter_disable = false;
        pc_in.pc[1].port_lockout = false;
        pc_in.pc[1].nmtc_xfer_enable = false;

	compare_pc(&pc_in.pc[0], &pc_out.pc[0]);
	compare_pc(&pc_in.pc[1], &pc_out.pc[1]);

	// Power up both ports...
	if (DEBUG_PRINTF) {
		printf("\nPower Up\n");
	}
	pc_in.pc[0].powered_up = true;
        pc_in.pc[0].pw = rio_pc_pw_2x;
        pc_in.pc[0].ls = rio_pc_ls_2p5;
        pc_in.pc[0].fc = rio_pc_fc_rx;
        pc_in.pc[0].iseq = rio_pc_is_one;
        pc_in.pc[0].xmitter_disable = false;
        pc_in.pc[0].port_lockout = false;
        pc_in.pc[0].nmtc_xfer_enable = true;

	pc_in.pc[1].powered_up = true;
        pc_in.pc[1].pw = rio_pc_pw_2x;
        pc_in.pc[1].ls = rio_pc_ls_2p5;
        pc_in.pc[1].fc = rio_pc_fc_rx;
        pc_in.pc[1].iseq = rio_pc_is_one;
        pc_in.pc[1].xmitter_disable = false;
        pc_in.pc[1].port_lockout = false;
        pc_in.pc[1].nmtc_xfer_enable = true;
	assert_int_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

	compare_pc(&pc_in.pc[0], &pc_out.pc[0]);
	compare_pc(&pc_in.pc[1], &pc_out.pc[1]);

	// Disable transmitter, lockout ports, disable nmtc_xfer on port 0
	if (DEBUG_PRINTF) {
		printf("\ndisable & lockout\n");
	}
	pc_in.pc[0].powered_up = true;
	pc_in.pc[0].port_lockout = true;
	pc_in.pc[0].xmitter_disable = true;
	pc_in.pc[0].nmtc_xfer_enable = false;
	pc_in.pc[1].powered_up = true;
	pc_in.pc[1].port_lockout = false;
	pc_in.pc[1].xmitter_disable = false;
	pc_in.pc[1].nmtc_xfer_enable = true;

	assert_int_equal(RIO_SUCCESS,
		rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
	assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

	compare_pc(&pc_in.pc[0], &pc_out.pc[0]);
	compare_pc(&pc_in.pc[1], &pc_out.pc[1]);

	// Check all lane swaps...
	if (DEBUG_PRINTF) {
		printf("\nlane swaps\n");
	}
	for (i = 0; i < (unsigned int)rio_lswap_last; i++) {
		pc_in.pc[0].tx_lswap = (rio_lane_swap_t)i;
		pc_in.pc[1].tx_lswap = (rio_lane_swap_t)i;
		for (j = 0; j < (unsigned int)rio_lswap_last; j++) {
			pc_in.pc[0].rx_lswap = (rio_lane_swap_t)j;
			pc_in.pc[1].rx_lswap = (rio_lane_swap_t)j;

			assert_int_equal(RIO_SUCCESS,
				rxs_rio_pc_set_config(
					&mock_dev_info, &pc_in, &pc_out));
			assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

			compare_pc(&pc_in.pc[0], &pc_out.pc[0]);
			compare_pc(&pc_in.pc[1], &pc_out.pc[1]);
		}
	}
	// Check all lane inversions...
	if (DEBUG_PRINTF) {
		printf("\nlane inversions\n");
	}
	for (i = 0; i < (1 << RIO_MAX_PORT_LANES); i++) {
		for (j = 0; j < RIO_MAX_PORT_LANES; j++) {
			if ((1 << j) & i) {
				pc_in.pc[0].tx_linvert[j] = true;
				pc_in.pc[0].rx_linvert[j] = false;
				pc_in.pc[1].tx_linvert[j] = false;
				pc_in.pc[1].rx_linvert[j] = true;
			} else {
				pc_in.pc[0].tx_linvert[j] = true;
				pc_in.pc[0].rx_linvert[j] = false;
				pc_in.pc[1].tx_linvert[j] = false;
				pc_in.pc[1].rx_linvert[j] = true;
			}

			assert_int_equal(RIO_SUCCESS,
				rxs_rio_pc_set_config(
					&mock_dev_info, &pc_in, &pc_out));
			assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

			compare_pc(&pc_in.pc[0], &pc_out.pc[0]);
			compare_pc(&pc_in.pc[1], &pc_out.pc[1]);
		}
	}
	(void)state;
}

static void rxs_rio_pc_set_config_success_one_4x_test(void **state)
{
	rio_pc_set_config_in_t pc_in;
	rio_pc_set_config_out_t pc_out;
	unsigned int i, j;
	uint32_t rc;

	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;

	if (l_st->real_hw) {
		return;
	}

	// Check all legal configurations of one 4x wide port.
	set_all_port_config(cfg_perfect, NO_TTL, NO_FILT, 0);

	// Test all lane speeds with default idle sequences.
	pc_in.lrto = 0x1000;
	pc_in.log_rto = 0x1000;
	pc_in.oob_reg_acc = false;
	pc_in.reg_acc_port = 5;
	pc_in.num_ports = 1;
	pc_out.imp_rc = RIO_SUCCESS;

	pc_in.pc[0].pnum = 0;
        pc_in.pc[0].port_available = true;
        pc_in.pc[0].powered_up = true;
        pc_in.pc[0].pw = rio_pc_pw_4x;
        pc_in.pc[0].ls = rio_pc_ls_2p5;
        pc_in.pc[0].fc = rio_pc_fc_rx;
        pc_in.pc[0].iseq = rio_pc_is_one;
        pc_in.pc[0].xmitter_disable = false;
        pc_in.pc[0].port_lockout = false;
        pc_in.pc[0].nmtc_xfer_enable = true;
        pc_in.pc[0].tx_lswap = rio_lswap_none;
        pc_in.pc[0].tx_linvert[0] = false;
        pc_in.pc[0].tx_linvert[1] = false;
        pc_in.pc[0].tx_linvert[2] = false;
        pc_in.pc[0].tx_linvert[3] = false;
        pc_in.pc[0].rx_lswap = rio_lswap_none;
        pc_in.pc[0].rx_linvert[0] = false;
        pc_in.pc[0].rx_linvert[1] = false;
        pc_in.pc[0].rx_linvert[2] = false;
        pc_in.pc[0].rx_linvert[3] = false;

	pc_out.imp_rc = RIO_SUCCESS;

	for (i = (unsigned int)rio_pc_ls_2p5; i < rio_pc_ls_last; i++) {
		pc_in.pc[0].ls = (rio_pc_ls_t)i;
		pc_in.pc[1].ls = (rio_pc_ls_t)i;
		pc_out.imp_rc = 0xFFFFFFFF;

		switch((rio_pc_ls_t)i) {
		case rio_pc_ls_2p5:
		case rio_pc_ls_3p125:
		case rio_pc_ls_5p0:
			pc_in.pc[0].iseq = rio_pc_is_one;
			break;
		case rio_pc_ls_6p25:
			pc_in.pc[0].iseq = rio_pc_is_two;
			break;
		case rio_pc_ls_10p3:
		case rio_pc_ls_12p5:
			pc_in.pc[0].iseq = rio_pc_is_three;
			break;
		case rio_pc_ls_1p25:
		case rio_pc_ls_last:
		default:
			// Should never get here...
			assert_true(false);
		}
		if (DEBUG_PRINTF) {
			printf("\nls %d %s\n", i, ls_to_str[i]);
		}
		rc = rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out);
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
		assert_int_equal(RIO_SUCCESS, rc);

		compare_pc(&pc_in.pc[0], &pc_out.pc[0]);
	}

	// Check all 4x port width values
	for (i = (unsigned int)rio_pc_pw_2x; i < rio_pc_pw_last; i++) {
		// Can't override to 2x redundant lane 1.
		if (i == (unsigned int) rio_pc_pw_1x_l1) {
			continue;
		}
		pc_in.pc[0].pw = (rio_pc_pw_t)i;

		if (DEBUG_PRINTF) {
			printf("\npw %d %s\n", i, pw_to_str[i]);
		}
		assert_int_equal(RIO_SUCCESS,
			rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out));
		assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);

		compare_pc(&pc_in.pc[0], &pc_out.pc[0]);
	}

	// Power down port...
	if (DEBUG_PRINTF) {
		printf("\npower down\n");
	}
	pc_in.pc[0].powered_up = false;
        pc_in.pc[0].pw = rio_pc_pw_4x;
        pc_in.pc[0].ls = rio_pc_ls_12p5;
        pc_in.pc[0].fc = rio_pc_fc_rx;
        pc_in.pc[0].iseq = rio_pc_is_three;
        pc_in.pc[0].xmitter_disable = false;
        pc_in.pc[0].port_lockout = false;
        pc_in.pc[0].nmtc_xfer_enable = false;
        pc_in.pc[0].tx_lswap = rio_lswap_none;
        pc_in.pc[0].tx_linvert[0] = false;
        pc_in.pc[0].tx_linvert[1] = false;
        pc_in.pc[0].tx_linvert[2] = false;
        pc_in.pc[0].tx_linvert[3] = false;
        pc_in.pc[0].rx_lswap = rio_lswap_none;
        pc_in.pc[0].rx_linvert[0] = false;
        pc_in.pc[0].rx_linvert[1] = false;
        pc_in.pc[0].rx_linvert[2] = false;
        pc_in.pc[0].rx_linvert[3] = false;

	rc = rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out);
	assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
	assert_int_equal(RIO_SUCCESS, rc);

        pc_in.pc[0].pw = rio_pc_pw_last;
        pc_in.pc[0].ls = rio_pc_ls_last;
        pc_in.pc[0].fc = rio_pc_fc_last;
        pc_in.pc[0].iseq = rio_pc_is_last;

	compare_pc(&pc_in.pc[0], &pc_out.pc[0]);

	// Power up port...
	if (DEBUG_PRINTF) {
		printf("\npower up\n");
	}
	pc_in.pc[0].powered_up = true;
        pc_in.pc[0].pw = rio_pc_pw_4x;
        pc_in.pc[0].ls = rio_pc_ls_12p5;
        pc_in.pc[0].fc = rio_pc_fc_rx;
        pc_in.pc[0].iseq = rio_pc_is_three;
        pc_in.pc[0].xmitter_disable = false;
        pc_in.pc[0].port_lockout = false;
        pc_in.pc[0].nmtc_xfer_enable = true;
        pc_in.pc[0].tx_lswap = rio_lswap_none;
        pc_in.pc[0].tx_linvert[0] = false;
        pc_in.pc[0].tx_linvert[1] = false;
        pc_in.pc[0].tx_linvert[2] = false;
        pc_in.pc[0].tx_linvert[3] = false;
        pc_in.pc[0].rx_lswap = rio_lswap_none;
        pc_in.pc[0].rx_linvert[0] = false;
        pc_in.pc[0].rx_linvert[1] = false;
        pc_in.pc[0].rx_linvert[2] = false;
        pc_in.pc[0].rx_linvert[3] = false;

	rc = rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out);
	assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
	assert_int_equal(RIO_SUCCESS, rc);

	compare_pc(&pc_in.pc[0], &pc_out.pc[0]);

	// Disable transmitter, lockout ports, disable nmtc_xfer on port 0
	if (DEBUG_PRINTF) {
		printf("\ndisable & lockout\n");
	}
	pc_in.pc[0].powered_up = true;
	pc_in.pc[0].port_lockout = true;
	pc_in.pc[0].xmitter_disable = true;
	pc_in.pc[0].nmtc_xfer_enable = false;

	rc = rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out);
	assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
	assert_int_equal(RIO_SUCCESS, rc);

	compare_pc(&pc_in.pc[0], &pc_out.pc[0]);

	// Disable transmitter, lockout ports, disable nmtc_xfer on port 0
	if (DEBUG_PRINTF) {
		printf("\nenable & unlock\n");
	}
	pc_in.pc[0].port_lockout = false;
	pc_in.pc[0].xmitter_disable = false;
	pc_in.pc[0].nmtc_xfer_enable = true;

	rc = rxs_rio_pc_set_config(&mock_dev_info, &pc_in, &pc_out);
	assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
	assert_int_equal(RIO_SUCCESS, rc);

	compare_pc(&pc_in.pc[0], &pc_out.pc[0]);

	// Check all lane swaps...
	if (DEBUG_PRINTF) {
		printf("\nlane swaps\n");
	}
	for (i = 0; i < (unsigned int)rio_lswap_last; i++) {
		pc_in.pc[0].tx_lswap = (rio_lane_swap_t)i;
		for (j = 0; j < (unsigned int)rio_lswap_last; j++) {
			pc_in.pc[0].rx_lswap = (rio_lane_swap_t)j;

			rc = rxs_rio_pc_set_config(
					&mock_dev_info, &pc_in, &pc_out);
			assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
			assert_int_equal(RIO_SUCCESS, rc);

			compare_pc(&pc_in.pc[0], &pc_out.pc[0]);
		}
	}
	// Check all lane inversions...
	if (DEBUG_PRINTF) {
		printf("\nlane inversions\n");
	}
	for (i = 0; i < (1 << RIO_MAX_PORT_LANES); i++) {
		for (j = 0; j < RIO_MAX_PORT_LANES; j++) {
			if ((1 << j) & i) {
				pc_in.pc[0].tx_linvert[j] = true;
				pc_in.pc[0].rx_linvert[j] = false;
			} else {
				pc_in.pc[0].tx_linvert[j] = true;
				pc_in.pc[0].rx_linvert[j] = false;
			}

			rc = rxs_rio_pc_set_config(
					&mock_dev_info, &pc_in, &pc_out);
			assert_int_equal(RIO_SUCCESS, pc_out.imp_rc);
			assert_int_equal(RIO_SUCCESS, rc);

			compare_pc(&pc_in.pc[0], &pc_out.pc[0]);
		}
	}
	(void)state;
}

int main(int argc, char** argv)
{
	const struct CMUnitTest tests[] = {
			cmocka_unit_test(rxs_rio_pc_macros_test),
			cmocka_unit_test_setup(
				rxs_rio_pc_clk_pd_success_test, setup),
			cmocka_unit_test_setup(
				rxs_rio_pc_clk_pd_fail_test, setup),

			cmocka_unit_test_setup(
				rxs_rio_pc_get_config_success, setup),
			cmocka_unit_test_setup(
				rxs_rio_pc_get_config_bad_parms, setup),

			cmocka_unit_test_setup(
				rxs_rio_pc_get_status_success, setup),
			cmocka_unit_test_setup(
				rxs_rio_pc_get_status_bad_parms, setup),

			cmocka_unit_test_setup(
				rxs_rio_pc_reset_port_bad_parms, setup),
			cmocka_unit_test_setup(
				rxs_rio_pc_reset_port_exclude_reg_acc, setup),

			cmocka_unit_test_setup(
				rxs_rio_pc_reset_link_partner_bad_parms, setup),
			cmocka_unit_test_setup(
				rxs_rio_pc_reset_link_partner_success, setup),

			cmocka_unit_test_setup(
				rxs_rio_pc_clr_errs_bad_parms, setup),
			cmocka_unit_test_setup(
				rxs_rio_pc_clr_errs_success, setup),
			cmocka_unit_test_setup(
				rxs_rio_pc_clr_errs_resync_ackids, setup),

			cmocka_unit_test_setup(
				rxs_rio_pc_secure_port_bad_parms, setup),
			cmocka_unit_test_setup(
				rxs_rio_pc_secure_port_success, setup),
			cmocka_unit_test_setup(
				rxs_rio_pc_secure_port_rst_cfg, setup),

			cmocka_unit_test_setup(
				rxs_rio_pc_dev_reset_config_bad_parms, setup),
			cmocka_unit_test_setup(
				rxs_rio_pc_dev_reset_config_rst_cfg, setup),

			cmocka_unit_test_setup(
				rxs_rio_pc_set_config_check_conflicts_test, setup),
			cmocka_unit_test_setup(
				rxs_rio_pc_set_config_check_parms_test, setup),
			cmocka_unit_test_setup(
				rxs_rio_pc_set_config_success_one_4x_test, setup),
			cmocka_unit_test_setup(
				rxs_rio_pc_set_config_success_two_2x_test, setup),
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
