ifeq ($(KERNELRELEASE),)

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

else

CFG_FLAGS += -O2  -I./
EXTRA_CFLAGS  += $(C_FLAGS) $(CFG_INC) $(CFG_INC)  


# 注意：驱动模块的名字 千万 不能和本文件夹内的任何文件同名！  
MODULE_NAME := find-task    					#指定驱动模块的名字  
RESMAIN_CORE_OBJS := find_task.o				#指定驱动模块的核心文件（有init 和 exit）  
RESMAIN_GLUE_OBJS := get_task.o		#依赖文件  


#xxx-objs := 指定驱动模块的所有依赖文件  
$(MODULE_NAME)-objs := $(RESMAIN_GLUE_OBJS) $(RESMAIN_CORE_OBJS)   

#最终由xxx-objs链接生成find-task.o，再生成find-task.ko  
obj-m := find-task.o   #最终由xxx-objs链接生成find-task.o，再生成find-task.ko  


endif  
  
