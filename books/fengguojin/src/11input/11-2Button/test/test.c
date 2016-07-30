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
	
	if ((fd = open("/dev/input/event3", O_RDONLY)) < 0) {
		perror("evdev open");
		exit(1);
	}
	
	if(ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
		perror("evdev ioctl");
	}
	
	size_t rb;
	struct input_event ev[2];
	
	while(1)
	{
		rb=read(fd,ev,sizeof(struct input_event)*2);
		if (rb < (int) sizeof(struct input_event))
		{
			perror("evtest: short read");
			exit (1);
		}
		for (yalv = 0;yalv < (int) (rb / sizeof(struct input_event));yalv++)
		{
			if (EV_KEY == ev[yalv].type)
			{
				printf("time:%ld.%06ld (s)  ",ev[yalv].time.tv_sec,ev[yalv].time.tv_usec);
				printf("type %d code %d value %d\n",ev[yalv].type,ev[yalv].code,ev[yalv].value);
			}
		}
	}
	
	close(fd);
}
