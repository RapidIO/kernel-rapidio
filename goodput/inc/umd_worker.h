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

extern struct tsi721_umd umd_engine;

extern int umd_init_engine_handle(struct tsi721_umd *engine_p);

extern int umd_config(struct tsi721_umd *engine_p, uint8_t chan_mask);

extern int umd_close(struct tsi721_umd *engine_p);

extern int umd_dma_num_cmd(struct worker *worker_info, uint32_t iter);

extern void umd_goodput(struct worker *info);

#ifdef __cplusplus
}
#endif

#endif /* __UMD_WORKER_H__ */


