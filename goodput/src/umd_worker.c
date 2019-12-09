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
static const uint8_t SUFFIX_PATTERN[PATTERN_SIZE]={0x1a, 0x2b, 0x3c, 0x4d, 0xa1, 0xb2, 0xc3, 0xd4};
static const uint8_t STATUS_PATTERN[PATTERN_SIZE]={0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

struct data_prefix
{
    uint32_t trans_nth; /*current transaction index*/
    uint32_t CRC32;
    uint64_t xfer_offset; /* Transfer occurs at xfer_offset bytes from the start of the target buffer */
    uint64_t xfer_size; /* Transfer consists of xfer_size*/
    uint8_t pattern[PATTERN_SIZE]; /*Predefined pattern*/
};

struct data_suffix
{
    uint8_t pattern[PATTERN_SIZE]; /*Predefined pattern*/
};

struct data_status
{
    int trans_nth; /*current transaction index*/
    int xfer_check; /* 0 – waiting for transfer check, 1 – passed 2 – failed */
    uint8_t pattern[PATTERN_SIZE]; /*Predefined pattern*/
};

static uint64_t queue_mem_h;

static int umd_allo_queue_mem(struct tsi721_umd *umd_engine_p)
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

static uint32_t crc32(uint32_t crc, void *buffer, uint32_t size)  
{  
    uint32_t i; 
    uint8_t *data_p = (uint8_t *)buffer;
    for (i = 0; i < size; i++)
    {  
        crc = crc32_table[(crc ^ data_p[i]) & 0xff] ^ (crc >> 8);  
    }  
    return crc ;  
}

static void umd_copy_xfer_data(void *buf, uint32_t size, uint64_t user_data)
{
    uint64_t *data_p =  (uint64_t *)buf; //Assume 64 byte alignment.
    uint32_t count = size / 4;
    uint32_t i;
    for(i = 0; i < count; i++)
    {
        *data_p =  user_data;
        data_p++;
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
    if(!umd_allo_queue_mem(engine_p))
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

static int umd_dma_num_write(struct worker *worker_info, uint32_t iter)
{
    data_status *status = NULL;
    data_prefix *prefix = NULL;
    data_suffix *suffix = NULL;
    void *xfer_p;
    int32_t ret = 0, rc;
    void *payload_phys, *prefix_phys;
    uint32_t payload_size = worker_info->rdma_buff_size - sizeof(data_prefix) - sizeof(data_suffix);
    prefix = (data_prefix*)worker_info->rdma_ptr;
    status = (data_status*)worker_info->ib_ptr;
    prefix_phys  = (void*)worker_info->rdma_kbuff;
    payload_phys = (void*)((uintptr_t)worker_info->rdma_kbuff + sizeof(data_prefix));

    if (worker_info->stop_req)
    {
       ret = -1;
       goto exit;
    }

    memset(status, 0, sizeof(data_status));
    memset(prefix, 0, sizeof(data_prefix));
    prefix->trans_nth = iter+1;
    memcpy(prefix->pattern, PREFIX_PATTERN, PATTERN_SIZE);
    prefix->xfer_offset = sizeof(data_prefix);
    prefix->xfer_size = worker_info->rdma_buff_size - sizeof(data_prefix) - sizeof(data_suffix);

    xfer_p = (void *)((uint8_t *)(worker_info->rdma_ptr) + prefix->xfer_offset);
    umd_copy_xfer_data(xfer_p, prefix->xfer_size, worker_info->user_data);
    prefix->CRC32 = crc32(INIT_CRC32, xfer_p, prefix->xfer_size);

    suffix = (data_suffix*)((uint64_t)(worker_info->rdma_ptr)  +   prefix->xfer_offset + prefix->xfer_size);
    memset(suffix, 0, sizeof(data_suffix));
    memcpy(suffix->pattern, SUFFIX_PATTERN, PATTERN_SIZE);

    // Send payload and suffix before prefix, as prefix is used to check for completion
    rc = tsi721_umd_send(worker_info->umd_engine_p, payload_phys, payload_size + sizeof(data_suffix), worker_info->rio_addr+sizeof(data_prefix), worker_info->did_val);
    if (rc != 0)
    {
        ERR("FAILED: payload dma transfer returned %d\n",ret);
        goto exit;
    }

    // Update prefix
    rc = tsi721_umd_send(worker_info->umd_engine_p, prefix_phys, sizeof(data_prefix), worker_info->rio_addr, worker_info->did_val);
    if(rc == 0)
    {
        while(status->xfer_check == 0)
        {
            if (worker_info->stop_req)
                goto exit;

            sleep(0.25);
        }

        if(status->xfer_check == 2 ||
            status->trans_nth != prefix->trans_nth ||
            !memcmp(status->pattern, STATUS_PATTERN, 8))
        {
            ERR("FAILED: Writer status(from Reader) pattern check error: loop %u\n" 
                        "xfer_check %d, data 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                        iter,
                        status->xfer_check,
                        status->pattern[0],
                        status->pattern[1],
                        status->pattern[2],
                        status->pattern[3],
                        status->pattern[4],
                        status->pattern[5],
                        status->pattern[6],
                        status->pattern[7]);
                    ret  = -1;
                    goto exit;
        }
    }
    else
    {
        ERR("FAILED: Writer UMD send failed\n");
        ret = -1;
        goto exit;
    }
exit:    
    return ret;
}

static int umd_dma_num_read(struct worker *worker_info, uint32_t iter)
{
    data_status *status = NULL;
    data_prefix *prefix = NULL;
    void *xfer_p;
    data_suffix *suffix = NULL;
    int32_t ret = 0, rc;
    
    status = (data_status*)worker_info->rdma_ptr;
    prefix = (data_prefix*)worker_info->ib_ptr;

    if (worker_info->stop_req)
        goto exit;

    while(prefix->trans_nth != iter+1 )
    {
        if (worker_info->stop_req)
            goto exit;

        sleep(0.25);
    }

    memcpy(status->pattern, STATUS_PATTERN, PATTERN_SIZE);
    status->trans_nth = prefix->trans_nth;

    if(memcmp(prefix->pattern, PREFIX_PATTERN, 8))
    {
        suffix = (data_suffix*)((uint64_t)(worker_info->ib_ptr) + prefix->xfer_offset + prefix->xfer_size);

        while(suffix->pattern[0] == 00 &&
                suffix->pattern[1] == 00 &&
                suffix->pattern[2] == 00 &&
                suffix->pattern[3] == 00 &&
                suffix->pattern[4] == 00 &&
                suffix->pattern[5] == 00 &&
                suffix->pattern[6] == 00 &&
                suffix->pattern[7] == 00)
        {
            sleep(0.25);
        }
                    
        if(memcmp(suffix->pattern, SUFFIX_PATTERN, 8))
        {
            xfer_p = (void *)((uint8_t *)(worker_info->ib_ptr) + prefix->xfer_offset);   
            if(prefix->CRC32 != crc32(INIT_CRC32, xfer_p, prefix->xfer_size))
            {
                status->xfer_check = -1;
                ERR("FAILED: Reader user data CRC32 validation error\n");
            }
            else
            {
                status->xfer_check = 1;
            }
        }
        else
        {
            ret = -1;
                   
            ERR("FAILED: Reader suffix validation error, loop %u\n" 
                    "data 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                        iter,
                        suffix->pattern[0],
                        suffix->pattern[1],
                        suffix->pattern[2],
                        suffix->pattern[3],
                        suffix->pattern[4],
                        suffix->pattern[5],
                        suffix->pattern[6],
                        suffix->pattern[7]);
                   
            status->xfer_check = 2;
        }
    }
    else
    {
        ret = -1;
            
       ERR("FAILED: Reader prefix validation error, loop %u\n"
                    "data 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                    iter,
                    prefix->pattern[0],
                    prefix->pattern[1],
                    prefix->pattern[2],
                    prefix->pattern[3],
                    prefix->pattern[4],
                    prefix->pattern[5],
                    prefix->pattern[6],
                    prefix->pattern[7]);
                
        status->xfer_check = 2;
    }

    memset(prefix, 0, sizeof(data_prefix));
    memset(suffix, 0, sizeof(data_suffix));

    rc = tsi721_umd_send(worker_info->umd_engine_p, (void *)worker_info->rdma_kbuff, worker_info->rdma_buff_size, worker_info->rio_addr, worker_info->did_val);
    if (rc !=0 )                
    {
        ret = -1;
        ERR("FAILED: Reader loop %u, UMD send failed\n", iter);
    }
    
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
        memset(worker_info->ib_ptr, 0x0, worker_info->ib_byte_cnt);

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
        ret = umd_dma_num_write(worker_info, iter);
    }
    else
    {
        ret = umd_dma_num_read(worker_info, iter);
    }

exit:

    return ret;
}


void umd_goodput(struct worker *info)
{
    if (alloc_dma_tx_buffer(info))
        goto exit;

    zero_stats(info);
    clock_gettime(CLOCK_MONOTONIC, &info->st_time);

    while (!info->stop_req) {
        start_iter_stats(info);
        for (uint64_t count = 0; (count < info->byte_cnt) && !info->stop_req;
             count += info->acc_size)
        {
            int rc = 0;
            rc = tsi721_umd_send(info->umd_engine_p,
                                 ADDR_P(info->rdma_kbuff, count),
                                 info->acc_size,
                                 ADDR_L(info->rio_addr, count),
                                 info->did_val
                                );
            if(rc)
            {
                ERR("FAILED: rc %d src 0x%p dest_id %d dest 0x%x size 0x%x\n",
                     rc,
                     ADDR_P(info->rdma_kbuff, count),
                     info->did_val,
                     ADDR_L(info->rio_addr, count),
                     info->acc_size)
                break;
            }
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



