CC = gcc
CFLAGS = 

iPortAI_LinuxC_MSTxRx: iPortAI_LinuxC_MSTxRx.c
	$(CC) $(CFLAGS) -o iPortAI_LinuxC_MSTxRx iPortAI_LinuxC_MSTxRx.c

clean:
	rm -f iPortAI_LinuxC_MSTxRx *~ iPortAI_LinuxC_MSTxRx.tar.gz iPortAI_LinuxC_MSTxRx.zip

tar:
	make clean
	tar -cvvf iPortAI_LinuxC_MSTxRx.tar ../iPortAI_LinuxC_MSTxRx
	gzip iPortAI_LinuxC_MSTxRx.tar

zip:
	make clean
	zip -r iPortAI_LinuxC_MSTxRx ../iPortAI_LinuxC_MSTxRx
