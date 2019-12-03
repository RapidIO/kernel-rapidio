#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include "tsi721_umd.h"
#include "tsi721_umd_dma.h"

#define TSI721_DESCRIPTOR_SIZE    32
#define RESPONSE_DESCRIPTOR_SIZE  64
#define DEFAULT_REQUEST_Q_SIZE    (128 * TSI721_DESCRIPTOR_SIZE)
#define DEFAULT_COMPLETION_Q_SIZE (64 * RESPONSE_DESCRIPTOR_SIZE)

static int32_t map_bar0(struct tsi721_umd* h, int32_t mport_id);

static int32_t map_bar0(struct tsi721_umd* h, int32_t mport_id)
{
	int32_t ret;
	int32_t fd;
	char bar_filename[256];
	void* ptr;
	struct stat fd_stat;

	snprintf(bar_filename,256,"/sys/class/rapidio_port/rapidio%d/device/resource0",mport_id);

	fd = open(bar_filename, O_RDWR | O_SYNC | O_CLOEXEC);

	if (fd < 0)
	{
		ERRMSG("Failed to open fd to mport %d bar 0 at filename %s, error %d %s\n",mport_id,bar_filename, errno, strerror(errno));
		return -1;
	}

	h->regs_fd = fd;

	ret = fstat(fd, &fd_stat);
	if (ret < 0)
	{
		ERRMSG("Failed to fstat %s: %d %s\n",bar_filename,ret,strerror(ret));
		return -1;
	}


	ptr = mmap(NULL, fd_stat.st_size,  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (ptr == MAP_FAILED)
	{
		ERRMSG("Failed to mmap %d bytes of bar0 space, error %d %s\n",(int)fd_stat.st_size,errno,strerror(errno));
		return -1;
	}

	printf("Mapped bar0 to %p\n",ptr);
	h->all_regs = ptr;

	return 0;
}


int32_t tsi721_umd_open(struct tsi721_umd* h, uint32_t mport_id)
{
	assert(h->state == TSI721_UMD_STATE_UNALLOCATED);
	
	memset(h, 0, sizeof(*h));

	// Get device handle
	h->dev_fd = rio_mport_open(mport_id, 0);
	if (h->dev_fd <= 0)
	{
		ERRMSG("Fail to open mport dev, err %d %s\n",errno,strerror(errno));
		return -1;
	}
	
	// Get pointer to BAR0 configuration space
	if (map_bar0(h,mport_id) < 0)
	{
		ERRMSG("Failed to map registers to process mem\n");
		return -1;
	}

	h->state = TSI721_UMD_STATE_UNCONFIGURED;

	return 0;
}

int32_t tsi721_umd_queue_config(struct tsi721_umd* h, uint8_t channel_num, void* queue_mem_phys, uint32_t queue_mem_size)
{
	int32_t ret;
	uint32_t page_size = sysconf(_SC_PAGE_SIZE);
	struct dma_channel* chan;

	if (!h)
		return -1;
	
	if (channel_num >= TSI721_DMA_CHNUM)
	{
		ERRMSG("Illegal channel number %d > %d\n",channel_num,TSI721_DMA_CHNUM-1);
		return -1;
	}

	if (h->chan_mask & (1<<channel_num))
	{
		ERRMSG("Channel %d already configured\n", channel_num);
		return -1;
	}
	
	chan = &h->chan[channel_num];

	// TBD: use user-config sizes for queues instead of a fixed default
	if (queue_mem_size < (DEFAULT_REQUEST_Q_SIZE + DEFAULT_COMPLETION_Q_SIZE))
	{
		ERRMSG("Queue memory size must be at least %d\n",DEFAULT_REQUEST_Q_SIZE + DEFAULT_COMPLETION_Q_SIZE);
		return -1;
	}
	else if	((queue_mem_size & (page_size-1)) != 0)
	{
		ERRMSG("Invalid queue memory size %d, must be a multiple of %d\n",queue_mem_size,page_size);
		return -1;
	}

	if (((uintptr_t)queue_mem_phys & (page_size-1)) != 0)
	{
		ERRMSG("Invalid queue memory address %p, must be page aligned to %x\n",queue_mem_phys,page_size);
		return -1;
	}
	
	// cdev driver needs to map physical memory to dev_fd
	uint64_t handle = (uintptr_t)queue_mem_phys;
	ret = rio_dbuf_alloc(h->dev_fd, queue_mem_size, &handle);
	if (ret < 0)
	{
		ERRMSG("Fail rio_dbuf_alloc sz %d ptr %p : err %d %s\n",queue_mem_size,queue_mem_phys,ret,strerror(ret));
	}
	chan->request_q_phys = queue_mem_phys;
	chan->request_q = mmap(NULL, queue_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, h->dev_fd, handle);

	if (chan->request_q == MAP_FAILED)
	{
		ERRMSG("Failed to mmap the queue memory at address %p size %d: err %d %s\n",queue_mem_phys,queue_mem_size,errno,strerror(errno));
		return -1;
	}

	chan->completion_q = (void*)((uintptr_t)chan->request_q + DEFAULT_REQUEST_Q_SIZE);
	chan->completion_q_phys = (void*)((uintptr_t)chan->request_q_phys + DEFAULT_REQUEST_Q_SIZE);
	
	chan->reg_base = (void*)((uintptr_t)h->all_regs + TSI721_DMAC_BASE(channel_num));
	
	// Config the descriptor pointer registers, queue size
	TSI721_WR32(TSI721_DMACXDPTRH, channel_num, ((uintptr_t)chan->request_q_phys) >> 32);
	TSI721_WR32(TSI721_DMACXDPTRL, channel_num, ((uintptr_t)chan->request_q_phys) & 0xFFFFFFFF);
	TSI721_WR32(TSI721_DMACXDSBH,  channel_num, ((uintptr_t)chan->completion_q_phys) >> 32);
	TSI721_WR32(TSI721_DMACXDSBL,  channel_num, ((uintptr_t)chan->completion_q_phys) & 0xFFFFFFFF);
	TSI721_WR32(TSI721_DMACXDSSZ,    channel_num, 2); // translates to 64 entries

	// Initialize hardware queue counters
	TSI721_WR32(TSI721_DMACXCTL,   channel_num, TSI721_DMACXCTL_INIT);

	chan->in_use = false;

	h->chan_mask |= (1<<channel_num);

	return 0;
}

int32_t tsi721_umd_queue_config_multi(struct tsi721_umd* h, uint8_t channel_mask, void* phys_mem, uint32_t phys_mem_size)
{
	int32_t i, ret, fail=0;
	uintptr_t ptr = (uintptr_t)phys_mem;
	const uint32_t queue_size = DEFAULT_REQUEST_Q_SIZE + DEFAULT_COMPLETION_Q_SIZE;

	assert(h->state == TSI721_UMD_STATE_UNCONFIGURED);

	if (phys_mem_size < queue_size * TSI721_DMA_CHNUM)
	{
		ERRMSG("Allocated physical (%d) memory size is insufficent",phys_mem_size);
		return -1;
	}

	h->chan_count = 0;
	for (i=0; i<TSI721_DMA_CHNUM; i++)
	{
		if ((1<<i) & channel_mask)
		{
			ret = tsi721_umd_queue_config(h,i,(void*)ptr,queue_size);
			if (ret < 0)
			{
				ERRMSG("Failed to configure queue %d\n",i);
				fail = 1;
			}
			else
			{
				printf("Success configure queue %d at address %lx size %d\n",i,ptr,queue_size);
				h->chan_count++;
			}
			ptr += queue_size;
		}
	}

	if (fail)
		return -1;

	h->state = TSI721_UMD_STATE_CONFIGURED;
	return 0;
}

int32_t tsi721_umd_start(struct tsi721_umd* h)
{
	int32_t ret;

	// Allocate channel dispatch mutex
	ret = pthread_mutex_init(&h->chan_mutex, NULL);
	if (ret < 0)
	{
		ERRMSG("Error initializing mutex, err %d %s\n",ret,strerror(ret));
		return -1;
	}

	// Initial semaphore count is all channels free
	ret = sem_init(&h->chan_sem, 0, h->chan_count);
	if (ret < 0)
	{
		ERRMSG("Error initializing channel semaphore. err %d %s\n",ret,strerror(ret));
		return -1;
	}

	h->state = TSI721_UMD_STATE_READY;

	return 0;
}

int32_t tsi721_umd_send(struct tsi721_umd* h, void* phys_addr, uint32_t num_bytes, uint64_t rio_addr, uint16_t dest_id)
{
	int32_t ret, i;
	int8_t chan = -1;
	tsi721_dma_desc descriptor; // TBD - check if need to handle splitting to multiple descriptor
	
	// Form descriptor in local mem
	tsi721_umd_create_dma_descriptor(
		&descriptor, // dest pointer
		dest_id,     // destination
		num_bytes,   // byte count
		rio_addr,    // dest address [63:0]
		0,           // dest addreess [65:64]
		phys_addr);  // source address (physical)
	
	
	// Wait for a channel to be available
	do {
		ret = sem_wait(&h->chan_sem);
	} while (ret != 0); // avoid spurious exit on signal

	// Mark a free channel as in use
	pthread_mutex_lock(&h->chan_mutex);
	for (i=0; i<TSI721_DMA_CHNUM; i++)
	{
		if ( (h->chan_mask & (1 << i)) && (!h->chan[i].in_use) )
		{
			chan = i;
			h->chan[i].in_use = true;
			break;
		}
	}
	pthread_mutex_unlock(&h->chan_mutex);

	assert(chan != -1); // this should not be possible (got sem but no channels free)

	// Copy descriptor to channel queue
	memcpy(h->chan[chan].request_q,&descriptor,sizeof(descriptor));
	
	// Start transfer.  Increment the req count and set it in the DMA descriptor write count register
	
	printf("Before send: channel %d write count %d read count %d\n",chan, TSI721_RD32(TSI721_DMACXDWRCNT, chan), TSI721_RD32(TSI721_DMACXDRDCNT, chan));
	printf("Before send: DMAChannel %d status %x\n",chan,TSI721_RD32(TSI721_DMACXSTS, chan));
	h->chan[chan].req_count++;
	TSI721_WR32(TSI721_DMACXDWRCNT, chan, h->chan[chan].req_count);
	printf("After send: channel %d write count %d read count %d\n",chan, TSI721_RD32(TSI721_DMACXDWRCNT, chan), TSI721_RD32(TSI721_DMACXDRDCNT, chan));

	// For debug: poll the local status register
	uint32_t status = TSI721_RD32(TSI721_DMACXSTS, chan);
	uint32_t prev_status = 0;
	while(status & (1<<21))
	{
		if (status != prev_status)
		{
			printf("DMAChannel %d status %x\n",chan,status);
			prev_status = status;
		}
		usleep(1000);
		status = TSI721_RD32(TSI721_DMACXSTS, chan);
	}

	if ((status>>16) & 0xF)
	{
		printf("DMAChannel %d completed with errors, status %x\n",chan,status);
	}
	
	// Wait for transfer completion
	// use maintenance transactions to poll the destination for received msg
	
	// Cleanup : mark DMA as idle and increment the free engine semaphore
	h->chan[chan].in_use = false;
	sem_post(&h->chan_sem); // mark a dma engine is free for use

	return 0;
}

int32_t tsi721_umd_stop(struct tsi721_umd* h)
{
	if (0) {
		free(h);
	}
	return -1;
}

int32_t tsi721_close(struct tsi721_umd* h)
{
	if (0) {
		free(h);
	}
	return -1;
}
