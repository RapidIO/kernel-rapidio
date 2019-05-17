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
#include "RXS_DeviceDriver.h"
#include "RXS2448.h"
#include "rio_standard.h"
#include "DAR_DB_Private.h"
#include "string_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RXS_DAR_WANTED

uint32_t rxs_rioSetEnumBound(DAR_DEV_INFO_t *dev_info, struct DAR_ptl *ptl,
		int enum_bnd_val)
{
	struct DAR_ptl good_ptl;
	uint32_t rc;
	unsigned int port_idx;
	rio_port_t port;
	uint32_t spx_ctl, new_spx_ctl;

	// Note: Parameters already checked for null

	rc = DARrioGetPortList(dev_info, ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		goto fail;
	}

	for (port_idx = 0; port_idx < good_ptl.num_ports; port_idx++) {
		port = good_ptl.pnums[port_idx];

		rc = DARRegRead(dev_info, RXS_SPX_CTL(port), &spx_ctl);
		if (RIO_SUCCESS != rc) {
			goto fail;
		}

		if (enum_bnd_val) {
			new_spx_ctl = spx_ctl | RXS_SPX_CTL_ENUM_B;
		} else {
			new_spx_ctl = spx_ctl & ~RXS_SPX_CTL_ENUM_B;
		}

		// Save a register write whenever possible
		if (new_spx_ctl != spx_ctl) {
			rc = DARRegWrite(dev_info, RXS_SPX_CTL(port), spx_ctl);
			if (RIO_SUCCESS != rc) {
				goto fail;
			}
		}
	}
fail:
	return rc;
}

uint32_t rxs_rioDeviceSupported(DAR_DEV_INFO_t *dev_info)
{
	uint32_t rc = DAR_DB_NO_DRIVER;

	if (RXS_DEVICE_VENDOR == (dev_info->devID & RIO_DEV_IDENT_VEND)) {
		if ((RIO_DEVI_IDT_RXS2448)
				== ((dev_info->devID & RIO_DEV_IDENT_DEVI) >> 16)) {
			/* Now fill out the DAR_info structure... */
			rc = DARDB_rioDeviceSupported(dev_info);

			if (rc == RIO_SUCCESS) {
				SAFE_STRNCPY(dev_info->name, "RXS2448",
						sizeof(dev_info->name));
			}
		} else if ((RIO_DEVI_IDT_RXS1632)
				== ((dev_info->devID & RIO_DEV_IDENT_DEVI) >> 16)) {
			/* Now fill out the DAR_info structure... */
			rc = DARDB_rioDeviceSupported(dev_info);

			if (rc == RIO_SUCCESS) {
				SAFE_STRNCPY(dev_info->name, "RXS1632",
						sizeof(dev_info->name));
			}
		}
	}
	return rc;
}

#endif /* RXS_DAR_WANTED */

#ifdef __cplusplus
}
#endif
