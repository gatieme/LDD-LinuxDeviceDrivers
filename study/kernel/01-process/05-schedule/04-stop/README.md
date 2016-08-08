Linux进程调度之stop调度器类与stop_machine机制
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |



所属调度器类为stop_sched_class的进程是系统中优先级最高的进程, 其次才是dl_shced_class和rt_sched_class


stop_sched_class用于停止CPU, 一般在SMP系统上使用， 用以实现负载平衡和CPU热插拔. 这个类有最高的调度优先级, 
如果你的系统没有定义CONFIG_SMP. 你可以试着将此类移除.


stop调度器类实现了Unix的stop_machine 特性(根据UNIX 风格，也可能是等效的其他特性)准备拼接新代码。

stop_machine 是一个通信信号	:	在SMP的情况下相当于暂时停止其他的CPU的运行, 它让一个 CPU 继续运行，而让所有其他CPU空闲. 在单CPU的情况下这个东西就相当于关中断

我的理解是如果Mulit CPU共享的东西需要修改, 且无法借助OS的lock, 关中断等策略来实现这一功能, 则需要stop_machine





#1		stop调度器类stop_sched_class
-------

stop调度器类是优先级最高的调度器类, [kernel/sched/stop_task.c](http://lxr.free-electrons.com/source/kernel/sched/stop_task.c?v=4.7#L112), 

```cpp
/*
 * Simple, special scheduling class for the per-CPU stop tasks:
 */
const struct sched_class stop_sched_class = {
    .next           = &dl_sched_class,

    .enqueue_task       = enqueue_task_stop,
    .dequeue_task       = dequeue_task_stop,
    .yield_task         = yield_task_stop,

    .check_preempt_curr     = check_preempt_curr_stop,

    .pick_next_task     = pick_next_task_stop,
    .put_prev_task      = put_prev_task_stop,

#ifdef CONFIG_SMP
    .select_task_rq     = select_task_rq_stop,
    .set_cpus_allowed       = set_cpus_allowed_common,
#endif

    .set_curr_task      = set_curr_task_stop,
    .task_tick          = task_tick_stop,

    .get_rr_interval    = get_rr_interval_stop,

    .prio_changed       = prio_changed_stop,
    .switched_to        = switched_to_stop,
    .update_curr        = update_curr_stop,
};
```


内核提供了sched_set_stop_task函数用来将某个进程stop的调度器类设置为stop_sched_class, 该函数定义在[/kernel/sched/core.c, line 849](http://lxr.free-electrons.com/source/kernel/sched/core.c#L849)
```cpp
void sched_set_stop_task(int cpu, struct task_struct *stop)
{
    struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };
    struct task_struct *old_stop = cpu_rq(cpu)->stop;			/*  获取到cpu上之前的stop进程  */

    if (stop)
    {
        /*
         * Make it appear like a SCHED_FIFO task, its something
         * userspace knows about and won't get confused about.
         *
         * Also, it will make PI more or less work without too
         * much confusion -- but then, stop work should not
         * rely on PI working anyway.
         */
        sched_setscheduler_nocheck(stop, SCHED_FIFO, &param);  /*  使用SCHED_FIFO策略设置stop进程的调度信息  */

        stop->sched_class = &stop_sched_class;		/*  设置stop进程的调度器类为stop_sched_class  */
    }

    cpu_rq(cpu)->stop = stop;	/*  设置cpu的运行队列的stop进程为设置好的struct task_struct *stop */

    if (old_stop)						   /*  如果cpu的运行队列上之前有stop进程  */
    {
        /*
         * Reset it back to a normal scheduling class so that
         * it can die in pieces.
         */
        old_stop->sched_class = &rt_sched_class;	/*  恢复cpu运行队列上之前的stop进程的调度器类为rt_sched_class  */
    }
}
```


#2	stop_machine机制
-------


内核中很少有地方使用了stop_sched_class, 因为这个调度器类并不像dl_shced_class, rt_sched_class和fair_sched_class一样直接调度进程

相反它用于完成stop_machine机制, 有关stop_machine机制的实现都在[include/linux/stop_machine.h, line 120](http://lxr.free-electrons.com/source/include/linux/stop_machine.h#L120)和[kernel/stop_machine.c?v=4.7, line 482](http://lxr.free-electrons.com/source/kernel/stop_machine.c?v=4.7#L482)


##2.1	cpu_stop_work
-------

struct cpu_stop_work是用以完成stop_machine工作的任务实体信息, 他在SMP和非SMP结构下有不同的定义, 参见[include/linux/stop_machine.h?v=4.7, line 23](http://lxr.free-electrons.com/source/include/linux/stop_machine.h?v=4.7#L23)

```cpp
#ifdef CONFIG_SMP

#ifdef CONFIG_SMP

struct cpu_stop_work {
        struct list_head        list;           	    /* cpu_stopper->works */
        cpu_stop_fn_t           fn;			  	   /*  stop进程的工作函数  */
        void                    *arg;			       /*  stop进程工作函数的参数信息  */
        struct cpu_stop_done    *done;	 			 /*  额外女巫的完成情况, 包括返回值等信息  */
};


#else   /* CONFIG_SMP */

#include <linux/workqueue.h>

struct cpu_stop_work {
        struct work_struct      work;
        cpu_stop_fn_t           fn;
        void                    *arg;
};
```

##2.2	stop_one_cpu
-------

在非SMP系统中, 使用stop_one_cpu等一组函数来停止一个CPU的工作, 其实质相当于关中断, 定义在[include/linux/stop_machine.h?v=4.7](http://lxr.free-electrons.com/source/include/linux/stop_machine.h?v=4.7#L49)

| 函数 | 描述 |
|:-------:|:-------:|
|  stop_one_cpu | 停止CPU工作, 关闭中断, 并执行fn(arg)函数 |
|  stop_one_cpu_nowait_workfn | 开始一个任务来完成fn(arg)的工作, 而该函数无需等待fn工作的完成 |
| stop_one_cpu_nowait | 关闭中断, 并执行fn(arg)函数, 但不等待其完成 |
|  stop_cpus | 同stop_one_cpu |
|  try_stop_cpus | 同stop_cpus |


下面我们列出了, stop_one_cpu函数的实现, 以供参考 定义在[include/linux/stop_machine.h?v=4.7, line 49](http://lxr.free-electrons.com/source/include/linux/stop_machine.h?v=4.7#L49)

```cpp
static inline int stop_one_cpu(unsigned int cpu, cpu_stop_fn_t fn, void *arg)
{
        int ret = -ENOENT;
        preempt_disable();
        if (cpu == smp_processor_id())
                ret = fn(arg);
        preempt_enable();
        return ret;
}
```

在SMP系统中, 则实现了如下函数, 声明在[include/linux/stop_machine.h?v=4.7, line 30](http://lxr.free-electrons.com/source/include/linux/stop_machine.h?v=4.7#L30), 定义在[kernel/stop_machine.c?v=4.7, line 120](http://lxr.free-electrons.com/source/kernel/stop_machine.c?v=4.7#L120)

```cpp
int stop_one_cpu(unsigned int cpu, cpu_stop_fn_t fn, void *arg);
int stop_two_cpus(unsigned int cpu1, unsigned int cpu2, cpu_stop_fn_t fn, void *arg);
bool stop_one_cpu_nowait(unsigned int cpu, cpu_stop_fn_t fn, void *arg,
                         struct cpu_stop_work *work_buf);
int stop_cpus(const struct cpumask *cpumask, cpu_stop_fn_t fn, void *arg);
int try_stop_cpus(const struct cpumask *cpumask, cpu_stop_fn_t fn, void *arg);
```


下面是stop_one_cpu函数的smp实现

```cpp
int stop_one_cpu(unsigned int cpu, cpu_stop_fn_t fn, void *arg)
{
        struct cpu_stop_done done;
        struct cpu_stop_work work = { .fn = fn, .arg = arg, .done = &done };

        cpu_stop_init_done(&done, 1);
        if (!cpu_stop_queue_work(cpu, &work))
                return -ENOENT;
        wait_for_completion(&done.completion);
        return done.ret;
}
```


##2.3	stop_machine
-------


```cpp
#if defined(CONFIG_SMP) || defined(CONFIG_HOTPLUG_CPU)

/*
声明在http://lxr.free-electrons.com/source/include/linux/stop_machine.h?v=4.7#L120
定义在http://lxr.free-electrons.com/source/kernel/stop_machine.c#L565
*/
int stop_machine(cpu_stop_fn_t fn, void *data, const struct cpumask *cpus);

int stop_machine_from_inactive_cpu(cpu_stop_fn_t fn, void *data,
                                   const struct cpumask *cpus);
#else   /* CONFIG_SMP || CONFIG_HOTPLUG_CPU */

static inline int stop_machine(cpu_stop_fn_t fn, void *data,
                                 const struct cpumask *cpus)
{
        unsigned long flags;
        int ret;
        local_irq_save(flags);
        ret = fn(data);
        local_irq_restore(flags);
        return ret;
}

static inline int stop_machine_from_inactive_cpu(cpu_stop_fn_t fn, void *data,
                                                 const struct cpumask *cpus)
{
        return stop_machine(fn, data, cpus);
}
```

#2.4	stop_machine机制的应用
-------

一般来说, 内核会在如下情况下使用stop_machine技术


| 应用 | 描述 |
|:-----:|:------:|
| module install and remove | 增加删除模块, 在不需要重启内核的情况下, 加载和删除模块 |
| cpu hotplug | CPU的热插拔, 用以执行任务迁移的工作, [cpu_stop_threads](http://lxr.free-electrons.com/source/kernel/stop_machine.c?v=4.7#L29), 该任务由CPU绑定的migration内核线程来完成  |
| memory hotplug | Memory的热插拔 |
| ftrace | 内核trace，debug功能, 参见[kernel/trace/ftrace.c](http://lxr.free-electrons.com/source/kernel/trace/ftrace.c?v=4.7#L2571)  |
| hwlat_detector | 检测系统硬件引入的latency，debug功能 |
| Kernel Hotpatch | [Ksplice](http://www.ibm.com/developerworks/cn/aix/library/au-spunix_ksplice/)可以在不到一秒时间里动态地应用内核补丁, 不需要重新引导 |




