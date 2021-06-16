target=fork COW



all:$(target)

fork : fork.o
	$(CC) $^ -o $@ $(LDFLAGS)

COW : COW.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o : %.c
	$(CC) -g -c $^ -o $@  $(CFLAGS) $(DEFINES)

clean :
	rm -rf *.o
	rm -rf $(target)
