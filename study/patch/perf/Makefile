target=bug_fork

all:$(target)

bug_fork:bug_fork.c
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread -DMUTEX -static
	@echo "mutex lock..."

%.o : %.c
	$(CC) -c $^ -o $@  $(CFLAGS) $(DEFINES)

%.o	: %.cpp
	$(CXX) -c $^ -o $@  $(CFLAGS) $(DEFINES)

clean :
	rm -rf *.o
	rm -rf $(target)
