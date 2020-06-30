/*****************************************************************
文件名：mem.c
输入参数：
pid 接收待查询进程的PID
va 接收待查询的虚拟地址
*****************************************************************/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <linux/printk.h>
#include <linux/memblock.h>
#include <linux/kallsyms.h>
#include <asm/fixmap.h>

MODULE_LICENSE("GPL");

typedef unsigned long (*PFUNC)(void);

#if 0
unsigned long SYMS_FUN(char *name)
{
	PFUNC func = (PFUNC)kallsyms_lookup_name(name);

	//printk("0x%16lx\n", (unsigned long)func);
	return (*func)();
}
#endif
void show_layout(void)
{
#define SYMS_VAL(name)	((unsigned long)kallsyms_lookup_name(name))
#define SYMS_FUN(name)	((*((PFUNC)kallsyms_lookup_name(name)))())
#define MLK(b, t) b, t, ((t) - (b)) >> 10
#define MLM(b, t) b, t, ((t) - (b)) >> 20
#define MLG(b, t) b, t, ((t) - (b)) >> 30
#define MLK_ROUNDUP(b, t) b, t, DIV_ROUND_UP(((t) - (b)), SZ_1K)
#if 0
	pr_notice("Virtual kernel memory layout:\n");
#ifdef CONFIG_KASAN
	pr_notice("    kasan   : 0x%16lx - 0x%16lx   (%6ld GB)\n",
		MLG(KASAN_SHADOW_START, KASAN_SHADOW_END));
#endif
	pr_notice("    modules : 0x%16lx - 0x%16lx   (%6ld MB)\n",
		MLM(MODULES_VADDR, MODULES_END));
	pr_notice("    vmalloc : 0x%16lx - 0x%16lx   (%6ld GB)\n",
		MLG(VMALLOC_START, VMALLOC_END));
	pr_notice("    kernel  : 0x%16lx - 0x%16lx   (%6ld KB)\n",
		MLK(SYMS_VAL("_text"), SYMS_VAL("_end")));
	pr_notice("      .text : 0x%16lx - 0x%16lx   (%6ld KB)\n",
		MLK_ROUNDUP(SYMS_VAL("_text"), SYMS_VAL("_etext")));
	pr_notice("    .rodata : 0x%16lx - 0x%16lx   (%6ld KB)\n",
		MLK_ROUNDUP(SYMS_VAL("__start_rodata"), SYMS_VAL("__init_begin")));
	pr_notice("      .init : 0x%16lx - 0x%16lx   (%6ld KB)\n",
		MLK_ROUNDUP(SYMS_VAL("__init_begin"), SYMS_VAL("__init_end")));
	pr_notice("      .data : 0x%16lx - 0x%16lx   (%6ld KB)\n",
		MLK_ROUNDUP(SYMS_VAL("_sdata"), SYMS_VAL("_edata")));
	pr_notice("       .bss : 0x%16lx - 0x%16lx   (%6ld KB)\n",
		MLK_ROUNDUP(SYMS_VAL("__bss_start"), SYMS_VAL("__bss_stop")));
	pr_notice("    fixed   : 0x%16lx - 0x%16lx   (%6ld KB)\n",
		MLK(FIXADDR_START, FIXADDR_TOP));
	pr_notice("    PCI I/O : 0x%16lx - 0x%16lx   (%6ld MB)\n",
		MLM(PCI_IO_START, PCI_IO_END));
#ifdef CONFIG_SPARSEMEM_VMEMMAP
	pr_notice("    vmemmap : 0x%16lx - 0x%16lx   (%6ld GB maximum)\n",
		MLG(SYMS_VAL("vmemmap"), SYMS_VAL("vmemmap") + VMEMMAP_SIZE));
	pr_notice("              0x%16lx - 0x%16lx   (%6ld MB actual)\n",
		MLM((unsigned long)phys_to_page(SYMS_FUN("memblock_start_of_DRAM")),
		    (unsigned long)virt_to_page(high_memory)));
#endif
	pr_notice("    memory  : 0x%16lx - 0x%16lx   (%6ld MB)\n",
		MLM((unsigned long)phys_to_virt(SYMS_FUN("memblock_start_of_DRAM")),
		    (unsigned long)high_memory));
#endif
	//pr_notice("KIMAGE_VADDR: 0x%16lx\n", KIMAGE_VADDR);
	pr_notice("PAGE_OFFSET : 0x%16lx\n", PAGE_OFFSET);
#undef MLK
#undef MLM
#undef MLK_ROUNDUP
}

static int __init show_layout_init(void)
{
	show_layout();

        return 0;
}

static void __exit show_layout_exit(void)
{
        printk(KERN_INFO"Goodbye!\n");
}

module_init(show_layout_init);
module_exit(show_layout_exit);
