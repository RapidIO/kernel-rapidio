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

#ifndef __RAPIDIO_ERROR_MANAGEMENT_API_H__
#define __RAPIDIO_ERROR_MANAGEMENT_API_H__

#include <stdint.h>
#include <stdbool.h>

#include "rio_standard.h"
#include "RapidIO_Routing_Table_API.h"

#ifdef __cplusplus
extern "C" {
#endif

// Parameters to control where port-writes will be sent.
// rio_em_cfg_pw must be called before enabling any port-write
// reporting.

#define PORT_WRITE_RE_TX_NSEC 352

typedef struct rio_em_cfg_pw_t_TAG {
	// Implementation specific return code, useful for debug.
	// Only valid on output, not useful for input.
	uint32_t imp_rc;

	// tt_dev16 if this is a 16-bit destID,
	// tt_dev8  if this is an 8-bit destID.
	tt_t deviceID_tt;

	// Where to send the port-write
	did_reg_t port_write_destID;

	// True if the sourceID can be set on this device,
	// and should be set on this device.
	bool srcID_valid;

	// Control of the source ID of the port-write,
	// which is a reserved field.
	did_reg_t port_write_srcID;

	// Priority of the port-write.
	// Value is 0-3. Generally should be 3.
	uint8_t priority;

	// Set Critical Request Flow bit for port-write priority
	// Not available on all devices.
	// Should be set whenever possible.
	bool CRF;

	// Controls the rate at which port-writes
	// are retransmitted.  Max value is 0xFFFFFF.
	// Granularity is 352 nsec.
	// 0 means retransmission is disabled, and usually means
	//   one port write is received for each event.
	// Some devices restrict the retransmit granularity.
	// Refer to definitions of constants below.
	uint32_t port_write_re_tx;
} rio_em_cfg_pw_t;

typedef rio_em_cfg_pw_t rio_em_cfg_pw_in_t; 
typedef rio_em_cfg_pw_t rio_em_cfg_pw_out_t; 

#define RIO_EM_PW_RE_TX_DISABLED       (0)

#define RIO_EM_TSI578_PW_RE_TX_167P7MS (167700000/PORT_WRITE_RE_TX_NSEC)
#define RIO_EM_TSI578_PW_RE_TX_335P5MS (335500000/PORT_WRITE_RE_TX_NSEC)
#define RIO_EM_TSI578_PW_RE_TX_671P1MS (671100000/PORT_WRITE_RE_TX_NSEC)
#define RIO_EM_TSI578_PW_RE_TX_1340MS (1340000000/PORT_WRITE_RE_TX_NSEC)

#define RIO_EM_TSI721_PW_RE_TX_103US (103000/PORT_WRITE_RE_TX_NSEC)
#define RIO_EM_TSI721_PW_RE_TX_205US (205000/PORT_WRITE_RE_TX_NSEC)
#define RIO_EM_TSI721_PW_RE_TX_410US (410000/PORT_WRITE_RE_TX_NSEC)
#define RIO_EM_TSI721_PW_RE_TX_820US (820000/PORT_WRITE_RE_TX_NSEC)

// Note that the actual configuration of the port-write transmission is 
// set in the output parameter of this routine.  This might not match
// what was requested on input.

/* Enumerated type of all events supported by all switches.
 * _f_ : fatal error condition, packets cannot be exchanged on a link
 * _d_ : condition where individual packets can be dropped
 * _i_ : information events, not a fault
 * _a_ : action, used for clearing events
 *
 * Note that all other events are handled through the "debug" interface.
 */
typedef enum rio_em_events_t_TAG {
	// FATAL: Loss of signal.
	//	em_info: Period, in nanoseconds, when the link is electrically
	//		idle before  detecting this event. Indicates that the
	// 		link partner has gone silent, and that the link is
	// 		reinitializing
	//	rio_em_clr_events clears ackIDs to 0 for this event!
	rio_em_f_los = 0,

	// FATAL: Port error.
	//	em_info: Not used.
	//	rio_em_clr_events clears ackIDs to 0 for this event!
	rio_em_f_port_err = 1,

	// FATAL: Too many consecutive physical layer retries
	//	em_info: number of consecutive retries. It is illegal to set
	// 		em_info to 0!!!
	//	rio_em_clr_events clears ackIDs to 0 for this event!
	rio_em_f_2many_retx = 2,

	// FATAL: Too many consecutive packet-not-accepted responses.
	//	em_info: Number of PNAs which indicate link failure. This must
	// 		be a non-zero value, maximum of 0xFFFF
	//		NOTE: This can happen if the link partner is in
	//		PORT_LOCKOUT,	or if the transmitter is trying to send
	//		a corrupted packet.
	//	rio_em_clr_events clears ackIDs to 0 for this event!
	rio_em_f_2many_pna = 3,

	// FATAL: The recoverable error rate is too high.
	//	em_info:  See rio_em_compute_f_err_rate_info
	//	rio_em_clr_events clears ackIDs to 0 for this event!
	rio_em_f_err_rate = 4,

	// DROP : Packet dropped due to time to live expiry
	//	em_info: time, in nanoseconds, before Time To Live expires for
	// 		a packet. As per the RapidIO spec, there is one TTL
	//		setting for all ports on the device.  Setting this for
	// 		one port sets it for all ports.
	rio_em_d_ttl = 5,

	// DROP : Packet dropped due to routing configuration.
	//	em_info: Not used.
	//	Packet dropped due to routing table entry, or packet routed to
	// 	a disabled/locked port
	rio_em_d_rte = 6,

	// DROP : Packet dropped due to logical layer error.
	//	em_info: Value of standard RIO_STD_LOG_ERR_DET_EN register. For
	//		switches, logical layer errors are limited to those
	// 		related to maintenance packet types.
	rio_em_d_log = 7,

	// INFO : Link has reinitialized
	//	em_info: Controls when signal detection is enabled and disabled
	//		- If 0, signal detection is enabled on available,
	// 		powered up ports whose status is "port_uninit".
	//		Signal detection is disabled on available, powered up
	//		ports whose status is "Port OK".
	//		- If 1, signal detection is enabled/disabled on
	//		available, powered up ports
	rio_em_i_sig_det = 8,

	// INFO : Reset request received on link.
	//	em_info: Not used.
	//	When enabled, and the device supports it, a reset request is
	// 	converted to an interrupt. rio_em_clr_events clears ackIDs to
	//	0 for this event! Other reset request handling options are
	//	accessed through rio_pc_secure_port and rio_pc_dev_reset_config
	rio_em_i_rst_req = 9,

	// INFO : Loading register values from I2C has failed, or other
	//	automated initialization action has failed.
	//	em_info: Not used.
	// 	No parameters.  There is one I2C loader for a device - changing
	// 		this on one port changes it for all ports.
	rio_em_i_init_fail = 10,

	// This is a notification that the last pending event for a port has
	//	been found, so the port write pending indication can now be
	// 	cleared.
	rio_em_a_clr_pwpnd = 11,

	// This is a "null event".  It can be used to remove events from a list
	// 	without the complication of copying events to different
	// 	locations in thelist.
	rio_em_a_no_event = 12,

	// Last event of this type
	rio_em_last = 13
} rio_em_events_t;

// rio_em_compute_f_err_rate_info packs information from the 
// SPx_RATE_EN, SPX_ERR_RATE, and SPX_ERR_THRESH registers into 
// the "info" value used to configure the rio_em_f_err_rate event.
//
// NOTE: The RIO_SPX_RATE_EN_IMP_SPEC_ERR cannot be enabled by this routine.
//       Depending on the device, some events will not be enabled/counted.
//       The RIO_EMHS_SPX_RATE_RR, RIO_EMHS_SPX_RATE_RB and 
//          RIO_EMHS_SPX_THRESH_FAIL are packet into *info.
uint32_t rio_em_compute_f_err_rate_info(uint32_t spx_rate_en,
		uint32_t spx_err_rate, uint32_t spx_err_thresh,
		uint32_t *info);

// The rio_em_get_f_err_rate_info extracts register values from
// the "info" value used to configure the rio_em_f_err_rate event.
// The "info" value is computed by rio_em_compute_f_err_rate_info.
uint32_t rio_em_get_f_err_rate_info(uint32_t info,
		uint32_t *spx_rate_en, uint32_t *spx_err_rate,
		uint32_t *spx_err_thresh);

extern char *rio_em_events_names[(uint8_t)(rio_em_last)];

#define EVENT_NAME_STR(x) ((x < (uint8_t)(rio_em_last))?(rio_em_events_names[x]):"OORange") 

typedef enum rio_em_notfn_ctl_t_TAG {
	rio_em_notfn_none = 0,	// Disable event notification
	rio_em_notfn_int = 1,	// Use interrupt  notification mechanism
	rio_em_notfn_pw = 2,	// Use port-write notification mechanism
	rio_em_notfn_both = 3,	// Use interrupt and port-write notification mechanisms
	rio_em_notfn_0delta = 4,// Do not change  notification mechanism settings
	rio_em_notfn_last = 5	// Last control value, used for loop comparisons
} rio_em_notfn_ctl_t;

extern char *rio_em_notfn_names[ (uint8_t)(rio_em_notfn_last) ];

#define NOTFN_CTL_STR(x) ((x < (uint8_t)(rio_em_notfn_last))?(rio_em_notfn_names[x]):"OORange")

typedef enum rio_em_detect_t_TAG  {
	rio_em_detect_off,	// Disable event detection
	rio_em_detect_on,	// (Re)Enable event detection
	rio_em_detect_0delta,	// No change to event detection status
	rio_em_detect_last	// Last control value, used for loop comparisons
} rio_em_detect_t;

extern char *rio_em_detect_names[ (uint8_t)(rio_em_detect_last) ];

#define DETECT_CTL_STR(x) ((x < (uint8_t)(rio_em_detect_last))?(rio_em_detect_names[x]):"OORange")

typedef struct rio_em_cfg_t_TAG {
	// Event
	rio_em_events_t em_event;

	// Indicates whether detection of the event should be enabled/disabled.
	rio_em_detect_t em_detect;

	// Parameter which controls detection of an event.
	uint32_t em_info;
} rio_em_cfg_t;
 
#define EM_MAX_EVENT_LIST_SIZE ((uint8_t)(rio_em_last) * RIO_MAX_PORTS)

typedef struct rio_em_cfg_set_in_t_TAG {
	// Port list to configure these events for.
	struct DAR_ptl ptl;

	// Notification setting for ALL events for the port.
	rio_em_notfn_ctl_t notfn;

	// Number of entries in the events[] array
	//    Maximum value is rio_em_last.
	uint8_t num_events;

	// All events to be configured for selected port.
	rio_em_cfg_t *events;
} rio_em_cfg_set_in_t;

typedef struct rio_em_cfg_set_out_t_TAG {
	// Implementation specific return code, useful for debug
	uint32_t imp_rc;

	// Indicates the port that was being configured
	//    at the time of a failure.
	uint8_t fail_port_num;

	// Index in events[] of the entry that failed.
	uint8_t fail_idx;

	// Current notification setting for ALL events for the port.
	rio_em_notfn_ctl_t notfn;
} rio_em_cfg_set_out_t;

typedef struct rio_em_cfg_get_in_t_TAG {
	// Retrieve configuration for this port number.
	//    RIO_ALL_PORTS is an illegal value.
	uint8_t port_num;

	// Number of entries in events[]
	//    Maximum value is rio_em_last.
	uint8_t num_events;

	// Events whose status is requested
	rio_em_events_t *event_list;

	// Location to return requested information.
	//    When RIO_SUCCESS is returned, info[] has
	//    a valid entry for every entry in events[],
	//    in the same order as the events[] were requested.
	rio_em_cfg_t *events;
} rio_em_cfg_get_in_t;

typedef struct rio_em_cfg_get_out_t_TAG {
	// Implementation specific return code, useful for debug
	uint32_t imp_rc;

	// Index in events of the entry that failed.
	uint8_t fail_idx;

	// Current notification setting for ALL events for the port.
	rio_em_notfn_ctl_t notfn;
} rio_em_cfg_get_out_t;

typedef struct rio_em_dev_rpt_ctl_in_t_TAG {
	// Port number to configure event notification for.
	//    May be set to RIO_ALL_PORTS.
	struct DAR_ptl ptl;

	// Required notification setting for ALL events for the port(s).
	rio_em_notfn_ctl_t notfn;
} rio_em_dev_rpt_ctl_in_t;

typedef struct rio_em_dev_rpt_ctl_out_t_TAG {
	// Implementation specific return code, useful for debug
	uint32_t imp_rc;

	// Current notification setting for ALL events for the port(s).
	rio_em_notfn_ctl_t notfn;
} rio_em_dev_rpt_ctl_out_t;

typedef struct rio_em_event_n_loc_t_TAG {
	// Port number where the event occurred.
	uint8_t port_num;

	// One of the events which triggered the notification
	rio_em_events_t event;
} rio_em_event_n_loc_t;

#define RIO_EM_PW_COMP_TAG_IDX  RIO_EMHS_PW_COMPTAG_IDX
#define RIO_EM_PW_P_ERR_DET_IDX RIO_EMHS_PW_P_ERR_DET_IDX
#define RIO_EM_PW_IMP_SPEC_IDX  RIO_EMHS_PW_IMP_SPEC_IDX
#define RIO_EM_PW_L_ERR_DET_IDX RIO_EMHS_PW_LL_DET_IDX

#define RIO_EM_PW_IMP_SPEC_PORT_MASK RIO_EMHS_PW_IMP_SPEC_PORT
#define RIO_EM_PW_IMP_SPEC_MASK      RIO_EMHS_PW_IMP_SPEC_BITS

typedef struct rio_em_parse_pw_in_t_TAG {
	// Payload of a port-write packet.
	// Note that the handling routine must select the correct
	// device to decode the port-write format based on the
	//   component tag value in the port-write.
	uint32_t pw[RIO_EMHS_PW_WORDS];

	// Maximum number of events that can be
	//    saved in array pointed to by *events
	// - Array is indexed from 0 to num_events-1
	// - Valid range is from 1 to rio_em_last
	uint8_t num_events;

	// Array of returned event(s), which triggered the port-write
	rio_em_event_n_loc_t *events;
} rio_em_parse_pw_in_t;

typedef struct rio_em_parse_pw_out_t_TAG {
	// Implementation specific return code, useful for debug
	uint32_t imp_rc;

	// Number of entries in the arrays pointed to by
	//    *events which are valid.
	uint8_t num_events;

	// true if there were more events present than could be returned
	bool too_many;
	// true if other (debug?) events are indicated by the port-write.
	bool other_events;
} rio_em_parse_pw_out_t;

typedef struct rio_em_get_int_stat_in_t_TAG {
	// Port number which may have an interrupt asserted.
	//    May be set to RIO_ALL_PORTS.  All ports will be checked for events.
	struct DAR_ptl ptl;

	// Number of entries in events[].
	//    Maximum value is EM_MAX_EVENT_LIST_SIZE.
	uint16_t num_events;

	// Array of returned event(s), which triggered the interrupt
	rio_em_event_n_loc_t *events;
} rio_em_get_int_stat_in_t;

typedef struct rio_em_get_int_stat_out_t_TAG {
	// Implementation specific return code, useful for debug
	uint32_t imp_rc;

	// Number of entries in events[] which are valid.
	//    Maximum value is RIO_MAX_PORTS * rio_em_last.
	uint8_t num_events;

	// true if there were more events present than could be returned
	bool too_many;

	// true if debug events are present in the interrupt status
	bool other_events;
} rio_em_get_int_stat_out_t;

typedef struct rio_em_get_pw_stat_in_t_TAG {
	// Port number for which port-write event status must be gathered.
	//    May be set to RIO_ALL_PORTS if port-write status for
	//    all ports must be gathered.
	struct DAR_ptl ptl;

	// Port writes are currently being sent for this port number.
	//    To successfully clear all events triggering port-write
	//    transmission, events for this port number must be
	//    cleared last.  If set to RIO_ALL_PORTS, then the device may
	//    be sending port-writes only for logical layer errors, or not
	//    sending port-writes at all.
	uint8_t pw_port_num;

	// Maximum size of *events.
	//    Maximum value is EM_MAX_EVENT_LIST_SIZE.
	uint16_t num_events;

	// Array of returned event(s), which can/will trigger port-writes
	rio_em_event_n_loc_t *events;
} rio_em_get_pw_stat_in_t;

typedef struct rio_em_get_pw_stat_out_t_TAG {
	// Implementation specific return code, useful for debug
	uint32_t imp_rc;

	// Number of entries in the arrays popped to by *events which are valid.
	uint16_t num_events;

	// true if there were more events present than could be returned
	bool too_many;

	// true if debug events can/will trigger port-writes.
	bool other_events;
} rio_em_get_pw_stat_out_t;

// Note: It is possible to set num_events to 1, and the *event entry to {RIO_ALL_PORTS,rio_em_last}
// This will clear all events in all ports on the device.
// 
// Note: The list of events returned by rio_em_get_pw_stat and rio_em_get_int_stat must be
//       passed to rio_em_clr_events in the same order.
//
typedef struct rio_em_clr_events_in_t_TAG {
	// Number of valid entries in *events
	//    Maximum value is EM_MAX_EVENT_LIST_SIZE.
	uint16_t num_events;

	// Array of events which must be cleared.
	rio_em_event_n_loc_t *events;
} rio_em_clr_events_in_t;

typedef struct rio_em_clr_events_out_t_TAG {
	// Implementation specific return code.
	uint32_t imp_rc;

	// Index in *events where failure occurred.
	uint16_t failure_idx;

	// True if there are still events reported via port-write asserted by the device
	bool pw_events_remain;

	// True if there are still events reported via interrupt asserted by the device
	bool int_events_remain;
} rio_em_clr_events_out_t;

typedef struct rio_em_create_events_in_t_TAG {
	// Number of valid entries in *events
	//    Maximum value is EM_MAX_EVENT_LIST_SIZE.
	uint16_t num_events;

	// Array of events which must be created.
	rio_em_event_n_loc_t *events;
} rio_em_create_events_in_t;

typedef struct rio_em_create_events_out_t_TAG {
	// Implementation specific return code.
	uint32_t imp_rc;

	// Index in *events where failure occurred.
	uint16_t failure_idx;
} rio_em_create_events_out_t;

// Implementation specific error return codes
#define EM_CFG_PW_0           (DAR_FIRST_IMP_SPEC_ERROR+0x2000)
#define EM_CFG_SET_0          (DAR_FIRST_IMP_SPEC_ERROR+0x2100)
#define EM_CFG_GET_0          (DAR_FIRST_IMP_SPEC_ERROR+0x2200)
#define EM_DEV_RPT_CTL_0      (DAR_FIRST_IMP_SPEC_ERROR+0x2300)
#define EM_PARSE_PW_0         (DAR_FIRST_IMP_SPEC_ERROR+0x2400)
#define EM_GET_INT_STAT_0     (DAR_FIRST_IMP_SPEC_ERROR+0x2500)
#define EM_GET_PW_STAT_0      (DAR_FIRST_IMP_SPEC_ERROR+0x2600)
#define EM_CLR_EVENTS_0       (DAR_FIRST_IMP_SPEC_ERROR+0x2700)
#define EM_CREATE_EVENTS_0    (DAR_FIRST_IMP_SPEC_ERROR+0x2800)

#define EM_FIRST_SUBROUTINE_0 (DAR_FIRST_IMP_SPEC_ERROR+0x200000)


// Routines to configure port write transmission, 
// and set/query specific event detection/reporting.

#define EM_CFG_PW(x) (EM_CFG_PW_0+x)

uint32_t rio_em_cfg_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_pw_in_t *in_parms,
		rio_em_cfg_pw_out_t *out_parms);

#define EM_CFG_SET(x) (EM_CFG_SET_0+x)

uint32_t rio_em_cfg_set(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_set_in_t *in_parms,
		rio_em_cfg_set_out_t *out_parms);

#define EM_CFG_GET(x) (EM_CFG_GET_0+x)

uint32_t rio_em_cfg_get(DAR_DEV_INFO_t *dev_info,
		rio_em_cfg_get_in_t *in_parms,
		rio_em_cfg_get_out_t *out_parms);

// Routines to query and control port-write and interrupt
// reporting configuration for a port/device.

#define EM_DEV_RPT_CTL(x) (EM_DEV_RPT_CTL_0+x)

uint32_t rio_em_dev_rpt_ctl(DAR_DEV_INFO_t *dev_info,
		rio_em_dev_rpt_ctl_in_t *in_parms,
		rio_em_dev_rpt_ctl_out_t *out_parms);

// Routines to convert port-write contents into an event list,
// and to query a port/device and return a list of asserted events 
// which are reported via interrupt or port-write, 

#define EM_PARSE_PW(x) (EM_PARSE_PW_0+x)

uint32_t rio_em_parse_pw(DAR_DEV_INFO_t *dev_info,
		rio_em_parse_pw_in_t *in_parms,
		rio_em_parse_pw_out_t *out_parms);

#define EM_GET_INT_STAT(x) (EM_GET_INT_STAT_0+x)

uint32_t rio_em_get_int_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_int_stat_in_t *in_parms,
		rio_em_get_int_stat_out_t *out_parms);

#define EM_GET_PW_STAT(x) (EM_GET_PW_STAT_0+x)

uint32_t rio_em_get_pw_stat(DAR_DEV_INFO_t *dev_info,
		rio_em_get_pw_stat_in_t *in_parms,
		rio_em_get_pw_stat_out_t *out_parms);

// Routine to clear events, and a routine to create events
// for software testing purposes.
//
// Note:  Clearing fatal errors assumes that all packets in
//        the affected port should be discarded, and that
//        ackIDs should be returned to 0.  This will be       
//        done without issuing any control symbols or packets.  
//        The state of the link partner must be made consistent
//        with the state of the affected port using separate 
//        routines found in RapidIO_Port_Config_API.h

#define EM_CLR_EVENTS(x) (EM_CLR_EVENTS_0+x)

uint32_t rio_em_clr_events(DAR_DEV_INFO_t *dev_info,
		rio_em_clr_events_in_t *in_parms,
		rio_em_clr_events_out_t *out_parms);

#define EM_CREATE_EVENTS(x)    (EM_CREATE_EVENTS_0+x)

uint32_t rio_em_create_events(DAR_DEV_INFO_t *dev_info,
		rio_em_create_events_in_t *in_parms,
		rio_em_create_events_out_t *out_parms);


void rio_em_add_pw_event(rio_em_get_pw_stat_in_t *in_parms,
		rio_em_get_pw_stat_out_t *out_parms, uint8_t pnum,
		rio_em_events_t event);

void rio_em_add_int_event(rio_em_get_int_stat_in_t *in_parms,
		rio_em_get_int_stat_out_t *out_parms, uint8_t pnum,
		rio_em_events_t event);

#ifdef __cplusplus
}
#endif
#endif /* __RAPIDIO_ERROR_MANAGEMENT_API_H__ */
