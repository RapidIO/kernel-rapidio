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

#include "rio_route.h"
#include "tok_parse.h"
#include "liblog.h"
#include "rio_ecosystem.h"

#ifdef __cplusplus
extern "C" {
#endif

static void msg_fmt_test(void **state)
{
	char buf[256];

	// ensure accidental typos do not appear in the format strings
	// This test would disappear if translations were supported and all the strings
	// were captured in a translation type file.
	assert_string_equal("Destination Id must be between 0 and 0xff\n",
			TOK_ERR_DID_MSG_FMT);
	assert_string_equal("Component tag must be between 0 and 0xffffffff\n",
			TOK_ERR_CT_MSG_FMT);
	assert_string_equal("Hopcount must be between 0 and 0xff\n",
			TOK_ERR_HC_MSG_FMT);
	assert_string_equal("Mport device index must be between 0 and 7\n",
			TOK_ERR_MPORT_MSG_FMT);
	assert_string_equal("Log level must be between 1 and 7\n",
			TOK_ERR_LOG_LEVEL_MSG_FMT);
	assert_string_equal("%s must be between 0 and 0xffff\n",
			TOK_ERR_SOCKET_MSG_FMT);
	assert_string_equal(
			"Port number must be between 0 and " STR(RIO_MAX_DEV_PORT-1) "\n",
			TOK_ERR_PORT_NUM_MSG_FMT);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, TOK_ERR_ULL_HEX_MSG_FMT, "Fred");
	assert_string_equal("Fred must be between 0x0 and 0xffffffffffffffff\n",
			buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, TOK_ERR_UL_HEX_MSG_FMT, "Barney");
	assert_string_equal("Barney must be between 0x0 and 0xffffffff\n", buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, TOK_ERR_US_HEX_MSG_FMT, "Bam Bam");
	assert_string_equal("Bam Bam must be between 0x0 and 0xffff\n", buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, TOK_ERR_ULONGLONG_MSG_FMT, "ulonglong", 281474976710655,
			68719476735);
	assert_string_equal(
			"ulonglong must be between 281474976710655 and 68719476735\n",
			buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, TOK_ERR_ULONG_MSG_FMT, "ulong", 1048575, 16777215);
	assert_string_equal("ulong must be between 1048575 and 16777215\n",
			buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, TOK_ERR_USHORT_MSG_FMT, "ushort", 1, 255);
	assert_string_equal("ushort must be between 1 and 255\n", buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, TOK_ERR_ULONGLONG_MSG_FMT, "longlong", INT64_MIN,
	INT64_MAX);
	assert_string_equal(
			"longlong must be between 9223372036854775808 and 9223372036854775807\n",
			buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, TOK_ERR_ULONG_MSG_FMT, "long", INT32_MIN, INT32_MAX);
	assert_string_equal("long must be between 2147483648 and 2147483647\n",
			buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, TOK_ERR_USHORT_MSG_FMT, "short", INT16_MIN, INT16_MAX);
	assert_string_equal("short must be between 4294934528 and 32767\n",
			buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, TOK_ERR_ULONGLONG_HEX_MSG_FMT, "ulonglong",
			281474976710655, 68719476735);
	assert_string_equal(
			"ulonglong must be between 0xffffffffffff and 0xfffffffff\n",
			buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, TOK_ERR_ULONG_HEX_MSG_FMT, "ulong", 1048575, 16777215);
	assert_string_equal("ulong must be between 0xfffff and 0xffffff\n",
			buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, TOK_ERR_USHORT_HEX_MSG_FMT, "ushort", 1, 255);
	assert_string_equal("ushort must be between 0x1 and 0xff\n", buf);

	(void)state; // unused
}

static void tok_parse_ulonglong_null_parm_test(void **state)
{
	uint64_t value;
	char buf[128];
	int rc;

	// token cannot be null
	errno = 0xcafebabe;
	rc = tok_parse_ulonglong(NULL, &value, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// value cannot be null
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, NULL, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdeadbeaf, value);
	assert_string_equal("123", buf);

	// both cannot be null
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(NULL, NULL, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdeadbeaf, value);

	(void)state; // unused
}

static void tok_parse_ulonglong_implicit_base_test(void **state)
{
	const uint64_t min = 0;
	const uint64_t max = 0xffff;

	uint64_t value;
	char buf[128];
	int rc;

	// decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(123, value);
	assert_string_equal("123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x123, value);
	assert_string_equal("0x123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0X321");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x321, value);
	assert_string_equal("0X321", buf);

	// octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x53, value);
	assert_string_equal("0123", buf);

	// negative decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-123", buf);

	// negative hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0x123", buf);

	// negative octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0123", buf);

	(void)state; // unused
}

static void tok_parse_ulonglong_explicit_base_test(void **state)
{
	const uint64_t min = 0;
	const uint64_t max = 0xffffffff;

	uint64_t value;
	char buf[128];
	int rc;
	int i;

	int b[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			33, 34, 35};
	int v[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 17, 19, 21, 23, 25,
			27, 29, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 93, 97,
			101, 105, 109};

	// do not verify the base 0, it is already done

	// invalid base, valid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, min, max, 1);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	assert_string_equal("1", buf);

	// valid base, invalid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz"); // base 35 [0-9, A-Y]
	for (i = 2; i < 36; i++) {
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_ulonglong(buf, &value, min, max, i);
		assert_int_equal(-1, rc);
		assert_int_equal(EINVAL, errno);
		assert_int_equal(0, value);
		assert_string_equal("123abcz", buf);
	}

	// valid base, valid value
	for (i = 0; i < 34; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", i + 1);
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_ulonglong(buf, &value, min, max, b[i]);
		assert_int_equal(0, rc);
		assert_int_equal(0, errno);
		assert_int_equal(v[i], value);
	}

	// base 36 [0-9, A-Z]
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, min, max, 36);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x89489303, value); // observation

	// bad base
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "beef");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, min, max, 37);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_ulonglong_bounds_test(void **state)
{
	uint64_t value;
	char buf[128];
	int rc;

	// decimal out of range (min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "14");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, 15, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// decimal out of range (max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "256");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, 15, 0xff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x14");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, 0x15, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x256");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, 0x15, 0x255, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "014");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, 0xd, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0256");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, 0xd, 0xad, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// greater than max allowed
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0xfffffffffffffffff");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, 0, INT32_MAX, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_ulonglong_nan_test(void **state)
{
	const uint64_t min = 0;
	const uint64_t max = INT32_MAX;

	uint64_t value;
	char buf[128];
	int rc;

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "hello");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-goodbye");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulonglong(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	(void)state; // unused
}

static void tok_parse_ulong_null_parm_test(void **state)
{
	uint32_t value;
	char buf[128];
	int rc;

	// token cannot be null
	errno = 0xcafebabe;
	rc = tok_parse_ulong(NULL, &value, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// value cannot be null
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, NULL, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdeadbeaf, value);
	assert_string_equal("123", buf);

	// both cannot be null
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(NULL, NULL, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdeadbeaf, value);

	(void)state; // unused
}

static void tok_parse_ulong_implicit_base_test(void **state)
{
	const uint32_t min = 0;
	const uint32_t max = 0xffff;

	uint32_t value;
	char buf[128];
	int rc;

	// decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(123, value);
	assert_string_equal("123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x123, value);
	assert_string_equal("0x123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0X321");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x321, value);
	assert_string_equal("0X321", buf);

	// octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x53, value);
	assert_string_equal("0123", buf);

	// negative decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-123", buf);

	// negative hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0x123", buf);

	// negative octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0123", buf);

	(void)state; // unused
}

static void tok_parse_ulong_explicit_base_test(void **state)
{
	const uint32_t min = 0;
	const uint32_t max = 0xffffffff;

	uint32_t value;
	char buf[128];
	int rc;
	int i;

	int b[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			33, 34, 35};
	int v[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 17, 19, 21, 23, 25,
			27, 29, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 93, 97,
			101, 105, 109};

	// do not verify the base 0, it is already done

	// invalid base, valid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, min, max, 1);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	assert_string_equal("1", buf);

	// valid base, invalid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz"); // base 35 [0-9, A-Y]
	for (i = 2; i < 36; i++) {
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_ulong(buf, &value, min, max, i);
		assert_int_equal(-1, rc);
		assert_int_equal(EINVAL, errno);
		assert_int_equal(0, value);
		assert_string_equal("123abcz", buf);
	}

	// valid base, valid value
	for (i = 0; i < 34; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", i + 1);
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_ulong(buf, &value, min, max, b[i]);
		assert_int_equal(0, rc);
		assert_int_equal(0, errno);
		assert_int_equal(v[i], value);
	}

	// base 36 [0-9, A-Z]
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, min, max, 36);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x89489303, value); // observation

	// bad base
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "beef");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, min, max, 37);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_ulong_bounds_test(void **state)
{
	uint32_t value;
	char buf[128];
	int rc;

	// decimal out of range (min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "14");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, 15, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// decimal out of range (max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "256");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, 15, 0xff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x14");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, 0x15, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x256");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, 0x15, 0x255, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "014");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, 0xd, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0256");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, 0xd, 0xad, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// greater than max allowed
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0xfffffffffffffffff");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, 0, INT32_MAX, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_ulong_nan_test(void **state)
{
	const uint32_t min = 0;
	const uint32_t max = INT32_MAX;

	uint32_t value;
	char buf[128];
	int rc;

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "hello");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-goodbye");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ulong(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	(void)state; // unused
}

static void tok_parse_ushort_null_parm_test(void **state)
{
	uint16_t value;
	char buf[128];
	int rc;

	// token cannot be null
	errno = 0xcafebabe;
	rc = tok_parse_ushort(NULL, &value, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// value cannot be null
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, NULL, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdead, value);
	assert_string_equal("123", buf);

	// both cannot be null
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(NULL, NULL, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdead, value);

	(void)state; // unused
}

static void tok_parse_ushort_implicit_base_test(void **state)
{
	const uint16_t min = 0;
	const uint16_t max = 0xffff;

	uint16_t value;
	char buf[128];
	int rc;

	// decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(123, value);
	assert_string_equal("123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x123, value);
	assert_string_equal("0x123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0X321");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x321, value);
	assert_string_equal("0X321", buf);

	// octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x53, value);
	assert_string_equal("0123", buf);

	// negative decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-123", buf);

	// negative hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0x123", buf);

	// negative octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0123", buf);

	(void)state; // unused
}

static void tok_parse_ushort_explicit_base_test(void **state)
{
	const uint16_t min = 0;
	const uint16_t max = 0xffff;

	uint16_t value;
	char buf[128];
	int rc;
	int i;

	int b[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			33, 34, 35};
	int v[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 17, 19, 21, 23, 25,
			27, 29, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 93, 97,
			101, 105, 109};

	// do not verify the base 0, it is already done

	// invalid base, valid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, min, max, 1);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	assert_string_equal("1", buf);

	// valid base, invalid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz"); // base 35 [0-9, A-Y]
	for (i = 2; i < 36; i++) {
		errno = 0xcafebabe;
		value = 0xdead;
		rc = tok_parse_ushort(buf, &value, min, max, i);
		assert_int_equal(-1, rc);
		assert_int_equal(EINVAL, errno);
		assert_int_equal(0, value);
		assert_string_equal("123abcz", buf);
	}

	// valid base, valid value
	for (i = 0; i < 34; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", i + 1);
		errno = 0xcafebabe;
		value = 0xdead;
		rc = tok_parse_ushort(buf, &value, min, max, b[i]);
		assert_int_equal(0, rc);
		assert_int_equal(0, errno);
		assert_int_equal(v[i], value);
	}

	// base 36 [0-9, A-Z]
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "bcz");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, min, max, 36);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(14723, value); // observation

	// bad base
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "beef");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, min, max, 37);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_ushort_bounds_test(void **state)
{
	uint16_t value;
	char buf[128];
	int rc;

	// decimal out of range (min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "14");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, 15, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// decimal out of range (max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "256");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, 15, 0xff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x14");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, 0x15, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x256");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, 0x15, 0x255, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "014");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, 0xd, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0256");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, 0xd, 0xad, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// greater than max allowed
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0xfffffffffffffffff");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, 0, INT16_MAX, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_ushort_nan_test(void **state)
{
	const uint16_t min = 0;
	const uint16_t max = INT16_MAX;

	uint16_t value;
	char buf[128];
	int rc;

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "hello");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-goodbye");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ushort(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	(void)state; // unused
}

static void tok_parse_longlong_null_parm_test(void **state)
{
	int64_t value;
	char buf[128];
	int rc;

	// token cannot be null
	errno = 0xcafebabe;
	rc = tok_parse_longlong(NULL, &value, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// value cannot be null
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, NULL, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdeadbeaf, value);
	assert_string_equal("123", buf);

	// both cannot be null
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(NULL, NULL, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdeadbeaf, value);

	(void)state; // unused
}

static void tok_parse_longlong_implicit_base_test(void **state)
{
	const int64_t min = -0xffff;
	const int64_t max = 0xffff;

	int64_t value;
	char buf[128];
	int rc;

	// decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(123, value);
	assert_string_equal("123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x123, value);
	assert_string_equal("0x123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0X321");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x321, value);
	assert_string_equal("0X321", buf);

	// octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x53, value);
	assert_string_equal("0123", buf);

	// negative decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(-123, value);
	assert_string_equal("-123", buf);

	// negative hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(-0x123, value);
	assert_string_equal("-0x123", buf);

	// negative octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(-83, value);
	assert_string_equal("-0123", buf);

	(void)state; // unused
}

static void tok_parse_longlong_explicit_base_test(void **state)
{
	const int64_t min = 0;
	const int64_t max = 0xffffffff;

	int64_t value;
	char buf[128];
	int rc;
	int i;

	int b[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			33, 34, 35};
	int v[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 17, 19, 21, 23, 25,
			27, 29, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 93, 97,
			101, 105, 109};

	// do not verify the base 0, it is already done

	// invalid base, valid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, min, max, 1);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	assert_string_equal("1", buf);

	// valid base, invalid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz"); // base 35 [0-9, A-Y]
	for (i = 2; i < 36; i++) {
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_longlong(buf, &value, min, max, i);
		assert_int_equal(-1, rc);
		assert_int_equal(EINVAL, errno);
		assert_int_equal(0, value);
		assert_string_equal("123abcz", buf);
	}

	// valid base, valid value
	for (i = 0; i < 34; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", i + 1);
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_longlong(buf, &value, min, max, b[i]);
		assert_int_equal(0, rc);
		assert_int_equal(0, errno);
		assert_int_equal(v[i], value);
	}

	// base 36 [0-9, A-Z]
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, min, max, 36);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x89489303, value); // observation

	// bad base
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "beef");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, min, max, 37);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_longlong_bounds_test(void **state)
{
	int64_t value;
	char buf[128];
	int rc;

	// decimal out of range (min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "14");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, 15, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// decimal out of range (max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "256");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, 15, 0xff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x14");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, 0x15, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x256");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, 0x15, 0x255, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "014");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, 0xd, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0256");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, 0xd, 0xad, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// greater than max allowed
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0xfffffffffffffffff");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, 0, INT32_MAX, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_longlong_nan_test(void **state)
{
	const int64_t min = 0;
	const int64_t max = INT32_MAX;

	int64_t value;
	char buf[128];
	int rc;

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "hello");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-goodbye");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_longlong(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	(void)state; // unused
}

static void tok_parse_long_null_parm_test(void **state)
{
	int32_t value;
	char buf[128];
	int rc;

	// token cannot be null
	errno = 0xcafebabe;
	rc = tok_parse_long(NULL, &value, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// value cannot be null
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0x78123;
	rc = tok_parse_long(buf, NULL, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0x78123, value);
	assert_string_equal("123", buf);

	// both cannot be null
	errno = 0xcafebabe;
	value = 0x78123;
	rc = tok_parse_long(NULL, NULL, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0x78123, value);

	(void)state; // unused
}

static void tok_parse_long_implicit_base_test(void **state)
{
	const int32_t min = 0;
	const int32_t max = 0xffff;

	int32_t value;
	char buf[128];
	int rc;

	// decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(123, value);
	assert_string_equal("123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x123, value);
	assert_string_equal("0x123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0X321");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x321, value);
	assert_string_equal("0X321", buf);

	// octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x53, value);
	assert_string_equal("0123", buf);

	// negative decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-123", buf);

	// negative hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0x123", buf);

	// negative octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0123", buf);

	(void)state; // unused
}

static void tok_parse_long_explicit_base_test(void **state)
{
	const int32_t min = 0;
	const int32_t max = INT32_MAX;

	int32_t value;
	char buf[128];
	int rc;
	int i;

	int b[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			33, 34, 35};
	int v[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 17, 19, 21, 23, 25,
			27, 29, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 93, 97,
			101, 105, 109};

	// do not verify the base 0, it is already done

	// invalid base, valid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, min, max, 1);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	assert_string_equal("1", buf);

	// valid base, invalid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz"); // base 35 [0-9, A-Y]
	for (i = 2; i < 36; i++) {
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_long(buf, &value, min, max, i);
		assert_int_equal(-1, rc);
		assert_int_equal(EINVAL, errno);
		assert_int_equal(0, value);
		assert_string_equal("123abcz", buf);
	}

	// valid base, valid value
	for (i = 0; i < 34; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", i + 1);
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_long(buf, &value, min, max, b[i]);
		assert_int_equal(0, rc);
		assert_int_equal(0, errno);
		assert_int_equal(v[i], value);
	}

	// base 36 [0-9, A-Z]
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "bcz");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, min, max, 36);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(14723, value); // observation

	// bad base
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "beef");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, min, max, 37);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_long_bounds_test(void **state)
{
	int32_t value;
	char buf[128];
	int rc;

	// decimal out of range (min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "14");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, 15, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// decimal out of range (max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "256");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, 15, 0xff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x14");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, 0x15, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x256");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, 0x15, 0x255, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "014");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, 0xd, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0256");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, 0xd, 0xad, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// greater than max allowed
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0xfffffffffffffffff");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, 0, INT32_MAX, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_long_nan_test(void **state)
{
	const int32_t min = 0;
	const int32_t max = INT32_MAX;

	int32_t value;
	char buf[128];
	int rc;

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "hello");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-goodbye");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_long(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	(void)state; // unused
}

static void tok_parse_short_null_parm_test(void **state)
{
	int16_t value;
	char buf[128];
	int rc;

	// token cannot be null
	errno = 0xcafebabe;
	rc = tok_parse_short(NULL, &value, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// value cannot be null
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0x1234;
	rc = tok_parse_short(buf, NULL, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0x1234, value);
	assert_string_equal("123", buf);

	// both cannot be null
	errno = 0xcafebabe;
	value = 0x1234;
	rc = tok_parse_short(NULL, NULL, 0, 1, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0x1234, value);

	(void)state; // unused
}

static void tok_parse_short_implicit_base_test(void **state)
{
	const int16_t min = 0;
	const int16_t max = INT16_MAX;

	int16_t value;
	char buf[128];
	int rc;

	// decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(123, value);
	assert_string_equal("123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x123, value);
	assert_string_equal("0x123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0X321");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x321, value);
	assert_string_equal("0X321", buf);

	// octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, min, max, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x53, value);
	assert_string_equal("0123", buf);

	// negative decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-123", buf);

	// negative hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0x123", buf);

	// negative octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0123", buf);

	(void)state; // unused
}

static void tok_parse_short_explicit_base_test(void **state)
{
	const int16_t min = 0;
	const int16_t max = INT16_MAX;

	int16_t value;
	char buf[128];
	int rc;
	int i;

	int b[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			33, 34, 35};
	int v[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 17, 19, 21, 23, 25,
			27, 29, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 93, 97,
			101, 105, 109};

	// do not verify the base 0, it is already done

	// invalid base, valid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, min, max, 1);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	assert_string_equal("1", buf);

	// valid base, invalid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz"); // base 35 [0-9, A-Y]
	for (i = 2; i < 36; i++) {
		errno = 0xcafebabe;
		value = 0xdead;
		rc = tok_parse_short(buf, &value, min, max, i);
		assert_int_equal(-1, rc);
		assert_int_equal(EINVAL, errno);
		assert_int_equal(0, value);
		assert_string_equal("123abcz", buf);
	}

	// valid base, valid value
	for (i = 0; i < 34; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", i + 1);
		errno = 0xcafebabe;
		value = 0xdead;
		rc = tok_parse_short(buf, &value, min, max, b[i]);
		assert_int_equal(0, rc);
		assert_int_equal(0, errno);
		assert_int_equal(v[i], value);
	}

	// base 36 [0-9, A-Z]
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "bcz");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, min, max, 36);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(14723, value); // observation

	// bad base
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "beef");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, min, max, 37);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_short_bounds_test(void **state)
{
	int16_t value;
	char buf[128];
	int rc;

	// decimal out of range (min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "14");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, 15, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// decimal out of range (max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "256");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, 15, 0xff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x14");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, 0x15, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x256");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, 0x15, 0x255, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "014");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, 0xd, 0xffff, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0256");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, 0xd, 0xad, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// greater than max allowed
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0xfffffffffffffffff");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, 0, INT16_MAX, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_short_nan_test(void **state)
{
	const int16_t min = 0;
	const int16_t max = INT16_MAX;

	int16_t value;
	char buf[128];
	int rc;

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "hello");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-goodbye");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_short(buf, &value, min, max, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	(void)state; // unused
}

static void tok_parse_ull_null_parm_test(void **state)
{
	uint64_t value;
	char buf[128];
	int rc;

	// token cannot be null
	errno = 0xcafebabe;
	rc = tok_parse_ull(NULL, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// value cannot be null
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, NULL, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdeadbeaf, value);
	assert_string_equal("123", buf);

	// both cannot be null
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(NULL, NULL, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdeadbeaf, value);

	(void)state; // unused
}

static void tok_parse_ull_implicit_base_test(void **state)
{
	uint64_t value;
	char buf[128];
	int rc;

	// decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(123, value);
	assert_string_equal("123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x123, value);
	assert_string_equal("0x123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0X321");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x321, value);
	assert_string_equal("0X321", buf);

	// octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x53, value);
	assert_string_equal("0123", buf);

	// negative values are just big unsigned values
	// negative decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0xffffffffffffff85, value);
	assert_string_equal("-123", buf);

	// negative hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0xfffffffffffffedd, value);
	assert_string_equal("-0x123", buf);

	// negative octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0xffffffffffffffad, value);
	assert_string_equal("-0123", buf);

	(void)state; // unused
}

static void tok_parse_ull_explicit_base_test(void **state)
{
	uint64_t value;
	char buf[128];
	int rc;
	int i;

	int b[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			33, 34, 35};
	int v[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 17, 19, 21, 23, 25,
			27, 29, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 93, 97,
			101, 105, 109};

	// do not verify the base 0, it is already done

	// invalid base, valid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 1);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	assert_string_equal("1", buf);

	// valid base, invalid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz"); // base 35 [0-9, A-Y]
	for (i = 2; i < 36; i++) {
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_ull(buf, &value, i);
		assert_int_equal(-1, rc);
		assert_int_equal(EINVAL, errno);
		assert_int_equal(0, value);
		assert_string_equal("123abcz", buf);
	}

	// valid base, valid value
	for (i = 0; i < 34; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", i + 1);
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_ull(buf, &value, b[i]);
		assert_int_equal(0, rc);
		assert_int_equal(0, errno);
		assert_int_equal(v[i], value);
	}

	// base 36 [0-9, A-Z]
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 36);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x89489303, value); // observation

	// bad base
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "beef");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 37);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_ull_bounds_test(void **state)
{
	uint64_t value;
	char buf[128];
	int rc;

	// negative values are big unsigned values
	// decimal out of range (min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", -1);
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0xffffffffffffffff, value);

	// decimal out of range (max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "295147905179352842046");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0xffffffffffffffff, value);

	// hex out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0xffffffffffffffff1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-01");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0xffffffffffffffff, value);

	// octal out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "040000000000000000000000");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0xffffffffffffffff1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_ull_nan_test(void **state)
{
	uint64_t value;
	char buf[128];
	int rc;

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "hello");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-goodbye");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ull(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	(void)state; // unused
}

static void tok_parse_ul_null_parm_test(void **state)
{
	uint32_t value;
	char buf[128];
	int rc;

	// token cannot be null
	errno = 0xcafebabe;
	rc = tok_parse_ul(NULL, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// value cannot be null
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, NULL, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdeadbeaf, value);
	assert_string_equal("123", buf);

	// both cannot be null
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(NULL, NULL, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdeadbeaf, value);

	(void)state; // unused
}

static void tok_parse_ul_implicit_base_test(void **state)
{
	uint32_t value;
	char buf[128];
	int rc;

	// decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(123, value);
	assert_string_equal("123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x123, value);
	assert_string_equal("0x123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0X321");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x321, value);
	assert_string_equal("0X321", buf);

	// octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x53, value);
	assert_string_equal("0123", buf);

	// negative decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-123", buf);

	// negative hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0x123", buf);

	// negative octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0123", buf);

	(void)state; // unused
}

static void tok_parse_ul_explicit_base_test(void **state)
{
	uint32_t value;
	char buf[128];
	int rc;
	int i;

	int b[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			33, 34, 35};
	int v[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 17, 19, 21, 23, 25,
			27, 29, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 93, 97,
			101, 105, 109};

	// do not verify the base 0, it is already done

	// invalid base, valid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 1);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	assert_string_equal("1", buf);

	// valid base, invalid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz"); // base 35 [0-9, A-Y]
	for (i = 2; i < 36; i++) {
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_ul(buf, &value, i);
		assert_int_equal(-1, rc);
		assert_int_equal(EINVAL, errno);
		assert_int_equal(0, value);
		assert_string_equal("123abcz", buf);
	}

	// valid base, valid value
	for (i = 0; i < 34; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", i + 1);
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_ul(buf, &value, b[i]);
		assert_int_equal(0, rc);
		assert_int_equal(0, errno);
		assert_int_equal(v[i], value);
	}

	// base 36 [0-9, A-Z]
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 36);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x89489303, value); // observation

	// bad base
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "beef");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 37);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_ul_bounds_test(void **state)
{
	uint32_t value;
	char buf[128];
	int rc;

	// decimal out of range (min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// decimal out of range (max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "68719476721");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0xffffffff1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-01");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0777777777761");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// greater than max allowed
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0xfffffffffffffffff");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_ul_nan_test(void **state)
{
	uint32_t value;
	char buf[128];
	int rc;

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "hello");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-goodbye");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ul(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	(void)state; // unused
}

static void tok_parse_us_null_parm_test(void **state)
{
	uint16_t value;
	char buf[128];
	int rc;

	// token cannot be null
	errno = 0xcafebabe;
	rc = tok_parse_us(NULL, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// value cannot be null
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, NULL, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdead, value);
	assert_string_equal("123", buf);

	// both cannot be null
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(NULL, NULL, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdead, value);

	(void)state; // unused
}

static void tok_parse_us_implicit_base_test(void **state)
{
	uint16_t value;
	char buf[128];
	int rc;

	// decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(123, value);
	assert_string_equal("123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x123, value);
	assert_string_equal("0x123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0X321");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x321, value);
	assert_string_equal("0X321", buf);

	// octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x53, value);
	assert_string_equal("0123", buf);

	// negative decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-123", buf);

	// negative hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0x123", buf);

	// negative octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);
	assert_string_equal("-0123", buf);

	(void)state; // unused
}

static void tok_parse_us_explicit_base_test(void **state)
{
	uint16_t value;
	char buf[128];
	int rc;
	int i;

	int b[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			33, 34, 35};
	int v[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 17, 19, 21, 23, 25,
			27, 29, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 93, 97,
			101, 105, 109};

	// do not verify the base 0, it is already done

	// invalid base, valid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 1);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	assert_string_equal("1", buf);

	// valid base, invalid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz"); // base 35 [0-9, A-Y]
	for (i = 2; i < 36; i++) {
		errno = 0xcafebabe;
		value = 0xdead;
		rc = tok_parse_us(buf, &value, i);
		assert_int_equal(-1, rc);
		assert_int_equal(EINVAL, errno);
		assert_int_equal(0, value);
		assert_string_equal("123abcz", buf);
	}

	// valid base, valid value
	for (i = 0; i < 34; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", i + 1);
		errno = 0xcafebabe;
		value = 0xdead;
		rc = tok_parse_us(buf, &value, b[i]);
		assert_int_equal(0, rc);
		assert_int_equal(0, errno);
		assert_int_equal(v[i], value);
	}

	// base 36 [0-9, A-Z]
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "bcz");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 36);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(14723, value); // observation

	// bad base
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "beef");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 37);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_us_bounds_test(void **state)
{
	uint16_t value;
	char buf[128];
	int rc;

	// decimal out of range (min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-1");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// decimal out of range (max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1048561");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x1");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0xffff1");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-01");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// octal out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "03777761");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// greater than max allowed
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0xfffffffffffffffff");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_us_nan_test(void **state)
{
	uint16_t value;
	char buf[128];
	int rc;

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "hello");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-goodbye");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_us(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	(void)state; // unused
}

static void tok_parse_ll_null_parm_test(void **state)
{
	int64_t value;
	char buf[128];
	int rc;

	// token cannot be null
	errno = 0xcafebabe;
	rc = tok_parse_ll(NULL, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// value cannot be null
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, NULL, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdeadbeaf, value);
	assert_string_equal("123", buf);

	// both cannot be null
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(NULL, NULL, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0xdeadbeaf, value);

	(void)state; // unused
}

static void tok_parse_ll_implicit_base_test(void **state)
{
	int64_t value;
	char buf[128];
	int rc;

	// decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(123, value);
	assert_string_equal("123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x123, value);
	assert_string_equal("0x123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0X321");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x321, value);
	assert_string_equal("0X321", buf);

	// octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x53, value);
	assert_string_equal("0123", buf);

	// negative decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(-123, value);
	assert_string_equal("-123", buf);

	// negative hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(-0x123, value);
	assert_string_equal("-0x123", buf);

	// negative octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(-83, value);
	assert_string_equal("-0123", buf);

	(void)state; // unused
}

static void tok_parse_ll_explicit_base_test(void **state)
{
	int64_t value;
	char buf[128];
	int rc;
	int i;

	int b[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			33, 34, 35};
	int v[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 17, 19, 21, 23, 25,
			27, 29, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 93, 97,
			101, 105, 109};

	// do not verify the base 0, it is already done

	// invalid base, valid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 1);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	assert_string_equal("1", buf);

	// valid base, invalid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz"); // base 35 [0-9, A-Y]
	for (i = 2; i < 36; i++) {
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_ll(buf, &value, i);
		assert_int_equal(-1, rc);
		assert_int_equal(EINVAL, errno);
		assert_int_equal(0, value);
		assert_string_equal("123abcz", buf);
	}

	// valid base, valid value
	for (i = 0; i < 34; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", i + 1);
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_ll(buf, &value, b[i]);
		assert_int_equal(0, rc);
		assert_int_equal(0, errno);
		assert_int_equal(v[i], value);
	}

	// base 36 [0-9, A-Z]
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 36);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x89489303, value); // observation

	// bad base
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "beef");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 37);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_ll_bounds_test(void **state)
{
	int64_t value;
	char buf[128];
	int rc;

	// decimal out of range (min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-295147905179352842048");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// decimal out of range (max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "295147905179352842046");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0xffffffffffffffff1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0xffffffffffffffff1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_ll_nan_test(void **state)
{
	int64_t value;
	char buf[128];
	int rc;

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "hello");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-goodbye");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_ll(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	(void)state; // unused
}

static void tok_parse_l_null_parm_test(void **state)
{
	int32_t value;
	char buf[128];
	int rc;

	// token cannot be null
	errno = 0xcafebabe;
	rc = tok_parse_l(NULL, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// value cannot be null
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0x78123;
	rc = tok_parse_l(buf, NULL, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0x78123, value);
	assert_string_equal("123", buf);

	// both cannot be null
	errno = 0xcafebabe;
	value = 0x78123;
	rc = tok_parse_l(NULL, NULL, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0x78123, value);

	(void)state; // unused
}

static void tok_parse_l_implicit_base_test(void **state)
{
	int32_t value;
	char buf[128];
	int rc;

	// decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(123, value);
	assert_string_equal("123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x123, value);
	assert_string_equal("0x123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0X321");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x321, value);
	assert_string_equal("0X321", buf);

	// octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x53, value);
	assert_string_equal("0123", buf);

	// negative decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(-123, value);
	assert_string_equal("-123", buf);

	// negative hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(-0x123, value);
	assert_string_equal("-0x123", buf);

	// negative octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0123");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(-0123, value);
	assert_string_equal("-0123", buf);

	(void)state; // unused
}

static void tok_parse_l_explicit_base_test(void **state)
{
	int32_t value;
	char buf[128];
	int rc;
	int i;

	int b[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			33, 34, 35};
	int v[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 17, 19, 21, 23, 25,
			27, 29, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 93, 97,
			101, 105, 109};

	// do not verify the base 0, it is already done

	// invalid base, valid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 1);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	assert_string_equal("1", buf);

	// valid base, invalid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz"); // base 35 [0-9, A-Y]
	for (i = 2; i < 36; i++) {
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_l(buf, &value, i);
		assert_int_equal(-1, rc);
		assert_int_equal(EINVAL, errno);
		assert_int_equal(0, value);
		assert_string_equal("123abcz", buf);
	}

	// valid base, valid value
	for (i = 0; i < 34; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", i + 1);
		errno = 0xcafebabe;
		value = 0xdeadbeaf;
		rc = tok_parse_l(buf, &value, b[i]);
		assert_int_equal(0, rc);
		assert_int_equal(0, errno);
		assert_int_equal(v[i], value);
	}

	// base 36 [0-9, A-Z]
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "bcz");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 36);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(14723, value); // observation

	// bad base
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "beef");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 37);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_l_bounds_test(void **state)
{
	int32_t value;
	char buf[128];
	int rc;

	// decimal out of range (min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-4294967295");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// decimal out of range (max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "4294967296");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0xffffffff1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0xffffffff1");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_l_nan_test(void **state)
{
	int32_t value;
	char buf[128];
	int rc;

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "hello");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-goodbye");
	errno = 0xcafebabe;
	value = 0xdeadbeaf;
	rc = tok_parse_l(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	(void)state; // unused
}

static void tok_parse_s_null_parm_test(void **state)
{
	int16_t value;
	char buf[128];
	int rc;

	// token cannot be null
	errno = 0xcafebabe;
	rc = tok_parse_s(NULL, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// value cannot be null
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0x1234;
	rc = tok_parse_s(buf, NULL, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0x1234, value);
	assert_string_equal("123", buf);

	// both cannot be null
	errno = 0xcafebabe;
	value = 0x1234;
	rc = tok_parse_s(NULL, NULL, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0x1234, value);

	(void)state; // unused
}

static void tok_parse_s_implicit_base_test(void **state)
{
	int16_t value;
	char buf[128];
	int rc;

	// decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(123, value);
	assert_string_equal("123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0x123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x123, value);
	assert_string_equal("0x123", buf);

	// hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0X321");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x321, value);
	assert_string_equal("0X321", buf);

	// octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(0x53, value);
	assert_string_equal("0123", buf);

	// negative decimal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(-123, value);
	assert_string_equal("-123", buf);

	// negative hex
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0x123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(-0x123, value);
	assert_string_equal("-0x123", buf);

	// negative octal
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0123");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(-0123, value);
	assert_string_equal("-0123", buf);

	(void)state; // unused
}

static void tok_parse_s_explicit_base_test(void **state)
{
	int16_t value;
	char buf[128];
	int rc;
	int i;

	int b[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			33, 34, 35};
	int v[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 17, 19, 21, 23, 25,
			27, 29, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 93, 97,
			101, 105, 109};

	// do not verify the base 0, it is already done

	// invalid base, valid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 1);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);
	assert_string_equal("1", buf);

	// valid base, invalid value
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "123abcz"); // base 35 [0-9, A-Y]
	for (i = 2; i < 36; i++) {
		errno = 0xcafebabe;
		value = 0xdead;
		rc = tok_parse_s(buf, &value, i);
		assert_int_equal(-1, rc);
		assert_int_equal(EINVAL, errno);
		assert_int_equal(0, value);
		assert_string_equal("123abcz", buf);
	}

	// valid base, valid value
	for (i = 0; i < 34; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", i + 1);
		errno = 0xcafebabe;
		value = 0xdead;
		rc = tok_parse_s(buf, &value, b[i]);
		assert_int_equal(0, rc);
		assert_int_equal(0, errno);
		assert_int_equal(v[i], value);
	}

	// base 36 [0-9, A-Z]
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "bcz");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 36);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(14723, value); // observation

	// bad base
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "beef");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 37);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_s_bounds_test(void **state)
{
	int16_t value;
	char buf[128];
	int rc;

	// decimal out of range (min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-1048561");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// decimal out of range (max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1048562");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(min)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-0xffff1");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	// hex out of range(max)
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "0xffff1");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_s_nan_test(void **state)
{
	int16_t value;
	char buf[128];
	int rc;

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "hello");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	// nan
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "-goodbye");
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_s(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(EINVAL, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_did_test(void **state)
{
	did_val_t value;
	char buf[128];
	int rc;

	// fast range check
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", RIO_LAST_DEV8);
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_did(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(RIO_LAST_DEV8, value);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", RIO_LAST_DEV16 + 1);
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_did(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_ct_test(void **state)
{
	ct_t value;
	char buf[128];
	int rc;

	// fast range check
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%llu", (unsigned long long)UINT32_MAX);
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ct(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(UINT32_MAX, value);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%llu", (unsigned long long)(UINT32_MAX) + 1);
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_ct(buf, &value, 0);
	assert_int_equal(0, value);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);

	(void)state; // unused
}

static void tok_parse_hc_test(void **state)
{
	hc_t value;
	char buf[128];
	int rc;

	// fast range check
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", HC_MP);
	errno = 0xcafebabe;
	value = 0x12;
	rc = tok_parse_hc(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(HC_MP, value);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", HC_MP + 1);
	errno = 0xcafebabe;
	value = 0x12;
	rc = tok_parse_hc(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_mport_test(void **state)
{
	uint32_t value;
	char buf[128];
	int rc;

	// fast range check
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", RIO_MAX_MPORTS - 1);
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_mport_id(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(RIO_MAX_MPORTS - 1, value);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", RIO_MAX_MPORTS);
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_mport_id(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_log_level_test(void **state)
{
	uint32_t value;
	char buf[128];
	int rc;

	// fast range check
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", RDMA_LL_DBG);
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_log_level(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(RDMA_LL_DBG, value);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", RDMA_LL_DBG + 1);
	errno = 0xcafebabe;
	value = 0xdead;
	rc = tok_parse_log_level(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_socket_test(void **state)
{
	uint16_t value;
	char buf[128];
	int rc;

	// fast range check
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%llu", (unsigned long long)UINT16_MAX);
	errno = 0xcafebabe;
	value = 0x123;
	rc = tok_parse_socket(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(UINT16_MAX, value);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%llu", (unsigned long long)(UINT16_MAX + 1));
	errno = 0xcafebabe;
	value = 0x123;
	rc = tok_parse_socket(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

static void tok_parse_port_num_test(void **state)
{
	uint32_t value;
	char buf[128];
	int rc;

	// fast range check
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", RIO_MAX_DEV_PORT - 1);
	errno = 0xcafebabe;
	value = 0x123;
	rc = tok_parse_port_num(buf, &value, 0);
	assert_int_equal(0, rc);
	assert_int_equal(0, errno);
	assert_int_equal(RIO_MAX_DEV_PORT - 1, value);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", RIO_MAX_DEV_PORT);
	errno = 0xcafebabe;
	value = 0x123;
	rc = tok_parse_port_num(buf, &value, 0);
	assert_int_equal(-1, rc);
	assert_int_equal(ERANGE, errno);
	assert_int_equal(0, value);

	(void)state; // unused
}

int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
	cmocka_unit_test(msg_fmt_test),
	cmocka_unit_test(tok_parse_ulonglong_null_parm_test),
	cmocka_unit_test(tok_parse_ulonglong_implicit_base_test),
	cmocka_unit_test(tok_parse_ulonglong_explicit_base_test),
	cmocka_unit_test(tok_parse_ulonglong_bounds_test),
	cmocka_unit_test(tok_parse_ulonglong_nan_test),
	cmocka_unit_test(tok_parse_ulong_null_parm_test),
	cmocka_unit_test(tok_parse_ulong_implicit_base_test),
	cmocka_unit_test(tok_parse_ulong_explicit_base_test),
	cmocka_unit_test(tok_parse_ulong_bounds_test),
	cmocka_unit_test(tok_parse_ulong_nan_test),
	cmocka_unit_test(tok_parse_ushort_null_parm_test),
	cmocka_unit_test(tok_parse_ushort_implicit_base_test),
	cmocka_unit_test(tok_parse_ushort_explicit_base_test),
	cmocka_unit_test(tok_parse_ushort_bounds_test),
	cmocka_unit_test(tok_parse_ushort_nan_test),
	cmocka_unit_test(tok_parse_longlong_null_parm_test),
	cmocka_unit_test(tok_parse_longlong_implicit_base_test),
	cmocka_unit_test(tok_parse_longlong_explicit_base_test),
	cmocka_unit_test(tok_parse_longlong_bounds_test),
	cmocka_unit_test(tok_parse_longlong_nan_test),
	cmocka_unit_test(tok_parse_long_null_parm_test),
	cmocka_unit_test(tok_parse_long_implicit_base_test),
	cmocka_unit_test(tok_parse_long_explicit_base_test),
	cmocka_unit_test(tok_parse_long_bounds_test),
	cmocka_unit_test(tok_parse_long_nan_test),
	cmocka_unit_test(tok_parse_short_null_parm_test),
	cmocka_unit_test(tok_parse_short_implicit_base_test),
	cmocka_unit_test(tok_parse_short_explicit_base_test),
	cmocka_unit_test(tok_parse_short_bounds_test),
	cmocka_unit_test(tok_parse_short_nan_test),
	cmocka_unit_test(tok_parse_ull_null_parm_test),
	cmocka_unit_test(tok_parse_ull_implicit_base_test),
	cmocka_unit_test(tok_parse_ull_explicit_base_test),
	cmocka_unit_test(tok_parse_ull_bounds_test),
	cmocka_unit_test(tok_parse_ull_nan_test),
	cmocka_unit_test(tok_parse_ul_null_parm_test),
	cmocka_unit_test(tok_parse_ul_implicit_base_test),
	cmocka_unit_test(tok_parse_ul_explicit_base_test),
	cmocka_unit_test(tok_parse_ul_bounds_test),
	cmocka_unit_test(tok_parse_ul_nan_test),
	cmocka_unit_test(tok_parse_us_null_parm_test),
	cmocka_unit_test(tok_parse_us_implicit_base_test),
	cmocka_unit_test(tok_parse_us_explicit_base_test),
	cmocka_unit_test(tok_parse_us_bounds_test),
	cmocka_unit_test(tok_parse_us_nan_test),
	cmocka_unit_test(tok_parse_ll_null_parm_test),
	cmocka_unit_test(tok_parse_ll_implicit_base_test),
	cmocka_unit_test(tok_parse_ll_explicit_base_test),
	cmocka_unit_test(tok_parse_ll_bounds_test),
	cmocka_unit_test(tok_parse_ll_nan_test),
	cmocka_unit_test(tok_parse_l_null_parm_test),
	cmocka_unit_test(tok_parse_l_implicit_base_test),
	cmocka_unit_test(tok_parse_l_explicit_base_test),
	cmocka_unit_test(tok_parse_l_bounds_test),
	cmocka_unit_test(tok_parse_l_nan_test),
	cmocka_unit_test(tok_parse_s_null_parm_test),
	cmocka_unit_test(tok_parse_s_implicit_base_test),
	cmocka_unit_test(tok_parse_s_explicit_base_test),
	cmocka_unit_test(tok_parse_s_bounds_test),
	cmocka_unit_test(tok_parse_s_nan_test),
	cmocka_unit_test(tok_parse_did_test),
	cmocka_unit_test(tok_parse_ct_test),
	cmocka_unit_test(tok_parse_hc_test),
	cmocka_unit_test(tok_parse_mport_test),
	cmocka_unit_test(tok_parse_log_level_test),
	cmocka_unit_test(tok_parse_socket_test),
	cmocka_unit_test(tok_parse_port_num_test), };
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#ifdef __cplusplus
}
#endif
