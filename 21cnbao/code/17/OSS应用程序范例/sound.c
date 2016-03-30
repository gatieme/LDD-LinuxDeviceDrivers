/*                                                   
 * sound.c   
 * 先录制几秒种音频数据，将其存放在内存缓冲区中，然后再进行回放，其所有的功能都是通过读写/dev/dsp设备文件来完成                                        
 */                                                  
#include <unistd.h>                                  
#include <fcntl.h>                                   
#include <sys/types.h>                               
#include <sys/ioctl.h>                               
#include <stdlib.h>                                  
#include <stdio.h>                                   
#include <linux/soundcard.h>                         
#define LENGTH 3    /* 存储秒数 */                   
#define RATE 8000   /* 采样频率 */                   
#define SIZE 8      /* 量化位数 */                   
#define CHANNELS 1  /* 声道数目 */                   
/* 用于保存数字音频数据的内存缓冲区 */               
unsigned char buf[LENGTH*RATE*SIZE*CHANNELS/8];      
int main()                                           
{                                                    
  int fd;	/* 声音设备的文件描述符 */                 
  int arg;	/* 用于ioctl调用的参数 */                
  int status;   /* 系统调用的返回值 */               
  /* 打开声音设备 */                                 
  fd = open("/dev/dsp", O_RDWR);                     
  if (fd < 0) {                                      
    perror("open of /dev/dsp failed");               
    exit(1);                                         
  }                                                  
  /* 设置采样时的量化位数 */                         
  arg = SIZE;                                        
  status = ioctl(fd, SOUND_PCM_WRITE_BITS, &arg);    
  if (status == -1)                                  
    perror("SOUND_PCM_WRITE_BITS ioctl failed");     
  if (arg != SIZE)                                   
    perror("unable to set sample size");             
  /* 设置采样时的声道数目 */                         
  arg = CHANNELS;                                    
  status = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &arg);
  if (status == -1)                                  
    perror("SOUND_PCM_WRITE_CHANNELS ioctl failed"); 
  if (arg != CHANNELS)                               
    perror("unable to set number of channels");      
  /* 设置采样时的采样频率 */                         
  arg = RATE;                                        
  status = ioctl(fd, SOUND_PCM_WRITE_RATE, &arg);    
  if (status == -1)                                  
    perror("SOUND_PCM_WRITE_WRITE ioctl failed");    
  /* 循环，直到按下Control-C */                      
  while (1) {                                        
    printf("Say something:\n");                      
    status = read(fd, buf, sizeof(buf)); /* 录音 */  
    if (status != sizeof(buf))                       
      perror("read wrong number of bytes");          
    printf("You said:\n");                           
    status = write(fd, buf, sizeof(buf)); /* 回放 */ 
    if (status != sizeof(buf))                       
      perror("wrote wrong number of bytes");         
    /* 在继续录音前等待回放结束 */                   
    status = ioctl(fd, SOUND_PCM_SYNC, 0);           
    if (status == -1)                                
      perror("SOUND_PCM_SYNC ioctl failed");         
  }                                                  
}                                                    