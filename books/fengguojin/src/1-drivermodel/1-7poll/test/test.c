#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <linux/ioctl.h>
#include <sys/select.h>

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

    fd_set      rfds;   /*  the set of the readable files   */
    fd_set      wfds;   /*  the set of the writeable files  */
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
			printf("read successfully : %s\n",data);
		}

        sleep(1);
	}
    return (void *)0;
}



int main(void)
{

	int     i;
	int     retval;
    char    str[3][81] = { "gatieme", "wangpanpan", "Hello World!!!" };
    //char    (*pstr)[81] = str;

    fd = open(DEV_FILE, O_RDWR);
    if(fd == -1)
	{
		perror("error open\n");
		exit(-1);
	}
	printf("open " DEV_FILE " successfully\n");

	pthread_t   tid;
	pthread_create(&tid, NULL, readthread, NULL);

    srand(time(NULL));

	while( 1 )
	{
        i = rand( ) % 3;
		retval = write(fd, str[i], strlen(str[i]));
		if(retval == -1)
		{
			perror("write error\n");
			exit(-1);
		}
        sleep(2);
	}

	close(fd);
}
