#
#http://blog.sina.com.cn/s/blog_694f2ae70101amh6.html
#
target=futex_align

CURRENT_BIN_DIR=futex_bug
INSTALL_ROOT_DIR=~/Work/Kernel/runninglinuxkernel/filesystem
INSTALL_HOME_DIR=home/chengjian

ifeq ($(ARCH), arm64)
	CROSS_COMPILE=/opt/arm/toolschain/linaro/gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
	ARCH_FILE_DIR=_install_arm64
else ifeq ($(ARCH), arm)
	CROSS_COMPILE=arm-linux-gnueabi-
	ARCH_FILE_DIR=_install_arm32
else	# ifeq ($(ARCH), x86_64)
	ARCH_FILE_DIR=_install_x86
endif

INSTALL_BIN_DIR=$(INSTALL_ROOT_DIR)/$(ARCH_FILE_DIR)/$(INSTALL_HOME_DIR)/ #$(CURRENT_BIN_DIR)

CC=${CROSS_COMPILE}gcc
AS=${CROSS_COMPILE}as
LD=${CROSS_COMPILE}ld
LDFLAGS+=-static


all : $(target)

futex_align:futex_align.o
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "ptrace singlestep..."


%.o : %.c
	$(CC) -c $^ -o $@  $(CFLAGS) $(DEFINES)

%.o	: %.cpp
	$(CXX) -c $^ -o $@  $(CFLAGS) $(DEFINES)

clean :
	rm -rf *.o
	rm -rf $(target)

install	:
	mkdir -p $(CURRENT_BIN_DIR)
	cp $(target) $(CURRENT_BIN_DIR)
	#echo $(INSTALL_BIN_DIR)
	cp -rf $(CURRENT_BIN_DIR) $(INSTALL_BIN_DIR) 
