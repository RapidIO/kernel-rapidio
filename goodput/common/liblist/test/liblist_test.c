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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <stdarg.h>
#include <setjmp.h>
#include "cmocka.h"

#include "liblist.h"

#ifdef __cplusplus
extern "C" {
#endif

// frees the allocated item memory
static inline void free_list_items(struct l_head_t *list)
{
	struct l_item_t *current = list->head;
	while (NULL != current) {
		struct l_item_t *next = current->next;
		free(current);
		list->cnt--;
		current = next;
	}

	assert_int_equal(0, list->cnt);
}

static void assumptions(void **state)
{
	// although it appears a little bogus, duplicate the two internal
	// structures for testing. Ensure they remain the same and other
	// assumptions in subsequent tests are valid
	struct dup_l_item_t {
		struct l_item_t *next;
		struct l_item_t *prev;
		uint32_t key;
		void *item;
	};
	assert_int_equal(sizeof(struct dup_l_item_t), sizeof(struct l_item_t));

	struct dup_l_head_t {
		int cnt;
		struct l_item_t *head;
		struct l_item_t *tail;
	};
	assert_int_equal(sizeof(struct dup_l_head_t), sizeof(struct l_head_t));

	(void)state; // unused
}

static void l_init_null_parm_test(void **state)
{
	l_init(NULL); // returns silently

	(void)state; // unused
}

static void l_init_initial_state_test(void **state)
{
	struct l_head_t list;
	struct l_item_t head, tail;

	// manually provide non default values
	list.cnt = 3;
	list.head = &head;
	list.tail = &tail;
	assert_int_equal(3, list.cnt);
	assert_non_null(list.head);
	assert_non_null(list.tail);
	assert_ptr_not_equal(list.head, list.tail);

	// ensure structure returned is initialized
	l_init(&list);
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	(void)state; // unused
}

static void l_init_initial_memory_test(void **state)
{
	struct l_head_t *list;
	struct l_item_t *head, *tail;

	list = (struct l_head_t *)calloc(1, sizeof(struct l_head_t));
	assert_non_null(list);

	head = (struct l_item_t *)calloc(1, sizeof(struct l_item_t));
	assert_non_null(head);

	tail = (struct l_item_t *)calloc(1, sizeof(struct l_item_t));
	assert_non_null(tail);
	assert_ptr_not_equal(head, tail);

	// manually provide non default values
	list->cnt = 3;
	list->head = head;
	list->tail = tail;
	assert_int_equal(3, list->cnt);
	assert_non_null(list->head);
	assert_non_null(list->tail);

	// ensure structure returned is initialized
	l_init(list);
	assert_int_equal(0, list->cnt);
	assert_null(list->head);
	assert_null(list->tail);

	free(tail);
	free(head);
	free(list);

	(void)state; // unused
}

static void l_push_tail_null_parm_test(void **state)
{
	struct l_head_t list;
	struct l_item_t item;

	// ensure structure returned is initialized
	l_init(&list);
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	// null list
	l_push_tail(NULL, &item); // returns silently
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	// null item
	l_push_tail(&list, NULL); // returns silently
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	// null list and null item
	l_push_tail(NULL, NULL); // returns silently
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	(void)state; // unused
}

static void l_push_tail_test(void **state)
{
	const int max_items = 10;
	struct l_head_t list;
	int items[max_items];
	struct l_item_t *addr[max_items];
	int i;

	// ensure structure returned is initialized
	l_init(&list);
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	for (i = 0; i < max_items; i++) {
		items[i] = i;
	}

	// push first item
	l_push_tail(&list, &items[0]);
	assert_int_equal(1, list.cnt);
	assert_non_null(list.head);
	assert_ptr_equal(list.tail, list.head);

	assert_true(0 == list.head->key);
	assert_ptr_equal(&items[0], list.head->item);
	assert_null(list.head->next);
	assert_null(list.head->prev);
	addr[0] = list.tail;

	// add all items
	for (i = 1; i < max_items; i++) {
		l_push_tail(&list, &items[i]);
		assert_int_equal(i + 1, list.cnt);
		addr[i] = list.tail;
	}
	assert_int_equal(max_items, list.cnt);
	assert_ptr_equal(addr[0], list.head);
	assert_ptr_equal(addr[max_items - 1], list.tail);

	// walk the structure
	struct l_item_t *current = list.head->next;
	for (i = 1; i < max_items - 1; i++) {
		assert_ptr_equal(addr[i + 1], current->next);
		assert_ptr_equal(addr[i - 1], current->prev);
		assert_ptr_equal(&items[i], current->item);
		assert_true(0 == current->key);
		assert_int_equal(i, *(int * )current->item);
		current = current->next;
	}

	// should be at the tail right now
	assert_null(current->next);
	assert_ptr_equal(addr[max_items - 2], current->prev);
	assert_ptr_equal(&items[max_items - 1], current->item);
	assert_true(0 == current->key);
	assert_int_equal(max_items - 1, *(int * )current->item);

	current = list.head;
	assert_ptr_equal(addr[1], current->next);
	assert_null(current->prev);
	assert_ptr_equal(&items[0], current->item);
	assert_true(0 == current->key);
	assert_int_equal(0, *(int * )current->item);

	// free allocated memory
	free_list_items(&list);

	(void)state; // unused
}

static void l_pop_head_null_parm_test(void **state)
{
	assert_null(l_pop_head(NULL));

	(void)state; // unused
}

static void l_pop_head_test(void **state)
{
	const int max_items = 15;
	struct l_head_t list;
	int items[max_items];
	struct l_item_t *addr[max_items];
	int i;

	// ensure structure returned is initialized
	l_init(&list);
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	// remove item from empty list, should be null
	struct l_item_t *current = addr[0];
	current = (struct l_item_t*)l_pop_head(&list);
	assert_null(current);

	// ensure list is still null
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	for (i = 0; i < max_items; i++) {
		items[i] = i;
	}

	// push one item
	l_push_tail(&list, &items[0]);
	assert_int_equal(1, list.cnt);
	assert_non_null(list.head);
	assert_ptr_equal(list.tail, list.head);

	// pop it off
	current = NULL;
	current = (struct l_item_t*)l_pop_head(&list);
	assert_non_null(current);

	// list empty
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	// push all items
	for (i = 0; i < max_items; i++) {
		l_push_tail(&list, &items[i]);
		assert_int_equal(i + 1, list.cnt);
		addr[i] = list.tail;
	}
	assert_int_equal(max_items, list.cnt);
	assert_ptr_equal(addr[0], list.head);
	assert_ptr_equal(addr[max_items - 1], list.tail);

	// pop them off - frees list item
	current = NULL;
	for (i = 0; i < max_items - 1; i++) {
		current = (struct l_item_t*)l_pop_head(&list);
		assert_int_equal(max_items - (i + 1), list.cnt);
		assert_ptr_equal(addr[i + 1], list.head);
		assert_ptr_equal(addr[max_items - 1], list.tail);
	}
	assert_int_equal(1, list.cnt);
	assert_ptr_equal(addr[max_items - 1], list.head);

	current = (struct l_item_t*)l_pop_head(&list);
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	(void)state; // unused
}

static void l_add_null_parm_test(void **state)
{
	struct l_head_t list;
	struct l_item_t item;

	// null list
	assert_null(l_add(NULL, 1, &item));

	// null item
	l_init(&list);
	assert_null(l_add(&list, 1, NULL));
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	(void)state; // unused
}

static void l_add_equal_test(void **state)
{
	// all keys have same value
	const int max_items = 26;
	struct l_head_t list;
	int items[max_items];
	struct l_item_t *addr[max_items];
	int i;

	// ensure structure returned is initialized
	l_init(&list);
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	for (i = 0; i < max_items; i++) {
		items[i] = i;
	}

	// add first item
	l_add(&list, 0, &items[0]);
	assert_int_equal(1, list.cnt);
	assert_non_null(list.head);
	assert_ptr_equal(list.tail, list.head);

	assert_true(0 == list.head->key);
	assert_ptr_equal(&items[0], list.head->item);
	assert_null(list.head->next);
	assert_null(list.head->prev);
	addr[0] = list.tail;

	// add all items
	for (i = 1; i < max_items; i++) {
		l_add(&list, 0, &items[i]);
		addr[i] = list.tail;
	}
	assert_int_equal(max_items, list.cnt);
	assert_ptr_equal(addr[0], list.head);
	assert_ptr_equal(addr[max_items - 1], list.tail);

	// walk the structure
	struct l_item_t *current = list.head->next;
	for (i = 1; i < max_items - 1; i++) {
		assert_ptr_equal(addr[i - 1], current->prev);
		assert_ptr_equal(addr[i], current);
		assert_ptr_equal(addr[i + 1], current->next);
		assert_ptr_equal(&items[i], current->item);
		assert_true(0 == current->key);
		assert_int_equal(i, *(int * )current->item);
		current = current->next;
	}

	// should be at the tail right now
	assert_ptr_equal(addr[max_items - 2], current->prev);
	assert_ptr_equal(addr[max_items - 1], current);
	assert_null(current->next);
	assert_ptr_equal(&items[max_items - 1], current->item);
	assert_true(0 == current->key);
	assert_int_equal(max_items - 1, *(int * )current->item);

	// and the head
	current = list.head;
	assert_null(current->prev);
	assert_ptr_equal(addr[0], current);
	assert_ptr_equal(addr[1], current->next);
	assert_ptr_equal(&items[0], current->item);
	assert_true(0 == current->key);
	assert_int_equal(0, *(int * )current->item);

	// free allocated memory
	free_list_items(&list);

	(void)state; // unused
}

static void l_add_greater_test(void **state)
{
	// add values with keys in ascending order - tail enqueue
	const int max_items = 13;
	struct l_head_t list;
	int items[max_items];
	struct l_item_t *addr[max_items];
	int i;

	// ensure structure returned is initialized
	l_init(&list);
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	for (i = 0; i < max_items; i++) {
		items[i] = i;
	}

	// add first item
	l_add(&list, 0, &items[0]);
	assert_int_equal(1, list.cnt);
	assert_non_null(list.head);
	assert_ptr_equal(list.tail, list.head);

	assert_true(0 == list.head->key);
	assert_ptr_equal(&items[0], list.head->item);
	assert_null(list.head->next);
	assert_null(list.head->prev);
	addr[0] = list.tail;

	// add all items
	for (i = 1; i < max_items; i++) {
		l_add(&list, i, &items[i]);
		addr[i] = list.tail;
	}
	assert_int_equal(max_items, list.cnt);
	assert_ptr_equal(addr[0], list.head);
	assert_ptr_equal(addr[max_items - 1], list.tail);

	// walk the structure
	struct l_item_t *current = list.head->next;
	for (i = 1; i < max_items - 1; i++) {
		assert_ptr_equal(addr[i - 1], current->prev);
		assert_ptr_equal(addr[i], current);
		assert_ptr_equal(addr[i + 1], current->next);
		assert_ptr_equal(&items[i], current->item);
		assert_int_equal(i, current->key);
		assert_int_equal(i, *(int * )current->item);
		current = current->next;
	}

	// should be at the tail right now
	assert_ptr_equal(addr[max_items - 2], current->prev);
	assert_ptr_equal(addr[max_items - 1], current);
	assert_null(current->next);
	assert_ptr_equal(&items[max_items - 1], current->item);
	assert_int_equal(max_items - 1, current->key);
	assert_int_equal(max_items - 1, *(int * )current->item);

	// and the head
	current = list.head;
	assert_null(current->prev);
	assert_ptr_equal(addr[0], current);
	assert_ptr_equal(addr[1], current->next);
	assert_ptr_equal(&items[0], current->item);
	assert_true(0 == current->key);
	assert_int_equal(0, *(int * )current->item);

	// free allocated memory
	free_list_items(&list);

	(void)state; // unused
}

static void l_add_lessthan_test(void **state)
{
	// add values with keys in descending order - head enqueue
	const int max_items = 7;
	struct l_head_t list;
	int items[max_items];
	struct l_item_t *addr[max_items];
	int i;

	// ensure structure returned is initialized
	l_init(&list);
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	for (i = max_items - 1; i >= 0; i--) {
		items[i] = i;
	}

	// add first item
	l_add(&list, 100, &items[0]);
	assert_int_equal(1, list.cnt);
	assert_non_null(list.head);
	assert_ptr_equal(list.tail, list.head);

	assert_true(100 == list.head->key);
	assert_ptr_equal(&items[0], list.head->item);
	assert_null(list.head->next);
	assert_null(list.head->prev);
	addr[max_items - 1] = list.head;

	// add all items
	for (i = 1; i < max_items; i++) {
		l_add(&list, 100 - i, &items[i]);
		addr[max_items - 1 - i] = list.head;
	}
	assert_int_equal(max_items, list.cnt);
	assert_ptr_equal(addr[max_items - 1], list.tail);
	assert_ptr_equal(addr[0], list.head);

	// walk the structure
	struct l_item_t *current = list.head->next;
	unsigned int key_value = 100 - max_items + 2;
	for (i = 1; i < max_items - 1; i++) {
		assert_ptr_equal(addr[i - 1], current->prev);
		assert_ptr_equal(addr[i], current);
		assert_ptr_equal(addr[i + 1], current->next);
		assert_ptr_equal(&items[max_items - 1 - i], current->item);
		assert_true(key_value == current->key);
		assert_int_equal(max_items - 1 - i, *(int * )current->item);
		key_value++;
		current = current->next;
	}

	// should be at the tail right now
	assert_ptr_equal(addr[max_items - 2], current->prev);
	assert_ptr_equal(addr[max_items - 1], current);
	assert_null(current->next);
	assert_ptr_equal(&items[0], current->item);
	assert_true(100 == current->key);
	assert_int_equal(0, *(int * )current->item);

	// and the head
	current = list.head;
	assert_null(current->prev);
	assert_ptr_equal(addr[0], current);
	assert_ptr_equal(addr[1], current->next);
	assert_ptr_equal(&items[max_items - 1], current->item);
	assert_true((100 - max_items + 1) == current->key);
	assert_int_equal(max_items - 1, *(int * )current->item);

	// free allocated memory
	free_list_items(&list);

	(void)state; // unused
}

static void l_add_test(void **state)
{
	// keys jump around
	const int max_items = 5;
	struct l_head_t list;
	unsigned int items[] = {1, 3, 5, 7, 11};
	int order[] = {3, 0, 4, 1, 2};
	int i;

	// ensure structure returned is initialized
	l_init(&list);
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	// add items as per order[]
	for (i = 0; i < max_items; i++) {
		l_add(&list, items[order[i]], &items[order[i]]);
	}

	assert_int_equal(max_items, list.cnt);
	assert_null(list.head->prev);
	assert_non_null(list.head->next);
	assert_non_null(list.tail->prev);
	assert_null(list.tail->next);

	// walk the structure
	struct l_item_t *current = list.head;
	for (i = 0; i < max_items; i++) {
		assert_ptr_equal(&items[i], current->item);
		assert_true(items[i] == current->key);
		assert_int_equal(items[i], *(int * )current->item);
		current = current->next;
	}

	// free allocated memory
	free_list_items(&list);

	(void)state; // unused
}

static void l_remove_null_parm_test(void **state)
{
	struct l_head_t list;
	struct l_item_t *item;
	struct l_item_t *delMe;
	int *value;

	value = (int *)calloc(1, sizeof(int));
	item = (struct l_item_t *)calloc(1, sizeof(struct l_item_t));
	delMe = item;
	*value = 0xdead;

	item->key = 123;
	item->next = item;
	item->prev = item;
	item->item = value;

	// null list
	l_remove(NULL, item);
	assert_int_equal(0xdead, *value);
	assert_true(123 == item->key);
	assert_ptr_equal(item, item->next);
	assert_ptr_equal(item, item->prev);
	assert_ptr_equal(value, item->item);

	// add an item to the list
	l_init(&list);
	item = l_add(&list, 0, value);
	assert_int_equal(1, list.cnt);
	assert_int_equal(0xdead, *value);
	assert_non_null(item);
	assert_ptr_not_equal(delMe, item);
	assert_true(123 == delMe->key);
	assert_ptr_equal(delMe, delMe->next);
	assert_ptr_equal(delMe, delMe->prev);
	assert_ptr_equal(value, delMe->item);
	free(delMe);

	// null item
	l_remove(&list, NULL);
	assert_int_equal(1, list.cnt);
	assert_int_equal(0xdead, *value);
	assert_non_null(item);

	// both
	l_remove(NULL, NULL);
	assert_int_equal(1, list.cnt);
	assert_int_equal(0xdead, *value);
	assert_non_null(item);

	// remove the item - else a memory leak
	l_remove(&list, item);
	assert_int_equal(0, list.cnt);
	assert_non_null(item);
	assert_non_null(value);

	(void)state; // unused
}

static void l_remove_test(void **state)
{
	const int max_items = 5;
	struct l_head_t list;
	struct l_head_t *items[max_items];
	struct l_item_t *addr[max_items];
	struct l_item_t *current;
	struct l_head_t *removed;
	int i, count;

	for (i = 0; i < max_items; i++) {
		items[i] = (struct l_head_t *)calloc(1,
				sizeof(struct l_head_t));
		items[i]->cnt = i;
	}

	// add all items
	l_init(&list);
	for (i = 0; i < max_items; i++) {
		l_push_tail(&list, items[i]);
		assert_int_equal(i + 1, list.cnt);
		addr[i] = list.tail;
	}

	// walk the structure
	current = list.head;
	for (i = 0; i < max_items; i++) {
		assert_ptr_equal(items[i], current->item);
		assert_int_equal(i, items[i]->cnt);
		assert_true(0 == current->key);
		current = current->next;
	}

	// remove first item
	count = list.cnt;
	removed = (struct l_head_t*)addr[0]->item;
	l_remove(&list, addr[0]);
	assert_int_equal(--count, list.cnt);

	current = list.head;
	for (i = 1; i < max_items; i++) {
		assert_ptr_not_equal(removed, current->item);
		assert_ptr_equal(items[i], current->item);
		assert_int_equal(i, items[i]->cnt);
		assert_true(0 == current->key);
		current = current->next;
	}
	assert_null(list.tail->next);

	// remove last item
	removed = (struct l_head_t *)addr[max_items - 1]->item;
	l_remove(&list, addr[max_items - 1]);
	assert_int_equal(--count, list.cnt);

	current = list.head;
	for (i = 1; i < max_items - 1; i++) {
		assert_ptr_not_equal(removed, current->item);
		assert_ptr_equal(items[i], current->item);
		assert_int_equal(i, items[i]->cnt);
		assert_true(0 == current->key);
		current = current->next;
	}
	assert_null(list.tail->next);

	// remove middle item
	removed = (struct l_head_t *)addr[2]->item;
	l_remove(&list, addr[2]);
	assert_int_equal(--count, list.cnt);
	assert_null(list.tail->next);

	current = list.head;
	for (i = 1; i < max_items - 1; i += 2) {
		assert_ptr_not_equal(removed, current->item);
		assert_ptr_equal(items[i], current->item);
		assert_int_equal(i, items[i]->cnt);
		assert_true(0 == current->key);
		current = current->next;
	}
	assert_null(list.tail->next);

	// free allocated memory
	free_list_items(&list);
	items[0] = items[2] = items[4] = NULL;
	for (i = 0; i < max_items; i++) {
		free(items[i]);
	}

	(void)state; // unused
}

static void l_lremove_null_parm_test(void **state)
{
	struct l_head_t list;
	struct l_item_t *item;
	struct l_item_t *delMe;
	int *value;

	value = (int *)calloc(1, sizeof(int));
	item = (struct l_item_t *)calloc(1, sizeof(struct l_item_t));
	delMe = item;
	*value = 0xdead;

	item->key = 123;
	item->next = item;
	item->prev = item;
	item->item = value;

	// null list
	l_lremove(NULL, item);
	assert_int_equal(0xdead, *value);
	assert_true(123 == item->key);
	assert_ptr_equal(item, item->next);
	assert_ptr_equal(item, item->prev);
	assert_ptr_equal(value, item->item);

	// add an item to the list
	l_init(&list);
	item = l_add(&list, 0, value);
	assert_int_equal(1, list.cnt);
	assert_int_equal(0xdead, *value);
	assert_non_null(item);
	assert_ptr_not_equal(delMe, item);
	assert_true(123 == delMe->key);
	assert_ptr_equal(delMe, delMe->next);
	assert_ptr_equal(delMe, delMe->prev);
	assert_ptr_equal(value, delMe->item);
	free(delMe);

	// null item
	l_lremove(&list, NULL);
	assert_int_equal(1, list.cnt);
	assert_int_equal(0xdead, *value);
	assert_non_null(item);

	// both
	l_lremove(NULL, NULL);
	assert_int_equal(1, list.cnt);
	assert_int_equal(0xdead, *value);
	assert_non_null(item);

	// remove the item - else a memory leak
	l_lremove(&list, item);
	assert_int_equal(0, list.cnt);
	assert_non_null(value);
	assert_non_null(item);

	free(value);
	(void)state; // unused
}

static void l_lremove_test(void **state)
{
	const int max_items = 5;
	struct l_head_t list;
	struct l_head_t *items[max_items];
	struct l_item_t *addr[max_items];
	int i;

	for (i = 0; i < max_items; i++) {
		items[i] = (struct l_head_t *)calloc(1,
				sizeof(struct l_head_t));
		items[i]->cnt = i;
	}

	// add all items
	l_init(&list);
	for (i = 0; i < max_items; i++) {
		l_push_tail(&list, items[i]);
		assert_int_equal(i + 1, list.cnt);
		addr[i] = list.tail;
	}

	// walk the structure
	struct l_item_t *current = list.head;
	for (i = 0; i < max_items; i++) {
		assert_ptr_equal(items[i], current->item);
		assert_int_equal(i, items[i]->cnt);
		assert_true(0 == current->key);
		current = current->next;
	}

	// remove first item
	int count = list.cnt;
	l_lremove(&list, addr[0]);
	assert_int_equal(--count, list.cnt);

	current = list.head;
	for (i = 1; i < max_items; i++) {
		assert_ptr_equal(items[i], current->item);
		assert_int_equal(i, items[i]->cnt);
		assert_true(0 == current->key);
		current = current->next;
	}
	assert_null(list.tail->next);

	// remove last item
	l_lremove(&list, addr[max_items - 1]);
	assert_int_equal(--count, list.cnt);

	current = list.head;
	for (i = 1; i < max_items - 1; i++) {
		assert_ptr_equal(items[i], current->item);
		assert_int_equal(i, items[i]->cnt);
		assert_true(0 == current->key);
		current = current->next;
	}
	assert_null(list.tail->next);

	// remove middle item
	l_lremove(&list, addr[2]);
	assert_int_equal(--count, list.cnt);
	assert_null(list.tail->next);

	current = list.head;
	for (i = 1; i < max_items - 1; i += 2) {
		assert_ptr_equal(items[i], current->item);
		assert_int_equal(i, items[i]->cnt);
		assert_true(0 == current->key);
		current = current->next;
	}
	assert_null(list.tail->next);

	// free allocated memory
	free_list_items(&list);
	for (i = 0; i < max_items; i++) {
		free(items[i]);
	}

	(void)state; // unused
}

static void l_find_null_parm_test(void **state)
{
	int anInt;
	struct l_head_t list;
	struct l_item_t *item;
	void *found;

	// null list
	found = &anInt;
	assert_non_null(found);
	found = l_find(NULL, 1, &item);
	assert_null(found);

	// null item
	found = &list;
	assert_non_null(found);
	found = l_find(&list, 1, NULL);
	assert_null(found);

	// null list and item
	found = &list;
	assert_non_null(found);
	found = l_find(NULL, 1, NULL);
	assert_null(found);

	(void)state; // unused
}

static void l_find_default_test(void **state)
{
	// find based on key (from head)
	// note there is nothing guaranteeing that key is unique
	const int max_items = 7;
	struct l_head_t list;
	struct l_item_t *item;
	int items[max_items];
	int *found;
	int i;

	for (i = 0; i < max_items; i++) {
		items[i] = i;
	}

	// ensure structure returned is initialized
	l_init(&list);
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	// empty list
	item = NULL;
	found = &items[0];
	found = (int *)l_find(&list, 0, &item);
	assert_null(found);

	// add all items
	for (i = 0; i < max_items; i++) {
		l_push_tail(&list, &items[i]);
		assert_int_equal(i + 1, list.cnt);
	}
	assert_int_equal(max_items, list.cnt);

	// find an item - will be same one over and over again
	item = NULL;
	found = NULL;
	for (i = 0; i < max_items; i++) {
		found = (int *)l_find(&list, 0, &item);

		assert_non_null(found);
		assert_non_null(item);
		assert_int_equal(max_items, list.cnt);
		assert_int_equal(0, *found);
		assert_ptr_equal(&items[0], item->item);
		item = NULL;
		found = NULL;
	}

	// item that isn't there
	found = (int *)l_find(&list, 1, &item);
	assert_null(found);
	assert_null(item);

	// free allocated memory
	free_list_items(&list);

	(void)state; // unused
}

static void l_find_duplicates_test(void **state)
{
	// find based on key (from head)
	// note there is nothing guaranteeing that key is unique
	const int number_keys = 3;
	const int max_items = number_keys * 2;
	struct l_head_t list;
	struct l_item_t *item;
	int items[max_items];
	int keys[] = {1, 2, 3};
	int *found;
	int i;

	for (i = 0; i < max_items; i++) {
		items[i] = i;
	}

	// ensure structure returned is initialized
	l_init(&list);
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	// add all items
	for (i = 0; i < number_keys; i++) {
		l_add(&list, keys[i], &items[i]);
		assert_int_equal(i + 1, list.cnt);
	}
	for (i = 0; i < number_keys; i++) {
		l_add(&list, keys[i], &items[number_keys + i]);
		assert_int_equal(number_keys + i + 1, list.cnt);
	}
	assert_int_equal(max_items, list.cnt);

	// find an item - will be first one
	item = NULL;
	found = NULL;
	for (i = 0; i < number_keys; i++) {
		found = (int *)l_find(&list, keys[i], &item);

		assert_non_null(found);
		assert_non_null(item);
		assert_int_equal(max_items, list.cnt);
		assert_int_equal(items[i], *found);
		assert_ptr_equal(&items[i], item->item);
		item = NULL;
		found = NULL;
	}

	// item that isn't there
	found = (int *)l_find(&list, 0, &item);
	assert_null(found);
	assert_null(item);

	// free allocated memory
	free_list_items(&list);

	(void)state; // unused
}

static void l_find_unique_test(void **state)
{
	// find based on key (from head)
	// note there is nothing guaranteeing that key is unique
	const int max_items = 7;
	struct l_head_t list;
	struct l_item_t *item;
	int items[max_items];
	int *found;
	int i;

	for (i = 0; i < max_items; i++) {
		items[i] = i;
	}

	// ensure structure returned is initialized
	l_init(&list);
	assert_int_equal(0, list.cnt);
	assert_null(list.head);
	assert_null(list.tail);

	// add all items
	for (i = 0; i < max_items; i++) {
		l_add(&list, i, &items[i]);
		assert_int_equal(i + 1, list.cnt);
	}
	assert_int_equal(max_items, list.cnt);

	// find an item - will be same one over and over again
	item = NULL;
	found = NULL;
	for (i = 0; i < max_items; i++) {
		found = (int *)l_find(&list, 0, &item);

		assert_non_null(found);
		assert_non_null(item);
		assert_int_equal(max_items, list.cnt);
		assert_int_equal(0, *found);
		assert_ptr_equal(&items[0], item->item);
		item = NULL;
		found = NULL;
	}

	// item that isn't there
	found = (int *)l_find(&list, max_items + 1, &item);
	assert_null(found);
	assert_null(item);

	// free allocated memory
	free_list_items(&list);

	(void)state; // unused
}

static void l_size_null_parm_test(void **state)
{
	assert_int_equal(0, l_size(NULL));

	(void)state; // unused
}

static void l_size_test(void **state)
{
	const int max_items = 11;
	int items[max_items];
	struct l_head_t *list;
	int i;

	list = (struct l_head_t *)calloc(1, sizeof(struct l_head_t));

	// empty list
	l_init(list);
	assert_int_equal(0, list->cnt);
	assert_int_equal(0, l_size(list));

	// push items
	for (i = 0; i < max_items; i++) {
		l_push_tail(list, &items[i]);
		assert_int_equal(i + 1, list->cnt);
	}
	assert_int_equal(max_items, list->cnt);

	// free allocated memory
	free_list_items(list);
	free(list);

	(void)state; // unused
}

static void l_head_null_parm_test(void **state)
{
	int anInt;
	struct l_head_t list;
	struct l_item_t *item;
	void *found;

	// null list
	found = &anInt;
	assert_non_null(found);
	found = l_head(NULL, &item);
	assert_null(found);

	// null item
	found = &list;
	assert_non_null(found);
	found = l_head(&list, NULL);
	assert_null(found);

	// null list and item
	found = &list;
	assert_non_null(found);
	found = l_head(NULL, NULL);
	assert_null(found);

	(void)state; // unused
}

static void l_head_empty_list_test(void **state)
{
	struct l_head_t list;
	struct l_item_t *item;

	l_init(&list);
	void *found = &list;

	assert_non_null(found);
	found = l_head(&list, &item);
	assert_null(found);
	assert_null(item);

	(void)state; // unused
}

static void l_head_test(void **state)
{
	const int max_items = 3;
	struct l_head_t list;
	struct l_item_t *item;
	int *found;
	int items[max_items];
	struct l_item_t *addr[max_items];
	int i;

	for (i = 0; i < max_items; i++) {
		items[i] = i;
	}

	// push all items
	l_init(&list);
	for (i = 0; i < max_items; i++) {
		l_push_tail(&list, &items[i]);
		assert_int_equal(i + 1, list.cnt);
		addr[i] = list.tail;
	}
	assert_int_equal(max_items, list.cnt);
	assert_ptr_equal(addr[0], list.head);
	assert_ptr_equal(addr[max_items - 1], list.tail);

	// get the head item
	found = NULL;
	item = NULL;
	found = (int *)l_head(&list, &item);
	assert_non_null(found);
	assert_int_equal(*found, items[0]);
	assert_ptr_equal(addr[0], item);

	// get the head item (repeat)
	found = NULL;
	item = NULL;
	found = (int *)l_head(&list, &item);
	assert_non_null(found);
	assert_int_equal(*found, items[0]);
	assert_ptr_equal(addr[0], item);

	// move the head
	l_pop_head(&list);
	assert_int_equal(max_items - 1, list.cnt);

	// get the head item
	found = NULL;
	item = NULL;
	found = (int *)l_head(&list, &item);
	assert_non_null(found);
	assert_int_equal(*found, items[1]);
	assert_ptr_equal(addr[1], item);

	// free allocated memory
	free_list_items(&list);

	(void)state; // unused
}

static void l_next_null_parm_test(void **state)
{
	int *anInt;
	struct l_item_t *item = NULL;
	void *found;

	// null item
	found = &anInt;
	assert_non_null(found);
	found = l_next(NULL);
	assert_null(found);

	// null deref
	found = &anInt;
	assert_non_null(found);
	found = l_next(&item);
	assert_null(found);

	(void)state; // unused
}

static void l_next_test(void **state)
{
	int anInt;
	const int max_items = 3;
	struct l_head_t list;
	struct l_item_t *item;
	int *found;
	int items[max_items];
	struct l_item_t *addr[max_items];
	int i;

	// empty list
	l_init(&list);
	assert_int_equal(0, list.cnt);

	found = &anInt;
	item = list.head;
	found = (int *)l_next(&item);
	assert_null(found);

	for (i = 0; i < max_items; i++) {
		items[i] = i;
	}

	// push all items
	l_init(&list);
	for (i = 0; i < max_items; i++) {
		l_push_tail(&list, &items[i]);
		assert_int_equal(i + 1, list.cnt);
		addr[i] = list.tail;
	}
	assert_int_equal(max_items, list.cnt);
	assert_ptr_equal(addr[0], list.head);
	assert_ptr_equal(addr[max_items - 1], list.tail);

	// at head, get 1
	found = NULL;
	item = list.head;
	found = (int *)l_next(&item);
	assert_non_null(found);
	assert_ptr_equal(addr[1], item);
	assert_int_equal(max_items, list.cnt);

	// at 1, get 2
	found = NULL;
	found = (int *)l_next(&item);
	assert_non_null(found);
	assert_ptr_equal(addr[2], item);
	assert_int_equal(max_items, list.cnt);

	// at 2, no more
	found = NULL;
	found = (int *)l_next(&item);
	assert_null(found);
	assert_null(item);
	assert_int_equal(max_items, list.cnt);

	// free allocated memory
	free_list_items(&list);

	(void)state; // unused
}

int main(int argc, char** argv)
{
	(void)argv; // not used
	argc++; // not used

	const struct CMUnitTest tests[] = {
	cmocka_unit_test(assumptions),
	cmocka_unit_test(l_init_null_parm_test),
	cmocka_unit_test(l_init_initial_state_test),
	cmocka_unit_test(l_init_initial_memory_test),
	cmocka_unit_test(l_push_tail_null_parm_test),
	cmocka_unit_test(l_push_tail_test),
	cmocka_unit_test(l_pop_head_null_parm_test),
	cmocka_unit_test(l_pop_head_test),
	cmocka_unit_test(l_add_null_parm_test),
	cmocka_unit_test(l_add_equal_test),
	cmocka_unit_test(l_add_greater_test),
	cmocka_unit_test(l_add_lessthan_test),
	cmocka_unit_test(l_add_test),
	cmocka_unit_test(l_remove_null_parm_test),
	cmocka_unit_test(l_remove_test),
	cmocka_unit_test(l_lremove_null_parm_test),
	cmocka_unit_test(l_lremove_test),
	cmocka_unit_test(l_find_null_parm_test),
	cmocka_unit_test(l_find_default_test),
	cmocka_unit_test(l_find_duplicates_test),
	cmocka_unit_test(l_find_unique_test),
	cmocka_unit_test(l_size_null_parm_test),
	cmocka_unit_test(l_size_test),
	cmocka_unit_test(l_head_null_parm_test),
	cmocka_unit_test(l_head_empty_list_test),
	cmocka_unit_test(l_head_test),
	cmocka_unit_test(l_next_null_parm_test),
	cmocka_unit_test(l_next_test), };

	return cmocka_run_group_tests(tests, NULL, NULL);
}

#ifdef __cplusplus
}
#endif

