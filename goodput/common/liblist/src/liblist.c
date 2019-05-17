/* RSKT List utility, could be replaced with other implementations */
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

#include <stdlib.h>
#include "liblist.h"

#ifdef UNIT_TESTING
#include <stdarg.h>
#include <setjmp.h>
#include "cmocka.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void l_init(struct l_head_t *l)
{
	if (NULL == l) {
		return;
	}

	l->cnt = 0;
	l->head = l->tail = NULL;
}
;

void l_push_tail(struct l_head_t *l, void *item)
{
	struct l_item_t *li;

	if ((NULL == l) || (NULL == item)) {
		return;
	}

	li = (struct l_item_t *)calloc(1, sizeof(struct l_item_t));
	if (NULL == li) {
		return;
	}

	li->key = 0;
	li->item = item;
	li->next = li->prev = NULL;

	if (l->cnt) {
		l->tail->next = li;
		li->prev = l->tail;
		l->tail = li;
	} else {
		l->tail = l->head = li;
	}

	l->cnt++;
}

void *l_pop_head(struct l_head_t *l)
{
	void *li = NULL;
	struct l_item_t *temp;

	if (NULL == l) {
		return NULL;
	}

	if (l->cnt) {
		li = l->head->item;
		if (1 == l->cnt) {
			free(l->head);
			l_init(l);
		} else {
			temp = l->head;
			l->head->next->prev = NULL;
			l->head = l->head->next;
			l->cnt--;
			free(temp);
		}
	}

	return li;
}

struct l_item_t *l_add(struct l_head_t *l, uint32_t key, void *item)
{
	struct l_item_t *new_li;
	struct l_item_t *li;

	if ((NULL == l) || (NULL == item)) {
		return NULL;
	}

	new_li = (struct l_item_t *)calloc(1, sizeof(struct l_item_t));
	if (NULL == new_li) {
		return NULL;
	}

	new_li->next = new_li->prev = NULL;
	new_li->key = key;
	new_li->item = item;

	/* take care of special cases */
	if (!l->cnt) {
		l->head = l->tail = new_li;
		goto exit;
	}

	if (l->tail->key <= key) {
		l->tail->next = new_li;
		new_li->prev = l->tail;
		l->tail = new_li;
		goto exit;
	}

	if (l->head->key > key) {
		/* Insert at head */
		l->head->prev = new_li;
		new_li->next = l->head;
		l->head = new_li;
		goto exit;
	}

	/* No more special cases, now always have an li before and after */
	li = l->head->next;
	while (NULL != li) {
		if (li->key > key) {
			/* Insert before this item */
			li->prev->next = new_li;
			new_li->next = li;
			new_li->prev = li->prev;
			li->prev = new_li;
			break;
		}
		li = li->next;
	}

exit:
	l->cnt++;
	return new_li;
}

void l_lremove(struct l_head_t *l, struct l_item_t *li)
{
	if ((NULL == l) || (NULL == li)) {
		return;
	}

	if (1 == l->cnt) {
		l_init(l);
	} else {
		if (li == l->head) {
			l->head->next->prev = NULL;
			l->head = l->head->next;
		} else {
			if (li == l->tail) {
				l->tail->prev->next = NULL;
				l->tail = l->tail->prev;
			} else {
				li->next->prev = li->prev;
				li->prev->next = li->next;
			}
		}
		l->cnt--;
	}
	free(li);
}

void l_remove(struct l_head_t *l, struct l_item_t *li)
{
	void *l_val;

	if ((NULL == l) || (NULL == li)) {
		return;
	}

	l_val = li->item;
	l_lremove(l, li);
	free(l_val);
}

void *l_find(struct l_head_t *l, uint32_t key, struct l_item_t **l_item)
{
	struct l_item_t *li;
	void *ret = NULL;

	if ((NULL == l) || (NULL == l_item)) {
		return NULL;
	}

	li = l->head;
	*l_item = NULL;
	while (NULL != li) {
		if (li->key == key) {
			*l_item = li;
			ret = (*l_item)->item;
			break;
		}
		li = li->next;
	}
	return ret;
}

int l_size(struct l_head_t *l)
{
	return (l == NULL ? 0 : l->cnt);
}

void *l_head(struct l_head_t *l, struct l_item_t **li)
{
	if ((NULL == li) || (NULL == l)) {
		return NULL;
	}

	*li = l->head;
	return (NULL == *li ? NULL : (*li)->item);
}

void *l_next(struct l_item_t **li)
{
	if ((NULL == li) || (NULL == *li)) {
		return NULL;
	}

	*li = (*li)->next;
	return (NULL == *li ? NULL : (*li)->item);
}

#ifdef __cplusplus
}
#endif

