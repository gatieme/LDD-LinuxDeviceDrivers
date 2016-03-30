/*======================================================================
    A test program to access /dev/second
    This example is to help understand kernel timer 
    
    The initial developer of the original code is Baohua Song
    <author@linuxdriver.cn>. All Rights Reserved.
======================================================================*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

main()
{
  int fd;
  int counter = 0;
  int old_counter = 0;
  
  /*打开/dev/second设备文件*/
  fd = open("/dev/second", O_RDONLY);
  if (fd !=  - 1)
  {
    while (1)
    {
      read(fd,&counter, sizeof(unsigned int));//读目前经历的秒数
      if(counter!=old_counter)
      {	
      	printf("seconds after open /dev/second :%d\n",counter);
      	old_counter = counter;
      }	
    }    
  }
  else
  {
    printf("Device open failure\n");
  }
}
