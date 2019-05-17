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

#include "DAR_DB_Private.h"
#include "DSF_DB_Private.h"
#include "RapidIO_Routing_Table_API.h"
#include "CPS_DeviceDriver.h"
#include "CPS1848.h"
#include "CPS1616.h"
#include "rio_standard.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CPS_DAR_WANTED

#define NUM_CPS_MC_MASKS(x) ((NUM_MC_MASKS(x) > CPS_MAX_MC_MASK)? \
			CPS_MAX_MC_MASK : NUM_MC_MASKS(x))

#define CPS_FIRST_MC_MASK                       (0x00000040)
#define CPS_LAST_MC_MASK                        (0x00000067)

#define CPS_RTE_PT_0                            (0x00000000)
#define CPS_RTE_PT_LAST                         (0x00000012)

#define RTE_SET_COMMON_0      (RT_FIRST_SUBROUTINE_0+0x0100) // 0x100100
#define PROGRAM_RTE_ENTRIES_0 (RT_FIRST_SUBROUTINE_0+0x1900)
#define PROGRAM_MC_MASKS_0    (RT_FIRST_SUBROUTINE_0+0x1A00)
#define READ_MC_MASKS_0       (RT_FIRST_SUBROUTINE_0+0x1B00)
#define READ_RTE_ENTRIES_0    (RT_FIRST_SUBROUTINE_0+0x1C00)

#define READ_MC_MASKS(x) (READ_MC_MASKS_0+x)
#define READ_RTE_ENTRIES(x) (READ_RTE_ENTRIES_0+x)

#define MC_MASK_ADDR(b,m) (b+(4*m))
#define PROGRAM_MC_MASKS(x) (PROGRAM_MC_MASKS_0+x)

#define SET_ALL     true
#define SET_CHANGED false

#define DEV_RTE_ADDR(b,n) (b+(4*n))
#define DOM_RTE_ADDR(b,n) (b+(4*n))
#define PROGRAM_RTE_ENTRIES(x) (PROGRAM_RTE_ENTRIES_0+x)

#define RTE_SET_COMMON(x) (RTE_SET_COMMON_0+x)

// Used by CPS_test.c
uint32_t cps_rte_translate_std_to_CPS(DAR_DEV_INFO_t *dev_info,
				uint32_t std_in, uint32_t *cps_out) 
{
	switch(std_in) {
	case RIO_RTE_DFLT_PORT: *cps_out = CPS_RT_USE_DEFAULT_ROUTE;
				goto success;
 	case RIO_RTE_DROP: *cps_out = CPS_RT_NO_ROUTE;
				goto success;
	case RIO_RTE_LVL_G0: *cps_out = CPS_RT_USE_DEVICE_TABLE;
				goto success;
	default:
		// CPS does not support implementation specific bits
		if (std_in & ~RIO_RTE_VAL) {
			break;
		}

		if (RIO_RTV_IS_PORT(std_in) &&
				(RIO_RTV_GET_PORT(std_in) < NUM_PORTS(dev_info))) {
			*cps_out = std_in;
			goto success;
		}

		if (RIO_RTV_IS_MC_MSK(std_in) &&
				(RIO_RTV_GET_MC_MSK(std_in) < CPS_MAX_MC_MASK)) {
			*cps_out = CPS_MC_PORT(RIO_RTV_GET_MC_MSK(std_in));
			goto success;
		}
		break;
	}
	return RIO_ERR_INVALID_PARAMETER;

success:
	return RIO_SUCCESS;
}

// Used by CPS_test.c
uint32_t cps_rte_translate_CPS_to_std(DAR_DEV_INFO_t *dev_info,
				uint32_t cps_in, uint32_t *std_out)
{
	switch(cps_in) {
	case CPS_RT_USE_DEFAULT_ROUTE: *std_out = RIO_RTE_DFLT_PORT;
				goto success;
 	case CPS_RT_NO_ROUTE: *std_out = RIO_RTE_DROP;
				goto success;
	case CPS_RT_USE_DEVICE_TABLE: *std_out = RIO_RTE_LVL_G0;
				goto success;
	default:
		if (cps_in < NUM_PORTS(dev_info)) {
			*std_out = RIO_RTV_PORT(cps_in);
			goto success;
		}
		if (IS_CPS_MC_PORT(cps_in)) {
			*std_out = RIO_RTV_MC_MSK(IS_CPS_MC_MASK_NO(cps_in));
			goto success;
		}
		break;
	}
	return RIO_ERR_INVALID_PARAMETER;

success:
	return RIO_SUCCESS;
}

/* initializes the routing table hardware and/or routing table state structure.
*/
static uint32_t cps_check_port_for_discard(DAR_DEV_INFO_t *dev_info,
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

	if (NUM_CPS_PORTS(dev_info) <= port) {
		out_parms->reason_for_discard = rio_rt_disc_probe_abort;
		out_parms->imp_rc = RT_PROBE(1);
		goto exit;
	}

	cfg_in.ptl.num_ports = 1;
	cfg_in.ptl.pnums[0] = port;
	rc = CPS_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->reason_for_discard = rio_rt_disc_probe_abort;
		out_parms->imp_rc = RT_PROBE(2);
		goto exit;
	}

	stat_in.ptl.num_ports = 1;
	stat_in.ptl.pnums[0] = port;
	rc = CPS_rio_pc_get_status(dev_info, &stat_in, &stat_out);
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
										CPS1848_PORT_X_CTL_1_CSR(
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

static uint32_t cps_read_mc_masks(DAR_DEV_INFO_t *dev_info, uint8_t pnum,
		rio_rt_state_t *rt, uint32_t *imp_rc)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t mask_idx;
	uint32_t reg_val, port_mask = ((uint32_t)(1) << NUM_CPS_PORTS(dev_info))
			- 1;
	rio_rt_dealloc_mc_mask_in_t d_in_parm;
	rio_rt_dealloc_mc_mask_out_t d_out_parm;

	d_in_parm.rt = rt;
	for (mask_idx = NUM_CPS_MC_MASKS(dev_info);
			mask_idx < RIO_MAX_MC_MASKS; mask_idx++) {
		d_in_parm.mc_mask_rte = RIO_RTV_MC_MSK(mask_idx);
		rc = DSF_rio_rt_dealloc_mc_mask(dev_info, &d_in_parm,
				&d_out_parm);
		if (RIO_SUCCESS != rc) {
			*imp_rc = d_out_parm.imp_rc;
			goto exit;
		}
	}

	for (mask_idx = 0; mask_idx < NUM_CPS_MC_MASKS(dev_info); mask_idx++) {
		rc = DARRegRead(dev_info,
				CPS1848_PORT_X_MCAST_MASK_Y(pnum, mask_idx),
				&reg_val);
		if (RIO_SUCCESS != rc) {
			*imp_rc = READ_MC_MASKS(0x01);
			goto exit;
		}

		rt->mc_masks[mask_idx].allocd = false;
		rt->mc_masks[mask_idx].changed = false;
		rt->mc_masks[mask_idx].tt = tt_dev8;
		rt->mc_masks[mask_idx].in_use = false;
		rt->mc_masks[mask_idx].mc_destID = 0x0;
		rt->mc_masks[mask_idx].mc_mask = reg_val & port_mask;
	}

exit:
	return rc;
}

static uint32_t cps_read_rte_entries(DAR_DEV_INFO_t *dev_info, uint8_t pnum,
		rio_rt_state_t *rt, uint32_t *imp_rc)
{
	uint32_t rc;
	uint32_t destID, rte_val, first_mc_destID;
	bool found_one = false;

	// Fill in default route value

	rc = DARRegRead(dev_info, CPS1848_RTE_DEFAULT_PORT_CSR, &rte_val);
	if (RIO_SUCCESS != rc) {
		*imp_rc = READ_RTE_ENTRIES(0x01);
		goto exit;
	}

	rt->default_route = (uint8_t)(rte_val
			& CPS1848_RTE_DEFAULT_PORT_CSR_DEFAULT_PORT);
	if (rt->default_route >= NUM_CPS_PORTS(dev_info)) {
		rt->default_route = RIO_RTE_DROP;
	}

	// Read all of the domain routing table entries.
	//
	// CPS Programming model assumes that device IDs of the form 0x00yy are
	// routed using the device routing table.  This is accomplished by ensuring
	// that the RIO_DOMAIN register is always 0.  Note that RIO_DOMAIN is a
	// global register, affecting routing on all ports.

	rt->dom_table[0].rte_val = RIO_RTE_LVL_G0;
	rt->dom_table[0].changed = false;
	first_mc_destID = 0;

	for (destID = 1; destID < RIO_RT_GRP_SZ; destID++) {
		rt->dom_table[destID].changed = false;

		// Read routing table entry for deviceID
		rc = DARRegRead(dev_info,
				CPS1848_PORT_X_DOM_RTE_TABLE_Y(pnum, destID),
				&rte_val);
		if (RIO_SUCCESS != rc) {
			*imp_rc = READ_RTE_ENTRIES(0x04);
			goto exit;
		}

		rte_val &= CPS1848_PORT_X_DOM_RTE_TABLE_Y_PORT;
		rc = cps_rte_translate_CPS_to_std(dev_info, rte_val, &rte_val);
		if (RIO_SUCCESS != rc) {
			*imp_rc = READ_RTE_ENTRIES(0x05);
			goto exit;
		}
		rt->dom_table[destID].rte_val = rte_val;

		if (RIO_RTE_LVL_G0 == rte_val) {
			if (!found_one) {
				first_mc_destID = destID << 8;
				found_one = true;
			}
		} else {
			if (((RIO_RTE_DFLT_PORT) != rte_val)
					&& (RIO_RTE_DROP != rte_val)
					&& (NUM_CPS_PORTS(dev_info) <= rte_val))
			{
				rt->dom_table[destID].rte_val = RIO_RTE_DROP;
			}
		}
	}

	// Read all of the device routing table entries.
	// Update multicast entries as we go...
	//
	for (destID = 0; destID < RIO_RT_GRP_SZ; destID++) {
		uint32_t mask_idx;

		rt->dev_table[destID].changed = false;

		rc = DARRegRead(dev_info,
				CPS1848_PORT_X_DEV_RTE_TABLE_Y(pnum, destID),
				&rte_val);
		if (RIO_SUCCESS != rc) {
			*imp_rc = READ_RTE_ENTRIES(8);
			goto exit;
		}

		rte_val &= CPS1848_PORT_X_DEV_RTE_TABLE_Y_PORT;
		rc = cps_rte_translate_CPS_to_std(dev_info, rte_val, &rte_val);
		if (RIO_SUCCESS != rc) {
			*imp_rc = READ_RTE_ENTRIES(0x09);
			goto exit;
		}
		rt->dev_table[destID].rte_val = (uint32_t)(rte_val);

		mask_idx = RIO_RTV_GET_MC_MSK(rte_val);
		if ((RIO_RTE_BAD != mask_idx)
				&& !(rt->mc_masks[mask_idx].in_use)) {
			rt->mc_masks[mask_idx].tt = tt_dev16;
			rt->mc_masks[mask_idx].in_use = true;
			rt->mc_masks[mask_idx].mc_destID =
					(did_reg_t)(first_mc_destID + destID);
		}

		if ((rte_val >= NUM_CPS_PORTS(dev_info))
				&& (!RIO_RTV_IS_MC_MSK(rte_val))
				&& (RIO_RTE_DROP != rte_val)
				&& (RIO_RTE_DFLT_PORT != rte_val)) {
			rt->dev_table[destID].rte_val = RIO_RTE_DROP;
		}
	}

exit:
	return rc;
}

static uint32_t cps_program_mc_mask(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_all_in_t *in_parms,
		bool set_all, // true if all entries should be set
		uint32_t *imp_rc)
{
	uint32_t rc = RIO_SUCCESS;
	// Note that the base address for CPS1848, CPS1432, CPS1616, SPS1616
	// are all the same.
	uint32_t mask_num;
	uint32_t base_addr, mask_mask;

	if (RIO_DEVI_IDT_CPS1848 == DEV_CODE(dev_info)) {
		mask_mask = CPS1848_BCAST__MCAST_MASK_X_PORT_MASK;
	} else {
		mask_mask = CPS1616_BCAST__MCAST_MASK_X_PORT_MASK;
	}

	if (RIO_ALL_PORTS == in_parms->set_on_port) {
		base_addr = CPS1848_BCAST_MCAST_MASK_X(0);
	} else {
		base_addr = CPS1848_PORT_X_MCAST_MASK_Y(in_parms->set_on_port,
				0);
	}

	for (mask_num = 0; mask_num < NUM_CPS_MC_MASKS(dev_info); mask_num++) {
		if (in_parms->rt->mc_masks[mask_num].changed || set_all) {
			if (in_parms->rt->mc_masks[mask_num].mc_mask
					& ~mask_mask) {
				rc = RIO_ERR_INVALID_PARAMETER;
				*imp_rc = PROGRAM_MC_MASKS(3);
				goto exit;
			}
			rc = DARRegWrite(dev_info,
					MC_MASK_ADDR(base_addr, mask_num),
					in_parms->rt->mc_masks[mask_num].mc_mask
							& mask_mask);
			if (RIO_SUCCESS != rc) {
				*imp_rc = PROGRAM_MC_MASKS(4);
				goto exit;
			}
			in_parms->rt->mc_masks[mask_num].changed = false;
		}
	}

exit:
	return rc;
}

static uint32_t cps_program_rte_entries(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_all_in_t *in_parms,
		bool set_all, // true if all entries should be set
		uint32_t *imp_rc)
{
	uint32_t rc = RIO_SUCCESS;
	// Note that the base address for CPS1848, CPS1432, CPS1616, SPS1616
	// are all the same.
	uint16_t rte_num;
	uint32_t dev_rte_base, dom_rte_base, cps_val;

	rc = cps_rte_translate_std_to_CPS(dev_info, in_parms->rt->default_route,
			&cps_val);
	if (RIO_SUCCESS != rc) {
		*imp_rc = PROGRAM_RTE_ENTRIES(0x06);
		goto exit;
	}
	rc = DARRegWrite(dev_info, CPS1848_RTE_DEFAULT_PORT_CSR, cps_val);
	if (RIO_SUCCESS != rc) {
		*imp_rc = PROGRAM_RTE_ENTRIES(0x10);
		goto exit;
	}

	if (RIO_ALL_PORTS == in_parms->set_on_port) {
		dev_rte_base = CPS1848_BCAST_DEV_RTE_TABLE_X(0);
		dom_rte_base = CPS1848_BCAST_DOM_RTE_TABLE_X(0);
	} else {
		dev_rte_base = CPS1848_PORT_X_DEV_RTE_TABLE_Y(
				in_parms->set_on_port, 0);
		dom_rte_base = CPS1848_PORT_X_DOM_RTE_TABLE_Y(
				in_parms->set_on_port, 0);
	}

	// DOMAIN REGISTER MUST ALWAYS BE 0
	// THIS MAKES 16 BIT DESTIDS OF THE FORM 0x00YY
	// EQUIVALENT TO 8 BIT DESTID ROUTING

	rc = DARRegWrite(dev_info, CPS1848_RIO_DOMAIN, 0);
	if (RIO_SUCCESS != rc) {
		*imp_rc = PROGRAM_RTE_ENTRIES(0);
		goto exit;
	}

	for (rte_num = 0; rte_num < RIO_RT_GRP_SZ; rte_num++) {
		if (in_parms->rt->dom_table[rte_num].changed || set_all) {
			//@sonar:off - Collapsible "if" statements should be merged
			// Validate value to be programmed.
			if (in_parms->rt->dom_table[rte_num].rte_val
					>= NUM_CPS_PORTS(dev_info)) {
				// Domain table can be a port number, use device table, use default route, or drop.
				if ((in_parms->rt->dom_table[rte_num].rte_val
						!= RIO_RTE_LVL_G0)
						&& (in_parms->rt->dom_table[rte_num].rte_val
								!= RIO_RTE_DFLT_PORT)
						&& (in_parms->rt->dom_table[rte_num].rte_val
								!= RIO_RTE_DROP)) {
					rc = RIO_ERR_INVALID_PARAMETER;
					*imp_rc = PROGRAM_RTE_ENTRIES(1);
					goto exit;
				}
			}
			//@sonar:on
			rc =
					cps_rte_translate_std_to_CPS(dev_info,
							in_parms->rt->dom_table[rte_num].rte_val,
							&cps_val);
			if (RIO_SUCCESS != rc) {
				*imp_rc = PROGRAM_RTE_ENTRIES(0x07);
				goto exit;
			}

			rc = DARRegWrite(dev_info,
					DOM_RTE_ADDR(dom_rte_base, rte_num),
					cps_val);
			if (RIO_SUCCESS != rc) {
				*imp_rc = PROGRAM_RTE_ENTRIES(2);
				goto exit;
			}
			in_parms->rt->dom_table[rte_num].changed = false;
		}
	}

	for (rte_num = 0; rte_num < RIO_RT_GRP_SZ; rte_num++) {
		if (in_parms->rt->dev_table[rte_num].changed || set_all) {
			//@sonar:off - Collapsible "if" statements should be merged
			// Validate value to be programmed.
			if (in_parms->rt->dev_table[rte_num].rte_val
					>= NUM_CPS_PORTS(dev_info)) {
				// Device table can be a port number, a multicast mask, use default route, or drop.
				if ((RIO_RTV_GET_MC_MSK(
						in_parms->rt->dev_table[rte_num].rte_val)
						== RIO_RTE_BAD)
						&& (in_parms->rt->dev_table[rte_num].rte_val
								!= RIO_RTE_DFLT_PORT)
						&& (in_parms->rt->dev_table[rte_num].rte_val
								!= RIO_RTE_DROP)) {
					rc = RIO_ERR_INVALID_PARAMETER;
					*imp_rc = PROGRAM_RTE_ENTRIES(3);
					goto exit;
				}
			}
			//@sonar:on
			rc =
					cps_rte_translate_std_to_CPS(dev_info,
							in_parms->rt->dev_table[rte_num].rte_val,
							&cps_val);
			if (RIO_SUCCESS != rc) {
				*imp_rc = PROGRAM_RTE_ENTRIES(0x08);
				goto exit;
			}

			rc = DARRegWrite(dev_info,
					DEV_RTE_ADDR(dev_rte_base, rte_num),
					cps_val);
			if (RIO_SUCCESS != rc) {
				*imp_rc = PROGRAM_RTE_ENTRIES(4);
				goto exit;
			}
			in_parms->rt->dev_table[rte_num].changed = false;
		}
	}

exit:
	return rc;
}

static uint32_t cps_rt_set_common(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_all_in_t *in_parms, rio_rt_set_all_out_t *out_parms,
		bool set_all) // true if all entries should be set
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;

	out_parms->imp_rc = RIO_SUCCESS;

	if ((((uint8_t)(RIO_ALL_PORTS) != in_parms->set_on_port)
			&& (in_parms->set_on_port >= NUM_CPS_PORTS(dev_info)))
			|| (!in_parms->rt)) {
		out_parms->imp_rc = RTE_SET_COMMON(1);
		goto exit;
	}

	if ((NUM_CPS_PORTS(dev_info) <= in_parms->rt->default_route)
			&& !(RIO_RTE_DROP == in_parms->rt->default_route)) {
		out_parms->imp_rc = RTE_SET_COMMON(2);
		goto exit;
	}

	out_parms->imp_rc = RIO_SUCCESS;
	rc = cps_program_mc_mask(dev_info, in_parms, set_all,
			&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	rc = cps_program_rte_entries(dev_info, in_parms, set_all,
			&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

exit:
	return rc;
}

// Make sure that we're not orphaning a multicast mask...

static uint32_t cps_tidy_routing_table(DAR_DEV_INFO_t *dev_info, uint8_t dev_idx,
		rio_rt_state_t *rt, uint32_t *fail_pt)
{
	uint32_t rc = RIO_SUCCESS;
	uint16_t srch_idx;
	bool found_one = false;

	if (RIO_RTV_IS_MC_MSK(rt->dev_table[dev_idx].rte_val)) {
		for (srch_idx = 0; (srch_idx < RIO_RT_GRP_SZ) && !found_one;
								srch_idx++) {
			if (dev_idx == srch_idx)
				continue;
			if (rt->dev_table[dev_idx].rte_val
					== rt->dev_table[srch_idx].rte_val)
				found_one = true;
		}

		if (!found_one) {
			rio_rt_dealloc_mc_mask_in_t in_parms;
			rio_rt_dealloc_mc_mask_out_t out_parms;
			in_parms.rt = rt;
			in_parms.mc_mask_rte = rt->dev_table[dev_idx].rte_val;
			rc = DSF_rio_rt_dealloc_mc_mask(dev_info, &in_parms,
					&out_parms);
			if (RIO_SUCCESS != rc) {
				*fail_pt = out_parms.imp_rc;
			}
		}
	}
	return rc;
}

// Routing Table
//
uint32_t CPS_rio_rt_initialize(DAR_DEV_INFO_t *dev_info,
		rio_rt_initialize_in_t *in_parms,
		rio_rt_initialize_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t destID;
	uint32_t mc_idx;
	rio_rt_set_changed_in_t all_in;
	rio_rt_set_changed_out_t all_out;
	rio_rt_state_t rt_state;

	// Validate parameters

	if ((in_parms->default_route >= NUM_CPS_PORTS(dev_info))
			&& !(RIO_RTE_DROP == in_parms->default_route)) {
		out_parms->imp_rc = RT_INITIALIZE(1);
		goto exit;
	}

	if ((in_parms->default_route_table_port >= NUM_CPS_PORTS(dev_info))
			&& !((RIO_RTE_DFLT_PORT
					== in_parms->default_route_table_port)
					|| (RIO_RTE_DROP
							== in_parms->default_route_table_port))) {
		out_parms->imp_rc = RT_INITIALIZE(2);
		goto exit;
	}

	if ((in_parms->set_on_port >= NUM_CPS_PORTS(dev_info))
			&& !(RIO_ALL_PORTS == in_parms->set_on_port)) {
		out_parms->imp_rc = RT_INITIALIZE(3);
		goto exit;
	}

	out_parms->imp_rc = RIO_SUCCESS;
	all_in.set_on_port = in_parms->set_on_port;

	if (!in_parms->rt)
		all_in.rt = &rt_state;
	else
		all_in.rt = in_parms->rt;

	all_in.rt->default_route = in_parms->default_route;

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
		if ((mc_idx < CPS_MAX_MC_MASK)
				&& (mc_idx < RIO_MAX_MC_MASKS)) {
			all_in.rt->mc_masks[mc_idx].changed = true;
		} else {
			all_in.rt->mc_masks[mc_idx].changed = false;
		}
	}

	if (in_parms->update_hw) {
		uint8_t port, start_port, end_port;
		uint32_t ops;

		if (RIO_ALL_PORTS == in_parms->set_on_port) {
			start_port = 0;
			end_port = NUM_CPS_PORTS(dev_info) - 1;
		} else {
			start_port = end_port = in_parms->set_on_port;
		}

		/* Clear self-association MC bit */
		for (port = start_port; port <= end_port; port++) {
			rc = DARRegRead(dev_info, CPS1848_PORT_X_OPS(port),
					&ops);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = RT_INITIALIZE(4);
				goto exit;
			}

			/* Self association bit off */
			ops &= ~CPS1848_PORT_X_OPS_SELF_MCAST_EN;

			rc = DARRegWrite(dev_info, CPS1848_PORT_X_OPS(port),
					ops);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = RT_INITIALIZE(5);
				goto exit;
			}
		}
		rc = CPS_rio_rt_set_changed(dev_info, &all_in, &all_out);
	} else {
		rc = RIO_SUCCESS;
	}

	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = all_out.imp_rc;
	}

exit:
	return rc;
}

/* This function probes the hardware status of a routing table entry for
 *   the specified port and destination ID
 */
uint32_t CPS_rio_rt_probe(DAR_DEV_INFO_t *dev_info, rio_rt_probe_in_t *in_parms,
		rio_rt_probe_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t bit;
	uint32_t regVal;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->valid_route = false;
	out_parms->routing_table_value = RIO_ALL_PORTS;
	for (bit = 0; bit < NUM_CPS_PORTS(dev_info); bit++)
		out_parms->mcast_ports[bit] = false;
	out_parms->reason_for_discard = rio_rt_disc_probe_abort;

	if (((NUM_CPS_PORTS(dev_info) <= in_parms->probe_on_port)
			&& (RIO_ALL_PORTS != in_parms->probe_on_port))
			|| (!in_parms->rt)) {
		out_parms->imp_rc = RT_PROBE(0x11);
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PKT_TTL_CSR, &regVal);
	if ( RIO_SUCCESS != rc) {
		out_parms->imp_rc = RT_PROBE(0x12);
		goto exit;
	}
	out_parms->time_to_live_active =
			(regVal & CPS1848_PKT_TTL_CSR_TTL) ? true : false;

	rc = DARRegRead(dev_info, CPS1848_DEVICE_CTL_1, &regVal);
	if ( RIO_SUCCESS != rc) {
		out_parms->imp_rc = RT_PROBE(0x13);
		goto exit;
	}
	out_parms->trace_function_active =
			(regVal & CPS1848_DEVICE_CTL_1_TRACE_EN) ? true : false;
	out_parms->filter_function_active = false;

	if (RIO_ALL_PORTS != in_parms->probe_on_port) {
		rc = DARRegRead(dev_info,
				CPS1848_PORT_X_OPS(in_parms->probe_on_port),
				&regVal);
		if ( RIO_SUCCESS != rc) {
			out_parms->imp_rc = RT_PROBE(0x14);
			goto exit;
		}

		out_parms->trace_function_active =
				(regVal & CPS1848_PORT_X_OPS_ANY_TRACE) ?
						true : false;
		out_parms->filter_function_active =
				(regVal & CPS1848_PORT_X_OPS_ANY_FILTER) ?
						true : false;
	}

	rc = RIO_SUCCESS;

	// Note, no failure possible...
	rio_rt_check_multicast_routing(dev_info, in_parms, out_parms);

	/* Done if hit in multicast masks. */
	if (RIO_ALL_PORTS != out_parms->routing_table_value)
		goto exit;

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
		rc = cps_check_port_for_discard(dev_info, in_parms,
				out_parms);
	}

exit:
	return rc;
}

/* This function returns the complete hardware state of packet routing
 * in a routing table state structure.
 *
 * The routing table hardware must be initialized using rio_rt_initialize()
 *    before calling this routine.
 */
uint32_t CPS_rio_rt_probe_all(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_all_in_t *in_parms,
		rio_rt_probe_all_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t probe_port;

	out_parms->imp_rc = RIO_SUCCESS;
	if ((((uint8_t)(RIO_ALL_PORTS) != in_parms->probe_on_port)
			&& (in_parms->probe_on_port >= NUM_CPS_PORTS(dev_info)))
			|| (!in_parms->rt)) {
		out_parms->imp_rc = RT_PROBE_ALL(1);
		goto exit;
	}

	probe_port = (RIO_ALL_PORTS == in_parms->probe_on_port) ?
			0 : in_parms->probe_on_port;

	rc = cps_read_mc_masks(dev_info, probe_port, in_parms->rt,
			&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	rc = cps_read_rte_entries(dev_info, probe_port, in_parms->rt,
			&out_parms->imp_rc);

exit:
	return rc;
}

/* This function sets the routing table hardware to match every entry
 *    in the routing table state structure. 
 * After rio_rt_set_all is called, no entries are marked as changed in
 *    the routing table state structure.
 */
uint32_t CPS_rio_rt_set_all(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_all_in_t *in_parms, rio_rt_set_all_out_t *out_parms)
{
	return cps_rt_set_common(dev_info, in_parms, out_parms, SET_ALL);
}

/* This function sets the the routing table hardware to match every entry
 *    that has been changed in the routing table state structure. 
 * Changes must be made using rio_rt_alloc_mc_mask, rio_rt_deallocate_mc_mask,
 *    rio_rt_change_rte, and rio_rt_change_mc.
 * After rio_rt_set_changed is called, no entries are marked as changed in
 *    the routing table state structure.
 */
uint32_t CPS_rio_rt_set_changed(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_changed_in_t *in_parms,
		rio_rt_set_changed_out_t *out_parms)
{
	return cps_rt_set_common(dev_info, in_parms, out_parms, SET_CHANGED);
}

/* This function updates an rio_rt_state_t structure to
 * change a routing table entry, and tracks changes.
 */
uint32_t CPS_rio_rt_change_rte(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_rte_in_t *in_parms,
		rio_rt_change_rte_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;

	out_parms->imp_rc = RIO_SUCCESS;

	if (!in_parms->rt) {
		out_parms->imp_rc = RT_CHANGE_RTE(1);
		goto exit;
	}

	// Validate rte_value
	if ((RIO_RTE_LVL_G0 != in_parms->rte_value)
			&& (RIO_RTE_DFLT_PORT != in_parms->rte_value)
			&& (RIO_RTE_DROP != in_parms->rte_value)
			&& (in_parms->rte_value >= NUM_CPS_PORTS(dev_info))) {
		out_parms->imp_rc = RT_CHANGE_RTE(2);
		goto exit;
	}

	if ((RIO_RTE_LVL_G0 == in_parms->rte_value)
			&& (!in_parms->dom_entry)) {
		out_parms->imp_rc = RT_CHANGE_RTE(3);
		goto exit;
	}

	rc = RIO_SUCCESS;

	// Do not allow any changes to index 0 of the domain table.
	// This must be set to "RIO_RTE_LVL_G0" at all times,
	// as this is the behavior required by the CPS RIO Domain register.

	if (in_parms->dom_entry && !in_parms->idx)
		goto exit;

	//@sonar:off - Collapsible "if" statements should be merged
	// If the entry has not already been changed, see if it is being changed
	if (in_parms->dom_entry) {
		if (!in_parms->rt->dom_table[in_parms->idx].changed) {
			if (in_parms->rt->dom_table[in_parms->idx].rte_val
					!= in_parms->rte_value)
				in_parms->rt->dom_table[in_parms->idx].changed =
						true;
		}
		in_parms->rt->dom_table[in_parms->idx].rte_val =
				in_parms->rte_value;
	} else {
		if (!in_parms->rt->dev_table[in_parms->idx].changed) {
			if (in_parms->rt->dev_table[in_parms->idx].rte_val
					!= in_parms->rte_value)
				in_parms->rt->dev_table[in_parms->idx].changed =
						true;
		}
		in_parms->rt->dev_table[in_parms->idx].rte_val =
				in_parms->rte_value;
	}
	//@sonar:on

exit:
	return rc;
}

/* This function updates an rio_rt_state_t structure to
 * change a multicast mask value, and tracks changes.
 */
uint32_t CPS_rio_rt_change_mc_mask(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_mc_mask_in_t *in_parms,
		rio_rt_change_mc_mask_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t chg_idx, dom_idx, dev_idx;
	uint32_t illegal_ports = ~((1 << CPS_MAX_PORTS) - 1);
	uint32_t avail_ports = (1 << NUM_CPS_PORTS(dev_info)) - 1;

	out_parms->imp_rc = RIO_SUCCESS;

	if (!in_parms->rt) {
		out_parms->imp_rc = CHANGE_MC_MASK(1);
		goto exit;
	}

	// Check destination ID value against tt, and that the multicast mask
	// does not select ports which do not exist on the CPS device.
	if ((in_parms->mc_info.mc_destID > RIO_LAST_DEV16)
			|| ((in_parms->mc_info.mc_destID > RIO_LAST_DEV8)
					&& (tt_dev8 == in_parms->mc_info.tt))
			|| (in_parms->mc_info.mc_mask & illegal_ports)) {
		out_parms->imp_rc = CHANGE_MC_MASK(2);
		goto exit;
	}

	if (!in_parms->mc_info.in_use) {
		rio_rt_dealloc_mc_mask_in_t d_in_parm;
		rio_rt_dealloc_mc_mask_out_t d_out_parm;

		d_in_parm.mc_mask_rte = in_parms->mc_mask_rte;
		d_in_parm.rt = in_parms->rt;

		rc = DSF_rio_rt_dealloc_mc_mask(dev_info, &d_in_parm,
				&d_out_parm);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = d_out_parm.imp_rc;
		}
		goto exit;
	}

	// Allow requests to change masks not supported by CPS family
	// but there's nothing to do...

	chg_idx = RIO_RTV_GET_MC_MSK(in_parms->mc_mask_rte);

	if (chg_idx >= NUM_CPS_MC_MASKS(dev_info)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = CHANGE_MC_MASK(3);
		goto exit;
	}

	//@sonar:off - Collapsible "if" statements should be merged
	// If entry has not already been changed, see if it is being changed
	if (!in_parms->rt->mc_masks[chg_idx].changed) {
		if ((in_parms->rt->mc_masks[chg_idx].mc_mask
				!= in_parms->mc_info.mc_mask)
				|| (in_parms->rt->mc_masks[chg_idx].in_use
						!= in_parms->mc_info.in_use)) {
			in_parms->rt->mc_masks[chg_idx].changed = true;
		}
	}
	//@sonar:on

	// Note: The multicast mask must be in use now.  We must make sure that
	// the routing tables are set appropriately.
	dom_idx = (in_parms->mc_info.mc_destID & 0xFF00) >> 8;
	if ((tt_dev16 == in_parms->mc_info.tt) && (dom_idx)
			&& (RIO_RTE_LVL_G0
					!= in_parms->rt->dom_table[dom_idx].rte_val)) {
		in_parms->rt->dom_table[dom_idx].rte_val =
				RIO_RTE_LVL_G0;
		in_parms->rt->dom_table[dom_idx].changed = true;
	}

	dev_idx = (in_parms->mc_info.mc_destID & 0x00FF);
	if (in_parms->mc_mask_rte != in_parms->rt->dev_table[dev_idx].rte_val) {
		rc = cps_tidy_routing_table(dev_info, dev_idx, in_parms->rt,
				&out_parms->imp_rc);
		if (RIO_SUCCESS != rc)
			goto exit;

		in_parms->rt->dev_table[dev_idx].rte_val =
				in_parms->mc_mask_rte;
		in_parms->rt->dev_table[dev_idx].changed = true;
	}

	in_parms->rt->mc_masks[chg_idx].in_use = true;
	in_parms->rt->mc_masks[chg_idx].mc_destID = in_parms->mc_info.mc_destID;
	in_parms->rt->mc_masks[chg_idx].tt = in_parms->mc_info.tt;
	in_parms->rt->mc_masks[chg_idx].mc_mask = (in_parms->mc_info.mc_mask
			& avail_ports);

	rc = RIO_SUCCESS;

exit:
	return rc;
}

#endif /* CPS_DAR_WANTED */

#ifdef __cplusplus
}
#endif
