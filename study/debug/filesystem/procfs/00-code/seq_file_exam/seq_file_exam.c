//kernel module: seqfile_exam.c
#include <linux/init.h>
#include <linux/module.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <linux/percpu.h>
#include <linux/sched.h>
#include <linux/version.h>


static struct proc_dir_entry *entry;
#define PROC_NAME   "exam_seq_file"


static void *l_start(struct seq_file *m, loff_t * pos)
{
    loff_t index = *pos;

    if (index == 0) {
        seq_printf(m, "Current all the processes in system:\n"
                   "%-24s%-5s\n", "name", "pid");
        return &init_task;
    }
    else {
        return NULL;
    }
}

static void *l_next(struct seq_file *m, void *p, loff_t * pos)
{
    struct task_struct *task = (struct task_struct *)p;

    task = next_task(task);
    if ((*pos != 0) && (task == &init_task)) {
        return NULL;
    }
    ++*pos;
    return task;
}

static void l_stop(struct seq_file *m, void *p)
{
}

static int l_show(struct seq_file *m, void *p)
{
    struct task_struct * task = (struct task_struct *)p;

    seq_printf(m, "%-24s%-5d\n", task->comm, task->pid);
    return 0;
}

static struct seq_operations exam_seq_op = {
    .start = l_start,
    .next  = l_next,
    .stop  = l_stop,
    .show  = l_show
};

static int exam_seq_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &exam_seq_op);
}

static struct file_operations exam_seq_fops = {
    .open = exam_seq_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release,
};

static int __init exam_seq_init(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
    entry = proc_create(PROC_NAME, 0, NULL, &exam_seq_fops);
#else
    entry = create_proc_entry(PROC_NAME, 0666, NULL);
    if (entry != NULL)
        entry->proc_fops = &exam_seq_fops;
#endif

    if(entry == NULL)
    {
        remove_proc_entry(PROC_NAME, NULL);
        printk(KERN_DEBUG "Error: Could not initialize /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    return 0;
}

static void __exit exam_seq_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_DEBUG "/proc/%s removed\n", PROC_NAME);

}

module_init(exam_seq_init);
module_exit(exam_seq_exit);
MODULE_LICENSE("GPL");
