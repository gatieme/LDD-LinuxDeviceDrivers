CC = /opt/host/armv4l/bin/armv4l-unknown-linux-gcc


CFLAGS =  -I/linux2410/arm830/kernel/include -Wall -Wstrict-prototypes -Wno-trigraphs -Os -mapcs -fno-strict-aliasing -fno-common -fno-common -pipe -mapcs-32 
      
write: write.c
	$(CC) $(CFLAGS) -o write  write.c

clean:
	-rm -f *.o
