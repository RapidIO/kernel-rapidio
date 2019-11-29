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
#include "src/Tsi721_DeviceDriver.h"
#include "rio_standard.h"
#include "rio_ecosystem.h"
#include "tok_parse.h"
#include "libcli.h"
#include "rio_mport_lib.h"

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
	argc++;// not used

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(tsi721_not_supported_test)};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif /* TSI721_DAR_WANTED */

#ifdef TSI721_DAR_WANTED

// BEGIN common Tsi721 emulation setup
#define DEBUG_PRINTF 0

typedef struct Tsi721_test_state_t_TAG {
	int argc;
	char **argv;
	bool real_hw;
	uint32_t mport;
	hc_t hc;
	did_reg_t did_reg_val;
	int mp_h;
	bool mp_h_valid;
} Tsi721_test_state_t;

Tsi721_test_state_t st;

uint32_t tsi721_regs[] = {
	TSI721_SP_LT_CTL,
	TSI721_SR_RSP_TO,
	TSI721_SP_GEN_CTL,
	TSI721_SP_LM_REQ,
	TSI721_SP_LM_RESP,
	TSI721_SP_ACKID_STAT,
	TSI721_SP_CTL2,
	TSI721_SP_ERR_STAT,
	TSI721_SP_CTL,
	TSI721_PW_TGT_ID,
	TSI721_PW_CTL,
	TSI721_PW_ROUTE,
	TSI721_BASE_ID,
	TSI721_PLM_IMP_SPEC_CTL,
	TSI721_PLM_PW_ENABLE,
	TSI721_PLM_DENIAL_CTL,
	TSI721_SP_RATE_EN,
	TSI721_SP_ERR_RATE,
	TSI721_SP_ERR_THRESH,
	TSI721_SP_CTL,
	TSI721_EM_RST_PW_EN,
	TSI721_DEVCTL, // Read Only!!!
	TSI721_LOCAL_ERR_EN,
	TSI721_PLM_INT_ENABLE,
	TSI721_I2C_INT_ENABLE,
	TSI721_EM_RST_INT_EN,
	TSI721_PLM_ALL_INT_EN,
	TSI721_EM_INT_ENABLE,
	TSI721_EM_DEV_INT_EN,
	TSI721_PLM_ALL_PW_EN,
	TSI721_EM_PW_ENABLE,
	TSI721_EM_DEV_PW_EN,
	TSI721_PLM_EVENT_GEN,
	TSI721_I2C_INT_SET,
	TSI721_PLM_STATUS,
	TSI721_SP_ERR_DET,
	TSI721_LOCAL_ERR_DET,
	TSI721_I2C_INT_STAT,
	TSI721_EM_INT_STAT,
	TSI721_EM_PW_STAT,
	TSI721_EM_RST_PORT_STAT,
	TSI721_SP_ERR_ATTR_CAPT,
};

#define TSI721_TEST_DEV16_ID 0x2233
#define TSI721_TEST_DEV08_ID 0x11

#define TSI721_NUM_MOCK_REGS (sizeof(tsi721_regs)/sizeof(tsi721_regs[0]))
#define NUM_DAR_REG (TSI721_NUM_MOCK_REGS + TSI721_NUM_PERF_CTRS)
#define UPB_DAR_REG (NUM_DAR_REG+1)
#define MOCK_REG_ADDR(x) (x | 1)

static rio_perf_opt_reg_t mock_dar_reg[UPB_DAR_REG];
static DAR_DEV_INFO_t mock_dev_info;

static void tsi721_init_mock_dev_info(void)
{
	uint8_t idx;

	mock_dev_info.privateData = 0x0;
	mock_dev_info.accessInfo = 0x0;
	strcpy(mock_dev_info.name, "Tsi721");
	mock_dev_info.dsf_h = 0x00380000;
	mock_dev_info.extFPtrForPort = TSI721_SP_MB_HEAD;
	mock_dev_info.extFPtrPortType = RIO_EFB_T_SP_EP_SAER;
	mock_dev_info.extFPtrForLane = TSI721_PER_LANE_BH;
	mock_dev_info.extFPtrForErr = TSI721_ERR_RPT_BH;
	mock_dev_info.extFPtrForVC = 0;
	mock_dev_info.extFPtrForVOQ = 0;
	mock_dev_info.devID = 0x80AB0038;
	mock_dev_info.driver_family = RIO_TSI721_DEVICE;
	mock_dev_info.devInfo = 0;
	mock_dev_info.assyInfo = 0;
	mock_dev_info.features = 0xC000003F;
	mock_dev_info.swPortInfo = 0x00000100;
	mock_dev_info.swRtInfo = 0;
	mock_dev_info.srcOps = 0x0000FC04;
	mock_dev_info.dstOps = 0x0000FC04;
	mock_dev_info.swMcastInfo = 0;
	for (idx = 0; idx < RIO_MAX_PORTS; idx++) {
		mock_dev_info.ctl1_reg[idx] = 0;
	}

	for (idx = 0; idx < MAX_DAR_SCRPAD_IDX; idx++) {
		mock_dev_info.scratchpad[idx] = 0;
	}
}

// The behavior of the performance optimization register access
// must be overridden when registers are mocked.
//
// The elegant way to do this, while retaining transparency in the code,
// is to add 1 to the defined register offset to compute the mocked register
// offset..  This prevents the standard performance optimization register
// support from recognizing the register and updating the poregs array.

static uint32_t tsi721_get_poreg_idx(DAR_DEV_INFO_t *dev_info, uint32_t offset)
{
	return DAR_get_poreg_idx(dev_info, MOCK_REG_ADDR(offset));
}

static uint32_t tsi721_add_poreg(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t data)
{
	return DAR_add_poreg(dev_info, MOCK_REG_ADDR(offset), data);
}

/* Initialize the mock register structure for different registers.
 */

static uint32_t Tsi721ReadReg(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t *readdata)
{
	uint32_t rc = 0xFFFFFFFF;
	uint32_t idx = tsi721_get_poreg_idx(dev_info, MOCK_REG_ADDR(offset));

	if (NULL == dev_info) {
		return rc;
	}

	if (!st.real_hw) {
		if ((DAR_POREG_BAD_IDX == idx) && DEBUG_PRINTF) {
			printf("\nMissing offset is 0x%x\n", offset);
			assert_int_equal(0xFFFFFFFF, offset);
			assert_true(st.real_hw);
			idx = 0;
		} else {
			rc = RIO_SUCCESS;
		}
		*readdata = mock_dar_reg[idx].data;
		goto exit;
	}

	assert_true(st.mp_h_valid);
	if (0xFF == st.hc) {
		rc = rio_lcfg_read(st.mp_h, offset, 4, readdata);
	} else {
		rc = rio_maint_read(st.mp_h, st.did_reg_val, st.hc,
				offset, 4, readdata);
	}

exit:
	return rc;
}

static void tsi721_emulate_reg_write(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t writedata)
{
	uint32_t idx = tsi721_get_poreg_idx(dev_info, MOCK_REG_ADDR(offset));
	uint32_t t_idx;

	if ((DAR_POREG_BAD_IDX == idx) && DEBUG_PRINTF) {
		printf("\nMissing offset is 0x%x\n", offset);
		assert_int_equal(0xFFFFFFFF, offset);
		assert_true(st.real_hw);
		return;
	}

	switch (offset) {
	case TSI721_PLM_STATUS:
		dev_info->poregs[idx].data &= ~writedata;
		break;

	case TSI721_PLM_EVENT_GEN:
		// Event generation register clears itself...
		dev_info->poregs[idx].data = 0;
		t_idx = tsi721_get_poreg_idx(dev_info, TSI721_PLM_STATUS);
		if (DAR_POREG_BAD_IDX == t_idx) {
			assert_true(false);
			return;
		}
		dev_info->poregs[t_idx].data |= writedata;
		if (!(writedata & TSI721_PLM_EVENT_GEN_RST_REQ)) {
			return;
		}
		t_idx = tsi721_get_poreg_idx(dev_info, TSI721_EM_RST_PORT_STAT);
		if (DAR_POREG_BAD_IDX == t_idx) {
			assert_true(false);
			return;
		}
		dev_info->poregs[t_idx].data = TSI721_EM_RST_PORT_STAT_RST_REQ;
		t_idx = tsi721_get_poreg_idx(dev_info, TSI721_EM_RST_INT_EN);
		if (DAR_POREG_BAD_IDX == t_idx) {
			assert_true(false);
			return;
		}
		if (dev_info->poregs[t_idx].data) {
			t_idx = tsi721_get_poreg_idx(dev_info,
					TSI721_EM_INT_STAT);
			if (DAR_POREG_BAD_IDX == t_idx) {
				assert_true(false);
				return;
			}
			dev_info->poregs[t_idx].data |= TSI721_EM_INT_STAT_RCS;
		}
		t_idx = tsi721_get_poreg_idx(dev_info, TSI721_EM_RST_PW_EN);
		if (DAR_POREG_BAD_IDX == t_idx) {
			assert_true(false);
			return;
		}
		if (dev_info->poregs[t_idx].data) {
			t_idx = tsi721_get_poreg_idx(dev_info,
					TSI721_EM_PW_STAT);
			if (DAR_POREG_BAD_IDX == t_idx) {
				assert_true(false);
				return;
			}
			dev_info->poregs[t_idx].data |= TSI721_EM_PW_STAT_RCS;
		}
		break;
	case TSI721_I2C_INT_SET:
		dev_info->poregs[idx].data = 0;
		t_idx = tsi721_get_poreg_idx(dev_info, TSI721_I2C_INT_STAT);
		if (DAR_POREG_BAD_IDX == t_idx) {
			assert_true(false);
			return;
		}
		dev_info->poregs[t_idx].data |= writedata;
		break;
	case TSI721_LOCAL_ERR_DET:
		writedata &= (TSI721_LOCAL_ERR_DET_ILL_TYPE |
		TSI721_LOCAL_ERR_DET_ILL_ID);
		dev_info->poregs[idx].data = writedata;

		t_idx = tsi721_get_poreg_idx(dev_info, TSI721_LOCAL_ERR_EN);
		if (DAR_POREG_BAD_IDX == t_idx) {
			assert_true(false);
			return;
		}
		writedata &= dev_info->poregs[t_idx].data;

		t_idx = tsi721_get_poreg_idx(dev_info, TSI721_EM_INT_STAT);
		if (DAR_POREG_BAD_IDX == t_idx) {
			assert_true(false);
			return;
		}
		if (writedata) {
			dev_info->poregs[t_idx].data |=
					TSI721_EM_INT_STAT_LOCALOG;
		} else {
			dev_info->poregs[t_idx].data &=
					~TSI721_EM_INT_STAT_LOCALOG;
		}
		t_idx = tsi721_get_poreg_idx(dev_info, TSI721_EM_PW_STAT);
		if (DAR_POREG_BAD_IDX == t_idx) {
			assert_true(false);
			return;
		}
		if (writedata) {
			dev_info->poregs[t_idx].data |=
					TSI721_EM_PW_STAT_LOCALOG;
		} else {
			dev_info->poregs[t_idx].data &=
					~TSI721_EM_PW_STAT_LOCALOG;
		}
		break;
	case TSI721_EM_RST_PORT_STAT:
		dev_info->poregs[idx].data &= ~writedata;

		if (!(dev_info->poregs[idx].data)) {
			t_idx = tsi721_get_poreg_idx(dev_info,
			TSI721_EM_INT_STAT);
			if (DAR_POREG_BAD_IDX == t_idx) {
				assert_true(false);
				return;
			}
			dev_info->poregs[t_idx].data &= ~TSI721_EM_INT_STAT_RCS;

			t_idx = tsi721_get_poreg_idx(dev_info,
			TSI721_EM_PW_STAT);
			if (DAR_POREG_BAD_IDX == t_idx) {
				assert_true(false);
				return;
			}
			dev_info->poregs[t_idx].data &= ~TSI721_EM_PW_STAT_RCS;
		}

		break;
	case TSI721_I2C_INT_STAT:
		dev_info->poregs[idx].data &= ~writedata;
		break;

	default:
		dev_info->poregs[idx].data = writedata;
		break;
	}
}

static uint32_t Tsi721WriteReg(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t writedata)
{
	uint32_t rc = 0xFFFFFFFF;

	if (NULL == dev_info) {
		return rc;
	}

	if (!st.real_hw) {
		tsi721_emulate_reg_write(dev_info, offset, writedata);
		return RIO_SUCCESS;
	}

	assert_true(st.mp_h_valid);
	if (0xFF == st.hc) {
		rc = rio_lcfg_write(st.mp_h, offset, 4, writedata);
	} else {
		rc = rio_maint_write(st.mp_h, st.did_reg_val, st.hc,
				offset, 4, writedata);
	}

	return rc;
}

static void Tsi721WaitSec(uint32_t delay_nsec, uint32_t delay_sec)
{
	if (st.real_hw) {
		uint64_t counter = delay_nsec
				+ ((uint64_t)delay_sec * 1000000000);
		for (; counter; counter--)
			;
	}
}

void tsi721_init_mock_reg(void **state)
{
	// idx is always should be less than UPB_DAR_REG.
	uint32_t cntr, idx;
	Tsi721_test_state_t *l_st = (Tsi721_test_state_t *)*state;

	DAR_proc_ptr_init(Tsi721ReadReg, Tsi721WriteReg, Tsi721WaitSec);
	if (l_st->real_hw) {
		mock_dev_info.poregs_max = 0;
		mock_dev_info.poreg_cnt = 0;
		mock_dev_info.poregs = NULL;
	} else {
		mock_dev_info.poregs_max = UPB_DAR_REG;
		mock_dev_info.poreg_cnt = 0;
		mock_dev_info.poregs = mock_dar_reg;
		for (idx = 0; idx < TSI721_NUM_MOCK_REGS; idx++) {
			assert_int_equal(RIO_SUCCESS,
					tsi721_add_poreg(&mock_dev_info,
							tsi721_regs[idx], 0));
		}
		// Ensure Tsi721 port is powered up and available
		assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info, TSI721_DEVCTL, TSI721_DEVCTL_SRBOOT_CMPL | TSI721_DEVCTL_PCBOOT_CMPL));

		// initialize performance counters
		for (cntr = 0; cntr < TSI721_NUM_PERF_CTRS; cntr++) {
			if (tsi721_dev_ctrs[cntr].split
					&& !tsi721_dev_ctrs[cntr].os) {
				continue;
			}
			assert_int_equal(RIO_SUCCESS,
					tsi721_add_poreg(&mock_dev_info,
							tsi721_dev_ctrs[cntr].os,
							0));
		}
	}

	// Set base device ID...
	DARRegWrite(&mock_dev_info, MOCK_REG_ADDR(TSI721_BASE_ID),
			(TSI721_TEST_DEV08_ID << 16) | TSI721_TEST_DEV16_ID);
}

static int tsi721_grp_setup(void **state)
{
	*state = (void *)&st;
	char *token_list = (char *)"-m -h -d ";
	char *tok, *parm;
	int tok_idx = 1;
	bool got_hc = false;
	bool got_did_reg_val = false;

	while (tok_idx < st.argc) {
		tok = st.argv[tok_idx];
		tok_idx++;
		if (!(tok_idx < st.argc)) {
			printf("\nMissing option value.\n");
			goto fail;
		}
		parm = st.argv[tok_idx++];
		switch (parm_idx(tok, token_list)) {
		case 0:
			if (tok_parse_mport_id(parm, &st.mport, 0)) {
				printf("\nFailed tok_parse_mport_id\n");
				goto fail;
			}
			st.real_hw = true;
			if (!got_hc) {
				st.hc = 0xFF;
			}
			break;
		case 1:
			if (tok_parse_hc(parm, &st.hc, 0)) {
				printf("\nFailed tok_parse_hc\n");
				goto fail;
			}
			st.real_hw = true;
			got_hc = true;
			break;
			break;
		case 2:
			if (tok_parse_did(parm, &st.did_reg_val, 0)) {
				printf("\nFailed tok_parse_did\n");
				goto fail;
			}
			st.real_hw = true;
			got_did_reg_val = true;
			break;
		default:
			printf("\nUnknown option\n");
			goto fail;
			break;
		}
	}

	if ((got_hc || got_did_reg_val) && !(got_hc && got_did_reg_val)) {
		printf("\nMust enter both -h and -d, or none of them.\n");
		goto fail;
	}

	if (st.real_hw) {
		st.mp_h = rio_mport_open(st.mport, 0);
		if (st.mp_h  < 0) {
			printf("\nCould not open mport %d\n", st.mport);
			goto fail;
		}
		st.mp_h_valid = true;
	} else {
		// Always check that interrupt support works on
		// virtual hardware.
		tsi721_int_supported = true;
	}

	return 0;

fail:
	printf("\nSyntax:"
		"\n-m <mport #>"
		"\n-h <hopcount>"
		"\n-d <destID>"
		"\nSpecify -m <mport #> to test local Tsi721"
		"\nSpecify -m, -h and -d to test remote Tsi721\n\n");

	return -1;
}

static int tsi721_grp_teardown(void **state)
{
	if (st.real_hw) {
		if (st.mp_h_valid) {
			close(st.mp_h);
			st.mp_h = -1;
		}
	}

	return 0;
	(void)state;
}
// END common Tsi721 emulation setup

static int tsi721_em_grp_setup(void **state)
{
	return tsi721_grp_setup(state);
}

static int tsi721_em_grp_teardown(void **state)
{
	return tsi721_grp_teardown(state);
}

/* The setup function which should be called before any unit tests that need to be executed.
 */
static int tsi721_em_setup(void **state)
{
	memset(&mock_dev_info, 0, sizeof(mock_dev_info));
	tsi721_init_mock_dev_info();
	tsi721_init_mock_reg(state);

	(void)state; // unused
	return 0;
}

static void tsi721_em_cfg_pw_success_test(void **state)
{
	rio_em_cfg_pw_in_t in_parms;
	rio_em_cfg_pw_out_t out_parms;
	did_reg_t targ_id = 0x1234;
	uint32_t chkdata;

	// Test for dev16 destIDs...
	in_parms.imp_rc = 0xFFFFFFFF;
	in_parms.deviceID_tt = tt_dev16;
	in_parms.port_write_destID = targ_id;
	in_parms.srcID_valid = false;
	in_parms.port_write_srcID = 0;
	in_parms.priority = 1;
	in_parms.CRF = false;
	in_parms.port_write_re_tx = RIO_EM_TSI721_PW_RE_TX_103US;
	memset(&out_parms, 0, sizeof(out_parms));
	in_parms.imp_rc = 0xFFFFFFFF;

	assert_int_equal(RIO_SUCCESS,
			tsi721_rio_em_cfg_pw(&mock_dev_info, &in_parms,
					&out_parms));
	assert_int_equal(0, out_parms.imp_rc);
	assert_int_equal(tt_dev16, out_parms.deviceID_tt);
	assert_int_equal(targ_id, out_parms.port_write_destID);
	assert_true(out_parms.srcID_valid);
	assert_int_equal((did_reg_t)TSI721_TEST_DEV16_ID, out_parms.port_write_srcID);
	assert_int_equal(3, out_parms.priority);
	assert_true(out_parms.CRF);
	assert_int_equal(RIO_EM_TSI721_PW_RE_TX_103US,
			in_parms.port_write_re_tx);

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_PW_TGT_ID, &chkdata));
	assert_int_equal(chkdata, (targ_id << 16) | TSI721_PW_TGT_ID_LRG_TRANS);

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_PW_CTL, &chkdata));
	assert_int_equal(chkdata, TSI721_PW_CTL_PW_TIMER_103US);

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_PW_ROUTE, &chkdata));
	assert_int_equal(chkdata, TSI721_PW_ROUTE_PORT);

	// Test for dev8 destIDs...
	in_parms.imp_rc = 0xFFFFFFFF;
	in_parms.deviceID_tt = tt_dev8;
	in_parms.port_write_destID = targ_id;
	in_parms.srcID_valid = false;
	in_parms.port_write_srcID = 0;
	in_parms.priority = 1;
	in_parms.CRF = false;
	in_parms.port_write_re_tx = RIO_EM_TSI721_PW_RE_TX_820US;
	memset(&out_parms, 0, sizeof(out_parms));
	in_parms.imp_rc = 0xFFFFFFFF;

	assert_int_equal(RIO_SUCCESS,
			tsi721_rio_em_cfg_pw(&mock_dev_info, &in_parms,
					&out_parms));
	assert_int_equal(0, out_parms.imp_rc);
	assert_int_equal(tt_dev8, out_parms.deviceID_tt);
	assert_int_equal((did_reg_t)(targ_id & 0xFF), out_parms.port_write_destID);
	assert_true(out_parms.srcID_valid);
	assert_int_equal((did_reg_t)TSI721_TEST_DEV08_ID, out_parms.port_write_srcID);
	assert_int_equal(3, out_parms.priority);
	assert_true(out_parms.CRF);
	assert_int_equal(RIO_EM_TSI721_PW_RE_TX_820US,
			in_parms.port_write_re_tx);

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_PW_TGT_ID, &chkdata));
	assert_int_equal(chkdata, (targ_id << 16) & TSI721_PW_TGT_ID_PW_TGT_ID);

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_PW_CTL, &chkdata));
	assert_int_equal(chkdata, TSI721_PW_CTL_PW_TIMER_820US);

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_PW_ROUTE, &chkdata));
	assert_int_equal(chkdata, TSI721_PW_ROUTE_PORT);
	(void)state; // unused
}

static void tsi721_em_cfg_pw_bad_parms_test(void **state)
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
	in_parms.port_write_re_tx = RIO_EM_TSI721_PW_RE_TX_103US;
	memset(&out_parms, 0, sizeof(out_parms));

	in_parms.deviceID_tt = tt_dev16;
	in_parms.priority = 4;
	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_cfg_pw(&mock_dev_info, &in_parms,
					&out_parms));
	assert_int_not_equal(0, out_parms.imp_rc);

	(void)state; // unused
}

typedef struct tsi721_pw_retx_info_t_TAG {
	uint32_t timer_val_in;
	uint32_t timer_val_out;
	uint32_t reg_val_out;
} tsi721_pw_retx_info_t;

// Verify the Tsi721 exact values for port-write retransmission
static void tsi721_rio_em_cfg_pw_retx_compute_test(void **state)
{
	rio_em_cfg_pw_in_t in_p;
	rio_em_cfg_pw_out_t out_p;
	did_reg_t targ_id = 0x1234;
	uint32_t chkdata;
	const tsi721_pw_retx_info_t tests[] = {{1,
	RIO_EM_TSI721_PW_RE_TX_103US,
	TSI721_PW_CTL_PW_TIMER_103US}, {RIO_EM_TSI721_PW_RE_TX_103US + 1,
	RIO_EM_TSI721_PW_RE_TX_205US,
	TSI721_PW_CTL_PW_TIMER_205US}, {RIO_EM_TSI721_PW_RE_TX_205US + 1,
	RIO_EM_TSI721_PW_RE_TX_410US,
	TSI721_PW_CTL_PW_TIMER_410US}, {RIO_EM_TSI721_PW_RE_TX_410US + 1,
	RIO_EM_TSI721_PW_RE_TX_820US,
	TSI721_PW_CTL_PW_TIMER_820US}, {RIO_EM_TSI721_PW_RE_TX_820US + 1,
	RIO_EM_TSI721_PW_RE_TX_820US,
	TSI721_PW_CTL_PW_TIMER_820US}, {0x00FFFFFF,
	RIO_EM_TSI721_PW_RE_TX_820US,
	TSI721_PW_CTL_PW_TIMER_820US}, };
	const int num_tests = sizeof(tests) / sizeof(tests[0]);
	int i;

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
				tsi721_rio_em_cfg_pw(&mock_dev_info, &in_p,
						&out_p));

		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(tt_dev16, out_p.deviceID_tt);
		assert_int_equal(targ_id, out_p.port_write_destID);
		assert_true(out_p.srcID_valid);
		assert_int_equal((did_reg_t)TSI721_TEST_DEV16_ID, out_p.port_write_srcID);
		assert_int_equal(3, out_p.priority);
		assert_true(out_p.CRF);
		assert_int_equal(tests[i].timer_val_out,
				out_p.port_write_re_tx);

		assert_int_equal(RIO_SUCCESS,
				DARRegRead(&mock_dev_info, TSI721_PW_CTL, &chkdata));
		assert_int_equal(chkdata, tests[i].reg_val_out);
	}

	(void)state; // unused
}

static void chk_plm_event_enables(rio_em_notfn_ctl_t ntfn, uint32_t event_mask)
{
	uint32_t plm_int_en;
	uint32_t plm_pw_en;
	bool ints;
	bool pws;

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_PLM_INT_ENABLE, &plm_int_en));
	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_PLM_PW_ENABLE, &plm_pw_en));
	ints = (plm_int_en & event_mask);
	pws = (plm_pw_en & event_mask);

	switch (ntfn) {
	case rio_em_notfn_none:
		if (tsi721_int_supported) {
			assert_false(ints);
		}
		assert_false(pws);
		break;
	case rio_em_notfn_int:
		if (tsi721_int_supported) {
			assert_true(ints);
		}
		assert_false(pws);
		break;
	case rio_em_notfn_pw:
		if (tsi721_int_supported) {
			assert_false(ints);
		}
		assert_true(pws);
		break;
	case rio_em_notfn_both:
		if (tsi721_int_supported) {
			assert_true(ints);
		}
		assert_true(pws);
		break;
	case rio_em_notfn_0delta:
	case rio_em_notfn_last:
	default:
		assert_true(false);
		break;
	}
}

static void chk_regs_los(rio_em_cfg_t *event)
{
	uint32_t plm_ctl;
	uint32_t temp;

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_ctl));

	plm_ctl &= TSI721_PLM_IMP_SPEC_CTL_DLT_THRESH;

	switch (event->em_detect) {
	case rio_em_detect_off:
		assert_int_equal(0, plm_ctl);
		break;
	case rio_em_detect_on:
		temp = event->em_info / 1000 / 256;
		if (!temp && event->em_info) {
			temp = 1;
		}
		assert_int_equal(temp, plm_ctl);
		break;
	case rio_em_detect_0delta:
	case rio_em_detect_last:
	default:
		assert_true(false);
	}
}

static void chk_regs_2many_retx(rio_em_cfg_t *event)
{
	uint32_t plm_denial_ctl;
	uint32_t mask = TSI721_PLM_DENIAL_CTL_CNT_RTY;
	uint32_t thresh;

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, TSI721_PLM_DENIAL_CTL,
							&plm_denial_ctl));
	thresh = plm_denial_ctl & TSI721_PLM_DENIAL_CTL_DENIAL_THRESH;

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

static void chk_regs_2many_pna(rio_em_cfg_t *event)
{
	uint32_t plm_denial_ctl;
	uint32_t mask = TSI721_PLM_DENIAL_CTL_CNT_PNA;
	uint32_t thresh;

	assert_int_equal(RIO_SUCCESS,
		DARRegRead(&mock_dev_info, TSI721_PLM_DENIAL_CTL,
							&plm_denial_ctl));
	thresh = plm_denial_ctl & TSI721_PLM_DENIAL_CTL_DENIAL_THRESH;

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

static void chk_regs_err_rate(rio_em_cfg_t *event)
{
	uint32_t sp_ctl;
	uint32_t rate_en;
	uint32_t err_rate;
	uint32_t err_thr;
	uint32_t info;
	uint32_t ctl_mask = TSI721_SP_CTL_DROP_EN | TSI721_SP_CTL_STOP_FAIL_EN;
	uint32_t em_rate_en;
	uint32_t em_err_rate;
	uint32_t em_err_thr;

	assert_int_equal(RIO_SUCCESS,
			rio_em_get_f_err_rate_info(event->em_info, &em_rate_en,
					&em_err_rate, &em_err_thr));
	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_SP_RATE_EN, &rate_en));
	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_SP_ERR_RATE, &err_rate));
	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_SP_ERR_THRESH, &err_thr));
	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_SP_CTL, &sp_ctl));

	if (rio_em_detect_on == event->em_detect) {
		assert_int_equal(em_rate_en,
				rate_en & ~TSI721_ERR_RATE_EVENT_EXCLUSIONS);
		assert_int_equal(em_err_rate, err_rate);
		assert_int_equal(em_err_thr,
				err_thr & TSI721_SP_ERR_THRESH_ERR_RFT);
		assert_int_equal(ctl_mask, sp_ctl & ctl_mask);
		assert_int_equal(RIO_SUCCESS,
				rio_em_compute_f_err_rate_info(rate_en,
						err_rate, err_thr, &info));
		assert_int_equal(info, event->em_info);
	} else {
		assert_int_equal(0,
				rate_en & ~TSI721_ERR_RATE_EVENT_EXCLUSIONS);
	}
}

static void chk_regs_log(rio_em_cfg_t *event)
{
	uint32_t log_err_en;
	uint32_t mask = TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
	TSI721_LOCAL_ERR_EN_ILL_ID_EN;

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_LOCAL_ERR_EN, &log_err_en));

	if (rio_em_detect_on == event->em_detect) {
		assert_int_equal(mask, log_err_en & mask);
	} else {
		assert_int_equal(0, log_err_en & mask);
	}
}

static void chk_regs_sig_det(rio_em_cfg_t *event)
{
	uint32_t sp_ctl;
	uint32_t mask = TSI721_SP_CTL_PORT_LOCKOUT;

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_SP_CTL, &sp_ctl));

	if (rio_em_detect_on == event->em_detect) {
		assert_int_equal(mask, sp_ctl & mask);
	} else {
		assert_int_equal(0, sp_ctl & mask);
	}
}

static void chk_regs_rst_req(rio_em_cfg_t *event)
{
	uint32_t plm_ctl;
	uint32_t mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_ctl));

	if (rio_em_detect_on == event->em_detect) {
		assert_int_equal(0, plm_ctl & mask);
	} else {
		assert_int_equal(mask, plm_ctl & mask);
	}
}

static void chk_regs_init_fail(rio_em_cfg_t *event)
{
	uint32_t i2c_i_en;
	uint32_t mask = TSI721_I2C_INT_ENABLE_BL_FAIL;

	assert_int_equal(RIO_SUCCESS,
			DARRegRead(&mock_dev_info, TSI721_I2C_INT_ENABLE, &i2c_i_en));

	if ((rio_em_detect_on == event->em_detect) && tsi721_int_supported) {
		assert_int_equal(mask, i2c_i_en & mask);
	} else {
		assert_int_equal(0, i2c_i_en & mask);
	}
}

static void tsi721_rio_em_cfg_set_get_chk_regs(rio_em_cfg_t *event,
		rio_em_notfn_ctl_t ntfn)
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
		chk_regs_los(event);
		chk_plm_en = true;
		plm_mask = TSI721_PLM_STATUS_DLT;
		break;
	case rio_em_f_port_err:
		chk_plm_en = true;
		plm_mask = TSI721_PLM_STATUS_PORT_ERR;
		break;
	case rio_em_f_2many_retx:
		chk_regs_2many_retx(event);
		chk_plm_en = true;
		plm_mask = TSI721_PLM_STATUS_MAX_DENIAL;
		break;
	case rio_em_f_2many_pna:
		chk_regs_2many_pna(event);
		chk_plm_en = true;
		plm_mask = TSI721_PLM_STATUS_MAX_DENIAL;
		break;
	case rio_em_f_err_rate:
		chk_regs_err_rate(event);
		chk_plm_en = true;
		plm_mask = TSI721_PLM_STATUS_OUTPUT_FAIL;
		break;
	case rio_em_d_ttl:
		break;
	case rio_em_d_rte:
		break;
	case rio_em_d_log:
		chk_regs_log(event);
		break;
	case rio_em_i_sig_det:
		chk_regs_sig_det(event);
		chk_plm_en = true;
		plm_mask = TSI721_PLM_STATUS_LINK_INIT;
		break;
	case rio_em_i_rst_req:
		chk_regs_rst_req(event);
		break;
	case rio_em_i_init_fail:
		chk_regs_init_fail(event);
		break;
	case rio_em_a_clr_pwpnd:
		break;
	case rio_em_a_no_event:
		break;
	default:
		assert_true(false);
	}

	if (chk_plm_en) {
		chk_plm_event_enables(ntfn, plm_mask);
	}
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

static void tsi721_rio_em_cfg_set_success_em_info_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_cfg_get_in_t get_cfg_in;
	rio_em_cfg_get_out_t get_cfg_out;
	rio_em_notfn_ctl_t set_chk_notfn =
			(tsi721_int_supported) ?
					rio_em_notfn_both : rio_em_notfn_pw;
	uint32_t chk_em_info;
	rio_em_events_t get_cfg_event_req;
	rio_em_cfg_t get_cfg_event_info;

	rio_em_cfg_t events[] = {
		{rio_em_f_los, rio_em_detect_on, 1 * 256 * 1000}, 
		{rio_em_f_los, rio_em_detect_on, 2 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_on, 3 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_on, 4 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_on, 5 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_on, 6 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_on, 7 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_on, 9 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_on, 100 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_on, 200 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_on, 300 * 256 * 1000}, // 10
		{rio_em_f_port_err, rio_em_detect_on, 0},
		{rio_em_f_2many_retx, rio_em_detect_on, 0x00ff},
		{rio_em_f_2many_retx, rio_em_detect_on, 0x0001},
		{rio_em_f_2many_retx, rio_em_detect_on, 0x0088},
		{rio_em_f_2many_pna, rio_em_detect_on, 0x0001}, // 15
		{rio_em_f_2many_pna, rio_em_detect_on, 0x00FE},
		{rio_em_f_2many_pna, rio_em_detect_on, 0x00FF},
		{rio_em_f_2many_pna, rio_em_detect_on, 0x009E},
		{rio_em_d_log, rio_em_detect_on,
					TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
					TSI721_LOCAL_ERR_EN_ILL_ID_EN},
		{rio_em_i_sig_det, rio_em_detect_on, 0}, // 20
		{rio_em_i_rst_req, rio_em_detect_on, 0},
		{rio_em_i_init_fail, rio_em_detect_on, 0},
		{rio_em_f_los, rio_em_detect_off, 1 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_off, 2 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_off, 3 * 256 * 1000}, // 25
		{rio_em_f_los, rio_em_detect_off, 4 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_off, 5 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_off, 6 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_off, 7 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_off, 9 * 256 * 1000}, // 30
		{rio_em_f_los, rio_em_detect_off, 100 * 256 * 1000},
		{rio_em_f_los, rio_em_detect_off, 200 * 256 * 1000},
		{ rio_em_f_los, rio_em_detect_off, 300 * 256 * 1000},
		{rio_em_f_port_err, rio_em_detect_off, 0},
		{rio_em_f_2many_retx, rio_em_detect_off, 0x0000}, // 35
		{rio_em_f_2many_retx, rio_em_detect_off, 0x0000},
		{rio_em_f_2many_retx, rio_em_detect_off, 0x0000},
		{rio_em_f_2many_pna, rio_em_detect_off, 0x0000},
		{rio_em_f_2many_pna, rio_em_detect_off, 0x0000},
		{rio_em_f_2many_pna, rio_em_detect_off, 0x0000}, // 40
		{rio_em_f_2many_pna, rio_em_detect_off, 0x0000},
		{rio_em_d_log, rio_em_detect_off, 0},
		{rio_em_i_sig_det, rio_em_detect_off, 0},
		{rio_em_i_rst_req, rio_em_detect_off, 0},
		{rio_em_a_clr_pwpnd, rio_em_detect_off, 0}, // 45
		{rio_em_d_ttl, rio_em_detect_off, 0},
		{rio_em_d_rte, rio_em_detect_off, 0},
		{rio_em_a_no_event, rio_em_detect_off, 0},
		{rio_em_i_init_fail, rio_em_detect_off, 0},
	};

	unsigned int num_events = sizeof(events) / sizeof(events[0]);
	unsigned int i;
	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	for (i = 0; i < num_events; i++) {
		if (DEBUG_PRINTF) {
			printf("\nevent idx %d\n",  i);
		}
		if (rio_em_i_rst_req == events[i].em_event) {
			// If we're testing disabling the Reset Request
			// event, do the real disable since this events
			// detection is actually controlled by Port Config
			// functionality.

			assert_int_equal(RIO_SUCCESS,
				DARRegRead(&mock_dev_info,
					TSI721_PLM_IMP_SPEC_CTL,
					&plm_imp_spec_ctl));
			if (rio_em_detect_off == events[i].em_detect) {
				plm_imp_spec_ctl |= t_mask;
			} else {
				plm_imp_spec_ctl &= ~t_mask;
			}
			assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info,
					TSI721_PLM_IMP_SPEC_CTL,
					plm_imp_spec_ctl));
		}
		set_chk_notfn = (tsi721_int_supported) ?
				rio_em_notfn_both : rio_em_notfn_pw;
		set_chk_notfn = (events[i].em_detect == rio_em_detect_on) ?
				set_chk_notfn : rio_em_notfn_none;

		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = 0;
		set_cfg_in.notfn =
				(events[i].em_detect == rio_em_detect_on) ?
						rio_em_notfn_both :
						rio_em_notfn_none;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &events[i];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(set_chk_notfn, set_cfg_out.notfn);

		tsi721_rio_em_cfg_set_get_chk_regs(&events[i], set_chk_notfn);

		get_cfg_in.port_num = 0;
		get_cfg_in.num_events = 1;
		get_cfg_in.event_list = &get_cfg_event_req;
		get_cfg_event_req = events[i].em_event;
		get_cfg_in.events = &get_cfg_event_info;

		get_cfg_out.imp_rc = 0xFFFFFFFF;
		get_cfg_out.fail_idx = 0xFF;
		get_cfg_out.notfn = (rio_em_notfn_ctl_t)0xFF;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_cfg_get(&mock_dev_info,
						&get_cfg_in, &get_cfg_out));
		assert_int_equal(0, get_cfg_out.imp_rc);
		assert_int_equal(rio_em_last, get_cfg_out.fail_idx);
		assert_int_equal(set_chk_notfn, get_cfg_out.notfn);

		if (rio_em_detect_on == events[i].em_detect) {
			chk_em_info = events[i].em_info;
		} else {
			chk_em_info = 0;
		}
		assert_int_equal(events[i].em_event,
				get_cfg_in.events[0].em_event);
		if ((rio_em_i_init_fail == events[i].em_event)
				&& (rio_em_detect_on == events[i].em_detect)
				&& !tsi721_int_supported) {
			assert_int_equal(rio_em_detect_off,
					get_cfg_in.events[0].em_detect);
		} else {
			assert_int_equal(events[i].em_detect,
					get_cfg_in.events[0].em_detect);
		}
		assert_int_equal(chk_em_info, get_cfg_in.events[0].em_info);
	}
	(void)state;
}

// Check that some events that should be ignored can be successfully
// configured.

static void tsi721_rio_em_cfg_set_ignore_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_cfg_get_in_t get_cfg_in;
	rio_em_cfg_get_out_t get_cfg_out;
	rio_em_notfn_ctl_t set_chk_notfn =
			(tsi721_int_supported) ?
					rio_em_notfn_both : rio_em_notfn_pw;
	rio_em_events_t get_cfg_event_req;
	rio_em_cfg_t get_cfg_event_info;

	rio_em_cfg_t events[] = {
		{rio_em_d_ttl, rio_em_detect_on, 0},
		{rio_em_d_rte, rio_em_detect_on, 0},
		{rio_em_a_no_event, rio_em_detect_on, 0},
	};

	unsigned int num_events = sizeof(events) / sizeof(events[0]);
	unsigned int i;

	for (i = 0; i < num_events; i++) {
		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = 0;
		set_cfg_in.notfn = rio_em_notfn_both;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &events[i];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(set_chk_notfn, set_cfg_out.notfn);

		get_cfg_in.port_num = 0;
		get_cfg_in.num_events = 1;
		get_cfg_in.event_list = &get_cfg_event_req;
		get_cfg_event_req = events[i].em_event;
		get_cfg_in.events = &get_cfg_event_info;

		get_cfg_out.imp_rc = 0xFFFFFFFF;
		get_cfg_out.fail_idx = 0xFF;
		get_cfg_out.notfn = (rio_em_notfn_ctl_t)0xFF;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_cfg_get(&mock_dev_info,
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

// Check that an attempt to configure unsupported error rate events
// is ignored.  Descrambling loss only applies to IDLE2, so it should
// never be set.  Implementation specific errors cannot be set, as there
// is not way to control which errors get counted.

static void tsi721_rio_em_cfg_set_err_rate_unsup_test(void **state)
{
	err_rate_checks_t events[] = {
		{TSI721_SP_RATE_EN_DSCRAM_LOS_EN, 
			RIO_EMHS_SPX_RATE_RB_NONE | RIO_EMHS_SPX_RATE_RR_LIM_2,
			0x01000000,
			{rio_em_f_err_rate, rio_em_detect_on, 0}},
		{TSI721_SP_RATE_EN_IMP_SPEC_EN,
			RIO_EMHS_SPX_RATE_RB_10_MS | RIO_EMHS_SPX_RATE_RR_LIM_16,
			0xFFAAAAAA,
			{rio_em_f_err_rate, rio_em_detect_on, 0}}
	};

	unsigned int num_events = sizeof(events) / sizeof(events[0]);
	unsigned int i;

	for (i = 0; i < num_events; i++) {
		assert_int_not_equal(RIO_SUCCESS,
				rio_em_compute_f_err_rate_info(
						events[i].sp_rate_en,
						events[i].sp_rate,
						events[i].sp_thresh,
						&events[i].event.em_info));
	}
	(void)state;
}
//
// Check that various error rate, error leak rates, and error count maximums
// are supported correctly.

static void tsi721_rio_em_cfg_set_err_rate_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_cfg_get_in_t get_cfg_in;
	rio_em_cfg_get_out_t get_cfg_out;
	rio_em_notfn_ctl_t set_chk_notfn =
			(tsi721_int_supported) ?
					rio_em_notfn_both : rio_em_notfn_pw;
	rio_em_events_t get_cfg_event_req;
	rio_em_cfg_t get_cfg_event_info;

	err_rate_checks_t events[] =
			{{TSI721_SP_RATE_EN_LINK_TO_EN,
			RIO_EMHS_SPX_RATE_RB_NONE | RIO_EMHS_SPX_RATE_RR_LIM_2,
					0x01000000, {rio_em_f_err_rate,
							rio_em_detect_on, 0}},
					{TSI721_SP_RATE_EN_DELIN_ERR_E,
					RIO_EMHS_SPX_RATE_RB_10_MS |
					RIO_EMHS_SPX_RATE_RR_LIM_16, 0xFFAAAAAA,
							{rio_em_f_err_rate,
									rio_em_detect_on,
									0}},
					{TSI721_SP_RATE_EN_CS_ACK_ILL_EN,
					RIO_EMHS_SPX_RATE_RB_1_MS |
					RIO_EMHS_SPX_RATE_RR_LIM_4, 0x10999999,
							{rio_em_f_err_rate,
									rio_em_detect_on,
									0}},
					{TSI721_SP_RATE_EN_PROT_ERR_EN,
					RIO_EMHS_SPX_RATE_RB_100_MS |
					RIO_EMHS_SPX_RATE_RR_LIM_NONE,
							0x30AAAAAA,
							{rio_em_f_err_rate,
									rio_em_detect_on,
									0}},
					{TSI721_SP_RATE_EN_LR_ACKID_ILL_EN,
					RIO_EMHS_SPX_RATE_RB_1_SEC |
					RIO_EMHS_SPX_RATE_RR_LIM_NONE,
							0x40BBBBBB,
							{rio_em_f_err_rate,
									rio_em_detect_on,
									0}},
					{TSI721_SP_RATE_EN_LR_ACKID_ILL_EN |
					TSI721_SP_RATE_EN_LINK_TO_EN |
					TSI721_SP_RATE_EN_DELIN_ERR_E |
					TSI721_SP_RATE_EN_CS_ACK_ILL_EN |
					TSI721_SP_RATE_EN_PROT_ERR_EN,
					RIO_EMHS_SPX_RATE_RB_10_SEC |
					RIO_EMHS_SPX_RATE_RR_LIM_NONE,
							0x50000000,
							{rio_em_f_err_rate,
									rio_em_detect_on,
									0}},
					{TSI721_SP_RATE_EN_PKT_ILL_SIZE_EN,
					RIO_EMHS_SPX_RATE_RB_100_SEC |
					RIO_EMHS_SPX_RATE_RR_LIM_NONE,
							0x01000000,
							{rio_em_f_err_rate,
									rio_em_detect_on,
									0}},
					{TSI721_SP_RATE_EN_PKT_CRC_ERR_EN,
					RIO_EMHS_SPX_RATE_RB_1000_SEC |
					RIO_EMHS_SPX_RATE_RR_LIM_NONE,
							0x01000000,
							{rio_em_f_err_rate,
									rio_em_detect_on,
									0}},
					{TSI721_SP_RATE_EN_PKT_ILL_ACKID_EN,
					RIO_EMHS_SPX_RATE_RB_10000_SEC |
					RIO_EMHS_SPX_RATE_RR_LIM_NONE,
							0x01000000,
							{rio_em_f_err_rate,
									rio_em_detect_on,
									0}},
					{TSI721_SP_RATE_EN_CS_ILL_ID_EN,
					RIO_EMHS_SPX_RATE_RB_NONE |
					RIO_EMHS_SPX_RATE_RR_LIM_NONE,
							0x01000000,
							{rio_em_f_err_rate,
									rio_em_detect_on,
									0}},
					{TSI721_SP_RATE_EN_CS_CRC_ERR_EN,
					RIO_EMHS_SPX_RATE_RB_NONE |
					RIO_EMHS_SPX_RATE_RR_LIM_NONE,
							0x010CCCCC,
							{rio_em_f_err_rate,
									rio_em_detect_on,
									0}},
					{TSI721_SP_RATE_EN_LR_ACKID_ILL_EN |
					TSI721_SP_RATE_EN_LINK_TO_EN |
					TSI721_SP_RATE_EN_DELIN_ERR_E |
					TSI721_SP_RATE_EN_CS_ACK_ILL_EN |
					TSI721_SP_RATE_EN_PROT_ERR_EN |
					TSI721_SP_RATE_EN_CS_CRC_ERR_EN |
					TSI721_SP_RATE_EN_CS_ILL_ID_EN |
					TSI721_SP_RATE_EN_PKT_ILL_ACKID_EN |
					TSI721_SP_RATE_EN_PKT_CRC_ERR_EN |
					TSI721_SP_RATE_EN_PKT_ILL_SIZE_EN,
					RIO_EMHS_SPX_RATE_RB_NONE |
					RIO_EMHS_SPX_RATE_RR_LIM_NONE,
							0x010DDDDD,
							{rio_em_f_err_rate,
									rio_em_detect_on,
									0}}};

	unsigned int num_events = sizeof(events) / sizeof(events[0]);
	unsigned int i;

	for (i = 0; i < num_events; i++) {
		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = 0;
		set_cfg_in.notfn = rio_em_notfn_both;
		set_cfg_in.num_events = 1;
		assert_int_equal(RIO_SUCCESS,
				rio_em_compute_f_err_rate_info(
						events[i].sp_rate_en,
						events[i].sp_rate,
						events[i].sp_thresh,
						&events[i].event.em_info));
		set_cfg_in.events = &events[i].event;

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xFF;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(set_chk_notfn, set_cfg_out.notfn);

		tsi721_rio_em_cfg_set_get_chk_regs(&events[i].event,
				set_chk_notfn);

		get_cfg_in.port_num = 0;
		get_cfg_in.num_events = 1;
		get_cfg_in.event_list = &get_cfg_event_req;
		get_cfg_event_req = events[i].event.em_event;
		get_cfg_in.events = &get_cfg_event_info;

		get_cfg_out.imp_rc = 0xFFFFFFFF;
		get_cfg_out.fail_idx = 0xFF;
		get_cfg_out.notfn = (rio_em_notfn_ctl_t)0xFF;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_cfg_get(&mock_dev_info,
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

// Test various illegal (generally 0) em_info parameters are detected
// and reported.

static void tsi721_rio_em_cfg_set_fail_em_info_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;

	rio_em_cfg_t events[] = {
		{rio_em_f_los, rio_em_detect_on, 0},
		{rio_em_f_2many_retx, rio_em_detect_on, 0},
		{rio_em_f_2many_pna, rio_em_detect_on, 0},
		{rio_em_f_err_rate, rio_em_detect_on, 0},
		{rio_em_d_log, rio_em_detect_on, 0},
		{rio_em_last, rio_em_detect_on, 0},
	};
	rio_em_cfg_t pass_events[2];

	unsigned int num_events = sizeof(events) / sizeof(events[0]);
	unsigned int i;

	pass_events[0].em_event = rio_em_a_no_event;
	pass_events[0].em_detect = rio_em_detect_0delta;
	pass_events[0].em_info = 0;

	for (i = 0; i < num_events; i++) {
		memcpy(&pass_events[1], &events[i], sizeof(pass_events[0]));
		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = 0;
		set_cfg_in.notfn = rio_em_notfn_both;
		set_cfg_in.num_events = 2;
		set_cfg_in.events = &pass_events[0];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xFF;

		assert_int_not_equal(RIO_SUCCESS,
				tsi721_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_not_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(0, set_cfg_out.fail_port_num);
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

static void tsi721_rio_em_cfg_set_roundup_test(void **state)
{
	rio_em_cfg_set_in_t set_cfg_in;
	rio_em_cfg_set_out_t set_cfg_out;
	rio_em_cfg_get_in_t get_cfg_in;
	rio_em_cfg_get_out_t get_cfg_out;
	rio_em_notfn_ctl_t set_chk_notfn =
			(tsi721_int_supported) ?
					rio_em_notfn_both : rio_em_notfn_pw;
	rio_em_events_t get_cfg_event_req;
	rio_em_cfg_t get_cfg_event_info;

	em_info_diff_checks_t events[] = {
		{0x000000ff,
			{rio_em_f_2many_retx, rio_em_detect_on, 0x0100}},
		{0x000000ff,
			{rio_em_f_2many_retx, rio_em_detect_on, 0xFFFF}},
		{0x000000ff,
			{rio_em_f_2many_pna, rio_em_detect_on, 0x0100}},
		{0x000000ff,
			{rio_em_f_2many_pna, rio_em_detect_on, 0xFFFF}},
		{1 * 256 * 1000,
			{rio_em_f_los, rio_em_detect_on, 1}},
		{1 * 256 * 1000,
			{rio_em_f_los, rio_em_detect_on, (1 * 256 * 1000) - 1}},
		{2 * 256 * 1000,
			{rio_em_f_los, rio_em_detect_on, (1 * 256 * 1000) + 1}},
		{7 * 256 * 1000,
			{rio_em_f_los, rio_em_detect_on, (6 * 256 * 1000) + 1}}
	};

	unsigned int num_events = sizeof(events) / sizeof(events[0]);
	unsigned int i;

	for (i = 0; i < num_events; i++) {
		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = 0;
		set_cfg_in.notfn = rio_em_notfn_both;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &events[i].event;

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xFF;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(set_chk_notfn, set_cfg_out.notfn);

		events[i].event.em_info = events[i].chk_info;
		tsi721_rio_em_cfg_set_get_chk_regs(&events[i].event,
				set_chk_notfn);

		get_cfg_in.port_num = 0;
		get_cfg_in.num_events = 1;
		get_cfg_in.event_list = &get_cfg_event_req;
		get_cfg_event_req = events[i].event.em_event;
		get_cfg_in.events = &get_cfg_event_info;
		get_cfg_out.imp_rc = 0xFFFFFFFF;
		get_cfg_out.fail_idx = 0xFF;
		get_cfg_out.notfn = (rio_em_notfn_ctl_t)0xFF;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_cfg_get(&mock_dev_info,
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

static void tsi721_rio_em_cfg_get_bad_parms_test(void **state)
{
	rio_em_cfg_get_in_t p_in;
	rio_em_cfg_get_out_t p_out;
	rio_em_events_t get_cfg_event_req;
	rio_em_cfg_t get_cfg_event_info;

	// Bad port number
	p_in.port_num = 1;
	p_in.num_events = 1;
	p_in.event_list = &get_cfg_event_req;
	get_cfg_event_req = rio_em_f_2many_pna;
	p_in.events = &get_cfg_event_info;

	p_out.imp_rc = 0;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_cfg_get(&mock_dev_info, &p_in, &p_out));
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
			tsi721_rio_em_cfg_get(&mock_dev_info, &p_in, &p_out));
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
			tsi721_rio_em_cfg_get(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(rio_em_last, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	// NULL events pointer
	p_in.event_list = &get_cfg_event_req;
	p_in.events = NULL;

	p_out.imp_rc = 0;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_cfg_get(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(rio_em_last, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	// Bad event in list   
	p_in.events = &get_cfg_event_info;
	p_in.event_list = &get_cfg_event_req;
	get_cfg_event_req = rio_em_last;
	p_in.events = &get_cfg_event_info;

	p_out.imp_rc = 0;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_cfg_get(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(0, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	(void)state;
}

// Test bad parameter values are detected and reported.

static void tsi721_rio_em_cfg_set_bad_parms_test(void **state)
{
	rio_em_cfg_set_in_t p_in;
	rio_em_cfg_set_out_t p_out;
	rio_em_cfg_t events = {rio_em_f_2many_pna, rio_em_detect_on, 0x0100};

	// Bad number of ports
	p_in.ptl.num_ports = 2;
	p_in.ptl.pnums[0] = 0;
	p_in.ptl.pnums[1] = 0;
	p_in.num_events = 1;
	p_in.events = &events;

	p_out.imp_rc = 0;
	p_out.fail_port_num = 0x99;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_cfg_set(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(0, p_out.fail_port_num);
	assert_int_equal(rio_em_last, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	// Bad pnum
	p_in.ptl.num_ports = 1;
	p_in.ptl.pnums[0] = 1;
	p_in.num_events = 1;
	p_in.events = &events;

	p_out.imp_rc = 0;
	p_out.fail_port_num = 0x99;
	p_out.fail_idx = 0xFF;
	p_out.notfn = (rio_em_notfn_ctl_t)0xFF;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_cfg_set(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(0, p_out.fail_port_num);
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
			tsi721_rio_em_cfg_set(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(0, p_out.fail_port_num);
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
			tsi721_rio_em_cfg_set(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(0, p_out.fail_port_num);
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
			tsi721_rio_em_cfg_set(&mock_dev_info, &p_in, &p_out));
	assert_int_not_equal(0, p_out.imp_rc);
	assert_int_equal(0, p_out.fail_port_num);
	assert_int_equal(rio_em_last, p_out.fail_idx);
	assert_int_equal(rio_em_notfn_0delta, p_out.notfn);

	(void)state;
}

static void tsi721_rio_em_dev_rpt_ctl_chk_regs(rio_em_notfn_ctl_t chk_notfn,
		tsi721_rpt_ctl_regs_t *regs)
{
	switch (chk_notfn) {
	case rio_em_notfn_none:
		if (tsi721_int_supported) {
			assert_int_equal(0, regs->plm_all_int_en);
			assert_int_equal(0, regs->em_dev_int_en);
		}
		assert_int_equal(0, regs->plm_all_pw_en);
		assert_int_equal(0, regs->em_dev_pw_en);
		break;

	case rio_em_notfn_int:
		assert_int_equal(TSI721_PLM_ALL_INT_EN_IRQ_EN,
				regs->plm_all_int_en);
		assert_int_equal(TSI721_EM_DEV_INT_EN_INT_EN,
				regs->em_dev_int_en);
		assert_int_equal(0, regs->plm_all_pw_en);
		assert_int_equal(0, regs->em_dev_pw_en);
		break;

	case rio_em_notfn_pw:
		if (tsi721_int_supported) {
			assert_int_equal(0, regs->plm_all_int_en);
			assert_int_equal(0, regs->em_dev_int_en);
		}
		assert_int_equal(TSI721_PLM_ALL_PW_EN_PW_EN,
				regs->plm_all_pw_en);
		assert_int_equal(TSI721_EM_DEV_PW_EN_PW_EN, regs->em_dev_pw_en);
		break;

	case rio_em_notfn_both:
		if (tsi721_int_supported) {
			assert_int_equal(TSI721_PLM_ALL_INT_EN_IRQ_EN,
					regs->plm_all_int_en);
			assert_int_equal(TSI721_EM_DEV_INT_EN_INT_EN,
					regs->em_dev_int_en);
		}
		assert_int_equal(TSI721_PLM_ALL_PW_EN_PW_EN,
				regs->plm_all_pw_en);
		assert_int_equal(TSI721_EM_DEV_PW_EN_PW_EN, regs->em_dev_pw_en);
		break;

	default:
		assert_true(false);
		break;
	}
}

// Test that port-write/interrupt reporting control is working.
// Note that this test works with and without interrupt support.

static void tsi721_rio_em_dev_rpt_ctl_success_test(void **state)
{
	rio_em_dev_rpt_ctl_in_t in_p;
	rio_em_dev_rpt_ctl_out_t out_p;
	rio_em_notfn_ctl_t notfn, chk_notfn;
	tsi721_rpt_ctl_regs_t regs;

	in_p.ptl.num_ports = 1;
	in_p.ptl.pnums[0] = 0;
	for (notfn = rio_em_notfn_none; notfn < rio_em_notfn_0delta; notfn =
			(rio_em_notfn_ctl_t)(notfn + 1)) {
		in_p.notfn = notfn;
		chk_notfn = notfn;
		if (!tsi721_int_supported) {
			switch (notfn) {
			case rio_em_notfn_int:
				chk_notfn = rio_em_notfn_none;
				break;
			case rio_em_notfn_both:
				chk_notfn = rio_em_notfn_pw;
				break;

			case rio_em_notfn_none:
			case rio_em_notfn_pw:
			default:
				break;
			}
		}
		out_p.imp_rc = 0xFFFFFFFF;
		out_p.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				rio_em_dev_rpt_ctl(&mock_dev_info, &in_p,
						&out_p));

		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(chk_notfn, out_p.notfn);

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_dev_rpt_ctl_reg_read(
						&mock_dev_info, &regs));
		tsi721_rio_em_dev_rpt_ctl_chk_regs(chk_notfn, &regs);

		// Repeat check with 0delta
		in_p.notfn = rio_em_notfn_0delta;
		out_p.imp_rc = 0xFFFFFFFF;
		out_p.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				rio_em_dev_rpt_ctl(&mock_dev_info, &in_p,
						&out_p));
		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(chk_notfn, out_p.notfn);

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_dev_rpt_ctl_reg_read(
						&mock_dev_info, &regs));
		tsi721_rio_em_dev_rpt_ctl_chk_regs(chk_notfn, &regs);
	}
	(void)state;
}

// Test bad parameter values are detected and reported.

static void tsi721_rio_em_dev_rpt_ctl_bad_parms_test(void **state)
{
	rio_em_dev_rpt_ctl_in_t in_p;
	rio_em_dev_rpt_ctl_out_t out_p;

	in_p.ptl.num_ports = 2;
	in_p.ptl.pnums[0] = 0;
	in_p.ptl.pnums[1] = 1;
	in_p.notfn = rio_em_notfn_none;

	out_p.imp_rc = 0;
	out_p.notfn = rio_em_notfn_last;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_dev_rpt_ctl(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(rio_em_notfn_last, out_p.notfn);

	in_p.ptl.num_ports = 1;
	in_p.ptl.pnums[0] = 1;
	in_p.notfn = rio_em_notfn_none;

	out_p.imp_rc = 0;
	out_p.notfn = rio_em_notfn_last;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_dev_rpt_ctl(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(rio_em_notfn_last, out_p.notfn);

	in_p.ptl.num_ports = RIO_ALL_PORTS;
	in_p.notfn = rio_em_notfn_last;

	out_p.imp_rc = 0;
	out_p.notfn = rio_em_notfn_last;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_dev_rpt_ctl(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(rio_em_notfn_last, out_p.notfn);

	in_p.ptl.num_ports = RIO_ALL_PORTS;
	in_p.notfn = (rio_em_notfn_ctl_t)((uint8_t)rio_em_notfn_last + 1);

	out_p.imp_rc = 0;
	out_p.notfn = rio_em_notfn_last;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_dev_rpt_ctl(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(rio_em_notfn_last, out_p.notfn);

	(void)state;
}

// Test that a port-write with no events is parsed correctly.

static void tsi721_rio_em_parse_pw_no_events_test(void **state)
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
			tsi721_rio_em_parse_pw(&mock_dev_info, &in_p, &out_p));

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

static void tsi721_rio_em_parse_pw_all_events_test(void **state)
{
	rio_em_parse_pw_in_t in_p;
	rio_em_parse_pw_out_t out_p;
	rio_em_event_n_loc_t events[(int)rio_em_last];
	parse_pw_info_t pws[] = {
		{{0, 0, TSI721_PW_RST_REQ, 0},
			{0, rio_em_i_rst_req}},
		{{0, TSI721_SP_ERR_DET_PKT_CRC_ERR, TSI721_PW_OUTPUT_FAIL, 0},
			{0, rio_em_f_err_rate}},
		{{0, 0, TSI721_PW_PORT_ERR, 0}, {0, rio_em_f_port_err}},
		{{0, 0, TSI721_PW_DLT, 0}, {0, rio_em_f_los}},
		{{0, 0, TSI721_PW_LINK_INIT, 0}, {0, rio_em_i_sig_det}},
		{{0, 0, TSI721_PW_LOCALOG, 0}, {0, rio_em_d_log}},
		{{0, 0, TSI721_PW_MAX_DENIAL, 0}, {0, rio_em_f_2many_retx}},
		{{0, TSI721_SP_ERR_DET_CS_NOT_ACC, TSI721_PW_MAX_DENIAL, 0},
						{0, rio_em_f_2many_pna}},
	};
	unsigned int num_pws = sizeof(pws) / sizeof(pws[0]);
	unsigned int i;

	// Try each event individually...
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
			tsi721_rio_em_parse_pw(&mock_dev_info, &in_p, &out_p));

		assert_int_equal(0, out_p.imp_rc);
		if (num_pws - 1 != i) {
			assert_int_equal(1, out_p.num_events);
			assert_int_equal(pws[i].event.event, in_p.events[0].event);
			assert_int_equal(pws[i].event.port_num,
					in_p.events[0].port_num);
			assert_false(out_p.too_many);
			assert_false(out_p.other_events);
			continue;
		}
		// Creation of a 2many_pna event also creates an
		// 2many_retx event.
		assert_int_equal(2, out_p.num_events);
		assert_int_equal(pws[i - 1].event.event, in_p.events[0].event);
		assert_int_equal(pws[i - 1].event.port_num,
				in_p.events[0].port_num);
		assert_int_equal(pws[i].event.event, in_p.events[1].event);
		assert_int_equal(pws[i].event.port_num,
				in_p.events[1].port_num);
		assert_false(out_p.too_many);
		assert_false(out_p.other_events);
	}

	(void)state;
}

// Test that a port-write with other, unsupported events is parsed correctly

static void tsi721_rio_em_parse_pw_oth_events_test(void **state)
{
	rio_em_parse_pw_in_t in_p;
	rio_em_parse_pw_out_t out_p;
	rio_em_event_n_loc_t events[(int)rio_em_last];
	parse_pw_info_t pws[] = {{{0, 0, TSI721_PLM_STATUS_OUTPUT_DEGR, 0}, {0,
			rio_em_f_los}}, {{0, 0, TSI721_PLM_STATUS_TLM_PW, 0}, {
			0, rio_em_f_los}}, {{0, 0, TSI721_PLM_STATUS_PBM_PW, 0},
			{0, rio_em_f_los}}, {{0, TSI721_SP_ERR_DET_DSCRAM_LOS,
			0, 0}, {0, rio_em_f_los}}, {{0,
			TSI721_SP_ERR_DET_IMP_SPEC, 0, 0}, {0, rio_em_f_los}}};
	unsigned int num_pws = sizeof(pws) / sizeof(pws[0]);
	unsigned int i;

	// Should not have any events, but should have "other" events
	for (i = 0; i < num_pws; i++) {
		memcpy(in_p.pw, pws[i].pw, sizeof(in_p.pw));
		in_p.num_events = (int)rio_em_last;
		memset(events, 0, sizeof(events));
		in_p.events = events;

		out_p.imp_rc = 0xFFFFFFFF;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_parse_pw(&mock_dev_info, &in_p,
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

static void tsi721_rio_em_parse_pw_bad_parms_test(void **state)
{
	rio_em_parse_pw_in_t in_p;
	rio_em_parse_pw_out_t out_p;
	rio_em_event_n_loc_t events[(int)rio_em_last];

	memset(in_p.pw, 0, sizeof(in_p.pw));
	in_p.num_events = (uint8_t)rio_em_last;
	memset(events, 0, sizeof(events));
	in_p.events = NULL;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_parse_pw(&mock_dev_info, &in_p, &out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	in_p.num_events = 0;
	in_p.events = events;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_parse_pw(&mock_dev_info, &in_p, &out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	in_p.num_events = (uint8_t)rio_em_last;
	in_p.pw[RIO_EM_PW_IMP_SPEC_IDX] = RIO_EM_PW_IMP_SPEC_PORT_MASK;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_parse_pw(&mock_dev_info, &in_p, &out_p));
	assert_int_not_equal(RIO_SUCCESS, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	(void)state;
}

typedef struct offset_value_event_t_TAG {
	uint32_t offset;
	uint32_t value;
	rio_em_event_n_loc_t event;
} offset_value_event_t;

// Test that bad parameters are detected and reported
// Also tests that "no_event" is ignored when creating events.

static void tsi721_rio_em_create_events_bad_parms_test(void **state)
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
			tsi721_rio_em_create_events(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0xFF, out_p.failure_idx);

	// NULL event pointer
	in_p.num_events = 1;
	in_p.events = NULL;

	out_p.imp_rc = 0;
	out_p.failure_idx = 0xff;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_create_events(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0xFF, out_p.failure_idx);

	// Illegal port in event
	in_p.num_events = 2;
	in_p.events = events;
	events[0].port_num = 0;
	events[0].event = rio_em_a_no_event;
	events[1].port_num = 1;
	events[1].event = rio_em_f_port_err;

	out_p.imp_rc = 0;
	out_p.failure_idx = 0xff;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_create_events(&mock_dev_info, &in_p,
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
			tsi721_rio_em_create_events(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(1, out_p.failure_idx);

	(void)state;
}

// Test that each individual event can be created.

static void tsi721_rio_em_create_events_success_test(void **state)
{
	rio_em_create_events_in_t in_p;
	rio_em_create_events_out_t out_p;
	rio_em_event_n_loc_t events[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;

	offset_value_event_t tests[] = {
		{TSI721_PLM_STATUS, TSI721_PLM_EVENT_GEN_DLT,
			{0, rio_em_f_los}},
		{TSI721_PLM_STATUS, TSI721_PLM_EVENT_GEN_PORT_ERR,
			{0, rio_em_f_port_err}},
		{TSI721_PLM_STATUS, TSI721_PLM_EVENT_GEN_MAX_DENIAL,
			{0, rio_em_f_2many_pna}},
		{TSI721_PLM_STATUS, TSI721_PLM_EVENT_GEN_OUTPUT_FAIL,
			{0, rio_em_f_err_rate}},
		{TSI721_PLM_STATUS, TSI721_PLM_EVENT_GEN_MAX_DENIAL,
			{0, rio_em_f_2many_retx}},
		{TSI721_LOCAL_ERR_DET, TSI721_LOCAL_ERR_DET_ILL_TYPE |
					TSI721_LOCAL_ERR_DET_ILL_ID,
			{0, rio_em_d_log}},
		{TSI721_PLM_STATUS, TSI721_PLM_EVENT_GEN_LINK_INIT,
			{0, rio_em_i_sig_det}},
		{TSI721_PLM_STATUS, TSI721_PLM_EVENT_GEN_RST_REQ,
			{0, rio_em_i_rst_req}},
		{ TSI721_I2C_INT_STAT, TSI721_I2C_INT_STAT_BL_FAIL,
			{0, rio_em_i_init_fail}}
	};
	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i;
	uint32_t chk_val;

	for (i = 0; i < test_cnt; i++) {
		in_p.num_events = 1;
		in_p.events = events;
		events[0].port_num = 0;
		events[0].event = tests[i].event.event;

		out_p.imp_rc = 0xFFFFFF;
		out_p.failure_idx = 0xff;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_create_events(dev_info, &in_p,
						&out_p));

		assert_int_equal(RIO_SUCCESS, out_p.imp_rc);
		assert_int_equal(0, out_p.failure_idx);
		assert_int_equal(RIO_SUCCESS,
				DARRegRead(dev_info, tests[i].offset,
						&chk_val));
		// When interrupts are not supported, it is not possible to
		// create an I2C_BL_FAIL event...
		if ((rio_em_i_init_fail == tests[i].event.event)
				&& !tsi721_int_supported) {
			continue;
		}
		assert_int_equal(chk_val & tests[i].value, tests[i].value);
	}

	(void)state;
}

// Test that events which should be ignored are successfull.

static void tsi721_rio_em_create_ignored_events_test(void **state)
{
	rio_em_create_events_in_t in_p;
	rio_em_create_events_out_t out_p;
	rio_em_event_n_loc_t events[(uint8_t)rio_em_last];
	DAR_DEV_INFO_t *dev_info = &mock_dev_info;

	rio_em_events_t tests[] = {rio_em_a_clr_pwpnd, rio_em_a_no_event,
			rio_em_d_ttl, rio_em_d_rte};
	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i;

	for (i = 0; i < test_cnt; i++) {
		in_p.num_events = 1;
		in_p.events = events;
		events[0].port_num = 0;
		events[0].event = tests[i];

		out_p.imp_rc = 0xFFFFFF;
		out_p.failure_idx = 0xff;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_create_events(dev_info, &in_p,
						&out_p));

		assert_int_equal(RIO_SUCCESS, out_p.imp_rc);
		assert_int_equal(0, out_p.failure_idx);
	}

	(void)state;
}

// Test bad parameter detection.

static void tsi721_rio_em_get_int_stat_bad_parms_test(void **state)
{
	rio_em_get_int_stat_in_t in_p;
	rio_em_get_int_stat_out_t out_p;
	rio_em_event_n_loc_t events[(uint8_t)rio_em_last];

	// Illegal number of ports
	in_p.ptl.num_ports = 2;
	in_p.ptl.pnums[0] = 0;
	in_p.ptl.pnums[1] = 0;
	in_p.num_events = (uint8_t)rio_em_last;
	in_p.events = events;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_get_int_stat(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	// Illegal port number
	in_p.ptl.num_ports = 1;
	in_p.ptl.pnums[0] = 1;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_get_int_stat(&mock_dev_info, &in_p,
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
			tsi721_rio_em_get_int_stat(&mock_dev_info, &in_p,
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
			tsi721_rio_em_get_int_stat(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	(void)state;
}

// Test the interrupt status is correctly determined for all events
//
// This test is skipped if interrupts are not supported.

static void tsi721_rio_em_get_int_stat_success_test(void **state)
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

	// NOTE: A 2many_pna event also causes a 2many_retx event.
	// For this reason, "2many_pna" must be the last test, 
	// and 2many_retx must occur before 2many_pna.
	rio_em_cfg_t tests[] = {
		{rio_em_f_los, rio_em_detect_on, 1 * 256 * 1000},
		{ rio_em_f_port_err, rio_em_detect_on, 0},
		{rio_em_f_err_rate, rio_em_detect_on, 0x100000FF},
		{rio_em_f_2many_retx, rio_em_detect_on, 0x0010},
		{rio_em_d_log, rio_em_detect_on,
				TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
				TSI721_LOCAL_ERR_EN_ILL_ID_EN},
		{rio_em_i_sig_det, rio_em_detect_on, 0},
		{rio_em_i_rst_req, rio_em_detect_on, 0},
		{rio_em_i_init_fail, rio_em_detect_on, 0},
		{ rio_em_f_2many_pna, rio_em_detect_on, 0x0010}
	};

	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i, chk_i, srch_i;

	if (!tsi721_int_supported) {
		return;
	}

	for (i = 0; i < test_cnt; i++) {
		// Enable detection of each event.
		if (rio_em_i_rst_req == tests[i].em_event) {
			// If we're testing disabling the Reset Request
			// event, do the real disable since this events
			// detection is actually controlled by Port Config
			// functionality.

			assert_int_equal(RIO_SUCCESS,
					DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
			plm_imp_spec_ctl &= ~t_mask;
			assert_int_equal(RIO_SUCCESS,
					DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
		}

		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = 0;
		set_cfg_in.notfn = rio_em_notfn_int;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &tests[i];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(rio_em_notfn_int, set_cfg_out.notfn);

		// Create the event
		c_in.num_events = 1;
		c_in.events = c_e;
		c_e[0].port_num = 0;
		c_e[0].event = tests[i].em_event;

		c_out.imp_rc = 0xFFFFFF;
		c_out.failure_idx = 0xff;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_create_events(dev_info, &c_in,
						&c_out));

		assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
		assert_int_equal(0, c_out.failure_idx);

		// Query the event interrupt status
		in_p.ptl.num_ports = 1;
		in_p.ptl.pnums[0] = 0;
		in_p.num_events = (uint8_t)rio_em_last;
		in_p.events = stat_e;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_p.imp_rc = 0;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_get_int_stat(&mock_dev_info,
						&in_p, &out_p));
		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(i + 1, out_p.num_events);
		assert_false(out_p.too_many);
		assert_false(out_p.other_events);

		// Check that all events created to date are all found...
		for (chk_i = 0; chk_i <= i; chk_i++) {
			bool found = false;
			for (srch_i = 0; !found && (srch_i <= i); srch_i++) {
				if (tests[chk_i].em_event
						== stat_e[srch_i].event) {
					found = true;
				}
			}
			if (!found && DEBUG_PRINTF) {
				printf("i %d event_cnt %d chk_i %d event %d", i,
						out_p.num_events, chk_i,
						tests[chk_i].em_event);
			}
			assert_true(found);
		}

		// Query the event interrupt status again, and trigger the
		// "too many events" flag.
		if (!i) {
			continue;
		}
		in_p.ptl.num_ports = 1;
		in_p.ptl.pnums[0] = 0;
		in_p.num_events = i;
		in_p.events = stat_e;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_p.imp_rc = 0;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_get_int_stat(&mock_dev_info,
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
// the "other events" fields behave correctly.
//
// This test is skipped if interrupts are not supported.

static void tsi721_rio_em_get_int_stat_other_events_test(void **state)
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

	// NOTE: A 2many_pna event also causes an 2many_retx event.
	// This test skips "2many_pna" events.
	rio_em_cfg_t tests[] = {
		{rio_em_f_los, rio_em_detect_on, 1 * 256 * 1000},
		{rio_em_f_port_err, rio_em_detect_on, 0},
		{rio_em_f_err_rate, rio_em_detect_on, 0x1000000F},
		{ rio_em_f_2many_retx, rio_em_detect_on, 0x0010},
		{rio_em_d_log, rio_em_detect_on,
					TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
					TSI721_LOCAL_ERR_EN_ILL_ID_EN},
		{rio_em_i_sig_det, rio_em_detect_on, 0},
		{rio_em_i_rst_req, rio_em_detect_on, 0},
		{rio_em_i_init_fail, rio_em_detect_on, 0}
	};

	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i, t;
	Tsi721_test_state_t *l_st = (Tsi721_test_state_t *)*state;

	// Cannot run the interrupt test if interrupts are not supported,
	// or on real hardware.
	if (!tsi721_int_supported) {
		return;
	}

	if (l_st->real_hw) {
		return;
	}

	// Use test_cnt - 1 here to avoid trying for rio_em_f_2many_pna 
	// without also setting rio_em_f_2many_retx.
	for (i = 0; i < test_cnt - 1; i++) {
		for (t = 0; t < test_cnt; t++) {
			// Must have two different events for this test.
			if (i == t) {
				continue;
			}
			// This test requires a clean slate at the beginning 
			// of each attempt
			tsi721_em_setup(state);

			// Enable detection of the current event.
			if (rio_em_i_rst_req == tests[i].em_event) {
				// If we're testing disabling the Reset Request
				// event, do the real disable since this events
				// detection is actually controlled by 
				// Port Config functionality.

				assert_int_equal(RIO_SUCCESS,
						DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
				if (rio_em_detect_off == tests[i].em_detect) {
					plm_imp_spec_ctl |= t_mask;
				} else {
					plm_imp_spec_ctl &= ~t_mask;
				}
				assert_int_equal(RIO_SUCCESS,
						DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
			}

			// Enable the i'th test
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = 0;
			set_cfg_in.notfn = rio_em_notfn_int;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &tests[i];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_cfg_set(&mock_dev_info,
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
			c_e[0].port_num = 0;
			c_e[0].event = tests[i].em_event;
			c_e[1].port_num = 0;
			c_e[1].event = tests[t].em_event;

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event interrupt status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = 0;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = true;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_int_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(1, out_p.num_events);
			assert_false(out_p.too_many);
			assert_true(out_p.other_events);

			// Check that the event created was found
			assert_int_equal(tests[i].em_event, stat_e[0].event);
		}
	}

	(void)state;
}

// Test bad parameter values are correctly detected and reported.

static void tsi721_rio_em_get_pw_stat_bad_parms_test(void **state)
{
	rio_em_get_pw_stat_in_t in_p;
	rio_em_get_pw_stat_out_t out_p;
	rio_em_event_n_loc_t events[(uint8_t)rio_em_last];

	// Illegal number of ports
	in_p.ptl.num_ports = 2;
	in_p.ptl.pnums[0] = 0;
	in_p.ptl.pnums[1] = 0;
	in_p.num_events = (uint8_t)rio_em_last;
	in_p.events = events;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_get_pw_stat(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	// Illegal port number
	in_p.ptl.num_ports = 1;
	in_p.ptl.pnums[0] = 1;

	out_p.imp_rc = 0;
	out_p.num_events = 0xFF;
	out_p.too_many = true;
	out_p.other_events = true;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_get_pw_stat(&mock_dev_info, &in_p,
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
			tsi721_rio_em_get_pw_stat(&mock_dev_info, &in_p,
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
			tsi721_rio_em_get_pw_stat(&mock_dev_info, &in_p,
					&out_p));
	assert_int_not_equal(0, out_p.imp_rc);
	assert_int_equal(0, out_p.num_events);
	assert_false(out_p.too_many);
	assert_false(out_p.other_events);

	(void)state;
}

// Test port-write status is correctly determined

static void tsi721_rio_em_get_pw_stat_success_test(void **state)
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

	// NOTE: A 2many_pna event also causes an err_rate event.
	// For this reason, "2many_pna" must be the last test, 
	// and err_rate must occur before 2many_pna.
	rio_em_cfg_t tests[] = {
		{rio_em_f_los, rio_em_detect_on, 1 * 256 * 1000},
		{rio_em_f_port_err, rio_em_detect_on, 0},
		{rio_em_f_err_rate, rio_em_detect_on, 0x100000FF},
		{rio_em_f_2many_retx, rio_em_detect_on, 0x0010},
		{rio_em_d_log, rio_em_detect_on,
					TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
					TSI721_LOCAL_ERR_EN_ILL_ID_EN},
		{rio_em_i_sig_det, rio_em_detect_on, 0},
		{rio_em_i_rst_req, rio_em_detect_on, 0},
		{rio_em_f_2many_pna, rio_em_detect_on, 0x0010}
	};

	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i, chk_i, srch_i;
	Tsi721_test_state_t *l_st = (Tsi721_test_state_t *)*state;

	if (l_st->real_hw) {
		return;
	}

	for (i = 0; i < test_cnt; i++) {
		// Enable detection of each event.
		if (rio_em_i_rst_req == tests[i].em_event) {
			// If we're testing disabling the Reset Request
			// event, do the real disable since this events
			// detection is actually controlled by Port Config
			// functionality.

			assert_int_equal(RIO_SUCCESS,
					DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
			plm_imp_spec_ctl &= ~t_mask;
			assert_int_equal(RIO_SUCCESS,
					DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
		}

		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = 0;
		set_cfg_in.notfn = rio_em_notfn_pw;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &tests[i];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(rio_em_notfn_pw, set_cfg_out.notfn);

		// Create the event
		c_in.num_events = 1;
		c_in.events = c_e;
		c_e[0].port_num = 0;
		c_e[0].event = tests[i].em_event;

		c_out.imp_rc = 0xFFFFFF;
		c_out.failure_idx = 0xff;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_create_events(dev_info, &c_in,
						&c_out));

		assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
		assert_int_equal(0, c_out.failure_idx);

		// Query the event port-write status
		in_p.ptl.num_ports = 1;
		in_p.ptl.pnums[0] = 0;
		in_p.num_events = (uint8_t)rio_em_last;
		in_p.events = stat_e;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_p.imp_rc = 0;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_get_pw_stat(&mock_dev_info, &in_p,
						&out_p));
		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(i + 1, out_p.num_events);
		assert_false(out_p.too_many);
		assert_false(out_p.other_events);

		// Check that all events created to date are all found...
		for (chk_i = 0; chk_i <= i; chk_i++) {
			bool found = false;
			for (srch_i = 0; !found && (srch_i <= i); srch_i++) {
				if (tests[chk_i].em_event
						== stat_e[srch_i].event) {
					found = true;
				}
			}
			if (!found && DEBUG_PRINTF) {
				printf("i %d event_cnt %d chk_i %d event %d", i,
						out_p.num_events, chk_i,
						tests[chk_i].em_event);
			}
			assert_true(found);
		}

		// Query the event interrupt status again, and trigger the
		// "too many events" flag.
		if (!i) {
			continue;
		}
		in_p.ptl.num_ports = 1;
		in_p.ptl.pnums[0] = 0;
		in_p.num_events = i;
		in_p.events = stat_e;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_p.imp_rc = 0;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_get_pw_stat(&mock_dev_info, &in_p,
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

static void tsi721_rio_em_get_pw_stat_other_events_test(void **state)
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

	// NOTE: A 2many_pna event also causes an 2many_retx event.
	// For this reason, "2many_pna" must be the last test, 
	// and 2many_retx must occur before 2many_pna.
	//
	rio_em_cfg_t tests[] = {
		{rio_em_f_los, rio_em_detect_on, 1 * 256 * 1000},
		{rio_em_f_port_err, rio_em_detect_on, 0},
		{rio_em_f_err_rate, rio_em_detect_on, 0x1000000F},
		{rio_em_d_log, rio_em_detect_on,
					TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
					TSI721_LOCAL_ERR_EN_ILL_ID_EN},
		{rio_em_i_sig_det, rio_em_detect_on, 0},
		{rio_em_i_rst_req, rio_em_detect_on, 0},
		{rio_em_f_2many_retx, rio_em_detect_on, 0x0010},
		{ rio_em_f_2many_pna, rio_em_detect_on, 0x0010}
	};

	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i, t;
	Tsi721_test_state_t *l_st = (Tsi721_test_state_t *)*state;

	if (l_st->real_hw) {
		return;
	}

	// Use test_cnt - 1 here to avoid trying for rio_em_f_2many_pna 
	// without also setting rio_em_f_2many_retx.
	for (i = 0; i < test_cnt - 1; i++) {
		for (t = 0; t < test_cnt; t++) {
			// Must have two different events for this test.
			if (i == t) {
				continue;
			}
			if (DEBUG_PRINTF) {
				printf("\ni %d t %d\n", i, t);
			}
			// This test requires a clean slate at the beginning 
			// of each attempt
			tsi721_em_setup(state);

			// Enable detection of the current event.
			if (rio_em_i_rst_req == tests[i].em_event) {
				// If we're testing disabling the Reset Request
				// event, do the real disable since this events
				// detection is actually controlled by 
				// Port Config functionality.

				assert_int_equal(RIO_SUCCESS,
						DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
				if (rio_em_detect_off == tests[i].em_detect) {
					plm_imp_spec_ctl |= t_mask;
				} else {
					plm_imp_spec_ctl &= ~t_mask;
				}
				assert_int_equal(RIO_SUCCESS,
						DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
			}

			// Enable the i'th test
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = 0;
			set_cfg_in.notfn = rio_em_notfn_pw;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &tests[i];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_cfg_set(&mock_dev_info,
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
			c_e[0].port_num = 0;
			c_e[0].event = tests[i].em_event;
			c_e[1].port_num = 0;
			c_e[1].event = tests[t].em_event;

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = 0;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = true;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(1, out_p.num_events);
			assert_false(out_p.too_many);
			assert_int_equal(tests[i].em_event, stat_e[0].event);

			// Check that the other events were found, EXCEPT
			// when the two events are 2many_retx & 2many_pna
			if ((rio_em_f_2many_retx == tests[i].em_event) &&
				(rio_em_f_2many_pna == tests[t].em_event)) {
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
//
// This test is skipped if interrupts are not supported.

static void tsi721_rio_em_get_int_pw_stat_other_events_test(void **state)
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

	// NOTE: A 2many_pna event also causes an err_rate event.
	// For this reason, "2many_pna" must be the last test, 
	// and err_rate must occur before 2many_pna.
	rio_em_cfg_t tests[] = {
		{rio_em_f_los, rio_em_detect_on, 1 * 256 * 1000},
		{rio_em_f_port_err, rio_em_detect_on, 0},
		{rio_em_f_err_rate, rio_em_detect_on, 0x1000000F},
		{rio_em_d_log, rio_em_detect_on,
					TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
					TSI721_LOCAL_ERR_EN_ILL_ID_EN},
		{rio_em_i_sig_det, rio_em_detect_on, 0},
		{rio_em_i_rst_req, rio_em_detect_on, 0},
		{rio_em_i_init_fail, rio_em_detect_on, 0},
		{rio_em_f_2many_retx, rio_em_detect_on,0x0010},
		{rio_em_f_2many_pna, rio_em_detect_on, 0x0010}
	};

	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i, p;
	Tsi721_test_state_t *l_st = (Tsi721_test_state_t *)*state;
	rio_em_notfn_ctl_t i_notfn = rio_em_notfn_int;
	rio_em_notfn_ctl_t p_notfn = rio_em_notfn_pw;

	// Cannot run the interrupt test if interrupts are not supported,
	// or on real hardware.
	if (!tsi721_int_supported) {
		return;
	}

	if (l_st->real_hw) {
		return;
	}

	for (i = 0; i < test_cnt; i++) {
		for (p = 0; p < test_cnt; p++) {
			// Must have two different events for this test.
			if (i == p) {
				continue;
			}
			// init_fail events can only receive interrupt notifn..
			if (rio_em_i_init_fail == tests[p].em_event) {
				continue;
			}
			// 2many_retx and 2many_pna cause the same event,
			// so skip ahead if they're both selected...
			if ((rio_em_f_2many_retx == tests[i].em_event) &&
				(rio_em_f_2many_pna == tests[p].em_event)) {
				continue;
			}
			if ((rio_em_f_2many_retx == tests[p].em_event) &&
				(rio_em_f_2many_pna == tests[i].em_event)) {
				continue;
			}
			if (DEBUG_PRINTF) {
				printf("\ni = %d p = %d event = %d %d\n", i, p,
						tests[i].em_event,
						tests[p].em_event);
			}
			if (DEBUG_PRINTF) {
				printf("\ni %d p %d\n", i, p);
			}

			// This test requires a clean slate at the beginning 
			// of each attempt
			tsi721_em_setup(state);

			// Enable detection of the current event.
			// NOTE: Only one of tests[i].em_event and
			// tests[p].em_event can be true at any one time.
			if (rio_em_i_rst_req == tests[i].em_event) {
				// If we're testing the Reset Request
				// event, do the disable/enable since this
				// events detection is actually controlled by 
				// Port Config functionality.

				assert_int_equal(RIO_SUCCESS,
						DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
				if (rio_em_detect_off == tests[i].em_detect) {
					plm_imp_spec_ctl |= t_mask;
				} else {
					plm_imp_spec_ctl &= ~t_mask;
				}
				assert_int_equal(RIO_SUCCESS,
						DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
			}
			if (rio_em_i_rst_req == tests[p].em_event) {
				// If we're testing the Reset Request
				// event, do the disable/enable since this
				// events detection is actually controlled by 
				// Port Config functionality.

				assert_int_equal(RIO_SUCCESS,
						DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
				if (rio_em_detect_off == tests[p].em_detect) {
					plm_imp_spec_ctl |= t_mask;
				} else {
					plm_imp_spec_ctl &= ~t_mask;
				}
				assert_int_equal(RIO_SUCCESS,
						DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
			}

			// Enable event with interrupt notification
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = 0;
			set_cfg_in.notfn = i_notfn;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &tests[i];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_cfg_set(&mock_dev_info,
							&set_cfg_in,
							&set_cfg_out));
			assert_int_equal(0, set_cfg_out.imp_rc);
			assert_int_equal(RIO_ALL_PORTS,
					set_cfg_out.fail_port_num);
			assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
			assert_int_equal(i_notfn, set_cfg_out.notfn);

			// Enable event with port-write notification
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = 0;
			set_cfg_in.notfn = p_notfn;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &tests[p];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_cfg_set(&mock_dev_info,
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
			c_e[0].port_num = 0;
			c_e[0].event = tests[i].em_event;
			c_e[1].port_num = 0;
			c_e[1].event = tests[p].em_event;

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event interrupt status
			in_i.ptl.num_ports = 1;
			in_i.ptl.pnums[0] = 0;
			in_i.num_events = (uint8_t)rio_em_last;
			in_i.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_i.imp_rc = 0;
			out_i.num_events = 0xFF;
			out_i.too_many = false;
			out_i.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			if (rio_em_notfn_int == i_notfn) {
				assert_int_equal(1, out_i.num_events);
				assert_false(out_i.too_many);
				assert_true(out_i.other_events);
				assert_int_equal(stat_e[0].port_num, 0);
				assert_int_equal(stat_e[0].event,
						tests[i].em_event);
			} else {
				bool found;

				assert_int_equal(2, out_i.num_events);
				assert_false(out_i.too_many);
				assert_false(out_i.other_events);
				assert_int_equal(stat_e[0].port_num, 0);
				assert_int_equal(stat_e[1].port_num, 0);

				found =
						((stat_e[0].event
								== tests[p].em_event)
								&& (stat_e[1].event
										== tests[i].em_event))
								|| ((stat_e[0].event
										== tests[i].em_event)
										&& (stat_e[1].event
												== tests[p].em_event));
				assert_true(found);
			}

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = 0;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = false;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			if (rio_em_notfn_pw == p_notfn) {
				assert_int_equal(1, out_p.num_events);
				assert_false(out_p.too_many);
				// init_fail is an interrupt-only event,
				// so it cannot trigger "other events" for
				// a port-write
				if (rio_em_i_init_fail == tests[i].em_event) {
					assert_false(out_p.other_events);
				} else {
					assert_true(out_p.other_events);
				}
				assert_int_equal(stat_e[0].port_num, 0);
				assert_int_equal(stat_e[0].event,
						tests[p].em_event);
			} else {
				bool found;

				assert_int_equal(2, out_p.num_events);
				assert_false(out_p.too_many);
				assert_false(out_p.other_events);
				assert_int_equal(stat_e[0].port_num, 0);
				assert_int_equal(stat_e[1].port_num, 0);
				found =
						((stat_e[0].event
								== tests[p].em_event)
								&& (stat_e[1].event
										== tests[i].em_event))
								|| ((stat_e[0].event
										== tests[i].em_event)
										&& (stat_e[1].event
												== tests[p].em_event));
				assert_true(found);
			}
		}
	}

	(void)state;
}

// Test that if two events are configured with both interrupt and
// port-write notification, that the interrupt and port-write status is
// correct.
//
// This test is skipped if interrupts are not supported.

static void tsi721_rio_em_get_int_pw_stat_both_test(void **state)
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

	// NOTE: A 2many_pna event also causes an err_rate event.
	// For this reason, "2many_pna" must be the last test, 
	// and err_rate must occur before 2many_pna.
	rio_em_cfg_t tests[] = {
		{rio_em_f_los, rio_em_detect_on, 1 * 256 * 1000},
		{rio_em_f_port_err, rio_em_detect_on, 0},
		{rio_em_f_err_rate, rio_em_detect_on, 0x1000000F},
		{rio_em_d_log, rio_em_detect_on,
				TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
				TSI721_LOCAL_ERR_EN_ILL_ID_EN},
		{rio_em_i_sig_det, rio_em_detect_on, 0},
		{rio_em_i_rst_req, rio_em_detect_on, 0},
		{rio_em_i_init_fail, rio_em_detect_on, 0},
		{rio_em_f_2many_retx, rio_em_detect_on, 0x0010},
		{rio_em_f_2many_pna, rio_em_detect_on,0x0010}
	};

	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i, p, srch_i, chk_i;
	Tsi721_test_state_t *l_st = (Tsi721_test_state_t *)*state;

	// Cannot run the interrupt test if interrupts are not supported,
	// or on real hardware.
	if (!tsi721_int_supported) {
		return;
	}

	if (l_st->real_hw) {
		return;
	}

	// Use test_cnt - 1 here to avoid trying for rio_em_f_2many_pna 
	// without also setting rio_em_f_2many_retx.
	for (i = 0; i < test_cnt - 1; i++) {
		for (p = 0; p < test_cnt; p++) {
			// Must have two different events for this test.
			if (i == p) {
				continue;
			}
			// Cannot cause an init_fail event to generate a
			// port-write
			if (rio_em_i_init_fail == tests[p].em_event) {
				continue;
			}
			// This test requires a clean slate at the beginning 
			// of each attempt
			tsi721_em_setup(state);

			// Enable detection of the current event.
			// NOTE: Only one of tests[i].em_event and
			// tests[p].em_event can be true at any one time.
			if (rio_em_i_rst_req == tests[i].em_event) {
				// If we're testing the Reset Request
				// event, do the disable/enable since this
				// events detection is actually controlled by 
				// Port Config functionality.

				assert_int_equal(RIO_SUCCESS,
						DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
				if (rio_em_detect_off == tests[i].em_detect) {
					plm_imp_spec_ctl |= t_mask;
				} else {
					plm_imp_spec_ctl &= ~t_mask;
				}
				assert_int_equal(RIO_SUCCESS,
						DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
			}
			if (rio_em_i_rst_req == tests[p].em_event) {
				// If we're testing the Reset Request
				// event, do the disable/enable since this
				// events detection is actually controlled by 
				// Port Config functionality.

				assert_int_equal(RIO_SUCCESS,
						DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
				if (rio_em_detect_off == tests[p].em_detect) {
					plm_imp_spec_ctl |= t_mask;
				} else {
					plm_imp_spec_ctl &= ~t_mask;
				}
				assert_int_equal(RIO_SUCCESS,
						DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
			}

			// Configure the i'th and p'th event
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = 0;
			set_cfg_in.notfn = rio_em_notfn_both;
			set_cfg_in.num_events = 2;
			memcpy(&tests_in[0], &tests[i], sizeof(tests_in[0]));
			memcpy(&tests_in[1], &tests[p], sizeof(tests_in[1]));
			set_cfg_in.events = tests_in;

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_cfg_set(&mock_dev_info,
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
			c_e[0].port_num = 0;
			c_e[0].event = tests[i].em_event;
			c_e[1].port_num = 0;
			c_e[1].event = tests[p].em_event;

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event interrupt status
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
					tsi721_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(2, out_i.num_events);
			assert_false(out_i.too_many);
			assert_false(out_i.other_events);
			assert_int_equal(stat_e[0].port_num, 0);
			assert_int_equal(stat_e[1].port_num, 0);

			// Check that events created are found...
			for (chk_i = 0; chk_i <= i; chk_i++) {
				bool found = false;
				if ((chk_i != i) && (chk_i != p)) {
					continue;
				}
				for (srch_i = 0;
						!found
								&& (srch_i
										<= out_i.num_events);
						srch_i++) {
					if (tests[chk_i].em_event
							== stat_e[srch_i].event) {
						found = true;
					}
				}
				if (!found && DEBUG_PRINTF) {
					printf(
							"i %d event_cnt %d chk_i %d event %d",
							i, out_p.num_events,
							chk_i,
							tests[chk_i].em_event);
				}
				assert_true(found);
			}

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = 0;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = true;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			// It is not possible to detect a port-write init_fail
			// event so reduce the event count by 1.
			if (rio_em_i_init_fail == tests[i].em_event) {
				assert_int_equal(1, out_p.num_events);
				assert_int_equal(stat_e[0].port_num, 0);
			} else {
				assert_int_equal(2, out_p.num_events);
				assert_int_equal(stat_e[0].port_num, 0);
				assert_int_equal(stat_e[1].port_num, 0);
			}
			assert_false(out_p.too_many);
			assert_false(out_p.other_events);

			// Check that events created are found...
			for (chk_i = 0; chk_i <= i; chk_i++) {
				bool found = false;
				if ((chk_i != i) && (chk_i != p)) {
					continue;
				}
				if ((chk_i == i)
						&& (rio_em_i_init_fail
								== tests[i].em_event)) {
					continue;
				}
				for (srch_i = 0;
						!found
								&& (srch_i
										<= out_i.num_events);
						srch_i++) {
					if (tests[chk_i].em_event
							== stat_e[srch_i].event) {
						found = true;
					}
				}
				if (!found && DEBUG_PRINTF) {
					printf(
							"i %d event_cnt %d chk_i %d event %d",
							i, out_p.num_events,
							chk_i,
							tests[chk_i].em_event);
				}
				assert_true(found);
			}
		}
	}

	(void)state;
}

// Test that bad parameter values are detected and reported.

static void tsi721_rio_em_clr_events_bad_parms_test(void **state)
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
			tsi721_rio_em_clr_events(&mock_dev_info, &in_c,
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
			tsi721_rio_em_clr_events(&mock_dev_info, &in_c,
					&out_c));
	assert_int_not_equal(0, out_c.imp_rc);
	assert_int_equal(0xFF, out_c.failure_idx);
	assert_true(out_c.pw_events_remain);
	assert_true(out_c.int_events_remain);

	// Illegal port in event
	in_c.num_events = 1;
	in_c.events = events;
	events[0].port_num = 1;
	events[0].event = rio_em_a_no_event;

	out_c.imp_rc = 0;
	out_c.failure_idx = 0xFF;
	out_c.pw_events_remain = true;
	out_c.int_events_remain = true;

	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_em_clr_events(&mock_dev_info, &in_c,
					&out_c));
	assert_int_not_equal(0, out_c.imp_rc);
	assert_int_equal(0, out_c.failure_idx);
	assert_false(out_c.pw_events_remain);
	assert_false(out_c.int_events_remain);

	(void)state;
}

// Verify that each interrupt event can be cleared.
//
// This test is skipped if interrupts are not supported.

static void tsi721_rio_em_clr_int_events_success_test(void **state)
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

	// NOTE: A 2many_pna event also causes an 2many_retx event.
	// For this reason, "2many_pna" must be the last test, 
	// and 2many_retx must occur before 2many_pna.
	rio_em_cfg_t tests[] = {
		{rio_em_f_los, rio_em_detect_on, 1 * 256 * 1000},
		{rio_em_f_port_err, rio_em_detect_on, 0},
		{rio_em_f_err_rate, rio_em_detect_on, 0x100000FF},
		{rio_em_f_2many_retx, rio_em_detect_on, 0x0010},
		{rio_em_d_log, rio_em_detect_on,
					TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
					TSI721_LOCAL_ERR_EN_ILL_ID_EN},
		{rio_em_i_sig_det, rio_em_detect_on, 0},
		{rio_em_i_rst_req, rio_em_detect_on, 0},
		{rio_em_i_init_fail, rio_em_detect_on, 0},
		{rio_em_f_2many_pna, rio_em_detect_on, 0x0010}
	};

	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i, chk_i, srch_i;
	Tsi721_test_state_t *l_st = (Tsi721_test_state_t *)*state;

	// Cannot run the interrupt test if interrupts are not supported,
	// or on real hardware.
	if (!tsi721_int_supported) {
		return;
	}

	if (l_st->real_hw) {
		return;
	}

	for (i = 0; i < test_cnt; i++) {
		if (DEBUG_PRINTF) {
			printf("\ni = %d event = %d\n", i, tests[i].em_event);
		}
		// Enable detection of each event.
		if (rio_em_i_rst_req == tests[i].em_event) {
			// If we're testing disabling the Reset Request
			// event, do the real disable since this events
			// detection is actually controlled by Port Config
			// functionality.

			assert_int_equal(RIO_SUCCESS,
					DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
			plm_imp_spec_ctl &= ~t_mask;
			assert_int_equal(RIO_SUCCESS,
					DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
		}

		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = 0;
		set_cfg_in.notfn = rio_em_notfn_both;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &tests[i];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(rio_em_notfn_both, set_cfg_out.notfn);

		// Create all events
		c_in.num_events = i + 1;
		c_in.events = c_e;
		for (srch_i = 0; srch_i <= i; srch_i++) {
			c_e[srch_i].port_num = 0;
			c_e[srch_i].event = tests[srch_i].em_event;
		}

		c_out.imp_rc = 0xFFFFFF;
		c_out.failure_idx = 0xff;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_create_events(dev_info, &c_in,
						&c_out));

		assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
		assert_int_equal(0, c_out.failure_idx);

		// Query the event interrupt status
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
				tsi721_rio_em_get_int_stat(&mock_dev_info,
						&in_i, &out_i));
		assert_int_equal(0, out_i.imp_rc);
		assert_int_equal(i + 1, out_i.num_events);
		assert_false(out_i.too_many);
		assert_false(out_i.other_events);

		// Check that all events created to date are all found...
		for (chk_i = 0; chk_i <= i; chk_i++) {
			bool found = false;
			for (srch_i = 0; !found && (srch_i <= i); srch_i++) {
				if (tests[chk_i].em_event
						== stat_e[srch_i].event) {
					found = true;
				}
			}
			if (!found && DEBUG_PRINTF) {
				printf("i %d event_cnt %d chk_i %d event %d", i,
						out_i.num_events, chk_i,
						tests[chk_i].em_event);
			}
			assert_true(found);
		}

		// Clear all interrupt events...
		in_c.num_events = out_i.num_events;
		in_c.events = in_i.events;

		out_c.imp_rc = 0xFFFF;
		out_c.failure_idx = 0xFF;
		out_c.pw_events_remain = true;
		out_c.int_events_remain = true;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_clr_events(&mock_dev_info, &in_c,
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
				tsi721_rio_em_get_int_stat(&mock_dev_info,
						&in_i, &out_i));
		assert_int_equal(0, out_i.imp_rc);
		if (out_i.num_events && DEBUG_PRINTF) {
			printf("\n%d events, first is %d\n", out_i.num_events,
					stat_e[0].event);
		}
		assert_int_equal(0, out_i.num_events);
		assert_false(out_i.too_many);
		assert_false(out_i.other_events);
	}

	(void)state;
}

// Verify that each port-write event can be cleared.

static void tsi721_rio_em_clr_pw_events_success_test(void **state)
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

	// NOTE: A 2many_pna event also causes an err_rate event.
	// For this reason, "2many_pna" must be the last test, 
	// and err_rate must occur before 2many_pna.
	rio_em_cfg_t tests[] = {
			{rio_em_f_los, rio_em_detect_on, 1 * 256 * 1000}, {
					rio_em_f_port_err, rio_em_detect_on, 0},
			{rio_em_f_err_rate, rio_em_detect_on, 0x100000FF}, {
					rio_em_f_2many_retx, rio_em_detect_on,
					0x0010}, {rio_em_d_log,
					rio_em_detect_on,
					TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
					TSI721_LOCAL_ERR_EN_ILL_ID_EN}, {
					rio_em_i_sig_det, rio_em_detect_on, 0},
			{rio_em_i_rst_req, rio_em_detect_on, 0}, {
					rio_em_f_2many_pna, rio_em_detect_on,
					0x0010}};

	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i, chk_i, srch_i;

	// Before beginning, clear all events in hardware
	// Fail if any events remain.

	for (i = 0; i < test_cnt; i++) {
		c_e[i].event = tests[i].em_event;
		c_e[i].port_num = 0;
	}
	in_c.num_events = test_cnt;
	in_c.events = c_e;

	out_c.imp_rc = 0xFFFF;
	out_c.failure_idx = 0xFF;
	out_c.pw_events_remain = true;
	out_c.int_events_remain = true;

	assert_int_equal(RIO_SUCCESS,
			tsi721_rio_em_clr_events(&mock_dev_info, &in_c,
					&out_c));

	assert_int_equal(0, out_c.imp_rc);
	assert_int_equal(0, out_c.failure_idx);
	assert_false(out_c.pw_events_remain);
	assert_false(out_c.int_events_remain);

	// Now grind through creating and clearing all events.

	for (i = 0; i < test_cnt; i++) {
		if (DEBUG_PRINTF) {
			printf("\ni = %d event = %d\n", i, tests[i].em_event);
		}
		// Enable detection of each event.
		if (rio_em_i_rst_req == tests[i].em_event) {
			// If we're testing disabling the Reset Request
			// event, do the real disable since this events
			// detection is actually controlled by Port Config
			// functionality.

			assert_int_equal(RIO_SUCCESS,
					DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
			plm_imp_spec_ctl &= ~t_mask;
			assert_int_equal(RIO_SUCCESS,
					DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
		}

		set_cfg_in.ptl.num_ports = 1;
		set_cfg_in.ptl.pnums[0] = 0;
		set_cfg_in.notfn = rio_em_notfn_pw;
		set_cfg_in.num_events = 1;
		set_cfg_in.events = &tests[i];

		set_cfg_out.imp_rc = 0xFFFFFFFF;
		set_cfg_out.fail_port_num = 0x99;
		set_cfg_out.fail_idx = 0xFF;
		set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_cfg_set(&mock_dev_info,
						&set_cfg_in, &set_cfg_out));
		assert_int_equal(0, set_cfg_out.imp_rc);
		assert_int_equal(RIO_ALL_PORTS, set_cfg_out.fail_port_num);
		assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
		assert_int_equal(rio_em_notfn_pw, set_cfg_out.notfn);

		// Create the event
		c_in.num_events = i + 1;
		c_in.events = c_e;
		for (srch_i = 0; srch_i <= i; srch_i++) {
			c_e[srch_i].port_num = 0;
			c_e[srch_i].event = tests[srch_i].em_event;
		}

		c_out.imp_rc = 0xFFFFFF;
		c_out.failure_idx = 0xff;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_create_events(dev_info, &c_in,
						&c_out));

		assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
		assert_int_equal(0, c_out.failure_idx);

		// Query the event port-write status
		in_p.ptl.num_ports = 1;
		in_p.ptl.pnums[0] = 0;
		in_p.num_events = (uint8_t)rio_em_last;
		in_p.events = stat_e;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_p.imp_rc = 0;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_get_pw_stat(&mock_dev_info, &in_p,
						&out_p));
		assert_int_equal(0, out_p.imp_rc);
		assert_int_equal(i + 1, out_p.num_events);
		assert_false(out_p.too_many);
		assert_false(out_p.other_events);

		// Check that all events created to date are all found...
		for (chk_i = 0; chk_i <= i; chk_i++) {
			bool found = false;
			for (srch_i = 0; !found && (srch_i <= i); srch_i++) {
				if (tests[chk_i].em_event
						== stat_e[srch_i].event) {
					found = true;
				}
			}
			if (!found && DEBUG_PRINTF) {
				printf("i %d event_cnt %d chk_i %d event %d", i,
						out_p.num_events, chk_i,
						tests[chk_i].em_event);
			}
			assert_true(found);
		}

		// Clear all port-write events...
		in_c.num_events = out_p.num_events;
		in_c.events = in_p.events;

		out_c.imp_rc = 0xFFFF;
		out_c.failure_idx = 0xFF;
		out_c.pw_events_remain = true;
		out_c.int_events_remain = true;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_clr_events(&mock_dev_info, &in_c,
						&out_c));
		assert_int_equal(0, out_c.imp_rc);
		assert_int_equal(0, out_c.failure_idx);
		assert_false(out_c.pw_events_remain);
		assert_false(out_c.int_events_remain);

		// Query the event port-write status, check all events are gone
		in_p.ptl.num_ports = 1;
		in_p.ptl.pnums[0] = 0;
		in_p.num_events = (uint8_t)rio_em_last;
		in_p.events = stat_e;
		memset(stat_e, 0xFF, sizeof(stat_e));

		out_p.imp_rc = 0;
		out_p.num_events = 0xFF;
		out_p.too_many = true;
		out_p.other_events = true;

		assert_int_equal(RIO_SUCCESS,
				tsi721_rio_em_get_pw_stat(&mock_dev_info, &in_p,
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
//
// This test is skipped if interrupts are not supported.

static void tsi721_rio_em_clr_int_events_other_events_test(void **state)
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

	// NOTE: A 2many_pna event also causes an 2many_retx event.
	// For this reason, "2many_pna" must be the last test, 
	// and 2many_retx must occur before 2many_pna.
	rio_em_cfg_t tests[] = {
		{rio_em_f_los, rio_em_detect_on, 1 * 256 * 1000},
		{rio_em_f_port_err, rio_em_detect_on, 0},
		{rio_em_f_err_rate, rio_em_detect_on, 0x1000000F},
		{rio_em_d_log, rio_em_detect_on,
					TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
					TSI721_LOCAL_ERR_EN_ILL_ID_EN},
		{rio_em_i_sig_det, rio_em_detect_on, 0},
		{rio_em_i_rst_req, rio_em_detect_on, 0},
		{rio_em_i_init_fail, rio_em_detect_on, 0},
		{rio_em_f_2many_retx, rio_em_detect_on, 0x0010},
		{rio_em_f_2many_pna, rio_em_detect_on, 0x0010}
	};

	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i, t;
	Tsi721_test_state_t *l_st = (Tsi721_test_state_t *)*state;

	// Cannot run the interrupt test if interrupts are not supported,
	// or on real hardware.
	if (!tsi721_int_supported) {
		return;
	}

	if (l_st->real_hw) {
		return;
	}

	// Use test_cnt - 1 here to avoid trying for rio_em_f_2many_pna 
	// without also setting rio_em_f_err_rate.
	for (i = 0; i < test_cnt - 1; i++) {
		for (t = 0; t < test_cnt; t++) {
			// Must have two different events for this test.
			if (i == t) {
				continue;
			}
			// init_fail events can only cause interrupts on the
			// Tsi721, so if they are the test event, don't bother
			if (rio_em_i_init_fail == tests[t].em_event) {
				continue;
			}
			// 2many_retx and 2many_pna cause the same event,
			// so skip ahead if they're both selected...
			if ((rio_em_f_2many_retx == tests[i].em_event) &&
				(rio_em_f_2many_pna == tests[t].em_event)) {
				continue;
			}
			if ((rio_em_f_2many_retx == tests[t].em_event) &&
				(rio_em_f_2many_pna == tests[i].em_event)) {
				continue;
			}
			if (DEBUG_PRINTF) {
				printf("\ni = %d t = %d event = %d %d\n", i, t,
						tests[i].em_event,
						tests[t].em_event);
			}
			// This test requires a clean slate at the beginning 
			// of each attempt
			tsi721_em_setup(state);

			// Enable detection of the current event.
			if (rio_em_i_rst_req == tests[i].em_event) {
				// If we're testing disabling the Reset Request
				// event, do the real disable since this events
				// detection is actually controlled by 
				// Port Config functionality.

				assert_int_equal(RIO_SUCCESS,
						DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
				if (rio_em_detect_off == tests[i].em_detect) {
					plm_imp_spec_ctl |= t_mask;
				} else {
					plm_imp_spec_ctl &= ~t_mask;
				}
				assert_int_equal(RIO_SUCCESS,
						DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
			}

			// Enable the i'th test
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = 0;
			set_cfg_in.notfn = rio_em_notfn_int;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &tests[i];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_cfg_set(&mock_dev_info,
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
			c_e[0].port_num = 0;
			c_e[0].event = tests[i].em_event;
			c_e[1].port_num = 0;
			c_e[1].event = tests[t].em_event;

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event interrupt status
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
					tsi721_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(1, out_i.num_events);
			assert_false(out_i.too_many);
			assert_true(out_i.other_events);

			// Check that the event created was found
			assert_int_equal(tests[i].em_event, stat_e[0].event);

			// Clear all interrupt events...
			in_c.num_events = out_i.num_events;
			in_c.events = in_i.events;

			out_c.imp_rc = 0xFFFF;
			out_c.failure_idx = 0xFF;
			out_c.pw_events_remain = true;
			out_c.int_events_remain = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_clr_events(&mock_dev_info,
							&in_c, &out_c));
			assert_int_equal(0, out_c.imp_rc);
			assert_int_equal(0, out_c.failure_idx);

			assert_true(out_c.pw_events_remain);
			assert_true(out_c.int_events_remain);

			// Query the event interrupt status, confirm that
			// port-write events remain...
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
					tsi721_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(0, out_i.num_events);
			assert_false(out_i.too_many);
			assert_true(out_i.other_events);
		}
	}

	(void)state;
}

// Test that if one event is configured with port-write
// notification and all other events are disabled, that when the port-write
// event is cleared the "other events" fields behave correctly.
// 
// This test is skipped on real hardware.

static void tsi721_rio_em_clr_pw_events_other_events_test(void **state)
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

	// NOTE: A 2many_pna event also causes an err_rate event.
	// For this reason, "2many_pna" must be the last test, 
	// and err_rate must occur before 2many_pna.
	rio_em_cfg_t tests[] = {
		{rio_em_f_los, rio_em_detect_on, 1 * 256 * 1000},
		{rio_em_f_port_err, rio_em_detect_on, 0},
		{rio_em_f_err_rate, rio_em_detect_on, 0x1000000F},
		{rio_em_d_log, rio_em_detect_on,
					TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
					TSI721_LOCAL_ERR_EN_ILL_ID_EN},
		{rio_em_i_sig_det, rio_em_detect_on, 0},
		{rio_em_i_rst_req, rio_em_detect_on, 0},
		{rio_em_f_2many_retx, rio_em_detect_on, 0x0010},
		{rio_em_f_2many_pna, rio_em_detect_on, 0x0010}
	};

	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i, t;
	Tsi721_test_state_t *l_st = (Tsi721_test_state_t *)*state;

	if (l_st->real_hw) {
		return;
	}

	// Use test_cnt - 1 here to avoid trying for rio_em_f_2many_pna 
	// without also setting rio_em_f_err_rate.
	for (i = 0; i < test_cnt - 1; i++) {
		for (t = 0; t < test_cnt; t++) {
			// Must have two different events for this test.
			if (i == t) {
				continue;
			}
			// 2many_retx and 2many_pna cause the same event,
			// so skip ahead if they're both selected...
			if ((rio_em_f_2many_retx == tests[i].em_event) &&
				(rio_em_f_2many_pna == tests[t].em_event)) {
				continue;
			}
			if ((rio_em_f_2many_retx == tests[t].em_event) &&
				(rio_em_f_2many_pna == tests[i].em_event)) {
				continue;
			}
			if (DEBUG_PRINTF) {
				printf("\ni = %d t = %d event = %d %d\n", i, t,
						tests[i].em_event,
						tests[t].em_event);
			}
			// This test requires a clean slate at the beginning 
			// of each attempt
			tsi721_em_setup(state);

			// Enable detection of the current event.
			if (rio_em_i_rst_req == tests[i].em_event) {
				// If we're testing disabling the Reset Request
				// event, do the real disable since this events
				// detection is actually controlled by 
				// Port Config functionality.

				assert_int_equal(RIO_SUCCESS,
						DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
				if (rio_em_detect_off == tests[i].em_detect) {
					plm_imp_spec_ctl |= t_mask;
				} else {
					plm_imp_spec_ctl &= ~t_mask;
				}
				assert_int_equal(RIO_SUCCESS,
						DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
			}

			// Enable the i'th test
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = 0;
			set_cfg_in.notfn = rio_em_notfn_pw;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &tests[i];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_cfg_set(&mock_dev_info,
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
			c_e[0].port_num = 0;
			c_e[0].event = tests[i].em_event;
			c_e[1].port_num = 0;
			c_e[1].event = tests[t].em_event;

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = 0;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = true;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(1, out_p.num_events);
			assert_false(out_p.too_many);
			assert_true(out_p.other_events);

			// Check that the event created was found
			assert_int_equal(tests[i].em_event, stat_e[0].event);

			// Clear all port-write events...
			in_c.num_events = out_p.num_events;
			in_c.events = in_p.events;

			out_c.imp_rc = 0xFFFF;
			out_c.failure_idx = 0xFF;
			out_c.pw_events_remain = true;
			out_c.int_events_remain = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_clr_events(&mock_dev_info,
							&in_c, &out_c));
			assert_int_equal(0, out_c.imp_rc);
			assert_int_equal(0, out_c.failure_idx);
			assert_true(out_c.pw_events_remain);
			assert_true(out_c.int_events_remain);

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = 0;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_e;
			memset(stat_e, 0xFF, sizeof(stat_e));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = true;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			assert_int_equal(0, out_p.num_events);
			assert_false(out_p.too_many);
			assert_true(out_p.other_events);
		}
	}

	(void)state;
}

// Test that if one event is configured with port-write
// notification and another is configured with interrupt notification,
// that when the events are created and cleared the 
// "other events" fields behave correctly.
//
// This test is skipped if interrupts are not supported.

static void tsi721_rio_em_clr_int_pw_events_other_events_test(void **state)
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

	// NOTE: A 2many_pna event also causes an err_rate event.
	// For this reason, "2many_pna" must be the last test, 
	// and err_rate must occur before 2many_pna.
	//
	// Note that the rio_em_f_err_rate.em_info value excludes
	// RIO_EM_REC_ERR_SET_CS_NOT_ACC, so the events
	// for rio_em_f_err_rate and rio_em_f_2many_pna are exclusive of
	// each other.
	rio_em_cfg_t tests[] = {
		{rio_em_f_los, rio_em_detect_on, 1 * 256 * 1000},
		{rio_em_f_port_err, rio_em_detect_on, 0},
		{rio_em_f_err_rate, rio_em_detect_on, 0x1000000F},
		{rio_em_d_log, rio_em_detect_on,
					TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
					TSI721_LOCAL_ERR_EN_ILL_ID_EN},
		{rio_em_i_sig_det, rio_em_detect_on, 0},
		{rio_em_i_rst_req, rio_em_detect_on, 0},
		{rio_em_i_init_fail, rio_em_detect_on, 0},
		{rio_em_f_2many_retx, rio_em_detect_on, 0x0010},
		{rio_em_f_2many_pna, rio_em_detect_on, 0x0010}
	};

	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i, p;
	Tsi721_test_state_t *l_st = (Tsi721_test_state_t *)*state;
	rio_em_notfn_ctl_t i_notfn, p_notfn;

	// Cannot run the interrupt test if interrupts are not supported,
	// or on real hardware.
	if (!tsi721_int_supported) {
		return;
	}

	if (l_st->real_hw) {
		return;
	}

	for (i = 0; i < test_cnt; i++) {
		for (p = 0; p < test_cnt; p++) {
			// Must have two different events for this test.
			if (i == p) {
				continue;
			}
			// init_fail events can only use pw notification
			if (rio_em_i_init_fail == tests[p].em_event) {
				continue;
			}
			// 2many_retx & 2many_pna both create the same event,
			// so skip this test configuration.
			if ((rio_em_f_2many_retx == tests[i].em_event) &&
				(rio_em_f_2many_pna == tests[p].em_event)) {
				continue;
			}
			if ((rio_em_f_2many_retx == tests[p].em_event) &&
				(rio_em_f_2many_pna == tests[i].em_event)) {
				continue;
			}
			if (DEBUG_PRINTF) {
				printf("\ni = %d p = %d event = %d %d\n", i, p,
						tests[i].em_event,
						tests[p].em_event);
			}
			// This test requires a clean slate at the beginning 
			// of each attempt
			tsi721_em_setup(state);

			// Enable detection of the current event.
			// NOTE: Only one of tests[i].em_event and
			// tests[p].em_event can be true at any one time.
			if (rio_em_i_rst_req == tests[i].em_event) {
				// If we're testing the Reset Request
				// event, do the disable/enable since this
				// events detection is actually controlled by 
				// Port Config functionality.

				assert_int_equal(RIO_SUCCESS,
						DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
				if (rio_em_detect_off == tests[i].em_detect) {
					plm_imp_spec_ctl |= t_mask;
				} else {
					plm_imp_spec_ctl &= ~t_mask;
				}
				assert_int_equal(RIO_SUCCESS,
						DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
			}
			if (rio_em_i_rst_req == tests[p].em_event) {
				// If we're testing the Reset Request
				// event, do the disable/enable since this
				// events detection is actually controlled by 
				// Port Config functionality.

				assert_int_equal(RIO_SUCCESS,
						DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
				if (rio_em_detect_off == tests[p].em_detect) {
					plm_imp_spec_ctl |= t_mask;
				} else {
					plm_imp_spec_ctl &= ~t_mask;
				}
				assert_int_equal(RIO_SUCCESS,
						DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
			}

			// Enable the i'th and p'th event
			// Special notification case for f_err_rate and
			// 2many_pna: Both are port_fail events, so notification
			// must be "both".
			i_notfn = rio_em_notfn_int;
			p_notfn = rio_em_notfn_pw;
			if (((rio_em_f_err_rate == tests[i].em_event)
					|| (rio_em_f_2many_pna
							== tests[i].em_event))
					&& ((rio_em_f_err_rate
							== tests[p].em_event)
							|| (rio_em_f_2many_pna
									== tests[p].em_event))) {
				i_notfn = rio_em_notfn_both;
				p_notfn = rio_em_notfn_both;
			}
			// Enable event with interrupt notification
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = 0;
			set_cfg_in.notfn = i_notfn;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &tests[i];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_cfg_set(&mock_dev_info,
							&set_cfg_in,
							&set_cfg_out));
			assert_int_equal(0, set_cfg_out.imp_rc);
			assert_int_equal(RIO_ALL_PORTS,
					set_cfg_out.fail_port_num);
			assert_int_equal(rio_em_last, set_cfg_out.fail_idx);
			assert_int_equal(i_notfn, set_cfg_out.notfn);

			// Enable event with port-write notification
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = 0;
			set_cfg_in.notfn = p_notfn;
			set_cfg_in.num_events = 1;
			set_cfg_in.events = &tests[p];

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_cfg_set(&mock_dev_info,
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
			c_e[0].port_num = 0;
			c_e[0].event = tests[i].em_event;
			c_e[1].port_num = 0;
			c_e[1].event = tests[p].em_event;

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event interrupt status
			in_i.ptl.num_ports = 1;
			in_i.ptl.pnums[0] = 0;
			in_i.num_events = (uint8_t)rio_em_last;
			in_i.events = stat_i;
			memset(stat_i, 0xFF, sizeof(stat_i));

			out_i.imp_rc = 0;
			out_i.num_events = 0xFF;
			out_i.too_many = false;
			out_i.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			if (rio_em_notfn_int == i_notfn) {
				assert_int_equal(1, out_i.num_events);
				assert_false(out_i.too_many);
				assert_true(out_i.other_events);
				assert_int_equal(stat_i[0].port_num, 0);
				assert_int_equal(stat_i[0].event,
						tests[i].em_event);
			} else {
				bool found;

				assert_int_equal(2, out_i.num_events);
				assert_false(out_i.too_many);
				assert_false(out_i.other_events);
				assert_int_equal(stat_i[0].port_num, 0);
				assert_int_equal(stat_i[1].port_num, 0);

				found =
						((stat_i[0].event
								== tests[p].em_event)
								&& (stat_i[1].event
										== tests[i].em_event))
								|| ((stat_i[0].event
										== tests[i].em_event)
										&& (stat_i[1].event
												== tests[p].em_event));
				assert_true(found);
			}

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = 0;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_p;
			memset(stat_p, 0xFF, sizeof(stat_p));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = false;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			if (rio_em_notfn_pw == p_notfn) {
				assert_int_equal(1, out_p.num_events);
				assert_false(out_p.too_many);
				// init_fail is an interrupt-only event,
				// so it cannot trigger "other events" for
				// a port-write
				if (rio_em_i_init_fail == tests[i].em_event) {
					assert_false(out_p.other_events);
				} else {
					assert_true(out_p.other_events);
				}
				assert_int_equal(stat_p[0].port_num, 0);
				assert_int_equal(stat_p[0].event,
						tests[p].em_event);
			} else {
				bool found;

				assert_int_equal(2, out_p.num_events);
				assert_false(out_p.too_many);
				assert_false(out_p.other_events);
				assert_int_equal(stat_p[0].port_num, 0);
				assert_int_equal(stat_p[1].port_num, 0);
				found =
						((stat_p[0].event
								== tests[p].em_event)
								&& (stat_p[1].event
										== tests[i].em_event))
								|| ((stat_p[0].event
										== tests[i].em_event)
										&& (stat_p[1].event
												== tests[p].em_event));
				assert_true(found);
			}

			// Clear all interrupt events...
			in_c.num_events = out_i.num_events;
			in_c.events = in_i.events;

			out_c.imp_rc = 0xFFFF;
			out_c.failure_idx = 0xFF;
			out_c.pw_events_remain = true;
			out_c.int_events_remain = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_clr_events(&mock_dev_info,
							&in_c, &out_c));
			assert_int_equal(0, out_c.imp_rc);
			assert_int_equal(0, out_c.failure_idx);
			// if notfn_pw != p_notfn, notfn_both == p_notfn
			// which implies that the events are 2many_pna
			// and err_rate, which are generally cleared together.
			if (rio_em_notfn_pw == p_notfn) {
				assert_true(out_c.pw_events_remain);
				assert_true(out_c.int_events_remain);
			} else {
				assert_false(out_c.pw_events_remain);
				assert_false(out_c.int_events_remain);
			}

			// Clear all port-write events...
			in_c.num_events = out_p.num_events;
			in_c.events = in_p.events;

			out_c.imp_rc = 0xFFFF;
			out_c.failure_idx = 0xFF;
			out_c.pw_events_remain = true;
			out_c.int_events_remain = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_clr_events(&mock_dev_info,
							&in_c, &out_c));
			assert_int_equal(0, out_c.imp_rc);
			assert_int_equal(0, out_c.failure_idx);
			assert_false(out_c.pw_events_remain);
			assert_false(out_c.int_events_remain);

			// Query the event interrupt status, confirm they're 
			// gone.
			in_i.ptl.num_ports = 1;
			in_i.ptl.pnums[0] = 0;
			in_i.num_events = (uint8_t)rio_em_last;
			in_i.events = stat_i;
			memset(stat_i, 0xFF, sizeof(stat_i));

			out_i.imp_rc = 0;
			out_i.num_events = 0xFF;
			out_i.too_many = false;
			out_i.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(0, out_i.num_events);
			assert_false(out_i.too_many);
			assert_false(out_i.other_events);

			// Query port-write events, confirm they're gone
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = 0;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_p;
			memset(stat_p, 0xFF, sizeof(stat_p));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = false;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_pw_stat(
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
//
// This test is skipped if interrupts are not supported.

static void tsi721_rio_em_clr_int_pw_events_both_test(void **state)
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

	// NOTE: A 2many_pna event also causes an err_rate event.
	// For this reason, "2many_pna" must be the last test, 
	// and err_rate must occur before 2many_pna.
	//
	// Note that the rio_em_f_err_rate.em_info value excludes
	// RIO_EM_REC_ERR_SET_CS_NOT_ACC, so the events
	// for rio_em_f_err_rate and rio_em_f_2many_pna are exclusive of
	// each other.
	rio_em_cfg_t tests[] = {
			{rio_em_f_los, rio_em_detect_on, 1 * 256 * 1000}, {
					rio_em_f_port_err, rio_em_detect_on, 0},
			{rio_em_f_err_rate, rio_em_detect_on, 0x1000000F}, {
					rio_em_f_2many_retx, rio_em_detect_on,
					0x0010}, {rio_em_d_log,
					rio_em_detect_on,
					TSI721_LOCAL_ERR_EN_ILL_TYPE_EN |
					TSI721_LOCAL_ERR_EN_ILL_ID_EN}, {
					rio_em_i_sig_det, rio_em_detect_on, 0},
			{rio_em_i_rst_req, rio_em_detect_on, 0},
			{rio_em_i_init_fail, rio_em_detect_on, 0}, {
					rio_em_f_2many_pna, rio_em_detect_on,
					0x0010}};

	uint32_t plm_imp_spec_ctl;
	uint32_t t_mask = TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;

	const unsigned int test_cnt = sizeof(tests) / sizeof(tests[0]);
	unsigned int i, p, srch_i, chk_i;
	Tsi721_test_state_t *l_st = (Tsi721_test_state_t *)*state;

	// Cannot run the interrupt test if interrupts are not supported,
	// or on real hardware.
	if (!tsi721_int_supported) {
		return;
	}

	if (l_st->real_hw) {
		return;
	}

	// Use test_cnt - 1 here to avoid trying for rio_em_f_2many_pna 
	// without also setting rio_em_f_err_rate.
	for (i = 0; i < test_cnt - 1; i++) {
		for (p = 0; p < test_cnt; p++) {
			// Must have two different events for this test.
			if (i == p) {
				continue;
			}
			// Cannot cause an init_fail event to generate a
			// port-write
			if (rio_em_i_init_fail == tests[p].em_event) {
				continue;
			}
			if (DEBUG_PRINTF) {
				printf("\ni = %d p = %d event = %d %d\n", i, p,
						tests[i].em_event,
						tests[p].em_event);
			}
			// This test requires a clean slate at the beginning 
			// of each attempt
			tsi721_em_setup(state);

			// Enable detection of the current event.
			// NOTE: Only one of tests[i].em_event and
			// tests[p].em_event can be true at any one time.
			if (rio_em_i_rst_req == tests[i].em_event) {
				// If we're testing the Reset Request
				// event, do the disable/enable since this
				// events detection is actually controlled by 
				// Port Config functionality.

				assert_int_equal(RIO_SUCCESS,
						DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
				if (rio_em_detect_off == tests[i].em_detect) {
					plm_imp_spec_ctl |= t_mask;
				} else {
					plm_imp_spec_ctl &= ~t_mask;
				}
				assert_int_equal(RIO_SUCCESS,
						DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
			}
			if (rio_em_i_rst_req == tests[p].em_event) {
				// If we're testing the Reset Request
				// event, do the disable/enable since this
				// events detection is actually controlled by 
				// Port Config functionality.

				assert_int_equal(RIO_SUCCESS,
						DARRegRead(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, &plm_imp_spec_ctl));
				if (rio_em_detect_off == tests[p].em_detect) {
					plm_imp_spec_ctl |= t_mask;
				} else {
					plm_imp_spec_ctl &= ~t_mask;
				}
				assert_int_equal(RIO_SUCCESS,
						DARRegWrite(&mock_dev_info, TSI721_PLM_IMP_SPEC_CTL, plm_imp_spec_ctl));
			}

			// Configure the i'th and p'th event
			set_cfg_in.ptl.num_ports = 1;
			set_cfg_in.ptl.pnums[0] = 0;
			set_cfg_in.notfn = rio_em_notfn_both;
			set_cfg_in.num_events = 2;
			memcpy(&tests_in[0], &tests[i], sizeof(tests_in[0]));
			memcpy(&tests_in[1], &tests[p], sizeof(tests_in[1]));
			set_cfg_in.events = tests_in;

			set_cfg_out.imp_rc = 0xFFFFFFFF;
			set_cfg_out.fail_port_num = 0x99;
			set_cfg_out.fail_idx = 0xFF;
			set_cfg_out.notfn = (rio_em_notfn_ctl_t)0xF;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_cfg_set(&mock_dev_info,
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
			c_e[0].port_num = 0;
			c_e[0].event = tests[i].em_event;
			c_e[1].port_num = 0;
			c_e[1].event = tests[p].em_event;

			c_out.imp_rc = 0xFFFFFF;
			c_out.failure_idx = 0xff;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_create_events(dev_info,
							&c_in, &c_out));

			assert_int_equal(RIO_SUCCESS, c_out.imp_rc);
			assert_int_equal(0, c_out.failure_idx);

			// Query the event interrupt status
			in_i.ptl.num_ports = 1;
			in_i.ptl.pnums[0] = 0;
			in_i.num_events = (uint8_t)rio_em_last;
			in_i.events = stat_i;
			memset(stat_i, 0xFF, sizeof(stat_i));

			out_i.imp_rc = 0;
			out_i.num_events = 0xFF;
			out_i.too_many = true;
			out_i.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(2, out_i.num_events);
			assert_false(out_i.too_many);
			assert_false(out_i.other_events);
			assert_int_equal(stat_i[0].port_num, 0);
			assert_int_equal(stat_i[1].port_num, 0);

			// Check that events created are found...
			for (chk_i = 0; chk_i <= i; chk_i++) {
				bool found = false;
				if ((chk_i != i) && (chk_i != p)) {
					continue;
				}
				for (srch_i = 0;
						!found
								&& (srch_i
										<= out_i.num_events);
						srch_i++) {
					if (tests[chk_i].em_event
							== stat_i[srch_i].event) {
						found = true;
					}
				}
				if (!found && DEBUG_PRINTF) {
					printf(
							"i %d event_cnt %d chk_i %d event %d",
							i, out_p.num_events,
							chk_i,
							tests[chk_i].em_event);
				}
				assert_true(found);
			}

			// Query the event port-write status
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = 0;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_p;
			memset(stat_p, 0xFF, sizeof(stat_p));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = true;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_pw_stat(
							&mock_dev_info, &in_p,
							&out_p));
			assert_int_equal(0, out_p.imp_rc);
			// It is not possible to detect a pw init_fail event
			// so reduce the event count by 1.
			if (rio_em_i_init_fail == tests[i].em_event) {
				assert_int_equal(1, out_p.num_events);
				assert_int_equal(stat_p[0].port_num, 0);
			} else {
				assert_int_equal(2, out_p.num_events);
				assert_int_equal(stat_p[0].port_num, 0);
				assert_int_equal(stat_p[1].port_num, 0);
			}
			assert_false(out_p.too_many);
			assert_false(out_p.other_events);

			// Check that events created are found...
			for (chk_i = 0; chk_i <= i; chk_i++) {
				bool found = false;
				if ((chk_i != i) && (chk_i != p)) {
					continue;
				}
				if ((chk_i == i)
						&& (rio_em_i_init_fail
								== tests[i].em_event)) {
					continue;
				}
				for (srch_i = 0;
						!found
								&& (srch_i
										<= out_i.num_events);
						srch_i++) {
					if (tests[chk_i].em_event
							== stat_p[srch_i].event) {
						found = true;
					}
				}
				if (!found && DEBUG_PRINTF) {
					printf(
							"i %d event_cnt %d chk_i %d event %d",
							i, out_p.num_events,
							chk_i,
							tests[chk_i].em_event);
				}
				assert_true(found);
			}

			// init_fail is an interrupt only event, clear it
			// first...
			if (rio_em_i_init_fail == tests[i].em_event) {
				in_c.num_events = 1;
				in_c.events = c_e;
				c_e[0].port_num = 0;
				c_e[0].event = rio_em_i_init_fail;

				out_c.imp_rc = 0xFFFF;
				out_c.failure_idx = 0xFF;
				out_c.pw_events_remain = true;
				out_c.int_events_remain = true;

				assert_int_equal(RIO_SUCCESS,
						tsi721_rio_em_clr_events(
								&mock_dev_info,
								&in_c, &out_c));
				assert_int_equal(0, out_c.imp_rc);
				assert_int_equal(0, out_c.failure_idx);
				assert_true(out_c.pw_events_remain);
				assert_true(out_c.int_events_remain);
			}

			// Clear all port-write events...
			in_c.num_events = out_p.num_events;
			in_c.events = in_p.events;

			out_c.imp_rc = 0xFFFF;
			out_c.failure_idx = 0xFF;
			out_c.pw_events_remain = true;
			out_c.int_events_remain = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_clr_events(&mock_dev_info,
							&in_c, &out_c));
			assert_int_equal(0, out_c.imp_rc);
			assert_int_equal(0, out_c.failure_idx);
			assert_false(out_c.pw_events_remain);
			assert_false(out_c.int_events_remain);

			// Query the event interrupt status, confirm they're 
			// gone.
			in_i.ptl.num_ports = 1;
			in_i.ptl.pnums[0] = 0;
			in_i.num_events = (uint8_t)rio_em_last;
			in_i.events = stat_i;
			memset(stat_i, 0xFF, sizeof(stat_i));

			out_i.imp_rc = 0;
			out_i.num_events = 0xFF;
			out_i.too_many = false;
			out_i.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_int_stat(
							&mock_dev_info, &in_i,
							&out_i));
			assert_int_equal(0, out_i.imp_rc);
			assert_int_equal(0, out_i.num_events);
			assert_false(out_i.too_many);
			assert_false(out_i.other_events);

			// Query port-write events, confirm they're gone
			in_p.ptl.num_ports = 1;
			in_p.ptl.pnums[0] = 0;
			in_p.num_events = (uint8_t)rio_em_last;
			in_p.events = stat_p;
			memset(stat_p, 0xFF, sizeof(stat_p));

			out_p.imp_rc = 0;
			out_p.num_events = 0xFF;
			out_p.too_many = false;
			out_p.other_events = true;

			assert_int_equal(RIO_SUCCESS,
					tsi721_rio_em_get_pw_stat(
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
			tsi721_rio_em_dev_rpt_ctl_success_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_dev_rpt_ctl_bad_parms_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_em_cfg_pw_success_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_em_cfg_pw_bad_parms_test,
			tsi721_em_setup),

	cmocka_unit_test_setup(
			tsi721_rio_em_cfg_set_success_em_info_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_cfg_set_ignore_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_cfg_set_err_rate_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_cfg_set_err_rate_unsup_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_cfg_set_fail_em_info_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_cfg_set_roundup_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_cfg_set_bad_parms_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_cfg_get_bad_parms_test,
			tsi721_em_setup),

	cmocka_unit_test_setup(
			tsi721_rio_em_parse_pw_no_events_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_parse_pw_all_events_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_parse_pw_oth_events_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_parse_pw_bad_parms_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_create_events_bad_parms_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_create_events_success_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_create_ignored_events_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_get_int_stat_bad_parms_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_get_int_stat_success_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_get_int_stat_other_events_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_get_pw_stat_bad_parms_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_get_pw_stat_success_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_get_pw_stat_other_events_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_get_int_pw_stat_other_events_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_get_int_pw_stat_both_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_clr_events_bad_parms_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_clr_int_events_success_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_clr_pw_events_success_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_clr_int_events_other_events_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_clr_pw_events_other_events_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_clr_int_pw_events_other_events_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_clr_int_pw_events_both_test,
			tsi721_em_setup),
	cmocka_unit_test_setup(
			tsi721_rio_em_cfg_pw_retx_compute_test,
			tsi721_em_setup),
	};

	return cmocka_run_group_tests(tests, tsi721_em_grp_setup,
			tsi721_em_grp_teardown);
}

#endif /* TSI721_DAR_WANTED */

#ifdef __cplusplus
}
#endif

