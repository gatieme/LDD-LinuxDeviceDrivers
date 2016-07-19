all:rbtree-tst

CFLAGS=-g -O0 -Wall

rbtree-tst:rbtree-tst.o rbtree.o

rbtree.o:rbtree.h rbtree.c

rbtree-tst.o:rbtree-tst.c

.PHONY:clean

clean:
	rm *.o rbtree-tst
