/*
****************************************************************************
Copyright (c) 2019, Renesas Electronics Corporation.
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

#include "tsi721_umd.h"
#include "libtime_utils.h"
#include "umd_worker.h"
#include "pw_handling.h"
#include "goodput.h"
#include "Tsi721.h"
#include "CPS1848.h"
#include "rio_misc.h"
#include "did.h"


#ifdef __cplusplus
extern "C" {
#endif

#define PATTERN_SIZE  8
#define UDM_QUEUE_SIZE  (8 * 8192)
#define INIT_CRC32 0xFF00FF00

#define ADDR_L(x,y) ((uint64_t)((uint64_t)x + (uint64_t)y))
#define ADDR_P(x,y) ((void *)((uint64_t)x + (uint64_t)y))

static const uint8_t PREFIX_PATTERN[PATTERN_SIZE]={0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF1};
static const uint8_t STATUS_PATTERN[PATTERN_SIZE]={0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

struct data_prefix
{
    uint32_t trans_nth_head; /*current transaction index*/
    uint32_t CRC32;
    uint64_t xfer_offset; /* Transfer occurs at xfer_offset bytes from the start of the target buffer */
    uint64_t xfer_size; /* Transfer consists of xfer_size*/
    uint32_t trans_nth_tail; /*current transaction index*/
    uint32_t padding;
    uint8_t payload[]; /* Start of data buffer */
};

struct data_status
{
    int trans_nth_head; /*current transaction index*/
    int xfer_check; /* 0 – waiting for transfer check, 1 – passed 2 – failed */
    uint32_t CRC32; /*CRC for the buffer */
    int trans_nth_tail; /*current transaction index*/
};

static uint64_t queue_mem_h;

static int umd_alloc_queue_mem(struct tsi721_umd *umd_engine_p)
{
    int ret = 0;

    queue_mem_h = RIO_MAP_ANY_ADDR;

    ret = rio_dbuf_alloc(umd_engine_p->dev_fd, UDM_QUEUE_SIZE, &queue_mem_h);

    if (ret)
    {
        ERR("FAILED: riomp_dma_dbuf_alloc queue buffer rc %d: %d %s\n",
                        ret, errno, strerror(errno));
    }
    else
    {
        INFO("INFO: DMA queue handle 0x%lx\n", &queue_mem_h);
    }

    return ret;
}

static int umd_free_queue_mem(struct tsi721_umd *umd_engine_p)
{
    if (queue_mem_h)
    {
        rio_dbuf_free(umd_engine_p->dev_fd, &queue_mem_h);
        queue_mem_h = 0;
    }

    return 0;
}

static uint32_t crc32_table[256];

static uint32_t crc32(uint32_t crc, uint8_t *data_p, uint32_t sz)  
{  
    uint32_t i; 
    for (i = 0; i < sz; i++)
    {  
        crc = crc32_table[(crc ^ data_p[i]) & 0xff] ^ (crc >> 8);  
    }  
    return crc ;  
}

static void umd_init_xfer_data(data_prefix *prefix, uint32_t offset, uint32_t sz, struct worker *info)
{
    uint32_t remainder;
    uint32_t idx;

    for (idx = 0; idx < sz; idx++)
    {
        remainder = idx % 8; 
        prefix->payload[offset + idx] = info->data8[remainder];
        // If randomized data has been requested, choose new data...
        if (!remainder && idx && info->seed)
        {
             for (int i = 0; i < 8; i++)
             {
                 info->data8[i] = (uint8_t)(rand() % 256);
             }
        }
    }
}

static void umd_init_crc_table(void)  
{  
    uint32_t c;  
    uint32_t i, j;  
      
    for (i = 0; i < 256; i++)
    {  
        c = (uint32_t)i;  
        for (j = 0; j < 8; j++) 
        {  
            if (c & 1)
            {
                c = 0xedb88320L ^ (c >> 1);  
            }
            else
            {
                c = c >> 1;
            }
        }  
        crc32_table[i] = c;  
    }  
}  


int umd_init_engine_handle(struct tsi721_umd *engine_p)
{
    memset(engine_p, 0x0, sizeof(struct tsi721_umd));

    umd_init_crc_table();

    return 0;
}

int umd_config(struct tsi721_umd *engine_p, uint8_t chan_mask)
{
    if(!umd_alloc_queue_mem(engine_p))
    {
       INFO("Queue mem is allocated.\n");

       if(!tsi721_umd_queue_config_multi(engine_p, chan_mask, (void *)queue_mem_h, UDM_QUEUE_SIZE))
       {
           INFO("UMD queue is configured. Channel mask 0x%x\n", chan_mask);  
           return 0;
       }
       else
       {
           ERR("FAILED: Engine configure error. Channel mask 0x%x\n", chan_mask);
           umd_free_queue_mem(engine_p);
       }
    }
    else
    {
        ERR("Allocate queue memory error.\n");
    }

    return -1;
}

int umd_close(struct tsi721_umd *engine_p)
{
    umd_free_queue_mem(engine_p);
    
    if(tsi721_umd_close(engine_p) == 0)
    {
        return 0;
    }
    else
    {
        ERR("FAILED: UMD close returns failure\n");
    }
    
    return -1;
}

static int umd_dma_num_writer(struct worker *worker_info, uint32_t iter)
{
    data_status *status = NULL;
    data_prefix *prefix = NULL;
    int32_t ret = -1, rc;
    uint64_t prefix_phys, prefix_rio;
    uint64_t payload_phys, payload_rio;
    uint64_t payload_size = worker_info->rdma_buff_size - sizeof(data_prefix);
    uint64_t xfer_offset = 0;
    uint64_t xfer_size;

    if (worker_info->stop_req)
    {
       goto exit;
    }

    // Clear status area to 0, ready to receive status from reader
    status = (data_status*)worker_info->ib_ptr;
    memset(status, 0, sizeof(data_status));

    // Initialize pointers to virtual (mapped), physical and reader RapidIO
    // memory.
    prefix = (data_prefix*)worker_info->rdma_ptr;
    prefix_phys = worker_info->rdma_kbuff;
    prefix_rio = worker_info->rio_addr;

    payload_phys = prefix_phys + sizeof(data_prefix);
    payload_rio = worker_info->rio_addr + sizeof(data_prefix);

    xfer_size = payload_size;
    xfer_offset = 0;

    // By default, send the entire buffer
    if (worker_info->seed)
    {
        DBG("Writer randomizing start, size, and offset.")
        // Transfer some number of bytes
        xfer_offset = rand() % payload_size;
        xfer_size = (rand() % (payload_size - xfer_offset)) + 1;

        payload_phys += xfer_offset;
        payload_rio  += xfer_offset;
    }

    prefix->trans_nth_head = prefix->trans_nth_tail = iter+1;
    prefix->padding = 0xFFFFFFFF;
    prefix->xfer_offset = xfer_offset;
    prefix->xfer_size = worker_info->rdma_buff_size - sizeof(data_prefix);
    DBG("Writer Offset %x Size %x Phys 0x%p RIO 0x%p",
            xfer_offset, xfer_size, (void *)payload_phys, (void *)payload_rio);

    umd_init_xfer_data(prefix, xfer_offset, xfer_size, worker_info);

    // Send payload before prefix, as prefix is used to check for completion
    DBG("Writer DMA Payload send 0x%p 0x%x 0x%p %d",
        (void *)payload_phys, xfer_size,
        payload_rio, worker_info->did_val);
    rc = tsi721_umd_send(worker_info->umd_engine_p,
                        (void *)payload_phys, xfer_size,
                         payload_rio, worker_info->did_val);
    if (rc != 0)
    {
        ERR("FAILED: payload dma transfer returned %d\n"
            "        Offset 0x%x Pay Phys 0x%lx  Size 0x%x RIO 0x%x Did %d\n",
            rc, prefix_phys, sizeof(data_prefix), prefix_rio, worker_info->did_val);
        goto exit;
    }

    //Calculate CRC after DMA sending to let H/W have some time to transfer data to the target device. 
    prefix->CRC32 = crc32(INIT_CRC32, &prefix->payload[0], prefix->xfer_size);

    // Update prefix
    DBG("Writer prefix: iter %d %d CRC32 0x%x offset 0x%lx Size 0x%lx\n",
        prefix->trans_nth_head,
        prefix->trans_nth_tail,
        prefix->CRC32,
        prefix->xfer_offset,
        prefix->xfer_size);

    DBG("Writer DMA Prefix send P 0x%p Bytes 0x%x RIO 0x%p Did %d",
        (void *)prefix_phys, sizeof(data_prefix),
        (void *)prefix_rio, worker_info->did_val);
    rc = tsi721_umd_send(worker_info->umd_engine_p,
                        (void *)prefix_phys, sizeof(data_prefix),
                         prefix_rio, worker_info->did_val);
    if(rc != 0)
    {
        ERR("FAILED: Prefix dma transfer returned %d "
            "Offset 0x%x Pay Phys 0x%lx  Size 0x%x RIO 0x%x Did %d\n",
            rc, prefix_phys, sizeof(data_prefix), prefix_rio, worker_info->did_val);
        goto exit;
    }

    // Wait for reader to check data and send response.
    DBG("Writer waiting for status update.\n");
    while(status->xfer_check == 0)
    {
        if (worker_info->stop_req)
        {
            goto exit;
        }
        usleep(10);
    }
    // Give time for memory writes to complete.
    usleep(10);

    DBG("Writer status update received.\n");
    if((status->xfer_check == 2)
    || (status->trans_nth_head != prefix->trans_nth_head)
    || (status->CRC32 != prefix->CRC32))
    {
        ERR("FAILED: Writer status (from Reader) stat %d : loop %u\n" 
                    "CRC was 0x%x not 0x%x\n",
                    status->xfer_check,
                    iter,
                    status->CRC32,
                    prefix->CRC32);
        ERR("FAILED: Offset 0x%x Pay Phys 0x%lx  Size 0x%x RIO 0x%x Did %d\n",
            payload_phys, xfer_size, payload_rio, worker_info->did_val);
        ERR("  Offset 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
        for (int offset = 0; offset < 0x100; offset+=0x10) {
            ERR("\n%8lx %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x",
                   (uint64_t)(offset),
                   prefix->payload[0 + offset],
                   prefix->payload[1 + offset],
                   prefix->payload[2 + offset],
                   prefix->payload[3 + offset],
                   prefix->payload[4 + offset],
                   prefix->payload[5 + offset],
                   prefix->payload[6 + offset],
                   prefix->payload[7 + offset],
                   prefix->payload[8 + offset],
                   prefix->payload[9 + offset],
                   prefix->payload[10 + offset],
                   prefix->payload[11 + offset],
                   prefix->payload[12 + offset],
                   prefix->payload[13 + offset],
                   prefix->payload[14 + offset],
                   prefix->payload[15 + offset]);
        }
        goto exit;
    }
    ret = 0;
    // Clear payload area back to zero in preparation for next transfer
    memset((void *)&prefix->payload[xfer_offset], 0, xfer_size);

exit:    
    return ret;
}

static int umd_dma_num_reader(struct worker *worker_info, uint32_t iter)
{
    data_status *status = NULL;
    data_prefix *prefix = NULL;
    int32_t ret = -1, rc;
    
    status = (data_status*)worker_info->rdma_ptr;
    prefix = (data_prefix*)worker_info->ib_ptr;

    if (worker_info->stop_req)
        goto exit;

    DBG("Reader waiting for iteration %d, currently %d %d",
          iter + 1, prefix->trans_nth_head, prefix->trans_nth_tail)
    while ((prefix->trans_nth_head != iter+1) ||
           (prefix->trans_nth_tail != iter+1))
    {
        if (worker_info->stop_req)
            goto exit;

        usleep(10);
    }

    status->trans_nth_head = status->trans_nth_tail = prefix->trans_nth_head;

    status->CRC32 = crc32(INIT_CRC32, &prefix->payload[0], prefix->xfer_size);
    DBG("Reader CRCs Rxed  0x%x Computed 0x%x\n", prefix->CRC32, status->CRC32);
    // If the CRC check passes, zero the inbound buffer.
    // If the CRC check fails, preserve the inbound buffer for debug.
    if (status->CRC32 == prefix->CRC32)
    {
        status->xfer_check = 1;
        DBG("Reader zeroing inbound buffer 0x%p 0x%x\n",
             worker_info->ib_ptr, worker_info->ib_byte_cnt);
        memset(worker_info->ib_ptr, 0x0, worker_info->ib_byte_cnt);

    }
    else
    {
        status->xfer_check = 2;
        ERR("FAILED: Reader user data CRC32 validation error\n");
    }

    DBG("Reader sending status buf 0x%p bytes RIO 0x%x DID 0x%x\n",
                 (void *)worker_info->rdma_kbuff,
                 sizeof(data_status),
                 worker_info->rio_addr,
                 worker_info->did_val);
    rc = tsi721_umd_send(worker_info->umd_engine_p,
                 (void *)worker_info->rdma_kbuff,
                 sizeof(data_status),
                 worker_info->rio_addr,
                 worker_info->did_val);
    if (rc !=0 )                
    {
        ERR("FAILED: Reader loop %u, UMD send failed\n", iter);
        goto exit;
    }
    ret = 0;
    
exit:
    return ret;    
}


int umd_dma_num_cmd(struct worker *worker_info, uint32_t iter)
{
    int32_t ret = 0;
    
    if (worker_info->stop_req)
        goto exit;

    // Check for allocated inbound window
    if (!worker_info->ib_valid || !worker_info->ib_ptr)
    {
        ERR("FAILED: inbound window was not allocated\n");
        ret = -1;
        goto exit;
    }

    if (iter == 0)
    {
        memset(worker_info->ib_ptr, 0x0, worker_info->ib_byte_cnt);
	if (worker_info->seed)
        {
            srand(worker_info->seed);
        }
    }

    if (!worker_info->rdma_ptr || !worker_info->rdma_kbuff)
    {
        ERR("FAILED: kernel DMA tx buffer not allocated\n");
        ret  = -1;
        goto exit;
    }

    if (!worker_info->rio_addr || !worker_info->rdma_buff_size)
    {
        ERR("FAILED: rio_addr, rdma_buff_size is 0!\n");
        ret = -1;
        goto exit;
    }

    if(worker_info->wr)
    {
        ret = umd_dma_num_writer(worker_info, iter);
    }
    else
    {
        ret = umd_dma_num_reader(worker_info, iter);
    }

    if (iter && !(iter % 1000))
    {
        INFO("UMD Success for %d transfers\n", iter);
    }
    if ((iter == (worker_info->num_trans - 1)) && worker_info->num_trans)
    {
        CRIT("UMD COMPLETED %d transfers\n", worker_info->num_trans);
    }
exit:

    return ret;
}


void umd_goodput(struct worker *info)
{
    uint32_t i;

    if (alloc_dma_tx_buffer(info))
        goto exit;
    
    zero_stats(info);
    clock_gettime(CLOCK_MONOTONIC, &info->st_time);


    for (i=0; i<info->num_packet; i++)
    {
        info->packet[i].phys_addr = ADDR_P(info->rdma_kbuff, i*info->acc_size);
        info->packet[i].rio_addr  = ADDR_L(info->rio_addr, i*info->acc_size);
        info->packet[i].num_bytes = info->acc_size;
        info->packet[i].dest_id   = info->did_val;
    }

    while (!info->stop_req) {
        start_iter_stats(info);

        int rc = tsi721_umd_send_multi(info->umd_engine_p, info->packet, info->num_packet);
        if (rc)
        {
            ERR("FAILED: rc %d src 0x%p dest_id %d dest 0x%lx size 0x%lx acc_size 0x%lx\n",
                 rc,
                 ADDR_P(info->rdma_kbuff, 0),
                 info->did_val,
                 ADDR_L(info->rio_addr, 0),
                 info->byte_cnt,
                 info->acc_size);
            break;
        }

        info->perf_byte_cnt += info->byte_cnt;
        finish_iter_stats(info);
        clock_gettime(CLOCK_MONOTONIC, &info->end_time);
    }

exit:
    dealloc_dma_tx_buffer(info);
}

#ifdef __cplusplus
}
#endif



