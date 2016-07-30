
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/version.h>

static void my_print(char *str)
{
	struct tty_struct *my_tty;
	my_tty = current->signal->tty;
	if (my_tty != NULL) 
	{
		((my_tty->driver)->ops->write) (my_tty,str,	strlen(str));
		((my_tty->driver)->ops->write) (my_tty,"......\n",	7);
	}
}

static int __init my_print_init(void)
{
	my_print("my_print_init!");
	return 0;
}

static void __exit my_print_exit(void)
{
	my_print("my_print_exit!");
}

module_init(my_print_init);
module_exit(my_print_exit);
MODULE_LICENSE("GPL");