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
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

#include "tok_parse.h"
#include "liblog.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parse a string token for a bounded long long (64-bit) numeric value
 *
 * @param[in] token the string representation of the numeric value
 * @param[out] value the numeric value
 * @param[in] min the minimum accepted value
 * @param[in] max the maximum accepted value
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_ulonglong(char *token, uint64_t *value, uint64_t min,
		uint64_t max, int base)
{
	uint64_t data;
	char *end = NULL;

	if ((NULL == token) || (NULL == value)) {
		if (NULL != value) {
			*value = 0;
		}
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	data = strtoull(token, &end, base);
	if (errno != 0) {
		*value = 0;
		return -1;
	}

	if (end == token || (end != NULL && *end != '\0')) {
		*value = 0;
		errno = EINVAL;
		return -1;
	}

	if ((data < min) || (data > max)) {
		*value = 0;
		errno = ERANGE;
		return -1;
	}

	*value = data;
	return 0;
}

/**
 * Parse a string token for a bounded long (32-bit) numeric value
 *
 * @param[in] token the string representation of the numeric value
 * @param[out] value the numeric value
 * @param[in] min the minimum accepted value
 * @param[in] max the maximum accepted value
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_ulong(char *token, uint32_t *value, uint32_t min, uint32_t max,
		int base)
{
	uint64_t data;
	int rc;

	if (NULL == value) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_ulonglong(token, &data, min, max, base);
	*value = (uint32_t)data;
	return rc;
}

/**
 * Parse a string token for a bounded short (16-bit) numeric value
 *
 * @param[in] token the string representation of the numeric value
 * @param[out] value the numeric value
 * @param[in] min the minimum accepted value
 * @param[in] max the maximum accepted value
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_ushort(char *token, uint16_t *value, uint16_t min, uint16_t max,
		int base)
{
	uint64_t data;
	int rc;

	if (NULL == value) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_ulonglong(token, &data, min, max, base);
	*value = (uint16_t)data;
	return rc;
}

/**
 * Parse a string token for a bounded signed long long (64-bit) numeric value
 *
 * @param[in] token the string representation of the numeric value
 * @param[out] value the numeric value
 * @param[in] min the minimum accepted value
 * @param[in] max the maximum accepted value
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_longlong(char *token, int64_t *value, int64_t min, int64_t max,
		int base)
{
	int64_t data;
	char *end = NULL;

	if ((NULL == token) || (NULL == value)) {
		if (NULL != value) {
			*value = 0;
		}
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	data = strtoll(token, &end, base);
	if (errno != 0) {
		*value = 0;
		return -1;
	}

	if (errno != 0) {
		*value = 0;
		return -1;
	}

	if (end == token || (end != NULL && *end != '\0')) {
		*value = 0;
		errno = EINVAL;
		return -1;
	}

	if ((data < min) || (data > max)) {
		*value = 0;
		errno = ERANGE;
		return -1;
	}

	*value = data;
	return 0;
}

/**
 * Parse a string token for a bounded signed long (32-bit) numeric value
 *
 * @param[in] token the string representation of the numeric value
 * @param[out] value the numeric value
 * @param[in] min the minimum accepted value
 * @param[in] max the maximum accepted value
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_long(char *token, int32_t *value, int32_t min, int32_t max,
		int base)
{
	int64_t data;
	int rc;

	if (NULL == value) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_longlong(token, &data, min, max, base);
	*value = (int32_t)data;
	return rc;
}

/**
 * Parse a string token for a bounded signed short (16-bit) numeric value
 *
 * @param[in] token the string representation of the numeric value
 * @param[out] value the numeric value
 * @param[in] min the minimum accepted value
 * @param[in] max the maximum accepted value
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_short(char *token, int16_t *value, int16_t min, int16_t max,
		int base)
{
	int64_t data;
	int rc;

	if (NULL == value) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_longlong(token, &data, min, max, base);
	*value = (int16_t)data;
	return rc;
}

/**
 * Parse a string token for a long long (64-bit) numeric value
 *
 * @param[in] token the string representation of the numeric value
 * @param[out] value the numeric value
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_ull(char *token, uint64_t *value, int base)
{
	return tok_parse_ulonglong(token, value, 0, UINT64_MAX, base);
}

/**
 * Parse a string token for a long (32-bit) numeric value
 *
 * @param[in] token the string representation of the numeric value
 * @param[out] value the numeric value
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_ul(char *token, uint32_t *value, int base)
{
	uint64_t data;
	int rc;

	if (NULL == value) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_ulonglong(token, &data, 0, UINT32_MAX, base);
	*value = (uint32_t)data;
	return rc;
}

/**
 * Parse a string token for a short (16-bit) numeric value.
 *
 * @param[in] token the string representation of the numeric value
 * @param[out] value the numeric value
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_us(char *token, uint16_t *value, int base)
{
	uint64_t data;
	int rc;

	if (NULL == value) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_ulonglong(token, &data, 0, UINT16_MAX, base);
	*value = (uint16_t)data;
	return rc;

}

/**
 * Parse a string token for a signed long long (64-bit) numeric value
 *
 * @param[in] token the string representation of the numeric value
 * @param[out] value the numeric value
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_ll(char *token, int64_t *value, int base)
{
	return tok_parse_longlong(token, value, INT64_MIN, INT64_MAX, base);
}

/**
 * Parse a string token for a signed long (32-bit) numeric value
 *
 * @param[in] token the string representation of the numeric value
 * @param[out] value the numeric value
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_l(char *token, int32_t *value, int base)
{
	int64_t data;
	int rc;

	if (NULL == value) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_longlong(token, &data, INT32_MIN, INT32_MAX, base);
	*value = (int32_t)data;
	return rc;
}

/**
 * Parse a string token for a signed short (16-bit) numeric value.
 *
 * @param[in] token the string representation of the numeric value
 * @param[out] value the numeric value
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_s(char *token, int16_t *value, int base)
{
	int64_t data;
	int rc;

	if (NULL == value) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_longlong(token, &data, INT16_MIN, INT16_MAX, base);
	*value = (int16_t)data;
	return rc;

}

/**
 * Parse a string for a destination Id value
 *
 * @param[in] token the string representation of the destination Id
 * @param[out] did the destination Id, limited to dev16_sz range
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_did(char *token, did_val_t *did, int base)
{
	uint64_t value;
	int rc;

	if (NULL == did) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_ulonglong(token, &value, 0, RIO_LAST_DEV16, base);
	*did = (did_val_t)value;
	return rc;
}

/**
 * Parse a string for a component tag value
 *
 * @param[in] token the string representation of the component tag
 * @param[out] ct the numeric representation for the component tag
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_ct(char *token, ct_t *ct, int base)
{
	uint64_t value;
	int rc;

	if (NULL == ct) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_ulonglong(token, &value, 0, UINT32_MAX, base);
	*ct = (ct_t)value;
	return rc;
}

/**
 * Parse a string for a hopcount value
 *
 * @param[in] token the string representation of the hopcount
 * @param[out] hc the numeric representation for the hopcount
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_hc(char *token, hc_t *hc, int base)
{
	uint64_t value;
	int rc;

	if (NULL == hc) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_ulonglong(token, &value, 0, HC_MP, base);
	*hc = (hc_t)value;
	return rc;
}

/**
 * Parse a string for a mport Id value
 *
 * @param[in] token the string representation of the mport Id
 * @param[out] mport_id the numeric representation for the mport Id
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_mport_id(char *token, uint32_t *mport_id, int base)
{
	uint64_t value;
	int rc;

	if (NULL == mport_id) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_ulonglong(token, &value, 0, RIO_MAX_MPORTS - 1, base);
	*mport_id = (uint32_t)value;
	return rc;
}

/**
 * Parse a string for a log level
 *
 * @param[in] token the string representation of the log level
 * @param[out] level the numeric representation for the log level
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_log_level(char *token, uint32_t *level, int base)
{
	uint64_t value;
	int rc;

	if (NULL == level) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_ulonglong(token, &value, RDMA_LL_OFF, RDMA_LL_DBG, base);
	*level = (uint32_t)value;
	return rc;
}

/**
 * Parse a string for a socket number
 *
 * @param[in] token the string representation of the socket number
 * @param[out] level the numeric representation for the socket number
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_socket(char *token, uint16_t *socket, int base)
{
	uint64_t value;
	int rc;

	if (NULL == socket) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_ulonglong(token, &value, 1, UINT16_MAX, base);
	*socket = (uint16_t)value;
	return rc;
}

/**
 * Parse a string for a port number
 *
 * @param[in] token the string representation of the port number
 * @param[out] level the numeric representation for the port number
 * @param[in] base the base of the numeric value
 *
 * @retval 0 on success, -1 on failure
 */
int tok_parse_port_num(char *token, uint32_t *port_num, int base)
{
	uint64_t value;
	int rc;

	if (NULL == port_num) {
		errno = EINVAL;
		return -1;
	}

	rc = tok_parse_ulonglong(token, &value, 0, RIO_MAX_DEV_PORT - 1, base);
	*port_num = (uint32_t)value;
	return rc;
}

#ifdef __cplusplus
}
#endif
