/*
 ****************************************************************************
 Copyright (c) 2014, Integrated Device Technology Inc.
 Copyright (c) 2014, RapidIO Trade Association
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
#include <string.h>

#include "rio_standard.h"
#include "DAR_DB_Private.h"
#include "DSF_DB_Private.h"
#include "CPS_DeviceDriver.h"
#include "RapidIO_Error_Management_API.h"
#include "CPS_DeviceDriver.h"
#include "CPS1848.h"
#include "CPS1616.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CPS_DAR_WANTED

#define EM_SET_EVENT_EN_0     (EM_FIRST_SUBROUTINE_0+0x2100) // 202100
#define EM_SET_EVENT_PW_0     (EM_FIRST_SUBROUTINE_0+0x2200) // 202200
#define EM_SET_EVENT_INT_0    (EM_FIRST_SUBROUTINE_0+0x2300) // 202300
#define EM_EN_ERR_CTR_0       (EM_FIRST_SUBROUTINE_0+0x2400) // 202400
#define EM_UPDATE_RESET_0     (EM_FIRST_SUBROUTINE_0+0x2500) // 202500
#define EM_DET_NOTFN_0        (EM_FIRST_SUBROUTINE_0+0x2600) // 202600
#define EM_CREATE_RATE_0      (EM_FIRST_SUBROUTINE_0+0x2700) // 202700
#define EM_GET_EC_REGS_0      (EM_FIRST_SUBROUTINE_0+0x2800) // 202800
#define EM_SET_EC_REGS_0      (EM_FIRST_SUBROUTINE_0+0x2900) // 202900

#define EM_GET_EC_REGS(x) (EM_GET_EC_REGS_0+x)
#define EM_SET_EC_REGS(x) (EM_SET_EC_REGS_0+x)

// Per port registers to play with when configuring events
typedef struct cps_event_cfg_reg_values_t_TAG {
	uint32_t pctl1_csr; // CPS1848_PORT_X_CTL_1_CSR(pnum)
	uint32_t log_en_csr; // CPS1848_LT_ERR_EN_CSR
	uint32_t ttl_csr; // CPS1848_PKT_TTL_CSR
	uint32_t err_rate_en_csr; // CPS1848_PORT_X_ERR_RATE_EN_CSR(pnum)
	uint32_t err_rate_csr; // CPS1848_PORT_X_ERR_RATE_CSR(pnum)
	bool wr_err_rate_csr; // true if the err_rate_csr can be written.
	uint32_t err_thrsh_csr; // CPS1848_PORT_X_ERR_RATE_THRESH_CSR(pnum)
	uint32_t err_rpt; // CPS1848_PORT_X_ERR_RPT_EN(pnum)
	uint32_t imp_err_rpt; // CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(pnum)
	uint32_t imp_err_rate; // CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(pnum)
	uint32_t retry_lim; // CPS1848_PORT_X_RETRY_CNTR(pnum)
	uint32_t lane_err_rate[CPS_MAX_PORT_LANES]; // CPS1848_LANE_X_ERR_RATE_EN(lnum)
	uint32_t stat_n_ctl; // CPS1848_PORT_X_STATUS_AND_CTL(pnum)
	bool wr_dev_ctl_regs; // true if dev_ctl1, i2c_mast_ctl, aux_port capture
			      //      should be written
	uint32_t dev_ctl1; // CPS1848_DEVICE_CTL_1
	uint32_t i2c_mast_ctl; // CPS1848_I2C_MASTER_CTL
	uint32_t aux_port_capt; // CPS1848_AUX_PORT_ERR_CAPT_EN
	bool wr_err_det_regs; // true if the registers below this line should be
			      //      written.  Note that these registers are
			      //      only written to initialize event status
			      //      by clearing all events.
	uint32_t err_det_csr; // CPS1848_PORT_X_ERR_DET_CSR(pnum)
	uint32_t imp_err_det; // CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum)
} cps_event_cfg_reg_values_t;


extern uint32_t init_sw_pi(DAR_DEV_INFO_t *dev_info, cps_port_info_t *pi); // CPS_PC

static uint32_t cps_get_event_cfg_reg_vals(DAR_DEV_INFO_t *dev_info, uint8_t pnum,
		uint8_t start_lane, uint8_t end_lane,
		cps_event_cfg_reg_values_t *vals, uint32_t *fail_pt)
{
	uint32_t rc;
	uint8_t lnum;

	rc = DARRegRead(dev_info, CPS1848_PORT_X_CTL_1_CSR(pnum),
			&vals->pctl1_csr);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x01);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_LT_ERR_EN_CSR, &vals->log_en_csr);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x02);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PKT_TTL_CSR, &vals->ttl_csr);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x02);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_RATE_EN_CSR(pnum),
			&vals->err_rate_en_csr);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x02);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_RATE_CSR(pnum),
			&vals->err_rate_csr);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x02);
		goto exit;
	}
	vals->wr_err_rate_csr = false;

	rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_RATE_THRESH_CSR(pnum),
			&vals->err_thrsh_csr);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x03);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_RPT_EN(pnum),
			&vals->err_rpt);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x04);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(pnum),
			&vals->imp_err_rpt);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x06);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(pnum),
			&vals->imp_err_rate);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x07);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_RETRY_CNTR(pnum),
			&vals->retry_lim);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x07);
		goto exit;
	}

	for (lnum = start_lane;
			(lnum < end_lane)
					&& ((lnum - start_lane)
							< CPS_MAX_PORT_LANES);
			lnum++) {
		rc = DARRegRead(dev_info, CPS1848_LANE_X_ERR_RATE_EN(lnum),
				&vals->lane_err_rate[lnum - start_lane]);
		if (RIO_SUCCESS != rc) {
			*fail_pt = EM_GET_EC_REGS(0x08);
			goto exit;
		}
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_STATUS_AND_CTL(pnum),
			&vals->stat_n_ctl);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x08);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_DEVICE_CTL_1, &vals->dev_ctl1);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x08);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_I2C_MASTER_CTL, &vals->i2c_mast_ctl);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x08);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_AUX_PORT_ERR_CAPT_EN,
			&vals->aux_port_capt);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_GET_EC_REGS(0x08);
		goto exit;
	}

	vals->wr_err_det_regs = false;
	vals->err_det_csr = 0;
	vals->imp_err_det = 0;

	// The SET_ACKID_EN event is useless, because it is set every time the
	// CPS1848_PORT_X_LOCAL_ACKID_CSR register is written.  This driver uses
	// this event as an indication that it is OK to clear all events on the port.
	//
	// The RATE_EN and RPT_EN registers are also cleared to prevent events not
	// managed by this code from creating events that can't be handled.
	if ((vals->imp_err_rate
			& CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_SET_ACKID_EN)
			|| (vals->imp_err_rpt
					& CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_SET_ACKID_EN)) {
		vals->wr_err_det_regs = true;
		vals->imp_err_rate = 0;
		vals->imp_err_rpt = 0;
	}

exit:
	return rc;
}

static uint32_t cps_set_event_cfg_reg_vals(DAR_DEV_INFO_t *dev_info, uint8_t pnum,
		uint8_t start_lane, uint8_t end_lane,
		cps_event_cfg_reg_values_t *vals, uint32_t *fail_pt)
{
	uint32_t rc;
	uint8_t lnum;

	// Hook to manage err_rpt[CPS1848_PORT_X_ERR_RPT_EN_IMP_SPEC_ERR_EN]
	// This function is implemented here as the best place to make a
	// central decision on whether or not this bit must be set.
	if (vals->imp_err_rpt) {
		vals->err_rpt |= CPS1848_PORT_X_ERR_RPT_EN_IMP_SPEC_ERR_EN;
	} else {
		vals->err_rpt &= ~CPS1848_PORT_X_ERR_RPT_EN_IMP_SPEC_ERR_EN;
	}

	rc = DARRegWrite(dev_info, CPS1848_PORT_X_CTL_1_CSR(pnum),
			vals->pctl1_csr);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_SET_EC_REGS(0x01);
		goto exit;
	}

	// Must keep the Log Enable and Log Report Enable in sync
	rc = DARRegWrite(dev_info, CPS1848_LT_ERR_EN_CSR, vals->log_en_csr);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_SET_EC_REGS(0x02);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_LT_ERR_RPT_EN, vals->log_en_csr);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_SET_EC_REGS(0x02);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_PKT_TTL_CSR, vals->ttl_csr);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_SET_EC_REGS(0x02);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_PORT_X_ERR_RATE_EN_CSR(pnum),
			vals->err_rate_en_csr);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_SET_EC_REGS(0x02);
		goto exit;
	}

	if (vals->wr_err_rate_csr) {
		rc = DARRegWrite(dev_info, CPS1848_PORT_X_ERR_RATE_CSR(pnum),
				vals->err_rate_csr);
		if (RIO_SUCCESS != rc) {
			*fail_pt = EM_SET_EC_REGS(0x02);
			goto exit;
		}
	}

	// Hook to enforce programming model where the degraded threshold must
	// always be less than the failure threshold.

	if (((vals->err_thrsh_csr
			& CPS1848_PORT_X_ERR_RATE_THRESH_CSR_DEGR_THRESH) >> 16)
			> ((vals->err_thrsh_csr
					& CPS1848_PORT_X_ERR_RATE_THRESH_CSR_FAIL_THRESH)
					>> 24))
		vals->err_thrsh_csr &=
		CPS1848_PORT_X_ERR_RATE_THRESH_CSR_FAIL_THRESH;

	rc = DARRegWrite(dev_info, CPS1848_PORT_X_ERR_RATE_THRESH_CSR(pnum),
			vals->err_thrsh_csr);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_SET_EC_REGS(0x03);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_PORT_X_ERR_RPT_EN(pnum),
			vals->err_rpt);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_SET_EC_REGS(0x04);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(pnum),
			vals->imp_err_rpt);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_SET_EC_REGS(0x06);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(pnum),
			vals->imp_err_rate);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_SET_EC_REGS(0x07);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_PORT_X_STATUS_AND_CTL(pnum),
			vals->stat_n_ctl);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_SET_EC_REGS(0x08);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_PORT_X_RETRY_CNTR(pnum),
			vals->retry_lim);
	if (RIO_SUCCESS != rc) {
		*fail_pt = EM_SET_EC_REGS(0x07);
		goto exit;
	}

	for (lnum = start_lane;
			(lnum < end_lane)
					&& ((lnum - start_lane)
							< CPS_MAX_PORT_LANES);
			lnum++) {
		rc = DARRegWrite(dev_info, CPS1848_LANE_X_ERR_RATE_EN(lnum),
				vals->lane_err_rate[lnum - start_lane]);
		if (RIO_SUCCESS != rc) {
			*fail_pt = EM_SET_EC_REGS(0x08);
			goto exit;
		}
	}

	if (vals->wr_dev_ctl_regs) {
		rc = DARRegWrite(dev_info, CPS1848_DEVICE_CTL_1,
				vals->dev_ctl1);
		if (RIO_SUCCESS != rc) {
			*fail_pt = EM_SET_EC_REGS(0x08);
			goto exit;
		}

		rc = DARRegWrite(dev_info, CPS1848_I2C_MASTER_CTL,
				vals->i2c_mast_ctl);
		if (RIO_SUCCESS != rc) {
			*fail_pt = EM_SET_EC_REGS(0x08);
			goto exit;
		}

		rc = DARRegWrite(dev_info, CPS1848_AUX_PORT_ERR_CAPT_EN,
				vals->aux_port_capt);
		if (RIO_SUCCESS != rc) {
			*fail_pt = EM_SET_EC_REGS(0x08);
			goto exit;
		}
	}

	if (vals->wr_err_det_regs) {
		rc = DARRegWrite(dev_info, CPS1848_PORT_X_ERR_DET_CSR(pnum),
				vals->err_det_csr);
		if (RIO_SUCCESS != rc) {
			*fail_pt = EM_SET_EC_REGS(0x08);
			goto exit;
		}

		rc = DARRegWrite(dev_info,
				CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
				vals->imp_err_det);
		if (RIO_SUCCESS != rc) {
			*fail_pt = EM_SET_EC_REGS(0x08);
			goto exit;
		}
	}

exit:
	return rc;
}

#define SET_EVENT_EN(x) (EM_SET_EVENT_EN_0+x)

static void cps_em_f_los_ctl(uint8_t first_lane, uint8_t last_lane,
		rio_em_cfg_t *event, cps_event_cfg_reg_values_t *regs)
{
	uint8_t lnum;

	// Set all of the per-lane enables...

	for (lnum = first_lane; lnum < last_lane; lnum++) {
		if (rio_em_detect_on == event->em_detect) {
			regs->lane_err_rate[lnum - first_lane] =
			CPS1616_LANE_X_ERR_RATE_EN_LANE_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_RDY_EN;
		} else {
			regs->lane_err_rate[lnum - first_lane] &=
					~(CPS1616_LANE_X_ERR_RATE_EN_LANE_SYNC_EN
					| CPS1616_LANE_X_ERR_RATE_EN_LANE_RDY_EN);
		}
	}

	// Enable loss-of-alignment events as well.
	// This is not reliable for high speed links/lanes, so we need the
	// rate based events as well...

	if (rio_em_detect_on == event->em_detect) {
		regs->imp_err_rate =
		CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_LOA_EN;
		regs->imp_err_rpt |=
		CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_ERR_RATE_EN;
		regs->err_rpt = CPS1848_PORT_X_ERR_RPT_EN_IMP_SPEC_ERR_EN;
	} else {
		regs->imp_err_rate &=
			~CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_LOA_EN;
		regs->imp_err_rpt &=
			~CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_ERR_RATE_EN;
		regs->err_rpt &=
			~CPS1848_PORT_X_ERR_RPT_EN_IMP_SPEC_ERR_EN;
	}

	// Enable isolation and counting ...
	// Note: Isolation and counting are never disabled once enabled.

	if (rio_em_detect_on == event->em_detect) {
		regs->pctl1_csr |=
		CPS1848_PORT_X_CTL_1_CSR_STOP_ON_PORT_FAIL_ENC_EN |
		CPS1848_PORT_X_CTL_1_CSR_DROP_PKT_EN;

		regs->err_rate_en_csr =
		CPS1848_PORT_X_ERR_RATE_EN_CSR_IMP_SPEC_ERR_EN;
	} else {
		regs->pctl1_csr &=
		~(CPS1848_PORT_X_CTL_1_CSR_STOP_ON_PORT_FAIL_ENC_EN |
		CPS1848_PORT_X_CTL_1_CSR_DROP_PKT_EN);

		regs->err_rate_en_csr &=
		~CPS1848_PORT_X_ERR_RATE_EN_CSR_IMP_SPEC_ERR_EN;
	}

	// Now manage thresholds.
	//
	// Always clear counters regardless of enable or disable.
	//
	regs->wr_err_rate_csr = true;
	regs->err_rate_csr &= (RIO_EMHS_SPX_RATE_RR | RIO_EMHS_SPX_RATE_RB);

	// Only LOS events are enabled.
	if (rio_em_detect_on == event->em_detect) {
		regs->err_rate_csr = RIO_EMHS_SPX_RATE_RB_NONE
				| RIO_EMHS_SPX_RATE_RR_LIM_NONE;
		regs->err_thrsh_csr = (1 << 24)
				& RIO_EMHS_SPX_THRESH_FAIL;
	} else {
		regs->err_rate_csr = 0;
		regs->err_thrsh_csr = 0;
	}
}

#define CPSGEN2_ALL_LOG (CPS1848_LT_ERR_EN_CSR_IMP_SPEC_ERR_EN | \
		CPS1848_LT_ERR_EN_CSR_UNSUP_TRAN_EN | \
		CPS1848_LT_ERR_EN_CSR_UNSOL_RESP_EN | \
		CPS1848_LT_ERR_EN_CSR_ILL_TRAN_EN)

// Implementation specific error controlled by rio_em_f_los
// CS NOT ACC controlled by rio_em_f_2many_pna, different threshold
// LR ACKID ILL causes rio_em_f_port_err, so no threshold possible
// Note: LR_TIMEOUT only causes a port err after 16 consecutive,
// so can put a lower threshold on these events.
//
#define CPSGEN2_ERR_RATE_EVENT_EXCLUSIONS (CPS1848_PORT_X_ERR_DET_CSR_IMP_SPEC_ERR | \
		CPS1848_PORT_X_ERR_RATE_EN_CSR_CS_NOT_ACC_EN | \
		CPS1848_PORT_X_ERR_RATE_EN_CSR_LR_ACKID_ILL_EN)

static uint32_t cps_set_event_en_cfg(DAR_DEV_INFO_t *dev_info, uint8_t pnum,
		uint8_t first_lane, uint8_t last_lane, rio_em_cfg_t *event,
		cps_event_cfg_reg_values_t *regs, uint32_t *fail_pt)
{
	uint32_t ttl;
	uint32_t rate_en;
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;

	if ((event->em_detect >= rio_em_detect_last)
			|| (event->em_event >= rio_em_last)) {
		*fail_pt = SET_EVENT_EN(2);
		goto exit;
	}

	if (rio_em_detect_0delta == event->em_detect) {
		rc = RIO_SUCCESS;
		goto exit;
	}

	switch (event->em_event) {
	case rio_em_f_los: // CPS does not have a "dead link timer", just a simple
			   //    indication of whether or not the link partner is no longer
			   //    transmitting.
			   // Indy has the ability to "infer" silence by detecting the lack
			   //    of Clock Compensation Sequences.  This is problematic when
			   //    connected to Tsi57x devices.
			   // NOTE: LOS is a RATE event so that isolation occurs on failure
		// SET_EVENT_EN 0x10-0x1F
		cps_em_f_los_ctl(first_lane, last_lane, event, regs);
		break;

	case rio_em_f_port_err: // CPS has indirect notification of a port error.
				// A port error occurs for a FATAL_TO, or whenever a
				// link response with an illegal ackID is received.
				// Isolation is controlled by the
				// CPS1848_DEVICE_CTL_1_FATAL_ERR_PKT_MGT bit.

		if (rio_em_detect_on == event->em_detect) {
			regs->err_rpt |=
			CPS1848_PORT_X_ERR_RPT_EN_LR_ACKID_ILL_EN;
			regs->imp_err_rpt |=
			CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_FATAL_TO_EN;
			regs->dev_ctl1 &=
					~CPS1848_DEVICE_CTL_1_FATAL_ERR_PKT_MGT;
		} else {
			regs->err_rpt &=
					~CPS1848_PORT_X_ERR_RPT_EN_LR_ACKID_ILL_EN;
			regs->imp_err_rpt &=
					~CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_FATAL_TO_EN;
		}
		break;

	case rio_em_f_2many_retx: // Too many retries has it's own control for "rate".
				  // Isolation always happens when this event is detected,
				  // so it is a "Report" event.
		if (rio_em_detect_on == event->em_detect) {
			regs->imp_err_rpt |=
			CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_MANY_RETRY_EN;
			regs->retry_lim = (event->em_info << 16)
					& CPS1848_PORT_X_RETRY_CNTR_RETRY_LIM;
			if (!regs->retry_lim) {
				*fail_pt = SET_EVENT_EN(0x32);
				goto exit;
			}
			regs->stat_n_ctl |=
			CPS1848_PORT_X_STATUS_AND_CTL_RETRY_LIM_EN;
		} else {
			// Note: Writing 0 to retry_lim will cause a 2 MANY RETRIES event.
			regs->imp_err_rpt &=
					~CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_MANY_RETRY_EN;
			regs->retry_lim = 0xFFFF0000;
			regs->stat_n_ctl &=
					~CPS1848_PORT_X_STATUS_AND_CTL_RETRY_LIM_EN;
		}
		break;

	case rio_em_f_2many_pna: // Too many PNAs will set a rate according to em_info if
				 //    no standard rate event is enabled.  This will overwrite
				 //    rates set for rio_em_f_los.
		if (rio_em_detect_on == event->em_detect) {
			regs->imp_err_rate |=
			CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_PNA_EN;
			regs->err_rpt |=
			CPS1848_PORT_X_ERR_RPT_EN_IMP_SPEC_ERR_EN;
			regs->pctl1_csr |=
			CPS1848_PORT_X_CTL_1_CSR_STOP_ON_PORT_FAIL_ENC_EN |
			CPS1848_PORT_X_CTL_1_CSR_DROP_PKT_EN;

		} else {
			regs->imp_err_rate &=
					~CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_PNA_EN;
		}

		// Only set the thresholds, leak rate and max count values
		// when no standard error rate events are enabled.
		// Make sure the error threshold register is written, writing it may have
		// been disabled by other events.
		regs->wr_err_rate_csr = true;
		regs->err_rate_csr &= (RIO_EMHS_SPX_RATE_RB
				| RIO_EMHS_SPX_RATE_RR);

		//@sonar:off - Collapsible "if" statements should be merged
		if (!(regs->err_rate_en_csr & ~CPSGEN2_ERR_RATE_EVENT_EXCLUSIONS)) {
			if (rio_em_detect_on == event->em_detect) {
				// Set the RB & RD to defaults, clear error counter.
				regs->err_rate_csr = RIO_EMHS_SPX_RATE_RB_NONE
						| RIO_EMHS_SPX_RATE_RR_LIM_NONE;
				// Set the threshold according to em_info
				regs->err_thrsh_csr &=
						~CPS1848_PORT_X_ERR_RATE_THRESH_CSR_FAIL_THRESH;
				regs->err_thrsh_csr |=
						(event->em_info << 24)
								& CPS1848_PORT_X_ERR_RATE_THRESH_CSR_FAIL_THRESH;
				if (!(regs->err_thrsh_csr
						& CPS1848_PORT_X_ERR_RATE_THRESH_CSR_FAIL_THRESH)) {
					*fail_pt = SET_EVENT_EN(0x45);
					goto exit;
				}
			}
		}
		//@sonar:on
		break;

	case rio_em_f_err_rate:
		// Always clear the counter on enable or disable...
		regs->wr_err_rate_csr = true;
		regs->err_rate_csr &= (RIO_EMHS_SPX_RATE_RB
				| RIO_EMHS_SPX_RATE_RR);

		if (event->em_detect == rio_em_detect_on) {
			rc = rio_em_get_f_err_rate_info(event->em_info,
					&rate_en, &regs->err_rate_csr,
					&regs->err_thrsh_csr);
			if (RIO_SUCCESS != rc) {
				*fail_pt = SET_EVENT_EN(0x50);
				goto exit;
			}
			regs->err_rate_en_csr =
					(rate_en
							& ~CPSGEN2_ERR_RATE_EVENT_EXCLUSIONS)
							| (regs->err_rate_en_csr
									& CPSGEN2_ERR_RATE_EVENT_EXCLUSIONS);

			regs->pctl1_csr |=
			CPS1848_PORT_X_CTL_1_CSR_STOP_ON_PORT_FAIL_ENC_EN |
			CPS1848_PORT_X_CTL_1_CSR_DROP_PKT_EN;
		} else {
			regs->err_rate_en_csr &=
			CPSGEN2_ERR_RATE_EVENT_EXCLUSIONS;
		}
		break;

	case rio_em_d_ttl:  // TTL is a REPORT event
		if (rio_em_detect_on == event->em_detect) {
			ttl = (((event->em_info + 1599) / 1600) << 16)
					& CPS1848_PKT_TTL_CSR_TTL;
			if (!ttl) {
				// Error to enable TTL with an interval of 0.
				*fail_pt = SET_EVENT_EN(0x61);
				goto exit;
			}
			regs->ttl_csr = ttl;
			regs->imp_err_rpt |=
			CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_TTL_EVENT_EN;
		} else {
			regs->ttl_csr = 0;
		}
		// Never report TTL events, as they may cause port-writes to be
		// transmitted at unexpected times.  This breaks ackID clearing
		// and ackID syncing when handling errors.
		regs->imp_err_rpt &= ~CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_TTL_EVENT_EN;
		break;

	case rio_em_d_rte: // Routing issues are a REPORT event.

		if (rio_em_detect_on == event->em_detect) {
			regs->imp_err_rpt |=
			CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_RTE_ISSUE_EN;
		} else {
			regs->imp_err_rpt &=
					~CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_RTE_ISSUE_EN;
		}
		break;

	case rio_em_d_log: // Logical/transport layer errors detection is controlled by INFO.
		if (rio_em_detect_on == event->em_detect) {
			regs->log_en_csr = event->em_info & CPSGEN2_ALL_LOG;
		} else {
			regs->log_en_csr = 0;
		}
		regs->wr_dev_ctl_regs = true;
		break;

	case rio_em_i_sig_det:  // This is a REPORT event
		// If the port is not available, fail as this routine should
		// only be called for available, powered up ports...
		if (first_lane == last_lane) {
			rc = RIO_ERR_SW_FAILURE;
			*fail_pt = SET_EVENT_EN(0x90);
			goto exit;
		}

		if (!event->em_info) {
			uint32_t err_stat_csr;
			// Turn notification on/off for ports that need it.
			// Notification should be turned on for UNINIT ports.
			// Notification should be turned off for PORT_OK ports.

			rc = DARRegRead(dev_info,
					CPS1848_PORT_X_ERR_STAT_CSR(pnum),
					&err_stat_csr);
			if (RIO_SUCCESS != rc) {
				*fail_pt = SET_EVENT_EN(0x92);
				goto exit;
			}

			if (((rio_em_detect_off == event->em_detect)
					&& (!(err_stat_csr
							& CPS1848_PORT_X_ERR_STAT_CSR_PORT_OK)))
					|| ((rio_em_detect_on
							== event->em_detect)
							&& (err_stat_csr
									& CPS1848_PORT_X_ERR_STAT_CSR_PORT_OK))) {
				break;
			}
		}

		if (rio_em_detect_on == event->em_detect) {
			regs->imp_err_rpt |=
			CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_PORT_INIT_EN;
		} else {
			regs->imp_err_rpt &=
					~CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_PORT_INIT_EN;
		}
		break;

	case rio_em_a_no_event:  // Nothing to do
	case rio_em_a_clr_pwpnd:  // Nothing to do
	case rio_em_i_rst_req:
		break;  // Not supported...

	case rio_em_i_init_fail: // Notification for I2C Initialization failures
		if (rio_em_detect_on == event->em_detect) {
			regs->i2c_mast_ctl &=
					~CPS1848_I2C_MASTER_CTL_CHKSUM_DIS;
			regs->aux_port_capt |=
			CPS1848_AUX_PORT_ERR_CAPT_EN_I2C_CHKSUM_ERR_EN;
		} else {
			regs->i2c_mast_ctl |= CPS1848_I2C_MASTER_CTL_CHKSUM_DIS;
			regs->aux_port_capt &=
					~CPS1848_AUX_PORT_ERR_CAPT_EN_I2C_CHKSUM_ERR_EN;
		}
		regs->wr_dev_ctl_regs = true;
		break;

	//@sonar:off - c:S3458
	case rio_em_last:
	default:
	//@sonar:on
		*fail_pt = SET_EVENT_EN(0x95);
		goto exit;
	}

	rc = RIO_SUCCESS;

exit:
	return rc;
}

#define SET_EVENT_PW(x) (EM_SET_EVENT_PW_0+x)

static uint32_t cps_set_pw_cfg(DAR_DEV_INFO_t *dev_info, struct DAR_ptl *good_ptl,
		rio_em_notfn_ctl_t notfn, cps_port_info_t *pi,
		uint32_t *fail_pt)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t port_idx, pnum;

	// Validate notfn
	if (rio_em_notfn_last <= notfn) {
		*fail_pt = SET_EVENT_PW(1);
		goto exit;
	}

	// Check if something must be done.
	if (rio_em_notfn_0delta == notfn) {
		rc = RIO_SUCCESS;
		goto exit;
	}

	for (port_idx = 0; port_idx < good_ptl->num_ports; port_idx++) {
		uint8_t quadrant, quad_cfg;
		uint8_t first_lane, last_lane, lnum;
		uint32_t lctl, ops;

		pnum = good_ptl->pnums[port_idx];
		quadrant = pi->cpr[pnum].quadrant;
		quad_cfg = pi->quad_cfg_val[quadrant];

		// If port has no lanes assigned, skip it
		if (!pi->cpr[pnum].cfg[quad_cfg].lane_count) {
			continue;
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_OPS(pnum), &ops);
		if (RIO_SUCCESS != rc) {
			*fail_pt = SET_EVENT_PW(3);
			goto exit;
		}

		//@sonar:off - c:S3458
		switch (notfn) {
		default:  // Default case will not be activated...
		case rio_em_notfn_none:
		case rio_em_notfn_int:
			// Disable port write event notification
			ops &= ~CPS1848_PORT_X_OPS_PORT_PW_EN;
			break;
		case rio_em_notfn_pw:
		case rio_em_notfn_both:
			// Enable port write event notification
			ops |= CPS1848_PORT_X_OPS_PORT_PW_EN;
			break;
		}
		//@sonar:on

		rc = DARRegWrite(dev_info, CPS1848_PORT_X_OPS(pnum), ops);
		if (RIO_SUCCESS != rc) {
			*fail_pt = SET_EVENT_PW(4);
			goto exit;
		}

		first_lane = pi->cpr[pnum].cfg[quad_cfg].first_lane;
		last_lane = pi->cpr[pnum].cfg[quad_cfg].lane_count + first_lane;
		for (lnum = first_lane; lnum < last_lane; lnum++) {
			rc = DARRegRead(dev_info, CPS1848_LANE_X_CTL(lnum),
					&lctl);
			if (RIO_SUCCESS != rc) {
				*fail_pt = SET_EVENT_PW(5);
				goto exit;
			}

			//@sonar:off - c:S3458
			switch (notfn) {
			default:  // Default case will not be activated...
			case rio_em_notfn_none:
			case rio_em_notfn_int:
				// Disable port write event notification
				lctl &= ~CPS1848_LANE_X_CTL_LANE_PW_EN;
				break;
			case rio_em_notfn_pw:
			case rio_em_notfn_both:
				// Enable port write event notification
				lctl |= CPS1848_LANE_X_CTL_LANE_PW_EN;
				break;
			}
			//@sonar:on

			rc = DARRegWrite(dev_info, CPS1848_LANE_X_CTL(lnum),
					lctl);
			if (RIO_SUCCESS != rc) {
				*fail_pt = SET_EVENT_PW(6);
				goto exit;
			}
		}
	}

	// Set up global port write controls if "all ports" were requested...
	if (good_ptl->num_ports > 1) {
		uint32_t dev1, i2c_ctl;

		rc = DARRegRead(dev_info, CPS1848_DEVICE_CTL_1, &dev1);
		if (RIO_SUCCESS != rc) {
			*fail_pt = SET_EVENT_PW(7);
			goto exit;
		}

		rc = DARRegRead(dev_info, CPS1848_I2C_MASTER_CTL, &i2c_ctl);
		if (RIO_SUCCESS != rc) {
			*fail_pt = SET_EVENT_PW(8);
			goto exit;
		}

		//@sonar:off - c:S3458
		switch (notfn) {
		default: // Default case will not be activated...
		case rio_em_notfn_none:
		case rio_em_notfn_int:
			// Disable port write event notification
			dev1 &= ~CPS1848_DEVICE_CTL_1_LT_PW_EN;
			i2c_ctl &= ~CPS1848_I2C_MASTER_CTL_I2C_PW_EN;
			break;
		case rio_em_notfn_pw:
		case rio_em_notfn_both:
			// Enable port write event notification
			dev1 |= CPS1848_DEVICE_CTL_1_LT_PW_EN;
			i2c_ctl |= CPS1848_I2C_MASTER_CTL_I2C_PW_EN;
			break;
		}
		//@sonar:on

		rc = DARRegWrite(dev_info, CPS1848_DEVICE_CTL_1, dev1);
		if (RIO_SUCCESS != rc) {
			*fail_pt = SET_EVENT_PW(9);
			goto exit;
		}

		rc = DARRegWrite(dev_info, CPS1848_I2C_MASTER_CTL, i2c_ctl);
		if (RIO_SUCCESS != rc) {
			*fail_pt = SET_EVENT_PW(0xA);
			goto exit;
		}
	}

	rc = RIO_SUCCESS;

exit:
	return rc;
}

#define SET_EVENT_INT(x) (EM_SET_EVENT_INT_0+x)

static uint32_t cps_set_int_cfg(DAR_DEV_INFO_t *dev_info, struct DAR_ptl *good_ptl,
		rio_em_notfn_ctl_t notfn, cps_port_info_t *pi,
		uint32_t *fail_pt)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t port_idx, pnum;

	// Validate notfn
	if (rio_em_notfn_last <= notfn) {
		*fail_pt = SET_EVENT_INT(1);
		goto exit;
	}

	// Check if something must be done.
	if (rio_em_notfn_0delta == notfn) {
		rc = RIO_SUCCESS;
		goto exit;
	}

	for (port_idx = 0; port_idx < good_ptl->num_ports; port_idx++) {
		uint8_t quadrant, quad_cfg;
		uint8_t first_lane, last_lane, lnum;
		uint32_t lctl, ops;

		pnum = good_ptl->pnums[port_idx];
		quadrant = pi->cpr[pnum].quadrant;
		quad_cfg = pi->quad_cfg_val[quadrant];

		// If port has no lanes assigned, skip it
		if (!pi->cpr[pnum].cfg[quad_cfg].lane_count) {
			continue;
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_OPS(pnum), &ops);
		if (RIO_SUCCESS != rc) {
			*fail_pt = SET_EVENT_INT(3);
			goto exit;
		}

		//@sonar:off - c:S3458
		switch (notfn) {
		default: // Default case will not be activated...
		case rio_em_notfn_none:
		case rio_em_notfn_pw:
			// Disable interrupt event notification
			ops &= ~CPS1848_PORT_X_OPS_PORT_INT_EN;
			break;
		case rio_em_notfn_int:
		case rio_em_notfn_both:
			// Enable interrupt event notification
			ops |= CPS1848_PORT_X_OPS_PORT_INT_EN;
			break;
		}
		//@sonar:on

		rc = DARRegWrite(dev_info, CPS1848_PORT_X_OPS(pnum), ops);
		if (RIO_SUCCESS != rc) {
			*fail_pt = SET_EVENT_INT(4);
			goto exit;
		}

		first_lane = pi->cpr[pnum].cfg[quad_cfg].first_lane;
		last_lane = pi->cpr[pnum].cfg[quad_cfg].lane_count + first_lane;
		for (lnum = first_lane; lnum < last_lane; lnum++) {
			rc = DARRegRead(dev_info, CPS1848_LANE_X_CTL(lnum),
					&lctl);
			if (RIO_SUCCESS != rc) {
				*fail_pt = SET_EVENT_INT(5);
				goto exit;
			}

			//@sonar:off - c:S3458
			switch (notfn) {
			default: // Default case will not be activated...
			case rio_em_notfn_none:
			case rio_em_notfn_pw:
				// Disable interrupt event notification
				lctl &= ~CPS1848_LANE_X_CTL_LANE_INT_EN;
				break;
			case rio_em_notfn_int:
			case rio_em_notfn_both:
				// Enable interrupt event notification
				lctl |= CPS1848_LANE_X_CTL_LANE_INT_EN;
				break;
			}
			//@sonar:on

			rc = DARRegWrite(dev_info, CPS1848_LANE_X_CTL(lnum),
					lctl);
			if (RIO_SUCCESS != rc) {
				*fail_pt = SET_EVENT_INT(6);
				goto exit;
			}
		}
	}

	// Set up global interrupt controls if "all ports" were requested...
	if (good_ptl->num_ports > 1) {
		uint32_t dev1, i2c_ctl;

		rc = DARRegRead(dev_info, CPS1848_DEVICE_CTL_1, &dev1);
		if (RIO_SUCCESS != rc) {
			*fail_pt = SET_EVENT_INT(7);
			goto exit;
		}

		rc = DARRegRead(dev_info, CPS1848_I2C_MASTER_CTL, &i2c_ctl);
		if (RIO_SUCCESS != rc) {
			*fail_pt = SET_EVENT_INT(8);
			goto exit;
		}

		//@sonar:off - c:S3458
		switch (notfn) {
		default: // Default case will not be activated...
		case rio_em_notfn_none:
		case rio_em_notfn_pw:
			// Disable interrupt event notification
			dev1 &= ~CPS1848_DEVICE_CTL_1_LT_INT_EN;
			i2c_ctl &= ~CPS1848_I2C_MASTER_CTL_I2C_INT_EN;
			break;
		case rio_em_notfn_int:
		case rio_em_notfn_both:
			// Enable interrupt event notification
			dev1 |= CPS1848_DEVICE_CTL_1_LT_INT_EN;
			i2c_ctl |= CPS1848_I2C_MASTER_CTL_I2C_INT_EN;
			break;
		}
		//@sonar:on

		rc = DARRegWrite(dev_info, CPS1848_DEVICE_CTL_1, dev1);
		if (RIO_SUCCESS != rc) {
			*fail_pt = SET_EVENT_INT(9);
			goto exit;
		}

		rc = DARRegWrite(dev_info, CPS1848_I2C_MASTER_CTL, i2c_ctl);
		if (RIO_SUCCESS != rc) {
			*fail_pt = SET_EVENT_INT(0xA);
			goto exit;
		}
	}

	rc = RIO_SUCCESS;

exit:
	return rc;
}

static uint32_t cps_determine_notfn(DAR_DEV_INFO_t *dev_info,
		rio_em_notfn_ctl_t *notfn, struct DAR_ptl *good_ptl,
		cps_port_info_t *pi, uint32_t *fail_pt)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t pnum, port_idx;

	*notfn = rio_em_notfn_last;

	for (port_idx = 0; port_idx <= good_ptl->num_ports; port_idx++) {
		uint8_t quadrant, quad_cfg;
		uint32_t ops;
		rio_em_notfn_ctl_t t_notfn;

		pnum = good_ptl->pnums[port_idx];

		quadrant = pi->cpr[pnum].quadrant;
		quad_cfg = pi->quad_cfg_val[quadrant];

		// If port has no lanes assigned, skip it
		if (!pi->cpr[pnum].cfg[quad_cfg].lane_count) {
			continue;
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_OPS(pnum), &ops);
		if (RIO_SUCCESS != rc) {
			*fail_pt = SET_EVENT_INT(3);
			goto exit;
		}

		if (ops & CPS1848_PORT_X_OPS_PORT_PW_EN) {
			if (ops & CPS1848_PORT_X_OPS_PORT_INT_EN) {
				t_notfn = rio_em_notfn_both;
			} else {
				t_notfn = rio_em_notfn_pw;
			}
		} else {
			if (ops & CPS1848_PORT_X_OPS_PORT_INT_EN) {
				t_notfn = rio_em_notfn_int;
			} else {
				t_notfn = rio_em_notfn_none;
			}
		}

		if (rio_em_notfn_last == *notfn) {
			*notfn = t_notfn;
		} else {
			if (t_notfn != *notfn) {
				*notfn = rio_em_notfn_0delta;
			}
		}
	}

exit:
	return rc;
}

// Routines to convert port-write contents into an event list,
// and to query a port/device and return a list of asserted events
// which are reported via interrupt or port-write,

#define PW_PORT_NUM_MASK  (uint32_t)(0x000000FF)
#define PW_ES_MASK        (uint32_t)(0x0000FF00)
#define PW_EC_MASK        (uint32_t)(0x000000FF)
#define PW_LANE_ES_MASK   (uint32_t)(0x00000008)
#define PW_LANE_ES_VAL    (uint32_t)(0x00000008)
#define PW_LANE_IMP_SPEC  (uint32_t)(0x00000008)

// Parse port-write with a format compliant to the RapidIO Standard, Part 8
//

typedef struct CPS_known_events_t_TAG {
	uint8_t error_code;
	rio_em_events_t event;
} CPS_known_events_t;

const CPS_known_events_t port_err_codes[] = {{0x77, rio_em_f_los}, // Loss of alignment
		{0x8B, rio_em_f_port_err}, // LR_ACKID_ILL
		{0x8C, rio_em_f_port_err}, // FATAL_TO
		{0xAA, rio_em_f_2many_retx}, // MANY_RETRY
		{0xA7, rio_em_f_err_rate}, // ERR_RATE
		{0xA4, rio_em_d_ttl}, // Time to Live
		{0xA1, rio_em_d_rte}, // Routing table has "No Route" value
		{0xA2, rio_em_d_rte}, // Destination port is disabled (PORT_DIS = 1)
		{0xA3, rio_em_d_rte}, // Destination port has PORT_ERR
		{0xA9, rio_em_d_rte}, // Destination port has PORT_LOCKOUT = 1
		{0x85, rio_em_i_sig_det}, // PORT_INIT
		{0x00, rio_em_last}, // End of list
		};

const CPS_known_events_t lane_err_codes[] = {{0x60, rio_em_f_los}, // LANE_SYNC
		{0x61, rio_em_f_los}, // LANE_RDY
		{0x00, rio_em_last}, // End of list
		};

const CPS_known_events_t i2c_err_codes[] = {{0x10, rio_em_i_init_fail}, // I2C_LENGTH_ERR
		{0x14, rio_em_i_init_fail}, // I2C_CHKSUM_ERR
		{0x00, rio_em_last}, // End of list
		};

static bool cps_find_event_code(rio_em_parse_pw_in_t *in_parms,
		rio_em_parse_pw_out_t *out_parms, uint8_t code_val,
		const CPS_known_events_t *err_codes)
{
	uint8_t code_idx = 0;
	bool found_one = false;

	while (rio_em_last != err_codes[code_idx].event) {
		if (err_codes[code_idx].error_code == code_val) {
			in_parms->events[out_parms->num_events].event =
					err_codes[code_idx].event;
			out_parms->num_events++;
			found_one = true;
			break;
		}
		code_idx++;
	}

	return found_one;
}


static void cps_parse_std_pw(rio_em_parse_pw_in_t *in_parms,
		rio_em_parse_pw_out_t *out_parms)
{
	uint32_t imp_spec = (in_parms->pw[RIO_EM_PW_IMP_SPEC_IDX]
			& RIO_EM_PW_IMP_SPEC_MASK) >> 8;
	in_parms->events[0].port_num =
			(uint8_t)(in_parms->pw[RIO_EM_PW_IMP_SPEC_IDX]
					& RIO_EM_PW_IMP_SPEC_PORT_MASK);

	// Logical layer error check...
	if (in_parms->pw[RIO_EM_PW_L_ERR_DET_IDX]) {
		in_parms->events[0].event = rio_em_d_log;
		out_parms->num_events = 1;
		return;
	}

	// Physical layer port error check
	if (in_parms->pw[RIO_EM_PW_P_ERR_DET_IDX]) {
		if (!(in_parms->pw[RIO_EM_PW_IMP_SPEC_IDX]
				& RIO_EM_PW_IMP_SPEC_MASK)) {
			// Repeated port write format.  No additional information, other than what's in
			// the Port n Error Detect CSR.  Check for fatal error, and other events, and move on...
			if (CPS1848_PORT_X_ERR_DET_CSR_LR_ACKID_ILL
					& in_parms->pw[RIO_EM_PW_P_ERR_DET_IDX]) {
				in_parms->events[0].event = rio_em_f_port_err;
				out_parms->num_events = 1;
			}

			if (~CPS1848_PORT_X_ERR_DET_CSR_LR_ACKID_ILL
					& in_parms->pw[RIO_EM_PW_P_ERR_DET_IDX])
				out_parms->other_events = true;

			return;
		}

		if (!cps_find_event_code(in_parms, out_parms,
				imp_spec & PW_EC_MASK, port_err_codes))
			out_parms->other_events = true;

		return;
	}

	// Check for "repeated port write format".
	// There is no information on the specific event, so rather than guess
	// just note "other events"

	if (!((in_parms->pw[RIO_EM_PW_IMP_SPEC_IDX] & RIO_EM_PW_IMP_SPEC_MASK)
			>> 12)) {
		out_parms->other_events = true;
		return;
	}

	switch ((imp_spec & PW_ES_MASK ) >> 12) {
	case 0: // I2C, Config, or JTAG event
		if (cps_find_event_code(in_parms, out_parms,
				imp_spec & PW_EC_MASK, i2c_err_codes)) {
			in_parms->events[0].port_num = RIO_ALL_PORTS;
		} else {
			out_parms->other_events = true;
		}
		break;

	case 4: // Lanes  0-15
	case 5: // Lanes 16-31
	case 6: // Lanes 32-47
		// There is a lane event in the port-write.
		// Check the lane event code for known events.
		if (cps_find_event_code(in_parms, out_parms,
				imp_spec & PW_EC_MASK, lane_err_codes)) {
			in_parms->events[0].port_num = RIO_ALL_PORTS;
		} else {
			out_parms->other_events = true;
		}
		break;

		// Intentionally fall through
	default:
		out_parms->other_events = true;
	}
}

typedef struct CPS_known_error_sources_t_TAG {
	uint8_t begin_error_source;
	uint8_t end_error_source;
	uint8_t port_num;
	const CPS_known_events_t * const errs;
} CPS_known_error_sources_t;

const CPS_known_error_sources_t CPS_err_sources[] = { //
		{0x2A, 0x2A, 0, port_err_codes}, //
		{0x29, 0x29, 1, port_err_codes}, //
		{0x34, 0x34, 2, port_err_codes}, //
		{0x33, 0x33, 3, port_err_codes}, //
		{0x32, 0x32, 4, port_err_codes}, //
		{0x31, 0x31, 5, port_err_codes}, //
		{0x3C, 0x3C, 6, port_err_codes}, //
		{0x3B, 0x3B, 7, port_err_codes}, //
		{0x3A, 0x3A, 8, port_err_codes}, //
		{0x39, 0x39, 9, port_err_codes}, //
		{0x3D, 0x3D, 10, port_err_codes}, //
		{0x3E, 0x3E, 11, port_err_codes}, //
		{0x1C, 0x1C, 12, port_err_codes}, //
		{0x1D, 0x1D, 13, port_err_codes}, //
		{0x27, 0x27, 14, port_err_codes}, //
		{0x26, 0x26, 15, port_err_codes}, //
		{0x25, 0x25, 16, port_err_codes}, //
		{0x24, 0x24, 17, port_err_codes}, //
		{0x40, 0x6F, RIO_ALL_PORTS, lane_err_codes}, //
		{0x00, 0x00, RIO_ALL_PORTS, i2c_err_codes}, //
		{0x00, 0x00, RIO_ALL_PORTS, (const CPS_known_events_t *)(0)}, //
};

// Parse port-write with the proprietary CPS Event Log format
//
// Index in in_parms->pw for the event log source
#define CPS_ELPW_ERR_SRC_IDX 0
#define CPS_ELPW_ERR_CODE_IDX 0
#define CPS_ELPW_ERR_SRC_MASK  0x03F00000
#define CPS_ELPW_ERR_CODE_MASK 0x000FF000

static void cps_parse_el_pw(rio_em_parse_pw_in_t *in_parms,
		rio_em_parse_pw_out_t *out_parms)
{
	uint8_t e_src = (uint8_t)((in_parms->pw[CPS_ELPW_ERR_SRC_IDX]
			& CPS_ELPW_ERR_SRC_MASK) >> 24);
	uint8_t e_cd = (uint8_t)((in_parms->pw[CPS_ELPW_ERR_CODE_IDX]
			& CPS_ELPW_ERR_CODE_MASK) >> 16);
	uint8_t src_idx = 0;
	bool found_cd = false;

	in_parms->events[out_parms->num_events].port_num = RIO_ALL_PORTS;

	while (CPS_err_sources[src_idx].errs) {
		if ((CPS_err_sources[src_idx].begin_error_source <= e_src)
				&& (CPS_err_sources[src_idx].end_error_source
						>= e_src)) {
			if (RIO_ALL_PORTS
					!= CPS_err_sources[src_idx].port_num) {
				in_parms->events[out_parms->num_events].port_num =
						CPS_err_sources[src_idx].port_num;
			}

			found_cd = cps_find_event_code(in_parms, out_parms,
					e_cd, CPS_err_sources[src_idx].errs);
			if (!found_cd) {
				out_parms->other_events = true;
			}
			break;
		}
		src_idx++;
	}
}

static uint32_t clr_bits(DAR_DEV_INFO_t *dev_info, uint32_t reg_addr,
		uint32_t mask_val, uint32_t fail_pt_val, uint32_t *fail_pt_addr)
{
	uint32_t rc;
	uint32_t regData;

	rc = DARRegRead(dev_info, reg_addr, &regData);
	if (RIO_SUCCESS != rc) {
		*fail_pt_addr = fail_pt_val;
		goto exit;
	}

	regData &= ~mask_val;
	rc = DARRegWrite(dev_info, reg_addr, regData);
	if (RIO_SUCCESS != rc) {
		*fail_pt_addr = fail_pt_val + 1;
	}

exit:
	return rc;
}

typedef struct clr_err_info_t_TAG {
	uint32_t err_det; // CPS1848_PORT_X_ERR_DET_CSR(pnum)
	uint32_t imp_err_det; // CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum)
	bool write_err_rate_n_stat;
	bool clr_pwpnd;
	uint32_t err_rate_csr; // CPS1848_PORT_X_ERR_RATE_CSR(pnum)
	uint32_t err_stat_csr; // CPS1848_PORT_X_ERR_STAT_CSR(pnum),
	bool clr_ackids;
} clr_err_info_t;

static uint32_t read_err_info(DAR_DEV_INFO_t *dev_info, cps_port_info_t *pi,
		clr_err_info_t *info, uint32_t *fail_pt)
{
	uint32_t rc = RIO_SUCCESS;
	uint8_t pnum, quadrant, quad_cfg;

	for (pnum = 0; pnum < NUM_CPS_PORTS(dev_info); pnum++) {
		quadrant = pi->cpr[pnum].quadrant;
		quad_cfg = pi->quad_cfg_val[quadrant];

		info[pnum].write_err_rate_n_stat = false;
		info[pnum].clr_pwpnd = false;

		if (!pi->cpr[pnum].cfg[quad_cfg].lane_count) {
			continue;
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_DET_CSR(pnum),
				&info[pnum].err_det);
		if (RIO_SUCCESS != rc) {
			*fail_pt = EM_CLR_EVENTS(0xF0);
			goto exit;
		}

		rc = DARRegRead(dev_info,
				CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
				&info[pnum].imp_err_det);
		if (RIO_SUCCESS != rc) {
			*fail_pt = EM_CLR_EVENTS(0xF1);
			goto exit;
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_RATE_CSR(pnum),
				&info[pnum].err_rate_csr);
		if (RIO_SUCCESS != rc) {
			*fail_pt = EM_CLR_EVENTS(0xF2);
			goto exit;
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_STAT_CSR(pnum),
				&info[pnum].err_stat_csr);
		if (RIO_SUCCESS != rc) {
			*fail_pt = EM_CLR_EVENTS(0xF3);
			goto exit;
		}
		info[pnum].clr_ackids = false;
	}
exit:
	return rc;
}

static uint32_t write_err_info(DAR_DEV_INFO_t *dev_info, cps_port_info_t *pi,
		clr_err_info_t *orig, clr_err_info_t *info, uint32_t *fail_pt)
{
	uint32_t rc = RIO_SUCCESS;
	uint32_t temp2;
	uint8_t pnum, quadrant, quad_cfg;

	for (pnum = 0; pnum < NUM_CPS_PORTS(dev_info); pnum++) {
		quadrant = pi->cpr[pnum].quadrant;
		quad_cfg = pi->quad_cfg_val[quadrant];

		if (!pi->cpr[pnum].cfg[quad_cfg].lane_count) {
			continue;
		}

		if (info[pnum].write_err_rate_n_stat) {
			if (orig[pnum].err_rate_csr != info[pnum].err_rate_csr) {
				rc = DARRegWrite(dev_info,
					CPS1848_PORT_X_ERR_RATE_CSR(pnum),
					info[pnum].err_rate_csr);
				if (RIO_SUCCESS != rc) {
					*fail_pt = EM_CLR_EVENTS(0xFB);
					goto exit;
				}
			}

			rc = DARRegWrite(dev_info,
					CPS1848_PORT_X_ERR_STAT_CSR(pnum),
					info[pnum].err_stat_csr);
			if (RIO_SUCCESS != rc) {
				*fail_pt = EM_CLR_EVENTS(0xFC);
				goto exit;
			}
		}

		if (info[pnum].clr_pwpnd) {
			rc = DARRegWrite(dev_info,
					CPS1848_PORT_X_ERR_STAT_CSR(pnum),
					info[pnum].err_stat_csr |
					CPS1848_PORT_X_ERR_STAT_CSR_PW_PNDG);
			if (RIO_SUCCESS != rc) {
				*fail_pt = EM_CLR_EVENTS(0xFD);
				goto exit;
			}
		}
		if (info[pnum].clr_ackids && (pnum != CONN_PORT(dev_info))) {
			uint32_t ackid_chk, limit = 100;
			do {
				// Must clear port-write pending
				// before clearing ackIDs
				rc |= DARRegWrite(dev_info,
					CPS1848_PORT_X_LOCAL_ACKID_CSR(pnum),
					CPS1848_PORT_X_LOCAL_ACKID_CSR_CLR);
				if (RIO_SUCCESS != rc) {
					*fail_pt = EM_CLR_EVENTS(0xF6);
					goto exit;
				}
				rc = DARRegRead(dev_info,
					CPS1848_PORT_X_LOCAL_ACKID_CSR(pnum),
					&ackid_chk);
				if (RIO_SUCCESS != rc) {
					*fail_pt = EM_CLR_EVENTS(0xF7);
					goto exit;
				}
			} while (ackid_chk && limit--);
			if (ackid_chk) {
				rc = 0x1004;
				*fail_pt = EM_CLR_EVENTS(0xF8);
				goto exit;
			}
		}


		if (orig[pnum].imp_err_det != info[pnum].imp_err_det) {
			rc = DARRegWrite(dev_info,
					CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
					info[pnum].imp_err_det);
			if (RIO_SUCCESS != rc) {
				*fail_pt = EM_CLR_EVENTS(0xFA);
				goto exit;
			}
		}

		if (orig[pnum].err_det != info[pnum].err_det) {
			rc = DARRegWrite(dev_info,
					CPS1848_PORT_X_ERR_DET_CSR(pnum),
					info[pnum].err_det);
			if (RIO_SUCCESS != rc) {
				*fail_pt = EM_CLR_EVENTS(0xF9);
				goto exit;
			}
		}
	}

exit:
	return rc;
}

#define EM_GET_INT_STAT(x) (EM_GET_INT_STAT_0+x)
#define EM_EM_GET_PW_STAT(x) (EM_EM_GET_PW_STAT_0+x)
#define CREATE_RATE(x) (EM_CREATE_RATE_0+x)

static uint32_t cps_create_rate_event(DAR_DEV_INFO_t *dev_info, uint8_t pnum,
		uint32_t err_det_val, uint32_t err_det_addr,
		uint32_t rate_en_addr, uint32_t *imp_err)
{
	uint32_t rc;
	uint32_t regVal, limit, idx;

	rc = DARRegRead(dev_info, rate_en_addr, &regVal);
	if (RIO_SUCCESS != rc) {
		*imp_err = CREATE_RATE(0x80);
		goto exit;
	}

	// If requested event is not enabled for counting, can't create the event.
	if (!(err_det_val & regVal)) {
		rc = RIO_ERR_NOT_SUP_BY_CONFIG;
		*imp_err = CREATE_RATE(0x81);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_RATE_THRESH_CSR(pnum),
			&regVal);
	if (RIO_SUCCESS != rc) {
		*imp_err = CREATE_RATE(0x82);
		goto exit;
	}

	limit = (regVal & CPS1848_PORT_X_ERR_RATE_THRESH_CSR_FAIL_THRESH) >> 24;
	// If there is no error threshold set, cannot create event.
	if (!limit) {
		rc = RIO_ERR_NOT_SUP_BY_CONFIG;
		*imp_err = CREATE_RATE(0x83);
		goto exit;
	}

	// Create the event.  Preserve other events in the error detect register.
	// Note that this may fail if a "fast" leak rate is set.
	// Note that this may be overkill if multiple events occur for each write.
	rc = DARRegRead(dev_info, err_det_addr, &regVal);
	if (RIO_SUCCESS != rc) {
		*imp_err = CREATE_RATE(0x84);
		goto exit;
	}

	for (idx = 0; idx < limit; idx++) {
		rc = DARRegWrite(dev_info, err_det_addr, regVal | err_det_val);
		if (RIO_SUCCESS != rc) {
			*imp_err = CREATE_RATE(0x85);
			goto exit;
		}
	}

exit:
	return rc;

}

static uint32_t cps_rio_em_create_events_rio_em_f_los(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_out_t *out_parms, uint32_t pnum,
		rio_pc_ls_t ls, cps_port_info_t *pi)
{
	uint8_t quadrant;
	uint8_t quad_cfg;
	uint32_t rc;

	// Test both sources of LOS, the per port LOA bit and the
	// per lane Sync/Ready bits.
	// Assume that the LOA bit is reliable at 3.125 Gbaud and below,
	// and that the Sync/Ready bits are required at 5.0 Gbaud and above
	// for testing purposes only...
	if (ls <= rio_pc_ls_3p125) {
		rc = cps_create_rate_event(dev_info, pnum,
		CPS1848_PORT_X_IMPL_SPEC_ERR_DET_LOA,
				CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
				CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(pnum),
				&out_parms->imp_rc);
	} else {
		quadrant = pi->cpr[pnum].quadrant;
		quad_cfg = pi->quad_cfg_val[quadrant];
		rc =
				cps_create_rate_event(dev_info, pnum,
						(CPS1848_LANE_X_ERR_DET_LANE_SYNC
								| CPS1848_LANE_X_ERR_DET_LANE_RDY),
						CPS1848_LANE_X_ERR_DET(
								pi->cpr[pnum].cfg[quad_cfg].first_lane),
						CPS1848_LANE_X_ERR_RATE_EN(
								pi->cpr[pnum].cfg[quad_cfg].first_lane),
						&out_parms->imp_rc);
	}

	return rc;
}

static uint32_t cps_rio_em_create_events_rio_em_f_port_err(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_out_t *out_parms, uint32_t pnum, rio_pc_ls_t ls)
{
	uint32_t temp;
	uint32_t ackID;
	uint32_t imp_spec_det;
	uint32_t cmd = RIO_SPX_LM_REQ_CMD_LR_IS;
	uint32_t rc;

	// Check that port_err detection is enabled.
	rc = DARRegRead(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(pnum),
			&temp);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x11);
		goto exit;
	}

	if (!(temp & CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_FATAL_TO_EN)) {
		rc = RIO_ERR_NOT_SUP_BY_CONFIG;
		out_parms->imp_rc = EM_CREATE_EVENTS(0x12);
		goto exit;
	}

	// Invert the next expected ackID, then issue a link request...
	rc = DARRegRead(dev_info, CPS1848_PORT_X_LOCAL_ACKID_CSR(pnum), &ackID);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x13);
		goto exit;
	}
	ackID = ackID ^ (CPS1848_PORT_X_LOCAL_ACKID_CSR_OUTBOUND | CPS1848_PORT_X_LOCAL_ACKID_CSR_OUTSTD);
	rc = DARRegWrite(dev_info, CPS1848_PORT_X_LOCAL_ACKID_CSR(pnum), ackID);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x14);
		goto exit;
	}
	// The above access sets the "Set an Illegal Ackid" bit, so we
	// must clear this bit to avoid bad behavior caused by the
	// initialization hook.
	rc = DARRegRead(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
			&imp_spec_det);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x15);
		goto exit;
	}

	imp_spec_det &= ~CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_SET_ACKID_EN;
	rc = DARRegWrite(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
			imp_spec_det);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x16);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_PORT_X_LINK_MAINT_REQ_CSR(pnum),
			cmd);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x17);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_LINK_MAINT_RESP_CSR(pnum),
			&cmd);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x18);
		goto exit;
	}

	// Create the fake timeout/link error indication
	if (ls <= rio_pc_ls_3p125) {
		rc = DARRegWrite(dev_info, CPS1848_PORT_X_ERR_DET_CSR(pnum),
		CPS1848_PORT_X_ERR_DET_CSR_LR_ACKID_ILL);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_CREATE_EVENTS(0x19);
		}
	} else {
		rc = DARRegWrite(dev_info,
				CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
				CPS1848_PORT_X_IMPL_SPEC_ERR_DET_FATAL_TO);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_CREATE_EVENTS(0x1A);
		}
	}

exit:
	return rc;
}

static uint32_t cps_rio_em_create_events_rio_em_f_2many_retx(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_out_t *out_parms, uint32_t pnum)
{
	uint32_t err_rpt;
	uint32_t impl_err_det;
	uint32_t rc;

	// No need to create a rate event - just write the register...
	rc = DARRegRead(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(pnum),
			&err_rpt);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x21);
		goto exit;
	}

	if (!(err_rpt & CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_MANY_RETRY_EN)) {
		rc = RIO_ERR_NOT_SUP_BY_CONFIG;
		out_parms->imp_rc = EM_CREATE_EVENTS(0x22);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
			&impl_err_det);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x23);
		goto exit;
	}

	impl_err_det |= CPS1848_PORT_X_IMPL_SPEC_ERR_DET_MANY_RETRY;
	rc = DARRegWrite(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
			impl_err_det);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x24);
	}

exit:
	return rc;
}

static uint32_t cps_rio_em_create_events_rio_em_d_ttl(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_out_t *out_parms, uint32_t pnum)
{
	uint32_t temp;
	uint32_t rc;

	rc = DARRegRead(dev_info, CPS1848_PKT_TTL_CSR, &temp);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x30);
		goto exit;
	}

	if (!temp) {
		rc = RIO_ERR_NOT_SUP_BY_CONFIG;
		out_parms->imp_rc = EM_CREATE_EVENTS(0x31);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
	CPS1848_PORT_X_IMPL_SPEC_ERR_DET_TTL_EVENT);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x32);
	}

exit:
	return rc;
}

static uint32_t cps_rio_em_create_events_rio_em_d_rte(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_out_t *out_parms, uint32_t pnum)
{
	uint32_t temp;
	uint32_t rc;

	rc = DARRegRead(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(pnum),
			&temp);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x40);
		goto exit;
	}

	if (!(temp & CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_RTE_ISSUE_EN)) {
		rc = RIO_ERR_NOT_SUP_BY_CONFIG;
		out_parms->imp_rc = EM_CREATE_EVENTS(0x41);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
	CPS1848_PORT_X_IMPL_SPEC_ERR_DET_RTE_ISSUE);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x42);
	}

exit:
	return rc;
}

static uint32_t cps_rio_em_create_events_rio_em_d_log(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_out_t *out_parms)
{
	uint32_t temp;
	uint32_t rc;

	rc = DARRegRead(dev_info, CPS1848_LT_ERR_EN_CSR, &temp);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x50);
		goto exit;
	}

	if (!temp) {
		rc = RIO_ERR_NOT_SUP_BY_CONFIG;
		out_parms->imp_rc = EM_CREATE_EVENTS(0x51);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_LT_ERR_DET_CSR, CPSGEN2_ALL_LOG);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x52);
	}

exit:
	return rc;
}

static uint32_t cps_rio_em_create_events_rio_em_i_sig_det(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_out_t *out_parms, uint32_t pnum)
{
	uint32_t temp;
	uint32_t rc;

	rc = DARRegRead(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(pnum),
			&temp);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x61);
		goto exit;
	}

	if (!(temp & CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_PORT_INIT_EN)) {
		rc = RIO_ERR_NOT_SUP_BY_CONFIG;
		out_parms->imp_rc = EM_CREATE_EVENTS(0x62);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
	CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PORT_INIT);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x63);
	}

exit:
	return rc;
}

static uint32_t cps_rio_em_create_events_rio_em_i_init_fail(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_out_t *out_parms)
{
	uint32_t temp;
	uint32_t rc;

	rc = DARRegRead(dev_info, CPS1848_I2C_MASTER_CTL, &temp);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x71);
		goto exit;
	}

	if (temp & CPS1848_I2C_MASTER_CTL_CHKSUM_DIS) {
		rc = RIO_ERR_NOT_SUP_BY_CONFIG;
		out_parms->imp_rc = EM_CREATE_EVENTS(0x72);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_AUX_PORT_ERR_DET,
			CPS1848_AUX_PORT_ERR_DET_I2C_CHKSUM_ERR);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x73);
	}

exit:
	return rc;
}

// Error Management
//
uint32_t CPS_rio_em_cfg_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_pw_in_t *in_parms, rio_em_cfg_pw_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t pw_retx;
	uint32_t pw_tgt_devid;
	uint32_t pw_ctl = 0;

	if (in_parms->priority > 3) {
		out_parms->imp_rc = EM_CFG_PW(0x01);
		goto exit;
	}

	out_parms->imp_rc = RIO_SUCCESS;

	// First, set destination ID for port-writes
	if (tt_dev8 == in_parms->deviceID_tt) {
		pw_tgt_devid = ((uint32_t)(in_parms->port_write_destID) << 16)
				& CPS1848_PW_TARGET_DEVICEID_CSR_DESTID;
		if (in_parms->srcID_valid) {
			pw_ctl = ((uint32_t)(in_parms->port_write_srcID << 16))
					& CPS1848_PW_CTL_SRCID;
		}
	} else {
		pw_tgt_devid = ((uint32_t)(in_parms->port_write_destID) << 16)
				& (CPS1848_PW_TARGET_DEVICEID_CSR_DESTID |
						CPS1848_PW_TARGET_DEVICEID_CSR_DESTID_MSB);
		pw_tgt_devid |= CPS1848_PW_TARGET_DEVICEID_CSR_LARGE;
		if (in_parms->srcID_valid) {
			pw_ctl = ((uint32_t)(in_parms->port_write_srcID) << 16)
					& (CPS1848_PW_CTL_SRCID |
							CPS1848_PW_CTL_SRCID_MSB);
		}
	}

	pw_ctl |= ((did_reg_t)(in_parms->priority) << 14) & CPS1848_PW_CTL_PRIO;
	if (in_parms->CRF) {
		pw_ctl |= CPS1848_PW_CTL_CRF;
	}

	if (in_parms->port_write_re_tx) {
		pw_retx = (in_parms->port_write_re_tx << 8)
				& CPS1616_DEVICE_PW_TIMEOUT_TIMEOUT;
	} else {
		pw_retx = CPS1616_DEVICE_PW_TIMEOUT_TIMEOUT_DIS;
	}

	rc = DARRegWrite(dev_info, CPS1848_PW_TARGET_DEVICEID_CSR,
			pw_tgt_devid);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(0x10);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1848_PW_CTL, pw_ctl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(0x11);
		goto exit;
	}

	rc = DARRegWrite(dev_info, CPS1616_DEVICE_PW_TIMEOUT, pw_retx);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(0x12);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PW_TARGET_DEVICEID_CSR,
			&pw_tgt_devid);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(0x13);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PW_CTL, &pw_ctl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(0x14);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1616_DEVICE_PW_TIMEOUT, &pw_retx);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(0x15);
		goto exit;
	}

	if (pw_tgt_devid & CPS1848_PW_TARGET_DEVICEID_CSR_LARGE) {
		out_parms->deviceID_tt = tt_dev16;
		out_parms->port_write_destID = (did_reg_t)((pw_tgt_devid
				& (CPS1848_PW_TARGET_DEVICEID_CSR_DESTID |
				CPS1848_PW_TARGET_DEVICEID_CSR_DESTID_MSB))
				>> 16);
		out_parms->port_write_srcID = (did_reg_t)((pw_ctl
				& (CPS1848_PW_CTL_SRCID |
				CPS1848_PW_CTL_SRCID_MSB)) >> 16);
	} else {
		out_parms->deviceID_tt = tt_dev8;
		out_parms->port_write_destID = (did_reg_t)((pw_tgt_devid
				& CPS1848_PW_TARGET_DEVICEID_CSR_DESTID) >> 16);
		out_parms->port_write_srcID = (did_reg_t)((pw_ctl
				& CPS1848_PW_CTL_SRCID) >> 16);
	}

	out_parms->srcID_valid = true;

	out_parms->priority = (uint8_t)((pw_ctl & CPS1848_PW_CTL_PRIO) >> 14);
	if (pw_ctl & CPS1848_PW_CTL_CRF) {
		out_parms->CRF = true;
	} else {
		out_parms->CRF = false;
	}

	if (CPS1616_DEVICE_PW_TIMEOUT_TIMEOUT_DIS == pw_retx) {
		out_parms->port_write_re_tx = 0;
	} else {
		out_parms->port_write_re_tx = (pw_retx
				& CPS1616_DEVICE_PW_TIMEOUT_TIMEOUT) >> 8;
	}

	rc = RIO_SUCCESS;

exit:
	return rc;
}

uint32_t CPS_rio_em_cfg_set(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_set_in_t *in_parms, rio_em_cfg_set_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint16_t idx;
	uint8_t e_idx, pnum;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	cps_event_cfg_reg_values_t regs;
	cps_port_info_t pi;
	struct DAR_ptl good_ptl;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->fail_port_num = RIO_ALL_PORTS;
	out_parms->fail_idx = rio_em_last;
	out_parms->notfn = rio_em_notfn_0delta;

	if ((in_parms->num_events > (uint8_t)(rio_em_last))
			|| ( NULL == in_parms->events)) {
		out_parms->imp_rc = EM_CFG_SET(0x11);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if ((RIO_SUCCESS != rc) || (good_ptl.num_ports > CPS_MAX_PORTS)) {
		out_parms->imp_rc = EM_CFG_SET(0x10);
		goto exit;
	}

	cfg_in.ptl = good_ptl;
	rc = CPS_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	if (!cfg_out.num_ports || (cfg_out.num_ports > CPS_MAX_PORTS)) {
		rc = RIO_ERR_NO_PORT_AVAIL;
		out_parms->imp_rc = cfg_out.imp_rc;
		if (!out_parms->imp_rc) {
			out_parms->imp_rc = EM_CFG_SET(0x8F);
		}
		goto exit;
	}

	rc = init_sw_pi(dev_info, &pi);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_SET(0x90);
		goto exit;
	}

	for (idx = 0; idx < cfg_out.num_ports; idx++) {
		uint8_t quadrant, quad_cfg, first_lane, last_lane;
		if (cfg_out.pc[idx].port_available
				&& cfg_out.pc[idx].powered_up) {
			pnum = cfg_out.pc[idx].pnum;
			quadrant = pi.cpr[pnum].quadrant;
			quad_cfg = pi.quad_cfg_val[quadrant];
			first_lane = pi.cpr[pnum].cfg[quad_cfg].first_lane;
			last_lane = pi.cpr[pnum].cfg[quad_cfg].lane_count
					+ first_lane;

			out_parms->fail_port_num = pnum;
			out_parms->fail_idx = 0;

			rc = cps_get_event_cfg_reg_vals(dev_info, pnum, first_lane,
					last_lane, &regs, &out_parms->imp_rc);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}

			// Disable all events first, then enable events
			for (e_idx = 0;
					(e_idx < in_parms->num_events)
							&& (e_idx
									< (uint8_t)(rio_em_last));
					e_idx++) {
				out_parms->fail_idx = e_idx;
				out_parms->fail_port_num = pnum;
				// Failure points 0x20-0x80
				if (rio_em_detect_off
						== in_parms->events[e_idx].em_detect) {
					rc =
							cps_set_event_en_cfg(
									dev_info,
									pnum,
									first_lane,
									last_lane,
									&(in_parms->events[e_idx]),
									&regs,
									&out_parms->imp_rc);
					if (RIO_SUCCESS != rc) {
						goto exit;
					}
				}
			}

			// Disable all events first, then enable events
			for (e_idx = 0;
					(e_idx < in_parms->num_events)
							&& (e_idx
									< (uint8_t)(rio_em_last));
					e_idx++) {
				out_parms->fail_idx = e_idx;
				out_parms->fail_port_num = pnum;
				// Failure points 0x20-0x80
				if (rio_em_detect_on
						== in_parms->events[e_idx].em_detect) {
					rc =
							cps_set_event_en_cfg(
									dev_info,
									pnum,
									first_lane,
									last_lane,
									&(in_parms->events[e_idx]),
									&regs,
									&out_parms->imp_rc);
					if (RIO_SUCCESS != rc) {
						goto exit;
					}
				}
			}

			rc = cps_set_event_cfg_reg_vals(dev_info, pnum, first_lane,
					last_lane, &regs, &out_parms->imp_rc);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
		}
	}

	out_parms->fail_port_num = RIO_ALL_PORTS;
	out_parms->fail_idx = rio_em_last;

	rc = cps_set_int_cfg(dev_info, &good_ptl, in_parms->notfn, &pi,
			&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	rc = cps_set_pw_cfg(dev_info, &good_ptl, in_parms->notfn, &pi,
			&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	out_parms->notfn = in_parms->notfn;
	if ((out_parms->notfn > rio_em_notfn_both)
			&& (1 == good_ptl.num_ports)) {
		rc = cps_determine_notfn(dev_info, &out_parms->notfn,
				&good_ptl, &pi, &out_parms->imp_rc);
	}

exit:
	return rc;
}

uint32_t CPS_rio_em_cfg_get(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_get_in_t *in_parms, rio_em_cfg_get_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	cps_port_info_t pi;
	uint32_t err_rpt, imp_err_rpt, imp_err_rate, lctl;
	uint8_t pnum, quadrant, quad_cfg, first_lane;
	uint8_t e_idx;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	uint32_t rate_en, rate, thresh;
	uint32_t log_en;
	uint32_t mast_ctl;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->fail_idx = rio_em_last;
	out_parms->notfn = rio_em_notfn_last;

	if (in_parms->port_num >= NUM_CPS_PORTS(dev_info)) {
		out_parms->imp_rc = EM_CFG_GET(0x01);
		goto exit;
	} else {
		cfg_in.ptl.num_ports = 1;
		cfg_in.ptl.pnums[0] = in_parms->port_num;
	}

	if ((in_parms->num_events > (uint8_t)(rio_em_last))
			|| (NULL == in_parms->event_list)
			|| (NULL == in_parms->events)) {
		out_parms->imp_rc = EM_CFG_GET(0x02);
		goto exit;
	}

	rc = init_sw_pi(dev_info, &pi);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_GET(0x03);
		goto exit;
	}

	rc = CPS_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto exit;
	}

	if (!cfg_out.num_ports) {
		rc = RIO_ERR_NO_PORT_AVAIL;
		out_parms->imp_rc = EM_CFG_GET(0x04);
		goto exit;
	}

	if (!cfg_out.pc[0].port_available || !cfg_out.pc[0].powered_up) {
		rc = RIO_ERR_NO_PORT_AVAIL;
		out_parms->imp_rc = EM_CFG_GET(0x05);
		goto exit;
	}

	pnum = in_parms->port_num;
	quadrant = pi.cpr[pnum].quadrant;
	quad_cfg = pi.quad_cfg_val[quadrant];
	first_lane = pi.cpr[pnum].cfg[quad_cfg].first_lane;

	rc = DARRegRead(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(pnum),
			&imp_err_rpt);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_GET(0x20);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(pnum),
			&imp_err_rate);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_GET(0x21);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_RPT_EN(pnum), &err_rpt);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_GET(0x22);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1616_LANE_X_ERR_RATE_EN(first_lane),
			&lctl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_GET(0x23);
		goto exit;
	}

	for (e_idx = 0;
			((e_idx < in_parms->num_events)
					&& (e_idx < (uint8_t)(rio_em_last)));
			e_idx++) {
		// Initialize event such that event is disabled.
		out_parms->fail_idx = e_idx;
		in_parms->events[e_idx].em_event = in_parms->event_list[e_idx];
		in_parms->events[e_idx].em_detect = rio_em_detect_off;
		in_parms->events[e_idx].em_info = 0;

		switch (in_parms->events[e_idx].em_event) {
		case rio_em_f_los:
			if (lctl & (CPS1616_LANE_X_ERR_RATE_EN_LANE_SYNC_EN |
			CPS1616_LANE_X_ERR_RATE_EN_LANE_RDY_EN)) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
				in_parms->events[e_idx].em_info = 1;
			}
			break;

		case rio_em_f_port_err: // CPS has indirect notification of a port error.
			if ((imp_err_rpt
					& CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_FATAL_TO_EN)
					|| (err_rpt
							& CPS1848_PORT_X_ERR_RPT_EN_LR_ACKID_ILL_EN)) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
				in_parms->events[e_idx].em_info = 1;
			}
			break;

		case rio_em_f_2many_retx:
			if (imp_err_rpt
					& CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_MANY_RETRY_EN) {
				uint32_t retry_lim;
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;

				rc = DARRegRead(dev_info,
						CPS1848_PORT_X_RETRY_CNTR(pnum),
						&retry_lim);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = EM_CFG_GET(0x31);
					goto exit;
				}
				in_parms->events[e_idx].em_info =
						(retry_lim
								& CPS1848_PORT_X_RETRY_CNTR_RETRY_LIM)
								>> 16;
			}
			break;

		case rio_em_f_2many_pna:
			if (imp_err_rate
					& CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_PNA_EN) {
				uint32_t thresh;

				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
				rc =
						DARRegRead(dev_info,
								CPS1848_PORT_X_ERR_RATE_THRESH_CSR(
										pnum),
								&thresh);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = EM_CFG_GET(0x44);
					goto exit;
				}
				in_parms->events[e_idx].em_info =
						(thresh
								& CPS1848_PORT_X_ERR_RATE_THRESH_CSR_FAIL_THRESH)
								>> 24;
			}
			break;

		case rio_em_f_err_rate:
			rc = DARRegRead(dev_info,
					CPS1848_PORT_X_ERR_RATE_EN_CSR(pnum),
					&rate_en);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CFG_GET(0x51);
				goto exit;
			}

			if (rate_en & ~CPSGEN2_ERR_RATE_EVENT_EXCLUSIONS) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
				rc =
						DARRegRead(dev_info,
								CPS1848_PORT_X_ERR_RATE_THRESH_CSR(
										pnum),
								&thresh);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = EM_CFG_GET(0x52);
					goto exit;
				}

				rc = DARRegRead(dev_info,
						CPS1848_PORT_X_ERR_RATE_CSR(
								pnum), &rate);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = EM_CFG_GET(0x53);
					goto exit;
				}

				rate_en &= ~CPSGEN2_ERR_RATE_EVENT_EXCLUSIONS;
				rc =
						rio_em_compute_f_err_rate_info(
								rate_en, rate,
								thresh,
								&in_parms->events[e_idx].em_info);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = EM_CFG_GET(0x54);
					goto exit;
				}
			}
			break;

		case rio_em_d_ttl:
			if (imp_err_rpt
					& CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_TTL_EVENT_EN) {
				uint32_t ttl;
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
				rc = DARRegRead(dev_info, CPS1848_PKT_TTL_CSR,
						&ttl);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = EM_CFG_GET(0x61);
					goto exit;
				}

				in_parms->events[e_idx].em_info =
						((ttl & CPS1848_PKT_TTL_CSR_TTL)
								>> 16) * 1600;
			}
			break;

		case rio_em_d_rte:
			if (imp_err_rpt
					& CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_RTE_ISSUE_EN) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
			}
			break;

		case rio_em_d_log:
			rc = DARRegRead(dev_info, CPS1848_LT_ERR_EN_CSR,
					&log_en);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CFG_GET(0x80);
				goto exit;
			}

			if (log_en & CPSGEN2_ALL_LOG) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
				in_parms->events[e_idx].em_info = log_en
						& CPSGEN2_ALL_LOG;
			}
			break;

		case rio_em_i_sig_det:
			if (err_rpt
					& CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_PORT_INIT_EN) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
			}
			break;

		case rio_em_a_no_event:  // Nothing to do
		case rio_em_a_clr_pwpnd:  // Nothing to do
		case rio_em_i_rst_req:
			break;  // Not supported...

		case rio_em_i_init_fail: // Notification for I2C Initialization failures
			rc = DARRegRead(dev_info, CPS1848_I2C_MASTER_CTL,
					&mast_ctl);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CFG_GET(0x90);
				goto exit;
			}

			if (!(mast_ctl & CPS1848_I2C_MASTER_CTL_CHKSUM_DIS)) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
			}
			break;

		default:
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = EM_CFG_GET(0xA0);
			goto exit;
		}
	}

	out_parms->fail_idx = (uint8_t)(rio_em_last);

	// Now figure out the current notification setting for this port.
	rc = cps_determine_notfn(dev_info, &out_parms->notfn, &cfg_in.ptl,
			&pi, &out_parms->imp_rc);

exit:
	return rc;
}


uint32_t CPS_rio_em_dev_rpt_ctl(DAR_DEV_INFO_t *dev_info,
		rio_em_dev_rpt_ctl_in_t *in_parms,
		rio_em_dev_rpt_ctl_out_t *out_parms)
{
	uint32_t rc;
	cps_port_info_t pi;
	struct DAR_ptl good_ptl;

	out_parms->notfn = rio_em_notfn_0delta;
	out_parms->imp_rc = RIO_SUCCESS;

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_DEV_RPT_CTL(1);
		goto exit;
	}

	rc = init_sw_pi(dev_info, &pi);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_DEV_RPT_CTL(2);
		goto exit;
	}

	rc = cps_set_int_cfg(dev_info, &good_ptl, in_parms->notfn, &pi,
			&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	rc = cps_set_pw_cfg(dev_info, &good_ptl, in_parms->notfn, &pi,
			&out_parms->imp_rc);
	if (RIO_SUCCESS == rc) {
		out_parms->notfn = in_parms->notfn;
	}

	out_parms->notfn = in_parms->notfn;
	if ((1 == good_ptl.num_ports)
			&& (rio_em_notfn_0delta == in_parms->notfn)) {
		rc = cps_determine_notfn(dev_info, &out_parms->notfn,
				&good_ptl, &pi, &out_parms->imp_rc);
	}

exit:
	return rc;
}

uint32_t CPS_rio_em_parse_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_parse_pw_in_t *in_parms,
		rio_em_parse_pw_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;

	out_parms->num_events = 0;
	out_parms->too_many = false;
	out_parms->other_events = false;

	if ((!in_parms->num_events)
			|| (in_parms->num_events > (uint8_t)(rio_em_last))
			|| !in_parms->events || (NULL == dev_info)) {
		out_parms->imp_rc = EM_PARSE_PW(1);
		goto exit;
	}

	rc = RIO_SUCCESS;
	out_parms->imp_rc = RIO_SUCCESS;

	// First, check the Component Tag to see if either of the most significant 2 bits are set.
	// If they are, treat this as a standard port-write.
	// If they are not, treat this as an "Event Log" port-write.

	if (0xC0000000 & in_parms->pw[RIO_EM_PW_COMP_TAG_IDX]) {
		cps_parse_std_pw(in_parms, out_parms);
	} else {
		cps_parse_el_pw(in_parms, out_parms);
	}

exit:
	return rc;
}

uint32_t CPS_rio_em_get_int_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_int_stat_in_t *in_parms,
		rio_em_get_int_stat_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t pnum;
	uint8_t idx;
	uint32_t dev_ctl1, i2c_err_det, i2c_capt_en, i2c_mast_ctl;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	uint32_t log_err_det, log_err_en;
	cps_port_info_t pi;
	bool got_los;
	uint32_t imp_spec_chkd = 0;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->num_events = 0;
	out_parms->too_many = false;
	out_parms->other_events = false;

	if ((!in_parms->num_events)
			|| (in_parms->num_events > EM_MAX_EVENT_LIST_SIZE)
			|| (NULL == in_parms->events)) {
		out_parms->imp_rc = EM_GET_INT_STAT(1);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &cfg_in.ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(2);
		goto exit;
	}

	rc = CPS_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto exit;
	}

	if (!cfg_out.num_ports) {
		rc = RIO_ERR_NO_PORT_AVAIL;
		out_parms->imp_rc = EM_GET_INT_STAT(4);
		goto exit;
	}

	rc = init_sw_pi(dev_info, &pi);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(5);
		goto exit;
	}

	// As the very first thing, check for logical layer errors
	rc = DARRegRead(dev_info, CPS1848_LT_ERR_DET_CSR, &log_err_det);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x10);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_LT_ERR_EN_CSR, &log_err_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x11);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_DEVICE_CTL_1, &dev_ctl1);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x12);
		goto exit;
	}

	if (log_err_det) {
		if (log_err_en) {
			rio_em_add_int_event(in_parms, out_parms, RIO_ALL_PORTS,
					rio_em_d_log);
		} else {
			out_parms->other_events = true;
		}
	}

	// As the very next thing, check for I2C errors
	rc = DARRegRead(dev_info, CPS1848_AUX_PORT_ERR_DET, &i2c_err_det);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x20);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_AUX_PORT_ERR_CAPT_EN, &i2c_capt_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x21);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_I2C_MASTER_CTL, &i2c_mast_ctl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x22);
		goto exit;
	}

	if (i2c_err_det & CPS1848_AUX_PORT_ERR_DET_I2C_CHKSUM_ERR) {
		if (i2c_capt_en & CPS1848_AUX_PORT_ERR_CAPT_EN_I2C_CHKSUM_ERR_EN) {
			rio_em_add_int_event(in_parms, out_parms, RIO_ALL_PORTS,
					rio_em_i_init_fail);
		} else {
			out_parms->other_events = true;
		}
	}

	for (idx = 0; idx < cfg_out.num_ports; idx++) {
		uint32_t err_n_stat, err_det, err_rate, imp_spec_det,
				err_rpt_en, err_rate_en;
		uint32_t l_err_det, l_err_rate, t_lane;
		uint8_t first_lane, last_lane, lnum;
		uint32_t ops;
		got_los = false;

		if (!(cfg_out.pc[idx].port_available
				&& cfg_out.pc[idx].powered_up)) {
			continue;
		}

		pnum = cfg_out.pc[idx].pnum;

		// Check that port is reporting events using interrupts
		rc = DARRegRead(dev_info, CPS1848_PORT_X_OPS(pnum), &ops);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_INT_STAT(0x30);
			goto exit;
		}

		// Read per-port registers for event status
		rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_STAT_CSR(pnum),
				&err_n_stat);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_INT_STAT(0x31);
			goto exit;
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_DET_CSR(pnum),
				&err_det);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_INT_STAT(0x32);
			goto exit;
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_RATE_EN_CSR(pnum),
				&err_rate);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_INT_STAT(0x33);
			goto exit;
		}

		rc = DARRegRead(dev_info,
				CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
				&imp_spec_det);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_INT_STAT(0x34);
			goto exit;
		}

		rc = DARRegRead(dev_info,
				CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(pnum),
				&err_rpt_en);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_INT_STAT(0x35);
			goto exit;
		}

		rc = DARRegRead(dev_info,
				CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(pnum),
				&err_rate_en);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_INT_STAT(0x36);
			goto exit;
		}

		// Lane status is the "or" of status of each lane.
		first_lane =
				pi.cpr[pnum].cfg[pi.quad_cfg_val[pi.cpr[pnum].quadrant]].first_lane;
		last_lane =
				pi.cpr[pnum].cfg[pi.quad_cfg_val[pi.cpr[pnum].quadrant]].lane_count
						+ first_lane;
		l_err_det = 0;

		for (lnum = first_lane; lnum < last_lane; lnum++) {
			rc = DARRegRead(dev_info, CPS1848_LANE_X_ERR_DET(lnum),
					&t_lane);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_GET_PW_STAT(0x36);
				goto exit;
			}
			l_err_det |= t_lane;
		}

		rc =
				DARRegRead(dev_info,
						CPS1848_LANE_X_ERR_RATE_EN(
								pi.cpr[pnum].cfg[pi.quad_cfg_val[pi.cpr[pnum].quadrant]].first_lane),
						&l_err_rate);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_INT_STAT(0x38);
			goto exit;
		}

		imp_spec_chkd |= CPS1848_PORT_X_IMPL_SPEC_ERR_DET_ERR_RATE
				| CPS1848_PORT_X_IMPL_SPEC_ERR_DET_LOA |
				CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PNA;

		// Check on events relate to PORT_FAIL conditions.
		if (CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_FAIL & err_n_stat) {
			// Check for Loss of Signal...
			if (((CPS1848_LANE_X_ERR_DET_LANE_SYNC
					| CPS1848_LANE_X_ERR_DET_LANE_RDY)
					& l_err_det)
					|| (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_LOA
							& imp_spec_det)) {
				// Note, only need to check l_err_rate, as err_rpt_en is always set when l_err_rate is set.
				if ((CPS1848_LANE_X_ERR_RATE_EN_LANE_SYNC_EN
						| CPS1848_LANE_X_ERR_RATE_EN_LANE_RDY_EN)
						& l_err_rate) {
					rio_em_add_int_event(in_parms, out_parms,
							pnum, rio_em_f_los);
					got_los = true;
				} else {
					out_parms->other_events = true;
				}
			}

			// Check for Error Rate
			if (~CPSGEN2_ERR_RATE_EVENT_EXCLUSIONS & err_det) {
				if (~CPSGEN2_ERR_RATE_EVENT_EXCLUSIONS
						& err_rate) {
					rio_em_add_int_event(in_parms, out_parms,
							pnum,
							rio_em_f_err_rate);
				} else {
					out_parms->other_events = true;
				}
			}

			// Check for "too many PNA"
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PNA
					& imp_spec_det) {
				if (CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_PNA_EN
						& err_rate_en) {
					rio_em_add_int_event(in_parms, out_parms,
							pnum,
							rio_em_f_2many_pna);
				} else {
					out_parms->other_events = true;
				}
			}
		}

		if (!got_los
				&& (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_LOA
						& imp_spec_det)) {
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_LOA_EN
					& err_rate_en) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_f_los);
			} else {
				out_parms->other_events = true;
			}
		}

		// Check for PORT_ERR
		imp_spec_chkd |= CPS1848_PORT_X_IMPL_SPEC_ERR_DET_FATAL_TO;
		if ((CPS1848_PORT_X_IMPL_SPEC_ERR_DET_FATAL_TO & imp_spec_det)
				|| (CPS1848_PORT_X_ERR_DET_CSR_LR_ACKID_ILL
						& err_det)
				|| (CPS1848_PORT_X_ERR_STAT_CSR_PORT_ERR
						& err_n_stat)) {
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_FATAL_TO_EN
					& err_rpt_en) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_f_port_err);
			} else {
				out_parms->other_events = true;
			}
		}

		// Too many retries
		imp_spec_chkd |= CPS1848_PORT_X_IMPL_SPEC_ERR_DET_MANY_RETRY;
		if (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_MANY_RETRY
				& imp_spec_det) {
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_MANY_RETRY_EN
					& err_rpt_en) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_f_2many_retx);
			} else {
				out_parms->other_events = true;
			}
		}

		// Time to live
		imp_spec_chkd |= CPS1848_PORT_X_IMPL_SPEC_ERR_DET_TTL_EVENT;
		if (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_TTL_EVENT & imp_spec_det) {
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_TTL_EVENT_EN
					& err_rpt_en) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_d_ttl);
			} else {
				out_parms->other_events = true;
			}
		}

		// Routing Error
		imp_spec_chkd |= CPS1848_PORT_X_IMPL_SPEC_ERR_DET_RTE_ISSUE;
		if (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_RTE_ISSUE & imp_spec_det) {
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_RTE_ISSUE_EN
					& err_rpt_en) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_d_rte);
			} else {
				out_parms->other_events = true;
			}
		}

		// Signal Detected...
		imp_spec_chkd |= CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PORT_INIT;
		if (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PORT_INIT & imp_spec_det) {
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_PORT_INIT_EN
					& err_rpt_en) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_i_sig_det);
			} else {
				out_parms->other_events = true;
			}
		}

		// Check for other events, not handled by this code,
		// which are enabled for reporting or counting.
		if ((~imp_spec_chkd & imp_spec_det & err_rpt_en)
				|| (~imp_spec_chkd & imp_spec_det & err_rate_en)) {
			out_parms->other_events = true;
		}
	}

	rc = RIO_SUCCESS;

exit:
	return rc;
}

uint32_t CPS_rio_em_get_pw_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_pw_stat_in_t *in_parms,
		rio_em_get_pw_stat_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t pnum;
	uint8_t idx;
	uint32_t start_idx;
	uint32_t dev_ctl1, i2c_err_det, i2c_capt_en, i2c_mast_ctl;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	uint32_t log_err_det, log_err_en;
	bool got_los;
	uint32_t imp_spec_chkd = 0;
	cps_port_info_t pi;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->num_events = 0;
	out_parms->too_many = false;
	out_parms->other_events = false;

	if ((!in_parms->num_events)
			|| (in_parms->num_events > EM_MAX_EVENT_LIST_SIZE)
			|| (NULL == in_parms->events)) {
		out_parms->imp_rc = EM_GET_PW_STAT(1);
		goto exit;
	}

	if ((RIO_ALL_PORTS != in_parms->pw_port_num)
			&& (NUM_CPS_PORTS(dev_info) <= in_parms->pw_port_num)) {
		out_parms->imp_rc = EM_GET_PW_STAT(3);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &cfg_in.ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(2);
		goto exit;
	}

	rc = CPS_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto exit;
	}

	if (!cfg_out.num_ports) {
		rc = RIO_ERR_NO_PORT_AVAIL;
		out_parms->imp_rc = EM_GET_PW_STAT(4);
		goto exit;
	}

	rc = init_sw_pi(dev_info, &pi);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(5);
		goto exit;
	}

	// As the very first thing, check for logical layer errors
	rc = DARRegRead(dev_info, CPS1848_LT_ERR_DET_CSR, &log_err_det);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x10);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_LT_ERR_EN_CSR, &log_err_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x11);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_DEVICE_CTL_1, &dev_ctl1);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x12);
		goto exit;
	}

	if (log_err_det) {
		if (log_err_en) {
			rio_em_add_pw_event(in_parms, out_parms, RIO_ALL_PORTS,
					rio_em_d_log);
		} else {
			out_parms->other_events = true;
		}
	}

	// As the very next thing, check for I2C errors
	rc = DARRegRead(dev_info, CPS1848_AUX_PORT_ERR_DET, &i2c_err_det);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x20);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_AUX_PORT_ERR_CAPT_EN, &i2c_capt_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x21);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_I2C_MASTER_CTL, &i2c_mast_ctl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x22);
		goto exit;
	}

	if (i2c_err_det & CPS1848_AUX_PORT_ERR_DET_I2C_CHKSUM_ERR) {
		if (i2c_capt_en & CPS1848_AUX_PORT_ERR_CAPT_EN_I2C_CHKSUM_ERR_EN) {
			rio_em_add_pw_event(in_parms, out_parms, RIO_ALL_PORTS,
					rio_em_i_init_fail);
		} else {
			out_parms->other_events = true;
		}
	}

	for (idx = 0; idx < cfg_out.num_ports; idx++) {
		uint32_t err_n_stat, err_det, err_rate, imp_spec_det,
				err_rpt_en, err_rate_en;
		uint32_t l_err_det = 0, l_err_rate, t_lane;
		uint32_t ops;
		uint8_t first_lane, last_lane, lnum;
		got_los = false;

		if (!(cfg_out.pc[idx].port_available
				&& cfg_out.pc[idx].powered_up)) {
			continue;
		}

		start_idx = out_parms->num_events;
		pnum = cfg_out.pc[idx].pnum;

		// Check that port is reporting events using port-writes
		rc = DARRegRead(dev_info, CPS1848_PORT_X_OPS(pnum), &ops);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_PW_STAT(0x30);
			goto exit;
		}

		// Read per-port registers for event status
		rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_STAT_CSR(pnum),
				&err_n_stat);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_PW_STAT(0x31);
			goto exit;
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_DET_CSR(pnum),
				&err_det);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_PW_STAT(0x32);
			goto exit;
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_RATE_EN_CSR(pnum),
				&err_rate);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_PW_STAT(0x33);
			goto exit;
		}

		rc = DARRegRead(dev_info,
				CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
				&imp_spec_det);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_PW_STAT(0x34);
			goto exit;
		}

		rc = DARRegRead(dev_info,
				CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(pnum),
				&err_rpt_en);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_PW_STAT(0x35);
			goto exit;
		}

		rc = DARRegRead(dev_info,
				CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(pnum),
				&err_rate_en);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_PW_STAT(0x36);
			goto exit;
		}

		// Lane status is the "or" of status of each lane.
		first_lane =
				pi.cpr[pnum].cfg[pi.quad_cfg_val[pi.cpr[pnum].quadrant]].first_lane;
		last_lane =
				pi.cpr[pnum].cfg[pi.quad_cfg_val[pi.cpr[pnum].quadrant]].lane_count
						+ first_lane;
		l_err_det = 0;

		for (lnum = first_lane; lnum < last_lane; lnum++) {
			rc = DARRegRead(dev_info, CPS1848_LANE_X_ERR_DET(lnum),
					&t_lane);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_GET_PW_STAT(0x37);
				goto exit;
			}
			l_err_det |= t_lane;
		}

		rc =
				DARRegRead(dev_info,
						CPS1848_LANE_X_ERR_RATE_EN(
								pi.cpr[pnum].cfg[pi.quad_cfg_val[pi.cpr[pnum].quadrant]].first_lane),
						&l_err_rate);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_PW_STAT(0x38);
			goto exit;
		}

		// Check on events related to PORT_FAIL conditions.
		imp_spec_chkd |= CPS1848_PORT_X_IMPL_SPEC_ERR_DET_ERR_RATE
				| CPS1848_PORT_X_IMPL_SPEC_ERR_DET_LOA |
				CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PNA;
		if (CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_FAIL & err_n_stat) {
			// Check for Loss of Signal...
			if (((CPS1848_LANE_X_ERR_DET_LANE_SYNC
					| CPS1848_LANE_X_ERR_DET_LANE_RDY)
					& l_err_det)
					|| (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_LOA
							& imp_spec_det)) {
				// Note, only need to check l_err_rate, as err_rpt_en is always set when l_err_rate is set.
				if ((CPS1848_LANE_X_ERR_RATE_EN_LANE_SYNC_EN
						| CPS1848_LANE_X_ERR_RATE_EN_LANE_RDY_EN)
						& l_err_rate) {
					rio_em_add_pw_event(in_parms, out_parms,
							pnum, rio_em_f_los);
					got_los = true;
				} else {
					out_parms->other_events = true;
				}
			}

			// Check for Error Rate
			if (~CPSGEN2_ERR_RATE_EVENT_EXCLUSIONS & err_det) {
				if (~CPSGEN2_ERR_RATE_EVENT_EXCLUSIONS
						& err_rate) {
					rio_em_add_pw_event(in_parms, out_parms,
							pnum,
							rio_em_f_err_rate);
				} else {
					out_parms->other_events = true;
				}
			}

			// Check for "too many PNA"
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PNA
					& imp_spec_det) {
				if (CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_PNA_EN
						& err_rate_en) {
					rio_em_add_pw_event(in_parms, out_parms,
							pnum,
							rio_em_f_2many_pna);
				} else {
					out_parms->other_events = true;
				}
			}
		}

		if (!got_los
				&& (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_LOA
						& imp_spec_det)) {
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_LOA_EN
					& err_rate_en) {
				rio_em_add_pw_event(in_parms, out_parms, pnum,
						rio_em_f_los);
			} else {
				out_parms->other_events = true;
			}
		}

		// Check for PORT_ERR
		imp_spec_chkd |=
		CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_FATAL_TO_EN;
		if ((CPS1848_PORT_X_IMPL_SPEC_ERR_DET_FATAL_TO & imp_spec_det)
				|| (CPS1848_PORT_X_ERR_DET_CSR_LR_ACKID_ILL
						& err_det)
				|| (CPS1848_PORT_X_ERR_STAT_CSR_PORT_ERR
						& err_n_stat)) {
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_FATAL_TO_EN
					& err_rpt_en) {
				rio_em_add_pw_event(in_parms, out_parms, pnum,
						rio_em_f_port_err);
			} else {
				out_parms->other_events = true;
			}
		}

		// Too many retries
		imp_spec_chkd |= CPS1848_PORT_X_IMPL_SPEC_ERR_DET_MANY_RETRY;
		if (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_MANY_RETRY
				& imp_spec_det) {
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_MANY_RETRY_EN
					& err_rpt_en) {
				rio_em_add_pw_event(in_parms, out_parms, pnum,
						rio_em_f_2many_retx);
			} else {
				out_parms->other_events = true;
			}
		}

		// Time to live
		imp_spec_chkd |= CPS1848_PORT_X_IMPL_SPEC_ERR_DET_TTL_EVENT;
		if (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_TTL_EVENT & imp_spec_det) {
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_TTL_EVENT_EN
					& err_rpt_en) {
				rio_em_add_pw_event(in_parms, out_parms, pnum,
						rio_em_d_ttl);
			} else {
				out_parms->other_events = true;
			}
		}

		// Routing Error
		imp_spec_chkd |= CPS1848_PORT_X_IMPL_SPEC_ERR_DET_RTE_ISSUE;
		if (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_RTE_ISSUE & imp_spec_det) {
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_RTE_ISSUE_EN
					& err_rpt_en) {
				rio_em_add_pw_event(in_parms, out_parms, pnum,
						rio_em_d_rte);
			} else {
				out_parms->other_events = true;
			}
		}

		// Signal Detected...
		imp_spec_chkd |= CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PORT_INIT;
		if (CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PORT_INIT & imp_spec_det) {
			if (CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_PORT_INIT_EN
					& err_rpt_en) {
				rio_em_add_pw_event(in_parms, out_parms, pnum,
						rio_em_i_sig_det);
			} else {
				out_parms->other_events = true;
			}
		}

		// Check for other events, not handled by this code,
		// which are enabled for reporting or counting.
		if ((~imp_spec_chkd & imp_spec_det & err_rpt_en)
				|| (~imp_spec_chkd & imp_spec_det & err_rate_en)) {
			out_parms->other_events = true;
		}

		if ((start_idx != out_parms->num_events)
				|| (CPS1848_PORT_X_ERR_STAT_CSR_PW_PNDG
						& err_n_stat)) {
			rio_em_add_pw_event(in_parms, out_parms, pnum,
					rio_em_a_clr_pwpnd);
		}
	}
	rc = RIO_SUCCESS;

exit:
	return rc;
}

uint32_t CPS_rio_em_clr_events(DAR_DEV_INFO_t *dev_info,
		rio_em_clr_events_in_t *in_parms,
		rio_em_clr_events_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t temp;
	int pnum;
	int idx;
	uint8_t lnum, quadrant, quad_cfg;
	bool clear_port_fail = false;
	cps_port_info_t pi;
	clr_err_info_t regs[CPS_MAX_PORTS];
	clr_err_info_t orig[CPS_MAX_PORTS];

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->failure_idx = 0;
	out_parms->pw_events_remain = false;
	out_parms->int_events_remain = false;

	if ((!in_parms->num_events)
			|| (in_parms->num_events > EM_MAX_EVENT_LIST_SIZE)
			|| (NULL == in_parms->events)) {
		out_parms->imp_rc = EM_CLR_EVENTS(1);
		goto exit;
	}

	rc = init_sw_pi(dev_info, &pi);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CLR_EVENTS(1);
		goto exit;
	}

	rc = read_err_info(dev_info, &pi, orig, &out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	memcpy((void *)(regs), (void *)(orig),
			sizeof(clr_err_info_t) * CPS_MAX_PORTS);

	for (idx = 0; idx < in_parms->num_events; idx++) {
		bool glob_event;
		bool all_ports;
		out_parms->failure_idx = idx;
		pnum = in_parms->events[idx].port_num;
		// Complex statement below enforces
		// - valid port number values
		// - RIO_ALL_PORTS cannot be used with any other events.
		// - RIO_ALL_PORTS must be used with rio_em_d_log and rio_em_i_init_fail.
		// - valid event values

		all_ports = RIO_ALL_PORTS == pnum;
		if (((pnum >= NUM_CPS_PORTS(dev_info)) && !all_ports)
				|| (rio_em_last <= in_parms->events[idx].event)) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = EM_CLR_EVENTS(2);
			goto exit;
		}

		glob_event = (rio_em_d_log == in_parms->events[idx].event)
				|| (rio_em_i_init_fail
						== in_parms->events[idx].event);
		if ((all_ports && !glob_event) || (!all_ports && glob_event)) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = EM_CLR_EVENTS(3);
			goto exit;
		}

		switch (in_parms->events[idx].event) {
		case rio_em_f_los: // Clear each lanes events, as well as the per-port event.
			quadrant = pi.cpr[pnum].quadrant;
			quad_cfg = pi.quad_cfg_val[quadrant];
			for (lnum = pi.cpr[pnum].cfg[quad_cfg].first_lane;
				lnum < pi.cpr[pnum].cfg[quad_cfg].first_lane
					+ pi.cpr[pnum].cfg[quad_cfg].lane_count;
				lnum++) {
					rc = clr_bits(dev_info,
							CPS1848_LANE_X_ERR_DET(lnum),
							(CPS1848_LANE_X_ERR_DET_LANE_SYNC
							| CPS1848_LANE_X_ERR_DET_LANE_RDY),
							EM_CLR_EVENTS( 0x10),
							&out_parms->imp_rc);
				if (RIO_SUCCESS != rc) {
					goto exit;
				}
			}

			regs[pnum].imp_err_det &=
				~(CPS1848_PORT_X_IMPL_SPEC_ERR_DET_LOA
				| CPS1848_PORT_X_IMPL_SPEC_ERR_DET_ERR_RATE);
			clear_port_fail = true;
			break;

		case rio_em_f_port_err: // PORT_ERR occurs due to multiple causes,
					// all handled by CPS_rio_em_clr_events.
			regs[pnum].imp_err_det = 0;
			regs[pnum].err_det = 0;
			clear_port_fail = true;
			break;

		case rio_em_f_2many_retx:
			regs[pnum].imp_err_det &=
					~( CPS1848_PORT_X_IMPL_SPEC_ERR_DET_MANY_RETRY);

			rc = DARRegRead(dev_info,
					CPS1848_PORT_X_STATUS_AND_CTL(pnum),
					&temp);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x21);
				goto exit;
			}

			temp |= CPS1848_PORT_X_STATUS_AND_CTL_CLR_MANY_RETRY;
			rc = DARRegWrite(dev_info,
					CPS1848_PORT_X_STATUS_AND_CTL(pnum),
					temp);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x22);
				goto exit;
			}

			clear_port_fail = true;
			break;

		case rio_em_i_init_fail:
			rc = clr_bits(dev_info, CPS1848_AUX_PORT_ERR_DET,
			CPS1848_AUX_PORT_ERR_DET_I2C_CHKSUM_ERR,
					EM_CLR_EVENTS(0x30),
					&out_parms->imp_rc);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			break;

		case rio_em_d_ttl:
			regs[pnum].imp_err_det &=
					~( CPS1848_PORT_X_IMPL_SPEC_ERR_DET_TTL_EVENT);
			break;

		case rio_em_d_rte:
			regs[pnum].imp_err_det &=
					~( CPS1848_PORT_X_IMPL_SPEC_ERR_DET_RTE_ISSUE);
			break;

		case rio_em_f_2many_pna:
			regs[pnum].imp_err_det &=
					~( CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PNA);
			clear_port_fail = true;
			break;

		case rio_em_f_err_rate:
			// Clear all events that contribute to the fatal error rate event
			// This line is correct - we want to clear all bits that are not exclusions.
			regs[pnum].err_det &= CPSGEN2_ERR_RATE_EVENT_EXCLUSIONS;
			clear_port_fail = true;
			break;

		case rio_em_d_log: // Clear all logical layer errors, must write 0
			// Clear latched information
			rc = DARRegWrite(dev_info, CPS1848_LT_DEVICEID_CAPT_CSR,
					0);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x71);
				goto exit;
			}
			rc = DARRegWrite(dev_info, CPS1848_LT_CTL_CAPT_CSR, 0);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x72);
				goto exit;
			}
			rc = DARRegWrite(dev_info, CPS1848_LT_ERR_DET_CSR, 0);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x70);
				goto exit;
			}
			break;

		case rio_em_i_sig_det:
			regs[pnum].imp_err_det = 0;
			regs[pnum].err_det = 0;
			break;

		case rio_em_i_rst_req: // Nothing to do.
		case rio_em_a_no_event:
			break;

		case rio_em_a_clr_pwpnd:
			regs[pnum].clr_pwpnd = true;
			break;

		default:
			out_parms->imp_rc = EM_CLR_EVENTS(0x92);
			rc = RIO_ERR_INVALID_PARAMETER;
			goto exit;
			break;
		}

		if (clear_port_fail) {
			regs[pnum].imp_err_det = 0;
				// ~CPS1848_PORT_X_IMPL_SPEC_ERR_DET_ERR_RATE;
			regs[pnum].err_det = 0;
			regs[pnum].err_rate_csr &=
				(CPS1848_PORT_X_ERR_RATE_CSR_ERR_RATE_REC
				| CPS1848_PORT_X_ERR_RATE_CSR_ERR_RATE_BIAS);
			// Clear port-write pending indication last, after all
			// other events have been cleared.
			regs[pnum].err_stat_csr &=
					~CPS1848_PORT_X_ERR_STAT_CSR_PW_PNDG;
			regs[pnum].err_stat_csr |=
					CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_FAIL;
			regs[pnum].write_err_rate_n_stat = true;
			regs[pnum].clr_ackids = true;
		}
	}

	rc = write_err_info(dev_info, &pi, orig, regs, &out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	out_parms->failure_idx = EM_MAX_EVENT_LIST_SIZE;

exit:
	return rc;
}

uint32_t CPS_rio_em_create_events(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_in_t *in_parms,
		rio_em_create_events_out_t *out_parms)
{
	uint32_t rc;
	uint8_t pnum;
	uint8_t idx;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	cps_port_info_t pi;

	out_parms->failure_idx = 0;
	out_parms->imp_rc = RIO_SUCCESS;

	if ((!in_parms->num_events)
			|| (in_parms->num_events > EM_MAX_EVENT_LIST_SIZE)
			|| (NULL == in_parms->events)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = EM_CREATE_EVENTS(1);
		goto exit;
	}

	cfg_in.ptl.num_ports = RIO_ALL_PORTS;
	rc = CPS_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto exit;
	}

	if (!cfg_out.num_ports) {
		rc = RIO_ERR_NO_PORT_AVAIL;
		out_parms->imp_rc = EM_CREATE_EVENTS(2);
		goto exit;
	}

	rc = init_sw_pi(dev_info, &pi);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CREATE_EVENTS(3);
		goto exit;
	}

	for (idx = 0; idx < in_parms->num_events; idx++) {
		bool glob_event;
		bool all_ports;
		out_parms->failure_idx = idx;
		pnum = in_parms->events[idx].port_num;

		all_ports = RIO_ALL_PORTS == pnum;
		glob_event = (rio_em_d_log == in_parms->events[idx].event)
				|| (rio_em_i_init_fail
						== in_parms->events[idx].event);

		if (((pnum >= NUM_CPS_PORTS(dev_info)) && !all_ports)
				|| (rio_em_last <= in_parms->events[idx].event)) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = EM_CREATE_EVENTS(4);
			goto exit;
		}

		if ((glob_event && !all_ports) || (all_ports && !glob_event)) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = EM_CREATE_EVENTS(4);
			goto exit;
		}

		if (!all_ports
				&& (!(cfg_out.pc[pnum].port_available
						&& cfg_out.pc[pnum].powered_up))) {
			continue;
		}

		switch (in_parms->events[idx].event) {
		case rio_em_f_los:
			rc = cps_rio_em_create_events_rio_em_f_los(dev_info,
					out_parms, pnum, cfg_out.pc[pnum].ls,
					&pi);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			break;

		case rio_em_f_port_err:
			rc = cps_rio_em_create_events_rio_em_f_port_err(dev_info,
					out_parms, pnum, cfg_out.pc[pnum].ls);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			break;

		case rio_em_f_2many_retx:
			rc = cps_rio_em_create_events_rio_em_f_2many_retx(dev_info,
					out_parms, pnum);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			break;

		case rio_em_f_2many_pna:
			rc = cps_create_rate_event(dev_info, pnum,
			CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PNA,
					CPS1848_PORT_X_IMPL_SPEC_ERR_DET(pnum),
					CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(
							pnum),
					&out_parms->imp_rc);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			break;

		case rio_em_f_err_rate:
			rc = cps_create_rate_event(dev_info, pnum,
					~CPSGEN2_ERR_RATE_EVENT_EXCLUSIONS,
					CPS1848_PORT_X_ERR_DET_CSR(pnum),
					CPS1848_PORT_X_ERR_RATE_EN_CSR(pnum),
					&out_parms->imp_rc);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			break;

		case rio_em_d_ttl:
			rc = cps_rio_em_create_events_rio_em_d_ttl(dev_info,
					out_parms, pnum);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			break;

		case rio_em_d_rte:
			rc = cps_rio_em_create_events_rio_em_d_rte(dev_info,
					out_parms, pnum);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			break;

		case rio_em_d_log: // Set all logical layer errors
			rc = cps_rio_em_create_events_rio_em_d_log(dev_info,
					out_parms);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			break;

		case rio_em_i_sig_det:
			rc = cps_rio_em_create_events_rio_em_i_sig_det(dev_info,
					out_parms, pnum);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			break;

		case rio_em_i_rst_req:
			rc = RIO_ERR_FEATURE_NOT_SUPPORTED;
			out_parms->imp_rc = EM_CREATE_EVENTS(0x68);
			goto exit;
			break;

		case rio_em_i_init_fail:
			rc = cps_rio_em_create_events_rio_em_i_init_fail(dev_info,
					out_parms);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			break;

		default:
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = EM_CREATE_EVENTS(0x80);
			goto exit;
			break;
		}
	}

	out_parms->failure_idx = EM_MAX_EVENT_LIST_SIZE;

exit:
	return rc;
}

#endif /* CPS_DAR_WANTED */

#ifdef __cplusplus
}
#endif
