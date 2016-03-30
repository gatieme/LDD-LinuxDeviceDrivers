#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int fd;
	
	if ((fd = open("/dev/Kbd7279",0)) < 0)
	  {
		printf("cannot open /dev/Kbd7279\n");
		exit(0);
	};

	for (;;)
		ioctl(fd, 0, 0);
	
	close(fd);
}
