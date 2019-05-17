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
#include <string.h>

#include "RapidIO_Driver_Utilities.h"

#ifdef __cplusplus
extern "C" {
#endif

void rio_determine_op_fc(rio_pc_fc *fc, uint32_t errstat)
{
	*fc = rio_pc_fc_last;
	if (errstat & RIO_SPX_ERR_STAT_OK) {
		*fc = (errstat & RIO_SPX_ERR_STAT_TXFC) ? rio_pc_fc_tx :
							rio_pc_fc_rx;
	}
}

void rio_determine_op_iseq(rio_pc_idle_seq *iseq, uint32_t errstat)
{
	*iseq = rio_pc_is_last;
	if (errstat & RIO_SPX_ERR_STAT_OK) {
		switch (errstat & RIO_SPX_ERR_STAT_IDLE_SEQ) {
		case RIO_SPX_ERR_STAT_IDLE_SEQ1:
			*iseq = rio_pc_is_one;
			break;
		case RIO_SPX_ERR_STAT_IDLE_SEQ2:
			*iseq = rio_pc_is_two;
			break;
		case RIO_SPX_ERR_STAT_IDLE_SEQ3:
			*iseq = rio_pc_is_three;
			break;
		default:
			break;
		}
	}
}

typedef struct spx_ctl2_ls_check_info_t_TAG {
	uint32_t ls_en;
	uint32_t ls_sup;
	rio_pc_ls_t ls;
} spx_ctl2_ls_check_info_t;

spx_ctl2_ls_check_info_t rio_ls_check[] = {
	{ RIO_SPX_CTL2_GB_1P25_EN , RIO_SPX_CTL2_GB_1P25 , rio_pc_ls_1p25},
	{ RIO_SPX_CTL2_GB_2P5_EN  , RIO_SPX_CTL2_GB_2P5  , rio_pc_ls_2p5 },
	{ RIO_SPX_CTL2_GB_3P125_EN, RIO_SPX_CTL2_GB_3P125, rio_pc_ls_3p125},
	{ RIO_SPX_CTL2_GB_5P0_EN  , RIO_SPX_CTL2_GB_5P0  , rio_pc_ls_5p0  },
	{ RIO_SPX_CTL2_GB_6P25_EN , RIO_SPX_CTL2_GB_6P25 , rio_pc_ls_6p25 },
	{ RIO_SPX_CTL2_GB_10P3_EN , RIO_SPX_CTL2_GB_10P3 , rio_pc_ls_10p3 },
	{ RIO_SPX_CTL2_GB_12P5_EN , RIO_SPX_CTL2_GB_12P5 , rio_pc_ls_12p5 },
	{ 0x00000000		, 0x00000000		, rio_pc_ls_last },
};

void rio_determine_ls(rio_pc_ls_t *ls, uint32_t ctl2)
{
	uint32_t idx;

	*ls = rio_pc_ls_last;

	for (idx = 0; rio_ls_check[idx].ls_en; idx++) {
		// If speed is not supported, continue
		if (!(rio_ls_check[idx].ls_sup & ctl2)) {
			continue;
		}
		// If speed is not enabled, continue
		if (!(rio_ls_check[idx].ls_en & ctl2)) {
			continue;
		}
		// More than one speed supported & enabled,
		// programming error!
		if (rio_pc_ls_last != *ls) {
			*ls = rio_pc_ls_last;
			break;
		}
		*ls = rio_ls_check[idx].ls;
	}
}

void rio_determine_cfg_pw(rio_pc_pw_t *pw, uint32_t ctl)
{
	switch (ctl & RIO_SPX_CTL_PTW_OVER) {
	case RIO_SPX_CTL_PTW_OVER_RSVD:
	case RIO_SPX_CTL_PTW_OVER_IMP_SPEC:
		*pw = rio_pc_pw_last;
		break;
	case RIO_SPX_CTL_PTW_OVER_4X_NO_2X:
		if (ctl & RIO_SPX_CTL_PTW_MAX_4X) {
			*pw = rio_pc_pw_4x;
		} else {
			*pw = rio_pc_pw_1x;
		}
		break;
	case RIO_SPX_CTL_PTW_OVER_NONE_2:
	case RIO_SPX_CTL_PTW_OVER_NONE:
		if (ctl & RIO_SPX_CTL_PTW_MAX_4X) {
			*pw = rio_pc_pw_4x;
		} else if (ctl & RIO_SPX_CTL_PTW_MAX_2X) {
			*pw = rio_pc_pw_2x;
		} else {
			*pw = rio_pc_pw_1x;
		}
		break;
	case RIO_SPX_CTL_PTW_OVER_1X_L0:
		if ((ctl & RIO_SPX_CTL_PTW_MAX_4X) ||
				(ctl & RIO_SPX_CTL_PTW_MAX_2X)) {
			*pw = rio_pc_pw_1x_l0;
		} else {
			*pw = rio_pc_pw_1x;
		}
		break;
	case RIO_SPX_CTL_PTW_OVER_1X_LR:
		if (ctl & RIO_SPX_CTL_PTW_MAX_4X) {
			*pw = rio_pc_pw_1x_l2;
		} else if (ctl & RIO_SPX_CTL_PTW_MAX_2X) {
			*pw = rio_pc_pw_1x_l1;
		} else {
			// Override to a redundant lane,
			// When the port only supports 1x.
			*pw = rio_pc_pw_last;
		}
		break;
	case RIO_SPX_CTL_PTW_OVER_2X_NO_4X:
		if (ctl & RIO_SPX_CTL_PTW_MAX_2X) {
			*pw = rio_pc_pw_2x;
		} else {
			// Override to 2x, but 2x is not available.
			// Operate as a 1x port.
			*pw = rio_pc_pw_1x;
		}
		break;
	default:
		*pw = rio_pc_pw_last;
	}
}

void rio_determine_op_pw(rio_pc_pw_t *pw, uint32_t ctl, uint32_t errstat)
{
	rio_pc_pw_t cfg_pw;

	// If the link has not initialized, the operating
	// port width is unknown.
	if (!(errstat & RIO_SPX_ERR_STAT_OK)) {
		*pw = rio_pc_pw_last;
		return;
	}

	rio_determine_cfg_pw(&cfg_pw, ctl);

	*pw = rio_pc_pw_last;
	switch (ctl & RIO_SPX_CTL_PTW_INIT) {
	case RIO_SPX_CTL_PTW_INIT_4X:
		if (rio_pc_pw_4x == cfg_pw) {
			*pw = rio_pc_pw_4x;
		} else {
			*pw = rio_pc_pw_last;
		}
		break;
	case RIO_SPX_CTL_PTW_INIT_2X:
		switch(cfg_pw) {
		case rio_pc_pw_2x:
		case rio_pc_pw_4x:
			if (ctl & RIO_SPX_CTL_PTW_MAX_2X) {
				// If 2x mode has been overridden, indicate an
				// error.
				if ((ctl & RIO_SPX_CTL_PTW_OVER) ==
						RIO_SPX_CTL_PTW_OVER_4X_NO_2X) {
					*pw = rio_pc_pw_last;
				} else {
					*pw = rio_pc_pw_2x;
				}
			} else {
				// If 2x mode is not enabled, cannot initizlize
				// to be a 2x port.
				*pw = rio_pc_pw_last;
			}
			break;
		case rio_pc_pw_1x:
		case rio_pc_pw_1x_l0:
		case rio_pc_pw_1x_l1:
		case rio_pc_pw_1x_l2:
		default:
			// Should never initialize to 2x when configured
			// to use either lane 0, or a redundant lane.
			*pw = rio_pc_pw_last;
			break;
		}
		break;
	case RIO_SPX_CTL_PTW_INIT_1X_L0:
		switch(cfg_pw) {
		case rio_pc_pw_1x:
			*pw = rio_pc_pw_1x;
			break;
		case rio_pc_pw_1x_l0:
			// If either 2x or 4x is enabled,
			// indicate that a redundant lane has trained.
			if ((ctl & RIO_SPX_CTL_PTW_MAX_2X) ||
				 	(ctl & RIO_SPX_CTL_PTW_MAX_4X)) {
				*pw = rio_pc_pw_1x_l0;
			} else {
				*pw = rio_pc_pw_1x;
			}
			break;
		case rio_pc_pw_2x:
		case rio_pc_pw_4x:
		case rio_pc_pw_1x_l1:
		case rio_pc_pw_1x_l2:
			*pw = rio_pc_pw_1x_l0;
			break;
		default:
			break;
		}
		break;
	case RIO_SPX_CTL_PTW_INIT_1X_LR:
		switch(cfg_pw) {
		case rio_pc_pw_1x:
			// Should never have a port trained as 1x_LR
			// that is configured as a 1x port...
			*pw = rio_pc_pw_last;
			break;
		case rio_pc_pw_2x:
		case rio_pc_pw_1x_l1:
			*pw = rio_pc_pw_1x_l1;
			break;
		case rio_pc_pw_4x:
		case rio_pc_pw_1x_l2:
			*pw = rio_pc_pw_1x_l2;
			break;
		case rio_pc_pw_1x_l0:
			// Trained on redundant lane, but configured to use
			// lane 0.  Figure out if port is 4x or 2x, and
			// choose the lane accordingly.
			if (ctl & RIO_SPX_CTL_PTW_MAX_4X) {
				*pw = rio_pc_pw_1x_l2;
			} else if (ctl & RIO_SPX_CTL_PTW_MAX_2X) {
				*pw = rio_pc_pw_1x_l1;
			} else {
				// Not a 4x or 2x port, but trained on
				// redundant lane.  Indicate an error.
				*pw = rio_pc_pw_last;
			}
			break;
		default:
			break;
		}
		break;
	default:
		*pw = rio_pc_pw_last;
	}
}

#ifdef __cplusplus
}
#endif
