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
#include <string.h>

#include "DAR_DB_Private.h"
#include "DSF_DB_Private.h"
#include "RapidIO_Statistics_Counter_API.h"
#include "CPS1848.h"
#include "CPS1616.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CPS_DAR_WANTED

typedef struct cps_sc_info_t_TAG {
	rio_sc_ctr_val_t sc_info;
	uint32_t reg_base; // Port 0 address for this counter
	uint32_t mask;// PORT_X_OPS setting req'd to enable counter
}cps_sc_info_t;

#define TRACE_0_IDX  0
#define FILTER_0_IDX 4

cps_sc_info_t cps_sc_info[] = {
  {{0, 0, rio_sc_disabled    , DIR_TX, DIR_SRIO}, CPS1848_PORT_X_TRACE_CNTR_0(0)        , CPS1848_PORT_X_OPS_TRACE_0_EN },
  {{0, 0, rio_sc_disabled    , DIR_TX, DIR_SRIO}, CPS1848_PORT_X_TRACE_CNTR_1(0)        , CPS1848_PORT_X_OPS_TRACE_1_EN },
  {{0, 0, rio_sc_disabled    , DIR_TX, DIR_SRIO}, CPS1848_PORT_X_TRACE_CNTR_2(0)        , CPS1848_PORT_X_OPS_TRACE_2_EN },
  {{0, 0, rio_sc_disabled    , DIR_TX, DIR_SRIO}, CPS1848_PORT_X_TRACE_CNTR_3(0)        , CPS1848_PORT_X_OPS_TRACE_3_EN },
  {{0, 0, rio_sc_disabled    , DIR_TX, DIR_SRIO}, CPS1848_PORT_X_FILTER_CNTR_0(0)       , CPS1848_PORT_X_OPS_FILTER_0_EN},
  {{0, 0, rio_sc_disabled    , DIR_TX, DIR_SRIO}, CPS1848_PORT_X_FILTER_CNTR_1(0)       , CPS1848_PORT_X_OPS_FILTER_1_EN},
  {{0, 0, rio_sc_disabled    , DIR_TX, DIR_SRIO}, CPS1848_PORT_X_FILTER_CNTR_2(0)       , CPS1848_PORT_X_OPS_FILTER_2_EN},
  {{0, 0, rio_sc_disabled    , DIR_TX, DIR_SRIO}, CPS1848_PORT_X_FILTER_CNTR_3(0)       , CPS1848_PORT_X_OPS_FILTER_3_EN},
  {{0, 0, rio_sc_pa          , DIR_TX, DIR_SRIO}, CPS1848_PORT_X_VC0_PA_TX_CNTR(0)      , CPS1848_PORT_X_OPS_CNTRS_EN   },
  {{0, 0, rio_sc_pna         , DIR_TX, DIR_SRIO}, CPS1848_PORT_X_NACK_TX_CNTR(0)        , CPS1848_PORT_X_OPS_CNTRS_EN   },
  {{0, 0, rio_sc_retries     , DIR_TX, DIR_SRIO}, CPS1848_PORT_X_VC0_RTRY_TX_CNTR(0)    , CPS1848_PORT_X_OPS_CNTRS_EN   },
  {{0, 0, rio_sc_pkt         , DIR_TX, DIR_SRIO}, CPS1848_PORT_X_VC0_PKT_TX_CNTR(0)     , CPS1848_PORT_X_OPS_CNTRS_EN   },
  {{0, 0, rio_sc_pa          , DIR_RX, DIR_SRIO}, CPS1848_PORT_X_VC0_PA_RX_CNTR(0)      , CPS1848_PORT_X_OPS_CNTRS_EN   },
  {{0, 0, rio_sc_pna         , DIR_RX, DIR_SRIO}, CPS1848_PORT_X_NACK_RX_CNTR(0)        , CPS1848_PORT_X_OPS_CNTRS_EN   },
  {{0, 0, rio_sc_retries     , DIR_RX, DIR_SRIO}, CPS1848_PORT_X_VC0_RTRY_RX_CNTR(0)    , CPS1848_PORT_X_OPS_CNTRS_EN   },
  {{0, 0, rio_sc_pkt         , DIR_RX, DIR_FAB }, CPS1848_PORT_X_VC0_CPB_TX_CNTR(0)     , CPS1848_PORT_X_OPS_CNTRS_EN   },
  {{0, 0, rio_sc_pkt         , DIR_RX, DIR_SRIO}, CPS1848_PORT_X_VC0_PKT_RX_CNTR(0)     , CPS1848_PORT_X_OPS_CNTRS_EN   },
  {{0, 0, rio_sc_pkt_drop    , DIR_RX, DIR_SRIO}, CPS1848_PORT_X_VC0_PKT_DROP_RX_CNTR(0), CPS1848_PORT_X_OPS_CNTRS_EN   },
  {{0, 0, rio_sc_pkt_drop    , DIR_TX, DIR_SRIO}, CPS1848_PORT_X_VC0_PKT_DROP_TX_CNTR(0), CPS1848_PORT_X_OPS_CNTRS_EN   },
  {{0, 0, rio_sc_pkt_drop_ttl, DIR_TX, DIR_SRIO}, CPS1848_PORT_X_VC0_TTL_DROP_CNTR(0)   , CPS1848_PORT_X_OPS_CNTRS_EN   } 
};

#define NUM_CPS_SC (sizeof(cps_sc_info)/sizeof(cps_sc_info_t))
#define CPS_SC_ADDR(p,i) (cps_sc_info[i].reg_base+(0x100*p))

/* Configure counters on selected ports of a
 * CPS device.
 */
uint32_t rio_sc_cfg_cps_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_cfg_cps_ctrs_in_t *in_parms,
		rio_sc_cfg_cps_ctrs_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t ctl_reg, new_ctl_reg = 0, unused;
	uint8_t ctr;
	uint8_t srch_i, srch_p, port_num;
	bool found;
	struct DAR_ptl good_ptl;

	out_parms->imp_rc = RIO_SUCCESS;

	if (NULL == in_parms->dev_ctrs) {
		out_parms->imp_rc = SC_CFG_CPS_CTRS(2);
		goto exit;
	}

	if (NULL == in_parms->dev_ctrs->p_ctrs) {
		out_parms->imp_rc = SC_CFG_CPS_CTRS(4);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = SC_CFG_CPS_CTRS(6);
		goto exit;
	}

	if (!in_parms->dev_ctrs->num_p_ctrs
			|| (in_parms->dev_ctrs->num_p_ctrs > RIO_MAX_PORTS)
			|| !in_parms->dev_ctrs->valid_p_ctrs
			|| (in_parms->dev_ctrs->num_p_ctrs
					< in_parms->dev_ctrs->valid_p_ctrs)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = SC_CFG_CPS_CTRS(0x08);
		goto exit;
	}

	if (in_parms->dev_ctrs->valid_p_ctrs < good_ptl.num_ports) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = SC_CFG_CPS_CTRS(0x0A);
		goto exit;
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

				// Always program the control value...
				rc = DARRegRead(dev_info,
						CPS1848_PORT_X_OPS(port_num),
						&ctl_reg);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = SC_CFG_CPS_CTRS(
							0x40);
					goto exit;
				}

				if (in_parms->enable_ctrs) {
					new_ctl_reg =
							ctl_reg
									| CPS1848_PORT_X_OPS_CNTRS_EN;
				} else {
					new_ctl_reg =
							ctl_reg
									& ~CPS1848_PORT_X_OPS_CNTRS_EN;
				}

				rc = DARRegWrite(dev_info,
						CPS1848_PORT_X_OPS(port_num),
						new_ctl_reg);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = SC_CFG_CPS_CTRS(
							0x41);
					goto exit;
				}

				for (ctr = 0; ctr < NUM_CPS_SC; ctr++) {

					if (new_ctl_reg != ctl_reg) {
						rc =
								DARRegRead(
										dev_info,
										CPS_SC_ADDR(
												port_num,
												ctr),
										&unused);
						if (RIO_SUCCESS != rc) {
							out_parms->imp_rc =
									SC_CFG_CPS_CTRS(
											0x50 + ctr);
							goto exit;
						}
						in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[ctr].total =
								0;
						in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[ctr].last_inc =
								0;
					}
					if (in_parms->enable_ctrs) {
						in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[ctr].sc =
								cps_sc_info[ctr].sc_info.sc;
						in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[ctr].srio =
								cps_sc_info[ctr].sc_info.srio;
						in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[ctr].tx =
								cps_sc_info[ctr].sc_info.tx;
					} else {
						in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[ctr].sc =
								rio_sc_disabled;
						in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[ctr].srio =
								true;
						in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[ctr].tx =
								true;
					}
				}
			}
		}

		if (!found) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = SC_CFG_CPS_CTRS(0x70 + port_num);
			goto exit;
		}
	}

exit:
	return rc;
}

uint32_t CPS_rio_sc_init_dev_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_init_dev_ctrs_in_t *in_parms,
		rio_sc_init_dev_ctrs_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t idx, cntr_i;
	struct DAR_ptl good_ptl;

	out_parms->imp_rc = RIO_SUCCESS;

	if (NULL == in_parms->dev_ctrs) {
		out_parms->imp_rc = SC_INIT_DEV_CTRS(2);
		goto exit;
	}

	if (NULL == in_parms->dev_ctrs->p_ctrs) {
		out_parms->imp_rc = SC_INIT_DEV_CTRS(4);
		goto exit;
	}

	if (!in_parms->dev_ctrs->num_p_ctrs
			|| (in_parms->dev_ctrs->num_p_ctrs > RIO_MAX_PORTS)) {
		out_parms->imp_rc = SC_INIT_DEV_CTRS(6);
		goto exit;
	}
	// Set up pnums_cnt and pnums_l based on in_parms->num_ports and in_parms->pnums.
	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = SC_INIT_DEV_CTRS(8);
		goto exit;
	}

	if ((in_parms->dev_ctrs->num_p_ctrs < good_ptl.num_ports)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = SC_INIT_DEV_CTRS(0x10);
		goto exit;
	}

	in_parms->dev_ctrs->valid_p_ctrs = good_ptl.num_ports;
	for (idx = 0; idx < good_ptl.num_ports; idx++) {
		in_parms->dev_ctrs->p_ctrs[idx].pnum = good_ptl.pnums[idx];
		in_parms->dev_ctrs->p_ctrs[idx].ctrs_cnt = NUM_CPS_SC;
		for (cntr_i = 0; cntr_i < NUM_CPS_SC; cntr_i++) {
			in_parms->dev_ctrs->p_ctrs[idx].ctrs[cntr_i] =
					cps_sc_info[cntr_i].sc_info;
		}
	}
	rc = RIO_SUCCESS;

exit:
	return rc;
}

/* Reads enabled counters on selected ports
 */

uint32_t CPS_rio_sc_read_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_read_ctrs_in_t *in_parms,
		rio_sc_read_ctrs_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t srch_i, srch_p, port_num, cntr;
	bool found;
	uint32_t ctl_reg;
	struct DAR_ptl good_ptl;

	out_parms->imp_rc = RIO_SUCCESS;

	if (NULL == in_parms->dev_ctrs) {
		out_parms->imp_rc = SC_READ_CTRS(1);
		goto exit;
	}

	if (NULL == in_parms->dev_ctrs->p_ctrs) {
		out_parms->imp_rc = SC_READ_CTRS(2);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = SC_READ_CTRS(4);
		goto exit;
	}

	if (!in_parms->dev_ctrs->num_p_ctrs
			|| (in_parms->dev_ctrs->num_p_ctrs > RIO_MAX_PORTS)
			|| !in_parms->dev_ctrs->valid_p_ctrs
			|| (in_parms->dev_ctrs->num_p_ctrs
					< in_parms->dev_ctrs->valid_p_ctrs)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = SC_READ_CTRS(0x05);
		goto exit;
	}

	if (in_parms->dev_ctrs->valid_p_ctrs < good_ptl.num_ports) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = SC_READ_CTRS(0x06);
		goto exit;
	}

	// For generality, must establish a list of ports.
	// Do not assume that the port number equals the index in the structure...

	for (srch_p = 0; srch_p < good_ptl.num_ports; srch_p++) {
		found = false;
		port_num = good_ptl.pnums[srch_p];
		for (srch_i = 0; srch_i < in_parms->dev_ctrs->valid_p_ctrs;
				srch_i++) {
			if (in_parms->dev_ctrs->p_ctrs[srch_i].pnum
					== port_num) {
				found = true;

				// Read the control value to determine if any counters are enabled
				rc = DARRegRead(dev_info,
						CPS1848_PORT_X_OPS(port_num),
						&ctl_reg);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = SC_READ_CTRS(0x40);
					goto exit;
				}

				// Read the port performance counters...
				for (cntr = 0;
						cntr
								< in_parms->dev_ctrs->p_ctrs[srch_i].ctrs_cnt;
						cntr++) {
					if ((rio_sc_disabled
							!= in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[cntr].sc)
							&& (ctl_reg
									& cps_sc_info[cntr].mask)) {
						rc =
								DARRegRead(
										dev_info,
										CPS_SC_ADDR(
												port_num,
												cntr),
										&in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[cntr].last_inc);
						if (RIO_SUCCESS != rc) {
							out_parms->imp_rc =
									SC_READ_CTRS(
											0x70 + cntr);
							goto exit;
						}
						in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[cntr].total +=
								in_parms->dev_ctrs->p_ctrs[srch_i].ctrs[cntr].last_inc;
					}
				}
				break;
			}
		}

		if (!found) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = SC_READ_CTRS(0x90 + port_num);
			goto exit;
		}
	}

exit:
	return rc;
}

#endif /* CPS_DAR_WANTED */

#ifdef __cplusplus
}
#endif
