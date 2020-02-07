#----------------------------------------------------------------------------*
#Copyright (c) 2016, Integrated Device Technology Inc.
#Copyright (c) 2016, RapidIO Trade Association
#All rights reserved.
#
##Redistribution and use in source and binary forms, with or without modification,
#are permitted provided that the following conditions are met:
#
##1. Redistributions of source code must retain the above copyright notice, this
#list of conditions and the following disclaimer.
#
##2. Redistributions in binary form must reproduce the above copyright notice,
#this list of conditions and the following disclaimer in the documentation
#and/or other materials provided with the distribution.
#
##3. Neither the name of the copyright holder nor the names of its contributors
#may be used to endorse or promote products derived from this software without
#specific prior written permisson.
#
##THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#----------------------------------------------------------------------------*

ifeq (,$(TOPDIR))
$(error ************  ERROR: Please define TOPDIR in your Makefile ************)
endif

ifdef TEST
MAKE_CTRL_FILE=$(TOPDIR)/.makeTest
MAKE_OLD_CTRL_FILE=$(TOPDIR)/.makeAll
else
MAKE_CTRL_FILE=$(TOPDIR)/.makeAll
MAKE_OLDCTRL_FILE=$(TOPDIR)/.makeTest
endif

# If clean is not a specified target, then check what type of build was last performed.
# A clean must be performed between a test and non test build as cmocka compiles differently
# depending on the inclusion/exclusion of test define constants eventually leading to link errors
ifneq (,$(filter clean,$(MAKECMDGOALS)))
$(shell rm -rf $(TOPDIR)/.makeTest)
$(shell rm -rf $(TOPDIR)/.makeAll)
else
# Look for compile control files
ifeq ($(shell test -f $(TOPDIR)/.makeAll && echo -n yes),yes)
LAST_BUILD_TRGT=ALL
endif
ifeq ($(shell test -f $(TOPDIR)/.makeTest && echo -n yes),yes)
LAST_BUILD_TRGT=TEST
endif
# If built before, verify building test or not
ifdef LAST_BUILD_TRGT
ifdef TEST
ifeq (ALL,$(LAST_BUILD_TRGT))
$(error ************  ERROR: Must clean between regular and test builds ************)
endif
else
ifeq (TEST,$(LAST_BUILD_TRGT))
$(error ************  ERROR: Must clean between regular and test builds ************)
endif
endif
endif
$(shell touch $(MAKE_CTRL_FILE))
endif

CC    =$(CROSS_COMPILE)gcc
CXX   =$(CROSS_COMPILE)g++
AR    =$(CROSS_COMPILE)ar

export CC
export CXX

KDIR=$(TOPDIR)/libmport/include
RIODIR=$(TOPDIR)/libmport/include

#ARCH := $(shell arch)
ARCH = arm64

OPTFLAGS = -ggdb -Ofast -fno-strict-overflow

ifeq ($(ARCH), x86_64)
 OPTFLAGS += -march=native -mfpmath=sse -ffast-math
endif
ifeq ($(ARCH), i686)
 OPTFLAGS += -march=native -mfpmath=sse -ffast-math
endif

## by default, debug logging is enabled
LOG_LEVEL?=7

## by default, debug is disabled 
## [export DEBUG_CTL="DEBUG=3" | export DEBUG_CTL=DEBUG]
DEBUG_CTL?=NDEBUG

ifneq ($(DEBUG_CTL), NDEBUG)
 OPTFLAGS = -ggdb -Og
endif

HERE := $(shell pwd)

$(info Building $(HERE) on $(ARCH) with optimisations $(OPTFLAGS))

COMMONDIR=$(TOPDIR)/common
COMMONINC=$(COMMONDIR)/include
COMMONLIB=$(COMMONDIR)/libs_a

MPORTDIR=$(TOPDIR)/../libmport
MPORTINC=$(MPORTDIR)/include
MPORTLIB=$(MPORTDIR)

KERNELDIR=$(TOPDIR)/..
KERNELINC=$(KERNELDIR)/include

STD_FLAGS=$(OPTFLAGS) -pthread -Wall -Wextra -Werror -fPIC
STD_FLAGS+=-I$(TOPDIR)/include -I$(COMMONINC) -I. -I./inc
STD_FLAGS+=-I$(MPORTINC) -I$(KERNELINC)
STD_FLAGS+=-L$(COMMONLIB) -L$(MPORTLIB)
ifdef TEST
STD_FLAGS+=-DUNIT_TESTING
endif

CFLAGS+=$(STD_FLAGS)
CXXFLAGS+=$(STD_FLAGS) -std=c++11

LDFLAGS_STATIC?=-Wl,-Bstatic
LDFLAGS_DYNAMIC?=-Wl,-Bdynamic

ifdef TEST
TST_LIBS=-lcmocka
TST_INCS=-I$(COMMONDIR)/libcmocka/inc
endif

UNIT_TEST_FAIL_POLICY?=set -e;
