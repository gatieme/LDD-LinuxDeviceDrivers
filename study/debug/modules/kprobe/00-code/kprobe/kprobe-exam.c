/* kprobe-exam.c */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>
#include <linux/time.h>



static struct kprobe kp;
static struct timeval start, end;
static int schedule_counter = 0;

int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	printk("current task on CPU#%d: %s (before scheduling), preempt_count = %d\n", smp_processor_id(), current->comm, preempt_count());
	schedule_counter++;
	return 0;
}

void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
	printk("current task on CPU#%d: %s (after scheduling), preempt_count = %d\n", smp_processor_id(), current->comm, preempt_count());
}

int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
	printk("A fault happened during probing.\n");
	return 0;
}



int init_module(void)
{
	int ret;

	kp.pre_handler = handler_pre;
	kp.post_handler = handler_post;
	kp.fault_handler = handler_fault;
	kp.addr = (kprobe_opcode_t*) kallsyms_lookup_name("schedule");

	if (kp.addr == NULL)
    {
		printk("Couldn't get the address of schedule.\n");
		return -1;
	}

	if ((ret = register_kprobe(&kp) < 0))
    {
		printk("register_kprobe failed, returned %d\n", ret);
		return -1;
	}

	do_gettimeofday(&start);

	printk("kprobe registered\n");
	return 0;
}




void cleanup_module(void)
{
	unregister_kprobe(&kp);
	do_gettimeofday(&end);
	printk("Scheduling times is %d during of %ld milliseconds.\n", schedule_counter, ((end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec))/1000);
	printk("kprobe unregistered\n");
}

MODULE_LICENSE("GPL");
