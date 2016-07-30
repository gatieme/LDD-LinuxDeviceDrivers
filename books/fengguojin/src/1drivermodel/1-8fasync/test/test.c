#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<linux/rtc.h>
#include<linux/ioctl.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>

int fd;
void fasync_handler(int num)
{
   printf("fasync_handler entering\n");
}

void main()
{
  int i=2;
  char data[256];
  int oflags=0;
  int retval;

  signal(SIGIO, fasync_handler);
  fd=open("/dev/fcn",O_RDWR);
  if(fd==-1)
  {
     perror("error open\n");
     exit(-1);
  }
  printf("open /dev/fcn successfully\n");
  //使能了异步的通知到当前进程
  fcntl(fd, F_SETOWN, getpid());
  oflags=fcntl(fd, F_GETFL);
  fcntl(fd, F_SETFL, oflags | FASYNC);

  while(1);
  close(fd);
}
