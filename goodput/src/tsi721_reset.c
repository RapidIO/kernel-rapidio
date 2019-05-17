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

#include "RapidIO_Error_Management_API.h"
#include "RapidIO_Port_Config_API.h"
#include "RapidIO_Routing_Table_API.h"

#ifdef __cplusplus
extern "C" {
#endif

/* tsi721_check_port_ok
 *
 * Confirms whether or not the link is successfully connected to a link partner.
 *
 * Return values:
 * -1 - register access error
 *  0 - Port Not OK
 *  1 - Port OK
 */

int tsi721_check_port_ok(void)
{
	int ret = 0;
	uint32_t estat;

	ret = rio_lcfg_read(mp_h, TSI721_SP_ERR_STAT, 4, &estat);

	if (ret)
		return -1;
	return (estat & TSI721_SP_ERR_STAT_PORT_OK) ? 1 : 0;
}

/* tsi721_dma_active
 *
 * Checks to see if the DMA engines are sending packets, or if the link
 * partner is sending us packets.
 *
 * Return values:
 *  0 - DMA engines are NOT sending/receiving packets
 *  1 - DMA engines are sending/receiving packets
 */

int tsi721_dma_active(int check_port_lockout)
{
	int ret;
	uint32_t dma_cnt, brg_cnt, ctl, clear;
	const struct timespec point_one_ms = {0, 100000};

	if (check_port_lockout) {
		ret = rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &ctl);
		if (!(ctl & TSI721_SP_CTL_PORT_LOCKOUT)) {
			ERR("dma_active must be called with LOCKOUT set.\n");
			return TSI721_FAIL;
		}
	}

	ret |= rio_lcfg_read(mp_h, TSI721_TXPKT_BDMA_CNT, 4, &clear);
	ret |= rio_lcfg_read(mp_h, TSI721_RXPKT_BRG_CNT, 4, &clear);
	for (int i = 0; i < 3; i++) {
		nanosleep(&point_one_ms, NULL);
		ret |= rio_lcfg_read(mp_h, TSI721_TXPKT_BDMA_CNT, 4,
								&dma_cnt);
		ret |= rio_lcfg_read(mp_h, TSI721_RXPKT_BRG_CNT, 4,
								&brg_cnt);
		if (dma_cnt || brg_cnt) {
			INFO("dma_active tx 0x%x rx 0x%x.\n", dma_cnt, brg_cnt);
			return TSI721_AGAIN;
		}
	}
	return TSI721_OK;
}

/* tsi721_check_empty
 *
 * Confirms that the Tsi721 inbound and outbound packet buffers are empty
 * based on the PBM status.
 *
 * Return values:
 *  1 - empty
 *  0 - not empty
 */

int tsi721_check_empty(void)
{
	uint32_t pbm_stat, pcie_stat;
	const uint32_t empty = TSI721_PBM_SP_STATUS_EG_EMPTY
				| TSI721_PBM_SP_STATUS_IG_EMPTY;
	rio_lcfg_read(mp_h, TSI721_PBM_SP_STATUS, 4, &pbm_stat);
	rio_lcfg_read(mp_h, TSI721_PCIEDCTL, 4, &pcie_stat);
	return (empty == (pbm_stat & empty))
		&& !(pcie_stat & TSI721_PCIEDCTL_TP);
}

/* tsi721_clear_status
 *
 * Clear Tsi721 error status indications SAFELY.
 *
 * Clearing any fatal error conditions when the Tsi721
 * PLM is exchanging packets and/or not in reset results in undefined
 * operation.
 *
 * Return values:
 *  0 - all went well
 *  1 - some failure occurred.
 */
int tsi721_clear_status(int in_reset)
{
	int ret = 0;;
	uint32_t plm_clr = TSI721_PLM_STATUS_OUTPUT_DEGR
			| TSI721_PLM_STATUS_MAX_DENIAL;
	uint32_t err_clr = TSI721_SP_ERR_STAT_INPUT_ERR_STOP
			| TSI721_SP_ERR_STAT_INPUT_ERR_ENCTR
			|TSI721_SP_ERR_STAT_INPUT_RS
			| TSI721_SP_ERR_STAT_OUTPUT_ERR_STOP
			| TSI721_SP_ERR_STAT_OUTPUT_ERR_ENCTR
			| TSI721_SP_ERR_STAT_OUTPUT_RS
			| TSI721_SP_ERR_STAT_OUTPUT_R
			| TSI721_SP_ERR_STAT_OUTPUT_RE
			| TSI721_SP_ERR_STAT_OUTPUT_DEGR
			| TSI721_SP_ERR_STAT_OUTPUT_DROP;

	// Note: never automatically clear errors that disable the port
	// like PORT_ERR and OUTPUT_FAIL.
	if (in_reset) {
		plm_clr |= TSI721_PLM_STATUS_DLT
			| TSI721_PLM_STATUS_LINK_INIT
			| TSI721_PLM_STATUS_MECS
			| TSI721_PLM_STATUS_OUTPUT_FAIL
			| TSI721_PLM_STATUS_PORT_ERR;
		err_clr |= TSI721_SP_ERR_STAT_PORT_ERR
			| TSI721_SP_ERR_STAT_OUTPUT_FAIL;
	}

	// Clear ERR_DET
	ret |= rio_lcfg_write(mp_h, TSI721_SP_ERR_DET, 4, 0);

	// Clear PBM STATUS events
	ret |= rio_lcfg_write(mp_h, TSI721_PBM_SP_STATUS, 4, 0x0FFFFFFF);

	// Clear PLM events not related to hot swap.
	ret |= rio_lcfg_write(mp_h, TSI721_PLM_STATUS, 4, plm_clr);

	// Clear err_stat
	ret |= rio_lcfg_write(mp_h, TSI721_SP_ERR_STAT, 4, err_clr);

	if (ret) {
		ERR("Register access failed: 0x%x\n", ret);
		return TSI721_FAIL;
	}
	return TSI721_OK;
}

/* tsi721_clear_lockout
 *
 * Clear Tsi721 PORT_LOCKOUT control bit.
 *
 * PORT_LOCKOUT is set to cause all outbound packets to drain when the
 * Tsi721 RapidIO link becomes uninitialized.
 *
 * NOTE: PORT_LOCKOUT is a fatal error conditions which must be cleared
 * when the Tsi721 PLM is not exchanging packets and the physical layer
 * is in reset!
 *
 * Return values:
 *  0 - all went well
 *  1 - some failure occurred.
 */

int tsi721_clear_lockout(void)
{
	int ret = 0;
	uint32_t plm_ctl, sp_ctl;
	const struct timespec delay = {0, DLT_DELAY_NSEC};

	DBG("7TEST: Clear Port Lockout\n");

	if (!tsi721_check_empty()) {
		ERR("Tsi721 is not empty!!!\n");
		return TSI721_AGAIN;
	}

	// Keep port in reset when PORT_LOCKOUT is removed.
	ret |= rio_lcfg_read(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4, &plm_ctl);
	ret |= rio_lcfg_write(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4,
			plm_ctl | TSI721_PLM_IMP_SPEC_CTL_SOFT_RST);

	ret |= rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &sp_ctl);
	sp_ctl &= ~TSI721_SP_CTL_PORT_LOCKOUT;
	ret |= rio_lcfg_write(mp_h, TSI721_SP_CTL, 4, sp_ctl);
	ret |= tsi721_clear_status(1);

	ret |= rio_lcfg_write(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4, plm_ctl);

	nanosleep(&delay, NULL);

	if (ret) {
		ERR("Register read/write problem %d\n", ret);
		return TSI721_FAIL;
	}
	return TSI721_OK;
}

/* tsi721_clear_port_dis
 *
 * Clear Tsi721 PORT_DISABLE control bit.
 *
 * PORT_DISABLE is set by software to cause the Tsi721 RapidIO link as
 * part of hot swap fault insertion testing.
 *
 * NOTE: PORT_DISABLE is a fatal error conditions which must be cleared
 * when the Tsi721 PLM is not exchanging packets and the physical layer
 * is in reset!
 *
 * Return values:
 *  0 - all went well
 *  1 - some failure occurred.
 */

uint32_t tsi721_clear_port_dis(void)
{
	uint32_t ret = 0;
	uint32_t sp_ctl, pbm_ctl;
	const struct timespec delay = {0, DLT_DELAY_NSEC};

	if (!tsi721_check_empty()) {
		ERR("Tsi721 is not empty!!!\n");
		return TSI721_AGAIN;
	}

	// Keep port in reset when PORT_LOCKOUT is removed.
	ret |= rio_lcfg_read(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4, &pbm_ctl);
	ret |= rio_lcfg_write(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4,
			pbm_ctl | TSI721_PLM_IMP_SPEC_CTL_SOFT_RST);

	ret |= rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &sp_ctl);
	sp_ctl &= ~TSI721_SP_CTL_PORT_DIS;
	ret |= rio_lcfg_write(mp_h, TSI721_SP_CTL, 4, sp_ctl);
	ret |= tsi721_clear_status(1);

	ret |= rio_lcfg_write(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4, pbm_ctl);
	nanosleep(&delay, NULL);

	if (ret) {
		ERR("Register read/write problem %d\n", ret);
		return TSI721_FAIL;
	}
	return TSI721_OK;
}

/* tsi721_config_port_reset
 *
 * Configure the Tsi721 physical layer to correctly handle reset requests.
 *
 * NOTE: If the Tsi721 is not configured correctly, and a reset request is
 * received, a PCIe fundamental reset will occur resulting in loss of access
 * to the Tsi721 from the PCIe bus.
 *
 * Return values:
 *  0 - all went well
 *  1 - some failure occurred.
 */

int tsi721_config_port_reset(void)
{
	int ret;
	const uint32_t plm_ctl_oset = TSI721_PLM_IMP_SPEC_CTL;
	uint32_t plm_ctl, devctl;

	// Set up for reset
	INFO("7TEST: Configuring port reset.\n");
	// Set DEVCTL to only reset RapidIO
	ret = rio_lcfg_read(mp_h, TSI721_DEVCTL, 4, &devctl);
	devctl |= TSI721_DEVCTL_SR_RST_MODE_SRIO_ONLY;
	ret |= rio_lcfg_write(mp_h, TSI721_DEVCTL, 4, devctl);

	// Set PLM CTL to only reset the port, not all RapidIO logic
	ret |= rio_lcfg_read(mp_h, plm_ctl_oset, 4, &plm_ctl);
	plm_ctl |= TSI721_PLM_IMP_SPEC_CTL_PORT_SELF_RST;
	ret |= rio_lcfg_write(mp_h, plm_ctl_oset, 4, plm_ctl);

	if (ret) {
		ERR("tsi721_config_port_reset FAILED\n");
		return TSI721_FAIL;
	}
	return TSI721_OK;
}

/* tsi721_reset_own_port_from_cli
 *
 * Resets the Tsi721 port and clears error status.
 *
 * Return values:
 *  0 - all went well
 *  1 - some failure occurred.
 */

int tsi721_reset_own_port_from_cli(void)
{
	uint32_t ret = 0;
	uint32_t pbm_ctl;
	const struct timespec delay = {0, DLT_DELAY_NSEC};

	if (!tsi721_check_empty()) {
		ERR("Tsi721 is not empty!!!\n");
		return TSI721_AGAIN;
	}

	// Toggle local reset control
	ret |= rio_lcfg_read(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4, &pbm_ctl);
	ret |= rio_lcfg_write(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4,
			pbm_ctl | TSI721_PLM_IMP_SPEC_CTL_SOFT_RST);
	ret |= tsi721_clear_status(1);
	ret |= rio_lcfg_write(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4, pbm_ctl);

	nanosleep(&delay, NULL);

	if ( ret) {
		ERR("Register access failed\n");
		return TSI721_FAIL;
	}
	return TSI721_OK;
}

/* tsi721_link_reset
 *
 * Resets the Tsi721 link partner, then resets the Tsi721 RapidIO port
 * and clears error status.
 *
 * Return values:
 *  0 - all went well
 *  1 - some failure occurred.
 */

int tsi721_link_reset(void)
{
	int ret = 0;
	int lim = 10000;
	uint32_t response;
	uint32_t pbm_ctl;
	const struct timespec delay = {0, DLT_DELAY_NSEC};

	ret = tsi721_check_empty();
	if (!ret) {
		ERR("Tsi721 is not empty!!!\n");
		goto exit;
	}

	// Clear response status
	ret = rio_lcfg_read(mp_h, TSI721_SP_LM_RESP, 4, &response);

	// Make link request
	ret = rio_lcfg_write(mp_h, TSI721_SP_LM_REQ, 4,
					STYPE1_LREQ_CMD_RST_DEV);
	do {
		ret |= rio_lcfg_read(mp_h, TSI721_SP_LM_RESP, 4, &response);
	} while (!ret && (--lim > 1) &&
			!(response & TSI721_SP_LM_RESP_RESP_VLD));

	// Reset own port by toggling soft reset control.
	ret |= rio_lcfg_read(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4, &pbm_ctl);
	ret |= rio_lcfg_write(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4,
			pbm_ctl | TSI721_PLM_IMP_SPEC_CTL_SOFT_RST);
	ret |= tsi721_clear_status(1);
	ret |= rio_lcfg_write(mp_h, TSI721_PLM_IMP_SPEC_CTL, 4, pbm_ctl);

	nanosleep(&delay, NULL);

	if (ret) {
		ERR("tsi721_link_reset failed.\n");
		return TSI721_FAIL;
	}
	if (!(response & TSI721_SP_LM_RESP_RESP_VLD)) {
		INFO("LP Reset request may not have been sent.\n");
	}
exit:
	return ret;
}

/* tsi721_wait_for_empty_base
 *
 * Waits until the following conditions hold:
 * - Tsi721 packet buffers are empty
 * - DMA engines are not sending packets
 * - No packets are being received
 * - PORT_LOCKOUT is set (optional)
 *
 * Return values:
 *  0 - The Tsi721 may be safely reset
 *  1 - The Tsi721 may NOT be reset, and/or a failure occurred.
 */

int tsi721_wait_for_empty_base(struct worker *info, int check_port_lockout)
{

	int rc = -1;
	int empty;
	const struct timespec point_one_ms = {0, 100000};
	// Check that no DMA is flowing any more...
	int dma_active;
	int limit = 1000000;
	int info_ok;

	DBG("Waiting for OK, Idle, and Empty\n");
	do {
		nanosleep(&point_one_ms, NULL);
		dma_active = tsi721_dma_active(check_port_lockout);
		empty = tsi721_check_empty();
		if (NULL == info) {
			info_ok = 1;
			limit = 1000000;
		} else {
			info_ok = !info->port_ok && !info->stop_req;
		}
		DBG("E %d D %d I %d L %d\n", empty, dma_active, info_ok, limit);
	} while ((!empty || (TSI721_AGAIN == dma_active)) && info_ok && limit--);

	if (info && info->stop_req) {
		ERR("Wait for empty did not complete, aborting.\n");
		goto exit;
	}
	if (empty && !dma_active)
		rc = 0;
exit:
	return rc;
}

/* tsi721_wait_for_empty
 *
 * Waits until the Tsi721 may be safely reset.
 * Does not require PORT_LOCKOUT to be set.
 *
 * Return values:
 *  0 - The Tsi721 may be safely reset
 *  1 - The Tsi721 may NOT be reset, and/or a failure occurred.
 */

int tsi721_wait_for_empty(struct worker *info)
{
	return tsi721_wait_for_empty_base(info, 0);
}

/* tsi721_wait_for_empty_and_unlock
 *
 * Waits until the Tsi721 may be safely reset, then safely clears
 * the PORT_LOCKOUT bit with a reset.
 *
 * Return values:
 *  0 - The Tsi721 was reset and PORT_LOCKOUT is cleared
 *  1 - A failure occurred.
 */

int tsi721_wait_for_empty_and_unlock(struct worker *info)
{
	int rc = TSI721_FAIL;
	uint32_t ret;

	ret = tsi721_wait_for_empty_base(info, 1);
	if (ret) {
		ERR("tsi721_wait_for_empty fail %d\n", ret);
		goto exit;
	}
	ret = tsi721_clear_lockout();
	if (ret) {
		ERR("Register read/write problem %d\n", ret);
		goto exit;
	}
	rc = TSI721_OK;
exit:
	return rc;
}

/* tsi721_clear_all_from_cli
 *
 * Clears all possible fault insertion conditions on the Tsi721,
 * then resets both ends of the RapidIO link.
 *
 * Return values:
 *  0 - All fault insertion conditions have been cleared.
 *  1 - A failure occurred.
 */

int tsi721_clear_all_from_cli(void)
{
	uint32_t ret, ctl, ibw;

	// Clear inbound window error condition
	ret = rio_lcfg_read(mp_h, TSI721_IBWIN_LBX(0), 4, &ibw);
	ret |= rio_lcfg_write(mp_h, TSI721_IBWIN_LBX(0), 4, ibw & ~0x10000000);

	// Clear response timeout error condition
	ret |= rio_lcfg_write(mp_h, TSI721_TLM_SP_FTYPE_FILTER_CTL, 4, 0);

	ret |= rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &ctl);
	ret |= rio_lcfg_write(mp_h, TSI721_SP_CTL, 4,
					ctl | TSI721_SP_CTL_PORT_LOCKOUT);
	// Push PCIe writes above to completion
	ret |= rio_lcfg_read(mp_h, TSI721_SP_CTL, 4, &ctl);
	ret |= tsi721_wait_for_empty_and_unlock(NULL);
	if (ret)
		goto exit;
	ret |= tsi721_clear_port_dis();
	ret |= tsi721_clear_lockout();
	ret |= tsi721_link_reset();
exit:
	return ret;
}

/* tsi721_link_req
 *
 * Sends a standard RapidIO link-request/input-status control symbol,
 * and returns the response value.
 *
 * Return values:
 *  0 - Response is valid.
 *  1 - A failure occurred.
 */

uint32_t tsi721_link_req(uint32_t *response)
{
        uint32_t ret = 0;
        int lim = 10000;

        ret = rio_lcfg_read(mp_h, TSI721_SP_LM_RESP, 4, response);
        ret = rio_lcfg_write(mp_h, TSI721_SP_LM_REQ, 4,
                        STYPE1_LREQ_CMD_PORT_STAT);
        do {
                ret |= rio_lcfg_read(mp_h, TSI721_SP_LM_RESP, 4, response);
        } while (!ret && (--lim > 1) &&
                !(*response & TSI721_SP_LM_RESP_RESP_VLD));
	DBG("ret %d lim %d response 0x%x\n", ret, lim, *response);
        return ret | (!(*response & TSI721_SP_LM_RESP_RESP_VLD));
}

/* tsi721_sync_cps_ackids
 *
 * Resynchronizes ackIDs between a CPS1848 port and a Tsi721 port.
 *
 * NOTE: The CPS1848 port must not have any error conditions or configuration
 *       which prevents it from attempting to send a maintenance response
 *       packet.
 */

int tsi721_sync_cps_ackids(uint32_t port)
{
	uint32_t resp, ackid_stat;
	uint32_t cps_stat;
	DAR_DEV_INFO_t d_info;
	uint32_t ret;

	// Experimentally, 3 resets is sufficient to clear issues.
	for (int i = 0; i < 3; i++) {
		ret = tsi721_clear_all_from_cli();
		if (ret) {
			ERR("tsi721_link_reset returned 0x%x\n");
			goto exit;
		}
	}

	ret = tsi721_link_req(&resp);
	if (ret) {
		ERR("tsi721_link_req failed: 0x%x", ret);
		goto exit;
	}
	ret = rio_lcfg_read(mp_h, TSI721_SP_ACKID_STAT, 4, &ackid_stat);
	if (ackid_stat) {
		ret = ackid_stat;
		ERR("AckID status 0x%x not 0 after resets.");
		goto exit;
	}

	// Set own ackID register based on link response.
	DBG("port %d resp 0x%x\n", port, resp);
	ackid_stat = ((resp & RIO_SPX_LM_RESP_ACK_ID1) >> 5)
			& RIO_SPX_ACKID_ST_OUTB;
	DBG("resp ackid 0x%x\n", ackid_stat);
	ackid_stat = ((resp & RIO_SPX_LM_RESP_ACK_ID1) >> 5);
	cps_stat = (ackid_stat << 24) | CPS1848_PORT_X_LOCAL_ACKID_CSR_CLR;

	ackid_stat |= ((ackid_stat << 8) & RIO_SPX_ACKID_ST_OUTST);
	ackid_stat |= TSI721_SP_ACKID_STAT_CLR_OUTSTD_ACKID;

	DBG("721 ackid_stat 0x%x\n", ackid_stat);
	ret = rio_lcfg_write(mp_h, TSI721_SP_ACKID_STAT, 4, ackid_stat);
	if (ret) {
		ERR("rio_lcfg_write failed 0x%x.", ret);
		goto exit;
	}

	// Now set link partner ackID register.
	// The register address used depends n the port and the link
	// partner device.  This only works for CPS.

	ret = SRIO_API_WriteRegFunc(&d_info,
				CPS1848_PORT_X_LOCAL_ACKID_CSR(port),
				cps_stat);
	if (ret) {
		DBG("CPS Write failed 0x%x\n", ret);
		goto exit;
	}

	// Must now update our own ackIDs based on the new status of the
	// CPS switch.
	ackid_stat |= 0x01000000;
	DBG("721 ackid_stat 0x%x\n", ackid_stat);
	ret = rio_lcfg_write(mp_h, TSI721_SP_ACKID_STAT, 4, ackid_stat);
	if (ret) {
		ERR("rio_lcfg_write 2 failed 0x%x.", ret);
		goto exit;
	}
exit:
	return ret;
}

#ifdef __cplusplus
}
#endif
