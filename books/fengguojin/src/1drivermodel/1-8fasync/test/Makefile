ROOT=../../../

target=test

LDFLAGS+= -lpthread

all:$(target)


test : test.o
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "例1.7  poll接口驱动应用程序示例..."

%.o : %.c
	$(CC) -c $^ -o $@  $(CFLAGS) $(DEFINES)

clean :
	rm -rf *.o
	rm -rf $(target)


run :
	sudo ./test

.PHONY	: clean run
