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

#define PATTERN_SIZE  4

struct data_prefix
{
    int trans_nth; /*current transaction index*/
    uint64_t xferf_offset; /* Transfer occurs at xfer_offset bytes from the start of the target buffer */
    uint64_t xfer_size; /* Transfer consists of xfer_size*/
    char pattern[PATTERN_SIZE]; /*Predefined pattern*/
};
    
struct data_suffix
{
    char pattern[PATTERN_SIZE]; /*Predefined pattern*/
};
    
struct data_status
{
    int trans_nth; /*current transaction index*/
    int xfer_check; /* 0 – waiting for transfer check, 1 – passed 2 – failed */
    char pattern[PATTERN_SIZE]; /*Predefined pattern*/
};

static bool umd_allo_ibw(struct UMDEngineInfo *info)
{
    tsi721_umd* h;

    h = info->channel_h;
    h->dev_fd = info->mport_id;
    return true;
}

static bool umd_free_ibw(struct UMDEngineInfo *info)
{
    tsi721_umd* h;

    h = info->channel_h;
    h->dev_fd = info->mport_id;
    return true;
}


bool umd_open(struct UMDEngineInfo *info)
{
    if(tsi721_umd_open(info->channel_h, info->mport_id) == 0)
    {
        return true;
    }
    return false;
}

bool umd_config(struct UMDEngineInfo *info)
{
    if (umd_allo_ibw(info))
    {
        return false;
    }

    if(tsi721_umd_queue_config(info->channel_h, info->ch_idx, (void*)info->ib_rio_addr, info->ib_byte_cnt) == 0)
    {
        return true;
    }

    return false;
}

bool umd_start(struct UMDEngineInfo *info)
{
    if(tsi721_umd_start(info->channel_h) == 0)
    {
        return true;
    }
    return false;
}

bool umd_stop(struct UMDEngineInfo *info)
{
    if(tsi721_umd_stop(info->channel_h) == 0)
    {
        return true;
    }
    return false;
}

bool umd_close(struct UMDEngineInfo *info)
{
    if(umd_free_ibw(info))
    {
        return false;
    }
    
    if(tsi721_close(info->channel_h) == 0)
    {
        return true;
    }
    
    return false;
}

void umd_dma_num_cmd(struct UMDEngineInfo *info)
{
    data_status *status;
    data_prefix *prefix;
    data_suffix *suffix;
    int32_t err = 0;
    int i;

    if (!info->rio_addr || !info->buf_size) 
    {
        ERR("FAILED: rio_addr, buf_size is 0!\n");
        return;
    }

    if(info->wr)
    {
        prefix = (data_prefix*)info->rio_addr;
    
        for(i=0; i<info->max_iter; i++)
        {
            status = (data_status*)info->ib_ptr;
            memset(status, 0, sizeof(data_status));

            memset(prefix, 0, sizeof(data_prefix));            
            prefix->trans_nth = i+1; 
            prefix->pattern[0] = 0x12;
            prefix->pattern[1] = 0x34;
            prefix->pattern[2] = 0x56;
            prefix->pattern[3] = 0x78;
            prefix->xferf_offset = info->rio_addr+sizeof(data_prefix);
            prefix->xfer_size = info->buf_size+sizeof(data_prefix);

            suffix = (data_suffix*)(prefix + prefix->xfer_size);
            memset(suffix, 0, sizeof(data_suffix));
            suffix->pattern[0] = 0x1a;
            suffix->pattern[1] = 0x2b;
            suffix->pattern[2] = 0x3c;
            suffix->pattern[3] = 0x4d;

            prefix->xfer_size += sizeof(data_suffix);

            err = tsi721_umd_send(info->channel_h, (void*)info->ib_rio_addr, prefix->xfer_size, info->rio_addr, info->dest_id);
            if(err == 0)
            {
                while(status->xfer_check == 0)
                {
                    sleep(0.25);
                }

                if(status->xfer_check == 2 || 
                    status->trans_nth != prefix->trans_nth ||
                    status->pattern[0] != 0x11 ||
                    status->pattern[1] != 0x22 ||
                    status->pattern[2] != 0x33 ||
                    status->pattern[3] != 0x44 )
                {
                    ERR("FAILED: pattern check error %d\n", err);
                    break;
                }
            }
            else
            {
                ERR("FAILED: UMD send failed\n");
                break;
            }
        }
    }
    else
    {
        status = (data_status*)info->ib_ptr;
        prefix = (data_prefix*)info->rio_addr;
        
        if(prefix->pattern[0] == 0x12 &&
            prefix->pattern[1] == 0x34 &&
            prefix->pattern[2] == 0x56 &&
            prefix->pattern[3] == 0x78)
        {
           suffix = (data_suffix*)(prefix + info->buf_size + sizeof(data_prefix));
           if(suffix->pattern[0] == 0x1a &&
              suffix->pattern[1] == 0x2b &&
              suffix->pattern[2] == 0x3c &&
              suffix->pattern[3] == 0x4d)
           {
              status->trans_nth = prefix->trans_nth;
              status->xfer_check = 1;
              status->pattern[0] = 0x11;
              status->pattern[1] = 0x22;
              status->pattern[2] = 0x33;
              status->pattern[3] = 0x44;
           }
           else
           {
              status->xfer_check = 2;
           }
        }
        else
        {
            status->xfer_check = 2;
        }
    }
}

#ifdef __cplusplus
}
#endif



