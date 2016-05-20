/*************************************************************************
    > File Name: process.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: 2016年04月01日 星期五 21时09分29秒
 ************************************************************************/



#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/pid.h>



/*===================================================*/
int kthread_function(void *argc)
{
    printk(KERN_ALERT "in the kernel thread function!\n");
    return 0;
}


void print_kernel_thread_pid(void)
{
    pid_t               res = -1;
    struct pid          *ktpid = NULL;
    struct task_struct  *task = NULL;

    //  create a kernel thread
    res = kernel_thread(kthread_function, NULL, /* CLONE_KERNEL | */ SIGCHLD);

    //  get the pid of the kernel thread you create
    ktpid = find_get_pid(res);

    //  get the task info of the kernel thread
    task = pid_task(ktpid, PIDTYPE_PID);

    printk(KERN_ALERT "the state of the task is : %ld\n", task->state);      //显示任务当前所处的状态

    //  get the real PID of the kernel thread
    printk(KERN_ALERT "the pid of the task is  :%d\n", task->pid);        //显示任务的进程号

    //  get the thread id of the kernel thread
    printk(KERN_ALERT "the tgid of the task is : %d\n", task->tgid);

    //  显示函数kernel_thread( )函数执行结果
    printk(KERN_ALERT "the kernel_thread result is : %d\n", res);
    printk(KERN_ALERT "out pid_task_init.\n");

}


/*===================================================*/
static int noop(void *dummy)
{
    int i = 0;
    //daemonize("mythread");

    while(i++ < 5)
    {
        printk("current->mm = %p/n", current->mm);
        printk("current->active_mm = %p/n", current->active_mm);
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(10 * HZ);
    }

    return 0;
}

static int init_kernel_thread(void)
{
    print_kernel_thread_pid( );

    kernel_thread(noop, NULL, /*CLONE_KERNEL| */SIGCHLD);

    return 0;
}

static void exit_kernel_thread(void)
{
    printk(KERN_ALERT "GOOD BYE:kernel_thread!!\n");
    do_exit(0);
}



module_init(init_kernel_thread);
module_exit(exit_kernel_thread);

MODULE_AUTHOR("gatieme");
MODULE_LICENSE("GPL");
