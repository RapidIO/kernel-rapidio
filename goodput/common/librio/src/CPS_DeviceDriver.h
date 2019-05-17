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

#ifndef __CPS_DEVICEDRIVER_H__
#define __CPS_DEVICEDRIVER_H__

#include <stdint.h>
#include <stddef.h>

#include "RapidIO_Device_Access_Routines_API.h"
#include "RapidIO_Error_Management_API.h"
#include "RapidIO_Port_Config_API.h"
#include "RapidIO_Routing_Table_API.h"
#include "RapidIO_Statistics_Counter_API.h"
#include "CPS1848.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_CPS_PORTS(dev_info) ((NUM_PORTS(dev_info) > CPS_MAX_PORTS) ? \
		CPS_MAX_PORTS : NUM_PORTS(dev_info))

#define INVALID_PLL 0xFF
#define MAX_CPS_PLL 2
#define MAX_CPS_DEP_PORTS 3

typedef struct p_q_cfg_t_TAG {
	uint8_t lane_count;		// Number of lanes controlled by the port.
					//    0 means the port is unavailable
	uint8_t first_lane;		// Lane number of first lane controlled by
					//    the port.  Valid iff 0 != lane_count
	uint8_t pll[MAX_CPS_PLL];	// PLL number(s) for the port
	uint8_t other_ports[MAX_CPS_DEP_PORTS]; // Other ports which are dependent upon these PLLs.
} p_q_cfg_t;

typedef struct cps_port_relations_t_TAG {
	uint8_t port_num;	// Port number
	uint8_t quadrant;	// Quadrant which has the port
	p_q_cfg_t cfg[4];	// Information about the port for possible quadrant
				//    configuration setting values
} cps_port_relations_t;

typedef struct cps_quadrant_ports_t_TAG {
	uint8_t port_num[CPS_MAX_QUADRANT_PORTS];
} cps_quadrant_ports_t;

typedef struct cps_ports_per_quadrant_t_TAG {
	cps_quadrant_ports_t qdrt[4];
} cps_ports_per_quadrant_t;

typedef struct cps_port_info_t_TAG {
	uint32_t bitshift; // Separation between cfg_vals in quad_cfg register
	uint32_t quad_cfg;  // Quadrant configuration register
	uint8_t quad_cfg_val[4]; // Currently selected quadrant configs
	const cps_ports_per_quadrant_t *ppq; // List of ports per quadrant
	const cps_port_relations_t *cpr; // CPS port to lane/PLL mapping info
} cps_port_info_t;

// Device Access
//
uint32_t CPS_rioGetPortList(DAR_DEV_INFO_t *dev_info, struct DAR_ptl *ptl_in,
		struct DAR_ptl *ptl_out);

uint32_t CPS_rioSetAssmblyInfo(DAR_DEV_INFO_t *dev_info, uint32_t AsmblyVendID,
		uint16_t AsmblyRev);

uint32_t CPS_rioDeviceSupported(DAR_DEV_INFO_t *dev_info);

// Error Management
//
uint32_t CPS_rio_em_cfg_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_pw_in_t *in_parms,
		rio_em_cfg_pw_out_t *out_parms);

uint32_t CPS_rio_em_cfg_set(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_set_in_t *in_parms,
		rio_em_cfg_set_out_t *out_parms);

uint32_t CPS_rio_em_cfg_get(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_get_in_t *in_parms,
		rio_em_cfg_get_out_t *out_parms);

uint32_t CPS_rio_em_dev_rpt_ctl(DAR_DEV_INFO_t *dev_info,
		rio_em_dev_rpt_ctl_in_t *in_parms,
		rio_em_dev_rpt_ctl_out_t *out_parms);

uint32_t CPS_rio_em_parse_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_parse_pw_in_t *in_parms,
		rio_em_parse_pw_out_t *out_parms);

uint32_t CPS_rio_em_get_int_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_int_stat_in_t *in_parms,
		rio_em_get_int_stat_out_t *out_parms);

uint32_t CPS_rio_em_get_pw_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_pw_stat_in_t *in_parms,
		rio_em_get_pw_stat_out_t *out_parms);

uint32_t CPS_rio_em_clr_events(DAR_DEV_INFO_t *dev_info,
		rio_em_clr_events_in_t *in_parms,
		rio_em_clr_events_out_t *out_parms);

uint32_t CPS_rio_em_create_events(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_in_t *in_parms,
		rio_em_create_events_out_t *out_parms);

// Port Config
//
uint32_t CPS_rio_pc_get_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_config_in_t *in_parms,
		rio_pc_get_config_out_t *out_parms);

uint32_t CPS_rio_pc_set_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_set_config_in_t *in_parms,
		rio_pc_set_config_out_t *out_parms);

uint32_t CPS_rio_pc_get_status(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_status_in_t *in_parms,
		rio_pc_get_status_out_t *out_parms);

uint32_t CPS_rio_pc_reset_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_port_in_t *in_parms,
		rio_pc_reset_port_out_t *out_parms);

uint32_t CPS_rio_pc_reset_link_partner(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_link_partner_in_t *in_parms,
		rio_pc_reset_link_partner_out_t *out_parms);

uint32_t CPS_rio_pc_clr_errs(DAR_DEV_INFO_t *dev_info,
		rio_pc_clr_errs_in_t *in_parms,
		rio_pc_clr_errs_out_t *out_parms);

uint32_t CPS_rio_pc_secure_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_secure_port_in_t *in_parms,
		rio_pc_secure_port_out_t *out_parms);

uint32_t CPS_rio_pc_dev_reset_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_dev_reset_config_in_t *in_parms,
		rio_pc_dev_reset_config_out_t *out_parms);

// Routing Table
//
uint32_t CPS_rio_rt_initialize(DAR_DEV_INFO_t *dev_info,
		rio_rt_initialize_in_t *in_parms,
		rio_rt_initialize_out_t *out_parms);

uint32_t CPS_rio_rt_probe(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_in_t *in_parms,
		rio_rt_probe_out_t *out_parms);

uint32_t CPS_rio_rt_probe_all(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_all_in_t *in_parms,
		rio_rt_probe_all_out_t *out_parms);

uint32_t CPS_rio_rt_set_all(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_all_in_t *in_parms,
		rio_rt_set_all_out_t *out_parms);

uint32_t CPS_rio_rt_set_changed(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_changed_in_t *in_parms,
		rio_rt_set_changed_out_t *out_parms);

uint32_t CPS_rio_rt_change_rte(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_rte_in_t *in_parms,
		rio_rt_change_rte_out_t *out_parms);

uint32_t CPS_rio_rt_change_mc_mask(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_mc_mask_in_t *in_parms,
		rio_rt_change_mc_mask_out_t *out_parms);

// Statistics Counter
//
uint32_t CPS_rio_sc_init_dev_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_init_dev_ctrs_in_t *in_parms,
		rio_sc_init_dev_ctrs_out_t *out_parms);

uint32_t CPS_rio_sc_read_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_read_ctrs_in_t *in_parms,
		rio_sc_read_ctrs_out_t *out_parms);

#ifdef __cplusplus
}
#endif

#endif /* __CPS_DEVICEDRIVER_H__ */
