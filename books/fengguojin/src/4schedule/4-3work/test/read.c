#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
void main()
{
	int fd;
	char buf[2];

	fd = open("/dev/fgj", O_RDWR, S_IRUSR | S_IWUSR);
	if (fd != - 1)
	{
		read(fd, &buf,2); 
		buf[2]=0;
		printf("The data is %s\n", buf);
	}
	else
	{
		printf("device open failed\n");
	}
	close(fd);
}
