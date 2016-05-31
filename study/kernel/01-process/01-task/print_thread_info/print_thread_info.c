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

static void print_thread_info_list(void)
{
	struct task_struct *ptask;
	struct pid *k;
	struct vm_area_struct *tmp;

    /* find tak by pid */
	k = find_vpid(pid);
	ptask = pid_task(k, PIDTYPE_PID);

	printk("process:%s,pid:%d\n",p->comm,p->pid);

}


struct thread_info* get_threaf_info_by_task(struct task_struct *ptask)
{
    struct thread_info   *threadinfo = NULL;

    /* for the struct task_struct *task is the member of the struct thread_info
     * we can find the threadinfo by task use container_of  */
    threadinfo = container_of(ptask, struct thread_info, task);

    /* for the struct task_struct *task is the member of the struct thread_info
     * we can find the threadinfo or stack by
     */
    threadinfo = (struct thread_info *)ptask;


    return threadinfo;
}


void print_thread_info(struct thread_info *pthreadinfo)
{
    printk(KERN_INFO "THREAD_SIZE : %d\n", THREAD_SZIE)
}


static void __exit print_thread_info_exit(void)
{
   printk(KERN_INFO "exit ---------------------!\n");
}

module_init(print_thread_info_init);
module_exit(print_thread_info_exit);
MODULE_LICENSE("GPL");
