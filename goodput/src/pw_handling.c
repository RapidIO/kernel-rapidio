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

#include <stdint.h>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include <pthread.h>

#include <sched.h>

#include "rio_route.h"
#include "string_util.h"
#include "libcli.h"
#include "rio_mport_lib.h"
#include "liblog.h"

#include "libtime_utils.h"
#include "worker.h"
#include "goodput.h"
#include "Tsi721.h"
#include "CPS1848.h"
#include "rio_misc.h"
#include "did.h"
#include "tsi721_reset.h"
#include "pw_handling.h"

#include "RapidIO_Error_Management_API.h"
#include "RapidIO_Port_Config_API.h"
#include "RapidIO_Routing_Table_API.h"

#ifdef __cplusplus
extern "C" {
#endif

void tsi721_disable_dma(void)
{
	// Turn off DMA until the link is successfully recovered.
	INFO("DMA is Disabled.\n");
	for (int i = 0; i < MAX_WORKERS; i++) {
		wkr[i].port_ok = 0;
	}
}

int tsi721_config_dlt(void)
{
	int ret;
	uint32_t plm_ctl;
	const uint32_t DLT_delay = TSI721_NSEC_TO_DLT(DLT_DELAY_NSEC);
	const uint32_t RSP_TO = TSI721_NSEC_TO_RTO(RSP_TO_DELAY_NSEC);

	// Compute 50 millisecond DLT timeout period.
	//
	// 50 millisceonds was chosen to avoid DLT notifications for
	// short term link interrupts, such as what happens whenever
	// the port is reset to clear PORT_LOCKOUT and PORT_DIS.

	INFO("7TEST: Enable DLT\n");

	// Enable on dead link timer event detection
	ret = rio_lcfg_read(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4, &plm_ctl);
	plm_ctl &= ~TSI721_PLM_IMP_SPEC_CTL_DLT_THRESH;
	plm_ctl |= DLT_delay;
	ret |= rio_lcfg_write(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4, plm_ctl);
	ret |= rio_lcfg_write(mp_h, TSI721_SR_RSP_TO, 4, RSP_TO);

	if (ret) {
		ERR("Register read/write problem %d\n", ret);
		return TSI721_FAIL;
	}
	return TSI721_OK;
}

int tsi721_MECS_send_and_wait(struct worker *info)
{
	int ret = 0;
	uint32_t status;
	int send_three = 3;
	const uint32_t mecs = TSI721_PLM_STATUS_MECS;
	const uint32_t plm_stat_oset = TSI721_PLM_STATUS;
	const uint32_t cs_tx = TSI721_PLM_LONG_CS_TX1;

	const struct timespec point_one_ms = {0, 100000};

	// Clear MECS status to remove stale events
	ret |= tsi721_clear_status(0);

	// INFW: Make teh link recovery a separate thread triggered by
	// the main event reception thread.  Do not check for port_ok but
	// rather for whether or not the DMA statu sis enabled or disabled.
	INFO("Waiting to receive first MECS.\n")
	do {
		ret |= rio_lcfg_write(info->mp_h, cs_tx, 4, TSI721_MECS_CS);
		nanosleep(&point_one_ms, NULL);
		ret |= rio_lcfg_read(info->mp_h, plm_stat_oset, 4, &status);
	} while (!ret && !info->stop_req && !(status & mecs));

	if (ret) {
		ERR("Register access problem: %d\n", ret);
		return TSI721_FAIL;
	}

	if (info->stop_req) {
		ERR("Link Init: halted while waiting for MECS, exiting.\n");
		goto exit;
	}

	INFO("Sending three MECS.\n")
	// Send three more MECS after receiving the first one.
	do {
		ret |= rio_lcfg_write(info->mp_h, cs_tx, 4, TSI721_MECS_CS);
		ret |= rio_lcfg_write(info->mp_h, plm_stat_oset, 4, mecs);
		nanosleep(&point_one_ms, NULL);
	} while (send_three-- && !ret && !info->stop_req);

	if (ret) {
		ERR("Register access problem: %d\n", ret);
		return TSI721_FAIL;
	}

	if (info->stop_req) {
		ERR("Link Init: halted while sending 3 MECS, exiting.\n");
		goto exit;
	}

	// Turn off DMA until the link is successfully recovered.
	INFO("DMA is Enabled\n");
	for (int i = 0; i < MAX_WORKERS; i++) {
		wkr[i].port_ok = 1;
	}
exit:
	return TSI721_OK;
}

void do_tsi721_handle_link_init(struct worker *info)
{
	DBG("\tWaiting for tsi721_mutex\n");
	sem_wait(&tsi721_mutex);
	INFO("\tHandling Link Init\n");
	switch (tsi721_wait_for_empty_and_unlock(info)) {
	default: ERR("Link Init: wait for empty FAIL\n");
		goto exit;
	case TSI721_AGAIN: break;
	case TSI721_OK: break;
	}
	INFO("Link Init: MECS send and wait\n");
	switch (tsi721_MECS_send_and_wait(info)) {
	default: ERR("Link Init: MECS send and wait FAIL\n");
		break;
	case TSI721_AGAIN: break;
	case TSI721_OK: break;
	}
exit:
	DBG("\tPOST tsi721_mutex\n");
	sem_post(&tsi721_mutex);
}

void do_tsi721_tsi721_recovery(struct worker *info)
{
	int ret = 0;
	unsigned int mp_h_evt_mask;
	uint32_t pw_ct_mask = 0xFFFFFFFF;
	uint32_t pw_ct_low = 0;
	uint32_t pw_ct_high = 0xFFFFFFFF;

	INFO("DMA is DISABLED\n");
	for (int i = 0; i < MAX_WORKERS; i++) {
		wkr[i].port_ok = 0;
	}

	ret = rio_get_event_mask(info->mp_h, &mp_h_evt_mask);
	if (ret) {
		ERR("Could not get event mask.");
		goto exit;
	}
	ret = rio_set_event_mask(info->mp_h, mp_h_evt_mask | RIO_PORTWRITE);
	if (ret) {
		ERR("Could not set event mask.");
		goto exit;
	}
	ret = rio_pwrange_enable(info->mp_h, pw_ct_mask, pw_ct_low, pw_ct_high);
	if (ret) {
		ERR("Could not enable port-write range.");
		goto exit;
	}

	ret = tsi721_config_port_reset();
	ret |= tsi721_config_dlt();
	ret |= tsi721_clear_status(0);
	if (ret)
		goto exit;

	while (!ret && !info->stop_req) {
		struct rio_event evt;
		uint32_t plm_stat;
		int bytes;

		bytes = read(info->mp_h, &evt, sizeof(evt));
		if (bytes < 0) {
			if (EAGAIN == errno) {
				continue;
			} else {
				ret = bytes;
				ERR("Failed to read event, %d err=%d\n",
					bytes, errno);
				break;
			}
		}

		if (evt.header != RIO_PORTWRITE) {
			ERR("Ignoring event type %d)\n", evt.header);
			continue;
		}

		INFO("Port-write message:\n");
		INFO("\tComp Tag: 0x%08x\n", evt.u.portwrite.payload[0]);
		INFO("\tErr Det : 0x%08x\n", evt.u.portwrite.payload[1]);
		INFO("\tImp Spec: 0x%08x\n", evt.u.portwrite.payload[2]);
		INFO("\tLog EDet: 0x%08x\n", evt.u.portwrite.payload[3]);

		plm_stat = evt.u.portwrite.payload[2];

		if (plm_stat & TSI721_PLM_STATUS_DLT) {
			INFO("\tHandling DLT\n");
			tsi721_disable_dma();
			info->hs_worker[0]->stop_req = 1;
		}
		if (plm_stat & TSI721_PLM_STATUS_LINK_INIT) {
			if (info->hs_worker[TSI721_I]->stat != 2) {
				ERR("Worker not halted! Link Init dropped.\n");
				continue;
			}
			INFO("\tHandling Link Init\n");
			info->hs_worker[TSI721_I]->action = seven_handle_link_init;
			info->hs_worker[TSI721_I]->action_mode = kernel_action;
			info->hs_worker[TSI721_I]->stop_req = 0;
			sem_post(&info->hs_worker[TSI721_I]->run);
		}
	}
exit:
	if (ret) {
		ERR("Register access failed\n");
	}
}

void do_tsi721_fault_ins(struct worker *info)
{
	int ret = 0;
	uint32_t wait_period = info->seven_test_period
			- info->seven_test_downtime
			- info->seven_test_err_resp_time
			- info->seven_test_resp_to_time;
	uint32_t ibw;
	uint32_t filter;
	const uint32_t ibw_oset = TSI721_IBWIN_LBX(0);
	const uint32_t filt_oset = TSI721_TLM_SP_FTYPE_FILTER_CTL;
	const uint32_t fadd = TSI721_TLM_SP_FTYPE_FILTER_CTL_F13_RESPONSE_DATA
			| TSI721_TLM_SP_FTYPE_FILTER_CTL_F13_RESPONSE
			| TSI721_TLM_SP_FTYPE_FILTER_CTL_F13_OTHER;
	uint32_t ctl;
	uint32_t ctl_oset = TSI721_SP_CTL;
	uint32_t iter = 0;

	uint32_t did, dev, vend, devi;
	rio_port_t port;

	// INFW: Test to confirm that switch link partner is correctly configured.
	ret = rio_lcfg_read(info->mp_h, RIO_DEVID, 4, &did);
	port = DID_TO_PORT(GET_DEV8_FROM_HW(did));
	ret = rio_maint_read(mp_h, SWITCH_DEVID, SWITCH_HOPCOUNT,
				RIO_DEV_IDENT, sizeof(dev), &dev);
	vend = dev & RIO_DEV_IDENT_VEND;
	devi = (dev & RIO_DEV_IDENT_DEVI) >> 16;
	DBG("vend 0x%x devi 0x%x\n", vend, devi);

	dev = 0;
	if (vend == RIO_VEND_IDT) {
		if ((RIO_DEVI_IDT_CPS1848 == devi)
			|| (RIO_DEVI_IDT_CPS1432 == devi)
			|| (RIO_DEVI_IDT_CPS1616 == devi)
			|| (RIO_DEVI_IDT_VPS1616 == devi)
			|| (RIO_DEVI_IDT_SPS1616 == devi))
			dev = 1;
	}

	if (ret)
		dev = 0;
	DBG("ret 0x%x dev %d\n", ret, dev);
	ret = 0;

	while (!info->stop_req && !ret) {
		HIGH("7Test: Iter %d Tsi721 UpTime\n", iter);

		// Clear PORT_DIS if it is set.
		// PORT_DIS may not be set if
		// this run is starting on a "fresh" port
		DBG("\tWaiting for tsi721_mutex\n");
		sem_wait(&tsi721_mutex);
		ret = rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &ctl);
		if (ctl & TSI721_SP_CTL_PORT_DIS) {
			ret= tsi721_wait_for_empty(info);
			if (ret) {
				ERR("Wait for empty failed %d ...\n", ret);
			} else {
				uint32_t unused;

				ret |= tsi721_clear_port_dis();
				// Force completion of previous PCIe writes
				ret |= rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &unused);
				ret |= rio_lcfg_write(mp_h, TSI721_SP_ACKID_STAT, 4,
						TSI721_SP_ACKID_STAT_CLR_OUTSTD_ACKID);
				// Force completion of previous PCIe write
				ret |= rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &unused);
			}
		}
		DBG("\tPOST tsi721_mutex\n");
		sem_post(&tsi721_mutex);

		if (ret)
			break;
		sleep(wait_period);

		if (info->stop_req) {
			break;
		}

		// Assumes that DMA is performed to inbound window 0
		if (!info->seven_test_err_resp_time)
			goto resp_to_time;

		ret = rio_lcfg_read(info->mp_h, ibw_oset, 4, &ibw);
		DBG("IBW 0x%x\n", ibw);
		ret |= rio_lcfg_write(info->mp_h, ibw_oset, 4,
				ibw | 0x10000000);
		HIGH("7Test: Iter %d Tsi721 ERR RESP Time\n", iter);
		if (ret)
			break;
		sleep(info->seven_test_err_resp_time);
		ret |= rio_lcfg_write(info->mp_h, ibw_oset, 4, ibw);

		if (ret || info->stop_req)
			break;
resp_to_time:
		if (!info->seven_test_resp_to_time)
			goto down_time;

		ret = rio_lcfg_read(info->mp_h, filt_oset, 4, &filter);
		DBG("FILTER 0x%x\n", filter);
		ret = rio_lcfg_write(info->mp_h, filt_oset, 4, filter | fadd);
		HIGH("7Test: Iter %d Tsi721 RESP TO Time\n", iter);
		if (ret)
			break;
		sleep(info->seven_test_resp_to_time);
		ret = rio_lcfg_write(info->mp_h, filt_oset, 4, filter);

		if (ret || info->stop_req)
			break;
down_time:
		if (!info->seven_test_downtime)
			continue;
		HIGH("7Test: Iter %d Tsi721 DOWNTIME\n", iter++);
		// INFW: Test to confirm that switch link partner is correctly
		// 	configured.
		// Test: before shutting down a port, confirm that the
		// fault handling is in place to notify everyone of the failure.
		DBG("\tWaiting for tsi721_mutex\n");
		sem_wait(&tsi721_mutex);
		if (dev) {
			uint32_t maskval, rc;
			rc = rio_maint_read(mp_h, SWITCH_DEVID, SWITCH_HOPCOUNT,
					CPS1848_BCAST_MCAST_MASK_X(0),
					sizeof(maskval), &maskval);
			if (!rc && !((1 << port) & maskval)) {
				CRIT("Port %d not in mask 0x%x\n", port, maskval);
			}
		}
		ret = rio_lcfg_read(info->mp_h, ctl_oset, 4, &ctl);
		ctl |= TSI721_SP_CTL_PORT_DIS;
		ret = rio_lcfg_write(info->mp_h, ctl_oset, 4, ctl);

		sleep(info->seven_test_downtime);
		ret= tsi721_wait_for_empty(info);
		if (ret) {
			ERR("Wait for empty failed %d ...\n", ret);
		} else {
			ret |= tsi721_clear_port_dis();
		}
		// This is a LONG time to wait for a semaphore, but it is
		// necessary to prevent port-write handling from occurring
		// while the link is down.
		DBG("\tPOST tsi721_mutex\n");
		sem_post(&tsi721_mutex);
	}

	if (ret) {
		ERR("Register access failed ERR %d %s\n", ret, strerror(ret));
	}
	INFO("Halting\n", ret, strerror(ret));
}

static volatile sig_atomic_t rcv_exit;

static void pw_rx_sig_handler(int signum)
{
	switch (signum) {
	case SIGTERM:
		rcv_exit = 1;
		break;
	case SIGINT:
		rcv_exit = 1;
		break;
	case SIGUSR1:
		rcv_exit = 1;
		break;
	}
}

void do_tsi721_pw_rx(struct worker *info)
{
	int ret = 0;
	unsigned int mp_h_evt_mask;
	uint32_t pw_ct_mask = 0xFFFFFFFF;
	uint32_t pw_ct_low = 0;
	uint32_t pw_ct_high = 0xFFFFFFFF;

        /* Trap signals that we expect to receive */
	signal(SIGINT,  pw_rx_sig_handler);
	signal(SIGTERM, pw_rx_sig_handler);
	signal(SIGUSR1, pw_rx_sig_handler);

	tsi721_wait_for_empty_and_unlock(info);
	ret |= tsi721_config_dlt();

	ret = rio_get_event_mask(info->mp_h, &mp_h_evt_mask);
	if (ret) {
		ERR("Could not get event mask.");
		goto exit;
	}
	ret = rio_set_event_mask(info->mp_h, mp_h_evt_mask | RIO_PORTWRITE);
	if (ret) {
		ERR("Could not set event mask.");
		goto exit;
	}

	ret = rio_pwrange_enable(info->mp_h, pw_ct_mask, pw_ct_low, pw_ct_high);

	while (!ret & !info->stop_req && !rcv_exit) {
		struct rio_event evt;
		uint32_t plm_stat;
		int bytes;

		bytes = read(info->mp_h, &evt, sizeof(evt));
		if (bytes < 0) {
			if (EAGAIN == errno) {
				continue;
			} else {
				ret = bytes;
				ERR("Failed to read event, %d err=%d\n",
					bytes, errno);
				break;
			}
		}

		if (evt.header != RIO_PORTWRITE) {
			ERR("Ignoring event type %d)\n", evt.header);
			continue;
		}
		INFO("Port-write message:\n");
		INFO("\tComp Tag: 0x%08x\n", evt.u.portwrite.payload[0]);
		INFO("\tErr Det : 0x%08x\n", evt.u.portwrite.payload[1]);
		INFO("\tImp Spec: 0x%08x\n", evt.u.portwrite.payload[2]);
		INFO("\tLog EDet: 0x%08x\n", evt.u.portwrite.payload[3]);

		plm_stat = evt.u.portwrite.payload[2];

		if (plm_stat & TSI721_PLM_STATUS_DLT) {
			INFO("\tHandling DLT\n");
		}
		if (plm_stat & TSI721_PLM_STATUS_LINK_INIT) {
			INFO("\tHandling Link Init\n");
		}
	}
exit:
	if (ret) {
		ERR("Register access problem %d.\n", ret);
	}
	ret = rio_pwrange_disable(info->mp_h, pw_ct_mask, pw_ct_low, pw_ct_high);
	if (ret) {
		ERR("Failed to disable PW range, %d err=%d\n", ret, errno);
	}
	ret = rio_set_event_mask(info->mp_h, mp_h_evt_mask);
	if (ret) {
		ERR("Could not restore event mask.");
		goto exit;
	}
}

uint32_t SRIO_API_ReadRegFunc(DAR_DEV_INFO_t *UNUSED(d_info),
				uint32_t offset,
				uint32_t *readdata)
{
	uint32_t x;
	uint32_t rc;

	rc = rio_maint_read(mp_h, SWITCH_DEVID, SWITCH_HOPCOUNT,
				offset, sizeof(x), &x)
				? RIO_ERR_ACCESS : RIO_SUCCESS;

	if (RIO_SUCCESS == rc) {
		*readdata = x;
	}
	return rc;
}

uint32_t SRIO_API_WriteRegFunc(DAR_DEV_INFO_t *UNUSED(d_info),
				uint32_t offset,
				uint32_t writedata)
{
	uint32_t rc = RIO_ERR_INVALID_PARAMETER;
	rc = rio_maint_write(mp_h, SWITCH_DEVID, SWITCH_HOPCOUNT,
				offset, sizeof(writedata), writedata)
				? RIO_ERR_ACCESS : RIO_SUCCESS;
	return rc;
}

void SRIO_API_DelayFunc(uint32_t delay_nsec, uint32_t delay_sec)
{
	struct timespec delay = {delay_sec, delay_nsec};
	nanosleep(&delay, NULL);
}

uint32_t init_switch_handle(DAR_DEV_INFO_t *dev_h)
{
	uint32_t ret;
	uint32_t rc = 1;
	uint32_t temp_devid;
	uint32_t switch_comp_tag;
	int i;

	ret = SRIO_API_ReadRegFunc(dev_h, RIO_DEV_IDENT, &temp_devid);
	if (ret)
		goto exit;

	INFO("DMA is Enabled\n");
	for (i = 0; i < MAX_WORKERS; i++)
		wkr[i].port_ok = 1;

	dev_h->devID = temp_devid;
	dev_h->driver_family = rio_get_driver_family(temp_devid);
	ret = DAR_Find_Driver_for_Device(1, dev_h);
	if (RIO_SUCCESS != ret) {
		ERR("Unable to find driver for switch, type 0x%x\n, ret 0x%x",
				temp_devid, ret);
		rc = EOPNOTSUPP;
		goto exit;
	}
	if (!(SWITCH(dev_h))) {
		ERR("Link partner is not a switch, type 0x%x\n, ret 0x%x",
				temp_devid, ret);
		rc = EOPNOTSUPP;
		goto exit;
	}
	ret = DARrioGetComponentTag(dev_h, &switch_comp_tag);
	if (RIO_SUCCESS != ret) {
		ERR("Could not read switch component tag: 0x%x\n", ret)
		rc = EOPNOTSUPP;
		goto exit;
	}
	if (SWITCH_COMPTAG == switch_comp_tag) {
		rc = 0;
		goto exit;
	}

	ret = DARrioSetComponentTag(dev_h, SWITCH_COMPTAG);
	if (RIO_SUCCESS != ret) {
		ERR("Could not write switch component tag: 0x%x\n", ret)
		goto exit;
	}

	rc = 0;
exit:
	INFO("Init switch handle EXIT rc %d...\n", rc);
	return rc;
}

uint32_t tsi721_init_no_kernel_scan(struct worker *info)
{
	uint32_t reg;
	int ret;

	// Configure port control.
	ret = rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &reg);
	reg |= TSI721_SP_CTL_INP_EN | TSI721_SP_CTL_OTP_EN;
	ret |= rio_lcfg_write(mp_h, TSI721_SP_CTL, 4, reg);


	// Ensure MAST_EN is set.
	ret |= rio_lcfg_write(mp_h, TSI721_SP_GEN_CTL, 4,
				TSI721_SP_GEN_CTL_DISC
				| TSI721_SP_GEN_CTL_MAST_EN
				| TSI721_SP_GEN_CTL_HOST);

	// Hardcode mem50 size.
	ret |= rio_lcfg_write(mp_h, RIO_PE_LL_CTL, 4, RIO_PE_LL_CTL_50BIT);

	ret |= tsi721_config_port_reset();
	ret |= tsi721_config_dlt();

	ret |= tsi721_wait_for_empty(info);
	if (reg & TSI721_SP_CTL_PORT_DIS) {
		ret |= tsi721_clear_port_dis();
	}

	if (reg & TSI721_SP_CTL_PORT_LOCKOUT) {
		ret |= tsi721_clear_lockout();
	}
	ret |= tsi721_clear_status(0);

	return ret;
}

uint32_t config_tsi721_based_on_switch(DAR_DEV_INFO_t *dev_h,
					struct librio_status *sw_h,
					struct worker *info)
{
	uint32_t did = PORT_TO_DID(CONN_PORT(dev_h));
	int ret = rio_destid_set(mp_h, did);

	if (ret)
		ERR("rio_destid_set Failed: ret 0x%x errno %d", ret, errno);

	info->tsi721_ct = did | (did << 16);
	ret = rio_lcfg_write(info->mp_h, TSI721_COMP_TAG, 4, info->tsi721_ct);
	if (ret)
		ERR("Component tag set Failed: ret 0x%x\n", ret);

	// If the switch cannot retransmit port writes, enable reliable
	// port-write delivery in the Tsi721.

	if (!sw_h->em_pw_cfg.port_write_re_tx) {
		uint32_t pw_ctl;
		ret = rio_lcfg_read(info->mp_h, TSI721_PW_CTL, 4, &pw_ctl);
		pw_ctl |= TSI721_PW_CTL_PWC_MODE;
		ret |= rio_lcfg_write(info->mp_h, TSI721_PW_CTL, 4, pw_ctl);

		if (ret)
			ERR("register read/write failed: ret 0x%x", ret);
	}

	ret = rio_lcfg_write(info->mp_h, TSI721_SP_LT_CTL, 4,
			TSI721_NSEC_TO_RTO(RSP_TO_DELAY_NSEC));
	if (ret)
		ERR("Response timeout setting Failed: ret 0x%x\n", ret);

	ret = rio_lcfg_write(info->mp_h, TSI721_PLM_SILENCE_TIMER, 4,
				0x20000000);
	if (ret)
		ERR("Tsi721 silence timer setting Failed: ret 0x%x\n", ret);

	return ret;
}

/* Ensure mulitcast mask for port-writes is allocated and configured.
 * Ensure routing table entry for this endpoint is configured.
 */
uint32_t set_mc_mask_and_route(DAR_DEV_INFO_t *dev_h,
			struct librio_status *sw_h,
			uint32_t my_devid,
			int initialization)
{
	uint32_t ret;
	rio_rt_alloc_mc_mask_in_t alloc_in;
	rio_rt_alloc_mc_mask_out_t alloc_out;
	rio_rt_change_rte_in_t chg_in;
	rio_rt_change_rte_out_t chg_out;
	rio_rt_change_mc_mask_in_t mcm_in;
	rio_rt_change_mc_mask_out_t mcm_out;
	rio_rt_set_changed_in_t set_in;
	rio_rt_set_changed_out_t set_out;
	rio_rt_probe_in_t pr_in;
	rio_rt_probe_out_t pr_out;
	uint32_t mask_rte;

	// If multicast mask has not been allocated, allocate the mask.
	pr_in.probe_on_port = RIO_ALL_PORTS;
	pr_in.tt = tt_dev16; // Default destID size from probe_all
	pr_in.destID = PW_DID;
	pr_in.rt = &sw_h->g_rt;
	ret = rio_rt_probe(dev_h, &pr_in, &pr_out);
	if (ret) {
		ERR("rio_rt_probe Fail 0x%8x 0x%8x\n",
			ret, pr_out.imp_rc);
		ret = 5;
		goto exit;
	}
	mask_rte = pr_out.routing_table_value;

	DBG("valid route %d mask %d\n",
		pr_out.valid_route, RIO_RTV_IS_MC_MSK(mask_rte));

	if (pr_out.valid_route && RIO_RTV_IS_MC_MSK(mask_rte))
		goto update_mask;

	INFO("DBG: allocating mc mask\n");
	alloc_in.rt = &sw_h->g_rt;
	ret = rio_rt_alloc_mc_mask(dev_h, &alloc_in, &alloc_out);
	if (ret) {
		ERR("DBG: rio_rt_alloc_mc_mask Fail 0x%x 0x%x\n",
			ret, alloc_out.imp_rc);
		goto exit;
	}
	mask_rte = alloc_out.mc_mask_rte;

update_mask:
	if (mask_rte != 0x100)
		ERR("mask rte 0x%x\n", mask_rte);
	mcm_in.mc_mask_rte = mask_rte;
	mcm_in.rt = &sw_h->g_rt;
	mcm_in.mc_info = sw_h->g_rt.mc_masks[RIO_RTV_GET_MC_MSK(mask_rte)];
	DBG("port %d\n", DID_TO_PORT(my_devid));
	mcm_in.mc_info.mc_mask = MC_MASK_ADD_PORT(mcm_in.mc_info.mc_mask,
					DID_TO_PORT(my_devid));
	DBG("mask 0x%08x\n", mcm_in.mc_info.mc_mask);
	mcm_in.mc_info.mc_destID = PW_DID;
	mcm_in.mc_info.in_use = 1;
	mcm_in.mc_info.allocd = 1;
	DBG("mc_destID 0x%x\n", mcm_in.mc_info.mc_destID);
	DBG("mask 0x%x\n", mcm_in.mc_info.mc_mask);
	DBG("in_use 0x%x\n", mcm_in.mc_info.in_use);
	DBG("allocd 0x%x\n", mcm_in.mc_info.allocd);
	if (!sw_h->g_rt.mc_masks[RIO_RTV_GET_MC_MSK(mask_rte)].changed) {
		DBG("Forced multicast mask change to 0x%x\n",
			mcm_in.mc_info.mc_mask);
		sw_h->g_rt.mc_masks[RIO_RTV_GET_MC_MSK(mask_rte)].changed = 1;
	}
	ret = rio_rt_change_mc_mask(dev_h, &mcm_in, &mcm_out);
	if (ret) {
		ERR("rio_rt_change_mc_mask Fail 0x%x 0x%x\n",
			ret, mcm_out.imp_rc);
		goto exit;
	}

	chg_in.dom_entry = 0;
	chg_in.idx = my_devid;
	chg_in.rte_value = RIO_RTV_PORT(DID_TO_PORT(my_devid));
	chg_in.rt = &sw_h->g_rt;
	ret = rio_rt_change_rte(dev_h, &chg_in, &chg_out);
	if (ret) {
		ERR("rio_rt_change_rte Fail 0x%x 0x%x\n",
			ret, alloc_out.imp_rc);
		goto exit;
	}
	if (!sw_h->g_rt.dev_table[my_devid].changed) {
		DBG("Forced routing table entry change...\n");
		sw_h->g_rt.dev_table[my_devid].changed = 1;
	}

	if (initialization) {
		chg_in.dom_entry = 0;
		chg_in.idx = PW_DID;
		chg_in.rte_value = RIO_RTV_PORT(DID_TO_PORT(my_devid));
		chg_in.rt = &sw_h->g_rt;
		ret = rio_rt_change_rte(dev_h, &chg_in, &chg_out);
		if (ret) {
			ERR("rio_rt_change_rte for PW Fail 0x%x 0x%x\n",
				ret, alloc_out.imp_rc);
			goto exit;
		}
	}

	if (!sw_h->g_rt.dev_table[my_devid].changed) {
		DBG("Forced PW routing table entry change...\n");
		sw_h->g_rt.dev_table[PW_DID].changed = 1;
	}

	set_in.set_on_port = RIO_ALL_PORTS;
	set_in.rt = &sw_h->g_rt;
	set_in.rt->default_route = RIO_RTE_DROP;
	ret = rio_rt_set_changed(dev_h, &set_in, &set_out);
	if (ret) {
		ERR("rio_rt_set_changed 0x%x 0x%x\n",
			ret, set_out.imp_rc);
		goto exit;
	}
exit:
	return ret;
}

uint32_t set_switch_routing_table(DAR_DEV_INFO_t *dev_h,
				struct librio_status *sw_h,
				uint32_t my_devid,
				int initialization)
{
	uint32_t ret;

	// Read current routing table
	rio_rt_probe_all_in_t rt_in;
	rio_rt_probe_all_out_t rt_out;
	rio_rt_probe_in_t pr_in;
	rio_rt_probe_out_t pr_out;

	rt_in.probe_on_port = RIO_ALL_PORTS;
	rt_in.rt = &sw_h->g_rt;
	ret = rio_rt_probe_all(dev_h, &rt_in, &rt_out);

	if (ret) {
		ERR("rio_rt_probe_all Fail 0x%8x 0x%8x\n",
			ret, rt_out.imp_rc);
		ret = 4;
		goto exit;
	}
	sw_h->g_rt.default_route = RIO_RTE_DROP;

	// Check which destIDs currently have valid routes.
	pr_in.probe_on_port = RIO_ALL_PORTS;
	pr_in.tt = tt_dev8; // Default destID size from probe_all
	pr_in.rt = &sw_h->g_rt;
	for (did_reg_t did = 1; did < MAX_DEVID_STATUS; did++) {
		if ((SWITCH_DEVID == did)
		|| (PW_DID == did)
		|| (my_devid == did))
			continue;

		pr_in.destID = did;
		ret = rio_rt_probe(dev_h, &pr_in, &pr_out);
		if (ret) {
			ERR("rio_rt_probe Fail did %d Err 0x%x Imp Rc 0x%x\n",
				did, ret, pr_out.imp_rc);
			continue;
		}
		devid_status[did] = pr_out.valid_route;
	}

	ret = set_mc_mask_and_route(dev_h, sw_h, my_devid, initialization);
	if (ret) {
		ERR("CONFIG_SWITCH: set_mc_mask_and_route Fail 0x%8x\n", ret);
		ret = 6;
		goto exit;
	}
exit:
	return ret;
}

uint32_t set_switch_event_management(DAR_DEV_INFO_t *dev_h,
			struct librio_status *sw_h)
{
	rio_em_cfg_set_in_t cfg_in;
	rio_em_cfg_set_out_t cfg_out;

	rio_em_cfg_t events[3];
	uint32_t ret;

	// Enable time-to-live for 100 usec.

	cfg_in.ptl.num_ports = RIO_ALL_PORTS;
	cfg_in.notfn = rio_em_notfn_none;
	cfg_in.num_events = 1;
	cfg_in.events = events;
	cfg_in.events[0].em_event = rio_em_d_ttl;
	cfg_in.events[0].em_detect = rio_em_detect_on;
	cfg_in.events[0].em_info = TTL_TIMEOUT_NSEC;

	ret = rio_em_cfg_set(dev_h, &cfg_in, &cfg_out);
	if (ret) {
		ERR("CONFIG_EV_MGMT: TTL Cfg Err 0x%8x Imp Rc 0x%8x\n",
			ret, sw_h->pc.imp_rc);
		ret = 2;
	}
	return ret;
}

// Set CPS link initialization silence time to ~13 usec.
// This may avoid some response timeouts when the link is reinitialized.
uint32_t set_switch_silence_timer(DAR_DEV_INFO_t *dev_h,
				int initialization)
{
	uint32_t ret = 0;
	rio_port_t st_port = 0;
	rio_port_t end_port = NUM_PORTS(dev_h);

	if (!initialization) {
		st_port = CONN_PORT(dev_h);
		end_port = st_port + 1;
	}
	for (rio_port_t port = st_port; port < end_port; port++) {
		uint32_t ops;
		ret |= DARRegRead(dev_h, CPS1848_PORT_X_OPS(port), &ops);
		ops &= ~CPS1848_PORT_X_OPS_SILENCE_CTL;
		ops |= 0x00400000;
		ret |= DARRegWrite(dev_h, CPS1848_PORT_X_OPS(port),
				ops);
	}
	return ret;
}

uint32_t set_switch_per_port_reset(DAR_DEV_INFO_t *dev_h)
{
	uint32_t ret;
	rio_pc_dev_reset_config_in_t rst_in = {rio_pc_rst_port};
	rio_pc_dev_reset_config_out_t rst_out;

	ret = rio_pc_dev_reset_config(dev_h, &rst_in, &rst_out);
	if (ret || (rst_out.rst != rst_in.rst)) {
		ERR("CONFIG_SWITCH: Reset Err 0x%8x Rst in: %d out %d 0x%8x\n",
			ret, rst_in.rst, rst_out.rst, rst_out.imp_rc);
	}
	return ret;
}

uint32_t set_switch_counter_enables(DAR_DEV_INFO_t *dev_h,
				int initialization)
{
	uint32_t ops;
	uint32_t ret = 0;
	rio_port_t st_port = 0;
	rio_port_t end_port = NUM_PORTS(dev_h);

	if (!initialization) {
		st_port = CONN_PORT(dev_h);
		end_port = st_port + 1;
	}

	for (int i = st_port; i < end_port; i++) {
		ret |= DARRegRead(dev_h, CPS1848_PORT_X_OPS(i), &ops);
		ret |= DARRegWrite(dev_h, CPS1848_PORT_X_OPS(i),
				ops | CPS1848_PORT_X_OPS_CNTRS_EN);
	}
	return ret;
}

uint32_t set_switch_port_enables(DAR_DEV_INFO_t *dev_h,
			struct librio_status *sw_h,
			int initialization)
{
	uint32_t ret;

	rio_pc_get_config_in_t get_in;
	rio_pc_set_config_in_t set_in;

	if (initialization) {
		get_in.ptl.num_ports = RIO_ALL_PORTS;
	} else {
		get_in.ptl.num_ports = 1;
		get_in.ptl.pnums[0] = CONN_PORT(dev_h);
	}
	ret = rio_pc_get_config(dev_h, &get_in, &sw_h->pc);
	if (ret) {
		ERR("CONFIG_SWITCH: Port Cfg Err 0x%8x Imp Rc 0x%8x\n",
			ret, sw_h->pc.imp_rc);
		goto exit;
	}

	set_in.lrto = 50;
	set_in.log_rto = 190;
	set_in.oob_reg_acc = 0;
	set_in.reg_acc_port = CONN_PORT(dev_h);
	set_in.num_ports = sw_h->pc.num_ports;
	memcpy(set_in.pc, sw_h->pc.pc, set_in.num_ports * sizeof(set_in.pc[0]));

	// ensure all ports are enabled (clear PORT_DIS), and packet exchange
	// is allowed (port_lockout is cleared, IN/OUT Enable is set)
	for (rio_port_t pt = 0; pt < set_in.num_ports; pt++) {
		if (!set_in.pc[pt].port_available || !set_in.pc[pt].powered_up)
			continue;
		set_in.pc[pt].xmitter_disable = 0;
		set_in.pc[pt].port_lockout = 0;
		set_in.pc[pt].nmtc_xfer_enable = 1;
	}
	ret = rio_pc_set_config(dev_h, &set_in, &sw_h->pc);
	if (ret) {
		ERR("CONFIG_SWITCH: Port Cfg Set Err 0x%8x Imp Rc 0x%8x\n",
			ret, sw_h->pc.imp_rc);
		ret = 3;
		goto exit;
	}
exit:
	return ret;
}

uint32_t lock_switch(DAR_DEV_INFO_t *dev_h,
			struct worker *info,
			uint16_t *my_devid,
			uint16_t *sw_lock)
{
	uint32_t ret;
	uint32_t limit = 30000;
	const struct timespec point_one_ms = {0, 100000};
	uint32_t mc_mask, err_stat;

	*my_devid = PORT_TO_DID(CONN_PORT(dev_h));
	if (!*my_devid) {
		CRIT("0 IS AN ILLEGAL DEVID!\n");
		ret = 0xFFFFFFFF;
		goto exit;
	}

	do {
		ret = DARrioAcquireDeviceLock(dev_h, *my_devid, sw_lock);
		if (!ret && (*sw_lock == *my_devid))
			break;
		if (RIO_ERR_ACCESS == ret)
			goto sleep;
		if (1 != tsi721_check_port_ok()) {
			CRIT("Port not OK when trying to lock???");
			goto sleep;
		}

		if (RIO_HOST_LOCK_DEVID == *sw_lock)
			continue;

		if (!*sw_lock || (*sw_lock >= PORT_TO_DID(NUM_PORTS(dev_h)))) {
			uint16_t new_lock;
			CRIT("SWITCH LOCKED BY DID %d - BAD.\n", *sw_lock);
			DARrioReleaseDeviceLock(dev_h, *sw_lock, &new_lock);
			nanosleep(&point_one_ms, NULL);
			continue;
		}
		// Lock value should be valid at this point.
		// Check lock valur port against multicast mask.
		ret |= DARRegRead(dev_h, CPS1848_BCAST_MCAST_MASK_X(0),
								&mc_mask);
		if (!(mc_mask & (1 << DID_TO_PORT(*sw_lock)))) {
			INFO("DestID %d (Port %d) owns lock but not enabled.\n",
				*sw_lock, DID_TO_PORT(*sw_lock));
			DARrioGetPortErrorStatus(dev_h, DID_TO_PORT(*sw_lock),
							&err_stat);
			if (!RIO_PORT_OK(err_stat)) {
				uint16_t new_lock;
				CRIT("DestID %d (Port %d) owns lock, NOT PORT OK."
					" Releasing lock.\n");
				DARrioReleaseDeviceLock(dev_h, *sw_lock, &new_lock);
			}
		}
sleep:
		nanosleep(&point_one_ms, NULL);
	} while (((ret == RIO_ERR_LOCK) || (ret == RIO_ERR_ACCESS))
		&& !info->stop_req && (limit-- > 0));

exit:
	if (ret || (*sw_lock != *my_devid)) {
		CRIT("Could not get device lock, Lock 0x%x Limit %d Err 0x%x\n",
			*sw_lock, limit, ret);
		if (!ret)
			ret = 1;
	}
	return ret;
}

uint32_t config_switch(DAR_DEV_INFO_t *dev_h,
			struct worker *info,
			struct librio_status *sw_h,
			int initialization)
{
	uint32_t ret;
	uint32_t rc;
	uint16_t my_devid = PORT_TO_DID(CONN_PORT(dev_h));
	uint16_t sw_lock;

	ret = lock_switch(dev_h, info, &my_devid, &sw_lock);
	if (ret) {
		ERR("Could not get device lock\n");
		goto exit;
	}

	ret = set_switch_silence_timer(dev_h, initialization);
	if (ret) {
		ERR("set_switch_silence_timer Fail\n");
		goto exit;
	}

	/* Configure device reset handling */
	ret = set_switch_per_port_reset(dev_h);
	if (ret) {
		ERR("set_switch_per_port_reset Fail\n");
		goto exit;
	}

	/* Add this device to routing table */
	ret = set_switch_counter_enables(dev_h, initialization);
	if (ret) {
		ERR("set_switch_counter_enables Fail.\n");
		goto exit;
	}

	/* Add this device to routing table */
	ret = set_switch_port_enables(dev_h, sw_h, initialization);
	if (ret) {
		ERR("set_switch_port_enables Fail.\n");
		goto exit;
	}

	// Configure port-writes after routing table has been configured.

	// Now that notification is configured, set up event notification
	// for the connected port and for all uninitialized ports.
	//ret = set_switch_event_management(dev_h, sw_h);
	//if (ret) {
		// ERR("set_switch_event_management Fail.\n");
		// goto exit;
	// }

	// Only enable routing tables after event management is configured.
	ret = set_switch_routing_table(dev_h, sw_h, my_devid, initialization);
	if (ret) {
		ERR("set_switch_routing_table Fail.\n");
		goto exit;
	}

exit:
	rc = DARrioReleaseDeviceLock(dev_h, my_devid, &sw_lock);
	if (rc) {
		if (sw_lock != my_devid)
			return ret;
		ERR("DARrioReleaseDeviceLock Failed Lock 0x%x Err 0x%x.\n",
			sw_lock, rc);
		if (!ret)
			ret = rc;
	}
	return ret;
}

uint32_t enable_switch_portwrites(DAR_DEV_INFO_t *dev_h)
{
	uint32_t ret;

	rio_em_dev_rpt_ctl_in_t rpt_in;
	rio_em_dev_rpt_ctl_out_t rpt_out;

	rpt_in.ptl.num_ports = RIO_ALL_PORTS;
	rpt_in.notfn = rio_em_notfn_pw;

	ret = rio_em_dev_rpt_ctl(dev_h, &rpt_in, &rpt_out);
	if (ret)
		ERR("enable_switch_portwrites failed: 0x%x 0x%x\n",
			ret, rpt_out.imp_rc);
	return ret;
}

uint32_t handle_tsi721_port_write(DAR_DEV_INFO_t *dev_h,
				struct librio_status *sw_h,
				struct worker *info,
				uint32_t *pw_payload)
{
	uint32_t ret = 0;
	uint32_t plm_stat;
	uint32_t ctl;
	uint32_t unused;
	plm_stat = pw_payload[2];

	if (plm_stat & TSI721_PLM_STATUS_DLT) {
		INFO("\tHandling DLT\n");
		tsi721_disable_dma();
		ret = tsi721_wait_for_empty(info);
		if (ret) {
			ERR("Wait for empty failed %d ...\n", ret);
			goto exit;
		}
		tsi721_reset_own_port_from_cli();
		// Force completion of previous PCIe writes
		ret |= rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &unused);
		ret |= rio_lcfg_write(mp_h, TSI721_SP_ACKID_STAT, 4,
				TSI721_SP_ACKID_STAT_CLR_OUTSTD_ACKID);
		// Force completion of previous PCIe write
		ret |= rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &unused);
	}

	if (!(plm_stat & TSI721_PLM_STATUS_LINK_INIT))
		goto exit;

	//DBG("\tSleeping\n");
	//sleep(2);

	DBG("\tWaiting for tsi721_mutex\n");
	sem_wait(&tsi721_mutex);

	INFO("\tHandling LINK_INIT\n");
	ret |= rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &ctl);
	ret |= rio_lcfg_write(mp_h, TSI721_SP_CTL, 4,
	ctl | TSI721_SP_CTL_PORT_LOCKOUT);
	// Push PCIe writes above to completion
	ret |= rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &ctl);
	ret |= tsi721_wait_for_empty_and_unlock(NULL);
	if (ret) {
		ERR("tsi721_wait_for_empty_and_unlock failed.\n");
		goto unlock;
	}
	ret |= tsi721_clear_lockout();
	ret |= tsi721_link_reset();
	// Ensure all previous PCIe writes have been completed
	ret |= rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &ctl);
	// Ensure all previous PCIe writes have been completed
	ret |= rio_lcfg_write(mp_h, TSI721_SP_ACKID_STAT, 4,
			TSI721_SP_ACKID_STAT_CLR_OUTSTD_ACKID);
	// Ensure all previous PCIe writes have been completed
	ret |= rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &ctl);

	// Wait until the other side is sure to have finished processing
	// the link init port-write before sending packets on the link.
	ret = init_switch_handle(dev_h);
	if (ret) {
		ERR("init_switch_handle failed: 0x%08x\n", ret);
		goto unlock;
	}

	/* May have been reconnected to a different switch. */
	ret = config_tsi721_based_on_switch(dev_h, sw_h, info);
	if (RIO_SUCCESS != ret) {
		ERR("Failed configuring tsi721: 0x%08x\n", ret);
		goto unlock;
	}

	ret = config_switch(dev_h, info, sw_h, 0);
	if (RIO_SUCCESS != ret) {
		ERR("Failed configuring switch: 0x%08x\n", ret);
		goto unlock;
	}
	INFO("Switch Configuration successful.\n");
	INFO("DMA is Enabled\n");
	for (int i = 0; i < MAX_WORKERS; i++) {
		wkr[i].port_ok = 1;
	}
unlock:
	DBG("\tPOST tsi721_mutex\n");
	sem_post(&tsi721_mutex);
exit:
	return ret;
}

uint32_t parse_switch_port_write(DAR_DEV_INFO_t *dev_h,
				uint32_t *pw_payload,
				rio_em_event_n_loc_t *events,
				rio_em_parse_pw_in_t *pw_in,
				rio_em_parse_pw_out_t *pw_out)
{
	uint32_t ret;
	rio_port_t my_port = CONN_PORT(dev_h);
	uint32_t found_one = 0;

	for (int i = 0; i < 4; i++)
		pw_in->pw[i] = pw_payload[i];
	pw_in->num_events = MAX_PW_EVENTS;
	pw_in->events = events;

	ret = rio_em_parse_pw(dev_h, pw_in, pw_out);
	if (ret) {
		ERR("rio_em_parse_pw 0x%x imp_rc 0x%x\n",
			ret, pw_out->imp_rc);
		goto exit;
	}
	INFO("PARSED PW %d events\n", pw_out->num_events);
	for (uint8_t i = 0; i < pw_out->num_events; i++) {
		DBG("%d : P%d : %s\n", i,
			events[i].port_num,
			EVENT_NAME_STR(events[i].event));
		if (events[i].port_num != my_port)
			found_one = 1;
	}
	if (pw_out->too_many)
		ERR("too_many     : %d\n", pw_out->too_many);
	if (pw_out->other_events)
		DBG("other_events : %d\n", pw_out->other_events);
	if (!found_one) {
		WARN("Zeroing port-write, only event is for this port???\n");
		pw_out->num_events = 0;
	}
exit:
	return ret;
}

uint32_t disable_switch_port_writes(DAR_DEV_INFO_t *dev_h)
{
	rio_em_dev_rpt_ctl_in_t rpt_in;
	rio_em_dev_rpt_ctl_out_t rpt_out;
	uint32_t ret;

	rpt_in.ptl.num_ports = RIO_ALL_PORTS;
	rpt_in.notfn = rio_em_notfn_none;
	ret = rio_em_dev_rpt_ctl(dev_h, &rpt_in, &rpt_out);
	if (ret) {
		ERR("rio_em_dev_rpt_ctl 0x%x imp_rc 0x%x\n",
			ret, rpt_out.imp_rc);
	}
	return ret;
}

uint32_t enable_switch_port_writes(DAR_DEV_INFO_t *dev_h)
{
	rio_em_dev_rpt_ctl_in_t rpt_in;
	rio_em_dev_rpt_ctl_out_t rpt_out;
	uint32_t ret;

	rpt_in.ptl.num_ports = RIO_ALL_PORTS;
	rpt_in.notfn = rio_em_notfn_pw;
	ret = rio_em_dev_rpt_ctl(dev_h, &rpt_in, &rpt_out);
	if (ret) {
		ERR("rio_em_dev_rpt_ctl 0x%x imp_rc 0x%x\n",
			ret, rpt_out.imp_rc);
	}
	return ret;
}

uint32_t query_switch_hot_swap_events(DAR_DEV_INFO_t *dev_h,
				rio_port_t port,
				rio_em_event_n_loc_t *events,
				int max_events,
				rio_em_get_pw_stat_in_t *stat_in,
				rio_em_get_pw_stat_out_t *stat_out)
{
	uint32_t ret;

	stat_in->ptl.num_ports = 1;
	stat_in->ptl.pnums[0] = port;
	stat_in->pw_port_num = port;
	DBG("pw port num %d\n", stat_in->pw_port_num);
	stat_in->num_events = max_events;
	stat_in->events = events;

	ret = rio_em_get_pw_stat(dev_h, stat_in, stat_out);
	if (ret) {
		ERR("rio_em_get_pw_stat Failed: 0x%x 0x%x",
			ret, stat_out->imp_rc);
		goto exit;
	}
	INFO("STATUS %d events:\n", stat_out->num_events);

	for (int i = 0; i < stat_out->num_events; i++) {
		if (events[i].port_num != port) {
			CRIT("Event Idx %d %d '%s' P%d PW %d\n", i,
				events[i].event,
				EVENT_NAME_STR(events[i].event),
				events[i].port_num , port);
			events[i].event = rio_em_a_no_event;
		}
		INFO("%d: P%d : %s\n", i, events[i].port_num,
			EVENT_NAME_STR(events[i].event));
	}
	if (stat_out->too_many)
		ERR("Too many: %d\n", stat_out->too_many);
	if (stat_out->other_events)
		INFO("Other   : %d\n", stat_out->other_events);
exit:
	return ret;
}


uint32_t switch_display_event_get_err_stat(DAR_DEV_INFO_t *dev_h,
					rio_port_t port,
					uint32_t *err_stat,
					rio_em_events_t ev)
{
	uint32_t ret;
	uint32_t reg_val = 0;
	ret = DARrioGetPortErrorStatus(dev_h, port, &reg_val);
	if (ret) {
		ERR("DARrioGetPortErrorStatus failed 0x%d\n", ret);
		goto exit;
	}
	reg_val &= CPS1848_PORT_X_ERR_STAT_CSR_PORT_OK;
	INFO("P%d Err_stat 0x%8x\n", port, reg_val);

	if ((rio_em_i_sig_det == ev) && !reg_val)
		WARN("Port %d Sig Det but NOT PORT_OK\n");

	if ((rio_em_f_los == ev) && reg_val)
		WARN("Port %d LOS but PORT_OK\n");

	if ((rio_em_f_err_rate == ev) && reg_val)
		WARN("Port %d F_ERR_RATE but PORT_OK\n");
	*err_stat = reg_val;
exit:
	return ret;
}

void handle_switch_port_write_only( rio_port_t pw_port,
				rio_em_events_t ev)
{
	const int pw_stat = (rio_em_i_sig_det == ev);
	const int did = PORT_TO_DID(pw_port);

	if (devid_status[did] && !pw_stat) {
		HIGH("Port %d DISABLED (DevID %d) by pw\n",
			pw_port, did);
	}
	if (!devid_status[did] && pw_stat) {
		HIGH("Port %d ENABLED (DevID %d) by pw\n",
			pw_port, did);
	}
	devid_status[did] = pw_stat;
}

uint32_t disable_switch_sig_det(DAR_DEV_INFO_t *dev_h, rio_port_t port)
{
	uint32_t ret;
	rio_em_cfg_set_in_t cfg_in;
	rio_em_cfg_set_out_t cfg_out;
	const int SIG = 0;
	rio_em_cfg_t enables;

	cfg_in.ptl.num_ports = 1;
	cfg_in.ptl.pnums[0] = port;
	cfg_in.notfn = rio_em_notfn_pw;
	cfg_in.num_events = 1;
	cfg_in.events = &enables;

	cfg_in.events[SIG].em_event = rio_em_i_sig_det;
	cfg_in.events[SIG].em_detect = rio_em_detect_off;
	cfg_in.events[SIG].em_info = 1;

	DBG("Port %d disable SIG DET\n", port);

	ret = rio_em_cfg_set(dev_h, &cfg_in, &cfg_out);
	if (ret) {
		ERR("rio_em_cfg_set Ret 0x%x Imp Rc 0x%x\n",
			ret, cfg_out.imp_rc);
		goto exit;
	}
exit:
	return ret;
}

uint32_t disable_switch_port(DAR_DEV_INFO_t *dev_h,
				rio_port_t pw_port)
{
	uint32_t ret;
	uint32_t mc_mask;
	rio_em_cfg_set_in_t cfg_in;
	rio_em_cfg_set_out_t cfg_out;
	rio_em_cfg_t enables[2];
	const int SIG = 0;
	const int LOS = 1;
	const int did = PORT_TO_DID(pw_port);

	// First things first: isolate the port by removing the port from
	// the routing table and multicast mask.  Then, wait a while to
	// ensure all packets drain both from the inbound port and at
	// the outbound port from the fatal LOS event.
	//
	// NOTE: This is specific to the CPS programming model
	ret = DARRegWrite(dev_h,
		CPS1848_BCAST_DEV_RTE_TABLE_X(did), CPS_RT_NO_ROUTE);
	if (devid_status[did])
		HIGH("Port %d DISABLED (DevID %d)\n", DID_TO_PORT(did), did);
	devid_status[did] = 0;
	sleep(0.25);

	// First handler for loss of signal event.
	// DISABLE loss of signal detection,
	// ENABLE  signal detection.
	cfg_in.ptl.num_ports = 1;
	cfg_in.ptl.pnums[0] = pw_port;
	cfg_in.notfn = rio_em_notfn_pw;
	cfg_in.num_events = 2;
	cfg_in.events = enables;
	cfg_in.events[SIG].em_event = rio_em_i_sig_det;
	cfg_in.events[SIG].em_detect = rio_em_detect_on;
	cfg_in.events[SIG].em_info = 1; // Fifty millisceconds
	cfg_in.events[LOS].em_event = rio_em_f_los;
	cfg_in.events[LOS].em_detect = rio_em_detect_off;
	// The requested LOS/DLT time below hAS NO EFFECT ON CPS!!!
	cfg_in.events[LOS].em_info = 0;

	ret = rio_em_cfg_set(dev_h, &cfg_in, &cfg_out);
	if (ret)
		ERR("Ret 0x%x Imp Rc 0x%x\n", ret, cfg_out.imp_rc);

	HIGH("Port %d now detects SIG DET\n", pw_port);

	// Lastly, after clearing the error that would have been multicast
	// to the just disabled port, remove the port from the multicast
	// mask.
	//
	// NOTE: This is specific to the CPS programming model
	ret |= DARRegRead(dev_h, CPS1848_BCAST_MCAST_MASK_X(0), &mc_mask);
	mc_mask = MC_MASK_REM_PORT(mc_mask, pw_port);
	ret |= DARRegWrite(dev_h, CPS1848_BCAST_MCAST_MASK_X(0), mc_mask);
	if (ret) {
		ERR("Switch register access failed 0x%x\n", ret);
		goto exit;
	}
exit:
	return ret;
}

uint32_t handle_switch_port_write_event(DAR_DEV_INFO_t *dev_h,
					rio_port_t pw_port,
					rio_em_events_t ev)
{
	uint32_t ret;
	const int did = PORT_TO_DID(pw_port);

	// We are the first to handle this event on the switch.
	// For signal detection events, just turn off signal detect.
	if (rio_em_i_sig_det == ev) {
		ret = disable_switch_sig_det(dev_h, pw_port);
		if (ret) {
			ERR("disable_switch_sig_det failed %d\n", ret);
			goto exit;
		}
		if (!devid_status[did])
			HIGH("Port %d ENABLED (DevID %d)\n", pw_port, did);
		devid_status[did] = 1;
		goto exit;
	}

	ret = disable_switch_port(dev_h, pw_port);
	if (ret)
		ERR("disable_switch_port failed %d\n", ret);
exit:
	return ret;
}

uint32_t reset_cps_port(DAR_DEV_INFO_t *dev_h, uint32_t port)
{
	uint32_t rst_reg = CPS1848_DEVICE_RESET_CTL_DO_RESET | (1 << port);

	DBG("Reset switch port %d\n", port);
	return SRIO_API_WriteRegFunc(dev_h, CPS1848_DEVICE_RESET_CTL, rst_reg);
}

uint32_t clear_switch_events(DAR_DEV_INFO_t *dev_h,
				uint32_t num_events,
				rio_em_event_n_loc_t *events)
{
	uint32_t ret, los = 0;
	rio_em_clr_events_in_t clr_in;
	rio_em_clr_events_out_t clr_out;

	INFO("Clearing %d events...\n", num_events);

	clr_in.num_events = num_events;
	clr_in.events = events;

	ret = rio_em_clr_events(dev_h, &clr_in, &clr_out);
	if (ret)
		ERR("rio_em_clr_events Failed: 0x%x %d 0x%x\n",
			ret, clr_out.failure_idx,
			clr_out.imp_rc);
	if (clr_out.pw_events_remain)
		ERR("PW Events : %d\n", clr_out.pw_events_remain);
	if (clr_out.int_events_remain)
		DBG("Int Events: %d\n",clr_out.int_events_remain);

	for (int i = 0; i < num_events; i++) {
		if (rio_em_f_los == events[i].event)
			los = 1;
	}
	if (los) {
		ret = reset_cps_port(dev_h, events[0].port_num);
		if (ret)
			ERR("reset_cps_port Failed: 0x%x\n", ret);
	}

	return ret;
};

uint32_t handle_switch_port_write(DAR_DEV_INFO_t *dev_h,
				struct worker *info,
				uint32_t *pw_payload)
{
	rio_em_get_pw_stat_in_t stat_in;
	rio_em_get_pw_stat_out_t stat_out;
	rio_em_parse_pw_in_t pw_in;
	rio_em_parse_pw_out_t pw_out;

	rio_em_event_n_loc_t events[MAX_PW_EVENTS];

	uint16_t my_devid, sw_lock;
	uint32_t ret, rc;
	rio_port_t pw_port = pw_payload[RIO_EMHS_PW_IMP_SPEC_IDX]
			& RIO_EM_PW_IMP_SPEC_PORT_MASK;


	ret = parse_switch_port_write(dev_h, pw_payload, events,
					&pw_in, &pw_out);
	if (ret)
		goto exit;
	if (!pw_out.num_events)
		goto exit;

	ret = lock_switch(dev_h, info, &my_devid, &sw_lock);
	if (ret) {
		ERR("Could not lock switch.\n");
		goto exit;
	}

	ret = disable_switch_port_writes(dev_h);
	if (ret) {
		ERR("disable_switch_port_writes failed.\n");
		goto unlock;
	}
	// Get all events from the port-write port...
	//
	ret = query_switch_hot_swap_events(dev_h, pw_port,
					&events[pw_out.num_events],
		sizeof(events)/sizeof(events[0]) - pw_out.num_events,
					&stat_in, &stat_out);

	if (!(pw_out.num_events + stat_out.num_events)) {
		INFO("No events active...\n");
		goto enable_pw;
	}

	INFO("Processing %d events\n", stat_out.num_events + pw_out.num_events);

	// Disable/enable events and update destID status.
	for (int i = 0; i < stat_out.num_events + pw_out.num_events; i++) {
		uint32_t err_stat;
		rio_em_events_t ev = events[i].event;

		// If its not Loss of Signal or Signal Detected, skip it...
		if ((rio_em_f_los != ev)
			&& (rio_em_f_err_rate != ev)
			&& (rio_em_i_sig_det != ev)) {
			INFO("Skipping event Idx %d %d '%s'\n", i,
				events[i].event,
				EVENT_NAME_STR(events[i].event));
			continue;
		}

		// Something is going on with this port.
		// Check its current state and set up accordingly.

		ret = switch_display_event_get_err_stat(dev_h, pw_port,
					&err_stat, ev);
		if (stat_out.num_events) {
			ret = handle_switch_port_write_event(dev_h,
						pw_port, ev);
		} else {
			handle_switch_port_write_only(pw_port, ev);
		}

	}

	// Only clear events if events exist on the device.
	if (!stat_out.num_events)
		goto enable_pw;

	// Wait for packets to drain before clearing the isolation event
	// and clearing ackIDs.

	ret = clear_switch_events(dev_h,
			pw_out.num_events + stat_out.num_events, events);
	if (ret)
		ERR("clear_switch_events failed 0x%x\n", ret);

enable_pw:
	ret = enable_switch_port_writes(dev_h);
	if (ret)
		ERR("enable_switch_port_writes failed.\n");

unlock:
	rc = DARrioReleaseDeviceLock(dev_h, my_devid, &sw_lock);
	if (!rc || (RIO_ERR_LOCK == rc))
		goto exit;
	if (rc && !(sw_lock == my_devid)) {
		ERR("DARrioReleaseDeviceLock Failed 0x%x lock 0x%x devid 0x%x.\n",
			rc, sw_lock, my_devid);
		if (!ret)
			ret = rc;
	}
exit:
	return ret;
}

void do_tsi721_handle_tsi_pw(struct worker *info)
{
	uint32_t ret;
	struct rio_event *evt;

	do {
		sem_wait(&info->process_pw);
		if (info->stop_req)
			break;

		sem_wait(&info->pw_mutex);
		evt = (struct rio_event *)l_pop_head(&info->pw);
		sem_post(&info->pw_mutex);

		ret = handle_tsi721_port_write(info->dev_h,
					(struct librio_status *)info->status,
					info, evt->u.portwrite.payload);
		if (ret) {
			ERR("handle_tsi721_port_write Failed: 0x%x\n", ret);
		}
		free(evt);
	} while (1);
}

void do_tsi721_handle_cps_pw(struct worker *info)
{
	uint32_t ret;
	struct rio_event *evt;

	do {
		ret = 0;
		sem_wait(&info->process_pw);
		if (info->stop_req)
			break;

		sem_wait(&info->pw_mutex);
		evt = (struct rio_event *)l_pop_head(&info->pw);
		sem_post(&info->pw_mutex);

		DBG("\tWaiting for tsi721_mutex\n");
		sem_wait(&tsi721_mutex);
		if (1 == tsi721_check_port_ok()) {
			ret = handle_switch_port_write(info->dev_h, info,
						evt->u.portwrite.payload);
		} else {
			CRIT("handle_switch_port_write when port NOT OK???\n");
		}
		DBG("\tPOST tsi721_mutex\n");
		sem_post(&tsi721_mutex);
		if (ret) {
			ERR("handle_switch_port_write Failed: 0x%x\n", ret);
		}
		free(evt);
	} while (1);
}

uint32_t handle_cps_link_up(DAR_DEV_INFO_t *dev_h,
			int port)
{
	uint32_t route;
	uint32_t ret;
	uint32_t did = PORT_TO_DID(port);

	ret = DARRegRead(dev_h,
		CPS1848_BCAST_DEV_RTE_TABLE_X(did), &route);
	if (ret) {
		ERR("Cannot read route for port %d\n", port)
		goto exit;
	}
	if (route == port) {
		HIGH("Port %d ENABLED (DevID %d)\n", port, did);
		devid_status[did] = 1;
	}
exit:
	return ret;
}

uint32_t reset_cps_port(DAR_DEV_INFO_t *dev_h, uint32_t port);

uint32_t handle_cps_link_down(DAR_DEV_INFO_t *dev_h,
			int port)
{
	uint32_t ctl, cpb = 1, tx_disc = 1;
	uint32_t ret;
	uint32_t did = PORT_TO_DID(port);
	uint32_t limit;
	const struct timespec one_ms = {0, 1 * 1000 * 1000};

	HIGH("Port %d DISABLED (DevID %d)\n", port, did);
	devid_status[did] = 0;

	ret = DARRegWrite(dev_h,
		CPS1848_BCAST_DEV_RTE_TABLE_X(did), CPS_RT_NO_ROUTE);
	nanosleep(&one_ms, NULL);

	// Set port lockout to cause packet discard
	ret |= DARRegRead(dev_h, CPS1848_PORT_X_CTL_1_CSR(port), &ctl);
	ret |= DARRegWrite(dev_h, CPS1848_PORT_X_CTL_1_CSR(port),
				ctl | CPS1848_PORT_X_CTL_1_CSR_PORT_LOCKOUT);
	if (ret) {
		ERR("Error reading/writing port control 0x%x\n", ret);
		return ret;
	}
	nanosleep(&one_ms, NULL);

	// Wait until no packets are routed to or discarded by the port
	limit = 100;
	do {
		const struct timespec point_one_ms = {0, 100000};
		nanosleep(&point_one_ms, NULL);

		ret |= DARRegRead(dev_h,
			CPS1848_PORT_X_VC0_CPB_TX_CNTR(port), &cpb);
		ret |= DARRegRead(dev_h,
			CPS1848_PORT_X_VC0_PKT_DROP_TX_CNTR(port), &tx_disc);
	} while ((cpb | tx_disc) && limit-- && !ret);

	if (ret) {
		ERR("Error polling CPS port performance counters 0x%x\n", ret);
		return ret;
	}

	//  Port is now empty, and all packets have been discarded.
	ret |= DARRegWrite(dev_h, CPS1848_PORT_X_CTL_1_CSR(port), ctl);
	// Ensure all previous PCIe writes are completed
	ret |= DARRegRead(dev_h, CPS1848_PORT_X_CTL_1_CSR(port), &ctl);

	if (ret) {
		ERR("Error clearing PORT_LOCKOUT 0x%x\n", ret);
		return ret;
	}

	// Reset the CPS port multiple times
	// to clear all error conditions and
	// drop all packets
	for (int i = 0; i < 3; i++) {
		const struct timespec fifty_ms = {0, 50 * 1000 * 1000};

		ret |= reset_cps_port(dev_h, port);
		nanosleep(&fifty_ms, NULL);
	}

	return ret;
}

uint32_t cps_poll_for_status(struct worker *info)
{
	uint32_t ret = 0, rc;
	uint32_t err_stat;
	struct worker *hndlr = info->hs_worker[CPS_I];
	DAR_DEV_INFO_t *dev_h = hndlr->dev_h;
	uint16_t my_devid;
	uint16_t sw_lock;

	// There is no interlock between starting the poll loop and
	// initialization of the handler thread.  Just return if the handler
	// thread has not been initialized yet.
	if (!dev_h)
		return 0;

	my_devid = PORT_TO_DID(CONN_PORT(dev_h));
	ret = lock_switch(dev_h, info, &my_devid, &sw_lock);
	if (ret) {
		ERR("Could not get device lock\n");
		goto exit;
	}
	for (int port = 0; port < NUM_PORTS(dev_h); port++) {
		int did = PORT_TO_DID(port);
		if (CONN_PORT(dev_h) == port)
			continue;

		ret = DARRegRead(dev_h, CPS1848_PORT_X_ERR_STAT_CSR(port),
				&err_stat);
		if (ret) {
			ERR("Poll port %d error 0x%x\n", port, ret);
			break;
		}
		err_stat = !!(CPS1848_PORT_X_ERR_STAT_CSR_PORT_OK & err_stat);
		if (err_stat == devid_status[did])
			continue;

		// Port staus different from devid_status.
		// Handle the change...
		//
		if (err_stat)
			ret = handle_cps_link_up(dev_h, port);
		else
			ret = handle_cps_link_down(dev_h, port);
		if (ret) {
			ERR("CPS port %d handling error 0x%x\n", port, ret);
			break;
		}
	}

	rc = DARrioReleaseDeviceLock(dev_h, my_devid, &sw_lock);
	if (rc) {
		if (sw_lock != my_devid)
			return ret;
		ERR("DARrioReleaseDeviceLock Failed Lock 0x%x Err 0x%x.\n",
			sw_lock, rc);
		if (!ret)
			ret = rc;
	}
exit:
	return ret;
}

void do_cps_poll_for_pw(struct worker *info)
{
	uint32_t ret;
	do {
		DBG("\tSleeping for 1 second\n");
		sleep(1);
		if (info->stop_req)
			break;
		DBG("\tWaiting for tsi721_mutex\n");
		sem_wait(&tsi721_mutex);
		ret = cps_poll_for_status(info);
		sem_post(&tsi721_mutex);
	} while (!info->stop_req && !ret);
	if (ret)
		CRIT("\tCPS POLLING EXITING DUE TO ERROR\n");
}

uint32_t receive_port_writes(DAR_DEV_INFO_t *dev_h,
				struct worker *info)
{
	uint32_t ret, rc;
	unsigned int mp_h_evt_mask;
	uint32_t pw_ct_mask = 0xFFFFFFFF;
	uint32_t pw_ct_low = 0;
	uint32_t pw_ct_high = 0xFFFFFFFF;
	struct worker *hndlr;

	ret = rio_get_event_mask(info->mp_h, &mp_h_evt_mask);
	if (ret) {
		ERR("Could not get event mask.");
		goto exit;
	}
	ret = rio_set_event_mask(info->mp_h, mp_h_evt_mask | RIO_PORTWRITE);
	if (ret) {
		ERR("rio_set_event_mask Failed: 0x%x", ret);
		goto cleanup_mask;
	}
	ret = rio_pwrange_enable(info->mp_h, pw_ct_mask, pw_ct_low, pw_ct_high);
	if (ret) {
		ERR("Could not enable port-write range.");
		goto cleanup_range;
	}

	ret = enable_switch_portwrites(dev_h);
	if (ret) {
		ERR("Could not enable switch port-writes.");
		goto cleanup_range;
	}

	while (!info->stop_req) {
		struct rio_event evt, *post_event;
		int bytes;

		bytes = read(info->mp_h, &evt, sizeof(evt));
		if (bytes < 0) {
			if (EAGAIN == errno) {
				continue;
			} else {
				ret = bytes;
				ERR("Failed to read event, %d err=%d\n",
					bytes, errno);
				break;
			}
		}

		if (evt.header != RIO_PORTWRITE) {
			ERR("Ignoring event type %d)\n", evt.header);
			continue;
		}

		INFO("Port-write message:\n");
		DBG("\tBytes RXed: %d\n", bytes);
		INFO("\tComp Tag: 0x%08x\n", evt.u.portwrite.payload[0]);
		DBG("\tErr Det : 0x%08x\n", evt.u.portwrite.payload[1]);
		INFO("\tImp Spec: 0x%08x\n", evt.u.portwrite.payload[2]);
		DBG("\tLog EDet: 0x%08x\n", evt.u.portwrite.payload[3]);

		if (SWITCH_COMPTAG == evt.u.portwrite.payload[0]) {
			hndlr = info->hs_worker[CPS_I];
			DBG("Handling switch pw\n");
		} else if (info->tsi721_ct == evt.u.portwrite.payload[0]) {
			hndlr = info->hs_worker[TSI721_I];
			DBG("Handling TSI721 pw\n");
		} else {
			ERR("Ignoring port-write ct 0x%08x\n",
				evt.u.portwrite.payload[0]);
			continue;
		}
		post_event = (struct rio_event *)malloc(sizeof(struct rio_event));
		memcpy(post_event, &evt, sizeof(struct rio_event));
		if (post_event->u.portwrite.payload[0] != evt.u.portwrite.payload[0])
			ERR("Bad event copy\n");

		sem_wait(&hndlr->pw_mutex);
		l_push_tail(&hndlr->pw, post_event);
		sem_post(&hndlr->pw_mutex);

		sem_post(&hndlr->process_pw);
	}

	for (int i = 0; i < sizeof(info->hs_worker)/sizeof(info->hs_worker[0]); i++) {
		info->hs_worker[i]->stop_req = 1;
		sem_post(&info->hs_worker[i]->process_pw);
	}

cleanup_range:
	rc = rio_pwrange_disable(info->mp_h, pw_ct_mask, pw_ct_low, pw_ct_high);
	if (rc)
		ERR("Cleanup rio_pwrange_disable Failed: 0x%x", ret);
cleanup_mask:
	rc = rio_set_event_mask(info->mp_h, mp_h_evt_mask | RIO_PORTWRITE);
	if (rc)
		ERR("Cleanup rio_set_event_mask Failed: 0x%x", ret);
exit:
	return ret;
}

void do_tsi721_manage_switch(struct worker *info)
{
	int ret = 0;
	DAR_DEV_INFO_t dev_h;
	librio_status sw_h;

	for (int i = 0; i < MAX_DEVID_STATUS; i++)
		devid_status[i] = 0;

	ret = DAR_proc_ptr_init(SRIO_API_ReadRegFunc, SRIO_API_WriteRegFunc,
				SRIO_API_DelayFunc);
	/* First, get device handle. */
	ret = init_switch_handle(&dev_h);
	if (RIO_SUCCESS != ret) {
		ERR("Failed initializing switch handle: 0x%08x\n", ret);
		return;
	}


	ret = config_tsi721_based_on_switch(&dev_h, &sw_h, info);
	if (RIO_SUCCESS != ret) {
		ERR("Failed configuring tsi721: 0x%08x\n", ret);
		return;
	}

	ret = config_switch(&dev_h, info, &sw_h, 1);
	if (RIO_SUCCESS != ret) {
		ERR("Failed configuring switch: 0x%08x\n", ret);
		return;
	}
	INFO("Switch Configuration successful.\n");

	for (int i = 0; i < sizeof(info->hs_worker)/sizeof(info->hs_worker[0]); i++) {
		info->hs_worker[i]->dev_h = &dev_h;
		info->hs_worker[i]->status = &sw_h;
	}

	ret = receive_port_writes(&dev_h, info);
	if (RIO_SUCCESS != ret) {
		ERR("receive_port_writes Failed: 0x%08x\n", ret);
	}
}

void do_tsi721_test_switch_lock(struct worker *info)
{
	DAR_DEV_INFO_t dev_h;
	librio_status sw_h;
	uint16_t my_devid;
	uint16_t sw_lock;
	uint16_t byte_val = 0;
	uint16_t curr_byte_val = 0;
	uint32_t reg;
	uint32_t mask;
	uint32_t shift;
	uint32_t did;
	uint32_t ret;
	uint32_t count = 0;
	const uint32_t reg_addr = CPS1848_ASSY_IDENT_CAR_OVRD;
	// DID TRANS   0  1  2  3  4  5  6
	int trans[] = {-1, 0, 1, 0, 0, 2, 3};

	for (int i = 0; i < MAX_DEVID_STATUS; i++)
		devid_status[i] = 0;

	ret = DAR_proc_ptr_init(SRIO_API_ReadRegFunc, SRIO_API_WriteRegFunc,
				SRIO_API_DelayFunc);
	/* First, get device handle. */
	ret = init_switch_handle(&dev_h);
	if (RIO_SUCCESS != ret) {
		ERR("Failed initializing switch handle: 0x%08x\n", ret);
		return;
	}


	ret = config_tsi721_based_on_switch(&dev_h, &sw_h, info);
	if (RIO_SUCCESS != ret) {
		ERR("Failed configuring tsi721: 0x%08x\n", ret);
		return;
	}

	ret = config_switch(&dev_h, info, &sw_h, 1);
	if (RIO_SUCCESS != ret) {
		ERR("Failed configuring switch: 0x%08x\n", ret);
		return;
	}
	INFO("Switch Configuration successful.\n");
	ret = SRIO_API_ReadRegFunc(&dev_h, reg_addr, &reg);
	if (RIO_SUCCESS != ret) {
		ERR("Failed reading assy override: 0x%08x\n", ret);
		return;
	}
	did = PORT_TO_DID(CONN_PORT(&dev_h));
	if (did > sizeof(trans)/sizeof(trans[0])) {
		ERR("DID %d out of range\n", did);
		return;
	}
	my_devid = did;
	shift = trans[did] * 8;
	mask = 0xFF << shift;
	byte_val = (reg & mask) >> shift;
	INFO("DID %d shift %d mask 0x%x  byte_val 0x%x\n",
		did, shift, mask, byte_val);

	while (!info->stop_req) {
		sem_wait(&tsi721_mutex);
		lock_switch(&dev_h, info, &my_devid, &sw_lock);
		ret = SRIO_API_ReadRegFunc(&dev_h, reg_addr, &reg);
		if (ret) {
			ERR("Failed reading assy override: 0x%08x\n", ret);
			goto unlock;
		}

		curr_byte_val = (reg & mask) >> shift;
		if (curr_byte_val != byte_val) {
			ERR("Failed CURR: 0x%x EXP: 0x%x REG 0x%x RET 0x%x\n",
				curr_byte_val, byte_val, reg, ret);
			goto unlock;
		}
		byte_val = (byte_val + 1) & 0xFF;
		reg = (reg & ~mask) | (byte_val << shift);

		ret = SRIO_API_WriteRegFunc(&dev_h, reg_addr, reg);
		if (ret) {
			ERR("Failed writing assy override: 0x%08x\n", ret);
			goto unlock;
		}
		count++;
		if (!(count & 0xFFFF)) {
			HIGH("0x10000 iterations\n");
		}
		ret = DARrioReleaseDeviceLock(&dev_h, my_devid, &sw_lock);
		if (!ret || (RIO_ERR_LOCK == ret)) {
			sem_post(&tsi721_mutex);
			continue;
		}
		ERR("ReleaseDeviceLock Failed Lock 0x%x Err 0x%x.\n",
				sw_lock, ret);
		break;
	}
unlock:
	sem_post(&tsi721_mutex);
}


#ifdef __cplusplus
}
#endif
