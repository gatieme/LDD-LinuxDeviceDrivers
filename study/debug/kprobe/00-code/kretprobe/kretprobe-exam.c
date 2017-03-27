/*kretprobe-exam.c*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>

static struct kretprobe kretp;

static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	// Substitute the appropriate register name for your architecture --
	// e.g., regs->rax for x86_64, regs->gpr[3] for ppc64.
	int retval = (int) regs->ax;
	if (retval < 0) {
		printk("sys_open returns %d\n", retval);
	}
	return 0;
}

int init_module(void)
{
	int ret;

	kretp.kp.addr = (kprobe_opcode_t *) kallsyms_lookup_name("sys_open");
	if (!kretp.kp.addr) {
		printk("Couldn't find sys_open.\n");
		return -1;
	}

	kretp.handler = ret_handler,
	kretp.maxactive = 1;

	if ((ret = register_kretprobe(&kretp)) < 0) {
		printk("register_kretprobe failed, returned %d\n", ret);
		return -1;
	}
	printk("Registered a return probe.\n");
	return 0;
}

void cleanup_module(void)
{
	unregister_kretprobe(&kretp);
	printk("kretprobe unregistered\n");
	printk("Missed %d sys_open probe instances.\n", kretp.nmissed);
}

MODULE_LICENSE("GPL");
