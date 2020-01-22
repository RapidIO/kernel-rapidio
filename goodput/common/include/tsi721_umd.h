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

#ifndef __TSI721_UMD_H__
#define __TSI721_UMD_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include "rio_mport_lib.h"

#ifdef __cplusplus
extern "C" {
#endif


#define TSI721_DMA_CHNUM (8)
#define TSI721_DMAC_BASE(ch) (0x51000 + (0x1000 * ch))

// This can be arbitrarily large up to SEM_VALUE_MAX, but high numbers affect stop time
#define TSI721_UMD_MAX_CLIENT_THREADS (32768)

typedef enum {
	TSI721_UMD_STATE_UNALLOCATED,
	TSI721_UMD_STATE_UNCONFIGURED,
	TSI721_UMD_STATE_CONFIGURED,
	TSI721_UMD_STATE_READY
} TSI721_UMD_State;


struct dma_channel
{
	bool      in_use;
	void     *request_q_phys;    // physical memory address, needs to be reserved on boot
	void     *request_q;         // virtual memory address mapped to request_q_phys
	void     *completion_q_phys;
	void     *completion_q;
	uint32_t *dma_engine_regs;
	uint32_t  queue_mem_size;    // total size for all queues
	uint32_t  req_count;
	uint32_t  status_count;
	void     *reg_base;          // DMA register base address for this channel
};

struct tsi721_umd
{
	int32_t dev_fd;     // rio_mport device handle
	int32_t regs_fd;    // handle from mmap of register memory
	volatile void *all_regs;  // register memory pointer
	uint32_t regs_map_size; // size of the BAR0 register mapping
	uint8_t chan_count; // count of channels used
	uint8_t chan_mask;  // bitfield, allocated channels
	struct dma_channel chan[TSI721_DMA_CHNUM]; // channel descriptors
	sem_t              chan_sem;   // for blocking until a channel is available
	pthread_mutex_t    chan_mutex; // Mutex updating channel status
	TSI721_UMD_State state;
};

struct tsi721_umd_packet
{
	void*    phys_addr;
	uint64_t rio_addr;
	uint32_t num_bytes;
	uint32_t dest_id;
};

extern int32_t tsi721_umd_open(struct tsi721_umd* h, uint32_t mport_id);
extern int32_t tsi721_umd_queue_config(struct tsi721_umd* h, uint8_t channel_num, void* queue_mem_phys, uint32_t queue_mem_size);
extern int32_t tsi721_umd_queue_config_multi(struct tsi721_umd* h, uint8_t channel_mask, void* phys_mem, uint32_t queue_mem_size);
extern int32_t tsi721_umd_start(struct tsi721_umd* h);
extern int32_t tsi721_umd_send(struct tsi721_umd* h, void *phys_addr, uint32_t num_bytes, uint64_t rio_addr, uint16_t dest_id);
extern int32_t tsi721_umd_send_multi(struct tsi721_umd* h, struct tsi721_umd_packet* packet, uint32_t num_packets);
extern int32_t tsi721_umd_stop(struct tsi721_umd* h);
extern int32_t tsi721_umd_close(struct tsi721_umd* h);

#ifdef __cplusplus
}
#endif

#endif /* __TSI721_UMD_H__ */
