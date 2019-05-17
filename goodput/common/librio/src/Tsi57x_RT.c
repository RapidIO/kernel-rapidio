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
#include <stdbool.h>
#include <stddef.h>

#include "rio_standard.h"
#include "rio_ecosystem.h"
#include "RapidIO_Device_Access_Routines_API.h"
#include "RapidIO_Routing_Table_API.h"
#include "Tsi57x_DeviceDriver.h"
#include "Tsi578.h"
#include "DSF_DB_Private.h"
#include "rio_standard.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TSI57X_DAR_WANTED

#define HW_DFLT_RT 0xFF

#define PROGRAM_RTE_ENTRIES_0 (RT_FIRST_SUBROUTINE_0+0xA000) // 10A000
#define PROGRAM_MC_MASKS_0    (RT_FIRST_SUBROUTINE_0+0xB000) // 10B000
#define READ_MC_MASKS_0       (RT_FIRST_SUBROUTINE_0+0xC000) // 10C000
#define READ_RTE_ENTRIES_0    (RT_FIRST_SUBROUTINE_0+0xD000) // 10D000

#define READ_MC_MASKS(x) (READ_MC_MASKS_0+x)
#define READ_RTE_ENTRIES(x) (READ_RTE_ENTRIES_0+x)

#define RT_PROBE_ALL(x) (RT_PROBE_ALL_0+x)

#define ALL_ENTRIES true
#define CHG_ENTRIES false
#define PROGRAM_RTE_ENTRIES(x) (PROGRAM_RTE_ENTRIES_0+x)

#define ALL_MASKS true
#define CHG_MASKS false
#define PROGRAM_MC_MASKS(x) (PROGRAM_MC_MASKS_0+x)

static uint32_t tsi57x_read_mc_masks(DAR_DEV_INFO_t *dev_info,
		rio_rt_state_t *rt, uint32_t *imp_rc)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	pe_rt_val mask_idx;
	uint32_t reg_val;
	rio_rt_dealloc_mc_mask_in_t d_in_parm;
	rio_rt_dealloc_mc_mask_out_t d_out_parm;

	d_in_parm.rt = rt;
	for (mask_idx = TSI578_MAX_MC_MASKS; mask_idx < RIO_MAX_MC_MASKS;
			mask_idx++) {
		d_in_parm.mc_mask_rte = RIO_RTV_MC_MSK(mask_idx);
		rc = DSF_rio_rt_dealloc_mc_mask(dev_info, &d_in_parm,
				&d_out_parm);
		if (RIO_SUCCESS != rc) {
			*imp_rc = d_out_parm.imp_rc;
			goto exit;
		}
	}

	for (mask_idx = 0; mask_idx < TSI578_MAX_MC_MASKS; mask_idx++) {
		rc = DARRegRead(dev_info, TSI578_RIO_MC_IDX(mask_idx),
				&reg_val);
		if (RIO_SUCCESS != rc) {
			*imp_rc = READ_MC_MASKS(1);
			goto exit;
		}

		rt->mc_masks[mask_idx].allocd = false;
		rt->mc_masks[mask_idx].changed = false;
		rt->mc_masks[mask_idx].tt =
				(reg_val & TSI578_RIO_MC_IDX_LARGE_SYS) ?
						tt_dev16 : tt_dev8;
		rt->mc_masks[mask_idx].in_use =
				(reg_val & TSI578_RIO_MC_IDX_MC_EN) ?
				true :
									false;
		rt->mc_masks[mask_idx].mc_destID =
				(reg_val
						& ((tt_dev16
								== rt->mc_masks[mask_idx].tt) ?
								TSI578_RIO_MC_IDX_MC_ID :
								(TSI578_RIO_MC_IDX_MC_ID
										>> 8)));

		rc = DARRegRead(dev_info, TSI578_RIO_MC_MSKX(mask_idx),
				&reg_val);
		if (RIO_SUCCESS != rc) {
			*imp_rc = READ_MC_MASKS(2);
			goto exit;
		}

		if (reg_val & ~TSI578_RIO_MC_MSKX_MC_MSK) {
			rc = RIO_ERR_RT_CORRUPTED;
			*imp_rc = READ_MC_MASKS(3);
			goto exit;
		}

		rt->mc_masks[mask_idx].mc_mask = (reg_val
				& TSI578_RIO_MC_MSKX_MC_MSK) >> 16;
	}

exit:
	return rc;
}

static uint32_t program_mc_masks(DAR_DEV_INFO_t *dev_info, rio_rt_state_t *rt,
bool prog_all,  // Use ALL_MASKS or CHG_MASKS
		uint32_t *imp_rc)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	pe_rt_val mask_idx;
	uint32_t reg_val;
	uint32_t invalid_mc_mask = ~(uint32_t)((1 << TSI57X_NUM_PORTS(dev_info))
			- 1);

	for (mask_idx = TSI578_MAX_MC_MASKS; mask_idx < RIO_MAX_MC_MASKS;
			mask_idx++) {
		if (rt->mc_masks[mask_idx].in_use
				|| rt->mc_masks[mask_idx].changed
				|| rt->mc_masks[mask_idx].allocd) {
			*imp_rc = PROGRAM_MC_MASKS(1);
			goto exit;
		}
	}

	for (mask_idx = 0; mask_idx < TSI578_MAX_MC_MASKS; mask_idx++) {
		if (invalid_mc_mask & rt->mc_masks[mask_idx].mc_mask) {
			*imp_rc = PROGRAM_MC_MASKS(2);
			goto exit;
		}
		if (prog_all || rt->mc_masks[mask_idx].changed) {
			rt->mc_masks[mask_idx].changed = false;
			reg_val = (uint32_t)(rt->mc_masks[mask_idx].mc_destID);
			reg_val |= (tt_dev16 == rt->mc_masks[mask_idx].tt) ?
					TSI578_RIO_MC_IDX_LARGE_SYS : 0;
			reg_val |= (rt->mc_masks[mask_idx].in_use) ?
					TSI578_RIO_MC_IDX_MC_EN : 0;
			rc = DARRegWrite(dev_info, TSI578_RIO_MC_IDX(mask_idx),
					reg_val);
			if (RIO_SUCCESS != rc) {
				*imp_rc = PROGRAM_MC_MASKS(3);
				goto exit;
			}

			reg_val = (rt->mc_masks[mask_idx].mc_mask << 16)
					& TSI578_RIO_MC_MSKX_MC_MSK;
			rc = DARRegWrite(dev_info, TSI578_RIO_MC_MSKX(mask_idx),
					reg_val);
			if (RIO_SUCCESS != rc) {
				*imp_rc = PROGRAM_MC_MASKS(4);
				goto exit;
			}
		}
	}

	rc = RIO_SUCCESS;
	*imp_rc = RIO_SUCCESS;

exit:
	return rc;
}

static uint32_t tsi57x_read_rte_entries(DAR_DEV_INFO_t *dev_info, uint8_t pnum,
		rio_rt_state_t *rt, uint32_t *imp_rc)
{
	uint32_t rc;
	uint32_t destID, idx_val, rte_val, base_reg;

	// Fill in default route value

	rc = DARRegRead(dev_info, TSI578_RIO_LUT_ATTR, &rte_val);
	if (RIO_SUCCESS != rc) {
		*imp_rc = READ_RTE_ENTRIES(1);
		goto exit;
	}

	if ( HW_DFLT_RT == (rte_val & TSI578_RIO_LUT_ATTR_DEFAULT_PORT)) {
		rt->default_route = RIO_RTE_DROP;
	} else {
		rt->default_route = rte_val & TSI578_RIO_LUT_ATTR_DEFAULT_PORT;
	}

	// Determine the base id for hierarchical mode.
	rc = DARRegRead(dev_info, TSI578_SPX_ROUTE_BASE(pnum), &base_reg);
	if (RIO_SUCCESS != rc) {
		*imp_rc = READ_RTE_ENTRIES(10);
		goto exit;
	}

	// Read all of the domain routing table entries.
	//
	for (destID = 0; destID < RIO_RT_GRP_SZ; destID++) {
		rt->dom_table[destID].changed = false;

		// Set deviceID, read routing table entry for deviceID
		idx_val = destID << 8;
		rc = DARRegWrite(dev_info, TSI578_SPX_ROUTE_CFG_DESTID(pnum),
				idx_val);
		if (RIO_SUCCESS != rc) {
			*imp_rc = READ_RTE_ENTRIES(7);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_ROUTE_CFG_PORT(pnum),
				&rte_val);
		if (RIO_SUCCESS != rc) {
			*imp_rc = READ_RTE_ENTRIES(8);
			goto exit;
		}

		if (HW_DFLT_RT == rte_val) {
			rt->dom_table[destID].rte_val =
					RIO_RTE_DFLT_PORT;
		} else {
			rt->dom_table[destID].rte_val = (uint8_t)(rte_val
					& TSI578_SPX_ROUTE_CFG_PORT_PORT);
		}
	}

	destID = (base_reg & TSI578_SPX_ROUTE_BASE_BASE) >> 24;
	rt->dom_table[destID].rte_val = RIO_RTE_LVL_G0;
	base_reg = destID << 8;

	// Read all of the device routing table entries.
	//
	//
	for (destID = 0; destID < RIO_RT_GRP_SZ; destID++) {
		rt->dev_table[destID].changed = false;

		// Set deviceID, read routing table entry for deviceID,
		idx_val = base_reg + destID;

		rc = DARRegWrite(dev_info, TSI578_SPX_ROUTE_CFG_DESTID(pnum),
				idx_val);
		if (RIO_SUCCESS != rc) {
			*imp_rc = READ_RTE_ENTRIES(3);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_ROUTE_CFG_PORT(pnum),
				&rte_val);
		if (RIO_SUCCESS != rc) {
			*imp_rc = READ_RTE_ENTRIES(4);
			goto exit;
		}

		if (HW_DFLT_RT == rte_val) {
			rt->dev_table[destID].rte_val =
					RIO_RTE_DFLT_PORT;
		} else {
			rt->dev_table[destID].rte_val = (uint8_t)(rte_val
					& TSI578_SPX_ROUTE_CFG_PORT_PORT);
		}
	}

exit:
	return rc;
}

static uint32_t program_rte_entries(DAR_DEV_INFO_t *dev_info,
		rio_rt_state_t *rt, uint8_t pnum,
		bool prog_all, // Use ALL_ENTRIES/CHG_ENTRIES
		uint32_t *imp_rc)
{

	uint32_t rc;
	uint32_t destID, baseID = 0;
	uint32_t rte_val, idx_val;
	bool set_base = false;
	uint8_t port, start_port, end_port;

	if (RIO_ALL_PORTS == pnum) {
		start_port = 0;
		end_port = TSI57X_NUM_PORTS(dev_info) - 1;
	} else {
		start_port = end_port = pnum;
	}

	// Set the default route output port

	if ( RIO_RTE_DROP == rt->default_route) {
		rte_val = HW_DFLT_RT & TSI578_RIO_LUT_ATTR_DEFAULT_PORT;
	} else {
		rte_val = rt->default_route & TSI578_RIO_LUT_ATTR_DEFAULT_PORT;
	}

	rc = DARRegWrite(dev_info, TSI578_RIO_LUT_ATTR, rte_val);
	if (RIO_SUCCESS != rc) {
		*imp_rc = PROGRAM_RTE_ENTRIES(1);
		goto exit;
	}

	// Find base ID, and set it.
	for (destID = 0; destID < RIO_RT_GRP_SZ; destID++) {
		idx_val = destID << 8;
		rte_val = rt->dom_table[destID].rte_val;
		if (RIO_RTE_LVL_G0 == rte_val) {
			if (set_base) {
				rc = RIO_ERR_INVALID_PARAMETER;
				*imp_rc = PROGRAM_RTE_ENTRIES(2);
				goto exit;
			} else {
				set_base = true;
				baseID = idx_val;
			}
		}
	}

	for (port = start_port; port <= end_port; port++) {
		rc = DARRegWrite(dev_info, TSI578_SPX_ROUTE_BASE(port),
				baseID << 16);
		if (RIO_SUCCESS != rc) {
			*imp_rc = PROGRAM_RTE_ENTRIES(6);
			goto exit;
		}
	}

	// Set all of the domain routing table entries
	for (destID = 0; destID < RIO_RT_GRP_SZ; destID++) {
		if (prog_all || rt->dom_table[destID].changed) {
			idx_val = destID << 8;
			rte_val = rt->dom_table[destID].rte_val;

			if (RIO_RTE_LVL_G0 != rte_val) {
				if (RIO_RTE_DFLT_PORT == rte_val) {
					rte_val = HW_DFLT_RT;
				} else {
					if (RIO_RTE_DROP == rte_val) {
						rte_val = HW_DFLT_RT;
						idx_val |=
								TSI578_SPX_ROUTE_CFG_DESTID_PAR_INVERT;
					} else {
						if (TSI57X_NUM_PORTS(dev_info)
								<= rte_val) {
							rc =
									RIO_ERR_INVALID_PARAMETER;
							*imp_rc =
									PROGRAM_RTE_ENTRIES(
											3);
							goto exit;
						}
					}
				}
				for (port = start_port; port <= end_port;
						port++) {
					rc =
							DARRegWrite(dev_info,
									TSI578_SPX_ROUTE_CFG_DESTID(
											port),
									idx_val);
					if (RIO_SUCCESS != rc) {
						*imp_rc = PROGRAM_RTE_ENTRIES(
								4);
						goto exit;
					}

					rc =
							DARRegWrite(dev_info,
									TSI578_SPX_ROUTE_CFG_PORT(
											port),
									rte_val);
					if (RIO_SUCCESS != rc) {
						*imp_rc = PROGRAM_RTE_ENTRIES(
								5);
						goto exit;
					}
				}
			}
			rt->dom_table[destID].changed = false;
		}
	}

	// Set all of the device routing table entries
	for (destID = 0; destID < RIO_RT_GRP_SZ; destID++) {
		if (prog_all || rt->dev_table[destID].changed) {
			idx_val = baseID + destID;
			rte_val = rt->dev_table[destID].rte_val;
			if (RIO_RTE_DFLT_PORT == rte_val) {
				rte_val = HW_DFLT_RT;
			} else {
				if (RIO_RTE_DROP == rte_val) {
					rte_val = HW_DFLT_RT;
					idx_val |=
							TSI578_SPX_ROUTE_CFG_DESTID_PAR_INVERT;
				} else {
					if ((RIO_RTE_LVL_G0 == rte_val)
							|| (TSI57X_NUM_PORTS(
									dev_info)
									<= rte_val)) {
						rc = RIO_ERR_INVALID_PARAMETER;
						*imp_rc = PROGRAM_RTE_ENTRIES(
								7);
						goto exit;
					}
				}
			}

			for (port = start_port; port <= end_port; port++) {
				rc = DARRegWrite(dev_info,
						TSI578_SPX_ROUTE_CFG_DESTID(
								port), idx_val);
				if (RIO_SUCCESS != rc) {
					*imp_rc = PROGRAM_RTE_ENTRIES(8);
					goto exit;
				}

				rc = DARRegWrite(dev_info,
						TSI578_SPX_ROUTE_CFG_PORT(port),
						rte_val);
				if (RIO_SUCCESS != rc) {
					*imp_rc = PROGRAM_RTE_ENTRIES(9);
					goto exit;
				}
			}
			rt->dev_table[destID].changed = false;
		}
	}

	rc = RIO_SUCCESS;
	*imp_rc = RIO_SUCCESS;

exit:
	return rc;
}

static uint32_t tsi_check_port_for_discard(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_in_t *in_parms, rio_rt_probe_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t ctlData;
	uint8_t port;
	bool dflt_rt = (RIO_RTE_DFLT_PORT
			== out_parms->routing_table_value) ? true : false;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	rio_pc_get_status_in_t stat_in;
	rio_pc_get_status_out_t stat_out;

	port = (dflt_rt) ?
			in_parms->rt->default_route :
			out_parms->routing_table_value;

	if (TSI57X_NUM_PORTS(dev_info) <= port) {
		out_parms->reason_for_discard = rio_rt_disc_probe_abort;
		out_parms->imp_rc = RT_PROBE(1);
		goto exit;
	}

	cfg_in.ptl.num_ports = 1;
	cfg_in.ptl.pnums[0] = port;
	rc = tsi57x_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->reason_for_discard = rio_rt_disc_probe_abort;
		out_parms->imp_rc = RT_PROBE(2);
		goto exit;
	}

	stat_in.ptl.num_ports = 1;
	stat_in.ptl.pnums[0] = port;
	rc = tsi57x_rio_pc_get_status(dev_info, &stat_in, &stat_out);
	if (RIO_SUCCESS != rc) {
		out_parms->reason_for_discard = rio_rt_disc_probe_abort;
		out_parms->imp_rc = RT_PROBE(3);
		goto exit;
	}

	if (!cfg_out.pc[0].port_available) {
		out_parms->reason_for_discard =
				(dflt_rt) ? rio_rt_disc_dflt_pt_unavail : rio_rt_disc_port_unavail;
	} else {
		if (!cfg_out.pc[0].powered_up) {
			out_parms->reason_for_discard =
					(dflt_rt) ? rio_rt_disc_dflt_pt_pwdn : rio_rt_disc_port_pwdn;
		} else {
			if (!stat_out.ps[0].port_ok) {
				if (cfg_out.pc[0].xmitter_disable) {
					out_parms->reason_for_discard =
							(dflt_rt) ? rio_rt_disc_dflt_pt_lkout_or_dis : rio_rt_disc_port_lkout_or_dis;
				} else {
					out_parms->reason_for_discard =
							(dflt_rt) ? rio_rt_disc_dflt_pt_no_lp : rio_rt_disc_port_no_lp;
				}
			} else {
				if (stat_out.ps[0].port_error) {
					out_parms->reason_for_discard =
							(dflt_rt) ? rio_rt_disc_dflt_pt_fail : rio_rt_disc_port_fail;
				} else {
					if (cfg_out.pc[0].port_lockout) {
						out_parms->reason_for_discard =
								(dflt_rt) ? rio_rt_disc_dflt_pt_lkout_or_dis : rio_rt_disc_port_lkout_or_dis;
					} else {
						rc =
								DARRegRead(
										dev_info,
										TSI578_SPX_CTL(
												port),
										&ctlData);
						if (RIO_SUCCESS != rc) {
							out_parms->reason_for_discard =
									rio_rt_disc_probe_abort;
							out_parms->imp_rc =
									RT_PROBE(
											4);
							goto exit;
						}

						if ((RIO_SPX_CTL_INP_EN
								| RIO_SPX_CTL_OTP_EN)
								!= ((RIO_SPX_CTL_INP_EN
										| RIO_SPX_CTL_OTP_EN)
										& ctlData)) {
							out_parms->reason_for_discard =
									(dflt_rt) ? rio_rt_disc_dflt_pt_in_out_dis : rio_rt_disc_port_in_out_dis;
						}
					}
				}
			}
		}
	}

	rc = RIO_SUCCESS;

exit:
	if (rio_rt_disc_not != out_parms->reason_for_discard) {
		out_parms->valid_route = false;
	}
	return rc;
}

uint32_t tsi57x_rio_rt_initialize(DAR_DEV_INFO_t *dev_info,
		rio_rt_initialize_in_t *in_parms,
		rio_rt_initialize_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t destID, spx_mode;
	uint32_t mc_idx;
	uint8_t port, start_port, end_port;
	rio_rt_set_changed_in_t all_in;
	rio_rt_set_changed_out_t all_out;
	rio_rt_state_t rt_state;

	// Validate parameters

	if (((in_parms->default_route >= TSI57X_NUM_PORTS(dev_info))
			&& !((RIO_RTE_DFLT_PORT
					== in_parms->default_route)
					|| (RIO_RTE_DROP
							== in_parms->default_route)))) {
		out_parms->imp_rc = RT_INITIALIZE(1);
		goto exit;
	}

	if ((in_parms->default_route_table_port >= TSI57X_NUM_PORTS(dev_info))
			&& !(RIO_RTE_DROP
					== in_parms->default_route_table_port)) {
		out_parms->imp_rc = RT_INITIALIZE(2);
		goto exit;
	}

	if ((in_parms->set_on_port >= TSI57X_NUM_PORTS(dev_info))
			&& !(RIO_ALL_PORTS == in_parms->set_on_port)) {
		out_parms->imp_rc = RT_INITIALIZE(3);
		goto exit;
	}

	out_parms->imp_rc = RIO_SUCCESS;
	all_in.set_on_port = in_parms->set_on_port;

	if (NULL == in_parms->rt) {
		all_in.rt = &rt_state;
	} else {
		all_in.rt = in_parms->rt;
	}

	all_in.rt->default_route = in_parms->default_route;

	// Ensure routing tables are operating in hierarchical mode

	if (in_parms->update_hw) {
		if (RIO_ALL_PORTS == in_parms->set_on_port) {
			start_port = 0;
			end_port = TSI57X_NUM_PORTS(dev_info) - 1;
		} else {
			start_port = end_port = in_parms->set_on_port;
		}

		for (port = start_port; port <= end_port; port++) {
			rc = DARRegRead(dev_info, TSI578_SPX_MODE(port),
					&spx_mode);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = RT_INITIALIZE(4);
				goto exit;
			}

			spx_mode &= ~TSI578_SPX_MODE_LUT_512;

			rc = DARRegWrite(dev_info, TSI578_SPX_MODE(port),
					spx_mode);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = RT_INITIALIZE(5);
				goto exit;
			}
		}
	}

	// Configure initialization of all of the routing table entries
	for (destID = 0; destID < RIO_RT_GRP_SZ; destID++) {
		all_in.rt->dev_table[destID].changed = true;
		all_in.rt->dev_table[destID].rte_val =
				in_parms->default_route_table_port;
	}

	all_in.rt->dom_table[0].changed = true;
	all_in.rt->dom_table[0].rte_val = RIO_RTE_LVL_G0;

	for (destID = 1; destID < RIO_RT_GRP_SZ; destID++) {
		all_in.rt->dom_table[destID].changed = true;
		all_in.rt->dom_table[destID].rte_val =
				in_parms->default_route_table_port;
	}

	// Configure initialization of multicast masks and associations as necessary.
	for (mc_idx = 0; mc_idx < RIO_MAX_MC_MASKS; mc_idx++) {
		all_in.rt->mc_masks[mc_idx].mc_destID = 0;
		all_in.rt->mc_masks[mc_idx].tt = tt_dev8;
		all_in.rt->mc_masks[mc_idx].mc_mask = 0;
		all_in.rt->mc_masks[mc_idx].in_use = false;
		all_in.rt->mc_masks[mc_idx].allocd = false;
		// Only change multicast masks that exist AND
		// when all ports are being initialized.
		if ((mc_idx < TSI578_MAX_MC_MASKS)
				&& (RIO_ALL_PORTS == in_parms->set_on_port)) {
			all_in.rt->mc_masks[mc_idx].changed = true;
		} else {
			all_in.rt->mc_masks[mc_idx].changed = false;
		}
	}

	if (in_parms->update_hw) {
		rc = tsi57x_rio_rt_set_changed(dev_info, &all_in, &all_out);
	} else {
		rc = RIO_SUCCESS;
	}

	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = all_out.imp_rc;
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_rt_probe(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_in_t *in_parms, rio_rt_probe_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t bit;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->valid_route = false;
	out_parms->routing_table_value = RIO_ALL_PORTS;
	out_parms->filter_function_active = false; /* not supported on Tsi */
	out_parms->trace_function_active = false; /* not supported on Tsi */
	out_parms->time_to_live_active = false; /* not supported on Tsi */
	for (bit = 0; bit < TSI57X_NUM_PORTS(dev_info); bit++)
		out_parms->mcast_ports[bit] = false;
	out_parms->reason_for_discard = rio_rt_disc_probe_abort;

	if (((TSI57X_NUM_PORTS(dev_info) <= in_parms->probe_on_port)
			&& (RIO_ALL_PORTS != in_parms->probe_on_port))
			|| ( NULL == in_parms->rt)) {
		out_parms->imp_rc = RT_PROBE(0x11);
		goto exit;
	}

	rc = RIO_SUCCESS;

	// Note, no failure possible...
	rio_rt_check_multicast_routing(dev_info, in_parms, out_parms);

	/* Done if hit in multicast masks. */
	if (RIO_ALL_PORTS != out_parms->routing_table_value) {
		goto exit;
	}

	/*  Determine routing table value for the specified destination ID.
	 *  If out_parms->valid_route is true
	 *  the valid values for out_parms->routing_table_value are
	 *  - a valid port number, OR
	 *  - RIO_RTE_DFLT_PORT
	 *  When out_parms->routing_table_value is RIO_RTE_DFLT_PORT, the
	 *  default route is a valid switch port number.
	 */

	rio_rt_check_unicast_routing(dev_info, in_parms, out_parms);

	if (out_parms->valid_route) {
		rc = tsi_check_port_for_discard(dev_info, in_parms, out_parms);
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_rt_probe_all(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_all_in_t *in_parms,
		rio_rt_probe_all_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t probe_port;

	out_parms->imp_rc = RIO_SUCCESS;
	if ((((uint8_t)(RIO_ALL_PORTS) != in_parms->probe_on_port)
			&& (in_parms->probe_on_port
					>= TSI57X_NUM_PORTS(dev_info)))
			|| ( NULL == in_parms->rt)) {
		out_parms->imp_rc = RT_PROBE_ALL(1);
		goto exit;
	}

	probe_port = (RIO_ALL_PORTS == in_parms->probe_on_port) ?
			0 : in_parms->probe_on_port;

	rc = tsi57x_read_mc_masks(dev_info, in_parms->rt, &out_parms->imp_rc);
	if (RIO_SUCCESS != rc)
		goto exit;

	rc = tsi57x_read_rte_entries(dev_info, probe_port, in_parms->rt,
			&out_parms->imp_rc);

exit:
	return rc;
}

uint32_t tsi57x_rio_rt_set_all(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_all_in_t *in_parms, rio_rt_set_all_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;

	if ((((uint8_t)(RIO_ALL_PORTS) != in_parms->set_on_port)
			&& (in_parms->set_on_port >= TSI57X_NUM_PORTS(dev_info)))
			|| ( NULL == in_parms->rt)) {
		out_parms->imp_rc = RT_SET_ALL(1);
		goto exit;
	}

	if ((TSI57X_NUM_PORTS(dev_info) <= in_parms->rt->default_route)
			&& !(RIO_RTE_DROP == in_parms->rt->default_route)) {
		out_parms->imp_rc = RT_SET_ALL(2);
		goto exit;
	}

	rc = program_mc_masks(dev_info, in_parms->rt, ALL_MASKS,
			&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	rc = program_rte_entries(dev_info, in_parms->rt, in_parms->set_on_port,
			ALL_ENTRIES, &out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_rt_set_changed(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_changed_in_t *in_parms,
		rio_rt_set_changed_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;

	if ((((uint8_t)(RIO_ALL_PORTS) != in_parms->set_on_port)
			&& (in_parms->set_on_port >= TSI57X_NUM_PORTS(dev_info)))
			|| ( NULL == in_parms->rt)) {
		out_parms->imp_rc = RT_SET_CHANGED(1);
		goto exit;
	}

	if ((TSI57X_NUM_PORTS(dev_info) <= in_parms->rt->default_route)
			&& !(RIO_RTE_DROP == in_parms->rt->default_route)) {
		out_parms->imp_rc = RT_SET_CHANGED(2);
		goto exit;
	}

	out_parms->imp_rc = RIO_SUCCESS;
	rc = program_mc_masks(dev_info, in_parms->rt, CHG_MASKS,
			&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	rc = program_rte_entries(dev_info, in_parms->rt, in_parms->set_on_port,
			CHG_ENTRIES, &out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_rt_change_rte(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_rte_in_t *in_parms,
		rio_rt_change_rte_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint16_t idx;

	out_parms->imp_rc = RIO_SUCCESS;

	if (NULL == in_parms->rt) {
		out_parms->imp_rc = RT_CHANGE_RTE(1);
		goto exit;
	}

	// Validate rte_value
	if ((RIO_RTE_LVL_G0 != in_parms->rte_value)
			&& (RIO_RTE_DFLT_PORT != in_parms->rte_value)
			&& (RIO_RTE_DROP != in_parms->rte_value)
			&& (in_parms->rte_value >= TSI57X_NUM_PORTS(dev_info))) {
		out_parms->imp_rc = RT_CHANGE_RTE(2);
		goto exit;
	}

	if ((RIO_RTE_LVL_G0 == in_parms->rte_value)
			&& (!in_parms->dom_entry)) {
		out_parms->imp_rc = RT_CHANGE_RTE(3);
		goto exit;
	}

	rc = RIO_SUCCESS;

	// If entry has not already been changed, see if it is being changed
	if (in_parms->dom_entry) {
		//@sonar:off - Collapsible "if" statements should be merged
		if (!in_parms->rt->dom_table[in_parms->idx].changed) {
			if (in_parms->rt->dom_table[in_parms->idx].rte_val
					!= in_parms->rte_value) {
				in_parms->rt->dom_table[in_parms->idx].changed =
						true;
			}
		}
		//@sonar:on
		in_parms->rt->dom_table[in_parms->idx].rte_val =
				in_parms->rte_value;

		// Since only one entry in the domain table can have a value of
		// RIO_RTE_LVL_G0, if that entry is marked changed
		// then it is possible that another entry has this value.  Search
		// to clear all other entries with this value, and mark them changed.
		if (RIO_RTE_LVL_G0 == in_parms->rte_value) {
			for (idx = 0; idx < RIO_RT_GRP_SZ; idx++) {
				if ((RIO_RTE_LVL_G0
						== in_parms->rt->dom_table[idx].rte_val)
						&& (idx != in_parms->idx)) {
					in_parms->rt->dom_table[idx].rte_val =
							RIO_RTE_DROP;
					in_parms->rt->dom_table[idx].changed =
							true;
				}
			}
		}
	} else {
		//@sonar:off - Collapsible "if" statements should be merged
		if (!in_parms->rt->dev_table[in_parms->idx].changed) {
			if (in_parms->rt->dev_table[in_parms->idx].rte_val
					!= in_parms->rte_value) {
				in_parms->rt->dev_table[in_parms->idx].changed =
						true;
			}
		}
		//@sonar:on
		in_parms->rt->dev_table[in_parms->idx].rte_val =
				in_parms->rte_value;
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_rt_change_mc_mask(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_mc_mask_in_t *in_parms,
		rio_rt_change_mc_mask_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t mc_idx;
	uint8_t chg_idx;
	uint32_t illegal_ports = ~((1 << TSI578_MAX_PORTS) - 1);

	out_parms->imp_rc = RIO_SUCCESS;

	if ((NULL == in_parms->rt) || (NULL == dev_info)) {
		out_parms->imp_rc = CHANGE_MC_MASK(1);
		goto exit;
	}

	// Check destination ID value against tt, and that the multicast mask
	// does not select ports which do not exist on the Tsi57x device.
	if ((in_parms->mc_info.mc_destID > RIO_LAST_DEV16)
			|| ((in_parms->mc_info.mc_destID > RIO_LAST_DEV8)
					&& (tt_dev8 == in_parms->mc_info.tt))
			|| (in_parms->mc_info.mc_mask & illegal_ports)) {
		out_parms->imp_rc = CHANGE_MC_MASK(2);
		goto exit;
	}

	// Check that the destination ID is not duplicated elsewhere in the
	// multicast table.

	chg_idx = RIO_RTV_GET_MC_MSK(in_parms->mc_mask_rte);

	for (mc_idx = 0; mc_idx < TSI578_MAX_MC_MASKS; mc_idx++) {
		if ((mc_idx != chg_idx)
				&& (in_parms->rt->mc_masks[mc_idx].mc_destID
						== in_parms->mc_info.mc_destID)
				&& (in_parms->rt->mc_masks[mc_idx].tt
						== in_parms->mc_info.tt)) {
			rc = RIO_ERR_ROUTE_ERROR;
			out_parms->imp_rc = CHANGE_MC_MASK(3);
			goto exit;
		}
	}

	rc = RIO_SUCCESS;

	// Allow requests to change masks not supported by TSI57x family
	// but there's nothing to do...
	if (RIO_RTV_GET_MC_MSK(in_parms->mc_mask_rte) >= TSI578_MAX_MC_MASKS) {
		goto exit;
	}

	// If entry has not already been changed, see if it is being changed
	//@sonar:off - Collapsible "if" statements should be merged
	if (!in_parms->rt->mc_masks[chg_idx].changed) {
		if ((in_parms->rt->mc_masks[chg_idx].mc_destID
				!= in_parms->mc_info.mc_destID)
				|| (in_parms->rt->mc_masks[chg_idx].tt
						!= in_parms->mc_info.tt)
				|| (in_parms->rt->mc_masks[chg_idx].mc_mask
						!= in_parms->mc_info.mc_mask)
				|| (in_parms->rt->mc_masks[chg_idx].in_use
						!= in_parms->mc_info.in_use)) {
			in_parms->rt->mc_masks[chg_idx].changed = true;
		}
	}
	//@sonar:on

	in_parms->rt->mc_masks[chg_idx].in_use = in_parms->mc_info.in_use;
	in_parms->rt->mc_masks[chg_idx].mc_destID = in_parms->mc_info.mc_destID;
	in_parms->rt->mc_masks[chg_idx].tt = in_parms->mc_info.tt;
	in_parms->rt->mc_masks[chg_idx].mc_mask = in_parms->mc_info.mc_mask;

exit:
	return rc;
}

#endif /* TSI57X_DAR_WANTED */

#ifdef __cplusplus
}
#endif
