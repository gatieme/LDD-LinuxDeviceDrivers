ifneq ($(KERNELRELEASE),)

obj-m := kv2p.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean


endif



insmod:
	sudo insmod ./kv2p.ko

rmmod:
	sudo rmmod kv2p

reinsmod:
	make rmmod
	make insmod


