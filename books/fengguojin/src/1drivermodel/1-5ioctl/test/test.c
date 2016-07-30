#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<linux/rtc.h>
#include<linux/ioctl.h>
#include<stdio.h>
#include<stdlib.h>

#define COMMAND1 1
#define COMMAND2 2

void main(void)
{
	int fd;
	int i;
	char data[256];
	
	int retval;
	fd=open("/dev/fgj",O_RDWR);
	if(fd==-1)
	{
		perror("error open\n");
		exit(-1);
	}
	printf("open /dev/fgj successfully\n");
	
	retval=ioctl(fd,COMMAND1,0);
	if(retval==-1)
	{
		perror("ioctl error\n");
		exit(-1);
	}
	printf("send command1 successfully\n");
	retval=ioctl(fd,COMMAND2,0);
	if(retval==-1)
	{
		perror("ioctl error\n");
		exit(-1);
	}
	printf("send command1 successfully\n");
	close(fd);
}
