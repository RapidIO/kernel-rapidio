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
#include "RXS2448.h"
#include "src/RXS_DeviceDriver.c"
#include "src/RXS_RT.c"
#include "rio_ecosystem.h"
#include "tok_parse.h"
#include "libcli.h"
#include "rio_mport_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RXS_DAR_WANTED

static void rxs_not_supported_test(void **state)
{
	(void)state; // not used
}

int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++;// not used

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(rxs_not_supported_test)};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif /* RXS_DAR_WANTED */

#ifdef RXS_DAR_WANTED

static void assumptions_test(void **state)
{
	// verify constants
	assert_int_equal(8, RXS2448_MAX_SC);
	assert_int_equal(24, RXS2448_MAX_PORTS);
	assert_int_equal(48, RXS2448_MAX_LANES);

	(void)state; // unused
}

static void macros_test(void **state)
{
	assert_int_equal(0x80000000, RIO_SPX_CTL_PTW_MAX_2X);
	assert_int_equal(0x40000000, RIO_SPX_CTL_PTW_MAX_4X);

	assert_int_equal(0x1C100, RXS_SPX_PCNTR_EN(0x00));
	assert_int_equal(0x1D800, RXS_SPX_PCNTR_EN(0x17));
	assert_int_equal(0x1C110, RXS_SPX_PCNTR_CTL(0x00, 0x00));
	assert_int_equal(0x1D82C, RXS_SPX_PCNTR_CTL(0x17, 0x07));
	assert_int_equal(0x1C130, RXS_SPX_PCNTR_CNT(0x00, 0x00));
	assert_int_equal(0x1D84C, RXS_SPX_PCNTR_CNT(0x17, 0x07));
	assert_int_equal(0x30000, RXS_BC_L0_G0_ENTRYX_CSR(0x00));
	assert_int_equal(0x30028, RXS_BC_L0_G0_ENTRYX_CSR(0x0A));
	assert_int_equal(0x30400, RXS_BC_L1_GX_ENTRYY_CSR(0x00, 0x00));
	assert_int_equal(0x30428, RXS_BC_L1_GX_ENTRYY_CSR(0x00, 0x0A));
	assert_int_equal(0x31000, RXS_BC_L2_GX_ENTRYY_CSR(0x00, 0x00));
	assert_int_equal(0x31028, RXS_BC_L2_GX_ENTRYY_CSR(0x00, 0x0A));
	assert_int_equal(0x32000, RXS_BC_MC_X_S_CSR(0x00));
	assert_int_equal(0x32050, RXS_BC_MC_X_S_CSR(0x0A));
	assert_int_equal(0x50000, RXS_SPX_L0_G0_ENTRYY_CSR(0x00, 0x00));
	assert_int_equal(0x64028, RXS_SPX_L0_G0_ENTRYY_CSR(0x0A, 0x0A));
	assert_int_equal(0x50400,
			RXS_SPX_L1_GY_ENTRYZ_CSR(0x00, 0x00, 0x00));
	assert_int_equal(0x64428,
			RXS_SPX_L1_GY_ENTRYZ_CSR(0x0A, 0x00, 0x0A));
	assert_int_equal(0x51000,
			RXS_SPX_L2_GY_ENTRYZ_CSR(0x00, 0x00, 0x00));
	assert_int_equal(0x65028,
			RXS_SPX_L2_GY_ENTRYZ_CSR(0x0A, 0x00, 0x0A));
	assert_int_equal(0x80000, RXS_SPX_MC_Y_S_CSR(0x00, 0x00));
	assert_int_equal(0x8A050, RXS_SPX_MC_Y_S_CSR(0x0A, 0x0A));

	assert_int_equal(0x0FFFFFFF, RIO_RTE_BAD);

	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL_UNINIT, 0x00000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL_UNSUP, 0x10000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL_2_5GB, 0x20000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL_3_125GB, 0x30000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL_5_0GB, 0x40000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL_6_25GB, 0x50000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL_10_3125GB, 0x60000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL_12_5GB, 0x70000000);

	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL &
			RXS_SPX_CTL2_BAUD_SEL_UNINIT, 0x00000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL &
			RXS_SPX_CTL2_BAUD_SEL_UNSUP, 0x10000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL &
			RXS_SPX_CTL2_BAUD_SEL_2_5GB, 0x20000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL &
			RXS_SPX_CTL2_BAUD_SEL_3_125GB, 0x30000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL &
			RXS_SPX_CTL2_BAUD_SEL_5_0GB, 0x40000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL &
			RXS_SPX_CTL2_BAUD_SEL_6_25GB, 0x50000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL &
			RXS_SPX_CTL2_BAUD_SEL_10_3125GB, 0x60000000);
	assert_int_equal(RXS_SPX_CTL2_BAUD_SEL &
			RXS_SPX_CTL2_BAUD_SEL_12_5GB, 0x70000000);

	assert_int_equal(RXS_PLM_SPX_POL_CTL_TX_ALL_POL,
			RXS_PLM_SPX_POL_CTL_TX0_POL |
			RXS_PLM_SPX_POL_CTL_TX1_POL |
			RXS_PLM_SPX_POL_CTL_TX2_POL |
			RXS_PLM_SPX_POL_CTL_TX3_POL);
	assert_int_equal(RXS_PLM_SPX_POL_CTL_RX_ALL_POL,
			RXS_PLM_SPX_POL_CTL_RX0_POL |
			RXS_PLM_SPX_POL_CTL_RX1_POL |
			RXS_PLM_SPX_POL_CTL_RX2_POL |
			RXS_PLM_SPX_POL_CTL_RX3_POL);

	assert_int_equal(6000, RXS_PKT_TIME_LIVE_NSEC);
	assert_int_equal(0xFFFC, RXS_PKT_TIME_LIVE_MAX);

	assert_int_equal(0x253, RXS_SPX_DLT_CSR_TIMEOUT_NSEC);
	assert_int_equal(0xFFFFFF, RXS_SPX_DLT_CSR_TIMEOUT_MAX);

	assert_int_equal(297, RXS_PW_CTL_PW_TMR_NSEC);
	assert_int_equal(0xFFFFFF, RXS_PW_CTL_PW_TMR_MAX);
	
	(void)state; // unused
}


int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
	cmocka_unit_test(macros_test),
	cmocka_unit_test(assumptions_test), };

	return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif /* RXS_DAR_WANTED */

#ifdef __cplusplus
}
#endif
