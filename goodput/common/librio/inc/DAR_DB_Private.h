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

#ifndef __DAR_DB_PRIVATE_H__
#define __DAR_DB_PRIVATE_H__

#include "rio_standard.h"
#include "RapidIO_Device_Access_Routines_API.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Hide these two macros in order to preserve details of the device handle
 *  implementation.
 */

#define VALIDATE_DEV_INFO(dev_info) ((NULL != dev_info) \
		&& (((dev_info->dsf_h >> 16) & RIO_DEV_IDENT_VEND) \
				== (dev_info->devID & RIO_DEV_IDENT_VEND)) \
		&& ((dev_info->driver_family >= RIO_CPS_DEVICE) && \
				(dev_info->driver_family <= RIO_UNKNOWN_DEVICE)))

#define VENDOR_ID(dev_info) ((uint16_t)(dev_info->devID & RIO_DEV_IDENT_VEND))

#define DEVICE_ID(dev_info) ((uint16_t)((dev_info->devID & \
		RIO_DEV_IDENT_DEVI) >> 16))

#define DECODE_VENDOR_ID(device) ((uint16_t)(device & RIO_DEV_IDENT_VEND))

#define DECODE_DEVICE_ID(device) ((uint16_t)((device & RIO_DEV_IDENT_DEVI) >> 16))

uint32_t DARDB_ReadRegNoDriver(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t *readdata);

uint32_t DARDB_WriteRegNoDriver(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t writedata);

/* DARDB_rioGetPortList
 * Default implementation of rioGetPortList, intended to be called by
 * driver routines that support different devices which do and do not
 * have contiguous port numbers.
 */
uint32_t DARDB_rioGetPortList(DAR_DEV_INFO_t *dev_info, struct DAR_ptl *ptl_in,
		struct DAR_ptl *ptl_out);

uint32_t DARDB_rioSetAssmblyInfo(DAR_DEV_INFO_t *dev_info,
		uint32_t asmblyVendID, uint16_t asmblyRev);

uint32_t DARDB_rioSetEnumBound(DAR_DEV_INFO_t *dev_info, struct DAR_ptl *ptl,
		int enum_bnd_val);

/* DARDB_rioDeviceSupported
 *  Initializes the dev_info fields by reading RapidIO standard registers from
 *  the device..
 *
 *  Each device driver may call DARDB_rioDeviceSupported to
 *    initialize the device driver information structure.
 *
 *  If this is not a device supported by the driver, rioDeviceSupported
 *    implementations must return DAR_DB_NO_DRIVER.
 *
 *  If this is a device supported by the driver, rioDeviceSupported must
 *    return RIO_SUCCESS.
 */
uint32_t DARDB_rioDeviceSupported(DAR_DEV_INFO_t *dev_info);

uint32_t DARDB_rioDeviceSupportedStub(DAR_DEV_INFO_t *dev_info);

#ifdef __cplusplus
}
#endif
#endif /* __DAR_DB_PRIVATE_H__ */
