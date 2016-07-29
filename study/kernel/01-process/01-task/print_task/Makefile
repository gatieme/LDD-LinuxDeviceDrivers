ifneq ($(KERNELRELEASE),)

obj-m := print_task.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean


endif



insmod:
	sudo insmod ./print_task.ko

rmmod:
	sudo rmmod print_task

reinsmod:
	make rmmod
	make insmod


