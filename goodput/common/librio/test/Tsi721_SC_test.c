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

static rio_sc_dev_ctrs_t *mock_dev_ctrs = NULL;
static rio_sc_p_ctrs_val_t *pp_ctrs = NULL;

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
		rc = rio_maint_write(st.mp_h, st.did_reg_val, st.hc, offset, 4,
				writedata);
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

static void tsi721_init_mock_reg(void **state)
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
		if (st.mp_h < 0) {
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

static int tsi721_sc_grp_setup(void **state)
{
	if (!tsi721_grp_setup(state)) {
		mock_dev_ctrs =
				(rio_sc_dev_ctrs_t *)malloc(sizeof(rio_sc_dev_ctrs_t));
		if (NULL == mock_dev_ctrs) {
			goto fail;
		}

		pp_ctrs =
				(rio_sc_p_ctrs_val_t *)malloc((RIO_MAX_PORTS)* sizeof(rio_sc_p_ctrs_val_t));
		if (NULL == pp_ctrs) {
			goto fail;
		}
	}
	return 0;

fail:
	free(mock_dev_ctrs);
	free(pp_ctrs);
	mock_dev_ctrs = NULL;
	pp_ctrs = NULL;

	return -1;
}

static int tsi721_sc_grp_teardown(void **state)
{
	tsi721_grp_teardown(state);
	free(mock_dev_ctrs);
	free(pp_ctrs);

	return 0;
}

/* The setup function which should be called before any unit tests that need to be executed.
 */
static int tsi721_sc_setup(void **state)
{
	uint8_t idx, pnum;
	rio_sc_ctr_val_t init = INIT_RIO_SC_CTR_VAL;

	memset(&mock_dev_info, 0, sizeof(mock_dev_info));
	memset(mock_dev_ctrs, 0, sizeof(rio_sc_dev_ctrs_t));
	memset(pp_ctrs, 0, sizeof(rio_sc_p_ctrs_val_t));

	tsi721_init_mock_dev_info();
	mock_dev_ctrs->num_p_ctrs = RIO_MAX_PORTS;
	mock_dev_ctrs->valid_p_ctrs = 0;

	for (pnum = 0; pnum < RIO_MAX_PORTS; pnum++) {
		pp_ctrs[pnum].pnum = pnum;
		pp_ctrs[pnum].ctrs_cnt = TSI721_NUM_PERF_CTRS;
		for (idx = 0; idx < RIO_MAX_SC; idx++) {
			pp_ctrs[pnum].ctrs[idx] = init;
		}
	}
	mock_dev_ctrs->p_ctrs = pp_ctrs;

	tsi721_init_mock_reg(state);

	(void)state; // unused
	return 0;
}

static void tsi721_init_ctrs(rio_sc_init_dev_ctrs_in_t *parms_in)
{
	uint8_t pnum;

	parms_in->ptl.num_ports = RIO_ALL_PORTS;
	for (pnum = 0; pnum < RIO_MAX_DEV_PORT; pnum++) {
		parms_in->ptl.pnums[pnum] = 0x00;
	}

	parms_in->dev_ctrs = mock_dev_ctrs;
}

static void tsi721_init_dev_ctrs_test_success(void **state)
{
	rio_sc_init_dev_ctrs_in_t mock_sc_in;
	rio_sc_init_dev_ctrs_out_t mock_sc_out;
	rio_sc_p_ctrs_val_t *p_ctrs;
	unsigned int j;

	// Success case, all ports
	tsi721_init_ctrs(&mock_sc_in);

	assert_int_equal(RIO_SUCCESS,
			tsi721_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	assert_int_equal(1, mock_sc_in.dev_ctrs->valid_p_ctrs);

	p_ctrs = mock_sc_in.dev_ctrs->p_ctrs;
	assert_int_equal(0, p_ctrs->pnum);
	assert_int_equal(TSI721_NUM_PERF_CTRS, p_ctrs->ctrs_cnt);
	for (j = 0; j < TSI721_NUM_PERF_CTRS; j++) {
		assert_int_equal(0, p_ctrs->ctrs[j].total);
		assert_int_equal(0, p_ctrs->ctrs[j].last_inc);
		assert_int_equal(tsi721_dev_ctrs[j].ctr_t, p_ctrs->ctrs[j].sc);
		assert_int_equal(tsi721_dev_ctrs[j].tx, p_ctrs->ctrs[j].tx);
		assert_int_equal(tsi721_dev_ctrs[j].srio, p_ctrs->ctrs[j].srio);
	}
	(void)state; // unused
}

static void tsi721_init_dev_ctrs_test_bad_ptrs(void **state)
{
	rio_sc_init_dev_ctrs_in_t mock_sc_in;
	rio_sc_init_dev_ctrs_out_t mock_sc_out;

	// Test invalid dev_ctrs pointer
	tsi721_init_ctrs(&mock_sc_in);
	mock_sc_in.dev_ctrs = NULL;
	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	// Test invalid dev_ctrs->p_ctrs pointer
	tsi721_init_ctrs(&mock_sc_in);
	mock_sc_in.dev_ctrs->p_ctrs = NULL;
	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	(void)state; // unused
}

static void tsi721_init_dev_ctrs_test_bad_p_ctrs(void **state)
{
	rio_sc_init_dev_ctrs_in_t mock_sc_in;
	rio_sc_init_dev_ctrs_out_t mock_sc_out;

	// Test invalid number of p_ctrs
	tsi721_init_ctrs(&mock_sc_in);
	mock_sc_in.dev_ctrs->num_p_ctrs = 0;
	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	tsi721_init_ctrs(&mock_sc_in);
	mock_sc_in.dev_ctrs->num_p_ctrs = RIO_MAX_PORTS + 1;
	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	tsi721_init_ctrs(&mock_sc_in);
	mock_sc_in.dev_ctrs->valid_p_ctrs = TSI721_MAX_PORTS + 1;
	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	(void)state; // unused
}

static void tsi721_init_dev_ctrs_test_bad_ptl_1(void **state)
{
	rio_sc_init_dev_ctrs_in_t mock_sc_in;
	rio_sc_init_dev_ctrs_out_t mock_sc_out;

	// Test that a bad Port list is reported correctly.
	tsi721_init_ctrs(&mock_sc_in);
	mock_sc_in.ptl.num_ports = 1;
	mock_sc_in.ptl.pnums[0] = 1;
	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	(void)state; // unused
}

static void tsi721_init_dev_ctrs_test_bad_ptl_2(void **state)
{
	rio_sc_init_dev_ctrs_in_t mock_sc_in;
	rio_sc_init_dev_ctrs_out_t mock_sc_out;

	tsi721_init_ctrs(&mock_sc_in);
	mock_sc_in.ptl.num_ports = 1;
	mock_sc_in.ptl.pnums[0] = -1;
	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	(void)state; // unused
}

static void tsi721_init_dev_ctrs_test_good_ptl(void **state)
{
	rio_sc_init_dev_ctrs_in_t mock_sc_in;
	rio_sc_init_dev_ctrs_out_t mock_sc_out;
	rio_sc_p_ctrs_val_t *p_ctrs;
	unsigned int j;

	// Test Port list with a few good entries...
	tsi721_init_ctrs(&mock_sc_in);
	mock_sc_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			tsi721_rio_sc_init_dev_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	assert_int_equal(1, mock_sc_in.dev_ctrs->valid_p_ctrs);
	assert_int_equal(0, mock_sc_in.dev_ctrs->p_ctrs[0].pnum);
	assert_int_equal(TSI721_NUM_PERF_CTRS,
			mock_sc_in.dev_ctrs->p_ctrs[0].ctrs_cnt);

	p_ctrs = mock_sc_in.dev_ctrs->p_ctrs;
	for (j = 0; j < TSI721_NUM_PERF_CTRS; j++) {
		assert_int_equal(0, p_ctrs->ctrs[j].total);
		assert_int_equal(0, p_ctrs->ctrs[j].last_inc);
		assert_int_equal(tsi721_dev_ctrs[j].ctr_t, p_ctrs->ctrs[j].sc);
		assert_int_equal(tsi721_dev_ctrs[j].tx, p_ctrs->ctrs[j].tx);
		assert_int_equal(tsi721_dev_ctrs[j].srio, p_ctrs->ctrs[j].srio);
	}
	(void)state; // unused
}

static void tsi721_init_read_ctrs(rio_sc_read_ctrs_in_t *parms_in)
{
	parms_in->ptl.num_ports = RIO_ALL_PORTS;
	parms_in->dev_ctrs = mock_dev_ctrs;
}

static void tsi721_read_dev_ctrs_test(void **state)
{
	rio_sc_read_ctrs_in_t mock_sc_in;
	rio_sc_read_ctrs_out_t mock_sc_out;
	rio_sc_init_dev_ctrs_in_t init_in;
	rio_sc_init_dev_ctrs_out_t init_out;
	unsigned int idx, ridx;
	rio_sc_ctr_val_t *ctrs;
	uint64_t wrap_base = 0x00000000FFFFFFFF;
	const int ridx_start = 0x10;

	// Tsi721 performance counters are not writeable,
	// so we can't run this test on real hardware.
	Tsi721_test_state_t *l_st = *(Tsi721_test_state_t **)state;
	if (l_st->real_hw) {
		return;
	}

	// Initialize counters structure
	tsi721_init_ctrs(&init_in);
	assert_int_equal(RIO_SUCCESS,
			tsi721_rio_sc_init_dev_ctrs(&mock_dev_info, &init_in,
					&init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Set up counters 
	tsi721_init_read_ctrs(&mock_sc_in);

	// Set up counter registers
	for (idx = 0; idx < TSI721_NUM_PERF_CTRS; idx++) {
		uint32_t data, chkdata;
		// Set non-zero counter value for the port
		if (tsi721_dev_ctrs[idx].split && !tsi721_dev_ctrs[idx].os) {
			continue;
		}
		ridx = ridx_start + idx;
		if (tsi721_dev_ctrs[idx].split) {
			data = (ridx << 17) + ridx;
		} else {
			data = ridx;
		}
		assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info,
						tsi721_dev_ctrs[idx].os, data));
		assert_int_equal(RIO_SUCCESS,
				DARRegRead(&mock_dev_info,
						tsi721_dev_ctrs[idx].os,
						&chkdata));
		assert_int_equal(data, chkdata);
		assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info,
						tsi721_dev_ctrs[idx].os, data));
	}

	// Check for successfull reads...
	assert_int_equal(RIO_SUCCESS,
			tsi721_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	// Check counter values... 
	ctrs = &mock_sc_in.dev_ctrs->p_ctrs[0].ctrs[0];
	for (idx = 0; idx < TSI721_NUM_PERF_CTRS; idx++) {
		// Check the counter value for the port...
		if (tsi721_dev_ctrs[idx].split && !tsi721_dev_ctrs[idx].os) {
			continue;
		}
		ridx = ridx_start + idx;
		if (!tsi721_dev_ctrs[idx].split) {
			assert_int_equal(ridx, ctrs[idx].total);
			assert_int_equal(ridx, ctrs[idx].last_inc);
			continue;
		}
		assert_int_equal(2 * ridx, ctrs[idx].total);
		assert_int_equal(2 * ridx, ctrs[idx].last_inc);
		assert_int_equal(ridx, ctrs[idx + 1].total);
		assert_int_equal(ridx, ctrs[idx + 1].last_inc);
	}

	// Change counter registers
	for (idx = 0; idx < TSI721_NUM_PERF_CTRS; idx++) {
		uint32_t data;
		// Set non-zero counter value for the port
		if (tsi721_dev_ctrs[idx].split && !tsi721_dev_ctrs[idx].os) {
			continue;
		}
		ridx = ridx_start + idx;
		if (tsi721_dev_ctrs[idx].split) {
			data = (ridx << 18) + (3 * ridx);
		} else {
			data = 3 * ridx;
		}
		assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info,
						tsi721_dev_ctrs[idx].os, data));
	}

	// Check for successfull reads...
	assert_int_equal(RIO_SUCCESS,
			tsi721_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	// Check counter values... 
	for (idx = 0; idx < TSI721_NUM_PERF_CTRS; idx++) {
		// Check the counter value for the port...
		if (tsi721_dev_ctrs[idx].split && !tsi721_dev_ctrs[idx].os) {
			continue;
		}
		ridx = ridx_start + idx;

		if (!tsi721_dev_ctrs[idx].split) {
			assert_int_equal(4 * ridx, ctrs[idx].total);
			assert_int_equal(3 * ridx, ctrs[idx].last_inc);
			continue;
		}
		assert_int_equal(6 * ridx, ctrs[idx].total);
		assert_int_equal(4 * ridx, ctrs[idx].last_inc);
		assert_int_equal(4 * ridx, ctrs[idx + 1].total);
		assert_int_equal(3 * ridx, ctrs[idx + 1].last_inc);
	}

	// Set all totals registers to wrap over 32 bit boundary...
	for (idx = 0; idx < TSI721_NUM_PERF_CTRS; idx++) {
		// Check the counter value for the port...
		ctrs[idx].total = wrap_base;
	}

	// Restore the counter registers in case
	// the previous reads cleared the counters,
	// as when we're running on real hardware.
	for (idx = 0; idx < TSI721_NUM_PERF_CTRS; idx++) {
		uint32_t data;
		// Set non-zero counter value for the port
		if (tsi721_dev_ctrs[idx].split && !tsi721_dev_ctrs[idx].os) {
			continue;
		}
		ridx = ridx_start + idx;
		if (tsi721_dev_ctrs[idx].split) {
			data = (ridx << 18) + (3 * ridx);
		} else {
			data = 3 * ridx;
		}
		assert_int_equal(RIO_SUCCESS,
				DARRegWrite(&mock_dev_info,
						tsi721_dev_ctrs[idx].os, data));
	}

	// Read the same values again...
	assert_int_equal(RIO_SUCCESS,
			tsi721_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	// Check counter values... 
	for (idx = 0; idx < TSI721_NUM_PERF_CTRS; idx++) {
		uint64_t tot;
		// Check the counter value for the port...
		if (tsi721_dev_ctrs[idx].split && !tsi721_dev_ctrs[idx].os) {
			continue;
		}
		ridx = ridx_start + idx;
		if (!tsi721_dev_ctrs[idx].split) {
			tot = wrap_base + (3 * ridx);
			assert_int_equal(tot, ctrs[idx].total);
			assert_int_equal(3 * ridx, ctrs[idx].last_inc);
			continue;
		}
		tot = wrap_base + (4 * ridx);
		assert_int_equal(tot, ctrs[idx].total);
		assert_int_equal(4 * ridx, ctrs[idx].last_inc);
		tot = wrap_base + (3 * ridx);
		assert_int_equal(tot, ctrs[idx + 1].total);
		assert_int_equal(3 * ridx, ctrs[idx + 1].last_inc);
	}
	(void)state; // unused
}

static void tsi721_read_dev_ctrs_test_bad_parms1(void **state)
{
	rio_sc_read_ctrs_in_t mock_sc_in;
	rio_sc_read_ctrs_out_t mock_sc_out;
	rio_sc_init_dev_ctrs_in_t init_in;
	rio_sc_init_dev_ctrs_out_t init_out;

	// Initialize counters structure
	tsi721_init_ctrs(&init_in);
	assert_int_equal(RIO_SUCCESS,
			tsi721_rio_sc_init_dev_ctrs(&mock_dev_info, &init_in,
					&init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Set up counters 
	tsi721_init_read_ctrs(&mock_sc_in);

	// Now try some bad parameters/failure test cases
	mock_sc_in.ptl.num_ports = TSI721_MAX_PORTS + 1;
	mock_sc_out.imp_rc = RIO_SUCCESS;
	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	mock_sc_in.ptl.num_ports = 1;
	mock_sc_in.ptl.pnums[0] = TSI721_MAX_PORTS + 1;
	mock_sc_out.imp_rc = RIO_SUCCESS;
	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	mock_sc_in.ptl.num_ports = RIO_ALL_PORTS;
	mock_sc_in.dev_ctrs->p_ctrs = NULL;
	mock_sc_out.imp_rc = RIO_SUCCESS;
	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	mock_sc_in.dev_ctrs = NULL;
	mock_sc_out.imp_rc = RIO_SUCCESS;
	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);
	(void)state; // unused
}

static void tsi721_read_dev_ctrs_test_bad_parms2(void **state)
{
	rio_sc_read_ctrs_in_t mock_sc_in;
	rio_sc_read_ctrs_out_t mock_sc_out;
	rio_sc_init_dev_ctrs_in_t init_in;
	rio_sc_init_dev_ctrs_out_t init_out;

	// Initialize counters structure
	tsi721_init_ctrs(&init_in);
	assert_int_equal(RIO_SUCCESS,
			tsi721_rio_sc_init_dev_ctrs(&mock_dev_info, &init_in,
					&init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);

	// Set up counters 
	tsi721_init_read_ctrs(&mock_sc_in);

	// Try to read a port that is not in the port list.
	tsi721_init_ctrs(&init_in);
	init_in.ptl.num_ports = RIO_ALL_PORTS;
	assert_int_equal(RIO_SUCCESS,
			tsi721_rio_sc_init_dev_ctrs(&mock_dev_info, &init_in,
					&init_out));
	assert_int_equal(RIO_SUCCESS, init_out.imp_rc);
	tsi721_init_read_ctrs(&mock_sc_in);

	mock_sc_in.ptl.num_ports = 1;
	mock_sc_in.ptl.pnums[0] = TSI721_MAX_PORTS;
	mock_sc_out.imp_rc = RIO_SUCCESS;
	assert_int_not_equal(RIO_SUCCESS,
			tsi721_rio_sc_read_ctrs(&mock_dev_info, &mock_sc_in,
					&mock_sc_out));
	assert_int_not_equal(RIO_SUCCESS, mock_sc_out.imp_rc);

	(void)state; // unused
}

static void tsi721_init_read_dev_ctrs_test(void **state)
{
	tsi721_init_dev_ctrs_test_success(state);
	tsi721_read_dev_ctrs_test(state);

	(void)state; // unused
}

int main(int argc, char** argv)
{
	memset(&st, 0, sizeof(st));
	st.argc = argc;
	st.argv = argv;

	const struct CMUnitTest tests[] = {
	cmocka_unit_test_setup(
			tsi721_init_dev_ctrs_test_success,
			tsi721_sc_setup),
	cmocka_unit_test_setup(
			tsi721_init_dev_ctrs_test_bad_ptrs,
			tsi721_sc_setup),
	cmocka_unit_test_setup(
			tsi721_init_dev_ctrs_test_bad_p_ctrs,
			tsi721_sc_setup),
	cmocka_unit_test_setup(
			tsi721_init_dev_ctrs_test_bad_ptl_1,
			tsi721_sc_setup),
	cmocka_unit_test_setup(
			tsi721_init_dev_ctrs_test_bad_ptl_2,
			tsi721_sc_setup),
	cmocka_unit_test_setup(
			tsi721_init_dev_ctrs_test_good_ptl,
			tsi721_sc_setup),

	cmocka_unit_test_setup(
			tsi721_read_dev_ctrs_test,
			tsi721_sc_setup),
	cmocka_unit_test_setup(
			tsi721_read_dev_ctrs_test_bad_parms1,
			tsi721_sc_setup),
	cmocka_unit_test_setup(
			tsi721_read_dev_ctrs_test_bad_parms2,
			tsi721_sc_setup),
	cmocka_unit_test_setup(
			tsi721_init_read_dev_ctrs_test,
			tsi721_sc_setup),
	};

	return cmocka_run_group_tests(tests, tsi721_sc_grp_setup,
			tsi721_sc_grp_teardown);
}

#endif /* TSI721_DAR_WANTED */

#ifdef __cplusplus
}
#endif

