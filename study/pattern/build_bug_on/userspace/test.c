#include <stdio.h>
#include <stdbool.h>
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
int main()
{
   BUILD_BUG_ON(0==0);
   bool zero = false;
   printf("%d\n", !!zero);
   printf("%d\n", !zero);
   return 0;
}