// http://blog.csdn.net/liyuanbhu/article/details/42470577
/*
	mm/memory.o: In function `insert_pfn':
	/home/gatieme/Work/Kernel/linux/linux/mm/memory.c:1813: undefined reference to `pte_mkdevmap'
*/
#include <stdlib.h>
#include <stdio.h>

//#define CONFIG_TARGET_X86_64

#ifdef CONFIG_TARGET_X86_64
static void A( )
{
	printf("A\n");
}
#else
void A( ) __attribute__ ((weak));;
#endif

static void B( )
{
	printf("B\n");
}


static int xx( )
{
#ifdef CONFIG_TARGET_X86_64
  return 1;
#else
  return 0;
#endif
}

int main(void)
{
  if (xx( ))   /* define CONFIG_TARGET_X86_64 */
    A( );
  else
    B( );
}
