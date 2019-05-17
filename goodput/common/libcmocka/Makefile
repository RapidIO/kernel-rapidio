#----------------------------------------------------------------------------*
#Copyright (c) 2016, Integrated Device Technology Inc.
#Copyright (c) 2016, RapidIO Trade Association
#All rights reserved.
#
###Redistribution and use in source and binary forms, with or without modification,
#are permitted provided that the following conditions are met:
#
###1. Redistributions of source code must retain the above copyright notice, this
#list of conditions and the following disclaimer.
#
###2. Redistributions in binary form must reproduce the above copyright notice,
#this list of conditions and the following disclaimer in the documentation
#and/or other materials provided with the distribution.
#
###3. Neither the name of the copyright holder nor the names of its contributors
#may be used to endorse or promote products derived from this software without
#specific prior written permission.
#
###THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#--------------------------------------------------------------------------***

TOPDIR?=$(shell pwd)/../..
include $(TOPDIR)/rules.mk

NAME:=cmocka
TARGETS:=lib$(NAME).a

OBJECTS:=$(patsubst src/%.c,src/%.o,$(wildcard src/*.c))

# Keep clobbered as an acceptable warning, not our code...
CFLAGS+=-Wno-error=clobbered -Wno-error=maybe-uninitialized

# begin cmocka config.h
CFLAGS+=-DHAVE_ASSERT_H=1
CFLAGS+=-DHAVE_INTTYPES_H=1
CFLAGS+=-DHAVE_MALLOC_H=1
CFLAGS+=-DHAVE_MEMORY_H=1
CFLAGS+=-DHAVE_SETJMP_H=1
CFLAGS+=-DHAVE_SIGNAL_H=1
CFLAGS+=-DHAVE_STDARG_H=1
CFLAGS+=-DHAVE_STDDEF_H=1
CFLAGS+=-DHAVE_STDINT_H=1
CFLAGS+=-DHAVE_STDIO_H=1
CFLAGS+=-DHAVE_STDLIB_H=1
CFLAGS+=-DHAVE_STRINGS_H=1
CFLAGS+=-DHAVE_STRING_H=1
CFLAGS+=-DHAVE_SYS_STAT_H=1
CFLAGS+=-DHAVE_SYS_TYPE_H=1
CFLAGS+=-DHAVE_TIME_H=1
CFLAGS+=-DHAVE_UNISTD_H=1

CFLAGS+=-DHAVE_STRUCT_TIMESPEC=1

CFLAGS+=-DHAVE_CALLOC=1
CFLAGS+=-DHAVE_EXIT=1
CFLAGS+=-DHAVE_FPRINTF=1
CFLAGS+=-DHAVE_SNPRINTF=1
CFLAGS+=-DHAVE_VSNPRINTF=1
CFLAGS+=-DHAVE_FREE=1
CFLAGS+=-DHAVE_LONGJMP=1
CFLAGS+=-DHAVE_SIGLONGJMP=1
CFLAGS+=-DHAVE_MALLOC=1
CFLAGS+=-DHAVE_MEMCPY=1
CFLAGS+=-DHAVE_MEMSET=1
CFLAGS+=-DHAVE_PRINTF=1
CFLAGS+=-DHAVE_SETJMP=1
CFLAGS+=-DHAVE_SIGNAL=1
CFLAGS+=-DHAVE_SNPRINTF=1
CFLAGS+=-DHAVE_STRCMP=1
CFLAGS+=-DHAVE_VSNPRINTF=1
CFLAGS+=-DHAVE_STRSIGNAL=1
CFLAGS+=-DHAVE_CLOCK_GETTIME=1

CFLAGS+=-DHAVE_GCC_THREAD_LOCAL_STORAGE
CFLAGS+=-DHAVE_CLOCK_GETTIME_REALTIME

CFLAGS+=-DWORDS_SIZEOF_VOID_P=8
CFLAGS+=-w

# end cmocka config.h


.PHONY: all clean

all: $(TARGETS)

%.a: $(OBJECTS)
	@echo ---------- Building $@
	$(AR) rcs $@ $^

src/%.o: src/%.c
	@echo ---------- Building $@
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	@echo ---------- Cleaning lib$(NAME)...
	rm -f $(TARGETS) $(OBJECTS) \
	inc/*~ src/*~ *~
