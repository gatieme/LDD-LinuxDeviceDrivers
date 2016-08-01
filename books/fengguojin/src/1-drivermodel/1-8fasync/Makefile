ROOT=../../../../..

DIRS=	simple test

COMMIT="[例1.8  异步通知实例](http://book.51cto.com/art/201205/337673.htm)"

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
