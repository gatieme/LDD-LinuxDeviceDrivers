/*************************************************************************
    > File Name: build_bug_on_zero.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: Wed 12 Apr 2017 01:14:14 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>

//#include "bug.h"

#ifndef BUILD_BUG_ON_ZERO
#define BUILD_BUG_ON_ZERO(e) (sizeof(struct { int:-!!(e); }))
#endif


#define NUM 0
int main(int argc, char *argv[])
{
    //sizeof(struct{ int : -!!( 0); });
    //sizeof(struct{ int : -!!(-1); });
    BUILD_BUG_ON_ZERO(NUM == 1);
    BUILD_BUG_ON_ZERO(NUM == 0);


	return EXIT_SUCCESS;
}


