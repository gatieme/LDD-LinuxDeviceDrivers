#direntorys
HELLO_DIR=hello
CPLD_DIR=cpld
FPGA_DIR=fpga
TEST_MODULE_DIR=test_module
DIRS=$(HELLO_DIR) $(CPLD_DIR) $(FPGA_DIR) $(TEST_MODULE_DIR)

SUBDIRS=$(DIRS)

.PHONY:all $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
		$(MAKE) -C $@
		@echo "=======BUILD $@ SUCCESS======="

#all	:
#	for i in $(DIRS); do \
#		(cd $$i && echo "making $$i" && $(MAKE) ) || exit 1; \
#	done

clean:
	for i in $(DIRS); do \
		(cd $$i && echo "cleaning $$i" && $(MAKE) clean) || exit 1; \
	done

install :
	for i in $(DIRS); do \
		(cd $$i && echo "install $$i" && $(MAKE) install) || exit 1; \
	done
