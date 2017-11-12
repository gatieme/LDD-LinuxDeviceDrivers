target=mutex_bug sem_bug bug \
       sem_test check_lock 	\
	futex_test futex_bug


all:$(target)

mutex_bug:lock_bug.c
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread -DMUTEX
	@echo "mutex lock..."

sem_bug:lock_bug.c
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread -DSEM
	@echo "mutex lock..."

bug:lock_bug.c
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "none lock..."

sem_test:sem_test.c
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "bug use mutex..."


check_lock:check_lock.c
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "check lock..."


futex_bug:futex_bug.c
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "bug use futex..."


futex_test:futex_test.c
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "futex workthread sendthread..."


bind:bind.c
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "set cpu affine..."
#%.o : %.c
#	$(CC) -c $^ -o $@  $(CFLAGS) $(DEFINES)
#
#%.o	: %.cpp
#	$(CXX) -c $^ -o $@  $(CFLAGS) $(DEFINES)

clean :
	rm -rf *.o
	rm -rf $(target)
