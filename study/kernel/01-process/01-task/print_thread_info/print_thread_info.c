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


static int PID = 1;

module_param(PID, int, 0644);


struct thread_info* get_thread_info(struct task_struct *ptask)
{
    struct thread_info   *threadinfo = NULL;

    /* for the struct task_struct *task is the member of the struct thread_info
     * we can find the threadinfo by task use container_of  */
    threadinfo = container_of(ptask, struct thread_info, task);
    printk(KERN_INFO "thread_info           : %p", threadinfo);


    return threadinfo;
}

union thread_union* get_thread_union(struct thread_info *threadinfo)
{
    union thread_union *threadunion = NULL;
    /* for the struct thread_info is the first member of the struct thread_info
     * we can find the threadinfo or stack by thread_info
     */
    threadunion = (union thread_union *)threadinfo;
    printk(KERN_INFO "thread_union address  : %p\n", threadunion);
    threadunion = NULL;
    threadunion = container_of(threadinfo, union thread_union, thread_info);
    printk(KERN_INFO "thread_union address  : %p\n", threadunion);

    return threadunion;
}


unsigned long show_kstack(union thread_union *threadunion)
{
    unsigned long kstack = (unsigned long)threadunion->stack;
    printk(KERN_INFO "THREAD_SIZE           : %d == %dKB", THREAD_SIZE, THREAD_SIZE / 1024);
    printk(KERN_INFO "union_stack           : %p\n", (unsigned long)kstack);
    printk(KERN_INFO "union_stack align     : %p\n", ((unsigned long)kstack & ~(THREAD_SIZE - 1)));
    printk(KERN_INFO "kstack start          : %p\n", (void *)(((unsigned long) kstack & ~(THREAD_SIZE - 1)) + THREAD_SIZE - 1));
    printk(KERN_INFO "kstack end            : %p\n", (void *)(((unsigned long) kstack & ~(THREAD_SIZE - 1)) + sizeof(struct thread_info)));
}

static void print_thread_info(int pid)
{
	struct task_struct *ptask;
	struct pid *k;
	struct vm_area_struct *tmp;
    struct thread_info *threadinfo = NULL;
    union thread_union *threadunion = NULL;
    /* find tak by pid */
	k = find_vpid(pid);
	ptask = pid_task(k, PIDTYPE_PID);

	printk(KERN_INFO "process   : %s, pid   : %d\n", ptask->comm, ptask->pid);

    threadinfo = get_thread_info(ptask);

    threadunion = get_thread_union(threadinfo);

    show_kstack(threadunion);

    printk(KERN_INFO "task stack            : %p\n", ptask->stack);
}




static int __init print_thread_info_init(void)
{
    print_thread_info(PID);

    return 0;
}

static void __exit print_thread_info_exit(void)
{
   printk(KERN_INFO "exit ---------------------!\n");
}

module_init(print_thread_info_init);
module_exit(print_thread_info_exit);
MODULE_LICENSE("GPL");
