#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<linux/rtc.h>
#include<linux/ioctl.h>
#include<stdio.h>
#include<stdlib.h>

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
	
	retval=write(fd,"fgj",3);
	if(retval==-1)
	{
		perror("write error\n");
		exit(-1);
	}
	retval=read(fd,data,3);
	if(retval==-1)
	{
		perror("read error\n");
		exit(-1);
	}
	data[retval]=0;
	printf("read successfully:%s\n",data);
	
	close(fd);
}
