#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Gatieme");
MODULE_DESCRIPTION("none_exit world");



static int none_exit_init(void)
{
    printk("none_exit init\n");
    return 0;
}
module_init(none_exit_init);


#if 0
static void none_exit_exit(void)
{
    printk(KERN_ERR"exit");
}
module_exit(none_exit_exit);
#endif

