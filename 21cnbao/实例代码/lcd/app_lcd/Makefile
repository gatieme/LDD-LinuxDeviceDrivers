CC = /opt/host/armv4l/bin/armv4l-unknown-linux-gcc


CFLAGS =  -I/linux2410/kernel/include -Wall -Wstrict-prototypes -Wno-trigraphs -Os -mapcs -fno-strict-aliasing -fno-common -fno-common -pipe -mapcs-32 
      
app_lcd: app_lcd.c
	$(CC) $(CFLAGS) -o app_lcdd  app_lcd.c

clean:
	-rm -f *.o