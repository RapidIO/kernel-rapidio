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

#ifndef __RIO_STANDARD_H__
#define __RIO_STANDARD_H__

#include <stdint.h>

/* rio_regdefs.h contains definitons for all RapidIO Standard Registers
*/

#ifdef __cplusplus
extern "C" {
#endif

/* Offsets, field masks and value definitions for Device CARs and CSRs
*/
#define RIO_DEV_IDENT       (0x00)
#define RIO_DEV_INF         (0x04)
#define RIO_ASSY_ID         (0x08)
#define RIO_ASSY_INF        (0x0c)
#define RIO_PE_FEAT         (0x10)
#define RIO_SW_PORT_INF     (0x14)
#define RIO_SRC_OPS         (0x18)
#define RIO_DST_OPS         (0x1C)
#define RIO_SW_MC_SUP       (0x30)
#define RIO_SW_RT_TBL_LIM   (0x34)
#define RIO_SW_MC_INF       (0x38)
#define RIO_PE_LL_CTL       (0x4C)
#define RIO_LCS_ADDR0       (0x58)
#define RIO_LCS_ADDR1       (0x5C)
#define RIO_DEVID           (0x60)
#define RIO_HOST_LOCK       (0x68)
#define RIO_COMPTAG         (0x6c)
#define RIO_DEVID_RTE       (0x70)
#define RIO_RTE             (0x74)
#define RIO_DFLT_RTE        (0x78)
#define RIO_MC_MSK_CFG      (0x80)
#define RIO_MC_CON_SEL      (0x84)
#define RIO_MC_CON_OP       (0x88)

/* RIO_DEV_IDENT : Register Bits Masks Definitions */
#define RIO_DEV_IDENT_VEND                               (0x0000ffff)
#define RIO_DEV_IDENT_DEVI                               (0xffff0000)

/* RIO_DEV_INF : Register Bits Masks Definitions */
#define RIO_DEV_INF_MASK                                 (0xffffffff)

/* RIO_ASSY_IDENT : Register Bits Masks Definitions */
#define RIO_ASSY_ID_VEND                                 (0x0000ffff)
#define RIO_ASSY_ID_ASSY                                 (0xffff0000)

/* RIO_ASSY_INF : Register Bits Masks Definitions */
#define RIO_ASSY_INF_EFB_PTR                             (0x0000ffff)
#define RIO_ASSY_INF_ASSY_REV                            (0xffff0000)

/* Device Id helper macros*/
#define RIO_HOST_LOCK_BASE_ID_MASK 0x0000ffff
#define RIO_DID_GET_BASE_DEVICE_ID(did) (did >> 16) /* Get base device id */
#define RIO_DID_UNSET 0x00ffffff /* Base Device ID when PE is initialized */

/* RIO_PROC_ELEM_FEAT : Register Bits Masks Definitions */
typedef uint32_t RIO_PE_FEAT_T;
typedef uint32_t RIO_PE_ADDR_T;
#define RIO_PE_FEAT_EXT_ADDR                        (0x00000007)
#define RIO_PE_FEAT_EXT_ADDR34                  ((RIO_PE_ADDR_T)(0x00000001))
#define RIO_PE_FEAT_EXT_ADDR50                  ((RIO_PE_ADDR_T)(0x00000002))
#define RIO_PE_FEAT_EXT_ADDR66                  ((RIO_PE_ADDR_T)(0x00000004))
#define RIO_PE_FEAT_EFB_VALID                       (0x00000008)
#define RIO_PE_FEAT_CTLS                            (0x00000010)
#define RIO_PE_FEAT_CRF                             (0x00000020)
#define RIO_PE_FEAT_IMP_SPEC                        (0x00000040)
#define RIO_PE_FEAT_FLOW_CTL                        (0x00000080)
#define RIO_PE_FEAT_STD_RTE                         (0x00000100)
#define RIO_PE_FEAT_EXTD_RTE                        (0x00000200)
#define RIO_PE_FEAT_MC                              (0x00000400)
#define RIO_PE_FEAT_FLOW_ARB                        (0x00000800)
#define RIO_PE_FEAT_MULTIP                          (0x08000000)
#define RIO_PE_FEAT_SW                              (0x10000000)
#define RIO_PE_FEAT_PROC                            (0x20000000)
#define RIO_PE_FEAT_MEM                             (0x40000000)
#define RIO_PE_FEAT_BRDG                            (0x80000000)

#define PE_IS_SW(x)            (x & RIO_PE_FEAT_SW)
#define PE_IS_PROC(x)          (x & RIO_PE_FEAT_PROC)
#define PE_IS_MEM(x)           (x & RIO_PE_FEAT_MEM)
#define PE_IS_BRIDGE(x)        (x & RIO_PE_FEAT_BRDG)

/* RIO_SW_PORT_INF : Register Bits Masks Definitions */
typedef uint32_t RIO_SW_PORT_INF_T;
#define RIO_SW_PORT_INF_PORT                           (0x000000ff)
#define RIO_SW_PORT_INF_TOT                            (0x0000ff00)

#define RIO_ACCESS_PORT(i) (i & RIO_SW_PORT_INF_PORT)
#define RIO_AVAIL_PORTS(i) ((i & RIO_SW_PORT_INF_TOT) >> 8)
#define RIO_PORT_IS_VALID(p,i) (p < RIO_AVAIL_PORTS(i)) 

/* RIO_SRC_OPS : Register Bits Masks Definitions */
typedef uint32_t RIO_SRC_OPS_T;
#define RIO_SRC_OPS_PORT_WR       (0x00000004)
#define RIO_SRC_OPS_A_CLEAR       (0x00000010)
#define RIO_SRC_OPS_A_SET         (0x00000020)
#define RIO_SRC_OPS_A_DEC         (0x00000040)
#define RIO_SRC_OPS_A_INC         (0x00000080)
#define RIO_SRC_OPS_A_TSWAP       (0x00000100)
#define RIO_SRC_OPS_DBELL         (0x00000400)
#define RIO_SRC_OPS_D_MSG         (0x00000800)
#define RIO_SRC_OPS_WR_RES        (0x00001000)
#define RIO_SRC_OPS_STRM_WR       (0x00002000)
#define RIO_SRC_OPS_WRITE         (0x00004000)
#define RIO_SRC_OPS_READ          (0x00008000)
#define RIO_SRC_OPS_IMPLEMENT_DEF (0x00030000)
#define RIO_SRC_OPS_DSTM          (0x00040000)
#define RIO_SRC_OPS_GSM_TLBIS     (0x00400000)
#define RIO_SRC_OPS_GSM_TLBI      (0x00800000)
#define RIO_SRC_OPS_GSM_IINV      (0x01000000)
#define RIO_SRC_OPS_GSM_RD        (0x02000000)
#define RIO_SRC_OPS_GSM_FLUSH     (0x04000000)
#define RIO_SRC_OPS_GSM_CAST      (0x08000000)
#define RIO_SRC_OPS_GSM_DINV      (0x10000000)
#define RIO_SRC_OPS_GSM_R4O       (0x20000000)
#define RIO_SRC_OPS_GSM_IR        (0x40000000)
#define RIO_SRC_OPS_GSM_R         (0x80000000)

/* RIO_DST_OPS : Register Bits Masks Definitions */
typedef uint32_t RIO_DST_OPS_T;
#define RIO_DST_OPS_PORT_WR       (0x00000004)
#define RIO_DST_OPS_A_CLEAR       (0x00000010)
#define RIO_DST_OPS_A_SET         (0x00000020)
#define RIO_DST_OPS_A_DEC         (0x00000040)
#define RIO_DST_OPS_A_INC         (0x00000080)
#define RIO_DST_OPS_A_TSWAP       (0x00000100)
#define RIO_DST_OPS_DBELL         (0x00000400)
#define RIO_DST_OPS_D_MSG         (0x00000800)
#define RIO_DST_OPS_WR_RES        (0x00001000)
#define RIO_DST_OPS_STRM_WR       (0x00002000)
#define RIO_DST_OPS_WRITE         (0x00004000)
#define RIO_DST_OPS_READ          (0x00008000)
#define RIO_DST_OPS_IMPLEMENT_DEF (0x00030000)
#define RIO_DST_OPS_DSTM          (0x00040000)
#define RIO_DST_OPS_GSM_TLBIS     (0x00400000)
#define RIO_DST_OPS_GSM_TLBI      (0x00800000)
#define RIO_DST_OPS_GSM_IINV      (0x01000000)
#define RIO_DST_OPS_GSM_RD        (0x02000000)
#define RIO_DST_OPS_GSM_FLUSH     (0x04000000)
#define RIO_DST_OPS_GSM_CAST      (0x08000000)
#define RIO_DST_OPS_GSM_DINV      (0x10000000)
#define RIO_DST_OPS_GSM_R4O       (0x20000000)
#define RIO_DST_OPS_GSM_IR        (0x40000000)
#define RIO_DST_OPS_GSM_R         (0x80000000)

/* RIO_SW_MC_SUP : Register Bits Masks Definitions */
#define RIO_SW_MC_SUP_SIMPLE                        (0x80000000)

/* RIO_SW_RT_TBL_LIM : Register Bits Masks Definitions */
#define RIO_SW_RT_TBL_LIM_MAX_DESTID                  (0x0000ffff)

/* RIO_SW_MC_INF : Register Bits Masks Definitions */
#define RIO_SW_MC_INF_MC_MSK                     (0x0000ffff)
#define RIO_SW_MC_INF_MAX_DESTID                     (0x3fff0000)
#define RIO_SW_MC_INF_PER_PORT                       (0x40000000)
#define RIO_SW_MC_INF_BLK                            (0x80000000)

/* RIO_PE_LL_CTL : Register Bits Masks Definitions */
#define RIO_PE_LL_CTL_34BIT  RIO_PE_FEAT_EXT_ADDR34
#define RIO_PE_LL_CTL_50BIT  RIO_PE_FEAT_EXT_ADDR50
#define RIO_PE_LL_CTL_66BIT  RIO_PE_FEAT_EXT_ADDR66

/* RIO_LCS_ADDR0 : Register Bits Masks Definitions */
#define RIO_LCS_ADDR0_34           (0x00000000)
#define RIO_LCS_ADDR0_50           (0x00007FFF)
#define RIO_LCS_ADDR0_66           (0x7FFFFFFF)
#define RIO_LCS_ADDR0_66M          (0x60000000)
#define RIO_LCS_ADDR0_66L          (~RIO_LCS_ADDR0_66M & RIO_LCS_ADDR0_66)

/* RIO_LCS_ADDR1 : Register Bits Masks Definitions */
#define RIO_LCS_ADDR1_34           (0x7FFFFFFF)
#define RIO_LCS_ADDR1_50           (0xFFFFFFFF)
#define RIO_LCS_ADDR1_66           (0xFFFFFFFF)

#define RIO_LCS_LOW64(a0,a1,m0,m1) ((((uint64_t)a0 & m0) << 35) | \
				    (((uint64_t)a1 & m1) << 3))
#define RIO_LCS_ADDR34(a0,a1) \
	((uint64_t)RIO_LCS_LOW64(a0, a1, RIO_LCS_ADDR0_34, RIO_LCS_ADDR1_34))

#define RIO_LCS_ADDR50(a0,a1) \
	((uint64_t)RIO_LCS_LOW64(a0, a1, RIO_LCS_ADDR0_50, RIO_LCS_ADDR1_50))

#define RIO_LCS_ADDR66M(a0) ((a0 & RIO_LCS_ADDR0_66M) >> 29)
#define RIO_LCS_ADDR66L(a0,a1) \
	((uint64_t)RIO_LCS_LOW64(a0, a1, RIO_LCS_ADDR0_66L, RIO_LCS_ADDR1_66))

/* RIO_DEVID : Register Bits Masks Definitions */

typedef uint32_t did_reg_t;

#define RIO_DEVID_DEV16             (0x0000ffff)
#define RIO_DEVID_DEV8              (0x00ff0000)
#define GET_DEV8_FROM_HW(x) ((x & RIO_DEVID_DEV8) >> 16)
#define GET_DEV16_FROM_HW(x) (x & RIO_DEVID_DEV16)
#define MAKE_HW_FROM_DEV8(x) ((x << 16) & RIO_DEVID_DEV8)
#define MAKE_HW_FROM_DEV16(x) (x & RIO_DEVID_DEV16)
#define MAKE_HW_FROM_DEV8N16(d8,d16) (MAKE_HW_FROM_DEV8(d8) | \
					MAKE_HW_FROM_DEV16(d16))

/* RIO_HOST_LOCK : Register Bits Masks Definitions */
#define RIO_HOST_LOCK_UNLOCKED                         (0x0000ffff)
#define RIO_HOST_LOCK_DEVID                            (0x0000ffff)
#define RIO_HOST_LOCK_DEVID32                          (0xffff0000)

/* RIO_COMP_TAG : Register Bits Masks Definitions */
#define RIO_COMPTAG_CTAG                                  (0xffffffff)

/* RIO_DEVID_RTE : Register Bits Masks Definitions */
#define RIO_DEVID_RTE_DEV8                                (0x000000ff)
#define RIO_DEVID_RTE_DEV16                               (0x0000ff00)
#define RIO_DEVID_RTE_EXTD_EN                             (0x80000000)

/* RIO_RTE : Register Bits Masks Definitions */
#define RIO_RTE_PORT           (0x000000ff)
#define RIO_RTE_EXTD_PORT_1    (0x0000ff00)
#define RIO_RTE_EXTD_PORT_2    (0x00ff0000)
#define RIO_RTE_EXTD_PORT_3    (0xff000000)

#define RIO_RTE_IMP_SPEC (0xF0000000)

#define RIO_RTE_TYPE     (0x00000300)
#define RIO_RTE_TYPE_PORT	(0x00000000)
#define RIO_RTE_TYPE_MC_MSK	(0x00000100)
#define RIO_RTE_TYPE_LVL_GRP	(0x00000200)

#define RIO_RTE_IDX      (0x000000FF)
#define RIO_RTE_VAL      (RIO_RTE_TYPE | RIO_RTE_IDX)

typedef uint32_t pe_rt_val;

#define RIO_RTE_PT_0             (0x00000000)
#define RIO_RTE_PT_LAST          (0x000000FF)
#define RIO_RTE_MC_0             (0x00000100)
#define RIO_RTE_MC_LAST          (0x000001FF)
#define RIO_RTE_LVL_G0           (0x00000200)
#define RIO_RTE_LVL_GLAST        (0x000002FF)
#define RIO_RTE_DROP             (0x00000300)
#define RIO_RTE_DFLT_PORT        (0x00000301)
#define RIO_RTE_BAD              (0x0FFFFFFF)

#ifdef UNIT_TESTING
#define RIO_RT_GRP_SZ 256
#else
#define RIO_RT_GRP_SZ 0x28
#endif

#define RIO_LAST_DEV8 (0x0FF)
#define RIO_LAST_DEV16 (0x0FFFF)
#define RIO_LAST_DEV32 (0xFFFFFFFF)

#define RIO_RTV_IS_PORT(n) (RIO_RTE_TYPE_PORT == ((n) & RIO_RTE_TYPE))
#define RIO_RTV_IS_MC_MSK(n) (RIO_RTE_TYPE_MC_MSK == ((n) & RIO_RTE_TYPE))
#define RIO_RTV_IS_LVL_GRP(n) (RIO_RTE_TYPE_LVL_GRP == ((n) & RIO_RTE_TYPE))

#define RIO_RTV_PORT(n) ((n & ~RIO_RTE_IDX) ? RIO_RTE_BAD : \
				(RIO_RTE_TYPE_PORT + (n) ))
#define RIO_RTV_MC_MSK(n) ((n & ~RIO_RTE_IDX) ? RIO_RTE_BAD : \
				(RIO_RTE_TYPE_MC_MSK + ((n))))
#define RIO_RTV_LVL_GRP(n) ((n & ~RIO_RTE_IDX) ? RIO_RTE_BAD : \
				(RIO_RTE_TYPE_LVL_GRP + ((n))))

#define RIO_RTV_GET_PORT(n) (!RIO_RTV_IS_PORT(n) ? RIO_RTE_BAD : \
				((n) & RIO_RTE_IDX))
#define RIO_RTV_GET_MC_MSK(n) (!RIO_RTV_IS_MC_MSK(n) ? RIO_RTE_BAD : \
				((n) & RIO_RTE_IDX))
#define RIO_RTV_GET_LVL_GRP(n) (!RIO_RTV_IS_LVL_GRP(n) ? RIO_RTE_BAD : \
				((n) & RIO_RTE_IDX))


/* RIO_DFLT_RTE : Register Bits Masks Definitions */
#define RIO_DFLT_RTE_RTE        RIO_RTE_RTE
#define RIO_DFLT_RTE_IMP_SPEC   RIO_RTE_IMP_SPEC

/* RIO_MC_MSK_CFG : Register Bits Masks Definitions */
#define RIO_MC_MSK_CFG_PRESENT  (0x00000001)
#define RIO_MC_MSK_CFG_CMD      (0x00000070)
#define RIO_MC_MSK_CFG_PT_NUM   (0x0000ff00)
#define RIO_MC_MSK_CFG_MSK_NUM  (0xffff0000)

#define RIO_MC_MSK_CFG_CMD_READ     (0x00000000)
#define RIO_MC_MSK_CFG_CMD_ADD      (0x00000010)
#define RIO_MC_MSK_CFG_CMD_DEL      (0x00000020)
#define RIO_MC_MSK_CFG_CMD_DEL_ALL  (0x00000040)
#define RIO_MC_MSK_CFG_CMD_ADD_ALL  (0x00000050)

/* RIO_MC_CON_SEL : Register Bits Masks Definitions */
#define RIO_MC_CON_SEL_MASK    (0x0000ffff)
#define RIO_MC_CON_SEL_DEV8    (0x00ff0000)
#define RIO_MC_CON_SEL_DEV16   (0xff000000)

/* RIO_MC_CON_OP : Register Bits Masks Definitions */
#define RIO_MC_CON_OP_PRESENT  (0x00000001)
#define RIO_MC_CON_OP_CMD      (0x00000060)
#define RIO_MC_CON_OP_DEV16M   (0x00000080)
#define RIO_MC_CON_OP_PORT     (0x0000ff00)
#define RIO_MC_CON_OP_BLK      (0xffff0000)

#define RIO_MC_CON_OP_CMD_READ   (0x00000000)
#define RIO_MC_CON_OP_CMD_DEL    (0x00000040)
#define RIO_MC_CON_OP_CMD_ADD    (0x00000060)

/* Definitions for RapidIO standard extension blocks */
/* Each block has a defined header register */

#define RIO_EFB_T         (0x0000ffff)
#define RIO_EFB_NEXT      (0xffff0000)
#define RIO_EFB_NO_NEXT   (0x00000000)

#define RIO_EFB_T_FIXME		(0xDEAD)
#define RIO_EFB_T_SP_EP		(0x0001)
#define RIO_EFB_T_SP_EP_SAER	(0x0002)
#define RIO_EFB_T_SP_NOEP	(0x0003)
#define RIO_EFB_SER_EP_ID	(0x0004) /* [VI] LP/Serial EP Devices */
#define RIO_EFB_SER_EP_REC_ID	(0x0005) /* [VI] LP/Serial EP Recovery Devices */
#define RIO_EFB_SER_EP_FREE_ID	(0x0006) /* [VI] LP/Serial EP Free Devices */
#define RIO_EFB_T_SP_NOEP_SAER	(0x0009)

#define RIO_EFB_T_SP_EP3           (0x0011)
#define RIO_EFB_T_SP_EP3_SAER      (0x0012)
#define RIO_EFB_T_SP_NOEP3         (0x0013)
#define RIO_EFB_T_SP_NOEP3_SAER    (0x0019)

#define RIO_EFB_T_EMHS  (0x0007)  /* Error Management & Hot Swap */
#define RIO_EFB_T_HS    (0x0017)  /* Hot Swap */
#define RIO_EFB_T_VC    (0x000A)  /* Virtual Channel Support */
#define RIO_EFB_T_VOQ   (0x000B)  /* VoQ Backpressure Support */
#define RIO_EFB_T_LANE  (0x000D)  /* Lane Status */
#define RIO_EFB_T_RT    (0x000E)  /* Routing Table */
#define RIO_EFB_T_TS    (0x000F)  /* Timestamp Synchronization */
#define RIO_EFB_T_MISC  (0x0010)  /* Miscellaneous Register Block */

#define RIO_EFB_GET_NEXT(x) ((x & RIO_EFB_NEXT) >> 16)
#define RIO_EFB_ID(x) (x & RIO_EFB_T)


#define RIO_PEF_EXT_FEATURES            (0x00000008) /* [I] EFT_PTR valid */

/* LP-Serial Port Extended Features Block Register Addresses
 * 
 * Parameter meaning:
 * b - base address of the extended features block,
 * t - type of the extended features block
 * p - the port number for per-port registers.
 *
 * Note: Depending on the block type encoded in the header register,
 * some registers may not appear in the block.
 *
 * EP - Processing element has Endpoint functionality
 * NOEP - Processing element does not have an Endpoint.
 *        Generally, this means it's a switch.
 * SAER - Software Assisted Error Recovery
 */

#define RIO_SP3_VLD(t)  (((t == RIO_EFB_T_SP_EP3       ) || \
			  (t == RIO_EFB_T_SP_EP3_SAER  ) || \
			  (t == RIO_EFB_T_SP_NOEP3     ) || \
			  (t == RIO_EFB_T_SP_NOEP3_SAER))?1:0)

#define RIO_SP_STEP(t) (RIO_SP3_VLD(t)?0x40:0x20)

#define RIO_SP_VLD(t)  (((t == RIO_EFB_T_SP_EP        ) || \
		 	 (t == RIO_EFB_T_SP_EP_SAER   ) || \
		 	 (t == RIO_EFB_T_SP_NOEP      ) || \
		 	 (t == RIO_EFB_T_SP_NOEP_SAER ) || \
			RIO_SP3_VLD(t))?1:0)

#define RIO_SP3_SAER(t)  (((t == RIO_EFB_T_SP_EP3_SAER  ) || \
		 	   (t == RIO_EFB_T_SP_NOEP3_SAER ))?1:0)

#define RIO_SP12_SAER(t) (((t == RIO_EFB_T_SP_EP_SAER  ) || \
		 	   (t == RIO_EFB_T_SP_NOEP_SAER))?1:0)

#define RIO_SP_SAER(t)  (( RIO_SP12_SAER(t) || RIO_SP3_SAER(t))?1:0)

#define RIO_SP_EFB_HEAD(b)       (b)
#define RIO_SP_LT_CTL(b)         (b + 0x20)
#define RIO_SP_RTO_CTL(b)        (b + 0x24)
#define RIO_SP_GEN_CTL(b)        (b + 0x3c)

#define RIO_SPX_LM_REQ(b,t,p)    ((b+0x40+(RIO_SP_STEP(t)*p))*RIO_SP_SAER(t))
#define RIO_SPX_LM_RESP(b,t,p)   ((b+0x44+(RIO_SP_STEP(t)*p))*RIO_SP_SAER(t))
#define RIO_SPX_ACKID_ST(b,t,p)  ((b+0x48+(RIO_SP_STEP(t)*p))*RIO_SP12_SAER(t))
#define RIO_SPX_CTL2(b,t,p)      ((b+0x54+(RIO_SP_STEP(t)*p))*RIO_SP_VLD(t))
#define RIO_SPX_ERR_STAT(b,t,p)  ((b+0x58+(RIO_SP_STEP(t)*p))*RIO_SP_VLD(t))
#define RIO_SPX_CTL(b,t,p)       ((b+0x5c+(RIO_SP_STEP(t)*p))*RIO_SP_VLD(t))
#define RIO_SPX_OUT_ACKID(b,t,p) ((b+0x60+(RIO_SP_STEP(t)*p))*RIO_SP3_SAER(t))
#define RIO_SPX_IN_ACKID(b,t,p)  ((b+0x64+(RIO_SP_STEP(t)*p))*RIO_SP3_SAER(t))
#define RIO_SPX_PWR_MGMT(b,t,p)  ((b+0x68+(RIO_SP_STEP(t)*p))*RIO_SP3_VLD(t))
#define RIO_SPX_LAT_OPT(b,t,p)   ((b+0x6c+(RIO_SP_STEP(t)*p))*RIO_SP3_VLD(t))
#define RIO_SPX_TMR_CTL(b,t,p)   ((b+0x70+(RIO_SP_STEP(t)*p))*RIO_SP3_VLD(t))
#define RIO_SPX_TMR_CTL2(b,t,p)  ((b+0x74+(RIO_SP_STEP(t)*p))*RIO_SP3_VLD(t))
#define RIO_SPX_TMR_CTL3(b,t,p)  ((b+0x78+(RIO_SP_STEP(t)*p))*RIO_SP3_VLD(t))

/* SP_MB_HEAD : Register Bits Masks Definitions */
#define RIO_SP_EFB_HEAD_TYPE     RIO_EFB_T
#define RIO_SP_EFB_HEAD_NEXT     RIO_EFB_NEXT

/* SP_LT_CTL : Register Bits Masks Definitions */
#define RIO_SP_LT_CTL_TVAL                                   (0xffffff00)
#define RIO_SP_LT_CTL_TVAL_MAX (RIO_SP_LT_CTL_TVAL >> 8)

#define THREE_SECS_IN_NSECS	((uint64_t)3000000000)
#define SIX_SECS_IN_NSECS	((uint64_t)6000000000)

#define RIO_SP_LT_CTL_MIN_GRAN	(THREE_SECS_IN_NSECS/RIO_SP_LT_CTL_TVAL_MAX)
#define RIO_SP_LT_CTL_MAX_GRAN	(SIX_SECS_IN_NSECS/RIO_SP_LT_CTL_TVAL_MAX)

/* SP_RTO_CTL : Register Bits Masks Definitions */
#define RIO_SP_RTO_CTL_TVAL                                  (0xffffff00)
#define RIO_SP_RTO_CTL_TVAL_MAX (RIO_SP_RTO_CTL_TVAL >> 8)

#define RIO_SP_RTO_CTL_MIN_GRAN	(THREE_SECS_IN_NSECS/RIO_SP_RTO_CTL_TVAL_MAX)
#define RIO_SP_RTO_CTL_MAX_GRAN	(SIX_SECS_IN_NSECS/RIO_SP_RTO_CTL_TVAL_MAX)

/* RIO_SP_GEN_CTL : Register Bits Masks Definitions */
#define RIO_SP_GEN_CTL_DISC      (0x20000000)
#define RIO_SP_GEN_CTL_MAST_EN   (0x40000000)
#define RIO_SP_GEN_CTL_HOST      (0x80000000)

/* RIO_SPX_LM_REQ : Register Bits Masks Definitions */
#define RIO_SPX_LM_REQ_CMD           (0x00000007)
#define RIO_SPX_LM_REQ_CMD_RST_PT    (0x00000002)
#define RIO_SPX_LM_REQ_CMD_RST_DEV   (0x00000003)
#define RIO_SPX_LM_REQ_CMD_LR_IS     (0x00000004)

/* RIO_SPX_LM_RESP : Register Bits Masks Definitions */
typedef uint32_t RIO_SPX_LM_RESP_STAT_T;
#define RIO_SPX_LM_RESP_STAT12                 (0x0000001f)
#define RIO_SPX_LM_RESP_ACK_ID3                (0x0001ffe0)
#define RIO_SPX_LM_RESP_ACK_ID2                (0x000007e0)
#define RIO_SPX_LM_RESP_ACK_ID1                (0x000003e0)
#define RIO_SPX_LM_RESP_STAT3                  (0x1ffe0000)
#define RIO_SPX_LM_RESP_VLD                    (0x80000000)

#define RIO_SPX_LM_RESP_IS_VALID(x) ((x & RIO_SPX_LM_RESP_VLD)?1:0)

#define RIO_SPX_LM_RESP_STAT12_ERR             (0x00000002)
#define RIO_SPX_LM_RESP_STAT12_IRS             (0x00000004)
#define RIO_SPX_LM_RESP_STAT12_IES             (0x00000005)
#define RIO_SPX_LM_RESP_STAT12_NOERR           (0x00000010)

#define RIO_SPX_LM_RESP_ACK_ID(idle,v) \
	(((3 == idle)?(v & RIO_SPX_LM_RESP_ACK_ID3): \
	  (2 == idle)?(v & RIO_SPX_LM_RESP_ACK_ID2): \
	  (1 == idle)?(v & RIO_SPX_LM_RESP_ACK_ID1): \
	  0xFFFFFFE0) >> 5)

#define RIO_SPX_LM_RESP_STAT3_I_STAT           (0x0C000000)
#define RIO_SPX_LM_RESP_STAT3_I_STAT_NOERR     (0x00000000)
#define RIO_SPX_LM_RESP_STAT3_I_STAT_IRS       (0x04000000)
#define RIO_SPX_LM_RESP_STAT3_I_STAT_IES       (0x08000000)
#define RIO_SPX_LM_RESP_STAT3_I_STAT_FAIL      (0x0C000000)
#define RIO_SPX_LM_RESP_STAT3_I_EN             (0x02000000)
#define RIO_SPX_LM_RESP_STAT3_O_STAT           (0x00C00000)
#define RIO_SPX_LM_RESP_STAT3_O_STAT_NOERR     (0x00000000)
#define RIO_SPX_LM_RESP_STAT3_O_STAT_IRS       (0x00400000)
#define RIO_SPX_LM_RESP_STAT3_O_STAT_IES       (0x00800000)
#define RIO_SPX_LM_RESP_STAT3_O_STAT_FAIL      (0x00C00000)
#define RIO_SPX_LM_RESP_STAT3_O_EN             (0x00200000)
#define RIO_SPX_LM_RESP_STAT3_PW_PEND          (0x00100000)
#define RIO_SPX_LM_RESP_STAT3_O_FAIL           (0x00080000)

/* RIO_SPX_ACKID_ST : Register Bits Masks Definitions */
#define RIO_SPX_ACKID_ST_OUTB        (0x0000003f)
#define RIO_SPX_ACKID_ST_OUTST       (0x00003f00)
#define RIO_SPX_ACKID_ST_INB         (0x3f000000)
#define RIO_SPX_ACKID_ST_CLR         (0x80000000)

/* RIO_SPX_CTL2 : Register Bits Masks Definitions */
#define RIO_SPX_CTL2_RTEC_EN     (0x00000001)
#define RIO_SPX_CTL2_RTEC        (0x00000002)
#define RIO_SPX_CTL2_D_SCRM_DIS  (0x00000004)
#define RIO_SPX_CTL2_INACT_LN_EN (0x00000008)
#define RIO_SPX_CTL2_RETRAIN_EN  (0x00000010)
#define RIO_SPX_CTL2_GB_12P5_EN  (0x00001000)
#define RIO_SPX_CTL2_GB_12P5     (0x00002000)
#define RIO_SPX_CTL2_GB_10P3_EN  (0x00004000)
#define RIO_SPX_CTL2_GB_10P3     (0x00008000)
#define RIO_SPX_CTL2_GB_6P25_EN  (0x00010000)
#define RIO_SPX_CTL2_GB_6P25     (0x00020000)
#define RIO_SPX_CTL2_GB_5P0_EN   (0x00040000)
#define RIO_SPX_CTL2_GB_5P0      (0x00080000)
#define RIO_SPX_CTL2_GB_3P125_EN (0x00100000)
#define RIO_SPX_CTL2_GB_3P125    (0x00200000)
#define RIO_SPX_CTL2_GB_2P5_EN   (0x00400000)
#define RIO_SPX_CTL2_GB_2P5      (0x00800000)
#define RIO_SPX_CTL2_GB_1P25_EN  (0x01000000)
#define RIO_SPX_CTL2_GB_1P25     (0x02000000)
#define RIO_SPX_CTL2_BAUD_DISC   (0x08000000)
#define RIO_SPX_CTL2_BAUD_SEL    (0xf0000000)

#define RIO_SPX_CTL2_BAUD_SEL_NO_BR        (0x00000000)
#define RIO_SPX_CTL2_BAUD_SEL_1P25_BR      (0x10000000)
#define RIO_SPX_CTL2_BAUD_SEL_2P5_BR       (0x20000000)
#define RIO_SPX_CTL2_BAUD_SEL_3P125_BR     (0x30000000)
#define RIO_SPX_CTL2_BAUD_SEL_5P0_BR       (0x40000000)
#define RIO_SPX_CTL2_BAUD_SEL_6P25_BR      (0x50000000)
#define RIO_SPX_CTL2_BAUD_SEL_10P3125_BR   (0x60000000)
#define RIO_SPX_CTL2_BAUD_SEL_12P5_BR      (0x70000000)

/* RIO_SPX_ERR_STAT : Register Bits Masks Definitions */
typedef uint32_t RIO_SPX_ERR_STAT_T;
#define RIO_SPX_ERR_STAT_UNINIT    (0x00000001)
#define RIO_SPX_ERR_STAT_OK        (0x00000002)
#define RIO_SPX_ERR_STAT_ERR       (0x00000004)
#define RIO_SPX_ERR_STAT_UNAVL     (0x00000008)
#define RIO_SPX_ERR_STAT_PW_PEND   (0x00000010)
#define RIO_SPX_ERR_STAT_PW_DIS    (0x00000020)
#define RIO_SPX_ERR_STAT_IES       (0x00000100)
#define RIO_SPX_ERR_STAT_IE        (0x00000200)
#define RIO_SPX_ERR_STAT_IRS       (0x00000400)
#define RIO_SPX_ERR_STAT_OES       (0x00010000)
#define RIO_SPX_ERR_STAT_OE        (0x00020000)
#define RIO_SPX_ERR_STAT_ORS       (0x00040000)
#define RIO_SPX_ERR_STAT_ORC       (0x00080000)
#define RIO_SPX_ERR_STAT_ORE       (0x00100000)
#define RIO_SPX_ERR_STAT_DEGR      (0x02000000)
#define RIO_SPX_ERR_STAT_FAIL      (0x02000000)
#define RIO_SPX_ERR_STAT_DROP      (0x04000000)
#define RIO_SPX_ERR_STAT_TXFC      (0x08000000)
#define RIO_SPX_ERR_STAT_IDLE_SEQ  (0x30000000)
#define RIO_SPX_ERR_STAT_IDLE2_EN  (0x40000000)
#define RIO_SPX_ERR_STAT_IDLE2     (0x80000000)

#define RIO_SPX_ERR_STAT_IDLE_SEQ1   (0x00000000)
#define RIO_SPX_ERR_STAT_IDLE_SEQ2   (0x20000000)
#define RIO_SPX_ERR_STAT_IDLE_SEQ3   (0x30000000)

#define RIO_SPX_ERR_STAT_MASK      (0x07120214)
#define RIO_SPX_ERR_STAT_CLR_ALL   RIO_SPX_ERR_STAT_MASK

#define RIO_PORT_OK(x) ((x & RIO_SPX_ERR_STAT_OK)?1:0)

/* RIO_SPX_CTL : Register Bits Masks Definitions */
#define RIO_SPX_CTL_PTYP            (0x00000001)
#define RIO_SPX_CTL_LOCKOUT         (0x00000002)
#define RIO_SPX_CTL_DROP_EN         (0x00000004)
#define RIO_SPX_CTL_STOP_FAIL_EN    (0x00000008)
#define RIO_SPX_CTL_PTW_MAX2        (0x00003000)
#define RIO_SPX_CTL_PTW_OVER2       (0x0000C000)
#define RIO_SPX_CTL_FLOW_ARB        (0x00010000)
#define RIO_SPX_CTL_ENUM_B          (0x00020000)
#define RIO_SPX_CTL_FLOW_CTRL       (0x00040000)
#define RIO_SPX_CTL_MULT_CS         (0x00080000)
#define RIO_SPX_CTL_ERR_DIS         (0x00100000)
#define RIO_SPX_CTL_INP_EN          (0x00200000)
#define RIO_SPX_CTL_OTP_EN          (0x00400000)
#define RIO_SPX_CTL_PORT_DIS        (0x00800000)
#define RIO_SPX_CTL_PTW_OVER        (0x07000000)
#define RIO_SPX_CTL_PTW_INIT        (0x38000000)
#define RIO_SPX_CTL_PTW_MAX         (0xc0000000)

#define RIO_SPX_CTL_PTW_INIT_1X_L0  (0x00000000)
#define RIO_SPX_CTL_PTW_INIT_1X_LR  (0x08000000)
#define RIO_SPX_CTL_PTW_INIT_4X     (0x10000000)
#define RIO_SPX_CTL_PTW_INIT_2X     (0x18000000)

#define RIO_SPX_CTL_PTW_OVER_NONE     (0x00000000)
#define RIO_SPX_CTL_PTW_OVER_RSVD     (0x01000000)
#define RIO_SPX_CTL_PTW_OVER_1X_L0    (0x02000000)
#define RIO_SPX_CTL_PTW_OVER_1X_LR    (0x03000000)
#define RIO_SPX_CTL_PTW_OVER_IMP_SPEC (0x04000000)
#define RIO_SPX_CTL_PTW_OVER_2X_NO_4X (0x05000000)
#define RIO_SPX_CTL_PTW_OVER_4X_NO_2X (0x06000000)
#define RIO_SPX_CTL_PTW_OVER_NONE_2   (0x07000000)

#define RIO_SPX_CTL_PTW_MAX_2X      (0x80000000)
#define RIO_SPX_CTL_PTW_MAX_4X      (0x40000000)
#define RIO_SPX_CTL_PTW_MAX_LANES(x) ((x & RIO_SPX_CTL_PTW_MAX_4X) ? 4 : \
			((x & RIO_SPX_CTL_PTW_MAX_2X) ? 2 : 1)) \

/* RIO_SPX_OUT_ACKID : Register Bits Masks Definitions */
#define RIO_SPX_OUT_ACKID_OUTB_ACKID                     (0x00000fff)
#define RIO_SPX_OUT_ACKID_OUTSTD_ACKID                   (0x00fff000)
#define RIO_SPX_OUT_ACKID_CLR_OUTSTD_ACKID               (0x80000000)

/* RIO_SPX_IN_ACKID : Register Bits Masks Definitions */
#define RIO_SPX_IN_ACKID_INB_ACKID                       (0x00000fff)

/* RIO_SPX_PWR_MGMT : Register Bits Masks Definitions */
#define RIO_SPX_PWR_MGMT_LP_TX_STATUS                  (0x000000c0)
#define RIO_SPX_PWR_MGMT_CHG_LP_TX_WIDTH               (0x00000700)
#define RIO_SPX_PWR_MGMT_MY_TX_STATUS                  (0x00001800)
#define RIO_SPX_PWR_MGMT_CHG_MY_TX_WIDTH               (0x0000e000)
#define RIO_SPX_PWR_MGMT_RX_WIDTH_STATUS               (0x00070000)
#define RIO_SPX_PWR_MGMT_TX_WIDTH_STATUS               (0x00380000)
#define RIO_SPX_PWR_MGMT_ASYM_MODE_EN                  (0x07c00000)
#define RIO_SPX_PWR_MGMT_ASYM_MODE_SUP                 (0xf8000000)

/* RIO_SPX_LAT_OPT : Register Bits Masks Definitions */
#define RIO_SPX_LAT_OPT_PNA_ERR_REC_EN                   (0x00400000)
#define RIO_SPX_LAT_OPT_MULT_ACK_EN                      (0x00800000)
#define RIO_SPX_LAT_OPT_PNA_ACKID                        (0x20000000)
#define RIO_SPX_LAT_OPT_PNA_ERR_REC                      (0x40000000)
#define RIO_SPX_LAT_OPT_MULT_ACK                         (0x80000000)

/* RIO_SPX_TMR_CTL : Register Bits Masks Definitions */
#define RIO_SPX_TMR_CTL_EMPHASIS_CMD_TIMEOUT                 (0x000000ff)
#define RIO_SPX_TMR_CTL_CW_CMPLT_TMR                         (0x0000ff00)
#define RIO_SPX_TMR_CTL_DME_WAIT_FRAMES                      (0x00ff0000)
#define RIO_SPX_TMR_CTL_DME_CMPLT_TMR                        (0xff000000)

/* RIO_SPX_TMR_CTL2 : Register Bits Masks Definitions */
#define RIO_SPX_TMR_CTL2_RECOVERY_TMR                        (0x0000ff00)
#define RIO_SPX_TMR_CTL2_DISCOVERY_CMPLT_TMR                 (0x00ff0000)
#define RIO_SPX_TMR_CTL2_RETRAIN_CMPLT_TMR                   (0xff000000)

/* RIO_SPX_TMR_CTL3 : Register Bits Masks Definitions */
#define RIO_SPX_TMR_CTL3_KEEP_ALIVE_INTERVAL                 (0x000003ff)
#define RIO_SPX_TMR_CTL3_KEEP_ALIVE_PERIOD                   (0x0000fc00)
#define RIO_SPX_TMR_CTL3_RX_WIDTH_CMD_TIMEOUT                (0x00ff0000)
#define RIO_SPX_TMR_CTL3_TX_WIDTH_CMD_TIMEOUT                (0xff000000)

/* Error Management & Hot Swap Extended Features Block Register Addresses
*/

#define RIO_EMHS_EM(t) ((t==RIO_EFB_T_EMHS)?1:0)
#define RIO_EMHS_HS(t) ((t==RIO_EFB_T_HS)?1:0)

#define RIO_EMHS_EFB_HEAD(b)	(b+0x000)
#define RIO_EMHS_INFO(b)	(b+0x004)
#define RIO_EMHS_LL_DET(b)	(b+0x008)
#define RIO_EMHS_LL_EN(b)	(b+0x00c)
#define RIO_EMHS_LL_H_ADDR(b)	(b+0x010)
#define RIO_EMHS_LL_ADDR(b)	(b+0x014)
#define RIO_EMHS_LL_ID(b)	(b+0x018)
#define RIO_EMHS_LL_CTRL(b)	(b+0x01c)
#define RIO_EMHS_DEV32_DEST(b)	(b+0x020)
#define RIO_EMHS_DEV32_SRC(b)	(b+0x024)
#define RIO_EMHS_PW_DESTID(b)	(b+0x028)
#define RIO_EMHS_TTL(b)		(b+0x02c)
#define RIO_EMHS_PW_DEV32(b)	(b+0x030)
#define RIO_EMHS_PW_TRAN_CTL(b)	(b+0x034)

#define RIO_EMHS_SPX_ERR_DET(b,p)     (b+0x040 + 0x040*(p))
#define RIO_EMHS_SPX_RATE_EN(b,p)     (b+0x044 + 0x040*(p))
#define RIO_EMHS_SPX_ATTR(b,t,p)     ((b+0x048 + 0x040*(p))*RIO_EMHS_EM(t))
#define RIO_EMHS_SPX_CAPT0(b,t,p)    ((b+0x04C + 0x040*(p))*RIO_EMHS_EM(t))
#define RIO_EMHS_SPX_CAPT1(b,t,p)    ((b+0x050 + 0x040*(p))*RIO_EMHS_EM(t))
#define RIO_EMHS_SPX_CAPT2(b,t,p)    ((b+0x054 + 0x040*(p))*RIO_EMHS_EM(t))
#define RIO_EMHS_SPX_CAPT3(b,t,p)    ((b+0x058 + 0x040*(p))*RIO_EMHS_EM(t))
#define RIO_EMHS_SPX_CAPT4(b,t,p)    ((b+0x05C + 0x040*(p))*RIO_EMHS_EM(t))
#define RIO_EMHS_SPX_RATE(b,t,p)     ((b+0x068 + 0x040*(p))*RIO_EMHS_EM(t))
#define RIO_EMHS_SPX_THRESH(b,t,p)   ((b+0x06C + 0x040*(p))*RIO_EMHS_EM(t))
#define RIO_EMHS_SPX_DLT(b,p)         (b+0x070 + 0x040*(p))
#define RIO_EMHS_SPX_FIFO(b,t,p)     ((b+0x07C + 0x040*(p))*RIO_EMHS_EM(t))

/* RIO_EMHS_EFB_HEAD : Register Bits Masks Definitions */
#define RIO_EMHS_EFB_HEAD_TYPE     RIO_EFB_T
#define RIO_EMHS_EFB_HEAD_NEXT     RIO_EFB_NEXT

/* RIO_EMHS_INFO : Register Bits Masks Definitions */
#define RIO_EMHS_INFO_HOT_SWAP      (0x40000000)
#define RIO_EMHS_INFO_NO_ERR_MGMT   (0x80000000)

/* RIO_EMHS_LL_DET : Register Bits Masks Definitions */
#define RIO_EMHS_LL_DET_ILL_TYPE    (0x00400000)
#define RIO_EMHS_LL_DET_UNS_RSP     (0x00800000)
#define RIO_EMHS_LL_DET_ILL_ID      (0x04000000)

/* RIO_EMHS_LL_EN : Register Bits Masks Definitions */
#define RIO_EMHS_LL_EN_ILL_TYPE    (0x00400000)
#define RIO_EMHS_LL_EN_UNS_RSP     (0x00800000)
#define RIO_EMHS_LL_EN_ILL_ID      (0x04000000)

/* RIO_EMHS_LL_H_ADDR : Register Bits Masks Definitions */
#define RIO_EMHS_LL_H_ADDR64     (0xffffffff)
#define RIO_EMHS_LL_H_ADDR50     (0x0000ffff)
#define RIO_EMHS_LL_H_ADDR34     (0x00000000)
#define RIO_EMHS_LL_H_MTC        (0x00000000)

/* RIO_EMHS_LL_ADDR : Register Bits Masks Definitions */
#define RIO_EMHS_LL_ADDR_XAMSBS  (0x00000003)
#define RIO_EMHS_LL_ADDR_MTC     (0x00FFFFF8)
#define RIO_EMHS_LL_ADDR_MEM     (0xFFFFFFF8)

/* RIO_EMHS_LL_ID : Register Bits Masks Definitions */
#define RIO_EMHS_LL_ID_SRC_DEV8     (0x000000ff)
#define RIO_EMHS_LL_ID_SRC_DEV16    (0x0000ff00)
#define RIO_EMHS_LL_ID_DEST_DEV8    (0x00ff0000)
#define RIO_EMHS_LL_ID_DEST_DEV16   (0xff000000) 

/* RIO_EMHS_LL_CTRL : Register Bits Masks Definitions */
#define RIO_EMHS_LL_CTRL_TT           (0x00000003)
#define RIO_EMHS_LL_CTRL_IMPL_SPEC    (0x0000fff0)
#define RIO_EMHS_LL_CTRL_MSG_INFO     (0x00ff0000)
#define RIO_EMHS_LL_CTRL_TTYPE        (0x0f000000)
#define RIO_EMHS_LL_CTRL_FTYPE        (0xf0000000)

/* RIO_EMHS_DEV32_DEST : Register Bits Masks Definitions */
#define RIO_EMHS_DEV32_DEST_MASK    (0xffffffff)

/* RIO_EMHS_DEV32_SRC : Register Bits Masks Definitions */
#define RIO_EMHS_DEV32_SRC_MASK     (0xffffffff)

/* RIO_EMHS_PW_DESTID : Register Bits Masks Definitions */
#define RIO_EMHS_PW_DESTID_32CTL         (0x00004000)
#define RIO_EMHS_PW_DESTID_16CTL         (0x00008000)
#define RIO_EMHS_PW_DESTID_DEV8          (0x00ff0000)
#define RIO_EMHS_PW_DESTID_DEV16         (0xffff0000)
#define GET_DEV8_FROM_PW_TGT_HW(x) ((x & RIO_EMHS_PW_DESTID_16CTL)? \
                        RIO_LAST_DEV8:((x & RIO_EMHS_PW_DESTID_DEV8) >> 16))
#define GET_DEV16_FROM_PW_TGT_HW(x) (!(x & RIO_EMHS_PW_DESTID_16CTL)? \
                RIO_LAST_DEV16:((x & RIO_EMHS_PW_DESTID_DEV16) >> 16))

/* RIO_EMHS_TTL : Register Bits Masks Definitions */
#define RIO_EMHS_TTL_MASK                (0xffff0000)

/* RIO_EMHS_PW_DEV32 : Register Bits Masks Definitions */
#define RIO_EMHS_PW_DEV32_MASK           (0xffffffff)

/* PW_TRAN_CTL : Register Bits Masks Definitions */
#define RIO_EMHS_PW_TRAN_CTL_PW_DIS      (0x00000001)


/* RIO_EMHS_SPX_ERR_DET : Register Bits Masks Definitions */
#define RIO_EMHS_SPX_ERR_DET_LINK_TO       (0x00000001)
#define RIO_EMHS_SPX_ERR_DET_CS_ACK_ILL    (0x00000002)
#define RIO_EMHS_SPX_ERR_DET_DELIN_ERR     (0x00000004)
#define RIO_EMHS_SPX_ERR_DET_PROT_ERR      (0x00000010)
#define RIO_EMHS_SPX_ERR_DET_LR_ACKID_ILL  (0x00000020)
#define RIO_EMHS_SPX_ERR_DET_BAD_OS        (0x00002000)
#define RIO_EMHS_SPX_ERR_DET_LODS          (0x00004000)
#define RIO_EMHS_SPX_ERR_DET_BAD_IDLE1     (0x00008000)
#define RIO_EMHS_SPX_ERR_DET_BAD_CHAR      (0x00010000)
#define RIO_EMHS_SPX_ERR_DET_PKT_ILL_SIZE  (0x00020000)
#define RIO_EMHS_SPX_ERR_DET_PKT_CRC_ERR   (0x00040000)
#define RIO_EMHS_SPX_ERR_DET_PKT_ILL_ACKID (0x00080000)
#define RIO_EMHS_SPX_ERR_DET_CS_NOT_ACC    (0x00100000)
#define RIO_EMHS_SPX_ERR_DET_CS_ILL_ID     (0x00200000)
#define RIO_EMHS_SPX_ERR_DET_CS_CRC_ERR    (0x00400000)
#define RIO_EMHS_SPX_ERR_DET_LINK_INIT     (0x10000000)
#define RIO_EMHS_SPX_ERR_DET_LINK_DEAD     (0x20000000)
#define RIO_EMHS_SPX_ERR_DET_LINK_UNINT    (0x40000000)
#define RIO_EMHS_SPX_ERR_DET_IMP_SPEC_ERR  (0x80000000)

/* RIO_EMHS_SPX_RATE_EN : Register Bits Masks Definitions */
#define RIO_EMHS_SPX_RATE_EN_LINK_TO       (0x00000001)
#define RIO_EMHS_SPX_RATE_EN_CS_ACK_ILL    (0x00000002)
#define RIO_EMHS_SPX_RATE_EN_DELIN_ERR     (0x00000004)
#define RIO_EMHS_SPX_RATE_EN_PROT_ERR      (0x00000010)
#define RIO_EMHS_SPX_RATE_EN_LR_ACKID_ILL  (0x00000020)
#define RIO_EMHS_SPX_RATE_EN_BAD_OS        (0x00002000)
#define RIO_EMHS_SPX_RATE_EN_LODS          (0x00004000)
#define RIO_EMHS_SPX_RATE_EN_BAD_IDLE1     (0x00008000)
#define RIO_EMHS_SPX_RATE_EN_BAD_CHAR      (0x00010000)
#define RIO_EMHS_SPX_RATE_EN_PKT_ILL_SIZE  (0x00020000)
#define RIO_EMHS_SPX_RATE_EN_PKT_CRC_ERR   (0x00040000)
#define RIO_EMHS_SPX_RATE_EN_PKT_ILL_ACKID (0x00080000)
#define RIO_EMHS_SPX_RATE_EN_CS_NOT_ACC    (0x00100000)
#define RIO_EMHS_SPX_RATE_EN_CS_ILL_ID     (0x00200000)
#define RIO_EMHS_SPX_RATE_EN_CS_CRC_ERR    (0x00400000)
#define RIO_EMHS_SPX_RATE_EN_LINK_INIT     (0x10000000)
#define RIO_EMHS_SPX_RATE_EN_LINK_DEAD     (0x20000000)
#define RIO_EMHS_SPX_RATE_EN_LINK_UNINT    (0x40000000)
#define RIO_EMHS_SPX_RATE_EN_IMP_SPEC_ERR  (0x80000000)

/* RIO_EMHS_SPX_ATTR : Register Bits Masks Definitions * */
#define RIO_EMHS_SPX_ATTR_VLD        (0x00000001)
#define RIO_EMHS_SPX_ATTR_TYPE       (0x1f000000)
#define RIO_EMHS_SPX_ATTR_INFO_TYPE  (0xe0000000)

/* RIO_EMHS_SPX_CAPT0 : Register Bits Masks Definitions */
#define RIO_EMHS_SPX_CAPT0_MASK      (0xffffffff)

/* RIO_EMHS_SPX_CAPT1 : Register Bits Masks Definitions */
#define RIO_EMHS_SPX_CAPT1_MASK      (0xffffffff)

/* RIO_EMHS_SPX_CAPT2 : Register Bits Masks Definitions */
#define RIO_EMHS_SPX_CAPT2_MASK      (0xffffffff)

/* RIO_EMHS_SPX_CAPT3 : Register Bits Masks Definitions */
#define RIO_EMHS_SPX_CAPT3_MASK      (0xffffffff)

/* RIO_EMHS_SPX_CAPT4 : Register Bits Masks Definitions */
#define RIO_EMHS_SPX_CAPT4_MASK      (0xffffffff)

/* RIO_EMHS_SPX_RATE : Register Bits Masks Definitions */
#define RIO_EMHS_SPX_RATE_CNT          (0x000000ff)
#define RIO_EMHS_SPX_RATE_PEAK         (0x0000ff00)
#define RIO_EMHS_SPX_RATE_RR           (0x00030000)
#define RIO_EMHS_SPX_RATE_RB           (0xff000000)

#define RIO_EMHS_SPX_RATE_RB_NONE           (0x00000000)
#define RIO_EMHS_SPX_RATE_RB_1_MS           (0x01000000)
#define RIO_EMHS_SPX_RATE_RB_10_MS          (0x02000000)
#define RIO_EMHS_SPX_RATE_RB_100_MS         (0x04000000)
#define RIO_EMHS_SPX_RATE_RB_1_SEC          (0x08000000)
#define RIO_EMHS_SPX_RATE_RB_10_SEC         (0x10000000)
#define RIO_EMHS_SPX_RATE_RB_100_SEC        (0x20000000)
#define RIO_EMHS_SPX_RATE_RB_1000_SEC       (0x40000000)
#define RIO_EMHS_SPX_RATE_RB_10000_SEC      (0x80000000)

#define RIO_EMHS_SPX_RATE_RR_LIM_2          (0x00000000)
#define RIO_EMHS_SPX_RATE_RR_LIM_4          (0x00010000)
#define RIO_EMHS_SPX_RATE_RR_LIM_16         (0x00020000)
#define RIO_EMHS_SPX_RATE_RR_LIM_NONE       (0x00030000)

/* RIO_EMHS_SPX_THRESH : Register Bits Masks Definitions */
#define RIO_EMHS_SPX_THRESH_DGRD            (0x00ff0000)
#define RIO_EMHS_SPX_THRESH_FAIL            (0xff000000)

/* RIO_EMHS_SPX_DLT : Register Bits Masks Definitions */
#define RIO_EMHS_SPX_DLT_TO                (0xffffff00)

/* RIO_EMHS_SPX_FIFO : Register Bits Masks Definitions */
#define RIO_EMHS_SPX_FIFO_LINK_TO       (0x00000001)
#define RIO_EMHS_SPX_FIFO_CS_ACK_ILL    (0x00000002)
#define RIO_EMHS_SPX_FIFO_DELIN_ERR     (0x00000004)
#define RIO_EMHS_SPX_FIFO_PROT_ERR      (0x00000010)
#define RIO_EMHS_SPX_FIFO_LR_ACKID_ILL  (0x00000020)
#define RIO_EMHS_SPX_FIFO_BAD_OS        (0x00002000)
#define RIO_EMHS_SPX_FIFO_LODS          (0x00004000)
#define RIO_EMHS_SPX_FIFO_BAD_IDLE1     (0x00008000)
#define RIO_EMHS_SPX_FIFO_BAD_CHAR      (0x00010000)
#define RIO_EMHS_SPX_FIFO_PKT_ILL_SIZE  (0x00020000)
#define RIO_EMHS_SPX_FIFO_PKT_CRC_ERR   (0x00040000)
#define RIO_EMHS_SPX_FIFO_PKT_ILL_ACKID (0x00080000)
#define RIO_EMHS_SPX_FIFO_CS_NOT_ACC    (0x00100000)
#define RIO_EMHS_SPX_FIFO_CS_ILL_ID     (0x00200000)
#define RIO_EMHS_SPX_FIFO_CS_CRC_ERR    (0x00400000)
#define RIO_EMHS_SPX_FIFO_LINK_INIT     (0x10000000)
#define RIO_EMHS_SPX_FIFO_LINK_DEAD     (0x20000000)
#define RIO_EMHS_SPX_FIFO_LINK_UNINT    (0x40000000)
#define RIO_EMHS_SPX_FIFO_IMP_SPEC_ERR  (0x80000000)

/* Error Management Standard Port-write Definitions
 *
 * A port write carries four 4-byte words of data:
 * 0 - RIO_COMPTAG (Component Tag value)
 * 1 - RIO_EMHS_SPX_ERR_DET(x) Port x Error Detect value
 * 2 - 8 bit port number for Port Error Detect Value, and
 *     24 implementation specific bits.
 * 3 - RIO_EMHS_LL_DET (Logical/Transport Layer Error Detect CSR)
*/

#define RIO_EMHS_PW_COMPTAG_IDX		0
#define RIO_EMHS_PW_P_ERR_DET_IDX	1
#define RIO_EMHS_PW_IMP_SPEC_IDX	2
#define RIO_EMHS_PW_LL_DET_IDX		3
#define RIO_EMHS_PW_BYTES		16
#define RIO_EMHS_PW_WORDS		(RIO_EMHS_PW_BYTES/4)

/* RIO_EMHS_PW_COMPTAG_IDX : Port Write Field Mask Definitions */
#define RIO_EMHS_PW_COMPTAG_CTAG RIO_COMPTAG_CTAG

/* RIO_EMHS_PW_P_ERR_DET_IDX : Port Write Field Mask Definitions */
#define RIO_EMHS_PW_P_ERR_DET_LINK_TO		RIO_EMHS_SPX_ERR_DET_LINK_TO
#define RIO_EMHS_PW_P_ERR_DET_CS_ACK_ILL	RIO_EMHS_SPX_ERR_DET_CS_ACK_ILL
#define RIO_EMHS_PW_P_ERR_DET_DELIN_ERR		RIO_EMHS_SPX_ERR_DET_DELIN_ERR
#define RIO_EMHS_PW_P_ERR_DET_PROT_ERR		RIO_EMHS_SPX_ERR_DET_PROT_ERR
#define RIO_EMHS_PW_P_ERR_DET_LR_ACKID_ILL	RIO_EMHS_SPX_ERR_DET_LR_ACKID_ILL
#define RIO_EMHS_PW_P_ERR_DET_BAD_OS		RIO_EMHS_SPX_ERR_DET_BAD_OS
#define RIO_EMHS_PW_P_ERR_DET_LODS		RIO_EMHS_SPX_ERR_DET_LODS
#define RIO_EMHS_PW_P_ERR_DET_BAD_IDLE1		RIO_EMHS_SPX_ERR_DET_BAD_IDLE1
#define RIO_EMHS_PW_P_ERR_DET_BAD_CHAR		RIO_EMHS_SPX_ERR_DET_BAD_CHAR
#define RIO_EMHS_PW_P_ERR_DET_PKT_ILL_SIZE	RIO_EMHS_SPX_ERR_DET_PKT_ILL_SIZE
#define RIO_EMHS_PW_P_ERR_DET_PKT_CRC_ERR	RIO_EMHS_SPX_ERR_DET_PKT_CRC_ERR
#define RIO_EMHS_PW_P_ERR_DET_PKT_ILL_ACKID	RIO_EMHS_SPX_ERR_DET_PKT_ILL_ACKID
#define RIO_EMHS_PW_P_ERR_DET_CS_NOT_ACC	RIO_EMHS_SPX_ERR_DET_CS_NOT_ACC
#define RIO_EMHS_PW_P_ERR_DET_CS_ILL_ID		RIO_EMHS_SPX_ERR_DET_CS_ILL_ID
#define RIO_EMHS_PW_P_ERR_DET_CS_CRC_ERR	RIO_EMHS_SPX_ERR_DET_CS_CRC_ERR
#define RIO_EMHS_PW_P_ERR_DET_LINK_INIT		RIO_EMHS_SPX_ERR_DET_LINK_INIT
#define RIO_EMHS_PW_P_ERR_DET_LINK_DEAD		RIO_EMHS_SPX_ERR_DET_LINK_DEAD
#define RIO_EMHS_PW_P_ERR_DET_LINK_UNINT	RIO_EMHS_SPX_ERR_DET_LINK_UNINT
#define RIO_EMHS_PW_P_ERR_DET_IMP_SPEC_ERR	RIO_EMHS_SPX_ERR_DET_IMP_SPEC_ERR

/* RIO_EMHS_PW_IMP_SPEC_IDX : Port Write Field Mask Definitions */
#define RIO_EMHS_PW_IMP_SPEC_PORT		0x000000FF
#define RIO_EMHS_PW_IMP_SPEC_BITS		0xFFFFFF00

/* RIO_EMHS_PW_LL_DET_IDX : Port Write Field Mask Definitions */
#define RIO_EMHS_PW_LL_DET_ILL_TYPE	RIO_EMHS_LL_DET_ILL_TYPE
#define RIO_EMHS_PW_LL_DET_UNS_RSP	RIO_EMHS_LL_DET_UNS_RSP
#define RIO_EMHS_PW_LL_DET_ILL_ID	RIO_EMHS_LL_DET_ILL_ID
#define RIO_EMHS_PW_LL_DET_IMP_SPEC	0x000000FF

/* RIO_EMHS_SPX_FIFO : Register Bits Masks Definitions */
/* Lane Status Extended Features Block Register Offsets and bit fields
*/

#define RIO_LN_EFB_HEAD(b)	(b+0x000)
#define RIO_LNX_ST0(b,l)	(b+0x010 + 0x020*(l))
#define RIO_LNX_ST1(b,l)	(b+0x014 + 0x020*(l))
#define RIO_LNX_ST2(b,l)	(b+0x018 + 0x020*(l))
#define RIO_LNX_ST3(b,l)	(b+0x01c + 0x020*(l))
#define RIO_LNX_ST4(b,l)	(b+0x020 + 0x020*(l))
#define RIO_LNX_ST5(b,l)	(b+0x024 + 0x020*(l))
#define RIO_LNX_ST6(b,l)	(b+0x028 + 0x020*(l))
#define RIO_LNX_ST7(b,l)	(b+0x02C + 0x020*(l))

/* RIO_EMHS_EFB_HEAD : Register Bits Masks Definitions */
#define RIO_EMHS_EFB_HEAD_TYPE     RIO_EFB_T
#define RIO_EMHS_EFB_HEAD_NEXT     RIO_EFB_NEXT

/* RIO_LNX_ST0 : Register Bits Masks Definitions */
#define RIO_LNX_ST0_STAT2_7                              (0x00000007)
#define RIO_LNX_ST0_STAT1                                (0x00000008)
#define RIO_LNX_ST0_CHG_TRN                              (0x00000040)
#define RIO_LNX_ST0_CHG_SYNC                             (0x00000080)
#define RIO_LNX_ST0_ERR_CNT                              (0x00000f00)
#define RIO_LNX_ST0_RX_RDY                               (0x00001000)
#define RIO_LNX_ST0_RX_SYNC                              (0x00002000)
#define RIO_LNX_ST0_RX_TRN                               (0x00004000)
#define RIO_LNX_ST0_RX_INV                               (0x00008000)
#define RIO_LNX_ST0_RX_TYPE                              (0x00030000)
#define RIO_LNX_ST0_TX_MODE                              (0x00040000)
#define RIO_LNX_ST0_TX_TYPE                              (0x00080000)
#define RIO_LNX_ST0_LANE_NUM                             (0x00f00000)
#define RIO_LNX_ST0_PORT_NUM                             (0xff000000)

/* RIO_LNX_ST1 : Register Bits Masks Definitions */
#define RIO_LNX_ST1_CWR_CMPLT                            (0x00000020)
#define RIO_LNX_ST1_CWR_FAIL                             (0x00000040)
#define RIO_LNX_ST1_CW_CMPLT                             (0x00000080)
#define RIO_LNX_ST1_CW_FAIL                              (0x00000100)
#define RIO_LNX_ST1_DME_CMPLT                            (0x00000200)
#define RIO_LNX_ST1_DME_FAIL                             (0x00000400)
#define RIO_LNX_ST1_TRAIN_TYPE                           (0x00003800)
#define RIO_LNX_ST1_SIG_LOST                             (0x00004000)
#define RIO_LNX_ST1_LP_SCRM                              (0x00008000)
#define RIO_LNX_ST1_LP_TAP_P1                            (0x00030000)
#define RIO_LNX_ST1_LP_TAP_M1                            (0x000c0000)
#define RIO_LNX_ST1_LP_LANE_NUM                          (0x00f00000)
#define RIO_LNX_ST1_LP_WIDTH                             (0x07000000)
#define RIO_LNX_ST1_LP_RX_TRN                            (0x08000000)
#define RIO_LNX_ST1_IMPL_SPEC                            (0x10000000)
#define RIO_LNX_ST1_CHG                                  (0x20000000)
#define RIO_LNX_ST1_INFO_OK                              (0x40000000)
#define RIO_LNX_ST1_IDLE_RX                              (0x80000000)

/* RIO_LNX_ST2 : Register Bits Masks Definitions */
#define RIO_LNX_ST2_LP_RX_W_NACK                         (0x00000004)
#define RIO_LNX_ST2_LP_RX_W_ACK                          (0x00000008)
#define RIO_LNX_ST2_LP_RX_W_CMD                          (0x00000070)
#define RIO_LNX_ST2_LP_TRAINED                           (0x00000080)
#define RIO_LNX_ST2_LP_RX_LANE_RDY                       (0x00000100)
#define RIO_LNX_ST2_LP_RX_LANES_RDY                      (0x00000e00)
#define RIO_LNX_ST2_LP_RX_W                              (0x00007000)
#define RIO_LNX_ST2_LP_TX_1X                             (0x00008000)
#define RIO_LNX_ST2_LP_PORT_INIT                         (0x00010000)
#define RIO_LNX_ST2_LP_ASYM_EN                           (0x00020000)
#define RIO_LNX_ST2_LP_RETRN_EN                          (0x00040000)
#define RIO_LNX_ST2_LP_TX_ADJ_SUP                        (0x00080000)
#define RIO_LNX_ST2_LP_LANE                              (0x00f00000)
#define RIO_LNX_ST2_LP_PORT                              (0xff000000)

/* RIO_LNX_ST3 : Register Bits Masks Definitions */
#define RIO_LNX_ST3_SC_RSVD                              (0x00000ff0)
#define RIO_LNX_ST3_LP_L_SILENT                          (0x00001000)
#define RIO_LNX_ST3_LP_P_SILENT                          (0x00002000)
#define RIO_LNX_ST3_LP_RETRN                             (0x00004000)
#define RIO_LNX_ST3_LP_RETRN_RDY                         (0x00008000)
#define RIO_LNX_ST3_LP_RETRN_GNT                         (0x00010000)
#define RIO_LNX_ST3_LP_TX_EMPH_STAT                      (0x000e0000)
#define RIO_LNX_ST3_LP_TX_EMPH_CMD                       (0x00700000)
#define RIO_LNX_ST3_LP_TAP                               (0x07800000)
#define RIO_LNX_ST3_LP_TX_SC_REQ                         (0x08000000)
#define RIO_LNX_ST3_LP_TX_W_PEND                         (0x10000000)
#define RIO_LNX_ST3_LP_TX_W_REQ                          (0xe0000000)

/* Switch Routing TableExtended Features Block Register Offsets and bit fields
 *
 * FOR ROUTING TABLE ENTRY (RTE) VALUES, MASKS and MACROS, SEE RIO_RTE
 *
 * All registers that accept RTE, and each routing table entry in the
 * routing table groups, accepts these values.
 *
 * Multicast mask register values are defined below.
*/

#define RIO_RT_EFB_HEAD(b)        (b+0x000)
#define RIO_RT_BC_CTL(b)          (b+0x020)
#define RIO_RT_BC_MC(b)      (b+0x028)
#define RIO_RT_BC_LVL0(b)    (b+0x030)
#define RIO_RT_BC_LVL1(b)    (b+0x034)
#define RIO_RT_BC_LVL2(b)    (b+0x038)
#define RIO_RT_SPX_CTL(b,p)      (b+0x040 + 0x020*(p))
#define RIO_RT_SPX_MC(b,p)       (b+0x048 + 0x020*(p))
#define RIO_RT_SPX_LVL0(b,p)     (b+0x050 + 0x020*(p))
#define RIO_RT_SPX_LVL1(b,p)     (b+0x054 + 0x020*(p))
#define RIO_RT_SPX_LVL2(b,p)     (b+0x058 + 0x020*(p))

/* RIO_RT_EFB_HEAD : Register Bits Masks Definitions */
#define RIO_RT_EFB_HEAD_TYPE     RIO_EFB_T
#define RIO_RT_EFB_HEAD_NEXT     RIO_EFB_NEXT

/* RIO_RT_BC_CTL : Register Bits Masks Definitions */
#define RIO_RT_BC_CTL_MC_MASK_SZ     (0x03000000)
#define RIO_RT_BC_CTL_DEV32_RT_CTRL  (0x40000000)
#define RIO_RT_BC_CTL_THREE_LEVELS   (0x80000000)

#define RIO_RT_BC_CTL_MC_MASK_SZ8    (0x00000000)
#define RIO_RT_BC_CTL_MC_MASK_SZ16   (0x01000000)
#define RIO_RT_BC_CTL_MC_MASK_SZ32   (0x02000000)
#define RIO_RT_BC_CTL_MC_MASK_SZ64   (0x03000000)

#define RIO_RT_PTR_MASK (0X00fffc00)

/* RIO_RT_BC_MC : Register Bits Masks Definitions */
#define RIO_RT_BC_MC_MASK_PTR        RIO_RT_PTR_MASK
#define RIO_RT_BC_MC_MASK_CNT        (0xff000000)

/* RIO_RT_BC_LVL0 : Register Bits Masks Definitions */
#define RIO_RT_BC_LVL0_PTR RIO_RT_PTR_MASK
#define RIO_RT_BC_LVL0_CNT (0xff000000)

/* RIO_RT_BC_LVL1 : Register Bits Masks Definitions */
#define RIO_RT_BC_LVL1_PTR RIO_RT_PTR_MASK
#define RIO_RT_BC_LVL1_CNT (0xff000000)

/* RIO_RT_BC_LVL2 : Register Bits Masks Definitions */
#define RIO_RT_BC_LVL2_PTR RIO_RT_PTR_MASK
#define RIO_RT_BC_LVL2_CNT (0xff000000)

/* RIO_RT_SPX_CTL : Register Bits Masks Definitions */
#define RIO_RT_SPX_CTL_MC_MASK_SZ    (0x03000000)
#define RIO_RT_SPX_CTL_DEV32_RT_CTRL (0x40000000)
#define RIO_RT_SPX_CTL_THREE_LEVELS  (0x80000000)

#define RIO_RT_SPX_CTL_MC_MASK_SZ8  RIO_RT_BC_CTL_MC_MASK_SZ8
#define RIO_RT_SPX_CTL_MC_MASK_SZ16 RIO_RT_BC_CTL_MC_MASK_SZ16
#define RIO_RT_SPX_CTL_MC_MASK_SZ32 RIO_RT_BC_CTL_MC_MASK_SZ32
#define RIO_RT_SPX_CTL_MC_MASK_SZ64 RIO_RT_BC_CTL_MC_MASK_SZ64

/* RIO_RT_SPX_MC : Register Bits Masks Definitions */
#define RIO_RT_SPX_MC_MASK_PTR     RIO_RT_PTR_MASK
#define RIO_RT_SPX_MC_MAXK_CNT     (0xff000000)

/* RIO_RT_SPX_LVL0 : Register Bits Masks Definitions */
#define RIO_RT_SPX_LVL0_PTR        RIO_RT_PTR_MASK
#define RIO_RT_SPX_LVL0_CNT        (0xff000000)

/* RIO_RT_SPX_LVL1 : Register Bits Masks Definitions */
#define RIO_RT_SPX_LVL1_PTR        RIO_RT_PTR_MASK
#define RIO_RT_SPX_LVL1_CNT        (0xff000000)

/* RIO_RT_SPX_LVL2 : Register Bits Masks Definitions */
#define RIO_RT_SPX_LVL2_PTR        RIO_RT_PTR_MASK
#define RIO_RT_SPX_LVL2_CNT        (0xff000000)

/* Macros to determine addresses of routing table entries and MC MASK regs */

/* ptr : Value from RIO_RT_*_PTR field
 * g   : Group number
 * idx : Index of routing table entry
 * sz  : Value from RIO_RT_*_MC_MASK_SZ
 * m   : Mask number
 */

#define RIO_RT_RTE_ADDR(ptr,g,idx) ((ptr & RIO_RT_PTR_MASK) + \
					(g * RIO_RT_GRP_SZ * 4) + (idx * 4))

#define RIO_RT_MC_MASK_SZ(sz) ((sz == RIO_RT_SPX_CTL_MC_MASK_SZ8) ? 8 : \
		(sz == RIO_RT_SPX_CTL_MC_MASK_SZ16) ? 16 : \
		(sz == RIO_RT_SPX_CTL_MC_MASK_SZ32) ? 32 : \
		(sz == RIO_RT_SPX_CTL_MC_MASK_SZ64) ? 64 : 0xFFFFFFFF)

#define RIO_RT_MC_SET_MASK_ADDR(ptr,sz,m) ((ptr & RIO_RT_PTR_MASK) + \
					(RIO_RT_MC_MASK_SZ(sz) * m))

#define RIO_RT_MC_CLR_MASK_ADDR(ptr,sz,m)  ((ptr & RIO_RT_PTR_MASK) + \
			((RIO_RT_MC_MASK_SZ(sz)) * m) + \
			((RIO_RT_MC_MASK_SZ(sz)) / 2))

#ifdef __cplusplus
}
#endif

#endif /* __RIO_STANDARD_H__ */

