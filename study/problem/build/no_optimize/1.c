/*************************************************************************
    > File Name: 1.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: Sun 26 Nov 2017 11:47:59 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>


static void A( );


#ifdef CONFIG_TEST_MACRO
static void A( )
{
	printf("A\n");
}
#endif

static void xx( )
{
	A( );
}

int main(int argc, char *argv[])
{
	xx( );

	return EXIT_SUCCESS;
}


