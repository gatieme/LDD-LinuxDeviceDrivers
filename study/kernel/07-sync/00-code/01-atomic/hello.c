#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/atomic.h>
//#include <linux/list.h>
//#include <linux/mm.h>
//#include <linux/mm_types.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Gatieme");
MODULE_DESCRIPTION("test atomic_t");


atomic_t count = ATOMIC_INIT(1);


static int hello_init(void)
{
    printk("init : %d\n", atomic_read(count));

    atomic_set(count, 2);
    printk("set2 : %d\n", atomic_read(count));

    atomic_add(count, 3);
    printk("add3 : %d\n", atomic_read(count));

    atomic_sub(count);
    printk("sub3 : %d\n", atomic_read(count));


    atomic_inc(count);
    printk("inc  : %d\n", atomic_read(count));

    atomic_dec(count);
    printk("dev  : %d\n", atomic_read(count));

    return 0;
}



static void hello_exit(void)
{
    printk(KERN_ERR"exit");
}


module_init(hello_init);
module_exit(hello_exit);
