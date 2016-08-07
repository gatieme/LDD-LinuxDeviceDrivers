ifneq ($(KERNELRELEASE),)

obj-m := print_thread_info.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

insmod:
	sudo insmod ./print_thread_info.ko

rmmod:
	sudo rmmod print_thread_info

reinsmod:
	make rmmod
	make insmod


endif
