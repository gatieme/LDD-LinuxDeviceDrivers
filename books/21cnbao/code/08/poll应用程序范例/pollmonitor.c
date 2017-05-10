/*======================================================================
    A test program in userspace
    This example is to introduce the ways to use "select"
     and driver poll

    The initial developer of the original code is Baohua Song
    <author@linuxdriver.cn>. All Rights Reserved.
======================================================================*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#define FIFO_CLEAR 0x1
#define BUFFER_LEN 20
main()
{
  int fd, num;
  char rd_ch[BUFFER_LEN];
  fd_set rfds,wfds;

  /*以非阻塞方式打开/dev/globalmem设备文件*/
  fd = open("/dev/globalfifo", O_RDONLY | O_NONBLOCK);
  if (fd !=  - 1)
  {
    /*FIFO清0*/
    if (ioctl(fd, FIFO_CLEAR, 0) < 0)
    {
      printf("ioctl command failed\n");
    }
    while (1)
    {
      FD_ZERO(&rfds);
      FD_ZERO(&wfds);
      FD_SET(fd, &rfds);
      FD_SET(fd, &wfds);

      select(fd + 1, &rfds, &wfds, NULL, NULL);
      /*数据可获得*/
      if (FD_ISSET(fd, &rfds))
      {
      	printf("Poll monitor:can be read\n");
      }
      /*数据可写入*/
      if (FD_ISSET(fd, &wfds))
      {
      	printf("Poll monitor:can be written\n");
      }
    }
  }
  else
  {
    printf("Device open failure\n");
  }
}
