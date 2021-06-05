#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kthread.h>
#include <linux/wait_bit.h>

#define pr_fmt(fmt)	"WAIT_BIT" fmt
MODULE_LICENSE("Dual BSD/GPL");


#define WAIT_BIT	0
static unsigned long flag;


static int wait_bit_func(void *data)
{
        while (!kthread_should_stop()) {
		wait_on_bit(&flag, WAIT_BIT, TASK_INTERRUPTIBLE);

		pr_info("[%s %d]\n", __func__, __LINE__);

		set_bit(WAIT_BIT, &flag);
	}

	return 0;
}

struct task_struct *wait_task = NULL;

static __init int wait_bit_test_init(void)
{
	printk("wait_bit module init\n");

	wait_task = kthread_create(wait_bit_func, NULL, "wait_bit_thread");
	if (wait_task)
		wake_up_process(wait_task);

	clear_bit(WAIT_BIT, &flag);
	smp_mb__after_atomic();
	wake_up_bit(&flag, WAIT_BIT);

	return 0;
}

static __exit void wait_bit_test_exit(void)
{
	if (wait_task)
		kthread_stop(wait_task);

	printk(KERN_ALERT "wait_bit module exit\n");
}


module_init(wait_bit_test_init);
module_exit(wait_bit_test_exit);
