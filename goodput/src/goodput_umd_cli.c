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
#include <stdio.h>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <math.h>
#include <time.h>

#include "rio_misc.h"
#include "rio_route.h"
#include "tok_parse.h"
#include "rio_mport_lib.h"
#include "string_util.h"
#include "goodput_cli.h"


#include "libtime_utils.h"
#include "librsvdmem.h"
#include "liblog.h"
#include "assert.h"
#include "math_util.h"
#include "RapidIO_Error_Management_API.h"
#include "pw_handling.h"
#include "CPS1848.h"
#include "CPS1616.h"
#include "RXS2448.h"
#include "Tsi721.h"
#include "cps_event_test.h"
#include "umd_worker.h"
#include "goodput_umd_cli.h"


#ifdef __cplusplus
extern "C" {
#endif

#define DMA_USER_DATA_PATTERN 0xFEB6FEB5C9D8C9D7

struct UMDEngineInfo umd_engine;

int umdOpenCmd(struct cli_env *env, int argc, char **argv);
int umdConfigCmd(struct cli_env *env, int argc, char **argv);
int umdStartCmd(struct cli_env *env, int argc, char **argv);

// Parse the token ensuring it is within the provided range. Further ensure it
// is a power of 2
static int gp_parse_ull_pw2(struct cli_env *env, char *tok, const char *name,
        uint64_t *value, uint64_t min, uint64_t max)
{
    if (tok_parse_ulonglong(tok, value, min, max, 0)) {
        LOGMSG(env, "\n");
        LOGMSG(env, TOK_ERR_ULONGLONG_HEX_MSG_FMT, name, min, max);
        goto err;
    }

    if ((*value - 1) & *value) {
        LOGMSG(env, "\n%s must be a power of 2\n", name);
        goto err;
    }

    return 0;
err:
    return 1;
}


static int gp_parse_did(struct cli_env *env, char *tok, did_val_t *did_val)
{
    if (tok_parse_did(tok, did_val, 0)) {
        LOGMSG(env, "\n");
        LOGMSG(env, "<did> must be between 0 and 0xff\n");
        return 1;
    }
    return 0;
}

// Parse the token ensuring it is within the range for a worker index and
// check the status of the worker thread.
static int gp_parse_worker_index(struct cli_env *env, char *tok, uint16_t *idx)
{
    if (tok_parse_ushort(tok, idx, 0, MAX_WORKER_IDX, 0)) {
        LOGMSG(env, "\n");
        LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, "<idx>", 0, MAX_WORKER_IDX);
        return 1;
    }
    return 0;
}

// Parse the token ensuring it is within the range for a worker index and
// check the status of the worker thread.
static int gp_parse_worker_index_check_thread(struct cli_env *env, char *tok,
        uint16_t *idx, int want_halted)
{
    if (gp_parse_worker_index(env, tok, idx)) {
        goto err;
    }

    switch (want_halted) {
    case 0: if (2 == wkr[*idx].stat) {
            LOGMSG(env, "\nWorker halted\n");
            goto err;
        }
        break;

    case 1: if (2 != wkr[*idx].stat) {
            LOGMSG(env, "\nWorker not halted\n");
            goto err;
        }
        break;
    case 2: if (1 != wkr[*idx].stat) {
            LOGMSG(env, "\nWorker not running\n");
            goto err;
        }
        break;
    default: goto err;
    }
    return 0;
err:
    return 1;
}


int umdDmaNumCmd(struct cli_env *env, int argc, char **argv)
{
    uint16_t idx;
    uint64_t ib_size;
    uint64_t ib_rio_addr = RIO_MAP_ANY_ADDR;
    did_val_t did_val;
    uint64_t rio_addr;
    uint64_t buf_sz;
    uint16_t wr;
    uint32_t num_trans;
    uint64_t user_data = DMA_USER_DATA_PATTERN;
    struct UMDEngineInfo *engine_p = &umd_engine;
    int ret = -1;
    int n = 0;
    engine_p->env = env;

    if (gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1)) {
        goto exit;
    }

    if (gp_parse_ull_pw2(env, argv[n++], "<ib_size>", &ib_size, FOUR_KB, 4 * SIXTEEN_MB))
    {
        goto exit;
    }

    if(tok_parse_ulonglong(argv[n++], &ib_rio_addr, 1, UINT64_MAX, 0))
    {
        LOGMSG(env, "\n");
        LOGMSG(env, TOK_ERR_ULONGLONG_HEX_MSG_FMT, "<ib_rio_addr>",
                (uint64_t )1, (uint64_t)UINT64_MAX);
        goto exit;
    }

    if ((ib_rio_addr != RIO_MAP_ANY_ADDR) && ((ib_size-1) & ib_rio_addr))
    {
        LOGMSG(env, "\n<addr> not aligned with <size>\n");
        goto exit;
    }

    if (gp_parse_did(env, argv[n++], &did_val))
    {
        goto exit;
    }

    if (tok_parse_ulonglong(argv[n++], &rio_addr, 1, UINT64_MAX, 0))
    {
        LOGMSG(env, "\n");
        LOGMSG(env, TOK_ERR_ULONGLONG_HEX_MSG_FMT, "<rio_addr>",
                (uint64_t)1, (uint64_t)UINT64_MAX);
        goto exit;
    }

    if (gp_parse_ull_pw2(env, argv[n++], "<buf_size>", &buf_sz, FOUR_KB, 4 * SIXTEEN_MB))
    {
        goto exit;
    }

    if (tok_parse_ushort(argv[n++], &wr, 0, 2, 0))
    {
        goto exit;
    }

    if (argc > 7 && tok_parse_ul(argv[n++], &num_trans, 0))
    {
        LOGMSG(env, "\n");
        LOGMSG(env, TOK_ERR_UL_HEX_MSG_FMT, "<num>");
        goto exit;
    }

    if( argc > 8 && tok_parse_ull(argv[n], &user_data,0))
    {
        LOGMSG(env, "\n");
        LOGMSG(env, TOK_ERR_ULL_HEX_MSG_FMT, "<user_data>");
        user_data = DMA_USER_DATA_PATTERN;
    }
    else
    {
        LOGMSG(env, "User data 0x%lx\n",user_data);
    }

    // For convenience, if the engine is unallocated then open and allocate it now
    // Assume mport 0
    if (umd_engine.stat == ENGINE_UNALLOCATED)
    {
        printf("UMD worker started without engine ready. Opening with default mport and channels\n");
        umdOpenCmd(env, 0, NULL);
        umdConfigCmd(env, 0, NULL);
        umdStartCmd(env, 0, NULL);
    }

    wkr[idx].action = dma_tx_num;
    wkr[idx].action_mode = user_mode_action;
    wkr[idx].did_val = did_val;
    wkr[idx].rio_addr = rio_addr;
    wkr[idx].byte_cnt = buf_sz;
    wkr[idx].acc_size = buf_sz;
    wkr[idx].wr = (int)wr;
    wkr[idx].use_kbuf = 1;
    wkr[idx].dma_trans_type = RIO_EXCHANGE_NWRITE;
    wkr[idx].dma_sync_type = RIO_TRANSFER_ASYNC;
    wkr[idx].rdma_buff_size = buf_sz;
    wkr[idx].num_trans = (int)num_trans;
    wkr[idx].user_data = user_data;
    LOGMSG(env, "Wr %x rio 0x%lx num_trans %d sz 0x%lx\n",
        wr, rio_addr, num_trans, buf_sz);

    ret = 0;

    sem_post(&wkr[idx].run);

exit:
    return ret;
}

struct cli_cmd UMDDmaNum =
{
    "Udnum",
    2,
    7,
    "Send a specified number of DMA reads/writes",
    "Udnum <idx> <ib_size> <ib_rio_addr> <did> <rio_addr> <buf_sz> <wr> <num> <data>\n"
        "<idx>      User DMA test thread index: 0 to 7\n"
        "<ib_size>  inbound window size. Must be a power of two from 0x1000 to 0x01000000\n"
        "<ib_rio_addr> is the RapidIO address fo the inbound window\n"
        "       NOTE: <addr> must be aligned to <size>\n"
        "<did>      target device ID\n"
        "<rio_addr> is target RapidIO memory address to access\n"
        "<buf_sz>   target buffer size, must be a power of two from 0x1000 to 0x01000000\n"
        "<wr>       0: Read, 1: Write,2:Ramdom Writer\n"
        "<num>      Optional. Default is 0. Number of transactions to send. 0 indicates infinite loop\n"
        "<data>     RND, or constant data value, written every 8 bytes",
    umdDmaNumCmd,
    ATTR_NONE
};

int umdOpenCmd(struct cli_env *env, int argc, char **argv)
{
    uint32_t mport_id = 0;
    struct UMDEngineInfo *engine_p = &umd_engine;
    engine_p->env = env;

    if(argc && tok_parse_mport_id(argv[0], &mport_id, 0)) {
    LOGMSG(env, TOK_ERR_MPORT_MSG_FMT);
        goto exit;
    }

    engine_p->mport_id = mport_id;

    if (umd_open(engine_p))
    {
        LOGMSG(env, "Tsi721 UMD Open FAILED.\n");
    }
    else
    {
        LOGMSG(env, "Tsi721 UMD Open PASSED.\n");
    }
exit:
    return 0;
}

struct cli_cmd UMDOpen =
{
    "Uopen",
    2,
    0,
    "Reserve a UMD engine",
    "Uopen <mport_num>\n"
    "Set the Tsi721 User Mode Driver to OPEN state.\n"
    "<mport_num> optional local rapidio device port. Default is 0. \n",
    umdOpenCmd,
    ATTR_NONE
};


int umdConfigCmd(struct cli_env *env, int argc, char **argv)
{
    int ret = -1;
    struct UMDEngineInfo *engine_p = &umd_engine;
    uint16_t chan_mask = 0x40;
    engine_p->env = env;

    if(argc)
    {
        if(tok_parse_ushort(argv[0], &chan_mask, 0x01, 0xFF, 0))
        {
            LOGMSG(env, "\n");
            LOGMSG(env, TOK_ERR_SHORT_MSG_FMT,"<chan_mask>", 0X1, 0XFF);
            LOGMSG(env, "Use default 0x%x", 0x40);
            chan_mask = 0x40;
        }
    }

    engine_p->chan_mask = (uint8_t)chan_mask;
    if(!umd_config(engine_p))
    {
        ret = 0;
    }

    return ret;
}

struct cli_cmd UMDConfig =
{
    "Uconfig",
    3,
    0,
    "Configure a UMD engine",
    "Uconfig <chan_mask>\n"
    "Confiure DMA engine queue memory/channels. Set Tsi721 User Mode Driver to CONFIGURED state.\n"
    "<chan_mask> is an optional parameter to allocate DMA channles.\n" 
    "            Bitfield. Range 0x1 to 0xFF, default 0x40\n",
    umdConfigCmd,
    ATTR_NONE
};


int umdStartCmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
    int ret = -1;
    struct UMDEngineInfo *engine_p = &umd_engine;
    engine_p->env = env;

    if(!umd_start(engine_p))
    {
        ret = 0;
    }

    return ret;
}

struct cli_cmd UMDStart =
{
    "Ustart",
    4,
    0,
    "Get a UMD engine ready",
    "Ustart\n"
    "Add a DMA engine ID to list of available DMA engines.\n"
    "Set Tsi721 User Mode Driver to READY state.\n",
    umdStartCmd,
    ATTR_NONE
};

int umdStopCmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
    int ret = -1;
    struct UMDEngineInfo *engine_p = &umd_engine;
    engine_p->env = env;

    if(!umd_stop(engine_p))
    {
        ret = 0;
    }

    return ret;

}

struct cli_cmd UMDStop =
{
    "Ustop",
    4,
    0,
    "Stop a UMD engine",
    "Ustop \n"
    "Remove a DMA engine ID from list of available DMA engines.\n"
    "Set Tsi721 User Mode Driver back to CONFIGURED state.\n",
    umdStopCmd,
    ATTR_NONE

};

int umdCloseCmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
    int ret = -1;
    struct UMDEngineInfo *engine_p = &umd_engine;
    engine_p->env = env;

    if(!umd_close(engine_p))
    {
        ret = 0;
    }

    return ret;

}

struct cli_cmd UMDClose =
{
    "Uclose",
    3,
    0,
    "Free a UMD engine",
    "Uclose\n"
    "Free all resource of a DMA engine.\n"
    "Set Tsi721 User Mode Driver back to OPEN state.\n",
    umdCloseCmd,
    ATTR_NONE
};


#ifdef __cplusplus
}
#endif
