/* Utility to query for reserved memory space availability based on keyword */
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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tok_parse.h"
#include "rrmap_config.h"
#include "rio_mport_lib.h"
#include "rio_mport_cdev.h"
#include "libcli.h"

#ifdef __cplusplus
extern "C" {
#endif

int read_a_line(int fd, char *buff, int max)
{
	int pos = 0;
	int bytes = 0;

	memset((void *)buff, 0, max);

	bytes = read(fd, (void *)buff, 1);
	while ((1 == bytes) && (pos < (max - 2)) && (buff[pos] != '\n')) {
		pos++;
		bytes = read(fd, (void *)&buff[pos], 1);
	}

	if (!pos) {
		return 0;
	}
	return pos + 1;
}

const char *delim = " \r\n";

int get_phys_mem(const char *filename, char *parm_name, uint64_t *sa,
		uint64_t *sz)
{
	int fd;
	char buff[MEM_CFG_MAX_LINE_LEN + 1];
	int buff_bytes;
	bool done = false;
	char *saveptr;

	*sa = RIO_MAP_ANY_ADDR;
	*sz = 0;
	errno = 0;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		return -1;
	}

	while (!done) {
		char *token;

		// Read until line termination character
		buff_bytes = read_a_line(fd, &buff[0], MEM_CFG_MAX_LINE_LEN);

		// No bytes returned means end of file.
		if (buff_bytes <= 0) {
			break;
		}

		if ((MEM_CFG_MAX_LINE_LEN - 1 == buff_bytes)
				&& (buff[MEM_CFG_MAX_LINE_LEN - 1] != '\n')) {
			errno = EFBIG;
			goto fail;
		}

		/* Find parm_name on the line */
		token = strtok_r(buff, delim, &saveptr);
		while ((NULL != token) && parm_idx(token, parm_name)) {
			token = strtok_r(NULL, delim, &saveptr);
		}

		if (NULL == token) {
			continue;
		}

		/* Next token must be address. */
		token = strtok_r(NULL, delim, &saveptr);
		if (NULL == token) {
			errno = EDOM;
			goto fail;
		}
		if (tok_parse_ull(token, sa, 0)) {
			errno = EDOM;
			goto fail;
		}

		token = strtok_r(NULL, delim, &saveptr);
		if (NULL == token) {
			*sa = RIO_MAP_ANY_ADDR;
			errno = EDOM;
			goto fail;
		}
		if (tok_parse_ull(token, sz, 0)) {
			*sa = RIO_MAP_ANY_ADDR;
			errno = EDOM;
			goto fail;
		}

		/* Address must be aligned to size, or somethings busted. */
		if ((*sz - 1) & *sa) {
			errno = EDOM;
			goto fail;
		}
		done = true;
	}

	close(fd);
	return done ? 0 : -1;

fail:
	close(fd);
	return -1;

}

int get_rsvd_phys_mem(char *parm_name, uint64_t *start_addr, uint64_t *size)
{
	return get_phys_mem((char *)MEM_CFG_DFLT_FN, parm_name, start_addr,
			size);
}

#ifdef __cplusplus
}
#endif

