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


#ifndef __UMD_WORKER_H__
#define __UMD_WORKER_H__

#include "worker.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_UMD_CH_IDX 7
#define MAX_UMD_CH (MAX_UMD_CH_IDX + 1)

struct UMDChannelInfo
{
    int ch_idx;
    int mport_id;
    struct tsi721_umd* channel_h;

    uint64_t rio_addr; /* Target RapidIO address for direct IO and DMA */
    uint64_t buf_size; /* Number of bytes to access for direct IO and DMA */
    int      max_iter; /* For infinite loop tests make this the upper bound of loops*/

    int ib_valid;
    uint64_t ib_handle; /* Inbound window RapidIO handle */
    uint64_t ib_rio_addr; /* Inbound window RapidIO address */
    uint64_t ib_byte_cnt; /* Inbound window size */
    void *ib_ptr; /* Pointer to mapped ib_handle. Start address of ibw in user space */
    
};


bool umd_open(struct UMDChannelInfo *info);

bool umd_config(struct UMDChannelInfo *info);

bool umd_start(struct UMDChannelInfo *info);

bool umd_stop(struct UMDChannelInfo *info);

bool umd_close(struct UMDChannelInfo *info);

void umd_dma_num_cmd(struct UMDChannelInfo *info);


#ifdef __cplusplus
}
#endif

#endif /* __UMD_WORKER_H__ */


