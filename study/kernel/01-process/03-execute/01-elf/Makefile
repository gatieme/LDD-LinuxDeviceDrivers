ROOT=../../../
PLATFORM=$(shell $(ROOT)/systype.sh)
include $(ROOT)/Make.defines.$(PLATFORM)

target=testelf

MAIN_OBJS=testelf.o
SUBS_OBJS=add.o sub.o
DYNA_FILE=libtestelf.so
STAT_FILE=libtestelf.a

all:target

%.o : %.c
	$(CC) -c $^ -o $@  $(CFLAGS) $(DEFINES)

clean :
	rm *.o
	rm $(target)

# Complie the execute 
testelf:$(MAIN-OBJS) 
	gcc  $^ -o $(target) -ltestelf

# Complie the Dynamic Link Library libtestelf.so
libtestelf.so:
	gcc -shared $^ -o $@

# Complie the Static Link Library libtestelf.so
libtestelf.a:
	ar -rc libtestelf.a $(OBJS)