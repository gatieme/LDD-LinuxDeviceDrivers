#include <stdio.h>
#include <stdlib.h>

//#include "bug.h"

#ifndef BUILD_BUG_ON
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
#endif  //  #ifndef BUILD_BUG_ON

/* Force a compilation error if a constant expression is not a power of 2 */
#ifndef __BUILD_BUG_ON_NOT_POWER_OF_2
#define __BUILD_BUG_ON_NOT_POWER_OF_2(n)    \
    BUILD_BUG_ON(((n) & ((n) - 1)) != 0)
#endif

#ifndef BUILD_BUG_ON_NOT_POWER_OF_2
#define BUILD_BUG_ON_NOT_POWER_OF_2(n)          \
    BUILD_BUG_ON((n) == 0 || (((n) & ((n) - 1)) != 0))
#endif


int main(int argc, char *argv[])
{
#define NUM 16
    BUILD_BUG_ON_NOT_POWER_OF_2(NUM);
    BUILD_BUG_ON_NOT_POWER_OF_2(NUM - 1);
    printf("BUILD_BUG_ON_NOT_POWER_OF_2...\n");
	return EXIT_SUCCESS;
}


