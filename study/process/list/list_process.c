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
//#include <asm/current.h>



//#define METHOD 2
static unsigned int METHOD = 1;
module_param(METHOD, uint,0400);

static int list_process_init(void)
{
    struct task_struct *task = NULL, *pTask = NULL;
    struct list_head *pos = 0;
    int count = 0;
    char *method = NULL;


    count = 0;
    task = &init_task;
    printk(KERN_ALERT"PID\tCOMM\n");


    switch(METHOD)
    {
        case 1 :
            method = "list_for_each";
            list_for_each(pos, &task->tasks)
            {
                pTask = list_entry(pos, struct task_struct, tasks);
                count++;
                printk(KERN_ALERT "%d\t%s\n", pTask->pid, pTask->comm);
            }

            //  because the list of process is an double-linked circular list
            //  we can also list all the process from the current process
            list_for_each(pos, &current->children)
            {
                pTask = list_entry(pos, struct task_struct, sibling);
                printk(KERN_ALERT "%d\t%s\n", pTask->pid, pTask->comm);
            }
            break;
        case 2 :
            method = "for_each_process";
            for_each_process(pTask)
            {
                count++;
                printk(KERN_ALERT "%d\t%s\n", pTask->pid, pTask->comm);
            }
            break;
        case 3 :

            method = "list_for_each_entry";
            list_for_each_entry(pTask, &task->tasks, tasks)
            {
                count++;
                printk(KERN_ALERT "%d\t%s\n", pTask->pid, pTask->comm);
            }
            break;
    }

    printk(KERN_ALERT "The method is %s\n", method);
    printk(KERN_ALERT "there are %d process in your system now...\n", count);
    printk(KERN_ALERT "current : %d\t%s\n", current->pid, current->comm);
    return 0;
}

static void list_process_exit(void)
{
    printk(KERN_ALERT "GOOD BYE!!\n");
}



module_init(list_process_init);
module_exit(list_process_exit);

MODULE_AUTHOR("gatieme");
MODULE_LICENSE("GPL");
