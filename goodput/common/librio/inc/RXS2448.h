/*
****************************************************************************
Copyright (c) 2017, Integrated Device Technology Inc.
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

#ifndef __RXS2448_H__
#define __RXS2448_H__

#ifdef __cplusplus
extern "C" {
#endif

#define RXS2448_MAX_PORTS	24
#define RXS2448_MAX_LANES	48
#define RXS2448_MAX_SC		8
#define RXS2448_MAX_MC_MASK	0xFF
#define RXS2448_MC_MASK_CNT	(RXS2448_MAX_MC_MASK + 1)

#define RXS_MAX_L0_GROUP	1
#define RXS_MAX_L1_GROUP	3
#define RXS_MAX_L2_GROUP	4

#define FIRST_BYTE_MASK		((uint32_t)0x000000FF)
#define SECOND_BYTE_MASK	((uint32_t)0x0000FF00)
#define THIRD_BYTE_MASK		((uint32_t)0x00FF0000)
#define FOURTH_BYTE_MASK	((uint32_t)0xFF000000)

/*****************************************************/
/* RapidIO Register Address Offset Definitions       */
/*****************************************************/

#define RXS_DEV_ID                                   ((uint32_t)0x00000000)
#define RXS_DEV_INFO                                 ((uint32_t)0x00000004)
#define RXS_ASBLY_ID                                 ((uint32_t)0x00000008)
#define RXS_ASBLY_INFO                               ((uint32_t)0x0000000c)
#define RXS_PE_FEAT                                  ((uint32_t)0x00000010)
#define RXS_SW_PORT                                  ((uint32_t)0x00000014)
#define RXS_SRC_OP                                   ((uint32_t)0x00000018)
#define RXS_SW_RT_DEST_ID                            ((uint32_t)0x00000034)
#define RXS_PE_LL_CTL                                ((uint32_t)0x0000004c)
#define RXS_HOST_BASE_ID_LOCK                        ((uint32_t)0x00000068)
#define RXS_COMP_TAG                                 ((uint32_t)0x0000006c)
#define RXS_ROUTE_CFG_DESTID                         ((uint32_t)0x00000070)
#define RXS_ROUTE_CFG_PORT                           ((uint32_t)0x00000074)
#define RXS_ROUTE_DFLT_PORT                          ((uint32_t)0x00000078)
#define RXS_SP_MB_HEAD                               ((uint32_t)0x00000100)
#define RXS_SP_LT_CTL                                ((uint32_t)0x00000120)
#define RXS_SP_GEN_CTL                               ((uint32_t)0x0000013c)

#define RXS_SPX_PP_OSET				     ((uint32_t)0x40)
#define RXS_SPX_LM_REQ(X)         ((uint32_t)(0x0140 + (RXS_SPX_PP_OSET * (X))))
#define RXS_SPX_LM_RESP(X)        ((uint32_t)(0x0144 + (RXS_SPX_PP_OSET * (X))))
#define RXS_SPX_CTL2(X)           ((uint32_t)(0x0154 + (RXS_SPX_PP_OSET * (X))))
#define RXS_SPX_ERR_STAT(X)       ((uint32_t)(0x0158 + (RXS_SPX_PP_OSET * (X))))
#define RXS_SPX_CTL(X)            ((uint32_t)(0x015c + (RXS_SPX_PP_OSET * (X))))
#define RXS_SPX_OUT_ACKID_CSR(X)  ((uint32_t)(0x0160 + (RXS_SPX_PP_OSET * (X))))
#define RXS_SPX_IN_ACKID_CSR(X)   ((uint32_t)(0x0164 + (RXS_SPX_PP_OSET * (X))))
#define RXS_SPX_POWER_MNGT_CSR(X) ((uint32_t)(0x0168 + (RXS_SPX_PP_OSET * (X))))
#define RXS_SPX_LAT_OPT_CSR(X)    ((uint32_t)(0x016c + (RXS_SPX_PP_OSET * (X))))
#define RXS_SPX_TMR_CTL(X)        ((uint32_t)(0x0170 + (RXS_SPX_PP_OSET * (X))))
#define RXS_SPX_TMR_CTL2(X)       ((uint32_t)(0x0174 + (RXS_SPX_PP_OSET * (X))))
#define RXS_SPX_TMR_CTL3(X)       ((uint32_t)(0x0178 + (RXS_SPX_PP_OSET * (X))))
#define RXS_ERR_RPT_BH                               ((uint32_t)0x00001000)
#define RXS_ERR_MGMT_HS                              ((uint32_t)0x00001004)
#define RXS_ERR_DET                                  ((uint32_t)0x00001008)
#define RXS_ERR_EN                                   ((uint32_t)0x0000100c)
#define RXS_H_ADDR_CAPT                              ((uint32_t)0x00001010)
#define RXS_ADDR_CAPT                                ((uint32_t)0x00001014)
#define RXS_ID_CAPT                                  ((uint32_t)0x00001018)
#define RXS_CTRL_CAPT                                ((uint32_t)0x0000101c)
#define RXS_DEV32_DESTID_CAPT                        ((uint32_t)0x00001020)
#define RXS_DEV32_SRCID_CAPT                         ((uint32_t)0x00001024)
#define RXS_PW_TGT_ID                                ((uint32_t)0x00001028)
#define RXS_PKT_TIME_LIVE                            ((uint32_t)0x0000102c)
#define RXS_DEV32_PW_TGT_ID                          ((uint32_t)0x00001030)
#define RXS_PW_TRAN_CTL                              ((uint32_t)0x00001034)
#define RXS_SPX_ERR_DET(X)        ((uint32_t)(0x1040 + (RXS_SPX_PP_OSET * (X))))
#define RXS_SPX_RATE_EN(X)        ((uint32_t)(0x1044 + (RXS_SPX_PP_OSET * (X))))
#define RXS_SPX_DLT_CSR(X)        ((uint32_t)(0x1070 + (RXS_SPX_PP_OSET * (X))))
#define RXS_PER_LANE_BH                              ((uint32_t)0x00003000)
#define RXS_LANEX_STAT0(X)                 ((uint32_t)(0x3010 + 0x020*(X)))
#define RXS_LANEX_STAT1(X)                 ((uint32_t)(0x3014 + 0x020*(X)))
#define RXS_LANEX_STAT2(X)                 ((uint32_t)(0x3018 + 0x020*(X)))
#define RXS_LANEX_STAT3(X)                 ((uint32_t)(0x301c + 0x020*(X)))
#define RXS_LANEX_STAT4(X)                 ((uint32_t)(0x3020 + 0x020*(X)))
#define RXS_LANEX_STAT5(X)                 ((uint32_t)(0x3024 + 0x020*(X)))
#define RXS_LANEX_STAT6(X)                 ((uint32_t)(0x3028 + 0x020*(X)))
#define RXS_SWITCH_RT_BH                             ((uint32_t)0x00005000)
#define RXS_BC_RT_CTL                                ((uint32_t)0x00005020)
#define RXS_BC_MC_INFO                               ((uint32_t)0x00005028)
#define RXS_BC_RT_LVL0_INFO                          ((uint32_t)0x00005030)
#define RXS_BC_RT_LVL1_INFO                          ((uint32_t)0x00005034)
#define RXS_BC_RT_LVL2_INFO                          ((uint32_t)0x00005038)
#define RXS_SPX_RT_CTL(X)                  ((uint32_t)(0x5040 + 0x020*(X)))
#define RXS_SPX_MC_INFO(X)                 ((uint32_t)(0x5048 + 0x020*(X)))
#define RXS_SPX_RT_LVL0_INFO(X)            ((uint32_t)(0x5050 + 0x020*(X)))
#define RXS_SPX_RT_LVL1_INFO(X)            ((uint32_t)(0x5054 + 0x020*(X)))
#define RXS_SPX_RT_LVL2_INFO(X)            ((uint32_t)(0x5058 + 0x020*(X)))

#define RXS_IMP_SPEC_PP_OSET		  ((uint32_t)0x100)
#define RXS_PLM_BH                                   ((uint32_t)0x00010000)
#define RXS_PLM_SPX_IMP_SPEC_CTL(X)       ((uint32_t)(0x10100 + 0x100*(X)))
#define RXS_PLM_SPX_PWDN_CTL(X)           ((uint32_t)(0x10104 + 0x100*(X)))
#define RXS_PLM_SPX_1WR(X)                ((uint32_t)(0x10108 + 0x100*(X)))
#define RXS_PLM_SPX_MULT_ACK_CTL(X)       ((uint32_t)(0x1010c + 0x100*(X)))
#define RXS_PLM_SPX_STAT(X)               ((uint32_t)(0x10110 + 0x100*(X)))
#define RXS_PLM_SPX_INT_EN(X)             ((uint32_t)(0x10114 + 0x100*(X)))
#define RXS_PLM_SPX_PW_EN(X)              ((uint32_t)(0x10118 + 0x100*(X)))
#define RXS_PLM_SPX_EVENT_GEN(X)          ((uint32_t)(0x1011c + 0x100*(X)))
#define RXS_PLM_SPX_ALL_INT_EN(X)         ((uint32_t)(0x10120 + 0x100*(X)))
#define RXS_PLM_SPX_PATH_CTL(X)           ((uint32_t)(0x1012c + 0x100*(X)))
#define RXS_PLM_SPX_SILENCE_TMR(X)        ((uint32_t)(0x10138 + 0x100*(X)))
#define RXS_PLM_SPX_VMIN_EXP(X)           ((uint32_t)(0x1013c + 0x100*(X)))
#define RXS_PLM_SPX_POL_CTL(X)                ((uint32_t)(0x10140 + 0x100*(X)))
#define RXS_PLM_SPX_CLKCOMP_CTL(X)        ((uint32_t)(0x10144 + 0x100*(X)))
#define RXS_PLM_SPX_DENIAL_CTL(X)         ((uint32_t)(0x10148 + 0x100*(X)))
#define RXS_PLM_SPX_ERR_REC_CTL(X)        ((uint32_t)(0x1014c + 0x100*(X)))
#define RXS_PLM_SPX_CS_TX1(X)             ((uint32_t)(0x10160 + 0x100*(X)))
#define RXS_PLM_SPX_CS_TX2(X)             ((uint32_t)(0x10164 + 0x100*(X)))
#define RXS_PLM_SPX_PNA_CAP(X)            ((uint32_t)(0x10168 + 0x100*(X)))
#define RXS_PLM_SPX_ACKID_CAP(X)          ((uint32_t)(0x1016c + 0x100*(X)))
#define RXS_PLM_SPX_SCRATCHY(X,Y)  ((uint32_t)(0x10190 + 0x100*(X) + 0x4*(Y)))
#define RXS_PLM_SPX_MAX_SCRATCHY 15
#define RXS_TLM_BH                                   ((uint32_t)0x00014000)
#define RXS_TLM_SPX_CONTROL(X)            ((uint32_t)(0x14100 + 0x100*(X)))
#define RXS_TLM_SPX_STAT(X)               ((uint32_t)(0x14110 + 0x100*(X)))
#define RXS_TLM_SPX_INT_EN(X)             ((uint32_t)(0x14114 + 0x100*(X)))
#define RXS_TLM_SPX_PW_EN(X)              ((uint32_t)(0x14118 + 0x100*(X)))
#define RXS_TLM_SPX_EVENT_GEN(X)          ((uint32_t)(0x1411c + 0x100*(X)))
#define RXS_TLM_SPX_FTYPE_FILT(X)         ((uint32_t)(0x14160 + 0x100*(X)))
#define RXS_TLM_SPX_FTYPE_CAPT(X)         ((uint32_t)(0x14164 + 0x100*(X)))
#define RXS_TLM_SPX_MTC_ROUTE_EN(X)       ((uint32_t)(0x14170 + 0x100*(X)))
#define RXS_TLM_SPX_ROUTE_EN(X)           ((uint32_t)(0x14174 + 0x100*(X)))
#define RXS_PBM_BH                                   ((uint32_t)0x00018000)
#define RXS_PBM_SPX_CONTROL(X)            ((uint32_t)(0x18100 + 0x100*(X)))
#define RXS_PBM_SPX_STAT(X)               ((uint32_t)(0x18110 + 0x100*(X)))
#define RXS_PBM_SPX_INT_EN(X)             ((uint32_t)(0x18114 + 0x100*(X)))
#define RXS_PBM_SPX_PW_EN(X)              ((uint32_t)(0x18118 + 0x100*(X)))
#define RXS_PBM_SPX_EVENT_GEN(X)          ((uint32_t)(0x1811c + 0x100*(X)))
#define RXS_PBM_SPX_EG_RESOURCES(X)       ((uint32_t)(0x18124 + 0x100*(X)))
#define RXS_PBM_SPX_BUFF_STATUS(X)        ((uint32_t)(0x18170 + 0x100*(X)))
#define RXS_PBM_SPX_SCRATCH1(X)           ((uint32_t)(0x18180 + 0x100*(X)))
#define RXS_PBM_SPX_SCRATCH2(X)           ((uint32_t)(0x18184 + 0x100*(X)))
#define RXS_PCNTR_BH                                 ((uint32_t)0x0001c000)
#define RXS_PCNTR_CTL                                ((uint32_t)0x0001c004)
#define RXS_SPX_PCNTR_EN(X)               ((uint32_t)(0x1c100 + 0x100*(X)))
#define RXS_SPX_PCNTR_CTL(X,Y) ((uint32_t)(0x1c110 + 0x100*(X) + 0x4*(Y)))
#define RXS_SPX_PCNTR_CNT(X,Y) ((uint32_t)(0x1c130 + 0x100*(X) + 0x4*(Y)))
#define RXS_PCAP_BH                                  ((uint32_t)0x00020000)
#define RXS_SPX_PCAP_ACC(X)               ((uint32_t)(0x20100 + 0x100*(X)))
#define RXS_SPX_PCAP_CTL(X)               ((uint32_t)(0x20104 + 0x100*(X)))
#define RXS_SPX_PCAP_STAT(X)              ((uint32_t)(0x20110 + 0x100*(X)))
#define RXS_SPX_PCAP_GEN(X)               ((uint32_t)(0x2011c + 0x100*(X)))
#define RXS_SPX_PCAP_SB_DATA(X)           ((uint32_t)(0x20120 + 0x100*(X)))
#define RXS_SPX_PCAP_MEM_DEPTH(X)         ((uint32_t)(0x20124 + 0x100*(X)))
#define RXS_SPX_PCAP_DATAY(X,Y) ((uint32_t)(0x20130 + 0x100*(X) + 0x4*(Y)))
#define RXS_DBG_EL_BH                                ((uint32_t)0x00024000)
#define RXS_SPX_DBG_EL_ACC(X)             ((uint32_t)(0x24100 + 0x100*(X)))
#define RXS_SPX_DBG_EL_INFO(X)            ((uint32_t)(0x24104 + 0x100*(X)))
#define RXS_SPX_DBG_EL_CTL(X)             ((uint32_t)(0x24108 + 0x100*(X)))
#define RXS_SPX_DBG_EL_INT_EN(X)          ((uint32_t)(0x2410c + 0x100*(X)))
#define RXS_SPX_DBG_EL_SRC_LOG_EN(X)      ((uint32_t)(0x24110 + 0x100*(X)))
#define RXS_SPX_DBG_EL_TRIG0_MASK(X)      ((uint32_t)(0x24114 + 0x100*(X)))
#define RXS_SPX_DBG_EL_TRIG0_VAL(X)       ((uint32_t)(0x24118 + 0x100*(X)))
#define RXS_SPX_DBG_EL_TRIG1_MASK(X)      ((uint32_t)(0x2411c + 0x100*(X)))
#define RXS_SPX_DBG_EL_TRIG1_VAL(X)       ((uint32_t)(0x24120 + 0x100*(X)))
#define RXS_SPX_DBG_EL_TRIG_STAT(X)       ((uint32_t)(0x24124 + 0x100*(X)))
#define RXS_SPX_DBG_EL_WR_TRIG_IDX(X)     ((uint32_t)(0x24128 + 0x100*(X)))
#define RXS_SPX_DBG_EL_RD_IDX(X)          ((uint32_t)(0x2412c + 0x100*(X)))
#define RXS_SPX_DBG_EL_DATA(X)            ((uint32_t)(0x24130 + 0x100*(X)))
#define RXS_SPX_DBG_EL_SRC_STATY(X,Y)  ((uint32_t)(0x24140 + 0x100*(X) + 0x4*(Y)))
#define RXS_SPX_DBG_EL_SRC_STAT4(X)       ((uint32_t)(0x24150 + 0x100*(X)))
#define RXS_SPX_DBG_EL_SRC_STAT5(X)       ((uint32_t)(0x24154 + 0x100*(X)))
#define RXS_SPX_DBG_EL_SRC_STAT6(X)       ((uint32_t)(0x24158 + 0x100*(X)))
#define RXS_SPX_DBG_EL_SRC_STAT7(X)       ((uint32_t)(0x2415c + 0x100*(X)))
#define RXS_SPX_DBG_EL_SRC_STAT8(X)       ((uint32_t)(0x24160 + 0x100*(X)))
#define RXS_SPX_DBG_EL_SRC_STAT9(X)       ((uint32_t)(0x24164 + 0x100*(X)))
#define RXS_SPX_DBG_EL_SRC_STAT10(X)      ((uint32_t)(0x24168 + 0x100*(X)))
#define RXS_SPX_DBG_EL_SRC_STAT11(X)      ((uint32_t)(0x2416c + 0x100*(X)))
#define RXS_LANE_TEST_BH                             ((uint32_t)0x00028000)
#define RXS_LANEX_PRBS_CTRL(X)            ((uint32_t)(0x28104 + 0x100*(X)))
#define RXS_LANEX_PRBS_STATUS(X)          ((uint32_t)(0x28108 + 0x100*(X)))
#define RXS_LANEX_PRBS_ERR_CNT(X)         ((uint32_t)(0x28120 + 0x100*(X)))
#define RXS_LANEX_PRBS_SEED_0U(X)         ((uint32_t)(0x28124 + 0x100*(X)))
#define RXS_LANEX_PRBS_SEED_0M(X)         ((uint32_t)(0x28128 + 0x100*(X)))
#define RXS_LANEX_PRBS_SEED_0L(X)         ((uint32_t)(0x2812c + 0x100*(X)))
#define RXS_LANEX_PRBS_SEED_1U(X)         ((uint32_t)(0x28134 + 0x100*(X)))
#define RXS_LANEX_PRBS_SEED_1M(X)         ((uint32_t)(0x28138 + 0x100*(X)))
#define RXS_LANEX_PRBS_SEED_1L(X)         ((uint32_t)(0x2813c + 0x100*(X)))
#define RXS_LANEX_BER_CTL(X)              ((uint32_t)(0x28140 + 0x100*(X)))
#define RXS_LANEX_BER_DATA_0(X)           ((uint32_t)(0x28144 + 0x100*(X)))
#define RXS_LANEX_BER_DATA_1(X)           ((uint32_t)(0x28148 + 0x100*(X)))
#define RXS_LANEX_PCS_DBG(X)              ((uint32_t)(0x28150 + 0x100*(X)))
#define RXS_LANEX_BERM_CTL(X)             ((uint32_t)(0x28160 + 0x100*(X)))
#define RXS_LANEX_BERM_CNTR(X)            ((uint32_t)(0x28164 + 0x100*(X)))
#define RXS_LANEX_BERM_PD(X)              ((uint32_t)(0x28168 + 0x100*(X)))
#define RXS_LANEX_BERM_BITS(X)            ((uint32_t)(0x2816c + 0x100*(X)))
#define RXS_LANEX_BERM_ERRORS(X)          ((uint32_t)(0x28170 + 0x100*(X)))
#define RXS_LANEX_BERM_PERIODS(X)         ((uint32_t)(0x28174 + 0x100*(X)))
#define RXS_LANEX_DME_TEST(X)             ((uint32_t)(0x281fc + 0x100*(X)))
#define RXS_FAB_PORT_BH                              ((uint32_t)0x0002c000)
#define RXS_FP_X_IB_BUFF_WM_01(X)         ((uint32_t)(0x2c100 + 0x100*(X)))
#define RXS_FP_X_IB_BUFF_WM_23(X)         ((uint32_t)(0x2c104 + 0x100*(X)))
#define RXS_FP_X_PLW_SCRATCH(X)           ((uint32_t)(0x2c108 + 0x100*(X)))
#define RXS_BC_L0_G0_ENTRYX_CSR(X)        ((uint32_t)(0x30000 + 0x004*(X)))
#define RXS_BC_L1_GX_ENTRYY_CSR(X,Y)  ((uint32_t)(0x30400 + 0x400*(X) + 0x4*(Y)))
#define RXS_BC_L2_GX_ENTRYY_CSR(X,Y)  ((uint32_t)(0x31000 + 0x400*(X) + 0x4*(Y)))
#define RXS_BC_MC_X_S_CSR(X)              ((uint32_t)(0x32000 + 0x008*(X)))
#define RXS_BC_MC_X_C_CSR(X)              ((uint32_t)(0x32004 + 0x008*(X)))
#define RXS_FAB_INGR_CTL_BH                          ((uint32_t)0x00034000)
#define RXS_FAB_IG_X_2X4X_4X2X_WM(X)      ((uint32_t)(0x34104 + 0x100*(X)))
#define RXS_FAB_IG_X_2X2X_WM(X)           ((uint32_t)(0x34108 + 0x100*(X)))
#define RXS_FAB_IG_X_4X4X_WM(X)           ((uint32_t)(0x34110 + 0x100*(X)))
#define RXS_FAB_IG_X_MTC_VOQ_ACT(X)       ((uint32_t)(0x34120 + 0x100*(X)))
#define RXS_FAB_IG_X_VOQ_ACT(X)           ((uint32_t)(0x34130 + 0x100*(X)))
#define RXS_FAB_IG_X_CTL(X)               ((uint32_t)(0x34140 + 0x100*(X)))
#define RXS_FAB_IG_X_SCRATCH(X)           ((uint32_t)(0x341f8 + 0x100*(X)))
#define RXS_EM_BH                                    ((uint32_t)0x00040000)
#define RXS_EM_INT_STAT                              ((uint32_t)0x00040010)
#define RXS_EM_INT_EN                                ((uint32_t)0x00040014)
#define RXS_EM_INT_PORT_STAT                         ((uint32_t)0x00040018)
#define RXS_EM_PW_STAT                               ((uint32_t)0x00040020)
#define RXS_EM_PW_EN                                 ((uint32_t)0x00040024)
#define RXS_EM_PW_PORT_STAT                          ((uint32_t)0x00040028)
#define RXS_EM_DEV_INT_EN                            ((uint32_t)0x00040030)
#define RXS_EM_EVENT_GEN                             ((uint32_t)0x00040034)
#define RXS_EM_MECS_CTL                              ((uint32_t)0x00040038)
#define RXS_EM_MECS_INT_EN                           ((uint32_t)0x00040040)
#define RXS_EM_MECS_PORT_STAT                        ((uint32_t)0x00040050)
#define RXS_EM_RST_PORT_STAT                         ((uint32_t)0x00040060)
#define RXS_EM_RST_INT_EN                            ((uint32_t)0x00040068)
#define RXS_EM_RST_PW_EN                             ((uint32_t)0x00040070)
#define RXS_EM_FAB_INT_STAT                          ((uint32_t)0x00040080)
#define RXS_EM_FAB_PW_STAT                           ((uint32_t)0x00040088)
#define RXS_PW_BH                                    ((uint32_t)0x00040200)
#define RXS_PW_CTL                                   ((uint32_t)0x00040204)
#define RXS_PW_ROUTE                                 ((uint32_t)0x00040208)
#define RXS_MPM_BH                                   ((uint32_t)0x00040400)
#define RXS_RB_RESTRICT                              ((uint32_t)0x00040404)
#define RXS_MTC_WR_RESTRICT                          ((uint32_t)0x00040408)
#define RXS_MTC_RD_RESTRICT                          ((uint32_t)0x00040418)
#define RXS_MPM_SCRATCH1                             ((uint32_t)0x00040424)
#define RXS_PORT_NUMBER                              ((uint32_t)0x00040428)
#define RXS_PRESCALAR_SRV_CLK                        ((uint32_t)0x00040430)
#define RXS_REG_RST_CTL                              ((uint32_t)0x00040434)
#define RXS_MPM_SCRATCH2                             ((uint32_t)0x00040438)
#define RXS_ASBLY_ID_OVERRIDE                        ((uint32_t)0x00040470)
#define RXS_ASBLY_INFO_OVERRIDE                      ((uint32_t)0x00040474)
#define RXS_MPM_MTC_RESP_PRIO                        ((uint32_t)0x00040478)
#define RXS_MPM_MTC_ACTIVE                           ((uint32_t)0x0004047c)
#define RXS_MPM_CFGSIG0                              ((uint32_t)0x000404f0)
#define RXS_MPM_CFGSIG1                              ((uint32_t)0x000404f4)
#define RXS_MPM_CFGSIG2                              ((uint32_t)0x000404f8)
#define RXS_FAB_GEN_BH                               ((uint32_t)0x00040600)
#define RXS_FAB_GLOBAL_MEM_PWR_MODE                  ((uint32_t)0x00040700)
#define RXS_FAB_GLOBAL_CLK_GATE                      ((uint32_t)0x00040708)
#define RXS_FAB_4X_MODE                              ((uint32_t)0x00040720)
#define RXS_FAB_MBCOL_ACT                            ((uint32_t)0x00040760)
#define RXS_FAB_MIG_ACT                              ((uint32_t)0x00040770)
#define RXS_PHY_SERIAL_IF_EN                         ((uint32_t)0x00040780)
#define RXS_PHY_TX_DISABLE_CTRL                      ((uint32_t)0x00040784)
#define RXS_PHY_LOOPBACK_CTRL                        ((uint32_t)0x00040788)
#define RXS_PHY_PORT_OK_CTRL                         ((uint32_t)0x0004078c)
#define RXS_MCM_ROUTE_EN                             ((uint32_t)0x000407a0)
#define RXS_BOOT_CTL                                 ((uint32_t)0x000407c0)
#define RXS_FAB_GLOBAL_PWR_GATE_CLR                  ((uint32_t)0x000407e8)
#define RXS_FAB_GLOBAL_PWR_GATE                      ((uint32_t)0x000407ec)
#define RXS_DEL_BH                                   ((uint32_t)0x00040800)
#define RXS_DEL_ACC                                  ((uint32_t)0x00040804)
#define RXS_DEL_INFO                                 ((uint32_t)0x00040984)
#define RXS_DEL_CTL                                  ((uint32_t)0x00040988)
#define RXS_DEL_INT_EN                               ((uint32_t)0x0004098c)
#define RXS_DEL_SRC_LOG_EN                           ((uint32_t)0x00040990)
#define RXS_DEL_TRIG0_MASK                           ((uint32_t)0x00040994)
#define RXS_DEL_TRIG0_VAL                            ((uint32_t)0x00040998)
#define RXS_DEL_TRIG1_MASK                           ((uint32_t)0x0004099c)
#define RXS_DEL_TRIG1_VAL                            ((uint32_t)0x000409a0)
#define RXS_DEL_TRIG_STAT                            ((uint32_t)0x000409a4)
#define RXS_DEL_WR_TRIG_IDX                          ((uint32_t)0x000409a8)
#define RXS_DEL_RD_IDX                               ((uint32_t)0x000409ac)
#define RXS_DEL_DATA                                 ((uint32_t)0x000409b0)
#define RXS_DEL_SRC_STAT0                            ((uint32_t)0x000409c0)
#define RXS_DEL_SRC_STAT1                            ((uint32_t)0x000409c4)
#define RXS_DEL_SRC_STAT2                            ((uint32_t)0x000409c8)
#define RXS_DEL_SRC_STAT3                            ((uint32_t)0x000409cc)
#define RXS_DEL_SRC_STAT4                            ((uint32_t)0x000409d0)
#define RXS_DEL_SRC_STATX(X)              ((uint32_t)(0x409d4 + 0x004*(X)))
#define RXS_FAB_CBCOL_BH                             ((uint32_t)0x00048000)
#define RXS_FAB_CBCOL_X_CTL(X)            ((uint32_t)(0x48100 + 0x100*(X)))
#define RXS_FAB_CBCOL_X_SAT_CTL(X)        ((uint32_t)(0x48120 + 0x100*(X)))
#define RXS_FAB_CBCOL_X_OS_INP_EN(X)      ((uint32_t)(0x48130 + 0x100*(X)))
#define RXS_FAB_CBCOL_X_ACT(X)            ((uint32_t)(0x48140 + 0x100*(X)))
#define RXS_FAB_CBCOL_X_ACT_SUMM(X)       ((uint32_t)(0x48150 + 0x100*(X)))
#define RXS_FAB_CBCOL_X_PG_WM(X)          ((uint32_t)(0x48160 + 0x100*(X)))
#define RXS_FAB_CBCOL_X_ND_WM_01(X)       ((uint32_t)(0x48164 + 0x100*(X)))
#define RXS_FAB_CBCOL_X_ND_WM_23(X)       ((uint32_t)(0x48168 + 0x100*(X)))
#define RXS_FAB_CBCOL_X_CLK_GATE(X)       ((uint32_t)(0x48180 + 0x100*(X)))
#define RXS_FAB_CBCOL_X_SCRATCH(X)        ((uint32_t)(0x481f8 + 0x100*(X)))
#define RXS_PKT_GEN_BH                               ((uint32_t)0x0004c000)
#define RXS_FAB_PGEN_X_ACC(X)             ((uint32_t)(0x4c100 + 0x100*(X)))
#define RXS_FAB_PGEN_X_STAT(X)            ((uint32_t)(0x4c110 + 0x100*(X)))
#define RXS_FAB_PGEN_X_INT_EN(X)          ((uint32_t)(0x4c114 + 0x100*(X)))
#define RXS_FAB_PGEN_X_PW_EN(X)           ((uint32_t)(0x4c118 + 0x100*(X)))
#define RXS_FAB_PGEN_X_GEN(X)             ((uint32_t)(0x4c11c + 0x100*(X)))
#define RXS_FAB_PGEN_X_CTL(X)             ((uint32_t)(0x4c120 + 0x100*(X)))
#define RXS_FAB_PGEN_X_DATA_CTL(X)        ((uint32_t)(0x4c124 + 0x100*(X)))
#define RXS_FAB_PGEN_X_DATA_Y(X,Y)        ((uint32_t)(0x4c130 + 0x100*(X) + 0x4*(Y)))
#define RXS_SPX_L0_G0_ENTRYY_CSR(X,Y)     ((uint32_t)(0x50000 + 0x2000*(X) + 0x4*(Y)))
#define RXS_SPX_L1_GY_ENTRYZ_CSR(X,Y,Z)   ((uint32_t)(0x50400 + 0x2000*(X) + 0x400*(Y) + 0x4*(Z)))
#define RXS_SPX_L2_GY_ENTRYZ_CSR(X,Y,Z)   ((uint32_t)(0x51000 + 0x2000*(X) + 0x400*(Y) + 0x4*(Z)))
#define RXS_SPX_MC_Y_S_CSR(X,Y)           ((uint32_t)(0x80000 + 0x1000*(X) + 0x8*(Y)))
#define RXS_SPX_MC_Y_C_CSR(X,Y)           ((uint32_t) (0x80004 + 0x1000*(X) + 0x8*(Y)))


/***************************************************************/
/* RapidIO Register Bit Masks and Reset Values Definitions     */
/***************************************************************/

/* RXS_DEV_ID : Register Bits Masks Definitions */
#define RXS_DEV_IDENT_VEND                           ((uint32_t)0x0000ffff)
#define RXS_DEV_IDENT_DEVI                           ((uint32_t)0xffff0000)
#define RXS_DEVICE_VENDOR                            ((uint32_t)0x00000038)

/* RXS_DEV_INFO : Register Bits Masks Definitions */
#define RXS_DEV_INFO_DEV_REV                         ((uint32_t)0xffffffff)

/* RXS_ASBLY_ID : Register Bits Masks Definitions */
#define RXS_ASBLY_ID_ASBLY_VEN_ID                    ((uint32_t)0x0000ffff)
#define RXS_ASBLY_ID_ASBLY_ID                        ((uint32_t)0xffff0000)

/* RXS_ASBLY_INFO : Register Bits Masks Definitions */
#define RXS_ASBLY_INFO_EXT_FEAT_PTR                  ((uint32_t)0x0000ffff)
#define RXS_ASBLY_INFO_ASBLY_REV                     ((uint32_t)0xffff0000)

/* RXS_PE_FEAT : Register Bits Masks Definitions */
#define RXS_PE_FEAT_EXT_AS                           ((uint32_t)0x00000007)
#define RXS_PE_FEAT_EXT_FEA                          ((uint32_t)0x00000008)
#define RXS_PE_FEAT_CTLS                             ((uint32_t)0x00000010)
#define RXS_PE_FEAT_CRF                              ((uint32_t)0x00000020)
#define RXS_PE_FEAT_FLOW_CTRL                        ((uint32_t)0x00000080)
#define RXS_PE_FEAT_SRTC                             ((uint32_t)0x00000100)
#define RXS_PE_FEAT_ERTC                             ((uint32_t)0x00000200)
#define RXS_PE_FEAT_MC                               ((uint32_t)0x00000400)
#define RXS_PE_FEAT_FLOW_ARB                         ((uint32_t)0x00000800)
#define RXS_PE_FEAT_DEV32                            ((uint32_t)0x00001000)
#define RXS_PE_FEAT_MULT_P                           ((uint32_t)0x08000000)
#define RXS_PE_FEAT_SW                               ((uint32_t)0x10000000)
#define RXS_PE_FEAT_PROC                             ((uint32_t)0x20000000)
#define RXS_PE_FEAT_MEM                              ((uint32_t)0x40000000)
#define RXS_PE_FEAT_BRDG                             ((uint32_t)0x80000000)

/* RXS_SW_PORT : Register Bits Masks Definitions */
#define RXS_SW_PORT_PORT_NUM                         ((uint32_t)0x000000ff)
#define RXS_SW_PORT_PORT_TOTAL                       ((uint32_t)0x0000ff00)

/* RXS_SRC_OP : Register Bits Masks Definitions */
#define RXS_SRC_OP_IMP_DEF2                          ((uint32_t)0x00000003)
#define RXS_SRC_OP_PORT_WR                           ((uint32_t)0x00000004)
#define RXS_SRC_OP_A_SWAP                            ((uint32_t)0x00000008)
#define RXS_SRC_OP_A_CLEAR                           ((uint32_t)0x00000010)
#define RXS_SRC_OP_A_SET                             ((uint32_t)0x00000020)
#define RXS_SRC_OP_A_DEC                             ((uint32_t)0x00000040)
#define RXS_SRC_OP_A_INC                             ((uint32_t)0x00000080)
#define RXS_SRC_OP_ATSWAP                            ((uint32_t)0x00000100)
#define RXS_SRC_OP_ACSWAP                            ((uint32_t)0x00000200)
#define RXS_SRC_OP_DBELL                             ((uint32_t)0x00000400)
#define RXS_SRC_OP_D_MSG                             ((uint32_t)0x00000800)
#define RXS_SRC_OP_WR_RES                            ((uint32_t)0x00001000)
#define RXS_SRC_OP_STRM_WR                           ((uint32_t)0x00002000)
#define RXS_SRC_OP_WRITE                             ((uint32_t)0x00004000)
#define RXS_SRC_OP_READ                              ((uint32_t)0x00008000)
#define RXS_SRC_OP_IMP_DEF                           ((uint32_t)0x00030000)
#define RXS_SRC_OP_DS                                ((uint32_t)0x00040000)
#define RXS_SRC_OP_DS_TM                             ((uint32_t)0x00080000)
#define RXS_SRC_OP_RIO_RSVD_11                       ((uint32_t)0x00100000)
#define RXS_SRC_OP_RIO_RSVD_10                       ((uint32_t)0x00200000)
#define RXS_SRC_OP_G_TLB_SYNC                        ((uint32_t)0x00400000)
#define RXS_SRC_OP_G_TLB_INVALIDATE                  ((uint32_t)0x00800000)
#define RXS_SRC_OP_G_IC_INVALIDATE                   ((uint32_t)0x01000000)
#define RXS_SRC_OP_G_IO_READ                         ((uint32_t)0x02000000)
#define RXS_SRC_OP_G_DC_FLUSH                        ((uint32_t)0x04000000)
#define RXS_SRC_OP_G_CASTOUT                         ((uint32_t)0x08000000)
#define RXS_SRC_OP_G_DC_INVALIDATE                   ((uint32_t)0x10000000)
#define RXS_SRC_OP_G_READ_OWN                        ((uint32_t)0x20000000)
#define RXS_SRC_OP_G_IREAD                           ((uint32_t)0x40000000)
#define RXS_SRC_OP_G_READ                            ((uint32_t)0x80000000)

/* RXS_SW_RT_DEST_ID : Register Bits Masks Definitions */
#define RXS_SW_RT_DEST_ID_MAX_DEST_ID                ((uint32_t)0x0000ffff)

/* RXS_PE_LL_CTL : Register Bits Masks Definitions */
#define RXS_PE_LL_CTL_EACTRL                         ((uint32_t)0x00000007)

/* RXS_HOST_BASE_ID_LOCK : Register Bits Masks Definitions */
#define RXS_HOST_BASE_ID_LOCK_HOST_BASE_ID           ((uint32_t)0x0000ffff)
#define RXS_HOST_BASE_ID_LOCK_HOST_BASE_DEV32ID      ((uint32_t)0xffff0000)

/* RXS_COMP_TAG : Register Bits Masks Definitions */
#define RXS_COMP_TAG_CTAG                            ((uint32_t)0xffffffff)

/* RXS_ROUTE_CFG_DESTID : Register Bits Masks Definitions */
#define RXS_ROUTE_CFG_DESTID_DEV8_DEST_ID            ((uint32_t)0x000000ff)
#define RXS_ROUTE_CFG_DESTID_DEV16_MSB_DEST_ID       ((uint32_t)0x0000ff00)
#define RXS_ROUTE_CFG_DESTID_ERTC_EN                 ((uint32_t)0x80000000)

/* RXS_ROUTE_CFG_PORT : Register Bits Masks Definitions */
#define RXS_ROUTE_CFG_PORT_CFG_OUT_PORT              ((uint32_t)0x000000ff)
#define RXS_ROUTE_CFG_PORT_ROUTE_TYPE                ((uint32_t)0x00000300)
#define RXS_ROUTE_CFG_PORT_CAPTURE                   ((uint32_t)0x80000000)

/* RXS_ROUTE_DFLT_PORT : Register Bits Masks Definitions */
#define RXS_ROUTE_DFLT_PORT_DEFAULT_OUT_PORT         ((uint32_t)0x000000ff)
#define RXS_ROUTE_DFLT_PORT_ROUTE_TYPE               ((uint32_t)0x00000300)
#define RXS_ROUTE_DFLT_PORT_CAPTURE                  ((uint32_t)0x80000000)

/* RXS_SP_MB_HEAD : Register Bits Masks Definitions */
#define RXS_SP_MB_HEAD_EF_ID                         ((uint32_t)0x0000ffff)
#define RXS_SP_MB_HEAD_EF_PTR                        ((uint32_t)0xffff0000)

/* RXS_SP_LT_CTL : Register Bits Masks Definitions */
#define RXS_SP_LT_CTL_TVAL                           ((uint32_t)0xffffff00)
#define RXS_SP_LT_CTL_MAX             ((uint32_t)(RXS_SP_LT_CTL_TVAL >> 8))

/* RXS_SP_GEN_CTL : Register Bits Masks Definitions */
#define RXS_SP_GEN_CTL_DISC                          ((uint32_t)0x20000000)

/* RXS_SPX_LM_REQ : Register Bits Masks Definitions */
#define RXS_SPX_LM_REQ_CMD                           ((uint32_t)0x00000007)

/* RXS_SPX_LM_RESP : Register Bits Masks Definitions */
#define RXS_SPX_LM_RESP_LINK_STAT                    ((uint32_t)0x0000001f)
#define RXS_SPX_LM_RESP_ACK_ID_STAT                  ((uint32_t)0x0001ffe0)
#define RXS_SPX_LM_RESP_STAT_CS64                    ((uint32_t)0x1ffe0000)
#define RXS_SPX_LM_RESP_RESP_VLD                     ((uint32_t)0x80000000)

/* RXS_SPX_CTL2 : Register Bits Masks Definitions */
#define RXS_SPX_CTL2_RTEC_EN                         ((uint32_t)0x00000001)
#define RXS_SPX_CTL2_RTEC                            ((uint32_t)0x00000002)
#define RXS_SPX_CTL2_D_SCRM_DIS                      ((uint32_t)0x00000004)
#define RXS_SPX_CTL2_INACT_LN_EN                     ((uint32_t)0x00000008)
#define RXS_SPX_CTL2_RETRAIN_EN                      ((uint32_t)0x00000010)
#define RXS_SPX_CTL2_GB_12P5_EN                      ((uint32_t)0x00001000)
#define RXS_SPX_CTL2_GB_12P5                         ((uint32_t)0x00002000)
#define RXS_SPX_CTL2_GB_10P3_EN                      ((uint32_t)0x00004000)
#define RXS_SPX_CTL2_GB_10P3                         ((uint32_t)0x00008000)
#define RXS_SPX_CTL2_GB_6P25_EN                      ((uint32_t)0x00010000)
#define RXS_SPX_CTL2_GB_6P25                         ((uint32_t)0x00020000)
#define RXS_SPX_CTL2_GB_5P0_EN                       ((uint32_t)0x00040000)
#define RXS_SPX_CTL2_GB_5P0                          ((uint32_t)0x00080000)
#define RXS_SPX_CTL2_GB_3P125_EN                     ((uint32_t)0x00100000)
#define RXS_SPX_CTL2_GB_3P125                        ((uint32_t)0x00200000)
#define RXS_SPX_CTL2_GB_2P5_EN                       ((uint32_t)0x00400000)
#define RXS_SPX_CTL2_GB_2P5                          ((uint32_t)0x00800000)
#define RXS_SPX_CTL2_GB_1P25_EN                      ((uint32_t)0x01000000)
#define RXS_SPX_CTL2_GB_1P25                         ((uint32_t)0x02000000)
#define RXS_SPX_CTL2_BAUD_DISC                       ((uint32_t)0x08000000)
#define RXS_SPX_CTL2_BAUD_SEL                        ((uint32_t)0xf0000000)

#define RXS_SPX_CTL2_BAUD_SEL_UNINIT                 ((uint32_t)0x00000000)
#define RXS_SPX_CTL2_BAUD_SEL_UNSUP                  ((uint32_t)0x10000000)
#define RXS_SPX_CTL2_BAUD_SEL_2_5GB                  ((uint32_t)0x20000000)
#define RXS_SPX_CTL2_BAUD_SEL_3_125GB                ((uint32_t)0x30000000)
#define RXS_SPX_CTL2_BAUD_SEL_5_0GB                  ((uint32_t)0x40000000)
#define RXS_SPX_CTL2_BAUD_SEL_6_25GB                 ((uint32_t)0x50000000)
#define RXS_SPX_CTL2_BAUD_SEL_10_3125GB              ((uint32_t)0x60000000)
#define RXS_SPX_CTL2_BAUD_SEL_12_5GB                 ((uint32_t)0x70000000)

/* RXS_SPX_ERR_STAT : Register Bits Masks Definitions */
#define RXS_SPX_ERR_STAT_PORT_UNINIT                 ((uint32_t)0x00000001)
#define RXS_SPX_ERR_STAT_PORT_OK                     ((uint32_t)0x00000002)
#define RXS_SPX_ERR_STAT_PORT_ERR                    ((uint32_t)0x00000004)
#define RXS_SPX_ERR_STAT_PORT_UNAVL                  ((uint32_t)0x00000008)
#define RXS_SPX_ERR_STAT_PORT_W_P                    ((uint32_t)0x00000010)
#define RXS_SPX_ERR_STAT_PORT_W_DIS                  ((uint32_t)0x00000020)
#define RXS_SPX_ERR_STAT_INPUT_ERR_STOP              ((uint32_t)0x00000100)
#define RXS_SPX_ERR_STAT_INPUT_ERR_ENCTR             ((uint32_t)0x00000200)
#define RXS_SPX_ERR_STAT_INPUT_RS                    ((uint32_t)0x00000400)
#define RXS_SPX_ERR_STAT_OUTPUT_ERR_STOP             ((uint32_t)0x00010000)
#define RXS_SPX_ERR_STAT_OUTPUT_ERR_ENCTR            ((uint32_t)0x00020000)
#define RXS_SPX_ERR_STAT_OUTPUT_RS                   ((uint32_t)0x00040000)
#define RXS_SPX_ERR_STAT_OUTPUT_R                    ((uint32_t)0x00080000)
#define RXS_SPX_ERR_STAT_OUTPUT_RE                   ((uint32_t)0x00100000)
#define RXS_SPX_ERR_STAT_OUTPUT_FAIL                 ((uint32_t)0x02000000)
#define RXS_SPX_ERR_STAT_OUTPUT_DROP                 ((uint32_t)0x04000000)
#define RXS_SPX_ERR_STAT_TXFC                        ((uint32_t)0x08000000)
#define RXS_SPX_ERR_STAT_IDLE_SEQ                    ((uint32_t)0x30000000)
#define RXS_SPX_ERR_STAT_IDLE2_EN                    ((uint32_t)0x40000000)
#define RXS_SPX_ERR_STAT_IDLE2                       ((uint32_t)0x80000000)

/* RXS_SPX_CTL : Register Bits Masks Definitions */
#define RXS_SPX_CTL_PTYP                             ((uint32_t)0x00000001)
#define RXS_SPX_CTL_PORT_LOCKOUT                     ((uint32_t)0x00000002)
#define RXS_SPX_CTL_DROP_EN                          ((uint32_t)0x00000004)
#define RXS_SPX_CTL_STOP_FAIL_EN                     ((uint32_t)0x00000008)
#define RXS_SPX_CTL_PORT_WIDTH2                      ((uint32_t)0x00003000)
#define RXS_SPX_CTL_OVER_PWIDTH2                     ((uint32_t)0x0000c000)
#define RXS_SPX_CTL_FLOW_ARB                         ((uint32_t)0x00010000)
#define RXS_SPX_CTL_ENUM_B                           ((uint32_t)0x00020000)
#define RXS_SPX_CTL_FLOW_CTRL                        ((uint32_t)0x00040000)
#define RXS_SPX_CTL_MULT_CS                          ((uint32_t)0x00080000)
#define RXS_SPX_CTL_ERR_DIS                          ((uint32_t)0x00100000)
#define RXS_SPX_CTL_INP_EN                           ((uint32_t)0x00200000)
#define RXS_SPX_CTL_OTP_EN                           ((uint32_t)0x00400000)
#define RXS_SPX_CTL_PORT_DIS                         ((uint32_t)0x00800000)
#define RXS_SPX_CTL_OVER_PWIDTH                      ((uint32_t)0x07000000)
#define RXS_SPX_CTL_INIT_PWIDTH                      ((uint32_t)0x38000000)
#define RXS_SPX_CTL_PORT_WIDTH                       ((uint32_t)0xc0000000)

/* RXS_SPX_OUT_ACKID_CSR : Register Bits Masks Definitions */
#define RXS_SPX_OUT_ACKID_CSR_OUTB_ACKID             ((uint32_t)0x00000fff)
#define RXS_SPX_OUT_ACKID_CSR_OUTSTD_ACKID           ((uint32_t)0x00fff000)
#define RXS_SPX_OUT_ACKID_CSR_CLR_OUTSTD_ACKID       ((uint32_t)0x80000000)

/* RXS_SPX_IN_ACKID_CSR : Register Bits Masks Definitions */
#define RXS_SPX_IN_ACKID_CSR_INB_ACKID               ((uint32_t)0x00000fff)

/* RXS_SPX_POWER_MNGT_CSR : Register Bits Masks Definitions */
#define RXS_SPX_POWER_MNGT_CSR_LP_TX_STATUS          ((uint32_t)0x000000c0)
#define RXS_SPX_POWER_MNGT_CSR_CHG_LP_TX_WIDTH       ((uint32_t)0x00000700)
#define RXS_SPX_POWER_MNGT_CSR_MY_TX_STATUS          ((uint32_t)0x00001800)
#define RXS_SPX_POWER_MNGT_CSR_CHG_MY_TX_WIDTH       ((uint32_t)0x0000e000)
#define RXS_SPX_POWER_MNGT_CSR_RX_WIDTH_STATUS       ((uint32_t)0x00070000)
#define RXS_SPX_POWER_MNGT_CSR_TX_WIDTH_STATUS       ((uint32_t)0x00380000)
#define RXS_SPX_POWER_MNGT_CSR_ASYM_MODE_EN          ((uint32_t)0x07c00000)
#define RXS_SPX_POWER_MNGT_CSR_ASYM_MODE_SUP         ((uint32_t)0xf8000000)

/* RXS_SPX_LAT_OPT_CSR : Register Bits Masks Definitions */
#define RXS_SPX_LAT_OPT_CSR_PNA_ERR_REC_EN           ((uint32_t)0x00400000)
#define RXS_SPX_LAT_OPT_CSR_MULT_ACK_EN              ((uint32_t)0x00800000)
#define RXS_SPX_LAT_OPT_CSR_PNA_ACKID                ((uint32_t)0x20000000)
#define RXS_SPX_LAT_OPT_CSR_PNA_ERR_REC              ((uint32_t)0x40000000)
#define RXS_SPX_LAT_OPT_CSR_MULT_ACK                 ((uint32_t)0x80000000)

/* RXS_SPX_TMR_CTL : Register Bits Masks Definitions */
#define RXS_SPX_TMR_CTL_EMPHASIS_CMD_TIMEOUT         ((uint32_t)0x000000ff)
#define RXS_SPX_TMR_CTL_CW_CMPLT_TMR                 ((uint32_t)0x0000ff00)
#define RXS_SPX_TMR_CTL_DME_WAIT_FRAMES              ((uint32_t)0x00ff0000)
#define RXS_SPX_TMR_CTL_DME_CMPLT_TMR                ((uint32_t)0xff000000)

/* RXS_SPX_TMR_CTL2 : Register Bits Masks Definitions */
#define RXS_SPX_TMR_CTL2_RECOVERY_TMR                ((uint32_t)0x0000ff00)
#define RXS_SPX_TMR_CTL2_DISCOVERY_CMPLT_TMR         ((uint32_t)0x00ff0000)
#define RXS_SPX_TMR_CTL2_RETRAIN_CMPLT_TMR           ((uint32_t)0xff000000)

/* RXS_SPX_TMR_CTL3 : Register Bits Masks Definitions */
#define RXS_SPX_TMR_CTL3_KEEP_ALIVE_INTERVAL         ((uint32_t)0x000003ff)
#define RXS_SPX_TMR_CTL3_KEEP_ALIVE_PERIOD           ((uint32_t)0x0000fc00)
#define RXS_SPX_TMR_CTL3_RX_WIDTH_CMD_TIMEOUT        ((uint32_t)0x00ff0000)
#define RXS_SPX_TMR_CTL3_TX_WIDTH_CMD_TIMEOUT        ((uint32_t)0xff000000)

/* RXS_ERR_RPT_BH : Register Bits Masks Definitions */
#define RXS_ERR_RPT_BH_EF_ID                         ((uint32_t)0x0000ffff)
#define RXS_ERR_RPT_BH_EF_PTR                        ((uint32_t)0xffff0000)

/* RXS_ERR_MGMT_HS : Register Bits Masks Definitions */
#define RXS_ERR_MGMT_HS_HOT_SWAP                     ((uint32_t)0x40000000)
#define RXS_ERR_MGMT_HS_NO_ERR_MGMT                  ((uint32_t)0x80000000)

/* RXS_ERR_DET : Register Bits Masks Definitions */
#define RXS_ERR_DET_ILL_TYPE                         ((uint32_t)0x00400000)
#define RXS_ERR_DET_UNS_RSP                          ((uint32_t)0x00800000)
#define RXS_ERR_DET_ILL_ID                           ((uint32_t)0x04000000)

/* RXS_ERR_EN : Register Bits Masks Definitions */
#define RXS_ERR_EN_ILL_TYPE_EN                       ((uint32_t)0x00400000)
#define RXS_ERR_EN_UNS_RSP_EN                        ((uint32_t)0x00800000)
#define RXS_ERR_EN_ILL_ID_EN                         ((uint32_t)0x04000000)

/* RXS_H_ADDR_CAPT : Register Bits Masks Definitions */
#define RXS_H_ADDR_CAPT_ADDR                         ((uint32_t)0xffffffff)

/* RXS_ADDR_CAPT : Register Bits Masks Definitions */
#define RXS_ADDR_CAPT_XAMSBS                         ((uint32_t)0x00000003)
#define RXS_ADDR_CAPT_ADDR                           ((uint32_t)0xfffffff8)

/* RXS_ID_CAPT : Register Bits Masks Definitions */
#define RXS_ID_CAPT_SRC_ID                           ((uint32_t)0x000000ff)
#define RXS_ID_CAPT_MSB_SRC_ID                       ((uint32_t)0x0000ff00)
#define RXS_ID_CAPT_DEST_ID                          ((uint32_t)0x00ff0000)
#define RXS_ID_CAPT_MSB_DEST_ID                      ((uint32_t)0xff000000)

/* RXS_CTRL_CAPT : Register Bits Masks Definitions */
#define RXS_CTRL_CAPT_TT                             ((uint32_t)0x00000003)
#define RXS_CTRL_CAPT_IMPL_SPECIFIC                  ((uint32_t)0x0000fff0)
#define RXS_CTRL_CAPT_MSG_INFO                       ((uint32_t)0x00ff0000)
#define RXS_CTRL_CAPT_TTYPE                          ((uint32_t)0x0f000000)
#define RXS_CTRL_CAPT_FTYPE                          ((uint32_t)0xf0000000)

/* RXS_DEV32_DESTID_CAPT : Register Bits Masks Definitions */
#define RXS_DEV32_DESTID_CAPT_DEV32_DESTID           ((uint32_t)0xffffffff)

/* RXS_DEV32_SRCID_CAPT : Register Bits Masks Definitions */
#define RXS_DEV32_SRCID_CAPT_DEV32_SRCID             ((uint32_t)0xffffffff)

/* RXS_PW_TGT_ID : Register Bits Masks Definitions */
#define RXS_PW_TGT_ID_DEV32                          ((uint32_t)0x00004000)
#define RXS_PW_TGT_ID_DEV16                          ((uint32_t)0x00008000)
#define RXS_PW_TGT_ID_PW_TGT_ID                      ((uint32_t)0x00ff0000)
#define RXS_PW_TGT_ID_MSB_PW_ID                      ((uint32_t)0xff000000)

/* RXS_PKT_TIME_LIVE : Register Bits Masks Definitions */
#define RXS_PKT_TIME_LIVE_PKT_TIME_LIVE              ((uint32_t)0xffff0000)
#define RXS_PKT_TIME_LIVE_NSEC                       ((uint32_t)(6000))
#define RXS_PKT_TIME_LIVE_MAX                        ((uint32_t)(0xFFFC))

/* RXS_DEV32_PW_TGT_ID : Register Bits Masks Definitions */
#define RXS_DEV32_PW_TGT_ID_DEV32                    ((uint32_t)0xffffffff)

/* RXS_PW_TRAN_CTL : Register Bits Masks Definitions */
#define RXS_PW_TRAN_CTL_PW_DIS                       ((uint32_t)0x00000001)

/* RXS_SPX_ERR_DET : Register Bits Masks Definitions */
#define RXS_SPX_ERR_DET_LINK_INIT                    ((uint32_t)0x10000000)
#define RXS_SPX_ERR_DET_DLT                          ((uint32_t)0x20000000)
#define RXS_SPX_ERR_DET_OK_TO_UNINIT                 ((uint32_t)0x40000000)

/* RXS_SPX_RATE_EN : Register Bits Masks Definitions */
#define RXS_SPX_RATE_EN_LINK_INIT                    ((uint32_t)0x10000000)
#define RXS_SPX_RATE_EN_DLT                          ((uint32_t)0x20000000)
#define RXS_SPX_RATE_EN_OK_TO_UNINIT                 ((uint32_t)0x40000000)

/* RXS_SPX_DLT_CSR : Register Bits Masks Definitions */
#define RXS_SPX_DLT_CSR_TIMEOUT                      ((uint32_t)0xffffff00)
#define RXS_SPX_DLT_CSR_TIMEOUT_NSEC ((uint64_t)9990000000 / \
				((uint64_t)(RXS_SPX_DLT_CSR_TIMEOUT) >> 8))
#define RXS_SPX_DLT_CSR_TIMEOUT_MAX  ((uint32_t)RXS_SPX_DLT_CSR_TIMEOUT >> 8)

// RXS Definitions for the "Implementation specific" field of Port-writes
#define RXS_PW_ZERO            ((uint32_t)0x00000100)
#define RXS_PW_MULTIPORT       ((uint32_t)0x00000200)
#define RXS_PW_DEV_RCS         ((uint32_t)0x00000400)
#define RXS_PW_FAB_OR_DEL      ((uint32_t)0x00000800)
#define RXS_PW_INIT_FAIL       ((uint32_t)0x00001000)
#define RXS_PW_DEV_ECC         ((uint32_t)0x00002000)
#define RXS_PW_TLM_PW          RXS_PLM_SPX_STAT_TLM_PW
#define RXS_PW_PBM_PW          RXS_PLM_SPX_STAT_PBM_PW
#define RXS_PW_RST_REQ         RXS_PLM_SPX_STAT_RST_REQ
#define RXS_PW_PRST_REQ        RXS_PLM_SPX_STAT_PRST_REQ
#define RXS_PW_EL_INTA         RXS_PLM_SPX_STAT_EL_INTA
#define RXS_PW_EL_INTB         RXS_PLM_SPX_STAT_EL_INTB
#define RXS_PW_II_CHG_0        RXS_PLM_SPX_STAT_II_CHG_0
#define RXS_PW_II_CHG_1        RXS_PLM_SPX_STAT_II_CHG_1
#define RXS_PW_II_CHG_2        RXS_PLM_SPX_STAT_II_CHG_2
#define RXS_PW_II_CHG_3        RXS_PLM_SPX_STAT_II_CHG_3
#define RXS_PW_PCAP            RXS_PLM_SPX_STAT_PCAP
#define RXS_PW_DWNGD           RXS_PLM_SPX_STAT_DWNGD
#define RXS_PW_PBM_FATAL       RXS_PLM_SPX_STAT_PBM_FATAL
#define RXS_PW_PORT_ERR        RXS_PLM_SPX_STAT_PORT_ERR
#define RXS_PW_LINK_INIT       RXS_PLM_SPX_STAT_LINK_INIT
#define RXS_PW_DLT             RXS_PLM_SPX_STAT_DLT
#define RXS_PW_OK_TO_UNINIT    RXS_PLM_SPX_STAT_OK_TO_UNINIT
#define RXS_PW_MAX_DENIAL      RXS_PLM_SPX_STAT_MAX_DENIAL


/* RXS_PER_LANE_BH : Register Bits Masks Definitions */
#define RXS_PER_LANE_BH_EF_ID                        ((uint32_t)0x0000ffff)
#define RXS_PER_LANE_BH_EF_PTR                       ((uint32_t)0xffff0000)

/* RXS_LANEX_STAT0 : Register Bits Masks Definitions */
#define RXS_LANEX_STAT0_STAT2_7                      ((uint32_t)0x00000007)
#define RXS_LANEX_STAT0_STAT1                        ((uint32_t)0x00000008)
#define RXS_LANEX_STAT0_CHG_TRN                      ((uint32_t)0x00000040)
#define RXS_LANEX_STAT0_CHG_SYNC                     ((uint32_t)0x00000080)
#define RXS_LANEX_STAT0_ERR_CNT                      ((uint32_t)0x00000f00)
#define RXS_LANEX_STAT0_RX_RDY                       ((uint32_t)0x00001000)
#define RXS_LANEX_STAT0_RX_SYNC                      ((uint32_t)0x00002000)
#define RXS_LANEX_STAT0_RX_TRN                       ((uint32_t)0x00004000)
#define RXS_LANEX_STAT0_RX_INV                       ((uint32_t)0x00008000)
#define RXS_LANEX_STAT0_RX_TYPE                      ((uint32_t)0x00030000)
#define RXS_LANEX_STAT0_TX_MODE                      ((uint32_t)0x00040000)
#define RXS_LANEX_STAT0_TX_TYPE                      ((uint32_t)0x00080000)
#define RXS_LANEX_STAT0_LANE_NUM                     ((uint32_t)0x00f00000)
#define RXS_LANEX_STAT0_PORT_NUM                     ((uint32_t)0xff000000)

/* RXS_LANEX_STAT1 : Register Bits Masks Definitions */
#define RXS_LANEX_STAT1_CWR_CMPLT                    ((uint32_t)0x00000020)
#define RXS_LANEX_STAT1_CWR_FAIL                     ((uint32_t)0x00000040)
#define RXS_LANEX_STAT1_CW_CMPLT                     ((uint32_t)0x00000080)
#define RXS_LANEX_STAT1_CW_FAIL                      ((uint32_t)0x00000100)
#define RXS_LANEX_STAT1_DME_CMPLT                    ((uint32_t)0x00000200)
#define RXS_LANEX_STAT1_DME_FAIL                     ((uint32_t)0x00000400)
#define RXS_LANEX_STAT1_TRAIN_TYPE                   ((uint32_t)0x00003800)
#define RXS_LANEX_STAT1_SIG_LOST                     ((uint32_t)0x00004000)
#define RXS_LANEX_STAT1_LP_SCRM                      ((uint32_t)0x00008000)
#define RXS_LANEX_STAT1_LP_TAP_P1                    ((uint32_t)0x00030000)
#define RXS_LANEX_STAT1_LP_TAP_M1                    ((uint32_t)0x000c0000)
#define RXS_LANEX_STAT1_LP_LANE_NUM                  ((uint32_t)0x00f00000)
#define RXS_LANEX_STAT1_LP_WIDTH                     ((uint32_t)0x07000000)
#define RXS_LANEX_STAT1_LP_RX_TRN                    ((uint32_t)0x08000000)
#define RXS_LANEX_STAT1_IMPL_SPEC                    ((uint32_t)0x10000000)
#define RXS_LANEX_STAT1_CHG                          ((uint32_t)0x20000000)
#define RXS_LANEX_STAT1_INFO_OK                      ((uint32_t)0x40000000)
#define RXS_LANEX_STAT1_IDLE_RX                      ((uint32_t)0x80000000)

/* RXS_LANEX_STAT2 : Register Bits Masks Definitions */
#define RXS_LANEX_STAT2_LP_RX_W_NACK                 ((uint32_t)0x00000004)
#define RXS_LANEX_STAT2_LP_RX_W_ACK                  ((uint32_t)0x00000008)
#define RXS_LANEX_STAT2_LP_RX_W_CMD                  ((uint32_t)0x00000070)
#define RXS_LANEX_STAT2_LP_TRAINED                   ((uint32_t)0x00000080)
#define RXS_LANEX_STAT2_LP_RX_LANE_RDY               ((uint32_t)0x00000100)
#define RXS_LANEX_STAT2_LP_RX_LANES_RDY              ((uint32_t)0x00000e00)
#define RXS_LANEX_STAT2_LP_RX_W                      ((uint32_t)0x00007000)
#define RXS_LANEX_STAT2_LP_TX_1X                     ((uint32_t)0x00008000)
#define RXS_LANEX_STAT2_LP_PORT_INIT                 ((uint32_t)0x00010000)
#define RXS_LANEX_STAT2_LP_ASYM_EN                   ((uint32_t)0x00020000)
#define RXS_LANEX_STAT2_LP_RETRN_EN                  ((uint32_t)0x00040000)
#define RXS_LANEX_STAT2_LP_TX_ADJ_SUP                ((uint32_t)0x00080000)
#define RXS_LANEX_STAT2_LP_LANE                      ((uint32_t)0x00f00000)
#define RXS_LANEX_STAT2_LP_PORT                      ((uint32_t)0xff000000)

/* RXS_LANEX_STAT3 : Register Bits Masks Definitions */
#define RXS_LANEX_STAT3_SC_RSVD                      ((uint32_t)0x00000ff0)
#define RXS_LANEX_STAT3_LP_L_SILENT                  ((uint32_t)0x00001000)
#define RXS_LANEX_STAT3_LP_P_SILENT                  ((uint32_t)0x00002000)
#define RXS_LANEX_STAT3_LP_RETRN                     ((uint32_t)0x00004000)
#define RXS_LANEX_STAT3_LP_RETRN_RDY                 ((uint32_t)0x00008000)
#define RXS_LANEX_STAT3_LP_RETRN_GNT                 ((uint32_t)0x00010000)
#define RXS_LANEX_STAT3_LP_TX_EMPH_STAT              ((uint32_t)0x000e0000)
#define RXS_LANEX_STAT3_LP_TX_EMPH_CMD               ((uint32_t)0x00700000)
#define RXS_LANEX_STAT3_LP_TAP                       ((uint32_t)0x07800000)
#define RXS_LANEX_STAT3_LP_TX_SC_REQ                 ((uint32_t)0x08000000)
#define RXS_LANEX_STAT3_LP_TX_W_PEND                 ((uint32_t)0x10000000)
#define RXS_LANEX_STAT3_LP_TX_W_REQ                  ((uint32_t)0xe0000000)

/* RXS_LANEX_STAT4 : Register Bits Masks Definitions */
#define RXS_LANEX_STAT4_GEN_STAT6_CHG                ((uint32_t)0x00000001)
#define RXS_LANEX_STAT4_GEN_STAT5_CHG                ((uint32_t)0x00000002)
#define RXS_LANEX_STAT4_GEN_STAT1_CHG                ((uint32_t)0x00000004)
#define RXS_LANEX_STAT4_GEN_STAT0_CHG_TRN            ((uint32_t)0x00000008)
#define RXS_LANEX_STAT4_GEN_STAT0_CHG_SYNC           ((uint32_t)0x00000010)
#define RXS_LANEX_STAT4_STAT6_CHG                    ((uint32_t)0x02000000)
#define RXS_LANEX_STAT4_STAT5_CHG                    ((uint32_t)0x04000000)
#define RXS_LANEX_STAT4_STAT1_CHG                    ((uint32_t)0x40000000)
#define RXS_LANEX_STAT4_STAT0_CHG                    ((uint32_t)0x80000000)

/* RXS_LANEX_STAT5 : Register Bits Masks Definitions */
#define RXS_LANEX_STAT5_CM1_STAT                     ((uint32_t)0x00000003)
#define RXS_LANEX_STAT5_CP1_STAT                     ((uint32_t)0x0000000c)
#define RXS_LANEX_STAT5_C0_STAT                      ((uint32_t)0x00000070)
#define RXS_LANEX_STAT5_LOC_REMOTE_EN                ((uint32_t)0x00000080)
#define RXS_LANEX_STAT5_SENT                         ((uint32_t)0x00000100)
#define RXS_LANEX_STAT5_RETRAIN_LANE                 ((uint32_t)0x00000400)
#define RXS_LANEX_STAT5_TRAIN_LANE                   ((uint32_t)0x00000800)
#define RXS_LANEX_STAT5_CW_SELECT                    ((uint32_t)0x00001000)
#define RXS_LANEX_STAT5_DME_MODE                     ((uint32_t)0x00002000)
#define RXS_LANEX_STAT5_UNTRAINED                    ((uint32_t)0x00004000)
#define RXS_LANEX_STAT5_SILENCE_ENTERED              ((uint32_t)0x00008000)
#define RXS_LANEX_STAT5_CM1_CMD                      ((uint32_t)0x00030000)
#define RXS_LANEX_STAT5_CP1_CMD                      ((uint32_t)0x000c0000)
#define RXS_LANEX_STAT5_C0_CMD                       ((uint32_t)0x00700000)
#define RXS_LANEX_STAT5_SILENT_NOW                   ((uint32_t)0x00800000)
#define RXS_LANEX_STAT5_TAP                          ((uint32_t)0x0f000000)
#define RXS_LANEX_STAT5_RX_RDY                       ((uint32_t)0x10000000)
#define RXS_LANEX_STAT5_CHG                          ((uint32_t)0x20000000)
#define RXS_LANEX_STAT5_VALID                        ((uint32_t)0x40000000)
#define RXS_LANEX_STAT5_HW_ACK                       ((uint32_t)0x80000000)

/* RXS_LANEX_STAT6 : Register Bits Masks Definitions */
#define RXS_LANEX_STAT6_CM1_STAT                     ((uint32_t)0x00000003)
#define RXS_LANEX_STAT6_CP1_STAT                     ((uint32_t)0x0000000c)
#define RXS_LANEX_STAT6_C0_STAT                      ((uint32_t)0x00000070)
#define RXS_LANEX_STAT6_LP_REMOTE_EN                 ((uint32_t)0x00000080)
#define RXS_LANEX_STAT6_SENT                         ((uint32_t)0x00000100)
#define RXS_LANEX_STAT6_TRAINING_FAILED              ((uint32_t)0x00000200)
#define RXS_LANEX_STAT6_RETRAIN_LANE                 ((uint32_t)0x00000400)
#define RXS_LANEX_STAT6_TRAIN_LANE                   ((uint32_t)0x00000800)
#define RXS_LANEX_STAT6_CW_SELECT                    ((uint32_t)0x00001000)
#define RXS_LANEX_STAT6_DME_MODE                     ((uint32_t)0x00002000)
#define RXS_LANEX_STAT6_UNTRAINED                    ((uint32_t)0x00004000)
#define RXS_LANEX_STAT6_SILENCE_ENTERED              ((uint32_t)0x00008000)
#define RXS_LANEX_STAT6_CM1_CMD                      ((uint32_t)0x00030000)
#define RXS_LANEX_STAT6_CP1_CMD                      ((uint32_t)0x000c0000)
#define RXS_LANEX_STAT6_C0_CMD                       ((uint32_t)0x00700000)
#define RXS_LANEX_STAT6_SILENT_NOW                   ((uint32_t)0x00800000)
#define RXS_LANEX_STAT6_TAP                          ((uint32_t)0x0f000000)
#define RXS_LANEX_STAT6_RX_RDY                       ((uint32_t)0x10000000)
#define RXS_LANEX_STAT6_CHG                          ((uint32_t)0x20000000)
#define RXS_LANEX_STAT6_VALID                        ((uint32_t)0x40000000)
#define RXS_LANEX_STAT6_HW_CMD                       ((uint32_t)0x80000000)

/* RXS_SWITCH_RT_BH : Register Bits Masks Definitions */
#define RXS_SWITCH_RT_BH_EF_ID                       ((uint32_t)0x0000ffff)
#define RXS_SWITCH_RT_BH_EF_PTR                      ((uint32_t)0xffff0000)

/* RXS_BC_RT_CTL : Register Bits Masks Definitions */
#define RXS_BC_RT_CTL_MC_MASK_SZ                     ((uint32_t)0x03000000)
#define RXS_BC_RT_CTL_DEV32_RT_CTRL                  ((uint32_t)0x40000000)
#define RXS_BC_RT_CTL_THREE_LEVELS                   ((uint32_t)0x80000000)

/* RXS_BC_MC_INFO : Register Bits Masks Definitions */
#define RXS_BC_MC_INFO_MASK_PTR                      ((uint32_t)0x00fffc00)
#define RXS_BC_MC_INFO_NUM_MASKS                     ((uint32_t)0xff000000)

/* RXS_BC_RT_LVL0_INFO : Register Bits Masks Definitions */
#define RXS_BC_RT_LVL0_INFO_L0_GROUP_PTR             ((uint32_t)0x00fffc00)
#define RXS_BC_RT_LVL0_INFO_NUM_L0_GROUPS            ((uint32_t)0xff000000)

/* RXS_BC_RT_LVL1_INFO : Register Bits Masks Definitions */
#define RXS_BC_RT_LVL1_INFO_L1_GROUP_PTR             ((uint32_t)0x00fffc00)
#define RXS_BC_RT_LVL1_INFO_NUM_L1_GROUPS            ((uint32_t)0xff000000)

/* RXS_BC_RT_LVL2_INFO : Register Bits Masks Definitions */
#define RXS_BC_RT_LVL2_INFO_L2_GROUP_PTR             ((uint32_t)0x00fffc00)
#define RXS_BC_RT_LVL2_INFO_NUM_L2_GROUPS            ((uint32_t)0xff000000)

/* RXS_SPX_RT_CTL : Register Bits Masks Definitions */
#define RXS_SPX_RT_CTL_MC_MASK_SZ                    ((uint32_t)0x03000000)
#define RXS_SPX_RT_CTL_DEV32_RT_CTRL                 ((uint32_t)0x40000000)
#define RXS_SPX_RT_CTL_THREE_LEVELS                  ((uint32_t)0x80000000)

/* RXS_SPX_MC_INFO : Register Bits Masks Definitions */
#define RXS_SPX_MC_INFO_MASK_PTR                     ((uint32_t)0x00fffc00)
#define RXS_SPX_MC_INFO_NUM_MASKS                    ((uint32_t)0xff000000)

/* RXS_SPX_RT_LVL0_INFO : Register Bits Masks Definitions */
#define RXS_SPX_RT_LVL0_INFO_L0_GROUP_PTR            ((uint32_t)0x00fffc00)
#define RXS_SPX_RT_LVL0_INFO_NUM_L0_GROUPS           ((uint32_t)0xff000000)

/* RXS_SPX_RT_LVL1_INFO : Register Bits Masks Definitions */
#define RXS_SPX_RT_LVL1_INFO_L1_GROUP_PTR            ((uint32_t)0x00fffc00)
#define RXS_SPX_RT_LVL1_INFO_NUM_L1_GROUPS           ((uint32_t)0xff000000)

/* RXS_SPX_RT_LVL2_INFO : Register Bits Masks Definitions */
#define RXS_SPX_RT_LVL2_INFO_L2_GROUP_PTR            ((uint32_t)0x00fffc00)
#define RXS_SPX_RT_LVL2_INFO_NUM_L2_GROUPS           ((uint32_t)0xff000000)

/* RXS_PLM_BH : Register Bits Masks Definitions */
#define RXS_PLM_BH_BLK_TYPE                          ((uint32_t)0x00000fff)
#define RXS_PLM_BH_BLK_REV                           ((uint32_t)0x0000f000)
#define RXS_PLM_BH_NEXT_BLK_PTR                      ((uint32_t)0xffff0000)

/* RXS_PLM_SPX_IMP_SPEC_CTL : Register Bits Masks Definitions */
#define RXS_PLM_SPX_IMP_SPEC_CTL_PA_BKLOG_THRESH     ((uint32_t)0x0000003f)
#define RXS_PLM_SPX_IMP_SPEC_CTL_CONT_PNA            ((uint32_t)0x00000040)
#define RXS_PLM_SPX_IMP_SPEC_CTL_CONT_LR             ((uint32_t)0x00000080)
#define RXS_PLM_SPX_IMP_SPEC_CTL_BLIP_CS             ((uint32_t)0x00000100)
#define RXS_PLM_SPX_IMP_SPEC_CTL_LOST_CS_DIS         ((uint32_t)0x00000200)
#define RXS_PLM_SPX_IMP_SPEC_CTL_INFER_SELF_RST      ((uint32_t)0x00000400)
#define RXS_PLM_SPX_IMP_SPEC_CTL_DLT_FATAL           ((uint32_t)0x00000800)
#define RXS_PLM_SPX_IMP_SPEC_CTL_PRE_SILENCE         ((uint32_t)0x00001000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_OK2U_FATAL          ((uint32_t)0x00002000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_MAXD_FATAL          ((uint32_t)0x00004000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_DWNGD_FATAL         ((uint32_t)0x00008000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_SWAP_RX             ((uint32_t)0x00030000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_SWAP_TX             ((uint32_t)0x000c0000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_SELF_RST            ((uint32_t)0x00100000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_PORT_SELF_RST       ((uint32_t)0x00200000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_RESET_REG           ((uint32_t)0x00400000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_LLB_EN              ((uint32_t)0x00800000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_CS_FIELD1           ((uint32_t)0x01000000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_SOFT_RST_PORT       ((uint32_t)0x02000000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_FORCE_REINIT        ((uint32_t)0x04000000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_DME_TRAINING        ((uint32_t)0x08000000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_DLB_EN              ((uint32_t)0x10000000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_USE_IDLE1           ((uint32_t)0x20000000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_USE_IDLE2           ((uint32_t)0x40000000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_USE_IDLE3           ((uint32_t)0x80000000)

#define RXS_PLM_SPX_IMP_SPEC_CTL_SWAP_RX_NONE        ((uint32_t)0x00000000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_SWAP_RX_1032        ((uint32_t)0x00010000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_SWAP_RX_3210        ((uint32_t)0x00020000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_SWAP_RX_2301        ((uint32_t)0x00030000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_SWAP_TX_NONE        ((uint32_t)0x00000000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_SWAP_TX_1032        ((uint32_t)0x00040000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_SWAP_TX_3210        ((uint32_t)0x00080000)
#define RXS_PLM_SPX_IMP_SPEC_CTL_SWAP_TX_2301        ((uint32_t)0x000C0000)

/* RXS_PLM_SPX_PWDN_CTL : Register Bits Masks Definitions */
#define RXS_PLM_SPX_PWDN_CTL_PWDN_PORT               ((uint32_t)0x00000001)

/* RXS_PLM_SPX_1WR : Register Bits Masks Definitions */
#define RXS_PLM_SPX_1WR_TIMEOUT                      ((uint32_t)0x0000000f)
#define RXS_PLM_SPX_1WR_PORT_SELECT                  ((uint32_t)0x00000f00)
#define RXS_PLM_SPX_1WR_PATH_MODE                    ((uint32_t)0x00070000)
#define RXS_PLM_SPX_1WR_TX_MODE                      ((uint32_t)0x00800000)
#define RXS_PLM_SPX_1WR_IDLE_SEQ                     ((uint32_t)0x03000000)
#define RXS_PLM_SPX_1WR_BAUD_EN                      ((uint32_t)0xf0000000)

#define RXS_PLM_SPX_1WR_IDLE_SEQ_DFLT                ((uint32_t)0x00000000)
#define RXS_PLM_SPX_1WR_IDLE_SEQ_1                   ((uint32_t)0x01000000)
#define RXS_PLM_SPX_1WR_IDLE_SEQ_2                   ((uint32_t)0x02000000)
#define RXS_PLM_SPX_1WR_IDLE_SEQ_3                   ((uint32_t)0x03000000)

#define RXS_PLM_SPX_1WR_BAUD_EN_2P5                  ((uint32_t)0x20000000)
#define RXS_PLM_SPX_1WR_BAUD_EN_3P125                ((uint32_t)0x30000000)
#define RXS_PLM_SPX_1WR_BAUD_EN_5P0                  ((uint32_t)0x40000000)
#define RXS_PLM_SPX_1WR_BAUD_EN_6P25                 ((uint32_t)0x50000000)
#define RXS_PLM_SPX_1WR_BAUD_EN_10P3                 ((uint32_t)0x80000000)
#define RXS_PLM_SPX_1WR_BAUD_EN_12P5                 ((uint32_t)0x90000000)

/* RXS_PLM_SPX_MULT_ACK_CTL : Register Bits Masks Definitions */
#define RXS_PLM_SPX_MULT_ACK_CTL_MULT_ACK_DLY        ((uint32_t)0x000000ff)

/* RXS_PLM_SPX_STAT : Register Bits Masks Definitions */
#define RXS_PLM_SPX_STAT_BERM_0                      ((uint32_t)0x00000001)
#define RXS_PLM_SPX_STAT_BERM_1                      ((uint32_t)0x00000002)
#define RXS_PLM_SPX_STAT_BERM_2                      ((uint32_t)0x00000004)
#define RXS_PLM_SPX_STAT_BERM_3                      ((uint32_t)0x00000008)
#define RXS_PLM_SPX_STAT_TLM_INT                     ((uint32_t)0x00000400)
#define RXS_PLM_SPX_STAT_PBM_INT                     ((uint32_t)0x00000800)
#define RXS_PLM_SPX_STAT_MECS                        ((uint32_t)0x00001000)
#define RXS_PLM_SPX_STAT_TLM_PW                      ((uint32_t)0x00004000)
#define RXS_PLM_SPX_STAT_PBM_PW                      ((uint32_t)0x00008000)
#define RXS_PLM_SPX_STAT_RST_REQ                     ((uint32_t)0x00010000)
#define RXS_PLM_SPX_STAT_PRST_REQ                    ((uint32_t)0x00020000)
#define RXS_PLM_SPX_STAT_EL_INTA                     ((uint32_t)0x00040000)
#define RXS_PLM_SPX_STAT_EL_INTB                     ((uint32_t)0x00080000)
#define RXS_PLM_SPX_STAT_II_CHG_0                    ((uint32_t)0x00100000)
#define RXS_PLM_SPX_STAT_II_CHG_1                    ((uint32_t)0x00200000)
#define RXS_PLM_SPX_STAT_II_CHG_2                    ((uint32_t)0x00400000)
#define RXS_PLM_SPX_STAT_II_CHG_3                    ((uint32_t)0x00800000)
#define RXS_PLM_SPX_STAT_PCAP                        ((uint32_t)0x01000000)
#define RXS_PLM_SPX_STAT_DWNGD                       ((uint32_t)0x02000000)
#define RXS_PLM_SPX_STAT_PBM_FATAL                   ((uint32_t)0x04000000)
#define RXS_PLM_SPX_STAT_PORT_ERR                    ((uint32_t)0x08000000)
#define RXS_PLM_SPX_STAT_LINK_INIT                   ((uint32_t)0x10000000)
#define RXS_PLM_SPX_STAT_DLT                         ((uint32_t)0x20000000)
#define RXS_PLM_SPX_STAT_OK_TO_UNINIT                ((uint32_t)0x40000000)
#define RXS_PLM_SPX_STAT_MAX_DENIAL                  ((uint32_t)0x80000000)

// These events are steered to port-write and/or interrupt notification
// by means other that the PLM port-write and interrupt enables.
#define RXS_PLM_SPX_UNMASKABLE_MASK (RXS_PLM_SPX_STAT_TLM_INT | \
				RXS_PLM_SPX_STAT_PBM_INT | \
				RXS_PLM_SPX_STAT_MECS | \
				RXS_PLM_SPX_STAT_TLM_PW | \
				RXS_PLM_SPX_STAT_PBM_PW | \
				RXS_PLM_SPX_STAT_RST_REQ | \
				RXS_PLM_SPX_STAT_PRST_REQ)

/* RXS_PLM_SPX_INT_EN : Register Bits Masks Definitions */
#define RXS_PLM_SPX_INT_EN_BERM_0                    ((uint32_t)0x00000001)
#define RXS_PLM_SPX_INT_EN_BERM_1                    ((uint32_t)0x00000002)
#define RXS_PLM_SPX_INT_EN_BERM_2                    ((uint32_t)0x00000004)
#define RXS_PLM_SPX_INT_EN_BERM_3                    ((uint32_t)0x00000008)
#define RXS_PLM_SPX_INT_EN_EL_INTA                   ((uint32_t)0x00040000)
#define RXS_PLM_SPX_INT_EN_EL_INTB                   ((uint32_t)0x00080000)
#define RXS_PLM_SPX_INT_EN_II_CHG_0                  ((uint32_t)0x00100000)
#define RXS_PLM_SPX_INT_EN_II_CHG_1                  ((uint32_t)0x00200000)
#define RXS_PLM_SPX_INT_EN_II_CHG_2                  ((uint32_t)0x00400000)
#define RXS_PLM_SPX_INT_EN_II_CHG_3                  ((uint32_t)0x00800000)
#define RXS_PLM_SPX_INT_EN_PCAP                      ((uint32_t)0x01000000)
#define RXS_PLM_SPX_INT_EN_DWNGD                     ((uint32_t)0x02000000)
#define RXS_PLM_SPX_INT_EN_PBM_FATAL                 ((uint32_t)0x04000000)
#define RXS_PLM_SPX_INT_EN_PORT_ERR                  ((uint32_t)0x08000000)
#define RXS_PLM_SPX_INT_EN_LINK_INIT                 ((uint32_t)0x10000000)
#define RXS_PLM_SPX_INT_EN_DLT                       ((uint32_t)0x20000000)
#define RXS_PLM_SPX_INT_EN_OK_TO_UNINIT              ((uint32_t)0x40000000)
#define RXS_PLM_SPX_INT_EN_MAX_DENIAL                ((uint32_t)0x80000000)

/* RXS_PLM_SPX_PW_EN : Register Bits Masks Definitions */
#define RXS_PLM_SPX_PW_EN_BERM_0                     ((uint32_t)0x00000001)
#define RXS_PLM_SPX_PW_EN_BERM_1                     ((uint32_t)0x00000002)
#define RXS_PLM_SPX_PW_EN_BERM_2                     ((uint32_t)0x00000004)
#define RXS_PLM_SPX_PW_EN_BERM_3                     ((uint32_t)0x00000008)
#define RXS_PLM_SPX_PW_EN_EL_INTA                    ((uint32_t)0x00040000)
#define RXS_PLM_SPX_PW_EN_EL_INTB                    ((uint32_t)0x00080000)
#define RXS_PLM_SPX_PW_EN_II_CHG_0                   ((uint32_t)0x00100000)
#define RXS_PLM_SPX_PW_EN_II_CHG_1                   ((uint32_t)0x00200000)
#define RXS_PLM_SPX_PW_EN_II_CHG_2                   ((uint32_t)0x00400000)
#define RXS_PLM_SPX_PW_EN_II_CHG_3                   ((uint32_t)0x00800000)
#define RXS_PLM_SPX_PW_EN_PCAP                       ((uint32_t)0x01000000)
#define RXS_PLM_SPX_PW_EN_DWNGD                      ((uint32_t)0x02000000)
#define RXS_PLM_SPX_PW_EN_PBM_FATAL                  ((uint32_t)0x04000000)
#define RXS_PLM_SPX_PW_EN_PORT_ERR                   ((uint32_t)0x08000000)
#define RXS_PLM_SPX_PW_EN_LINK_INIT                  ((uint32_t)0x10000000)
#define RXS_PLM_SPX_PW_EN_DLT                        ((uint32_t)0x20000000)
#define RXS_PLM_SPX_PW_EN_OK_TO_UNINIT               ((uint32_t)0x40000000)
#define RXS_PLM_SPX_PW_EN_MAX_DENIAL                 ((uint32_t)0x80000000)

/* RXS_PLM_SPX_EVENT_GEN : Register Bits Masks Definitions */
#define RXS_PLM_SPX_EVENT_GEN_MECS                   ((uint32_t)0x00001000)
#define RXS_PLM_SPX_EVENT_GEN_RST_REQ                ((uint32_t)0x00010000)
#define RXS_PLM_SPX_EVENT_GEN_PRST_REQ               ((uint32_t)0x00020000)
#define RXS_PLM_SPX_EVENT_GEN_DWNGD                  ((uint32_t)0x02000000)
#define RXS_PLM_SPX_EVENT_GEN_PBM_FATAL              ((uint32_t)0x04000000)
#define RXS_PLM_SPX_EVENT_GEN_PORT_ERR               ((uint32_t)0x08000000)
#define RXS_PLM_SPX_EVENT_GEN_MAX_DENIAL             ((uint32_t)0x80000000)

/* RXS_PLM_SPX_ALL_INT_EN : Register Bits Masks Definitions */
#define RXS_PLM_SPX_ALL_INT_EN_IRQ_EN                ((uint32_t)0x00000001)

/* RXS_PLM_SPX_PATH_CTL : Register Bits Masks Definitions */
#define RXS_PLM_SPX_PATH_CTL_PATH_MODE               ((uint32_t)0x00000007)
#define RXS_PLM_SPX_PATH_CTL_PATH_CONFIGURATION      ((uint32_t)0x00000700)
#define RXS_PLM_SPX_PATH_CTL_PATH_ID                 ((uint32_t)0x001f0000)

/* RXS_PLM_SPX_SILENCE_TMR : Register Bits Masks Definitions */
#define RXS_PLM_SPX_SILENCE_TMR_TENB_PATTERN         ((uint32_t)0x000fffff)
#define RXS_PLM_SPX_SILENCE_TMR_SILENCE_TMR          ((uint32_t)0xfff00000)

/* RXS_PLM_SPX_VMIN_EXP : Register Bits Masks Definitions */
#define RXS_PLM_SPX_VMIN_EXP_DS_MIN                  ((uint32_t)0x000000ff)
#define RXS_PLM_SPX_VMIN_EXP_MMAX                    ((uint32_t)0x00000f00)
#define RXS_PLM_SPX_VMIN_EXP_IMAX                    ((uint32_t)0x000f0000)
#define RXS_PLM_SPX_VMIN_EXP_VMIN_EXP                ((uint32_t)0x1f000000)
#define RXS_PLM_SPX_VMIN_EXP_DISC_IGNORE_LOLS        ((uint32_t)0x40000000)
#define RXS_PLM_SPX_VMIN_EXP_LOLS_RECOV_DIS          ((uint32_t)0x80000000)

/* RXS_PLM_SPX_POL_CTL : Register Bits Masks Definitions */
#define RXS_PLM_SPX_POL_CTL_RX0_POL                      ((uint32_t)0x00000001)
#define RXS_PLM_SPX_POL_CTL_RX1_POL                      ((uint32_t)0x00000002)
#define RXS_PLM_SPX_POL_CTL_RX2_POL                      ((uint32_t)0x00000004)
#define RXS_PLM_SPX_POL_CTL_RX3_POL                      ((uint32_t)0x00000008)
#define RXS_PLM_SPX_POL_CTL_TX0_POL                      ((uint32_t)0x00010000)
#define RXS_PLM_SPX_POL_CTL_TX1_POL                      ((uint32_t)0x00020000)
#define RXS_PLM_SPX_POL_CTL_TX2_POL                      ((uint32_t)0x00040000)
#define RXS_PLM_SPX_POL_CTL_TX3_POL                      ((uint32_t)0x00080000)

#define RXS_PLM_SPX_POL_CTL_RX_ALL_POL                   ((uint32_t)0x0000000F)
#define RXS_PLM_SPX_POL_CTL_TX_ALL_POL                   ((uint32_t)0x000F0000)

/* RXS_PLM_SPX_CLKCOMP_CTL : Register Bits Masks Definitions */
#define RXS_PLM_SPX_CLKCOMP_CTL_CLK_COMP_CNT         ((uint32_t)0x00001fff)

/* RXS_PLM_SPX_DENIAL_CTL : Register Bits Masks Definitions */
#define RXS_PLM_SPX_DENIAL_CTL_DENIAL_THRESH         ((uint32_t)0x0000ffff)
#define RXS_PLM_SPX_DENIAL_CTL_CNT_RTY               ((uint32_t)0x10000000)
#define RXS_PLM_SPX_DENIAL_CTL_CNT_PNA               ((uint32_t)0x20000000)

/* RXS_PLM_SPX_ERR_REC_CTL : Register Bits Masks Definitions */
#define RXS_PLM_SPX_ERR_REC_CTL_LREQ_LIMIT           ((uint32_t)0x003fffff)

/* RXS_PLM_SPX_CS_TX1 : Register Bits Masks Definitions */
#define RXS_PLM_SPX_CS_TX1_PAR_1                     ((uint32_t)0x0000fff0)
#define RXS_PLM_SPX_CS_TX1_PAR_0                     ((uint32_t)0x0fff0000)
#define RXS_PLM_SPX_CS_TX1_STYPE_0                   ((uint32_t)0xf0000000)

/* RXS_PLM_SPX_CS_TX2 : Register Bits Masks Definitions */
#define RXS_PLM_SPX_CS_TX2_CMD                       ((uint32_t)0x00000700)
#define RXS_PLM_SPX_CS_TX2_STYPE_1                   ((uint32_t)0x00003800)
#define RXS_PLM_SPX_CS_TX2_STYPE1_CS64               ((uint32_t)0x0000c000)
#define RXS_PLM_SPX_CS_TX2_PARM                      ((uint32_t)0x1fff0000)
#define RXS_PLM_SPX_CS_TX2_STYPE2                    ((uint32_t)0x20000000)

/* RXS_PLM_SPX_PNA_CAP : Register Bits Masks Definitions */
#define RXS_PLM_SPX_PNA_CAP_PARM_1                   ((uint32_t)0x00000fff)
#define RXS_PLM_SPX_PNA_CAP_PARM_0                   ((uint32_t)0x0fff0000)
#define RXS_PLM_SPX_PNA_CAP_VALID                    ((uint32_t)0x80000000)

/* RXS_PLM_SPX_ACKID_CAP : Register Bits Masks Definitions */
#define RXS_PLM_SPX_ACKID_CAP_ACKID                  ((uint32_t)0x00000fff)
#define RXS_PLM_SPX_ACKID_CAP_VALID                  ((uint32_t)0x80000000)

/* RXS_PLM_SPX_SCRATCHY : Register Bits Masks Definitions */
#define RXS_PLM_SPX_SCRATCHY_SCRATCH                 ((uint32_t)0xffffffff)

/* RXS_TLM_BH : Register Bits Masks Definitions */
#define RXS_TLM_BH_BLK_TYPE                          ((uint32_t)0x00000fff)
#define RXS_TLM_BH_BLK_REV                           ((uint32_t)0x0000f000)
#define RXS_TLM_BH_NEXT_BLK_PTR                      ((uint32_t)0xffff0000)

/* RXS_TLM_SPX_CONTROL : Register Bits Masks Definitions */
#define RXS_TLM_SPX_CONTROL_LENGTH                   ((uint32_t)0x00001f00)
#define RXS_TLM_SPX_CONTROL_BLIP_CRC32               ((uint32_t)0x01000000)
#define RXS_TLM_SPX_CONTROL_BLIP_CRC16               ((uint32_t)0x02000000)
#define RXS_TLM_SPX_CONTROL_VOQ_SELECT               ((uint32_t)0x30000000)
#define RXS_TLM_SPX_CONTROL_PORTGROUP_SELECT         ((uint32_t)0x40000000)

/* RXS_TLM_SPX_STAT : Register Bits Masks Definitions */
#define RXS_TLM_SPX_STAT_IG_LUT_UNCOR                ((uint32_t)0x00010000)
#define RXS_TLM_SPX_STAT_IG_LUT_COR                  ((uint32_t)0x00020000)
#define RXS_TLM_SPX_STAT_EG_BAD_CRC                  ((uint32_t)0x00040000)
#define RXS_TLM_SPX_STAT_LUT_DISCARD                 ((uint32_t)0x00400000)
#define RXS_TLM_SPX_STAT_IG_FTYPE_FILTER             ((uint32_t)0x00800000)
#define RXS_TLM_SPX_STAT_IG_BAD_VC                   ((uint32_t)0x80000000)

/* RXS_TLM_SPX_INT_EN : Register Bits Masks Definitions */
#define RXS_TLM_SPX_INT_EN_IG_LUT_UNCOR              ((uint32_t)0x00010000)
#define RXS_TLM_SPX_INT_EN_IG_LUT_COR                ((uint32_t)0x00020000)
#define RXS_TLM_SPX_INT_EN_EG_BAD_CRC                ((uint32_t)0x00040000)
#define RXS_TLM_SPX_INT_EN_LUT_DISCARD               ((uint32_t)0x00400000)
#define RXS_TLM_SPX_INT_EN_IG_FTYPE_FILTER           ((uint32_t)0x00800000)
#define RXS_TLM_SPX_INT_EN_IG_BAD_VC                 ((uint32_t)0x80000000)

/* RXS_TLM_SPX_PW_EN : Register Bits Masks Definitions */
#define RXS_TLM_SPX_PW_EN_IG_LUT_UNCOR               ((uint32_t)0x00010000)
#define RXS_TLM_SPX_PW_EN_IG_LUT_COR                 ((uint32_t)0x00020000)
#define RXS_TLM_SPX_PW_EN_EG_BAD_CRC                 ((uint32_t)0x00040000)
#define RXS_TLM_SPX_PW_EN_LUT_DISCARD                ((uint32_t)0x00400000)
#define RXS_TLM_SPX_PW_EN_IG_FTYPE_FILTER            ((uint32_t)0x00800000)
#define RXS_TLM_SPX_PW_EN_IG_BAD_VC                  ((uint32_t)0x80000000)

/* RXS_TLM_SPX_EVENT_GEN : Register Bits Masks Definitions */
#define RXS_TLM_SPX_EVENT_GEN_IG_LUT_UNCOR           ((uint32_t)0x00010000)
#define RXS_TLM_SPX_EVENT_GEN_IG_LUT_COR             ((uint32_t)0x00020000)
#define RXS_TLM_SPX_EVENT_GEN_EG_BAD_CRC             ((uint32_t)0x00040000)
#define RXS_TLM_SPX_EVENT_GEN_LUT_DISCARD            ((uint32_t)0x00400000)
#define RXS_TLM_SPX_EVENT_GEN_IG_FTYPE_FILTER        ((uint32_t)0x00800000)
#define RXS_TLM_SPX_EVENT_GEN_IG_BAD_VC              ((uint32_t)0x80000000)

/* RXS_TLM_SPX_FTYPE_FILT : Register Bits Masks Definitions */
#define RXS_TLM_SPX_FTYPE_FILT_F15_IMP               ((uint32_t)0x00000002)
#define RXS_TLM_SPX_FTYPE_FILT_F14_RSVD              ((uint32_t)0x00000004)
#define RXS_TLM_SPX_FTYPE_FILT_F13_OTHER             ((uint32_t)0x00000008)
#define RXS_TLM_SPX_FTYPE_FILT_F13_RESPONSE_DATA     ((uint32_t)0x00000010)
#define RXS_TLM_SPX_FTYPE_FILT_F13_RESPONSE          ((uint32_t)0x00000020)
#define RXS_TLM_SPX_FTYPE_FILT_F12_RSVD              ((uint32_t)0x00000040)
#define RXS_TLM_SPX_FTYPE_FILT_F11_MSG               ((uint32_t)0x00000080)
#define RXS_TLM_SPX_FTYPE_FILT_F10_DOORBELL          ((uint32_t)0x00000100)
#define RXS_TLM_SPX_FTYPE_FILT_F9_DATA_STREAMING     ((uint32_t)0x00000200)
#define RXS_TLM_SPX_FTYPE_FILT_F8_OTHER              ((uint32_t)0x00000400)
#define RXS_TLM_SPX_FTYPE_FILT_F8_PWR                ((uint32_t)0x00000800)
#define RXS_TLM_SPX_FTYPE_FILT_F8_MWR                ((uint32_t)0x00001000)
#define RXS_TLM_SPX_FTYPE_FILT_F8_MRR                ((uint32_t)0x00002000)
#define RXS_TLM_SPX_FTYPE_FILT_F8_MW                 ((uint32_t)0x00004000)
#define RXS_TLM_SPX_FTYPE_FILT_F8_MR                 ((uint32_t)0x00008000)
#define RXS_TLM_SPX_FTYPE_FILT_F7_FLOW               ((uint32_t)0x00010000)
#define RXS_TLM_SPX_FTYPE_FILT_F6_STREAMING_WRITE    ((uint32_t)0x00020000)
#define RXS_TLM_SPX_FTYPE_FILT_F5_OTHER              ((uint32_t)0x00040000)
#define RXS_TLM_SPX_FTYPE_FILT_F5_ATOMIC             ((uint32_t)0x00080000)
#define RXS_TLM_SPX_FTYPE_FILT_F5_NWRITE_R           ((uint32_t)0x00100000)
#define RXS_TLM_SPX_FTYPE_FILT_F5_NWRITE             ((uint32_t)0x00200000)
#define RXS_TLM_SPX_FTYPE_FILT_F5_GSM                ((uint32_t)0x00400000)
#define RXS_TLM_SPX_FTYPE_FILT_F4_RSVD               ((uint32_t)0x00800000)
#define RXS_TLM_SPX_FTYPE_FILT_F3_RSVD               ((uint32_t)0x01000000)
#define RXS_TLM_SPX_FTYPE_FILT_F2_ATOMIC             ((uint32_t)0x02000000)
#define RXS_TLM_SPX_FTYPE_FILT_F2_NREAD              ((uint32_t)0x04000000)
#define RXS_TLM_SPX_FTYPE_FILT_F2_GSM                ((uint32_t)0x08000000)
#define RXS_TLM_SPX_FTYPE_FILT_F1_INTERVENTION       ((uint32_t)0x10000000)
#define RXS_TLM_SPX_FTYPE_FILT_F0_IMP                ((uint32_t)0x40000000)

/* RXS_TLM_SPX_FTYPE_CAPT : Register Bits Masks Definitions */
#define RXS_TLM_SPX_FTYPE_CAPT_F15_IMP               ((uint32_t)0x00000002)
#define RXS_TLM_SPX_FTYPE_CAPT_F14_RSVD              ((uint32_t)0x00000004)
#define RXS_TLM_SPX_FTYPE_CAPT_F13_OTHER             ((uint32_t)0x00000008)
#define RXS_TLM_SPX_FTYPE_CAPT_F13_RESPONSE_DATA     ((uint32_t)0x00000010)
#define RXS_TLM_SPX_FTYPE_CAPT_F13_RESPONSE          ((uint32_t)0x00000020)
#define RXS_TLM_SPX_FTYPE_CAPT_F12_RSVD              ((uint32_t)0x00000040)
#define RXS_TLM_SPX_FTYPE_CAPT_F11_MSG               ((uint32_t)0x00000080)
#define RXS_TLM_SPX_FTYPE_CAPT_F10_DOORBELL          ((uint32_t)0x00000100)
#define RXS_TLM_SPX_FTYPE_CAPT_F9_DATA_STREAMING     ((uint32_t)0x00000200)
#define RXS_TLM_SPX_FTYPE_CAPT_F8_OTHER              ((uint32_t)0x00000400)
#define RXS_TLM_SPX_FTYPE_CAPT_F8_PWR                ((uint32_t)0x00000800)
#define RXS_TLM_SPX_FTYPE_CAPT_F8_MWR                ((uint32_t)0x00001000)
#define RXS_TLM_SPX_FTYPE_CAPT_F8_MRR                ((uint32_t)0x00002000)
#define RXS_TLM_SPX_FTYPE_CAPT_F8_MW                 ((uint32_t)0x00004000)
#define RXS_TLM_SPX_FTYPE_CAPT_F8_MR                 ((uint32_t)0x00008000)
#define RXS_TLM_SPX_FTYPE_CAPT_F7_FLOW               ((uint32_t)0x00010000)
#define RXS_TLM_SPX_FTYPE_CAPT_F6_STREAMING_WRITE    ((uint32_t)0x00020000)
#define RXS_TLM_SPX_FTYPE_CAPT_F5_OTHER              ((uint32_t)0x00040000)
#define RXS_TLM_SPX_FTYPE_CAPT_F5_ATOMIC             ((uint32_t)0x00080000)
#define RXS_TLM_SPX_FTYPE_CAPT_F5_NWRITE_R           ((uint32_t)0x00100000)
#define RXS_TLM_SPX_FTYPE_CAPT_F5_NWRITE             ((uint32_t)0x00200000)
#define RXS_TLM_SPX_FTYPE_CAPT_F5_GSM                ((uint32_t)0x00400000)
#define RXS_TLM_SPX_FTYPE_CAPT_F4_RSVD               ((uint32_t)0x00800000)
#define RXS_TLM_SPX_FTYPE_CAPT_F3_RSVD               ((uint32_t)0x01000000)
#define RXS_TLM_SPX_FTYPE_CAPT_F2_ATOMIC             ((uint32_t)0x02000000)
#define RXS_TLM_SPX_FTYPE_CAPT_F2_NREAD              ((uint32_t)0x04000000)
#define RXS_TLM_SPX_FTYPE_CAPT_F2_GSM                ((uint32_t)0x08000000)
#define RXS_TLM_SPX_FTYPE_CAPT_F1_INTERVENTION       ((uint32_t)0x10000000)
#define RXS_TLM_SPX_FTYPE_CAPT_F0_IMP                ((uint32_t)0x40000000)

/* RXS_TLM_SPX_MTC_ROUTE_EN : Register Bits Masks Definitions */
#define RXS_TLM_SPX_MTC_ROUTE_EN_MTC_EN              ((uint32_t)0x00000001)

/* RXS_TLM_SPX_ROUTE_EN : Register Bits Masks Definitions */
#define RXS_TLM_SPX_ROUTE_EN_RT_EN                   ((uint32_t)0x0000ffff)
#define RXS_TLM_SPX_ROUTE_EN_UNUSED                  ((uint32_t)0x00ff0000)

/* RXS_PBM_BH : Register Bits Masks Definitions */
#define RXS_PBM_BH_BLK_TYPE                          ((uint32_t)0x00000fff)
#define RXS_PBM_BH_BLK_REV                           ((uint32_t)0x0000f000)
#define RXS_PBM_BH_NEXT_BLK_PTR                      ((uint32_t)0xffff0000)

/* RXS_PBM_SPX_CONTROL : Register Bits Masks Definitions */
#define RXS_PBM_SPX_CONTROL_EG_REORDER_STICK         ((uint32_t)0x00000007)
#define RXS_PBM_SPX_CONTROL_EG_REORDER_MODE          ((uint32_t)0x00000030)
#define RXS_PBM_SPX_CONTROL_EG_STORE_MODE            ((uint32_t)0x00001000)

/* RXS_PBM_SPX_STAT : Register Bits Masks Definitions */
#define RXS_PBM_SPX_STAT_EG_BABBLE_PACKET            ((uint32_t)0x00000001)
#define RXS_PBM_SPX_STAT_EG_BAD_CHANNEL              ((uint32_t)0x00000002)
#define RXS_PBM_SPX_STAT_EG_CRQ_OVERFLOW             ((uint32_t)0x00000008)
#define RXS_PBM_SPX_STAT_EG_DATA_OVERFLOW            ((uint32_t)0x00000010)
#define RXS_PBM_SPX_STAT_EG_DNFL_FATAL               ((uint32_t)0x00000020)
#define RXS_PBM_SPX_STAT_EG_DNFL_COR                 ((uint32_t)0x00000040)
#define RXS_PBM_SPX_STAT_EG_DOH_FATAL                ((uint32_t)0x00000080)
#define RXS_PBM_SPX_STAT_EG_DOH_COR                  ((uint32_t)0x00000100)
#define RXS_PBM_SPX_STAT_EG_TTL_EXPIRED              ((uint32_t)0x00000200)
#define RXS_PBM_SPX_STAT_EG_DATA_UNCOR               ((uint32_t)0x00000800)
#define RXS_PBM_SPX_STAT_EG_DATA_COR                 ((uint32_t)0x00001000)
#define RXS_PBM_SPX_STAT_EG_EMPTY                    ((uint32_t)0x00008000)

/* RXS_PBM_SPX_INT_EN : Register Bits Masks Definitions */
#define RXS_PBM_SPX_INT_EN_EG_BABBLE_PACKET          ((uint32_t)0x00000001)
#define RXS_PBM_SPX_INT_EN_EG_BAD_CHANNEL            ((uint32_t)0x00000002)
#define RXS_PBM_SPX_INT_EN_EG_CRQ_OVERFLOW           ((uint32_t)0x00000008)
#define RXS_PBM_SPX_INT_EN_EG_DATA_OVERFLOW          ((uint32_t)0x00000010)
#define RXS_PBM_SPX_INT_EN_EG_DNFL_FATAL             ((uint32_t)0x00000020)
#define RXS_PBM_SPX_INT_EN_EG_DNFL_COR               ((uint32_t)0x00000040)
#define RXS_PBM_SPX_INT_EN_EG_DOH_FATAL              ((uint32_t)0x00000080)
#define RXS_PBM_SPX_INT_EN_EG_DOH_COR                ((uint32_t)0x00000100)
#define RXS_PBM_SPX_INT_EN_EG_TTL_EXPIRED            ((uint32_t)0x00000200)
#define RXS_PBM_SPX_INT_EN_EG_DATA_UNCOR             ((uint32_t)0x00000800)
#define RXS_PBM_SPX_INT_EN_EG_DATA_COR               ((uint32_t)0x00001000)

/* RXS_PBM_SPX_PW_EN : Register Bits Masks Definitions */
#define RXS_PBM_SPX_PW_EN_EG_BABBLE_PACKET           ((uint32_t)0x00000001)
#define RXS_PBM_SPX_PW_EN_EG_BAD_CHANNEL             ((uint32_t)0x00000002)
#define RXS_PBM_SPX_PW_EN_EG_CRQ_OVERFLOW            ((uint32_t)0x00000008)
#define RXS_PBM_SPX_PW_EN_EG_DATA_OVERFLOW           ((uint32_t)0x00000010)
#define RXS_PBM_SPX_PW_EN_EG_DNFL_FATAL              ((uint32_t)0x00000020)
#define RXS_PBM_SPX_PW_EN_EG_DNFL_COR                ((uint32_t)0x00000040)
#define RXS_PBM_SPX_PW_EN_EG_DOH_FATAL               ((uint32_t)0x00000080)
#define RXS_PBM_SPX_PW_EN_EG_DOH_COR                 ((uint32_t)0x00000100)
#define RXS_PBM_SPX_PW_EN_EG_TTL_EXPIRED             ((uint32_t)0x00000200)
#define RXS_PBM_SPX_PW_EN_EG_DATA_UNCOR              ((uint32_t)0x00000800)
#define RXS_PBM_SPX_PW_EN_EG_DATA_COR                ((uint32_t)0x00001000)

/* RXS_PBM_SPX_EVENT_GEN : Register Bits Masks Definitions */
#define RXS_PBM_SPX_EVENT_GEN_EG_BABBLE_PACKET       ((uint32_t)0x00000001)
#define RXS_PBM_SPX_EVENT_GEN_EG_BAD_CHANNEL         ((uint32_t)0x00000002)
#define RXS_PBM_SPX_EVENT_GEN_EG_CRQ_OVERFLOW        ((uint32_t)0x00000008)
#define RXS_PBM_SPX_EVENT_GEN_EG_DATA_OVERFLOW       ((uint32_t)0x00000010)
#define RXS_PBM_SPX_EVENT_GEN_EG_DNFL_FATAL          ((uint32_t)0x00000020)
#define RXS_PBM_SPX_EVENT_GEN_EG_DNFL_COR            ((uint32_t)0x00000040)
#define RXS_PBM_SPX_EVENT_GEN_EG_DOH_FATAL           ((uint32_t)0x00000080)
#define RXS_PBM_SPX_EVENT_GEN_EG_DOH_COR             ((uint32_t)0x00000100)
#define RXS_PBM_SPX_EVENT_GEN_EG_TTL_EXPIRED         ((uint32_t)0x00000200)
#define RXS_PBM_SPX_EVENT_GEN_EG_DATA_UNCOR          ((uint32_t)0x00000800)
#define RXS_PBM_SPX_EVENT_GEN_EG_DATA_COR            ((uint32_t)0x00001000)

/* RXS_PBM_SPX_EG_RESOURCES : Register Bits Masks Definitions */
#define RXS_PBM_SPX_EG_RESOURCES_CRQ_ENTRIES         ((uint32_t)0x000000ff)
#define RXS_PBM_SPX_EG_RESOURCES_DATANODES           ((uint32_t)0x03ff0000)

/* RXS_PBM_SPX_BUFF_STATUS : Register Bits Masks Definitions */
#define RXS_PBM_SPX_BUFF_STATUS_EG_DATA_F            ((uint32_t)0x00200000)
#define RXS_PBM_SPX_BUFF_STATUS_EG_CRQ_F             ((uint32_t)0x00400000)
#define RXS_PBM_SPX_BUFF_STATUS_EG_MT                ((uint32_t)0x00800000)

/* RXS_PBM_SPX_SCRATCH1 : Register Bits Masks Definitions */
#define RXS_PBM_SPX_SCRATCH1_SCRATCH                 ((uint32_t)0xffffffff)

/* RXS_PBM_SPX_SCRATCH2 : Register Bits Masks Definitions */
#define RXS_PBM_SPX_SCRATCH2_SCRATCH                 ((uint32_t)0xffffffff)

/* RXS_PCNTR_BH : Register Bits Masks Definitions */
#define RXS_PCNTR_BH_BLK_TYPE                        ((uint32_t)0x00000fff)
#define RXS_PCNTR_BH_BLK_REV                         ((uint32_t)0x0000f000)
#define RXS_PCNTR_BH_NEXT_BLK_PTR                    ((uint32_t)0xffff0000)

/* RXS_PCNTR_CTL : Register Bits Masks Definitions */
#define RXS_PCNTR_CTL_CNTR_CLR                       ((uint32_t)0x40000000)
#define RXS_PCNTR_CTL_CNTR_FRZ                       ((uint32_t)0x80000000)

/* RXS_SPX_PCNTR_EN : Register Bits Masks Definitions */
#define RXS_SPX_PCNTR_EN_ENABLE                      ((uint32_t)0x80000000)

/* RXS_SPX_PCNTR_CTL : Register Bits Masks Definitions */
#define RXS_SPX_PCNTR_CTL_TX                         ((uint32_t)0x00000080)
#define RXS_SPX_PCNTR_CTL_PRIO0                      ((uint32_t)0x00000100)
#define RXS_SPX_PCNTR_CTL_PRIO0C                     ((uint32_t)0x00000200)
#define RXS_SPX_PCNTR_CTL_PRIO1                      ((uint32_t)0x00000400)
#define RXS_SPX_PCNTR_CTL_PRIO1C                     ((uint32_t)0x00000800)
#define RXS_SPX_PCNTR_CTL_PRIO2                      ((uint32_t)0x00001000)
#define RXS_SPX_PCNTR_CTL_PRIO2C                     ((uint32_t)0x00002000)
#define RXS_SPX_PCNTR_CTL_PRIO3                      ((uint32_t)0x00004000)
#define RXS_SPX_PCNTR_CTL_PRIO3C                     ((uint32_t)0x00008000)
#define RXS_SPC_PCNTR_CTL_PRIO  ((uint32_t)(RXS_SPX_PCNTR_CTL_PRIO0 | \
				RXS_SPX_PCNTR_CTL_PRIO0C | \
				RXS_SPX_PCNTR_CTL_PRIO1  | \
				RXS_SPX_PCNTR_CTL_PRIO1C | \
				RXS_SPX_PCNTR_CTL_PRIO2 | \
				RXS_SPX_PCNTR_CTL_PRIO2C | \
				RXS_SPX_PCNTR_CTL_PRIO3 | \
				RXS_SPX_PCNTR_CTL_PRIO3C))
#define RXS_SPX_PCNTR_CTL_SEL                        ((uint32_t)0x0000007f)
#define RXS_SPX_PCNTR_CTL_SEL_RIO_PKT                ((uint32_t)0x00000000)
#define RXS_SPX_PCNTR_CTL_SEL_FAB_PKT                ((uint32_t)0x00000001)
#define RXS_SPX_PCNTR_CTL_SEL_RIO_PAYLOAD            ((uint32_t)0x00000002)
#define RXS_SPX_PCNTR_CTL_SEL_FAB_PAYLOAD            ((uint32_t)0x00000003)
#define RXS_SPX_PCNTR_CTL_SEL_RIO_TTL_PKTCNTR        ((uint32_t)0x00000007)
#define RXS_SPX_PCNTR_CTL_SEL_RETRIES                ((uint32_t)0x00000008)
#define RXS_SPX_PCNTR_CTL_SEL_PNA                    ((uint32_t)0x00000009)
#define RXS_SPX_PCNTR_CTL_SEL_PKT_DROP               ((uint32_t)0x0000000A)
#define RXS_SPX_PCNTR_CTL_SEL_DISABLED               ((uint32_t)0x0000007F)

/* RXS_SPX_PCNTR_CNTY : Register Bits Masks Definitions */
#define RXS_SPX_PCNTR_CNTY_COUNT                     ((uint32_t)0xffffffff)

/* RXS_PCAP_BH : Register Bits Masks Definitions */
#define RXS_PCAP_BH_BLK_TYPE                         ((uint32_t)0x00000fff)
#define RXS_PCAP_BH_BLK_REV                          ((uint32_t)0x0000f000)
#define RXS_PCAP_BH_NEXT_BLK_PTR                     ((uint32_t)0xffff0000)

/* RXS_SPX_PCAP_ACC : Register Bits Masks Definitions */
#define RXS_SPX_PCAP_ACC_DCM_ON                      ((uint32_t)0x80000000)

/* RXS_SPX_PCAP_CTL : Register Bits Masks Definitions */
#define RXS_SPX_PCAP_CTL_PORT                        ((uint32_t)0x00000003)
#define RXS_SPX_PCAP_CTL_FRM_DME                     ((uint32_t)0x00000004)
#define RXS_SPX_PCAP_CTL_RAW                         ((uint32_t)0x00000008)
#define RXS_SPX_PCAP_CTL_ADDR                        ((uint32_t)0x00000ff0)
#define RXS_SPX_PCAP_CTL_ADD_WR_COUNT                ((uint32_t)0x00fff000)
#define RXS_SPX_PCAP_CTL_STOP_4_EL_INTB              ((uint32_t)0x01000000)
#define RXS_SPX_PCAP_CTL_STOP_4_EL_INTA              ((uint32_t)0x02000000)
#define RXS_SPX_PCAP_CTL_MODE                        ((uint32_t)0x0c000000)
#define RXS_SPX_PCAP_CTL_STOP                        ((uint32_t)0x10000000)
#define RXS_SPX_PCAP_CTL_DONE                        ((uint32_t)0x20000000)
#define RXS_SPX_PCAP_CTL_START                       ((uint32_t)0x40000000)
#define RXS_SPX_PCAP_CTL_PC_INIT                     ((uint32_t)0x80000000)

/* RXS_SPX_PCAP_STAT : Register Bits Masks Definitions */
#define RXS_SPX_PCAP_STAT_TRIG_ADDR                  ((uint32_t)0x00000ff0)
#define RXS_SPX_PCAP_STAT_FULL                       ((uint32_t)0x01000000)
#define RXS_SPX_PCAP_STAT_TRIG                       ((uint32_t)0x02000000)
#define RXS_SPX_PCAP_STAT_WRAP                       ((uint32_t)0x04000000)
#define RXS_SPX_PCAP_STAT_EVNT                       ((uint32_t)0x80000000)

/* RXS_SPX_PCAP_GEN : Register Bits Masks Definitions */
#define RXS_SPX_PCAP_GEN_GEN                         ((uint32_t)0x80000000)

/* RXS_SPX_PCAP_SB_DATA : Register Bits Masks Definitions */
#define RXS_SPX_PCAP_SB_DATA_DME_M3                  ((uint32_t)0x00000010)
#define RXS_SPX_PCAP_SB_DATA_DME_M2                  ((uint32_t)0x00000020)
#define RXS_SPX_PCAP_SB_DATA_DME_M1                  ((uint32_t)0x00000040)
#define RXS_SPX_PCAP_SB_DATA_DME_M0                  ((uint32_t)0x00000080)
#define RXS_SPX_PCAP_SB_DATA_CG_V3                   ((uint32_t)0x00001000)
#define RXS_SPX_PCAP_SB_DATA_CG_V2                   ((uint32_t)0x00002000)
#define RXS_SPX_PCAP_SB_DATA_CG_V1                   ((uint32_t)0x00004000)
#define RXS_SPX_PCAP_SB_DATA_CG_V0                   ((uint32_t)0x00008000)
#define RXS_SPX_PCAP_SB_DATA_CW_E1                   ((uint32_t)0x00040000)
#define RXS_SPX_PCAP_SB_DATA_CW_T1                   ((uint32_t)0x00080000)
#define RXS_SPX_PCAP_SB_DATA_CW_V1                   ((uint32_t)0x00100000)
#define RXS_SPX_PCAP_SB_DATA_CW_E0                   ((uint32_t)0x00200000)
#define RXS_SPX_PCAP_SB_DATA_CW_T0                   ((uint32_t)0x00400000)
#define RXS_SPX_PCAP_SB_DATA_CW_V0                   ((uint32_t)0x00800000)
#define RXS_SPX_PCAP_SB_DATA_LEN                     ((uint32_t)0x1c000000)
#define RXS_SPX_PCAP_SB_DATA_TRUNCATED               ((uint32_t)0x20000000)
#define RXS_SPX_PCAP_SB_DATA_DISCARD                 ((uint32_t)0x40000000)
#define RXS_SPX_PCAP_SB_DATA_EOP                     ((uint32_t)0x80000000)

/* RXS_SPX_PCAP_MEM_DEPTH : Register Bits Masks Definitions */
#define RXS_SPX_PCAP_MEM_DEPTH_MEM_DEPTH             ((uint32_t)0xffffffff)

/* RXS_SPX_PCAP_DATAY : Register Bits Masks Definitions */
#define RXS_SPX_PCAP_DATAY_DATA                      ((uint32_t)0xffffffff)

/* RXS_DBG_EL_BH : Register Bits Masks Definitions */
#define RXS_DBG_EL_BH_BLK_TYPE                       ((uint32_t)0x00000fff)
#define RXS_DBG_EL_BH_BLK_REV                        ((uint32_t)0x0000f000)
#define RXS_DBG_EL_BH_NEXT_BLK_PTR                   ((uint32_t)0xffff0000)

/* RXS_SPX_DBG_EL_ACC : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_ACC_EL_ON                     ((uint32_t)0x80000000)

/* RXS_SPX_DBG_EL_INFO : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_INFO_MAX_STAT_INDEX           ((uint32_t)0x0000000f)
#define RXS_SPX_DBG_EL_INFO_MEM_SIZE                 ((uint32_t)0x00000700)
#define RXS_SPX_DBG_EL_INFO_VERSION                  ((uint32_t)0xff000000)

/* RXS_SPX_DBG_EL_CTL : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_CTL_T0_EN                     ((uint32_t)0x00000001)
#define RXS_SPX_DBG_EL_CTL_T0_INTA                   ((uint32_t)0x00000002)
#define RXS_SPX_DBG_EL_CTL_T0_INTB                   ((uint32_t)0x00000004)
#define RXS_SPX_DBG_EL_CTL_T1_EN                     ((uint32_t)0x00000010)
#define RXS_SPX_DBG_EL_CTL_T1_INTA                   ((uint32_t)0x00000020)
#define RXS_SPX_DBG_EL_CTL_T1_INTB                   ((uint32_t)0x00000040)
#define RXS_SPX_DBG_EL_CTL_TR_COND                   ((uint32_t)0x00000700)
#define RXS_SPX_DBG_EL_CTL_TR_INTA                   ((uint32_t)0x00001000)
#define RXS_SPX_DBG_EL_CTL_TR_INTB                   ((uint32_t)0x00002000)
#define RXS_SPX_DBG_EL_CTL_RST                       ((uint32_t)0x00008000)
#define RXS_SPX_DBG_EL_CTL_TRIG_POS                  ((uint32_t)0x00ff0000)

/* RXS_SPX_DBG_EL_INT_EN : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_INT_EN_EN_A                   ((uint32_t)0x00000001)
#define RXS_SPX_DBG_EL_INT_EN_EN_B                   ((uint32_t)0x00000002)

/* RXS_SPX_DBG_EL_SRC_LOG_EN : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_0               ((uint32_t)0x00000001)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_1               ((uint32_t)0x00000002)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_2               ((uint32_t)0x00000004)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_3               ((uint32_t)0x00000008)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_4               ((uint32_t)0x00000010)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_5               ((uint32_t)0x00000020)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_6               ((uint32_t)0x00000040)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_7               ((uint32_t)0x00000080)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_8               ((uint32_t)0x00000100)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_9               ((uint32_t)0x00000200)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_10              ((uint32_t)0x00000400)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_11              ((uint32_t)0x00000800)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_12              ((uint32_t)0x00001000)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_13              ((uint32_t)0x00002000)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_14              ((uint32_t)0x00004000)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_EN_15              ((uint32_t)0x00008000)
#define RXS_SPX_DBG_EL_SRC_LOG_EN_CLR                ((uint32_t)0x80000000)

/* RXS_SPX_DBG_EL_TRIG0_MASK : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_TRIG0_MASK_MASK               ((uint32_t)0x07ffffff)
#define RXS_SPX_DBG_EL_TRIG0_MASK_SAME_TIME_MASK     ((uint32_t)0x08000000)
#define RXS_SPX_DBG_EL_TRIG0_MASK_SRC_MASK           ((uint32_t)0xf0000000)

/* RXS_SPX_DBG_EL_TRIG0_VAL : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_TRIG0_VAL_EVENTS              ((uint32_t)0x07ffffff)
#define RXS_SPX_DBG_EL_TRIG0_VAL_SAME_TIME           ((uint32_t)0x08000000)
#define RXS_SPX_DBG_EL_TRIG0_VAL_SRC_VAL             ((uint32_t)0xf0000000)

/* RXS_SPX_DBG_EL_TRIG1_MASK : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_TRIG1_MASK_MASK               ((uint32_t)0x07ffffff)
#define RXS_SPX_DBG_EL_TRIG1_MASK_SAME_TIME_MASK     ((uint32_t)0x08000000)
#define RXS_SPX_DBG_EL_TRIG1_MASK_SRC_MASK           ((uint32_t)0xf0000000)

/* RXS_SPX_DBG_EL_TRIG1_VAL : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_TRIG1_VAL_EVENTS              ((uint32_t)0x07ffffff)
#define RXS_SPX_DBG_EL_TRIG1_VAL_SAME_TIME           ((uint32_t)0x08000000)
#define RXS_SPX_DBG_EL_TRIG1_VAL_SRC_VAL             ((uint32_t)0xf0000000)

/* RXS_SPX_DBG_EL_TRIG_STAT : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_TRIG_STAT_TRIG_0              ((uint32_t)0x00000001)
#define RXS_SPX_DBG_EL_TRIG_STAT_TRIG_1              ((uint32_t)0x00000002)
#define RXS_SPX_DBG_EL_TRIG_STAT_TRIG                ((uint32_t)0x00000004)
#define RXS_SPX_DBG_EL_TRIG_STAT_FULL                ((uint32_t)0x00000008)
#define RXS_SPX_DBG_EL_TRIG_STAT_INT_A               ((uint32_t)0x00000010)
#define RXS_SPX_DBG_EL_TRIG_STAT_INT_B               ((uint32_t)0x00000020)
#define RXS_SPX_DBG_EL_TRIG_STAT_ALL_VALID           ((uint32_t)0x00000080)

/* RXS_SPX_DBG_EL_WR_TRIG_IDX : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_WR_TRIG_IDX_WR_IDX            ((uint32_t)0x000000ff)
#define RXS_SPX_DBG_EL_WR_TRIG_IDX_TRIG_IDX          ((uint32_t)0x00ff0000)

/* RXS_SPX_DBG_EL_RD_IDX : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_RD_IDX_RD_IDX                 ((uint32_t)0x000000ff)

/* RXS_SPX_DBG_EL_DATA : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_DATA_EVENTS                   ((uint32_t)0x07ffffff)
#define RXS_SPX_DBG_EL_DATA_SAME_TIME                ((uint32_t)0x08000000)
#define RXS_SPX_DBG_EL_DATA_SOURCE                   ((uint32_t)0xf0000000)

/* RXS_SPX_DBG_EL_SRC_STATY : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_SRC_STATY_CW_LOCK             ((uint32_t)0x00000001)
#define RXS_SPX_DBG_EL_SRC_STATY_CW_LOCK_LOST        ((uint32_t)0x00000002)
#define RXS_SPX_DBG_EL_SRC_STATY_CW_BAD_TYPE         ((uint32_t)0x00000004)
#define RXS_SPX_DBG_EL_SRC_STATY_IG_CDC_OV           ((uint32_t)0x00000008)
#define RXS_SPX_DBG_EL_SRC_STATY_SEED_ERR            ((uint32_t)0x00000010)
#define RXS_SPX_DBG_EL_SRC_STATY_SKIPM_ERR           ((uint32_t)0x00000020)
#define RXS_SPX_DBG_EL_SRC_STATY_SKIPC_ERR           ((uint32_t)0x00000040)
#define RXS_SPX_DBG_EL_SRC_STATY_BAD_DME_FM          ((uint32_t)0x00000080)
#define RXS_SPX_DBG_EL_SRC_STATY_EG_CDC_OV           ((uint32_t)0x00000100)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ3                ((uint32_t)0x00000200)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ2                ((uint32_t)0x00000400)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ1                ((uint32_t)0x00000800)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ0                ((uint32_t)0x00001000)
#define RXS_SPX_DBG_EL_SRC_STATY_RE_FAIL             ((uint32_t)0x00002000)
#define RXS_SPX_DBG_EL_SRC_STATY_RE_TRN2             ((uint32_t)0x00004000)
#define RXS_SPX_DBG_EL_SRC_STATY_RE_TRN1             ((uint32_t)0x00008000)
#define RXS_SPX_DBG_EL_SRC_STATY_RE_TRN0             ((uint32_t)0x00010000)
#define RXS_SPX_DBG_EL_SRC_STATY_KEEP_ALIVE          ((uint32_t)0x00020000)
#define RXS_SPX_DBG_EL_SRC_STATY_TRAINED             ((uint32_t)0x00040000)
#define RXS_SPX_DBG_EL_SRC_STATY_CW_FAIL             ((uint32_t)0x00080000)
#define RXS_SPX_DBG_EL_SRC_STATY_CW_TRN1             ((uint32_t)0x00100000)
#define RXS_SPX_DBG_EL_SRC_STATY_CW_TRN0             ((uint32_t)0x00200000)
#define RXS_SPX_DBG_EL_SRC_STATY_DME_FAIL            ((uint32_t)0x00400000)
#define RXS_SPX_DBG_EL_SRC_STATY_DME_TRN2            ((uint32_t)0x00800000)
#define RXS_SPX_DBG_EL_SRC_STATY_DME_TRN1            ((uint32_t)0x01000000)
#define RXS_SPX_DBG_EL_SRC_STATY_DME_TRN0            ((uint32_t)0x02000000)
#define RXS_SPX_DBG_EL_SRC_STATY_UNTRAINED           ((uint32_t)0x04000000)

/* RXS_SPX_DBG_EL_SRC_STAT4 : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_SRC_STAT4_IDLE                ((uint32_t)0x00000001)
#define RXS_SPX_DBG_EL_SRC_STAT4_XMT_WIDTH           ((uint32_t)0x00000002)
#define RXS_SPX_DBG_EL_SRC_STAT4_RE_TRN0             ((uint32_t)0x00000004)
#define RXS_SPX_DBG_EL_SRC_STAT4_RE_TRN1             ((uint32_t)0x00000008)
#define RXS_SPX_DBG_EL_SRC_STAT4_RE_TRN2             ((uint32_t)0x00000010)
#define RXS_SPX_DBG_EL_SRC_STAT4_RE_TRN3             ((uint32_t)0x00000020)
#define RXS_SPX_DBG_EL_SRC_STAT4_RE_TRN4             ((uint32_t)0x00000040)
#define RXS_SPX_DBG_EL_SRC_STAT4_RE_TRN5             ((uint32_t)0x00000080)
#define RXS_SPX_DBG_EL_SRC_STAT4_RE_TRN_TO           ((uint32_t)0x00000100)
#define RXS_SPX_DBG_EL_SRC_STAT4_SOFTWARE            ((uint32_t)0x00000600)
#define RXS_SPX_DBG_EL_SRC_STAT4_LOST_CS             ((uint32_t)0x00000800)
#define RXS_SPX_DBG_EL_SRC_STAT4_RETRAIN_1X          ((uint32_t)0x00001000)
#define RXS_SPX_DBG_EL_SRC_STAT4_RETRAIN_2X          ((uint32_t)0x00002000)
#define RXS_SPX_DBG_EL_SRC_STAT4_RETRAIN_4X          ((uint32_t)0x00004000)
#define RXS_SPX_DBG_EL_SRC_STAT4_MODE_2X             ((uint32_t)0x00008000)
#define RXS_SPX_DBG_EL_SRC_STAT4_RECOV_2X            ((uint32_t)0x00010000)
#define RXS_SPX_DBG_EL_SRC_STAT4_MODE_4X             ((uint32_t)0x00020000)
#define RXS_SPX_DBG_EL_SRC_STAT4_RECOV_4X            ((uint32_t)0x00040000)
#define RXS_SPX_DBG_EL_SRC_STAT4_LANE2_1XR           ((uint32_t)0x00080000)
#define RXS_SPX_DBG_EL_SRC_STAT4_LANE1_1XR           ((uint32_t)0x00100000)
#define RXS_SPX_DBG_EL_SRC_STAT4_LANE0_1XR           ((uint32_t)0x00200000)
#define RXS_SPX_DBG_EL_SRC_STAT4_RECOV_1X            ((uint32_t)0x00400000)
#define RXS_SPX_DBG_EL_SRC_STAT4_DISCOVERY           ((uint32_t)0x00800000)
#define RXS_SPX_DBG_EL_SRC_STAT4_SEEK                ((uint32_t)0x01000000)
#define RXS_SPX_DBG_EL_SRC_STAT4_ASYM_MODE           ((uint32_t)0x02000000)
#define RXS_SPX_DBG_EL_SRC_STAT4_SILENT              ((uint32_t)0x04000000)

/* RXS_SPX_DBG_EL_SRC_STAT5 : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_SRC_STAT5_ASYM_TX_EXIT        ((uint32_t)0x00000001)
#define RXS_SPX_DBG_EL_SRC_STAT5_ASYM_TX_IDLE        ((uint32_t)0x00000002)
#define RXS_SPX_DBG_EL_SRC_STAT5_TX_NACK             ((uint32_t)0x00000004)
#define RXS_SPX_DBG_EL_SRC_STAT5_SK_TX_1X            ((uint32_t)0x00000008)
#define RXS_SPX_DBG_EL_SRC_STAT5_SK_TX1_1X           ((uint32_t)0x00000010)
#define RXS_SPX_DBG_EL_SRC_STAT5_SK_TX2_1X           ((uint32_t)0x00000020)
#define RXS_SPX_DBG_EL_SRC_STAT5_SK_TX3_1X           ((uint32_t)0x00000040)
#define RXS_SPX_DBG_EL_SRC_STAT5_TX_ACK_1X           ((uint32_t)0x00000080)
#define RXS_SPX_DBG_EL_SRC_STAT5_TX_1X               ((uint32_t)0x00000100)
#define RXS_SPX_DBG_EL_SRC_STAT5_SK_TX_2X            ((uint32_t)0x00000200)
#define RXS_SPX_DBG_EL_SRC_STAT5_SK_TX1_2X           ((uint32_t)0x00000400)
#define RXS_SPX_DBG_EL_SRC_STAT5_SK_TX2_2X           ((uint32_t)0x00000800)
#define RXS_SPX_DBG_EL_SRC_STAT5_SK_TX3_2X           ((uint32_t)0x00001000)
#define RXS_SPX_DBG_EL_SRC_STAT5_TX_ACK_2X           ((uint32_t)0x00002000)
#define RXS_SPX_DBG_EL_SRC_STAT5_TX_2X               ((uint32_t)0x00004000)
#define RXS_SPX_DBG_EL_SRC_STAT5_SK_TX_4X            ((uint32_t)0x00008000)
#define RXS_SPX_DBG_EL_SRC_STAT5_SK_TX1_4X           ((uint32_t)0x00010000)
#define RXS_SPX_DBG_EL_SRC_STAT5_SK_TX2_4X           ((uint32_t)0x00020000)
#define RXS_SPX_DBG_EL_SRC_STAT5_SK_TX3_4X           ((uint32_t)0x00040000)
#define RXS_SPX_DBG_EL_SRC_STAT5_TX_ACK_4X           ((uint32_t)0x00080000)
#define RXS_SPX_DBG_EL_SRC_STAT5_TX_4X               ((uint32_t)0x00100000)
#define RXS_SPX_DBG_EL_SRC_STAT5_SOFTWARE            ((uint32_t)0x00600000)
#define RXS_SPX_DBG_EL_SRC_STAT5_TXW_IDLE            ((uint32_t)0x00800000)
#define RXS_SPX_DBG_EL_SRC_STAT5_TXW_CMD3            ((uint32_t)0x01000000)
#define RXS_SPX_DBG_EL_SRC_STAT5_TXW_CMD2            ((uint32_t)0x02000000)
#define RXS_SPX_DBG_EL_SRC_STAT5_TXW_CMD1            ((uint32_t)0x04000000)

/* RXS_SPX_DBG_EL_SRC_STAT6 : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_SRC_STAT6_ASYM_RX_EXIT        ((uint32_t)0x00000001)
#define RXS_SPX_DBG_EL_SRC_STAT6_ASYM_RX_IDLE        ((uint32_t)0x00000002)
#define RXS_SPX_DBG_EL_SRC_STAT6_RX_NACK             ((uint32_t)0x00000004)
#define RXS_SPX_DBG_EL_SRC_STAT6_SK_RX_1X            ((uint32_t)0x00000008)
#define RXS_SPX_DBG_EL_SRC_STAT6_ACK_RX_1X           ((uint32_t)0x00000010)
#define RXS_SPX_DBG_EL_SRC_STAT6_RECOV_RX_1X         ((uint32_t)0x00000020)
#define RXS_SPX_DBG_EL_SRC_STAT6_RX_1X               ((uint32_t)0x00000040)
#define RXS_SPX_DBG_EL_SRC_STAT6_SK_RX_2X            ((uint32_t)0x00000080)
#define RXS_SPX_DBG_EL_SRC_STAT6_ACK_RX_2X           ((uint32_t)0x00000100)
#define RXS_SPX_DBG_EL_SRC_STAT6_RECOV_RX_2X         ((uint32_t)0x00000200)
#define RXS_SPX_DBG_EL_SRC_STAT6_RX_2X               ((uint32_t)0x00000400)
#define RXS_SPX_DBG_EL_SRC_STAT6_SK_RX_4X            ((uint32_t)0x00000800)
#define RXS_SPX_DBG_EL_SRC_STAT6_ACK_RX_4X           ((uint32_t)0x00001000)
#define RXS_SPX_DBG_EL_SRC_STAT6_RECOV_RX_4X         ((uint32_t)0x00002000)
#define RXS_SPX_DBG_EL_SRC_STAT6_RX_4X               ((uint32_t)0x00004000)
#define RXS_SPX_DBG_EL_SRC_STAT6_SOFTWARE            ((uint32_t)0x007f8000)
#define RXS_SPX_DBG_EL_SRC_STAT6_RXW_IDLE            ((uint32_t)0x00800000)
#define RXS_SPX_DBG_EL_SRC_STAT6_RXW_CMD3            ((uint32_t)0x01000000)
#define RXS_SPX_DBG_EL_SRC_STAT6_RXW_CMD2            ((uint32_t)0x02000000)
#define RXS_SPX_DBG_EL_SRC_STAT6_RXW_CMD1            ((uint32_t)0x04000000)

/* RXS_SPX_DBG_EL_SRC_STAT7 : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_SRC_STAT7_PORT_UNINIT         ((uint32_t)0x00000001)
#define RXS_SPX_DBG_EL_SRC_STAT7_PORT_OK             ((uint32_t)0x00000002)
#define RXS_SPX_DBG_EL_SRC_STAT7_PORT_ERR_SET        ((uint32_t)0x00000004)
#define RXS_SPX_DBG_EL_SRC_STAT7_PORT_ERR_CLR        ((uint32_t)0x00000008)
#define RXS_SPX_DBG_EL_SRC_STAT7_PW_PEND_SET         ((uint32_t)0x00000010)
#define RXS_SPX_DBG_EL_SRC_STAT7_PW_PEND_CLR         ((uint32_t)0x00000020)
#define RXS_SPX_DBG_EL_SRC_STAT7_INP_ERR_SET         ((uint32_t)0x00000040)
#define RXS_SPX_DBG_EL_SRC_STAT7_INP_ERR_CLR         ((uint32_t)0x00000080)
#define RXS_SPX_DBG_EL_SRC_STAT7_OUT_ERR_SET         ((uint32_t)0x00000100)
#define RXS_SPX_DBG_EL_SRC_STAT7_OUT_ERR_CLR         ((uint32_t)0x00000200)
#define RXS_SPX_DBG_EL_SRC_STAT7_CS_CRC_ERR          ((uint32_t)0x00000400)
#define RXS_SPX_DBG_EL_SRC_STAT7_PKT_CRC_ERR         ((uint32_t)0x00000800)
#define RXS_SPX_DBG_EL_SRC_STAT7_PKT_ILL_ACKID       ((uint32_t)0x00001000)
#define RXS_SPX_DBG_EL_SRC_STAT7_CS_ILL_ACKID        ((uint32_t)0x00002000)
#define RXS_SPX_DBG_EL_SRC_STAT7_PROT_ERR            ((uint32_t)0x00004000)
#define RXS_SPX_DBG_EL_SRC_STAT7_DESCRAM_LOS         ((uint32_t)0x00008000)
#define RXS_SPX_DBG_EL_SRC_STAT7_DESCRAM_RESYNC      ((uint32_t)0x00010000)
#define RXS_SPX_DBG_EL_SRC_STAT7_LINK_TO             ((uint32_t)0x00020000)
#define RXS_SPX_DBG_EL_SRC_STAT7_STYP0_RSVD          ((uint32_t)0x00040000)
#define RXS_SPX_DBG_EL_SRC_STAT7_STYP1_RSVD          ((uint32_t)0x00080000)
#define RXS_SPX_DBG_EL_SRC_STAT7_DELIN_ERR           ((uint32_t)0x00100000)
#define RXS_SPX_DBG_EL_SRC_STAT7_PRBS_ERROR          ((uint32_t)0x00200000)
#define RXS_SPX_DBG_EL_SRC_STAT7_PKT_ILL_SIZE        ((uint32_t)0x00400000)
#define RXS_SPX_DBG_EL_SRC_STAT7_LR_ACKID_ILL        ((uint32_t)0x00800000)
#define RXS_SPX_DBG_EL_SRC_STAT7_CS_ACK_ILL          ((uint32_t)0x01000000)
#define RXS_SPX_DBG_EL_SRC_STAT7_OUT_FAIL_SET        ((uint32_t)0x02000000)
#define RXS_SPX_DBG_EL_SRC_STAT7_OUT_FAIL_CLR        ((uint32_t)0x04000000)

/* RXS_SPX_DBG_EL_SRC_STAT8 : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_SRC_STAT8_RX_SOP              ((uint32_t)0x00000001)
#define RXS_SPX_DBG_EL_SRC_STAT8_RX_EOP              ((uint32_t)0x00000002)
#define RXS_SPX_DBG_EL_SRC_STAT8_RX_STOMP            ((uint32_t)0x00000004)
#define RXS_SPX_DBG_EL_SRC_STAT8_RX_RFR              ((uint32_t)0x00000008)
#define RXS_SPX_DBG_EL_SRC_STAT8_RX_LREQ             ((uint32_t)0x00000010)
#define RXS_SPX_DBG_EL_SRC_STAT8_RX_LRESP            ((uint32_t)0x00000020)
#define RXS_SPX_DBG_EL_SRC_STAT8_RX_PACK             ((uint32_t)0x00000040)
#define RXS_SPX_DBG_EL_SRC_STAT8_RX_RETRY            ((uint32_t)0x00000080)
#define RXS_SPX_DBG_EL_SRC_STAT8_RX_PNA              ((uint32_t)0x00000100)
#define RXS_SPX_DBG_EL_SRC_STAT8_SOFTWARE3           ((uint32_t)0x00000600)
#define RXS_SPX_DBG_EL_SRC_STAT8_ALIGN6              ((uint32_t)0x00000800)
#define RXS_SPX_DBG_EL_SRC_STAT8_ALIGN5_7            ((uint32_t)0x00001000)
#define RXS_SPX_DBG_EL_SRC_STAT8_ALIGN4              ((uint32_t)0x00002000)
#define RXS_SPX_DBG_EL_SRC_STAT8_ALIGN3              ((uint32_t)0x00004000)
#define RXS_SPX_DBG_EL_SRC_STAT8_ALIGN2              ((uint32_t)0x00008000)
#define RXS_SPX_DBG_EL_SRC_STAT8_SOFTWARE5           ((uint32_t)0x00010000)
#define RXS_SPX_DBG_EL_SRC_STAT8_ALIGNED             ((uint32_t)0x00020000)
#define RXS_SPX_DBG_EL_SRC_STAT8_SOFTWARE2           ((uint32_t)0x00040000)
#define RXS_SPX_DBG_EL_SRC_STAT8_NALIGN2_3           ((uint32_t)0x00080000)
#define RXS_SPX_DBG_EL_SRC_STAT8_NALIGN1             ((uint32_t)0x00100000)
#define RXS_SPX_DBG_EL_SRC_STAT8_NALIGN              ((uint32_t)0x00200000)
#define RXS_SPX_DBG_EL_SRC_STAT8_SOFTWARE            ((uint32_t)0x00400000)
#define RXS_SPX_DBG_EL_SRC_STAT8_LP_RX_TRAIND        ((uint32_t)0x00800000)
#define RXS_SPX_DBG_EL_SRC_STAT8_RX_TRAIND           ((uint32_t)0x01000000)
#define RXS_SPX_DBG_EL_SRC_STAT8_SOFTWARE4           ((uint32_t)0x06000000)

/* RXS_SPX_DBG_EL_SRC_STAT9 : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_SRC_STAT9_SOFTWARE            ((uint32_t)0x07ffffff)

/* RXS_SPX_DBG_EL_SRC_STAT10 : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_SRC_STAT10_TX_SOP             ((uint32_t)0x00000001)
#define RXS_SPX_DBG_EL_SRC_STAT10_TX_EOP             ((uint32_t)0x00000002)
#define RXS_SPX_DBG_EL_SRC_STAT10_TX_STOMP           ((uint32_t)0x00000004)
#define RXS_SPX_DBG_EL_SRC_STAT10_TX_RFR             ((uint32_t)0x00000008)
#define RXS_SPX_DBG_EL_SRC_STAT10_TX_LREQ            ((uint32_t)0x00000010)
#define RXS_SPX_DBG_EL_SRC_STAT10_TX_LRESP           ((uint32_t)0x00000020)
#define RXS_SPX_DBG_EL_SRC_STAT10_TX_PACK            ((uint32_t)0x00000040)
#define RXS_SPX_DBG_EL_SRC_STAT10_TX_RETRY           ((uint32_t)0x00000080)
#define RXS_SPX_DBG_EL_SRC_STAT10_TX_PNA             ((uint32_t)0x00000100)
#define RXS_SPX_DBG_EL_SRC_STAT10_SOFTWARE           ((uint32_t)0x07fffe00)

/* RXS_SPX_DBG_EL_SRC_STAT11 : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_SRC_STAT11_SOFTWARE5          ((uint32_t)0x00000001)
#define RXS_SPX_DBG_EL_SRC_STAT11_PNA_IN_OES         ((uint32_t)0x00000002)
#define RXS_SPX_DBG_EL_SRC_STAT11_TWO_RETRY          ((uint32_t)0x00000004)
#define RXS_SPX_DBG_EL_SRC_STAT11_UNEXP_PR_PA        ((uint32_t)0x00000008)
#define RXS_SPX_DBG_EL_SRC_STAT11_SOP_EOP            ((uint32_t)0x00000010)
#define RXS_SPX_DBG_EL_SRC_STAT11_TWO_SOP            ((uint32_t)0x00000020)
#define RXS_SPX_DBG_EL_SRC_STAT11_XTRA_LR            ((uint32_t)0x00000040)
#define RXS_SPX_DBG_EL_SRC_STAT11_UNEXP_EOP          ((uint32_t)0x00000080)
#define RXS_SPX_DBG_EL_SRC_STAT11_UNEXP_RFR          ((uint32_t)0x00000100)
#define RXS_SPX_DBG_EL_SRC_STAT11_UNEXP_LR           ((uint32_t)0x00000200)
#define RXS_SPX_DBG_EL_SRC_STAT11_BAD_CSEB_END       ((uint32_t)0x00000400)
#define RXS_SPX_DBG_EL_SRC_STAT11_BAD_CS_CW_SEQ      ((uint32_t)0x00000800)
#define RXS_SPX_DBG_EL_SRC_STAT11_BAD_SC_OS          ((uint32_t)0x00001000)
#define RXS_SPX_DBG_EL_SRC_STAT11_SOFTWARE4          ((uint32_t)0x00002000)
#define RXS_SPX_DBG_EL_SRC_STAT11_CTL_CW_RSVD        ((uint32_t)0x00004000)
#define RXS_SPX_DBG_EL_SRC_STAT11_BAD_OS             ((uint32_t)0x00008000)
#define RXS_SPX_DBG_EL_SRC_STAT11_IDLE3_BAD_COL      ((uint32_t)0x00010000)
#define RXS_SPX_DBG_EL_SRC_STAT11_SOFTWARE2          ((uint32_t)0x00020000)
#define RXS_SPX_DBG_EL_SRC_STAT11_IDLE3_DATA_NON_ZERO ((uint32_t)0x00040000)
#define RXS_SPX_DBG_EL_SRC_STAT11_LR_NO_SEED         ((uint32_t)0x00080000)
#define RXS_SPX_DBG_EL_SRC_STAT11_NON_ZERO           ((uint32_t)0x00100000)
#define RXS_SPX_DBG_EL_SRC_STAT11_OS_IN_PKT          ((uint32_t)0x00200000)
#define RXS_SPX_DBG_EL_SRC_STAT11_SOFTWARE3          ((uint32_t)0x00400000)
#define RXS_SPX_DBG_EL_SRC_STAT11_BAD_PAD            ((uint32_t)0x00800000)
#define RXS_SPX_DBG_EL_SRC_STAT11_SOFTWARE           ((uint32_t)0x07000000)

/* RXS_SPX_DBG_EL_SRC_STATY : Register Bits Masks Definitions */
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ17               ((uint32_t)0x00000001)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ16               ((uint32_t)0x00000002)
#define RXS_SPX_DBG_EL_SRC_STATY_BAD_LANE_CHECK      ((uint32_t)0x00000004)
#define RXS_SPX_DBG_EL_SRC_STATY_BIP_IBIP_MISMATCH   ((uint32_t)0x00000008)
#define RXS_SPX_DBG_EL_SRC_STATY_BAD_BIP             ((uint32_t)0x00000010)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ15               ((uint32_t)0x00000020)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ14               ((uint32_t)0x00000040)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ13               ((uint32_t)0x00000080)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ12               ((uint32_t)0x00000100)
#define RXS_SPX_DBG_EL_SRC_STATY_FL_TST_MK           ((uint32_t)0x00000200)
#define RXS_SPX_DBG_EL_SRC_STATY_FL_NEW_MK           ((uint32_t)0x00000400)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ11               ((uint32_t)0x00000800)
#define RXS_SPX_DBG_EL_SRC_STATY_FL_OOFM             ((uint32_t)0x00001000)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ9                ((uint32_t)0x00002000)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ8                ((uint32_t)0x00004000)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ7                ((uint32_t)0x00008000)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ6                ((uint32_t)0x00010000)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ5                ((uint32_t)0x00020000)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ4                ((uint32_t)0x00040000)
#define RXS_SPX_DBG_EL_SRC_STATY_NO_SYNC_1           ((uint32_t)0x00080000)
#define RXS_SPX_DBG_EL_SRC_STATY_NO_SYNC2_3_4        ((uint32_t)0x00100000)
#define RXS_SPX_DBG_EL_SRC_STATY_SYNC                ((uint32_t)0x00200000)
#define RXS_SPX_DBG_EL_SRC_STATY_SYNC1_2             ((uint32_t)0x00400000)
#define RXS_SPX_DBG_EL_SRC_STATY_NO_LOCK             ((uint32_t)0x00800000)
#define RXS_SPX_DBG_EL_SRC_STATY_AEQ10               ((uint32_t)0x01000000)
#define RXS_SPX_DBG_EL_SRC_STATY_NO_LOCK1_2_3_4      ((uint32_t)0x02000000)
#define RXS_SPX_DBG_EL_SRC_STATY_LOCK_1_2_3          ((uint32_t)0x04000000)

/* RXS_LANE_TEST_BH : Register Bits Masks Definitions */
#define RXS_LANE_TEST_BH_BLK_TYPE                    ((uint32_t)0x00000fff)
#define RXS_LANE_TEST_BH_BLK_REV                     ((uint32_t)0x0000f000)
#define RXS_LANE_TEST_BH_NEXT_BLK_PTR                ((uint32_t)0xffff0000)

/* RXS_LANEX_PRBS_CTRL : Register Bits Masks Definitions */
#define RXS_LANEX_PRBS_CTRL_TRANSMIT                 ((uint32_t)0x01000000)
#define RXS_LANEX_PRBS_CTRL_ENABLE                   ((uint32_t)0x02000000)
#define RXS_LANEX_PRBS_CTRL_TRAIN                    ((uint32_t)0x04000000)
#define RXS_LANEX_PRBS_CTRL_INVERT                   ((uint32_t)0x08000000)
#define RXS_LANEX_PRBS_CTRL_PATTERN                  ((uint32_t)0xf0000000)

/* RXS_LANEX_PRBS_STATUS : Register Bits Masks Definitions */
#define RXS_LANEX_PRBS_STATUS_FIXED_UNLOCK           ((uint32_t)0x00000001)
#define RXS_LANEX_PRBS_STATUS_PRBS_LOS               ((uint32_t)0x00000002)

/* RXS_LANEX_PRBS_ERR_CNT : Register Bits Masks Definitions */
#define RXS_LANEX_PRBS_ERR_CNT_COUNT                 ((uint32_t)0xffffffff)

/* RXS_LANEX_PRBS_SEED_0U : Register Bits Masks Definitions */
#define RXS_LANEX_PRBS_SEED_0U_SEED                  ((uint32_t)0x0000ffff)

/* RXS_LANEX_PRBS_SEED_0M : Register Bits Masks Definitions */
#define RXS_LANEX_PRBS_SEED_0M_SEED                  ((uint32_t)0xffffffff)

/* RXS_LANEX_PRBS_SEED_0L : Register Bits Masks Definitions */
#define RXS_LANEX_PRBS_SEED_0L_SEED                  ((uint32_t)0xffffffff)

/* RXS_LANEX_PRBS_SEED_1U : Register Bits Masks Definitions */
#define RXS_LANEX_PRBS_SEED_1U_SEED                  ((uint32_t)0x0000ffff)

/* RXS_LANEX_PRBS_SEED_1M : Register Bits Masks Definitions */
#define RXS_LANEX_PRBS_SEED_1M_SEED                  ((uint32_t)0xffffffff)

/* RXS_LANEX_PRBS_SEED_1L : Register Bits Masks Definitions */
#define RXS_LANEX_PRBS_SEED_1L_SEED                  ((uint32_t)0xffffffff)

/* RXS_LANEX_BER_CTL : Register Bits Masks Definitions */
#define RXS_LANEX_BER_CTL_COUNT                      ((uint32_t)0x000fffff)
#define RXS_LANEX_BER_CTL_BLIP_CNT                   ((uint32_t)0x01000000)
#define RXS_LANEX_BER_CTL_BLIP_SCRAM                 ((uint32_t)0x02000000)
#define RXS_LANEX_BER_CTL_BLIP_BIP                   ((uint32_t)0x04000000)
#define RXS_LANEX_BER_CTL_WIDTH                      ((uint32_t)0xf0000000)

/* RXS_LANEX_BER_DATA_0 : Register Bits Masks Definitions */
#define RXS_LANEX_BER_DATA_0_DATA_LO                 ((uint32_t)0xffffffff)

/* RXS_LANEX_BER_DATA_1 : Register Bits Masks Definitions */
#define RXS_LANEX_BER_DATA_1_DATA_HI                 ((uint32_t)0xffffffff)

/* RXS_LANEX_PCS_DBG : Register Bits Masks Definitions */
#define RXS_LANEX_PCS_DBG_LOCK_DIS                   ((uint32_t)0x00000001)
#define RXS_LANEX_PCS_DBG_KEEP_SKIP                  ((uint32_t)0x00000002)
#define RXS_LANEX_PCS_DBG_FRC_RX_EN                  ((uint32_t)0x00000004)
#define RXS_LANEX_PCS_DBG_LSYNC_DIS                  ((uint32_t)0x80000000)

/* RXS_LANEX_BERM_CTL : Register Bits Masks Definitions */
#define RXS_LANEX_BERM_CTL_DEGRADED_THRESHOLD        ((uint32_t)0x00000007)
#define RXS_LANEX_BERM_CTL_TEST_PERIODS              ((uint32_t)0x00000070)
#define RXS_LANEX_BERM_CTL_COUNTING_MODE             ((uint32_t)0x00040000)
#define RXS_LANEX_BERM_CTL_INVALID_DEGRADES          ((uint32_t)0x00080000)
#define RXS_LANEX_BERM_CTL_UNUSED2                   ((uint32_t)0x00100000)
#define RXS_LANEX_BERM_CTL_DEGRADED                  ((uint32_t)0x00200000)
#define RXS_LANEX_BERM_CTL_VALID                     ((uint32_t)0x00400000)
#define RXS_LANEX_BERM_CTL_MAX_COUNT                 ((uint32_t)0x00800000)
#define RXS_LANEX_BERM_CTL_STOP_ON_ERR               ((uint32_t)0x01000000)
#define RXS_LANEX_BERM_CTL_TRAINING_CHECK            ((uint32_t)0x02000000)
#define RXS_LANEX_BERM_CTL_UNUSED1                   ((uint32_t)0x04000000)
#define RXS_LANEX_BERM_CTL_RETRAIN                   ((uint32_t)0x08000000)
#define RXS_LANEX_BERM_CTL_HALT                      ((uint32_t)0x10000000)
#define RXS_LANEX_BERM_CTL_MODE                      ((uint32_t)0x20000000)
#define RXS_LANEX_BERM_CTL_MONITOR                   ((uint32_t)0x40000000)
#define RXS_LANEX_BERM_CTL_HW_BERM                   ((uint32_t)0x80000000)

/* RXS_LANEX_BERM_CNTR : Register Bits Masks Definitions */
#define RXS_LANEX_BERM_CNTR_CUR_ERROR_COUNT          ((uint32_t)0x0000ffff)
#define RXS_LANEX_BERM_CNTR_LAST_ERROR_COUNT         ((uint32_t)0xffff0000)

/* RXS_LANEX_BERM_PD : Register Bits Masks Definitions */
#define RXS_LANEX_BERM_PD_TEST_PERIOD                ((uint32_t)0xffffffff)

/* RXS_LANEX_BERM_BITS : Register Bits Masks Definitions */
#define RXS_LANEX_BERM_BITS_TESTED_BITS              ((uint32_t)0xffffffff)

/* RXS_LANEX_BERM_ERRORS : Register Bits Masks Definitions */
#define RXS_LANEX_BERM_ERRORS_ERROR_THRESHOLD        ((uint32_t)0x0000ffff)

/* RXS_LANEX_BERM_PERIODS : Register Bits Masks Definitions */
#define RXS_LANEX_BERM_PERIODS_DEGRADED_PERIODS      ((uint32_t)0x0000ff00)
#define RXS_LANEX_BERM_PERIODS_VALID_PERIODS         ((uint32_t)0x00ff0000)
#define RXS_LANEX_BERM_PERIODS_COMPLETED_PERIODS     ((uint32_t)0xff000000)

/* RXS_LANEX_DME_TEST : Register Bits Masks Definitions */
#define RXS_LANEX_DME_TEST_LOCK                      ((uint32_t)0x00000001)
#define RXS_LANEX_DME_TEST_BAD_CELL                  ((uint32_t)0x00000002)
#define RXS_LANEX_DME_TEST_BAD_RES                   ((uint32_t)0x00000004)
#define RXS_LANEX_DME_TEST_BAD_FM                    ((uint32_t)0x00000008)
#define RXS_LANEX_DME_TEST_BAD_FM_MAX                ((uint32_t)0x00070000)
#define RXS_LANEX_DME_TEST_CC_STRICT                 ((uint32_t)0x03000000)
#define RXS_LANEX_DME_TEST_FM_STRICT                 ((uint32_t)0x30000000)

/* RXS_FAB_PORT_BH : Register Bits Masks Definitions */
#define RXS_FAB_PORT_BH_BLK_TYPE                     ((uint32_t)0x00000fff)
#define RXS_FAB_PORT_BH_BLK_REV                      ((uint32_t)0x0000f000)
#define RXS_FAB_PORT_BH_NEXT_BLK_PTR                 ((uint32_t)0xffff0000)

/* RXS_FP_X_IB_BUFF_WM_01 : Register Bits Masks Definitions */
#define RXS_FP_X_IB_BUFF_WM_01_IB_PRIO0_CRF1_RSVD_PAGES ((uint32_t)0x0000ff00)
#define RXS_FP_X_IB_BUFF_WM_01_IB_PRIO1_CRF0_RSVD_PAGES ((uint32_t)0x00ff0000)
#define RXS_FP_X_IB_BUFF_WM_01_IB_PRIO1_CRF1_RSVD_PAGES ((uint32_t)0xff000000)

/* RXS_FP_X_IB_BUFF_WM_23 : Register Bits Masks Definitions */
#define RXS_FP_X_IB_BUFF_WM_23_IB_PRIO2_CRF0_RSVD_PAGES ((uint32_t)0x000000ff)
#define RXS_FP_X_IB_BUFF_WM_23_IB_PRIO2_CRF1_RSVD_PAGES ((uint32_t)0x0000ff00)
#define RXS_FP_X_IB_BUFF_WM_23_IB_PRIO3_CRF0_RSVD_PAGES ((uint32_t)0x00ff0000)
#define RXS_FP_X_IB_BUFF_WM_23_IB_PRIO3_CRF1_RSVD_PAGES ((uint32_t)0xff000000)

/* RXS_FP_X_PLW_SCRATCH : Register Bits Masks Definitions */
#define RXS_FP_X_PLW_SCRATCH_SCRATCH                 ((uint32_t)0xffffffff)

/* RXS_BC_L0_G0_ENTRYX_CSR : Register Bits Masks Definitions */
#define RXS_BC_L0_G0_ENTRYX_CSR_ROUTING_VALUE        ((uint32_t)0x000003ff)
#define RXS_BC_L0_G0_ENTRYX_CSR_CAPTURE              ((uint32_t)0x80000000)

/* RXS_BC_L1_GX_ENTRYY_CSR : Register Bits Masks Definitions */
#define RXS_BC_L1_GX_ENTRYY_CSR_ROUTING_VALUE        ((uint32_t)0x000003ff)
#define RXS_BC_L1_GX_ENTRYY_CSR_CAPTURE              ((uint32_t)0x80000000)

/* RXS_BC_L2_GX_ENTRYY_CSR : Register Bits Masks Definitions */
#define RXS_BC_L2_GX_ENTRYY_CSR_ROUTING_VALUE        ((uint32_t)0x000003ff)
#define RXS_BC_L2_GX_ENTRYY_CSR_CAPTURE              ((uint32_t)0x80000000)

/* RXS_BC_MC_X_S_CSR : Register Bits Masks Definitions */
#define RXS_BC_MC_X_S_CSR_SET                        ((uint32_t)0x0000ffff)
#define RXS_BC_MC_X_S_CSR_UNUSED                     ((uint32_t)0x00ff0000)

/* RXS_BC_MC_X_C_CSR : Register Bits Masks Definitions */
#define RXS_BC_MC_X_C_CSR_CLR                        ((uint32_t)0x0000ffff)
#define RXS_BC_MC_X_C_CSR_UNUSED                     ((uint32_t)0x00ff0000)

/* RXS_FAB_INGR_CTL_BH : Register Bits Masks Definitions */
#define RXS_FAB_INGR_CTL_BH_BLK_TYPE                 ((uint32_t)0x00000fff)
#define RXS_FAB_INGR_CTL_BH_BLK_REV                  ((uint32_t)0x0000f000)
#define RXS_FAB_INGR_CTL_BH_NEXT_BLK_PTR             ((uint32_t)0xffff0000)

/* RXS_FAB_IG_X_2X4X_4X2X_WM : Register Bits Masks Definitions */
#define RXS_FAB_IG_X_2X4X_4X2X_WM_CB_PRIO1_RSVD_PAGES ((uint32_t)0x00001f00)
#define RXS_FAB_IG_X_2X4X_4X2X_WM_CB_PRIO2_RSVD_PAGES ((uint32_t)0x001f0000)
#define RXS_FAB_IG_X_2X4X_4X2X_WM_CB_PRIO3_RSVD_PAGES ((uint32_t)0x1f000000)

/* RXS_FAB_IG_X_2X2X_WM : Register Bits Masks Definitions */
#define RXS_FAB_IG_X_2X2X_WM_CB_PRIO1_RSVD_PAGES     ((uint32_t)0x00000f00)
#define RXS_FAB_IG_X_2X2X_WM_CB_PRIO2_RSVD_PAGES     ((uint32_t)0x000f0000)
#define RXS_FAB_IG_X_2X2X_WM_CB_PRIO3_RSVD_PAGES     ((uint32_t)0x0f000000)

/* RXS_FAB_IG_X_4X4X_WM : Register Bits Masks Definitions */
#define RXS_FAB_IG_X_4X4X_WM_CB_PRIO1_RSVD_PAGES     ((uint32_t)0x00003f00)
#define RXS_FAB_IG_X_4X4X_WM_CB_PRIO2_RSVD_PAGES     ((uint32_t)0x003f0000)
#define RXS_FAB_IG_X_4X4X_WM_CB_PRIO3_RSVD_PAGES     ((uint32_t)0x3f000000)

/* RXS_FAB_IG_X_MTC_VOQ_ACT : Register Bits Masks Definitions */
#define RXS_FAB_IG_X_MTC_VOQ_ACT_ACTIVE              ((uint32_t)0x00000001)

/* RXS_FAB_IG_X_VOQ_ACT : Register Bits Masks Definitions */
#define RXS_FAB_IG_X_VOQ_ACT_ACTIVE                  ((uint32_t)0x0000ffff)
#define RXS_FAB_IG_X_VOQ_ACT_UNUSED                  ((uint32_t)0x00ff0000)

/* RXS_FAB_IG_X_CTL : Register Bits Masks Definitions */
#define RXS_FAB_IG_X_CTL_GRAD_THRESHOLD              ((uint32_t)0x001f0000)
#define RXS_FAB_IG_X_CTL_GRAD_AGE                    ((uint32_t)0x01000000)
#define RXS_FAB_IG_X_CTL_INST_AGE                    ((uint32_t)0x10000000)

/* RXS_FAB_IG_X_SCRATCH : Register Bits Masks Definitions */
#define RXS_FAB_IG_X_SCRATCH_SCRATCH                 ((uint32_t)0xffffffff)

/* RXS_EM_BH : Register Bits Masks Definitions */
#define RXS_EM_BH_BLK_TYPE                           ((uint32_t)0x00000fff)
#define RXS_EM_BH_BLK_REV                            ((uint32_t)0x0000f000)
#define RXS_EM_BH_NEXT_BLK_PTR                       ((uint32_t)0xffff0000)

/* RXS_EM_INT_STAT : Register Bits Masks Definitions */
#define RXS_EM_INT_STAT_IG_DATA_UNCOR                ((uint32_t)0x00000400)
#define RXS_EM_INT_STAT_IG_DATA_COR                  ((uint32_t)0x00000800)
#define RXS_EM_INT_STAT_DEL_A                        ((uint32_t)0x00001000)
#define RXS_EM_INT_STAT_DEL_B                        ((uint32_t)0x00002000)
#define RXS_EM_INT_STAT_BOOT_MEM_UNCOR               ((uint32_t)0x00004000)
#define RXS_EM_INT_STAT_BOOT_MEM_COR                 ((uint32_t)0x00008000)
#define RXS_EM_INT_STAT_EXTERNAL                     ((uint32_t)0x00fe0000)
#define RXS_EM_INT_STAT_EXTERNAL_I2C                 ((uint32_t)0x00800000)
#define RXS_EM_INT_STAT_EXTERNAL_AEC                 ((uint32_t)0x00400000)
#define RXS_EM_INT_STAT_EXTERNAL_PHY_MEM             ((uint32_t)0x00200000)
#define RXS_EM_INT_STAT_EXTERNAL_PVT_SNSR            ((uint32_t)0x00100000)
#define RXS_EM_INT_STAT_MECS                         ((uint32_t)0x04000000)
#define RXS_EM_INT_STAT_RCS                          ((uint32_t)0x08000000)
#define RXS_EM_INT_STAT_LOG                          ((uint32_t)0x10000000)
#define RXS_EM_INT_STAT_PORT                         ((uint32_t)0x20000000)
#define RXS_EM_INT_STAT_FAB                          ((uint32_t)0x40000000)

/* RXS_EM_INT_EN : Register Bits Masks Definitions */
#define RXS_EM_INT_EN_IG_DATA_UNCOR                  ((uint32_t)0x00000400)
#define RXS_EM_INT_EN_IG_DATA_COR                    ((uint32_t)0x00000800)
#define RXS_EM_INT_EN_DEL_A                          ((uint32_t)0x00001000)
#define RXS_EM_INT_EN_DEL_B                          ((uint32_t)0x00002000)
#define RXS_EM_INT_EN_BOOT_MEM_UNCOR                 ((uint32_t)0x00004000)
#define RXS_EM_INT_EN_BOOT_MEM_COR                   ((uint32_t)0x00008000)
#define RXS_EM_INT_EN_EXTERNAL                       ((uint32_t)0x00fe0000)
#define RXS_EM_INT_EN_EXTERNAL_I2C                   ((uint32_t)0x00800000)
#define RXS_EM_INT_EN_EXTERNAL_AEC                   ((uint32_t)0x00400000)
#define RXS_EM_INT_EN_EXTERNAL_PHY_MEM               ((uint32_t)0x00200000)
#define RXS_EM_INT_EN_EXTERNAL_PVT_SNSR              ((uint32_t)0x00100000)
#define RXS_EM_INT_EN_MECS                           ((uint32_t)0x04000000)
#define RXS_EM_INT_EN_LOG                            ((uint32_t)0x10000000)
#define RXS_EM_INT_EN_FAB                            ((uint32_t)0x40000000)

/* RXS_EM_INT_PORT_STAT : Register Bits Masks Definitions */
#define RXS_EM_INT_PORT_STAT_IRQ_PENDING             ((uint32_t)0x0000ffff)
#define RXS_EM_INT_PORT_STAT_UNUSED                  ((uint32_t)0x00ff0000)

/* RXS_EM_PW_STAT : Register Bits Masks Definitions */
#define RXS_EM_PW_STAT_MULTIPORT_ERR                 ((uint32_t)0x00000200)
#define RXS_EM_PW_STAT_IG_DATA_UNCOR                 ((uint32_t)0x00000400)
#define RXS_EM_PW_STAT_IG_DATA_COR                   ((uint32_t)0x00000800)
#define RXS_EM_PW_STAT_DEL_A                         ((uint32_t)0x00001000)
#define RXS_EM_PW_STAT_DEL_B                         ((uint32_t)0x00002000)
#define RXS_EM_PW_STAT_BOOT_MEM_UNCOR                ((uint32_t)0x00004000)
#define RXS_EM_PW_STAT_BOOT_MEM_COR                  ((uint32_t)0x00008000)
#define RXS_EM_PW_STAT_EXTERNAL                      ((uint32_t)0x00fe0000)
#define RXS_EM_PW_STAT_EXTERNAL_I2C                  ((uint32_t)0x00800000)
#define RXS_EM_PW_STAT_EXTERNAL_AEC                  ((uint32_t)0x00400000)
#define RXS_EM_PW_STAT_EXTERNAL_PHY_MEM              ((uint32_t)0x00200000)
#define RXS_EM_PW_STAT_EXTERNAL_PVT_SNSR             ((uint32_t)0x00100000)
#define RXS_EM_PW_STAT_RCS                           ((uint32_t)0x08000000)
#define RXS_EM_PW_STAT_LOG                           ((uint32_t)0x10000000)
#define RXS_EM_PW_STAT_PORT                          ((uint32_t)0x20000000)
#define RXS_EM_PW_STAT_FAB                           ((uint32_t)0x40000000)

/* RXS_EM_PW_EN : Register Bits Masks Definitions */
#define RXS_EM_PW_EN_IG_DATA_UNCOR                   ((uint32_t)0x00000400)
#define RXS_EM_PW_EN_IG_DATA_COR                     ((uint32_t)0x00000800)
#define RXS_EM_PW_EN_DEL_A                           ((uint32_t)0x00001000)
#define RXS_EM_PW_EN_DEL_B                           ((uint32_t)0x00002000)
#define RXS_EM_PW_EN_BOOT_MEM_UNCOR                  ((uint32_t)0x00004000)
#define RXS_EM_PW_EN_BOOT_MEM_COR                    ((uint32_t)0x00008000)
#define RXS_EM_PW_EN_EXTERNAL                        ((uint32_t)0x00fe0000)
#define RXS_EM_PW_EN_EXTERNAL_I2C                    ((uint32_t)0x00800000)
#define RXS_EM_PW_EN_EXTERNAL_AEC                    ((uint32_t)0x00400000)
#define RXS_EM_PW_EN_EXTERNAL_PHY_MEM                ((uint32_t)0x00200000)
#define RXS_EM_PW_EN_EXTERNAL_PVT_SNSR               ((uint32_t)0x00100000)
#define RXS_EM_PW_EN_LOG                             ((uint32_t)0x10000000)
#define RXS_EM_PW_EN_FAB                             ((uint32_t)0x40000000)

/* RXS_EM_PW_PORT_STAT : Register Bits Masks Definitions */
#define RXS_EM_PW_PORT_STAT_PW_PENDING               ((uint32_t)0x0000ffff)
#define RXS_EM_PW_PORT_STAT_UNUSED                   ((uint32_t)0x00ff0000)

/* RXS_EM_DEV_INT_EN : Register Bits Masks Definitions */
#define RXS_EM_DEV_INT_EN_INT_EN                     ((uint32_t)0x00000001)
#define RXS_EM_DEV_INT_EN_INT                        ((uint32_t)0x00010000)

/* RXS_EM_EVENT_GEN : Register Bits Masks Definitions */
#define RXS_EM_EVENT_GEN_IG_DATA_UNCOR               ((uint32_t)0x00000400)
#define RXS_EM_EVENT_GEN_IG_DATA_COR                 ((uint32_t)0x00000800)
#define RXS_EM_EVENT_GEN_BOOT_MEM_UNCOR              ((uint32_t)0x00004000)
#define RXS_EM_EVENT_GEN_BOOT_MEM_COR                ((uint32_t)0x00008000)

/* RXS_EM_MECS_CTL : Register Bits Masks Definitions */
#define RXS_EM_MECS_CTL_OUT_EN                       ((uint32_t)0x00000001)
#define RXS_EM_MECS_CTL_IN_EN                        ((uint32_t)0x00000002)
#define RXS_EM_MECS_CTL_SEND                         ((uint32_t)0x00000010)
#define RXS_EM_MECS_CTL_IN_EDGE                      ((uint32_t)0x00000100)

/* RXS_EM_MECS_INT_EN : Register Bits Masks Definitions */
#define RXS_EM_MECS_INT_EN_MECS_INT_EN               ((uint32_t)0x0000ffff)
#define RXS_EM_MECS_INT_EN_UNUSED                    ((uint32_t)0x00ff0000)

/* RXS_EM_MECS_PORT_STAT : Register Bits Masks Definitions */
#define RXS_EM_MECS_PORT_STAT_PORT                   ((uint32_t)0x0000ffff)
#define RXS_EM_MECS_PORT_STAT_UNUSED                 ((uint32_t)0x00ff0000)

/* RXS_EM_RST_PORT_STAT : Register Bits Masks Definitions */
#define RXS_EM_RST_PORT_STAT_RST_REQ                 ((uint32_t)0x0000ffff)
#define RXS_EM_RST_PORT_STAT_UNUSED                  ((uint32_t)0x00ff0000)

/* RXS_EM_RST_INT_EN : Register Bits Masks Definitions */
#define RXS_EM_RST_INT_EN_RST_INT_EN                 ((uint32_t)0x0000ffff)
#define RXS_EM_RST_INT_EN_UNUSED                     ((uint32_t)0x00ff0000)

/* RXS_EM_RST_PW_EN : Register Bits Masks Definitions */
#define RXS_EM_RST_PW_EN_RST_PW_EN                   ((uint32_t)0x0000ffff)
#define RXS_EM_RST_PW_EN_UNUSED                      ((uint32_t)0x00ff0000)

/* RXS_EM_FAB_INT_STAT : Register Bits Masks Definitions */
#define RXS_EM_FAB_INT_STAT_PORT                     ((uint32_t)0x0000ffff)
#define RXS_EM_FAB_INT_STAT_UNUSED                   ((uint32_t)0x00ff0000)

/* RXS_EM_FAB_PW_STAT : Register Bits Masks Definitions */
#define RXS_EM_FAB_PW_STAT_PORT                      ((uint32_t)0x0000ffff)
#define RXS_EM_FAB_PW_STAT_UNUSED                    ((uint32_t)0x00ff0000)

/* RXS_PW_BH : Register Bits Masks Definitions */
#define RXS_PW_BH_BLK_TYPE                           ((uint32_t)0x00000fff)
#define RXS_PW_BH_BLK_REV                            ((uint32_t)0x0000f000)
#define RXS_PW_BH_NEXT_BLK_PTR                       ((uint32_t)0xffff0000)

/* RXS_PW_CTL : Register Bits Masks Definitions */
#define RXS_PW_CTL_PW_TMR                            ((uint32_t)0xffffff00)
#define RXS_PW_CTL_PW_TMR_NSEC          ((uint32_t)(4990000000 / 0xFFFFFF))
#define RXS_PW_CTL_PW_TMR_MAX            ((uint32_t)RXS_PW_CTL_PW_TMR >> 8)

/* RXS_PW_ROUTE : Register Bits Masks Definitions */
#define RXS_PW_ROUTE_PORT                            ((uint32_t)0x0000ffff)
#define RXS_PW_ROUTE_UNUSED                          ((uint32_t)0x00ff0000)

/* RXS_MPM_BH : Register Bits Masks Definitions */
#define RXS_MPM_BH_BLK_TYPE                          ((uint32_t)0x00000fff)
#define RXS_MPM_BH_BLK_REV                           ((uint32_t)0x0000f000)
#define RXS_MPM_BH_NEXT_BLK_PTR                      ((uint32_t)0xffff0000)

/* RXS_RB_RESTRICT : Register Bits Masks Definitions */
#define RXS_RB_RESTRICT_I2C_WR                       ((uint32_t)0x00000001)
#define RXS_RB_RESTRICT_JTAG_WR                      ((uint32_t)0x00000002)
#define RXS_RB_RESTRICT_SRIO_WR                      ((uint32_t)0x00000004)
#define RXS_RB_RESTRICT_I2C_RD                       ((uint32_t)0x00010000)
#define RXS_RB_RESTRICT_JTAG_RD                      ((uint32_t)0x00020000)
#define RXS_RB_RESTRICT_SRIO_RD                      ((uint32_t)0x00040000)

/* RXS_MTC_WR_RESTRICT : Register Bits Masks Definitions */
#define RXS_MTC_WR_RESTRICT_WR_DIS                   ((uint32_t)0x0000ffff)
#define RXS_MTC_WR_RESTRICT_UNUSED                   ((uint32_t)0x00ff0000)

/* RXS_MTC_RD_RESTRICT : Register Bits Masks Definitions */
#define RXS_MTC_RD_RESTRICT_RD_DIS                   ((uint32_t)0x0000ffff)
#define RXS_MTC_RD_RESTRICT_UNUSED                   ((uint32_t)0x00ff0000)

/* RXS_MPM_SCRATCH1 : Register Bits Masks Definitions */
#define RXS_MPM_SCRATCH1_SCRATCH                     ((uint32_t)0xffffffff)

/* RXS_PORT_NUMBER : Register Bits Masks Definitions */
#define RXS_PORT_NUMBER_PORT_NUM                     ((uint32_t)0x000000ff)
#define RXS_PORT_NUMBER_PORT_TOTAL                   ((uint32_t)0x0000ff00)

/* RXS_PRESCALAR_SRV_CLK : Register Bits Masks Definitions */
#define RXS_PRESCALAR_SRV_CLK_PRESCALAR_SRV_CLK      ((uint32_t)0x000000ff)

/* RXS_REG_RST_CTL : Register Bits Masks Definitions */
#define RXS_REG_RST_CTL_CLEAR_STICKY                 ((uint32_t)0x00000001)
#define RXS_REG_RST_CTL_SOFT_RST_DEV                 ((uint32_t)0x00000002)

/* RXS_MPM_SCRATCH2 : Register Bits Masks Definitions */
#define RXS_MPM_SCRATCH2_SCRATCH                     ((uint32_t)0xffffffff)

/* RXS_ASBLY_ID_OVERRIDE : Register Bits Masks Definitions */
#define RXS_ASBLY_ID_OVERRIDE_ASBLY_VEN_ID           ((uint32_t)0x0000ffff)
#define RXS_ASBLY_ID_OVERRIDE_ASBLY_ID               ((uint32_t)0xffff0000)

/* RXS_ASBLY_INFO_OVERRIDE : Register Bits Masks Definitions */
#define RXS_ASBLY_INFO_OVERRIDE_EXT_FEAT_PTR         ((uint32_t)0x0000ffff)
#define RXS_ASBLY_INFO_OVERRIDE_ASBLY_REV            ((uint32_t)0xffff0000)

/* RXS_MPM_MTC_RESP_PRIO : Register Bits Masks Definitions */
#define RXS_MPM_MTC_RESP_PRIO_CRF                    ((uint32_t)0x00000001)
#define RXS_MPM_MTC_RESP_PRIO_PRIO                   ((uint32_t)0x00000006)
#define RXS_MPM_MTC_RESP_PRIO_PLUS_1                 ((uint32_t)0x00000010)

/* RXS_MPM_MTC_ACTIVE : Register Bits Masks Definitions */
#define RXS_MPM_MTC_ACTIVE_MTC_ACTIVE                ((uint32_t)0x00ffffff)

/* RXS_MPM_CFGSIG0 : Register Bits Masks Definitions */
#define RXS_MPM_CFGSIG0_SWAP_RX                      ((uint32_t)0x00000003)
#define RXS_MPM_CFGSIG0_RX_POLARITY                  ((uint32_t)0x00000004)
#define RXS_MPM_CFGSIG0_SWAP_TX                      ((uint32_t)0x00000030)
#define RXS_MPM_CFGSIG0_TX_POLARITY                  ((uint32_t)0x00000040)
#define RXS_MPM_CFGSIG0_CORECLK_SELECT               ((uint32_t)0x00000600)
#define RXS_MPM_CFGSIG0_CORECLK_SELECT_LO_LAT        ((uint32_t)0x00000600)
#define RXS_MPM_CFGSIG0_CORECLK_SELECT_RSVD          ((uint32_t)0x00000400)
#define RXS_MPM_CFGSIG0_CORECLK_SELECT_LO_PWR_12G    ((uint32_t)0x00000200)
#define RXS_MPM_CFGSIG0_CORECLK_SELECT_LO_PWR_10G    ((uint32_t)0x00000000)
#define RXS_MPM_CFGSIG0_REFCLK_SELECT                ((uint32_t)0x00000800)
#define RXS_MPM_CFGSIG0_REFCLK_SELECT_100MHZ         ((uint32_t)0x00000000)
#define RXS_MPM_CFGSIG0_REFCLK_SELECT_156P25MHZ      ((uint32_t)0x00000800)
#define RXS_MPM_CFGSIG0_BAUD_SELECT                  ((uint32_t)0x0000f000)
#define RXS_MPM_CFGSIG0_HW_CMD                       ((uint32_t)0x01000000)
#define RXS_MPM_CFGSIG0_HW_ACK                       ((uint32_t)0x02000000)
#define RXS_MPM_CFGSIG0_TX_MODE                      ((uint32_t)0x10000000)

/* RXS_MPM_CFGSIG1 : Register Bits Masks Definitions */
#define RXS_MPM_CFGSIG1_PORT_SELECT_STRAP            ((uint32_t)0x00ffffff)

/* RXS_MPM_CFGSIG2 : Register Bits Masks Definitions */
#define RXS_MPM_CFGSIG2_POWER_ON                     ((uint32_t)0x00000100)
#define RXS_MPM_CFGSIG2_CLOCK_ON                     ((uint32_t)0x00000200)
#define RXS_MPM_CFGSIG2_SLEEP_DISABLE                ((uint32_t)0x00000400)
#define RXS_MPM_CFGSIG2_UNUSED                       ((uint32_t)0x00030000)
#define RXS_MPM_CFGSIG2_I2C_MA                       ((uint32_t)0x01000000)
#define RXS_MPM_CFGSIG2_I2C_SEL                      ((uint32_t)0x02000000)
#define RXS_MPM_CFGSIG2_I2C_DISABLE                  ((uint32_t)0x04000000)
#define RXS_MPM_CFGSIG2_I2C_SA                       ((uint32_t)0xf0000000)

/* RXS_FAB_GEN_BH : Register Bits Masks Definitions */
#define RXS_FAB_GEN_BH_BLK_TYPE                      ((uint32_t)0x00000fff)
#define RXS_FAB_GEN_BH_BLK_REV                       ((uint32_t)0x0000f000)
#define RXS_FAB_GEN_BH_NEXT_BLK_PTR                  ((uint32_t)0xffff0000)

/* RXS_FAB_GLOBAL_MEM_PWR_MODE : Register Bits Masks Definitions */
#define RXS_FAB_GLOBAL_MEM_PWR_MODE_PWR_MODE         ((uint32_t)0x00000003)

/* RXS_FAB_GLOBAL_CLK_GATE : Register Bits Masks Definitions */
#define RXS_FAB_GLOBAL_CLK_GATE_GLBL_PORT_CGATE      ((uint32_t)0x0000ffff)
#define RXS_FAB_GLOBAL_CLK_GATE_UNUSED               ((uint32_t)0x00ff0000)

/* RXS_FAB_4X_MODE : Register Bits Masks Definitions */
#define RXS_FAB_4X_MODE_FAB_4X_MODE                  ((uint32_t)0x000000ff)
#define RXS_FAB_4X_MODE_UNUSED                       ((uint32_t)0x00000f00)

/* RXS_FAB_MBCOL_ACT : Register Bits Masks Definitions */
#define RXS_FAB_MBCOL_ACT_ACTIVE                     ((uint32_t)0x0000ffff)
#define RXS_FAB_MBCOL_ACT_UNUSED                     ((uint32_t)0x00ff0000)

/* RXS_FAB_MIG_ACT : Register Bits Masks Definitions */
#define RXS_FAB_MIG_ACT_ACTIVE                       ((uint32_t)0x0000ffff)
#define RXS_FAB_MIG_ACT_UNUSED                       ((uint32_t)0x00ff0000)

/* RXS_PHY_SERIAL_IF_EN : Register Bits Masks Definitions */
#define RXS_PHY_SERIAL_IF_EN_SERIAL_PORT_OK_EN       ((uint32_t)0x20000000)
#define RXS_PHY_SERIAL_IF_EN_SERIAL_LOOPBACK_EN      ((uint32_t)0x40000000)
#define RXS_PHY_SERIAL_IF_EN_SERIAL_TX_DISABLE_EN    ((uint32_t)0x80000000)

/* RXS_PHY_TX_DISABLE_CTRL : Register Bits Masks Definitions */
#define RXS_PHY_TX_DISABLE_CTRL_PHY_TX_DISABLE       ((uint32_t)0x00ffffff)
#define RXS_PHY_TX_DISABLE_CTRL_DONE                 ((uint32_t)0x20000000)
#define RXS_PHY_TX_DISABLE_CTRL_POLARITY             ((uint32_t)0x40000000)
#define RXS_PHY_TX_DISABLE_CTRL_START                ((uint32_t)0x80000000)

/* RXS_PHY_LOOPBACK_CTRL : Register Bits Masks Definitions */
#define RXS_PHY_LOOPBACK_CTRL_PHY_LOOPBACK           ((uint32_t)0x00ffffff)
#define RXS_PHY_LOOPBACK_CTRL_DONE                   ((uint32_t)0x20000000)
#define RXS_PHY_LOOPBACK_CTRL_POLARITY               ((uint32_t)0x40000000)
#define RXS_PHY_LOOPBACK_CTRL_START                  ((uint32_t)0x80000000)

/* RXS_PHY_PORT_OK_CTRL : Register Bits Masks Definitions */
#define RXS_PHY_PORT_OK_CTRL_CFG_TMR                 ((uint32_t)0x00ffffff)
#define RXS_PHY_PORT_OK_CTRL_POLARITY                ((uint32_t)0x80000000)

/* RXS_MCM_ROUTE_EN : Register Bits Masks Definitions */
#define RXS_MCM_ROUTE_EN_RT_EN                       ((uint32_t)0x0000ffff)
#define RXS_MCM_ROUTE_EN_UNUSED                      ((uint32_t)0x00ff0000)

/* RXS_BOOT_CTL : Register Bits Masks Definitions */
#define RXS_BOOT_CTL_BOOT_FAIL                       ((uint32_t)0x20000000)
#define RXS_BOOT_CTL_BOOT_OK                         ((uint32_t)0x40000000)
#define RXS_BOOT_CTL_BOOT_CMPLT                      ((uint32_t)0x80000000)

/* RXS_FAB_GLOBAL_PWR_GATE_CLR : Register Bits Masks Definitions */
#define RXS_FAB_GLOBAL_PWR_GATE_CLR_PGATE_CLR        ((uint32_t)0x80000000)

/* RXS_FAB_GLOBAL_PWR_GATE : Register Bits Masks Definitions */
#define RXS_FAB_GLOBAL_PWR_GATE_CB_COL_PGATE         ((uint32_t)0x000000ff)
#define RXS_FAB_GLOBAL_PWR_GATE_UNUSED               ((uint32_t)0x00000f00)

/* RXS_DEL_BH : Register Bits Masks Definitions */
#define RXS_DEL_BH_BLK_TYPE                          ((uint32_t)0x00000fff)
#define RXS_DEL_BH_BLK_REV                           ((uint32_t)0x0000f000)
#define RXS_DEL_BH_NEXT_BLK_PTR                      ((uint32_t)0xffff0000)

/* RXS_DEL_ACC : Register Bits Masks Definitions */
#define RXS_DEL_ACC_EL_ON                            ((uint32_t)0x80000000)

/* RXS_DEL_INFO : Register Bits Masks Definitions */
#define RXS_DEL_INFO_MAX_STAT_INDEX                  ((uint32_t)0x0000000f)
#define RXS_DEL_INFO_MEM_SIZE                        ((uint32_t)0x00000700)
#define RXS_DEL_INFO_VERSION                         ((uint32_t)0xff000000)

/* RXS_DEL_CTL : Register Bits Masks Definitions */
#define RXS_DEL_CTL_T0_EN                            ((uint32_t)0x00000001)
#define RXS_DEL_CTL_T0_INTA                          ((uint32_t)0x00000002)
#define RXS_DEL_CTL_T0_INTB                          ((uint32_t)0x00000004)
#define RXS_DEL_CTL_T1_EN                            ((uint32_t)0x00000010)
#define RXS_DEL_CTL_T1_INTA                          ((uint32_t)0x00000020)
#define RXS_DEL_CTL_T1_INTB                          ((uint32_t)0x00000040)
#define RXS_DEL_CTL_TR_COND                          ((uint32_t)0x00000700)
#define RXS_DEL_CTL_TR_INTA                          ((uint32_t)0x00001000)
#define RXS_DEL_CTL_TR_INTB                          ((uint32_t)0x00002000)
#define RXS_DEL_CTL_RST                              ((uint32_t)0x00008000)
#define RXS_DEL_CTL_TRIG_POS                         ((uint32_t)0x00ff0000)

/* RXS_DEL_INT_EN : Register Bits Masks Definitions */
#define RXS_DEL_INT_EN_EN_A                          ((uint32_t)0x00000001)
#define RXS_DEL_INT_EN_EN_B                          ((uint32_t)0x00000002)

/* RXS_DEL_SRC_LOG_EN : Register Bits Masks Definitions */
#define RXS_DEL_SRC_LOG_EN_EN_0                      ((uint32_t)0x00000001)
#define RXS_DEL_SRC_LOG_EN_EN_1                      ((uint32_t)0x00000002)
#define RXS_DEL_SRC_LOG_EN_EN_2                      ((uint32_t)0x00000004)
#define RXS_DEL_SRC_LOG_EN_EN_3                      ((uint32_t)0x00000008)
#define RXS_DEL_SRC_LOG_EN_EN_4                      ((uint32_t)0x00000010)
#define RXS_DEL_SRC_LOG_EN_EN_5                      ((uint32_t)0x00000020)
#define RXS_DEL_SRC_LOG_EN_EN_6                      ((uint32_t)0x00000040)
#define RXS_DEL_SRC_LOG_EN_EN_7                      ((uint32_t)0x00000080)
#define RXS_DEL_SRC_LOG_EN_CLR                       ((uint32_t)0x80000000)

/* RXS_DEL_TRIG0_MASK : Register Bits Masks Definitions */
#define RXS_DEL_TRIG0_MASK_MASK                      ((uint32_t)0x07ffffff)
#define RXS_DEL_TRIG0_MASK_SAME_TIME_MASK            ((uint32_t)0x08000000)
#define RXS_DEL_TRIG0_MASK_SRC_MASK                  ((uint32_t)0xf0000000)

/* RXS_DEL_TRIG0_VAL : Register Bits Masks Definitions */
#define RXS_DEL_TRIG0_VAL_EVENTS                     ((uint32_t)0x07ffffff)
#define RXS_DEL_TRIG0_VAL_SAME_TIME                  ((uint32_t)0x08000000)
#define RXS_DEL_TRIG0_VAL_SRC_VAL                    ((uint32_t)0xf0000000)

/* RXS_DEL_TRIG1_MASK : Register Bits Masks Definitions */
#define RXS_DEL_TRIG1_MASK_MASK                      ((uint32_t)0x07ffffff)
#define RXS_DEL_TRIG1_MASK_SAME_TIME_MASK            ((uint32_t)0x08000000)
#define RXS_DEL_TRIG1_MASK_SRC_MASK                  ((uint32_t)0xf0000000)

/* RXS_DEL_TRIG1_VAL : Register Bits Masks Definitions */
#define RXS_DEL_TRIG1_VAL_EVENTS                     ((uint32_t)0x07ffffff)
#define RXS_DEL_TRIG1_VAL_SAME_TIME                  ((uint32_t)0x08000000)
#define RXS_DEL_TRIG1_VAL_SRC_VAL                    ((uint32_t)0xf0000000)

/* RXS_DEL_TRIG_STAT : Register Bits Masks Definitions */
#define RXS_DEL_TRIG_STAT_TRIG_0                     ((uint32_t)0x00000001)
#define RXS_DEL_TRIG_STAT_TRIG_1                     ((uint32_t)0x00000002)
#define RXS_DEL_TRIG_STAT_TRIG                       ((uint32_t)0x00000004)
#define RXS_DEL_TRIG_STAT_FULL                       ((uint32_t)0x00000008)
#define RXS_DEL_TRIG_STAT_INT_A                      ((uint32_t)0x00000010)
#define RXS_DEL_TRIG_STAT_INT_B                      ((uint32_t)0x00000020)
#define RXS_DEL_TRIG_STAT_ALL_VALID                  ((uint32_t)0x00000080)

/* RXS_DEL_WR_TRIG_IDX : Register Bits Masks Definitions */
#define RXS_DEL_WR_TRIG_IDX_WR_IDX                   ((uint32_t)0x000000ff)
#define RXS_DEL_WR_TRIG_IDX_TRIG_IDX                 ((uint32_t)0x00ff0000)

/* RXS_DEL_RD_IDX : Register Bits Masks Definitions */
#define RXS_DEL_RD_IDX_RD_IDX                        ((uint32_t)0x000000ff)

/* RXS_DEL_DATA : Register Bits Masks Definitions */
#define RXS_DEL_DATA_EVENTS                          ((uint32_t)0x07ffffff)
#define RXS_DEL_DATA_SAME_TIME                       ((uint32_t)0x08000000)
#define RXS_DEL_DATA_SOURCE                          ((uint32_t)0xf0000000)

/* RXS_DEL_SRC_STAT0 : Register Bits Masks Definitions */
#define RXS_DEL_SRC_STAT0_INT_ASSERT                 ((uint32_t)0x00ffffff)
#define RXS_DEL_SRC_STAT0_SOFTWARE                   ((uint32_t)0x07000000)

/* RXS_DEL_SRC_STAT1 : Register Bits Masks Definitions */
#define RXS_DEL_SRC_STAT1_INT_DASSERT                ((uint32_t)0x00ffffff)
#define RXS_DEL_SRC_STAT1_SOFTWARE                   ((uint32_t)0x07000000)

/* RXS_DEL_SRC_STAT2 : Register Bits Masks Definitions */
#define RXS_DEL_SRC_STAT2_PW_ASSERT                  ((uint32_t)0x00ffffff)
#define RXS_DEL_SRC_STAT2_SOFTWARE                   ((uint32_t)0x07000000)

/* RXS_DEL_SRC_STAT3 : Register Bits Masks Definitions */
#define RXS_DEL_SRC_STAT3_PW_DASSERT                 ((uint32_t)0x00ffffff)
#define RXS_DEL_SRC_STAT3_SOFTWARE                   ((uint32_t)0x07000000)

/* RXS_DEL_SRC_STAT4 : Register Bits Masks Definitions */
#define RXS_DEL_SRC_STAT4_RESET                      ((uint32_t)0x00ffffff)
#define RXS_DEL_SRC_STAT4_SOFTWARE                   ((uint32_t)0x07000000)

/* RXS_DEL_SRC_STATX : Register Bits Masks Definitions */
#define RXS_DEL_SRC_STATX_SOFTWARE                   ((uint32_t)0x07ffffff)

/* RXS_FAB_CBCOL_BH : Register Bits Masks Definitions */
#define RXS_FAB_CBCOL_BH_BLK_TYPE                    ((uint32_t)0x00000fff)
#define RXS_FAB_CBCOL_BH_BLK_REV                     ((uint32_t)0x0000f000)
#define RXS_FAB_CBCOL_BH_NEXT_BLK_PTR                ((uint32_t)0xffff0000)

/* RXS_FAB_CBCOL_X_CTL : Register Bits Masks Definitions */
#define RXS_FAB_CBCOL_X_CTL_OS_SAT                   ((uint32_t)0x00000100)
#define RXS_FAB_CBCOL_X_CTL_OS_CRED                  ((uint32_t)0x01ff0000)
#define RXS_FAB_CBCOL_X_CTL_OS_ARB_MODE              ((uint32_t)0x10000000)

/* RXS_FAB_CBCOL_X_SAT_CTL : Register Bits Masks Definitions */
#define RXS_FAB_CBCOL_X_SAT_CTL_MIN_SAT              ((uint32_t)0x0000ffff)
#define RXS_FAB_CBCOL_X_SAT_CTL_MAX_SAT              ((uint32_t)0xffff0000)

/* RXS_FAB_CBCOL_X_OS_INP_EN : Register Bits Masks Definitions */
#define RXS_FAB_CBCOL_X_OS_INP_EN_INP_EN             ((uint32_t)0x0000ffff)
#define RXS_FAB_CBCOL_X_OS_INP_EN_UNUSED             ((uint32_t)0x00ff0000)
#define RXS_FAB_CBCOL_X_OS_INP_EN_DPG_EN             ((uint32_t)0x01000000)

/* RXS_FAB_CBCOL_X_ACT : Register Bits Masks Definitions */
#define RXS_FAB_CBCOL_X_ACT_ACTIVE                   ((uint32_t)0x0000ffff)
#define RXS_FAB_CBCOL_X_ACT_UNUSED                   ((uint32_t)0x00ff0000)

/* RXS_FAB_CBCOL_X_ACT_SUMM : Register Bits Masks Definitions */
#define RXS_FAB_CBCOL_X_ACT_SUMM_MB_ACT              ((uint32_t)0x00000001)
#define RXS_FAB_CBCOL_X_ACT_SUMM_COL_ACT             ((uint32_t)0x00000002)

/* RXS_FAB_CBCOL_X_PG_WM : Register Bits Masks Definitions */
#define RXS_FAB_CBCOL_X_PG_WM_PBME_PRIO1_RSVD_PAGES  ((uint32_t)0x0000ff00)
#define RXS_FAB_CBCOL_X_PG_WM_PBME_PRIO2_RSVD_PAGES  ((uint32_t)0x00ff0000)
#define RXS_FAB_CBCOL_X_PG_WM_PBME_PRIO3_RSVD_PAGES  ((uint32_t)0xff000000)

/* RXS_FAB_CBCOL_X_ND_WM_01 : Register Bits Masks Definitions */
#define RXS_FAB_CBCOL_X_ND_WM_01_PBME_PRIO1_RSVD_NODES  ((uint32_t)0x03ff0000)

/* RXS_FAB_CBCOL_X_ND_WM_23 : Register Bits Masks Definitions */
#define RXS_FAB_CBCOL_X_ND_WM_23_PBME_PRIO2_RSVD_NODES  ((uint32_t)0x000003ff)
#define RXS_FAB_CBCOL_X_ND_WM_23_PBME_PRIO3_RSVD_NODES  ((uint32_t)0x03ff0000)

/* RXS_FAB_CBCOL_X_CLK_GATE : Register Bits Masks Definitions */
#define RXS_FAB_CBCOL_X_CLK_GATE_COL_PORT_CGATE      ((uint32_t)0x0000ffff)
#define RXS_FAB_CBCOL_X_CLK_GATE_UNUSED              ((uint32_t)0x00ff0000)

/* RXS_FAB_CBCOL_X_SCRATCH : Register Bits Masks Definitions */
#define RXS_FAB_CBCOL_X_SCRATCH_SCRATCH              ((uint32_t)0xffffffff)

/* RXS_PKT_GEN_BH : Register Bits Masks Definitions */
#define RXS_PKT_GEN_BH_BLK_TYPE                      ((uint32_t)0x00000fff)
#define RXS_PKT_GEN_BH_BLK_REV                       ((uint32_t)0x0000f000)
#define RXS_PKT_GEN_BH_NEXT_BLK_PTR                  ((uint32_t)0xffff0000)

/* RXS_FAB_PGEN_X_ACC : Register Bits Masks Definitions */
#define RXS_FAB_PGEN_X_ACC_REG_ACC                   ((uint32_t)0x80000000)

/* RXS_FAB_PGEN_X_STAT : Register Bits Masks Definitions */
#define RXS_FAB_PGEN_X_STAT_EVNT                     ((uint32_t)0x80000000)

/* RXS_FAB_PGEN_X_INT_EN : Register Bits Masks Definitions */
#define RXS_FAB_PGEN_X_INT_EN_INT_EN                 ((uint32_t)0x80000000)

/* RXS_FAB_PGEN_X_PW_EN : Register Bits Masks Definitions */
#define RXS_FAB_PGEN_X_PW_EN_PW_EN                   ((uint32_t)0x80000000)

/* RXS_FAB_PGEN_X_GEN : Register Bits Masks Definitions */
#define RXS_FAB_PGEN_X_GEN_GEN                       ((uint32_t)0x80000000)

/* RXS_FAB_PGEN_X_CTL : Register Bits Masks Definitions */
#define RXS_FAB_PGEN_X_CTL_REPEAT                    ((uint32_t)0x0000ffff)
#define RXS_FAB_PGEN_X_CTL_PACE                      ((uint32_t)0x0fff0000)
#define RXS_FAB_PGEN_X_CTL_DONE                      ((uint32_t)0x20000000)
#define RXS_FAB_PGEN_X_CTL_START                     ((uint32_t)0x40000000)
#define RXS_FAB_PGEN_X_CTL_PG_INIT                   ((uint32_t)0x80000000)

/* RXS_FAB_PGEN_X_DATA_CTL : Register Bits Masks Definitions */
#define RXS_FAB_PGEN_X_DATA_CTL_ADDR                 ((uint32_t)0x000001f0)
#define RXS_FAB_PGEN_X_DATA_CTL_PKT_TTL              ((uint32_t)0x00070000)
#define RXS_FAB_PGEN_X_DATA_CTL_PORT                 ((uint32_t)0x00100000)
#define RXS_FAB_PGEN_X_DATA_CTL_LEN                  ((uint32_t)0x07000000)
#define RXS_FAB_PGEN_X_DATA_CTL_PRIO                 ((uint32_t)0x70000000)

/* RXS_FAB_PGEN_X_DATA_Y : Register Bits Masks Definitions */
#define RXS_FAB_PGEN_X_DATA_Y_DATA                   ((uint32_t)0xffffffff)

/* RXS_SPX_L0_G0_ENTRYY_CSR : Register Bits Masks Definitions */
#define RXS_SPX_L0_G0_ENTRYY_CSR_ROUTING_VALUE       ((uint32_t)0x000003ff)
#define RXS_SPX_L0_G0_ENTRYY_CSR_CAPTURE             ((uint32_t)0x80000000)

/* RXS_SPX_L1_GY_ENTRYZ_CSR : Register Bits Masks Definitions */
#define RXS_SPX_L1_GY_ENTRYZ_CSR_ROUTING_VALUE       ((uint32_t)0x000003ff)
#define RXS_SPX_L1_GY_ENTRYZ_CSR_CAPTURE             ((uint32_t)0x80000000)

/* RXS_SPX_L2_GY_ENTRYZ_CSR : Register Bits Masks Definitions */
#define RXS_SPX_L2_GY_ENTRYZ_CSR_ROUTING_VALUE       ((uint32_t)0x000003ff)
#define RXS_SPX_L2_GY_ENTRYZ_CSR_CAPTURE             ((uint32_t)0x80000000)

/* RXS_SPX_MC_Y_S_CSR : Register Bits Masks Definitions */
#define RXS_SPX_MC_Y_S_CSR_SET                       ((uint32_t)0x0000ffff)
#define RXS_SPX_MC_Y_S_CSR_UNUSED                    ((uint32_t)0x00ff0000)

/* RXS_SPX_MC_Y_C_CSR : Register Bits Masks Definitions */
#define RXS_SPX_MC_Y_C_CSR_CLR                       ((uint32_t)0x0000ffff)
#define RXS_SPX_MC_Y_C_CSR_UNUSED                    ((uint32_t)0x00ff0000)

/* RXS_SPX_MC_Y_S_CSR : Register Bits Masks Definitions */
#define RXS2448_RIO_SPX_MC_Y_S_CSR_SET                   ((uint32_t)0x00FFFFFF)
#define RXS1632_RIO_SPX_MC_Y_S_CSR_SET                   ((uint32_t)0x0000FFFF)

/* RXS_BC_MC_X_S_CSR : Register Bits Masks Definitions */
#define RXS2448_RIO_BC_MC_X_S_CSR_SET                    ((uint32_t)0x00FFFFFF)
#define RXS1632_RIO_BC_MC_X_S_CSR_SET                    ((uint32_t)0x0000FFFF)


/***************************************************************/
/* SerDes Register Address Offset Definitions                  */
/***************************************************************/

#define RXS_SERDES_BH                                ((uint32_t)0x00098000)
#define RXS_AEC_X_STAT_CSR(X)            ((uint32_t)(0x98004 + 0x3900*(X)))
#define RXS_AEC_X_INT_EN_CSR(X)          ((uint32_t)(0x98008 + 0x3900*(X)))
#define RXS_AEC_X_EVENT_GEN_CSR(X)       ((uint32_t)(0x9800c + 0x3900*(X)))
#define RXS_AEC_X_PHY_IND_WR_CSR(X)      ((uint32_t)(0x98010 + 0x3900*(X)))
#define RXS_AEC_X_PHY_IND_RD_CSR(X)      ((uint32_t)(0x98014 + 0x3900*(X)))
#define RXS_AEC_X_SCRATCH(X)             ((uint32_t)(0x98018 + 0x3900*(X)))
#define RXS_AEC_X_RESET_CTRL_CSR(X)      ((uint32_t)(0x98024 + 0x3900*(X)))
#define RXS_AEC_X_PHY_PLL_CTL_CSR(X)     ((uint32_t)(0x98028 + 0x3900*(X)))
#define RXS_AEC_X_LANE_Y_TAP_SPACE_LIMIT1_CSR(X,Y)  ((uint32_t)(0x98100 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_TAP_CM1_LIMIT1_CSR(X,Y)    ((uint32_t)(0x98104 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_TAP_C0_LIMIT1_CSR(X,Y)     ((uint32_t)(0x98108 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_TAP_CP1_LIMIT1_CSR(X,Y)    ((uint32_t)(0x9810c + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_TAP_SPACE_LIMIT2_CSR(X,Y)  ((uint32_t)(0x98110 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_TAP_CM1_LIMIT2_CSR(X,Y)    ((uint32_t)(0x98114 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_TAP_C0_LIMIT2_CSR(X,Y)     ((uint32_t)(0x98118 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_TAP_CP1_LIMIT2_CSR(X,Y)    ((uint32_t)(0x9811c + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_TAP_CM1_VALUE_CSR(X,Y)     ((uint32_t)(0x98120 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_TAP_C0_VALUE_CSR(X,Y)      ((uint32_t)(0x98124 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_TAP_CP1_VALUE_CSR(X,Y)     ((uint32_t)(0x98128 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR(X,Y)         ((uint32_t)(0x9812c + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR(X,Y)         ((uint32_t)(0x98134 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_STAT_CSR(X,Y)              ((uint32_t)(0x98140 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_INT_EN_CSR(X,Y)            ((uint32_t)(0x98144 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_EVENT_GEN_CSR(X,Y)         ((uint32_t)(0x98148 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR(X,Y)       ((uint32_t)(0x98150 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG2_CSR(X,Y)       ((uint32_t)(0x98154 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_RX_LOS_TMR_CSR(X,Y)        ((uint32_t)(0x98158 + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_LANE_Y_ADAPT_STAT_CTRL_CSR(X,Y)   ((uint32_t)(0x9815c + 0x3900*(X) + 0x100*(Y)))
#define RXS_AEC_X_MPLLA_CSR(X)                      ((uint32_t)(0x98538 + 0x3900*(X)))
#define RXS_AEC_X_MPLLB_CSR(X)                      ((uint32_t)(0x98544 + 0x3900*(X)))
#define RXS_AEC_X_LANE_Y_TXFIFO_BYPASS_CSR(X,Y)     ((uint32_t)(0x9899c + 0x3900*(X) + 0x400*(Y)))

/***************************************************************/
/* SerDes Register Bit Masks and Reset Values Definitions      */
/***************************************************************/

/* RXS_SERDES_BH : Register Bits Masks Definitions */
#define RXS_SERDES_BH_BLK_TYPE                       ((uint32_t)0x00000fff)
#define RXS_SERDES_BH_BLK_REV                        ((uint32_t)0x0000f000)
#define RXS_SERDES_BH_NEXT_BLK_PTR                   ((uint32_t)0xffff0000)

/* RXS_AEC_X_STAT_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_STAT_CSR_PHY_MEM_UNCOR             ((uint32_t)0x00400000)
#define RXS_AEC_X_STAT_CSR_PHY_MEM_COR               ((uint32_t)0x00800000)
#define RXS_AEC_X_STAT_CSR_AEC_STAT1                 ((uint32_t)0x20000000)
#define RXS_AEC_X_STAT_CSR_AEC_STAT0                 ((uint32_t)0x40000000)
#define RXS_AEC_X_STAT_CSR_TXEQ_PROT_ERR             ((uint32_t)0x80000000)

/* RXS_AEC_X_INT_EN_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_INT_EN_CSR_PHY_MEM_UNCOR           ((uint32_t)0x00400000)
#define RXS_AEC_X_INT_EN_CSR_PHY_MEM_COR             ((uint32_t)0x00800000)
#define RXS_AEC_X_INT_EN_CSR_AEC_CFG1                ((uint32_t)0x20000000)
#define RXS_AEC_X_INT_EN_CSR_AEC_CFG0                ((uint32_t)0x40000000)
#define RXS_AEC_X_INT_EN_CSR_TXEQ_PROT_ERR           ((uint32_t)0x80000000)

/* RXS_AEC_X_EVENT_GEN_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_EVENT_GEN_CSR_PHY_MEM_UNCOR        ((uint32_t)0x00400000)
#define RXS_AEC_X_EVENT_GEN_CSR_PHY_MEM_COR          ((uint32_t)0x00800000)

/* RXS_AEC_X_PHY_IND_WR_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_PHY_IND_WR_CSR_WR_DATA             ((uint32_t)0x0000ffff)
#define RXS_AEC_X_PHY_IND_WR_CSR_WR_ADDR             ((uint32_t)0xffff0000)

/* RXS_AEC_X_PHY_IND_RD_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_PHY_IND_RD_CSR_RD_DATA             ((uint32_t)0x0000ffff)
#define RXS_AEC_X_PHY_IND_RD_CSR_RD_ADDR             ((uint32_t)0xffff0000)

/* RXS_AEC_X_SCRATCH : Register Bits Masks Definitions */
#define RXS_AEC_X_SCRATCH_SCRATCH                    ((uint32_t)0xffffffff)

/* RXS_AEC_X_RESET_CTRL_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_RESET_CTRL_CSR_AEC_STAT0           ((uint32_t)0x00000080)
#define RXS_AEC_X_RESET_CTRL_CSR_AEC_CFG1            ((uint32_t)0x00008000)
#define RXS_AEC_X_RESET_CTRL_CSR_AEC_CFG0            ((uint32_t)0x00c00000)
#define RXS_AEC_X_RESET_CTRL_CSR_QUAD_DEBUG_0        ((uint32_t)0x20000000)
#define RXS_AEC_X_RESET_CTRL_CSR_PHY_REG_UPD         ((uint32_t)0x40000000)
#define RXS_AEC_X_RESET_CTRL_CSR_PHY_PLL_CFG_UPD     ((uint32_t)0x80000000)

/* RXS_AEC_X_PHY_PLL_CTL_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_PHY_PLL_CTL_CSR_TX_VBOOST_LVL      ((uint32_t)0x00000007)
#define RXS_AEC_X_PHY_PLL_CTL_CSR_AEC_CFG1           ((uint32_t)0x40000000)
#define RXS_AEC_X_PHY_PLL_CTL_CSR_AEC_CFG0           ((uint32_t)0x80000000)

/* RXS_AEC_X_LANE_Y_TAP_SPACE_LIMIT1_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TAP_SPACE_LIMIT1_CSR_LOWER_BND1  ((uint32_t)0x000000ff)
#define RXS_AEC_X_LANE_Y_TAP_SPACE_LIMIT1_CSR_UPPER_BND0  ((uint32_t)0x0000ff00)
#define RXS_AEC_X_LANE_Y_TAP_SPACE_LIMIT1_CSR_LOWER_BND0  ((uint32_t)0x00ff0000)

/* RXS_AEC_X_LANE_Y_TAP_CM1_LIMIT1_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TAP_CM1_LIMIT1_CSR_CM1_INITIALIZE ((uint32_t)0x0000003f)
#define RXS_AEC_X_LANE_Y_TAP_CM1_LIMIT1_CSR_CM1_PRESET     ((uint32_t)0x00003f00)
#define RXS_AEC_X_LANE_Y_TAP_CM1_LIMIT1_CSR_CM1_MIN        ((uint32_t)0x003f0000)
#define RXS_AEC_X_LANE_Y_TAP_CM1_LIMIT1_CSR_CM1_MAX        ((uint32_t)0x3f000000)

/* RXS_AEC_X_LANE_Y_TAP_C0_LIMIT1_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TAP_C0_LIMIT1_CSR_C0_INITIALIZE ((uint32_t)0x0000003f)
#define RXS_AEC_X_LANE_Y_TAP_C0_LIMIT1_CSR_C0_PRESET     ((uint32_t)0x00003f00)
#define RXS_AEC_X_LANE_Y_TAP_C0_LIMIT1_CSR_C0_MIN        ((uint32_t)0x003f0000)
#define RXS_AEC_X_LANE_Y_TAP_C0_LIMIT1_CSR_C0_MAX        ((uint32_t)0x3f000000)

/* RXS_AEC_X_LANE_Y_TAP_CP1_LIMIT1_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TAP_CP1_LIMIT1_CSR_CP1_INITIALIZE ((uint32_t)0x0000003f)
#define RXS_AEC_X_LANE_Y_TAP_CP1_LIMIT1_CSR_CP1_PRESET     ((uint32_t)0x00003f00)
#define RXS_AEC_X_LANE_Y_TAP_CP1_LIMIT1_CSR_CP1_MIN        ((uint32_t)0x003f0000)
#define RXS_AEC_X_LANE_Y_TAP_CP1_LIMIT1_CSR_CP1_MAX        ((uint32_t)0x3f000000)

/* RXS_AEC_X_LANE_Y_TAP_SPACE_LIMIT2_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TAP_SPACE_LIMIT2_CSR_LOWER_BND1 ((uint32_t)0x000000ff)
#define RXS_AEC_X_LANE_Y_TAP_SPACE_LIMIT2_CSR_UPPER_BND0 ((uint32_t)0x0000ff00)
#define RXS_AEC_X_LANE_Y_TAP_SPACE_LIMIT2_CSR_LOWER_BND0 ((uint32_t)0x00ff0000)

/* RXS_AEC_X_LANE_Y_TAP_CM1_LIMIT2_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TAP_CM1_LIMIT2_CSR_CM1_INITIALIZE ((uint32_t)0x0000003f)
#define RXS_AEC_X_LANE_Y_TAP_CM1_LIMIT2_CSR_CM1_PRESET     ((uint32_t)0x00003f00)
#define RXS_AEC_X_LANE_Y_TAP_CM1_LIMIT2_CSR_CM1_MIN        ((uint32_t)0x003f0000)
#define RXS_AEC_X_LANE_Y_TAP_CM1_LIMIT2_CSR_CM1_MAX        ((uint32_t)0x3f000000)

/* RXS_AEC_X_LANE_Y_TAP_C0_LIMIT2_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TAP_C0_LIMIT2_CSR_C0_INITIALIZE ((uint32_t)0x0000003f)
#define RXS_AEC_X_LANE_Y_TAP_C0_LIMIT2_CSR_C0_PRESET     ((uint32_t)0x00003f00)
#define RXS_AEC_X_LANE_Y_TAP_C0_LIMIT2_CSR_C0_MIN        ((uint32_t)0x003f0000)
#define RXS_AEC_X_LANE_Y_TAP_C0_LIMIT2_CSR_C0_MAX        ((uint32_t)0x3f000000)

/* RXS_AEC_X_LANE_Y_TAP_CP1_LIMIT2_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TAP_CP1_LIMIT2_CSR_CP1_INITIALIZE ((uint32_t)0x0000003f)
#define RXS_AEC_X_LANE_Y_TAP_CP1_LIMIT2_CSR_CP1_PRESET     ((uint32_t)0x00003f00)
#define RXS_AEC_X_LANE_Y_TAP_CP1_LIMIT2_CSR_CP1_MIN        ((uint32_t)0x003f0000)
#define RXS_AEC_X_LANE_Y_TAP_CP1_LIMIT2_CSR_CP1_MAX        ((uint32_t)0x3f000000)

/* RXS_AEC_X_LANE_Y_TAP_CM1_VALUE_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TAP_CM1_VALUE_CSR_CM1_CURRENT       (0x00003f00)
#define RXS_AEC_X_LANE_Y_TAP_CM1_VALUE_CSR_USE_MANUAL_TAP    (0x00010000)
#define RXS_AEC_X_LANE_Y_TAP_CM1_VALUE_CSR_CM1_MANUAL        (0x3f000000)

/* RXS_AEC_X_LANE_Y_TAP_C0_VALUE_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TAP_C0_VALUE_CSR_C0_CURRENT         (0x00003fc0)
#define RXS_AEC_X_LANE_Y_TAP_C0_VALUE_CSR_USE_MANUAL_TAP     (0x00010000)
#define RXS_AEC_X_LANE_Y_TAP_C0_VALUE_CSR_C0_MANUAL          (0x3fc00000)

/* RXS_AEC_X_LANE_Y_TAP_CP1_VALUE_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TAP_CP1_VALUE_CSR_CP1_CURRENT    ((uint32_t)0x00003f00)
#define RXS_AEC_X_LANE_Y_TAP_CP1_VALUE_CSR_USE_MANUAL_TAP ((uint32_t)0x00010000)
#define RXS_AEC_X_LANE_Y_TAP_CP1_VALUE_CSR_CP1_MANUAL     ((uint32_t)0x3f000000)

/* RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_RST_RX_BEFORE_ADAPT ((uint32_t)0x00000001)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_RX_FINE_TUNE_MODE   ((uint32_t)0x00000006)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_TXEQ_MODE           ((uint32_t)0x00000018)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_AEC_CFG4            ((uint32_t)0x000003e0)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_AEC_CFG3            ((uint32_t)0x00000400)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_AEC_CFG2            ((uint32_t)0x00001000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_AEC_CFG1            ((uint32_t)0x00002000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_RX_LAST_ADAPT_EN    ((uint32_t)0x00004000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_AEC_CFG0            ((uint32_t)0x00008000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_LOS_INFERENCE       ((uint32_t)0x00030000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_MODIFY_EMPHASIS     ((uint32_t)0x000c0000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_CP1_INC_STEP        ((uint32_t)0x00300000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_CP1_DEC_STEP        ((uint32_t)0x00c00000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_C0_INC_STEP         ((uint32_t)0x03000000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_C0_DEC_STEP         ((uint32_t)0x0c000000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_CM1_INC_STEP        ((uint32_t)0x30000000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL1_CSR_CM1_DEC_STEP        ((uint32_t)0xc0000000)

/* RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR_AEC_CFG4         ((uint32_t)0x000000ff)
#define RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR_AEC_CFG3         ((uint32_t)0x00000100)
#define RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR_LOCAL_TXEQ_INIT  ((uint32_t)0x00000600)
#define RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR_AEC_CFG2         ((uint32_t)0x0007f800)
#define RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR_RX_ADAPT_DFE_EN  ((uint32_t)0x00180000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR_AEC_CFG1         ((uint32_t)0x00600000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR_RX_ADAPT_AFE_EN  ((uint32_t)0x01800000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR_RX_OFFCAN_CONT   ((uint32_t)0x02000000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR_RX_CONT_ADAPT    ((uint32_t)0x04000000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR_AEC_CFG0         ((uint32_t)0x08000000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR_TXEQ_VBOOST      ((uint32_t)0x30000000)
#define RXS_AEC_X_LANE_Y_EMPH_CTL3_CSR_TX_VBOOST_EN     ((uint32_t)0xc0000000)

/* RXS_AEC_X_LANE_Y_STAT_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_STAT_CSR_AEC_STAT3          ((uint32_t)0x00800000)
#define RXS_AEC_X_LANE_Y_STAT_CSR_AEC_STAT2          ((uint32_t)0x10000000)
#define RXS_AEC_X_LANE_Y_STAT_CSR_AEC_STAT1          ((uint32_t)0x20000000)
#define RXS_AEC_X_LANE_Y_STAT_CSR_TXEQ_CMD_SEQ_ERR   ((uint32_t)0x40000000)
#define RXS_AEC_X_LANE_Y_STAT_CSR_AEC_STAT0          ((uint32_t)0x80000000)

/* RXS_AEC_X_LANE_Y_INT_EN_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_INT_EN_CSR_AEC_CFG3         ((uint32_t)0x00800000)
#define RXS_AEC_X_LANE_Y_INT_EN_CSR_AEC_CFG2         ((uint32_t)0x10000000)
#define RXS_AEC_X_LANE_Y_INT_EN_CSR_AEC_CFG1         ((uint32_t)0x20000000)
#define RXS_AEC_X_LANE_Y_INT_EN_CSR_TXEQ_CMD_SEQ_ERR ((uint32_t)0x40000000)
#define RXS_AEC_X_LANE_Y_INT_EN_CSR_AEC_CFG0         ((uint32_t)0x80000000)

/* RXS_AEC_X_LANE_Y_EVENT_GEN_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_EVENT_GEN_CSR_AEC_CFG3          ((uint32_t)0x00800000)
#define RXS_AEC_X_LANE_Y_EVENT_GEN_CSR_AEC_CFG2          ((uint32_t)0x10000000)
#define RXS_AEC_X_LANE_Y_EVENT_GEN_CSR_AEC_CFG1          ((uint32_t)0x20000000)
#define RXS_AEC_X_LANE_Y_EVENT_GEN_CSR_TXEQ_CMD_SEQ_ERR  ((uint32_t)0x40000000)
#define RXS_AEC_X_LANE_Y_EVENT_GEN_CSR_AEC_CFG0          ((uint32_t)0x80000000)

/* RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR_AEC_CFG8           ((uint32_t)0x0000003f)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR_PHY_TX2RX_SLPB_EN  ((uint32_t)0x00000040)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR_PHY_RX2TX_PLPB_EN  ((uint32_t)0x00000080)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR_AEC_CFG7           ((uint32_t)0x00003f00)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR_AEC_CFG6           ((uint32_t)0x00008000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR_AEC_CFG5           ((uint32_t)0x001f0000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR_AEC_CFG4           ((uint32_t)0x00200000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR_AEC_CFG3           ((uint32_t)0x00c00000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR_AEC_CFG2           ((uint32_t)0x03000000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR_AEC_CFG1           ((uint32_t)0x0c000000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR_AEC_STAT0          ((uint32_t)0x70000000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG1_CSR_AEC_CFG0           ((uint32_t)0x80000000)

/* RXS_AEC_X_LANE_Y_TXEQ_DEBUG2_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG2_CSR_ADAPT_FOM         ((uint32_t)0x000000ff)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG2_CSR_TXPOST_DIR        ((uint32_t)0x00000c00)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG2_CSR_AEC_CFG2          ((uint32_t)0x00003000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG2_CSR_TXPRE_DIR         ((uint32_t)0x0000c000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG2_CSR_AEC_CFG1          ((uint32_t)0x00010000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG2_CSR_AEC_STAT0         ((uint32_t)0x00400000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG2_CSR_ADPT_ACK          ((uint32_t)0x00800000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG2_CSR_AEC_CFG0          ((uint32_t)0x03000000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG2_CSR_ENABLE_EVENT_LOG  ((uint32_t)0x30000000)
#define RXS_AEC_X_LANE_Y_TXEQ_DEBUG2_CSR_REQ_ADPT          ((uint32_t)0xc0000000)

/* RXS_AEC_X_LANE_Y_RX_LOS_TMR_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_RX_LOS_TMR_CSR_REQ_ADAPT_AFTER_RXRST  ((uint32_t)0x00000001)
#define RXS_AEC_X_LANE_Y_RX_LOS_TMR_CSR_AEC_CFG0               ((uint32_t)0x000000c0)
#define RXS_AEC_X_LANE_Y_RX_LOS_TMR_CSR_LOS_TMR_DME            ((uint32_t)0x0000ff00)
#define RXS_AEC_X_LANE_Y_RX_LOS_TMR_CSR_LOS_TMR_IDLE3          ((uint32_t)0x00ff0000)
#define RXS_AEC_X_LANE_Y_RX_LOS_TMR_CSR_LOS_TMR_IDLE1_2        ((uint32_t)0xff000000)

/* RXS_AEC_X_LANE_Y_ADAPT_STAT_CTRL_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_ADAPT_STAT_CTRL_CSR_LANE_DEBUG_2  ((uint32_t)0x00002000)
#define RXS_AEC_X_LANE_Y_ADAPT_STAT_CTRL_CSR_LANE_DEBUG_1  ((uint32_t)0x00004000)
#define RXS_AEC_X_LANE_Y_ADAPT_STAT_CTRL_CSR_LANE_DEBUG_0  ((uint32_t)(0x00008000)
#define RXS_AEC_X_LANE_Y_ADAPT_STAT_CTRL_CSR_AEC_CFG3      ((uint32_t)0x00070000)
#define RXS_AEC_X_LANE_Y_ADAPT_STAT_CTRL_CSR_AEC_CFG2      ((uint32_t)0x00080000)
#define RXS_AEC_X_LANE_Y_ADAPT_STAT_CTRL_CSR_AEC_CFG1      ((uint32_t)0x00700000)
#define RXS_AEC_X_LANE_Y_ADAPT_STAT_CTRL_CSR_AEC_CFG0      ((uint32_t)0x00800000)
#define RXS_AEC_X_LANE_Y_ADAPT_STAT_CTRL_CSR_AEC_STAT0     ((uint32_t)0x80000000)

/* RXS_AEC_X_MPLLA_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_MPLLA_CSR_MPLLA_MULTIPLIER         ((uint32_t)0x00001fe0)

/* RXS_AEC_X_MPLLB_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_MPLLB_CSR_MPLLB_MULTIPLIER         ((uint32_t)0x00000ff0)

/* RXS_AEC_X_LANE_Y_TXFIFO_BYPASS_CSR : Register Bits Masks Definitions */
#define RXS_AEC_X_LANE_Y_TXFIFO_BYPASS_CSR_PHY_CFG0  ((uint32_t)0x0000ffff)


/*****************************************************/
/* I2C Register Address Offset Definitions           */
/*****************************************************/

#define I2C_DEVID                                        ((uint32_t)0x000c9100)
#define I2C_RESET                                        ((uint32_t)0x000c9104)
#define I2C_MST_CFG                                      ((uint32_t)0x000c9108)
#define I2C_MST_CNTRL                                    ((uint32_t)0x000c910c)
#define I2C_MST_RDATA                                    ((uint32_t)0x000c9110)
#define I2C_MST_TDATA                                    ((uint32_t)0x000c9114)
#define I2C_ACC_STAT                                     ((uint32_t)0x000c9118)
#define I2C_INT_STAT                                     ((uint32_t)0x000c911c)
#define I2C_INT_ENABLE                                   ((uint32_t)0x000c9120)
#define I2C_INT_SET                                      ((uint32_t)0x000c9124)
#define I2C_SLV_CFG                                      ((uint32_t)0x000c912c)
#define I2C_BOOT_CNTRL                                   ((uint32_t)0x000c9140)
#define EXI2C_REG_WADDR                                  ((uint32_t)0x000c9200)
#define EXI2C_REG_WDATA                                  ((uint32_t)0x000c9204)
#define EXI2C_REG_RADDR                                  ((uint32_t)0x000c9210)
#define EXI2C_REG_RDATA                                  ((uint32_t)0x000c9214)
#define EXI2C_ACC_STAT                                   ((uint32_t)0x000c9220)
#define EXI2C_ACC_CNTRL                                  ((uint32_t)0x000c9224)
#define EXI2C_STAT                                       ((uint32_t)0x000c9280)
#define EXI2C_STAT_ENABLE                                ((uint32_t)0x000c9284)
#define EXI2C_MBOX_OUT                                   ((uint32_t)0x000c9290)
#define EXI2C_MBOX_IN                                    ((uint32_t)0x000c9294)
#define I2C_EVENT                                        ((uint32_t)0x000c9300)
#define I2C_SNAP_EVENT                                   ((uint32_t)0x000c9304)
#define I2C_NEW_EVENT                                    ((uint32_t)0x000c9308)
#define I2C_EVENT_ENB                                    ((uint32_t)0x000c930c)
#define I2C_DIVIDER                                      ((uint32_t)0x000c9320)
#define I2C_FILTER_SCL_CFG                               ((uint32_t)0x000c9328)
#define I2C_FILTER_SDA_CFG                               ((uint32_t)0x000c932c)
#define I2C_START_SETUP_HOLD                             ((uint32_t)0x000c9340)
#define I2C_STOP_IDLE                                    ((uint32_t)0x000c9344)
#define I2C_SDA_SETUP_HOLD                               ((uint32_t)0x000c9348)
#define I2C_SCL_PERIOD                                   ((uint32_t)0x000c934c)
#define I2C_SCL_MIN_PERIOD                               ((uint32_t)0x000c9350)
#define I2C_SCL_ARB_TIMEOUT                              ((uint32_t)0x000c9354)
#define I2C_BYTE_TRAN_TIMEOUT                            ((uint32_t)0x000c9358)
#define I2C_BOOT_DIAG_TIMER                              ((uint32_t)0x000c935c)
#define I2C_BOOT_DIAG_PROGRESS                           ((uint32_t)0x000c93b8)
#define I2C_BOOT_DIAG_CFG                                ((uint32_t)0x000c93bc)

/************************************************************/
/* I2C Register Bit Masks and Reset Values Definitions      */
/************************************************************/

/* I2C_DEVID : Register Bits Masks Definitions */
#define I2C_DEVID_REV                                    ((uint32_t)0x0000000f)

/* I2C_RESET : Register Bits Masks Definitions */
#define I2C_RESET_SRESET                                 ((uint32_t)0x80000000)

/* I2C_MST_CFG : Register Bits Masks Definitions */
#define I2C_MST_CFG_DEV_ADDR                             ((uint32_t)0x0000007f)
#define I2C_MST_CFG_PA_SIZE                              ((uint32_t)0x00030000)
#define I2C_MST_CFG_DORDER                               ((uint32_t)0x00800000)

/* I2C_MST_CNTRL : Register Bits Masks Definitions */
#define I2C_MST_CNTRL_PADDR                              ((uint32_t)0x0000ffff)
#define I2C_MST_CNTRL_SIZE                               ((uint32_t)0x07000000)
#define I2C_MST_CNTRL_WRITE                              ((uint32_t)0x40000000)
#define I2C_MST_CNTRL_START                              ((uint32_t)0x80000000)

/* I2C_MST_RDATA : Register Bits Masks Definitions */
#define I2C_MST_RDATA_RBYTE0                             ((uint32_t)0x000000ff)
#define I2C_MST_RDATA_RBYTE1                             ((uint32_t)0x0000ff00)
#define I2C_MST_RDATA_RBYTE2                             ((uint32_t)0x00ff0000)
#define I2C_MST_RDATA_RBYTE3                             ((uint32_t)0xff000000)

/* I2C_MST_TDATA : Register Bits Masks Definitions */
#define I2C_MST_TDATA_TBYTE0                             ((uint32_t)0x000000ff)
#define I2C_MST_TDATA_TBYTE1                             ((uint32_t)0x0000ff00)
#define I2C_MST_TDATA_TBYTE2                             ((uint32_t)0x00ff0000)
#define I2C_MST_TDATA_TBYTE3                             ((uint32_t)0xff000000)

/* I2C_ACC_STAT : Register Bits Masks Definitions */
#define I2C_ACC_STAT_MST_NBYTES                          ((uint32_t)0x0000000f)
#define I2C_ACC_STAT_MST_AN                              ((uint32_t)0x00000100)
#define I2C_ACC_STAT_MST_PHASE                           ((uint32_t)0x00000e00)
#define I2C_ACC_STAT_MST_ACTIVE                          ((uint32_t)0x00008000)
#define I2C_ACC_STAT_SLV_PA                              ((uint32_t)0x00ff0000)
#define I2C_ACC_STAT_SLV_AN                              ((uint32_t)0x01000000)
#define I2C_ACC_STAT_SLV_PHASE                           ((uint32_t)0x06000000)
#define I2C_ACC_STAT_SLV_WAIT                            ((uint32_t)0x08000000)
#define I2C_ACC_STAT_BUS_ACTIVE                          ((uint32_t)0x40000000)
#define I2C_ACC_STAT_SLV_ACTIVE                          ((uint32_t)0x80000000)

/* I2C_INT_STAT : Register Bits Masks Definitions */
#define I2C_INT_STAT_MA_OK                               ((uint32_t)0x00000001)
#define I2C_INT_STAT_MA_ATMO                             ((uint32_t)0x00000002)
#define I2C_INT_STAT_MA_NACK                             ((uint32_t)0x00000004)
#define I2C_INT_STAT_MA_TMO                              ((uint32_t)0x00000008)
#define I2C_INT_STAT_MA_COL                              ((uint32_t)0x00000010)
#define I2C_INT_STAT_MA_DIAG                             ((uint32_t)0x00000080)
#define I2C_INT_STAT_SA_OK                               ((uint32_t)0x00000100)
#define I2C_INT_STAT_SA_READ                             ((uint32_t)0x00000200)
#define I2C_INT_STAT_SA_WRITE                            ((uint32_t)0x00000400)
#define I2C_INT_STAT_SA_FAIL                             ((uint32_t)0x00000800)
#define I2C_INT_STAT_BL_OK                               ((uint32_t)0x00010000)
#define I2C_INT_STAT_BL_FAIL                             ((uint32_t)0x00020000)
#define I2C_INT_STAT_IMB_FULL                            ((uint32_t)0x01000000)
#define I2C_INT_STAT_OMB_EMPTY                           ((uint32_t)0x02000000)

/* I2C_INT_ENABLE : Register Bits Masks Definitions */
#define I2C_INT_ENABLE_MA_OK                             ((uint32_t)0x00000001)
#define I2C_INT_ENABLE_MA_ATMO                           ((uint32_t)0x00000002)
#define I2C_INT_ENABLE_MA_NACK                           ((uint32_t)0x00000004)
#define I2C_INT_ENABLE_MA_TMO                            ((uint32_t)0x00000008)
#define I2C_INT_ENABLE_MA_COL                            ((uint32_t)0x00000010)
#define I2C_INT_ENABLE_MA_DIAG                           ((uint32_t)0x00000080)
#define I2C_INT_ENABLE_SA_OK                             ((uint32_t)0x00000100)
#define I2C_INT_ENABLE_SA_READ                           ((uint32_t)0x00000200)
#define I2C_INT_ENABLE_SA_WRITE                          ((uint32_t)0x00000400)
#define I2C_INT_ENABLE_SA_FAIL                           ((uint32_t)0x00000800)
#define I2C_INT_ENABLE_BL_OK                             ((uint32_t)0x00010000)
#define I2C_INT_ENABLE_BL_FAIL                           ((uint32_t)0x00020000)
#define I2C_INT_ENABLE_IMB_FULL                          ((uint32_t)0x01000000)
#define I2C_INT_ENABLE_OMB_EMPTY                         ((uint32_t)0x02000000)

/* I2C_INT_SET : Register Bits Masks Definitions */
#define I2C_INT_SET_MA_OK                                ((uint32_t)0x00000001)
#define I2C_INT_SET_MA_ATMO                              ((uint32_t)0x00000002)
#define I2C_INT_SET_MA_NACK                              ((uint32_t)0x00000004)
#define I2C_INT_SET_MA_TMO                               ((uint32_t)0x00000008)
#define I2C_INT_SET_MA_COL                               ((uint32_t)0x00000010)
#define I2C_INT_SET_MA_DIAG                              ((uint32_t)0x00000080)
#define I2C_INT_SET_SA_OK                                ((uint32_t)0x00000100)
#define I2C_INT_SET_SA_READ                              ((uint32_t)0x00000200)
#define I2C_INT_SET_SA_WRITE                             ((uint32_t)0x00000400)
#define I2C_INT_SET_SA_FAIL                              ((uint32_t)0x00000800)
#define I2C_INT_SET_BL_OK                                ((uint32_t)0x00010000)
#define I2C_INT_SET_BL_FAIL                              ((uint32_t)0x00020000)
#define I2C_INT_SET_IMB_FULL                             ((uint32_t)0x01000000)
#define I2C_INT_SET_OMB_EMPTY                            ((uint32_t)0x02000000)

/* I2C_SLV_CFG : Register Bits Masks Definitions */
#define I2C_SLV_CFG_SLV_ADDR                             ((uint32_t)0x0000007f)
#define I2C_SLV_CFG_SLV_UNLK                             ((uint32_t)0x01000000)
#define I2C_SLV_CFG_SLV_EN                               ((uint32_t)0x10000000)
#define I2C_SLV_CFG_ALRT_EN                              ((uint32_t)0x20000000)
#define I2C_SLV_CFG_WR_EN                                ((uint32_t)0x40000000)
#define I2C_SLV_CFG_RD_EN                                ((uint32_t)0x80000000)

/* I2C_BOOT_CNTRL : Register Bits Masks Definitions */
#define I2C_BOOT_CNTRL_PADDR                             ((uint32_t)0x00001fff)
#define I2C_BOOT_CNTRL_PAGE_MODE                         ((uint32_t)0x0000e000)
#define I2C_BOOT_CNTRL_BOOT_ADDR                         ((uint32_t)0x007f0000)
#define I2C_BOOT_CNTRL_BUNLK                             ((uint32_t)0x10000000)
#define I2C_BOOT_CNTRL_BINC                              ((uint32_t)0x20000000)
#define I2C_BOOT_CNTRL_PSIZE                             ((uint32_t)0x40000000)
#define I2C_BOOT_CNTRL_CHAIN                             ((uint32_t)0x80000000)

/* EXI2C_REG_WADDR : Register Bits Masks Definitions */
#define EXI2C_REG_WADDR_ADDR                             ((uint32_t)0xfffffffc)

/* EXI2C_REG_WDATA : Register Bits Masks Definitions */
#define EXI2C_REG_WDATA_WDATA                            ((uint32_t)0xffffffff)

/* EXI2C_REG_RADDR : Register Bits Masks Definitions */
#define EXI2C_REG_RADDR_ADDR                             ((uint32_t)0xfffffffc)

/* EXI2C_REG_RDATA : Register Bits Masks Definitions */
#define EXI2C_REG_RDATA_RDATA                            ((uint32_t)0xffffffff)

/* EXI2C_ACC_STAT : Register Bits Masks Definitions */
#define EXI2C_ACC_STAT_ALERT_FLAG                        ((uint32_t)0x00000001)
#define EXI2C_ACC_STAT_IMB_FLAG                          ((uint32_t)0x00000004)
#define EXI2C_ACC_STAT_OMB_FLAG                          ((uint32_t)0x00000008)
#define EXI2C_ACC_STAT_ACC_OK                            ((uint32_t)0x00000080)

/* EXI2C_ACC_CNTRL : Register Bits Masks Definitions */
#define EXI2C_ACC_CNTRL_WINC                             ((uint32_t)0x00000004)
#define EXI2C_ACC_CNTRL_RINC                             ((uint32_t)0x00000008)
#define EXI2C_ACC_CNTRL_WSIZE                            ((uint32_t)0x00000030)
#define EXI2C_ACC_CNTRL_RSIZE                            ((uint32_t)0x000000c0)

/* EXI2C_STAT : Register Bits Masks Definitions */
#define EXI2C_STAT_PORT                                  ((uint32_t)0x00000001)
#define EXI2C_STAT_DEL                                   ((uint32_t)0x00100000)
#define EXI2C_STAT_FAB                                   ((uint32_t)0x00200000)
#define EXI2C_STAT_LOG                                   ((uint32_t)0x00400000)
#define EXI2C_STAT_RCS                                   ((uint32_t)0x00800000)
#define EXI2C_STAT_MECS                                  ((uint32_t)0x01000000)
#define EXI2C_STAT_I2C                                   ((uint32_t)0x02000000)
#define EXI2C_STAT_IMBR                                  ((uint32_t)0x04000000)
#define EXI2C_STAT_OMBW                                  ((uint32_t)0x08000000)
#define EXI2C_STAT_SW_STAT0                              ((uint32_t)0x10000000)
#define EXI2C_STAT_SW_STAT1                              ((uint32_t)0x20000000)
#define EXI2C_STAT_SW_STAT2                              ((uint32_t)0x40000000)
#define EXI2C_STAT_RESET                                 ((uint32_t)0x80000000)

/* EXI2C_STAT_ENABLE : Register Bits Masks Definitions */
#define EXI2C_STAT_ENABLE_PORT                           ((uint32_t)0x00000001)
#define EXI2C_STAT_ENABLE_DEL                            ((uint32_t)0x00100000)
#define EXI2C_STAT_ENABLE_FAB                            ((uint32_t)0x00200000)
#define EXI2C_STAT_ENABLE_LOG                            ((uint32_t)0x00400000)
#define EXI2C_STAT_ENABLE_RCS                            ((uint32_t)0x00800000)
#define EXI2C_STAT_ENABLE_MECS                           ((uint32_t)0x01000000)
#define EXI2C_STAT_ENABLE_I2C                            ((uint32_t)0x02000000)
#define EXI2C_STAT_ENABLE_IMBR                           ((uint32_t)0x04000000)
#define EXI2C_STAT_ENABLE_OMBW                           ((uint32_t)0x08000000)
#define EXI2C_STAT_ENABLE_SW_STAT0                       ((uint32_t)0x10000000)
#define EXI2C_STAT_ENABLE_SW_STAT1                       ((uint32_t)0x20000000)
#define EXI2C_STAT_ENABLE_SW_STAT2                       ((uint32_t)0x40000000)
#define EXI2C_STAT_ENABLE_RESET                          ((uint32_t)0x80000000)

/* EXI2C_MBOX_OUT : Register Bits Masks Definitions */
#define EXI2C_MBOX_OUT_DATA                              ((uint32_t)0xffffffff)

/* EXI2C_MBOX_IN : Register Bits Masks Definitions */
#define EXI2C_MBOX_IN_DATA                               ((uint32_t)0xffffffff)

/* I2C_X : Register Bits Masks Definitions */
#define I2C_X_MARBTO                                     ((uint32_t)0x00000001)
#define I2C_X_MSCLTO                                     ((uint32_t)0x00000002)
#define I2C_X_MBTTO                                      ((uint32_t)0x00000004)
#define I2C_X_MTRTO                                      ((uint32_t)0x00000008)
#define I2C_X_MCOL                                       ((uint32_t)0x00000010)
#define I2C_X_MNACK                                      ((uint32_t)0x00000020)
#define I2C_X_BLOK                                       ((uint32_t)0x00000100)
#define I2C_X_BLNOD                                      ((uint32_t)0x00000200)
#define I2C_X_BLSZ                                       ((uint32_t)0x00000400)
#define I2C_X_BLERR                                      ((uint32_t)0x00000800)
#define I2C_X_BLTO                                       ((uint32_t)0x00001000)
#define I2C_X_MTD                                        ((uint32_t)0x00004000)
#define I2C_X_SSCLTO                                     ((uint32_t)0x00020000)
#define I2C_X_SBTTO                                      ((uint32_t)0x00040000)
#define I2C_X_STRTO                                      ((uint32_t)0x00080000)
#define I2C_X_SCOL                                       ((uint32_t)0x00100000)
#define I2C_X_OMBR                                       ((uint32_t)0x00400000)
#define I2C_X_IMBW                                       ((uint32_t)0x00800000)
#define I2C_X_DCMDD                                      ((uint32_t)0x01000000)
#define I2C_X_DHIST                                      ((uint32_t)0x02000000)
#define I2C_X_DTIMER                                     ((uint32_t)0x04000000)
#define I2C_X_SD                                         ((uint32_t)0x10000000)
#define I2C_X_SDR                                        ((uint32_t)0x20000000)
#define I2C_X_SDW                                        ((uint32_t)0x40000000)

/* I2C_NEW_EVENT : Register Bits Masks Definitions */
#define I2C_NEW_EVENT_MARBTO                             ((uint32_t)0x00000001)
#define I2C_NEW_EVENT_MSCLTO                             ((uint32_t)0x00000002)
#define I2C_NEW_EVENT_MBTTO                              ((uint32_t)0x00000004)
#define I2C_NEW_EVENT_MTRTO                              ((uint32_t)0x00000008)
#define I2C_NEW_EVENT_MCOL                               ((uint32_t)0x00000010)
#define I2C_NEW_EVENT_MNACK                              ((uint32_t)0x00000020)
#define I2C_NEW_EVENT_BLOK                               ((uint32_t)0x00000100)
#define I2C_NEW_EVENT_BLNOD                              ((uint32_t)0x00000200)
#define I2C_NEW_EVENT_BLSZ                               ((uint32_t)0x00000400)
#define I2C_NEW_EVENT_BLERR                              ((uint32_t)0x00000800)
#define I2C_NEW_EVENT_BLTO                               ((uint32_t)0x00001000)
#define I2C_NEW_EVENT_MTD                                ((uint32_t)0x00004000)
#define I2C_NEW_EVENT_SSCLTO                             ((uint32_t)0x00020000)
#define I2C_NEW_EVENT_SBTTO                              ((uint32_t)0x00040000)
#define I2C_NEW_EVENT_STRTO                              ((uint32_t)0x00080000)
#define I2C_NEW_EVENT_SCOL                               ((uint32_t)0x00100000)
#define I2C_NEW_EVENT_OMBR                               ((uint32_t)0x00400000)
#define I2C_NEW_EVENT_IMBW                               ((uint32_t)0x00800000)
#define I2C_NEW_EVENT_DCMDD                              ((uint32_t)0x01000000)
#define I2C_NEW_EVENT_DHIST                              ((uint32_t)0x02000000)
#define I2C_NEW_EVENT_DTIMER                             ((uint32_t)0x04000000)
#define I2C_NEW_EVENT_SD                                 ((uint32_t)0x10000000)
#define I2C_NEW_EVENT_SDR                                ((uint32_t)0x20000000)
#define I2C_NEW_EVENT_SDW                                ((uint32_t)0x40000000)

/* I2C_EVENT_ENB : Register Bits Masks Definitions */
#define I2C_EVENT_ENB_MARBTO                             ((uint32_t)0x00000001)
#define I2C_EVENT_ENB_MSCLTO                             ((uint32_t)0x00000002)
#define I2C_EVENT_ENB_MBTTO                              ((uint32_t)0x00000004)
#define I2C_EVENT_ENB_MTRTO                              ((uint32_t)0x00000008)
#define I2C_EVENT_ENB_MCOL                               ((uint32_t)0x00000010)
#define I2C_EVENT_ENB_MNACK                              ((uint32_t)0x00000020)
#define I2C_EVENT_ENB_BLOK                               ((uint32_t)0x00000100)
#define I2C_EVENT_ENB_BLNOD                              ((uint32_t)0x00000200)
#define I2C_EVENT_ENB_BLSZ                               ((uint32_t)0x00000400)
#define I2C_EVENT_ENB_BLERR                              ((uint32_t)0x00000800)
#define I2C_EVENT_ENB_BLTO                               ((uint32_t)0x00001000)
#define I2C_EVENT_ENB_MTD                                ((uint32_t)0x00004000)
#define I2C_EVENT_ENB_SSCLTO                             ((uint32_t)0x00020000)
#define I2C_EVENT_ENB_SBTTO                              ((uint32_t)0x00040000)
#define I2C_EVENT_ENB_STRTO                              ((uint32_t)0x00080000)
#define I2C_EVENT_ENB_SCOL                               ((uint32_t)0x00100000)
#define I2C_EVENT_ENB_OMBR                               ((uint32_t)0x00400000)
#define I2C_EVENT_ENB_IMBW                               ((uint32_t)0x00800000)
#define I2C_EVENT_ENB_DCMDD                              ((uint32_t)0x01000000)
#define I2C_EVENT_ENB_DHIST                              ((uint32_t)0x02000000)
#define I2C_EVENT_ENB_DTIMER                             ((uint32_t)0x04000000)
#define I2C_EVENT_ENB_SD                                 ((uint32_t)0x10000000)
#define I2C_EVENT_ENB_SDR                                ((uint32_t)0x20000000)
#define I2C_EVENT_ENB_SDW                                ((uint32_t)0x40000000)

/* I2C_DIVIDER : Register Bits Masks Definitions */
#define I2C_DIVIDER_MSDIV                                ((uint32_t)0x00000fff)
#define I2C_DIVIDER_USDIV                                ((uint32_t)0x0fff0000)

/* I2C_FILTER_SCL_CFG : Register Bits Masks Definitions */
#define I2C_FILTER_SCL_CFG_END_TAP                       ((uint32_t)0x00001f00)
#define I2C_FILTER_SCL_CFG_THRES0                        ((uint32_t)0x001f0000)
#define I2C_FILTER_SCL_CFG_THRES1                        ((uint32_t)0x1f000000)

/* I2C_FILTER_SDA_CFG : Register Bits Masks Definitions */
#define I2C_FILTER_SDA_CFG_END_TAP                       ((uint32_t)0x00001f00)
#define I2C_FILTER_SDA_CFG_THRES0                        ((uint32_t)0x001f0000)
#define I2C_FILTER_SDA_CFG_THRES1                        ((uint32_t)0x1f000000)

/* I2C_START_SETUP_HOLD : Register Bits Masks Definitions */
#define I2C_START_SETUP_HOLD_START_HOLD                  ((uint32_t)0x0000ffff)
#define I2C_START_SETUP_HOLD_START_SETUP                 ((uint32_t)0xffff0000)

/* I2C_STOP_IDLE : Register Bits Masks Definitions */
#define I2C_STOP_IDLE_IDLE_DET                           ((uint32_t)0x0000ffff)
#define I2C_STOP_IDLE_STOP_SETUP                         ((uint32_t)0xffff0000)

/* I2C_SDA_SETUP_HOLD : Register Bits Masks Definitions */
#define I2C_SDA_SETUP_HOLD_SDA_HOLD                      ((uint32_t)0x0000ffff)
#define I2C_SDA_SETUP_HOLD_SDA_SETUP                     ((uint32_t)0xffff0000)

/* I2C_SCL_PERIOD : Register Bits Masks Definitions */
#define I2C_SCL_PERIOD_SCL_LOW                           ((uint32_t)0x0000ffff)
#define I2C_SCL_PERIOD_SCL_HIGH                          ((uint32_t)0xffff0000)

/* I2C_SCL_MIN_PERIOD : Register Bits Masks Definitions */
#define I2C_SCL_MIN_PERIOD_SCL_MINL                      ((uint32_t)0x0000ffff)
#define I2C_SCL_MIN_PERIOD_SCL_MINH                      ((uint32_t)0xffff0000)

/* I2C_SCL_ARB_TIMEOUT : Register Bits Masks Definitions */
#define I2C_SCL_ARB_TIMEOUT_ARB_TO                       ((uint32_t)0x0000ffff)
#define I2C_SCL_ARB_TIMEOUT_SCL_TO                       ((uint32_t)0xffff0000)

/* I2C_BYTE_TRAN_TIMEOUT : Register Bits Masks Definitions */
#define I2C_BYTE_TRAN_TIMEOUT_TRAN_TO                    ((uint32_t)0x0000ffff)
#define I2C_BYTE_TRAN_TIMEOUT_BYTE_TO                    ((uint32_t)0xffff0000)

/* I2C_BOOT_DIAG_TIMER : Register Bits Masks Definitions */
#define I2C_BOOT_DIAG_TIMER_COUNT                        ((uint32_t)0x0000ffff)
#define I2C_BOOT_DIAG_TIMER_FREERUN                      ((uint32_t)0x80000000)

/* I2C_BOOT_DIAG_PROGRESS : Register Bits Masks Definitions */
#define I2C_BOOT_DIAG_PROGRESS_PADDR                     ((uint32_t)0x0000ffff)
#define I2C_BOOT_DIAG_PROGRESS_REGCNT                    ((uint32_t)0xffff0000)

/* I2C_BOOT_DIAG_CFG : Register Bits Masks Definitions */
#define I2C_BOOT_DIAG_CFG_BOOT_ADDR                      ((uint32_t)0x0000007f)
#define I2C_BOOT_DIAG_CFG_PINC                           ((uint32_t)0x10000000)
#define I2C_BOOT_DIAG_CFG_PASIZE                         ((uint32_t)0x20000000)
#define I2C_BOOT_DIAG_CFG_BDIS                           ((uint32_t)0x40000000)
#define I2C_BOOT_DIAG_CFG_BOOTING                        ((uint32_t)0x80000000)


/********************************************************/
/* PVT Register Address Offset Definitions              */
/********************************************************/

#define RXS_PVT_BH                                   ((uint32_t)0x000c3000)
#define RXS_PVT_MEAS_CSR                             ((uint32_t)0x000c3004)
#define RXS_PVT_SCRATCH                              ((uint32_t)0x000c3008)
#define RXS_PVT_STAT_CSR                             ((uint32_t)0x000c300c)
#define RXS_PVT_INT_EN_CSR                           ((uint32_t)0x000c3010)
#define RXS_PVT_EVENT_GEN_CSR                        ((uint32_t)0x000c3014)
#define RXS_PVT_X_TRIM_VAL_CSR(X)         ((uint32_t)(0xc3030 + 0x004*(X)))

/********************************************************/
/* PVT Register Bit Masks and Reset Values Definitions  */
/********************************************************/

/* RXS_PVT_BH : Register Bits Masks Definitions */
#define RXS_PVT_BH_BLK_TYPE                          ((uint32_t)0x00000fff)
#define RXS_PVT_BH_BLK_REV                           ((uint32_t)0x0000f000)
#define RXS_PVT_BH_NEXT_BLK_PTR                      ((uint32_t)0xffff0000)

/* RXS_PVT_MEAS_CSR : Register Bits Masks Definitions */
#define RXS_PVT_MEAS_CSR_MEAS_DATA                   ((uint32_t)0x0000ffc0)
#define RXS_PVT_MEAS_CSR_MEAS_MODE                   ((uint32_t)0x00e00000)
#define RXS_PVT_MEAS_CSR_ENA_MEAS                    ((uint32_t)0x3c000000)
#define RXS_PVT_MEAS_CSR_BUSY                        ((uint32_t)0x80000000)

/* RXS_PVT_SCRATCH : Register Bits Masks Definitions */
#define RXS_PVT_SCRATCH_SCRATCH                      ((uint32_t)0xffffffff)

/* RXS_PVT_STAT_CSR : Register Bits Masks Definitions */
#define RXS_PVT_STAT_CSR_MEAS_DONE                   ((uint32_t)0x80000000)

/* RXS_PVT_INT_EN_CSR : Register Bits Masks Definitions */
#define RXS_PVT_INT_EN_CSR_MEAS_DONE                 ((uint32_t)0x80000000)

/* RXS_PVT_EVENT_GEN_CSR : Register Bits Masks Definitions */
#define RXS_PVT_EVENT_GEN_CSR_MEAS_DONE              ((uint32_t)0x80000000)

/* RXS_PVT_X_TRIM_VAL_CSR : Register Bits Masks Definitions */
#define RXS_PVT_X_TRIM_VAL_CSR_TRIM_VALUE            ((uint32_t)0xf8000000)


/********************************************************/
/* BootRAM Register Address Offset Definitions          */
/********************************************************/

#define RXS_BOOT_MEMORY_BH                           ((uint32_t)0x000c3400)
#define RXS_PHY_BOOT_CTL                             ((uint32_t)0x000c3404)
#define RXS_BOOT_MEM_CTRL                            ((uint32_t)0x000c3408)
#define RXS_BOOT_MEM_STAT_CSR                        ((uint32_t)0x000c340c)
#define RXS_BOOT_MEM_INT_EN_CSR                      ((uint32_t)0x000c3410)
#define RXS_BOOT_MEM_EVENT_GEN_CSR                   ((uint32_t)0x000c3414)
#define RXS_BOOT_MEM_ENTRYX_CSR(X)        ((uint32_t)(0xc4000 + 0x004*(X)))

/***********************************************************/
/* BootRAM Register Bit Masks and Reset Values Definitions */
/***********************************************************/

/* RXS_BOOT_MEMORY_BH : Register Bits Masks Definitions */
#define RXS_BOOT_MEMORY_BH_BLK_TYPE                  ((uint32_t)0x00000fff)
#define RXS_BOOT_MEMORY_BH_BLK_REV                   ((uint32_t)0x0000f000)
#define RXS_BOOT_MEMORY_BH_NEXT_BLK_PTR              ((uint32_t)0xffff0000)

/* RXS_PHY_BOOT_CTL : Register Bits Masks Definitions */
#define RXS_PHY_BOOT_CTL_END_ADDR                    ((uint32_t)0x00000fff)
#define RXS_PHY_BOOT_CTL_BROADCAST_MASK              ((uint32_t)0x7ff80000)
#define RXS_PHY_BOOT_CTL_START_LOAD                  ((uint32_t)0x80000000)

/* RXS_BOOT_MEM_CTRL : Register Bits Masks Definitions */
#define RXS_BOOT_MEM_CTRL_BOOT_MEM_CGATE             ((uint32_t)0x00800000)

/* RXS_BOOT_MEM_STAT_CSR : Register Bits Masks Definitions */
#define RXS_BOOT_MEM_STAT_CSR_PHYMEM_BOOT_FAIL       ((uint32_t)0x80000000)

/* RXS_BOOT_MEM_INT_EN_CSR : Register Bits Masks Definitions */
#define RXS_BOOT_MEM_INT_EN_CSR_PHYMEM_BOOT_FAIL     ((uint32_t)0x80000000)

/* RXS_BOOT_MEM_EVENT_GEN_CSR : Register Bits Masks Definitions */
#define RXS_BOOT_MEM_EVENT_GEN_CSR_PHYMEM_BOOT_FAIL  ((uint32_t)0x80000000)

/* RXS_BOOT_MEM_ENTRYX_CSR : Register Bits Masks Definitions */
#define RXS_BOOT_MEM_ENTRYX_CSR_FW_DATA              ((uint32_t)0xffffffff)

#ifdef __cplusplus
}
#endif

#endif  // __RXS2448_H__

