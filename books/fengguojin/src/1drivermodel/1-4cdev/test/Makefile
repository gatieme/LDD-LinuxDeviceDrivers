ROOT=../../../

target=test



all:$(target)


memorylayout : test.o
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "测试使用register_chrdev注册的字符设备..."

%.o : %.c
	$(CC) -c $^ -o $@  $(CFLAGS) $(DEFINES)

clean :
	rm -rf *.o
	rm -rf $(target)


run :
	sudo ./test

.PHONY	: clean run
