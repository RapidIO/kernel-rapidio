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

#include "Tsi721.h"
#include "Tsi721_API.h"
#include "DAR_DB_Private.h"
#include "DSF_DB_Private.h"
#include "RapidIO_Utilities_API.h"
#include "RapidIO_Port_Config_API.h"
#include "RapidIO_Routing_Table_API.h"
#include "RapidIO_Error_Management_API.h"
#include "string_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TSI721_DAR_WANTED

struct tsi721_dev_ctr {
	rio_sc_ctr_t	ctr_t;
	bool		split;	// Two counters, one register
	bool		tx;	// Transmit (true), or receive (false)
	bool		srio;	// RapidIO, or other interface
	uint32_t	os;     // Register offset
};

#define TSI721_SPLIT_2ND_CTR 0

const struct tsi721_dev_ctr tsi721_dev_ctrs[] = {
//   Enum counter               SPLIT  TX     SRIO   OFFSET
    {rio_sc_pcie_msg_rx,	false, false, false, TSI721_CPL_SMSG_CNT},
    {rio_sc_pcie_msg_tx,	false, true , false, TSI721_TXTLP_SMSG_CNT},
    {rio_sc_pcie_dma_rx,	false, false, false, TSI721_CPL_BDMA_CNT},
    {rio_sc_pcie_dma_tx,	false, true , false, TSI721_TXTLP_BDMA_CNT},
    {rio_sc_pcie_brg_rx,	false, false, false, TSI721_RXTLP_BRG_CNT},
    {rio_sc_pcie_brg_tx,	false, true , false, TSI721_TXTLP_BRG_CNT},
    {rio_sc_rio_msg_tx,		false, true , true , TSI721_TXPKT_SMSG_CNT},
    {rio_sc_rio_msg_rx,		false, false, true , TSI721_RXPKT_SMSG_CNT},
    {rio_sc_rio_msg_tx_rty,	false, true , true , TSI721_RETRY_GEN_CNT},
    {rio_sc_rio_msg_rx_rty,	false, false, true , TSI721_RETRY_RX_CNT},
    {rio_sc_rio_dma_tx,		false, true , true , TSI721_TXPKT_BDMA_CNT},
    {rio_sc_rio_dma_rx,		false, false, true , TSI721_RXRSP_BDMA_CNT},
    {rio_sc_rio_brg_tx,		false, true , true , TSI721_TXPKT_BRG_CNT},
    {rio_sc_rio_brg_rx,		false, false, true , TSI721_RXPKT_BRG_CNT},
    {rio_sc_rio_brg_rx_err,	false, false, true , TSI721_BRG_PKT_ERR_CNT},
	// The following 6 counters are 'split':  One counter register
	// holds two 16 bit counts.  The "total" count is always the most 
	// significant 16 bits of the counter, and the first counter 
	// specified in this list.  
    {rio_sc_rio_dbel_tx,    	true , true , true , TSI721_ODB_CNTX(0)},
    {rio_sc_rio_dbel_ok_rx,    	true , false, true , TSI721_SPLIT_2ND_CTR},
    {rio_sc_rio_dbel_tx, 	true , true , true , TSI721_ODB_CNTX(1)},
    {rio_sc_rio_dbel_ok_rx,    	true , false, true , TSI721_SPLIT_2ND_CTR},
    {rio_sc_rio_dbel_tx,  	true , true , true , TSI721_ODB_CNTX(2)},
    {rio_sc_rio_dbel_ok_rx,    	true , false, true , TSI721_SPLIT_2ND_CTR},
    {rio_sc_rio_dbel_tx,  	true , true , true , TSI721_ODB_CNTX(3)},
    {rio_sc_rio_dbel_ok_rx,    	true , false, true , TSI721_SPLIT_2ND_CTR},
    {rio_sc_rio_dbel_tx,  	true , true , true , TSI721_ODB_CNTX(4)},
    {rio_sc_rio_dbel_ok_rx,    	true , false, true , TSI721_SPLIT_2ND_CTR},
    {rio_sc_rio_dbel_tx,  	true , true , true , TSI721_ODB_CNTX(5)},
    {rio_sc_rio_dbel_ok_rx,    	true , false, true , TSI721_SPLIT_2ND_CTR},
    {rio_sc_rio_dbel_tx,  	true , true , true , TSI721_ODB_CNTX(6)},
    {rio_sc_rio_dbel_ok_rx,    	true , false, true , TSI721_SPLIT_2ND_CTR},
    {rio_sc_rio_dbel_tx,  	true , true , true , TSI721_ODB_CNTX(7)},
    {rio_sc_rio_dbel_ok_rx,    	true , false, true , TSI721_SPLIT_2ND_CTR},
    {rio_sc_rio_nwr_tx,		true , true , true , TSI721_NWR_CNT},
    {rio_sc_rio_nwr_ok_rx,	true , false, true , TSI721_SPLIT_2ND_CTR},
    {rio_sc_rio_mwr_tx,		true , true , true , TSI721_MWR_CNT},
    {rio_sc_rio_mwr_ok_rx,	true , false, true , TSI721_SPLIT_2ND_CTR}
};

#define TSI721_NUM_PERF_CTRS (sizeof(tsi721_dev_ctrs) / \
			sizeof(struct tsi721_dev_ctr))

uint32_t tsi721_rio_sc_init_dev_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_init_dev_ctrs_in_t *in_parms,
		rio_sc_init_dev_ctrs_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t cntr_i;
	rio_sc_ctr_val_t init_val = INIT_RIO_SC_CTR_VAL;
	struct DAR_ptl good_ptl;
	rio_sc_ctr_val_t *ctrs;

	out_parms->imp_rc = RIO_SUCCESS;

	if (NULL == in_parms->dev_ctrs) {
		out_parms->imp_rc = SC_INIT_DEV_CTRS(0x01);
		goto exit;
	}

	if (NULL == in_parms->dev_ctrs->p_ctrs) {
		out_parms->imp_rc = SC_INIT_DEV_CTRS(0x02);
		goto exit;
	}

	if (!in_parms->dev_ctrs->num_p_ctrs
			|| (in_parms->dev_ctrs->num_p_ctrs > RIO_MAX_PORTS)
			|| (in_parms->dev_ctrs->num_p_ctrs
					< in_parms->dev_ctrs->valid_p_ctrs)) {
		out_parms->imp_rc = SC_INIT_DEV_CTRS(0x03);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if ((RIO_SUCCESS != rc) || (TSI721_MAX_PORTS != good_ptl.num_ports)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = SC_INIT_DEV_CTRS(0x10);
		goto exit;
	}

	ctrs = &in_parms->dev_ctrs->p_ctrs[0].ctrs[0];
	in_parms->dev_ctrs->valid_p_ctrs = TSI721_MAX_PORTS;
	in_parms->dev_ctrs->p_ctrs[0].pnum = 0;
	in_parms->dev_ctrs->p_ctrs[0].ctrs_cnt = TSI721_NUM_PERF_CTRS;
	for (cntr_i = 0; cntr_i < TSI721_NUM_PERF_CTRS; cntr_i++) {
		ctrs[cntr_i] = init_val;
		ctrs[cntr_i].sc = tsi721_dev_ctrs[cntr_i].ctr_t;
		ctrs[cntr_i].tx = tsi721_dev_ctrs[cntr_i].tx;
		ctrs[cntr_i].srio = tsi721_dev_ctrs[cntr_i].srio;
	}

	rc = RIO_SUCCESS;
exit:
	return rc;
}

uint32_t tsi721_rio_sc_read_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_read_ctrs_in_t *in_parms,
		rio_sc_read_ctrs_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t cntr;
	struct DAR_ptl good_ptl;
	rio_sc_ctr_val_t *ctrs;

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
			|| (in_parms->dev_ctrs->num_p_ctrs > RIO_MAX_PORTS)
			|| (in_parms->dev_ctrs->num_p_ctrs
					< in_parms->dev_ctrs->valid_p_ctrs)) {
		out_parms->imp_rc = SC_READ_CTRS(0x03);
		goto exit;
	}

	if (((RIO_ALL_PORTS == in_parms->ptl.num_ports)
			&& (in_parms->dev_ctrs->num_p_ctrs < NUM_PORTS(dev_info)))
			|| ((RIO_ALL_PORTS != in_parms->ptl.num_ports)
					&& (in_parms->dev_ctrs->num_p_ctrs
							< in_parms->ptl.num_ports))) {
		out_parms->imp_rc = SC_READ_CTRS(0x04);
		goto exit;
	}

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if ((RIO_SUCCESS != rc) || (TSI721_MAX_PORTS != good_ptl.num_ports)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = SC_READ_CTRS(0x10);
		goto exit;
	}

	// There's only one port, and one set of counters...
	ctrs = &in_parms->dev_ctrs->p_ctrs[0].ctrs[0];
	for (cntr = 0; cntr < TSI721_NUM_PERF_CTRS; cntr++) {
		uint32_t cnt, split_cnt;
		if (tsi721_dev_ctrs[cntr].split && !tsi721_dev_ctrs[cntr].os) {
			continue;
		}

		rc = DARRegRead(dev_info, tsi721_dev_ctrs[cntr].os, &cnt);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = SC_READ_CTRS(0x20 + cntr);
			goto exit;
		}

		if (!tsi721_dev_ctrs[cntr].split) {
			ctrs[cntr].last_inc = cnt;
			ctrs[cntr].total += cnt;
			continue;
		}
		split_cnt = (cnt & TSI721_MWR_CNT_MW_TOT_CNT) >> 16;
		cnt &= TSI721_MWR_CNT_MW_OK_CNT;
		ctrs[cntr].last_inc = split_cnt;
		ctrs[cntr].total += split_cnt;
		ctrs[cntr + 1].last_inc = cnt;
		ctrs[cntr + 1].total += cnt;
	}

	rc = RIO_SUCCESS;

exit:
	return rc;
}

#endif /* TSI721_DAR_WANTED */

#ifdef __cplusplus
}
#endif
