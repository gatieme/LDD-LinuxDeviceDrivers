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
	int             count;
	int             procFile;
	char            buff[MAX_LINE];

    bzero(buff, sizeof(buff));
    sprintf(buff, "echo \"hello\" > %s", PROC_FILE);
    printf("%s\n", buff);
	system(buff);
    printf("write success...\n");
    //wait for ack signal
	procFile = open(PROC_FILE, O_RDONLY);
	if(procFile == -1)
	{
		perror("Fail to open "PROC_FILE);
		exit(-1);
	}
    else
    {
        printf("Open success...\n");
    }

	bzero(buff, sizeof(buff));
    ret = read(procFile, buff, MAX_LINE);
    if(ret == -1)
    {
        perror("Fail to read " PROC_FILE);
    }
    else
    {
        printf("buff = %s\n", buff);
    }

    close(procFile);
}
