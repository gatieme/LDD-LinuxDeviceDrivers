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


static int pid = 1;

module_param(pid,int,0644);

static void print_vm_area(struct vm_area_struct)
{
    ((tmp->vm_flags & VM_READ)   == 1) ? printk("r") : printk("-");
    ((tmp->vm_flags & VM_WRITE)  == 1) ? printk("w") : printk("-");
    ((tmp->vm_flags & VM_EXEC)   == 1) ? printk("x") : printk("-");
    ((tmp->vm_flags & VM_SHARED) == 1) ? printk("s") : printk("p");

    printk("\n");
}

static void print_vm_area_by_list(struct task_struct *task)
{
	struct vm_area_struct *tmp = task->mm->mmap;

	printk("process:%s,pid:%d\n", task->comm, task->pid);

	while (tmp != NULL)
    {
        print_vm_area(tmp);
		tmp = tmp->vm_next;
	}
}


static void visit_rbtree_node(struct rb_node *root)
{
	struct vm_area_struct *tmp;

    tmp =  container_of(root,struct vm_area_struct,vm_rb);

    printk("0x%lx - 0x%lx\t",tmp->vm_start,tmp->vm_end);

    print_vm_area(tmp);
}

static void preorder_rbtree(struct rb_node *root)
{
	if (root != NULL)
    {
		visit(root);
		print_rb_tree(root->rb_left);
		print_rb_tree(root->rb_right);
	}
}

static void print_vm_area_by_rbtree(struct task_struct *task)
{
   struct task_struct *p;
	struct pid *k;
	struct rb_node *root = NULL;

	root = task->mm->mm_rb.rb_node;
	print_rb_tree(root);

}
