#include <linux/init.h>
#include <linux/module.h>

#include <linux/kthread.h>

#include <linux/delay.h>  // error: implicit declaration of function ‘msleep’


static struct task_struct *ptask;

static int thread_function(void *data)
{
    int time_count = 0;
    do
    {
        printk(KERN_INFO "thread_function: %d times", ++time_count);
        msleep(1000);
    }while(!kthread_should_stop()
        && time_count<=30);

    return time_count;
}

static int test_kthread_run_init(void)
{
    printk(KERN_INFO "test_kthread_run, world!\n");

    ptask = kthread_run(thread_function, NULL, "mythread%d", 1);

    if (IS_ERR(ptask))
    {
        printk(KERN_INFO "create kthread failed!\n");
    }
    else
    {
        printk(KERN_INFO "create ktrhead ok!\n");
    }

    return 0;
}

static void test_kthread_run_exit(void)
{
    printk(KERN_INFO "test_kthread_run, exit!\n");
 
    if (!IS_ERR(ptask))
    {
        int ret = kthread_stop(ptask);
        printk(KERN_INFO "thread function has run %ds\n", ret);
    }
}

module_exit(test_kthread_run_exit);
module_init(test_kthread_run_init);
MODULE_LICENSE("Dual BSD/GPL");

