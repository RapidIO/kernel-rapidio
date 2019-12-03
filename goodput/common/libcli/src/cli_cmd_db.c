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

#include <string.h>
#include "libcli.h"

#define MAX_CMDS 120
#define MIN(a, b) ((a < b)?a:b)

#ifdef __cplusplus
extern "C" {
#endif

int num_valid_cmds;
struct cli_cmd *cmds[MAX_CMDS];

int cli_print_help(struct cli_env *env, struct cli_cmd *cmd)
{
	LOGMSG(env, "\n%s : %s\n", cmd->name, cmd->longHelp);
	return 0;
}

int add_commands_to_cmd_db(int num_cmds, struct cli_cmd **cmd_list)
{
	int idx = 0;

	while ((num_valid_cmds < MAX_CMDS) && (idx < num_cmds)) {
		cmds[num_valid_cmds] = cmd_list[idx];
		idx++;
		num_valid_cmds++;
	}
	return (idx != num_cmds);
}

/******************************************************************************
 *  FUNCTION: find_cmd()
 *
 *  DESCRIPTION:
 *    Parse the command and return the index of the command, or -1 if command
 *    not found.
 *
 *
 *  PARAMETERS:
 *      null terminated string containing the command to parse.
 *
 *
 *  return VALUE:
 *         0 if command found.  *cmd is valid.
 *        -1 (command not found)  *cmd is invalid.
 *        -2 (ambiguous command)  *cmd is invalid.
 *
 *****************************************************************************/

#define CMD_NOT_FOUND -1
#define CMD_DUP_FOUND -2
#define CMD_FOUND      0

int find_cmd(char *cmd_name, struct cli_cmd **cmd)
{
	int i;
	size_t numChars;
	int foundIndex = CMD_NOT_FOUND;

	*cmd = NULL;

	for (i = 0; i < num_valid_cmds; i++) {
		if (NULL == cmds[i]->name ) {
			continue;
		}

		numChars = MIN(strlen(cmd_name), strlen(cmds[i]->name));
		if (strncmp(cmd_name, cmds[i]->name, numChars) == 0) {
			/* If an exact enough match has occurred,
			 * select that cmd
			 */
			if (numChars >= cmds[i]->min_match) {
				*cmd = cmds[i];
				foundIndex = CMD_FOUND;
				break;
			}

			/* If previously found at least a partial match, fail */
			if (foundIndex != CMD_NOT_FOUND) {
				foundIndex = CMD_DUP_FOUND;
				break;
			}

			/* Have at least a partial match */
			*cmd = cmds[i];
			foundIndex = 0;

			/* if we find an exact match in size,
			 * don't bother looking for more
			 */
			if (!strcmp(cmd_name, cmds[i]->name)) {
				break;
			}
		}
	}

	return foundIndex;
}

int CLIHelpCmd(struct cli_env *env, int argc, char **argv)
{
	int idx;
	struct cli_cmd *cmd_ptr;

	if (!argc) {
		LOGMSG(env, "\nCommands:\n");

		/* display list of commands in the database */
		for (idx = 0; idx < num_valid_cmds; idx++) {
			LOGMSG(env, "%10s : %s\n", cmds[idx]->name,
					cmds[idx]->shortHelp);
		}
	} else {
		idx = find_cmd(argv[0], &cmd_ptr);
		switch (idx) {
		case CMD_FOUND:
			cli_print_help(env, cmd_ptr);
			break;
		case CMD_NOT_FOUND:
			LOGMSG(env, "\nCommand \"%s\" not found.\n", argv[0]);
			break;
		case CMD_DUP_FOUND:
			LOGMSG(env, "\n\"%s\" ambiguous, use a longer name.\n",
					argv[0]);
			break;
		default:
			/* Should never get here... */
			LOGMSG(env, "\nInvalid return from find_cmd: 0x%08x\n",
					idx);
			LOGMSG(env, "Help command is broken...\n");
			break;
		}
	}

	return 0;
}

struct cli_cmd CLIHelp =
		{"?", 1, 0,
				"Help command, lists all commands or detailed help for one command",
				"<cmd>\n"
						"If no command is entered, briefly lists all available commands.\n"
						"If a command name is entered, gives detailed help for that command.\n",
				CLIHelpCmd,
				ATTR_NONE};

int init_cmd_db(void)
{
	struct cli_cmd *help_ptr = &CLIHelp;

	num_valid_cmds = 0;
	add_commands_to_cmd_db(1, &help_ptr);

	return 0;
}

#ifdef __cplusplus
}
#endif
