#include <linux/init.h>
#include <linux/module.h>

#include "add_sub.h"


/*  定义模块传递的参数 a, b  */
static long a = 1;
module_param(a, long, S_IRUGO);

static long b = 1;
module_param(b, long, S_IRUGO);

static int add_or_sub = 1;
module_param(add_or_sub, int, S_IRUGO);


static int test_init(void)
{
    long result = 0;
    printk(KERN_ALERT "test init\n");

    if(add_or_sub == 1)
    {
        result = add_integer(a, b);
    }
    else
    {
        result = sub_integer(a, b);
    }

    printk(KERN_ALERT "The %s result is %ld\n",
            add_or_sub == 1 ? "ADD" : "SUB", result);


    return 0;
}


static void test_exit(void)
{
    printk(KERN_ALERT "test exit\n");
}

module_init(test_init);
module_exit(test_exit);

/*  描述信息  */
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Zheng Qiang");
MODULE_DESCRIPTION("A module for testing module param and EXPORT_SYMBOL");
MODULE_VERSION("V1.0");
