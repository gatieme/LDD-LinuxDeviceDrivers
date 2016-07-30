ROOT=.


DIRS=	demo test

all:
	for i in $(DIRS); do \
		(cd $$i && echo "making $$i" && $(MAKE) ) || exit 1; \
	done


clean:
	for i in $(DIRS); do \
		(cd $$i && echo "cleaning $$i" && $(MAKE) clean) || exit 1; \
	done


insmod:
	cd demo && make insmod

rmmod:
	cd demo && make rmmod

reinsmod:
	cd demo && make reinsmod

run	:
	sudo ./test/test


