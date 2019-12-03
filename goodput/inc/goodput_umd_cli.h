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

#ifndef __GOODPUT_UMD_CLI_H__
#define __GOODPUT_UMD_CLI_H__

#ifdef __cplusplus
extern "C" {
#endif

int umdDmaNumCmd(struct cli_env *env, int UNUSED(argc), char **argv);

int umdOpen(struct cli_env *env, int UNUSED(argc), char **argv);

int umdConfig(struct cli_env *env, int UNUSED(argc), char **argv);

int umdStart(struct cli_env *env, int UNUSED(argc), char **argv);

int umdStop(struct cli_env *env, int UNUSED(argc), char **argv);

int umdClose(struct cli_env *env, int UNUSED(argc), char **argv);


struct cli_cmd UMDDmaNum =
{
    "umd_dnum",
    8,
    6,
    "Send a specified number of DMA reads/writes",
    "umd_dnum <did> <rio_addr> <bytes> <acc_sz> <wr> <num>\n"
        "<idx>      UMD channel index: 0 to Maximum channel - 1\n"
        "<did>      target device ID\n"
        "<rio_addr> RapidIO memory address to access\n"
        "<buf_sz>   buffer size, must be a power of two from 1 to 0xffffffff\n"
        "<wr>       0: Read, 1: Write,2:Ramdom Writer\n"
        "<num>      Optional default is 0, number of transactions to send. 0 indicates infinite loop\n"
        "<data>   RND, or constant data value, written every 8 bytes",
    umdDmaNumCmd,
    ATTR_NONE
};
    
struct cli_cmd UMDOpen =
{
    "umd_open",
    8,
    1,
    "Reserve a UMD engine",
    "umd_open <idx> <mport_num>\n"
        "<idx> UMD engine array index"
        "<mport_num> optional local rapidio device port. Defalut is 0. \n",
    umdOpen,
    ATTR_NONE   
};

struct cli_cmd UMDConfig =
{

    "umd_config",
    8,
    1,
    "Configure a UMD engine",
    "umd_config <idx> <size> <addr> <RSVD>\n"
        "<idx> UMD engine array index. \n"
        "<size> inbound window size. Must be a power of two from 0x1000 to 0x01000000\n"
        "<addr> is the optional RapidIO address for the inbound window\n"
        "       NOTE: <addr> must be aligned to <size>\n"
        "<RSVD> is a keyword for reserved memory area\n"
        "       NOTE: If <RSVD> is specified, <addr> is ignored\n",
    umdConfig,
    ATTR_NONE   

};

struct cli_cmd UMDStart =
{
    "umd_start",
    8,
    1,
    "Start a UMD engine",
    "umd_start <idx>\n"
        "<idx> UMD engine array index. \n",
    umdStart,
    ATTR_NONE   

};

struct cli_cmd UMDStop =
{
    "umd_stop",
    8,
    1,
    "Stop a UMD engine",
    "umd_stop <idx>\n"
        "<idx> UMD engine array index. \n",
    umdStop,
    ATTR_NONE   

};

struct cli_cmd UMDClose =
{
    "umd_cloe",
    8,
    1,
    "Free a UMD engine",
    "umd_close <idx>\n"
        "<idx> UMD engine array index. \n",
    umdClose,
    ATTR_NONE   
};

    

#ifdef __cplusplus
}
#endif

#endif /* __GOODPUT_UMD_CLI_H__ */

