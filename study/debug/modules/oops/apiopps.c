/*************************************************************************
    > File Name: apiopps.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: Sat 08 Apr 2017 08:27:08 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>

const  char array[] = "\x6b\xc0";

int main(int argc, char *argv[])
{
    printf("%p\n", array);
    *(int *)0 = 0;

	return EXIT_SUCCESS;
}


