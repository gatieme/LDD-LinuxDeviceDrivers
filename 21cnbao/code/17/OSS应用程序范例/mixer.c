/*                                                                   
 * mixer.c  
 * 对各种混音通道的增益进行调节，其所有的功能都通过读写/dev/mixer设备文件来完成                                                         
 */                                                                  
#include <unistd.h>                                                  
#include <stdlib.h>                                                  
#include <stdio.h>                                                   
#include <sys/ioctl.h>                                               
#include <fcntl.h>                                                   
#include <linux/soundcard.h>                                         
/* 用来存储所有可用混音设备的名称 */                                 
const char *sound_device_names[] = SOUND_DEVICE_NAMES;               
int fd;                  /* 混音设备所对应的文件描述符 */            
int devmask, stereodevs; /* 混音器信息对应的位图掩码 */              
char *name;                                                          
/* 显示命令的使用方法及所有可用的混音设备 */                         
void usage()                                                         
{                                                                    
  int i;                                                             
  fprintf(stderr, "usage: %s <device> <left-gain%%> <right-gain%%>\n"
	  "       %s <device> <gain%%>\n\n"                                
	  "Where <device> is one of:\n", name, name);                      
  for (i = 0 ; i < SOUND_MIXER_NRDEVICES ; i++)                      
    if ((1 << i) & devmask) /* 只显示有效的混音设备 */               
      fprintf(stderr, "%s ", sound_device_names[i]);                 
  fprintf(stderr, "\n");                                             
  exit(1);                                                           
}                                                                    
int main(int argc, char *argv[])                                     
{                                                                    
  int left, right, level;  /* 增益设置 */                            
  int status;              /* 系统调用的返回值 */                    
  int device;              /* 选用的混音设备 */                      
  char *dev;               /* 混音设备的名称 */                      
  int i;                                                             
  name = argv[0];                                                    
  /* 以只读方式打开混音设备 */                                       
  fd = open("/dev/mixer", O_RDONLY);                                 
  if (fd == -1) {                                                    
    perror("unable to open /dev/mixer");                             
    exit(1);                                                         
  }                                                                  
                                                                     
  /* 获得所需要的信息 */                                             
  status = ioctl(fd, SOUND_MIXER_READ_DEVMASK, &devmask);            
  if (status == -1)                                                  
    perror("SOUND_MIXER_READ_DEVMASK ioctl failed");                 
  status = ioctl(fd, SOUND_MIXER_READ_STEREODEVS, &stereodevs);      
  if (status == -1)                                                  
    perror("SOUND_MIXER_READ_STEREODEVS ioctl failed");              
  /* 检查用户输入 */                                                 
  if (argc != 3 && argc != 4)                                        
    usage();                                                         
  /* 保存用户输入的混音器名称 */                                     
  dev = argv[1];                                                     
  /* 确定即将用到的混音设备 */                                       
  for (i = 0 ; i < SOUND_MIXER_NRDEVICES ; i++)                      
    if (((1 << i) & devmask) && !strcmp(dev, sound_device_names[i])) 
      break;                                                         
  if (i == SOUND_MIXER_NRDEVICES) { /* 没有找到匹配项 */             
    fprintf(stderr, "%s is not a valid mixer device\n", dev);        
    usage();                                                         
  }                                                                  
  /* 查找到有效的混音设备 */                                         
  device = i;                                                        
  /* 获取增益值 */                                                   
  if (argc == 4) {                                                   
    /* 左、右声道均给定 */                                           
    left  = atoi(argv[2]);                                           
    right = atoi(argv[3]);                                           
  } else {                                                           
    /* 左、右声道设为相等 */                                         
    left  = atoi(argv[2]);                                           
    right = atoi(argv[2]);                                           
  }                                                                  
                                                                     
  /* 对非立体声设备给出警告信息 */                                   
  if ((left != right) && !((1 << i) & stereodevs)) {                 
    fprintf(stderr, "warning: %s is not a stereo device\n", dev);    
  }                                                                  
                                                                     
  /* 将两个声道的值合到同一变量中 */                                 
  level = (right << 8) + left;                                       
                                                                     
  /* 设置增益 */                                                     
  status = ioctl(fd, MIXER_WRITE(device), &level);                   
  if (status == -1) {                                                
    perror("MIXER_WRITE ioctl failed");                              
    exit(1);                                                         
  }                                                                  
  /* 获得从驱动返回的左右声道的增益 */                               
  left  = level & 0xff;                                              
  right = (level & 0xff00) >> 8;                                     
  /* 显示实际设置的增益 */                                           
  fprintf(stderr, "%s gain set to %d%% / %d%%\n", dev, left, right); 
  /* 关闭混音设备 */                                                 
  close(fd);                                                         
  return 0;                                                          
}                                                                    