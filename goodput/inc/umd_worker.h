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
#include "tsi721_umd.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_UDM_USER_THREAD_IDX 7
#define MAX_UDM_USER_THREAD (MAX_UDM_USER_THREAD_IDX + 1)

enum EngineStat
{
    ENGINE_UNALLOCATED,
    ENGINE_UNCONFIGURED,
    ENGINE_CONFIGURED,
    ENGINE_READY,
    ENGINE_STATE_MAX
};

struct DmaTransfer
{
    int index;
    bool is_in_use;

    bool wr;
    int  dest_id;
    uint64_t rio_addr; /* Target RapidIO address for direct IO and DMA */
    uint64_t buf_size; /* Total mumber of bytes to access for direct IO and DMA */
    uint64_t acc_size; /* Access size of one DMA iteratioin*/
    uint32_t num_trans; /* Number of loops for data transfer. 0 indicates infinite number of loops*/
    uint64_t user_data; /*User predinfed data*/
    struct timespec st_time; /* Start of the run, for throughput */
    struct timespec end_time; /* End of the run, for throughput*/

    int ib_valid;
    uint64_t ib_handle; /* Inbound window RapidIO handle */
    uint64_t ib_rio_addr; /* Inbound window RapidIO address */
    uint64_t ib_byte_cnt; /* Inbound window size */
    void *ib_ptr; /* Pointer to mapped ib_handle. Start address of ibw in user space */

    void *tx_ptr;
    uint64_t tx_mem_h;
};

struct UMDEngineInfo
{
    int mport_id;
    enum EngineStat stat; //all state transfer will require mutex for pretection. Assume race condition will rarely happen. So no pretection for now.
    struct tsi721_umd engine;
    uint8_t chan_mask;

    void *queue_mem_ptr;
    uint64_t queue_mem_h;

    struct DmaTransfer dma_trans[MAX_UDM_USER_THREAD];

    struct cli_env *env;
};

extern struct UMDEngineInfo umd_engine;

extern int umd_init_engine(struct UMDEngineInfo *info);

extern int umd_open(struct UMDEngineInfo *info);

extern int umd_config(struct UMDEngineInfo *info);

extern int umd_start(struct UMDEngineInfo *info);

extern int umd_stop(struct UMDEngineInfo *info);

extern int umd_close(struct UMDEngineInfo *info);

extern int umd_dma_num_cmd(struct worker *worker_info, uint32_t iter);
extern void umd_goodput(struct worker *info);

#ifdef __cplusplus
}
#endif

#endif /* __UMD_WORKER_H__ */


