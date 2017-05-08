ifeq ($(KERNELRELEASE),)

    KERNELDIR ?= /linux-2.6.29.4/linux-2.6.29.4
    PWD := $(shell pwd)
    PRINT_INC =$(PWD)/../include
    EXTRA_CFLAGS += -I $(PRINT_INC)

modules:
	$(MAKE) -I $(PRINT_INC) -C $(KERNELDIR) M=$(PWD) modules
	echo $(PRINT_INC)

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

.PHONY: modules modules_install clean

else
    # called from kernel build system: just declare what our modules are
    obj-m := add_sub.o 
endif



