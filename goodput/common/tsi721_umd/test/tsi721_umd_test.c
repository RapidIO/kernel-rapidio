/*
 Copyright (c) 2019, Renesas Electronics Corporation.
 All rights reserved.
 *
 * This software is available to you under a choice of one of two licenses.
 * You may choose to be licensed under the terms of the GNU General Public
 * License(GPL) Version 2, or the BSD-3 Clause license below:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <pthread.h>
#include <unistd.h>
#include "tsi721_umd.h"

// Tests will assume that these settings are valid
#define TEST_MPORT           0
#define TEST_PHYS_ADDR       0x50000000
#define TEST_PHYS_QUEUE_SIZE 64*1024
#define TEST_PHYS_BUF_SIZE   16*1024*1024
#define TEST_PHYS_SIZE       TEST_PHYS_QUEUE_SIZE + TEST_PHYS_BUF_SIZE
#define TEST_PHYS_DMA_BASE   TEST_PHYS_ADDR + TEST_PHYS_QUEUE_SIZE
#define RIO_DEST             0
#define RIO_BASE             0x10000

#define NUM_SIMULTANEOUS_THREADS 7 // for multithreaded tests

// Test that some assumed configuration is available
// 1) libmport device 0 is available
// 2) 64k of memory at 0x50000000 is reserved for libmport use
static void test_requirements(void** state)
{
    (void)state;
    int fd = rio_mport_open(TEST_MPORT,0);
    uint64_t addr = TEST_PHYS_ADDR;
    assert_true(fd > 0);

    int ret = rio_dbuf_alloc(fd, TEST_PHYS_SIZE, &addr);
    assert_true(ret == 0);

    rio_dbuf_free(fd,&addr);
    close(fd);
}

static int setup_open_umd(void** state)
{
    int ret;
    struct tsi721_umd* p_h;

    p_h = malloc(sizeof(*p_h));

    ret = tsi721_umd_open(p_h, TEST_MPORT);
    if (ret < 0)
        return -1;

    *state = p_h;

    return 0;
}

static int teardown_close_umd(void** state)
{
    int ret;
    struct tsi721_umd* p_h = (struct tsi721_umd*)*state;

    if (p_h == NULL)
        return -1;

    ret = tsi721_umd_close(p_h);

    free(p_h);

    return ret;
}

static int setup_open_umd_and_queue0(void** state)
{
    int ret;
    struct tsi721_umd* p_h;

    ret = setup_open_umd(state);
    p_h = (struct tsi721_umd*)*state;
    if (ret < 0)
        return ret;

    ret = tsi721_umd_queue_config_multi(p_h, 0x01, (void*)TEST_PHYS_ADDR, TEST_PHYS_QUEUE_SIZE);
    if (ret < 0)
        return ret;

    ret = tsi721_umd_start(p_h);
    if (ret < 0)
        return ret;

    return 0;
}

static int setup_open_umd_and_queues(void** state)
{
    int ret;
    struct tsi721_umd* p_h;

    ret = setup_open_umd(state);
    p_h = (struct tsi721_umd*)*state;
    if (ret < 0)
        return ret;

    ret = tsi721_umd_queue_config_multi(p_h, 0x7F, (void*)TEST_PHYS_ADDR, TEST_PHYS_QUEUE_SIZE);
    if (ret < 0)
        return ret;

    if (p_h->chan_count < 7)
        return -1;

    ret = tsi721_umd_start(p_h);
    if (ret < 0)
        return ret;

    return 0;
}

static int teardown_close_umd_and_queues(void** state)
{
    int ret;
    struct tsi721_umd* p_h = (struct tsi721_umd*)*state;

    ret = tsi721_umd_stop(p_h);
    if (ret < 0)
        return ret;

    ret = teardown_close_umd(state);
    if (ret < 0)
        return -1;

    return 0;
}

static void test_bad_mport_id(void ** state)
{
    (void)state;
    int ret;
    struct tsi721_umd h;

    ret = tsi721_umd_open(&h, 20000);

    assert_true(ret < 0);
}

static void test_close_without_open(void** state)
{
    (void)state;
    int ret;
    struct tsi721_umd h;

    ret = tsi721_umd_close(&h);
    assert_true(ret < 0);
}

static void test_stop_without_start(void** state)
{
    (void)state;
    int ret;
    struct tsi721_umd h;

    ret = tsi721_umd_open(&h, TEST_MPORT);
    assert_true(ret == 0);

    ret = tsi721_umd_stop(&h);
    assert_true(ret < 0);
}

static void test_normal_state_transition(void** state)
{
    (void)state;
    int ret;
    struct tsi721_umd h;

    ret = tsi721_umd_open(&h, TEST_MPORT);
    assert_true(ret == 0);
    assert_true(h.state == TSI721_UMD_STATE_UNCONFIGURED);

    ret = tsi721_umd_queue_config_multi(&h, 0x7, (void*)TEST_PHYS_ADDR, TEST_PHYS_QUEUE_SIZE);
    assert_true(ret == 0);
    assert_true(h.state == TSI721_UMD_STATE_CONFIGURED);

    ret = tsi721_umd_start(&h);
    assert_true(ret == 0);
    assert_true(h.state == TSI721_UMD_STATE_READY);
    
    ret = tsi721_umd_stop(&h);
    assert_true(ret == 0);
    assert_true(h.state == TSI721_UMD_STATE_CONFIGURED);

    ret = tsi721_umd_close(&h);
    assert_true(ret == 0);
    assert_true(h.state == TSI721_UMD_STATE_UNALLOCATED);

    ret = tsi721_umd_stop(&h);
    assert_true(ret < 0);
}

static void test_start_without_queue(void** state)
{
    struct tsi721_umd* p_h = (struct tsi721_umd*)*state;
    int ret;

    ret = tsi721_umd_start(p_h);
    assert_true(ret < 0);
}


static void test_invalid_queue(void** state)
{
    struct tsi721_umd* p_h = (struct tsi721_umd*)*state;
    int ret;

    ret = tsi721_umd_queue_config(p_h, 255, (void*)TEST_PHYS_ADDR, TEST_PHYS_QUEUE_SIZE);
    
    assert_true(ret < 0);
}

static void test_valid_queue(void** state)
{
    struct tsi721_umd* p_h = (struct tsi721_umd*)*state;
    int ret;
    int i;

    ret = tsi721_umd_queue_config_multi(p_h, 0xFF, (void*)TEST_PHYS_ADDR, TEST_PHYS_QUEUE_SIZE);
    assert_true(ret == 0);

    assert_true(p_h->chan_mask == 0xFF);
    for (i=0; i<8; i++)
    {
        assert_true(p_h->chan[i].request_q         != NULL);
        assert_true(p_h->chan[i].request_q_phys    != NULL);
        assert_true(p_h->chan[i].completion_q_phys != NULL);
        assert_true(p_h->chan[i].completion_q_phys != NULL);

        assert_true(p_h->chan[i].reg_base != NULL);
        assert_true(p_h->chan[i].in_use == false);

        assert_true(p_h->chan[i].req_count == 0);
        assert_true(p_h->chan[i].status_count == 0);
    }

    ret = tsi721_umd_start(p_h);
    assert_true(ret == 0);
}

static void test_writes(void** state)
{
    struct tsi721_umd* p_h = (struct tsi721_umd*)*state;
    int i,ret;

    uint32_t msg_size = TEST_PHYS_BUF_SIZE/1000;

    for (i=0; i<1000; i++)
    {
        ret = tsi721_umd_send(p_h, (void*)((uintptr_t)TEST_PHYS_DMA_BASE + i*msg_size), msg_size, RIO_BASE + msg_size, RIO_DEST);
        assert_true(ret == 0);
    }

    assert_true(p_h->chan[0].req_count == 1000);
}

static void test_write_multi_dma(void** state)
{
    struct tsi721_umd* p_h = (struct tsi721_umd*)*state;
    int ret;

    // mark DMA engine 0 as in use, check that DMA engine 1 gets used
    p_h->chan[0].in_use = true;
    
    ret = tsi721_umd_send(p_h, (void*)TEST_PHYS_DMA_BASE, 1024, RIO_BASE, RIO_DEST);
    assert_true(ret == 0);
    assert_true(p_h->chan[1].req_count == 1);

    p_h->chan[0].in_use = false;
}

struct run_dmas_info {
    struct tsi721_umd* p_h;
    uint8_t            thread_id;
    volatile bool*     run_thread;
    volatile bool*     kill_thread;
    uint32_t           dma_count;
    uint32_t           dma_size;
    bool               stopped;
};

static void* run_dmas_thread(void* arg)
{
    struct run_dmas_info* p = (struct run_dmas_info*)arg;
    int ret;
    
    p->dma_count = 0;

    while (!*p->kill_thread)
    {
        if (*p->run_thread)
        {
            ret = tsi721_umd_send(p->p_h, (void*)TEST_PHYS_DMA_BASE, p->dma_size, RIO_BASE, RIO_DEST);
            assert_true(ret == 0 || ret == -EPERM); // either send ok or other thread stopped
            p->dma_count++;
        }
        else
        {
            if (!p->stopped)
            {
                p->stopped = true;
            }
        }

        usleep(10);
    }

    return NULL;
}

static void test_stop_while_running(void** state)
{
    (void)state;
    struct tsi721_umd h;
    int ret;
    pthread_t thread[NUM_SIMULTANEOUS_THREADS];
    bool run_thread = false;
    bool kill_thread = false;
    struct run_dmas_info info[NUM_SIMULTANEOUS_THREADS];

    int i,j;
    const int num_iterations = 8;

    for (i=0; i<NUM_SIMULTANEOUS_THREADS; i++)
    {
        info[i].p_h         = &h;
        info[i].run_thread  = &run_thread;
        info[i].kill_thread = &kill_thread;
        info[i].thread_id   = i;
        info[i].dma_count   = 0;
        info[i].dma_size    = 0;
        info[i].stopped     = false;
        pthread_create(&thread[i], NULL, run_dmas_thread, &info[i]);
    }

    for (i=0; i<num_iterations; i++)
    {
        ret = tsi721_umd_open(&h, TEST_MPORT);
        assert_true(ret == 0);

        ret = tsi721_umd_queue_config_multi(&h, 0x7, (void*)TEST_PHYS_ADDR, TEST_PHYS_QUEUE_SIZE);
        assert_true(ret == 0);

        ret = tsi721_umd_start(&h);
        assert_true(ret == 0);
    
        uint32_t dma_size = (i+1)*1024;
        assert_true(dma_size <= TEST_PHYS_BUF_SIZE);
        for (j=0; j<NUM_SIMULTANEOUS_THREADS; j++)
        {
            info[j].stopped  = false;
            info[j].dma_size = dma_size;
        }

        // On some arch may need a barrier here to ensure store ordering
        run_thread = true;

        sleep(1);

        ret = tsi721_umd_stop(&h);
        assert_true(ret == 0);
        
        run_thread = false;

        sleep(1);

        for (j=0; j<NUM_SIMULTANEOUS_THREADS; j++)
            assert_true(info[j].stopped == true);
        
        ret = tsi721_umd_close(&h);
        assert_true(ret == 0);

        printf("%13s stop test iteration %2d/%2d thread_DMAs","",i+1,num_iterations);
        for (j=0; j<NUM_SIMULTANEOUS_THREADS; j++)
        {
            printf("% 4d",info[j].dma_count);
            info[j].dma_count = 0;
        }
        printf("\n");
    }

    kill_thread = true;

    for (i=0; i<NUM_SIMULTANEOUS_THREADS; i++)
    {
        ret = pthread_join(thread[i], NULL);
        assert_true(ret == 0);
    }
}

int main(int argc, char** argv)
{
    (void)argv; // not used
    argc++;// not used

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_requirements),
        cmocka_unit_test(test_bad_mport_id),
        cmocka_unit_test(test_normal_state_transition),
        cmocka_unit_test(test_close_without_open),
        cmocka_unit_test(test_stop_without_start),
        cmocka_unit_test_setup_teardown(test_start_without_queue, setup_open_umd, teardown_close_umd),
        cmocka_unit_test_setup_teardown(test_invalid_queue, setup_open_umd, teardown_close_umd),
        cmocka_unit_test_setup_teardown(test_valid_queue, setup_open_umd, teardown_close_umd_and_queues),
        cmocka_unit_test_setup_teardown(test_writes, setup_open_umd_and_queue0, teardown_close_umd_and_queues),
        cmocka_unit_test_setup_teardown(test_write_multi_dma, setup_open_umd_and_queues, teardown_close_umd_and_queues),
        cmocka_unit_test(test_stop_while_running),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
