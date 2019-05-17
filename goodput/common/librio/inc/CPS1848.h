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

#ifndef __CPS1848_H__
#define __CPS1848_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CPS_MAX_PORTS 18
#define CPS_MAX_QUADRANT_PORTS 5
#define CPS_MAX_PLLS 12
#define CPS_MAX_PORT_LANES 4
#define CPS_MAX_LANES 48

/**************************************************/
/* CPS1848 : Register address offset definitions  */
/**************************************************/
#define CPS1848_DEV_IDENT_CAR                                      (0x00000000)
#define CPS1848_DEV_INF_CAR                                        (0x00000004)
#define CPS1848_ASSY_IDENT_CAR                                     (0x00000008)
#define CPS1848_ASSY_INF_CAR                                       (0x0000000c)
#define CPS1848_PROC_ELEM_FEAT_CAR                                 (0x00000010)
#define CPS1848_SWITCH_PORT_INF_CAR                                (0x00000014)
#define CPS1848_SRC_OPS_CAR                                        (0x00000018)
#define CPS1848_SWITCH_MCAST_SUP_CAR                               (0x00000030)
#define CPS1848_SWITCH_RTE_TBL_LIM_CAR                             (0x00000034)
#define CPS1848_SWITCH_MULT_INF_CAR                                (0x00000038)
#define CPS1848_HOST_BASE_DEVICEID_LOCK_CSR                        (0x00000068)
#define CPS1848_COMP_TAG_CSR                                       (0x0000006c)
#define CPS1848_RTE_DESTID_CSR                                     (0x00000070)
#define CPS1848_RTE_PORT_CSR                                       (0x00000074)
#define CPS1848_RTE_DEFAULT_PORT_CSR                               (0x00000078)
#define CPS1848_MCAST_MASK_PORT_CSR                                (0x00000080)
#define CPS1848_MCAST_ASSOC_SEL_CSR                                (0x00000084)
#define CPS1848_MCAST_ASSOC_OP_CSR                                 (0x00000088)
#define CPS1848_PORT_MAINT_BLK_HEAD                                (0x00000100)
#define CPS1848_PORT_LINK_TO_CTL_CSR                               (0x00000120)
#define CPS1848_PORT_GEN_CTL_CSR                                   (0x0000013c)
#define CPS1848_PORT_X_LINK_MAINT_REQ_CSR(X)               (0x0140 + 0x020*(X))
#define CPS1848_PORT_X_LINK_MAINT_RESP_CSR(X)              (0x0144 + 0x020*(X))
#define CPS1848_PORT_X_LOCAL_ACKID_CSR(X)                  (0x0148 + 0x020*(X))
#define CPS1848_PORT_X_CTL_2_CSR(X)                        (0x0154 + 0x020*(X))
#define CPS1848_PORT_X_ERR_STAT_CSR(X)                     (0x0158 + 0x020*(X))
#define CPS1848_PORT_X_CTL_1_CSR(X)                        (0x015c + 0x020*(X))
#define CPS1848_VC_REGISTER_BLK_HEAD                               (0x00000600)
#define CPS1848_ERR_MGT_EXTENSION_BLK_HEAD                         (0x00001000)
#define CPS1848_LT_ERR_DET_CSR                                     (0x00001008)
#define CPS1848_LT_ERR_EN_CSR                                      (0x0000100c)
#define CPS1848_LT_DEVICEID_CAPT_CSR                               (0x00001018)
#define CPS1848_LT_CTL_CAPT_CSR                                    (0x0000101c)
#define CPS1848_PW_TARGET_DEVICEID_CSR                             (0x00001028)
#define CPS1848_PKT_TTL_CSR                                        (0x0000102c)
#define CPS1848_PORT_X_ERR_DET_CSR(X)                      (0x1040 + 0x040*(X))
#define CPS1848_PORT_X_ERR_RATE_EN_CSR(X)                  (0x1044 + 0x040*(X))
#define CPS1848_PORT_X_ATTR_CAPT_CSR(X)                    (0x1048 + 0x040*(X))
#define CPS1848_PORT_X_CAPT_0_CSR(X)                       (0x104c + 0x040*(X))
#define CPS1848_PORT_X_CAPT_1_CSR(X)                       (0x1050 + 0x040*(X))
#define CPS1848_PORT_X_CAPT_2_CSR(X)                       (0x1054 + 0x040*(X))
#define CPS1848_PORT_X_CAPT_3_CSR(X)                       (0x1058 + 0x040*(X))
#define CPS1848_PORT_X_ERR_RATE_CSR(X)                     (0x1068 + 0x040*(X))
#define CPS1848_PORT_X_ERR_RATE_THRESH_CSR(X)              (0x106c + 0x040*(X))
#define CPS1848_LANE_STATUS_BLK_HEAD                               (0x00002000)
#define CPS1848_LANE_X_STATUS_0_CSR(X)                     (0x2010 + 0x020*(X))
#define CPS1848_LANE_X_STATUS_1_CSR(X)                     (0x2014 + 0x020*(X))
#define CPS1848_LANE_X_STATUS_2_CSR(X)                     (0x2018 + 0x020*(X))
#define CPS1848_LANE_X_STATUS_3_CSR(X)                     (0x201c + 0x020*(X))
#define CPS1848_LANE_X_STATUS_4_CSR(X)                     (0x2020 + 0x020*(X))
#define CPS1848_RTE_PORT_SEL                                       (0x00010070)
#define CPS1848_MCAST_RTE_SEL                                      (0x00010080)
#define CPS1848_PORT_X_WM(X)                              (0x11000 + 0x010*(X))
#define CPS1848_BCAST_WM                                           (0x0001f000)
#define CPS1848_AUX_PORT_ERR_CAPT_EN                               (0x00020000)
#define CPS1848_AUX_PORT_ERR_DET                                   (0x00020004)
#define CPS1848_CFG_ERR_CAPT_EN                                    (0x00020008)
#define CPS1848_CFG_ERR_DET                                        (0x00020010)
#define CPS1848_IMPL_SPEC_LT_ADDR_CAPT                             (0x00021014)
#define CPS1848_LT_ERR_RPT_EN                                      (0x0003100c)
#define CPS1848_PORT_X_ERR_RPT_EN(X)                      (0x31044 + 0x040*(X))
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(X)            (0x3104c + 0x040*(X))
#define CPS1848_LANE_X_ERR_RPT_EN(X)                      (0x38010 + 0x100*(X))
#define CPS1848_BCAST_PORT_ERR_RPT_EN                              (0x0003ff04)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN                    (0x0003ff0c)
#define CPS1848_BCAST_LANE_ERR_RPT_EN                              (0x0003ff10)
#define CPS1848_PORT_X_PGC_MODE(X)                       (0x100100 + 0x010*(X))
#define CPS1848_PORT_X_PGC_DATA(X)                       (0x100104 + 0x010*(X))
#define CPS1848_BCAST_DEV_RTE_TABLE_X(X)                 (0xe00000 + 0x004*(X))
#define CPS1848_BCAST_DOM_RTE_TABLE_X(X)                 (0xe00400 + 0x004*(X))
#define CPS1848_PORT_X_DEV_RTE_TABLE_Y(X,Y)   (0xe10000 + 0x1000*(X) + 0x4*(Y))
#define CPS1848_PORT_X_DOM_RTE_TABLE_Y(X,Y)   (0xe10400 + 0x1000*(X) + 0x4*(Y))
#define CPS1848_PORT_X_TRACE_0_VAL_0(X)                  (0xe40000 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_0_VAL_1(X)                  (0xe40004 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_0_VAL_2(X)                  (0xe40008 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_0_VAL_3(X)                  (0xe4000c + 0x100*(X))
#define CPS1848_PORT_X_TRACE_0_VAL_4(X)                  (0xe40010 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_0_MASK_0(X)                 (0xe40014 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_0_MASK_1(X)                 (0xe40018 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_0_MASK_2(X)                 (0xe4001c + 0x100*(X))
#define CPS1848_PORT_X_TRACE_0_MASK_3(X)                 (0xe40020 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_0_MASK_4(X)                 (0xe40024 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_1_VAL_0(X)                  (0xe40028 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_1_VAL_1(X)                  (0xe4002c + 0x100*(X))
#define CPS1848_PORT_X_TRACE_1_VAL_2(X)                  (0xe40030 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_1_VAL_3(X)                  (0xe40034 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_1_VAL_4(X)                  (0xe40038 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_1_MASK_0(X)                 (0xe4003c + 0x100*(X))
#define CPS1848_PORT_X_TRACE_1_MASK_1(X)                 (0xe40040 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_1_MASK_2(X)                 (0xe40044 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_1_MASK_3(X)                 (0xe40048 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_1_MASK_4(X)                 (0xe4004c + 0x100*(X))
#define CPS1848_PORT_X_TRACE_2_VAL_0(X)                  (0xe40050 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_2_VAL_1(X)                  (0xe40054 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_2_VAL_2(X)                  (0xe40058 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_2_VAL_3(X)                  (0xe4005c + 0x100*(X))
#define CPS1848_PORT_X_TRACE_2_VAL_4(X)                  (0xe40060 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_2_MASK_0(X)                 (0xe40064 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_2_MASK_1(X)                 (0xe40068 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_2_MASK_2(X)                 (0xe4006c + 0x100*(X))
#define CPS1848_PORT_X_TRACE_2_MASK_3(X)                 (0xe40070 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_2_MASK_4(X)                 (0xe40074 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_3_VAL_0(X)                  (0xe40078 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_3_VAL_1(X)                  (0xe4007c + 0x100*(X))
#define CPS1848_PORT_X_TRACE_3_VAL_2(X)                  (0xe40080 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_3_VAL_3(X)                  (0xe40084 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_3_VAL_4(X)                  (0xe40088 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_3_MASK_0(X)                 (0xe4008c + 0x100*(X))
#define CPS1848_PORT_X_TRACE_3_MASK_1(X)                 (0xe40090 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_3_MASK_2(X)                 (0xe40094 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_3_MASK_3(X)                 (0xe40098 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_3_MASK_4(X)                 (0xe4009c + 0x100*(X))
#define CPS1848_PORT_MAX_TRACE_INST      4
#define CPS1848_PORT_MAX_TRACE_REGS      5

#define CPS1848_PORT_P_TRACE_I_VAL_R(P,I,R) (((I<CPS1848_PORT_MAX_TRACE_INST)  \
		                          &&  (R<CPS1848_PORT_MAX_TRACE_REGS)) \
		                      ?(0xe40000+(0x100*(P))+(0x28*(I))+(4*(R))):0)

#define CPS1848_PORT_P_TRACE_I_MASK_R(P,I,R) \
	                                    (((I<CPS1848_PORT_MAX_TRACE_INST)  \
		                          &&  (R<CPS1848_PORT_MAX_TRACE_REGS)) \
		                      ?(0xe40014+(0x100*(P))+(0x28*(I))+(4*(R))):0)

#define CPS1848_BCAST_TRACE_0_VAL_0                                (0x00e4f000)
#define CPS1848_BCAST_TRACE_0_VAL_1                                (0x00e4f004)
#define CPS1848_BCAST_TRACE_VAL_1_BLK_2                            (0x00e4f008)
#define CPS1848_BCAST_TRACE_0_VAL_3                                (0x00e4f00c)
#define CPS1848_BCAST_TRACE_0_VAL_4                                (0x00e4f010)
#define CPS1848_BCAST_TRACE_0_MASK_0                               (0x00e4f014)
#define CPS1848_BCAST_TRACE_0_MASK_1                               (0x00e4f018)
#define CPS1848_BCAST_TRACE_0_MASK_2                               (0x00e4f01c)
#define CPS1848_BCAST_TRACE_0_MASK_3                               (0x00e4f020)
#define CPS1848_BCAST_TRACE_0_MASK_4                               (0x00e4f024)
#define CPS1848_BCAST_TRACE_1_VAL_0                                (0x00e4f028)
#define CPS1848_BCAST_TRACE_1_VAL_1                                (0x00e4f02c)
#define CPS1848_BCAST_TRACE_1_VAL_2                                (0x00e4f030)
#define CPS1848_BCAST_TRACE_1_VAL_3                                (0x00e4f034)
#define CPS1848_BCAST_TRACE_1_VAL_4                                (0x00e4f038)
#define CPS1848_BCAST_TRACE_1_MASK_0                               (0x00e4f03c)
#define CPS1848_BCAST_TRACE_1_MASK_1                               (0x00e4f040)
#define CPS1848_BCAST_TRACE_1_MASK_2                               (0x00e4f044)
#define CPS1848_BCAST_TRACE_1_MASK_3                               (0x00e4f048)
#define CPS1848_BCAST_TRACE_1_MASK_4                               (0x00e4f04c)
#define CPS1848_BCAST_TRACE_2_VAL_0                                (0x00e4f050)
#define CPS1848_BCAST_TRACE_2_VAL_1                                (0x00e4f054)
#define CPS1848_BCAST_TRACE_2_VAL_2                                (0x00e4f058)
#define CPS1848_BCAST_TRACE_2_VAL_3                                (0x00e4f05c)
#define CPS1848_BCAST_TRACE_2_VAL_4                                (0x00e4f060)
#define CPS1848_BCAST_TRACE_2_MASK__0                              (0x00e4f064)
#define CPS1848_BCAST_TRACE_2_MASK_1                               (0x00e4f068)
#define CPS1848_BCAST_TRACE_2_MASK_2                               (0x00e4f06c)
#define CPS1848_BCAST_TRACE_2_MASK_3                               (0x00e4f070)
#define CPS1848_BCAST_TRACE_2_MASK_4                               (0x00e4f074)
#define CPS1848_BCAST_TRACE_3_VAL_0                                (0x00e4f078)
#define CPS1848_BCAST_TRACE_3_VAL_1                                (0x00e4f07c)
#define CPS1848_BCAST_TRACE_3_VAL_2                                (0x00e4f080)
#define CPS1848_BCAST_TRACE_3_VAL_3                                (0x00e4f084)
#define CPS1848_BCAST_TRACE_3_VAL_4                                (0x00e4f088)
#define CPS1848_BCAST_TRACE_3_MASK_0                               (0x00e4f08c)
#define CPS1848_BCAST_TRACE_3_MASK_1                               (0x00e4f090)
#define CPS1848_BCAST_TRACE_3_MASK_2                               (0x00e4f094)
#define CPS1848_BCAST_TRACE_3_MASK_3                               (0x00e4f098)
#define CPS1848_BCAST_TRACE_3_MASK_4                               (0x00e4f09c)
#define CPS1848_DEVICE_CTL_1                                       (0x00f2000c)
#define CPS1848_CFG_BLK_ERR_RPT                                    (0x00f20014)
#define CPS1848_AUX_PORT_ERR_RPT_EN                                (0x00f20018)
#define CPS1848_RIO_DOMAIN                                         (0x00f20020)
#define CPS1848_PW_CTL                                             (0x00f20024)
#define CPS1848_ASSY_IDENT_CAR_OVRD                                (0x00f2002c)
#define CPS1848_ASSY_INF_CAR_OVRD                                  (0x00f20030)
#define CPS1848_DEVICE_SOFT_RESET                                  (0x00f20040)
#define CPS1848_I2C_MASTER_CTL                                     (0x00f20050)
#define CPS1848_I2C_MASTER_STAT_CTL                                (0x00f20054)
#define CPS1848_JTAG_CTL                                           (0x00f2005c)
#define CPS1848_EXT_MECS_TRIG_CNTR                                 (0x00f20060)
#define CPS1848_MAINT_DROP_PKT_CNTR                                (0x00f20064)
#define CPS1848_SWITCH_PARAM_1                                     (0x00f20068)
#define CPS1848_SWITCH_PARAM_2                                     (0x00f2006c)
#define CPS1848_QUAD_CFG                                           (0x00f20200)
#define CPS1848_DEVICE_RESET_CTL                                   (0x00f20300)
#define CPS1848_BCAST_MCAST_MASK_X(X)                    (0xf30000 + 0x004*(X))
#define CPS1848_PORT_X_MCAST_MASK_Y(X,Y)       (0xf38000 + 0x100*(X) + 0x4*(Y))
#define CPS1848_PORT_X_OPS(X)                            (0xf40004 + 0x100*(X))
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET(X)              (0xf40008 + 0x100*(X))
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(X)          (0xf4000c + 0x100*(X))
#define CPS1848_PORT_X_VC0_PA_TX_CNTR(X)                 (0xf40010 + 0x100*(X))
#define CPS1848_PORT_X_NACK_TX_CNTR(X)                   (0xf40014 + 0x100*(X))
#define CPS1848_PORT_X_VC0_RTRY_TX_CNTR(X)               (0xf40018 + 0x100*(X))
#define CPS1848_PORT_X_VC0_PKT_TX_CNTR(X)                (0xf4001c + 0x100*(X))
#define CPS1848_PORT_X_TRACE_CNTR_0(X)                   (0xf40020 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_CNTR_1(X)                   (0xf40024 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_CNTR_2(X)                   (0xf40028 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_CNTR_3(X)                   (0xf4002c + 0x100*(X))
#define CPS1848_PORT_X_TRACE_CNTR_Y(X,Y)             (0xf40020+0x100*(X)+(4*Y))
#define CPS1848_PORT_X_FILTER_CNTR_0(X)                  (0xf40030 + 0x100*(X))
#define CPS1848_PORT_X_FILTER_CNTR_1(X)                  (0xf40034 + 0x100*(X))
#define CPS1848_PORT_X_FILTER_CNTR_2(X)                  (0xf40038 + 0x100*(X))
#define CPS1848_PORT_X_FILTER_CNTR_3(X)                  (0xf4003c + 0x100*(X))
#define CPS1848_PORT_X_FILTER_CNTR_Y(X,Y)            (0xf40030+0x100*(X)+(4*Y))
#define CPS1848_PORT_X_VC0_PA_RX_CNTR(X)                 (0xf40040 + 0x100*(X))
#define CPS1848_PORT_X_NACK_RX_CNTR(X)                   (0xf40044 + 0x100*(X))
#define CPS1848_PORT_X_VC0_RTRY_RX_CNTR(X)               (0xf40048 + 0x100*(X))
#define CPS1848_PORT_X_VC0_CPB_TX_CNTR(X)                (0xf4004c + 0x100*(X))
#define CPS1848_PORT_X_VC0_PKT_RX_CNTR(X)                (0xf40050 + 0x100*(X))
#define CPS1848_PORT_X_TRACE_PW_CTL(X)                   (0xf40058 + 0x100*(X))
#define CPS1848_PORT_X_LANE_SYNC(X)                      (0xf40060 + 0x100*(X))
#define CPS1848_PORT_X_VC0_PKT_DROP_RX_CNTR(X)           (0xf40064 + 0x100*(X))
#define CPS1848_PORT_X_VC0_PKT_DROP_TX_CNTR(X)           (0xf40068 + 0x100*(X))
#define CPS1848_PORT_X_VC0_TTL_DROP_CNTR(X)              (0xf4006c + 0x100*(X))
#define CPS1848_PORT_X_VC0_CRC_LIMIT_DROP_CNTR(X)        (0xf40070 + 0x100*(X))
#define CPS1848_PORT_X_RETRY_CNTR(X)                     (0xf400cc + 0x100*(X))
#define CPS1848_PORT_X_STATUS_AND_CTL(X)                 (0xf400f0 + 0x100*(X))
#define CPS1848_BCAST_PORT_OPS                                     (0x00f4ff04)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET                       (0x00f4ff08)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN                   (0x00f4ff0c)
#define CPS1848_LOG_CTL                                            (0x00fd0000)
#define CPS1848_LOG_DATA                                           (0x00fd0004)
#define CPS1848_LOG_MATCH_X(X)                           (0xfd0008 + 0x004*(X))
#define CPS1848_LOG_MATCH_STATUS                                   (0x00fd0028)
#define CPS1848_LOG_EVENTS                                         (0x00fd002c)
#define CPS1848_LOG_CTL2                                           (0x00fd0030)
#define CPS1848_PLL_X_CTL_1(X)                           (0xff0000 + 0x010*(X))
#define CPS1848_PLL_X_CTL_2(X)                           (0xff0004 + 0x010*(X))
#define CPS1848_BCAST_PLL_CTL                                      (0x00ff0ff0)
#define CPS1848_LANE_X_CTL(X)                            (0xff8000 + 0x100*(X))
#define CPS1848_LANE_X_PRBS_GEN_SEED(X)                  (0xff8004 + 0x100*(X))
#define CPS1848_LANE_X_PRBS_ERR_CNTR(X)                  (0xff8008 + 0x100*(X))
#define CPS1848_LANE_X_ERR_DET(X)                        (0xff800c + 0x100*(X))
#define CPS1848_LANE_X_ERR_RATE_EN(X)                    (0xff8010 + 0x100*(X))
#define CPS1848_LANE_X_ATTR_CAPT(X)                      (0xff8014 + 0x100*(X))
#define CPS1848_LANE_X_DATA_CAPT_0(X)                    (0xff8018 + 0x100*(X))
#define CPS1848_LANE_X_DATA_CAPT_1(X)                    (0xff801c + 0x100*(X))
#define CPS1848_LANE_X_DFE_1(X)                          (0xff8028 + 0x100*(X))
#define CPS1848_LANE_X_DFE_2(X)                          (0xff802c + 0x100*(X))
#define CPS1848_LANE_X_DFE_5(X)                          (0xff8038 + 0x100*(X))
#define CPS1848_BCAST_LANE_CTL                                     (0x00ffff00)
#define CPS1848_BCAST_LANE_GEN_SEED                                (0x00ffff04)
#define CPS1848_BCAST_LANE_ERR_DET                                 (0x00ffff0c)
#define CPS1848_BCAST_LANE_ERR_RATE_EN                             (0x00ffff10)
#define CPS1848_BCAST_LANE_ATTR_CAPT                               (0x00ffff14)
#define CPS1848_BCAST_LANE_DFE_1                                   (0x00ffff18)
#define CPS1848_BCAST_LANE_DFE_2                                   (0x00ffff1c)
#define CPS1848_BCAST_PORT_ERR_DET                                 (0x00ffff40)
#define CPS1848_BCAST_PORT_ERR_RATE_EN                             (0x00ffff44)

// Register address computations for the routing table registers.

#define CPS_BROADCAST_DEVICE_ROUTE_TABLE		0xE00000
#define CPS_BROADCAST_DOMAIN_ROUTE_TABLE		0xE00400

#define CPS_MAX_MC_MASK  (40)

/* Get a device destination ID (8 bit) from a 16 bit destination ID
*/
#define CPS_DEVICE_DEST_ID(destid) ((uint8_t)destid)

/* Get a domain ID (8 bit) from a 16 bit destination ID
*/
#define CPS_DOMAIN_ID(destid)      ((uint8_t)((uint16_t)destid >> 8))

#define CPS_RT_USE_DEVICE_TABLE                 (0x000000DD)
#define CPS_RT_USE_DEFAULT_ROUTE                (0x000000DE)
#define CPS_RT_NO_ROUTE                         (0x000000DF)

/* Broadcast Unicast Domain Route Table Entry: 0xE00400 + DestId * 4
*/
#define CPS_BROADCAST_UC_DOMAIN_RT_ENTRY(dest_id) \
            ( CPS_BROADCAST_DOMAIN_ROUTE_TABLE + \
              ((((uint32_t)dest_id & 0xFF00) >> 8) << 2) )

/* Broadcast Multicast Domain Route Table Entry: 0xE00400 + DestId * 4
*/
#define CPS_BROADCAST_MC_DOMAIN_RT_ENTRY(dest_id) \
            ( CPS_BROADCAST_DOMAIN_ROUTE_TABLE + \
              ((((uint32_t)dest_id & 0xFF00) >> 8) << 2) )

/* Port-base Unicast Domain Route Table Entry: 
   0xE10400 + 0x1000 * PortNo + DestId * 4
*/
#define CPS_PORT_BASE_UC_DOMAIN_RT_ENTRY(port, dest_id) \
            ( CPS_PORT_BASED_DOMAIN_ROUTE_TABLE + \
              (((uint32_t)port & 0x1F) << 12) + \
              ((((uint32_t)dest_id & 0xFF00) >> 8) << 2) )

/* Port-base Multicast Domain Route Table Entry: 
   0xE10400 + 0x1000 * PortNo + DestId * 4
*/
#define CPS_PORT_BASE_MC_DOMAIN_RT_ENTRY(port, dest_id) \
            ( CPS_PORT_BASED_DOMAIN_ROUTE_TABLE + \
              (((uint32_t)port & 0x1F) << 12) + \
              ((((uint32_t)dest_id & 0xFF00) >> 8) << 2) )

/* Broadcast Unicast Device Route Table Entry: 0xE00000 + DestId * 4
*/
#define CPS_BROADCAST_UC_DEVICE_RT_ENTRY(dest_id) \
            ( CPS_BROADCAST_DEVICE_ROUTE_TABLE + \
              (((uint32_t)dest_id & 0xFF) << 2) )

/* Broadcast Multicast Device Route Table Entry: 0xE00000 + DestId * 4
*/
#define CPS_BROADCAST_MC_DEVICE_RT_ENTRY(dest_id) \
            ( CPS_BROADCAST_DEVICE_ROUTE_TABLE + \
              (((uint32_t)dest_id & 0xFF) << 2) )

/* Port-base Unicast Device Route Table Entry:
   0xE10000 + 0x1000 * PortNo + DestId * 4
*/
#define CPS_PORT_BASE_UC_DEVICE_RT_ENTRY(port, dest_id) \
            ( CPS_PORT_BASED_DEVICE_ROUTE_TABLE + \
              (((uint32_t)port & 0x1F) << 12) + \
              (((uint32_t)dest_id & 0xFF) << 2))

/* Port-base Multicast Device Route Table Entry:
   0xE10000 + 0x1000 * PortNo + DestId * 4
*/
#define CPS_PORT_BASE_MC_DEVICE_RT_ENTRY(port, dest_id) \
            ( CPS_PORT_BASED_DEVICE_ROUTE_TABLE + \
              (((uint32_t)port & 0x1F) << 12) + \
              (((uint32_t)dest_id & 0xFF) << 2))

/* Validate Default Route Value
*/
#define IS_CPS_VALID_DEFAULT_ROUTE(dflt_route) \
            ( ((uint8_t)dflt_route == (uint8_t)RIO_DSF_RT_USE_DEVICE_TABLE) || \
              ((uint8_t)dflt_route == (uint8_t)RIO_DSF_RT_USE_DEFAULT_ROUTE) || \
              ((uint8_t)dflt_route == (uint8_t)RIO_DSF_RT_NO_ROUTE) )

/* Validate Default Route Table_Port
*/
#define IS_CPS_VALID_DEFAULT_ROUTE_TABLE_PORT(dflt_route_table_port) \
            ( (uint8_t)dflt_route_table_port == \
              (uint8_t)RIO_DSF_RT_USE_DEFAULT_ROUTE )

/* Validate Port-based Default Route Value
*/
#define CPS_IS_VALID_RT_PORT(route_port, swportinfo) \
            ( (uint8_t)route_port < \
                  (uint8_t)RIO_AVAIL_PORTS((uint32_t)swportinfo) )

/* Broadcast Multicast Mask Entry: 0xF30000 + MaskNo * 4
*/
#define CPS_BROADCAST_MC_MASK_ENTRY(maskno) \
            ( CPS_BROADCAST_MC_MASK_BASE + ((uint32_t)maskno << 2) )

/* Port-base Multicast Mask Entry: 0xF30000 + MaskNo * 4
*/
#define CPS_PORT_BASE_MC_MASK_ENTRY(port, maskno) \
            ( CPS_PORT_N_MC_MASK_BASE + \
              ((uint32_t)port << 8) + ((uint32_t)maskno << 2) )

/* Multicast route port
*/
#define CPS_MC_BASE_PORT (0x40)
#define CPS_MC_PORT(maskno) ((uint8_t)(maskno + CPS_MC_BASE_PORT))

/* Check Multicast route port
*/
#define IS_CPS_MC_PORT(route_port) \
            ( (route_port >= CPS_MC_BASE_PORT) && \
              (route_port < (CPS_MC_BASE_PORT + CPS_MAX_MC_MASK)))

/* Get Multicast mask no
*/
#define IS_CPS_MC_MASK_NO(route_port) ((uint8_t)(route_port-CPS_MC_BASE_PORT))

/* Validate Route Port Value
*/
#define IS_CPS_VALID_ROUTE_PORT(route_port, dev_info) \
            ( ( (uint8_t)route_port < \
                RIO_AVAIL_PORTS((uint32_t)dev_info->swPortInfo) ) || \
              ( ( (uint8_t)route_port & (uint8_t)CPS_MC_BASE_PORT ) && \
                ( ( (uint8_t)route_port & \
                    (uint8_t)(~((uint8_t)CPS_MC_BASE_PORT)) ) < \
  			      (uint8_t)RIO_MAX_MC_NO( \
                      (uint32_t)dev_info->swMcastInfo, \
                      RIO_AVAIL_PORTS((uint32_t)dev_info->swPortInfo) ) ) ) || \
 			  ((uint8_t)route_port == 0xDD) || \
 			  ((uint8_t)route_port == 0xDE) || \
 			  ((uint8_t)route_port == 0xDF) )

/**************************************************/
/* CPS1848 : Register Bit Masks and Reset Values  */
/*           definitions for every register       */
/**************************************************/

/* CPS1848_DEV_IDENT_CAR : Register Bits Masks Definitions */
#define CPS1848_DEV_IDENT_CAR_VENDOR                               (0x0000ffff)
#define CPS1848_DEV_IDENT_CAR_DEVICE                               (0xffff0000)

/* CPS1848_DEV_INF_CAR : Register Bits Masks Definitions */
#define CPS1848_DEV_INF_CAR_JTAG_REV                               (0x0000000f)
#define CPS1848_DEV_INF_CAR_MINOR_REV                              (0x001f0000)
#define CPS1848_DEV_INF_CAR_MAJOR_REV                              (0x00e00000)

/* CPS1848_ASSY_IDENT_CAR : Register Bits Masks Definitions */
#define CPS1848_ASSY_IDENT_CAR_VENDOR                              (0x0000ffff)
#define CPS1848_ASSY_IDENT_CAR_ASSY                                (0xffff0000)

/* CPS1848_ASSY_INF_CAR : Register Bits Masks Definitions */
#define CPS1848_ASSY_INF_CAR_EF_PTR                                (0x0000ffff)
#define CPS1848_ASSY_INF_CAR_ASSY_REV                              (0xffff0000)

/* CPS1848_PROC_ELEM_FEAT_CAR : Register Bits Masks Definitions */
#define CPS1848_PROC_ELEM_FEAT_CAR_EXT_ADDR                        (0x00000007)
#define CPS1848_PROC_ELEM_FEAT_CAR_EXT_FEAT                        (0x00000008)
#define CPS1848_PROC_ELEM_FEAT_CAR_CTLS                            (0x00000010)
#define CPS1848_PROC_ELEM_FEAT_CAR_CRF                             (0x00000020)
#define CPS1848_PROC_ELEM_FEAT_CAR_CRC_ERR_REC                     (0x00000040)
#define CPS1848_PROC_ELEM_FEAT_CAR_FLOW_CTL                        (0x00000080)
#define CPS1848_PROC_ELEM_FEAT_CAR_STD_RTE                         (0x00000100)
#define CPS1848_PROC_ELEM_FEAT_CAR_EXTD_RTE                        (0x00000200)
#define CPS1848_PROC_ELEM_FEAT_CAR_MCAST                           (0x00000400)
#define CPS1848_PROC_ELEM_FEAT_CAR_FLOW_ARB                        (0x00000800)
#define CPS1848_PROC_ELEM_FEAT_CAR_MULTIPORT                       (0x08000000)
#define CPS1848_PROC_ELEM_FEAT_CAR_SWITCH                          (0x10000000)
#define CPS1848_PROC_ELEM_FEAT_CAR_PROC                            (0x20000000)
#define CPS1848_PROC_ELEM_FEAT_CAR_MEM                             (0x40000000)
#define CPS1848_PROC_ELEM_FEAT_CAR_BRIDGE                          (0x80000000)

/* CPS1848_SWITCH_PORT_INF_CAR : Register Bits Masks Definitions */
#define CPS1848_SWITCH_PORT_INF_CAR_PORT                           (0x000000ff)
#define CPS1848_SWITCH_PORT_INF_CAR_TOTAL                          (0x0000ff00)

/* CPS1848_SRC_OPS_CAR : Register Bits Masks Definitions */
#define CPS1848_SRC_OPS_CAR_PW                                     (0x00000004)
#define CPS1848_SRC_OPS_CAR_ATOMIC_CLR                             (0x00000010)
#define CPS1848_SRC_OPS_CAR_ATOMIC_SET                             (0x00000020)
#define CPS1848_SRC_OPS_CAR_ATOMIC_DECR                            (0x00000040)
#define CPS1848_SRC_OPS_CAR_ATOMIC_INCR                            (0x00000080)
#define CPS1848_SRC_OPS_CAR_ATOMIC_TEST_AND_SWAP                   (0x00000100)
#define CPS1848_SRC_OPS_CAR_DOORBELL                               (0x00000400)
#define CPS1848_SRC_OPS_CAR_MSG                                    (0x00000800)
#define CPS1848_SRC_OPS_CAR_NWRITE_R                               (0x00001000)
#define CPS1848_SRC_OPS_CAR_SWRITE                                 (0x00002000)
#define CPS1848_SRC_OPS_CAR_NWRITE                                 (0x00004000)
#define CPS1848_SRC_OPS_CAR_NREAD                                  (0x00008000)

/* CPS1848_SWITCH_MCAST_SUP_CAR : Register Bits Masks Definitions */
#define CPS1848_SWITCH_MCAST_SUP_CAR_SIMPLE                        (0x80000000)

/* CPS1848_SWITCH_RTE_TBL_LIM_CAR : Register Bits Masks Definitions */
#define CPS1848_SWITCH_RTE_TBL_LIM_CAR_MAX_DESTID                  (0x0000ffff)

/* CPS1848_SWITCH_MULT_INF_CAR : Register Bits Masks Definitions */
#define CPS1848_SWITCH_MULT_INF_CAR_MCAST_MASK                     (0x0000ffff)
#define CPS1848_SWITCH_MULT_INF_CAR_MAX_DESTID                     (0x3fff0000)
#define CPS1848_SWITCH_MULT_INF_CAR_PER_PORT                       (0x40000000)
#define CPS1848_SWITCH_MULT_INF_CAR_BLK                            (0x80000000)

/* CPS1848_HOST_BASE_DEVICEID_LOCK_CSR : Register Bits Masks Definitions */
#define CPS1848_HOST_BASE_DEVICEID_LOCK_CSR_HOST_BASE_DEVICEID     (0x0000ffff)

/* CPS1848_COMP_TAG_CSR : Register Bits Masks Definitions */
#define CPS1848_COMP_TAG_CSR_CTAG                                  (0xffffffff)

/* CPS1848_RTE_DESTID_CSR : Register Bits Masks Definitions */
#define CPS1848_RTE_DESTID_CSR_DESTID_LSB                          (0x000000ff)
#define CPS1848_RTE_DESTID_CSR_DESTID_MSB                          (0x0000ff00)
#define CPS1848_RTE_DESTID_CSR_EXTD_EN                             (0x80000000)

/* CPS1848_RTE_PORT_CSR : Register Bits Masks Definitions */
#define CPS1848_RTE_PORT_CSR_PORT                                  (0x000000ff)
#define CPS1848_RTE_PORT_CSR_PORT_1                                (0x0000ff00)
#define CPS1848_RTE_PORT_CSR_PORT_2                                (0x00ff0000)
#define CPS1848_RTE_PORT_CSR_PORT_3                                (0xff000000)

/* CPS1848_RTE_DEFAULT_PORT_CSR : Register Bits Masks Definitions */
#define CPS1848_RTE_DEFAULT_PORT_CSR_DEFAULT_PORT                  (0x000000ff)

/* CPS1848_MCAST_MASK_PORT_CSR : Register Bits Masks Definitions */
#define CPS1848_MCAST_MASK_PORT_CSR_PORT_STATUS                    (0x00000001)
#define CPS1848_MCAST_MASK_PORT_CSR_MASK_CMD                       (0x00000070)
#define CPS1848_MCAST_MASK_PORT_CSR_EGRESS_PORT                    (0x0000ff00)
#define CPS1848_MCAST_MASK_PORT_CSR_MCAST_MASK                     (0xffff0000)

/* CPS1848_MCAST_ASSOC_SEL_CSR : Register Bits Masks Definitions */
#define CPS1848_MCAST_ASSOC_SEL_CSR_MASK                           (0x0000ffff)
#define CPS1848_MCAST_ASSOC_SEL_CSR_DESTID_LSB                     (0x00ff0000)
#define CPS1848_MCAST_ASSOC_SEL_CSR_DESTID_MSB                     (0xff000000)

/* CPS1848_MCAST_ASSOC_OP_CSR : Register Bits Masks Definitions */
#define CPS1848_MCAST_ASSOC_OP_CSR_STATUS                          (0x00000001)
#define CPS1848_MCAST_ASSOC_OP_CSR_CMD                             (0x00000060)
#define CPS1848_MCAST_ASSOC_OP_CSR_TYPE                            (0x00000080)

/* CPS1848_PORT_MAINT_BLK_HEAD : Register Bits Masks Definitions */
#define CPS1848_PORT_MAINT_BLK_HEAD_EF_ID                          (0x0000ffff)
#define CPS1848_PORT_MAINT_BLK_HEAD_EF_PTR                         (0xffff0000)

/* CPS1848_PORT_LINK_TO_CTL_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_LINK_TO_CTL_CSR_TIMEOUT                       (0xffffff00)

/* CPS1848_PORT_GEN_CTL_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_GEN_CTL_CSR_DISCV                             (0x20000000)

/* CPS1848_PORT_X_LINK_MAINT_REQ_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_LINK_MAINT_REQ_CSR_CMD                      (0x00000007)

/* CPS1848_PORT_X_LINK_MAINT_RESP_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_LINK_MAINT_RESP_CSR_LINK_STATUS             (0x0000001f)
#define CPS1848_PORT_X_LINK_MAINT_RESP_CSR_ACKID_STATUS            (0x000007e0)
#define CPS1848_PORT_X_LINK_MAINT_RESP_CSR_VALID                   (0x80000000)

/* CPS1848_PORT_X_LOCAL_ACKID_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_LOCAL_ACKID_CSR_OUTBOUND                    (0x0000003f)
#define CPS1848_PORT_X_LOCAL_ACKID_CSR_OUTSTD                      (0x00003f00)
#define CPS1848_PORT_X_LOCAL_ACKID_CSR_INBOUND                     (0x3f000000)
#define CPS1848_PORT_X_LOCAL_ACKID_CSR_CLR                         (0x80000000)

/* CPS1848_PORT_X_CTL_2_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_CTL_2_CSR_SCRAM_DIS                         (0x00000004)
#define CPS1848_PORT_X_CTL_2_CSR_INACT_LANES_EN                    (0x00000008)
#define CPS1848_PORT_X_CTL_2_CSR_AUTOBAUD                          (0x08000000)

/* CPS1848_PORT_X_ERR_STAT_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_ERR_STAT_CSR_PORT_UNINIT                    (0x00000001)
#define CPS1848_PORT_X_ERR_STAT_CSR_PORT_OK                        (0x00000002)
#define CPS1848_PORT_X_ERR_STAT_CSR_PORT_ERR                       (0x00000004)
#define CPS1848_PORT_X_ERR_STAT_CSR_PORT_UNAVL                     (0x00000008)
#define CPS1848_PORT_X_ERR_STAT_CSR_PW_PNDG                        (0x00000010)
#define CPS1848_PORT_X_ERR_STAT_CSR_INPUT_ERR_STOP                 (0x00000100)
#define CPS1848_PORT_X_ERR_STAT_CSR_INPUT_ERR                      (0x00000200)
#define CPS1848_PORT_X_ERR_STAT_CSR_INPUT_RETRY_STOP               (0x00000400)
#define CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_ERR_STOP                (0x00010000)
#define CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_ERR                     (0x00020000)
#define CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_RETRY_STOP              (0x00040000)
#define CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_RETRIED                 (0x00080000)
#define CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_RETRY                   (0x00100000)
#define CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_DEGR                    (0x01000000)
#define CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_FAIL                    (0x02000000)
#define CPS1848_PORT_X_ERR_STAT_CSR_OUTPUT_DROP                    (0x04000000)
#define CPS1848_PORT_X_ERR_STAT_CSR_IDLE_SEQ                       (0x20000000)
#define CPS1848_PORT_X_ERR_STAT_CSR_IDLE2_EN                       (0x40000000)
#define CPS1848_PORT_X_ERR_STAT_CSR_IDLE2                          (0x80000000)

/* CPS1848_PORT_X_CTL_1_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_CTL_1_CSR_PORT_TYPE                         (0x00000001)
#define CPS1848_PORT_X_CTL_1_CSR_PORT_LOCKOUT                      (0x00000002)
#define CPS1848_PORT_X_CTL_1_CSR_DROP_PKT_EN                       (0x00000004)
#define CPS1848_PORT_X_CTL_1_CSR_STOP_ON_PORT_FAIL_ENC_EN          (0x00000008)
#define CPS1848_PORT_X_CTL_1_CSR_ERR_MASK                          (0x00000ff0)
#define CPS1848_PORT_X_CTL_1_CSR_ENUM_B                            (0x00020000)
#define CPS1848_PORT_X_CTL_1_CSR_MCAST_CS                          (0x00080000)
#define CPS1848_PORT_X_CTL_1_CSR_ERR_CHK_DIS                       (0x00100000)
#define CPS1848_PORT_X_CTL_1_CSR_INPUT_PORT_EN                     (0x00200000)
#define CPS1848_PORT_X_CTL_1_CSR_OUTPUT_PORT_EN                    (0x00400000)
#define CPS1848_PORT_X_CTL_1_CSR_PORT_DIS                          (0x00800000)
#define CPS1848_PORT_X_CTL_1_CSR_PWIDTH_OVRD                       (0x07000000)
#define CPS1848_PORT_X_CTL_1_CSR_INIT_PWIDTH                       (0x38000000)
#define CPS1848_PORT_X_CTL_1_CSR_PWIDTH                            (0xc0000000)

/* CPS1848_VC_REGISTER_BLK_HEAD : Register Bits Masks Definitions */
#define CPS1848_VC_REGISTER_BLK_HEAD_EF_ID                         (0x0000ffff)
#define CPS1848_VC_REGISTER_BLK_HEAD_EF_PTR                        (0xffff0000)

/* CPS1848_ERR_MGT_EXTENSION_BLK_HEAD : Register Bits Masks Definitions */
#define CPS1848_ERR_MGT_EXTENSION_BLK_HEAD_EF_ID                   (0x0000ffff)
#define CPS1848_ERR_MGT_EXTENSION_BLK_HEAD_EF_PTR                  (0xffff0000)

/* CPS1848_LT_ERR_DET_CSR : Register Bits Masks Definitions */
#define CPS1848_LT_ERR_DET_CSR_IMP_SPEC_ERR                        (0x00000001)
#define CPS1848_LT_ERR_DET_CSR_UNSUP_TRAN                          (0x00400000)
#define CPS1848_LT_ERR_DET_CSR_UNSOL_RESP                          (0x00800000)
#define CPS1848_LT_ERR_DET_CSR_ILL_TRAN                            (0x08000000)

/* CPS1848_LT_ERR_EN_CSR : Register Bits Masks Definitions */
#define CPS1848_LT_ERR_EN_CSR_IMP_SPEC_ERR_EN                      (0x00000001)
#define CPS1848_LT_ERR_EN_CSR_UNSUP_TRAN_EN                        (0x00400000)
#define CPS1848_LT_ERR_EN_CSR_UNSOL_RESP_EN                        (0x00800000)
#define CPS1848_LT_ERR_EN_CSR_ILL_TRAN_EN                          (0x08000000)

/* CPS1848_LT_DEVICEID_CAPT_CSR : Register Bits Masks Definitions */
#define CPS1848_LT_DEVICEID_CAPT_CSR_SOURCEID                      (0x000000ff)
#define CPS1848_LT_DEVICEID_CAPT_CSR_SOURCEID_MSB                  (0x0000ff00)
#define CPS1848_LT_DEVICEID_CAPT_CSR_DESTID_LSB                    (0x00ff0000)
#define CPS1848_LT_DEVICEID_CAPT_CSR_DESTID_MSB                    (0xff000000)

/* CPS1848_LT_CTL_CAPT_CSR : Register Bits Masks Definitions */
#define CPS1848_LT_CTL_CAPT_CSR_IMP_SPEC                           (0x000000ff)
#define CPS1848_LT_CTL_CAPT_CSR_TTYPE                              (0x0f000000)
#define CPS1848_LT_CTL_CAPT_CSR_FTYPE                              (0xf0000000)

/* CPS1848_PW_TARGET_DEVICEID_CSR : Register Bits Masks Definitions */
#define CPS1848_PW_TARGET_DEVICEID_CSR_LARGE                       (0x00008000)
#define CPS1848_PW_TARGET_DEVICEID_CSR_DESTID                      (0x00ff0000)
#define CPS1848_PW_TARGET_DEVICEID_CSR_DESTID_MSB                  (0xff000000)

/* CPS1848_PKT_TTL_CSR : Register Bits Masks Definitions */
#define CPS1848_PKT_TTL_CSR_TTL                                    (0xffff0000)

/* CPS1848_PORT_X_ERR_DET_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_ERR_DET_CSR_LINK_TIMEOUT                    (0x00000001)
#define CPS1848_PORT_X_ERR_DET_CSR_CS_ACK_ILL                      (0x00000002)
#define CPS1848_PORT_X_ERR_DET_CSR_DELIN_ERR                       (0x00000004)
#define CPS1848_PORT_X_ERR_DET_CSR_PRTCL_ERR                       (0x00000010)
#define CPS1848_PORT_X_ERR_DET_CSR_LR_ACKID_ILL                    (0x00000020)
#define CPS1848_PORT_X_ERR_DET_CSR_IDLE1_ERR                       (0x00008000)
#define CPS1848_PORT_X_ERR_DET_CSR_PKT_ILL_SIZE                    (0x00020000)
#define CPS1848_PORT_X_ERR_DET_CSR_PKT_CRC_ERR                     (0x00040000)
#define CPS1848_PORT_X_ERR_DET_CSR_PKT_ILL_ACKID                   (0x00080000)
#define CPS1848_PORT_X_ERR_DET_CSR_CS_NOT_ACC                      (0x00100000)
#define CPS1848_PORT_X_ERR_DET_CSR_UNEXP_ACKID                     (0x00200000)
#define CPS1848_PORT_X_ERR_DET_CSR_CS_CRC_ERR                      (0x00400000)
#define CPS1848_PORT_X_ERR_DET_CSR_IMP_SPEC_ERR                    (0x80000000)

/* CPS1848_PORT_X_ERR_RATE_EN_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_ERR_RATE_EN_CSR_LINK_TIMEOUT_EN             (0x00000001)
#define CPS1848_PORT_X_ERR_RATE_EN_CSR_CS_ACK_ILL_EN               (0x00000002)
#define CPS1848_PORT_X_ERR_RATE_EN_CSR_DELIN_ERR_EN                (0x00000004)
#define CPS1848_PORT_X_ERR_RATE_EN_CSR_PRTCL_ERR_EN                (0x00000010)
#define CPS1848_PORT_X_ERR_RATE_EN_CSR_LR_ACKID_ILL_EN             (0x00000020)
#define CPS1848_PORT_X_ERR_RATE_EN_CSR_IDLE1_ERR_EN                (0x00008000)
#define CPS1848_PORT_X_ERR_RATE_EN_CSR_PKT_ILL_SIZE_EN             (0x00020000)
#define CPS1848_PORT_X_ERR_RATE_EN_CSR_PKT_CRC_ERR_EN              (0x00040000)
#define CPS1848_PORT_X_ERR_RATE_EN_CSR_PKT_ILL_ACKID_EN            (0x00080000)
#define CPS1848_PORT_X_ERR_RATE_EN_CSR_CS_NOT_ACC_EN               (0x00100000)
#define CPS1848_PORT_X_ERR_RATE_EN_CSR_UNEXP_ACKID_EN              (0x00200000)
#define CPS1848_PORT_X_ERR_RATE_EN_CSR_CS_CRC_ERR_EN               (0x00400000)
#define CPS1848_PORT_X_ERR_RATE_EN_CSR_IMP_SPEC_ERR_EN             (0x80000000)

/* CPS1848_PORT_X_ATTR_CAPT_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_ATTR_CAPT_CSR_VALID                         (0x00000001)
#define CPS1848_PORT_X_ATTR_CAPT_CSR_IMP_DEP                       (0x000001f0)
#define CPS1848_PORT_X_ATTR_CAPT_CSR_ERR_TYPE                      (0x1f000000)
#define CPS1848_PORT_X_ATTR_CAPT_CSR_INFO_TYPE                     (0xe0000000)

/* CPS1848_PORT_X_CAPT_0_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_CAPT_0_CSR_CAPT_0                           (0xffffffff)

/* CPS1848_PORT_X_CAPT_1_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_CAPT_1_CSR_CAPT_1                           (0xffffffff)

/* CPS1848_PORT_X_CAPT_2_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_CAPT_2_CSR_CAPT_2                           (0xffffffff)

/* CPS1848_PORT_X_CAPT_3_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_CAPT_3_CSR_CAPT_3                           (0xffffffff)

/* CPS1848_PORT_X_ERR_RATE_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_ERR_RATE_CSR_ERR_RATE_CNTR                  (0x000000ff)
#define CPS1848_PORT_X_ERR_RATE_CSR_PEAK_ERR_RATE                  (0x0000ff00)
#define CPS1848_PORT_X_ERR_RATE_CSR_ERR_RATE_REC                   (0x00030000)
#define CPS1848_PORT_X_ERR_RATE_CSR_ERR_RATE_BIAS                  (0xff000000)

/* CPS1848_PORT_X_ERR_RATE_THRESH_CSR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_ERR_RATE_THRESH_CSR_DEGR_THRESH             (0x00ff0000)
#define CPS1848_PORT_X_ERR_RATE_THRESH_CSR_FAIL_THRESH             (0xff000000)

/* CPS1848_LANE_STATUS_BLK_HEAD : Register Bits Masks Definitions */
#define CPS1848_LANE_STATUS_BLK_HEAD_EF_ID                         (0x0000ffff)
#define CPS1848_LANE_STATUS_BLK_HEAD_EF_PTR                        (0xffff0000)

/* CPS1848_LANE_X_STATUS_0_CSR : Register Bits Masks Definitions */
#define CPS1848_LANE_X_STATUS_0_CSR_STATUS_CSR                     (0x00000007)
#define CPS1848_LANE_X_STATUS_0_CSR_STATUS_1                       (0x00000008)
#define CPS1848_LANE_X_STATUS_0_CSR_RX_TRAINED_CHG                 (0x00000020)
#define CPS1848_LANE_X_STATUS_0_CSR_RX_SYNC_CHG                    (0x00000040)
#define CPS1848_LANE_X_STATUS_0_CSR_ERR_8B10B                      (0x00000780)
#define CPS1848_LANE_X_STATUS_0_CSR_RX_LANE_RDY                    (0x00000800)
#define CPS1848_LANE_X_STATUS_0_CSR_LP_RX_TRAINED                  (0x00001000)
#define CPS1848_LANE_X_STATUS_0_CSR_RX_LANE_SYNC                   (0x00002000)
#define CPS1848_LANE_X_STATUS_0_CSR_RX_TRAINED                     (0x00004000)
#define CPS1848_LANE_X_STATUS_0_CSR_RX_INVERT                      (0x00008000)
#define CPS1848_LANE_X_STATUS_0_CSR_RX_TYPE                        (0x00030000)
#define CPS1848_LANE_X_STATUS_0_CSR_TX_MODE                        (0x00040000)
#define CPS1848_LANE_X_STATUS_0_CSR_TX_TYPE                        (0x00080000)
#define CPS1848_LANE_X_STATUS_0_CSR_LANE                           (0x00f00000)
#define CPS1848_LANE_X_STATUS_0_CSR_PORT                           (0xff000000)

/* CPS1848_LANE_X_STATUS_1_CSR : Register Bits Masks Definitions */
#define CPS1848_LANE_X_STATUS_1_CSR_LP_SCRAM                       (0x00008000)
#define CPS1848_LANE_X_STATUS_1_CSR_LP_POS1_TAP                    (0x00030000)
#define CPS1848_LANE_X_STATUS_1_CSR_LP_NEG1_TAP                    (0x000c0000)
#define CPS1848_LANE_X_STATUS_1_CSR_LP_LANE                        (0x00f00000)
#define CPS1848_LANE_X_STATUS_1_CSR_LP_PORT_WIDTH                  (0x07000000)
#define CPS1848_LANE_X_STATUS_1_CSR_LP_TRAINED                     (0x08000000)
#define CPS1848_LANE_X_STATUS_1_CSR_LP_RX_TYPE                     (0x10000000)
#define CPS1848_LANE_X_STATUS_1_CSR_VALUES_CHG                     (0x20000000)
#define CPS1848_LANE_X_STATUS_1_CSR_CURRENT                        (0x40000000)
#define CPS1848_LANE_X_STATUS_1_CSR_IDLE2_RX                       (0x80000000)

/* CPS1848_LANE_X_STATUS_2_CSR : Register Bits Masks Definitions */
#define CPS1848_LANE_X_STATUS_2_CSR_POS1_ON_RST                    (0x0000003f)
#define CPS1848_LANE_X_STATUS_2_CSR_NEG1_ON_RST                    (0x00001f00)
#define CPS1848_LANE_X_STATUS_2_CSR_POS1_ON_PRE                    (0x001f8000)
#define CPS1848_LANE_X_STATUS_2_CSR_NEG1_ON_PRE                    (0x0f800000)

/* CPS1848_LANE_X_STATUS_3_CSR : Register Bits Masks Definitions */
#define CPS1848_LANE_X_STATUS_3_CSR_POS1_TAP                       (0x0000003f)
#define CPS1848_LANE_X_STATUS_3_CSR_NEG1_TAP                       (0x000007c0)
#define CPS1848_LANE_X_STATUS_3_CSR_POS1_CMD                       (0x00007800)
#define CPS1848_LANE_X_STATUS_3_CSR_NEG1_CMD                       (0x00078000)
#define CPS1848_LANE_X_STATUS_3_CSR_GBAUD_6P25_EN                  (0x00080000)
#define CPS1848_LANE_X_STATUS_3_CSR_GBAUD_5_EN                     (0x00100000)
#define CPS1848_LANE_X_STATUS_3_CSR_GBAUD_3P125_EN                 (0x00200000)
#define CPS1848_LANE_X_STATUS_3_CSR_GBAUD_2P5_EN                   (0x00400000)
#define CPS1848_LANE_X_STATUS_3_CSR_GBAUD_1P25_EN                  (0x00800000)
#define CPS1848_LANE_X_STATUS_3_CSR_GBAUD_6P25                     (0x01000000)
#define CPS1848_LANE_X_STATUS_3_CSR_GBAUD_5                        (0x02000000)
#define CPS1848_LANE_X_STATUS_3_CSR_GBAUD_3P125                    (0x04000000)
#define CPS1848_LANE_X_STATUS_3_CSR_GBAUD_2P5                      (0x08000000)
#define CPS1848_LANE_X_STATUS_3_CSR_GBAUD_1P25                     (0x10000000)
#define CPS1848_LANE_X_STATUS_3_CSR_AMP_PROG_EN                    (0x20000000)

/* CPS1848_LANE_X_STATUS_4_CSR : Register Bits Masks Definitions */
#define CPS1848_LANE_X_STATUS_4_CSR_CC_MONITOR_THRESH              (0x0000ffff)
#define CPS1848_LANE_X_STATUS_4_CSR_CC_MONITOR_EN                  (0x00010000)
#define CPS1848_LANE_X_STATUS_4_CSR_CC_MONITOR_STATUS              (0x00020000)
#define CPS1848_LANE_X_STATUS_4_CSR_CTL_BY_LP_EN                   (0x80000000)

#define CPS1848_LANE_X_STATUS_4_CSR_CC_MONITOR_THRESH_5K           (5000)
#define CPS1848_LANE_X_STATUS_4_CSR_CC_MONITOR_THRESH_10K          (10000)
#define CPS1848_LANE_X_STATUS_4_CSR_CC_MONITOR_THRESH_15K          (15000)

/* CPS1848_RTE_PORT_SEL : Register Bits Masks Definitions */
#define CPS1848_RTE_PORT_SEL_PORT                                  (0x0000001f)

/* CPS1848_MCAST_RTE_SEL : Register Bits Masks Definitions */
#define CPS1848_MCAST_RTE_SEL_PORT                                 (0x0000001f)

/* CPS1848_PORT_X_WM : Register Bits Masks Definitions */
#define CPS1848_PORT_X_WM_PRIO_0                                   (0x0000003f)
#define CPS1848_PORT_X_WM_PRIO_1                                   (0x00000fc0)
#define CPS1848_PORT_X_WM_PRIO_2                                   (0x0003f000)

/* CPS1848_BCAST_WM : Register Bits Masks Definitions */
#define CPS1848_BCAST_WM_PRIO_0                                    (0x0000003f)
#define CPS1848_BCAST_WM_PRIO_1                                    (0x00000fc0)
#define CPS1848_BCAST_WM_PRIO_2                                    (0x0003f000)

/* CPS1848_AUX_PORT_ERR_CAPT_EN : Register Bits Masks Definitions */
#define CPS1848_AUX_PORT_ERR_CAPT_EN_I2C_LENGTH_ERR_EN             (0x00000001)
#define CPS1848_AUX_PORT_ERR_CAPT_EN_I2C_ACK_ERR_EN                (0x00000002)
#define CPS1848_AUX_PORT_ERR_CAPT_EN_I2C_ADDR_ERR_EN               (0x00000004)
#define CPS1848_AUX_PORT_ERR_CAPT_EN_UNEXP_START_STOP_EN           (0x00000008)
#define CPS1848_AUX_PORT_ERR_CAPT_EN_I2C_CHKSUM_ERR_EN             (0x00000010)
#define CPS1848_AUX_PORT_ERR_CAPT_EN_JTAG_ERR_EN                   (0x00000020)

/* CPS1848_AUX_PORT_ERR_DET : Register Bits Masks Definitions */
#define CPS1848_AUX_PORT_ERR_DET_I2C_LENGTH_ERR                    (0x00000001)
#define CPS1848_AUX_PORT_ERR_DET_I2C_ACK_ERR                       (0x00000002)
#define CPS1848_AUX_PORT_ERR_DET_I2C_ADDR_ERR                      (0x00000004)
#define CPS1848_AUX_PORT_ERR_DET_UNEXP_START_STOP                  (0x00000008)
#define CPS1848_AUX_PORT_ERR_DET_I2C_CHKSUM_ERR                    (0x00000010)
#define CPS1848_AUX_PORT_ERR_DET_JTAG_ERR                          (0x00000020)

/* CPS1848_CFG_ERR_CAPT_EN : Register Bits Masks Definitions */
#define CPS1848_CFG_ERR_CAPT_EN_BAD_MASK_EN                        (0x00000001)
#define CPS1848_CFG_ERR_CAPT_EN_BAD_PORT_EN                        (0x00000002)
#define CPS1848_CFG_ERR_CAPT_EN_RTE_FORCE_EN                       (0x00000004)
#define CPS1848_CFG_ERR_CAPT_EN_BAD_RTE_EN                         (0x00000008)
#define CPS1848_CFG_ERR_CAPT_EN_BAD_MCAST_EN                       (0x00000010)

/* CPS1848_CFG_ERR_DET : Register Bits Masks Definitions */
#define CPS1848_CFG_ERR_DET_BAD_MASK                               (0x00000001)
#define CPS1848_CFG_ERR_DET_BAD_PORT                               (0x00000002)
#define CPS1848_CFG_ERR_DET_RTE_FORCE                              (0x00000004)
#define CPS1848_CFG_ERR_DET_BAD_RTE                                (0x00000008)
#define CPS1848_CFG_ERR_DET_BAD_MCAST                              (0x00000010)

/* CPS1848_IMPL_SPEC_LT_ADDR_CAPT : Register Bits Masks Definitions */
#define CPS1848_IMPL_SPEC_LT_ADDR_CAPT_LT_ADDR                     (0x003fffff)

/* CPS1848_LT_ERR_RPT_EN : Register Bits Masks Definitions */
#define CPS1848_LT_ERR_RPT_EN_IMP_SPEC_ERR_EN                      (0x000000ff)
#define CPS1848_LT_ERR_RPT_EN_UNSUP_TRAN_EN                        (0x00400000)
#define CPS1848_LT_ERR_RPT_EN_UNSOL_RESP_EN                        (0x00800000)
#define CPS1848_LT_ERR_RPT_EN_ILL_TRAN_EN                          (0x08000000)

/* CPS1848_PORT_X_ERR_RPT_EN : Register Bits Masks Definitions */
#define CPS1848_PORT_X_ERR_RPT_EN_LINK_TIMEOUT_EN                  (0x00000001)
#define CPS1848_PORT_X_ERR_RPT_EN_CS_ACK_ILL_EN                    (0x00000002)
#define CPS1848_PORT_X_ERR_RPT_EN_DELIN_ERR_EN                     (0x00000004)
#define CPS1848_PORT_X_ERR_RPT_EN_PRTCL_ERR_EN                     (0x00000010)
#define CPS1848_PORT_X_ERR_RPT_EN_LR_ACKID_ILL_EN                  (0x00000020)
#define CPS1848_PORT_X_ERR_RPT_EN_IDLE1_ERR_EN                     (0x00008000)
#define CPS1848_PORT_X_ERR_RPT_EN_PKT_ILL_SIZE_EN                  (0x00020000)
#define CPS1848_PORT_X_ERR_RPT_EN_PKT_CRC_ERR_EN                   (0x00040000)
#define CPS1848_PORT_X_ERR_RPT_EN_PKT_ILL_ACKID_EN                 (0x00080000)
#define CPS1848_PORT_X_ERR_RPT_EN_CS_NOT_ACC_EN                    (0x00100000)
#define CPS1848_PORT_X_ERR_RPT_EN_UNEXP_ACKID_EN                   (0x00200000)
#define CPS1848_PORT_X_ERR_RPT_EN_CS_CRC_ERR_EN                    (0x00400000)
#define CPS1848_PORT_X_ERR_RPT_EN_IMP_SPEC_ERR_EN                  (0x80000000)

/* CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN : Register Bits Masks Definitions */
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_REORDER_EN             (0x00000001)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_BAD_CTL_EN             (0x00000002)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_LOA_EN                 (0x00000004)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_IDLE_IN_PKT_EN         (0x00000008)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_PORT_WIDTH_EN          (0x00000010)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_PORT_INIT_EN           (0x00000020)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_UNEXP_STOMP_EN         (0x00000040)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_UNEXP_EOP_EN           (0x00000080)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_LR_X2_EN               (0x00000100)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_LR_CMD_EN              (0x00000200)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_RX_STOMP_EN            (0x00000400)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_STOMP_TO_EN            (0x00000800)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_RETRY_ACKID_EN         (0x00001000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_RETRY_EN               (0x00002000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_FATAL_TO_EN            (0x00004000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_UNSOL_RFR_EN           (0x00008000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_SHORT_EN               (0x00010000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_BAD_TT_EN              (0x00020000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_RX_DROP_EN             (0x00080000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_MANY_RETRY_EN          (0x00100000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_TX_DROP_EN             (0x00200000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_SET_ACKID_EN           (0x00400000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_RTE_ISSUE_EN           (0x01000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_PNA_RETRY_EN           (0x02000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_UNEXP_ACKID_EN         (0x04000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_UNSOL_LR_EN            (0x08000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_PNA_EN                 (0x10000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_CRC_EVENT_EN           (0x20000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_TTL_EVENT_EN           (0x40000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN_ERR_RATE_EN            (0x80000000)

/* CPS1848_LANE_X_ERR_RPT_EN : Register Bits Masks Definitions */
#define CPS1848_LANE_X_ERR_RPT_EN_LANE_SYNC_EN                     (0x00000001)
#define CPS1848_LANE_X_ERR_RPT_EN_LANE_RDY_EN                      (0x00000002)
#define CPS1848_LANE_X_ERR_RPT_EN_BAD_CHAR_EN                      (0x00000004)
#define CPS1848_LANE_X_ERR_RPT_EN_DESCRAM_SYNC_EN                  (0x00000008)
#define CPS1848_LANE_X_ERR_RPT_EN_TX_RX_MISMATCH_EN                (0x00000010)
#define CPS1848_LANE_X_ERR_RPT_EN_IDLE2_FRAME_EN                   (0x00000080)
#define CPS1848_LANE_X_ERR_RPT_EN_LANE_INVER_DET_EN                (0x00000100)
#define CPS1848_LANE_X_ERR_RPT_EN_BAD_SPEED_EN                     (0x00000200)
#define CPS1848_LANE_X_ERR_RPT_EN_UNUSED                           (0x00001c00)

/* CPS1848_BCAST_PORT_ERR_RPT_EN : Register Bits Masks Definitions */
#define CPS1848_BCAST_PORT_ERR_RPT_EN_LINK_TIMEOUT_EN              (0x00000001)
#define CPS1848_BCAST_PORT_ERR_RPT_EN_CS_ACK_ILL_EN                (0x00000002)
#define CPS1848_BCAST_PORT_ERR_RPT_EN_DELIN_ERR_EN                 (0x00000004)
#define CPS1848_BCAST_PORT_ERR_RPT_EN_PRTCL_ERR_EN                 (0x00000010)
#define CPS1848_BCAST_PORT_ERR_RPT_EN_LR_ACKID_ILL_EN              (0x00000020)
#define CPS1848_BCAST_PORT_ERR_RPT_EN_IDLE1_ERR_EN                 (0x00008000)
#define CPS1848_BCAST_PORT_ERR_RPT_EN_PKT_ILL_SIZE_EN              (0x00020000)
#define CPS1848_BCAST_PORT_ERR_RPT_EN_PKT_CRC_ERR_EN               (0x00040000)
#define CPS1848_BCAST_PORT_ERR_RPT_EN_PKT_ILL_ACKID_EN             (0x00080000)
#define CPS1848_BCAST_PORT_ERR_RPT_EN_CS_NOT_ACC_EN                (0x00100000)
#define CPS1848_BCAST_PORT_ERR_RPT_EN_UNEXP_ACKID_EN               (0x00200000)
#define CPS1848_BCAST_PORT_ERR_RPT_EN_CS_CRC_ERR_EN                (0x00400000)
#define CPS1848_BCAST_PORT_ERR_RPT_EN_IMP_SPEC_ERR_EN              (0x80000000)

/* CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN : Register Bits Masks Definitions */
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_REORDER_EN         (0x00000001)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_BAD_CTL_EN         (0x00000002)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_LOA_EN             (0x00000004)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_IDLE_IN_PKT_EN     (0x00000008)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_PORT_WIDTH_EN      (0x00000010)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_PORT_INIT_EN       (0x00000020)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_UNEXP_STOMP_EN     (0x00000040)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_UNEXP_EOP_EN       (0x00000080)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_LR_X2_EN           (0x00000100)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_LR_CMD_EN          (0x00000200)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_RX_STOMP_EN        (0x00000400)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_STOMP_TO_EN        (0x00000800)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_RETRY_ACKID_EN     (0x00001000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_RETRY_EN           (0x00002000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_FATAL_TO_EN        (0x00004000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_UNSOL_RFR_EN       (0x00008000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_SHORT_EN           (0x00010000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_BAD_TT_EN          (0x00020000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_RX_DROP_EN         (0x00080000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_MANY_RETRY_EN      (0x00100000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_TX_DROP_EN         (0x00200000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_SET_ACKID_EN       (0x00400000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_RTE_ISSUE_EN       (0x01000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_PNA_RETRY_EN       (0x02000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_UNEXP_ACKID_EN     (0x04000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_UNSOL_LR_EN        (0x08000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_PNA_EN             (0x10000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_CRC_EVENT_EN       (0x20000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_TTL_EVENT_EN       (0x40000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RPT_EN_ERR_RATE_EN        (0x80000000)

/* CPS1848_BCAST_LANE_ERR_RPT_EN : Register Bits Masks Definitions */
#define CPS1848_BCAST_LANE_ERR_RPT_EN_LANE_SYNC_EN                 (0x00000001)
#define CPS1848_BCAST_LANE_ERR_RPT_EN_LANE_RDY_EN                  (0x00000002)
#define CPS1848_BCAST_LANE_ERR_RPT_EN_BAD_CHAR_EN                  (0x00000004)
#define CPS1848_BCAST_LANE_ERR_RPT_EN_DESCRAM_SYNC_EN              (0x00000008)
#define CPS1848_BCAST_LANE_ERR_RPT_EN_TX_RX_MISMATCH_EN            (0x00000010)
#define CPS1848_BCAST_LANE_ERR_RPT_EN_IDLE2_FRAME_EN               (0x00000080)
#define CPS1848_BCAST_LANE_ERR_RPT_EN_LANE_INVER_DET_EN            (0x00000100)
#define CPS1848_BCAST_LANE_ERR_RPT_EN_BAD_SPEED_EN                 (0x00000200)
#define CPS1848_BCAST_LANE_ERR_RPT_EN_UNUSED                       (0x00001c00)

/* CPS1848_PORT_X_PGC_MODE : Register Bits Masks Definitions */
#define CPS1848_PORT_X_PGC_MODE_FLOW_TYPE                          (0x000003ff)
#define CPS1848_PORT_X_PGC_MODE_SOP                                (0x00000400)
#define CPS1848_PORT_X_PGC_MODE_EOP                                (0x00000800)
#define CPS1848_PORT_X_PGC_MODE_START_PORT                         (0x00001000)
#define CPS1848_PORT_X_PGC_MODE_END_PORT                           (0x00002000)
#define CPS1848_PORT_X_PGC_MODE_EN                                 (0x00004000)
#define CPS1848_PORT_X_PGC_MODE_RX_DONE                            (0x00008000)
#define CPS1848_PORT_X_PGC_MODE_START                              (0x00010000)

/* CPS1848_PORT_X_PGC_DATA : Register Bits Masks Definitions */
#define CPS1848_PORT_X_PGC_DATA_DATA                               (0xffffffff)

/* CPS1848_BCAST_DEV_RTE_TABLE_X : Register Bits Masks Definitions */
#define CPS1848_BCAST_DEV_RTE_TABLE_X_PORT                         (0x000000ff)

/* CPS1848_BCAST_DOM_RTE_TABLE_X : Register Bits Masks Definitions */
#define CPS1848_BCAST_DOM_RTE_TABLE_X_PORT                         (0x000000ff)

/* CPS1848_PORT_X_DEV_RTE_TABLE_Y : Register Bits Masks Definitions */
#define CPS1848_PORT_X_DEV_RTE_TABLE_Y_PORT                        (0x000000ff)

/* CPS1848_PORT_X_DOM_RTE_TABLE_Y : Register Bits Masks Definitions */
#define CPS1848_PORT_X_DOM_RTE_TABLE_Y_PORT                        (0x000000ff)

/* CPS1848_PORT_X_TRACE_0_VAL_0 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_0_VAL_0_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_0_VAL_1 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_0_VAL_1_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_0_VAL_2 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_0_VAL_2_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_0_VAL_3 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_0_VAL_3_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_0_VAL_4 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_0_VAL_4_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_0_MASK_0 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_0_MASK_0_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_0_MASK_1 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_0_MASK_1_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_0_MASK_2 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_0_MASK_2_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_0_MASK_3 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_0_MASK_3_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_0_MASK_4 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_0_MASK_4_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_1_VAL_0 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_1_VAL_0_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_1_VAL_1 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_1_VAL_1_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_1_VAL_2 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_1_VAL_2_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_1_VAL_3 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_1_VAL_3_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_1_VAL_4 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_1_VAL_4_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_1_MASK_0 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_1_MASK_0_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_1_MASK_1 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_1_MASK_1_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_1_MASK_2 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_1_MASK_2_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_1_MASK_3 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_1_MASK_3_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_1_MASK_4 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_1_MASK_4_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_2_VAL_0 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_2_VAL_0_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_2_VAL_1 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_2_VAL_1_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_2_VAL_2 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_2_VAL_2_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_2_VAL_3 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_2_VAL_3_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_2_VAL_4 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_2_VAL_4_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_2_MASK_0 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_2_MASK_0_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_2_MASK_1 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_2_MASK_1_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_2_MASK_2 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_2_MASK_2_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_2_MASK_3 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_2_MASK_3_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_2_MASK_4 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_2_MASK_4_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_3_VAL_0 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_3_VAL_0_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_3_VAL_1 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_3_VAL_1_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_3_VAL_2 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_3_VAL_2_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_3_VAL_3 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_3_VAL_3_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_3_VAL_4 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_3_VAL_4_VALUE                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_3_MASK_0 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_3_MASK_0_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_3_MASK_1 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_3_MASK_1_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_3_MASK_2 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_3_MASK_2_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_3_MASK_3 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_3_MASK_3_MASK                         (0xffffffff)

/* CPS1848_PORT_X_TRACE_3_MASK_4 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_3_MASK_4_MASK                         (0xffffffff)

/* CPS1848_BCAST_TRACE_0_VAL_0 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_0_VAL_0_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_0_VAL_1 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_0_VAL_1_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_VAL_1_BLK_2 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_VAL_1_BLK_2_VALUE                      (0xffffffff)

/* CPS1848_BCAST_TRACE_0_VAL_3 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_0_VAL_3_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_0_VAL_4 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_0_VAL_4_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_0_MASK_0 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_0_MASK_0_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_0_MASK_1 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_0_MASK_1_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_0_MASK_2 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_0_MASK_2_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_0_MASK_3 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_0_MASK_3_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_0_MASK_4 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_0_MASK_4_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_1_VAL_0 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_1_VAL_0_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_1_VAL_1 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_1_VAL_1_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_1_VAL_2 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_1_VAL_2_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_1_VAL_3 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_1_VAL_3_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_1_VAL_4 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_1_VAL_4_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_1_MASK_0 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_1_MASK_0_VALUE                         (0xffffffff)

/* CPS1848_BCAST_TRACE_1_MASK_1 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_1_MASK_1_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_1_MASK_2 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_1_MASK_2_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_1_MASK_3 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_1_MASK_3_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_1_MASK_4 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_1_MASK_4_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_2_VAL_0 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_2_VAL_0_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_2_VAL_1 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_2_VAL_1_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_2_VAL_2 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_2_VAL_2_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_2_VAL_3 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_2_VAL_3_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_2_VAL_4 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_2_VAL_4_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_2_MASK__0 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_2_MASK__0_MASK                         (0xffffffff)

/* CPS1848_BCAST_TRACE_2_MASK_1 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_2_MASK_1_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_2_MASK_2 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_2_MASK_2_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_2_MASK_3 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_2_MASK_3_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_2_MASK_4 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_2_MASK_4_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_3_VAL_0 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_3_VAL_0_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_3_VAL_1 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_3_VAL_1_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_3_VAL_2 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_3_VAL_2_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_3_VAL_3 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_3_VAL_3_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_3_VAL_4 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_3_VAL_4_VALUE                          (0xffffffff)

/* CPS1848_BCAST_TRACE_3_MASK_0 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_3_MASK_0_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_3_MASK_1 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_3_MASK_1_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_3_MASK_2 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_3_MASK_2_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_3_MASK_3 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_3_MASK_3_MASK                          (0xffffffff)

/* CPS1848_BCAST_TRACE_3_MASK_4 : Register Bits Masks Definitions */
#define CPS1848_BCAST_TRACE_3_MASK_4_MASK                          (0xffffffff)

/* CPS1848_DEVICE_CTL_1 : Register Bits Masks Definitions */
#define CPS1848_DEVICE_CTL_1_PORT_RST_CTL                          (0x00000001)
#define CPS1848_DEVICE_CTL_1_TRACE_OUT_PORT                        (0x0000003e)
#define CPS1848_DEVICE_CTL_1_CUT_THRU_EN                           (0x00001000)
#define CPS1848_DEVICE_CTL_1_CLK_RATE_CTL                          (0x00002000)
#define CPS1848_DEVICE_CTL_1_TRACE_EN                              (0x00008000)
#define CPS1848_DEVICE_CTL_1_TRACE_OUT_PORT_MODE                   (0x00010000)
#define CPS1848_DEVICE_CTL_1_LT_PW_EN                              (0x02000000)
#define CPS1848_DEVICE_CTL_1_LT_INT_EN                             (0x04000000)
#define CPS1848_DEVICE_CTL_1_PGC_EN                                (0x10000000)
#define CPS1848_DEVICE_CTL_1_EXT_MECS_EN                           (0x20000000)
#define CPS1848_DEVICE_CTL_1_FATAL_ERR_PKT_MGT                     (0x40000000)

/* CPS1848_CFG_BLK_ERR_RPT : Register Bits Masks Definitions */
#define CPS1848_CFG_BLK_ERR_RPT_CFG_LOG_EN                         (0x00000001)
#define CPS1848_CFG_BLK_ERR_RPT_CFG_PW_EN                          (0x00000002)
#define CPS1848_CFG_BLK_ERR_RPT_CFG_INT_EN                         (0x00000004)
#define CPS1848_CFG_BLK_ERR_RPT_CFG_PW_PEND                        (0x00000008)

/* CPS1848_AUX_PORT_ERR_RPT_EN : Register Bits Masks Definitions */
#define CPS1848_AUX_PORT_ERR_RPT_EN_JTAG_LOG_EN                    (0x00000001)
#define CPS1848_AUX_PORT_ERR_RPT_EN_I2C_LOG_EN                     (0x00000002)

/* CPS1848_RIO_DOMAIN : Register Bits Masks Definitions */
#define CPS1848_RIO_DOMAIN_DOMAIN                                  (0x000000ff)

/* CPS1848_PW_CTL : Register Bits Masks Definitions */
#define CPS1848_PW_CTL_CRF                                         (0x00002000)
#define CPS1848_PW_CTL_PRIO                                        (0x0000c000)
#define CPS1848_PW_CTL_SRCID                                       (0x00ff0000)
#define CPS1848_PW_CTL_SRCID_MSB                                   (0xff000000)

/* CPS1848_ASSY_IDENT_CAR_OVRD : Register Bits Masks Definitions */
#define CPS1848_ASSY_IDENT_CAR_OVRD_VENDOR                         (0x0000ffff)
#define CPS1848_ASSY_IDENT_CAR_OVRD_ASSY                           (0xffff0000)

/* CPS1848_ASSY_INF_CAR_OVRD : Register Bits Masks Definitions */
#define CPS1848_ASSY_INF_CAR_OVRD_ASSY_REV                         (0x0000ffff)

/* CPS1848_DEVICE_SOFT_RESET : Register Bits Masks Definitions */
#define CPS1848_DEVICE_SOFT_RESET_SOFT_RESET                       (0xffffffff)
#define CPS1848_DEVICE_SOFT_RESET_COMMAND                          (0x00030097)

/* CPS1848_I2C_MASTER_CTL : Register Bits Masks Definitions */
#define CPS1848_I2C_MASTER_CTL_EPROM_ADDR                          (0x000003ff)
#define CPS1848_I2C_MASTER_CTL_CHKSUM_DIS                          (0x00000800)
#define CPS1848_I2C_MASTER_CTL_CLK_DIV                             (0x007f0000)
#define CPS1848_I2C_MASTER_CTL_SPD_SEL                             (0x02000000)
#define CPS1848_I2C_MASTER_CTL_I2C_PW_EN                           (0x04000000)
#define CPS1848_I2C_MASTER_CTL_I2C_INT_EN                          (0x08000000)
#define CPS1848_I2C_MASTER_CTL_I2C_PW_PEND                         (0x10000000)

/* CPS1848_I2C_MASTER_STAT_CTL : Register Bits Masks Definitions */
#define CPS1848_I2C_MASTER_STAT_CTL_EPROM_START_ADDR               (0x0000ffff)
#define CPS1848_I2C_MASTER_STAT_CTL_START_READ                     (0x00010000)
#define CPS1848_I2C_MASTER_STAT_CTL_ABORT                          (0x00100000)
#define CPS1848_I2C_MASTER_STAT_CTL_SUCCESS                        (0x00200000)
#define CPS1848_I2C_MASTER_STAT_CTL_READING                        (0x00400000)
#define CPS1848_I2C_MASTER_STAT_CTL_CHKSUM_FAIL                    (0x00800000)
#define CPS1848_I2C_MASTER_STAT_CTL_WORD_ERR_32BIT                 (0x01000000)
#define CPS1848_I2C_MASTER_STAT_CTL_WORD_ERR_22BIT                 (0x02000000)
#define CPS1848_I2C_MASTER_STAT_CTL_NACK                           (0x04000000)
#define CPS1848_I2C_MASTER_STAT_CTL_UNEXP_START_STOP               (0x08000000)

/* CPS1848_JTAG_CTL : Register Bits Masks Definitions */
#define CPS1848_JTAG_CTL_JTAG_PW_PEND                              (0x00000001)
#define CPS1848_JTAG_CTL_JTAG_PW_EN                                (0x00000002)
#define CPS1848_JTAG_CTL_JTAG_INT_EN                               (0x00000004)

/* CPS1848_EXT_MECS_TRIG_CNTR : Register Bits Masks Definitions */
#define CPS1848_EXT_MECS_TRIG_CNTR_TRIG_CNT                        (0x000000ff)

/* CPS1848_MAINT_DROP_PKT_CNTR : Register Bits Masks Definitions */
#define CPS1848_MAINT_DROP_PKT_CNTR_COUNT                          (0xffffffff)

/* CPS1848_SWITCH_PARAM_1 : Register Bits Masks Definitions */
#define CPS1848_SWITCH_PARAM_1_INPUT_STARV_LIM                     (0x0000001f)
#define CPS1848_SWITCH_PARAM_1_OUTPUT_CREDIT_RSVN                  (0x00003fe0)
#define CPS1848_SWITCH_PARAM_1_ARB_MODE                            (0x0001c000)
#define CPS1848_SWITCH_PARAM_1_BUF_ALLOC                           (0x00020000)
#define CPS1848_SWITCH_PARAM_1_FB_ALLOC                            (0x001c0000)

#define CPS1848_SWITCH_PARAM_1_ARB_MODE_FAIR_AGING                 (0x00000000)
#define CPS1848_SWITCH_PARAM_1_ARB_MODE_RR_AGING                   (0x0000C000)
#define CPS1848_SWITCH_PARAM_1_ARB_MODE_RR_AGELESS                 (0x0001C000)

/* CPS1848_SWITCH_PARAM_2 : Register Bits Masks Definitions */
#define CPS1848_SWITCH_PARAM_2_OUTPUT_CREDIT_MAX                   (0x0000ffff)
#define CPS1848_SWITCH_PARAM_2_OUTPUT_CREDIT_MIN                   (0xffff0000)

/* CPS1848_QUAD_CFG : Register Bits Masks Definitions */
#define CPS1848_QUAD_CFG_QUAD0_CFG                                 (0x00000003)
#define CPS1848_QUAD_CFG_QUAD1_CFG                                 (0x0000000c)
#define CPS1848_QUAD_CFG_QUAD2_CFG                                 (0x00000030)
#define CPS1848_QUAD_CFG_QUAD3_CFG                                 (0x000000c0)

/* CPS1848_DEVICE_RESET_CTL : Register Bits Masks Definitions */
#define CPS1848_DEVICE_RESET_CTL_PORT_SEL                          (0x0003ffff)
#define CPS1848_DEVICE_RESET_CTL_PLL_SEL                           (0x3ffc0000)
#define CPS1848_DEVICE_RESET_CTL_RESET_TYPE                        (0x40000000)
#define CPS1848_DEVICE_RESET_CTL_DO_RESET                          (0x80000000)

/* CPS1848_BCAST__MCAST_MASK_X : Register Bits Masks Definitions */
#define CPS1848_BCAST__MCAST_MASK_X_PORT_MASK                      (0x0003ffff)

/* CPS1848_PORT_X_MCAST_MASK_Y : Register Bits Masks Definitions */
#define CPS1848_PORT_X_MCAST_MASK_Y_PORT_MASK                      (0x0003ffff)

/* CPS1848_PORT_X_OPS : Register Bits Masks Definitions */
#define CPS1848_PORT_X_OPS_CRC_RETX_LIMIT                          (0x0000000e)
#define CPS1848_PORT_X_OPS_LT_LOG_EN                               (0x00000010)
#define CPS1848_PORT_X_OPS_LANE_LOG_EN                             (0x00000020)
#define CPS1848_PORT_X_OPS_PORT_LOG_EN                             (0x00000040)
#define CPS1848_PORT_X_OPS_TRACE_PW_EN                             (0x00000080)
#define CPS1848_PORT_X_OPS_PORT_LPBK_EN                            (0x00000100)
#define CPS1848_PORT_X_OPS_TRACE_0_EN                              (0x00000200)
#define CPS1848_PORT_X_OPS_TRACE_1_EN                              (0x00000400)
#define CPS1848_PORT_X_OPS_TRACE_2_EN                              (0x00000800)
#define CPS1848_PORT_X_OPS_TRACE_3_EN                              (0x00001000)
#define CPS1848_PORT_X_OPS_FILTER_0_EN                             (0x00002000)
#define CPS1848_PORT_X_OPS_FILTER_1_EN                             (0x00004000)
#define CPS1848_PORT_X_OPS_FILTER_2_EN                             (0x00008000)
#define CPS1848_PORT_X_OPS_FILTER_3_EN                             (0x00010000)
#define CPS1848_PORT_X_OPS_SELF_MCAST_EN                           (0x00020000)
#define CPS1848_PORT_X_OPS_TX_FLOW_CTL_DIS                         (0x00080000)
#define CPS1848_PORT_X_OPS_FORCE_REINIT                            (0x00100000)
#define CPS1848_PORT_X_OPS_SILENCE_CTL                             (0x03c00000)
#define CPS1848_PORT_X_OPS_CNTRS_EN                                (0x04000000)
#define CPS1848_PORT_X_OPS_PORT_PW_EN                              (0x08000000)
#define CPS1848_PORT_X_OPS_PORT_INT_EN                             (0x10000000)
#define CPS1848_PORT_X_OPS_CRC_DIS                                 (0x20000000)

#define CPS1848_PORT_X_OPS_ANY_TRACE (CPS1848_PORT_X_OPS_TRACE_0_EN | \
		                      CPS1848_PORT_X_OPS_TRACE_1_EN | \
		                      CPS1848_PORT_X_OPS_TRACE_2_EN | \
		                      CPS1848_PORT_X_OPS_TRACE_3_EN )

#define CPS1848_PORT_X_OPS_ANY_FILTER (CPS1848_PORT_X_OPS_FILTER_0_EN | \
		                       CPS1848_PORT_X_OPS_FILTER_1_EN | \
		                       CPS1848_PORT_X_OPS_FILTER_2_EN | \
		                       CPS1848_PORT_X_OPS_FILTER_3_EN )

/* CPS1848_PORT_X_IMPL_SPEC_ERR_DET : Register Bits Masks Definitions */
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_REORDER                   (0x00000001)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_BAD_CTL                   (0x00000002)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_LOA                       (0x00000004)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_IDLE_IN_PKT               (0x00000008)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PORT_WIDTH                (0x00000010)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PORT_INIT                 (0x00000020)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_UNEXP_STOMP               (0x00000040)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_UNEXP_EOP                 (0x00000080)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_LR_X2                     (0x00000100)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_LR_CMD                    (0x00000200)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_RX_STOMP                  (0x00000400)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_STOMP_TO                  (0x00000800)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_RETRY_ACKID               (0x00001000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_RETRY                     (0x00002000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_FATAL_TO                  (0x00004000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_UNSOL_RFR                 (0x00008000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_SHORT                     (0x00010000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_BAD_TT                    (0x00020000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_RX_DROP                   (0x00080000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_MANY_RETRY                (0x00100000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_TX_DROP                   (0x00200000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_SET_ACKID                 (0x00400000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_RTE_ISSUE                 (0x01000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PNA_RETRY                 (0x02000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_UNEXP_ACKID               (0x04000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_UNSOL_LR                  (0x08000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_PNA                       (0x10000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_CRC_EVENT                 (0x20000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_TTL_EVENT                 (0x40000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_DET_ERR_RATE                  (0x80000000)

/* CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN : Register Bits Masks Definitions */
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_REORDER_EN            (0x00000001)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_BAD_CTL_EN            (0x00000002)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_LOA_EN                (0x00000004)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_IDLE_IN_PKT_EN        (0x00000008)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_PORT_WIDTH_EN         (0x00000010)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_PORT_INIT_EN          (0x00000020)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_UNEXP_STOMP_EN        (0x00000040)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_UNEXP_EOP_EN          (0x00000080)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_LR_X2_EN              (0x00000100)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_LR_CMD_EN             (0x00000200)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_RX_STOMP_EN           (0x00000400)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_STOMP_TO_EN           (0x00000800)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_RETRY_ACKID_EN        (0x00001000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_RETRY_EN              (0x00002000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_FATAL_TO_EN           (0x00004000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_UNSOL_RFR_EN          (0x00008000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_SHORT_EN              (0x00010000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_BAD_TT_EN             (0x00020000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_RX_DROP_EN            (0x00080000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_MANY_RETRY_EN         (0x00100000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_TX_DROP_EN            (0x00200000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_SET_ACKID_EN          (0x00400000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_RTE_ISSUE_EN          (0x01000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_PNA_RETRY_EN          (0x02000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_UNEXP_ACKID_EN        (0x04000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_UNSOL_LR_EN           (0x08000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_PNA_EN                (0x10000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_CRC_EVENT_EN          (0x20000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_TTL_EVENT_EN          (0x40000000)
#define CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN_ERR_RATE_EN           (0x80000000)

/* CPS1848_PORT_X_VC0_PA_TX_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_VC0_PA_TX_CNTR_COUNT                        (0xffffffff)

/* CPS1848_PORT_X_NACK_TX_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_NACK_TX_CNTR_COUNT                          (0xffffffff)

/* CPS1848_PORT_X_VC0_RTRY_TX_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_VC0_RTRY_TX_CNTR_COUNT                      (0xffffffff)

/* CPS1848_PORT_X_VC0_PKT_TX_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_VC0_PKT_TX_CNTR_COUNT                       (0xffffffff)

/* CPS1848_PORT_X_TRACE_CNTR_0 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_CNTR_0_COUNT                          (0xffffffff)

/* CPS1848_PORT_X_TRACE_CNTR_1 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_CNTR_1_COUNT                          (0xffffffff)

/* CPS1848_PORT_X_TRACE_CNTR_2 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_CNTR_2_COUNT                          (0xffffffff)

/* CPS1848_PORT_X_TRACE_CNTR_3 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_CNTR_3_COUNT                          (0xffffffff)

/* CPS1848_PORT_X_FILTER_CNTR_0 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_FILTER_CNTR_0_COUNT                         (0xffffffff)

/* CPS1848_PORT_X_FILTER_CNTR_1 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_FILTER_CNTR_1_COUNT                         (0xffffffff)

/* CPS1848_PORT_X_FILTER_CNTR_2 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_FILTER_CNTR_2_COUNT                         (0xffffffff)

/* CPS1848_PORT_X_FILTER_CNTR_3 : Register Bits Masks Definitions */
#define CPS1848_PORT_X_FILTER_CNTR_3_COUNT                         (0xffffffff)

/* CPS1848_PORT_X_VC0_PA_RX_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_VC0_PA_RX_CNTR_COUNT                        (0xffffffff)

/* CPS1848_PORT_X_NACK_RX_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_NACK_RX_CNTR_COUNT                          (0xffffffff)

/* CPS1848_PORT_X_VC0_RTRY_RX_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_VC0_RTRY_RX_CNTR_COUNT                      (0xffffffff)

/* CPS1848_PORT_X_VC0_CPB_TX_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_VC0_CPB_TX_CNTR_COUNT                       (0xffffffff)

/* CPS1848_PORT_X_VC0_PKT_RX_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_VC0_PKT_RX_CNTR_COUNT                       (0xffffffff)

/* CPS1848_PORT_X_TRACE_PW_CTL : Register Bits Masks Definitions */
#define CPS1848_PORT_X_TRACE_PW_CTL_PW_DIS                         (0x00000001)

/* CPS1848_PORT_X_LANE_SYNC : Register Bits Masks Definitions */
#define CPS1848_PORT_X_LANE_SYNC_VMIN                              (0x00000007)

/* CPS1848_PORT_X_VC0_PKT_DROP_RX_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_VC0_PKT_DROP_RX_CNTR_COUNT                  (0xffffffff)

/* CPS1848_PORT_X_VC0_PKT_DROP_TX_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_VC0_PKT_DROP_TX_CNTR_COUNT                  (0xffffffff)

/* CPS1848_PORT_X_VC0_TTL_DROP_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_VC0_TTL_DROP_CNTR_COUNT                     (0xffffffff)

/* CPS1848_PORT_X_VC0_CRC_LIMIT_DROP_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_VC0_CRC_LIMIT_DROP_CNTR_COUNT               (0xffffffff)

/* CPS1848_PORT_X_RETRY_CNTR : Register Bits Masks Definitions */
#define CPS1848_PORT_X_RETRY_CNTR_RETRY_LIM                        (0xffff0000)

/* CPS1848_PORT_X_STATUS_AND_CTL : Register Bits Masks Definitions */
#define CPS1848_PORT_X_STATUS_AND_CTL_RX_FC                        (0x00000001)
#define CPS1848_PORT_X_STATUS_AND_CTL_RETRY_LIM_EN                 (0x00000002)
#define CPS1848_PORT_X_STATUS_AND_CTL_CLR_MANY_RETRY               (0x00000004)

/* CPS1848_BCAST_PORT_OPS : Register Bits Masks Definitions */
#define CPS1848_BCAST_PORT_OPS_CRC_RETX_LIMIT                      (0x0000000e)
#define CPS1848_BCAST_PORT_OPS_LT_LOG_EN                           (0x00000010)
#define CPS1848_BCAST_PORT_OPS_LANE_LOG_EN                         (0x00000020)
#define CPS1848_BCAST_PORT_OPS_PORT_LOG_EN                         (0x00000040)
#define CPS1848_BCAST_PORT_OPS_TRACE_PW_EN                         (0x00000080)
#define CPS1848_BCAST_PORT_OPS_PORT_LPBK_EN                        (0x00000100)
#define CPS1848_BCAST_PORT_OPS_TRACE_0_EN                          (0x00000200)
#define CPS1848_BCAST_PORT_OPS_TRACE_1_EN                          (0x00000400)
#define CPS1848_BCAST_PORT_OPS_TRACE_2_EN                          (0x00000800)
#define CPS1848_BCAST_PORT_OPS_TRACE_3_EN                          (0x00001000)
#define CPS1848_BCAST_PORT_OPS_FILTER_0_EN                         (0x00002000)
#define CPS1848_BCAST_PORT_OPS_FILTER_1_EN                         (0x00004000)
#define CPS1848_BCAST_PORT_OPS_FILTER_2_EN                         (0x00008000)
#define CPS1848_BCAST_PORT_OPS_FILTER_3_EN                         (0x00010000)
#define CPS1848_BCAST_PORT_OPS_SELF_MCAST_EN                       (0x00020000)
#define CPS1848_BCAST_PORT_OPS_TX_FLOW_CTL_DIS                     (0x00080000)
#define CPS1848_BCAST_PORT_OPS_FORCE_REINIT                        (0x00100000)
#define CPS1848_BCAST_PORT_OPS_SILENCE_CTL                         (0x03c00000)
#define CPS1848_BCAST_PORT_OPS_CNTRS_EN                            (0x04000000)
#define CPS1848_BCAST_PORT_OPS_PORT_PW_EN                          (0x08000000)
#define CPS1848_BCAST_PORT_OPS_PORT_INT_EN                         (0x10000000)
#define CPS1848_BCAST_PORT_OPS_CRC_DIS                             (0x20000000)

/* CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET : Register Bits Masks Definitions */
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_REORDER               (0x00000001)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_BAD_CTL               (0x00000002)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_LOA                   (0x00000004)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_IDLE_IN_PKT           (0x00000008)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_PORT_WIDTH            (0x00000010)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_PORT_INIT             (0x00000020)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_UNEXP_STOMP           (0x00000040)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_UNEXP_EOP             (0x00000080)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_LR_X2                 (0x00000100)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_LR_CMD                (0x00000200)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_RX_STOMP              (0x00000400)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_STOMP_TO              (0x00000800)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_RETRY_ACKID           (0x00001000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_RETRY                 (0x00002000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_FATAL_TO              (0x00004000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_UNSOL_RFR             (0x00008000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_SHORT                 (0x00010000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_BAD_TT                (0x00020000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_RX_DROP               (0x00080000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_MANY_RETRY            (0x00100000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_TX_DROP               (0x00200000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_SET_ACKID             (0x00400000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_RTE_ISSUE             (0x01000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_PNA_RETRY             (0x02000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_UNEXP_ACKID           (0x04000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_UNSOL_LR              (0x08000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_PNA                   (0x10000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_CRC_EVENT             (0x20000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_TTL_EVENT             (0x40000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_DET_ERR_RATE              (0x80000000)

/* CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN : Register Bits Masks Definitions */
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_REORDER_EN        (0x00000001)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_BAD_CTL_EN        (0x00000002)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_LOA_EN            (0x00000004)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_IDLE_IN_PKT_EN    (0x00000008)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_PORT_WIDTH_EN     (0x00000010)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_PORT_INIT_EN      (0x00000020)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_UNEXP_STOMP_EN    (0x00000040)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_UNEXP_EOP_EN      (0x00000080)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_LR_X2_EN          (0x00000100)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_LR_CMD_EN         (0x00000200)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_RX_STOMP_EN       (0x00000400)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_STOMP_TO_EN       (0x00000800)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_RETRY_ACKID_EN    (0x00001000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_RETRY_EN          (0x00002000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_FATAL_TO_EN       (0x00004000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_UNSOL_RFR_EN      (0x00008000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_SHORT_EN          (0x00010000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_BAD_TT_EN         (0x00020000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_RX_DROP_EN        (0x00080000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_MANY_RETRY_EN     (0x00100000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_TX_DROP_EN        (0x00200000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_SET_ACKID_EN      (0x00400000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_RTE_ISSUE_EN      (0x01000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_PNA_RETRY_EN      (0x02000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_UNEXP_ACKID_EN    (0x04000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_UNSOL_LR_EN       (0x08000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_PNA_EN            (0x10000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_CRC_EVENT_EN      (0x20000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_TTL_EVENT_EN      (0x40000000)
#define CPS1848_BCAST_PORT_IMPL_SPEC_ERR_RATE_EN_ERR_RATE_EN       (0x80000000)

/* CPS1848_LOG_CTL : Register Bits Masks Definitions */
#define CPS1848_LOG_CTL_ALL_FLAG_STOP                              (0x00000001)
#define CPS1848_LOG_CTL_CNTR_MAX_STOP                              (0x00000002)
#define CPS1848_LOG_CTL_LOG_TBL_OVERWRITE                          (0x00000004)

/* CPS1848_LOG_DATA : Register Bits Masks Definitions */
#define CPS1848_LOG_DATA_ERR_NUM                                   (0x0000000f)
#define CPS1848_LOG_DATA_ERR_GROUP                                 (0x000000f0)
#define CPS1848_LOG_DATA_ERR_SOURCE                                (0x00007f00)

/* CPS1848_LOG_MATCH_X : Register Bits Masks Definitions */
#define CPS1848_LOG_MATCH_X_ERR_NUM                                (0x0000000f)
#define CPS1848_LOG_MATCH_X_ERR_GROUP                              (0x000000f0)
#define CPS1848_LOG_MATCH_X_ERR_SOURCE                             (0x00007f00)
#define CPS1848_LOG_MATCH_X_STOP_EN                                (0x00010000)
#define CPS1848_LOG_MATCH_X_MAINT_PKT_EN                           (0x00020000)
#define CPS1848_LOG_MATCH_X_FLAG_EN                                (0x00040000)
#define CPS1848_LOG_MATCH_X_CNT_EN                                 (0x00080000)
#define CPS1848_LOG_MATCH_X_ERR_NUM_MASK                           (0x00100000)
#define CPS1848_LOG_MATCH_X_ERR_GROUP_MASK                         (0x00200000)
#define CPS1848_LOG_MATCH_X_ERR_SOURCE_MASK                        (0x00400000)

/* CPS1848_LOG_MATCH_STATUS : Register Bits Masks Definitions */
#define CPS1848_LOG_MATCH_STATUS_FLAG_ERR_0                        (0x00000001)
#define CPS1848_LOG_MATCH_STATUS_FLAG_ERR_1                        (0x00000002)
#define CPS1848_LOG_MATCH_STATUS_FLAG_ERR_2                        (0x00000004)
#define CPS1848_LOG_MATCH_STATUS_FLAG_ERR_3                        (0x00000008)
#define CPS1848_LOG_MATCH_STATUS_FLAG_ERR_4                        (0x00000010)
#define CPS1848_LOG_MATCH_STATUS_FLAG_ERR_5                        (0x00000020)
#define CPS1848_LOG_MATCH_STATUS_FLAG_ERR_6                        (0x00000040)
#define CPS1848_LOG_MATCH_STATUS_FLAG_ERR_7                        (0x00000080)

/* CPS1848_LOG_EVENTS : Register Bits Masks Definitions */
#define CPS1848_LOG_EVENTS_CNT                                     (0x0000ffff)

/* CPS1848_LOG_CTL2 : Register Bits Masks Definitions */
#define CPS1848_LOG_CTL2_FLAG_RESET                                (0x00000002)
#define CPS1848_LOG_CTL2_CNT_RESET                                 (0x00000004)
#define CPS1848_LOG_CTL2_ERR_FIFO_RESET                            (0x00000008)
#define CPS1848_LOG_CTL2_MAINT_PKT_DIS                             (0x00000010)
#define CPS1848_LOG_CTL2_STOP_EM                                   (0x00000020)

/* CPS1848_PLL_X_CTL_1 : Register Bits Masks Definitions */
#define CPS1848_PLL_X_CTL_1_PLL_DIV_SEL                            (0x00000001)
#define CPS1848_PLL_X_CTL_1_PLL_PWR_DOWN                           (0x00000002)

/* CPS1848_PLL_X_CTL_2 : Register Bits Masks Definitions */
#define CPS1848_PLL_X_CTL_2_PLL_AUTO_RESET                         (0x00000002)

/* CPS1848_BCAST_PLL_CTL : Register Bits Masks Definitions */
#define CPS1848_BCAST_PLL_CTL_PLL_DIV_SEL                          (0x00000001)

/* CPS1848_LANE_X_CTL : Register Bits Masks Definitions */
#define CPS1848_LANE_X_CTL_LANE_DIS                                (0x00000001)
#define CPS1848_LANE_X_CTL_RX_RATE                                 (0x00000006)
#define CPS1848_LANE_X_CTL_TX_RATE                                 (0x00000018)
#define CPS1848_LANE_X_CTL_TX_AMP_CTL                              (0x000007e0)
#define CPS1848_LANE_X_CTL_TX_SYMBOL_CTL                           (0x00001800)
#define CPS1848_LANE_X_CTL_LPBK_8BIT_EN                            (0x00004000)
#define CPS1848_LANE_X_CTL_LPBK_10BIT_EN                           (0x00008000)
#define CPS1848_LANE_X_CTL_PRBS_RX_CHECKER_MODE                    (0x00020000)
#define CPS1848_LANE_X_CTL_XMITPRBS                                (0x00040000)
#define CPS1848_LANE_X_CTL_PRBS_EN                                 (0x00080000)
#define CPS1848_LANE_X_CTL_PRBS_TRAIN                              (0x00100000)
#define CPS1848_LANE_X_CTL_LANE_PW_EN                              (0x00200000)
#define CPS1848_LANE_X_CTL_LANE_INT_EN                             (0x00400000)
#define CPS1848_LANE_X_CTL_PRBS_UNIDIR_BERT_MODE_EN                (0x00800000)
#define CPS1848_LANE_X_CTL_PRBS_MODE                               (0x1e000000)

#define CPS1848_LANE_X_CTL_SPD1                                    (0x0000000A)
#define CPS1848_LANE_X_CTL_SPD2                                    (0x00000014)

/* CPS1848_LANE_X_PRBS_GEN_SEED : Register Bits Masks Definitions */
#define CPS1848_LANE_X_PRBS_GEN_SEED_PRBS_SEED                     (0x7fffffff)

/* CPS1848_LANE_X_PRBS_ERR_CNTR : Register Bits Masks Definitions */
#define CPS1848_LANE_X_PRBS_ERR_CNTR_PRBS_ERR_CNT                  (0x000000ff)
#define CPS1848_LANE_X_PRBS_ERR_CNTR_PRBS_ERR                      (0x00000100)

/* CPS1848_LANE_X_ERR_DET : Register Bits Masks Definitions */
#define CPS1848_LANE_X_ERR_DET_LANE_SYNC                           (0x00000001)
#define CPS1848_LANE_X_ERR_DET_LANE_RDY                            (0x00000002)
#define CPS1848_LANE_X_ERR_DET_BAD_CHAR                            (0x00000004)
#define CPS1848_LANE_X_ERR_DET_DESCRAM_SYNC                        (0x00000008)
#define CPS1848_LANE_X_ERR_DET_TX_RX_MISMATCH                      (0x00000010)
#define CPS1848_LANE_X_ERR_DET_IDLE2_FRAME                         (0x00000080)
#define CPS1848_LANE_X_ERR_DET_LANE_INVER_DET                      (0x00000100)
#define CPS1848_LANE_X_ERR_DET_BAD_SPEED                           (0x00000200)
#define CPS1848_LANE_X_ERR_DET_UNUSED                              (0x00001c00)

/* CPS1848_LANE_X_ERR_RATE_EN : Register Bits Masks Definitions */
#define CPS1848_LANE_X_ERR_RATE_EN_LANE_SYNC_EN                    (0x00000001)
#define CPS1848_LANE_X_ERR_RATE_EN_LANE_RDY_EN                     (0x00000002)
#define CPS1848_LANE_X_ERR_RATE_EN_BAD_CHAR_EN                     (0x00000004)
#define CPS1848_LANE_X_ERR_RATE_EN_DESCRAM_SYNC_EN                 (0x00000008)
#define CPS1848_LANE_X_ERR_RATE_EN_TX_RX_MISMATCH_EN               (0x00000010)
#define CPS1848_LANE_X_ERR_RATE_EN_IDLE2_FRAME_EN                  (0x00000080)
#define CPS1848_LANE_X_ERR_RATE_EN_LANE_INVER_DET_EN               (0x00000100)
#define CPS1848_LANE_X_ERR_RATE_EN_BAD_SPEED_EN                    (0x00000200)
#define CPS1848_LANE_X_ERR_RATE_EN_UNUSED                          (0x00001c00)

/* CPS1848_LANE_X_ATTR_CAPT : Register Bits Masks Definitions */
#define CPS1848_LANE_X_ATTR_CAPT_VALID                             (0x00000001)
#define CPS1848_LANE_X_ATTR_CAPT_ERR_TYPE                          (0x1f000000)
#define CPS1848_LANE_X_ATTR_CAPT_INFO_TYPE                         (0xe0000000)

/* CPS1848_LANE_X_DATA_CAPT_0 : Register Bits Masks Definitions */
#define CPS1848_LANE_X_DATA_CAPT_0_CAPT_DATA                       (0xffffffff)

/* CPS1848_LANE_X_DATA_CAPT_1 : Register Bits Masks Definitions */
#define CPS1848_LANE_X_DATA_CAPT_1_CAPT_DATA                       (0xffffffff)

/* CPS1848_LANE_X_DFE_1 : Register Bits Masks Definitions */
#define CPS1848_LANE_X_DFE_1_TAP_0_SEL                             (0x00001000)
#define CPS1848_LANE_X_DFE_1_TAP_1_SEL                             (0x00002000)
#define CPS1848_LANE_X_DFE_1_TAP_2_SEL                             (0x00004000)
#define CPS1848_LANE_X_DFE_1_TAP_3_SEL                             (0x00008000)
#define CPS1848_LANE_X_DFE_1_TAP_4_SEL                             (0x00010000)
#define CPS1848_LANE_X_DFE_1_TAP_OFFSET_SEL                        (0x00020000)
#define CPS1848_LANE_X_DFE_1_RX_DFE_DIS                            (0x00040000)

/* CPS1848_LANE_X_DFE_2 : Register Bits Masks Definitions */
#define CPS1848_LANE_X_DFE_2_CFG_EN                                (0x00000001)
#define CPS1848_LANE_X_DFE_2_TAP_0_CFG                             (0x0000001e)
#define CPS1848_LANE_X_DFE_2_TAP_1_CFG                             (0x000007e0)
#define CPS1848_LANE_X_DFE_2_TAP_2_CFG                             (0x0000f800)
#define CPS1848_LANE_X_DFE_2_TAP_2_CFG_SIGN                        (0x00008000)
#define CPS1848_LANE_X_DFE_2_TAP_3_CFG                             (0x000f0000)
#define CPS1848_LANE_X_DFE_2_TAP_3_CFG_SIGN                        (0x00080000)
#define CPS1848_LANE_X_DFE_2_TAP_4_CFG                             (0x00700000)
#define CPS1848_LANE_X_DFE_2_TAP_4_CFG_SIGN                        (0x00400000)
#define CPS1848_LANE_X_DFE_2_TAP_OFFSET_CFG                        (0x1f800000)

/* CPS1848_LANE_X_DFE_5 : Register Bits Masks Definitions */
#define CPS1848_LANE_X_DFE_5_MAGIC                                 (0x00000018)

/* CPS1848_BCAST_LANE_CTL : Register Bits Masks Definitions */
#define CPS1848_BCAST_LANE_CTL_LANE_DIS                            (0x00000001)
#define CPS1848_BCAST_LANE_CTL_RX_RATE                             (0x00000006)
#define CPS1848_BCAST_LANE_CTL_TX_RATE                             (0x00000018)
#define CPS1848_BCAST_LANE_CTL_TX_AMP_CTL                          (0x000007e0)
#define CPS1848_BCAST_LANE_CTL_TX_SYMBOL_CTL                       (0x00001800)
#define CPS1848_BCAST_LANE_CTL_LPBK_8BIT_EN                        (0x00004000)
#define CPS1848_BCAST_LANE_CTL_LPBK_10BIT_EN                       (0x00008000)
#define CPS1848_BCAST_LANE_CTL_PRBS_RX_CHECKER_MODE                (0x00020000)
#define CPS1848_BCAST_LANE_CTL_XMITPRBS                            (0x00040000)
#define CPS1848_BCAST_LANE_CTL_PRBS_EN                             (0x00080000)
#define CPS1848_BCAST_LANE_CTL_PRBS_TRAIN                          (0x00100000)
#define CPS1848_BCAST_LANE_CTL_LANE_PW_EN                          (0x00200000)
#define CPS1848_BCAST_LANE_CTL_LANE_INT_EN                         (0x00400000)
#define CPS1848_BCAST_LANE_CTL_PRBS_UNIDIR_BERT_MODE_EN            (0x00800000)
#define CPS1848_BCAST_LANE_CTL_PRBS_MODE                           (0x1e000000)

/* CPS1848_BCAST_LANE_GEN_SEED : Register Bits Masks Definitions */
#define CPS1848_BCAST_LANE_GEN_SEED_PRBS_SEED                      (0x7fffffff)

/* CPS1848_BCAST_LANE_ERR_DET : Register Bits Masks Definitions */
#define CPS1848_BCAST_LANE_ERR_DET_LANE_SYNC                       (0x00000001)
#define CPS1848_BCAST_LANE_ERR_DET_LANE_RDY                        (0x00000002)
#define CPS1848_BCAST_LANE_ERR_DET_BAD_CHAR                        (0x00000004)
#define CPS1848_BCAST_LANE_ERR_DET_DESCRAM_SYNC                    (0x00000008)
#define CPS1848_BCAST_LANE_ERR_DET_TX_RX_MISMATCH                  (0x00000010)
#define CPS1848_BCAST_LANE_ERR_DET_IDLE2_FRAME                     (0x00000080)
#define CPS1848_BCAST_LANE_ERR_DET_LANE_INVER_DET                  (0x00000100)
#define CPS1848_BCAST_LANE_ERR_DET_BAD_SPEED                       (0x00000200)
#define CPS1848_BCAST_LANE_ERR_DET_UNUSED                          (0x00001c00)

/* CPS1848_BCAST_LANE_ERR_RATE_EN : Register Bits Masks Definitions */
#define CPS1848_BCAST_LANE_ERR_RATE_EN_LANE_SYNC_EN                (0x00000001)
#define CPS1848_BCAST_LANE_ERR_RATE_EN_LANE_RDY_EN                 (0x00000002)
#define CPS1848_BCAST_LANE_ERR_RATE_EN_BAD_CHAR_EN                 (0x00000004)
#define CPS1848_BCAST_LANE_ERR_RATE_EN_DESCRAM_SYNC_EN             (0x00000008)
#define CPS1848_BCAST_LANE_ERR_RATE_EN_TX_RX_MISMATCH_EN           (0x00000010)
#define CPS1848_BCAST_LANE_ERR_RATE_EN_IDLE2_FRAME_EN              (0x00000080)
#define CPS1848_BCAST_LANE_ERR_RATE_EN_LANE_INVER_DET_EN           (0x00000100)
#define CPS1848_BCAST_LANE_ERR_RATE_EN_BAD_SPEED_EN                (0x00000200)
#define CPS1848_BCAST_LANE_ERR_RATE_EN_UNUSED                      (0x00001c00)

/* CPS1848_BCAST_LANE_ATTR_CAPT : Register Bits Masks Definitions */
#define CPS1848_BCAST_LANE_ATTR_CAPT_VALID                         (0x00000001)

/* CPS1848_BCAST_LANE_DFE_1 : Register Bits Masks Definitions */
#define CPS1848_BCAST_LANE_DFE_1_TAP_0_SEL                         (0x00001000)
#define CPS1848_BCAST_LANE_DFE_1_TAP_1_SEL                         (0x00002000)
#define CPS1848_BCAST_LANE_DFE_1_TAP_2_SEL                         (0x00004000)
#define CPS1848_BCAST_LANE_DFE_1_TAP_3_SEL                         (0x00008000)
#define CPS1848_BCAST_LANE_DFE_1_TAP_4_SEL                         (0x00010000)
#define CPS1848_BCAST_LANE_DFE_1_TAP_OFFSET_SEL                    (0x00020000)
#define CPS1848_BCAST_LANE_DFE_1_RX_DFE_DIS                        (0x00040000)

/* CPS1848_BCAST_LANE_DFE_2 : Register Bits Masks Definitions */
#define CPS1848_BCAST_LANE_DFE_2_CFG_EN                            (0x00000001)
#define CPS1848_BCAST_LANE_DFE_2_TAP_0_CFG                         (0x0000001e)
#define CPS1848_BCAST_LANE_DFE_2_TAP_1_CFG                         (0x000007e0)
#define CPS1848_BCAST_LANE_DFE_2_TAP_2_CFG                         (0x0000f800)
#define CPS1848_BCAST_LANE_DFE_2_TAP_3_CFG                         (0x000f0000)
#define CPS1848_BCAST_LANE_DFE_2_TAP_4_CFG                         (0x00700000)
#define CPS1848_BCAST_LANE_DFE_2_TAP_OFFSET_CFG                    (0x1f800000)

/* CPS1848_BCAST_PORT_ERR_DET : Register Bits Masks Definitions */
#define CPS1848_BCAST_PORT_ERR_DET_LINK_TIMEOUT                    (0x00000001)
#define CPS1848_BCAST_PORT_ERR_DET_CS_ACK_ILL                      (0x00000002)
#define CPS1848_BCAST_PORT_ERR_DET_DELIN_ERR                       (0x00000004)
#define CPS1848_BCAST_PORT_ERR_DET_PRTCL_ERR                       (0x00000010)
#define CPS1848_BCAST_PORT_ERR_DET_LR_ACKID_ILL                    (0x00000020)
#define CPS1848_BCAST_PORT_ERR_DET_IDLE1_ERR                       (0x00008000)
#define CPS1848_BCAST_PORT_ERR_DET_PKT_ILL_SIZE                    (0x00020000)
#define CPS1848_BCAST_PORT_ERR_DET_PKT_CRC_ERR                     (0x00040000)
#define CPS1848_BCAST_PORT_ERR_DET_PKT_ILL_ACKID                   (0x00080000)
#define CPS1848_BCAST_PORT_ERR_DET_CS_NOT_ACC                      (0x00100000)
#define CPS1848_BCAST_PORT_ERR_DET_UNEXP_ACKID                     (0x00200000)
#define CPS1848_BCAST_PORT_ERR_DET_CS_CRC_ERR                      (0x00400000)
#define CPS1848_BCAST_PORT_ERR_DET_IMP_SPEC_ERR                    (0x80000000)

/* CPS1848_BCAST_PORT_ERR_RATE_EN : Register Bits Masks Definitions */
#define CPS1848_BCAST_PORT_ERR_RATE_EN_LINK_TIMEOUT_EN             (0x00000001)
#define CPS1848_BCAST_PORT_ERR_RATE_EN_CS_ACK_ILL_EN               (0x00000002)
#define CPS1848_BCAST_PORT_ERR_RATE_EN_DELIN_ERR_EN                (0x00000004)
#define CPS1848_BCAST_PORT_ERR_RATE_EN_PRTCL_ERR_EN                (0x00000010)
#define CPS1848_BCAST_PORT_ERR_RATE_EN_LR_ACKID_ILL_EN             (0x00000020)
#define CPS1848_BCAST_PORT_ERR_RATE_EN_IDLE1_ERR_EN                (0x00008000)
#define CPS1848_BCAST_PORT_ERR_RATE_EN_PKT_ILL_SIZE_EN             (0x00020000)
#define CPS1848_BCAST_PORT_ERR_RATE_EN_PKT_CRC_ERR_EN              (0x00040000)
#define CPS1848_BCAST_PORT_ERR_RATE_EN_PKT_ILL_ACKID_EN            (0x00080000)
#define CPS1848_BCAST_PORT_ERR_RATE_EN_CS_NOT_ACC_EN               (0x00100000)
#define CPS1848_BCAST_PORT_ERR_RATE_EN_UNEXP_ACKID_EN              (0x00200000)
#define CPS1848_BCAST_PORT_ERR_RATE_EN_CS_CRC_ERR_EN               (0x00400000)
#define CPS1848_BCAST_PORT_ERR_RATE_EN_IMP_SPEC_ERR_EN             (0x80000000)

#ifdef __cplusplus
}
#endif

#endif /* __CPS1848_H__ */
