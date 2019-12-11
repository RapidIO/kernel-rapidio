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
#ifdef USE_READLINE
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "string_util.h"
#include "cli_cmd_db.h"
#include "cli_cmd_line.h"
#include "cli_rem_conn.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void (*cons_cleanup)(struct cli_env *env);

void splashScreen(struct cli_env *env, char *app_name)
{
	char *dash =
			(char *)"----------------------------------------------------------";
	char *dash_ends =
			(char *)"---                                                    ---";
	int dash_len = strlen(dash) - 6;
	int app_name_len = (int)((dash_len + strlen(app_name)) / 2);
	int app_name_rem = dash_len - app_name_len;
	LOGMSG(env, "%s\n", dash);
	LOGMSG(env, "%s\n", dash_ends);
	LOGMSG(env, "---%*s%*s---\n", app_name_len, app_name, app_name_rem, "");
	LOGMSG(env, "%s\n", dash_ends);
	LOGMSG(env, "%s\n", dash);
	LOGMSG(env, "---         Version: %2s.%2s (%11s-%8s)      ---\n",
			CLI_VERSION_YR, CLI_VERSION_MO, __DATE__, __TIME__);
	LOGMSG(env, "%s\n", dash);
	LOGMSG(env, "            RapidIO Trade Association\n");
	LOGMSG(env, "            Copyright 2016\n");
}

const char *delimiter = " ,\t\n"; /* Input token delimiter */

void logMsg(struct cli_env *env)
{
	uint8_t use_skt = (env->sess_socket >= 0) ? TRUE : FALSE;

	if (use_skt) {
		int n = write(env->sess_socket, env->output,
				strlen(env->output));
		unsigned int bytes_sent = n;

		while ((n >= 0) && (bytes_sent < strlen(env->output))) {
			n = write(env->sess_socket, env->output,
					strlen(env->output));
			bytes_sent += n;
		}
		memset(env->output, 0, sizeof(env->output));
	} else {
		printf("%s", env->output);
	}

	if (NULL != env->fout) { /* is logging enabled? */
		fprintf(env->fout, "%s", env->output);
	}
}

int process_command(struct cli_env *env, char *input)
{
	char *cmd;
	int rc;
	struct cli_cmd* cmd_p = NULL;
	int argc = 0;
	char* argv[30] = {NULL};
	int exitStat = 0;
	char* status = NULL;

	if (NULL != env->fout) {
		fprintf((FILE *)env->fout, "%s", input); /* Log command file */
	}

	cmd = strtok_r(input, delimiter, &status);/* Tokenize input array */

	if ((NULL != cmd) && (cmd[0] != '/') && (cmd[0] != '\n')
			&& (cmd[0] != '\r')) {
		rc = find_cmd(cmd, &cmd_p);

		if (rc == -1) {
			LOGMSG(env,
					"Unknown command: Type '?' for a list of commands\n");
		} else if (rc == -2) {
			LOGMSG(env,
					"Ambiguous command: Type '?' for command usage\n");
		} else if (!rc && (cmd_p->func != NULL)) {
			argc = 0;
			while ((argv[argc] = strtok_r(NULL, delimiter, &status))
					!= NULL) {
				argc++;
			}

			if (argc < cmd_p->min_parms) {
				LOGMSG(env,
						"FAILED: Need %d parameters for command \"%s\"",
						cmd_p->min_parms, cmd_p->name);
				LOGMSG(env, "\n%s not executed.\nHelp:\n",
						cmd_p->name);
				cli_print_help(env, cmd_p);
			} else {
				exitStat = (cmd_p->func(env, argc, argv));
				/* store the command */
				if (cmd_p->attributes & ATTR_RPT) {
					env->cmd_prev = cmd_p;
				} else {
					env->cmd_prev = NULL;
				}
			}
		}
	} else {
		/* empty string passed to CLI */
		if (NULL != env->cmd_prev) {
			exitStat = env->cmd_prev->func(env, 0, NULL);
		}
	}
	return exitStat;
}

int process_skt_inp(struct cli_env *env, char *cmd, char *inp, int *len)
{
	char *cmd_off;
	char *prev_cmd_off = inp;
	int errorStat = 0;

	cmd_off = strchr(inp, '\n');
	while ((NULL != cmd_off) && !errorStat) {
		memcpy(&cmd[*len], prev_cmd_off, cmd_off - prev_cmd_off);
		errorStat |= process_command(env, cmd);
		memset(cmd, 0, BUFLEN);
		prev_cmd_off = cmd_off + 1;
		cmd_off = strchr(prev_cmd_off, '\n');
	}

	*len = strlen(prev_cmd_off);
	if (*len > 0) {
		SAFE_STRNCPY(cmd, prev_cmd_off, *len);
	}
	return errorStat;
}
;

int cli_terminal(struct cli_env *env)
{
	/* State variables */
	char *input;
#ifndef USE_READLINE
	char input_buf[BUFLEN];
	int lastChar = 0;
#endif

	char inp[BUFLEN], cmd[BUFLEN];
	int cmdlen = 0;
	unsigned int errorStat = 0;
	int use_skt = (env->sess_socket >= 0) ? TRUE : FALSE;
	int one = 1;

	if (use_skt) {
		setsockopt(env->sess_socket, IPPROTO_TCP, TCP_NODELAY, &one,
				sizeof(one));
	}

	memset(cmd, 0, sizeof(cmd));
	while (!errorStat) {
		env->output[0] = '\0';

		if (use_skt) {
			char buff[BUFLEN] = {0};

			snprintf(buff, sizeof(buff), "%s\n", env->prompt);
			lastChar = write(env->sess_socket, buff, strlen(buff));
			if (lastChar < 0) {
				errorStat = 2;
				goto exit;
			}

			memset(inp, 0, sizeof(inp));
			lastChar = read(env->sess_socket, inp, sizeof(inp));
			if (lastChar < 0) {
				errorStat = 2;
				goto exit;
			}

			inp[BUFLEN - 1] = '\0';
			errorStat = process_skt_inp(env, cmd, inp, &cmdlen);
		} else {
			fflush(stdin);
#ifdef USE_READLINE
			/* Readline returns a buffer that it allocates.
			 * Free previous buffer if it exists.
			 */
			if (NULL != input) {
				free(input);
				input = NULL;
			}

			input = readline(env->prompt);

			if (input && *input) {
				add_history(input);
			}
#else
			/* when using mingw, command line parsing seems to be
			 * built into fgets.  No need for readline in this case
			 */
			input = input_buf;
			input[0] = '\0';
			printf("%s", env->prompt); /* Display prompt */
			if (NULL == fgets(input, BUFLEN - 1, stdin)) {
				goto exit;
			}

			/* have to remove the \n at the end of the line
			 * when using fgets
			 */
			lastChar = strlen(input);
			if (input[lastChar - 1] == '\n') {
				input[lastChar - 1] = '\0';
			}
#endif
			errorStat = process_command(env, input);

#ifdef USE_READLINE
			free(input);
			input = NULL;
#endif
		}
	}

exit:
	return errorStat; /* no error */
}

/******************************************************************************
 *  FUNCTION: cli_script()
 *
 *  DESCRIPTION:
 *		main UI read and execute loop from the terminal.
 *
 *  PARAMETERS:
 *      None
 *
 *
 *  return VALUE:
 *      0 = no error logged
 *      1 = error logged
 *
 *****************************************************************************/

int cli_script(struct cli_env *env, char *script, int verbose)
{
	FILE *fin;
	int end = FALSE; /* End of command processing loop? */
	char input[BUFLEN] = {0}; /* Input buffer */

	unsigned int errorStat = 0;
	struct cli_env temp_env = *env;

	temp_env.script = script;

	fin = fopen(script, "re");
	if (NULL == fin) {
		sprintf(&temp_env.output[0],
				"\t/*Error: cannot open file named \"%s\"/\n",
				script);
		logMsg(&temp_env);
		end = TRUE;
	}

	while (!end && !errorStat) {
		/* Initialize all local variables and flush streams */
		fflush(stdin);

		if (NULL == fgets(input, BUFLEN - 1, fin)) {
			end = TRUE;
			break;
		} else if (verbose) {
			sprintf(&temp_env.output[0], "%s\n", input);
			logMsg(&temp_env);
		}

		errorStat = process_command(&temp_env, input);
	}

	if (NULL != fin) {
		fclose(fin);
	}
	return errorStat;
}

int CLIDebugCmd(struct cli_env *env, int argc, char **argv)
{
	if (argc) {
		env->DebugLevel = getHex(argv[0], 0);
	}
	LOGMSG(env, "Debug level: %d\n", env->DebugLevel);
	return 0;
}

struct cli_cmd CLIDebug =
		{"debug", 1, 0, "Set/query CLI debug output level",
				"<debug level>\n"
						"<debug level> is the new debug level.\n"
						"If <debug_level> is omitted, print the current debug level.\n",
				CLIDebugCmd,
				ATTR_NONE};

int CLIOpenLogFileCmd(struct cli_env *env, int argc, char **argv);

struct cli_cmd CLIOpenLogFile =
		{"log", 3, 1, "Open a log file",
				"<file name> <verbose>\n"
						"<file name> is the log file name.  No spaces are allowed in the name.\n"
						"<verbose> is 0 or non-zero, and controls how much detail is\n"
						"printed to the log file.\n",
				CLIOpenLogFileCmd,
				ATTR_NONE};

int CLIOpenLogFileCmd(struct cli_env *env, int argc, char **argv)
{
	int errorStat = 0;

	if (argc > 1) {
		LOGMSG(env, "FAILED: Extra parms ignored: \"%s\"\n", argv[1]);
		cli_print_help(env, &CLIOpenLogFile);
		goto exit;
	}

	if (NULL != env->fout) {
		fclose(env->fout);
		env->fout = NULL;
	}

	/* Make sure cariage return is not included in string,
	 * otherwise fopen will fail
	 */
	if (argv[0][strlen(argv[0]) - 1] == 0xD) {
		argv[0][strlen(argv[0]) - 1] = '\0';
	}

	env->fout = fopen(argv[0], "we"); /* Open log file for writing */
	if (NULL == env->fout) {
		LOGMSG(env,
				"\t/*FAILED: Log file \"%s\" could not be opened*/\n",
				argv[0]);
		errorStat = 1;
	} else {
		LOGMSG(env, "\t/*Log file %s opened*/\n", argv[0]);
	}

exit:
	return errorStat;
}

int CLICloseLogFileCmd(struct cli_env *env, int argc, char **argv);

struct cli_cmd CLICloseLogFile =
		{"close", 1, 0, "Close a currently open log file",
				"No Parameters\n"
						"Close the currently open log file, if one exists.",
				CLICloseLogFileCmd,
				ATTR_NONE};

int CLICloseLogFileCmd(struct cli_env *env, int argc, char **argv)
{
	int errorStat = 0;

	if (argc > 1) {
		LOGMSG(env, "FAILED: Extra parms ignored: \"%s\"\n", argv[1]);
		cli_print_help(env, &CLICloseLogFile);
		goto exit;
	}

	if (NULL == env->fout) {
		LOGMSG(env, "\t/*FAILED: No log file to close */\n");
	} else {
		LOGMSG(env, "\t/*Log file closed*/\n");
		fclose(env->fout); /* Finish writes to file before closing */
		env->fout = NULL;
	}

exit:
	return errorStat;
}

#define SCRIPT_PATH_SIZE 1024
#define SCRIPT_PATH_LEN (SCRIPT_PATH_SIZE-1)

char script_path[SCRIPT_PATH_SIZE];

int CLIScriptCmd(struct cli_env *env, int argc, char **argv)
{
	int errorStat = 0;
	int verbose = 0;
	char full_script_name[2 * SCRIPT_PATH_SIZE] = {0};

	if (argc > 1) {
		verbose = getHex(argv[0], 0);
	}

	LOGMSG(env, "\tPrefix: \"%s\"\n", script_path);
	LOGMSG(env, "\tScript: \"%s\"\n", argv[0]);

	memset(full_script_name, 0, 2 * SCRIPT_PATH_SIZE);
	if (argv[0][0] == '.' || argv[0][0] == '/' || argv[0][0] == '\\') {
		snprintf(full_script_name, 2 * SCRIPT_PATH_SIZE - 1, "%s",
				argv[0]);
	} else {
		snprintf(full_script_name, 2 * SCRIPT_PATH_SIZE - 1, "%s%s",
				script_path, argv[0]);
	}

	LOGMSG(env, "\tFile  : \"%s\"\n", full_script_name);
	errorStat = cli_script(env, full_script_name, verbose);

	LOGMSG(env, "script %s completed, status %x\n", argv[0], errorStat);

	return errorStat;
}

struct cli_cmd CLIScript =
		{"script", 3, 1, "execute commands in a script file",
				"<filename> <verbose>\n"
						"<filename> : File name of the script\n"
						"<verbose>  : zero/non-zero value to control amount of output.",
				CLIScriptCmd,
				ATTR_NONE};
struct cli_cmd CLIDotScript =
		{".", 3, 1, "dot-include - execute commands in a script file",
				"<filename> <verbose>\n"
						"<filename> : File name of the script\n"
						"<verbose>  : zero/non-zero value to control amount of output.",
				CLIScriptCmd,
				ATTR_NONE};

int set_script_path(char *path)
{
	char endc = '\0';
	int len = strlen(path);

	if (!('/' == path[len - 1]) && !('\\' == path[len - 1])) {
		len++;
		endc = '/';
	}

	if (len > SCRIPT_PATH_LEN) {
		return 1;
	}

	snprintf(script_path, SCRIPT_PATH_LEN, "%s%c", path, endc);
	return 0;
}

int CLIScriptPathCmd(struct cli_env *env, int argc, char **argv)
{
	int errorStat = 0;

	if (argc) {
		if (set_script_path(argv[0])) {
			LOGMSG(env,
					"FAILED: Maximum path length is %d characters\n",
					SCRIPT_PATH_LEN);
			goto exit;
		}
	}

	LOGMSG(env, "\tPrefix: \"%s\"\n", script_path);

exit:
	return errorStat;
}

struct cli_cmd CLIScriptPath =
		{"scrpath", 4, 0, "set directory prefix for script files",
				"{<path>}\n"
						"<path> : Directory path used to find script files\n"
						"If no path is supplied, the current path is printed.",
				CLIScriptPathCmd,
				ATTR_NONE};

int CLIQuitCmd(struct cli_env *env, int argc, char **argv);

struct cli_cmd CLIQuit = {"quit", 4, 0, "exit the CLI session",
		"No Parameters\n"
				"Exit from the current CLI", CLIQuitCmd,
		ATTR_NONE};

int CLIQuitCmd(struct cli_env *env, int argc, char **argv)
{
	if (argc) {
		LOGMSG(env, "FAILED: Extra parms ignored: \"%s\"\n", argv[0]);
		cli_print_help(env, &CLIQuit);
		goto exit;
	}

	if ((-1 == env->sess_socket) && (NULL != cons_cleanup))	{
		(*cons_cleanup)(env);
	}
	_exit(0);
	//return 1;

exit:
	return 0;
}

int CLIEchoCmd(struct cli_env *env, int argc, char **argv)
{
	int i;

	for (i = 0; i < argc; i++) {
		LOGMSG(env, "%s ", argv[i]);
	}
	LOGMSG(env, "\n");
	return 0;
}

struct cli_cmd CLIEcho =
		{"echo", 4, 0, "Echo any entered parameters to the terminal",
				"{parms}\n"
						"{parms} are printed to the terminal, followed by <CR><LF>\n",
				CLIEchoCmd,
				ATTR_NONE};

std::map<std::string, std::string> SET_VARS;

int SetCmd(struct cli_env *env, int argc, char **argv)
{
	if (argc == 0) {
		if (SET_VARS.size() == 0) {
			LOGMSG(env, "\nNo env vars\n");
			return 0;
		}

		std::stringstream ss;
		std::map<std::string, std::string>::iterator it;

		it = SET_VARS.begin();
		for (; it != SET_VARS.end(); it++) {
			ss << "\n\t" << it->first << "=" << it->second;
		}

		LOGMSG(env, "\nEnv vars: %s\n", ss.str().c_str());
		goto exit;
	}

	if (NULL == argv[0]) {
		goto exit;
	}

	if (argc == 1 && !strcmp(argv[0], "-X")) {
		SET_VARS.clear();
		goto exit;
	}

	if (argc == 1) {
		// Delete var                
		std::map<std::string, std::string>::iterator it
			= SET_VARS.find(argv[0]);
		if (it == SET_VARS.end()) {
			fprintf(stderr, "No such env var '%s'\n", argv[0]);
			return 0;
		}
		SET_VARS.erase(it);
		goto exit;
	}

	// Set var
	if (NULL == argv[1])
		goto exit;

	do {
		int start = 1;
		if (!strcmp(argv[1], "?")) { // "set key ? val" do not assign val if var "key" exists
			if (GetEnv(argv[0])) {
				break;
			}
			start = 2;
		}

		char buf[4100] = {0};
		for (int i = start; i < argc; i++) {
			strncat(buf, argv[i], 4096);
			strncat(buf, " ", 4096);
		}

		const int N = strlen(buf);
		buf[N - 1] = '\0';
		SET_VARS[argv[0]] = buf;
	} while (0);

exit:
	return 0;
}

struct cli_cmd CLISet =
		{"set", 3, 0, "Set/display environment vars",
				"set {var {val}}\n"
						"\"set -X\" deletes ALL variables from env\n"
						"\"set key\" deletes the variable from env\n"
						"\"set key val ...\" sets key:=val\n"
						"\"set key ? val ...\" sets key:=val iff key does not exist\n"
						"Note: val can be multiple words\n"
						"Default is display env vars.\n",
				SetCmd,
				ATTR_RPT};

void printProgress(struct cli_env *env)

{
	char progress[] = {'/', '-', '|', '-', '\\'};

	printf("%c\b", progress[env->progressState++]);
	if (env->progressState >= sizeof(progress) / sizeof(progress[0])) {
		env->progressState = 0;
	}
}

struct cli_cmd *cmd_line_cmds[] = {&CLIDebug, &CLIOpenLogFile, &CLICloseLogFile,
		&CLIScript, &CLIDotScript, &CLIScriptPath, &CLIEcho, &CLIQuit,
		&CLISet, &CLIConnect};

int bind_cli_cmd_line_cmds(void)
{
	char cwd[SCRIPT_PATH_SIZE];

	memset(script_path, 0, SCRIPT_PATH_SIZE);
	memset(cwd, 0, SCRIPT_PATH_SIZE);

	if (NULL == getcwd(cwd, sizeof(cwd))) {
		snprintf(script_path, SCRIPT_PATH_LEN, "scripts/");
	} else {
		snprintf(script_path, SCRIPT_PATH_LEN, "%s/scripts/", cwd);
	}

	return add_commands_to_cmd_db(
			sizeof(cmd_line_cmds) / sizeof(struct cli_cmd *),
			cmd_line_cmds);
}

#ifdef __cplusplus
}
#endif
