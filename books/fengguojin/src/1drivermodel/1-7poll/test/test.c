#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<linux/rtc.h>
#include<linux/ioctl.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

int fd;

void *readthread(void *arg)
{
	char data[256];
    fd_set rfds; //¶ÁÃèÊö·û¼¯ºÏ
    fd_set wfds; //Ð´ÃèÊö·û¼¯ºÏ
	int retval=0;
	while(1)
	{
		FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
		select(fd+1, &rfds, &wfds, NULL, NULL);		
		if(FD_ISSET(fd, &rfds))
		{
			retval=read(fd,data,3);
			if(retval==-1)
			{
				perror("read error\n");
				exit(-1);
			}
			data[retval]=0;
			printf("read successfully:%s\n",data);
		}
	}
    return (void *)0;
} 

main()
{
	
	int i;
	int retval;
	fd=open("/dev/fgj",O_RDWR);
	if(fd==-1)
	{
		perror("error open\n");
		exit(-1);
	}
	printf("open /dev/fgj successfully\n");
	
	pthread_t tid;
	pthread_create(&tid, NULL, readthread, NULL);
	
	while(1)
	{
		retval=write(fd,"fgj",3);
		if(retval==-1)
		{
			perror("write error\n");
			exit(-1);
		}
	}
	close(fd);
}
