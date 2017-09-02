/*************************************************************************
    > File Name: test2.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: Tue 18 Jul 2017 09:21:55 AM CST
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#define test1(a)                                \
do {                                            \
        typeof(a) temp = 7;                     \
}while(0);

int test2(int a)
{
        typeof(a) temp = 7;
}

int main(int argc, char *argv[])
{
        printf("%d\n", __LINE__);
        test1(*(int *)0);                       //  -=> run OK
        printf("%d\n", __LINE__);
        test2(*(int *)0);                       //  -=> run error
        printf("%d\n", __LINE__);

	return EXIT_SUCCESS;
}
