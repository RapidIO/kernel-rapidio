
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

#ifndef __DID_H__
#define __DID_H__

#include <stdint.h>
#include <stdbool.h>

#include "rio_route.h"
#include "rio_standard.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	invld_sz = 0, dev08_sz = 8, dev16_sz = 16, dev32_sz = 32,
} did_sz_t;

typedef struct {
	did_val_t value;
	did_sz_t size;
} did_t;

typedef struct {
	did_sz_t size;
	did_val_t base;
	uint16_t next;
	did_sz_t l_dev16[RIO_RT_GRP_SZ];
} did_grp_t;

#define DEV08_IDX 0
#define DEV16_IDX 1
#define DEV32_IDX 2
#define MAX_DEV_SZ_IDX (DEV32_IDX + 1)

#define ANY_ID RIO_LAST_DEV8
#define DID_ANY_DEV8_ID ((did_t) {ANY_ID, dev08_sz})
#define DID_ANY_DEV16_ID ((did_t) {ANY_ID, dev16_sz})
#define DID_ANY_DEV32_ID ((did_t) {ANY_ID, dev32_sz})
#define DID_INVALID_ID ((did_t) {0, invld_sz})
#define DID_ANY_ID(s) ((dev08_sz == s) ? DID_ANY_DEV8_ID : DID_ANY_DEV16_ID)
#define DID_ANY_VAL(s) ((dev08_sz == s) ? RIO_LAST_DEV8 : RIO_LAST_DEV8)

#define DID_DEV_VAL(x) ((did_val_t)(x & 0x00FF))
#define DID_DOM_VAL(x) ((did_val_t)((x & 0xFF00) >> 8))

int did_size_from_int(did_sz_t *size, uint32_t asInt);
int did_size_as_int(did_sz_t size);

int did_create(did_t *did, did_sz_t size);
int did_create_from_data(did_t *did, did_val_t value, did_sz_t size);
int did_get(did_t *did, did_val_t value);
int did_from_value(did_t *did, uint32_t value, uint32_t size);
int did_to_value(did_t did, uint32_t *value, uint32_t *size);
int did_release(did_t did);
int did_not_inuse(did_t did);

int did_alloc_dev16_grp(did_grp_t **group);
int did_grp_resrv_did(did_grp_t *group, did_t *did);
int did_grp_unresrv_did(did_grp_t *group, did_t did);

did_val_t did_get_value(did_t did);
did_sz_t did_get_size(did_t did);
bool did_equal(did_t did, did_t other);

#ifdef __cplusplus
}
#endif

#endif /* __DID_H__ */
