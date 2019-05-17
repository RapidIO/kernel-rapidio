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

#ifndef __RAPIDIO_DEVICE_ACCESS_ROUTINES_API_H__
#define __RAPIDIO_DEVICE_ACCESS_ROUTINES_API_H__

#include <stdint.h>
#include <stdbool.h>

#include "rio_route.h"
#include "rio_standard.h"
#include "rio_ecosystem.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Device Access Routine (DAR) Device Driver Interface
* 
*  RapidIO_Device_Access_Routines_API.h defines the interface for invoking
*  the device driver routines defined in the RapidIO Specification Annex 1.
* 
*  There are also hooks to extend the use of the DAR to support Device 
*  Specific Functions (DSF)
*/


/* Device type based on the Vendor and Device code of the DAR_DEV_INFO_t.
 * See rio_get_driver_family(uint32_t)
 */
typedef enum rio_driver_family_t_TAG {
	RIO_UNITIALIZED_DEVICE	= 0x00,
	RIO_CPS_DEVICE		= 0x01,
	RIO_RXS_DEVICE		= 0x02,
	RIO_TSI721_DEVICE	= 0x04,
	RIO_TSI57X_DEVICE	= 0x08,
	RIO_UNKNOWN_DEVICE	= 0x10,
} rio_driver_family_t;

/* Each unique device driver is identified with a handle.
*  The structure of the handle is hidden from the user.
*  0 is always an invalid handle.
*/
#define INVALID_HANDLE 0

typedef uint32_t DAR_Handle_t;    /* Device Specific Function Handle */

#define NAME_SIZE	15
#define MAX_DAR_SCRPAD_IDX  30

typedef struct rio_perf_opt_reg_t_TAG {
	uint32_t offset;
	uint32_t data;
} rio_perf_opt_reg_t;

typedef struct DAR_DEV_INFO_t_TAG
{
	// Pointer to a fabric management private data for this device.
	void *privateData;

	// Pointer to an access info structure
	void *accessInfo;

	// Text name of this device.
	char name[NAME_SIZE];

	// Based on value of devID
	rio_driver_family_t driver_family;

	// Handle for access to device-specific functions, if provided.
	// The following fields are used by the ReadReg and WriteReg host
	// specific routines to determine where to access registers
	// (Host or RapidIO network) and to specify parameters for routine
	// maintenance requests in the RapidIO network.

	// Address offsets for various register blocks.
	// A value of 0x00000000 means that the block does not exist on this device.
	DAR_Handle_t dsf_h;

	// Offset of LP-Serial Register Extensions block.
	uint32_t extFPtrForPort;

	// RO: ID	RIO_EFB_T_SP_EP, RIO_EFB_T_SP_EP_SAER, 
	// 		RIO_EFB_T_SP_NOEP, RIO_EFB_T_SP_NOEP_SAER,
	// 		RIO_EFB_T_SP_EP3, RIO_EFB_T_SP_EP3_SAER,
	// 		RIO_EFB_T_SP_NOEP3, RIO_EFB_T_SP_NOEP3_SAER
	uint32_t extFPtrPortType;

	// RO: ID  0x0D: Lane Status
	uint32_t extFPtrForLane;

	// RO: ID  0x07: Error Management
	uint32_t extFPtrForErr;

	// RO: ID  0x0A: Virtual Channel
	uint32_t extFPtrForVC;

	// RO: ID  0x0B: Virtual Output Queueing
	uint32_t extFPtrForVOQ;

	// RO: ID  0x0E: Routing Tablet
	uint32_t extFPtrForRT;

	// RO: ID  0x0F: Timestamp Synchronization
	uint32_t extFPtrForTS;

	// RO: ID  0x10: Miscellaneous Register Block
	uint32_t extFPtrForMISC;

	// RO: ID  0x17: Hot Swap
	uint32_t extFPtrForHS;

	// Values of RapidIO Standard Registers, useful for understanding device
	//     capabilities without reading registers.

	// Contents of RapidIO Standard register RIO_DEV_IDENT
	uint32_t devID;

	// Contents of RapidIO Standard register RIO_DEV_INF
	uint32_t devInfo;

	// Contents of RapidIO Standard register RIO_ASSY_INF_CAR
	uint32_t assyInfo;

	// Contents of RapidIO Standard register RIO_PE_FEAT
	uint32_t features;

	// Contents of RapidIO Standard register RIO_SW_PORT_INF
	uint32_t swPortInfo;

	// Contents of RapidIO Standard register RIO_SW_RT_TBL_LIM
	uint32_t swRtInfo;

	// Contents of RapidIO Standard register RIO_SRC_OPS
	uint32_t srcOps;

	// Contents of RapidIO Standard register RIO_DST_OPS
	uint32_t dstOps;
	// Contents of RapidIO Standard register RIO_SW_RT_TBL_LIM_MAX_DESTID
	uint32_t swMcastInfo;

	// ctl1_reg values are tracked to implement EmergencyLockout as a
	// single write, instead of a read/modify/write, for all devices.
	//

	// Last register value read from or written to the Port x Control 1 CSR
	uint32_t ctl1_reg[RIO_MAX_PORTS];

	// Scratchpad to be used by different devices as they see fit.
	uint32_t scratchpad[MAX_DAR_SCRPAD_IDX];

	// Register access performance optimization facility.
	// poregs_max indicates the total number of poregs available.
	// poreg_cnt indicates the number of poregs that are valid.
	//
	// poregs are searched as part of DARRegRead, and if the offset is
	// found, the poregs[x].data value is returned without performing
	// an external register read.
	//
	// Similarly, DARRegWrite searches poregs and updates poregs[x].data
	// if the offset matches the register just written.
	//
	// Use add_poreg to add a register with a known value.
	uint32_t poregs_max;
	uint32_t poreg_cnt;
	rio_perf_opt_reg_t *poregs;
} DAR_DEV_INFO_t;

uint32_t DAR_add_poreg(DAR_DEV_INFO_t *dev_info, uint32_t oset, uint32_t data);

#define DAR_POREG_BAD_IDX 0xFFFFFFFF
uint32_t DAR_get_poreg_idx(DAR_DEV_INFO_t *dev_info, uint32_t oset);

#define NUM_PORTS(x)    (( uint8_t)((((x)->swPortInfo ) & RIO_SW_PORT_INF_TOT) >>  8))
#define CONN_PORT(x)    (( uint8_t)(((x)->swPortInfo ) & RIO_SW_PORT_INF_PORT))
#define NUM_MC_MASKS(x) ( (uint8_t)((((x)->swMcastInfo) & RIO_SW_MC_INF_MC_MSK)))
#define VEND_CODE(x)    ((uint16_t)(((x)->devID) & RIO_DEV_IDENT_VEND))
#define DEV_CODE(x)     ((uint16_t)((((x)->devID) & RIO_DEV_IDENT_DEVI) >> 16))
#define SWITCH(x)	((bool)(((x)->features & RIO_PE_FEAT_SW) ? true:false))
#define MEMORY(x)	((bool)(((x)->features & RIO_PE_FEAT_MEM) ? true:false))

/* Determine the Device family as per the DAR_DEV_INFO_t
 * @param[in] the device ID as read from the RIO_DEV_IDENT register
 * @return the rio_driver_family_t
 */
rio_driver_family_t rio_get_driver_family(uint32_t devID);

/* DAR_Find_Driver_for_Device
* 
*  Function to bind a device instance to a set of DAR routines
*  Updates the device info with a value which allows quick access to DAR 
*  routines.  The fields in dev_info must be set as follows before calling 
*  this routine:
*  dev_info_devID_valid - If this flag set to 1, use dev_info->devID instead of
*                         probing.  Otherwise, the internal routine probe the
*                         SRIO device to get the Device ID
*  On exit, all fields in dev_info will be correctly initialized.
*/
uint32_t DAR_Find_Driver_for_Device( bool dev_info_devID_valid,
		DAR_DEV_INFO_t *dev_info);

/* DAR_Release_Driver_for_Device
*
*  If a device has been removed from the system, call this routine to
*  free up resources associated with the driver.
*/

uint32_t DAR_Release_Driver_for_Device ( DAR_DEV_INFO_t *dev_info );

/* Routines to access DAR device registers.  This is a hook to allow
*      device specific register read/write routines to correct non-compliances
*      with registers.  Could also be used as a hook for debugging.
*  The default definition of these routines is a call to ReadReg/WriteReg.
*/
uint32_t DAR_proc_ptr_init(
		uint32_t (*ReadRegCall)(DAR_DEV_INFO_t *dev_info,
				uint32_t offset, uint32_t *readdata),
		uint32_t (*WriteRegCall)(DAR_DEV_INFO_t *dev_info,
				uint32_t offset, uint32_t writedata),
		void (*WaitSecCall)(uint32_t delay_nsec,
				uint32_t delay_sec));

extern uint32_t (*ReadReg)(DAR_DEV_INFO_t *dev_info,
				uint32_t offset,
				uint32_t *readdata);

extern uint32_t (*WriteReg)(DAR_DEV_INFO_t *dev_info,
				uint32_t offset,
				uint32_t writedata);

extern void (*WaitSec)(uint32_t delay_nsec, uint32_t delay_sec);

uint32_t DARRegRead ( DAR_DEV_INFO_t *dev_info, uint32_t offset, uint32_t *readdata );
uint32_t DARRegWrite( DAR_DEV_INFO_t *dev_info, uint32_t offset, uint32_t writedata );
void DAR_WaitSec( uint32_t delay_nsec, uint32_t delay_sec);

/* Routines which invoke the associated device driver function.
* 
*  All of these routines have default DAR implementations which rely on 
*  RapidIO standard registers.
*  For more information, refer to Annex I of the RapidIO Specification, 
*      available from www.rapidio.org
*/
uint32_t DARrioGetNumLocalPorts(DAR_DEV_INFO_t *dev_info,
		uint32_t *numLocalPorts);
uint32_t DARrioGetFeatures(DAR_DEV_INFO_t *dev_info,
		RIO_PE_FEAT_T *features);
uint32_t DARrioGetSwitchPortInfo(DAR_DEV_INFO_t *dev_info,
		RIO_SW_PORT_INF_T *portinfo);
uint32_t DARrioGetExtFeaturesPtr(DAR_DEV_INFO_t *dev_info,
		uint32_t *extfptr);
uint32_t DARrioGetNextExtFeaturesPtr(DAR_DEV_INFO_t *dev_info,
		uint32_t currfptr, uint32_t *extfptr);
uint32_t DARrioGetSourceOps(DAR_DEV_INFO_t *dev_info,
		RIO_SRC_OPS_T *srcops);
uint32_t DARrioGetDestOps(DAR_DEV_INFO_t *dev_info,
		RIO_DST_OPS_T *dstops);
uint32_t DARrioGetAddressMode(DAR_DEV_INFO_t *dev_info,
		RIO_PE_ADDR_T *amode);
uint32_t DARrioGetBaseDeviceId(DAR_DEV_INFO_t *dev_info,
		uint32_t *deviceid);
uint32_t DARrioSetBaseDeviceId(DAR_DEV_INFO_t *dev_info,
		uint32_t newdeviceid);
uint32_t DARrioAcquireDeviceLock(DAR_DEV_INFO_t *dev_info,
		uint16_t hostdeviceid, uint16_t *hostlockid);
uint32_t DARrioReleaseDeviceLock(DAR_DEV_INFO_t *dev_info,
		uint16_t hostdeviceid, uint16_t *hostlockid);
uint32_t DARrioGetComponentTag(DAR_DEV_INFO_t *dev_info,
		uint32_t *componenttag);
uint32_t DARrioSetComponentTag(DAR_DEV_INFO_t *dev_info,
		uint32_t componenttag);
uint32_t DARrioGetAddrMode(DAR_DEV_INFO_t *dev_info,
		RIO_PE_ADDR_T *addr_mode);
uint32_t DARrioSetAddrMode(DAR_DEV_INFO_t *dev_info,
		RIO_PE_ADDR_T addr_mode);
uint32_t DARrioGetPortErrorStatus(DAR_DEV_INFO_t *dev_info,
		uint8_t portnum, RIO_SPX_ERR_STAT_T *err_status);

uint32_t DARrioLinkReqNResp(DAR_DEV_INFO_t *dev_info, uint8_t portnum,
		RIO_SPX_LM_RESP_STAT_T *link_stat);

/* Routing table support routines use the default routing table registers.
* 
*  The routing table routines assume that setting the routing table entry to 
*      an invalid port number will cause packets sent to that destID to be 
*      dropped.
*   
*  If the system has multiple hosts, these routines require that the host must
*      have used DARrioAquireDeviceLock() to gain ownership of the device.
*      DARrioReleaseDeviceLock() must be called to release ownership of the 
*      device.  The default implementations do not check for device lock, and 
*      assume that there is only one host in the system.
*  These routines are only valid if the device has switching capability.
*  Because this API is aimed at system bring up, there is no support 
*      for multicast.
*/
/* Use RIO_ALL_PORTS to cause packets with that route destid to be discarded.
*/
#define RIO_ALL_PORTS ((uint8_t)(0xFF))

uint32_t DARrioStdRouteAddEntry(DAR_DEV_INFO_t *dev_info,
		uint16_t routedestid, uint8_t routeportno);
uint32_t DARrioStdRouteGetEntry(DAR_DEV_INFO_t *dev_info,
		uint16_t routedestid, uint8_t *routeportno);

/* NEW DAR ROUTINE, initializes all routing table entries to use route portno.
 */
uint32_t DARrioStdRouteInitAll(DAR_DEV_INFO_t *dev_info,
		uint8_t routeportno);

/* NEW DAR ROUTINE, invalidates the routing for the route destid
 */
uint32_t DARrioStdRouteRemoveEntry(DAR_DEV_INFO_t *dev_info,
		uint16_t routedestid);

/* NEW DAR ROUTINE, sets the default route
 */
uint32_t DARrioStdRouteSetDefault(DAR_DEV_INFO_t *dev_info,
		uint8_t routeportno);

/* NEW DAR ROUTINE, sets assembly information
 */
uint32_t DARrioSetAssmblyInfo(DAR_DEV_INFO_t *dev_info,
		uint32_t AsmblyVendID, uint16_t AsmblyRev);

/* NEW DAR ROUTINE, gets assembly information
 */
uint32_t DARrioGetAssmblyInfo(DAR_DEV_INFO_t *dev_info,
		uint32_t *AsmblyVendID, uint16_t *AsmblyRev);

/* NEW DAR ROUTINE, lists of ports on a device.
 * if ptl_in->num_ports == RIO_ALL_PORTS,
		ptl_out contains a count, and list, of valid port numbers for the device.
   else
		ptl_out contains the same values as ptl_in. DARRioGetPortList checks 
		that all of the port numbers are valid for the device, and ensures that
		there are no duplicates.
	Note that the port numbers provided may not have any lanes mapped to them, 
	or have a trained link.
*/

struct DAR_ptl {
	uint8_t	num_ports; /* Number of valid entries in port */
	uint8_t	pnums[RIO_MAX_PORTS]; /* List of port numbers available */
};

#define PTL_ALL_PORTS { RIO_ALL_PORTS, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}
extern const struct DAR_ptl ptl_all_ports;

uint32_t DARrioGetPortList(DAR_DEV_INFO_t *dev_info,
		struct DAR_ptl *ptl_in, struct DAR_ptl *ptl_out);

/* NEW DAR ROUTINE, sets enumeration boundary bit for specified port(s)
*/

uint32_t DARrioSetEnumBound(DAR_DEV_INFO_t *dev_info,
		struct DAR_ptl *ptl, int enum_bnd_val);

/* NEW DAR ROUTINE, gets status of device reset initialization
   Returns either RIO_SUCCESS or a non-zero failure code.
*/
uint32_t DARrioGetDevResetInitStatus ( DAR_DEV_INFO_t *dev_info );

/* NEW DAR ROUTINE, controls Port Enable, Port Lockout and In/Out Port Enable
   for specified list of ports
*/
	uint32_t DARrioPortEnable(DAR_DEV_INFO_t *dev_info, struct DAR_ptl *ptl,
	bool port_ena,
	bool port_lkout,
	bool in_out_ena);

	/* NEW DAR ROUTINE, ensures that the PORT_LOCKOUT bit is set for
	 the specified port
	 */
	uint32_t DARrioEmergencyLockout(DAR_DEV_INFO_t *dev_info,
			uint8_t port_no);

/* Definitions to subdivide the uint32_t return code for implementation
 * specific routines
 */


/* Success status code for all DAR and DSF driver routines
 * is RIO_SUCCESS
 */
 
/* Warning codes for DSF routines */
#define DAR_FIRST_IMP_SPEC_WARNING  ((uint32_t)(0x0800))  

/*    Routine has succeeded, but something is a bit fishy.
*/
#define DAR_LAST_IMP_SPEC_WARNING   ((uint32_t)(0x0FFF))  

/* Error codes for DSF routines
*/
#define DAR_FIRST_IMP_SPEC_ERROR    ((uint32_t)(0x40000000)) 

/*    Routine has failed for an Implementation/device specific reason.
*/
#define DAR_LAST_IMP_SPEC_ERROR     ((uint32_t)(0x6FFFFFFF)) 

/* Definitions for DAR standard return codes.
   Success status code
*/
#define RIO_SUCCESS                 ((uint32_t)(0x0000)) 

/* DAR Standard warnings
   Used by rioRouteGetEntry, indicates that the routeportno returned
   is not the same for all ports
*/
#define RIO_WARN_INCONSISTENT       ((uint32_t)(0x0001))  

/* DAR Device Specific Function warnings
*/
#define RIO_DAR_IMP_SPEC_WARNING  DAR_FIRST_IMP_SPEC_WARNING

/* DAR Standard errors
*/

/* Another host has a higher priority
*/
#define RIO_ERR_SLAVE                ((uint32_t)(0x1001))  
/* One or more input parameters had an invalid value
*/
#define RIO_ERR_INVALID_PARAMETER    ((uint32_t)(0x1002))  
/* The RapidIO fabric returned a Response Packet with ERROR status reported
*/
#define RIO_ERR_RIO                  ((uint32_t)(0x1003))  
/* A device-specific hardware interface was unable to generate a maintenance
   transaction and reported an error
*/
#define RIO_ERR_ACCESS               ((uint32_t)(0x1004))  
/* Another host already acquired the specified processor element
*/
#define RIO_ERR_LOCK                 ((uint32_t)(0x1005))  
/* Device Access Routine does not provide services for this device
*/
#define RIO_ERR_NO_DEVICE_SUPPORT    ((uint32_t)(0x1006))  
/* Insufficient storage available in Device Access Routine private storage area
*/
#define RIO_ERR_INSUFFICIENT_RESOURCES  ((uint32_t)(0x1007))  
/* Switch cannot support requested routing
*/
#define RIO_ERR_ROUTE_ERROR          ((uint32_t)(0x1008))  
/* Target device is not a switch
*/
#define RIO_ERR_NO_SWITCH            ((uint32_t)(0x1009))  
/* Target device is not capable of the feature requested.
*/
#define RIO_ERR_FEATURE_NOT_SUPPORTED   ((uint32_t)(0x100A))  
/* Port value is not correct/acceptable to this routine
*/
#define RIO_ERR_BAD_PORT                ((uint32_t)(0x100B))
/* Value for default route register illegal/not supported
*/
#define RIO_ERR_BAD_DEFAULT_ROUTE       ((uint32_t)(0x100C))
/* Value for routing table initialize illegal/not supported
*/
#define RIO_ERR_BAD_DEFAULT_ROUTE_TABLE_PORT ((uint32_t)(0x100D))
/* Routing table values unintelligible/corrupted
*/
#define RIO_ERR_RT_CORRUPTED            ((uint32_t)(0x100E))
/* Port value for routing table illegal/not supported
*/
#define RIO_ERR_BAD_ROUTE_PORT          ((uint32_t)(0x100F))
/* Multicast mask value illegal/not supported
*/
#define RIO_ERR_BAD_MC_MASK             ((uint32_t)(0x1010))
/* Port has error conditions after attempting to clear errors
*/
#define RIO_ERR_ERRS_NOT_CL             ((uint32_t)(0x1011))
/* Routine was called with a pointer value of "NULL"
 * when this is not allowed.
*/
#define RIO_ERR_NULL_PARM_PTR           ((uint32_t)(0x1012))
/* Requested action cannot be performed due to current     
 * configuration of the device.
*/
#define RIO_ERR_NOT_SUP_BY_CONFIG       ((uint32_t)(0x1013))
/* Requested action cannot be performed due to an internal 
 * software error.                 
*/
#define RIO_ERR_SW_FAILURE              ((uint32_t)(0x1014))

/* Routine has been stubbed out...
*/
#define RIO_STUBBED                  ((uint32_t)(0xffff))  

/* Implementation specific routine has failed.  Check routine outputs for
       more information on failure.
*/
#define RIO_DAR_IMP_SPEC_FAILURE     ((uint32_t)(0x40000000))  

/* DAR DB Standard errors
   No devices have been bound to the DAR
*/
/* The DAR DB cannot accept more device drivers. See DAR_DB.h to increase 
 *       the size of the database.
 *       */
#define DAR_DB_NO_HANDLES            ((uint32_t)(0x70000002)) 
/* dev_info.dsf_h parameter passed in is invalid.
*/
#define DAR_DB_INVALID_HANDLE        ((uint32_t)(0x70000004))  
/* No device driver bound in supports the device.
*/
#define DAR_DB_NO_DRIVER             ((uint32_t)(0x70000006))  

/* Standard Register access errors
   Register Access Interface invalid
*/
#define RIO_ERR_REG_ACCESS_IF_UNKNOWN       ((uint32_t)(0x80000001))  
/* Register Access I2C File Descriptor is unknown
*/
#define RIO_ERR_REG_ACCESS_I2C_FD_UNKNOWN   ((uint32_t)(0x80000002))  
/* Register Access RIO File Descriptor is unknown
*/
#define RIO_ERR_REG_ACCESS_RIO_FD_UNKNOWN   ((uint32_t)(0x80000003))  
/* Private Data Structure is not defined
*/
#define RIO_ERR_PRIV_STRUCT_UNDEFINED       ((uint32_t)(0x80000004))  
/* Invalid a returned parameter
*/
#define RIO_ERR_INVALID_RETURN_PARAM        ((uint32_t)(0x80000005))  
/* The device is not an end-point
*/
#define RIO_ERR_NOT_ENDPOINT                ((uint32_t)(0x80000006))  
/* No function is supported
*/
#define RIO_ERR_NO_FUNCTION_SUPPORT         ((uint32_t)(0x80000007))  
/* Read Register return an invalid value
*/
#define RIO_ERR_READ_REG_RETURN_INVALID_VAL ((uint32_t)(0x80000008))  
/* Detected no Port OK
*/
#define RIO_ERR_PORT_OK_NOT_DETECTED        ((uint32_t)(0x80000009))  
/* No result returned
*/
#define RIO_ERR_RETURN_NO_RESULT            ((uint32_t)(0x8000000A))  
/* Abort function call
*/
#define RIO_ERR_ABORT_FUNCTION_CALL         ((uint32_t)(0x8000000B))
/* Not expected returned value
*/
#define RIO_ERR_NOT_EXPECTED_RETURN_VALUE   ((uint32_t)(0x8000000C))
/* No lane is available
*/
#define RIO_ERR_NO_LANE_AVAIL               ((uint32_t)(0x8000000D))
/* No port is available
*/
#define RIO_ERR_NO_PORT_AVAIL               ((uint32_t)(0x8000000E))
/* Port is unavailable
*/
#define RIO_ERR_PORT_UNAVAIL                ((uint32_t)(0x8000000F))
/* Illegal port number at index X in list of ports
 * Used by DARRioGetPortList.
*/
#define RIO_ERR_PORT_ILLEGAL(x)                ((uint32_t)(0x80000010+x))
/* No response for register access
*/
#define RIO_ERR_REG_ACCESS_FAIL             ((uint32_t)(0xFFFFFFFF))  

#ifdef __cplusplus
}
#endif

#endif /* __RAPIDIO_DEVICE_ACCESS_ROUTINES_API_H__ */
