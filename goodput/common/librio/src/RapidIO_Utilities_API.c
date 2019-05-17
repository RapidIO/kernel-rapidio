/*
 ****************************************************************************
 Copyright (c) 2014, Integrated Device Technology Inc.
 Copyright (c) 2014, RapidIO Trade Association
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
/* Device Access Routine (DAR) Utility Routines
 
 Multiple register read and read/modify/write utilities

 Control symbol composition and parsing utilities related
 to Error Management Extensions registers
 Control symbols field value description strings

 Packet composition and parsing utilities for related
 to Error Management Extensions registers
 Packet field value description strings
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "RapidIO_Utilities_API.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *  FUNCTION: DAR_util_get_ftype()
 *
 *  DESCRIPTION:
 *        Convert DAR_pkt_type to a RapidIO ftype.
 *   
 *  PARAMETERS:
 *      pkt_type - INPUT 
 *   
 *  return VALUE:
 *      -1   - Unsupported type
 *      0-15 - Valid RapidIO FTYPE
 *      16   - Raw packet
 *
 *****************************************************************************/

#define BAD_FTYPE 0xFFFF

static uint32_t DAR_util_get_ftype(DAR_pkt_type pkt_type)
{
	uint32_t rc;
	switch (pkt_type) {
	case (pkt_raw):
		rc = 16; /* Raw, uninitialized packet */
		break;
	case (pkt_nr):
	case (pkt_nr_inc):
	case (pkt_nr_dec):
	case (pkt_nr_set):
	case (pkt_nr_clr):
		rc = 2; /* NREAD packet */
		break;
	case (pkt_nw): /* NWRITE packet */
	case (pkt_nwr):
	case (pkt_nw_swap):
	case (pkt_nw_cmp_swap):
	case (pkt_nw_tst_swap):
		rc = 5; /* NWRITE_R packet */
		break;
	case (pkt_sw):
		rc = 6; /* SWRITE packet */
		break;
	case (pkt_fc):
		rc = 7; /* Flow control packet */
		break;
	case (pkt_mr): /* Maintenance Read */
	case (pkt_mw): /* Maintenance Write */
	case (pkt_mrr): /* Maintenance Read Response, */
	case (pkt_mwr): /* Maintenance Write Response */
	case (pkt_pw):
		rc = 8; /* Port-Write */
		break;
	case (pkt_dstm):
		rc = 9; /* Data streaming packet */
		break;
	case (pkt_db):
		rc = 10; /* Doorbell (type 10) packet */
		break;
	case (pkt_msg):
		rc = 11; /* Message (Type 11) packet */
		break;
	case (pkt_resp):
	case (pkt_resp_data):
	case (pkt_msg_resp):
		rc = 13; /* Response (Type 13) packet */
		break;
	default:
		rc = BAD_FTYPE;
	}
	return rc;
}

/******************************************************************************
 *  FUNCTION: DAR_util_get_rdsize_wdptr()
 *
 *  DESCRIPTION:
 *        Convert address and a number of bytes to a RapidIO rdsize and wdptr.
 *   
 *  PARAMETERS:
 *      addr - Byte address of data to be read
 *      pkt_bytes - Number of bytes to be read.  
 *                - This is restricted to certain supported sizes by the
 *                  RapidIO protocol
 *      rdsize - Updated to encode the rdsize value to be placed into a packet.
 *      wdptr  - Updated to encode the wdptr value to be placed into a packet.
 *      NOTE: rdsize and wdptr combinations are dictated by Part 1 of the 
 *            RapidIO spec.
 *   
 *  return VALUE:
 *      N/A
 *
 *****************************************************************************/

#define BAD_SIZE 0xFFFFFFFF

static void DAR_util_get_rdsize_wdptr(uint32_t addr, uint32_t pkt_bytes,
		uint32_t *rdsize, uint32_t *wdptr)
{
	*wdptr = BAD_SIZE;
	*rdsize = BAD_SIZE;
	switch (pkt_bytes) {
	case (1):
		*wdptr = (addr & 4) >> 2;
		*rdsize = (addr & 3);
		break;
	case (2):
		if (!(addr & 1)) {
			*wdptr = (addr & 4) >> 2;
			*rdsize = 4 + (addr & 2);
		}
		break;
	case (3):
		if (!(addr & 7) || (5 == (addr & 7))) {
			*wdptr = (addr & 4) >> 2;
			*rdsize = 5;
		}
		break;
	case (4):
		if (!(addr & 7) || (4 == (addr & 7))) {
			*wdptr = (addr & 4) >> 2;
			*rdsize = 8;
		}
		break;
	case (5):
		if (!(addr & 7) || (3 == (addr & 7))) {
			*wdptr = addr & 1;
			*rdsize = 7;
		}
		break;
	case (6):
		if (!(addr & 7) || (2 == (addr & 7))) {
			*wdptr = (addr & 2) >> 1;
			*rdsize = 9;
		}
		break;
	case (7):
		if (!(addr & 7) || (1 == (addr & 7))) {
			*wdptr = addr & 1;
			*rdsize = 10;
		}
		break;
	case (8):
	case (16):
		*wdptr = (pkt_bytes / 8) - 1;
		*rdsize = 11;
		break;
	case (32):
	case (64):
		*wdptr = (pkt_bytes / 32) - 1;
		*rdsize = 12;
		break;
	case (96):
		*wdptr = 0;
		*rdsize = 13;
		break;
	case (128):
		*wdptr = 1;
		*rdsize = 13;
		break;
	case (160):
		*wdptr = 0;
		*rdsize = 14;
		break;
	case (192):
		*wdptr = 1;
		*rdsize = 14;
		break;
	case (224):
		*wdptr = 0;
		*rdsize = 15;
		break;
	case (256):
		*wdptr = 1;
		*rdsize = 15;
		break;
	default:
		break;
	}

	if ((*rdsize >= 11) && (addr & 7)) {
		*wdptr = BAD_SIZE;
		*rdsize = BAD_SIZE;
	}
}

/******************************************************************************
 *  FUNCTION: DAR_util_compute_rd_bytes_n_align()
 *
 *  DESCRIPTION:
 *      Inverse function of DAR_util_get_rdsize_wdptr, this function accepts 
 *      wptr and rdsize values and returns a number of bytes and the lowest
 *      3 bits of the address of the first valid byte.
 *   
 *  PARAMETERS:
 *      wptr - Input - wptr indicator, as per the RapidIO specification
 *      rdsize - Input - rdsize value, as per the RapidIO specification
 *      num_bytes - Output - number of valid bytes according to wptr/rdsize
 *      align     - Output - offset of first valid byte according to wptr/rdsize
 *   
 *  return VALUE:
 *       0 - Successful Decode
 *       >0 - Error code
 *
 *****************************************************************************/
#define SIZE_RC_FAIL 0xFFFFFFFF

static uint32_t DAR_util_compute_rd_bytes_n_align(uint32_t rdsize,
		uint32_t wptr, uint32_t *num_bytes, uint32_t *align)
{
	int rc = 0;

	*align = 0;

	if ((0 != wptr) && (1 != wptr)) {
		*num_bytes = 0;
		return SIZE_RC_FAIL;
	}

	switch (rdsize) {
	case 0:
	case 1:
	case 2:
	case 3:
		*num_bytes = 1;
		*align = rdsize + (wptr << 2);
		break;
	case 4:
	case 6:
		*num_bytes = 2;
		*align = (rdsize & 2) + (wptr << 2);
		break;
	case 5:
		*num_bytes = 3;
		*align = wptr ? 5 : 0;
		break;
	case 8:
		*num_bytes = 4;
		*align = wptr << 2;
		break;
	case 7:
		*num_bytes = 5;
		*align = wptr ? 3 : 0;
		break;
	case 9:
		*num_bytes = 6;
		*align = wptr ? 2 : 0;
		break;
	case 10:
		*num_bytes = 7;
		*align = wptr;
		break;
	case 11:
	case 12:
		*num_bytes = 8 << ((rdsize - 11) * 2 + wptr);
		break;
	case 13:
	case 14:
	case 15:
		*num_bytes = 64 * (rdsize - 12) + 32 + (32 * wptr);
		break;
	default:
		*num_bytes = 0;
		rc = SIZE_RC_FAIL;
	}

	return rc;
}

/******************************************************************************
 *  FUNCTION: DAR_util_get_wrsize_wdptr()
 *
 *  DESCRIPTION:
 *        Convert address and a number of bytes to a RapidIO wrsize and wdptr.
 *   
 *  PARAMETERS:
 *      addr - Byte address of data to be written
 *      pkt_bytes - Number of bytes to be written.  
 *                - This is restricted to certain supported sizes by the 
 *                  RapidIO protocol
 *        wrsize - Updated to encode the wrsize value to be placed into a 
 *                 packet.
 *      wdptr  - Updated to encode the wdptr value to be placed into a packet.
 *      NOTE: wrsize and wdptr combinations are dictated by Part 1 of the
 *            RapidIO spec.
 *   
 *  return VALUE:
 *      N/A
 *
 *****************************************************************************/

static void DAR_util_get_wrsize_wdptr(uint32_t addr, uint32_t pkt_bytes,
		uint32_t *wrsize, uint32_t *wdptr)
{
	*wrsize = BAD_SIZE;
	*wdptr = BAD_SIZE;
	switch (pkt_bytes) {
	case (0):
		break;
	case (1):
		*wdptr = (addr & 4) >> 2;
		*wrsize = (addr & 3);
		break;
	case (2):
		if (!(addr & 1)) {
			*wdptr = (addr & 4) >> 2;
			*wrsize = 4 + (addr & 2);
		}
		break;
	case (3):
		switch (addr & 7) {
		case (0):
		case (5):
			*wdptr = (addr & 4) >> 2;
			*wrsize = 5;
			break;
		default:
			break;
		}
		break;
	case (4):
		switch (addr & 7) {
		case (0):
		case (4):
			*wdptr = (addr & 4) >> 2;
			*wrsize = 8;
			break;
		default:
			break;
		}
		break;
	case (5):
		switch (addr & 7) {
		case (0):
		case (3):
			*wdptr = addr & 1;
			*wrsize = 7;
			break;
		default:
			break;
		}
		break;
	case (6):
		switch (addr & 7) {
		case (0):
		case (2):
			*wdptr = (addr & 2) >> 1;
			*wrsize = 9;
			break;
		default:
			break;
		}
		break;
	case (7):
		switch (addr & 7) {
		case (0):
		case (1):
			*wdptr = addr & 1;
			*wrsize = 10;
			break;
		default:
			break;
		}
		break;
	case (8):
	case (16):
		*wdptr = (pkt_bytes / 8) - 1;
		*wrsize = 11;
		break;
	default:
		if ((addr & 7) || (pkt_bytes & 7)) {
			*wrsize = BAD_SIZE;
			*wdptr = BAD_SIZE;
		} else {
			if (pkt_bytes <= 64) {
				*wrsize = 12;
				*wdptr = (pkt_bytes > 32);
			} else if (pkt_bytes <= 128) {
				*wrsize = 13;
				*wdptr = 1;
			} else {
				*wrsize = 15;
				*wdptr = 1;
			}
		}
	}
	if ((*wrsize >= 11) && (addr & 7)) {
		*wrsize = BAD_SIZE;
		*wdptr = BAD_SIZE;
	}
}

/******************************************************************************
 *  FUNCTION: DAR_util_compute_wr_bytes_n_align()
 *
 *  DESCRIPTION:
 *      Inverse function of DAR_util_get_wrsize_wdptr, this function accepts 
 *      wptr and rdsize values and returns a number of bytes and the lowest 
 *      3 bits of the address of the first valid byte.
 *   
 *  PARAMETERS:
 *      wptr - Input - wptr indicator, as per the RapidIO specification
 *      wrsize - Input - wrsize value, as per the RapidIO specification
 *      num_bytes - Output - number of valid bytes according to wptr/rdsize
 *      align     - Output - offset of first valid byte according to wptr/rdsize
 *   
 *  return VALUE:
 *       RIO_SUCCESS - valid wrszie/wptr/num_bytes combination
 *       RIO_ERROR
 *
 *****************************************************************************/

static uint32_t DAR_util_compute_wr_bytes_n_align(uint32_t wrsize,
		uint32_t wptr, uint32_t *num_bytes, uint32_t *align)
{
	int rc = 0;

	*align = 0;

	if ((0 != wptr) && (1 != wptr)) {
		*num_bytes = 0;
		return SIZE_RC_FAIL;
	}

	switch (wrsize) {
	case 0:
	case 1:
	case 2:
	case 3:
		*num_bytes = 1;
		*align = wrsize + (wptr << 2);
		break;
	case 4:
	case 6:
		*num_bytes = 2;
		*align = (wrsize & 2) + (wptr << 2);
		break;
	case 5:
		*num_bytes = 3;
		*align = wptr ? 5 : 0;
		break;
	case 8:
		*num_bytes = 4;
		*align = wptr << 2;
		break;
	case 7:
		*num_bytes = 5;
		*align = wptr ? 3 : 0;
		break;
	case 9:
		*num_bytes = 6;
		*align = wptr ? 2 : 0;
		break;
	case 10:
		*num_bytes = 7;
		*align = wptr;
		break;
	case 11:
	case 12:
		*num_bytes = 8 << ((wrsize - 11) * 2 + wptr);
		break;
	case 13:
		if (wptr) {
			*num_bytes = 128;
		} else {
			*num_bytes = 0;
			rc = SIZE_RC_FAIL;
		}
		break;
	case 15:
		if (wptr) {
			*num_bytes = 256;
		} else {
			*num_bytes = 0;
			rc = SIZE_RC_FAIL;
		}
		break;
	default:
		*num_bytes = 0;
		rc = SIZE_RC_FAIL;
	}

	return rc;
}

/******************************************************************************
 *  FUNCTION: DAR_util_pkt_bytes_init()
 *
 *  DESCRIPTION:
 *        Initialize the fields of a composed packet.
 *   
 *  PARAMETERS:
 *      comp_pkt - Output, packet that has been initialized.
 *                 Packet has 0 bytes after initialization
 *   
 *  return VALUE:
 *      None, initialization always succeeds
 *
 *****************************************************************************/

static void DAR_util_pkt_bytes_init(DAR_pkt_bytes_t *comp_pkt)
{
	memset(comp_pkt, 0, sizeof(DAR_pkt_bytes_t));
	comp_pkt->num_chars = 0xFFFFFFFF;
	comp_pkt->pkt_addr_size = rio_addr_34;
}

/******************************************************************************
 *  FUNCTION: DAR_add_rw_addr()
 *
 *  DESCRIPTION:
 *        Add an address to an NREAD, NWRITE, NWRITE_R, SWRITE or 
 *        Maintenance packet
 *
 *  PARAMETERS:
 *      pkt_descr - Input, description of the packet to be composed
 *      comp_pkt - Output, packet that has been composed.
 *                 Packet is composed in endian-agnostic byte order.
 *      wptr     - RapidIO word pointer value
 *
 *****************************************************************************/

static uint32_t DAR_add_rw_addr(rio_addr_size pkt_addr_size, uint32_t *addr,
		uint32_t wptr, DAR_pkt_bytes_t *bytes_out)
{
	uint32_t rc = 0;
	uint32_t xaddr_xamsbs = 0;

	if ((0 != wptr) && (1 != wptr)) {
		return DAR_UTIL_INVALID_WPTR;
	}

	// Add Transaction Address

	switch (pkt_addr_size) {
	case rio_addr_66:
		xaddr_xamsbs = addr[2] & 3;
		bytes_out->pkt_data[bytes_out->num_chars++] =
				(uint8_t)((addr[1] >> 24) & 0x000000FF);
		bytes_out->pkt_data[bytes_out->num_chars++] =
				(uint8_t)((addr[1] >> 16) & 0x000000FF);
		//no break

	case rio_addr_50:
		if (rio_addr_50 == pkt_addr_size) {
			xaddr_xamsbs = (uint8_t)((addr[1] >> 16) & 0x00000003);
		}

		bytes_out->pkt_data[bytes_out->num_chars++] =
				(uint8_t)((addr[1] >> 8) & 0x000000FF);
		bytes_out->pkt_data[bytes_out->num_chars++] =
				(uint8_t)(addr[1] & 0x000000FF);
		//no break

	case rio_addr_34:
		if (rio_addr_34 == pkt_addr_size) {
			xaddr_xamsbs = (uint8_t)(addr[1] & 0x00000003);
		}
		//no break

	case rio_addr_32:
		bytes_out->pkt_data[bytes_out->num_chars++] =
				(uint8_t)((addr[0] >> 24) & 0x000000FF);
		//no break

	case rio_addr_21: // Maintenance transaction
		bytes_out->pkt_data[bytes_out->num_chars++] =
				(uint8_t)((addr[0] & 0x00FF0000) >> 16);
		bytes_out->pkt_data[bytes_out->num_chars++] =
				(uint8_t)((addr[0] & 0x0000FF00) >> 8);
		bytes_out->pkt_data[bytes_out->num_chars++] =
				(uint8_t)((addr[0] & 0x000000F8) + ((wptr & 1) << 2) + xaddr_xamsbs);
		break;

	default:
		rc = DAR_UTIL_BAD_ADDRSIZE;
	}

	bytes_out->pkt_addr_size = pkt_addr_size;
	return rc;
}

/******************************************************************************
 *  FUNCTION: DAR_get_rw_addr()
 *
 *  DESCRIPTION:
 *        Extract an address to an NREAD, NWRITE, NWRITE_R, SWRITE or 
 *        Maintenance packet
 *   
 *  PARAMETERS:
 *      fields_out - Output, Updated with the description of the address
 *      bytes_in - Input, packet that has an address in it.
 *                 Packet is composed in endian-agnostic byte order.
 *      bidx  - Index in comp_pkt->pkt_data at which the address starts
 *   
 *  return VALUE:
 *      Index of first byte after the address
 *
 *****************************************************************************/

static int DAR_get_rw_addr(DAR_pkt_bytes_t *bytes_in,
		DAR_pkt_fields_t *fields_out, uint32_t bidx)
{
	int index;
	uint32_t lbidx;

	// Extract Transaction Address from a stream of bytes

	lbidx = bidx;
	fields_out->log_rw.pkt_addr_size = bytes_in->pkt_addr_size;
	for (index = 0; index < 3; index++) {
		fields_out->log_rw.addr[index] = 0;
	}

	switch (fields_out->log_rw.pkt_addr_size) {
	case rio_addr_66:
		fields_out->log_rw.addr[2] =
				bytes_in->pkt_data[lbidx + 7] & 3;
		fields_out->log_rw.addr[1] |=
				(bytes_in->pkt_data[lbidx++] << 24);
		fields_out->log_rw.addr[1] |=
				(bytes_in->pkt_data[lbidx++] << 16);
		//no break

	case rio_addr_50:
		if (rio_addr_50 == fields_out->log_rw.pkt_addr_size) {
			fields_out->log_rw.addr[1] =
					(bytes_in->pkt_data[lbidx + 5] & 3) << 16;
		}

		fields_out->log_rw.addr[1] |=
				(bytes_in->pkt_data[lbidx++] << 8);
		fields_out->log_rw.addr[1] |=
				bytes_in->pkt_data[lbidx++];
		//no break

	case rio_addr_34:
		if (rio_addr_34 == fields_out->log_rw.pkt_addr_size) {
			fields_out->log_rw.addr[1] |=
					(bytes_in->pkt_data[lbidx + 3] & 3);
		}
		//no break

	case rio_addr_32:
		fields_out->log_rw.addr[0] =
				(bytes_in->pkt_data[lbidx++] << 24);
		//no break

	case rio_addr_21:
		fields_out->log_rw.addr[0] |=
				((uint32_t)(bytes_in->pkt_data[lbidx++]) << 16);
		fields_out->log_rw.addr[0] |=
				((uint32_t)(bytes_in->pkt_data[lbidx++]) << 8);
		fields_out->log_rw.addr[0] |=
				(uint32_t)(bytes_in->pkt_data[lbidx++]) & 0xF8;
		break;

	default:
		lbidx = DAR_UTIL_BAD_ADDRSIZE;
	}

	return lbidx;
}

/****************************************************************************
 *
 *  Control Symbols CRC definitions and support
 *
 ****************************************************************************/

/* Values from RapidIO 2.1 Part 6 Table 3-10 */

#define NUM_CRC13_VECTORS 35

int CS_13bit_crc_bit_mask[NUM_CRC13_VECTORS] = {
		0x0888,
		0x0444,
		0x0222,
		0x0111,
		0x121A,
		0x090D,
		0x1614,
		0x0B0A,
		0x0585,
		0x1050,
		0x0828,
		0x0414,
		0x020a,
		0x0105,
		0x1210,
		0x0908,
		0x0484,
		0x0242,
		0x0121,
		0x1202,
		0x0901,
		0x1612,
		0x0b09,
		0x1716,
		0x0b8b,
		0x1757,
		0x1939,
		0x1e0e,
		0x0f07,
		0x1511,
		0x181a,
		0x0c0d,
		0x1494,
		0x0a4a,
		0x0525,
};

/* Compute_13bit_crc expects the first 32 bits of the control symbol data to be
 in msbits, and the least 3 bits of the control symbol payload to be in the
 most significant bits of lsbits.
 */
static int compute_13bit_crc(uint32_t msbits, uint32_t lsbits)
{
	uint32_t crc = 0x1FFF, i;
	uint32_t bit = 0x80000000;

	lsbits &= 0xE0000000;

	for (i = 0; i < 32; i++) {
		if (msbits & bit) {
			crc = crc ^ CS_13bit_crc_bit_mask[i];
		}

		if ((lsbits & bit) && (i + 32 < NUM_CRC13_VECTORS)) {
			crc = crc ^ CS_13bit_crc_bit_mask[i + 32];
		}

		bit = bit >> 1;
	}

	return crc;
}

int CS_5bit_crc_bit_mask[5] = {
		0xB8536,
		0x70A67,
		0x591EA,
		0xB23B6,
		0xDC29A,
};

int CS_5bit_crc_invert_mask[5] = {
		0x00000,
		0x00002,
		0x00002,
		0x00002,
		0x00000,
};

static int count_bits(int val)
{
	int rc = 0, i;

	if (val) {
		for (i = 0; i < 16; i++) {
			rc = ((1 << i) & val) ? rc + 1 : rc;
		}
	}
	return rc;
}

/* Accepts 23 bits of Control Symbol data, and returns 5 bits of CRC.
 The 23 bits of Control Symbol data must be left shifted by 1 to create
 a 24 bit quantity.  This is consistent with the algorithm presented in
 the RapidIO specification.
 */
static int compute_5bit_crc(uint32_t small_cs_data)
{
	int i, crc = 0;

	for (i = 0; i < 5; i++) {
		crc += (count_bits(
				(small_cs_data ^ CS_5bit_crc_invert_mask[i])
						& CS_5bit_crc_bit_mask[i]) & 1)
				<< (4 - i);
	}
	return crc;
}

/****************************************************************************
 *
 *  Control symbol composition and parsing routines
 *
 ****************************************************************************/

/* Data portion of K28.3 Special Character, used to
 * delimit control symbols which indicate start/end of a packet
 */
#define PD_CONTROL_SYMBOL       0x7C

/* Data portion of K28.0 Special Character, used to
 delimit control symbols which do not indicate the start/end of a packet
 */
#define SC_START_CONTROL_SYMBOL 0x1C

#define SHORT_CS_VALUE(x,y,z) ((x << 11)+(y << 3)+((z&0xE0)>>5))
#define LONG_CS_VALUE_MS(w,x,y,z) ((w<<24)+(x<<16)+(y<<8)+z)
#define LONG_CS_VALUE_LS(x) ((x & 0xE0)<<24)

uint32_t CS_fields_to_bytes(CS_field_t *fields_in, CS_bytes_t *bytes_out)
{
	int large_crc;
	bytes_out->cs_type_valid = fields_in->cs_size;

	if (cs_invalid == fields_in->cs_size) {
		return RIO_ERR_INVALID_PARAMETER;
	}

	if ((stype1_sop == fields_in->cs_t1)
			|| (stype1_stomp == fields_in->cs_t1)
			|| (stype1_eop == fields_in->cs_t1)
			|| (stype1_lreq == fields_in->cs_t1)) {
		bytes_out->cs_bytes[0] = PD_CONTROL_SYMBOL;
	} else {
		bytes_out->cs_bytes[0] = SC_START_CONTROL_SYMBOL;
	}

	if (cs_small == fields_in->cs_size) {
		bytes_out->cs_bytes[1] =
				(uint8_t)((((char)(fields_in->cs_t0) & 7) << 5)
						| (fields_in->parm_0 & 0x1F));
		bytes_out->cs_bytes[2] =
				(uint8_t)(((fields_in->parm_1 & 0x1F) << 3)
						| ((uint8_t)(fields_in->cs_t1) & 7));
		bytes_out->cs_bytes[3] = (uint8_t)((fields_in->cs_t1_cmd
				& 7) << 5);

		bytes_out->cs_bytes[3] +=
				compute_5bit_crc(
					SHORT_CS_VALUE(
							bytes_out->cs_bytes[1],
							bytes_out->cs_bytes[2],
							bytes_out->cs_bytes[3]));
	} else {
		bytes_out->cs_bytes[1] =
				(uint8_t)((((char)(fields_in->cs_t0) & 7) << 5)
						| ((fields_in->parm_0 & 0x3E) >> 1));
		bytes_out->cs_bytes[2] =
				(uint8_t)(((fields_in->parm_0 & 1) << 7)
						| ((fields_in->parm_1 & 0x3F) << 1)
						| (((char)(fields_in->cs_t1) & 4) >> 2));
		bytes_out->cs_bytes[3] =
				(uint8_t)((((char)(fields_in->cs_t1) & 3) << 6)
						| ((fields_in->cs_t1_cmd & 7) << 3)
						| (((char)(fields_in->cs_t2) & 7)));

		bytes_out->cs_bytes[4] =
				(uint8_t)((fields_in->cs_t2_val & 0x7F8) >> 3);
		bytes_out->cs_bytes[5] =
				(uint8_t)((fields_in->cs_t2_val & 7) << 5);

		large_crc =
				compute_13bit_crc(
						LONG_CS_VALUE_MS(
								bytes_out->cs_bytes[1],
								bytes_out->cs_bytes[2],
								bytes_out->cs_bytes[3],
								bytes_out->cs_bytes[4]),
						LONG_CS_VALUE_LS(
								bytes_out->cs_bytes[5]));

		bytes_out->cs_bytes[6] = (uint8_t)(large_crc & 0x00FF);
		bytes_out->cs_bytes[5] += (uint8_t)((large_crc & 0x1F00) >> 8);
		bytes_out->cs_bytes[7] = bytes_out->cs_bytes[0];
	}
	return RIO_SUCCESS;
}

uint32_t CS_bytes_to_fields(CS_bytes_t *bytes_in, CS_field_t *fields_out)
{
	uint32_t rc = RIO_SUCCESS;
	int byte_offset = 0;
	bool control_symbol = false;

	fields_out->cs_size = bytes_in->cs_type_valid;

	if (cs_invalid != bytes_in->cs_type_valid) {
		if ((PD_CONTROL_SYMBOL == bytes_in->cs_bytes[0])
				|| (SC_START_CONTROL_SYMBOL == bytes_in->cs_bytes[0])) {
			byte_offset = 1;
			control_symbol = true;
		}

		if (cs_small == bytes_in->cs_type_valid) {
			fields_out->cs_t0 =
					(stype0)((bytes_in->cs_bytes[0 + byte_offset] & 0xE0) >> 5);
			fields_out->parm_0 =
					bytes_in->cs_bytes[0 + byte_offset] & 0x1F;
			fields_out->parm_1 =
					(bytes_in->cs_bytes[1 + byte_offset] & 0xF8) >> 3;
			fields_out->cs_t1 =
					(stype1)(bytes_in->cs_bytes[1 + byte_offset] & 0x07);
			fields_out->cs_t1_cmd =
					(bytes_in->cs_bytes[2 + byte_offset] & 0xE0) >> 5;
			fields_out->cs_t2 = (stype2)(0);
			fields_out->cs_t2_val = 0;

			fields_out->cs_crc_correct =
					compute_5bit_crc(
							SHORT_CS_VALUE(
									bytes_in->cs_bytes[0 + byte_offset],
									bytes_in->cs_bytes[1 + byte_offset],
									bytes_in->cs_bytes[2 + byte_offset]))
							== (bytes_in->cs_bytes[2 + byte_offset] & 0x1F);
		} else {
			fields_out->cs_t0 =
					(stype0)((bytes_in->cs_bytes[0 + byte_offset] & 0xE0) >> 5);
			fields_out->parm_0 =
					((bytes_in->cs_bytes[0 + byte_offset] & 0x1F) << 1)
						+ ((bytes_in->cs_bytes[1 + byte_offset] & 0x80) >> 7);
			fields_out->parm_1 =
					((bytes_in->cs_bytes[1 + byte_offset] & 0x7E) >> 1);
			fields_out->cs_t1 =
					(stype1)(((bytes_in->cs_bytes[1 + byte_offset] & 0x01) << 2)
							+ ((bytes_in->cs_bytes[2 + byte_offset] & 0xC0) >> 6));
			fields_out->cs_t1_cmd =
					(bytes_in->cs_bytes[2 + byte_offset] & 0x38) >> 3;
			fields_out->cs_t2 =
					(stype2)(bytes_in->cs_bytes[2 + byte_offset] & 0x07);
			fields_out->cs_t2_val =
					(bytes_in->cs_bytes[3 + byte_offset] << 3)
						+ ((bytes_in->cs_bytes[4 + byte_offset] & 0xE0) >> 5);

			fields_out->cs_crc_correct =
					compute_13bit_crc(
							LONG_CS_VALUE_MS(
									bytes_in->cs_bytes[0 + byte_offset],
									bytes_in->cs_bytes[1 + byte_offset],
									bytes_in->cs_bytes[2 + byte_offset],
									bytes_in->cs_bytes[3 + byte_offset]),
							LONG_CS_VALUE_LS(
									bytes_in->cs_bytes[4 + byte_offset])
							)

							== (((bytes_in->cs_bytes[4 + byte_offset] & 0x1f) << 8)
									+ bytes_in->cs_bytes[5 + byte_offset]);
		}

		if (control_symbol) {
			if ((stype1_sop == fields_out->cs_t1)
					|| (stype1_stomp == fields_out->cs_t1)
					|| (stype1_eop == fields_out->cs_t1)
					|| (stype1_lreq == fields_out->cs_t1)) {
				if (PD_CONTROL_SYMBOL != bytes_in->cs_bytes[0]) {
					rc = DAR_FIRST_IMP_SPEC_WARNING;
				}
			} else {
				if (SC_START_CONTROL_SYMBOL != bytes_in->cs_bytes[0]) {
					rc = DAR_FIRST_IMP_SPEC_WARNING;
				}
			}
		}
	}

	return rc;
}

char *stype0_strings[8] = {
		(char *)"Packet-Accepted",
		(char *)"Packet-Retry",
		(char *)"Packet-no-Accepted",
		(char *)"Reserved",
		(char *)"Status",
		(char *)"VC_Status",
		(char *)"Link_response",
		(char *)"Imp_Spec",
};

/* Strings which name the 8 types of STYPE0 control symbols */
char *get_stype0_descr(CS_field_t *fields_in)
{
	return stype0_strings[(int)(fields_in->cs_t0)];
}

#define PNA_RESERVED_IDX  0
#define MAX_PNA_CAUSE_IDX 8

char *pna_cause_strings[MAX_PNA_CAUSE_IDX + 1] = {
		(char *)"Reserved",
		(char *)"Unexpected AckID",
		(char *)"Bad CS CRC",
		(char *)"Non-mtc RX stopped",
		(char *)"Bad Pkt CRC",
		(char *)"Bad 10b char",
		(char *)"Lack of resources",
		(char *)"Lost Descrambler Sync",
		(char *)"General Error",
};

/* Strings which describe the cause of a PNA */
char *get_stype0_PNA_cause_parm1(CS_field_t *fields_in)
{
	char *rc = 0;

	if (stype0_pna == fields_in->cs_t0) {
		if (fields_in->parm_1 <= PNA_NO_DESCRAM_SYNC) {
			rc = pna_cause_strings[fields_in->parm_1];
		} else if (PNA_GENERAL_ERR == fields_in->parm_1) {
			rc = pna_cause_strings[MAX_PNA_CAUSE_IDX];
		} else {
			rc = pna_cause_strings[PNA_RESERVED_IDX];
		}
	}

	return rc;
}

/* Strings which describe the port status field of a LR */
char *lr_port_status_strings[5] = {
		(char *)"Reserved",
		(char *)"Error",
		(char *)"Retry-stopped",
		(char *)"Error-stopped",
		(char *)"OK",
};

char *get_stype0_LR_port_status_parm1(CS_field_t *fields_in)
{
	char *rc = 0;

	if (stype0_lresp == fields_in->cs_t0) {
		switch (fields_in->parm_1) {
		case 2:
			rc = lr_port_status_strings[1];
			break;
		case 4:
			rc = lr_port_status_strings[2];
			break;
		case 5:
			rc = lr_port_status_strings[3];
			break;
		case 0x10:
			rc = lr_port_status_strings[4];
			break;
		default:
			rc = lr_port_status_strings[0];
			break;
		}
	}

	return rc;
}

/* Strings which name the 8 types of STYPE1 control symbols */
char *stype1_strings[8] = {
		(char *)"Start of Packet",
		(char *)"Stomp",
		(char *)"End-of-packet",
		(char *)"restart-from-retry",
		(char *)"Link-Request",
		(char *)"Multicast-Event",
		(char *)"Reserved",
		(char *)"NOP",
};

char *get_stype1_descr(CS_field_t *fields_in)
{
	return stype1_strings[(int)(fields_in->cs_t1)];
}

/* Strings which describe the 8 Link Request command values. */

char *lreq_cmd_strings[3] = {
		(char *)"Reserved",
		(char *)"reset-device",
		(char *)"input-status",
};

char *get_stype1_lreq_cmd(CS_field_t *fields_in)
{
	char *rc = 0;

	if (stype1_lreq == fields_in->cs_t1) {
		if (3 == fields_in->cs_t1_cmd) {
			rc = lreq_cmd_strings[1];
		} else if (4 == fields_in->cs_t1_cmd) {
			rc = lreq_cmd_strings[2];
		} else {
			rc = lreq_cmd_strings[0];
		}
	}
	return rc;
}

/* Strings which name the 8 types of STYPE2 control symbols */
char *stype2_strings[3] = {
		(char *)"NOP",
		(char *)"VoQ Backpressure",
		(char *)"Reserved",
};

char *get_stype2_descr(CS_field_t *fields_in)
{
	char *rc = 0;

	if (fields_in->cs_t2 > 1) {
		rc = stype2_strings[2];
	} else {
		rc = stype2_strings[fields_in->cs_t2];
	}
	return rc;
}

/****************************************************************************
 *
 *  Packet CRC definitions and support
 *
 ****************************************************************************/

int bytewise_data_bits[16] = {
		0x11, // 0
		0x22, // 1
		0x44, // 2
		0x88, // 3
		0x10, // 4
		0x31, // 5
		0x62, // 6
		0xc4, // 7
		0x88, // 8
		0x10, // 9
		0x20, // 10
		0x40, // 11
		0x91, // 12
		0x22, // 13
		0x44, // 14
		0x88, // 15
};

int bytewise_CRC_bits[16] = {
		0x1100, // 0
		0x2200, // 1
		0x4400, // 2
		0x8800, // 3
		0x1000, // 4
		0x3100, // 5
		0x6200, // 6
		0xc400, // 7
		0x8801, // 8
		0x1002, // 9
		0x2004, // 10
		0x4008, // 11
		0x9110, // 12
		0x2220, // 13
		0x4440, // 14
		0x8880, // 15
};

static int crc_comp_bytewise(uint8_t *val, int num_bytes)
{
	int old_crc = 0xFFFF, new_crc = 0, i, j, temp;

	for (i = 0; i < num_bytes; i++) {
		for (j = 0; j < 16; j++) {
			temp = count_bits(val[i] & bytewise_data_bits[j]);
			temp = temp + count_bits(old_crc & bytewise_CRC_bits[j]);
			new_crc += (temp & 1) << j;
		}
		old_crc = new_crc;
		new_crc = 0;
	}

	return old_crc;
}

static int compute_pkt_crc(uint8_t *pkt, int num_bytes)
{
	int temp, i;

	if (num_bytes > 80) {
		temp = crc_comp_bytewise(pkt, 80);
		for (i = num_bytes; i > 79; i--) {
			pkt[i + 2] = pkt[i];
		}
		pkt[80] = (uint8_t)((temp >> 8) & 0x00ff);
		pkt[81] = (uint8_t)(temp & 0x00ff);
		num_bytes += 2;
	}

	temp = crc_comp_bytewise(pkt, num_bytes);
	pkt[num_bytes] = (uint8_t)((temp >> 8) & 0x00ff);
	pkt[num_bytes + 1] = (uint8_t)(temp & 0x00ff);

	return num_bytes + 2;
}

static void update_pkt_crc(DAR_pkt_bytes_t *bytes_in)
{
	int temp;

	if (bytes_in->num_chars > 80) {
		temp = crc_comp_bytewise(bytes_in->pkt_data, 80);
		bytes_in->pkt_data[80] = (uint8_t)((temp >> 8) & 0x00ff);
		bytes_in->pkt_data[81] = (uint8_t)(temp & 0x00ff);
	}

	if (bytes_in->pkt_padded) {
		bytes_in->pkt_data[bytes_in->num_chars - 2] =
				bytes_in->pkt_data[bytes_in->num_chars - 1] = 0;

		temp = crc_comp_bytewise(bytes_in->pkt_data, bytes_in->num_chars - 4);

		bytes_in->pkt_data[bytes_in->num_chars - 4] =
				(uint8_t)((temp >> 8) & 0x00ff);
		bytes_in->pkt_data[bytes_in->num_chars - 3] =
				(uint8_t)(temp & 0x00ff);
	} else {
		temp = crc_comp_bytewise(bytes_in->pkt_data, bytes_in->num_chars - 2);
		bytes_in->pkt_data[bytes_in->num_chars - 2] =
				(uint8_t)((temp >> 8) & 0x00ff);
		bytes_in->pkt_data[bytes_in->num_chars - 1] =
				(uint8_t)(temp & 0x00ff);
	}
}

/****************************************************************************
 *
 *  Packet composition and parsing routines
 *
 ****************************************************************************/

uint32_t DAR_pkt_fields_to_bytes(DAR_pkt_fields_t *fields_in,
		DAR_pkt_bytes_t *bytes_out)
{
	uint32_t rc = RIO_SUCCESS;

	uint32_t i;
	bool add_data = true;
	bool check;
	uint32_t data_align, data_repeat;
	uint32_t wdptr, rdsize, wrsize, data_bytes, repeat_data_bytes = 1;
	uint32_t addr_rc;
	uint32_t trans_type;
	uint32_t size_rc;
	uint32_t mtc_trans_type;

	DAR_util_pkt_bytes_init(bytes_out);

	if (pkt_raw == fields_in->pkt_type) {
		/* For a Raw packet, copy the bytes and compute the CRC */
		for (i = 0; i < fields_in->pkt_bytes; i++) {
			bytes_out->pkt_data[i] = fields_in->pkt_data[i];
		}
		bytes_out->num_chars = fields_in->pkt_bytes;
	} else {
		/* Otherwise, compose the packet based on packet type
		 Complete the Physical Layer Header, 2 bytes
		 NOTE: Packets are always composed with an ackID of 0.
		 */

		bytes_out->pkt_data[0] =
				(uint8_t)(((fields_in->phys.pkt_vc ? 1 : 0) << 1)
						| (fields_in->phys.crf ? 1 : 0));
		bytes_out->pkt_data[1] =
				(uint8_t)(((fields_in->phys.pkt_prio & 3) << 6)
						| (((uint8_t)(fields_in->trans.tt_code)) << 4)
						| (DAR_util_get_ftype(fields_in->pkt_type) & 0xF));
		bytes_out->num_chars = 2;

		/* Complete the Transport Layer Header, 2 or 4 bytes */
		switch (fields_in->trans.tt_code) {
		case (tt_small):
			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)(fields_in->trans.destID & 0xFF);
			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)(fields_in->trans.srcID & 0xFF);
			break;
		case (tt_large):
			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)((fields_in->trans.destID >> 8) & 0x00FF);
			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)(fields_in->trans.destID & 0x00FF);
			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)((fields_in->trans.srcID >> 8) & 0x00FF);
			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)(fields_in->trans.srcID & 0x00FF);
			break;
		default:
			return DAR_UTIL_INVALID_TT;
		}

		data_bytes = 0;

		if (((8 == DAR_util_get_ftype(fields_in->pkt_type))
				&& (rio_addr_21 != fields_in->log_rw.pkt_addr_size))
				|| ((8 != DAR_util_get_ftype(fields_in->pkt_type))
						&& (rio_addr_21 == fields_in->log_rw.pkt_addr_size))) {
			return DAR_UTIL_BAD_ADDRSIZE;
		}

		/* Compose the Logical Layer Header, based on packet type */
		switch (DAR_util_get_ftype(fields_in->pkt_type)) {
		case 2: /* NREAD */
			/* NREADs have no data */
			add_data = false;

			/* Add Packet Transaction Type and Size */
			switch (fields_in->pkt_type) {
			case pkt_nr:
				trans_type = 4;
				break;
			case pkt_nr_inc:
				trans_type = 0xC;
				break;
			case pkt_nr_dec:
				trans_type = 0xD;
				break;
			case pkt_nr_set:
				trans_type = 0xE;
				break;
			case pkt_nr_clr:
				trans_type = 0xF;
				break;
			default:
				trans_type = 0x0;
			}

			DAR_util_get_rdsize_wdptr(fields_in->log_rw.addr[0],
					fields_in->pkt_bytes, &rdsize, &wdptr);
			check = !fields_in->pkt_bytes
					|| (fields_in->pkt_bytes > 4)
					|| (3 == fields_in->pkt_bytes);
			if ((BAD_SIZE == rdsize) || ((trans_type > 4) && check)) {
				return DAR_UTIL_INVALID_RDSIZE;
			} else {
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)((trans_type << 4) + (rdsize & 0xF));
			}

			/* Add Source Transaction ID */
			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)(fields_in->log_rw.tid);

			/* Add Address */
			rc = DAR_add_rw_addr(fields_in->log_rw.pkt_addr_size,
					fields_in->log_rw.addr, wdptr, bytes_out);
			if (rc) {
				return rc;
			}
			break;

		case 5: /* NWRITE, NWRITE_R */
			/* Add Packet Transaction Type and Size */

			data_bytes = fields_in->pkt_bytes;
			check = (pkt_nw_swap == fields_in->pkt_type);
			check |= (pkt_nw_cmp_swap == fields_in->pkt_type);
			check |= (pkt_nw_tst_swap == fields_in->pkt_type);
			if (check && ((3 == fields_in->pkt_bytes)
					|| (fields_in->pkt_bytes > 4))) {
				return DAR_UTIL_INVALID_RDSIZE;
			}

			switch (fields_in->pkt_type) {
			case pkt_nw: /* NWRITE packet */
				trans_type = 4;
				break;
			case pkt_nwr: /* NWRITE_R packet */
				trans_type = 5;
				break;
			case pkt_nw_swap: /* ATOMIC Swap */
				trans_type = 0xC;
				break;
			case pkt_nw_cmp_swap: /* ATOMIC compare and swap */
				trans_type = 0xD;
				repeat_data_bytes = 2;
				break;
			case pkt_nw_tst_swap: /* ATOMIC Test-and-swap */
				trans_type = 0xE;
				break;
			default:
				return DAR_UTIL_UNKNOWN_TRANS;
			}
			DAR_util_get_wrsize_wdptr(fields_in->log_rw.addr[0],
					data_bytes, &wrsize, &wdptr);

			if (BAD_SIZE == wrsize) {
				return DAR_UTIL_INVALID_RDSIZE;
			} else {
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)((trans_type << 4) + (wrsize & 0xF));
			}

			/* Add Source Transaction ID */
			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)(fields_in->log_rw.tid);

			/* Add Address */
			rc = DAR_add_rw_addr(fields_in->log_rw.pkt_addr_size,
					fields_in->log_rw.addr, wdptr, bytes_out);
			if (rc) {
				return rc;
			}
			break;

		case 6: /* SWRITE has only an address, no transaction */
			/* type/size/source TID.  Add Address, knowing that
			 it must be 8 byte aligned.
			 */
			rc = DAR_add_rw_addr(fields_in->log_rw.pkt_addr_size,
					fields_in->log_rw.addr, 0, bytes_out);
			if (rc) {
				return rc;
			}
			break;

		case 7: /* Flow control packet */
			/* Flow control packets have no data */
			add_data = false;
			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)((fields_in->log_fc.fc_xon ? 0x80 : 0)
							| ((fields_in->log_fc.fc_fam & 7) << 4));
			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)(((fields_in->log_fc.fc_flow& 0x7F) << 1)
							| (fields_in->log_fc.fc_soc_is_ep ? 1 : 0));
			break;

		case 8: /* Maintenance read/write, maintenance read/write response,
		 and port-write */
			mtc_trans_type = (int)(fields_in->pkt_type) - (pkt_mr);
			/* Add Packet Transaction Type and Size */
			switch (fields_in->pkt_type) {
			case pkt_mr: /* Maintenance Read */
				DAR_util_get_rdsize_wdptr(
						fields_in->log_rw.addr[0],
						fields_in->pkt_bytes,
						&size_rc,
						&wdptr);
				if ((BAD_SIZE == size_rc) || (fields_in->pkt_bytes > 64)) {
					return DAR_UTIL_INVALID_RDSIZE;
				}
				add_data = false;
				break;

			case pkt_mw: /* Maintenance Write */
				DAR_util_get_wrsize_wdptr(
						fields_in->log_rw.addr[0],
						fields_in->pkt_bytes,
						&size_rc,
						&wdptr);
				if ((BAD_SIZE == size_rc) || (fields_in->pkt_bytes > 64)) {
					return DAR_UTIL_INVALID_RDSIZE;
				}
				break;

			case pkt_mrr: /* Maintenance Read Response */
				DAR_util_get_rdsize_wdptr(
						fields_in->log_rw.addr[0],
						fields_in->pkt_bytes,
						&size_rc,
						&wdptr);
				if ((BAD_SIZE == size_rc)
						|| !(fields_in->pkt_bytes)
						|| (fields_in->pkt_bytes > 64)) {
					return DAR_UTIL_INVALID_RDSIZE;
				}
				size_rc = (int)(fields_in->log_rw.status);
				fields_in->log_rw.addr[0] = 0;
				wdptr = 0;
				break;

			case pkt_mwr: /* Maintenance Write Response */
				add_data = false;
				size_rc = (int)(fields_in->log_rw.status);
				fields_in->log_rw.addr[0] = 0;
				wdptr = 0;
				break;

			case pkt_pw: /* Port-Write */
				DAR_util_get_wrsize_wdptr(
						fields_in->log_rw.addr[0],
						fields_in->pkt_bytes,
						&size_rc,
						&wdptr);
				if ((BAD_SIZE == size_rc)
						|| !(fields_in->pkt_bytes)
						|| (fields_in->pkt_bytes & 7)) {
					return DAR_UTIL_INVALID_RDSIZE;
				}
				break;
			default:
				return DAR_UTIL_INVALID_MTC;
			}

			/* Add transaction type, size, and source TID */

			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)((mtc_trans_type << 4) + (size_rc & 0xF));
			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)(fields_in->log_rw.tid);

			/* Add Hop Count */
			bytes_out->pkt_data[bytes_out->num_chars++] =
					fields_in->trans.hopcount;

			/* Add Address */
			if (rio_addr_21 != fields_in->log_rw.pkt_addr_size) {
				return DAR_UTIL_BAD_ADDRSIZE;
			}

			rc = DAR_add_rw_addr(fields_in->log_rw.pkt_addr_size,
					fields_in->log_rw.addr,
					wdptr,
					bytes_out);
			if (rc) {
				return rc;
			}
			data_bytes = fields_in->pkt_bytes;
			break;

		case 9: /* Data Streaming */
			/* This clause handles the special case of adding data to
			 a data streaming packet, since these packets may have
			 a data size that is not a multiple of 8.
			 */
			add_data = false;
			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)(fields_in->log_ds.dstm_COS);
			if (fields_in->log_ds.dstm_xh_seg) {
				/* Extended header...
				 Set xh bit
				 */
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)(0x04 | ((fields_in->log_ds.dstm_xh_type & 0x7) << 3));
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)((fields_in->log_ds.dstm_streamid & 0xFF00) >> 8);
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)((fields_in->log_ds.dstm_streamid & 0x00FF));
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)(((fields_in->log_ds.dstm_xh_tm_op & 0xF) << 4)
								+ (fields_in->log_ds.dstm_xh_wildcard& 0x7));
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)(fields_in->log_ds.dstm_xh_COS_mask & 0xFF);
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)(fields_in->log_ds.dstm_xh_parm1);
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)(fields_in->log_ds.dstm_xh_parm2);
			} else {
				char temp = fields_in->log_ds.dstm_start_seg ? 0x80 : 0;

				if (fields_in->log_ds.dstm_end_seg) {
					temp |= 0x40
							| ((fields_in->log_ds.dstm_odd_data_amt) ? 2 : 0)
							| ((fields_in->log_ds.dstm_pad_data_amt) ? 1 : 0);
				}
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)(temp);

				if (fields_in->log_ds.dstm_start_seg) {
					bytes_out->pkt_data[bytes_out->num_chars++] =
							(uint8_t)((fields_in->log_ds.dstm_streamid >> 8) & 0x00FF);
					bytes_out->pkt_data[bytes_out->num_chars++] =
							(uint8_t)(fields_in->log_ds.dstm_streamid & 0x00FF);
				} else if (fields_in->log_ds.dstm_end_seg) {
					bytes_out->pkt_data[bytes_out->num_chars++] =
							(uint8_t)((fields_in->log_ds.dstm_PDU_len >> 8) & 0x00FF);
					bytes_out->pkt_data[bytes_out->num_chars++] =
							(uint8_t)(fields_in->log_ds.dstm_PDU_len & 0x00FF);
				}

				for (i = 0; i < fields_in->pkt_bytes; i++) {
					bytes_out->pkt_data[bytes_out->num_chars++] =
							fields_in->pkt_data[i];
				}

				/* Check that number of bytes is consistent with Pad
				 and Odd data indications
				 */
				switch (fields_in->pkt_bytes & 3) {
				case 0:
					if (fields_in->log_ds.dstm_pad_data_amt
							|| fields_in->log_ds.dstm_odd_data_amt) {
						return DAR_UTIL_BAD_DS_DSIZE;
					}
					break;

				case 1:
					if (!fields_in->log_ds.dstm_pad_data_amt
							|| !fields_in->log_ds.dstm_odd_data_amt) {
						return DAR_UTIL_BAD_DS_DSIZE;
					}
					break;
				case 2:
					if (fields_in->log_ds.dstm_pad_data_amt
							|| !fields_in->log_ds.dstm_odd_data_amt) {
						return DAR_UTIL_BAD_DS_DSIZE;
					}
					break;
				default: /* 3 */
					if (!fields_in->log_ds.dstm_pad_data_amt
							|| fields_in->log_ds.dstm_odd_data_amt) {
						return DAR_UTIL_BAD_DS_DSIZE;
					}
					break;
				}

				if (fields_in->log_ds.dstm_end_seg
						&& fields_in->log_ds.dstm_pad_data_amt) {
					bytes_out->pkt_data[bytes_out->num_chars++] = 0;
				}
			}
			break;
		case 10: /* Doorbell */
			/* Add reserved value + source TID */
			bytes_out->pkt_data[bytes_out->num_chars++] = 0;
			bytes_out->pkt_data[bytes_out->num_chars++] =
					fields_in->log_rw.tid;

			/* Doorbells are a special case of adding data:
			 Only two bytes are added
			 */
			if (2 != fields_in->pkt_bytes) {
				return DAR_UTIL_BAD_MSG_DSIZE;
			}
			bytes_out->pkt_data[bytes_out->num_chars++] =
					fields_in->pkt_data[0];
			bytes_out->pkt_data[bytes_out->num_chars++] =
					fields_in->pkt_data[1];
			add_data = false;
			break;

		case 11: /* Message */
			/* Add mailbox/msgsize */
			if ((!fields_in->log_ms.msg_len)
					|| (fields_in->log_ms.msg_len
							&& ((fields_in->log_ms.msgseg)
									== fields_in->log_ms.msg_len))) {
				/* Single segment message, or last segment of the
				 multisegment message. Size can be any multiple of 8 bytes
				 */
				if (fields_in->pkt_bytes & 7) {
					return DAR_UTIL_BAD_MSG_DSIZE;
				}

				/* Otherwise, start figuring out the size.
				 */
				if (fields_in->pkt_bytes > 128) {
					addr_rc = 14;
				} else if (fields_in->pkt_bytes > 64) {
					addr_rc = 13;
				} else if (fields_in->pkt_bytes > 32) {
					addr_rc = 12;
				} else if (fields_in->pkt_bytes > 16) {
					addr_rc = 11;
				} else if (fields_in->pkt_bytes > 8) {
					addr_rc = 10;
				}else {
					addr_rc = 9;
				}
			} else {
				/* Middle segment of a multisegment message.
				 Size must be a power of 2 between 8 & 256.
				 */
				switch (fields_in->pkt_bytes) {
				case 8:
					addr_rc = 9;
					break;
				case 16:
					addr_rc = 10;
					break;
				case 32:
					addr_rc = 11;
					break;
				case 64:
					addr_rc = 12;
					break;
				case 128:
					addr_rc = 13;
					break;
				case 256:
					addr_rc = 14;
					break;
				default:
					return DAR_UTIL_BAD_MSG_DSIZE;
				}
			}
			bytes_out->pkt_data[bytes_out->num_chars++] =
					(uint8_t)(((fields_in->log_ms.msg_len
							& 0xF) << 4) + addr_rc);

			/* Now add mailbox fields */
			if (fields_in->log_ms.msg_len) {
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)(((fields_in->log_ms.letter
								& 3) << 6)
								+ ((fields_in->log_ms.mbid
										& 3)
										<< 4)
								+ (fields_in->log_ms.msgseg
										& 0xF));
			} else {
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)(((fields_in->log_ms.letter
								& 3) << 6)
								+ (fields_in->log_ms.mbid
										& 0x3F));
			}
			break;

		case 13: /* Response */
			/* Add Transaction type and Status */
			switch (fields_in->pkt_type) {
			case pkt_msg_resp:
				/* Message response transaction type + status */
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)((fields_in->log_ms.status
								& 0xF) | 0x10);
				if (fields_in->log_ms.msg_len) {
					/* TID is replaced by MBID, Letter,
					 and message segment
					 */
					bytes_out->pkt_data[bytes_out->num_chars++] =
							(uint8_t)((fields_in->log_ms.letter
									<< 6)
									| (fields_in->log_ms.mbid
											<< 4)
									| (fields_in->log_ms.msgseg
											& 0xF));
				} else {
					/* TID is replaced by MBID, Letter,
					 and message number;
					 */
					bytes_out->pkt_data[bytes_out->num_chars++] =
							(uint8_t)((fields_in->log_ms.letter
									<< 6)
									| (fields_in->log_ms.mbid
											& 0x3F));
				}
				add_data = false;
				if (fields_in->pkt_bytes) {
					return DAR_UTIL_BAD_RESP_DSIZE;
				}
				break;

			case pkt_resp: /* Response without data */
				if (fields_in->pkt_bytes) {
					return DAR_UTIL_BAD_RESP_DSIZE;
				}

				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)(fields_in->log_rw.status
								& 0xF);
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)(fields_in->log_rw.tid
								& 0xFF);
				add_data = false;
				break;
			case pkt_resp_data: /* Response with data */
				if ((!fields_in->pkt_bytes
						&& (pkt_err
								!= fields_in->log_rw.status))
						|| (fields_in->pkt_bytes & 7)) {
					return DAR_UTIL_BAD_RESP_DSIZE;
				}

				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)((fields_in->log_rw.status
								& 0xF) | 0x80);
				bytes_out->pkt_data[bytes_out->num_chars++] =
						(uint8_t)(fields_in->log_rw.tid
								& 0xFF);
				add_data =
						(fields_in->pkt_bytes ?
								true : false);
				break;
			default:
				return DAR_UTIL_BAD_RESP_DSIZE;
			}
			break;

		default:
			return DAR_UTIL_UNKNOWN_TRANS;
		}

		/* Now, add the data to the transaction. */

		if (add_data && fields_in->pkt_bytes) {
			data_align = fields_in->log_rw.addr[0] & 7;

			if (fields_in->pkt_bytes < 8) {
				if ((data_align + fields_in->pkt_bytes) > 8) {
					return DAR_UTIL_BAD_DATA_SIZE;
				}

				/* Repeat twice for atomic-test-and-swap transactions,
				 all others only repeat once...
				 */
				for (data_repeat = 0;
						data_repeat < repeat_data_bytes;
						data_repeat++) {
					/* Pad 8 bytes of data before and after with 0's
					 */
					for (i = 0; i < data_align; i++) {
						bytes_out->pkt_data[bytes_out->num_chars++] =
								0;
					}

					for (i = 0; i < data_bytes; i++) {
						bytes_out->pkt_data[bytes_out->num_chars++] =
								fields_in->pkt_data[i
										+ (data_repeat
												* data_bytes)];
					}

					for (i = data_align
							+ fields_in->pkt_bytes;
							i < 8; i++) {
						bytes_out->pkt_data[bytes_out->num_chars++] =
								0;
					}
				}
			} else {
				/* Must add multiples of 8 bytes of 8 byte aligned data
				 */
				if (data_align || (fields_in->pkt_bytes & 0x7)) {
					return DAR_UTIL_BAD_DATA_SIZE;
				}

				for (i = 0; i < fields_in->pkt_bytes; i++) {
					bytes_out->pkt_data[bytes_out->num_chars++] =
							fields_in->pkt_data[i];
				}
			}
		}
	}

	/* Packet composition complete,
	 now tack on the CRC
	 */
	bytes_out->num_chars = compute_pkt_crc(bytes_out->pkt_data,
			bytes_out->num_chars);
	bytes_out->pkt_has_crc = true;

	/* Make sure packet is a multiple of 4 bytes in size.
	 */
	if (bytes_out->num_chars & 2) {
		bytes_out->pkt_data[bytes_out->num_chars++] = 0;
		bytes_out->pkt_data[bytes_out->num_chars++] = 0;
		bytes_out->pkt_padded = true;
	} else {
		bytes_out->pkt_padded = false;
	}

	return rc;
}

uint32_t DAR_update_pkt_CRC(DAR_pkt_bytes_t *bytes_in)
{
	if ((!bytes_in->pkt_data) || (!bytes_in->num_chars)
			|| (bytes_in->num_chars > RIO_MAX_PKT_BYTES)
			|| ((uint8_t)(bytes_in->pkt_addr_size)
					> (uint8_t)(rio_addr_66))) {
		return RIO_ERR_INVALID_PARAMETER;
	}

	if (bytes_in->pkt_has_crc) {
		update_pkt_crc(bytes_in);
	} else {
		if (bytes_in->num_chars > RIO_MAX_PKT_BYTES - 2) {
			return RIO_ERR_INVALID_PARAMETER;
		}
		bytes_in->num_chars = compute_pkt_crc(bytes_in->pkt_data,
				bytes_in->num_chars);
	}
	return RIO_SUCCESS;
}

uint32_t DAR_pkt_bytes_to_fields(DAR_pkt_bytes_t *bytes_in,
		DAR_pkt_fields_t *fields_out)
{
	uint32_t rc = RIO_SUCCESS;
	uint32_t repeat_data_bytes = 1;
	bool get_data = true;
	bool get_exact_data = false;
	uint32_t ftype;
	uint32_t pkt_addr_msbs = 0;
	uint32_t size_rc, rdsize, i, max_bytes, align_val;

	/* Start at the head of the packet, and decompose the physical layer header.
	 */
	uint32_t pkt_index = 2;

	if (bytes_in->num_chars < 8) {
		return DAR_UTIL_BAD_DATA_SIZE;
	}

	fields_out->phys.pkt_ackID = (bytes_in->pkt_data[0] & 0xFC) >> 2;
	fields_out->phys.pkt_vc = (bytes_in->pkt_data[0] & 2) >> 1;
	fields_out->phys.crf = bytes_in->pkt_data[0] & 1;
	fields_out->phys.pkt_prio = (bytes_in->pkt_data[1] & 0xC0) >> 6;
	fields_out->trans.tt_code = (rio_TT_code)((bytes_in->pkt_data[1] & 0x30)
			>> 4);
	ftype = bytes_in->pkt_data[1] & 0x0F;

	/* Get sourceID and destination ID
	 Note: Hopcount is parsed as part of FTYPE 8 (Maintenance) packet support
	 */

	switch (fields_out->trans.tt_code) {
	case tt_small:
		fields_out->trans.destID =
				(did_reg_t)(bytes_in->pkt_data[pkt_index++]);
		fields_out->trans.srcID =
				(did_reg_t)(bytes_in->pkt_data[pkt_index++]);
		break;

	case tt_large:
		fields_out->trans.destID =
				(did_reg_t)(bytes_in->pkt_data[pkt_index++])
						<< 8;
		fields_out->trans.destID +=
				(did_reg_t)(bytes_in->pkt_data[pkt_index++]);
		fields_out->trans.srcID =
				(did_reg_t)(bytes_in->pkt_data[pkt_index++])
						<< 8;
		fields_out->trans.srcID +=
				(did_reg_t)(bytes_in->pkt_data[pkt_index++]);
		break;

	default:
		return DAR_UTIL_INVALID_TT;
	}

	/* Now get into the logical layer stuff */

	switch (ftype) {
	case 2: /* NREAD and ATOMIC transactions */
		/* Parse transaction type */
		switch ((bytes_in->pkt_data[pkt_index] & 0xF0) >> 4) {
		case 4:
			fields_out->pkt_type = pkt_nr;
			break;
		case 0xC:
			fields_out->pkt_type = pkt_nr_inc;
			break;
		case 0xD:
			fields_out->pkt_type = pkt_nr_dec;
			break;
		case 0xE:
			fields_out->pkt_type = pkt_nr_set;
			break;
		case 0xF:
			fields_out->pkt_type = pkt_nr_clr;
			break;
		default:
			return DAR_UTIL_UNKNOWN_TRANS;
		}

		size_rc = bytes_in->pkt_data[pkt_index++] & 0xF;

		fields_out->log_rw.tid = bytes_in->pkt_data[pkt_index++];

		/* Now, get the address out... */

		pkt_index = DAR_get_rw_addr(bytes_in, fields_out, pkt_index);

		if (RIO_MAX_PKT_BYTES < pkt_index) {
			return pkt_index;
		}

		/* Now translate the size_rc to the appropriate number of bytes
		 and byte address.
		 */

		size_rc = DAR_util_compute_rd_bytes_n_align(size_rc,
				(bytes_in->pkt_data[pkt_index - 1] & 4) >> 2,
				&fields_out->pkt_bytes, &pkt_addr_msbs);
		if (SIZE_RC_FAIL == size_rc) {
			return DAR_UTIL_INVALID_RDSIZE;
		}

		fields_out->log_rw.addr[0] |= pkt_addr_msbs;

		get_data = false;
		break;

	case 5: /* NWRITE, NWRITE_R, and ATOMIC swap transactions */
		size_rc = bytes_in->pkt_data[pkt_index] & 0xF;

		switch ((bytes_in->pkt_data[pkt_index++] & 0xF0) >> 4) {
		case 4:
			fields_out->pkt_type = pkt_nw;
			break;
		case 5:
			fields_out->pkt_type = pkt_nwr;
			break;
			/* ATOMIC Swap */
		case 0xC:
			fields_out->pkt_type = pkt_nw_swap;
			break;
			/* ATOMIC compare and swap */
		case 0xD:
			fields_out->pkt_type = pkt_nw_cmp_swap;
			repeat_data_bytes = 2;
			break;
			/* ATOMIC Test-and-swap */
		case 0xE:
			fields_out->pkt_type = pkt_nw_tst_swap;
			break;
		default:
			return DAR_UTIL_UNKNOWN_TRANS;
		}

		fields_out->log_rw.tid = bytes_in->pkt_data[pkt_index++];

		/* Now, get the address out... */

		pkt_index = DAR_get_rw_addr(bytes_in, fields_out, pkt_index);

		if (RIO_MAX_PKT_BYTES < pkt_index) {
			return pkt_index;
		}

		/* Now translate the size_rc to the appropriate number of bytes
		 and byte address.
		 */

		size_rc = DAR_util_compute_wr_bytes_n_align(size_rc,
				(bytes_in->pkt_data[pkt_index - 1] & 4) >> 2,
				&fields_out->pkt_bytes, &pkt_addr_msbs);
		if (SIZE_RC_FAIL == size_rc) {
			return DAR_UTIL_INVALID_RDSIZE;
		}

		fields_out->log_rw.addr[0] |= pkt_addr_msbs;

		get_data = true;
		break;

	case 6: /* SWRITE */
		/* SWRITEs have no transaction type, size, word pointer, or
		 alignment.  All data is 8 byte aligned, the only parameter is
		 the address out...
		 */
		fields_out->pkt_type = pkt_sw;
		pkt_index = DAR_get_rw_addr(bytes_in, fields_out, pkt_index);

		fields_out->pkt_bytes = 256;

		get_data = true;
		break;

	case 7: /* Flow Control */
		fields_out->pkt_type = pkt_fc;
		fields_out->log_fc.fc_xon =
				(bytes_in->pkt_data[pkt_index] & 0x80) ?
						true : false;
		fields_out->log_fc.fc_fam =
				(rio_fc_fam_t)((bytes_in->pkt_data[pkt_index++]
						& 0x70) >> 4);
		fields_out->log_fc.fc_flow =
				(rio_fc_flow_id)((bytes_in->pkt_data[pkt_index]
						& 0xF7) >> 1);
		fields_out->log_fc.fc_soc_is_ep =
				(bytes_in->pkt_data[pkt_index++] & 0x01) ?
						true : false;
		fields_out->log_fc.fc_destID = fields_out->trans.destID;
		fields_out->log_fc.fc_srcID = fields_out->trans.srcID;
		get_data = false;
		break;

	case 8: /* Maintenance Transaction */
		if (((bytes_in->pkt_data[pkt_index] & 0xF0) >> 4) > 4) {
			return DAR_UTIL_UNKNOWN_TRANS;
		}

		size_rc = bytes_in->pkt_data[pkt_index] & 0xF;

		/* Figure out which kind of maintenance transaction */
		fields_out->pkt_type =
				(DAR_pkt_type)((int)(pkt_mr)
						+ ((bytes_in->pkt_data[pkt_index++]
								& 0xF0) >> 4));

		/* Get TID */
		fields_out->log_rw.tid = bytes_in->pkt_data[pkt_index++];

		/* Then, get hopcount... */
		fields_out->trans.hopcount = bytes_in->pkt_data[pkt_index++];

		/* Get address, always 3 bytes */
		fields_out->log_rw.pkt_addr_size = rio_addr_21;
		bytes_in->pkt_addr_size = rio_addr_21;
		align_val = 0;

		pkt_index = DAR_get_rw_addr(bytes_in, fields_out, pkt_index);

		switch (fields_out->pkt_type) {
		case pkt_mr:
			size_rc = DAR_util_compute_rd_bytes_n_align(size_rc,
					(bytes_in->pkt_data[pkt_index - 1] & 4)
							>> 2,
					&fields_out->pkt_bytes, &align_val);
			if (SIZE_RC_FAIL == size_rc) {
				return DAR_UTIL_INVALID_RDSIZE;
			}
			get_data = false;
			break;

		case pkt_pw:
		case pkt_mw:
			size_rc = DAR_util_compute_wr_bytes_n_align(size_rc,
					(bytes_in->pkt_data[pkt_index - 1] & 4)
							>> 2,
					&fields_out->pkt_bytes, &align_val);
			if (SIZE_RC_FAIL == size_rc) {
				return DAR_UTIL_INVALID_RDSIZE;
			}
			get_data = true;
			break;

		case pkt_mwr:
			fields_out->pkt_bytes = 0;
			get_data = false;
			break;

		case pkt_mrr:
			fields_out->pkt_bytes = 64;
			get_data = true;
			break;
		default:
			return DAR_UTIL_UNKNOWN_TRANS;
		}
		fields_out->log_rw.addr[0] |= align_val;
		break;
	case 9: /* Data Streaming packet */
		fields_out->pkt_type = pkt_dstm;
		fields_out->log_ds.dstm_COS = bytes_in->pkt_data[pkt_index++];

		if (bytes_in->pkt_data[pkt_index] & 4) {
			/* Extended header bit is set. */
			fields_out->pkt_bytes = 0;
			fields_out->log_ds.dstm_xh_seg = true;
			fields_out->log_ds.dstm_start_seg = false;
			fields_out->log_ds.dstm_end_seg = false;
			fields_out->log_ds.dstm_odd_data_amt = false;
			fields_out->log_ds.dstm_pad_data_amt = false;
			fields_out->log_ds.dstm_streamid = 0;
			fields_out->log_ds.dstm_PDU_len = 0;

			fields_out->log_ds.dstm_xh_type =
					(bytes_in->pkt_data[pkt_index++] & 0x38)
							>> 3;
			fields_out->log_ds.dstm_streamid =
					(bytes_in->pkt_data[pkt_index++] << 8);
			fields_out->log_ds.dstm_streamid +=
					bytes_in->pkt_data[pkt_index++];
			fields_out->log_ds.dstm_xh_tm_op =
					(bytes_in->pkt_data[pkt_index] & 0xF0)
							>> 4;
			fields_out->log_ds.dstm_xh_wildcard =
					(bytes_in->pkt_data[pkt_index++] & 0x07);
			fields_out->log_ds.dstm_xh_COS_mask =
					bytes_in->pkt_data[pkt_index++];
			fields_out->log_ds.dstm_xh_parm1 =
					bytes_in->pkt_data[pkt_index++];
			fields_out->log_ds.dstm_xh_parm2 =
					bytes_in->pkt_data[pkt_index++];
			get_data = false;
		} else {
			fields_out->pkt_bytes = 256;
			fields_out->log_ds.dstm_xh_seg = false;
			fields_out->log_ds.dstm_xh_type = 0;
			fields_out->log_ds.dstm_xh_tm_op = 0;
			fields_out->log_ds.dstm_xh_wildcard = 0;
			fields_out->log_ds.dstm_xh_COS_mask = 0;
			fields_out->log_ds.dstm_xh_parm1 = 0;
			fields_out->log_ds.dstm_xh_parm2 = 0;

			fields_out->log_ds.dstm_start_seg =
					(bytes_in->pkt_data[pkt_index] & 0x80) ?
							true : false;
			fields_out->log_ds.dstm_end_seg =
					(bytes_in->pkt_data[pkt_index] & 0x40) ?
							true : false;
			fields_out->log_ds.dstm_odd_data_amt =
					(bytes_in->pkt_data[pkt_index] & 0x02) ?
							true : false;
			fields_out->log_ds.dstm_pad_data_amt =
					(bytes_in->pkt_data[pkt_index++] & 0x01) ?
							true : false;

			fields_out->log_ds.dstm_streamid = 0;
			fields_out->log_ds.dstm_PDU_len = 0;
			if (fields_out->log_ds.dstm_start_seg) {
				fields_out->log_ds.dstm_streamid =
						bytes_in->pkt_data[pkt_index++]
								<< 8;
				fields_out->log_ds.dstm_streamid +=
						bytes_in->pkt_data[pkt_index++];
			} else if (fields_out->log_ds.dstm_end_seg) {
				fields_out->log_ds.dstm_PDU_len =
						bytes_in->pkt_data[pkt_index++]
								<< 8;
				fields_out->log_ds.dstm_PDU_len +=
						bytes_in->pkt_data[pkt_index++];
			}
		}
		break;

	case 10: /* Doorbell */
		fields_out->pkt_type = pkt_db;
		/* Skip reserved byte */
		pkt_index++;
		/* Get TID */
		fields_out->log_rw.tid = bytes_in->pkt_data[pkt_index++];
		/* And now get the data */
		fields_out->pkt_bytes = 2;
		fields_out->pkt_data[0] = bytes_in->pkt_data[pkt_index++];
		fields_out->pkt_data[1] = bytes_in->pkt_data[pkt_index++];
		get_data = false;
		break;

	case 11: /* Message */
		fields_out->pkt_type = pkt_msg;

		/* Figure out message size */
		fields_out->log_ms.msg_len = (bytes_in->pkt_data[pkt_index]
				& 0xF0) >> 4;
		rdsize = bytes_in->pkt_data[pkt_index++] & 0xF;

		if (rdsize < 9) {
			return DAR_UTIL_BAD_MSG_DSIZE;
		}

		fields_out->pkt_bytes = 8 << (rdsize - 9);

		/* Now figure out mbox/letter/xmbox etc... */

		fields_out->log_ms.letter = (bytes_in->pkt_data[pkt_index]
				& 0xC0) >> 6;

		if (fields_out->log_ms.msg_len) {
			/* Multisegment message, fill out mbox and msgseg */
			fields_out->log_ms.mbid = (bytes_in->pkt_data[pkt_index]
					& 0x30) >> 4;
			fields_out->log_ms.msgseg =
					(bytes_in->pkt_data[pkt_index++] & 0x0F);
			get_exact_data = !(fields_out->log_ms.msgseg
					== fields_out->log_ms.msg_len);
		} else {
			/* Single segment message, fill out mbox only */
			fields_out->log_ms.msgseg = 0;
			fields_out->log_ms.mbid =
					(bytes_in->pkt_data[pkt_index++] & 0x3F);
			get_exact_data = false;
		}
		get_data = true;
		break;

	case 13: /* Response */
		/* Figure out transaction type, and whether there should be
		 data there to get
		 */
		get_data = false;
		switch ((bytes_in->pkt_data[pkt_index] & 0xF0) >> 4) {
		case 8: /* Response with data */
			fields_out->pkt_type = pkt_resp_data;
			get_data = true;
			fields_out->pkt_bytes = 256;
			break;
		case 0: /* Response without data */
			fields_out->pkt_type = pkt_resp;
			fields_out->pkt_bytes = 0;
			break;
		case 1:
			fields_out->pkt_type = pkt_msg_resp;
			fields_out->pkt_bytes = 0;
			break;
		default:
			return DAR_UTIL_UNKNOWN_TRANS;
		}

		/* Now figure out response status, and whether or not the
		 transaction can actually have data
		 */
		switch (bytes_in->pkt_data[pkt_index++] & 0x0F) {
		case 0:
			fields_out->log_ms.status = pkt_done;
			fields_out->log_rw.status = pkt_done;
			break;
		case 7:
			fields_out->log_ms.status = pkt_err;
			fields_out->log_rw.status = pkt_err;
			break;
		case 3:
			fields_out->log_ms.status = pkt_retry;
			fields_out->log_rw.status = pkt_retry;
			get_data = false;
			fields_out->pkt_bytes = 0;
			break;
		default:
			return DAR_UTIL_UNKNOWN_STATUS;
		}

		/* Get TID */
		if (pkt_msg_resp == fields_out->pkt_type) {
			fields_out->log_ms.letter =
					(bytes_in->pkt_data[pkt_index] & 0xC0)
							>> 6;
			fields_out->log_ms.mbid = (bytes_in->pkt_data[pkt_index]
					& 0x30) >> 4;
			fields_out->log_ms.msgseg =
					(bytes_in->pkt_data[pkt_index++] & 0x0F);
		} else {
			fields_out->log_rw.tid =
					bytes_in->pkt_data[pkt_index++];
		}
		break;

	default:
		fields_out->pkt_type = pkt_raw;
		pkt_index = 0;
		get_data = true;
		fields_out->pkt_bytes = 276;
		break;
	}

	/* Lastly, get the data, remembering to skip the CRC if necessary. */

	if (get_data) {
		max_bytes = fields_out->pkt_bytes;
		if (fields_out->pkt_bytes <= 8) {
			uint32_t j;

			for (j = 0; j < repeat_data_bytes; j++) {
				/* Must make use of address offset and number of bytes
				 to get data
				 */
				if ((pkt_dstm != fields_out->pkt_type)
						&& (pkt_msg
								!= fields_out->pkt_type)) {
					pkt_index += fields_out->log_rw.addr[0]
							& 7;
				}

				for (i = 0; i < fields_out->pkt_bytes; i++) {
					fields_out->pkt_data[i
							+ (j
									* fields_out->pkt_bytes)] =
							bytes_in->pkt_data[pkt_index++];
				}

				if ((pkt_dstm != fields_out->pkt_type)
						&& (pkt_msg
								!= fields_out->pkt_type)) {
					pkt_index +=
							8
									- (fields_out->log_rw.addr[0]
											& 7)
									- fields_out->pkt_bytes;
				}
			}
		} else {
			/* Must retrieve all data except the CRC.
			 * data_count = bytes_in->num_chars;
			 */
			fields_out->pkt_bytes = 0;

			/* Get data, excluding intermediate and final CRC as necessary
			 */
			for (i = pkt_index;
					i
							< (bytes_in->num_chars
									- (bytes_in->pkt_has_crc ?
											2 :
											0));
					i++) {
				if ((80 != i) && (81 != i)) {
					fields_out->pkt_data[fields_out->pkt_bytes++] =
							bytes_in->pkt_data[i];
				}
			}

			/* Adjust the data size as necessary for CRC and Data Streaming.
			 First, eliminate cases where we're off by 1.
			 Can't fix those...
			 */
			if (fields_out->pkt_bytes & 1) {
				return DAR_UTIL_BAD_DATA_SIZE;
			}

			/* Need to adjust number of data bytes to reflect location of CRC,
			 and for Type 9 padding etc.
			 */
			fields_out->pad_bytes = 0;

			/* Assume that if the last two bytes are zero when the packet has
			 CRC, that the packet had two pad bytes
			 */
			if (bytes_in->pkt_has_crc && (bytes_in->num_chars != 84)
					&& !bytes_in->pkt_data[bytes_in->num_chars
							- 1]
					&& !bytes_in->pkt_data[bytes_in->num_chars
							- 2]) {
				fields_out->pad_bytes = 2;
				fields_out->pkt_bytes -= 2;
			}

			/* Check for consistency with Data Streaming size, if necessary
			 */
			if (pkt_dstm == fields_out->pkt_type) {
				if (true == fields_out->log_ds.dstm_end_seg) {
					switch (fields_out->pkt_bytes & 3) {
					case 0:
						if (fields_out->log_ds.dstm_odd_data_amt) {
							return DAR_UTIL_BAD_DS_DSIZE;
						}
						if (fields_out->log_ds.dstm_pad_data_amt) {
							fields_out->pkt_bytes--;
						}
						break;
					case 2:
						if (!fields_out->log_ds.dstm_odd_data_amt) {
							return DAR_UTIL_BAD_DS_DSIZE;
						}
						if (fields_out->log_ds.dstm_pad_data_amt) {
							fields_out->pkt_bytes--;
						}
						break;

					default: /* Should never reach this clause */
						return DAR_UTIL_BAD_DS_DSIZE;
					}
				} else if (fields_out->pkt_bytes & 7)
					/* Packets other than end segments must
					 have a multiple of 8 bytes
					 */
					return DAR_UTIL_BAD_DS_DSIZE;
			}
		}
		if (fields_out->pkt_bytes > max_bytes) {
			return DAR_UTIL_BAD_DATA_SIZE;
		}

		// Check for inexact amount of data returned

		if ((fields_out->pkt_bytes != max_bytes) && get_exact_data) {
			return DAR_UTIL_BAD_DATA_SIZE;
		}
	}

	return rc;
}

/* Returns string naming packet FTYPE. */
char *pkt_ftype_strings[17] = {
		(char *)"Implementation Specific",
		(char *)"Reserved",
		(char *)"NRead",
		(char *)"Reserved",
		(char *)"Reserved",
		(char *)"NWrite",
		(char *)"SWrite",
		(char *)"Flow Control",
		(char *)"Maintenance",
		(char *)"Data Streaming",
		(char *)"Doorbell",
		(char *)"Message",
		(char *)"Reserved",
		(char *)"Response",
		(char *)"Reserved",
		(char *)"Implementation Specific",
		(char *)"RAW"
};

char *DAR_pkt_ftype_descr(DAR_pkt_fields_t *pkt_fields)
{
	int ftype = DAR_util_get_ftype(pkt_fields->pkt_type);

	if (BAD_FTYPE == ftype) {
		ftype = 1;
	}
	return pkt_ftype_strings[ftype];
}

/* Returns string naming packet transaction type.
 Works for all FTYPEs.
 If the FTYPEs does not have a transaction type,
 this routine returns NULL.
 */
char *pkt_trans_strings[(int)(pkt_type_max+1)] = {
		(char *)"",
		(char *)"NREAD",
		(char *)"ATOMIC inc",
		(char *)"ATOMIC dec",
		(char *)"ATOMIC set",
		(char *)"ATOMIC clr",
		(char *)"NWRITE",
		(char *)"NWRITE_R",
		(char *)"ATOMIC swap",
		(char *)"ATOMIC cmp swap",
		(char *)"ATOMIC tst swap",
		(char *)"",
		(char *)"",
		(char *)"MtcRead",
		(char *)"MtcWrite",
		(char *)"MtcReadResp",
		(char *)"MtcWriteResp",
		(char *)"Port-Write",
		(char *)"",
		(char *)"",
		(char *)"",
		(char *)"",
		(char *)"Resp with Data",
		(char *)"Msg Response",
};

char *DAR_pkt_trans_descr(DAR_pkt_fields_t *pkt_fields)
{
	return pkt_trans_strings[pkt_fields->pkt_type];
}

/* Returns string for response packet (FTYPE8, FTYPE 13) 
 status field (rio_pkt_status)
 If the packet passed in is not a response packet,
 the routine returns NULL.
 If the status field is invalid, this routine
 returns "UNKNOWN"
 */
char *status_strings[4] = {
		(char *)"Done",
		(char *)"Retry",
		(char *)"Error",
		(char *)"Unknown",
};

char *DAR_pkt_resp_status_descr(DAR_pkt_fields_t *pkt_fields)
{
	char *rc = status_strings[3];

	if ((pkt_mrr == pkt_fields->pkt_type)
			|| (pkt_mwr == pkt_fields->pkt_type)
			|| (pkt_resp == pkt_fields->pkt_type)
			|| (pkt_msg_resp == pkt_fields->pkt_type)) {
		if (pkt_done == pkt_fields->log_rw.status) {
			rc = status_strings[0];
		} else if ((pkt_retry == pkt_fields->log_rw.status)
				&& ((pkt_resp == pkt_fields->pkt_type)
						|| (pkt_msg_resp
								== pkt_fields->pkt_type))) {
			rc = status_strings[1];
		} else if (pkt_err == pkt_fields->log_rw.status) {
			rc = status_strings[2];
		}
	}
	return rc;
}

#ifdef __cplusplus
}
#endif

