#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<linux/rtc.h>
#include<linux/ioctl.h>
#include<stdio.h>
#include<stdlib.h>
#include<linux/input.h>

void main()
{
	int fd = -1;
	char name[256]= "Unknown";
	int yalv;
	
	if ((fd = open("/dev/input/event2", O_WRONLY)) < 0) {
		perror("evdev open");
		exit(1);
	}
	
	if(ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
		perror("evdev ioctl");
	}
	
	size_t rb;
	struct input_event ev[2];
	ev[0].type=EV_LED;
	ev[0].code=LED_NUML;
	ev[0].value=0;
	
	ev[1].type=EV_LED;
	ev[1].code=LED_NUML;
	ev[1].value=1;
	while(1)
	{
		rb=write(fd,ev,sizeof(struct input_event));
		
		if (rb < (int) sizeof(struct input_event))
		{
			perror("evtest: short write");
			exit (1);
		}
		sleep(1);
		
		rb=write(fd,&ev[1],sizeof(struct input_event));
		if (rb < (int) sizeof(struct input_event))
		{
			perror("evtest: short write");
			exit (1);
		}
		sleep(1);
	}
	
	close(fd);
}
