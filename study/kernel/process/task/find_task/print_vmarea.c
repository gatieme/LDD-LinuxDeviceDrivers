#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>

#ifndef offsetof
#define offsetof(type, field)   ((long) &((type *)0)->field)
#endif   /* offsetof */

#ifndef container_of
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

static void print_vmarea(struct vm_area_struct *vmarea)
{
    printk("0x%lx - 0x%lx\t", vmarea->vm_start, vmarea->vm_end);

    ((vmarea->vm_flags & VM_READ)   == 1) ? printk("r") : printk("-");
    ((vmarea->vm_flags & VM_WRITE)  == 1) ? printk("w") : printk("-");
    ((vmarea->vm_flags & VM_EXEC)   == 1) ? printk("x") : printk("-");
    ((vmarea->vm_flags & VM_SHARED) == 1) ? printk("s") : printk("p");

    printk("\n");
}

static void print_vmarea_by_list(struct task_struct *task)
{
	struct vm_area_struct *vmarea = task->mm->mmap;

	printk("process : %s, pid : %d\n", task->comm, task->pid);

	while (vmarea != NULL)
    {
        print_vmarea(vmarea);
		vmarea = vmarea->vm_next;
	}
}


static void visit_rbtree_node(struct rb_node *root)
{
	struct vm_area_struct *vmarea = NULL;

    vmarea = container_of(root,struct vm_area_struct,vm_rb);

    print_vmarea(vmarea);
}

static void preorder_rbtree(struct rb_node *root)
{
	if (root != NULL)
    {
		visit_rbtree_node(root);
		preorder_rbtree(root->rb_left);
		preorder_rbtree(root->rb_right);
	}
}

static void print_vmarea_by_rbtree(struct task_struct *task)
{
	struct rb_node *root = NULL;

	root = task->mm->mm_rb.rb_node;

    preorder_rbtree(root);

}

void print_task_vmarea(struct task_struct *task)
{
    printk(KERN_ALERT "print vmarea_list");
    print_vmarea_by_list(task);
    printk(KERN_ALERT "print vmarea_rbtree");
    print_vmarea_by_rbtree(task);

}
