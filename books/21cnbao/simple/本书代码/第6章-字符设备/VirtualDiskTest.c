/*
 * File Name: VirtualDiskTest.c
 */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

void main()
{
   int fileno;/*用于文件表述符*/
   int number;
   char data[]="one two three four five six";/*写入VirtualDisk的数据*/
   char str[1024];/*用户空间缓冲*/
   int len;

   fileno = open("/dev/VirtualDisk",O_RDWR);/*以读写方式打开设备文件*/

   if (fileno == -1)/*打开文件失败*/
   {
   	printf("open VirtualDisk device errr!\n");
	return 0;
   }
    
   write(fileno,data,strlen(data));/*将数据写入设备*/
   close(fileno);/*关闭设备文件*/

   fileno=open("/dev/VirtualDisk",O_RDWR);/*以读写方式打开设备文件*/
   len=read(fileno,str,1024);/*读出设备中的数据*/
   str[len]='\0';
   printf("%s\n",str);/*显示设备的数据*/
   close(fileno);/*关闭设备文件*/
   
   fileno=open("/dev/VirtualDisk",O_RDWR);/*以读写方式打开设备文件*/
   lseek(fileno,4,SEEK_SET);/*将文件指针后移4字节，当前位置的字符为t*/
   len=read(fileno,str,1024);/*读出设备中的数据*/
   str[len]='\0';
   printf("%s\n",str);/*显示设备的数据*/
   close(fileno);/*关闭设备文件*/

   return 0;
}