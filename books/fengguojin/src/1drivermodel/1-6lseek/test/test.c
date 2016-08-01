#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<linux/rtc.h>
#include<linux/ioctl.h>
#include<stdio.h>
#include<stdlib.h>


#define     MAX_SIZE    256
#define     DEV_FILE    "/dev/gatieme"

int main(void)
{
	int fd;
	int i;
	char data[MAX_SIZE];

	int retval;

	fd = open(DEV_FILE, O_RDWR);
	if(fd == -1)
	{
		perror("error open\n");
		exit(-1);
	}
	printf("open " DEV_FILE " successfully\n");

    //  read data
    retval = read(fd, data, MAX_SIZE);
	if(retval == -1)
	{
		perror("read error");
		exit(-1);
	}
	printf("read successfully : %s\n", data);

    // lseek 5 and read data
    retval = lseek(fd, 5, 0);
	if(retval == -1)
	{
		perror("lseek error\n");
		exit(-1);
	}
    printf("lseek 5\n");

	retval = read(fd, data, MAX_SIZE);
	if(retval == -1)
	{
		perror("read error");
		exit(-1);
	}

	data[retval] = 0;
	printf("read successfully : %s\n", data);

    // lseek 2 and read data
	retval = lseek(fd, 2, 0);
	if(retval == -1)
	{
		perror("lseek error");
		exit(-1);
	}
    printf("lseek 2\n");

    retval = read(fd, data, MAX_SIZE);
	if(retval == -1)
	{
		perror("read error");
		exit(-1);
	}
	data[retval] = 0;

	printf("read successfully : %s\n", data);

	close(fd);
}
