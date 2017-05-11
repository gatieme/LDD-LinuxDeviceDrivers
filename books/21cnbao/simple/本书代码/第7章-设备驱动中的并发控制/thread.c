#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>//wake_up_process()
#include <linux/kthread.h>//kthread_create()¡¢kthread_run()
#include <linux/version.h>
//#include <err.h>

static struct task_struct * _task1;
static struct task_struct * _task2;
static struct task_struct * _task3;

static int thread_func(void *data)
{
        int j,k;
        int timeout = HZ;
        wait_queue_head_t timeout_wq;
		char * data1=(char*)data;
        static int i = 0;
		i++;
        j = 0;
        k = i;
        printk("thread_func %d started\n", i);
   		printk("data=%s\n", data1);
        init_waitqueue_head(&timeout_wq);
        while(!kthread_should_stop())
        {
/*
 * http://stackoverflow.com/questions/30017344/moxa-realtty-module-compilation-error-on-linux-kernel-3-16
 *
 * Function interruptible_sleep_on_timeout has been removed in kernel version 3.15.
 *
 * The workaround for the missing function can be found in many patches,
 *
 * for example [this lm-sensors patch](http://lists.lm-sensors.org/pipermail/lm-sensors/2005-February/010415.html)

 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0)
                /* http://www.codeweblog.com/interruptible_sleep_on_timeout-%E7%9D%A1%E7%9C%A0%E8%B6%85%E6%97%B6%E8%AE%BE%E5%AE%9A  */
                timeout = interruptible_sleep_on_timeout(&timeout_wq, HZ);
#else
                DEFINE_WAIT(wait);
                prepare_to_wait(&timeout_wq, &wait, TASK_INTERRUPTIBLE);
                timeout = schedule_timeout(timeout);
                finish_wait(&timeout_wq, &wait);
#endif
                printk("[%d]sleeping..%d\n", k, j++);
        }
        return 0;
}

static __init int my_start_thread(void)
{

        //_task = kthread_create(thread_func, NULL, "thread_func2");
        //wake_up_process(_task);
        _task1 = kthread_run(thread_func, NULL, "thread_func1");
        _task2 = kthread_run(thread_func, NULL, "thread_func2");
        _task3 = kthread_run(thread_func, NULL, "thread_func3");
        if (!IS_ERR(_task1))
        {
                printk("kthread_create done\n");
        }
        else
        {
                printk("kthread_create error\n");
        }

        return 0;
}

static __exit void my_end_thread(void)
{
        int ret = 0;
        ret = kthread_stop(_task1);
        printk("end thread. ret = %d\n" , ret);
        ret = kthread_stop(_task2);
        printk("end thread. ret = %d\n" , ret);
        ret = kthread_stop(_task3);
        printk("end thread. ret = %d\n" , ret);
}
module_init(my_start_thread);
module_exit(my_end_thread);

