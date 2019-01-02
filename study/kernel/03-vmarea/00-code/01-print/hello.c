#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
//#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
//#include <asm/pgtable_32_types.h>
//#include <asm-generic/sections.h>
#include <linux/kallsyms.h>
#include <linux/slab.h>

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



#ifdef CONFIG_X86
static void print_virtual_kernel_memoty_layout(void)
{
	printk(KERN_INFO "virtual kernel memory layout:\n"
		"    fixmap  : 0x%08lx - 0x%08lx   (%4ld kB)\n"
		"  cpu_entry : 0x%08lx - 0x%08lx   (%4ld kB)\n"
#ifdef CONFIG_HIGHMEM
		"    pkmap   : 0x%08lx - 0x%08lx   (%4ld kB)\n"
#endif
		"    vmalloc : 0x%08lx - 0x%08lx   (%4ld MB)\n"
		"    lowmem  : 0x%08lx - 0x%08lx   (%4ld MB)\n",
		//"      .init : 0x%08lx - 0x%08lx   (%4ld kB)\n"
		//"      .data : 0x%08lx - 0x%08lx   (%4ld kB)\n"
		//"      .text : 0x%08lx - 0x%08lx   (%4ld kB)\n",
		FIXADDR_START, FIXADDR_TOP,
		(FIXADDR_TOP - FIXADDR_START) >> 10,

		CPU_ENTRY_AREA_BASE,
		CPU_ENTRY_AREA_BASE + CPU_ENTRY_AREA_MAP_SIZE,
		CPU_ENTRY_AREA_MAP_SIZE >> 10,

#ifdef CONFIG_HIGHMEM
		PKMAP_BASE, PKMAP_BASE+LAST_PKMAP*PAGE_SIZE,
		(LAST_PKMAP*PAGE_SIZE) >> 10,
#endif

		VMALLOC_START, VMALLOC_END,
		(VMALLOC_END - VMALLOC_START) >> 20,

		(unsigned long)__va(0), (unsigned long)high_memory,
		((unsigned long)high_memory - (unsigned long)__va(0)) >> 20);

		//(unsigned long)&__init_begin, (unsigned long)&__init_end,
		//((unsigned long)&__init_end -
		// (unsigned long)&__init_begin) >> 10,

		//(unsigned long)&_etext, (unsigned long)&_edata,
		//((unsigned long)&_edata - (unsigned long)&_etext) >> 10,

		//.autorelabel(unsigned long)&_text, (unsigned long)&_etext,
		//((unsigned long)&_etext - (unsigned long)&_text) >> 10);

	/*
	 * Check boundaries twice: Some fundamental inconsistencies can
	 * be detected at build time already.
	 */
#define __FIXADDR_TOP (-PAGE_SIZE)
#ifdef CONFIG_HIGHMEM
	BUILD_BUG_ON(PKMAP_BASE + LAST_PKMAP*PAGE_SIZE	> FIXADDR_START);
	BUILD_BUG_ON(VMALLOC_END			> PKMAP_BASE);
#endif
#define high_memory (-128UL << 20)
	BUILD_BUG_ON(VMALLOC_START			>= VMALLOC_END);
#undef high_memory
#undef __FIXADDR_TOP

#ifdef CONFIG_HIGHMEM
	BUG_ON(PKMAP_BASE + LAST_PKMAP*PAGE_SIZE	> FIXADDR_START);
	BUG_ON(VMALLOC_END				> PKMAP_BASE);
#endif
	BUG_ON(VMALLOC_START				>= VMALLOC_END);
	BUG_ON((unsigned long)high_memory		> VMALLOC_START);
}

#elif defined(CONFIG_ARM)

static void print_virtual_kernel_memoty_layout(void)
{
#define MLK(b, t) b, t, ((t) - (b)) >> 10
#define MLM(b, t) b, t, ((t) - (b)) >> 20
#define MLK_ROUNDUP(b, t) b, t, DIV_ROUND_UP(((t) - (b)), SZ_1K)

	printk("Virtual kernel memory layout:\n"
			"	vector  : 0x%08lx - 0x%08lx   (%4ld kB)\n"
#ifdef CONFIG_HAVE_TCM
			"	DTCM    : 0x%08lx - 0x%08lx   (%4ld kB)\n"
			"	ITCM    : 0x%08lx - 0x%08lx   (%4ld kB)\n"
#endif
			"	fixmap  : 0x%08lx - 0x%08lx   (%4ld kB)\n"
			"	vmalloc : 0x%08lx - 0x%08lx   (%4ld MB)\n"
			"	lowmem  : 0x%08lx - 0x%08lx   (%4ld MB)\n"
#ifdef CONFIG_HIGHMEM
			"	pkmap   : 0x%08lx - 0x%08lx   (%4ld MB)\n"
#endif
#ifdef CONFIG_MODULES
			"	modules : 0x%08lx - 0x%08lx   (%4ld MB)\n"
#endif
			"	  .text : 0x%p" " - 0x%p" "   (%4td kB)\n"
			"	  .init : 0x%p" " - 0x%p" "   (%4td kB)\n"
			"	  .data : 0x%p" " - 0x%p" "   (%4td kB)\n",
			//"	   .bss : 0x%p" " - 0x%p" "   (%4td kB)\n",

			MLK(VECTORS_BASE, VECTORS_BASE + PAGE_SIZE),
#ifdef CONFIG_HAVE_TCM
			MLK(DTCM_OFFSET, (unsigned long) dtcm_end),
			MLK(ITCM_OFFSET, (unsigned long) itcm_end),
#endif
			MLK(FIXADDR_START, FIXADDR_END),
			MLM(VMALLOC_START, VMALLOC_END),
			MLM(PAGE_OFFSET, (unsigned long)high_memory),
#ifdef CONFIG_HIGHMEM
			MLM(PKMAP_BASE, (PKMAP_BASE) + (LAST_PKMAP) *
				(PAGE_SIZE)),
#endif
#ifdef CONFIG_MODULES
			MLM(MODULES_VADDR, MODULES_END),
#endif

			MLK_ROUNDUP(_text, _etext),
			MLK_ROUNDUP(__init_begin, __init_end),
			MLK_ROUNDUP(_sdata, _edata));
			//MLK_ROUNDUP(__bss_start, __bss_stop));

#undef MLK
#undef MLM
#undef MLK_ROUNDUP

	/*
	 * Check boundaries twice: Some fundamental inconsistencies can
	 * be detected at build time already.
	 */
#ifdef CONFIG_MMU
	BUILD_BUG_ON(TASK_SIZE				> MODULES_VADDR);
	BUG_ON(TASK_SIZE 				> MODULES_VADDR);
#endif

#ifdef CONFIG_HIGHMEM
	BUILD_BUG_ON(PKMAP_BASE + LAST_PKMAP * PAGE_SIZE > PAGE_OFFSET);
	BUG_ON(PKMAP_BASE + LAST_PKMAP * PAGE_SIZE	> PAGE_OFFSET);
#endif
}
#endif


static void test_virtual_kernel_memoty_layout(void)
{
#define TEST_KMALLOC_SIZE 10
	char *test_kmalloc = NULL;
	test_kmalloc = kmalloc(sizeof(char) * TEST_KMALLOC_SIZE, GFP_KERNEL);
	if (test_kmalloc)
		printk("[%s %d] test_kmalloc : addr = 0x%0lx, size = %d\n", __func__, __LINE__, test_kmalloc, TEST_KMALLOC_SIZE);
	kfree(test_kmalloc);
	test_kmalloc = NULL;

#define TEST_VMALLOC_SIZE (108 * 1024 * 1024)
	char *test_vmalloc = NULL;
	test_vmalloc = vmalloc(sizeof(char) * TEST_VMALLOC_SIZE);
	if (test_vmalloc)
		printk("[%s %d] test_vmalloc : addr = 0x%0lx, size = %d\n", __func__, __LINE__, test_vmalloc, TEST_VMALLOC_SIZE);
	vfree(test_vmalloc);
	test_vmalloc = NULL;
}


static void print_vmarea(void)
{
#define HIGHMEM_END (unsigned long)(4 * 1024 * 1024 * 1024)
#define HIGHMEM_START ((unsigned long)(-128UL << 20))
#define high_memory HIGHMEM_START

	printk("|---------------------|	HIGHMEM_END = 0x1%0lx(4GB)\n", (unsigned long)4 << 30);
	printk("|                     |	[%ldK]\n", (HIGHMEM_END - FIXADDR_TOP));
	printk("|---------------------|	FIXADDR_TOP = 0x%0lx\n", FIXADDR_TOP);
	printk("|                     |	Fix-mappinged Linear Address [%ldK]\n", (FIXADDR_TOP - FIXADDR_START) >> 10);
	printk("|---------------------|	FIXADDR_START = 0x%0lx\n", FIXADDR_START);
	printk("|                     |	Persistent Kernel Mapping [%ldM]\n", (FIXADDR_START - PKMAP_BASE) >> 20);
	printk("|---------------------|	PKMAP_BASE = 0x%0lx\n", PKMAP_BASE);
	printk("|                     |	[%ldK]\n", (PKMAP_BASE - VMALLOC_END) >> 10);
	printk("|---------------------|	VMALLOC_END   = 0x%0lx\n", VMALLOC_END);
	printk("|                     |	Vmalloc Area [%ldM]\n", (VMALLOC_END - VMALLOC_START) >> 20);
	printk("|---------------------|	VMALLOC_START = 0x%0lx\n", VMALLOC_START);
	printk("|                     |	VMALLOC_OFFSET = [%luM/%luM]\n", (unsigned long)VMALLOC_OFFSET >> 20, (unsigned long)(VMALLOC_START - HIGHMEM_START) >> 20);
	printk("|---------------------|	HIGHMEM_START = 0x%0lx\n", HIGHMEM_START);
	printk("|                     |	Physical Memory Mapping[%ldM]\n", (HIGHMEM_START - PAGE_OFFSET) >> 20);
	printk("|---------------------|	PAGE_OFFSET   = 0x%0lx\n", PAGE_OFFSET);
	printk("TASK_SIZE = 0x%0lx(%ldG)\n", TASK_SIZE, TASK_SIZE >> 30);
	printk("---------------------\n");
	printk("STACK_TOP = 0x%0lx(%ldG)\n", STACK_TOP, STACK_TOP >> 30);

#undef high_memory

	print_virtual_kernel_memoty_layout();
	test_virtual_kernel_memoty_layout();
}

// http://lxr.free-electrons.com/source/arch/x86/include/asm/segment.h#L123
#if CONFIG_X86
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
	printk("__KERNEL_CS = 0x%0lx\n", data);
	//print_bit(&data, 2);

	data = __KERNEL_DS;
	printk("__KERNEL_DS = 0x%0lx\n", data);
	//print_bit(&data, 2);

	data = __USER_CS;
	printk("__USER_CS   = 0x%0lx\n", data);
	//print_bit(&data, 2);

	data = __USER_DS;
	printk("__USER_DS   = 0x%0lx\n", data);
	//print_bit(&data, 2);
	//printk("__ESPFIX_SS = %0x\n", __ESPFIX_SS);
}
#else
static inline void print_segment(void)
{
}
#endif

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
