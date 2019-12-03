#----------------------------------------------------------------------------*
#Copyright (c) 2015, Integrated Device Technology Inc.
#Copyright (c) 2015, RapidIO Trade Association
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without modification,
#are permitted provided that the following conditions are met:
#
#1. Redistributions of source code must retain the above copyright notice, this
#list of conditions and the following disclaimer.
#
#2. Redistributions in binary form must reproduce the above copyright notice,
#this list of conditions and the following disclaimer in the documentation
#and/or other materials provided with the distribution.
#
#3. Neither the name of the copyright holder nor the names of its contributors
#may be used to endorse or promote products derived from this software without
#specific prior written permission.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
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

TOPDIR?=$(shell pwd)/..
include $(TOPDIR)/rules.mk

all: libcmocka libcli liblist liblog libtime_utils \
	librsvdmem librio tsi721_umd

libcmocka: FORCE
	$(MAKE) -C libcmocka
	mkdir -p libs_a
	cp libcmocka/libcmocka.a libs_a/

libcli: FORCE libcmocka
	$(MAKE) all -C libcli
	mkdir -p libs_a
	cp libcli/libcli.a libs_a/
		
liblist: FORCE libcmocka 
	$(MAKE) all -C liblist
	mkdir -p libs_a
	cp liblist/liblist.a libs_a/
		
liblog: FORCE libcmocka 
	$(MAKE) all -C liblog
	mkdir -p libs_a
	cp liblog/liblog.a libs_a/
		
libtime_utils: FORCE libcmocka liblog libcli
	$(MAKE) all -C libtime_utils
	mkdir -p libs_a
	cp libtime_utils/libtime_utils.a libs_a/
		
librsvdmem: FORCE libcmocka libcli
	$(MAKE) all -C librsvdmem
	mkdir -p libs_a
	cp librsvdmem/librsvdmem.a libs_a/
		
librio: FORCE libcmocka
	$(MAKE) all -C librio
	mkdir -p libs_a
	cp librio/librio.a libs_a/

tsi721_umd: FORCE libcmocka
	$(MAKE) all -C tsi721_umd
	mkdir -p libs_a
	cp tsi721_umd/libumd.a libs_a/

FORCE:

runtests:
	$(MAKE) runtests -C libcli
	$(MAKE) runtests -C liblist
	$(MAKE) runtests -C librsvdmem
	$(MAKE) runtests -C libtime_utils
	$(MAKE) runtests -C librio

clean:
	$(MAKE) clean -C tsi721_umd
	$(MAKE) clean -C libcli
	$(MAKE) clean -C libcmocka
	$(MAKE) clean -C liblist
	$(MAKE) clean -C liblog
	$(MAKE) clean -C librsvdmem
	$(MAKE) clean -C libtime_utils
	$(MAKE) clean -C librio
	rm -f libs_a/*

