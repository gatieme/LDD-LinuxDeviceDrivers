ROOT=$(shell pwd)
PLATFORM=$(shell $(ROOT)/systype.sh)
include $(ROOT)/Make.defines.$(PLATFORM)



DIRS=injector engine tools


BIN=$(ROOT)/../workspace
ENGINE=$(ROOT)/engine
INJECTOR=$(ROOT)/injector
TOOLS=$(ROOT)/tools

all:
	for i in $(DIRS); do \
		(cd $$i && echo "making $$i" && $(MAKE) ) || exit 1; \
	done

clean:
	for i in $(DIRS); do \
		(cd $$i && echo "cleaning $$i" && $(MAKE) clean) || exit 1; \
	done

install :
#	for i in $(DIRS); do \
#		(cd $$i && echo "install $$i" && $(MAKE) install) || exit 1; \
#	done
	cp $(INJECTOR)/memInjector $(BIN)/memInjector
	cp $(ENGINE)/memoryEngine.ko $(BIN)/memoryEngine.ko
