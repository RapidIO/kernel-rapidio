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

#include "RapidIO_Driver_Utilities.h"

#define DEBUG_PRINTF 0

#ifdef __cplusplus
extern "C" {
#endif

static void assumptions(void **state)
{

	(void)state; // unused
}

static void rio_determine_op_fc_test(void **state)
{
	rio_pc_fc fc;

	// No link status, FC is unknown
	rio_determine_op_fc(&fc, 0);
	assert_int_equal(rio_pc_fc_last, fc);

	// Port uninit, FC is unknown
	rio_determine_op_fc(&fc, RIO_SPX_ERR_STAT_UNINIT);
	assert_int_equal(rio_pc_fc_last, fc);

	// Port OK, FC is RX
	rio_determine_op_fc(&fc, RIO_SPX_ERR_STAT_OK);
	assert_int_equal(rio_pc_fc_rx, fc);

	// Port OK & TX indication, FC is TX
	rio_determine_op_fc(&fc, RIO_SPX_ERR_STAT_OK | RIO_SPX_ERR_STAT_TXFC);
	assert_int_equal(rio_pc_fc_tx, fc);

	// No link status, FC is unknown
	rio_determine_op_fc(&fc, ~RIO_SPX_ERR_STAT_OK);
	assert_int_equal(rio_pc_fc_last, fc);

	// Port OK, FC is RX
	rio_determine_op_fc(&fc, ~RIO_SPX_ERR_STAT_TXFC);
	assert_int_equal(rio_pc_fc_rx, fc);

	// Port OK & TX indication, FC is TX
	rio_determine_op_fc(&fc, 0xFFFFFFFF);
	assert_int_equal(rio_pc_fc_tx, fc);

	(void)state; // unused
}

struct ls_en_op_t {
	uint32_t ls_en;
	uint32_t ls_op;
	rio_pc_ls_t ls;
};

static struct ls_en_op_t ls_chks[] = {
	{RIO_SPX_CTL2_GB_12P5_EN, RIO_SPX_CTL2_GB_12P5, rio_pc_ls_12p5},
	{RIO_SPX_CTL2_GB_10P3_EN, RIO_SPX_CTL2_GB_10P3, rio_pc_ls_10p3},
	{RIO_SPX_CTL2_GB_6P25_EN, RIO_SPX_CTL2_GB_6P25, rio_pc_ls_6p25},
	{RIO_SPX_CTL2_GB_5P0_EN, RIO_SPX_CTL2_GB_5P0, rio_pc_ls_5p0},
	{RIO_SPX_CTL2_GB_3P125_EN, RIO_SPX_CTL2_GB_3P125, rio_pc_ls_3p125},
	{RIO_SPX_CTL2_GB_2P5_EN, RIO_SPX_CTL2_GB_2P5, rio_pc_ls_2p5},
	{RIO_SPX_CTL2_GB_1P25_EN, RIO_SPX_CTL2_GB_1P25, rio_pc_ls_1p25},
	{0, 0, rio_pc_ls_last}
};

static void rio_determine_ls_test(void **state)
{
	rio_pc_ls_t ls;
	const int ls_chks_max = sizeof(ls_chks)/sizeof(ls_chks[0]);
	int i;

	for (i = 0; i < ls_chks_max; i++) {
		// No link status, FC is unknown
		rio_determine_ls(&ls, ls_chks[i].ls_en);
		assert_int_equal(rio_pc_ls_last, ls);

		rio_determine_ls(&ls, ls_chks[i].ls_op);
		assert_int_equal(rio_pc_ls_last, ls);

		rio_determine_ls(&ls, ls_chks[i].ls_en | ls_chks[i].ls_op);
		assert_int_equal(ls_chks[i].ls, ls);
	}

	(void)state; // unused
}

static void rio_determine_op_iseq_test(void **state)
{
	rio_pc_idle_seq	iseq;

	// No link status, idle sequence is unknown
	rio_determine_op_iseq(&iseq, 0);
	assert_int_equal(rio_pc_is_last, iseq);

	// Link uninit, idle sequence is unknown
	rio_determine_op_iseq(&iseq, RIO_SPX_ERR_STAT_UNINIT);
	assert_int_equal(rio_pc_is_last, iseq);

	// Link OK, idle sequence is IDLE1
	rio_determine_op_iseq(&iseq, RIO_SPX_ERR_STAT_OK);
	assert_int_equal(rio_pc_is_one, iseq);

	// Link OK, idle sequence is IDLE2
	rio_determine_op_iseq(&iseq, RIO_SPX_ERR_STAT_OK |
					RIO_SPX_ERR_STAT_IDLE_SEQ2);
	assert_int_equal(rio_pc_is_two, iseq);

	// Link OK, idle sequence is IDLE3
	rio_determine_op_iseq(&iseq, RIO_SPX_ERR_STAT_OK |
					RIO_SPX_ERR_STAT_IDLE_SEQ3);
	assert_int_equal(rio_pc_is_three, iseq);

	// Link OK, idle sequence is invalid
	rio_determine_op_iseq(&iseq, RIO_SPX_ERR_STAT_OK | 0x10000000);
	assert_int_equal(rio_pc_is_last, iseq);

	(void)state; // unused
}

struct pw_tests_t {
	rio_pc_pw_t pw;
	uint32_t ctl;
};

#define EN_4X RIO_SPX_CTL_PTW_MAX_4X
#define EN_2X RIO_SPX_CTL_PTW_MAX_2X
#define EN_4X2X (EN_4X | EN_2X)

// Exhaustive possible combinations of overrides and enabled link widths

static struct pw_tests_t cfg_pw_tests[] = {
	{rio_pc_pw_1x, RIO_SPX_CTL_PTW_OVER_NONE}, // 0
	{rio_pc_pw_last, RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_1x, RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_last, RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last, RIO_SPX_CTL_PTW_OVER_IMP_SPEC},
	{rio_pc_pw_1x, RIO_SPX_CTL_PTW_OVER_2X_NO_4X}, // 5
	{rio_pc_pw_1x, RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_1x, RIO_SPX_CTL_PTW_OVER_NONE_2},

	{rio_pc_pw_4x, EN_4X | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_last, RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_1x_l0, EN_4X | RIO_SPX_CTL_PTW_OVER_1X_L0}, // 10
	{rio_pc_pw_1x_l2, EN_4X | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last, EN_4X | RIO_SPX_CTL_PTW_OVER_IMP_SPEC},
	{rio_pc_pw_1x, EN_4X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_4x, EN_4X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_4x, EN_4X | RIO_SPX_CTL_PTW_OVER_NONE_2}, // 15

	{rio_pc_pw_2x, EN_2X | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_last, RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_1x_l0, EN_2X | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_1x_l1, EN_2X | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last, EN_2X | RIO_SPX_CTL_PTW_OVER_IMP_SPEC}, // 20
	{rio_pc_pw_2x, EN_2X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_1x, EN_2X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_2x, EN_2X | RIO_SPX_CTL_PTW_OVER_NONE_2},

	{rio_pc_pw_4x, EN_4X2X | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_last, RIO_SPX_CTL_PTW_OVER_RSVD}, // 25
	{rio_pc_pw_1x_l0, EN_4X2X | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_1x_l2, EN_4X2X | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last, EN_4X2X | RIO_SPX_CTL_PTW_OVER_IMP_SPEC},
	{rio_pc_pw_2x, EN_4X2X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_4x, EN_4X2X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X}, // 30
	{rio_pc_pw_4x, EN_4X2X | RIO_SPX_CTL_PTW_OVER_NONE_2},
};

static void rio_determine_cfg_pw_test(void **state)
{
	rio_pc_pw_t pw;
	unsigned int i;

	if (DEBUG_PRINTF) {
		printf("\n");
	}

	for (i = 0; i < sizeof(cfg_pw_tests)/sizeof(cfg_pw_tests[0]); i++) {
		if (DEBUG_PRINTF) {
			printf("i %d ctl 0x%x\n", i, cfg_pw_tests[i].ctl);
		}
		rio_determine_cfg_pw(&pw, cfg_pw_tests[i].ctl);
		assert_int_equal(pw, cfg_pw_tests[i].pw);
	}

	(void)state; // unused
}

#define L0 RIO_SPX_CTL_PTW_INIT_1X_L0
#define LR RIO_SPX_CTL_PTW_INIT_1X_LR
#define L4 RIO_SPX_CTL_PTW_INIT_4X
#define L2 RIO_SPX_CTL_PTW_INIT_2X

static struct pw_tests_t op_pw_tests[] = {
	// Trained lane 0, 1x
	{rio_pc_pw_1x,   L0 | RIO_SPX_CTL_PTW_OVER_NONE}, // 0
	{rio_pc_pw_1x,   L0 | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_last, L0 | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last, L0 | RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_1x,   L0 | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_1x,   L0 | RIO_SPX_CTL_PTW_OVER_4X_NO_2X}, // 5
	{rio_pc_pw_1x,   L0 | RIO_SPX_CTL_PTW_OVER_NONE_2},

	// Trained lane 0, 4x
	{rio_pc_pw_1x_l0, L0 | EN_4X | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_1x_l0, L0 | EN_4X | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_1x_l0, L0 | EN_4X | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last,  L0 | EN_4X | RIO_SPX_CTL_PTW_OVER_RSVD}, // 10
	{rio_pc_pw_1x,    L0 | EN_4X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_1x_l0, L0 | EN_4X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_1x_l0, L0 | EN_4X | RIO_SPX_CTL_PTW_OVER_NONE_2},

	// Trained lane 0, 2x
	{rio_pc_pw_1x_l0, L0 | EN_2X | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_1x_l0, L0 | EN_2X | RIO_SPX_CTL_PTW_OVER_1X_L0}, // 15
	{rio_pc_pw_1x_l0, L0 | EN_2X | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last,  L0 | EN_2X | RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_1x_l0, L0 | EN_2X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_1x,    L0 | EN_2X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_1x_l0, L0 | EN_2X | RIO_SPX_CTL_PTW_OVER_NONE_2}, // 20

	// Trained lane 0, 4x & 2x
	{rio_pc_pw_1x_l0, L0 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_1x_l0, L0 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_1x_l0, L0 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last,  L0 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_1x_l0, L0 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X}, // 25
	{rio_pc_pw_1x_l0, L0 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_1x_l0, L0 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_NONE_2},

	// Trained redundant lane, 1x
	{rio_pc_pw_last, LR | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_last, LR | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_last, LR | RIO_SPX_CTL_PTW_OVER_1X_LR}, // 30
	{rio_pc_pw_last, LR | RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_last, LR | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_last, LR | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_last, LR | RIO_SPX_CTL_PTW_OVER_NONE_2},

	// Trained redundant lane, 4x
	{rio_pc_pw_1x_l2, LR | EN_4X | RIO_SPX_CTL_PTW_OVER_NONE}, // 35
	{rio_pc_pw_1x_l2, LR | EN_4X | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_1x_l2, LR | EN_4X | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last,  LR | EN_4X | RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_last,  LR | EN_4X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_1x_l2, LR | EN_4X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X}, // 40
	{rio_pc_pw_1x_l2, LR | EN_4X | RIO_SPX_CTL_PTW_OVER_NONE_2},

	// Trained redundant lane, 2x
	{rio_pc_pw_1x_l1, LR | EN_2X | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_1x_l1, LR | EN_2X | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_1x_l1, LR | EN_2X | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last,  LR | EN_2X | RIO_SPX_CTL_PTW_OVER_RSVD}, // 45
	{rio_pc_pw_1x_l1, LR | EN_2X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_last,  LR | EN_2X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_1x_l1, LR | EN_2X | RIO_SPX_CTL_PTW_OVER_NONE_2},

	// Trained redundant lane, 4x & 2x
	{rio_pc_pw_1x_l2, LR | EN_4X2X | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_1x_l2, LR | EN_4X2X | RIO_SPX_CTL_PTW_OVER_1X_L0}, // 50
	{rio_pc_pw_1x_l2, LR | EN_4X2X | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last,  LR | EN_4X2X | RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_1x_l1, LR | EN_4X2X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_1x_l2, LR | EN_4X2X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_1x_l2, LR | EN_4X2X | RIO_SPX_CTL_PTW_OVER_NONE_2}, // 55

	// Trained 2x, Configured as 1x
	{rio_pc_pw_last, L2 | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_last, L2 | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_last, L2 | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last, L2 | RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_last, L2 | RIO_SPX_CTL_PTW_OVER_2X_NO_4X}, // 60
	{rio_pc_pw_last, L2 | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_last, L2 | RIO_SPX_CTL_PTW_OVER_NONE_2},

	// Trained 2x, Configured as 4x only
	{rio_pc_pw_last, L2 | EN_4X | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_last, L2 | EN_4X | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_last, L2 | EN_4X | RIO_SPX_CTL_PTW_OVER_1X_LR}, // 65
	{rio_pc_pw_last, L2 | EN_4X | RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_last, L2 | EN_4X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_last, L2 | EN_4X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_last, L2 | EN_4X | RIO_SPX_CTL_PTW_OVER_NONE_2},

	// Trained 2x, Configured as 2x only
	{rio_pc_pw_2x,   L2 | EN_2X | RIO_SPX_CTL_PTW_OVER_NONE}, // 70
	{rio_pc_pw_last, L2 | EN_2X | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_last, L2 | EN_2X | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last, L2 | EN_2X | RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_2x,   L2 | EN_2X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_last, L2 | EN_2X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X}, // 75
	{rio_pc_pw_2x,   L2 | EN_2X | RIO_SPX_CTL_PTW_OVER_NONE_2},

	// Trained 2x, Configured as 2x and 4x
	{rio_pc_pw_2x,   L2 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_last, L2 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_last, L2 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last, L2 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_RSVD}, // 80
	{rio_pc_pw_2x,   L2 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_last, L2 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_2x,   L2 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_NONE_2},

	// Trained 4x, Configured as 1x
	{rio_pc_pw_last, L4 | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_last, L4 | RIO_SPX_CTL_PTW_OVER_1X_L0}, // 85
	{rio_pc_pw_last, L4 | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last, L4 | RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_last, L4 | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_last, L4 | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_last, L4 | RIO_SPX_CTL_PTW_OVER_NONE_2}, // 90

	// Trained 4x, Configured as 4x
	{rio_pc_pw_4x,   L4 | EN_4X | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_last, L4 | EN_4X | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_last, L4 | EN_4X | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last, L4 | EN_4X | RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_last, L4 | EN_4X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X}, // 95
	{rio_pc_pw_4x,   L4 | EN_4X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_4x,   L4 | EN_4X | RIO_SPX_CTL_PTW_OVER_NONE_2},

	// Trained 4x, Configured as 2x
	{rio_pc_pw_last, L4 | EN_2X | RIO_SPX_CTL_PTW_OVER_NONE},
	{rio_pc_pw_last, L4 | EN_2X | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_last, L4 | EN_2X | RIO_SPX_CTL_PTW_OVER_1X_LR}, // 100
	{rio_pc_pw_last, L4 | EN_2X | RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_last, L4 | EN_2X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_last, L4 | EN_2X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X},
	{rio_pc_pw_last, L4 | EN_2X | RIO_SPX_CTL_PTW_OVER_NONE_2},

	// Trained 4x, Configured as 2x and 4x
	{rio_pc_pw_4x,	 L4 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_NONE}, // 105
	{rio_pc_pw_last, L4 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_1X_L0},
	{rio_pc_pw_last, L4 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_1X_LR},
	{rio_pc_pw_last, L4 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_RSVD},
	{rio_pc_pw_last, L4 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_2X_NO_4X},
	{rio_pc_pw_4x,   L4 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_4X_NO_2X}, // 110
	{rio_pc_pw_4x,   L4 | EN_4X2X | RIO_SPX_CTL_PTW_OVER_NONE_2},
};

static void rio_determine_op_pw_test(void **state)
{
	rio_pc_pw_t pw, cfg_pw;
	uint32_t ctl = 0;
	uint32_t errstat = ~RIO_SPX_ERR_STAT_OK;
	unsigned int idx;

	// Untrained link
	ctl = RIO_SPX_CTL_PTW_INIT_4X | RIO_SPX_CTL_PTW_MAX_4X;
	rio_determine_op_pw(&pw, ctl, errstat);
	assert_int_equal(rio_pc_pw_last, pw);

	errstat |= RIO_SPX_ERR_STAT_OK;

	if (DEBUG_PRINTF) {
		printf("\n");
	}

	for (idx = 0; idx < sizeof(op_pw_tests)/sizeof(op_pw_tests[0]); idx++) {
		rio_determine_cfg_pw(&cfg_pw, op_pw_tests[idx].ctl);
		if (DEBUG_PRINTF) {
			printf("i %d cfg_pw %d op_pw %d ctl 0x%x\n",
				idx, cfg_pw, op_pw_tests[idx].pw,
				op_pw_tests[idx].ctl);
		}
		pw = (rio_pc_pw_t)99;
		rio_determine_op_pw(&pw, op_pw_tests[idx].ctl, errstat);
		assert_int_equal(pw, op_pw_tests[idx].pw);
	}

	(void)state; // unused
}

int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
	cmocka_unit_test(assumptions),
	cmocka_unit_test(rio_determine_op_fc_test),
	cmocka_unit_test(rio_determine_ls_test),
	cmocka_unit_test(rio_determine_op_iseq_test),
	cmocka_unit_test(rio_determine_cfg_pw_test),
	cmocka_unit_test(rio_determine_op_pw_test),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#ifdef __cplusplus
}
#endif
