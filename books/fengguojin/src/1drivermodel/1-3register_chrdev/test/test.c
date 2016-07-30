
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <linux/ioctl.h>





#define MAX_LINE    256
#define DEV_FILE    "/dev/gatieme"


int main(void)
{
	int fd;
	int i;
	char data[MAX_LINE] = "Hello World!";
	int retval;

	fd = open(DEV_FILE, O_RDWR);
	if(fd == -1)
	{
        perror("error open");
		exit(-1);
	}
	printf("open " DEV_FILE " successfully\n");

	retval = write(fd, data, strlen(data));
	if(retval == -1)
	{
		perror("write error");
		exit(-1);
	}
    else
    {
        printf("write data successfully\n");
    }
	retval = read(fd, data, MAX_LINE);
	if(retval == -1)
	{
		perror("read error");
		exit(-1);
	}

	data[retval] = '\0';
	printf("read successfully:%s\n", data);

	close(fd);
    printf("close " DEV_FILE "\n");
}
