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

#include "Tsi57x_API.h"
#include "Tsi721_API.h"
#include "rio_ecosystem.h"

#include "CPS_DeviceDriver.h"
#include "RXS_DeviceDriver.h"
#include "Tsi721_DeviceDriver.h"
#include "Tsi57x_DeviceDriver.h"

#include "DAR_DB_Private.h"
#include "string_util.h"

#ifdef __cplusplus
extern "C" {
#endif

const struct DAR_ptl ptl_all_ports = PTL_ALL_PORTS;

/* Device Access Routine (DAR) Device Driver routines
  
   This file contains the implementation of all of the device driver
   routines for the DAR.  These routines all have the same form:
   - Validate the dev_info parameter passed in
   - Invoke the device driver routine
*/

uint32_t (*ReadReg) (DAR_DEV_INFO_t *dev_info, uint32_t offset,
						uint32_t *readdata);
uint32_t (*WriteReg) (DAR_DEV_INFO_t *dev_info, uint32_t  offset,
						uint32_t  writedata );
void (*WaitSec)(uint32_t delay_nsec, uint32_t delay_sec);

rio_driver_family_t rio_get_driver_family(uint32_t devID);

extern uint32_t tsi721_ReadReg(DAR_DEV_INFO_t *dev_info,
		uint32_t offset, uint32_t *readdata);

extern uint32_t tsi57x_ReadReg(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t *readdata);

uint32_t DARRegRead(DAR_DEV_INFO_t *dev_info, uint32_t offset,
						uint32_t *readdata)
{
	int rc;
	unsigned int i;

	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (RIO_UNITIALIZED_DEVICE == dev_info->driver_family) {
		dev_info->driver_family = rio_get_driver_family(dev_info->devID);
	}

	if (!ReadReg) {
		return DAR_DB_NO_DRIVER;
	}

	// Performance optimization, search for cached regs...
	i = DAR_get_poreg_idx(dev_info, offset);
	if (DAR_POREG_BAD_IDX != i) {
		*readdata = dev_info->poregs[i].data;
		return RIO_SUCCESS;
	}

	//@sonar:off - c:S1871
	switch (dev_info->driver_family) {
	case RIO_RXS_DEVICE:
	case RIO_CPS_DEVICE:
		rc = ReadReg(dev_info, offset, readdata);
		break;
	case RIO_TSI57X_DEVICE:
		rc = tsi57x_ReadReg(dev_info, offset, readdata);
		break;
	case RIO_TSI721_DEVICE:
		rc = tsi721_ReadReg(dev_info, offset, readdata);
		break;
	case RIO_UNKNOWN_DEVICE:
		rc = ReadReg(dev_info, offset, readdata);
		break;
	default:
		rc = RIO_ERR_SW_FAILURE;
		break;
	}
	//@sonar:on

	return rc;
}

extern uint32_t tsi721_WriteReg(DAR_DEV_INFO_t *dev_info,
		uint32_t offset, uint32_t writedata);

extern uint32_t tsi57x_WriteReg(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t writedata);

uint32_t DARRegWrite(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t writedata)
{
	uint32_t rc;
	unsigned int i;

	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (RIO_UNITIALIZED_DEVICE == dev_info->driver_family) {
		dev_info->driver_family = rio_get_driver_family(dev_info->devID);
	}

	if (!WriteReg) {
		return DAR_DB_NO_DRIVER;
	}

	//@sonar:off - c:S1871
	switch (dev_info->driver_family) {
	case RIO_RXS_DEVICE:
	case RIO_CPS_DEVICE:
		rc = WriteReg(dev_info, offset, writedata);
		break;
	case RIO_TSI57X_DEVICE:
		rc = tsi57x_WriteReg(dev_info, offset, writedata);
		break;
	case RIO_TSI721_DEVICE:
		rc = tsi721_WriteReg(dev_info, offset, writedata);
		break;
	case RIO_UNKNOWN_DEVICE:
		rc = WriteReg(dev_info, offset, writedata);
		break;
	default:
		rc = RIO_ERR_SW_FAILURE;
		break;
	}
	//@sonar:on

	if (RIO_SUCCESS != rc) {
		return rc;
	}

	// If this is a performance optimization register,
	// update the cached value.
	i = DAR_get_poreg_idx(dev_info, offset);
	if (DAR_POREG_BAD_IDX != i) {
		dev_info->poregs[i].data = writedata;
	}

	return rc;
}

uint32_t DAR_add_poreg(DAR_DEV_INFO_t *dev_info, uint32_t oset, uint32_t data)
{
	if (NULL == dev_info) {
		return RIO_ERR_NULL_PARM_PTR;
	}

	if (NULL == dev_info->poregs) {
		return RIO_ERR_NULL_PARM_PTR;
	}

	if (dev_info->poregs_max <= dev_info->poreg_cnt) {
		return RIO_ERR_INSUFFICIENT_RESOURCES;
	}

	dev_info->poregs[dev_info->poreg_cnt].offset = oset;
	dev_info->poregs[dev_info->poreg_cnt].data = data;
	dev_info->poreg_cnt++;

	return RIO_SUCCESS;
}

uint32_t DAR_get_poreg_idx(DAR_DEV_INFO_t *dev_info, uint32_t oset)
{
	unsigned int i;
	// Performance optimization, search for cached regs...
	for (i = 0; (i < dev_info->poreg_cnt) && dev_info->poregs; i++) {
		if (dev_info->poregs[i].offset == oset) {
			return i;
		}
	}
	return DAR_POREG_BAD_IDX;
}

void DAR_WaitSec( uint32_t delay_nsec, uint32_t delay_sec )
{
	if (WaitSec) {
		WaitSec(delay_nsec, delay_sec);
	}
}

uint32_t DAR_proc_ptr_init(
		uint32_t (*ReadRegCall)(DAR_DEV_INFO_t *dev_info,
				uint32_t offset, uint32_t *readdata),
		uint32_t (*WriteRegCall)(DAR_DEV_INFO_t *dev_info,
				uint32_t offset, uint32_t writedata),
		void (*WaitSecCall)(uint32_t delay_nsec, uint32_t delay_sec))
{
	ReadReg = ReadRegCall;
	WriteReg = WriteRegCall;
	WaitSec = WaitSecCall;

	return RIO_SUCCESS;
}

rio_driver_family_t rio_get_driver_family(uint32_t devID)
{
	uint16_t vend_code = (uint16_t)(devID & RIO_DEV_IDENT_VEND);
	uint16_t dev_code = (uint16_t)((devID & RIO_DEV_IDENT_DEVI) >> 16);

	switch (vend_code) {
	case RIO_VEND_IDT:
		switch (dev_code) {
		case RIO_DEVI_IDT_CPS1848:
		case RIO_DEVI_IDT_CPS1432:
		case RIO_DEVI_IDT_CPS1616:
		case RIO_DEVI_IDT_SPS1616:
#ifdef CPS_DAR_WANTED
			return RIO_CPS_DEVICE;
#endif
			break;

		case RIO_DEVI_IDT_RXS2448:
		case RIO_DEVI_IDT_RXS1632:
#ifdef RXS_DAR_WANTED
			return RIO_RXS_DEVICE;
#endif
			break;

		case RIO_DEVI_IDT_TSI721:
#ifdef TSI721_DAR_WANTED
			return RIO_TSI721_DEVICE;
#endif
			break;

		default:
			break;
		}
		break;
	case RIO_VEND_TUNDRA:
		switch (dev_code) {
		case RIO_DEVI_TSI572:
		case RIO_DEVI_TSI574:
		case RIO_DEVI_TSI577:
		case RIO_DEVI_TSI578:
#ifdef TSI57X_DAR_WANTED
			return RIO_TSI57X_DEVICE;
#endif
			break;

		default:
			break;
		}
		break;
	default:
		break;
	}
	return RIO_UNKNOWN_DEVICE;
}


uint32_t update_dev_info_regvals(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t reg_val)
{
	uint32_t rc = RIO_SUCCESS;

	//@sonar:off - Collapsible "if" statements should be merged
	if (dev_info->extFPtrForPort && RIO_SP_VLD(dev_info->extFPtrPortType)) {
		if ((offset >= RIO_SPX_CTL(dev_info->extFPtrForPort,
						dev_info->extFPtrPortType, 0))
		&& (offset <= RIO_SPX_CTL(dev_info->extFPtrForPort,
					dev_info->extFPtrPortType,
					(NUM_PORTS(dev_info) - 1)))) {
			if ((0x1C == (offset & 0x1C) &&
				!RIO_SP3_VLD( dev_info->extFPtrPortType))
				|| (0x3C == (offset & 0x3C)
				&& RIO_SP3_VLD( dev_info->extFPtrPortType))) {
				uint8_t idx;

				idx = (offset -
					RIO_SPX_CTL(dev_info->extFPtrForPort,
						dev_info->extFPtrPortType,
																0))
				/ RIO_SP_STEP( dev_info->extFPtrPortType);
				if (idx >= NUM_PORTS(dev_info)) {
					rc = RIO_ERR_SW_FAILURE;
				} else {
					dev_info->ctl1_reg[idx] = reg_val;
				}
			}
		}
	}
	//@sonar:on

	return rc;
}

uint32_t DARrioGetNumLocalPorts(DAR_DEV_INFO_t *dev_info,
		uint32_t *numLocalPorts)
{
	uint32_t rc;

	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	rc = DARRegRead(dev_info, RIO_SW_PORT_INF, numLocalPorts);
	if (RIO_SUCCESS == rc) {
		*numLocalPorts = RIO_AVAIL_PORTS(*numLocalPorts);
	} else {
		*numLocalPorts = 0;
	}
	return rc;
}


uint32_t DARrioGetFeatures(DAR_DEV_INFO_t *dev_info, RIO_PE_FEAT_T *features)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}
	return DARRegRead(dev_info, RIO_PE_FEAT, features);
}


uint32_t DARrioGetSwitchPortInfo(DAR_DEV_INFO_t *dev_info,
		RIO_SW_PORT_INF_T *portinfo)
{
	uint32_t rc;

	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	rc = DARRegRead(dev_info, RIO_SW_PORT_INF, portinfo);
	//@sonar:off - Collapsible "if" statements should be merged
	if (RIO_SUCCESS == rc) {
		/* If this is not a switch or a multiport-endpoint, portinfo
		 should be 0.  Fake the existence of the switch port info register
		 by supplying 1 for the number of ports, and  0 as the port that
		 we're connected to.

		 Otherwise, leave portinfo alone.
		 */
		if (!(dev_info->features & (RIO_PE_FEAT_SW | RIO_PE_FEAT_MULTIP))
				&& (!*portinfo)) {
			*portinfo = 0x00000100;
		}
	}
	//@sonar:on
	return rc;
}


uint32_t DARrioGetExtFeaturesPtr(DAR_DEV_INFO_t *dev_info, uint32_t *extfptr)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		*extfptr = 0x00000000;
		return DAR_DB_INVALID_HANDLE;
	}

	if (dev_info->features & RIO_PE_FEAT_EFB_VALID) {
		*extfptr = dev_info->assyInfo & RIO_ASSY_INF_EFB_PTR;
		return RIO_SUCCESS;
	}
	return RIO_ERR_FEATURE_NOT_SUPPORTED;
}


uint32_t DARrioGetNextExtFeaturesPtr(DAR_DEV_INFO_t *dev_info,
		uint32_t currfptr, uint32_t *extfptr)
{
	uint32_t rc;

	if (!VALIDATE_DEV_INFO(dev_info)) {
		*extfptr = 0x00000000;
		return DAR_DB_INVALID_HANDLE;
	}

	if (currfptr & ( RIO_ASSY_INF_EFB_PTR >> 16)) {
		rc = DARRegRead(dev_info, currfptr, extfptr);
		*extfptr = (*extfptr & RIO_ASSY_INF_EFB_PTR) >> 16;
	} else {
		rc = RIO_ERR_FEATURE_NOT_SUPPORTED;
	}
	return rc;
}


uint32_t DARrioGetSourceOps(DAR_DEV_INFO_t *dev_info, RIO_SRC_OPS_T *srcops)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}
	return DARRegRead(dev_info, RIO_SRC_OPS, srcops);
}


uint32_t DARrioGetDestOps(DAR_DEV_INFO_t *dev_info, RIO_DST_OPS_T *dstops)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}
	return DARRegRead(dev_info, RIO_DST_OPS, dstops);
}


uint32_t DARrioGetAddressMode(DAR_DEV_INFO_t *dev_info, RIO_PE_ADDR_T *amode)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (dev_info->features & RIO_PE_FEAT_EXT_ADDR) {
		return DARRegRead(dev_info, RIO_PE_LL_CTL, amode);
	}
	return RIO_ERR_FEATURE_NOT_SUPPORTED;
}


uint32_t DARrioGetBaseDeviceId(DAR_DEV_INFO_t *dev_info, uint32_t *deviceid)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}
	if (dev_info->features & (RIO_PE_FEAT_PROC |
			RIO_PE_FEAT_MEM |
			RIO_PE_FEAT_BRDG)) {
		return DARRegRead(dev_info, RIO_DEVID, deviceid);
	}
	return RIO_ERR_FEATURE_NOT_SUPPORTED;
}


uint32_t DARrioSetBaseDeviceId(DAR_DEV_INFO_t *dev_info, uint32_t newdeviceid)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (dev_info->features & (RIO_PE_FEAT_PROC |
			RIO_PE_FEAT_MEM |
			RIO_PE_FEAT_BRDG)) {
		return DARRegWrite(dev_info, RIO_DEVID, newdeviceid);
	}
	return RIO_ERR_FEATURE_NOT_SUPPORTED;
}


uint32_t DARrioAcquireDeviceLock(DAR_DEV_INFO_t *dev_info,
		uint16_t hostdeviceid, uint16_t *hostlockid)
{
	uint32_t regVal;
	uint32_t rc;

	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	// Reset value is 0xFFFF.
	// Write value 0x1234, it becomes 0x1234
	// Write value 0x1234 again, it becomes 0xFFFF.
	// Ignore writes of any other value when <> 0xFFFF
	//
	if ((hostdeviceid == RIO_HOST_LOCK_DEVID) || (!hostlockid)) {
		return RIO_ERR_INVALID_PARAMETER;
	}

	rc = DARRegRead(dev_info, RIO_HOST_LOCK, &regVal);
	if (RIO_SUCCESS != rc) {
		return rc;
	}

	*hostlockid = hostdeviceid;
	if ((uint16_t)(regVal) == hostdeviceid) {
		// Lock already held by this entity, return success
		return rc;
	}

	rc = DARRegWrite(dev_info, RIO_HOST_LOCK, hostdeviceid);
	if (RIO_SUCCESS != rc) {
		return rc;
	}

	rc = DARRegRead(dev_info, RIO_HOST_LOCK, &regVal);
	if (RIO_SUCCESS != rc) {
		return rc;
	}

	regVal &= RIO_HOST_LOCK_DEVID;
	*hostlockid = (uint16_t)(regVal);
	if (regVal != hostdeviceid) {
		return RIO_ERR_LOCK;
	}
	return RIO_SUCCESS;
}


uint32_t DARrioReleaseDeviceLock(DAR_DEV_INFO_t *dev_info,
		uint16_t hostdeviceid, uint16_t *hostlockid)
{
	uint32_t regVal;
	uint32_t rc;

	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	rc = DARRegWrite(dev_info, RIO_HOST_LOCK, hostdeviceid);
	if (RIO_SUCCESS == rc) {
		rc = DARRegRead(dev_info, RIO_HOST_LOCK, &regVal);
		if (RIO_SUCCESS == rc) {
			*hostlockid = (uint16_t)(regVal & RIO_HOST_LOCK_DEVID);
			if (0xFFFF != *hostlockid) {
				rc = RIO_ERR_LOCK;
			}
		}
	}
	return rc;
}


uint32_t DARrioGetComponentTag(DAR_DEV_INFO_t *dev_info, uint32_t *componenttag)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}
	return DARRegRead(dev_info, RIO_COMPTAG, componenttag);
}


uint32_t DARrioSetComponentTag(DAR_DEV_INFO_t *dev_info, uint32_t componenttag)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}
	return DARRegWrite(dev_info, RIO_COMPTAG, componenttag);
}


uint32_t DARrioGetAddrMode(DAR_DEV_INFO_t *dev_info, RIO_PE_ADDR_T *addr_mode)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}
	return DARRegRead(dev_info, RIO_PE_LL_CTL, addr_mode);
}


uint32_t DARrioSetAddrMode(DAR_DEV_INFO_t *dev_info, RIO_PE_ADDR_T addr_mode)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}
	return DARRegWrite(dev_info, RIO_PE_LL_CTL, addr_mode);
}


uint32_t DARrioGetPortErrorStatus(DAR_DEV_INFO_t *dev_info, uint8_t portnum,
		RIO_SPX_ERR_STAT_T *err_status)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (dev_info->extFPtrForPort) {
		if ((!portnum)
				|| (portnum
						< RIO_AVAIL_PORTS(
								dev_info->swPortInfo))) {
			return DARRegRead(dev_info,
					RIO_SPX_ERR_STAT(
							dev_info->extFPtrForPort,
							dev_info->extFPtrPortType,
							portnum), err_status);
		}
		return RIO_ERR_INVALID_PARAMETER;
	}
	return RIO_ERR_FEATURE_NOT_SUPPORTED;
}


uint32_t DARrioLinkReqNResp(DAR_DEV_INFO_t *dev_info, uint8_t portnum,
		uint32_t *link_stat)
{
	uint32_t rc;
	uint8_t attempts;
	uint32_t err_n_stat;

	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}
	if (dev_info->extFPtrForPort) {
		if ((!portnum)
				|| (portnum
						< RIO_AVAIL_PORTS(
								dev_info->swPortInfo))) {
			rc =
					DARRegRead(dev_info,
							RIO_SPX_ERR_STAT(
									dev_info->extFPtrForPort,
									dev_info->extFPtrPortType,
									portnum),
							&err_n_stat);
			if (RIO_SUCCESS != rc) {
				return rc;
			}

			if (!RIO_PORT_OK(err_n_stat)) {
				return RIO_ERR_PORT_OK_NOT_DETECTED;
			}

			rc =
					DARRegWrite(dev_info,
							RIO_SPX_LM_REQ(
									dev_info->extFPtrForPort,
									dev_info->extFPtrPortType,
									portnum),
							RIO_SPX_LM_REQ_CMD_LR_IS);
			if (RIO_SUCCESS != rc) {
				return rc;
			}

			/* Note that typically a link-response will be received faster than another
			 * maintenance packet can be issued.  Nevertheless, the routine polls 10 times
			 * to check for a valid response, where 10 is a small number assumed to give
			 * enough time for even the slowest device to respond.
			 */
			for (attempts = 0; attempts < 10; attempts++) {
				rc =
						DARRegRead(dev_info,
								RIO_SPX_LM_RESP(
										dev_info->extFPtrForPort,
										dev_info->extFPtrPortType,
										portnum),
								link_stat);

				if (RIO_SUCCESS != rc) {
					return rc;
				}

				if (RIO_SPX_LM_RESP_IS_VALID(*link_stat)) {
					return rc;
				}
			}
			return RIO_ERR_NOT_EXPECTED_RETURN_VALUE;
		}
		return RIO_ERR_INVALID_PARAMETER;
	}
	return RIO_ERR_FEATURE_NOT_SUPPORTED;
}


uint32_t DARrioStdRouteAddEntry(DAR_DEV_INFO_t *dev_info, uint16_t routedestid,
		uint8_t routeportno)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (dev_info->features & RIO_PE_FEAT_SW) {
		if (RIO_SUCCESS
				== DARRegWrite(dev_info, RIO_DEVID_RTE,
						routedestid)) {
			return DARRegWrite(dev_info, RIO_RTE, routeportno);
		}
		return RIO_ERR_NO_SWITCH;
	}
	return RIO_ERR_NO_SWITCH;
}


uint32_t DARrioStdRouteGetEntry(DAR_DEV_INFO_t *dev_info, uint16_t routedestid,
		uint8_t *routeportno)
{
	uint32_t rc;
	uint32_t regVal;

	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (dev_info->features & RIO_PE_FEAT_SW) {
		rc = DARRegWrite(dev_info, RIO_DEVID_RTE, routedestid);
		if (RIO_SUCCESS == rc) {
			rc = DARRegRead(dev_info, RIO_RTE, &regVal);
			*routeportno = (uint8_t)(regVal & RIO_RTE_PORT);
		}
		return rc;
	}
	return RIO_ERR_NO_SWITCH;
}


uint32_t DARrioStdRouteInitAll(DAR_DEV_INFO_t *dev_info, uint8_t routeportno)
{
	uint32_t rc;
	int32_t num_dests;
	int32_t dest_idx;

	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (dev_info->swRtInfo && (dev_info->features & RIO_PE_FEAT_SW)) {
		num_dests = (dev_info->swRtInfo & RIO_SW_RT_TBL_LIM_MAX_DESTID);
		rc = DARrioStdRouteSetDefault(dev_info, routeportno);
		for (dest_idx = 0;
				((dest_idx <= num_dests) && (RIO_SUCCESS == rc));
				dest_idx++) {
			rc = DARrioStdRouteAddEntry(dev_info, dest_idx,
					routeportno);
		}
		return rc;
	}
	return RIO_ERR_NO_SWITCH;
}


uint32_t DARrioStdRouteRemoveEntry(DAR_DEV_INFO_t *dev_info,
		uint16_t routedestid)
{
	uint32_t rc;

	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (dev_info->features & RIO_PE_FEAT_SW) {
		rc = DARRegWrite(dev_info, RIO_DEVID_RTE, routedestid);
		if (RIO_SUCCESS == rc) {
			return DARRegWrite(dev_info, RIO_RTE,
			RIO_ALL_PORTS);
		}
		return rc;
	}
	return RIO_ERR_NO_SWITCH;
}


uint32_t DARrioStdRouteSetDefault(DAR_DEV_INFO_t *dev_info, uint8_t routeportno)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (dev_info->features & RIO_PE_FEAT_SW) {
		return DARRegWrite(dev_info, RIO_DFLT_RTE, routeportno);
	}
	return RIO_ERR_NO_SWITCH;
}


uint32_t DARrioSetAssmblyInfo(DAR_DEV_INFO_t *dev_info, uint32_t asmblyVendID,
		uint16_t asmblyRev)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (RIO_CPS_DEVICE == dev_info->driver_family) {
		return CPS_rioSetAssmblyInfo(dev_info, asmblyVendID, asmblyRev);
	}
	return RIO_ERR_FEATURE_NOT_SUPPORTED;
}


uint32_t DARrioGetAssmblyInfo(DAR_DEV_INFO_t *dev_info, uint32_t *AsmblyVendID,
		uint16_t *AsmblyRev)
{
	uint32_t temp;
	uint32_t rc;

	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	rc = DARRegRead(dev_info, RIO_ASSY_ID, AsmblyVendID);
	if (RIO_SUCCESS == rc) {
		rc = DARRegRead(dev_info, RIO_ASSY_INF, &temp);
		if (RIO_SUCCESS == rc) {
			temp = (temp & RIO_ASSY_INF_ASSY_REV) >> 16;
			*AsmblyRev = (uint16_t)(temp);
		}
	}
	return rc;
}

uint32_t DARDB_rioGetPortList(DAR_DEV_INFO_t *dev_info,
		struct DAR_ptl *ptl_in,
		struct DAR_ptl *ptl_out)
{
	uint8_t idx;
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	bool dup_port_num[RIO_MAX_PORTS] = {false};

	ptl_out->num_ports = 0;

	if ((ptl_in->num_ports > NUM_PORTS(dev_info))
			&& (ptl_in->num_ports != RIO_ALL_PORTS)) {
		goto exit;
	}
	if ((ptl_in->num_ports > RIO_MAX_PORTS)
			&& (ptl_in->num_ports != RIO_ALL_PORTS)) {
		goto exit;
	}

	if (!(ptl_in->num_ports)) {
		goto exit;
	}

	if (RIO_ALL_PORTS == ptl_in->num_ports) {
		ptl_out->num_ports = NUM_PORTS(dev_info);
		for (idx = 0; idx < NUM_PORTS(dev_info); idx++) {
			ptl_out->pnums[idx] = idx;
		}
	} else {
		ptl_out->num_ports = ptl_in->num_ports;
		for (idx = 0; idx < ptl_in->num_ports; idx++) {
			ptl_out->pnums[idx] = ptl_in->pnums[idx];
			if ((ptl_out->pnums[idx] >= NUM_PORTS(dev_info))
					|| (dup_port_num[ptl_out->pnums[idx]])) {
				ptl_out->num_ports = 0;
				rc = RIO_ERR_PORT_ILLEGAL(idx);
				goto exit;
			}
			dup_port_num[ptl_out->pnums[idx]] = true;
		}
	}
	rc = RIO_SUCCESS;

exit:
	return rc;
}

uint32_t DARrioGetPortList(DAR_DEV_INFO_t *dev_info, struct DAR_ptl *ptl_in,
		struct DAR_ptl *ptl_out)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (!ptl_in || !ptl_out) {
		return RIO_ERR_NULL_PARM_PTR;
	}

	if (RIO_CPS_DEVICE == dev_info->driver_family) {
		return CPS_rioGetPortList(dev_info, ptl_in, ptl_out);
	}
	return DARDB_rioGetPortList(dev_info, ptl_in, ptl_out);
}


uint32_t DARrioSetEnumBound(DAR_DEV_INFO_t *dev_info, struct DAR_ptl *ptl,
		int enum_bnd_val)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (!ptl) {
		return RIO_ERR_NULL_PARM_PTR;
	}

	if (RIO_RXS_DEVICE == dev_info->driver_family) {
		return rxs_rioSetEnumBound(dev_info, ptl, enum_bnd_val);
	}
	return DARDB_rioSetEnumBound(dev_info, ptl, enum_bnd_val);
}


uint32_t DARrioGetDevResetInitStatus(DAR_DEV_INFO_t *dev_info)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}
	return RIO_SUCCESS;
}


uint32_t DARrioPortEnable(DAR_DEV_INFO_t *dev_info, struct DAR_ptl *ptl,
		bool port_ena, bool port_lkout, bool in_out_ena)
{
	uint32_t rc;
	uint32_t port_n_ctrl1_addr;
	uint32_t port_n_ctrl1_reg;
	struct DAR_ptl good_ptl;
	uint8_t idx;

	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (!ptl) {
		return RIO_ERR_NULL_PARM_PTR;
	}

	/* Check whether 'portno' is assigned to a valid port value or not
	 */
	rc = DARrioGetPortList(dev_info, ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		return rc;
	}

	for (idx = 0; idx < good_ptl.num_ports; idx++) {
		port_n_ctrl1_addr = RIO_SPX_CTL(dev_info->extFPtrForPort,
				dev_info->extFPtrPortType, good_ptl.pnums[idx]);

		rc = DARRegRead(dev_info, port_n_ctrl1_addr, &port_n_ctrl1_reg);
		if (RIO_SUCCESS != rc) {
			return rc;
		}

		if (port_ena) {
			port_n_ctrl1_reg &= ~RIO_SPX_CTL_PORT_DIS;
		} else {
			port_n_ctrl1_reg |= RIO_SPX_CTL_PORT_DIS;
		}

		if (port_lkout) {
			port_n_ctrl1_reg |= RIO_SPX_CTL_LOCKOUT;
		} else {
			port_n_ctrl1_reg &= ~RIO_SPX_CTL_LOCKOUT;
		}

		if (in_out_ena) {
			port_n_ctrl1_reg |= RIO_SPX_CTL_INP_EN
					| RIO_SPX_CTL_OTP_EN;
		} else {
			port_n_ctrl1_reg &= ~(RIO_SPX_CTL_INP_EN
					| RIO_SPX_CTL_OTP_EN);
		}

		rc = DARRegWrite(dev_info, port_n_ctrl1_addr, port_n_ctrl1_reg);
	}
	return rc;
}


uint32_t DARrioEmergencyLockout(DAR_DEV_INFO_t *dev_info, uint8_t port_num)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	if (dev_info->extFPtrForPort && RIO_SP_VLD(dev_info->extFPtrPortType)
			&& (port_num < NUM_PORTS(dev_info))) {
		return DARRegWrite(dev_info,
				RIO_SPX_CTL(dev_info->extFPtrForPort,
						dev_info->extFPtrPortType,
						port_num),
				dev_info->ctl1_reg[port_num]
						| RIO_SPX_CTL_LOCKOUT);
	}
	return RIO_ERR_INVALID_PARAMETER;
}

uint32_t DARDB_rioSetEnumBound(DAR_DEV_INFO_t *dev_info,
		struct DAR_ptl *ptl, int enum_bnd_val)
{
	uint32_t rc = RIO_ERR_FEATURE_NOT_SUPPORTED;
	uint32_t currCSR, tempCSR;
	struct DAR_ptl good_ptl;
	uint8_t idx;

	if (dev_info->extFPtrForPort) {
		rc = DARrioGetPortList(dev_info, ptl, &good_ptl);
		if (rc != RIO_SUCCESS) {
			goto exit;
		}

		for (idx = 0; idx < good_ptl.num_ports; idx++) {
			rc =
					DARRegRead(dev_info,
							RIO_SPX_CTL(
									dev_info->extFPtrForPort,
									dev_info->extFPtrPortType,
									good_ptl.pnums[idx]),
							&currCSR);
			if (RIO_SUCCESS == rc) {
				if (enum_bnd_val) {
					tempCSR = currCSR | RIO_SPX_CTL_ENUM_B;
				} else {
					tempCSR = currCSR & ~RIO_SPX_CTL_ENUM_B;
				}

				if (tempCSR == currCSR) {
					continue;
				}

				rc =
						DARRegWrite(dev_info,
								RIO_SPX_CTL(
										dev_info->extFPtrForPort,
										dev_info->extFPtrPortType,
										good_ptl.pnums[idx]),
								tempCSR);
				if (RIO_SUCCESS == rc) {
					rc =
							DARRegRead(dev_info,
									RIO_SPX_CTL(
											dev_info->extFPtrForPort,
											dev_info->extFPtrPortType,
											good_ptl.pnums[idx]),
									&tempCSR);
					if (tempCSR != currCSR) {
						rc =
								RIO_ERR_FEATURE_NOT_SUPPORTED;
					}
				}
			}
		}
	}
exit:
	return rc;
}


/* The dev_info->devID field must be valid when this routine is called.

   Default driver for a device is the DARDB_ routines defined above.
   This routine is the last driver bound in to the DAR DB, so it will
   allow otherwise 'unsupported' devices to get minimal support.
  
   This routine should also be called directly by all devices to ensure that
   their DAR_DEV_INFO_t fields are correctly filled in.  Device specific
   fixups can be added after a call to this routine.
*/
uint32_t DARDB_rioDeviceSupported(DAR_DEV_INFO_t *dev_info)
{
	uint32_t rc;

	/* The reading devID should be unnecessary,
	 but it should not hurt to do so here.
	 */
	rc = ReadReg(dev_info, RIO_DEV_IDENT, &dev_info->devID);
	if (RIO_SUCCESS != rc) {
		return rc;
	}
	dev_info->driver_family = rio_get_driver_family(dev_info->devID);

	rc = ReadReg(dev_info, RIO_DEV_INF, &dev_info->devInfo);
	if (RIO_SUCCESS != rc) {
		return rc;
	}

	rc = ReadReg(dev_info, RIO_ASSY_INF, &dev_info->assyInfo);
	if (RIO_SUCCESS != rc)
		return rc;

	rc = ReadReg(dev_info, RIO_PE_FEAT, &dev_info->features);
	if (RIO_SUCCESS != rc) {
		return rc;
	}

	/* swPortInfo can be supported by endpoints.
	 May as well read the register...
	 */
	rc = ReadReg(dev_info, RIO_SW_PORT_INF, &dev_info->swPortInfo);
	if (RIO_SUCCESS != rc) {
		return rc;
	}

	if (!dev_info->swPortInfo)
		dev_info->swPortInfo = 0x00000100; /* Default for endpoints is 1 port
		 */

	if (dev_info->features & RIO_PE_FEAT_SW) {
		rc = ReadReg(dev_info, RIO_SW_RT_TBL_LIM, &dev_info->swRtInfo);
		if (RIO_SUCCESS != rc) {
			return rc;
		}
	}

	rc = ReadReg(dev_info, RIO_SRC_OPS, &dev_info->srcOps);
	if (RIO_SUCCESS != rc) {
		return rc;
	}

	rc = ReadReg(dev_info, RIO_DST_OPS, &dev_info->dstOps);
	if (RIO_SUCCESS != rc) {
		return rc;
	}

	if (dev_info->features & RIO_PE_FEAT_MC) {
		rc = ReadReg(dev_info, RIO_SW_MC_INF, &dev_info->swMcastInfo);
		if (RIO_SUCCESS != rc) {
			return rc;
		}
	}

	if (dev_info->features & RIO_PE_FEAT_EFB_VALID) {
		uint32_t curr_ext_feat;
		uint32_t prev_addr;

		rc = DARrioGetExtFeaturesPtr(dev_info, &prev_addr);
		while (( RIO_SUCCESS == rc) && prev_addr) {
			rc = ReadReg(dev_info, prev_addr, &curr_ext_feat);
			if (RIO_SUCCESS != rc) {
				return rc;
			}

			switch (curr_ext_feat & RIO_EFB_T) {
			case RIO_EFB_T_SP_EP:
			case RIO_EFB_T_SP_EP_SAER:
			case RIO_EFB_T_SP_NOEP:
			case RIO_EFB_T_SP_NOEP_SAER:
			case RIO_EFB_T_SP_EP3:
			case RIO_EFB_T_SP_EP3_SAER:
			case RIO_EFB_T_SP_NOEP3:
			case RIO_EFB_T_SP_NOEP3_SAER:
				dev_info->extFPtrPortType = curr_ext_feat &
				RIO_EFB_T;
				dev_info->extFPtrForPort = prev_addr;
				break;

			case RIO_EFB_T_EMHS:
				dev_info->extFPtrForErr = prev_addr;
				break;

			case RIO_EFB_T_HS:
				dev_info->extFPtrForHS = prev_addr;
				break;

			case RIO_EFB_T_LANE:
				dev_info->extFPtrForLane = prev_addr;
				break;

			case RIO_EFB_T_VC:
				dev_info->extFPtrForVC= prev_addr;
				break;

			case RIO_EFB_T_VOQ:
				dev_info->extFPtrForVOQ = prev_addr;
				break;

			case RIO_EFB_T_RT:
				dev_info->extFPtrForRT = prev_addr;
				break;

			case RIO_EFB_T_TS:
				dev_info->extFPtrForTS = prev_addr;
				break;

			case RIO_EFB_T_MISC:
				dev_info->extFPtrForMISC = prev_addr;
				break;

			default:
				if (0xFFFFFFFF == curr_ext_feat) {
					// Register access has failed.
					return RIO_ERR_ACCESS;
				}
				break;
			}
			prev_addr = curr_ext_feat >> 16;
		}
	}
	return rc;
}

uint32_t DARDB_rioDeviceSupportedStub(DAR_DEV_INFO_t *dev_info)
{
	if (NULL == dev_info) {
		return RIO_ERR_NULL_PARM_PTR;

	}
	return DAR_DB_NO_DRIVER;
}

uint32_t DARDB_rioSetAssmblyInfo(DAR_DEV_INFO_t *dev_info,
		uint32_t asmblyVendID, uint16_t asmblyRev)
{
	if (NULL == dev_info) {
		return RIO_ERR_NULL_PARM_PTR;
	}

	if (asmblyVendID || asmblyRev) {
		return RIO_ERR_FEATURE_NOT_SUPPORTED;
	}

	return RIO_ERR_FEATURE_NOT_SUPPORTED;
}

uint32_t DARrioDeviceSupported(DAR_DEV_INFO_t *dev_info)
{
	if (!VALIDATE_DEV_INFO(dev_info)) {
		return DAR_DB_INVALID_HANDLE;
	}

	switch(dev_info->driver_family) {
	case RIO_CPS_DEVICE:
		return CPS_rioDeviceSupported(dev_info);
	case RIO_RXS_DEVICE:
		return rxs_rioDeviceSupported(dev_info);
	case RIO_TSI721_DEVICE:
		return tsi721_rioDeviceSupported(dev_info);
	case RIO_TSI57X_DEVICE:
		return tsi57x_rioDeviceSupported(dev_info);
	case RIO_UNITIALIZED_DEVICE:
	case RIO_UNKNOWN_DEVICE:
		return DARDB_rioDeviceSupported(dev_info);
	default:
		return RIO_ERR_SW_FAILURE;
	}
}

uint32_t DAR_Find_Driver_for_Device(bool dev_info_devID_valid,
		DAR_DEV_INFO_t *dev_info)
{
	uint32_t rc = RIO_SUCCESS;

	SAFE_STRNCPY(dev_info->name, "UNKNOWN", sizeof(dev_info->name));

	// If dev_info_devID_valid is true, we are using a static devID instead
	// of probing.  Otherwise, we are probing a SRIO device to get a devID

	if (!dev_info_devID_valid) {
		rc = ReadReg(dev_info, RIO_DEV_IDENT, &dev_info->devID);
		if (RIO_SUCCESS != rc) {
			goto fail;
		}
		dev_info->driver_family = rio_get_driver_family(dev_info->devID);
	}

	dev_info->dsf_h = VENDOR_ID(dev_info) << 16;
	dev_info->poregs_max = 0;
	dev_info->poreg_cnt = 0;
	rc = DARrioDeviceSupported(dev_info) ;

	if ((RIO_SUCCESS == rc) && dev_info->extFPtrForPort) {
		// NOTE: All register manipulations must be done with
		// DARReadReg and DARWriteReg, or the application must
		// manage dev_info->ctl1_reg themselves manually.
		//
		// Otherwise, the EmergencyLockout call may have
		// unintended consequences.

		uint32_t ctl1;
		uint8_t idx;
		for (idx = 0; idx < NUM_PORTS(dev_info); idx++) {
			rc = ReadReg(dev_info,
				RIO_SPX_CTL(dev_info->extFPtrForPort,
						dev_info->extFPtrPortType,
						idx),
					&ctl1);
			if (RIO_SUCCESS != rc) {
				goto fail;
			}
			dev_info->ctl1_reg[idx] = ctl1;
		}
	}

fail:
	return rc;
}

#ifdef __cplusplus
}
#endif
