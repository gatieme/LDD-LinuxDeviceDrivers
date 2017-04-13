/*************************************************************************
    > File Name: build_bug_on_invalid.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: Thu 13 Apr 2017 11:26:18 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>


#include "bug.h"

# define __force        __attribute__((force))

#ifndef BUILD_BUG_ON_INVALID
/*
 * BUILD_BUG_ON_INVALID() permits the compiler to check the validity of the
 * expression but avoids the generation of any code, even if that expression
 * has side-effects.
 */
#define BUILD_BUG_ON_INVALID(e) ((void)(sizeof((__force long)(e))))
#endif


#define NUM 12_$
int main(int argc, char *argv[])
{
    BUILD_BUG_ON_INVALID(NUM);

	return EXIT_SUCCESS;
}


