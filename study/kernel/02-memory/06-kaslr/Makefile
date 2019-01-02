target=sp

all:$(target)

sp : sp.o

$(target) :
	$(CC) $^ -o $@ $(LDFLAGS)

%.o : %.c
	$(CC) -c $^ -o $@  $(CFLAGS) $(DEFINES)

clean :
	rm -rf *.o
	rm -rf $(target)
