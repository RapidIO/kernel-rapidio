/*
 ****************************************************************************
 Copyright (c) 2015, Integrated Device Technology Inc.
 Copyright (c) 2015, RapidIO Trade Association
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 l of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this l of conditions and the following disclaimer in the documentation
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

#ifndef __LIBLIST_H__
#define __LIBLIST_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_item_t {
	struct l_item_t *next;
	struct l_item_t *prev;
	void *item;
	uint64_t key;
};

struct l_head_t {
	int cnt;
	struct l_item_t *head;
	struct l_item_t *tail;
};

void l_init(struct l_head_t *l);
void l_push_tail(struct l_head_t *l, void *item);
void *l_pop_head(struct l_head_t *l);
struct l_item_t *l_add(struct l_head_t *l, uint32_t key, void *item);
void l_remove(struct l_head_t *l, struct l_item_t *l_item);
void l_lremove(struct l_head_t *l, struct l_item_t *l_item);
void *l_find(struct l_head_t *l, uint32_t key, struct l_item_t **l_item);
int l_size(struct l_head_t *l);
void *l_head(struct l_head_t *l, struct l_item_t **l_item);
void *l_next(struct l_item_t **l_item);

#ifdef __cplusplus
}
#endif

#endif /* __LIBLIST_H__ */
