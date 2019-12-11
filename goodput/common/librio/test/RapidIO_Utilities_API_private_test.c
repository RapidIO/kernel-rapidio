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

#include "RapidIO_Utilities_API.h"
#include "src/RapidIO_Utilities_API.c"
#include "rio_ecosystem.h"

#ifdef __cplusplus
extern "C" {
#endif

static void assumptions(void **state)
{
	assert_int_equal(0xFFFF, BAD_FTYPE);
	assert_int_equal(0xFFFFFFFF, BAD_SIZE);
	assert_int_equal(0xFFFFFFFF, SIZE_RC_FAIL);

	assert_int_equal(35, NUM_CRC13_VECTORS);

	assert_int_equal(0x7C, PD_CONTROL_SYMBOL);
	assert_int_equal(0x1C, SC_START_CONTROL_SYMBOL);

	assert_int_equal(0, PNA_RESERVED_IDX);
	assert_int_equal(8, MAX_PNA_CAUSE_IDX);

	(void)state; // unused
}

static void DAR_util_get_ftype_test(void **state)
{
	assert_int_equal(16, DAR_util_get_ftype(pkt_raw));

	assert_int_equal(2, DAR_util_get_ftype(pkt_nr));
	assert_int_equal(2, DAR_util_get_ftype(pkt_nr_inc));
	assert_int_equal(2, DAR_util_get_ftype(pkt_nr_dec));
	assert_int_equal(2, DAR_util_get_ftype(pkt_nr_set));
	assert_int_equal(2, DAR_util_get_ftype(pkt_nr_clr));

	assert_int_equal(5, DAR_util_get_ftype(pkt_nw));
	assert_int_equal(5, DAR_util_get_ftype(pkt_nwr));
	assert_int_equal(5, DAR_util_get_ftype(pkt_nw_swap));
	assert_int_equal(5, DAR_util_get_ftype(pkt_nw_cmp_swap));
	assert_int_equal(5, DAR_util_get_ftype(pkt_nw_tst_swap));

	assert_int_equal(6, DAR_util_get_ftype(pkt_sw));

	assert_int_equal(7, DAR_util_get_ftype(pkt_fc));

	assert_int_equal(8, DAR_util_get_ftype(pkt_mr));
	assert_int_equal(8, DAR_util_get_ftype(pkt_mw));
	assert_int_equal(8, DAR_util_get_ftype(pkt_mrr));
	assert_int_equal(8, DAR_util_get_ftype(pkt_mwr));
	assert_int_equal(8, DAR_util_get_ftype(pkt_pw));

	assert_int_equal(9, DAR_util_get_ftype(pkt_dstm));

	assert_int_equal(10, DAR_util_get_ftype(pkt_db));

	assert_int_equal(11, DAR_util_get_ftype(pkt_msg));

	assert_int_equal(13, DAR_util_get_ftype(pkt_resp));
	assert_int_equal(13, DAR_util_get_ftype(pkt_resp_data));

	assert_int_equal(13, DAR_util_get_ftype(pkt_msg_resp));

	assert_int_equal(BAD_FTYPE,
			DAR_util_get_ftype((DAR_pkt_type )(pkt_type_max + 1)));

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_1_test(void **state)
{
	const uint32_t pkt_bytes = 1;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0, wdptr);
			assert_int_equal(0, rdsize);
			break;
		case 0x1:
		case 0x9:
			assert_int_equal(0, wdptr);
			assert_int_equal(1, rdsize);
			break;

		case 0x2:
		case 0xa:
			assert_int_equal(0, wdptr);
			assert_int_equal(2, rdsize);
			break;
		case 0x3:
		case 0xb:
			assert_int_equal(0, wdptr);
			assert_int_equal(3, rdsize);
			break;
		case 0x4:
		case 0xc:
			assert_int_equal(1, wdptr);
			assert_int_equal(0, rdsize);
			break;
		case 0x5:
		case 0xd:
			assert_int_equal(1, wdptr);
			assert_int_equal(1, rdsize);
			break;
		case 0x6:
		case 0xe:
			assert_int_equal(1, wdptr);
			assert_int_equal(2, rdsize);
			break;
		case 0x7:
		case 0xf:
			assert_int_equal(1, wdptr);
			assert_int_equal(3, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_2_test(void **state)
{
	const uint32_t pkt_bytes = 2;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0, wdptr);
			assert_int_equal(4, rdsize);
			break;
		case 0x2:
		case 0xa:
			assert_int_equal(0, wdptr);
			assert_int_equal(6, rdsize);
			break;
		case 0x4:
		case 0xc:
			assert_int_equal(1, wdptr);
			assert_int_equal(4, rdsize);
			break;
		case 0x6:
		case 0xe:
			assert_int_equal(1, wdptr);
			assert_int_equal(6, rdsize);
			break;
		case 0x1:
		case 0x3:
		case 0x5:
		case 0x7:
		case 0x9:
		case 0xb:
		case 0xd:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_3_test(void **state)
{
	const uint32_t pkt_bytes = 3;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0, wdptr);
			assert_int_equal(5, rdsize);
			break;
		case 0x5:
		case 0xd:
			assert_int_equal(1, wdptr);
			assert_int_equal(5, rdsize);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_4_test(void **state)
{
	const uint32_t pkt_bytes = 4;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0, wdptr);
			assert_int_equal(8, rdsize);
			break;
		case 0x4:
		case 0xc:
			assert_int_equal(1, wdptr);
			assert_int_equal(8, rdsize);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_5_test(void **state)
{
	const uint32_t pkt_bytes = 5;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0, wdptr);
			assert_int_equal(7, rdsize);
			break;
		case 0x3:
		case 0xb:
			assert_int_equal(1, wdptr);
			assert_int_equal(7, rdsize);
			break;
		case 0x1:
		case 0x2:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_6_test(void **state)
{
	const uint32_t pkt_bytes = 6;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0, wdptr);
			assert_int_equal(9, rdsize);
			break;
		case 0x2:
		case 0xa:
			assert_int_equal(1, wdptr);
			assert_int_equal(9, rdsize);
			break;
		case 0x1:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_7_test(void **state)
{
	const uint32_t pkt_bytes = 7;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0, wdptr);
			assert_int_equal(0xa, rdsize);
			break;
		case 0x1:
		case 0x9:
			assert_int_equal(1, wdptr);
			assert_int_equal(0xa, rdsize);
			break;
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_8_test(void **state)
{
	const uint32_t pkt_bytes = 8;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0, wdptr);
			assert_int_equal(0xb, rdsize);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_16_test(void **state)
{
	const uint32_t pkt_bytes = 16;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(1, wdptr);
			assert_int_equal(0xb, rdsize);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_32_test(void **state)
{
	const uint32_t pkt_bytes = 32;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0, wdptr);
			assert_int_equal(0xc, rdsize);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_64_test(void **state)
{
	const uint32_t pkt_bytes = 64;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(1, wdptr);
			assert_int_equal(0xc, rdsize);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_96_test(void **state)
{
	const uint32_t pkt_bytes = 96;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0, wdptr);
			assert_int_equal(0xd, rdsize);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_128_test(void **state)
{
	const uint32_t pkt_bytes = 128;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(1, wdptr);
			assert_int_equal(0xd, rdsize);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_160_test(void **state)
{
	const uint32_t pkt_bytes = 160;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0, wdptr);
			assert_int_equal(0xe, rdsize);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_192_test(void **state)
{
	const uint32_t pkt_bytes = 192;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(1, wdptr);
			assert_int_equal(0xe, rdsize);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_224_test(void **state)
{
	const uint32_t pkt_bytes = 224;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0, wdptr);
			assert_int_equal(0xf, rdsize);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_pkt_bytes_256_test(void **state)
{
	const uint32_t pkt_bytes = 256;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		rdsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize, &wdptr);
		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(1, wdptr);
			assert_int_equal(0xf, rdsize);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_rdsize_wdptr_test(void **state)
{
	uint32_t pkt_bytes;

	uint32_t addr;
	uint32_t rdsize;
	uint32_t wdptr;
	int idx;

	wdptr = 0xbeef;
	for (pkt_bytes = 0; pkt_bytes < 1024; pkt_bytes++) {
		for (idx = 0; idx < 0x10; idx++) {
			addr = 0xcafebab0 + idx;
			rdsize = 0xdead;

			switch (pkt_bytes) {
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 16:
			case 32:
			case 64:
			case 96:
			case 128:
			case 160:
			case 192:
			case 224:
			case 256:
				// handled above
				continue;
			default:
				break;
			}

			DAR_util_get_rdsize_wdptr(addr, pkt_bytes, &rdsize,
					&wdptr);
			assert_int_equal(BAD_SIZE, wdptr);
			assert_int_equal(BAD_SIZE, rdsize);
		}
	}

	(void)state; // unused
}

static void DAR_util_compute_rd_bytes_n_align_wptr_0_test(void **state)
{
	const uint32_t wptr = 0;

	uint32_t num_bytes;
	uint32_t align;
	uint32_t rc;

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(0, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(1, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(1, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(2, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(2, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(3, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(3, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(4, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(2, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(5, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(3, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(6, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(2, num_bytes);
	assert_int_equal(2, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(7, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(5, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(8, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(4, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(9, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(6, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(10, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(7, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(11, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(8, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(12, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0x20, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(13, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0x60, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(14, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0xa0, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(15, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0xe0, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(16, wptr, &num_bytes, &align);
	assert_int_equal(SIZE_RC_FAIL, rc);
	assert_int_equal(0, num_bytes);
	assert_int_equal(0, align);

	(void)state; // not used
}

static void DAR_util_compute_rd_bytes_n_align_wptr_1_test(void **state)
{
	const uint32_t wptr = 1;

	uint32_t num_bytes;
	uint32_t align;
	uint32_t rc;

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(0, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(4, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(1, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(5, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(2, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(6, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(3, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(7, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(4, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(2, num_bytes);
	assert_int_equal(4, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(5, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(3, num_bytes);
	assert_int_equal(5, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(6, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(2, num_bytes);
	assert_int_equal(6, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(7, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(5, num_bytes);
	assert_int_equal(3, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(8, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(4, num_bytes);
	assert_int_equal(4, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(9, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(6, num_bytes);
	assert_int_equal(2, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(10, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(7, num_bytes);
	assert_int_equal(1, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(11, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0x10, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(12, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0x40, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(13, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0x80, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(14, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0xc0, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(15, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0x100, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_rd_bytes_n_align(16, wptr, &num_bytes, &align);
	assert_int_equal(SIZE_RC_FAIL, rc);
	assert_int_equal(0, num_bytes);
	assert_int_equal(0, align);

	(void)state; // not used
}

static void DAR_util_compute_rd_bytes_n_align_test(void **state)
{
	uint32_t wptr;
	uint32_t num_bytes;
	uint32_t align;
	uint32_t rc;

	for (wptr = 2; wptr < 0x100; wptr++) {
		rc = DAR_util_compute_rd_bytes_n_align(9, wptr, &num_bytes,
				&align);
		assert_int_equal(SIZE_RC_FAIL, rc);
		assert_int_equal(0, num_bytes);
		assert_int_equal(0, align);
	}

	(void)state; // not used
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_0_test(void **state)
{
	const uint32_t pkt_bytes = 0;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);
		assert_int_equal(BAD_SIZE, wrsize);
		assert_int_equal(BAD_SIZE, wdptr);
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_1_test(void **state)
{
	const uint32_t pkt_bytes = 1;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		switch (idx) {
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb:
			assert_int_equal(idx % 4, wrsize);
			assert_int_equal(0, wdptr);
			break;
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(idx % 4, wrsize);
			assert_int_equal(1, wdptr);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_2_test(void **state)
{
	const uint32_t pkt_bytes = 2;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(4, wrsize);
			assert_int_equal(0, wdptr);
			break;
		case 0x2:
		case 0xa:
			assert_int_equal(6, wrsize);
			assert_int_equal(0, wdptr);
			break;
		case 0x4:
		case 0xc:
			assert_int_equal(4, wrsize);
			assert_int_equal(1, wdptr);
			break;
		case 0x6:
		case 0xe:
			assert_int_equal(6, wrsize);
			assert_int_equal(1, wdptr);
			break;
		case 0x1:
		case 0x3:
		case 0x5:
		case 0x7:
		case 0x9:
		case 0xb:
		case 0xd:
		case 0xf:
			assert_int_equal(BAD_SIZE, wrsize);
			assert_int_equal(BAD_SIZE, wdptr);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_3_test(void **state)
{
	const uint32_t pkt_bytes = 3;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(5, wrsize);
			assert_int_equal(0, wdptr);
			break;
		case 0x5:
		case 0xd:
			assert_int_equal(5, wrsize);
			assert_int_equal(1, wdptr);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wrsize);
			assert_int_equal(BAD_SIZE, wdptr);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_4_test(void **state)
{
	const uint32_t pkt_bytes = 4;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(8, wrsize);
			assert_int_equal(0, wdptr);
			break;
		case 0x4:
		case 0xc:
			assert_int_equal(8, wrsize);
			assert_int_equal(1, wdptr);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wrsize);
			assert_int_equal(BAD_SIZE, wdptr);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_5_test(void **state)
{
	const uint32_t pkt_bytes = 5;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(7, wrsize);
			assert_int_equal(0, wdptr);
			break;
		case 0x3:
		case 0xb:
			assert_int_equal(7, wrsize);
			assert_int_equal(1, wdptr);
			break;
		case 0x1:
		case 0x2:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wrsize);
			assert_int_equal(BAD_SIZE, wdptr);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_6_test(void **state)
{
	const uint32_t pkt_bytes = 6;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(9, wrsize);
			assert_int_equal(0, wdptr);
			break;
		case 0x2:
		case 0xa:
			assert_int_equal(9, wrsize);
			assert_int_equal(1, wdptr);
			break;
		case 0x1:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wrsize);
			assert_int_equal(BAD_SIZE, wdptr);
			break;

		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_7_test(void **state)
{
	const uint32_t pkt_bytes = 7;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0xa, wrsize);
			assert_int_equal(0, wdptr);
			break;
		case 0x1:
		case 0x9:
			assert_int_equal(0xa, wrsize);
			assert_int_equal(1, wdptr);
			break;
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wrsize);
			assert_int_equal(BAD_SIZE, wdptr);
			break;

		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_8_test(void **state)
{
	const uint32_t pkt_bytes = 8;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0xb, wrsize);
			assert_int_equal(0, wdptr);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wrsize);
			assert_int_equal(BAD_SIZE, wdptr);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_9_test(void **state)
{
	const uint32_t pkt_bytes = 9;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		assert_int_equal(BAD_SIZE, wrsize);
		assert_int_equal(BAD_SIZE, wdptr);
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_10_test(void **state)
{
	const uint32_t pkt_bytes = 10;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		assert_int_equal(BAD_SIZE, wrsize);
		assert_int_equal(BAD_SIZE, wdptr);
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_11_test(void **state)
{
	const uint32_t pkt_bytes = 11;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		assert_int_equal(BAD_SIZE, wrsize);
		assert_int_equal(BAD_SIZE, wdptr);
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_12_test(void **state)
{
	const uint32_t pkt_bytes = 12;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		assert_int_equal(BAD_SIZE, wrsize);
		assert_int_equal(BAD_SIZE, wdptr);
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_13_test(void **state)
{
	const uint32_t pkt_bytes = 13;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		assert_int_equal(BAD_SIZE, wrsize);
		assert_int_equal(BAD_SIZE, wdptr);
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_14_test(void **state)
{
	const uint32_t pkt_bytes = 14;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		assert_int_equal(BAD_SIZE, wrsize);
		assert_int_equal(BAD_SIZE, wdptr);
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_15_test(void **state)
{
	const uint32_t pkt_bytes = 15;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		assert_int_equal(BAD_SIZE, wrsize);
		assert_int_equal(BAD_SIZE, wdptr);
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_pkt_bytes_16_test(void **state)
{
	const uint32_t pkt_bytes = 16;

	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (idx = 0; idx < 0x10; idx++) {
		addr = 0xcafebab0 + idx;
		wrsize = 0xdead;
		wdptr = 0xbeef;

		DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize, &wdptr);

		switch (idx) {
		case 0x0:
		case 0x8:
			assert_int_equal(0xb, wrsize);
			assert_int_equal(1, wdptr);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			assert_int_equal(BAD_SIZE, wrsize);
			assert_int_equal(BAD_SIZE, wdptr);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void DAR_util_get_wrsize_wdptr_test(void **state)
{
	uint32_t pkt_bytes;
	uint32_t addr;
	uint32_t wrsize;
	uint32_t wdptr;
	int idx;

	// the least significant digit of the address is used to determine
	// the return values, so cycle through 0-F with a random value in the
	// most significant digits

	for (pkt_bytes = 17; pkt_bytes < 257; pkt_bytes++) {
		for (idx = 0; idx < 0x10; idx++) {
			addr = 0xcafebab0 + idx;
			wrsize = 0xdead;
			wdptr = 0xbeef;

			DAR_util_get_wrsize_wdptr(addr, pkt_bytes, &wrsize,
					&wdptr);

			switch (pkt_bytes) {
			case 0x18:
			case 0x20:
				if ((0 == idx) || (8 == idx)) {
					assert_int_equal(0xc, wrsize);
					assert_int_equal(0, wdptr);
				} else {
					assert_int_equal(BAD_SIZE, wrsize);
					assert_int_equal(BAD_SIZE, wdptr);
				}
				break;
			case 0x28:
			case 0x30:
			case 0x38:
			case 0x40:
				if ((0 == idx) || (8 == idx)) {
					assert_int_equal(0xc, wrsize);
					assert_int_equal(1, wdptr);
				} else {
					assert_int_equal(BAD_SIZE, wrsize);
					assert_int_equal(BAD_SIZE, wdptr);
				}
				break;
			case 0x48:
			case 0x50:
			case 0x58:
			case 0x60:
			case 0x68:
			case 0x70:
			case 0x78:
			case 0x80:
				if ((0 == idx) || (8 == idx)) {
					assert_int_equal(0xd, wrsize);
					assert_int_equal(1, wdptr);
				} else {
					assert_int_equal(BAD_SIZE, wrsize);
					assert_int_equal(BAD_SIZE, wdptr);
				}
				break;
			case 0x88:
			case 0x90:
			case 0x98:
			case 0xa0:
			case 0xa8:
			case 0xb0:
			case 0xb8:
			case 0xc0:
			case 0xc8:
			case 0xd0:
			case 0xd8:
			case 0xe0:
			case 0xe8:
			case 0xf0:
			case 0xf8:
			case 0x100:
				if ((0 == idx) || (8 == idx)) {
					assert_int_equal(0xf, wrsize);
					assert_int_equal(1, wdptr);
				} else {
					assert_int_equal(BAD_SIZE, wrsize);
					assert_int_equal(BAD_SIZE, wdptr);
				}
				break;
			default:
				assert_int_equal(BAD_SIZE, wrsize);
				assert_int_equal(BAD_SIZE, wdptr);
			}
		}
	}

	(void)state; // unused
}

static void DAR_util_compute_wr_bytes_n_align_wptr_0_test(void **state)
{
	const uint32_t wptr = 0;

	uint32_t num_bytes;
	uint32_t align;
	uint32_t rc;

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(0, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(1, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(1, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(2, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(2, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(3, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(3, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(4, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(2, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(5, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(3, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(6, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(2, num_bytes);
	assert_int_equal(2, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(7, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(5, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(8, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(4, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(9, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(6, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(10, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(7, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(11, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(8, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(12, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0x20, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(13, wptr, &num_bytes, &align);
	assert_int_equal(SIZE_RC_FAIL, rc);
	assert_int_equal(0, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(14, wptr, &num_bytes, &align);
	assert_int_equal(SIZE_RC_FAIL, rc);
	assert_int_equal(0, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(15, wptr, &num_bytes, &align);
	assert_int_equal(SIZE_RC_FAIL, rc);
	assert_int_equal(0, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(16, wptr, &num_bytes, &align);
	assert_int_equal(SIZE_RC_FAIL, rc);
	assert_int_equal(0, num_bytes);
	assert_int_equal(0, align);

	(void)state; // not used
}

static void DAR_util_compute_wr_bytes_n_align_wptr_1_test(void **state)
{
	const uint32_t wptr = 1;

	uint32_t num_bytes;
	uint32_t align;
	uint32_t rc;

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(0, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(4, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(1, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(5, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(2, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(6, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(3, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(1, num_bytes);
	assert_int_equal(7, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(4, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(2, num_bytes);
	assert_int_equal(4, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(5, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(3, num_bytes);
	assert_int_equal(5, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(6, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(2, num_bytes);
	assert_int_equal(6, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(7, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(5, num_bytes);
	assert_int_equal(3, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(8, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(4, num_bytes);
	assert_int_equal(4, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(9, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(6, num_bytes);
	assert_int_equal(2, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(10, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(7, num_bytes);
	assert_int_equal(1, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(11, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0x10, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(12, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0x40, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(13, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0x80, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(14, wptr, &num_bytes, &align);
	assert_int_equal(SIZE_RC_FAIL, rc);
	assert_int_equal(0, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(15, wptr, &num_bytes, &align);
	assert_int_equal(0, rc);
	assert_int_equal(0x100, num_bytes);
	assert_int_equal(0, align);

	num_bytes = 0x12345678;
	align = 0xdead;
	rc = DAR_util_compute_wr_bytes_n_align(16, wptr, &num_bytes, &align);
	assert_int_equal(SIZE_RC_FAIL, rc);
	assert_int_equal(0, num_bytes);
	assert_int_equal(0, align);

	(void)state; // not used
}

static void DAR_util_compute_wr_bytes_n_align_test(void **state)
{
	uint32_t wptr;
	uint32_t num_bytes;
	uint32_t align;
	uint32_t rc;

	for (wptr = 2; wptr < 0x100; wptr++) {
		rc = DAR_util_compute_wr_bytes_n_align(9, wptr, &num_bytes,
				&align);
		assert_int_equal(SIZE_RC_FAIL, rc);
		assert_int_equal(0, num_bytes);
		assert_int_equal(0, align);
	}

	(void)state; // not used
}

static void DAR_util_pkt_bytes_init_test(void **state)
{
	DAR_pkt_bytes_t comp_pkt;
	int idx;

	memset(&comp_pkt, 0xa5, sizeof(DAR_pkt_bytes_t));
	assert_int_equal(0xa5a5a5a5, comp_pkt.num_chars);
	for (idx = 0; idx < RIO_MAX_PKT_BYTES; idx++) {
		assert_int_equal(0xa5, comp_pkt.pkt_data[idx]);;
	}

	DAR_util_pkt_bytes_init(&comp_pkt);
	assert_int_equal(0xFFFFFFFF, comp_pkt.num_chars);
	assert_int_equal(rio_addr_34, comp_pkt.pkt_addr_size);
	assert_false(comp_pkt.pkt_has_crc);
	assert_false(comp_pkt.pkt_padded);
	for (idx = 0; idx < RIO_MAX_PKT_BYTES; idx++) {
		assert_int_equal(0, comp_pkt.pkt_data[idx]);;
	}

	memset(&comp_pkt, 0x5a, sizeof(DAR_pkt_bytes_t));
	assert_int_equal(0x5a5a5a5a, comp_pkt.num_chars);
	for (idx = 0; idx < RIO_MAX_PKT_BYTES; idx++) {
		assert_int_equal(0x5a, comp_pkt.pkt_data[idx]);;
	}

	DAR_util_pkt_bytes_init(&comp_pkt);
	assert_int_equal(0xFFFFFFFF, comp_pkt.num_chars);
	assert_int_equal(rio_addr_34, comp_pkt.pkt_addr_size);
	assert_false(comp_pkt.pkt_has_crc);
	assert_false(comp_pkt.pkt_padded);
	for (idx = 0; idx < RIO_MAX_PKT_BYTES; idx++) {
		assert_int_equal(0, comp_pkt.pkt_data[idx]);;
	}

	(void)state; // unused
}

static void tst_init_rw_addr_data(uint8_t *expected, DAR_pkt_bytes_t *bytes_out,
		rio_addr_size pkt_addr_size, uint32_t num_chars_in)
{
	memset(bytes_out, 0, sizeof(DAR_pkt_bytes_t));
	bytes_out->pkt_addr_size = pkt_addr_size;
	bytes_out->num_chars = num_chars_in;
	bytes_out->pkt_has_crc = true;
	memset(bytes_out->pkt_data, 0xa5, num_chars_in);
	memset(expected, 0xa5, num_chars_in);

	switch (pkt_addr_size) {
	case rio_addr_21:
		expected[num_chars_in] = 0xad;
		expected[num_chars_in + 1] = 0xbe;
		expected[num_chars_in + 2] = 0xe8;
		break;
	case rio_addr_32:
		expected[num_chars_in] = 0xde;
		expected[num_chars_in + 1] = 0xad;
		expected[num_chars_in + 2] = 0xbe;
		expected[num_chars_in + 3] = 0xe8;
		break;
	case rio_addr_34:
		expected[num_chars_in] = 0xde;
		expected[num_chars_in + 1] = 0xad;
		expected[num_chars_in + 2] = 0xbe;
		expected[num_chars_in + 3] = 0xea;
		break;
	case rio_addr_50:
		expected[num_chars_in] = 0xba;
		expected[num_chars_in + 1] = 0xbe;
		expected[num_chars_in + 2] = 0xde;
		expected[num_chars_in + 3] = 0xad;
		expected[num_chars_in + 4] = 0xbe;
		expected[num_chars_in + 5] = 0xea;
		break;
	case rio_addr_66:
		expected[num_chars_in] = 0xca;
		expected[num_chars_in + 1] = 0xfe;
		expected[num_chars_in + 2] = 0xba;
		expected[num_chars_in + 3] = 0xbe;
		expected[num_chars_in + 4] = 0xde;
		expected[num_chars_in + 5] = 0xad;
		expected[num_chars_in + 6] = 0xbe;
		expected[num_chars_in + 7] = 0xea;
		break;
	default:
		fail_msg("Invalid %u address size", pkt_addr_size);
	}
}

static void DAR_add_rw_addr_addr_size_21_test(void **state)
{
	const rio_addr_size pkt_addr_size = rio_addr_21;
	const uint32_t num_chars_in = 6;
	const uint32_t num_chars_out = 6 + 3;

	uint32_t addr[3] = {0xdeadbeef, 0xcafebabe, 0xa5a55a5a};
	uint32_t wptr;
	DAR_pkt_bytes_t bytes_out;

	uint32_t rc;
	uint8_t expected[RIO_MAX_PKT_BYTES];

	// valid wptr
	wptr = 0;
	tst_init_rw_addr_data(expected, &bytes_out, pkt_addr_size,
			num_chars_in);

	rc = DAR_add_rw_addr(pkt_addr_size, addr, wptr, &bytes_out);
	assert_int_equal(0, rc);
	assert_int_equal(pkt_addr_size, bytes_out.pkt_addr_size);
	assert_true(bytes_out.pkt_has_crc);
	assert_false(bytes_out.pkt_padded);
	assert_int_equal(num_chars_out, bytes_out.num_chars);
	assert_memory_equal(expected, bytes_out.pkt_data, num_chars_out);

	// valid wptr
	wptr = 1;
	tst_init_rw_addr_data(expected, &bytes_out, pkt_addr_size,
			num_chars_in);
	expected[num_chars_in + 2] = 0xec;

	rc = DAR_add_rw_addr(pkt_addr_size, addr, wptr, &bytes_out);
	assert_int_equal(0, rc);
	assert_int_equal(pkt_addr_size, bytes_out.pkt_addr_size);
	assert_true(bytes_out.pkt_has_crc);
	assert_false(bytes_out.pkt_padded);
	assert_int_equal(num_chars_out, bytes_out.num_chars);
	assert_memory_equal(expected, bytes_out.pkt_data, num_chars_out);

	// invalid wptr
	tst_init_rw_addr_data(expected, &bytes_out, pkt_addr_size,
			num_chars_in);
	for (wptr = 2; wptr < 10; wptr++) {
		rc = DAR_add_rw_addr(pkt_addr_size, addr, wptr, &bytes_out);
		assert_int_equal(DAR_UTIL_INVALID_WPTR, rc);
		assert_int_equal(pkt_addr_size, bytes_out.pkt_addr_size);
		assert_true(bytes_out.pkt_has_crc);
		assert_false(bytes_out.pkt_padded);
		assert_int_equal(num_chars_in, bytes_out.num_chars);
		assert_memory_equal(expected, bytes_out.pkt_data, num_chars_in);
	}

	(void)state; // unused
}

static void DAR_add_rw_addr_addr_size_32_test(void **state)
{
	const rio_addr_size pkt_addr_size = rio_addr_32;
	const uint32_t num_chars_in = 6;
	const uint32_t num_chars_out = 6 + 4;

	uint32_t addr[3] = {0xdeadbeef, 0xcafebabe, 0xa5a55a5a};
	uint32_t wptr;
	DAR_pkt_bytes_t bytes_out;

	uint32_t rc;
	uint8_t expected[RIO_MAX_PKT_BYTES];

	// valid wptr
	for (wptr = 0; wptr < 2; wptr++) {
		tst_init_rw_addr_data(expected, &bytes_out, pkt_addr_size,
				num_chars_in);

		rc = DAR_add_rw_addr(pkt_addr_size, addr, wptr, &bytes_out);
		assert_int_equal(0, rc);
		assert_int_equal(pkt_addr_size, bytes_out.pkt_addr_size);
		assert_true(bytes_out.pkt_has_crc);
		assert_false(bytes_out.pkt_padded);
		assert_int_equal(num_chars_out, bytes_out.num_chars);
		if (0 == wptr) {
			expected[num_chars_out - 1] = 0xe8;
		} else {
			expected[num_chars_out - 1] = 0xec;
		}
		assert_memory_equal(expected, bytes_out.pkt_data,
				num_chars_out);
	}

	// invalid wptr
	tst_init_rw_addr_data(expected, &bytes_out, pkt_addr_size,
			num_chars_in);
	for (wptr = 2; wptr < 10; wptr++) {
		rc = DAR_add_rw_addr(pkt_addr_size, addr, wptr, &bytes_out);
		assert_int_equal(DAR_UTIL_INVALID_WPTR, rc);
		assert_int_equal(pkt_addr_size, bytes_out.pkt_addr_size);
		assert_true(bytes_out.pkt_has_crc);
		assert_false(bytes_out.pkt_padded);
		assert_int_equal(num_chars_in, bytes_out.num_chars);
		assert_memory_equal(expected, bytes_out.pkt_data, num_chars_in);
	}

	(void)state; // unused
}

static void DAR_add_rw_addr_addr_size_34_test(void **state)
{
	const rio_addr_size pkt_addr_size = rio_addr_34;
	const uint32_t num_chars_in = 6;
	const uint32_t num_chars_out = 6 + 4;

	uint32_t addr[3] = {0xdeadbeef, 0xcafebabe, 0xa5a55a5a};
	uint32_t wptr;
	DAR_pkt_bytes_t bytes_out;

	uint32_t rc;
	uint8_t expected[RIO_MAX_PKT_BYTES];

	// valid wptr
	for (wptr = 0; wptr < 2; wptr++) {
		tst_init_rw_addr_data(expected, &bytes_out, pkt_addr_size,
				num_chars_in);

		rc = DAR_add_rw_addr(pkt_addr_size, addr, wptr, &bytes_out);
		assert_int_equal(0, rc);
		assert_int_equal(pkt_addr_size, bytes_out.pkt_addr_size);
		assert_true(bytes_out.pkt_has_crc);
		assert_false(bytes_out.pkt_padded);
		assert_int_equal(num_chars_out, bytes_out.num_chars);
		if (0 == wptr) {
			expected[num_chars_out - 1] = 0xea;
		} else {
			expected[num_chars_out - 1] = 0xee;
		}
		assert_memory_equal(expected, bytes_out.pkt_data,
				num_chars_out);
	}

	// invalid wptr
	tst_init_rw_addr_data(expected, &bytes_out, pkt_addr_size,
			num_chars_in);
	for (wptr = 2; wptr < 10; wptr++) {
		rc = DAR_add_rw_addr(pkt_addr_size, addr, wptr, &bytes_out);
		assert_int_equal(DAR_UTIL_INVALID_WPTR, rc);
		assert_int_equal(pkt_addr_size, bytes_out.pkt_addr_size);
		assert_true(bytes_out.pkt_has_crc);
		assert_false(bytes_out.pkt_padded);
		assert_int_equal(num_chars_in, bytes_out.num_chars);
		assert_memory_equal(expected, bytes_out.pkt_data, num_chars_in);
	}

	(void)state; // unused
}

static void DAR_add_rw_addr_addr_size_50_test(void **state)
{
	const rio_addr_size pkt_addr_size = rio_addr_50;
	const uint32_t num_chars_in = 6;
	const uint32_t num_chars_out = 6 + 6;

	uint32_t addr[3] = {0xdeadbeef, 0xcafebabe, 0xa5a55a5a};
	uint32_t wptr;
	DAR_pkt_bytes_t bytes_out;

	uint32_t rc;
	uint8_t expected[RIO_MAX_PKT_BYTES];

	// valid wptr
	for (wptr = 0; wptr < 2; wptr++) {
		tst_init_rw_addr_data(expected, &bytes_out, pkt_addr_size,
				num_chars_in);

		rc = DAR_add_rw_addr(pkt_addr_size, addr, wptr, &bytes_out);
		assert_int_equal(0, rc);
		assert_int_equal(pkt_addr_size, bytes_out.pkt_addr_size);
		assert_true(bytes_out.pkt_has_crc);
		assert_false(bytes_out.pkt_padded);
		assert_int_equal(num_chars_out, bytes_out.num_chars);
		if (0 == wptr) {
			expected[num_chars_out - 1] = 0xea;
		} else {
			expected[num_chars_out - 1] = 0xee;
		}
		assert_memory_equal(expected, bytes_out.pkt_data,
				num_chars_out);
	}

	// invalid wptr
	tst_init_rw_addr_data(expected, &bytes_out, pkt_addr_size,
			num_chars_in);
	for (wptr = 2; wptr < 10; wptr++) {
		rc = DAR_add_rw_addr(pkt_addr_size, addr, wptr, &bytes_out);
		assert_int_equal(DAR_UTIL_INVALID_WPTR, rc);
		assert_int_equal(pkt_addr_size, bytes_out.pkt_addr_size);
		assert_true(bytes_out.pkt_has_crc);
		assert_false(bytes_out.pkt_padded);
		assert_int_equal(num_chars_in, bytes_out.num_chars);
		assert_memory_equal(expected, bytes_out.pkt_data, num_chars_in);
	}

	(void)state; // unused
}

static void DAR_add_rw_addr_addr_size_66_test(void **state)
{
	const rio_addr_size pkt_addr_size = rio_addr_66;
	const uint32_t num_chars_in = 6;
	const uint32_t num_chars_out = 6 + 8;

	uint32_t addr[3] = {0xdeadbeef, 0xcafebabe, 0xa5a55a5a};
	uint32_t wptr;
	DAR_pkt_bytes_t bytes_out;

	uint32_t rc;
	uint8_t expected[RIO_MAX_PKT_BYTES];

	// valid wptr
	for (wptr = 0; wptr < 2; wptr++) {
		tst_init_rw_addr_data(expected, &bytes_out, pkt_addr_size,
				num_chars_in);

		rc = DAR_add_rw_addr(pkt_addr_size, addr, wptr, &bytes_out);
		assert_int_equal(0, rc);
		assert_int_equal(pkt_addr_size, bytes_out.pkt_addr_size);
		assert_true(bytes_out.pkt_has_crc);
		assert_false(bytes_out.pkt_padded);
		assert_int_equal(num_chars_out, bytes_out.num_chars);
		if (0 == wptr) {
			expected[num_chars_out - 1] = 0xea;
		} else {
			expected[num_chars_out - 1] = 0xee;
		}
		assert_memory_equal(expected, bytes_out.pkt_data,
				num_chars_out);
	}

	// invalid wptr
	tst_init_rw_addr_data(expected, &bytes_out, pkt_addr_size,
			num_chars_in);
	for (wptr = 2; wptr < 10; wptr++) {
		rc = DAR_add_rw_addr(pkt_addr_size, addr, wptr, &bytes_out);
		assert_int_equal(DAR_UTIL_INVALID_WPTR, rc);
		assert_int_equal(pkt_addr_size, bytes_out.pkt_addr_size);
		assert_true(bytes_out.pkt_has_crc);
		assert_false(bytes_out.pkt_padded);
		assert_int_equal(num_chars_in, bytes_out.num_chars);
		assert_memory_equal(expected, bytes_out.pkt_data, num_chars_in);
	}

	(void)state; // unused
}

static void tst_init_get_rw_addr_data(DAR_pkt_fields_t *fields_out,
		DAR_pkt_bytes_t *bytes_in, rio_addr_size pkt_addr_size,
		uint32_t *bidx)
{
	uint32_t idx = 0;

	memset(fields_out, 0, sizeof(DAR_pkt_fields_t));
	fields_out->tot_bytes = 0x123;
	fields_out->pad_bytes = 0x321;
	fields_out->log_rw.pkt_addr_size = rio_addr_66;
	fields_out->log_rw.addr[0] = 0xdeadbeef;
	fields_out->log_rw.addr[1] = 0xcafebabe;
	fields_out->log_rw.addr[2] = 0xa5a55a5a;

	memset(bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in->pkt_addr_size = pkt_addr_size;
	bytes_in->pkt_data[idx++] = 0xde;
	bytes_in->pkt_data[idx++] = 0xad;
	bytes_in->pkt_data[idx++] = 0xbe;
	bytes_in->pkt_data[idx++] = 0xef;
	bytes_in->pkt_data[idx++] = 0xca;
	bytes_in->pkt_data[idx++] = 0xfe;
	bytes_in->pkt_data[idx++] = 0xba;
	bytes_in->pkt_data[idx++] = 0xbe;
	bytes_in->pkt_data[idx++] = 0xa5;
	bytes_in->pkt_data[idx++] = 0x5a;
	bytes_in->pkt_data[idx++] = 0xe7;
	*bidx = idx;
}

static void DAR_get_rw_addr_addr_size_21_test(void **state)
{
	const rio_addr_size pkt_addr_size = rio_addr_21;
	const uint32_t consumed = 3;

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint32_t bidx;
	uint32_t rc;

	tst_init_get_rw_addr_data(&fields_out, &bytes_in, pkt_addr_size, &bidx);
	bidx -= 9;

	assert_int_equal(0xdeadbeef, fields_out.log_rw.addr[0]);
	assert_int_equal(0xcafebabe, fields_out.log_rw.addr[1]);
	assert_int_equal(0xa5a55a5a, fields_out.log_rw.addr[2]);

	rc = DAR_get_rw_addr(&bytes_in, &fields_out, bidx);
	assert_int_equal(bidx + consumed, rc);
	assert_int_equal(0xbeefc8, fields_out.log_rw.addr[0]);
	assert_int_equal(0, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);
	assert_int_equal(0x123, fields_out.tot_bytes);
	assert_int_equal(0x321, fields_out.pad_bytes);
	assert_int_equal(pkt_addr_size, fields_out.log_rw.pkt_addr_size);

	// again
	tst_init_get_rw_addr_data(&fields_out, &bytes_in, pkt_addr_size, &bidx);
	bidx -= 8;

	assert_int_equal(0xdeadbeef, fields_out.log_rw.addr[0]);
	assert_int_equal(0xcafebabe, fields_out.log_rw.addr[1]);
	assert_int_equal(0xa5a55a5a, fields_out.log_rw.addr[2]);

	rc = DAR_get_rw_addr(&bytes_in, &fields_out, bidx);
	assert_int_equal(bidx + consumed, rc);
	assert_int_equal(0xefcaf8, fields_out.log_rw.addr[0]);
	assert_int_equal(0, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);
	assert_int_equal(0x123, fields_out.tot_bytes);
	assert_int_equal(0x321, fields_out.pad_bytes);
	assert_int_equal(pkt_addr_size, fields_out.log_rw.pkt_addr_size);

	(void)state; // unused
}

static void DAR_get_rw_addr_addr_size_32_test(void **state)
{
	const rio_addr_size pkt_addr_size = rio_addr_32;
	const uint32_t consumed = 4;

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint32_t bidx;
	uint32_t rc;

	tst_init_get_rw_addr_data(&fields_out, &bytes_in, pkt_addr_size, &bidx);
	bidx -= 9;

	assert_int_equal(0xdeadbeef, fields_out.log_rw.addr[0]);
	assert_int_equal(0xcafebabe, fields_out.log_rw.addr[1]);
	assert_int_equal(0xa5a55a5a, fields_out.log_rw.addr[2]);

	rc = DAR_get_rw_addr(&bytes_in, &fields_out, bidx);
	assert_int_equal(bidx + consumed, rc);
	assert_int_equal(0xbeefcaf8, fields_out.log_rw.addr[0]);
	assert_int_equal(0, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);
	assert_int_equal(0x123, fields_out.tot_bytes);
	assert_int_equal(0x321, fields_out.pad_bytes);
	assert_int_equal(pkt_addr_size, fields_out.log_rw.pkt_addr_size);

	// again
	tst_init_get_rw_addr_data(&fields_out, &bytes_in, pkt_addr_size, &bidx);
	bidx -= 8;

	assert_int_equal(0xdeadbeef, fields_out.log_rw.addr[0]);
	assert_int_equal(0xcafebabe, fields_out.log_rw.addr[1]);
	assert_int_equal(0xa5a55a5a, fields_out.log_rw.addr[2]);

	rc = DAR_get_rw_addr(&bytes_in, &fields_out, bidx);
	assert_int_equal(bidx + consumed, rc);
	assert_int_equal(0xefcafeb8, fields_out.log_rw.addr[0]);
	assert_int_equal(0, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);
	assert_int_equal(0x123, fields_out.tot_bytes);
	assert_int_equal(0x321, fields_out.pad_bytes);
	assert_int_equal(pkt_addr_size, fields_out.log_rw.pkt_addr_size);

	(void)state; // unused
}

static void DAR_get_rw_addr_addr_size_34_test(void **state)
{
	const rio_addr_size pkt_addr_size = rio_addr_34;
	const uint32_t consumed = 4;

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint32_t bidx;
	uint32_t rc;

	tst_init_get_rw_addr_data(&fields_out, &bytes_in, pkt_addr_size, &bidx);
	bidx -= 9;

	assert_int_equal(0xdeadbeef, fields_out.log_rw.addr[0]);
	assert_int_equal(0xcafebabe, fields_out.log_rw.addr[1]);
	assert_int_equal(0xa5a55a5a, fields_out.log_rw.addr[2]);

	rc = DAR_get_rw_addr(&bytes_in, &fields_out, bidx);
	assert_int_equal(bidx + consumed, rc);
	assert_int_equal(0xbeefcaf8, fields_out.log_rw.addr[0]);
	assert_int_equal(2, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);
	assert_int_equal(0x123, fields_out.tot_bytes);
	assert_int_equal(0x321, fields_out.pad_bytes);
	assert_int_equal(pkt_addr_size, fields_out.log_rw.pkt_addr_size);

	// again
	tst_init_get_rw_addr_data(&fields_out, &bytes_in, pkt_addr_size, &bidx);
	bidx -= 8;

	assert_int_equal(0xdeadbeef, fields_out.log_rw.addr[0]);
	assert_int_equal(0xcafebabe, fields_out.log_rw.addr[1]);
	assert_int_equal(0xa5a55a5a, fields_out.log_rw.addr[2]);

	rc = DAR_get_rw_addr(&bytes_in, &fields_out, bidx);
	assert_int_equal(bidx + consumed, rc);
	assert_int_equal(0xefcafeb8, fields_out.log_rw.addr[0]);
	assert_int_equal(2, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);
	assert_int_equal(0x123, fields_out.tot_bytes);
	assert_int_equal(0x321, fields_out.pad_bytes);
	assert_int_equal(pkt_addr_size, fields_out.log_rw.pkt_addr_size);

	(void)state; // unused
}

static void DAR_get_rw_addr_addr_size_50_test(void **state)
{
	const rio_addr_size pkt_addr_size = rio_addr_50;
	const uint32_t consumed = 6;

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint32_t bidx;
	uint32_t rc;

	tst_init_get_rw_addr_data(&fields_out, &bytes_in, pkt_addr_size, &bidx);
	bidx -= 9;

	assert_int_equal(0xdeadbeef, fields_out.log_rw.addr[0]);
	assert_int_equal(0xcafebabe, fields_out.log_rw.addr[1]);
	assert_int_equal(0xa5a55a5a, fields_out.log_rw.addr[2]);

	rc = DAR_get_rw_addr(&bytes_in, &fields_out, bidx);
	assert_int_equal(bidx + consumed, rc);
	assert_int_equal(0xcafebab8, fields_out.log_rw.addr[0]);
	assert_int_equal(0x2beef, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);
	assert_int_equal(0x123, fields_out.tot_bytes);
	assert_int_equal(0x321, fields_out.pad_bytes);
	assert_int_equal(pkt_addr_size, fields_out.log_rw.pkt_addr_size);

	// again
	tst_init_get_rw_addr_data(&fields_out, &bytes_in, pkt_addr_size, &bidx);
	bidx -= 8;

	assert_int_equal(0xdeadbeef, fields_out.log_rw.addr[0]);
	assert_int_equal(0xcafebabe, fields_out.log_rw.addr[1]);
	assert_int_equal(0xa5a55a5a, fields_out.log_rw.addr[2]);

	rc = DAR_get_rw_addr(&bytes_in, &fields_out, bidx);
	assert_int_equal(bidx + consumed, rc);
	assert_int_equal(0xfebabea0, fields_out.log_rw.addr[0]);
	assert_int_equal(0x1efca, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);
	assert_int_equal(0x123, fields_out.tot_bytes);
	assert_int_equal(0x321, fields_out.pad_bytes);
	assert_int_equal(pkt_addr_size, fields_out.log_rw.pkt_addr_size);

	(void)state; // unused
}

static void DAR_get_rw_addr_addr_size_66_test(void **state)
{
	const rio_addr_size pkt_addr_size = rio_addr_66;
	const uint32_t consumed = 8;

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint32_t bidx;
	uint32_t rc;

	tst_init_get_rw_addr_data(&fields_out, &bytes_in, pkt_addr_size, &bidx);
	bidx -= 9;
	fields_out.log_rw.pkt_addr_size = rio_addr_21;

	assert_int_equal(0xdeadbeef, fields_out.log_rw.addr[0]);
	assert_int_equal(0xcafebabe, fields_out.log_rw.addr[1]);
	assert_int_equal(0xa5a55a5a, fields_out.log_rw.addr[2]);

	rc = DAR_get_rw_addr(&bytes_in, &fields_out, bidx);
	assert_int_equal(bidx + consumed, rc);
	assert_int_equal(0xbabea558, fields_out.log_rw.addr[0]);
	assert_int_equal(0xbeefcafe, fields_out.log_rw.addr[1]);
	assert_int_equal(2, fields_out.log_rw.addr[2]);
	assert_int_equal(0x123, fields_out.tot_bytes);
	assert_int_equal(0x321, fields_out.pad_bytes);
	assert_int_equal(pkt_addr_size, fields_out.log_rw.pkt_addr_size);

	// again
	tst_init_get_rw_addr_data(&fields_out, &bytes_in, pkt_addr_size, &bidx);
	bidx -= 8;
	fields_out.log_rw.pkt_addr_size = rio_addr_21;

	assert_int_equal(0xdeadbeef, fields_out.log_rw.addr[0]);
	assert_int_equal(0xcafebabe, fields_out.log_rw.addr[1]);
	assert_int_equal(0xa5a55a5a, fields_out.log_rw.addr[2]);

	rc = DAR_get_rw_addr(&bytes_in, &fields_out, bidx);
	assert_int_equal(bidx + consumed, rc);
	assert_int_equal(0xbea55ae0, fields_out.log_rw.addr[0]);
	assert_int_equal(0xefcafeba, fields_out.log_rw.addr[1]);
	assert_int_equal(3, fields_out.log_rw.addr[2]);
	assert_int_equal(0x123, fields_out.tot_bytes);
	assert_int_equal(0x321, fields_out.pad_bytes);
	assert_int_equal(pkt_addr_size, fields_out.log_rw.pkt_addr_size);

	(void)state; // unused
}

static void DAR_addr_size_addr_size_21_roundtrip_test(void **state)
{
	// assuming that the DAR_get_rw_addr_addr_size_XX_test and
	// DAR_add_rw_addr_addr_size_XX_test all pass, then a round
	// trip should also logically pass
	const uint8_t input[] = {0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe};
	const rio_addr_size pkt_addr_size = rio_addr_21;
	const uint32_t ret_idx = 3;

	DAR_pkt_bytes_t bytes_out;
	DAR_pkt_fields_t fields_out;
	DAR_pkt_bytes_t bytes_in;
	uint32_t idx;
	uint32_t lsb;
	uint32_t rc;

	// wptr = 0
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	bytes_in.pkt_addr_size = pkt_addr_size;

	idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
	assert_int_equal(ret_idx, idx);
	assert_int_equal(0xdeadb8, fields_out.log_rw.addr[0]);
	assert_int_equal(0, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);

	rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 0,
			&bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(0xde, bytes_in.pkt_data[0]);
	assert_int_equal(0xad, bytes_in.pkt_data[1]);
	assert_int_equal(0xbe, bytes_in.pkt_data[2]);
	bytes_in.pkt_data[2] = 0xb8;
	assert_memory_equal(&bytes_in.pkt_data, &bytes_out.pkt_data, ret_idx);

	// wptr = 1
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	bytes_in.pkt_addr_size = pkt_addr_size;

	idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
	assert_int_equal(ret_idx, idx);
	assert_int_equal(0xdeadb8, fields_out.log_rw.addr[0]);
	assert_int_equal(0, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);

	rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 1,
			&bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(0xde, bytes_in.pkt_data[0]);
	assert_int_equal(0xad, bytes_in.pkt_data[1]);
	assert_int_equal(0xbe, bytes_in.pkt_data[2]);
	bytes_in.pkt_data[2] = 0xbc;
	assert_memory_equal(&bytes_in.pkt_data, &bytes_out.pkt_data, ret_idx);

	// alignment
	for (lsb = 0; lsb < 0x10; lsb++) {
		// wptr = 0
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
		memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
		memcpy(&bytes_in.pkt_data, &input, sizeof(input));
		bytes_in.pkt_addr_size = pkt_addr_size;
		bytes_in.pkt_data[2] = 0xb0 + lsb;

		idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
		assert_int_equal(ret_idx, idx);
		if (lsb < 8) {
			assert_int_equal(0xdeadb0, fields_out.log_rw.addr[0]);
		} else {
			assert_int_equal(0xdeadb8, fields_out.log_rw.addr[0]);
		}
		assert_int_equal(0, fields_out.log_rw.addr[1]);
		assert_int_equal(0, fields_out.log_rw.addr[2]);

		rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 0,
				&bytes_out);
		assert_int_equal(RIO_SUCCESS, rc);
		assert_int_equal(0xde, bytes_in.pkt_data[0]);
		assert_int_equal(0xad, bytes_in.pkt_data[1]);
		assert_int_equal(0xb0 + lsb, bytes_in.pkt_data[2]);
		if (lsb < 8) {
			bytes_in.pkt_data[2] = 0xb0;
		} else {
			bytes_in.pkt_data[2] = 0xb8;
		}
		assert_memory_equal(&bytes_in.pkt_data, &bytes_out.pkt_data,
				ret_idx);

		// wptr = 1
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
		memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
		memcpy(&bytes_in.pkt_data, &input, sizeof(input));
		bytes_in.pkt_addr_size = pkt_addr_size;
		bytes_in.pkt_data[2] = 0xb0 + lsb;

		idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
		assert_int_equal(ret_idx, idx);
		if (lsb < 8) {
			assert_int_equal(0xdeadb0, fields_out.log_rw.addr[0]);
		} else {
			assert_int_equal(0xdeadb8, fields_out.log_rw.addr[0]);
		}
		assert_int_equal(0, fields_out.log_rw.addr[1]);
		assert_int_equal(0, fields_out.log_rw.addr[2]);

		rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 1,
				&bytes_out);
		assert_int_equal(RIO_SUCCESS, rc);
		assert_int_equal(0xde, bytes_in.pkt_data[0]);
		assert_int_equal(0xad, bytes_in.pkt_data[1]);
		assert_int_equal(0xb0 + lsb, bytes_in.pkt_data[2]);
		if (lsb < 8) {
			bytes_in.pkt_data[2] = 0xb4;
		} else {
			bytes_in.pkt_data[2] = 0xbc;
		}
		assert_memory_equal(&bytes_in.pkt_data, &bytes_out.pkt_data,
				ret_idx);
	}

	(void)state; // unused
}

static void DAR_addr_size_addr_size_32_roundtrip_test(void **state)
{
	// assuming that the DAR_get_rw_addr_addr_size_XX_test and
	// DAR_add_rw_addr_addr_size_XX_test all pass, then a round
	// trip should also logically pass
	const uint8_t input[] = {0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe};
	const rio_addr_size pkt_addr_size = rio_addr_32;
	const uint32_t ret_idx = 4;

	DAR_pkt_bytes_t bytes_out;
	DAR_pkt_fields_t fields_out;
	DAR_pkt_bytes_t bytes_in;
	uint32_t idx;
	uint32_t lsb;
	uint32_t rc;

	// wptr = 0
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	bytes_in.pkt_addr_size = pkt_addr_size;

	idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
	assert_int_equal(ret_idx, idx);
	assert_int_equal(0xdeadbee8, fields_out.log_rw.addr[0]);
	assert_int_equal(0, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);

	rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 0,
			&bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	bytes_in.pkt_data[3] = 0xe8;
	assert_memory_equal(&bytes_in.pkt_data, &bytes_out.pkt_data, ret_idx);

	// wptr = 1
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	bytes_in.pkt_addr_size = pkt_addr_size;

	idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
	assert_int_equal(ret_idx, idx);
	assert_int_equal(0xdeadbee8, fields_out.log_rw.addr[0]);
	assert_int_equal(0, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);

	rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 1,
			&bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	bytes_in.pkt_data[3] = 0xec;
	assert_memory_equal(&bytes_in.pkt_data, &bytes_out.pkt_data, ret_idx);

	// alignment
	for (lsb = 0; lsb < 0x10; lsb++) {
		// wptr = 0
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
		memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
		memcpy(&bytes_in.pkt_data, &input, sizeof(input));
		bytes_in.pkt_addr_size = pkt_addr_size;
		bytes_in.pkt_data[0] = 0xd0 + lsb;

		idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
		assert_int_equal(ret_idx, idx);
		assert_int_equal(((0xd0 | lsb) << 24) | 0xadbee8,
				fields_out.log_rw.addr[0]);
		assert_int_equal(0, fields_out.log_rw.addr[1]);
		assert_int_equal(0, fields_out.log_rw.addr[2]);

		rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 0,
				&bytes_out);
		assert_int_equal(RIO_SUCCESS, rc);
		assert_int_equal(0xd0 | lsb, bytes_in.pkt_data[0]);
		bytes_in.pkt_data[3] = 0xe8;
		assert_memory_equal(&bytes_in.pkt_data, &bytes_out.pkt_data,
				ret_idx);

		// wptr = 1
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
		memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
		memcpy(&bytes_in.pkt_data, &input, sizeof(input));
		bytes_in.pkt_addr_size = pkt_addr_size;
		bytes_in.pkt_data[0] = 0xd0 + lsb;

		idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
		assert_int_equal(ret_idx, idx);
		assert_int_equal(((0xd0 | lsb) << 24) | 0xadbee8,
				fields_out.log_rw.addr[0]);
		assert_int_equal(0, fields_out.log_rw.addr[1]);
		assert_int_equal(0, fields_out.log_rw.addr[2]);

		rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 1,
				&bytes_out);
		assert_int_equal(RIO_SUCCESS, rc);
		assert_int_equal(0xd0 | lsb, bytes_in.pkt_data[0]);
		bytes_in.pkt_data[3] = 0xec;
		assert_memory_equal(&bytes_in.pkt_data, &bytes_out.pkt_data,
				ret_idx);
	}

	(void)state; // unused
}

static void DAR_addr_size_addr_size_34_roundtrip_test(void **state)
{
	// assuming that the DAR_get_rw_addr_addr_size_XX_test and
	// DAR_add_rw_addr_addr_size_XX_test all pass, then a round
	// trip should also logically pass
	const uint8_t input[] = {0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe};
	const rio_addr_size pkt_addr_size = rio_addr_34;
	const uint32_t ret_idx = 4;

	DAR_pkt_bytes_t bytes_out;
	DAR_pkt_fields_t fields_out;
	DAR_pkt_bytes_t bytes_in;
	uint8_t output[sizeof(input)];
	uint32_t idx;
	uint32_t lsb;
	uint32_t rc;

	// wptr = 0
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	memcpy(&output, &input, sizeof(output));
	bytes_in.pkt_addr_size = pkt_addr_size;
	output[ret_idx - 1] = 0xeb;

	idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
	assert_int_equal(ret_idx, idx);
	assert_int_equal(0xdeadbee8, fields_out.log_rw.addr[0]);
	assert_int_equal(3, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);

	rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 0,
			&bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&input, &bytes_in.pkt_data, sizeof(input));
	assert_memory_equal(&output, &bytes_out.pkt_data, ret_idx);

	// wptr = 1
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	memcpy(&output, &input, sizeof(output));
	bytes_in.pkt_addr_size = pkt_addr_size;

	idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
	assert_int_equal(ret_idx, idx);
	assert_int_equal(0xdeadbee8, fields_out.log_rw.addr[0]);
	assert_int_equal(3, fields_out.log_rw.addr[1]);
	assert_int_equal(0, fields_out.log_rw.addr[2]);

	rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 1,
			&bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&input, &bytes_in.pkt_data, sizeof(input));
	assert_memory_equal(&output, &bytes_out.pkt_data, ret_idx);

	// alignment
	for (lsb = 0; lsb < 0x10; lsb++) {
		// wptr = 0
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
		memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
		memcpy(&bytes_in.pkt_data, &input, sizeof(input));
		bytes_in.pkt_addr_size = pkt_addr_size;
		bytes_in.pkt_data[ret_idx - 1] = 0xd0 + lsb;

		idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
		assert_int_equal(ret_idx, idx);
		if (lsb < 8) {
			assert_int_equal(0xdeadbed0, fields_out.log_rw.addr[0]);
		} else {
			assert_int_equal(0xdeadbed8, fields_out.log_rw.addr[0]);
		}
		assert_int_equal(lsb % 4, fields_out.log_rw.addr[1]);
		assert_int_equal(0, fields_out.log_rw.addr[2]);

		rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 0,
				&bytes_out);
		assert_int_equal(RIO_SUCCESS, rc);
		memcpy(&output, &input, sizeof(output));
		output[ret_idx - 1] = 0xd0 | lsb;
		assert_memory_equal(&output, &bytes_in.pkt_data, sizeof(input));
		if (lsb < 8) {
			output[ret_idx - 1] = 0xd0 | (lsb % 4);
		} else {
			output[ret_idx - 1] = 0xd0 | ((lsb % 4) + 8);
		}
		assert_memory_equal(&output, &bytes_out.pkt_data, ret_idx);

		// wptr = 1
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
		memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
		memcpy(&bytes_in.pkt_data, &input, sizeof(input));
		bytes_in.pkt_addr_size = pkt_addr_size;
		bytes_in.pkt_data[ret_idx - 1] = 0xd0 + lsb;

		idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
		assert_int_equal(ret_idx, idx);
		if (lsb < 8) {
			assert_int_equal(0xdeadbed0, fields_out.log_rw.addr[0]);
		} else {
			assert_int_equal(0xdeadbed8, fields_out.log_rw.addr[0]);
		}
		assert_int_equal(lsb % 4, fields_out.log_rw.addr[1]);
		assert_int_equal(0, fields_out.log_rw.addr[2]);

		rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 1,
				&bytes_out);
		assert_int_equal(RIO_SUCCESS, rc);
		memcpy(&output, &input, sizeof(output));
		output[ret_idx - 1] = 0xd0 | lsb;
		assert_memory_equal(&output, &bytes_in.pkt_data, sizeof(input));
		if (lsb < 8) {
			output[ret_idx - 1] = 0xd0 | ((lsb % 4) + 4);
		} else {
			output[ret_idx - 1] = 0xd0 | ((lsb % 4) + 12);
		}
		assert_memory_equal(&output, &bytes_out.pkt_data, ret_idx);
	}

	(void)state; // unused
}

static void DAR_addr_size_addr_size_50_roundtrip_test(void **state)
{
	// assuming that the DAR_get_rw_addr_addr_size_XX_test and
	// DAR_add_rw_addr_addr_size_XX_test all pass, then a round
	// trip should also logically pass
	const uint8_t input[] = {0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe};
	const rio_addr_size pkt_addr_size = rio_addr_50;
	const uint32_t ret_idx = 6;

	DAR_pkt_bytes_t bytes_out;
	DAR_pkt_fields_t fields_out;
	DAR_pkt_bytes_t bytes_in;
	uint8_t output[sizeof(input)];
	uint32_t idx;
	uint32_t lsb;
	uint32_t rc;

	// wptr = 0
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	memcpy(&output, &input, sizeof(input));
	bytes_in.pkt_addr_size = pkt_addr_size;
	output[ret_idx - 1] = 0xfa;

	idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
	assert_int_equal(ret_idx, idx);
	rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 0,
			&bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&output, &bytes_out.pkt_data, ret_idx);

	// wptr = 1
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	memcpy(&output, &input, sizeof(input));
	bytes_in.pkt_addr_size = pkt_addr_size;

	idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
	assert_int_equal(ret_idx, idx);
	rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 1,
			&bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&output, &bytes_out.pkt_data, ret_idx);

	// alignment
	for (lsb = 0; lsb < 0x10; lsb++) {
		// wptr = 0
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
		memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
		memcpy(&bytes_in.pkt_data, &input, sizeof(input));
		memcpy(&output, &input, sizeof(input));
		bytes_in.pkt_addr_size = pkt_addr_size;
		bytes_in.pkt_data[ret_idx - 1] = 0xf0 + lsb;
		if (lsb < 8) {
			output[ret_idx - 1] = 0xf0 | (lsb % 4);
		} else {
			output[ret_idx - 1] = 0xf0 | ((lsb % 4) + 8);
		}

		idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
		assert_int_equal(ret_idx, idx);
		rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 0,
				&bytes_out);
		assert_int_equal(RIO_SUCCESS, rc);
		assert_memory_equal(&output, &bytes_out.pkt_data, ret_idx);

		// wptr = 1
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
		memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
		memcpy(&bytes_in.pkt_data, &input, sizeof(input));
		memcpy(&output, &input, sizeof(input));
		bytes_in.pkt_addr_size = pkt_addr_size;
		bytes_in.pkt_data[ret_idx - 1] = 0xf0 + lsb;
		if (lsb < 8) {
			output[ret_idx - 1] = 0xf0 | ((lsb % 4) + 4);
		} else {
			output[ret_idx - 1] = 0xf0 | ((lsb % 4) + 12);
		}

		idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
		assert_int_equal(ret_idx, idx);
		rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 1,
				&bytes_out);
		assert_int_equal(RIO_SUCCESS, rc);
		assert_memory_equal(&output, &bytes_out.pkt_data, ret_idx);
	}

	(void)state; // unused
}

static void DAR_addr_size_addr_size_66_roundtrip_test(void **state)
{
	// assuming that the DAR_get_rw_addr_addr_size_XX_test and
	// DAR_add_rw_addr_addr_size_XX_test all pass, then a round
	// trip should also logically pass
	const uint8_t input[] = {0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe};
	const rio_addr_size pkt_addr_size = rio_addr_66;
	const uint32_t ret_idx = 8;

	DAR_pkt_bytes_t bytes_out;
	DAR_pkt_fields_t fields_out;
	DAR_pkt_bytes_t bytes_in;
	uint8_t output[sizeof(input)];
	uint32_t idx;
	uint32_t lsb;
	uint32_t rc;

	// wptr = 0
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	memcpy(&output, &input, sizeof(input));
	bytes_in.pkt_addr_size = pkt_addr_size;
	output[ret_idx - 1] = 0xba;

	idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
	assert_int_equal(ret_idx, idx);
	rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 0,
			&bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&output, &bytes_out.pkt_data, ret_idx);

	// wptr = 1
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	memcpy(&output, &input, sizeof(input));
	bytes_in.pkt_addr_size = pkt_addr_size;

	idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
	assert_int_equal(ret_idx, idx);
	rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 1,
			&bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&output, &bytes_out.pkt_data, ret_idx);

	// alignment
	for (lsb = 0; lsb < 0x10; lsb++) {
		// wptr = 0
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
		memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
		memcpy(&bytes_in.pkt_data, &input, sizeof(input));
		memcpy(&output, &input, sizeof(input));
		bytes_in.pkt_addr_size = pkt_addr_size;
		bytes_in.pkt_data[ret_idx - 1] = 0xb0 + lsb;
		if (lsb < 8) {
			output[ret_idx - 1] = 0xb0 | (lsb % 4);
		} else {
			output[ret_idx - 1] = 0xb0 | ((lsb % 4) + 8);
		}

		idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
		assert_int_equal(ret_idx, idx);
		rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 0,
				&bytes_out);
		assert_int_equal(RIO_SUCCESS, rc);
		assert_memory_equal(&output, &bytes_out.pkt_data, ret_idx);

		// wptr = 1
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));
		memset(&bytes_in, 0, sizeof(DAR_pkt_fields_t));
		memcpy(&bytes_in.pkt_data, &input, sizeof(input));
		memcpy(&output, &input, sizeof(input));
		bytes_in.pkt_addr_size = pkt_addr_size;
		bytes_in.pkt_data[ret_idx - 1] = 0xb0 + lsb;
		if (lsb < 8) {
			output[ret_idx - 1] = 0xb0 | ((lsb % 4) + 4);
		} else {
			output[ret_idx - 1] = 0xb0 | ((lsb % 4) + 12);
		}

		idx = DAR_get_rw_addr(&bytes_in, &fields_out, 0);
		assert_int_equal(ret_idx, idx);
		rc = DAR_add_rw_addr(pkt_addr_size, fields_out.log_rw.addr, 1,
				&bytes_out);
		assert_int_equal(RIO_SUCCESS, rc);
		assert_memory_equal(&output, &bytes_out.pkt_data, ret_idx);
	}

	(void)state; // unused
}

static void count_bits_test(void **state)
{
	assert_int_equal(0, count_bits(0x0));
	assert_int_equal(1, count_bits(0x1));
	assert_int_equal(1, count_bits(0x2));
	assert_int_equal(2, count_bits(0x3));
	assert_int_equal(1, count_bits(0x4));
	assert_int_equal(2, count_bits(0x5));
	assert_int_equal(2, count_bits(0x6));
	assert_int_equal(3, count_bits(0x7));
	assert_int_equal(1, count_bits(0x8));
	assert_int_equal(2, count_bits(0x9));
	assert_int_equal(2, count_bits(0xa));
	assert_int_equal(3, count_bits(0xb));
	assert_int_equal(2, count_bits(0xc));
	assert_int_equal(3, count_bits(0xd));
	assert_int_equal(3, count_bits(0xe));
	assert_int_equal(4, count_bits(0xf));

	assert_int_equal(4, count_bits(0xf0));
	assert_int_equal(5, count_bits(0xf1));
	assert_int_equal(5, count_bits(0xf2));
	assert_int_equal(6, count_bits(0xf3));
	assert_int_equal(5, count_bits(0xf4));
	assert_int_equal(6, count_bits(0xf5));
	assert_int_equal(6, count_bits(0xf6));
	assert_int_equal(7, count_bits(0xf7));
	assert_int_equal(5, count_bits(0xf8));
	assert_int_equal(6, count_bits(0xf9));
	assert_int_equal(6, count_bits(0xfa));
	assert_int_equal(7, count_bits(0xfb));
	assert_int_equal(6, count_bits(0xfc));
	assert_int_equal(7, count_bits(0xfd));
	assert_int_equal(7, count_bits(0xfe));
	assert_int_equal(8, count_bits(0xff));

	// Only 16 bits count
	assert_int_equal(13, count_bits(0xbeef));
	assert_int_equal(13, count_bits(0xfeeb));
	assert_int_equal(13, count_bits(0xebef));
	assert_int_equal(13, count_bits(0xdeadbeef));

	assert_int_equal(11, count_bits(0xcafe));
	assert_int_equal(11, count_bits(0xacfe));
	assert_int_equal(11, count_bits(0xefca));

	(void)state; // unused
}


int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
	cmocka_unit_test(assumptions),
	cmocka_unit_test(DAR_util_get_ftype_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_1_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_2_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_3_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_4_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_5_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_6_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_7_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_8_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_16_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_32_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_64_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_96_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_128_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_160_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_192_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_224_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_pkt_bytes_256_test),
	cmocka_unit_test(DAR_util_get_rdsize_wdptr_test),
	cmocka_unit_test(DAR_util_compute_rd_bytes_n_align_wptr_0_test),
	cmocka_unit_test(DAR_util_compute_rd_bytes_n_align_wptr_1_test),
	cmocka_unit_test(DAR_util_compute_rd_bytes_n_align_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_0_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_1_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_2_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_3_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_4_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_5_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_6_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_7_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_8_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_9_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_10_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_11_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_12_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_13_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_14_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_15_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_pkt_bytes_16_test),
	cmocka_unit_test(DAR_util_get_wrsize_wdptr_test),
	cmocka_unit_test(DAR_util_compute_wr_bytes_n_align_wptr_0_test),
	cmocka_unit_test(DAR_util_compute_wr_bytes_n_align_wptr_1_test),
	cmocka_unit_test(DAR_util_compute_wr_bytes_n_align_test),
	cmocka_unit_test(DAR_util_pkt_bytes_init_test),
	cmocka_unit_test(DAR_add_rw_addr_addr_size_21_test),
	cmocka_unit_test(DAR_add_rw_addr_addr_size_32_test),
	cmocka_unit_test(DAR_add_rw_addr_addr_size_34_test),
	cmocka_unit_test(DAR_add_rw_addr_addr_size_50_test),
	cmocka_unit_test(DAR_add_rw_addr_addr_size_66_test),
	cmocka_unit_test(DAR_get_rw_addr_addr_size_21_test),
	cmocka_unit_test(DAR_get_rw_addr_addr_size_32_test),
	cmocka_unit_test(DAR_get_rw_addr_addr_size_34_test),
	cmocka_unit_test(DAR_get_rw_addr_addr_size_50_test),
	cmocka_unit_test(DAR_get_rw_addr_addr_size_66_test),
	cmocka_unit_test(DAR_addr_size_addr_size_21_roundtrip_test),
	cmocka_unit_test(DAR_addr_size_addr_size_32_roundtrip_test),
	cmocka_unit_test(DAR_addr_size_addr_size_34_roundtrip_test),
	cmocka_unit_test(DAR_addr_size_addr_size_50_roundtrip_test),
	cmocka_unit_test(DAR_addr_size_addr_size_66_roundtrip_test),
	cmocka_unit_test(count_bits_test),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#ifdef __cplusplus
}
#endif
