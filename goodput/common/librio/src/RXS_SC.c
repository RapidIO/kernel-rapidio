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
#include "RapidIO_Statistics_Counter_API.h"
#include "RXS2448.h"
#include "src/RXS_DeviceDriver.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RXS_DAR_WANTED

static uint32_t rxs_read_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_read_ctrs_in_t *in_parms,
		rio_sc_read_ctrs_out_t *out_parms, int srch_i,
		rio_port_t port_num)
{
	int cntr;
	uint32_t count;
	uint64_t l_c; // last counter value
	uint64_t c_c; // current counter value
	uint64_t tot; // new total counter value
	rio_sc_ctr_val_t *counter;
	uint32_t rc;

	for (cntr = 0; cntr < RXS2448_MAX_SC; cntr++) {
		counter = &in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[cntr];
		if (rio_sc_disabled == counter->sc) {
			continue;
		}

		rc = DARRegRead(dev_info, RXS_SPX_PCNTR_CNT(port_num, cntr),
				&count);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = SC_READ_RXS_CTRS(0x71 + cntr);
			goto exit;
		}
		c_c = count;
		l_c = counter->total & (uint64_t)0x00000000FFFFFFFF;
		tot = counter->total & (uint64_t)0xFFFFFFFF00000000;
		tot |= c_c;

		// If the counter has wrapped, increment tot by 1.
		//
		// Note: if c_c and l_c are equal, assume that the
		// counter has not changed since it was last read.
		if (l_c > c_c) {
			tot += (uint64_t)0x0000000100000000;
			c_c |= (uint64_t)0x0000000100000000;
		}
		counter->last_inc = c_c - l_c;
		counter->total = tot;
	}
	rc = RIO_SUCCESS;

exit:
	return rc;
}


/* Configure counters on selected ports of a
 * RXS device.
 */
uint32_t rio_sc_cfg_rxs_ctr(DAR_DEV_INFO_t *dev_info,
		rio_sc_cfg_rxs_ctr_in_t *in_parms,
		rio_sc_cfg_rxs_ctr_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t new_ctl = 0, ctl_reg;
	rio_port_t s_i, srch_p, pt;
	bool found;
	struct DAR_ptl good_ptl;
	bool srio = true;
	uint8_t c_i = in_parms->ctr_idx;

	out_parms->imp_rc = RIO_SUCCESS;

	if (NULL == in_parms->dev_ctrs) {
		out_parms->imp_rc = SC_CFG_RXS_CTRS(0x01);
		goto exit;
	}

	if (NULL == in_parms->dev_ctrs->p_ctrs) {
		out_parms->imp_rc = SC_CFG_RXS_CTRS(0x02);
		goto exit;
	}

	if (!in_parms->dev_ctrs->num_p_ctrs
			|| (in_parms->dev_ctrs->num_p_ctrs > RIO_MAX_PORTS)
			|| (in_parms->dev_ctrs->num_p_ctrs
					< in_parms->dev_ctrs->valid_p_ctrs)) {
		out_parms->imp_rc = SC_CFG_RXS_CTRS(0x03);
		goto exit;
	}

	if (((RIO_ALL_PORTS == in_parms->ptl.num_ports)
			&& (in_parms->dev_ctrs->num_p_ctrs
					< NUM_RXS_PORTS(dev_info)))
			|| ((RIO_ALL_PORTS != in_parms->ptl.num_ports)
					&& (in_parms->dev_ctrs->num_p_ctrs
							< in_parms->ptl.num_ports))) {
		out_parms->imp_rc = SC_CFG_RXS_CTRS(0x04);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = SC_CFG_RXS_CTRS(0x10);
		goto exit;
	}

	if ((in_parms->dev_ctrs->num_p_ctrs < good_ptl.num_ports)
			|| (in_parms->dev_ctrs->valid_p_ctrs
					< good_ptl.num_ports)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = SC_CFG_RXS_CTRS(0x05);
		goto exit;
	}

	// Create SC_CTL
	new_ctl = ((uint32_t)(in_parms->prio_mask) << 8);
	new_ctl &= RXS_SPC_PCNTR_CTL_PRIO;
	new_ctl |= (in_parms->tx) ? RXS_SPX_PCNTR_CTL_TX : 0;

	switch (in_parms->ctr_type) {
	case rio_sc_pkt:
		new_ctl |= RXS_SPX_PCNTR_CTL_SEL_RIO_PKT;
		break;
	case rio_sc_fab_pkt:
		new_ctl |= RXS_SPX_PCNTR_CTL_SEL_FAB_PKT;
		srio = false;
		// Fabric packet counts are prioirty specific.
		// Report a programming error if the priority mask is 0.
		if (!in_parms->prio_mask) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = SC_CFG_RXS_CTRS(0x31);
			goto exit;
		}
		break;
	case rio_sc_rio_pload:
		new_ctl |= RXS_SPX_PCNTR_CTL_SEL_RIO_PAYLOAD;
		break;
	case rio_sc_fab_pload:
		new_ctl |= RXS_SPX_PCNTR_CTL_SEL_FAB_PAYLOAD;
		srio = false;
		// Fabric packet data counts are prioirty specific.
		// Report a programming error if the priority mask is 0.
		if (!in_parms->prio_mask) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = SC_CFG_RXS_CTRS(0x32);
			goto exit;
		}
		break;
	case rio_sc_rio_bwidth:
		// Count of the total number of code-groups/codewords
		// transmitted on the RapidIO interface per lane.
		// Hardware does not support count for RX (!TX).
		new_ctl |= RXS_SPX_PCNTR_CTL_SEL_RIO_TTL_PKTCNTR;
		if (!in_parms->tx) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = SC_CFG_RXS_CTRS(0x33);
			goto exit;
		}
		break;
	case rio_sc_retries:
		new_ctl |= RXS_SPX_PCNTR_CTL_SEL_RETRIES;
		break;
	case rio_sc_pna:
		new_ctl |= RXS_SPX_PCNTR_CTL_SEL_PNA;
		break;
	case rio_sc_pkt_drop:
		new_ctl |= RXS_SPX_PCNTR_CTL_SEL_PKT_DROP;
		// Packet drop counts are prioirty specific.
		// Report a programming error if the priority mask is 0.
		if (!in_parms->prio_mask) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = SC_CFG_RXS_CTRS(0x34);
			goto exit;
		}
		break;
	case rio_sc_disabled:
		new_ctl |= RXS_SPX_PCNTR_CTL_SEL_DISABLED;
		break;
	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = SC_CFG_RXS_CTRS(0x35);
		goto exit;
	}

	for (srch_p = 0; srch_p < good_ptl.num_ports; srch_p++) {
		pt = good_ptl.pnums[srch_p];
		found = false;

		// Enable counters for each port before programming the counter
		// control value.
		if (in_parms->ctr_en) {
			rc = DARRegWrite(dev_info, RXS_SPX_PCNTR_EN(pt),
			RXS_SPX_PCNTR_EN_ENABLE);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = SC_CFG_RXS_CTRS(0x40);
				goto exit;
			}
		}

		for (s_i = 0; s_i < in_parms->dev_ctrs->valid_p_ctrs; s_i++) {
			if (in_parms->dev_ctrs->p_ctrs[s_i].pnum != pt) {
				continue;
			}
			rio_sc_ctr_val_t *ctr_p;
			ctr_p = &in_parms->dev_ctrs->p_ctrs[s_i].ctrs[c_i];
			found = true;
			// Always program the control value...
			rc = DARRegRead(dev_info,
					RXS_SPX_PCNTR_CTL(pt, c_i),
					&ctl_reg);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = SC_CFG_RXS_CTRS(0x41);
				goto exit;
			}

			rc = DARRegWrite(dev_info,
					RXS_SPX_PCNTR_CTL(pt, c_i),
					new_ctl);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = SC_CFG_RXS_CTRS(0x42);
				goto exit;
			}

			if (ctl_reg != new_ctl) {
				// If the counted value has changed,
				// update the control structure and
				// zero the hw & sw  counters
				ctr_p->sc = in_parms->ctr_type;
				ctr_p->tx = in_parms->tx;
				ctr_p->srio = srio;
				ctr_p->total = 0;
				ctr_p->last_inc = 0;
				rc = DARRegWrite(dev_info,
						RXS_SPX_PCNTR_CNT(pt, c_i),
						0);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = SC_CFG_RXS_CTRS(
							0x43);
					goto exit;
				}
			}
		}
		if (!found) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = SC_CFG_RXS_CTRS(0x44);
			goto exit;
		}
		// Disable counters for each port after programming the counter
		// control value.
		if (!in_parms->ctr_en) {
			rio_sc_ctr_val_t init_val = INIT_RIO_SC_CTR_VAL;
			int cntr_i;

			rc = DARRegWrite(dev_info, RXS_SPX_PCNTR_EN(pt), 0);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = SC_CFG_RXS_CTRS(0x60);
				goto exit;
			}
			for (cntr_i = 0; cntr_i < RXS2448_MAX_SC; cntr_i++) {
				in_parms->dev_ctrs->p_ctrs[s_i].ctrs[cntr_i] =
						init_val;
			}
		}
	}

	rc = RIO_SUCCESS;
exit:
	return rc;
}
/* Reads enabled counters on selected ports
 */
uint32_t rxs_rio_sc_init_dev_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_init_dev_ctrs_in_t *in_parms,
		rio_sc_init_dev_ctrs_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t idx, cntr_i;
	rio_sc_ctr_val_t init_val = INIT_RIO_SC_CTR_VAL;
	struct DAR_ptl good_ptl;

	out_parms->imp_rc = RIO_SUCCESS;

	if (NULL == in_parms->dev_ctrs) {
		out_parms->imp_rc = SC_INIT_RXS_CTRS(0x01);
		goto exit;
	}

	if (NULL == in_parms->dev_ctrs->p_ctrs) {
		out_parms->imp_rc = SC_INIT_RXS_CTRS(0x02);
		goto exit;
	}

	if (!in_parms->dev_ctrs->num_p_ctrs
			|| (in_parms->dev_ctrs->num_p_ctrs > RIO_MAX_PORTS)
			|| (in_parms->dev_ctrs->num_p_ctrs
					< in_parms->dev_ctrs->valid_p_ctrs)) {
		out_parms->imp_rc = SC_INIT_RXS_CTRS(0x03);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = SC_INIT_RXS_CTRS(0x10);
		goto exit;
	}

	in_parms->dev_ctrs->valid_p_ctrs = good_ptl.num_ports;
	for (idx = 0; idx < good_ptl.num_ports; idx++) {
		in_parms->dev_ctrs->p_ctrs[idx].pnum = good_ptl.pnums[idx];
		in_parms->dev_ctrs->p_ctrs[idx].ctrs_cnt = RXS2448_MAX_SC;
		for (cntr_i = 0; cntr_i < RXS2448_MAX_SC; cntr_i++) {
			in_parms->dev_ctrs->p_ctrs[idx].ctrs[cntr_i] = init_val;
		}
	}

	rc = RIO_SUCCESS;

exit:
	return rc;
}

uint32_t rxs_rio_sc_read_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_read_ctrs_in_t *in_parms,
		rio_sc_read_ctrs_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t srch_i, srch_p, port_num;
	bool found;
	struct DAR_ptl good_ptl;

	out_parms->imp_rc = RIO_SUCCESS;

	if (NULL == in_parms->dev_ctrs) {
		out_parms->imp_rc = SC_READ_RXS_CTRS(0x01);
		goto exit;
	}

	if (NULL == in_parms->dev_ctrs->p_ctrs) {
		out_parms->imp_rc = SC_READ_RXS_CTRS(0x02);
		goto exit;
	}

	if (!in_parms->dev_ctrs->num_p_ctrs
			|| (in_parms->dev_ctrs->num_p_ctrs > RIO_MAX_PORTS)
			|| (in_parms->dev_ctrs->num_p_ctrs
					< in_parms->dev_ctrs->valid_p_ctrs)) {
		out_parms->imp_rc = SC_READ_RXS_CTRS(0x03);
		goto exit;
	}

	if (((RIO_ALL_PORTS == in_parms->ptl.num_ports)
			&& (in_parms->dev_ctrs->num_p_ctrs
					!= NUM_RXS_PORTS(dev_info)))
			|| ((RIO_ALL_PORTS != in_parms->ptl.num_ports)
					&& (in_parms->dev_ctrs->num_p_ctrs
							< in_parms->ptl.num_ports))) {
		out_parms->imp_rc = SC_READ_RXS_CTRS(0x04);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = SC_READ_RXS_CTRS(0x10);
		goto exit;
	}

	for (srch_p = 0; srch_p < good_ptl.num_ports; srch_p++) {
		port_num = good_ptl.pnums[srch_p];
		found = false;
		for (srch_i = 0; srch_i < in_parms->dev_ctrs->valid_p_ctrs;
				srch_i++) {
			if (in_parms->dev_ctrs->p_ctrs[srch_i].pnum
					== port_num) {
				found = true;
				rc = rxs_read_ctrs(dev_info, in_parms,
						out_parms, srch_i, port_num);
				if (RIO_SUCCESS != rc) {
					goto exit;
				}
			}
		}
		if (!found) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = SC_READ_RXS_CTRS(0x90 + srch_p);
			goto exit;
		}
	}
	rc = RIO_SUCCESS;

exit:
	return rc;
}

#endif /* RXS_DAR_WANTED */

#ifdef __cplusplus
}
#endif
