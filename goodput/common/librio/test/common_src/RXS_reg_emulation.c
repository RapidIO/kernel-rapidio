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
#include "src/RXS_DeviceDriver.h"
#include "rio_standard.h"
#include "rio_ecosystem.h"
#include "tok_parse.h"
#include "libcli.h"
#include "rio_mport_lib.h"

#include "RXS2448.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RXS_DAR_WANTED

// Common support for register emulation.  This is a superset
// of what is required for the individual SC, PC, RT, and EM tests.
//
// The behavior of the performance optimization register access
// must be overridden when registers are mocked.
//
// The elegant way to do this, while retaining transparency in the code,
// is to add 1 to the defined register offset to compute the mocked register
// offset.  This prevents the standard performance optimization register
// support from recognizing the register and updating the poregs array.

#define MOCK_REG_ADDR(x) (x | 1)

static uint32_t rxs_get_poreg_idx(DAR_DEV_INFO_t *dev_info, uint32_t offset)
{
	return DAR_get_poreg_idx(dev_info, MOCK_REG_ADDR(offset));
}

static uint32_t rxs_expect_poreg_idx(DAR_DEV_INFO_t *dev_info, uint32_t offset)
{
	uint32_t idx =  DAR_get_poreg_idx(dev_info, MOCK_REG_ADDR(offset));

	if (DAR_POREG_BAD_IDX == idx) {
		assert_int_equal(offset, idx);
	}
	assert_int_not_equal(idx, DAR_POREG_BAD_IDX);
	return idx;
}

static uint32_t rxs_add_poreg(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t data)
{
	return DAR_add_poreg(dev_info, MOCK_REG_ADDR(offset), data);
}

static DAR_DEV_INFO_t mock_dev_info;
static DAR_DEV_INFO_t mock_lp_dev_info;

#define MOCK_LP_RXS_SW_PORT_DFLT 0x1805
#define MOCK_LP_RXS_SW_PORT_SUCCESS 0x1804

#define ACKID_CAP_BASE 0xF80
static uint32_t emulate_lp_read(DAR_DEV_INFO_t *dev_info,
				uint32_t offset,
				uint32_t *readdata,
				uint32_t idx)
{
	uint32_t t_idx;
	rio_port_t port;
	uint32_t rc = RIO_SUCCESS;

	*readdata = dev_info->poregs[idx].data;

	// No additional behavior required, just read the data...
	if (RXS_SW_PORT != offset) {
		return rc;
	}

	// Have to fake reading the link partners port info register.
	// Must fail on first read, and pass on second.  Track this
	// with switch port information data.
	//
	// Highly dependent on tests of resync_ackids()
	if (MOCK_LP_RXS_SW_PORT_SUCCESS == *readdata) {
		dev_info->poregs[idx].data = MOCK_LP_RXS_SW_PORT_DFLT;
		return RIO_SUCCESS;
	}

	// Now need to update RXS_PLM_SPX_ACKID_CAP register to reflect
	// success or failure of reads...  Don't know which port is reading
	// this register, so we'll just update them all...

	dev_info->poregs[idx].data = MOCK_LP_RXS_SW_PORT_SUCCESS;
	for (port = 0; port < NUM_RXS_PORTS(&mock_dev_info); port++) {
		t_idx = rxs_expect_poreg_idx(&mock_dev_info,
					RXS_PLM_SPX_ACKID_CAP(port));
		mock_dev_info.poregs[t_idx].data =
			RXS_PLM_SPX_ACKID_CAP_VALID + ACKID_CAP_BASE + port;
	}

	if (MOCK_LP_RXS_SW_PORT_SUCCESS != *readdata) {
		rc = RIO_ERR_ACCESS;
	}
	return rc;
}

static void emulate_spx_pp_reg_read(DAR_DEV_INFO_t *dev_info,
				uint32_t offset,
				uint32_t *readdata,
				uint32_t idx)
{
	rio_port_t port = (offset - RXS_SPX_LM_REQ(0)) / RXS_SPX_PP_OSET;

	// Clear LM_RESP_VLD bit on read, just like real hardware
	if (RXS_SPX_LM_RESP(port) == offset) {
		*readdata = dev_info->poregs[idx].data;
		dev_info->poregs[idx].data &= ~RIO_SPX_LM_RESP_VLD;
		return;
	}

	// No additional behavior required, just read the data...
	*readdata = dev_info->poregs[idx].data;
}

static void emulate_plm_pp_reg_read(DAR_DEV_INFO_t *dev_info,
				uint32_t offset,
				uint32_t *readdata,
				uint32_t idx)
{
	rio_port_t port = (offset - RXS_PLM_SPX_IMP_SPEC_CTL(0)) /
			RXS_IMP_SPEC_PP_OSET;

	// Clear PLM_SPX_ACKID_CAP_VALID on read, just like real hardware
	if (RXS_PLM_SPX_ACKID_CAP(port) == offset) {
		*readdata = dev_info->poregs[idx].data;
		dev_info->poregs[idx].data &= ~RXS_PLM_SPX_ACKID_CAP_VALID;
		return;
	}

	// No additional behavior required, just read the data...
	*readdata = dev_info->poregs[idx].data;
}

#define IN_RANGE(a,b,c) (((a) <= (b)) && ((b) <= (c)))

static uint32_t RXSReadReg(DAR_DEV_INFO_t *dev_info,
			uint32_t  offset, uint32_t *readdata)
{
	uint32_t rc = 0xFFFFFFFF;
	uint32_t idx = rxs_get_poreg_idx(dev_info, MOCK_REG_ADDR(offset));

	if (NULL == dev_info) {
		return rc;
	}

	if (!st.real_hw) {
		if (DAR_POREG_BAD_IDX == idx) {
			assert_int_equal(offset, idx);
		}
		if (DEBUG_REGTRACE) {
			printf("\nREAD  OSET 0x%x Data 0x%x\n", offset,
						dev_info->poregs[idx].data);
		}

		// Emulating access to link partner registers...
		if (&mock_lp_dev_info == dev_info) {
			rc = emulate_lp_read(dev_info, offset, readdata, idx);
			goto exit;
		}

		rc = RIO_SUCCESS;
		if (IN_RANGE(RXS_SPX_LM_REQ(0), offset,
				RXS_SPX_TMR_CTL3(NUM_RXS_PORTS(dev_info) - 1)))
		{
			emulate_spx_pp_reg_read(dev_info,
							offset, readdata, idx);
			goto exit;
		}
		if (IN_RANGE(RXS_PLM_BH, offset,
			RXS_PLM_SPX_SCRATCHY(NUM_RXS_PORTS(dev_info) - 1,
						RXS_PLM_SPX_MAX_SCRATCHY))) {
			emulate_plm_pp_reg_read(
					dev_info, offset, readdata, idx);
			goto exit;
		}
		*readdata = dev_info->poregs[idx].data;
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

static void update_mc_masks(DAR_DEV_INFO_t *dev_info,
			rio_port_t port, uint32_t idx, uint32_t new_mask)
{
	uint32_t reg_idx;

	// Update register values for both the set and
	// clear mask.

	reg_idx = rxs_expect_poreg_idx(dev_info,
				RXS_SPX_MC_Y_S_CSR(port, idx));
	dev_info->poregs[reg_idx].data = new_mask;

	reg_idx = rxs_expect_poreg_idx(dev_info,
					RXS_SPX_MC_Y_C_CSR(port, idx));
	dev_info->poregs[reg_idx].data = new_mask;
}

static void update_plm_status(DAR_DEV_INFO_t *dev_info,
				uint32_t plm_mask,
				uint32_t plm_events,
				rio_port_t port)
{
	unsigned int st_idx;
	unsigned int en_idx;
	const uint32_t pp_mask = 1 << port;
	uint32_t pp_stat = 0;
	uint32_t rst_stat;
	uint32_t rst_mask;

	st_idx  = rxs_expect_poreg_idx(dev_info, RXS_PLM_SPX_STAT(port));

	dev_info->poregs[st_idx].data &= ~plm_mask;
	dev_info->poregs[st_idx].data |= plm_events;
	plm_events = dev_info->poregs[st_idx].data;

	// Update per-port interrupt status
	en_idx  = rxs_expect_poreg_idx(dev_info, RXS_PLM_SPX_ALL_INT_EN(port));
	if (plm_events &
		(dev_info->poregs[en_idx].data | RXS_PLM_SPX_UNMASKABLE_MASK)) {
		pp_stat = pp_mask;
	}

	st_idx  = rxs_expect_poreg_idx(dev_info, RXS_EM_INT_PORT_STAT);
	dev_info->poregs[st_idx].data &= ~pp_mask;
	dev_info->poregs[st_idx].data |= pp_stat;

	pp_stat = dev_info->poregs[st_idx].data;

	// Update global interrupt status for PORT
	st_idx  = rxs_expect_poreg_idx(dev_info, RXS_EM_INT_STAT);
	if (pp_stat) {
		dev_info->poregs[st_idx].data &= ~RXS_EM_INT_STAT_PORT;
	} else {
		dev_info->poregs[st_idx].data |= RXS_EM_INT_STAT_PORT;
	}

	// Update per-port port-write status
	pp_stat = 0;
	en_idx  = rxs_expect_poreg_idx(dev_info, RXS_SPX_CTL(port));
	if ((RXS_SPX_ERR_STAT_PORT_W_DIS & dev_info->poregs[en_idx].data) &&
								plm_events ) {
		pp_stat = pp_mask;
	}

	st_idx  = rxs_expect_poreg_idx(dev_info, RXS_EM_PW_PORT_STAT);
	dev_info->poregs[st_idx].data &= ~pp_mask;
	dev_info->poregs[st_idx].data |= pp_stat;

	pp_stat = dev_info->poregs[st_idx].data;

	// Update global port-write status for PORT
	st_idx  = rxs_expect_poreg_idx(dev_info, RXS_EM_PW_STAT);
	if (pp_stat) {
		dev_info->poregs[st_idx].data &= ~RXS_EM_PW_STAT_PORT;
	} else {
		dev_info->poregs[st_idx].data |= RXS_EM_PW_STAT_PORT;
	}

	// Update per-port reset request status...
	pp_stat = 0;
	if (plm_events & (RXS_PLM_SPX_STAT_RST_REQ| RXS_PLM_SPX_STAT_PRST_REQ)){
		pp_stat = pp_mask;
	}

	st_idx  = rxs_expect_poreg_idx(dev_info, RXS_EM_RST_PORT_STAT);
	dev_info->poregs[st_idx].data &= ~pp_mask;
	dev_info->poregs[st_idx].data |= pp_stat;

	rst_stat = dev_info->poregs[st_idx].data;

	// Update top-level reset request interrupt status...
	en_idx  = rxs_expect_poreg_idx(dev_info, RXS_EM_RST_INT_EN);
	rst_mask = dev_info->poregs[en_idx].data;

	st_idx  = rxs_expect_poreg_idx(dev_info, RXS_EM_INT_STAT);
	if (rst_stat & rst_mask) {
		dev_info->poregs[st_idx].data |= RXS_EM_INT_STAT_RCS;
	} else {
		dev_info->poregs[st_idx].data &= ~ RXS_EM_INT_STAT_RCS;
	}

	// Update top-level reset request port-write status...
	en_idx  = rxs_expect_poreg_idx(dev_info, RXS_EM_RST_PW_EN);
	rst_mask = dev_info->poregs[en_idx].data;

	st_idx  = rxs_expect_poreg_idx(dev_info, RXS_EM_PW_STAT);
	if (rst_stat & rst_mask) {
		dev_info->poregs[st_idx].data |= RXS_EM_PW_STAT_RCS;
	} else {
		dev_info->poregs[st_idx].data &= ~ RXS_EM_PW_STAT_RCS;
	}
}

static void emulate_spx_pp_reg_write(DAR_DEV_INFO_t *dev_info,
				uint32_t offset,
				uint32_t writedata,
				uint32_t idx)
{
	rio_port_t port = (offset - RXS_SPX_LM_REQ(0)) / RXS_SPX_PP_OSET;
	uint32_t t_idx;
	uint32_t link_resp_stat = RIO_SPX_LM_RESP_VLD |
				RIO_SPX_LM_RESP_STAT12_IES |
				RIO_SPX_LM_RESP_ACK_ID3 |
				RIO_SPX_LM_RESP_STAT3;
	unsigned int lanes;

	if (RXS_SPX_LM_REQ(port) == offset) {
		dev_info->poregs[idx].data = 0;
		t_idx = rxs_expect_poreg_idx(dev_info, RXS_SPX_LM_RESP(port));

		switch (writedata) {
		case RIO_SPX_LM_REQ_CMD_RST_PT:
			dev_info->poregs[t_idx].data = RIO_SPX_LM_RESP_VLD;
			break;
		case RIO_SPX_LM_REQ_CMD_RST_DEV:
			dev_info->poregs[t_idx].data = RIO_SPX_LM_RESP_VLD;
			break;
		case RIO_SPX_LM_REQ_CMD_LR_IS:
			dev_info->poregs[t_idx].data = link_resp_stat;
			break;
		default:
			assert_true(false);
		}
		return;
	}

	// Support writing to OVER_PWIDTH field...
	if (RXS_SPX_CTL(port) == offset) {
		lanes = RIO_SPX_CTL_PTW_MAX_LANES(writedata);

		writedata &= ~RXS_SPX_CTL_INIT_PWIDTH;
		switch(writedata & RXS_SPX_CTL_OVER_PWIDTH) {
		case RIO_SPX_CTL_PTW_OVER_NONE:
		case RIO_SPX_CTL_PTW_OVER_RSVD:
		case RIO_SPX_CTL_PTW_OVER_NONE_2:
			// No override, or train on Lane 0.
			// port trained to maximum width
			switch (lanes) {
			case 1:
				writedata |= RIO_SPX_CTL_PTW_INIT_1X_L0;
				break;
			case 2:
				writedata |= RIO_SPX_CTL_PTW_INIT_2X;
				break;
			case 4:
				writedata |= RIO_SPX_CTL_PTW_INIT_4X;
				break;
			default:
				assert_true(false);
			}
			break;
		case RIO_SPX_CTL_PTW_OVER_1X_L0:
			writedata |= RIO_SPX_CTL_PTW_INIT_1X_L0;
			break;

		case RIO_SPX_CTL_PTW_OVER_1X_LR:
			switch (lanes) {
			case 1:
				writedata |= RIO_SPX_CTL_PTW_INIT_1X_L0;
				break;
			case 2:
			case 4:
				writedata |= RIO_SPX_CTL_PTW_INIT_1X_LR;
				break;
			default:
				assert_true(false);
			}
			break;
		case RIO_SPX_CTL_PTW_OVER_2X_NO_4X:
			switch (lanes) {
			case 1:
				writedata |= RIO_SPX_CTL_PTW_INIT_1X_L0;
				break;
			case 2:
			case 4:
				writedata |= RIO_SPX_CTL_PTW_INIT_2X;
				break;
			default:
				assert_true(false);
			}
			break;
		case RIO_SPX_CTL_PTW_OVER_4X_NO_2X:
			switch (lanes) {
			case 1:
			case 2:
				writedata |= RIO_SPX_CTL_PTW_INIT_1X_L0;
				break;
			case 4:
				writedata |= RIO_SPX_CTL_PTW_INIT_4X;
				break;
			default:
				assert_true(false);
			}
			break;
		}
	}

	// No additional behavior required, just update the data...
	dev_info->poregs[idx].data = writedata;
}

static void emulate_emhs_pp_reg(DAR_DEV_INFO_t *dev_info,
				uint32_t offset,
				uint32_t writedata,
				uint32_t idx)
{
	rio_port_t port = (offset - RXS_SPX_ERR_DET(0)) / RXS_SPX_PP_OSET;
	uint32_t plm_mask = RXS_PLM_SPX_STAT_DLT |
			RXS_PLM_SPX_STAT_OK_TO_UNINIT |
			RXS_PLM_SPX_STAT_LINK_INIT;
	uint32_t plm_events = 0;

	// Might need to enhance this if, as shown in the docs,
	// PLM status bits are gated by RXS_SPX_RATE_EN(port) settings
	if (RXS_SPX_ERR_DET(port) == offset) {
		dev_info->poregs[idx].data = writedata;

		if (writedata & RXS_SPX_ERR_DET_DLT) {
			plm_events |= RXS_PLM_SPX_STAT_DLT;
		}
		if (writedata & RXS_SPX_ERR_DET_OK_TO_UNINIT) {
			plm_events |= RXS_PLM_SPX_STAT_OK_TO_UNINIT;
		}
		if (writedata & RXS_SPX_ERR_DET_LINK_INIT) {
			plm_events |= RXS_PLM_SPX_STAT_LINK_INIT;
		}

		update_plm_status(dev_info, plm_mask, plm_events, port);
		return;
	}

	// No additional behavior required, just update the data...
	dev_info->poregs[idx].data = writedata;
}

static void emulate_plm_reg_write(DAR_DEV_INFO_t *dev_info,
				uint32_t offset,
				uint32_t writedata,
				uint32_t idx)
{
	rio_port_t port = (offset - RXS_PLM_SPX_IMP_SPEC_CTL(0))
			/ RXS_IMP_SPEC_PP_OSET;
	unsigned int t_idx;
	uint32_t temp;

	if (RXS_PLM_SPX_STAT(port) == offset) {
		dev_info->poregs[idx].data &= ~writedata;
		update_plm_status(dev_info, 0xFFFFFFFF,
				dev_info->poregs[idx].data, port);
		return;
	}

	if (RXS_PLM_SPX_EVENT_GEN(port) == offset) {
		// Event generation register clears itself...
		dev_info->poregs[idx].data = 0;
		t_idx = rxs_expect_poreg_idx(dev_info, RXS_PLM_SPX_STAT(port));
		dev_info->poregs[t_idx].data |= writedata;

		update_plm_status(dev_info, 0xFFFFFFFF,
				dev_info->poregs[t_idx].data, port);
		return;
	}

	// emulate soft resetting the port, at least to the extent of
	// clearing all interrupt status registers...
	if ((RXS_PLM_SPX_IMP_SPEC_CTL(port) == offset) &&
		(writedata & RXS_PLM_SPX_IMP_SPEC_CTL_SOFT_RST_PORT)) {

		t_idx = rxs_expect_poreg_idx(dev_info, RXS_PBM_SPX_STAT(port));
		dev_info->poregs[t_idx].data = 0;

		t_idx = rxs_expect_poreg_idx(dev_info, RXS_TLM_SPX_STAT(port));
		dev_info->poregs[t_idx].data = 0;

		t_idx = rxs_expect_poreg_idx(dev_info, RXS_SPX_ERR_DET(port));
		dev_info->poregs[t_idx].data = 0;

		t_idx = rxs_expect_poreg_idx(dev_info, RXS_PLM_SPX_STAT(port));
		dev_info->poregs[t_idx].data = 0;

		update_plm_status(dev_info, 0xFFFFFFFF, 0, port);
	}

	if (RXS_PLM_SPX_1WR(port) == offset) {
		// One-write register controls lane speed and idle sequence.
		// Lane speed is found in the SPX_CTL2 register.
		t_idx = rxs_expect_poreg_idx(dev_info, RXS_SPX_CTL2(port));

		temp = dev_info->poregs[t_idx].data;
		temp &= RXS_SPX_CTL2_RTEC_EN | RXS_SPX_CTL2_RTEC |
			RXS_SPX_CTL2_D_SCRM_DIS | RXS_SPX_CTL2_INACT_LN_EN |
			RXS_SPX_CTL2_RETRAIN_EN;
		switch (writedata & RXS_PLM_SPX_1WR_BAUD_EN) {
		case RXS_PLM_SPX_1WR_BAUD_EN_2P5:
			temp |= RXS_SPX_CTL2_GB_2P5_EN |
				RXS_SPX_CTL2_GB_2P5 |
				RXS_SPX_CTL2_BAUD_SEL_2_5GB;
			break;

		case RXS_PLM_SPX_1WR_BAUD_EN_3P125:
			temp |= RXS_SPX_CTL2_GB_3P125_EN |
				RXS_SPX_CTL2_GB_3P125 |
				RXS_SPX_CTL2_BAUD_SEL_3_125GB;
			break;

		case RXS_PLM_SPX_1WR_BAUD_EN_5P0:
			temp |= RXS_SPX_CTL2_GB_5P0_EN |
				RXS_SPX_CTL2_GB_5P0 |
				RXS_SPX_CTL2_BAUD_SEL_5_0GB;
			break;

		case RXS_PLM_SPX_1WR_BAUD_EN_6P25:
			temp |= RXS_SPX_CTL2_GB_6P25_EN |
				RXS_SPX_CTL2_GB_6P25 |
				RXS_SPX_CTL2_BAUD_SEL_6_25GB;
			break;

		case RXS_PLM_SPX_1WR_BAUD_EN_10P3:
			temp |= RXS_SPX_CTL2_GB_10P3_EN |
				RXS_SPX_CTL2_GB_10P3 |
				RXS_SPX_CTL2_BAUD_SEL_10_3125GB;
			break;

		case RXS_PLM_SPX_1WR_BAUD_EN_12P5:
			temp |= RXS_SPX_CTL2_GB_12P5_EN |
				RXS_SPX_CTL2_GB_12P5 |
				RXS_SPX_CTL2_BAUD_SEL_12_5GB;
			break;

		default:
			assert_true(false);
		}
		dev_info->poregs[t_idx].data = temp;

		// Idle sequence is found in the PLM_SPX_CTL register.
		t_idx = rxs_expect_poreg_idx(dev_info,
						RXS_PLM_SPX_IMP_SPEC_CTL(port));
		temp = dev_info->poregs[t_idx].data;
		temp &= ~(RXS_PLM_SPX_IMP_SPEC_CTL_USE_IDLE1 |
			RXS_PLM_SPX_IMP_SPEC_CTL_USE_IDLE2 |
			RXS_PLM_SPX_IMP_SPEC_CTL_USE_IDLE3);
		switch (writedata & RXS_PLM_SPX_1WR_IDLE_SEQ) {
		case RXS_PLM_SPX_1WR_IDLE_SEQ_DFLT:
			break;
		case RXS_PLM_SPX_1WR_IDLE_SEQ_1:
			temp |= RXS_PLM_SPX_IMP_SPEC_CTL_USE_IDLE1;
			break;
		case RXS_PLM_SPX_1WR_IDLE_SEQ_2:
			temp |= RXS_PLM_SPX_IMP_SPEC_CTL_USE_IDLE2;
			break;
		case RXS_PLM_SPX_1WR_IDLE_SEQ_3:
			temp |= RXS_PLM_SPX_IMP_SPEC_CTL_USE_IDLE3;
			break;
		default:
			assert_true(false);
		}

		dev_info->poregs[t_idx].data = temp;
	}

	// No additional behavior required, just update the data...
	dev_info->poregs[idx].data = writedata;
}

static void tlm_update_plm_status(DAR_DEV_INFO_t *dev_info,
				rio_port_t port)
{
	uint32_t plm_mask = RXS_PLM_SPX_STAT_TLM_INT | RXS_PLM_SPX_STAT_TLM_PW;
	uint32_t plm_events = 0;
	uint32_t stat;
	unsigned int t_idx;

	// Determine if an interrupt or port-write event has been
	// triggered.
	t_idx = rxs_expect_poreg_idx(dev_info, RXS_TLM_SPX_STAT(port));
	stat = dev_info->poregs[t_idx].data;

	t_idx = rxs_expect_poreg_idx(dev_info, RXS_TLM_SPX_PW_EN(port));
	if (stat & dev_info->poregs[t_idx].data) {
		plm_events |= RXS_PLM_SPX_STAT_TLM_PW;
	}

	t_idx = rxs_expect_poreg_idx(dev_info, RXS_TLM_SPX_INT_EN(port));
	if (stat & dev_info->poregs[t_idx].data) {
		plm_events |= RXS_PLM_SPX_STAT_TLM_INT;
	}

	update_plm_status(dev_info, plm_mask, plm_events, port);
}

static void emulate_tlm_reg(DAR_DEV_INFO_t *dev_info,
				uint32_t offset,
				uint32_t writedata,
				uint32_t idx)
{
	rio_port_t port = (offset - RXS_TLM_SPX_CONTROL(0))
			/ RXS_IMP_SPEC_PP_OSET;
	unsigned int t_idx;

	if (RXS_TLM_SPX_STAT(port) == offset) {
		dev_info->poregs[idx].data &= ~writedata;

		tlm_update_plm_status(dev_info, port);
		return;
	}

	if (RXS_TLM_SPX_EVENT_GEN(port) == offset) {
		// Event generation register clears itself...
		dev_info->poregs[idx].data = 0;
		t_idx = rxs_expect_poreg_idx(dev_info, RXS_TLM_SPX_STAT(port));
		dev_info->poregs[t_idx].data |= writedata;

		tlm_update_plm_status(dev_info, port);
		return;
	}

	// No additional behavior required, just update the data...
	dev_info->poregs[idx].data = writedata;
}

static void pbm_update_plm_status(DAR_DEV_INFO_t *dev_info,
				rio_port_t port)
{
	uint32_t plm_mask = RXS_PLM_SPX_STAT_PBM_INT |
				RXS_PLM_SPX_STAT_PBM_PW |
				RXS_PLM_SPX_STAT_PBM_FATAL;
	uint32_t plm_events = 0;
	uint32_t stat;
	unsigned int t_idx;

	// Determine if an interrupt or port-write event has been
	// triggered.
	t_idx = rxs_expect_poreg_idx(dev_info, RXS_PBM_SPX_STAT(port));
	stat = dev_info->poregs[t_idx].data;

	t_idx = rxs_expect_poreg_idx(dev_info, RXS_PBM_SPX_PW_EN(port));
	if (stat & dev_info->poregs[t_idx].data) {
		plm_events |= RXS_PLM_SPX_STAT_PBM_PW;
	}

	t_idx = rxs_expect_poreg_idx(dev_info, RXS_PBM_SPX_INT_EN(port));
	if (stat & dev_info->poregs[t_idx].data) {
		plm_events |= RXS_PLM_SPX_STAT_PBM_INT;
	}

	if (stat & (RXS_PBM_SPX_STAT_EG_DNFL_FATAL |
			RXS_PBM_SPX_STAT_EG_DOH_FATAL |
			RXS_PBM_SPX_STAT_EG_DATA_UNCOR)) {
		plm_events |= RXS_PLM_SPX_STAT_PBM_FATAL;
	}

	update_plm_status(dev_info, plm_mask, plm_events, port);
}

static void emulate_pbm_reg(DAR_DEV_INFO_t *dev_info,
				uint32_t offset,
				uint32_t writedata,
				uint32_t idx)
{
	rio_port_t port = (offset - RXS_PBM_SPX_CONTROL(0))
			/ RXS_IMP_SPEC_PP_OSET;
	unsigned int t_idx;

	if (RXS_PBM_SPX_STAT(port) == offset) {
		dev_info->poregs[idx].data &= ~writedata;
		pbm_update_plm_status(dev_info, port);
		return;
	}

	if (RXS_PBM_SPX_EVENT_GEN(port) == offset) {
		// Event generation register clears itself...
		dev_info->poregs[idx].data = 0;
		t_idx = rxs_expect_poreg_idx(dev_info, RXS_PBM_SPX_STAT(port));
		dev_info->poregs[t_idx].data |= writedata;

		pbm_update_plm_status(dev_info, port);
		return;
	}

	// No additional behavior required, just update the data...
	dev_info->poregs[idx].data = writedata;
}

static void update_dev_int_pw_stat(DAR_DEV_INFO_t *dev_info,
					uint32_t int_mask,
					uint32_t pw_mask,
					uint32_t events)
{
	unsigned int t_idx;

	// Update device port-write status
	t_idx = rxs_expect_poreg_idx(dev_info, RXS_EM_PW_STAT);
	if (events) {
		dev_info->poregs[t_idx].data |= pw_mask;
	} else {
		dev_info->poregs[t_idx].data &= ~pw_mask;
	}

	// Update device interrupt status
	t_idx = rxs_expect_poreg_idx(dev_info, RXS_EM_INT_STAT);
	if (events) {
		dev_info->poregs[t_idx].data |= int_mask;
	} else {
		dev_info->poregs[t_idx].data &= ~int_mask;
	}

	// Update interrupt pin status
	events = dev_info->poregs[t_idx].data;
	t_idx = rxs_expect_poreg_idx(dev_info, RXS_EM_INT_EN);
	events &= dev_info->poregs[t_idx].data;

	t_idx = rxs_expect_poreg_idx(dev_info, RXS_EM_DEV_INT_EN);
	if (events) {
		dev_info->poregs[t_idx].data |= RXS_EM_DEV_INT_EN_INT;
	} else {
		dev_info->poregs[t_idx].data |= RXS_EM_DEV_INT_EN_INT;
	}
}

static void rxs_emulate_reg_write(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t writedata)
{
	uint32_t idx = rxs_expect_poreg_idx(dev_info, MOCK_REG_ADDR(offset));
	uint32_t t_idx;
	uint32_t events;

	if (DAR_POREG_BAD_IDX == idx) {
		assert_int_equal(0xFFFFFFFF, offset);
		return;
	}

	if (IN_RANGE(RXS_SPX_LM_REQ(0), offset,
			RXS_SPX_TMR_CTL3(NUM_RXS_PORTS(dev_info) - 1))) {
		emulate_spx_pp_reg_write(dev_info, offset, writedata, idx);
		return;
	}

	// Note: RXS_ERR_DET (logical layer errors) are emulated later in
	// this routine.
	if (IN_RANGE(RXS_SPX_ERR_DET(0), offset,
			RXS_SPX_DLT_CSR(NUM_RXS_PORTS(dev_info) - 1))) {
		emulate_emhs_pp_reg(dev_info, offset, writedata, idx);
		return;
	}

	if (IN_RANGE(RXS_PLM_BH, offset,
			RXS_PLM_SPX_SCRATCHY(NUM_RXS_PORTS(dev_info) - 1,
						RXS_PLM_SPX_MAX_SCRATCHY))) {
		emulate_plm_reg_write(dev_info, offset, writedata, idx);
		return;
	}

	if (IN_RANGE(RXS_TLM_BH, offset,
			RXS_TLM_SPX_ROUTE_EN(NUM_RXS_PORTS(dev_info) - 1))) {
		emulate_tlm_reg(dev_info, offset, writedata, idx);
		return;
	}

	if (IN_RANGE(RXS_PBM_BH, offset,
			RXS_PBM_SPX_SCRATCH2(NUM_RXS_PORTS(dev_info) - 1))) {
		emulate_pbm_reg(dev_info, offset, writedata, idx);
		return;
	}

	switch (offset) {
	case I2C_INT_STAT:
		dev_info->poregs[idx].data &= ~writedata;
		events = dev_info->poregs[idx].data;

		t_idx = rxs_expect_poreg_idx(dev_info, I2C_INT_ENABLE);
		events &= dev_info->poregs[t_idx].data;

		update_dev_int_pw_stat(dev_info,
					RXS_EM_INT_STAT_EXTERNAL_I2C,
					RXS_EM_PW_STAT_EXTERNAL_I2C,
					events);
		break;

	case I2C_INT_SET:
		dev_info->poregs[idx].data = 0;

		t_idx = rxs_expect_poreg_idx(dev_info, I2C_INT_STAT);
		dev_info->poregs[t_idx].data |= writedata;
		events = dev_info->poregs[t_idx].data;

		t_idx = rxs_expect_poreg_idx(dev_info, I2C_INT_ENABLE);
		events &= dev_info->poregs[t_idx].data;

		update_dev_int_pw_stat(dev_info,
					RXS_EM_INT_STAT_EXTERNAL_I2C,
					RXS_EM_PW_STAT_EXTERNAL_I2C,
					events);
		break;

	case RXS_ERR_DET:
		writedata &= (RXS_ERR_DET_ILL_TYPE |
				RXS_ERR_DET_UNS_RSP |
				RXS_ERR_DET_ILL_ID);

		dev_info->poregs[idx].data = writedata;
		events = dev_info->poregs[idx].data;

		t_idx = rxs_expect_poreg_idx(dev_info, RXS_ERR_EN);
		events &= dev_info->poregs[t_idx].data;

		update_dev_int_pw_stat(dev_info,
					RXS_EM_INT_STAT_LOG,
					RXS_EM_PW_STAT_LOG,
					events);
		break;

	default:
		dev_info->poregs[idx].data = writedata;
		break;
	}
}

static void check_write_bc(DAR_DEV_INFO_t *dev_info,
			uint32_t offset, uint32_t writedata)
{
	uint32_t did, mask_idx, mask;
	rio_port_t port;

	if (st.real_hw) {
		return;
	}

	// Handle writes to broadcast set/clear multicast mask registers
	if ((offset >= RXS_BC_MC_X_S_CSR(0)) &&
		(offset < RXS_BC_MC_X_C_CSR(RXS2448_MC_MASK_CNT))) {
		uint32_t new_mask;
		bool do_clear = (offset & 4) ? true : false;

		mask_idx = (offset - RXS_BC_MC_X_S_CSR(0)) / 8;
		assert_in_range(mask_idx, 0, RXS2448_MAX_MC_MASK);

		for (port = 0; port < NUM_RXS_PORTS(dev_info);  port++) {
			assert_int_equal(RIO_SUCCESS,
				DARRegRead(dev_info,
				RXS_SPX_MC_Y_S_CSR(port, mask_idx),
				&mask));
			if (do_clear) {
				new_mask = mask & ~writedata;
			} else {
				new_mask = mask | writedata;
			}
			update_mc_masks(dev_info, port, mask_idx, new_mask);
		}
		return;
	}

	// Handle writes to per-port set/clear multicast mask registers
	for (port = 0; port < NUM_RXS_PORTS(dev_info);  port++) {
		if ((offset >= RXS_SPX_MC_Y_S_CSR(port, 0)) &&
		(offset < RXS_SPX_MC_Y_C_CSR(port, RXS2448_MC_MASK_CNT))) {
			uint32_t new_mask;
			bool do_clear = (offset & 4) ? true : false;

			mask_idx = offset - RXS_SPX_MC_Y_S_CSR(port, 0);
			mask_idx /= 8;
			assert_in_range(mask_idx, 0, RXS2448_MAX_MC_MASK);

			assert_int_equal(RIO_SUCCESS,
				DARRegRead(dev_info,
				RXS_SPX_MC_Y_S_CSR(port, mask_idx),
				&mask));
			if (do_clear) {
				new_mask = mask & ~writedata;
			} else {
				new_mask = mask | writedata;
			}
			update_mc_masks(dev_info, port, mask_idx, new_mask);
			return;
		}
	}

	if ((offset >= RXS_BC_L2_GX_ENTRYY_CSR(0,0)) &&
		(offset <= RXS_BC_L2_GX_ENTRYY_CSR(0, RIO_RT_GRP_SZ-1))) {
		did = (offset - RXS_BC_L2_GX_ENTRYY_CSR(0,0)) / 4;

		for (port = 0; port < NUM_RXS_PORTS(dev_info);  port++) {
			assert_int_equal(RIO_SUCCESS,
				DARRegWrite(dev_info,
				RXS_SPX_L2_GY_ENTRYZ_CSR(port, 0, did),
				writedata));
		}
		return;
	}

	if ((offset >= RXS_BC_L1_GX_ENTRYY_CSR(0,0)) &&
		(offset <= RXS_BC_L1_GX_ENTRYY_CSR(0, RIO_RT_GRP_SZ-1))) {
		did = (offset - RXS_BC_L1_GX_ENTRYY_CSR(0,0)) / 4;

		for (port = 0; port < NUM_RXS_PORTS(dev_info);  port++) {
			assert_int_equal(RIO_SUCCESS,
				DARRegWrite(dev_info,
				RXS_SPX_L1_GY_ENTRYZ_CSR(port, 0, did),
				writedata));
		}
	}
	rxs_emulate_reg_write(dev_info, offset, writedata);
}

static uint32_t RXSWriteReg(DAR_DEV_INFO_t *dev_info,
			uint32_t  offset, uint32_t writedata)
{
	uint32_t rc = 0xFFFFFFFF;

	if (NULL == dev_info) {
		return rc;
	}

	if (!st.real_hw) {
		if (DEBUG_REGTRACE) {
			printf("\nWRITE OSET 0x%x Data 0x%x\n",
							offset, writedata);
		}
		check_write_bc(dev_info, offset, writedata);
		return RIO_SUCCESS;
	}
	assert_true(st.mp_h_valid);

	if (0xFF == st.hc) {
		rc = rio_lcfg_write(st.mp_h, offset, 4, writedata);
	} else {
		rc = rio_maint_write(st.mp_h, st.did_reg_val, st.hc, offset, 4, writedata);
	}

	return rc;
}

static void RXSWaitSec(uint32_t delay_nsec, uint32_t delay_sec)
{
	if (st.real_hw) {
		uint64_t counter = delay_nsec + ((uint64_t)delay_sec * 1000000000);
		for ( ; counter; counter--);
	}
}

uint32_t rxs_mock_lp_reg_oset[] = {
	RXS_SW_PORT
};

#define MOCK_DEV_LP_REG (sizeof(rxs_mock_lp_reg_oset)/ \
			sizeof(rxs_mock_lp_reg_oset[0]))
#define UPB_MOCK_LP_REG (MOCK_DEV_LP_REG + 1)

uint32_t rxs_mock_reg_oset[] = {
	RXS_PRESCALAR_SRV_CLK,
	RXS_MPM_CFGSIG0,
	RXS_SP_LT_CTL,
	RXS_SP_GEN_CTL,
	RXS_ROUTE_DFLT_PORT,
	RXS_PKT_TIME_LIVE,
	RXS_ERR_DET,
	RXS_ERR_EN,

	RXS_PW_TGT_ID,
	RXS_PW_CTL,
	RXS_PW_ROUTE,

	RXS_EM_RST_PW_EN,
	RXS_EM_RST_INT_EN,
	RXS_EM_INT_EN,
	RXS_EM_DEV_INT_EN,
	RXS_EM_PW_EN,
	RXS_PW_TRAN_CTL,
	RXS_EM_INT_STAT,
	RXS_EM_INT_PORT_STAT,
	RXS_EM_PW_STAT,
	RXS_EM_PW_PORT_STAT,
	RXS_EM_RST_PORT_STAT,

	RXS_PCNTR_CTL,

	I2C_INT_ENABLE,
	I2C_INT_SET,
	I2C_INT_STAT
};

// Emulated per-port (pp) registers

typedef struct rxs_mock_pp_reg_t_TAG {
	uint32_t base;
	uint32_t pp_oset;
	uint32_t val;
} rxs_mock_pp_reg_t;

#define RXS_SPX_LM_REQ_DFLT 0
#define RXS_SPX_LM_RESP_DFLT 0
#define RXS_SPX_IN_ACKID_CSR_DFLT 0xF23
#define RXS_SPX_OUT_ACKID_CSR_DFLT 0xA24A24
#define RXS_PLM_SPX_PW_EN_DFLT 0
#define RXS_PLM_SPX_INT_EN_DFLT 0
#define RXS_PLM_SPX_ALL_INT_EN_DFLT 0
#define RXS_PLM_SPX_DENIAL_CTL_DFLT 0
#define RXS_SPX_CTL2_DFLT (RXS_SPX_CTL2_GB_5P0_EN | \
				RXS_SPX_CTL2_GB_12P5 | \
				RXS_SPX_CTL2_GB_10P3 | \
				RXS_SPX_CTL2_GB_6P25 | \
				RXS_SPX_CTL2_GB_5P0 | \
				RXS_SPX_CTL2_GB_5P0_EN | \
				RXS_SPX_CTL2_GB_3P125 | \
				RXS_SPX_CTL2_GB_2P5 | \
				RXS_SPX_CTL2_GB_1P25)

#define RXS_SPX_ERR_STAT_DFLT (RXS_SPX_ERR_STAT_PORT_UNAVL)
#define RXS_SPX_CTL_DFLT (RXS_SPX_CTL_PORT_DIS | \
				RIO_SPX_CTL_PTW_INIT_4X | \
				RXS_SPX_CTL_PORT_WIDTH)
#define RXS_SPX_ERR_DET_DFLT 0
#define RXS_SPX_RATE_EN_DFLT 0
#define RXS_SPX_DLT_DFLT 0

#define RXS_PLM_SPX_IMP_SPEC_CTL_DFLT (RXS_PLM_SPX_IMP_SPEC_CTL_PORT_SELF_RST)
#define RXS_PLM_SPX_PWDN_CTL_DFLT (RXS_PLM_SPX_PWDN_CTL_PWDN_PORT)
#define RXS_PLM_SPX_1WR_DFLT (RXS_PLM_SPX_1WR_BAUD_EN_5P0 | \
				RXS_PLM_SPX_1WR_IDLE_SEQ_1)
#define RXS_PLM_SPX_POL_CTL_DFLT 0
#define RXS_PLM_SPX_PNA_CAP_DFLT (RXS_PLM_SPX_PNA_CAP_VALID)
#define RXS_PLM_SPX_STAT_DFLT 0
#define RXS_PLM_SPX_ACKID_CAP_DFLT 0
#define RXS_PLM_SPX_INT_EN_DFLT 0
#define RXS_PLM_SPX_EVENT_GEN_DFLT 0

#define RXS_TLM_SPX_STAT_DFLT 0
#define RXS_TLM_SPX_PW_EN_DFLT 0
#define RXS_TLM_SPX_INT_EN_DFLT 0
#define RXS_TLM_SPX_FTYPE_FILT_DFLT 0
#define RXS_TLM_SPX_EVENT_GEN_DFLT 0
#define RXS_TLM_SPX_ROUTE_EN_DFLT (0x00FFFFFF)
#define RXS_TLM_SPX_MTC_ROUTE_EN_DFLT (RXS_TLM_SPX_MTC_ROUTE_EN_MTC_EN)

#define RXS_PBM_SPX_STAT_DFLT 0
#define RXS_PBM_SPX_PW_EN_DFLT 0
#define RXS_PBM_SPX_INT_EN_DFLT 0
#define RXS_PBM_SPX_EVENT_GEN_DFLT 0

#define RXS_SPX_PCNTR_EN_DFLT 0
#define RXS_SPX_PCNTR_CTL_DFLT 0
#define RXS_SPX_PCNTR_CNT_DFLT 0

#define RXS_FAB_IG_MTC_VOQ_ACT_DFLT 0
#define RXS_FAB_IG_VOQ_ACT_DFLT 0

rxs_mock_pp_reg_t rxs_mock_pp_reg[] = {
	{RXS_SPX_LM_REQ(0), 0x40, RXS_SPX_LM_REQ_DFLT},
	{RXS_SPX_LM_RESP(0), 0x40, RXS_SPX_LM_RESP_DFLT},
	{RXS_SPX_IN_ACKID_CSR(0), 0x40, RXS_SPX_IN_ACKID_CSR_DFLT},
	{RXS_SPX_OUT_ACKID_CSR(0), 0x40, RXS_SPX_OUT_ACKID_CSR_DFLT},
	{RXS_SPX_CTL2(0), 0x40, RXS_SPX_CTL2_DFLT},
	{RXS_SPX_ERR_STAT(0), 0x40, RXS_SPX_ERR_STAT_DFLT},
	{RXS_SPX_CTL(0), 0x40, RXS_SPX_CTL_DFLT},
	{RXS_SPX_ERR_DET(0), 0x40, RXS_SPX_ERR_DET_DFLT},
	{RXS_SPX_RATE_EN(0), 0x40, RXS_SPX_RATE_EN_DFLT},
	{RXS_SPX_DLT_CSR(0), 0x40, RXS_SPX_DLT_DFLT},

	{RXS_PLM_SPX_IMP_SPEC_CTL(0), 0x100,
				RXS_PLM_SPX_IMP_SPEC_CTL_DFLT},
	{RXS_PLM_SPX_1WR(0), 0x100, RXS_PLM_SPX_1WR_DFLT},
	{RXS_PLM_SPX_STAT(0), 0x100, RXS_PLM_SPX_STAT_DFLT},
	{RXS_PLM_SPX_PW_EN(0), 0x100, RXS_PLM_SPX_PW_EN_DFLT},
	{RXS_PLM_SPX_INT_EN(0), 0x100, RXS_PLM_SPX_INT_EN_DFLT},
	{RXS_PLM_SPX_ALL_INT_EN(0), 0x100, RXS_PLM_SPX_ALL_INT_EN_DFLT},
	{RXS_PLM_SPX_PWDN_CTL(0), 0x100, RXS_PLM_SPX_PWDN_CTL_DFLT},
	{RXS_PLM_SPX_POL_CTL(0), 0x100, RXS_PLM_SPX_POL_CTL_DFLT},
	{RXS_PLM_SPX_PNA_CAP(0), 0x100, RXS_PLM_SPX_PNA_CAP_DFLT},
	{RXS_PLM_SPX_ACKID_CAP(0), 0x100, RXS_PLM_SPX_ACKID_CAP_DFLT},
	{RXS_PLM_SPX_DENIAL_CTL(0), 0x100, RXS_PLM_SPX_DENIAL_CTL_DFLT},
	{RXS_PLM_SPX_EVENT_GEN(0), 0x100, RXS_PLM_SPX_EVENT_GEN_DFLT},

	{RXS_TLM_SPX_STAT(0), 0x100, RXS_TLM_SPX_STAT_DFLT},
	{RXS_TLM_SPX_PW_EN(0), 0x100, RXS_TLM_SPX_PW_EN_DFLT},
	{RXS_TLM_SPX_INT_EN(0), 0x100, RXS_TLM_SPX_INT_EN_DFLT},
	{RXS_TLM_SPX_FTYPE_FILT(0), 0x100, RXS_TLM_SPX_FTYPE_FILT_DFLT},
	{RXS_TLM_SPX_EVENT_GEN(0), 0x100, RXS_TLM_SPX_EVENT_GEN_DFLT},
	{RXS_TLM_SPX_ROUTE_EN(0), 0x100, RXS_TLM_SPX_ROUTE_EN_DFLT},
	{RXS_TLM_SPX_MTC_ROUTE_EN(0), 0x100, RXS_TLM_SPX_MTC_ROUTE_EN_DFLT},

	{RXS_PBM_SPX_STAT(0), 0x100, RXS_PBM_SPX_STAT_DFLT},
	{RXS_PBM_SPX_PW_EN(0), 0x100, RXS_PBM_SPX_PW_EN_DFLT},
	{RXS_PBM_SPX_INT_EN(0), 0x100, RXS_PBM_SPX_INT_EN_DFLT},
	{RXS_PBM_SPX_EVENT_GEN(0), 0x100, RXS_PBM_SPX_EVENT_GEN_DFLT},

	{RXS_SPX_PCNTR_EN(0), 0x100, RXS_SPX_PCNTR_EN_DFLT},
	{RXS_SPX_PCNTR_CTL(0, 0), 0x100, RXS_SPX_PCNTR_CTL_DFLT},
	{RXS_SPX_PCNTR_CTL(0, 1), 0x100, RXS_SPX_PCNTR_CTL_DFLT},
	{RXS_SPX_PCNTR_CTL(0, 2), 0x100, RXS_SPX_PCNTR_CTL_DFLT},
	{RXS_SPX_PCNTR_CTL(0, 3), 0x100, RXS_SPX_PCNTR_CTL_DFLT},
	{RXS_SPX_PCNTR_CTL(0, 4), 0x100, RXS_SPX_PCNTR_CTL_DFLT},
	{RXS_SPX_PCNTR_CTL(0, 5), 0x100, RXS_SPX_PCNTR_CTL_DFLT},
	{RXS_SPX_PCNTR_CTL(0, 6), 0x100, RXS_SPX_PCNTR_CTL_DFLT},
	{RXS_SPX_PCNTR_CTL(0, 7), 0x100, RXS_SPX_PCNTR_CTL_DFLT},
	{RXS_SPX_PCNTR_CNT(0, 0), 0x100, RXS_SPX_PCNTR_CNT_DFLT},
	{RXS_SPX_PCNTR_CNT(0, 1), 0x100, RXS_SPX_PCNTR_CNT_DFLT},
	{RXS_SPX_PCNTR_CNT(0, 2), 0x100, RXS_SPX_PCNTR_CNT_DFLT},
	{RXS_SPX_PCNTR_CNT(0, 3), 0x100, RXS_SPX_PCNTR_CNT_DFLT},
	{RXS_SPX_PCNTR_CNT(0, 4), 0x100, RXS_SPX_PCNTR_CNT_DFLT},
	{RXS_SPX_PCNTR_CNT(0, 5), 0x100, RXS_SPX_PCNTR_CNT_DFLT},
	{RXS_SPX_PCNTR_CNT(0, 6), 0x100, RXS_SPX_PCNTR_CNT_DFLT},
	{RXS_SPX_PCNTR_CNT(0, 7), 0x100, RXS_SPX_PCNTR_CNT_DFLT},

	{RXS_FAB_IG_X_MTC_VOQ_ACT(0), 0x100, RXS_FAB_IG_MTC_VOQ_ACT_DFLT},
	{RXS_FAB_IG_X_VOQ_ACT(0), 0x100, RXS_FAB_IG_VOQ_ACT_DFLT},
#define RXS_FAB_IG_VOQ_ACT_DFLT 0
};

// Count up maximum registers saved.
// Note: only the first level 1 and level 2 routing table groups are supported
// Use *_MAX_PORTS + 1 to account for device broadcast registers
// There are 2 multicast mask registers for each multicast mask:
//    one to set, and one to clear

#define MOCK_DEV_REG (sizeof(rxs_mock_reg_oset)/sizeof(rxs_mock_reg_oset[0]))
#define MOCK_PP_REG (sizeof(rxs_mock_pp_reg)/sizeof(rxs_mock_pp_reg[0]))
#define MOCK_MC_REG ((RXS2448_MAX_PORTS + 1) * RXS2448_MC_MASK_CNT * 2)
#define MOCK_RT_REG ((RXS2448_MAX_PORTS + 1) * RIO_RT_GRP_SZ * 2)

#define TOT_MOCK_REG ((MOCK_DEV_REG + MOCK_MC_REG + MOCK_RT_REG) + \
			(MOCK_PP_REG * RXS2448_MAX_PORTS))
#define UPB_MOCK_REG (TOT_MOCK_REG + 1)

static void rxs_test_setup(void)
{
	uint8_t idx;

	mock_dev_info.privateData = 0x0;
	mock_dev_info.accessInfo = 0x0;
	strcpy(mock_dev_info.name, "RXS2448");
	mock_dev_info.dsf_h = 0x00380000;
	mock_dev_info.extFPtrForPort = 0x100;
	mock_dev_info.extFPtrPortType = 0x19;
	mock_dev_info.extFPtrForLane = 0x3000;
	mock_dev_info.extFPtrForErr = 0x1000;
	mock_dev_info.extFPtrForVC = 0;
	mock_dev_info.extFPtrForVOQ = 0;
	mock_dev_info.devID = 0x80E60038;
	mock_dev_info.driver_family = RIO_RXS_DEVICE;
	mock_dev_info.devInfo = 0;
	mock_dev_info.assyInfo = 256;
	mock_dev_info.features = 402658623;
	mock_dev_info.swPortInfo = 0x1805;
	mock_dev_info.swRtInfo = 255;
	mock_dev_info.srcOps = 4;
	mock_dev_info.dstOps = 0;
	mock_dev_info.swMcastInfo = 0; // RXS does not support MC INFO reg!!!

	mock_dev_ctrs->num_p_ctrs = RIO_MAX_PORTS;
	mock_dev_ctrs->valid_p_ctrs = RIO_MAX_PORTS;

	for (idx = 0; idx < RIO_MAX_PORTS; idx++) {
		mock_dev_info.ctl1_reg[idx] = 0;
	}

	for (idx = 0; idx < MAX_DAR_SCRPAD_IDX; idx++) {
		mock_dev_info.scratchpad[idx] = 0;
	}

	memcpy(&mock_lp_dev_info, &mock_dev_info, sizeof(mock_lp_dev_info));
}

/* Initialize the mock register structure for all mocked registers.
 */

static rio_perf_opt_reg_t mock_lp_dar_reg[UPB_MOCK_LP_REG];
static rio_perf_opt_reg_t mock_dar_reg[UPB_MOCK_REG];

static void init_mock_rxs_reg(void **state)
{
	// idx is always should be less than UPB_DAR_REG.
	uint32_t port, idev, i, val;
	RXS_test_state_t *l_st = *(RXS_test_state_t **)state;
	uint32_t dflt;

	DAR_proc_ptr_init(RXSReadReg, RXSWriteReg, RXSWaitSec);
	if (l_st->real_hw) {
		mock_dev_info.poregs_max = 0;
		mock_dev_info.poreg_cnt = 0;
		mock_dev_info.poregs = NULL;
		mock_lp_dev_info.poregs_max = 0;
		mock_lp_dev_info.poreg_cnt = 0;
		mock_lp_dev_info.poregs = NULL;
		return;
	}

	mock_lp_dev_info.poregs_max = 1;
	mock_lp_dev_info.poreg_cnt = 0;
	mock_lp_dev_info.poregs = mock_lp_dar_reg;

	for (i = 0; i < MOCK_DEV_LP_REG; i++) {
		assert_int_equal(RIO_SUCCESS,
			rxs_add_poreg(&mock_lp_dev_info,
				rxs_mock_lp_reg_oset[i],
				MOCK_LP_RXS_SW_PORT_DFLT));
	}

	mock_dev_info.poregs_max = UPB_MOCK_REG;
	mock_dev_info.poreg_cnt = 0;
	mock_dev_info.poregs = mock_dar_reg;

	// Initialize RXS device regs
	for (i = 0; i < MOCK_DEV_REG; i++) {
		dflt = 0;
		if (RXS_PRESCALAR_SRV_CLK == rxs_mock_reg_oset[i]) {
			dflt = 38;
		}
		if (RXS_MPM_CFGSIG0 == rxs_mock_reg_oset[i]) {
			dflt = RXS_MPM_CFGSIG0_CORECLK_SELECT_LO_PWR_12G |
				RXS_MPM_CFGSIG0_REFCLK_SELECT_156P25MHZ;
		}
		assert_int_equal(RIO_SUCCESS,
			rxs_add_poreg(&mock_dev_info,
					rxs_mock_reg_oset[i], dflt));
	}

	// Initialize RXS per port regs
	for (port = 0; port < RXS2448_MAX_PORTS; port++) {
		for (i = 0; i < MOCK_PP_REG; i++) {
			// Odd ports cannot support 4x, and they train at 2x
			val = rxs_mock_pp_reg[i].val;
			if (RXS_SPX_CTL(0) == rxs_mock_pp_reg[i].base) {
				if (port & 1) {
					val &= ~RIO_SPX_CTL_PTW_MAX_4X;
					val &= ~RIO_SPX_CTL_PTW_INIT_4X;
					val |= RIO_SPX_CTL_PTW_INIT_2X;
				}
			}
			assert_int_equal(RIO_SUCCESS,
				rxs_add_poreg(&mock_dev_info,
					rxs_mock_pp_reg[i].base +
					(rxs_mock_pp_reg[i].pp_oset * port),
					val));
		}
	}

	// Initialize RXS_BC_MC_Y_S_CSR and RXS_BC_MC_Y_C_CSR
	for (idev = 0; idev < RXS2448_MC_MASK_CNT; idev++) {
		assert_int_equal(RIO_SUCCESS,
			rxs_add_poreg(&mock_dev_info,
				RXS_BC_MC_X_S_CSR(idev),
				0x00));
		assert_int_equal(RIO_SUCCESS,
			rxs_add_poreg(&mock_dev_info,
				RXS_BC_MC_X_C_CSR(idev),
				0x00));
	}

	// Initialize RXS_SPX_MC_Y_S_CSR and RXS_SPX_MC_Y_C_CSR
	for (port = 0; port < RXS2448_MAX_PORTS; port++) {
		for (idev = 0; idev < RXS2448_MC_MASK_CNT; idev++) {
			assert_int_equal(RIO_SUCCESS,
				rxs_add_poreg(&mock_dev_info,
					RXS_SPX_MC_Y_S_CSR(port, idev),
					0x00));
			assert_int_equal(RIO_SUCCESS,
				rxs_add_poreg(&mock_dev_info,
					RXS_SPX_MC_Y_C_CSR(port, idev),
					0x00));
		}
	}

	// Initialize RXS_BC_L2_GX_ENTRYY_CSR
	for (idev = 0; idev < RIO_RT_GRP_SZ; idev++) {
		assert_int_equal(RIO_SUCCESS,
			rxs_add_poreg(&mock_dev_info,
				RXS_BC_L2_GX_ENTRYY_CSR(0, idev),
				0x00));
	}

	// Initialize RXS_BC_L1_GX_ENTRYY_CSR
	for (idev = 0; idev < RIO_RT_GRP_SZ; idev++) {
		assert_int_equal(RIO_SUCCESS,
			rxs_add_poreg(&mock_dev_info,
				RXS_BC_L1_GX_ENTRYY_CSR(0, idev),
				0x00));
	}

	// Initialize RXS_SPX_L2_GY_ENTRYZ_CSR
	for (port = 0; port < RXS2448_MAX_PORTS; port++) {
		for (idev = 0; idev < RIO_RT_GRP_SZ; idev++) {
			assert_int_equal(RIO_SUCCESS,
					rxs_add_poreg(&mock_dev_info,
				RXS_SPX_L2_GY_ENTRYZ_CSR(port, 0, idev),
					0x00));
		}
	}

	// Initialize RXS_SPX_L1_GY_ENTRYZ_CSR
	for (port = 0; port < RXS2448_MAX_PORTS; port++) {
		for (idev = 0; idev < RIO_RT_GRP_SZ; idev++) {
			assert_int_equal(RIO_SUCCESS,
					rxs_add_poreg(&mock_dev_info,
				RXS_SPX_L1_GY_ENTRYZ_CSR(port, 0, idev),
					(!idev)? RIO_RTE_LVL_G0 : 0x00));
		}
	}
}

// The setup function which should be called before any unit tests that
// need to be executed.

static int setup(void **state)
{
	uint32_t sw_port_info;
	uint32_t rc;
	RXS_test_state_t *RXS = *(RXS_test_state_t **)state;

	memset(&mock_dev_info, 0x00, sizeof(rio_sc_dev_ctrs_t));
	rxs_test_setup();
	init_mock_rxs_reg(state);
	if (RXS->real_hw) {
		rc = RXSReadReg(&mock_dev_info, RIO_SW_PORT_INF, &sw_port_info);
		assert_int_equal(rc, 0);
		RXS->conn_port = sw_port_info & RIO_SW_PORT_INF_PORT;
	}

	return 0;
}

static void update_ctl_pwidth(uint32_t *ctl)
{
	*ctl &= ~RXS_SPX_CTL_INIT_PWIDTH;

	switch (*ctl & RXS_SPX_CTL_OVER_PWIDTH) {
	case RIO_SPX_CTL_PTW_OVER_NONE:
	case RIO_SPX_CTL_PTW_OVER_NONE_2:
	case RIO_SPX_CTL_PTW_OVER_RSVD:
		if (*ctl & RIO_SPX_CTL_PTW_MAX_4X) {
			*ctl |= RIO_SPX_CTL_PTW_INIT_4X;
		} else if (*ctl & RIO_SPX_CTL_PTW_MAX_2X) {
			*ctl |= RIO_SPX_CTL_PTW_INIT_2X;
		} else {
			*ctl |= RIO_SPX_CTL_PTW_INIT_1X_L0;
		}
		break;
	case RIO_SPX_CTL_PTW_OVER_1X_L0:
		*ctl |= RIO_SPX_CTL_PTW_INIT_1X_L0;
		break;
	case RIO_SPX_CTL_PTW_OVER_1X_LR:
		if (*ctl & RIO_SPX_CTL_PTW_MAX) {
			*ctl |= RIO_SPX_CTL_PTW_INIT_1X_LR;
		} else {
			*ctl |= RIO_SPX_CTL_PTW_INIT_1X_L0;
		}
		break;
	case RIO_SPX_CTL_PTW_OVER_2X_NO_4X:
		if (*ctl & RIO_SPX_CTL_PTW_MAX_2X) {
			*ctl |= RIO_SPX_CTL_PTW_INIT_2X;
		} else {
			*ctl |= RIO_SPX_CTL_PTW_INIT_1X_L0;
		}
		break;
	case RIO_SPX_CTL_PTW_OVER_4X_NO_2X:
		if (*ctl & RIO_SPX_CTL_PTW_MAX_4X) {
			*ctl |= RIO_SPX_CTL_PTW_INIT_4X;
		} else {
			*ctl |= RIO_SPX_CTL_PTW_INIT_1X_L0;
		}
		break;
	default:
		assert_true(false);
	}
}

// Routine to set virtual register status for
// rxs_pc_get_config and rxs_pc_get_status

typedef enum config_hw_t_TAG {
	cfg_unavl,
	cfg_avl_pwdn,
	cfg_pwup_txdis,
	cfg_txen_no_lp,
	cfg_txen_lp_perr,
	cfg_lp_lkout,
	cfg_lp_nmtc_dis,
	cfg_lp_lpbk,
	cfg_lp_ecc,
	cfg_perfect,
	cfg_perfect_2x
} config_hw_t;

#define NO_TTL false
#define YES_TTL true
#define NO_FILT false
#define YES_FILT true

void set_all_port_config(config_hw_t cfg,
					bool ttl,
					bool filter,
					rio_port_t port)
{
	uint32_t ctl2, err_stat, ctl, plm_ctl, plm_stat, pwdn;
	uint32_t err_stat_avail = RXS_SPX_ERR_STAT_DFLT &
				~RXS_SPX_ERR_STAT_PORT_UNAVL;
	uint32_t err_stat_nolp = err_stat_avail |
			RXS_SPX_ERR_STAT_PORT_UNINIT;
	uint32_t err_stat_lp_ok = err_stat_avail |
			RXS_SPX_ERR_STAT_PORT_OK;
	uint32_t err_stat_lp_perr = err_stat_lp_ok |
			RXS_SPX_ERR_STAT_PORT_ERR;
	uint32_t lpbk_mask = RXS_PLM_SPX_IMP_SPEC_CTL_DLB_EN |
				RXS_PLM_SPX_IMP_SPEC_CTL_LLB_EN;
	uint32_t pbm_gen = 0;
	uint32_t pbm_stat = 0;

	uint32_t ttl_reg = (ttl) ? RXS_PKT_TIME_LIVE_PKT_TIME_LIVE : 0;
	uint32_t filt = (filter) ? 0xFFFFFFFF : 0;
	rio_port_t st_port = port, end_port = port;

	if (RIO_ALL_PORTS == port) {
		st_port = 0;
		end_port = NUM_RXS_PORTS(&mock_dev_info) - 1;
	}

	assert_int_equal(RIO_SUCCESS,
		RXSWriteReg(&mock_dev_info, RXS_PKT_TIME_LIVE, ttl_reg));

	for (port = st_port; port <= end_port; port++) {
		assert_int_equal(RIO_SUCCESS,
			RXSReadReg(&mock_dev_info,
				RXS_SPX_CTL2(port), &ctl2));
		assert_int_equal(RIO_SUCCESS,
			RXSReadReg(&mock_dev_info,
				RXS_SPX_ERR_STAT(port), &err_stat));
		assert_int_equal(RIO_SUCCESS,
			RXSReadReg(&mock_dev_info,
				RXS_SPX_CTL(port), &ctl));
		assert_int_equal(RIO_SUCCESS,
			RXSReadReg(&mock_dev_info,
				RXS_PLM_SPX_IMP_SPEC_CTL(port), &plm_ctl));
		assert_int_equal(RIO_SUCCESS,
			RXSReadReg(&mock_dev_info,
				RXS_PLM_SPX_STAT(port), &plm_stat));
		assert_int_equal(RIO_SUCCESS,
			RXSReadReg(&mock_dev_info,
				RXS_PLM_SPX_PWDN_CTL(port), &pwdn));
		switch(cfg) {
		case cfg_unavl:
			err_stat = RXS_SPX_ERR_STAT_DFLT;
			break;;
		case cfg_avl_pwdn:
			err_stat = err_stat_avail;
			pwdn = RXS_PLM_SPX_PWDN_CTL_DFLT;
			break;
		case cfg_pwup_txdis:
			err_stat = err_stat_avail;
			pwdn = 0;
			ctl |= RXS_SPX_CTL_PORT_DIS;
			break;
		case cfg_txen_no_lp:
			err_stat = err_stat_nolp;
			pwdn = 0;
			ctl &= ~RXS_SPX_CTL_PORT_DIS;
			break;
		case cfg_txen_lp_perr:
			err_stat = err_stat_lp_perr;
			pwdn = 0;
			ctl &= ~RXS_SPX_CTL_PORT_DIS;
			update_ctl_pwidth(&ctl);
			break;
		case cfg_lp_lkout:
			err_stat = err_stat_lp_ok;
			pwdn = 0;
			ctl &= ~RXS_SPX_CTL_PORT_DIS;
			ctl |= RXS_SPX_CTL_PORT_LOCKOUT;
			update_ctl_pwidth(&ctl);
			break;
		case cfg_lp_nmtc_dis:
			err_stat = err_stat_lp_ok;
			pwdn = 0;
			ctl &= ~RXS_SPX_CTL_PORT_DIS;
			ctl &= ~RXS_SPX_CTL_PORT_LOCKOUT;
			ctl &= ~(RXS_SPX_CTL_INP_EN |
				RXS_SPX_CTL_OTP_EN);
			update_ctl_pwidth(&ctl);
			break;
		case cfg_lp_lpbk:
			err_stat = err_stat_lp_ok;
			pwdn = 0;
			ctl &= ~RXS_SPX_CTL_PORT_DIS;
			ctl &= ~RXS_SPX_CTL_PORT_LOCKOUT;
			ctl |= RXS_SPX_CTL_INP_EN | RXS_SPX_CTL_OTP_EN;
			plm_ctl |= lpbk_mask;
			update_ctl_pwidth(&ctl);
			break;
		case cfg_lp_ecc:
			err_stat = err_stat_lp_ok;
			pwdn = 0;
			ctl &= ~RXS_SPX_CTL_PORT_DIS;
			ctl &= ~RXS_SPX_CTL_PORT_LOCKOUT;
			ctl |= RXS_SPX_CTL_INP_EN | RXS_SPX_CTL_OTP_EN;
			update_ctl_pwidth(&ctl);
			plm_ctl &= ~lpbk_mask;
			pbm_gen = RXS_PBM_SPX_EVENT_GEN_EG_DATA_UNCOR;
			break;
		case cfg_perfect:
			err_stat = err_stat_lp_ok;
			pwdn = 0;
			ctl &= ~RXS_SPX_CTL_PORT_DIS;
			ctl &= ~RXS_SPX_CTL_PORT_LOCKOUT;
			ctl |= RXS_SPX_CTL_INP_EN | RXS_SPX_CTL_OTP_EN;
			update_ctl_pwidth(&ctl);
			plm_ctl &= ~RXS_PLM_SPX_IMP_SPEC_CTL_LLB_EN;
			pbm_stat = 0xFFFFFFFF;
			plm_stat = 0xFFFFFFFF;
			break;
		case cfg_perfect_2x:
			err_stat = err_stat_lp_ok;
			pwdn = 0;
			ctl &= ~RXS_SPX_CTL_PORT_DIS;
			ctl &= ~RXS_SPX_CTL_PORT_LOCKOUT;
			ctl |= RXS_SPX_CTL_INP_EN | RXS_SPX_CTL_OTP_EN;
			ctl &= ~RIO_SPX_CTL_PTW_MAX_4X;
			ctl |= RIO_SPX_CTL_PTW_MAX_2X;
			ctl |= RIO_SPX_CTL_PTW_INIT_2X;
			update_ctl_pwidth(&ctl);
			plm_ctl &= ~RXS_PLM_SPX_IMP_SPEC_CTL_LLB_EN;
			pbm_stat = 0xFFFFFFFF;
			plm_stat = 0xFFFFFFFF;
			break;
		default:
			assert_true(false);
		}
		assert_int_equal(RIO_SUCCESS,
			RXSWriteReg(&mock_dev_info,
				RXS_SPX_CTL2(port), ctl2));
		assert_int_equal(RIO_SUCCESS,
			RXSWriteReg(&mock_dev_info,
				RXS_SPX_ERR_STAT(port), err_stat));
		assert_int_equal(RIO_SUCCESS,
			RXSWriteReg(&mock_dev_info, RXS_SPX_CTL(port), ctl));
		assert_int_equal(RIO_SUCCESS,
			RXSWriteReg(&mock_dev_info,
				RXS_PLM_SPX_IMP_SPEC_CTL(port), plm_ctl));
		assert_int_equal(RIO_SUCCESS,
			RXSWriteReg(&mock_dev_info,
				RXS_PLM_SPX_STAT(port), plm_stat));
		assert_int_equal(RIO_SUCCESS,
			RXSWriteReg(&mock_dev_info,
				RXS_PLM_SPX_PWDN_CTL(port), pwdn));
		assert_int_equal(RIO_SUCCESS,
			RXSWriteReg(&mock_dev_info,
				RXS_TLM_SPX_FTYPE_FILT(port), filt));
		assert_int_equal(RIO_SUCCESS,
			RXSWriteReg(&mock_dev_info,
				RXS_PBM_SPX_STAT(port), pbm_stat));
		assert_int_equal(RIO_SUCCESS,
			RXSWriteReg(&mock_dev_info,
				RXS_PBM_SPX_EVENT_GEN(port), pbm_gen));
	}
}

#endif /* RXS_DAR_WANTED */

#ifdef __cplusplus
}
#endif
