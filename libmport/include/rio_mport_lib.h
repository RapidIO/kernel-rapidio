/*
 * Copyright 2014, 2015 Integrated Device Technology, Inc.
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
 */

#ifndef _RIO_MPORT_LIB_H_
#define _RIO_MPORT_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdint.h> /* For size_t */
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>

//#include <linux/rio_cm_cdev.h>
#include "../../include/rio_cm_cdev.h"

#define CONFIG_RAPIDIO_DMA_ENGINE
//#include <linux/rio_mport_cdev.h>
#include "../../include/rio_mport_cdev.h"

#define RIO_MAX_MPORTS 8 /* max number of RIO mports supported by platform */
#define RIO_MPORT_DEV_PATH "/dev/rio_mport"
#define RIO_CMDEV_PATH "/dev/rio_cm"

#define RIO_SOCKET_MSG_SIZE 0x1000

enum rio_link_speed {
	RIO_LINK_DOWN = 0, /* SRIO Link not initialized */
	RIO_LINK_125 = 1, /* 1.25 GBaud  */
	RIO_LINK_250 = 2, /* 2.5 GBaud   */
	RIO_LINK_312 = 3, /* 3.125 GBaud */
	RIO_LINK_500 = 4, /* 5.0 GBaud   */
	RIO_LINK_625 = 5  /* 6.25 GBaud  */
};

enum rio_link_width {
	RIO_LINK_1X  = 0,
	RIO_LINK_1XR = 1,
	RIO_LINK_2X  = 3,
	RIO_LINK_4X  = 2,
	RIO_LINK_8X  = 4,
	RIO_LINK_16X = 5
};

enum rio_mport_flags {
	RIO_MPORT_DMA	 = (1 << 0), /* supports DMA data transfers */
	RIO_MPORT_DMA_SG = (1 << 1), /* DMA supports HW SG mode */
	RIO_MPORT_IBSG	 = (1 << 2), /* inbound mapping supports SG */
};

struct rio_mailbox {
	int fd;
	uint8_t mport_id;
};

struct rio_socket {
	struct rio_mailbox *mbox;
	struct rio_cm_channel cdev;
	uint8_t	*rx_buffer;
	uint8_t	*tx_buffer;
};

typedef struct rio_mailbox  *rio_mailbox_t;
typedef struct rio_socket   *rio_socket_t;

int rio_mport_open(uint32_t mport_id, int flags);

int rio_cm_open(void);

int rio_mport_get_mport_list(uint32_t **dev_ids, uint8_t *number_of_mports);

int rio_mport_free_mport_list(uint32_t **dev_ids);

int rio_mport_get_ep_list(uint8_t mport_id, uint32_t **destids,
			  uint32_t *number_of_eps);

int rio_mport_free_ep_list(uint32_t **destids);

int rio_dma_write(int fd, uint16_t destid, uint64_t tgt_addr, void *buf,
		  uint32_t size, enum rio_exchange wr_mode,
		  enum rio_transfer_sync sync,
		  struct rio_dma_interleave *interleave);

int rio_dma_write_d(int fd, uint16_t destid, uint64_t tgt_addr,
		      uint64_t handle, uint32_t offset, uint32_t size,
		      enum rio_exchange wr_mode, enum rio_transfer_sync sync,
			  struct rio_dma_interleave *interleave);

int rio_dma_read(int fd, uint16_t destid, uint64_t tgt_addr, void *buf,
		   uint32_t size, enum rio_transfer_sync sync,
		   struct rio_dma_interleave *interleave);

int rio_dma_read_d(int fd, uint16_t destid, uint64_t tgt_addr,
		     uint64_t handle, uint32_t offset, uint32_t size,
		     enum rio_transfer_sync sync,
		     struct rio_dma_interleave *interleave);

int rio_wait_async(int fd, uint32_t cookie, uint32_t tmo);

int rio_ibwin_map(int fd, uint64_t *rio_base, uint32_t size, uint64_t *handle);

int rio_ibwin_free(int fd, uint64_t *handle);

int rio_obwin_map(int fd, uint16_t destid, uint64_t rio_base, uint32_t size,
		    uint64_t *handle);

int rio_obwin_free(int fd, uint64_t *handle);

int rio_dbuf_alloc(int fd, uint32_t size, uint64_t *handle);

int rio_dbuf_free(int fd, uint64_t *handle);

int rio_query_mport(int fd, struct rio_mport_properties *qresp);

int rio_lcfg_read(int fd, uint32_t offset, uint32_t size, uint32_t *data);

int rio_lcfg_write(int fd, uint32_t offset, uint32_t size, uint32_t data);

int rio_maint_read(int fd, uint32_t destid, uint32_t hc, uint32_t offset,
		     uint32_t size, uint32_t *data);

int rio_maint_write(int fd, uint32_t destid, uint32_t hc, uint32_t offset,
		      uint32_t size, uint32_t data);

int rio_dbrange_enable(int fd, uint32_t rioid, uint16_t start, uint16_t end);

int rio_dbrange_disable(int fd, uint32_t rioid, uint16_t start, uint16_t end);

int rio_pwrange_enable(int fd, uint32_t mask, uint32_t low, uint32_t high);

int rio_pwrange_disable(int fd, uint32_t mask, uint32_t low, uint32_t high);

int rio_set_event_mask(int fd, unsigned int mask);

int rio_get_event_mask(int fd, unsigned int *mask);

int rio_destid_set(int fd, uint16_t destid);

int rio_device_add(int fd, uint16_t destid, uint8_t hc, uint32_t ctag,
		     const char *name);

int rio_device_del(int fd, uint16_t destid, uint8_t hc, uint32_t ctag,
		   const char *name);

int rio_mbox_create_handle(uint8_t mport_id, uint8_t mbox_id,
			     rio_mailbox_t *mailbox);

int rio_socket_socket(rio_mailbox_t mailbox, rio_socket_t *socket_handle);

int rio_socket_send(rio_socket_t socket_handle, void *buf, uint32_t size);

int rio_socket_receive(rio_socket_t socket_handle, void **buf,
		uint32_t timeout, volatile int *stop_req);

int rio_socket_release_receive_buffer(rio_socket_t socket_handle, void *buf);

int rio_socket_close(rio_socket_t *socket_handle);

int rio_mbox_destroy_handle(rio_mailbox_t *mailbox);

int rio_socket_bind(rio_socket_t socket_handle, uint16_t local_channel);

int rio_socket_listen(rio_socket_t socket_handle);

int rio_socket_accept(rio_socket_t socket_handle, rio_socket_t *conn,
			uint32_t timeout, volatile int *stop_req);

int rio_socket_connect(rio_socket_t socket_handle, uint16_t destid,
		uint16_t channel, volatile int *stop_req);

int rio_socket_request_send_buffer(rio_socket_t socket_handle, void **buf);

int rio_socket_release_send_buffer(rio_socket_t socket_handle, void *buf);

void display_mport_info(struct rio_mport_properties *prop);

#ifdef __cplusplus
}
#endif
#endif /* _RIO_MPORT_LIB_H_ */
