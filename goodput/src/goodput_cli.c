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

#include <stdint.h>
#include <stdio.h>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <math.h>
#include <time.h>

#include "rio_misc.h"
#include "rio_route.h"
#include "tok_parse.h"
#include "rio_mport_lib.h"
#include "string_util.h"
#include "goodput_cli.h"
#include "goodput_umd_cli.h"
#include "libtime_utils.h"
#include "librsvdmem.h"
#include "liblog.h"
#include "assert.h"
#include "math_util.h"
#include "RapidIO_Error_Management_API.h"
#include "pw_handling.h"
#include "CPS1848.h"
#include "CPS1616.h"
#include "RXS2448.h"
#include "Tsi721.h"
#include "cps_event_test.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FLOAT_STR_SIZE 20

#define CM_HEADER_BYTES (0x18)

char *req_type_str[(int)last_action+1] = {
	(char *)"NO_ACT",
	(char *)"DIO",
	(char *)"ioTlat",
	(char *)"ioRlat",
	(char *)"DMA",
	(char *)"DmaNum",
	(char *)"dT_Lat",
	(char *)"dR_Lat",
	(char *)"dR_Gpt",
	(char *)"MSG_Tx",
	(char *)"mT_Lat",
	(char *)"mTx_Oh",
	(char *)"MSG_Rx",
	(char *)"mR_Lat",
	(char *)"mRx_Oh",
	(char *)" IBWIN",
	(char *)"~IBWIN",
	(char *)"SHTDWN",
	(char *)"RgScrb",
	(char *)"7T SwM",
	(char *)"721 FI",
	(char *)"721Rec",
	(char *)"721HLI",
	(char *)"7PW_RX",
	(char *)"CPSPWh",
	(char *)"721PWh",
	(char *)"Pol4PW",
	(char *)"SwLock",
	(char *)"Maint ",
	(char *)"LAST"
};

// Parse the token ensuring it is within the range for a worker index and
// check the status of the worker thread.
static int gp_parse_worker_index(struct cli_env *env, char *tok, uint16_t *idx)
{
	if (tok_parse_ushort(tok, idx, 0, MAX_WORKER_IDX, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "<idx>", 0, MAX_WORKER_IDX);
		return 1;
	}
	return 0;
}

// Parse the token ensuring it is within the range for a worker index and
// check the status of the worker thread.
static int gp_parse_worker_index_check_thread(struct cli_env *env, char *tok,
		uint16_t *idx, int want_halted)
{
	if (gp_parse_worker_index(env, tok, idx)) {
		goto err;
	}

	switch (want_halted) {
	case 0: if (2 == wkr[*idx].stat) {
			LOGMSG(env, "\nWorker halted\n");
			goto err;
		}
		break;

	case 1: if (2 != wkr[*idx].stat) {
			LOGMSG(env, "\nWorker not halted\n");
			goto err;
		}
		break;
	case 2: if (1 != wkr[*idx].stat) {
			LOGMSG(env, "\nWorker not running\n");
			goto err;
		}
		break;
	default: goto err;
	}
	return 0;
err:
	return 1;
}

// Parse the token as a boolean value. The range of the token is restricted
// to the numeric values of 0 (false) and 1 (true)
static int gp_parse_bool(struct cli_env *env, char *tok, const char *name, uint16_t *boo)
{
	if (tok_parse_ushort(tok, boo, 0, 1, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, name, 0, 1);
		return 1;
	}
	return 0;
}

// Parse the token ensuring it is within the provided range. Further ensure it
// is a power of 2
static int gp_parse_ull_pw2(struct cli_env *env, char *tok, const char *name,
		uint64_t *value, uint64_t min, uint64_t max)
{
	if (tok_parse_ulonglong(tok, value, min, max, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULONGLONG_HEX_MSG_FMT, name, min, max);
		goto err;
	}

	if ((*value - 1) & *value) {
		LOGMSG(env, "\n%s must be a power of 2\n", name);
		goto err;
	}

	return 0;
err:
	return 1;
}

static int gp_parse_cpu(struct cli_env *env, char *dec_parm, int *cpu)
{
	const int MAX_GOODPUT_CPU = getCPUCount() - 1;

	if (tok_parse_long(dec_parm, cpu, -1, MAX_GOODPUT_CPU, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_LONG_MSG_FMT, "<cpu>", -1, MAX_GOODPUT_CPU);
		return 1;
	}
	return 0;
}

static int gp_parse_did(struct cli_env *env, char *tok, did_val_t *did_val)
{
	if (tok_parse_did(tok, did_val, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, "<did> must be between 0 and 0xff\n");
		return 1;
	}
	return 0;
}

#define ACTION_STR(x) (char *)((x < last_action)?req_type_str[x]:"UNKWN!")
#define MODE_STR(x) (char *)((x == kernel_action)?"KRNL":"User")
#define THREAD_STR(x) (char *)((0 == x)?"---":((1 == x)?"Run":"Hlt"))

static int ThreadCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint16_t idx;
	uint16_t new_dma;
	int cpu;

	if (gp_parse_worker_index(env, argv[0], &idx)) {
		goto exit;
	}

	if (gp_parse_cpu(env, argv[1], &cpu)) {
		goto exit;
	}

	if (gp_parse_bool(env, argv[2], "<new_dma>", &new_dma)) {
		goto exit;
	}

	if (wkr[idx].stat) {
		LOGMSG(env, "\nWorker %u already alive\n", idx);
		goto exit;
	}

	wkr[idx].idx = (int)idx;
	start_worker_thread(&wkr[idx], (int)new_dma, cpu);

exit:
	return 0;
}

struct cli_cmd Thread = {
"thread",
1,
3,
"Start a thread on a cpu",
"start <idx> <cpu> <new_dma>\n"
	"<idx>     is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<cpu>     is a cpu number, or -1 to indicate no cpu affinity\n"
	"<new_dma> 0: share DMA channel, 1: try to get a new DMA channel\n",
ThreadCmd,
ATTR_NONE
};

static int KillCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint16_t st_idx = 0, end_idx = MAX_WORKER_IDX, i;

	if (strncmp(argv[0], "all", 3)) {
		if (gp_parse_worker_index(env, argv[0], &st_idx)) {
			goto exit;
		}
		end_idx = st_idx;
	}

	for (i = st_idx; i <= end_idx; i++) {
		shutdown_worker_thread(&wkr[i]);
	}

exit:
	return 0;
}

struct cli_cmd Kill = {
"kill",
4,
1,
"Kill a thread",
"kill <idx>\n"
	"<idx> is a worker index from 0 to " STR(MAX_WORKER_IDX) ", or \"all\"\n",
KillCmd,
ATTR_NONE
};

static int HaltCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint16_t st_idx = 0, end_idx = MAX_WORKER_IDX, i;

	if (strncmp(argv[0], "all", 3)) {
		if (gp_parse_worker_index(env, argv[0], &st_idx)) {
			goto exit;
		}
		end_idx = st_idx;
	}

	for (i = st_idx; i <= end_idx; i++) {
		wkr[i].stop_req = 2;
	}

exit:
	return 0;
}

struct cli_cmd Halt = {
"halt",
2,
1,
"Halt execution of a thread command",
"halt <idx>\n"
	"<idx> is a worker index from 0 to " STR(MAX_WORKER_IDX) ", or \"all\"\n",
HaltCmd,
ATTR_NONE
};

static int MoveCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint16_t idx;
	int cpu;

	if (gp_parse_worker_index(env, argv[0], &idx)) {
		goto exit;
	}

	if (gp_parse_cpu(env, argv[1], &cpu)) {
		goto exit;
	}

	if (0 == wkr[idx].stat) {
		LOGMSG(env, "\nThread %u was not alive\n", idx);
		goto exit;
	}

	wkr[idx].wkr_thr.cpu_req = cpu;
	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);

exit:
	return 0;
}

struct cli_cmd Move = {
"move",
2,
2,
"Move a thread to a different CPU",
"move <idx> <cpu>\n"
	"<idx> is a worker index from 0 to " STR(MAX_WORKER_IDX)",\n"
	"<cpu> is a cpu number, or -1 to indicate no cpu affinity\n",
MoveCmd,
ATTR_NONE
};

static int WaitCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	const struct timespec ten_usec = {0, 10 * 1000};

	uint16_t idx, limit = 10000;
	int state;

	if (gp_parse_worker_index(env, argv[0], &idx)) {
		goto exit;
	}

	switch (argv[1][0]) {
	case '0':
	case 'd':
	case 'D':
		state = 0;
		break;
	case '1':
	case 'r':
	case 'R':
		state = 1;
		break;
	case '2':
	case 'h':
	case 'H':
		state = 2;
		break;
	default:
		state = -1;
		break;
	}

	if (-1 == state) {
		LOGMSG(env, "\nState must be 0|d|D, 1|r|R, or 2|h|H\n");
		goto exit;
	}

	while ((wkr[idx].stat != state) && limit--) {
		time_sleep(&ten_usec);
	}

	if (wkr[idx].stat == state) {
		LOGMSG(env, "\nPassed, Worker %u is now %s\n", idx,
				THREAD_STR(wkr[idx].stat));
	} else {
		LOGMSG(env, "\nFAILED, Worker %u is now %s\n", idx,
				THREAD_STR(wkr[idx].stat));
	}

exit:
	return 0;
}

struct cli_cmd Wait = {
"wait",
2,
2,
"Wait until a thread reaches a particular state",
"wait <idx> <state>\n"
	"<idx>   is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<state> 0|d|D - Dead, 1|r|R - Run, 2|h|H - Halted\n",
WaitCmd,
ATTR_NONE
};

static int SleepCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	float sec;
	double fractional;
	double seconds;
	struct timespec delay;

	sec = GetFloatParm(argv[0], 0);
	if(sec > 0) {
		LOGMSG(env, "\nSleeping %f sec\n", sec);
		fractional = modf(sec, &seconds);
		delay.tv_sec = seconds;
		delay.tv_nsec = fractional * 1000000 * 1000;
		time_sleep(&delay);
	}
	return 0;
}

struct cli_cmd Sleep = {
"sleep",
2,
1,
"Sleep for a number of seconds (fractional allowed)",
"sleep <sec>\n"
	"<sec> is the number of seconds to sleep\n",
SleepCmd,
ATTR_NONE
};

#define FOUR_KB (4*1024)
#define SIXTEEN_MB (16*1024*1024)

static int IBAllocCmd(struct cli_env *env, int argc, char **argv)
{
	uint16_t idx;
	uint64_t ib_size;
	uint64_t ib_rio_addr = RIO_MAP_ANY_ADDR;
	uint64_t ib_phys_addr= RIO_MAP_ANY_ADDR;

	if (gp_parse_worker_index_check_thread(env, argv[0], &idx, 1)) {
		goto exit;
	}

	if (gp_parse_ull_pw2(env, argv[1], "<size>", &ib_size, FOUR_KB,
			4 * SIXTEEN_MB)) {
		goto exit;
	}

	/* Note: RSVD overrides rio_addr */
	if (argc > 3) {
		if (get_rsvd_phys_mem(argv[3], &ib_phys_addr, &ib_size)) {
			LOGMSG(env, "\nNo reserved memory found for keyword %s",
					argv[3]);
			goto exit;
		}
	} else if ((argc > 2)
			&& (tok_parse_ulonglong(argv[2], &ib_rio_addr, 1,
					UINT64_MAX, 0))) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULONGLONG_HEX_MSG_FMT, "<addr>",
				(uint64_t )1, (uint64_t)UINT64_MAX);
		goto exit;
	}

	if ((ib_rio_addr != RIO_MAP_ANY_ADDR) && ((ib_size-1) & ib_rio_addr)) {
		LOGMSG(env, "\n<addr> not aligned with <size>\n");
		goto exit;
	}

	wkr[idx].action = alloc_ibwin;
	wkr[idx].ib_byte_cnt = ib_size;
	wkr[idx].ib_rio_addr = ib_rio_addr;
	wkr[idx].ib_handle = ib_phys_addr;
	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);

exit:
	return 0;
}

struct cli_cmd IBAlloc = {
"IBAlloc",
3,
2,
"Allocate an inbound window",
"IBAlloc <idx> <size> {<addr> {<RSVD>}}\n"
	"<size> must be a power of two from 0x1000 to 0x01000000\n"
	"<addr> is the optional RapidIO address for the inbound window\n"
	"       NOTE: <addr> must be aligned to <size>\n"
	"<RSVD> is a keyword for reserved memory area\n"
	"       NOTE: If <RSVD> is specified, <addr> is ignored\n",
IBAllocCmd,
ATTR_NONE
};

static int IBDeallocCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint16_t idx;

	if (gp_parse_worker_index_check_thread(env, argv[0], &idx, 1)) {
		goto exit;
	}

	wkr[idx].action = free_ibwin;
	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);

exit:
	return 0;
}

struct cli_cmd IBDealloc = {
"IBDealloc",
3,
1,
"Deallocate an inbound window",
"IBDealloc <idx>\n"
	"<idx> is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n",
IBDeallocCmd,
ATTR_NONE
};

#define PROC_STAT_PFMT "\nTot CPU Jiffies %lu %lu %lu %lu %lu %lu %lu\n"

#define CPUOCC_BUFF_SIZE 1024

static void cpu_occ_parse_proc_line(char *file_line, uint64_t *proc_new_utime,
		uint64_t *proc_new_stime)
{
	char *tok;
	char *saveptr;
	char *delim = (char *)" ";
	int tok_cnt = 0;
	char fl_cpy[CPUOCC_BUFF_SIZE];

	SAFE_STRNCPY(fl_cpy, file_line, sizeof(fl_cpy));
	tok = strtok_r(file_line, delim, &saveptr);
	while ((NULL != tok) && (tok_cnt < 13)) {
		tok = strtok_r(NULL, delim, &saveptr);
		tok_cnt++;
	}

	if (NULL == tok) {
		goto error;
	}
	if (tok_parse_ull(tok, proc_new_utime, 0)) {
		goto error;
	}

	tok = strtok_r(NULL, delim, &saveptr);
	if (NULL == tok) {
		goto error;
	}
	if (tok_parse_ull(tok, proc_new_stime, 0)) {
		goto error;
	}
	return;

error:
	ERR("\nFAILED: proc_line \"%s\"\n", fl_cpy);
}

static void cpu_occ_parse_stat_line(char *file_line, uint64_t *p_user,
		uint64_t *p_nice, uint64_t *p_system, uint64_t *p_idle,
		uint64_t *p_iowait, uint64_t *p_irq, uint64_t *p_softirq)
{
	char *tok, *saveptr;
	char *delim = (char *)" ";
	char fl_cpy[CPUOCC_BUFF_SIZE];

	SAFE_STRNCPY(fl_cpy, file_line, sizeof(fl_cpy));

	/* Throw the first token away. */
	tok = strtok_r(file_line, delim, &saveptr);
	if (NULL == tok)
		goto error;

	tok = strtok_r(NULL, delim, &saveptr);
	if (NULL == tok)
		goto error;
	if (tok_parse_ull(tok, p_user, 0))
		goto error;

	tok = strtok_r(NULL, delim, &saveptr);
	if (NULL == tok)
		goto error;
	if (tok_parse_ull(tok, p_nice, 0))
		goto error;

	tok = strtok_r(NULL, delim, &saveptr);
	if (NULL == tok)
		goto error;
	if (tok_parse_ull(tok, p_system, 0))
		goto error;

	tok = strtok_r(NULL, delim, &saveptr);
	if (NULL == tok)
		goto error;
	if (tok_parse_ull(tok, p_idle, 0))
		goto error;

	tok = strtok_r(NULL, delim, &saveptr);
	if (NULL == tok)
		goto error;
	if (tok_parse_ull(tok, p_iowait, 0))
		goto error;

	tok = strtok_r(NULL, delim, &saveptr);
	if (NULL == tok)
		goto error;
	if (tok_parse_ull(tok, p_irq, 0))
		goto error;

	tok = strtok_r(NULL, delim, &saveptr);
	if (NULL == tok)
		goto error;
	if (tok_parse_ull(tok, p_softirq, 0))
		goto error;

	return;
error:
	ERR("\nFAILED: stat_line \"%s\"\n", fl_cpy);
}

int cpu_occ_valid;

uint64_t old_tot_jifis;
uint64_t old_proc_kern_jifis;
uint64_t old_proc_user_jifis;
uint64_t new_tot_jifis;
uint64_t new_proc_kern_jifis;
uint64_t new_proc_user_jifis;

float cpu_occ_pct;

static int cpu_occ_set(uint64_t *tot_jifis, uint64_t *proc_kern_jifis,
		uint64_t *proc_user_jifis)
{
	FILE *stat_fp = NULL, *cpu_stat_fp = NULL;
	char filename[256] = {0};
	char file_line[CPUOCC_BUFF_SIZE] = {0};
	uint64_t p_user = 1, p_nice = 1, p_system = 1, p_idle = 1;
	uint64_t p_iowait = 1, p_irq = 1, p_softirq = 1;

	pid_t my_pid = getpid();

	snprintf(filename, 255, "/proc/%d/stat", my_pid);

	stat_fp = fopen(filename, "re" );
	if (NULL == stat_fp) {
		ERR( "FAILED: Open proc stat file \"%s\": %d %s\n",
			filename, errno, strerror(errno));
		goto exit;
	}

	cpu_stat_fp = fopen("/proc/stat", "re");
	if (NULL == cpu_stat_fp) {
		ERR("FAILED: Open file \"/proc/stat\": %d %s\n",
			errno, strerror(errno));
		goto exit;
	}

	if (NULL == fgets(file_line, sizeof(file_line), stat_fp)) {
		ERR("Unexpected EOF 1: %d %s\n", errno, strerror(errno));
		goto exit;
	}

	cpu_occ_parse_proc_line(file_line, proc_user_jifis, proc_kern_jifis);


	memset(file_line, 0, sizeof(file_line));
	if (NULL == fgets(file_line, sizeof(file_line), cpu_stat_fp)) {
		ERR("Unexpected EOF 2: %d %s\n", errno, strerror(errno));
		goto exit;
	}

	cpu_occ_parse_stat_line(file_line, &p_user, &p_nice, &p_system,
			&p_idle, &p_iowait, &p_irq, &p_softirq);

	*tot_jifis = p_user + p_nice + p_system + p_idle +
			p_iowait + p_irq + p_softirq;

exit:
	if (stat_fp != NULL) {
		fclose(stat_fp);
	}
	if (cpu_stat_fp != NULL) {
		fclose(cpu_stat_fp);
	}
	return 0;
}

static int CPUOccSetCmd(struct cli_env *env, int UNUSED(argc),
		char **UNUSED(argv))
{

	if (cpu_occ_set(&old_tot_jifis, &old_proc_kern_jifis,
			&old_proc_user_jifis)) {
		LOGMSG(env, "\nFAILED: Could not get proc info \n");
		goto exit;
	}
	LOGMSG(env, "\nSet CPU Occ measurement start point\n");

	cpu_occ_valid = 1;

exit:
	return 0;
}

struct cli_cmd CPUOccSet = {
"oset",
2,
0,
"Set CPU Occupancy measurement start point.",
"oset\n"
	"No parameters\n",
CPUOccSetCmd,
ATTR_NONE
};

int cpu_occ_saved_idx;

static int CPUOccDisplayCmd(struct cli_env *env, int UNUSED(argc),
		char **UNUSED(argv))
{
	char pctg[FLOAT_STR_SIZE];
	int cpus = getCPUCount();

	if (!cpus) {
		cpus = 1;
	}

	if (!cpu_occ_valid) {
		LOGMSG(env, "\nFAILED: CPU OCC measurement start not set\n");
		goto exit;
	}

	if (cpu_occ_set(&new_tot_jifis, &new_proc_kern_jifis,
			&new_proc_user_jifis)) {
		LOGMSG(env, "\nFAILED: Could not get proc info \n");
		goto exit;
	}

	cpu_occ_pct = (((float)(new_proc_kern_jifis + new_proc_user_jifis
			- old_proc_kern_jifis - old_proc_user_jifis))
			/ ((float)(new_tot_jifis - old_tot_jifis))) * 100.0
			* cpus;
	snprintf(pctg, sizeof(pctg), "%4.2f", cpu_occ_pct);

	LOGMSG(env, "\n-Kernel- ProcUser ProcKern CPU_Occ\n");
	LOGMSG(env, "%8ld %8ld %8ld %7s\n", new_tot_jifis - old_tot_jifis,
			new_proc_user_jifis - old_proc_user_jifis,
			new_proc_kern_jifis - old_proc_kern_jifis, pctg);

exit:
	return 0;
}

struct cli_cmd CPUOccDisplay = {
"odisp",
2,
0,
"Display cpu occupancy",
"odisp\n"
	"No parameters\n",
CPUOccDisplayCmd,
ATTR_RPT
};

static int obdio_cmd(struct cli_env *env, int UNUSED(argc), char **argv,
		enum req_type action)
{
	uint16_t idx;
	did_val_t did_val;
	uint64_t rio_addr;
	uint64_t bytes;
	uint64_t acc_sz;
	uint16_t wr;
	uint32_t min_obwin_size = 0x10000;

	int n = 0;
	if (gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1)) {
		goto exit;
	}

	if (gp_parse_did(env, argv[n++], &did_val)) {
		goto exit;
	}

	if (tok_parse_ulonglong(argv[n++], &rio_addr, 1, UINT64_MAX, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULONGLONG_HEX_MSG_FMT, "<rio_addr>",
				(uint64_t)1, (uint64_t)UINT64_MAX);
		goto exit;
	}

	if (direct_io == action) {
		if (tok_parse_ull(argv[n++], &bytes, 0)) {
			LOGMSG(env, "\n");
			LOGMSG(env, TOK_ERR_ULL_HEX_MSG_FMT, "<bytes>");
			goto exit;
		}
		if (gp_parse_ull_pw2(env, argv[n++], "<acc_sz>", &acc_sz, 1, 8)) {
			goto exit;
		}
		if (gp_parse_bool(env, argv[n++], "<wr>", &wr)) {
			goto exit;
		}
	} else {
		if (gp_parse_ull_pw2(env, argv[n++], "<acc_sz>", &acc_sz, 1, 8)) {
			goto exit;
		}
		bytes = acc_sz;
		if (direct_io_tx_lat == action) {
			if (gp_parse_bool(env, argv[n++], "<wr>", &wr)) {
				goto exit;
			}
		} else {
			wr = 1;
		}
	}

	if ((direct_io_tx_lat == action)
			&& (!wkr[idx].ib_valid || (NULL == wkr[idx].ib_ptr))) {
		LOGMSG(env, "\nNo mapped inbound window present\n");
		goto exit;
	}

	wkr[idx].action = action;
	wkr[idx].action_mode = kernel_action;
	wkr[idx].did_val = did_val;
	wkr[idx].rio_addr = rio_addr;
	wkr[idx].byte_cnt = bytes;
	wkr[idx].acc_size = acc_sz;
	wkr[idx].wr = wr;
	// Set up the size of the outbound window, which must be:
	// - at least 0x10000 bytes
	// - larger than the bytes to transfer,
	// - a power of 2.
	wkr[idx].ob_byte_cnt = min_obwin_size;
	if ((direct_io == action) && (bytes > min_obwin_size)) {
		wkr[idx].ob_byte_cnt = roundup_pw2(bytes);
		if (!wkr[idx].ob_byte_cnt) {
			LOGMSG(env, "\nInvalid outbound window size\n");
			goto exit;
		}
	}

	if (bytes % acc_sz) {
		LOGMSG(env, "\nBytes must be a multiple of acc_sz\n");
		goto exit;
	}

	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);

exit:
	return 0;
}

static int OBDIOCmd(struct cli_env *env, int argc, char **argv)
{
	return obdio_cmd(env, argc, argv, direct_io);
}

struct cli_cmd OBDIO = {
"OBDIO",
5,
6,
"Measure goodput of reads/writes through an outbound window",
"OBDIO <idx> <did> <rio_addr> <bytes> <acc_sz> <wr>\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<did>      target device ID\n"
	"<rio_addr> RapidIO memory address to access\n"
	"<bytes>    total bytes to transfer, must be a multiple of <acc_sz>\n"
	"<acc_sz>   access size, 1, 2, 4, 8\n"
	"<wr>       0: Read, 1: Write\n",
OBDIOCmd,
ATTR_NONE
};

static int OBDIOTxLatCmd(struct cli_env *env, int argc, char **argv)
{
	return obdio_cmd(env, argc, argv, direct_io_tx_lat);
}

struct cli_cmd OBDIOTxLat = {
"DIOTxLat",
8,
5,
"Measure latency of reads/writes through an outbound window",
"DIOTxLat <idx> <did> <rio_addr> <acc_sz> <wr>\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<did>      target device ID\n"
	"<rio_addr> RapidIO memory address to access\n"
	"<acc_sz>   access size, values: 1, 2, 4, 8\n"
	"<wr>       0: Read, 1: Write\n"
	"           NOTE: For <wr> = 1, there must be a <did> thread running OBDIORxLat!\n",
OBDIOTxLatCmd,
ATTR_NONE
};

static int OBDIORxLatCmd(struct cli_env *env, int argc, char **argv)
{
	return obdio_cmd(env, argc, argv, direct_io_rx_lat);
}

struct cli_cmd OBDIORxLat = {
"DIORxLat",
4,
4,
"Loop back DIOTxLat writes through an outbound window",
"DIORxLat <idx> <did> <rio_addr> <acc_sz>\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<did>      target device ID\n"
	"<rio_addr> RapidIO memory address to access\n"
	"<acc_sz>   access size, 1, 2, 4 or 8\n"
	"\nNOTE: DIORxLat must be run before OBDIOTxLat!\n",
OBDIORxLatCmd,
ATTR_NONE
};

// "<trans>  0 NW, 1 SW, 2 NW_R, 3 SW_R 4 NW_R_ALL\n"
enum rio_exchange convert_int_to_riomp_dma_directio_type(uint16_t trans)
{
	switch (trans) {
	default:
	case 0:
		return RIO_EXCHANGE_NWRITE;
	case 1:
		return RIO_EXCHANGE_SWRITE;
	case 2:
		return RIO_EXCHANGE_NWRITE_R;
	case 3:
		return RIO_EXCHANGE_SWRITE_R;
	case 4:
		return RIO_EXCHANGE_NWRITE_R_ALL;
	}
}

static int dmaCmd(struct cli_env *env, int argc, char **argv)
{
	uint16_t idx;
	did_val_t did_val;
	uint64_t rio_addr;
	uint64_t bytes;
	uint64_t acc_sz;
	uint16_t wr;
	uint16_t kbuf;
	uint16_t trans;
	uint16_t sync;

	int n = 0;
	if (gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1)) {
		goto exit;
	}

	if (gp_parse_did(env, argv[n++], &did_val)) {
		goto exit;
	}

	if (tok_parse_ulonglong(argv[n++], &rio_addr, 1, UINT64_MAX, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULONGLONG_HEX_MSG_FMT, "<rio_addr>",
				(uint64_t)1, (uint64_t)UINT64_MAX);
		goto exit;
	}

	if (tok_parse_ull(argv[n++], &bytes, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULL_HEX_MSG_FMT, "<bytes>");
		goto exit;
	}

	if (gp_parse_ull_pw2(env, argv[n++], "<acc_sz>", &acc_sz, 1, UINT32_MAX)) {
		goto exit;
	}

	if (gp_parse_bool(env, argv[n++], "<wr>", &wr)) {
		goto exit;
	}

	if (gp_parse_bool(env, argv[n++], "<kbuf>", &kbuf)) {
		goto exit;
	}

	if (tok_parse_ushort(argv[n++], &trans, 0, 4, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "<trans>", 0, 4);
		goto exit;
	}

	if (tok_parse_ushort(argv[n++], &sync, 0, 2, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "<sync>", 0, 2);
		goto exit;
	}

	// Optional parameters - ssdist, sssize, dsdist, dssize
	wkr[idx].ssdist = 0;
	wkr[idx].sssize = 0;
	wkr[idx].dsdist = 0;
	wkr[idx].dssize = 0;
	wkr[idx].ignore_dma_errs = 0;

	if ((argc > 9)
			&& (tok_parse_ushort(argv[n++], &wkr[idx].ssdist, 0,
					0xFFFF, 0))) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "<ssdist>", 0, 0xFFFF);
		goto exit;
	}

	if ((argc > 10)
			&& (tok_parse_ushort(argv[n++], &wkr[idx].sssize, 0,
					0x0FFF, 0))) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "<sssize>", 0, 0x0FFF);
		goto exit;
	}

	if ((argc > 11)
			&& (tok_parse_ushort(argv[n++], &wkr[idx].dsdist, 0,
					0xFFFF, 0))) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "<dsdist>", 0, 0xFFFF);
		goto exit;
	}

	if ((argc > 12)
			&& (tok_parse_ushort(argv[n++], &wkr[idx].dssize, 0,
					0x0FFF, 0))) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "<dssize>", 0, 0x0FFF);
		goto exit;
	}

	if ((argc > 13)
			&& (tok_parse_ushort(argv[n++],
					&wkr[idx].ignore_dma_errs, 0,
					0x0FFF, 0))) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "<ignore_errs>", 0, 0x0FFF);
		goto exit;
	}

	if (did_val < MAX_DEVID_STATUS)
		devid_status[did_val] = 1;
	wkr[idx].action = dma_tx;
	wkr[idx].action_mode = kernel_action;
	wkr[idx].did_val = did_val;
	wkr[idx].rio_addr = rio_addr;
	wkr[idx].byte_cnt = bytes;
	wkr[idx].acc_size = acc_sz;
	wkr[idx].wr = wr;
	wkr[idx].use_kbuf = kbuf;
	wkr[idx].dma_trans_type = convert_int_to_riomp_dma_directio_type(trans);
	wkr[idx].dma_sync_type = (enum rio_transfer_sync)sync;
	wkr[idx].rdma_buff_size = bytes;

	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);

exit:
	return 0;
}

struct cli_cmd dma = {
"dma",
3,
9,
"Measure goodput of DMA reads/writes",
"dma <idx> <did> <rio_addr> <bytes> <acc_sz> <wr> <kbuf> <trans> <sync> [<ssdist> <sssize> <dsdist> <dssize> <no_err>]\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<did>      target device ID\n"
	"<rio_addr> RapidIO memory address to access\n"
	"<bytes>    total bytes to transfer\n"
	"<acc_sz>   access size, must be a power of two from 1 to 0xffffffff\n"
	"<wr>       0: Read, 1: Write\n"
	"<kbuf>     0: User memory, 1: Kernel buffer\n"
	"<trans>    0: NW, 1: SW, 2: NW_R, 3: SW_R, 4: NW_R_ALL\n"
	"<sync>     0: SYNC, 1: ASYNC, 2: FAF\n"
	"<ssdist>   source stride distance (Optional, default to 0)\n"
	"<sssize>   source stride size (Optional, default to 0)\n"
	"<dsdist>   destination stride distance (Optional, default to 0)\n"
	"<dssize>   destination stride size (Optional, default to 0)\n"
	"<no_err>   If non-zero, DMA errors are ignored.\n",
dmaCmd,
ATTR_NONE
};

static int dmaNumCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint16_t idx;
	did_val_t did_val;
	uint64_t rio_addr;
	uint64_t bytes;
	uint64_t acc_sz;
	uint16_t wr;
	uint16_t kbuf;
	uint16_t trans;
	uint16_t sync;
	uint32_t num_trans;

	int n = 0;
	if (gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1)) {
		goto exit;
	}

	if (gp_parse_did(env, argv[n++], &did_val)) {
		goto exit;
	}

	if (tok_parse_ulonglong(argv[n++], &rio_addr, 1, UINT64_MAX, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULONGLONG_HEX_MSG_FMT, "<rio_addr>",
				(uint64_t)1, (uint64_t)UINT64_MAX);
		goto exit;
	}

	if (tok_parse_ull(argv[n++], &bytes, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULL_HEX_MSG_FMT, "<bytes>");
		goto exit;
	}

	if (gp_parse_ull_pw2(env, argv[n++], "<acc_sz>", &acc_sz, 1, UINT32_MAX)) {
		goto exit;
	}

	if (gp_parse_bool(env, argv[n++], "<wr>", &wr)) {
		goto exit;
	}

	if (gp_parse_bool(env, argv[n++], "<kbuf>", &kbuf)) {
		goto exit;
	}

	if (tok_parse_ushort(argv[n++], &trans, 0, 4, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "<trans>", 0, 4);
		goto exit;
	}

	if (tok_parse_ushort(argv[n++], &sync, 0, 2, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "<sync>", 0, 2);
		goto exit;
	}

	if (tok_parse_ul(argv[n++], &num_trans, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_UL_HEX_MSG_FMT, "<num>");
		goto exit;
	}

	if (did_val < MAX_DEVID_STATUS)
		devid_status[did_val] = 1;
	wkr[idx].action = dma_tx_num;
	wkr[idx].action_mode = kernel_action;
	wkr[idx].did_val = did_val;
	wkr[idx].rio_addr = rio_addr;
	wkr[idx].byte_cnt = bytes;
	wkr[idx].acc_size = acc_sz;
	wkr[idx].wr = (int)wr;
	wkr[idx].use_kbuf = (int)kbuf;
	wkr[idx].dma_trans_type = convert_int_to_riomp_dma_directio_type(trans);
	wkr[idx].dma_sync_type = (enum rio_transfer_sync)sync;
	wkr[idx].rdma_buff_size = bytes;
	wkr[idx].num_trans = (int)num_trans;
	LOGMSG(env, "Wr %x kbuf %x trans %x sync %x num %x\n",
		wkr[idx].wr, wkr[idx].use_kbuf,
		wkr[idx].dma_trans_type, wkr[idx].dma_sync_type,
		wkr[idx].num_trans);

	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);

exit:
	return 0;
}

struct cli_cmd dmaNum = {
"dnum",
3,
10,
"Send a specified number of DMA reads/writes",
"dnum <idx> <did> <rio_addr> <bytes> <acc_sz> <wr> <kbuf> <trans> <sync> <num>\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<did>      target device ID\n"
	"<rio_addr> RapidIO memory address to access\n"
	"<bytes>    total bytes to transfer\n"
	"<acc_sz>   access size, must be a power of two from 1 to 0xffffffff\n"
	"<wr>       0: Read, 1: Write\n"
	"<kbuf>     0: User memory, 1: Kernel buffer\n"
	"<trans>    0: NW, 1: SW, 2: NW_R, 3: SW_R, 4: NW_R_ALL\n"
	"<sync>     0: SYNC, 1: ASYNC, 2: FAF\n"
	"<num>      number of transactions to send\n",
dmaNumCmd,
ATTR_NONE
};

static int dmaTxLatCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint16_t idx;
	did_val_t did_val;
	uint64_t rio_addr;
	uint64_t bytes;
	uint16_t wr;
	uint16_t kbuf;
	uint16_t trans;

	int n = 0;
	if (gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1)) {
		goto exit;
	}

	if (gp_parse_did(env, argv[n++], &did_val)) {
		goto exit;
	}

	if (tok_parse_ulonglong(argv[n++], &rio_addr, 1, UINT64_MAX, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULONGLONG_HEX_MSG_FMT, "<rio_addr>",
				(uint64_t)1, (uint64_t)UINT64_MAX);
		goto exit;
	}

	if (tok_parse_ull(argv[n++], &bytes, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULL_HEX_MSG_FMT, "<bytes>");
		goto exit;
	}

	if (gp_parse_bool(env, argv[n++], "<wr>", &wr)) {
		goto exit;
	}

	if (gp_parse_bool(env, argv[n++], "<kbuf>", &kbuf)) {
		goto exit;
	}

	if (tok_parse_ushort(argv[n++], &trans, 0, 4, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "<trans>", 0, 4);
		goto exit;
	}

	if (wr && (!wkr[idx].ib_valid || (NULL == wkr[idx].ib_ptr))) {
		LOGMSG(env, "\nNo mapped inbound window present\n");
		goto exit;
	}

	if (did_val < MAX_DEVID_STATUS)
		devid_status[did_val] = 1;
	wkr[idx].action = dma_tx_lat;
	wkr[idx].action_mode = kernel_action;
	wkr[idx].did_val = did_val;
	wkr[idx].rio_addr = rio_addr;
	wkr[idx].byte_cnt = bytes;
	wkr[idx].acc_size = bytes;
	wkr[idx].wr = wr;
	wkr[idx].use_kbuf = kbuf;
	wkr[idx].dma_trans_type = convert_int_to_riomp_dma_directio_type(trans);
	wkr[idx].dma_sync_type = RIO_TRANSFER_SYNC;

	if (bytes < MIN_RDMA_BUFF_SIZE) {
		wkr[idx].rdma_buff_size = MIN_RDMA_BUFF_SIZE;
	} else {
		wkr[idx].rdma_buff_size = bytes;
	}

	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);

exit:
	return 0;
}

struct cli_cmd dmaTxLat = {
"dTxLat",
2,
7,
"Measure lantecy of DMA reads/writes",
"dTxLat <idx> <did> <rio_addr> <bytes> <wr> <kbuf> <trans>\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<did>      target device ID\n"
	"<rio_addr> RapidIO memory address to access\n"
	"<bytes>    total bytes to transfer\n"
	"<wr>       0: Read, 1: Write\n"
	"           NOTE: For <wr> = 1, there must be a thread on <did> running dRxLat!\n"
	"<kbuf>     0: User memory, 1: Kernel buffer\n"
	"<trans>    0: NW, 1: SW, 2: NW_R, 3: SW_R, 4: NW_R_ALL\n",
dmaTxLatCmd,
ATTR_NONE
};

static int dmaRxLatCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint16_t idx;
	did_val_t did_val;
	uint64_t rio_addr;
	uint64_t bytes;

	int n = 0;
	if (gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1)) {
		goto exit;
	}

	if (gp_parse_did(env, argv[n++], &did_val)) {
		goto exit;
	}

	if (tok_parse_ulonglong(argv[n++], &rio_addr, 1, UINT64_MAX, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULONGLONG_HEX_MSG_FMT, "<rio_addr>",
				(uint64_t)1, (uint64_t)UINT64_MAX);
		goto exit;
	}

	if (tok_parse_ulonglong(argv[n++], &bytes, 1, UINT32_MAX, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULONGLONG_HEX_MSG_FMT, "<bytes>",
				(uint64_t )1, (uint64_t)UINT32_MAX);
		goto exit;
	}

	wkr[idx].action = dma_rx_lat;
	wkr[idx].action_mode = kernel_action;
	wkr[idx].did_val = did_val;
	wkr[idx].rio_addr = rio_addr;
	wkr[idx].byte_cnt = bytes;
	wkr[idx].acc_size = bytes;
	wkr[idx].wr = 1;
	wkr[idx].use_kbuf = 1;
	wkr[idx].dma_trans_type = RIO_EXCHANGE_NWRITE;
	wkr[idx].dma_sync_type = RIO_TRANSFER_SYNC;

	if (bytes < MIN_RDMA_BUFF_SIZE) {
		wkr[idx].rdma_buff_size = MIN_RDMA_BUFF_SIZE;
	} else {
		wkr[idx].rdma_buff_size = bytes;
	}

	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);

exit:
	return 0;
}

struct cli_cmd dmaRxLat = {
"dRxLat",
2,
4,
"Loop back DMA writes for dTxLat command.",
"dRxLat <idx> <did> <rio_addr> <bytes>\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<did>      target device ID\n"
	"<rio_addr> RapidIO memory address to access\n"
	"<bytes>    total bytes to transfer\n"
	"\nNOTE: The dRxLat command must be run before dTxLat!\n",
dmaRxLatCmd,
ATTR_NONE
};

static int dmaRxGoodputCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint16_t idx;
	uint64_t bytes;

	int n = 0;
	if (gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1)) {
		goto exit;
	}

	if (tok_parse_ulonglong(argv[n++], &bytes, 1, UINT32_MAX, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULONGLONG_HEX_MSG_FMT, "<bytes>",
				(uint64_t )1, (uint64_t)UINT32_MAX);
		goto exit;
	}

	wkr[idx].action = dma_rx_gp;
	wkr[idx].action_mode = kernel_action;
	wkr[idx].byte_cnt = bytes;
	wkr[idx].acc_size = bytes;

	if (bytes < MIN_RDMA_BUFF_SIZE) {
		wkr[idx].rdma_buff_size = MIN_RDMA_BUFF_SIZE;
	} else {
		wkr[idx].rdma_buff_size = bytes;
	}

	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);

exit:
	return 0;
}

struct cli_cmd dmaRxGoodput = {
"dRxGoodput",
2,
2,
"Measure bytes received from other nodes DMA.\n",
"dRxLat <idx> <bytes>\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<bytes>    total bytes in each transfer\n"
	"\nNOTE: The dRxGoodput command must be run before dTx!\n",
dmaRxGoodputCmd,
ATTR_NONE
};

static void roundoff_message_size(uint32_t *bytes)
{
	if (*bytes > 4096) {
		*bytes = 4096;
	}

	if (*bytes < 24) {
		*bytes = 24;
	}

	*bytes = (*bytes + 7) & 0x1FF8;
}

static int msg_tx_cmd(struct cli_env *env, int UNUSED(argc), char **argv, enum req_type req)
{
	uint16_t idx;
	did_val_t did_val;
	uint16_t sock_num;
	uint32_t bytes;

	int n = 0;
	if (gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1)) {
		goto exit;
	}

	if (gp_parse_did(env, argv[n++], &did_val)) {
		goto exit;
	}

	if (tok_parse_socket(argv[n++], &sock_num, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_SOCKET_MSG_FMT, "<sock_num>");
		goto exit;
	}

	if (tok_parse_ulong(argv[n++], &bytes, CM_HEADER_BYTES, RIO_MAX_MSG_SIZE,
			0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULONG_HEX_MSG_FMT, "<size>", CM_HEADER_BYTES,
				RIO_MAX_MSG_SIZE);
		goto exit;
	}

	roundoff_message_size(&bytes);

	wkr[idx].action = req;
	wkr[idx].action_mode = kernel_action;
	wkr[idx].did_val = did_val;
	wkr[idx].sock_num = sock_num;
	wkr[idx].msg_size = bytes;

	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);

exit:
	return 0;
}

static int msgTxCmd(struct cli_env *env, int argc, char **argv)
{
	return msg_tx_cmd(env, argc, argv, message_tx);
}

struct cli_cmd msgTx = {
"msgTx",
4,
4,
"Measure goodput of channelized messages",
"msgTx <idx> <did> <sock_num> <size>\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<did>      target device ID\n"
	"<sock_num> RapidIO Channelized Messaging channel number to connect\n"
	"<size>     bytes per message. Must be a multiple of 8 from 24 to 4096\n"
	"\nNOTE: msgTx must send to a corresponding msgRx!\n",
msgTxCmd,
ATTR_NONE
};

static int msgTxLatCmd(struct cli_env *env, int argc, char **argv)
{
	return msg_tx_cmd(env, argc, argv, message_tx_lat);
}

struct cli_cmd msgTxLat = {
"mTxLat",
2,
4,
"Measures latency of channelized messages",
"mTxLat <idx> <did> <sock_num> <size>\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<did>      target device ID\n"
	"<sock_num> RapidIO Channelized Messaging channel number to connect\n"
	"<size>     bytes per message. Must be a multiple of 8 from 24 to 4096\n"
	"\nNOTE: mTxLat must be sending to a node running mRxLat!\n"
	"NOTE: mRxLat must be run before mTxLat!\n",
msgTxLatCmd,
ATTR_NONE
};

static int msgRxCmdExt(struct cli_env *env, int UNUSED(argc), char **argv, enum req_type action)
{
	uint16_t idx;
	uint16_t sock_num;
	uint32_t bytes;

	int n = 0;
	if (gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1)) {
		goto exit;
	}

	if (tok_parse_socket(argv[n++], &sock_num, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_SOCKET_MSG_FMT, "<sock_num>");
		goto exit;
	}

	if (tok_parse_ulong(argv[n++], &bytes, CM_HEADER_BYTES, RIO_MAX_MSG_SIZE,
			0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULONG_HEX_MSG_FMT, "<size>", CM_HEADER_BYTES,
				RIO_MAX_MSG_SIZE);
		goto exit;
	}

	roundoff_message_size(&bytes);

	wkr[idx].action = action;
	wkr[idx].action_mode = kernel_action;
	wkr[idx].did_val = 0;
	wkr[idx].sock_num = sock_num;
	wkr[idx].msg_size = bytes;

	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);

exit:
	return 0;
}

static int msgRxLatCmd(struct cli_env *env, int argc, char **argv)
{
	return msgRxCmdExt(env, argc, argv, message_rx_lat);
}

struct cli_cmd msgRxLat = {
"mRxLat",
2,
3,
"Loops back received messages to mTxLat sender",
"mRxLat <idx> <sock_num> <size>\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<sock_num> RapidIO Channelized Messaging channel number to accept\n"
	"<size>     bytes per message. Must be a multiple of 8 from 24 to 4096\n"
	"\nNOTE: mRxLat must be run before mTxLat!\n",
msgRxLatCmd,
ATTR_NONE
};

static int msgRxCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint16_t idx;
	uint16_t sock_num;

	int n = 0;
	if (gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1)) {
		goto exit;
	}

	if (tok_parse_socket(argv[n++], &sock_num, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_SOCKET_MSG_FMT, "<sock_num>");
		goto exit;
	}

	wkr[idx].action = message_rx;
	wkr[idx].action_mode = kernel_action;
	wkr[idx].did_val = 0;
	wkr[idx].sock_num = sock_num;

	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);

exit:
	return 0;
}

struct cli_cmd msgRx = {
"msgRx",
4,
2,
"Receives channelized messages as requested",
"msgRx <idx> <sock_num>\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<sock_num> target socket number for connections from msgTx command\n"
	"\nNOTE: msgRx must be running before msgTx!\n",
msgRxCmd,
ATTR_NONE
};

static int msgTxOhCmd(struct cli_env *env, int argc, char **argv)
{
	return msg_tx_cmd(env, argc, argv, message_tx_oh);
}

struct cli_cmd msgTxOh = {
"mTxOh",
4,
4,
"Measures overhead of channelized messages",
"mTxOh <idx> <did> <sock_num> <size>\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<did>      target device ID\n"
	"<sock_num> RapidIO Channelized Messaging channel number to connect\n"
	"<size>     bytes per message. Must be a multiple of 8 from 24 to 4096\n"
	"\nNOTE: mTxOh must be sending to a node running mRxOh!\n"
	"NOTE: mRxOh must be run before mTxOh!\n",
msgTxOhCmd,
ATTR_NONE
};

static int msgRxOhCmd(struct cli_env *env, int argc, char **argv)
{
	return msgRxCmdExt(env, argc, argv, message_rx_oh);
}

struct cli_cmd msgRxOh = {
"mRxOh",
4,
3,
"Loops back received messages to mTxOh sender",
"mRxOh <idx> <sock_num> <size>\n"
	"<idx>      is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<sock_num> RapidIO Channelized Messaging channel number to accept\n"
	"<size>     bytes per message. Must be a multiple of 8 from 24 to 4096\n"
	"\nNOTE: mRxOh must be run before mTxOh!\n",
msgRxOhCmd,
ATTR_NONE
};

static int GoodputCmd(struct cli_env *env, int argc, char **UNUSED(argv))
{
	int i;
	float MBps;
	float Gbps;
	float Msgpersec;
	float link_occ;
	uint64_t byte_cnt;
	float tot_MBps = 0;
	float tot_Gbps = 0;
	float tot_Msgpersec = 0;
	uint64_t tot_byte_cnt = 0;
	char MBps_str[FLOAT_STR_SIZE];
	char Gbps_str[FLOAT_STR_SIZE];
	char link_occ_str[FLOAT_STR_SIZE];

	LOGMSG(env, "\n W STS <<<<--Data-->>>> --MBps-- -Gbps- Messages  Link_Occ\n");

	for (i = 0; i < MAX_WORKERS; i++) {
		struct timespec elapsed;
		uint64_t nsec;

		Msgpersec = wkr[i].perf_msg_cnt;
		byte_cnt = wkr[i].perf_byte_cnt;

		elapsed = time_difference(wkr[i].st_time, wkr[i].end_time);
		nsec = elapsed.tv_nsec + (elapsed.tv_sec * 1000000000);

		MBps = (float)(byte_cnt / (1024*1024)) /
			((float)nsec / 1000000000.0);
		Gbps = (MBps * 1024.0 * 1024.0 * 8.0) / 1000000000.0;
		link_occ = Gbps/0.95;

		memset(MBps_str, 0, FLOAT_STR_SIZE);
		memset(Gbps_str, 0, FLOAT_STR_SIZE);
		memset(link_occ_str, 0, FLOAT_STR_SIZE);
		snprintf(MBps_str, sizeof(MBps_str), "%4.3f", MBps);
		snprintf(Gbps_str, sizeof(Gbps_str), "%2.3f", Gbps);
		snprintf(link_occ_str, sizeof(link_occ_str), "%2.3f", link_occ);

		LOGMSG(env, "%2d %3s %16lx %8s %6s %9.0f  %6s\n", i,
				THREAD_STR(wkr[i].stat), byte_cnt, MBps_str,
				Gbps_str, Msgpersec, link_occ_str);

		if (byte_cnt) {
			tot_byte_cnt += byte_cnt;
			tot_MBps += MBps;
			tot_Gbps += Gbps;
		}
		tot_Msgpersec += Msgpersec;

		if (argc) {
			wkr[i].perf_byte_cnt = 0;
			wkr[i].perf_msg_cnt = 0;
			clock_gettime(CLOCK_MONOTONIC, &wkr[i].st_time);
		}
	}

	link_occ = tot_Gbps/0.95;
	memset(MBps_str, 0, FLOAT_STR_SIZE);
	memset(Gbps_str, 0, FLOAT_STR_SIZE);
	memset(link_occ_str, 0, FLOAT_STR_SIZE);
	snprintf(MBps_str, sizeof(MBps_str), "%4.3f", tot_MBps);
	snprintf(Gbps_str, sizeof(Gbps_str), "%2.3f", tot_Gbps);
	snprintf(link_occ_str, sizeof(link_occ_str), "%2.3f", link_occ);
	LOGMSG(env, "Total  %16lx %8s %6s %9.0f  %6s\n", tot_byte_cnt, MBps_str,
			Gbps_str, tot_Msgpersec, link_occ_str);

	return 0;
}

struct cli_cmd Goodput = {
"goodput",
1,
0,
"Print current performance for threads.",
"goodput {<optional>}\n"
	"Any parameter to goodput causes the byte and message counts of all\n"
	"   running threads to be zeroed after they are displayed\n",
GoodputCmd,
ATTR_RPT
};

static int LatCmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
	int i;
	char min_lat_str[FLOAT_STR_SIZE];
	char avg_lat_str[FLOAT_STR_SIZE];
	char max_lat_str[FLOAT_STR_SIZE];

	LOGMSG(env, "\n W STS <<<<-Count-->>>> <<<<Min uSec>>>> <<<<Avg uSec>>>> <<<<Max uSec>>>>\n");

	for (i = 0; i < MAX_WORKERS; i++) {
		uint64_t tot_nsec;
		uint64_t avg_nsec;
		uint64_t divisor;

		divisor = (wkr[i].wr)?2:1;

		tot_nsec = wkr[i].tot_iter_time.tv_nsec +
				(wkr[i].tot_iter_time.tv_sec * 1000000000);

		/* Note: divide by 2 to account for round trip latency. */
		if (wkr[i].perf_iter_cnt) {
			avg_nsec = tot_nsec/divisor/wkr[i].perf_iter_cnt;
		} else {
			avg_nsec = 0;
		}

		memset(min_lat_str, 0, FLOAT_STR_SIZE);
		memset(avg_lat_str, 0, FLOAT_STR_SIZE);
		memset(max_lat_str, 0, FLOAT_STR_SIZE);
		snprintf(min_lat_str, sizeof(min_lat_str), "%4.3f",
			(float)(wkr[i].min_iter_time.tv_nsec/divisor)/1000.0);
		snprintf(avg_lat_str, sizeof(avg_lat_str), "%4.3f", (float)avg_nsec/1000.0);
		snprintf(max_lat_str, sizeof(max_lat_str), "%4.3f",
			(float)(wkr[i].max_iter_time.tv_nsec/divisor)/1000.0);

		LOGMSG(env, "%2d %3s %16ld %16s %16s %16s\n", i,
				THREAD_STR(wkr[i].stat), wkr[i].perf_iter_cnt,
				min_lat_str, avg_lat_str, max_lat_str);
	}

	return 0;
}

struct cli_cmd Lat = {
"lat",
3,
0,
"Print current latency for threads.",
"<No Parameters>\n",
LatCmd,
ATTR_RPT
};


static inline void display_cpu(struct cli_env *env, int cpu)
{
	if (-1 == cpu) {
		LOGMSG(env, "Any ");
		return;
	}
	LOGMSG(env, "%3d ", cpu);
}

static void display_gen_status(struct cli_env *env)
{
	LOGMSG(env, "\n W STS CPU RUN ACTION  MODE DID LDID <<<<--ADDR-->>>> ByteCnt AccSize W H OB IB MB\n");

	for (int i = 0; i < MAX_WORKERS; i++) {
		LOGMSG(env, "%2d %3s ", i, THREAD_STR(wkr[i].stat));
		display_cpu(env, wkr[i].wkr_thr.cpu_req);
		display_cpu(env, wkr[i].wkr_thr.cpu_run);
		LOGMSG(env,
			"%7s %4s %3d %4d %16lx %7lx %7lx %1d %1d %2d %2d %2d\n",
			ACTION_STR(wkr[i].action),
			MODE_STR(wkr[i].action_mode), wkr[i].did_val,
			wkr[i].prev_did_val,
			wkr[i].rio_addr, wkr[i].byte_cnt, wkr[i].acc_size,
			wkr[i].wr, wkr[i].mp_h_is_mine,
			wkr[i].ob_valid, wkr[i].ib_valid,
			wkr[i].mb_valid);
	}
}

static void display_ibwin_status(struct cli_env *env)
{
	LOGMSG(env, "\n W STS CPU RUN ACTION  MODE IB <<<< HANDLE >>>> <<<<RIO ADDR>>>> <<<<  SIZE  >>>\n");

	for (int i = 0; i < MAX_WORKERS; i++) {
		LOGMSG(env, "%2d %3s ", i, THREAD_STR(wkr[i].stat));
		display_cpu(env, wkr[i].wkr_thr.cpu_req);
		display_cpu(env, wkr[i].wkr_thr.cpu_run);
		LOGMSG(env,
			"%7s %4s %2d %16lx %16lx %15lx\n",
			ACTION_STR(wkr[i].action),
			MODE_STR(wkr[i].action_mode),
			wkr[i].ib_valid, wkr[i].ib_handle, wkr[i].ib_rio_addr,
			wkr[i].ib_byte_cnt);
	}
}

static void display_msg_status(struct cli_env *env)
{
	int i;

	LOGMSG(env,
	"\n W STS CPU RUN ACTION  MODE MB ACC CON Msg_Size SockNum TX RX\n");

	for (i = 0; i < MAX_WORKERS; i++) {
		LOGMSG(env, "%2d %3s ", i, THREAD_STR(wkr[i].stat));
		display_cpu(env, wkr[i].wkr_thr.cpu_req);
		display_cpu(env, wkr[i].wkr_thr.cpu_run);
		LOGMSG(env,
			"%7s %4s %2d %3d %3d %8d %7d %2d %2d\n",
			ACTION_STR(wkr[i].action),
			MODE_STR(wkr[i].action_mode),
			wkr[i].mb_valid, wkr[i].acc_skt_valid,
			wkr[i].con_skt_valid, wkr[i].msg_size,
			wkr[i].sock_num, (NULL != wkr[i].sock_tx_buf),
			(NULL != wkr[i].sock_rx_buf)
		);
	}
}

static int StatusCmd(struct cli_env *env, int argc, char **argv)
{
	char sel_stat = 'g';

	if (argc) {
		sel_stat = argv[0][0];
	}

	switch (sel_stat) {
		case 'i':
		case 'I':
			display_ibwin_status(env);
			break;
		case 'm':
		case 'M':
			display_msg_status(env);
			break;
		case 'g':
		case 'G':
			display_gen_status(env);
			break;
		default:
			LOGMSG(env, "Unknown option \"%c\"\n", argv[0][0]);
			return 0;
	}

	return 0;
}

struct cli_cmd Status = {
"status",
2,
0,
"Display status of all threads",
"status {i|m|g}\n"
	"Optionally enter a character to select the status type:\n"
	"i : IBWIN status\n"
	"m : Messaging status\n"
	"g : General status\n"
	"u : user mode driver activity status"
	"Default is general status\n",
StatusCmd,
ATTR_RPT
};

int dump_idx;
uint64_t dump_base_offset;
uint64_t dump_size;

static int DumpCmd(struct cli_env *env, int argc, char **argv)
{
	uint16_t idx;
	uint64_t offset, base_offset;
	uint64_t size;
	int n = 0;

	if (argc) {
		if (gp_parse_worker_index(env, argv[n++], &idx)) {
			goto exit;
		}

		if (tok_parse_ull(argv[n++], &base_offset, 0)) {
			LOGMSG(env, "\n");
			LOGMSG(env, TOK_ERR_ULL_HEX_MSG_FMT, "<base_offset>");
			goto exit;
		}

		if (tok_parse_ull(argv[n++], &size, 0)) {
			LOGMSG(env, "\n");
			LOGMSG(env, TOK_ERR_ULL_HEX_MSG_FMT, "<size>");
			goto exit;
		}
	} else {
		idx = dump_idx;
		base_offset = dump_base_offset;
		size = dump_size;
	}

	if (!wkr[idx].ib_valid || (NULL == wkr[idx].ib_ptr)) {
		LOGMSG(env, "\nNo mapped inbound window present\n");
		goto exit;
	}

	if ((base_offset + size) > wkr[idx].ib_byte_cnt) {
		LOGMSG(env, "\nOffset + size exceeds window bytes\n");
		goto exit;
	}

	dump_idx = idx;
	dump_base_offset = base_offset;
	dump_size = size;

	LOGMSG(env,
		"          Offset 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
	for (offset = 0; offset < size; offset++) {
		if (!(offset & 0xF)) {
			LOGMSG(env, "\n%" PRIx64 "", base_offset + offset);
		}
		LOGMSG(env, " %2x",
			*((uint8_t *)wkr[idx].ib_ptr + base_offset + offset));
	}
	LOGMSG(env, "\n");

exit:
	return 0;
}

struct cli_cmd Dump = {
"dump",
2,
3,
"Dump inbound memory area",
"Dump <idx> <offset> <size>\n"
	"<idx>    is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<offset> is the hexadecimal offset, in bytes, from the window start\n"
	"<size>   is the number of bytes to display, starting at <offset>\n",
DumpCmd,
ATTR_RPT
};

static int FillCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint16_t idx;
	uint64_t offset, base_offset;
	uint64_t size;
	uint16_t tmp;
	uint8_t data;

	int n = 0;
	if (gp_parse_worker_index(env, argv[n++], &idx)) {
		goto exit;
	}

	if (tok_parse_ull(argv[n++], &base_offset, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULL_HEX_MSG_FMT, "<offset>");
		goto exit;
	}

	if (tok_parse_ull(argv[n++], &size, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_ULL_HEX_MSG_FMT, "<size>");
		goto exit;
	}

	if (tok_parse_ushort(argv[n++], &tmp, 0, 0xff, 0)) {
		LOGMSG(env, "\n");
		LOGMSG(env, TOK_ERR_USHORT_HEX_MSG_FMT, "<data>", 0, 0xff);
		goto exit;
	}
	data = (uint8_t)tmp;

	if (!wkr[idx].ib_valid || (NULL == wkr[idx].ib_ptr)) {
		LOGMSG(env, "\nNo mapped inbound window present\n");
		goto exit;
	}

	if ((base_offset + size) > wkr[idx].ib_byte_cnt) {
		LOGMSG(env, "\nOffset + size exceeds window bytes\n");
		goto exit;
	}

	dump_idx = idx;
	dump_base_offset = base_offset;
	dump_size = size;

	for (offset = 0; offset < size; offset++) {
		*((uint8_t *)wkr[idx].ib_ptr + base_offset + offset) = data;
	}

exit:
	return 0;
}

struct cli_cmd Fill = {
"fill",
4,
4,
"Fill inbound memory area",
"Fill <idx> <offset> <size> <data>\n"
	"<idx>    is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
	"<offset> is the offset, in bytes, from the window start\n"
	"<size>   is the number of bytes to display, starting at <offset>\n"
	"<data>   is the 8 bit value to write\n",
FillCmd,
ATTR_RPT
};

uint32_t acc_offset = 0;

static int LReadCmd(struct cli_env *env, int argc, char **argv)
{
	const uint32_t max_offset = 0xFFFFFC;
	uint32_t offset;
	uint32_t data;
	int rc;
	int n = 0;

	// Valid register offsets run from 0 to 0xFFFFFC, since
	// this routine uses 4 byte register writes.
	if (argc) {
		if (tok_parse_ulong(argv[n], &offset, 0, max_offset, 0)) {
			LOGMSG(env, "ERR: %s is not a valid offset\n",
					argv[n]);
			goto exit;
		}
	} else {
		offset = acc_offset;
	}

	// Ensure offset is a multiple of 4
	offset &= max_offset;

	rc = rio_lcfg_read(mp_h, offset, 4, &data);
	acc_offset = offset;

	if (rc) {
		LOGMSG(env, "Read failed rc: %d\n", rc);
		goto exit;
	}

	LOGMSG(env, "Read value: 0x%08x\n", data);
exit:
	return 0;
}

struct cli_cmd LRead = {
"lread",
2,
1,
"Display local mport register",
"<offset>\n"
"Read local mport register at <offset>\n"
"offset : offset of register to read.\n",
LReadCmd,
ATTR_RPT
};

uint32_t acc_data = 0;

static int LWriteCmd(struct cli_env *env, int argc, char **argv)
{
	const uint32_t max_offset = 0xFFFFFC;
	uint32_t offset;
	uint32_t data;
	int rc;
	int n = 0;

	// Valid register offsets run from 0 to 0xFFFFFC, since
	// this routine uses 4 byte register writes.
	if (argc) {
		if (tok_parse_ulong(argv[n], &offset, 0, max_offset, 0)) {
			LOGMSG(env, "ERR: %s is not a valid offset\n",
					argv[n]);
			goto exit;
		}
		n++;
		if (tok_parse_ulong(argv[n], &data, 0, 0xFFFFFFFF, 0)) {
			LOGMSG(env, "ERR: %s is not a valid data value\n",
					argv[n]);
			goto exit;
		}
	} else {
		offset = acc_offset;
		data = acc_data;
	}
	// Ensure offset is a multiple of 4
	offset &= max_offset;

	rc = rio_lcfg_write(mp_h, offset, 4, data);

	if (rc) {
		LOGMSG(env, "Write failed rc: %d\n", rc);
		goto exit;
	}

	LOGMSG(env, "Write 0x%08x successful.\n", data);
	acc_offset = offset;
	acc_data = data;

	rc = rio_lcfg_read(mp_h, offset, 4, &data);

	if (rc) {
		LOGMSG(env, "Read failed rc: %d\n", rc);
		goto exit;
	}
	LOGMSG(env, "Read  0x%08x.\n", data);

exit:
	return 0;
}

struct cli_cmd LWrite = {
"lwrite",
2,
2,
"Write and read back local mport register",
"<offset> <value> \n"
"Write <value> to local mport register at <offset>\n"
"offset : offset of register to write.\n"
"value  : value to write to the register.\n",
LWriteCmd,
ATTR_NONE
};

uint32_t racc_hc = 0x00;
uint32_t racc_did = 0x1;

static int RReadCmd(struct cli_env *env, int argc, char **argv)
{
	const uint32_t max_offset = 0xFFFFFC;
	uint32_t offset = 0;
	did_val_t did_val = 1;
	hc_t hc = 0;
	uint32_t data;
	int rc;
	int n = 0;

	// Valid register offsets run from 0 to 0xFFFFFC, since
	// this routine uses 4 byte register writes.
	if (argc) {
		if (tok_parse_ulong(argv[n], &offset, 0, max_offset, 0)) {
			LOGMSG(env, "ERR: %s is not a valid offset\n",
					argv[n]);
			goto exit;
		}
		n++;
		if (n < argc) {
			if (tok_parse_did(argv[n], &did_val, 0)) {
				LOGMSG(env, "ERR: %s is not a valid destination ID\n",
						argv[n]);
				goto exit;
			}
		}
		n++;
		if (n < argc) {
			if (tok_parse_hc(argv[n], &hc, 0)) {
				LOGMSG(env, "ERR: %s is not a valid hopcount\n",
						argv[n]);
				goto exit;
			}
		}
	} else {
		offset = acc_offset;
		hc = racc_hc;
		did_val = racc_did;
	}
	// Ensure offset is a multiple of 4
	offset &= max_offset;

        DBG("\tWaiting for tsi721_mutex\n");
	sem_wait(&tsi721_mutex);

	rc = rio_maint_read(mp_h, did_val, hc, offset, 4, &data);

        DBG("\tPOST tsi721_mutex\n");
	sem_post(&tsi721_mutex);

	if (rc) {
		LOGMSG(env, "Read failed rc: %d\n", rc);
		goto exit;
	}

	LOGMSG(env, "Read value: 0x%08x\n", data);
	acc_offset = offset;
	racc_hc = hc;
	racc_did = did_val;
exit:
	return 0;
}

struct cli_cmd RRead = {
"rread",
2,
1,
"Display remote device register",
"<offset> {<did> {<hc>}}\n"
"Maintenance read register at <offset> on device at <did> <hc>.\n"
"offset : Hex offset of register to read.\n"
"did    : Device ID, default is 0\n"
"hc     : Hop count, default is 0\n",
RReadCmd,
ATTR_RPT
};

static int RWriteCmd(struct cli_env *env, int argc, char **argv)
{
	const uint32_t max_offset = 0xFFFFFC;
	uint32_t offset;
	uint32_t data;
	did_val_t did_val = 1;
	hc_t hc = 0;
	int rc;
	int n = 0;

	// Valid register offsets run from 0 to 0xFFFFFC, since
	// this routine uses 4 byte register writes.
	if (argc) {
		if (tok_parse_ulong(argv[n], &offset, 0, max_offset, 0)) {
			LOGMSG(env, "ERR: %s is not a valid offset\n",
					argv[n]);
			goto exit;
		}
		n++;
		if (tok_parse_ulong(argv[n], &data, 0, 0xFFFFFFFF, 0)) {
			LOGMSG(env, "ERR: %s is not a valid data value\n",
					argv[n]);
			goto exit;
		}
		n++;
		if (n < argc) {
			if (tok_parse_did(argv[n], &did_val, 0)) {
				LOGMSG(env, "ERR: %s is not a valid destination ID\n",
						argv[n]);
				goto exit;
			}
		}
		n++;
		if (n < argc) {
			if (tok_parse_hc(argv[n], &hc, 0)) {
				LOGMSG(env, "ERR: %s is not a valid hopcount\n",
						argv[n]);
				goto exit;
			}
		}
	} else {
		offset = acc_offset;
		data = acc_data;
		hc = racc_hc;
		did_val = racc_did;
	}

	// Ensure offset is a multiple of 4
	offset &= max_offset;

        DBG("\tWaiting for tsi721_mutex\n");
	sem_wait(&tsi721_mutex);
	rc = rio_maint_write(mp_h, did_val, hc, offset, 4, data);
        DBG("\tPOST tsi721_mutex\n");
	sem_post(&tsi721_mutex);

	if (rc) {
		LOGMSG(env, "Write failed rc: %d\n", rc);
		goto exit;
	}

	LOGMSG(env, "Write 0x%08x successful.\n", data);
	acc_offset = offset;
	acc_data = data;
	racc_hc = hc;
	racc_did = did_val;

exit:
	return 0;
}

struct cli_cmd RWrite = {
"rwrite",
2,
2,
"Write and read back remote device register",
"<offset> <value> {<did> {<hc>}}\n"
"Maintenance write <value> at <offset> with specified device ID and hopcount.\n"
"offset : offset of register to write.\n"
"value  : value to write to the register.\n"
"did    : Device ID, default is 0\n"
"hc     : Hop count, default is 0\n",
RWriteCmd,
ATTR_NONE
};

static int SevenTestCmd(struct cli_env *env, int argc, char **argv)
{
	int rc;
	int n = 0;
	uint32_t period  = 30;
	uint32_t downtime = 10;
	uint32_t to_time = 5;
	uint32_t err_time = 5;
	did_val_t mast_did = 2;
	hc_t mast_hc = 1;
	float delay = 0;
	uint16_t idx;
	req_type req;

	rc = gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1);
	if (rc) {
		LOGMSG(env, "Need index of a halted thread.\n");
		goto exit;
	}
	switch (argv[n++][0]) {
	case 'I':
	case 'i': req = seven_test_721I;
		break;
	case 'P':
	case 'p': req = seven_test_pw_rx;
		break;
	default:
		LOGMSG(env, "Unknown action %s\n", argv[n]);
		goto exit;
	}
	if (n < argc) {
		rc = tok_parse_ul(argv[n], &period, 10);
		if (rc) {
			LOGMSG(env, "Unrecognized period value: %s\n", argv[n]);
			goto exit;
		}
	}
	n++;
	if (n < argc) {
		rc = tok_parse_ul(argv[n], &downtime, 10);
		if (rc) {
			LOGMSG(env, "Unrecognized downtime value: %s\n", argv[n]);
			goto exit;
		}
	}
	n++;
	if (n < argc) {
		rc = tok_parse_ul(argv[n], &to_time, 10);
		if (rc) {
			LOGMSG(env, "Unrecognized TOTime value: %s\n", argv[n]);
			goto exit;
		}
	}

	n++;
	if (n < argc) {
		rc = tok_parse_ul(argv[n], &err_time, 10);
		if (rc) {
			LOGMSG(env, "Unrecognized ErrTime value: %s\n", argv[n]);
			goto exit;
		}
	}

	n++;
	if (n < argc) {
		rc = tok_parse_did(argv[n], &mast_did, 10);
		if (rc) {
			LOGMSG(env, "Unrecognized MastDID value: %s\n", argv[n]);
			goto exit;
		}
	}

	n++;
	if (n < argc) {
		rc = tok_parse_hc(argv[n], &mast_hc, 10);
		if (rc) {
			LOGMSG(env, "Unrecognized MastHC value: %s\n", argv[n]);
			goto exit;
		}
	}

	n++;
	if (n < argc) {
		rc = tok_parse_f(argv[n], &delay);
		if (rc) {
			LOGMSG(env, "Unrecognized Delay value: %s\n", argv[n]);
			goto exit;
		}
	}

	if ((err_time + to_time + downtime) >= period) {
		LOGMSG(env, "Times don't add up.\n");
		goto exit;
	}

	if (delay > (float)(period)) {
		LOGMSG(env, "Delay must be less than period.\n");
		goto exit;
	}

	// The link should recover itself gracefully.
	wkr[idx].action = req;
	wkr[idx].action_mode = kernel_action;
	wkr[idx].seven_test_period = period;
	wkr[idx].seven_test_downtime = downtime;
	wkr[idx].seven_test_err_resp_time = err_time;
	wkr[idx].seven_test_resp_to_time = to_time;
	wkr[idx].did_val = mast_did;
	wkr[idx].hc = mast_hc;
	wkr[idx].seven_test_delay = delay;

	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);
exit:
	return 0;
}

struct cli_cmd SevenTest = {
"7Test",
2,
2,
"Tsi721 Test commands for Tsi721 <=> CPS configuration.",
"<idx> <option> {<Period> {<DownTime> <TOTime> <ErrTime>}\n"
"Fault insertion and CPS switch management tasks.\n"
"<idx>   : halted thread that will run the test.\n"
"<option>: P : Receive and display all port-writes and local interrupts.\n"
"              Do not handle the port-writes or interrupts.\n"
"          I : Tsi721 fault insertion.  Requires <Period>, <DownTime>\n"
"              <TOTime> <ErrTime> <MasterDid> <MasterHC> <Delay>.\n"
"Period  : Time in seconds between attempts to kill the link, default is 30.\n"
"          Downtime, TOTime, and ErrTime must add up to less than Period.\n"
"DownTime: Time in seconds that the link will be down, default is 10.\n"
"TOTime  : Time in seconds that this Tsi721 will see response timeouts.\n"
"          Filters all responses using TLM FILTER (offset 0x103e0)\n"
"          Default is 5 seconds.\n"
"ErrTime : Time in seconds that other Tsi721s will see error respones.\n"
"          Corrupts the address of inbound window 0 (offset 0x29000)\n"
"          Default is 5 seconds.\n"
"MastDID : Destination ID of first endpoint to run fault insertion.\n"
"MastHC  : Hopcount to read registers on MastDID device.\n"
"Delay   : Delay from start of Mast DID fault insertion to start of fault\n"
"          insertion on this device.\n",
SevenTestCmd,
ATTR_NONE
};

static int CPSHotSwapCmd(struct cli_env *env, int argc, char **argv)
{
	int rc;
	uint16_t idx;
	uint16_t cps_wkr;
	uint16_t tsi_wkr;
	req_type req;

	rc = gp_parse_worker_index_check_thread(env, argv[0], &idx, 1);
	if (rc) {
		LOGMSG(env, "Need index of a halted thread.\n");
		goto exit;
	}

	switch (argv[1][0]) {
	case 'T' : req = cps_handle_721_pw;
		break;
	case 'C' : req = cps_handle_cps_pw;
		break;
	case 'R' : req = cps_poll_for_pw;
		break;
	case 'M' : req = cps_test_switch_lock;
		break;
	case 'P' : req = seven_test_switch_mgmt;
		if (argc < 4) {
			LOGMSG(env, "Missing parameters.\n");
			goto exit;
		}
		break;
	default: LOGMSG(env, "Unknown option %s\n", argv[1]);
		goto exit;
	}

	if ((seven_test_switch_mgmt == req) || (cps_poll_for_pw == req)) {
		rc = gp_parse_worker_index_check_thread(env, argv[2], &tsi_wkr, 2);
		if (rc) {
			LOGMSG(env, "Need index of a running thread.\n");
			goto exit;
		}
	}
	if (seven_test_switch_mgmt == req) {
		rc = gp_parse_worker_index_check_thread(env, argv[3], &cps_wkr, 2);
		if (rc) {
			LOGMSG(env, "Need index of a running thread.\n");
			goto exit;
		}
	}

	// The link should recover itself gracefully.
	wkr[idx].action = req;
	wkr[idx].action_mode = kernel_action;
	l_init(&wkr[idx].pw);
	sem_init(&wkr[idx].pw_mutex, 0, 1);
	sem_init(&wkr[idx].process_pw, 0, 0);

	if (seven_test_switch_mgmt == req) {
		wkr[idx].hs_worker[TSI721_I] = &wkr[tsi_wkr];
		wkr[idx].hs_worker[CPS_I] = &wkr[cps_wkr];
	}
	if (cps_poll_for_pw == req)
		wkr[idx].hs_worker[CPS_I] = &wkr[tsi_wkr];

	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);
exit:
	return 0;
}

struct cli_cmd CPSHotSwap = {
"CHotSwap",
2,
2,
"CPS Hot Swap port write handling configuration.",
"<idx> <option> {<tsi_wkr> <cps_wkr>}\n"
"CPS Hot Swap port-write handling configuration.\n"
"<idx>    : halted thread that will run the option.\n"
"<option> : T : Start Tsi721 port-write handler for CPS configuration.\n"
"           C : Start the CPS port-write handler.\n"
"           R : Start the CPS port-write polling thread.\n"
"           M : Start switch lock (mutex) test.\n"
"           P : Start the CPS port-write receiving worker.\n"
"               The CPS port-write receiving worker distributes\n"
"               port-writes to the Tsi721 and CPS port-write handlers.\n"
"<cps_wkr>: Required for option 'P', worker index of previously\n"
"           started 'C' worker.\n"
"<tsi_wkr>: Required for option 'P', worker index of previously\n"
"           started 'T' worker.\n",
CPSHotSwapCmd,
ATTR_NONE
};

static int MaintTrafficCmd(struct cli_env *env, int argc, char **argv)
{
	int rc;
	int n = 0;
	uint16_t idx;

	rc = gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1);
	if (rc) {
		LOGMSG(env, "Need index of a halted thread.\n");
		goto exit;
	}

	wkr[idx].did_val = 0;
	wkr[idx].hc = 0;

	if (argc > 1) {
		if (tok_parse_did(argv[1], &wkr[idx].did_val, 0)) {
			LOGMSG(env, TOK_ERR_DID_MSG_FMT);
			goto exit;
		}
	}

	if (argc > 2) {
		if (tok_parse_hc(argv[2], &wkr[idx].hc, 0)) {
			LOGMSG(env, TOK_ERR_HC_MSG_FMT);
			goto exit;
		}
	}

	wkr[idx].action = maint_traffic;
	wkr[idx].action_mode = kernel_action;

	wkr[idx].stop_req = 0;
	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);
exit:
	return 0;
}

struct cli_cmd MaintTraffic = {
"maint",
2,
1,
"Generate maintenance read traffic to specified destination.\n",
"<idx> {<destid> {<hopcount>}}\n"
"Perform continuous maintenance transactions to specified destination.\n"
"<idx>     : halted thread that will generation maintenance reads.\n"
"<destid>  : Destination ID for maintenance reads.\n"
"            Default value is 0.\n"
"<hopcount>: Hop count for maintenance reads.\n"
"            Default value is 0.\n",
MaintTrafficCmd,
ATTR_NONE
};


static int SevenHotSwapCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	int rc;
	int n = 0;
	uint16_t idx, wkr_idx;

	rc = gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1);
	if (rc) {
		LOGMSG(env, "Need index of a halted thread.\n");
		goto exit;
	}
	rc = gp_parse_worker_index_check_thread(env, argv[n++], &wkr_idx, 1);
	if (rc) {
		LOGMSG(env, "Worker needs index of a halted thread.\n");
		goto exit;
	}
	if (idx == wkr_idx) {
		LOGMSG(env, "Index and worker index must be different.\n");
		goto exit;
	}

	wkr[idx].action = seven_test_721_recovery;
	wkr[idx].action_mode = kernel_action;
	wkr[idx].hs_worker[TSI721_I] = &wkr[wkr_idx];

	wkr[idx].stop_req = 0;
	wkr[wkr_idx].stop_req = 0;
	sem_post(&wkr[idx].run);
exit:
	return 0;
}

struct cli_cmd SevenHotSwap = {
"7HotSwap",
2,
2,
"Hot Swap handler for Tsi721<=>Tsi721 link config.",
"<idx> <wkr_idx>\n"
"Handle Dead Link Timer and Link Init events for connected Tsi721's.\n"
"<idx>    : halted thread that will receive Tsi721 port-writes\n"
"<wkr_idx>: halted thread that will process Tsi721 link initialization\n",
SevenHotSwapCmd,
ATTR_NONE
};

static int SevenStatusCmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
	struct {
		uint32_t oset;
		char *lable;
	} regs[] = {
	{TSI721_SP_GEN_CTL, (char *)"GEN_CTL"},
	{TSI721_SP_ACKID_STAT, (char *)"ACKID"},
	{TSI721_SP_ERR_STAT, (char *)"ERR STAT"},
	{TSI721_SP_CTL, (char *)"CTL"},
	{TSI721_SP_ERR_DET, (char *)"ERR DET"},
	{TSI721_PLM_IMP_SPEC_CTL, (char *)"PLM CTL "},
	{TSI721_PLM_STATUS, (char *)"PLM STAT"},
	{TSI721_PLM_INT_ENABLE, (char *)"PlmIntEn"},
	{TSI721_PLM_ALL_INT_EN, (char *)"PlmAllIE"},
	{TSI721_TLM_SP_STATUS, (char *)"TLM STAT"},
	{TSI721_PBM_SP_STATUS, (char *)"PBM STAT"},
	{TSI721_TXPKT_SMSG_CNT, (char *)"TX MSG"},
	{TSI721_TXPKT_BDMA_CNT, (char *)"TX DMA"},
	{TSI721_TXPKT_BRG_CNT, (char *)"TX BRG"},
	{TSI721_RXPKT_SMSG_CNT, (char *)"RX MSG"},
	{TSI721_RXRSP_BDMA_CNT, (char *)"RX DMA"},
	{TSI721_RXPKT_BRG_CNT, (char *)"RX BRG"},
	{TSI721_BRG_PKT_ERR_CNT, (char *)"RXERR BRG"},
	{TSI721_IBWIN_LBX(0), (char *)"IBW0 LB"},
	{TSI721_TLM_SP_FTYPE_FILTER_CTL, (char *)"FILTER"},
	{TSI721_PCIEDCTL, (char *)"PCIE ST"},
};
	int got_one = 0;

	for (int i = 0; i < sizeof(regs)/sizeof(regs[0]); i++) {
		uint32_t data;
		rio_lcfg_read(mp_h, regs[i].oset, 4, &data);
		LOGMSG(env, "0x%9x %10s : 0x%8x\n",
				regs[i].oset, regs[i].lable, data);
	}
	LOGMSG(env, "PORT_OK  : %d\n", wkr[0].port_ok);
	LOGMSG(env, "PORT STAT: ");
	// 18 is the maximum possible destID for IDT switches.
	for (int i = 1; i < MAX_DEVID_STATUS; i++) {
		if (devid_status[i]) {
			if (got_one) {
				LOGMSG(env, ", %d", i - 1);
			} else {
				got_one = 1;
				LOGMSG(env, "%d", i - 1);
			}
		}
	}
	if (got_one) {
		LOGMSG(env, "\n");
	} else {
		LOGMSG(env, "No valid ports.\n");
	}

	return 0;
}

struct cli_cmd SevenStatus = {
"7status",
2,
0,
"Print Tsi721 status registers for RapidIO link.",
"<No parameters>\n"
"Print Tsi721 status registers for RapidIO link.\n"
"Registers up to PBM STAT are error status and configuration registers.\n"
"TX MSG, TX DMA, TX BRG count packets sent using the messaging engines,\n"
"                     DMA Engines, or the PCIe-to-RapidIO translation bridge\n"
"RX MSG counts responses and message requests received\n"
"RX DMA counts responses received for DMA requests.\n"
"RX BRG counts read/write packets received.\n"
"RXERR BRG counts received packets that cause an ERROR response.\n"
"IBW0 LB and FILTER indicate fault insertion status.\n",
SevenStatusCmd,
ATTR_RPT
};

static int CPSStatusCmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
	struct {
		uint32_t oset;
		uint32_t pp_oset;
		char *lable;
		int no_zeros;
	} regs[] = {
	{CPS1848_PORT_X_LOCAL_ACKID_CSR(0), 0x20, (char *)"ACKIDs  ", 0},
	{CPS1848_PORT_X_ERR_STAT_CSR(0), 0x20, (char *)"ERR_STAT", 0},
	{CPS1848_PORT_X_CTL_1_CSR(0), 0x20, (char *)"CTL", 0},
	{CPS1848_PORT_X_OPS(0), 0x100, (char *)"OPS", 0},
	{CPS1848_PORT_X_ERR_DET_CSR(0), 0x40, (char *)"ERR_DET", 0},
	{CPS1848_PORT_X_ERR_RATE_EN_CSR(0), 0x40, (char *)"RATE_EN", 0},
	{CPS1848_PORT_X_ERR_RPT_EN(0), 0x100, (char *)"ErrRptE", 0},
	{CPS1848_PORT_X_ERR_RATE_CSR(0), 0x40, (char *)"ERRRATE", 0},
	{CPS1848_PORT_X_ERR_RATE_THRESH_CSR(0), 0x40, (char *)"ERR_THR", 0},
	{CPS1848_PORT_X_IMPL_SPEC_ERR_DET(0), 0x100, (char *)"I_ERRDET", 0},
	{CPS1848_PORT_X_IMPL_SPEC_ERR_RATE_EN(0), 0x100, (char *)"IERRRAT", 0},
	{CPS1848_PORT_X_IMPL_SPEC_ERR_RPT_EN(0), 0x40, (char *)"IERRrpt", 0},
	{CPS1616_LANE_X_ERR_RATE_EN(0),	0x400, (char *)"L0 EN", 0},
	{CPS1616_LANE_X_ERR_RATE_EN(1),	0x400, (char *)"L1 EN", 0},
	{CPS1616_LANE_X_ERR_RATE_EN(2),	0x400, (char *)"L2 EN", 0},
	{CPS1616_LANE_X_ERR_RATE_EN(3),	0x400, (char *)"L3 EN", 0},
	{CPS1848_LANE_X_ERR_DET(0),	0x400, (char *)"L0 DET", 0},
	{CPS1848_LANE_X_ERR_DET(1),	0x400, (char *)"L1 DET", 0},
	{CPS1848_LANE_X_ERR_DET(2),	0x400, (char *)"L2 DET", 0},
	{CPS1848_LANE_X_ERR_DET(3),	0x400, (char *)"L3 DET", 0},
	{CPS1848_PORT_X_DEV_RTE_TABLE_Y(0,0x20), 0x1000, (char *)"PW RT", 0},
	{CPS1848_PORT_X_MCAST_MASK_Y(0,0), 0x100, (char *)"MC_MASK", 0},
	{CPS1848_ASSY_IDENT_CAR_OVRD, 0, (char *)"ASSY_ID", 0},
	};

	const rio_port_t ports[] = { 0, 1, 4, 5 };
	uint32_t ret = 1;
	int i, reg;
	uint32_t addr;
	uint32_t data;

	// Always enable performance counters
        DBG("\tWaiting for tsi721_mutex\n");
	sem_wait(&tsi721_mutex);
	LOGMSG(env, "REG NAME  offset  ");
	for (i = 0; i < sizeof(ports); i++) {
		uint32_t rt_val;
		addr = CPS1848_BCAST_DEV_RTE_TABLE_X(ports[i] + 1);
		if (rio_maint_read(mp_h, 0, 0, addr, 4, &rt_val))
			goto exit;
		LOGMSG(env, "P%2d -%2x- ", ports[i], rt_val);
	}
	for (reg = 0; reg < sizeof(regs)/sizeof(regs[0]); reg++) {
		LOGMSG(env, "\n%8s %8x ", regs[reg].lable, regs[reg].oset);
		for (i = 0; i < sizeof(ports); i++) {
			if (!regs[reg].pp_oset && i)
				continue;
			addr = regs[reg].oset;
			addr += ports[i] * regs[reg].pp_oset;
			if (rio_maint_read(mp_h, 0, 0, addr, 4, &data))
				goto exit;
			if (data || (!data && !regs[reg].no_zeros)) {
				LOGMSG(env, "%8x ", data);
			} else {
				LOGMSG(env, "         ");
			}
		}
	}
	LOGMSG(env, "\n");
	ret = 0;
exit:
        DBG("\tPOST tsi721_mutex\n");
	sem_post(&tsi721_mutex);
	if (ret) {
		LOGMSG(env, "Register access error %d\n", ret);
	}
	return 0;
}

struct cli_cmd CPSStatus = {
"cstatus",
2,
0,
"Print CPS status registers for RapidIO link.",
"<No parameters>\n"
"Print CPS status registers for RapidIO links.\n"
"Note that the routing table value for each port is printed in the header.\n"
"All registers up to L3 DET are link status and configuration registers.\n"
"PW RT is the port-write routing table value, usually 0x40 (multicast mask 0)\n"
"MC_MASK is the value of multicast mask 0.\n",
CPSStatusCmd,
ATTR_RPT
};

static int CPSCountersCmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
	struct {
		uint32_t oset;
		uint32_t pp_oset;
		char *lable;
		int no_zeros;
	} regs[] = {
	{CPS1848_PORT_X_VC0_PKT_RX_CNTR(0), 0x100, (char *)"PKT RX", 1},
	{CPS1848_PORT_X_VC0_PKT_DROP_RX_CNTR(0), 0x100, (char *)"DROP RX", 1},
	{CPS1848_PORT_X_VC0_PA_TX_CNTR(0), 0x100, (char *)"PA TX", 1},
	{CPS1848_PORT_X_VC0_RTRY_TX_CNTR(0), 0x100, (char *)"RTY TX", 1},
	{CPS1848_PORT_X_NACK_TX_CNTR(0), 0x100, (char *)"NACK TX", 1},
	{CPS1848_PORT_X_VC0_CPB_TX_CNTR(0), 0x100, (char *)"CPB TX", 1},
	{CPS1848_PORT_X_VC0_PKT_DROP_TX_CNTR(0), 0x100, (char *)"DROP TX", 1},
	{CPS1848_PORT_X_VC0_PKT_TX_CNTR(0), 0x100, (char *)"PKT TX", 1},
	{CPS1848_PORT_X_VC0_PA_RX_CNTR(0), 0x100, (char *)"PA RX", 1},
	{CPS1848_PORT_X_VC0_RTRY_RX_CNTR(0), 0x100, (char *)"RTY RX", 1},
	{CPS1848_PORT_X_NACK_RX_CNTR(0), 0x100, (char *)"NACK RX", 1},
	{CPS1848_PORT_X_VC0_TTL_DROP_CNTR(0), 0x100, (char *)"TTL DROP", 1},
	{CPS1848_PORT_X_VC0_CRC_LIMIT_DROP_CNTR(0), 0x100, (char *)"CRC DROP", 1},
	{CPS1848_MAINT_DROP_PKT_CNTR, 0, (char *)"MTC DROP", 1},
	};

	const rio_port_t ports[] = { 0, 1, 4, 5 };
	uint32_t ret = 1;
	int i, reg;
	uint32_t addr;
	uint32_t data;
	uint32_t got_one = 0;

        DBG("\tWaiting for tsi721_mutex\n");
	sem_wait(&tsi721_mutex);
	// Always enable performance counters
	for (i = 0; i < sizeof(ports); i++) {
		uint32_t ops;
		uint32_t addr = CPS1848_PORT_X_OPS(ports[i]);
		if (rio_maint_read(mp_h, 0, 0, addr, 4, &ops))
			goto exit;
		ops |= CPS1848_PORT_X_OPS_CNTRS_EN;
		if (rio_maint_write(mp_h, 0, 0, addr, 4, ops))
			goto exit;
	}

	LOGMSG(env, "REG NAME           0        1        4        5\n");
	for (reg = 0; reg < sizeof(regs)/sizeof(regs[0]); reg++) {
		LOGMSG(env, "%11s ", regs[reg].lable);
		for (i = 0; i < sizeof(ports); i++) {
			addr = regs[reg].oset;
			addr += ports[i] * regs[reg].pp_oset;
			if (rio_maint_read(mp_h, 0, 0, addr, 4, &data))
				goto exit;
			if (data || (!data && !regs[reg].no_zeros)) {
				LOGMSG(env, "%08x ", data);
			} else {
				LOGMSG(env, "         ");
			}
		}
		LOGMSG(env, "\n");
		if (!regs[reg].oset)
			break;
	}

	LOGMSG(env, "\nPORT STAT: ");
	// 18 is the maximum possible destID for IDT switches.
	for (int i = 1; i < MAX_DEVID_STATUS; i++) {
		if (devid_status[i]) {
			if (got_one) {
				LOGMSG(env, ", %d", i - 1);
			} else {
				got_one = 1;
				LOGMSG(env, "%d", i - 1);
			}
		}
	}
	if (got_one) {
		LOGMSG(env, "\n");
	} else {
		LOGMSG(env, "No valid devIDs.\n");
	}

	got_one = false;
	LOGMSG(env, "BAD DMA: ");
	for (int i = 0; i < MAX_WORKERS; i++) {
		int did;
		int chan;
		uint32_t did_n_chan;

		if (!wkr[i].stat)
			continue;
		ret = rio_mport_query_dma(wkr[i].mp_h, &did_n_chan);
		if (ret)
			continue;
		did = did_n_chan >> 16;
		if (did > MAX_DEVID_STATUS)
			continue;
		if (!devid_status[did]) {
			if (got_one)
				LOGMSG(env, ", ");
			got_one = 1;
			chan = did_n_chan & 0xFFFF;
			LOGMSG(env, "%d %d %d", i, chan, did - 1);
		}
	}
	LOGMSG(env, "\n");

	ret = 0;
exit:
        DBG("\tPOST tsi721_mutex\n");
	sem_post(&tsi721_mutex);
	if (ret) {
		LOGMSG(env, "Register access error %d\n", ret);
	}
	return 0;
}

struct cli_cmd CPSCounters = {
"ccounters",
2,
0,
"Enable and print CPS performance registers for RapidIO link.",
"<No parameters>\n"
"Print CPS performance registers for RapidIO link.\n"
" PKT RX : Packets received by this port\n"
"  PA TX : Packets successfully received by this port\n"
"DROP RX : Packets dropped by the receiving port, usually due to routing\n"
" RTY TX : Retries acknowledgements.\n"
"          If RTY TX == PKT RX, no packets are making forward progress.\n"
"NACK TX : Packet Not Accepted transmitted, bad packet.\n"
" CPB TX : Port received a packet from the switch fabric.\n"
"DROP TX : Transmitting port dropped a packet.\n"
" PKT TX : Packets transmitted by this port\n"
"  PA TX : Packet successfully transmitted by this port\n"
" RTY RX : Retries received.\n"
"          If RTY RX == PKT TX, no packets are making forward progress.\n"
"NACK RX : Packet Not Accepted received, bad packet.\n"
"TTL DROP: Packet dropped on transmit port due to time to live expiry.\n",
CPSCountersCmd,
ATTR_RPT
};

static int SevenMagicCmd(struct cli_env *UNUSED(env), int UNUSED(argc), char **UNUSED(argv))
{
        DBG("\tWaiting for tsi721_mutex\n");
	sem_wait(&tsi721_mutex);
	rio_lcfg_write(mp_h, TSI721_PLM_LONG_CS_TX1, 4, TSI721_MAGIC_CS);
        DBG("\tPOST tsi721_mutex\n");
	sem_post(&tsi721_mutex);

	return 0;
}

struct cli_cmd SevenMagic = {
"7magic",
2,
0,
"Send Magic (PNA+LR/IS) control symbol.",
"<No parameters>\n"
"Send a Magic control symbol to link parter.\n"
"A Magic control symbol consists of a Packet Not Accepted (PNA) and\n"
"a Link-Request/Input-Status (LR/IS) in the same control symbol.\n"
"The PNA triggers error recovery by the link partner.\n"
"The LR-IS performs error recovery on this port.\n",
SevenMagicCmd,
ATTR_RPT
};

static int SevenMECSCmd(struct cli_env *UNUSED(env), int UNUSED(argc), char **UNUSED(argv))
{
        DBG("\tWaiting for tsi721_mutex\n");
	sem_wait(&tsi721_mutex);
	rio_lcfg_write(mp_h, TSI721_PLM_LONG_CS_TX1, 4, TSI721_MECS_CS);
        DBG("\tPOST tsi721_mutex\n");
	sem_post(&tsi721_mutex);

	return 0;
}

struct cli_cmd SevenMECS = {
"7MECS",
2,
0,
"Send Multicast Event Control Symbol.",
"<No parameters>\n"
"Send Multicast Event Control Symbol.\n",
SevenMECSCmd,
ATTR_RPT
};

static int SevenResetLinkCmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
	int ret;

	ret = tsi721_config_port_reset();
	if (ret)
		goto exit;
        DBG("\tWaiting for tsi721_mutex\n");
	sem_wait(&tsi721_mutex);
	ret = tsi721_link_reset();
        DBG("\tPOST tsi721_mutex\n");
	sem_post(&tsi721_mutex);
exit:
	if (ret) {
		LOGMSG(env, "Link reset FAILED: ret = %d.\n", ret);
	} else {
		LOGMSG(env, "Link reset SUCCESS.\n");
	}
	return 0;
}

struct cli_cmd SevenResetLink = {
"7Reset",
2,
0,
"Reset Tsi721 link partner and Tsi721 RapidIO PHY.",
"<No parameters>\n"
"Reset Tsi721 link partner and Tsi721 RapidIO PHY.\n",
SevenResetLinkCmd,
ATTR_RPT
};

static int SevenAckidSyncCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint32_t port;
	int ret;

	ret = tok_parse_port_num(argv[0], &port, 10);
	if (ret) {
		LOGMSG(env, "Parse port failed : ret = %d.\n", ret);
		goto exit;
	}

	ret = tsi721_sync_cps_ackids(port);

	if (ret) {
		LOGMSG(env, "Sync ackIDs FAILED: ret = 0x%x.\n", ret);
	} else {
		LOGMSG(env, "Sync ackIDs SUCCESS.\n");
	}
exit:
	return 0;
}

struct cli_cmd SevenAckidSync = {
"7Ackid",
2,
1,
"Synchronise Tsi721 and CPS ackIDs without a reset.",
"<port>\n"
"Synchronize Tsi721 and CPS ackIDs without a reset.\n"
"<port> : CPS port, link partner for this Tsi721.\n",
SevenAckidSyncCmd,
ATTR_NONE
};

static int SevenLinkReqCmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
	uint32_t response;
	uint32_t ret = tsi721_link_req(&response);
	if (ret) {
		LOGMSG(env, "Link req failed %d\n", ret);
	}
	LOGMSG(env, "Response is 0x%8x\n", response);
	LOGMSG(env, "Port status: 0x%8x\n", response & 0x1F);
	LOGMSG(env, "AckID      : 0x%8x\n", (response & 0x7E0) >> 5);
	return 0;
}

struct cli_cmd SevenLinkReq = {
"7Linkreq",
2,
0,
"Send link request to link partner, display link repsonse.",
"<No parameters>\n"
"Send link request to link partner, display link repsonse.",
SevenLinkReqCmd,
ATTR_RPT
};

static int SevenResetTsi721Cmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
	int ret;

	ret = tsi721_config_port_reset();
	if (ret)
		goto exit;
        DBG("\tWaiting for tsi721_mutex\n");
	sem_wait(&tsi721_mutex);
	ret = tsi721_reset_own_port_from_cli();
        DBG("\tPOST tsi721_mutex\n");
	sem_post(&tsi721_mutex);
exit:
	if (ret) {
		LOGMSG(env, "Tsi721 reset FAILED: ret = %d.\n", ret);
	} else {
		LOGMSG(env, "Tsi721 reset SUCCESS.\n");
	}
	return 0;
}

struct cli_cmd SevenResetTsi721= {
"7reset",
2,
0,
"Reset Tsi721 RapidIO PHY.",
"<No parameters>\n"
"Reset Tsi721 RapidIO PHY.\n",
SevenResetTsi721Cmd,
ATTR_RPT
};

static int CPSVerificationCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint32_t port;
	uint32_t ret = tok_parse_port_num(argv[0], &port, 10);
	if (ret) {
		LOGMSG(env, "Parse port failed : ret = %d.\n", ret);
		goto exit;
	}

	cps_event_handling_test(env, port);
exit:
	return 0;
}

struct cli_cmd CPSVerification = {
"cverify",
2,
1,
"Verify CPS Event Management routines.",
"<port>\n"
"Verify CPS Event Management routines on real hardware.\n"
"<port> : Port to be used for testing purposes.\n"
"NOTE: Assumes the endpoint is directly connected to a CPS switch.\n",
CPSVerificationCmd,
ATTR_NONE
};

static int CPSResetCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint32_t port;
	uint32_t ret = tok_parse_port_num(argv[0], &port, 10);
	DAR_DEV_INFO_t dev_h;

	if (ret) {
		LOGMSG(env, "Parse port failed : ret = %d.\n", ret);
		goto exit;
	}

	ret = reset_cps_port(&dev_h, port);
	if (ret) {
		LOGMSG(env, "Reset port failed : ret = %d.\n", ret);
	} else {
		LOGMSG(env, "Reset port Passed.\n");
	}
exit:
	return 0;
}

struct cli_cmd CPSReset = {
"creset",
2,
1,
"Reset specified CPS port.",
"<port>\n"
"Reset CPS port using DEVICE_RESET_CTL register.\n"
"<port> : Port to be reset.\n"
"NOTE: Assumes the endpoint is directly connected to a CPS switch.\n",
CPSResetCmd,
ATTR_NONE
};

static int SevenClearTsi721Cmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
	int ret;

	ret = tsi721_config_port_reset();
	if (ret)
		goto exit;
        DBG("\tWaiting for tsi721_mutex\n");
	sem_wait(&tsi721_mutex);
	ret = tsi721_clear_all_from_cli();
        DBG("\tPOST tsi721_mutex\n");
	sem_post(&tsi721_mutex);
exit:
	LOGMSG(env, "Tsi721 clear all fatal errors, returned %d.\n", ret);
	return 0;
}

struct cli_cmd SevenClearTsi721 = {
"7Clear",
2,
0,
"Clear all Tsi721 error conditions, PORT_DIS and PORT_LOCKOUT",
"<No parameters>\n"
"Clear all Tsi721 error conditions, PORT_DIS and PORT_LOCKOUT\n"
"This will reset the Tsi721 link and link partner.\n",
SevenClearTsi721Cmd,
ATTR_RPT
};

static int SevenDisablePortCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint32_t ret;
	uint8_t port;
	DAR_DEV_INFO_t dev_h;

	if (tok_parse_hc(argv[0], &port, 0)) {
		LOGMSG(env, "ERR: %s is not a valid port\n",
				argv[0]);
		goto exit;
	}

	ret = DAR_proc_ptr_init(SRIO_API_ReadRegFunc, SRIO_API_WriteRegFunc,
                                SRIO_API_DelayFunc);

        dev_h.devID = 0x03750038;
        dev_h.driver_family = rio_get_driver_family(dev_h.devID);

        DBG("\tWaiting for tsi721_mutex\n");
	sem_wait(&tsi721_mutex);
        ret = DAR_Find_Driver_for_Device(1, &dev_h);
	if (ret) {
		LOGMSG(env, "ERR: Could not find driver for device 0x%x\n", ret);
		goto unlock;
	};
	ret = disable_switch_port(&dev_h, port);
unlock:
        DBG("\tPOST tsi721_mutex\n");
	sem_post(&tsi721_mutex);
	if (ret) {
		LOGMSG(env, "Failed, err = 0x%x\n", ret);
	}

	LOGMSG(env, "completed.\n");
exit:
	return 0;
}

struct cli_cmd SevenDisablePort = {
"7Disable",
2,
1,
"Disable CPS switch port.",
"<port>\n"
"Disable CPS switch port using disable_switch_port\n"
"<port> : Valid port number on CPS switch.\n",
SevenDisablePortCmd,
ATTR_RPT
};

static int TsiPortWriteCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	rio_event *event;
	uint32_t plm = 0;
	uint16_t idx;

	if (gp_parse_worker_index_check_thread(env, argv[0], &idx, 2))
		goto exit;

	if (wkr[idx].action != cps_handle_721_pw) {
		LOGMSG(env, "Thread does not handle Tsi721 port-writes.\n");
		goto exit;
	}

	switch (argv[1][0]) {
	case 'd':
	case 'D':
		// Dead link timer
		plm = TSI721_PLM_STATUS_DLT;
		break;
	case 'l':
	case 'L':
		// Dead link timer
		plm = TSI721_PLM_STATUS_LINK_INIT;
		break;
	default:
		LOGMSG(env, "Unknown action %s, exiting.\n", argv[1]);
		goto exit;
	}

	event = (struct rio_event *)malloc(sizeof(struct rio_event));
	event->header = RIO_PORTWRITE;
	event->u.portwrite.payload[0] = wkr[idx].tsi721_ct;
	event->u.portwrite.payload[1] = 0;
	event->u.portwrite.payload[2] = plm;
	event->u.portwrite.payload[3] = 0;

	LOGMSG(env, "Waiting for queue mutex.\n");
	sem_wait(&wkr[idx].pw_mutex);
	l_push_tail(&wkr[idx].pw, event);
	sem_post(&wkr[idx].pw_mutex);

	LOGMSG(env, "Starting thread.\n");
	sem_post(&wkr[idx].process_pw);
	LOGMSG(env, "completed.\n");
exit:
	return 0;
}

struct cli_cmd TsiPortWrite = {
"tportwrite",
2,
2,
"Process port-write for Tsi721.",
"<idx> <event>\n"
"Process a port-write event for this Tsi721.\n"
"<idx>   : Worker that will process the port-write.\n"
"<event> : Event to process, either D (DLT) or L (Link init)\n",
TsiPortWriteCmd,
ATTR_NONE
};

static int CPSEventCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint32_t ret;
	uint32_t port;
	DAR_DEV_INFO_t dev_h;
	rio_em_create_events_in_t ev_in;
	rio_em_create_events_out_t ev_out;
	rio_em_event_n_loc_t event;

	ret = tok_parse_port_num(argv[0], &port, 10);
	if (ret) {
		LOGMSG(env, "Parse port failed : ret = %d.\n", ret);
		goto exit;
	}
	event.port_num = port;

	switch (argv[1][0]) {
	case 'd':
	case 'D':
		// Dead link timer
		event.event =  rio_em_f_los;
		LOGMSG(env, "Event %d\n", event.event);
		break;
	case 'l':
	case 'L':
		// Link initialization
		event.event = rio_em_i_sig_det;
		LOGMSG(env, "Event %d\n", event.event);
		break;
	default:
		LOGMSG(env, "Unknown action %s, exiting.\n", argv[1]);
		goto exit;
	}

	ret = DAR_proc_ptr_init(SRIO_API_ReadRegFunc, SRIO_API_WriteRegFunc,
                                SRIO_API_DelayFunc);

        dev_h.devID = 0x03750038;
        dev_h.driver_family = rio_get_driver_family(dev_h.devID);

        DBG("\tWaiting for tsi721_mutex\n");
	sem_wait(&tsi721_mutex);
        ret = DAR_Find_Driver_for_Device(1, &dev_h);
	if (ret) {
		LOGMSG(env, "ERR: Could not find driver for device 0x%x\n", ret);
		goto unlock;
	};

	ev_in.num_events = 1;
	ev_in.events = &event;
	ret = rio_em_create_events(&dev_h, &ev_in, &ev_out);

	LOGMSG(env, "Completed, ret: 0x%x imp_rc: 0x%x.\n", ret, ev_out.imp_rc);
unlock:
        DBG("\tPOST tsi721_mutex\n");
	sem_post(&tsi721_mutex);
exit:
	return 0;
}

struct cli_cmd CPSEvent = {
"cevent",
2,
2,
"Create an event on a CPS1848 port.",
"<port> <event>\n"
"Create an event ona CPS1848 port.\n"
"<port> : Port that originated the port-write.\n"
"<event>: Event to create, either D (DLT) or L (Link init)\n",
CPSEventCmd,
ATTR_NONE
};

static int CPSPortWriteCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint32_t ret;
	uint32_t port;
	rio_em_events_t event = rio_em_last;
	uint16_t idx;

	ret = tok_parse_port_num(argv[0], &port, 10);
	if (ret) {
		LOGMSG(env, "Parse port failed : ret = %d.\n", ret);
		goto exit;
	}

	switch (argv[1][0]) {
	case 'd':
	case 'D':
		// Dead link timer
		event =  rio_em_f_los;
		LOGMSG(env, "Event %d\n", event);
		break;
	case 'l':
	case 'L':
		// Dead link timer
		event = rio_em_i_sig_det;
		LOGMSG(env, "Event %d\n", event);
		break;
	default:
		LOGMSG(env, "Unknown action %s, exiting.\n", argv[1]);
		goto exit;
	}

	if (gp_parse_worker_index_check_thread(env, argv[2], &idx, 2))
		goto exit;

	if (wkr[idx].action != cps_handle_cps_pw) {
		LOGMSG(env, "Thread does not handle CPS port-writes.\n");
		goto exit;
	}

	LOGMSG(env, "Process CPS PW on port %d, event %d\n",
		port, (int)event);
	ret = handle_switch_port_write_event(wkr[idx].dev_h, port, event);

	LOGMSG(env, "completed, ret: 0x%x.\n", ret);
exit:
	return 0;
}

struct cli_cmd CPSPortWrite = {
"cportwrite",
2,
3,
"Process port-write for CPS1848 port.",
"<port> <event> <idx>\n"
"Process a port-write event for the CPS1848.\n"
"<port> : Port that originated the port-write.\n"
"<event>: Event to process, either D (DLT) or L (Link init)\n"
"<idx>  : Index of a worker processing CPS port-writes.\n",
CPSPortWriteCmd,
ATTR_NONE
};


static int MpdevsCmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
	uint32_t *mport_list = NULL;
	uint32_t *list_ptr;
	uint8_t number_of_mports = RIO_MAX_MPORTS;
	uint8_t mport_id;

	did_val_t *ep_list = NULL;
	uint32_t number_of_eps = 0;
	uint32_t ep;
	int i;
	int ret;

	ret = rio_mport_get_mport_list(&mport_list, &number_of_mports);
	if (ret) {
		LOGMSG(env, "rio_mport_get_mport_list ERR %d:%s\n", ret,
				strerror(ret));
		LOGMSG(env, "Is rio-cm loaded?\n");
		goto exit;
	}

	LOGMSG(env, "\nAvailable %d local mport(s):\n", number_of_mports);

	if (number_of_mports > RIO_MAX_MPORTS) {
		LOGMSG(env, "WARNING: Only %d out of %d have been retrieved\n",
				RIO_MAX_MPORTS, number_of_mports);
	}

	list_ptr = mport_list;
	for (i = 0; i < number_of_mports; i++, list_ptr++) {
		mport_id = *list_ptr >> 16;
		LOGMSG(env, "+++ mport_id: %u dest_id: %u\n", mport_id,
				*list_ptr & 0xffff);

		/* Display EPs for this MPORT */

		ret = rio_mport_get_ep_list(mport_id, &ep_list,
				&number_of_eps);
		if (ret) {
			LOGMSG(env, "ERR: riodp_ep_get_list() ERR %d: %s\n",
					ret, strerror(ret));
			break;
		}

		printf("\t%u Endpoints (dest_ID): ", number_of_eps);
		for (ep = 0; ep < number_of_eps; ep++) {
			LOGMSG(env, "%u ", *(ep_list + ep));
		}
		LOGMSG(env, "\n");

		ret = rio_mport_free_ep_list(&ep_list);
		if (ret) {
			LOGMSG(env, "ERR: riodp_ep_free_list() ERR %d: %s\n",
					ret, strerror(ret));
		}

	}

	LOGMSG(env, "\n");

	ret = rio_mport_free_mport_list(&mport_list);
	if (ret) {
		LOGMSG(env, "ERR: riodp_ep_free_list() ERR %d: %s\n", ret,
				strerror(ret));
	}

exit:
	return 0;
}

struct cli_cmd Mpdevs = {
"mpdevs",
2,
0,
"Display mports and devices",
"<No Parameters>\n",
MpdevsCmd,
ATTR_NONE
};


static int QueryDmaCmd(struct cli_env *env, int argc, char **argv)
{
        int file_no = mp_h;
	uint32_t did_n_chan;
	int chan_num, did;
	uint16_t idx = -1;;
        uint32_t ret;

	if (argc) {
		if (gp_parse_worker_index(env, argv[0], &idx)) {
			goto exit;
		}
		if (!wkr[idx].stat) {
			LOGMSG(env, "Worker must be halted or running.\n");
			goto exit;
		}
		file_no = wkr[idx].mp_h;
	}

        ret = rio_mport_query_dma(file_no, &did_n_chan);
        if (ret) {
                LOGMSG(env, "ERR rio_mport_query_dma file %d %d:%s\n",
                                file_no, ret, strerror(ret));
                goto exit;
        }

	did = did_n_chan >> 16;
	chan_num = did_n_chan & 0xFFFF;
        if (chan_num < 0) {
                LOGMSG(env, "No channel assigned.\n");
                goto exit;
        }

        LOGMSG(env, "\n0x%x Channel %d DestID %d\n", did_n_chan, chan_num, did);
exit:
        return 0;
}

struct cli_cmd QueryDma = {
"qdma",
2,
0,
"Display dma channel registers",
"{<idx>} (Optional)\n"
"Display dma channel registers for open mport, or for worker index.\n"
"<idx> : worker index from 0 to " STR(MAX_WORKER_IDX) "\n",
QueryDmaCmd,
ATTR_RPT
};

static int RegScrubCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	uint16_t idx;
	did_val_t did;
	hc_t hc;
	uint32_t offset;
	uint32_t count;
	uint32_t val;
	int n = 0;
	const uint32_t max_offset = 0xFFFFFC;

	if (gp_parse_worker_index(env, argv[n], &idx)) {
		goto exit;
	}

	if (2 != wkr[idx].stat) {
		LOGMSG(env, "ERR: worker %d is not halted\n", idx);
		goto exit;
	}

	n++;
	if (tok_parse_did(argv[n], &did, 0)) {
		LOGMSG(env, "ERR: %s is not a valid destination ID\n",
				argv[n]);
		goto exit;
	}

	n++;
	if (tok_parse_hc(argv[n], &hc, 0)) {
		LOGMSG(env, "ERR: %s is not a valid hopcount\n",
				argv[n]);
		goto exit;
	}

	// Valid register offsets run from 0 to 0xFFFFFC, since
	// this routine uses 4 byte register writes.
	n++;
	if (tok_parse_ulong(argv[n], &offset, 0, max_offset, 0)) {
		LOGMSG(env, "ERR: %s is not a valid offset\n",
				argv[n]);
		goto exit;
	}
	// Ensure offset is a multiple of 4
	offset &= max_offset;

	n++;
	if (tok_parse_ulong(argv[n], &count, 1, 0xFFFFFFFF, 0)) {
		LOGMSG(env, "ERR: %s is not a valid count \n",
				argv[n]);
		goto exit;
	}

	n++;
	if (tok_parse_ul(argv[n], &val, 0)) {
		LOGMSG(env, "ERR: %s is not a valid value\n",
				argv[n]);
		goto exit;
	}

	wkr[idx].action = reg_scrub;
	wkr[idx].did_val = did;
	wkr[idx].data8_tx = hc;
	wkr[idx].rio_addr = offset;
	wkr[idx].byte_cnt = count * 4;
	wkr[idx].data32_tx = val;
	wkr[idx].stop_req = 0;
	sem_post(&wkr[idx].run);

exit:
	return 0;
}

struct cli_cmd RegScrub = {
"regscrub",
3,
6,
"Repeatedly perform register writes to a range of addresses.",
"<idx> <DevDid> <HC> <offset> <count> <value>\n"
"Use worker <idx> to write to the device <DevDid> <HC> registers.\n"
"The registers start at <offset>. <count> registers are written with <value>.\n"
"<idx>    : a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
"<DevDid> : Destination ID to use when accessing the device\n"
"<HC>     : Hop count to use when accessing the device\n"
"<offset> : Offset to use for first register write\n"
"<count>  : Number of registers to write\n"
"<value>  : 4 byte value to write to all registers\n",
RegScrubCmd,
ATTR_NONE
};

static int FileRegsCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
	FILE *fp;
	uint32_t ret;
	uint32_t val;

	fp = fopen(argv[0], "w+");
	if (!fp) {
		LOGMSG(env, "ERR: Cannot open file '%s'\n", argv[0]);
		goto exit;
	}

	for (uint32_t offset = 0; offset < 0x1000000; offset += 4) {
		ret = rio_maint_read(mp_h, 0, 0, offset, 4, &val);
		if (ret) {
			LOGMSG(env, "Err reading registers offset 0x%x\n",
					offset);
			break;
		}
		fprintf(fp, "0x%08x 0x%08x\n", offset, val);
	}
	fclose(fp);

	if (!ret) {
		LOGMSG(env, "Wrote switch registers to file '%s'\n", argv[0]);
	}
exit:
	return 0;
}

struct cli_cmd FileRegs = {
"FRegs",
2,
1,
"Write register values to a file.",
"<file_name>\n"
"Write register values to file <file_name>\n",
FileRegsCmd,
ATTR_NONE
};

static int program_rxs_mc_mask(struct cli_env *env,
				did_val_t mc_did,
				int did_cnt,
				did_val_t *dids)
{
	const int mc_mask_idx = 0;
	uint32_t mc_mask = 0;
	uint32_t mc_mask_chk;
	uint32_t port, rte;
	int ret;
	int did_i;
	int rc = 1;

	// Determine which port this endpoint is connected to
	ret = rio_maint_read(mp_h, 0, 0, RXS_SW_PORT, 4, &port);
	if (ret) {
		LOGMSG(env, "ERR: Could not read SW_PORT ERR %d %s\n",
							ret, strerror(ret));
		goto exit;
	}

	port &= RXS_SW_PORT_PORT_NUM;

	// Read routing table values for requested destIDs, fail if they
	// are not port numbers.
	// >>** Assumes 8 bit destIDs!!! **<<

	for (did_i = 0; did_i < did_cnt; did_i++) {
		ret = rio_maint_read(mp_h, 0, 0,
			RXS_SPX_L2_GY_ENTRYZ_CSR(port, 0, dids[did_i]),
			4, &rte);
		if (ret) {
			LOGMSG(env, "ERR: Could not read DID %d ERR %d %s\n",
					dids[did_i], ret, strerror(ret));
			goto exit;
		}
		LOGMSG(env, "Did: %d Port: 0x%x\n", dids[did_i], rte);

		if (rte >= RXS2448_MAX_PORTS) {
			LOGMSG(env,
			"Routing table value 0x%x for id %d is unsupported\n",
					rte, dids[did_i]);
			goto exit;
		}
		if ((1 << rte) & mc_mask) {
			LOGMSG(env, "ERR: Port %d or did %d duplicated\n",
					rte, dids[did_i]);
			goto exit;
		}
		mc_mask |= 1 << rte;
	}

	LOGMSG(env, "Multicast mask: 0x%06x\n", mc_mask);

	// Program multicast mask 0.
	ret = rio_maint_write(mp_h, 0, 0,
		RXS_SPX_MC_Y_S_CSR(port, mc_mask_idx), 4, mc_mask);
	if (ret) {
		LOGMSG(env, "ERR: Could not write MC mask set %d ERR %d %s\n",
				mc_mask_idx, ret, strerror(ret));
		goto exit;
	}

	ret = rio_maint_write(mp_h, 0, 0,
		RXS_SPX_MC_Y_C_CSR(port, mc_mask_idx), 4, ~mc_mask);
	if (ret) {
		LOGMSG(env, "ERR: Could not write MC mask clr %d ERR %d %s\n",
				mc_mask_idx, ret, strerror(ret));
		goto exit;
	}

	// Paranoid check that MC mask is correct
	ret = rio_maint_read(mp_h, 0, 0,
		RXS_SPX_MC_Y_S_CSR(port, mc_mask_idx), 4, &mc_mask_chk);
	if (ret) {
		LOGMSG(env, "ERR: Could not read MC mask %d ERR %d %s\n",
				mc_mask_idx, ret, strerror(ret));
		goto exit;
	}

	if (mc_mask != mc_mask_chk) {
		LOGMSG(env, "ERR: MC mask 0x%x not 0x%x\n",
				mc_mask_chk, mc_mask);
		goto exit;
	}

	// Program mc_did routing table entry to select MC MASK 0
	ret = rio_maint_write(mp_h, 0, 0,
		RXS_SPX_L2_GY_ENTRYZ_CSR(port, 0, mc_did),
		4, RIO_RTV_MC_MSK(mc_mask_idx));
	if (ret) {
		LOGMSG(env, "ERR: Could not write did entry %d ERR %d %s\n",
				mc_did, ret, strerror(ret));
		goto exit;
	}

	// Paranoid check that mc_did routing table entry is correct
	ret = rio_maint_read(mp_h, 0, 0,
		RXS_SPX_L2_GY_ENTRYZ_CSR(port, 0, mc_did), 4, &rte);
	if (ret) {
		LOGMSG(env, "ERR: Could not read did entry %d ERR %d %s\n",
				mc_did, ret, strerror(ret));
		goto exit;
	}

	if (RIO_RTV_MC_MSK(mc_mask_idx) != rte) {
		LOGMSG(env, "ERR: did %d value %d not %d\n",
				mc_did, rte, RIO_RTV_MC_MSK(mc_mask_idx));
		goto exit;
	}

	rc = 0;
exit:
	return rc;
}

static int program_cps_mc_mask(struct cli_env *env,
				did_val_t mc_did,
				int did_cnt,
				did_val_t *dids)
{
	const int mc_mask_idx = 0;
	uint32_t mc_mask = 0;
	uint32_t mc_mask_chk;
	uint32_t rte, tmp;
	int ret;
	int did_i;
	int rc = 1;

	// Read routing table values for requested destIDs, fail if they
	// are not port numbers.
	// >>** Assumes 8 bit destIDs!!! **<<

	for (did_i = 0; did_i < did_cnt; did_i++) {
		ret = rio_maint_read(mp_h, 0, 0,
			CPS_BROADCAST_UC_DEVICE_RT_ENTRY(dids[did_i]), 4, &rte);
		if (ret) {
			LOGMSG(env, "ERR: Could not read DID %d ERR %d %s\n",
					dids[did_i], ret, strerror(ret));
			goto exit;
		}
		LOGMSG(env, "Did: %d Port: 0x%x\n", dids[did_i], rte);

		if (rte >= CPS_MAX_PORTS) {
			LOGMSG(env,
			"Routing table value %d for id %d is unsupported\n",
					rte, dids[did_i]);
			goto exit;
		}
		if ((1 << rte) & mc_mask) {
			LOGMSG(env, "ERR: Port %d or did %d duplicated\n",
					rte, dids[did_i]);
			goto exit;
		}
		mc_mask |= 1 << rte;
	}

	LOGMSG(env, "Multicast mask: 0x%05x\n", mc_mask);
	// Program multicast mask 0.
	ret = rio_maint_write(mp_h, 0, 0,
		CPS1848_BCAST_MCAST_MASK_X(mc_mask_idx), 4, mc_mask);
	if (ret) {
		LOGMSG(env, "ERR: Could not write MC mask %d ERR %d %s\n",
				mc_mask_idx, ret, strerror(ret));
		goto exit;
	}

	// Paranoid check that MC mask is correct
	ret = rio_maint_read(mp_h, 0, 0,
		CPS1848_BCAST_MCAST_MASK_X(mc_mask_idx), 4, &mc_mask_chk);
	if (ret) {
		LOGMSG(env, "ERR: Could not read MC mask %d ERR %d %s\n",
				mc_mask_idx, ret, strerror(ret));
		goto exit;
	}

	if (mc_mask != mc_mask_chk) {
		LOGMSG(env, "ERR: MC mask 0x%x not 0x%x\n",
				mc_mask_chk, mc_mask);
		goto exit;
	}

	// Program mc_did routing table entry to select MC MASK 0
	ret = rio_maint_write(mp_h, 0, 0,
		CPS_BROADCAST_UC_DEVICE_RT_ENTRY(mc_did), 4,
		CPS_MC_PORT(mc_mask_idx));
	if (ret) {
		LOGMSG(env, "ERR: Could not write did entry %d ERR %d %s\n",
				mc_did, ret, strerror(ret));
		goto exit;
	}

	// Paranoid check that mc_did routing table entry is correct
	ret = rio_maint_read(mp_h, 0, 0,
		CPS_BROADCAST_UC_DEVICE_RT_ENTRY(mc_did), 4, &rte);
	if (ret) {
		LOGMSG(env, "ERR: Could not read did entry %d ERR %d %s\n",
				mc_did, ret, strerror(ret));
		goto exit;
	}

	if (CPS_MC_PORT(mc_mask_idx) != rte) {
		LOGMSG(env, "ERR: did %d value %d not %d\n",
				mc_did, rte, CPS_MC_PORT(mc_mask_idx));
		goto exit;
	}

	// One more thing: alter switch parameters
	// - Set arb mode to 0b111
	//   round robin with input scheduler aging disabled.
	// - Set crosspoint buffer allocate to '1' buffer per prio 1, 2, 3
	// - Set OUTPUT_CREDIT_RSVN to 0
	// - Set INPUT_STARV_LIM to 0
	ret = rio_maint_read(mp_h, 0, 0, CPS1848_SWITCH_PARAM_1, 4, &tmp);
	if (ret) {
		LOGMSG(env, "ERR: Could not read ARB_MODE ERR %d %s\n",
						ret, strerror(ret));
		goto exit;
	}

	tmp |= CPS1848_SWITCH_PARAM_1_ARB_MODE_RR_AGELESS;
	tmp |= CPS1848_SWITCH_PARAM_1_BUF_ALLOC;
	tmp &= ~(CPS1848_SWITCH_PARAM_1_INPUT_STARV_LIM);
	tmp &= ~(CPS1848_SWITCH_PARAM_1_OUTPUT_CREDIT_RSVN);

	ret = rio_maint_write(mp_h, 0, 0, CPS1848_SWITCH_PARAM_1, 4, tmp);
	if (ret) {
		LOGMSG(env, "ERR: Could not write ARB_MODE ERR %d %s\n",
						ret, strerror(ret));
		goto exit;
	}

	rc = 0;
exit:
	return rc;
}

// Program multicast mask to include ports for specified device IDs.
static int program_mc_mask(struct cli_env *env,
				did_val_t mc_did,
				int did_cnt,
				did_val_t *dids)
{
	uint32_t id;
	int ret;
	int rc = 1;

	// Check that this device is connected to a CPS1848 or CPS1432.

	ret = rio_maint_read(mp_h, 0, 0, CPS1848_DEV_IDENT_CAR, 4, &id);
	if (ret) {
		LOGMSG(env, "ERR: Could not read RIO_DEV_IDENT %d %s\n",
							ret, strerror(ret));
		goto exit;
	}

	if ((id & RIO_DEV_IDENT_VEND) != RIO_VEND_IDT) {
		LOGMSG(env, "ERR: Unsupported device vendor ID 0x%x\n",
						id & RIO_DEV_IDENT_DEVI);
		goto exit;
	}

	if (((id & RIO_DEV_IDENT_DEVI) == (RIO_DEVI_IDT_RXS2448 << 16)) ||
		((id & RIO_DEV_IDENT_DEVI) == (RIO_DEVI_IDT_RXS1632 << 16))) {
		rc = program_rxs_mc_mask(env, mc_did, did_cnt, dids);
		goto exit;
	}

	if (((id & RIO_DEV_IDENT_DEVI) == (RIO_DEVI_IDT_CPS1848 << 16)) ||
		((id & RIO_DEV_IDENT_DEVI) == (RIO_DEVI_IDT_CPS1432 << 16))) {
		rc = program_cps_mc_mask(env, mc_did, did_cnt, dids);
		goto exit;
	}

	LOGMSG(env, "ERR: Unsupported device 0x%x\n", id & RIO_DEV_IDENT_DEVI);
exit:
	return rc;
}

static int MulticastCmd(struct cli_env *env, int argc, char **argv)
{
	did_val_t *ep_list = NULL;
	uint32_t number_of_eps = 0;
	uint32_t ep;
	int did_i;
	int ret;
	const int num_dids = 10;
	did_val_t dids[num_dids];
	int did_cnt = 0;
	did_val_t mc_did;
	bool found_one;

	if (tok_parse_did(argv[0], &mc_did, 0)) {
		LOGMSG(env, "ERR: %s is not a valid destination ID\n",
				argv[0]);
		goto exit;
	}

	while (did_cnt + 1 < argc) {
		if (tok_parse_did(argv[did_cnt + 1], &dids[did_cnt], 0)) {
			LOGMSG(env, "ERR: %s is not a valid destination ID\n",
				argv[did_cnt + 1]);
			goto exit;
		}
		did_cnt++;
	}

	// Get EPs for this MPORT

	ret = rio_mport_get_ep_list(mp_h_num, &ep_list, &number_of_eps);
	if (ret) {
		LOGMSG(env, "ERR: riodp_ep_get_list() ERR %d: %s\n",
					ret, strerror(ret));
		goto exit;
	}

	// Check that requested DestIDs exist in the system
	// Allow the destID of this node to be part of the list

	for (did_i = 0; did_i < did_cnt; did_i++) {
		found_one = (qresp.hdid == dids[did_i]);
		for (ep = 0; (ep < number_of_eps) && !found_one; ep++) {
			found_one = (ep_list[ep] == dids[did_i]);
		}
		if (!found_one) {
			LOGMSG(env, "ERR: Endpoint %d does not exist.\n",
				dids[did_i]);
			goto exit;
		}
	}

	// Check that multicast DestID does not exist in the system
	// If it does, fail, as multicasting to an existing endpoint would
	// mess up routing to that endpoint until a reboot occurred.

	found_one = false;
	for (ep = 0; (ep < number_of_eps) && !found_one; ep++) {
		found_one = (ep_list[ep] == mc_did);
	}
	if (found_one) {
		LOGMSG(env,
			"ERR: Endpoint %d cannot be used for as the MC did.\n",
			mc_did);
		goto exit;
	}

	if (qresp.hdid == mc_did) {
		LOGMSG(env,
			"ERR: This endpoint's DestID %d "
			"cannot be used as the MC did.\n",
			mc_did);
		goto exit;
	}

	// Check that multicast DestID does not exist in the system

	ret = rio_mport_free_ep_list(&ep_list);
	if (ret) {
		LOGMSG(env, "ERR: riodp_ep_free_list() ERR %d: %s\n",
				ret, strerror(ret));
	}

	// Program multicast mask, CPS1848 specific
	// Assumes use of global routing table.

	if (program_mc_mask(env, mc_did, did_cnt, dids)) {
		LOGMSG(env, "ERR: program_mc_mask() failed\n");
	}
exit:
	return 0;
}

struct cli_cmd Multicast = {
"multicast",
2,
2,
"Set multicast for list of destination IDs",
"<mc_did> <did>...\n"
	"<mc_did>: the otherwise unused device ID for multicast\n"
	"<did>   : device IDs of the target(s) of the multicast\n"
	"          Must enter at least one, maximum 10.\n",
MulticastCmd,
ATTR_NONE
};

static int UTimeCmd(struct cli_env *env, int argc, char **argv)
{
	uint16_t idx, st_i = 0, end_i = MAX_TIMESTAMPS-1;
	struct seq_ts *ts_p = NULL;
	uint64_t lim = 0;
	int got_one = 0;
	struct timespec diff, min, max, tot;

	if (gp_parse_worker_index_check_thread(env, argv[0], &idx, 0)) {
		goto exit;
	}

	switch (argv[1][0]) {
	case 'd':
	case 'D':
		ts_p = &wkr[idx].desc_ts;
		break;
	case 'f':
	case 'F':
		ts_p = &wkr[idx].fifo_ts;
		break;
	case 'm':
	case 'M':
		ts_p = &wkr[idx].meas_ts;
		break;
	default:
		LOGMSG(env, "\nFAILED: <type> not 'd', 'f' or 'm'\n");
		goto exit;
	}

	switch (argv[2][0]) {
	case 's':
	case 'S':
		init_seq_ts(ts_p, MAX_TIMESTAMPS);
		break;
	case '-':
		if (argc > 4) {
			if (tok_parse_ushort(argv[3], &st_i, 0, MAX_TIMESTAMPS-1, 0)) {
				LOGMSG(env, "\n");
				LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "st_i", 0, MAX_TIMESTAMPS-1);
				goto exit;
			}
			if (tok_parse_ushort(argv[4], &end_i, 0, MAX_TIMESTAMPS-1, 0)) {
				LOGMSG(env, "\n");
				LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "end_i", 0, MAX_TIMESTAMPS-1);
				goto exit;
			}
		} else {
			LOGMSG(env, "\nFAILED: Must enter two indices\n");
			goto exit;
		}

		if (end_i < st_i) {
			LOGMSG(env, "\nFAILED: End index is less than start index\n");
			goto exit;
		}

		if (ts_p->ts_idx < MAX_TIMESTAMPS - 1) {
			LOGMSG(env, "\nWARNING: Last valid timestamp is %d\n",
					ts_p->ts_idx);
		}
		diff = time_difference(ts_p->ts_val[st_i], ts_p->ts_val[end_i]);
		LOGMSG(env, "\n---->> Sec<<---- Nsec---MMMuuuNNN\n");
		LOGMSG(env, "%16ld %16ld\n",
				diff.tv_sec, diff.tv_nsec);
		break;

	case 'p':
	case 'P':
		if ((argc > 3)&& (tok_parse_ushort(argv[3], &st_i, 0,
						MAX_TIMESTAMPS - 1, 0))) {
			LOGMSG(env, "\n");
			LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "st_i", 0,
					MAX_TIMESTAMPS-1);
			goto exit;
		}
		if ((argc > 4)&& (tok_parse_ushort(argv[4], &end_i, 0,
						MAX_TIMESTAMPS - 1, 0))) {
			LOGMSG(env, "\n");
			LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "end_i", 0,
					MAX_TIMESTAMPS-1);
			goto exit;
		}

		if (end_i < st_i) {
			LOGMSG(env, "\nFAILED: End index is less than start index\n");
			goto exit;
		}

		if (ts_p->ts_idx < MAX_TIMESTAMPS - 1) {
			LOGMSG(env,
				"\nWARNING: Last valid timestamp is %d\n",
				ts_p->ts_idx);
		}

		LOGMSG(env,
			"\nIdx ---->> Sec<<---- Nsec---mmmuuunnn Marker\n");
		for (idx = st_i; idx <= end_i; idx++) {
			LOGMSG(env, "%4d %16ld %16ld %d\n", idx,
				ts_p->ts_val[idx].tv_sec,
				ts_p->ts_val[idx].tv_nsec,
				ts_p->ts_mkr[idx]);
		}
		break;

	case 'l':
	case 'L':
		if (argc > 3) {
			if (tok_parse_ull(argv[3], &lim, 0)) {
				LOGMSG(env, "\n");
				LOGMSG(env, TOK_ERR_ULL_HEX_MSG_FMT, "lim");
				goto exit;
			}
		} else {
			lim = 0;
		}

		for (idx = st_i; idx < end_i; idx++) {
			time_track(idx, ts_p->ts_val[idx], ts_p->ts_val[idx+1],
				&tot, &min, &max);
			diff = time_difference(ts_p->ts_val[idx],
						ts_p->ts_val[idx+1]);
			if ((uint64_t)diff.tv_nsec < lim)
				continue;
			if (!got_one) {
				LOGMSG(env,
				"\nIdx ---->> Sec<<---- Nsec---MMMuuuNNN Marker\n");
				got_one = 1;
			}
			LOGMSG(env, "%4d %16ld %16ld %d -> %d\n", idx,
				diff.tv_sec, diff.tv_nsec,
				ts_p->ts_mkr[idx], ts_p->ts_mkr[idx+1]);
		}

		if (!got_one) {
			LOGMSG(env,
				"\nNo delays found bigger than %ld\n", lim);
		}
		LOGMSG(env,
			"\n==== ---->> Sec<<---- Nsec---MMMuuuNNN\n");
		LOGMSG(env, "Min: %16ld %16ld\n",
				min.tv_sec, min.tv_nsec);
		diff = time_div(tot, end_i - st_i);
		LOGMSG(env, "Avg: %16ld %16ld\n",
				diff.tv_sec, diff.tv_nsec);
		LOGMSG(env, "Max: %16ld %16ld\n",
				max.tv_sec, max.tv_nsec);
		break;
	default:
		LOGMSG(env, "FAILED: <cmd> not 's','p' or 'l'\n");
	}

exit:
	return 0;
}

struct cli_cmd UTime = {
"utime",
2,
3,
"Timestamp buffer command",
"<idx> <type> <cmd> <parms>\n"
"Display or find timestamps in the timestamp buffer\n"
"<idx> is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
"<type> is:\n"
"      'd' - descriptor timestamps\n"
"      'f' - FIFO (descriptor completions)\n"
"      'm' - measurement (development only)\n"
"<cmd> is the command to perform on the buffer, one of:\n"
"      's' - sample timestamps again\n"
"      '-' - return difference in two timestamp indices\n"
"            Note: Must enter two timestamp indexes\n"
"      'p' - print the existing counter values\n"
"            Note: optionally enter start and end indexes\n"
"      'l' - locate differences greater than x nsec\n"
"            Note: Must enter the number of nanoseconds\n",
UTimeCmd,
ATTR_NONE
};

static int Tsi721WCmd(struct cli_env *env, int argc, char **argv)
{
	int rc;
	uint16_t w_idx;
	uint64_t address;
	uint32_t win_size;
	char *mem_sizes = (char *)"32K 64K 128K 256K 512K 1M 2M 4M 8M 16M ";
	char *disp_sizes[] = {(char *)"32K",
				(char *)"64K",
				(char *)"128K",
				(char *)"256K",
				(char *)"512K",
				(char *)"1M",
				(char *)"2M",
				(char *)"4M",
				(char *)"8M",
				(char *)"16M" };
	uint32_t obwin_u, obwin_l, obwin_sz;

	if (tok_parse_ushort(argv[0], &w_idx, 0, 7, 0)) {
		LOGMSG(env, "Window index must be 0 to 7.\n");
		goto exit;
	}

	if (argc > 2) {
		uint32_t unused;
		uint32_t addr_mask;

		if (tok_parse_ull(argv[1], &address, 0)) {
			LOGMSG(env, "Unrecognized address value.\n");
			goto exit;
		}

		win_size = parm_idx(argv[2], mem_sizes);
		if (win_size > 9) {
			LOGMSG(env, "Window size must be one of:\n%s\n",
					mem_sizes);
			goto exit;
		}

		addr_mask = (1 << (15 + win_size)) - 1;
		if (address & addr_mask) {
			LOGMSG(env, "Window address must be aligned to"
				" window size");
			goto exit;
		}
		obwin_l = address & TSI721_OBWINLBX_ADD;
		obwin_l |= TSI721_OBWINLBX_WIN_EN;
		obwin_u = (uint32_t)(address >> 32) & TSI721_OBWINUBX_ADD;
		obwin_sz = (win_size << 8) & TSI721_OBWINSZX_SIZE;

		// Before changing the window, disable it.
		rc = rio_lcfg_write(mp_h, TSI721_OBWINLBX(w_idx), 4, 0);
		rc |= rio_lcfg_read(mp_h, TSI721_OBWINLBX(w_idx), 4, &unused);
		// Reprogram the window, re-enable is the last write
		rc |= rio_lcfg_write(mp_h, TSI721_OBWINSZX(w_idx), 4, obwin_sz);
		rc |= rio_lcfg_write(mp_h, TSI721_OBWINUBX(w_idx), 4, obwin_u);
		rc |= rio_lcfg_write(mp_h, TSI721_OBWINLBX(w_idx), 4, obwin_l);
		rc |= rio_lcfg_read(mp_h, TSI721_OBWINLBX(w_idx), 4, &unused);
		if (rc) {
			LOGMSG(env, "Error writing registers.\n");
			goto exit;
		}
	}

	rc = rio_lcfg_read(mp_h, TSI721_OBWINSZX(w_idx), 4, &obwin_sz);
	rc |= rio_lcfg_read(mp_h, TSI721_OBWINUBX(w_idx), 4, &obwin_u);
	rc |= rio_lcfg_read(mp_h, TSI721_OBWINLBX(w_idx), 4, &obwin_l);
	if (rc) {
		LOGMSG(env, "Error reading registers.\n");
		goto exit;
	}

	address = ((uint64_t)(obwin_u) << 32)
		| (uint64_t)(obwin_l & TSI721_OBWINLBX_ADD);
	win_size = (obwin_sz & TSI721_OBWINSZX_SIZE) >> 8;

	if (obwin_l & TSI721_OBWINLBX_WIN_EN) {
		LOGMSG(env, "Window %d enabled.\n", w_idx);
		LOGMSG(env, "Window    Address: 0x%lx\n", address);
		LOGMSG(env, "Window    Size   : 0x%x %s\n",
			win_size,
			win_size < 10 ? disp_sizes[win_size] : "Unknown");
	} else {
		LOGMSG(env, "Window %d Disabled.\n", w_idx);
	}
exit:
	return 0;
}

struct cli_cmd Tsi721W = {
"Window",
2,
1,
"Tsi721 Outbound Window command.",
"<window> {<PciAddr> <size>}\n"
"Manage Tsi721 outbound windows.  Optionally, set Tsi721 outbound windows.\n"
"<window> : Value 0-7, selecting a window\n"
"<PciAddr>: Memory address the window should respond to.\n"
"           This value MUST be in the currently configured BAR2 or BAR4.\n"
"           Use lspci from linux command line to determine current values.\n"
"<size>   : One of 32K, 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M.\n",
Tsi721WCmd,
ATTR_NONE
};

static int Tsi721ZCmd(struct cli_env *env, int argc, char **argv)
{
	int rc;
	uint16_t w_idx;
	uint16_t z_idx;
	uint64_t trans_addr;
	hc_t hc;
	did_val_t did;
	int dev16_sz;
	int mtc_trans;
	char *mem_or_mtc = (char *)"Mem Mtc ";
	char *dev8_or_16 = (char *)"dev8 dev16 ";
	uint32_t zone_sel, lut_0, lut_1, lut_2, temp;

	if (tok_parse_ushort(argv[0], &w_idx, 0, 7, 0)) {
		LOGMSG(env, "Window index must be 0 to 7.\n");
		goto exit;
	}

	if (tok_parse_ushort(argv[1], &z_idx, 0, 7, 0)) {
		LOGMSG(env, "Zone index must be 0 to 7.\n");
		goto exit;
	}

	if (argc >= 6) {
		mtc_trans = parm_idx(argv[2], mem_or_mtc);
		if (mtc_trans > 1) {
			LOGMSG(env, "Translation type must be one of:\n%s\n",
					mem_or_mtc);
			goto exit;
		}

		if (tok_parse_ull(argv[3], &trans_addr, 0)) {
			LOGMSG(env, "Unrecognized address value.\n");
			goto exit;
		}
		dev16_sz = parm_idx(argv[4], dev8_or_16);
		if (dev16_sz > 1) {
			LOGMSG(env, "DestID size must be be one of:\n%s\n",
					dev8_or_16);
			goto exit;
		}
		if (tok_parse_did(argv[5], &did, 0)) {
			LOGMSG(env, "Device ID must be between 0 and 0xFFFF\n");
			goto exit;
		}
		if (mtc_trans) {
			if (argc < 7) {
				LOGMSG(env, "Missing parameter: hopcount.\n");
				goto exit;
			}
			if (tok_parse_hc(argv[6], &hc, 0)) {
				LOGMSG(env, TOK_ERR_HC_MSG_FMT);
				goto exit;
			}
		}

		zone_sel = ((w_idx << 3) & TSI721_ZONE_SEL_WIN_SEL)
			| (z_idx & TSI721_ZONE_SEL_ZONE_SEL)
			| TSI721_ZONE_SEL_ZONE_GO;
		lut_0 = trans_addr & TSI721_LUT_DATA0_ADD;
		lut_1 = (trans_addr >> 32) & TSI721_LUT_DATA1_ADD;
		lut_2 = did & TSI721_LUT_DATA2_DEVICEID;
		if (mtc_trans) {
			lut_0 |= 0x202;
			lut_2 |= ((uint32_t)(hc) << 24)
				& TSI721_LUT_DATA2_HOP_CNT;
		} else {
			lut_0 |= 0x101;
		}
		if (dev16_sz) {
			lut_2 |= (0x10000 & TSI721_LUT_DATA2_TT);
		}

		LOGMSG(env, "Writing Lut0 0x%x\n", lut_0);
		LOGMSG(env, "Writing Lut1 0x%x\n", lut_1);
		LOGMSG(env, "Writing Lut2 0x%x\n\n", lut_2);

		// Check that zone_go is 0
		do
			rc = rio_lcfg_read(mp_h, TSI721_ZONE_SEL, 4, &temp);
		while (!rc && (temp & TSI721_ZONE_SEL_ZONE_GO));

		// Program the zone
		rc |= rio_lcfg_write(mp_h, TSI721_LUT_DATA0, 4, lut_0);
		rc |= rio_lcfg_write(mp_h, TSI721_LUT_DATA1, 4, lut_1);
		rc |= rio_lcfg_write(mp_h, TSI721_LUT_DATA2, 4, lut_2);
		rc |= rio_lcfg_write(mp_h, TSI721_ZONE_SEL, 4, zone_sel);

		// Wait for write zone registers to finish
		do
			rc = rio_lcfg_read(mp_h, TSI721_ZONE_SEL, 4, &temp);
		while (!rc && (temp & TSI721_ZONE_SEL_ZONE_GO));
		if (rc) {
			LOGMSG(env, "Error writing registers.\n");
			goto exit;
		}

	}

	// Check that zone_go is 0
	do
		rc = rio_lcfg_read(mp_h, TSI721_ZONE_SEL, 4, &temp);
	while (!rc && (temp & TSI721_ZONE_SEL_ZONE_GO));

	zone_sel = ((w_idx << 3) & TSI721_ZONE_SEL_WIN_SEL)
		| (z_idx & TSI721_ZONE_SEL_ZONE_SEL)
		| TSI721_ZONE_SEL_ZONE_GO
		| TSI721_ZONE_SEL_RD_WRB;
	rc |= rio_lcfg_write(mp_h, TSI721_ZONE_SEL, 4, zone_sel);

	// Wait until read of data registers is complete (zone_go == 0)
	do
		rc = rio_lcfg_read(mp_h, TSI721_ZONE_SEL, 4, &temp);
	while (!rc && (temp & TSI721_ZONE_SEL_ZONE_GO));

	rc |= rio_lcfg_read(mp_h, TSI721_LUT_DATA0, 4, &lut_0);
	rc |= rio_lcfg_read(mp_h, TSI721_LUT_DATA1, 4, &lut_1);
	rc |= rio_lcfg_read(mp_h, TSI721_LUT_DATA2, 4, &lut_2);
	if (rc) {
		LOGMSG(env, "Error reading registers.\n");
		goto exit;
	}

	trans_addr = (uint64_t)(lut_0 & TSI721_LUT_DATA0_ADD)
		| ((uint64_t)(lut_1 & TSI721_LUT_DATA1_ADD) << 32);
	mtc_trans = ((lut_0 & TSI721_LUT_DATA0_WR_TYPE) == 2);
	did = lut_2 & TSI721_LUT_DATA2_DEVICEID;
	dev16_sz = !!(lut_2 & TSI721_LUT_DATA2_TT);
	hc = (lut_2 & TSI721_LUT_DATA2_HOP_CNT) >> 24;

	LOGMSG(env, "Zone Address: 0x%lx\n", trans_addr);
	LOGMSG(env, "Translation : %s\n", mtc_trans ? "Mtc" : "Mem");
	LOGMSG(env, "DestID size : %s\n", dev16_sz ? "dev16" : "dev8");
	LOGMSG(env, "DestID      : 0x%x\n", did);
	LOGMSG(env, "Hopcount    : 0x%x\n", hc);
exit:
	return 0;
}

struct cli_cmd Tsi721Z = {
"Zone",
2,
2,
"Tsi721 Outbound Window Zone command.",
"<window> <zone> {<Mem|Mtc> <rio_addr> <dev8|dev16> <did> {hopcount}}\n"
"Display and/or write Tsi721 outbound window zones.\n"
"window    : Value 0-7, selecting a window\n"
"zone      : Value 0-7, selecting a zone\n"
"Mem|Mtc   : Select read/write translation type, either Memory or Maintenance\n"
"            If Maintenance is selected, hopcount must be entered.\n"
"rio_addr  : RapidIO memory address, or maintenance offset.\n"
"dev8|dev16: Size of destination ID to use, either 8 or 16 bit.\n"
"did       : Destination ID value.\n"
"hopcount  : Hopcount for maintenance transactions.\n",
Tsi721ZCmd,
ATTR_NONE
};


struct cli_cmd *goodput_cmds[] = {
	&IBAlloc,
	&IBDealloc,
	&Dump,
	&Fill,
	&OBDIO,
	&OBDIOTxLat,
	&OBDIORxLat,
	&dmaTxLat,
	&dmaRxLat,
	&dma,
	&dmaNum,
	&dmaRxGoodput,
	&msgTx,
	&msgRx,
	&msgTxLat,
	&msgRxLat,
	&msgTxOh,
	&msgRxOh,
	&Goodput,
	&Lat,
	&Status,
	&Thread,
	&Kill,
	&Halt,
	&Move,
	&Wait,
	&Sleep,
	&CPUOccSet,
	&CPUOccDisplay,
	&Mpdevs,
	&Multicast,
	&UTime,
	&RegScrub,
	&LRead,
	&LWrite,
	&RRead,
	&RWrite,
	&QueryDma,
	&SevenTest,
	&SevenHotSwap,
	&SevenStatus,
	&SevenMagic,
	&SevenMECS,
	&CPSStatus,
	&CPSCounters,
	&SevenClearTsi721,
	&SevenResetTsi721,
	&SevenResetLink,
	&SevenLinkReq,
	&SevenAckidSync,
	&SevenDisablePort,
	&CPSPortWrite,
	&TsiPortWrite,
	&CPSVerification,
	&CPSEvent,
	&CPSReset,
	&FileRegs,
	&CPSHotSwap,
	&MaintTraffic,
	&Tsi721W,
	&Tsi721Z,
	&UMDDmaNum,
	&UMDOpen,
	&UMDConfig,
	&UMDStart,
	&UMDStop,
	&UMDClose
};

void bind_goodput_cmds(void)
{
	dump_idx = 0;
	dump_base_offset = 0;
	dump_size = 0x100;
	cpu_occ_saved_idx = 0;

	cpu_occ_valid = 0;
	new_proc_user_jifis = 0;
	new_proc_kern_jifis = 0;
	old_proc_user_jifis = 0;
	old_proc_kern_jifis = 0;
	old_tot_jifis = 1;
	new_tot_jifis = 2;
	cpu_occ_pct = 0.0;

	acc_offset = 0;
	acc_data = 0;
	racc_hc = 0xff;
	racc_did = 0x1;

	sem_init(&tsi721_mutex, 0, 0);
	sem_post(&tsi721_mutex);

	add_commands_to_cmd_db(sizeof(goodput_cmds) / sizeof(goodput_cmds[0]),
			goodput_cmds);
}

#ifdef __cplusplus
}
#endif
