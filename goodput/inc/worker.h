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

#ifndef __WORKER_H__
#define __WORKER_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "rio_route.h"
#include "rio_mport_lib.h"
#include "libtime_utils.h"
#include "RapidIO_Device_Access_Routines_API.h"
#include "liblist.h"
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

enum req_type {
	no_action,
	direct_io,
	direct_io_tx_lat,
	direct_io_rx_lat,
	dma_tx,
	dma_tx_num,
	dma_tx_lat,
	dma_rx_lat,
	dma_rx_gp,
	message_tx,
	message_tx_lat,
	message_tx_oh,
	message_rx,
	message_rx_lat,
	message_rx_oh,
	alloc_ibwin,
	free_ibwin,
	shutdown_worker,
	reg_scrub,
	seven_test_switch_mgmt,
	seven_test_721I,
	seven_test_721_recovery,
	seven_handle_link_init,
	seven_test_pw_rx,
	cps_handle_cps_pw,
	cps_handle_721_pw,
	cps_poll_for_pw,
	cps_test_switch_lock,
	maint_traffic,
	umd_dma_num,
	last_action
};

enum req_mode {
	kernel_action,
	user_mode_action
};

#define MIN_RDMA_BUFF_SIZE 0x10000

struct thread_cpu {
	int cpu_req; /* Requested CPU, -1 means no CPU affinity */
	int cpu_run; /* Currently running on this CPU */
	pthread_t thr; /* Thread being migrated... */
};

enum thread_type{
    kernel_thread,
    user_mode_thread
};

struct worker {
	int idx; /* index of this worker thread */
	struct thread_cpu wkr_thr;
	sem_t started;
	enum thread_type thr_type;
	int stat; /* 0 - dead, 1 - running, 2 stopped */
	volatile int stop_req; /* 0 - continue, 1 - stop 2 - shutdown, SOFT_RESTART */
	volatile int port_ok; // True if endpoint's link is in working order,
				// False if DMA must wait for links return.
	sem_t run;  /* Managed by controller, post this sem to start a stopped woker */
	enum req_type action;
	enum req_mode action_mode;
	did_val_t did_val;
	did_val_t prev_did_val;
	hc_t hc;

	uint16_t ignore_dma_errs; // 0 - halt on DMA error, 1: continue
	uint64_t rio_addr; /* Target RapidIO address for direct IO and DMA */
	uint64_t byte_cnt; /* Number of bytes to access for direct IO and DMA */
	uint64_t acc_size; /* Bytes per transfer for direct IO and DMA */
	int      max_iter; /* For infinite loop tests make this the upper bound of loops*/

	int wr;
	int mp_num;	/* Mport index */
	int mp_h_is_mine; /* 0 - common mp_h, 1 - worker specific mp_h */
	int mp_h;

	int ob_valid;
	uint64_t ob_handle; /* Outbound window RapidIO address */
	uint64_t ob_byte_cnt; /* Outbound window size */
	void *ob_ptr; /* Pointer to mapped ob_handle */

	int ib_valid;
	uint64_t ib_handle; /* Inbound window RapidIO handle */
	uint64_t ib_rio_addr; /* Inbound window RapidIO address */
	uint64_t ib_byte_cnt; /* Inbound window size */
	void *ib_ptr; /* Pointer to mapped ib_handle */

	uint8_t data8_tx;
	uint16_t data16_tx;
	uint32_t data32_tx;
	uint64_t data64_tx;

	uint8_t data8_rx;
	uint16_t data16_rx;
	uint32_t data32_rx;
	uint64_t data64_rx;

	int use_kbuf;
	enum rio_exchange dma_trans_type;
	enum rio_transfer_sync dma_sync_type;
	uint64_t rdma_kbuff;
	uint64_t rdma_buff_size;
	void *rdma_ptr;
	int num_trans;

	int mb_valid;
	rio_mailbox_t mb;
	rio_socket_t acc_skt;
	int acc_skt_valid;
	rio_socket_t con_skt;
	int con_skt_valid;
	int msg_size;  /* Minimum 20 bytes for CM messaging!!! */
	uint16_t sock_num; /* RIO CM socket to connect to */
	void *sock_tx_buf;
	void *sock_rx_buf;

	uint64_t perf_msg_cnt; /* Messages read/written */
	uint64_t perf_byte_cnt; /* bytes read/written */
	struct timespec st_time; /* Start of the run, for throughput */
	struct timespec end_time; /* End of the run, for throughput*/

	uint64_t perf_iter_cnt; /* Number of repetitions */
	struct timespec iter_st_time; /* Start of the iteration, latency */
	struct timespec iter_end_time; /* End of the iteration, latency */
	struct timespec tot_iter_time; /* Total time for all iterations */
	struct timespec min_iter_time; /* Minimum time over all iterations */
	struct timespec max_iter_time; /* Maximum time over all iterations */
	struct timespec iter_time_lim; /* Maximum time for an iteration. */
					/* Drop all times above this limit */

	struct seq_ts desc_ts;
	struct seq_ts fifo_ts;
	struct seq_ts meas_ts;

	uint16_t ssdist;
	uint16_t sssize;
	uint16_t dsdist;
	uint16_t dssize;

/* pw mutex, list, and process_pw are only used by the CPS hot swap
 * scheme.
 */
	void *status;
	DAR_DEV_INFO_t *dev_h;
	sem_t pw_mutex;
	struct l_head_t pw;
	sem_t process_pw;
#define TSI721_I 0
#define CPS_I 1
	struct worker *hs_worker[2];
	uint32_t tsi721_ct;
	uint32_t seven_test_period;
	uint32_t seven_test_downtime;
	uint32_t seven_test_err_resp_time;
	uint32_t seven_test_resp_to_time;
	float seven_test_delay;

/*  UMD data structure TBD */
	int umd_engine_h;
	int umd_queue_h;	
};

/**
 * @brief Returns number of CPUs as reported in /proc/cpuinfo
 */
int getCPUCount();

/**
 * @brief Initializes worker structure
 *
 * @param[in] info Pointer to worker info
 * @param[in] first_time If non-zero, performs one-time initialization
 * @return None, always succeeds
 */

void init_worker_info(struct worker *info, int first_time);

/**
 * @brief Starts a worker thread that performs Direct IO, DMA, and/or messaging
 *
 * @param[in] info Pointer to worker info, must have been initialized by
 *		init_worker_info prior to this call.
 * @param[in] new_mp_h If <> 0, open mport again to get new DMA channel
 * @param[in] cpu The cpu that should run the thread, -1 for all cpus
 * @return Null pointer, no return status
 */

void start_worker_thread(struct worker *info, int new_mp_h, int cpu);

/**
 * @brief Stops a worker thread and cleans up all resources.
 *
 * @param[in] info Pointer to worker info
 * @return None, always succeeds.
 */

void shutdown_worker_thread(struct worker *info);

void *worker_thread(void *parm);

bool dma_alloc_ibwin(struct worker *info);

#define MAX_DEVID_STATUS 18
extern volatile int devid_status[MAX_DEVID_STATUS];

// Period for Dead Link Timer detection of loss of lane sync.
// After performing an action which resets the port, it is important to
// delay for this period of time.  If the port has not trained in this
// time, a Dead Link Timer event is waiting...
//
// Note that one DLT tick is 256 usec - a long time...
// DLT_DELAY_NSEC is therefor 1.024 msec.
#define DLT_DELAY_NSEC ((TSI721_PLM_IMP_SPEC_CTL_DLT_TICK_USEC * 1000) * 4)
// The logical layer response timeout must be greater than the DLT_DELAY
// Otherwise, a response timeout could occur before the DLT has
// kicked in to drain requests and allow the channel/Tsi721 to be reset.
//
// Response timeout is 4.096 msec
#define RSP_TO_DELAY_NSEC (DLT_DELAY_NSEC * 4)
// The TTL_TIMEOUT must be less than half of the logical layer response
// timeout to ensure that all transactions associated with a logical layer
// transaction are discarded when a logical layer response timeout occurs.
//
// TTl timeout is 1.638 msec
#define TTL_TIMEOUT_NSEC (RSP_TO_DELAY_NSEC*2/5)

#define TSI721_OK 0
#define TSI721_AGAIN 1
#define TSI721_FAIL 2


int tsi721_config_port_reset(void);
int tsi721_link_reset(void);
int tsi721_reset_own_port_from_cli(void);
int tsi721_clear_all_from_cli(void);

uint32_t SRIO_API_WriteRegFunc(DAR_DEV_INFO_t *d_info,
                                uint32_t offset,
                                uint32_t writedata);
void SRIO_API_DelayFunc(uint32_t delay_nsec, uint32_t delay_sec);
uint32_t SRIO_API_ReadRegFunc(DAR_DEV_INFO_t *d_info,
                                uint32_t offset,
                                uint32_t *readdata);
int tsi721_sync_cps_ackids(uint32_t port);

uint32_t disable_switch_port(DAR_DEV_INFO_t *dev_h,
                                rio_port_t pw_port);

extern sem_t tsi721_mutex;

uint32_t tsi721_link_req(uint32_t *response);

#ifdef __cplusplus
}
#endif

#endif /* __WORKER_H__ */
