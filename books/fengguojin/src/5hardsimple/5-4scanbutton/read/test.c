#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<linux/rtc.h>
#include<linux/ioctl.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

int offset[]={0x3E,0x3D,0x3B,0x37,0x2F,0x1F};
void main(void)
{
	int fd;
	int i;
	int retval=0,oldvalue=0;
	fd=open("/dev/fgj",O_RDWR);
	if(fd==-1)
	{
		perror("error open\n");
		exit(-1);
	}
	printf("open /dev/fgj successfully\n");
	while(1)
	{
		i=read(fd,&retval,4);
		if(i>0)
		{
			if(retval!=oldvalue)
			{
				for(i=0;i<6;i++)
				{
					if(offset[i]==retval)
					{
						printf("key%d is pressed\n",i+1);
					}
				}
			}
			oldvalue=retval;
		}
		usleep(10);
	}
	close(fd);
}
