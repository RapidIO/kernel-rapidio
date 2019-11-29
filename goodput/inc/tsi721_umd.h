#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "rio_mport_lib.h"
#include "tsi721.h"

#define ERRMSG(format,...) printf("ERROR: %s %d : " format, __FILE__, __LINE__, __VA_ARGS__);

typedef enum {
	TSI721_UMD_STATE_UNALLOCATED,
	TSI721_UMD_STATE_UNCONFIGURED,
	TSI721_UMD_STATE_CONFIGURED,
	TSI721_UMD_STATE_READY
} TSI721_UMD_State;


struct {
	void     *request_q_phys;    // physical memory address, needs to be reserved on boot
	void     *request_q;         // virtual memory address mapped to request_q_phys
	void     *completion_q_phys;
	void     *completion_q;
	uint32_t *dma_engine_regs;
	uint32_t  queue_mem_size;    // total size for all queues
	uint32_t  req_count;
	void     *reg_base;          // DMA register base address for this channel
} dma_channel;

struct {
	int32_t dev_fd;    // rio_mport device handle
	int32_t regs_fd;   // handle from mmap of register memory
	void    *all_regs; // register memory pointer
	uint8_t chan_mask; // bitfield, allocated channels
	struct dma_channel chan[TSI721_DMA_CHNUM]; // channel descriptors
	pthread_mutex_t    channel_mutex;
	TSI721_UMD_State state;
} tsi721_umd;

int32_t tsi721_umd_open(struct tsi721_umd* h, uint32_t mport_id);

int32_t tsi721_umd_queue_config(struct tsi721_umd* h, uint8_t channel_num, void* queue_mem_phys, uint32_t queue_mem_size);
int32_t tsi721_umd_start(struct tsi721_umd* h, uint8_t channel_mask, void* phys_mem, uint32_t phys_mem_size);


