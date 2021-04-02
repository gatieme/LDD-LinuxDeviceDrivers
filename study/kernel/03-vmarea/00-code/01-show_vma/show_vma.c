#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
//#include <linux/moduleparam.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/string_helpers.h>
#include <linux/version.h>

#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <linux/sched/task.h>
#include <linux/sched/signal.h>

#define __PS(S, F) SEQ_printf(m, "%-45s:%21Ld\n", S, (long long)(F))
#define __P(F) __PS(#F, F)
#define   P(F) __PS(#F, p->F)
#define __PSN(S, F) SEQ_printf(m, "%-45s:%14Ld.%-10ld\n", S, SPLIT_NS((long long)(F)))
#define __PN(F) __PSN(#F, F)
#define   PN(F) __PSN(#F, p->F)

/*
 * This allows printing both to /proc/sched_debug and
 * to the console
 */
#define SEQ_printf(m, x...)			\
	 do {						\
		if (m)					\
			seq_printf(m, x);		\
		else					\
			pr_cont(x);			\
	  } while (0)

/*
static long long nsec_high(unsigned long long nsec)
{
	if ((long long)nsec < 0) {
			nsec = -nsec;
			do_div(nsec, NSEC_PER_SEC);
			return -nsec;
		}
	do_div(nsec, NSEC_PER_SEC);

	return nsec;
}

static unsigned long nsec_low(unsigned long long nsec)
{
	if ((long long)nsec < 0)
		nsec = -nsec;

	return do_div(nsec, NSEC_PER_SEC);
}
*/
#define SPLIT_NS(x) nsec_high(x), nsec_low(x)

#define MAX_LINE 100
static int PID = -1;
static DEFINE_SPINLOCK(mm_show_vma_lock);
//module_param(PID, int, 0644);

static void show_task_vma(struct task_struct *tsk, struct seq_file *m)
{
	struct mm_struct *mm;
	struct vm_area_struct *vma;
	int nr = 0;
	unsigned long start, end, length;

	mm = tsk->mm;
	if (mm == NULL) {
		SEQ_printf(m, "mm_struct addr = 0x%p\n", mm);
		return;
	}
	SEQ_printf(m, "mm_struct addr = 0x%p\n", mm);
	vma = mm->mmap;

	down_read(&mm->mmap_sem);
	SEQ_printf(m, "vmas: vma start end length\n");

	while (vma) {
		nr++;
		start = vma->vm_start;
		end = vma->vm_end;
		length = end - start;
		SEQ_printf(m, "%6d: %16p %12lx %12lx %8ld\n",
				nr, vma, start, end, length);
		vma = vma->vm_next;
	}
	up_read(&mm->mmap_sem);
}

static void mm_show_vma(struct task_struct *p, struct seq_file *m)
{
	SEQ_printf(m, "PID: %d\nCOMM: %s\nthreads: %d\n",
			p->pid,
			p->comm,
			get_nr_threads(p));
	show_task_vma(p, m);
}

#if 0
void proc_task_name(struct seq_file *m, struct task_struct *p, bool escape)
{
	char *buf;
	size_t size;
	char tcomm[64];
	int ret;
#if 0
	if (p->flags & PF_WQ_WORKER)
		wq_worker_comm(tcomm, sizeof(tcomm), p);
	else
#endif
		__get_task_comm(tcomm, sizeof(tcomm), p);


	size = seq_get_buf(m, &buf);
	if (escape) {
		ret = string_escape_str(tcomm, buf, size,
				ESCAPE_SPACE | ESCAPE_SPECIAL, "\n\\");
		if (ret >= size)
			ret = -1;
	} else {
		ret = strscpy(buf, tcomm, size);
	}

	seq_commit(m, ret);
}
#endif

static int proc_mm_show_vma(struct seq_file *m, void *v)
{
	int ret = 0;
	struct pid *kpid;
	struct task_struct *task;
	unsigned long flags;



	spin_lock_irqsave(&mm_show_vma_lock, flags);
	if (PID <= 0) {
		ret = -ESRCH;
		goto out;
	}

	kpid = find_vpid(PID);
	if (kpid == NULL) {
		ret = -ESRCH;
		goto out;
	}

	task = pid_task(kpid, PIDTYPE_PID);
	if (task == NULL) {
		ret = -ESRCH;
		goto out;
	}

	get_task_struct(task);
	mm_show_vma(task, m);
	put_task_struct(task);

out:
	spin_unlock_irqrestore(&mm_show_vma_lock, flags);
	return ret;
}


static int sched_task_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, proc_mm_show_vma, inode);
}


static ssize_t
sched_pid_write(struct file *file, const char __user *buf,
			    size_t count, loff_t *offset)
{
	int ret, pid;
	char srw[MAX_LINE];
	unsigned long flags;

	memset(srw, '\0', sizeof(srw));

	ret = copy_from_user(srw, buf, count);
	if (ret) {
		pr_err("[%s %d] %d\n", __func__, __LINE__, ret);
		return ret;
	}

	ret = kstrtoint(srw, 10, &pid);
	if (ret) {
		pr_err("[%s %d] %d\n", __func__, __LINE__, ret);
		return ret;
	}

	spin_lock_irqsave(&mm_show_vma_lock, flags);
	PID = pid;
	spin_unlock_irqrestore(&mm_show_vma_lock, flags);
	pr_info("PID : %d\n", PID);

	return count;
}

static const struct file_operations show_task_fops = {
	.owner		= THIS_MODULE,
	.open		= sched_task_open,
	.read		= seq_read,
	.write		= sched_pid_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static struct proc_dir_entry *entry;
#define PROC_NAME   "mm_show_vma"

static int __init mm_show_vma_init(void)
{
	entry = proc_create(PROC_NAME, 0664, NULL, &show_task_fops);
	if(entry == NULL) {
		pr_err("Error: Could not initialize /proc/%s\n", PROC_NAME);
		return -ENOMEM;
	}

	pr_info("/proc/%s create\n", PROC_NAME);
	pr_info("init ---------------------!\n");

	return 0;
}

static void __exit mm_show_vma_exit(void)
{
	remove_proc_entry(PROC_NAME, NULL);

	pr_info("/proc/%s removed\n", PROC_NAME);
	pr_info("exit ---------------------!\n");
}

module_init(mm_show_vma_init);
module_exit(mm_show_vma_exit);

MODULE_LICENSE("GPL");
