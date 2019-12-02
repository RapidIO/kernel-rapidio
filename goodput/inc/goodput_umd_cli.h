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

int umdThreadCmd(struct cli_env *env, int UNUSED(argc), char **argv);

int umdKillCmd(struct cli_env *env, int UNUSED(argc), char **argv);

int umdStatusCmd(struct cli_env *env, int UNUSED(argc), char **argv);


struct cli_cmd UMDDmaNum =
{
	"umd_dnum",
	8,
	6,
	"Send a specified number of DMA reads/writes",
	"dnum <idx> <did> <rio_addr> <bytes> <acc_sz> <wr> <num>\n"
		"<idx>		is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
		"<did>		target device ID\n"
		"<rio_addr> RapidIO memory address to access\n"
		"<acc_sz>	access size, must be a power of two from 1 to 0xffffffff\n"
		"<wr>		0: Read, 1: Write\n"
		"<num>		number of transactions to send\n",
	umdDmaNumCmd,
	ATTR_NONE
};
	
struct cli_cmd UMDThread =
{
	"umd_thread",
	10,
	2,
	"Start a thread on a cpu",
	"start <idx> <cpu>\n"
		"<idx>	   is a worker index from 0 to " STR(MAX_WORKER_IDX) "\n"
		"<cpu>	   is a cpu number, or -1 to indicate no cpu affinity\n"
	ThreadCmd,
	ATTR_NONE	
};
	
struct cli_cmd UMDKill =
{
	"umd_kill",
	8,
	1,
	"Kill a thread",
	"kill <idx>\n"
		"<idx> is a UMD worker index from 0 to " STR(MAX_WORKER_IDX) ", or \"all\"\n",
	umdKillCmd,
	ATTR_NONE
};

struct cli_cmd UMDStatus =
{
	"umd_status",
	6,
	0,
	"Display status of all threads",
	"status {i|m|g}\n"
		"Optionally enter a character to select the status type:\n"
		"TODO\n"
		"Default is general status\n",
	umdStatusCmd,
	ATTR_RPT
};


#ifdef __cplusplus
}
#endif

#endif /* __GOODPUT_UMD_CLI_H__ */

