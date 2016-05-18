#include <linux/module.h>


MODULE_LICENSE("Dual BSD/GPL");


static int hello_init(void)
{
    unsigned int cpu = get_cpu();
    struct module *mod;

    printk(KERN_ALERT "this module: %p==%p\n", &__this_module, THIS_MODULE);
    printk(KERN_ALERT "module state: %d\n", THIS_MODULE->state);
    printk(KERN_ALERT "module name: %s\n", THIS_MODULE->name);

    list_for_each_entry(mod, *(&THIS_MODULE->list.prev), list);
    printk(KERN_ALERT "module name: %s\n", mod->name);

    return 0;
}


static void hello_exit(void)
{
    printk(KERN_ALERT "module state: %d\n", THIS_MODULE->state);
}


module_init(hello_init);
module_exit(hello_exit);
