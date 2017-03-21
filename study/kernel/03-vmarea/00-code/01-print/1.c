/*************************************************************************
    > File Name: 1.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: Tue 21 Mar 2017 11:45:53 PM CST
 ************************************************************************/



//  Linux内核源代码情景分析
//  2.2  地址映射的全过程
#include <stdio.h>
#include <stdlib.h>


void greeting(void)
{
    printf("Hello World\n");
}

int main(int argc, char *argv[])
{
    greeting( );

	return EXIT_SUCCESS;
}


