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
#include "RapidIO_Statistics_Counter_API.h"
#include "Tsi57x_DeviceDriver.h"
#include "Tsi578.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TSI57X_DAR_WANTED

#define PRIO_MASK (TSI578_SPX_PSC0N1_CTRL_PS1_PRIO0 | \
		TSI578_SPX_PSC0N1_CTRL_PS1_PRIO1 | \
		TSI578_SPX_PSC0N1_CTRL_PS1_PRIO2 | \
		TSI578_SPX_PSC0N1_CTRL_PS1_PRIO3)

uint32_t rio_sc_cfg_tsi57x_ctr(DAR_DEV_INFO_t *dev_info,
		rio_sc_cfg_tsi57x_ctr_in_t *in_parms,
		rio_sc_cfg_tsi57x_ctr_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t new_ctl = 0, ctl_reg, new_ctl_reg, reg_mask;
	uint8_t p_to_i[TSI578_MAX_PORTS] = {TSI578_MAX_PORTS};
	uint8_t srch_i, srch_p, port_num;
	bool found;
	bool check;
	struct DAR_ptl good_ptl;

	out_parms->imp_rc = RIO_SUCCESS;

	if (NULL == in_parms->dev_ctrs) {
		out_parms->imp_rc = SC_CFG_TSI57X_CTR(0x01);
		goto exit;
	}

	if (NULL == in_parms->dev_ctrs->p_ctrs) {
		out_parms->imp_rc = SC_CFG_TSI57X_CTR(0x02);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = SC_CFG_TSI57X_CTR(0x4);
		goto exit;
	}

	rc = RIO_ERR_INVALID_PARAMETER;

	if (!in_parms->dev_ctrs->num_p_ctrs
			|| (in_parms->dev_ctrs->num_p_ctrs > RIO_MAX_PORTS)
			|| (in_parms->dev_ctrs->num_p_ctrs
					< in_parms->dev_ctrs->valid_p_ctrs)) {
		out_parms->imp_rc = SC_CFG_TSI57X_CTR(0x06);
		goto exit;
	}

	if ((in_parms->dev_ctrs->num_p_ctrs < good_ptl.num_ports)
			|| (in_parms->dev_ctrs->valid_p_ctrs
					< good_ptl.num_ports)) {
		out_parms->imp_rc = SC_CFG_TSI57X_CTR(0x08);
		goto exit;
	}

	// Create SC_CTL
	if (rio_sc_disabled != in_parms->ctr_type) {
		new_ctl = ((uint32_t)(in_parms->prio_mask) << 12) & PRIO_MASK;
		// It is a programming error to have an empty priority mask when counting
		// packets or packet data...
		check = (rio_sc_uc_req_pkts == in_parms->ctr_type);
		check |= (rio_sc_uc_pkts == in_parms->ctr_type);
		check |= (rio_sc_uc_4b_data == in_parms->ctr_type);
		check |= (rio_sc_mc_pkts == in_parms->ctr_type);
		check |= (rio_sc_mc_4b_data == in_parms->ctr_type);
		if (!new_ctl && check) {
			out_parms->imp_rc = SC_CFG_TSI57X_CTR(0x20);
			goto exit;
		}
		new_ctl |= (in_parms->tx) ? TSI578_SPX_PSC0N1_CTRL_PS1_DIR : 0;
		switch (in_parms->ctr_type) {
		case rio_sc_uc_req_pkts:
			break;
		case rio_sc_uc_pkts:
			new_ctl |= 0x1;
			break;
		case rio_sc_retries:
			new_ctl |= 0x2 | PRIO_MASK;
			break;
		case rio_sc_all_cs:
			new_ctl |= 0x3 | PRIO_MASK;
			break;
		case rio_sc_uc_4b_data:
			new_ctl |= 0x4;
			break;
		case rio_sc_mc_pkts:
			new_ctl |= 0x5;
			break;
		case rio_sc_mecs:
			new_ctl |= 0x6 | PRIO_MASK;
			break;
		case rio_sc_mc_4b_data:
			new_ctl |= 0x7;
			break;
		default:
			out_parms->imp_rc = SC_CFG_TSI57X_CTR(0x30);
			goto exit;
		}
	}

	if (in_parms->ctr_idx & 1) {
		reg_mask = 0x0000FFFF;
	} else {
		reg_mask = 0xFFFF0000;
		new_ctl = new_ctl << 16;
	}

	// Update hardware and data structures to reflect change in programming.
	// - program the counter control values requested
	// - update the associated counters structure (what is counted)
	// - clear the counter value in the counters structure if what is counted changed
	// - clear the physical counter value if what is counted changed
	//
	for (srch_p = 0; srch_p < good_ptl.num_ports; srch_p++) {
		port_num = good_ptl.pnums[srch_p];
		found = false;
		for (srch_i = 0; srch_i < in_parms->dev_ctrs->valid_p_ctrs;
				srch_i++) {
			if (in_parms->dev_ctrs->p_ctrs[srch_i].pnum
					== port_num) {
				found = true;
				// If the port hasn't previously been programmed and the counter structure is
				// correctly initialized, keep going...
				if ((TSI578_MAX_PORTS == p_to_i[port_num])
						&& (TSI578_NUM_PERF_CTRS
								== in_parms->dev_ctrs->p_ctrs[srch_i].ctrs_cnt)) {
					p_to_i[port_num] = srch_i;
				} else {
					// Port number appears multiple times in the list,
					// or number of performance counters is incorrect/uninitialized...
					rc = RIO_ERR_INVALID_PARAMETER;
					out_parms->imp_rc = SC_CFG_TSI57X_CTR(
							0x40 + port_num);
					goto exit;
				}

				// Always program the control value...
				rc =
						DARRegRead(dev_info,
								TSI578_SPX_PSC_CTRL(
										port_num,
										in_parms->ctr_idx),
								&ctl_reg);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = SC_CFG_TSI57X_CTR(
							0x50);
					goto exit;
				}
				new_ctl_reg = ctl_reg & ~reg_mask;
				new_ctl_reg |= new_ctl;
				rc =
						DARRegWrite(dev_info,
								TSI578_SPX_PSC_CTRL(
										port_num,
										in_parms->ctr_idx),
								new_ctl_reg);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = SC_CFG_TSI57X_CTR(
							0x51);
					goto exit;
				}

				if ((in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[in_parms->ctr_idx].sc
						!= in_parms->ctr_type)
						|| (in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[in_parms->ctr_idx].tx
								!= in_parms->tx)
						|| (ctl_reg != new_ctl_reg)) {
					// If the counted value has changed in the structure or in hardware,
					// zero the counters and zero hardware counters
					in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[in_parms->ctr_idx].sc =
							in_parms->ctr_type;
					in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[in_parms->ctr_idx].tx =
							in_parms->tx;
					in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[in_parms->ctr_idx].total =
							0;
					in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[in_parms->ctr_idx].last_inc =
							0;
					rc =
							DARRegRead(dev_info,
									TSI578_SPX_PSCY(
											port_num,
											in_parms->ctr_idx),
									&ctl_reg);
					if (RIO_SUCCESS != rc) {
						out_parms->imp_rc =
								SC_CFG_TSI57X_CTR(
										0x52);
						goto exit;
					}
				}
			}
		}
		if (!found) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = SC_CFG_TSI57X_CTR(0x53);
			goto exit;
		}
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_sc_init_dev_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_init_dev_ctrs_in_t *in_parms,
		rio_sc_init_dev_ctrs_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	rio_sc_ctr_val_t init_val = INIT_RIO_SC_CTR_VAL;
	uint8_t idx,
	cntr_i;
	struct DAR_ptl good_ptl;

	out_parms->imp_rc = RIO_SUCCESS;

	if (NULL == in_parms->dev_ctrs) {
		out_parms->imp_rc = SC_INIT_DEV_CTRS(1);
		goto exit;
	}

	if (NULL == in_parms->dev_ctrs->p_ctrs) {
		out_parms->imp_rc = SC_INIT_DEV_CTRS(2);
		goto exit;
	}

	if (!in_parms->dev_ctrs->num_p_ctrs
			|| (in_parms->dev_ctrs->num_p_ctrs > RIO_MAX_PORTS)) {
		out_parms->imp_rc = SC_INIT_DEV_CTRS(3);
		goto exit;
	}

	if (((RIO_ALL_PORTS == in_parms->ptl.num_ports)
			&& (in_parms->dev_ctrs->num_p_ctrs
					< TSI57X_NUM_PORTS(dev_info)))
			|| ((RIO_ALL_PORTS != in_parms->ptl.num_ports)
					&& (in_parms->dev_ctrs->num_p_ctrs
							< in_parms->ptl.num_ports))) {
		out_parms->imp_rc = SC_INIT_DEV_CTRS(4);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = SC_INIT_DEV_CTRS(0x10);
		goto exit;
	}

	in_parms->dev_ctrs->valid_p_ctrs = good_ptl.num_ports;
	for (idx = 0; idx < good_ptl.num_ports; idx++) {
		in_parms->dev_ctrs->p_ctrs[idx].pnum = good_ptl.pnums[idx];
		in_parms->dev_ctrs->p_ctrs[idx].ctrs_cnt = TSI578_NUM_PERF_CTRS;
		for (cntr_i = 0; cntr_i < TSI578_NUM_PERF_CTRS; cntr_i++) {
			in_parms->dev_ctrs->p_ctrs[idx].ctrs[cntr_i] = init_val;
		}
	}

	rc = RIO_SUCCESS;

exit:
	return rc;
}

/* Reads enabled counters on selected ports
 */
uint32_t tsi57x_rio_sc_read_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_read_ctrs_in_t *in_parms,
		rio_sc_read_ctrs_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t p_to_i[TSI578_MAX_PORTS] = {TSI578_MAX_PORTS};
	uint8_t srch_i, srch_p, port_num, cntr;
	bool found;
	struct DAR_ptl good_ptl;

	out_parms->imp_rc = RIO_SUCCESS;

	if (NULL == in_parms->dev_ctrs) {
		out_parms->imp_rc = SC_READ_CTRS(0x01);
		goto exit;
	}

	if (NULL == in_parms->dev_ctrs->p_ctrs) {
		out_parms->imp_rc = SC_READ_CTRS(0x02);
		goto exit;
	}

	if (!in_parms->dev_ctrs->num_p_ctrs
			|| (in_parms->dev_ctrs->num_p_ctrs > TSI578_MAX_PORTS)
			|| (in_parms->dev_ctrs->num_p_ctrs
					< in_parms->dev_ctrs->valid_p_ctrs)) {
		out_parms->imp_rc = SC_READ_CTRS(0x03);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = SC_READ_CTRS(0x30);
		goto exit;
	}

	if ((in_parms->dev_ctrs->num_p_ctrs < good_ptl.num_ports)
			|| (in_parms->dev_ctrs->valid_p_ctrs
					< good_ptl.num_ports)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = SC_READ_CTRS(0x04);
		goto exit;
	}

	// For generality, must establish a list of ports.
	// Do not assume that the port number equals the index in the structure...

	for (srch_p = 0; srch_p < good_ptl.num_ports; srch_p++) {
		port_num = good_ptl.pnums[srch_p];
		found = false;
		for (srch_i = 0; srch_i < in_parms->dev_ctrs->valid_p_ctrs;
				srch_i++) {
			if (in_parms->dev_ctrs->p_ctrs[srch_i].pnum
					== port_num) {
				found = true;
				// If the port hasn't previously been read and the counter structure is
				// correctly initialized, keep going...
				if ((TSI578_MAX_PORTS == p_to_i[port_num])
						&& (TSI578_NUM_PERF_CTRS
								== in_parms->dev_ctrs->p_ctrs[srch_i].ctrs_cnt)) {
					p_to_i[port_num] = srch_i;
				} else {
					// Port number appears multiple times in the list,
					// or number of performance counters is incorrect/uninitialized...
					rc = RIO_ERR_INVALID_PARAMETER;
					out_parms->imp_rc = SC_READ_CTRS(
							0x50 + port_num);
					goto exit;
				}

				// Read the port performance counters...
				for (cntr = 0; cntr < TSI578_NUM_PERF_CTRS;
						cntr++) {
					if (rio_sc_disabled
							!= in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[cntr].sc) {
						rc =
								DARRegRead(
										dev_info,
										TSI578_SPX_PSCY(
												port_num,
												cntr),
										&in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[cntr].last_inc);
						if (RIO_SUCCESS != rc) {
							rc =
									RIO_ERR_INVALID_PARAMETER;
							out_parms->imp_rc =
									SC_READ_CTRS(
											0x70 + cntr);
							goto exit;
						}
						in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[cntr].total +=
								in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[cntr].last_inc;
					}
				}
			}
		}
		if (!found) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = SC_READ_CTRS(0x90 + srch_p);
			goto exit;
		}
	}

exit:
	return rc;
}

#endif /* TSI57X_DAR_WANTED */

#ifdef __cplusplus
}
#endif
