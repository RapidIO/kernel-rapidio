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

#ifndef __LIBUNIT_TEST_H__
#define __LIBUNIT_TEST_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>

#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>

#include "libtime_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct worker
 *
 * @brief Structure for each worker thread in the goodput environment
 *
 * Every worker thread is passed one of these structures to control what
 * it is doing.
 *
 */
struct thread_cpu {
	int cpu_req; /* Requested CPU, -1 means no CPU affinity */
	int cpu_run; /* Currently running on this CPU */
	pthread_t thr; /* Thread being migrated... */
};

enum worker_stat {
	worker_dead, worker_halted, worker_running
};

#define MAX_UNIT_TEST_TS_IDX 3
#define UNIT_TEST_NO_ACTION 0
#define UNIT_TEST_SHUTDOWN  -1

struct worker {
	// Index of this worker thread
	int idx;

	// thread structure for the worker
	struct thread_cpu wkr_thr;

	// Worker posts this sema when it has started
	sem_t started;

	// Current status of this worker
	enum worker_stat stat;

	// worker_run means keep going
	// worker_halt means halt the current action
	// worker_dead means the thread should clean up, exit
	volatile enum worker_stat stop_req;

	// Managed by controller, post this sem to start a stopped worker
	sem_t run;

	// Test specific code for the worker action.
	// 0 means "no action".
	// -1 means "cleanup and exit".
	int action;

	struct seq_ts ts[MAX_UNIT_TEST_TS_IDX];

	// Test specific worker information
	volatile void *priv_info;
};

#define MAX_WORKER_IDX 11
#define MAX_WORKERS (MAX_WORKER_IDX+1)

extern struct worker wkr[MAX_WORKERS];

/**
 * @brief Re-Initializes worker structure
 *
 * May only be called after config_unit_test().
 *
 * @param[in] info Pointer to worker info
 * @return None, always succeeds
 */
void init_worker(struct worker *info);

/**
 * @brief Returns number of CPUs as reported in /proc/cpuinfo
 */
int getCPUCount();

/**
 * @brief Starts a worker thread
 *
 * @param[in] info Pointer to worker info, must have been initialized by
 *		init_worker_info prior to this call.
 * @param[in] cpu The cpu that should run the thread, -1 for no cpu affinity
 * @return 0 - success, 1 on failure
 */
int start_worker_thread(struct worker *info, int cpu);

/**
 * @brief Dispatches an action to the specified worker
 *
 * @param[in] info Pointer to worker info, must currently be halted.
 * @param[in] action Action to be performed by the worker.
 * Note that other data may need to be configured before running the action.
 * @return 0 - success, 1 on failure
 */
int run_worker_action(struct worker *info, int action);

/**
 * @brief Changes CPU affinity for a thread
 *
 * @param[in] info Pointer to worker info, must have been initialized by
 *		init_worker_info prior to this call, and worker must be halted
 * @param[in] cpu The new cpu that should run the thread, -1 for no cpu affinity
 * @return Null pointer, no return status
 */
int migrate_worker_thread(struct worker *info, int cpu);

/**
 * @brief Signal that the thread should die, and then wait for it to die.
 *
 * Returns when the thread is dead.
 *
 * @param[in] info Pointer to worker info, must have been initialized by
 *		init_worker_info prior to this call, and worker must be halted
 * @return Null pointer, no return status
 */
void kill_worker_thread(struct worker *info);

/**
 * @brief Signal that the thread should halt, and wait for it to halt.
 *
 * @param[in] info Pointer to worker info, must have been initialized by
 *		init_worker_info prior to this call, and worker must be halted
 * @return Null pointer, no return status
 */
void halt_worker_thread(struct worker *info);

/**
 * @brief Waits until the thread has achieved the desired state.
 *
 * @param[in] info Pointer to worker info, must have been initialized by
 *		init_worker_info prior to this call, and worker must be halted
 * @param[in] desired Wait until the thread has the desired state.
 * @return 0 - success, 1 - failure/timeout
 */
int wait_for_worker_status(struct worker *info, enum worker_stat desired);

struct unit_test_driver {
	// Return constant string for action
	char *(*action_str)(int action);

	// Translates parm to info->tx[] index
	int (*ts_sel)(char *parm);

	// Support for actions
	// Non-zero rc for failure
	int (*worker_body)(struct worker *info);

	// Init info->priv_info
	void (*create_priv)(struct worker *info);

	// Free worker resources
	void (*destroy_priv)(struct worker *info);
};

/**
 * @brief Initialize unit test base and bind drivers.
 */
void config_unit_test(struct unit_test_driver *drv);

/**
 * @brief Start a new console with the unit test thread commands and others
 *
 * @param[in] title_str Title used for the test environment CLI header
 * @param[in] prompt_str String used as the prefix for the CLI prompt.
 */
void run_unit_test_cli(char *title_str, char *prompt_str);

/**
 * @brief Bind the unit test thread commands into another CLI session.
 */
void bind_unit_test_thread_cli_cmds(void);

#ifdef __cplusplus
}
#endif

#endif /* __LIBUNIT_TEST_H__ */
