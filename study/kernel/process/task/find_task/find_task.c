/*************************************************************************
    > File Name: process.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: 2016年04月01日 星期五 21时09分29秒
 ************************************************************************/


#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/sched.h>

#include "get_task.h"

/*
 *  macro for find_task_by_pid in the process list
 *  LIST_FOR_EACH       to traversing the process linked list by `list_for_each`
 *  FOR_EACH_PROCESS    to traversing the process linked list by `for_each_process`
 *  LIST_FOR_EACH_ENTRY to traversing the process linked list by `list_for_each_entry`
 *  LIST_ALL            to  traversing the process linked list use all above macro or function
 */
#define LIST_ALL                1
#define LIST_FOR_EACH           0
#define FOR_EACH_PROCESS        0
#define LIST_FOR_EACH_ENTRY     0



//#define METHOD 2
static unsigned int METHOD = 1;
module_param(METHOD, uint, 0400);

static unsigned int PID = 1;
module_param(PID, uint, 0400);



void print_vm_list(struct task_struct *task);


void getTaskinfo(struct task_struct *task)
{

}

struct task_struct* find_task_by_pid_in_rbtree(int pid)
{
    return NULL;
}


struct task_struct* find_task_by_pid_in_ns(int pid)
{
    struct task_struct *pTask = NULL;

    printk(KERN_ALERT "pid_task");

    pTask = pid_task(find_vpid(pid), PIDTYPE_PID);

    if(pTask != NULL)
    {
        printk(KERN_ALERT "%d\t%s\t%p\n", pTask->pid, pTask->comm, (void *)pTask);
    }

    return pTask;
}


struct task_struct* find_task_by_pid_in_list(int pid)
{
    struct task_struct *task = NULL, *pTask = NULL;
    struct list_head *pos = NULL;

    task = &init_task;


#if LIST_ALL || LIST_FOR_EACH
    pTask = NULL;
    task = &init_task;
    pos = NULL;

    printk(KERN_ALERT "list_for_each");

    list_for_each(pos, &task->tasks)
    {
        pTask = list_entry(pos, struct task_struct, tasks );
        if(pTask->pid == (pid_t)pid)
        {
            printk(KERN_ALERT"PID\tCOMM\tADDR\n");
            printk(KERN_ALERT "%d\t%s\t%p\n", pTask->pid, pTask->comm, pTask);
            break;
        }
    }
#endif

#if LIST_ALL || FOR_EACH_PROCESS
    pTask = NULL;
    task = &init_task;

    printk(KERN_ALERT "for_each_process");

    for_each_process(task)
    {
        if(task->pid == pid)
        {
            printk(KERN_ALERT"PID\tCOMM\tADDR\n");
            printk(KERN_ALERT "%d\t%s\t%p\n", task->pid, task->comm, task);
            break;
        }
    }
#endif

#if LIST_ALL || LIST_FOR_EACH_ENTRY
    pTask = NULL;
    task = &init_task;

    printk(KERN_ALERT "list_for_each_entry");

    list_for_each_entry(pTask, &task->tasks, tasks)
    {
        if(pTask->pid == (pid_t)pid)
        {
            printk(KERN_ALERT"PID\tCOMM\tADDR\n");
            printk(KERN_ALERT "%d\t%s\t%p\n", pTask->pid, pTask->comm, pTask);
            break;
        }
    }
#endif

    return pTask;
}

static int init_find_task(void)
{
    struct task_struct  *pTask = NULL;

    switch(METHOD)
    {
        case 1 :
        {
            pTask = find_task_by_pid_in_ns(PID);
            break;
        }

        case 2 :
        {
            pTask = find_task_by_pid_in_list(PID);
            break;
        }

        case 3 :
        {
            pTask = find_task_by_pid_in_rbtree(PID);
            break;
        }
    }
    print_vm_list(pTask);
    return 0;
}

static void exit_find_task(void)
{
    printk(KERN_ALERT "GOOD BYE:find_task!!\n");
}



module_init(init_find_task);
module_exit(exit_find_task);

MODULE_AUTHOR("gatieme");
MODULE_LICENSE("GPL");
