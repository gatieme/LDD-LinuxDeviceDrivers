#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<linux/rtc.h>
#include<linux/ioctl.h>
#include<stdio.h>
#include<stdlib.h>

#define COMMAND_LEDON   1
#define COMMAND_LEDOFF  2

void main()
{
	int fd;
	int i=2;
	char data[256];
	
	int retval;
	fd=open("/dev/led",O_RDWR);
	if(fd==-1)
	{
		perror("error open\n");
		exit(-1);
	}
	printf("open /dev/led successfully\n");
	
	for(i=0;i<4;i++)
	{
		retval=ioctl(fd,COMMAND_LEDON,&i);
		if(retval==-1)
		{
			perror("ioctl LEDON error\n");
			exit(-1);
		}
		sleep(1);
		retval=ioctl(fd,COMMAND_LEDOFF,&i);
		if(retval==-1)
		{
			perror("ioctl LEDOFF error\n");
			exit(-1);
		}
	}
	close(fd);
}
