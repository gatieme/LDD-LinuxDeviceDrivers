CFLAGS = -std=gnu99
target=not_power_of_2 zero null

all:$(target)

not_power_of_2: build_bug_on_not_power_of_2.o
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "BUILD_BUG_ON_NOT_POWER_OF2..."

zero : build_bug_on_zero.o
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "BUILD_BUG_ON_ZERO..."


null : build_bug_on_null.o
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "BUILD_BUG_ON_NULL..."

invalid : build_bug_on_invalid.o
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "BUILD_BUG_ON_INVALID..."

%.o : %.c
	$(CC) -c $^ -o $@  $(CFLAGS) $(DEFINES)

%.o	: %.cpp
	$(CXX) -c $^ -o $@  $(CFLAGS) $(DEFINES)

clean :
	rm -rf *.o
	rm -rf $(target)
