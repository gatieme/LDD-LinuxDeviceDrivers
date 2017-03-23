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


#define bit_get(number, pos) ((number) >> (pos) & 1)     /// 用宏得到某数的某位
#define bit_set(number, pos) ((number) |= 1 << (pos))    /// 把某位置1
#define bit_clr(number, pos) ((number) &= ~(1 << (pos))) /// 把某位清0
#define bit_cpl(number, pos) ((number) ^= 1 << (pos))    /// 把number的POS位取反

#define get_segment_INDEX(n)    ((n) >> 3)
#define get_segment_TI(n)       (((n) >> 2) & 0x01)
#define get_segment_RPL(n)      ((n) & 0x03)

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
    printk("__KERNEL_CS = %0x, index = %d, TI = %d, RPL = %d\n", data,
            get_segment_INDEX(data),
            get_segment_TI(data),
            get_segment_RPL(data));

    data = __KERNEL_DS;
    printk("__KERNEL_DS = %0x, index = %d, TI = %d, RPL = %d\n", data,
            get_segment_INDEX(data),
            get_segment_TI(data),
            get_segment_RPL(data));

    data = __USER_CS;
    printk("__USER_CS   = %0x, index = %d, TI = %d, RPL = %d\n", data,
            get_segment_INDEX(data),
            get_segment_TI(data),
            get_segment_RPL(data));

    data = __USER_DS;
    printk("__USER_DS   = %0x, index = %d, TI = %d, RPL = %d\n", data,
            get_segment_INDEX(data),
            get_segment_TI(data),
            get_segment_RPL(data));
}


static void print_desc_struct(struct desc_struct *desc)
{
#if 0
    printk("limit0 = %0x, ", desc->limit0);
    printk("base0 = %0x, ", desc->base0);

    printk("base1 = %0x, ", desc->base1);
    printk("type = %0x, ", desc->type);
    printk("s = %0x, ", desc->s);
    printk("dpl = %0x, ", desc->dpl);
    printk("p = %0x, ", desc->p);

    printk("limit = %0x, ", desc->limit);
    printk("avl = %0x, ", desc->avl);
    printk("l = %0x, ", desc->l);
    printk("d = %0x, ", desc->d);
    printk("g = %0x, ", desc->g);

    printk("base2 = %0x\n", desc->base2);
#endif
    printk("0x%08x, 0x%08x | 0x%x  |  %d  | %d |\n",
            desc->b, desc->a, desc->type, desc->dpl, desc->s);
}


static void print_cpu_gdt_table(struct desc_struct *desc, int size)
{
    int i = 0;
    printk("-----------------------------------\n");
    printk("|\t\t\t\t| type | dpl | s |\n");
    for(i = 0; i < size; i++)
    {
        printk("| [%0x] = ", i);
        print_desc_struct(desc + i);
    }
    printk("-----------------------------------\n");

}

static int hello_init(void)
{
    printk("GDT_ENTRIES = %d\n", GDT_ENTRIES);
    struct desc_struct *gdt_table = get_cpu_gdt_table(1);

    print_cpu_gdt_table(gdt_table, GDT_ENTRIES);

    print_segment();
    return 0;
}



static void hello_exit(void)
{
}


module_init(hello_init);
module_exit(hello_exit);
