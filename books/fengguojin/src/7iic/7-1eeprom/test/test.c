#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#define I2C_RETRIES     0x0701
#define I2C_TIMEOUT     0x0702
#define I2C_SLAVE       0x0703
#define I2C_TENBIT      0x0704

   
#define CHIP_ADDR 0x50  // 设备地址    
#define PAGE_SIZE 8    //页写入大小    
#define I2C_DEV   "/dev/i2c-0"    
   
static int read_eeprom (int fd,char buff[],int addr,int count)   
{   
  int res;   
  if (write(fd,&addr,1)!=1)  //写地址失败    
     return -1;   
  res=read(fd,buff,count);   
  printf("read %d byte at 0x %x\n",res,addr);   
  return res;   
}   
//缓冲区不能超过一页    
static int write_eeprom (int fd, char buff[],int addr,int count)   
{   
  int res;   
  int i;   
  static char sendbuffer[PAGE_SIZE+1];   
  memcpy(sendbuffer + 1,buff,count);   
  sendbuffer[0] = addr;   
  res = write(fd,&sendbuffer,count + 1);   
  printf("write %d byte at 0x%x\n",res,addr);   
     
}   
   
int main(void)   
{   
   int fd,n,res;   
   unsigned char buf[PAGE_SIZE]={1,2,3,4,5,6,7,8};   
      
   fd = open(I2C_DEV,O_RDWR);   
   if(fd<0)   
    {   
        printf("####i2c test device open failed ####\n");   
        return(-1);   
    }   
  res = ioctl(fd,I2C_TENBIT,0);  //不是10位模式    
  res = ioctl(fd,I2C_SLAVE,CHIP_ADDR);//设置I2C从设备地址
     
  write_eeprom(fd,buf,0,sizeof(buf));   
  memset(buf,0,sizeof(buf));
  read_eeprom(fd,buf,0,sizeof(buf));   
     
  for(n= 0;n<sizeof(buf);n++)
  {
	printf("0x%x\n",buf[n]);
  } 
  close(fd);
}