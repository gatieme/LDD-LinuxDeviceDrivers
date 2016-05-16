/*************************************************************************
    > File Name: get_task.c
    > Author: gatieme
    > Created Time: Mon 16 May 2016 03:28:46 PM CST
 ************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>


void print_vm_list(struct task_struct *task)
{
    struct vm_area_struct *tmp = task->mm->mmap;

    printk("process:%s,pid:%d\n", task->comm, task->pid);

    while (tmp != NULL)
    {

        printk("0x%lx - 0x%lx\t",tmp->vm_start,tmp->vm_end);

        (tmp->vm_flags & VM_READ)   ? printk("r") : printk("-");
        (tmp->vm_flags & VM_WRITE)  ? printk("w") : printk("-");
        (tmp->vm_flags & VM_EXEC)   ? printk("x") : printk("-");
        (tmp->vm_flags & VM_SHARED) ? printk("s") : printk("p");
        printk("\n");

        tmp = tmp->vm_next;
    }
}


