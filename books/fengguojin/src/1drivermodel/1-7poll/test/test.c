#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <linux/ioctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>


#define     DEV_FILE    "/dev/gatieme"
#define     MAX_SIZE    256


int         fd;



void *readthread(void *arg)
{
	char data[MAX_SIZE];

    fd_set      rfds; //¶ÁÃèÊö·û¼¯ºÏ
    fd_set      wfds; //Ð´ÃèÊö·û¼¯ºÏ
	int         retval = 0;

    while( 1 )
	{
		FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
		select(fd + 1, &rfds, &wfds, NULL, NULL);

        if(FD_ISSET(fd, &rfds))
		{
			retval = read(fd, data, MAX_SIZE);

			if(retval == -1)
            {
				perror("read error\n");
				exit(-1);
			}
			data[retval] = 0;
			printf("read successfully:%s\n",data);
		}
	}
    return (void *)0;
}



int main(void)
{

	int i;
	int retval;

    fd = open(DEV_FILE, O_RDWR);
    if(fd == -1)
	{
		perror("error open\n");
		exit(-1);
	}
	printf("open " DEV_FILE " successfully\n");

	pthread_t   tid;
	pthread_create(&tid, NULL, readthread, NULL);

	while( 1 )
	{
		retval = write(fd, "gatieme", strlen("gatieme"));
		if(retval == -1)
		{
			perror("write error\n");
			exit(-1);
		}
	}

	close(fd);
}
