#include <linux/init.h>
#include <linux/module.h>

long multiply_test(long a,long b)
{
	return a*b;
}

long divide_test(long a,long b)
{
	if(b!=0)
	{
		return a/b;
	}
	return 0;
}
EXPORT_SYMBOL(multiply_test);
EXPORT_SYMBOL(divide_test);
MODULE_LICENSE("Dual BSD/GPL"); 
