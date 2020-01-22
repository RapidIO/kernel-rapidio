/*
 Copyright (c) 2019, Renesas Electronics Corporation.
 All rights reserved.
 *
 * This software is available to you under a choice of one of two licenses.
 * You may choose to be licensed under the terms of the GNU General Public
 * License(GPL) Version 2, or the BSD-3 Clause license below:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "tsi721_umd.h"
#include "tsi721_umd_dma.h"

#define TSI721_DESCRIPTOR_SIZE    32
#define RESPONSE_DESCRIPTOR_SIZE  64
#define REQUEST_Q_COUNT           128
#define REQUEST_Q_MASK            (REQUEST_Q_COUNT-1)
#define COMPLETION_Q_COUNT        64
#define COMPLETION_Q_MASK         (COMPLETION_Q_COUNT-1)
#define DEFAULT_REQUEST_Q_SIZE    (REQUEST_Q_COUNT * TSI721_DESCRIPTOR_SIZE)
#define DEFAULT_COMPLETION_Q_SIZE (COMPLETION_Q_COUNT * RESPONSE_DESCRIPTOR_SIZE)

#define min(x,y) ((x) < (y) ? (x) : (y))

static void* request_q_entry(struct dma_channel* chan, uint32_t id, const bool phys)
{
    void* ptr;

    if (phys)
    {
        ptr = chan->request_q_phys;
    }
    else
    {
        ptr = chan->request_q;
    }

    return (void*)((uintptr_t)ptr + (id & REQUEST_Q_MASK) * TSI721_DESCRIPTOR_SIZE);
}

static void* completion_q_entry(struct dma_channel* chan, uint32_t id, const bool phys)
{
    void* ptr;

    if (phys)
    {
        ptr = chan->completion_q_phys;
    }
    else
    {
        ptr = chan->completion_q;
    }

    return (void*)((uintptr_t)ptr + (id & COMPLETION_Q_MASK) * RESPONSE_DESCRIPTOR_SIZE);
}

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
        return -ENODEV;
    }

    h->regs_fd = fd;

    ret = fstat(fd, &fd_stat);
    if (ret < 0)
    {
        return -ENODEV;
    }


    h->regs_map_size = fd_stat.st_size;
    ptr = mmap(NULL, h->regs_map_size,  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (ptr == MAP_FAILED)
    {
        return -ENOMEM;
    }

    h->all_regs = ptr;

    return 0;
}


/* Opens a handle to the user-mode driver
 *
 * h        - pointer to an allocated and uninitialized tsi721_umd handle
 * mport_id - mport ID for the device to be opened
 *
 * Returns 0 if successful, negative on error
 */
int32_t tsi721_umd_open(struct tsi721_umd* h, uint32_t mport_id)
{
    int32_t ret;

    memset(h, 0, sizeof(*h));

    // Get device handle
    h->dev_fd = rio_mport_open(mport_id, 0);
    if (h->dev_fd <= 0)
    {
        return -ENODEV;
    }
    
    // Get pointer to BAR0 configuration space
    ret = map_bar0(h,mport_id);
    if (ret < 0)
    {
        return -123;//ret;
    }

    h->state = TSI721_UMD_STATE_UNCONFIGURED;

    return 0;
}

/* Configures one DMA channel for the user-mode driver
 *
 * h              - handle to the user-mode driver
 * channel_num    - TSI721 DMA channel to configure
 * queue_mem_phys - physical memory address to use for the request and completion queues
 * queue_mem_size - size of the allocated physical memory
 *
 * Returns 0 if successful, negative on error
 */
int32_t tsi721_umd_queue_config(struct tsi721_umd* h, uint8_t channel_num, void* queue_mem_phys, uint32_t queue_mem_size)
{
    int32_t ret;
    struct dma_channel* chan;

    if (!h)
    {
        return -EINVAL;
    }
    
    if (channel_num >= TSI721_DMA_CHNUM)
    {
        return -EINVAL;
    }

    if (h->chan_mask & (1<<channel_num))
    {
        return -EINVAL;
    }
    
    chan = &h->chan[channel_num];

    if (queue_mem_size < (DEFAULT_REQUEST_Q_SIZE + DEFAULT_COMPLETION_Q_SIZE))
    {
        return -EINVAL;
    }
    
    // cdev driver needs to map physical memory to dev_fd
    uint64_t handle = (uintptr_t)queue_mem_phys;
    ret = rio_dbuf_alloc(h->dev_fd, queue_mem_size, &handle);
    if (ret < 0)
    {
        return -ENOMEM;
    }

    chan->request_q_phys = queue_mem_phys;
    chan->request_q = mmap(NULL, queue_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, h->dev_fd, handle);

    if (chan->request_q == MAP_FAILED)
    {
        return -ENOMEM;
    }

    chan->completion_q = (void*)((uintptr_t)chan->request_q + DEFAULT_REQUEST_Q_SIZE);
    chan->completion_q_phys = (void*)((uintptr_t)chan->request_q_phys + DEFAULT_REQUEST_Q_SIZE);
    
    chan->reg_base = (void*)((uintptr_t)h->all_regs + TSI721_DMAC_BASE(channel_num));
    
    // Config the descriptor pointer registers, queue size
    TSI721_WR32(TSI721_DMACXDPTRH(channel_num), ((uintptr_t)chan->request_q_phys) >> 32);
    TSI721_WR32(TSI721_DMACXDPTRL(channel_num), ((uintptr_t)chan->request_q_phys) & 0xFFFFFFFF);
    TSI721_WR32(TSI721_DMACXDSBH(channel_num),  ((uintptr_t)chan->completion_q_phys) >> 32);
    TSI721_WR32(TSI721_DMACXDSBL(channel_num),  ((uintptr_t)chan->completion_q_phys) & 0xFFFFFFFF);
    TSI721_WR32(TSI721_DMACXDSSZ(channel_num),  2); // translates to 64 entries

    // Initialize hardware queue counters
    TSI721_WR32(TSI721_DMACXCTL(channel_num), TSI721_DMACXCTL_INIT);

    chan->in_use = false;

    h->chan_mask |= (1<<channel_num);

    return 0;
}

/* Configures multiple TSI721 DMA channels for use by the user-mode driver
 *
 * h             - handle to the user-mode driver
 * channel_mask  - bitmask of the channels to be enabled, bit N being 1 indicates the channel is to be configured
 * phys_mem      - physical address to be used for the channels' request/completion queues
 * phys_mem_size - total size of the allocated physical memory
 *
 * Returns 0 if successful, negative on error
 */
int32_t tsi721_umd_queue_config_multi(struct tsi721_umd* h, uint8_t channel_mask, void* phys_mem, uint32_t phys_mem_size)
{
    int32_t i, ret;
    uintptr_t ptr = (uintptr_t)phys_mem;
    const uint32_t queue_size = DEFAULT_REQUEST_Q_SIZE + DEFAULT_COMPLETION_Q_SIZE;

    if (h->state != TSI721_UMD_STATE_UNCONFIGURED)
    {
        return -EPERM;
    }

    if (phys_mem_size < queue_size * TSI721_DMA_CHNUM)
    {
        return -EINVAL;
    }

    h->chan_count = 0;
    for (i=0; i<TSI721_DMA_CHNUM; i++)
    {
        if ((1<<i) & channel_mask)
        {
            ret = tsi721_umd_queue_config(h,i,(void*)ptr,queue_size);
            if (ret < 0)
            {
                return ret;
            }
            else
            {
                h->chan_count++;
            }
            
            ptr += queue_size;
        }
    }

    h->state = TSI721_UMD_STATE_CONFIGURED;

    return 0;
}

/* Start the user-mode driver.  This allocates the synchronization objects and sets the state to ready to receive
 * send commands.
 *
 * h - handle to the user-mode driver
 *
 * Returns 0 if successful, negative on error
 */
int32_t tsi721_umd_start(struct tsi721_umd* h)
{
    int32_t ret, i;

    if (h->state != TSI721_UMD_STATE_CONFIGURED)
        return -EINVAL;

    if (h->chan_count == 0)
        return -EINVAL;

    // Allocate channel dispatch mutex
    ret = pthread_mutex_init(&h->chan_mutex, NULL);
    if (ret < 0)
    {
        return ret;
    }

    // Initial semaphore count is all channels free
    ret = sem_init(&h->chan_sem, 0, h->chan_count);
    if (ret < 0)
    {
        return ret;
    }

    // Pre-initialize all descriptors
    for (i=0; i<TSI721_DMA_CHNUM; i++)
    {
        if (h->chan_mask & (1<<i))
        {
            tsi721_dma_desc* descriptor = request_q_entry(&h->chan[i],0,false);
            tsi721_umd_init_dma_descriptors(descriptor, REQUEST_Q_COUNT);
        }
    }

    h->state = TSI721_UMD_STATE_READY;

    return 0;
}

/* Perform a DMA transfer using the user-mode driver.  This sends on the first available idle DMA channel.
 * If no channels are available, this will block until a channel is available then perform the send.
 * This will block until the send completes.
 * Send will be performed as an NWRITE transaction.
 *
 * h         - handle to the user-mode driver
 * phys_addr - physical source address
 * num_bytes - size in bytes of the message to be sent
 * rio_addr  - destination RIO address
 * dest_id   - destination RIO ID
 *
 * Returns 0 if successful, negative on error
 */
int32_t tsi721_umd_send(struct tsi721_umd* h, void* phys_addr, uint32_t num_bytes, uint64_t rio_addr, uint16_t dest_id)
{
    int32_t ret, i;
    int8_t chan = -1;

    if (h->state != TSI721_UMD_STATE_READY)
        return -EINVAL;

    if (h->chan_mask == 0)
        return -EPERM;
    
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

    if (chan == -1)
    {
        return -EPERM; // this should only occur on stop
    }

    // Update descriptor in channel queue
    void* request_q_descriptor = request_q_entry(&h->chan[chan],h->chan[chan].req_count,false);
    tsi721_umd_update_dma_descriptor(request_q_descriptor, rio_addr, 0, dest_id, num_bytes, phys_addr);
    
    // Start transfer.  Increment the req count and set it in the DMA descriptor write count register
    h->chan[chan].req_count++;
    TSI721_WR32(TSI721_DMACXDWRCNT(chan), h->chan[chan].req_count);

    // Poll status to wait for completion
    uint32_t status = TSI721_RD32(TSI721_DMACXSTS(chan));
    while(status & TSI721_DMACXSTS_RUN)
    {
        usleep(1);
        status = TSI721_RD32(TSI721_DMACXSTS(chan));
    }
    
    if (status & TSI721_DMACXSTS_CS)
    {
        // DMA completed with error
        return -EIO;
    }

    // If this was the end of the descriptor array, move pointer back to start
    if ((h->chan[chan].req_count & REQUEST_Q_MASK) == 0)
    {
        TSI721_WR32(TSI721_DMACXDPTRH(chan), ((uintptr_t)h->chan[chan].request_q_phys) >> 32);
        TSI721_WR32(TSI721_DMACXDPTRL(chan), ((uintptr_t)h->chan[chan].request_q_phys) & 0xFFFFFFFF);
    }
    
    // Clear status entry and increment the status fifo read pointer
    uint64_t* completion_queue_entry = completion_q_entry(&h->chan[chan],h->chan[chan].status_count,false);
    for (i=0; i<8; i++)
        completion_queue_entry[i] = 0;

    h->chan[chan].status_count = (h->chan[chan].status_count + 1) & TSI721_DMACXDSRP_RD_PTR;
    TSI721_WR32(TSI721_DMACXDSRP(chan), h->chan[chan].status_count);
    
    // Cleanup : mark DMA as idle and increment the free engine semaphore
    h->chan[chan].in_use = false;

    // Give sem to unblock any waiting threads
    // Do not give the sem if we've stopped since it may already be destroyed
    if (h->chan_mask != 0)
        sem_post(&h->chan_sem);

    sched_yield();

    return 0;
}

/* Perform multiple DMA transfer using the user-mode driver.  This sends on the first available idle DMA channel.
 * If no channels are available, this will block until a channel is available then perform the send.
 * This will block until all sends are complete.
 * Send will be performed as an NWRITE transaction.
 *
 * h          - handle to the user-mode driver
 * packet     - pointer to the array of packets to send
 * num_packet - number of packets to send
 *
 * Returns 0 if successful, negative on error
 */
int32_t tsi721_umd_send_multi(struct tsi721_umd* h, struct tsi721_umd_packet *packet, uint32_t num_packet)
{
    int32_t ret, i, j;
    int8_t chan = -1;
    uint32_t packets_sent = 0, packets_remain = num_packet;

    if (h->state != TSI721_UMD_STATE_READY)
        return -EINVAL;

    if (h->chan_mask == 0)
        return -EPERM;

    if (num_packet == 0)
        return 0;

    if (packet == NULL)
        return -EINVAL;

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

    if (chan == -1)
    {
        return -EPERM; // this should only occur on stop
    }

    while (packets_sent < num_packet)
    {
        uint32_t sent_this_iter = min(packets_remain, min(REQUEST_Q_COUNT,COMPLETION_Q_COUNT));
        
        // Set descriptor write pointer to start of array
        TSI721_WR32(TSI721_DMACXDPTRH(chan), ((uintptr_t)h->chan[chan].request_q_phys) >> 32);
        TSI721_WR32(TSI721_DMACXDPTRL(chan), ((uintptr_t)h->chan[chan].request_q_phys) & 0xFFFFFFFF);

        // Update descriptors with values from the packets. Always start from index 0
        tsi721_dma_desc* request_q_descriptor = request_q_entry(&h->chan[chan], 0, false);
        for (i=0; i<(int32_t)sent_this_iter; i++)
        {
            struct tsi721_umd_packet* p = &packet[packets_sent + i];
            tsi721_umd_update_dma_descriptor(request_q_descriptor + i, p->rio_addr, 0, p->dest_id, p->num_bytes, p->phys_addr);
        }
        
        // Start transfer.  Increment the req count and set it in the DMA descriptor write count register
        h->chan[chan].req_count += sent_this_iter;
        TSI721_WR32(TSI721_DMACXDWRCNT(chan), h->chan[chan].req_count);

        // Poll status to wait for completion
        uint32_t status = TSI721_RD32(TSI721_DMACXSTS(chan));
        while(status & TSI721_DMACXSTS_RUN)
        {
            usleep(1);
            status = TSI721_RD32(TSI721_DMACXSTS(chan));
        }
        
        if (status & TSI721_DMACXSTS_CS)
        {
            // DMA completed with error
            return -EIO;
        }

        // Clear status entries and increment the status fifo read pointer
        i = 0;
        do
        {
            uint64_t* completion_queue_entry = completion_q_entry(&h->chan[chan],h->chan[chan].status_count+i,false);
            for (j=0; j<8; j++)
                completion_queue_entry[j] = 0;

            i+= 8;
        } while (i < (int32_t)sent_this_iter);

        TSI721_WR32(TSI721_DMACXDSRP(chan), h->chan[chan].status_count);

        packets_remain -= sent_this_iter;
        packets_sent   += sent_this_iter;
    }
    
    // Cleanup : mark DMA as idle and increment the free engine semaphore
    h->chan[chan].in_use = false;

    // Give sem to unblock any waiting threads
    // Do not give the sem if we've stopped since it may already be destroyed
    if (h->chan_mask != 0)
        sem_post(&h->chan_sem);

    sched_yield();

    return 0;
}

/* Stop the user-mode driver.  This waits for all DMA queues to go idle then frees all synchronization objects.
 *
 * h - handle to the user-mode driver
 *
 * Returns 0 if successful, negative on error
 */
int32_t tsi721_umd_stop(struct tsi721_umd* h)
{
    int32_t ret, i, mask;

    if (h->state != TSI721_UMD_STATE_READY)
    {
        return -EPERM;
    }

    // Clear all available channels to avoid further DMAs
    pthread_mutex_lock(&h->chan_mutex);
    mask = h->chan_mask;
    h->chan_mask = 0;
    pthread_mutex_unlock(&h->chan_mutex);

    // Give the semaphore to unblock any waiting threads
    for (i=0; i<TSI721_UMD_MAX_CLIENT_THREADS; i++)
    {
        sem_post(&h->chan_sem);
    }

    // Wait for any running channels to finish
    for (i=0; i<TSI721_DMA_CHNUM; i++)
    {
        if ((1<<i) & mask)
        {
            // Wait for this channel to go idle
            while (h->chan[i].in_use)
                usleep(100);

            // Unmap this channel's queues
            if (munmap(h->chan[i].request_q, DEFAULT_REQUEST_Q_SIZE + DEFAULT_COMPLETION_Q_SIZE) < 0)
            {
                return errno;
            }

            h->chan[i].request_q         = NULL;
            h->chan[i].request_q_phys    = NULL;
            h->chan[i].completion_q      = NULL;
            h->chan[i].completion_q_phys = NULL;
        }
    }

    ret = pthread_mutex_destroy(&h->chan_mutex);
    if (ret < 0)
    {
        return ret;
    }

    ret = sem_destroy(&h->chan_sem);
    if (ret < 0)
    {
        return ret;
    }

    h->state = TSI721_UMD_STATE_CONFIGURED;

    return 0;
}

/* Close user-mode driver.  This removes the mappings to the TSI721 registers and closes the handle
 * to the RIO device.
 *
 * h - handle to the user-mode driver
 *
 * Returns 0 if successful, negative on error
 */
int32_t tsi721_umd_close(struct tsi721_umd* h)
{
    if (h->state != TSI721_UMD_STATE_CONFIGURED && h->state != TSI721_UMD_STATE_UNCONFIGURED)
    {
        return -EPERM;
    }

    if (h->state == TSI721_UMD_STATE_CONFIGURED)
    {
        if (h->all_regs == NULL)
        {
            return -EPERM;
        }

        munmap((void*)h->all_regs, h->regs_map_size);
        h->all_regs = NULL;
    
        close(h->regs_fd);
    }

    close(h->dev_fd);

    h->state = TSI721_UMD_STATE_UNALLOCATED;

    return 0;
}
