/*
 ************************************************************************ 
 Copyright (c) 2016, Integrated Device Technology Inc.
 Copyright (c) 2016, RapidIO Trade Association
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

#include "RapidIO_Device_Access_Routines_API.h"
#include "CPS1848.h"
#include "src/CPS_RT.c"
#include "rio_standard.h"
#include "rio_ecosystem.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CPS_DAR_WANTED

static void cps_not_supported_test(void **state)
{
	(void)state; // not used
}

int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++;// not used

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(cps_not_supported_test)};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif /* CPS_DAR_WANTED */

#ifdef CPS_DAR_WANTED

typedef struct mock_dar_reg_t_TAG {
	uint32_t offset;
	uint32_t data;
} mock_dar_reg_t;

static DAR_DEV_INFO_t mock_dev_info;

/* Create a mock dev_info.
 */
static void cps_test_setup(void)
{
	uint8_t idx;

	mock_dev_info.privateData = 0x0;
	mock_dev_info.accessInfo = 0x0;
	strcpy(mock_dev_info.name, "CPS1848");
	mock_dev_info.dsf_h = 0x00380000;
	mock_dev_info.extFPtrForPort = 0;
	mock_dev_info.extFPtrPortType = 0;
	mock_dev_info.extFPtrForLane = 12288;
	mock_dev_info.extFPtrForErr = 0;
	mock_dev_info.extFPtrForVC = 0;
	mock_dev_info.extFPtrForVOQ = 0;
	mock_dev_info.devID = 0x03740038;
	mock_dev_info.driver_family = RIO_CPS_DEVICE;
	mock_dev_info.devInfo = 0;
	mock_dev_info.assyInfo = 256;
	mock_dev_info.features = 0x18000779;
	mock_dev_info.swPortInfo = 0x1201;
	mock_dev_info.swRtInfo = 0xFF;
	mock_dev_info.srcOps = 4;
	mock_dev_info.dstOps = 0;
	mock_dev_info.swMcastInfo = 0x00FF0028;
	mock_dev_info.poregs_max = 0;
	mock_dev_info.poreg_cnt = 0;
	mock_dev_info.poregs = NULL;

	for (idx = 0; idx < RIO_MAX_PORTS; idx++) {
		mock_dev_info.ctl1_reg[idx] = 0;
	}

	for (idx = 0; idx < MAX_DAR_SCRPAD_IDX; idx++) {
		mock_dev_info.scratchpad[idx] = 0;
	}
}

/* The setup function which should be called before any unit tests that need to be executed.
 */
static int setup(void **state)
{
	cps_test_setup();

	(void)state; // unused
	return 0;
}

/* The teardown function to be called after any tests have finished.
 */
static int teardown(void **state)
{
	(void)state; //unused
	return 0;
}

static void assumptions_test(void **state)
{
	// verify constants
	assert_int_equal(0, CPS_RTE_PT_0);
	assert_int_equal(0x12, CPS_RTE_PT_LAST);
	assert_int_equal(0x40, CPS_FIRST_MC_MASK);
	assert_int_equal(0x28, CPS_MAX_MC_MASK);
	assert_int_equal(0x67, CPS_LAST_MC_MASK);
	assert_int_equal(0xDD, CPS_RT_USE_DEVICE_TABLE);
	assert_int_equal(0xDE, CPS_RT_USE_DEFAULT_ROUTE);
	assert_int_equal(0xDF, CPS_RT_NO_ROUTE);

	(void)state; // unused
}

static void macros_test(void **state)
{
	assert_true(RIO_RTV_IS_PORT(0));
	assert_true(RIO_RTV_IS_PORT(0x18));
	assert_true(RIO_RTV_IS_PORT(0xFF));
	assert_false(RIO_RTV_IS_PORT(0x100));
	assert_int_equal(0x00, RIO_RTV_GET_PORT(0x00));
	assert_int_equal(0x23, RIO_RTV_GET_PORT(0x23));
	assert_int_equal(0xFF, RIO_RTV_GET_PORT(0xFF));
	assert_int_equal(RIO_RTE_BAD, RIO_RTV_GET_PORT(0x100));
	assert_int_equal(0x00, RIO_RTV_PORT(0x00));
	assert_int_equal(0x23, RIO_RTV_PORT(0x23));
	assert_int_equal(0xFF, RIO_RTV_PORT(0xFF));
	assert_int_equal(RIO_RTE_BAD, RIO_RTV_PORT(0x100));

	assert_true(RIO_RTV_IS_MC_MSK(0x100));
	assert_true(RIO_RTV_IS_MC_MSK(0x104));
	assert_true(RIO_RTV_IS_MC_MSK(0x1FF));
	assert_false(RIO_RTV_IS_MC_MSK(0x0FF));
	assert_false(RIO_RTV_IS_MC_MSK(0x200));
	assert_int_equal(0x00, RIO_RTV_GET_MC_MSK(0x100));
	assert_int_equal(0x23, RIO_RTV_GET_MC_MSK(0x123));
	assert_int_equal(0xFF, RIO_RTV_GET_MC_MSK(0x1FF));
	assert_int_equal(RIO_RTE_BAD, RIO_RTV_GET_MC_MSK(0x098));
	assert_int_equal(RIO_RTE_BAD, RIO_RTV_GET_MC_MSK(0x205));
	assert_int_equal(0x100, RIO_RTV_MC_MSK(0x00));
	assert_int_equal(0x123, RIO_RTV_MC_MSK(0x23));
	assert_int_equal(0x1FF, RIO_RTV_MC_MSK(0xFF));
	assert_int_equal(RIO_RTE_BAD, RIO_RTV_MC_MSK(0x100));
	assert_int_equal(RIO_RTE_BAD, RIO_RTV_MC_MSK(0x205));

	assert_true(RIO_RTV_IS_LVL_GRP(0x200));
	assert_true(RIO_RTV_IS_LVL_GRP(0x204));
	assert_true(RIO_RTV_IS_LVL_GRP(0x2FF));
	assert_false(RIO_RTV_IS_LVL_GRP(0x1FF));
	assert_false(RIO_RTV_IS_LVL_GRP(0x300));
	assert_int_equal(0x00, RIO_RTV_GET_LVL_GRP(0x200));
	assert_int_equal(0x23, RIO_RTV_GET_LVL_GRP(0x223));
	assert_int_equal(0xFF, RIO_RTV_GET_LVL_GRP(0x2FF));
	assert_int_equal(RIO_RTE_BAD, RIO_RTV_GET_LVL_GRP(0x198));
	assert_int_equal(RIO_RTE_BAD, RIO_RTV_GET_LVL_GRP(0x305));
	assert_int_equal(0x200, RIO_RTV_LVL_GRP(0x00));
	assert_int_equal(0x223, RIO_RTV_LVL_GRP(0x23));
	assert_int_equal(0x2FF, RIO_RTV_LVL_GRP(0xFF));
	assert_int_equal(RIO_RTE_BAD, RIO_RTV_LVL_GRP(0x100));
	assert_int_equal(RIO_RTE_BAD, RIO_RTV_LVL_GRP(0x197));
	assert_int_equal(RIO_RTE_BAD, RIO_RTV_LVL_GRP(0x305));

	(void)state; // unused
}

static void cps_rte_translate_CPS_to_std_test(void **state)
{
	uint32_t sout;
	unsigned int i;

	// Test valid port range...
	for (i = 0; i < CPS_RTE_PT_LAST; i++) {
		assert_int_equal(0,
				cps_rte_translate_CPS_to_std(&mock_dev_info, i,
						&sout));
		assert_int_equal(i, sout);
	}

	// Test invalid CPS port range...
	for (i = CPS_RTE_PT_LAST; i < CPS_FIRST_MC_MASK; i++) {
		assert_int_not_equal(0,
				cps_rte_translate_CPS_to_std(&mock_dev_info, i,
						&sout));
	}

	// Test valid MC Group range...
	for (i = CPS_FIRST_MC_MASK; i <= CPS_LAST_MC_MASK; i++) {
		assert_int_equal(0,
				cps_rte_translate_CPS_to_std(&mock_dev_info, i,
						&sout));
		assert_int_equal(RIO_RTV_MC_MSK(IS_CPS_MC_MASK_NO(i)), sout);
	}
	// Test invalid MC Group port range...
	for (i = CPS_LAST_MC_MASK + 1; i < CPS_RT_USE_DEVICE_TABLE; i++) {
		assert_int_not_equal(0,
				cps_rte_translate_CPS_to_std(&mock_dev_info, i,
						&sout));
	}

	// Check the next level/discard/default route values
	assert_int_equal(0,
			cps_rte_translate_CPS_to_std(&mock_dev_info, CPS_RT_USE_DEVICE_TABLE, &sout));
	assert_int_equal(RIO_RTE_LVL_G0, sout);
	assert_int_equal(0,
			cps_rte_translate_CPS_to_std(&mock_dev_info, CPS_RT_USE_DEFAULT_ROUTE, &sout));
	assert_int_equal(RIO_RTE_DFLT_PORT, sout);
	assert_int_equal(0,
			cps_rte_translate_CPS_to_std(&mock_dev_info, CPS_RT_NO_ROUTE, &sout));
	assert_int_equal(RIO_RTE_DROP, sout);

	// Test invalid port range...
	for (i = CPS_RT_NO_ROUTE + 1; i < 0x100FF; i++) {
		assert_int_not_equal(0,
				cps_rte_translate_CPS_to_std(&mock_dev_info, i,
						&sout));
	}

	(void)state; // unused
}

static void cps_rte_translate_std_to_CPS_test(void **state)
{
	uint32_t cps;
	unsigned int i;

	// Test valid port range...
	for (i = 0; i < RIO_RTV_PORT(CPS_RTE_PT_LAST); i++) {
		assert_int_equal(0,
				cps_rte_translate_std_to_CPS(&mock_dev_info, i,
						&cps));
		assert_int_equal(i, cps);
	}

	// Test invalid CPS port range...
	for (i = RIO_RTV_PORT(CPS_RTE_PT_LAST); i <= RIO_RTE_PT_LAST; i++) {
		assert_int_not_equal(0,
				cps_rte_translate_std_to_CPS(&mock_dev_info, i,
						&cps));
	}

	// Test valid MC Group range...
	for (i = RIO_RTE_MC_0; i < RIO_RTV_MC_MSK(CPS_MAX_MC_MASK); i++) {
		assert_int_equal(0,
				cps_rte_translate_std_to_CPS(&mock_dev_info, i,
						&cps));
		assert_int_equal(CPS_MC_PORT(RIO_RTV_GET_MC_MSK(i)), cps);
	}
	// Test invalid MC Group port range...
	for (i = RIO_RTV_MC_MSK(CPS_MAX_MC_MASK); i <= RIO_RTE_MC_LAST; i++) {
		assert_int_not_equal(0,
				cps_rte_translate_std_to_CPS(&mock_dev_info, i,
						&cps));
	}

	// Check the next level/discard/default route values
	assert_int_equal(0,
			cps_rte_translate_std_to_CPS(&mock_dev_info, RIO_RTE_LVL_G0, &cps));
	assert_int_equal(CPS_RT_USE_DEVICE_TABLE, cps);

	// Test invalid MC Group port range...
	for (i = RIO_RTE_LVL_G0 + 1; i <= RIO_RTE_LVL_GLAST; i++) {
		assert_int_not_equal(0,
				cps_rte_translate_std_to_CPS(&mock_dev_info, i,
						&cps));
	}
	assert_int_equal(0,
			cps_rte_translate_std_to_CPS(&mock_dev_info, RIO_RTE_DROP, &cps));
	assert_int_equal(CPS_RT_NO_ROUTE, cps);
	assert_int_equal(0,
			cps_rte_translate_std_to_CPS(&mock_dev_info, RIO_RTE_DFLT_PORT, &cps));
	assert_int_equal(CPS_RT_USE_DEFAULT_ROUTE, cps);

	// Test invalid port range...
	for (i = RIO_RTE_DFLT_PORT + 1; i < 0x100FF; i++) {
		assert_int_not_equal(0,
				cps_rte_translate_std_to_CPS(&mock_dev_info, i,
						&cps));
	}

	(void)state; // unused
}

int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
	cmocka_unit_test(macros_test),
	cmocka_unit_test_setup(assumptions_test, setup),
	cmocka_unit_test_setup_teardown(
			cps_rte_translate_CPS_to_std_test, setup,
			teardown),
	cmocka_unit_test_setup_teardown(
			cps_rte_translate_std_to_CPS_test, setup,
			teardown), 
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif /* CPS_DAR_WANTED */

#ifdef __cplusplus
}
#endif

