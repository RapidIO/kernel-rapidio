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
#include "Tsi721_DeviceDriver.h"
#include "Tsi721.h"
#include "rio_standard.h"
#include "rio_ecosystem.h"
#include "DAR_DB_Private.h"
#include "string_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TSI721_DAR_WANTED

uint32_t tsi721_rioDeviceSupported(DAR_DEV_INFO_t *dev_info)
{
	uint32_t rc = DAR_DB_NO_DRIVER;

	if ((TSI721_DEVICE_VENDOR == (dev_info->devID & RIO_DEV_IDENT_VEND))
			&& (RIO_DEVI_IDT_TSI721
					== ((dev_info->devID
							& RIO_DEV_IDENT_DEVI)
							>> 16))) {

		// Now fill out the DAR_info structure...
		rc = DARDB_rioDeviceSupported(dev_info);
		if (rc == RIO_SUCCESS) {
			SAFE_STRNCPY(dev_info->name, "Tsi721",
					sizeof(dev_info->name));
		}
	}
	return rc;
}

#endif /* TSI721_DAR_WANTED */

#ifdef __cplusplus
}
#endif
