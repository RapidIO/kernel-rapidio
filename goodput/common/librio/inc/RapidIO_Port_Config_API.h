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

#ifndef __RAPIDIO_PORT_CONFIG_API_H__
#define __RAPIDIO_PORT_CONFIG_API_H__

#include <stdint.h>
#include <stdbool.h>

#include "rio_route.h"
#include "rio_ecosystem.h"
#include "RapidIO_Device_Access_Routines_API.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Definitions of parameter structures for RapidIO Port Configuration routines.

   Generic structure which contains the parameters which describe the
   configuration of a port.
*/
#define RIO_PC_NO_ASSIGNED_LANE	0

#define RIO_PC_UNINIT_PARM	0xFF

#define RIO_PC_SUCCESS		0x000
#define RIO_PC_INVALID		0x001

#define RIO_PC_PORT_UNAVAILABLE	0x100
#define RIO_PC_CORRUPTED	0x103
#define RIO_PC_DEV_SPEC		0x104
#define RIO_PC_PORT_ERR		0x105

typedef enum rio_pc_pw_t_tag
{
	// Single lane port
	rio_pc_pw_1x = 0,

	// Two lane port
	rio_pc_pw_2x = 1,

	// Four lane port
	rio_pc_pw_4x = 2,

	// Force multilane port to operate on lane 0
	rio_pc_pw_1x_l0 = 3,

	// Force multilane port to operate on lane 1
	rio_pc_pw_1x_l1 = 4,

	// Force multilane port to operate on lane 2
	rio_pc_pw_1x_l2 = 5,

	// Last port width value
	rio_pc_pw_last = rio_pc_pw_1x_l2 + 1,
} rio_pc_pw_t;

extern int         pw_to_lanes[]; // Converts rio_pc_pw_t to lane count
extern char       *pw_to_str[]  ; // Converts rio_pc_pw_t to string
extern rio_pc_pw_t lanes_to_pw[]; // Converts lane count to rio_pc_pw_t

#define PW_TO_LANES(x) (x>=rio_pc_pw_last?pw_to_lanes[rio_pc_pw_last]:pw_to_lanes[x])
#define PW_TO_STR(x) (x>=rio_pc_pw_last?pw_to_str[rio_pc_pw_last]:pw_to_str[x])
#define LANES_TO_PW(x) (x>4?rio_pc_pw_last:lanes_to_pw[x])

typedef enum rio_pc_ls_t_tag {
	// 1.25 Gbaud lane speed
	rio_pc_ls_1p25 = 0,

	// 2.5  Gbaud lane speed
	rio_pc_ls_2p5 = 1,

	// 3.125 Gbaud lane speed
	rio_pc_ls_3p125 = 2,

	// 5.0 Gbaud lane speed
	rio_pc_ls_5p0 = 3,

	// 6.25 Gbaud lane speed
	rio_pc_ls_6p25 = 4,

	// 10.3 Gbaud lane speed
	rio_pc_ls_10p3 = 5,

	// 12.5 Gbaud lane speed
	rio_pc_ls_12p5 = 6,

	// last lane speed, not used
	rio_pc_ls_last = rio_pc_ls_12p5 + 1,
} rio_pc_ls_t;

// Convert lane speed to a string
extern char *ls_to_str[];

#define LS_TO_STR(x) (x>=rio_pc_ls_last?ls_to_str[rio_pc_ls_last]:ls_to_str[x])

enum rio_pc_fc {
	rio_pc_fc_rx,
	rio_pc_fc_tx,
	rio_pc_fc_last
};

// Convert flow control to a string
extern char *fc_to_str[];
#define FC_TO_STR(x) (x>=rio_pc_fc_last?fc_to_str[rio_pc_fc_last]:fc_to_str[x])

enum rio_pc_idle_seq {
	rio_pc_is_one = 0,
	rio_pc_is_two = 1,
	rio_pc_is_three = 2,
	rio_pc_is_dflt = 3,
	rio_pc_is_last = 4,
};

// Convert idle sequenceto a string
extern char *is_to_str[];

#define ISEQ_TO_STR(x) (x>=rio_pc_is_last?is_to_str[rio_pc_is_last]:is_to_str[x])

enum rio_lane_swap_t {
	rio_lswap_none = 0,
	rio_lswap_ABCD_BADC = 1,
	rio_lswap_ABCD_DCBA = 2,
	rio_lswap_ABCD_CDAB = 3,
	rio_lswap_last = rio_lswap_ABCD_CDAB + 1
};

typedef struct rio_pc_one_port_config_t_TAG {
	// Port number.  Allows port configuration information to be declared
	// out of order, as per quadrants/port numbering.
	uint8_t pnum;

	// true if the port is available, false if the port is not available
	// If the port is not available, resources associated with the port may
	// be powered down as long as they do not interfere with other ports.
	// If the port is not available, only the "powerdown" field of this
	// structure is valid.
	bool port_available;

	// true if the port is powered up false if the port is powered down.
	bool powered_up;

	// Port width
	rio_pc_pw_t pw;

	// Lane speed
	rio_pc_ls_t ls;

	// Type of flow control that is enabled.
	enum rio_pc_fc fc;

	// Idle sequence configured IDLE2 means that IDLE2 or IDLE1 may be used
	enum rio_pc_idle_seq iseq;

	// True if the port should be disabled, false if the port should be
	// enabled.  NOTE that if a port is disabled, the transmitter is
	// disabled and the port will not be able to train. Packets routed to a
	// disabled port are dropped. Corresponds to standard "port disable" bit.
	bool xmitter_disable;

	// True if the port should be locked out, false if the port should not
	// be locked out.  NOTE that if a port is locked out, the transmitter
	// and receiver are enabled, so the port can achieve PORT_OK, but
	// packets cannot be exchanged.
	bool port_lockout;

	// True if the port should be able to send and receive non-maintenance
	// packets, false if non-maintenance packets should not be exchanged by
	// this port.  NOTE: To prevent all packets from being exchanged on the
	// port, set port_lockout. Corresponds to standard "output port enable"
	// and "input port enable" bit.
	bool nmtc_xfer_enable;

	// True if the transmit lanes are connected to a port in order of
	// highest numbered lane to lowest numbered
	enum rio_lane_swap_t tx_lswap;

	// True if the tracks of a differential pair are inverted. Does not
	// reflect lane swapping status.
	bool tx_linvert[RIO_MAX_PORT_LANES];

	// Trued if the receive lanes are connected to a port in order of
	// highest numbered lane to lowest numbered.
	enum rio_lane_swap_t rx_lswap;

	// True if the tracks of a differential pair are inverted. Does not
	// reflect lane swapping status.
	bool rx_linvert[RIO_MAX_PORT_LANES];
} rio_pc_one_port_config_t;

/* Structure which captures the status of a port.
*/
typedef struct rio_pc_one_port_status_t_TAG {
	// Port number.  Allows port configuration information to be declared
	// out of order, as per quadrants/port numbering.
	uint8_t pnum;

	// true if a link partner is present and control symbols can be
	// exchanged successfully.
	bool port_ok;

	// Port width
	rio_pc_pw_t pw;

	// Flow control algorithm for the link
	enum rio_pc_fc fc;

	// Idle sequence being used
	enum rio_pc_idle_seq iseq;

	// true if a fatal error is present which prevents packet transmission.
	// This is a combination of the standard PORT_ERR indications, and the
	// PORT_FAILED indication when "Stop on Fail" is setin the Port x
	// Control CSR.
	bool port_error;

	// true if the port has detected a transmission error or has retried a
	// packet and is awaiting packet transmission to start again.
	bool input_stopped;

	// true if the port is attempting error recovery with the link partner.
	bool output_stopped;

	// Number of lanes connected to the port. 0 means that the port is
	// unavailable, powered down, or both.
	uint8_t num_lanes;

	// Lane number of the first lane connected to the port.  It is assumed
	// that sequentially numbered lanes are connected to a port.
	bool first_lane;

} rio_pc_one_port_status_t;

typedef struct rio_pc_get_config_in_t_TAG {
	// Return configuration for this port list.
	struct DAR_ptl ptl;
} rio_pc_get_config_in_t;

typedef struct rio_pc_set_config_in_t_TAG {
	// Link response timeout value for all ports. Specified in hundreds
	// of nanoseconds, range of 0 (disabled) up to 60,000,000 (6 seconds).
	uint32_t lrto;

	// Logical layer response timeout, specified in hundreds of nanoseconds
	// Range of 0 (disabled) up to 60,000,000 (6 seconds)
	uint32_t log_rto;

	// If true, register access is not dependent upon RapidIO
	// (i.e. JTAG, I2C).  It is possible to reprogram the port used for
	// RapidIO connectivity.
	bool oob_reg_acc;

	uint8_t reg_acc_port;
	// Register access port. Valid when oob_reg_acc is false. Must be
	// filled in to protect against inadvertently disabling/resetting
	// the port(s) used for connectivity to the switch.

	// Number of ports which should be updated. If RIO_ALL_PORTS was
	// passed in, only the first entry of pc must be valid. All ports
	// will be configured according to this entry.
	uint8_t num_ports;

	rio_pc_one_port_config_t pc[RIO_MAX_PORTS];
} rio_pc_set_config_in_t;

#define RIO_LRTO_NSEC 100
#define RIO_LOG_RTO_NSEC 100
#define RIO_LRTO_MAX_100NS (6000000000 / RIO_LRTO_NSEC)

typedef struct rio_pc_set_config_out_t_TAG {
	// Implementation specific return code information.
	uint32_t imp_rc;

	// Link response timeout value for all ports. Specified in hundreds of
	// nanoseconds, range of 0 (disabled) up to 60,000,000 (6 seconds).
	uint32_t lrto;

	// Logical layer response timeout, specified in hundreds of
	// nanoseconds.  Range of 0 (disabled) up to 60,000,000 (6 seconds)
	uint32_t log_rto;

	// Number of ports which are now present. If RIO_ALL_PORTS was passed
	// in, this reflects the actual number of ports present after the
	// configuration was changed.
	uint8_t num_ports;

	// Current configuration of the devices ports.
	rio_pc_one_port_config_t pc[RIO_MAX_PORTS];
} rio_pc_set_config_out_t;

typedef rio_pc_set_config_out_t rio_pc_get_config_out_t;

typedef struct rio_pc_get_status_in_t_TAG {
	struct DAR_ptl ptl;
} rio_pc_get_status_in_t;

typedef struct rio_pc_get_status_out_t_TAG {
	// Implementation specific return code information.
	uint32_t imp_rc;

	// Number of ports for which status was returned. If RIO_ALL_PORTS was
	// passed in rio_pc_get_status_in_t.ptl.num_ports, this reflects the
	// number of ports which have information present in pc.
	uint8_t num_ports;

	rio_pc_one_port_status_t ps[RIO_MAX_PORTS];
} rio_pc_get_status_out_t;

// The RapidIO port used to access the switch will not be reset 
// using this routine unless oob_reg_acc is true.
//
// When out-of-band access methods are used (I2C or JTAG),
// Port 0 will not be reset unless oob_reg_acc is true.
//
// Resetting a port requires multiple register
// accesses.  When registers are accessed using RapidIO, 
// the port used for connectivity must be reset by using 
// rio_pc_reset_link_partner.

typedef struct rio_pc_reset_port_in_t_TAG {
	// NOTE: If RIO_ALL_PORTS == port_num, reset_lp is true, and
	// preserve_config is false, this routine MAY reset the entire device.

	// Port which should be reset on this device. RIO_ALL_PORTS is an
	// acceptable value
	uint8_t port_num;

	// If true, register access is not dependent upon RapidIO
	// (i.e. JTAG, I2C).  It is possible to reset the port used for
	// RapidIO connectivity.
	bool oob_reg_acc;

	// Register access port. Valid when oob_reg_acc is false.  Must be
	// filled in to protect against inadvertently resetting the port(s)
	// used for connectivity to the switch.
	uint8_t reg_acc_port;

	// If true, reset the link partner just before resetting this port.
	bool reset_lp;

	// If true, preserves port configuration state which may be destroyed
	// by the reset. When false, the configuration of the port may be
	// destroyed by the reset.  This may be a more comprehensive reset than
	// what is done when preserve_config is true.
	bool preserve_config;
} rio_pc_reset_port_in_t;

typedef struct rio_pc_reset_port_out_t_TAG {
	// Implementation specific return code information.
	uint32_t imp_rc;
} rio_pc_reset_port_out_t;

typedef struct rio_pc_reset_link_partner_in_t_TAG {
	// Port whose link partner should be reset. Must be a valid port
	// number. RIO_ALL_PORTS is not supported. Port must have PORT_OK
	// status forthis routine to succeed.
	uint8_t port_num;

	// If true, attempts to resychronize ackIDs by clearing ackIDs to 0 on
	// the local port after the reset.
	bool resync_ackids;
} rio_pc_reset_link_partner_in_t;

typedef struct rio_pc_reset_link_partner_out_t_TAG {
	// Implementation specific return code information.
	uint32_t imp_rc;
} rio_pc_reset_link_partner_out_t;

typedef struct rio_pc_clr_errs_in_t_TAG {
	// Port on this device which should have its input-err stop, output-err
	// stop and port_err error conditions cleared. RIO_ALL_PORTS is an
	// illegal value for this field. The port MUST have PORT_OK status for
	// this routine to be successfull.  If the port does not have PORT_OK
	// status, then the port may be reset to clear errors.
	uint8_t port_num;

	// rio_pc_clr_errs will always attempt to clear input error-stop,
	// output error-stop and port error conditions on the local port. This
	// may involve sending control symbols to the link partner. If true,
	// then this routine will attempt to clear port_err conditions on the
	// link partner. Depending on the device, this may involve sending
	// control symbols and packets to the link partner to establish ackID
	// synchronization.  It may involve resetting the link partner.
	// Maintenance requests may time out. If false, then this routine will
	// not reset the link partner or send packets to the link partner.
	// AckID values are cleared to 0 on the local end of the link. This is
	// a lower risk option option that can be used if the capacity to clear
	// port_err locally exists at both ends of the link.
	bool clr_lp_port_err;

	// Device information for the link partner. If this pointer is NULL
	// when clr_lp_port_err is true, error recovery is limited to resetting
	// the link partner and the local port.
	DAR_DEV_INFO_t *lp_dev_info;

	// Number of entries in the lp_port_list.
	// If this value is 0 when clr_lp_port_err is true, then ports on the
	// link partner will be checked until the port_err condition is
	// cleared. Note that both available and unavailable ports will be
	// checked.  If more than one port is supplied, response timeouts may
	// occur for maintenance packets.
	uint8_t num_lp_ports;

	// List of possible link partner port numbers that the local port could
	// be connected to.
	uint8_t lp_port_list[RIO_MAX_PORTS];
} rio_pc_clr_errs_in_t;

typedef struct rio_pc_clr_errs_out_t_TAG {
	// Implementation specific return code information.
	uint32_t imp_rc;
} rio_pc_clr_errs_out_t;

typedef enum {
	// Link reset request resets the device
	rio_pc_rst_device = 0,

	// Just reset the port the reset request was received on
	rio_pc_rst_port = 1,

	// Assert an interrupt if a reset request was received
	rio_pc_rst_int = 2,

	// Send a port-write if a reset request was received
	rio_pc_rst_pw = 3,

	// Ignore reset requests received.
	rio_pc_rst_ignore = 4,

	// Begin illegal parameter values
	rio_pc_rst_last = rio_pc_rst_ignore + 1,
} rio_pc_rst_handling;

// Convert reset configuration to a string
extern char *rst_to_str[];

/* The secure_port routine may be used on any device.  Some parameters
 * may not apply to all devices.
 */
typedef struct rio_pc_secure_port_in_t_TAG {
	// List of ports to configure.
	struct DAR_ptl ptl;

	// If false, filter out maintenance packets if possible.
	bool mtc_pkts_allowed;

	// If false, this port will not retransmit MECS events
	bool MECS_participant;

	// If false, this port will ignore MECS
	bool MECS_acceptance;

	// Configure reset handling for this port.
	rio_pc_rst_handling rst;
} rio_pc_secure_port_in_t;

typedef struct rio_pc_secure_port_out_t_TAG {
	// Implementation specific return code information.
	uint32_t imp_rc;

	// Current configuration values for specified port.
	bool bc_mtc_pkts_allowed;

	bool MECS_participant;
	bool MECS_acceptance;
	rio_pc_rst_handling rst;
} rio_pc_secure_port_out_t;

typedef struct rio_pc_dev_reset_config_in_t_TAG {
	// Configure reset handling for the device.
	rio_pc_rst_handling rst;
} rio_pc_dev_reset_config_in_t;

typedef struct rio_pc_dev_reset_config_out_t_TAG {
	// Implementation specific return code information
	uint32_t imp_rc;

	// Current configuration value.
	rio_pc_rst_handling rst;
} rio_pc_dev_reset_config_out_t;

typedef struct rio_pc_probe_in_t_TAG {
	//* Check this port for problems exchanging control symbols
	uint8_t port;
} rio_pc_probe_in_t;

typedef enum rio_pc_port_status_t_TAG {
	port_ok,	//
	port_degr,	//
	port_los,	//
	port_err,
} rio_pc_port_status_t;

typedef struct rio_pc_probe_out_t_TAG {
	// Implementation specific return code information
	uint32_t imp_rc;

	// Current configuration value.
	rio_pc_port_status_t status;
} rio_pc_probe_out_t;

// Implementation specific return codes for Port Configuration routines

#define PC_GET_CONFIG_0       (DAR_FIRST_IMP_SPEC_ERROR+0x0100)
#define PC_SET_CONFIG_0       (DAR_FIRST_IMP_SPEC_ERROR+0x0200)
#define PC_GET_STATUS_0       (DAR_FIRST_IMP_SPEC_ERROR+0x0300)
#define PC_RESET_PORT_0       (DAR_FIRST_IMP_SPEC_ERROR+0x0400)
#define PC_RESET_LP_0         (DAR_FIRST_IMP_SPEC_ERROR+0x0500)
#define PC_CLR_ERRS_0         (DAR_FIRST_IMP_SPEC_ERROR+0x0600)
#define PC_SECURE_PORT_0      (DAR_FIRST_IMP_SPEC_ERROR+0x0700)
#define PC_DEV_RESET_CONFIG_0 (DAR_FIRST_IMP_SPEC_ERROR+0x0800)
#define PC_PROBE_0            (DAR_FIRST_IMP_SPEC_ERROR+0x0900)

#define PC_FIRST_SUBROUTINE_0 (DAR_FIRST_IMP_SPEC_ERROR+0x1000)

/* The following functions are implemented to support the above structures
 * Refer to the above structures for the implementation detail
 */

/* This function returns the port's configuration */
#define PC_GET_CONFIG(x) (PC_GET_CONFIG_0+x)
uint32_t rio_pc_get_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_config_in_t *in_parms,
		rio_pc_get_config_out_t *out_parms);

/* This function sets up the port width, lane speed, etc. */
#define PC_SET_CONFIG(x) (PC_SET_CONFIG_0+x)
uint32_t rio_pc_set_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_set_config_in_t *in_parms,
		rio_pc_set_config_out_t *out_parms);

/* This function returns the status of port */
#define PC_GET_STATUS(x) (PC_GET_STATUS_0+x)
uint32_t rio_pc_get_status(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_status_in_t *in_parms,
		rio_pc_get_status_out_t *out_parms);

/* This function resets the specified port as well as the link partner */
#define PC_RESET_PORT(x) (PC_RESET_PORT_0+x)

uint32_t rio_pc_reset_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_port_in_t *in_parms,
		rio_pc_reset_port_out_t *out_parms);

/* This function resets the link partner */
#define PC_RESET_LP(x) (PC_RESET_LP_0+x)
uint32_t rio_pc_reset_link_partner(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_link_partner_in_t *in_parms,
		rio_pc_reset_link_partner_out_t *out_parms);

/* This function clear error flags */
#define PC_CLR_ERRS(x) (PC_CLR_ERRS_0+x)
uint32_t rio_pc_clr_errs(DAR_DEV_INFO_t *dev_info,
		rio_pc_clr_errs_in_t *in_parms,
		rio_pc_clr_errs_out_t *out_parms);

/* This function configures MECS and Maintenance Pkt Allowance */
#define PC_SECURE_PORT(x) (PC_SECURE_PORT_0+x)
uint32_t rio_pc_secure_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_secure_port_in_t *in_parms,
		rio_pc_secure_port_out_t *out_parms);

/* This function configures device behavior when a reset request is received */
#define PC_DEV_RESET_CONFIG(x) (PC_DEV_RESET_CONFIG_0+x)
uint32_t rio_pc_dev_reset_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_dev_reset_config_in_t *in_parms,
		rio_pc_dev_reset_config_out_t *out_parms);

/* This function determines if packets can be exchanged on a port. */
#define PC_PROBE(x) (PC_PROBE_0+x)
uint32_t rio_pc_probe(DAR_DEV_INFO_t *dev_info,
		rio_pc_probe_in_t *in_parms,
		rio_pc_probe_out_t *out_parms);

#ifdef __cplusplus
}
#endif

#endif /* __RAPIDIO_PORT_CONFIG_API_H__ */
