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
#include <stdbool.h>
#include <string.h>

#include "RapidIO_Device_Access_Routines_API.h"
#include "RapidIO_Port_Config_API.h"
#include "RapidIO_Error_Management_API.h"
#include "RapidIO_Utilities_API.h"
#include "Tsi57x_DeviceDriver.h"
#include "Tsi578.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TSI57X_DAR_WANTED

#define PC_DET_PORTS_2_SKIP_0 (PC_FIRST_SUBROUTINE_0+0x0900) // 001900
#define TSI57X_LP_0           (PC_FIRST_SUBROUTINE_0+0x0A00) // 001A00

#define PC_DET_PORTS_2_SKIP(x) (PC_DET_PORTS_2_SKIP_0+x)
#define TSI57X_LP(x) (TSI57X_LP_0+x)


#define MAX_PORTS_TO_SKIP 2

#define END_PMR_ARRAY {RIO_ALL_PORTS,0, 0, 0, 0, 0, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, {RIO_ALL_PORTS, RIO_ALL_PORTS, RIO_ALL_PORTS} }

//          4x 1x PWR
//  P  M L1 LC LC  M     Power Down Mask                      Reset Mask                           Other Mac Ports
port_mac_relations_t tsi578_pmr[] = {
  { 0, 0, 0, 4, 1, 0, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 1, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 1, 0, 1, 0, 1, 0, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 0, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 2, 1, 0, 4, 1, 1, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 3, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 3, 1, 1, 0, 1, 1, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 2, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 4, 2, 0, 4, 1, 2, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 5, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 5, 2, 1, 0, 1, 2, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 4, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 6, 3, 0, 4, 1, 3, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 7, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 7, 3, 1, 0, 1, 3, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 6, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 8, 4, 0, 4, 1, 4, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 9, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 9, 4, 1, 0, 1, 4, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 8, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  {10, 5, 0, 4, 1, 5, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, {11, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  {11, 5, 1, 0, 1, 5, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, {10, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  {12, 6, 0, 4, 1, 6, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, {13, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  {13, 6, 1, 0, 1, 6, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, {12, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  {14, 7, 0, 4, 1, 7, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, {15, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  {15, 7, 1, 0, 1, 7, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, {14, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  END_PMR_ARRAY
};

//          4x 1x PWR
//  P  M L1 LC LC  M     Power Down Mask                      REset Mask                           Other Mac Ports
port_mac_relations_t tsi577_pmr[] = {
  { 0, 0, 0, 4, 1, 0, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 1,  8,  9} },
  { 1, 0, 1, 0, 1, 0, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 0,  8,  9} },
  { 2, 1, 0, 4, 1, 1, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 3, 10, 11} },
  { 3, 1, 1, 0, 1, 1, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 2, 10, 11} },
  { 4, 2, 0, 4, 1, 2, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 5, 12, 13} },
  { 5, 2, 1, 0, 1, 2, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 4, 12, 13} },
  { 6, 3, 0, 4, 1, 3, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 7, 14, 15} },
  { 7, 3, 1, 0, 1, 3, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 6, 14, 15} },
  { 8, 0, 2, 0, 1, 4, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 0,  1,  9} },
  { 9, 0, 3, 0, 1, 4, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 0,  1,  8} },
  {10, 1, 2, 0, 1, 5, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 2,  3, 11} },
  {11, 1, 3, 0, 1, 5, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 2,  3, 10} },
  {12, 2, 2, 0, 1, 6, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 4,  5, 13} },
  {13, 2, 3, 0, 1, 6, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 4,  5, 12} },
  {14, 3, 2, 0, 1, 7, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 6,  7, 15} },
  {15, 3, 3, 0, 1, 7, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 6,  7, 14} },
  END_PMR_ARRAY
};

port_mac_relations_t tsi574_pmr[] = {
  { 0, 0, 0, 4, 1, 0, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 1, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 1, 0, 1, 0, 1, 0, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 0, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 2, 1, 0, 4, 1, 1, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 3, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 3, 1, 1, 0, 1, 1, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 2, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 4, 2, 0, 4, 1, 2, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 5, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 5, 2, 1, 0, 1, 2, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 4, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 6, 3, 0, 4, 1, 3, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 7, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 7, 3, 1, 0, 1, 3, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 6, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  END_PMR_ARRAY
};

port_mac_relations_t tsi572_pmr[] = {
  { 0, 0, 0, 4, 1, 0, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 1, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 1, 0, 1, 0, 1, 0, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 0, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 2, 1, 0, 4, 1, 1, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 3, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 3, 1, 1, 0, 1, 1, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 2, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 4, 2, 0, 4, 1, 2, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 5, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 5, 2, 1, 0, 1, 2, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 4, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 6, 3, 0, 4, 1, 3, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4, { 7, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  { 7, 3, 1, 0, 1, 3, TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1, TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1, { 6, RIO_ALL_PORTS, RIO_ALL_PORTS} },
  END_PMR_ARRAY
};

#define ALL_BITS ((uint32_t)(0xFFFFFFFF))

#define MC_IDX_MASK (TSI578_RIO_MC_IDX_MC_ID| \
		TSI578_RIO_MC_IDX_LARGE_SYS| \
		TSI578_RIO_MC_IDX_MC_EN)

#define PW_MASK (TSI578_RIO_PW_DESTID_LARGE_DESTID | \
		TSI578_RIO_PW_DESTID_DESTID_LSB | \
		TSI578_RIO_PW_DESTID_DESTID_MSB)

#define ERR_DET_MASK (TSI578_RIO_LOG_ERR_DET_EN_UNSUP_TRANS_EN | \
		TSI578_RIO_LOG_ERR_DET_EN_ILL_RESP_EN | \
		TSI578_RIO_LOG_ERR_DET_EN_ILL_TRANS_EN)

#define MC_MASK_CFG_MASK ((uint32_t)(TSI578_RIO_MC_MASK_CFG_PORT_PRESENT | \
		TSI578_RIO_MC_MASK_CFG_MASK_CMD | \
		TSI578_RIO_MC_MASK_CFG_EG_PORT_NUM | \
		TSI578_RIO_MC_MASK_CFG_MC_MASK_NUM))

#define MC_DESTID_MASK ((uint32_t)(TSI578_RIO_MC_DESTID_CFG_DESTID_BASE | \
		TSI578_RIO_MC_DESTID_CFG_DESTID_BASE_LT | \
		TSI578_RIO_MC_DESTID_CFG_MASK_NUM_BASE))

#define MC_ASSOC_MASK ((uint32_t)(TSI578_RIO_MC_DESTID_ASSOC_ASSOC_PRESENT | \
		TSI578_RIO_MC_DESTID_ASSOC_CMD | \
		TSI578_RIO_MC_DESTID_ASSOC_LARGE))

const struct scrpad_info scratchpad_const[MAX_DAR_SCRPAD_IDX] = {
	{TSI578_RIO_MC_IDX(0) , MC_IDX_MASK},
	{TSI578_RIO_MC_IDX(1) , MC_IDX_MASK},
	{TSI578_RIO_MC_IDX(2) , MC_IDX_MASK},
	{TSI578_RIO_MC_IDX(3) , MC_IDX_MASK},
	{TSI578_RIO_MC_IDX(4) , MC_IDX_MASK},
	{TSI578_RIO_MC_IDX(5) , MC_IDX_MASK},
	{TSI578_RIO_MC_IDX(6) , MC_IDX_MASK},
	{TSI578_RIO_MC_IDX(7) , MC_IDX_MASK},
	{TSI578_RIO_MC_MSKX(0), TSI578_RIO_MC_MSKX_MC_MSK},/* SCRPAD_MASK_IDX for offsets to preserve/track */
	{TSI578_RIO_MC_MSKX(1), TSI578_RIO_MC_MSKX_MC_MSK},
	{TSI578_RIO_MC_MSKX(2), TSI578_RIO_MC_MSKX_MC_MSK},
	{TSI578_RIO_MC_MSKX(3),TSI578_RIO_MC_MSKX_MC_MSK},
	{TSI578_RIO_MC_MSKX(4), TSI578_RIO_MC_MSKX_MC_MSK},
	{TSI578_RIO_MC_MSKX(5), TSI578_RIO_MC_MSKX_MC_MSK},
	{TSI578_RIO_MC_MSKX(6), TSI578_RIO_MC_MSKX_MC_MSK},
	{TSI578_RIO_MC_MSKX(7),TSI578_RIO_MC_MSKX_MC_MSK},
	{TSI578_RIO_COMP_TAG   , TSI578_RIO_COMP_TAG_CTAG },
	{TSI578_RIO_LUT_ATTR   , TSI578_RIO_LUT_ATTR_DEFAULT_PORT},
	{TSI578_RIO_SW_LT_CTL  , TSI578_RIO_SW_LT_CTL_TVAL},
	{TSI578_RIO_PW_DESTID  , PW_MASK},
	{TSI578_RIO_LOG_ERR_DET_EN, ERR_DET_MASK},
	{TSI578_RIO_PKT_TTL   ,  TSI578_RIO_PKT_TTL_TTL },
	{TSI578_RIO_MC_MASK_CFG, MC_MASK_CFG_MASK},
	{TSI578_RIO_MC_DESTID_CFG,   MC_DESTID_MASK},  /* Code expects that this
							* is the register immediately before
							* TSI578_RIO_MC_DESTID_ASSOC.
							*/
	{TSI578_RIO_MC_DESTID_ASSOC, MC_ASSOC_MASK},
	{SCRPAD_EOF_OFFSET, ALL_BITS}
};

#define TSI57X_HIDDEN_SERDES_REG(xx,yy) ((uint32_t)(0x1E00C+(2*0x100*xx)+(0x40*yy)))
#define TSI57X_HIDDEN_SERDES_TX_INV 0x00080000
#define TSI57X_HIDDEN_SERDES_RX_INV 0x00040000
#define MAX(a,b) ((a>b)?a:b)
#define MAC(x) (x/2)
#define EVEN_PORT(x) (x & ~1)

#define PC_CLR_ERRS(x) (PC_CLR_ERRS_0+x)

#define EM_UPDATE_RESET_0 (EM_FIRST_SUBROUTINE_0+0x2D00) // 202D00
#define UPDATE_RESET(x) (EM_UPDATE_RESET_0+x)
#define PC_SECURE_PORT(x) (PC_SECURE_PORT_0+x)
#define PC_DEV_RESET_CONFIG(x) (PC_DEV_RESET_CONFIG_0+x)

typedef enum { Tsi57x_none, Tsi57x_4x, Tsi57x_2_1x } Tsi57x_MAC_MODE;
typedef enum { Tsi57x_no_ch, Tsi57x_1p25, Tsi57x_2p5, Tsi57x_3p125 } Tsi57x_LS;

typedef struct mac_indexes_t_TAG {
	int32_t port_idx[2];
} mac_indexes_t;

typedef struct tsi_lane_diffs_per_port_t_TAG {
	int32_t first_lane;
	int32_t num_lanes;
	int32_t change_mask;
	int32_t invert_mask;
} tsi_lane_diffs_per_port_t;

typedef struct tsi_lane_diffs_per_mac_t_TAG {
	tsi_lane_diffs_per_port_t lane_diffs[2];
} tsi_lane_diffs_per_mac_t;

typedef struct port_cfg_chg_t_TAG {
	bool valid[TSI578_MAX_PORTS];
	bool laneRegsChanged[TSI578_MAX_MAC][TSI578_MAX_PORT_LANES];
	uint32_t laneRegs[TSI578_MAX_MAC][TSI578_MAX_PORT_LANES];
	uint32_t dloopCtl[TSI578_MAX_MAC];
	uint32_t dloopCtlOrig[TSI578_MAX_MAC];
	uint32_t spxCtl[TSI578_MAX_PORTS];
	uint32_t spxCtlOrig[TSI578_MAX_PORTS];
} port_cfg_chg_t;

#define RESET_PORT_SKIP   true
#define CONFIG_PORT_SKIP  false

typedef struct powerup_reg_offsets_t_TAG {
	uint32_t offset;          // Base register address
	uint32_t per_port_offset; // If there is a per-port offset
	uint32_t mask;            // Mask applied to zero out fields
				  //    that should not be preserved.
} powerup_reg_offsets_t;

powerup_reg_offsets_t reg_offsets[]=
// Start of global registers with per-port copies that must be preserved
{
// Start of per-port registers that must be preserved
  { (uint32_t)TSI578_SPX_CTL(0)             , (uint32_t)0x20, ALL_BITS  },
  { (uint32_t)TSI578_SPX_RATE_EN(0)         , (uint32_t)0x40, ALL_BITS  },
  { (uint32_t)TSI578_SPX_ERR_RATE(0)        , (uint32_t)0x40, (uint32_t)~TSI578_SPX_ERR_RATE_ERR_RATE_CNT  },
  { (uint32_t)TSI578_SPX_ERR_THRESH(0)      , (uint32_t)0x40, ALL_BITS  },
  { (uint32_t)TSI578_SPX_DISCOVERY_TIMER(0) , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_MODE(0)            , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_RIO_WM(0)          , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_ROUTE_CFG_DESTID(0), (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_ROUTE_BASE(0)      , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_CTL_INDEP(0)       , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_SILENCE_TIMER(0)   , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_RIOX_MC_LAT_LIMIT(0)   , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_PSC0N1_CTRL(0)     , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_PSC2N3_CTRL(0)     , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_PSC4N5_CTRL(0)     , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_TX_Q_D_THRESH(0)   , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_TX_Q_STATUS(0)   , (uint32_t)0x100, (uint32_t)TSI578_SPX_TX_Q_STATUS_CONG_THRESH }, //  MASK
  { (uint32_t)TSI578_SPX_TX_Q_PERIOD(0)     , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_RX_Q_D_THRESH(0)   , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_RX_Q_STATUS(0)   , (uint32_t)0x100, (uint32_t)TSI578_SPX_RX_Q_STATUS_CONG_THRESH }, //  MASK
  { (uint32_t)TSI578_SPX_RX_Q_PERIOD(0)     , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_REORDER_CTR(0)     , (uint32_t)0x100, (uint32_t)TSI578_SPX_REORDER_CTR_THRESH  }, //  MASK
  { (uint32_t)TSI578_SPX_ISF_WM(0)          , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_WRR_0(0)           , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_WRR_1(0)           , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_WRR_2(0)           , (uint32_t)0x100, ALL_BITS },
  { (uint32_t)TSI578_SPX_WRR_3(0)           , (uint32_t)0x100, ALL_BITS }
};

#define PRESERVED_REGS_SIZE ((uint32_t)(sizeof(reg_offsets)/sizeof(reg_offsets[0])))
typedef uint32_t preserved_regs[PRESERVED_REGS_SIZE];

const struct scrpad_info *tsi57x_get_scrpad_info()
{
	return scratchpad_const;
}

uint32_t tsi57x_init_scratchpad(DAR_DEV_INFO_t *DAR_info)
{
	uint32_t rc;
	uint8_t idx;

	for (idx = SCRPAD_FIRST_IDX; idx < MAX_DAR_SCRPAD_IDX; idx++) {
		if (SCRPAD_EOF_OFFSET == scratchpad_const[idx].offset) {
			rc = RIO_ERR_REG_ACCESS_FAIL;
			break;
		}

		rc = ReadReg(DAR_info, scratchpad_const[idx].offset,
				&DAR_info->scratchpad[idx]);
		if (RIO_SUCCESS != rc)
			break;
	}
	return rc;
}

uint32_t tsi57x_init_sw_pmr(DAR_DEV_INFO_t *dev_info,
		port_mac_relations_t **sw_pmr)
{
	uint32_t rc = RIO_SUCCESS;

	switch ((dev_info->devID & TSI578_RIO_DEV_IDENT_DEVI) >> 16) {
	case TSI578_RIO_DEVID_VAL:
		*sw_pmr = tsi578_pmr;
		break;
	case TSI574_RIO_DEVID_VAL:
		*sw_pmr = tsi574_pmr;
		break;
	case TSI572_RIO_DEVID_VAL:
		*sw_pmr = tsi572_pmr;
		break;
	case TSI577_RIO_DEVID_VAL:
		*sw_pmr = tsi577_pmr;
		break;
	default:
		rc = RIO_ERR_NO_FUNCTION_SUPPORT;
	}

	return rc;
}

static uint32_t tsi57x_set_lrto(DAR_DEV_INFO_t *dev_info,
		rio_pc_set_config_in_t *in_parms)
{
	uint32_t lrto;

	// LRTO register has a granularity of 320 nsec.
	lrto = (in_parms->lrto * 10) / 32;
	if (lrto > 0xFFFFFF) {
		lrto = 0xFFFFFF;
	}

	if (!lrto) {
		lrto = 1;
	}

	return DARRegWrite(dev_info, TSI578_RIO_SW_LT_CTL, lrto << 8);
}

static uint32_t tsi57x_get_lrto(DAR_DEV_INFO_t *dev_info,
		rio_pc_set_config_out_t *out_parms)
{
	uint32_t rc;
	uint32_t lrto;

	rc = DARRegRead(dev_info, TSI578_RIO_SW_LT_CTL, &lrto);
	if (RIO_SUCCESS != rc) {
		return rc;
	}

	// LRTO granularity is 320 nanoseconds.
	lrto = ((lrto >> 8) * 32) / 10;
	out_parms->lrto = lrto;

	return rc;
}

static void tsi57x_computer_lane_reg(port_mac_relations_t *sw_pmr, port_cfg_chg_t *chg,
		uint8_t port_num, uint8_t lane_num,
		bool tx_linvert,
		bool rx_linvert)
{
	uint8_t mac = sw_pmr[port_num].mac_num;
	uint32_t reg_val = chg->laneRegs[mac][lane_num];

	if (tx_linvert) {
		reg_val |= TSI57X_HIDDEN_SERDES_TX_INV;
	} else {
		reg_val &= ~TSI57X_HIDDEN_SERDES_TX_INV;
	}
	if (rx_linvert) {
		reg_val |= TSI57X_HIDDEN_SERDES_RX_INV;
	} else {
		reg_val &= ~TSI57X_HIDDEN_SERDES_RX_INV;
	}

	if (reg_val != chg->laneRegs[mac][lane_num]) {
		chg->laneRegs[mac][lane_num] = reg_val;
		chg->laneRegsChanged[mac][lane_num] = true;
	}
}

static uint32_t tsi57x_compute_port_config_changes(DAR_DEV_INFO_t *dev_info,
		port_cfg_chg_t *chg, port_mac_relations_t *sw_pmr,
		rio_pc_one_port_config_t *in_parms_sorted,
		rio_pc_set_config_out_t *out_parms)
{
	uint32_t rc = RIO_SUCCESS;
	rio_pc_get_config_in_t curr_cfg_in;
	rio_pc_get_config_out_t curr_cfg_out;
	uint8_t mac, pnum, port_num, lane_idx, change_requested;

	rc = tsi57x_init_sw_pmr(dev_info, &sw_pmr);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x21);
		goto exit;
	}

	/* First, get current operating mode of ALL ports
	 * Curr_cfg_out will have information about all ports in
	 * ascending order, contiguously, for all ports on the device.
	 */

	curr_cfg_in.ptl.num_ports = RIO_ALL_PORTS;
	rc = rio_pc_get_config(dev_info, &curr_cfg_in, &curr_cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = curr_cfg_out.imp_rc;
		rc = PC_SET_CONFIG(0x22);
		goto exit;
	}

	// Get DLOOP values
	for (mac = 0; mac < TSI578_MAX_MAC; mac++) {
		rc = DARRegRead(dev_info, TSI578_SMACX_DLOOP_CLK_SEL(mac * 2),
				&chg->dloopCtl[mac]);

		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SET_CONFIG(0x23);
			goto exit;
		}

		chg->dloopCtlOrig[mac] = chg->dloopCtl[mac];
	}

	// Get SPXCTL values
	for (pnum = 0; pnum < TSI57X_NUM_PORTS(dev_info); pnum++) {
		rc = DARRegRead(dev_info, TSI578_SPX_CTL(pnum),
				&chg->spxCtl[pnum]);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = rc;
			rc = PC_SET_CONFIG(0x24);
			goto exit;
		}
		chg->spxCtlOrig[pnum] = chg->spxCtl[pnum];
	}

	for (pnum = 0; pnum < TSI57X_NUM_PORTS(dev_info); pnum++) {
		// If this entry is valid, and there's a change in configuration,
		// then deal with it.
		change_requested =
				(chg->valid[pnum]
						&& memcmp(
								(void *)(&curr_cfg_out.pc[pnum]),
								(void *)(&in_parms_sorted[pnum]),
								sizeof(rio_pc_one_port_config_t)));

		if (change_requested) {
			uint32_t start_dloop;
			// Change the DLOOP CONTROL VALUE
			mac = sw_pmr[pnum].mac_num;

			start_dloop = chg->dloopCtl[mac];
			// Set the powerup state
			if (in_parms_sorted[pnum].powered_up) {
				chg->dloopCtl[mac] &=
						~sw_pmr[pnum].pwr_down_mask;
			} else {
				chg->dloopCtl[mac] |=
						sw_pmr[pnum].pwr_down_mask;
			}

			// If we're powering up a port, the starting value for SPx_CTL is wrong.
			// Correct it.
			if ((chg->dloopCtl[mac] != start_dloop)
					&& in_parms_sorted[pnum].powered_up) {
				chg->spxCtl[pnum] = 0x00600001;
			}

			// If port is powered up, change configuration as necessary
			if (sw_pmr[pnum].lane_count_4x && in_parms_sorted[pnum].powered_up) {
				// Port width - 4x or 1x
				if (rio_pc_pw_1x == in_parms_sorted[pnum].pw) {
					chg->dloopCtl[mac] |= TSI578_SMACX_DLOOP_CLK_SEL_MAC_MODE;
				} else {
					chg->dloopCtl[mac] &= ~TSI578_SMACX_DLOOP_CLK_SEL_MAC_MODE;
				}

				if (rio_pc_pw_4x == in_parms_sorted[pnum].pw) {
					// Clear any overrides in the SPx_CTL register
					chg->spxCtl[pnum] =
							(chg->spxCtl[pnum]
									& ~TSI578_SPX_CTL_OVER_PWIDTH)
									| RIO_SPX_CTL_PTW_OVER_NONE;

					// Apply the requested RX and TX swap values
					if (rio_lswap_none == in_parms_sorted[pnum].tx_lswap) {
						chg->dloopCtl[mac] &=
								~TSI578_SMACX_DLOOP_CLK_SEL_SWAP_TX;
					} else {
						chg->dloopCtl[mac] |=
								TSI578_SMACX_DLOOP_CLK_SEL_SWAP_TX;
					}

					if (rio_lswap_none == in_parms_sorted[pnum].rx_lswap) {
						chg->dloopCtl[mac] &=
								~TSI578_SMACX_DLOOP_CLK_SEL_SWAP_RX;
					} else {
						chg->dloopCtl[mac] |=
								TSI578_SMACX_DLOOP_CLK_SEL_SWAP_RX;
					}
				} else {
					// Lane swapping is not supported in any 1x mode.
					chg->dloopCtl[mac] &=
							~(TSI578_SMACX_DLOOP_CLK_SEL_SWAP_RX
									| TSI578_SMACX_DLOOP_CLK_SEL_SWAP_TX);

					if (rio_pc_pw_1x == in_parms_sorted[pnum].pw) {
						// Clear any overrides in the SPx_CTL register
						chg->spxCtl[pnum] =
								(chg->spxCtl[pnum]
										& ~TSI578_SPX_CTL_OVER_PWIDTH)
										| RIO_SPX_CTL_PTW_OVER_NONE;
					} else {
						// Need to set overrides in the port width control register.
						if (rio_pc_pw_1x_l0 == in_parms_sorted[pnum].pw) {
							chg->spxCtl[pnum] =
									(chg->spxCtl[pnum]
											& ~TSI578_SPX_CTL_OVER_PWIDTH)
											| RIO_SPX_CTL_PTW_OVER_1X_L0;
						} else {
							chg->spxCtl[pnum] =
									(chg->spxCtl[pnum]
											& ~TSI578_SPX_CTL_OVER_PWIDTH)
											| RIO_SPX_CTL_PTW_OVER_1X_LR;
						}
					}
				}

				// Set link speed
				chg->dloopCtl[mac] =
						(chg->dloopCtl[mac]
								& ~(TSI578_SMACX_DLOOP_CLK_SEL_CLK_SEL))
								| (uint32_t)(in_parms_sorted[pnum].ls);
			} // 4x port && Powered up

			// Change the SPX_CTL value
			if (in_parms_sorted[pnum].xmitter_disable) {
				chg->spxCtl[pnum] |= TSI578_SPX_CTL_PORT_DIS;
			} else {
				chg->spxCtl[pnum] &= ~TSI578_SPX_CTL_PORT_DIS;
			}

			if (in_parms_sorted[pnum].port_lockout) {
				chg->spxCtl[pnum] |=
						TSI578_SPX_CTL_PORT_LOCKOUT;
			} else {
				chg->spxCtl[pnum] &=
						~TSI578_SPX_CTL_PORT_LOCKOUT;
			}

			if (in_parms_sorted[pnum].nmtc_xfer_enable) {
				chg->spxCtl[pnum] |= TSI578_SPX_CTL_INPUT_EN
						| TSI578_SPX_CTL_OUTPUT_EN;
			} else {
				chg->spxCtl[pnum] &= ~(TSI578_SPX_CTL_INPUT_EN
						| TSI578_SPX_CTL_OUTPUT_EN);
			}
		}
	}

	// Update lane register values for the 4x ports.
	// If the port is operating in 4x mode, take the lane inversion values from the port.
	// Otherwise, take the lane inversion values from the dependent port(s).

	for (port_num = 0; port_num < TSI57X_NUM_PORTS(dev_info); port_num++) {
		if (!chg->valid[port_num]
				|| !in_parms_sorted[port_num].port_available
				|| !in_parms_sorted[port_num].powered_up)
			continue;

		if (rio_pc_pw_4x == in_parms_sorted[port_num].pw) {
			for (lane_idx = 0; lane_idx < TSI578_MAX_PORT_LANES;
					lane_idx++) {
				tsi57x_computer_lane_reg(sw_pmr, chg, port_num,
						lane_idx,
						in_parms_sorted[port_num].tx_linvert[lane_idx],
						in_parms_sorted[port_num].rx_linvert[lane_idx]);
			}
		} else {
			//  1x ports always have only one lane to check for inversions :)
			uint8_t dep_lnum = sw_pmr[port_num].first_mac_lane;

			tsi57x_computer_lane_reg(sw_pmr, chg, port_num, dep_lnum,
					in_parms_sorted[port_num].tx_linvert[0],
					in_parms_sorted[port_num].rx_linvert[0]);

		}
	}

exit:
	return rc;
}

static uint32_t tsi57x_determine_ports_to_skip(DAR_DEV_INFO_t *dev_info,
		bool oob_reg_acc, uint8_t *ports_to_skip, uint32_t *imp_rc,
		port_mac_relations_t *sw_pmr,
		bool reset_port)
{
	uint32_t rc = RIO_SUCCESS;
	uint8_t idx;
	uint8_t conn_port;
	rio_pc_get_status_in_t stat_in;
	rio_pc_get_status_out_t stat_out;
	uint32_t sw_port;

	for (idx = 0; idx < MAX_PORTS_TO_SKIP; idx++) {
		ports_to_skip[idx] = RIO_ALL_PORTS;
	}

	// If resetting the connected port is OK, or
	// if we're running on the Tsi577, there are no
	// dependencies between powering down one port
	// and another.  Just return.
	if (oob_reg_acc || (TSI577_RIO_DEVID_VAL == DEV_CODE(dev_info))) {
		return rc;
	}

	// Determine the connected port.
	rc = DARRegRead(dev_info, TSI578_RIO_SW_PORT, &sw_port);
	if (RIO_SUCCESS != rc) {
		*imp_rc = PC_DET_PORTS_2_SKIP(1);
		goto exit;
	}

	conn_port = (uint8_t)(sw_port & TSI578_RIO_SW_PORT_PORT_NUM);

	// Check status:  If conn_port is not PORT_OK, then
	// it can't be in use to access this device.  Reset this port.
	stat_in.ptl.num_ports = 1;
	stat_in.ptl.pnums[0] = conn_port;

	rc = tsi57x_rio_pc_get_status(dev_info, &stat_in, &stat_out);
	if (RIO_SUCCESS != rc) {
		*imp_rc = stat_out.imp_rc;
		goto exit;
	}

	if (!stat_out.ps[0].port_ok) {
		*imp_rc = PC_DET_PORTS_2_SKIP(2);
		goto exit;
	}

	// The connected port is in use.
	// Initialize ports_to_skip based on port_in and conn_port relationship

	if (reset_port) {
		if (sw_pmr[conn_port].lane_count_4x) {
			// If the connected port is a 4x port, then don't reset it.
			ports_to_skip[0] = conn_port;
			*imp_rc = PC_DET_PORTS_2_SKIP(3);
		} else {
			// If the connected port is not a 4x port, then don't reset it
			// and don't reset the 4x port on the same MAC.
			ports_to_skip[0] = sw_pmr[conn_port].other_mac_ports[0];
			ports_to_skip[1] = conn_port;
			*imp_rc = PC_DET_PORTS_2_SKIP(4);
		}
	} else {
		// Must skip reconfiguration of all ports on the connected port's MAC.
		ports_to_skip[0] = conn_port;
		ports_to_skip[1] = sw_pmr[conn_port].other_mac_ports[0];
		*imp_rc = PC_DET_PORTS_2_SKIP(5);
	}

exit:
	return rc;
}

static uint32_t tsi57x_preserve_regs_for_powerup(DAR_DEV_INFO_t *dev_info,
		uint8_t port_num, preserved_regs *saved_regs,
		bool do_all_regs)
{
	uint32_t rc = RIO_SUCCESS;
	uint32_t idx;

	if (!do_all_regs) {
		goto exit;
	}

	for (idx = 0; idx < PRESERVED_REGS_SIZE; idx++) {
		rc =
				DARRegRead(dev_info,
						reg_offsets[idx].offset
								+ (port_num
										* reg_offsets[idx].per_port_offset),
						&((*saved_regs)[idx]));
		if (RIO_SUCCESS != rc) {
			goto exit;
		}
		(*saved_regs)[idx] &= reg_offsets[idx].mask;
	}

exit:
	return rc;
}

static uint32_t tsi57x_restore_regs_from_powerup(DAR_DEV_INFO_t *dev_info,
		uint8_t port_num, preserved_regs *saved_regs,
		bool do_all_regs)
{
	uint32_t rc = RIO_SUCCESS;
	uint32_t idx;

	for (idx = 0; idx < MAX_DAR_SCRPAD_IDX; idx++) {
		/* Multicast programming registers do not need to be restored. */
		if ((scratchpad_const[idx].offset == TSI578_RIO_MC_MASK_CFG)
				|| (scratchpad_const[idx].offset
						== TSI578_RIO_MC_DESTID_CFG)
				|| (scratchpad_const[idx].offset
						== TSI578_RIO_MC_DESTID_ASSOC)) {
			continue;
		}

		rc = DARRegWrite(dev_info, scratchpad_const[idx].offset,
				dev_info->scratchpad[idx]);
		if (RIO_SUCCESS != rc) {
			goto exit;
		}
	}

	if (!do_all_regs) {
		goto exit;
	}

	for (idx = 0; idx < PRESERVED_REGS_SIZE; idx++) {
		(*saved_regs)[idx] &= reg_offsets[idx].mask;
		rc =
				DARRegWrite(dev_info,
						reg_offsets[idx].offset
								+ (port_num
										* reg_offsets[idx].per_port_offset),
						(*saved_regs)[idx]);
		if (RIO_SUCCESS != rc) {
			goto exit;
		}
	}

exit:
	return rc;
}

static uint32_t tsi57x_update_lane_inversions(DAR_DEV_INFO_t *dev_info,
		uint8_t port_num, port_cfg_chg_t *chg,
		port_mac_relations_t *sw_pmr)
{
	uint32_t rc = RIO_SUCCESS;
	uint8_t mac_num = sw_pmr[port_num].mac_num, lane_idx;

	for (lane_idx = 0; lane_idx < TSI578_MAX_PORT_LANES; lane_idx++) {
		if (chg->laneRegsChanged[mac_num][lane_idx]) {
			rc = DARRegWrite(dev_info,
					TSI57X_HIDDEN_SERDES_REG(mac_num,
							lane_idx),
					chg->laneRegs[mac_num][lane_idx]);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}
		}
	}

exit:
	return rc;
}

static uint32_t tsi57x_set_config_init_parms_check_conflicts_all_ports(
		DAR_DEV_INFO_t *dev_info, port_cfg_chg_t *chg,
		port_mac_relations_t *sw_pmr, rio_pc_set_config_in_t *in_parms,
		rio_pc_set_config_in_t *in_parms_sorted,
		rio_pc_set_config_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_FEATURE_NOT_SUPPORTED;
	uint8_t port_num;
	bool check;

	out_parms->num_ports = TSI57X_NUM_PORTS(dev_info);

	if ((in_parms->pc[0].pw >= rio_pc_pw_last)
			|| (in_parms->pc[0].pw == rio_pc_pw_2x)
			|| (in_parms->pc[0].pw == rio_pc_pw_1x_l1)
			|| (in_parms->pc[0].ls >= rio_pc_ls_5p0)) {
		out_parms->imp_rc = PC_SET_CONFIG(0x50);
		goto exit;
	}

	if (in_parms->pc[0].pw == rio_pc_pw_1x) {
		// Make sure that no lane swapping is attempted,
		// and that no lane inversion is requested.
		check = (rio_lswap_none != in_parms->pc[0].tx_lswap);
		check |= (rio_lswap_none != in_parms->pc[0].rx_lswap);
		if (check || in_parms->pc[0].tx_linvert[0]
				|| in_parms->pc[0].rx_linvert[0]) {
			out_parms->imp_rc = PC_SET_CONFIG(0x51);
			goto exit;
		}
	}

	for (port_num = 0; port_num < TSI57X_NUM_PORTS(dev_info); port_num +=
			2) {
		chg->valid[port_num] = true;
		chg->valid[port_num + 1] = true;
		in_parms_sorted->pc[port_num].pnum = port_num;
		in_parms_sorted->pc[port_num + 1].pnum = port_num + 1;

		if (sw_pmr[port_num].lane_count_4x) {
			// Usual 2 port 4 lane MAC.  Always set up the even port,
			// If the even port is set up for 1x, duplicate the setup
			// for the odd port.
			// If the even port is set up for 4x, make the odd port
			// unavailable and powered down.
			memcpy((void *)(&in_parms_sorted->pc[port_num]),
					(void *)(&in_parms->pc[0]),
					sizeof(rio_pc_one_port_config_t));

			if (in_parms->pc[0].pw == rio_pc_pw_1x) {
				memcpy(
						(void *)(&in_parms_sorted->pc[port_num
								+ 1]),
						(void *)(&in_parms->pc[0]),
						sizeof(rio_pc_one_port_config_t));
			} else {
				// If even ports are being set to 4x, then
				in_parms_sorted->pc[port_num + 1].port_available =
						false;
				in_parms_sorted->pc[port_num + 1].powered_up =
						false;
			}
		} else {
			// Two 1x ports for a Tsi577 4 port 4 lane MAC.
			// If the setup is for 1x, clone both ports to be 1x.
			// If the setup is for 4x, both of these ports must be
			// unavailable and powered down.
			if (in_parms->pc[0].pw == rio_pc_pw_1x) {
				memcpy((void *)(&in_parms_sorted->pc[port_num]),
						(void *)(&in_parms->pc[0]),
						sizeof(rio_pc_one_port_config_t));
				memcpy(
						(void *)(&in_parms_sorted->pc[port_num
								+ 1]),
						(void *)(&in_parms->pc[0]),
						sizeof(rio_pc_one_port_config_t));
			} else {
				// If even ports are being set to 4x, then
				in_parms_sorted->pc[port_num].port_available =
						false;
				in_parms_sorted->pc[port_num + 1].port_available =
						false;
				in_parms_sorted->pc[port_num].powered_up =
						false;
				in_parms_sorted->pc[port_num + 1].powered_up =
						false;
			}
		}
	}

	rc = RIO_SUCCESS;
exit:
	return rc;
}

static uint32_t tsi57x_set_config_init_parms_check_conflict(
		DAR_DEV_INFO_t *dev_info, port_cfg_chg_t *chg,
		port_mac_relations_t *sw_pmr, rio_pc_set_config_in_t *in_parms,
		rio_pc_set_config_in_t *in_parms_sorted,
		rio_pc_set_config_out_t *out_parms)
{
	uint32_t rc;
	uint8_t port_num;
	uint8_t pnum;
	uint8_t port_idx;
	uint8_t lane_num;
	bool check;
	bool check2;

	// Have to read all of the lane registers to know if they have changed or not.

	for (port_num = 0; port_num < TSI57X_NUM_PORTS(dev_info); port_num++) {
		uint8_t mac = sw_pmr[port_num].mac_num;
		chg->valid[port_num] = false;
		for (lane_num = 0; lane_num < TSI578_MAX_PORT_LANES;
				lane_num++) {
			if (sw_pmr[port_num].lane_count_4x) {
				rc = DARRegRead(dev_info,
						TSI57X_HIDDEN_SERDES_REG(mac,
								lane_num),
						&chg->laneRegs[mac][lane_num]);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = PC_SET_CONFIG(0x30);
					goto exit;
				}
			}
			chg->laneRegsChanged[mac][lane_num] = false;
		}
	}

	if (RIO_ALL_PORTS == in_parms->num_ports) {
		rc = tsi57x_set_config_init_parms_check_conflicts_all_ports(
				dev_info, chg, sw_pmr, in_parms,
				in_parms_sorted, out_parms);
		goto exit;
	}
	// NOT ALL PORTS
	rc = RIO_ERR_INVALID_PARAMETER;

	if (in_parms->num_ports >= TSI57X_NUM_PORTS(dev_info)) {
		out_parms->imp_rc = PC_SET_CONFIG(0x99);
		goto exit;
	}

	out_parms->num_ports = in_parms->num_ports;
	for (port_idx = 0; port_idx < out_parms->num_ports; port_idx++) {
		port_num = in_parms->pc[port_idx].pnum;
		if ((TSI57X_NUM_PORTS(dev_info) <= port_num)
				|| (chg->valid[port_num])) {
			rc = RIO_ERR_INVALID_PARAMETER;
			out_parms->imp_rc = PC_SET_CONFIG(0x31);
			goto exit;
		}

		chg->valid[port_num] = true;
		memcpy((void *)(&in_parms_sorted->pc[port_num]),
				(void *)(&in_parms->pc[port_idx]),
				sizeof(rio_pc_one_port_config_t));
	}

	// Only check for conflicts between the 4x port and the 1x ports.
	for (port_num = 0; port_num < (TSI57X_NUM_PORTS(dev_info));
			port_num++) {
		// Tsi57x family does not support lane speeds above 3.125 Gbaud
		// or 2 lane port widths.
		if (((in_parms_sorted->pc[port_num].ls > rio_pc_ls_3p125)
				|| (in_parms_sorted->pc[port_num].pw
						== rio_pc_pw_2x)
				|| (in_parms_sorted->pc[port_num].pw
						== rio_pc_pw_1x_l1))
				&& chg->valid[port_num]) {
			rc = RIO_ERR_FEATURE_NOT_SUPPORTED;
			out_parms->imp_rc = PC_SET_CONFIG(0x32);
			goto exit;
		}

		if ((in_parms_sorted->pc[port_num].pw >= rio_pc_pw_last)
				&& chg->valid[port_num]) {
			out_parms->imp_rc = PC_SET_CONFIG(0x33);
			goto exit;
		}

		if ((4 == sw_pmr[port_num].lane_count_4x)
				&& chg->valid[port_num]) {
			port_idx = 0;
			while ((port_idx < MAX_OTHER_MAC_PORTS)
					&& (RIO_ALL_PORTS
							!= sw_pmr[port_num].other_mac_ports[port_idx])) {
				// The dependent port is always 1x.
				pnum =
						sw_pmr[port_num].other_mac_ports[port_idx];
				if (chg->valid[pnum]) {
					// Check that if the 4x port uses 4 lanes and is available, then the 1x port is not available
					if (in_parms_sorted->pc[port_num].port_available
							&& (in_parms_sorted->pc[port_num].pw
									!= rio_pc_pw_1x)
							&& in_parms_sorted->pc[pnum].port_available) {
						rc = RIO_ERR_INVALID_PARAMETER;
						out_parms->imp_rc =
								PC_SET_CONFIG(
										0x34);
						goto exit;
					}

					// Check that if the 4x port is powered down (not on a Tsi577), then the 1x port is either
					// not available or powered down or both.
					if (!in_parms_sorted->pc[port_num].powered_up
							&& in_parms_sorted->pc[pnum].port_available
							&& in_parms_sorted->pc[pnum].powered_up
							&& !(TSI577_RIO_DEVID_VAL
									== DEV_CODE(
											dev_info))) {
						rc = RIO_ERR_INVALID_PARAMETER;
						out_parms->imp_rc =
								PC_SET_CONFIG(
										0x35);
						goto exit;
					}

					// Check that the lane speeds of the ports on the same quad are all the same.
					if (in_parms_sorted->pc[port_num].port_available
							&& in_parms_sorted->pc[pnum].port_available
							&& (in_parms_sorted->pc[port_num].ls
									!= in_parms_sorted->pc[pnum].ls)) {
						rc = RIO_ERR_FEATURE_NOT_SUPPORTED;
						out_parms->imp_rc = PC_SET_CONFIG(0x36);
						goto exit;
					}

					// Tsi57x family does not support lane swapping for downgraded 4x ports
					check = (rio_pc_pw_1x == in_parms_sorted->pc[port_num].pw);
					check |= (rio_pc_pw_1x_l0 == in_parms_sorted->pc[port_num].pw);
					check |= (rio_pc_pw_1x_l2 == in_parms_sorted->pc[port_num].pw);

					check2 = (rio_lswap_none != in_parms_sorted->pc[port_num].tx_lswap);
					check2 |= (rio_lswap_none != in_parms_sorted->pc[port_num].rx_lswap);
					if (check && check2) {
						rc = RIO_ERR_FEATURE_NOT_SUPPORTED;
						out_parms->imp_rc = PC_SET_CONFIG(0x37);
						goto exit;
					}
				}
				port_idx++;
			}
		}
	}

	rc = RIO_SUCCESS;
exit:
	return rc;
}

static void tsi57x_rst_policy_vals(rio_pc_rst_handling rst_policy_in,
		uint32_t *spx_mode_val, rio_pc_rst_handling *rst_policy_out)
{
	*spx_mode_val = 0;
	*rst_policy_out = rst_policy_in;

	// Tsi57x devices support
	// - reset the device
	// - interrupt on reset
	// - ignore
	// Rst_pw and Rst_port both get translated to "ignore".
	if (rio_pc_rst_device == rst_policy_in) {
		*spx_mode_val = TSI578_SPX_MODE_SELF_RST;
	} else {
		if (rio_pc_rst_int == rst_policy_in) {
			*spx_mode_val = TSI578_SPX_MODE_RCS_INT_EN;
		} else {
			*rst_policy_out = rio_pc_rst_ignore;
		}
	}
}

static uint32_t tsi57x_update_reset_policy(DAR_DEV_INFO_t *dev_info, uint8_t pnum,
		uint32_t spx_mode_val, uint32_t *imp_rc)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t ctl_indep;

	if (pnum >= TSI57X_NUM_PORTS(dev_info)) {
		*imp_rc = UPDATE_RESET(1);
		goto exit;
	}

	rc = DARRegWrite(dev_info, TSI578_SPX_MODE(pnum), spx_mode_val);
	if (RIO_SUCCESS != rc) {
		*imp_rc = UPDATE_RESET(2);
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI578_SPX_CTL_INDEP(pnum), &ctl_indep);
	if (RIO_SUCCESS != rc) {
		*imp_rc = UPDATE_RESET(3);
		goto exit;
	}

	// Make a "note" of whether or not reset interrupts should be
	// enabled in SPx_CTL_INDEP[30], used by error management routines
	// to know whether or not to enable/disable reset interrupts.
	if (spx_mode_val & TSI578_SPX_MODE_RCS_INT_EN) {
		ctl_indep |= TSI578_SPX_CTL_INDEP_RSVD1;
	} else {
		ctl_indep &= ~TSI578_SPX_CTL_INDEP_RSVD1;
	}

	rc = DARRegWrite(dev_info, TSI578_SPX_CTL_INDEP(pnum), ctl_indep);
	if (RIO_SUCCESS != rc) {
		*imp_rc = UPDATE_RESET(4);
		goto exit;
	}

exit:
	return rc;
}

static uint32_t tsi57x_reset_lp(DAR_DEV_INFO_t *dev_info, uint8_t port_num,
		uint32_t *imp_rc)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint32_t lr_cmd = STYPE1_LREQ_CMD_RST_DEV;

	if (port_num >= TSI57X_NUM_PORTS(dev_info)) {
		*imp_rc = TSI57X_LP(0x10 + port_num);
		goto exit;
	}

	rc = DARRegWrite(dev_info, TSI578_SPX_LM_REQ(port_num), lr_cmd);
	if (RIO_SUCCESS != rc) {
		*imp_rc = TSI57X_LP(0x20 + port_num);
		goto exit;
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_pc_get_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_config_in_t *in_parms,
		rio_pc_get_config_out_t *out_parms)
{
	uint32_t rc;
	uint8_t port_num;
	uint32_t port_idx;
	uint32_t dloopRegVal, SerDesRegVal, spxCtl;
	int32_t first_lane, num_lanes, lane_num;
	port_mac_relations_t *sw_pmr;
	struct DAR_ptl good_ptl;

	out_parms->num_ports = 0;
	out_parms->lrto = 0;
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

	rc = tsi57x_init_sw_pmr(dev_info, &sw_pmr);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_GET_CONFIG(3);
		goto exit;
	}

	rc = tsi57x_get_lrto(dev_info, out_parms);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_GET_CONFIG(4);
		goto exit;
	}

	for (port_idx = 0; port_idx < out_parms->num_ports; port_idx++) {
		port_num = out_parms->pc[port_idx].pnum;
		out_parms->pc[port_idx].pw = rio_pc_pw_last;
		out_parms->pc[port_idx].ls = rio_pc_ls_last;
		out_parms->pc[port_idx].fc = rio_pc_fc_rx;
		out_parms->pc[port_idx].iseq = rio_pc_is_one;
		out_parms->pc[port_idx].nmtc_xfer_enable = false;
		out_parms->pc[port_idx].xmitter_disable = false;
		out_parms->pc[port_idx].port_lockout = false;
		out_parms->pc[port_idx].rx_lswap = rio_lswap_none;
		out_parms->pc[port_idx].tx_lswap = rio_lswap_none;
		for (lane_num = 0; lane_num < TSI578_MAX_PORT_LANES;
				lane_num++) {
			out_parms->pc[port_idx].tx_linvert[lane_num] = false;
			out_parms->pc[port_idx].rx_linvert[lane_num] = false;
		}

		/* 1x ports are not available when the 4x port is configured
		 to be a 4x port.
		 */
		rc = DARRegRead(dev_info,
				TSI578_SMACX_DLOOP_CLK_SEL(
						sw_pmr[port_num].mac_num * 2),
				&dloopRegVal);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_GET_CONFIG(4);
			goto exit;
		}

		first_lane = sw_pmr[port_num].first_mac_lane;

		/* Deal with 1x port configuration. */
		if (dloopRegVal & TSI578_SMACX_DLOOP_CLK_SEL_MAC_MODE) {
			num_lanes = sw_pmr[port_num].lane_count_1x;
		} else {
			num_lanes = sw_pmr[port_num].lane_count_4x;
		}

		switch (num_lanes) {
		case 0: // If the port is not available, we still need to
			// check if the port is powered up/down.
			out_parms->pc[port_idx].port_available = false;
			break;
		case 1:
			out_parms->pc[port_idx].port_available = true;
			out_parms->pc[port_idx].pw = rio_pc_pw_1x;
			// Get the real swap values for 4x ports, or for a 1x
			// port that could be a 4x port.
			if (4 == sw_pmr[port_num].lane_count_4x) {
				out_parms->pc[port_idx].rx_lswap =
						(dloopRegVal & TSI578_SMACX_DLOOP_CLK_SEL_SWAP_RX) ?
								rio_lswap_ABCD_DCBA : rio_lswap_none;
				out_parms->pc[port_idx].tx_lswap =
						(dloopRegVal & TSI578_SMACX_DLOOP_CLK_SEL_SWAP_TX) ?
								rio_lswap_ABCD_DCBA : rio_lswap_none;
			}
			break;
		case 4:
			out_parms->pc[port_idx].port_available = true;
			// Check for port width overrides...
			rc = DARRegRead(dev_info, TSI578_SPX_CTL(port_num),
					&spxCtl);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_GET_CONFIG(5);
				goto exit;
			}
			switch (spxCtl & RIO_SPX_CTL_PTW_OVER) {
			case RIO_SPX_CTL_PTW_OVER_NONE:
				out_parms->pc[port_idx].pw = rio_pc_pw_4x;
				break;
			case RIO_SPX_CTL_PTW_OVER_1X_L0:
				out_parms->pc[port_idx].pw = rio_pc_pw_1x_l0;
				break;
			case RIO_SPX_CTL_PTW_OVER_1X_LR:
				out_parms->pc[port_idx].pw = rio_pc_pw_1x_l2;
				break;
			default:
				out_parms->pc[port_idx].pw = rio_pc_pw_last;
			}
			out_parms->pc[port_idx].rx_lswap =
					(dloopRegVal & TSI578_SMACX_DLOOP_CLK_SEL_SWAP_RX) ?
							rio_lswap_ABCD_DCBA : rio_lswap_none;
			out_parms->pc[port_idx].tx_lswap =
					(dloopRegVal & TSI578_SMACX_DLOOP_CLK_SEL_SWAP_TX) ?
							rio_lswap_ABCD_DCBA : rio_lswap_none;
			break;
		default:
			rc = RIO_ERR_NOT_EXPECTED_RETURN_VALUE;
			out_parms->imp_rc = PC_GET_CONFIG(6);
			goto exit;
		}

		switch (dloopRegVal & TSI578_SMACX_DLOOP_CLK_SEL_CLK_SEL) {
		case 0:
			out_parms->pc[port_idx].ls = rio_pc_ls_1p25;
			break;
		case 1:
			out_parms->pc[port_idx].ls = rio_pc_ls_2p5;
			break;
		default:
			out_parms->pc[port_idx].ls = rio_pc_ls_3p125;
			break;
		}

		if (sw_pmr[port_num].mac_num != sw_pmr[port_num].pwr_mac_num) {
			rc =
					DARRegRead(dev_info,
							TSI578_SMACX_DLOOP_CLK_SEL(
									sw_pmr[port_num].pwr_mac_num
											* 2),
							&dloopRegVal);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_GET_CONFIG(7);
				goto exit;
			}
		}

		out_parms->pc[port_idx].powered_up =
				(dloopRegVal & sw_pmr[port_num].pwr_down_mask) ?
						false : true;

		if (out_parms->pc[port_idx].powered_up) {
			rc = DARRegRead(dev_info, TSI578_SPX_CTL(port_num),
					&spxCtl);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_GET_CONFIG(8);
				goto exit;
			}

			out_parms->pc[port_idx].xmitter_disable =
					(spxCtl & TSI578_SPX_CTL_PORT_DIS) ?
							true : false;
			out_parms->pc[port_idx].port_lockout =
					(spxCtl & TSI578_SPX_CTL_PORT_LOCKOUT) ?
							true : false;
			out_parms->pc[port_idx].nmtc_xfer_enable =
					((spxCtl & TSI578_SPX_CTL_INPUT_EN)
							&& (spxCtl
									& TSI578_SPX_CTL_OUTPUT_EN)) ?
							true : false;

			for (lane_num = first_lane;
					lane_num < (first_lane + num_lanes);
					lane_num++) {
				rc =
						DARRegRead(dev_info,
								TSI57X_HIDDEN_SERDES_REG(
										(sw_pmr[port_num].mac_num),
										lane_num),
								&SerDesRegVal);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = PC_GET_CONFIG(9);
					goto exit;
				}

				out_parms->pc[port_idx].rx_linvert[lane_num
						- first_lane] =
						(SerDesRegVal
								& TSI57X_HIDDEN_SERDES_RX_INV) ?
								true : false;
				out_parms->pc[port_idx].tx_linvert[lane_num
						- first_lane] =
						(SerDesRegVal
								& TSI57X_HIDDEN_SERDES_TX_INV) ?
								true : false;
			}
		}
	}
exit:
	return rc;
}

uint32_t tsi57x_rio_pc_set_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_set_config_in_t *in_parms,
		rio_pc_set_config_out_t *out_parms)
{
	uint32_t rc = RIO_SUCCESS;
	rio_pc_get_config_in_t curr_cfg_in;
	rio_pc_set_config_in_t in_parms_sorted;
	port_mac_relations_t *sw_pmr;
	port_cfg_chg_t chg;
	uint8_t even_odd, port_num, idx;
	uint8_t ports_to_skip[MAX_PORTS_TO_SKIP];
	bool restore_all_4x = false, restore_all_1x = false;

	out_parms->imp_rc = RIO_SUCCESS;
	out_parms->num_ports = 0;
	out_parms->lrto = 0;

	if ((TSI57X_NUM_PORTS(dev_info) < in_parms->num_ports)
			&& !(RIO_ALL_PORTS == in_parms->num_ports)) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = PC_SET_CONFIG(0x40);
		goto exit;
	}

	rc = tsi57x_init_sw_pmr(dev_info, &sw_pmr);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x41);
		goto exit;
	}

	// Check for incorrect parameters and configuration conflicts
	rc = tsi57x_set_config_init_parms_check_conflict(dev_info, &chg, sw_pmr,
			in_parms, &in_parms_sorted, out_parms);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	// Make sure we don't reconfigure the port that we're connected to.
	// Bad things can happen, like permanent connectivity loss :)
	rc = tsi57x_determine_ports_to_skip(dev_info, in_parms->oob_reg_acc,
			ports_to_skip, &out_parms->imp_rc, sw_pmr,
			CONFIG_PORT_SKIP);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	for (idx = 0; idx < MAX_PORTS_TO_SKIP; idx++) {
		if (RIO_ALL_PORTS != ports_to_skip[idx]) {
			chg.valid[ports_to_skip[idx]] = false;
		}
	}

	// Compute configuration register changes
	rc = tsi57x_compute_port_config_changes(dev_info, &chg, sw_pmr,
			&in_parms_sorted.pc[0], out_parms);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	/* Implement the changes to the Dloop registers */
	for (port_num = 0; port_num < TSI57X_NUM_PORTS(dev_info); port_num +=
			2) {
		uint8_t mac = sw_pmr[port_num].mac_num;
		bool lanes_changed = chg.laneRegsChanged[mac][0]
				|| chg.laneRegsChanged[mac][1]
				|| chg.laneRegsChanged[mac][2]
				|| chg.laneRegsChanged[mac][3];

		if ((chg.dloopCtlOrig[mac] != chg.dloopCtl[mac])
				|| lanes_changed) {
			/* Something changed.  Set up the powerdown mask, and preserve registers as necessary
			 */
			uint32_t powerdown_mask = 0;
			preserved_regs even_port_regs, odd_port_regs;
			uint32_t reg_val = chg.dloopCtlOrig[mac];

			if (((chg.dloopCtlOrig[mac] ^ chg.dloopCtl[mac])
					& ~TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1)
					|| lanes_changed) {
				powerdown_mask =
						TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4
								|
								TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1;

			} else {
				powerdown_mask =
						TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1;
			}
			if (!(chg.dloopCtlOrig[mac]
					& TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4)) {
				restore_all_4x = true;
				rc = tsi57x_preserve_regs_for_powerup(dev_info,
						port_num, &even_port_regs,
						true);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = PC_SET_CONFIG(0x43);
					goto exit;
				}
			}

			if (!(chg.dloopCtlOrig[mac]
					& TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1)) {
				restore_all_1x = true;
				rc = tsi57x_preserve_regs_for_powerup(dev_info,
						port_num + 1, &odd_port_regs,
						true);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = PC_SET_CONFIG(0x44);
					goto exit;
				}
			}

			reg_val |= powerdown_mask;

			/* To change values in the DLOOP clock select register,
			 it is necessary to power down the ports,
			 then change the values, then power the ports up.
			 */
			rc = DARRegWrite(dev_info,
					TSI578_SMACX_DLOOP_CLK_SEL(port_num),
					reg_val);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SET_CONFIG(0x46);
				goto exit;
			}

			reg_val = chg.dloopCtl[mac] | powerdown_mask;

			rc = DARRegWrite(dev_info,
					TSI578_SMACX_DLOOP_CLK_SEL(port_num),
					reg_val);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SET_CONFIG(0x47);
				goto exit;
			}

			rc = DARRegWrite(dev_info,
					TSI578_SMACX_DLOOP_CLK_SEL(port_num),
					chg.dloopCtl[mac]);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SET_CONFIG(0x49);
				goto exit;
			}

			// Only restore registers if the port was and is powered up...
			if (!(chg.dloopCtl[mac]
					& TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4)) {
				rc = tsi57x_restore_regs_from_powerup(dev_info,
						port_num, &even_port_regs,
						restore_all_4x);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = PC_SET_CONFIG(0x4A);
					goto exit;
				}
			}

			// Lane registers can only be written when the MAC is powered up.
			rc = tsi57x_update_lane_inversions(dev_info, port_num, &chg,
					sw_pmr);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_SET_CONFIG(0x48);
				goto exit;
			}

			if (!(chg.dloopCtl[mac]
					& TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1)) {
				rc = tsi57x_restore_regs_from_powerup(dev_info,
						port_num + 1, &odd_port_regs,
						restore_all_1x);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = PC_SET_CONFIG(0x4B);
					goto exit;
				}
			}
		}

		// Set PORT_DIS, PORT_LOCKOUT, and INPUT/OUTPUT_ENABLE as requested.
		for (even_odd = 0; even_odd < 2; even_odd++) {
			uint8_t pnum = port_num + even_odd;
			if (chg.valid[pnum]
					&& (chg.spxCtl[pnum]
							!= chg.spxCtlOrig[pnum])) {
				rc = DARRegWrite(dev_info, TSI578_SPX_CTL(pnum),
						chg.spxCtl[pnum]);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = PC_SET_CONFIG(0x4C);
					goto exit;
				}
			}
		}
	}

	// Initialize input parameters to select ports for updating
	// current configuration.
	if (RIO_ALL_PORTS == in_parms->num_ports) {
		curr_cfg_in.ptl.num_ports = RIO_ALL_PORTS;
	} else {
		curr_cfg_in.ptl.num_ports = 0;
		for (port_num = 0; port_num < TSI57X_NUM_PORTS(dev_info);
				port_num++) {
			if (chg.valid[port_num]) {
				curr_cfg_in.ptl.pnums[curr_cfg_in.ptl.num_ports] =
						port_num;
				curr_cfg_in.ptl.num_ports++;
			}
		}
	}

	// Update link response timeout value as the last item.
	// This is the best place to do it, as it reflects the current
	// power up/power down state of Port 0...
	rc = tsi57x_set_lrto(dev_info, in_parms);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_SET_CONFIG(0x42);
		goto exit;
	}

	rc = tsi57x_rio_pc_get_config(dev_info, &curr_cfg_in, out_parms);

exit:
	return rc;
}

uint32_t tsi57x_rio_pc_get_status(DAR_DEV_INFO_t *dev_info,
		rio_pc_get_status_in_t *in_parms,
		rio_pc_get_status_out_t *out_parms)
{
	uint32_t rc;
	uint8_t port_idx, port_num;
	uint32_t dloop, err_n_stat, spx_ctl;
	port_mac_relations_t *sw_pmr;
	struct DAR_ptl good_ptl;

	out_parms->num_ports = 0;
	out_parms->imp_rc = RIO_SUCCESS;

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_GET_STATUS(1);
		goto exit;
	}

	out_parms->num_ports = good_ptl.num_ports;
	for (port_idx = 0; port_idx < good_ptl.num_ports; port_idx++)
		out_parms->ps[port_idx].pnum = good_ptl.pnums[port_idx];

	rc = tsi57x_init_sw_pmr(dev_info, &sw_pmr);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_GET_STATUS(3);
		goto exit;
	}

	for (port_idx = 0; port_idx < out_parms->num_ports; port_idx++) {
		port_num = out_parms->ps[port_idx].pnum;

		rc = DARRegRead(dev_info,
				TSI578_SMACX_DLOOP_CLK_SEL(
						sw_pmr[port_num].mac_num * 2),
				&dloop);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_GET_STATUS(0x10+port_idx);
			goto exit;
		}

		out_parms->ps[port_idx].first_lane =
				sw_pmr[port_num].first_mac_lane;
		out_parms->ps[port_idx].num_lanes = (uint8_t)(
				(dloop & TSI578_SMACX_DLOOP_CLK_SEL_MAC_MODE) ?
						sw_pmr[port_num].lane_count_1x :
						sw_pmr[port_num].lane_count_4x);

		// If there are lanes assigned to the port, check to see if the power is powered down.
		if (out_parms->ps[port_idx].num_lanes) {
			if (sw_pmr[port_num].mac_num
					!= sw_pmr[port_num].pwr_mac_num) {
				rc =
						DARRegRead(dev_info,
								TSI578_SMACX_DLOOP_CLK_SEL(
										sw_pmr[port_num].pwr_mac_num
												* 2),
								&dloop);
				if (RIO_SUCCESS != rc) {
					out_parms->imp_rc = PC_GET_STATUS(
							0x20+port_idx);
					goto exit;
				}
			}
			if (dloop & sw_pmr[port_num].pwr_down_mask) {
				out_parms->ps[port_idx].num_lanes = 0;
			}
		}

		out_parms->ps[port_idx].pw = rio_pc_pw_last;
		if (!out_parms->ps[port_idx].num_lanes) {
			out_parms->ps[port_idx].port_ok = false;
			out_parms->ps[port_idx].port_error = false;
			out_parms->ps[port_idx].input_stopped = false;
			out_parms->ps[port_idx].output_stopped = false;
			continue;
		}

		// Port is available and powered up, so let's figure out the status...
		rc = DARRegRead(dev_info, TSI578_SPX_ERR_STATUS(port_num),
				&err_n_stat);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_GET_STATUS(0x30+port_idx);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_CTL(port_num), &spx_ctl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_GET_STATUS(0x40+port_idx);
			goto exit;
		}

		out_parms->ps[port_idx].port_ok =
				(err_n_stat & TSI578_SPX_ERR_STATUS_PORT_OK) ?
						true : false;
		out_parms->ps[port_idx].input_stopped =
				(err_n_stat
						& TSI578_SPX_ERR_STATUS_INPUT_ERR_STOP) ?
						true : false;
		out_parms->ps[port_idx].output_stopped =
				(err_n_stat
						& TSI578_SPX_ERR_STATUS_OUTPUT_ERR_STOP) ?
						true : false;

		if (out_parms->ps[port_idx].port_ok) {
			switch (spx_ctl & TSI578_SPX_CTL_INIT_PWIDTH) {
			case RIO_SPX_CTL_PTW_INIT_1X_L0:
				if (1 == out_parms->ps[port_idx].num_lanes) {
					out_parms->ps[port_idx].pw =
							rio_pc_pw_1x;
				} else {
					out_parms->ps[port_idx].pw =
							rio_pc_pw_1x_l0;
				}
				break;
			case RIO_SPX_CTL_PTW_INIT_1X_LR:
				out_parms->ps[port_idx].pw = rio_pc_pw_1x_l2;
				break;
			case RIO_SPX_CTL_PTW_INIT_4X:
				out_parms->ps[port_idx].pw = rio_pc_pw_4x;
				break;
			default:
				out_parms->ps[port_idx].pw = rio_pc_pw_last;
			}
		}

		// Port Error is true if a PORT_ERR is present, OR
		// if a OUTPUT_FAIL is present when STOP_FAIL_EN is set.
		out_parms->ps[port_idx].port_error =
				((err_n_stat & TSI578_SPX_ERR_STATUS_PORT_ERR)
						| ((spx_ctl
								& TSI578_SPX_CTL_STOP_FAIL_EN)
								&& (err_n_stat
										& TSI578_SPX_ERR_STATUS_OUTPUT_FAIL))) ?
						true : false;
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_pc_reset_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_port_in_t *in_parms,
		rio_pc_reset_port_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	preserved_regs saved_regs[2];
	rio_pc_get_config_in_t cfg_in;
	rio_pc_get_config_out_t cfg_out;
	uint32_t dloop, pwrdwn;
	uint8_t port_idx, port_num, dep_port_idx, dep_port_num,
			ports_to_skip[MAX_PORTS_TO_SKIP];
	port_mac_relations_t *sw_pmr;
	bool found;

	out_parms->imp_rc = RIO_SUCCESS;

	if ((RIO_ALL_PORTS != in_parms->port_num)
			&& (in_parms->port_num >= TSI57X_NUM_PORTS(dev_info))) {
		out_parms->imp_rc = PC_RESET_PORT(1);
		goto exit;
	}

	rc = tsi57x_init_sw_pmr(dev_info, &sw_pmr);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_RESET_PORT(2);
		goto exit;
	}

	rc = tsi57x_determine_ports_to_skip(dev_info, in_parms->oob_reg_acc,
			ports_to_skip, &out_parms->imp_rc, sw_pmr,
			RESET_PORT_SKIP);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}
	out_parms->imp_rc = RIO_SUCCESS;

	if (RIO_ALL_PORTS == in_parms->port_num) {
		cfg_in.ptl.num_ports = RIO_ALL_PORTS;
	} else {
		cfg_in.ptl.num_ports = 1;
		cfg_in.ptl.pnums[0] = in_parms->port_num;
		if (sw_pmr[in_parms->port_num].lane_count_4x
				&& !(TSI577_RIO_DEVID_VAL == DEV_CODE(dev_info))) {
			cfg_in.ptl.num_ports = 2;
			cfg_in.ptl.pnums[1] =
					sw_pmr[in_parms->port_num].other_mac_ports[0];
		}
	}

	rc = tsi57x_rio_pc_get_config(dev_info, &cfg_in, &cfg_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = cfg_out.imp_rc;
		goto exit;
	}

	for (port_idx = 0; port_idx < cfg_out.num_ports; port_idx++) {
		port_num = cfg_out.pc[port_idx].pnum;
		dep_port_num = RIO_ALL_PORTS;

		// Do not reset ports required for connectivity.
		// Also skip ports that are not available or powered down.
		if ((port_num == ports_to_skip[0])
				|| (port_num == ports_to_skip[1])
				|| (!cfg_out.pc[port_idx].port_available)
				|| (!cfg_out.pc[port_idx].powered_up))
			continue;

		if (in_parms->preserve_config) {
			rc = tsi57x_preserve_regs_for_powerup(dev_info, port_num,
					&saved_regs[0],
					in_parms->preserve_config);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_RESET_PORT(4);
				goto exit;
			}

			// If this is an even MAC on a non-Tsi577, check the
			// current configuration to see if the odd MAC port is powered up and active.
			// If it is, then we're resetting 2 ports at a time, so save the state.
			if (sw_pmr[port_num].lane_count_4x
					&& !(TSI577_RIO_DEVID_VAL
							== DEV_CODE(dev_info))) {
				dep_port_num = port_num + 1;
				found = false;
				for (dep_port_idx = 0;
						(dep_port_idx
								< cfg_out.num_ports)
								&& !found;
						dep_port_idx++) {
					if (cfg_out.pc[dep_port_idx].pnum
							== dep_port_num) {
						found = true;
						if ((cfg_out.pc[port_idx].port_available)
								&& (cfg_out.pc[port_idx].powered_up)) {
							rc =
									tsi57x_preserve_regs_for_powerup(
											dev_info,
											dep_port_num,
											&saved_regs[1],
											in_parms->preserve_config);
							if (RIO_SUCCESS != rc) {
								goto exit;
							}
						} else {
							dep_port_num =
									RIO_ALL_PORTS;
						}
					}
				}
				if (!found) {
					rc = RIO_ERR_RETURN_NO_RESULT;
					out_parms->imp_rc = PC_RESET_PORT(5);
					goto exit;
				}
			}
		}

		if (in_parms->reset_lp) {
			rc = tsi57x_reset_lp(dev_info, port_num,
					&out_parms->imp_rc);
			if (RIO_SUCCESS != rc) {
				goto exit;
			}

			if (RIO_ALL_PORTS != dep_port_num) {
				rc = tsi57x_reset_lp(dev_info, dep_port_num,
						&out_parms->imp_rc);
				if (RIO_SUCCESS != rc) {
					goto exit;
				}
			}
		}

		/* Now, reset the port.
		 * Blow away the configuration by power cycling the port.
		 */
		rc = DARRegRead(dev_info,
				TSI578_SMACX_DLOOP_CLK_SEL(
						sw_pmr[port_num].pwr_mac_num
								* 2), &dloop);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_PORT(6);
			goto exit;
		}

		pwrdwn = dloop | sw_pmr[port_num].pwr_down_mask;

		rc = DARRegWrite(dev_info,
				TSI578_SMACX_DLOOP_CLK_SEL(
						sw_pmr[port_num].pwr_mac_num
								* 2), pwrdwn);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_PORT(7);
			goto exit;
		}

		rc = DARRegWrite(dev_info,
				TSI578_SMACX_DLOOP_CLK_SEL(
						sw_pmr[port_num].pwr_mac_num
								* 2), dloop);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_PORT(8);
			goto exit;
		}

		/* If we preserved state, restore the state now. */
		rc = tsi57x_restore_regs_from_powerup(dev_info, port_num,
				&saved_regs[0], in_parms->preserve_config);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_RESET_PORT(9);
			goto exit;
		}
		if (RIO_ALL_PORTS != dep_port_num) {
			rc = tsi57x_restore_regs_from_powerup(dev_info, dep_port_num,
					&saved_regs[1],
					in_parms->preserve_config);
			if (RIO_SUCCESS != rc) {
				out_parms->imp_rc = PC_RESET_PORT(0xA);
			}
		}
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_pc_reset_link_partner(DAR_DEV_INFO_t *dev_info,
		rio_pc_reset_link_partner_in_t *in_parms,
		rio_pc_reset_link_partner_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;

	out_parms->imp_rc = RIO_SUCCESS;

	if (TSI57X_NUM_PORTS(dev_info) <= in_parms->port_num) {
		out_parms->imp_rc = PC_RESET_LP(1);
		goto exit;
	}

	rc = tsi57x_reset_lp(dev_info, in_parms->port_num, &out_parms->imp_rc);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	if (in_parms->resync_ackids) {
		rc = DARRegWrite(dev_info,
				TSI578_SPX_ACKID_STAT(in_parms->port_num),
				TSI578_SPX_ACKID_STAT_CLR_PKTS);
		if (RIO_SUCCESS != rc)
			out_parms->imp_rc = PC_RESET_LP(2);
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_pc_clr_errs(DAR_DEV_INFO_t *dev_info,
		rio_pc_clr_errs_in_t *in_parms,
		rio_pc_clr_errs_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t port_idx, lp_port_num;
	CS_field_t magic_cs;
	CS_bytes_t magic_cs_bytes;
	uint32_t cs_reg_val;
	uint32_t dlay;
	uint32_t lr_cmd = STYPE1_LREQ_CMD_PORT_STAT; /* Command for link-request/input-status. */
	uint32_t lresp = 0;
	uint32_t ackid_stat;
	uint32_t err_stat;
	rio_pc_get_status_in_t status_in;
	rio_pc_get_status_out_t status_out;

	out_parms->imp_rc = RIO_SUCCESS;

	if (TSI57X_NUM_PORTS(dev_info) <= in_parms->port_num) {
		out_parms->imp_rc = PC_CLR_ERRS(1);
		goto exit;
	}

	if (in_parms->clr_lp_port_err) {
		if (!in_parms->num_lp_ports
				|| (in_parms->num_lp_ports > TSI578_MAX_PORTS)
				|| (NULL == in_parms->lp_dev_info)) {
			out_parms->imp_rc = PC_CLR_ERRS(2);
			goto exit;
		}
		for (port_idx = 0; port_idx < in_parms->num_lp_ports;
				port_idx++) {
			if (in_parms->lp_port_list[port_idx]
					>= TSI57X_NUM_PORTS(
							in_parms->lp_dev_info)) {
				out_parms->imp_rc = PC_CLR_ERRS(3);
				goto exit;
			}
		}
	}

	// If the port is not PORT_OK, it is not possible to clear error conditions.
	status_in.ptl.num_ports = 1;
	status_in.ptl.pnums[0] = in_parms->port_num;
	rc = tsi57x_rio_pc_get_status(dev_info, &status_in, &status_out);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = status_out.imp_rc;
		goto exit;
	}

	if ((status_out.num_ports != 1) || (!status_out.ps[0].port_ok)) {
		rc = RIO_ERR_ERRS_NOT_CL;
		out_parms->imp_rc = PC_CLR_ERRS(4);
		goto exit;
	}

	/* First, ensure input/output error-stopped conditions are cleared
	 on our port and the link partner by sending the magic control symbol.
	 */

	magic_cs.cs_size = cs_small;
	magic_cs.cs_t0 = stype0_pna;
	magic_cs.parm_0 = 0;
	magic_cs.parm_1 = PNA_GENERAL_ERR;
	magic_cs.cs_t1 = stype1_lreq;
	magic_cs.cs_t1_cmd = 0;

	CS_fields_to_bytes(&magic_cs, &magic_cs_bytes);

	cs_reg_val = ((uint32_t)(magic_cs_bytes.cs_bytes[1]) << 24)
			|| ((uint32_t)(magic_cs_bytes.cs_bytes[2]) << 16)
			|| (((uint32_t)(magic_cs_bytes.cs_bytes[3]) << 8)
					& TSI578_SPX_CS_TX_CMD);

	rc = DARRegWrite(dev_info, TSI578_SPX_CS_TX(in_parms->port_num),
			cs_reg_val);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_CLR_ERRS(0x10);
		goto exit;
	}

	//  Delay while waiting for control symbol magic to complete.
	//  Should be > 5 usec, just to allow some margin for different link speeds
	//  and link partners.

	DAR_WaitSec(5000, 0);

	// Prepare to clear any port-err conditions that may exist on this port.
	//     Send link-request/input-status to learn what link partners
	//     next expected ackID is.
	rc = DARRegWrite(dev_info, TSI578_SPX_LM_REQ(in_parms->port_num),
			lr_cmd);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_CLR_ERRS(0x11);
		goto exit;
	}

	// Poll until we get a response.  Fail if no response is received.

	dlay = 10;
	while (!(lresp & TSI578_SPX_LM_RESP_RESP_VLD) && dlay) {
		dlay--;
		rc = DARRegRead(dev_info,
				TSI578_SPX_LM_RESP(in_parms->port_num), &lresp);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_CLR_ERRS(0x12);
			goto exit;
		}
	}

	if (!(lresp & TSI578_SPX_LM_RESP_RESP_VLD)) {
		rc = RIO_ERR_NOT_EXPECTED_RETURN_VALUE;
		out_parms->imp_rc = PC_CLR_ERRS(0x13);
		goto exit;
	}

	// We have valid ackID information.  Update our local ackID status.
	// The act of updating our local ackID status will clear a local
	// port-err condition.

	rc = DARRegRead(dev_info, TSI578_SPX_ACKID_STAT(in_parms->port_num),
			&ackid_stat);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_CLR_ERRS(0x14);
		goto exit;
	}

	lresp = (lresp & TSI578_SPX_LM_RESP_ACK_ID_STAT) >> 5;
	ackid_stat = ackid_stat & TSI578_SPX_ACKID_STAT_INBOUND;
	ackid_stat = ackid_stat | lresp | (lresp << 8);

	rc = DARRegWrite(dev_info, TSI578_SPX_ACKID_STAT(in_parms->port_num),
			ackid_stat);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_CLR_ERRS(0x15);
		goto exit;
	}

	if (in_parms->clr_lp_port_err) {
		/* Update the link partners ackID status register to match that of
		 the Tsi57x.
		 Increment the expected inbound ackID to reflect the reception of
		 the maintenance write packet.
		 Link partners inbound value should be our outbound value, plus 1.
		 Link partners outbound value should be our inbound value.
		 */
		lresp = (ackid_stat + 1) & TSI578_SPX_ACKID_STAT_OUTBOUND;
		ackid_stat = (ackid_stat & TSI578_SPX_ACKID_STAT_INBOUND) >> 24;
		ackid_stat |= (ackid_stat << 8) | (lresp << 24);

		for (port_idx = 0; port_idx < in_parms->num_lp_ports;
				port_idx++) {
			lp_port_num = in_parms->lp_port_list[port_idx];
			rc =
					DARRegWrite(in_parms->lp_dev_info,
							RIO_SPX_ACKID_ST(
									in_parms->lp_dev_info->extFPtrForPort,
									RIO_EFB_T_FIXME,
									lp_port_num),
							ackid_stat);
			if (RIO_SUCCESS != rc) {
				// The write can fail because the incorrect port was selected.
				//    Call ourselves to clear errors on the local port, and then
				//    try the next link partner port.
				rio_pc_clr_errs_in_t temp_in;
				rio_pc_clr_errs_out_t temp_out;
				uint32_t temp_rc;

				memcpy(&temp_in, in_parms,
						sizeof(rio_pc_clr_errs_in_t));
				temp_in.clr_lp_port_err = false;

				temp_rc = tsi57x_rio_pc_clr_errs(dev_info,
						&temp_in, &temp_out);
				if (RIO_SUCCESS != temp_rc) {
					rc = temp_rc;
					out_parms->imp_rc = PC_CLR_ERRS(0x16);
					goto exit;
				}
			}
		}
	}

	// Lastly, clear physical layer error status indications for the port.
	rc = DARRegRead(dev_info, TSI578_SPX_ERR_STATUS(in_parms->port_num),
			&err_stat);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	rc = DARRegWrite(dev_info, TSI578_SPX_ERR_STATUS(in_parms->port_num),
			err_stat);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	rc = DARRegRead(dev_info, TSI578_SPX_ERR_STATUS(in_parms->port_num),
			&err_stat);
	if (RIO_SUCCESS != rc) {
		goto exit;
	}

	if (err_stat & (TSI578_SPX_ERR_STATUS_PORT_ERR |
	TSI578_SPX_ERR_STATUS_INPUT_ERR_STOP |
	TSI578_SPX_ERR_STATUS_OUTPUT_ERR_STOP |
	TSI578_SPX_ERR_STATUS_OUTPUT_FAIL)) {
		rc = RIO_ERR_ERRS_NOT_CL;
		out_parms->imp_rc = PC_CLR_ERRS(0x20);
		goto exit;
	}

exit:
	return rc;
}

uint32_t tsi57x_rio_pc_secure_port(DAR_DEV_INFO_t *dev_info,
		rio_pc_secure_port_in_t *in_parms,
		rio_pc_secure_port_out_t *out_parms)
{
	uint32_t rc;
	uint8_t pnum;
	uint32_t port_mode = 0;
	uint32_t port_mode_mod, port_mode_mask, port_ctl, glob_int_en, port_idx;
	struct DAR_ptl good_ptl;

	out_parms->imp_rc = RIO_SUCCESS;

	rc = DARrioGetPortList(dev_info, &in_parms->ptl, &good_ptl);
	if ((rc != RIO_SUCCESS) || !good_ptl.num_ports) {
		out_parms->imp_rc = PC_SECURE_PORT(1);
		return rc;
	}

	if (in_parms->rst >= rio_pc_rst_last) {
		rc = RIO_ERR_INVALID_PARAMETER;
		out_parms->imp_rc = PC_SECURE_PORT(2);
		return rc;
	}

	for (port_idx = 0; port_idx < good_ptl.num_ports; port_idx++) {
		port_mode_mask = ~(TSI578_SPX_MODE_MCS_INT_EN
				| TSI578_SPX_MODE_RCS_INT_EN
				| TSI578_SPX_MODE_SELF_RST);
		pnum = good_ptl.pnums[port_idx];

		rc = DARRegRead(dev_info, TSI578_SPX_MODE(pnum), &port_mode);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SECURE_PORT(3);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_CTL(pnum), &port_ctl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SECURE_PORT(4);
			goto exit;
		}

		tsi57x_rst_policy_vals(in_parms->rst, &port_mode_mod,
				&out_parms->rst);

		if (in_parms->MECS_acceptance) {
			port_mode_mod |= TSI578_SPX_MODE_MCS_INT_EN;
		}

		if (in_parms->MECS_participant) {
			port_ctl |= TSI578_SPX_CTL_MCS_EN;
		} else {
			port_ctl &= ~TSI578_SPX_CTL_MCS_EN;
		}

		out_parms->bc_mtc_pkts_allowed = true;
		out_parms->MECS_acceptance = true;
		out_parms->MECS_participant = in_parms->MECS_participant;

		port_mode = (port_mode & port_mode_mask) | port_mode_mod;

		rc = tsi57x_update_reset_policy(dev_info, pnum, port_mode,
				&out_parms->imp_rc);
		if (RIO_SUCCESS != rc) {
			goto exit;
		}

		rc = DARRegWrite(dev_info, TSI578_SPX_CTL(pnum), port_ctl);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SECURE_PORT(6);
		}
	}

	// Program reset interrupt control at the top level
	if ((port_mode & TSI578_SPX_MODE_MCS_INT_EN)
			|| (port_mode & TSI578_SPX_MODE_RCS_INT_EN)) {
		rc = DARRegRead(dev_info, TSI578_GLOB_INT_ENABLE, &glob_int_en);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SECURE_PORT(7);
			goto exit;
		}

		if (port_mode & TSI578_SPX_MODE_RCS_INT_EN) {
			glob_int_en |= TSI578_GLOB_INT_ENABLE_RCS_EN;
		}

		if (port_mode & TSI578_SPX_MODE_MCS_INT_EN) {
			glob_int_en |= TSI578_GLOB_INT_ENABLE_MCS_EN;
		}

		rc = DARRegWrite(dev_info, TSI578_GLOB_INT_ENABLE, glob_int_en);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_SECURE_PORT(8);
		}
	}
exit:
	return rc;
}

uint32_t tsi57x_rio_pc_dev_reset_config(DAR_DEV_INFO_t *dev_info,
		rio_pc_dev_reset_config_in_t *in_parms,
		rio_pc_dev_reset_config_out_t *out_parms)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	uint8_t port_num;
	uint32_t port_mode, port_mode_mask, port_mode_value, glob_int_en;

	out_parms->rst = in_parms->rst;
	out_parms->imp_rc = RIO_SUCCESS;

	if ((uint8_t)(out_parms->rst) >= (uint8_t)(rio_pc_rst_last)) {
		out_parms->imp_rc = PC_DEV_RESET_CONFIG(1);
		goto exit;
	}

	port_mode_mask = ~(TSI578_SPX_MODE_SELF_RST |
	TSI578_SPX_MODE_RCS_INT_EN);

	tsi57x_rst_policy_vals(in_parms->rst, &port_mode_value,
			&out_parms->rst);

	// Configure reset interrupt and reset response in SPx_MODE registers
	for (port_num = 0; port_num < TSI57X_NUM_PORTS(dev_info); port_num++) {
		rc = DARRegRead(dev_info, TSI578_SPX_MODE(port_num),
				&port_mode);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_DEV_RESET_CONFIG(2);
			goto exit;
		}

		port_mode = (port_mode & port_mode_mask) | port_mode_value;

		rc = tsi57x_update_reset_policy(dev_info, port_num, port_mode,
				&out_parms->imp_rc);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_DEV_RESET_CONFIG(3);
			goto exit;
		}

		rc = DARRegRead(dev_info, TSI578_SPX_MODE(port_num),
				&port_mode);
		if (RIO_SUCCESS != rc) {
			out_parms->imp_rc = PC_DEV_RESET_CONFIG(2);
			goto exit;
		}
	}

	// Program reset interrupt control at the top level
	rc = DARRegRead(dev_info, TSI578_GLOB_INT_ENABLE, &glob_int_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_DEV_RESET_CONFIG(4);
		goto exit;
	}

	if (rio_pc_rst_int == out_parms->rst) {
		glob_int_en |= TSI578_GLOB_INT_ENABLE_RCS_EN;
	} else {
		glob_int_en &= ~TSI578_GLOB_INT_ENABLE_RCS_EN;
	}

	rc = DARRegWrite(dev_info, TSI578_GLOB_INT_ENABLE, glob_int_en);
	if (RIO_SUCCESS != rc) {
		out_parms->imp_rc = PC_DEV_RESET_CONFIG(5);
	}

exit:
	return rc;
}

#endif /* TSI57X_DAR_WANTED */

#ifdef __cplusplus
}
#endif
