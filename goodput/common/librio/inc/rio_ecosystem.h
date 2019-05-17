/*
****************************************************************************
Copyright (c) 2014, Integrated Device Technology Inc.
Copyright (c) 2014, Prodrive Technologies
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

THIS SOFTWARE IS PROVENDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
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

#ifndef __RIO_ECOSYSTEM_H__
#define __RIO_ECOSYSTEM_H__

/* Definitions for the RapidIO ecosystem, including vendors/devices and 
 * device port/lane limits.
 */

#include <stdint.h>
#include "rio_standard.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t rio_port_t;
typedef uint8_t rio_lane_t;

typedef uint32_t rio_mc_mask_t; /* Biggest mask available */

/** @brief max number of RIO mports supported by platform */
#define RIO_MAX_MPORTS 8

/** @brief max number of ports and lanes supported by silicon */
#define RIO_SW_PORT_INF_PORT_MAX ((RIO_SW_PORT_INF_T)(24))
#define RIO_SW_PORT_INF_LANE_MAX ((RIO_SW_PORT_INF_T)(48))
#define RIO_MAX_PORTS RIO_SW_PORT_INF_PORT_MAX

#define RIO_BAD_PORT_NUM(x) (x >= RIO_SW_PORT_INF_PORT_MAX)

#define RIO_ANY_PORT        ((rio_port_t)(0xff))
#define RIO_MAX_DEV_PORT    ((rio_port_t)(RIO_SW_PORT_INF_PORT_MAX))
#define RIO_MAX_PORT_LANES  ((rio_lane_t)(4))
#define RIO_MAX_DEV_LANES   ((rio_lane_t)(RIO_SW_PORT_INF_LANE_MAX))
#define RIO_MAX_MC_MASKS    RIO_RT_GRP_SZ

/** @brief max number of per-port statistics counters */
#define RIO_MAX_SC 40

/* RIO_DEV_IDENT : RIO_DEV_IDENT_VEND and RIO_DEV_IDENT_DEVI Values */

#define RIO_VEND_RESERVED        0xffff
#define RIO_DEVI_RESERVED        0xffff

#define RIO_VEND_FREESCALE      0x0002
#define RIO_VEND_TUNDRA         0x000d

#define RIO_DEVI_TSI500         0x0500
#define RIO_DEVI_TSI568         0x0568
#define RIO_DEVI_TSI57X         0x0570
#define RIO_DEVI_TSI572         0x0572
#define RIO_DEVI_TSI574         0x0574
#define RIO_DEVI_TSI576         0x0578 /* Same ID as Tsi578 */
#define RIO_DEVI_TSI577         0x0577
#define RIO_DEVI_TSI578         0x0578

#define RIO_VEND_TI             0x0030

#define RIO_VEND_IDT            0x0038

#define RIO_DEVI_IDT_CPS1848    0x0374
#define RIO_DEVI_IDT_CPS1432    0x0375
#define RIO_DEVI_IDT_CPS1616    0x0379
#define RIO_DEVI_IDT_VPS1616    0x0377
#define RIO_DEVI_IDT_SPS1616    0x0378

#define RIO_DEVI_IDT_TSI721     0x80ab

#define RIO_DEVI_IDT_RXSX       0x80e0
#define RIO_DEVI_IDT_RXS2448    0x80e6
#define RIO_DEVI_IDT_RXS1632    0x80e5

#define RIO_VEND_PRODRIVE       0x00a4

struct riocp_pe_vendor {
        uint16_t vid;
        const char *vendor;
};

struct riocp_pe_dev_id {
        uint16_t vid;
        uint16_t did;
        const char *name;
};

static const struct riocp_pe_vendor riocp_pe_vendors[] = {
        {0x0000,                "Reserved"},
        {0x0001,                "Mercury Computer Systems"},
        {RIO_VEND_FREESCALE,     "Freescale"},
        {0x0003,                "Alcatel Corporation"},
        {0x0005,                "EMC Corporation"},
        {0x0006,                "Ericsson"},
        {0x0007,                "Alcatel-Lucent Technologies"},
        {0x0008,                "Nortel Networks"},
        {0x0009,                "Altera"},
        {0x000a,                "LSA Corporation"},
        {0x000b,                "Rydal Research"},
        {RIO_VEND_TUNDRA,        "Tundra Semiconductor"},
        {0x000e,                "Xilinx"},
        {0x0019,                "Curtiss-Wright Controls Embedded Computing"},
        {0x001f,                "Raytheon Company"},
        {0x0028,                "VMetro"},
        {RIO_VEND_TI,            "Texas Instruments"},
        {0x0035,                "Cypress Semiconductor"},
        {0x0037,                "Cadence Design Systems"},
        {RIO_VEND_IDT,           "Integrated Device Technology"},
        {0x003d,                "Thales Computer"},
        {0x003f,                "Praesum Communications"},
        {0x0040,                "Lattice Semiconductor"},
        {0x0041,                "Honeywell Inc."},
        {0x005a,                "Jennic, Inc."},
        {0x0064,                "AMCC"},
        {0x0066,                "GDA Technologies"},
        {0x006a,                "Fabric Embedded Tools Corporation"},
        {0x006c,                "Silicon Turnkey Express"},
        {0x006e,                "Micro Memory"},
        {0x0072,                "PA Semi, Inc."},
        {0x0074,                "SRISA - Scientific Research Inst for System Analysis"},
        {0x0076,                "Nokia Siemens Networks"},
        {0x0079,                "Nokia Siemens Networks"},
        {0x007c,                "Hisilicon Technologies Co."},
        {0x007e,                "Creatuve Electronix Systems"},
        {0x0080,                "ELVEES"},
        {0x0082,                "GE Fanuc Embedded Systems"},
        {0x0084,                "Wintegra"},
        {0x0088,                "HDL Design House"},
        {0x008a,                "Motorola"},
        {0x008c,                "Cavium Networks"},
        {0x008e,                "Mindspeed Technologies"},
        {0x0094,                "Eclipse Electronic Systems, Inc."},
        {0x009a,                "Sandia National Laboratories"},
        {0x009e,                "HCL Technologies, Ltd."},
        {0x00a2,                "ASML"},
        {RIO_VEND_PRODRIVE,      "Prodrive Technologies"},
        {0x00a6,                "BAE Systems"},
        {0x00a8,                "Broadcom"},
        {0x00aa,                "Mobiveil, Inc."},
        {0xffff,                "Reserved"},
};

static const struct riocp_pe_dev_id riocp_pe_device_ids[] = {
        /* Prodrive */
        {0x0000, 0x5130, "QHA (domo capable)"},
        {0x0000, 0x5131, "QHA"},
        {0x0000, 0x5148, "QHA"},
        {0x0000, 0x4130, "AMCBTB"},
        {0x0000, 0x4131, "AMCBTB"},
        {0x0000, 0x0001, "SMA"},
        {0x0000, 0x534d, "SMA"},

        /* Freescale*/
        {RIO_VEND_FREESCALE, 0x0012, "MPC8548E"},
        {RIO_VEND_FREESCALE, 0x0013, "MPC8548"},
        {RIO_VEND_FREESCALE, 0x0014, "MPC8543E"},
        {RIO_VEND_FREESCALE, 0x0015, "MPC8543"},
        {RIO_VEND_FREESCALE, 0x0018, "MPC8547E"},
        {RIO_VEND_FREESCALE, 0x0019, "MPC8545E"},
        {RIO_VEND_FREESCALE, 0x001a, "MPC8545"},
        {RIO_VEND_FREESCALE, 0x0400, "P4080E"},
        {RIO_VEND_FREESCALE, 0x0401, "P4080"},
        {RIO_VEND_FREESCALE, 0x0408, "P4040E"},
        {RIO_VEND_FREESCALE, 0x0409, "P4040"},
        {RIO_VEND_FREESCALE, 0x0420, "P5020E"},
	{RIO_VEND_FREESCALE, 0x0421, "P5020"},
	{RIO_VEND_FREESCALE, 0x0428, "P5010E"},
	{RIO_VEND_FREESCALE, 0x0429, "P5010"},
        {RIO_VEND_FREESCALE, 0x1810, "MSC8151, MSC8152, MSC8154, MSC8251, MSC8252 or MSC8254"},
        {RIO_VEND_FREESCALE, 0x1812, "MSC8154E"},
        {RIO_VEND_FREESCALE, 0x1818, "MSC8156 or MSC8256"},
        {RIO_VEND_FREESCALE, 0x181a, "MSC8156E"},

        /* Tundra */
        {RIO_VEND_TUNDRA, RIO_DEVI_TSI500,        "Tsi500"},
        {RIO_VEND_TUNDRA, RIO_DEVI_TSI568,        "Tsi568"},
        {RIO_VEND_TUNDRA, RIO_DEVI_TSI572,        "Tsi572"},
        {RIO_VEND_TUNDRA, RIO_DEVI_TSI574,        "Tsi574"},
        {RIO_VEND_TUNDRA, RIO_DEVI_TSI577,        "Tsi577"},
        {RIO_VEND_TUNDRA, RIO_DEVI_TSI576,        "Tsi576"},
        {RIO_VEND_TUNDRA, RIO_DEVI_TSI578,        "Tsi578"},

        /* IDT */
        {RIO_VEND_IDT, RIO_DEVI_IDT_CPS1432,      "CPS1432"},
        {RIO_VEND_IDT, RIO_DEVI_IDT_CPS1848,      "CPS1848"},
        {RIO_VEND_IDT, RIO_DEVI_IDT_CPS1616,      "CPS1616"},
        {RIO_VEND_IDT, RIO_DEVI_IDT_SPS1616,      "SPS1616"},
        {RIO_VEND_IDT, RIO_DEVI_IDT_VPS1616,      "VPS1616"},
        {RIO_VEND_IDT, RIO_DEVI_IDT_TSI721,       "Tsi721"},

        {RIO_VEND_IDT, RIO_DEVI_IDT_RXS2448,      "RXS2448"},
        {RIO_VEND_IDT, RIO_DEVI_IDT_RXS1632,      "RXS1632"},

        /* Texas Instruments */
        {RIO_VEND_TI, 0x009e, "TMS320C6678"},
        {RIO_VEND_TI, 0xb981, "66AK2H12/06"},

        /* End of list */
        {RIO_VEND_RESERVED, RIO_DEVI_RESERVED, "Unknown"},
};

#ifdef __cplusplus
}
#endif

#endif /* __RIO_ECOSYSTEM_H__ */

