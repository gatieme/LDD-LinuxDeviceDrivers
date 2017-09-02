/*************************************************************************
    > File Name: test.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: Tue 18 Jul 2017 09:08:52 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>


struct test
{
        int     a;
        int     b;
};

int main(int argc, char *argv[])
{
        int *temp = &(((struct test *)0)->b);   //  -=> OK
        typeof(((struct test *)0)->b) t1 = 8;   //  -=> OK
        int t2 = ((struct test *)0)->b;
        printf("temp = %d\n", *temp);

	return EXIT_SUCCESS;
}
