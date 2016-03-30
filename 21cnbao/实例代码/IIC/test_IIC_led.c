#include <unistd.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>




#define LED_SLAVE_ADD	0x70

int main()
{
        int fd = open("/dev/led",O_RDWR);	//can read and write
	char null_buf[5]={-1,-1,-1,-1,-1};



        if (fd == -1)
        {
                perror("can't open device led_board!");
                exit(1);
        }



	while(1)
	{
		write(fd,null_buf,5);
		sleep(1);
	}
      close(fd);

}

