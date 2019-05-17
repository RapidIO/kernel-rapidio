/*
****************************************************************************
Copyright (c) 2017, Integrated Device Technology Inc.
Copyright (c) 2017, RapidIO Trade Association
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
#include <stddef.h>

#include "Tsi721.h"
#include "Tsi721_API.h"
#include "DAR_DB_Private.h"
#include "DSF_DB_Private.h"
#include "RapidIO_Utilities_API.h"
#include "RapidIO_Port_Config_API.h"
#include "RapidIO_Routing_Table_API.h"
#include "RapidIO_Error_Management_API.h"
#include "string_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TSI721_DAR_WANTED

// CHANGES
//
// Check port width computation for get_config and set_config
// Check set_config with "all ports" as an input parameter.  Endless loop?
//

#define EM_UPDATE_RESET_0 EM_FIRST_SUBROUTINE_0

#define NO_RESETS     false
#define MANAGE_RESETS true

#define PC_CLR_ERRS(x) (PC_CLR_ERRS_0+x)

#define UPDATE_RESET(x) (EM_UPDATE_RESET_0+x)

#define PC_SECURE_PORT(x) (PC_SECURE_PORT_0+x)

#define PC_DEV_RESET_CONFIG(x) (PC_DEV_RESET_CONFIG_0+x)

typedef struct spx_ctl2_ls_check_info_t_TAG {
	uint32_t ls_en_val;
	uint32_t ls_sup_val;
	rio_pc_ls_t ls;
	uint32_t prescalar_srv_clk;
} spx_ctl2_ls_check_info_t;

spx_ctl2_ls_check_info_t ls_check[] = {
	{ RIO_SPX_CTL2_GB_1P25_EN , RIO_SPX_CTL2_GB_1P25 , rio_pc_ls_1p25 , 13 },
	{ RIO_SPX_CTL2_GB_2P5_EN  , RIO_SPX_CTL2_GB_2P5  , rio_pc_ls_2p5  , 13 },
	{ RIO_SPX_CTL2_GB_3P125_EN, RIO_SPX_CTL2_GB_3P125, rio_pc_ls_3p125, 16 },
	{ RIO_SPX_CTL2_GB_5P0_EN  , RIO_SPX_CTL2_GB_5P0  , rio_pc_ls_5p0  , 25 },
	{ RIO_SPX_CTL2_GB_6P25_EN , RIO_SPX_CTL2_GB_6P25 , rio_pc_ls_6p25 , 31 },
	{ 0x00000000          , 0x00000000           , rio_pc_ls_last ,  0 }
};

static uint32_t reg_lswap(enum rio_lane_swap_t swap)
{
	uint32_t reg_val = 0;

	switch(swap) {
	default:
	case rio_lswap_none:
		reg_val = (TSI721_PLM_IMP_SPEC_CTL_SWAP_RX_NONE >> 16);
		break;
	case rio_lswap_ABCD_BADC:
		reg_val = (TSI721_PLM_IMP_SPEC_CTL_SWAP_RX_1032 >> 16);
		break;
	case rio_lswap_ABCD_DCBA:
		reg_val = (TSI721_PLM_IMP_SPEC_CTL_SWAP_RX_3210 >> 16);
		break;
	case rio_lswap_ABCD_CDAB:
		reg_val = (TSI721_PLM_IMP_SPEC_CTL_SWAP_RX_2301 >> 16);
		break;
	}
	return reg_val;
}

static enum rio_lane_swap_t lswap(uint32_t reg_val)
{
	enum rio_lane_swap_t swap_val;

	switch(reg_val) {
	default:
	case  (TSI721_PLM_IMP_SPEC_CTL_SWAP_RX_NONE >> 16):
		swap_val = rio_lswap_none;
		break;
	case  (TSI721_PLM_IMP_SPEC_CTL_SWAP_RX_1032 >> 16):
		swap_val = rio_lswap_ABCD_BADC;
		break;
	case  (TSI721_PLM_IMP_SPEC_CTL_SWAP_RX_3210 >> 16):
		swap_val = rio_lswap_ABCD_DCBA;
		break;
	case  (TSI721_PLM_IMP_SPEC_CTL_SWAP_RX_2301 >> 16):
		swap_val = rio_lswap_ABCD_CDAB;
		break;
	}
	return swap_val;
}

// Note: in_parms contains the configuration to change to,
//       out_parms contains the current configuration...
//
static uint32_t tsi721_set_config_with_resets(DAR_DEV_INFO_t *dev_info,
		rio_pc_set_config_in_t *in_parms,
		rio_pc_set_config_out_t *out_parms,
		bool manage_resets)
{
	uint32_t rc = RIO_SUCCESS;
	uint32_t spxCtl, spxCtl2, devCtl, plmCtl;
	uint32_t saved_plmCtl, saved_devCtl;

	rc = DARRegRead(dev_info, TSI721_PLM_IMP_SPEC_CTL, &saved_plmCtl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(2);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI721_DEVCTL, &saved_devCtl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(3);
		goto exit;
	}

	// Default is to ignore resets...
	plmCtl = saved_plmCtl & ~(TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST);
	devCtl = saved_devCtl & ~TSI721_DEVCTL_SR_RST_MODE;

	plmCtl |= TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;
	devCtl |= TSI721_DEVCTL_SR_RST_MODE_SRIO_ONLY;

	rc = DARRegWrite(dev_info, TSI721_PLM_IMP_SPEC_CTL, plmCtl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(8);
		goto exit;
	}

	rc = DARRegWrite(dev_info, TSI721_DEVCTL, devCtl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(9);
		goto exit;
	}

	// Check that RapidIO transmitter is enabled...
	if ((out_parms->pc[0].xmitter_disable != in_parms->pc[0].xmitter_disable)
			|| (out_parms->pc[0].port_lockout
					!= in_parms->pc[0].port_lockout)
			|| (out_parms->pc[0].nmtc_xfer_enable
					!= in_parms->pc[0].nmtc_xfer_enable)
			|| (out_parms->pc[0].pw != in_parms->pc[0].pw)) {

		rc = DARRegRead(dev_info, TSI721_SP_CTL, &spxCtl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SET_CONFIG(0x20);
			goto exit;
		}

		if (in_parms->pc[0].xmitter_disable) {
			spxCtl |= TSI721_SP_CTL_PORT_DIS;
		} else {
			spxCtl &= ~TSI721_SP_CTL_PORT_DIS;
		}

		if (in_parms->pc[0].port_lockout) {
			spxCtl |= TSI721_SP_CTL_PORT_LOCKOUT;
		} else {
			spxCtl &= ~TSI721_SP_CTL_PORT_LOCKOUT;
		}

		if (in_parms->pc[0].nmtc_xfer_enable) {
			spxCtl |= TSI721_SP_CTL_INP_EN | TSI721_SP_CTL_OTP_EN;
		} else {
			spxCtl &=
					~(TSI721_SP_CTL_INP_EN
							| TSI721_SP_CTL_OTP_EN);
		}

		//@sonar:off - c:S3458
		spxCtl &= ~TSI721_SP_CTL_OVER_PWIDTH;
		switch (in_parms->pc[0].pw) {
		case rio_pc_pw_2x:
			spxCtl |= RIO_SPX_CTL_PTW_OVER_2X_NO_4X;
			break;
		case rio_pc_pw_4x:
			spxCtl |= RIO_SPX_CTL_PTW_OVER_NONE;
			break;
		case rio_pc_pw_1x:
		case rio_pc_pw_1x_l0:
			spxCtl |= RIO_SPX_CTL_PTW_OVER_1X_L0;
			break;
		case rio_pc_pw_1x_l2:
			spxCtl |= RIO_SPX_CTL_PTW_OVER_1X_LR;
			break;
		default:
		case rio_pc_pw_1x_l1:
			out_parms->imp_rc = PC_SET_CONFIG(8);
			goto exit;
		}
		//@sonar:on

		rc = DARRegWrite(dev_info, TSI721_SP_CTL, spxCtl);
		if (manage_resets) {
			// Wait a while just in case a reset has occurred, and
			// ignore register access failures here...
			DAR_WaitSec(1000000, 0);

			rc = RIO_SUCCESS;
		}
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SET_CONFIG(0x30);
			goto exit;
		}
	}

	// Configure port speed, if necessary...
	if (out_parms->pc[0].ls != in_parms->pc[0].ls) {
		uint32_t reg;
		uint32_t fiveg_wa_val =
				(in_parms->pc[0].ls >= rio_pc_ls_5p0) ?
						TSI721_WA_VAL_5G :
						TSI721_WA_VAL_3G;

		if (in_parms->pc[0].ls
				>= sizeof(ls_check)
						/ sizeof(spx_ctl2_ls_check_info_t)) {
			out_parms->imp_rc = PC_SET_CONFIG(0x31);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI721_SP_CTL2, &spxCtl2);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SET_CONFIG(0x40);
			goto exit;
		}
		spxCtl2 &= ~(TSI721_SP_CTL2_GB_6P25_EN |
		TSI721_SP_CTL2_GB_5P0_EN |
		TSI721_SP_CTL2_GB_3P125_EN |
		TSI721_SP_CTL2_GB_2P5_EN |
		TSI721_SP_CTL2_GB_1P25_EN);

		spxCtl2 |= ls_check[(int)(in_parms->pc[0].ls)].ls_en_val;
		rc = DARRegWrite(dev_info, TSI721_SP_CTL2, spxCtl2);
		if (manage_resets) {
			// Wait a while just in case a reset/reinitialization has occurred.
			// Ignore register access failures here...
			DAR_WaitSec(1000000, 0);
			rc = RIO_SUCCESS;
		}
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SET_CONFIG(0x41);
			goto exit;
		}

		// Apply 5G training work around, or remove it, as necessary
		for (reg = 0; reg < TSI721_NUM_WA_REGS; reg++) {
			rc = DARRegWrite(dev_info, TSI721_5G_WA_REG0(reg),
					fiveg_wa_val);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SET_CONFIG(0x3A);
				goto exit;
			}
		}

		if (in_parms->pc[0].ls > rio_pc_ls_3p125) {
			for (reg = 0; reg < TSI721_NUM_WA_REGS; reg++) {
				rc = DARRegWrite(dev_info,
						TSI721_5G_WA_REG1(reg), 0);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = PC_SET_CONFIG(0x3C);
					goto exit;
				}
			}
		}
		// Update PRESCALAR_SRV_CLK value
		rc =
				DARRegWrite(dev_info, TSI721_PRESCALAR_SRV_CLK,
						ls_check[(int)(in_parms->pc[0].ls)].prescalar_srv_clk);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SET_CONFIG(0x3E);
			goto exit;
		}
	}

	// Check for lane swapping & inversion
	if ((out_parms->pc[0].tx_lswap != in_parms->pc[0].tx_lswap)
		|| (out_parms->pc[0].rx_lswap != in_parms->pc[0].rx_lswap)) {
		rc = DARRegRead(dev_info, TSI721_PLM_IMP_SPEC_CTL, &plmCtl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SET_CONFIG(0x40);
			goto exit;
		}

		plmCtl &= ~(TSI721_PLM_IMP_SPEC_CTL_SWAP_RX |
			TSI721_PLM_IMP_SPEC_CTL_SWAP_TX);

		plmCtl |= reg_lswap(in_parms->pc[0].rx_lswap) << 16;
		plmCtl |= reg_lswap(in_parms->pc[0].tx_lswap) << 18;

		rc = DARRegWrite(dev_info, TSI721_PLM_IMP_SPEC_CTL, plmCtl);
		if (manage_resets) {
			// Wait a while just in case a reset/reinitialization has occurred.
			// Ignore register access failures here...
			DAR_WaitSec(1000000, 0);
			rc = RIO_SUCCESS;
		}
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SET_CONFIG(0x50);
			goto exit;
		}
	}

	plmCtl = plmCtl & ~(TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST);
	plmCtl |= saved_plmCtl & (TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST);
	devCtl = devCtl & ~(TSI721_DEVCTL_SR_RST_MODE);
	devCtl |= saved_devCtl & TSI721_DEVCTL_SR_RST_MODE;

	rc = DARRegWrite(dev_info, TSI721_PLM_IMP_SPEC_CTL, plmCtl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x60);
		goto exit;
	}

	rc = DARRegWrite(dev_info, TSI721_DEVCTL, devCtl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x61);
		goto exit;
	}

exit:
	return rc;
}

static uint32_t tsi721_reset_lp(DAR_DEV_INFO_t *dev_info, uint32_t *imp_rc)
{
	uint32_t rc;
	uint32_t lr_resp;

	rc = DARRegWrite(dev_info, TSI721_SP_LM_REQ, STYPE1_LREQ_CMD_RST_DEV);
	if (RIO_SUCCESS != rc) {
		*imp_rc = PC_RESET_LP(0x20);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI721_SP_LM_RESP, &lr_resp);
	if (RIO_SUCCESS != rc) {
		*imp_rc = PC_RESET_LP(0x21);
		goto exit;
	}

	if (!(lr_resp & TSI721_SP_LM_RESP_RESP_VLD)) {
		rc = RIO_ERR_READ_REG_RETURN_INVALID_VAL;
		*imp_rc = PC_RESET_LP(0x22);
	}

exit:
	return rc;
}

static uint32_t tsi721_update_reset_policy(DAR_DEV_INFO_t *dev_info,
		rio_pc_rst_handling rst_policy_in, uint32_t *saved_plmctl,
		uint32_t *saved_devctl, uint32_t *saved_rstint,
		uint32_t *saved_rstpw, uint32_t *imp_rc)
{
	uint32_t rc;
	uint32_t plmCtl, devCtl, rstInt = 0, rstPw = 0;

	rc = DARRegRead(dev_info, TSI721_PLM_IMP_SPEC_CTL, saved_plmctl);
	if (RIO_SUCCESS != rc) {
		*imp_rc = UPDATE_RESET(1);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI721_DEVCTL, saved_devctl);
	if (RIO_SUCCESS != rc) {
		*imp_rc = UPDATE_RESET(2);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI721_EM_RST_INT_EN, saved_rstint);
	if (RIO_SUCCESS != rc) {
		*imp_rc = UPDATE_RESET(3);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI721_EM_RST_PW_EN, saved_rstpw);
	if (RIO_SUCCESS != rc) {
		*imp_rc = UPDATE_RESET(4);
		goto exit;
	}

	// Default is to ignore resets...
	plmCtl = *saved_plmctl & ~(TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
	TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST);
	devCtl = *saved_devctl & ~TSI721_DEVCTL_SR_RST_MODE;

	//@sonar:off - c:S3458
	switch (rst_policy_in) {
	case rio_pc_rst_device:
		plmCtl |= TSI721_PLM_IMP_SPEC_CTL_SELF_RST |
		TSI721_PLM_IMP_SPEC_CTL_RESET_REG;
		devCtl |= TSI721_DEVCTL_SR_RST_MODE_HOT_RST;
		break;
	case rio_pc_rst_port:
		plmCtl |= TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;
		devCtl |= TSI721_DEVCTL_SR_RST_MODE_SRIO_ONLY;
		break;
	case rio_pc_rst_int:
		rstInt = TSI721_EM_RST_INT_EN_RST_INT_EN;
		break;
	case rio_pc_rst_pw:
		rstPw = TSI721_EM_RST_PW_EN_RST_PW_EN;
		break;
	case rio_pc_rst_ignore:
	default:
		break;
	}
	//@sonar:on

	rc = DARRegWrite(dev_info, TSI721_PLM_IMP_SPEC_CTL, plmCtl);
	if (RIO_SUCCESS != rc) {
		*imp_rc = UPDATE_RESET(3);
		goto exit;
	}

	rc = DARRegWrite(dev_info, TSI721_DEVCTL, devCtl);
	if (RIO_SUCCESS != rc) {
		*imp_rc = UPDATE_RESET(4);
		goto exit;
	}

	rc = DARRegWrite(dev_info, TSI721_EM_RST_INT_EN, rstInt);
	if (RIO_SUCCESS != rc) {
		*imp_rc = UPDATE_RESET(5);
		goto exit;
	}

	rc = DARRegWrite(dev_info, TSI721_EM_RST_PW_EN, rstPw);
	if (RIO_SUCCESS != rc) {
		*imp_rc = UPDATE_RESET(6);
		goto exit;
	}

exit:
	return rc;
}

uint32_t tsi721_rio_pc_get_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_config_in_t *in_parms,
		rio_pc_get_config_out_t *out_parms)
{
	uint32_t rc;
	uint32_t port_idx, idx;
	bool misconfigured = false;
	uint32_t plmCtl, spxCtl, devStat, spxCtl2;
	int32_t lane_num;
	struct DAR_ptl good_ptl;
	uint32_t temp;

	out_parms->num_ports = 0;
	out_parms->imp_rc = 0;

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x1);
		goto exit;
	}

	out_parms->num_ports = good_ptl.num_ports;
	for (port_idx = 0; port_idx < good_ptl.num_ports; port_idx++)
		out_parms->pc[port_idx].pnum = good_ptl.pnums[port_idx];

	// Always get LRTO
	rc = DARRegRead(dev_info, TSI721_SP_LT_CTL, &temp);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x2);
		goto exit;
	}
	out_parms->lrto = temp >> 8;

	// Always get LOG_RTO
	rc = DARRegRead(dev_info, TSI721_SR_RSP_TO, &temp);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x3);
		goto exit;
	}
	out_parms->log_rto = ((temp >> 8) * 188) / 100;

	for (port_idx = 0; port_idx < out_parms->num_ports; port_idx++) {
		out_parms->pc[port_idx].port_available = true;
		out_parms->pc[port_idx].pw = rio_pc_pw_last;
		out_parms->pc[port_idx].ls = rio_pc_ls_last;
		out_parms->pc[port_idx].iseq = rio_pc_is_one;
		out_parms->pc[port_idx].fc = rio_pc_fc_rx;
		out_parms->pc[port_idx].xmitter_disable = false;
		out_parms->pc[port_idx].port_lockout = false;
		out_parms->pc[port_idx].nmtc_xfer_enable = false;
		out_parms->pc[port_idx].rx_lswap = rio_lswap_none;
		out_parms->pc[port_idx].tx_lswap = rio_lswap_none;
		for (lane_num = 0; lane_num < RIO_MAX_PORT_LANES; lane_num++) {
			out_parms->pc[port_idx].tx_linvert[lane_num] = false;
			out_parms->pc[port_idx].rx_linvert[lane_num] = false;
		}

		// Check that the RapidIO port has been enabled...
		rc = DARRegRead(dev_info, TSI721_DEVCTL, &devStat);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_GET_CONFIG(5);
			goto exit;
		}

		out_parms->pc[port_idx].powered_up =
			((devStat & TSI721_DEVCTL_SRBOOT_CMPL)
			&& (devStat & TSI721_DEVCTL_PCBOOT_CMPL)) ?
						true : false;

		if (!out_parms->pc[port_idx].powered_up) {
			goto exit;
		}

		// Check that RapidIO transmitter is enabled...
		rc = DARRegRead(dev_info, TSI721_SP_CTL, &spxCtl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_GET_CONFIG(8);
			goto exit;
		}

		out_parms->pc[port_idx].xmitter_disable =
				(spxCtl & TSI721_SP_CTL_PORT_DIS) ?
						true : false;

		// OK, port is enabled so it can train.
		// Check for port width overrides...
		rc = DARRegRead(dev_info, TSI721_SP_CTL, &spxCtl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_GET_CONFIG(0x10);
			goto exit;
		}

		switch (spxCtl & RIO_SPX_CTL_PTW_OVER) {
		case RIO_SPX_CTL_PTW_OVER_4X_NO_2X:
		case RIO_SPX_CTL_PTW_OVER_NONE_2:
		case RIO_SPX_CTL_PTW_OVER_NONE:
			out_parms->pc[port_idx].pw = rio_pc_pw_4x;
			break;
		case RIO_SPX_CTL_PTW_OVER_1X_L0:
			out_parms->pc[port_idx].pw = rio_pc_pw_1x_l0;
			break;
		case RIO_SPX_CTL_PTW_OVER_1X_LR:
			out_parms->pc[port_idx].pw = rio_pc_pw_1x_l2;
			break;
		case RIO_SPX_CTL_PTW_OVER_2X_NO_4X:
			out_parms->pc[port_idx].pw = rio_pc_pw_2x;
			break;
		default:
			out_parms->pc[port_idx].pw = rio_pc_pw_last;
		}

		// Determine configured port speed...
		rc = DARRegRead(dev_info, TSI721_SP_CTL2, &spxCtl2);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_GET_CONFIG(0x11);
			goto exit;
		}

		out_parms->pc[port_idx].ls = rio_pc_ls_last;
		misconfigured = false;

		for (idx = 0; (ls_check[idx].ls_en_val) && !misconfigured;
				idx++) {
			if (ls_check[idx].ls_en_val & spxCtl2) {
				if (!(ls_check[idx].ls_sup_val & spxCtl2)) {
					misconfigured = true;
					out_parms->pc[port_idx].ls =
							rio_pc_ls_last;
				} else {
					if (rio_pc_ls_last
							!= out_parms->pc[port_idx].ls) {
						misconfigured = true;
						out_parms->pc[port_idx].ls =
								rio_pc_ls_last;
					} else {
						out_parms->pc[port_idx].ls =
								ls_check[idx].ls;
					}
				}
			}
		}

		out_parms->pc[port_idx].port_lockout =
				(spxCtl & TSI721_SP_CTL_PORT_LOCKOUT) ?
						true : false;

		out_parms->pc[port_idx].nmtc_xfer_enable =
				((spxCtl
						& (TSI721_SP_CTL_INP_EN
								| TSI721_SP_CTL_OTP_EN))
						== (TSI721_SP_CTL_INP_EN
								| TSI721_SP_CTL_OTP_EN));

		// Check for lane swapping
		rc = DARRegRead(dev_info, TSI721_PLM_IMP_SPEC_CTL, &plmCtl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_GET_CONFIG(0x20);
			goto exit;
		}

		temp = (plmCtl & TSI721_PLM_IMP_SPEC_CTL_SWAP_RX) >> 16;
		out_parms->pc[port_idx].rx_lswap = lswap(temp);

		temp = (plmCtl & TSI721_PLM_IMP_SPEC_CTL_SWAP_TX) >> 18;
		out_parms->pc[port_idx].tx_lswap = lswap(temp);
	}

exit:
	return rc;
}

uint32_t tsi721_rio_pc_set_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_set_config_in_t *in_parms,
		rio_pc_set_config_out_t *out_parms)
{
	uint32_t rc = RIO_SUCCESS;
	rio_pc_get_config_in_t curr_cfg_in;
	uint32_t devStat;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->num_ports = 0;

	if ((NUM_PORTS(dev_info) < in_parms->num_ports)
			&& !(RIO_ALL_PORTS == in_parms->num_ports)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = PC_SET_CONFIG(0x1);
		goto exit;
	}

	if (!(RIO_ALL_PORTS == in_parms->num_ports)
			&& (in_parms->pc[0].pnum > 0)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = PC_SET_CONFIG(0x2);
		goto exit;
	}

	// Always set LRTO.  LRTO coincidentally is in units of 100 nsec...
	rc = DARRegWrite(dev_info, TSI721_SP_LT_CTL,
			((in_parms->lrto) << 8) & TSI721_SP_LT_CTL_TVAL);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x1);
		goto exit;
	}

	// Always set LOG_RTO
	// Note: Tsi721 logical response timeout field does not appear at the
	// correct location in the register.  We generically correct this in the
	// tsi721_WriteReg routine, so we must write a generic value here.
	rc = DARRegWrite(dev_info, TSI721_SR_RSP_TO,
			((((in_parms->log_rto * 100) + 187) / 188) << 8)
					& RIO_SP_RTO_CTL_TVAL);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x1);
		goto exit;
	}
 
	// The Tsi721 has a separate logical layer response timeout for
	// the messaging engine, TSI721_RQRPTO.  Set this register to the
	// same requested value for logical response timeout.
	//
	// Computation is for shortest possible timeout, which is
	// (count value * 8 * 4 nsec)
	rc = DARRegWrite(dev_info, TSI721_RQRPTO,
			(((in_parms->log_rto * 100) + 31) / (8 * 4))
					& TSI721_RQRPTO_REQ_RSP_TO);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x1);
		goto exit;
	}

	curr_cfg_in.ptl.num_ports = RIO_ALL_PORTS;
	rc = tsi721_rio_pc_get_config(dev_info, &curr_cfg_in, out_parms);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x3);
		goto exit;
	}

	// Work our way from most to least severe changes...
	//
	// If boot complete has not been set, the port is disabled, or we've got
	// out-of-band register access, this implies that the RapidIO port can be
	// reconfigured without fear of resets.
	//
	// Perform all changes to port configuration and don't manage resets.

	if (!out_parms->pc[0].powered_up || out_parms->pc[0].xmitter_disable
			|| in_parms->oob_reg_acc) {
		rc = tsi721_set_config_with_resets(dev_info, in_parms,
				out_parms, NO_RESETS);
		if (RIO_SUCCESS != rc) {
			goto exit;
		}

		if (in_parms->pc[0].powered_up) {
			// Set boot complete, now that the configuration is done...
			rc = DARRegRead(dev_info, TSI721_DEVSTAT, &devStat);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SET_CONFIG(0x10);
				goto exit;
			}

			devStat |= TSI721_DEVSTAT_SR_BOOT;
			rc = DARRegWrite(dev_info, TSI721_DEVSTAT, devStat);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SET_CONFIG(0x11);
				goto exit;
			}
		}
		goto done;
	}

	// RapidIO port is powered up, port_dis is cleared, and the Tsi721
	// is being accessed in band.
	//
	// Disabling the port of the device will prevent future communication,
	// so don't do it...
	// Changing the lane speed of the device will cause a device reset, and
	// clear all of the device regsiters.  Don't do it.

	if ((out_parms->pc[0].xmitter_disable != in_parms->pc[0].xmitter_disable)
			|| (out_parms->pc[0].ls != in_parms->pc[0].ls)) {
		rc = RIO_ERR_NOT_SUP_BY_CONFIG;
		out_parms->imp_rc = PC_SET_CONFIG(5);
		goto exit;
	}

	// All other changes can be made without causing a reset of the device...
	// Note that they may cause link reinitialization...
	rc = tsi721_set_config_with_resets(dev_info, in_parms, out_parms,
			MANAGE_RESETS);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

done:
	rc = tsi721_rio_pc_get_config(dev_info, &curr_cfg_in, out_parms);

exit:
	return rc;
}

uint32_t tsi721_rio_pc_get_status(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_status_in_t *in_parms,
		rio_pc_get_status_out_t *out_parms)
{
	uint32_t rc;
	uint8_t port_idx;
	uint32_t errStat, spxCtl, devCtl;
	struct DAR_ptl good_ptl;

	out_parms->num_ports = 0;
	out_parms->imp_rc = RIO_SUCCESS;

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_GET_STATUS(1);
		goto exit;
	}

	out_parms->num_ports = good_ptl.num_ports;
	for (port_idx = 0; port_idx < good_ptl.num_ports; port_idx++) {
		out_parms->ps[port_idx].pnum = good_ptl.pnums[port_idx];
	}

	for (port_idx = 0; port_idx < out_parms->num_ports; port_idx++) {
		out_parms->ps[port_idx].pw = rio_pc_pw_last;
		out_parms->ps[port_idx].port_error = false;
		out_parms->ps[port_idx].input_stopped = false;
		out_parms->ps[port_idx].output_stopped = false;

		rc = DARRegRead(dev_info, TSI721_DEVCTL, &devCtl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_GET_STATUS(5);
			goto exit;
		}

		out_parms->ps[port_idx].first_lane = 0;
		if (devCtl & TSI721_DEVCTL_SRBOOT_CMPL) {
			out_parms->ps[port_idx].num_lanes = true;
		} else {
			out_parms->ps[port_idx].num_lanes = false;
		}

		if (!out_parms->ps[port_idx].num_lanes) {
			continue;
		}

		// Port is available and powered up, so let's figure out the status...
		rc = DARRegRead(dev_info, TSI721_SP_ERR_STAT, &errStat);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_GET_STATUS(0x30+port_idx);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI721_SP_CTL, &spxCtl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_GET_STATUS(0x40+port_idx);
			goto exit;
		}

		out_parms->ps[port_idx].port_ok =
				(errStat & TSI721_SP_ERR_STAT_PORT_OK) ?
						true : false;
		out_parms->ps[port_idx].input_stopped =
				(errStat & TSI721_SP_ERR_STAT_INPUT_ERR_STOP) ?
						true : false;
		out_parms->ps[port_idx].output_stopped =
				(errStat & TSI721_SP_ERR_STAT_OUTPUT_ERR_STOP) ?
						true : false;

		// Port Error is true if a PORT_ERR is present, OR
		// if a OUTPUT_FAIL is present when STOP_FAIL_EN is set.
		out_parms->ps[port_idx].port_error =
				((errStat & TSI721_SP_ERR_STAT_PORT_ERR)
						|| ((spxCtl
								& TSI721_SP_CTL_STOP_FAIL_EN)
								&& (errStat
										& TSI721_SP_ERR_STAT_OUTPUT_FAIL)));

		// Baudrate and portwidth status are only defined when
		// PORT_OK is asserted...
		if (out_parms->ps[port_idx].port_ok) {
			switch (spxCtl & TSI721_SP_CTL_INIT_PWIDTH) {
			case RIO_SPX_CTL_PTW_INIT_1X_L0:
				out_parms->ps[port_idx].pw = rio_pc_pw_1x_l0;
				break;
			case RIO_SPX_CTL_PTW_INIT_1X_LR:
				out_parms->ps[port_idx].pw = rio_pc_pw_1x_l2;
				break;
			case RIO_SPX_CTL_PTW_INIT_2X:
				out_parms->ps[port_idx].pw = rio_pc_pw_2x;
				break;
			case RIO_SPX_CTL_PTW_INIT_4X:
				out_parms->ps[port_idx].pw = rio_pc_pw_4x;
				break;
			default:
				out_parms->ps[port_idx].pw = rio_pc_pw_last;
			}
		}
	}

exit:
	return rc;
}

uint32_t tsi721_rio_pc_reset_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_port_in_t *in_parms,
		rio_pc_reset_port_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	uint8_t port_idx;

	out_parms->imp_rc = RIO_SUCCESS;

	if ((RIO_ALL_PORTS != in_parms->port_num)
			&& (in_parms->port_num >= NUM_PORTS(dev_info))) {
		out_parms->imp_rc = PC_RESET_PORT(1);
		goto exit;
	}

	out_parms->imp_rc = RIO_SUCCESS;

	if (RIO_ALL_PORTS == in_parms->port_num) {
		cfg_in.ptl.num_ports = RIO_ALL_PORTS;
	} else {
		cfg_in.ptl.num_ports = 1;
		cfg_in.ptl.pnums[0] = in_parms->port_num;
	}

	rc = tsi721_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto exit;
	}

	for (port_idx = 0; port_idx < cfg_out.num_ports; port_idx++) {
		uint32_t devctl, plmctl, ctl2_saved, ctl2;
		uint32_t rstint, rstpw;

		// Do not reset ports required for connectivity.
		// Also skip ports that are not available or powered down.
		if ((!cfg_out.pc[port_idx].port_available)
				|| (!cfg_out.pc[port_idx].powered_up)) {
			continue;
		}

		rc = DARRegRead(dev_info, TSI721_SP_CTL2, &ctl2_saved);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_PORT(0x07);
			goto exit;
		}

		ctl2 = ctl2_saved ^ (TSI721_SP_CTL2_GB_6P25_EN |
		TSI721_SP_CTL2_GB_5P0_EN |
		TSI721_SP_CTL2_GB_3P125_EN |
		TSI721_SP_CTL2_GB_2P5_EN |
		TSI721_SP_CTL2_GB_1P25_EN);

		rc = tsi721_update_reset_policy(dev_info, rio_pc_rst_port,
				&plmctl, &devctl, &rstint, &rstpw,
				&out_parms->imp_rc);
		if (RIO_SUCCESS != rc) {
			goto exit;
		}

		if (in_parms->reset_lp) {
			rc = tsi721_reset_lp(dev_info, &out_parms->imp_rc);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
		}

		/* Trigger a port reset by blipping the enabled baudrates.
		 */

		rc = DARRegWrite(dev_info, TSI721_SP_CTL2, ctl2);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_PORT(0x40);
			goto exit;
		}

		rc = DARRegWrite(dev_info, TSI721_SP_CTL2, ctl2_saved);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_PORT(0x41);
			goto exit;
		}

		/* Then restore the reset policy
		 */
		rc = DARRegWrite(dev_info, TSI721_DEVCTL, devctl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_PORT(0x42);
			goto exit;
		}

		rc = DARRegWrite(dev_info, TSI721_PLM_IMP_SPEC_CTL, plmctl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_PORT(0x43);
			goto exit;
		}

		rc = DARRegWrite(dev_info, TSI721_EM_RST_INT_EN, rstint);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_PORT(0x44);
			goto exit;
		}

		rc = DARRegWrite(dev_info, TSI721_EM_RST_PW_EN, rstpw);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_PORT(0x45);
			goto exit;
		}
	}

exit:
	return rc;
}

uint32_t tsi721_rio_pc_reset_link_partner(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_link_partner_in_t *in_parms,
		rio_pc_reset_link_partner_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;

	out_parms->imp_rc = RIO_SUCCESS;

	if (NUM_PORTS(dev_info) <= in_parms->port_num) {
		out_parms->imp_rc = PC_RESET_LP(1);
		goto exit;
	}

	rc = tsi721_reset_lp(dev_info, &out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	if (in_parms->resync_ackids) {
		rc = DARRegWrite(dev_info, TSI721_SP_ACKID_STAT,
		TSI721_SP_ACKID_STAT_CLR_OUTSTD_ACKID);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_LP(2);
		}
	}

exit:
	return rc;
}

uint32_t tsi721_rio_pc_clr_errs(DAR_DEV_INFO_t *dev_info,
		rio_pc_clr_errs_in_t *in_parms,
		rio_pc_clr_errs_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t port_idx;
	uint32_t dlay;
	uint32_t lresp = 0;
	uint32_t ackid_stat;
	uint32_t err_stat;
	rio_pc_get_status_in_t status_in;
	rio_pc_get_status_out_t status_out;

	out_parms->imp_rc = RIO_SUCCESS;

	if (NUM_PORTS(dev_info) <= in_parms->port_num) {
		out_parms->imp_rc = PC_CLR_ERRS(1);
		goto exit;
	}

	if (in_parms->clr_lp_port_err) {
		if (!in_parms->num_lp_ports
				|| (in_parms->num_lp_ports > RIO_MAX_PORTS)
				|| (NULL == in_parms->lp_dev_info)) {
			out_parms->imp_rc = PC_CLR_ERRS(2);
			goto exit;
		}
		for (port_idx = 0; port_idx < in_parms->num_lp_ports;
				port_idx++) {
			if (in_parms->lp_port_list[port_idx]
					>= NUM_PORTS(in_parms->lp_dev_info)) {
				out_parms->imp_rc = PC_CLR_ERRS(3);
				goto exit;
			}
		}
	}

	// If the port is not PORT_OK, it is not possible to clear error conditions.
	status_in.ptl.num_ports = 1;
	status_in.ptl.pnums[0] = in_parms->port_num;
	rc = tsi721_rio_pc_get_status(dev_info, &status_in, &status_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = status_out.imp_rc;
		goto exit;
	}

	if ((status_out.num_ports != 1) || (!status_out.ps[0].port_ok)) {
		rc = RIO_ERR_ERRS_NOT_CL;
		out_parms->imp_rc = PC_CLR_ERRS(4);
		goto exit;
	}

	/* First, ensure input/output error-stopped conditions are cleared
	 on our port and the link partner by sending the magic control symbol.
	 */

	rc = DARRegWrite(dev_info, TSI721_PLM_LONG_CS_TX1, TSI721_MAGIC_CS);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_CLR_ERRS(0x10);
		goto exit;
	}

	//  Delay while waiting for control symbol magic to complete.
	//  Should be > 5 usec, just to allow some margin for different link speeds
	//  and link partners.

	DAR_WaitSec(5000, 0);

	// Prepare to clear any port-err conditions that may exist on this port.
	//     Send link-request/input-status to learn what link partners
	//     next expected ackID is.
	rc = DARRegWrite(dev_info, TSI721_SP_LM_REQ, STYPE1_LREQ_CMD_PORT_STAT);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_CLR_ERRS(0x11);
		goto exit;
	}

	// Poll until we get a response.  Fail if no response is received.

	dlay = 10;
	while (!(lresp & TSI721_SP_LM_RESP_RESP_VLD) && dlay) {
		dlay--;
		rc = DARRegRead(dev_info, TSI721_SP_LM_RESP, &lresp);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_CLR_ERRS(0x12);
			goto exit;
		}
	}

	if (!(lresp & TSI721_SP_LM_RESP_RESP_VLD)) {
		rc = RIO_ERR_NOT_EXPECTED_RETURN_VALUE;
		out_parms->imp_rc = PC_CLR_ERRS(0x13);
		goto exit;
	}

	// We have valid ackID information.  Update our local ackID status.
	// The act of updating our local ackID status will clear a local
	// port-err condition.

	rc = DARRegRead(dev_info, TSI721_SP_ACKID_STAT, &ackid_stat);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_CLR_ERRS(0x14);
		goto exit;
	}

	lresp = (lresp & TSI721_SP_LM_RESP_ACK_ID_STAT) >> 5;
	ackid_stat = ackid_stat & TSI721_SP_ACKID_STAT_INB_ACKID;
	ackid_stat = ackid_stat | lresp | (lresp << 8);

	rc = DARRegWrite(dev_info, TSI721_SP_ACKID_STAT, ackid_stat);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_CLR_ERRS(0x15);
		goto exit;
	}

	// Clearing link partner port errors is not supported.
	if (in_parms->clr_lp_port_err) {
		out_parms->imp_rc = PC_CLR_ERRS(0x15);
		goto exit;
	}

	// Lastly, clear physical layer error status indications for the port.
	rc = DARRegRead(dev_info, TSI721_SP_ERR_STAT, &err_stat);
	if (RIO_SUCCESS != rc)
		goto exit;

	rc = DARRegWrite(dev_info, TSI721_SP_ERR_STAT, err_stat);
	if (RIO_SUCCESS != rc)
		goto exit;

	rc = DARRegRead(dev_info, TSI721_SP_ERR_STAT, &err_stat);
	if (RIO_SUCCESS != rc)
		goto exit;

	if (err_stat & (TSI721_SP_ERR_STAT_PORT_ERR |
	TSI721_SP_ERR_STAT_INPUT_ERR_STOP |
	TSI721_SP_ERR_STAT_OUTPUT_ERR_STOP |
	TSI721_SP_ERR_STAT_OUTPUT_FAIL)) {
		rc = RIO_ERR_ERRS_NOT_CL;
		out_parms->imp_rc = PC_CLR_ERRS(0x20);
		goto exit;
	}

exit:
	return rc;
}

uint32_t tsi721_rio_pc_secure_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_secure_port_in_t *in_parms,
		rio_pc_secure_port_out_t *out_parms)
{
	uint32_t rc;
	uint32_t ftype_filt;
	struct DAR_ptl good_ptl;
	uint32_t unused1, unused2, unused3, unused4;

	out_parms->imp_rc = RIO_SUCCESS;

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		goto exit;
		out_parms->imp_rc = PC_SECURE_PORT(1);
	}

	if (in_parms->rst >= rio_pc_rst_last) {
		out_parms->imp_rc = PC_SECURE_PORT(2);
		return rc;
	}

	// Take care of reset policy update
	out_parms->rst = in_parms->rst;
	rc = tsi721_update_reset_policy(dev_info, in_parms->rst, &unused1,
			&unused2, &unused3, &unused4, &out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	// Update MECS policy.
	// Will always accept MECS physically.  Since this is an endpoint
	// with only one port, MECS cannot be forwarded.

	out_parms->MECS_acceptance = true;
	out_parms->MECS_participant = false;

	// Tsi721 allows maintenance packets to be filtered out.
	rc = DARRegRead(dev_info, TSI721_TLM_SP_FTYPE_FILTER_CTL, &ftype_filt);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SECURE_PORT(3);
		goto exit;
	}

	if (in_parms->MECS_acceptance) {
		ftype_filt |= TSI721_TLM_SP_FTYPE_FILTER_CTL_F8_MR
				| TSI721_TLM_SP_FTYPE_FILTER_CTL_F8_MW;
	} else {
		ftype_filt &= ~(TSI721_TLM_SP_FTYPE_FILTER_CTL_F8_MR
				| TSI721_TLM_SP_FTYPE_FILTER_CTL_F8_MW);
	}

	rc = DARRegWrite(dev_info, TSI721_TLM_SP_FTYPE_FILTER_CTL, ftype_filt);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SECURE_PORT(4);
	}

exit:
	return rc;
}

uint32_t tsi721_rio_pc_dev_reset_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_dev_reset_config_in_t *in_parms,
		rio_pc_dev_reset_config_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t unused1, unused2, unused3, unused4;

	out_parms->rst = in_parms->rst;
	out_parms->imp_rc = RIO_SUCCESS;

	if ((uint8_t)(out_parms->rst) >= (uint8_t)(rio_pc_rst_last)) {
		out_parms->imp_rc = PC_DEV_RESET_CONFIG(1);
		goto exit;
	}

	rc = tsi721_update_reset_policy(dev_info, in_parms->rst, &unused1,
			&unused2, &unused3, &unused4, &out_parms->imp_rc);

exit:
	return rc;
}

#endif /* TSI721_DAR_WANTED */

#ifdef __cplusplus
}
#endif
