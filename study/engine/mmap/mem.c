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
static int pid;
static unsigned long va;

module_param(pid,int,0644);
module_param(va,ulong,0644);

static int find_pgd_init(void)
{
        unsigned long pa = 0;
        struct task_struct *pcb_tmp = NULL;
        pgd_t *pgd_tmp = NULL;
        pud_t *pud_tmp = NULL;
        pmd_t *pmd_tmp = NULL;
        pte_t *pte_tmp = NULL;

        printk(KERN_INFO"PAGE_OFFSET = 0x%lx\n",PAGE_OFFSET);
        printk(KERN_INFO"PGDIR_SHIFT = %d\n",PGDIR_SHIFT);
        printk(KERN_INFO"PUD_SHIFT = %d\n",PUD_SHIFT);
        printk(KERN_INFO"PMD_SHIFT = %d\n",PMD_SHIFT);
        printk(KERN_INFO"PAGE_SHIFT = %d\n",PAGE_SHIFT);

        printk(KERN_INFO"PTRS_PER_PGD = %d\n",PTRS_PER_PGD);
        printk(KERN_INFO"PTRS_PER_PUD = %d\n",PTRS_PER_PUD);
        printk(KERN_INFO"PTRS_PER_PMD = %d\n",PTRS_PER_PMD);
        printk(KERN_INFO"PTRS_PER_PTE = %d\n",PTRS_PER_PTE);

        printk(KERN_INFO"PAGE_MASK = 0x%lx\n",PAGE_MASK);

        if(!(pcb_tmp = find_task_by_pid(pid))) {
                printk(KERN_INFO"Can't find the task %d .\n",pid);
                return 0;
        }
        printk(KERN_INFO"pgd = 0x%p\n",pcb_tmp->mm->pgd);
                /* 判断给出的地址va是否合法(va&lt;vm_end)*/
        if(!find_vma(pcb_tmp->mm,va)){
                printk(KERN_INFO"virt_addr 0x%lx not available.\n",va);
                return 0;
        }
        pgd_tmp = pgd_offset(pcb_tmp->mm,va);
        printk(KERN_INFO"pgd_tmp = 0x%p\n",pgd_tmp);
        printk(KERN_INFO"pgd_val(*pgd_tmp) = 0x%lx\n",pgd_val(*pgd_tmp));
        if(pgd_none(*pgd_tmp)){
                printk(KERN_INFO"Not mapped in pgd.\n");
                return 0;
        }
        pud_tmp = pud_offset(pgd_tmp,va);
        printk(KERN_INFO"pud_tmp = 0x%p\n",pud_tmp);
        printk(KERN_INFO"pud_val(*pud_tmp) = 0x%lx\n",pud_val(*pud_tmp));
        if(pud_none(*pud_tmp)){
                printk(KERN_INFO"Not mapped in pud.\n");
                return 0;
        }
        pmd_tmp = pmd_offset(pud_tmp,va);
        printk(KERN_INFO"pmd_tmp = 0x%p\n",pmd_tmp);
        printk(KERN_INFO"pmd_val(*pmd_tmp) = 0x%lx\n",pmd_val(*pmd_tmp));
        if(pmd_none(*pmd_tmp)){
                printk(KERN_INFO"Not mapped in pmd.\n");
                return 0;
        }
        /*在这里，把原来的pte_offset_map()改成了pte_offset_kernel*/
        pte_tmp = pte_offset_kernel(pmd_tmp,va);

        printk(KERN_INFO"pte_tmp = 0x%p\n",pte_tmp);
        printk(KERN_INFO"pte_val(*pte_tmp) = 0x%lx\n",pte_val(*pte_tmp));
        if(pte_none(*pte_tmp)){
                printk(KERN_INFO"Not mapped in pte.\n");
                return 0;
        }
        if(!pte_present(*pte_tmp)){
                printk(KERN_INFO"pte not in RAM.\n");
                return 0;
        }
        pa = (pte_val(*pte_tmp) &amp; PAGE_MASK) |(va &amp; ~PAGE_MASK);
        printk(KERN_INFO"virt_addr 0x%lx in RAM is 0x%lx .\n",va,pa);
        printk(KERN_INFO"contect in 0x%lx is 0x%lx\n",pa,
                *(unsigned long *)((char *)pa + PAGE_OFFSET));

        return 0;

}

static void  find_pgd_exit(void)
{
        printk(KERN_INFO"Goodbye!\n");

}

module_init(find_pgd_init);
module_exit(find_pgd_exit);
