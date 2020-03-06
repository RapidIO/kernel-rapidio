CC ?= $(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
KDIR?=/usr/src/linux
LINUX_INCLUDE_DIR ?= $(KDIR)/include
LOG_LEVEL ?= 4
PREFIX?=/usr
LIBDIR=$(DESTDIR)$(PREFIX)/lib
BINDIR=$(DESTDIR)$(PREFIX)/bin
INCDIR=./include
O?=.
debug=1

ifdef debug
CFLAGS=-Wall -Wsign-compare -fPIC -DDEBUG -g -march=haswell -O3
else
CFLAGS=-O2 -Wall -Wsign-compare -fPIC -DNDEBUG
endif

LIBS=rt	pthread
# LDFLAGS?=-static
INCLUDES=$(INCDIR) $(LINUX_INCLUDE_DIR)
LIBPATHS=$(LIBDIR) $(O)

SAMPLES_TARGETS=$(addprefix $(O)/, rio_test_dma rio_test_misc \
		rio_test_client rio_test_server rio_test_buf \
		rio_test_db rio_test_pw rio_test_obw rio_test_devs)
LIB_TARGETS=$(addprefix $(O)/, libmport.a libmport.so)

.PHONY: all clean libs samples

libs: $(LIB_TARGETS)

all: $(LIB_TARGETS) $(SAMPLES_TARGETS)

samples: $(SAMPLES_TARGETS)

libmport.so: src/rio_mport_lib.o
	$(CC) -shared -o $@ $^

libmport.a: src/rio_mport_lib.o
	$(AR) -r $@ $^

$(O)/%: $(O)/samples/%.o libmport.so libmport.a
	$(CC) $(LDFLAGS) -o $@ $(addprefix -L, $(LIBPATHS)) $< $(addprefix -l, $(LIBS)) libmport.a

$(O)/%: $(O)/src/%.o
	$(CC) $(LDFLAGS) -o $@ $(addprefix -L, $(LIBPATHS)) $< $(addprefix -l, $(LIBS))

$(O)/%.o: %.c
	$(CC) -c -o $@ $(addprefix -I, $(INCLUDES)) $(CFLAGS) $<

clean:
	rm -rf $(SAMPLES_TARGETS) $(LIB_TARGETS) $(O)/src/*.o $(O)/samples/*.o
