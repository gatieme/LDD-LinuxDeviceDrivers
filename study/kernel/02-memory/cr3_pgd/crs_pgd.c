//#include <asm/system.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/sched.h>
#include <linux/types.h>

static unsigned long read_pgd(void)
{
    return (unsigned long)current->mm->pgd;
}

static int __init init_cr3_pgd(void)
{
    unsigned long cr3;
    unsigned long pgd = 0;

    cr3 = read_cr3();
    printk("cr3----------------->0x%lx\n", cr3);
    pgd = __pa(read_pgd());
    printk("pgd----------------->0x%lx\n", pgd);
    return 0;
}

static void __exit exit_cr3_pgd(void)
{
    printk("--------------------->exit!\n");
    return ;
}

module_init(init_cr3_pgd);
module_exit(exit_cr3_pgd);
