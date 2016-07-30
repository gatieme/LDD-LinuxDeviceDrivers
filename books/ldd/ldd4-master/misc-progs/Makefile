
FILES = nbtest load50 mapcmp polltest mapper setlevel setconsole inp outp \
	datasize dataalign netifdebug

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
INCLUDEDIR = $(KERNELDIR)/include
ccflags-y = -O2 -fomit-frame-pointer -Wall -I$(INCLUDEDIR)

all: $(FILES)

clean:
	rm -f $(FILES) *~ core

