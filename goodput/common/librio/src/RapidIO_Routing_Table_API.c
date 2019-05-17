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

#include "CPS_DeviceDriver.h"
#include "RXS_DeviceDriver.h"
#include "Tsi721_DeviceDriver.h"
#include "Tsi57x_DeviceDriver.h"
#include "DSF_DB_Private.h"
#include "RXS2448.h"

#ifdef __cplusplus
extern "C" {
#endif

char *rio_em_disc_reason_names[(uint8_t)(rio_rt_disc_last)] = {
		(char *)"NoDiscard",	// rio_rt_disc_not
		(char *)"RteInvalid",	// rio_rt_disc_rt_invalid
		(char *)"Deliberate",	// rio_rt_disc_deliberately
		(char *)"PrtUnavail",	// rio_rt_disc_port_unavail
		(char *)"PrtPwrDwn",	// rio_rt_disc_port_pwdn
		(char *)"PrtFail",	// rio_rt_disc_port_fail
		(char *)"PrtNoLp",	// rio_rt_disc_port_no_lp
		(char *)"LkoutOrDis",	// rio_rt_disc_port_lkout_or_dis
		(char *)"InpOutpDis",	// rio_rt_disc_port_in_out_dis
		(char *)"MCEmpty",	// rio_rt_disc_mc_empty
		(char *)"MC1bit",	// rio_rt_disc_mc_one_bit
		(char *)"MCMultMask",	// rio_rt_disc_mc_mult_masks
		(char *)"DPInvalid",	// rio_rt_disc_dflt_pt_invalid
		(char *)"DPDelibrat",	// rio_rt_disc_dflt_pt_deliberately
		(char *)"DPPrtUaval",	// rio_rt_disc_dflt_pt_unavail
		(char *)"DPPwrDwn",	// rio_rt_disc_dflt_pt_pwdn
		(char *)"DPFail",	// rio_rt_disc_dflt_pt_fail
		(char *)"DPNoLp",	// rio_rt_disc_dflt_pt_no_lp
		(char *)"DPLkoutDis",	// rio_rt_disc_dflt_pt_lkout_or_dis
		(char *)"DPInpOutpD",	// rio_rt_disc_dflt_pt_in_out_dis
		(char *)"ProbeABORT"	// rio_rt_disc_probe_abort
};

void rio_rt_check_multicast_routing(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_in_t *in_parms, rio_rt_probe_out_t *out_parms)
{
	uint8_t mc_idx, bit;
	uint32_t mc_mask;
	bool found = false;

	for (mc_idx = 0; mc_idx < NUM_MC_MASKS(dev_info); mc_idx++) {
		if ((in_parms->tt == in_parms->rt->mc_masks[mc_idx].tt)
				&& (in_parms->rt->mc_masks[mc_idx].in_use)) {
			if (tt_dev8 == in_parms->tt) {
				mc_mask = 0x00FF;
			} else {
				mc_mask = 0xFFFF;
			}

			if ((in_parms->destID & mc_mask)
					== (in_parms->rt->mc_masks[mc_idx].mc_destID
							& mc_mask)) {
				if (found) {
					out_parms->reason_for_discard =
							rio_rt_disc_mc_mult_masks;
					out_parms->valid_route = false;
					break;
				} else {
					found = true;
					out_parms->routing_table_value =
						RIO_RTV_MC_MSK(mc_idx);
					for (bit = 0; bit < NUM_PORTS(dev_info);
							bit++) {
						out_parms->mcast_ports[bit] =
								((uint32_t)(1
										<< bit)
										& in_parms->rt->mc_masks[mc_idx].mc_mask) ?
										true :
										false;
					}

					if (in_parms->rt->mc_masks[mc_idx].mc_mask) {
						if ((uint32_t)((uint32_t)(1)
								<< in_parms->probe_on_port)
								== in_parms->rt->mc_masks[mc_idx].mc_mask) {
							out_parms->reason_for_discard =
									rio_rt_disc_mc_one_bit;
						} else {
							out_parms->reason_for_discard =
									rio_rt_disc_not;
							out_parms->valid_route =
									true;
						}
					} else {
						out_parms->reason_for_discard =
								rio_rt_disc_mc_empty;
					}
				}
			}
		}
	}
}

void rio_rt_check_unicast_routing(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_in_t *in_parms, rio_rt_probe_out_t *out_parms)
{
	uint8_t idx;
	uint32_t phys_rte, rte = 0;
	bool dflt_pt;

	if (NULL == dev_info) {
		return;
	}

	if (tt_dev16 == in_parms->tt) {
		idx = (uint8_t)((in_parms->destID & 0xFF00) >> 8);
		rte = in_parms->rt->dom_table[idx].rte_val;
	}

	if ((tt_dev8 == in_parms->tt) || (RIO_RTE_LVL_G0 == rte)) {
		idx = (uint8_t)(in_parms->destID & 0x00FF);
		rte = in_parms->rt->dev_table[idx].rte_val;
	}

	out_parms->routing_table_value = rte;
	out_parms->valid_route = true;
	out_parms->reason_for_discard = rio_rt_disc_not;
	dflt_pt = (RIO_RTE_DFLT_PORT == rte) ? true : false;

	phys_rte = (dflt_pt) ? in_parms->rt->default_route : rte;

	if (RIO_RTE_DROP == phys_rte) {
		out_parms->valid_route = false;
		out_parms->reason_for_discard =
				(dflt_pt) ? rio_rt_disc_dflt_pt_deliberately : rio_rt_disc_deliberately;
	} else {
		if (phys_rte >= NUM_PORTS(dev_info)) {
			out_parms->valid_route = false;
			out_parms->reason_for_discard =
					(dflt_pt) ? rio_rt_disc_dflt_pt_invalid : rio_rt_disc_rt_invalid;
		}
	}
}

/* User function calls for a routing table configuration */
uint32_t rio_rt_initialize(DAR_DEV_INFO_t *dev_info,
		rio_rt_initialize_in_t *in_parms,
		rio_rt_initialize_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_rt_initialize(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_rt_initialize(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return DSF_rio_rt_initialize(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_rt_initialize(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_rt_initialize(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_rt_probe(DAR_DEV_INFO_t *dev_info, rio_rt_probe_in_t *in_parms,
		rio_rt_probe_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_rt_probe(dev_info, in_parms, out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_rt_probe(dev_info, in_parms, out_parms);
		case RIO_TSI721_DEVICE:
			return DSF_rio_rt_probe(dev_info, in_parms, out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_rt_probe(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_rt_probe(dev_info, in_parms, out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_rt_probe_all(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_all_in_t *in_parms,
		rio_rt_probe_all_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_rt_probe_all(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_rt_probe_all(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return DSF_rio_rt_probe_all(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_rt_probe_all(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_rt_probe_all(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_rt_set_all(DAR_DEV_INFO_t *dev_info, rio_rt_set_all_in_t *in_parms,
		rio_rt_set_all_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_rt_set_all(dev_info, in_parms, out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_rt_set_all(dev_info, in_parms, out_parms);
		case RIO_TSI721_DEVICE:
			return DSF_rio_rt_set_all(dev_info, in_parms, out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_rt_set_all(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_rt_set_all(dev_info, in_parms, out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_rt_set_changed(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_changed_in_t *in_parms,
		rio_rt_set_changed_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_rt_set_changed(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_rt_set_changed(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return DSF_rio_rt_set_changed(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_rt_set_changed(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_rt_set_changed(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_rt_alloc_mc_mask(DAR_DEV_INFO_t *dev_info,
		rio_rt_alloc_mc_mask_in_t *in_parms,
		rio_rt_alloc_mc_mask_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return DSF_rio_rt_alloc_mc_mask(dev_info,
					in_parms, out_parms);
		case RIO_RXS_DEVICE:
			return DSF_rio_rt_default_alloc_mc_mask(dev_info,
					in_parms, out_parms,
					RXS2448_MC_MASK_CNT);
		case RIO_TSI721_DEVICE:
			return DSF_rio_rt_alloc_mc_mask(dev_info,
					in_parms, out_parms);
		case RIO_TSI57X_DEVICE:
			return DSF_rio_rt_alloc_mc_mask(dev_info,
					in_parms, out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_rt_alloc_mc_mask(dev_info,
					in_parms, out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_rt_dealloc_mc_mask(DAR_DEV_INFO_t *dev_info,
		rio_rt_dealloc_mc_mask_in_t *in_parms,
		rio_rt_dealloc_mc_mask_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return DSF_rio_rt_dealloc_mc_mask(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return DSF_rio_rt_dealloc_mc_mask(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return DSF_rio_rt_dealloc_mc_mask(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return DSF_rio_rt_dealloc_mc_mask(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_rt_dealloc_mc_mask(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_rt_change_rte(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_rte_in_t *in_parms,
		rio_rt_change_rte_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_rt_change_rte(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_rt_change_rte(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return DSF_rio_rt_change_rte(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_rt_change_rte(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_rt_change_rte(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

uint32_t rio_rt_change_mc_mask(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_mc_mask_in_t *in_parms,
		rio_rt_change_mc_mask_out_t *out_parms)
{
	NULL_CHECK

	if (VALIDATE_DEV_INFO(dev_info)) {
		//@sonar:off - c:S1871, c:S3458
		switch (dev_info->driver_family) {
		case RIO_CPS_DEVICE:
			return CPS_rio_rt_change_mc_mask(dev_info, in_parms,
					out_parms);
		case RIO_RXS_DEVICE:
			return rxs_rio_rt_change_mc_mask(dev_info, in_parms,
					out_parms);
		case RIO_TSI721_DEVICE:
			return DSF_rio_rt_change_mc_mask(dev_info, in_parms,
					out_parms);
		case RIO_TSI57X_DEVICE:
			return tsi57x_rio_rt_change_mc_mask(dev_info, in_parms,
					out_parms);
		case RIO_UNKNOWN_DEVICE:
			return DSF_rio_rt_change_mc_mask(dev_info, in_parms,
					out_parms);
		case RIO_UNITIALIZED_DEVICE:
		default:
			return RIO_DAR_IMP_SPEC_FAILURE;
		}
		//@sonar:on
	}
	return DAR_DB_INVALID_HANDLE;
}

#ifdef __cplusplus
}
#endif
