/*
****************************************************************************
Copyright (c) 2015, Integrated Device Technology Inc.
Copyright (c) 2015, RapidIO Trade Association
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

#ifndef __PW_HANDLING_H__
#define __PW_HANDLING_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "rio_route.h"
#include "rio_mport_lib.h"
#include "libtime_utils.h"
#include "RapidIO_Device_Access_Routines_API.h"
#include "RapidIO_Error_Management_API.h"
#include "RapidIO_Port_Config_API.h"
#include "liblist.h"
#include "worker.h"
#ifdef __cplusplus
extern "C" {
#endif

struct librio_status {
        rio_pc_get_config_out_t         pc;
        rio_pc_get_status_out_t         ps;
        rio_rt_state_t                  g_rt;
        rio_em_cfg_pw_t                 em_pw_cfg;
        rio_em_dev_rpt_ctl_out_t        em_notfn;
};

// Switch offset and device/destination ID is hard coded.
// This can be easily extended to support multiple switches
// by assigning destID and hopcount to an allocated strucure that
// is referenced by the d_info->acc_info pointer

#define SWITCH_DEVID 0
#define SWITCH_COMPTAG 0x80000000
#define SWITCH_HOPCOUNT 0
#define PW_DID 0x20
#define MAX_PW_EVENTS 10
#define DID_TO_PORT(x) (x - 1)
#define PORT_TO_DID(x) (x + 1)

uint32_t tsi721_init_no_kernel_scan(struct worker *info);
void do_tsi721_fault_ins(struct worker *info);
void do_tsi721_handle_link_init(struct worker *info);
void do_tsi721_pw_rx(struct worker *info);
void do_tsi721_handle_tsi_pw(struct worker *info);
void do_tsi721_handle_cps_pw(struct worker *info);
void do_tsi721_manage_switch(struct worker *info);
void do_cps_poll_for_pw(struct worker *info);
void do_tsi721_tsi721_recovery(struct worker *info);
uint32_t reset_cps_port(DAR_DEV_INFO_t *dev_h, uint32_t port);
uint32_t init_switch_handle(DAR_DEV_INFO_t *dev_h);
uint32_t set_switch_port_enables(DAR_DEV_INFO_t *dev_h,
                        struct librio_status *sw_h);
uint32_t handle_switch_port_write_event(DAR_DEV_INFO_t *dev_h,
                                        rio_port_t pw_port,
                                        rio_em_events_t ev);


#ifdef __cplusplus
}
#endif

#endif /* __PW_HANDLING_H__ */
