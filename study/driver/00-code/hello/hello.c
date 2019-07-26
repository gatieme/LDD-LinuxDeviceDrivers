#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
//#include <linux/list.h>
//#include <linux/mm.h>
//#include <linux/mm_types.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Gatieme");
MODULE_DESCRIPTION("hello world");

static int hello_init(void)
{
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule();

	return 0;
}



static void hello_exit(void)
{
}


module_init(hello_init);
module_exit(hello_exit);
