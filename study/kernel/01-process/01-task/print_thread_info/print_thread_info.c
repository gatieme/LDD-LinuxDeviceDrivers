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


#ifndef task_thread_info
#define task_thread_info(task)  ((struct thread_info *)(task)->stack)
#endif


#ifndef assert
#define assert(expr)                                                \
    do {                                                            \
        if (unlikely(!(expr))) {                                    \
            printk(KERN_ERR "Assertion failed! %s,%s,%s,line=%d\n", \
                   #expr, __FILE__, __func__, __LINE__);            \
        }                                                           \
    } while (0)
#endif

static int PID = 1;
module_param(PID, int, 0644);


struct thread_info* get_thread_info(struct task_struct *ptask)
{
    struct thread_info   *threadinfo = NULL;

    /* for the struct task_struct *task is the member of the struct thread_info
     * we can find the threadinfo by task use container_of

       error : because the type of member thread_info->task is 'struct task_struct *'
               so we need
       threadinfo = NULL;
       threadinfo = container_of(ptask, struct thread_info, task);
       printk(KERN_INFO "thread_info           : %p\n", threadinfo);
    */
    threadinfo = task_thread_info(ptask);
    printk(KERN_INFO "thread_info           : %p\n", threadinfo);
    return threadinfo;
}

union thread_union* get_thread_union(struct thread_info *threadinfo)
{
    /* for the struct thread_info is the first member of the struct thread_info
     * we can find the threadinfo or stack by thread_info
     */
    union thread_union *threadunion_info = NULL;
    union thread_union *threadunion_cont = NULL;

    threadunion_info = (union thread_union *)threadinfo;
    threadunion_cont = container_of(threadinfo, union thread_union, thread_info);

    printk(KERN_INFO "thread_union address  : %p == %p\n",
            (void *)threadunion_info, (void *)threadunion_cont);

    assert(threadunion_info == threadunion_cont);
    return threadunion_info;
}


unsigned long show_kstack(union thread_union *threadunion)
{
    unsigned long kstack_thread = (unsigned long)threadunion->stack;
    unsigned long kstack_task   = (unsigned long)threadunion->thread_info.task->stack;

    printk(KERN_INFO "THREAD_SIZE           : %ld == %ld == %ldKB",
           sizeof(union thread_union), THREAD_SIZE, THREAD_SIZE / 1024);
    //printk(KERN_INFO "THREAD_MASK           : 0x%x\n", ~(THREAD_SIZE - 1));

    printk(KERN_INFO "union_stack           : %p == %p\n",
           (void *)kstack_thread, (void *)kstack_task);

    printk(KERN_INFO "kernel stack          : [%p, %p]\n",
           (void *)kstack_thread + sizeof(struct thread_info),
           (void *)(kstack_task) + sizeof(union thread_union));

    assert(kstack_thread == kstack_task);
    return kstack_thread;
}

static void print_thread_info(int pid)
{
	struct task_struct *ptask;
	struct pid *k;
    struct thread_info *threadinfo = NULL;
    union thread_union *threadunion = NULL;
    /* find tak by pid */
	k = find_vpid(pid);
	ptask = pid_task(k, PIDTYPE_PID);

	printk(KERN_INFO "process   : %s, pid   : %d\n", ptask->comm, ptask->pid);

    /*  ptask->stack point to thread_unuion  */
    threadinfo = get_thread_info(ptask);

    /* union thread_union
     * {
     *     struct thread_info thread_info;
     *     unsigned long stack[THREAD_SIZE/sizeof(long)];
     * };
     */
    threadunion = get_thread_union(threadinfo);

    show_kstack(threadunion);

    //printk(KERN_INFO "task stack            : %p\n", ptask->stack);
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
