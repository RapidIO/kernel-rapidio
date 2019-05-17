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
#include <stdbool.h>

#include "RapidIO_Device_Access_Routines_API.h"
#include "RapidIO_Routing_Table_API.h"
#include "RXS_DeviceDriver.h"
#include "DSF_DB_Private.h"
#include "RXS2448.h"
#include "RXS_DeviceDriver.h"
#include "rio_standard.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RXS_DAR_WANTED

#define MC_MASK_ADDR(b,m) ((b)+(8*m))

#define DEV_RTE_ADDR(b,n) ((b)+(4*n))
#define DOM_RTE_ADDR(b,n) ((b)+(4*n))

#define RXS_RTE_SET_COMMON_0                  (RT_FIRST_SUBROUTINE_0+0x0100)
#define RXS_PROGRAM_RTE_ENTRIES_0             (RT_FIRST_SUBROUTINE_0+0x1900)
#define RXS_PROGRAM_MC_MASKS_0                (RT_FIRST_SUBROUTINE_0+0x1A00)
#define RXS_READ_MC_MASKS_0                   (RT_FIRST_SUBROUTINE_0+0x1B00)
#define RXS_READ_RTE_ENTRIES_0                (RT_FIRST_SUBROUTINE_0+0x1C00)

#define RXS_PROGRAM_MC_MASKS(x)               (RXS_PROGRAM_MC_MASKS_0+x)
#define RXS_PROGRAM_RTE_ENTRIES(x)            (RXS_PROGRAM_RTE_ENTRIES_0+x)
#define RXS_RTE_SET_COMMON(x)                 (RXS_RTE_SET_COMMON_0+x)
#define RXS_READ_MC_MASKS(x)                  (RXS_READ_MC_MASKS_0+x)
#define RXS_READ_RTE_ENTRIES(x)               (RXS_READ_RTE_ENTRIES_0+x)

void rxs_chk_and_corr_rtv(DAR_DEV_INFO_t *dev_info, rio_rt_uc_info_t *rtv,
					bool dom_value, bool dflt_port)
{
	uint32_t chk_val = rtv->rte_val & ~RIO_RTE_IMP_SPEC;

	// If any other bits are set outside the known bits,
	// fail...
	if (chk_val & ~RIO_RTE_VAL) {
		rtv->rte_val = RIO_RTE_DROP;
		rtv->changed = true;
	}

	// Check port value, update routing accordingly
	if (RIO_RTV_IS_PORT(chk_val)) {
		// Routing packets to an invalid port results in a drop
		if (RIO_RTV_GET_PORT(chk_val) >= NUM_PORTS(dev_info)) {
			rtv->rte_val = RIO_RTE_DROP;
			rtv->changed = true;
		}
		return;
	}

	// All multicast mask values are valid
	if (RIO_RTV_IS_MC_MSK(chk_val)) {
		return;
	}

	// Validate level group value
	if (RIO_RTV_IS_LVL_GRP(chk_val)) {
		if (!dom_value || dflt_port) {
			// Using a level group indicator when not allowed
			// results in a drop.
			rtv->rte_val = RIO_RTE_DROP;
			rtv->changed = true;
			return;
		}
		if (RIO_RTE_LVL_G0 != chk_val) {
			// Restrict level group usage to group 0
			rtv->rte_val = RIO_RTE_DROP;
			rtv->changed = true;
		}
		return;
	}

	// Validate default port.
	if (RIO_RTE_DFLT_PORT == chk_val) {
		if (dflt_port) {
			// Selecting default port for the default port
			// routing value results in a drop.
			rtv->rte_val = RIO_RTE_DROP;
			rtv->changed = true;
		}
		return;
	}

	// Dropping packets is always valid
	if (RIO_RTE_DROP == chk_val) {
		return;
	}

	// All other values result in packets being dropped.
	// Should never get here...
	rtv->rte_val = RIO_RTE_DROP;
	rtv->changed = true;
}

bool rxs_chk_dflt_rte_reg(DAR_DEV_INFO_t *dev_info, pe_rt_val val)
{
	rio_rt_uc_info_t rtv = {val, false};

	rxs_chk_and_corr_rtv(dev_info, &rtv, false, true);
	return rtv.changed;
}

bool rxs_chk_dom_rte_reg(DAR_DEV_INFO_t *dev_info, pe_rt_val val)
{
	rio_rt_uc_info_t rtv = {val, false};

	rxs_chk_and_corr_rtv(dev_info, &rtv, true, false);
	return rtv.changed;
}

bool rxs_chk_dev_rte_reg(DAR_DEV_INFO_t *dev_info, pe_rt_val val)
{
	rio_rt_uc_info_t rtv = {val, false};

	rxs_chk_and_corr_rtv(dev_info, &rtv, false, false);
	return rtv.changed;
}

uint32_t rxs_rio_rt_initialize(DAR_DEV_INFO_t *dev_info,
		rio_rt_initialize_in_t *in_parms,
		rio_rt_initialize_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t destID;
	rio_rt_set_changed_in_t all_in;
	rio_rt_set_changed_out_t all_out;
	rio_rt_state_t rt_state;
	uint32_t mc_idx;
	rio_rt_uc_info_t rtv;

	// Validate parameters

	if (rxs_chk_dflt_rte_reg(dev_info, in_parms->default_route)) {
		out_parms->imp_rc = RT_INITIALIZE(1);
		goto exit;
	}

	rtv.rte_val = in_parms->default_route_table_port;
	rtv.changed = false;
	rxs_chk_and_corr_rtv(dev_info, &rtv, false, false);
	if (rtv.changed) {
		out_parms->imp_rc = RT_INITIALIZE(1);
		goto exit;
	}

	if ((in_parms->set_on_port >= NUM_RXS_PORTS(dev_info))
			&& !(RIO_ALL_PORTS == in_parms->set_on_port)) {
		out_parms->imp_rc = RT_INITIALIZE(3);
		goto exit;
	}

	out_parms->imp_rc = RIO_SUCCESS;
	all_in.set_on_port = in_parms->set_on_port;

	if (!in_parms->rt) {
		all_in.rt = &rt_state;
	} else {
		all_in.rt = in_parms->rt;
	}

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

	for (mc_idx = 0; mc_idx < RIO_MAX_MC_MASKS; mc_idx++) {
		all_in.rt->mc_masks[mc_idx].mc_destID = 0;
		all_in.rt->mc_masks[mc_idx].tt = tt_dev8;
		all_in.rt->mc_masks[mc_idx].mc_mask = 0;
		all_in.rt->mc_masks[mc_idx].in_use = false;
		all_in.rt->mc_masks[mc_idx].allocd = false;
		all_in.rt->mc_masks[mc_idx].changed = true;
	}

	if (in_parms->update_hw) {
		rc = rxs_rio_rt_set_changed(dev_info, &all_in, &all_out);
	} else {
		rc = RIO_SUCCESS;
	}

	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = all_out.imp_rc;
	}

exit:
	return rc;
}

static uint32_t rxs_program_mc_masks(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_all_in_t *in_parms,
		bool set_all, // true if all entries should be set
		uint32_t *imp_rc)
{
	uint32_t rc = RIO_SUCCESS;
	// Note that the base address for RXS2448 and RXS1632
	// are all the same.
	uint32_t idx;
	uint32_t set_base_addr, clr_base_addr, mask_mask;
	rio_port_t port = in_parms->set_on_port;
	bool broadcast = false;

	switch (DEV_CODE(dev_info)) {
	case RIO_DEVI_IDT_RXS2448:
		mask_mask = RXS2448_RIO_BC_MC_X_S_CSR_SET;
		break;
	case RIO_DEVI_IDT_RXS1632:
		mask_mask = RXS1632_RIO_BC_MC_X_S_CSR_SET;
		break;
	default:
		rc = RIO_ERR_NO_FUNCTION_SUPPORT;
		*imp_rc = RXS_PROGRAM_MC_MASKS(0x01);
		goto exit;
	}

	if (RIO_ALL_PORTS == port) {
		// Must set all bits on broadcast, as it is not certain
		// what ports have what bits set or cleared.
		broadcast = true;
		set_base_addr = RXS_BC_MC_X_S_CSR(0);
		clr_base_addr = RXS_BC_MC_X_C_CSR(0);
	} else {
		set_base_addr = RXS_SPX_MC_Y_S_CSR(port, 0);
		clr_base_addr = RXS_SPX_MC_Y_C_CSR(port, 0);
	}

	for (idx = 0; idx < RXS2448_MC_MASK_CNT; idx++) {
		uint32_t mc_mask = in_parms->rt->mc_masks[idx].mc_mask;
		uint32_t curr_msk = 0;
		uint32_t chg_bits;

		// If the mask didn't change and we do not have to set all
		// masks, continue.
		if (!(in_parms->rt->mc_masks[idx].changed || set_all)) {
			continue;
		}

		if (mc_mask & ~mask_mask) {
			rc = RIO_ERR_INVALID_PARAMETER;
			*imp_rc = RXS_PROGRAM_MC_MASKS(5);
			goto exit;
		}

		if (!broadcast) {
			rc = DARRegRead(dev_info,
				MC_MASK_ADDR(set_base_addr, idx), &curr_msk);
			if (RIO_SUCCESS != rc) {
				*imp_rc = RXS_PROGRAM_MC_MASKS(0xf);
				goto exit;
			}
		}

		// If there are bits to set, set them.
		chg_bits = ~curr_msk & mc_mask & mask_mask;
		if (chg_bits) {
			rc = DARRegWrite(dev_info,
				MC_MASK_ADDR(set_base_addr, idx), chg_bits);
			if (RIO_SUCCESS != rc) {
				*imp_rc = RXS_PROGRAM_MC_MASKS(0x10);
				goto exit;
			}
		}
		if (broadcast) {
			curr_msk = mask_mask;
		}
		// If there are bits to clear, clear them
		chg_bits = curr_msk & ~mc_mask & mask_mask;
		if (chg_bits) {
			rc = DARRegWrite(dev_info,
				MC_MASK_ADDR(clr_base_addr, idx), chg_bits);
			if (RIO_SUCCESS != rc) {
				*imp_rc = RXS_PROGRAM_MC_MASKS(0x20);
				goto exit;
			}
		}
		in_parms->rt->mc_masks[idx].changed = false;
	}
exit:
	return rc;
}

static uint32_t rxs_program_rte_entries(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_all_in_t *in_parms,
		bool set_all, // true if all entries should be set
		uint32_t *imp_rc)
{
	uint32_t rc = RIO_SUCCESS;
	// Note that the base address for RXS2448 and RXS1632
	// are all the same.
	uint16_t rte_num;
	uint32_t dev_rte_base, dom_rte_base;

	rc = DARRegWrite(dev_info, RXS_ROUTE_DFLT_PORT,
			in_parms->rt->default_route);
	if (RIO_SUCCESS != rc) {
		*imp_rc = RXS_PROGRAM_RTE_ENTRIES(0x10);
		goto exit;
	}

	if (RIO_ALL_PORTS == in_parms->set_on_port) {
		dev_rte_base = RXS_BC_L2_GX_ENTRYY_CSR(0, 0);
		dom_rte_base = RXS_BC_L1_GX_ENTRYY_CSR(0, 0);
	} else {
		dev_rte_base = RXS_SPX_L2_GY_ENTRYZ_CSR(
				in_parms->set_on_port, 0, 0);
		dom_rte_base = RXS_SPX_L1_GY_ENTRYZ_CSR(
				in_parms->set_on_port, 0, 0);
	}

	for (rte_num = 0; rte_num < RIO_RT_GRP_SZ; rte_num++) {
		if (in_parms->rt->dom_table[rte_num].changed || set_all) {
			rc = DARRegWrite(dev_info,
				DOM_RTE_ADDR( dom_rte_base, rte_num),
				in_parms->rt->dom_table[rte_num].rte_val);
			if (RIO_SUCCESS != rc) {
				*imp_rc = RXS_PROGRAM_RTE_ENTRIES(2);
				goto exit;
			}
			in_parms->rt->dom_table[rte_num].changed = false;
		}
	}

	for (rte_num = 0; rte_num < RIO_RT_GRP_SZ; rte_num++) {
		if (in_parms->rt->dev_table[rte_num].changed || set_all) {
			// Validate value to be programmed.
			if (RIO_RTV_IS_LVL_GRP(
				in_parms->rt->dev_table[rte_num].rte_val)) {
				rc = RIO_ERR_INVALID_PARAMETER;
				*imp_rc = RXS_PROGRAM_RTE_ENTRIES(3);
				goto exit;
			}

			rc = DARRegWrite(dev_info,
				DEV_RTE_ADDR( dev_rte_base, rte_num),
				in_parms->rt->dev_table[rte_num].rte_val);
			if (RIO_SUCCESS != rc) {
				*imp_rc = RXS_PROGRAM_RTE_ENTRIES(4);
				goto exit;
			}
			in_parms->rt->dev_table[rte_num].changed = false;
		}
	}

exit:
	return rc;
}

#define RXS_SET_ALL     true
#define RXS_SET_CHANGED false

static uint32_t rxs_rt_set_common(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_all_in_t *in_parms, rio_rt_set_all_out_t *out_parms,
		bool set_all) // true if all entries should be set
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;

	out_parms->imp_rc = RIO_SUCCESS;

	if ((RIO_ALL_PORTS != in_parms->set_on_port)
			&& (in_parms->set_on_port >= NUM_RXS_PORTS(dev_info))) {
		out_parms->imp_rc = RXS_RTE_SET_COMMON(1);
		goto exit;
	}

	if (NULL == in_parms->rt) {
		out_parms->imp_rc = RXS_RTE_SET_COMMON(2);
		goto exit;
	}

	if (rxs_chk_dflt_rte_reg(dev_info, in_parms->rt->default_route)) {
		out_parms->imp_rc = RXS_RTE_SET_COMMON(3);
		goto exit;
	}

	out_parms->imp_rc = RIO_SUCCESS;
	rc = rxs_program_mc_masks(dev_info, in_parms, set_all,
			&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	rc = rxs_program_rte_entries(dev_info, in_parms, set_all,
			&out_parms->imp_rc);
exit:
	return rc;
}

uint32_t rxs_rio_rt_set_all(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_all_in_t *in_parms, rio_rt_set_all_out_t *out_parms)
{
	return rxs_rt_set_common(dev_info, in_parms, out_parms, RXS_SET_ALL);
}

uint32_t rxs_rio_rt_set_changed(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_changed_in_t *in_parms,
		rio_rt_set_changed_out_t *out_parms)
{
	return rxs_rt_set_common(dev_info, in_parms, out_parms, RXS_SET_CHANGED);
}

static void rxs_check_multicast_routing(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_in_t *in_parms, rio_rt_probe_out_t *out_parms,
		pe_rt_val rte)
{
	uint32_t mc_idx;
	uint32_t bit, set_cnt = 0;
	uint32_t legal_ports = (1 << NUM_RXS_PORTS(dev_info)) - 1;
	rio_rt_mc_info_t *mc;

	mc_idx = RIO_RTV_GET_MC_MSK(rte);
	if (mc_idx > RXS2448_MAX_MC_MASK) {
		out_parms->valid_route = false;
		out_parms->reason_for_discard = rio_rt_disc_mc_mult_masks;
		return;
	}
	mc = &in_parms->rt->mc_masks[mc_idx];

	// If mask is invalid, we're done...
	if (!mc->in_use) {
		out_parms->valid_route = false;
		out_parms->reason_for_discard = rio_rt_disc_mc_empty;
		return;
	}

	// If there aren't any bits set in the mask, we're done...
	if (!(mc->mc_mask & legal_ports)) {
		out_parms->valid_route = false;
		out_parms->reason_for_discard = rio_rt_disc_mc_empty;
		return;
	}

	for (bit = 0; bit < NUM_RXS_PORTS(dev_info); bit++) {
		// Never set the bit associated with the probed port...
		if (bit == in_parms->probe_on_port) {
			continue;
		}
		if ((1 << bit) & (mc->mc_mask)) {
			set_cnt++;
		}
		out_parms->mcast_ports[bit] = (1 << bit) & (mc->mc_mask);
	}

	// Mask was not zero, but no bits are set.  This indicates the
	// was only one bit set, for the probed port.  Since a packet cannot
	// be multicast out the port it was received on, the mask is
	// functionally empty.
	if (!set_cnt) {
		out_parms->valid_route = false;
		out_parms->reason_for_discard = rio_rt_disc_mc_one_bit;
	}
}

static void rxs_check_routing(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_in_t *in_parms,
		rio_rt_probe_out_t *out_parms)
{
	uint8_t idx;
	uint32_t rte = 0;

	if (NULL == dev_info)
		return;

	if (tt_dev16 == in_parms->tt) {
		idx = (uint8_t)((in_parms->destID &
				RIO_DEVID_RTE_DEV16) >> 8);
		rte = in_parms->rt->dom_table[idx].rte_val;
		if (rxs_chk_dom_rte_reg(dev_info, rte)) {
			out_parms->valid_route = false;
			out_parms->reason_for_discard = rio_rt_disc_rt_invalid;
			return;
		}
	}

	if ((tt_dev8 == in_parms->tt) ||
				(RIO_RTE_LVL_G0 == (rte & RIO_RTE_VAL))) {
		idx = (uint8_t)(in_parms->destID & 0x00FF);
		rte = in_parms->rt->dev_table[idx].rte_val;
		if (rxs_chk_dev_rte_reg(dev_info, rte)) {
			out_parms->valid_route = false;
			out_parms->reason_for_discard = rio_rt_disc_rt_invalid;
			return;
		}
	}

	out_parms->routing_table_value = rte;
	out_parms->valid_route = true;
	out_parms->reason_for_discard = rio_rt_disc_not;

	if (RIO_RTE_DROP == rte) {
		out_parms->valid_route = false;
		out_parms->reason_for_discard = rio_rt_disc_deliberately;
		return;
	}

	if (RIO_RTE_DFLT_PORT == (rte & RIO_RTE_VAL)) {
		rte = out_parms->default_route;
		if (rxs_chk_dflt_rte_reg(dev_info, rte)) {
			out_parms->valid_route = false;
			out_parms->reason_for_discard =
						rio_rt_disc_dflt_pt_invalid;
			return;
		}
		if (RIO_RTE_DROP == rte) {
			out_parms->valid_route = false;
			out_parms->reason_for_discard =
					rio_rt_disc_dflt_pt_deliberately;
			return;
		}
	}

	if (RIO_RTV_IS_MC_MSK(rte)) {
		rxs_check_multicast_routing(dev_info, in_parms, out_parms, rte);
	}
}

static uint32_t rxs_check_port_for_discard(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_out_t *out_parms,
		rio_port_t port,
		bool dflt_port)
{
	uint32_t rc = RIO_SUCCESS;
	uint32_t ctlData;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	rio_pc_get_status_in_t stat_in;
	rio_pc_get_status_out_t stat_out;

	uint32_t lpbk_mask = RXS_PLM_SPX_IMP_SPEC_CTL_DLB_EN |
				RXS_PLM_SPX_IMP_SPEC_CTL_LLB_EN;

	if (NUM_RXS_PORTS(dev_info) <= port) {
		out_parms->valid_route = false;
		out_parms->reason_for_discard = rio_rt_disc_probe_abort;
		out_parms->imp_rc = RT_PROBE(0x30);
		goto exit;
	}

	rc = DARRegRead(dev_info, RXS_TLM_SPX_FTYPE_FILT(port),
								&ctlData);
	if (RIO_SUCCESS != rc) {
		out_parms->reason_for_discard = rio_rt_disc_probe_abort;
		out_parms->imp_rc = RT_PROBE(0x58);
		goto exit;
	}

	out_parms->filter_function_active |= ctlData ? true : false;
	cfg_in.ptl.num_ports = 1;
	cfg_in.ptl.pnums[0] = port;
	rc = rxs_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->reason_for_discard = rio_rt_disc_probe_abort;
		out_parms->imp_rc = RT_PROBE(0x38);
		goto exit;
	}

	stat_in.ptl.num_ports = 1;
	stat_in.ptl.pnums[0] = port;
	rc = rxs_rio_pc_get_status(dev_info, &stat_in, &stat_out);
	if (RIO_SUCCESS != rc) {
		out_parms->reason_for_discard = rio_rt_disc_probe_abort;
		out_parms->imp_rc = RT_PROBE(0x40);
		goto exit;
	}

	if (!cfg_out.pc[0].port_available) {
		if (dflt_port) {
			out_parms->reason_for_discard =
					rio_rt_disc_dflt_pt_unavail;
		} else {
			out_parms->reason_for_discard =
					rio_rt_disc_port_unavail;
		};
		goto exit;
	}

	if (!cfg_out.pc[0].powered_up) {
		if (dflt_port) {
			out_parms->reason_for_discard =
					rio_rt_disc_dflt_pt_pwdn;
		} else {
			out_parms->reason_for_discard =
					 rio_rt_disc_port_pwdn;
		}
		goto exit;
	}

	if (!stat_out.ps[0].port_ok) {
		if (cfg_out.pc[0].xmitter_disable) {
			if (dflt_port) {
				out_parms->reason_for_discard =
					rio_rt_disc_dflt_pt_lkout_or_dis;
			} else {
				out_parms->reason_for_discard =
					rio_rt_disc_port_lkout_or_dis;
			}
			goto exit;
		}
		if (dflt_port) {
			out_parms->reason_for_discard =
					rio_rt_disc_dflt_pt_no_lp;
		} else {
			out_parms->reason_for_discard =
					rio_rt_disc_port_no_lp;
		}
		goto exit;
	}
	if (stat_out.ps[0].port_error) {
		if (dflt_port) {
			out_parms->reason_for_discard =
					rio_rt_disc_dflt_pt_fail;
		} else {
			out_parms->reason_for_discard =
					rio_rt_disc_port_fail;
		}
		goto exit;
	}

	if (cfg_out.pc[0].port_lockout) {
		if (dflt_port) {
			out_parms->reason_for_discard =
					rio_rt_disc_dflt_pt_lkout_or_dis;
		} else {
			out_parms->reason_for_discard =
					rio_rt_disc_port_lkout_or_dis;
		}
		goto exit;
	}

	if (!cfg_out.pc[0].nmtc_xfer_enable) {
		if (dflt_port) {
			out_parms->reason_for_discard =
					rio_rt_disc_dflt_pt_in_out_dis;
		} else {
			out_parms->reason_for_discard =
					rio_rt_disc_port_in_out_dis;
		}
		goto exit;
	}

	rc = DARRegRead(dev_info, RXS_PLM_SPX_IMP_SPEC_CTL(port), &ctlData);
	if (RIO_SUCCESS != rc) {
		out_parms->reason_for_discard = rio_rt_disc_probe_abort;
		out_parms->imp_rc = RT_PROBE(0x48);
		goto exit;
	}
	if (lpbk_mask & ctlData) {
		out_parms->reason_for_discard = rio_rt_disc_imp_spec;
		goto exit;
	}

	rc = DARRegRead(dev_info, RXS_PLM_SPX_STAT(port), &ctlData);
	if (RIO_SUCCESS != rc) {
		out_parms->reason_for_discard = rio_rt_disc_probe_abort;
		out_parms->imp_rc = RT_PROBE(0x50);
		goto exit;
	}
	if (ctlData & (RXS_PLM_SPX_STAT_PBM_FATAL |
			RXS_PLM_SPX_STAT_MAX_DENIAL)) {
		out_parms->reason_for_discard = rio_rt_disc_imp_spec;
		goto exit;
	}

exit:
	if (rio_rt_disc_not != out_parms->reason_for_discard) {
		out_parms->valid_route = false;
	}
	return rc;
}

static uint32_t rxs_check_for_discard(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_in_t *in_parms, rio_rt_probe_out_t *out_parms)
{
	uint32_t rc = RIO_SUCCESS;
	rio_port_t port;
	pe_rt_val rte = out_parms->routing_table_value;
	bool dflt_port = false;

	if (RIO_RTE_DFLT_PORT == rte) {
		rte = out_parms->default_route;
		dflt_port = true;
	}

	if (RIO_RTV_IS_PORT(rte)) {
		return rxs_check_port_for_discard(dev_info, out_parms,
				RIO_RTV_GET_PORT(rte), dflt_port);
	}

	// Must be a multicast mask...
	// Check each port to see if it will discard packets...
	for (port = 0; port < NUM_RXS_PORTS(dev_info); port++) {
		if (port == in_parms->probe_on_port) {
			continue;
		}
		if (!out_parms->mcast_ports[port]) {
			continue;
		}

		out_parms->valid_route = true;
		rc = rxs_check_port_for_discard(dev_info, out_parms,
						port, false);
		if (rc) {
			break;
		}
		if (!out_parms->valid_route) {
			out_parms->mcast_ports[port] = false;
		}
	}

	// Then check to see if any ports remain set...
	out_parms->valid_route = false;
	for (port = 0; port < NUM_RXS_PORTS(dev_info); port++) {
		if (!out_parms->mcast_ports[port]) {
			continue;
		}
		out_parms->valid_route = true;
		break;
	}
	return rc;
}


uint32_t rxs_rio_rt_probe(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_in_t *in_parms,
		rio_rt_probe_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t bit;
	uint32_t regVal;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->valid_route = false;
	out_parms->routing_table_value = RIO_ALL_PORTS;
	out_parms->time_to_live_active = false;
	out_parms->filter_function_active = false;
	out_parms->trace_function_active = false; // not supported on RXS

	for (bit = 0; bit < NUM_RXS_PORTS(dev_info); bit++) {
		out_parms->mcast_ports[bit] = false;
	}
	out_parms->reason_for_discard = rio_rt_disc_probe_abort;

	if (((NUM_RXS_PORTS(dev_info) <= in_parms->probe_on_port)
			&& (RIO_ALL_PORTS != in_parms->probe_on_port))
			|| (!in_parms->rt)) {
		out_parms->imp_rc = RT_PROBE(0x11);
		goto exit;
	}

	switch(in_parms->tt) {
	case tt_dev8:
		if (in_parms->destID > RIO_LAST_DEV8) {
			out_parms->imp_rc = RT_PROBE(0x12);
			goto exit;
		}
		break;
	case tt_dev16:
		break;
	default:
		out_parms->imp_rc = RT_PROBE(0x13);
		goto exit;
	}

	rc = DARRegRead(dev_info, RXS_PKT_TIME_LIVE, &regVal);
	if ( RIO_SUCCESS != rc) {
		out_parms->imp_rc = RT_PROBE(0x20);
		goto exit;
	}
	out_parms->time_to_live_active =
			(regVal & RXS_PKT_TIME_LIVE_PKT_TIME_LIVE) ?
					true : false;
	rc = RIO_SUCCESS;
	out_parms->default_route = in_parms->rt->default_route;

	rxs_check_routing(dev_info, in_parms, out_parms);

	if (out_parms->valid_route) {
		rc = rxs_check_for_discard(dev_info, in_parms, out_parms);
	}

exit:
	return rc;
}

int32_t rxs_rio_read_default_route(DAR_DEV_INFO_t *dev_info,
				rio_rt_state_t *rt, uint32_t *imp_rc)
{
	uint32_t rc;
	rio_rt_uc_info_t temp_rtv = {0, false};

	rc = DARRegRead(dev_info, RXS_ROUTE_DFLT_PORT, &temp_rtv.rte_val);
	if (RIO_SUCCESS != rc) {
		*imp_rc = RXS_READ_RTE_ENTRIES(1);
		return rc;
	}

	rxs_chk_and_corr_rtv(dev_info, &temp_rtv, false, true);
	rt->default_route = temp_rtv.rte_val;
	return RIO_SUCCESS;
}

uint32_t rxs_update_mc_msk(DAR_DEV_INFO_t *dev_info, rio_rt_state_t *rt,
			rio_port_t port, pe_rt_val mc_msk,
			uint32_t did_idx, tt_t did_sz)
{
	uint32_t mc_idx = RIO_RTV_GET_MC_MSK(mc_msk);
	uint32_t rc = RIO_SUCCESS;

	if (rt->mc_masks[mc_idx].in_use) {
		goto done;
	}

	rt->mc_masks[mc_idx].in_use = true;
	rt->mc_masks[mc_idx].allocd = true;
	rt->mc_masks[mc_idx].changed = false;
	rt->mc_masks[mc_idx].tt = did_sz;
	switch(rt->mc_masks[mc_idx].tt) {
	case tt_dev8:
		rt->mc_masks[mc_idx].mc_destID = (did_reg_t)did_idx;
		break;
	case tt_dev16:
		rt->mc_masks[mc_idx].mc_destID = (did_reg_t)(did_idx << 8);
		break;
	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		goto done;
	}

	rc = DARRegRead(dev_info, RXS_SPX_MC_Y_S_CSR(port, mc_idx),
			&rt->mc_masks[mc_idx].mc_mask);
done:
	return rc;
}

static uint32_t rxs_read_rte_entries(DAR_DEV_INFO_t *dev_info,
		rio_rt_initialize_in_t *init_in,
		uint32_t *imp_rc)
{
	uint32_t rc;
	uint32_t idx;
	rio_rt_state_t *rt;

	rt = init_in->rt;

	// Read all device table entries
	for (idx = 0; idx < RIO_RT_GRP_SZ; idx++) {
		rt->dev_table[idx].changed = false;

		// Read routing table entry for deviceID
		rc = DARRegRead(dev_info,
				RXS_SPX_L2_GY_ENTRYZ_CSR(
					init_in->set_on_port, 0, idx),
				&rt->dev_table[idx].rte_val);
		if (RIO_SUCCESS != rc) {
			*imp_rc = RXS_READ_RTE_ENTRIES(4);
			goto exit;
		}

		rxs_chk_and_corr_rtv(dev_info, &rt->dev_table[idx],
					false, false);

		if (!RIO_RTV_IS_MC_MSK(rt->dev_table[idx].rte_val)) {
			continue;
		}

		rc = rxs_update_mc_msk(dev_info, rt, init_in->set_on_port,
				rt->dev_table[idx].rte_val, idx, tt_dev8);
		if (RIO_SUCCESS != rc) {
			*imp_rc = RXS_READ_RTE_ENTRIES(6);
			goto exit;
		}
	}

	// Read all of the domain routing table entries.
	// Update multicast entries as we go...
	for (idx = 0; idx < RIO_RT_GRP_SZ; idx++) {
		rt->dom_table[idx].changed = false;

		// Read routing table entry for deviceID
		rc = DARRegRead(dev_info,
				RXS_SPX_L1_GY_ENTRYZ_CSR(
					init_in->set_on_port, 0, idx),
				&rt->dom_table[idx].rte_val);
		if (RIO_SUCCESS != rc) {
			*imp_rc = RXS_READ_RTE_ENTRIES(4);
			goto exit;
		}

		rxs_chk_and_corr_rtv(dev_info, &rt->dom_table[idx],
					true, false);

		if (!RIO_RTV_IS_MC_MSK(rt->dom_table[idx].rte_val)) {
			continue;
		}

		rc = rxs_update_mc_msk(dev_info, rt, init_in->set_on_port,
				rt->dom_table[idx].rte_val, idx, tt_dev8);
		if (RIO_SUCCESS != rc) {
			*imp_rc = RXS_READ_RTE_ENTRIES(6);
			goto exit;
		}
	}

exit:
	return rc;
}

uint32_t rxs_rio_rt_probe_all(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_all_in_t *in_parms,
		rio_rt_probe_all_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t probe_port;
	rio_rt_initialize_in_t init_in;
	rio_rt_initialize_out_t init_out;

	out_parms->imp_rc = RIO_SUCCESS;
	if ((((uint8_t)(RIO_ALL_PORTS) != in_parms->probe_on_port)
			&& (in_parms->probe_on_port >= NUM_RXS_PORTS(dev_info)))
			|| (!in_parms->rt)) {
		out_parms->imp_rc = RT_PROBE_ALL(1);
		goto exit;
	}

	probe_port = (RIO_ALL_PORTS == in_parms->probe_on_port) ?
			0 : in_parms->probe_on_port;

	init_in.set_on_port = probe_port;
	init_in.default_route = RIO_RTE_DROP;
	init_in.default_route_table_port = RIO_RTE_DROP;
	init_in.update_hw = false;
	init_in.rt = in_parms->rt;

	rc = rxs_rio_rt_initialize(dev_info, &init_in, &init_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = init_out.imp_rc;
		goto exit;
	}

	// Fill in default route value
	rc = rxs_rio_read_default_route(dev_info, in_parms->rt,
							&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	// Read all routing table entries and set up the multicast mask
	// information.  Use init_in to pass in port numbers and reduce
	// parameters.  In_parms->rt is passed in init_in.rt.
	rc = rxs_read_rte_entries(dev_info, &init_in, &out_parms->imp_rc);
exit:
	return rc;
}

static uint32_t rxs_tidy_routing_table(DAR_DEV_INFO_t *dev_info, 
		rio_rt_state_t *rt, uint8_t idx, bool is_dom_table,
		uint32_t *fail_pt)
{
	uint32_t rc = RIO_SUCCESS;
	uint16_t srch;
	bool found_one = false;
	pe_rt_val chk_val;

	if (is_dom_table) {
		chk_val = rt->dom_table[idx].rte_val;
	} else {
		chk_val = rt->dev_table[idx].rte_val;
	}

	if (!RIO_RTV_IS_MC_MSK(chk_val)) {
		goto done;
	}

	//@sonar:off - Collapsible "if" statements should be merged
	for (srch = 0; (srch < RIO_RT_GRP_SZ) && !found_one; srch++) {
		if ((idx != srch) || is_dom_table) {
			if (rt->dev_table[srch].rte_val == chk_val) {
				found_one = true;
			}
		}
		if ((idx != srch) || !is_dom_table) {
			if (rt->dom_table[srch].rte_val == chk_val) {
				found_one = true;
			}
		}
	}
	//@sonar:on

	if (!found_one) {
		rio_rt_dealloc_mc_mask_in_t in_parms;
		rio_rt_dealloc_mc_mask_out_t out_parms;

		in_parms.rt = rt;
		in_parms.mc_mask_rte = chk_val;
		rc = DSF_rio_rt_dealloc_mc_mask(dev_info,
						&in_parms, &out_parms);
		if (RIO_SUCCESS != rc) {
			*fail_pt = out_parms.imp_rc;
		}
	}
done:
	return rc;
}

uint32_t rxs_rio_rt_change_rte(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_rte_in_t *in_parms,
		rio_rt_change_rte_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	rio_rt_uc_info_t temp_rte;
	rio_rt_uc_info_t *tgt_rte;
	uint32_t mc_idx;

	out_parms->imp_rc = RIO_SUCCESS;

	if (!in_parms->rt) {
		out_parms->imp_rc = RT_CHANGE_RTE(1);
		goto exit;
	}

	// Do not allow any changes to index 0 of the domain table.
	// This must be set to "RXS_DSF_RT_USE_PACKET_ROUTE" at all times,
	// as this is the behavior required by the RXS RIO Domain register.

	if (in_parms->dom_entry && !in_parms->idx) {
		if (in_parms->rte_value == in_parms->rt->dom_table[0].rte_val) {
			rc = 0;
			goto exit;
		}
		out_parms->imp_rc = RT_CHANGE_RTE(4);
		goto exit;
	}

	// Check and correct proposed new value.
	// If the new value had to be changed, it's invalid.
	temp_rte.rte_val = in_parms->rte_value;
	temp_rte.changed = false;

	rxs_chk_and_corr_rtv(dev_info, &temp_rte, in_parms->dom_entry, false);
	if (temp_rte.changed) {
		out_parms->imp_rc = RT_CHANGE_RTE(0x10);
		goto exit;
	}

	// If the new value is a multicast mask and the multicast
	// mask value has not been set yet, bail.
	if (RIO_RTV_IS_MC_MSK(in_parms->rte_value)) {
		mc_idx = RIO_RTV_GET_MC_MSK(in_parms->rte_value);
		if (!in_parms->rt->mc_masks[mc_idx].in_use) {
			out_parms->imp_rc = RT_CHANGE_RTE(0x20);
			goto exit;
		}
	}

	// If the new value is the same as the old value
	// ignore the change
	if (in_parms->dom_entry) {
		tgt_rte = &in_parms->rt->dom_table[in_parms->idx];
	} else {
		tgt_rte = &in_parms->rt->dev_table[in_parms->idx];
	}

	if (tgt_rte->rte_val == temp_rte.rte_val) {
		rc = RIO_SUCCESS;
		goto exit;
	}

	// Routing value is changing.  Tidy the routing table to
	// deallocate unused multicast masks, if necessary.
	rc = rxs_tidy_routing_table(dev_info, in_parms->rt,
			in_parms->idx, in_parms->dom_entry, &out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}
	tgt_rte->changed = true;
	tgt_rte->rte_val = in_parms->rte_value;
exit:
	return rc;
}

uint32_t rxs_rio_rt_change_mc_mask(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_mc_mask_in_t *in_parms,
		rio_rt_change_mc_mask_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t chg_idx, dom_idx, dev_idx;
	uint32_t illegal_ports = ~((1 << NUM_RXS_PORTS(dev_info)) - 1);
	rio_rt_mc_info_t *tgt_mc_msk;
	rio_rt_uc_info_t *dom_rte;
	rio_rt_uc_info_t *dev_rte;

	out_parms->imp_rc = RIO_SUCCESS;

	if (!in_parms->rt) {
		out_parms->imp_rc = CHANGE_MC_MASK(1);
		goto exit;
	}

	// Check destination ID value against tt
	if ((in_parms->mc_info.mc_destID > RIO_LAST_DEV16)
			|| ((in_parms->mc_info.mc_destID > RIO_LAST_DEV8)
					&& (tt_dev8 == in_parms->mc_info.tt))) {
		out_parms->imp_rc = CHANGE_MC_MASK(2);
		goto exit;
	}

	// Check that the multicast mask does not select ports which do not exist
	// on the RXS device
	if ((in_parms->mc_info.mc_mask & illegal_ports)
			|| (in_parms->mc_info.tt > tt_dev16)) {
		out_parms->imp_rc = CHANGE_MC_MASK(3);
		goto exit;
	}

	// If the updated mc mask is not in use, deallocate it and return.
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

	chg_idx = RIO_RTV_GET_MC_MSK(in_parms->mc_mask_rte);

	if (chg_idx >= RXS2448_MC_MASK_CNT) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = CHANGE_MC_MASK(4);
		goto exit;
	}

	rc = RIO_SUCCESS;
	tgt_mc_msk = &in_parms->rt->mc_masks[chg_idx];

	// Always update multicast mask
	if (tgt_mc_msk->mc_mask != in_parms->mc_info.mc_mask) {
		tgt_mc_msk->mc_mask = in_parms->mc_info.mc_mask;
		tgt_mc_msk->changed = true;
	}

	// Update routing table entry or entries
	dom_idx = (in_parms->mc_info.mc_destID & 0xFF00) >> 8;
	dom_rte = &in_parms->rt->dom_table[dom_idx];
	dev_idx = (in_parms->mc_info.mc_destID & 0x00FF);
	dev_rte = &in_parms->rt->dev_table[dev_idx];

	if (tt_dev16 == in_parms->mc_info.tt) {
		// If the domain table entry points to the device table,
		// update the device table.
		if (RIO_RTV_IS_LVL_GRP(dom_rte->rte_val)) {
			if (dev_rte->rte_val != in_parms->mc_mask_rte) {
				dev_rte->rte_val = in_parms->mc_mask_rte;
				dev_rte->changed = true;
			}
		} else {
			// Only update the domain routing table.
			// This will apply the multicasst mask to all
			// dev16 deviceIDs of the form 0xDDXX, where DD
			// matches the upper byte of the current destID.
			if (dom_rte->rte_val != in_parms->mc_mask_rte) {
				dom_rte->rte_val = in_parms->mc_mask_rte;
				dom_rte->changed = true;
			}
		}
	} else {
		if (dev_rte->rte_val != in_parms->mc_mask_rte) {
			dev_rte->rte_val = in_parms->mc_mask_rte;
			dev_rte->changed = true;
		}
	}

	// If the the multicast mask is allocated and in use,
	// the destID and TT must already be valid.  Not need to
	// change.
	if ((tgt_mc_msk->in_use) && (tgt_mc_msk->allocd)) {
		goto exit;
	}

	// Update destID and TT
	tgt_mc_msk->changed = true;
	tgt_mc_msk->in_use = true;
	tgt_mc_msk->allocd = true;

	tgt_mc_msk->mc_destID = in_parms->mc_info.mc_destID;
	tgt_mc_msk->tt = in_parms->mc_info.tt;

exit:
	return rc;
}

#endif /* RXS_DAR_WANTED */

#ifdef __cplusplus
}
#endif
