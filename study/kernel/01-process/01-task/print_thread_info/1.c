/*************************************************************************
    > File Name: 1.c
    > Author: gatieme
    > Created Time: Thu 02 Jun 2016 01:09:41 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>


 int main(void)
{

    long a = 10;
    long *pa = &a;
    printf("0x%x, 0x%x\n", ++a, a);
    printf("%p\n", pa);
    printf("%p\n", ++pa);
}
