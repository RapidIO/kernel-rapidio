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
#include <stdbool.h>

#include "DAR_DB_Private.h"
#include "DSF_DB_Private.h"

#include "RapidIO_Device_Access_Routines_API.h"
#include "Tsi57x_DeviceDriver.h"
#include "Tsi578.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TSI57X_DAR_WANTED

extern const struct scrpad_info *tsi57x_get_scrpad_info(); // Tsi57x_PC

static void tsi57x_WriteReg_mask_cfg(DAR_DEV_INFO_t *dev_info,
		uint32_t writedata)
{
	uint32_t mask = (writedata & TSI578_RIO_MC_MASK_CFG_MC_MASK_NUM) >> 16;
	uint8_t port = (writedata & RIO_MC_MSK_CFG_PT_NUM) >> 8;
	uint32_t cmd  = (writedata & RIO_MC_MSK_CFG_CMD);
	/* Write to TSI578_RIO_MC_MASK_CFG can update mask registers.
	 * Emulate effect on mask registers, as we can't trust reading the
	 * global mask registers if Port 0 is powered down.
	 */

	switch (cmd) {
	case RIO_MC_MSK_CFG_CMD_ADD:
		dev_info->scratchpad[mask+SCRPAD_MASK_IDX] |= ((uint32_t)(1) << (port + 16));
		break;
	case RIO_MC_MSK_CFG_CMD_DEL:
		dev_info->scratchpad[mask+SCRPAD_MASK_IDX] &= ~((uint32_t)(1) << (port + 16));
		break;
	case RIO_MC_MSK_CFG_CMD_DEL_ALL:
		dev_info->scratchpad[mask+SCRPAD_MASK_IDX] &= ~TSI578_RIO_MC_MSKX_MC_MSK;
		break;
	case RIO_MC_MSK_CFG_CMD_ADD_ALL:
		dev_info->scratchpad[mask+SCRPAD_MASK_IDX] |= TSI578_RIO_MC_MSKX_MC_MSK;
		break;
	default:
		break;
	}
}

static void tsi57x_WriteReg_destid_assoc(DAR_DEV_INFO_t *dev_info, uint8_t idx)
{
	uint8_t mask;
	uint32_t destid;
	bool large = (dev_info->scratchpad[idx] & RIO_MC_CON_OP_DEV16M);
	uint32_t cmd = (dev_info->scratchpad[idx] & RIO_MC_CON_OP_CMD);

	mask = (dev_info->scratchpad[idx - 1] & RIO_MC_CON_SEL_MASK);
	destid = dev_info->scratchpad[idx - 1] &
		(RIO_MC_CON_SEL_DEV8 | RIO_MC_CON_SEL_DEV16);

	/* Write to TSI578_RIO_MC_DESTID_ASSOC can update destID registers.
	 * Must emulate the effect, as it is not possible to trust the value
	 * of the destID register selected when port 0 is powered down.
	 */
	switch (cmd) {
	case RIO_MC_CON_OP_CMD_DEL:
		dev_info->scratchpad[mask] = 0;
		break;
	case RIO_MC_CON_OP_CMD_ADD:
		dev_info->scratchpad[mask] = (destid >> 16) |
		TSI578_RIO_MC_IDX_MC_EN
				| ((large) ? (TSI578_RIO_MC_IDX_LARGE_SYS) : 0);
		break;
	default:
		break;
	}
}

uint32_t tsi57x_WriteReg(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t writedata)
{
	const struct scrpad_info *scratchpad = tsi57x_get_scrpad_info();
	uint8_t idx;
	uint32_t rc;


	rc = WriteReg(dev_info, offset, writedata);
	if (RIO_SUCCESS == rc) {
		for (idx = SCRPAD_FIRST_IDX; idx < MAX_DAR_SCRPAD_IDX; idx++) {
			if (scratchpad[idx].offset == offset) {
				writedata &= scratchpad[idx].rw_mask;
				dev_info->scratchpad[idx] = writedata;

				switch (offset) {
				case TSI578_RIO_MC_MASK_CFG:
					tsi57x_WriteReg_mask_cfg(dev_info,
							writedata);
					break;
				case TSI578_RIO_MC_DESTID_ASSOC:
					if (idx) {
						tsi57x_WriteReg_destid_assoc(
								dev_info, idx);
					} else {
						rc = RIO_ERR_SW_FAILURE;
					}
					break;
				default:
					break;
				}
				break;
			}
		}
	}
	return rc;
}

uint32_t tsi57x_ReadReg(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t *readdata)
{
	const struct scrpad_info *scratchpad = tsi57x_get_scrpad_info();
	bool found_one = false;
	uint8_t idx ;

	for (idx = SCRPAD_FIRST_IDX; idx < MAX_DAR_SCRPAD_IDX; idx++) {
		if (scratchpad[idx].offset == offset) {
			switch (offset) {
			case TSI578_RIO_MC_DESTID_ASSOC:
			case TSI578_RIO_MC_MASK_CFG:
			case TSI578_RIO_MC_DESTID_CFG:
				continue;
			default:
				*readdata = dev_info->scratchpad[idx];
				found_one = true;
				continue;
			}
		}
	}

	if (!found_one) {
		return ReadReg( dev_info, offset, readdata );
	}
	return RIO_SUCCESS;
}

#endif /* TSI57X_DAR_WANTED */

#ifdef __cplusplus
}
#endif
