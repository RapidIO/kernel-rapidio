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

#include "rio_ecosystem.h"
#include "string_util.h"

#include "CPS_DeviceDriver.h"
#include "RapidIO_Device_Access_Routines_API.h"

#include "DAR_DB_Private.h"
#include "CPS1848.h"
#include "rio_ecosystem.h"
#include "string_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CPS_DAR_WANTED

uint32_t CPS_rioGetPortList(DAR_DEV_INFO_t *dev_info, struct DAR_ptl *ptl_in,
		struct DAR_ptl *ptl_out)
{
	uint8_t idx;
	bool dup_ports[RIO_MAX_PORTS];
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;

	if (RIO_DEVI_IDT_CPS1432 == (DECODE_DEVICE_ID(dev_info->devID))) {
		if ((ptl_in->num_ports > NUM_CPS_PORTS(dev_info))
				&& (ptl_in->num_ports != RIO_ALL_PORTS)) {
			goto exit;
		}

		if (!(ptl_in->num_ports)) {
			goto exit;
		}

		if (ptl_in->num_ports == RIO_ALL_PORTS) {
			ptl_out->num_ports = 14;
			for (idx = 0; idx < 8; idx++) {
				ptl_out->pnums[idx] = idx;
			}
			for (idx = 10; idx < 16; idx++) {
				ptl_out->pnums[idx - 2] = idx;
			}
			rc = RIO_SUCCESS;
			goto exit;
		} else {
			for (idx = 0; idx < RIO_MAX_PORTS; idx++) {
				dup_ports[idx] = false;
			}
			dup_ports[8] = true;
			dup_ports[9] = true;

			ptl_out->num_ports = ptl_in->num_ports;
			for (idx = 0; idx < ptl_in->num_ports; idx++) {
				ptl_out->pnums[idx] = ptl_in->pnums[idx];
				if ((ptl_out->pnums[idx] >= NUM_PORTS(dev_info))
						|| (dup_ports[ptl_out->pnums[idx]])) {
					rc = RIO_ERR_PORT_ILLEGAL(idx);
					goto exit;
				}
				dup_ports[ptl_out->pnums[idx]] = true;
			}
		}
		rc = RIO_SUCCESS;
	} else {
		rc = DARDB_rioGetPortList(dev_info, ptl_in, ptl_out);
	}
exit:
	return rc;
}


uint32_t CPS_rioSetAssmblyInfo(DAR_DEV_INFO_t *dev_info, uint32_t asmblyVendID,
		uint16_t asmblyRev)
{
	uint32_t rc = DARRegWrite(dev_info, CPS1848_ASSY_IDENT_CAR_OVRD,
			asmblyVendID);

	if (RIO_SUCCESS == rc) {
		rc = DARRegWrite(dev_info, CPS1848_ASSY_INF_CAR_OVRD,
				asmblyRev);
	}
	return rc;
}

uint32_t CPS_init_dev_info(DAR_DEV_INFO_t *dev_info, const char *name,uint32_t swPortInfo)
{
	uint32_t sw_p_info, ret;

	SAFE_STRNCPY(dev_info->name, name, sizeof(dev_info->name));
	dev_info->assyInfo = 0x100;
	dev_info->devInfo = 0;
	dev_info->srcOps = 0x4;
	dev_info->dstOps = 0;
	dev_info->features = 0x18000779;
	dev_info->extFPtrPortType = 0x9;
	dev_info->extFPtrForErr = 0x1000;
	dev_info->extFPtrForLane = 0x2000;
	dev_info->extFPtrForPort = 0x100;
	dev_info->extFPtrForVC = 0;
	dev_info->extFPtrForVOQ = 0;
	dev_info->swRtInfo = 0x00FF;
	dev_info->swPortInfo = swPortInfo;
	dev_info->swMcastInfo = 0x00FF0028;

	// Some CPS devices have an incorrect port count.
	// Keep the port count, and initialize the connected port number.
        ret = DARrioGetSwitchPortInfo(dev_info, &sw_p_info);
        if (RIO_SUCCESS != ret)
                goto exit;
        dev_info->swPortInfo &= ~RIO_SW_PORT_INF_PORT;
        dev_info->swPortInfo |= sw_p_info & RIO_SW_PORT_INF_PORT;
exit:
	return RIO_SUCCESS;
}

uint32_t CPS_rioDeviceSupported(DAR_DEV_INFO_t *dev_info)
{
	uint32_t rc = DAR_DB_NO_DRIVER;

	if (RIO_VEND_IDT == (DECODE_VENDOR_ID(dev_info->devID))) {
		if (RIO_DEVI_IDT_CPS1848 == (DECODE_DEVICE_ID(dev_info->devID))) {
			rc = CPS_init_dev_info(dev_info, "CPS1848", 0x1200);
		} else if (RIO_DEVI_IDT_CPS1432 == (DECODE_DEVICE_ID(dev_info->devID))) {
			rc = CPS_init_dev_info(dev_info, "CPS1432", 0x1000);
		} else if (RIO_DEVI_IDT_CPS1616 == (DECODE_DEVICE_ID(dev_info->devID))) {
			rc = CPS_init_dev_info(dev_info, "CPS1616", 0x1000);
		} else if (RIO_DEVI_IDT_VPS1616 == (DECODE_DEVICE_ID(dev_info->devID))) {
			rc = CPS_init_dev_info(dev_info, "VPS1616", 0x1000);
		} else if (RIO_DEVI_IDT_SPS1616 == (DECODE_DEVICE_ID(dev_info->devID))) {
			rc = CPS_init_dev_info(dev_info, "SPS1616", 0x1000);
		} else if (DECODE_DEVICE_ID( dev_info->devID ) == 0) {
			/* Now fill out the DAR_info structure... */
			rc = DARDB_rioDeviceSupported(dev_info);
		}
	}
	return rc;
}

#endif /* CPS_DAR_WANTED */

#ifdef __cplusplus
}
#endif
