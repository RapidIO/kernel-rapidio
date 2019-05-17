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

// Converts rio_pc_pw_t to lane count
int pw_to_lanes[ (int)(rio_pc_pw_last)+1] = {1, 2, 4, 1, 2, 4, 0};

// Converts rio_pc_pw_t to string
char *pw_to_str[(int)(rio_pc_pw_last) + 1] = {
		(char *)"1x",
		(char *)"2x",
		(char *)"4x",
		(char *)"1xL0",
		(char *)"1xL1",
		(char *)"1xL2",
		(char *)"FAIL",
};

// Converts lane count to rio_pc_pw_t
rio_pc_pw_t lanes_to_pw[5] = {
		rio_pc_pw_last, // 0, illegal
		rio_pc_pw_1x, // 1
		rio_pc_pw_2x, // 2
		rio_pc_pw_last, // 3, illegal
		rio_pc_pw_4x, // 4
};

// Converts lane speed to a string
char *ls_to_str[(int)(rio_pc_ls_last) + 1] = {
		(char *)" 1.25 ",
		(char *)" 2.5  ",
		(char *)" 3.125",
		(char *)" 5.0  ",
		(char *)" 6.25 ",
		(char *)"10.313",
		(char *)"12.5  ",
		(char *)"*FAIL*",
};

// Converts reset configuration to a string
char *rst_to_str[(int)(rio_pc_rst_last) + 1] = {
		(char *)"Dev ",
		(char *)"Port",
		(char *)"Int ",
		(char *)"PtWr",
		(char *)"Ignr",
		(char *)"??",
};

char *fc_to_str[(int)(rio_pc_fc_last) + 1] = {
		(char *)"RX",
		(char *)"TX",
		(char *)"??",
};

char *is_to_str[(int)(rio_pc_is_last) + 1] = {
		(char *)" 1",
		(char *)" 2",
		(char *)" 3",
		(char *)"DF",
		(char *)"??",
};

uint32_t rio_pc_get_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_config_in_t *in_parms,
		rio_pc_get_config_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_pc_get_config(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_pc_get_config(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_pc_get_config(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_pc_get_config(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_pc_get_config(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_pc_set_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_set_config_in_t *in_parms,
		rio_pc_set_config_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_pc_set_config(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_pc_set_config(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_pc_set_config(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_pc_set_config(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_pc_set_config(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_pc_get_status(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_status_in_t *in_parms,
		rio_pc_get_status_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_pc_get_status(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_pc_get_status(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_pc_get_status(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_pc_get_status(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_pc_get_status(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_pc_reset_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_port_in_t *in_parms,
		rio_pc_reset_port_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_pc_reset_port(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_pc_reset_port(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_pc_reset_port(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_pc_reset_port(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_pc_reset_port(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_pc_reset_link_partner(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_link_partner_in_t *in_parms,
		rio_pc_reset_link_partner_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_pc_reset_link_partner(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_pc_reset_link_partner(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_pc_reset_link_partner(dev_info,
					in_parms, out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_pc_reset_link_partner(dev_info,
					in_parms, out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_pc_reset_link_partner(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_pc_clr_errs(DAR_DEV_INFO_t *dev_info,
		rio_pc_clr_errs_in_t *in_parms,
		rio_pc_clr_errs_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_pc_clr_errs(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_pc_clr_errs(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_pc_clr_errs(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_pc_clr_errs(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_pc_clr_errs(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_pc_secure_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_secure_port_in_t *in_parms,
		rio_pc_secure_port_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_pc_secure_port(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_pc_secure_port(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_pc_secure_port(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_pc_secure_port(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_pc_secure_port(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_pc_dev_reset_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_dev_reset_config_in_t *in_parms,
		rio_pc_dev_reset_config_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_pc_dev_reset_config(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_pc_dev_reset_config(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_pc_dev_reset_config(dev_info,
					in_parms, out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_pc_dev_reset_config(dev_info,
					in_parms, out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_pc_dev_reset_config(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_pc_probe(DAR_DEV_INFO_t *dev_info, rio_pc_probe_in_t *in_parms,
		rio_pc_probe_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return default_rio_pc_probe(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return default_rio_pc_probe(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return default_rio_pc_probe(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return default_rio_pc_probe(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return default_rio_pc_probe(dev_info, in_parms,
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
