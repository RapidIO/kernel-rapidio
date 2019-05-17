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

#include "RapidIO_Device_Access_Routines_API.h"
#include "RapidIO_Statistics_Counter_API.h"
#include "rio_standard.h"
#include "rio_ecosystem.h"

#ifdef __cplusplus
extern "C" {
#endif

sc_info_t sc_info[(uint8_t)(rio_sc_last) + 2] = {
		{(char *)"Disabled__", 0},
		{(char *)"Enabled___", 0},
		{(char *)"UC_REQ_PKT", SC_F_PKT}, // Tsi57x start
		{(char *)"UC_ALL_PKT", SC_F_PKT},
		{(char *)"Retry___CS", SC_F_CS | SC_F_RTY },
		{(char *)"All_____CS", SC_F_CS },
		{(char *)"UC_4B_Data", SC_F_DATA},
		{(char *)"MCast__PKT", SC_F_PKT},
		{(char *)"MECS____CS", SC_F_CS},
		{(char *)"MC_4B_Data", SC_F_DATA}, // Tsi57x end
		{(char *)"PktAcc__CS", SC_F_CS}, // CPS1848 start
		{(char *)"ALL____PKT", SC_F_PKT},
		{(char *)"PktNotA_CS", SC_F_CS | SC_F_ERR},
		{(char *)"Drop___PKT", SC_F_PKT | SC_F_ERR | SC_F_DROP},
		{(char *)"DropTTLPKT", SC_F_PKT | SC_F_ERR | SC_F_DROP}, // CPS1848 end
		{(char *)"FAB____PKT", SC_F_PKT}, // RXS start
		{(char *)"8B_DAT_PKT", SC_F_DATA},
		{(char *)"8B_DAT_PKT", SC_F_DATA},
		{(char *)"RAW_BWIDTH", 0}, // RXS end
		{(char *)"PCI_M__PKT", SC_F_PKT},
		{(char *)"PCI_M__PKT", SC_F_PKT},
		{(char *)"PCI__D_PKT", SC_F_PKT},
		{(char *)"PCI__D_PKT", SC_F_PKT},
		{(char *)"PCI_BG_PKT", SC_F_PKT},
		{(char *)"PCI_BG_PKT", SC_F_PKT},
		{(char *)"NWR____PKT", SC_F_PKT},
		{(char *)"NWR_OK_PKT", SC_F_PKT},
		{(char *)"DB_____PKT", SC_F_PKT},
		{(char *)"DB__OK_PKT", SC_F_PKT},
		{(char *)"MSG____PKT", SC_F_PKT},
		{(char *)"MSG____PKT", SC_F_PKT},
		{(char *)"MSG_RTYPKT", SC_F_PKT | SC_F_RTY},
		{(char *)"MSG_RTYPKT", SC_F_PKT | SC_F_RTY},
		{(char *)"DMA____PKT", SC_F_PKT},
		{(char *)"DMA____PKT", SC_F_PKT},
		{(char *)"BRG____PKT", SC_F_PKT},
		{(char *)"BRG____PKT", SC_F_PKT},
		{(char *)"BRG_ERRPKT", SC_F_PKT | SC_F_ERR},
		{(char *)"MWR____PKT", SC_F_PKT},
		{(char *)"MWR_OK_PKT", SC_F_PKT},
		{(char *)"Last______", 0},
		{(char *)"Invalid___", 0},
};

char *sc_flag_names[(uint8_t)(sc_f_LAST) + 2] = {
		(char *)"DROP",
		(char *)"ERR",
		(char *)"RTY",
		(char *)"CS",
		(char *)"PKT",
		(char *)"DATA",
		(char *)"Last",
		(char *)"Ivld"
};

const char *sc_other_if_names_PCIe = (char *)"PCIExp";
const char *sc_other_if_names_FABRIC = (char *)"FABRIC";
const char *sc_other_if_names_Invalid = (char *)"INVALID";
const char *sc_other_if_names_UNKNOWN = (char *)"UNKNOWN";

uint32_t rio_sc_other_if_names(DAR_DEV_INFO_t *dev_h, const char **name)
{
	uint32_t rc;

	if ((NULL == dev_h) || (NULL == name)) {
		return RIO_ERR_NULL_PARM_PTR;
	}
	
	if (RIO_UNITIALIZED_DEVICE == dev_h->driver_family) {
		dev_h->driver_family = rio_get_driver_family(dev_h->devID);
	}

	switch (dev_h->driver_family) {
	case RIO_CPS_DEVICE:
	case RIO_RXS_DEVICE:
		*name = sc_other_if_names_FABRIC;
		rc = RIO_SUCCESS;
		break;

	case RIO_TSI721_DEVICE:
		*name = sc_other_if_names_PCIe;
		rc = RIO_SUCCESS;
		break;

	case RIO_TSI57X_DEVICE:
		*name = sc_other_if_names_Invalid;
		rc = RIO_ERR_NO_DEVICE_SUPPORT;
		break;

	default:
		*name = sc_other_if_names_UNKNOWN;
		rc = RIO_ERR_NO_DEVICE_SUPPORT;
	}
	return rc;
}

/* User function calls for a routing table configuration */
uint32_t rio_sc_init_dev_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_init_dev_ctrs_in_t *in_parms,
		rio_sc_init_dev_ctrs_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_sc_init_dev_ctrs(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_sc_init_dev_ctrs(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_sc_init_dev_ctrs(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_sc_init_dev_ctrs(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_sc_init_dev_ctrs(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_sc_read_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_read_ctrs_in_t *in_parms,
		rio_sc_read_ctrs_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_sc_read_ctrs(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_sc_read_ctrs(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return tsi721_rio_sc_read_ctrs(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_sc_read_ctrs(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_sc_read_ctrs(dev_info, in_parms,
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
