#include <linux/sched.h>//wake_up_process()
#include <linux/kthread.h>//kthread_create()¡¢kthread_run()
#include <err.h>

static struct task_struct * _task;
static struct task_struct * _task2;
static struct task_struct * _task3;
static int thread_func(void *data)
{
        int j,k;
        int timeout;
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
                interruptible_sleep_on_timeout(&timeout_wq, HZ);
                printk("[%d]sleeping..%d\n", k, j++);
        }
        return 0;
}   
void my_start_thread(void)
{
           
        //_task = kthread_create(thread_func, NULL, "thread_func2");
        //wake_up_process(_task);
        _task = kthread_run(thread_func, NULL, "thread_func1");
        _task2 = kthread_run(thread_func, NULL, "thread_func2");
        _task3 = kthread_run(thread_func, NULL, "thread_func3");
        if (!IS_ERR(_task))
        {
                printk("kthread_create done\n");
        }
        else
        {
                printk("kthread_create error\n");
        }
}
void my_end_thread(void)
{
        int ret = 0;
        ret = kthread_stop(_task);
        printk("end thread. ret = %d\n" , ret);
        ret = kthread_stop(_task2);
        printk("end thread. ret = %d\n" , ret);
        ret = kthread_stop(_task3);
        printk("end thread. ret = %d\n" , ret);
}
module_init(my_start_thread);
module_exit(my_end_thread);

