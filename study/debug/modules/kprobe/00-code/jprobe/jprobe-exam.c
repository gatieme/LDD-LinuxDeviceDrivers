/* jprobe-exam.c */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

static struct jprobe jp;

asmlinkage long jprobe_sys_open(const char __user *filename, int flags, int mode)
{
	int len = PATH_MAX;
	char * tmpfilename = NULL;

	if (TASK_SIZE - (unsigned long) filename < PATH_MAX) {
		len = TASK_SIZE - (unsigned long) filename;
	}

	tmpfilename = kmalloc(len, GFP_ATOMIC);
	if (tmpfilename == NULL) return 0;

	if (copy_from_user(tmpfilename, filename, len)) return 0;

	printk("process '%s' call open('%s', %d, %d)\n", current->comm, tmpfilename, flags, mode);
	jprobe_return();
	return 0;
}


int init_module(void)
{
	int ret;

	jp.entry = (kprobe_opcode_t *) jprobe_sys_open;
	jp.kp.addr = (kprobe_opcode_t *)kallsyms_lookup_name("sys_open");
	if (!jp.kp.addr) {
		printk("Couldn't find the address of sys_open\n");
		return -1;
	}

	if ((ret = register_jprobe(&jp)) <0) {
		printk("register_jprobe failed, returned %d\n", ret);
		return -1;
	}
	printk("Registered a jprobe.\n");
	return 0;
}

void cleanup_module(void)
{
	unregister_jprobe(&jp);
	printk("jprobe unregistered\n");
}

MODULE_LICENSE("GPL");
