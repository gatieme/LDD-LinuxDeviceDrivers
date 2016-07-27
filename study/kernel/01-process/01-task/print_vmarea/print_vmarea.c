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



#define STACK_SIZE  (8192 * 1024)

static int pid = 1;

module_param(pid,int,0644);

void print_vmraea_node(struct vm_area_struct *vmnode)
{
	printk("0x%lx - 0x%lx\t", vmnode->vm_start, vmnode->vm_end);
	if (vmnode->vm_flags & VM_READ)
		printk("r");
	else
		printk("-");

	if (vmnode->vm_flags & VM_WRITE)
		printk("w");
	else
		printk("-");

	if (vmnode->vm_flags & VM_EXEC)
		printk("x");
	else
		printk("-");

	if (vmnode->vm_flags & VM_SHARED)
		printk("s\n");
	else
		printk("p\n");
}


static void print_vm_list(struct vm_area_struct *vmlist)
{
    struct vm_area_struct   *vmnode = vmlist;

    while (vmnode != NULL)
    {
        print_vmraea_node(vmnode);

        vmnode = vmnode->vm_next;
	}
}



static void visit(struct rb_node *root)
{
	struct vm_area_struct *vmnode;
	vmnode =  container_of(root,struct vm_area_struct,vm_rb);
    print_vmraea_node(vmnode);
}


static void print_vm_rb_tree(struct rb_node *root)
{
	if (root != NULL)
    {
		print_vm_rb_tree(root->rb_left);
		visit(root);
		print_vm_rb_tree(root->rb_right);
	}
}


void print_mm_struct(struct mm_struct *mm)
{
    printk("code : [0x%lx, 0x%lx]\n", mm->start_code, mm->end_code);
    printk("data : [0x%lx, 0x%lx]\n", mm->start_data, mm->end_data);

    //  heap从低地址向高地址扩展，做内存管理相对要简单些。
    //  stack从高地址向低地址扩展，这样栈空间的起始位置就能确定下来，动态的调整栈空间大>
    printk("heap : [0x%lx, 0x%lx]\n", mm->start_brk, mm->brk);
    printk("stack: [0x%lx, 0x%lx]\n", mm->start_stack - STACK_SIZE, mm->start_stack);
}


static void print_vmarea(void)
{
    struct task_struct      *p = NULL;
	struct pid              *k = NULL;

    struct  mm_struct       *mm = NULL;

    struct vm_area_struct   *vmlist = NULL;
	struct rb_node          *root = NULL;

	k = find_vpid(pid);
	p = pid_task(k, PIDTYPE_PID);

    mm = p->mm;
    print_mm_struct(mm);

    //  visit vmarea by list
	printk("-------[visit vmarea by list   START]-------\n");
    vmlist = mm->mmap;
    print_vm_list(vmlist);
	printk("-------[visit vmarea by list     END]-------\n");

    // visit vmarea by rbtree
	printk("-------[visit vmarea by rbtree START]-------\n");
	root = mm->mm_rb.rb_node;
    print_vm_rb_tree(root);

	printk("-------[visit vmarea by rbtree   END]-------\n");

    return ;
}

static int __init printvm_init(void)
{
    print_vmarea( );
    return 0;
}

static void __exit printvm_exit(void)
{
   printk(KERN_INFO"printvm exit ---------------------!\n");
}



module_init(printvm_init);
module_exit(printvm_exit);
MODULE_LICENSE("GPL");
