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
    struct desc_struct *gdt_table = get_cpu_gdt_table(0);

    print_cpu_gdt_table(gdt_table, GDT_ENTRIES);

    return 0;
}



static void hello_exit(void)
{
}


module_init(hello_init);
module_exit(hello_exit);
