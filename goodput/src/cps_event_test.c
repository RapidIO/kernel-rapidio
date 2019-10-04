/*
****************************************************************************
Copyright (c) 2015, Integrated Device Technology Inc.
Copyright (c) 2015, RapidIO Trade Association
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

#include <stdint.h>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include "rio_route.h"
#include "string_util.h"
#include "libcli.h"
#include "rio_mport_lib.h"
#include "liblog.h"

#include "libtime_utils.h"
#include "worker.h"
#include "goodput.h"
#include "Tsi721.h"
#include "CPS1848.h"
#include "CPS1616.h"
#include "rio_misc.h"
#include "did.h"
#include "tsi721_reset.h"
#include "pw_handling.h"

#include "RapidIO_Error_Management_API.h"
#include "RapidIO_Port_Config_API.h"
#include "RapidIO_Routing_Table_API.h"

#ifdef __cplusplus
extern "C" {
#endif

/* test_init_switch_handle
 *
 * Initializes a handle for use by librio routines.
 */

uint32_t test_init_switch_handle(DAR_DEV_INFO_t *dev_h)
{
	uint32_t ret;
	uint32_t rc = 1;
	uint32_t temp_devid;
	uint32_t switch_comp_tag;

	ret = SRIO_API_ReadRegFunc(dev_h, RIO_DEV_IDENT, &temp_devid);
	if (ret) {
		ERR("Reading device identity failed: 0x%x", ret);
		goto exit;
	}

	dev_h->devID = temp_devid;
	dev_h->driver_family = rio_get_driver_family(temp_devid);
	ret = DAR_Find_Driver_for_Device(1, dev_h);
	if (RIO_SUCCESS != ret) {
		ERR("Unable to find driver for switch, type 0x%x\n, ret 0x%x",
				temp_devid, ret);
		rc = EOPNOTSUPP;
		goto exit;
	}
	if (!(SWITCH(dev_h))) {
		ERR("Link partner is not a switch, type 0x%x\n, ret 0x%x",
				temp_devid, ret);
		rc = EOPNOTSUPP;
		goto exit;
	}
	ret = DARrioGetComponentTag(dev_h, &switch_comp_tag);
	if (RIO_SUCCESS != ret) {
		ERR("Could not read switch component tag: 0x%x\n", ret)
		rc = EOPNOTSUPP;
		goto exit;
	}
	if (SWITCH_COMPTAG == switch_comp_tag) {
		rc = 0;
		goto exit;
	}

	ret = DARrioSetComponentTag(dev_h, SWITCH_COMPTAG);
	if (RIO_SUCCESS != ret) {
		ERR("Could not write switch component tag: 0x%x\n", ret)
		goto exit;
	}

	rc = 0;
exit:
	INFO("Init switch handle EXIT rc %d...\n", rc);
	return rc;
}

/* test_config_pw_gen
 *
 * Configures CPS1848 port-write control registers.
 *
 * Note: Routing of port-writes is controlled by the global routing table.
 */

uint32_t test_config_pw_gen(DAR_DEV_INFO_t *dev_h)
{
	uint32_t ret;
        rio_em_cfg_pw_in_t pw_in;
        rio_em_cfg_pw_out_t pw_out;

	pw_in.deviceID_tt = tt_dev8;
        pw_in.port_write_destID = PW_DID;
        pw_in.srcID_valid = 1;
        pw_in.port_write_srcID = SWITCH_DEVID;
        pw_in.priority = 3;
        pw_in.CRF = 1;

        pw_in.port_write_re_tx = (100000000 + 351)/352;

        ret = rio_em_cfg_pw(dev_h, &pw_in, &pw_out);
        if (ret) {
                ERR("CONFIG_SWITCH: Config PW Err 0x%8x 0x%8x\n",
                        ret, pw_out.imp_rc);
		goto exit;	
        }

	ret = SRIO_API_WriteRegFunc(dev_h, CPS1848_RTE_DEFAULT_PORT_CSR,
					CPS_RT_NO_ROUTE);
exit:
	return ret;
}

typedef struct {
	uint32_t base;
	uint32_t pp_oset;
	char *lable;
	uint32_t mask;
} cps_evt_chk_regs_t;

typedef struct {
	uint32_t base;
	uint32_t val;
} chk_reg_t;

uint32_t write_test_reg(DAR_DEV_INFO_t *dev_h, rio_port_t pt,
			const cps_evt_chk_regs_t *chk,
			const chk_reg_t *val)
{
	uint32_t ret = 1;
	uint32_t rmw;
	uint32_t addr = chk->base + (chk->pp_oset * pt);

	if (val->val & ~chk->mask) {
		ERR("Base 0x%x Mask 0x%x Val 0x%x Bits set outside mask\n");
		goto exit;
	}
	ret = SRIO_API_ReadRegFunc(dev_h, addr, &rmw);
	if (ret)
		goto exit;
	rmw &= ~chk->mask;
	rmw |= val->val;
	ret = SRIO_API_WriteRegFunc(dev_h, addr, rmw);
exit:
	return ret;
}

uint32_t write_regs_for_test(DAR_DEV_INFO_t *dev_h, rio_port_t pt,
			const int num_chks, const cps_evt_chk_regs_t *chk,
			const int num_regs, const chk_reg_t *val)
{
	int found_it;
	uint32_t ret = 1;

	for (int i = 0; i < num_regs; i++) {
		found_it = 0;
		for (int j = 0; j < num_chks; j++) {
			if (val[i].base == chk[j].base) {
				found_it = 1;
				ret = write_test_reg(dev_h, pt,
							&chk[j], &val[i]);
				if (ret) {
					ERR("I %d Base 0x%x V 0x%x Err 0x%x\n",
					i, val[i].base, val[i].val, ret);
					goto exit;
				}
			}
		}
		if (!found_it) {
			ERR("I %d Base 0x%x V 0x%x not found.\n",
				i, val[i].base, val[i].val);
			goto exit;
		}
	}
	INFO("Wrote %d registers\n", num_regs);
	ret = 0;
exit:
	return ret;
}

uint32_t check_test_reg(DAR_DEV_INFO_t *dev_h, rio_port_t pt,
			const cps_evt_chk_regs_t *chk,
			const chk_reg_t *val)
{
	uint32_t ret = 1;
	uint32_t reg;
	uint32_t addr = chk->base + (chk->pp_oset * pt);

	if (val->val & ~chk->mask) {
		ERR("Base 0x%x Mask 0x%x Val 0x%x Bits set outside mask\n",
			chk->base, chk->mask, val->val);
		goto exit;
	}
	ret = SRIO_API_ReadRegFunc(dev_h, addr, &reg);
	if (ret)
		goto exit;
	reg &= chk->mask;
	if (reg != val->val) {
		ret = 2;
		ERR("Base 0x%x Mask 0x%x Val 0x%x Reg 0x%x Err 0x%x\n",
			chk->base, chk->mask, val->val, reg, val->val ^ reg);
		goto exit;
	}
	ret = 0;
exit:
	return ret;
}

uint32_t check_regs_for_test(DAR_DEV_INFO_t *dev_h, rio_port_t pt,
			const int num_chks, const cps_evt_chk_regs_t *chk,
			const int num_regs, const chk_reg_t *val)
{
	int found_it;
	uint32_t ret = 1;

	for (int i = 0; i < num_regs; i++) {
		found_it = 0;
		for (int j = 0; j < num_chks; j++) {
			if (val[i].base == chk[j].base) {
				found_it = 1;
				ret = check_test_reg(dev_h, pt,
							&chk[j], &val[i]);
				if (ret) {
					ERR("I %d Base 0x%x V 0x%x Err 0x%x\n",
					i, val[i].base, val[i].val, ret);
					goto exit;
				}
			}
		}
		if (!found_it) {
			ERR("I %d Base 0x%x V 0x%x not found.\n",
				i, val[i].base, val[i].val);
			goto exit;
		}
	}
	INFO("Checked %d registers\n", num_regs);
	ret = 0;
exit:
	return ret;
}

/* cps_evt_cfg_chk_regs
 *
 * Constant listing offsets and names of registers used to control link
 * initialization, "loss of sync" link failure and
 * time to live event detection reporting.
 *
 */

const cps_evt_chk_regs_t cps_evt_cfg_chk_regs[] = {
        {CPS1848_PORT_X_LOCAL_ACKID_CSR(0),
			CPS1848_PORT_X_LOCAL_ACKID_CSR(1) -
			CPS1848_PORT_X_LOCAL_ACKID_CSR(0),
			(char *)"ACKIDs",
			CPS1848_PORT_X_LOCAL_ACKID_CSR_CLR |
			CPS1848_PORT_X_LOCAL_ACKID_CSR_INBOUND |
			CPS1848_PORT_X_LOCAL_ACKID_CSR_OUTSTD |
			CPS1848_PORT_X_LOCAL_ACKID_CSR_OUTBOUND},
        {CPS1848_PORT_X_ERR_STAT_CSR(0),
			CPS1848_PORT_X_ERR_STAT_CSR(1) -
			CPS1848_PORT_X_ERR_STAT_CSR(0),
			(char *)"ErrStat",
			CPS1848_PORT_X_ERR_STAT_CSR_PW_PNDG |
			CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_FAIL},
        {CPS1848_PORT_X_CTL_1_CSR(0),
			CPS1848_PORT_X_CTL_1_CSR(1) -
			CPS1848_PORT_X_CTL_1_CSR(0),
			(char *)"CTL",
			CPS1848_PORT_X_CTL_1_CSR_PORT_LOCKOUT |
			CPS1848_PORT_X_CTL_1_CSR_DROP_PKT_EN |
			CPS1848_PORT_X_CTL_1_CSR_STOP_ON_PORT_FAIL_ENC_EN |
			CPS1848_PORT_X_CTL_1_CSR_INPUT_PORT_EN |
			CPS1848_PORT_X_CTL_1_CSR_OUTPUT_PORT_EN |
			CPS1848_PORT_X_CTL_1_CSR_PORT_DIS},
        {CPS1848_PORT_X_OPS(0),
			CPS1848_PORT_X_OPS(1) - CPS1848_PORT_X_OPS(0),
			(char *)"OPS",
			CPS1848_PORT_X_OPS_PORT_PW_EN |
			CPS1848_PORT_X_OPS_PORT_INT_EN},
        {CPS1848_PORT_X_ERR_RATE_EN_CSR(0),
			CPS1848_PORT_X_ERR_RATE_EN_CSR(1) -
			CPS1848_PORT_X_ERR_RATE_EN_CSR(0),
			(char *)"RATE_EN",
			CPS1848_PORT_X_ERR_RATE_EN_CSR_DELIN_ERR_EN |
			CPS1848_PORT_X_ERR_RATE_EN_CSR_PRTCL_ERR_EN |
			CPS1848_PORT_X_ERR_RATE_EN_CSR_IMP_SPEC_ERR_EN},
        {CPS1848_PORT_X_ERR_RPT_EN(0),
			CPS1848_PORT_X_ERR_RPT_EN(1) -
			CPS1848_PORT_X_ERR_RPT_EN(0),
			(char *)"ErrRptE",
			CPS1848_PORT_X_ERR_RPT_EN_LINK_TIMEOUT_EN |
			CPS1848_PORT_X_ERR_RPT_EN_PRTCL_ERR_EN |
			CPS1848_PORT_X_ERR_RPT_EN_IMP_SPEC_ERR_EN},
        {CPS1848_PORT_X_ERR_DET_CSR(0),
			CPS1848_PORT_X_ERR_DET_CSR(1) -
			CPS1848_PORT_X_ERR_DET_CSR(0),
			(char *)"ErrDet",
			CPS1848_PORT_X_ERR_DET_CSR_LINK_TIMEOUT |
			CPS1848_PORT_X_ERR_DET_CSR_CS_ACK_ILL |
			CPS1848_PORT_X_ERR_DET_CSR_DELIN_ERR |
			CPS1848_PORT_X_ERR_DET_CSR_PRTCL_ERR |
			CPS1848_PORT_X_ERR_DET_CSR_LR_ACKID_ILL |
			CPS1848_PORT_X_ERR_DET_CSR_IDLE1_ERR |
			CPS1848_PORT_X_ERR_DET_CSR_PKT_ILL_SIZE |
			CPS1848_PORT_X_ERR_DET_CSR_PKT_CRC_ERR |
			CPS1848_PORT_X_ERR_DET_CSR_PKT_ILL_ACKID |
			CPS1848_PORT_X_ERR_DET_CSR_CS_NOT_ACC |
			CPS1848_PORT_X_ERR_DET_CSR_UNEXP_ACKID |
			CPS1848_PORT_X_ERR_DET_CSR_CS_CRC_ERR |
			CPS1848_PORT_X_ERR_DET_CSR_IMP_SPEC_ERR},
        {CPS1848_PORT_X_ERR_RATE_CSR(0),
			CPS1848_PORT_X_ERR_RATE_CSR(1) -
			CPS1848_PORT_X_ERR_RATE_CSR(0),
			(char *)"ERRRATE",
			CPS1848_PORT_X_ERR_RATE_CSR_ERR_RATE_CNTR |
			CPS1848_PORT_X_ERR_RATE_CSR_PEAK_ERR_RATE |
			CPS1848_PORT_X_ERR_RATE_CSR_ERR_RATE_REC |
			CPS1848_PORT_X_ERR_RATE_CSR_ERR_RATE_BIAS},
        {CPS1848_PORT_X_ERR_RATE_THRESH_CSR(0),
			CPS1848_PORT_X_ERR_RATE_THRESH_CSR(1) -
			CPS1848_PORT_X_ERR_RATE_THRESH_CSR(0),
			(char *)"ERR_THR",
			CPS1848_PORT_X_ERR_RATE_THRESH_CSR_DEGR_THRESH |
			CPS1848_PORT_X_ERR_RATE_THRESH_CSR_FAIL_THRESH},
        {CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(0),
			CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(1) -
			CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(0),
			(char *)"IERRRAT",
			CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_LOA_EN |
			CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_PORT_INIT_EN |
			CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_TTL_EVENT_EN |
			CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_ERR_RATE_EN},
        {CPS1848_PORT_X_IMPL_SPEC_ERR_DET(0),
			CPS1848_PORT_X_IMPL_SPEC_ERR_DET(1) -
			CPS1848_PORT_X_IMPL_SPEC_ERR_DET(0),
			(char *)"IERRdet",
			CPS1848_PORT_X_IMPL_SPEC_ERR_DET_LOA |
			CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PORT_INIT |
			CPS1848_PORT_X_IMPL_SPEC_ERR_DET_ERR_RATE |
			CPS1848_PORT_X_IMPL_SPEC_ERR_DET_TTL_EVENT},
        {CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(0),
			CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(1) -
			CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(0),
			(char *)"IERRrpt",
			CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_LOA_EN |
			CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_PORT_INIT_EN |
			CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_TTL_EVENT_EN |
			CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_ERR_RATE_EN},
        {CPS1616_LANE_X_ERR_RATE_EN(0), 0x400, (char *)"L0 EN",
			CPS1616_LANE_X_ERR_RATE_EN_LANE_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_RDY_EN |
			CPS1616_LANE_X_ERR_RATE_EN_BAD_CHAR_EN |
			CPS1616_LANE_X_ERR_RATE_EN_DESCRAM_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_TX_RX_MISMATCH_EN |
			CPS1616_LANE_X_ERR_RATE_EN_IDLE2_FRAME_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_INVER_DET_EN |
			CPS1616_LANE_X_ERR_RATE_EN_BAD_SPEED_EN},
        {CPS1616_LANE_X_ERR_RATE_EN(1), 0x400, (char *)"L1 EN",
			CPS1616_LANE_X_ERR_RATE_EN_LANE_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_RDY_EN |
			CPS1616_LANE_X_ERR_RATE_EN_BAD_CHAR_EN |
			CPS1616_LANE_X_ERR_RATE_EN_DESCRAM_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_TX_RX_MISMATCH_EN |
			CPS1616_LANE_X_ERR_RATE_EN_IDLE2_FRAME_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_INVER_DET_EN |
			CPS1616_LANE_X_ERR_RATE_EN_BAD_SPEED_EN},
        {CPS1616_LANE_X_ERR_RATE_EN(2), 0x400, (char *)"L2 EN",
			CPS1616_LANE_X_ERR_RATE_EN_LANE_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_RDY_EN |
			CPS1616_LANE_X_ERR_RATE_EN_BAD_CHAR_EN |
			CPS1616_LANE_X_ERR_RATE_EN_DESCRAM_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_TX_RX_MISMATCH_EN |
			CPS1616_LANE_X_ERR_RATE_EN_IDLE2_FRAME_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_INVER_DET_EN |
			CPS1616_LANE_X_ERR_RATE_EN_BAD_SPEED_EN},
        {CPS1616_LANE_X_ERR_RATE_EN(3), 0x400, (char *)"L3 EN",
			CPS1616_LANE_X_ERR_RATE_EN_LANE_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_RDY_EN |
			CPS1616_LANE_X_ERR_RATE_EN_BAD_CHAR_EN |
			CPS1616_LANE_X_ERR_RATE_EN_DESCRAM_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_TX_RX_MISMATCH_EN |
			CPS1616_LANE_X_ERR_RATE_EN_IDLE2_FRAME_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_INVER_DET_EN |
			CPS1616_LANE_X_ERR_RATE_EN_BAD_SPEED_EN},
	{CPS1848_PKT_TTL_CSR,		0,	(char *)"TTL",
			CPS1848_PKT_TTL_CSR_TTL}
};

const int num_chks = sizeof(cps_evt_cfg_chk_regs)
		/ sizeof(cps_evt_cfg_chk_regs[0]);

/* test_config_events_on_port
 *
 * Tests configuration of events on a port.
 * cfg_vals_b4: Register values to be written before running the test
 * cfg_vals_after_en: Register values when all events are enabled
 * cfg_vals_after_dis: Register values when all events are disabled
 *
 */

uint32_t test_config_events_on_port(struct cli_env	*env,
				DAR_DEV_INFO_t	*dev_h,
				rio_port_t	port)
{
	rio_em_cfg_set_in_t cfg_in;
	rio_em_cfg_set_out_t cfg_out;
	const int SIG = 0;
	const int LOS = 1;
	const int TTL = 2;
	rio_em_cfg_t enables[TTL + 1];

	const chk_reg_t cfg_vals_b4[] = {

        {CPS1848_PORT_X_CTL_1_CSR(0),
			CPS1848_PORT_X_CTL_1_CSR_INPUT_PORT_EN |
			CPS1848_PORT_X_CTL_1_CSR_OUTPUT_PORT_EN},
        {CPS1848_PORT_X_OPS(0), 0},
        {CPS1848_PORT_X_ERR_RATE_EN_CSR(0),
			CPS1848_PORT_X_ERR_RATE_EN_CSR_DELIN_ERR_EN |
			CPS1848_PORT_X_ERR_RATE_EN_CSR_PRTCL_ERR_EN},
        {CPS1848_PORT_X_ERR_RPT_EN(0),
			CPS1848_PORT_X_ERR_RPT_EN_LINK_TIMEOUT_EN |
			CPS1848_PORT_X_ERR_RPT_EN_PRTCL_ERR_EN},
        {CPS1848_PORT_X_ERR_RATE_CSR(0),
			CPS1848_PORT_X_ERR_RATE_CSR_ERR_RATE_CNTR |
			CPS1848_PORT_X_ERR_RATE_CSR_PEAK_ERR_RATE |
			CPS1848_PORT_X_ERR_RATE_CSR_ERR_RATE_REC |
			CPS1848_PORT_X_ERR_RATE_CSR_ERR_RATE_BIAS},
        {CPS1848_PORT_X_ERR_RATE_THRESH_CSR(0),
			CPS1848_PORT_X_ERR_RATE_THRESH_CSR_DEGR_THRESH |
			CPS1848_PORT_X_ERR_RATE_THRESH_CSR_FAIL_THRESH},
        {CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(0),
			CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_TTL_EVENT_EN},
        {CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(0), 0},
        {CPS1616_LANE_X_ERR_RATE_EN(0),
			CPS1616_LANE_X_ERR_RATE_EN_BAD_CHAR_EN |
			CPS1616_LANE_X_ERR_RATE_EN_DESCRAM_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_TX_RX_MISMATCH_EN |
			CPS1616_LANE_X_ERR_RATE_EN_IDLE2_FRAME_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_INVER_DET_EN |
			CPS1616_LANE_X_ERR_RATE_EN_BAD_SPEED_EN},
        {CPS1616_LANE_X_ERR_RATE_EN(1),
			CPS1616_LANE_X_ERR_RATE_EN_BAD_CHAR_EN |
			CPS1616_LANE_X_ERR_RATE_EN_DESCRAM_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_TX_RX_MISMATCH_EN |
			CPS1616_LANE_X_ERR_RATE_EN_IDLE2_FRAME_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_INVER_DET_EN |
			CPS1616_LANE_X_ERR_RATE_EN_BAD_SPEED_EN},
        {CPS1616_LANE_X_ERR_RATE_EN(2),
			CPS1616_LANE_X_ERR_RATE_EN_BAD_CHAR_EN |
			CPS1616_LANE_X_ERR_RATE_EN_DESCRAM_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_TX_RX_MISMATCH_EN |
			CPS1616_LANE_X_ERR_RATE_EN_IDLE2_FRAME_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_INVER_DET_EN |
			CPS1616_LANE_X_ERR_RATE_EN_BAD_SPEED_EN},
        {CPS1616_LANE_X_ERR_RATE_EN(3),
			CPS1616_LANE_X_ERR_RATE_EN_BAD_CHAR_EN |
			CPS1616_LANE_X_ERR_RATE_EN_DESCRAM_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_TX_RX_MISMATCH_EN |
			CPS1616_LANE_X_ERR_RATE_EN_IDLE2_FRAME_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_INVER_DET_EN |
			CPS1616_LANE_X_ERR_RATE_EN_BAD_SPEED_EN},
	{CPS1848_PKT_TTL_CSR, 0},
	};

	const chk_reg_t cfg_vals_after_en[] = {
        {CPS1848_PORT_X_CTL_1_CSR(0),
			CPS1848_PORT_X_CTL_1_CSR_INPUT_PORT_EN |
			CPS1848_PORT_X_CTL_1_CSR_OUTPUT_PORT_EN |
			CPS1848_PORT_X_CTL_1_CSR_DROP_PKT_EN |
			CPS1848_PORT_X_CTL_1_CSR_STOP_ON_PORT_FAIL_ENC_EN},
        {CPS1848_PORT_X_OPS(0), CPS1848_PORT_X_OPS_PORT_PW_EN},
        {CPS1848_PORT_X_ERR_RATE_EN_CSR(0),
			CPS1848_PORT_X_ERR_RATE_EN_CSR_IMP_SPEC_ERR_EN},
        {CPS1848_PORT_X_ERR_RPT_EN(0),
			CPS1848_PORT_X_ERR_RPT_EN_IMP_SPEC_ERR_EN},
        {CPS1848_PORT_X_ERR_RATE_CSR(0),
			CPS1848_PORT_X_ERR_RATE_CSR_ERR_RATE_REC},
        {CPS1848_PORT_X_ERR_RATE_THRESH_CSR(0), 0x01000000},
        {CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(0),
			CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_LOA_EN},
        {CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(0),
			CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_PORT_INIT_EN |
			CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_ERR_RATE_EN},
        {CPS1616_LANE_X_ERR_RATE_EN(0),
			CPS1616_LANE_X_ERR_RATE_EN_LANE_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_RDY_EN},
        {CPS1616_LANE_X_ERR_RATE_EN(1),
			CPS1616_LANE_X_ERR_RATE_EN_LANE_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_RDY_EN},
        {CPS1616_LANE_X_ERR_RATE_EN(2),
			CPS1616_LANE_X_ERR_RATE_EN_LANE_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_RDY_EN},
        {CPS1616_LANE_X_ERR_RATE_EN(3),
			CPS1616_LANE_X_ERR_RATE_EN_LANE_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_RDY_EN},
	{CPS1848_PKT_TTL_CSR, 0x02710000}, // 625
	};

	const chk_reg_t cfg_vals_after_dis[] = {
        {CPS1848_PORT_X_CTL_1_CSR(0),
			CPS1848_PORT_X_CTL_1_CSR_INPUT_PORT_EN |
			CPS1848_PORT_X_CTL_1_CSR_OUTPUT_PORT_EN},
        {CPS1848_PORT_X_OPS(0), CPS1848_PORT_X_OPS_PORT_PW_EN},
        {CPS1848_PORT_X_ERR_RATE_EN_CSR(0), 0},
        {CPS1848_PORT_X_ERR_RPT_EN(0), 0},
        {CPS1848_PORT_X_ERR_RATE_CSR(0), 0},
        {CPS1848_PORT_X_ERR_RATE_THRESH_CSR(0), 0},
        {CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(0), 0},
        {CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(0), 0},
        {CPS1616_LANE_X_ERR_RATE_EN(0), 0},
        {CPS1616_LANE_X_ERR_RATE_EN(1), 0},
        {CPS1616_LANE_X_ERR_RATE_EN(2), 0},
        {CPS1616_LANE_X_ERR_RATE_EN(3), 0},
	{CPS1848_PKT_TTL_CSR, 0}, // 5320
	};

	const int num_vals_b4 = sizeof(cfg_vals_b4)
			/ sizeof(cfg_vals_b4[0]);
	const int num_vals_after_en = sizeof(cfg_vals_after_en)
			/ sizeof(cfg_vals_after_en[0]);
	const int num_vals_after_dis = sizeof(cfg_vals_after_dis)
			/ sizeof(cfg_vals_after_dis[0]);

	INFO("Start\n");

	uint32_t ret = write_regs_for_test(dev_h, port, num_chks, cps_evt_cfg_chk_regs,
				num_vals_b4, cfg_vals_b4);
	if (ret) {
		ERR("write_regs_for_test rc 0x%x\n", ret);
		goto exit;
	}

	cfg_in.ptl.num_ports = 1;
	cfg_in.ptl.pnums[0] = port;
	cfg_in.notfn = rio_em_notfn_pw;
	cfg_in.num_events = 3;
	cfg_in.events = enables;
	cfg_in.events[SIG].em_event = rio_em_i_sig_det;
	cfg_in.events[SIG].em_detect = rio_em_detect_on;
	cfg_in.events[SIG].em_info = 1; // Fifty millisceconds
	cfg_in.events[LOS].em_event = rio_em_f_los;
	cfg_in.events[LOS].em_detect = rio_em_detect_on;
	cfg_in.events[LOS].em_info = 0;
	cfg_in.events[TTL].em_event = rio_em_d_ttl;
	cfg_in.events[TTL].em_detect = rio_em_detect_on;
	cfg_in.events[TTL].em_info = 1000000;

	ret = rio_em_cfg_set(dev_h, &cfg_in, &cfg_out);
	if (ret) {
		ERR("Ret 0x%x Imp Rc 0x%x\n", ret, cfg_out.imp_rc);
		goto exit;
	}

	ret = check_regs_for_test(dev_h, port, num_chks, cps_evt_cfg_chk_regs,
				num_vals_after_en, cfg_vals_after_en);
	if (ret) {
		ERR("First check_regs_for_test rc 0x%x\n", ret);
		goto exit;
	}

	cfg_in.ptl.num_ports = 1;
	cfg_in.ptl.pnums[0] = port;
	cfg_in.notfn = rio_em_notfn_pw;
	cfg_in.num_events = 3;
	cfg_in.events = enables;
	cfg_in.events[SIG].em_event = rio_em_i_sig_det;
	cfg_in.events[SIG].em_detect = rio_em_detect_off;
	cfg_in.events[SIG].em_info = 1; // Fifty millisceconds
	cfg_in.events[LOS].em_event = rio_em_f_los;
	cfg_in.events[LOS].em_detect = rio_em_detect_off;
	cfg_in.events[LOS].em_info = 0;
	cfg_in.events[TTL].em_event = rio_em_d_ttl;
	cfg_in.events[TTL].em_detect = rio_em_detect_off;
	cfg_in.events[TTL].em_info = 0;

	ret = rio_em_cfg_set(dev_h, &cfg_in, &cfg_out);
	if (ret) {
		ERR("Ret 0x%x Imp Rc 0x%x\n", ret, cfg_out.imp_rc);
		goto exit;
	}

	ret = check_regs_for_test(dev_h, port, num_chks, cps_evt_cfg_chk_regs,
				num_vals_after_dis, cfg_vals_after_dis);
	if (ret) {
		ERR("Second check_regs_for_test rc 0x%x\n", ret);
		goto exit;
	}

	cfg_in.ptl.num_ports = 1;
	cfg_in.ptl.pnums[0] = port;
	cfg_in.notfn = rio_em_notfn_pw;
	cfg_in.num_events = 3;
	cfg_in.events = enables;
	cfg_in.events[SIG].em_event = rio_em_i_sig_det;
	cfg_in.events[SIG].em_detect = rio_em_detect_on;
	cfg_in.events[SIG].em_info = 1; // Fifty millisceconds
	cfg_in.events[LOS].em_event = rio_em_f_los;
	cfg_in.events[LOS].em_detect = rio_em_detect_on;
	cfg_in.events[LOS].em_info = 0;
	cfg_in.events[TTL].em_event = rio_em_d_ttl;
	cfg_in.events[TTL].em_detect = rio_em_detect_on;
	cfg_in.events[TTL].em_info = 1000000;

	ret = rio_em_cfg_set(dev_h, &cfg_in, &cfg_out);
	if (ret) {
		ERR("Ret 0x%x Imp Rc 0x%x\n", ret, cfg_out.imp_rc);
		goto exit;
	}

	ret = check_regs_for_test(dev_h, port, num_chks, cps_evt_cfg_chk_regs,
				num_vals_after_en, cfg_vals_after_en);
	if (ret) {
		ERR("Third check_regs_for_test rc 0x%x\n", ret);
		goto exit;
	}
exit:
	if (ret) {
		LOGMSG(env, "test_config_events_on_port Failed 0x%x\n", ret);
	}

	return ret;
}

/* test_create_events_on_port
 *
 * Tests that all events can be created under software control.
 *
 */

uint32_t test_create_events_on_port(struct cli_env	*env,
				DAR_DEV_INFO_t	*dev_h,
				rio_port_t	port)
{
	uint32_t ret;
	rio_em_event_n_loc_t events[2];
	rio_em_create_events_in_t cr_in;
	rio_em_create_events_out_t cr_out;

	const chk_reg_t events_chk[] = {
        {CPS1848_PORT_X_ERR_STAT_CSR(0),
			CPS1848_PORT_X_ERR_STAT_CSR_PW_PNDG |
			CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_FAIL},
        {CPS1848_PORT_X_ERR_DET_CSR(0),
			CPS1848_PORT_X_ERR_DET_CSR_IMP_SPEC_ERR},
        {CPS1848_PORT_X_IMPL_SPEC_ERR_DET(0),
			CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PORT_INIT},
	};
	const int num_vals_chk = sizeof(events_chk)
			/ sizeof(events_chk[0]);

	INFO("Start\n");

	cr_in.num_events = 2;
	cr_in.events = events;
	events[0].port_num = port;
	events[0].event = rio_em_f_los;
	events[1].port_num = port;
	events[1].event = rio_em_i_sig_det;

	ret = rio_em_create_events(dev_h, &cr_in, &cr_out);
	if (ret) {
		ERR("rio_em_create_events ret 0x%x\n", ret);
		goto exit;
	}

	ret = check_regs_for_test(dev_h, port, num_chks, cps_evt_cfg_chk_regs,
				num_vals_chk, events_chk);
	if (ret) {
		ERR("check_regs_for_test rc 0x%x\n", ret);
		goto exit;
	}
exit:
	if (ret) {
		LOGMSG(env, "test_create_events_on_port Failed 0x%x\n", ret);
	}

	return ret;
}
/* test_detect_events_on_port
 *
 * Tests that all events can be detected by software.
 *
 */

uint32_t test_detect_events_on_port(struct cli_env	*env,
				DAR_DEV_INFO_t	*dev_h,
				rio_port_t	port)
{
	uint32_t ret;
	rio_em_event_n_loc_t events[3];
	rio_em_get_pw_stat_in_t ev_in;
	rio_em_get_pw_stat_out_t ev_out;

	INFO("Start\n");

	ev_in.ptl.num_ports = 1;
	ev_in.ptl.pnums[0] = port;
	ev_in.pw_port_num = port;
	ev_in.num_events = 3;
	ev_in.events = events;
	ret = rio_em_get_pw_stat(dev_h, &ev_in, &ev_out);
	if (ret) {
		ERR("rio_em_get_pw_stat ret 0x%x\n", ret);
		goto exit;
	}

	if (ev_out.num_events != 3) {
		ERR("Num events = %d not 3\n", ev_out.num_events);
        	for (int i = 0; i < ev_out.num_events; i++) {
                	INFO("%d: P%d : %s\n", i, ev_in.events[i].port_num,
                        	EVENT_NAME_STR(ev_in.events[i].event));
		}
		goto exit;
	}
	if (ev_out.too_many) {
		ERR("Too many = %d not 0\n", ev_out.too_many);
		goto exit;
	}
	if (ev_out.other_events) {
		ERR("Other events = %d not 0\n", ev_out.other_events);
		goto exit;
	}
	if (ev_in.events[0].port_num != port) {
		ERR("0 port_num = %d not %d\n", ev_in.events[0].port_num, port);
		goto exit;
	}
	if (ev_in.events[0].event != rio_em_f_los) {
		ERR("0 event %d not %d\n", ev_in.events[0].event, rio_em_f_los);
		goto exit;
	}
	if (ev_in.events[1].port_num != port) {
		ERR("1 port_num = %d not %d\n", ev_in.events[1].port_num, port);
		goto exit;
	}
	if (ev_in.events[1].event != rio_em_i_sig_det) {
		ERR("1 event %d not %d\n", ev_in.events[1].event, rio_em_i_sig_det);
		goto exit;
	}
	if (ev_in.events[2].port_num != port) {
		ERR("2 port_num = %d not %d\n", ev_in.events[2].port_num, port);
		goto exit;
	}
	if (ev_in.events[2].event != rio_em_a_clr_pwpnd) {
		ERR("2 event %d not %d\n", ev_in.events[2].event, rio_em_a_clr_pwpnd);
		goto exit;
	}
exit:
	if (ret) {
		LOGMSG(env, "test_detect_events_on_port Failed 0x%x\n", ret);
	}

	return ret;
}

/* test_clear_events_on_port
 *
 * Tests that all events can be cleared by software.
 *
 */

uint32_t test_clear_events_on_port(struct cli_env	*env,
				DAR_DEV_INFO_t	*dev_h,
				rio_port_t	port)
{
	uint32_t ret;
	const int LOS = 0;
	const int SIG = 1;
	const int PWPDG = 2;
	rio_em_event_n_loc_t events[3];
	rio_em_clr_events_in_t ev_in;
	rio_em_clr_events_out_t ev_out;
	rio_em_cfg_set_in_t cfg_in;
	rio_em_cfg_set_out_t cfg_out;
	const int TTL = 2;
	rio_em_cfg_t enables[TTL + 1];
	const chk_reg_t cfg_vals_b4[] = {
        {CPS1848_PORT_X_LOCAL_ACKID_CSR(0),
			CPS1848_PORT_X_LOCAL_ACKID_CSR_CLR | 0x04001313},
	};

	const chk_reg_t events_chk[] = {
        {CPS1848_PORT_X_LOCAL_ACKID_CSR(0), 0},
        {CPS1848_PORT_X_ERR_STAT_CSR(0), 0},
        {CPS1848_PORT_X_ERR_DET_CSR(0), 0},
        {CPS1848_PORT_X_IMPL_SPEC_ERR_DET(0), 0},
	};
	const int num_vals_b4 = sizeof(cfg_vals_b4)/sizeof(cfg_vals_b4[0]);
	const int num_vals_chk = sizeof(events_chk)
			/ sizeof(events_chk[0]);

	INFO("Start\n");

	ret = write_regs_for_test(dev_h, port, num_chks, cps_evt_cfg_chk_regs,
				num_vals_b4, cfg_vals_b4);
	if (ret) {
		ERR("write_regs_for_test rc 0x%x\n", ret);
		goto exit;
	}

	cfg_in.ptl.num_ports = 1;
	cfg_in.ptl.pnums[0] = port;
	cfg_in.notfn = rio_em_notfn_pw;
	cfg_in.num_events = 3;
	cfg_in.events = enables;
	cfg_in.events[SIG].em_event = rio_em_i_sig_det;
	cfg_in.events[SIG].em_detect = rio_em_detect_off;
	cfg_in.events[SIG].em_info = 1; // Fifty millisceconds
	cfg_in.events[LOS].em_event = rio_em_f_los;
	cfg_in.events[LOS].em_detect = rio_em_detect_off;
	cfg_in.events[LOS].em_info = 0;
	cfg_in.events[TTL].em_event = rio_em_d_ttl;
	cfg_in.events[TTL].em_detect = rio_em_detect_off;
	cfg_in.events[TTL].em_info = 0;

	ret = rio_em_cfg_set(dev_h, &cfg_in, &cfg_out);
	if (ret) {
		ERR("Ret 0x%x Imp Rc 0x%x\n", ret, cfg_out.imp_rc);
		goto exit;
	}

	events[0].port_num = events[1].port_num = events[2].port_num = port;
	events[LOS].event = rio_em_f_los;
	events[SIG].event = rio_em_i_sig_det;
	events[PWPDG].event = rio_em_a_clr_pwpnd;

	ev_in.num_events = 3;
	ev_in.events = events;
	ret = rio_em_clr_events(dev_h, &ev_in, &ev_out);
	if (ret) {
		ERR("rio_em_clr_events ret 0x%x\n", ret);
		goto exit;
	}

	ret = check_regs_for_test(dev_h, port, num_chks, cps_evt_cfg_chk_regs,
				num_vals_chk, events_chk);
	if (ret) {
		ERR("check_regs_for_test rc 0x%x\n", ret);
		goto exit;
	}
exit:
	if (ret) {
		LOGMSG(env, "test_clear_events_on_port Failed 0x%x\n", ret);
	}

	return ret;
}
/* cps_event_handling_test performs a unit test of the CPS_EM.c event handling
 * routines.  The scope is limited to handling of rio_em_f_los (Loss of Signal),
 * rio_em_f_err_rate, rio_em_d_ttl and rio_em_i_sig_det events.
 *
 * Configuration (enable/disable), event creation, event detection, and event
 * clearing are tested.  Only port-write notification is tested.
 *
 * the test assumes that the switch device is directly connected to the
 * endpoint.
 */
void cps_event_handling_test(struct cli_env *env, rio_port_t port)
{
	int ret = 0;
	DAR_DEV_INFO_t dev_h;
        librio_status sw_h;

	ret = DAR_proc_ptr_init(SRIO_API_ReadRegFunc, SRIO_API_WriteRegFunc,
				SRIO_API_DelayFunc);

	/* First, get device handle. */
	ret = test_init_switch_handle(&dev_h);
	if (RIO_SUCCESS != ret) {
		LOGMSG(env, "Failed initializing switch handle: 0x%08x\n", ret);
		return;
	}

	if (port >= NUM_PORTS((&dev_h))) {
		LOGMSG(env, "Port number %d, switch max %d. Fail.\n",
			port, NUM_PORTS(&dev_h));
		return;
	}

	ret = set_switch_port_enables(&dev_h, &sw_h, 1);
	if (RIO_SUCCESS != ret) {
		LOGMSG(env, "Failed set_switch_port_enables:0x%08x\n", ret);
		return;
	}

	ret = test_config_pw_gen(&dev_h);
	if (RIO_SUCCESS != ret) {
		LOGMSG(env, "Failed test_config_pw_gen:0x%08x\n", ret);
		return;
	}

	for (int i = 0; i < 10; i++) {
		ret = test_config_events_on_port(env, &dev_h, port);
		if (RIO_SUCCESS != ret) {
			LOGMSG(env, "test_config_events_on_port %d:0x%08x\n",
				i, ret);
			return;
		}

		ret = test_create_events_on_port(env, &dev_h, port);
		if (RIO_SUCCESS != ret) {
			LOGMSG(env, "test_create_events_on_port %d:0x%08x\n",
				i, ret);
			return;
		}

		ret = test_detect_events_on_port(env, &dev_h, port);
		if (RIO_SUCCESS != ret) {
			LOGMSG(env, "test_detect_events_on_port %d:0x%08x\n",
				i, ret);
			return;
		}

		ret = test_clear_events_on_port(env, &dev_h, port);
		if (RIO_SUCCESS != ret) {
			LOGMSG(env, "test_clear_events_on_port %d:0x%08x\n",
				i, ret);
			return;
		}
		LOGMSG(env, "Iteration %d Passed.\n", i);
	}
}


#ifdef __cplusplus
}
#endif
