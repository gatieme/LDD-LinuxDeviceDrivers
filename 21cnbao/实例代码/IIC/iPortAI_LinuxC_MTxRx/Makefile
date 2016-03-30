CC = gcc
CFLAGS = 

iPortAI_LinuxC_MTxRx: iPortAI_LinuxC_MTxRx.c
	$(CC) $(CFLAGS) -o iPortAI_LinuxC_MTxRx iPortAI_LinuxC_MTxRx.c

clean:
	rm -f iPortAI_LinuxC_MTxRx *~ iPortAI_LinuxC_MTxRx.tar.gz iPortAI_LinuxC_MTxRx.zip

tar:
	make clean
	tar -cvvf iPortAI_LinuxC_MTxRx.tar ../iPortAI_LinuxC_MTxRx
	gzip iPortAI_LinuxC_MTxRx.tar

zip:
	make clean
	zip -r iPortAI_LinuxC_MTxRx ../iPortAI_LinuxC_MTxRx
