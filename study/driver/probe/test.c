#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

/*
 * Jumper probe for do_fork.
 * Mirror principle enables access to arguments of the probed routine
 * from the probe handler.
 */

/* Proxy routine having the same arguments as actual do_fork() routine */
static long jdo_fork(unsigned long clone_flags, unsigned long stack_start,
          struct pt_regs *regs, unsigned long stack_size,
          int __user *parent_tidptr, int __user *child_tidptr)
{
    printk(KERN_INFO "jprobe: clone_flags = 0x%lx, stack_size = 0x%lx,"
            " regs = 0x%p\n",
           clone_flags, stack_size, regs);

    /* Always end with a call to jprobe_return(). */
    jprobe_return();
    return 0;
}


asmlinkage __visible void __sched jschedule(void)
{

    /* Always end with a call to jprobe_return(). */
    jprobe_return();
    return 0;
}

static struct jprobe my_jprobe = {
    .entry            = jschedule,
    /*  注意 symbol_name与addr不能同时存在
     *  参见kprobe_addr函数--http://lxr.free-electrons.com/source/kernel/kprobes.c#L1359
     *  否则会提示参数错误ERR_PTR(-EINVAL);
     *  内核中errno的值在http://lxr.free-electrons.com/source/include/uapi/asm-generic/errno-base.h#L25
     *  */
    .kp = {
        .symbol_name    = "schedule",
        //.addr
    },
};

static int __init jprobe_init(void)
{
    int ret;

    //my_jprobe.kp.symbol_name = "_do_fork";
    //my_jprobe.kp.addr = kallsyms_lookup_name("_do_fork");

    if(my_jprobe.kp.symbol_name == NULL)
    {
        my_jprobe.kp.addr = (kprobe_opcode_t *)kallsyms_lookup_name("_do_fork");
        printk("find _do_fork address at 0x%p\n", my_jprobe.kp.addr);
    }

    ret = register_jprobe(&my_jprobe);
    if (ret < 0) {
        printk(KERN_INFO "register_jprobe failed, returned %d\n", ret);
        return -1;
    }
    printk(KERN_INFO "Planted jprobe at %p, handler addr %p\n",
           my_jprobe.kp.addr, my_jprobe.entry);
    return 0;
}

static void __exit jprobe_exit(void)
{
    unregister_jprobe(&my_jprobe);
    printk(KERN_INFO "jprobe at %p unregistered\n", my_jprobe.kp.addr);
}

module_init(jprobe_init)
module_exit(jprobe_exit)
MODULE_LICENSE("GPL");
