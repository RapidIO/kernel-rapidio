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

TOPDIR?=$(shell pwd)
include $(TOPDIR)/rules.mk

NAME=goodput
TARGET:=$(NAME)
DEBUG?=DEBUG

INCLUDES= inc/worker.h inc/goodput.h inc/goodput_cli.h
OBJECTS:=$(patsubst src/%.c,src/%.o,$(wildcard src/*.c))

# Kernel-mode flags
CXXFLAGS+=-Wno-sign-compare
CXXFLAGS+=-D$(DEBUG_CTL) -DRDMA_LL=$(LOG_LEVEL)
CXXFLAGS+=-I$(COMMONDIR) -I$(MPORTINC) -I$(COMMONDIR)/librio/inc
CXXFLAGS+=-pthread

LDFLAGS_STATIC+=-L$(COMMONLIB) -L$(MPORTLIB)
LDFLAGS_STATIC+=-lmport -lrsvdmem -ltime_utils -llog -lcli -lrio -llist
LDFLAGS_STATIC+=-lcmocka
LDFLAGS_DYNAMIC+=-lpthread -ldl -lrt


.PHONY: all clean logs common scripts

all: $(TARGET) common libmport

runtests: FORCE
	$(MAKE) runtests -C common

libmport: FORCE
	$(MAKE) all -C $(MPORTDIR)

common: FORCE libmport
	$(MAKE) all -C common

scripts:
	@ln -s scripts mport0 &>/dev/null || true

src/%.o: src/%.c
	@echo ---------- Building $@
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(TARGET): $(OBJECTS) common libmport
	$(CXX) -o $@ $(OBJECTS) \
	$(LDFLAGS_STATIC) \
	$(LDFLAGS_DYNAMIC)

clean:
	@echo ---------- Cleaning $(NAME)...
	@rm -f $(TARGET) $(OBJECTS) \
	src/*~ inc/*~ ~*
	$(MAKE) clean -C common
	$(MAKE) clean -C $(MPORTDIR)

FORCE:

logs: FORCE
	rm -f logs/*.log logs/*.res

