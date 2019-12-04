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

struct UMDEngineInfo umd_engine;


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


int umdDmaNumCmd(struct cli_env *env, int argc, char **argv)
{
    int idx;
    uint64_t ib_size;
    uint64_t ib_rio_addr = RIO_MAP_ANY_ADDR;
    did_val_t did_val;
    uint64_t rio_addr;
    uint64_t buf_sz;
    uint16_t wr;
    uint32_t num_trans;
    char *user_data_p = NULL;
    struct UMDEngineInfo *engine_p = &umd_engine;
    struct DmaTransfer *dma_trans_p;
    int ret = -1;
    int n = 0;
    engine_p->env = env;

    if(tok_parse_long(argv[n++], &idx, 0, MAX_UDM_USER_THREAD, 0))
    {
        LOGMSG(env, "\n");
        LOGMSG(env, TOK_ERR_LONG_MSG_FMT,"<idx>", 0, MAX_UDM_USER_THREAD);
        goto exit;
    }
    dma_trans_p = &(engine_p->dma_trans[idx]);

    if (gp_parse_ull_pw2(env, argv[1], "<ib_size>", &ib_size, FOUR_KB,
            4 * SIXTEEN_MB)) {
        goto exit;
    }

    if(tok_parse_ulonglong(argv[2], &ib_rio_addr, 1, UINT64_MAX, 0))
    {
        LOGMSG(env, "\n");
        LOGMSG(env, TOK_ERR_ULONGLONG_HEX_MSG_FMT, "<ib_rio_addr>",
                (uint64_t )1, (uint64_t)UINT64_MAX);
        goto exit;
    }

    if (gp_parse_ull_pw2(env, argv[1], "<ib_size>", &ib_size, FOUR_KB,
            4 * SIXTEEN_MB)) {
        goto exit;
    }

    if ((ib_rio_addr != RIO_MAP_ANY_ADDR) && ((ib_size-1) & ib_rio_addr)) {
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

    if( argc > 8)
    {
        user_data_p = argv[n];
    }

    if (engine_p->stat == ENGINE_READY && !dma_trans_p->is_in_use)
    {
        dma_trans_p->ib_byte_cnt = ib_size;
        dma_trans_p->ib_rio_addr = ib_rio_addr;
        dma_trans_p->rio_addr = rio_addr;
        dma_trans_p->buf_size = buf_sz;
        dma_trans_p->num_trans = num_trans;
        dma_trans_p->wr = wr;
        memcpy(dma_trans_p->user_data, user_data_p,8);
        umd_dma_num_cmd(engine_p, idx);
        ret = 0;
    }

exit:
    return ret;

}

struct cli_cmd UMDDmaNum =
{
    "Udnum",
    2,
    7,
    "Send a specified number of DMA reads/writes",
    "Udnum <idx> <inb_size> <inb_rio_addr> <did> <rio_addr> <bytes> <buf_sz> <wr> <num> <data>\n"
        "<idx>      User DMA test thread index: 0 to 7\n"
        "<ib_rio_addr> is the RapidIO address for the inbound window\n"
        "       NOTE: <addr> must be aligned to <size>\n"
        "<ib_size> inbound window size. Must be a power of two from 0x1000 to 0x01000000\n"
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


int umdConfigCmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
    int ret = -1;
    struct UMDEngineInfo *engine_p = &umd_engine;
    engine_p->env = env;

    if(umd_config(engine_p))
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
    "Uconfig\n"
    "Confiure DMA engine queue memory. Set Tsi721 User Mode Driver to CONFIGURED state.\n",
    umdConfigCmd,
    ATTR_NONE
};


int umdStartCmd(struct cli_env *env, int UNUSED(argc), char **UNUSED(argv))
{
    int ret = -1;
    struct UMDEngineInfo *engine_p = &umd_engine;
    engine_p->env = env;

    if(umd_start(engine_p))
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

    if(umd_stop(engine_p))
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

    if(umd_close(engine_p))
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
