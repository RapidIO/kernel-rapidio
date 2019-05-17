# Copyright (c) 2014, Prodrive Technologies
# Copyright (c) 2014, Integrated Device Technology
#
# SPDX-License-Identifier: BSD-3-Clause

TOPDIR?=$(shell pwd)/../..
include $(TOPDIR)/rules.mk

NAME:=rio
TARGETS:=lib$(NAME).a
TEST_TARGET:=$(NAME)_test

OBJECTS:=$(patsubst src/%.c,src/%.o,$(wildcard src/*.c))
TEST_OBJECTS:=$(patsubst test/%.c,test/%.o,$(wildcard test/*.c))
TEST_TARGETS:=$(patsubst test/%.c,test/%,$(wildcard test/*.c))

HEADERS:=inc
LOG_LEVEL?=4
DEBUG?=DEBUG
SRC_CFG_LIBRIO?=-DTSI57X_DAR_WANTED -DCPS_DAR_WANTED -DRXS_DAR_WANTED -DTSI721_DAR_WANTED -DTSI721_EXCLUDE_EM_INTERRUPT_SUPPORT

CFLAGS+=$(addprefix -W,all extra) -fPIC
CFLAGS+=-DNDEBUG
CFLAGS+=$(SRC_CFG_LIBRIO)
CFLAGS+=-I./inc
CFLAGS+=-I./src
CFLAGS+=-I../libct/inc
CFLAGS+=-I../libdid/inc
CFLAGS+=-w

LDFLAGS_STATIC+=-L. -L$(COMMONLIB) -L$(MPORTLIB) -l$(NAME)
LDFLAGS_STATIC+=$(TST_LIBS) -lcli -lmport
LDFLAGS_DYNAMIC+=-lpthread


.PHONY: all clean

ifdef TEST
all: $(TARGETS) $(TEST_TARGET)
else
all: $(TARGETS)
endif

runtests: $(TEST_TARGETS)
	@$(foreach f,$^, \
		echo ------------ Running $(f); \
		$(UNIT_TEST_FAIL_POLICY) \
		./$(f); \
		echo; \
	)

%.a: $(OBJECTS)
	@echo ---------- Building $@
	$(AR) rcs $@ $^

src/%.o: src/%.c
	@echo ---------- Building $@
	$(CXX) -c $(CFLAGS) $< -o $@ \
	$(TST_INCS)

test/%.o: test/%.c
	@echo ---------- Building $@
	$(CXX) -c $(CFLAGS) $< -o $@ \
	$(TST_INCS)

test/%: test/%.o
	@echo ---------- Building $@
	$(CXX) -o $@ $< \
	$(LDFLAGS_STATIC) \
	$(LDFLAGS_DYNAMIC)

$(TEST_TARGET): $(TEST_TARGETS)

clean:
	@echo ---------- Cleaning lib$(NAME)...
	rm -f $(TARGETS) $(OBJECTS) \
	$(TEST_TARGETS) $(TEST_OBJECTS) \
	inc/*~ src/*~ test/*~ *~

