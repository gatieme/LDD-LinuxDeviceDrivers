target=null_operation test1 test2 test_container_of

all:$(target)

null_operation : null_operation.o
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "NULL指针对象的合法和非法操作..."

test1 : test1.o
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "NULL指针结构体对象的合法和非法操作1..."

test2 : test2.o
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "NULL指针结构体对象的合法和非法操作2..."

test_container_of : test_container_of.o
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "container_of宏函数测试用例..."

%.o : %.c
	$(CC) -c $^ -o $@  $(CFLAGS) $(DEFINES)

%.o	: %.cpp
	$(CXX) -c $^ -o $@  $(CFLAGS) $(DEFINES)

clean :
	rm -rf *.o
	rm -rf $(target)
