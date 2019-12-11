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

static uint32_t zero_memory[RIO_MAX_PKT_BYTES] = {0};

static void assumptions(void **state)
{
	assert_int_equal(276, RIO_MAX_PKT_BYTES);
	assert_int_equal(256, RIO_MAX_PKT_PAYLOAD);

	assert_int_equal(0, stype0_min);
	assert_int_equal(0, stype0_pa);
	assert_int_equal(1, stype0_rty);
	assert_int_equal(2, stype0_pna);
	assert_int_equal(3, stype0_rsvd);
	assert_int_equal(4, stype0_status);
	assert_int_equal(5, stype0_VC_status);
	assert_int_equal(6, stype0_lresp);
	assert_int_equal(7, stype0_imp);
	assert_int_equal(7, stype0_max);

	assert_int_equal(1, PNA_PKT_UNEXP_ACKID);
	assert_int_equal(2, PNA_CS_UNEXP_ACKID);
	assert_int_equal(3, PNA_NONMTC_STOPPED);
	assert_int_equal(4, PNA_PKT_BAD_CRC);
	assert_int_equal(5, PNA_DELIN_ERR);
	assert_int_equal(6, PNA_RETRY);
	assert_int_equal(7, PNA_NO_DESCRAM_SYNC);
	assert_int_equal(0x1F, PNA_GENERAL_ERR);

	assert_int_equal(0, stype1_min);
	assert_int_equal(0, stype1_sop);
	assert_int_equal(1, stype1_stomp);
	assert_int_equal(2, stype1_eop);
	assert_int_equal(3, stype1_rfr);
	assert_int_equal(4, stype1_lreq);
	assert_int_equal(5, stype1_mecs);
	assert_int_equal(6, stype1_rsvd);
	assert_int_equal(7, stype1_nop);
	assert_int_equal(7, stype1_max);

	assert_int_equal(0, STYPE1_CMD_RSVD);
	assert_int_equal(3, STYPE1_LREQ_CMD_RST_DEV);
	assert_int_equal(4, STYPE1_LREQ_CMD_PORT_STAT);

	assert_int_equal(0, stype2_min);
	assert_int_equal(0, stype2_nop);
	assert_int_equal(1, stype2_vc_stat);
	assert_int_equal(2, stype2_rsvd2);
	assert_int_equal(3, stype2_rsvd3);
	assert_int_equal(4, stype2_rsvd4);
	assert_int_equal(5, stype2_rsvd5);
	assert_int_equal(6, stype2_rsvd6);
	assert_int_equal(7, stype2_rsvd7);
	assert_int_equal(7, stype2_max);

	assert_int_equal(0, rio_pkt_status_min);
	assert_int_equal(0, pkt_done);
	assert_int_equal(3, pkt_retry);
	assert_int_equal(7, pkt_err);
	assert_int_equal(7, rio_pkt_status_max);

	assert_int_equal(0, tt_small);
	assert_int_equal(1, tt_large);
	assert_int_equal(2, undef2);
	assert_int_equal(3, undef3);
	assert_int_equal(0, rio_TT_code_min);
	assert_int_equal(3, rio_TT_code_max);

	assert_int_equal(0x78000000, DAR_UTIL_INVALID_TT);
	assert_int_equal(0x78000001, DAR_UTIL_BAD_ADDRSIZE);
	assert_int_equal(0x78000002, DAR_UTIL_INVALID_RDSIZE);
	assert_int_equal(0x78000003, DAR_UTIL_BAD_DATA_SIZE);
	assert_int_equal(0x78000004, DAR_UTIL_INVALID_MTC);
	assert_int_equal(0x78000005, DAR_UTIL_BAD_MSG_DSIZE);
	assert_int_equal(0x78000006, DAR_UTIL_BAD_RESP_DSIZE);
	assert_int_equal(0x78000007, DAR_UTIL_UNKNOWN_TRANS);
	assert_int_equal(0x78000008, DAR_UTIL_BAD_DS_DSIZE);
	assert_int_equal(0x78000009, DAR_UTIL_UNKNOWN_STATUS);
	assert_int_equal(0x78000010, DAR_UTIL_UNKNOWN_FTYPE);
	assert_int_equal(0x78000011, DAR_UTIL_0_MASK_VAL_ERR);
	assert_int_equal(0x78000012, DAR_UTIL_INVALID_WPTR);

	(void)state; // unused
}

static void CS_fields_to_bytes_small_test(void **state)
{
	const uint32_t size = 8;
	const rio_cs_size cs_size = cs_small;
	const uint8_t pattern_a5[size] = {0x1c, 0xa5, 0xa5, 0xab};
	const uint8_t pattern_5a[size] = {0x7c, 0x5a, 0x5a, 0x52};

	CS_field_t fields_in;
	CS_bytes_t bytes_out;
	uint8_t expected[size];
	uint32_t rc;
	uint32_t idx;

	// invalid size
	memset(&bytes_out, 0, sizeof(CS_bytes_t));
	memset(expected, 0, size);
	fields_in.cs_size = cs_invalid;
	rc = CS_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_ERR_INVALID_PARAMETER, rc);
	assert_memory_equal(&expected, &bytes_out, size);

	// generate a5a5 pattern with valid crc
	memset(&bytes_out, 0, sizeof(CS_bytes_t));
	fields_in.cs_size = cs_size;
	fields_in.cs_t0 = stype0_VC_status;
	fields_in.parm_0 = 0x5;
	fields_in.parm_1 = 0x14;
	fields_in.cs_t1 = stype1_mecs;
	fields_in.cs_t1_cmd = 0x5;
	fields_in.cs_t2 = stype2_rsvd2; // ignored

	rc = CS_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, bytes_out.cs_type_valid);
	assert_memory_equal(&pattern_a5, bytes_out.cs_bytes, 8);

	// generate 5a5a pattern with valid crc
	memset(&bytes_out, 0, sizeof(CS_bytes_t));
	fields_in.cs_size = cs_size;
	fields_in.cs_t0 = stype0_pna;
	fields_in.parm_0 = 0x1a;
	fields_in.parm_1 = 0x0b;
	fields_in.cs_t1 = stype1_eop;
	fields_in.cs_t1_cmd = 0x2;
	fields_in.cs_t2 = stype2_rsvd2; // ignored

	rc = CS_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, bytes_out.cs_type_valid);
	assert_memory_equal(&pattern_5a, bytes_out.cs_bytes, 8);

	// ensure first byte is correct (not going to look at rest)
	memset(&bytes_out, 0, sizeof(CS_bytes_t));
	fields_in.cs_size = cs_size;
	fields_in.cs_t0 = stype0_pna;
	fields_in.parm_0 = 0x1a;
	fields_in.parm_1 = 0x0b;
	fields_in.cs_t1_cmd = 0x2;
	for (idx = stype1_sop; idx <= stype1_nop; idx++) {
		bytes_out.cs_bytes[0] = 0;
		fields_in.cs_t1 = (stype1)idx;

		rc = CS_fields_to_bytes(&fields_in, &bytes_out);
		assert_int_equal(RIO_SUCCESS, rc);
		switch (idx) {
		case 0:
		case 1:
		case 2:
		case 4:
			assert_int_equal(PD_CONTROL_SYMBOL,
					bytes_out.cs_bytes[0]);
			break;
		case 3:
		case 5:
		case 6:
		case 7:
			assert_int_equal(SC_START_CONTROL_SYMBOL,
					bytes_out.cs_bytes[0]);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void CS_fields_to_bytes_large_test(void **state)
{
	const uint32_t size = 8;
	const rio_cs_size cs_size = cs_large;
	const uint8_t pattern_a5[size] = {0x1c, 0xa5, 0xa5, 0xa5, 0xa5, 0xac,
			0x4c, 0x1c};
	const uint8_t pattern_5a[size] = {0x7c, 0x5a, 0x5a, 0x5a, 0x5a, 0x5c,
			0xbc, 0x7c};

	CS_field_t fields_in;
	CS_bytes_t bytes_out;
	uint8_t expected[size];
	uint32_t rc;
	uint32_t idx;

	// invalid size
	memset(&bytes_out, 0, sizeof(CS_bytes_t));
	memset(expected, 0, size);
	fields_in.cs_size = cs_invalid;
	rc = CS_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_ERR_INVALID_PARAMETER, rc);
	assert_memory_equal(&expected, &bytes_out, size);

	// generate a5a5 pattern with crc
	memset(&bytes_out, 0, sizeof(CS_bytes_t));
	fields_in.cs_size = cs_size;
	fields_in.cs_t0 = stype0_VC_status;
	fields_in.parm_0 = 0x0b;
	fields_in.parm_1 = 0x12;
	fields_in.cs_t1 = stype1_rsvd;
	fields_in.cs_t1_cmd = 0x4;
	fields_in.cs_t2 = stype2_rsvd5;
	fields_in.cs_t2_val = 0x52d;

	rc = CS_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, bytes_out.cs_type_valid);
	assert_memory_equal(&pattern_a5, bytes_out.cs_bytes, 8);

	// generate 5a5a pattern with valid crc
	memset(&bytes_out, 0, sizeof(CS_bytes_t));
	fields_in.cs_size = cs_size;
	fields_in.cs_t0 = stype0_pna;
	fields_in.parm_0 = 0x34;
	fields_in.parm_1 = 0x2d;
	fields_in.cs_t1 = stype1_stomp;
	fields_in.cs_t1_cmd = 3;
	fields_in.cs_t2 = stype2_rsvd2;
	fields_in.cs_t2_val = 0x2d2;

	rc = CS_fields_to_bytes(&fields_in, &bytes_out);

	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, bytes_out.cs_type_valid);
	assert_memory_equal(&pattern_5a, bytes_out.cs_bytes, 8);

	// ensure first byte is correct (not going to look at rest)
	memset(&bytes_out, 0, sizeof(CS_bytes_t));
	fields_in.cs_size = cs_size;
	fields_in.cs_t0 = stype0_pna;
	fields_in.parm_0 = 0x1a;
	fields_in.parm_1 = 0x0b;
	fields_in.cs_t1_cmd = 0x2;
	for (idx = stype1_sop; idx <= stype1_nop; idx++) {
		bytes_out.cs_bytes[0] = 0;
		fields_in.cs_t1 = (stype1)idx;

		rc = CS_fields_to_bytes(&fields_in, &bytes_out);
		assert_int_equal(RIO_SUCCESS, rc);
		switch (idx) {
		case 0:
		case 1:
		case 2:
		case 4:
			assert_int_equal(PD_CONTROL_SYMBOL,
					bytes_out.cs_bytes[0]);
			break;
		case 3:
		case 5:
		case 6:
		case 7:
			assert_int_equal(SC_START_CONTROL_SYMBOL,
					bytes_out.cs_bytes[0]);
			break;
		default:
			fail_msg("Invalid index %u", idx);
		}
	}

	(void)state; // unused
}

static void CS_bytes_to_fields_small_test(void **state)
{
	const uint8_t size = 4;
	const rio_cs_size cs_size = cs_small;
	const uint8_t pattern_a5[size] = {0x1c, 0xa5, 0xa5, 0xab};
	const uint8_t pattern_5a[size] = {0x7c, 0x5a, 0x5a, 0x52};

	CS_bytes_t bytes_in;
	CS_field_t fields_out;
	uint32_t rc;

	// generate a5a5 pattern with invalid crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0xa5, sizeof(bytes_in.cs_bytes));

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_VC_status, fields_out.cs_t0);
	assert_int_equal(0x5, fields_out.parm_0);
	assert_int_equal(0x14, fields_out.parm_1);
	assert_int_equal(stype1_mecs, fields_out.cs_t1);
	assert_int_equal(0x5, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_nop, fields_out.cs_t2);
	assert_int_equal(0, fields_out.cs_t2_val);
	assert_false(fields_out.cs_crc_correct);

	// generate a5a5 pattern with crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0xa5, sizeof(bytes_in.cs_bytes));
	bytes_in.cs_bytes[2] = 0xab;

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_VC_status, fields_out.cs_t0);
	assert_int_equal(0x5, fields_out.parm_0);
	assert_int_equal(0x14, fields_out.parm_1);
	assert_int_equal(stype1_mecs, fields_out.cs_t1);
	assert_int_equal(0x5, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_nop, fields_out.cs_t2);
	assert_int_equal(0, fields_out.cs_t2_val);
	assert_true(fields_out.cs_crc_correct);

	// generate a5a5 pattern with control symbol and invalid crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0, sizeof(bytes_in.cs_bytes));
	memcpy(&bytes_in.cs_bytes, pattern_a5, size);
	bytes_in.cs_bytes[size - 1] = 0xa5;

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_VC_status, fields_out.cs_t0);
	assert_int_equal(0x5, fields_out.parm_0);
	assert_int_equal(0x14, fields_out.parm_1);
	assert_int_equal(stype1_mecs, fields_out.cs_t1);
	assert_int_equal(0x5, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_nop, fields_out.cs_t2);
	assert_int_equal(0, fields_out.cs_t2_val);
	assert_false(fields_out.cs_crc_correct);

	// generate a5a5 pattern with control symbol and crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0, sizeof(bytes_in.cs_bytes));
	memcpy(&bytes_in.cs_bytes, pattern_a5, size);

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_VC_status, fields_out.cs_t0);
	assert_int_equal(0x5, fields_out.parm_0);
	assert_int_equal(0x14, fields_out.parm_1);
	assert_int_equal(stype1_mecs, fields_out.cs_t1);
	assert_int_equal(0x5, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_nop, fields_out.cs_t2);
	assert_int_equal(0, fields_out.cs_t2_val);
	assert_true(fields_out.cs_crc_correct);

	// generate 5a5a pattern with invalid crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0x5a, sizeof(bytes_in.cs_bytes));

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_pna, fields_out.cs_t0);
	assert_int_equal(0x1a, fields_out.parm_0);
	assert_int_equal(0x0b, fields_out.parm_1);
	assert_int_equal(stype1_eop, fields_out.cs_t1);
	assert_int_equal(0x2, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_nop, fields_out.cs_t2);
	assert_int_equal(0, fields_out.cs_t2_val);
	assert_false(fields_out.cs_crc_correct);

	// generate 5a5a pattern with crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0x5a, sizeof(bytes_in.cs_bytes));
	bytes_in.cs_bytes[2] = 0x52;

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_pna, fields_out.cs_t0);
	assert_int_equal(0x1a, fields_out.parm_0);
	assert_int_equal(0x0b, fields_out.parm_1);
	assert_int_equal(stype1_eop, fields_out.cs_t1);
	assert_int_equal(0x2, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_nop, fields_out.cs_t2);
	assert_int_equal(0, fields_out.cs_t2_val);
	assert_true(fields_out.cs_crc_correct);

	// generate 5a5a pattern with control symbol and invalid crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0, sizeof(bytes_in.cs_bytes));
	memcpy(&bytes_in.cs_bytes, pattern_5a, size);
	bytes_in.cs_bytes[size - 1] = 0x5a;

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_pna, fields_out.cs_t0);
	assert_int_equal(0x1a, fields_out.parm_0);
	assert_int_equal(0x0b, fields_out.parm_1);
	assert_int_equal(stype1_eop, fields_out.cs_t1);
	assert_int_equal(0x2, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_nop, fields_out.cs_t2);
	assert_int_equal(0, fields_out.cs_t2_val);
	assert_false(fields_out.cs_crc_correct);

	// generate 5a5a pattern with control symbol and crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0, sizeof(bytes_in.cs_bytes));
	memcpy(&bytes_in.cs_bytes, pattern_5a, size);

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_pna, fields_out.cs_t0);
	assert_int_equal(0x1a, fields_out.parm_0);
	assert_int_equal(0x0b, fields_out.parm_1);
	assert_int_equal(stype1_eop, fields_out.cs_t1);
	assert_int_equal(0x2, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_nop, fields_out.cs_t2);
	assert_int_equal(0, fields_out.cs_t2_val);
	assert_true(fields_out.cs_crc_correct);

	(void)state; // unused
}

static void CS_bytes_to_fields_large_test(void **state)
{

	const uint8_t size = 8;
	const rio_cs_size cs_size = cs_large;
	const uint8_t pattern_a5[size] = {0x1c, 0xa5, 0xa5, 0xa5, 0xa5, 0xac,
			0x4c, 0x1c};
	const uint8_t pattern_5a[size] = {0x7c, 0x5a, 0x5a, 0x5a, 0x5a, 0x5c,
			0xbc, 0x7c};

	CS_bytes_t bytes_in;
	CS_field_t fields_out;
	uint32_t rc;

	// generate a5a5 pattern with invalid crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0xa5, sizeof(bytes_in.cs_bytes));

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_VC_status, fields_out.cs_t0);
	assert_int_equal(0xb, fields_out.parm_0);
	assert_int_equal(0x12, fields_out.parm_1);
	assert_int_equal(stype1_rsvd, fields_out.cs_t1);
	assert_int_equal(0x4, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_rsvd5, fields_out.cs_t2);
	assert_int_equal(0x52d, fields_out.cs_t2_val);
	assert_false(fields_out.cs_crc_correct);

	// generate a5a5 pattern with crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0xa5, sizeof(bytes_in.cs_bytes));
	bytes_in.cs_bytes[4] = 0xac;
	bytes_in.cs_bytes[5] = 0x4c;

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_VC_status, fields_out.cs_t0);
	assert_int_equal(0xb, fields_out.parm_0);
	assert_int_equal(0x12, fields_out.parm_1);
	assert_int_equal(stype1_rsvd, fields_out.cs_t1);
	assert_int_equal(0x4, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_rsvd5, fields_out.cs_t2);
	assert_int_equal(0x52d, fields_out.cs_t2_val);
	assert_true(fields_out.cs_crc_correct);

	// generate a5a5 pattern with control symbol and invalid crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0, sizeof(bytes_in.cs_bytes));
	memcpy(&bytes_in.cs_bytes, pattern_a5, size);
	bytes_in.cs_bytes[size - 2] = 0xa5;

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_VC_status, fields_out.cs_t0);
	assert_int_equal(0xb, fields_out.parm_0);
	assert_int_equal(0x12, fields_out.parm_1);
	assert_int_equal(stype1_rsvd, fields_out.cs_t1);
	assert_int_equal(0x4, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_rsvd5, fields_out.cs_t2);
	assert_int_equal(0x52d, fields_out.cs_t2_val);
	assert_false(fields_out.cs_crc_correct);

	// generate a5a5 pattern with control symbol and crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0, sizeof(bytes_in.cs_bytes));
	memcpy(&bytes_in.cs_bytes, pattern_a5, size);

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_VC_status, fields_out.cs_t0);
	assert_int_equal(0xb, fields_out.parm_0);
	assert_int_equal(0x12, fields_out.parm_1);
	assert_int_equal(stype1_rsvd, fields_out.cs_t1);
	assert_int_equal(0x4, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_rsvd5, fields_out.cs_t2);
	assert_int_equal(0x52d, fields_out.cs_t2_val);
	assert_true(fields_out.cs_crc_correct);;

	// generate 5a5a pattern with invalid crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0x5a, sizeof(bytes_in.cs_bytes));

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_pna, fields_out.cs_t0);
	assert_int_equal(0x34, fields_out.parm_0);
	assert_int_equal(0x2d, fields_out.parm_1);
	assert_int_equal(stype1_stomp, fields_out.cs_t1);
	assert_int_equal(0x3, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_rsvd2, fields_out.cs_t2);
	assert_int_equal(0x2d2, fields_out.cs_t2_val);
	assert_false(fields_out.cs_crc_correct);

	// generate 5a5a pattern with crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0x5a, sizeof(bytes_in.cs_bytes));
	bytes_in.cs_bytes[4] = 0x5c;
	bytes_in.cs_bytes[5] = 0xbc;

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_pna, fields_out.cs_t0);
	assert_int_equal(0x34, fields_out.parm_0);
	assert_int_equal(0x2d, fields_out.parm_1);
	assert_int_equal(stype1_stomp, fields_out.cs_t1);
	assert_int_equal(0x3, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_rsvd2, fields_out.cs_t2);
	assert_int_equal(0x2d2, fields_out.cs_t2_val);
	assert_true(fields_out.cs_crc_correct);

	// generate 5a5a pattern with control symbol and invalid crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0, sizeof(bytes_in.cs_bytes));
	memcpy(&bytes_in.cs_bytes, pattern_5a, size);
	bytes_in.cs_bytes[size - 2] = 0x5a;

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_pna, fields_out.cs_t0);
	assert_int_equal(0x34, fields_out.parm_0);
	assert_int_equal(0x2d, fields_out.parm_1);
	assert_int_equal(stype1_stomp, fields_out.cs_t1);
	assert_int_equal(0x3, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_rsvd2, fields_out.cs_t2);
	assert_int_equal(0x2d2, fields_out.cs_t2_val);
	assert_false(fields_out.cs_crc_correct);

	// generate 5a5a pattern with control symbol and crc
	bytes_in.cs_type_valid = cs_size;
	memset(&bytes_in.cs_bytes, 0, sizeof(bytes_in.cs_bytes));
	memcpy(&bytes_in.cs_bytes, pattern_5a, size);

	rc = CS_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(cs_size, fields_out.cs_size);
	assert_int_equal(stype0_pna, fields_out.cs_t0);
	assert_int_equal(0x34, fields_out.parm_0);
	assert_int_equal(0x2d, fields_out.parm_1);
	assert_int_equal(stype1_stomp, fields_out.cs_t1);
	assert_int_equal(0x3, fields_out.cs_t1_cmd);
	assert_int_equal(stype2_rsvd2, fields_out.cs_t2);
	assert_int_equal(0x2d2, fields_out.cs_t2_val);
	assert_true(fields_out.cs_crc_correct);

	(void)state; // unused
}

static void get_stype0_descr_test(void **state)
{
	// highlights if a string changes value
	CS_field_t fields_in;

	fields_in.cs_t0 = stype0_pa;
	assert_string_equal("Packet-Accepted", get_stype0_descr(&fields_in));

	fields_in.cs_t0 = stype0_rty;
	assert_string_equal("Packet-Retry", get_stype0_descr(&fields_in));

	fields_in.cs_t0 = stype0_pna;
	assert_string_equal("Packet-no-Accepted", get_stype0_descr(&fields_in));

	fields_in.cs_t0 = stype0_rsvd;
	assert_string_equal("Reserved", get_stype0_descr(&fields_in));

	fields_in.cs_t0 = stype0_status;
	assert_string_equal("Status", get_stype0_descr(&fields_in));

	fields_in.cs_t0 = stype0_VC_status;
	assert_string_equal("VC_Status", get_stype0_descr(&fields_in));

	fields_in.cs_t0 = stype0_lresp;
	assert_string_equal("Link_response", get_stype0_descr(&fields_in));

	fields_in.cs_t0 = stype0_imp;
	assert_string_equal("Imp_Spec", get_stype0_descr(&fields_in));

	(void)state; // unused
}

static void get_stype0_PNA_cause_parm1_test(void **state)
{
	// highlights if a string changes value
	CS_field_t fields_in;
	uint32_t idx;

	for (idx = stype0_min; idx <= stype0_max; idx++) {
		if (idx == stype0_pna) {
			continue;
		}
		fields_in.cs_t0 = (stype0)idx;
		assert_null(get_stype0_PNA_cause_parm1(&fields_in));
	}

	fields_in.cs_t0 = stype0_pna;
	for (idx = 0; idx < 0xff; idx++) { // upb is arbitrary
		fields_in.parm_1 = idx;
		switch (idx) {
		case 0:
			assert_string_equal("Reserved",
					(get_stype0_PNA_cause_parm1(&fields_in)));
			break;
		case 1:
			assert_string_equal("Unexpected AckID",
					(get_stype0_PNA_cause_parm1(&fields_in)));
			break;
		case 2:
			assert_string_equal("Bad CS CRC",
					(get_stype0_PNA_cause_parm1(&fields_in)));
			break;
		case 3:
			assert_string_equal("Non-mtc RX stopped",
					(get_stype0_PNA_cause_parm1(&fields_in)));
			break;
		case 4:
			assert_string_equal("Bad Pkt CRC",
					(get_stype0_PNA_cause_parm1(&fields_in)));
			break;
		case 5:
			assert_string_equal("Bad 10b char",
					(get_stype0_PNA_cause_parm1(&fields_in)));
			break;
		case 6:
			assert_string_equal("Lack of resources",
					(get_stype0_PNA_cause_parm1(&fields_in)));
			break;
		case 7:
			assert_string_equal("Lost Descrambler Sync",
					(get_stype0_PNA_cause_parm1(&fields_in)));
			break;
		case 0x1f:
			assert_string_equal("General Error",
					(get_stype0_PNA_cause_parm1(&fields_in)));
			break;
		default:
			assert_string_equal("Reserved",
					(get_stype0_PNA_cause_parm1(&fields_in)));
			break;
		}
	}

	(void)state; // unused
}

static void get_stype0_LR_port_status_parm1_test(void **state)
{
	// highlights if a string changes value
	CS_field_t fields_in;
	uint32_t idx;

	for (idx = stype0_min; idx <= stype0_max; idx++) {
		if (idx == stype0_lresp) {
			continue;
		}
		fields_in.cs_t0 = (stype0)idx;
		assert_null(get_stype0_LR_port_status_parm1(&fields_in));
	}

	fields_in.cs_t0 = stype0_lresp;
	for (idx = 0; idx < 0xff; idx++) { // upb is arbitrary
		fields_in.parm_1 = idx;
		switch (fields_in.parm_1) {
		case 2:
			assert_string_equal("Error",
					(get_stype0_LR_port_status_parm1(
							&fields_in)));
			break;
		case 4:
			assert_string_equal("Retry-stopped",
					(get_stype0_LR_port_status_parm1(
							&fields_in)));
			break;
		case 5:
			assert_string_equal("Error-stopped",
					(get_stype0_LR_port_status_parm1(
							&fields_in)));
			break;
		case 0x10:
			assert_string_equal("OK",
					(get_stype0_LR_port_status_parm1(
							&fields_in)));
			break;
		default:
			assert_string_equal("Reserved",
					(get_stype0_LR_port_status_parm1(
							&fields_in)));
			break;
		}
	}

	(void)state; // unused
}

static void get_stype1_descr_test(void **state)
{
	// highlights if a string changes value
	CS_field_t fields_in;

	fields_in.cs_t1 = stype1_sop;
	assert_string_equal("Start of Packet", get_stype1_descr(&fields_in));

	fields_in.cs_t1 = stype1_stomp;
	assert_string_equal("Stomp", get_stype1_descr(&fields_in));

	fields_in.cs_t1 = stype1_eop;
	assert_string_equal("End-of-packet", get_stype1_descr(&fields_in));

	fields_in.cs_t1 = stype1_rfr;
	assert_string_equal("restart-from-retry", get_stype1_descr(&fields_in));

	fields_in.cs_t1 = stype1_lreq;
	assert_string_equal("Link-Request", get_stype1_descr(&fields_in));

	fields_in.cs_t1 = stype1_mecs;
	assert_string_equal("Multicast-Event", get_stype1_descr(&fields_in));

	fields_in.cs_t1 = stype1_rsvd;
	assert_string_equal("Reserved", get_stype1_descr(&fields_in));

	fields_in.cs_t1 = stype1_nop;
	assert_string_equal("NOP", get_stype1_descr(&fields_in));

	(void)state; // unused
}

static void get_stype1_lreq_cmd_test(void **state)
{
	// highlights if a string changes value
	CS_field_t fields_in;
	uint32_t idx;

	for (idx = stype1_min; idx <= stype1_max; idx++) {
		if (idx == stype1_lreq) {
			continue;
		}
		fields_in.cs_t1 = (stype1)idx;
		assert_null(get_stype1_lreq_cmd(&fields_in));
	}

	fields_in.cs_t1 = stype1_lreq;
	for (idx = 0; idx < 0xff; idx++) {
		fields_in.cs_t1_cmd = idx;
		switch (fields_in.cs_t1_cmd) {
		case 3:
			assert_string_equal("reset-device",
					(get_stype1_lreq_cmd(&fields_in)));
			break;
		case 4:
			assert_string_equal("input-status",
					(get_stype1_lreq_cmd(&fields_in)));
			break;
		default:
			assert_string_equal("Reserved",
					(get_stype1_lreq_cmd(&fields_in)));
			break;
		}
	}

	(void)state; // unused
}

static void get_stype2_descr_test(void **state)
{
	// highlights if a string changes value
	CS_field_t fields_in;
	uint32_t idx;

	for (idx = stype2_min; idx <= stype2_max; idx++) {
		fields_in.cs_t2 = (stype2)idx;
		switch (idx) {
		case 0:
			assert_string_equal("NOP",
					(get_stype2_descr(&fields_in)));
			break;
		case 1:
			assert_string_equal("VoQ Backpressure",
					(get_stype2_descr(&fields_in)));
			break;
		default:
			assert_string_equal("Reserved",
					(get_stype2_descr(&fields_in)));
			break;
		}
	}

	(void)state; // unused
}

static void DAR_pkt_fields_to_bytes_ftype_2_memsz_34_test(void **state)
{
	const uint8_t expected[] = {0x02, 0x82, 0xde, 0xad, 0x44, 0xa1, 0xde,
			0xad, 0xbe, 0xeb, 0xa9, 0xc0};

	DAR_pkt_fields_t fields_in;
	DAR_pkt_bytes_t bytes_out;
	uint32_t rc;

	memset(&fields_in, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));

	fields_in.tot_bytes = 0xff;
	fields_in.pad_bytes = 0xff;

	fields_in.phys.pkt_ackID = 0x4; // skipped by algorithm
	fields_in.phys.pkt_vc = 1;
	fields_in.phys.crf = 0;
	fields_in.phys.pkt_prio = 2;

	fields_in.trans.tt_code = tt_small;
	fields_in.trans.destID = 0xde;
	fields_in.trans.srcID = 0xad;
	fields_in.trans.hopcount = 0xff; // ignored

	fields_in.pkt_type = pkt_nr;
	fields_in.pkt_bytes = 2;
	fields_in.pkt_data = 0;

	fields_in.log_rw.pkt_addr_size = rio_addr_34;
	fields_in.log_rw.addr[0] = 0xdeadbee8;
	fields_in.log_rw.addr[1] = 0x3;
	fields_in.log_rw.addr[2] = 0x0;
	fields_in.log_rw.tid = 0xa1;
	fields_in.log_rw.status = pkt_err; // ignored

	// not used by packet
	memset(&fields_in.log_fc, 0xca, sizeof(fields_in.log_fc));
	memset(&fields_in.log_ds, 0xa5, sizeof(fields_in.log_ds));
	memset(&fields_in.log_ms, 0x5a, sizeof(fields_in.log_ms));

	rc = DAR_pkt_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&expected, &bytes_out.pkt_data, sizeof(expected));
	assert_int_equal(12, bytes_out.num_chars);
	assert_true(bytes_out.pkt_has_crc);
	assert_false(bytes_out.pkt_padded);

	(void)state; // unused
}

static void DAR_pkt_fields_to_bytes_ftype_2_memsz_50_test(void **state)
{
	const uint8_t expected[] = {0x01, 0x52, 0xde, 0xad, 0xbe, 0xef, 0x44,
			0x1a, 0x00, 0x03, 0xca, 0xfe, 0xba, 0xbc, 0x84, 0xd1};

	DAR_pkt_fields_t fields_in;
	DAR_pkt_bytes_t bytes_out;
	uint32_t rc;

	memset(&fields_in, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));

	fields_in.tot_bytes = 0xff;
	fields_in.pad_bytes = 0xff;

	fields_in.phys.pkt_ackID = 0x2; // skipped by algorithm
	fields_in.phys.pkt_vc = 0;
	fields_in.phys.crf = 1;
	fields_in.phys.pkt_prio = 1;

	fields_in.trans.tt_code = tt_large;
	fields_in.trans.destID = 0xdead;
	fields_in.trans.srcID = 0xbeef;
	fields_in.trans.hopcount = 0xff; // ignored

	fields_in.pkt_type = pkt_nr;
	fields_in.pkt_bytes = 2;
	fields_in.pkt_data = 0;

	fields_in.log_rw.pkt_addr_size = rio_addr_50;
	fields_in.log_rw.addr[0] = 0xcafebabc;
	fields_in.log_rw.addr[1] = 0x3;
	fields_in.log_rw.addr[2] = 0x0;
	fields_in.log_rw.tid = 0x1a;
	fields_in.log_rw.status = pkt_err; // ignored

	// not used by packet
	memset(&fields_in.log_fc, 0xca, sizeof(fields_in.log_fc));
	memset(&fields_in.log_ds, 0xa5, sizeof(fields_in.log_ds));
	memset(&fields_in.log_ms, 0x5a, sizeof(fields_in.log_ms));

	rc = DAR_pkt_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&expected, &bytes_out.pkt_data, sizeof(expected));
	assert_int_equal(16, bytes_out.num_chars);
	assert_true(bytes_out.pkt_has_crc);
	assert_false(bytes_out.pkt_padded);

	(void)state; // unused
}

static void DAR_pkt_fields_to_bytes_ftype_5_memsz_66_test(void **state)
{
	const uint8_t expected[] = {0x02, 0xd5, 0xca, 0xfe, 0xba, 0xbe, 0x54,
			0x67, 0xa5, 0x5a, 0xa5, 0x5a, 0xde, 0xad, 0xbe, 0xe8,
			0xa5, 0xa5, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf0, 0x91};

	DAR_pkt_fields_t fields_in;
	DAR_pkt_bytes_t bytes_out;
	uint8_t pkt_data[RIO_MAX_PKT_BYTES];
	uint32_t rc;

	memset(&fields_in, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));

	fields_in.tot_bytes = 0xff;
	fields_in.pad_bytes = 0xff;

	fields_in.phys.pkt_ackID = 0x1; // skipped by algorithm
	fields_in.phys.pkt_vc = 1;
	fields_in.phys.crf = 0;
	fields_in.phys.pkt_prio = 3;

	fields_in.trans.tt_code = tt_large;
	fields_in.trans.destID = 0xcafe;
	fields_in.trans.srcID = 0xbabe;
	fields_in.trans.hopcount = 0xff; // ignored

	fields_in.pkt_type = pkt_nwr;
	fields_in.pkt_bytes = 2;

	fields_in.log_rw.pkt_addr_size = rio_addr_66;
	fields_in.log_rw.addr[0] = 0xdeadbee8;
	fields_in.log_rw.addr[1] = 0xa55aa55a;
	fields_in.log_rw.addr[2] = 0x0;
	fields_in.log_rw.tid = 0x67;
	fields_in.log_rw.status = pkt_err; // ignored

	// not used by packet
	memset(&fields_in.log_fc, 0xca, sizeof(fields_in.log_fc));
	memset(&fields_in.log_ds, 0xa5, sizeof(fields_in.log_ds));
	memset(&fields_in.log_ms, 0x5a, sizeof(fields_in.log_ms));

	memset(&pkt_data, 0xa5, sizeof(pkt_data));
	fields_in.pkt_data = pkt_data;

	rc = DAR_pkt_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&expected, &bytes_out.pkt_data, sizeof(expected));
	assert_int_equal(28, bytes_out.num_chars);
	assert_true(bytes_out.pkt_has_crc);
	assert_true(bytes_out.pkt_padded);

	(void)state; // unused
}

static void DAR_pkt_fields_to_bytes_ftype_6_memsz_32_test(void **state)
{
	const uint8_t expected[] = {0x02, 0x46, 0xca, 0xfe, 0xde, 0xad, 0xbe,
			0xe8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x52, 0xc1};

	DAR_pkt_fields_t fields_in;
	DAR_pkt_bytes_t bytes_out;
	uint32_t rc;

	memset(&fields_in, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));

	fields_in.tot_bytes = 0xff;
	fields_in.pad_bytes = 0xff;

	fields_in.phys.pkt_ackID = 0xcf; // skipped by algorithm
	fields_in.phys.pkt_vc = 1;
	fields_in.phys.crf = 0;
	fields_in.phys.pkt_prio = 1;

	fields_in.trans.tt_code = tt_small;
	fields_in.trans.destID = 0xca;
	fields_in.trans.srcID = 0xfe;
	fields_in.trans.hopcount = 0xff; // ignored

	fields_in.pkt_type = pkt_sw;
	fields_in.pkt_bytes = 2;

	fields_in.log_rw.pkt_addr_size = rio_addr_32;
	fields_in.log_rw.addr[0] = 0xdeadbee8;
	fields_in.log_rw.addr[1] = 0x0;
	fields_in.log_rw.addr[2] = 0x0;
	fields_in.log_rw.tid = 0x67; // not used
	fields_in.log_rw.status = pkt_err; // ignored

	// not used by packet
	memset(&fields_in.log_fc, 0xca, sizeof(fields_in.log_fc));
	memset(&fields_in.log_ds, 0xa5, sizeof(fields_in.log_ds));
	memset(&fields_in.log_ms, 0x5a, sizeof(fields_in.log_ms));

	rc = DAR_pkt_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&expected, &bytes_out.pkt_data, sizeof(expected));
	assert_int_equal(16, bytes_out.num_chars);
	assert_true(bytes_out.pkt_has_crc);
	assert_false(bytes_out.pkt_padded);

	(void)state; // unused
}

static void DAR_pkt_fields_to_bytes_ftype_7_pkt_type_test(void **state)
{
	const uint8_t expected[] = {0x02, 0x47, 0xca, 0xfe, 0x30, 0x3, 0xe5,
			0x46};

	DAR_pkt_fields_t fields_in;
	DAR_pkt_bytes_t bytes_out;
	uint32_t rc;

	memset(&fields_in, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));

	fields_in.tot_bytes = 0xff;
	fields_in.pad_bytes = 0xff;

	fields_in.phys.pkt_ackID = 0xcf; // skipped by algorithm
	fields_in.phys.pkt_vc = 1;
	fields_in.phys.crf = 0;
	fields_in.phys.pkt_prio = 1;

	fields_in.trans.tt_code = tt_small;
	fields_in.trans.destID = 0xca;
	fields_in.trans.srcID = 0xfe;
	fields_in.trans.hopcount = 0xff; // ignored

	fields_in.log_fc.fc_destID = 0xde; // not used
	fields_in.log_fc.fc_srcID = 0xad; // not used
	fields_in.log_fc.fc_fam = fc_fam_011;
	fields_in.log_fc.fc_flow = fc_flow_0B;
	fields_in.log_fc.fc_soc_is_ep = true;
	fields_in.log_fc.fc_xon = false;

	fields_in.pkt_type = pkt_fc;
	fields_in.pkt_bytes = 2;

	// not used by packet
	memset(&fields_in.log_rw, 0xca, sizeof(fields_in.log_rw));
	memset(&fields_in.log_ds, 0xa5, sizeof(fields_in.log_ds));
	memset(&fields_in.log_ms, 0x5a, sizeof(fields_in.log_ms));

	rc = DAR_pkt_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&expected, &bytes_out.pkt_data, sizeof(expected));
	assert_int_equal(8, bytes_out.num_chars);
	assert_true(bytes_out.pkt_has_crc);
	assert_false(bytes_out.pkt_padded);

	(void)state; // unused
}

static void DAR_pkt_fields_to_bytes_ftype_8_memsz_21_test(void **state)
{
	const uint8_t expected[] = {0x02, 0x48, 0xca, 0xfe, 0x4, 0x49, 0xff, 0xad, 0xbe, 0xe8, 0x84, 0x57};

	DAR_pkt_fields_t fields_in;
	DAR_pkt_bytes_t bytes_out;
	uint32_t rc;

	memset(&fields_in, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));

	fields_in.tot_bytes = 0xff;
	fields_in.pad_bytes = 0xff;

	fields_in.phys.pkt_ackID = 0xcf; // skipped by algorithm
	fields_in.phys.pkt_vc = 1;
	fields_in.phys.crf = 0;
	fields_in.phys.pkt_prio = 1;

	fields_in.trans.tt_code = tt_small;
	fields_in.trans.destID = 0xca;
	fields_in.trans.srcID = 0xfe;
	fields_in.trans.hopcount = 0xff; // ignored

	fields_in.pkt_type = pkt_mr;
	fields_in.pkt_bytes = 2;

	fields_in.log_rw.pkt_addr_size = rio_addr_21;
	fields_in.log_rw.addr[0] = 0xdeadbee8;
	fields_in.log_rw.addr[1] = 0x0;
	fields_in.log_rw.addr[2] = 0x0;
	fields_in.log_rw.tid = 0x49; // not used
	fields_in.log_rw.status = pkt_err; // ignored

	// not used by packet
	memset(&fields_in.log_fc, 0xca, sizeof(fields_in.log_fc));
	memset(&fields_in.log_ds, 0xa5, sizeof(fields_in.log_ds));
	memset(&fields_in.log_ms, 0x5a, sizeof(fields_in.log_ms));

	rc = DAR_pkt_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&expected, &bytes_out.pkt_data, sizeof(expected));
	assert_int_equal(12, bytes_out.num_chars);
	assert_true(bytes_out.pkt_has_crc);
	assert_false(bytes_out.pkt_padded);

	(void)state; // unused
}

static void DAR_pkt_fields_to_bytes_ftype_9_pkt_type_test(void **state)
{
	const uint8_t expected[] = {0x02, 0x49, 0xca, 0xfe, 0x0, 0xc, 0xca, 0xfe, 0x42, 0xbe, 0xef, 0x5a,
			  0xde, 0x6e};

	DAR_pkt_fields_t fields_in;
	DAR_pkt_bytes_t bytes_out;
	uint32_t rc;

	memset(&fields_in, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));

	fields_in.tot_bytes = 0xff;
	fields_in.pad_bytes = 0xff;

	fields_in.phys.pkt_ackID = 0xcf; // skipped by algorithm
	fields_in.phys.pkt_vc = 1;
	fields_in.phys.crf = 0;
	fields_in.phys.pkt_prio = 1;

	fields_in.trans.tt_code = tt_small;
	fields_in.trans.destID = 0xca;
	fields_in.trans.srcID = 0xfe;
	fields_in.trans.hopcount = 0xff; // ignored

	fields_in.pkt_type = pkt_dstm;
	fields_in.pkt_bytes = 2;

	fields_in.log_ds.dstm_COS = 0;
	fields_in.log_ds.dstm_PDU_len = 0x03;
	fields_in.log_ds.dstm_end_seg = true;
	fields_in.log_ds.dstm_odd_data_amt = false;
	fields_in.log_ds.dstm_pad_data_amt = true;
	fields_in.log_ds.dstm_start_seg = false;
	fields_in.log_ds.dstm_streamid = 0xcafe;
	fields_in.log_ds.dstm_xh_COS_mask = 0xbabe;
	fields_in.log_ds.dstm_xh_parm1 = 0xdeadbeef;
	fields_in.log_ds.dstm_xh_parm2 = 0xa55aa55a;
	fields_in.log_ds.dstm_xh_seg = true;
	fields_in.log_ds.dstm_xh_tm_op = 0x1234;
	fields_in.log_ds.dstm_xh_type = 0x4321;
	fields_in.log_ds.dstm_xh_wildcard = 0x66aa55aa;

	// not used by packet
	memset(&fields_in.log_rw, 0xa5, sizeof(fields_in.log_rw));
	memset(&fields_in.log_fc, 0xca, sizeof(fields_in.log_fc));
	memset(&fields_in.log_ms, 0x5a, sizeof(fields_in.log_ms));

	rc = DAR_pkt_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&expected, &bytes_out.pkt_data, sizeof(expected));
	assert_int_equal(16, bytes_out.num_chars);
	assert_true(bytes_out.pkt_has_crc);
	assert_true(bytes_out.pkt_padded);

	(void)state; // unused
}

static void DAR_pkt_fields_to_bytes_ftype_10_pkt_type_test(void **state)
{
	const uint8_t expected[] = {0x02, 0x4a, 0xca, 0xfe, 0x0, 0x1a, 0xa1, 0xa1, 0x57, 0xd0};

	DAR_pkt_fields_t fields_in;
	DAR_pkt_bytes_t bytes_out;
	uint8_t pkt_data[RIO_MAX_PKT_BYTES];
	uint32_t rc;

	memset(&fields_in, 0, sizeof(DAR_pkt_fields_t));
	memset(&pkt_data, 0xa1, 2);
	fields_in.pkt_data = pkt_data;
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));

	fields_in.tot_bytes = 0xff;
	fields_in.pad_bytes = 0xff;

	fields_in.phys.pkt_ackID = 0xcf; // skipped by algorithm
	fields_in.phys.pkt_vc = 1;
	fields_in.phys.crf = 0;
	fields_in.phys.pkt_prio = 1;

	fields_in.trans.tt_code = tt_small;
	fields_in.trans.destID = 0xca;
	fields_in.trans.srcID = 0xfe;
	fields_in.trans.hopcount = 0xff; // ignored

	fields_in.pkt_type = pkt_db;
	fields_in.pkt_bytes = 2;

	fields_in.log_rw.pkt_addr_size = rio_addr_32; // not used
	fields_in.log_rw.addr[0] = 0xdeadbee8; // not used
	fields_in.log_rw.addr[1] = 0xcaca; // not used
	fields_in.log_rw.addr[2] = 0xacac; // not used
	fields_in.log_rw.tid = 0x1a;
	fields_in.log_rw.status = pkt_err; // ignored

	// not used by packet
	memset(&fields_in.log_ds, 0xa5, sizeof(fields_in.log_ds));
	memset(&fields_in.log_fc, 0xca, sizeof(fields_in.log_fc));
	memset(&fields_in.log_ms, 0x5a, sizeof(fields_in.log_ms));

	rc = DAR_pkt_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&expected, &bytes_out.pkt_data, sizeof(expected));
	assert_int_equal(12, bytes_out.num_chars);
	assert_true(bytes_out.pkt_has_crc);
	assert_true(bytes_out.pkt_padded);

	(void)state; // unused
}

static void DAR_pkt_fields_to_bytes_ftype_11_pkt_type_test(void **state)
{
	const uint8_t expected[] = {0x02, 0x4b, 0xca, 0xfe, 0x19, 0x61, 0xa1, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x00,
			  0x0, 0xd4, 0xac};

	DAR_pkt_fields_t fields_in;
	DAR_pkt_bytes_t bytes_out;
	uint8_t pkt_data[RIO_MAX_PKT_BYTES];
	uint32_t rc;

	memset(&fields_in, 0, sizeof(DAR_pkt_fields_t));
	memset(&pkt_data, 0xa1, 2);
	fields_in.pkt_data = pkt_data;
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));

	fields_in.tot_bytes = 0xff;
	fields_in.pad_bytes = 0xff;

	fields_in.phys.pkt_ackID = 0xcf; // skipped by algorithm
	fields_in.phys.pkt_vc = 1;
	fields_in.phys.crf = 0;
	fields_in.phys.pkt_prio = 1;

	fields_in.trans.tt_code = tt_small;
	fields_in.trans.destID = 0xca;
	fields_in.trans.srcID = 0xfe;
	fields_in.trans.hopcount = 0xff; // ignored

	fields_in.pkt_type = pkt_msg;
	fields_in.pkt_bytes = 8;

	fields_in.log_ms.letter = 1;
	fields_in.log_ms.mbid = 2;
	fields_in.log_ms.msg_len = 1;
	fields_in.log_ms.msgseg = 1;

	// not used by packet
	memset(&fields_in.log_ds, 0xa5, sizeof(fields_in.log_ds));
	memset(&fields_in.log_fc, 0xca, sizeof(fields_in.log_fc));
	memset(&fields_in.log_rw, 0x5a, sizeof(fields_in.log_rw));
	fields_in.log_rw.addr[0] = 0xcafeb8;

	rc = DAR_pkt_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&expected, &bytes_out.pkt_data, sizeof(expected));
	assert_int_equal(16, bytes_out.num_chars);
	assert_true(bytes_out.pkt_has_crc);
	assert_false(bytes_out.pkt_padded);

	(void)state; // unused
}
static void DAR_pkt_fields_to_bytes_ftype_13_pkt_type_test(void **state)
{
	const uint8_t expected[] = {0x02, 0x4d, 0xca, 0xfe, 0x10, 0x61, 0xe9, 0xea};

	DAR_pkt_fields_t fields_in;
	DAR_pkt_bytes_t bytes_out;
	uint8_t pkt_data[RIO_MAX_PKT_BYTES];
	uint32_t rc;

	memset(&fields_in, 0, sizeof(DAR_pkt_fields_t));
	memset(&pkt_data, 0xa1, 2);
	fields_in.pkt_data = pkt_data;
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));

	fields_in.tot_bytes = 0xff;
	fields_in.pad_bytes = 0xff;

	fields_in.phys.pkt_ackID = 0xcf; // skipped by algorithm
	fields_in.phys.pkt_vc = 1;
	fields_in.phys.crf = 0;
	fields_in.phys.pkt_prio = 1;

	fields_in.trans.tt_code = tt_small;
	fields_in.trans.destID = 0xca;
	fields_in.trans.srcID = 0xfe;
	fields_in.trans.hopcount = 0xff; // ignored

	fields_in.pkt_type = pkt_msg_resp;

	fields_in.log_ms.letter = 1;
	fields_in.log_ms.mbid = 2;
	fields_in.log_ms.msg_len = 1;
	fields_in.log_ms.msgseg = 1;

	// not used by packet
	memset(&fields_in.log_ds, 0xa5, sizeof(fields_in.log_ds));
	memset(&fields_in.log_fc, 0xca, sizeof(fields_in.log_fc));
	memset(&fields_in.log_rw, 0x5a, sizeof(fields_in.log_rw));
	fields_in.log_rw.addr[0] = 0xcafeb8;

	rc = DAR_pkt_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&expected, &bytes_out.pkt_data, sizeof(expected));
	assert_int_equal(8, bytes_out.num_chars);
	assert_true(bytes_out.pkt_has_crc);
	assert_false(bytes_out.pkt_padded);

	(void)state; // unused
}

static void DAR_pkt_fields_to_bytes_ftype_raw_pkt_type_test(void **state)
{
	const uint8_t expected[] = {0xa1, 0xa1, 0xa1, 0xa1, 0xa1, 0xa1, 0xa1, 0xa1, 0x34, 0x3d};

	DAR_pkt_fields_t fields_in;
	DAR_pkt_bytes_t bytes_out;
	uint8_t pkt_data[RIO_MAX_PKT_BYTES];
	uint32_t rc;

	memset(&fields_in, 0, sizeof(DAR_pkt_fields_t));
	memset(&pkt_data, 0xa1, 8);
	fields_in.pkt_data = pkt_data;
	memset(&bytes_out, 0, sizeof(DAR_pkt_bytes_t));

	fields_in.tot_bytes = 0xff;
	fields_in.pad_bytes = 0xff;

	fields_in.phys.pkt_ackID = 0xcf; // skipped by algorithm
	fields_in.phys.pkt_vc = 1;
	fields_in.phys.crf = 0;
	fields_in.phys.pkt_prio = 1;

	fields_in.trans.tt_code = tt_small;
	fields_in.trans.destID = 0xca;
	fields_in.trans.srcID = 0xfe;
	fields_in.trans.hopcount = 0xff; // ignored

	fields_in.pkt_type = pkt_raw;
	fields_in.pkt_bytes = 8;

	// not used by packet
	memset(&fields_in.log_ds, 0xa5, sizeof(fields_in.log_ds));
	memset(&fields_in.log_fc, 0xca, sizeof(fields_in.log_fc));
	memset(&fields_in.log_rw, 0x5a, sizeof(fields_in.log_rw));
	memset(&fields_in.log_ms, 0x5a, sizeof(fields_in.log_ms));
	fields_in.log_rw.addr[0] = 0xcafebee8;

	rc = DAR_pkt_fields_to_bytes(&fields_in, &bytes_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_memory_equal(&expected, &bytes_out.pkt_data, sizeof(expected));
	assert_int_equal(12, bytes_out.num_chars);
	assert_true(bytes_out.pkt_has_crc);
	assert_true(bytes_out.pkt_padded);

	(void)state; // unused
}

#define TST_MIN_NUM_CHARS 8
#define TST_INVLD_TT_CODE rio_TT_code_max

static void DAR_pkt_bytes_to_fields_invalid_tt_code_test(void **state)
{
	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	DAR_pkt_fields_t expected;

	uint32_t idx;
	uint32_t rc;

	// test isolates the tt_code and verifies all invalid values
	// ignores other fields and values
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&expected, 0, sizeof(DAR_pkt_fields_t));

	for (idx = rio_TT_code_min; idx <= rio_TT_code_max; idx++) {
		if ((tt_small == (rio_TT_code)idx)
				|| (tt_large == (rio_TT_code)idx)) {
			continue;
		}
		memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
		bytes_in.num_chars = TST_MIN_NUM_CHARS;
		bytes_in.pkt_data[1] = idx << 4;
		expected.trans.tt_code = (rio_TT_code)idx;

		rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
		assert_int_equal(DAR_UTIL_INVALID_TT, rc);
		assert_memory_equal(&expected, &fields_out,
				sizeof(DAR_pkt_fields_t));
	}

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_not_enough_bytes_test(void **state)
{
	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	DAR_pkt_fields_t expected;

	uint32_t idx;
	uint32_t rc;

	// test ensures the minimum number of bytes is provided
	// ensures in the event of failure the out parameter is not updated
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&expected, 0, sizeof(DAR_pkt_fields_t));

	for (idx = 0; idx < TST_MIN_NUM_CHARS; idx++) {
		memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
		bytes_in.num_chars = idx;
		rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
		assert_int_equal(DAR_UTIL_BAD_DATA_SIZE, rc);
		assert_memory_equal(&expected, &fields_out,
				sizeof(DAR_pkt_fields_t));
	}

	// give enough bytes, but fail on the rio_TT_code for simplicity
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	bytes_in.pkt_data[1] = TST_INVLD_TT_CODE << 4;

	// not going to verify the fields_out data as that is the responsibility
	// of the test that ensure the rio_TT_code is handled correctly
	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(DAR_UTIL_INVALID_TT, rc);

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_bytes_0_and_1_test(void **state)
{
	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;

	uint32_t rc;

	// verify the contents of bytes 0 and 1, fail on a rio_TT_code error for simplicity
	// ignores subsequent fields and values
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	bytes_in.pkt_data[0] = 0xa5;
	bytes_in.pkt_data[1] = 0xbd;

	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(DAR_UTIL_INVALID_TT, rc);
	assert_int_equal(0x29, fields_out.phys.pkt_ackID);
	assert_int_equal(0x0, fields_out.phys.pkt_vc);
	assert_int_equal(0x1, fields_out.phys.crf);
	assert_int_equal(0x2, fields_out.phys.pkt_prio);
	assert_int_equal(0x3, fields_out.trans.tt_code);

	// verify the contents of bytes 0 and 1, fail on a rio_TT_code error for simplicity
	// ignores subsequent fields and values
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	bytes_in.pkt_data[0] = 0xd2;
	bytes_in.pkt_data[1] = 0xea;

	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(DAR_UTIL_INVALID_TT, rc);
	assert_int_equal(0x34, fields_out.phys.pkt_ackID);
	assert_int_equal(0x1, fields_out.phys.pkt_vc);
	assert_int_equal(0x0, fields_out.phys.crf);
	assert_int_equal(0x3, fields_out.phys.pkt_prio);
	assert_int_equal(0x2, fields_out.trans.tt_code);

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_src_dst_addr_size_test(void **state)
{
	// ftype = bytes_in->pkt_data[1]
	// tt_code = bytes_in->pkt_data[1] & 0x30
	const uint8_t small_addr[] = {0x12, 0x02, 0xde, 0xad, 0x40};
	const uint8_t large_addr[] = {0x34, 0x12, 0xde, 0xad, 0xbe, 0xef, 0xc0};

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint32_t rc;

	// verify small addresses are correctly parsed
	// ignores subsequent fields and values
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	memcpy(&bytes_in.pkt_data, &small_addr, sizeof(small_addr));

	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(0xde, fields_out.trans.destID);
	assert_int_equal(0xad, fields_out.trans.srcID);

	// verify large addresses are correctly parsed
	// ignores subsequent fields and values
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	memcpy(&bytes_in.pkt_data, &large_addr, sizeof(large_addr));

	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(0xdead, fields_out.trans.destID);
	assert_int_equal(0xbeef, fields_out.trans.srcID);

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_2_pkt_type_test(void **state)
{
	const uint8_t input[] = {0x12, 0x02, 0xde, 0xad, 0x44, 0xa1, 0xc2, 0xc4,
			0xc0, 0xa3};
	const uint32_t pkt_type_idx = 4;
	const uint32_t pkt_type_shift = 4;

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint32_t expected_rc;
	uint32_t expected_pkt_type;
	uint32_t idx;
	uint32_t rc;

	// verify pkt_type
	// ignore other fields
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	bytes_in.pkt_addr_size = rio_addr_34;
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));

	for (idx = 0; idx < 0x10; idx++) {
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		expected_rc = RIO_SUCCESS;
		switch (idx) {
		case 0x4:
			expected_pkt_type = pkt_nr;
			break;
		case 0xc:
			expected_pkt_type = pkt_nr_inc;
			break;
		case 0xd:
			expected_pkt_type = pkt_nr_dec;
			break;
		case 0xe:
			expected_pkt_type = pkt_nr_set;
			break;
		case 0xf:
			expected_pkt_type = pkt_nr_clr;
			break;
		default:
			expected_rc = DAR_UTIL_UNKNOWN_TRANS;
			expected_pkt_type = pkt_raw;
		}

		bytes_in.pkt_data[pkt_type_idx] = (idx << pkt_type_shift);
		rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
		assert_int_equal(expected_rc, rc);
		assert_int_equal(0xde, fields_out.trans.destID);
		assert_int_equal(0xad, fields_out.trans.srcID);
		assert_int_equal(expected_pkt_type, fields_out.pkt_type);

		assert_memory_equal(&zero_memory, &fields_out.log_ds, sizeof(fields_out.log_ds));
		assert_memory_equal(&zero_memory, &fields_out.log_fc, sizeof(fields_out.log_fc));
		assert_memory_equal(&zero_memory, &fields_out.log_ms, sizeof(fields_out.log_ms));
	}

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_2_memsz_34_test(void **state)
{
	const uint8_t input[] = {0x02, 0x82, 0xde, 0xad, 0x44, 0xa1, 0xde, 0xad,
			0xbe, 0xeb, 0xa9, 0xc0};

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint32_t rc;

	// verify fields for an ftype of 2, and pkt_type of pkt_nr
	// uses an address size of rio_addr_34
	// uses and rdsize of 4
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	bytes_in.pkt_addr_size = rio_addr_34;
	bytes_in.num_chars = sizeof(input);
	bytes_in.pkt_has_crc = true;
	bytes_in.pkt_padded = false;

	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);

//	assert_int_equal(4, fields_out.phys.pkt_ackID);
	assert_int_equal(0, fields_out.phys.pkt_ackID);
	assert_int_equal(1, fields_out.phys.pkt_vc);
	assert_int_equal(0, fields_out.phys.crf);

	assert_int_equal(2, fields_out.phys.pkt_prio);
	assert_int_equal(tt_small, fields_out.trans.tt_code);
	assert_int_equal(pkt_nr, fields_out.pkt_type);

	assert_int_equal(0xde, fields_out.trans.destID);
	assert_int_equal(0xad, fields_out.trans.srcID);

	assert_int_equal(0xa1, fields_out.log_rw.tid);
	assert_int_equal(rio_addr_34, fields_out.log_rw.pkt_addr_size);
	assert_int_equal(0xdeadbee8, fields_out.log_rw.addr[0]);
	assert_int_equal(0x3, fields_out.log_rw.addr[1]);
	assert_int_equal(0x0, fields_out.log_rw.addr[2]);

	assert_int_equal(2, fields_out.pkt_bytes);

	assert_memory_equal(&zero_memory, &fields_out.log_ds, sizeof(fields_out.log_ds));
	assert_memory_equal(&zero_memory, &fields_out.log_fc, sizeof(fields_out.log_fc));
	assert_memory_equal(&zero_memory, &fields_out.log_ms, sizeof(fields_out.log_ms));

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_2_memsz_50_test(void **state)
{
	const uint8_t input[] = {0x01, 0x52, 0xde, 0xad, 0xbe, 0xef, 0x44, 0x1a,
			0x00, 0x03, 0xca, 0xfe, 0xba, 0xbc, 0x84, 0xd1};

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint32_t rc;

	// verify fields for an ftype of 2, and pkt_type of pkt_nr_dec
	// uses an address size of rio_addr_50
	// uses and rdsize of 6
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	bytes_in.pkt_addr_size = rio_addr_50;
	bytes_in.num_chars = sizeof(input);
	bytes_in.pkt_has_crc = true;
	bytes_in.pkt_padded = false;

	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);

//	assert_int_equal(2, fields_out.phys.pkt_ackID);
	assert_int_equal(0, fields_out.phys.pkt_ackID);
	assert_int_equal(0, fields_out.phys.pkt_vc);
	assert_int_equal(1, fields_out.phys.crf);

	assert_int_equal(1, fields_out.phys.pkt_prio);
	assert_int_equal(tt_large, fields_out.trans.tt_code);
	assert_int_equal(pkt_nr, fields_out.pkt_type);

	assert_int_equal(0xdead, fields_out.trans.destID);
	assert_int_equal(0xbeef, fields_out.trans.srcID);

	assert_int_equal(0x1a, fields_out.log_rw.tid);
	assert_int_equal(rio_addr_50, fields_out.log_rw.pkt_addr_size);
	assert_int_equal(0xcafebabc, fields_out.log_rw.addr[0]);
	assert_int_equal(0x3, fields_out.log_rw.addr[1]);
	assert_int_equal(0x0, fields_out.log_rw.addr[2]);

	assert_int_equal(2, fields_out.pkt_bytes);

	assert_memory_equal(&zero_memory, &fields_out.log_ds, sizeof(fields_out.log_ds));
	assert_memory_equal(&zero_memory, &fields_out.log_fc, sizeof(fields_out.log_fc));
	assert_memory_equal(&zero_memory, &fields_out.log_ms, sizeof(fields_out.log_ms));

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_5_pkt_type_test(void **state)
{
	const uint8_t input[] = {0x12, 0x05, 0xde, 0xad, 0x54, 0xa1, 0xc2, 0xc4,
			0xc0, 0xa3};
	const uint32_t pkt_type_idx = 4;
	const uint32_t pkt_type_shift = 4;

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint8_t pkt_data[RIO_MAX_PKT_BYTES];
	uint32_t expected_rc;
	uint32_t expected_pkt_type;
	uint32_t idx;
	uint32_t rc;

	// verify pkt_type
	// ignore other fields
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	bytes_in.pkt_addr_size = rio_addr_34;
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));

	for (idx = 0; idx < 0x10; idx++) {
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		fields_out.pkt_data = pkt_data;
		expected_rc = RIO_SUCCESS;
		switch (idx) {
		case 0x4:
			expected_pkt_type = pkt_nw;
			break;
		case 0x5:
			expected_pkt_type = pkt_nwr;
			break;
		case 0xc:
			expected_pkt_type = pkt_nw_swap;
			break;
		case 0xd:
			expected_pkt_type = pkt_nw_cmp_swap;
			break;
		case 0xe:
			expected_pkt_type = pkt_nw_tst_swap;
			break;
		default:
			expected_rc = DAR_UTIL_UNKNOWN_TRANS;
			expected_pkt_type = 0;
		}

		bytes_in.pkt_data[pkt_type_idx] = (idx << pkt_type_shift);
		rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
		assert_int_equal(expected_rc, rc);
		assert_int_equal(0xde, fields_out.trans.destID);
		assert_int_equal(0xad, fields_out.trans.srcID);
		assert_int_equal(expected_pkt_type, fields_out.pkt_type);

		assert_memory_equal(&zero_memory, &fields_out.log_ds, sizeof(fields_out.log_ds));
		assert_memory_equal(&zero_memory, &fields_out.log_fc, sizeof(fields_out.log_fc));
		assert_memory_equal(&zero_memory, &fields_out.log_ms, sizeof(fields_out.log_ms));
	}

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_5_memsz_66_test(void **state)
{
	const uint8_t input[] = {0x02, 0xd5, 0xca, 0xfe, 0xba, 0xbe, 0x54, 0x67,
			0xa5, 0x5a, 0xa5, 0x5a, 0xde, 0xad, 0xbe, 0xe8, 0xa5,
			0xa5, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf0, 0x91};

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint8_t pkt_data[RIO_MAX_PKT_BYTES];
	uint32_t rc;

	// verify fields for an ftype of 2, and pkt_type of pkt_nr_dec
	// uses an address size of rio_addr_50
	// uses and rdsize of 6
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&pkt_data, 0, sizeof(pkt_data));
	fields_out.pkt_data = pkt_data;

	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	bytes_in.pkt_addr_size = rio_addr_66;
	bytes_in.num_chars = sizeof(input);
	bytes_in.pkt_has_crc = true;
	bytes_in.pkt_padded = true;

	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);

//	assert_int_equal(1, fields_out.phys.pkt_ackID);
	assert_int_equal(0, fields_out.phys.pkt_ackID);
	assert_int_equal(1, fields_out.phys.pkt_vc);
	assert_int_equal(0, fields_out.phys.crf);

	assert_int_equal(3, fields_out.phys.pkt_prio);
	assert_int_equal(tt_large, fields_out.trans.tt_code);
	assert_int_equal(pkt_nwr, fields_out.pkt_type);

	assert_int_equal(0xcafe, fields_out.trans.destID);
	assert_int_equal(0xbabe, fields_out.trans.srcID);

	assert_int_equal(0x67, fields_out.log_rw.tid);
	assert_int_equal(rio_addr_66, fields_out.log_rw.pkt_addr_size);
	assert_int_equal(0xdeadbee8, fields_out.log_rw.addr[0]);
	assert_int_equal(0xa55aa55a, fields_out.log_rw.addr[1]);
	assert_int_equal(0x0, fields_out.log_rw.addr[2]);

	assert_memory_equal(&zero_memory, &fields_out.log_ds, sizeof(fields_out.log_ds));
	assert_memory_equal(&zero_memory, &fields_out.log_fc, sizeof(fields_out.log_fc));
	assert_memory_equal(&zero_memory, &fields_out.log_ms, sizeof(fields_out.log_ms));

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_6_pkt_type_test(void **state)
{
	// ftype = bytes_in->pkt_data[1]
	// pkt_type = bytes_in->pkt_data[4] & 0xF0) >> 4) (small addr)
	const uint8_t input[] = {0x12, 0x06, 0xde, 0xad, 0x64, 0xa1, 0xc2, 0xc4,
			0xc0, 0xa3};
	const uint32_t pkt_type_idx = 4;
	const uint32_t pkt_type_shift = 4;

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint8_t pkt_data[RIO_MAX_PKT_BYTES];
	uint32_t idx;
	uint32_t rc;

	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&pkt_data, 0, sizeof(pkt_data));
	fields_out.pkt_data = pkt_data;

	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	bytes_in.pkt_addr_size = rio_addr_34;
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));

	for (idx = 0; idx < 0x10; idx++) {
		bytes_in.pkt_data[pkt_type_idx] = (idx << pkt_type_shift);
		rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
		assert_int_equal(RIO_SUCCESS, rc);
		assert_int_equal(0xde, fields_out.trans.destID);
		assert_int_equal(0xad, fields_out.trans.srcID);
		assert_int_equal(pkt_sw, fields_out.pkt_type);

		assert_memory_equal(&zero_memory, &fields_out.log_ds, sizeof(fields_out.log_ds));
		assert_memory_equal(&zero_memory, &fields_out.log_fc, sizeof(fields_out.log_fc));
		assert_memory_equal(&zero_memory, &fields_out.log_ms, sizeof(fields_out.log_ms));
	}

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_6_memsz_32_test(void **state)
{
	const uint8_t input[] = {0x2, 0x46, 0xca, 0xfe, 0xde, 0xad, 0xbe, 0xe8,
			0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x52, 0xc1};

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint8_t pkt_data[RIO_MAX_PKT_BYTES];
	uint32_t rc;

	// verify fields for an ftype of 6, and pkt_type of pkt_sw
	// uses an address size of rio_addr_32
	// uses and rdsize of 6
	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&pkt_data, 0x5a, sizeof(pkt_data));
	fields_out.pkt_data = pkt_data;

	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	bytes_in.pkt_addr_size = rio_addr_32;
	bytes_in.num_chars = sizeof(input);
	bytes_in.pkt_has_crc = true;
	bytes_in.pkt_padded = true;

	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);

//	assert_int_equal(1, fields_out.phys.pkt_ackID);
	assert_int_equal(0, fields_out.phys.pkt_ackID);
	assert_int_equal(1, fields_out.phys.pkt_vc);
	assert_int_equal(0, fields_out.phys.crf);

	assert_int_equal(1, fields_out.phys.pkt_prio);
	assert_int_equal(tt_small, fields_out.trans.tt_code);
	assert_int_equal(pkt_sw, fields_out.pkt_type);

	assert_int_equal(0xca, fields_out.trans.destID);
	assert_int_equal(0xfe, fields_out.trans.srcID);

	assert_int_equal(0x0, fields_out.log_rw.tid);
	assert_int_equal(rio_addr_32, fields_out.log_rw.pkt_addr_size);
	assert_int_equal(0xdeadbee8, fields_out.log_rw.addr[0]);
	assert_int_equal(0x0, fields_out.log_rw.addr[1]);
	assert_int_equal(0x0, fields_out.log_rw.addr[2]);

	assert_memory_equal(&zero_memory, &fields_out.log_ds, sizeof(fields_out.log_ds));
	assert_memory_equal(&zero_memory, &fields_out.log_fc, sizeof(fields_out.log_fc));
	assert_memory_equal(&zero_memory, &fields_out.log_ms, sizeof(fields_out.log_ms));

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_7_pkt_type_test(void **state)
{
	// ftype = bytes_in->pkt_data[1]
	// pkt_type = bytes_in->pkt_data[4] & 0xF0) >> 4) (small addr)
	const uint8_t input[] = {0x12, 0x07, 0xde, 0xad, 0x64, 0xa1, 0xc2};

	const uint32_t pkt_type_idx = 4;
	const uint32_t pkt_type_shift = 4;

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint32_t idx;
	uint32_t rc;

	// verify pkt_type and all other fields
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	bytes_in.pkt_addr_size = rio_addr_34;
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));

	for (idx = 0; idx < 0x10; idx++) {
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		bytes_in.pkt_data[pkt_type_idx] = (idx << pkt_type_shift);
		rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
		assert_int_equal(RIO_SUCCESS, rc);
		assert_int_equal(0xde, fields_out.trans.destID);
		assert_int_equal(0xad, fields_out.trans.srcID);
		assert_int_equal(pkt_fc, fields_out.pkt_type);
		if (idx < 8) {
			assert_false(fields_out.log_fc.fc_xon);
		} else {
			assert_true(fields_out.log_fc.fc_xon);
		}
		assert_int_equal(idx % 8, fields_out.log_fc.fc_fam);
		assert_int_equal(0x50, fields_out.log_fc.fc_flow);
		assert_true(fields_out.log_fc.fc_soc_is_ep);
		assert_int_equal(0xde, fields_out.log_fc.fc_destID);
		assert_int_equal(0xad, fields_out.log_fc.fc_srcID);

		assert_memory_equal(&zero_memory, &fields_out.log_rw, sizeof(fields_out.log_rw));
		assert_memory_equal(&zero_memory, &fields_out.log_ds, sizeof(fields_out.log_ds));
		assert_memory_equal(&zero_memory, &fields_out.log_ms, sizeof(fields_out.log_ms));
	}

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_8_memsz_21_test(void **state)
{
	// ftype = bytes_in->pkt_data[1 & 0x0F]
	// pkt_type = bytes_in->pkt_data[4] & 0xF0) >> 4) (small addr)
	const uint8_t input[] = {0x12, 0x08, 0xde, 0xad, 0x44, 0xa1, 0xc2, 0xca,
			0xfe, 0xba};
	const uint32_t pkt_type_idx = 4;
	const uint32_t pkt_type_shift = 4;

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint8_t pkt_data[RIO_MAX_PKT_BYTES];
	uint32_t expected_rc;
	uint32_t expected_pkt_type;
	uint32_t expected_addr;
	uint32_t expected_pkt_bytes;
	uint32_t idx;
	uint32_t rc;

	// verify pkt_type
	// ignore other fields
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	bytes_in.pkt_addr_size = rio_addr_21;
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));

	for (idx = 0; idx < 0x10; idx++) {
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		fields_out.pkt_data = pkt_data;

		expected_rc = RIO_SUCCESS;
		expected_pkt_bytes = 0;
		switch (idx) {
		case 0:
			expected_pkt_type = pkt_mr;
			expected_addr = 0xcafeb8;
			expected_pkt_bytes = 1;
			break;
		case 1:
			expected_pkt_type = pkt_mw;
			expected_addr = 0xcafeb8;
			expected_pkt_bytes = 1;
			break;
		case 2:
			expected_pkt_type = pkt_mrr;
			expected_addr = 0xcafeb8;
			break;
		case 3:
			expected_pkt_type = pkt_mwr;
			expected_addr = 0xcafeb8;
			break;
		case 4:
			expected_pkt_type = pkt_pw;
			expected_addr = 0xcafeb8;
			expected_pkt_bytes = 1;
			break;
		default:
			expected_rc = DAR_UTIL_UNKNOWN_TRANS;
			expected_pkt_type = pkt_raw;
			expected_addr = 0x0;
		}

		bytes_in.pkt_addr_size = rio_addr_21;
		bytes_in.pkt_data[pkt_type_idx] = (idx << pkt_type_shift);
		rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
		assert_int_equal(expected_rc, rc);
		assert_int_equal(0xde, fields_out.trans.destID);
		assert_int_equal(0xad, fields_out.trans.srcID);
		assert_int_equal(expected_pkt_type, fields_out.pkt_type);
		assert_int_equal(expected_addr, fields_out.log_rw.addr[0]);
		assert_int_equal(0x0, fields_out.log_rw.addr[1]);
		assert_int_equal(0x0, fields_out.log_rw.addr[2]);
		assert_int_equal(expected_pkt_bytes, fields_out.pkt_bytes);
		if ((idx > 4)) {
			assert_int_equal(rio_addr_21, bytes_in.pkt_addr_size);
			assert_int_equal(0, fields_out.log_rw.tid);
			assert_int_equal(0, fields_out.trans.hopcount);
			assert_int_equal(0, fields_out.log_rw.pkt_addr_size);
		} else {
			assert_int_equal(rio_addr_21, bytes_in.pkt_addr_size);
			assert_int_equal(0xa1, fields_out.log_rw.tid);
			assert_int_equal(0xc2, fields_out.trans.hopcount);
			assert_int_equal(rio_addr_21,
					fields_out.log_rw.pkt_addr_size);
		}

		assert_memory_equal(&zero_memory, &fields_out.log_fc, sizeof(fields_out.log_fc));
		assert_memory_equal(&zero_memory, &fields_out.log_ds, sizeof(fields_out.log_ds));
		assert_memory_equal(&zero_memory, &fields_out.log_ms, sizeof(fields_out.log_ms));
	}

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_9_pkt_type_test(void **state)
{
	// ftype = bytes_in->pkt_data[1 & 0x0F]
	const uint8_t input[] = {0x12, 0x09, 0xde, 0xad, 0x44, 0xa1, 0xca, 0xfe,
			0xba, 0xde, 0xad, 0xbe, 0xef};

	const uint32_t ext_idx = 5;

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint32_t rc;

	// extended bit not set
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	bytes_in.pkt_addr_size = rio_addr_34;
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));

	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(0xde, fields_out.trans.destID);
	assert_int_equal(0xad, fields_out.trans.srcID);
	assert_int_equal(pkt_dstm, fields_out.pkt_type);
	assert_int_equal(0x44, fields_out.log_ds.dstm_COS);

	assert_false(fields_out.log_ds.dstm_xh_seg);
	assert_int_equal(0, fields_out.log_ds.dstm_xh_type);
	assert_int_equal(0, fields_out.log_ds.dstm_xh_tm_op);
	assert_int_equal(0, fields_out.log_ds.dstm_xh_wildcard);
	assert_int_equal(0, fields_out.log_ds.dstm_xh_COS_mask);
	assert_int_equal(0, fields_out.log_ds.dstm_xh_parm1);
	assert_int_equal(0, fields_out.log_ds.dstm_xh_parm2);
	assert_true(fields_out.log_ds.dstm_start_seg);
	assert_false(fields_out.log_ds.dstm_end_seg);
	assert_false(fields_out.log_ds.dstm_odd_data_amt);
	assert_true(fields_out.log_ds.dstm_pad_data_amt);
	assert_int_equal(0xcafe, fields_out.log_ds.dstm_streamid);
	assert_int_equal(0, fields_out.log_ds.dstm_PDU_len);

	// extended bit set
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	bytes_in.pkt_addr_size = rio_addr_34;
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	bytes_in.pkt_data[ext_idx] = 0x27;

	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(0xde, fields_out.trans.destID);
	assert_int_equal(0xad, fields_out.trans.srcID);
	assert_int_equal(pkt_dstm, fields_out.pkt_type);
	assert_int_equal(0x44, fields_out.log_ds.dstm_COS);

	assert_true(fields_out.log_ds.dstm_xh_seg);
	assert_false(fields_out.log_ds.dstm_start_seg);
	assert_false(fields_out.log_ds.dstm_end_seg);
	assert_false(fields_out.log_ds.dstm_odd_data_amt);
	assert_false(fields_out.log_ds.dstm_pad_data_amt);
	assert_int_equal(4, fields_out.log_ds.dstm_xh_type);
	assert_int_equal(0xcafe, fields_out.log_ds.dstm_streamid);
	assert_int_equal(0, fields_out.log_ds.dstm_PDU_len);
	assert_int_equal(0xb, fields_out.log_ds.dstm_xh_tm_op);
	assert_int_equal(2, fields_out.log_ds.dstm_xh_wildcard);
	assert_int_equal(2, fields_out.log_ds.dstm_xh_wildcard);
	assert_int_equal(0xde, fields_out.log_ds.dstm_xh_COS_mask);
	assert_int_equal(0xad, fields_out.log_ds.dstm_xh_parm1);
	assert_int_equal(0xbe, fields_out.log_ds.dstm_xh_parm2);

	assert_memory_equal(&zero_memory, &fields_out.log_rw, sizeof(fields_out.log_rw));
	assert_memory_equal(&zero_memory, &fields_out.log_fc, sizeof(fields_out.log_fc));
	assert_memory_equal(&zero_memory, &fields_out.log_ms, sizeof(fields_out.log_ms));

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_10_pkt_type_test(void **state)
{
	// ftype = bytes_in->pkt_data[1 & 0x0F]
	const uint8_t input[] = {0x12, 0x0a, 0xde, 0xad, 0x44, 0xa1, 0xca, 0xfe,
			0xba, 0xbe};

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	DAR_pkt_fields_t log_rw;
	uint8_t pkt_data[RIO_MAX_PKT_BYTES];
	uint8_t out[RIO_MAX_PKT_BYTES];
	uint32_t rc;

	// verify all fields
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));

	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(pkt_data, 0xa5, sizeof(pkt_data));
	fields_out.pkt_data = pkt_data;

	memset(out, 0xa5, sizeof(pkt_data));
	out[0] = 0xca;
	out[1] = 0xfe;

	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(0xde, fields_out.trans.destID);
	assert_int_equal(0xad, fields_out.trans.srcID);
	assert_int_equal(pkt_db, fields_out.pkt_type);
	assert_int_equal(0xa1, fields_out.log_rw.tid);
	assert_int_equal(0xca, fields_out.pkt_data[0]);
	assert_int_equal(0xfe, fields_out.pkt_data[1]);
	assert_int_equal(2, fields_out.pkt_bytes);
	assert_memory_equal(out, fields_out.pkt_data, sizeof(out));

	memset(&log_rw, 0, sizeof(DAR_pkt_fields_t));
	log_rw.log_rw.tid = 0xa1;

	assert_memory_equal(&log_rw.log_rw, &fields_out.log_rw, sizeof(fields_out.log_rw));
	assert_memory_equal(&zero_memory, &fields_out.log_fc, sizeof(fields_out.log_fc));
	assert_memory_equal(&zero_memory, &fields_out.log_ds, sizeof(fields_out.log_ds));
	assert_memory_equal(&zero_memory, &fields_out.log_ms, sizeof(fields_out.log_ms));

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_11_pkt_type_test(void **state)
{
	// ftype = bytes_in->pkt_data[1 & 0x0F]
	const uint8_t input[] = {0x12, 0x0b, 0xde, 0xad, 0xae, 0x5a, 0x44, 0x88,
			0xba, 0xbe};
	const uint32_t length_idx = 4;

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint8_t pkt_data[RIO_MAX_PKT_BYTES];
	uint32_t idx;
	uint32_t rc;

	// verify all fields
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));

	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(pkt_data, 0xa5, sizeof(pkt_data));
	fields_out.pkt_data = pkt_data;

	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(0xde, fields_out.trans.destID);
	assert_int_equal(0xad, fields_out.trans.srcID);
	assert_int_equal(pkt_msg, fields_out.pkt_type);
	assert_int_equal(0xa, fields_out.log_ms.msg_len);
	assert_int_equal(1, fields_out.log_ms.letter);

	if (fields_out.log_ms.msg_len) {
		assert_int_equal(1, fields_out.log_ms.mbid);
		assert_int_equal(0xa, fields_out.log_ms.msgseg);
	} else {
		assert_int_equal(0, fields_out.log_ms.mbid);
		assert_int_equal(0, fields_out.log_ms.msgseg);
	}

	assert_memory_equal(&zero_memory, &fields_out.log_rw, sizeof(fields_out.log_rw));
	assert_memory_equal(&zero_memory, &fields_out.log_fc, sizeof(fields_out.log_fc));
	assert_memory_equal(&zero_memory, &fields_out.log_ds, sizeof(fields_out.log_ds));

	// verify failure for rdsize < 9
	for (idx = 0; idx < 9; idx++) {
		bytes_in.pkt_data[length_idx] = (0xa0 | idx);
		rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
		assert_int_equal(DAR_UTIL_BAD_MSG_DSIZE, rc);
	}

	bytes_in.pkt_data[length_idx] = 0xaa;
	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_13_pkt_type_test(void **state)
{
	// ftype = bytes_in->pkt_data[1 & 0x0F]
	// pkt_type = bytes_in->pkt_data[4] & 0xF0) >> 4) (small addr)
	const uint8_t input[] = {0x12, 0x0d, 0xde, 0xad, 0x44, 0x5a, 0xca, 0xfe,
			0xba, 0xbe};
	const uint32_t pkt_type_idx = 4;
	const uint32_t pkt_type_shift = 4;

	DAR_pkt_bytes_t bytes_in;
	DAR_pkt_fields_t fields_out;
	uint8_t out[RIO_MAX_PKT_BYTES];

	uint32_t expected_rc;
	uint32_t expected_pkt_type;
	uint32_t expected_status;
	uint32_t idx;
	uint32_t idx2;
	uint32_t rc;

	// verify all fields
	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	bytes_in.num_chars = TST_MIN_NUM_CHARS;
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));

	for (idx = 0; idx < 0x10; idx++) {
		memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
		memset(out, 0, sizeof(out));
		fields_out.pkt_data = out;
		expected_rc = RIO_SUCCESS;

		switch (idx) {
		case 0:
			expected_pkt_type = pkt_resp;
			break;
		case 1:
			expected_pkt_type = pkt_msg_resp;
			break;
		case 8:
			expected_pkt_type = pkt_resp_data;
			break;
		default:
			expected_pkt_type = 0;
			expected_rc = DAR_UTIL_UNKNOWN_TRANS;
		}

		bytes_in.pkt_data[pkt_type_idx] = (idx << pkt_type_shift);
		rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
		assert_int_equal(expected_rc, rc);
		assert_int_equal(0xde, fields_out.trans.destID);
		assert_int_equal(0xad, fields_out.trans.srcID);
		assert_int_equal(expected_pkt_type, fields_out.pkt_type);

		if (DAR_UTIL_UNKNOWN_TRANS == rc) {
			continue;
		}

		for (idx2 = 0; idx2 < 0x10; idx2++) {
			memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
			memset(out, 0, sizeof(out));
			fields_out.pkt_data = out;
			expected_rc = RIO_SUCCESS;

			switch (idx2) {
			case 0:
				expected_status = pkt_done;
				break;
			case 3:
				expected_status = pkt_retry;
				break;
			case 7:
				expected_status = pkt_err;
				break;
			default:
				expected_status = 0;
				expected_rc = DAR_UTIL_UNKNOWN_STATUS;
			}

			bytes_in.pkt_data[pkt_type_idx] = ((idx
					<< pkt_type_shift) | idx2);
			rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
			assert_int_equal(expected_rc, rc);
			assert_int_equal(expected_status,
					fields_out.log_ms.status);
			assert_int_equal(expected_status,
					fields_out.log_rw.status);
		}

		if (DAR_UTIL_UNKNOWN_STATUS == rc) {
			continue;
		}

		if (pkt_msg_resp == fields_out.pkt_type) {
			assert_int_equal(1, fields_out.log_ms.letter);
			assert_int_equal(2, fields_out.log_ms.mbid);
			assert_int_equal(0, fields_out.log_ms.msgseg);
			assert_int_equal(0, fields_out.log_rw.tid);
		} else {
			assert_int_equal(0, fields_out.log_ms.letter);
			assert_int_equal(0, fields_out.log_ms.mbid);
			assert_int_equal(0, fields_out.log_ms.msgseg);
			assert_int_equal(0x5a, fields_out.log_rw.tid);
		}

		assert_memory_equal(&zero_memory, &fields_out.log_ds, sizeof(fields_out.log_ds));
		assert_memory_equal(&zero_memory, &fields_out.log_fc, sizeof(fields_out.log_fc));
	}

	(void)state; // unused
}

static void DAR_pkt_bytes_to_fields_ftype_raw_pkt_type_test(void **state)
{
	const uint8_t input[] = {0x12, 0x00, 0xde, 0xad, 0xa1, 0xa1, 0xa1, 0xa1, 0xa1, 0xa1, 0xa1, 0xa1, 0x34, 0x3d};

	DAR_pkt_fields_t fields_out;
	DAR_pkt_bytes_t bytes_in;
	uint8_t pkt_data[RIO_MAX_PKT_BYTES];
	uint32_t rc;

	memset(&fields_out, 0, sizeof(DAR_pkt_fields_t));
	memset(&pkt_data, 0, sizeof(pkt_data));
	fields_out.pkt_data = pkt_data;

	memset(&bytes_in, 0, sizeof(DAR_pkt_bytes_t));
	memcpy(&bytes_in.pkt_data, &input, sizeof(input));
	bytes_in.pkt_has_crc = true;
	bytes_in.pkt_padded = true;
	bytes_in.num_chars = 12;
	bytes_in.pkt_addr_size = rio_addr_32;

	rc = DAR_pkt_bytes_to_fields(&bytes_in, &fields_out);
	assert_int_equal(RIO_SUCCESS, rc);
	assert_int_equal(0xde, fields_out.trans.destID);
	assert_int_equal(0xad, fields_out.trans.srcID);
	assert_int_equal(pkt_raw, fields_out.pkt_type);

	assert_int_equal(0, fields_out.tot_bytes);
	assert_int_equal(0, fields_out.pad_bytes);

	assert_int_equal(4, fields_out.phys.pkt_ackID);
	assert_int_equal(1, fields_out.phys.pkt_vc);
	assert_int_equal(0, fields_out.phys.crf);
	assert_int_equal(0, fields_out.phys.pkt_prio);

	assert_int_equal(tt_small, fields_out.trans.tt_code);
	assert_int_equal(0xde, fields_out.trans.destID);
	assert_int_equal(0xad, fields_out.trans.srcID);
	assert_int_equal(0, fields_out.trans.hopcount);

	assert_int_equal(pkt_raw, fields_out.pkt_type);
	assert_int_equal(10, fields_out.pkt_bytes);

	assert_memory_equal(&bytes_in.pkt_data, &pkt_data, 10);

	assert_memory_equal(&zero_memory, &fields_out.log_rw, sizeof(fields_out.log_rw));
	assert_memory_equal(&zero_memory, &fields_out.log_fc, sizeof(fields_out.log_fc));
	assert_memory_equal(&zero_memory, &fields_out.log_ds, sizeof(fields_out.log_ds));
	assert_memory_equal(&zero_memory, &fields_out.log_ms, sizeof(fields_out.log_ms));

	(void)state; // unused
}

static void DAR_pkt_ftype_descr_test(void **state)
{
	// highlights if a string changes value
	DAR_pkt_fields_t pkt_fields;
	uint32_t idx;

	for (idx = pkt_type_min; idx <= pkt_type_max; idx++) {
		pkt_fields.pkt_type = (DAR_pkt_type)idx;
		switch (idx) {
		case pkt_raw:
			assert_string_equal("RAW",
					(DAR_pkt_ftype_descr(&pkt_fields)));
			break;
		case (pkt_nr):
		case (pkt_nr_inc):
		case (pkt_nr_dec):
		case (pkt_nr_set):
		case (pkt_nr_clr):
			assert_string_equal("NRead",
					(DAR_pkt_ftype_descr(&pkt_fields)));
			break;
		case (pkt_nw):
		case (pkt_nwr):
		case (pkt_nw_swap):
		case (pkt_nw_cmp_swap):
		case (pkt_nw_tst_swap):
			assert_string_equal("NWrite",
					(DAR_pkt_ftype_descr(&pkt_fields)));
			break;
		case (pkt_sw):
			assert_string_equal("SWrite",
					(DAR_pkt_ftype_descr(&pkt_fields)));
			break;
		case (pkt_fc):
			assert_string_equal("Flow Control",
					(DAR_pkt_ftype_descr(&pkt_fields)));
			break;
		case (pkt_mr):
		case (pkt_mw):
		case (pkt_mrr):
		case (pkt_mwr):
		case (pkt_pw):
			assert_string_equal("Maintenance",
					(DAR_pkt_ftype_descr(&pkt_fields)));
			break;
		case (pkt_dstm):
			assert_string_equal("Data Streaming",
					(DAR_pkt_ftype_descr(&pkt_fields)));
			break;
		case (pkt_db):
			assert_string_equal("Doorbell",
					(DAR_pkt_ftype_descr(&pkt_fields)));
			break;
		case (pkt_msg):
			assert_string_equal("Message",
					(DAR_pkt_ftype_descr(&pkt_fields)));
			break;
		case (pkt_resp):
		case (pkt_resp_data):
		case (pkt_msg_resp):
			assert_string_equal("Response",
					(DAR_pkt_ftype_descr(&pkt_fields)));
			break;
		default:
			assert_string_equal("Reserved",
					(DAR_pkt_ftype_descr(&pkt_fields)));
			break;
		}
	}

	(void)state; // unused
}

static void DAR_pkt_trans_descr_test(void **state)
{
	// highlights if a string changes value
	DAR_pkt_fields_t pkt_fields;
	uint32_t idx;

	for (idx = pkt_type_min; idx <= pkt_type_max; idx++) {
		pkt_fields.pkt_type = (DAR_pkt_type)idx;
		switch (idx) {
		case pkt_raw:
			assert_string_equal("",
					DAR_pkt_trans_descr(&pkt_fields));
			break;
		case (pkt_nr):
			assert_string_equal("NREAD",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;
		case (pkt_nr_inc):
			assert_string_equal("ATOMIC inc",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;
		case (pkt_nr_dec):
			assert_string_equal("ATOMIC dec",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;
		case (pkt_nr_set):
			assert_string_equal("ATOMIC set",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;
		case (pkt_nr_clr):
			assert_string_equal("ATOMIC clr",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;
		case (pkt_nw):
			assert_string_equal("NWRITE",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;
		case (pkt_nwr):
			assert_string_equal("NWRITE_R",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;
		case (pkt_nw_swap):
			assert_string_equal("ATOMIC swap",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;

		case (pkt_nw_cmp_swap):
			assert_string_equal("ATOMIC cmp swap",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;

		case (pkt_nw_tst_swap):
			assert_string_equal("ATOMIC tst swap",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;
		case (pkt_sw):
		case (pkt_fc):
			assert_string_equal("",
					DAR_pkt_trans_descr(&pkt_fields));
			break;
		case (pkt_mr):
			assert_string_equal("MtcRead",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;
		case (pkt_mw):
			assert_string_equal("MtcWrite",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;

		case (pkt_mrr):
			assert_string_equal("MtcReadResp",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;

		case (pkt_mwr):
			assert_string_equal("MtcWriteResp",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;

		case (pkt_pw):
			assert_string_equal("Port-Write",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;
		case (pkt_dstm):
		case (pkt_db):
		case (pkt_msg):
		case (pkt_resp):
			assert_string_equal("",
					DAR_pkt_trans_descr(&pkt_fields));
			break;
		case (pkt_resp_data):
			assert_string_equal("Resp with Data",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;
		case (pkt_msg_resp):
			assert_string_equal("Msg Response",
					(DAR_pkt_trans_descr(&pkt_fields)));
			break;
		default:
			assert_string_equal("",
					DAR_pkt_trans_descr(&pkt_fields));
			break;
		}
	}

	(void)state; // unused
}

static void DAR_pkt_resp_status_descr_test(void **state)
{
	DAR_pkt_fields_t pkt_fields;
	uint32_t i, j;

	for (i = pkt_type_min; i <= pkt_type_max; i++) {
		pkt_fields.pkt_type = (DAR_pkt_type)i;
		switch (i) {
		case pkt_mrr:
		case pkt_mwr:
		case pkt_resp:
		case pkt_msg_resp:
			continue;
		default:
			assert_string_equal("Unknown",
					DAR_pkt_resp_status_descr(&pkt_fields));
		}
	}

	for (i = pkt_type_min; i <= pkt_type_max; i++) {
		pkt_fields.pkt_type = (DAR_pkt_type)i;
		switch (i) {
		case pkt_mrr:
		case pkt_mwr:
		case pkt_resp:
		case pkt_msg_resp:
			break;
		default:
			continue;
		}

		for (j = rio_pkt_status_min; j <= rio_pkt_status_max; j++) {
			pkt_fields.log_rw.status = (rio_pkt_status)j;
			switch (j) {
			case pkt_done:
				assert_string_equal("Done",
						DAR_pkt_resp_status_descr(
								&pkt_fields));
				break;
			case pkt_retry:
				if ((pkt_resp == pkt_fields.pkt_type)
						|| (pkt_msg_resp
								== pkt_fields.pkt_type)) {
					assert_string_equal("Retry",
							DAR_pkt_resp_status_descr(
									&pkt_fields));
				} else {
					assert_string_equal("Unknown",
							DAR_pkt_resp_status_descr(
									&pkt_fields));
				}
				break;
			case pkt_err:
				assert_string_equal("Error",
						DAR_pkt_resp_status_descr(
								&pkt_fields));
				break;
			default:
				assert_string_equal("Unknown",
						DAR_pkt_resp_status_descr(
								&pkt_fields));
				break;
			}
		}
	}

	(void)state; // unused
}

int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
	cmocka_unit_test(assumptions),
	cmocka_unit_test(CS_fields_to_bytes_small_test),
	cmocka_unit_test(CS_fields_to_bytes_large_test),
	cmocka_unit_test(CS_bytes_to_fields_small_test),
	cmocka_unit_test(CS_bytes_to_fields_large_test),
	cmocka_unit_test(get_stype0_descr_test),
	cmocka_unit_test(get_stype0_PNA_cause_parm1_test),
	cmocka_unit_test(get_stype0_LR_port_status_parm1_test),
	cmocka_unit_test(get_stype1_descr_test),
	cmocka_unit_test(get_stype1_lreq_cmd_test),
	cmocka_unit_test(get_stype2_descr_test),
	cmocka_unit_test(DAR_pkt_fields_to_bytes_ftype_2_memsz_34_test),
	cmocka_unit_test(DAR_pkt_fields_to_bytes_ftype_2_memsz_50_test),
	cmocka_unit_test(DAR_pkt_fields_to_bytes_ftype_5_memsz_66_test),
	cmocka_unit_test(DAR_pkt_fields_to_bytes_ftype_6_memsz_32_test),
	cmocka_unit_test(DAR_pkt_fields_to_bytes_ftype_7_pkt_type_test),
	cmocka_unit_test(DAR_pkt_fields_to_bytes_ftype_8_memsz_21_test),
	cmocka_unit_test(DAR_pkt_fields_to_bytes_ftype_9_pkt_type_test),
	cmocka_unit_test(DAR_pkt_fields_to_bytes_ftype_10_pkt_type_test),
	cmocka_unit_test(DAR_pkt_fields_to_bytes_ftype_11_pkt_type_test),
	cmocka_unit_test(DAR_pkt_fields_to_bytes_ftype_13_pkt_type_test),
	cmocka_unit_test(DAR_pkt_fields_to_bytes_ftype_raw_pkt_type_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_invalid_tt_code_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_not_enough_bytes_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_bytes_0_and_1_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_src_dst_addr_size_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_2_pkt_type_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_2_memsz_34_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_2_memsz_50_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_5_pkt_type_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_5_memsz_66_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_6_pkt_type_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_6_memsz_32_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_7_pkt_type_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_8_memsz_21_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_9_pkt_type_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_10_pkt_type_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_11_pkt_type_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_13_pkt_type_test),
	cmocka_unit_test(DAR_pkt_bytes_to_fields_ftype_raw_pkt_type_test),
	cmocka_unit_test(DAR_pkt_ftype_descr_test),
	cmocka_unit_test(DAR_pkt_trans_descr_test),
	cmocka_unit_test(DAR_pkt_resp_status_descr_test),

	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#ifdef __cplusplus
}
#endif
