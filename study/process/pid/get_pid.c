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
#include <linux/pid.h>



//#define METHOD 2
static unsigned int METHOD = 1;
module_param(METHOD, uint,0400);

static unsigned int PID = 1;
module_param(PID, uint,0400);


int kthread_function(void *argc)
{
    printk(KERN_ALERT "in the kernel thread function!\n");
    return 0;
}

void do_print_pid()
{
    pid_t res;

    //  create a kernel thread
    res = kernel_thread(kthread_function, NULL, CLONE_KERNEL);

    //  get the pid of the kernel thread you create
    struct pid* ktpid = find_get_pid(res);

    //  get the task info of the kernel thread
    struct task_struct* task = pid_task(kpid, PIDTYPE_PID);

    printk(KERN_ALERT "the state of the task is:%d\n", task->state);      //显示任务当前所处的状态

    //  get the real PID of the kernel thread
    printk(KERN_ALERT "the pid of the task is:%d\n", task->pid);        //显示任务的进程号

    //  get the thread id of the kernel thread
    printk(KERN_ALERT "the tgid of the task is:%d\n", task->tgid);

    //  显示函数kernel_thread( )函数执行结果
    printk(KERN_ALERT "the kernel_thread result is:%d\n", res);
    printk(KERN_ALERT "out pid_task_init.\n");

    return 0;
}


static int init_print_pid(void)
{

}

static void exit_print_pid(void)
{
    printk(KERN_ALERT "GOOD BYE:print_pid!!\n");
}



module_init(init_print_pidt);
module_exit(exit_print_pid);

MODULE_AUTHOR("gatieme");
MODULE_LICENSE("GPL");
