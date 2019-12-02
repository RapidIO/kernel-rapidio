# #All rights reserved.
# #
# ##Redistribution and use in source and binary forms, with or without modification,
# #are permitted provided that the following conditions are met:
# #
# ##1. Redistributions of source code must retain the above copyright notice, this
# #list of conditions and the following disclaimer.
# #
# ##2. Redistributions in binary form must reproduce the above copyright notice,
# #this list of conditions and the following disclaimer in the documentation
# #and/or other materials provided with the distribution.
# #
# ##3. Neither the name of the copyright holder nor the names of its contributors
# #may be used to endorse or promote products derived from this software without
# #specific prior written permission.
# #
# ##THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# #ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# #WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# #DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# #FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# #DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# #SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# #CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# #OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# #OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# #--------------------------------------------------------------------------***
# Read values from ones exported in master Makefile. Or, override on the command
# line if building separately.

TOPDIR?=$(shell pwd)/../../..
include $(TOPDIR)/rules.mk


CFLAGS+=-D$(DEBUG_CTL) -DRDMA_LL=$(LOG_LEVEL) -Lpthread 
CFLAGS+=-I../../../inc -lpthread
CFLAGS+=-lumd -l:libmport.a -L../../libs_a -L../../../../libmport -I../../../../libmport/include -I../../../inc -O0

.PHONY: all clean test

TARGETS:=basic

all: $(TARGETS)

basic: basic.c
	$(CC) -g basic.c -o basic $(CFLAGS)
#gcc basic.c -o basic -lumd -l:libmport.a -L.. -L../../../../libmport -I../../../../libmport/include -I../../../inc

clean:
	@echo ---------- Cleaning Tsi721_UMD tests...
	rm -f $(TARGETS)
