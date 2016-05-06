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



//#define METHOD 2
static unsigned int METHOD = 1;
module_param(METHOD, uint,0400);

static int list_process_init(void)
{
    struct task_struct *task, *p;
    struct list_head *pos;
    int count;
    char *method;
    count = 0; /*下面这些初始化完全是为了消除编译时的警告信息*/
    p = 0;
    task = 0;
    pos = 0;
    method = 0;
    task = &init_task;

    printk(KERN_ALERT"PID\tCOMM\n");


    switch(METHOD)
    {
        case 1 :
            method = "list_for_each";
            list_for_each( pos, &task->tasks )
            {
                p = list_entry( pos, struct task_struct, tasks );
                count++;
                printk( KERN_ALERT "%d\t%s\n", p->pid, p->comm );
            }
            break;
        case 2 :
            method = "for_each_process";
            for_each_process(task)
            {
                count++;
                printk( KERN_ALERT "%d\t%s\n", task->pid, task->comm );
            }
            break;
        case 3 :

            method = "list_for_each_entry";
            list_for_each_entry( p, &task->tasks, tasks )
            {
                count++;
                printk( KERN_ALERT "%d\t%s\n", p->pid, p->comm );
            }
    }

    printk( "The method is %s\n", method );
    printk("there are %d process in your system now...", count);

    return 0;
}

static void list_process_exit(void)
{
    printk( KERN_ALERT "GOOD BYE!!\n");
}



module_init(list_process_init);
module_exit(list_process_exit);

MODULE_AUTHOR("gatieme");
MODULE_LICENSE("GPL");
