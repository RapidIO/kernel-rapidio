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
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <errno.h>

#include <stdarg.h>
#include <setjmp.h>
#include "cmocka.h"

#include "libcli.h"

// Replace LOGMSG macro with LOGMSG test function
#undef LOGMSG
#ifdef __cplusplus
extern "C" {
#endif
void LOG_MSG(struct cli_env *env, char *format, ...);
#define LOGMSG(env, format, ...) LOG_MSG(env, (char *)format, ##__VA_ARGS__)
#ifdef __cplusplus
}
#endif

#include "../src/cli_cmd_db.c"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_OUTPUT (2 * MAX_CMDS)

static char output[MAX_OUTPUT][100];
static int out_idx;
static bool output_overflow;

static struct cli_env *unused_env;

void LOG_MSG(struct cli_env *env, char *format, ...)
{
	va_list args;
	int rc;

	if (0) {
		unused_env = env; // unused
	}

	va_start(args, format);
	if (out_idx == MAX_OUTPUT - 1) {
		output_overflow = true;
	}

	rc = vsnprintf(&output[out_idx][0], sizeof(output[0]),
			(const char *)format, args);
	assert_return_code(rc, 0);

	out_idx++;
	if (out_idx >= MAX_OUTPUT) {
		out_idx = MAX_OUTPUT - 1;
	}
	va_end(args);
}

static void setup_output(void)
{
	out_idx = 0;
	output_overflow = false;
}

#define NUM_CMDS_AFTER_INIT 1
#define NUM_FREE_CMDS (MAX_CMDS - NUM_CMDS_AFTER_INIT)

static char cmd_names[MAX_CMDS][100];
static char short_help[MAX_CMDS][100];
static char long_help[MAX_CMDS][100];

static char *nomatch = (char*)"nomatch";

static struct cli_cmd test_cmds[MAX_CMDS];
static struct cli_cmd *test_cmd_array[MAX_CMDS];

static int test_func(struct cli_env *env, int argc, char **argv)
{
	if (0) {
		unused_env = env;
		*argv = NULL;
	}
	return argc;
}

static void setup_commands(void)
{
	int i;
	char *c_pfix = (char *)"CMD_";
	char *s_pfix = (char *)"SHORT_HELP_";
	char *l_pfix = (char *)"LONG_HELP_";

	// Formats below assume MAX_CMDS maximum is 999
	assert_in_range(MAX_CMDS, 1, 999);
	for (i = 0; i < MAX_CMDS; i++) {
		snprintf(&cmd_names[i][0], sizeof(cmd_names[0]), "%s%3d",
				c_pfix, i);
		snprintf(&short_help[i][0], sizeof(short_help[0]), "%s%3d",
				s_pfix, i);
		snprintf(&long_help[i][0], sizeof(long_help[0]), "%s%3d",
				l_pfix, i);
		memset(&test_cmds[i], 0, sizeof(test_cmds[0]));
		test_cmds[i].name = cmd_names[i];
		test_cmds[i].min_match = strlen(cmd_names[i]);
		test_cmds[i].min_parms = i % 4;
		test_cmds[i].shortHelp = &short_help[i][0];
		test_cmds[i].longHelp = &long_help[i][0];
		test_cmds[i].func = test_func;
		test_cmds[i].attributes = 0;
		test_cmd_array[i] = &test_cmds[i];
	}
}

static void assumptions_test(void **state)
{
	assert_int_not_equal(0, MAX_CMDS);
	assert_int_not_equal(0, NUM_CMDS_AFTER_INIT);
	assert_int_not_equal(MAX_CMDS, NUM_CMDS_AFTER_INIT);
	assert_int_equal(1, MIN(1,2));
	assert_int_equal(2, MIN(3,2));
	assert_int_equal(2, MIN(2,2));

	(void)state; // unused
}

static void cli_print_help_success_test(void **state)
{
	char test_buffer[100];
	struct cli_env *unused = NULL;

	setup_output();
	assert_int_equal(0, cli_print_help(unused, &test_cmds[0]));
	snprintf(test_buffer, sizeof(test_buffer), "\n%s : %s\n",
			test_cmds[0].name, test_cmds[0].longHelp);
	assert_string_equal(test_buffer, output[0]);

	(void)state; // unused
}

static void init_cmd_db_success_test(void **state)
{
	struct cli_cmd *cmd_rc;

	num_valid_cmds = 1000;
	memset(cmds, 0xFF, sizeof(cmds));

	init_cmd_db();
	assert_int_equal(NUM_CMDS_AFTER_INIT, num_valid_cmds);
	assert_ptr_equal(cmds[0], &CLIHelp);
	assert_int_equal(-1, find_cmd(nomatch, &cmd_rc));

	(void)state; // unused
}

static void add_commands_to_cmd_db_test(void **state)
{
	int i, j;

	setup_commands();
	for (i = 1; i <= NUM_FREE_CMDS; i++) {
		init_cmd_db();
		assert_int_equal(0, add_commands_to_cmd_db(i, test_cmd_array));

		assert_int_equal(i + NUM_CMDS_AFTER_INIT, num_valid_cmds);
		for (j = 0; j < i; j++) {
			assert_ptr_equal(cmds[j + NUM_CMDS_AFTER_INIT],
					&test_cmds[j]);
		}
	}

	assert_int_equal(num_valid_cmds, MAX_CMDS);
	// command array should be at capacity.  Adding 0 commands should pass
	assert_int_equal(0, add_commands_to_cmd_db(0, test_cmd_array));
	// Adding one more command should fail
	assert_int_not_equal(0, add_commands_to_cmd_db(1, test_cmd_array));

	(void)state; // unused
}

static void find_cmd_exact_match_test(void **state)
{
	int i;

	setup_commands();

	init_cmd_db();
	assert_int_equal(0,
			add_commands_to_cmd_db(NUM_FREE_CMDS, test_cmd_array));
	for (i = 0; i < MAX_CMDS; i++) {
		struct cli_cmd *cmd = NULL;

		assert_int_equal(CMD_FOUND,
				find_cmd((char * )cmds[i]->name, &cmd));
		assert_ptr_equal(cmd, cmds[i]);
	}

	(void)state; // unused
}

static void find_cmd_partial_match_test(void **state)
{
	int i;

	setup_commands();

	init_cmd_db();
	assert_int_equal(0,
			add_commands_to_cmd_db(NUM_FREE_CMDS, test_cmd_array));
	for (i = NUM_CMDS_AFTER_INIT; i < MAX_CMDS; i++) {
		struct cli_cmd *cmd = NULL;
		char cmd_name[100];

		// Get inexact string length
		memset(cmd_name, 0, sizeof(cmd_name));
		memcpy(cmd_name, cmds[i]->name, strlen(cmds[i]->name) - 1);

		assert_int_equal(CMD_DUP_FOUND, find_cmd(cmd_name, &cmd));
	}

	(void)state; // unused
}

static void find_cmd_no_match_test(void **state)
{
	unsigned int i, j;
	const int num_names = 3;
	char base_name[100];
	char test_name[100];

	setup_commands();

	init_cmd_db();
	assert_int_equal(0,
			add_commands_to_cmd_db(NUM_FREE_CMDS, test_cmd_array));
	for (i = 0; i < num_names; i++) {
		memset(base_name, 0, sizeof(base_name));
		memcpy(base_name, cmds[i]->name, strlen(cmds[i]->name));
		assert_in_range(strlen(base_name) + 2, 1,
				sizeof(base_name) - 1);

		for (j = 0; j <= strlen(base_name); j++) {
			struct cli_cmd *cmd = &CLIHelp;
			memcpy(test_name, base_name, sizeof(test_name));
			test_name[j] = '&';

			if (strlen(base_name) == j) {
				assert_int_equal(0, find_cmd(test_name, &cmd));
				assert_ptr_equal(cmd, cmds[i]);
			} else {
				assert_int_equal(-1, find_cmd(test_name, &cmd));
				assert_ptr_equal(cmd, NULL);
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
	cmocka_unit_test(assumptions_test),
	cmocka_unit_test(cli_print_help_success_test),
	cmocka_unit_test(init_cmd_db_success_test),
	cmocka_unit_test(add_commands_to_cmd_db_test),
	cmocka_unit_test(find_cmd_exact_match_test),
	cmocka_unit_test(find_cmd_partial_match_test),
	cmocka_unit_test(find_cmd_no_match_test), };
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#ifdef __cplusplus
}
#endif
