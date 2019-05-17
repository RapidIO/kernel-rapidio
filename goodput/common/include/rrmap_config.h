/*
 ****************************************************************************
 Copyright (c) 2014, Integrated Device Technology Inc.
 Copyright (c) 2014, RapidIO Trade Association
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

#ifndef __RRMAP_CONFIG_H__
#define __RRMAP_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/** \brief TCP/IP POSIX Socket Numbers, remotely connect to Daemon CLIs */
#define FMD_DFLT_CLI_SKT 2222
#define RDMA_DFLT_CLI_SKT 2224
#define RSKT_DFLT_CLI_SKT 2226
#define FXFR_DFLT_CLI_SKT 2228
#define RSKTS_DFLT_CLI_SKT 2230

/** \brief Location of configuration files and sysfs files for LINUX */
#define FMD_DFLT_CFG_FN "/etc/rapidio/fmd.conf"
#define MEM_CFG_DFLT_FN "/etc/rapidio/rsvd_phys_mem.conf"
#define MEM_CFG_MAX_LINE_LEN 255

#define FMD_DFLT_DEV_DIR "/sys/bus/rapidio/devices/"

/** \brief libmport CM Socket Numbers to connect daemon to daemon */
#define FMD_DFLT_MAST_CM_PORT 3434
#define RDMA_DFLT_DMN_CM_PORT 5544
#define RSKT_DFLT_DMN_CM_PORT 4455

/** \brief FMD configuration options */
#define FMD_DFLT_INIT_DD 0
#define FMD_DFLT_RUN_CONS 1
#define FMD_DFLT_LOG_LEVEL ((RDMA_LL_ERR < RDMA_LL)?RDMA_LL_WARN:RDMA_LL)
#define FMD_DFLT_MAST_INTERVAL 5
#define FMD_DFLT_MAST_DEVID 0xFD

/** \brief File transfer and CM_SOCK demo default CM ports */
#define FXFR_DFLT_SVR_CM_PORT 5555
#define CM_SOCK_DFLT_SVR_CM_PORT 5556

/** \brief RapidIO temporary directory, survives reboots */
#define RRMAP_TEMP_DIR_PATH "/var/tmp/rapidio/"
#define DEFAULT_LOG_DIR RRMAP_TEMP_DIR_PATH

/** \brief File system definitions for FMD */
#define FMD_DFLT_APP_PORT_NUM 9797
#define FMD_APP_MSG_SKT_FMT RRMAP_TEMP_DIR_PATH "FMD%05d"

#define FMD_MAX_LOG_FILE_NAME 100
#define FMD_LOG_FILE_FMT "fmd_%05d_log"

#define FMD_MAX_SHM_FN_LEN 100
#define FMD_DFLT_SHM_DIR "/dev/shm"
#define FMD_DFLT_DD_FN "/RIO_SM_DEV_DIR"
#define FMD_DFLT_DD_MTX_FN "/RIO_SM_DEV_DIR_MUTEX"

#ifdef __cplusplus
}

#endif

#endif /* __RRMAP_CONFIG_H__ */
