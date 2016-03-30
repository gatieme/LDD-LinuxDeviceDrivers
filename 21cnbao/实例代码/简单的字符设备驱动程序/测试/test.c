#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>      // open() close()
#include <unistd.h>     // read() write()

#define DEVICE_NAME "/dev/gpio_led_ctl"

//define LED STATUS
#define LED_ON  0
#define LED_OFF 1


//------------------------------------- main ---------------------------------------------
int main(void)
{
        int fd;
	int ret;
	char *i;

        printf("\nstart gpio_led_driver test\n\n");

        fd = open(DEVICE_NAME, O_RDWR);
	
	printf("fd = %d\n",fd);
	

        if (fd == -1)
        {
                printf("open device %s error\n",DEVICE_NAME);
        }
        else
        {
		while(1)
		{	ioctl(fd,LED_OFF);
			sleep(1);//等待1秒再做下一步操作
			ioctl(fd,LED_ON);
			sleep(1);

		}
	        // close 
		ret = close(fd);
		printf ("ret=%d\n",ret);
		printf ("close gpio_led_driver test\n");
        }

        return 0;
}// end main
