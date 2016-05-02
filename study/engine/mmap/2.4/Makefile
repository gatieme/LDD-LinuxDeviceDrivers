CC=gcc
INCLUDEDIR = /home/linux/include/
MODCFLAGS :=-Wall -O -DMODULE -D__KERNEL__ -DLINUX -I$(INCLUDEDIR)
map_driver.o: map_driver.c 
	$(CC) $(MODCFLAGS) -c map_driver.c
