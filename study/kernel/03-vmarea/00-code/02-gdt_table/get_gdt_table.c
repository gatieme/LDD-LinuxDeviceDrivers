#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
//#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <asm/desc.h>


// [《深入理解Linux内核》内存寻址学习心得](http://blog.chinaunix.net/uid-27776472-id-4304663.html)
// [lxr-get_cpu_gdt_table-Identifier Search](http://lxr.free-electrons.com/source/arch/x86/include/asm/desc.h#L48)
// [lxr-desc_struct-Identifier Search](http://lxr.free-electrons.com/source/arch/x86/include/asm/desc_defs.h#L22)


MODULE_LICENSE("Dual BSD/GPL");




static int hello_init(void)
{
    struct desc_struct *gdt_table = get_cpu_gdt_table(0);
    return 0;
}



static void hello_exit(void)
{
}


module_init(hello_init);
module_exit(hello_exit);
