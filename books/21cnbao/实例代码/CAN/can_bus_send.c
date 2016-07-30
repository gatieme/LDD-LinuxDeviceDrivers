#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <strings.h>

int main()
{
        int fd;
        int i,ret;
        char buf[11]={0 ,0xa5,0x28,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};	// can use
        fd=open("/dev/can_bus",O_RDWR);
        if (fd<0)
        {
        printf("Error open can_bus\n");
        }
        
        ret=write(fd,(char *)buf,11);
        
        return 0;

}
