ROOT=../../../../..

DIRS=	simple test

all:
	for i in $(DIRS); do \
		(cd $$i && echo "making $$i" && $(MAKE) ) || exit 1; \
	done


clean:
	for i in $(DIRS); do \
		(cd $$i && echo "cleaning $$i" && $(MAKE) clean) || exit 1; \
	done


insmod:
	cd simple && make insmod

rmmod:
	cd simple && make rmmod

reinsmod:
	cd simple && make reinsmod

run		:
	sudo ./test/test

github	:
	cd $(ROOT)	&& make github
