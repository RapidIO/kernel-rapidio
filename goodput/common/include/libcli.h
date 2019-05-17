/*
 ****************************************************************************
 Copyright (c) 2014, Integrated Device Technology Inc.
 Copyright (c) 2014, RapidIO Trade Association
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

#ifndef __LIBCLI_H__
#define __LIBCLI_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BUFLEN 1024

#define PROMPTLEN 29

#define CLI_VERSION_YR "15"
#define CLI_VERSION_MO "01"

/* Send string to one/all of the many output streams supported by cli_env */
#define LOGMSG(env, format, ...) \
	snprintf(env->output, sizeof(env->output), format, ##__VA_ARGS__); \
	logMsg(env);

struct cli_cmd;

struct cli_env {
	// Socket to use for input/output. Only valid if >0
	int sess_socket;

	// Name of script file to be processed.
	// If null, process commands from stdin.
	char *script;

	// Log file to be written to.
	// If null, output is only written to stdout.
	FILE *fout;

	// Prompt to print at command line, preserve last byte for NULL
	// termination.
	char prompt[PROMPTLEN + 1];

	// Buffered output string for echo
	char output[BUFLEN];

	// Input read from socket etc
	char input[BUFLEN];

	// Debug level of the current environment
	int DebugLevel;

	// Use to output spinning bar progress execution
	uint8_t progressState;

	// Device which is the subject of the current command.
	void *h;

	// store last valid command
	struct cli_cmd *cmd_prev;
};

void init_cli_env(struct cli_env *env);

#define ATTR_NONE  0x0
#define ATTR_RPT   0x1

struct cli_cmd {
	const char *name;
	uint8_t min_match; /* Minimum # of characters to match for this cmd */
	uint8_t min_parms; /* Minimum # of parameters to enter for this cmd */
	const char *shortHelp;
	const char *longHelp;
	int (*func)(struct cli_env *env, int argc, char **argv);
	int attributes;
};

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE !FALSE
#endif

/* parsing support routines */
int getDecParm(char *token, int defaultData);
uint64_t getHex(char *token, unsigned long defaultData);
float getFloatParm(char *token, float defaultData);
int parm_idx(char *token, char *token_list);

/* Routines to manage environment variables within the CLI */
char* GetEnv(char* var);
int SetEnvVar(char* arg);
char* SubstituteParam(char* arg);

/* Parsing support routines that support parameter substitution */
int GetDecParm(char* arg, int dflt);
uint64_t GetHex(char* arg, int dflt);
float GetFloatParm(char* arg, float dflt);

/* Updates *target with a copy of length len bytes of new_str
 */
int update_string(char **target, char *new_str, int len);

/* Updates *target with a copy of length len bytes of new_str
 * If chk_slash <> 0, then parm must start with '/' to be valid.
 */
int get_v_str(char **target, char *parm, int chk_slash);

/* CLI initialization/command binding routine.
 * The console_cleanup function is invoked by the "quit" command
 * on exit from the CLI.
 */
extern int cli_init_base(void (*console_cleanup)(struct cli_env *env));

extern int add_commands_to_cmd_db(int num_cmds, struct cli_cmd **cmd_list);

/* Display help for a command */
extern int cli_print_help(struct cli_env *env, struct cli_cmd *cmd);
extern const char *delimiter;

/* Display routines for start of a console or cli_terminal call */
/* NOTE: These messages are sent to stdout */
extern void splashScreen(struct cli_env *env, char *app_name);

/* Use LOGMSG */
extern void logMsg(struct cli_env *env);

/* Command processing:
 * process_command accepts a string and processes a command, if any is present
 * cli_terminal processes commands from a specified input stream until "quit"
 *              is entered.
 * console runs cli_terminal in a separate thread, accepting commands from
 *          stdin, until "quit" is entered.
 * cli_script executes a script file passed in based on the the environment.
 * set_script_path sets the file system path prepended to any script name.
 */
extern int process_command(struct cli_env *env, char *input);

extern int cli_terminal(struct cli_env *env);

extern int cli_script(struct cli_env *env, char *script, int verbose);
extern int set_script_path(char *path);

/* cons_parm must be "void" to match up with pthread types.
 * The cons_parm should be the prompt string to be used.
 */
void *console(void *cons_parm);

///< Argument (forced to void*) to console_rc
typedef struct {
	const char* prompt; ///< CLI prompt
	const char* script; ///< RC script
} ConsoleRc_t;

/** \brief Run the console but first execute the RC script
 * \note cons_parm must be "void" to match up with pthread types
 */
void* console_rc(void *cons_parm);

/** \brief remote_login starts a thread which listens to the specified TCP
 * port number.  Connections to the port number start a new thread with a CLI
 * session.  Note that "quit" will kill the calling process.  There is no
 * mutext between commands, so register reads/writes in different CLI sessions
 * may collide.
 *
 * portno - TCP socket number remote_login binds/listens/accepts on.
 * thr_name - Name for the remote_login pthread.
 *          - The name of the threads managing individual connections is
 *            'thr_name(session #)'.  This is used when naming threads, as well
 *            as part of the splash screen for each session.
 *          - The prompt for each thread managing an individual connection is
 *            the name of the thread with a '>' appended.
 */
struct remote_login_parms {
	int portno; /* Bind to this TCP Socket number  */
	char thr_name[16]; /* Thread name */
	int *status; /* Status bool, set to 1 when listening and 0 when not */
};

extern pthread_t remote_login_thread;

void* remote_login(void *remote_login_parm);

#ifdef __cplusplus
}
#endif

#endif /* __LIBCLI_H__ */
