/*
 ****************************************************************************
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
#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <errno.h>

#include <stdarg.h>
#include <setjmp.h>
#include "cmocka.h"

#include "math_util.h"

#ifdef __cplusplus
extern "C" {
#endif

static void math_util_roundup_pw2_success(void **state)
{
	assert_int_equal(0x20000000, roundup_pw2(0x12345678));
	assert_int_equal(0x100000000, roundup_pw2(0x87654321));
	assert_int_equal(0x8, roundup_pw2(0x6));

	(void)state; // unused
}

static void math_util_roundup_pw2_pow2(void **state)
{
	uint64_t value = 1;

	// fast range check
	while (value) {
		assert_int_equal(value, roundup_pw2(value));
		value <<= 1;
	}

	(void)state; // unused
}

static void math_util_roundup_pw2_minus1(void **state)
{
	uint64_t value = 3, check = 4;

	// fast range check
	while (check) {
		assert_int_equal(check, roundup_pw2(value));
		value = (value << 1) + 1;
		check <<= 1;
	}

	(void)state; // unused
}

static void math_util_roundup_pw2_limit(void **state)
{
	assert_int_equal(0, roundup_pw2(0x8000000000000001));
	assert_int_equal(0, roundup_pw2(0xFFFFFFFFFFFFFFFF));
	assert_int_equal(0, roundup_pw2(0x8000000dead00001));
	assert_int_equal(0, roundup_pw2(0));

	(void)state; // unused
}

int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
	cmocka_unit_test(math_util_roundup_pw2_success),
	cmocka_unit_test(math_util_roundup_pw2_pow2),
	cmocka_unit_test(math_util_roundup_pw2_minus1),
	cmocka_unit_test(math_util_roundup_pw2_limit), };
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#ifdef __cplusplus
}
#endif
