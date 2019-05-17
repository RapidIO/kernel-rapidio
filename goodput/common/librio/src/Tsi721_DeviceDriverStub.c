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

#include "DAR_DB_Private.h"
#include "DSF_DB_Private.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TSI721_DAR_WANTED

uint32_t tsi721_ReadReg(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t *readdata)
{
	return DARDB_ReadRegNoDriver(dev_info, offset, readdata);
}

uint32_t tsi721_WriteReg(DAR_DEV_INFO_t *dev_info, uint32_t offset,
		uint32_t writedata)
{
	return DARDB_WriteRegNoDriver(dev_info, offset, writedata);
}

// Device Access
//
uint32_t tsi721_rioDeviceSupported(DAR_DEV_INFO_t *dev_info)
{
	return DARDB_rioDeviceSupportedStub(dev_info);
}

// Error Management
//
uint32_t tsi721_rio_em_cfg_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_pw_in_t *in_parms, rio_em_cfg_pw_out_t *out_parms)
{
	return DSF_rio_em_cfg_pw(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_em_cfg_set(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_set_in_t *in_parms, rio_em_cfg_set_out_t *out_parms)
{
	return DSF_rio_em_cfg_set(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_em_cfg_get(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_get_in_t *in_parms, rio_em_cfg_get_out_t *out_parms)
{
	return DSF_rio_em_cfg_get(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_em_dev_rpt_ctl(DAR_DEV_INFO_t *dev_info,
		rio_em_dev_rpt_ctl_in_t *in_parms,
		rio_em_dev_rpt_ctl_out_t *out_parms)
{
	return DSF_rio_em_dev_rpt_ctl(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_em_parse_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_parse_pw_in_t *in_parms,
		rio_em_parse_pw_out_t *out_parms)
{
	return DSF_rio_em_parse_pw(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_em_get_int_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_int_stat_in_t *in_parms,
		rio_em_get_int_stat_out_t *out_parms)
{
	return DSF_rio_em_get_int_stat(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_em_get_pw_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_pw_stat_in_t *in_parms,
		rio_em_get_pw_stat_out_t *out_parms)
{
	return DSF_rio_em_get_pw_stat(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_em_clr_events(DAR_DEV_INFO_t *dev_info,
		rio_em_clr_events_in_t *in_parms,
		rio_em_clr_events_out_t *out_parms)
{
	return DSF_rio_em_clr_events(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_em_create_events(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_in_t *in_parms,
		rio_em_create_events_out_t *out_parms)
{
	return DSF_rio_em_create_events(dev_info, in_parms, out_parms);
}

// Port Config
//
uint32_t tsi721_rio_pc_get_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_config_in_t *in_parms,
		rio_pc_get_config_out_t *out_parms)
{
	return DSF_rio_pc_get_config(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_pc_set_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_set_config_in_t *in_parms,
		rio_pc_set_config_out_t *out_parms)
{
	return DSF_rio_pc_set_config(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_pc_get_status(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_status_in_t *in_parms,
		rio_pc_get_status_out_t *out_parms)
{
	return DSF_rio_pc_get_status(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_pc_reset_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_port_in_t *in_parms,
		rio_pc_reset_port_out_t *out_parms)
{
	return DSF_rio_pc_reset_port(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_pc_reset_link_partner(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_link_partner_in_t *in_parms,
		rio_pc_reset_link_partner_out_t *out_parms)
{
	return DSF_rio_pc_reset_link_partner(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_pc_clr_errs(DAR_DEV_INFO_t *dev_info,
		rio_pc_clr_errs_in_t *in_parms,
		rio_pc_clr_errs_out_t *out_parms)
{
	return DSF_rio_pc_clr_errs(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_pc_secure_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_secure_port_in_t *in_parms,
		rio_pc_secure_port_out_t *out_parms)
{
	return DSF_rio_pc_secure_port(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_pc_dev_reset_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_dev_reset_config_in_t *in_parms,
		rio_pc_dev_reset_config_out_t *out_parms)
{
	return DSF_rio_pc_dev_reset_config(dev_info, in_parms, out_parms);
}

// Statistics Counter
//
uint32_t tsi721_rio_sc_init_dev_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_init_dev_ctrs_in_t *in_parms,
		rio_sc_init_dev_ctrs_out_t *out_parms)
{
	return DSF_rio_sc_init_dev_ctrs(dev_info, in_parms, out_parms);
}

uint32_t tsi721_rio_sc_read_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_read_ctrs_in_t *in_parms,
		rio_sc_read_ctrs_out_t *out_parms)
{
	return DSF_rio_sc_read_ctrs(dev_info, in_parms, out_parms);
}

#endif /* TSI721_DAR_WANTED */

#ifdef __cplusplus
}
#endif
