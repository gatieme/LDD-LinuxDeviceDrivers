/*
 * 评测题目: 编写一个内核模块，获取指定进程的调度信息，包括名字、运行时长、策略、优先级
 **/
/*

v0.9 完成初版功能
v1.0 使用 procfs 方式进行交互
v1.1 split show_task_base/policy/time
v1.2 引入 spinlock 保证互斥.

# 1 testcase
-------

```cpp
#cat test.sh
#!/bin/bash

if [ $# -eq 1 ]; then
PID=$1
else
PID=$$
fi


insmod show_task.ko

echo $PID >  /proc/sched_show_task

cat /proc/sched_show_task

rmmod show_task.ko
```

# 2 How to Use or Test
-------

```cpp
#sh test.sh `pidof yes`

```
PID: 6390
COMM: yes
threads: 1
POLICY: NORMAL CFS(0)
BASE-------------------------------------------------------------------
prio                                         :                  125
se.nr_migrations                             :                   47
nr_switches                                  :                  741
nr_voluntary_switches                        :                    0
nr_involuntary_switches                      :                  741
se.load.weight                               :               343040
TIME(s)-------------------------------------------------------------------
now                                          :      14956139.657933187
se.exec_start                                :      14955774.229852083
se.vruntime                                  :       6789957.542831753
se.sum_exec_runtime                          :           161.523601661
start_time                                   :      14955978.66683773
real_start_time                              :      14955978.66683873
delta_time                                   :           161.591249414
```

# 3 DESCRIPTION
-------

| 字段 | 描述 |
|:----:|:----:|
| prio | 优先级 |
| nr_migrations | 迁移次数 |
| nr_switches | 总切换次数 |
| nr_voluntary_switches | 主动上下文切换次数, 主动睡眠导致的上下文切换, 一般是因为等待资源而退出  |
| nr_involuntary_switches | 被动上下文切换次数, 线程被抢占导致的调度 |
| load.weight | 进程的负载权重 |
| now | 当前的系统时间 |
| exec_start | 当前周期内, 进程开始执行的时间 |
| vruntime | CFS 进程的 vruntime |
| sum_exec_runtime | 进程的累计运行时间 |
| start_time | 进程启动(创建)的时间  |
| real_start_time/start_boottime | 以 bootime 为基准的, 进程启动(创建)的时间 |
| delta_time | 进程创建至今, 经过的时间(不需要一定是 on_cpu/on_rq) 的 |
*/


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
 *  * Ease the printing of nsec fields:
 *   */
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

#define SPLIT_NS(x) nsec_high(x), nsec_low(x)

#define MAX_LINE 100
static int PID = -1;
static DEFINE_SPINLOCK(sched_show_task_lock);
//module_param(PID, int, 0644);

static void show_task_policy(struct task_struct *p, struct seq_file *m)
{
	int policy = p->policy;

	SEQ_printf(m, "POLICY: ");
	switch(policy) {
	case SCHED_FIFO     :
		SEQ_printf(m, "Real-Time FIFO(%d)\n", policy);
		break;
	case SCHED_RR       :
		SEQ_printf(m, "Real-Time RR(%d)\n", policy);
		break;
	case SCHED_DEADLINE :
		SEQ_printf(m, "EDF Deadline(%d)\n", policy);
		break;
	case SCHED_NORMAL   :
		SEQ_printf(m, "NORMAL CFS(%d)\n", policy);
		break;
	case SCHED_BATCH    :
		SEQ_printf(m, "BATCH CFS(%d)\n", policy);
		break;
	case SCHED_IDLE     :
		SEQ_printf(m, "IDLE CFS(%d)\n", policy);
		break;
	default:
		SEQ_printf(m, "Unknown POLICY(%d)\n", policy);
		break;
	}
}

/* see fill_stats_for_tgid */
static void show_task_time(struct task_struct *p, struct seq_file *m)
{
	u64 now, delta_time;

	SEQ_printf(m,
			"TIME(s)---------------------------------------------------------"
			"----------\n");

	now = ktime_get_ns();
	__PN(now);
	PN(se.exec_start);
	PN(se.vruntime);
	PN(se.sum_exec_runtime);

	PN(start_time);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0))
	/*
	 * cf25e24db61cc time: Rename tsk->real_start_time to ->start_boottime
	 */
	PN(start_boottime);
#else
	PN(real_start_time);
#endif

	/* calculate task elapsed time in nsec */
	delta_time = now - p->start_time;
	/* Convert to seconds */
	__PN(delta_time); /* S */
}

static void show_task_base(struct task_struct *p, struct seq_file *m)
{
	unsigned long nr_switches = p->nvcsw + p->nivcsw;

	SEQ_printf(m,
			"BASE---------------------------------------------------------"
			"----------\n");
	P(prio);
	P(se.nr_migrations);

	__P(nr_switches);
	__PS("nr_voluntary_switches", p->nvcsw);
	__PS("nr_involuntary_switches", p->nivcsw);

	P(se.load.weight);
#ifdef CONFIG_UCLAMP_TASK
	__PS("uclamp.min", p->uclamp_req[UCLAMP_MIN].value);
	__PS("uclamp.max", p->uclamp_req[UCLAMP_MAX].value);
	__PS("effective uclamp.min", uclamp_eff_value(p, UCLAMP_MIN));
	__PS("effective uclamp.max", uclamp_eff_value(p, UCLAMP_MAX));
#endif
}

static void sched_show_task(struct task_struct *p, struct seq_file *m)
{
	SEQ_printf(m, "PID: %d\nCOMM: %s\nthreads: %d\n",
			p->pid,
			p->comm,
			get_nr_threads(p));
	show_task_policy(p, m);
	show_task_base(p, m);
	show_task_time(p, m);
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

static int proc_sched_show_task(struct seq_file *m, void *v)
{
	int ret = 0;
	struct pid *kpid;
	struct task_struct *task;
	unsigned long flags;



	spin_lock_irqsave(&sched_show_task_lock, flags);
	if (PID <= 0) {
		ret = -ESRCH;
		goto out;
	}
#if 0
	/* Not Support idle task */
	if (PID == 0) {
		task = idle_task(smp_processor_id()); /* use idle_task of this_rq */
#endif
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
	sched_show_task(task, m);
	put_task_struct(task);

out:
	spin_unlock_irqrestore(&sched_show_task_lock, flags);
	return ret;
}


static int sched_task_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, proc_sched_show_task, inode);
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

	spin_lock_irqsave(&sched_show_task_lock, flags);
	PID = pid;
	spin_unlock_irqrestore(&sched_show_task_lock, flags);
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
#define PROC_NAME   "sched_show_task"

static int __init sched_show_task_init(void)
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

static void __exit sched_show_task_exit(void)
{
	remove_proc_entry(PROC_NAME, NULL);

	pr_info("/proc/%s removed\n", PROC_NAME);
	pr_info("exit ---------------------!\n");
}

module_init(sched_show_task_init);
module_exit(sched_show_task_exit);

MODULE_LICENSE("GPL");
