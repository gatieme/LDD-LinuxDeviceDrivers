#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>

#ifndef offsetof
#define offsetof(type, field)   ((long) &((type *)0)->field)
#endif   /* offsetof */

#ifndef container_of
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif


static int pid = 1;
module_param(pid,int,0644);

static void print_task_struct(struct task_struct *ptask)
{

}


static void print_task(int pid)
{
	struct task_struct *ptask;
	struct pid *k;

	k = find_vpid(pid);
	ptask = pid_task(k, PIDTYPE_PID);

    print_task_struct(task);


    return ;
}

static int __init print_task_init(void)
{
	printk(KERN_INFO"------------------------\n");
    print_task(pid);
	printk(KERN_INFO"------------------------\n");

    return 0;
}

static void __exit print_task_exit(void)
{
   printk(KERN_INFO"exit ---------------------!\n");
}

module_init(print_task_init);
module_exit(print_task_exit);
MODULE_LICENSE("GPL");
