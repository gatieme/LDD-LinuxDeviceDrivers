#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>


#include <linux/sched.h>
#include <linux/sched/rt.h>



#if !defined(NICE_TO_PRIO)
#define NICE_TO_PRIO(nice)      ((nice) + DEFAULT_PRIO)
#endif

#if !defined(PRIO_TO_NICE)
#define PRIO_TO_NICE(prio)      ((prio) - DEFAULT_PRIO)
#endif

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


static inline int print_task_policy(int policy)
{
    printk("POLICY = ");
    switch(policy)
    {
        case SCHED_FIFO     :
            printk("Real-Time FIFO SCHEDULER TASK\n");
            break;
        case SCHED_RR       :
            printk("Real-Time RR SCHEDULER TASK\n");
            break;
        case SCHED_DEADLINE :
            printk("Real-Time EDF SCHEDULER TASK\n");
            break;
        case SCHED_NORMAL   :
            printk("Normal NORMAL SCHEDULER TASK\n");
            break;
        case SCHED_BATCH    :
            printk("Normal BATCH SCHEDULER TASK\n");
            break;
        case SCHED_IDLE     :
            printk("IDLE SCHEDULER TASK\n");
            break;
    }


    return policy;
}



void print_task_sched_class(struct sched_class  *schedclass)
{
    switch(sched_class)
    {
        case &rt_
    }
}

void print_task_priority(struct task_struct *ptask)
{
    //  priority
    if(rt_task(ptask) == 1)
    {
        printk("Real Time Process\n");
        printk("rt_priority = %d\n", ptask->rt_priority);
    }
    else
    {
        printk("Normal Process\n");
        printk("static prio = %d, NICE(%d)\n", ptask->static_prio, PRIO_TO_NICE(ptask->static_prio));
    }
    printk("normal_prio = %d\n", ptask->normal_prio);
    printk("prio = %d\n", ptask->prio);
}




void print_task_struct(struct task_struct *ptask)
{
    printk("flag = 0x%x\n", ptask->flags);
    //  priority
    print_task_priority(ptask);

    //  policy
    print_task_policy(ptask->policy);
}



static void print_task(int pid)
{
	struct task_struct *ptask;
	struct pid *k;

	k = find_vpid(pid);
	ptask = pid_task(k, PIDTYPE_PID);

    print_task_struct(ptask);


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
