#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/list.h>

#ifndef THREAD_SIZE
#define THREAD_SIZE 0x00002000
#endif

#define my_current my_get_current
#define my_get_current   (my_current_thread_info()->task)

register unsigned long my_current_stack_pointer asm("esp") __used;

 static inline struct thread_info *my_current_thread_info(void)
{
         return (struct thread_info *)
                 (my_current_stack_pointer & ~(THREAD_SIZE - 1));
}

static int __init lkp_init(void)
{
    printk("pid  = %d\n", current->pid);
    printk("name = %s\n", current->comm);
    printk("pid  = %d\n", my_current->pid);
    printk("name = %s\n", my_current->comm);
    printk("esp   stack : %p\n", my_current_stack_pointer);
    printk("thread_info : %p\n", my_current_thread_info);
    printk("stack       : %p\n", my_current->stack);

    return 0;
}

static void __exit lkp_cleanup(void)
{
    printk("<1>bye!\n");
}

module_init(lkp_init);
module_exit(lkp_cleanup);
MODULE_LICENSE("GPL");
