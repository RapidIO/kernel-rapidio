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
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <stdio.h>
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

#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include <pthread.h>

#include <sched.h>

#include "rio_route.h"
#include "string_util.h"
#include "libcli.h"
#include "rio_mport_lib.h"
#include "liblog.h"

#include "libtime_utils.h"
#include "worker.h"
#include "pw_handling.h"
#include "goodput.h"
#include "Tsi721.h"
#include "CPS1848.h"
#include "rio_misc.h"
#include "did.h"

#include "RapidIO_Error_Management_API.h"
#include "RapidIO_Port_Config_API.h"
#include "RapidIO_Routing_Table_API.h"

#ifdef __cplusplus
extern "C" {
#endif

volatile int devid_status[MAX_DEVID_STATUS];

sem_t tsi721_mutex;

void init_worker_info(struct worker *info, int first_time)
{
	if (first_time) {
		sem_init(&info->started, 0, 0);
		sem_init(&info->run, 0, 0);
	}

	info->stat = 0;
	info->stop_req = 0;
	info->port_ok = 1;
	info->wkr_thr.cpu_req = -1;
	info->wkr_thr.cpu_run = -1;
	info->action = no_action;
	info->action_mode = kernel_action;
	info->did_val = (did_val_t)(-1);

	info->rio_addr = 0;
	info->byte_cnt = 0;
	info->acc_size = 0; /* Bytes per transfer for direct IO and DMA */
	info->max_iter = -1;
	info->wr = 0;
	info->mp_h_is_mine = 0;
	info->mp_h = -1;

	info->ob_valid = 0;
	info->ob_handle = 0;
	info->ob_byte_cnt = 0;

	info->ib_valid = 0;
	info->ib_handle = 0;
	info->ib_rio_addr = 0;
	info->ib_byte_cnt = 0;
	info->ib_ptr = NULL;

	info->data8_tx = 0x12;
	info->data16_tx= 0x3456;
	info->data32_tx = 0x789abcde;
	info->data64_tx = 0xf123456789abcdef;

	info->data8_rx = 0;
	info->data16_rx = 0;
	info->data32_rx = 0;
	info->data64_rx = 0;

	info->use_kbuf = 0;
	info->dma_trans_type = RIO_EXCHANGE_NWRITE;
	info->dma_sync_type = RIO_TRANSFER_SYNC;
	info->rdma_kbuff = 0;
	info->rdma_ptr = NULL;
	info->num_trans = 0;

	info-> mb_valid = 0;
	info->acc_skt = NULL;
	info->acc_skt_valid = 0;
	info->con_skt = NULL;
	info->con_skt_valid = 0;
	info->sock_num = 0;
	info->sock_tx_buf = NULL;
	info->sock_rx_buf = NULL;

	init_seq_ts(&info->desc_ts, MAX_TIMESTAMPS);
	init_seq_ts(&info->fifo_ts, MAX_TIMESTAMPS);
	init_seq_ts(&info->meas_ts, MAX_TIMESTAMPS);

	info->ssdist = 0;
	info->sssize = 0;
	info->dsdist = 0;
	info->dssize = 0;
}

void msg_cleanup_con_skt(struct worker *info);
void msg_cleanup_acc_skt(struct worker *info);
void msg_cleanup_mb(struct worker *info);
void direct_io_obwin_unmap(struct worker *info);
void dma_free_ibwin(struct worker *info);
void dealloc_dma_tx_buffer(struct worker *info);

void shutdown_worker_thread(struct worker *info)
{
	int rc;
	if (info->stat) {
		info->action = shutdown_worker;
		info->stop_req = 2;
		sem_post(&info->run);
		pthread_join(info->wkr_thr.thr, NULL);
	}

	dma_free_ibwin(info);
	direct_io_obwin_unmap(info);
	dealloc_dma_tx_buffer(info);

	msg_cleanup_con_skt(info);
	msg_cleanup_acc_skt(info);
	msg_cleanup_mb(info);

	if (info->mp_h_is_mine) {
		rc = close(info->mp_h);
		info->mp_h = -1;
		info->mp_h_is_mine = 0;
		if (rc) {
			ERR("Shutdown mp_h rc %d:%s\n",
					rc, strerror(errno));
		}
	}

	init_worker_info(info, 0);
}

int getCPUCount()
{
	FILE* f = fopen("/proc/cpuinfo", "rte");
	int count = 0;

	if (NULL == f) {
		CRIT("Could not open /proc/cpuinfo\n");
		return 1;
	}

	while (! feof(f)) {
		char buf[257] = {0};
		if (NULL == fgets(buf, 256, f)) {
			break;
		}
		if (buf[0] == '\0') {
			break;
		}
		if (strstr(buf, "processor\t:")) {
			count++;
		}
	}

	fclose(f);

	return count;
}

int migrate_thread_to_cpu(struct thread_cpu *info)
{
	const struct timespec one_usec = {0, 1 * 1000};

	cpu_set_t cpuset;
	int chk_cpu_lim = 10;
	int rc;

	const int cpu_count = getCPUCount();

	if (-1 == info->cpu_req) {
		CPU_ZERO(&cpuset);
		for (int c = 0; c < cpu_count; c++) {
			CPU_SET(c, &cpuset);
		}
	} else {
		if (info->cpu_req >= cpu_count) {
			ERR("\n\tInvalid cpu %d cpu count is %d\n",
					info->cpu_req, cpu_count);
			return 1;
		}
		CPU_ZERO(&cpuset);
		CPU_SET(info->cpu_req, &cpuset);
	}

	rc = pthread_setaffinity_np(info->thr, sizeof(cpu_set_t), &cpuset);
	if (rc) {
		ERR("pthread_setaffinity_np rc %d:%s\n", rc, strerror(errno));
		return 1;
	}

	if (-1 == info->cpu_req) {
		info->cpu_run = info->cpu_req;
		return 0;
	}

	rc = pthread_getaffinity_np(info->thr, sizeof(cpu_set_t), &cpuset);
	if (rc) {
		ERR("pthread_getaffinity_np rc %d:%s\n", rc, strerror(errno));
		return 1;
	}

	info->cpu_run = sched_getcpu();
	while ((info->cpu_run != info->cpu_req) && chk_cpu_lim) {
		time_sleep(&one_usec);
		info->cpu_run = sched_getcpu();
		chk_cpu_lim--;
	}
	rc = info->cpu_run != info->cpu_req;
	if (rc) {
		ERR("Unable to schedule thread on cpu %d\n", info->cpu_req);
		return 1;
	}
	return rc;
}

void zero_stats(struct worker *info)
{
	info->perf_msg_cnt = 0;
	info->perf_byte_cnt = 0;
	info->perf_iter_cnt = 0;
	info->min_iter_time = (struct timespec){0,0};
	info->tot_iter_time = (struct timespec){0,0};
	info->max_iter_time = (struct timespec){0,0};
	info->iter_time_lim = (struct timespec){0xFFFFFFFF,0xFFFFFFFF};

	info->data8_tx = 0x12;
	info->data16_tx = 0x3456;
	info->data32_tx = 0x789abcde;
	info->data64_tx = 0xf123456789abcdef;

	info->data8_rx = 0;
	info->data16_rx = 0;
	info->data32_rx = 0;
	info->data64_rx = 0;
}

void start_iter_stats(struct worker *info)
{
	clock_gettime(CLOCK_MONOTONIC, &info->iter_st_time);
}

void finish_iter_stats(struct worker *info)
{
	clock_gettime(CLOCK_MONOTONIC, &info->iter_end_time);
	time_track_lim(info->perf_iter_cnt, &info->iter_time_lim,
		&info->iter_st_time, &info->iter_end_time,
		&info->tot_iter_time, &info->min_iter_time,
		&info->max_iter_time);
	info->perf_iter_cnt++;
}

void direct_io_tx(struct worker *info, volatile void * volatile ptr)
{
	while (!info->port_ok && !info->stop_req) {
		const struct timespec point_one_ms = {0, 100000};
		nanosleep(&point_one_ms, NULL);
	}
	if (info->stop_req)
		return;

	switch (info->acc_size) {
	case 1: if (info->wr)
			*(uint8_t *)ptr = info->data8_tx;
		else
			info->data8_rx = *(uint8_t *)ptr;
		break;

	case 2: if (info->wr)
			*(uint16_t *)ptr = info->data16_tx;
		else
			info->data16_rx = *(uint16_t *)ptr;
		break;
	case 3:
	case 4: if (info->wr)
			*(uint32_t *)ptr = info->data32_tx;
		else
			info->data32_rx = *(uint32_t *)ptr;
		break;

	case 8: if (info->wr)
			*(uint64_t *)ptr = info->data64_tx;
		else
			info->data64_rx = *(uint64_t *)ptr;
		break;
	default:
		break;
	}
}

void init_direct_io_rx_data(struct worker *info, void * volatile ptr )
{
	switch (info->acc_size) {
	case 1: *(uint8_t *)ptr = ~info->data8_tx;
		break;

	case 2: *(uint16_t *)ptr = ~info->data16_tx;
		break;
	case 3:
	case 4: *(uint32_t *)ptr = ~info->data32_tx;
		break;

	case 8: *(uint64_t *)ptr = ~info->data64_tx;
		break;
	default:
		break;
	}
}

int direct_io_wait_for_change(struct worker *info)
{
	volatile void * volatile ptr = info->ib_ptr;
	uint64_t no_change = 1;
	uint64_t limit;

	if ((info->action == direct_io_tx_lat) && !info->wr)
		return 0;

	while (!info->stop_req && no_change) {
		limit = 0x100000;
		do {
			switch (info->acc_size) {
			case 1: info->data8_rx = *(uint8_t *)ptr;
				no_change = (info->data8_rx != info->data8_tx);
				break;

			case 2: info->data16_rx = *(uint16_t *)ptr;
				no_change = (info->data16_rx != info->data16_tx);
				break;
			case 3:
			case 4: info->data32_rx = *(uint32_t *)ptr;
				no_change = (info->data32_rx != info->data32_tx);
				break;

			case 8: info->data64_rx = *(uint64_t *)ptr;
				no_change = (info->data64_rx != info->data64_tx);
				break;
			default:
				no_change = 0;
				break;
			}
		} while (no_change && --limit);
	}
	return 0;
}

void incr_direct_io_data(struct worker *info)
{
	info->data8_tx++;
	info->data16_tx++;
	info->data32_tx++;
	info->data64_tx++;
}

int direct_io_obwin_map(struct worker *info)
{
	int rc;

	rc = rio_obwin_map(info->mp_h, info->did_val, info->rio_addr,
					info->ob_byte_cnt, &info->ob_handle);
	if (rc) {
		ERR("FAILED: riomp_dma_obwin_map rc %d:%s\n",
					rc, strerror(errno));
		goto exit;
	}

	info->ob_valid = 1;

	info->ob_ptr = mmap(NULL, info->ob_byte_cnt, PROT_READ | PROT_WRITE,
		MAP_SHARED, info->mp_h, info->ob_handle);

	if (info->ob_ptr == MAP_FAILED)
		info->ob_ptr = NULL;

	if (info->ob_ptr == NULL) {
		ERR("FAILED: mmap %d:%s\n",errno,strerror(errno));
		rc = 1;
		goto exit;
	}

	// rc = riomp_dma_map_memory(info->mp_h, info->ob_byte_cnt,
	//			info->ob_handle, &info->ob_ptr);
	//if (rc)
	//	ERR("FAILED: riomp_dma_map_memory rc %d:%s\n",
	//		rc, strerror(errno));
exit:
	return rc;
}

void direct_io_obwin_unmap(struct worker *info)
{
	int rc = 0;

	if (info->ob_ptr && info->ob_valid) {
		rc = munmap(info->ob_ptr, info->ob_byte_cnt);
		info->ob_ptr = NULL;
		if (rc)
			ERR("FAILED: munmap rc %d:%s\n",
						rc, strerror(errno));
	}

	if (info->ob_handle) {
		rc = rio_dbuf_free(info->mp_h, &info->ob_handle);
		if (rc)
			ERR("FAILED: rio_obwin_free rc %d:%s\n",
						rc, strerror(errno));
	}

	info->ob_valid = 0;
	info->ob_handle = 0;
	info->ob_byte_cnt = 0;
	info->ob_ptr = NULL;
}

void direct_io_goodput(struct worker *info)
{
	if (!info->rio_addr || !info->byte_cnt || !info->acc_size) {
		ERR("FAILED: rio_addr, window size or acc_size is 0\n");
		return;
	}

	if (!info->ob_byte_cnt) {
		ERR("FAILED: ob_byte_cnt is 0\n");
		return;
	}

	if (direct_io_obwin_map(info))
		goto exit;

	zero_stats(info);
	clock_gettime(CLOCK_MONOTONIC, &info->st_time);

	while (!info->stop_req) {

		uint64_t cnt;

		for (cnt = 0; (cnt < info->byte_cnt) && !info->stop_req;
							cnt += info->acc_size) {
			volatile void * volatile ptr;

			ptr = (void *)((uint64_t)info->ob_ptr + cnt);

			direct_io_tx(info, ptr);
		}

		info->perf_byte_cnt += info->byte_cnt;
		clock_gettime(CLOCK_MONOTONIC, &info->end_time);
		incr_direct_io_data(info);
	}
exit:
	direct_io_obwin_unmap(info);
}

void direct_io_tx_latency(struct worker *info)
{
	if (!info->rio_addr || !info->byte_cnt || !info->acc_size) {
		ERR("FAILED: rio_addr, window size or acc_size is 0\n");
		return;
	}

	if (!info->ob_byte_cnt) {
		ERR("FAILED: ob_byte_cnt is 0\n");
		return;
	}

	if (info->byte_cnt != info->acc_size) {
		ERR("WARNING: access size == byte count for latency\n");
		info->byte_cnt = info->acc_size;
	}

	if (!info->ib_valid && info->wr) {
		ERR("FAILED: ibwin is not valid!\n");
		return;
	}

	if (direct_io_obwin_map(info))
		goto exit;

	zero_stats(info);
	/* Set maximum latency time to 5 microseconds */
	info->iter_time_lim = (struct timespec){0, 5000};
	clock_gettime(CLOCK_MONOTONIC, &info->st_time);

	while (!info->stop_req) {
		if (info->wr) {
			init_direct_io_rx_data(info, info->ob_ptr);
			init_direct_io_rx_data(info, info->ib_ptr);
		}

		start_iter_stats(info);

		direct_io_tx(info, info->ob_ptr);
		if (direct_io_wait_for_change(info))
			goto exit;
		finish_iter_stats(info);

		info->perf_byte_cnt += info->byte_cnt;
		clock_gettime(CLOCK_MONOTONIC, &info->end_time);
		incr_direct_io_data(info);
	}
exit:
	direct_io_obwin_unmap(info);
}

void direct_io_rx_latency(struct worker *info)
{
	if (!info->rio_addr || !info->byte_cnt || !info->acc_size) {
		ERR("FAILED: rio_addr, window size or acc_size is 0\n");
		return;
	}

	if (!info->ob_byte_cnt) {
		ERR("FAILED: ob_byte_cnt is 0\n");
		return;
	}

	if (info->byte_cnt != info->acc_size) {
		ERR("WARNING: access size == byte count for latency\n");
		info->byte_cnt = info->acc_size;
	}

	if (!info->ib_valid) {
		ERR("FAILED: ibwin is not valid!\n");
		return;
	}

	if (direct_io_obwin_map(info))
		goto exit;

	zero_stats(info);
	clock_gettime(CLOCK_MONOTONIC, &info->st_time);

	while (!info->stop_req) {
		if (direct_io_wait_for_change(info))
			goto exit;
		direct_io_tx(info, info->ob_ptr);
		incr_direct_io_data(info);
	}
exit:
	direct_io_obwin_unmap(info);

}

#define ADDR_L(x,y) ((uint64_t)((uint64_t)x + (uint64_t)y))
#define ADDR_P(x,y) ((void *)((uint64_t)x + (uint64_t)y))


int alloc_dma_tx_buffer(struct worker *info)
{
	int rc = 0;

	if (info->use_kbuf) {
		info->rdma_kbuff = RIO_MAP_ANY_ADDR;
		rc = rio_dbuf_alloc(info->mp_h, info->rdma_buff_size,
					&info->rdma_kbuff);
		if (rc) {
			ERR("FAILED: riomp_dma_dbuf_alloc rc %d:%s\n",
						rc, strerror(errno));
			goto exit;
		}
		info->rdma_ptr = NULL;
		info->rdma_ptr = mmap(NULL, info->rdma_buff_size,
			PROT_READ | PROT_WRITE, MAP_SHARED,
			info->mp_h, info->rdma_kbuff);
		// rc = riomp_dma_map_memory(info->mp_h, info->rdma_buff_size,
		//			info->rdma_kbuff, &info->rdma_ptr);
		if (info->rdma_ptr == MAP_FAILED)
			info->rdma_ptr = NULL;

		if (NULL == info->rdma_ptr) {
			ERR("FAILED: mmap rc %d:%s\n",
						rc, strerror(errno));
			rc = 1;
			goto exit;
		}
	} else {
		info->rdma_ptr = malloc(info->rdma_buff_size);
		if (NULL == info->rdma_ptr) {
			rc = 1;
			ERR("FAILED: Could not allocate local memory!\n");
			goto exit;
		}
	}

	return 0;
exit:
	return rc;
}

void dealloc_dma_tx_buffer(struct worker *info)
{
	if (info->use_kbuf) {
		if (NULL != info->rdma_ptr) {
			munmap(info->rdma_ptr, info->rdma_buff_size);
			info->rdma_ptr = NULL;
		}
		if (info->rdma_kbuff) {
			rio_dbuf_free(info->mp_h, &info->rdma_kbuff);
			info->rdma_kbuff = 0;
		}
	} else {
		if (NULL != info->rdma_ptr) {
			free(info->rdma_ptr);
			info->rdma_ptr = NULL;
		}
	}
}

int single_dma_access(struct worker *info, uint64_t offset)
{
	int dma_rc = 0;
	did_reg_t did_val = info->did_val;
	const int ANY_DID = DID_ANY_VAL(dev08_sz);
	struct rio_dma_interleave interleave;

	interleave.ssdist = info->ssdist;
	interleave.sssize = info->sssize;
	interleave.dsdist = info->dsdist;
	interleave.dssize = info->dssize;

	if (ANY_DID == info->did_val)
		did_val = info->prev_did_val;

	while ((!info->port_ok
		|| ((did_val < MAX_DEVID_STATUS) && !devid_status[did_val]))
		&& !info->stop_req) {
		const struct timespec point_one_ms = {0, 100000};
		nanosleep(&point_one_ms, NULL);
		if (ANY_DID == info->did_val && !devid_status[did_val]) {
			for (int i = 0; i < MAX_DEVID_STATUS; i++) {
				int did = (did_val + i) % MAX_DEVID_STATUS;
				if (devid_status[did]) {
					did_val = did;
					break;
				}
			}
		}
	}
	if (info->stop_req)
		return dma_rc;

	info->prev_did_val = did_val;

	do {
		if (info->use_kbuf && info->wr)
			dma_rc = rio_dma_write_d(info->mp_h,
						did_val,
						ADDR_L(info->rio_addr, offset),
						info->rdma_kbuff,
						offset,
						info->acc_size,
						info->dma_trans_type,
						info->dma_sync_type,
						&interleave);

		if (info->use_kbuf && !info->wr)
			dma_rc = rio_dma_read_d(info->mp_h,
						did_val,
						ADDR_L(info->rio_addr, offset),
						info->rdma_kbuff,
						offset,
						info->acc_size,
						info->dma_sync_type,
						&interleave);

		if (!info->use_kbuf && info->wr)
			dma_rc = rio_dma_write(info->mp_h,
						did_val,
						ADDR_L(info->rio_addr, offset),
						ADDR_P(info->rdma_ptr, offset),
						info->acc_size,
						info->dma_trans_type,
						info->dma_sync_type,
						&interleave);

		if (!info->use_kbuf && !info->wr)
			dma_rc = rio_dma_read(info->mp_h,
						did_val,
						ADDR_L(info->rio_addr, offset),
						ADDR_P(info->rdma_ptr, offset),
						info->acc_size,
						info->dma_sync_type,
						&interleave);
	} while ((EINTR == -dma_rc) || (EBUSY == -dma_rc) || (EAGAIN == -dma_rc)
	|| ((info->dma_sync_type != RIO_TRANSFER_ASYNC) &&
	((EINTR == dma_rc) || (EBUSY == dma_rc) || (EAGAIN == dma_rc))));

	if ((dma_rc < 0)
	|| ((dma_rc > 0) && (info->dma_sync_type != RIO_TRANSFER_ASYNC)))
		ERR("DMA RETURN CODE: %d\n", dma_rc, strerror(dma_rc));
	return dma_rc;
}

void dma_goodput(struct worker *info)
{
	int dma_rc;
	uint8_t * volatile rx_flag;
	uint8_t * volatile tx_flag;

	if (!info->rio_addr || !info->byte_cnt || !info->acc_size) {
		ERR("FAILED: rio_addr, byte_cnd or access size is 0!\n");
		return;
	}

	if (!info->rdma_buff_size) {
		ERR("FAILED: rdma_buff_size is 0!\n");
		return;
	}

	if (dma_tx_lat == info->action) {
		if ((!info->ib_valid || (NULL == info->ib_ptr)) && info->wr) {
			ERR("FAILED: Must do IBA before measuring latency.\n");
			return;
		}
		if (info->byte_cnt != info->acc_size)  {
			ERR("WARNING: For latency, acc_size = byte count.\n");
		}
		info->acc_size = info->byte_cnt;
	}


	if (alloc_dma_tx_buffer(info))
		goto exit;

	zero_stats(info);

	rx_flag = (uint8_t *)info->ib_ptr + info->byte_cnt - 1;
	tx_flag = (uint8_t *)info->rdma_ptr +
			info->byte_cnt - 1;

	clock_gettime(CLOCK_MONOTONIC, &info->st_time);

	while (!info->stop_req) {
		uint64_t cnt;

		if (dma_tx_lat == info->action && info->wr) {
			start_iter_stats(info);
			*rx_flag = info->perf_iter_cnt + 1;
		}
		*tx_flag = info->perf_iter_cnt;

		/* Note: when info->action == dma_tx_lat, the loop below
		* will go through one iteration.
		*/
		for (cnt = 0; (cnt < info->byte_cnt) && !info->stop_req;
							cnt += info->acc_size) {
			// FAF transactions go so fast they can overwhelm the
			// small kernel transmit buffer.  Attempt the
			// transaction until the resource is not busy.
			ts_now_mark(&info->meas_ts, 5);
			dma_rc = single_dma_access(info, cnt);
			if (!info->ignore_dma_errs && (dma_rc < 0)) {
				ERR("DMA RC %d\n", dma_rc);
				goto exit;
			}

			if (RIO_TRANSFER_ASYNC == info->dma_sync_type)
			{
				dma_rc = rio_wait_async(info->mp_h, dma_rc, 0);
				if (!info->ignore_dma_errs && dma_rc) {
					ERR("DMA WAIT RC %d\n", dma_rc);
					goto exit;
				}
			}
			ts_now_mark(&info->meas_ts, 555);
			if (!info->ignore_dma_errs && dma_rc) {
				ERR("FAILED: dma transfer rc %d:%s\n",
						dma_rc, strerror(errno));
				goto exit;
			}
		}
		if (dma_tx_lat == info->action) {
			uint64_t dly = 1000000000;
			uint64_t st_dlay = dly;
			uint8_t iter_cnt_as_byte = info->perf_iter_cnt;

			if (info->wr) {
				while (dly && (*rx_flag != iter_cnt_as_byte))
					dly--;
			}

			finish_iter_stats(info);

			if (dly) {
				INFO("Response after %" PRIu64 " iterations.\n",
					st_dlay - dly);
			} else {
				ERR("FAILED: No response in %" PRIu64 " checks\n",
					st_dlay);
				break;
			}
		} else {
			info->perf_iter_cnt++;
		}
		info->perf_byte_cnt += info->byte_cnt;
		clock_gettime(CLOCK_MONOTONIC, &info->end_time);

	}
exit:
	dealloc_dma_tx_buffer(info);
}

void dma_tx_num_cmd(struct worker *info)
{
	int dma_rc;
	int trans_count;

	if (!info->rio_addr || !info->byte_cnt || !info->acc_size) {
		ERR("FAILED: rio_addr, byte_cnd or access size is 0!\n");
		return;
	}

	if (!info->rdma_buff_size) {
		ERR("FAILED: rdma_buff_size is 0!\n");
		return;
	}

	if (alloc_dma_tx_buffer(info))
		goto exit;

	zero_stats(info);

	clock_gettime(CLOCK_MONOTONIC, &info->st_time);

	for (trans_count = 0; trans_count < info->num_trans; trans_count++) {
		ts_now_mark(&info->meas_ts, 5);
		dma_rc = single_dma_access(info, 0);
		if (dma_rc < 0) {
			ERR("FAILED: dma_rc %d on trans %d!\n",
				dma_rc, trans_count);
			return;
		}
		if (RIO_TRANSFER_ASYNC == info->dma_sync_type) {
			dma_rc = rio_wait_async(info->mp_h, dma_rc, 0);
			if (dma_rc) {
				ERR("FAILED: DMA WAIT RC %d\n", dma_rc);
				goto exit;
			}
		}
		ts_now_mark(&info->meas_ts, 555);
	}

	clock_gettime(CLOCK_MONOTONIC, &info->end_time);

	while (!info->stop_req) {
		sleep(0);
	}
exit:
	dealloc_dma_tx_buffer(info);
}

void dma_rx_latency(struct worker *info)
{
	uint8_t * volatile rx_flag;
	uint8_t * volatile tx_flag;

	if (!info->rio_addr || !info->byte_cnt || !info->acc_size) {
		ERR("FAILED: rio_addr, byte_cnd or access size is 0!\n");
		return;
	}

	if (!info->rdma_buff_size) {
		ERR("FAILED: rdma_buff_size is 0!\n");
		return;
	}

	if (!info->ib_valid || (NULL == info->ib_ptr))  {
		ERR("FAILED: Must do IBA before measuring latency.\n");
		return;
	}

	if (info->byte_cnt != info->acc_size)  {
		ERR("WARNING: For latency, acc_size = byte count.\n");
	}

	if (alloc_dma_tx_buffer(info))
		goto exit;

	rx_flag = (uint8_t *)info->ib_ptr + info->byte_cnt - 1;

	tx_flag = (uint8_t *)info->rdma_ptr +
			info->byte_cnt - 1;
	info->perf_iter_cnt = 0;
	*tx_flag = info->perf_iter_cnt;
	*rx_flag = info->perf_iter_cnt + 1;

	while (!info->stop_req) {
		int dma_rc;
		uint8_t iter_cnt_as_byte;

		iter_cnt_as_byte = info->perf_iter_cnt;
		*tx_flag = iter_cnt_as_byte;
		//@sonar:off - c:EmptyCompoundStatement
		while ((*rx_flag != iter_cnt_as_byte) && !info->stop_req) {

		}
		//@sonar:on
		*rx_flag = info->perf_iter_cnt + 2;

		dma_rc = single_dma_access(info, 0);
		info->perf_iter_cnt ++;
		*tx_flag = info->perf_iter_cnt;
		if (dma_rc < 0) {
			ERR("FAILED: dma transfer rc %d:%s\n",
					-dma_rc, strerror(errno));
			goto exit;
		}
	}
exit:
	dealloc_dma_tx_buffer(info);
}

void dma_rx_goodput(struct worker *info)
{
	uint8_t * volatile rx_flag;
	uint8_t curr_rx_flag_val;
	uint8_t new_rx_flag_val;

	if (!info->byte_cnt || !info->acc_size) {
		ERR("FAILED: byte_cnt or access size is 0!\n");
		return;
	}

	if (!info->ib_valid || (NULL == info->ib_ptr))  {
		ERR("FAILED: Must do IBA before measuring RX Goodput.\n");
		return;
	}

	rx_flag = (uint8_t *)info->ib_ptr + info->byte_cnt - 1;

	info->perf_iter_cnt = 0;
	curr_rx_flag_val = info->perf_iter_cnt;
	*rx_flag = curr_rx_flag_val;

	zero_stats(info);

	clock_gettime(CLOCK_MONOTONIC, &info->st_time);

	// *rx_flag will be incremented by dma_goodput executing on another
	// node.  This loop increments goodput measured for this node
	// based on changes in *rx_flag.  This monitoring proves that
	// data is being received.
	//
	// NOTE: There is no interlock with dma_goodput, so it is possible
	// that dma_rx_goodput may miss some DMA transactions.

	while (!info->stop_req) {
		uint8_t incr;

		//@sonar:off - c:EmptyCompoundStatement
		while ((*rx_flag == curr_rx_flag_val) && !info->stop_req) {

		}
		//@sonar:on
		new_rx_flag_val = *rx_flag;
		incr = new_rx_flag_val - curr_rx_flag_val;
		curr_rx_flag_val = new_rx_flag_val;

		clock_gettime(CLOCK_MONOTONIC, &info->end_time);
		info->perf_byte_cnt += (info->byte_cnt * incr);
		info->perf_iter_cnt += incr;
	}
}

int alloc_msg_tx_rx_buffs(struct worker *info)
{
	int rc;

	rc = rio_socket_request_send_buffer(info->con_skt, &info->sock_tx_buf);
	if (rc) {
		ERR("FAILED: rio_socket_request_send_buffer rc %d:%s\n",
			rc, strerror(errno));
		return rc;
	}

	if (NULL == info->sock_rx_buf) {
		info->sock_rx_buf = malloc(RIO_MAX_MSG_SIZE);
	}

	if (NULL == info->sock_rx_buf) {
		free(info->sock_tx_buf);
		info->sock_tx_buf = NULL;
		return 1;
	}
	return 0;
}

void msg_cleanup_con_skt(struct worker *info)
{
	int rc;

	if (info->sock_rx_buf) {
		rc = rio_socket_release_receive_buffer(info->con_skt,
					info->sock_rx_buf);
		info->sock_rx_buf = NULL;
		if (rc)
			ERR("rio_socket_release_receive_buffer rc con_skt %d:%s\n",
				rc, strerror(errno));
	}

	if (info->sock_tx_buf) {
		rc = rio_socket_release_send_buffer(info->con_skt,
						info->sock_tx_buf);
		info->sock_tx_buf = NULL;
		if (rc) {
			ERR("rio_socket_release_send_buffer rc con_skt %d:%s\n",
				rc, strerror(errno));
		}
	}

	if (2 == info->con_skt_valid) {
		rc = rio_socket_close(&info->con_skt);
		info->con_skt_valid = 0;
		if (rc) {
			ERR("rio_socket_close rc con_skt %d:%s\n",
					rc, strerror(errno));
		}
	}
	info->con_skt_valid = 0;

}

void msg_cleanup_acc_skt(struct worker *info)
{
	int rc;

	if (info->acc_skt_valid) {
		rc = rio_socket_close(&info->acc_skt);
		if (rc)
			ERR("rio_socket_close acc_skt rc %d:%s\n",
					rc, strerror(errno));
		info->acc_skt_valid = 0;
	}
}

void msg_cleanup_mb(struct worker *info)
{
	int rc;

	if (info->mb_valid) {
		rc = rio_mbox_destroy_handle(&info->mb);
		if (rc)
			ERR("FAILED: rio_mbox_destroy_handle rc %d:%s\n",
					rc, strerror(errno));
		info->mb_valid = 0;
	}
}

int send_resp_msg(struct worker *info)
{
	const struct timespec ten_usec = {0, 10 * 1000};
	int rc;

	time_sleep(&ten_usec);
	errno = 0;
	rc = rio_socket_send(info->con_skt, info->sock_tx_buf, info->msg_size);
	if (rc) {
		ERR("FAILED: rio_socket_send rc %d:%s\n", rc, strerror(errno));
	}
	return rc;
}

void msg_rx_goodput(struct worker *info)
{
	int rc;

	if (info->mb_valid || info->acc_skt_valid || info->con_skt_valid) {
		ERR("FAILED: mailbox, access socket, or con socket in use.\n");
		return;
	}

	if (!info->sock_num) {
		ERR("FAILED: Socket number cannot be 0.\n");
		return;
	}

	rc = rio_mbox_create_handle(mp_h_num, 0, &info->mb);
	if (rc) {
		ERR("FAILED: rio_mbox_create_handle rc %d:%s\n",
			rc, strerror(errno));
		return;
	}

	info->mb_valid = 1;

	rc = rio_socket_socket(info->mb, &info->acc_skt);
	if (rc) {
		ERR("FAILED: rio_socket_socket acc_skt rc %d:%s\n",
			rc, strerror(errno));
		return;
	}

	info->acc_skt_valid = 1;

	rc = rio_socket_bind(info->acc_skt, info->sock_num);
	if (rc) {
		ERR("FAILED: rio_socket_bind rc %d:%s\n",
			rc, strerror(errno));
		return;
	}

	rc = rio_socket_listen(info->acc_skt);
	if (rc) {
		ERR("FAILED: rio_socket_listen rc %d:%s\n", rc, strerror(errno));
		return;
	}

	while (!info->stop_req) {
		int rc;

		if (!info->con_skt_valid) {
			rc = rio_socket_socket(info->mb, &info->con_skt);
			if (rc) {
				ERR("FAILED: rio_socket_socket con_skt rc %d:%s\n",
					rc, strerror(errno));
				goto exit;
			}
			info->con_skt_valid = 1;
		}

		rc = alloc_msg_tx_rx_buffs(info);
		if (rc)
			break;

		rc = rio_socket_accept(info->acc_skt, &info->con_skt, 1000,
				&info->stop_req);
		if (rc) {
			ERR("FAILED: rio_socket_accept rc %d:%s\n", rc,
					strerror(errno));
			break;
		}

		info->con_skt_valid = 2;

		zero_stats(info);
		clock_gettime(CLOCK_MONOTONIC, &info->st_time);

		while (!rc && !info->stop_req) {
			rc = rio_socket_receive(info->con_skt,
				&info->sock_rx_buf,
				1000,
				&info->stop_req);
			if (rc) {
				break;
			}

			info->perf_msg_cnt++;
			//@sonar:off - Collapsible "if" statements should be merged
			if ((message_rx_lat == info->action)
					|| (message_rx_oh == info->action)) {
				if (send_resp_msg(info)) {
					break;
				}
			}
			//@sonar:on
			clock_gettime(CLOCK_MONOTONIC, &info->end_time);
		}
		msg_cleanup_con_skt(info);
	}
exit:
	msg_cleanup_con_skt(info);
	msg_cleanup_acc_skt(info);
	msg_cleanup_mb(info);

}

void msg_tx_goodput(struct worker *info)
{
	const struct timespec ten_usec = {0, 10 * 1000};
	int rc;

	if (info->mb_valid || info->acc_skt_valid || info->con_skt_valid) {
		ERR("FAILED: mailbox, access socket, or con socket in use.\n");
		return;
	}

	if (!info->sock_num) {
		ERR("FAILED: Socket number cannot be 0.\n");
		return;
	}

	rc = rio_mbox_create_handle(mp_h_num, 0, &info->mb);
	if (rc) {
		ERR("FAILED: rio_mbox_create_handle rc %d:%s\n",
			rc, strerror(errno));
		return;
	}

	info->mb_valid = 1;

	rc = rio_socket_socket(info->mb, &info->con_skt);
	if (rc) {
		ERR("FAILED: rio_socket_socket rc %d:%s\n",
			rc, strerror(errno));
		return;
	}

	info->con_skt_valid = 1;

	rc = rio_socket_connect(info->con_skt, info->did_val, info->sock_num,
			&info->stop_req);
	if (rc) {
		ERR("FAILED: rio_socket_connect rc %d:%s\n",
			rc, strerror(errno));
		return;
	}

	info->con_skt_valid = 2;

	alloc_msg_tx_rx_buffs(info);
	zero_stats(info);
	clock_gettime(CLOCK_MONOTONIC, &info->st_time);

	while (!info->stop_req) {
		time_sleep(&ten_usec);
		if (message_tx_lat == info->action)
			start_iter_stats(info);

		rc = rio_socket_send(info->con_skt,
				info->sock_tx_buf, info->msg_size);

		if (rc) {
			ERR("FAILED: rio_socket_send rc %d:%s\n", rc,
					strerror(errno));
			goto exit;
		}

		if (message_tx_lat == info->action) {
			rc = 1;
			while (rc && !info->stop_req) {
				rc = rio_socket_receive(info->con_skt,
						&info->sock_rx_buf, 1000,
						&info->stop_req);
				if (rc) {
					if ((errno == ETIME)
							|| (errno == EINTR)) {
						continue;
					}
					ERR(
							"FAILED: rio_socket_receive rc %d:%s\n",
							rc, strerror(errno));
					goto exit;
				}
			}
			finish_iter_stats(info);
		}

		info->perf_msg_cnt++;
		info->perf_byte_cnt += info->msg_size;
		clock_gettime(CLOCK_MONOTONIC, &info->end_time);
	}
exit:
	msg_cleanup_con_skt(info);
	msg_cleanup_mb(info);

}


void msg_tx_overhead(struct worker *info)
{
	int rc;

	if (info->mb_valid || info->acc_skt_valid || info->con_skt_valid) {
		ERR("FAILED: mailbox, access socket, or con socket in use.\n");
		return;
	}

	if (!info->sock_num) {
		ERR("FAILED: Socket number cannot be 0.\n");
		return;
	}

	rc = rio_mbox_create_handle(mp_h_num, 0, &info->mb);
	if (rc) {
		ERR("FAILED: rio_mbox_create_handle rc %d:%s\n",
			rc, strerror(errno));
		return;
	}

	info->mb_valid = 1;

	alloc_msg_tx_rx_buffs(info);
	zero_stats(info);
	clock_gettime(CLOCK_MONOTONIC, &info->st_time);

	while (!info->stop_req) {
		rc = rio_socket_socket(info->mb, &info->con_skt);
		if (rc) {
			ERR("FAILED: rio_socket_socket rc %d:%s\n",
				rc, strerror(errno));
			return;
		}
		start_iter_stats(info);

		info->con_skt_valid = 1;
		rc = rio_socket_connect(info->con_skt, info->did_val,
				info->sock_num, &info->stop_req);
		if (rc) {
			ERR("FAILED: rio_socket_connect rc %d:%s\n",
				rc, strerror(errno));
			return;
		}
		info->con_skt_valid = 2;

		rc = rio_socket_send(info->con_skt,
				info->sock_tx_buf, info->msg_size);
		if (rc) {
			ERR("FAILED: rio_socket_send rc %d:%s\n",
				rc, strerror(errno));
			goto exit;
		}
		break;

		rc = 1;
		while (rc && !info->stop_req) {
			rc = rio_socket_receive(info->con_skt,
					&info->sock_rx_buf,
					1000,
					&info->stop_req);
			if (rc) {
				ERR(
				"FAILED: rio_socket_receive rc %d:%s\n",
					rc, strerror(errno));
				goto exit;
			}
		}

		rc = rio_socket_close(&info->con_skt);
		info->con_skt_valid = 0;
		if (rc) {
			ERR("rio_socket_close rc con_skt %d:%s\n",
					rc, strerror(errno));
		}
		finish_iter_stats(info);

		info->perf_msg_cnt++;
		info->perf_byte_cnt += info->msg_size;
		clock_gettime(CLOCK_MONOTONIC, &info->end_time);
	}
exit:
	msg_cleanup_con_skt(info);
	msg_cleanup_mb(info);

}

bool dma_alloc_ibwin(struct worker *info)
{
	uint64_t i;
	int rc;

	if (!info->ib_byte_cnt || info->ib_valid) {
		ERR("FAILED: window size of 0 or ibwin already exists\n");
		return true;
	}

	rc = rio_ibwin_map(info->mp_h, &info->ib_rio_addr,
					info->ib_byte_cnt, &info->ib_handle);
	if (rc) {
		ERR("FAILED: rio_ibwin_map rc %d:%s\n",
					rc, strerror(errno));
		return false;
	}
	if (info->ib_handle == 0) {
		ERR("FAILED: rio_ibwin_map failed silently with info->ib_handle==0!\n");
		return false;
	}


	info->ib_ptr = NULL;
	info->ib_ptr = mmap(NULL, info->ib_byte_cnt, PROT_READ | PROT_WRITE,
				MAP_SHARED, info->mp_h, info->ib_handle);
	// rc = riomp_dma_map_memory(info->mp_h, info->ib_byte_cnt,
	//				info->ib_handle, &info->ib_ptr);
	if (info->ib_ptr == MAP_FAILED)
		info->ib_ptr = NULL;

	if (NULL == info->ib_ptr) {
		rio_ibwin_free(info->mp_h, &info->ib_handle);
		ERR("FAILED: riomp_dma_map_memory rc %d:%s\n",
					rc, strerror(errno));
		return false;
	}
	if (info->ib_ptr == NULL) {
		rio_ibwin_free(info->mp_h, &info->ib_handle);
		ERR("FAILED: riomp_dma_map_memory failed silently with ib_ptr==NULL!\n");
		return false;
	}

	for (i = 0; i < info->ib_byte_cnt; i += 8) {
		uint64_t *d_ptr;

		d_ptr = (uint64_t *)((uint64_t)info->ib_ptr + i);
		*d_ptr = i + (i << 32);
	}

	info->ib_valid = 1;
	return false;
}

void dma_free_ibwin(struct worker *info)
{
	int rc;
	if (!info->ib_valid)
		return;

	if (info->ib_ptr && info->ib_valid) {
		rc = munmap(info->ib_ptr, info->ib_byte_cnt);
		info->ib_ptr = NULL;
		if (rc)
			ERR("munmap ib rc %d: %s\n",
				rc, strerror(errno));
	}

	rc = rio_ibwin_free(info->mp_h, &info->ib_handle);
	if (rc) {
		ERR("FAILED: rio_ibwin_free rc %d:%s\n",
					rc, strerror(errno));
		return;
	}

	info->ib_valid = 0;
	info->ib_rio_addr = 0;
	info->ib_byte_cnt = 0;
	info->ib_handle = 0;
}

void do_reg_scrub(struct worker *info)
{
	uint32_t oset;
	int ret = 0;
	hc_t hc = info->data8_tx;
	uint32_t val = info->data32_tx;

	zero_stats(info);

	while (!info->stop_req) {
		start_iter_stats(info);
		for (oset = 0;
			((oset < info->byte_cnt) && !ret && !info->stop_req);
				oset += 4) {
			ret = rio_maint_write(info->mp_h,
				info->did_val,
				hc,
				info->rio_addr + oset,
				4,
				val);
		}
		info->perf_byte_cnt += info->byte_cnt;
		finish_iter_stats(info);
	}

	if (ret) {
		ERR("Register write failed ERR %d %s\n", ret, strerror(ret));
	}
}

void do_maint_traffic(struct worker *info)
{
	uint32_t x;

	while (!info->stop_req)
		rio_maint_read(info->mp_h, info->did_val, info->hc, 0, 4, &x);
}


void *worker_thread(void *parm)
{
	struct worker *info = (struct worker *)parm;

	sem_post(&info->started);

	info->stat = 1;
	while (info->stat) {
		if (info->wkr_thr.cpu_req != info->wkr_thr.cpu_run)
			migrate_thread_to_cpu(&info->wkr_thr);

		//@sonar:off - c:S3458
		switch (info->action) {
		case direct_io:
			direct_io_goodput(info);
			break;
		case direct_io_tx_lat:
			direct_io_tx_latency(info);
			break;
		case direct_io_rx_lat:
			direct_io_rx_latency(info);
			break;
		case dma_tx:
		case dma_tx_lat:
			dma_goodput(info);
			break;
		case dma_tx_num:
			dma_tx_num_cmd(info);
			break;
		case dma_rx_lat:
			dma_rx_latency(info);
			break;
		case dma_rx_gp:
			dma_rx_goodput(info);
			break;
		case message_tx:
		case message_tx_lat:
			msg_tx_goodput(info);
			break;
		case message_rx:
		case message_rx_lat:
		case message_rx_oh:
			msg_rx_goodput(info);
			break;
		case message_tx_oh:
			msg_tx_overhead(info);
			break;
		case alloc_ibwin:
			dma_alloc_ibwin(info);
			break;
		case free_ibwin:
			dma_free_ibwin(info);
			break;

		case shutdown_worker:
			info->stat = 0;
			break;
		case reg_scrub:
			do_reg_scrub(info);
			break;
		case seven_test_721I:
			do_tsi721_fault_ins(info);
			break;
		case seven_test_721_recovery:
			do_tsi721_tsi721_recovery(info);
			break;
		case seven_handle_link_init:
			do_tsi721_handle_link_init(info);
			break;
		case seven_test_pw_rx:
			do_tsi721_pw_rx(info);
			break;
		case seven_test_switch_mgmt:
			tsi721_init_no_kernel_scan(info);
			do_tsi721_manage_switch(info);
			break;
		case cps_handle_721_pw:
			do_tsi721_handle_tsi_pw(info);
			break;
		case cps_handle_cps_pw:
			do_tsi721_handle_cps_pw(info);
			break;
		case cps_poll_for_pw:
			do_cps_poll_for_pw(info);
			break;
		case cps_test_switch_lock:
			do_tsi721_test_switch_lock(info);
			break;
		case maint_traffic:
			do_maint_traffic(info);
			break;
		case no_action:
		case last_action:
		default:
			break;
		}
		//@sonar:on

		if (info->stat) {
			info->stat = 2;
			sem_wait(&info->run);
			info->stat = 1;
		}
	}

	shutdown_worker_thread(info);
	pthread_exit(parm);
}


void start_worker_thread(struct worker *info, int new_mp_h, int cpu)
{
	int rc;

	init_worker_info(info, 0);

	if (new_mp_h) {
		info->mp_h = rio_mport_open(0, 0);
		if (info->mp_h < 0)
			return;
		info->mp_h_is_mine = 1;
	} else {
		info->mp_h = mp_h;
	}
	info->mp_num = mp_h_num;
	info->wkr_thr.cpu_req = cpu;

	rc = pthread_create(&info->wkr_thr.thr, NULL, worker_thread,
			(void *)info);

	if (!rc)
		sem_wait(&info->started);
}

#ifdef __cplusplus
}
#endif
