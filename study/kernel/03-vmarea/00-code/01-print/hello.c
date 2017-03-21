#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
//#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>


// [《深入理解Linux内核》内存寻址学习心得](http://blog.chinaunix.net/uid-27776472-id-4304663.html)
MODULE_LICENSE("Dual BSD/GPL");



int print_bit(char *addr, int size)
{

    unsigned char *ptr  = (unsigned char *)addr;
    int print_bytes     = 0;
    int print_bits      = 7;

    if(ptr == NULL)
    {
        return -1;
    }

    for(print_bytes = 0;
        print_bytes < size;
        print_bytes++, ptr++)
    {
        for(print_bits = 7;
        print_bits >= 0;
        print_bits--)
        {
            printk("%d", ((*ptr >> print_bits) & 1));
        }

    }
    printk("\n");
    return print_bytes;
}



/*
 * print the module information
 */
static void print_module(void)
{
    struct module *mod;

    printk(KERN_ALERT "this module: %p==%p\n", &__this_module, THIS_MODULE);
    printk(KERN_ALERT "module state: %d\n", THIS_MODULE->state);
    printk(KERN_ALERT "module name: %s\n", THIS_MODULE->name);

    list_for_each_entry(mod, *(&THIS_MODULE->list.prev), list);
    printk(KERN_ALERT "module name: %s\n", mod->name);
    printk(KERN_ALERT "module state: %d\n", THIS_MODULE->state);
}


static void print_vmarea(void)
{
    printk("---------------------\n");
    printk("TASK_SIZE = %p\n", TASK_SIZE);
    printk("---------------------\n");
    printk("STACK_TOP = %p\n", STACK_TOP);
    //printf("MMAP_BASE = %p\n", TASK_UNMAPPED_SIZE);
}

// http://lxr.free-electrons.com/source/arch/x86/include/asm/segment.h#L123
static void print_segment(void)
{
    long data = 0;

    /*

        ---------------------------------------------------------------------------------------------
        |                         |       INDEX               | TI| RPL |
        ---------------------------------------------------------------------------------------------
        __KERNEL_CS = 0x0010 =    B 0 0 0 0 0 0 0 0 0 0 0 1 0 | 0 | 0 0 | index = 2, TI = 0, RPL = 0
        ---------------------------------------------------------------------------------------------
        __KERNEL_DS = 0x0018 =    B 0 0 0 0 0 0 0 0 0 0 0 1 1 | 0 | 0 0 | index = 3, TI = 0, RPL = 0
        ---------------------------------------------------------------------------------------------
        __USER_DS   = 0x0033 =    B 0 0 0 0 0 0 0 0 0 0 1 1 0 | 0 | 1 1 | index = 6, TI = 0, RPL = 3
        ---------------------------------------------------------------------------------------------
        __USER_DS   = 0x002B =    B 0 0 0 0 0 0 0 0 0 0 1 0 1 | 0 | 1 1 | index = 5, TI = 0, RPL = 3
        ---------------------------------------------------------------------------------------------

        |   LE little-endian   |  低字节 -=>  高字节  |
        ---------------------------------------------------------------------------------------------
        __KERNEL_CS = 0x0010 = | 00010000    00000000 |
        __KERNEL_DS = 0x0018 = | 00011000    00000000 |
        __USER_DS   = 0x0033 = | 00110011    00000000 |
        __USER_DS   = 0x002B = | 00101011    00000000 |
     */
    data = __KERNEL_CS;
    printk("__KERNEL_CS = %0x\n", data);
    print_bit(&data, 2);

    data = __KERNEL_DS;
    printk("__KERNEL_DS = %0x\n", data);
    print_bit(&data, 2);

    data = __USER_CS;
    printk("__USER_CS   = %0x\n", data);
    print_bit(&data, 2);

    data = __USER_DS;
    printk("__USER_DS   = %0x\n", data);
    print_bit(&data, 2);
    //printk("__ESPFIX_SS = %0x\n", __ESPFIX_SS);

}


static int hello_init(void)
{
    print_module( );

    printk(KERN_ALERT "run in cpu %d\n", get_cpu());

    //printk(KERN_ALERT "PAGE_OFFSET : 0x%lx, TASK_SIZE : 0x%lx", PAGE_OFFSET, TASK_SIZE);
    printk(KERN_ALERT "PAGE_OFFSET : 0x%lx\n", PAGE_OFFSET);

    print_vmarea( );

    print_segment( );

    return 0;
}



static void hello_exit(void)
{
}


module_init(hello_init);
module_exit(hello_exit);
