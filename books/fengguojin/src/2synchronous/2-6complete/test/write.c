#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
main()
{
	int fd;
	char num[2];
	int i=0;

	fd = open("/dev/fgj", O_RDWR, S_IRUSR | S_IWUSR);
	if (fd != - 1)
	{
                num[0]='F';
		num[1]='G';
		write(fd, num, 2);
		close(fd);
	}
	else
	{
		printf("device open failure\n");
	}
	exit(0);
}
