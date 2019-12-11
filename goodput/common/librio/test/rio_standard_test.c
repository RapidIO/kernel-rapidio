/*
 ************************************************************************
 Copyright (c) 2017, Integrated Device Technology Inc.
 Copyright (c) 2017, RapidIO Trade Association
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 l of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this l of conditions and the following disclaimer in the documentation
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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <stdarg.h>
#include <setjmp.h>
#include "cmocka.h"

#include "rio_standard.h"
#include "did.h"

#ifdef __cplusplus
extern "C" {
#endif

static void assumptions(void **state)
{
	assert_int_equal(sizeof(RIO_PE_FEAT_T), sizeof(uint32_t));
	assert_int_equal(sizeof(RIO_PE_ADDR_T), sizeof(uint32_t));

	assert_int_equal(PE_IS_SW(RIO_PE_FEAT_SW), 0x10000000);
	assert_int_equal(PE_IS_PROC(RIO_PE_FEAT_PROC), 0x20000000);
	assert_int_equal(PE_IS_MEM(RIO_PE_FEAT_MEM), 0x40000000);
	assert_int_equal(PE_IS_BRIDGE(RIO_PE_FEAT_BRDG), 0x80000000);

	assert_int_equal(RIO_EFB_T_SP_EP, 0x01);
	assert_int_equal(RIO_EFB_T_SP_EP_SAER, 0x02);
	assert_int_equal(RIO_EFB_T_SP_NOEP, 0x03);
	assert_int_equal(RIO_EFB_T_SP_NOEP_SAER, 0x09);
	assert_int_equal(RIO_EFB_T_SP_EP3, 0x11);
	assert_int_equal(RIO_EFB_T_SP_EP3_SAER, 0x12);
	assert_int_equal(RIO_EFB_T_SP_NOEP3, 0x13);
	assert_int_equal(RIO_EFB_T_SP_NOEP3_SAER, 0x19);
	assert_int_equal(RIO_EFB_T_EMHS, 0x07);
	assert_int_equal(RIO_EFB_T_HS, 0x17);
	assert_int_equal(RIO_EFB_T_VC, 0x0A);
	assert_int_equal(RIO_EFB_T_VOQ, 0x0B);
	assert_int_equal(RIO_EFB_T_LANE, 0x0D);
	assert_int_equal(RIO_EFB_T_RT, 0x0E);
	assert_int_equal(RIO_EFB_T_TS, 0x0F);
	assert_int_equal(RIO_EFB_T_MISC, 0x10);

        assert_int_equal(0x08000000, RIO_SPX_ERR_STAT_TXFC);

        assert_int_equal(0x00001000, RIO_SPX_CTL2_GB_12P5_EN);
        assert_int_equal(0x00002000, RIO_SPX_CTL2_GB_12P5);
        assert_int_equal(0x00004000, RIO_SPX_CTL2_GB_10P3_EN);
        assert_int_equal(0x00008000, RIO_SPX_CTL2_GB_10P3);
        assert_int_equal(0x00010000, RIO_SPX_CTL2_GB_6P25_EN);
        assert_int_equal(0x00020000, RIO_SPX_CTL2_GB_6P25);
        assert_int_equal(0x00040000, RIO_SPX_CTL2_GB_5P0_EN);
        assert_int_equal(0x00080000, RIO_SPX_CTL2_GB_5P0);
        assert_int_equal(0x00100000, RIO_SPX_CTL2_GB_3P125_EN);
        assert_int_equal(0x00200000, RIO_SPX_CTL2_GB_3P125);
        assert_int_equal(0x00400000, RIO_SPX_CTL2_GB_2P5_EN);
        assert_int_equal(0x00800000, RIO_SPX_CTL2_GB_2P5);
        assert_int_equal(0x01000000, RIO_SPX_CTL2_GB_1P25_EN);
        assert_int_equal(0x02000000, RIO_SPX_CTL2_GB_1P25);

        assert_int_equal(0x02000000, RIO_SPX_CTL2_GB_1P25);
        assert_int_equal(0x07000000, RIO_SPX_CTL_PTW_OVER);
        assert_int_equal(0x38000000, RIO_SPX_CTL_PTW_INIT);
        assert_int_equal(0xc0000000, RIO_SPX_CTL_PTW_MAX);

        assert_int_equal(0x00000000, RIO_SPX_CTL_PTW_INIT_1X_L0);
        assert_int_equal(0x08000000, RIO_SPX_CTL_PTW_INIT_1X_LR);
        assert_int_equal(0x10000000, RIO_SPX_CTL_PTW_INIT_4X);
        assert_int_equal(0x18000000, RIO_SPX_CTL_PTW_INIT_2X);

        assert_int_equal(0x00000000, RIO_SPX_CTL_PTW_OVER_NONE);
        assert_int_equal(0x01000000, RIO_SPX_CTL_PTW_OVER_RSVD);
        assert_int_equal(0x02000000, RIO_SPX_CTL_PTW_OVER_1X_L0);
        assert_int_equal(0x03000000, RIO_SPX_CTL_PTW_OVER_1X_LR);
        assert_int_equal(0x04000000, RIO_SPX_CTL_PTW_OVER_IMP_SPEC);
        assert_int_equal(0x05000000, RIO_SPX_CTL_PTW_OVER_2X_NO_4X);
        assert_int_equal(0x06000000, RIO_SPX_CTL_PTW_OVER_4X_NO_2X);
        assert_int_equal(0x07000000, RIO_SPX_CTL_PTW_OVER_NONE_2);

        assert_int_equal(0x80000000, RIO_SPX_CTL_PTW_MAX_2X);
        assert_int_equal(0x40000000, RIO_SPX_CTL_PTW_MAX_4X);

	assert_int_equal(3000000000, THREE_SECS_IN_NSECS);
	assert_int_equal(6000000000, SIX_SECS_IN_NSECS);
	assert_int_equal(0x00FFFFFF, RIO_SP_LT_CTL_TVAL_MAX);
	assert_int_equal(3000000000/0xFFFFFF, RIO_SP_LT_CTL_MIN_GRAN);
	assert_int_equal(6000000000/0xFFFFFF, RIO_SP_LT_CTL_MAX_GRAN);
	assert_int_equal(0x00FFFFFF, RIO_SP_RTO_CTL_TVAL_MAX);
	assert_int_equal(3000000000/0xFFFFFF, RIO_SP_RTO_CTL_MIN_GRAN);
	assert_int_equal(6000000000/0xFFFFFF, RIO_SP_RTO_CTL_MAX_GRAN);

	assert_int_equal(RIO_SPX_LM_REQ_CMD_RST_PT, 0x2);
	assert_int_equal(RIO_SPX_LM_REQ_CMD_RST_DEV, 0x3);
	assert_int_equal(RIO_SPX_LM_REQ_CMD_LR_IS, 0x4);

	assert_int_equal(sizeof(RIO_SW_PORT_INF_T), sizeof(uint32_t));
	assert_int_equal(sizeof(RIO_SRC_OPS_T), sizeof(uint32_t));
	assert_int_equal(sizeof(RIO_DST_OPS_T), sizeof(uint32_t));
	assert_int_equal(sizeof(pe_rt_val), sizeof(uint32_t));
	assert_int_equal(sizeof(RIO_SPX_LM_RESP_STAT_T), sizeof(uint32_t));
	assert_int_equal(sizeof(RIO_SPX_ERR_STAT_T), sizeof(uint32_t));

	(void)state; // unused
}

static void macros(void **state)
{
	assert_int_equal(RIO_ACCESS_PORT(0x9911), 0x11);
	assert_int_equal(RIO_AVAIL_PORTS(0x9911), 0x99);
	assert_true(RIO_PORT_IS_VALID(0x00, 0x9911));
	assert_true(RIO_PORT_IS_VALID(0x10, 0x9911));
	assert_true(RIO_PORT_IS_VALID(0x98, 0x9911));
	assert_false(RIO_PORT_IS_VALID(0x99, 0x9911));

	assert_int_equal(RIO_LCS_ADDR34(0x12345678, 0x71111111), 0x388888888);
	assert_int_equal(RIO_LCS_ADDR50(0x12347111, 0x11111111),
							0x3888888888888);
	assert_int_equal(RIO_LCS_ADDR66M(0x60000000), 0x3);
	assert_int_equal(RIO_LCS_ADDR66L(0xF1111111, 0x11111111),
							0x8888888888888888);

	assert_int_equal(GET_DEV8_FROM_HW(0x00FF0000), 0xFF);
	assert_int_equal(GET_DEV16_FROM_HW(0x0000FFFF), 0xFFFF);
	assert_int_equal(MAKE_HW_FROM_DEV8(0x000000FF), 0x00FF0000);
	assert_int_equal(MAKE_HW_FROM_DEV16(0x0000FFFF), 0x0000FFFF);
	assert_int_equal(MAKE_HW_FROM_DEV8N16(0x12, 0x00003456), 0x00123456);

	assert_int_equal(RIO_RTE_VAL, 0x3FF);

	assert_true(RIO_RTV_IS_PORT(RIO_RTE_PT_0));
	assert_true(RIO_RTV_IS_PORT(RIO_RTE_PT_LAST));
	assert_false(RIO_RTV_IS_PORT(RIO_RTE_PT_0 - 1));
	assert_false(RIO_RTV_IS_PORT(RIO_RTE_PT_LAST + 1));

	assert_true(RIO_RTV_IS_MC_MSK(RIO_RTE_MC_0));
	assert_true(RIO_RTV_IS_MC_MSK(RIO_RTE_MC_LAST));
	assert_false(RIO_RTV_IS_MC_MSK(RIO_RTE_MC_0 - 1));
	assert_false(RIO_RTV_IS_MC_MSK(RIO_RTE_MC_LAST + 1));

	assert_true(RIO_RTV_IS_LVL_GRP(RIO_RTE_LVL_G0));
	assert_true(RIO_RTV_IS_LVL_GRP(RIO_RTE_LVL_GLAST));
	assert_false(RIO_RTV_IS_LVL_GRP(RIO_RTE_LVL_G0 - 1));
	assert_false(RIO_RTV_IS_LVL_GRP(RIO_RTE_LVL_GLAST + 1));

	assert_int_equal(RIO_RTV_PORT(0), RIO_RTE_PT_0);
	assert_int_equal(RIO_RTV_PORT(0xFF), RIO_RTE_PT_LAST);
	assert_int_equal(RIO_RTV_PORT(0x100), 0x0FFFFFFF);
	assert_int_equal(RIO_RTV_PORT(0x100), RIO_RTE_BAD);

	assert_int_equal(RIO_RTV_MC_MSK(0), RIO_RTE_MC_0);
	assert_int_equal(RIO_RTV_MC_MSK(0xFF), RIO_RTE_MC_LAST);
	assert_int_equal(RIO_RTV_MC_MSK(0x100), 0x0FFFFFFF);
	assert_int_equal(RIO_RTV_MC_MSK(0x100), RIO_RTE_BAD);

	assert_int_equal(RIO_RTV_LVL_GRP(0), RIO_RTE_LVL_G0);
	assert_int_equal(RIO_RTV_LVL_GRP(0xFF), RIO_RTE_LVL_GLAST);
	assert_int_equal(RIO_RTV_LVL_GRP(0x100), 0x0FFFFFFF);
	assert_int_equal(RIO_RTV_LVL_GRP(0x100), RIO_RTE_BAD);

	assert_int_equal(RIO_RTV_GET_PORT(RIO_RTE_PT_0), 0);
	assert_int_equal(RIO_RTV_GET_PORT(RIO_RTE_PT_LAST), 0xFF);
	assert_int_equal(RIO_RTV_GET_PORT(RIO_RTE_PT_0 - 1), RIO_RTE_BAD);
	assert_int_equal(RIO_RTV_GET_PORT(RIO_RTE_PT_LAST + 1), RIO_RTE_BAD);

	assert_int_equal(RIO_RTV_GET_MC_MSK(RIO_RTE_MC_0), 0);
	assert_int_equal(RIO_RTV_GET_MC_MSK(RIO_RTE_MC_LAST), 0xFF);
	assert_int_equal(RIO_RTV_GET_MC_MSK(RIO_RTE_MC_0 - 1), RIO_RTE_BAD);
	assert_int_equal(RIO_RTV_GET_MC_MSK(RIO_RTE_MC_LAST + 1), RIO_RTE_BAD);

	assert_int_equal(RIO_RTV_GET_LVL_GRP(RIO_RTE_LVL_G0), 0);
	assert_int_equal(RIO_RTV_GET_LVL_GRP(RIO_RTE_LVL_GLAST), 0xFF);
	assert_int_equal(RIO_RTV_GET_LVL_GRP(RIO_RTE_LVL_G0 - 1), RIO_RTE_BAD);
	assert_int_equal(RIO_RTV_GET_LVL_GRP(RIO_RTE_LVL_GLAST + 1), RIO_RTE_BAD);

	assert_true(RIO_SP3_VLD(RIO_EFB_T_SP_EP3));
	assert_true(RIO_SP3_VLD(RIO_EFB_T_SP_EP3_SAER));
	assert_true(RIO_SP3_VLD(RIO_EFB_T_SP_NOEP3));
	assert_true(RIO_SP3_VLD(RIO_EFB_T_SP_NOEP3_SAER));

	assert_false(RIO_SP3_VLD(RIO_EFB_T_SP_EP));
	assert_false(RIO_SP3_VLD(RIO_EFB_T_SP_EP_SAER));
	assert_false(RIO_SP3_VLD(RIO_EFB_T_SP_NOEP));
	assert_false(RIO_SP3_VLD(RIO_EFB_T_SP_NOEP_SAER));

	assert_false(RIO_SP3_VLD(RIO_EFB_T_EMHS));
	assert_false(RIO_SP3_VLD(RIO_EFB_T_HS));
	assert_false(RIO_SP3_VLD(RIO_EFB_T_VC));
	assert_false(RIO_SP3_VLD(RIO_EFB_T_VOQ));
	assert_false(RIO_SP3_VLD(RIO_EFB_T_LANE));
	assert_false(RIO_SP3_VLD(RIO_EFB_T_RT));
	assert_false(RIO_SP3_VLD(RIO_EFB_T_TS));
	assert_false(RIO_SP3_VLD(RIO_EFB_T_MISC));

	assert_int_equal(RIO_EFB_GET_NEXT(0xFFFF0000), 0xFFFF);
	assert_int_equal(RIO_EFB_GET_NEXT(0x0000FFFF), 0x0000);
	assert_int_equal(RIO_EFB_ID(0xFFFF0000), 0x0000);
	assert_int_equal(RIO_EFB_ID(0x0000FFFF), 0xFFFF);

	assert_int_equal(RIO_SP_STEP(RIO_EFB_T_SP_EP3), 0x40);
	assert_int_equal(RIO_SP_STEP(RIO_EFB_T_SP_EP3_SAER), 0x40);
	assert_int_equal(RIO_SP_STEP(RIO_EFB_T_SP_NOEP3), 0x40);
	assert_int_equal(RIO_SP_STEP(RIO_EFB_T_SP_NOEP3_SAER), 0x40);

	assert_int_equal(RIO_SP_STEP(RIO_EFB_T_SP_EP), 0x20);
	assert_int_equal(RIO_SP_STEP(RIO_EFB_T_SP_EP_SAER), 0x20);
	assert_int_equal(RIO_SP_STEP(RIO_EFB_T_SP_NOEP), 0x20);
	assert_int_equal(RIO_SP_STEP(RIO_EFB_T_SP_NOEP_SAER), 0x20);

	assert_true(RIO_SP_VLD(RIO_EFB_T_SP_EP3));
	assert_true(RIO_SP_VLD(RIO_EFB_T_SP_EP3_SAER));
	assert_true(RIO_SP_VLD(RIO_EFB_T_SP_NOEP3));
	assert_true(RIO_SP_VLD(RIO_EFB_T_SP_NOEP3_SAER));
	assert_true(RIO_SP_VLD(RIO_EFB_T_SP_EP));
	assert_true(RIO_SP_VLD(RIO_EFB_T_SP_EP_SAER));
	assert_true(RIO_SP_VLD(RIO_EFB_T_SP_NOEP));
	assert_true(RIO_SP_VLD(RIO_EFB_T_SP_NOEP_SAER));

	assert_false(RIO_SP_VLD(RIO_EFB_T_EMHS));
	assert_false(RIO_SP_VLD(RIO_EFB_T_HS));
	assert_false(RIO_SP_VLD(RIO_EFB_T_VC));
	assert_false(RIO_SP_VLD(RIO_EFB_T_VOQ));
	assert_false(RIO_SP_VLD(RIO_EFB_T_LANE));
	assert_false(RIO_SP_VLD(RIO_EFB_T_RT));
	assert_false(RIO_SP_VLD(RIO_EFB_T_TS));
	assert_false(RIO_SP_VLD(RIO_EFB_T_MISC));

	assert_true(RIO_SP_SAER(RIO_EFB_T_SP_EP_SAER));
	assert_true(RIO_SP_SAER(RIO_EFB_T_SP_NOEP_SAER));
	assert_true(RIO_SP_SAER(RIO_EFB_T_SP_EP3_SAER));
	assert_true(RIO_SP_SAER(RIO_EFB_T_SP_NOEP3_SAER));

	assert_false(RIO_SP_SAER(RIO_EFB_T_SP_EP));
	assert_false(RIO_SP_SAER(RIO_EFB_T_SP_NOEP));
	assert_false(RIO_SP_SAER(RIO_EFB_T_SP_EP3));
	assert_false(RIO_SP_SAER(RIO_EFB_T_SP_NOEP3));
	assert_false(RIO_SP_SAER(RIO_EFB_T_EMHS));
	assert_false(RIO_SP_SAER(RIO_EFB_T_HS));
	assert_false(RIO_SP_SAER(RIO_EFB_T_VC));
	assert_false(RIO_SP_SAER(RIO_EFB_T_VOQ));
	assert_false(RIO_SP_SAER(RIO_EFB_T_LANE));
	assert_false(RIO_SP_SAER(RIO_EFB_T_RT));
	assert_false(RIO_SP_SAER(RIO_EFB_T_TS));
	assert_false(RIO_SP_SAER(RIO_EFB_T_MISC));

	assert_int_equal(RIO_SP_EFB_HEAD(0x100), 0x100);
	assert_int_equal(RIO_SP_LT_CTL(0x100), 0x120);
	assert_int_equal(RIO_SP_RTO_CTL(0x100), 0x124);
	assert_int_equal(RIO_SP_GEN_CTL(0x100), 0x13c);

	assert_int_equal(RIO_SPX_LM_REQ(0x100, RIO_EFB_T_LANE, 1), 0);
	assert_int_equal(RIO_SPX_LM_RESP(0x100, RIO_EFB_T_LANE, 1), 0);
	assert_int_equal(RIO_SPX_ACKID_ST(0x100, RIO_EFB_T_LANE, 1), 0);
	assert_int_equal(RIO_SPX_CTL2(0x100, RIO_EFB_T_LANE, 1), 0);
	assert_int_equal(RIO_SPX_ERR_STAT(0x100, RIO_EFB_T_LANE, 1), 0);
	assert_int_equal(RIO_SPX_CTL(0x100, RIO_EFB_T_LANE, 1), 0);
	assert_int_equal(RIO_SPX_OUT_ACKID(0x100, RIO_EFB_T_LANE, 1), 0);
	assert_int_equal(RIO_SPX_IN_ACKID(0x100, RIO_EFB_T_LANE, 1), 0);
	assert_int_equal(RIO_SPX_PWR_MGMT(0x100, RIO_EFB_T_LANE, 1), 0);
	assert_int_equal(RIO_SPX_LAT_OPT(0x100, RIO_EFB_T_LANE, 1), 0);
	assert_int_equal(RIO_SPX_TMR_CTL(0x100, RIO_EFB_T_LANE, 1), 0);
	assert_int_equal(RIO_SPX_TMR_CTL2(0x100, RIO_EFB_T_LANE, 1), 0);
	assert_int_equal(RIO_SPX_TMR_CTL3(0x100, RIO_EFB_T_LANE, 1), 0);

	assert_int_equal(RIO_SPX_LM_REQ(0x100, RIO_EFB_T_SP_EP, 1), 0);
	assert_int_equal(RIO_SPX_LM_RESP(0x100, RIO_EFB_T_SP_EP, 1), 0);
	assert_int_equal(RIO_SPX_ACKID_ST(0x100, RIO_EFB_T_SP_EP, 1), 0);
	assert_int_equal(RIO_SPX_CTL2(0x100, RIO_EFB_T_SP_EP, 1), 0x174);
	assert_int_equal(RIO_SPX_ERR_STAT(0x100, RIO_EFB_T_SP_EP, 1), 0x178);
	assert_int_equal(RIO_SPX_CTL(0x100, RIO_EFB_T_SP_EP, 1), 0x17C);
	assert_int_equal(RIO_SPX_OUT_ACKID(0x100, RIO_EFB_T_SP_EP, 1), 0);
	assert_int_equal(RIO_SPX_IN_ACKID(0x100, RIO_EFB_T_SP_EP, 1), 0);
	assert_int_equal(RIO_SPX_PWR_MGMT(0x100, RIO_EFB_T_SP_EP, 1), 0);
	assert_int_equal(RIO_SPX_LAT_OPT(0x100, RIO_EFB_T_SP_EP, 1), 0);
	assert_int_equal(RIO_SPX_TMR_CTL(0x100, RIO_EFB_T_SP_EP, 1), 0);
	assert_int_equal(RIO_SPX_TMR_CTL2(0x100, RIO_EFB_T_SP_EP, 1), 0);
	assert_int_equal(RIO_SPX_TMR_CTL3(0x100, RIO_EFB_T_SP_EP, 1), 0);

	assert_int_equal(RIO_SPX_LM_REQ(0x100, RIO_EFB_T_SP_EP_SAER, 1), 0x160);
	assert_int_equal(RIO_SPX_LM_RESP(0x100, RIO_EFB_T_SP_EP_SAER, 1), 0x164);
	assert_int_equal(RIO_SPX_ACKID_ST(0x100, RIO_EFB_T_SP_EP_SAER, 1), 0x168);
	assert_int_equal(RIO_SPX_CTL2(0x100, RIO_EFB_T_SP_EP_SAER, 1), 0x174);
	assert_int_equal(RIO_SPX_ERR_STAT(0x100, RIO_EFB_T_SP_EP_SAER, 1), 0x178);
	assert_int_equal(RIO_SPX_CTL(0x100, RIO_EFB_T_SP_EP_SAER, 1), 0x17C);
	assert_int_equal(RIO_SPX_OUT_ACKID(0x100, RIO_EFB_T_SP_EP_SAER, 1), 0);
	assert_int_equal(RIO_SPX_IN_ACKID(0x100, RIO_EFB_T_SP_EP_SAER, 1), 0);
	assert_int_equal(RIO_SPX_PWR_MGMT(0x100, RIO_EFB_T_SP_EP_SAER, 1), 0);
	assert_int_equal(RIO_SPX_LAT_OPT(0x100, RIO_EFB_T_SP_EP_SAER, 1), 0);
	assert_int_equal(RIO_SPX_TMR_CTL(0x100, RIO_EFB_T_SP_EP_SAER, 1), 0);
	assert_int_equal(RIO_SPX_TMR_CTL2(0x100, RIO_EFB_T_SP_EP_SAER, 1), 0);
	assert_int_equal(RIO_SPX_TMR_CTL3(0x100, RIO_EFB_T_SP_EP_SAER, 1), 0);

	assert_int_equal(RIO_SPX_LM_REQ(0x100, RIO_EFB_T_SP_EP3_SAER, 1), 0x180);
	assert_int_equal(RIO_SPX_LM_RESP(0x100, RIO_EFB_T_SP_EP3_SAER, 1), 0x184);
	assert_int_equal(RIO_SPX_ACKID_ST(0x100, RIO_EFB_T_SP_EP3_SAER, 1), 0);
	assert_int_equal(RIO_SPX_CTL2(0x100, RIO_EFB_T_SP_EP3_SAER, 1), 0x194);
	assert_int_equal(RIO_SPX_ERR_STAT(0x100, RIO_EFB_T_SP_EP3_SAER, 1), 0x198);
	assert_int_equal(RIO_SPX_CTL(0x100, RIO_EFB_T_SP_EP3_SAER, 1), 0x19C);
	assert_int_equal(RIO_SPX_OUT_ACKID(0x100, RIO_EFB_T_SP_EP3_SAER, 1), 0x1A0);
	assert_int_equal(RIO_SPX_IN_ACKID(0x100, RIO_EFB_T_SP_EP3_SAER, 1), 0x1A4);
	assert_int_equal(RIO_SPX_PWR_MGMT(0x100, RIO_EFB_T_SP_EP3_SAER, 1), 0x1A8);
	assert_int_equal(RIO_SPX_LAT_OPT(0x100, RIO_EFB_T_SP_EP3_SAER, 1), 0x1AC);
	assert_int_equal(RIO_SPX_TMR_CTL(0x100, RIO_EFB_T_SP_EP3_SAER, 1), 0x1B0);
	assert_int_equal(RIO_SPX_TMR_CTL2(0x100, RIO_EFB_T_SP_EP3_SAER, 1), 0x1B4);
	assert_int_equal(RIO_SPX_TMR_CTL3(0x100, RIO_EFB_T_SP_EP3_SAER, 1), 0x1B8);

	assert_true(RIO_SPX_LM_RESP_IS_VALID(0x80000000));
	assert_false(RIO_SPX_LM_RESP_IS_VALID(0x7FFFFFFF));

	assert_int_equal(RIO_SPX_LM_RESP_ACK_ID(3, 0x0001FFE0), 0x00000FFF);
	assert_int_equal(RIO_SPX_LM_RESP_ACK_ID(2, 0x000007E0), 0x0000003F);
	assert_int_equal(RIO_SPX_LM_RESP_ACK_ID(1, 0x000003E0), 0x0000001F);
	assert_int_equal(RIO_SPX_LM_RESP_ACK_ID(0, 0x000003E0), 0x07FFFFFF);

	assert_true(RIO_PORT_OK(2));
	assert_false(RIO_PORT_OK(~2));

	assert_int_equal(4, RIO_SPX_CTL_PTW_MAX_LANES(0xC0000000));
	assert_int_equal(2, RIO_SPX_CTL_PTW_MAX_LANES(0x80000000));
	assert_int_equal(1, RIO_SPX_CTL_PTW_MAX_LANES(0x00000000));

	assert_int_equal(RIO_EMHS_EFB_HEAD(0x1000), 0x1000);
	assert_int_equal(RIO_EMHS_INFO(0x1000), 0x1004);
	assert_int_equal(RIO_EMHS_LL_DET(0x1000), 0x1008);
	assert_int_equal(RIO_EMHS_LL_EN(0x1000), 0x100C);
	assert_int_equal(RIO_EMHS_LL_H_ADDR(0x1000), 0x1010);
	assert_int_equal(RIO_EMHS_LL_ADDR(0x1000), 0x1014);
	assert_int_equal(RIO_EMHS_LL_ID(0x1000), 0x1018);
	assert_int_equal(RIO_EMHS_LL_CTRL(0x1000), 0x101C);
	assert_int_equal(RIO_EMHS_DEV32_DEST(0x1000), 0x1020);
	assert_int_equal(RIO_EMHS_DEV32_SRC(0x1000), 0x1024);
	assert_int_equal(RIO_EMHS_PW_DESTID(0x1000), 0x1028);
	assert_int_equal(RIO_EMHS_TTL(0x1000), 0x102C);
	assert_int_equal(RIO_EMHS_PW_DEV32(0x1000), 0x1030);
	assert_int_equal(RIO_EMHS_PW_TRAN_CTL(0x1000), 0x1034);

	assert_int_equal(RIO_EMHS_SPX_ERR_DET(0x1000, 1), 0x1080);
	assert_int_equal(RIO_EMHS_SPX_RATE_EN(0x1000, 1), 0x1084);
	assert_int_equal(RIO_EMHS_SPX_ATTR(0x1000, RIO_EFB_T_EMHS, 1), 0x1088);
	assert_int_equal(RIO_EMHS_SPX_CAPT0(0x1000, RIO_EFB_T_EMHS, 1), 0x108C);
	assert_int_equal(RIO_EMHS_SPX_CAPT1(0x1000, RIO_EFB_T_EMHS, 1), 0x1090);
	assert_int_equal(RIO_EMHS_SPX_CAPT2(0x1000, RIO_EFB_T_EMHS, 1), 0x1094);
	assert_int_equal(RIO_EMHS_SPX_CAPT3(0x1000, RIO_EFB_T_EMHS, 1), 0x1098);
	assert_int_equal(RIO_EMHS_SPX_CAPT4(0x1000, RIO_EFB_T_EMHS, 1), 0x109C);
	assert_int_equal(RIO_EMHS_SPX_RATE(0x1000, RIO_EFB_T_EMHS, 1), 0x10A8);
	assert_int_equal(RIO_EMHS_SPX_THRESH(0x1000, RIO_EFB_T_EMHS, 1), 0x10AC);
	assert_int_equal(RIO_EMHS_SPX_DLT(0x1000, 1), 0x10B0);
	assert_int_equal(RIO_EMHS_SPX_FIFO(0x1000, RIO_EFB_T_EMHS, 1), 0x10BC);

	assert_int_equal(RIO_EMHS_SPX_ATTR(0x1000, RIO_EFB_T_HS, 1), 0);
	assert_int_equal(RIO_EMHS_SPX_CAPT0(0x1000, RIO_EFB_T_HS, 1), 0);
	assert_int_equal(RIO_EMHS_SPX_CAPT1(0x1000, RIO_EFB_T_HS, 1), 0);
	assert_int_equal(RIO_EMHS_SPX_CAPT2(0x1000, RIO_EFB_T_HS, 1), 0);
	assert_int_equal(RIO_EMHS_SPX_CAPT3(0x1000, RIO_EFB_T_HS, 1), 0);
	assert_int_equal(RIO_EMHS_SPX_CAPT4(0x1000, RIO_EFB_T_HS, 1), 0);
	assert_int_equal(RIO_EMHS_SPX_RATE(0x1000, RIO_EFB_T_HS, 1), 0);
	assert_int_equal(RIO_EMHS_SPX_THRESH(0x1000, RIO_EFB_T_HS, 1), 0);
	assert_int_equal(RIO_EMHS_SPX_FIFO(0x1000, RIO_EFB_T_HS, 1), 0);

	assert_int_equal(GET_DEV8_FROM_PW_TGT_HW(0x00810000), 0x81);
	assert_int_equal(GET_DEV8_FROM_PW_TGT_HW(0x00818000), 0xFF);
	assert_int_equal(GET_DEV16_FROM_PW_TGT_HW(0x80018000), 0x8001);
	assert_int_equal(GET_DEV16_FROM_PW_TGT_HW(0x80010000), 0xFFFF);

	assert_int_equal(RIO_LN_EFB_HEAD(0x300), 0x300);
	assert_int_equal(RIO_LNX_ST0(0x300, 1), 0x330);
	assert_int_equal(RIO_LNX_ST1(0x300, 1), 0x334);
	assert_int_equal(RIO_LNX_ST2(0x300, 1), 0x338);
	assert_int_equal(RIO_LNX_ST3(0x300, 1), 0x33C);
	assert_int_equal(RIO_LNX_ST4(0x300, 1), 0x340);
	assert_int_equal(RIO_LNX_ST5(0x300, 1), 0x344);
	assert_int_equal(RIO_LNX_ST6(0x300, 1), 0x348);
	assert_int_equal(RIO_LNX_ST7(0x300, 1), 0x34C);

	assert_int_equal(RIO_RT_EFB_HEAD(0x800), 0x800);
	assert_int_equal(RIO_RT_BC_CTL(0x800), 0x820);
	assert_int_equal(RIO_RT_BC_MC(0x800), 0x828);
	assert_int_equal(RIO_RT_BC_LVL0(0x800), 0x830);
	assert_int_equal(RIO_RT_BC_LVL1(0x800), 0x834);
	assert_int_equal(RIO_RT_BC_LVL2(0x800), 0x838);
	assert_int_equal(RIO_RT_SPX_CTL(0x800, 1), 0x860);
	assert_int_equal(RIO_RT_SPX_MC(0x800, 1), 0x868);
	assert_int_equal(RIO_RT_SPX_LVL0(0x800, 1), 0x870);
	assert_int_equal(RIO_RT_SPX_LVL1(0x800, 1), 0x874);
	assert_int_equal(RIO_RT_SPX_LVL2(0x800, 1), 0x878);

	assert_int_equal(RIO_RT_RTE_ADDR(0x00800400, 0, 0), 0x00800400);
	assert_int_equal(RIO_RT_RTE_ADDR(0x00800400, 2, 2), 0x00800C08);
	assert_int_equal(RIO_RT_RTE_ADDR(0x00801400, 4, 4), 0x00802410);
	assert_int_equal(RIO_RT_RTE_ADDR(0x00800000, 0xFF, 0xFF), 0x0083FFFC);

	assert_int_equal(RIO_RT_MC_SET_MASK_ADDR(0x00800400, RIO_RT_SPX_CTL_MC_MASK_SZ8, 0),
										0x800400);
	assert_int_equal(RIO_RT_MC_SET_MASK_ADDR(0x00800400, RIO_RT_SPX_CTL_MC_MASK_SZ16, 0),
										0x800400);
	assert_int_equal(RIO_RT_MC_SET_MASK_ADDR(0x00800400, RIO_RT_SPX_CTL_MC_MASK_SZ32, 0),
										0x800400);
	assert_int_equal(RIO_RT_MC_SET_MASK_ADDR(0x00800400, RIO_RT_SPX_CTL_MC_MASK_SZ64, 0),
										0x800400);
	assert_int_equal(RIO_RT_MC_SET_MASK_ADDR(0x00800000, RIO_RT_SPX_CTL_MC_MASK_SZ8, 1),
										0x800008);
	assert_int_equal(RIO_RT_MC_SET_MASK_ADDR(0x00800000, RIO_RT_SPX_CTL_MC_MASK_SZ16, 1),
										0x800010);
	assert_int_equal(RIO_RT_MC_SET_MASK_ADDR(0x00800000, RIO_RT_SPX_CTL_MC_MASK_SZ32, 1),
										0x800020);
	assert_int_equal(RIO_RT_MC_SET_MASK_ADDR(0x00800000, RIO_RT_SPX_CTL_MC_MASK_SZ64, 1),
										0x800040);

	assert_int_equal(RIO_RT_MC_CLR_MASK_ADDR(0x00800400, RIO_RT_SPX_CTL_MC_MASK_SZ8, 0),
										0x800404);
	assert_int_equal(RIO_RT_MC_CLR_MASK_ADDR(0x00800400, RIO_RT_SPX_CTL_MC_MASK_SZ16, 0),
										0x800408);
	assert_int_equal(RIO_RT_MC_CLR_MASK_ADDR(0x00800400, RIO_RT_SPX_CTL_MC_MASK_SZ32, 0),
										0x800410);
	assert_int_equal(RIO_RT_MC_CLR_MASK_ADDR(0x00800400, RIO_RT_SPX_CTL_MC_MASK_SZ64, 0),
										0x800420);
	assert_int_equal(RIO_RT_MC_CLR_MASK_ADDR(0x00800000, RIO_RT_SPX_CTL_MC_MASK_SZ8, 1),
										0x80000C);
	assert_int_equal(RIO_RT_MC_CLR_MASK_ADDR(0x00800000, RIO_RT_SPX_CTL_MC_MASK_SZ16, 1),
										0x800018);
	assert_int_equal(RIO_RT_MC_CLR_MASK_ADDR(0x00800000, RIO_RT_SPX_CTL_MC_MASK_SZ32, 1),
										0x800030);
	assert_int_equal(RIO_RT_MC_CLR_MASK_ADDR(0x00800000, RIO_RT_SPX_CTL_MC_MASK_SZ64, 1),
										0x800060);

	assert_int_not_equal(RIO_RT_MC_CLR_MASK_ADDR(0x00800000, 0x04000000, 0), 0x00800000);

	(void)state; // unused
}

int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
	cmocka_unit_test(assumptions),
	cmocka_unit_test(macros),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#ifdef __cplusplus
}
#endif
