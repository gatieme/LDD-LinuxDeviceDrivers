ifneq ($(KERNELRELEASE),)

obj-m := mmap_test.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build


PWD := $(shell pwd)

all:
	make -C $(KERNELDIR) M=$(PWD) modules
	gcc test.c -o test

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	rm ./test


endif
