#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
//**********************
#define       DEBUG
//*********************

int main()
{
        int fd;
        int i,j,ret;
        char buf[11];
        fd=open("/dev/can_bus",O_RDWR);
        if (fd<0)
        {
        printf("Error open can_bus\n");
        }

      
#ifdef  DEBUG
        printf("read begin!\n");
#endif
        ret=read(fd,buf,11);
        printf("\nData received is:");
//        sleep(10);
	
	for(i=1;i<11;i++)
	{
	printf("%x",buf[i]);
	}

#ifdef  DEBUG
        printf("\nread end!\n");
#endif
        


}
