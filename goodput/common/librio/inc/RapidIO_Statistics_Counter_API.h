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

#ifndef __RAPIDIO_STATISTICS_COUNTER_API_H__
#define __RAPIDIO_STATISTICS_COUNTER_API_H__

#include <stdint.h>
#include <stdbool.h>

#include "rio_route.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Definitions of parameter structures for IDT Port Configuration routines.

   Generic structure which contains the parameters which describe the
   configuration of a port.
*/

typedef enum rio_sc_ctr_t_TAG {
	// The counter is disabled.
	rio_sc_disabled,

	// The counter is enabled.
	rio_sc_enabled,

	// START OF TSI57X SPECIFIC COUNTERS

	// Tsi57x Unicast request packets
	//   Excludes response packets, maintenance packets,
	//   maintenance packets with hop count of 0,
	//   and packets which are multicast.
	rio_sc_uc_req_pkts,

	// Tsi57x Unicast packets.
	//   Excludes all packets which are multicast.
	rio_sc_uc_pkts,

	// Count retry control symbols
	rio_sc_retries,

	// Excludes retries, and Status+NOP control symbols
	rio_sc_all_cs,

	// Count of multiple of words (4 bytes) of packet data
	//    for unicast packets.  Excludes multicast packets.
	rio_sc_uc_4b_data,

	// Count of multicast packets.
	//   Excludes all packets which are unicast.
	rio_sc_mc_pkts,

	// Count of Multicast Event Control Symbols
	rio_sc_mecs,

	// Count of multiple of words (4 bytes) of packet data
	//    for multicast packets.  Excludes unicast packets.
	rio_sc_mc_4b_data,

	// END OF TSI57X SPECIFIC COUNTERS

	// CPS Packet acknowledgements count (TX or RX)
	rio_sc_pa,

	// CPS Packets count (TX or RX)
	rio_sc_pkt,

	// CPS Packet negative acknowledgements count (RX only)
	rio_sc_pna,

	// CPS Packets dropped count (TX or RX)
	rio_sc_pkt_drop,

	// CPS RX Packets dropped count due to TTL (TX only)
	rio_sc_pkt_drop_ttl,

	// START OF RXS2448 SPECIFIC PERFORMANCE COUNTERS

	// Number of packets received/transmitted on the fabric
	rio_sc_fab_pkt,

	// Count of packet payload received/transmitted on the RapidIO interface
	rio_sc_rio_pload,

	// Count of packet payload received/transmitted on the fabric
	rio_sc_fab_pload,

	// Count of the total number of code-groups/codewords
	// transmitted on the RapidIO interface per lane
	rio_sc_rio_bwidth,

	// END OF RXS2448 SPECIFIC PERFORMANCE COUNTERS
	// START OF TSI721 PERFORMANCE COUNTERS

	// Received Completion Count for Messaging Engine Register.
	rio_sc_pcie_msg_rx,

	// Sent TLP Count of Messaging Engine Register
	rio_sc_pcie_msg_tx,

	// Received Completion Count for Block DMA Engine Register
	rio_sc_pcie_dma_rx,

	// Sent TLP Count of Block DMA Engine Register
	rio_sc_pcie_dma_tx,

	// Received Bridging TLP Count Register
	rio_sc_pcie_brg_rx,

	// Sent Bridging TLP Count Register
	rio_sc_pcie_brg_tx,

	// NWRITE_R Total Count Register
	rio_sc_rio_nwr_tx,

	// NWRITE_R RX OK Count Register
	rio_sc_rio_nwr_ok_rx,

	// Total outbound doorbells total sent
	rio_sc_rio_dbel_tx,

	// Total outbound doorbells responses OK
	rio_sc_rio_dbel_ok_rx,

	// Sent Packet Count of Messaging Engine Register
	rio_sc_rio_msg_tx,

	// Received Packet Count for Messaging Engine Register
	rio_sc_rio_msg_rx,

	// Generated Message Segment Retry Count Register
	rio_sc_rio_msg_tx_rty,

	// Received Retry Message Response Count Register
	rio_sc_rio_msg_rx_rty,

	// Sent Packet Count of Block DMA Engine Register
	rio_sc_rio_dma_tx,

	// Received Response Count for Block DMA Engine Register
	rio_sc_rio_dma_rx,

	// Sent Bridging Packet Count Register
	rio_sc_rio_brg_tx,

	// Received Bridging Packet Count Register
	rio_sc_rio_brg_rx,

	// Received Bridging Packet Error Count Register
	rio_sc_rio_brg_rx_err,

	// Maintenance Write Total Count Register
	rio_sc_rio_mwr_tx,

	// Maintenance Write OK Count Register
	rio_sc_rio_mwr_ok_rx,

	// END OF TSI721 PERFORMANCE COUNTERS

	// Last index for enumerated type
	rio_sc_last
} rio_sc_ctr_t;

typedef enum rio_sc_ctr_flag_t_TAG {
	sc_f_DROP	= 0,
	sc_f_ERR	= 1,
	sc_f_RTY	= 2,
	sc_f_CS		= 3,
	sc_f_PKT	= 4,
	sc_f_DATA	= 5,
	sc_f_LAST	= 6,
} rio_sc_ctr_flag_t;

#define SC_FLAG_NAMES "DROP ERR RTY CS PKT DATA "

#define SC_F_DROP (1 << (uint32_t)(sc_f_DROP))
#define SC_F_ERR  (1 << (uint32_t)(sc_f_ERR ))
#define SC_F_RTY  (1 << (uint32_t)(sc_f_RTY ))
#define SC_F_CS   (1 << (uint32_t)(sc_f_CS  ))
#define SC_F_PKT  (1 << (uint32_t)(sc_f_PKT ))
#define SC_F_DATA (1 << (uint32_t)(sc_f_DATA))

typedef struct rio_sc_info_t_TAG {
	char *name;
	uint32_t flags;
} sc_info_t;

extern sc_info_t sc_info[(uint8_t)(rio_sc_last)+2];
#define SC_NAME(x) ((x<=rio_sc_last)? \
		sc_info[x].name : sc_info[(uint8_t)(rio_sc_last)+1].name)
#define SC_FLAG(x) ((x<=rio_sc_last)? \
		sc_info[x].flags : sc_info[(uint8_t)(rio_sc_last)+1].flags)

extern char *sc_flag_names[(uint8_t)(sc_f_LAST)+2];
#define SC_FLAG_NAME(x) ((x<=sc_f_LAST)? \
		sc_flag_names[x]:sc_flag_names[(uint8_t)(sc_f_LAST)+1])

#define SC_GEN_FLAG_NAMES "TX RX SRIO OTH "
#define SC_F_TX   0
#define SC_F_RX   1
#define SC_F_SRIO 2
#define SC_F_OTH  3

extern uint32_t rio_sc_other_if_names(DAR_DEV_INFO_t *dev_h, const char **name);

#define DIR_TX true
#define DIR_RX !DIR_TX
#define DIR_SRIO true
#define DIR_FAB  !DIR_SRIO

typedef uint8_t prio_mask_g1_t;

// Gen1 definitions for priority
#define SC_PRIO_MASK_G1_0	((prio_mask_g1_t)(0x01))
#define SC_PRIO_MASK_G1_1	((prio_mask_g1_t)(0x02))
#define SC_PRIO_MASK_G1_2	((prio_mask_g1_t)(0x04))
#define SC_PRIO_MASK_G1_3	((prio_mask_g1_t)(0x08))
#define SC_PRIO_MASK_G1_ALL	((prio_mask_g1_t)(0x0F))

typedef uint8_t prio_mask_t;
#define SC_PRIO_MASK_0	((prio_mask_t)(0x01))
#define SC_PRIO_MASK_0C	((prio_mask_t)(0x02))
#define SC_PRIO_MASK_1	((prio_mask_t)(0x04))
#define SC_PRIO_MASK_1C	((prio_mask_t)(0x08))
#define SC_PRIO_MASK_2	((prio_mask_t)(0x10))
#define SC_PRIO_MASK_2C	((prio_mask_t)(0x20))
#define SC_PRIO_MASK_3	((prio_mask_t)(0x40))
#define SC_PRIO_MASK_3C	((prio_mask_t)(0x80))
#define SC_PRIO_MASK_ALL ((prio_mask_t)(0xFF))

typedef struct rio_sc_ctr_val_t_TAG {
	// Accumulated counter value since counter was
	//   enabled/configured
	long long total;

	// Value the counter increased since previous read.
	uint32_t last_inc;

	// What is being counted
	//    May be modified by device specific configuration routines,
	//    as counters are configured/enabled/disabled
	//    The fields "total" and "last_inc" are 0 when sc == rio_sc_dir
	rio_sc_ctr_t sc;

	// true : transmitted "sc" are being counted.
	// false: received    "sc" are being counted.
	bool tx;

	// true : Counter type reflects information on the RapidIO interface
	// false: Counter type reflects information on the internal fabric interface
	bool srio;
} rio_sc_ctr_val_t;

#define INIT_RIO_SC_CTR_VAL {0, 0, rio_sc_disabled, false, true} 

typedef struct rio_sc_p_ctrs_val_t_TAG {
	// Port number for these counters
	uint8_t pnum;

	// Number of valid entries in ctrs.
	//    Device specific.
	uint8_t ctrs_cnt;

	// Counter values for the device
	rio_sc_ctr_val_t ctrs[RIO_MAX_SC];
} rio_sc_p_ctrs_val_t;

typedef struct rio_sc_dev_ctrs_t_TAG {
	// Number of allocated entries in p_ctrs[],
	//    Maximum value is RIO_MAX_PORTS
	uint8_t num_p_ctrs;

	// Number of valid entries in p_ctrs[],
	//    Maximum value is num_p_ctrs;
	// Initialized by rio_sc_init_dev_ctrs()...
	uint8_t valid_p_ctrs;

	// Location of performance counters structure array
	rio_sc_p_ctrs_val_t *p_ctrs;
} rio_sc_dev_ctrs_t;

typedef struct rio_sc_init_dev_ctrs_in_t_TAG {
	// Port list
	struct DAR_ptl ptl;

	// Device performance counters state
	rio_sc_dev_ctrs_t *dev_ctrs;
} rio_sc_init_dev_ctrs_in_t;

typedef struct rio_sc_init_dev_ctrs_out_t_TAG {
	// Implementation specific return code information.
	uint32_t imp_rc;
} rio_sc_init_dev_ctrs_out_t;

typedef struct rio_sc_read_ctrs_in_t_TAG {
	// Port list
	struct DAR_ptl ptl;

	// Device performance counters.
	rio_sc_dev_ctrs_t *dev_ctrs;
} rio_sc_read_ctrs_in_t;

typedef struct rio_sc_read_ctrs_out_t_TAG {
	// Implementation specific return code information.
	uint32_t imp_rc;
} rio_sc_read_ctrs_out_t;

typedef struct rio_sc_cfg_tsi57x_ctr_in_t_TAG {
	// Port list
	struct DAR_ptl ptl;

	// Index of the Tsi57x counter to be configured.  Range 0-5.
	uint8_t ctr_idx;

	// Priority of packets to be counted.
	// Not used for control symbol counters.
	// Uses SC_PRIO_MASK_G1_x constant definitions.
	prio_mask_g1_t prio_mask;

	// Determines direction for the counter.  !tx = rx.
	bool tx;

	// Valid counter type, valid range from rio_sc_disabled to rio_sc_uc_4b_data
	rio_sc_ctr_t ctr_type;

	// Device counters data type, initialized by rio_sc_init_dev_ctrs
	rio_sc_dev_ctrs_t *dev_ctrs;
} rio_sc_cfg_tsi57x_ctr_in_t;

typedef struct rio_sc_cfg_tsi57x_ctr_out_t_TAG {
	// Implementation specific return code information.
	uint32_t imp_rc;
} rio_sc_cfg_tsi57x_ctr_out_t;

typedef struct rio_sc_cfg_cps_ctrs_in_t_TAG {
	// Port list
	struct DAR_ptl ptl;

	// true - enable all counters, false - disable all counters
	bool enable_ctrs;

	// Device counters data type, initialized by rio_sc_init_dev_ctrs
	rio_sc_dev_ctrs_t *dev_ctrs;
} rio_sc_cfg_cps_ctrs_in_t;

typedef struct rio_sc_cfg_cps_ctrs_out_t_TAG {
	// Implementation specific return code information.
	uint32_t imp_rc;
} rio_sc_cfg_cps_ctrs_out_t;

typedef struct rio_sc_cfg_rxs_ctr_in_t_TAG {
	// Port list.
	struct DAR_ptl ptl;

	// Index of the RXS counter [0..7] to be configured.
	uint8_t ctr_idx;

	// Packet priority, use SC_PRIO_MASK_x consts
	prio_mask_t prio_mask;

	// Enable/disable port counters
	bool ctr_en;

	// Determines direction for the counter.  !tx = rx.
	bool tx;

	// What to count
	rio_sc_ctr_t ctr_type;

	// Initialized by rio_sc_init_dev_ctrs
	rio_sc_dev_ctrs_t *dev_ctrs;
} rio_sc_cfg_rxs_ctr_in_t;

typedef struct rio_sc_cfg_rxs_ctr_out_t_TAG
{
	uint32_t imp_rc; // Implementation specific return code information.
} rio_sc_cfg_rxs_ctr_out_t;


// Implementation specific return codes for Statistics Counter routines

#define SC_INIT_DEV_CTRS_0    (DAR_FIRST_IMP_SPEC_ERROR+0x0100)
#define SC_READ_CTRS_0        (DAR_FIRST_IMP_SPEC_ERROR+0x0200)

#define SC_CFG_TSI57X_CTR_0   (DAR_FIRST_IMP_SPEC_ERROR+0x0300)
#define SC_CFG_CPS_CTRS_0     (DAR_FIRST_IMP_SPEC_ERROR+0x0400)
#define SC_CFG_CPS_TRACE_0    (DAR_FIRST_IMP_SPEC_ERROR+0x0500)

#define SC_INIT_RXS_CTRS_0    (DAR_FIRST_IMP_SPEC_ERROR+0x0600)
#define SC_READ_RXS_CTRS_0    (DAR_FIRST_IMP_SPEC_ERROR+0x0700)
#define SC_CFG_RXS_CTR_0      (DAR_FIRST_IMP_SPEC_ERROR+0x0800)

#define SC_INIT_RXS_CTRS(x) (SC_INIT_RXS_CTRS_0+x)
#define SC_READ_RXS_CTRS(x) (SC_READ_RXS_CTRS_0+x)

/* The following functions are implemented to support the above structures
 * Refer to the above structures for the implementation detail
 */

/* This function initializes an rio_sc_dev_ctrs structure based
 * on input parameters and the current hardware state.
 */
#define SC_INIT_DEV_CTRS(x) (SC_INIT_DEV_CTRS_0+x)

uint32_t rio_sc_init_dev_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_init_dev_ctrs_in_t *in_parms,
		rio_sc_init_dev_ctrs_out_t *out_parms);

/* Reads enabled/configured counters on selected ports */
#define SC_READ_CTRS(x) (SC_READ_CTRS_0+x)
uint32_t rio_sc_read_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_read_ctrs_in_t *in_parms,
		rio_sc_read_ctrs_out_t *out_parms);

/* Configure counters on selected ports of a Tsi device. */
#define SC_CFG_TSI57X_CTR(x) (SC_CFG_TSI57X_CTR_0+x)
uint32_t rio_sc_cfg_tsi57x_ctr(DAR_DEV_INFO_t *dev_info,
		rio_sc_cfg_tsi57x_ctr_in_t *in_parms,
		rio_sc_cfg_tsi57x_ctr_out_t *out_parms);

/* Configure counters on selected ports of a CPS device. */
#define SC_CFG_CPS_CTRS(x) (SC_CFG_CPS_CTRS_0+x)
uint32_t rio_sc_cfg_cps_ctrs(DAR_DEV_INFO_t *dev_info,
		rio_sc_cfg_cps_ctrs_in_t *in_parms,
		rio_sc_cfg_cps_ctrs_out_t *out_parms);


/* Configure counters on selected ports of an RXS device. */
#define SC_CFG_RXS_CTRS(x)  (SC_CFG_RXS_CTR_0+x)
uint32_t rio_sc_cfg_rxs_ctr(DAR_DEV_INFO_t *dev_info,
		rio_sc_cfg_rxs_ctr_in_t *in_parms,
		rio_sc_cfg_rxs_ctr_out_t *out_parms);

#ifdef __cplusplus
}
#endif

#endif /* __RAPIDIO_STATISTICS_COUNTER_API_H__ */
