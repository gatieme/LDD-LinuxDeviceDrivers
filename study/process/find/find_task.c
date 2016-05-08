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


/*
*	find_task_by_pid maybe not supported
*	O(n) is fine :)
*/
struct task_struct * findTaskByPid(pid_t pid)
{
	struct task_struct *task = NULL;

	return NULL;
}

/*
 * show the task info
 */
void getTaskinfo(struct task_struct *task)
{
}

//#define METHOD 2
static unsigned int METHOD = 1;
module_param(METHOD, uint,0400);

static unsigned int PID = 1;
module_param(PID, uint,0400);
static int find_task_init(void)
{
    struct task_struct *task = NULL, *pTask = NULL;
    struct list_head *pos = NULL;
    char *method = NULL;

    task = &init_task;

    printk(KERN_ALERT"PID\tCOMM\tADDR\n");



    switch(METHOD)
    {
        case 1 :
            method = "list_for_each";
            list_for_each(pos, &task->tasks)
            {
                pTask = list_entry(pos, struct task_struct, tasks );
                if(pTask->pid == (pid_t)PID)
                {
                    printk(KERN_ALERT "%d\t%s\t%p\n", pTask->pid, pTask->comm, pTask);
                    break;
                }
            }

            break;
        case 2 :
            method = "for_each_process";
            for_each_process(task)
            {
                if(task->pid == PID)
                {
                    printk(KERN_ALERT "%d\t%s\t%p\n", task->pid, task->comm, task);
                    break;
                }
            }
            break;
        case 3 :
            method = "list_for_each_entry";
            list_for_each_entry(pTask, &task->tasks, tasks)
            {
                if(pTask->pid == PID)
                {
                    printk(KERN_ALERT "%d\t%s\t%p\n", pTask->pid, pTask->comm, pTask);
                }
            }
    }
    printk("the method is %s", method);
    return 0;
}

static void find_task_exit(void)
{
    printk(KERN_ALERT "GOOD BYE:find_task!!\n");
}



module_init(find_task_init);
module_exit(find_task_exit);

MODULE_AUTHOR("gatieme");
MODULE_LICENSE("GPL");
