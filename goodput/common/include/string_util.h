/*
 ****************************************************************************
 Copyright (c) 2016, Integrated Device Technology Inc.
 Copyright (c) 2016, RapidIO Trade Association
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

#ifndef __STRING_UTIL_H__
#define __STRING_UTIL_H__

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Copies at most n bytes of src, including the terminating null byte ('\0'),
 * to the buffer  pointed  to  by  dest. The dest buffer is guaranteed to be
 * null-terminated.
 *
 * If the length of src is less than n, SAFE_STRNCPY() writes additional null
 * bytes to dest to ensure that a total of n bytes are written.
 *
 * @param[out] dest the buffer to be written
 * @param[in] src the string to be copied
 * @param[in] n the maximum number of bytes to copy, must be less than or equal
 * to the size of dest
 * @return none
 */
#define SAFE_STRNCPY(dest,src,n) \
	strncpy(dest, src, n); \
	dest[n-1] = '\0';

// substitution of constants into strings in define statements
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifdef __cplusplus
}
#endif

#endif /* __STRING_UTIL_H__ */
