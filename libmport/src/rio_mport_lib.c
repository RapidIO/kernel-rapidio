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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>

#include "../../include/rio_cm_cdev.h"

#define CONFIG_RAPIDIO_DMA_ENGINE
#include "../../include/rio_mport_cdev.h"

#include <rio_mport_lib.h>

#ifdef __cplusplus
extern "C" {
#endif

int rio_mport_open(uint32_t mport_id, int flags)
{
	char path[32];

	snprintf(path, sizeof(path), RIO_MPORT_DEV_PATH "%d", mport_id);
	return open(path, O_RDWR | O_CLOEXEC | flags);
}

int rio_cm_open(void)
{
	return open(RIO_CMDEV_PATH, O_RDWR | O_CLOEXEC);
}

/* Get the list of available mport devices
 *
 * dev_ids - List of Mports installed (See Notes)
 * number_of_mports - Just set it to RIO_MAX_MPORTS
 * NOTE_1: dev_ids is an array of ((mport_id << 16) | destid)
 * NOTE_2: Free dev_ids via riomp_mgmt_free_mport_list.
 *
 * Returns 0 if OK, negative on error
 */
int rio_mport_get_mport_list(uint32_t **dev_ids, uint8_t *number_of_mports)
{
	int fd;
	uint32_t entries = *number_of_mports;
	uint32_t *list;
	int ret = -1;

	/* Open RapidIO Channel Manager */
	fd = rio_cm_open();
	if (fd < 0)
		return -1;

	list = (uint32_t *)calloc((entries + 1), sizeof(*list));
	if (list == NULL)
		goto outfd;

	/* Request MPORT list from the driver (first entry is list size) */
	list[0] = entries;
	if (ioctl(fd, RIO_CM_MPORT_GET_LIST, list)) {
		free(list);
		ret = errno;
		goto outfd;
	}

	/* Return list information */
	*dev_ids = &list[1]; /* pointer to the list */
	*number_of_mports = *list; /* return real number of mports */
	ret = 0;
outfd:
	close(fd);
	return ret;
}

int rio_mport_free_mport_list(uint32_t **dev_ids)
{
	/*
	 * Get head of the list, because we did hide the list size and mport ID
	 * parameters
	 */
	uint32_t *list;

	if (!dev_ids)
		return -1;
	list = (*dev_ids) - 1;
	free(list);
	return 0;
}

int rio_mport_get_ep_list(uint8_t mport_id, uint32_t **destids,
			  uint32_t *number_of_eps)
{
	int fd;
	int ret = 0;
	uint32_t entries;
	uint32_t *list;

	/* Open mport */
	fd = rio_cm_open();
	if (fd < 0)
		return -1;

	/* Get list size */
	entries = mport_id;
	if (ioctl(fd, RIO_CM_EP_GET_LIST_SIZE, &entries)) {
#ifdef MPORT_DEBUG
		fprintf(stderr, "%s RIO_CM_EP_GET_LIST_SIZE ioctl failed: %s\n",
			__func__, strerror(errno));
#endif
		ret = errno;
		goto outfd;
	}
#ifdef MPORT_DEBUG
	printf("RIO: %s() has %d entries\n", __func__, entries);
#endif
	/* Get list */
	list = (uint32_t *)calloc((entries + 2), sizeof(*list));
	if (list == NULL) {
		ret = -1;
		goto outfd;
	}

	/* Get list (first entry is list size) */
	list[0] = entries;
	list[1] = mport_id;
	if (ioctl(fd, RIO_CM_EP_GET_LIST, list)) {
		free(list);
		ret = errno;
		goto outfd;
	}

	/* Pass to callee, first entry of list is entries in list */
	*destids = &list[2];
	*number_of_eps = entries;

outfd:
	close(fd);
	return ret;
}

int rio_mport_free_ep_list(uint32_t **destids)
{
	/*
	 * Get head of the list, because we did hide the list size and mport ID
	 * parameters
	 */
	uint32_t *list;

	if (!destids)
		return -1;
	list = (*destids) - 2;
	free(list);
	return 0;
}

/*
 * Perform DMA data write transfer to a remote target using local
 * user-space source buffer
 */
int rio_dma_write(int fd, uint16_t destid, uint64_t tgt_addr, void *buf,
		uint32_t size, enum rio_exchange wr_mode,
		enum rio_transfer_sync sync,
		struct rio_dma_interleave *interleave)
{
	struct rio_transaction tran;
	struct rio_transfer_io xfer;
	int ret;

	xfer.rioid = destid;
	xfer.rio_addr = tgt_addr;
	xfer.loc_addr = (__u64)buf;
	xfer.length = size;
	xfer.handle = 0;
	xfer.offset = 0;
	xfer.method = wr_mode;

	if (!interleave) {
		xfer.ssdist = 0;
		xfer.sssize = 0;
		xfer.dsdist = 0;
		xfer.dssize = 0;
	} else {
		xfer.ssdist = interleave->ssdist;
		xfer.sssize = interleave->sssize;
		xfer.dsdist = interleave->dsdist;
		xfer.dssize = interleave->dssize;
	}

	tran.transfer_mode = RIO_TRANSFER_MODE_TRANSFER;
	tran.sync = sync;
	tran.dir = RIO_TRANSFER_DIR_WRITE;
	tran.count = 1;
	tran.block = (__u64)&xfer;

	ret = ioctl(fd, RIO_TRANSFER, &tran);
	return (ret < 0) ? errno : ret;
}

/*
 * Perform DMA data write transfer to a remote target using local
 * kernel-space source buffer
 */
int rio_dma_write_d(int fd, uint16_t destid, uint64_t tgt_addr,
		    uint64_t handle, uint32_t offset, uint32_t size,
		    enum rio_exchange wr_mode, enum rio_transfer_sync sync,
		    struct rio_dma_interleave *interleave)
{
	struct rio_transaction tran;
	struct rio_transfer_io xfer;
	int ret;

	xfer.rioid = destid;
	xfer.rio_addr = tgt_addr;
	xfer.loc_addr = (__u64)NULL;
	xfer.length = size;
	xfer.handle = handle;
	xfer.offset = offset;
	xfer.method = wr_mode;

	if (!interleave) {
		xfer.ssdist = 0;
		xfer.sssize = 0;
		xfer.dsdist = 0;
		xfer.dssize = 0;
	} else {
		xfer.ssdist = interleave->ssdist;
		xfer.sssize = interleave->sssize;
		xfer.dsdist = interleave->dsdist;
		xfer.dssize = interleave->dssize;
	}

	tran.transfer_mode = RIO_TRANSFER_MODE_TRANSFER;
	tran.sync = sync;
	tran.dir = RIO_TRANSFER_DIR_WRITE;
	tran.count = 1;
	tran.block = (__u64)&xfer;

	ret = ioctl(fd, RIO_TRANSFER, &tran);
	return (ret < 0) ? errno : ret;
}

/*
 * Perform DMA data read transfer from a remote target using local
 * user-space destination buffer
 */
int rio_dma_read(int fd, uint16_t destid, uint64_t tgt_addr, void *buf,
		 uint32_t size, enum rio_transfer_sync sync,
		 struct rio_dma_interleave *interleave)
{
	struct rio_transaction tran;
	struct rio_transfer_io xfer;
	int ret;

	xfer.rioid = destid;
	xfer.rio_addr = tgt_addr;
	xfer.loc_addr = (__u64)buf;
	xfer.length = size;
	xfer.handle = 0;
	xfer.offset = 0;

	if (!interleave) {
		xfer.ssdist = 0;
		xfer.sssize = 0;
		xfer.dsdist = 0;
		xfer.dssize = 0;
	} else {
		xfer.ssdist = interleave->ssdist;
		xfer.sssize = interleave->sssize;
		xfer.dsdist = interleave->dsdist;
		xfer.dssize = interleave->dssize;
	}

	tran.transfer_mode = RIO_TRANSFER_MODE_TRANSFER;
	tran.sync = sync;
	tran.dir = RIO_TRANSFER_DIR_READ;
	tran.count = 1;
	tran.block = (__u64)&xfer;

	ret = ioctl(fd, RIO_TRANSFER, &tran);
	return (ret < 0) ? errno : ret;
}

/*
 * Perform DMA data read transfer from a remote target using local
 * kernel-space destination buffer
 */
int rio_dma_read_d(int fd, uint16_t destid, uint64_t tgt_addr,
		   uint64_t handle, uint32_t offset, uint32_t size,
		   enum rio_transfer_sync sync,
		   struct rio_dma_interleave *interleave)
{
	struct rio_transaction tran;
	struct rio_transfer_io xfer;
	int ret;

	xfer.rioid = destid;
	xfer.rio_addr = tgt_addr;
	xfer.loc_addr = (__u64)NULL;
	xfer.length = size;
	xfer.handle = handle;
	xfer.offset = offset;

	if (!interleave) {
		xfer.ssdist = 0;
		xfer.sssize = 0;
		xfer.dsdist = 0;
		xfer.dssize = 0;
	} else {
		xfer.ssdist = interleave->ssdist;
		xfer.sssize = interleave->sssize;
		xfer.dsdist = interleave->dsdist;
		xfer.dssize = interleave->dssize;
	}

	tran.transfer_mode = RIO_TRANSFER_MODE_TRANSFER;
	tran.sync = sync;
	tran.dir = RIO_TRANSFER_DIR_READ;
	tran.count = 1;
	tran.block = (__u64)&xfer;

	ret = ioctl(fd, RIO_TRANSFER, &tran);
	return (ret < 0) ? errno : ret;
}

/*
 * Wait for asynchronous DMA transfer completion
 */
int rio_wait_async(int fd, uint32_t cookie, uint32_t tmo)
{
	struct rio_async_tx_wait wparam;

	wparam.token = cookie;
	wparam.timeout = tmo;

	if (ioctl(fd, RIO_WAIT_FOR_ASYNC, &wparam))
		return errno;
	return 0;
}

/*
 * Allocate and map into RapidIO space a local kernel-space data buffer
 * (for inbound RapidIO data read/write requests)
 */
int rio_ibwin_map(int fd, uint64_t *rio_base, uint32_t size, uint64_t *handle)
{
	struct rio_mmap ib;

	if (!rio_base || !handle)
		return -EINVAL;

	ib.rio_addr = *rio_base;
	ib.length = size;
	ib.address = *handle;

	if (ioctl(fd, RIO_MAP_INBOUND, &ib))
		return errno;
	*handle = ib.handle;
	*rio_base = ib.rio_addr;
	return 0;
}

/*
 * Free and unmap from RapidIO space a local kernel-space data buffer
 */
int rio_ibwin_free(int fd, uint64_t *handle)
{
	if (!handle)
		return -EINVAL;

	if (ioctl(fd, RIO_UNMAP_INBOUND, handle))
		return errno;
	return 0;
}

int rio_obwin_map(int fd, uint16_t destid, uint64_t rio_base, uint32_t size,
		  uint64_t *handle)
{
	struct rio_mmap ob;

	if (!handle)
		return -EINVAL;

	ob.rioid = destid;
	ob.rio_addr = rio_base;
	ob.length = size;

	if (ioctl(fd, RIO_MAP_OUTBOUND, &ob))
		return errno;
	*handle = ob.handle;
	return 0;
}

int rio_obwin_free(int fd, uint64_t *handle)
{
	if (!handle)
		return -EINVAL;

	if (ioctl(fd, RIO_UNMAP_OUTBOUND, handle))
		return errno;
	return 0;
}

/*
 * Allocate a local kernel-space data buffer for DMA data transfers
 */
int rio_dbuf_alloc(int fd, uint32_t size, uint64_t *handle)
{
	struct rio_dma_mem db;

	if (!handle)
		return -EINVAL;

	db.length = size;
	db.dma_handle = 0;
	db.address = *handle;

	if (ioctl(fd, RIO_ALLOC_DMA, &db))
		return errno;
	*handle = db.dma_handle;
	return 0;
}

/*
 * Free previously allocated local kernel-space data buffer
 */
int rio_dbuf_free(int fd, uint64_t *handle)
{
	if (ioctl(fd, RIO_FREE_DMA, handle))
		return errno;

	return 0;
}

/*
 * Query mport status/capabilities
 */
int rio_query_mport(int fd, struct rio_mport_properties *qresp)
{
	if (!qresp)
		return -EINVAL;
	if (ioctl(fd, RIO_MPORT_GET_PROPERTIES, qresp))
		return errno;
	return 0;
}

/*
 * Read from local mport device register
 */
int rio_lcfg_read(int fd, uint32_t offset, uint32_t size, uint32_t *data)
{
	struct rio_mport_maint_io mt;

	mt.offset = offset;
	mt.length = size;
	mt.buffer = (__u64)data;

	if (ioctl(fd, RIO_MPORT_MAINT_READ_LOCAL, &mt))
		return errno;
	return 0;
}

/*
 * Write to local mport device register
 */
int rio_lcfg_write(int fd, uint32_t offset, uint32_t size, uint32_t data)
{
	struct rio_mport_maint_io mt;

	if (size != sizeof(uint32_t) || (offset & 0x3))
		return -EINVAL;

	mt.offset = offset;
	mt.length = size;
	mt.buffer = (__u64)&data;

	if (ioctl(fd, RIO_MPORT_MAINT_WRITE_LOCAL, &mt))
		return errno;
	return 0;
}

/*
 * Maintenance read from target RapidIO device register
 */
int rio_maint_read(int fd, uint32_t destid, uint32_t hc, uint32_t offset,
		     uint32_t size, uint32_t *data)
{
	struct rio_mport_maint_io mt;

	if (!data)
		return -EINVAL;

	mt.rioid = destid;
	mt.hopcount = hc;
	mt.offset = offset;
	mt.length = size;
	mt.buffer = (__u64)data;

	if (ioctl(fd, RIO_MPORT_MAINT_READ_REMOTE, &mt))
		return errno;
	return 0;
}

/*
 * Maintenance write to target RapidIO device register
 */
int rio_maint_write(int fd, uint32_t destid, uint32_t hc, uint32_t offset,
		      uint32_t size, uint32_t data)
{
	struct rio_mport_maint_io mt;

	/* size is enforced to match 'data' parameter type */
	if (size != sizeof(uint32_t))
		return -EINVAL;

	mt.rioid = destid;
	mt.hopcount = hc;
	mt.offset = offset;
	mt.length = size;
	mt.buffer = (__u64)&data;

	if (ioctl(fd, RIO_MPORT_MAINT_WRITE_REMOTE, &mt))
		return errno;
	return 0;
}

/*
 * Enable (register) receiving range of RapidIO doorbell events
 */
int rio_dbrange_enable(int fd, uint32_t rioid, uint16_t start, uint16_t end)
{
	struct rio_doorbell_filter dbf;

	dbf.rioid = rioid;
	dbf.low = start;
	dbf.high = end;

	if (ioctl(fd, RIO_ENABLE_DOORBELL_RANGE, &dbf))
		return errno;
	return 0;
}

/*
 * Disable (unregister) range of inbound RapidIO doorbell events
 */
int rio_dbrange_disable(int fd, uint32_t rioid, uint16_t start, uint16_t end)
{
	struct rio_doorbell_filter dbf;

	dbf.rioid = rioid;
	dbf.low = start;
	dbf.high = end;

	if (ioctl(fd, RIO_DISABLE_DOORBELL_RANGE, &dbf))
		return errno;
	return 0;
}

/*
 * Enable (register) filter for RapidIO port-write events
 */
int rio_pwrange_enable(int fd, uint32_t mask, uint32_t low, uint32_t high)
{
	struct rio_pw_filter pwf;

	pwf.mask = mask;
	pwf.low = low;
	pwf.high = high;

	if (ioctl(fd, RIO_ENABLE_PORTWRITE_RANGE, &pwf))
		return errno;
	return 0;
}

/*
 * Disable (unregister) filter for RapidIO port-write events
 */
int rio_pwrange_disable(int fd, uint32_t mask, uint32_t low, uint32_t high)
{
	struct rio_pw_filter pwf;

	pwf.mask = mask;
	pwf.low = low;
	pwf.high = high;

	if (ioctl(fd, RIO_DISABLE_PORTWRITE_RANGE, &pwf))
		return errno;
	return 0;
}

/*
 * Set event notification mask
 */
int rio_set_event_mask(int fd, unsigned int mask)
{
	if (ioctl(fd, RIO_SET_EVENT_MASK, mask))
		return errno;
	return 0;
}

/*
 * Get current value of event mask
 */
int rio_get_event_mask(int fd, unsigned int *mask)
{
	if (!mask)
		return -EINVAL;

	if (ioctl(fd, RIO_GET_EVENT_MASK, mask))
		return errno;
	return 0;
}

/*
 * Get current event data
 */
int rio_get_event(int fd, struct rio_event *revent)
{
	ssize_t bytes = 0;

	if (!revent)
		return -EINVAL;

	bytes = read(fd, revent, sizeof(revent));
	if (bytes == -1)
		return errno;
	if (bytes != sizeof(revent))
		return -EIO;

	return 0;
}

/*
 * send an event (only doorbell supported)
 */
int rio_send_event(int fd, struct rio_event *evt)
{
	char *p = (char *)evt;
	ssize_t ret;
	unsigned int len = 0;

	if (!evt)
		return -EINVAL;

	if (evt->header != RIO_DOORBELL)
		return -EOPNOTSUPP;

	while (len < sizeof(evt)) {
		ret = write(fd, p + len, sizeof(evt) - len);
		if (ret == -1)
			return errno;
		len += ret;
	}

	return 0;
}

/*
 * Set destination ID of local mport device
 */
int rio_destid_set(int fd, uint16_t destid)
{
	if (ioctl(fd, RIO_MPORT_MAINT_HDID_SET, &destid))
		return errno;
	return 0;
}

/*
 * Create a new kernel device object
 */
int rio_device_add(int fd, uint16_t destid, uint8_t hc, uint32_t ctag,
		    const char *name)
{
	struct rio_rdev_info dev;

	dev.destid = destid;
	dev.hopcount = hc;
	dev.comptag = ctag;
	if (name)
		strncpy(dev.name, name, RIO_MAX_DEVNAME_SZ);
	else
		*dev.name = '\0';

	if (ioctl(fd, RIO_DEV_ADD, &dev))
		return errno;
	return 0;
}

/*
 * Delete existing kernel device object
 */
int rio_device_del(int fd, uint16_t destid, uint8_t hc, uint32_t ctag,
		   const char *name)
{
	struct rio_rdev_info dev;

	dev.destid = destid;
	dev.hopcount = hc;
	dev.comptag = ctag;
	if (name)
		strncpy(dev.name, name, RIO_MAX_DEVNAME_SZ);
	else
		*dev.name = '\0';

	if (ioctl(fd, RIO_DEV_DEL, &dev))
		return errno;
	return 0;
}

/* Mailbox functions */
int rio_mbox_create_handle(uint8_t mport_id, uint8_t mbox_id,
			     rio_mailbox_t *mailbox)
{
	int fd;
	struct rio_mailbox *lhandle = NULL;

	/* Open mport */
	fd = rio_cm_open();
	if (fd < 0)
		return -1;

	/* Create handle */
	lhandle = (struct rio_mailbox *)malloc(sizeof(struct rio_mailbox));
	if (!(lhandle)) {
		close(fd);
		return -1;
	}

	lhandle->fd = fd;
	lhandle->mport_id = mport_id;
	*mailbox = lhandle;
	return 0;
}

int rio_socket_socket(rio_mailbox_t mailbox, rio_socket_t *socket_handle)
{
	struct rio_socket *handle = NULL;

	/* Create handle */
	handle = (struct rio_socket *)calloc(1, sizeof(struct rio_socket));
	if (!handle) {
		printf("error in calloc\n");
		return -1;
	}

	handle->mbox = mailbox;
	*socket_handle = handle;
	return 0;
}

int rio_socket_send(rio_socket_t socket_handle, void *buf, uint32_t size)
{
	int ret;
	struct rio_socket *handle = socket_handle;
	struct rio_cm_msg msg;

	msg.ch_num = handle->cdev.id;
	msg.size = size;
	msg.msg = (__u64)buf;
	ret = ioctl(handle->mbox->fd, RIO_CM_CHAN_SEND, &msg);
	if (ret) {
		printf("RIO_CM_CHAN_SEND: returned %d for ch_num=%d (err=%d)\n",
			ret, msg.ch_num, errno);
		return errno;
	}

	return 0;
}

int rio_socket_receive(rio_socket_t socket_handle, void **buf,
		uint32_t timeout, volatile int *stop_req)
{
	struct rio_socket *handle = socket_handle;
	struct rio_cm_msg msg;
	bool errCheck;
	bool stopCheck;
	int ret;

	do {
		msg.ch_num = handle->cdev.id;
		msg.size = 0x1000;
		msg.rxto = timeout;
		msg.msg = (__u64)*buf;
		errno = 0;

		ret = ioctl(handle->mbox->fd, RIO_CM_CHAN_RECEIVE, &msg);
		errCheck = (errno == ETIME || errno == EINTR
						|| errno == EAGAIN);
		stopCheck = (!stop_req || (stop_req && (!*stop_req)));
	} while (ret && errCheck && stopCheck);

	if (ret)
		return errno;
	return 0;
}

int riomp_sock_release_receive_buffer(rio_socket_t socket_handle,
		void *skt_msg) /* always 4k aligned buffers */
{
	free(skt_msg);
	return 0;
}

int rio_socket_close(rio_socket_t *socket_handle)
{
	int ret;
	struct rio_socket *handle = *socket_handle;
	uint16_t ch_num;

	if (!handle)
		return -1;

	ch_num = handle->cdev.id;
	ret = ioctl(handle->mbox->fd, RIO_CM_CHAN_CLOSE, &ch_num);
	if (ret < 0) {
		fprintf(stderr,
			"CLOSE IOCTL: returned %d for ch_num=%d (err=%d, %s)\n",
			ret, (*socket_handle)->cdev.id, errno, strerror(errno));
		ret = errno;
	}

	free(handle);
	*socket_handle = NULL;
	return ret;
}

int rio_mbox_destroy_handle(rio_mailbox_t *mailbox)
{
	struct rio_mailbox *mbox = *mailbox;

	if (mbox != NULL) {
		close(mbox->fd);
		free(mbox);
		return 0;
	}
	return -1;

}

int rio_socket_bind(rio_socket_t socket_handle, uint16_t local_channel)
{
	struct rio_socket *handle = socket_handle;
	uint16_t ch_num;
	int ret;
	struct rio_cm_channel cdev;

	ch_num = local_channel;

	ret = ioctl(handle->mbox->fd, RIO_CM_CHAN_CREATE, &ch_num);
	if (ret < 0)
		return errno;

	memset(&cdev, 0, sizeof(cdev));
	cdev.id = ch_num;
	cdev.mport_id = handle->mbox->mport_id;

	handle->cdev.id = cdev.id;
	handle->cdev.mport_id = cdev.mport_id;

	ret = ioctl(handle->mbox->fd, RIO_CM_CHAN_BIND, &cdev);
	if (ret < 0)
		return errno;

	return 0;
}

int rio_socket_listen(rio_socket_t socket_handle)
{
	struct rio_socket *handle = socket_handle;
	uint16_t ch_num;
	int ret;

	ch_num = handle->cdev.id;

	ret = ioctl(handle->mbox->fd, RIO_CM_CHAN_LISTEN, &ch_num);
	if (ret)
		return errno;
	return 0;
}

int rio_socket_accept(rio_socket_t socket_handle, rio_socket_t *conn,
		uint32_t timeout, volatile int *stop_req)
{
	struct rio_socket *handle = socket_handle;
	struct rio_socket *new_handle;
	struct rio_cm_accept param;
	bool errCheck;
	bool stopCheck;
	int ret;

	if (NULL == handle || NULL == conn)
		return -1;

	do {
		param.ch_num = handle->cdev.id;
		param.pad0 = 0;
		param.wait_to = timeout;
		errno = 0;

		ret = ioctl(handle->mbox->fd, RIO_CM_CHAN_ACCEPT, &param);
		errCheck = (errno == ETIME || errno == EINTR
				|| errno == EAGAIN);
		stopCheck = (!stop_req || (stop_req && (!*stop_req)));
	} while (ret && errCheck && stopCheck);

	new_handle = *conn;
	if (new_handle)
		new_handle->cdev.id = param.ch_num;

	return 0;
}

int rio_socket_connect(rio_socket_t socket_handle, uint16_t destid,
		uint16_t channel, volatile int *stop_req)
{
	struct rio_socket *handle = socket_handle;
	uint16_t ch_num = 0;
	struct rio_cm_channel cdev;
	bool errCheck;
	bool stopCheck;
	int ret;

	if (handle->cdev.id == 0) {
		if (ioctl(handle->mbox->fd, RIO_CM_CHAN_CREATE, &ch_num))
			return errno;
		handle->cdev.id = ch_num;
	}

	do {
		/* Configure and Send Connect IOCTL */
		handle->cdev.remote_channel = channel;
		handle->cdev.mport_id = handle->mbox->mport_id;

		cdev.id = handle->cdev.id;
		cdev.remote_channel = channel;
		cdev.remote_destid = destid;
		cdev.mport_id = handle->mbox->mport_id;
		errno = 0;

		ret = ioctl(handle->mbox->fd, RIO_CM_CHAN_CONNECT, &cdev);
		errCheck = (errno == ETIME || errno == EINTR
						|| errno == EAGAIN);
		stopCheck = (!stop_req || (stop_req && (!*stop_req)));
	} while (ret && errCheck && stopCheck);

	if (ret)
		return errno;
	return 0;
}

int rio_socket_request_send_buffer(rio_socket_t socket_handle,
				   void **buf) //always 4k aligned buffers
{
	/* socket_handle won't be used for now */

	*buf = malloc(0x1000); /* Always allocate maximum size buffers */
	if (*buf == NULL)
		return -1;

	return 0;
}

int rio_socket_release_receive_buffer(rio_socket_t socket_handle,
				      void *buf) /* always 4k aligned buffers */
{
	free(buf);
	return 0;
}

int rio_socket_release_send_buffer(rio_socket_t socket_handle,
				   void *buf) /* always 4k aligned buffers */
{
	free(buf);
	return 0;
}

const char *speed_to_string(int speed)
{
	switch (speed) {
	case RIO_LINK_DOWN:
		return "LINK DOWN";
	case RIO_LINK_125:
		return "1.25Gb";
	case RIO_LINK_250:
		return "2.5Gb";
	case RIO_LINK_312:
		return "3.125Gb";
	case RIO_LINK_500:
		return "5.0Gb";
	case RIO_LINK_625:
		return "6.25Gb";
	default:
		return "ERROR";
	}
}

const char *width_to_string(int width)
{
	switch (width) {
	case RIO_LINK_1X:
		return "1x";
	case RIO_LINK_1XR:
		return "1xR";
	case RIO_LINK_2X:
		return "2x";
	case RIO_LINK_4X:
		return "4x";
	case RIO_LINK_8X:
		return "8x";
	case RIO_LINK_16X:
		return "16x";
	default:
		return "ERROR";
	}
}

void display_mport_info(struct rio_mport_properties *attr)
{
	printf("\n+++ SRIO mport configuration +++\n");
	printf("mport: hdid=%d, id=%d, idx=%d, flags=0x%x, sys_size=%s\n",
		attr->hdid, attr->id, attr->index, attr->flags,
		attr->sys_size ? "large" : "small");

	printf("link: speed=%s width=%s\n", speed_to_string(attr->link_speed),
		width_to_string(attr->link_width));

	if (attr->flags & RIO_MPORT_DMA) {
		printf("DMA: max_sge=%d max_size=%d alignment=%d (%s)\n",
			attr->dma_max_sge, attr->dma_max_size,
			attr->dma_align,
			(attr->flags & RIO_MPORT_DMA_SG) ? "HW SG" : "no SG");
	} else
		printf("No DMA support\n");
	printf("\n");
}

#ifdef __cplusplus
}
#endif
