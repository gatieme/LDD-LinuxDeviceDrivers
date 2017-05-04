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


ROOT=..
#PLATFORM=$(shell $(ROOT)/systype.sh)
#include $(ROOT)/Make.defines.$(PLATFORM)

MODULE_NAME := globalmem

ifneq ($(KERNELRELEASE),)

#CFG_FLAGS += -O2  -I./
#EXTRA_CFLAGS  += $(C_FLAGS) $(CFG_INC) $(CFG_INC)



obj-m := $(MODULE_NAME).o #globalmem.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

modules:
	make -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	make -C $(KERNELDIR) M=$(PWD) modules_install



insmod:
	sudo insmod $(MODULE_NAME).ko
	sudo mknod /dev/gatieme c 300 0
	sudo chmod 777 /dev/gatieme
reinsmod:
	sudo rmmod $(MODULE_NAME)
	sudo insmod $(MODULE_NAME).ko

github:
	cd $(ROOT) && make github

rmmod:
	sudo rmmod $(MODULE_NAME)
	sudo rm /dev/gatieme 

test :
	sudo ../injector/memInjector -l stack -m random -t word_0 --time 1 --timeout 3 -p 1

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	rm -f modules.order Module.symvers Module.markers

.PHNOY:
	modules modules_install clean



endif

