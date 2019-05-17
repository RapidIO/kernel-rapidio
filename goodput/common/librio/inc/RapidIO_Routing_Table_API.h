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

#ifndef __RAPIDIO_ROUTING_TABLE_API_H__
#define __RAPIDIO_ROUTING_TABLE_API_H__

#include <stdint.h>
#include <stdbool.h>

#include "rio_route.h"
#include "rio_ecosystem.h"
#include "RapidIO_Device_Access_Routines_API.h"

#ifdef __cplusplus
extern "C" {
#endif

// This file is structured as:
// - Constant definitions
// - Input and output parameter structures for each routine
// - List of routines for routing table support

typedef enum { tt_dev8, tt_dev16 } tt_t;

typedef enum rio_rt_disc_reason_t_TAG
{
	// Packet is not discarded
	rio_rt_disc_not = 0,

	// Invalid value in routing table
	rio_rt_disc_rt_invalid = 1,

	// Routing table selects port discard
	rio_rt_disc_deliberately = 2,

	// Target port is unavailable
	rio_rt_disc_port_unavail = 3,

	// Target port is powered down
	rio_rt_disc_port_pwdn = 4,

	// Target port has PORT_ERR condition
	rio_rt_disc_port_fail = 5,

	// Target port does not have PORT_OK status
	rio_rt_disc_port_no_lp = 6,

	// Target port LOCKOUT or PORT_DISABLE bits are set
	rio_rt_disc_port_lkout_or_dis = 7,

	// Target port input/output enable bits are clear
	rio_rt_disc_port_in_out_dis = 8,

	// DestID is multicast with an empty mask
	rio_rt_disc_mc_empty = 9,

	// DestID is multicast with a mask empty except for this port
	rio_rt_disc_mc_one_bit = 0xA,

	// Some devices allow multiple masks to be mapped to the same device ID
	rio_rt_disc_mc_mult_masks = 0xB,

	// Default route port is invalid
	rio_rt_disc_dflt_pt_invalid = 0xC,

	// Default route port deliberately discards packets
	rio_rt_disc_dflt_pt_deliberately = 0x0D,

	// Default route port is unavailable
	rio_rt_disc_dflt_pt_unavail = 0x0E,

	rio_rt_disc_dflt_pt_pwdn = 0x0F,
	// Default route port is powered down

	// Default route port has PORT_ERR condition
	rio_rt_disc_dflt_pt_fail = 0x10,

	// Default route port does not have PORT_OK status
	rio_rt_disc_dflt_pt_no_lp = 0x11,

	// Default route port LOCKOUT or PORT_DISABLE bits are set
	rio_rt_disc_dflt_pt_lkout_or_dis = 0x12,

	// Default route port input/output enable bits are clear
	rio_rt_disc_dflt_pt_in_out_dis = 0x13,

	// An implementation specific reason causes the packet discard
	rio_rt_disc_imp_spec = 0x14,

	// Probe aborted due to routing error
	rio_rt_disc_probe_abort = 0x15,

	// Last entry...
	rio_rt_disc_last = 0x15,

	// DEPRECATED
	rio_rt_disc_dflt_pt_used = 0x16,
} rio_rt_disc_reason_t;

extern char *rio_em_disc_reason_names[ (uint8_t)(rio_rt_disc_last) ];

#define DISC_REASON_STR(x) ((x < (uint8_t)(rio_rt_disc_last))?(rio_em_disc_reason_names[x]):"OORange")

typedef struct rio_rt_mc_info_t_TAG {
	// Destination ID of packets to be multicast
	did_reg_t mc_destID;

	// Size of mc_destID, either 8 or 16 bit.
	tt_t tt;

	// Bit vector of ports.
	//   Least significant bit is port 0,
	//   Most significant bit is port 31.
	uint32_t mc_mask;

	// true if this multicast mask and mc_destID are in use.
	bool in_use;

	// true if this mask has been allocated by rio_rt_alloc_mc_mask.
	// Otherwise, should be ignored.
	bool allocd;

	// true if the mc_destID or mc_mask value has changed.
	bool changed;
} rio_rt_mc_info_t;

typedef struct rio_rt_uc_info_t_TAG {
	// Routing table entry value.
	uint32_t rte_val;

	// true if the rte_val value has changed.
	bool changed;
} rio_rt_uc_info_t;

#define MC_MASK_ADD_PORT(m,p) (m |  (uint32_t)(1 << p))
#define MC_MASK_REM_PORT(m,p) (m & ~(uint32_t)(1 << p))
#define MC_MASK_GOT_PORT(m,p) (m &  (uint32_t)(1 << p))

typedef struct rio_rt_state_t_TAG {
	// The 'default route' for ports routed using RIO_RTE_DFLT_PORT
	uint32_t default_route;

	// Encoded routing table value, device specific restrictions
	// may apply.
	rio_rt_uc_info_t dev_table[RIO_RT_GRP_SZ];
	rio_rt_uc_info_t dom_table[RIO_RT_GRP_SZ];

	rio_rt_mc_info_t mc_masks[RIO_MAX_MC_MASKS];
} rio_rt_state_t;

typedef struct rio_rt_initialize_in_t_TAG {
	// Must be a valid port number, or RIO_ALL_PORTS
	// Note that when RIO_ALL_PORTS is specified,  this function also
	//   clears all multicast masks and removes all associations between
	//   multicast masks and ports.
	uint8_t set_on_port;

	// Routing control for RIO_DSF_RT_DEFAULT_ROUTE routing table value.
	//   Must be a valid port number, or RIO_DSF_RT_NO_ROUTE
	uint32_t default_route;

	// Select the default routing for every destination ID in the routing table
	// Can be one of: a valid port number, RIO_DSF_RT_NO_ROUTE, or
	//   RIO_DSF_RT_USE_DEFAULT_ROUTE
	uint32_t default_route_table_port;

	// true : Update hardware state
	// false: Do not update hardware state
	bool update_hw;

	// Optionally provide a pointer to an rio_rt_state_t structure.
	//   If provided, the structure is initialized to match the requested initial routing table.
	rio_rt_state_t *rt;
} rio_rt_initialize_in_t;

typedef struct rio_rt_initialize_out_t_TAG
{
	// Implementation specific failure information
	uint32_t imp_rc;

} rio_rt_initialize_out_t;

typedef struct rio_rt_probe_in_t_TAG {
	uint8_t probe_on_port;
	// Must be a valid port number, or RIO_ALL_PORTS
	// RIO_ALL_PORTS probes the global routing table.

	// DestID size (8 bit or 16 bit)
	tt_t tt;

	// Check routing for specified device ID.
	did_reg_t destID;

	// A pointer to an rio_rt_state_t structure.
	//   This structure is used to determine the route.
	rio_rt_state_t *rt;
} rio_rt_probe_in_t;

typedef struct rio_rt_probe_out_t_TAG {
	//  Output fields necessary for RIO_DAR_RT_PROBE function

	// Implementation specific failure information
	uint32_t imp_rc;
	// true : packets will exit the port as defined by the
	//	routing_table_value.
	// false: Packets will be discarded as indicated by reason_for_discard.
	bool valid_route;

	// Encoded routing table value read
	pe_rt_val routing_table_value;

	// When routing_table_value is RIO_RTE_DFLT_PORT, this field
	//   contains the value of the default route register.
	pe_rt_val default_route;

	// If true, packets that match the FILTER MASK will be dropped.
	bool filter_function_active;

	// If true, packets that match the TRACE MASK will be copied to the
	// trace port.
	bool trace_function_active;

	// If true, packets buffered for longer than the time-to-live period
	// may be discarded.
	bool time_to_live_active;

	// True if packet will be multicast to this port, false if not.
	// For completeness, will return true for the port which was queried!
	// Only valid if the routing_table_value indicates a multicast mask.
	bool mcast_ports[RIO_MAX_PORTS];

	// Encoding for the reason that packets will be discarded.
	// Only valid if valid_route is false.
	rio_rt_disc_reason_t reason_for_discard;
} rio_rt_probe_out_t;


typedef struct rio_rt_probe_all_in_t_TAG {
	// Must be a valid port number, or RIO_ALL_PORTS. RIO_ALL_PORTS probes
	// the global routing table, which may not be consistent with per-port
	// routing tables.  No warning is given of inconsistency.
	uint8_t probe_on_port;

	// Pointer to routing table state structure.
	rio_rt_state_t *rt;
} rio_rt_probe_all_in_t;

typedef struct rio_rt_probe_all_out_t_TAG
{
	// Implementation specific failure information
	uint32_t imp_rc;
} rio_rt_probe_all_out_t;

typedef struct rio_rt_set_all_in_t_TAG
{
	// A valid port number, or RIO_ALL_PORTS. RIO_ALL_PORTS selects the
	// global routing table
	uint8_t set_on_port;

	// Pointer to routing table state structure. The state structure could
	// be initialized using rio_rt_probe_all, modified using
	// rio_rt_change_rte, rio_rt_alloc_mc_mask, rio_rt_dealloc_mc_mask, or
	// rio_rt_change_mc_mask, and then applied to hardware using
	// rio_rt_set_all.
	rio_rt_state_t *rt;
} rio_rt_set_all_in_t;

typedef struct rio_rt_set_all_out_t_TAG
{
	// Implementation specific failure information
	uint32_t imp_rc;
} rio_rt_set_all_out_t;

typedef rio_rt_set_all_in_t  rio_rt_set_changed_in_t;
typedef rio_rt_set_all_out_t rio_rt_set_changed_out_t;

typedef struct rio_rt_alloc_mc_mask_in_t_TAG
{
	// Pointer to routing table state structure
	rio_rt_state_t *rt;
} rio_rt_alloc_mc_mask_in_t;

typedef struct rio_rt_alloc_mc_mask_out_t_TAG {
	// Implementation specific failure information
	uint32_t imp_rc;

	// Routing table value which selects the allocated multicast mask.
	//   If no free multicast masks exist, set to RIO_DSF_BAD_MC_MASK.
	uint32_t mc_mask_rte;
} rio_rt_alloc_mc_mask_out_t;

typedef struct rio_rt_dealloc_mc_mask_in_t_TAG {
	// Multicast mask routing value to be removed from the routing table
	// state pointed to by "rt". The multicast mask is also cleared to 0 by
	// this routine.
	uint32_t mc_mask_rte;

	// Pointer to routing table state structure to be updated
	rio_rt_state_t *rt;
} rio_rt_dealloc_mc_mask_in_t;

typedef struct rio_rt_dealloc_mc_mask_out_t_TAG {
	// Implementation specific failure information
	uint32_t imp_rc;
} rio_rt_dealloc_mc_mask_out_t;

typedef struct rio_rt_change_rte_in_t_TAG {
	// true  if domain routing table entry is being updated
	// false if device routing table entry is being update
	bool dom_entry;

	// Index of routing table entry to be updated
	uint8_t idx;

	// Value for the routing table entry
	//  - Note that if the requested routing table entry matches the
	// 	routing table entry value in *rt, the routing table entry
	// 	status is "no change"
	uint32_t rte_value;

	// Pointer to routing table state structure to be updated
	rio_rt_state_t *rt;
} rio_rt_change_rte_in_t;

typedef struct rio_rt_change_rte_out_t_TAG {
	// Implementation specific failure information
	uint32_t imp_rc;
} rio_rt_change_rte_out_t;

typedef struct rio_rt_change_mc_mask_in_t_TAG {
	// Multicast mask routing value which identifies the
	//    mask to be modified.
	uint32_t mc_mask_rte;

	// Multicast information to be assigned to associated multicast entry
	rio_rt_mc_info_t mc_info;

	// Pointer to routing table state structure to be updated
	rio_rt_state_t *rt;
} rio_rt_change_mc_mask_in_t;

typedef struct rio_rt_change_mc_mask_out_t_TAG {
	// Implementation specific failure information
	uint32_t imp_rc;
} rio_rt_change_mc_mask_out_t;


// Implementation specific return code starting numbers for each
// standard routine.  These are the "base numbers" for the "imp_rc"
// fields in the return code structures.  
//
// RT_FIRST_SUBROUTINE_0 is the first "base number" to be used
// for implementation specific subroutines complex enough to
// warrant their own implementation specific return codes.

#define RT_INITIALIZE_0       (DAR_FIRST_IMP_SPEC_ERROR+0x1000)
#define RT_PROBE_0            (DAR_FIRST_IMP_SPEC_ERROR+0x1100)
#define RT_PROBE_ALL_0        (DAR_FIRST_IMP_SPEC_ERROR+0x1200)
#define RT_SET_ALL_0          (DAR_FIRST_IMP_SPEC_ERROR+0x1300)
#define RT_SET_CHANGED_0      (DAR_FIRST_IMP_SPEC_ERROR+0x1400)
#define RT_ALLOC_MC_MASK_0    (DAR_FIRST_IMP_SPEC_ERROR+0x1500)
#define RT_DEALLOC_MC_MASK_0  (DAR_FIRST_IMP_SPEC_ERROR+0x1600)
#define RT_CHANGE_RTE_0       (DAR_FIRST_IMP_SPEC_ERROR+0x1700)
#define RT_CHANGE_MC_MASK_0   (DAR_FIRST_IMP_SPEC_ERROR+0x1800)
#define RT_FIRST_SUBROUTINE_0 (DAR_FIRST_IMP_SPEC_ERROR+0x100000)

/* initializes the routing table hardware and/or routing table state structure. */
#define RT_INITIALIZE(x) (RT_INITIALIZE_0+x)
uint32_t rio_rt_initialize(DAR_DEV_INFO_t *dev_info,
		rio_rt_initialize_in_t *in_parms,
		rio_rt_initialize_out_t *out_parms);

/* This function probes the hardware status of a routing table entry for 
 *   the specified port and destination ID
 */

#define RT_PROBE(x) (RT_PROBE_0+x)
uint32_t rio_rt_probe(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_in_t *in_parms,
		rio_rt_probe_out_t *out_parms);

/* This function returns the complete hardware state of packet routing
 * in a routing table state structure.
 *
 * The routing table hardware must be initialized using rio_rt_initialize() 
 * before calling this routine.
 */
#define RT_PROBE_ALL(x) (RT_PROBE_ALL_0+x)
uint32_t rio_rt_probe_all(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_all_in_t *in_parms,
		rio_rt_probe_all_out_t *out_parms);

/* This function sets the routing table hardware to match every entry
 *    in the routing table state structure. 
 * After rio_rt_set_all is called, no entries are marked as changed in
 *    the routing table state structure.
 */
#define RT_SET_ALL(x) (RT_SET_ALL_0+x)
uint32_t rio_rt_set_all(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_all_in_t *in_parms,
		rio_rt_set_all_out_t *out_parms);

/* This function sets the the routing table hardware to match every entry
 *    that has been changed in the routing table state structure. 
 * Changes must be made using rio_rt_alloc_mc_mask, rio_rt_deallocate_mc_mask,
 *    rio_rt_change_rte, and rio_rt_change_mc.
 * After rio_rt_set_changed is called, no entries are marked as changed in
 *    the routing table state structure.
 */
#define RT_SET_CHANGED(x) (RT_SET_CHANGED_0+x)
uint32_t rio_rt_set_changed(DAR_DEV_INFO_t *dev_info,
		rio_rt_set_changed_in_t *in_parms,
		rio_rt_set_changed_out_t *out_parms);

/* This function updates an rio_rt_state_t structure to
 * find the first previously unused multicast mask.  
 * Can be called consecutively to allocate multiple 
 * multicast masks.
 */
#define RT_ALLOC_MC_MASK(x) (RT_ALLOC_MC_MASK_0+x)
uint32_t rio_rt_alloc_mc_mask(DAR_DEV_INFO_t *dev_info,
		rio_rt_alloc_mc_mask_in_t *in_parms,
		rio_rt_alloc_mc_mask_out_t *out_parms);

/* This function updates an rio_rt_state_t structure to
 * deallocate a specified multicast mask.  Routing tables
 * are updated to remove all references to the multicast mask.
 * After deallocation, the hardware state must be updated by
 * calling rio_rt_set_all() or rio_rt_set_changed().
 */
#define RT_DEALLOC_MC_MASK(x) (RT_DEALLOC_MC_MASK_0+x)
uint32_t rio_rt_dealloc_mc_mask(DAR_DEV_INFO_t *dev_info,
		rio_rt_dealloc_mc_mask_in_t *in_parms,
		rio_rt_dealloc_mc_mask_out_t *out_parms);

/* This function updates an rio_rt_state_t structure to
 * change a routing table entry, and tracks changes.
 *
 * NOTE: The Tsi57x family allows only a single domain
 * table entry to have a value of RIO_DSF_RT_USE_DEVICE_TABLE.
 * This limitation is supported by this routine. 
 *
 * NOTE: The CPS family FORCES domain table entry 0 
 * to have a value of RIO_DSF_RT_USE_DEVICE_TABLE.
 * This limitation is supported by this routine. 
 */
#define RT_CHANGE_RTE(x) (RT_CHANGE_RTE_0+x)
uint32_t rio_rt_change_rte(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_rte_in_t *in_parms,
		rio_rt_change_rte_out_t *out_parms);

/* This function updates an rio_rt_state_t structure to
 * change a multicast mask value, and tracks changes.
 */
#define CHANGE_MC_MASK(x) (RT_CHANGE_MC_MASK_0+x)
uint32_t rio_rt_change_mc_mask(DAR_DEV_INFO_t *dev_info,
		rio_rt_change_mc_mask_in_t *in_parms,
		rio_rt_change_mc_mask_out_t *out_parms);

void rio_rt_check_multicast_routing(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_in_t *in_parms, rio_rt_probe_out_t *out_parms);

// Determines route, and then determines packet discard based purely
// on the settings of the routing table.
void rio_rt_check_unicast_routing(DAR_DEV_INFO_t *dev_info,
		rio_rt_probe_in_t *in_parms, rio_rt_probe_out_t *out_parms);

#ifdef __cplusplus
}
#endif

#endif /* __RAPIDIO_ROUTING_TABLE_API_H__ */
