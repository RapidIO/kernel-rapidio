/*
****************************************************************************
Copyright (c) 2014, Integrated Device Technology Inc.
Copyright (c) 2014, RapidIO Trade Association
Copyright (c) 2017, Fabric Embedded Tools Corporation
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
#include <stdbool.h>

#include "rio_ecosystem.h"
#include "DAR_DB_Private.h"
#include "DSF_DB_Private.h"
#include "CPS_DeviceDriver.h"
#include "CPS1848.h"
#include "CPS1616.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CPS_DAR_WANTED

#define INVALID_PLL 0xFF
#define MAX_CPS_PLL 2
#define MAX_CPS_DEP_PORTS 3

#define CPS_ROOT_ACCESS        0xF20010
#define CPS_ROOT_ACCESS_PASSWD 0xFABADC05

#define CPS_LANE_X_CTL_1_ROOT(x) (0xF18000+(0x100*x))
#define CPS_LANE_X_CTL_1_ROOT_TX_INV 0x00000008
#define CPS_LANE_X_CTL_1_ROOT_RX_INV 0x00000002

#define CPS_MAX_TRACE_FILTER_MASK_VAL_BLKS 5
#define ARRAY_SIZE_CPS_RESET_REG_VALS 65

typedef struct reset_reg_vals_t_TAG {
	uint32_t val;		// Reset default value for register
	uint32_t mask;		// Bits to check in register.  Always write unmasked value.
	uint32_t addr;		// Address
	uint32_t addr_pinc;	// Address step per port number.
	uint32_t addr_linc;	// Address step per lane number.
	bool read_before;	// If true, read the register before writing the value
	bool check;		// If false, do not check the value of this register
				//    when verifying software operation.
} reset_reg_vals_t;

typedef struct set_cfg_port_info_t_TAG {
	uint32_t p_ctl1;
	uint32_t p_ops;
	uint32_t p_errstat;
	bool reset_reg_vals;
} set_cfg_port_info_t;

typedef struct set_cfg_lane_info_t_TAG {
	uint32_t l_ctl;
	uint32_t l_ctl_r;
	uint32_t l_stat_4;
} set_cfg_lane_info_t;

typedef struct set_cfg_glob_info_t_TAG {
	uint32_t quad_cfg;
	uint32_t shift_val;
	uint8_t quad_cfg_vals[4];
	uint32_t reset_vals[4];   // Contains the reset value to use when
				  //   changing the quadrant configuration
	uint32_t pll_ctl_vals[CPS_MAX_PLLS];
	uint8_t master_port; // If set to "RIO_ALL_PORTS", OOB register access is used to access the switch.
	uint8_t master_pll; // If set to "INVALID_PLL", OOB register access is used to access the switch.
} set_cfg_glob_info_t;

typedef struct set_cfg_info_t_TAG {
	set_cfg_glob_info_t glob_info;
	set_cfg_lane_info_t lanes[CPS_MAX_LANES];
	set_cfg_port_info_t ports[CPS_MAX_PORTS];
} set_cfg_info_t;

const cps_ports_per_quadrant_t cps1848_ppq = {{ //
		{{0, 4, 8, 12, 16}}, //
		{{1, 5, 9, 13, 17}}, //
		{{2, 6, 10, 14, RIO_ALL_PORTS}}, //
		{{3, 7, 11, 15, RIO_ALL_PORTS}}, //
	}};

const cps_ports_per_quadrant_t cps1616_ppq = {{ //
		{{ 0,  1,  2,  3, RIO_ALL_PORTS}}, //
		{{ 4,  5,  6,  7, RIO_ALL_PORTS}}, //
		{{ 8,  9, 10, 11, RIO_ALL_PORTS}}, //
		{{12, 13, 14, 15, RIO_ALL_PORTS}}, //
	}};

const cps_ports_per_quadrant_t cps1432_ppq = {{ //
		{{0, 4, 12, RIO_ALL_PORTS, RIO_ALL_PORTS}}, //
		{{1, 5, 13, RIO_ALL_PORTS, RIO_ALL_PORTS}}, //
		{{2, 6, 10, 14, RIO_ALL_PORTS}}, //
		{{3, 7, 11, 15, RIO_ALL_PORTS}}, //
	}};

const cps_ports_per_quadrant_t sps1616_ppq = {{ //
		{{0, 1, 2, 3, RIO_ALL_PORTS}}, //
		{{4, 5, 6, 7, RIO_ALL_PORTS}}, //
		{{8, 9, 10, 11, RIO_ALL_PORTS}}, //
		{{12, 13, 14, 15, RIO_ALL_PORTS}}, //
	}};

#define CONFIG_TERMINATOR \
  {RIO_ALL_PORTS, 4, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},    \
                       {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},    \
                       {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},    \
                       {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } }

cps_port_relations_t cps1848_cfg[] = {
  { 0, 0, { {4, 0, {          0, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 0, {          0, INVALID_PLL}, {           12, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 0, {          0, INVALID_PLL}, {           12, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 0, {          0, INVALID_PLL}, {           12,            16, RIO_ALL_PORTS}} } },
  { 1, 1, { {4, 4, {          1, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 4, {          1, INVALID_PLL}, {           13, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 4, {          1, INVALID_PLL}, {           13, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 4, {          1, INVALID_PLL}, {           13,            17, RIO_ALL_PORTS}} } },
  { 2, 2, { {4, 8, {          2, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 8, {          2, INVALID_PLL}, {           14, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 3, 3, { {4,12, {          3, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,12, {          3, INVALID_PLL}, {           15, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 4, 0, { {4,16, {          4, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,16, {          4, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,16, {          4, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,16, {          4, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 5, 1, { {4,20, {          5, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,20, {          5, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,20, {          5, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,20, {          5, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 6, 2, { {4,24, {          6, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,24, {          6, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 7, 3, { {4,28, {          7, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,28, {          7, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 8, 0, { {4,32, {          8, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,32, {          8, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,32, {          8, INVALID_PLL}, {           16, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,32, {          8, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 9, 1, { {4,36, {          9, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,36, {          9, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,36, {          9, INVALID_PLL}, {           17, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,36, {          9, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  {10, 2, { {4,40, {         10, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,40, {         10, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  {11, 3, { {4,44, {         11, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,44, {         11, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  {12, 0, { {0, 0, {          0, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 2, {          0, INVALID_PLL}, {            0, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 2, {          0, INVALID_PLL}, {            0, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 2, {          0, INVALID_PLL}, {            0,            16, RIO_ALL_PORTS}} } },
  {13, 1, { {0, 0, {          0, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 6, {          1, INVALID_PLL}, {            1, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 6, {          1, INVALID_PLL}, {            1, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 6, {          1, INVALID_PLL}, {            1,            17, RIO_ALL_PORTS}} } },
  {14, 2, { {0, 0, {          2, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,10, {          2, INVALID_PLL}, {            2, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  {15, 3, { {0, 0, {          3, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,14, {          3, INVALID_PLL}, {            3, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  {16, 0, { {0, 0, {          3, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {          3, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,34, {          8, INVALID_PLL}, {            8, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 3, {          0, INVALID_PLL}, {            0,            12, RIO_ALL_PORTS}} } },
  {17, 1, { {0, 0, {          3, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {          3, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,38, {          9, INVALID_PLL}, {            9, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 7, {          1, INVALID_PLL}, {            1,            13, RIO_ALL_PORTS}} } },
  CONFIG_TERMINATOR
};

cps_port_relations_t cps1616_cfg[] = {
  { 0, 0, { {4, 0, {          0,           1}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 0, {          0,           1}, {            2, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 0, {          0,           1}, {            2,             3, RIO_ALL_PORTS}},
            {1, 0, {          0,           1}, {            1,             2,             3}} } },
  { 1, 0, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 1, {          0,           1}, {            0,             2,             3}} } },
  { 2, 0, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 2, {          0,           1}, {            0, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 2, {          0,           1}, {            0,             2, RIO_ALL_PORTS}},
            {1, 2, {          0,           1}, {            0,             1,             3}} } },
  { 3, 0, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 3, {          0,           1}, {            0,             2, RIO_ALL_PORTS}},
            {1, 3, {          0,           1}, {            0,             1,             2}} } },
  { 4, 1, { {4, 4, {          2,           3}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 4, {          2,           3}, {            6, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 4, {          2,           3}, {            6,             7, RIO_ALL_PORTS}},
            {1, 4, {          2,           3}, {            5,             6,             7}} } },
  { 5, 1, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 5, {          2,           3}, {            4,             6,             7}} } },
  { 6, 1, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 6, {          2,           3}, {            4, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 6, {          2,           3}, {            4,             6, RIO_ALL_PORTS}},
            {1, 6, {          2,           3}, {            4,             5,             7}} } },
  { 7, 1, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 7, {          2,           3}, {            4,             6, RIO_ALL_PORTS}},
            {1, 7, {          2,           3}, {            4,             5,             6}} } },
  { 8, 2, { {4, 8, {          4,           5}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 8, {          4,           5}, {           10, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 8, {          4,           5}, {           10,            11, RIO_ALL_PORTS}},
            {1, 8, {          4,           5}, {            9,            10,            11}} } },
  { 9, 2, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 9, {          4,           5}, {            8,            10,            11}} } },
  {10, 2, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,10, {          4,           5}, {            8, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,10, {          4,           5}, {            8,            10, RIO_ALL_PORTS}},
            {1,10, {          4,           5}, {            8,             9,            11}} } },
  {11, 2, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,11, {          4,           5}, {            8,            10, RIO_ALL_PORTS}},
            {1,11, {          4,           5}, {            8,             9,            10}} } },

  {12, 3, { {4,12, {          6,           7}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,12, {          6,           7}, {           14, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,12, {          6,           7}, {           14,            15, RIO_ALL_PORTS}},
            {1,12, {          6,           7}, {           13,            14,            15}} } },
  {13, 3, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,13, {          6,           7}, {           12,            14,            15}} } },
  {14, 3, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,14, {          6,           7}, {           12, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,14, {          6,           7}, {           12,            14, RIO_ALL_PORTS}},
            {1,14, {          6,           7}, {           12,            13,            15}} } },
  {15, 3, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,15, {          6,           7}, {           12,            14, RIO_ALL_PORTS}},
            {1,15, {          6,           7}, {           12,            13,            14}} } },
  CONFIG_TERMINATOR
};

cps_port_relations_t cps1432_cfg[] = {
  { 0, 0, { {4, 0, {          0, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 0, {          0, INVALID_PLL}, {           12, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 1, 1, { {4, 4, {          1, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 4, {          1, INVALID_PLL}, {           13, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 2, 2, { {4, 8, {          2, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 8, {          2, INVALID_PLL}, {           14, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 8, {          2, INVALID_PLL}, {           14, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 8, {          2, INVALID_PLL}, {           10,            14, RIO_ALL_PORTS}} } },
  { 3, 3, { {4,12, {          3, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,12, {          3, INVALID_PLL}, {           15, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,12, {          3, INVALID_PLL}, {           15, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,12, {          3, INVALID_PLL}, {           11,            15, RIO_ALL_PORTS}} } },
  { 4, 0, { {4,16, {          4, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,16, {          4, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 5, 1, { {4,20, {          5, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,20, {          5, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 6, 2, { {4,24, {          6, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,24, {          6, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,24, {          6, INVALID_PLL}, {           10, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,24, {          6, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 7, 3, { {4,28, {          7, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,28, {          7, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,28, {          7, INVALID_PLL}, {           11, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {4,28, {          7, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 8, 0,
            { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  { 9, 0,
          { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  {10, 2, { {0, 0, {          0, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {          0, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,26, {          6, INVALID_PLL}, {            6, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,11, {          2, INVALID_PLL}, {            2,            14, RIO_ALL_PORTS}} } },
  {11, 3, { {0, 0, {          0, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {          0, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,30, {          7, INVALID_PLL}, {            7, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,15, {          3, INVALID_PLL}, {            3,            15, RIO_ALL_PORTS}} } },
  {12, 0, { {0, 0, {          0, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 2, {          0, INVALID_PLL}, {            0, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  {13, 1, { {0, 0, {          0, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 6, {          1, INVALID_PLL}, {            1, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}} } },
  {14, 2, { {0, 0, {          0, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,10, {          2, INVALID_PLL}, {            2, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,10, {          2, INVALID_PLL}, {            2, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,10, {          2, INVALID_PLL}, {            2,            10, RIO_ALL_PORTS}} } },
  {15, 3, { {0, 0, {          3, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,14, {          3, INVALID_PLL}, {            3, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,14, {          3, INVALID_PLL}, {            3, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,14, {          3, INVALID_PLL}, {            3,            11, RIO_ALL_PORTS}} } },
  CONFIG_TERMINATOR
};

cps_port_relations_t sps1616_cfg[] = {
  { 0, 0, { {4, 0, {          0,           1}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 0, {          0,           1}, {            2, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 0, {          0,           1}, {            2,             3, RIO_ALL_PORTS}},
            {1, 0, {          0,           1}, {            1,             2,             3}} } },
  { 1, 0, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 1, {          0,           1}, {            0,             2,             3}} } },
  { 2, 0, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 2, {          0,           1}, {            0, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 2, {          0,           1}, {            0,             3, RIO_ALL_PORTS}},
            {1, 2, {          0,           1}, {            0,             1,             3}} } },
  { 3, 0, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 3, {          0,           1}, {            0,             2, RIO_ALL_PORTS}},
            {1, 3, {          0,           1}, {            0,             1,             2}} } },
  { 4, 1, { {4, 4, {          2,           3}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 4, {          2,           3}, {            6, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 4, {          2,           3}, {            6,             7, RIO_ALL_PORTS}},
            {1, 4, {          2,           3}, {            5,             6,             7}} } },
  { 5, 1, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 5, {          2,           3}, {            4,             6,             7}} } },
  { 6, 1, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 6, {          2,           3}, {            4, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 6, {          2,           3}, {            4,             7, RIO_ALL_PORTS}},
            {1, 6, {          2,           3}, {            4,             5,             7}} } },
  { 7, 1, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 7, {          2,           3}, {            4,             6, RIO_ALL_PORTS}},
            {1, 7, {          2,           3}, {            4,             5,             6}} } },
  { 8, 2, { {4, 8, {          4,           5}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 8, {          4,           5}, {           10, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2, 8, {          4,           5}, {           10,            11, RIO_ALL_PORTS}},
            {1, 8, {          4,           5}, {            9,            10,            11}} } },
  { 9, 2, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1, 9, {          4,           5}, {            8,            10,            11}} } },
  {10, 2, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,10, {          4,           5}, {            8, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,10, {          4,           5}, {            8,            11, RIO_ALL_PORTS}},
            {1,10, {          4,           5}, {            8,             9,            11}} } },
  {11, 2, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,11, {          4,           5}, {            8,            10, RIO_ALL_PORTS}},
            {1,11, {          4,           5}, {            8,             9,            10}} } },
  {12, 3, { {4,12, {          6,           7}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,12, {          6,           7}, {           14, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,12, {          6,           7}, {           14,            15, RIO_ALL_PORTS}},
            {1,12, {          6,           7}, {           13,            14,            15}} } },
  {13, 3, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,13, {          6,           7}, {           12,            14,            15}} } },
  {14, 3, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {2,14, {          6,           7}, {           12, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,14, {          6,           7}, {           12,            15, RIO_ALL_PORTS}},
            {1,14, {          6,           7}, {           12,            13,            15}} } },
  {15, 3, { {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {0, 0, {INVALID_PLL, INVALID_PLL}, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS}},
            {1,15, {          6,           7}, {           12,            14, RIO_ALL_PORTS}},
            {1,15, {          6,           7}, {           12,            13,            14}} } },
  CONFIG_TERMINATOR
};

// Structure to track current configuration register values, and
// indications of which values must change.

// Init_sw_pi initializes constant and current configuration information
// about quadrant/port/lane/PLL relationships

uint32_t init_sw_pi(DAR_DEV_INFO_t *dev_info, cps_port_info_t *pi)
{
	uint32_t rc = RIO_ERR_NO_DEVICE_SUPPORT;
	uint32_t vend_id = dev_info->devID & RIO_DEV_IDENT_VEND;
	uint32_t dev_id = (dev_info->devID & RIO_DEV_IDENT_DEVI) >> 16;
	uint32_t shift_val = 2;
	uint8_t quad_idx;

	if (RIO_VEND_IDT != vend_id) {
		goto exit;
	}

	switch (dev_id) {
	case RIO_DEVI_IDT_CPS1848:
		pi->cpr = cps1848_cfg;
		pi->ppq = &cps1848_ppq;
		break;
	case RIO_DEVI_IDT_CPS1432:
		pi->cpr = cps1432_cfg;
		pi->ppq = &cps1432_ppq;
		break;
	case RIO_DEVI_IDT_CPS1616:
		pi->cpr = cps1616_cfg;
		pi->ppq = &cps1616_ppq;
		break;
	case RIO_DEVI_IDT_SPS1616:
		pi->cpr = sps1616_cfg;
		pi->ppq = &sps1616_ppq;
		shift_val = 4;
		break;
	default:
		goto exit;
	}

	// Get the quadrant configuration register value for all CPS GEN2 devices

	pi->bitshift = shift_val;

	rc = DARRegRead(dev_info, CPS1848_QUAD_CFG, &pi->quad_cfg);
	if ( RIO_SUCCESS != rc) {
		goto exit;
	}

	for (quad_idx = 0; quad_idx < 4; quad_idx++) {
		uint8_t bit_shift = quad_idx * shift_val;
		pi->quad_cfg_val[quad_idx] = (uint8_t)((pi->quad_cfg
				& (CPS1848_QUAD_CFG_QUAD0_CFG << bit_shift))
				>> bit_shift);
	}

	rc = RIO_SUCCESS;

exit:
	return rc;
}

static uint32_t cps_set_lrto(DAR_DEV_INFO_t *dev_info,
		rio_pc_set_config_in_t *in_parms)
{
	uint32_t lrto;

	// time = lrto * 100 / 352)
	//      = lrto * (5 * 5 * 4)/(32 * 11)
	//      = lrto * 25/(8 * 11)
	lrto = ((in_parms->lrto) * 25) / 88;

	if (lrto > 0xFFFFFF) {
		lrto = 0xFFFFFF;
	}

	// Never set LRTO to 0 on CPS devices.
	if (!lrto) {
		lrto = 1;
	}
	return DARRegWrite(dev_info, CPS1848_PORT_LINK_TO_CTL_CSR, lrto << 8);
}

static uint32_t cps_get_lrto(DAR_DEV_INFO_t *dev_info, rio_pc_set_config_out_t *out_parms)
{
	uint32_t rc;
	uint32_t lrto;

	rc = DARRegRead(dev_info, CPS1848_PORT_LINK_TO_CTL_CSR, &lrto);
	if (RIO_SUCCESS != rc) {
		return rc;
	}

	// 352 nsec = 0x160 = 11 * 0x20.  To avoid overflow in the computation below,
	// break it up into separate multiples and divides...
	lrto = lrto >> 8;
	lrto = ((lrto * 88) / 25);

	out_parms->lrto = lrto;

	return rc;
}


// Inputs  - requested configuration
//         - initialized port info structure
// Updates - sorted version of requested configuration, one entry for each port on the device
//         - current configuration of each port on the device
//         - Configuration register values for each port on the device
//         - Failure point if any of the above is mucked up
// Returns - status, RIO_SUCCESS or a failure type...

#define SPEED_GROUP(x) (((rio_pc_ls_1p25 == x) || (rio_pc_ls_2p5 == x) || (rio_pc_ls_5p0 == x))?0:((rio_pc_ls_3p125 == x) || (rio_pc_ls_6p25 == x))?1:2)

static uint32_t cps_set_config_init_parms_check_conflict(DAR_DEV_INFO_t *dev_info,
		rio_pc_set_config_in_t *in_parms, cps_port_info_t *pi,
		rio_pc_set_config_in_t *sorted, set_cfg_info_t *regs,
		uint32_t *fail_pt)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t quadrant;
	uint8_t port_idx;
	uint8_t pnum;
	uint8_t pll;
	uint8_t lane;
	bool found_flt;
	bool found_port;
	bool check;

	// Invalidate all sorted entries to start with...
	sorted->num_ports = NUM_CPS_PORTS(dev_info);
	for (port_idx = 0; port_idx < NUM_CPS_PORTS(dev_info); port_idx++) {
		sorted->pc[port_idx].pnum = RIO_ALL_PORTS;
	}

	if (RIO_ALL_PORTS == in_parms->num_ports) {
		for (pnum = 0; pnum < NUM_CPS_PORTS(dev_info); pnum++) {
			sorted->pc[pnum] = in_parms->pc[0];
			sorted->pc[pnum].pnum = pnum;
		}
	} else {
		if (in_parms->num_ports > NUM_CPS_PORTS(dev_info)) {
			*fail_pt = PC_SET_CONFIG(0x10);
			goto exit;
		}

		for (port_idx = 0; port_idx < in_parms->num_ports; port_idx++) {
			pnum = in_parms->pc[port_idx].pnum;
			if (pnum >= NUM_CPS_PORTS(dev_info)) {
				*fail_pt = PC_SET_CONFIG(0x11);
				goto exit;
			}

			sorted->pc[pnum] = in_parms->pc[port_idx];
		}
	}

	sorted->oob_reg_acc = in_parms->oob_reg_acc;
	sorted->reg_acc_port = in_parms->reg_acc_port;

	// Some basic checks for parameter validity...
	for (pnum = 0; pnum < NUM_CPS_PORTS(dev_info); pnum++) {
		if (sorted->pc[pnum].pnum == pnum) {
			if (sorted->pc[pnum].port_available) {
				check = (sorted->pc[pnum].pw >= rio_pc_pw_last);
				check |= (sorted->pc[pnum].ls >= rio_pc_ls_last);
				check |= (rio_lswap_none != sorted->pc[pnum].tx_lswap);
				check |= (rio_lswap_none != sorted->pc[pnum].rx_lswap);
				check |= (rio_pc_is_three == sorted->pc[pnum].iseq);
				if (check) {
					*fail_pt = PC_SET_CONFIG(0x12);
					goto exit;
				}
			} else {
				sorted->pc[pnum].pnum = RIO_ALL_PORTS;
			}
		}
	}

	// Confirm that there are no configuration conflicts for the requested parameters.
	// A configuration conflict exists if:
	// - It is not possible to determine a quadrant configuration value that supports the requested port widths
	// - It is not possible to pick a PLL control value for ports which depend on the same PLL.

	for (quadrant = 0; quadrant < 4; quadrant++) {
		uint8_t quad_cfg;
		found_port = false;
		// First, check to see if there are any ports to be configured
		// in this quadrant...
		for (port_idx = 0;
				(port_idx < CPS_MAX_QUADRANT_PORTS)
						&& (RIO_ALL_PORTS
								!= pi->ppq->qdrt[quadrant].port_num[port_idx])
						&& !found_port; port_idx++) {
			pnum = pi->ppq->qdrt[quadrant].port_num[port_idx];
			if (sorted->pc[pnum].pnum == pnum) {
				found_port = true;
			}
		}

		if (!found_port) {
			continue;
		}

		found_flt = true;

		// There is at least one port in this quadrant to be configured.
		// See if there is a quad config value that will support all ports
		// requested to be configured for this quadrant.
		for (quad_cfg = 0; (quad_cfg < 4) && found_flt; quad_cfg++) {
			found_flt = false;
			for (port_idx = 0;
					(port_idx < CPS_MAX_QUADRANT_PORTS)
							&& (RIO_ALL_PORTS
									!= pi->ppq->qdrt[quadrant].port_num[port_idx])
							&& !found_flt;
					port_idx++) {
				pnum =
						pi->ppq->qdrt[quadrant].port_num[port_idx];
				// When INVALID_PLL is the first PLL number, this indicates that the
				// quad_cfg value cannot be used.
				if (sorted->pc[pnum].pnum == pnum) {
					if ((INVALID_PLL
							== pi->cpr[pnum].cfg[quad_cfg].pll[0])) {
						found_flt = true;
					}
					if (pi->cpr[pnum].cfg[quad_cfg].lane_count
							< PW_TO_LANES(
									sorted->pc[pnum].pw)) {
						found_flt = true;
					}
				}
			}
			// If this quad config value works for the ports, check that it works for the PLL values.
			// There are PLL restrictions for CPS1848 and CPS1432, none for SPS1616.
			if (!found_flt
					&& !(RIO_DEVI_IDT_SPS1616
							== DEV_CODE(dev_info))
					&& !(RIO_DEVI_IDT_CPS1616
							== DEV_CODE(dev_info))) {
				uint8_t dep_pnum, dep_pidx;
				for (port_idx = 0;
						(port_idx
								< CPS_MAX_QUADRANT_PORTS)
								&& (RIO_ALL_PORTS
										!= pi->ppq->qdrt[quadrant].port_num[port_idx])
								&& !found_flt;
						port_idx++) {
					pnum =
							pi->ppq->qdrt[quadrant].port_num[port_idx];
					if ((pnum == sorted->pc[pnum].pnum)
							&& sorted->pc[pnum].powered_up) {
						// This port is being configured, and is requested to be available and powered up.
						// Check that the requested speed for this port is compatible with the other ports
						// dependent upon this PLL.
						for (dep_pidx = 0;
								(dep_pidx
										< MAX_CPS_DEP_PORTS)
										&& (RIO_ALL_PORTS
												!= pi->cpr[pnum].cfg[quad_cfg].other_ports[dep_pidx]);
								dep_pidx++) {
							dep_pnum = pi->cpr[pnum].cfg[quad_cfg].other_ports[dep_pidx];

							//@sonar:off - Collapsible "if" statements should be merged
							// If the ports are dependent upon the same PLL
							if ((dep_pnum
									== sorted->pc[dep_pnum].pnum)
									&& sorted->pc[dep_pnum].port_available
									&& sorted->pc[dep_pnum].powered_up) {
								if (pi->cpr[pnum].cfg[quad_cfg].pll[0]
										== pi->cpr[dep_pnum].cfg[quad_cfg].pll[0]) {
									if (SPEED_GROUP(
											sorted->pc[pnum].ls) != SPEED_GROUP( sorted->pc[dep_pnum].ls )) {
										found_flt =
												true;
									}
								}
							}
							//@sonar:on
						}
					}
				}
			}
		}
		// found_flt implies that no quad configuration value could
		// be found for this quadrant.
		if (found_flt) {
			rc = RIO_ERR_INVALID_PARAMETER;
			*fail_pt = PC_SET_CONFIG(0x20 + quadrant);
			goto exit;
		}
	}

	// Copy quadrant config values
	regs->glob_info.quad_cfg = pi->quad_cfg;
	for (quadrant = 0; quadrant < 4; quadrant++) {
		regs->glob_info.quad_cfg_vals[quadrant] =
				pi->quad_cfg_val[quadrant];
		regs->glob_info.reset_vals[quadrant] = 0;
	}

	// Get PLL register values
	for (pll = 0; pll < CPS_MAX_PLLS; pll++) {
		rc = DARRegRead(dev_info, CPS1848_PLL_X_CTL_1(pll),
				&regs->glob_info.pll_ctl_vals[pll]);
		if ( RIO_SUCCESS != rc) {
			*fail_pt = PC_SET_CONFIG(0x30);
			goto exit;
		}
	}

	// Read all lane registers
	rc = DARRegWrite(dev_info, CPS_ROOT_ACCESS, CPS_ROOT_ACCESS_PASSWD);
	if (RIO_SUCCESS != rc) {
		*fail_pt = PC_SET_CONFIG(0x31);
		goto exit;
	}

	for (lane = 0; lane < CPS_MAX_LANES; lane++) {
		rc = DARRegRead(dev_info, CPS1848_LANE_X_CTL(lane),
				&regs->lanes[lane].l_ctl);
		if (RIO_SUCCESS != rc) {
			*fail_pt = PC_SET_CONFIG(0x32);
			goto exit;
		}

		rc = DARRegRead(dev_info, CPS_LANE_X_CTL_1_ROOT(lane),
				&regs->lanes[lane].l_ctl_r);
		if (RIO_SUCCESS != rc) {
			*fail_pt = PC_SET_CONFIG(0x33);
			goto exit;
		}

		rc = DARRegRead(dev_info, CPS1848_LANE_X_STATUS_4_CSR(lane),
				&regs->lanes[lane].l_stat_4);
		if (RIO_SUCCESS != rc) {
			*fail_pt = PC_SET_CONFIG(0x33);
			goto exit;
		}
	}

	// Now read in all registers for ports being programmed...
	for (port_idx = 0; port_idx < NUM_CPS_PORTS(dev_info); port_idx++) {
		regs->ports[port_idx].reset_reg_vals = false;
		if (RIO_ALL_PORTS == sorted->pc[port_idx].pnum)
			continue;

		rc = DARRegRead(dev_info, CPS1848_PORT_X_CTL_1_CSR(port_idx),
				&regs->ports[port_idx].p_ctl1);
		if (RIO_SUCCESS != rc) {
			*fail_pt = PC_SET_CONFIG(0x34);
			goto exit;
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_OPS(port_idx),
				&regs->ports[port_idx].p_ops);
		if (RIO_SUCCESS != rc) {
			*fail_pt = PC_SET_CONFIG(0x34);
			goto exit;
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_STAT_CSR(port_idx),
				&regs->ports[port_idx].p_errstat);
		if (RIO_SUCCESS != rc) {
			*fail_pt = PC_SET_CONFIG(0x34);
			goto exit;
		}
	}

exit:
	return rc;
}

static uint32_t cps_compute_quad_config(cps_port_info_t *pi,
		rio_pc_set_config_in_t *sorted, set_cfg_info_t *chgd,
		uint32_t *fail_pt)
{
	uint32_t rc = RIO_SUCCESS;
	bool found_flt, found_port;
	uint8_t quad_val, quadrant, port_idx, pnum;

	// Figure out quadrant configuration, and which ports must be reset...
	// If the current quadrant config value supports the requested port config,
	// then no change/reset is requred...

	for (quadrant = 0; quadrant < 4; quadrant++) {
		found_port = false;
		found_flt = false;

		quad_val = chgd->glob_info.quad_cfg_vals[quadrant];
		for (port_idx = 0;
				(port_idx < CPS_MAX_QUADRANT_PORTS)
						&& (RIO_ALL_PORTS
								!= pi->ppq->qdrt[quadrant].port_num[port_idx])
						&& !found_flt; port_idx++) {
			pnum = pi->ppq->qdrt[quadrant].port_num[port_idx];
			//@sonar:off - Collapsible "if" statements should be merged
			if (sorted->pc[pnum].pnum == pnum) {
				found_port = true;
				if (sorted->pc[pnum].port_available) {
					if (pi->cpr[pnum].cfg[quad_val].lane_count
							< PW_TO_LANES(
									sorted->pc[pnum].pw))
						found_flt = true;
				}
			}
			//@sonar:on
		}

		if (!found_port) {
			continue;
		}

		// The current quadrant config value has to change.
		// Figure out what the value should be,
		// and the corresponding reset values.

		for (quad_val = 0; (quad_val < 4) && found_flt; quad_val++) {
			uint32_t rst_val;

			if (quad_val == chgd->glob_info.quad_cfg_vals[quadrant])
				continue;

			found_flt = false;
			rst_val = CPS1848_DEVICE_RESET_CTL_DO_RESET;
			for (port_idx = 0;
					(port_idx < CPS_MAX_QUADRANT_PORTS)
							&& (RIO_ALL_PORTS
									!= pi->ppq->qdrt[quadrant].port_num[port_idx])
							&& !found_flt;
					port_idx++) {
				pnum = pi->ppq->qdrt[quadrant].port_num[port_idx];

				//@sonar:off - Collapsible "if" statements should be merged
				if (pnum == sorted->pc[pnum].pnum) {
					rst_val |= (1 << pnum);
					if (sorted->pc[pnum].port_available
							&& sorted->pc[pnum].powered_up) {
						if (pi->cpr[pnum].cfg[quad_val].lane_count
								< PW_TO_LANES(
										sorted->pc[pnum].pw))
							found_flt = true;
					}
				}
				//@sonar:on
			}

			if (!found_flt) {
				chgd->glob_info.quad_cfg_vals[quadrant] =
						quad_val;
				chgd->glob_info.reset_vals[quadrant] = rst_val;
				break;
			}
		}
		// If found_flt is true at this point, it was not possible to compute
		// a quadrant configuration value.  This is a software error, because
		// the check routine should have ensured that there is at least one
		// quadrant configuration value.
		if (found_flt) {
			rc = RIO_ERR_SW_FAILURE;
			*fail_pt = PC_SET_CONFIG(0x50);
			goto exit;
		}
	}

	// Now that we know the quadrant configuration values, check that
	// - any newly available ports are marked to be powered down
	// - any now unavailable ports are removed from consideration

	for (quadrant = 0; quadrant < 4; quadrant++) {
		uint8_t old_qcfg = pi->quad_cfg_val[quadrant];
		uint8_t new_qcfg = chgd->glob_info.quad_cfg_vals[quadrant];

		if (old_qcfg == new_qcfg)
			continue;

		for (port_idx = 0; port_idx < CPS_MAX_QUADRANT_PORTS;
				port_idx++) {
			pnum = pi->ppq->qdrt[quadrant].port_num[port_idx];
			if (RIO_ALL_PORTS == pnum)
				continue;
			if (pnum >= CPS_MAX_PORTS) {
				rc = RIO_ERR_SW_FAILURE;
				*fail_pt = PC_SET_CONFIG(0x51);
				goto exit;
			}

			//@sonar:off - Collapsible "if" statements should be merged
			if (pi->cpr[pnum].cfg[old_qcfg].lane_count
					&& !pi->cpr[pnum].cfg[new_qcfg].lane_count) {
				if (RIO_ALL_PORTS == sorted->pc[pnum].pnum) {
					sorted->pc[pnum].pnum = pnum;
					sorted->pc[pnum].powered_up = false;
				}
			}
			//@sonar:on

			if (!pi->cpr[pnum].cfg[new_qcfg].lane_count) {
				sorted->pc[pnum].pnum = RIO_ALL_PORTS;
			}
		}
	}

exit:
	return rc;
}

// Computes baud rate change impacts for available ports.
// This differs between Indy/Bristol

static uint32_t cps_compute_baudrate_config(DAR_DEV_INFO_t *dev_info, cps_port_info_t *pi,
		rio_pc_get_config_out_t *curr, rio_pc_set_config_in_t *sorted,
		set_cfg_info_t *regs, set_cfg_info_t *chgd, uint32_t *fail_pt)
{
	uint32_t rc = RIO_ERR_SW_FAILURE;
	uint8_t quad_val, quadrant, pnum;
	uint8_t first_lane, last_lane, lane_num;

	for (pnum = 0; pnum < NUM_CPS_PORTS(dev_info); pnum++) {

		if ((RIO_ALL_PORTS == sorted->pc[pnum].pnum)
				|| !sorted->pc[pnum].port_available
				|| !sorted->pc[pnum].powered_up) {
			continue;
		}

		quadrant = pi->cpr[pnum].quadrant;
		quad_val = chgd->glob_info.quad_cfg_vals[quadrant];
		first_lane = pi->cpr[pnum].cfg[quad_val].first_lane;
		last_lane = pi->cpr[pnum].cfg[quad_val].lane_count + first_lane;
		if (!pi->cpr[pnum].cfg[quad_val].lane_count) {
			*fail_pt = PC_SET_CONFIG(0x60);
			goto exit;
		}

		// Determine if a port has just been powered up.
		// If so, make sure the registers are cleared to powerup values.
		if ((!(curr->pc[pnum].powered_up))
				&& sorted->pc[pnum].powered_up) {
			chgd->ports[pnum].reset_reg_vals = true;
		}

		for (lane_num = first_lane; lane_num < last_lane; lane_num++) {
			// If must set the registers to default values,
			// make sure the lane settings match...
			if (chgd->ports[pnum].reset_reg_vals) {
				chgd->lanes[lane_num].l_ctl &=
						(CPS1848_LANE_X_CTL_RX_RATE
								| CPS1848_LANE_X_CTL_TX_RATE);
				chgd->lanes[lane_num].l_ctl |= 0x000001e80;
				chgd->lanes[lane_num].l_ctl_r = 0x10;
				// NOTE: The value below does not match real reset value,
				// as link partner control of transmit emphasis should
				// always be disabled.
				chgd->lanes[lane_num].l_stat_4 = 0x10011388 &
						~CPS1848_LANE_X_STATUS_4_CSR_CTL_BY_LP_EN;
			}

			switch (sorted->pc[pnum].ls) {
			case rio_pc_ls_last: // Don't care about lane speed, don't try to program it
				break;
			case rio_pc_ls_1p25: // Lane speed 0 is selected by clearing TX & RX Rates
				chgd->lanes[lane_num].l_ctl &=
						~(CPS1848_LANE_X_CTL_RX_RATE
								| CPS1848_LANE_X_CTL_TX_RATE);
				chgd->lanes[lane_num].l_stat_4 =
						CPS1848_LANE_X_STATUS_4_CSR_CC_MONITOR_THRESH_10K;
				break;
			case rio_pc_ls_2p5:
			case rio_pc_ls_3p125:
				chgd->lanes[lane_num].l_ctl &=
						~(CPS1848_LANE_X_CTL_RX_RATE
								| CPS1848_LANE_X_CTL_TX_RATE);
				chgd->lanes[lane_num].l_ctl |=
						CPS1848_LANE_X_CTL_SPD1;
				chgd->lanes[lane_num].l_stat_4 =
						CPS1848_LANE_X_STATUS_4_CSR_CC_MONITOR_THRESH_10K;
				break;
			case rio_pc_ls_5p0:
			case rio_pc_ls_6p25:
				chgd->lanes[lane_num].l_ctl &=
						~(CPS1848_LANE_X_CTL_RX_RATE
								| CPS1848_LANE_X_CTL_TX_RATE);
				chgd->lanes[lane_num].l_ctl |=
						CPS1848_LANE_X_CTL_SPD2;
				chgd->lanes[lane_num].l_stat_4 =
						CPS1848_LANE_X_STATUS_4_CSR_CC_MONITOR_THRESH_10K
					| 	CPS1848_LANE_X_STATUS_4_CSR_CC_MONITOR_EN;
				break;
			default:
				*fail_pt = PC_SET_CONFIG(0x61);
				goto exit;
			}
			// Now set up PLL or LANE CTL depending on device type...
			if (sps1616_cfg == pi->cpr) {
				switch (sorted->pc[pnum].ls) {
				case rio_pc_ls_last: // Don't care about lane speed, don't try to program it
					break;
				case rio_pc_ls_1p25:
				case rio_pc_ls_2p5:
				case rio_pc_ls_5p0:
					chgd->lanes[lane_num].l_ctl &=
							~(CPS1616_LANE_X_CTL_RX_PLL_SEL
									| CPS1616_LANE_X_CTL_TX_PLL_SEL);
					break;
				case rio_pc_ls_3p125:
				case rio_pc_ls_6p25:
					chgd->lanes[lane_num].l_ctl |=
							(CPS1616_LANE_X_CTL_RX_PLL_SEL
									| CPS1616_LANE_X_CTL_TX_PLL_SEL);
					break;
				default:
					*fail_pt = PC_SET_CONFIG(0x62);
					goto exit;
				}
			}
		}
		if (sps1616_cfg != pi->cpr) {
			// Must set up the PLL selection register.
			// Must be sure that there is no conflict...
			uint32_t temp_pll_ctl;
			uint8_t pll_num;

			pll_num = pi->cpr[pnum].cfg[quad_val].pll[0];
			temp_pll_ctl = chgd->glob_info.pll_ctl_vals[pll_num];

			switch (sorted->pc[pnum].ls) {
			case rio_pc_ls_last: // Don't care about lane speed, don't try to program it
				break;
			case rio_pc_ls_1p25:
			case rio_pc_ls_2p5:
			case rio_pc_ls_5p0:
				temp_pll_ctl &=
						~CPS1848_PLL_X_CTL_1_PLL_DIV_SEL;
				break;
			case rio_pc_ls_3p125:
			case rio_pc_ls_6p25:
				temp_pll_ctl |= CPS1848_PLL_X_CTL_1_PLL_DIV_SEL;
				break;
			default:
				*fail_pt = PC_SET_CONFIG(0x63);
				goto exit;
			}

			if (temp_pll_ctl
					!= chgd->glob_info.pll_ctl_vals[pll_num]) {
				if (regs->glob_info.pll_ctl_vals[pll_num]
						!= chgd->glob_info.pll_ctl_vals[pll_num]) {
					// This condition is only true if we've changed the PLL control register,
					// and are now trying to change it back.  This should never happen.
					*fail_pt = PC_SET_CONFIG(0x64);
					goto exit;
				} else {

					chgd->glob_info.pll_ctl_vals[pll_num] =
							temp_pll_ctl;
				}
			}
		}
	}

	rc = RIO_SUCCESS;

exit:
	return rc;
}

static uint32_t cps_compute_laneswap_config(DAR_DEV_INFO_t *dev_info, cps_port_info_t *pi,
		rio_pc_set_config_in_t *sorted, set_cfg_info_t *chgd,
		uint32_t *fail_pt)
{
	uint32_t rc = RIO_ERR_SW_FAILURE;
	uint8_t quad_val, quadrant, pnum;
	uint8_t lnum, start_lane, end_lane;

	for (pnum = 0; pnum < NUM_CPS_PORTS(dev_info); pnum++) {
		if (RIO_ALL_PORTS == sorted->pc[pnum].pnum
				|| !sorted->pc[pnum].port_available
				|| !sorted->pc[pnum].powered_up) {
			continue;
		}

		quadrant = pi->cpr[pnum].quadrant;
		quad_val = chgd->glob_info.quad_cfg_vals[quadrant];
		start_lane = pi->cpr[pnum].cfg[quad_val].first_lane;
		end_lane = pi->cpr[pnum].cfg[quad_val].lane_count + start_lane;
		for (lnum = start_lane; lnum < end_lane; lnum++) {
			if (sorted->pc[pnum].tx_linvert[lnum - start_lane]) {
				chgd->lanes[lnum].l_ctl_r |=
						CPS_LANE_X_CTL_1_ROOT_TX_INV;
			} else {
				chgd->lanes[lnum].l_ctl_r &=
						~CPS_LANE_X_CTL_1_ROOT_TX_INV;
			}
			if (sorted->pc[pnum].rx_linvert[lnum - start_lane]) {
				chgd->lanes[lnum].l_ctl_r |=
						CPS_LANE_X_CTL_1_ROOT_RX_INV;
			} else {
				chgd->lanes[lnum].l_ctl_r &=
						~CPS_LANE_X_CTL_1_ROOT_RX_INV;
			}
		}

		// Also compute port configuration values...

		if (sorted->pc[pnum].xmitter_disable) {
			chgd->ports[pnum].p_ctl1 |=
					CPS1848_PORT_X_CTL_1_CSR_PORT_DIS;
		} else {
			chgd->ports[pnum].p_ctl1 &=
					~CPS1848_PORT_X_CTL_1_CSR_PORT_DIS;
		}

		if (sorted->pc[pnum].port_lockout) {
			chgd->ports[pnum].p_ctl1 |=
					CPS1848_PORT_X_CTL_1_CSR_PORT_LOCKOUT;
		} else {
			chgd->ports[pnum].p_ctl1 &=
					~CPS1848_PORT_X_CTL_1_CSR_PORT_LOCKOUT;
		}

		if (sorted->pc[pnum].nmtc_xfer_enable) {
			chgd->ports[pnum].p_ctl1 |=
					CPS1848_PORT_X_CTL_1_CSR_INPUT_PORT_EN
							| CPS1848_PORT_X_CTL_1_CSR_OUTPUT_PORT_EN;
		} else {
			// Note: 1432 & 1848 Rev C has errata about maintenance packet handling
			// OUTPUT_EN should always be set.
			if (DEV_CODE(dev_info) == RIO_DEVI_IDT_SPS1616) {
				chgd->ports[pnum].p_ctl1 &=
						~(CPS1848_PORT_X_CTL_1_CSR_INPUT_PORT_EN
								| CPS1848_PORT_X_CTL_1_CSR_OUTPUT_PORT_EN);
			} else {
				chgd->ports[pnum].p_ctl1 &=
						~CPS1848_PORT_X_CTL_1_CSR_INPUT_PORT_EN;
				chgd->ports[pnum].p_ctl1 |=
						CPS1848_PORT_X_CTL_1_CSR_OUTPUT_PORT_EN;
			}
		}

		//@sonar:off - c:S1871
		switch (sorted->pc[pnum].iseq) {
		case rio_pc_is_one:
			chgd->ports[pnum].p_errstat &=
					~CPS1848_PORT_X_ERR_STAT_CSR_IDLE2_EN;
			break;
		case rio_pc_is_two:
			chgd->ports[pnum].p_errstat |=
			CPS1848_PORT_X_ERR_STAT_CSR_IDLE2_EN;
			break;
		case rio_pc_is_dflt: /* No chnage to idle sequence */
			break;
		default:
			// Should never get here, illegal value that would be
			// detected as part of parameter checking
			break;
		}
		//@sonar:on

		//@sonar:off - c:S1871
		switch (sorted->pc[pnum].fc) {
		case rio_pc_fc_rx:
			chgd->ports[pnum].p_ops |=
					CPS1848_PORT_X_OPS_TX_FLOW_CTL_DIS;
			break;
		case rio_pc_fc_tx:
			chgd->ports[pnum].p_ops &=
					~CPS1848_PORT_X_OPS_TX_FLOW_CTL_DIS;
			break;
		case rio_pc_fc_last: /* No change to flow control setting */
		default:
			// Should never get here, illegal value that would be
			// detected as part of parameter checking
			break;
		}
		//@sonar:on

		// Must compute the override value
		chgd->ports[pnum].p_ctl1 &=
				~CPS1848_PORT_X_CTL_1_CSR_PWIDTH_OVRD;

		switch (pi->cpr[pnum].cfg[quad_val].lane_count) {
		default:
			*fail_pt = PC_SET_CONFIG(0x70);
			goto exit;

		case 1:
			break; // Single lane, nothing to do...

		case 2:
			switch (sorted->pc[pnum].pw) {
			case rio_pc_pw_1x:
			case rio_pc_pw_1x_l0:
				chgd->ports[pnum].p_ctl1 |=
						RIO_SPX_CTL_PTW_OVER_1X_L0;
				break;

			case rio_pc_pw_1x_l1:
			case rio_pc_pw_1x_l2:
				chgd->ports[pnum].p_ctl1 |=
						RIO_SPX_CTL_PTW_OVER_1X_LR;
				break;

			case rio_pc_pw_2x:
				break;

			//@sonar:off - c:S3458
			default:
			case rio_pc_pw_4x:
			//@sonar:on
				rc = RIO_ERR_INVALID_PARAMETER;
				*fail_pt = PC_SET_CONFIG(0x71);
				goto exit;
			}
			break;

		case 4:
			switch (sorted->pc[pnum].pw) {
			case rio_pc_pw_1x:
			case rio_pc_pw_1x_l0:
				chgd->ports[pnum].p_ctl1 |=
						RIO_SPX_CTL_PTW_OVER_1X_L0;
				break;

			case rio_pc_pw_1x_l1:
			case rio_pc_pw_1x_l2:
				chgd->ports[pnum].p_ctl1 |=
						RIO_SPX_CTL_PTW_OVER_1X_LR;
				break;

			case rio_pc_pw_2x:
				chgd->ports[pnum].p_ctl1 |=
						RIO_SPX_CTL_PTW_OVER_2X_NO_4X;
				break;

			case rio_pc_pw_4x:
				break;

			default:
				rc = RIO_ERR_INVALID_PARAMETER;
				*fail_pt = PC_SET_CONFIG(0x72);
				goto exit;
			}
			break;
		}
	}
	rc = RIO_SUCCESS;

exit:
	return rc;
}

static uint32_t cps_compute_powerdown_config(DAR_DEV_INFO_t *dev_info, cps_port_info_t *pi,
		rio_pc_set_config_in_t *sorted, set_cfg_info_t *chgd,
		uint32_t *fail_pt)
{
	uint32_t rc = RIO_ERR_SW_FAILURE;
	uint8_t quad_cfg, quadrant, pnum, pll_num;
	uint8_t lane_num, start_lane, end_lane;
	bool found_one;

	for (pnum = 0; pnum < NUM_CPS_PORTS(dev_info); pnum++) {
		if (!sorted->pc[pnum].port_available
				|| (RIO_ALL_PORTS == sorted->pc[pnum].pnum)) {
			continue;
		}

		quadrant = pi->cpr[pnum].quadrant;
		quad_cfg = chgd->glob_info.quad_cfg_vals[quadrant];
		pll_num = pi->cpr[pnum].cfg[quad_cfg].pll[0];
		start_lane = pi->cpr[pnum].cfg[quad_cfg].first_lane;
		end_lane = start_lane + pi->cpr[pnum].cfg[quad_cfg].lane_count;

		if (start_lane == end_lane) {
			*fail_pt = PC_SET_CONFIG(0x78);
			goto exit;
		}

		if (sorted->pc[pnum].powered_up) {
			// Make sure all of the lanes associated with this port are powered up.
			// Make sure the PLL associated with this port is powered up.

			chgd->glob_info.pll_ctl_vals[pll_num] &=
					~CPS1848_PLL_X_CTL_1_PLL_PWR_DOWN;
			for (lane_num = start_lane; lane_num < end_lane;
					lane_num++) {
				chgd->lanes[lane_num].l_ctl &=
						~CPS1848_LANE_X_CTL_LANE_DIS;
			}
		} else {
			// It is not possible to power down a CPS1848/1432 or SPS1616 port.
			// Fake it by disabling the lanes and, if possible, the PLL
			chgd->ports[pnum].p_ctl1 |=
					CPS1848_PORT_X_CTL_1_CSR_PORT_DIS;

			// Make sure all of the lanes associated with this port are powered down.
			// If all lanes associated with the PLL are powered down, power down the PLL.
			for (lane_num = start_lane; lane_num < end_lane;
					lane_num++) {
				chgd->lanes[lane_num].l_ctl |=
						CPS1848_LANE_X_CTL_LANE_DIS;
			}

			// Shortcut to get correct range of lanes for the PLL...
			start_lane &= ~(uint8_t)(CPS_MAX_PORT_LANES - 1);
			end_lane = start_lane + CPS_MAX_PORT_LANES;
			found_one = false;

			for (lane_num = start_lane;
					(lane_num < end_lane) && !found_one;
					lane_num++) {
				if (!(chgd->lanes[lane_num].l_ctl
						& CPS1848_LANE_X_CTL_LANE_DIS)) {
					found_one = true;
				}
			}
			if (!found_one) {
				chgd->glob_info.pll_ctl_vals[pll_num] |=
						CPS1848_PLL_X_CTL_1_PLL_PWR_DOWN;
			}
		}
	}

	rc = RIO_SUCCESS;

exit:
	return rc;
}

static uint32_t cps_set_config_compute_changes(DAR_DEV_INFO_t *dev_info,
		cps_port_info_t *pi, rio_pc_get_config_out_t *curr,
		rio_pc_set_config_in_t *sorted, set_cfg_info_t *regs,
		set_cfg_info_t *chgd, uint32_t *fail_pt)
{
	uint32_t rc;

	// Figure out quadrant configuration, and which ports must be reset...
	// If the current quadrant config value supports the requested port config,
	// then no change/reset is requred...

	// Error codes 0x50-0x5F
	rc = cps_compute_quad_config(pi, sorted, chgd, fail_pt);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	// Error codes 0x60-0x6F
	rc = cps_compute_baudrate_config(dev_info, pi, curr, sorted, regs, chgd,
			fail_pt);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	// Error codes 0x70-0x77
	rc = cps_compute_laneswap_config(dev_info, pi, sorted, chgd, fail_pt);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	// Error codes 0x78-0x7F
	rc = cps_compute_powerdown_config(dev_info, pi, sorted, chgd, fail_pt);

exit:
	return rc;
}

static uint32_t cps_set_config_preserve_host_port(DAR_DEV_INFO_t *dev_info,
		cps_port_info_t *pi, rio_pc_set_config_in_t *sorted,
		set_cfg_info_t *regs, set_cfg_info_t *chgd, uint32_t *fail_pt)
{
	uint32_t rc = RIO_SUCCESS;
	uint8_t quadrant, quad_cfg, chgd_quad_cfg, pnum;
	uint8_t first_lane, last_lane, lane_num;
	uint32_t err_n_stat, ctl1;

	chgd->glob_info.master_port = RIO_ALL_PORTS;
	chgd->glob_info.master_pll = INVALID_PLL;

	if (sorted->oob_reg_acc) {
		goto exit;
	}

	// Can't use the SWITCH_PORT_INFO register to determine which
	// port is in use to access the switch.  So, must pass in the
	// port number.  Check that the port number points to a valid
	// port which has PORT_OK status, no PORT_ERR, and does not
	// have LOCKOUT or PORT_DIS set.

	pnum = sorted->reg_acc_port;
	quadrant = pi->cpr[pnum].quadrant;
	quad_cfg = pi->quad_cfg_val[quadrant];

	if (!pi->cpr[pnum].cfg[quad_cfg].lane_count
			|| (INVALID_PLL == pi->cpr[pnum].cfg[quad_cfg].pll[0])) {
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_ERR_STAT_CSR(pnum),
			&err_n_stat);
	if (RIO_SUCCESS != rc) {
		*fail_pt = PC_SET_CONFIG(0x90);
		goto exit;
	}

	if (!(err_n_stat & CPS1848_PORT_X_ERR_STAT_CSR_PORT_OK)
			|| (err_n_stat & CPS1848_PORT_X_ERR_STAT_CSR_PORT_ERR)) {
		goto exit;
	}

	rc = DARRegRead(dev_info, CPS1848_PORT_X_CTL_1_CSR(pnum), &ctl1);
	if (RIO_SUCCESS != rc) {
		*fail_pt = PC_SET_CONFIG(0x90);
		goto exit;
	}

	if ((ctl1 & CPS1848_PORT_X_CTL_1_CSR_PORT_LOCKOUT)
			|| (ctl1 & CPS1848_PORT_X_CTL_1_CSR_PORT_DIS)) {
		goto exit;
	}

	// Probably using a port on the switch for access.
	// Make sure that this port and it's PLL are not reset or otherwise messed with...

	chgd_quad_cfg = chgd->glob_info.quad_cfg_vals[quadrant];
	regs->glob_info.master_port = chgd->glob_info.master_port = pnum;
	regs->glob_info.master_pll = chgd->glob_info.master_pll =
			pi->cpr[pnum].cfg[quad_cfg].pll[0];

	// Prevent disruptive changes to the master port...
	if (quad_cfg != chgd_quad_cfg) {
		if ((pi->cpr[pnum].cfg[quad_cfg].first_lane
				!= pi->cpr[pnum].cfg[chgd_quad_cfg].first_lane)
				|| (pi->cpr[pnum].cfg[quad_cfg].lane_count
						!= pi->cpr[pnum].cfg[chgd_quad_cfg].lane_count)
				|| (pi->cpr[pnum].cfg[quad_cfg].pll[0]
						!= pi->cpr[pnum].cfg[chgd_quad_cfg].pll[0])) {
			uint8_t dep_pnum, dep_pidx;

			// We're changing the lanes assigned to the master port, which will disrupt connectivity.
			// Back out the change to the quadrant configuration register.

			chgd->glob_info.quad_cfg_vals[quadrant] = quad_cfg;
			chgd->glob_info.reset_vals[quadrant] = 0;

			// Prevent any more changes by marking the master port,
			// and ports dependent on the same PLL as the master port,
			// as "do not touch"
			sorted->pc[pnum].pnum = RIO_ALL_PORTS;

			for (dep_pidx = 0;
					(dep_pidx < MAX_CPS_DEP_PORTS)
							&& (RIO_ALL_PORTS
									!= pi->cpr[pnum].cfg[chgd_quad_cfg].other_ports[dep_pidx]);
					dep_pidx++) {
				dep_pnum =
						pi->cpr[pnum].cfg[chgd_quad_cfg].other_ports[dep_pidx];
				sorted->pc[dep_pnum].pnum = RIO_ALL_PORTS;
			}
		} else {
			// This is not a disruptive change to the master port.
			// Make sure that the master port is not reset.

			chgd->glob_info.reset_vals[quadrant] &= ~((uint32_t)(1)
					<< chgd->glob_info.master_port);
		}
	}

	// Roll back PLL control register/lane control changes that affect the master port...
	if (chgd->glob_info.pll_ctl_vals[chgd->glob_info.master_pll]
			!= regs->glob_info.pll_ctl_vals[regs->glob_info.master_pll]) {
		chgd->glob_info.pll_ctl_vals[chgd->glob_info.master_pll] =
				regs->glob_info.pll_ctl_vals[regs->glob_info.master_pll];
		first_lane = pi->cpr[pnum].cfg[quad_cfg].first_lane
				& ~((uint8_t)(CPS_MAX_PORT_LANES - 1));
		last_lane = first_lane + CPS_MAX_PORT_LANES;
		for (lane_num = first_lane; lane_num < last_lane; lane_num++) {
			chgd->lanes[lane_num].l_ctl =
					regs->lanes[lane_num].l_ctl;
			chgd->lanes[lane_num].l_ctl_r =
					regs->lanes[lane_num].l_ctl_r;
			// NOTE: l_ctrl_4 does not affect master port operation.
			// Do not roll it back...
		}
	} else {
		// No PLL control change, roll back any lane speed/inverstion changes for this port
		first_lane = pi->cpr[pnum].cfg[quad_cfg].first_lane;
		last_lane = pi->cpr[pnum].cfg[quad_cfg].lane_count + first_lane;
		for (lane_num = first_lane; lane_num < last_lane; lane_num++) {
			chgd->lanes[lane_num].l_ctl =
					regs->lanes[lane_num].l_ctl;
			chgd->lanes[lane_num].l_ctl_r =
					regs->lanes[lane_num].l_ctl_r;
			// NOTE: l_ctrl_4 does not affect master port operation.
			// Do not roll it back...
		}
	}

	// Make sure the master ports PLL is not reset.
	chgd->glob_info.reset_vals[quadrant] &= ~((uint32_t)(1)
			<< (chgd->glob_info.master_pll + 18));

	// Make sure that this port does not get PORT_DIS or PORT_LOCKOUT set,
	// or change the port width override setting.
	// Or change idle sequence, or flow control setting, as found in err_stat
	// and ops respectively...
	chgd->ports[pnum].p_ctl1 &= ~(CPS1848_PORT_X_CTL_1_CSR_PORT_DIS
			| CPS1848_PORT_X_CTL_1_CSR_PWIDTH_OVRD |
			CPS1848_PORT_X_CTL_1_CSR_PORT_LOCKOUT);
	chgd->ports[pnum].p_ctl1 |= (regs->ports[pnum].p_ctl1
			& CPS1848_PORT_X_CTL_1_CSR_PWIDTH_OVRD);

	chgd->ports[pnum].p_errstat = regs->ports[pnum].p_errstat;
	chgd->ports[pnum].p_ops = regs->ports[pnum].p_ops;

	rc = RIO_SUCCESS;

exit:
	return rc;
}

reset_reg_vals_t CPS_reset_reg_vals[ARRAY_SIZE_CPS_RESET_REG_VALS] = {
//      Value    Address                                 pinc   linc  B4
   {0x00000000, 0xffffffff, CPS1848_PORT_X_LINK_MAINT_REQ_CSR(0) ,  0x020, 0x000, false, true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_LINK_MAINT_RESP_CSR(0),  0x020, 0x000, true , true},
   {0x80000000, 0x7FFFFFFF, CPS1848_PORT_X_LOCAL_ACKID_CSR(0)    ,  0x020, 0x000, false, true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_CTL_2_CSR(0)          ,  0x020, 0x000, false, true},
   {0xE0000002, 0xffffffff, CPS1848_PORT_X_ERR_STAT_CSR(0)       ,  0x020, 0x000, false, true},
   {0xD0400001, 0xffffffff, CPS1848_PORT_X_CTL_1_CSR(0)          ,  0x020, 0x000, false, true},
   {0x00000000, 0xffffffDf, CPS1848_PORT_X_IMPL_SPEC_ERR_DET(0)  ,  0x100, 0x000, false, true}, // Moved earlier to clear errors before
   {0xFF6FFFFF, 0xffffffff, CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(0),0x100, 0x000, false, true}, //    clearing ERR_DET, dont check link init event
   {0x00000000, 0x7fffffff, CPS1848_PORT_X_ERR_DET_CSR(0)        ,  0x040, 0x000, false, true}, // Dont check imp spec err, link_init event exists
   {0x00000000, 0xffffffff, CPS1848_PORT_X_ERR_RATE_EN_CSR(0)    ,  0x040, 0x000, false, true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_ATTR_CAPT_CSR(0)      ,  0x040, 0x000, false, true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_CAPT_0_CSR(0)         ,  0x040, 0x000, false, true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_CAPT_1_CSR(0)         ,  0x040, 0x000, false, true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_CAPT_2_CSR(0)         ,  0x040, 0x000, false, true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_CAPT_3_CSR(0)         ,  0x040, 0x000, false, true},
   {0x80000000, 0xffffffff, CPS1848_PORT_X_ERR_RATE_CSR(0)       ,  0x040, 0x000, false, true},
   {0xFFFF0000, 0xffffffff, CPS1848_PORT_X_ERR_RATE_THRESH_CSR(0),  0x040, 0x000, false, true},
   {0x00000000, 0xffffffff, CPS1848_LANE_X_STATUS_0_CSR(0)       ,  0x000, 0x020, true , false}, // Register values change
   {0x00000000, 0xffffffff, CPS1848_LANE_X_STATUS_1_CSR(0)       ,  0x000, 0x020, true , false}, // Register values change
   {0x00000000, 0xffffffff, CPS1848_LANE_X_STATUS_2_CSR(0)       ,  0x000, 0x020, false, true},
   {0xDFF80000, 0xffffffff, CPS1848_LANE_X_STATUS_3_CSR(0)       ,  0x000, 0x020, false, true},
   {0x80011388, 0xffffffff, CPS1848_LANE_X_STATUS_4_CSR(0)       ,  0x000, 0x020, false, false}, // idtPcSetConfig legitimately messes with these registers

   {0x000020C4, 0xffffffff, CPS1848_PORT_X_WM(0)                 ,  0x010, 0x000, false, true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_ERR_RPT_EN(0)         ,  0x040, 0x000, false, true},
   {0xFF6FFFFF, 0xffffffff, CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(0), 0x040, 0x000, false, true},
   {0x00000000, 0xffffffff, CPS1848_LANE_X_ERR_RPT_EN(0)         ,  0x000, 0x100, false, true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_PGC_MODE(0)           ,  0x010, 0x000, false, true},
// {0x00000000, 0xffffffff, CPS1848_PORT_X_PGC_DATA(0)           ,  0x010, 0x000, false, true},
   {0x000000DE, 0xffffffff, CPS1848_PORT_X_DEV_RTE_TABLE_Y(0,0)  ,  0x000, 0x000, false, true}, // Routing table implemented special
// {0x000000DE, 0xffffffff, CPS1848_PORT_X_DOM_RTE_TABLE_Y(0,0)  ,  0x000, 0x000, false, true}, // Routing table implemented special
   {0x00000000, 0xffffffff, CPS1848_PORT_X_TRACE_0_VAL_0(0)      ,  0x100, 0x000, false, true}, // Trace/filter registers implemented special
   {0x00000000, 0xffffffff, CPS1848_PORT_X_TRACE_0_MASK_0(0)     ,  0x100, 0x000, false, true}, // Trace/filter registers implemented special
   {0x00000000, 0xffffffff, CPS1848_PORT_X_MCAST_MASK_Y(0,0)     ,  0x000, 0x000, false, true}, // Multicast registers handled special

   {0x02400000, 0xffffffff, CPS1848_PORT_X_OPS(0)                ,  0x100, 0x000, false, true},
// {0x00000000, 0xffffffff, CPS1848_PORT_X_IMPL_SPEC_ERR_DET(0)  ,  0x100, 0x000, false, true}, // Moved earlier to clear errors before
// {0xFF6FFFFF, 0xffffffff, CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(0),0x100, 0x000, false, true}, //    clearing ERR_DET
   {0x00000000, 0xffffffff, CPS1848_PORT_X_VC0_PA_TX_CNTR(0)     ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_NACK_TX_CNTR(0)       ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_VC0_RTRY_TX_CNTR(0)   ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_VC0_PKT_TX_CNTR(0)    ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_TRACE_CNTR_0(0)       ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_TRACE_CNTR_1(0)       ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_TRACE_CNTR_2(0)       ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_TRACE_CNTR_3(0)       ,  0x100, 0x000, true , true},
   {0x00000000,	0xffffffff, CPS1848_PORT_X_FILTER_CNTR_0(0)      ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_FILTER_CNTR_1(0)      ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_FILTER_CNTR_2(0)      ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_FILTER_CNTR_3(0)      ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_NACK_RX_CNTR(0)       ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_VC0_RTRY_RX_CNTR(0)   ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_VC0_CPB_TX_CNTR(0)    ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_VC0_PKT_RX_CNTR(0)    ,  0x100, 0x000, true , true},

   {0x00000000, 0xffffffff, CPS1848_PORT_X_TRACE_PW_CTL(0)       ,  0x100, 0x000, false, true},
   {0x00000001, 0xffffffff, CPS1848_PORT_X_LANE_SYNC(0)          ,  0x100, 0x000, false, true},

   {0x00000000, 0xffffffff, CPS1848_PORT_X_VC0_PKT_DROP_RX_CNTR(0), 0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_VC0_PKT_DROP_TX_CNTR(0), 0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_VC0_TTL_DROP_CNTR(0)  ,  0x100, 0x000, true , true},
   {0x00000000, 0xffffffff, CPS1848_PORT_X_VC0_CRC_LIMIT_DROP_CNTR(0), 0x100, 0x000, true , true},
   {0xFFFF0000, 0xffffffff, CPS1848_PORT_X_RETRY_CNTR(0)         ,  0x100, 0x000, true , true},
   {0x00000004, 0xfffffffB, CPS1848_PORT_X_STATUS_AND_CTL(0)     ,  0x100, 0x000, false, true},
// {0x00000000, 0xffffffff, CPS1848_PLL_X_CTL_1(0)               ,  0x010, 0x000, false, true}, // Handled elsewhere by pc_set_cfg
// {0x00000000, 0xffffffff, CPS1848_PLL_X_CTL_2(0)               ,  0x010, 0x000, false, true}, // Handled elsewhere by pc_set_cfg

// {0x00000000, 0xffffffff, CPS1848_LANE_X_CTL(0)                ,  0x100, 0x000, false, true}, // Handled elsewhere by pc_set_cfg
   {0x7FFFFFFF, 0xffffffff, CPS1848_LANE_X_PRBS_GEN_SEED(0)      ,  0x000, 0x100, false, true},
   {0x00000000, 0xffffffff, CPS1848_LANE_X_PRBS_ERR_CNTR(0)      ,  0x000, 0x100, true , true},
   {0x00000000, 0xffffffff, CPS1848_LANE_X_ERR_DET(0)            ,  0x000, 0x100, false, false}, // Can detect errors at random during init
   {0x00000000, 0xffffffff, CPS1848_LANE_X_ERR_RATE_EN(0)        ,  0x000, 0x100, false, true},
   {0x00000000, 0xffffffff, CPS1848_LANE_X_ATTR_CAPT(0)          ,  0x000, 0x100, false, true},
   {0x00000000, 0xffffffff, CPS1848_LANE_X_DATA_CAPT_0(0)        ,  0x000, 0x100, false, true},
   {0x00000000, 0xffffffff, CPS1848_LANE_X_DATA_CAPT_1(0)        ,  0x000, 0x100, false, true},
   {0x00040555, 0xffffffff, CPS1848_LANE_X_DFE_1(0)              ,  0x000, 0x100, false, true},
   {0x10488010, 0xffffffff, CPS1848_LANE_X_DFE_2(0)              ,  0x000, 0x100, false, true}
};

static uint32_t cps_set_regs_to_reset_defaults(DAR_DEV_INFO_t *dev_info, uint8_t pnum,
		uint8_t start_lane, uint8_t end_lane, uint32_t *fail_pt)
{
	uint32_t rc = RIO_SUCCESS;
	int reg_idx;

	for (reg_idx = 0; reg_idx < ARRAY_SIZE_CPS_RESET_REG_VALS; reg_idx++) {
		if (CPS1848_PORT_X_DEV_RTE_TABLE_Y(0, 0)
				== CPS_reset_reg_vals[reg_idx].addr) {
			// Initialize routing table, and multicast masks, for port
			rio_rt_initialize_in_t in_parm;
			rio_rt_initialize_out_t out_parm;
			uint32_t temp;

			rc = DARRegRead(dev_info, CPS1848_RTE_DEFAULT_PORT_CSR,
					&temp);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
			in_parm.set_on_port = pnum;
			in_parm.default_route = (uint8_t)(temp &
				CPS1848_RTE_DEFAULT_PORT_CSR_DEFAULT_PORT);
			in_parm.default_route_table_port = RIO_RTE_DFLT_PORT;
			in_parm.update_hw = true;
			in_parm.rt = NULL;
			rc = rio_rt_initialize(dev_info, &in_parm, &out_parm);
			if (RIO_SUCCESS != rc) {
				*fail_pt = out_parm.imp_rc;
				goto exit;
			}
		} else {

			if (CPS1848_PORT_X_TRACE_0_VAL_0(0)
					== CPS_reset_reg_vals[reg_idx].addr) {
				int idx, lreg_idx;

				for (idx = 0; idx < CPS1848_PORT_MAX_TRACE_INST;
						idx++) {
					for (lreg_idx = 0;
							lreg_idx
									< CPS1848_PORT_MAX_TRACE_REGS;
							lreg_idx++) {
						rc =
								DARRegWrite(
										dev_info,
										CPS1848_PORT_P_TRACE_I_VAL_R(
												pnum,
												idx,
												lreg_idx),
										CPS_reset_reg_vals[lreg_idx].val);
						if (RIO_SUCCESS != rc) {
							*fail_pt = *fail_pt + 2;
							goto exit;
						}
					}
				}
			} else {
				if (CPS1848_PORT_X_TRACE_0_MASK_0(0)
						== CPS_reset_reg_vals[reg_idx].addr) {
					int idx, lreg_idx;
					for (idx = 0;
							idx
									< CPS1848_PORT_MAX_TRACE_INST;
							idx++) {
						for (lreg_idx = 0;
								lreg_idx
										< CPS1848_PORT_MAX_TRACE_REGS;
								lreg_idx++) {
							rc =
									DARRegWrite(
											dev_info,
											CPS1848_PORT_P_TRACE_I_MASK_R(
													pnum,
													idx,
													lreg_idx),
											CPS_reset_reg_vals[lreg_idx].val);
							if (RIO_SUCCESS != rc) {
								*fail_pt =
										*fail_pt
												+ 3;
								goto exit;
							}
						}
					}
				} else {
					if (CPS_reset_reg_vals[reg_idx].addr_linc) {
						uint8_t lnum;

						for (lnum = start_lane;
								lnum < end_lane;
								lnum++) {
							rc =
									DARRegWrite(
											dev_info,
											(CPS_reset_reg_vals[reg_idx].addr
													+ (CPS_reset_reg_vals[reg_idx].addr_pinc
															* pnum)
													+ (CPS_reset_reg_vals[reg_idx].addr_pinc
															* lnum)),
											CPS_reset_reg_vals[reg_idx].val);
							if (RIO_SUCCESS != rc) {
								*fail_pt =
										*fail_pt
												+ 4;
								goto exit;
							}
						}
					} else {
						rc =
								DARRegWrite(
										dev_info,
										CPS_reset_reg_vals[reg_idx].addr
												+ (CPS_reset_reg_vals[reg_idx].addr_pinc
														* pnum),
										CPS_reset_reg_vals[reg_idx].val);
						if (RIO_SUCCESS != rc) {
							*fail_pt = *fail_pt + 5;
							goto exit;
						}
					}
				}
			}
		}
	}
exit:
	return rc;
}

static uint32_t cps_set_config_write_changes(DAR_DEV_INFO_t *dev_info,
		cps_port_info_t *pi, rio_pc_set_config_in_t *sorted,
		set_cfg_info_t *regs, set_cfg_info_t *chgd, uint32_t *fail_pt)
{
	uint32_t rc;
	uint32_t quad_config = regs->glob_info.quad_cfg;
	uint8_t quadrant, quad_cfg, port_idx, pnum;
	uint8_t start_lane, end_lane, lane_num;

	// First step is to write all changes which cause PLL and port resets.
	// This includes quad config changes, PLL DIV_SEL changes, and port rate changes.
	// The algorithm is:
	// - Set the quadrant values
	//   - Write the quad config values
	//   - Reset the ports affected
	// - For each port which will have a PLL or lane rate change
	//   - Disable the port(s) affected by the PLL change
	//   - Change the PLL DIV_SEL
	//   - Change the lane rates, if necessary
	//   - Reset the port(s), and PLL.
	//   - Remove the port disable(s)
	//   - Write the updated port control register value

	for (quadrant = 0; quadrant < 4; quadrant++) {
		if (regs->glob_info.quad_cfg_vals[quadrant]
				== chgd->glob_info.quad_cfg_vals[quadrant]) {
			continue;
		}

		for (port_idx = 0;
				(port_idx < CPS_MAX_QUADRANT_PORTS)
						&& (RIO_ALL_PORTS
								!= pi->ppq->qdrt[quadrant].port_num[port_idx]);
				port_idx++) {
			pnum = pi->ppq->qdrt[quadrant].port_num[port_idx];
			if ((sorted->pc[pnum].pnum == pnum)
					&& (pnum != chgd->glob_info.master_port)) {
				rc =
						DARRegWrite(dev_info,
								CPS1848_PORT_X_CTL_1_CSR(
										pnum),
								regs->ports[pnum].p_ctl1
										| CPS1848_PORT_X_CTL_1_CSR_PORT_DIS);
				if (RIO_SUCCESS != rc) {
					*fail_pt = PC_SET_CONFIG(0x80);
					goto exit;
				}
			}
		}

		quad_config &= ~(CPS1848_QUAD_CFG_QUAD0_CFG
				<< (quadrant * pi->bitshift));
		quad_config |=
				(uint32_t)(chgd->glob_info.quad_cfg_vals[quadrant])
						<< (quadrant * pi->bitshift);

		rc = DARRegWrite(dev_info, CPS1848_QUAD_CFG, quad_config);
		if (RIO_SUCCESS != rc) {
			*fail_pt = PC_SET_CONFIG(0x81);
			goto exit;
		}

		rc = DARRegWrite(dev_info, CPS1848_DEVICE_RESET_CTL,
				chgd->glob_info.reset_vals[quadrant]);
		if (RIO_SUCCESS != rc) {
			*fail_pt = PC_SET_CONFIG(0x82);
			goto exit;
		}

		// Remove PORT_DIS...
		for (port_idx = 0;
				(port_idx < CPS_MAX_QUADRANT_PORTS)
						&& (RIO_ALL_PORTS
								!= pi->ppq->qdrt[quadrant].port_num[port_idx]);
				port_idx++) {
			pnum = pi->ppq->qdrt[quadrant].port_num[port_idx];
			if (sorted->pc[pnum].pnum == pnum) {
				// Write register twice to maintain port width override setting.
				rc = DARRegWrite(dev_info,
						CPS1848_PORT_X_CTL_1_CSR(pnum),
						regs->ports[pnum].p_ctl1);
				if (RIO_SUCCESS != rc) {
					*fail_pt = PC_SET_CONFIG(0x83);
					goto exit;
				}

				rc = DARRegWrite(dev_info,
						CPS1848_PORT_X_CTL_1_CSR(pnum),
						regs->ports[pnum].p_ctl1);
				if (RIO_SUCCESS != rc) {
					*fail_pt = PC_SET_CONFIG(0x84);
					goto exit;
				}
			}
		}
	}

	for (pnum = 0; pnum < NUM_CPS_PORTS(dev_info); pnum++) {
		uint32_t reset_val;
		uint8_t pll_num;
		bool make_chg, dep_ports;
		if (RIO_ALL_PORTS == sorted->pc[pnum].pnum) {
			continue;
		}

		reset_val = CPS1848_DEVICE_RESET_CTL_DO_RESET;
		quadrant = pi->cpr[pnum].quadrant;
		quad_cfg = chgd->glob_info.quad_cfg_vals[quadrant];
		pll_num = pi->cpr[pnum].cfg[quad_cfg].pll[0];
		make_chg = false;
		dep_ports = false;
		if (regs->glob_info.pll_ctl_vals[pll_num]
				!= chgd->glob_info.pll_ctl_vals[pll_num]) {
			make_chg = true;
			dep_ports = true;
			// Will reset pll, and the port
			reset_val |= 1 << (pll_num + 18);
			reset_val |= 1 << pnum;
		}

		start_lane = pi->cpr[pnum].cfg[quad_cfg].first_lane;
		end_lane = pi->cpr[pnum].cfg[quad_cfg].lane_count + start_lane;

		for (lane_num = start_lane; (lane_num < end_lane) && !make_chg;
				lane_num++) {
			if ((regs->lanes[lane_num].l_ctl
					!= chgd->lanes[lane_num].l_ctl)
					|| (regs->lanes[lane_num].l_ctl_r
							!= chgd->lanes[lane_num].l_ctl_r)) {
				make_chg = true;
				reset_val |= 1 << pnum;
			}
		}

		/* See if need to reset port for idle sequence or flow control change */
		if (regs->ports[pnum].p_errstat
				!= chgd->ports[pnum].p_errstat) {
			make_chg = true;
			reset_val |= 1 << pnum;
		}

		if (make_chg) {
			uint8_t dep_pidx, dep_pnum = 0xFF;
			uint32_t ctl1;

			if (dep_ports) {
				// Should never get here for SPS/CPS1616
				//
				// Make sure all dependent ports are disabled...
				for (dep_pidx = 0;
						(dep_pidx < MAX_CPS_DEP_PORTS)
								&& (RIO_ALL_PORTS
										!= pi->cpr[pnum].cfg[quad_cfg].other_ports[dep_pidx]);
						dep_pidx++) {
					dep_pnum =
							pi->cpr[pnum].cfg[quad_cfg].other_ports[dep_pidx];
					if (dep_pnum
							== chgd->glob_info.master_port) {
						continue;
					}

					if (RIO_ALL_PORTS
							== sorted->pc[dep_pnum].pnum) {
						rc =
								DARRegRead(
										dev_info,
										CPS1848_PORT_X_CTL_1_CSR(
												dep_pnum),
										&regs->ports[dep_pnum].p_ctl1);
						if (RIO_SUCCESS != rc) {
							*fail_pt =
									PC_SET_CONFIG(
											0x90);
							goto exit;
						}
					}
					ctl1 =
							regs->ports[dep_pnum].p_ctl1
									| CPS1848_PORT_X_CTL_1_CSR_PORT_DIS;
					rc =
							DARRegWrite(dev_info,
									CPS1848_PORT_X_CTL_1_CSR(
											dep_pnum),
									ctl1);
					if (RIO_SUCCESS != rc) {
						*fail_pt = PC_SET_CONFIG(0x91);
						goto exit;
					}
					reset_val |= 1 << dep_pnum;
				}
			}

			if (pnum != chgd->glob_info.master_port) {
				ctl1 =
						regs->ports[pnum].p_ctl1
								| CPS1848_PORT_X_CTL_1_CSR_PORT_DIS;
				rc = DARRegWrite(dev_info,
						CPS1848_PORT_X_CTL_1_CSR(pnum),
						ctl1);
				if (RIO_SUCCESS != rc) {
					*fail_pt = PC_SET_CONFIG(0x92);
					goto exit;
				}
			}

			// All ports are disabled, now start to update PLLs and lanes...
			//
			// If moving from speed group 1 to speed group 0, write the PLL DIV SEL register first
			if ((regs->glob_info.pll_ctl_vals[pll_num]
					!= chgd->glob_info.pll_ctl_vals[pll_num])
					&& !(chgd->glob_info.pll_ctl_vals[pll_num]
							& CPS1848_PLL_X_CTL_1_PLL_DIV_SEL)) {
				if ((sps1616_cfg == pi->cpr)
						&& ((regs->glob_info.pll_ctl_vals[pll_num]
								^ chgd->glob_info.pll_ctl_vals[pll_num])
								& CPS1848_PLL_X_CTL_1_PLL_DIV_SEL)) {
					rc = RIO_ERR_SW_FAILURE;
					*fail_pt = PC_SET_CONFIG(0x93);
					goto exit;
				}

				rc =
						DARRegWrite(dev_info,
								CPS1848_PLL_X_CTL_1(
										pll_num),
								chgd->glob_info.pll_ctl_vals[pll_num]);
				if (RIO_SUCCESS != rc) {
					*fail_pt = PC_SET_CONFIG(0x94);
					goto exit;
				}
				// Stop dependent ports, if any, from redoing the same change.
				regs->glob_info.pll_ctl_vals[pll_num] =
						chgd->glob_info.pll_ctl_vals[pll_num];
			}

			for (lane_num = start_lane; lane_num < end_lane;
					lane_num++) {
				if (regs->lanes[lane_num].l_ctl
						!= chgd->lanes[lane_num].l_ctl) {
					rc =
							DARRegWrite(dev_info,
									CPS1848_LANE_X_CTL(
											lane_num),
									chgd->lanes[lane_num].l_ctl);
					if (RIO_SUCCESS != rc) {
						*fail_pt = PC_SET_CONFIG(0x95);
						goto exit;
					}
					// Stop dependent ports, if any, from redoing the same change.
					regs->lanes[lane_num].l_ctl =
							chgd->lanes[lane_num].l_ctl;
				}

				if (regs->lanes[lane_num].l_ctl_r
						!= chgd->lanes[lane_num].l_ctl_r) {
					rc =
							DARRegWrite(dev_info,
									CPS_LANE_X_CTL_1_ROOT(
											lane_num),
									chgd->lanes[lane_num].l_ctl_r);
					if (RIO_SUCCESS != rc) {
						*fail_pt = PC_SET_CONFIG(0x96);
						goto exit;
					}
					// Stop dependent ports, if any, from redoing the same change.
					regs->lanes[lane_num].l_ctl_r =
							chgd->lanes[lane_num].l_ctl_r;
				}
				// Note: l_stat_4 is handled at the end for each port
			}

			if (dep_ports) {
				// Update dependent lanes at the same time...
				for (dep_pidx = 0;
						(dep_pidx < MAX_CPS_DEP_PORTS)
								&& (RIO_ALL_PORTS
										!= pi->cpr[pnum].cfg[quad_cfg].other_ports[dep_pidx]);
						dep_pidx++) {
					dep_pnum =
							pi->cpr[pnum].cfg[quad_cfg].other_ports[dep_pidx];
					if ((RIO_ALL_PORTS
							== sorted->pc[dep_pnum].pnum)
							|| (dep_pnum
									== chgd->glob_info.master_port)) {
						continue;
					}

					start_lane =
							pi->cpr[dep_pnum].cfg[quad_cfg].first_lane;
					end_lane =
							pi->cpr[dep_pnum].cfg[quad_cfg].lane_count
									+ start_lane;

					for (lane_num = start_lane;
							lane_num < end_lane;
							lane_num++) {
						if (regs->lanes[lane_num].l_ctl
								!= chgd->lanes[lane_num].l_ctl) {
							rc =
									DARRegWrite(
											dev_info,
											CPS1848_LANE_X_CTL(
													lane_num),
											chgd->lanes[lane_num].l_ctl);
							if (RIO_SUCCESS != rc) {
								*fail_pt =
										PC_SET_CONFIG(
												0x97);
								goto exit;
							}
							// Stop dependent ports, if any, from redoing the same change.
							regs->lanes[lane_num].l_ctl =
									chgd->lanes[lane_num].l_ctl;
						}

						if (regs->lanes[lane_num].l_ctl_r
								!= chgd->lanes[lane_num].l_ctl_r) {
							rc =
									DARRegWrite(
											dev_info,
											CPS_LANE_X_CTL_1_ROOT(
													lane_num),
											chgd->lanes[lane_num].l_ctl_r);
							if (RIO_SUCCESS != rc) {
								*fail_pt =
										PC_SET_CONFIG(
												0x98);
								goto exit;
							}
							// Stop dependent ports, if any, from redoing the same change.
							regs->lanes[lane_num].l_ctl_r =
									chgd->lanes[lane_num].l_ctl_r;
						}
						// Note: l_stat_4 changes are made at the end for each port.
					}
				}
			}

			// If moving from speed group 0 to speed group 1, write the PLL DIV SEL register last
			if ((regs->glob_info.pll_ctl_vals[pll_num]
					!= chgd->glob_info.pll_ctl_vals[pll_num])
					&& (chgd->glob_info.pll_ctl_vals[pll_num]
							& CPS1848_PLL_X_CTL_1_PLL_DIV_SEL)) {
				if (sps1616_cfg == pi->cpr) {
					rc = RIO_ERR_SW_FAILURE;
					*fail_pt = PC_SET_CONFIG(0xA0);
					goto exit;
				}

				rc =
						DARRegWrite(dev_info,
								CPS1848_PLL_X_CTL_1(
										pll_num),
								chgd->glob_info.pll_ctl_vals[pll_num]);
				if (RIO_SUCCESS != rc) {
					*fail_pt = PC_SET_CONFIG(0xA1);
					goto exit;
				}
				// Stop dependent ports, if any, from redoing the same change.
				regs->glob_info.pll_ctl_vals[pll_num] =
						chgd->glob_info.pll_ctl_vals[pll_num];
			}

			// Update errstat registers

			if (regs->ports[pnum].p_errstat
					!= chgd->ports[pnum].p_errstat) {
				rc = DARRegWrite(dev_info,
						CPS1848_PORT_X_ERR_STAT_CSR(
								pnum),
						chgd->ports[pnum].p_errstat);
				if (RIO_SUCCESS != rc) {
					*fail_pt = PC_SET_CONFIG(0xB9);
					goto exit;
				}
			}
			// Lanes and PLL controls are all updated,
			// time to reset the PLLs and lanes...

			rc = DARRegWrite(dev_info, CPS1848_DEVICE_RESET_CTL,
					reset_val);
			if (RIO_SUCCESS != rc) {
				*fail_pt = PC_SET_CONFIG(0xB0);
				goto exit;
			}

			// If we have to restore the reset default register values for a port,
			// then do so before removing port disable.

			// Failure points 0xB1, B2, B3 and B7
			if ((dep_pnum != 0xFF)
					&& regs->ports[dep_pnum].reset_reg_vals) {
				*fail_pt = PC_SET_CONFIG(0xB1);
				rc = cps_set_regs_to_reset_defaults(dev_info, pnum,
						start_lane, end_lane, fail_pt);
				if (RIO_SUCCESS != rc) {
					goto exit;
				}
			}

			// And finally, remove the "port disable" as necessary...
			if (dep_ports) {
				for (dep_pidx = 0;
						(dep_pidx < MAX_CPS_DEP_PORTS)
								&& (RIO_ALL_PORTS
										!= pi->cpr[pnum].cfg[quad_cfg].other_ports[dep_pidx]);
						dep_pidx++) {
					dep_pnum =
							pi->cpr[pnum].cfg[quad_cfg].other_ports[dep_pidx];
					rc =
							DARRegWrite(dev_info,
									CPS1848_PORT_X_CTL_1_CSR(
											dep_pnum),
									regs->ports[dep_pnum].p_ctl1);
					if (RIO_SUCCESS != rc) {
						*fail_pt = PC_SET_CONFIG(0xB8);
						goto exit;
					}
				}
			}

			rc = DARRegWrite(dev_info,
					CPS1848_PORT_X_CTL_1_CSR(pnum),
					regs->ports[pnum].p_ctl1);
			if (RIO_SUCCESS != rc) {
				*fail_pt = PC_SET_CONFIG(0xB9);
				goto exit;
			}
		}

		// Set lane clock compensation sequence monitoring.
		// This can be changed without affecting normal port operation.
		for (lane_num = start_lane; lane_num < end_lane; lane_num++) {
			if ((regs->lanes[lane_num].l_stat_4 != chgd->lanes[lane_num].l_stat_4)) {
				rc = DARRegWrite(dev_info, CPS1848_LANE_X_STATUS_4_CSR(lane_num), chgd->lanes[lane_num].l_stat_4);
				if (RIO_SUCCESS != rc) {
					*fail_pt = PC_SET_CONFIG(0xBA);
					goto exit;
				}
			}
		}
		// NOTE: Always write the register twice, once to remove PORT_DIS, the second time
		// to set the port-width override value.
		//
		// Do this to ensure that the port-width override setting is restored after any
		// PORT_DIS activities above...
		rc = DARRegWrite(dev_info, CPS1848_PORT_X_CTL_1_CSR(pnum),
				chgd->ports[pnum].p_ctl1);
		if (RIO_SUCCESS != rc) {
			*fail_pt = PC_SET_CONFIG(0xBB);
			goto exit;
		}

		rc = DARRegWrite(dev_info, CPS1848_PORT_X_CTL_1_CSR(pnum),
				chgd->ports[pnum].p_ctl1);
		if (RIO_SUCCESS != rc) {
			*fail_pt = PC_SET_CONFIG(0xBC);
			goto exit;
		}

		/* And lastly, if we're changing flow control operation, update the
		 * ops register and do a force_reinit.
		 */
		if (regs->ports[pnum].p_ops != chgd->ports[pnum].p_ops) {
			chgd->ports[pnum].p_ops |=
					CPS1848_PORT_X_OPS_FORCE_REINIT;

			rc = DARRegWrite(dev_info, CPS1848_PORT_X_OPS(pnum),
					chgd->ports[pnum].p_ops);
			if (RIO_SUCCESS != rc) {
				*fail_pt = PC_SET_CONFIG(0xBD);
				goto exit;
			}
		}
	}

	rc = RIO_SUCCESS;
exit:
	return rc;
}

static uint32_t cps_sps1616_port_err_clear(DAR_DEV_INFO_t *dev_info,
		rio_pc_clr_errs_in_t *in_parms,
		rio_pc_clr_errs_out_t *out_parms)
{

	out_parms->imp_rc =
			PC_CLR_ERRS(
					0x30 + in_parms->port_num + out_parms->imp_rc + SWITCH(dev_info));
	return RIO_STUBBED;
}

static uint32_t cps_set_int_pw_ignore_reset(DAR_DEV_INFO_t *dev_info, uint8_t port_num,
		rio_pc_rst_handling *policy_out, uint32_t *fail_pt)
{
	uint32_t rc = RIO_SUCCESS;
	uint8_t pnum, start_port, end_port;
	uint32_t regVal;

	if (RIO_ALL_PORTS == port_num) {
		start_port = 0;
		end_port = NUM_CPS_PORTS(dev_info) - 1;
	} else {
		start_port = end_port = port_num;
	}

	if (( RIO_VEND_IDT == VEND_CODE(dev_info))
			&& ((RIO_DEVI_IDT_SPS1616 == DEV_CODE(dev_info))
					|| (RIO_DEVI_IDT_CPS1616
							== DEV_CODE(dev_info)))) {
		*policy_out = rio_pc_rst_ignore;

		for (pnum = start_port; pnum <= end_port; pnum++) {
			rc = DARRegRead(dev_info,
					CPS1616_PORT_X_STATUS_AND_CTL(pnum),
					&regVal);
			if (RIO_SUCCESS != rc) {
				*fail_pt = 1;
				goto exit;
			}

			rc =
					DARRegWrite(dev_info,
							CPS1616_PORT_X_STATUS_AND_CTL(
									pnum),
							regVal
									| CPS1616_PORT_X_STATUS_AND_CTL_IGNORE_RST_CS);
			if (RIO_SUCCESS != rc) {
				*fail_pt = 2;
				goto exit;
			}
		}
	} else {
		*policy_out = rio_pc_rst_port;
		rc = DARRegRead(dev_info, CPS1848_DEVICE_CTL_1, &regVal);
		if (RIO_SUCCESS != rc) {
			*fail_pt = 3;
			goto exit;
		}

		rc = DARRegWrite(dev_info, CPS1848_DEVICE_CTL_1,
				regVal | CPS1848_DEVICE_CTL_1_PORT_RST_CTL);
		if (RIO_SUCCESS != rc) {
			*fail_pt = 4;
			goto exit;
		}
	}

exit:
	return rc;
}

typedef struct filt_n_trace_mask_n_val_t_TAG {
	uint32_t mask;
	uint32_t value;
} filt_n_trace_mask_n_val_t;

filt_n_trace_mask_n_val_t mtc_filter_on[CPS_MAX_TRACE_FILTER_MASK_VAL_BLKS] = {{
		0x000F0000, 0x00080000}, //
		{0, 0}, //
		{0, 0}, //
		{0, 0}, //
		{0, 0}};

filt_n_trace_mask_n_val_t mtc_filter_off[CPS_MAX_TRACE_FILTER_MASK_VAL_BLKS] = {
		{0, 0}, //
		{0, 0}, //
		{0, 0}, //
		{0, 0}, //
		{0, 0}};


// Port Config
//
uint32_t CPS_rio_pc_get_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_config_in_t *in_parms,
		rio_pc_get_config_out_t *out_parms)
{
	uint32_t rc;
	uint8_t pnum, port_idx;
	uint8_t quad_cfg;
	uint8_t lane_num, first_lane, last_lane, pll_num;
	cps_port_info_t pi;
	uint32_t pll_ctl1, lane_ctl, spx_ctl1, spx_errstat, spx_ops;
	struct DAR_ptl good_ptl;

	out_parms->num_ports = 0;
	out_parms->imp_rc = 0;

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_GET_CONFIG(1);
		goto exit;
	}

	out_parms->num_ports = good_ptl.num_ports;
	for (port_idx = 0; port_idx < good_ptl.num_ports; port_idx++) {
		out_parms->pc[port_idx].pnum = good_ptl.pnums[port_idx];
	}

	rc = init_sw_pi(dev_info, &pi);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_GET_CONFIG(3);
		goto exit;
	}

	rc = cps_get_lrto(dev_info, out_parms);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_GET_CONFIG(4);
		goto exit;
	}

	/* Obtain the port status */
	for (port_idx = 0; port_idx < out_parms->num_ports; port_idx++) {
		pnum = out_parms->pc[port_idx].pnum;
		quad_cfg = pi.quad_cfg_val[pi.cpr[pnum].quadrant];

		out_parms->pc[port_idx].pw = rio_pc_pw_last;
		out_parms->pc[port_idx].ls = rio_pc_ls_last;
		out_parms->pc[port_idx].iseq = rio_pc_is_last;
		out_parms->pc[port_idx].fc = rio_pc_fc_last;
		out_parms->pc[port_idx].xmitter_disable = false;
		out_parms->pc[port_idx].port_lockout = false;
		out_parms->pc[port_idx].nmtc_xfer_enable = false;
		out_parms->pc[port_idx].rx_lswap = rio_lswap_none; // Not supported by CPS
		out_parms->pc[port_idx].tx_lswap = rio_lswap_none; // Not supported by CPS

		for (lane_num = 0; lane_num < CPS_MAX_PORT_LANES; lane_num++) {
			out_parms->pc[port_idx].tx_linvert[lane_num] = false;
			out_parms->pc[port_idx].rx_linvert[lane_num] = false;
		}

		out_parms->pc[port_idx].port_available =
				pi.cpr[pnum].cfg[quad_cfg].lane_count ?
						true : false;

		if (!out_parms->pc[port_idx].port_available) {
			// switches automatically power down ports with no lanes connected.
			out_parms->pc[port_idx].powered_up = false;
			continue;
		}
		// Determine if the port is powered up...
		// A port is powered up if all of it's lanes are powered up,
		// and the PLL associated with the port is powered up.

		out_parms->pc[port_idx].powered_up = true;
		first_lane = pi.cpr[pnum].cfg[quad_cfg].first_lane;
		last_lane = pi.cpr[pnum].cfg[quad_cfg].lane_count + first_lane;
		for (lane_num = first_lane; lane_num < last_lane; lane_num++) {
			rc = DARRegRead(dev_info, CPS1848_LANE_X_CTL(lane_num),
					&lane_ctl);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_GET_CONFIG(0x10);
				goto exit;
			}

			if (lane_ctl & CPS1848_LANE_X_CTL_LANE_DIS) {
				out_parms->pc[port_idx].powered_up = false;
				break;
			}
		}

		if (out_parms->pc[port_idx].powered_up) {
			pll_num = pi.cpr[pnum].cfg[quad_cfg].pll[0];
			if (INVALID_PLL == pll_num) {
				rc = RIO_ERR_SW_FAILURE;
				out_parms->imp_rc = PC_GET_CONFIG(0x11);
				goto exit;
			}
			rc = DARRegRead(dev_info, CPS1848_PLL_X_CTL_1(pll_num),
					&pll_ctl1);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_GET_CONFIG(0x12);
				goto exit;
			}

			if (pll_ctl1 & CPS1848_PLL_X_CTL_1_PLL_PWR_DOWN) {
				out_parms->pc[port_idx].powered_up = false;
			}
		}

		if (out_parms->pc[port_idx].powered_up) {
			// If the port is powered up, determine the rest of the port configuration...
			// Determine configured port width
			rc = DARRegRead(dev_info,
					CPS1848_PORT_X_CTL_1_CSR(pnum),
					&spx_ctl1);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_GET_CONFIG(0x20);
				goto exit;
			}

			rc = DARRegRead(dev_info,
					CPS1848_PORT_X_ERR_STAT_CSR(pnum),
					&spx_errstat);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_GET_CONFIG(0x21);
				goto exit;
			}

			rc = DARRegRead(dev_info, CPS1848_PORT_X_OPS(pnum),
					&spx_ops);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_GET_CONFIG(0x22);
				goto exit;
			}

			switch (pi.cpr[pnum].cfg[quad_cfg].lane_count) {
			case 1:
				out_parms->pc[port_idx].pw = rio_pc_pw_1x;
				break;
			case 2:
				switch (spx_ctl1 & RIO_SPX_CTL_PTW_OVER) {
				case RIO_SPX_CTL_PTW_OVER_NONE:
				case RIO_SPX_CTL_PTW_OVER_NONE_2:
				case RIO_SPX_CTL_PTW_OVER_2X_NO_4X:
					out_parms->pc[port_idx].pw =
							rio_pc_pw_2x;
					break;

					// RIO_SPX_CTL_PTW_OVER_4X_NO_2X may have no effect
				case RIO_SPX_CTL_PTW_OVER_4X_NO_2X:
				case RIO_SPX_CTL_PTW_OVER_1X_L0:
					out_parms->pc[port_idx].pw =
							rio_pc_pw_1x_l0;
					break;
				case RIO_SPX_CTL_PTW_OVER_1X_LR:
					out_parms->pc[port_idx].pw =
							rio_pc_pw_1x_l1;
					break;
				case RIO_SPX_CTL_PTW_OVER_RSVD:
					out_parms->pc[port_idx].pw =
							rio_pc_pw_last;
					break;
				}
				break;
			case 4:
				switch (spx_ctl1 & RIO_SPX_CTL_PTW_OVER) {
				case RIO_SPX_CTL_PTW_OVER_NONE:
				case RIO_SPX_CTL_PTW_OVER_NONE_2:
				case RIO_SPX_CTL_PTW_OVER_4X_NO_2X:
					out_parms->pc[port_idx].pw =
							rio_pc_pw_4x;
					break;

				case RIO_SPX_CTL_PTW_OVER_2X_NO_4X:
					out_parms->pc[port_idx].pw =
							rio_pc_pw_2x;
					break;
				case RIO_SPX_CTL_PTW_OVER_1X_L0:
					out_parms->pc[port_idx].pw =
							rio_pc_pw_1x_l0;
					break;
				case RIO_SPX_CTL_PTW_OVER_1X_LR:
					out_parms->pc[port_idx].pw =
							rio_pc_pw_1x_l2;
					break;
				case RIO_SPX_CTL_PTW_OVER_RSVD:
					out_parms->pc[port_idx].pw =
							rio_pc_pw_last;
					break;
				}
				break;

			default:
				rc = RIO_ERR_SW_FAILURE;
				out_parms->imp_rc = PC_GET_CONFIG(0x25);
				goto exit;
			}

			if (out_parms->pc[port_idx].powered_up) {
				if (((sps1616_cfg == pi.cpr)
						&& (lane_ctl
								& CPS1616_LANE_X_CTL_RX_PLL_SEL))
						|| ((sps1616_cfg != pi.cpr)
								&& (pll_ctl1
										& CPS1848_PLL_X_CTL_1_PLL_DIV_SEL))) {
					switch ((lane_ctl
							& CPS1848_LANE_X_CTL_RX_RATE)
							>> 1) {
					case 0:
						out_parms->pc[port_idx].ls =
								rio_pc_ls_last;
						break;
					case 1:
						out_parms->pc[port_idx].ls =
								rio_pc_ls_3p125;
						break;
					default:
						out_parms->pc[port_idx].ls =
								rio_pc_ls_6p25;
						break;
					}
				} else {
					switch ((lane_ctl
							& CPS1848_LANE_X_CTL_RX_RATE)
							>> 1) {
					case 0:
						out_parms->pc[port_idx].ls =
								rio_pc_ls_1p25;
						break;
					case 1:
						out_parms->pc[port_idx].ls =
								rio_pc_ls_2p5;
						break;
					default:
						out_parms->pc[port_idx].ls =
								rio_pc_ls_5p0;
						break;
					}
				}
			}

			// Determine port disable and lockout status.
			if (spx_ctl1 & RIO_SPX_CTL_PORT_DIS) {
				out_parms->pc[port_idx].xmitter_disable = true;
			}

			if (spx_ctl1 & RIO_SPX_CTL_LOCKOUT) {
				out_parms->pc[port_idx].port_lockout = true;
			}

			/* Determine Idle Sequence enabled  */
			if (spx_errstat & CPS1848_PORT_X_ERR_STAT_CSR_IDLE2_EN) {
				out_parms->pc[port_idx].iseq = rio_pc_is_two;
			} else {
				out_parms->pc[port_idx].iseq = rio_pc_is_one;
			}

			/* Determine Flow Control enabled  */
			if (spx_ops & CPS1848_PORT_X_OPS_TX_FLOW_CTL_DIS) {
				out_parms->pc[port_idx].fc = rio_pc_fc_rx;
			} else {
				out_parms->pc[port_idx].fc = rio_pc_fc_tx;
			}

			// Note: 1432 & 1848 Rev C has errata about maintenance packet handling
			// OUTPUT_EN should always be set.
			if (DEV_CODE(dev_info) == RIO_DEVI_IDT_SPS1616) {
				if ((spx_ctl1
						& CPS1848_PORT_X_CTL_1_CSR_INPUT_PORT_EN)
						&& (spx_ctl1
								& CPS1848_PORT_X_CTL_1_CSR_OUTPUT_PORT_EN)) {
					out_parms->pc[port_idx].nmtc_xfer_enable =
							true;
				}
			} else {
				if (spx_ctl1
						& CPS1848_PORT_X_CTL_1_CSR_INPUT_PORT_EN) {
					out_parms->pc[port_idx].nmtc_xfer_enable =
							true;
				}
			}

			// Check to see if any lanes are inverted...
			//
			rc = DARRegWrite(dev_info, CPS_ROOT_ACCESS,
					CPS_ROOT_ACCESS_PASSWD);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_GET_CONFIG(0x30);
				goto exit;
			}

			for (lane_num = first_lane; lane_num < last_lane;
					lane_num++) {
				rc = DARRegRead(dev_info,
						CPS_LANE_X_CTL_1_ROOT(lane_num),
						&lane_ctl);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = PC_GET_CONFIG(0x40);
					goto exit;
				}

				if (lane_ctl & CPS_LANE_X_CTL_1_ROOT_TX_INV) {
					out_parms->pc[port_idx].tx_linvert[lane_num
							- first_lane] = true;
				}

				if (lane_ctl & CPS_LANE_X_CTL_1_ROOT_RX_INV) {
					out_parms->pc[port_idx].rx_linvert[lane_num
							- first_lane] = true;
				}
			} // lanes
		} // available
	}

exit:
	return rc;
}

uint32_t CPS_rio_pc_set_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_set_config_in_t *in_parms,
		rio_pc_set_config_out_t *out_parms)
{
	uint32_t rc = RIO_SUCCESS;
	cps_port_info_t pi;
	rio_pc_set_config_in_t sorted;
	set_cfg_info_t regs;
	set_cfg_info_t chgd;
	rio_pc_get_config_in_t get_cfg_in;
	rio_pc_get_config_out_t curr;
	uint8_t port_idx;

	memset(&sorted, 0, sizeof(sorted));
	out_parms->num_ports = 0;
	out_parms->lrto = 0;
	out_parms->imp_rc = RIO_SUCCESS;

	if ((NUM_CPS_PORTS(dev_info) < in_parms->num_ports)
			&& !(RIO_ALL_PORTS == in_parms->num_ports)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = PC_SET_CONFIG(0x01);
		goto exit;
	}

	rc = init_sw_pi(dev_info, &pi);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x02);
		goto exit;
	}

	// Failure codes 0x10 through 0x3E
	rc = cps_set_config_init_parms_check_conflict(dev_info, in_parms, &pi,
			&sorted, &regs, &out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	// Copy current register values to changed register values
	chgd = regs;

	// Get current configuration
	get_cfg_in.ptl.num_ports = RIO_ALL_PORTS;
	rc = CPS_rio_pc_get_config(dev_info, &get_cfg_in, &curr);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = curr.imp_rc;
		goto exit;
	}

	rc = cps_set_lrto(dev_info, in_parms);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x3F);
		goto exit;
	}

	// Failure codes 0x40 through 0x7F
	rc = cps_set_config_compute_changes(dev_info, &pi, &curr, &sorted,
			&regs, &chgd, &out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	// Ensure that the port used to connect to the device
	// is not reset, disabled, or locked-out by cps_set_config_write_changes.
	rc = cps_set_config_preserve_host_port(dev_info, &pi, &sorted, &regs,
			&chgd, &out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	// Failure codes 0x80 through 0xBF
	rc = cps_set_config_write_changes(dev_info, &pi, &sorted, &regs, &chgd,
			&out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	// Return updated configuration for all changed ports...
	get_cfg_in.ptl.num_ports = 0;
	for (port_idx = 0; port_idx < NUM_CPS_PORTS(dev_info); port_idx++) {
		uint8_t srch_idx;
		for (srch_idx = 0; srch_idx < NUM_CPS_PORTS(dev_info);
				srch_idx++) {
			if (port_idx == sorted.pc[srch_idx].pnum) {
				get_cfg_in.ptl.pnums[get_cfg_in.ptl.num_ports] =
						port_idx;
				get_cfg_in.ptl.num_ports++;
				break;
			}
		}
	}

	if (!get_cfg_in.ptl.num_ports) {
		get_cfg_in.ptl.num_ports = RIO_ALL_PORTS;
	}

	rc = CPS_rio_pc_get_config(dev_info, &get_cfg_in, out_parms);

exit:
	return rc;
}

uint32_t CPS_rio_pc_get_status(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_status_in_t *in_parms,
		rio_pc_get_status_out_t *out_parms)
{
	uint32_t rc;
	uint32_t err_stat, ctl1, err_det, stat_ctl;
	uint8_t pnum, quad_cfg, port_idx;
	cps_port_info_t pi;
	struct DAR_ptl good_ptl;

	out_parms->num_ports = 0;
	out_parms->imp_rc = 0;

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if ((RIO_SUCCESS != rc) || (good_ptl.num_ports > CPS_MAX_PORTS)) {
		out_parms->imp_rc = PC_GET_STATUS(1);
		goto exit;
	}

	out_parms->num_ports = good_ptl.num_ports;
	for (port_idx = 0; port_idx < good_ptl.num_ports; port_idx++)
		out_parms->ps[port_idx].pnum = good_ptl.pnums[port_idx];

	rc = init_sw_pi(dev_info, &pi);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_GET_STATUS(3);
		goto exit;
	}

	/* Obtain the port status */
	for (port_idx = 0; port_idx < out_parms->num_ports; port_idx++) {
		pnum = out_parms->ps[port_idx].pnum;
		quad_cfg = pi.quad_cfg_val[pi.cpr[pnum].quadrant];

		out_parms->ps[port_idx].port_ok = false;
		out_parms->ps[port_idx].pw = rio_pc_pw_last;
		out_parms->ps[port_idx].port_error = false;
		out_parms->ps[port_idx].input_stopped = false;
		out_parms->ps[port_idx].output_stopped = false;
		out_parms->ps[port_idx].fc = rio_pc_fc_last;
		out_parms->ps[port_idx].iseq = rio_pc_is_last;

		out_parms->ps[port_idx].num_lanes =
				pi.cpr[pnum].cfg[quad_cfg].lane_count;
		out_parms->ps[port_idx].first_lane =
				pi.cpr[pnum].cfg[quad_cfg].first_lane;

		if (out_parms->ps[port_idx].num_lanes) {
			rc = DARRegRead(dev_info,
					CPS1848_PORT_X_ERR_STAT_CSR(pnum),
					&err_stat);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_GET_STATUS(4);
				goto exit;
			}

			if (err_stat & CPS1848_PORT_X_ERR_STAT_CSR_PORT_OK) {
				out_parms->ps[port_idx].port_ok = true;

				// If the port is PORT_OK, then the port width is valid.
				// Figure out what that port-width is :)
				//
				rc = DARRegRead(dev_info,
						CPS1848_PORT_X_CTL_1_CSR(pnum),
						&ctl1);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = PC_GET_STATUS(4);
					goto exit;
				}

				rc = DARRegRead(dev_info,
						CPS1848_PORT_X_STATUS_AND_CTL(
								pnum),
						&stat_ctl);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = PC_GET_STATUS(5);
					goto exit;
				}

				switch (ctl1
						& CPS1848_PORT_X_CTL_1_CSR_INIT_PWIDTH) {
				case RIO_SPX_CTL_PTW_INIT_1X_L0:
					if (1
							== out_parms->ps[port_idx].num_lanes) {
						out_parms->ps[port_idx].pw =
								rio_pc_pw_1x;
					} else {
						out_parms->ps[port_idx].pw =
								rio_pc_pw_1x_l0;
					}
					break;
				case RIO_SPX_CTL_PTW_INIT_1X_LR:
					if (2
							== out_parms->ps[port_idx].num_lanes) {
						out_parms->ps[port_idx].pw =
								rio_pc_pw_1x_l1;
					} else {
						out_parms->ps[port_idx].pw =
								rio_pc_pw_1x_l2;
					}
					break;
				case RIO_SPX_CTL_PTW_INIT_2X:
					out_parms->ps[port_idx].pw =
							rio_pc_pw_2x;
					break;
				case RIO_SPX_CTL_PTW_INIT_4X:
					out_parms->ps[port_idx].pw =
							rio_pc_pw_4x;
					break;
				default:
					out_parms->ps[port_idx].pw =
							rio_pc_pw_last;
				}

				/* Fill in structure fields for PORT n ERR STAT register */
				if (err_stat
						& CPS1848_PORT_X_ERR_STAT_CSR_INPUT_ERR_STOP) {
					out_parms->ps[port_idx].input_stopped =
							true;
				}

				if (err_stat
						& CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_ERR_STOP) {
					out_parms->ps[port_idx].output_stopped =
							true;
				}

				// PORT_ERR should be set if there is any condition that prevents
				// packet exchange on the link:
				// - PORT_ERR
				// - PORT_FAIL when CPS1848_PORT_X_CTL_1_CSR_DROP_PKT_EN is set
				// - a "TOO MANY RETRIES" event has been detected.

				if (err_stat
						& CPS1848_PORT_X_ERR_STAT_CSR_PORT_ERR)
					out_parms->ps[port_idx].port_error =
							true;

				if (err_stat
						& CPS1848_PORT_X_ERR_STAT_CSR_IDLE_SEQ) {
					out_parms->ps[port_idx].iseq =
							rio_pc_is_two;
				} else {
					out_parms->ps[port_idx].iseq =
							rio_pc_is_one;
				}

				if (stat_ctl
						& CPS1848_PORT_X_STATUS_AND_CTL_RX_FC) {
					out_parms->ps[port_idx].fc =
							rio_pc_fc_rx;
				} else {
					out_parms->ps[port_idx].fc =
							rio_pc_fc_tx;
				}

				//@sonar:off - Collapsible "if" statements should be merged
				if (err_stat
						& CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_FAIL) {
					if (ctl1
							& CPS1848_PORT_X_CTL_1_CSR_DROP_PKT_EN)
						out_parms->ps[port_idx].port_error =
								true;
				}
				//@sonar:on

				rc =
						DARRegRead(dev_info,
								CPS1848_PORT_X_IMPL_SPEC_ERR_DET(
										pnum),
								&err_det);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = PC_GET_STATUS(4);
					goto exit;
				}

				if (err_det
						& CPS1848_PORT_X_IMPL_SPEC_ERR_DET_MANY_RETRY)
					out_parms->ps[port_idx].port_error =
							true;
			}
		}
	}

exit:
	return rc;
}

uint32_t CPS_rio_pc_reset_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_port_in_t *in_parms,
		rio_pc_reset_port_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t port_num, start_port, end_port;
	cps_port_info_t pi;

	out_parms->imp_rc = RIO_SUCCESS;

	if ((RIO_ALL_PORTS != in_parms->port_num)
			&& (in_parms->port_num >= NUM_CPS_PORTS(dev_info))) {
		out_parms->imp_rc = PC_RESET_PORT(1);
		goto exit;
	}

	if ((!in_parms->oob_reg_acc)
			&& (in_parms->reg_acc_port >= NUM_CPS_PORTS(dev_info))) {
		out_parms->imp_rc = PC_RESET_PORT(2);
		goto exit;
	}

	rc = init_sw_pi(dev_info, &pi);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_RESET_PORT(3);
		goto exit;
	}

	if (RIO_ALL_PORTS == in_parms->port_num) {
		start_port = 0;
		end_port = NUM_CPS_PORTS(dev_info) - 1;
	} else {
		start_port = end_port = in_parms->port_num;
	}

	// If requested, reset the device...
	if ((RIO_ALL_PORTS == in_parms->port_num) && (in_parms->reset_lp)
			&& (!in_parms->preserve_config)) {
		// Reset all link partners, then reset the device.
		for (port_num = start_port; port_num <= end_port; port_num++) {
			rc = DARRegWrite(dev_info,
					CPS1848_PORT_X_LINK_MAINT_REQ_CSR(
							port_num),
					RIO_SPX_LM_REQ_CMD_RST_DEV);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_RESET_PORT(0x10);
				goto exit;
			}
		}

		rc = DARRegWrite(dev_info, CPS1848_DEVICE_SOFT_RESET,
				CPS1848_DEVICE_SOFT_RESET_COMMAND);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_PORT(0x11);
			goto exit;
		}
	} else {
		// Otherwise, reset one port at a time...
		for (port_num = start_port; port_num <= end_port; port_num++) {
			if (!in_parms->oob_reg_acc
					&& (in_parms->reg_acc_port == port_num)) {
				continue;
			}

			rc = DARRegWrite(dev_info,
					CPS1848_PORT_X_LINK_MAINT_REQ_CSR(
							port_num),
					RIO_SPX_LM_REQ_CMD_RST_DEV);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_RESET_PORT(0x20);
				goto exit;
			}

			rc =
					DARRegWrite(dev_info,
							CPS1848_DEVICE_RESET_CTL,
							(uint32_t)(1 << port_num)
									| CPS1848_DEVICE_RESET_CTL_DO_RESET);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_RESET_PORT(0x21);
				goto exit;
			}
		}
	}

	out_parms->imp_rc = RIO_SUCCESS;

exit:
	return rc;
}

uint32_t CPS_rio_pc_reset_link_partner(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_link_partner_in_t *in_parms,
		rio_pc_reset_link_partner_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t regVal;

	out_parms->imp_rc = RIO_SUCCESS;

	if (in_parms->port_num >= NUM_CPS_PORTS(dev_info)) {
		out_parms->imp_rc = PC_RESET_LP(1);
		goto exit;
	}

	rc = DARRegRead(dev_info,
			CPS1848_PORT_X_ERR_STAT_CSR(in_parms->port_num),
			&regVal);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_RESET_LP(2);
		goto exit;
	}

	if (regVal & CPS1848_PORT_X_ERR_STAT_CSR_PORT_OK) {
		uint32_t temp;
		// Port should be reset after 1 reset attempt.
		rc = DARRegWrite(dev_info,
				CPS1848_PORT_X_LINK_MAINT_REQ_CSR(
						in_parms->port_num),
				RIO_SPX_LM_REQ_CMD_RST_DEV);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_LP(3);
			goto exit;
		}

		rc = DARRegRead(dev_info,
				CPS1848_PORT_X_LINK_MAINT_RESP_CSR(
						in_parms->port_num), &temp);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_LP(4);
			goto exit;
		}

		// This warning implies that errors should be cleared on the
		// port before reset transmission is attempted.
		if (!(temp & CPS1848_PORT_X_LINK_MAINT_RESP_CSR_VALID)) {
			out_parms->imp_rc = PC_RESET_LP(0x50);
		}

		if (in_parms->resync_ackids) {
			// The safest way to do this is to temporarily disable the port, then re-enable it.
			// Note: PORT_DIS will also reset the port, and discard packets held in the final buffer...
			rc = DARRegRead(dev_info,
					CPS1848_PORT_X_CTL_1_CSR(
							in_parms->port_num),
					&regVal);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_RESET_LP(6);
				goto exit;
			}

			rc =
					DARRegWrite(dev_info,
							CPS1848_PORT_X_CTL_1_CSR(
									in_parms->port_num),
							regVal
									| CPS1848_PORT_X_CTL_1_CSR_PORT_DIS);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_RESET_LP(7);
				goto exit;
			}

			// Note: Write this register twice, to ensure that port width overrides are restored.
			rc = DARRegWrite(dev_info,
					CPS1848_PORT_X_CTL_1_CSR(
							in_parms->port_num),
					regVal);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_RESET_LP(8);
				goto exit;
			}

			rc = DARRegWrite(dev_info,
					CPS1848_PORT_X_CTL_1_CSR(
							in_parms->port_num),
					regVal);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_RESET_LP(9);
				goto exit;
			}
		}
	}

	rc = RIO_SUCCESS;

exit:
	return rc;
}

uint32_t CPS_rio_pc_clr_errs(DAR_DEV_INFO_t *dev_info,
		rio_pc_clr_errs_in_t *in_parms,
		rio_pc_clr_errs_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t regVal;
	rio_pc_reset_port_in_t rst_port_in;
	rio_pc_reset_port_out_t rst_port_out;

	out_parms->imp_rc = RIO_SUCCESS; /* Default Value */

	if (in_parms->port_num >= NUM_CPS_PORTS(dev_info)) {
		out_parms->imp_rc = PC_CLR_ERRS(1);
		goto exit;
	}

	rc = DARRegRead(dev_info,
			CPS1848_PORT_X_ERR_STAT_CSR(in_parms->port_num),
			&regVal);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_CLR_ERRS(2);
		goto exit;
	}

	// Without PORT_OK, or if not allowed to clear link partner errors,
	// or if we don't know the kind of link partner, just reset the port...
	//
	// This should be sufficient assuming both ends of the link have
	// error recovery capabilities.
	//
	rst_port_in.port_num = in_parms->port_num;
	rst_port_in.oob_reg_acc = true; // May result in temporary loss of
					// connectivity on the register access
					// port
	rst_port_in.reg_acc_port = RIO_ALL_PORTS;
	rst_port_in.preserve_config = true;

	if ((regVal & CPS1848_PORT_X_ERR_STAT_CSR_PORT_OK)
			&& in_parms->clr_lp_port_err && in_parms->lp_dev_info
			&& (RIO_VEND_IDT == VEND_CODE(in_parms->lp_dev_info))) {
		rst_port_in.reset_lp = true; // Expect CPS link partner to support per-port reset.
		if ((DEV_CODE(in_parms->lp_dev_info) == RIO_DEVI_IDT_SPS1616)
				|| (DEV_CODE(in_parms->lp_dev_info)
						== RIO_DEVI_IDT_CPS1616)) {

			uint8_t port_idx;

			// Need to perform complex error recovery.
			// Check the parameters before calling the error recovery routine.
			if (NUM_CPS_PORTS(in_parms->lp_dev_info)
					< in_parms->num_lp_ports) {
				out_parms->imp_rc = PC_CLR_ERRS(3);
				goto exit;
			}

			for (port_idx = 0; port_idx < in_parms->num_lp_ports;
					port_idx++) {
				// Return codes 0x10 through 0x20
				if (NUM_CPS_PORTS(in_parms->lp_dev_info)
						<= in_parms->lp_port_list[port_idx]) {
					out_parms->imp_rc = PC_CLR_ERRS(
							0x10 + port_idx);
					goto exit;
				}
			}

			// Return codes 0x30 through 0xFF
			rc = cps_sps1616_port_err_clear(dev_info, in_parms,
					out_parms);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
		}
	} else {
		rst_port_in.reset_lp = false; // Can't do it...
	}

	rc = CPS_rio_pc_reset_port(dev_info, &rst_port_in, &rst_port_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = rst_port_out.imp_rc;
	}

exit:
	return rc;
}

uint32_t CPS_rio_pc_secure_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_secure_port_in_t *in_parms,
		rio_pc_secure_port_out_t *out_parms)
{
	uint32_t rc;
	uint8_t idx, port_idx;
	uint32_t regVal;
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	filt_n_trace_mask_n_val_t *filter;
	struct DAR_ptl good_ptl;

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SECURE_PORT(1);
		goto exit;
	}

	cfg_in.ptl = good_ptl;
	rc = CPS_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto exit;
	}

	if (!cfg_out.num_ports) {
		out_parms->imp_rc = PC_SECURE_PORT(3);
		goto exit;
	}

	out_parms->imp_rc = RIO_SUCCESS; /* Default Value */
	out_parms->rst = in_parms->rst; // Starting value */
	out_parms->bc_mtc_pkts_allowed = in_parms->mtc_pkts_allowed;
	out_parms->MECS_participant = in_parms->MECS_participant;
	out_parms->MECS_acceptance = in_parms->MECS_acceptance;

	for (port_idx = 0; port_idx < good_ptl.num_ports; port_idx++) {
		uint8_t pnum = good_ptl.pnums[port_idx];

		// If the ports' registers can be accessed, set it up...
		if (!(cfg_out.pc[port_idx].port_available
				&& cfg_out.pc[port_idx].powered_up)) {
			continue;
		}

		/* Only SPS1616 supports ignoring MECS */
		if ((RIO_DEVI_IDT_SPS1616 == DEV_CODE(dev_info))
				|| (RIO_DEVI_IDT_CPS1616 == DEV_CODE(dev_info))) {
			rc = DARRegRead(dev_info,
					CPS1616_PORT_X_STATUS_AND_CTL(pnum),
					&regVal);
			if ( RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SECURE_PORT(0x10);
				goto exit;
			}

			if (in_parms->MECS_acceptance) {
				regVal &=
						~CPS1616_PORT_X_STATUS_AND_CTL_IGNORE_MC_CS;
			} else {
				regVal |=
						CPS1616_PORT_X_STATUS_AND_CTL_IGNORE_MC_CS;
			}

			rc = DARRegWrite(dev_info,
					CPS1616_PORT_X_STATUS_AND_CTL(pnum),
					regVal);
			if ( RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SECURE_PORT(0x11);
				goto exit;
			}
		} else {
			out_parms->MECS_acceptance = true;
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_CTL_1_CSR(pnum),
				&regVal);
		if ( RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SECURE_PORT(0x12);
			goto exit;
		}

		if (in_parms->MECS_participant) {
			regVal |= CPS1848_PORT_X_CTL_1_CSR_MCAST_CS;
		} else {
			regVal &= ~CPS1848_PORT_X_CTL_1_CSR_MCAST_CS;
		}

		rc = DARRegWrite(dev_info, CPS1848_PORT_X_CTL_1_CSR(pnum),
				regVal);
		if ( RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SECURE_PORT(0x13);
			goto exit;
		}

		switch (in_parms->rst) {
		case rio_pc_rst_device:
		case rio_pc_rst_port:
			rc = DARRegRead(dev_info, CPS1848_DEVICE_CTL_1,
					&regVal);
			if ( RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SECURE_PORT(0x20);
				goto exit;
			}

			if (rio_pc_rst_port == in_parms->rst) {
				regVal |= CPS1848_DEVICE_CTL_1_PORT_RST_CTL;
			} else {
				regVal &= ~CPS1848_DEVICE_CTL_1_PORT_RST_CTL;
			}

			rc = DARRegWrite(dev_info, CPS1848_DEVICE_CTL_1,
					regVal);
			if ( RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SECURE_PORT(0x21);
				goto exit;
			}
			break;

		case rio_pc_rst_int:
		case rio_pc_rst_pw:
		case rio_pc_rst_ignore:
			rc = cps_set_int_pw_ignore_reset(dev_info, pnum,
					&out_parms->rst, &out_parms->imp_rc);
			if ( RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SECURE_PORT(
						0x22 + out_parms->imp_rc);
				goto exit;
			}
			break;

		//@sonar:off - c:S3458
		case rio_pc_rst_last:
		default:
		//@sonar:on
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = PC_SECURE_PORT(0x2F);
			goto exit;
		}

		if (in_parms->mtc_pkts_allowed) {
			filter = mtc_filter_off;
		} else {
			filter = mtc_filter_on;
		}

		for (idx = 0; idx < CPS1848_PORT_MAX_TRACE_REGS; idx++) {
			rc = DARRegWrite(dev_info,
					CPS1848_PORT_P_TRACE_I_MASK_R(pnum, 0,
							idx),
					filter[idx].value);
			if ( RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SECURE_PORT(0x30 + idx);
				goto exit;
			}

			rc = DARRegWrite(dev_info,
					CPS1848_PORT_P_TRACE_I_MASK_R(pnum, 0,
							idx), filter[idx].mask);
			if ( RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SECURE_PORT(0x38 + idx);
				goto exit;
			}
		}

		rc = DARRegRead(dev_info, CPS1848_PORT_X_OPS(pnum), &regVal);
		if ( RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SECURE_PORT(0x40);
			goto exit;
		}

		if (in_parms->mtc_pkts_allowed) {
			regVal &= ~CPS1848_PORT_X_OPS_FILTER_0_EN;
		} else {
			regVal |= CPS1848_PORT_X_OPS_FILTER_0_EN;
		}

		rc = DARRegWrite(dev_info, CPS1848_PORT_X_OPS(pnum), regVal);
		if ( RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SECURE_PORT(0x41);
			goto exit;
		}
	}

exit:
	return rc;
}

/* Device/Port Reset Configuration */
uint32_t CPS_rio_pc_dev_reset_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_dev_reset_config_in_t *in_parms,
		rio_pc_dev_reset_config_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t regVal;

	out_parms->imp_rc = RIO_SUCCESS;

	if (in_parms->rst >= rio_pc_rst_last) {
		out_parms->imp_rc = PC_DEV_RESET_CONFIG(1);
		goto exit;
	}

	out_parms->rst = in_parms->rst;

	switch (in_parms->rst) {
	case rio_pc_rst_device:
	case rio_pc_rst_port:
		rc = DARRegRead(dev_info, CPS1848_DEVICE_CTL_1, &regVal);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_DEV_RESET_CONFIG(2);
			goto exit;
		}

		if (rio_pc_rst_port == in_parms->rst) {
			regVal |= CPS1848_DEVICE_CTL_1_PORT_RST_CTL;
		} else {
			regVal &= ~CPS1848_DEVICE_CTL_1_PORT_RST_CTL;
		}

		rc = DARRegWrite(dev_info, CPS1848_DEVICE_CTL_1, regVal);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_DEV_RESET_CONFIG(3);
			goto exit;
		}
		break;

	case rio_pc_rst_int: // CPS devices do not support interrupt or PW notification.
	case rio_pc_rst_pw: // CPS1616/SPS1616 does support ignoring reset requests.
			    //    For these devices, translate the reset configuration to "ignore"
	case rio_pc_rst_ignore: //    For all other devices, translate the reset configuration to "reset port".
		rc = cps_set_int_pw_ignore_reset(dev_info, RIO_ALL_PORTS,
				&out_parms->rst, &out_parms->imp_rc);
		if (RIO_SUCCESS != rc) {
			// Failure points 6-10
			out_parms->imp_rc = PC_DEV_RESET_CONFIG(
					5 + out_parms->imp_rc);
			goto exit;
		}
		break;
	default: // Should never get here...
		rc = RIO_ERR_SW_FAILURE;
		out_parms->imp_rc = PC_DEV_RESET_CONFIG(0x10);
		goto exit;
	}

exit:
	return rc;
}

#endif /* CPS_DAR_WANTED */

#ifdef __cplusplus
}
#endif
