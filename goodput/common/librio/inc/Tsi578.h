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

#ifndef __TSI578_H__
#define __TSI578_H__

#define TSI578_MAX_MAC         8
#define TSI578_MAX_PORTS      16
#define TSI578_MAX_PORT_LANES  4
#define TSI578_MAX_MC_MASKS    8


/* ************************************************ */
/* Tsi578 : Register address offset definitions     */
/* ************************************************ */
#define TSI578_RIO_DEV_ID                                ((uint32_t)0x00000000)
#define TSI578_RIO_DEV_INFO                              ((uint32_t)0x00000004)
#define TSI578_RIO_ASBLY_ID                              ((uint32_t)0x00000008)
#define TSI578_RIO_ASBLY_INFO                            ((uint32_t)0x0000000c)
#define TSI578_RIO_PE_FEAT                               ((uint32_t)0x00000010)
#define TSI578_RIO_SW_PORT                               ((uint32_t)0x00000014)
#define TSI578_RIO_SRC_OP                                ((uint32_t)0x00000018)
#define TSI578_RIO_PE_MC_FEAT                            ((uint32_t)0x00000030)
#define TSI578_RIO_LUT_SIZE                              ((uint32_t)0x00000034)
#define TSI578_RIO_SW_MC_INFO                            ((uint32_t)0x00000038)
#define TSI578_RIO_HOST_BASE_ID_LOCK                     ((uint32_t)0x00000068)
#define TSI578_RIO_COMP_TAG                              ((uint32_t)0x0000006c)
#define TSI578_RIO_ROUTE_CFG_DESTID                      ((uint32_t)0x00000070)
#define TSI578_RIO_ROUTE_CFG_PORT                        ((uint32_t)0x00000074)
#define TSI578_RIO_LUT_ATTR                              ((uint32_t)0x00000078)
#define TSI578_RIO_MC_MASK_CFG                           ((uint32_t)0x00000080)
#define TSI578_RIO_MC_DESTID_CFG                         ((uint32_t)0x00000084)
#define TSI578_RIO_MC_DESTID_ASSOC                       ((uint32_t)0x00000088)
#define TSI578_RIO_SW_MB_HEAD                            ((uint32_t)0x00000100)
#define TSI578_RIO_SW_LT_CTL                             ((uint32_t)0x00000120)
#define TSI578_RIO_SW_GEN_CTL                            ((uint32_t)0x0000013c)
#define TSI578_SPX_LM_REQ(X)                   ((uint32_t)(0x0140 + 0x020*(X)))
#define TSI578_SPX_LM_RESP(X)                  ((uint32_t)(0x0144 + 0x020*(X)))
#define TSI578_SPX_ACKID_STAT(X)               ((uint32_t)(0x0148 + 0x020*(X)))
#define TSI578_SPX_ERR_STATUS(X)               ((uint32_t)(0x0158 + 0x020*(X)))
#define TSI578_SPX_CTL(X)                      ((uint32_t)(0x015c + 0x020*(X)))
#define TSI578_RIO_ERR_RPT_BH                            ((uint32_t)0x00001000)
#define TSI578_RIO_LOG_ERR_DET                           ((uint32_t)0x00001008)
#define TSI578_RIO_LOG_ERR_DET_EN                        ((uint32_t)0x0000100c)
#define TSI578_RIO_LOG_ERR_ADDR                          ((uint32_t)0x00001014)
#define TSI578_RIO_LOG_ERR_DEVID                         ((uint32_t)0x00001018)
#define TSI578_RIO_LOG_ERR_CTRL_INFO                     ((uint32_t)0x0000101c)
#define TSI578_RIO_PW_DESTID                             ((uint32_t)0x00001028)
#define TSI578_RIO_PKT_TTL                               ((uint32_t)0x0000102c)
#define TSI578_SPX_ERR_DET(X)                  ((uint32_t)(0x1040 + 0x040*(X)))
#define TSI578_SPX_RATE_EN(X)                  ((uint32_t)(0x1044 + 0x040*(X)))
#define TSI578_SPX_ERR_ATTR_CAPT_DBG0(X)       ((uint32_t)(0x1048 + 0x040*(X)))
#define TSI578_SPX_ERR_CAPT_0_DBG1(X)          ((uint32_t)(0x104c + 0x040*(X)))
#define TSI578_SPX_ERR_CAPT_1_DBG2(X)          ((uint32_t)(0x1050 + 0x040*(X)))
#define TSI578_SPX_ERR_CAPT_2_DBG3(X)          ((uint32_t)(0x1054 + 0x040*(X)))
#define TSI578_SPX_ERR_CAPT_3_DBG4(X)          ((uint32_t)(0x1058 + 0x040*(X)))
#define TSI578_SPX_ERR_RATE(X)                 ((uint32_t)(0x1068 + 0x040*(X)))
#define TSI578_SPX_ERR_THRESH(X)               ((uint32_t)(0x106c + 0x040*(X)))
#define TSI578_SPBC_DISCOVERY_TIMER                      ((uint32_t)0x00010000)
#define TSI578_SPX_DISCOVERY_TIMER(X)         ((uint32_t)(0x11000 + 0x100*(X)))
#define TSI578_SPBC_MODE                                 ((uint32_t)0x00010004)
#define TSI578_SPX_MODE(X)                    ((uint32_t)(0x11004 + 0x100*(X)))
#define TSI578_SPBC_CS_INT_STATUS                        ((uint32_t)0x00010008)
#define TSI578_SPX_CS_INT_STATUS(X)           ((uint32_t)(0x11008 + 0x100*(X)))
#define TSI578_SPBC_RIO_WM                               ((uint32_t)0x0001000c)
#define TSI578_SPX_RIO_WM(X)                  ((uint32_t)(0x1100c + 0x100*(X)))
#define TSI578_SPBC_ROUTE_CFG_DESTID                     ((uint32_t)0x00010070)
#define TSI578_SPX_ROUTE_CFG_DESTID(X)        ((uint32_t)(0x11070 + 0x100*(X)))
#define TSI578_SPBC_ROUTE_CFG_PORT                       ((uint32_t)0x00010074)
#define TSI578_SPX_ROUTE_CFG_PORT(X)          ((uint32_t)(0x11074 + 0x100*(X)))
#define TSI578_SPBC_ROUTE_BASE                           ((uint32_t)0x00010078)
#define TSI578_SPX_ROUTE_BASE(X)              ((uint32_t)(0x11078 + 0x100*(X)))
#define TSI578_RIO_MC_IDX(X)                  ((uint32_t)(0x10300 + 0x004*(X)))
#define TSI578_RIO_MC_MSKX(X)                 ((uint32_t)(0x10320 + 0x004*(X)))
#define TSI578_SPX_ID(X)                      ((uint32_t)(0x13000 + 0x100*(X)))
#define TSI578_SPX_CTL_INDEP(X)               ((uint32_t)(0x13004 + 0x100*(X)))
#define TSI578_SPX_SILENCE_TIMER(X)           ((uint32_t)(0x13008 + 0x100*(X)))
#define TSI578_SPX_SEND_MCS(X)                ((uint32_t)(0x1300c + 0x100*(X)))
#define TSI578_SPX_LUT_PAR_ERR_INFO(X)        ((uint32_t)(0x13010 + 0x100*(X)))
#define TSI578_SPX_CS_TX(X)                   ((uint32_t)(0x13014 + 0x100*(X)))
#define TSI578_SPX_INT_STATUS(X)              ((uint32_t)(0x13018 + 0x100*(X)))
#define TSI578_SPX_INT_GEN(X)                 ((uint32_t)(0x1301c + 0x100*(X)))
#define TSI578_SPX_PSC0N1_CTRL(X)             ((uint32_t)(0x13020 + 0x100*(X)))
#define TSI578_SPX_PSC2N3_CTRL(X)             ((uint32_t)(0x13024 + 0x100*(X)))
#define TSI578_SPX_PSC4N5_CTRL(X)             ((uint32_t)(0x13028 + 0x100*(X)))
#define TSI578_SPX_PSC_CTRL(X,Y)    ((uint32_t)(0x13020 + 0x100*(X)+((Y/2)*4)))
#define TSI578_NUM_PERF_CTRS                                                  6
#define TSI578_SPX_PSC0(X)                    ((uint32_t)(0x13040 + 0x100*(X)))
#define TSI578_SPX_PSC1(X)                    ((uint32_t)(0x13044 + 0x100*(X)))
#define TSI578_SPX_PSC2(X)                    ((uint32_t)(0x13048 + 0x100*(X)))
#define TSI578_SPX_PSC3(X)                    ((uint32_t)(0x1304c + 0x100*(X)))
#define TSI578_SPX_PSC4(X)                    ((uint32_t)(0x13050 + 0x100*(X)))
#define TSI578_SPX_PSC5(X)                    ((uint32_t)(0x13054 + 0x100*(X)))
#define TSI578_SPX_PSCY(X,Y)           ((uint32_t)(0x13040 +(0x100*(X))+(4*Y)))
#define TSI578_SPX_TX_Q_D_THRESH(X)           ((uint32_t)(0x13080 + 0x100*(X)))
#define TSI578_SPX_TX_Q_STATUS(X)             ((uint32_t)(0x13084 + 0x100*(X)))
#define TSI578_SPX_TX_Q_PERIOD(X)             ((uint32_t)(0x13088 + 0x100*(X)))
#define TSI578_SPX_RX_Q_D_THRESH(X)           ((uint32_t)(0x13090 + 0x100*(X)))
#define TSI578_SPX_RX_Q_STATUS(X)             ((uint32_t)(0x13094 + 0x100*(X)))
#define TSI578_SPX_RX_Q_PERIOD(X)             ((uint32_t)(0x13098 + 0x100*(X)))
#define TSI578_SPX_REORDER_CTR(X)             ((uint32_t)(0x130a0 + 0x100*(X)))
#define TSI578_SMACX_CFG_CH0(X)               ((uint32_t)(0x130b0 + 0x100*(X)))
#define TSI578_SMACX_CFG_CH1(X)               ((uint32_t)(0x130b4 + 0x100*(X)))
#define TSI578_SMACX_CFG_CH2(X)               ((uint32_t)(0x130b8 + 0x100*(X)))
#define TSI578_SMACX_CFG_CH3(X)               ((uint32_t)(0x130bc + 0x100*(X)))
#define TSI578_SMACX_CFG_GBL(X)               ((uint32_t)(0x130c0 + 0x100*(X)))
#define TSI578_SMACX_CFG_GBLB(X)              ((uint32_t)(0x130c4 + 0x100*(X)))
#define TSI578_SMACX_DLOOP_CLK_SEL(X)         ((uint32_t)(0x130c8 + 0x100*(X)))
#define TSI578_SMACX_SERDES_OUTPUT_PINS(X)    ((uint32_t)(0x130cc + 0x100*(X)))
#define TSI578_MCES_PIN_CTRL                             ((uint32_t)0x000130d0)
#define TSI578_SMACX_SPY_BUS_CFG(X)           ((uint32_t)(0x130fc + 0x100*(X)))
#define TSI578_ERRX_INJ_CMP_MASK(X)           ((uint32_t)(0x14004 + 0x100*(X)))
#define TSI578_ERRX_INJ_CMP_BIT_VLD(X)        ((uint32_t)(0x14008 + 0x100*(X)))
#define TSI578_ERRX_INJ(X)                    ((uint32_t)(0x1400c + 0x100*(X)))
#define TSI578_ERRX_INJ_BIT_VLD(X)            ((uint32_t)(0x14010 + 0x100*(X)))
#define TSI578_FAB_CTL                                   ((uint32_t)0x0001aa00)
#define TSI578_FAB_INT_STAT                              ((uint32_t)0x0001aa04)
#define TSI578_RIO_MC_LAT_ERR                            ((uint32_t)0x0001aa08)
#define TSI578_RIO_MC_LAT_ERR_SET                        ((uint32_t)0x0001aa0c)
#define TSI578_PKTGEN_CNTRL                              ((uint32_t)0x0001aaf0)
#define TSI578_WRKQ_DATA                                 ((uint32_t)0x0001aaf4)
#define TSI578_LST_DTM_STATUS                            ((uint32_t)0x0001aaf8)
#define TSI578_TX_ENBL                                   ((uint32_t)0x0001aafc)
#define TSI578_GLOB_INT_STATUS                           ((uint32_t)0x0001ac00)
#define TSI578_GLOB_INT_ENABLE                           ((uint32_t)0x0001ac04)
#define TSI578_GLOB_DEV_ID_SEL                           ((uint32_t)0x0001ac08)
#define TSI578_GLOB_SPY_BUS_CFG                          ((uint32_t)0x0001ac0c)
#define TSI578_RIO_PW_TIMEOUT                            ((uint32_t)0x0001ac14)
#define TSI578_RIO_PW_OREQ_STATUS                        ((uint32_t)0x0001ac18)
#define TSI578_RIOX_MC_REG_VER(X)             ((uint32_t)(0x1b000 + 0x100*(X)))
#define TSI578_RIOX_MC_LAT_LIMIT(X)           ((uint32_t)(0x1b004 + 0x100*(X)))
#define TSI578_SPX_ISF_WM(X)                  ((uint32_t)(0x1b008 + 0x100*(X)))
#define TSI578_SPX_WRR_0(X)                   ((uint32_t)(0x1b010 + 0x100*(X)))
#define TSI578_SPX_WRR_1(X)                   ((uint32_t)(0x1b014 + 0x100*(X)))
#define TSI578_SPX_WRR_2(X)                   ((uint32_t)(0x1b018 + 0x100*(X)))
#define TSI578_SPX_WRR_3(X)                   ((uint32_t)(0x1b01c + 0x100*(X)))
#define TSI578_I2C_DEVID                                 ((uint32_t)0x0001d100)
#define TSI578_I2C_RESET                                 ((uint32_t)0x0001d104)
#define TSI578_I2C_MST_CFG                               ((uint32_t)0x0001d108)
#define TSI578_I2C_MST_CNTRL                             ((uint32_t)0x0001d10c)
#define TSI578_I2C_MST_RDATA                             ((uint32_t)0x0001d110)
#define TSI578_I2C_MST_TDATA                             ((uint32_t)0x0001d114)
#define TSI578_I2C_ACC_STAT                              ((uint32_t)0x0001d118)
#define TSI578_I2C_INT_STAT                              ((uint32_t)0x0001d11c)
#define TSI578_I2C_INT_ENABLE                            ((uint32_t)0x0001d120)
#define TSI578_I2C_INT_SET                               ((uint32_t)0x0001d124)
#define TSI578_I2C_SLV_CFG                               ((uint32_t)0x0001d12c)
#define TSI578_I2C_BOOT_CNTRL                            ((uint32_t)0x0001d140)
#define TSI578_EXI2C_REG_WADDR                           ((uint32_t)0x0001d200)
#define TSI578_EXI2C_REG_WDATA                           ((uint32_t)0x0001d204)
#define TSI578_EXI2C_REG_RADDR                           ((uint32_t)0x0001d210)
#define TSI578_EXI2C_REG_RDATA                           ((uint32_t)0x0001d214)
#define TSI578_EXI2C_ACC_STAT                            ((uint32_t)0x0001d220)
#define TSI578_EXI2C_ACC_CNTRL                           ((uint32_t)0x0001d224)
#define TSI578_EXI2C_STAT                                ((uint32_t)0x0001d280)
#define TSI578_EXI2C_STAT_ENABLE                         ((uint32_t)0x0001d284)
#define TSI578_EXI2C_MBOX_OUT                            ((uint32_t)0x0001d290)
#define TSI578_EXI2C_MBOX_IN                             ((uint32_t)0x0001d294)
#define TSI578_I2C_EVENT                                 ((uint32_t)0x0001d300)
#define TSI578_I2C_SNAP_EVENT                            ((uint32_t)0x0001d304)
#define TSI578_I2C_NEW_EVENT                             ((uint32_t)0x0001d308)
#define TSI578_I2C_EVENT_ENB                             ((uint32_t)0x0001d30c)
#define TSI578_I2C_DIVIDER                               ((uint32_t)0x0001d320)
#define TSI578_I2C_FILTER_SCL_CFG                        ((uint32_t)0x0001d328)
#define TSI578_I2C_FILTER_SDA_CFG                        ((uint32_t)0x0001d32c)
#define TSI578_I2C_START_SETUP_HOLD                      ((uint32_t)0x0001d340)
#define TSI578_I2C_STOP_IDLE                             ((uint32_t)0x0001d344)
#define TSI578_I2C_SDA_SETUP_HOLD                        ((uint32_t)0x0001d348)
#define TSI578_I2C_SCL_PERIOD                            ((uint32_t)0x0001d34c)
#define TSI578_I2C_SCL_MIN_PERIOD                        ((uint32_t)0x0001d350)
#define TSI578_I2C_SCL_ARB_TIMEOUT                       ((uint32_t)0x0001d354)
#define TSI578_I2C_BYTE_TRAN_TIMEOUT                     ((uint32_t)0x0001d358)
#define TSI578_I2C_BOOT_DIAG_TIMER                       ((uint32_t)0x0001d35c)
#define TSI578_I2C_DIAG_FILTER_SCL                       ((uint32_t)0x0001d3a0)
#define TSI578_I2C_DIAG_FILTER_SDA                       ((uint32_t)0x0001d3a4)
#define TSI578_I2C_BOOT_DIAG_PROGRESS                    ((uint32_t)0x0001d3b8)
#define TSI578_I2C_BOOT_DIAG_CFG                         ((uint32_t)0x0001d3bc)
#define TSI578_I2C_DIAG_CNTRL                            ((uint32_t)0x0001d3c0)
#define TSI578_I2C_DIAG_STAT                             ((uint32_t)0x0001d3d0)
#define TSI578_I2C_DIAG_HIST                             ((uint32_t)0x0001d3d4)
#define TSI578_I2C_DIAG_MST_FSM                          ((uint32_t)0x0001d3d8)
#define TSI578_I2C_DIAG_SLV_FSM                          ((uint32_t)0x0001d3dc)
#define TSI578_I2C_DIAG_MST_SDA_SCL                      ((uint32_t)0x0001d3e0)
#define TSI578_I2C_DIAG_MST_SCL_PER                      ((uint32_t)0x0001d3e4)
#define TSI578_I2C_DIAG_MST_ARB_BOOT                     ((uint32_t)0x0001d3e8)
#define TSI578_I2C_DIAG_MST_BYTE_TRAN                    ((uint32_t)0x0001d3ec)
#define TSI578_I2C_DIAG_SLV_SDA_SCL                      ((uint32_t)0x0001d3f0)
#define TSI578_I2C_DIAG_SLV_BYTE_TRAN                    ((uint32_t)0x0001d3f4)
#define TSI578_SMACX_TX_CTL_STAT_0(X)         ((uint32_t)(0x1e000 + 0x100*(X)))
#define TSI578_SMACX_RX_STAT_0(X)             ((uint32_t)(0x1e004 + 0x100*(X)))
#define TSI578_SMACX_TX_RX_CTL_STAT_0(X)      ((uint32_t)(0x1e008 + 0x100*(X)))
#define TSI578_SMACX_RX_CTL_STAT_0(X)         ((uint32_t)(0x1e00c + 0x100*(X)))
#define TSI578_SMACX_PG_CTL_0(X)              ((uint32_t)(0x1e020 + 0x100*(X)))
#define TSI578_SMACX_PM_CTL_0(X)              ((uint32_t)(0x1e030 + 0x100*(X)))
#define TSI578_SMACX_FP_VAL_0(X)              ((uint32_t)(0x1e034 + 0x100*(X)))
#define TSI578_SMACX_SCP_RX_CTL_0(X)          ((uint32_t)(0x1e038 + 0x100*(X)))
#define TSI578_SMACX_RX_DBG_0(X)              ((uint32_t)(0x1e03c + 0x100*(X)))
#define TSI578_SMACX_RESET(X)                 ((uint32_t)(0x1e100 + 0x100*(X)))
#define TSI578_SMACX_CMP_A(X)                 ((uint32_t)(0x1e104 + 0x100*(X)))
#define TSI578_SMACX_CMP_B(X)                 ((uint32_t)(0x1e108 + 0x100*(X)))
#define TSI578_SMACX_SCP(X)                   ((uint32_t)(0x1e10c + 0x100*(X)))
#define TSI578_SMACX_DAC_RTUNE(X)             ((uint32_t)(0x1e110 + 0x100*(X)))
#define TSI578_SMACX_ADC(X)                   ((uint32_t)(0x1e114 + 0x100*(X)))
#define TSI578_SMACX_ID(X)                    ((uint32_t)(0x1e118 + 0x100*(X)))
#define TSI578_SMACX_CTL_VAL(X)               ((uint32_t)(0x1e11c + 0x100*(X)))
#define TSI578_SMACX_LVL_CTL_VAL(X)           ((uint32_t)(0x1e120 + 0x100*(X)))
#define TSI578_SMACX_CTL_OVR(X)               ((uint32_t)(0x1e124 + 0x100*(X)))
#define TSI578_SMACX_LVL_OVR(X)               ((uint32_t)(0x1e128 + 0x100*(X)))
#define TSI578_SMACX_MPLL_CTL(X)              ((uint32_t)(0x1e12c + 0x100*(X)))



/* ************************************************ */
/* Tsi578 : Register Bit Masks and Reset Values     */
/*           definitions for every register         */
/* ************************************************ */



/* TSI578_RIO_DEV_ID : Register Bits Masks Definitions */
#define TSI578_RIO_DEV_IDENT_VEND                        ((uint32_t)0x0000ffff)
#define TSI578_RIO_DEV_IDENT_DEVI                        ((uint32_t)0xffff0000)

#define TSI578_RIO_DEVID_VAL 0x0578
#define TSI577_RIO_DEVID_VAL 0x0577
#define TSI576_RIO_DEVID_VAL 0x0576
#define TSI574_RIO_DEVID_VAL 0x0574
#define TSI572_RIO_DEVID_VAL 0x0572

/* TSI578_RIO_DEV_INFO : Register Bits Masks Definitions */
#define TSI578_RIO_DEV_INFO_METAL_REV                    ((uint32_t)0x0000000f)
#define TSI578_RIO_DEV_INFO_SILICON_REV                  ((uint32_t)0x000000f0)

/* TSI578_RIO_ASBLY_ID : Register Bits Masks Definitions */
#define TSI578_RIO_ASBLY_ID_ASBLY_VEN_ID                 ((uint32_t)0x0000ffff)
#define TSI578_RIO_ASBLY_ID_ASBLY_ID                     ((uint32_t)0xffff0000)

/* TSI578_RIO_ASBLY_INFO : Register Bits Masks Definitions */
#define TSI578_RIO_ASSY_INF_EFB_PTR                      ((uint32_t)0x0000ffff)
#define TSI578_RIO_ASSY_INF_ASSY_REV                     ((uint32_t)0xffff0000)

/* TSI578_RIO_PE_FEAT : Register Bits Masks Definitions */
#define TSI578_RIO_PE_FEAT_EXT_AS                        ((uint32_t)0x00000007)
#define TSI578_RIO_PE_FEAT_EXT_FEA                       ((uint32_t)0x00000008)
#define TSI578_RIO_PE_FEAT_CTLS                          ((uint32_t)0x00000010)
#define TSI578_RIO_PE_FEAT_SBR                           ((uint32_t)0x00000100)
#define TSI578_RIO_PE_FEAT_MC                            ((uint32_t)0x00000400)
#define TSI578_RIO_PE_FEAT_SW                            ((uint32_t)0x10000000)
#define TSI578_RIO_PE_FEAT_PROC                          ((uint32_t)0x20000000)
#define TSI578_RIO_PE_FEAT_MEM                           ((uint32_t)0x40000000)
#define TSI578_RIO_PE_FEAT_BRDG                          ((uint32_t)0x80000000)

/* TSI578_RIO_SW_PORT : Register Bits Masks Definitions */
#define TSI578_RIO_SW_PORT_PORT_NUM                      ((uint32_t)0x000000ff)
#define TSI578_RIO_SW_PORT_PORT_TOTAL                    ((uint32_t)0x0000ff00)

/* TSI578_RIO_SRC_OP : Register Bits Masks Definitions */
#define TSI578_RIO_SRC_OP_PORT_WR                        ((uint32_t)0x00000004)
#define TSI578_RIO_SRC_OP_A_CLEAR                        ((uint32_t)0x00000010)
#define TSI578_RIO_SRC_OP_A_SET                          ((uint32_t)0x00000020)
#define TSI578_RIO_SRC_OP_A_DEC                          ((uint32_t)0x00000040)
#define TSI578_RIO_SRC_OP_A_INC                          ((uint32_t)0x00000080)
#define TSI578_RIO_SRC_OP_A_TSWAP                        ((uint32_t)0x00000100)
#define TSI578_RIO_SRC_OP_DBELL                          ((uint32_t)0x00000400)
#define TSI578_RIO_SRC_OP_D_MSG                          ((uint32_t)0x00000800)
#define TSI578_RIO_SRC_OP_WR_RES                         ((uint32_t)0x00001000)
#define TSI578_RIO_SRC_OP_STRM_WR                        ((uint32_t)0x00002000)
#define TSI578_RIO_SRC_OP_WRITE                          ((uint32_t)0x00004000)
#define TSI578_RIO_SRC_OP_READ                           ((uint32_t)0x00008000)
#define TSI578_RIO_SRC_OP_IMPLEMENT_DEF                  ((uint32_t)0x00030000)

/* TSI578_RIO_PE_MC_FEAT : Register Bits Masks Definitions */
#define TSI578_RIO_PE_MC_FEAT_SIMP                       ((uint32_t)0x80000000)

/* TSI578_RIO_LUT_SIZE : Register Bits Masks Definitions */
#define TSI578_RIO_LUT_SIZE_LUT_SIZE                     ((uint32_t)0x0000ffff)

/* TSI578_RIO_SW_MC_INFO : Register Bits Masks Definitions */
#define TSI578_RIO_SW_MC_INFO_MAX_MASKS                  ((uint32_t)0x0000ffff)
#define TSI578_RIO_SW_MC_INFO_MAX_DESTID_ASSOC           ((uint32_t)0x3fff0000)
#define TSI578_RIO_SW_MC_INFO_ASSOC_SCOPE                ((uint32_t)0x40000000)
#define TSI578_RIO_SW_MC_INFO_ASSOC_MODE                 ((uint32_t)0x80000000)

/* TSI578_RIO_HOST_BASE_ID_LOCK : Register Bits Masks Definitions */
#define TSI578_RIO_HOST_BASE_ID_LOCK_HOST_BASE_ID        ((uint32_t)0x0000ffff)

/* TSI578_RIO_COMP_TAG : Register Bits Masks Definitions */
#define TSI578_RIO_COMP_TAG_CTAG                         ((uint32_t)0xffffffff)

/* TSI578_RIO_ROUTE_CFG_DESTID : Register Bits Masks Definitions */
#define TSI578_RIO_ROUTE_CFG_DESTID_CFG_DEST_ID          ((uint32_t)0x000000ff)
#define TSI578_RIO_ROUTE_CFG_DESTID_LRG_CFG_DEST_ID      ((uint32_t)0x0000ff00)

/* TSI578_RIO_ROUTE_CFG_PORT : Register Bits Masks Definitions */
#define TSI578_RIO_ROUTE_CFG_PORT_PORT                   ((uint32_t)0x000000ff)

/* TSI578_RIO_LUT_ATTR : Register Bits Masks Definitions */
#define TSI578_RIO_LUT_ATTR_DEFAULT_PORT                 ((uint32_t)0x000000ff)

/* TSI578_RIO_MC_MASK_CFG : Register Bits Masks Definitions */
#define TSI578_RIO_MC_MASK_CFG_PORT_PRESENT              ((uint32_t)0x00000001)
#define TSI578_RIO_MC_MASK_CFG_MASK_CMD                  ((uint32_t)0x00000070)
#define TSI578_RIO_MC_MASK_CFG_EG_PORT_NUM               ((uint32_t)0x00000f00)
#define TSI578_RIO_MC_MASK_CFG_MC_MASK_NUM               ((uint32_t)0x00070000)

/* TSI578_RIO_MC_DESTID_CFG : Register Bits Masks Definitions */
#define TSI578_RIO_MC_DESTID_CFG_MASK_NUM_BASE           ((uint32_t)0x00000007)
#define TSI578_RIO_MC_DESTID_CFG_DESTID_BASE             ((uint32_t)0x00ff0000)
#define TSI578_RIO_MC_DESTID_CFG_DESTID_BASE_LT          ((uint32_t)0xff000000)

/* TSI578_RIO_MC_DESTID_ASSOC : Register Bits Masks Definitions */
#define TSI578_RIO_MC_DESTID_ASSOC_ASSOC_PRESENT         ((uint32_t)0x00000001)
#define TSI578_RIO_MC_DESTID_ASSOC_CMD                   ((uint32_t)0x00000060)
#define TSI578_RIO_MC_DESTID_ASSOC_LARGE                 ((uint32_t)0x00000080)
#define TSI578_RIO_MC_DESTID_ASSOC_INGRESS_PORT          ((uint32_t)0x0000ff00)
#define TSI578_RIO_MC_DESTID_ASSOC_ASSOC_BLK_SIZE        ((uint32_t)0xffff0000)

/* TSI578_RIO_SW_MB_HEAD : Register Bits Masks Definitions */
#define TSI578_RIO_SW_MB_HEAD_EF_ID                      ((uint32_t)0x0000ffff)
#define TSI578_RIO_SW_MB_HEAD_EF_PTR                     ((uint32_t)0xffff0000)

/* TSI578_RIO_SW_LT_CTL : Register Bits Masks Definitions */
#define TSI578_RIO_SW_LT_CTL_TVAL                        ((uint32_t)0xffffff00)

/* TSI578_RIO_SW_GEN_CTL : Register Bits Masks Definitions */
#define TSI578_RIO_SW_GEN_CTL_DISC                       ((uint32_t)0x20000000)

/* TSI578_SPX_LM_REQ : Register Bits Masks Definitions */
#define TSI578_SPX_LM_REQ_CMD                            ((uint32_t)0x00000007)

/* TSI578_SPX_LM_RESP : Register Bits Masks Definitions */
#define TSI578_SPX_LM_RESP_LINK_STAT                     ((uint32_t)0x0000001f)
#define TSI578_SPX_LM_RESP_ACK_ID_STAT                   ((uint32_t)0x000003e0)
#define TSI578_SPX_LM_RESP_RESP_VLD                      ((uint32_t)0x80000000)

/* TSI578_SPX_ACKID_STAT : Register Bits Masks Definitions */
#define TSI578_SPX_ACKID_STAT_OUTBOUND                   ((uint32_t)0x0000001f)
#define TSI578_SPX_ACKID_STAT_OUTSTANDING                ((uint32_t)0x00001f00)
#define TSI578_SPX_ACKID_STAT_INBOUND                    ((uint32_t)0x1f000000)
#define TSI578_SPX_ACKID_STAT_CLR_PKTS                   ((uint32_t)0x80000000)

/* TSI578_SPX_ERR_STATUS : Register Bits Masks Definitions */
#define TSI578_SPX_ERR_STATUS_PORT_UNINIT                ((uint32_t)0x00000001)
#define TSI578_SPX_ERR_STATUS_PORT_OK                    ((uint32_t)0x00000002)
#define TSI578_SPX_ERR_STATUS_PORT_ERR                   ((uint32_t)0x00000004)
#define TSI578_SPX_ERR_STATUS_PORT_W_PEND                ((uint32_t)0x00000010)
#define TSI578_SPX_ERR_STATUS_INPUT_ERR_STOP             ((uint32_t)0x00000100)
#define TSI578_SPX_ERR_STATUS_INPUT_ERR                  ((uint32_t)0x00000200)
#define TSI578_SPX_ERR_STATUS_INPUT_RS                   ((uint32_t)0x00000400)
#define TSI578_SPX_ERR_STATUS_OUTPUT_ERR_STOP            ((uint32_t)0x00010000)
#define TSI578_SPX_ERR_STATUS_OUTPUT_ERR                 ((uint32_t)0x00020000)
#define TSI578_SPX_ERR_STATUS_OUTPUT_RS                  ((uint32_t)0x00040000)
#define TSI578_SPX_ERR_STATUS_OUTPUT_R                   ((uint32_t)0x00080000)
#define TSI578_SPX_ERR_STATUS_OUTPUT_RE                  ((uint32_t)0x00100000)
#define TSI578_SPX_ERR_STATUS_OUTPUT_DEG                 ((uint32_t)0x01000000)
#define TSI578_SPX_ERR_STATUS_OUTPUT_FAIL                ((uint32_t)0x02000000)
#define TSI578_SPX_ERR_STATUS_OUTPUT_DROP                ((uint32_t)0x04000000)

/* TSI578_SPX_CTL : Register Bits Masks Definitions */
#define TSI578_SPX_CTL_PORT_TYPE                         ((uint32_t)0x00000001)
#define TSI578_SPX_CTL_PORT_LOCKOUT                      ((uint32_t)0x00000002)
#define TSI578_SPX_CTL_DROP_EN                           ((uint32_t)0x00000004)
#define TSI578_SPX_CTL_STOP_FAIL_EN                      ((uint32_t)0x00000008)
#define TSI578_SPX_CTL_ENUM_B                            ((uint32_t)0x00020000)
#define TSI578_SPX_CTL_MCS_EN                            ((uint32_t)0x00080000)
#define TSI578_SPX_CTL_ERR_DIS                           ((uint32_t)0x00100000)
#define TSI578_SPX_CTL_INPUT_EN                          ((uint32_t)0x00200000)
#define TSI578_SPX_CTL_OUTPUT_EN                         ((uint32_t)0x00400000)
#define TSI578_SPX_CTL_PORT_DIS                          ((uint32_t)0x00800000)
#define TSI578_SPX_CTL_OVER_PWIDTH                       ((uint32_t)0x07000000)
#define TSI578_SPX_CTL_INIT_PWIDTH                       ((uint32_t)0x38000000)
#define TSI578_SPX_CTL_PORT_WIDTH                        ((uint32_t)0xc0000000)

/* TSI578_RIO_ERR_RPT_BH : Register Bits Masks Definitions */
#define TSI578_RIO_ERR_RPT_BH_EF_ID                      ((uint32_t)0x0000ffff)
#define TSI578_RIO_ERR_RPT_BH_EF_PTR                     ((uint32_t)0xffff0000)

/* TSI578_RIO_LOG_ERR_DET : Register Bits Masks Definitions */
#define TSI578_RIO_LOG_ERR_DET_L_UNSUP_TRANS             ((uint32_t)0x00400000)
#define TSI578_RIO_LOG_ERR_DET_L_ILL_RESP                ((uint32_t)0x00800000)
#define TSI578_RIO_LOG_ERR_DET_L_ILL_TRANS               ((uint32_t)0x08000000)

/* TSI578_RIO_LOG_ERR_DET_EN : Register Bits Masks Definitions */
#define TSI578_RIO_LOG_ERR_DET_EN_UNSUP_TRANS_EN         ((uint32_t)0x00400000)
#define TSI578_RIO_LOG_ERR_DET_EN_ILL_RESP_EN            ((uint32_t)0x00800000)
#define TSI578_RIO_LOG_ERR_DET_EN_ILL_TRANS_EN           ((uint32_t)0x08000000)

/* TSI578_RIO_LOG_ERR_ADDR : Register Bits Masks Definitions */
#define TSI578_RIO_LOG_ERR_ADDR_WDPTR                    ((uint32_t)0x00000002)
#define TSI578_RIO_LOG_ERR_ADDR_ADDRESS                  ((uint32_t)0x00fffff8)

/* TSI578_RIO_LOG_ERR_DEVID : Register Bits Masks Definitions */
#define TSI578_RIO_LOG_ERR_DEVID_SRCID                   ((uint32_t)0x000000ff)
#define TSI578_RIO_LOG_ERR_DEVID_SRCID_MSB               ((uint32_t)0x0000ff00)

/* TSI578_RIO_LOG_ERR_CTRL_INFO : Register Bits Masks Definitions */
#define TSI578_RIO_LOG_ERR_CTRL_INFO_TTYPE               ((uint32_t)0x0f000000)
#define TSI578_RIO_LOG_ERR_CTRL_INFO_FTYPE               ((uint32_t)0xf0000000)

/* TSI578_RIO_PW_DESTID : Register Bits Masks Definitions */
#define TSI578_RIO_PW_DESTID_LARGE_DESTID                ((uint32_t)0x00008000)
#define TSI578_RIO_PW_DESTID_DESTID_LSB                  ((uint32_t)0x00ff0000)
#define TSI578_RIO_PW_DESTID_DESTID_MSB                  ((uint32_t)0xff000000)

/* TSI578_RIO_PKT_TTL : Register Bits Masks Definitions */
#define TSI578_RIO_PKT_TTL_TTL                           ((uint32_t)0xffff0000)

/* TSI578_SPX_ERR_DET : Register Bits Masks Definitions */
#define TSI578_SPX_ERR_DET_LINK_TO                       ((uint32_t)0x00000001)
#define TSI578_SPX_ERR_DET_CS_ACK_ILL                    ((uint32_t)0x00000002)
#define TSI578_SPX_ERR_DET_DELIN_ERR                     ((uint32_t)0x00000004)
#define TSI578_SPX_ERR_DET_PROT_ERR                      ((uint32_t)0x00000010)
#define TSI578_SPX_ERR_DET_LR_ACKID_ILL                  ((uint32_t)0x00000020)
#define TSI578_SPX_ERR_DET_PKT_ILL_SIZE                  ((uint32_t)0x00020000)
#define TSI578_SPX_ERR_DET_PKT_CRC_ERR                   ((uint32_t)0x00040000)
#define TSI578_SPX_ERR_DET_PKT_ILL_ACKID                 ((uint32_t)0x00080000)
#define TSI578_SPX_ERR_DET_CS_NOT_ACC                    ((uint32_t)0x00100000)
#define TSI578_SPX_ERR_DET_CS_ILL_ID                     ((uint32_t)0x00200000)
#define TSI578_SPX_ERR_DET_CS_CRC_ERR                    ((uint32_t)0x00400000)
#define TSI578_SPX_ERR_DET_IMP_SPEC_ERR                  ((uint32_t)0x80000000)

/* TSI578_SPX_RATE_EN : Register Bits Masks Definitions */
#define TSI578_SPX_RATE_EN_LINK_TO_EN                    ((uint32_t)0x00000001)
#define TSI578_SPX_RATE_EN_CS_ACK_ILL_EN                 ((uint32_t)0x00000002)
#define TSI578_SPX_RATE_EN_DELIN_ERR_EN                  ((uint32_t)0x00000004)
#define TSI578_SPX_RATE_EN_PROT_ERR_EN                   ((uint32_t)0x00000010)
#define TSI578_SPX_RATE_EN_LR_ACKID_ILL_EN               ((uint32_t)0x00000020)
#define TSI578_SPX_RATE_EN_PKT_ILL_SIZE_EN               ((uint32_t)0x00020000)
#define TSI578_SPX_RATE_EN_PKT_CRC_ERR_EN                ((uint32_t)0x00040000)
#define TSI578_SPX_RATE_EN_PKT_ILL_ACKID_EN              ((uint32_t)0x00080000)
#define TSI578_SPX_RATE_EN_CS_NOT_ACC_EN                 ((uint32_t)0x00100000)
#define TSI578_SPX_RATE_EN_CS_ILL_ID_EN                  ((uint32_t)0x00200000)
#define TSI578_SPX_RATE_EN_CS_CRC_ERR_EN                 ((uint32_t)0x00400000)
#define TSI578_SPX_RATE_EN_IMP_SPEC_ERR                  ((uint32_t)0x80000000)

/* TSI578_SPX_ERR_ATTR_CAPT_DBG0 : Register Bits Masks Definitions */
#define TSI578_SPX_ERR_ATTR_CAPT_DBG0_VAL_CAPT           ((uint32_t)0x00000001)
#define TSI578_SPX_ERR_ATTR_CAPT_DBG0_ERR_TYPE           ((uint32_t)0x1f000000)
#define TSI578_SPX_ERR_ATTR_CAPT_DBG0_INFO_TYPE          ((uint32_t)0xc0000000)

/* TSI578_SPX_ERR_CAPT_0_DBG1 : Register Bits Masks Definitions */
#define TSI578_SPX_ERR_CAPT_0_DBG1_CAPT_0                ((uint32_t)0xffffffff)

/* TSI578_SPX_ERR_CAPT_1_DBG2 : Register Bits Masks Definitions */
#define TSI578_SPX_ERR_CAPT_1_DBG2_CAPT_1                ((uint32_t)0xffffffff)

/* TSI578_SPX_ERR_CAPT_2_DBG3 : Register Bits Masks Definitions */
#define TSI578_SPX_ERR_CAPT_2_DBG3_CAPT_2                ((uint32_t)0xffffffff)

/* TSI578_SPX_ERR_CAPT_3_DBG4 : Register Bits Masks Definitions */
#define TSI578_SPX_ERR_CAPT_3_DBG4_CAPT_3                ((uint32_t)0xffffffff)

/* TSI578_SPX_ERR_RATE : Register Bits Masks Definitions */
#define TSI578_SPX_ERR_RATE_ERR_RATE_CNT                 ((uint32_t)0x000000ff)
#define TSI578_SPX_ERR_RATE_PEAK                         ((uint32_t)0x0000ff00)
#define TSI578_SPX_ERR_RATE_ERR_RR                       ((uint32_t)0x00030000)
#define TSI578_SPX_ERR_RATE_ERR_RB                       ((uint32_t)0xff000000)

/* TSI578_SPX_ERR_THRESH : Register Bits Masks Definitions */
#define TSI578_SPX_ERR_THRESH_ERR_RDT                    ((uint32_t)0x00ff0000)
#define TSI578_SPX_ERR_THRESH_ERR_RFT                    ((uint32_t)0xff000000)

/* TSI578_SPX_DISCOVERY_TIMER : Register Bits Masks Definitions */
#define TSI578_SPX_DISCOVERY_TIMER_PW_PRIORITY           ((uint32_t)0x00c00000)
#define TSI578_SPX_DISCOVERY_TIMER_DISCOVERY_TIMER       ((uint32_t)0xf0000000)

/* TSI578_SPX_MODE : Register Bits Masks Definitions */
#define TSI578_SPX_MODE_RCS_INT_EN                       ((uint32_t)0x00000001)
#define TSI578_SPX_MODE_MCS_INT_EN                       ((uint32_t)0x00000002)
#define TSI578_SPX_MODE_LUT_512                          ((uint32_t)0x01000000)
#define TSI578_SPX_MODE_SELF_RST                         ((uint32_t)0x02000000)
#define TSI578_SPX_MODE_PW_DIS                           ((uint32_t)0x08000000)
#define TSI578_SPX_MODE_IDLE_ERR_DIS                     ((uint32_t)0x20000000)

/* TSI578_SPX_CS_INT_STATUS : Register Bits Masks Definitions */
#define TSI578_SPX_CS_INT_STATUS_RCS                     ((uint32_t)0x00000001)
#define TSI578_SPX_CS_INT_STATUS_MCS                     ((uint32_t)0x00000002)

/* TSI578_SPX_RIO_WM : Register Bits Masks Definitions */
#define TSI578_SPX_RIO_WM_PRIO0WM                        ((uint32_t)0x00000007)
#define TSI578_SPX_RIO_WM_PRIO1WM                        ((uint32_t)0x00000700)
#define TSI578_SPX_RIO_WM_PRIO2WM                        ((uint32_t)0x00070000)

/* TSI578_SPX_ROUTE_CFG_DESTID : Register Bits Masks Definitions */
#define TSI578_SPX_ROUTE_CFG_DESTID_CFG_DEST_ID          ((uint32_t)0x000000ff)
#define TSI578_SPX_ROUTE_CFG_DESTID_LRG_CFG_DEST_ID      ((uint32_t)0x0000ff00)
#define TSI578_SPX_ROUTE_CFG_DESTID_PAR_INVERT           ((uint32_t)0x40000000)
#define TSI578_SPX_ROUTE_CFG_DESTID_AUTO_INC             ((uint32_t)0x80000000)

/* TSI578_SPX_ROUTE_CFG_PORT : Register Bits Masks Definitions */
#define TSI578_SPX_ROUTE_CFG_PORT_PORT                   ((uint32_t)0x000000ff)

/* TSI578_SPX_ROUTE_BASE : Register Bits Masks Definitions */
#define TSI578_SPX_ROUTE_BASE_BASE                       ((uint32_t)0xff000000)

/* TSI578_RIO_MC_IDX : Register Bits Masks Definitions */
#define TSI578_RIO_MC_IDX_MC_ID_DEV8                     ((uint32_t)0x000000ff)
#define TSI578_RIO_MC_IDX_MC_ID                          ((uint32_t)0x0000ffff)
#define TSI578_RIO_MC_IDX_LARGE_SYS                      ((uint32_t)0x40000000)
#define TSI578_RIO_MC_IDX_MC_EN                          ((uint32_t)0x80000000)

/* TSI578_RIO_MC_MSKX : Register Bits Masks Definitions */
#define TSI578_RIO_MC_MSKX_MC_MSK                        ((uint32_t)0xffff0000)

/* TSI578_SPX_ID : Register Bits Masks Definitions */
#define TSI578_SPX_ID_PORT_ID                            ((uint32_t)0x000000ff)

/* TSI578_SPX_CTL_INDEP : Register Bits Masks Definitions */
#define TSI578_SPX_CTL_INDEP_TEA_EN                      ((uint32_t)0x00000001)
#define TSI578_SPX_CTL_INDEP_RSVD1                       ((uint32_t)0x00000002)
#define TSI578_SPX_CTL_INDEP_INB_RDR_EN                  ((uint32_t)0x00000004)
#define TSI578_SPX_CTL_INDEP_INB_DEPTH_EN                ((uint32_t)0x00000008)
#define TSI578_SPX_CTL_INDEP_OUTB_DEPTH_EN               ((uint32_t)0x00000010)
#define TSI578_SPX_CTL_INDEP_MAX_RETRY_EN                ((uint32_t)0x00000020)
#define TSI578_SPX_CTL_INDEP_IRQ_EN                      ((uint32_t)0x00000040)
#define TSI578_SPX_CTL_INDEP_ILL_TRANS_ERR               ((uint32_t)0x00000080)
#define TSI578_SPX_CTL_INDEP_MAX_RETRY_THRESHOLD         ((uint32_t)0x0000ff00)
#define TSI578_SPX_CTL_INDEP_LUT_PAR_ERR_EN              ((uint32_t)0x00010000)
#define TSI578_SPX_CTL_INDEP_LINK_INIT_NOTIFICATION_EN   ((uint32_t)0x00020000)
#define TSI578_SPX_CTL_INDEP_MC_TEA_EN                   ((uint32_t)0x00040000)
#define TSI578_SPX_CTL_INDEP_PORT_ERR_EN                 ((uint32_t)0x00080000)
#define TSI578_SPX_CTL_INDEP_SEND_DBG_PKT                ((uint32_t)0x00400000)
#define TSI578_SPX_CTL_INDEP_DEBUG_MODE                  ((uint32_t)0x00800000)
#define TSI578_SPX_CTL_INDEP_TRANS_MODE                  ((uint32_t)0x01000000)
#define TSI578_SPX_CTL_INDEP_FORCE_REINIT                ((uint32_t)0x04000000)
#define TSI578_SPX_CTL_INDEP_SCRATCH                     ((uint32_t)0x20000000)

/* TSI578_SPX_SILENCE_TIMER : Register Bits Masks Definitions */
#define TSI578_SPX_SILENCE_TIMER_SILENCE_TIMER           ((uint32_t)0xf0000000)

/* TSI578_SPX_SEND_MCS : Register Bits Masks Definitions */
#define TSI578_SPX_SEND_MCS_SEND                         ((uint32_t)0x00000001)
#define TSI578_SPX_SEND_MCS_DONE                         ((uint32_t)0x00000002)

/* TSI578_SPX_LUT_PAR_ERR_INFO : Register Bits Masks Definitions */
#define TSI578_SPX_LUT_PAR_ERR_INFO_PORT_NUM             ((uint32_t)0x0000000f)
#define TSI578_SPX_LUT_PAR_ERR_INFO_LUT_VLD              ((uint32_t)0x00000040)
#define TSI578_SPX_LUT_PAR_ERR_INFO_PTY_BIT              ((uint32_t)0x00000080)
#define TSI578_SPX_LUT_PAR_ERR_INFO_LG_DESTID            ((uint32_t)0x00008000)
#define TSI578_SPX_LUT_PAR_ERR_INFO_DESTID_LSB           ((uint32_t)0x00ff0000)
#define TSI578_SPX_LUT_PAR_ERR_INFO_DESTID_MSB           ((uint32_t)0xff000000)

/* TSI578_SPX_CS_TX : Register Bits Masks Definitions */
#define TSI578_SPX_CS_TX_CS_EMB                          ((uint32_t)0x00001000)
#define TSI578_SPX_CS_TX_CMD                             ((uint32_t)0x0000e000)
#define TSI578_SPX_CS_TX_STYPE_1                         ((uint32_t)0x00070000)
#define TSI578_SPX_CS_TX_PAR_1                           ((uint32_t)0x00f80000)
#define TSI578_SPX_CS_TX_PAR_0                           ((uint32_t)0x1f000000)
#define TSI578_SPX_CS_TX_STYPE_0                         ((uint32_t)0xe0000000)

/* TSI578_SPX_INT_STATUS : Register Bits Masks Definitions */
#define TSI578_SPX_INT_STATUS_TEA                        ((uint32_t)0x00000001)
#define TSI578_SPX_INT_STATUS_TTL_EXPIRED                ((uint32_t)0x00000002)
#define TSI578_SPX_INT_STATUS_INB_RDR                    ((uint32_t)0x00000004)
#define TSI578_SPX_INT_STATUS_INB_DEPTH                  ((uint32_t)0x00000008)
#define TSI578_SPX_INT_STATUS_OUTB_DEPTH                 ((uint32_t)0x00000010)
#define TSI578_SPX_INT_STATUS_MAX_RETRY                  ((uint32_t)0x00000020)
#define TSI578_SPX_INT_STATUS_IRQ_ERR                    ((uint32_t)0x00000040)
#define TSI578_SPX_INT_STATUS_ILL_TRANS_ERR              ((uint32_t)0x00000080)
#define TSI578_SPX_INT_STATUS_LUT_PAR_ERR                ((uint32_t)0x00010000)
#define TSI578_SPX_INT_STATUS_LINK_INIT_NOTIFICATION     ((uint32_t)0x00020000)
#define TSI578_SPX_INT_STATUS_MC_TEA                     ((uint32_t)0x00040000)

/* TSI578_SPX_INT_GEN : Register Bits Masks Definitions */
#define TSI578_SPX_INT_GEN_TEA_GEN                       ((uint32_t)0x00000001)
#define TSI578_SPX_INT_GEN_TTL_EXPIRED_GEN               ((uint32_t)0x00000002)
#define TSI578_SPX_INT_GEN_INB_RDR_GEN                   ((uint32_t)0x00000004)
#define TSI578_SPX_INT_GEN_INB_DEPTH_GEN                 ((uint32_t)0x00000008)
#define TSI578_SPX_INT_GEN_OUTB_DEPTH_GEN                ((uint32_t)0x00000010)
#define TSI578_SPX_INT_GEN_MAX_RETRY_GEN                 ((uint32_t)0x00000020)
#define TSI578_SPX_INT_GEN_ILL_TRANS_GEN                 ((uint32_t)0x00000080)
#define TSI578_SPX_INT_GEN_LUT_PAR_ERR_GEN               ((uint32_t)0x00010000)
#define TSI578_SPX_INT_GEN_LINK_INIT_NOTIFICATION_GEN    ((uint32_t)0x00020000)
#define TSI578_SPX_INT_GEN_MC_TEA_GEN                    ((uint32_t)0x00040000)

/* TSI578_SPX_PSC0N1_CTRL : Register Bits Masks Definitions */
#define TSI578_SPX_PSC0N1_CTRL_PS1_TYPE                  ((uint32_t)0x00000007)
#define TSI578_SPX_PSC0N1_CTRL_PS1_DIR                   ((uint32_t)0x00000100)
#define TSI578_SPX_PSC0N1_CTRL_PS1_PRIO0                 ((uint32_t)0x00001000)
#define TSI578_SPX_PSC0N1_CTRL_PS1_PRIO1                 ((uint32_t)0x00002000)
#define TSI578_SPX_PSC0N1_CTRL_PS1_PRIO2                 ((uint32_t)0x00004000)
#define TSI578_SPX_PSC0N1_CTRL_PS1_PRIO3                 ((uint32_t)0x00008000)
#define TSI578_SPX_PSC0N1_CTRL_PS0_TYPE                  ((uint32_t)0x00070000)
#define TSI578_SPX_PSC0N1_CTRL_PS0_DIR                   ((uint32_t)0x01000000)
#define TSI578_SPX_PSC0N1_CTRL_PS0_PRIO0                 ((uint32_t)0x10000000)
#define TSI578_SPX_PSC0N1_CTRL_PS0_PRIO1                 ((uint32_t)0x20000000)
#define TSI578_SPX_PSC0N1_CTRL_PS0_PRIO2                 ((uint32_t)0x40000000)
#define TSI578_SPX_PSC0N1_CTRL_PS0_PRIO3                 ((uint32_t)0x80000000)

/* TSI578_SPX_PSC2N3_CTRL : Register Bits Masks Definitions */
#define TSI578_SPX_PSC2N3_CTRL_PS3_TYPE                  ((uint32_t)0x00000007)
#define TSI578_SPX_PSC2N3_CTRL_PS3_DIR                   ((uint32_t)0x00000100)
#define TSI578_SPX_PSC2N3_CTRL_PS3_PRIO0                 ((uint32_t)0x00001000)
#define TSI578_SPX_PSC2N3_CTRL_PS3_PRIO1                 ((uint32_t)0x00002000)
#define TSI578_SPX_PSC2N3_CTRL_PS3_PRIO2                 ((uint32_t)0x00004000)
#define TSI578_SPX_PSC2N3_CTRL_PS3_PRIO3                 ((uint32_t)0x00008000)
#define TSI578_SPX_PSC2N3_CTRL_PS2_TYPE                  ((uint32_t)0x00070000)
#define TSI578_SPX_PSC2N3_CTRL_PS2_DIR                   ((uint32_t)0x01000000)
#define TSI578_SPX_PSC2N3_CTRL_PS2_PRIO0                 ((uint32_t)0x10000000)
#define TSI578_SPX_PSC2N3_CTRL_PS2_PRIO1                 ((uint32_t)0x20000000)
#define TSI578_SPX_PSC2N3_CTRL_PS2_PRIO2                 ((uint32_t)0x40000000)
#define TSI578_SPX_PSC2N3_CTRL_PS2_PRIO3                 ((uint32_t)0x80000000)

/* TSI578_SPX_PSC4N5_CTRL : Register Bits Masks Definitions */
#define TSI578_SPX_PSC4N5_CTRL_PS5_TYPE                  ((uint32_t)0x00000007)
#define TSI578_SPX_PSC4N5_CTRL_PS5_DIR                   ((uint32_t)0x00000100)
#define TSI578_SPX_PSC4N5_CTRL_PS5_PRIO0                 ((uint32_t)0x00001000)
#define TSI578_SPX_PSC4N5_CTRL_PS5_PRIO1                 ((uint32_t)0x00002000)
#define TSI578_SPX_PSC4N5_CTRL_PS5_PRIO2                 ((uint32_t)0x00004000)
#define TSI578_SPX_PSC4N5_CTRL_PS5_PRIO3                 ((uint32_t)0x00008000)
#define TSI578_SPX_PSC4N5_CTRL_PS4_TYPE                  ((uint32_t)0x00070000)
#define TSI578_SPX_PSC4N5_CTRL_PS4_DIR                   ((uint32_t)0x01000000)
#define TSI578_SPX_PSC4N5_CTRL_PS4_PRIO0                 ((uint32_t)0x10000000)
#define TSI578_SPX_PSC4N5_CTRL_PS4_PRIO1                 ((uint32_t)0x20000000)
#define TSI578_SPX_PSC4N5_CTRL_PS4_PRIO2                 ((uint32_t)0x40000000)
#define TSI578_SPX_PSC4N5_CTRL_PS4_PRIO3                 ((uint32_t)0x80000000)

/* TSI578_SPX_PSC0 : Register Bits Masks Definitions */
#define TSI578_SPX_PSC0_PS0_CTR                          ((uint32_t)0xffffffff)

/* TSI578_SPX_PSC1 : Register Bits Masks Definitions */
#define TSI578_SPX_PSC1_PS1_CTR                          ((uint32_t)0xffffffff)

/* TSI578_SPX_PSC2 : Register Bits Masks Definitions */
#define TSI578_SPX_PSC2_PS2_CTR                          ((uint32_t)0xffffffff)

/* TSI578_SPX_PSC3 : Register Bits Masks Definitions */
#define TSI578_SPX_PSC3_PS3_CTR                          ((uint32_t)0xffffffff)

/* TSI578_SPX_PSC4 : Register Bits Masks Definitions */
#define TSI578_SPX_PSC4_PS4_CTR                          ((uint32_t)0xffffffff)

/* TSI578_SPX_PSC5 : Register Bits Masks Definitions */
#define TSI578_SPX_PSC5_PS5_CTR                          ((uint32_t)0xffffffff)

/* TSI578_SPX_TX_Q_D_THRESH : Register Bits Masks Definitions */
#define TSI578_SPX_TX_Q_D_THRESH_LEAK_RT                 ((uint32_t)0x000007ff)
#define TSI578_SPX_TX_Q_D_THRESH_DEPTH                   ((uint32_t)0x0000f000)
#define TSI578_SPX_TX_Q_D_THRESH_CONG_PERIOD             ((uint32_t)0xffff0000)

/* TSI578_SPX_TX_Q_STATUS : Register Bits Masks Definitions */
#define TSI578_SPX_TX_Q_STATUS_CONG_THRESH               ((uint32_t)0x0000ffff)
#define TSI578_SPX_TX_Q_STATUS_CONG_CTR                  ((uint32_t)0xffff0000)

/* TSI578_SPX_TX_Q_PERIOD : Register Bits Masks Definitions */
#define TSI578_SPX_TX_Q_PERIOD_CONG_PERIOD_CTR           ((uint32_t)0xffffffff)

/* TSI578_SPX_RX_Q_D_THRESH : Register Bits Masks Definitions */
#define TSI578_SPX_RX_Q_D_THRESH_DEPTH                   ((uint32_t)0x0000f000)
#define TSI578_SPX_RX_Q_D_THRESH_CONG_PERIOD             ((uint32_t)0xffff0000)

/* TSI578_SPX_RX_Q_STATUS : Register Bits Masks Definitions */
#define TSI578_SPX_RX_Q_STATUS_CONG_THRESH               ((uint32_t)0x0000ffff)
#define TSI578_SPX_RX_Q_STATUS_CONG_CTR                  ((uint32_t)0xffff0000)

/* TSI578_SPX_RX_Q_PERIOD : Register Bits Masks Definitions */
#define TSI578_SPX_RX_Q_PERIOD_CONG_PERIOD_CTR           ((uint32_t)0xffffffff)

/* TSI578_SPX_REORDER_CTR : Register Bits Masks Definitions */
#define TSI578_SPX_REORDER_CTR_THRESH                    ((uint32_t)0x0000ffff)
#define TSI578_SPX_REORDER_CTR_CTR                       ((uint32_t)0xffff0000)

/* TSI578_SMACX_CFG_CH0 : Register Bits Masks Definitions */
#define TSI578_SMACX_CFG_CH0_RX_ALIGN_EN                 ((uint32_t)0x00000002)
#define TSI578_SMACX_CFG_CH0_LOS_CTL                     ((uint32_t)0x0000000c)
#define TSI578_SMACX_CFG_CH0_TX_CKO_EN                   ((uint32_t)0x00000010)
#define TSI578_SMACX_CFG_CH0_RX_DPLL_MODE                ((uint32_t)0x000000e0)
#define TSI578_SMACX_CFG_CH0_RX_EQ_VAL                   ((uint32_t)0x00000700)
#define TSI578_SMACX_CFG_CH0_DPLL_RESET                  ((uint32_t)0x00002000)
#define TSI578_SMACX_CFG_CH0_RX_EN                       ((uint32_t)0x00004000)
#define TSI578_SMACX_CFG_CH0_RX_PLL_PWRON                ((uint32_t)0x00008000)
#define TSI578_SMACX_CFG_CH0_TX_BOOST                    ((uint32_t)0x000f0000)
#define TSI578_SMACX_CFG_CH0_TX_EN                       ((uint32_t)0x00700000)
#define TSI578_SMACX_CFG_CH0_TX_ATTEN                    ((uint32_t)0x07000000)
#define TSI578_SMACX_CFG_CH0_TX_CALC                     ((uint32_t)0x40000000)
#define TSI578_SMACX_CFG_CH0_HALF_RATE                   ((uint32_t)0x80000000)

/* TSI578_SMACX_CFG_CH1 : Register Bits Masks Definitions */
#define TSI578_SMACX_CFG_CH1_RX_ALIGN_EN                 ((uint32_t)0x00000002)
#define TSI578_SMACX_CFG_CH1_LOS_CTL                     ((uint32_t)0x0000000c)
#define TSI578_SMACX_CFG_CH1_TX_CKO_EN                   ((uint32_t)0x00000010)
#define TSI578_SMACX_CFG_CH1_RX_DPLL_MODE                ((uint32_t)0x000000e0)
#define TSI578_SMACX_CFG_CH1_RX_EQ_VAL                   ((uint32_t)0x00000700)
#define TSI578_SMACX_CFG_CH1_DPLL_RESET                  ((uint32_t)0x00002000)
#define TSI578_SMACX_CFG_CH1_RX_EN                       ((uint32_t)0x00004000)
#define TSI578_SMACX_CFG_CH1_RX_PLL_PWRON                ((uint32_t)0x00008000)
#define TSI578_SMACX_CFG_CH1_TX_BOOST                    ((uint32_t)0x000f0000)
#define TSI578_SMACX_CFG_CH1_TX_EN                       ((uint32_t)0x00700000)
#define TSI578_SMACX_CFG_CH1_TX_ATTEN                    ((uint32_t)0x07000000)
#define TSI578_SMACX_CFG_CH1_TX_CALC                     ((uint32_t)0x40000000)
#define TSI578_SMACX_CFG_CH1_HALF_RATE                   ((uint32_t)0x80000000)

/* TSI578_SMACX_CFG_CH2 : Register Bits Masks Definitions */
#define TSI578_SMACX_CFG_CH2_RX_ALIGN_EN                 ((uint32_t)0x00000002)
#define TSI578_SMACX_CFG_CH2_LOS_CTL                     ((uint32_t)0x0000000c)
#define TSI578_SMACX_CFG_CH2_TX_CKO_EN                   ((uint32_t)0x00000010)
#define TSI578_SMACX_CFG_CH2_RX_DPLL_MODE                ((uint32_t)0x000000e0)
#define TSI578_SMACX_CFG_CH2_RX_EQ_VAL                   ((uint32_t)0x00000700)
#define TSI578_SMACX_CFG_CH2_DPLL_RESET                  ((uint32_t)0x00002000)
#define TSI578_SMACX_CFG_CH2_RX_EN                       ((uint32_t)0x00004000)
#define TSI578_SMACX_CFG_CH2_RX_PLL_PWRON                ((uint32_t)0x00008000)
#define TSI578_SMACX_CFG_CH2_TX_BOOST                    ((uint32_t)0x000f0000)
#define TSI578_SMACX_CFG_CH2_TX_EN                       ((uint32_t)0x00700000)
#define TSI578_SMACX_CFG_CH2_TX_ATTEN                    ((uint32_t)0x07000000)
#define TSI578_SMACX_CFG_CH2_TX_CALC                     ((uint32_t)0x40000000)
#define TSI578_SMACX_CFG_CH2_HALF_RATE                   ((uint32_t)0x80000000)

/* TSI578_SMACX_CFG_CH3 : Register Bits Masks Definitions */
#define TSI578_SMACX_CFG_CH3_RX_ALIGN_EN                 ((uint32_t)0x00000002)
#define TSI578_SMACX_CFG_CH3_LOS_CTL                     ((uint32_t)0x0000000c)
#define TSI578_SMACX_CFG_CH3_TX_CKO_EN                   ((uint32_t)0x00000010)
#define TSI578_SMACX_CFG_CH3_RX_DPLL_MODE                ((uint32_t)0x000000e0)
#define TSI578_SMACX_CFG_CH3_RX_EQ_VAL                   ((uint32_t)0x00000700)
#define TSI578_SMACX_CFG_CH3_DPLL_RESET                  ((uint32_t)0x00002000)
#define TSI578_SMACX_CFG_CH3_RX_EN                       ((uint32_t)0x00004000)
#define TSI578_SMACX_CFG_CH3_RX_PLL_PWRON                ((uint32_t)0x00008000)
#define TSI578_SMACX_CFG_CH3_TX_BOOST                    ((uint32_t)0x000f0000)
#define TSI578_SMACX_CFG_CH3_TX_EN                       ((uint32_t)0x00700000)
#define TSI578_SMACX_CFG_CH3_TX_ATTEN                    ((uint32_t)0x07000000)
#define TSI578_SMACX_CFG_CH3_TX_CALC                     ((uint32_t)0x40000000)
#define TSI578_SMACX_CFG_CH3_HALF_RATE                   ((uint32_t)0x80000000)

/* TSI578_SMACX_CFG_GBL : Register Bits Masks Definitions */
#define TSI578_SMACX_CFG_GBL_MPLL_CK_OFF                 ((uint32_t)0x00000040)
#define TSI578_SMACX_CFG_GBL_MPLL_PWR_ON                 ((uint32_t)0x00000080)
#define TSI578_SMACX_CFG_GBL_LOS_LVL                     ((uint32_t)0x00001f00)
#define TSI578_SMACX_CFG_GBL_ACJT_LVL                    ((uint32_t)0x001f0000)
#define TSI578_SMACX_CFG_GBL_TX_LVL                      ((uint32_t)0x1f000000)
#define TSI578_SMACX_CFG_GBL_BYPASS_INIT                 ((uint32_t)0x40000000)
#define TSI578_SMACX_CFG_GBL_SERDES_RSTN                 ((uint32_t)0x80000000)

/* TSI578_SMACX_CFG_GBLB : Register Bits Masks Definitions */
#define TSI578_SMACX_CFG_GBLB_MPLL_NCY5                  ((uint32_t)0x000000c0)
#define TSI578_SMACX_CFG_GBLB_MPLL_PRESCALE              ((uint32_t)0x00300000)

/* TSI578_SMACX_DLOOP_CLK_SEL : Register Bits Masks Definitions */
#define TSI578_SMACX_DLOOP_CLK_SEL_CLK_SEL               ((uint32_t)0x00000003)
#define TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X4               ((uint32_t)0x00000004)
#define TSI578_SMACX_DLOOP_CLK_SEL_PWDN_X1               ((uint32_t)0x00000008)
#define TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X4           ((uint32_t)0x00000010)
#define TSI578_SMACX_DLOOP_CLK_SEL_SOFT_RST_X1           ((uint32_t)0x00000020)
#define TSI578_SMACX_DLOOP_CLK_SEL_SWAP_RX               ((uint32_t)0x00000040)
#define TSI578_SMACX_DLOOP_CLK_SEL_SWAP_TX               ((uint32_t)0x00000080)
#define TSI578_SMACX_DLOOP_CLK_SEL_DLB_EVEN_EN           ((uint32_t)0x00000100)
#define TSI578_SMACX_DLOOP_CLK_SEL_DLB_ODD_EN            ((uint32_t)0x00000200)
#define TSI578_SMACX_DLOOP_CLK_SEL_MAC_MODE              ((uint32_t)0x00000400)
#define TSI578_SMACX_DLOOP_CLK_SEL_LINE_LB               ((uint32_t)0x0000f000)
#define TSI578_SMACX_DLOOP_CLK_SEL_DLT_THRESH            ((uint32_t)0x7fff0000)
#define TSI578_SMACX_DLOOP_CLK_SEL_DLT_EN                ((uint32_t)0x80000000)

/* TSI578_SMACX_SERDES_OUTPUT_PINS : Register Bits Masks Definitions */
#define TSI578_SMACX_SERDES_OUTPUT_PINS_POWER_GOOD       ((uint32_t)0x00000004)
#define TSI578_SMACX_SERDES_OUTPUT_PINS_OP_DONE          ((uint32_t)0x00000008)
#define TSI578_SMACX_SERDES_OUTPUT_PINS_LOS              ((uint32_t)0x000000f0)
#define TSI578_SMACX_SERDES_OUTPUT_PINS_RX_VALID         ((uint32_t)0x00000f00)
#define TSI578_SMACX_SERDES_OUTPUT_PINS_RX_PLL_STATE     ((uint32_t)0x0000f000)
#define TSI578_SMACX_SERDES_OUTPUT_PINS_TX_RXPRES        ((uint32_t)0x000f0000)
#define TSI578_SMACX_SERDES_OUTPUT_PINS_TX_DONE          ((uint32_t)0x00f00000)
#define TSI578_SMACX_SERDES_OUTPUT_PINS_RX_COMMA_DET     ((uint32_t)0xff000000)

/* TSI578_MCES_PIN_CTRL : Register Bits Masks Definitions */
#define TSI578_MCES_PIN_CTRL_MCES_CTRL                   ((uint32_t)0x30000000)

/* TSI578_SMACX_SPY_BUS_CFG : Register Bits Masks Definitions */
#define TSI578_SMACX_SPY_BUS_CFG_SEL                     ((uint32_t)0x0fffffff)
#define TSI578_SMACX_SPY_BUS_CFG_EN                      ((uint32_t)0xf0000000)

/* TSI578_ERRX_INJ_CMP_MASK : Register Bits Masks Definitions */
#define TSI578_ERRX_INJ_CMP_MASK_CMP_MASK                ((uint32_t)0xffffffff)

/* TSI578_ERRX_INJ_CMP_BIT_VLD : Register Bits Masks Definitions */
#define TSI578_ERRX_INJ_CMP_BIT_VLD_CMP_BIT_VLD          ((uint32_t)0xffffffff)

/* TSI578_ERRX_INJ : Register Bits Masks Definitions */
#define TSI578_ERRX_INJ_ERR_INJ                          ((uint32_t)0xffffffff)

/* TSI578_ERRX_INJ_BIT_VLD : Register Bits Masks Definitions */
#define TSI578_ERRX_INJ_BIT_VLD_ERR_INJ_BIT_VLD          ((uint32_t)0xffffffff)

/* TSI578_FAB_CTL : Register Bits Masks Definitions */
#define TSI578_FAB_CTL_TEA_OUT                           ((uint32_t)0x0000ffff)
#define TSI578_FAB_CTL_TEA_EN                            ((uint32_t)0x00010000)
#define TSI578_FAB_CTL_TEA_INT_EN                        ((uint32_t)0x00020000)
#define TSI578_FAB_CTL_IN_ARB_MODE                       ((uint32_t)0x00300000)
#define TSI578_FAB_CTL_RDR_LIMIT_EN                      ((uint32_t)0x00800000)
#define TSI578_FAB_CTL_RDR_LIMIT                         ((uint32_t)0x0f000000)

/* TSI578_FAB_INT_STAT : Register Bits Masks Definitions */
#define TSI578_FAB_INT_STAT_PORT0_IRQ                    ((uint32_t)0x00000001)
#define TSI578_FAB_INT_STAT_PORT1_IRQ                    ((uint32_t)0x00000002)
#define TSI578_FAB_INT_STAT_PORT2_IRQ                    ((uint32_t)0x00000004)
#define TSI578_FAB_INT_STAT_PORT3_IRQ                    ((uint32_t)0x00000008)
#define TSI578_FAB_INT_STAT_PORT4_IRQ                    ((uint32_t)0x00000010)
#define TSI578_FAB_INT_STAT_PORT5_IRQ                    ((uint32_t)0x00000020)
#define TSI578_FAB_INT_STAT_PORT6_IRQ                    ((uint32_t)0x00000040)
#define TSI578_FAB_INT_STAT_PORT7_IRQ                    ((uint32_t)0x00000080)
#define TSI578_FAB_INT_STAT_PORT8_IRQ                    ((uint32_t)0x00000100)
#define TSI578_FAB_INT_STAT_PORT9_IRQ                    ((uint32_t)0x00000200)
#define TSI578_FAB_INT_STAT_PORT10_IRQ                   ((uint32_t)0x00000400)
#define TSI578_FAB_INT_STAT_PORT11_IRQ                   ((uint32_t)0x00000800)
#define TSI578_FAB_INT_STAT_PORT12_IRQ                   ((uint32_t)0x00001000)
#define TSI578_FAB_INT_STAT_PORT13_IRQ                   ((uint32_t)0x00002000)
#define TSI578_FAB_INT_STAT_PORT14_IRQ                   ((uint32_t)0x00004000)
#define TSI578_FAB_INT_STAT_PORT15_IRQ                   ((uint32_t)0x00008000)

/* TSI578_RIO_MC_LAT_ERR : Register Bits Masks Definitions */
#define TSI578_RIO_MC_LAT_ERR_P0_ERR                     ((uint32_t)0x00000001)
#define TSI578_RIO_MC_LAT_ERR_P1_ERR                     ((uint32_t)0x00000002)
#define TSI578_RIO_MC_LAT_ERR_P2_ERR                     ((uint32_t)0x00000004)
#define TSI578_RIO_MC_LAT_ERR_P3_ERR                     ((uint32_t)0x00000008)
#define TSI578_RIO_MC_LAT_ERR_P4_ERR                     ((uint32_t)0x00000010)
#define TSI578_RIO_MC_LAT_ERR_P5_ERR                     ((uint32_t)0x00000020)
#define TSI578_RIO_MC_LAT_ERR_P6_ERR                     ((uint32_t)0x00000040)
#define TSI578_RIO_MC_LAT_ERR_P7_ERR                     ((uint32_t)0x00000080)
#define TSI578_RIO_MC_LAT_ERR_P8_ERR                     ((uint32_t)0x00000100)
#define TSI578_RIO_MC_LAT_ERR_P9_ERR                     ((uint32_t)0x00000200)
#define TSI578_RIO_MC_LAT_ERR_P10_ERR                    ((uint32_t)0x00000400)
#define TSI578_RIO_MC_LAT_ERR_P11_ERR                    ((uint32_t)0x00000800)
#define TSI578_RIO_MC_LAT_ERR_P12_ERR                    ((uint32_t)0x00001000)
#define TSI578_RIO_MC_LAT_ERR_P13_ERR                    ((uint32_t)0x00002000)
#define TSI578_RIO_MC_LAT_ERR_P14_ERR                    ((uint32_t)0x00004000)
#define TSI578_RIO_MC_LAT_ERR_P15_ERR                    ((uint32_t)0x00008000)

/* TSI578_RIO_MC_LAT_ERR_SET : Register Bits Masks Definitions */
#define TSI578_RIO_MC_LAT_ERR_SET_P0_SET                 ((uint32_t)0x00000001)
#define TSI578_RIO_MC_LAT_ERR_SET_P1_SET                 ((uint32_t)0x00000002)
#define TSI578_RIO_MC_LAT_ERR_SET_P2_SET                 ((uint32_t)0x00000004)
#define TSI578_RIO_MC_LAT_ERR_SET_P3_SET                 ((uint32_t)0x00000008)
#define TSI578_RIO_MC_LAT_ERR_SET_P4_SET                 ((uint32_t)0x00000010)
#define TSI578_RIO_MC_LAT_ERR_SET_P5_SET                 ((uint32_t)0x00000020)
#define TSI578_RIO_MC_LAT_ERR_SET_P6_SET                 ((uint32_t)0x00000040)
#define TSI578_RIO_MC_LAT_ERR_SET_P7_SET                 ((uint32_t)0x00000080)
#define TSI578_RIO_MC_LAT_ERR_SET_P8_SET                 ((uint32_t)0x00000100)
#define TSI578_RIO_MC_LAT_ERR_SET_P9_SET                 ((uint32_t)0x00000200)
#define TSI578_RIO_MC_LAT_ERR_SET_P10_SET                ((uint32_t)0x00000400)
#define TSI578_RIO_MC_LAT_ERR_SET_P11_SET                ((uint32_t)0x00000800)
#define TSI578_RIO_MC_LAT_ERR_SET_P12_SET                ((uint32_t)0x00001000)
#define TSI578_RIO_MC_LAT_ERR_SET_P13_SET                ((uint32_t)0x00002000)
#define TSI578_RIO_MC_LAT_ERR_SET_P14_SET                ((uint32_t)0x00004000)
#define TSI578_RIO_MC_LAT_ERR_SET_P15_SET                ((uint32_t)0x00008000)

/* TSI578_PKTGEN_CNTRL : Register Bits Masks Definitions */
#define TSI578_PKTGEN_CNTRL_REPEAT                       ((uint32_t)0x00ff0000)
#define TSI578_PKTGEN_CNTRL_PACE                         ((uint32_t)0x3f000000)
#define TSI578_PKTGEN_CNTRL_INIT                         ((uint32_t)0x40000000)
#define TSI578_PKTGEN_CNTRL_PKTGEN_EN                    ((uint32_t)0x80000000)

/* Meaningful field values                               */
#define TSI578_PKTGEN_CNTRL_PACE_RANDOM                  ((uint32_t)0x3F000000)
#define TSI578_PKTGEN_CNTRL_PACE_SLOWEST                 ((uint32_t)0x3E000000)
#define TSI578_PKTGEN_CNTRL_PACE_FASTEST                 ((uint32_t)0x00000000)
#define TSI578_PKTGEN_CNTRL_REPEAT_ONCE                  ((uint32_t)0x00010000)
#define TSI578_PKTGEN_CNTRL_REPEAT_FOREVER               ((uint32_t)0x00000000)

// Multicast buffer holds 8 maximum sized packets.
#define TSI578_PKTGEN_BUFF_SIZE (35 * 8)

/* TSI578_WRKQ_DATA : Register Bits Masks Definitions */
#define TSI578_WRKQ_DATA_RD_WR_DATA                      ((uint32_t)0xffffffff)

/* TSI578_PKTGEN_CNTRL : Register Bits Masks Definitions */
#define TSI578_LST_DTM_STATUS_EOP                        ((uint32_t)0x80000000)
#define TSI578_LST_DTM_STATUS_STATUS                     ((uint32_t)0x60000000)
#define TSI578_LST_DTM_STATUS_MC_PKT_DISCARD             ((uint32_t)0x01000000)
#define TSI578_LST_DTM_STATUS_RD_PTR                     ((uint32_t)0x000003FF)

/* Odd or even multiple of 4 bytes                       */
#define TSI578_LST_DTM_STATUS_STATUS_ODD                 ((uint32_t)0x20000000)
#define TSI578_LST_DTM_STATUS_STATUS_EVEN                ((uint32_t)0x60000000)

/* TSI578_TX_ENBL        Register Bits Masks Definitions */
#define TSI578_TX_ENBL_START                             ((uint32_t)0x80000000)
#define TSI578_TX_ENBL_DONE                              ((uint32_t)0x40000000)

/* TSI578_GLOB_INT_STATUS : Register Bits Masks Definitions */
#define TSI578_GLOB_INT_STATUS_PORT0                     ((uint32_t)0x00000001)
#define TSI578_GLOB_INT_STATUS_PORT1                     ((uint32_t)0x00000002)
#define TSI578_GLOB_INT_STATUS_PORT2                     ((uint32_t)0x00000004)
#define TSI578_GLOB_INT_STATUS_PORT3                     ((uint32_t)0x00000008)
#define TSI578_GLOB_INT_STATUS_PORT4                     ((uint32_t)0x00000010)
#define TSI578_GLOB_INT_STATUS_PORT5                     ((uint32_t)0x00000020)
#define TSI578_GLOB_INT_STATUS_PORT6                     ((uint32_t)0x00000040)
#define TSI578_GLOB_INT_STATUS_PORT7                     ((uint32_t)0x00000080)
#define TSI578_GLOB_INT_STATUS_PORT8                     ((uint32_t)0x00000100)
#define TSI578_GLOB_INT_STATUS_PORT9                     ((uint32_t)0x00000200)
#define TSI578_GLOB_INT_STATUS_PORT10                    ((uint32_t)0x00000400)
#define TSI578_GLOB_INT_STATUS_PORT11                    ((uint32_t)0x00000800)
#define TSI578_GLOB_INT_STATUS_PORT12                    ((uint32_t)0x00001000)
#define TSI578_GLOB_INT_STATUS_PORT13                    ((uint32_t)0x00002000)
#define TSI578_GLOB_INT_STATUS_PORT14                    ((uint32_t)0x00004000)
#define TSI578_GLOB_INT_STATUS_PORT15                    ((uint32_t)0x00008000)
#define TSI578_GLOB_INT_STATUS_MC_LAT                    ((uint32_t)0x00020000)
#define TSI578_GLOB_INT_STATUS_TEA                       ((uint32_t)0x01000000)
#define TSI578_GLOB_INT_STATUS_I2C                       ((uint32_t)0x02000000)
#define TSI578_GLOB_INT_STATUS_MCS                       ((uint32_t)0x04000000)
#define TSI578_GLOB_INT_STATUS_RCS                       ((uint32_t)0x08000000)

/* TSI578_GLOB_INT_ENABLE : Register Bits Masks Definitions */
#define TSI578_GLOB_INT_ENABLE_PORT0_EN                  ((uint32_t)0x00000001)
#define TSI578_GLOB_INT_ENABLE_PORT1_EN                  ((uint32_t)0x00000002)
#define TSI578_GLOB_INT_ENABLE_PORT2_EN                  ((uint32_t)0x00000004)
#define TSI578_GLOB_INT_ENABLE_PORT3_EN                  ((uint32_t)0x00000008)
#define TSI578_GLOB_INT_ENABLE_PORT4_EN                  ((uint32_t)0x00000010)
#define TSI578_GLOB_INT_ENABLE_PORT5_EN                  ((uint32_t)0x00000020)
#define TSI578_GLOB_INT_ENABLE_PORT6_EN                  ((uint32_t)0x00000040)
#define TSI578_GLOB_INT_ENABLE_PORT7_EN                  ((uint32_t)0x00000080)
#define TSI578_GLOB_INT_ENABLE_PORT8_EN                  ((uint32_t)0x00000100)
#define TSI578_GLOB_INT_ENABLE_PORT9_EN                  ((uint32_t)0x00000200)
#define TSI578_GLOB_INT_ENABLE_PORT10_EN                 ((uint32_t)0x00000400)
#define TSI578_GLOB_INT_ENABLE_PORT11_EN                 ((uint32_t)0x00000800)
#define TSI578_GLOB_INT_ENABLE_PORT12_EN                 ((uint32_t)0x00001000)
#define TSI578_GLOB_INT_ENABLE_PORT13_EN                 ((uint32_t)0x00002000)
#define TSI578_GLOB_INT_ENABLE_PORT14_EN                 ((uint32_t)0x00004000)
#define TSI578_GLOB_INT_ENABLE_PORT15_EN                 ((uint32_t)0x00008000)
#define TSI578_GLOB_INT_ENABLE_MC_LAT_EN                 ((uint32_t)0x00020000)
#define TSI578_GLOB_INT_ENABLE_TEA_EN                    ((uint32_t)0x01000000)
#define TSI578_GLOB_INT_ENABLE_I2C_EN                    ((uint32_t)0x02000000)
#define TSI578_GLOB_INT_ENABLE_MCS_EN                    ((uint32_t)0x04000000)
#define TSI578_GLOB_INT_ENABLE_RCS_EN                    ((uint32_t)0x08000000)

/* TSI578_GLOB_DEV_ID_SEL : Register Bits Masks Definitions */
#define TSI578_GLOB_DEV_ID_SEL_SEL                       ((uint32_t)0x00000003)

/* TSI578_GLOB_SPY_BUS_CFG : Register Bits Masks Definitions */
#define TSI578_GLOB_SPY_BUS_CFG_EN                       ((uint32_t)0x00000001)
#define TSI578_GLOB_SPY_BUS_CFG_CTRL0                    ((uint32_t)0x000f0000)
#define TSI578_GLOB_SPY_BUS_CFG_CTRL1                    ((uint32_t)0x0f000000)

/* TSI578_RIO_PW_TIMEOUT : Register Bits Masks Definitions */
#define TSI578_RIO_PW_TIMEOUT_PW_TIMER                   ((uint32_t)0xf0000000)
#define TSI578_RIO_PW_TIMEOUT_DISABLE                    ((uint32_t)0x00000000)
#define TSI578_RIO_PW_TIMEOUT_167P7MS                    ((uint32_t)0x10000000)
#define TSI578_RIO_PW_TIMEOUT_335P5MS                    ((uint32_t)0x20000000)
#define TSI578_RIO_PW_TIMEOUT_671P1MS                    ((uint32_t)0x40000000)
#define TSI578_RIO_PW_TIMEOUT_1340MS                     ((uint32_t)0x80000000)

/* TSI578_RIO_PW_OREQ_STATUS : Register Bits Masks Definitions */
#define TSI578_RIO_PW_OREQ_STATUS_PORTX_OREG             ((uint32_t)0x0000ffff)

/* TSI578_RIOX_MC_REG_VER : Register Bits Masks Definitions */
#define TSI578_RIOX_MC_REG_VER_REG_VER                   ((uint32_t)0x000000ff)

/* TSI578_RIOX_MC_LAT_LIMIT : Register Bits Masks Definitions */
#define TSI578_RIOX_MC_LAT_LIMIT_MAX_MC_LAT              ((uint32_t)0x00ffffff)
#define TSI578_RIOX_MC_LAT_LIMIT_AUTODEAD                ((uint32_t)0x80000000)

/* TSI578_SPX_ISF_WM : Register Bits Masks Definitions */
#define TSI578_SPX_ISF_WM_PRIO0WM                        ((uint32_t)0x00000007)
#define TSI578_SPX_ISF_WM_PRIO1WM                        ((uint32_t)0x00000700)
#define TSI578_SPX_ISF_WM_PRIO2WM                        ((uint32_t)0x00070000)

/* TSI578_SPX_WRR_0 : Register Bits Masks Definitions */
#define TSI578_SPX_WRR_0_WEIGHT                          ((uint32_t)0x0000000f)
#define TSI578_SPX_WRR_0_CHOOSE_UC                       ((uint32_t)0x01000000)
#define TSI578_SPX_WRR_0_WRR_EN                          ((uint32_t)0x02000000)

/* TSI578_SPX_WRR_1 : Register Bits Masks Definitions */
#define TSI578_SPX_WRR_1_WEIGHT                          ((uint32_t)0x0000000f)
#define TSI578_SPX_WRR_1_CHOOSE_UC                       ((uint32_t)0x01000000)
#define TSI578_SPX_WRR_1_WRR_EN                          ((uint32_t)0x02000000)

/* TSI578_SPX_WRR_2 : Register Bits Masks Definitions */
#define TSI578_SPX_WRR_2_WEIGHT                          ((uint32_t)0x0000000f)
#define TSI578_SPX_WRR_2_CHOOSE_UC                       ((uint32_t)0x01000000)
#define TSI578_SPX_WRR_2_WRR_EN                          ((uint32_t)0x02000000)

/* TSI578_SPX_WRR_3 : Register Bits Masks Definitions */
#define TSI578_SPX_WRR_3_WEIGHT                          ((uint32_t)0x0000000f)
#define TSI578_SPX_WRR_3_CHOOSE_UC                       ((uint32_t)0x01000000)
#define TSI578_SPX_WRR_3_WRR_EN                          ((uint32_t)0x02000000)

/* TSI578_I2C_DEVID : Register Bits Masks Definitions */
#define TSI578_I2C_DEVID_REV                             ((uint32_t)0x0000000f)

/* TSI578_I2C_RESET : Register Bits Masks Definitions */
#define TSI578_I2C_RESET_SRESET                          ((uint32_t)0x80000000)

/* TSI578_I2C_MST_CFG : Register Bits Masks Definitions */
#define TSI578_I2C_MST_CFG_DEV_ADDR                      ((uint32_t)0x0000007f)
#define TSI578_I2C_MST_CFG_PA_SIZE                       ((uint32_t)0x00030000)
#define TSI578_I2C_MST_CFG_DORDER                        ((uint32_t)0x00800000)

/* TSI578_I2C_MST_CNTRL : Register Bits Masks Definitions */
#define TSI578_I2C_MST_CNTRL_PADDR                       ((uint32_t)0x0000ffff)
#define TSI578_I2C_MST_CNTRL_SIZE                        ((uint32_t)0x07000000)
#define TSI578_I2C_MST_CNTRL_WRITE                       ((uint32_t)0x40000000)
#define TSI578_I2C_MST_CNTRL_START                       ((uint32_t)0x80000000)

/* TSI578_I2C_MST_RDATA : Register Bits Masks Definitions */
#define TSI578_I2C_MST_RDATA_RBYTE0                      ((uint32_t)0x000000ff)
#define TSI578_I2C_MST_RDATA_RBYTE1                      ((uint32_t)0x0000ff00)
#define TSI578_I2C_MST_RDATA_RBYTE2                      ((uint32_t)0x00ff0000)
#define TSI578_I2C_MST_RDATA_RBYTE3                      ((uint32_t)0xff000000)

/* TSI578_I2C_MST_TDATA : Register Bits Masks Definitions */
#define TSI578_I2C_MST_TDATA_TBYTE0                      ((uint32_t)0x000000ff)
#define TSI578_I2C_MST_TDATA_TBYTE1                      ((uint32_t)0x0000ff00)
#define TSI578_I2C_MST_TDATA_TBYTE2                      ((uint32_t)0x00ff0000)
#define TSI578_I2C_MST_TDATA_TBYTE3                      ((uint32_t)0xff000000)

/* TSI578_I2C_ACC_STAT : Register Bits Masks Definitions */
#define TSI578_I2C_ACC_STAT_MST_NBYTES                   ((uint32_t)0x0000000f)
#define TSI578_I2C_ACC_STAT_MST_AN                       ((uint32_t)0x00000100)
#define TSI578_I2C_ACC_STAT_MST_PHASE                    ((uint32_t)0x00000e00)
#define TSI578_I2C_ACC_STAT_MST_ACTIVE                   ((uint32_t)0x00008000)
#define TSI578_I2C_ACC_STAT_SLV_PA                       ((uint32_t)0x00ff0000)
#define TSI578_I2C_ACC_STAT_SLV_AN                       ((uint32_t)0x01000000)
#define TSI578_I2C_ACC_STAT_SLV_PHASE                    ((uint32_t)0x06000000)
#define TSI578_I2C_ACC_STAT_SLV_WAIT                     ((uint32_t)0x08000000)
#define TSI578_I2C_ACC_STAT_BUS_ACTIVE                   ((uint32_t)0x40000000)
#define TSI578_I2C_ACC_STAT_SLV_ACTIVE                   ((uint32_t)0x80000000)

/* TSI578_I2C_INT_STAT : Register Bits Masks Definitions */
#define TSI578_I2C_INT_STAT_MA_OK                        ((uint32_t)0x00000001)
#define TSI578_I2C_INT_STAT_MA_ATMO                      ((uint32_t)0x00000002)
#define TSI578_I2C_INT_STAT_MA_NACK                      ((uint32_t)0x00000004)
#define TSI578_I2C_INT_STAT_MA_TMO                       ((uint32_t)0x00000008)
#define TSI578_I2C_INT_STAT_MA_COL                       ((uint32_t)0x00000010)
#define TSI578_I2C_INT_STAT_MA_DIAG                      ((uint32_t)0x00000080)
#define TSI578_I2C_INT_STAT_SA_OK                        ((uint32_t)0x00000100)
#define TSI578_I2C_INT_STAT_SA_READ                      ((uint32_t)0x00000200)
#define TSI578_I2C_INT_STAT_SA_WRITE                     ((uint32_t)0x00000400)
#define TSI578_I2C_INT_STAT_SA_FAIL                      ((uint32_t)0x00000800)
#define TSI578_I2C_INT_STAT_BL_OK                        ((uint32_t)0x00010000)
#define TSI578_I2C_INT_STAT_BL_FAIL                      ((uint32_t)0x00020000)
#define TSI578_I2C_INT_STAT_IMB_FULL                     ((uint32_t)0x01000000)
#define TSI578_I2C_INT_STAT_OMB_EMPTY                    ((uint32_t)0x02000000)

/* TSI578_I2C_INT_ENABLE : Register Bits Masks Definitions */
#define TSI578_I2C_INT_ENABLE_MA_OK                      ((uint32_t)0x00000001)
#define TSI578_I2C_INT_ENABLE_MA_ATMO                    ((uint32_t)0x00000002)
#define TSI578_I2C_INT_ENABLE_MA_NACK                    ((uint32_t)0x00000004)
#define TSI578_I2C_INT_ENABLE_MA_TMO                     ((uint32_t)0x00000008)
#define TSI578_I2C_INT_ENABLE_MA_COL                     ((uint32_t)0x00000010)
#define TSI578_I2C_INT_ENABLE_MA_DIAG                    ((uint32_t)0x00000080)
#define TSI578_I2C_INT_ENABLE_SA_OK                      ((uint32_t)0x00000100)
#define TSI578_I2C_INT_ENABLE_SA_READ                    ((uint32_t)0x00000200)
#define TSI578_I2C_INT_ENABLE_SA_WRITE                   ((uint32_t)0x00000400)
#define TSI578_I2C_INT_ENABLE_SA_FAIL                    ((uint32_t)0x00000800)
#define TSI578_I2C_INT_ENABLE_BL_OK                      ((uint32_t)0x00010000)
#define TSI578_I2C_INT_ENABLE_BL_FAIL                    ((uint32_t)0x00020000)
#define TSI578_I2C_INT_ENABLE_IMB_FULL                   ((uint32_t)0x01000000)
#define TSI578_I2C_INT_ENABLE_OMB_EMPTY                  ((uint32_t)0x02000000)

/* TSI578_I2C_INT_SET : Register Bits Masks Definitions */
#define TSI578_I2C_INT_SET_MA_OK                         ((uint32_t)0x00000001)
#define TSI578_I2C_INT_SET_MA_ATMO                       ((uint32_t)0x00000002)
#define TSI578_I2C_INT_SET_MA_NACK                       ((uint32_t)0x00000004)
#define TSI578_I2C_INT_SET_MA_TMO                        ((uint32_t)0x00000008)
#define TSI578_I2C_INT_SET_MA_COL                        ((uint32_t)0x00000010)
#define TSI578_I2C_INT_SET_MA_DIAG                       ((uint32_t)0x00000080)
#define TSI578_I2C_INT_SET_SA_OK                         ((uint32_t)0x00000100)
#define TSI578_I2C_INT_SET_SA_READ                       ((uint32_t)0x00000200)
#define TSI578_I2C_INT_SET_SA_WRITE                      ((uint32_t)0x00000400)
#define TSI578_I2C_INT_SET_SA_FAIL                       ((uint32_t)0x00000800)
#define TSI578_I2C_INT_SET_BL_OK                         ((uint32_t)0x00010000)
#define TSI578_I2C_INT_SET_BL_FAIL                       ((uint32_t)0x00020000)
#define TSI578_I2C_INT_SET_IMB_FULL                      ((uint32_t)0x01000000)
#define TSI578_I2C_INT_SET_OMB_EMPTY                     ((uint32_t)0x02000000)

/* TSI578_I2C_SLV_CFG : Register Bits Masks Definitions */
#define TSI578_I2C_SLV_CFG_SLV_ADDR                      ((uint32_t)0x0000007f)
#define TSI578_I2C_SLV_CFG_SLV_UNLK                      ((uint32_t)0x01000000)
#define TSI578_I2C_SLV_CFG_SLV_EN                        ((uint32_t)0x10000000)
#define TSI578_I2C_SLV_CFG_ALRT_EN                       ((uint32_t)0x20000000)
#define TSI578_I2C_SLV_CFG_WR_EN                         ((uint32_t)0x40000000)
#define TSI578_I2C_SLV_CFG_RD_EN                         ((uint32_t)0x80000000)

/* TSI578_I2C_BOOT_CNTRL : Register Bits Masks Definitions */
#define TSI578_I2C_BOOT_CNTRL_PADDR                      ((uint32_t)0x00001fff)
#define TSI578_I2C_BOOT_CNTRL_PAGE_MODE                  ((uint32_t)0x0000e000)
#define TSI578_I2C_BOOT_CNTRL_BOOT_ADDR                  ((uint32_t)0x007f0000)
#define TSI578_I2C_BOOT_CNTRL_BUNLK                      ((uint32_t)0x10000000)
#define TSI578_I2C_BOOT_CNTRL_BINC                       ((uint32_t)0x20000000)
#define TSI578_I2C_BOOT_CNTRL_PSIZE                      ((uint32_t)0x40000000)
#define TSI578_I2C_BOOT_CNTRL_CHAIN                      ((uint32_t)0x80000000)

/* TSI578_EXI2C_REG_WADDR : Register Bits Masks Definitions */
#define TSI578_EXI2C_REG_WADDR_ADDR                      ((uint32_t)0xfffffffc)

/* TSI578_EXI2C_REG_WDATA : Register Bits Masks Definitions */
#define TSI578_EXI2C_REG_WDATA_WDATA                     ((uint32_t)0xffffffff)

/* TSI578_EXI2C_REG_RADDR : Register Bits Masks Definitions */
#define TSI578_EXI2C_REG_RADDR_ADDR                      ((uint32_t)0xfffffffc)

/* TSI578_EXI2C_REG_RDATA : Register Bits Masks Definitions */
#define TSI578_EXI2C_REG_RDATA_RDATA                     ((uint32_t)0xffffffff)

/* TSI578_EXI2C_ACC_STAT : Register Bits Masks Definitions */
#define TSI578_EXI2C_ACC_STAT_ALERT_FLAG                 ((uint32_t)0x00000001)
#define TSI578_EXI2C_ACC_STAT_IMB_FLAG                   ((uint32_t)0x00000004)
#define TSI578_EXI2C_ACC_STAT_OMB_FLAG                   ((uint32_t)0x00000008)
#define TSI578_EXI2C_ACC_STAT_ACC_OK                     ((uint32_t)0x00000080)

/* TSI578_EXI2C_ACC_CNTRL : Register Bits Masks Definitions */
#define TSI578_EXI2C_ACC_CNTRL_WINC                      ((uint32_t)0x00000004)
#define TSI578_EXI2C_ACC_CNTRL_RINC                      ((uint32_t)0x00000008)
#define TSI578_EXI2C_ACC_CNTRL_WSIZE                     ((uint32_t)0x00000030)
#define TSI578_EXI2C_ACC_CNTRL_RSIZE                     ((uint32_t)0x000000c0)

/* TSI578_EXI2C_STAT : Register Bits Masks Definitions */
#define TSI578_EXI2C_STAT_PORT0                          ((uint32_t)0x00000001)
#define TSI578_EXI2C_STAT_PORT1                          ((uint32_t)0x00000002)
#define TSI578_EXI2C_STAT_PORT2                          ((uint32_t)0x00000004)
#define TSI578_EXI2C_STAT_PORT3                          ((uint32_t)0x00000008)
#define TSI578_EXI2C_STAT_PORT4                          ((uint32_t)0x00000010)
#define TSI578_EXI2C_STAT_PORT5                          ((uint32_t)0x00000020)
#define TSI578_EXI2C_STAT_PORT6                          ((uint32_t)0x00000040)
#define TSI578_EXI2C_STAT_PORT7                          ((uint32_t)0x00000080)
#define TSI578_EXI2C_STAT_PORT8                          ((uint32_t)0x00000100)
#define TSI578_EXI2C_STAT_PORT9                          ((uint32_t)0x00000200)
#define TSI578_EXI2C_STAT_PORT10                         ((uint32_t)0x00000400)
#define TSI578_EXI2C_STAT_PORT11                         ((uint32_t)0x00000800)
#define TSI578_EXI2C_STAT_PORT12                         ((uint32_t)0x00001000)
#define TSI578_EXI2C_STAT_PORT13                         ((uint32_t)0x00002000)
#define TSI578_EXI2C_STAT_PORT14                         ((uint32_t)0x00004000)
#define TSI578_EXI2C_STAT_PORT15                         ((uint32_t)0x00008000)
#define TSI578_EXI2C_STAT_MCE                            ((uint32_t)0x00010000)
#define TSI578_EXI2C_STAT_MC_LAT                         ((uint32_t)0x00020000)
#define TSI578_EXI2C_STAT_LOGICAL                        ((uint32_t)0x00040000)
#define TSI578_EXI2C_STAT_MCS                            ((uint32_t)0x00400000)
#define TSI578_EXI2C_STAT_RCS                            ((uint32_t)0x00800000)
#define TSI578_EXI2C_STAT_TEA                            ((uint32_t)0x01000000)
#define TSI578_EXI2C_STAT_I2C                            ((uint32_t)0x02000000)
#define TSI578_EXI2C_STAT_IMBR                           ((uint32_t)0x04000000)
#define TSI578_EXI2C_STAT_OMBW                           ((uint32_t)0x08000000)
#define TSI578_EXI2C_STAT_SW_STAT0                       ((uint32_t)0x10000000)
#define TSI578_EXI2C_STAT_SW_STAT1                       ((uint32_t)0x20000000)
#define TSI578_EXI2C_STAT_SW_STAT2                       ((uint32_t)0x40000000)
#define TSI578_EXI2C_STAT_RESET                          ((uint32_t)0x80000000)

/* TSI578_EXI2C_STAT_ENABLE : Register Bits Masks Definitions */
#define TSI578_EXI2C_STAT_ENABLE_PORT0                   ((uint32_t)0x00000001)
#define TSI578_EXI2C_STAT_ENABLE_PORT1                   ((uint32_t)0x00000002)
#define TSI578_EXI2C_STAT_ENABLE_PORT2                   ((uint32_t)0x00000004)
#define TSI578_EXI2C_STAT_ENABLE_PORT3                   ((uint32_t)0x00000008)
#define TSI578_EXI2C_STAT_ENABLE_PORT4                   ((uint32_t)0x00000010)
#define TSI578_EXI2C_STAT_ENABLE_PORT5                   ((uint32_t)0x00000020)
#define TSI578_EXI2C_STAT_ENABLE_PORT6                   ((uint32_t)0x00000040)
#define TSI578_EXI2C_STAT_ENABLE_PORT7                   ((uint32_t)0x00000080)
#define TSI578_EXI2C_STAT_ENABLE_PORT8                   ((uint32_t)0x00000100)
#define TSI578_EXI2C_STAT_ENABLE_PORT9                   ((uint32_t)0x00000200)
#define TSI578_EXI2C_STAT_ENABLE_PORT10                  ((uint32_t)0x00000400)
#define TSI578_EXI2C_STAT_ENABLE_PORT11                  ((uint32_t)0x00000800)
#define TSI578_EXI2C_STAT_ENABLE_PORT12                  ((uint32_t)0x00001000)
#define TSI578_EXI2C_STAT_ENABLE_PORT13                  ((uint32_t)0x00002000)
#define TSI578_EXI2C_STAT_ENABLE_PORT14                  ((uint32_t)0x00004000)
#define TSI578_EXI2C_STAT_ENABLE_PORT15                  ((uint32_t)0x00008000)
#define TSI578_EXI2C_STAT_ENABLE_MCE                     ((uint32_t)0x00010000)
#define TSI578_EXI2C_STAT_ENABLE_MC_LAT                  ((uint32_t)0x00020000)
#define TSI578_EXI2C_STAT_ENABLE_LOGICAL                 ((uint32_t)0x00040000)
#define TSI578_EXI2C_STAT_ENABLE_MCS                     ((uint32_t)0x00400000)
#define TSI578_EXI2C_STAT_ENABLE_RCS                     ((uint32_t)0x00800000)
#define TSI578_EXI2C_STAT_ENABLE_TEA                     ((uint32_t)0x01000000)
#define TSI578_EXI2C_STAT_ENABLE_I2C                     ((uint32_t)0x02000000)
#define TSI578_EXI2C_STAT_ENABLE_IMBR                    ((uint32_t)0x04000000)
#define TSI578_EXI2C_STAT_ENABLE_OMBW                    ((uint32_t)0x08000000)
#define TSI578_EXI2C_STAT_ENABLE_SW_STAT0                ((uint32_t)0x10000000)
#define TSI578_EXI2C_STAT_ENABLE_SW_STAT1                ((uint32_t)0x20000000)
#define TSI578_EXI2C_STAT_ENABLE_SW_STAT2                ((uint32_t)0x40000000)
#define TSI578_EXI2C_STAT_ENABLE_RESET                   ((uint32_t)0x80000000)

/* TSI578_EXI2C_MBOX_OUT : Register Bits Masks Definitions */
#define TSI578_EXI2C_MBOX_OUT_DATA                       ((uint32_t)0xffffffff)

/* TSI578_EXI2C_MBOX_IN : Register Bits Masks Definitions */
#define TSI578_EXI2C_MBOX_IN_DATA                        ((uint32_t)0xffffffff)

/* TSI578_I2C_X : Register Bits Masks Definitions */
#define TSI578_I2C_X_MARBTO                              ((uint32_t)0x00000001)
#define TSI578_I2C_X_MSCLTO                              ((uint32_t)0x00000002)
#define TSI578_I2C_X_MBTTO                               ((uint32_t)0x00000004)
#define TSI578_I2C_X_MTRTO                               ((uint32_t)0x00000008)
#define TSI578_I2C_X_MCOL                                ((uint32_t)0x00000010)
#define TSI578_I2C_X_MNACK                               ((uint32_t)0x00000020)
#define TSI578_I2C_X_BLOK                                ((uint32_t)0x00000100)
#define TSI578_I2C_X_BLNOD                               ((uint32_t)0x00000200)
#define TSI578_I2C_X_BLSZ                                ((uint32_t)0x00000400)
#define TSI578_I2C_X_BLERR                               ((uint32_t)0x00000800)
#define TSI578_I2C_X_BLTO                                ((uint32_t)0x00001000)
#define TSI578_I2C_X_MTD                                 ((uint32_t)0x00004000)
#define TSI578_I2C_X_SSCLTO                              ((uint32_t)0x00020000)
#define TSI578_I2C_X_SBTTO                               ((uint32_t)0x00040000)
#define TSI578_I2C_X_STRTO                               ((uint32_t)0x00080000)
#define TSI578_I2C_X_SCOL                                ((uint32_t)0x00100000)
#define TSI578_I2C_X_OMBR                                ((uint32_t)0x00400000)
#define TSI578_I2C_X_IMBW                                ((uint32_t)0x00800000)
#define TSI578_I2C_X_DCMDD                               ((uint32_t)0x01000000)
#define TSI578_I2C_X_DHIST                               ((uint32_t)0x02000000)
#define TSI578_I2C_X_DTIMER                              ((uint32_t)0x04000000)
#define TSI578_I2C_X_SD                                  ((uint32_t)0x10000000)
#define TSI578_I2C_X_SDR                                 ((uint32_t)0x20000000)
#define TSI578_I2C_X_SDW                                 ((uint32_t)0x40000000)

/* TSI578_I2C_NEW_EVENT : Register Bits Masks Definitions */
#define TSI578_I2C_NEW_EVENT_MARBTO                      ((uint32_t)0x00000001)
#define TSI578_I2C_NEW_EVENT_MSCLTO                      ((uint32_t)0x00000002)
#define TSI578_I2C_NEW_EVENT_MBTTO                       ((uint32_t)0x00000004)
#define TSI578_I2C_NEW_EVENT_MTRTO                       ((uint32_t)0x00000008)
#define TSI578_I2C_NEW_EVENT_MCOL                        ((uint32_t)0x00000010)
#define TSI578_I2C_NEW_EVENT_MNACK                       ((uint32_t)0x00000020)
#define TSI578_I2C_NEW_EVENT_BLOK                        ((uint32_t)0x00000100)
#define TSI578_I2C_NEW_EVENT_BLNOD                       ((uint32_t)0x00000200)
#define TSI578_I2C_NEW_EVENT_BLSZ                        ((uint32_t)0x00000400)
#define TSI578_I2C_NEW_EVENT_BLERR                       ((uint32_t)0x00000800)
#define TSI578_I2C_NEW_EVENT_BLTO                        ((uint32_t)0x00001000)
#define TSI578_I2C_NEW_EVENT_MTD                         ((uint32_t)0x00004000)
#define TSI578_I2C_NEW_EVENT_SSCLTO                      ((uint32_t)0x00020000)
#define TSI578_I2C_NEW_EVENT_SBTTO                       ((uint32_t)0x00040000)
#define TSI578_I2C_NEW_EVENT_STRTO                       ((uint32_t)0x00080000)
#define TSI578_I2C_NEW_EVENT_SCOL                        ((uint32_t)0x00100000)
#define TSI578_I2C_NEW_EVENT_OMBR                        ((uint32_t)0x00400000)
#define TSI578_I2C_NEW_EVENT_IMBW                        ((uint32_t)0x00800000)
#define TSI578_I2C_NEW_EVENT_DCMDD                       ((uint32_t)0x01000000)
#define TSI578_I2C_NEW_EVENT_DHIST                       ((uint32_t)0x02000000)
#define TSI578_I2C_NEW_EVENT_DTIMER                      ((uint32_t)0x04000000)
#define TSI578_I2C_NEW_EVENT_SD                          ((uint32_t)0x10000000)
#define TSI578_I2C_NEW_EVENT_SDR                         ((uint32_t)0x20000000)
#define TSI578_I2C_NEW_EVENT_SDW                         ((uint32_t)0x40000000)

/* TSI578_I2C_EVENT_ENB : Register Bits Masks Definitions */
#define TSI578_I2C_EVENT_ENB_MARBTO                      ((uint32_t)0x00000001)
#define TSI578_I2C_EVENT_ENB_MSCLTO                      ((uint32_t)0x00000002)
#define TSI578_I2C_EVENT_ENB_MBTTO                       ((uint32_t)0x00000004)
#define TSI578_I2C_EVENT_ENB_MTRTO                       ((uint32_t)0x00000008)
#define TSI578_I2C_EVENT_ENB_MCOL                        ((uint32_t)0x00000010)
#define TSI578_I2C_EVENT_ENB_MNACK                       ((uint32_t)0x00000020)
#define TSI578_I2C_EVENT_ENB_BLOK                        ((uint32_t)0x00000100)
#define TSI578_I2C_EVENT_ENB_BLNOD                       ((uint32_t)0x00000200)
#define TSI578_I2C_EVENT_ENB_BLSZ                        ((uint32_t)0x00000400)
#define TSI578_I2C_EVENT_ENB_BLERR                       ((uint32_t)0x00000800)
#define TSI578_I2C_EVENT_ENB_BLTO                        ((uint32_t)0x00001000)
#define TSI578_I2C_EVENT_ENB_MTD                         ((uint32_t)0x00004000)
#define TSI578_I2C_EVENT_ENB_SSCLTO                      ((uint32_t)0x00020000)
#define TSI578_I2C_EVENT_ENB_SBTTO                       ((uint32_t)0x00040000)
#define TSI578_I2C_EVENT_ENB_STRTO                       ((uint32_t)0x00080000)
#define TSI578_I2C_EVENT_ENB_SCOL                        ((uint32_t)0x00100000)
#define TSI578_I2C_EVENT_ENB_OMBR                        ((uint32_t)0x00400000)
#define TSI578_I2C_EVENT_ENB_IMBW                        ((uint32_t)0x00800000)
#define TSI578_I2C_EVENT_ENB_DCMDD                       ((uint32_t)0x01000000)
#define TSI578_I2C_EVENT_ENB_DHIST                       ((uint32_t)0x02000000)
#define TSI578_I2C_EVENT_ENB_DTIMER                      ((uint32_t)0x04000000)
#define TSI578_I2C_EVENT_ENB_SD                          ((uint32_t)0x10000000)
#define TSI578_I2C_EVENT_ENB_SDR                         ((uint32_t)0x20000000)
#define TSI578_I2C_EVENT_ENB_SDW                         ((uint32_t)0x40000000)

/* TSI578_I2C_DIVIDER : Register Bits Masks Definitions */
#define TSI578_I2C_DIVIDER_MSDIV                         ((uint32_t)0x00000fff)
#define TSI578_I2C_DIVIDER_USDIV                         ((uint32_t)0x0fff0000)

/* TSI578_I2C_FILTER_SCL_CFG : Register Bits Masks Definitions */
#define TSI578_I2C_FILTER_SCL_CFG_END_TAP                ((uint32_t)0x00001f00)
#define TSI578_I2C_FILTER_SCL_CFG_THRES0                 ((uint32_t)0x001f0000)
#define TSI578_I2C_FILTER_SCL_CFG_THRES1                 ((uint32_t)0x1f000000)

/* TSI578_I2C_FILTER_SDA_CFG : Register Bits Masks Definitions */
#define TSI578_I2C_FILTER_SDA_CFG_END_TAP                ((uint32_t)0x00001f00)
#define TSI578_I2C_FILTER_SDA_CFG_THRES0                 ((uint32_t)0x001f0000)
#define TSI578_I2C_FILTER_SDA_CFG_THRES1                 ((uint32_t)0x1f000000)

/* TSI578_I2C_START_SETUP_HOLD : Register Bits Masks Definitions */
#define TSI578_I2C_START_SETUP_HOLD_START_HOLD           ((uint32_t)0x0000ffff)
#define TSI578_I2C_START_SETUP_HOLD_START_SETUP          ((uint32_t)0xffff0000)

/* TSI578_I2C_STOP_IDLE : Register Bits Masks Definitions */
#define TSI578_I2C_STOP_IDLE_IDLE_DET                    ((uint32_t)0x0000ffff)
#define TSI578_I2C_STOP_IDLE_STOP_SETUP                  ((uint32_t)0xffff0000)

/* TSI578_I2C_SDA_SETUP_HOLD : Register Bits Masks Definitions */
#define TSI578_I2C_SDA_SETUP_HOLD_SDA_HOLD               ((uint32_t)0x0000ffff)
#define TSI578_I2C_SDA_SETUP_HOLD_SDA_SETUP              ((uint32_t)0xffff0000)

/* TSI578_I2C_SCL_PERIOD : Register Bits Masks Definitions */
#define TSI578_I2C_SCL_PERIOD_SCL_LOW                    ((uint32_t)0x0000ffff)
#define TSI578_I2C_SCL_PERIOD_SCL_HIGH                   ((uint32_t)0xffff0000)

/* TSI578_I2C_SCL_MIN_PERIOD : Register Bits Masks Definitions */
#define TSI578_I2C_SCL_MIN_PERIOD_SCL_MINL               ((uint32_t)0x0000ffff)
#define TSI578_I2C_SCL_MIN_PERIOD_SCL_MINH               ((uint32_t)0xffff0000)

/* TSI578_I2C_SCL_ARB_TIMEOUT : Register Bits Masks Definitions */
#define TSI578_I2C_SCL_ARB_TIMEOUT_ARB_TO                ((uint32_t)0x0000ffff)
#define TSI578_I2C_SCL_ARB_TIMEOUT_SCL_TO                ((uint32_t)0xffff0000)

/* TSI578_I2C_BYTE_TRAN_TIMEOUT : Register Bits Masks Definitions */
#define TSI578_I2C_BYTE_TRAN_TIMEOUT_TRAN_TO             ((uint32_t)0x0000ffff)
#define TSI578_I2C_BYTE_TRAN_TIMEOUT_BYTE_TO             ((uint32_t)0xffff0000)

/* TSI578_I2C_BOOT_DIAG_TIMER : Register Bits Masks Definitions */
#define TSI578_I2C_BOOT_DIAG_TIMER_COUNT                 ((uint32_t)0x0000ffff)
#define TSI578_I2C_BOOT_DIAG_TIMER_FREERUN               ((uint32_t)0x80000000)

/* TSI578_I2C_DIAG_FILTER_SCL : Register Bits Masks Definitions */
#define TSI578_I2C_DIAG_FILTER_SCL_SAMPLES               ((uint32_t)0xffffffff)

/* TSI578_I2C_DIAG_FILTER_SDA : Register Bits Masks Definitions */
#define TSI578_I2C_DIAG_FILTER_SDA_SAMPLES               ((uint32_t)0xffffffff)

/* TSI578_I2C_BOOT_DIAG_PROGRESS : Register Bits Masks Definitions */
#define TSI578_I2C_BOOT_DIAG_PROGRESS_PADDR              ((uint32_t)0x0000ffff)
#define TSI578_I2C_BOOT_DIAG_PROGRESS_REGCNT             ((uint32_t)0xffff0000)

/* TSI578_I2C_BOOT_DIAG_CFG : Register Bits Masks Definitions */
#define TSI578_I2C_BOOT_DIAG_CFG_BOOT_ADDR               ((uint32_t)0x0000007f)
#define TSI578_I2C_BOOT_DIAG_CFG_PINC                    ((uint32_t)0x10000000)
#define TSI578_I2C_BOOT_DIAG_CFG_PASIZE                  ((uint32_t)0x20000000)
#define TSI578_I2C_BOOT_DIAG_CFG_BDIS                    ((uint32_t)0x40000000)
#define TSI578_I2C_BOOT_DIAG_CFG_BOOTING                 ((uint32_t)0x80000000)

/* TSI578_I2C_DIAG_CNTRL : Register Bits Masks Definitions */
#define TSI578_I2C_DIAG_CNTRL_SDAB                       ((uint32_t)0x00000001)
#define TSI578_I2C_DIAG_CNTRL_SDAOV                      ((uint32_t)0x00000002)
#define TSI578_I2C_DIAG_CNTRL_SCLB                       ((uint32_t)0x00000004)
#define TSI578_I2C_DIAG_CNTRL_SCLOV                      ((uint32_t)0x00000008)
#define TSI578_I2C_DIAG_CNTRL_START_CMD                  ((uint32_t)0x00000100)
#define TSI578_I2C_DIAG_CNTRL_CMD                        ((uint32_t)0x00000e00)
#define TSI578_I2C_DIAG_CNTRL_NO_STRETCH                 ((uint32_t)0x00002000)
#define TSI578_I2C_DIAG_CNTRL_NOIDLE                     ((uint32_t)0x00004000)
#define TSI578_I2C_DIAG_CNTRL_NOFAIL                     ((uint32_t)0x00008000)
#define TSI578_I2C_DIAG_CNTRL_WRITE_DATA                 ((uint32_t)0x00ff0000)

/* TSI578_I2C_DIAG_STAT : Register Bits Masks Definitions */
#define TSI578_I2C_DIAG_STAT_SCLIN                       ((uint32_t)0x00000001)
#define TSI578_I2C_DIAG_STAT_SCLOUT                      ((uint32_t)0x00000002)
#define TSI578_I2C_DIAG_STAT_SDAIN                       ((uint32_t)0x00000004)
#define TSI578_I2C_DIAG_STAT_SDAOUT                      ((uint32_t)0x00000008)
#define TSI578_I2C_DIAG_STAT_IDSTATE                     ((uint32_t)0x000000e0)
#define TSI578_I2C_DIAG_STAT_READ_DATA                   ((uint32_t)0x0000ff00)
#define TSI578_I2C_DIAG_STAT_ACK                         ((uint32_t)0x00010000)
#define TSI578_I2C_DIAG_STAT_S_SCL                       ((uint32_t)0x00100000)
#define TSI578_I2C_DIAG_STAT_S_SDA                       ((uint32_t)0x00200000)
#define TSI578_I2C_DIAG_STAT_M_SCL                       ((uint32_t)0x00400000)
#define TSI578_I2C_DIAG_STAT_M_SDA                       ((uint32_t)0x00800000)
#define TSI578_I2C_DIAG_STAT_SDA_TIMER                   ((uint32_t)0x07000000)
#define TSI578_I2C_DIAG_STAT_SCLIT                       ((uint32_t)0x08000000)
#define TSI578_I2C_DIAG_STAT_SCLHT                       ((uint32_t)0x10000000)
#define TSI578_I2C_DIAG_STAT_SCLMHT                      ((uint32_t)0x20000000)
#define TSI578_I2C_DIAG_STAT_SLVSDA                      ((uint32_t)0x40000000)

/* TSI578_I2C_DIAG_HIST : Register Bits Masks Definitions */
#define TSI578_I2C_DIAG_HIST_COUNT                       ((uint32_t)0x0000000f)
#define TSI578_I2C_DIAG_HIST_H1                          ((uint32_t)0x00000030)
#define TSI578_I2C_DIAG_HIST_H2                          ((uint32_t)0x000000c0)
#define TSI578_I2C_DIAG_HIST_H3                          ((uint32_t)0x00000300)
#define TSI578_I2C_DIAG_HIST_H4                          ((uint32_t)0x00000c00)
#define TSI578_I2C_DIAG_HIST_H5                          ((uint32_t)0x00003000)
#define TSI578_I2C_DIAG_HIST_H6                          ((uint32_t)0x0000c000)
#define TSI578_I2C_DIAG_HIST_H7                          ((uint32_t)0x00030000)
#define TSI578_I2C_DIAG_HIST_H8                          ((uint32_t)0x000c0000)
#define TSI578_I2C_DIAG_HIST_H9                          ((uint32_t)0x00300000)
#define TSI578_I2C_DIAG_HIST_H10                         ((uint32_t)0x00c00000)
#define TSI578_I2C_DIAG_HIST_H11                         ((uint32_t)0x03000000)
#define TSI578_I2C_DIAG_HIST_H12                         ((uint32_t)0x0c000000)
#define TSI578_I2C_DIAG_HIST_H13                         ((uint32_t)0x30000000)
#define TSI578_I2C_DIAG_HIST_H14                         ((uint32_t)0xc0000000)

/* TSI578_I2C_DIAG_MST_FSM : Register Bits Masks Definitions */
#define TSI578_I2C_DIAG_MST_FSM_BIT_FSM                  ((uint32_t)0x0000001f)
#define TSI578_I2C_DIAG_MST_FSM_BYTE_FSM                 ((uint32_t)0x000001e0)
#define TSI578_I2C_DIAG_MST_FSM_PTOB_FSM                 ((uint32_t)0x00000e00)
#define TSI578_I2C_DIAG_MST_FSM_PROTO_FSM                ((uint32_t)0x0001f000)
#define TSI578_I2C_DIAG_MST_FSM_MST_FSM                  ((uint32_t)0x000e0000)
#define TSI578_I2C_DIAG_MST_FSM_BOOT_FSM                 ((uint32_t)0x00f00000)

/* TSI578_I2C_DIAG_SLV_FSM : Register Bits Masks Definitions */
#define TSI578_I2C_DIAG_SLV_FSM_BIT_FSM                  ((uint32_t)0x0000000f)
#define TSI578_I2C_DIAG_SLV_FSM_BYTE_FSM                 ((uint32_t)0x000000f0)
#define TSI578_I2C_DIAG_SLV_FSM_PTOB_FSM                 ((uint32_t)0x00000700)
#define TSI578_I2C_DIAG_SLV_FSM_PROTO_FSM                ((uint32_t)0x00007800)

/* TSI578_I2C_DIAG_MST_SDA_SCL : Register Bits Masks Definitions */
#define TSI578_I2C_DIAG_MST_SDA_SCL_SCLTO_IDLE           ((uint32_t)0x0000ffff)
#define TSI578_I2C_DIAG_MST_SDA_SCL_SDA_CNT              ((uint32_t)0xffff0000)

/* TSI578_I2C_DIAG_MST_SCL_PER : Register Bits Masks Definitions */
#define TSI578_I2C_DIAG_MST_SCL_PER_SCL_MIN_PER          ((uint32_t)0x0000ffff)
#define TSI578_I2C_DIAG_MST_SCL_PER_SCL_PER              ((uint32_t)0xffff0000)

/* TSI578_I2C_DIAG_MST_ARB_BOOT : Register Bits Masks Definitions */
#define TSI578_I2C_DIAG_MST_ARB_BOOT_BOOT_CNT            ((uint32_t)0x0000ffff)
#define TSI578_I2C_DIAG_MST_ARB_BOOT_ARB_TO_CNT          ((uint32_t)0xffff0000)

/* TSI578_I2C_DIAG_MST_BYTE_TRAN : Register Bits Masks Definitions */
#define TSI578_I2C_DIAG_MST_BYTE_TRAN_TRAN_TO_CNT        ((uint32_t)0x0000ffff)
#define TSI578_I2C_DIAG_MST_BYTE_TRAN_BYTE_TO_CNT        ((uint32_t)0xffff0000)

/* TSI578_I2C_DIAG_SLV_SDA_SCL : Register Bits Masks Definitions */
#define TSI578_I2C_DIAG_SLV_SDA_SCL_SCLTO                ((uint32_t)0x0000ffff)
#define TSI578_I2C_DIAG_SLV_SDA_SCL_SDA_CNT              ((uint32_t)0xffff0000)

/* TSI578_I2C_DIAG_SLV_BYTE_TRAN : Register Bits Masks Definitions */
#define TSI578_I2C_DIAG_SLV_BYTE_TRAN_TRAN_TO_CNT        ((uint32_t)0x0000ffff)
#define TSI578_I2C_DIAG_SLV_BYTE_TRAN_BYTE_TO_CNT        ((uint32_t)0xffff0000)

/* TSI578_SMACX_TX_CTL_STAT_0 : Register Bits Masks Definitions */
#define TSI578_SMACX_TX_CTL_STAT_0_TX_CK0_EN             ((uint32_t)0x00010000)
#define TSI578_SMACX_TX_CTL_STAT_0_TX_EN                 ((uint32_t)0x000e0000)
#define TSI578_SMACX_TX_CTL_STAT_0_TX_CLK_ALIGN          ((uint32_t)0x00100000)
#define TSI578_SMACX_TX_CTL_STAT_0_TX_CALC               ((uint32_t)0x00200000)
#define TSI578_SMACX_TX_CTL_STAT_0_TX_BOOST              ((uint32_t)0x03c00000)
#define TSI578_SMACX_TX_CTL_STAT_0_TX_ATTEN              ((uint32_t)0x1c000000)
#define TSI578_SMACX_TX_CTL_STAT_0_TX_EDGERATE           ((uint32_t)0x60000000)

/* TSI578_SMACX_RX_STAT_0 : Register Bits Masks Definitions */
#define TSI578_SMACX_RX_STAT_0_HALF_RATE                 ((uint32_t)0x00000001)
#define TSI578_SMACX_RX_STAT_0_RX_PLL_PWRON              ((uint32_t)0x00000002)
#define TSI578_SMACX_RX_STAT_0_RX_EN                     ((uint32_t)0x00000004)
#define TSI578_SMACX_RX_STAT_0_RX_ALIGN_EN               ((uint32_t)0x00000008)
#define TSI578_SMACX_RX_STAT_0_RX_TERM_EN                ((uint32_t)0x00000010)
#define TSI578_SMACX_RX_STAT_0_RX_EQ_VAL                 ((uint32_t)0x000000e0)
#define TSI578_SMACX_RX_STAT_0_RX_DPLL_MODE              ((uint32_t)0x00000700)
#define TSI578_SMACX_RX_STAT_0_DPLL_RESET                ((uint32_t)0x00000800)
#define TSI578_SMACX_RX_STAT_0_LOS_CTL                   ((uint32_t)0x00003000)
#define TSI578_SMACX_RX_STAT_0_RX_VALID                  ((uint32_t)0x00010000)
#define TSI578_SMACX_RX_STAT_0_RX_PLL_STATE              ((uint32_t)0x00020000)
#define TSI578_SMACX_RX_STAT_0_LOS                       ((uint32_t)0x00040000)
#define TSI578_SMACX_RX_STAT_0_TX_DONE                   ((uint32_t)0x00080000)
#define TSI578_SMACX_RX_STAT_0_TX_RXPRES                 ((uint32_t)0x00100000)

/* TSI578_SMACX_TX_RX_CTL_STAT_0 : Register Bits Masks Definitions */
#define TSI578_SMACX_TX_RX_CTL_STAT_0_TX_CK0_EN          ((uint32_t)0x00000001)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_TX_EN              ((uint32_t)0x0000000e)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_TX_CLK_ALIGN       ((uint32_t)0x00000010)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_TX_CALC            ((uint32_t)0x00000020)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_TX_BOOST           ((uint32_t)0x000003c0)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_TX_ATTEN           ((uint32_t)0x00001c00)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_TX_EDGERATE        ((uint32_t)0x00006000)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_OVRD_1             ((uint32_t)0x00008000)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_HALF_RATE          ((uint32_t)0x00010000)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_RX_PLL_PWRON       ((uint32_t)0x00020000)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_RX_EN              ((uint32_t)0x00040000)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_RX_ALIGN_EN        ((uint32_t)0x00080000)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_RX_TERM_EN         ((uint32_t)0x00100000)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_RX_EQ_VAL          ((uint32_t)0x00e00000)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_RX_DPLL_MODE       ((uint32_t)0x07000000)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_DPLL_RESET         ((uint32_t)0x08000000)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_LOS_CTL            ((uint32_t)0x30000000)
#define TSI578_SMACX_TX_RX_CTL_STAT_0_OVRD_0             ((uint32_t)0x40000000)

/* TSI578_SMACX_RX_CTL_STAT_0 : Register Bits Masks Definitions */
#define TSI578_SMACX_RX_CTL_STAT_0_RX_VALID              ((uint32_t)0x00000001)
#define TSI578_SMACX_RX_CTL_STAT_0_RX_PLL_STATE          ((uint32_t)0x00000002)
#define TSI578_SMACX_RX_CTL_STAT_0_LOS                   ((uint32_t)0x00000004)
#define TSI578_SMACX_RX_CTL_STAT_0_TX_DONE               ((uint32_t)0x00000008)
#define TSI578_SMACX_RX_CTL_STAT_0_TX_RXPRES             ((uint32_t)0x00000010)
#define TSI578_SMACX_RX_CTL_STAT_0_OVRD                  ((uint32_t)0x00000020)
#define TSI578_SMACX_RX_CTL_STAT_0_ZERO_TX_DATA          ((uint32_t)0x00010000)
#define TSI578_SMACX_RX_CTL_STAT_0_ZERO_RX_DATA          ((uint32_t)0x00020000)
#define TSI578_SMACX_RX_CTL_STAT_0_INVERT_TX             ((uint32_t)0x00040000)
#define TSI578_SMACX_RX_CTL_STAT_0_INVERT_RX             ((uint32_t)0x00080000)
#define TSI578_SMACX_RX_CTL_STAT_0_DISABLE_RX_CK         ((uint32_t)0x00100000)
#define TSI578_SMACX_RX_CTL_STAT_0_DTB_SEL0              ((uint32_t)0x03e00000)
#define TSI578_SMACX_RX_CTL_STAT_0_DTB_SEL1              ((uint32_t)0x7c000000)

/* TSI578_SMACX_PG_CTL_0 : Register Bits Masks Definitions */
#define TSI578_SMACX_PG_CTL_0_MODE                       ((uint32_t)0x00000007)
#define TSI578_SMACX_PG_CTL_0_TRIGGER_ERR                ((uint32_t)0x00000008)
#define TSI578_SMACX_PG_CTL_0_PAT0                       ((uint32_t)0x00003ff0)

/* TSI578_SMACX_PM_CTL_0 : Register Bits Masks Definitions */
#define TSI578_SMACX_PM_CTL_0_MODE                       ((uint32_t)0x00000007)
#define TSI578_SMACX_PM_CTL_0_SYNC                       ((uint32_t)0x00000008)
#define TSI578_SMACX_PM_CTL_0_COUNT                      ((uint32_t)0x7fff0000)
#define TSI578_SMACX_PM_CTL_0_OV14                       ((uint32_t)0x80000000)

/* TSI578_SMACX_FP_VAL_0 : Register Bits Masks Definitions */
#define TSI578_SMACX_FP_VAL_0_DTHR_1                     ((uint32_t)0x00000001)
#define TSI578_SMACX_FP_VAL_0_PVAL                       ((uint32_t)0x000007fe)
#define TSI578_SMACX_FP_VAL_0_DTHR_0                     ((uint32_t)0x00010000)
#define TSI578_SMACX_FP_VAL_0_FVAL                       ((uint32_t)0x3ffe0000)

/* TSI578_SMACX_SCP_RX_CTL_0 : Register Bits Masks Definitions */
#define TSI578_SMACX_SCP_RX_CTL_0_MODE                   ((uint32_t)0x00000003)
#define TSI578_SMACX_SCP_RX_CTL_0_DELAY                  ((uint32_t)0x000007fc)
#define TSI578_SMACX_SCP_RX_CTL_0_BASE                   ((uint32_t)0x00007800)

/* TSI578_SMACX_RX_DBG_0 : Register Bits Masks Definitions */
#define TSI578_SMACX_RX_DBG_0_DTB_SEL0                   ((uint32_t)0x0000000f)
#define TSI578_SMACX_RX_DBG_0_DTB_SEL1                   ((uint32_t)0x000000f0)

/* TSI578_SMACX_RESET : Register Bits Masks Definitions */
#define TSI578_SMACX_RESET_SERDES_RESET_1                ((uint32_t)0x00000001)
#define TSI578_SMACX_RESET_SERDES_RESET_0                ((uint32_t)0x00010000)

/* TSI578_SMACX_CMP_A : Register Bits Masks Definitions */
#define TSI578_SMACX_CMP_A_CRCMP_GT_LIMIT                ((uint32_t)0x0000ffff)

/* TSI578_SMACX_CMP_B : Register Bits Masks Definitions */
#define TSI578_SMACX_CMP_B_S0_LOW                        ((uint32_t)0x00000001)
#define TSI578_SMACX_CMP_B_S0_HIGH                       ((uint32_t)0x00000002)
#define TSI578_SMACX_CMP_B_S1_S0_LOW                     ((uint32_t)0x00000004)
#define TSI578_SMACX_CMP_B_S1_S0_HIGH                    ((uint32_t)0x00000008)
#define TSI578_SMACX_CMP_B_S0_OUTSIDE                    ((uint32_t)0x00000010)
#define TSI578_SMACX_CMP_B_S1_S0_OUTSIDE                 ((uint32_t)0x00000020)
#define TSI578_SMACX_CMP_B_HOLD_SCRATCH_0                ((uint32_t)0x00010000)
#define TSI578_SMACX_CMP_B_HOLD_SCRATCH_1                ((uint32_t)0x00020000)

/* TSI578_SMACX_SCP : Register Bits Masks Definitions */
#define TSI578_SMACX_SCP_SCOPE_SAMPLES                   ((uint32_t)0x0000ffff)
#define TSI578_SMACX_SCP_SCOPE_COUNT                     ((uint32_t)0xffff0000)

/* TSI578_SMACX_DAC_RTUNE : Register Bits Masks Definitions */
#define TSI578_SMACX_DAC_RTUNE_DAC_VAL                   ((uint32_t)0x000003ff)
#define TSI578_SMACX_DAC_RTUNE_OVRD_RTUNE_TX             ((uint32_t)0x00000400)
#define TSI578_SMACX_DAC_RTUNE_OVRD_RTUNE_RX             ((uint32_t)0x00000800)
#define TSI578_SMACX_DAC_RTUNE_DAC_MODE                  ((uint32_t)0x00007000)
#define TSI578_SMACX_DAC_RTUNE_MODE                      ((uint32_t)0x00030000)
#define TSI578_SMACX_DAC_RTUNE_FRC_PWRON                 ((uint32_t)0x00040000)
#define TSI578_SMACX_DAC_RTUNE_PWRON_LCL                 ((uint32_t)0x00080000)
#define TSI578_SMACX_DAC_RTUNE_SEL_ATBP                  ((uint32_t)0x00100000)
#define TSI578_SMACX_DAC_RTUNE_RSC_X4                    ((uint32_t)0x00200000)
#define TSI578_SMACX_DAC_RTUNE_DAC_CHOP                  ((uint32_t)0x00400000)
#define TSI578_SMACX_DAC_RTUNE_CMP_INVERT                ((uint32_t)0x00800000)
#define TSI578_SMACX_DAC_RTUNE_RTUNE_DIS                 ((uint32_t)0x01000000)
#define TSI578_SMACX_DAC_RTUNE_RTUNE_TRIG                ((uint32_t)0x02000000)
#define TSI578_SMACX_DAC_RTUNE_ADC_TRIG                  ((uint32_t)0x04000000)

/* TSI578_SMACX_ADC : Register Bits Masks Definitions */
#define TSI578_SMACX_ADC_ADC_OUT                         ((uint32_t)0x000003ff)
#define TSI578_SMACX_ADC_FRESH                           ((uint32_t)0x00000400)

/* TSI578_SMACX_ID : Register Bits Masks Definitions */
#define TSI578_SMACX_ID_SERDES_ID_HI                     ((uint32_t)0x0000ffff)
#define TSI578_SMACX_ID_SERDES_ID_LO                     ((uint32_t)0xffff0000)

/* TSI578_SMACX_CTL_VAL : Register Bits Masks Definitions */
#define TSI578_SMACX_CTL_VAL_PROP_CTL                    ((uint32_t)0x00000007)
#define TSI578_SMACX_CTL_VAL_INT_CTL                     ((uint32_t)0x00000038)
#define TSI578_SMACX_CTL_VAL_NCY5                        ((uint32_t)0x000000c0)
#define TSI578_SMACX_CTL_VAL_NCY                         ((uint32_t)0x00001f00)
#define TSI578_SMACX_CTL_VAL_PRESCALE                    ((uint32_t)0x00006000)
#define TSI578_SMACX_CTL_VAL_USE_REFCLK_ALT              ((uint32_t)0x00010000)
#define TSI578_SMACX_CTL_VAL_MPLL_CLK_OFF                ((uint32_t)0x00020000)
#define TSI578_SMACX_CTL_VAL_MPLL_PWRON                  ((uint32_t)0x00040000)
#define TSI578_SMACX_CTL_VAL_MPLL_SS_EN                  ((uint32_t)0x00080000)
#define TSI578_SMACX_CTL_VAL_CKO_ALIVE_CON               ((uint32_t)0x00300000)
#define TSI578_SMACX_CTL_VAL_CKO_WORD_CON                ((uint32_t)0x01c00000)
#define TSI578_SMACX_CTL_VAL_RTUNE_DO_TUNE               ((uint32_t)0x04000000)
#define TSI578_SMACX_CTL_VAL_WIDE_XFACE                  ((uint32_t)0x08000000)
#define TSI578_SMACX_CTL_VAL_VPH_IS_3P3                  ((uint32_t)0x10000000)
#define TSI578_SMACX_CTL_VAL_VPH_IS_1P2                  ((uint32_t)0x20000000)
#define TSI578_SMACX_CTL_VAL_FAST_TECH                   ((uint32_t)0x40000000)

/* TSI578_SMACX_LVL_CTL_VAL : Register Bits Masks Definitions */
#define TSI578_SMACX_LVL_CTL_VAL_ACJT_LVL                ((uint32_t)0x0000001f)
#define TSI578_SMACX_LVL_CTL_VAL_LOS_LVL                 ((uint32_t)0x000003e0)
#define TSI578_SMACX_LVL_CTL_VAL_TX_LVL                  ((uint32_t)0x00007c00)
#define TSI578_SMACX_LVL_CTL_VAL_CR_READ                 ((uint32_t)0x00010000)
#define TSI578_SMACX_LVL_CTL_VAL_CR_WRITE                ((uint32_t)0x00020000)
#define TSI578_SMACX_LVL_CTL_VAL_CR_CAP_DATA             ((uint32_t)0x00040000)
#define TSI578_SMACX_LVL_CTL_VAL_CR_CAP_ADDR             ((uint32_t)0x00080000)
#define TSI578_SMACX_LVL_CTL_VAL_CR_ACK                  ((uint32_t)0x00200000)
#define TSI578_SMACX_LVL_CTL_VAL_POWER_GOOD              ((uint32_t)0x00400000)
#define TSI578_SMACX_LVL_CTL_VAL_OP_DONE                 ((uint32_t)0x00800000)

/* TSI578_SMACX_CTL_OVR : Register Bits Masks Definitions */
#define TSI578_SMACX_CTL_OVR_PROP_CTL                    ((uint32_t)0x00000007)
#define TSI578_SMACX_CTL_OVR_INT_CTL                     ((uint32_t)0x00000038)
#define TSI578_SMACX_CTL_OVR_NCY5                        ((uint32_t)0x000000c0)
#define TSI578_SMACX_CTL_OVR_NCY                         ((uint32_t)0x00001f00)
#define TSI578_SMACX_CTL_OVR_PRESCALE                    ((uint32_t)0x00006000)
#define TSI578_SMACX_CTL_OVR_OVRD                        ((uint32_t)0x00008000)
#define TSI578_SMACX_CTL_OVR_USE_REFCLK_ALT              ((uint32_t)0x00010000)
#define TSI578_SMACX_CTL_OVR_MPLL_CLK_OFF                ((uint32_t)0x00020000)
#define TSI578_SMACX_CTL_OVR_MPLL_PWRON                  ((uint32_t)0x00040000)
#define TSI578_SMACX_CTL_OVR_MPLL_SS_EN                  ((uint32_t)0x00080000)
#define TSI578_SMACX_CTL_OVR_CK0_ALIVE_CON               ((uint32_t)0x00300000)
#define TSI578_SMACX_CTL_OVR_CK0_WORD_CON                ((uint32_t)0x01c00000)
#define TSI578_SMACX_CTL_OVR_OVRD_CLK                    ((uint32_t)0x02000000)
#define TSI578_SMACX_CTL_OVR_RTUNE_DO_TUNE               ((uint32_t)0x04000000)
#define TSI578_SMACX_CTL_OVR_WIDE_XFACE                  ((uint32_t)0x08000000)
#define TSI578_SMACX_CTL_OVR_VPH_IS_3P3                  ((uint32_t)0x10000000)
#define TSI578_SMACX_CTL_OVR_VPH_IS_1P2                  ((uint32_t)0x20000000)
#define TSI578_SMACX_CTL_OVR_FAST_TECH                   ((uint32_t)0x40000000)
#define TSI578_SMACX_CTL_OVR_OVRD_STATIC                 ((uint32_t)0x80000000)

/* TSI578_SMACX_LVL_OVR : Register Bits Masks Definitions */
#define TSI578_SMACX_LVL_OVR_ACJT_LVL                    ((uint32_t)0x0000001f)
#define TSI578_SMACX_LVL_OVR_LOS_LVL                     ((uint32_t)0x000003e0)
#define TSI578_SMACX_LVL_OVR_TX_LVL                      ((uint32_t)0x00007c00)
#define TSI578_SMACX_LVL_OVR_OVERIDE                     ((uint32_t)0x00008000)
#define TSI578_SMACX_LVL_OVR_CR_READ                     ((uint32_t)0x00010000)
#define TSI578_SMACX_LVL_OVR_CR_WRITE                    ((uint32_t)0x00020000)
#define TSI578_SMACX_LVL_OVR_CR_CAP_DATA                 ((uint32_t)0x00040000)
#define TSI578_SMACX_LVL_OVR_CR_CAP_ADDR                 ((uint32_t)0x00080000)
#define TSI578_SMACX_LVL_OVR_OVRD_IN                     ((uint32_t)0x00100000)
#define TSI578_SMACX_LVL_OVR_CR_ACK                      ((uint32_t)0x00200000)
#define TSI578_SMACX_LVL_OVR_POWER_GOOD                  ((uint32_t)0x00400000)
#define TSI578_SMACX_LVL_OVR_OP_DONE                     ((uint32_t)0x00800000)
#define TSI578_SMACX_LVL_OVR_OVRD_OUT                    ((uint32_t)0x01000000)

/* TSI578_SMACX_MPLL_CTL : Register Bits Masks Definitions */
#define TSI578_SMACX_MPLL_CTL_CLKDRV_ANA                 ((uint32_t)0x00000001)
#define TSI578_SMACX_MPLL_CTL_CLKDRV_DIG                 ((uint32_t)0x00000002)
#define TSI578_SMACX_MPLL_CTL_OVRD_CLKDRV                ((uint32_t)0x00000004)
#define TSI578_SMACX_MPLL_CTL_DIS_PARA_CREG              ((uint32_t)0x00000008)
#define TSI578_SMACX_MPLL_CTL_REFCLK_DELAY               ((uint32_t)0x00000010)
#define TSI578_SMACX_MPLL_CTL_DTB_SEL0                   ((uint32_t)0x000003e0)
#define TSI578_SMACX_MPLL_CTL_DTB_SEL1                   ((uint32_t)0x00007c00)
#define TSI578_SMACX_MPLL_CTL_ATB_SENSE                  ((uint32_t)0x00010000)
#define TSI578_SMACX_MPLL_CTL_MEAS_GD                    ((uint32_t)0x00020000)
#define TSI578_SMACX_MPLL_CTL_MEAS_IV                    ((uint32_t)0x1ffc0000)
#define TSI578_SMACX_MPLL_CTL_RESET_VAL                  ((uint32_t)0x20000000)
#define TSI578_SMACX_MPLL_CTL_GEARSGIFT_VAL              ((uint32_t)0x40000000)
#define TSI578_SMACX_MPLL_CTL_OVRD_CTL                   ((uint32_t)0x80000000)


#endif /* __TSI578_H__ */
