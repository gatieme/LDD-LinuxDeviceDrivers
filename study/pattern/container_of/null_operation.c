/*************************************************************************
    > File Name: 1.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: Tue 18 Jul 2017 11:09:48 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
        /////////////////////
        //  合法操作
        /////////////////////
	//  取地址
        printf("address = %p\n", &(*(int *)0));

        //  sizeof 运算
        printf("sizeof  = %ld\n", sizeof(*(int *)0));

        //  typeof 运算
        typeof(*(int *)0) temp1 = 8;
        printf("temp = %d\n", temp1);

        /////////////////////
        //  非法操作
        /////////////////////
        //  对 NULL 指针进行读
        typeof(*(int *)0) temp2 = *(int *)0;

        //  对 NULL 指针进行写
        *(int *)0 = 7;
        return EXIT_SUCCESS;
}


