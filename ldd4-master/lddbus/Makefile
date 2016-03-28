# Comment/uncomment the following line to disable/enable debugging
#DEBUG = y

# Add your debugging flag (or not) to ccflags.
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

ccflags-y += $(DEBFLAGS)
ccflags-y += -I$(LDDINCDIR)

ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m	:= lddbus.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) LDDINCDIR=$(PWD)/../include modules

endif



clean:
	rm -rf *.o *.ko *~ core .depend *.mod.c .*.cmd .tmp_versions .*.o.d

depend .depend dep:
	$(CC) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include .depend
endif
