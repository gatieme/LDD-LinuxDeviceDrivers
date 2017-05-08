#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

main()
{
	int fd,size;
	char s[]="Linux Programmer!\n";
	char buffer[80];
	
	fd=open("/tmp/temp",O_WRONLY|O_CREAT);/*以读写方式打开文件*/
	if(fd<=-1)
	{
		perror("open file error\n");
	}
	
	write(fd,s,sizeof(s));	/*写数据到文件*/
	close(fd);
	
	fd=open("/tmp/temp",O_RDONLY);
	if(fd<=-1)
	{
		perror("open file error\n");
	}
	
	size=read(fd,buffer,sizeof(buffer));/*从文件读出数据*/
	close(fd);
	
	printf("%s\n",buffer);
}