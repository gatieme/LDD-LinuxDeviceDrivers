#
#http://blog.sina.com.cn/s/blog_694f2ae70101amh6.html
#
target=ptrace_singlestep ptrace_singlestep2	\
       ptrace_peekuser a

#CROSS_COMPILE=/opt/arm/toolschain/linaro/gcc-linaro-7.1.1-2017.08-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
CC=${CROSS_COMPILE}gcc
AS=${CROSS_COMPILE}as
LD=${CROSS_COMPILE}ld


LDFLAGS+=-static
all : $(target)

ptrace_singlestep:ptrace_singlestep.c
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "ptrace singlestep..."

ptrace_singlestep2:ptrace_singlestep.c
	$(CC) $^ -o $@ $(LDFLAGS) -DDEBUG
	@echo "ptrace singlestep with debug info..."

ptrace_peekuser:ptrace_peekuser.c
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "ptrace peekuser..."

a:a.c
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "ptrace process..."


#%.o : %.c
#	$(CC) -c $^ -o $@  $(CFLAGS) $(DEFINES)
#
#%.o	: %.cpp
#	$(CXX) -c $^ -o $@  $(CFLAGS) $(DEFINES)

clean :
	rm -rf *.o
	rm -rf $(target)

install	:
	mkdir -p ptrace_bug
	cp $(target) ptrace_bug
#	cp -rf ptrace_bug
