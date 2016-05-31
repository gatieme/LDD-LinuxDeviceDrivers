#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/init.h>
static struct task_struct *test_task = NULL;

int kthread_function(void *argc)
{
    printk(KERN_ALERT "in the kernel thread function!\n");
    printk(KERN_ALERT "pid = %d, ppid = %d", current->pid, current->ppid);

    return 0;
}

static int init_test_kthread_modulee(void)
{

    int err;

    test_task = kthread_create(kthread_function, NULL, "test_task");

    if(IS_ERR(test_task))
    {

      printk("Unable to start kernel thread./n");

      err = PTR_ERR(test_task);

      test_task = NULL;

      return err;
    }

    wake_up_process(test_task);

    return 0;

}

static void exit_test_kthread_create(void)
{
    printk(KERN_ALERT "GOOD BYE:kernel_thread!!\n");
}

module_init(init_test_kthread_modulee);
module_exit(exit_test_kthread_create);
