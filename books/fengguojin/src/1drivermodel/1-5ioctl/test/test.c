#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <linux/ioctl.h>

#include <stdio.h>
#include <stdlib.h>


#define COMMAND_1    1
#define COMMAND_2    2


#define DEV_FILE    "/dev/gatieme"



int main(void)
{
	int fd;
	int i;
	char data[256];

	int retval;

    fd = open(DEV_FILE, O_RDWR);
	if(fd == -1)
	{
		perror("error open");
		exit(-1);
	}
	printf("open " DEV_FILE " successfully\n");


	retval = ioctl(fd, COMMAND_1, 0);
	if(retval == -1)
	{
		perror("ioctl error");
		exit(-1);
	}
	printf("send command1 successfully\n");


    retval = ioctl(fd, COMMAND_2, 0);
	if(retval == -1)
	{
		perror("ioctl error");
		exit(-1);
	}
	printf("send command2 successfully\n");

    close(fd);
}
