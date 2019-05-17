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

#ifndef __TSI57X_DEVICEDRIVER_H__
#define __TSI57X_DEVICEDRIVER_H__

#include <stdint.h>
#include <stddef.h>

#include "RapidIO_Device_Access_Routines_API.h"
#include "RapidIO_Error_Management_API.h"
#include "RapidIO_Port_Config_API.h"
#include "RapidIO_Routing_Table_API.h"
#include "RapidIO_Statistics_Counter_API.h"
#include "Tsi578.h"

#ifdef __cplusplus
extern "C" {
#endif


#define TSI57X_NUM_PORTS(x) ((NUM_PORTS(x) > TSI578_MAX_PORTS) ? \
		TSI578_MAX_PORTS : NUM_PORTS(x))

// Begin: Tsi57x_EM & Tsi57x_PC
#define MAX_OTHER_MAC_PORTS 3

typedef struct port_mac_relations_t_TAG {
	// Port number
	uint8_t port_num;
	// MAC number associated with configuration for this port.
	uint8_t mac_num;
	// Port associated with the first lane of the MAC
	uint8_t first_mac_lane;
	// If the MAC is in 4x mode, how many lanes?
	uint8_t lane_count_4x;
	// If the MAC is in 1x mode, how many lanes?
	uint8_t lane_count_1x;
	// MAC number for power control of this port.
	//     Usually the same as mac_num, not on Tsi577
	uint8_t pwr_mac_num;
	// Either TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4 or
	//     TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1
	uint32_t pwr_down_mask;
	// Either TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4 or
	//     TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1
	uint32_t rst_mask;
	// List of other MAC ports related to this MAC.
	//    Terminated with RIO_ALL_PORTS.
	uint8_t other_mac_ports[MAX_OTHER_MAC_PORTS];
} port_mac_relations_t;
// End: Tsi57x_EM & Tsi57x_PC

// Begin: Tsi57x_PC & Tsi75x_API
#define SCRPAD_EOF_OFFSET 0xFFFFFFFF
#define SCRPAD_FLAGS_IDX    0
#define SCRPAD_FIRST_IDX    0
#define SCRPAD_MASK_IDX     (SCRPAD_FIRST_IDX+TSI578_MAX_MC_MASKS)

struct scrpad_info {
	uint32_t offset;
	uint32_t rw_mask;
};
// End: Tsi57x_PC & Tsi75x_API


// Device Access
//
uint32_t tsi57x_rioDeviceSupported(DAR_DEV_INFO_t *dev_info);

// Error Management
//
uint32_t tsi57x_rio_em_cfg_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_pw_in_t *in_parms,
		rio_em_cfg_pw_out_t *out_parms);

uint32_t tsi57x_rio_em_cfg_set(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_set_in_t *in_parms,
		rio_em_cfg_set_out_t *out_parms);

uint32_t tsi57x_rio_em_cfg_get(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_get_in_t *in_parms,
		rio_em_cfg_get_out_t *out_parms);

uint32_t tsi57x_rio_em_dev_rpt_ctl(DAR_DEV_INFO_t *dev_info,
		rio_em_dev_rpt_ctl_in_t *in_parms,
		rio_em_dev_rpt_ctl_out_t *out_parms);

uint32_t tsi57x_rio_em_parse_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_parse_pw_in_t *in_parms,
		rio_em_parse_pw_out_t *out_parms);

uint32_t tsi57x_rio_em_get_int_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_int_stat_in_t *in_parms,
		rio_em_get_int_stat_out_t *out_parms);

uint32_t tsi57x_rio_em_get_pw_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_pw_stat_in_t *in_parms,
		rio_em_get_pw_stat_out_t *out_parms);

uint32_t tsi57x_rio_em_clr_events(DAR_DEV_INFO_t *dev_info,
		rio_em_clr_events_in_t *in_parms,
		rio_em_clr_events_out_t *out_parms);

uint32_t tsi57x_rio_em_create_events(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_in_t *in_parms,
		rio_em_create_events_out_t *out_parms);

// Port Config
//
uint32_t tsi57x_rio_pc_get_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_config_in_t *in_parms,
		rio_pc_get_config_out_t *out_parms);

uint32_t tsi57x_rio_pc_set_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_set_config_in_t *in_parms,
		rio_pc_set_config_out_t *out_parms);

uint32_t tsi57x_rio_pc_get_status(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_status_in_t *in_parms,
		rio_pc_get_status_out_t *out_parms);

uint32_t tsi57x_rio_pc_reset_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_port_in_t *in_parms,
		rio_pc_reset_port_out_t *out_parms);

uint32_t tsi57x_rio_pc_reset_link_partner(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_link_partner_in_t *in_parms,
		rio_pc_reset_link_partner_out_t *out_parms);

uint32_t tsi57x_rio_pc_clr_errs(DAR_DEV_INFO_t *dev_info,
		rio_pc_clr_errs_in_t *in_parms,
		rio_pc_clr_errs_out_t *out_parms);

uint32_t tsi57x_rio_pc_secure_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_secure_port_in_t *in_parms,
		rio_pc_secure_port_out_t *out_parms);

uint32_t tsi57x_rio_pc_dev_reset_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_dev_reset_config_in_t *in_parms,
		rio_pc_dev_reset_config_out_t *out_parms);

// Routing Table
//
uint32_t tsi57x_rio_rt_initialize(DAR_DEV_INFO_t *dev_info,
		rio_rt_initialize_in_t *in_parms,
		rio_rt_initialize_out_t *out_parms);

uint32_t tsi57x_rio_rt_probe(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_in_t *in_parms,
		rio_rt_probe_out_t *out_parms);

uint32_t tsi57x_rio_rt_probe_all(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_all_in_t *in_parms,
		rio_rt_probe_all_out_t *out_parms);

uint32_t tsi57x_rio_rt_set_all(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_all_in_t *in_parms,
		rio_rt_set_all_out_t *out_parms);

uint32_t tsi57x_rio_rt_set_changed(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_changed_in_t *in_parms,
		rio_rt_set_changed_out_t *out_parms);

uint32_t tsi57x_rio_rt_change_rte(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_rte_in_t *in_parms,
		rio_rt_change_rte_out_t *out_parms);

uint32_t tsi57x_rio_rt_change_mc_mask(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_mc_mask_in_t *in_parms,
		rio_rt_change_mc_mask_out_t *out_parms);

// Statistics Counter
//
uint32_t tsi57x_rio_sc_init_dev_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_init_dev_ctrs_in_t *in_parms,
		rio_sc_init_dev_ctrs_out_t *out_parms);

uint32_t tsi57x_rio_sc_read_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_read_ctrs_in_t *in_parms,
		rio_sc_read_ctrs_out_t *out_parms);

#ifdef __cplusplus
}
#endif

#endif /* __TSI57X_DEVICEDRIVER_H__ */
