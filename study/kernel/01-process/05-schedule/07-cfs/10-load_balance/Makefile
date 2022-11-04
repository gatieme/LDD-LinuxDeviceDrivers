ROOT=../../../
PLATFORM=$(shell $(ROOT)/systype.sh)
include $(ROOT)/Make.defines.$(PLATFORM)

target=memorylayout



all:$(target)


memorylayout : memorylayout.o
	$(CC) $^ -o $@ $(LDFLAGS)
	echo "C程序的内存布局..."

%.o : %.c
	$(CC) -c $^ -o $@  $(CFLAGS) $(DEFINES)

clean :
	rm -rf *.o
	rm -rf $(target)
