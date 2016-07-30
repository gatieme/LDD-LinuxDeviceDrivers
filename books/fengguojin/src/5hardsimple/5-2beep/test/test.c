#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BEEP_1_ON   0
#define BEEP_1_OFF  1


int main(void)
{
	int fd;
	int i=0;
    
	fd = open("/dev/beep",O_RDWR);
	if ( fd < 0  )
	{
		printf("Cann't open beep fd=0x%08X\n",fd);
		exit(0);
	}

	ioctl(fd,BEEP_1_ON,0,0);
	sleep(1);
	ioctl(fd,BEEP_1_OFF,0,0);
	
	close(fd);
	return 0;
}
