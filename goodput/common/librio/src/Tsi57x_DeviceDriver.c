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

#include "RapidIO_Device_Access_Routines_API.h"
#include "Tsi57x_DeviceDriver.h"
#include "Tsi578.h"
#include "rio_standard.h"
#include "DAR_DB_Private.h"
#include "string_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TSI57X_DAR_WANTED

struct {
	const char *name; /* Constant name string */
	const uint32_t devID; /* Vendor + Device ID   */
} device_names[] = //
		{
				{"Tsi572", ((uint32_t)(TSI572_RIO_DEVID_VAL)
						<< 16) + RIO_VEND_TUNDRA}, //
				{"Tsi574", ((uint32_t)(TSI574_RIO_DEVID_VAL)
						<< 16) + RIO_VEND_TUNDRA}, //
				{"Tsi578", ((uint32_t)(TSI578_RIO_DEVID_VAL)
						<< 16) + RIO_VEND_TUNDRA}, //
				{"Tsi576", ((uint32_t)(TSI576_RIO_DEVID_VAL)
						<< 16) + RIO_VEND_TUNDRA},
				{"Tsi577", ((uint32_t)(TSI577_RIO_DEVID_VAL)
						<< 16) + RIO_VEND_TUNDRA}, //
		};

static void getTsiName(DAR_DEV_INFO_t *dev_info)
{
	uint32_t i;

	for (i = 0; i < (sizeof(device_names) / sizeof(device_names[0])); i++) {
		if (device_names[i].devID == dev_info->devID) {
			SAFE_STRNCPY(dev_info->name, device_names[i].name, sizeof(dev_info->name));
			break;
		}
	}
}

extern uint32_t tsi57x_init_scratchpad(DAR_DEV_INFO_t *dev_info); // Tsi57x_PC
uint32_t tsi57x_rioDeviceSupported(DAR_DEV_INFO_t *dev_info)
{
	uint32_t rc = DAR_DB_NO_DRIVER;

	//@sonar:off - Collapsible "if" statements should be merged
	if (RIO_VEND_TUNDRA == (dev_info->devID & RIO_DEV_IDENT_VEND)) {
		if ((RIO_DEVI_TSI57X >> 4)
				== ((dev_info->devID & RIO_DEV_IDENT_DEVI) >> 20)) {
			/* Now fill out the DAR_info structure... */
			rc = DARDB_rioDeviceSupported(dev_info);
			if (RIO_SUCCESS != rc) {
				return rc;
			}

			rc = tsi57x_init_scratchpad(dev_info);
			if (rc == RIO_SUCCESS) {
				getTsiName(dev_info);
			}
		}
	}
	//@sonar:on
	return rc;
}

#endif /* TSI57X_DAR_WANTED */

#ifdef __cplusplus
}
#endif
