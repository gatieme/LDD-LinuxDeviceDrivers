#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#define I2C_READ_DATA  1
#define I2C_WRITE_DATA 2
struct I2C_MSGbuffer
{
	unsigned short len;
	unsigned char addr;
	char buffer[64];
};


int main(int argc, char **argv)
{ 
    struct I2C_MSGbuffer msg;
    unsigned int fd, i; 
    int ret; 
    fd = open("/dev/i2c", O_RDWR); 
    if (!fd) 
    {     
        printf("opening i2c device Error\n"); 
        return 0; 
    } 

	memset(&msg,0,sizeof(struct I2C_MSGbuffer));
	msg.addr=0x00;
	msg.len=5;
	for(i=0;i<5;i++)
	{
		msg.buffer[i]=0x80+i;
	}
	ret=ioctl(fd, I2C_WRITE_DATA, &msg);
	printf("write %d successfully\n",ret); 


	memset(&msg,0,sizeof(struct I2C_MSGbuffer));
	msg.addr=0x00;
	msg.len=5;
	ret=ioctl(fd, I2C_READ_DATA, &msg);
	if(ret>0)
	{
		for(i=0;i<ret;i++)
		{
			printf("add[0x%.2x]=0x%.2x\n",msg.addr+i,msg.buffer[i]); 
		}
	}
	else
	{
		 printf("read Error\n"); 
	}
    close(fd); 
    return;
}