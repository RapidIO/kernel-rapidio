/*
 ****************************************************************************
 Copyright (c) 2015, Integrated Device Technology Inc.
 Copyright (c) 2015, RapidIO Trade Association
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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdarg.h>
#include <setjmp.h>
#include "cmocka.h"

#include "librsvdmem.h"
#include "librsvdmem_private.h"
#include "rio_mport_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

// If the test files do not exist provide a meaningful message and stop test
static void check_file_exists(const char *filename)
{
	struct stat buffer;

	if (0 != stat(filename, &buffer)) {
		fail_msg("File: %s does not exist, are you in the correct directory?", filename);
	}
}

static void rsvd_phys_mem_32_bit_test(void **state)
{
	const char *test_file = "test/addr_32bit.conf";
	uint64_t start_addr = 0;
	uint64_t size = 0;

	check_file_exists(test_file);
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM, &start_addr, &size));
	assert_int_equal(0x18000000, start_addr);
	assert_int_equal(0x04000000, size);

	(void)state; // unused
}

static void rsvd_phys_mem_rdmad_32_bit_test(void **state)
{
	const char *test_file = "test/addr_32bit.conf";
	uint64_t start_addr = 0;
	uint64_t size = 0;

	check_file_exists(test_file);
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_RDMAD, &start_addr, &size));
	assert_int_equal(0x18000000, start_addr);
	assert_int_equal(0x01000000, size);

	start_addr = 0;
	size = 0;
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_RSKTD, &start_addr, &size));
	assert_int_equal(0x19000000, start_addr);
	assert_int_equal(0x01000000, size);

	start_addr = 0;
	size = 0;
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_DMA_TUN, &start_addr, &size));
	assert_int_equal(0x1A000000, start_addr);
	assert_int_equal(0x01000000, size);

	start_addr = 0;
	size = 0;
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_FXFR, &start_addr, &size));
	assert_int_equal(0x1B000000, start_addr);
	assert_int_equal(0x01000000, size);

	(void)state; // unused
}

static void rsvd_phys_mem_rsktd_32_bit_test(void **state)
{
	const char *test_file = "test/addr_32bit.conf";
	uint64_t start_addr = 0;
	uint64_t size = 0;

	check_file_exists(test_file);
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_RSKTD, &start_addr, &size));
	assert_int_equal(0x19000000, start_addr);
	assert_int_equal(0x01000000, size);

	start_addr = 0;
	size = 0;
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_DMA_TUN, &start_addr, &size));
	assert_int_equal(0x1A000000, start_addr);
	assert_int_equal(0x01000000, size);

	start_addr = 0;
	size = 0;
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_FXFR, &start_addr, &size));
	assert_int_equal(0x1B000000, start_addr);
	assert_int_equal(0x01000000, size);

	(void)state; // unused
}

static void rsvd_phys_mem_dma_tun_32_bit_test(void **state)
{
	const char *test_file = "test/addr_32bit.conf";
	uint64_t start_addr = 0;
	uint64_t size = 0;

	check_file_exists(test_file);
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_DMA_TUN, &start_addr, &size));
	assert_int_equal(0x1A000000, start_addr);
	assert_int_equal(0x01000000, size);

	start_addr = 0;
	size = 0;
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_FXFR, &start_addr, &size));
	assert_int_equal(0x1B000000, start_addr);
	assert_int_equal(0x01000000, size);

	(void)state; // unused
}

static void rsvd_phys_mem_fxfr_32_bit_test(void **state)
{
	const char *test_file = "test/addr_32bit.conf";
	uint64_t start_addr = 0;
	uint64_t size = 0;

	check_file_exists(test_file);
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_FXFR, &start_addr, &size));
	assert_int_equal(0x1B000000, start_addr);
	assert_int_equal(0x01000000, size);

	(void)state; // unused
}

static void rsvd_phys_mem_64_bit_test(void **state)
{
	const char *test_file = "test/addr_64bit.conf";
	uint64_t start_addr = 0;
	uint64_t size = 0;

	check_file_exists(test_file);
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM, &start_addr, &size));
	assert_int_equal(0x1800000000000000, start_addr);
	assert_int_equal(0x0400000000000000, size);

	(void)state; // unused
}

static void rsvd_phys_mem_rdmad_64_bit_test(void **state)
{
	const char *test_file = "test/addr_64bit.conf";
	uint64_t start_addr = 0;
	uint64_t size = 0;

	check_file_exists(test_file);
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_RDMAD, &start_addr, &size));
	assert_int_equal(0x1800000000000000, start_addr);
	assert_int_equal(0x0100000000000000, size);

	(void)state; // unused
}

static void rsvd_phys_mem_rsktd_64_bit_test(void **state)
{
	const char *test_file = "test/addr_64bit.conf";
	uint64_t start_addr = 0;
	uint64_t size = 0;

	check_file_exists(test_file);
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_RSKTD, &start_addr, &size));
	assert_int_equal(0x1900000000000000, start_addr);
	assert_int_equal(0x0100000000000000, size);

	(void)state; // unused
}

static void rsvd_phys_mem_dma_tun_64_bit_test(void **state)
{
	const char *test_file = "test/addr_64bit.conf";
	uint64_t start_addr = 0;
	uint64_t size = 0;

	check_file_exists(test_file);
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_DMA_TUN, &start_addr, &size));
	assert_int_equal(0x1A00000000000000, start_addr);
	assert_int_equal(0x0100000000000000, size);

	(void)state; // unused
}

static void rsvd_phys_mem_fxfr_64_bit_test(void **state)
{
	const char *test_file = "test/addr_64bit.conf";
	uint64_t start_addr = 0;
	uint64_t size = 0;

	check_file_exists(test_file);
	assert_int_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_FXFR, &start_addr, &size));
	assert_int_equal(0x1B00000000000000, start_addr);
	assert_int_equal(0x0100000000000000, size);

	(void)state; // unused
}

static void empty_file_test(void **state)
{
	const char *test_file = "test/empty.conf";
	uint64_t start_addr = 0xcafebabe;
	uint64_t size = 0xdeadbeef;

	check_file_exists(test_file);
	assert_int_not_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM, &start_addr, &size));
	assert_int_equal(RIO_MAP_ANY_ADDR, start_addr);
	assert_int_equal(0, size);

	(void)state; // unused
}

static void blank_line_test(void **state)
{
	const char *test_file = "test/blankline.conf";
	uint64_t start_addr = 0xcafebabe;
	uint64_t size = 0xdeadbeef;

	check_file_exists(test_file);
	assert_int_not_equal(0,
			get_phys_mem(test_file, RSVD_PHYS_MEM_RDMAD, &start_addr, &size));
	assert_int_equal(RIO_MAP_ANY_ADDR, start_addr);
	assert_int_equal(0, size);

	(void)state; // unused
}

static void missing_token_test(void **state)
{
	const char *test_file = "test/badformat.conf";
	uint64_t start_addr = 0xcafebabe;
	uint64_t size = 0xdeadbeef;

	check_file_exists(test_file);
	assert_int_not_equal(0,
			get_phys_mem((const char * )test_file,
					(char * )"TOKEN_NOT_FOUND", &start_addr,
					&size));
	assert_int_equal(RIO_MAP_ANY_ADDR, start_addr);
	assert_int_equal(0, size);

	(void)state; // unused
}

static void missing_address_and_size_test(void **state)
{
	const char *test_file = "test/badformat.conf";
	uint64_t start_addr = 0xcafebabe;
	uint64_t size = 0xdeadbeef;

	check_file_exists(test_file);
	assert_int_not_equal(0,
			get_phys_mem((const char * )test_file,
					(char * )"NO_ADDR_SZ", &start_addr,
					&size));
	assert_int_equal(RIO_MAP_ANY_ADDR, start_addr);
	assert_int_equal(0, size);

	(void)state; // unused
}

static void missing_size_test(void **state)
{
	const char *test_file = "test/badformat.conf";
	uint64_t start_addr = 0xcafebabe;
	uint64_t size = 0xdeadbeef;

	check_file_exists(test_file);
	assert_int_not_equal(0,
			get_phys_mem((const char * )test_file, (char * )"NO_SZ",
					&start_addr, &size));
	assert_int_equal(RIO_MAP_ANY_ADDR, start_addr);
	assert_int_equal(0, size);

	(void)state; // unused
}

static void invalid_line_format_test(void **state)
{
	const char *test_file = "test/badformat.conf";
	uint64_t start_addr = 0;
	uint64_t size = 0;

	check_file_exists(test_file);
	assert_int_equal(0,
			get_phys_mem((const char * )test_file,
					(char * )"STUFF_BEFORE", &start_addr,
					&size));
	assert_int_equal(0x1800000000000000, start_addr);
	assert_int_equal(0x010000000000000, size);

	(void)state; // unused
}

static void misaligned_address_test(void **state)
{
	const char *test_file = "test/badformat.conf";
	uint64_t start_addr;
	uint64_t size;
	int i;

	check_file_exists(test_file);
	for (i = 1; i <= 3; i++) {
		char keyword[30];
		memset(keyword, 0, 30);
		snprintf(keyword, 30, "MISALIGNED%d", i);

		start_addr = 0;
		size = 0;
		errno = 0;
		assert_int_not_equal(0,
				get_phys_mem((const char * )test_file,
						(char * )keyword, &start_addr,
						&size));
		assert_int_equal(EDOM, errno);
		assert_int_equal(0x0100000000000000, size);

		switch (i) {
		case 1:
			assert_int_equal(0x1900000000000001, start_addr);
			break;
		case 2:
			assert_int_equal(0x1980000000000000, start_addr);
			break;
		case 3:
			assert_int_equal(0x1900008000000000, start_addr);
			break;
		default:
			fail_msg("Invalid loop index %d", i);
		}

	}

	(void)state; // unused
}

static void illegal_address_characters_test(void **state)
{
	const char *test_file = "test/badformat.conf";
	uint64_t start_addr;
	uint64_t size;
	int i;

	check_file_exists(test_file);
	for (i = 1; i <= 6; i++) {
		char keyword[30];
		memset(keyword, 0, 30);
		snprintf(keyword, 30, "ILLEGAL_CHARS%d", i);

		start_addr = 0;
		size = 0;
		errno = 0;
		assert_int_not_equal(0,
				get_phys_mem((const char * )test_file,
						(char * )keyword, &start_addr,
						&size));
		assert_int_equal(EDOM, errno);
	}

	(void)state; // unused
}

int main(int argc, char *argv[])
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
	cmocka_unit_test(rsvd_phys_mem_32_bit_test),
	cmocka_unit_test(rsvd_phys_mem_rdmad_32_bit_test),
	cmocka_unit_test(rsvd_phys_mem_rsktd_32_bit_test),
	cmocka_unit_test(rsvd_phys_mem_dma_tun_32_bit_test),
	cmocka_unit_test(rsvd_phys_mem_fxfr_32_bit_test),
	cmocka_unit_test(rsvd_phys_mem_64_bit_test),
	cmocka_unit_test(rsvd_phys_mem_rdmad_64_bit_test),
	cmocka_unit_test(rsvd_phys_mem_rsktd_64_bit_test),
	cmocka_unit_test(rsvd_phys_mem_dma_tun_64_bit_test),
	cmocka_unit_test(rsvd_phys_mem_fxfr_64_bit_test),
	cmocka_unit_test(empty_file_test),
	cmocka_unit_test(blank_line_test),
	cmocka_unit_test(missing_token_test),
	cmocka_unit_test(missing_address_and_size_test),
	cmocka_unit_test(missing_size_test),
	cmocka_unit_test(invalid_line_format_test),
	cmocka_unit_test(misaligned_address_test),
	cmocka_unit_test(illegal_address_characters_test), };
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#ifdef __cplusplus
}
#endif

