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
#include "worker.h"
#include "umd_worker.h"
#include "pw_handling.h"
#include "goodput.h"
#include "Tsi721.h"
#include "CPS1848.h"
#include "rio_misc.h"
#include "did.h"
#include "tsi721_umd.h"




#ifdef __cplusplus
extern "C" {
#endif

static bool umd_allo_ibw(struct UMDChannelInfo *info)
{
    tsi721_umd* h;

    h = info->channel_h;
    h->dev_fd = info->mport_id;
    return true;
}

static bool umd_free_ibw(struct UMDChannelInfo *info)
{
    tsi721_umd* h;

    h = info->channel_h;
    h->dev_fd = info->mport_id;
    return true;
}


bool umd_open(struct UMDChannelInfo *info)
{
    tsi721_umd* h;

    h = info->channel_h;
    h->dev_fd = info->mport_id;
    return true;
}

bool umd_config(struct UMDChannelInfo *info)
{
    tsi721_umd* h;

    h = info->channel_h;
    h->dev_fd = info->mport_id;
    if (umd_allo_ibw(info))
    {
        return false;
    }

    return true;
}

bool umd_start(struct UMDChannelInfo *info)
{
    tsi721_umd* h;

    h = info->channel_h;
    h->dev_fd = info->mport_id;
    return true;

}

bool umd_stop(struct UMDChannelInfo *info)
{
    tsi721_umd* h;

    h = info->channel_h;
    h->dev_fd = info->mport_id;
    return true;
}

bool umd_close(struct UMDChannelInfo *info)
{
    tsi721_umd* h;

    h = info->channel_h;
    h->dev_fd = info->mport_id;
    if(umd_free_ibw(info))
    {
        return false;
    }
    return true;
}

void umd_dma_num_cmd(struct UMDChannelInfo *info)
{
    tsi721_umd* h;

    h = info->channel_h;
    h->dev_fd = info->mport_id;
}

#ifdef __cplusplus
}
#endif



