/*************************************************************************
    > File Name: test.c
    > Author: gatieme
    > Created Time: 2016年05月24日 星期二 19时45分38秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>


#define PROC_FILE   "/proc/jif"
#define MAX_LINE    81

int main( )
{
	int             ret;
	int             procFile;
	char            buff[MAX_LINE] = "hello";

    //wait for ack signal
	procFile = open(PROC_FILE, O_RDWR);
	if(procFile == -1)
	{
		perror("Fail to open "PROC_FILE);
		exit(-1);
	}
    else
    {
        printf("Open success...\n");
    }
    printf("user buffer : %p\n", buff);
    if((ret = write(procFile, buff, strlen(buff))) == -1)
    {
        perror("Fail to read "PROC_FILE);
    }
    else
    {
        printf("wriet success\n");
    }

    bzero(buff, sizeof(buff));
    if((ret = read(procFile, buff, MAX_LINE)) == -1)
    {
        perror("Fail to read "PROC_FILE);
    }
    else
    {
        printf("buff = %s\n", buff);
    }

    close(procFile);
}
