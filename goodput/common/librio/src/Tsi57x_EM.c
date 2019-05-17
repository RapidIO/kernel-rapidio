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
#include <stdbool.h>

#include "rio_standard.h"
#include "RapidIO_Device_Access_Routines_API.h"
#include "RapidIO_Error_Management_API.h"
#include "DSF_DB_Private.h"
#include "Tsi57x_DeviceDriver.h"
#include "Tsi578.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TSI57X_DAR_WANTED

#define EM_SET_EVENT_PW_0     (EM_FIRST_SUBROUTINE_0+0x2900) // 202900
#define EM_SET_EVENT_INT_0    (EM_FIRST_SUBROUTINE_0+0x2A00) // 202A00
#define EM_EN_ERR_CTR_0       (EM_FIRST_SUBROUTINE_0+0x2B00) // 202B00
#define EM_SET_EVENT_EN_0     (EM_FIRST_SUBROUTINE_0+0x2C00) // 202C00
#define EM_DET_NOTFN_0        (EM_FIRST_SUBROUTINE_0+0x2E00) // 202E00
#define EM_CREATE_RATE_0      (EM_FIRST_SUBROUTINE_0+0x2F00) // 202F00

#define TSI57X_ALL_LOG_ERRS ((uint32_t)(TSI578_RIO_LOG_ERR_DET_EN_UNSUP_TRANS_EN | \
		TSI578_RIO_LOG_ERR_DET_EN_ILL_RESP_EN | \
		TSI578_RIO_LOG_ERR_DET_EN_ILL_TRANS_EN))

#define NOT_IN_PW (0x0)
#define PW_IMP_SPEC_OTH (~(PW_MAX_RETRY | PW_ILL_TRANS | PW_PORT_ERR | \
		PW_LINK_INIT) & 0xFFFFFF00)

#define PW_MAX_RETRY    0x80000000
#define PW_ILL_TRANS    0x20000000
#define PW_PORT_ERR     0x10000000
#define PW_LINK_INIT    0x00200000

#define DET_NOTFN(x) (EM_DET_NOTFN_0+x)
#define EN_ERR_CTR(x) (EM_EN_ERR_CTR_0+x)
#define EM_CFG_SET(x) (EM_CFG_SET_0+x)

#define SET_EVENT_INT(x) (EM_SET_EVENT_INT_0+x)
#define SET_EVENT_EN(x) (EM_SET_EVENT_EN_0+x)
#define SET_EVENT_PW(x) (EM_SET_EVENT_PW_0+x)

#define GET_INT_STAT(x) (EM_GET_INT_STAT_0+x)
#define GET_PW_STAT(x) (EM_GET_PW_STAT_0+x)

#define CREATE_RATE(x) (EM_CREATE_RATE_0+x)
#define CREATE_EVENTS(x) (EM_CREATE_EVENTS_0+x)

#define EM_CFG_GET(x) (EM_CFG_GET_0+x)
#define EM_CFG_PW(x) (EM_CFG_PW_0+x)

#define PARSE_PW(x) (EM_PARSE_PW_0+x)

#define DEV_RPT_CTL(x) (EM_DEV_RPT_CTL_0+x)


#define EM_ERR_RATE_EVENT_EXCLUSIONS (TSI578_SPX_ERR_DET_DELIN_ERR|TSI578_SPX_ERR_DET_CS_NOT_ACC|TSI578_SPX_ERR_DET_IMP_SPEC_ERR)

typedef struct pw_parsing_info_t_TAG {
	// Index of 32 bit word to test in the port write, 0-3
	uint8_t pw_index;
	// If the bitwise and of this value and the port-write
	//    word at "pw_index" is non-zero, then the event has occurred
	uint32_t check;
	// Event checked for.
	rio_em_events_t event;
	// true if this is a per-port event, false for GLOBAL events
	bool per_port;
} pw_parsing_info_t;

pw_parsing_info_t pw_parsing_info[ (uint8_t)(rio_em_last) ] = {
	{ 1, TSI578_SPX_ERR_DET_DELIN_ERR, rio_em_f_los       , true },
	{ 2, PW_PORT_ERR                 , rio_em_f_port_err  , true },
	{ 2, PW_MAX_RETRY                , rio_em_f_2many_retx, true },
	{ 2, NOT_IN_PW                   , rio_em_i_init_fail , false},
	{ 2, NOT_IN_PW                   , rio_em_d_ttl       , false},
	{ 2, PW_ILL_TRANS                , rio_em_d_rte       , true },
	{ 3, 0xFFFFFFFF                  , rio_em_d_log       , false},
	{ 2, PW_LINK_INIT                , rio_em_i_sig_det   , true },
	{ 2, NOT_IN_PW                   , rio_em_i_rst_req   , true }
};

// Tsi57x_PC
extern uint32_t tsi57x_init_sw_pmr(DAR_DEV_INFO_t *dev_info,
		port_mac_relations_t **sw_pmr);

static uint32_t tsi57x_set_pw_cfg(DAR_DEV_INFO_t *dev_info,
		struct DAR_ptl *good_ptl, /* PTL has already been error-checked, just use it */
		rio_em_notfn_ctl_t notfn, uint32_t *imp_rc)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t regData;
	uint8_t port_idx, port_num;

	// Validate notfn and pnum
	if (rio_em_notfn_last <= notfn) {
		*imp_rc = SET_EVENT_PW(1);
		goto exit;
	}

	// Check if something must be done.
	if (rio_em_notfn_0delta == notfn) {
		rc = RIO_SUCCESS;
		goto exit;
	}

	for (port_idx = 0; port_idx <= good_ptl->num_ports; port_idx++) {
		port_num = good_ptl->pnums[port_idx];
		rc = DARRegRead(dev_info, TSI578_SPX_MODE(port_num), &regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_PW(2);
			goto exit;
		}

		//@sonar:off - c:S3458
		switch (notfn) {
		default: // Default case will not be activated...
		case rio_em_notfn_none:
		case rio_em_notfn_int:
			// Disable port-write event notification
			regData |= TSI578_SPX_MODE_PW_DIS;
			break;
		case rio_em_notfn_pw:
		case rio_em_notfn_both:
			// Enable port-write event notification
			regData &= ~(TSI578_SPX_MODE_PW_DIS);
			break;
		}
		//@sonar:on

		rc = DARRegWrite(dev_info, TSI578_SPX_MODE(port_num), regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_PW(3);
			goto exit;
		}
	}

exit:
	return rc;
}

static uint32_t tsi57x_set_int_cfg(DAR_DEV_INFO_t *dev_info,
		struct DAR_ptl *good_ptl, /* PTL has already been error-checked, just use it */
		rio_em_notfn_ctl_t notfn, uint32_t *imp_rc)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t glob_int_en, spx_ctl_indep, spx_mode, i2c_int_en;
	uint8_t port_idx, port_num = 0;
	bool en_mecs_int = false, en_rcs_int = false;

	// Validate notfn
	if (rio_em_notfn_last <= notfn) {
		*imp_rc = SET_EVENT_INT(1);
		goto exit;
	}

	// Check if something must be done.
	if (rio_em_notfn_0delta == notfn) {
		rc = RIO_SUCCESS;
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI578_GLOB_INT_ENABLE, &glob_int_en);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_INT(3);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI578_I2C_INT_ENABLE, &i2c_int_en);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_INT(4);
		goto exit;
	}

	// Interrupt enable/disable is complicated by
	// - Reset interrupts: When interrupts are enabled/disabled, the status of
	//                     rio_em_i_rst_req must be known.  This cannot be determined
	//                     from the state of SPx_MODE.SELF_RST.  Instead, the reserved
	//                     SPx_CTL_INDEP[30] is used to indicate whether Reset interrupts were
	//                     previously enabled, and so should be re-enabled.
	// - MECS interrupts:  Should only be enabled on the Tsi577, as all other devices have an
	//                     errata against MECS.
	//                     MECS interrupts are controlled only at the top level, not at the port level.
	// - I2C interrupts:   I2C Interrupts should only be enabled when TSI578_I2C_INT_ENABLE_BL_FAIL
	//                     is set.
	for (port_idx = 0; port_idx <= good_ptl->num_ports; port_idx++) {
		port_num = good_ptl->pnums[port_idx];
		rc = DARRegRead(dev_info, TSI578_SPX_MODE(port_num), &spx_mode);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_INT(5);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_CTL_INDEP(port_num),
				&spx_ctl_indep);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_INT(6);
			goto exit;
		}

		//@sonar:off - c:S3458
		switch (notfn) {
		default: // Default case will not be activated...
		case rio_em_notfn_none:
		case rio_em_notfn_pw:
			// Disable interrupt event notification
			glob_int_en &= ~(1 << port_num);
			spx_mode &= ~(TSI578_SPX_MODE_RCS_INT_EN);
			spx_ctl_indep &= ~TSI578_SPX_CTL_INDEP_IRQ_EN;
			break;
		case rio_em_notfn_int:
		case rio_em_notfn_both:
			// Enable interrupt event notification
			glob_int_en |= 1 << port_num;
			if (spx_ctl_indep & TSI578_SPX_CTL_INDEP_RSVD1) {
				spx_mode |= TSI578_SPX_MODE_RCS_INT_EN;
				en_rcs_int = true;
			}

			spx_ctl_indep |= TSI578_SPX_CTL_INDEP_IRQ_EN;
			if (spx_mode & TSI578_SPX_MODE_MCS_INT_EN)
				en_mecs_int = true;
			break;
		}
		//@sonar:on

		rc = DARRegWrite(dev_info, TSI578_SPX_MODE(port_num), spx_mode);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_INT(6);
			goto exit;
		}

		rc = DARRegWrite(dev_info, TSI578_SPX_CTL_INDEP(port_num),
				spx_ctl_indep);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_INT(7);
			goto exit;
		}
	}

	//@sonar:off - c:S3458
	switch (notfn) {
	default: // Default case will not be activated...
	case rio_em_notfn_none:
	case rio_em_notfn_pw:
		// Disable interrupt event notification
		if (RIO_ALL_PORTS == port_num) {
			glob_int_en &= ~(TSI578_GLOB_INT_ENABLE_RCS_EN
					| TSI578_GLOB_INT_ENABLE_MCS_EN
					| TSI578_GLOB_INT_ENABLE_I2C_EN);
		}
		break;
	case rio_em_notfn_int:
	case rio_em_notfn_both:
		if (RIO_ALL_PORTS == port_num) {
			if (en_rcs_int) {
				glob_int_en |= TSI578_GLOB_INT_ENABLE_RCS_EN;
			} else {
				glob_int_en &= ~TSI578_GLOB_INT_ENABLE_RCS_EN;
			}

			if (en_mecs_int) {
				glob_int_en |= TSI578_GLOB_INT_ENABLE_MCS_EN;
			} else {
				glob_int_en &= ~TSI578_GLOB_INT_ENABLE_MCS_EN;
			}
		}

		if (i2c_int_en & TSI578_I2C_INT_ENABLE_BL_FAIL) {
			glob_int_en |= TSI578_GLOB_INT_ENABLE_I2C_EN;
		} else {
			glob_int_en &= ~TSI578_GLOB_INT_ENABLE_I2C_EN;
		}
		break;
	}
	//@sonar:on

	rc = DARRegWrite(dev_info, TSI578_GLOB_INT_ENABLE, glob_int_en);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_INT(8);
	}

exit:
	return rc;
}

static uint32_t tsi57x_em_determine_notfn(DAR_DEV_INFO_t *dev_info,
		rio_em_notfn_ctl_t *notfn, uint8_t pnum, uint32_t *imp_rc)
{
	uint32_t rc;
	uint32_t spx_ctl_indep, spx_mode;

	rc = DARRegRead(dev_info, TSI578_SPX_MODE(pnum), &spx_mode);
	if (RIO_SUCCESS != rc) {
		*imp_rc = DET_NOTFN(0x01);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI578_SPX_CTL_INDEP(pnum), &spx_ctl_indep);
	if (RIO_SUCCESS != rc) {
		*imp_rc = DET_NOTFN(0x02);
		goto exit;
	}

	// Now figure out the current notification setting for this port.
	if (spx_ctl_indep & TSI578_SPX_CTL_INDEP_IRQ_EN) {
		if (spx_mode & TSI578_SPX_MODE_PW_DIS) {
			*notfn = rio_em_notfn_int;
		} else {
			*notfn = rio_em_notfn_both;
		}
	} else {
		if (spx_mode & TSI578_SPX_MODE_PW_DIS) {
			*notfn = rio_em_notfn_none;
		} else {
			*notfn = rio_em_notfn_pw;
		}
	}
exit:
	return rc;
}

static uint32_t tsi57x_enable_err_ctr(DAR_DEV_INFO_t *dev_info,
		uint8_t pnum, uint32_t spx_rate_en, uint32_t spx_rate_en_mask,
		uint32_t spx_err_rate, uint32_t spx_err_thresh,
		uint32_t *imp_rc)
{
	uint32_t rc;
	uint32_t regData;

	// Enable event counting.
	rc = DARRegRead(dev_info, TSI578_SPX_RATE_EN(pnum), &regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = EN_ERR_CTR(1);
		goto exit;
	}

	regData &= ~spx_rate_en_mask;
	regData |= spx_rate_en;

	rc = DARRegWrite(dev_info, TSI578_SPX_RATE_EN(pnum), regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = EN_ERR_CTR(2);
		goto exit;
	}

	// Clear counter, and set thresholds...
	rc = DARRegWrite(dev_info, TSI578_SPX_ERR_RATE(pnum), spx_err_rate);
	if (RIO_SUCCESS != rc) {
		*imp_rc = EN_ERR_CTR(3);
		goto exit;
	}

	// Hook to clear the "Degraded" threshold value.
	// This enforces a programming model where the
	// Degraded threshold must be less than or equal to
	// the Failed threshold.
	//
	// Also has the effect of clearing the reset default
	// value for the Degraded threshold to 0.

	if (((spx_err_thresh & TSI578_SPX_ERR_THRESH_ERR_RDT) >> 16)
			> ((spx_err_thresh & TSI578_SPX_ERR_THRESH_ERR_RFT)
					>> 24))
		spx_err_thresh &= TSI578_SPX_ERR_THRESH_ERR_RFT;

	rc = DARRegWrite(dev_info, TSI578_SPX_ERR_THRESH(pnum), spx_err_thresh);
	if (RIO_SUCCESS != rc) {
		*imp_rc = EN_ERR_CTR(4);
		goto exit;
	}

	// Tsi577/Tsi578/Tsi576/Tsi572 devices all detect PORT_FAIL,
	// and only discard packets within the switch, contrary to the
	// spec which requires continuous discard until PORT_FAIL is cleared.
	// See the errata sheet for these devices.
	//
	// The Tsi578A device, which is otherwise identical to Tsi578,
	// does perform continuous packet discard for PORT_FAIL
	// BUT only when the port is not in an output error state - which makes
	// the fix useless in most scenarios.

	rc = DARRegRead(dev_info, TSI578_SPX_CTL(pnum), &regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = EN_ERR_CTR(5);
		goto exit;
	}

	regData |= TSI578_SPX_CTL_STOP_FAIL_EN | TSI578_SPX_CTL_DROP_EN;
	rc = DARRegWrite(dev_info, TSI578_SPX_CTL(pnum), regData);

exit:
	return rc;
}

static uint32_t tsi57x_set_event_en_cfg_rio_f_los(DAR_DEV_INFO_t *dev_info,
		uint8_t pnum, rio_em_cfg_t *event, uint32_t *imp_rc)
{
	uint32_t regData;
	uint32_t temp;
	port_mac_relations_t *sw_pmr;
	bool set_delin_thresh = false;
	uint32_t rc = RIO_SUCCESS;

	// LOS is the dead link timer
	// If a short period is required, then single Delineation errors also
	//    result in LOS.

	rc = tsi57x_init_sw_pmr(dev_info, &sw_pmr);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0x10);
		goto exit;
	}

	if (event->em_detect == rio_em_detect_on) {
		// First, set up DLT
		rc = DARRegRead(dev_info,
				TSI578_SMACX_DLOOP_CLK_SEL(
						sw_pmr[pnum].pwr_mac_num * 2),
				&regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x11);
			goto exit;
		}

		temp = event->em_info / 81920;
		if (!temp) {
			temp = 1;
			set_delin_thresh = true;
		}

		regData |= TSI578_SMACX_DLOOP_CLK_SEL_DLT_EN;
		regData &= ~TSI578_SMACX_DLOOP_CLK_SEL_DLT_THRESH;
		regData |=
				((temp << 16)
						& TSI578_SMACX_DLOOP_CLK_SEL_DLT_THRESH);

		rc = DARRegWrite(dev_info,
				TSI578_SMACX_DLOOP_CLK_SEL(
						sw_pmr[pnum].pwr_mac_num * 2),
				regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x12);
			goto exit;
		}

		if (set_delin_thresh) {
			// Set the delineation error threshold to 1.
			// Configure the device to discard packets when the threshold has been met.
			// NOTE: This is only effective on the Tsi578A device. See errata.
			//
			uint32_t spx_err_rate_en, spx_err_rate, spx_err_thresh;
			rc = DARRegRead(dev_info, TSI578_SPX_RATE_EN(pnum),
					&spx_err_rate_en);
			if (RIO_SUCCESS != rc) {
				*imp_rc = SET_EVENT_EN(0x13);
				goto exit;
			}

			rc = DARRegRead(dev_info, TSI578_SPX_ERR_RATE(pnum),
					&spx_err_rate);
			if (RIO_SUCCESS != rc) {
				*imp_rc = SET_EVENT_EN(0x14);
				goto exit;
			}

			rc = DARRegRead(dev_info, TSI578_SPX_ERR_THRESH(pnum),
					&spx_err_thresh);
			if (RIO_SUCCESS != rc) {
				*imp_rc = SET_EVENT_EN(0x15);
				goto exit;
			}

			// Set the threshold to 1 only if no other threshold based events are enabled
			if (!(spx_err_rate_en & ~TSI578_SPX_ERR_DET_DELIN_ERR))
				spx_err_thresh = 0x01000000;

			rc = tsi57x_enable_err_ctr(dev_info, pnum,
			TSI578_SPX_ERR_DET_DELIN_ERR,
			TSI578_SPX_ERR_DET_DELIN_ERR, spx_err_rate,
					spx_err_thresh, imp_rc);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
		}
	} else { // rio_em_detect_off:
		rc = DARRegRead(dev_info,
				TSI578_SMACX_DLOOP_CLK_SEL(
						sw_pmr[pnum].pwr_mac_num * 2),
				&regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x16);
			goto exit;
		}

		regData &= ~(TSI578_SMACX_DLOOP_CLK_SEL_DLT_THRESH
				| TSI578_SMACX_DLOOP_CLK_SEL_DLT_EN);

		rc = DARRegWrite(dev_info,
				TSI578_SMACX_DLOOP_CLK_SEL(
						sw_pmr[pnum].pwr_mac_num * 2),
				regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x17);
			goto exit;
		}
	}

	if (!set_delin_thresh) {
		// Disable event counting.
		rc = DARRegRead(dev_info, TSI578_SPX_RATE_EN(pnum), &regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x18);
			goto exit;
		}

		regData &= ~TSI578_SPX_ERR_DET_DELIN_ERR;

		rc = DARRegWrite(dev_info, TSI578_SPX_RATE_EN(pnum), regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x19);
		}
	}

exit:
	return rc;
}

static uint32_t tsi57x_set_event_en_cfg_rio_em_f_port_err(DAR_DEV_INFO_t *dev_info,
		uint8_t pnum, rio_em_cfg_t *event, uint32_t *imp_rc)
{
	uint32_t regData;
	uint32_t rc = RIO_SUCCESS;

	rc = DARRegRead(dev_info, TSI578_SPX_CTL_INDEP(pnum), &regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0x20);
		goto exit;
	}

	if (event->em_detect == rio_em_detect_on) {
		uint32_t spx_err_rate, spx_err_thresh;
		rc = DARRegRead(dev_info, TSI578_SPX_ERR_RATE(pnum),
				&spx_err_rate);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x21);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_ERR_THRESH(pnum),
				&spx_err_thresh);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x22);
			goto exit;
		}

		if (!(spx_err_thresh & TSI578_SPX_ERR_THRESH_ERR_RFT)) {
			spx_err_thresh |= 0x01000000;
		}
		regData |= TSI578_SPX_CTL_INDEP_PORT_ERR_EN;

		rc = tsi57x_enable_err_ctr(dev_info, pnum,
		TSI578_SPX_ERR_DET_IMP_SPEC_ERR,
		TSI578_SPX_ERR_DET_IMP_SPEC_ERR, spx_err_rate, spx_err_thresh,
				imp_rc);
		if (RIO_SUCCESS != rc) {
			goto exit;
		}
	} else { // rio_em_detect_off:
		regData &= ~(TSI578_SPX_CTL_INDEP_PORT_ERR_EN);
	}

	rc = DARRegWrite(dev_info, TSI578_SPX_CTL_INDEP(pnum), regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0x23);
	}

exit:
	return rc;
}

static uint32_t tsi57x_set_event_en_cfg_rio_em_f_2many_retx(DAR_DEV_INFO_t *dev_info,
		uint8_t pnum, rio_em_cfg_t *event, uint32_t *imp_rc)
{
	uint32_t regData;
	uint32_t rc = RIO_SUCCESS;

	rc = DARRegRead(dev_info, TSI578_SPX_CTL_INDEP(pnum), &regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0x30);
		goto exit;
	}

	if (event->em_detect == rio_em_detect_on) {
		uint32_t retry_thresh = event->em_info;
		uint32_t spx_err_rate, spx_err_thresh;

		if (!retry_thresh) {
			rc = RIO_ERR_INVALID_PARAMETER;
			*imp_rc = SET_EVENT_EN(0x31);
			goto exit;
		}

		if (retry_thresh
				> (TSI578_SPX_CTL_INDEP_MAX_RETRY_THRESHOLD >> 8)) {
			retry_thresh =
			TSI578_SPX_CTL_INDEP_MAX_RETRY_THRESHOLD;
		} else {
			retry_thresh = retry_thresh << 8;
		}

		regData |= TSI578_SPX_CTL_INDEP_MAX_RETRY_EN;
		regData &= ~TSI578_SPX_CTL_INDEP_MAX_RETRY_THRESHOLD;
		regData |= retry_thresh;

		rc = DARRegRead(dev_info, TSI578_SPX_ERR_RATE(pnum),
				&spx_err_rate);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x32);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_ERR_THRESH(pnum),
				&spx_err_thresh);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x33);
			goto exit;
		}

		if (!(spx_err_thresh & TSI578_SPX_ERR_THRESH_ERR_RFT))
			spx_err_thresh |= 0x01000000;

		rc = tsi57x_enable_err_ctr(dev_info, pnum,
		TSI578_SPX_ERR_DET_IMP_SPEC_ERR,
		TSI578_SPX_ERR_DET_IMP_SPEC_ERR, spx_err_rate, spx_err_thresh,
				imp_rc);
		if (RIO_SUCCESS != rc) {
			goto exit;
		}
	} else { // rio_em_detect_off:
		 // Disable events.
		regData &= ~(TSI578_SPX_CTL_INDEP_MAX_RETRY_EN
				| TSI578_SPX_CTL_INDEP_MAX_RETRY_THRESHOLD);
	}

	rc = DARRegWrite(dev_info, TSI578_SPX_CTL_INDEP(pnum), regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0x34);
	}

exit:
	return rc;
}

static uint32_t tsi57x_set_event_en_cfg_rio_em_f_2many_pna(DAR_DEV_INFO_t *dev_info,
		uint8_t pnum, rio_em_cfg_t *event, uint32_t *imp_rc)
{
	uint32_t regData;
	uint32_t rc = RIO_SUCCESS;

	// Enable/disable event detection
	if (event->em_detect == rio_em_detect_on) {
		uint32_t spx_err_rate, spx_err_thresh;
		rc = DARRegRead(dev_info, TSI578_SPX_ERR_RATE(pnum),
				&spx_err_rate);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x40);
			goto exit;
		}
		rc = DARRegRead(dev_info, TSI578_SPX_ERR_THRESH(pnum),
				&spx_err_thresh);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x41);
			goto exit;
		}

		spx_err_thresh &= ~TSI578_SPX_ERR_THRESH_ERR_RFT;
		spx_err_thresh |= ((event->em_info << 24)
				& TSI578_SPX_ERR_THRESH_ERR_RFT);

		rc = tsi57x_enable_err_ctr(dev_info, pnum,
		TSI578_SPX_RATE_EN_CS_NOT_ACC_EN,
		TSI578_SPX_RATE_EN_CS_NOT_ACC_EN, spx_err_rate, spx_err_thresh,
				imp_rc);
	} else {
		// Disable event by not counting PNA's in error counter.
		rc = DARRegRead(dev_info, TSI578_SPX_RATE_EN(pnum), &regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x42);
			goto exit;
		}

		regData &= ~(TSI578_SPX_RATE_EN_CS_NOT_ACC_EN);
		rc = DARRegWrite(dev_info, TSI578_SPX_RATE_EN(pnum), regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x43);
		}
	}

exit:
	return rc;
}

static uint32_t tsi57x_set_event_en_cfg_rio_em_f_err_rate(DAR_DEV_INFO_t *dev_info,
		uint8_t pnum, rio_em_cfg_t *event, uint32_t *imp_rc)
{
	uint32_t spx_rate_en;
	uint32_t spx_err_rate;
	uint32_t spx_err_thresh;
	uint32_t regData;
	uint32_t rc = RIO_SUCCESS;

	rc = rio_em_get_f_err_rate_info(event->em_info, &spx_rate_en,
			&spx_err_rate, &spx_err_thresh);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0x50);
		goto exit;
	}

	spx_rate_en &= ~EM_ERR_RATE_EVENT_EXCLUSIONS;

	// Enable/disable event detection
	if (event->em_detect == rio_em_detect_on) {
		uint32_t curr_thresh;
		rc = DARRegRead(dev_info, TSI578_SPX_ERR_THRESH(pnum),
				&curr_thresh);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x51);
			goto exit;
		}

		curr_thresh &= ~TSI578_SPX_ERR_THRESH_ERR_RFT;
		curr_thresh |= (spx_err_thresh & TSI578_SPX_ERR_THRESH_ERR_RFT);

		rc = tsi57x_enable_err_ctr(dev_info, pnum, spx_rate_en,
				~EM_ERR_RATE_EVENT_EXCLUSIONS, spx_err_rate,
				curr_thresh, imp_rc);
	} else {
		// Disable event by disabling error counting for ALL f_err_rate errors
		rc = DARRegRead(dev_info, TSI578_SPX_RATE_EN(pnum), &regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x52);
			goto exit;
		}

		regData = regData & EM_ERR_RATE_EVENT_EXCLUSIONS;

		rc = DARRegWrite(dev_info, TSI578_SPX_RATE_EN(pnum), regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x53);
		}
	}

exit:
	return rc;
}

static uint32_t tsi57x_set_event_en_cfg_rio_em_i_init_fail(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_t *event, uint32_t *imp_rc)
{
	uint32_t regData;
	uint32_t rc = RIO_SUCCESS;

	// Enable reporting to top level
	// There is a top-level bit which gates whether or not this
	// event is reported as an interrupt.  It cannot be reported as
	// a port-write.
	rc = DARRegRead(dev_info, TSI578_I2C_INT_ENABLE, &regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0x60);
		goto exit;
	}

	if (event->em_detect == rio_em_detect_on) {
		regData |= TSI578_I2C_INT_ENABLE_BL_FAIL;
	} else {
		regData &= ~(TSI578_I2C_INT_ENABLE_BL_FAIL);
	}

	rc = DARRegWrite(dev_info, TSI578_I2C_INT_ENABLE, regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0x61);
	}

exit:
	return rc;
}

static uint32_t tsi57x_set_event_en_cfg_rio_em_d_log(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_t *event, uint32_t *imp_rc)
{
	uint32_t regData;
	uint32_t rc = RIO_SUCCESS;

	// Specific logical layer error enables are controlled by info parameter
	rc = DARRegRead(dev_info, TSI578_RIO_LOG_ERR_DET_EN, &regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0x70);
		goto exit;
	}

	if (event->em_detect == rio_em_detect_on) {
		regData = (regData & ~(TSI57X_ALL_LOG_ERRS))
				| (event->em_info & TSI57X_ALL_LOG_ERRS);
	} else {
		regData &= ~(TSI57X_ALL_LOG_ERRS);
	}

	rc = DARRegWrite(dev_info, TSI578_RIO_LOG_ERR_DET_EN, regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0x71);
	}

exit:
	return rc;
}

static uint32_t tsi57x_set_event_en_cfg_rio_em_d_rte(DAR_DEV_INFO_t *dev_info,
		uint8_t pnum, rio_em_cfg_t *event, uint32_t *imp_rc)
{
	uint32_t regData;
	uint32_t rc = RIO_SUCCESS;

	rc = DARRegRead(dev_info, TSI578_SPX_CTL_INDEP(pnum), &regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0x80);
		goto exit;
	}

	if (event->em_detect == rio_em_detect_on) {
		regData |= TSI578_SPX_CTL_INDEP_ILL_TRANS_ERR;
	} else {
		regData &= ~(TSI578_SPX_CTL_INDEP_ILL_TRANS_ERR);
	}

	rc = DARRegWrite(dev_info, TSI578_SPX_CTL_INDEP(pnum), regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0x81);
	}

exit:
	return rc;
}

static uint32_t tsi57x_set_event_en_cfg_rio_em_i_sig_det(DAR_DEV_INFO_t *dev_info,
		uint8_t pnum, rio_em_cfg_t *event, uint32_t *imp_rc)
{
	uint32_t regData;
	uint32_t rc = RIO_SUCCESS;

	if (!event->em_info) {
		rio_pc_get_status_in_t stat_in;
		rio_pc_get_status_out_t stat_out;

		stat_in.ptl.num_ports = 1;
		stat_in.ptl.pnums[0] = pnum;

		rc = rio_pc_get_status(dev_info, &stat_in, &stat_out);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x90);
			goto exit;
		}

		if (((rio_em_detect_off == event->em_detect)
				&& !stat_out.ps[0].port_ok)
				|| ((rio_em_detect_on == event->em_detect)
						&& stat_out.ps[0].port_ok)) {
			goto exit;
		}
	}

	rc = DARRegRead(dev_info, TSI578_SPX_CTL_INDEP(pnum), &regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0x91);
		goto exit;
	}

	if (event->em_detect == rio_em_detect_on) {
		regData |= TSI578_SPX_CTL_INDEP_LINK_INIT_NOTIFICATION_EN;
		rc = DARRegWrite(dev_info, TSI578_SPX_CTL_INDEP(pnum), regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x92);
			goto exit;
		}

		// Must set PORT_LOCKOUT for the event to be detected...
		rc = DARRegRead(dev_info, TSI578_SPX_CTL(pnum), &regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x93);
			goto exit;
		}

		regData |= TSI578_SPX_CTL_PORT_LOCKOUT;
		rc = DARRegWrite(dev_info, TSI578_SPX_CTL(pnum), regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x94);
		}
	} else {
		// Disable detection, reporting/counting remain unaffected.
		// Do not touch PORT_LOCKOUT. Let the PC routine control PORT_LOCKOUT.
		regData &= ~(TSI578_SPX_CTL_INDEP_LINK_INIT_NOTIFICATION_EN);
		rc = DARRegWrite(dev_info, TSI578_SPX_CTL_INDEP(pnum), regData);
		if (RIO_SUCCESS != rc) {
			*imp_rc = SET_EVENT_EN(0x95);
		}
	}

exit:
	return rc;
}

static uint32_t tsi57x_set_event_en_cfg_rio_em_i_rst_req(DAR_DEV_INFO_t *dev_info,
		uint8_t pnum, rio_em_cfg_t *event, uint32_t *imp_rc)
{
	uint32_t regData;
	uint32_t rc = RIO_SUCCESS;

	rc = DARRegRead(dev_info, TSI578_SPX_MODE(pnum), &regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0xa0);
		goto exit;
	}

	if (event->em_detect == rio_em_detect_on) {
		// Can't detect the event unless self_rst is disabled.
		regData &= ~(TSI578_SPX_MODE_SELF_RST);
		regData |= TSI578_SPX_MODE_RCS_INT_EN;
	} else {
		// DO NOT CHANGE SELF_RST
		// This should be handled by the PC routines.
		regData &= ~(TSI578_SPX_MODE_RCS_INT_EN);
	}

	rc = DARRegWrite(dev_info, TSI578_SPX_MODE(pnum), regData);
	if (RIO_SUCCESS != rc) {
		*imp_rc = SET_EVENT_EN(0xa1);
	}
exit:
	return rc;
}


static uint32_t tsi57x_set_event_en_cfg(DAR_DEV_INFO_t *dev_info,
		uint8_t pnum, rio_em_cfg_t *event, uint32_t *imp_rc)
{
	uint32_t rc = RIO_SUCCESS;

	if ((event->em_detect >= rio_em_detect_last)
			|| (event->em_event >= rio_em_last)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		*imp_rc = SET_EVENT_EN(0x01);
		goto exit;
	}

	// Nothing to do...
	if (event->em_detect == rio_em_detect_0delta) {
		goto exit;
	}

	switch (event->em_event) {
	case rio_em_f_los:
		rc = tsi57x_set_event_en_cfg_rio_f_los(dev_info, pnum, event, imp_rc);
		break;

	case rio_em_f_port_err:
		rc = tsi57x_set_event_en_cfg_rio_em_f_port_err(dev_info, pnum, event, imp_rc);
		break;

	case rio_em_f_2many_retx:
		rc = tsi57x_set_event_en_cfg_rio_em_f_2many_retx(dev_info, pnum, event, imp_rc);
		break;

	case rio_em_f_2many_pna:
		rc = tsi57x_set_event_en_cfg_rio_em_f_2many_pna(dev_info, pnum, event, imp_rc);
		break;

	case rio_em_f_err_rate:
		rc = tsi57x_set_event_en_cfg_rio_em_f_err_rate(dev_info, pnum, event, imp_rc);
		break;

	case rio_em_i_init_fail:
		rc = tsi57x_set_event_en_cfg_rio_em_i_init_fail(dev_info, event, imp_rc);
		break;

	case rio_em_d_log:
		rc = tsi57x_set_event_en_cfg_rio_em_d_log(dev_info, event, imp_rc);
		break;

	case rio_em_d_ttl:
	case rio_em_a_clr_pwpnd:
	case rio_em_a_no_event:
		// Nothing to do
		break;

	case rio_em_d_rte:
		rc = tsi57x_set_event_en_cfg_rio_em_d_rte(dev_info, pnum, event, imp_rc);
		break;

	case rio_em_i_sig_det:
		rc = tsi57x_set_event_en_cfg_rio_em_i_sig_det(dev_info, pnum, event, imp_rc);
		break;

	case rio_em_i_rst_req:
		rc = tsi57x_set_event_en_cfg_rio_em_i_rst_req(dev_info, pnum, event, imp_rc);
		break;

	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		*imp_rc = SET_EVENT_EN(0x02);
		break;
	}

exit:
	return rc;
}

static uint32_t tsi57x_create_rate_event(DAR_DEV_INFO_t *dev_info, uint8_t pnum,
		uint32_t spx_rate_en, uint32_t *imp_err)
{
	uint32_t rc;
	uint32_t regVal, limit, idx;

	rc = DARRegRead(dev_info, TSI578_SPX_RATE_EN(pnum), &regVal);
	if (RIO_SUCCESS != rc) {
		*imp_err = CREATE_RATE(0x80);
		goto exit;
	}

	// If requested event is not enabled for counting, can't create the event.
	if (!(spx_rate_en & regVal)) {
		rc = RIO_ERR_NOT_SUP_BY_CONFIG;
		*imp_err = CREATE_RATE(0x81);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI578_SPX_ERR_THRESH(pnum), &regVal);
	if (RIO_SUCCESS != rc) {
		*imp_err = CREATE_RATE(0x82);
		goto exit;
	}

	limit = (regVal & TSI578_SPX_ERR_THRESH_ERR_RFT) >> 24;
	// If there is no error threshold set, cannot create event.
	if (!limit) {
		rc = RIO_ERR_NOT_SUP_BY_CONFIG;
		*imp_err = CREATE_RATE(0x83);
		goto exit;
	}

	// Create the event.  Preserve other events in the error detect register.
	// Note that this may fail if a "fast" leak rate is set.
	// Note that this may be overkill if multiple events occur for each write.
	for (idx = 0; idx < limit; idx++) {
		rc = DARRegRead(dev_info, TSI578_SPX_ERR_DET(pnum), &regVal);
		if (RIO_SUCCESS != rc) {
			*imp_err = CREATE_RATE(0x84);
			goto exit;
		}

		rc = DARRegWrite(dev_info, TSI578_SPX_ERR_DET(pnum),
				regVal | spx_rate_en);
		if (RIO_SUCCESS != rc) {
			*imp_err = CREATE_RATE(0x85);
			goto exit;
		}
	}

exit:
	return rc;

}

uint32_t tsi57x_rio_em_cfg_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_pw_in_t *in_parms, rio_em_cfg_pw_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t regData;
	uint8_t port_num;

	out_parms->imp_rc = RIO_SUCCESS;

	if (in_parms->priority > 3) {
		out_parms->imp_rc = EM_CFG_PW(1);
		goto exit;
	}

	// Configure destination ID for port writes.
	regData = (uint32_t)(in_parms->port_write_destID) << 16;
	if (tt_dev16 == in_parms->deviceID_tt) {
		regData |= TSI578_RIO_PW_DESTID_LARGE_DESTID;
	} else {
		regData &= ~(TSI578_RIO_PW_DESTID_LARGE_DESTID
				| TSI578_RIO_PW_DESTID_DESTID_MSB);
	}

	rc = DARRegWrite(dev_info, TSI578_RIO_PW_DESTID, regData);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(2);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI578_RIO_PW_DESTID, &regData);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(3);
		goto exit;
	}

	out_parms->deviceID_tt =
			(regData & TSI578_RIO_PW_DESTID_LARGE_DESTID) ?
					tt_dev16 : tt_dev8;
	out_parms->port_write_destID = (did_reg_t)((regData
			& ( TSI578_RIO_PW_DESTID_DESTID_LSB
					| TSI578_RIO_PW_DESTID_DESTID_MSB))
			>> 16);
	// Cannot configure source ID for port-writes on Tsi57x family.
	out_parms->srcID_valid = true;
	out_parms->port_write_srcID = 0;

	// Configure port-write priority.  Cannot configure CRF.

	for (port_num = 0; port_num < TSI57X_NUM_PORTS(dev_info); port_num++) {
		rc = DARRegRead(dev_info, TSI578_SPX_DISCOVERY_TIMER(port_num),
				&regData);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_CFG_PW(4);
			goto exit;
		}

		regData &= ~TSI578_SPX_DISCOVERY_TIMER_PW_PRIORITY;
		regData |= (((uint32_t)(in_parms->priority)) << 22)
				& TSI578_SPX_DISCOVERY_TIMER_PW_PRIORITY;
		rc = DARRegWrite(dev_info, TSI578_SPX_DISCOVERY_TIMER(port_num),
				regData);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_CFG_PW(5);
			goto exit;
		}
	}

	out_parms->priority = in_parms->priority
			& (TSI578_SPX_DISCOVERY_TIMER_PW_PRIORITY >> 22);
	out_parms->CRF = false;

	// Configure port-write re-transmission rate.
	// Assumption: it is better to choose a longer retransmission time than the value requested.
	regData = in_parms->port_write_re_tx / RIO_EM_TSI578_PW_RE_TX_167P7MS;

	switch (regData) {
	case 0: // If the requested retransmission time is shorter than the
		//    minimum, set the minimum rather than disabling
		//    retransmission
		if (in_parms->port_write_re_tx) {
			regData = TSI578_RIO_PW_TIMEOUT_167P7MS;
		} else {
			regData = TSI578_RIO_PW_TIMEOUT_DISABLE;
		}
		break;
	case 1:
		regData = TSI578_RIO_PW_TIMEOUT_167P7MS;
		break;
	case 2:
		regData = TSI578_RIO_PW_TIMEOUT_335P5MS;
		break;
	case 3:
	case 4:
		regData = TSI578_RIO_PW_TIMEOUT_671P1MS;
		break;
	default:
		regData = TSI578_RIO_PW_TIMEOUT_1340MS;
		break;
	}

	rc = DARRegWrite(dev_info, TSI578_RIO_PW_TIMEOUT, regData);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(7);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI578_RIO_PW_TIMEOUT, &regData);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(8);
		goto exit;
	}

	regData = (regData & TSI578_RIO_PW_TIMEOUT_PW_TIMER) >> 28;

	switch (regData) {
	case 0:
		out_parms->port_write_re_tx = RIO_EM_PW_RE_TX_DISABLED;
		break;
	case 1:
		out_parms->port_write_re_tx = RIO_EM_TSI578_PW_RE_TX_167P7MS;
		break;
	case 2:
		out_parms->port_write_re_tx = RIO_EM_TSI578_PW_RE_TX_335P5MS;
		break;
	case 4:
		out_parms->port_write_re_tx = RIO_EM_TSI578_PW_RE_TX_671P1MS;
		break;
	case 8:
		out_parms->port_write_re_tx = RIO_EM_TSI578_PW_RE_TX_1340MS;
		break;
		break;
	default:
		out_parms->port_write_re_tx = regData;
		rc = RIO_ERR_READ_REG_RETURN_INVALID_VAL;
		out_parms->imp_rc = EM_CFG_PW(9);
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_em_cfg_set(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_set_in_t *in_parms, rio_em_cfg_set_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t pnum;
	uint8_t idx, e_idx;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	struct DAR_ptl good_ptl;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->fail_port_num = RIO_ALL_PORTS;
	out_parms->fail_idx = rio_em_last;
	out_parms->notfn = rio_em_notfn_0delta;

	if ((in_parms->num_events > (uint8_t)(rio_em_last))
			|| ( NULL == in_parms->events)) {
		out_parms->imp_rc = EM_CFG_SET(0x10);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_SET(0x15);
		goto exit;
	}

	cfg_in.ptl = good_ptl;
	rc = tsi57x_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto exit;
	}

	if (!cfg_out.num_ports) {
		rc = RIO_ERR_NO_PORT_AVAIL;
		out_parms->imp_rc = cfg_out.imp_rc;
		goto exit;
	}

	/* For all available ports, configure the events requested. */
	/* First, disable all events requested.                     */
	for (idx = 0; idx < cfg_out.num_ports; idx++) {
		if (cfg_out.pc[idx].port_available
				&& cfg_out.pc[idx].powered_up) {
			pnum = cfg_out.pc[idx].pnum;
			out_parms->fail_port_num = pnum;
			out_parms->fail_idx = 0;

			for (e_idx = 0;
					(e_idx < in_parms->num_events)
							&& (e_idx
									< (uint8_t)(rio_em_last));
					e_idx++) {
				if (in_parms->events[e_idx].em_detect
						== rio_em_detect_off) {
					out_parms->fail_idx = e_idx;
					rc =
							tsi57x_set_event_en_cfg(
									dev_info,
									pnum,
									&(in_parms->events[e_idx]),
									&out_parms->imp_rc);
					if (RIO_SUCCESS != rc) {
						goto exit;
					}
				}
			}

			for (e_idx = 0;
					(e_idx < in_parms->num_events)
							&& (e_idx
									< (uint8_t)(rio_em_last));
					e_idx++) {
				if (in_parms->events[e_idx].em_detect
						== rio_em_detect_on) {
					out_parms->fail_idx = e_idx;
					rc =
							tsi57x_set_event_en_cfg(
									dev_info,
									pnum,
									&(in_parms->events[e_idx]),
									&out_parms->imp_rc);
					if (RIO_SUCCESS != rc) {
						goto exit;
					}
				}
			}
		}
	}

	out_parms->fail_port_num = RIO_ALL_PORTS;
	out_parms->fail_idx = rio_em_last;

	rc = tsi57x_set_int_cfg(dev_info, &good_ptl, in_parms->notfn,
			&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	rc = tsi57x_set_pw_cfg(dev_info, &good_ptl, in_parms->notfn,
			&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	out_parms->notfn = in_parms->notfn;
	if ((out_parms->notfn > rio_em_notfn_both)
			&& (RIO_ALL_PORTS != in_parms->ptl.num_ports)) {
		rc = tsi57x_em_determine_notfn(dev_info, &out_parms->notfn,
				in_parms->ptl.pnums[0], &out_parms->imp_rc);
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_em_cfg_get(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_get_in_t *in_parms, rio_em_cfg_get_out_t *out_parms)
{

	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t spx_ctl, spx_mode, spx_ctl_indep, spx_rate_en, spx_dloop,
			i2c_int_enable, log_err_en, spx_err_thresh,
			spx_err_rate;
	uint8_t pnum;
	uint8_t e_idx;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;

	out_parms->fail_idx = (uint8_t)(rio_em_last);
	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->notfn = rio_em_notfn_0delta;

	if (in_parms->port_num >= TSI57X_NUM_PORTS(dev_info)) {
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

	rc = tsi57x_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto exit;
	}

	if (!cfg_out.num_ports) {
		rc = RIO_ERR_NO_PORT_AVAIL;
		out_parms->imp_rc = EM_CFG_GET(0x03);
		goto exit;
	}

	if (!cfg_out.pc[0].port_available || !cfg_out.pc[0].powered_up) {
		rc = RIO_ERR_NO_PORT_AVAIL;
		out_parms->imp_rc = EM_CFG_GET(0x04);
		goto exit;
	}

	pnum = in_parms->port_num;

	rc = DARRegRead(dev_info, TSI578_SPX_MODE(pnum), &spx_mode);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_GET(0x05);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI578_SPX_CTL_INDEP(pnum), &spx_ctl_indep);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_GET(0x06);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI578_SPX_RATE_EN(pnum), &spx_rate_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_GET(0x07);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI578_SPX_ERR_THRESH(pnum), &spx_err_thresh);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_GET(0x08);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI578_SPX_ERR_RATE(pnum), &spx_err_rate);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_GET(0x09);
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
			// Check setting of DLT, return timeout parameter if possible.
			rc = DARRegRead(dev_info,
					TSI578_SMACX_DLOOP_CLK_SEL(pnum & ~1),
					&spx_dloop);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CFG_GET(0x10);
				goto exit;
			}

			if (spx_dloop & TSI578_SMACX_DLOOP_CLK_SEL_DLT_EN) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
				if (TSI578_SPX_ERR_DET_DELIN_ERR & spx_rate_en)
					in_parms->events[e_idx].em_info = 1;
				else {
					in_parms->events[e_idx].em_info =
							(spx_dloop
									& TSI578_SMACX_DLOOP_CLK_SEL_DLT_THRESH)
									>> 16;
					in_parms->events[e_idx].em_info =
							in_parms->events[e_idx].em_info
									* 81920;
				}
			}
			break;

		case rio_em_f_port_err:
			if (spx_ctl_indep & TSI578_SPX_CTL_INDEP_PORT_ERR_EN) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
			}
			break;

		case rio_em_f_2many_retx:
			if ((spx_ctl_indep & TSI578_SPX_CTL_INDEP_MAX_RETRY_EN)
					&& (spx_ctl_indep
							& TSI578_SPX_CTL_INDEP_MAX_RETRY_THRESHOLD)) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
				in_parms->events[e_idx].em_info =
						(spx_ctl_indep
								& TSI578_SPX_CTL_INDEP_MAX_RETRY_THRESHOLD)
								>> 8;
			}
			break;

		case rio_em_f_2many_pna:
			if (spx_rate_en & TSI578_SPX_RATE_EN_CS_NOT_ACC_EN) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
				in_parms->events[e_idx].em_info =
						(spx_err_thresh
								& TSI578_SPX_ERR_THRESH_ERR_RFT)
								>> 24;
			}
			break;

		case rio_em_f_err_rate:
			rc =
					rio_em_compute_f_err_rate_info(
							spx_rate_en
									& ~EM_ERR_RATE_EVENT_EXCLUSIONS,
							spx_err_rate,
							spx_err_thresh,
							&in_parms->events[e_idx].em_info);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CFG_GET(0x30);
				goto exit;
			}
			if (spx_rate_en & ~EM_ERR_RATE_EVENT_EXCLUSIONS) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
			}
			break;

		case rio_em_i_init_fail:
			rc = DARRegRead(dev_info, TSI578_I2C_INT_ENABLE,
					&i2c_int_enable);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CFG_GET(0x11);
				goto exit;
			}

			if (i2c_int_enable & TSI578_I2C_INT_ENABLE_BL_FAIL) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
			}
			break;

		case rio_em_d_ttl:
		case rio_em_a_clr_pwpnd:
		case rio_em_a_no_event:
			// Nothing to do
			break;

		case rio_em_d_rte:
			if (spx_ctl_indep & TSI578_SPX_CTL_INDEP_ILL_TRANS_ERR) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
			}
			break;

		case rio_em_d_log:
			rc = DARRegRead(dev_info, TSI578_RIO_LOG_ERR_DET_EN,
					&log_err_en);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CFG_GET(0x12);
				goto exit;
			}

			if (log_err_en) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
				in_parms->events[e_idx].em_info = log_err_en
						& TSI57X_ALL_LOG_ERRS;
			}
			break;

		case rio_em_i_sig_det:
			rc = DARRegRead(dev_info, TSI578_SPX_CTL(pnum),
					&spx_ctl);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CFG_GET(0x13);
				goto exit;
			}

			if ((spx_ctl_indep
					& TSI578_SPX_CTL_INDEP_LINK_INIT_NOTIFICATION_EN)
					&& (spx_ctl
							& TSI578_SPX_CTL_PORT_LOCKOUT)) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
			}
			break;

		case rio_em_i_rst_req:
			if (!(spx_mode & TSI578_SPX_MODE_SELF_RST)
					&& (spx_ctl_indep
							& TSI578_SPX_CTL_INDEP_RSVD1)) {
				in_parms->events[e_idx].em_detect =
						rio_em_detect_on;
			}
			break;

		default:
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = EM_CFG_GET(0x14);
			goto exit;
		}
	}

	out_parms->fail_idx = (uint8_t)(rio_em_last);

	// Now figure out the current notification setting for this port.
	rc = tsi57x_em_determine_notfn(dev_info, &out_parms->notfn, pnum,
			&out_parms->imp_rc);

exit:
	return rc;
}

uint32_t tsi57x_rio_em_dev_rpt_ctl(DAR_DEV_INFO_t *dev_info,
		rio_em_dev_rpt_ctl_in_t *in_parms,
		rio_em_dev_rpt_ctl_out_t *out_parms)
{
	uint32_t rc;
	struct DAR_ptl good_ptl;

	out_parms->notfn = rio_em_notfn_0delta;
	out_parms->imp_rc = RIO_SUCCESS;

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = DEV_RPT_CTL(1);
		goto exit;
	}

	rc = tsi57x_set_int_cfg(dev_info, &good_ptl, in_parms->notfn,
			&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	rc = tsi57x_set_pw_cfg(dev_info, &good_ptl, in_parms->notfn,
			&out_parms->imp_rc);
	if (RIO_SUCCESS == rc) {
		out_parms->notfn = in_parms->notfn;
	}

	out_parms->notfn = in_parms->notfn;
	if ((1 == good_ptl.num_ports)
			&& (rio_em_notfn_0delta == in_parms->notfn)) {
		rc = tsi57x_em_determine_notfn(dev_info, &out_parms->notfn,
				good_ptl.pnums[0], &out_parms->imp_rc);
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_em_parse_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_parse_pw_in_t *in_parms,
		rio_em_parse_pw_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t idx;
	// Do not check COMP_TAG, IMP_SPEC bit or port number field for other events.
	uint32_t oth_masks[4] = {0xFFFFFFFF, 0x80000000, 0x000000FF, 0};

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->num_events = 0;
	out_parms->too_many = false;
	out_parms->other_events = false;

	if ((!in_parms->num_events)
			|| (in_parms->num_events > (uint8_t)(rio_em_last))
			|| (NULL == dev_info) || (NULL == in_parms->events)) {
		out_parms->imp_rc = PARSE_PW(1);
		goto exit;
	}

	// Check each bit within the port-write data for information on Error Management events
	for (idx = 0; (idx < (uint8_t)(rio_em_last)) && !(out_parms->too_many);
			idx++) {
		if (in_parms->pw[pw_parsing_info[idx].pw_index]
				& pw_parsing_info[idx].check) {
			if (out_parms->num_events < in_parms->num_events) {
				in_parms->events[out_parms->num_events].event =
						pw_parsing_info[idx].event;
				if (pw_parsing_info[idx].per_port)
					in_parms->events[out_parms->num_events].port_num =
							(uint8_t)(in_parms->pw[2]
									& 0x000000FF);
				else
					in_parms->events[out_parms->num_events].port_num =
							RIO_ALL_PORTS;
				out_parms->num_events++;
			} else {
				out_parms->too_many = true;
			}
		}
	}

	// Check for "other" events that have happenned...
	for (idx = 0; idx < (uint8_t)(rio_em_last); idx++) {
		oth_masks[pw_parsing_info[idx].pw_index] |=
				pw_parsing_info[idx].check;
	}
	for (idx = 0; (idx < 3) && !out_parms->other_events; idx++) {
		out_parms->other_events =
				(in_parms->pw[idx] & ~oth_masks[idx]) ?
						true : false;
	}

	rc = RIO_SUCCESS;

exit:
	return rc;
}

uint32_t tsi57x_rio_em_get_int_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_int_stat_in_t *in_parms,
		rio_em_get_int_stat_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t pnum;
	uint8_t idx;
	uint32_t spx_int_stat, spx_err_stat, spx_err_det, spx_rate_en,
			spx_ctl_indep, spx_cs_int, spx_dloop;
	uint32_t regData;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	port_mac_relations_t *sw_pmr;
	struct DAR_ptl good_ptl;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->num_events = 0;
	out_parms->too_many = false;
	out_parms->other_events = false;

	if ((!in_parms->num_events)
			|| (in_parms->num_events > EM_MAX_EVENT_LIST_SIZE)
			|| (NULL == in_parms->events)) {
		out_parms->imp_rc = GET_INT_STAT(1);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = GET_INT_STAT(2);
		goto exit;
	}

	cfg_in.ptl = good_ptl;
	rc = tsi57x_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto exit;
	}

	if (!cfg_out.num_ports) {
		rc = RIO_ERR_NO_PORT_AVAIL;
		out_parms->imp_rc = GET_INT_STAT(3);
		goto exit;
	}

	rc = tsi57x_init_sw_pmr(dev_info, &sw_pmr);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = GET_INT_STAT(4);
		goto exit;
	}

	for (idx = 0; idx < cfg_out.num_ports; idx++) {
		if (!(cfg_out.pc[idx].port_available
				&& cfg_out.pc[idx].powered_up))
			continue;

		pnum = cfg_out.pc[idx].pnum;

		rc = DARRegRead(dev_info,
				TSI578_SMACX_DLOOP_CLK_SEL(
						sw_pmr[pnum].mac_num * 2),
				&spx_dloop);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_INT_STAT(5);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_CTL_INDEP(pnum),
				&spx_ctl_indep);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_INT_STAT(6);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_INT_STATUS(pnum),
				&spx_int_stat);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_INT_STAT(7);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_ERR_STATUS(pnum),
				&spx_err_stat);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_INT_STAT(8);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_ERR_DET(pnum),
				&spx_err_det);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_INT_STAT(9);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_RATE_EN(pnum),
				&spx_rate_en);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_INT_STAT(0xA);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_CS_INT_STATUS(pnum),
				&spx_cs_int);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_INT_STAT(0xB);
			goto exit;
		}

		// Loss of signal events are a bit flakey, especially for rapid error detection.
		// Loss of Signal events only occur when the dead link timer has been enabled.
		// If a delineation error occurs and and "Output Fail" condition occurs as well,
		//   this indicates that rapid error detection has probably been invoked.
		// Otherwise, a loss of signal (DLT) event has probably occurred when a PORT_ERR
		//   is detected without link response timeouts or illegal ackIDs in the link response.
		// Neither of these conditions is deterministic.

		if (((TSI578_SPX_ERR_DET_DELIN_ERR & spx_err_det)
				&& (TSI578_SPX_ERR_STATUS_OUTPUT_FAIL
						& spx_err_stat))
				|| ((spx_err_stat
						& TSI578_SPX_ERR_STATUS_PORT_ERR)
						&& !(spx_err_det
								& (TSI578_SPX_ERR_DET_LINK_TO
										| TSI578_SPX_ERR_DET_LR_ACKID_ILL)))) {
			if (spx_dloop & TSI578_SMACX_DLOOP_CLK_SEL_DLT_EN) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_f_los);
				if ((RIO_ALL_PORTS != in_parms->ptl.num_ports)
						&&   //
						(TSI577_RIO_DEVID_VAL
								!= DEV_CODE(
										dev_info))
						&& (!sw_pmr[pnum].lane_count_4x)) {
					rio_em_add_int_event(in_parms, out_parms,
							sw_pmr[pnum].other_mac_ports[0],
							rio_em_f_port_err);
				}
			} else {
				out_parms->other_events = true;
			}
		}

		// Note: When the MAC is configured in two 1x mode, when PORT_ERR is set on the
		// odd MAC it is also set on the even MAC.
		//
		// PORT_ERR occurs when:
		// - there are 4 consecutive link-response timeouts, or
		// - 4 consecutive link-responses with illegal ackIDs.
		// - dead link timer expiry on the odd port of a non-Tsi577 device, and this is
		//   the even port.
		// If none of these errors is present, do not report the PORT_ERR.
		// There is a hook to ensure that PORT_ERR is cleared on the EVEN MAC.

		if ((spx_err_stat & TSI578_SPX_ERR_STATUS_PORT_ERR)
				&& (spx_err_det
						& (TSI578_SPX_ERR_DET_LINK_TO
								| TSI578_SPX_ERR_DET_LR_ACKID_ILL))) {
			if (spx_ctl_indep & TSI578_SPX_CTL_INDEP_PORT_ERR_EN) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_f_port_err);
				if ((RIO_ALL_PORTS != in_parms->ptl.num_ports)
						&&   //
						(TSI577_RIO_DEVID_VAL
								!= DEV_CODE(
										dev_info))
						&& (!sw_pmr[pnum].lane_count_4x)) {
					rio_em_add_int_event(in_parms, out_parms,
							sw_pmr[pnum].other_mac_ports[0],
							rio_em_f_port_err);
				}
			} else {
				out_parms->other_events = true;
			}
		}

		if (spx_int_stat & TSI578_SPX_INT_STATUS_MAX_RETRY) {
			if (spx_ctl_indep & TSI578_SPX_CTL_INDEP_MAX_RETRY_EN) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_f_2many_retx);
			} else {
				out_parms->other_events = true;
			}
		}

		// Check for too many PNA and ERR_RATE.
		if (spx_err_stat & TSI578_SPX_ERR_STATUS_OUTPUT_FAIL) {
			if (TSI578_SPX_ERR_DET_CS_NOT_ACC & spx_err_det
					& spx_rate_en) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_f_2many_pna);
			}

			if (~EM_ERR_RATE_EVENT_EXCLUSIONS & spx_err_det
					& spx_rate_en) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_f_err_rate);
			}

			if (spx_err_det & ~spx_rate_en) {
				out_parms->other_events = true;
			}
		}

		if (spx_int_stat & TSI578_SPX_INT_STATUS_ILL_TRANS_ERR) {
			if (spx_ctl_indep & TSI578_SPX_CTL_INDEP_ILL_TRANS_ERR) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_d_rte);
			} else {
				out_parms->other_events = true;
			}
		}

		if (spx_int_stat
				& TSI578_SPX_INT_STATUS_LINK_INIT_NOTIFICATION) {
			if (spx_ctl_indep
					& TSI578_SPX_CTL_INDEP_LINK_INIT_NOTIFICATION_EN) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_i_sig_det);
			} else {
				out_parms->other_events = true;
			}
		}

		if (spx_cs_int & TSI578_SPX_CS_INT_STATUS_RCS) {
			if (spx_ctl_indep & TSI578_SPX_CTL_INDEP_RSVD1) {
				rio_em_add_int_event(in_parms, out_parms, pnum,
						rio_em_i_rst_req);
			} else {
				out_parms->other_events = true;
			}
		}
	}

	// Logical Error interrupt occurs when logical layer error detected (rare)
	// and logical error events are enabled
	rc = DARRegRead(dev_info, TSI578_RIO_LOG_ERR_DET, &regData);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = GET_INT_STAT(0x10);
		goto exit;
	}

	if (regData) {
		uint32_t enData;

		rc = DARRegRead(dev_info, TSI578_RIO_LOG_ERR_DET_EN, &enData);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_INT_STAT(0x11);
			goto exit;
		}
		if (regData & enData) {
			rio_em_add_int_event(in_parms, out_parms, RIO_ALL_PORTS,
					rio_em_d_log);
		} else {
			out_parms->other_events = true;
		}
	}

	// Check for Initializatio (I2C Register loading) failure

	rc = DARRegRead(dev_info, TSI578_I2C_INT_STAT, &regData);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = GET_INT_STAT(0x21);
		goto exit;
	}

	if (regData & TSI578_I2C_INT_STAT_BL_FAIL) {
		rc = DARRegRead(dev_info, TSI578_I2C_INT_ENABLE, &regData);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_INT_STAT(0x22);
			goto exit;
		}
		if (regData & TSI578_I2C_INT_ENABLE_BL_FAIL) {
			rio_em_add_int_event(in_parms, out_parms, RIO_ALL_PORTS,
					rio_em_i_init_fail);
		} else {
			out_parms->other_events = true;
		}
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_em_get_pw_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_pw_stat_in_t *in_parms,
		rio_em_get_pw_stat_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t pnum;
	uint8_t idx;
	uint32_t spx_int_stat, spx_err_stat, spx_err_det, spx_rate_en,
			spx_ctl_indep, spx_dloop, start_idx;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	uint32_t log_err_det, log_err_en;
	struct DAR_ptl good_ptl;
	bool check;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->num_events = 0;
	out_parms->too_many = false;
	out_parms->other_events = false;

	check = (!in_parms->num_events);
	check |= (in_parms->num_events > EM_MAX_EVENT_LIST_SIZE);
	check |= (in_parms->pw_port_num >= TSI57X_NUM_PORTS(dev_info))
			&& (RIO_ALL_PORTS != in_parms->pw_port_num);
	if (check || (NULL == in_parms->events)) {
		out_parms->imp_rc = GET_PW_STAT(1);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = GET_PW_STAT(2);
		goto exit;
	}

	cfg_in.ptl = good_ptl;

	rc = tsi57x_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto exit;
	}

	if (!cfg_out.num_ports) {
		rc = RIO_ERR_NO_PORT_AVAIL;
		out_parms->imp_rc = GET_PW_STAT(4);
		goto exit;
	}

	// As the very first thing, check for a logical layer error.
	rc = DARRegRead(dev_info, TSI578_RIO_LOG_ERR_DET, &log_err_det);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = GET_PW_STAT(0x10);
		goto exit;
	}
	rc = DARRegRead(dev_info, TSI578_RIO_LOG_ERR_DET_EN, &log_err_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = GET_PW_STAT(0x11);
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
	for (idx = 0; idx < cfg_out.num_ports; idx++) {
		if (!(cfg_out.pc[idx].port_available
				&& cfg_out.pc[idx].powered_up))
			continue;

		start_idx = out_parms->num_events;
		pnum = cfg_out.pc[idx].pnum;

		if (pnum == in_parms->pw_port_num)
			continue;

		rc = DARRegRead(dev_info,
				TSI578_SMACX_DLOOP_CLK_SEL(pnum & 0xFE),
				&spx_dloop);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_PW_STAT(4);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_CTL_INDEP(pnum),
				&spx_ctl_indep);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_PW_STAT(5);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_INT_STATUS(pnum),
				&spx_int_stat);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_PW_STAT(6);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_ERR_STATUS(pnum),
				&spx_err_stat);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_PW_STAT(7);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_ERR_DET(pnum),
				&spx_err_det);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_PW_STAT(8);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_RATE_EN(pnum),
				&spx_rate_en);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = GET_PW_STAT(8);
			goto exit;
		}

		// Loss of signal events are a bit flakey, especially for rapid error detection.
		// Loss of Signal events only occur when the dead link timer has been enabled.
		// If a delineation error occurs and and "Output Fail" condition occurs as well,
		//   this indicates that rapid error detection has probably been invoked.
		// Otherwise, a loss of signal (DLT) event has probably occurred when a PORT_ERR
		//   is detected without link response timeouts or illegal ackIDs in the link response.
		// Neither of these conditions is deterministic.
		if (((spx_err_det & TSI578_SPX_ERR_DET_DELIN_ERR)
				&& (spx_err_stat
						& TSI578_SPX_ERR_STATUS_OUTPUT_FAIL))
				|| ((spx_err_stat
						& TSI578_SPX_ERR_STATUS_PORT_ERR)
						&& !(spx_err_det
								& (TSI578_SPX_ERR_DET_LINK_TO
										| TSI578_SPX_ERR_DET_LR_ACKID_ILL)))) {
			if (spx_dloop & TSI578_SMACX_DLOOP_CLK_SEL_DLT_EN) {
				rio_em_add_pw_event(in_parms, out_parms, pnum,
						rio_em_f_los);
			} else {
				out_parms->other_events = true;
			}
		}

		if (spx_err_stat & TSI578_SPX_ERR_STATUS_OUTPUT_FAIL) {
			uint32_t errs = spx_err_det & spx_rate_en;

			if (TSI578_SPX_ERR_DET_CS_NOT_ACC & errs) {
				rio_em_add_pw_event(in_parms, out_parms, pnum,
						rio_em_f_2many_pna);
			}
			if (~EM_ERR_RATE_EVENT_EXCLUSIONS & errs) {
				rio_em_add_pw_event(in_parms, out_parms, pnum,
						rio_em_f_err_rate);
			}

			if (spx_err_det & ~spx_rate_en) {
				out_parms->other_events = true;
			}
		}

		// Note: When the MAC is configured in two 1x mode, when PORT_ERR is set on the
		// odd MAC it is also set on the even MAC.
		//
		// PORT_ERR only occurs when there are too many timeouts or link-responses with
		// illegal ackIDs.  If neither of these errors is present, do not report the PORT_ERR.
		// There is a special little hook to ensure that PORT_ERR is cleared on the EVEN MAC.

		if ((spx_err_stat & TSI578_SPX_ERR_STATUS_PORT_ERR)
				&& (spx_err_det
						& (TSI578_SPX_ERR_DET_LINK_TO
								| TSI578_SPX_ERR_DET_LR_ACKID_ILL))) {
			if (spx_ctl_indep & TSI578_SPX_CTL_INDEP_PORT_ERR_EN) {
				rio_em_add_pw_event(in_parms, out_parms, pnum,
						rio_em_f_port_err);
			} else {
				out_parms->other_events = true;
			}
		}

		if (spx_int_stat & TSI578_SPX_INT_STATUS_MAX_RETRY) {
			if (spx_ctl_indep & TSI578_SPX_CTL_INDEP_MAX_RETRY_EN) {
				rio_em_add_pw_event(in_parms, out_parms, pnum,
						rio_em_f_2many_retx);
			} else {
				out_parms->other_events = true;
			}
		}

		if (spx_int_stat & TSI578_SPX_INT_STATUS_ILL_TRANS_ERR) {
			if (spx_ctl_indep & TSI578_SPX_CTL_INDEP_ILL_TRANS_ERR) {
				rio_em_add_pw_event(in_parms, out_parms, pnum,
						rio_em_d_rte);
			} else {
				out_parms->other_events = true;
			}
		}

		if (spx_int_stat
				& TSI578_SPX_INT_STATUS_LINK_INIT_NOTIFICATION) {
			if (spx_ctl_indep
					& TSI578_SPX_CTL_INDEP_LINK_INIT_NOTIFICATION_EN) {
				rio_em_add_pw_event(in_parms, out_parms, pnum,
						rio_em_i_sig_det);
			} else {
				out_parms->other_events = true;
			}
		}

		// NOTE: this must be the last per-port event added to the list.
		// If not, it is possible that port-write clearing will not operate as
		// expected.
		//
		// Note: Clear Port-write-pending whether or not there are events
		// reported by the port.
		if ((start_idx != out_parms->num_events)
				|| (spx_err_stat
						& TSI578_SPX_ERR_STATUS_PORT_W_PEND)) {
			rio_em_add_pw_event(in_parms, out_parms, pnum,
					rio_em_a_clr_pwpnd);
		}

		// Reset request cannot trigger a port write.
		// I2C bootload failure cannot trigger a port write.
	}

	if ((RIO_ALL_PORTS != in_parms->pw_port_num)
			&& (!out_parms->too_many)) {
		// Recursively request events for the last port.
		// Note that if successfull, this will detect a logical layer error.
		rio_em_get_pw_stat_in_t last_in_parms;
		rio_em_get_pw_stat_out_t last_out_parms;

		last_in_parms.ptl.num_ports = 1;
		last_in_parms.ptl.pnums[0] = in_parms->pw_port_num;
		last_in_parms.pw_port_num = RIO_ALL_PORTS;
		last_in_parms.num_events = in_parms->num_events
				- out_parms->num_events;
		last_in_parms.events =
				&(in_parms->events[out_parms->num_events]);

		rc = tsi57x_rio_em_get_pw_stat(dev_info, &last_in_parms,
				&last_out_parms);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = last_out_parms.imp_rc;
			goto exit;
		}

		if (last_out_parms.num_events) {
			out_parms->num_events += last_out_parms.num_events;
			out_parms->too_many = last_out_parms.too_many;
			out_parms->other_events |= last_out_parms.other_events;
		}
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_em_clr_events(DAR_DEV_INFO_t *dev_info,
		rio_em_clr_events_in_t *in_parms,
		rio_em_clr_events_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	int pnum;
	int idx;
	bool clear_port_fail = false;
	bool check;
	uint32_t regData;

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

	for (idx = 0; idx < in_parms->num_events; idx++) {
		out_parms->failure_idx = idx;
		pnum = in_parms->events[idx].port_num;
		// Complex statement below enforces
		// - valid port number values
		// - RIO_ALL_PORTS cannot be used with any other events.
		// - RIO_ALL_PORTS must be used with rio_em_d_log and rio_em_i_init_fail.
		// - valid event values
		check = (pnum >= TSI57X_NUM_PORTS(dev_info))
				&& (RIO_ALL_PORTS != pnum);
		check |= (RIO_ALL_PORTS == pnum)
				&& !((rio_em_d_log == in_parms->events[idx].event)
						|| (rio_em_i_init_fail == in_parms->events[idx].event));
		check  |= ((rio_em_d_log == in_parms->events[idx].event)
				|| (rio_em_i_init_fail == in_parms->events[idx].event))
				&& !(RIO_ALL_PORTS == pnum);
		if (check || (rio_em_last <= in_parms->events[idx].event)) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = EM_CLR_EVENTS(2);
			goto exit;
		}

		switch (in_parms->events[idx].event) {
		case rio_em_f_los: // Clear DELIN_ERR
			rc = DARRegRead(dev_info, TSI578_SPX_ERR_DET(pnum),
					&regData);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(3);
				goto exit;
			}

			regData &= ~( TSI578_SPX_ERR_DET_DELIN_ERR);

			rc = DARRegWrite(dev_info, TSI578_SPX_ERR_DET(pnum),
					regData);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(4);
				goto exit;
			}
			clear_port_fail = true;
			break;

		case rio_em_f_port_err: // PORT_ERR occurs due to multiple causes,
			clear_port_fail = true;
			break;

		case rio_em_f_2many_retx:
			//  Clear MAX_RETRY and IMP_SPEC_ERR events
			rc = DARRegWrite(dev_info,
					TSI578_SPX_INT_STATUS(pnum),
					TSI578_SPX_INT_STATUS_MAX_RETRY);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x10);
				goto exit;
			}

			rc = DARRegRead(dev_info, TSI578_SPX_ERR_DET(pnum),
					&regData);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x11);
				goto exit;
			}

			regData &= ~(TSI578_SPX_RATE_EN_IMP_SPEC_ERR);
			rc = DARRegWrite(dev_info, TSI578_SPX_ERR_DET(pnum),
					regData);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x12);
				goto exit;
			}
			clear_port_fail = true;
			break;

		case rio_em_i_init_fail:
			rc = DARRegWrite(dev_info, TSI578_I2C_INT_STAT,
					TSI578_I2C_INT_STAT_BL_FAIL);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x13);
				goto exit;
			}
			break;

		case rio_em_d_ttl:
		case rio_em_a_no_event:
			// Do nothing
			break;

		case rio_em_d_rte:
			rc = DARRegWrite(dev_info,
					TSI578_SPX_INT_STATUS(pnum),
					TSI578_SPX_INT_STATUS_ILL_TRANS_ERR);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x20);
				goto exit;
			}
			break;

		case rio_em_f_2many_pna:
			rc = DARRegRead(dev_info, TSI578_SPX_ERR_DET(pnum),
					&regData);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x22);
				goto exit;
			}

			regData &= ~TSI578_SPX_ERR_DET_CS_NOT_ACC;
			rc = DARRegWrite(dev_info, TSI578_SPX_ERR_DET(pnum),
					regData);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x23);
				goto exit;
			}
			clear_port_fail = true;
			break;

		case rio_em_f_err_rate:
			// Clear all events that contribute to the fatal error rate event
			rc = DARRegRead(dev_info, TSI578_SPX_ERR_DET(pnum),
					&regData);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x24);
				goto exit;
			}

			regData &= EM_ERR_RATE_EVENT_EXCLUSIONS;
			rc = DARRegWrite(dev_info, TSI578_SPX_ERR_DET(pnum),
					regData);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x25);
				goto exit;
			}
			clear_port_fail = true;
			break;

		case rio_em_d_log: // Clear all logical layer errors, must write 0
				   // Register will not clear if the value written is != 0.
			rc = DARRegWrite(dev_info, TSI578_RIO_LOG_ERR_DET, 0);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x26);
				goto exit;
			}
			break;

		case rio_em_i_sig_det:
			rc =
					DARRegWrite(dev_info,
							TSI578_SPX_INT_STATUS(
									pnum),
							TSI578_SPX_INT_STATUS_LINK_INIT_NOTIFICATION);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x28);
				goto exit;
			}
			break;

		case rio_em_i_rst_req:
			rc = DARRegWrite(dev_info, TSI578_SPX_ACKID_STAT(pnum),
					TSI578_SPX_ACKID_STAT_CLR_PKTS);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x2A);
				goto exit;
			}

			rc = DARRegWrite(dev_info,
					TSI578_SPX_CS_INT_STATUS(pnum),
					TSI578_SPX_CS_INT_STATUS_RCS);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x2B);
				goto exit;
			}
			break;

		case rio_em_a_clr_pwpnd:
			rc = DARRegRead(dev_info, TSI578_SPX_ERR_STATUS(pnum),
					&regData);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x2C);
				goto exit;
			}

			regData |= TSI578_SPX_ERR_STATUS_PORT_W_PEND;
			rc = DARRegWrite(dev_info,
					TSI578_SPX_ERR_STATUS(pnum),
					regData);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x2E);
				goto exit;
			}
			break;

		default:
			out_parms->imp_rc = EM_CLR_EVENTS(0x2F);
			rc = RIO_ERR_INVALID_PARAMETER;
			goto exit;
			break;
		}

		if (clear_port_fail) {
			// Clear AckID tracking register
			rc = DARRegWrite(dev_info, TSI578_SPX_ACKID_STAT(pnum),
					TSI578_SPX_ACKID_STAT_CLR_PKTS);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x30);
				goto exit;
			}

			// Clear PORT_FAIL counter
			rc = DARRegRead(dev_info, TSI578_SPX_ERR_RATE(pnum),
					&regData);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x31);
				goto exit;
			}

			// Clear counter and peak value, retain RR & RB settings
			regData &= ~(TSI578_SPX_ERR_RATE_ERR_RATE_CNT
					| TSI578_SPX_ERR_RATE_PEAK);

			rc = DARRegWrite(dev_info, TSI578_SPX_ERR_RATE(pnum),
					regData);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x32);
				goto exit;
			}

			clear_port_fail = false;
			rc = DARRegRead(dev_info, TSI578_SPX_ERR_STATUS(pnum),
					&regData);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x33);
				goto exit;
			}

			// Clear all error conditions, except the port write pending indication
			rc =
					DARRegWrite(dev_info,
							TSI578_SPX_ERR_STATUS(
									pnum),
							regData
									& ~TSI578_SPX_ERR_STATUS_PORT_W_PEND);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x34);
				goto exit;
			}
		}
	}

	out_parms->failure_idx = EM_MAX_EVENT_LIST_SIZE;

exit:
	return rc;
}

uint32_t tsi57x_rio_em_create_events(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_in_t *in_parms,
		rio_em_create_events_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t regVal;
	uint8_t pnum;
	uint8_t idx;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	bool check;

	out_parms->failure_idx = 0;
	out_parms->imp_rc = RIO_SUCCESS;

	if ((!in_parms->num_events)
			|| (in_parms->num_events > EM_MAX_EVENT_LIST_SIZE)
			|| (NULL == in_parms->events)) {
		out_parms->imp_rc = CREATE_EVENTS(1);
		goto exit;
	}

	cfg_in.ptl.num_ports = RIO_ALL_PORTS;

	rc = tsi57x_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto exit;
	}

	if (!cfg_out.num_ports) {
		rc = RIO_ERR_NO_PORT_AVAIL;
		out_parms->imp_rc = CREATE_EVENTS(3);
		goto exit;
	}

	for (idx = 0; idx < in_parms->num_events; idx++) {
		out_parms->failure_idx = idx;
		pnum = in_parms->events[idx].port_num;

		check = (pnum >= TSI57X_NUM_PORTS(dev_info))
				&& (RIO_ALL_PORTS != pnum);
		check |= (RIO_ALL_PORTS == pnum)
				&& !((rio_em_d_log == in_parms->events[idx].event)
						|| (rio_em_i_init_fail == in_parms->events[idx].event));
		check |= ((rio_em_d_log == in_parms->events[idx].event)
				|| (rio_em_i_init_fail == in_parms->events[idx].event))
				&& !(RIO_ALL_PORTS == pnum);
		if (check || (rio_em_last <= in_parms->events[idx].event)) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = CREATE_EVENTS(2);
			goto exit;
		}

		if ((RIO_ALL_PORTS != pnum)
				&& (!(cfg_out.pc[pnum].port_available
						&& cfg_out.pc[pnum].powered_up))) {
			continue;
		}

		switch (in_parms->events[idx].event) {
		case rio_em_f_los:
			// LOS is equal to a DELIN_ERR
			rc = DARRegRead(dev_info, TSI578_SPX_ERR_DET(pnum),
					&regVal);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = CREATE_EVENTS(0x10);
				goto exit;
			}
			rc = DARRegWrite(dev_info, TSI578_SPX_ERR_DET(pnum),
					regVal | TSI578_SPX_ERR_DET_DELIN_ERR);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = CREATE_EVENTS(0x11);
				goto exit;
			}
			break;

		case rio_em_f_port_err:  // Can't create a PORT_ERR
			rc = RIO_ERR_FEATURE_NOT_SUPPORTED;
			out_parms->imp_rc = CREATE_EVENTS(0x12);
			goto exit;
			break;

		case rio_em_f_2many_retx:
			rc = DARRegRead(dev_info, TSI578_SPX_INT_STATUS(pnum),
					&regVal);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = CREATE_EVENTS(0x13);
				goto exit;
			}
			rc =
					DARRegWrite(dev_info,
							TSI578_SPX_INT_GEN(
									pnum),
							regVal
									| TSI578_SPX_INT_GEN_MAX_RETRY_GEN);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = CREATE_EVENTS(0x14);
				goto exit;
			}
			break;

		case rio_em_f_2many_pna:
			rc = tsi57x_create_rate_event(dev_info, pnum,
					TSI578_SPX_RATE_EN_CS_NOT_ACC_EN,
					&out_parms->imp_rc);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			break;

		case rio_em_f_err_rate:
			rc = tsi57x_create_rate_event(dev_info, pnum,
					~EM_ERR_RATE_EVENT_EXCLUSIONS,
					&out_parms->imp_rc);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			break;

		case rio_em_d_ttl: // Do nothing
			rc = RIO_ERR_FEATURE_NOT_SUPPORTED;
			out_parms->imp_rc = CREATE_EVENTS(0x21);
			goto exit;
			break;

		case rio_em_d_rte:
			rc = DARRegRead(dev_info, TSI578_SPX_INT_STATUS(pnum),
					&regVal);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = CREATE_EVENTS(0x22);
				goto exit;
			}
			rc =
					DARRegWrite(dev_info,
							TSI578_SPX_INT_GEN(
									pnum),
							regVal
									| TSI578_SPX_INT_GEN_ILL_TRANS_GEN);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = CREATE_EVENTS(0x23);
				goto exit;
			}
			break;

		case rio_em_d_log: // Set all logical layer errors
			rc = DARRegWrite(dev_info, TSI578_RIO_LOG_ERR_DET,
					TSI57X_ALL_LOG_ERRS);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = CREATE_EVENTS(0x30);
				goto exit;
			}
			break;

		case rio_em_i_sig_det:
			rc = DARRegRead(dev_info, TSI578_SPX_INT_STATUS(pnum),
					&regVal);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = CREATE_EVENTS(0x31);
				goto exit;
			}
			rc =
					DARRegWrite(dev_info,
							TSI578_SPX_INT_GEN(
									pnum),
							regVal
									| TSI578_SPX_INT_GEN_LINK_INIT_NOTIFICATION_GEN);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = CREATE_EVENTS(0x32);
				goto exit;
			}
			break;

		case rio_em_i_rst_req:
			rc = RIO_ERR_FEATURE_NOT_SUPPORTED;
			out_parms->imp_rc = CREATE_EVENTS(0x33);
			goto exit;
			break;

		case rio_em_i_init_fail:
			rc = DARRegWrite(dev_info, TSI578_I2C_INT_SET,
					TSI578_I2C_INT_SET_BL_FAIL);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = CREATE_EVENTS(0x20);
				goto exit;
			}
			break;

		default:
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = CREATE_EVENTS(0x34);
			goto exit;
			break;
		}
	}

	out_parms->failure_idx = EM_MAX_EVENT_LIST_SIZE;

exit:
	return rc;
}

#endif /* TSI57X_DAR_WANTED */

#ifdef __cplusplus
}
#endif
