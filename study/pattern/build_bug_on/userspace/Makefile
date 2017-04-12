CFLAGS = -std=gnu99
target=not_power_of_2 zero

all:$(target)

not_power_of_2: build_bug_on_not_power_of_2.o
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "BUILD_BUG_ON_NOT_POWER_OF2..."

zero : build_bug_on_zero.o
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "BUILD_BUG_ON_ZERO..."


SCClient : SCClient.o
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "简单的模拟高并发的客户端..."

SCMClient : SCMClient.o
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "带线程锁的模拟高并发的客户端..."

%.o : %.c
	-$(CC) -c $^ -o $@  $(CFLAGS) $(DEFINES)

%.o	: %.cpp
	-$(CXX) -c $^ -o $@  $(CFLAGS) $(DEFINES)

clean :
	rm -rf *.o
	rm -rf $(target)
