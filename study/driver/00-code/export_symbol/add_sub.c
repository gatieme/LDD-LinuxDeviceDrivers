#include <linux/init.h>
#include <linux/module.h>


/*  函数返回 a 和 a 的和  */
long add_integer(int a, int b)
{
    return a + b;
}
EXPORT_SYMBOL(add_integer);


/*  函数返回 a 和 b 的差  */
long sub_integer(int a, int b)
{
    return a - b;
}
EXPORT_SYMBOL_GPL(sub_integer);


MODULE_LICENSE("Dual BSD/GPL");
