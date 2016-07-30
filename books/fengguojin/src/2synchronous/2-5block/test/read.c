#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
main()
{
	int fd;
	char num[2];

	fd = open("/dev/fgj", O_RDWR, S_IRUSR | S_IWUSR);
	if (fd != - 1)
	{
		while (1)
		{
			read(fd, &num,2); //程序将阻塞在此语句
			printf("The data is %s\n", num);

			//如果输入是5，则退出
			if (num[1] ==0x35)
			{
				close(fd);
				break;
			}
		}
	}
	else
	{
		printf("device open failure\n");
	}
}
