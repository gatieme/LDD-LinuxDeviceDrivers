#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/list.h>

struct simplelist
{
	struct list_head node;
	char buffer;
};

LIST_HEAD(mylist);

static int demo_module_init(void)
{
	int i=0;
	printk("demo_module_init\n");
	for(i=0;i<5;i++)
	{
		struct simplelist*p=(struct simplelist *)kmalloc(sizeof(struct simplelist),GFP_KERNEL);
		p->buffer=0x31+i;
		list_add_tail(&p->node,&mylist);
	}
	struct simplelist*slistp;
	list_for_each_entry(slistp,&mylist,node){
		printk("find a list buffer is %c\n",slistp->buffer);
	}

	return 0;
}

static void demo_module_exit(void)
{
	printk("demo_module_exit\n");
}
module_init(demo_module_init);
module_exit(demo_module_exit);

MODULE_DESCRIPTION("simple list module");
MODULE_LICENSE("GPL");

