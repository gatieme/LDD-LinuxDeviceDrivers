ifneq ($(KERNELRELEASE),)

#CFG_FLAGS += -O2  -I./
#EXTRA_CFLAGS  += $(C_FLAGS) $(CFG_INC) $(CFG_INC)  


MODULE_NAME := find-task 
RESMAIN_CORE_OBJS := find_task.o 
RESMAIN_GLUE_OBJS := get_task.o 


find-task-objs := $(RESMAIN_GLUE_OBJS) $(RESMAIN_CORE_OBJS)   
obj-m += find-task.o

#$(MODULE_NAME)-objs := $(RESMAIN_GLUE_OBJS) $(RESMAIN_CORE_OBJS)   
#obj-m := $(MODULE_NAME).o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

modules:
	make -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	make -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

.PHNOY:
	modules modules_install clean



endif  
  
