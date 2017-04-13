/*************************************************************************
    > File Name: build_bug_on_null.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: Thu 13 Apr 2017 11:20:31 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>


#include "bug.h"

#ifndef BUILD_BUG_ON_NULL
#define BUILD_BUG_ON_NULL(e) ((void *)sizeof(struct { int:-!!(e); }))
#endif

#define POINT  !(NULL)
int main(int argc, char *argv[])
{
    BUILD_BUG_ON_NULL(POINT);

	return EXIT_SUCCESS;
}


