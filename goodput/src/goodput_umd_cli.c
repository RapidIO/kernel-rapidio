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

struct UMDEngineInfo umd_engine[MAX_UMD_CH];

extern enum rio_exchange convert_int_to_riomp_dma_directio_type(uint16_t trans);


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

// Parse the token as a boolean value. The range of the token is restricted
// to the numeric values of 0 (false) and 1 (true)
static int gp_parse_bool(struct cli_env *env, char *tok, const char *name, uint16_t *boo)
{
    if (tok_parse_ushort(tok, boo, 0, 1, 0)) {
        LOGMSG(env, "\n");
        LOGMSG(env, TOK_ERR_USHORT_MSG_FMT, name, 0, 1);
        return 1;
    }
    return 0;
}

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

static int gp_parse_cpu(struct cli_env *env, char *dec_parm, int *cpu)
{
    const int MAX_GOODPUT_CPU = getCPUCount() - 1;

    if (tok_parse_long(dec_parm, cpu, -1, MAX_GOODPUT_CPU, 0)) {
        LOGMSG(env, "\n");
        LOGMSG(env, TOK_ERR_LONG_MSG_FMT, "<cpu>", -1, MAX_GOODPUT_CPU);
        return 1;
    }
    return 0;
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


int umdDmaNumCmd(struct cli_env *env, int UNUSED(argc), char **argv)
{
    uint16_t idx;
    did_val_t did_val;
    uint64_t rio_addr;
    uint64_t acc_sz;
    uint16_t wr;
    uint32_t num_trans;
    
    int n = 0;
    if (gp_parse_worker_index_check_thread(env, argv[n++], &idx, 1)) {
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

    if (gp_parse_ull_pw2(env, argv[n++], "<acc_sz>", &acc_sz, 1, UINT32_MAX))
    {
        goto exit;
    }
    
    if (gp_parse_bool(env, argv[n++], "<wr>", &wr)) 
    {
        goto exit;
    }
    
    
    if (tok_parse_ul(argv[n++], &num_trans, 0)) 
    {
        LOGMSG(env, "\n");
        LOGMSG(env, TOK_ERR_UL_HEX_MSG_FMT, "<num>");
        goto exit;
    }
    
    
exit:
    return 0;

}
    
int umdOpen(struct cli_env *env, int UNUSED(argc), char **argv)
{
    return 0;
}


int umdConfig(struct struct cli_env *env, int UNUSED(argc), char **argv)
{
    return 0;
}

int umdStart(struct struct cli_env *env, int UNUSED(argc), char **argv)
{
    return 0;
}

int umdStop(struct struct cli_env *env, int UNUSED(argc), char **argv)
{
    return 0;
}

int umdClose(struct struct cli_env *env, int UNUSED(argc), char **argv)
{
    return 0;
}

    

#ifdef __cplusplus
}
#endif


