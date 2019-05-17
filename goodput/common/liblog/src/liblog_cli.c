/*
 ****************************************************************************
 Copyright (c) 2015, Integrated Device Technology Inc.
 Copyright (c) 2015, RapidIO Trade Association
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
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
#include "liblog.h"
#include "libcli.h"
#include "tok_parse.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct cli_cmd LogLevel;
extern struct cli_cmd DispLevel;

extern unsigned g_level;
extern unsigned g_disp_level;

const char *level_strings[] = {"NOLOG", "NOLOG", "CRIT", "ERR", "WARN", "HIGH",
		"INFO", "DBG", };

#define LOG_STR(x) ((x>RDMA_LL_DBG)?level_strings[RDMA_LL_DBG]:level_strings[x])

int LogLevelCmd(struct cli_env *env, int argc, char **argv)
{
	uint32_t temp;
	if (argc) {
		if (tok_parse_log_level(argv[0], &temp, 0)) {
			LOGMSG(env, TOK_ERR_LOG_LEVEL_MSG_FMT);
			return 0;
		}

		if (temp > RDMA_LL) {
			temp = RDMA_LL;
		}
		g_level = (unsigned)temp;
	}

	LOGMSG(env, "\nCompiled log level %d: %s\n", RDMA_LL, LOG_STR(RDMA_LL));
	LOGMSG(env, "Current log level %d: %s\n", g_level, LOG_STR(g_level));

	return 0;
}

struct cli_cmd LogLevel =
		{"levelog", 3, 0, "Display or set current log level.",
				"{<level>}\n"
						"1: Logging disabled\n"
						"2: Critical, Failure critical to correct operation\n"
						"3: Error   , Error occurred, may not affect operation\n"
						"4: Warning , Something a bit strange occurred\n"
						"5: High priority info, mainly thread synchronization info\n"
						"6: Info    , Informational, trace operation of the system\n"
						"7: Debug   , Detailed information about system operation\n"
						"   NOTE: Levels above \"Error\" impact performance significantly.\n"
						"   NOTE: Maximum level available is set through the \"LOG_LEVEL\"\n"
						"         Makefile compile option.",
				LogLevelCmd,
				ATTR_NONE};

/**
 * Sets the debug level for messages displayed on the screen.
 */
int DispLevelCmd(struct cli_env *env, int argc, char **argv)
{
	uint32_t temp;
	if (argc) {
		if (tok_parse_log_level(argv[0], &temp, 0)) {
			LOGMSG(env, TOK_ERR_LOG_LEVEL_MSG_FMT);
			return 0;
		}

		if (temp > RDMA_LL) {
			temp = RDMA_LL;
		}
		g_disp_level = (unsigned)temp;
	}

	LOGMSG(env, "\nCompiled log level %d: %s\n", RDMA_LL, LOG_STR(RDMA_LL));
	LOGMSG(env, "Current DISPLAY log level %d: %s\n", g_disp_level,
			LOG_STR(g_disp_level));

	return 0;
}

struct cli_cmd DispLevel =
		{"dlevelog", 3, 0, "Display or set current DISPLAY level.",
				"{<level>}\n"
						"1: Logging disabled\n"
						"2: Critical, Failure critical to correct operation\n"
						"3: Error   , Error occurred, may not affect operation\n"
						"4: Warning , Something a bit strange occurred\n"
						"5: High priority info, mainly thread synchronization info\n"
						"6: Info    , Informational, trace operation of the system\n"
						"7: Debug   , Detailed information about system operation\n"
						"   NOTE: Levels above \"Error\" impact performance significantly.\n"
						"   NOTE: Maximum level available is set through the \"LOG_LEVEL\"\n"
						"         Makefile compile option.",
				DispLevelCmd,
				ATTR_NONE};

int log_dump_cmd_f(struct cli_env *env, int argc, char **argv)
{
	(void)env;
	(void)argc;
	(void)argv;
	rdma_log_dump();
	return 0;
} /* log_dump_cmd_f() */

struct cli_cmd log_dump_cmd = {"dlog", 4, 0, "Dump log to screen.", "{None}\n"
		"Dumps log to screen.\n", log_dump_cmd_f,
ATTR_NONE};

struct cli_cmd *liblog_cmds[] = {&LogLevel, &DispLevel, &log_dump_cmd};

void liblog_bind_cli_cmds(void)
{
	add_commands_to_cmd_db(sizeof(liblog_cmds) / sizeof(liblog_cmds[0]),
			liblog_cmds);
}

#ifdef __cplusplus
}
#endif
