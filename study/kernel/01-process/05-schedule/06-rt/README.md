Linux实时进程调度
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-07-12 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/details/51456569) |



按照POSIX的标准的强制要求, 除了"普通"进程之外, Linux还支持两种实时调度类.

调度器结构使得实时进程可以平滑集成到内核中, 而无需修改核心调度器, 这显然是调度类带来的好处.

实时进程与普通进程有一个根本的不同之处: 如果系统中有一个实时进程且可运行, 那么调度器总是会选中它运行, 除非有另外一个优先级更高的实时进程.

现有的两种实时类, 不同之处如下所示:

| 名称 | 调度策略 | 描述 |
|:-------:|:-------:|:-------:|
| 循环进程 | SCHED_RR | 有时间片, 其值在进程运行时会减少, 就像是普通进程. 在所有的时间段到期后, 则该值重置为初始值, 而进程则置与队列的末尾, 这保证了在有几个相同优先级相同的SCHED_RR进程的情况下, 他们总是依次执行 |
| 先进先出进程 | SCHED_FIFO | 没有时间片, 在被调度器选择执行后, 可以运行任意长时间. |

很明显, 如果实时进程编写的比较差, 系统可能长时间无法使用. 最简单的例子, 只要写一个无限循环, 循环体内不进入休眠即可. 因而我们在编写实时应用程序时, 应该格外小心.


#实时进程的数据结构
-------

##实时调度器类
-------

实时进程的调度器类用rt_sched_class来表示, 其定义在[kernel/sched/rt.c, line 2326](http://lxr.free-electrons.com/source/kernel/sched/rt.c?v=4.6#L2326)中, 如下所示.



```c
const struct sched_class rt_sched_class = 
{
    .next           = &fair_sched_class,
    .enqueue_task       = enqueue_task_rt,
    .dequeue_task       = dequeue_task_rt,
    .yield_task         = yield_task_rt,

    .check_preempt_curr     = check_preempt_curr_rt,

    .pick_next_task     = pick_next_task_rt,
    .put_prev_task      = put_prev_task_rt,

#ifdef CONFIG_SMP
    .select_task_rq     = select_task_rq_rt,

    .set_cpus_allowed       = set_cpus_allowed_common,
    .rq_online          = rq_online_rt,
    .rq_offline         = rq_offline_rt,
    .task_woken         = task_woken_rt,
    .switched_from      = switched_from_rt,
#endif

    .set_curr_task      = set_curr_task_rt,
    .task_tick          = task_tick_rt,

    .get_rr_interval    = get_rr_interval_rt,

    .prio_changed       = prio_changed_rt,
    .switched_to        = switched_to_rt,

    .update_curr        = update_curr_rt,
};
```
实时调度器类的实现比完全调度器简单. 

##实时进程的优先级
-------


实时进程的特点在于其优先级比普通进程高, 对应地, 其static_prio值总是普通进程低.

task_has_rt_policy则检测进程是否关联到实时进程策略.


**linux优先级的表示**


>在用户空间通过nice命令设置进程的静态优先级, 这在内部会调用nice系统调用, 进程的nice值在-20~+19之间. 值越低优先级越高.
>
>setpriority系统调用也可以用来设置进程的优先级. 它不仅能够修改单个线程的优先级, 还能修改进程组中所有进程的优先级, 或者通过制定UID来修改特定用户的所有进程的优先级


内核使用一些简单的数值范围0~139表示内部优先级, 数值越低, 优先级越高。

从0~99的范围专供实时进程使用, nice的值[-20,19]则映射到范围100~139, 即普通进程的优先级




| 优先级范围 | 描述 |
| ------------- |:-------------:|
| 0——99 | 实时进程 |
| 100——139 | 非实时进程 |

![内核的优先级标度](../images/priority.jpg)



内核表示优先级的所有信息基本都放在[include/linux/sched/prio.h](http://lxr.free-electrons.com/source/include/linux/sched/prio.h?v=4.6)中, 其中定义了一些表示优先级的宏和函数.


而MAX_RT_PRIO - 1 = 99指定了实时进程的最大优先级, 而MAX_PRIO - 1 = 139则是普通进程的最大优先级数值

```c
//  http://lxr.free-electrons.com/source/include/linux/sched/prio.h?v=4.6#L22
#define MAX_USER_RT_PRIO        100
#define MAX_RT_PRIO             MAX_USER_RT_PRIO

#define MAX_PRIO                (MAX_RT_PRIO + NICE_WIDTH)
#define DEFAULT_PRIO            (MAX_RT_PRIO + NICE_WIDTH / 2)
```


| 宏 | 值 | 描述 |
| ------------- |:-------------:|:-------------:|
| MIN_NICE | -20 | 对应于优先级100, 可以使用NICE_TO_PRIO和PRIO_TO_NICE转换 |
| MAX_NICE |  19 | 对应于优先级139, 可以使用NICE_TO_PRIO和PRIO_TO_NICE转换 |
| NICE_WIDTH | 40 | nice值得范围宽度, 即[-20, 19]共40个数字的宽度 |
| MAX_RT_PRIO, MAX_USER_RT_PRIO | 100 | 实时进程的最大优先级 |
| MAX_PRIO | 140 | 普通进程的最大优先级 |
| DEFAULT_PRIO | 120 | 进程的默认优先级, 对应于nice=0 |
| MAX_DL_PRIO | 0 | 使用EDF最早截止时间优先调度算法的实时进程最大的优先级 |
`


##实时就绪队列
-------

核心调度器的就绪队列也包含了用于实时进程的子就绪队列, 是一个嵌入的struct rt_rq实例.

参见[struct rq的定义](http://lxr.free-electrons.com/source/kernel/sched/sched.h?v4.6#L602)


```c
struct rq
{
	/*  ...... */
    struct rt_rq rt;
	/*  ...... */
}
```
实时rt进程的就绪队列定义在[kernel/sched/sched.h, line 450](http://lxr.free-electrons.com/source/kernel/sched/sched.h?v=4.6#L450)中

```c
/* Real-Time classes' related field in a runqueue: */
struct rt_rq {
    struct rt_prio_array active;
    unsigned int rt_nr_running;
    unsigned int rr_nr_running;
#if defined CONFIG_SMP || defined CONFIG_RT_GROUP_SCHED
    struct {
        int curr; /* highest queued rt task prio */
#ifdef CONFIG_SMP
        int next; /* next highest */
#endif
    } highest_prio;
#endif
#ifdef CONFIG_SMP
    unsigned long rt_nr_migratory;
    unsigned long rt_nr_total;
    int overloaded;
    struct plist_head pushable_tasks;
#ifdef HAVE_RT_PUSH_IPI
    int push_flags;
    int push_cpu;
    struct irq_work push_work;
    raw_spinlock_t push_lock;
#endif
#endif /* CONFIG_SMP */
    int rt_queued;

    int rt_throttled;
    u64 rt_time;
    u64 rt_runtime;
    /* Nests inside the rq lock: */
    raw_spinlock_t rt_runtime_lock;

#ifdef CONFIG_RT_GROUP_SCHED
    unsigned long rt_nr_boosted;

    struct rq *rq;
    struct task_group *tg;
#endif
```

其主要包含了rt_prio_array



```c
/*
 * This is the priority-queue data structure of the RT scheduling class:
 */
struct rt_prio_array {
    DECLARE_BITMAP(bitmap, MAX_RT_PRIO+1); /* include 1 bit for delimiter */
    struct list_head queue[MAX_RT_PRIO];
};
```
具有相同优先级的所有实时进程都保存在一个链表中, 表头为active.queue[prio], 而active.bitmap位图中每个比特位对应于一个链表, 凡包含了进程的链表, 对应的比特位置位. 如果连表中没有进程, 则对应的比特位不置位.




##实时进程的判断
-------


内核可以通过rt_task宏可以通过检查其优先级来检测给定进程是否是实时进程, 该函数定义在[include/linux/sched/rt.h, line 13](http://lxr.free-electrons.com/source/include/linux/sched/rt.h?v=4.6#L13)


```c
//  http://lxr.free-electrons.com/source/include/linux/sched/rt.h?v4.6#L6
static inline int rt_prio(int prio)
{
	if (unlikely(prio < MAX_RT_PRIO))
		return 1;
	return 0;
}

static inline int rt_task(struct task_struct *p)
{
	return rt_prio(p->prio);
}
```