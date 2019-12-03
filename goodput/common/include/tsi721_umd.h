#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include "rio_mport_lib.h"

#define ERRMSG(format, ...) printf("ERROR: %s %4d : " format, __FILE__, __LINE__, ## __VA_ARGS__)

#define TSI721_DMA_CHNUM (8)

#define TSI721_DMAC_BASE(ch) (0x51000 + (0x1000 * ch))

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

int32_t tsi721_umd_open(struct tsi721_umd* h, uint32_t mport_id);

int32_t tsi721_umd_queue_config(struct tsi721_umd* h, uint8_t channel_num, void* queue_mem_phys, uint32_t queue_mem_size);
int32_t tsi721_umd_queue_config_multi(struct tsi721_umd* h, uint8_t channel_mask, void* phys_mem, uint32_t queue_mem_size);
int32_t tsi721_umd_start(struct tsi721_umd* h);
int32_t tsi721_umd_send(struct tsi721_umd* h, void *phys_addr, uint32_t num_bytes, uint64_t rio_addr, uint16_t dest_id);
int32_t tsi721_umd_stop(struct tsi721_umd* h);
int32_t tsi721_umd_close(struct tsi721_umd* h);
