#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>


static int demo_module_init(void)
{
	printk("demo_module_init\n");
	return 0;
}

static void demo_module_exit(void)
{
	printk("demo_module_exit\n");
}
module_init(demo_module_init);
module_exit(demo_module_exit);

MODULE_DESCRIPTION("simple module");
MODULE_LICENSE("GPL");

