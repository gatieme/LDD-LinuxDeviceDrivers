#
# Linux Frame Buffer Device Configuration
#

CC =		gcc -Wall -O2 -I.
RM =		rm -f

All:		test

test:		fb_display.o test.o

test.o:		test.c
fb_display.o:	fb_display.c fb_display.h

clean:
		$(RM) *.o test
