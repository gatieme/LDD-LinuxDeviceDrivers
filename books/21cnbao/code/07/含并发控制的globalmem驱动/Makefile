# ------------------------------------------------------------------------------
#
# Makefile for the LDD-LinuxDeviceDrivers.
#
# Author: gatieme
# Create: 2016-07-29 15:50:46
# Last modified: 2016-07-29 16:10:29
# Description:
# 	This program is loaded as a kernel(v2.6.18 or later) module.
# 	Use "make install" to load it into kernel.
# 	Use "make remove" to remove the module out of kernel.
#
# ------------------------------------------------------------------------------


#	my driver description
DRIVER_VERSION := "1.0.0"
DRIVER_AUTHOR  := "Gatieme @ AderStep Inc..."
DRIVER_DESC    := "Linux input module for Elo MultiTouch(MT) devices"
DRIVER_LICENSE := "Dual BSD/GPL"


MODULE_NAME := globalmem_lock



ifneq ($(KERNELRELEASE),) 	# kernelspace

obj-m := $(MODULE_NAME).o

else						# userspace


LINUX_KERNEL ?= $(shell uname -r)
LINUX_KERNEL_PATH ?= /lib/modules/$(LINUX_KERNEL)/build

CURRENT_PATH ?= $(shell pwd)
CFG_INC = $(CURRENT_PATH)
MODCFLAGS:=-O2 -Wall -DMODULE -D__KERNEL__ -DLINUX -std=c99
EXTRA_CFLAGS  += $(MODULE_FLAGS) -I $(CFG_INC)


modules:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules

modules_install:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules_install

insmod:
	sudo insmod $(MODULE_NAME).ko globalmem_major=0
	sudo mknod /dev/gatieme c 242 0
	sudo chmod 666 /dev/gatieme

rmmod:
	sudo rmmod $(MODULE_NAME)
	sudo rm -rf /dev/gatieme

github:
	cd $(ROOT) && make github

test :
	echo "gatieme" > /dev/gatieme
	cat /dev/gatieme

clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
	rm -f modules.order Module.symvers Module.markers

.PHNOY:
	modules modules_install clean



endif

