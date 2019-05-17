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

#include "CPS_DeviceDriver.h"
#include "RXS_DeviceDriver.h"
#include "Tsi721_DeviceDriver.h"
#include "Tsi57x_DeviceDriver.h"
#include "DSF_DB_Private.h"

#ifdef __cplusplus
extern "C" {
#endif

char *rio_em_events_names[rio_em_last] = {
		(char *)"FLossOfSig",
		(char *)"FPortErr",
		(char *)"F2ManyReTx",
		(char *)"F2ManyPNA",
		(char *)"FErrRate",
		(char *)"DropTTL",
		(char *)"DropOnPurp",
		(char *)"DropLogErr",
		(char *)"ISigDet",
		(char *)"IRstReq",
		(char *)"IInitFail",
		(char *)"AClrPwPnd",
		(char *)"ANoEvent",
};

char *rio_em_notfn_names[rio_em_notfn_last] = {
		(char *)"NtfnNone",
		(char *)"NtfnInt",
		(char *)"NtfnPW",
		(char *)"NtfnBoth",
		(char *)"NtfnNoChg",
};

char *rio_em_detect_names[rio_em_detect_last] = {
		(char *)"DetOff",
		(char *)"DetOn",
		(char *)"DetNoChg",
};

#define RIO_EM_REC_ERR_SET_LINK_TO 		((uint32_t)(0x0001))
#define RIO_EM_REC_ERR_SET_CS_ACK_ILL		((uint32_t)(0x0002))
#define RIO_EM_REC_ERR_SET_DELIN_ERR		((uint32_t)(0x0004))
#define RIO_EM_REC_ERR_SET_PROT_ERR		((uint32_t)(0x0008))
#define RIO_EM_REC_ERR_SET_LR_ACKID_ILL		((uint32_t)(0x0010))
#define RIO_EM_REC_ERR_SET_PKT_ILL_SIZE		((uint32_t)(0x0020))
#define RIO_EM_REC_ERR_SET_PKT_CRC_ERR		((uint32_t)(0x0040))
#define RIO_EM_REC_ERR_SET_PKT_ILL_ACKID	((uint32_t)(0x0080))
#define RIO_EM_REC_ERR_SET_CS_NOT_ACC		((uint32_t)(0x0100))
#define RIO_EM_REC_ERR_SET_CS_ILL_ID		((uint32_t)(0x0200))
#define RIO_EM_REC_ERR_SET_CS_CRC_ERR		((uint32_t)(0x0400))

typedef struct rec_err_n_spx_err_t_TAG {
	uint32_t rec_err;
	uint32_t spx_err;
} rec_err_n_spx_err_t;

// Convert from standard bit mask encodings to consecutive bit encodings.

rec_err_n_spx_err_t rec_err_spx_err_table[] = {
	{ RIO_EM_REC_ERR_SET_LINK_TO      , RIO_EMHS_SPX_ERR_DET_LINK_TO      },
	{ RIO_EM_REC_ERR_SET_CS_ACK_ILL   , RIO_EMHS_SPX_ERR_DET_CS_ACK_ILL   },
	{ RIO_EM_REC_ERR_SET_DELIN_ERR    , RIO_EMHS_SPX_ERR_DET_DELIN_ERR    },
	{ RIO_EM_REC_ERR_SET_PROT_ERR     , RIO_EMHS_SPX_ERR_DET_PROT_ERR     },
	{ RIO_EM_REC_ERR_SET_LR_ACKID_ILL , RIO_EMHS_SPX_ERR_DET_LR_ACKID_ILL },
	{ RIO_EM_REC_ERR_SET_PKT_ILL_SIZE , RIO_EMHS_SPX_ERR_DET_PKT_ILL_SIZE },
	{ RIO_EM_REC_ERR_SET_PKT_CRC_ERR  , RIO_EMHS_SPX_ERR_DET_PKT_CRC_ERR  },
	{ RIO_EM_REC_ERR_SET_PKT_ILL_ACKID, RIO_EMHS_SPX_ERR_DET_PKT_ILL_ACKID},
	{ RIO_EM_REC_ERR_SET_CS_NOT_ACC   , RIO_EMHS_SPX_ERR_DET_CS_NOT_ACC   },
	{ RIO_EM_REC_ERR_SET_CS_ILL_ID    , RIO_EMHS_SPX_ERR_DET_CS_ILL_ID    },
	{ RIO_EM_REC_ERR_SET_CS_CRC_ERR   , RIO_EMHS_SPX_ERR_DET_CS_CRC_ERR   },
};

const unsigned int rec_err_spx_err_table_max =
	sizeof(rec_err_spx_err_table) / sizeof(rec_err_spx_err_table[0]);

// Encoding for RB, index = RB value
uint32_t rio_spx_err_rate_err_rb_vals[] = {
		RIO_EMHS_SPX_RATE_RB_NONE,
		RIO_EMHS_SPX_RATE_RB_1_MS,
		RIO_EMHS_SPX_RATE_RB_10_MS,
		RIO_EMHS_SPX_RATE_RB_100_MS,
		RIO_EMHS_SPX_RATE_RB_1_SEC,
		RIO_EMHS_SPX_RATE_RB_10_SEC,
		RIO_EMHS_SPX_RATE_RB_100_SEC,
		RIO_EMHS_SPX_RATE_RB_1000_SEC,
		RIO_EMHS_SPX_RATE_RB_10000_SEC
};

const unsigned int rio_spx_err_rate_err_rb_vals_max =
		sizeof(rio_spx_err_rate_err_rb_vals) /
		sizeof(rio_spx_err_rate_err_rb_vals[0]);

#define RB_SHIFT_AMT 20

void rio_em_add_int_event(rio_em_get_int_stat_in_t *in_parms,
		rio_em_get_int_stat_out_t *out_parms, uint8_t pnum,
		rio_em_events_t event)
{
	if (out_parms->num_events < in_parms->num_events) {
		in_parms->events[out_parms->num_events].event = event;
		in_parms->events[out_parms->num_events].port_num = pnum;
		out_parms->num_events++;
	} else {
		out_parms->too_many = true;
	}
}

void rio_em_add_pw_event(rio_em_get_pw_stat_in_t *in_parms,
		rio_em_get_pw_stat_out_t *out_parms, uint8_t pnum,
		rio_em_events_t event)
{
	if (out_parms->num_events < in_parms->num_events) {
		in_parms->events[out_parms->num_events].event = event;
		in_parms->events[out_parms->num_events].port_num = pnum;
		out_parms->num_events++;
	} else {
		out_parms->too_many = true;
	}
}

// Info is:
// 0x00000FFF - spx_rate_en bits
// 0x00030000 - spx_rate_rr (max counter overage)
// 0x00F00000 - spx_rate_rb (leak rate)
// 0xFF000000 - spx_fail_thresh
// 0x00C0F800 - unused

uint32_t rio_em_compute_f_err_rate_info(uint32_t spx_rate_en,
		uint32_t spx_err_rate, uint32_t spx_err_thresh, uint32_t *info)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t all_errs = 0, idx;
	bool found_one = false;

	*info = 0;

	for (idx = 0; idx < rec_err_spx_err_table_max; idx++) {
		all_errs |= rec_err_spx_err_table[idx].spx_err;
		if (rec_err_spx_err_table[idx].spx_err & spx_rate_en) {
			*info |= rec_err_spx_err_table[idx].rec_err;
		}
	}

	if (spx_rate_en & ~all_errs) {
		goto exit;
	}

	*info |= (RIO_EMHS_SPX_RATE_RR & spx_err_rate);
	*info |= (RIO_EMHS_SPX_THRESH_FAIL & spx_err_thresh);
	spx_err_rate &= RIO_EMHS_SPX_RATE_RB;

	for (idx = 0; idx < rio_spx_err_rate_err_rb_vals_max; idx++) {
		if (rio_spx_err_rate_err_rb_vals[idx] == spx_err_rate) {
			found_one = true;
			*info |= (idx << RB_SHIFT_AMT);
			break;
		}
	}

	if (!found_one) {
		goto exit;
	}
	rc = RIO_SUCCESS;

exit:
	return rc;
}

uint32_t rio_em_get_f_err_rate_info(uint32_t info, uint32_t *spx_rate_en,
		uint32_t *spx_err_rate, uint32_t *spx_err_thresh)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t idx;

	*spx_rate_en = 0;
	*spx_err_rate = info & RIO_EMHS_SPX_RATE_RR;
	*spx_err_thresh = info & RIO_EMHS_SPX_THRESH_FAIL;

	for (idx = 0; idx < rec_err_spx_err_table_max; idx++) {
		if (rec_err_spx_err_table[idx].rec_err & info) {
			*spx_rate_en |= rec_err_spx_err_table[idx].spx_err;
		}
	}

	info = (info & 0x00F00000) >> RB_SHIFT_AMT;
	if (info >= rio_spx_err_rate_err_rb_vals_max) {
		goto exit;
	}

	*spx_err_rate |= rio_spx_err_rate_err_rb_vals[info];

	rc = RIO_SUCCESS;
exit:
	return rc;
}

// Routines to configure port write transmission, 
// and set/query specific event detection/reporting.

uint32_t rio_em_cfg_pw(DAR_DEV_INFO_t *dev_info, rio_em_cfg_pw_in_t *in_parms,
		rio_em_cfg_pw_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_em_cfg_pw(dev_info, in_parms, out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_em_cfg_pw(dev_info, in_parms, out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_em_cfg_pw(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_em_cfg_pw(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_em_cfg_pw(dev_info, in_parms, out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_em_cfg_set(DAR_DEV_INFO_t *dev_info, rio_em_cfg_set_in_t *in_parms,
		rio_em_cfg_set_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_em_cfg_set(dev_info, in_parms, out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_em_cfg_set(dev_info, in_parms, out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_em_cfg_set(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_em_cfg_set(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_em_cfg_set(dev_info, in_parms, out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_em_cfg_get(DAR_DEV_INFO_t *dev_info, rio_em_cfg_get_in_t *in_parms,
		rio_em_cfg_get_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_em_cfg_get(dev_info, in_parms, out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_em_cfg_get(dev_info, in_parms, out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_em_cfg_get(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_em_cfg_get(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_em_cfg_get(dev_info, in_parms, out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

// Routines to query and control port-write and interrupt
// reporting configuration for a port/device.

uint32_t rio_em_dev_rpt_ctl(DAR_DEV_INFO_t *dev_info,
		rio_em_dev_rpt_ctl_in_t *in_parms,
		rio_em_dev_rpt_ctl_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_em_dev_rpt_ctl(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_em_dev_rpt_ctl(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_em_dev_rpt_ctl(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_em_dev_rpt_ctl(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_em_dev_rpt_ctl(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

// Routines to convert port-write contents into an event list,
// and to query a port/device and return a list of asserted events 
// which are reported via interrupt or port-write, 

uint32_t rio_em_parse_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_parse_pw_in_t *in_parms,
		rio_em_parse_pw_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_em_parse_pw(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_em_parse_pw(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_em_parse_pw(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_em_parse_pw(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_em_parse_pw(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:off
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_em_get_int_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_int_stat_in_t *in_parms,
		rio_em_get_int_stat_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_em_get_int_stat(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_em_get_int_stat(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_em_get_int_stat(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_em_get_int_stat(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_em_get_int_stat(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_em_get_pw_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_pw_stat_in_t *in_parms,
		rio_em_get_pw_stat_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_em_get_pw_stat(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_em_get_pw_stat(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_em_get_pw_stat(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_em_get_pw_stat(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_em_get_pw_stat(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

// Routine to clear events, and a routine to create events
// for software testing purposes.

uint32_t rio_em_clr_events(DAR_DEV_INFO_t *dev_info,
		rio_em_clr_events_in_t *in_parms,
		rio_em_clr_events_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_em_clr_events(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_em_clr_events(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_em_clr_events(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_em_clr_events(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_em_clr_events(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_em_create_events(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_in_t *in_parms,
		rio_em_create_events_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_em_create_events(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_em_create_events(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_em_create_events(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_em_create_events(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_em_create_events(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

#ifdef __cplusplus
}
#endif
