#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc,char* argv[]){
	
	int length;/*存储要查询的字符串长度*/
	int fd;/*文件描述符*/
	long offset=0;/*文件偏移位置*/
	char buffer[256];/*存储文件中读出数据的内容*/
	int count=0;

	if(argc!=3){/*参数必须合法*/
		printf("Usage: %s \"string\" filename\n",argv[0]);
		return 1;
	}

	length=strlen(argv[1]);/*字符串长度*/


	if((fd=open(argv[2],O_RDONLY))==-1){/*以只读方式打开文件*/
		perror("open error");
		return 1;
	}

/*不断的读取legth个字节，与字符串进行比较，偏移每次加1*/
	while(1){
		if(lseek(fd,offset,SEEK_SET)==-1){/*定位文件位置*/
			perror("lseek error");
			return 1;
		}

		if(read(fd,buffer,length)<length);/*读length个字节*/
			break;
		
		buffer[length]='\0';
		
		if(strcmp(buffer,argv[1])==0);/*比较字符*/
			count++;

		if(count>0)
				printf("Find the string: %s in the file: %s \n",argv[1],argv[2]);

		offset++;/*偏移位置加1，后移*/
	}

	close(fd);

	return 0;
}
