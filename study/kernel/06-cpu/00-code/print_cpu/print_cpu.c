#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
//#include <linux/list.h>
//#include <linux/mm.h>
//#include <linux/mm_types.h>

//  http://blog.csdn.net/guowenyan001/article/details/44224759
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Gatieme");
MODULE_DESCRIPTION("hello world");

static int __init init_print_cpu(void)
{
        int i;

        printk("init marker.\n");

        for_each_possible_cpu(i)
        {
                printk("possible cpu : %d\n", i);
        }

        for_each_online_cpu(i)
        {
                printk("online cpu : %d\n", i);
        }

        for_each_present_cpu(i)
        {
                printk("present cpu : %d\n", i);
        }
        return 0;
}

static void __exit exit_print_cpu(void)
{
        printk("exit marker.\n");
}



module_init(init_print_cpu);
module_exit(exit_print_cpu);
