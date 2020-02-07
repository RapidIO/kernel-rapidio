#
# Makefile for RIO driver 
#
#
RAPIDIO_ENABLE_RX_TX_PORTS 	:= TRUE
RAPIDIO_DMA_ENGINE		:= TRUE
RAPIDIO_DEBUG  			:= TRUE
RAPIDIO_ENUM_BASIC		:= TRUE
RAPIDIO_MPORT_CDEV		:= TRUE
TSI721_PCIE_GEN3_WORKAROUND	:= FALSE

#KERNEL_VERSION := $(shell uname -r)
#KERNELDIR = /lib/modules/$(KERNEL_VERSION)/build
KERNEL_VERSION := 4.4.52
KERNELDIR = /home/cwarring/kernel_build/4.4.52/kdriver

INSTDIR = /lib/modules/$(KERNEL_VERSION)/kernel/drivers/rapidio
KERNEL := kernel-$(KERNEL_VERSION)
PWD := $(shell pwd)

ifeq ($(RAPIDIO_DEBUG),TRUE)
EXTRA_CFLAGS += -DCONFIG_RAPIDIO_DEBUG -DDEBUG
endif

ifeq ($(RAPIDIO_ENABLE_RX_TX_PORTS),TRUE)
EXTRA_CFLAGS += -DCONFIG_RAPIDIO_ENABLE_RX_TX_PORTS
endif

ifeq ($(RAPIDIO_DMA_ENGINE),TRUE)
EXTRA_CFLAGS += -DCONFIG_RAPIDIO_DMA_ENGINE
endif

ifeq ($(TSI721_PCIE_GEN3_WORKAROUND),TRUE)
EXTRA_CFLAGS += -DCONFIG_TSI721_PCIE_GEN3_WORKAROUND
endif

obj-m := rapidio.o
rapidio-objs :=  rio.o rio-access.o	\
		rio-driver.o rio-sysfs.o
obj-m += rio-scan.o
obj-m += rio_cm.o
obj-m += switches/idtcps.o
obj-m += switches/idt_gen2.o
obj-m += switches/idt_gen3.o
obj-m += switches/tsi568.o
obj-m += switches/tsi57x.o
obj-m += tsi721_mport.o
obj-m += rionet.o

tsi721_mport-objs := devices/tsi721.o devices/tsi721_dma.o
obj-m += devices/rio_mport_cdev.o

all: prepare
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	cp *.ko $(KERNEL)/
	cp switches/*.ko $(KERNEL)/
	cp devices/*.ko $(KERNEL)/

prepare:
	rm -rf $(KERNEL)
	mkdir $(KERNEL)

instcom:
	find /lib/modules/$(KERNEL_VERSION) -name rapidio.ko -exec rm -f {} \; || true
	find /lib/modules/$(KERNEL_VERSION) -name idtcps.ko -exec rm -f {} \; || true
	find /lib/modules/$(KERNEL_VERSION) -name idt_gen2.ko -exec rm -f {} \; || true
	find /lib/modules/$(KERNEL_VERSION) -name idt_gen3.ko -exec rm -f {} \; || true
	find /lib/modules/$(KERNEL_VERSION) -name tsi568.ko -exec rm -f {} \; || true
	find /lib/modules/$(KERNEL_VERSION) -name tsi57x.ko -exec rm -f {} \; || true
	find /lib/modules/$(KERNEL_VERSION) -name tsi721_mport.ko -exec rm -f {} \; || true
	find /lib/modules/$(KERNEL_VERSION) -name rio_mport_cdev.ko -exec rm -f {} \; || true
	find /lib/modules/$(KERNEL_VERSION) -name rio-scan.ko -exec rm -f {} \; || true
	find /lib/modules/$(KERNEL_VERSION) -name rio_cm.ko -exec rm -f {} \; || true
	find /lib/modules/$(KERNEL_VERSION) -name rionet.ko -exec rm -f {} \; || true

	install -D -m 644 $(KERNEL)/rapidio.ko $(INSTDIR)/rapidio.ko
	install -D -m 644 $(KERNEL)/idtcps.ko $(INSTDIR)/idtcps.ko
	install -D -m 644 $(KERNEL)/idt_gen2.ko $(INSTDIR)/idt_gen2.ko
	install -D -m 644 $(KERNEL)/idt_gen3.ko $(INSTDIR)/idt_gen3.ko
	install -D -m 644 $(KERNEL)/tsi568.ko $(INSTDIR)/tsi568.ko
	install -D -m 644 $(KERNEL)/tsi57x.ko $(INSTDIR)/tsi57x.ko
	install -D -m 644 $(KERNEL)/tsi721_mport.ko $(INSTDIR)/tsi721_mport.ko
	install -D -m 644 $(KERNEL)/rio_mport_cdev.ko $(INSTDIR)/rio_mport_cdev.ko
	install -D -m 644 $(KERNEL)/rio-scan.ko $(INSTDIR)/rio-scan.ko
	install -D -m 644 $(KERNEL)/rio_cm.ko $(INSTDIR)/rio_cm.ko
#	install -D -m 644 $(KERNEL)/rionet.ko $(INSTDIR)/rionet.ko

	/sbin/depmod -a $(KERNEL_VERSION) || true

install: instcom
	cp -n config/rapidio.conf /etc/modprobe.d/
	cp -n config/70-rapidio-access.rules /etc/udev/rules.d/

# Copy cdev drivers header files into common location for all users

# FIXME. At this moment we copy cdev driver interface includes into two
# directories: one common for all kernel versions and other one that contains
# headers for running kernel version. We will decide which method to use after
# testing on different Linux distributions.

	mkdir -p /usr/src/rapidio/linux
	cp -f include/rio_mport_cdev.h /usr/src/rapidio/linux
	cp -f include/rio_cm_cdev.h /usr/src/rapidio/linux

	cp -f include/ri*.h $(KERNELDIR)/include/linux
	
uninstall:
	if [ -e $(INSTDIR)/rapidio.ko ] ; then \
	    rm -f $(INSTDIR)/*.ko ; \
	fi
	/sbin/depmod -a
	rm -rf /etc/modprobe.d/rapidio*
	
clean:
	rm -f *.o *.ko .*.cmd *.mod.*  *.unsigned *.order *.symvers .*.cmd.* .*.ko.* .*.mod.o.* .*.o.*
	rm -f -r .tmp_versions
	rm -rf $(KERNEL)
	rm -rf switches/*.o switches/*.ko switches/*.cmd switches/*.mod.* switches/*.unsigned switches/*.order switches/*.symvers switches/.*.cmd.* switches/.*.ko.* switches/.*.mod.o.* switches/.*.o.*
	rm -rf devices/*.o devices/*.ko devices/*.cmd devices/*.mod.* devices/*.unsigned devices/*.order devices/*.symvers  devices/.*.cmd.* devices/.*.ko.* devices/.*.mod.o.* devices/.*.o.*
