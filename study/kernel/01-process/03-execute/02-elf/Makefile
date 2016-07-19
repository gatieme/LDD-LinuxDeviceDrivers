ifeq ($(DEBUG), y)

	DEFFLAGS=-DDEBUG -DLINUX
	CFLAGS= -g3 -Wall
else

	DEFFLAGS=-DRELEASE -DLINUX

endif


target=testelf_normal testelf_dynamic testelf_static

MAIN_OBJS=testelf.o
SUBS_OBJS=add.o sub.o

DYNA_FILE=libtestelf.so
STAT_FILE=libtestelf.a

all:$(target)

%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean :
	rm -rf $(MAIN_OBJS) $(SUBS_OBJS)
	rm -rf $(DYNA_FILE) $(STAT_FILE)
	rm -rf $(target)


# Complie the execute
testelf_normal:$(MAIN_OBJS) $(SUBS_OBJS)
	gcc $^ -o $@

testelf_dynamic:$(MAIN_OBJS) $(DYNA_FILE)
	gcc  $^ -o $@ -L./ -ltestelf

testelf_static:$(MAIN_OBJS) $(STAT_FILE)
	gcc  testelf.o -o $@ -static -L./ -ltestelf



# Complie the Dynamic Link Library libtestelf.so
libtestelf.so:$(SUBS_OBJS)
	gcc -shared $^ -o $@

# Complie the Static Link Library libtestelf.so
STAT_FILE:$(STAT_FILE)(add.o) $(STAT_FILE)(sub.o)
libtestelf.a:$(SUBS_OBJS)
	ar -r $@ $^

#=======
#ERROR
#=======
#
#/usr/bin/ld: cannot find -lc
#collect2: error: ld returned 1 exit status
#	make: *** [testelf_static] Error 1
#
#yum install glibc-static
