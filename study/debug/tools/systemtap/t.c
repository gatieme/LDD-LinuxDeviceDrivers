/*************************************************************************
    > File Name: t.c
    > Author: gatieme
    > Created Time: Thu 13 Apr 2017 06:13:39 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>


void fun()
{
    malloc(1000);
}

int main(int argc, char *argv[])
{
    fun();

    return 0;
}
