CC = /opt/host/armv4l/bin/armv4l-unknown-linux-gcc

LD = /opt/host/armv4l/bin/armv4l-unknown-linux-ld

CFLAGS = -D__KERNEL__ -I/linux2410/arm830/kernel/include -Wall -Wstrict-prototypes -Wno-trigraphs -Os -mapcs -fno-strict-aliasing -fno-common -fno-common -pipe -mapcs-32 -march=armv4 -mtune=arm9tdmi -mshort-load-bytes -msoft-float -DKBUILD_BASENAME=arm7279_driver -I/opt/host/armv4l/src/linux/include -DMODULE
      
arm7279_driver.o: arm7279_driver.c
	$(CC) $(CFLAGS) -c $^ -o $@
	cp arm7279_driver.o /-f
clean:
	-rm -f *.o
