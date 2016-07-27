/*****************************************************************
文件名：mem.c
输入参数：
pid 接收待查询进程的PID
va 接收待查询的虚拟地址
*****************************************************************/


#include  <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <asm/pgtable.h>
#include <asm/page.h>

MODULE_LICENSE("GPL");


static unsigned long VA;
module_param(VA, ulong, 0644);



static int __init kv2p_init( )
{
    // use virt_to_phys
    // http://lxr.free-electrons.com/source/arch/arm64/include/asm/memory.h?v4.7#L189
    //
    virt_to_phys(VA);


    return 0;
}



static void __exit kv2p_exit(void)
{
    printk(KERN_INFO"Goodbye!\n");
}

module_init(kv2p_init);
module_exit(kv2p_exit);
