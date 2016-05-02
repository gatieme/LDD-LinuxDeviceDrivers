# Makefile for the memory fault injection engine.
#
# Author: HIT CS HDMC team.
# Create: 2010-3-12 8:46
# Last modified: 2010-6-13 13:55:29
# Description:
# 	This program is loaded as a kernel(v2.6.18 or later) module.
# 	Use "make install" to load it into kernel.
# 	Use "make remove" to remove the module out of kernel.
#

ifneq ($(KERNELRELEASE),)

obj-m := memoryEngine.o

else

obj-m := memoryEngine.o

KERNELDIR=/lib/modules/$(shell uname -r)/build

PWD=$(shell pwd)

all:
	make -C $(KERNELDIR) M=$(PWD) modules
install:
	insmod memoryEngine.ko

remove:
	rmmod memoryEngine

clean:
	make -C /lib/modules/`uname -r`/build M=`pwd` clean
	rm -f modules.order Module.symvers Module.markers

endif
