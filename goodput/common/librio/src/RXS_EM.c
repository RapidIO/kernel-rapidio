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

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "RXS2448.h"
#include "DAR_DB_Private.h"
#include "DSF_DB_Private.h"
#include "RapidIO_Error_Management_API.h"
#include "RapidIO_Device_Access_Routines_API.h"
#include "RXS_DeviceDriver.h"
#include "string_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RXS_DAR_WANTED

#define SAFE_ADD_EVENT_N_LOC(event_in, p) \
	if (out_parms->num_events < in_parms->num_events) { \
		in_parms->events[out_parms->num_events].event = event_in; \
		in_parms->events[out_parms->num_events++].port_num = p; \
	} else { \
		out_parms->too_many = true; \
	}

#define KEEP_W1C_ERR_STAT_MASK (~(RXS_SPX_ERR_STAT_PORT_W_P | \
				RXS_SPX_ERR_STAT_INPUT_ERR_ENCTR | \
				RXS_SPX_ERR_STAT_OUTPUT_ERR_ENCTR | \
				RXS_SPX_ERR_STAT_OUTPUT_RE | \
				RXS_SPX_ERR_STAT_OUTPUT_FAIL | \
				RXS_SPX_ERR_STAT_OUTPUT_DROP))

#define RXS_ALL_LOG_ERRS (RXS_ERR_EN_ILL_TYPE_EN | \
			RXS_ERR_EN_UNS_RSP_EN | \
			RXS_ERR_DET_ILL_ID)

uint32_t rxs_rio_em_cfg_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_pw_in_t *in_parms, rio_em_cfg_pw_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t regData;
	uint32_t pw_mask;
	uint64_t time; // Use uint64_t to avoid rounding errors for large values

	rio_rt_probe_all_in_t pr_all_in;
	rio_rt_probe_all_out_t pr_all_out;
	rio_rt_state_t rt;
	rio_rt_probe_in_t pr_in;
	rio_rt_probe_out_t pr_out;
	pe_rt_val rte;
	rio_port_t port;
	unsigned int idx;

	out_parms->imp_rc = RIO_SUCCESS;

	if (in_parms->priority > 3) {
		out_parms->imp_rc = EM_CFG_PW(1);
		goto exit;
	}

	if ((in_parms->deviceID_tt != tt_dev16)
			&& (in_parms->deviceID_tt != tt_dev8)) {
		out_parms->imp_rc = EM_CFG_PW(2);
		goto exit;
	}

	// Now things get a bit complicated.  Port-writes are sent to
	// ports indicated by a regster separate from the routing table.
	// The register contains a bitmask of ports.  So, we must probe
	// all ports to determine the contents of this bitmask.

	regData = 0;

	for (port = 0; port < NUM_RXS_PORTS(dev_info); port++) {
		pr_all_in.probe_on_port = port;
		pr_all_in.rt = &rt;
		rc = rxs_rio_rt_probe_all(dev_info, &pr_all_in, &pr_all_out);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = pr_all_out.imp_rc;
			goto exit;
		}

		pr_in.probe_on_port = port;
		pr_in.tt = in_parms->deviceID_tt;
		pr_in.destID = in_parms->port_write_destID;
		pr_in.rt = &rt;

		rc = rxs_rio_rt_probe(dev_info, &pr_in, &pr_out);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = pr_out.imp_rc;
			goto exit;
		}

		// Do not check if the route is valid,
		// just use the routing table value.
		rte = pr_out.routing_table_value;
		if (RIO_RTE_DFLT_PORT == rte) {
			rte = pr_out.default_route;
		}

		// Routing value must be either a port or multicast mask.
		if (RIO_RTV_IS_PORT(rte)) {
			regData |= (1 << RIO_RTV_GET_PORT(rte));
			continue;
		}
		// If it's not a multicast mask, something has gone sideways
		if (!RIO_RTV_IS_MC_MSK(rte)) {
			rc = RIO_ERR_SW_FAILURE;
			out_parms->imp_rc = EM_CFG_PW(4);
			goto exit;
		}
		for (idx = 0; idx < NUM_RXS_PORTS(dev_info); idx++) {
			if (pr_out.mcast_ports[idx]) {
				regData |= (1 << idx);
			}
		}
	}

	// Selected destination ID is not routed to any ports, so exit...
	if (!regData) {
		rc = RIO_ERR_BAD_ROUTE_PORT;
		out_parms->imp_rc = EM_CFG_PW(4);
		goto exit;
	}

	rc = DARRegWrite(dev_info, RXS_PW_ROUTE, regData);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(3);
		goto exit;
	}

	// Set all port-write configuration information.
	// For RXS2448, this is limitted to port-write
	// destination ID and retransmission rate.

	// Configure 8 or 16 bit destination ID for port writes.
	// No support for tt_dev32 yet.
	pw_mask = RXS_PW_TGT_ID_PW_TGT_ID;
	if (tt_dev16 == in_parms->deviceID_tt) {
		pw_mask |= RXS_PW_TGT_ID_MSB_PW_ID;
	}
	regData = ((uint32_t)(in_parms->port_write_destID)) << 16;
	regData &= pw_mask;
	if (tt_dev16 == in_parms->deviceID_tt) {
		regData |= RXS_PW_TGT_ID_DEV16;
	}

	rc = DARRegWrite(dev_info, RXS_PW_TGT_ID, regData);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(2);
		goto exit;
	}

	// Configure port-write re-transmission rate.
	// Assumption: it is better to choose a longer retransmission
	// time than the value requested.

	regData = 0;

	if (in_parms->port_write_re_tx) {
		time = (uint64_t)in_parms->port_write_re_tx;
		time *= (uint64_t)PORT_WRITE_RE_TX_NSEC;
		time +=  RXS_PW_CTL_PW_TMR_NSEC - 1;
		time /= RXS_PW_CTL_PW_TMR_NSEC;
		if (!time) {
			time = 1;
		}
		if (time > RXS_PW_CTL_PW_TMR_MAX) {
			time = RXS_PW_CTL_PW_TMR_MAX;
		}
		regData = (time << 8) & RXS_PW_CTL_PW_TMR;
	}

	rc = DARRegWrite(dev_info, RXS_PW_CTL, regData);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(3);
		goto exit;
	}

	// Read back current port write configuration data
	// Note: Only support dev8 and dev16...
	rc = DARRegRead(dev_info, RXS_PW_TGT_ID, &regData);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(4);
		goto exit;
	}

	out_parms->deviceID_tt = (regData & RXS_PW_TGT_ID_DEV16) ?
					tt_dev16 : tt_dev8;
	pw_mask = RXS_PW_TGT_ID_PW_TGT_ID;
	if (tt_dev16 == out_parms->deviceID_tt) {
		pw_mask |= RXS_PW_TGT_ID_MSB_PW_ID;
	}
	out_parms->port_write_destID = (uint16_t)((regData & pw_mask) >> 16);

	// Cannot configure source id, port-write priority or CRF.
	out_parms->srcID_valid = false;
	out_parms->port_write_srcID = 0;
	out_parms->priority = 3;
	out_parms->CRF = true;

	// Figure out retransmission value.
	rc = DARRegRead(dev_info, RXS_PW_CTL, &regData);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_PW(3);
		goto exit;
	}

	time = ((uint64_t)regData & (uint64_t)RXS_PW_CTL_PW_TMR) >> 8;
	time *= RXS_PW_CTL_PW_TMR_NSEC;
	time /= PORT_WRITE_RE_TX_NSEC;
	if (time > UINT32_MAX) {
		time = UINT32_MAX;
	}
	out_parms->port_write_re_tx = time;
exit:
	return rc;
}

// To cleanup DLT, PORT_ERR, 2MANY_PNA, PORT_FAIL, PORT_DIS, PORT_LOCKOUT,
// need to:
// - Lockout the port to drop all outstanding packets and
// - Place port in soft reset
// - Clear the bit(s) that disabled the port
// - Remove port from soft reset
//

typedef struct rxs_event_cfg_reg_vals_t_TAG {
	uint32_t plm_ctl;		// RXS_PLM_SPX_IMP_SPEC_CTL
	uint32_t plm_pw_en;		// RXS_PLM_SPX_PW_EN
	uint32_t pbm_pw_en;		// RXS_PBM_SPX_PW_EN
	uint32_t tlm_pw_en;		// RXS_TLM_SPX_PW_EN
	uint32_t plm_int_en;		// RXS_PLM_SPX_INT_EN
	uint32_t pbm_int_en;		// RXS_PBM_SPX_INT_EN
	uint32_t tlm_int_en;		// RXS_TLM_SPX_INT_EN
	uint32_t plm_denial_ctl;	// RXS_PLM_SPX_DENIAL_CTL
	uint32_t rate_en;		// RXS_SPX_RATE_EN
	uint32_t dlt_ctl;		// RXS_SPX_DLT_CSR
	uint32_t ctl;			// RXS_SPX_CTL
	uint32_t em_rst_pw_en;		// RXS_EM_RST_PW_EN
	uint32_t em_pw_en;		// RXS_EM_PW_EN
	uint32_t log_err_en;		// RXS_ERR_DET
	uint32_t ttl;			// RXS_PKT_TIME_LIVE
	uint32_t i2c_int_enable;	// I2C_INT_STAT
	uint32_t em_rst_int_en;		// RXS_EM_RST_INT_EN
	uint32_t em_int_en;		// RXS_EM_INT_EN
} rxs_event_cfg_reg_vals_t;

static uint32_t rxs_event_read_dev_regs(DAR_DEV_INFO_t *d_i,
		rxs_event_cfg_reg_vals_t *r)
{
	uint32_t rc;

	rc = DARRegRead(d_i, RXS_ERR_EN, &r->log_err_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_PKT_TIME_LIVE, &r->ttl);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_EM_RST_INT_EN, &r->em_rst_int_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_EM_RST_PW_EN, &r->em_rst_pw_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_EM_INT_EN, &r->em_int_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_EM_PW_EN, &r->em_pw_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, I2C_INT_ENABLE, &r->i2c_int_enable);
	if (rc) {
		goto fail;
	}
fail:
	return rc;
}

static uint32_t rxs_event_write_dev_regs(DAR_DEV_INFO_t *d_i,
		rxs_event_cfg_reg_vals_t *r)
{
	uint32_t rc;

	rc = DARRegWrite(d_i, RXS_ERR_EN, r->log_err_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_PKT_TIME_LIVE, r->ttl);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_EM_RST_INT_EN, r->em_rst_int_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_EM_RST_PW_EN, r->em_rst_pw_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_EM_INT_EN, r->em_int_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_EM_PW_EN, r->em_pw_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, I2C_INT_ENABLE, r->i2c_int_enable);
	if (rc) {
		goto fail;
	}
fail:
	return rc;
}

static uint32_t rxs_event_read_port_regs(DAR_DEV_INFO_t *d_i,
		rxs_event_cfg_reg_vals_t *r, rio_port_t port)
{
	uint32_t rc;

	rc = DARRegRead(d_i, RXS_SPX_CTL(port), &r->ctl);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_SPX_RATE_EN(port), &r->rate_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_SPX_DLT_CSR(port), &r->dlt_ctl);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_PLM_SPX_IMP_SPEC_CTL(port), &r->plm_ctl);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_PLM_SPX_PW_EN(port), &r->plm_pw_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_PLM_SPX_INT_EN(port), &r->plm_int_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_PLM_SPX_DENIAL_CTL(port), &r->plm_denial_ctl);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_TLM_SPX_PW_EN(port), &r->tlm_pw_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_TLM_SPX_INT_EN(port), &r->tlm_int_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_PBM_SPX_PW_EN(port), &r->pbm_pw_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegRead(d_i, RXS_PBM_SPX_INT_EN(port), &r->pbm_int_en);
	if (rc) {
		goto fail;
	}
fail:
	return rc;
}

static uint32_t rxs_event_write_port_regs(DAR_DEV_INFO_t *d_i,
		rxs_event_cfg_reg_vals_t *r, rio_port_t port)
{
	uint32_t rc;

	rc = DARRegWrite(d_i, RXS_SPX_CTL(port), r->ctl);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_SPX_RATE_EN(port), r->rate_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_SPX_DLT_CSR(port), r->dlt_ctl);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_PLM_SPX_IMP_SPEC_CTL(port), r->plm_ctl);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_PLM_SPX_PW_EN(port), r->plm_pw_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_PLM_SPX_INT_EN(port), r->plm_int_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_PLM_SPX_DENIAL_CTL(port), r->plm_denial_ctl);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_TLM_SPX_PW_EN(port), r->tlm_pw_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_TLM_SPX_INT_EN(port), r->tlm_int_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_PBM_SPX_PW_EN(port), r->pbm_pw_en);
	if (rc) {
		goto fail;
	}

	rc = DARRegWrite(d_i, RXS_PBM_SPX_INT_EN(port), r->pbm_int_en);
	if (rc) {
		goto fail;
	}
fail:
	return rc;
}

#define RXS_RST_REQ_EVENT_MASK (RXS_PLM_SPX_STAT_RST_REQ | \
				RXS_PLM_SPX_STAT_PRST_REQ)

#define RXS_LOS_EVENT_MASK (RXS_PLM_SPX_STAT_DWNGD | \
				RXS_PLM_SPX_STAT_DLT | \
				RXS_PLM_SPX_STAT_OK_TO_UNINIT)

// Events which are an aggregation/summary of per-port events.
// They do not signify an "other" event.
#define RXS_EM_INT_STAT_AGG_EVENTS_MASK (RXS_EM_INT_STAT_PORT | \
				RXS_EM_INT_STAT_RCS)
#define RXS_EM_PW_STAT_AGG_EVENTS_MASK (RXS_EM_PW_STAT_PORT | \
				RXS_EM_PW_STAT_RCS)

#define RXS_LOS_ERR_DET_EVENT_MASK (RXS_SPX_ERR_DET_DLT | \
				RXS_SPX_ERR_DET_OK_TO_UNINIT)

#define RXS_PBM_FATAL_EVENT_MASK (RXS_PBM_SPX_INT_EN_EG_DNFL_FATAL | \
				RXS_PBM_SPX_INT_EN_EG_DOH_FATAL | \
				RXS_PBM_SPX_INT_EN_EG_DATA_UNCOR)

static uint32_t rxs_event_cfg_get(rio_em_cfg_t *event,
		rxs_event_cfg_reg_vals_t *regs)
{
	uint32_t rc;
	uint32_t mask;
	uint64_t time;

	event->em_detect = rio_em_detect_0delta;
	event->em_info = 0;

	switch (event->em_event) {
	case rio_em_f_los:
		mask = RXS_SPX_ERR_DET_OK_TO_UNINIT | RXS_SPX_ERR_DET_DLT;
		event->em_info = 0;
		if (!(regs->rate_en & mask)) {
			event->em_detect = rio_em_detect_off;
			break;
		}
		event->em_detect = rio_em_detect_on;
		if (regs->rate_en & RXS_SPX_ERR_DET_DLT) {
			time = regs->dlt_ctl >> 8;
			time *= RXS_SPX_DLT_CSR_TIMEOUT_NSEC;
			if (time > UINT32_MAX) {
				time = UINT32_MAX;
			}
			event->em_info = time;
		}
		break;

	case rio_em_f_port_err:
		if ((regs->plm_int_en & RXS_PLM_SPX_INT_EN_PORT_ERR)
			| (regs->plm_pw_en & RXS_PLM_SPX_PW_EN_PORT_ERR)) {
			event->em_detect = rio_em_detect_on;
		} else {
			event->em_detect = rio_em_detect_off;
		}
		break;

	case rio_em_f_2many_retx:
		if (regs->plm_denial_ctl & RXS_PLM_SPX_DENIAL_CTL_CNT_RTY) {
			event->em_info = regs->plm_denial_ctl &
					RXS_PLM_SPX_DENIAL_CTL_DENIAL_THRESH;
			event->em_detect = rio_em_detect_on;
		} else {
			event->em_info = 0;
			event->em_detect = rio_em_detect_off;
		}
		break;

	case rio_em_f_2many_pna:
		if (regs->plm_denial_ctl & RXS_PLM_SPX_DENIAL_CTL_CNT_PNA) {
			event->em_info = regs->plm_denial_ctl &
					RXS_PLM_SPX_DENIAL_CTL_DENIAL_THRESH;
			event->em_detect = rio_em_detect_on;
		} else {
			event->em_info = 0;
			event->em_detect = rio_em_detect_off;
		}
		break;

	case rio_em_f_err_rate:
		if ((regs->plm_int_en & RXS_PLM_SPX_INT_EN_PBM_FATAL)
			| (regs->plm_pw_en & RXS_PLM_SPX_PW_EN_PBM_FATAL)) {
			event->em_detect = rio_em_detect_on;
		} else {
			event->em_detect = rio_em_detect_off;
		}
		break;

	case rio_em_d_ttl:
		if (regs->ttl) {
			event->em_detect = rio_em_detect_on;
			event->em_info = regs->ttl;
			event->em_info &= RXS_PKT_TIME_LIVE_PKT_TIME_LIVE;
			event->em_info = event->em_info >> 18;
			event->em_info++;
			event->em_info *= RXS_PKT_TIME_LIVE_NSEC;
		} else {
			event->em_detect = rio_em_detect_off;
			event->em_info = 0;
		}
		break;

	case rio_em_d_rte:
		if ((regs->tlm_int_en & RXS_TLM_SPX_INT_EN_LUT_DISCARD) |
			(regs->tlm_pw_en & RXS_TLM_SPX_INT_EN_LUT_DISCARD)) {
			event->em_detect = rio_em_detect_on;
		} else {
			event->em_detect = rio_em_detect_off;
		}
		event->em_info = 0;
		break;

	case rio_em_d_log:
		if (regs->log_err_en & RXS_ALL_LOG_ERRS) {
			event->em_detect = rio_em_detect_on;
			event->em_info = regs->log_err_en & RXS_ALL_LOG_ERRS;
		} else {
			event->em_detect = rio_em_detect_off;
			event->em_info = 0;
		}
		break;

	case rio_em_i_sig_det:
		if (regs->rate_en & RXS_SPX_ERR_DET_LINK_INIT) {
			event->em_detect = rio_em_detect_on;
		} else {
			event->em_detect = rio_em_detect_off;
		}
		break;

	case rio_em_i_rst_req:
		if ((regs->plm_ctl & RXS_PLM_SPX_IMP_SPEC_CTL_SELF_RST)
			&& (regs->plm_ctl
				& RXS_PLM_SPX_IMP_SPEC_CTL_PORT_SELF_RST)) {
			event->em_detect = rio_em_detect_off;
		} else {
			event->em_detect = rio_em_detect_on;
		}
		break;

	case rio_em_i_init_fail:
		if (regs->i2c_int_enable & I2C_INT_ENABLE_BL_FAIL) {
			event->em_detect = rio_em_detect_on;
		} else {
			event->em_detect = rio_em_detect_off;
		}
		break;

	case rio_em_a_clr_pwpnd:
	case rio_em_a_no_event:
		event->em_detect = rio_em_detect_off;
		event->em_info = 0;
		break;

	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		goto fail;
	}
	rc = RIO_SUCCESS;

fail:
	return rc;
}

uint32_t rxs_rio_em_cfg_get(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_get_in_t *in_parms, rio_em_cfg_get_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t idx;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	rxs_event_cfg_reg_vals_t regs;
	rio_em_dev_rpt_ctl_in_t rpt_ctl_in;
	rio_em_dev_rpt_ctl_out_t rpt_ctl_out;

	// Set output info
	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->fail_idx = rio_em_last;
	out_parms->notfn = rio_em_notfn_0delta;

	// Parameter checks.
	if ((NULL == in_parms->events) || !in_parms->num_events
			|| (NULL == in_parms->event_list)) {
		out_parms->imp_rc = EM_CFG_SET(0x1);
		goto fail;
	}

	// Get current port configuration.
	cfg_in.ptl.num_ports = 1;
	cfg_in.ptl.pnums[0] = in_parms->port_num;
	rc = rxs_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto fail;
	}

	// Nothing to set if the port is unavailable or powered down.
	if (!cfg_out.pc[0].port_available) {
		goto fail;
	}
	if (!cfg_out.pc[0].powered_up) {
		goto fail;
	}

	rc = rxs_event_read_dev_regs(dev_info, &regs);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_GET(0x8);
		goto fail;
	}

	rc = rxs_event_read_port_regs(dev_info, &regs, in_parms->port_num);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_GET(0x9);
		goto fail;
	}

	for (idx = 0; idx < in_parms->num_events; idx++) {
		in_parms->events[idx].em_event = in_parms->event_list[idx];
		rc = rxs_event_cfg_get(&in_parms->events[idx], &regs);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_CFG_GET(0x10);
			out_parms->fail_idx = idx;
			goto fail;
		}
	}

	rpt_ctl_in.ptl.num_ports = 1;
	rpt_ctl_in.ptl.pnums[0] = in_parms->port_num;
	rpt_ctl_in.notfn = rio_em_notfn_0delta;
	rc = rxs_rio_em_dev_rpt_ctl(dev_info, &rpt_ctl_in, &rpt_ctl_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = rpt_ctl_out.imp_rc;
		goto fail;
	}
	out_parms->notfn = rpt_ctl_out.notfn;
	rc = RIO_SUCCESS;
fail:
	return rc;
}

static uint32_t rxs_plm_set_notifn(rxs_event_cfg_reg_vals_t *regs,
		uint32_t mask, rio_em_notfn_ctl_t notfn)
{
	uint32_t rc = RIO_SUCCESS;

	switch (notfn) {
	case rio_em_notfn_none:
		regs->plm_int_en &= ~mask;
		regs->plm_pw_en &= ~mask;
		break;
	case rio_em_notfn_int:
		regs->plm_int_en |= mask;
		regs->plm_pw_en &= ~mask;
		break;
	case rio_em_notfn_pw:
		regs->plm_int_en &= ~mask;
		regs->plm_pw_en |= mask;
		break;
	case rio_em_notfn_both:
		regs->plm_int_en |= mask;
		regs->plm_pw_en |= mask;
		break;
	case rio_em_notfn_0delta:
		break;
	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		break;
	}
	return rc;
}

// Two RXS events are controlled by this routine:
// OK_TO_UNINIT: Fires immediately when a port transitions from OK to
//               uninitialized.  This is appropriate when an application
//               cannot tolerate any backpressure for flows going to a port.
// DLT : The "Dead Link Timer" event allows for the possibility for a link to
//               reinitialize before initiating packet discard.  This is
//               appropriate when an application can tolerate hundreds of
//               microseconds to milliseconds of delay, rather than accept
//               data loss.
// DWNGD : If the port reinitializes and downgrades, another event is sent.
//		This is only likely if the DLT is set to a "long" period,
//		where "long" is milliseconds for IDLE1/2 and seconds for IDLE3.
// All events initiate packet discard.
static uint32_t rxs_set_event_cfg_rio_em_f_los(rxs_event_cfg_reg_vals_t *regs,
		rio_em_cfg_t *event, rio_em_notfn_ctl_t nfn, uint32_t *imp_rc)
{
	uint32_t rc;
	uint32_t err_mask = RXS_SPX_RATE_EN_DLT | RXS_SPX_RATE_EN_OK_TO_UNINIT;
	uint32_t plm_ctl_mask = RXS_PLM_SPX_IMP_SPEC_CTL_OK2U_FATAL |
				RXS_PLM_SPX_IMP_SPEC_CTL_DLT_FATAL |
				RXS_PLM_SPX_IMP_SPEC_CTL_DWNGD_FATAL;
	uint32_t plm_ntfn_mask = RXS_PLM_SPX_STAT_OK_TO_UNINIT |
				RXS_PLM_SPX_STAT_DLT |
				RXS_PLM_SPX_STAT_DWNGD;
	uint64_t time;

	regs->rate_en &= ~err_mask;
	regs->plm_ctl &= ~plm_ctl_mask;
	regs->dlt_ctl = 0;
	if (rio_em_detect_on == event->em_detect) {

		time = event->em_info;
		time += RXS_SPX_DLT_CSR_TIMEOUT_NSEC - 1;
		time /= RXS_SPX_DLT_CSR_TIMEOUT_NSEC;
		if (!time) {
			time = 1;
		}
		if (time > RXS_SPX_DLT_CSR_TIMEOUT_MAX) {
			time = RXS_SPX_DLT_CSR_TIMEOUT_MAX;
		}

		regs->plm_ctl |= plm_ctl_mask;
		regs->ctl |= RXS_SPX_CTL_STOP_FAIL_EN | RXS_SPX_CTL_DROP_EN;

		// For non-0 time, use the DLT event
		// For 0 time, use the OK_TO_UNINIT event
		if (event->em_info) {
			regs->rate_en |= RXS_SPX_RATE_EN_DLT;
			regs->rate_en &= ~RXS_SPX_RATE_EN_OK_TO_UNINIT;
			regs->dlt_ctl = (time << 8) & RXS_SPX_DLT_CSR_TIMEOUT;
		} else {
			regs->rate_en |= RXS_SPX_RATE_EN_OK_TO_UNINIT;
			regs->rate_en &= ~RXS_SPX_RATE_EN_DLT;
			regs->dlt_ctl = 0;
		}
	}

	rc = rxs_plm_set_notifn(regs, plm_ntfn_mask, nfn);
	if (RIO_SUCCESS != rc) {
		*imp_rc = EM_CFG_SET(0x2A);
	}
	return rc;
}

// This routine controls two events:
// Too many retries: Continuous retries of a packet.
// Too many PNAs   : Continuous "Packet Not Accepted" responses for a packet.
// Both events use the same threshold register field.
// Both cause the same physical event (MAX DENIAL)
// The "Too many retries" should not be enabled without a detailed
//  understanding of traffic flows in the system, to ensure that
// "too many retries" means something really has gone wrong.
//
// Typically, too many PNA means that the link partner has set PORT_LOCKOUT
// or has not set Input Port Enable.  Too many PNA should be set as part of
// the normal operating mode of most devices.

static uint32_t rxs_set_event_cfg_rio_em_f_2many_retx_or_pna(
		rxs_event_cfg_reg_vals_t *regs, rio_em_cfg_t *event,
		rio_em_notfn_ctl_t nfn, uint32_t *imp_rc)
{
	uint32_t rc;
	uint32_t cnt_mask;
	uint32_t cnt_both_mask = RXS_PLM_SPX_DENIAL_CTL_CNT_RTY |
				RXS_PLM_SPX_DENIAL_CTL_CNT_PNA;

	if (rio_em_f_2many_retx == event->em_event) {
		cnt_mask = RXS_PLM_SPX_DENIAL_CTL_CNT_RTY;
	} else {
		cnt_mask = RXS_PLM_SPX_DENIAL_CTL_CNT_PNA;
	}

	if (rio_em_detect_on == event->em_detect) {
		regs->plm_denial_ctl |= cnt_mask;
		regs->plm_ctl |= RXS_PLM_SPX_IMP_SPEC_CTL_MAXD_FATAL;

		if (!event->em_info) {
			rc = RIO_ERR_INVALID_PARAMETER;
			*imp_rc = EM_CFG_SET(0x38);
			goto fail;
		}

		if (event->em_info >= RXS_PLM_SPX_DENIAL_CTL_DENIAL_THRESH) {
			regs->plm_denial_ctl |=
					RXS_PLM_SPX_DENIAL_CTL_DENIAL_THRESH;
		} else {
			regs->plm_denial_ctl &=
					~RXS_PLM_SPX_DENIAL_CTL_DENIAL_THRESH;
			regs->plm_denial_ctl |= event->em_info;
		}
	} else {
		regs->plm_denial_ctl &= ~cnt_mask;
		if (!(regs->plm_denial_ctl & cnt_both_mask)) {
			regs->plm_denial_ctl &=
					~RXS_PLM_SPX_DENIAL_CTL_DENIAL_THRESH;
			regs->plm_ctl &=
					~RXS_PLM_SPX_IMP_SPEC_CTL_MAXD_FATAL;
		}
	}

	rc = rxs_plm_set_notifn(regs, RXS_PLM_SPX_STAT_MAX_DENIAL, nfn);
	if (RIO_SUCCESS != rc) {
		*imp_rc = EM_CFG_SET(0x39);
	}
fail:
	return rc;
}

static uint32_t rxs_set_event_cfg_rio_em_f_err_rate(
		rxs_event_cfg_reg_vals_t *regs, rio_em_cfg_t *event,
		rio_em_notfn_ctl_t nfn, uint32_t *imp_rc)
{
	uint32_t rc;

	switch (event->em_detect) {
	case rio_em_detect_on:
		rc = rxs_plm_set_notifn(regs, RXS_PLM_SPX_STAT_PBM_FATAL, nfn);
		if (RIO_SUCCESS != rc) {
			*imp_rc = EM_CFG_SET(0x39);
		}
		break;
	case rio_em_detect_off:
		rc = rxs_plm_set_notifn(regs, RXS_PLM_SPX_STAT_PBM_FATAL,
							rio_em_notfn_none);
		if (RIO_SUCCESS != rc) {
			*imp_rc = EM_CFG_SET(0x39);
		}
		break;
	case rio_em_detect_0delta:
		rc = RIO_SUCCESS;
		break;
	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		*imp_rc = EM_CFG_SET(0x44);
	}
	return rc;
}

static uint32_t rxs_ttl_err_set_notifn(rxs_event_cfg_reg_vals_t *regs,
		rio_em_notfn_ctl_t notfn)
{
	uint32_t rc = RIO_SUCCESS;

	switch (notfn) {
	case rio_em_notfn_none:
		regs->pbm_int_en &= ~RXS_PBM_SPX_INT_EN_EG_TTL_EXPIRED;
		regs->pbm_pw_en &= ~RXS_PBM_SPX_PW_EN_EG_TTL_EXPIRED;
		break;
	case rio_em_notfn_int:
		regs->pbm_int_en |= RXS_PBM_SPX_INT_EN_EG_TTL_EXPIRED;
		regs->pbm_pw_en &= ~RXS_PBM_SPX_PW_EN_EG_TTL_EXPIRED;
		break;
	case rio_em_notfn_pw:
		regs->pbm_int_en &= ~RXS_PBM_SPX_INT_EN_EG_TTL_EXPIRED;
		regs->pbm_pw_en |= RXS_PBM_SPX_PW_EN_EG_TTL_EXPIRED;
		break;
	case rio_em_notfn_both:
		regs->pbm_int_en |= RXS_PBM_SPX_INT_EN_EG_TTL_EXPIRED;
		regs->pbm_pw_en |= RXS_PBM_SPX_PW_EN_EG_TTL_EXPIRED;
		break;
	case rio_em_notfn_0delta:
		break;
	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		goto fail;
	}
fail:
	return rc;
}

static uint32_t rxs_set_event_cfg_rio_em_d_ttl_port(
		rxs_event_cfg_reg_vals_t *regs, rio_em_cfg_t *event,
		rio_em_notfn_ctl_t nfn, uint32_t *imp_rc)
{
	uint32_t rc;

	switch (event->em_detect) {
	case rio_em_detect_on:
		rc = rxs_ttl_err_set_notifn(regs, nfn);
		break;
	case rio_em_detect_off:
		rc = rxs_ttl_err_set_notifn(regs, rio_em_notfn_none);
		break;
	case rio_em_detect_0delta:
		rc = RIO_SUCCESS;
		break;
	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		*imp_rc = EM_CFG_SET(0x44);
	}
	return rc;
}

static uint32_t rxs_set_event_cfg_rio_em_d_ttl_dev(
		rxs_event_cfg_reg_vals_t *regs, rio_em_cfg_t *event,
		uint32_t *imp_rc)
{
	uint32_t mask = RXS_PKT_TIME_LIVE_PKT_TIME_LIVE;
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint64_t time;

	switch (event->em_detect) {
	case rio_em_detect_on:
		if (!event->em_info) {
			*imp_rc = EM_CFG_SET(0x43);
			goto fail;
		}
		time = event->em_info + RXS_PKT_TIME_LIVE_NSEC - 1;
		time /= RXS_PKT_TIME_LIVE_NSEC;
		time--;
		time *= 4;
		if (!time) {
			time = 1;
		}
		if (time > RXS_PKT_TIME_LIVE_MAX) {
			time = RXS_PKT_TIME_LIVE_MAX;
		}
		regs->ttl = (time << 16) & mask;
		break;
	case rio_em_detect_off:
		regs->ttl &= ~mask;
		break;
	case rio_em_detect_0delta:
		break;
	default:
		*imp_rc = EM_CFG_SET(0x44);
		goto fail;
	}
	rc = RIO_SUCCESS;
fail:
	return rc;
}

static uint32_t rxs_rte_err_set_notifn(rxs_event_cfg_reg_vals_t *regs,
		rio_em_notfn_ctl_t notfn)
{
	uint32_t rc = RIO_SUCCESS;

	switch (notfn) {
	case rio_em_notfn_none:
		regs->tlm_int_en &= ~RXS_TLM_SPX_INT_EN_LUT_DISCARD;
		regs->tlm_pw_en &= ~RXS_TLM_SPX_PW_EN_LUT_DISCARD;
		break;
	case rio_em_notfn_int:
		regs->tlm_int_en |= RXS_TLM_SPX_INT_EN_LUT_DISCARD;
		regs->tlm_pw_en &= ~RXS_TLM_SPX_PW_EN_LUT_DISCARD;
		break;
	case rio_em_notfn_pw:
		regs->tlm_int_en &= ~RXS_TLM_SPX_INT_EN_LUT_DISCARD;
		regs->tlm_pw_en |= RXS_TLM_SPX_PW_EN_LUT_DISCARD;
		break;
	case rio_em_notfn_both:
		regs->tlm_int_en |= RXS_TLM_SPX_INT_EN_LUT_DISCARD;
		regs->tlm_pw_en |= RXS_TLM_SPX_PW_EN_LUT_DISCARD;
		break;
	case rio_em_notfn_0delta:
		break;
	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		break;
	}
	return rc;
}

static uint32_t rxs_set_event_cfg_rio_em_d_rte(rxs_event_cfg_reg_vals_t *regs,
		rio_em_cfg_t *event, rio_em_notfn_ctl_t nfn, uint32_t *imp_rc)
{
	uint32_t rc;

	switch (event->em_detect) {
	case rio_em_detect_on:
		rc = rxs_rte_err_set_notifn(regs, nfn);
		break;
	case rio_em_detect_off:
		rc = rxs_rte_err_set_notifn(regs, rio_em_notfn_none);
		break;
	case rio_em_detect_0delta:
		rc = RIO_SUCCESS;
		break;
	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		*imp_rc = EM_CFG_SET(0x44);
	}
	return rc;
}

static uint32_t rxs_log_err_set_notifn(rxs_event_cfg_reg_vals_t *regs,
		rio_em_notfn_ctl_t notfn)
{
	uint32_t rc = RIO_SUCCESS;

	switch (notfn) {
	case rio_em_notfn_none:
		regs->em_int_en &= ~RXS_EM_INT_EN_LOG;
		regs->em_pw_en &= ~RXS_EM_PW_EN_LOG;
		break;
	case rio_em_notfn_int:
		regs->em_int_en |= RXS_EM_INT_EN_LOG;
		regs->em_pw_en &= ~RXS_EM_PW_EN_LOG;
		break;
	case rio_em_notfn_pw:
		regs->em_int_en &= ~RXS_EM_INT_EN_LOG;
		regs->em_pw_en |= RXS_EM_PW_EN_LOG;
		break;
	case rio_em_notfn_both:
		regs->em_int_en |= RXS_EM_INT_EN_LOG;
		regs->em_pw_en |= RXS_EM_PW_EN_LOG;
		break;
	case rio_em_notfn_0delta:
		break;
	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		break;
	}
	return rc;
}

// RXS logical layer errors are always detected, using the
// standard error managment logical layer error information.

static uint32_t rxs_set_event_cfg_rio_em_d_log(rxs_event_cfg_reg_vals_t *regs,
		rio_em_cfg_t *event, rio_em_notfn_ctl_t nfn, uint32_t *imp_rc)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;

	switch (event->em_detect) {
	case rio_em_detect_on:
		regs->log_err_en &= ~RXS_ALL_LOG_ERRS;
		regs->log_err_en |= event->em_info & RXS_ALL_LOG_ERRS;
		if (!regs->log_err_en) {
			*imp_rc = EM_CFG_SET(0x43);
			goto fail;
		}
		rc = rxs_log_err_set_notifn(regs, nfn);
		break;
	case rio_em_detect_off:
		regs->log_err_en = 0;
		rc = rxs_log_err_set_notifn(regs, rio_em_notfn_none);
		break;
	case rio_em_detect_0delta:
		rc = RIO_SUCCESS;
		break;
	default:
		*imp_rc = EM_CFG_SET(0x44);
	}
fail:
	return rc;
}

static uint32_t rxs_set_event_cfg_rio_em_i_sig_det(
		rxs_event_cfg_reg_vals_t *regs, rio_em_cfg_t *event,
		rio_em_notfn_ctl_t nfn, uint32_t *imp_rc)
{
	uint32_t rc;

	switch (event->em_detect) {
	case rio_em_detect_on:
		regs->rate_en |= RXS_SPX_RATE_EN_LINK_INIT;
		break;
	case rio_em_detect_off:
		regs->rate_en &= ~RXS_SPX_RATE_EN_LINK_INIT;
		break;
	case rio_em_detect_0delta:
		break;
	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		*imp_rc = EM_CFG_SET(0x48);
		goto fail;
	}

	rc = rxs_plm_set_notifn(regs, RXS_PLM_SPX_STAT_LINK_INIT, nfn);
	if (RIO_SUCCESS != rc) {
		*imp_rc = EM_CFG_SET(0x49);
	}

fail:
	return rc;
}

// Note: Control of PLM detection and notification for
// this event is supported by the
// rio_pc_dev_reset_config/rxs_rio_pc_dev_reset_config
// routine.
//
// This routine is responsible for device notification control
// for the reset event.

static uint32_t rxs_set_event_cfg_rio_em_i_rst_req(
		rxs_event_cfg_reg_vals_t *regs, rio_em_notfn_ctl_t nfn,
		rio_port_t port, uint32_t *imp_rc)
{
	uint32_t rc;
	uint32_t mask = 1 << port;

	switch (nfn) {
	case rio_em_notfn_none:
		regs->em_rst_int_en &= ~mask;
		regs->em_rst_pw_en &= ~mask;
		break;

	case rio_em_notfn_int:
		regs->em_rst_int_en |= mask;
		regs->em_rst_pw_en &= ~mask;
		break;

	case rio_em_notfn_pw:
		regs->em_rst_int_en &= ~mask;
		regs->em_rst_pw_en |= mask;
		break;

	case rio_em_notfn_both:
		regs->em_rst_int_en |= mask;
		regs->em_rst_pw_en |= mask;
		break;

	case rio_em_notfn_0delta:
		break;

	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		*imp_rc = EM_CFG_SET(0x50);
		goto fail;
	}
	rc = RIO_SUCCESS;

fail:
	return rc;
}

static uint32_t rxs_set_event_cfg_rio_em_i_init_fail(rio_em_cfg_t *event,
		rxs_event_cfg_reg_vals_t *regs, rio_em_notfn_ctl_t nfn)
{
	uint32_t rc = RIO_SUCCESS;

	switch (event->em_detect) {
	case rio_em_detect_on:
		regs->i2c_int_enable |= I2C_INT_ENABLE_BL_FAIL;
		break;
	case rio_em_detect_off:
		regs->i2c_int_enable &= ~I2C_INT_ENABLE_BL_FAIL;
		nfn = rio_em_notfn_none;
		break;
	case rio_em_detect_0delta:
		goto exit;
		break;
	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		goto exit;
	}

	// Event is enabled, set notification
	switch (nfn) {
	case rio_em_notfn_none:
		regs->em_int_en &= ~RXS_EM_INT_EN_EXTERNAL_I2C;
		regs->em_pw_en &= ~RXS_EM_PW_EN_EXTERNAL_I2C;
		break;

	case rio_em_notfn_int:
		regs->em_int_en |= RXS_EM_INT_EN_EXTERNAL_I2C;
		regs->em_pw_en &= ~RXS_EM_PW_EN_EXTERNAL_I2C;
		break;

	case rio_em_notfn_pw:
		regs->em_int_en &= ~RXS_EM_INT_EN_EXTERNAL_I2C;
		regs->em_pw_en |= RXS_EM_PW_EN_EXTERNAL_I2C;
		break;

	case rio_em_notfn_both:
		regs->em_int_en |= RXS_EM_INT_EN_EXTERNAL_I2C;
		regs->em_pw_en |= RXS_EM_PW_EN_EXTERNAL_I2C;
		break;

	default:
		rc = RIO_ERR_INVALID_PARAMETER;
	}

exit:
	return rc;
}

static uint32_t rxs_set_event_cfg(rxs_event_cfg_reg_vals_t *regs,
		rio_em_cfg_t *event, rio_em_notfn_ctl_t nfn, rio_port_t port,
		uint32_t *imp_rc)
{
	uint32_t rc;

	if ((event->em_detect >= rio_em_detect_last)
			|| (event->em_event >= rio_em_last)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		*imp_rc = EM_CFG_SET(0x20);
		goto fail;
	}

	if (rio_em_detect_0delta == event->em_detect) {
		rc = RIO_SUCCESS;
		goto fail;
	}

	switch (event->em_event) {
	case rio_em_f_los:
		rc = rxs_set_event_cfg_rio_em_f_los(regs, event, nfn, imp_rc);
		if (RIO_SUCCESS != rc) {
			goto fail;
		}
		break;

	case rio_em_f_port_err:
		rc = rxs_plm_set_notifn(regs, RXS_PLM_SPX_STAT_PORT_ERR, nfn);
		if (RIO_SUCCESS != rc) {
			*imp_rc = EM_CFG_SET(0x30);
			goto fail;
		}
		break;

	case rio_em_f_2many_retx:
	case rio_em_f_2many_pna:
		rc = rxs_set_event_cfg_rio_em_f_2many_retx_or_pna(
				regs, event, nfn, imp_rc);
		if (RIO_SUCCESS != rc) {
			goto fail;
		}
		break;

	case rio_em_f_err_rate:
		rc = rxs_set_event_cfg_rio_em_f_err_rate(
				regs, event, nfn, imp_rc);
		if (RIO_SUCCESS != rc) {
			goto fail;
		}
		break;

	case rio_em_d_ttl:
		rc = rxs_set_event_cfg_rio_em_d_ttl_port(
				regs, event, nfn, imp_rc);
		if (RIO_SUCCESS != rc) {
			goto fail;
		}
		break;
	case rio_em_d_rte:
		rc = rxs_set_event_cfg_rio_em_d_rte(
				regs, event, nfn, imp_rc);
		if (RIO_SUCCESS != rc) {
			goto fail;
		}
		break;

	// Nothing to do..
	case rio_em_a_clr_pwpnd:
	case rio_em_a_no_event:
	// Nothing to do, set this as a device event once
	case rio_em_d_log:
	case rio_em_i_init_fail:
		break;

	case rio_em_i_sig_det:
		rc = rxs_set_event_cfg_rio_em_i_sig_det(
					regs, event, nfn, imp_rc);
		if (RIO_SUCCESS != rc) {
			goto fail;
		}
		break;

	case rio_em_i_rst_req:
		rc = rxs_set_event_cfg_rio_em_i_rst_req(
					regs, nfn, port, imp_rc);
		if (RIO_SUCCESS != rc) {
			goto fail;
		}
		break;

	default:
		rc = RIO_ERR_INVALID_PARAMETER;
		*imp_rc = EM_CFG_SET(0x88);
		goto fail;
	}
	rc = RIO_SUCCESS;

fail:
	return rc;
}

#define NO_EVENT_IDX 0xFFFFFFFF

uint32_t rxs_rio_em_cfg_set(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_set_in_t *in_parms, rio_em_cfg_set_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t idx;
	struct DAR_ptl good_ptl;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	rxs_event_cfg_reg_vals_t regs;
	rio_em_dev_rpt_ctl_in_t rpt_ctl_in;
	rio_em_dev_rpt_ctl_out_t rpt_ctl_out;
	rio_port_t port;
	uint32_t port_idx;
	uint32_t ttl_idx = NO_EVENT_IDX;
	uint32_t log_idx = NO_EVENT_IDX;
	uint32_t init_fail_idx = NO_EVENT_IDX;

	// Set output info
	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->fail_port_num = RIO_ALL_PORTS;
	out_parms->fail_idx = rio_em_last;
	out_parms->notfn = rio_em_notfn_0delta;

	// Parameter checks.
	if ((in_parms->num_events > (uint8_t)(rio_em_last))
			|| (NULL == in_parms->events)
			|| !in_parms->num_events
			|| (rio_em_notfn_last <= in_parms->notfn)) {
		out_parms->imp_rc = EM_CFG_SET(0x1);
		goto fail;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_SET(0x2);
		goto fail;
	}

	// Get current port configuration for all ports.
	cfg_in.ptl = good_ptl;
	rc = rxs_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto fail;
	}

	// Get registers with a single instance per RXS
	rc = rxs_event_read_dev_regs(dev_info, &regs);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_SET(0x8);
		goto fail;
	}
	// First, perform all event disables & validate events
	for (port_idx = 0; port_idx < good_ptl.num_ports; port_idx++) {
		port = good_ptl.pnums[port_idx];
		// Nothing to set if the port is unavailable or powered down.
		if (!cfg_out.pc[0].port_available || !cfg_out.pc[0].powered_up)
		{
			continue;
		}

		// Get registers for this particular port.
		rc = rxs_event_read_port_regs(dev_info, &regs, port);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_CFG_SET(0x8);
			goto fail;
		}
		for (idx = 0; idx < in_parms->num_events; idx++) {
			// Check each event to see if one of the device
			// events is present.  Saves one loop to reduce
			// complexity.
			rio_em_cfg_t *e = &in_parms->events[idx];
			if (rio_em_d_ttl == e->em_event) {
				ttl_idx = idx;
			}
			if (rio_em_d_log == e->em_event) {
				log_idx = idx;
			}
			if (rio_em_i_init_fail == e->em_event) {
				init_fail_idx = idx;
			}
			if (rio_em_detect_on != e->em_detect) {
				rc = rxs_set_event_cfg(
							&regs,
							e,
							in_parms->notfn,
							port,
							&out_parms->imp_rc);
				if (RIO_SUCCESS != rc) {
					out_parms->fail_idx = idx;
					out_parms->fail_port_num = port;
					goto fail;
				}
			}
		}

		// Next, perform all event enables.
		for (idx = 0; idx < in_parms->num_events; idx++) {
			if (rio_em_detect_on == in_parms->events[idx].em_detect) {
				rc = rxs_set_event_cfg(
						&regs,
						&in_parms->events[idx],
						in_parms->notfn,
						port,
						&out_parms->imp_rc);
				if (RIO_SUCCESS != rc) {
					out_parms->fail_idx = idx;
					out_parms->fail_port_num = port;
					goto fail;
				}
			}
		}
		rc = rxs_event_write_port_regs(dev_info, &regs, port);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_CFG_SET(0x8);
			goto fail;
		}
	}

	if (NO_EVENT_IDX != ttl_idx) {
		rc = rxs_set_event_cfg_rio_em_d_ttl_dev(&regs,
					&in_parms->events[ttl_idx],
					&out_parms->imp_rc);
		if (RIO_SUCCESS != rc) {
			out_parms->fail_idx = ttl_idx;
			goto fail;
		}
	}

	if (NO_EVENT_IDX != log_idx) {
		rc = rxs_set_event_cfg_rio_em_d_log(&regs,
					&in_parms->events[log_idx],
					in_parms->notfn,
					&out_parms->imp_rc);
		if (RIO_SUCCESS != rc) {
			out_parms->fail_idx = log_idx;
			goto fail;
		}
	}

	if (NO_EVENT_IDX != init_fail_idx) {
		rc = rxs_set_event_cfg_rio_em_i_init_fail(
					&in_parms->events[init_fail_idx],
					&regs,
					in_parms->notfn);
		if (RIO_SUCCESS != rc) {
			out_parms->fail_idx = init_fail_idx;
			goto fail;
		}
	}

	rc = rxs_event_write_dev_regs(dev_info, &regs);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CFG_SET(0x8);
		goto fail;
	}

	memcpy(&rpt_ctl_in.ptl, &good_ptl, sizeof(rpt_ctl_in.ptl));
	rpt_ctl_in.notfn = in_parms->notfn;
	rc = rxs_rio_em_dev_rpt_ctl(dev_info, &rpt_ctl_in, &rpt_ctl_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = rpt_ctl_out.imp_rc;
		goto fail;
	}
	out_parms->notfn = rpt_ctl_out.notfn;

fail:
	return rc;
}
typedef struct rxs_rpt_ctl_regs_t_TAG {
	uint32_t pw_trans_dis;
	uint32_t dev_int_en;
} rxs_rpt_ctl_regs_t;

static uint32_t rxs_rio_em_dev_rpt_ctl_reg_read(DAR_DEV_INFO_t *dev_info,
		rxs_rpt_ctl_regs_t *regs)
{
	uint32_t rc;

	rc = DARRegRead(dev_info, RXS_EM_DEV_INT_EN, &regs->dev_int_en);
	if (RIO_SUCCESS != rc) {
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_PW_TRAN_CTL, &regs->pw_trans_dis);

fail:
	return rc;

}

static uint32_t rxs_rio_em_dev_rpt_ctl_dev(DAR_DEV_INFO_t *dev_info,
		rio_em_notfn_ctl_t notfn)
{
	uint32_t rc;
	rxs_rpt_ctl_regs_t regs;

	rc = rxs_rio_em_dev_rpt_ctl_reg_read(dev_info, &regs);
	if (RIO_SUCCESS != rc) {
		goto fail;
	}

	if ((rio_em_notfn_int == notfn) || (rio_em_notfn_both == notfn)) {
		regs.dev_int_en = RXS_EM_DEV_INT_EN_INT_EN;
	} else {
		regs.dev_int_en = 0;
	}

	if ((rio_em_notfn_pw == notfn) || (rio_em_notfn_both == notfn)) {
		regs.pw_trans_dis = 0;
	} else {
		regs.pw_trans_dis = RXS_PW_TRAN_CTL_PW_DIS;
	}

	rc = DARRegWrite(dev_info, RXS_EM_DEV_INT_EN, regs.dev_int_en);
	if (RIO_SUCCESS != rc) {
		goto fail;
	}

	rc = DARRegWrite(dev_info, RXS_PW_TRAN_CTL, regs.pw_trans_dis);
	if (RIO_SUCCESS != rc) {
		goto fail;
	}

fail:
	return rc;
}

static uint32_t rxs_rio_em_dev_rpt_ctl_port(DAR_DEV_INFO_t *dev_info,
		rio_em_notfn_ctl_t notfn, rio_port_t port)
{
	uint32_t rc;
	uint32_t int_en;
	uint32_t err_stat;

	rc = DARRegRead(dev_info, RXS_PLM_SPX_ALL_INT_EN(port), &int_en);
	if (RIO_SUCCESS != rc) {
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_SPX_ERR_STAT(port), &err_stat);
	if (RIO_SUCCESS != rc) {
		goto fail;
	}

	err_stat &= KEEP_W1C_ERR_STAT_MASK;

	switch (notfn) {
	case rio_em_notfn_none:
		int_en &= ~RXS_PLM_SPX_ALL_INT_EN_IRQ_EN;
		err_stat |= RXS_SPX_ERR_STAT_PORT_W_DIS;
		break;

	case rio_em_notfn_int:
		int_en = RXS_PLM_SPX_ALL_INT_EN_IRQ_EN;
		err_stat |= RXS_SPX_ERR_STAT_PORT_W_DIS;
		break;

	case rio_em_notfn_pw:
		int_en &= ~RXS_PLM_SPX_ALL_INT_EN_IRQ_EN;
		err_stat &= ~RXS_SPX_ERR_STAT_PORT_W_DIS;
		break;

	case rio_em_notfn_both:
		int_en = RXS_PLM_SPX_ALL_INT_EN_IRQ_EN;
		err_stat &= ~RXS_SPX_ERR_STAT_PORT_W_DIS;
		break;

	default:
		// Should never get here...
		rc = RIO_ERR_SW_FAILURE;
		goto fail;
	}

	rc = DARRegWrite(dev_info, RXS_PLM_SPX_ALL_INT_EN(port), int_en);
	if (RIO_SUCCESS != rc) {
		goto fail;
	}

	rc = DARRegWrite(dev_info, RXS_SPX_ERR_STAT(port), err_stat);
	if (RIO_SUCCESS != rc) {
		goto fail;
	}

fail:
	return rc;
}

uint32_t rxs_rio_em_dev_rpt_ctl(DAR_DEV_INFO_t *dev_info,
		rio_em_dev_rpt_ctl_in_t *in_parms,
		rio_em_dev_rpt_ctl_out_t *out_parms)
{
	struct DAR_ptl good_ptl;
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t port_idx;
	rio_port_t port;
	rxs_rpt_ctl_regs_t regs;

	if (rio_em_notfn_last <= in_parms->notfn) {
		out_parms->imp_rc = EM_DEV_RPT_CTL(0x1);
		goto fail;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_DEV_RPT_CTL(0x2);
		goto fail;
	}

	if (rio_em_notfn_0delta == in_parms->notfn) {
		goto get_dev_rpt_ctl;
	}

	// Disable global reporting, change all port reporting, then
	// set global reporting.
	rc = rxs_rio_em_dev_rpt_ctl_dev(dev_info, rio_em_notfn_none);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_DEV_RPT_CTL(0x10);
		goto fail;
	}

	for (port_idx = 0; port_idx < good_ptl.num_ports; port_idx++) {
		port = good_ptl.pnums[port_idx];
		rc = rxs_rio_em_dev_rpt_ctl_port(
					dev_info, in_parms->notfn, port);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_DEV_RPT_CTL(0x10);
			goto fail;
		}
	}

	if (rio_em_notfn_none != in_parms->notfn) {
		rc = rxs_rio_em_dev_rpt_ctl_dev(dev_info, in_parms->notfn);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_DEV_RPT_CTL(0x10);
			goto fail;
		}
	}

get_dev_rpt_ctl:
	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->notfn = rio_em_notfn_0delta;
	rc = rxs_rio_em_dev_rpt_ctl_reg_read(dev_info, &regs);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_DEV_RPT_CTL(0x50);
		goto fail;
	}

	out_parms->notfn = rio_em_notfn_none;
	if (regs.dev_int_en) {
		if (!regs.pw_trans_dis) {
			out_parms->notfn = rio_em_notfn_both;
		} else {
			out_parms->notfn = rio_em_notfn_int;
		}
	} else {
		if (!regs.pw_trans_dis) {
			out_parms->notfn = rio_em_notfn_pw;
		}
	}
	rc = RIO_SUCCESS;
fail:
	return rc;
}

uint32_t rxs_rio_em_parse_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_parse_pw_in_t *in_parms,
		rio_em_parse_pw_out_t *out_parms)
{
	rio_port_t port;
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	const int IMP_SPEC_IDX = RIO_EMHS_PW_IMP_SPEC_IDX;
	const int ERR_DET_IDX = RIO_EMHS_PW_P_ERR_DET_IDX;
	const uint32_t other_mask = RXS_PW_ZERO |
				RXS_PW_MULTIPORT |
				RXS_PW_DEV_ECC |
				RXS_PW_II_CHG_0 |
				RXS_PW_II_CHG_1 |
				RXS_PW_II_CHG_2  |
				RXS_PW_II_CHG_3 |
				RXS_PW_FAB_OR_DEL |
				RXS_PW_EL_INTA |
				RXS_PW_EL_INTB |
				RXS_PW_PCAP;
	uint32_t mask;
	bool check;

	out_parms->imp_rc = 0;
	out_parms->num_events = 0;
	out_parms->too_many = false;
	out_parms->other_events = false;

	if (!(in_parms->num_events) || (NULL == in_parms->events)) {
		out_parms->imp_rc = EM_PARSE_PW(0x01);
		goto fail;
	}

	port = in_parms->pw[IMP_SPEC_IDX] & RIO_EM_PW_IMP_SPEC_PORT_MASK;

	if (port >= NUM_RXS_PORTS(dev_info)) {
		out_parms->imp_rc = EM_PARSE_PW(0x02);
		goto fail;
	}

	check = (in_parms->pw[ERR_DET_IDX] & RXS_SPX_ERR_DET_OK_TO_UNINIT);
	check |= (in_parms->pw[ERR_DET_IDX] & RXS_SPX_ERR_DET_DLT);
	check |= (in_parms->pw[IMP_SPEC_IDX] & RXS_PW_OK_TO_UNINIT);
	check |= (in_parms->pw[IMP_SPEC_IDX] & RXS_PW_DLT);
	check |= (in_parms->pw[IMP_SPEC_IDX] & RXS_PW_DWNGD);
	if (check) {
		SAFE_ADD_EVENT_N_LOC(rio_em_f_los, port);
	}

	if (in_parms->pw[IMP_SPEC_IDX] & RXS_PW_PORT_ERR) {
		SAFE_ADD_EVENT_N_LOC(rio_em_f_port_err, port);
	}

	// Assume a too many retries AND a too many PNA event happenned,
	// no way to tell which one or both from the port-write contents.
	if (in_parms->pw[IMP_SPEC_IDX] & RXS_PW_MAX_DENIAL) {
		SAFE_ADD_EVENT_N_LOC(rio_em_f_2many_retx, port);
		SAFE_ADD_EVENT_N_LOC(rio_em_f_2many_pna, port);
	}

	// RXS cannot actually detect an error rate event, so
	// we'll map this ECC-related fatal error to error rate events
	// so that the appropriate recovery is invoked (port reset).
	if (in_parms->pw[IMP_SPEC_IDX] & RXS_PW_PBM_FATAL) {
		SAFE_ADD_EVENT_N_LOC(rio_em_f_err_rate, port);
	}

	// Packet dropped by TTL is a PBM event
	if (in_parms->pw[IMP_SPEC_IDX] & RXS_PW_PBM_PW) {
		SAFE_ADD_EVENT_N_LOC(rio_em_d_ttl, port);
	}

	// Packet dropped by routing table entry is a TLM event
	if (in_parms->pw[IMP_SPEC_IDX] & RXS_PW_TLM_PW) {
		SAFE_ADD_EVENT_N_LOC(rio_em_d_rte, port);
	}

	// Device level event...
	if (in_parms->pw[RIO_EM_PW_L_ERR_DET_IDX]) {
		SAFE_ADD_EVENT_N_LOC(rio_em_d_log, RIO_ALL_PORTS);
	}

	// This port has a reset request going on...
	mask = RXS_PW_RST_REQ | RXS_PW_PRST_REQ;
	if (in_parms->pw[IMP_SPEC_IDX] & mask) {
		SAFE_ADD_EVENT_N_LOC(rio_em_i_rst_req, port);
	}

	// Some other port(s) has a reset request going on...
	if (in_parms->pw[IMP_SPEC_IDX] & RXS_PW_DEV_RCS) {
		SAFE_ADD_EVENT_N_LOC(rio_em_i_rst_req, RIO_ALL_PORTS);
	}

	// Link initialized
	if ((in_parms->pw[ERR_DET_IDX] & RXS_SPX_ERR_DET_LINK_INIT) ||
			(in_parms->pw[IMP_SPEC_IDX] & RXS_PW_LINK_INIT)) {
		SAFE_ADD_EVENT_N_LOC(rio_em_i_sig_det, port);
	}

	// Device level event...
	if (in_parms->pw[IMP_SPEC_IDX] & RXS_PW_INIT_FAIL) {
		SAFE_ADD_EVENT_N_LOC(rio_em_i_init_fail, RIO_ALL_PORTS);
	}

	if (in_parms->pw[IMP_SPEC_IDX] & other_mask) {
		out_parms->other_events = true;
	}
	rc = RIO_SUCCESS;

fail:
	return rc;
}

uint32_t rxs_rio_em_get_int_stat_port(DAR_DEV_INFO_t *dev_info,
		rio_em_get_int_stat_in_t *in_parms,
		rio_em_get_int_stat_out_t *out_parms, rio_port_t port)
{
	uint32_t plm_denial_ctl;
	uint32_t spx_err_det;
	uint32_t spx_rate_en;
	uint32_t plm_ints;
	uint32_t plm_int_en;
	uint32_t plm_int_stat;
	uint32_t tlm_ints;
	uint32_t tlm_int_en;
	uint32_t tlm_int_stat;
	uint32_t pbm_ints;
	uint32_t pbm_int_en;
	uint32_t pbm_int_stat;
	uint32_t rst_int_en;
	uint32_t pna_cap;
	uint32_t rc;

	rc = DARRegRead(dev_info, RXS_SPX_ERR_DET(port), &spx_err_det);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x10);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_SPX_RATE_EN(port), &spx_rate_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x10);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_PLM_SPX_STAT(port), &plm_ints);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x11);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_PLM_SPX_INT_EN(port), &plm_int_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x12);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_TLM_SPX_STAT(port), &tlm_ints);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x10);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_TLM_SPX_INT_EN(port), &tlm_int_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x12);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_PBM_SPX_STAT(port), &pbm_ints);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x10);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_PBM_SPX_INT_EN(port), &pbm_int_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x12);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_EM_RST_INT_EN, &rst_int_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x14);
		goto fail;
	}

	plm_int_stat = plm_ints & (plm_int_en | RXS_PLM_SPX_UNMASKABLE_MASK);

	if (plm_int_stat & RXS_LOS_EVENT_MASK) {
		SAFE_ADD_EVENT_N_LOC(rio_em_f_los, port);
	}

	if (plm_int_stat & RXS_PLM_SPX_STAT_PORT_ERR) {
		SAFE_ADD_EVENT_N_LOC(rio_em_f_port_err, port);
	}

	if (plm_int_stat & RXS_PLM_SPX_STAT_MAX_DENIAL) {
		rc = DARRegRead(dev_info, RXS_PLM_SPX_DENIAL_CTL(port),
							&plm_denial_ctl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_INT_STAT(0x1A);
			goto fail;
		}
		if (RXS_PLM_SPX_DENIAL_CTL_CNT_RTY & plm_denial_ctl) {
			SAFE_ADD_EVENT_N_LOC(rio_em_f_2many_retx, port);
		}
		if (RXS_PLM_SPX_DENIAL_CTL_CNT_PNA & plm_denial_ctl) {
			rc = DARRegRead(dev_info, RXS_PLM_SPX_PNA_CAP(port),
							&pna_cap);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_GET_INT_STAT(0x1A);
				goto fail;
			}
			if (RXS_PLM_SPX_PNA_CAP_VALID & pna_cap) {
				SAFE_ADD_EVENT_N_LOC(rio_em_f_2many_pna, port);
			}
		}
	}

	if (plm_int_stat & RXS_PLM_SPX_STAT_PBM_FATAL) {
		SAFE_ADD_EVENT_N_LOC(rio_em_f_err_rate, port);
	}

	pbm_int_stat = pbm_ints & pbm_int_en;
	if (pbm_int_stat & RXS_PBM_SPX_STAT_EG_TTL_EXPIRED) {
		SAFE_ADD_EVENT_N_LOC(rio_em_d_ttl, port);
	}

	tlm_int_stat = tlm_ints & tlm_int_en;
	if (tlm_int_stat & RXS_TLM_SPX_STAT_LUT_DISCARD) {
		SAFE_ADD_EVENT_N_LOC(rio_em_d_rte, port);
	}

	if (plm_int_stat & RXS_PLM_SPX_STAT_LINK_INIT) {
		SAFE_ADD_EVENT_N_LOC(rio_em_i_sig_det, port);
	}

	if (plm_int_stat & RXS_RST_REQ_EVENT_MASK) {
		SAFE_ADD_EVENT_N_LOC(rio_em_i_rst_req, port);
	}

	if (plm_ints & ~(plm_int_en | RXS_PLM_SPX_UNMASKABLE_MASK)) {
		out_parms->other_events = true;
	}
	if (tlm_ints & ~tlm_int_en) {
		out_parms->other_events = true;
	}
	if (pbm_ints & ~(pbm_int_en | RXS_PBM_FATAL_EVENT_MASK)) {
		out_parms->other_events = true;
	}
fail:
	return rc;
}

uint32_t get_dev_int_status(DAR_DEV_INFO_t *dev_info,
		rio_em_get_int_stat_in_t *in_parms,
		rio_em_get_int_stat_out_t *out_parms)
{
	uint32_t rc;
	uint32_t em_ints;
	uint32_t em_int_en;
	uint32_t em_int_stat;
	uint32_t log_err_det;
	uint32_t i2c_int_stat;

	rc = DARRegRead(dev_info, RXS_EM_INT_STAT, &em_ints);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x30);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_EM_INT_EN, &em_int_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x31);
		goto fail;
	}

	em_int_stat = em_ints & em_int_en;

	if (em_int_stat & RXS_EM_INT_STAT_LOG) {
		SAFE_ADD_EVENT_N_LOC(rio_em_d_log, RIO_ALL_PORTS);
	} else {
		rc = DARRegRead(dev_info, RXS_ERR_DET, &log_err_det);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_INT_STAT(0x31);
			goto fail;
		}
		if (log_err_det) {
			out_parms->other_events = true;
		}
	}

	if (em_int_stat & RXS_EM_INT_STAT_EXTERNAL_I2C) {
		SAFE_ADD_EVENT_N_LOC(rio_em_i_init_fail, RIO_ALL_PORTS);
	} else {
		rc = DARRegRead(dev_info, I2C_INT_STAT, &i2c_int_stat);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_INT_STAT(0x31);
			goto fail;
		}
		if (i2c_int_stat & I2C_INT_STAT_BL_FAIL) {
			out_parms->other_events = true;
		}
	}

	if (em_ints & ~(em_int_en | RXS_EM_INT_STAT_AGG_EVENTS_MASK)) {
		out_parms->other_events = true;
	}
fail:
	return rc;
}

uint32_t rxs_rio_em_get_int_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_int_stat_in_t *in_parms,
		rio_em_get_int_stat_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	struct DAR_ptl good_ptl;
	uint32_t port_idx;
	rio_port_t port;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->num_events = 0;
	out_parms->too_many = false;
	out_parms->other_events = false;

	if (!(in_parms->num_events) || (NULL == in_parms->events)) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x01);
		goto fail;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x02);
		goto fail;
	}

	// Get interrupt status for each requested port
	for (port_idx = 0; port_idx < good_ptl.num_ports; port_idx++) {
		port = good_ptl.pnums[port_idx];
		rc = rxs_rio_em_get_int_stat_port(dev_info, in_parms, out_parms,
									port);
		if (RIO_SUCCESS != rc) {
			goto fail;
		}
	}

	rc = get_dev_int_status(dev_info, in_parms, out_parms);
fail:
	return rc;
}

uint32_t rxs_rio_em_get_pw_stat_port(DAR_DEV_INFO_t *dev_info,
		rio_em_get_pw_stat_in_t *in_parms,
		rio_em_get_pw_stat_out_t *out_parms, rio_port_t port)
{
	uint32_t rc;
	uint32_t spx_err_det;
	uint32_t spx_rate_en;
	uint32_t plm_pws;
	uint32_t plm_pw_en;
	uint32_t plm_pw_stat;
	uint32_t tlm_pws;
	uint32_t tlm_pw_en;
	uint32_t tlm_pw_stat;
	uint32_t pbm_pws;
	uint32_t pbm_pw_en;
	uint32_t pbm_pw_stat;
	uint32_t em_pws;
	uint32_t em_pw_en;
	uint32_t rst_pw_en;
	uint32_t plm_denial_ctl;
	uint32_t pna_cap;

	rc = DARRegRead(dev_info, RXS_SPX_ERR_DET(port), &spx_err_det);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x10);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_SPX_RATE_EN(port), &spx_rate_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_INT_STAT(0x10);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_PLM_SPX_STAT(port), &plm_pws);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x10);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_PLM_SPX_PW_EN(port), &plm_pw_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x12);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_TLM_SPX_STAT(port), &tlm_pws);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x10);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_TLM_SPX_PW_EN(port), &tlm_pw_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x12);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_PBM_SPX_STAT(port), &pbm_pws);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x10);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_PBM_SPX_PW_EN(port), &pbm_pw_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x12);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_EM_RST_PW_EN, &rst_pw_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x14);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_EM_PW_STAT, &em_pws);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x16);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_EM_PW_EN, &em_pw_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x18);
		goto fail;
	}

	plm_pw_stat = plm_pws & (plm_pw_en| RXS_PLM_SPX_UNMASKABLE_MASK);
	if (plm_pw_stat & RXS_LOS_EVENT_MASK) {
		SAFE_ADD_EVENT_N_LOC(rio_em_f_los, port);
	}

	if (plm_pw_stat & RXS_PLM_SPX_STAT_PORT_ERR) {
		SAFE_ADD_EVENT_N_LOC(rio_em_f_port_err, port);
	}

	if (plm_pw_stat & RXS_PLM_SPX_STAT_MAX_DENIAL) {
		rc = DARRegRead(dev_info, RXS_PLM_SPX_DENIAL_CTL(port),
							&plm_denial_ctl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_INT_STAT(0x1A);
			goto fail;
		}
		if (RXS_PLM_SPX_DENIAL_CTL_CNT_RTY & plm_denial_ctl) {
			SAFE_ADD_EVENT_N_LOC(rio_em_f_2many_retx, port);
		}
		if (RXS_PLM_SPX_DENIAL_CTL_CNT_PNA & plm_denial_ctl) {
			rc = DARRegRead(dev_info, RXS_PLM_SPX_PNA_CAP(port),
							&pna_cap);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_GET_PW_STAT(0x1A);
				goto fail;
			}
			if (RXS_PLM_SPX_PNA_CAP_VALID & pna_cap) {
				SAFE_ADD_EVENT_N_LOC(rio_em_f_2many_pna, port);
			}
		}
	}

	if (plm_pw_stat & RXS_PLM_SPX_STAT_PBM_FATAL) {
		SAFE_ADD_EVENT_N_LOC(rio_em_f_err_rate, port);
	}

	pbm_pw_stat = pbm_pws & pbm_pw_en;
	if (pbm_pw_stat & RXS_PBM_SPX_STAT_EG_TTL_EXPIRED) {
		SAFE_ADD_EVENT_N_LOC(rio_em_d_ttl, port);
	}

	tlm_pw_stat = tlm_pws & tlm_pw_en;
	if (tlm_pw_stat & RXS_TLM_SPX_STAT_LUT_DISCARD) {
		SAFE_ADD_EVENT_N_LOC(rio_em_d_rte, port);
	}

	if (plm_pw_stat & RXS_PLM_SPX_STAT_LINK_INIT) {
		SAFE_ADD_EVENT_N_LOC(rio_em_i_sig_det, port);
	}

	if (plm_pw_stat & RXS_RST_REQ_EVENT_MASK) {
		SAFE_ADD_EVENT_N_LOC(rio_em_i_rst_req, port);
	}

	if (plm_pws & ~(plm_pw_en| RXS_PLM_SPX_UNMASKABLE_MASK)) {
		out_parms->other_events = true;
	}
	if (tlm_pws & ~tlm_pw_en) {
		out_parms->other_events = true;
	}
	if (pbm_pws & ~(pbm_pw_en | RXS_PBM_FATAL_EVENT_MASK)) {
		out_parms->other_events = true;
	}
fail:
	return rc;
}

uint32_t get_dev_pw_status(DAR_DEV_INFO_t *dev_info,
		rio_em_get_pw_stat_in_t *in_parms,
		rio_em_get_pw_stat_out_t *out_parms)
{
	uint32_t rc;
	uint32_t em_pws;
	uint32_t em_pw_en;
	uint32_t em_pw_stat;
	uint32_t log_err_det;
	uint32_t i2c_int_stat;

	rc = DARRegRead(dev_info, RXS_EM_PW_STAT, &em_pws);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x10);
		goto fail;
	}

	rc = DARRegRead(dev_info, RXS_EM_PW_EN, &em_pw_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x12);
		goto fail;
	}

	em_pw_stat = em_pws & em_pw_en;

	if (em_pw_stat & RXS_EM_PW_STAT_LOG) {
		SAFE_ADD_EVENT_N_LOC(rio_em_d_log, RIO_ALL_PORTS);
	} else {
		rc = DARRegRead(dev_info, RXS_ERR_DET, &log_err_det);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_PW_STAT(0x24);
			goto fail;
		}
		if (log_err_det) {
			out_parms->other_events = true;
		}
	}

	if (em_pw_stat & RXS_EM_PW_STAT_EXTERNAL_I2C) {
		SAFE_ADD_EVENT_N_LOC(rio_em_i_init_fail, RIO_ALL_PORTS);
	} else {
		rc = DARRegRead(dev_info, I2C_INT_STAT, &i2c_int_stat);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_GET_INT_STAT(0x31);
			goto fail;
		}
		if (i2c_int_stat & I2C_INT_STAT_BL_FAIL) {
			out_parms->other_events = true;
		}
	}

	if (em_pw_stat & ~(em_pw_en | RXS_EM_PW_STAT_AGG_EVENTS_MASK)) {
		out_parms->other_events = true;
	}
fail:
	return rc;
}

uint32_t rxs_rio_em_get_pw_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_pw_stat_in_t *in_parms,
		rio_em_get_pw_stat_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	struct DAR_ptl good_ptl;
	unsigned int port_idx;
	unsigned int pw_port_idx = RIO_ALL_PORTS;
	rio_port_t port;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->num_events = 0;
	out_parms->too_many = false;
	out_parms->other_events = false;

	if (!(in_parms->num_events) || (NULL == in_parms->events)) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x01);
		goto fail;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_GET_PW_STAT(0x02);
		goto fail;
	}

	// Get port-write status for each requested port EXCEPT
	// the port which the port-write was received on.
	for (port_idx = 0; port_idx < good_ptl.num_ports; port_idx++) {
		port = good_ptl.pnums[port_idx];
		if (in_parms->pw_port_num == port) {
			pw_port_idx = port_idx;
			continue;
		}
		rc = rxs_rio_em_get_pw_stat_port(
				dev_info, in_parms, out_parms, port);
		if (RIO_SUCCESS != rc) {
			goto fail;
		}
	}
	if (pw_port_idx != RIO_ALL_PORTS) {
		rc = rxs_rio_em_get_pw_stat_port(
			dev_info, in_parms, out_parms, in_parms->pw_port_num);
		if (RIO_SUCCESS != rc) {
			goto fail;
		}
	}

	rc = get_dev_pw_status(dev_info, in_parms, out_parms);

	// Last event in the list must be a clear port-write pending,
	// but only if the port which originated the port-write is part
	// of the port list searched for events.
	if (pw_port_idx != RIO_ALL_PORTS) {
		SAFE_ADD_EVENT_N_LOC(rio_em_a_clr_pwpnd,
					in_parms->pw_port_num);
	}
fail:
	return rc;
}

// Determine which ports must be reset to clear events.
// Events that require a port reset are:
//		rio_em_f_los,
//		rio_em_f_port_err,
//		rio_em_f_2many_retx,
//		rio_em_f_2many_pna, and
//		rio_em_f_err_rate.
//
// Determine which ports do not have any events.
// Also figure out what the event index is for logical layer errors
// and initialization error, if any.
//
// ptl.pnum[port] = RIO_ALL_PORTS if the port must be reset to clear events
// ptl.pnum[port] = NUM_RXS_PORTS when a port has events to be cleared
// ptl.pnum[port] = port when a port has no events to be cleared

static uint32_t rxs_clr_events_sort_events(DAR_DEV_INFO_t *dev_info,
		rio_em_clr_events_in_t *in_parms,
		rio_em_clr_events_out_t *out_parms, struct DAR_ptl *ptl,
		uint32_t *log_err_idx, uint32_t *init_err_idx,
		rio_port_t *pw_pt)
{
	uint32_t rc;
	unsigned int i;
	rio_port_t rxs_num_ports = NUM_RXS_PORTS(dev_info);
	rio_em_event_n_loc_t *event;

	*log_err_idx = NO_EVENT_IDX;
	*init_err_idx = NO_EVENT_IDX;
	*pw_pt = RIO_ALL_PORTS;

	rc = DARrioGetPortList(dev_info, (struct DAR_ptl *)&ptl_all_ports, ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CLR_EVENTS(0x03);
		goto fail;
	}

	rc = RIO_ERR_INVALID_PARAMETER;

	// Do any events require a soft reset to clear them?  Check!
	// Also validate port numbers and events while we're at it.
	for (i = 0; i < in_parms->num_events; i++) {
		event = &in_parms->events[i];
		out_parms->failure_idx = i;

		switch (event->event) {
		case rio_em_f_los:
		case rio_em_f_port_err:
		case rio_em_f_2many_retx:
		case rio_em_f_2many_pna:
		case rio_em_f_err_rate:
			if (NUM_RXS_PORTS(dev_info) <= event->port_num) {
				goto fail;
			}
			ptl->pnums[event->port_num] = RIO_ALL_PORTS;
			break;
		case rio_em_d_log:
			*log_err_idx = i;
			break;
		case rio_em_d_ttl:
		case rio_em_d_rte:
		case rio_em_i_sig_det:
		case rio_em_i_rst_req:
			if (NUM_RXS_PORTS(dev_info) <= event->port_num) {
				goto fail;
			}
			if (ptl->pnums[event->port_num] != RIO_ALL_PORTS) {
				ptl->pnums[event->port_num] = rxs_num_ports;
			}
			break;
		case rio_em_i_init_fail:
			*init_err_idx = i;
			break;
		case rio_em_a_clr_pwpnd:
			if (RIO_ALL_PORTS != *pw_pt) {
				// Multiple requests to clear port-write
				// Pending... Illegal!
				goto fail;
			}
			if (NUM_RXS_PORTS(dev_info) <= event->port_num) {
				goto fail;
			}
			*pw_pt = event->port_num;
			break;
		case rio_em_a_no_event:
			break;
		default:
			goto fail;
		}
	}
	rc = RIO_SUCCESS;
fail:
	if (RIO_SUCCESS == rc) {
		out_parms->failure_idx = 0;
	}
	return rc;
}

// Extracts ports which were reset or otherwise active from
// a ptl produced by rxs_clr_events_sort_events.

static void copy_sorted_ports(struct DAR_ptl *new_ptl, struct DAR_ptl *sorted_ptl)
{
	unsigned int i;

	new_ptl->num_ports = 0;
	for (i = 0; i < sorted_ptl->num_ports; i++) {
		if (sorted_ptl->pnums[i] == i) {
			continue;
		}
		new_ptl->pnums[new_ptl->num_ports] = i;
		new_ptl->num_ports++;
	}
}

static uint32_t rxs_clr_events_soft_reset(DAR_DEV_INFO_t *dev_info,
		rio_em_clr_events_out_t *out_parms, rio_port_t port)
{
	uint32_t rc;
	uint32_t plm_ctl, plm_ctl_rst;

	rc = DARRegRead(dev_info, RXS_PLM_SPX_IMP_SPEC_CTL(port), &plm_ctl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CLR_EVENTS(0x20);
		goto fail;
	}

	plm_ctl_rst = plm_ctl | RXS_PLM_SPX_IMP_SPEC_CTL_SOFT_RST_PORT;

	rc = DARRegWrite(dev_info, RXS_PLM_SPX_IMP_SPEC_CTL(port), plm_ctl_rst);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CLR_EVENTS(0x22);
		goto fail;
	}

	rc = DARRegWrite(dev_info, RXS_PLM_SPX_IMP_SPEC_CTL(port), plm_ctl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CLR_EVENTS(0x24);
		goto fail;
	}

	// Resetting the port creates an event in the device-level
	// reset port status register.  Clear that event.
	rc = DARRegWrite(dev_info, RXS_EM_RST_PORT_STAT, 1 << port);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CLR_EVENTS(0x30);
		goto fail;
	}
fail:
	return rc;
}

uint32_t rxs_clr_events_clr_port(DAR_DEV_INFO_t *dev_info,
		rio_em_clr_events_in_t *in_parms,
		rio_em_clr_events_out_t *out_parms, rio_port_t port)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t i;
	uint32_t temp;
	rio_em_event_n_loc_t *event;

	for (i = 0; i < in_parms->num_events; i++) {
		if (port != in_parms->events[i].port_num) {
			continue;
		}
		out_parms->failure_idx = i;
		event = &in_parms->events[i];

		switch (event->event) {
		// None of these events should be present for this port, as
		// they require a port reset to clear them.  Report the error.
		case rio_em_f_los:
		case rio_em_f_port_err:
		case rio_em_f_2many_retx:
		case rio_em_f_2many_pna:
		case rio_em_f_err_rate:
			goto fail;

		case rio_em_d_ttl:
			rc = DARRegWrite(dev_info, RXS_PBM_SPX_STAT(port),
					RXS_PBM_SPX_STAT_EG_TTL_EXPIRED);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x40);
				goto fail;
			}
			break;

		case rio_em_d_rte:
			rc = DARRegWrite(dev_info, RXS_TLM_SPX_STAT(port),
						RXS_TLM_SPX_STAT_LUT_DISCARD);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x50);
				goto fail;
			}
			break;

		// Logical errors are for a device, not a port.
		// Report the error.
		case rio_em_d_log:
			goto fail;

		case rio_em_i_sig_det:
			rc = DARRegWrite(dev_info, RXS_PLM_SPX_STAT(port),
						RXS_PLM_SPX_STAT_LINK_INIT);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x58);
				goto fail;
			}
			break;

		case rio_em_i_rst_req:
			rc = DARRegWrite(dev_info, RXS_PLM_SPX_STAT(port),
							RXS_RST_REQ_EVENT_MASK);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x60);
				goto fail;
			}

			rc = DARRegWrite(dev_info, RXS_EM_RST_PORT_STAT, 1 << port);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x68);
				goto fail;
			}
			break;

		// Initialization failures are for a device, not a port.
		// Report the error.

		case rio_em_i_init_fail:
			goto fail;

		case rio_em_a_clr_pwpnd:
			rc = DARRegRead(dev_info, RXS_SPX_ERR_STAT(port),
									&temp);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x70);
				goto fail;
			}
			temp |= RXS_SPX_ERR_STAT_PORT_W_P;
			rc = DARRegWrite(dev_info, RXS_SPX_ERR_STAT(port),
									temp);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CLR_EVENTS(0x78);
				goto fail;
			}
			break;

		case rio_em_a_no_event: // No event, nothing to do
			break;

		default:
			goto fail;
		}
	}
	rc = RIO_SUCCESS;
fail:
	if (RIO_SUCCESS == rc) {
		out_parms->failure_idx = 0;
	}
	return rc;
}

static uint32_t rxs_rio_em_clr_events_get_int_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_clr_events_out_t *out_parms, struct DAR_ptl *sorted_ptl)
{
	rio_em_get_int_stat_in_t in_i;
	rio_em_get_int_stat_out_t out_i;
	rio_em_event_n_loc_t stat_e[1];
	uint32_t rc = RIO_SUCCESS;

	copy_sorted_ports(&in_i.ptl, sorted_ptl);
	in_i.num_events = 1;
	in_i.events = stat_e;

	// If only device level events were cleared, no port exists to query
	// so don't do it...
	if (in_i.ptl.num_ports) {
		rc = rxs_rio_em_get_int_stat(dev_info, &in_i, &out_i);
	} else {
		out_i.imp_rc = RIO_SUCCESS;
		out_i.num_events = 0;
		out_i.too_many = false;
		out_i.other_events = false;

		rc = get_dev_int_status(dev_info, &in_i, &out_i);
	}
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = out_i.imp_rc;
	} else {
		if (out_i.num_events || out_i.other_events) {
			out_parms->int_events_remain = true;
		}
	}
	return rc;
}

static uint32_t rxs_rio_em_clr_events_get_pw_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_clr_events_out_t *out_parms, struct DAR_ptl *sorted_ptl)
{
	rio_em_get_pw_stat_in_t in_p;
	rio_em_get_pw_stat_out_t out_p;
	rio_em_event_n_loc_t stat_e[1];
	uint32_t rc;

	copy_sorted_ports(&in_p.ptl, sorted_ptl);
	in_p.num_events = 1;
	in_p.events = stat_e;

	if (in_p.ptl.num_ports) {
		rc = rxs_rio_em_get_pw_stat(dev_info, &in_p, &out_p);
	} else {
		out_p.imp_rc = RIO_SUCCESS;
		out_p.num_events = 0;
		out_p.too_many = false;
		out_p.other_events = false;

		rc = get_dev_pw_status(dev_info, &in_p, &out_p);
	}

	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = out_p.imp_rc;
	} else {
		if (out_p.num_events || out_p.other_events) {
			out_parms->pw_events_remain = true;
		}
	}
	return rc;
}

uint32_t rxs_rio_em_clr_events(DAR_DEV_INFO_t *dev_info,
		rio_em_clr_events_in_t *in_parms,
		rio_em_clr_events_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	struct DAR_ptl sorted_ptl;
	uint32_t log_err_idx;
	uint32_t init_err_idx;
	rio_port_t pw_pt;
	rio_port_t port;

	if (!(in_parms->num_events) || (NULL == in_parms->events)) {
		out_parms->imp_rc = EM_CLR_EVENTS(0x01);
		goto fail;
	}

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->failure_idx = 0;
	out_parms->pw_events_remain = false;
	out_parms->int_events_remain = false;

	rc = rxs_clr_events_sort_events(dev_info, in_parms, out_parms,
			&sorted_ptl, &log_err_idx, &init_err_idx, &pw_pt);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = EM_CLR_EVENTS(0x02);
		goto fail;
	}

	for (port = 0; port < NUM_RXS_PORTS(dev_info); port++) {
		// The port which sent the port write must have its errors
		// cleared last.
		if (pw_pt == port) {
			continue;
		}

		// If no events are present for the port, skip it
		if (sorted_ptl.pnums[port] == port) {
			continue;
		}

		// A soft reset will wipe out all events for the port.
		if (RIO_ALL_PORTS == sorted_ptl.pnums[port]) {
			rc = rxs_clr_events_soft_reset(dev_info, out_parms,
									port);
		} else {
			// Otherwise, we're left with events that just require
			// clearing.  Search for all events for the port, and
			// clear them.

			rc = rxs_clr_events_clr_port(dev_info, in_parms,
							out_parms, port);
		}
		if (RIO_SUCCESS != rc) {
			goto fail;
		}
	}

	// If no events are present for the port, skip it
	if (RIO_ALL_PORTS != pw_pt) {
		if (RIO_ALL_PORTS == sorted_ptl.pnums[pw_pt]) {
			rc = rxs_clr_events_soft_reset(
						dev_info, out_parms, pw_pt);
		} else {
			rc = rxs_clr_events_clr_port(dev_info, in_parms,
							out_parms, pw_pt);
		}
		if (RIO_SUCCESS != rc) {
			goto fail;
		}
	}

	if (NO_EVENT_IDX != init_err_idx) {
		rc = DARRegWrite(dev_info, I2C_INT_STAT, I2C_INT_STAT_BL_FAIL);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_CLR_EVENTS(0x80);
			goto fail;
		}
	}

	if (NO_EVENT_IDX != log_err_idx) {
		rc = DARRegWrite(dev_info, RXS_ERR_DET, 0);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = EM_CLR_EVENTS(0x90);
			goto fail;
		}
	}

	rc = rxs_rio_em_clr_events_get_int_stat(dev_info, out_parms,
								&sorted_ptl);
	if (RIO_SUCCESS != rc) {
		goto fail;
	}

	rc = rxs_rio_em_clr_events_get_pw_stat(dev_info, out_parms,
								&sorted_ptl);
	if (RIO_SUCCESS != rc) {
		goto fail;
	}

	rc = RIO_SUCCESS;

fail:
	return rc;
}

uint32_t rxs_rio_em_create_events(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_in_t *in_parms,
		rio_em_create_events_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	unsigned int i;
	uint32_t log_err_idx = NO_EVENT_IDX;
	uint32_t init_err_idx = NO_EVENT_IDX;
	rio_port_t pt;
	rio_em_event_n_loc_t *event;
	uint32_t spx_err_det;

	out_parms->imp_rc = RIO_SUCCESS;

	if (!in_parms->num_events || (NULL == in_parms->events)) {
		out_parms->imp_rc = EM_CREATE_EVENTS(0x01);
		goto fail;
	}

	out_parms->failure_idx = 0;

	for (i = 0; i < in_parms->num_events; i++) {
		event = &in_parms->events[i];
		pt = event->port_num;

		out_parms->failure_idx = i;

		if (pt >= NUM_RXS_PORTS(dev_info)) {
			if (rio_em_d_log == event->event) {
				log_err_idx = i;
				continue;
			}
			if (rio_em_i_init_fail == event->event) {
				init_err_idx = i;
				continue;
			}
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = EM_CREATE_EVENTS(0x08);
			goto fail;
		}

		switch (event->event) {
		case rio_em_f_los:
			rc = DARRegRead(dev_info, RXS_SPX_ERR_DET(pt),
								&spx_err_det);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CREATE_EVENTS(0x10);
				goto fail;
			}
			spx_err_det |= RXS_SPX_ERR_DET_DLT |
					RXS_SPX_ERR_DET_OK_TO_UNINIT;
			rc = DARRegWrite(dev_info, RXS_SPX_ERR_DET(pt),
								spx_err_det);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CREATE_EVENTS(0x11);
				goto fail;
			}
			rc = DARRegWrite(dev_info, RXS_PLM_SPX_EVENT_GEN(pt),
					RXS_PLM_SPX_EVENT_GEN_DWNGD);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CREATE_EVENTS(0x12);
				goto fail;
			}
			break;

		case rio_em_f_port_err:
			rc = DARRegWrite(dev_info, RXS_PLM_SPX_EVENT_GEN(pt),
						RXS_PLM_SPX_EVENT_GEN_PORT_ERR);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CREATE_EVENTS(0x18);
				goto fail;
			}
			break;

		case rio_em_f_err_rate:
			rc = DARRegWrite(dev_info, RXS_PBM_SPX_EVENT_GEN(pt),
					RXS_PBM_FATAL_EVENT_MASK);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CREATE_EVENTS(0x36);
				goto fail;
			}
			break;

		// Note that there is no way to fake reception of a PNA
		// using registers on real hardware, so without external
		// help this will always be just a 2many_retx event.
		case rio_em_f_2many_retx:
		case rio_em_f_2many_pna:
			rc = DARRegWrite(dev_info, RXS_PLM_SPX_EVENT_GEN(pt),
					RXS_PLM_SPX_EVENT_GEN_MAX_DENIAL);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CREATE_EVENTS(0x38);
				goto fail;
			}
			break;

		case rio_em_d_ttl:
			rc = DARRegWrite(dev_info, RXS_PBM_SPX_EVENT_GEN(pt),
					RXS_PBM_SPX_EVENT_GEN_EG_TTL_EXPIRED);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CREATE_EVENTS(0x38);
				goto fail;
			}
			break;

		case rio_em_d_rte:
			rc = DARRegWrite(dev_info, RXS_TLM_SPX_EVENT_GEN(pt),
					RXS_TLM_SPX_EVENT_GEN_LUT_DISCARD);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CREATE_EVENTS(0x38);
				goto fail;
			}
			break;

		// Nothing to do.
		case rio_em_a_clr_pwpnd:
		case rio_em_a_no_event:
		case rio_em_d_log:
		// Create this event at the end...
		case rio_em_i_init_fail:
			break;

		case rio_em_i_sig_det:
			rc = DARRegWrite(dev_info, RXS_SPX_ERR_DET(pt),
					RXS_SPX_ERR_DET_LINK_INIT);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CREATE_EVENTS(0x10);
				goto fail;
			}
			break;

		case rio_em_i_rst_req:
			rc = DARRegWrite(dev_info, RXS_PLM_SPX_EVENT_GEN(pt),
					RXS_PLM_SPX_EVENT_GEN_RST_REQ |
					RXS_PLM_SPX_EVENT_GEN_PRST_REQ);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = EM_CREATE_EVENTS(0x50);
				goto fail;
			}
			break;

		default:
			out_parms->imp_rc = EM_CREATE_EVENTS(0x58);
			out_parms->failure_idx = i;
			rc = RIO_ERR_INVALID_PARAMETER;
			goto fail;
		}
	}

	if (NO_EVENT_IDX != log_err_idx) {
		rc = DARRegWrite(dev_info, RXS_ERR_DET, RXS_ALL_LOG_ERRS);
		if (RIO_SUCCESS != rc) {
			out_parms->failure_idx = log_err_idx;
			out_parms->imp_rc = EM_CREATE_EVENTS(0x40);
			goto fail;
		}
	}

	if (NO_EVENT_IDX != init_err_idx) {
		rc = DARRegWrite(dev_info, I2C_INT_SET, I2C_INT_SET_BL_FAIL);
		if (RIO_SUCCESS != rc) {
			out_parms->failure_idx = init_err_idx;
			out_parms->imp_rc = EM_CREATE_EVENTS(0x58);
			goto fail;
		}
	}
	rc = RIO_SUCCESS;
fail:
	if (RIO_SUCCESS == rc) {
		out_parms->failure_idx = 0;
	}
	return rc;
}

#endif /* RXS_DAR_WANTED */

#ifdef __cplusplus
}
#endif
