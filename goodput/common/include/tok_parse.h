/*
 ****************************************************************************
 Copyright (c) 2016, Integrated Device Technology Inc.
 Copyright (c) 2016, RapidIO Trade Association
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

#ifndef __TOK_PARSE_H__
#define __TOK_PARSE_H__

#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "rio_route.h"
#include "rio_ecosystem.h"
#include "rio_standard.h"
#include "string_util.h"

#define TOK_ERR_DID_MSG_FMT ("Destination Id must be between 0 and 0xff\n")
#define TOK_ERR_DID_SZ_MSG_FMT ("Destination ID size must be 8, 16, or 32.\n")
#define TOK_ERR_CT_MSG_FMT ("Component tag must be between 0 and 0xffffffff\n")
#define TOK_ERR_HC_MSG_FMT ("Hopcount must be between 0 and 0xff\n")
#define TOK_ERR_MPORT_MSG_FMT ("Mport device index must be between 0 and 7\n")
#define TOK_ERR_LOG_LEVEL_MSG_FMT ("Log level must be between 1 and 7\n")
#define TOK_ERR_SOCKET_MSG_FMT ("%s must be between 0 and 0xffff\n")
#define TOK_ERR_PORT_NUM_MSG_FMT ("Port number must be between 0 and " STR(RIO_MAX_DEV_PORT-1) "\n")

#define TOK_ERR_ULL_HEX_MSG_FMT ("%s must be between 0x0 and 0xffffffffffffffff\n")
#define TOK_ERR_UL_HEX_MSG_FMT ("%s must be between 0x0 and 0xffffffff\n")
#define TOK_ERR_US_HEX_MSG_FMT ("%s must be between 0x0 and 0xffff\n")

#define TOK_ERR_ULONGLONG_MSG_FMT ("%s must be between %" PRIu64 " and %" PRIu64 "\n")
#define TOK_ERR_ULONG_MSG_FMT ("%s must be between %" PRIu32 " and %" PRIu32 "\n")
#define TOK_ERR_USHORT_MSG_FMT ("%s must be between %" PRIu16 " and %" PRIu16 "\n")

#define TOK_ERR_LONGLONG_MSG_FMT ("%s must be between %" PRId64 " and %" PRId64 "\n")
#define TOK_ERR_LONG_MSG_FMT ("%s must be between %" PRId32 " and %" PRId32 "\n")
#define TOK_ERR_SHORT_MSG_FMT ("%s must be between %" PRId16 " and %" PRId16 "\n")

#define TOK_ERR_ULONGLONG_HEX_MSG_FMT ("%s must be between 0x%" PRIx64 " and 0x%" PRIx64 "\n")
#define TOK_ERR_ULONG_HEX_MSG_FMT ("%s must be between 0x%" PRIx32 " and 0x%" PRIx32 "\n")
#define TOK_ERR_USHORT_HEX_MSG_FMT ("%s must be between 0x%" PRIx16 " and 0x%" PRIx16 "\n")

#ifdef __cplusplus
extern "C" {
#endif

int tok_parse_ulonglong(char *token, uint64_t *value, uint64_t min,
		uint64_t max, int base);
int tok_parse_ulong(char *token, uint32_t *value, uint32_t min, uint32_t max,
		int base);
int tok_parse_ushort(char *token, uint16_t *value, uint16_t min, uint16_t max,
		int base);

int tok_parse_longlong(char *token, int64_t *value, int64_t min, int64_t max,
		int base);
int tok_parse_long(char *token, int32_t *value, int32_t min, int32_t max,
		int base);
int tok_parse_short(char *token, int16_t *value, int16_t min, int16_t max,
		int base);

int tok_parse_ull(char *token, uint64_t *value, int base);
int tok_parse_ul(char *token, uint32_t *value, int base);
int tok_parse_us(char *token, uint16_t *value, int base);

int tok_parse_ll(char *token, int64_t *value, int base);
int tok_parse_l(char *token, int32_t *value, int base);
int tok_parse_s(char *token, int16_t *value, int base);
int tok_parse_f(char *token, float *value);

int tok_parse_did(char *token, did_val_t *did, int base);
int tok_parse_ct(char *token, ct_t *ct, int base);
int tok_parse_hc(char *token, hc_t *hc, int base);
int tok_parse_mport_id(char *token, uint32_t *mport_id, int base);
int tok_parse_log_level(char *token, uint32_t *level, int base);
int tok_parse_socket(char *token, uint16_t *socket, int base);
int tok_parse_port_num(char *token, uint32_t *port_num, int base);

#ifdef __cplusplus
}
#endif

#endif /* __TOK_PARSE_H__ */
