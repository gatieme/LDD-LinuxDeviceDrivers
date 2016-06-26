Linux进程调度器的设计
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |


#前景回顾
-------


##进程调度
-------

linux把进程区分为实时进程和非实时进程, 其中非实时进程进一步划分为交互式进程和批处理进程



根据进程的不同分类Linux采用不同的调度策略.

对于实时进程，采用FIFO或者Round Robin的调度策略.

对于普通进程，则采用CFS完全功能公平调度器

##linux调度器的演变
-------

一开始的调度器是复杂度为**$O(n)$的始调度算法**(实际上每次会遍历所有任务，所以复杂度为O(n)), 这个算法的缺点是当内核中有很多任务时，调度器本身就会耗费不少时间，所以，从linux2.5开始引入赫赫有名的**$O(1)$调度器**



| 字段 | 版本 |
| ------------- |:-------------:|
| O(n)的始调度算法 | linux-0.11~2.4 |
| O(1)调度器 | linux-2.5 |
| CFS调度器 | linux-2.6~至今 |


#Linux的调度器组成
-------


##2个调度器
-------

可以用两种方法来激活调度

*	一种是直接的, 比如进程打算睡眠或出于其他原因放弃CPU

*	另一种是通过周期性的机制, 以固定的频率运行, 不时的检测是否有必要

因此当前linux的调度程序由两个调度器组成：**主调度器**，**周期性调度器**(两者又统称为**通用调度器(generic scheduler)**或**核心调度器(core scheduler)**)

并且每个调度器包括两个内容：**调度框架**(其实质就是两个函数框架)及**调度器类**



##6种调度策略
-------


linux内核目前实现了6中调度策略(即调度算法), 用于对不同类型的进程进行调度, 或者支持某些特殊的功能

比如SCHED_NORMAL和SCHED_BATCH调度普通的非实时进程, SCHED_FIFO和SCHED_RR和SCHED_DEADLINE则采用不同的调度策略调度实时进程, SCHED_IDLE则在系统空闲时调用idle进程.




##5个调度器类
-------

而依据其调度策略的不同实现了5个调度器类, 一个调度器类可以用一种种或者多种调度策略调度某一类进程, 也可以用于特殊情况或者调度特殊功能的进程.



其所属进程的优先级顺序为
````c
stop_sched_class -> dl_sched_class -> rt_sched_class -> fair_sched_class -> idle_sched_class
```

##3个调度实体
-------

调度器不限于调度进程, 还可以调度更大的实体, 比如实现组调度: 可用的CPUI时间首先在一半的进程组(比如, 所有进程按照所有者分组)之间分配, 接下来分配的时间再在组内进行二次分配.

这种一般性要求调度器不直接操作进程, 而是处理可调度实体, 因此需要一个通用的数据结构描述这个调度实体,即seched_entity结构, 其实际上就代表了一个调度对象，可以为一个进程，也可以为一个进程组.

linux中针对当前可调度的实时和非实时进程, 定义了类型为seched_entity的3个调度实体


*	sched_dl_entity 采用EDF算法调度的DEADLINE实时调度实体

*	sched_rt_entity	采用Roound-Robin或者FIFO算法调度的实时调度实体

*	sched_entity 采用CFS算法调度的普通非实时进程的调度实体


每个进程都属于某个调度器类(由字段task_struct->sched_class标识), 由调度器类采用进程对应的调度策略调度(由task_struct->policy )进行调度, task_struct也存储了其对应的调度实体标识

linux实现了6种调度策略, 依据其调度策略的不同实现了5个调度器类, 一个调度器类可以用一种或者多种调度策略调度某一类进程, 也可以用于特殊情况或者调度特殊功能的进程.


调度器类, 调度策略, 调度实体他们之间的关系如下表

| 调度器类 | 调度策略 |  调度策略对应的调度算法 | 调度实体 | 调度实体对应的调度对象 |
| ------- |:-------:|:-------:|:-------:||:-------:|
| stop_sched_class | 无 | 无 | 无 | 特殊情况, 发生在cpu_stop_cpu_callback 进行cpu之间任务迁移migration或者HOTPLUG_CPU的情况下关闭任务 |
| dl_sched_class | SCHED_DEADLINE | Earliest-Deadline-First最早截至时间有限算法 | sched_dl_entity | 采用DEF最早截至时间有限算法调度实时进程 |
| rt_sched_class | SCHED_RR<br><br>SCHED_FIFO | Roound-Robin时间片轮转算法<br><br>FIFO先进先出算法 | sched_rt_entity | 采用Roound-Robin或者FIFO算法调度的实时调度实体 |
| fair_sched_class | SCHED_NORMAL<br><br>SCHED_BATCH | CFS完全公平懂调度算法 |sched_entity | 采用CFS算法普通非实时进程 |
| idle_sched_class | SCHED_IDLE | 无 | 无 | 特殊进程, 用于cpu空闲时调度空闲进程idle |

#cfs完全公平调度器
-------

##CFS调度器类fair_sched_class
-------

CFS完全公平调度器的调度器类叫fair_sched_class, 其定义在[kernel/sched/fair.c, line 8521](http://lxr.free-electrons.com/source/kernel/sched/fair.c#L8521), 它是我们熟知的是struct  sched_class调度器类类型, 将我们的CFS调度器与一些特定的函数关联起来

```c
/*
 * All the scheduling class methods:
 */
const struct sched_class fair_sched_class = {
        .next                   = &idle_sched_class,  /*  下个优先级的调度类, 所有的调度类通过next链接在一个链表中*/
        .enqueue_task           = enqueue_task_fair,
        .dequeue_task           = dequeue_task_fair,
        .yield_task             = yield_task_fair,
        .yield_to_task          = yield_to_task_fair,

        .check_preempt_curr     = check_preempt_wakeup,

        .pick_next_task         = pick_next_task_fair,
        .put_prev_task          = put_prev_task_fair,

#ifdef CONFIG_SMP
        .select_task_rq         = select_task_rq_fair,
        .migrate_task_rq        = migrate_task_rq_fair,

        .rq_online              = rq_online_fair,
        .rq_offline             = rq_offline_fair,

        .task_waking            = task_waking_fair,
        .task_dead              = task_dead_fair,
        .set_cpus_allowed       = set_cpus_allowed_common,
#endif

        .set_curr_task          = set_curr_task_fair,
        .task_tick              = task_tick_fair,
        .task_fork              = task_fork_fair,

        .prio_changed           = prio_changed_fair,
        .switched_from          = switched_from_fair,
        .switched_to            = switched_to_fair,

        .get_rr_interval        = get_rr_interval_fair,

        .update_curr            = update_curr_fair,

#ifdef CONFIG_FAIR_GROUP_SCHED
        .task_move_group        = task_move_group_fair,
#endif
};
```

| 成员 | 描述 |
| ------------- |:-------------:|
| enqueue_task | 向就绪队列中添加一个进程, 某个任务进入可运行状态时，该函数将得到调用。它将调度实体（进程）放入红黑树中，并对 nr_running 变量加 1 |
| dequeue_task | 将一个进程从就就绪队列中删除, 当某个任务退出可运行状态时调用该函数，它将从红黑树中去掉对应的调度实体，并从 nr_running 变量中减 1 |
| yield_task | 在进程想要资源放弃对处理器的控制权的时, 可使用在sched_yield系统调用, 会调用内核API yield_task完成此工作. compat_yield sysctl 关闭的情况下，该函数实际上执行先出队后入队；在这种情况下，它将调度实体放在红黑树的最右端 |
| check_preempt_curr | 该函数将检查当前运行的任务是否被抢占。在实际抢占正在运行的任务之前，CFS 调度程序模块将执行公平性测试。这将驱动唤醒式（wakeup）抢占 |
| pick_next_task | 该函数选择接下来要运行的最合适的进程 |
| put_prev_task | 用另一个进程代替当前运行的进程 |
| set_curr_task | 当任务修改其调度类或修改其任务组时，将调用这个函数 |
| task_tick | 在每次激活周期调度器时, 由周期性调度器调用, 该函数通常调用自 time tick 函数；它可能引起进程切换。这将驱动运行时（running）抢占 |
| task_new | 内核调度程序为调度模块提供了管理新任务启动的机会, 用于建立fork系统调用和调度器之间的关联, 每次新进程建立后, 则用new_task通知调度器, CFS 调度模块使用它进行组调度，而用于实时任务的调度模块则不会使用这个函数 |


##cfs的就绪队列
-------

就绪队列是全局调度器许多操作的起点, 但是进程并不是由就绪队列直接管理的, 调度管理是各个调度器的职责, 因此在各个就绪队列中嵌入了特定调度类的子就绪队列(cfs的顶级调度就队列 [struct cfs_rq](http://lxr.free-electrons.com/source/kernel/sched/sched.h?v=4.6#L359), 实时调度类的就绪队列[struct rt_rq](http://lxr.free-electrons.com/source/kernel/sched/sched.h?v=4.6#L449)和deadline调度类的就绪队列[struct dl_rq](http://lxr.free-electrons.com/source/kernel/sched/sched.h?v=4.6#L490)

```c
/* CFS-related fields in a runqueue */
/* CFS调度的运行队列，每个CPU的rq会包含一个cfs_rq，而每个组调度的sched_entity也会有自己的一个cfs_rq队列 */
struct cfs_rq {
	/* CFS运行队列中所有进程的总负载 */
    struct load_weight load;
	/*
     *  nr_running: cfs_rq中调度实体数量
     *  h_nr_running: 只对进程组有效，其下所有进程组中cfs_rq的nr_running之和
	*/
    unsigned int nr_running, h_nr_running;

    u64 exec_clock;
    
	/*
     * 当前CFS队列上最小运行时间，单调递增
     * 两种情况下更新该值: 
     * 1、更新当前运行任务的累计运行时间时
     * 2、当任务从队列删除去，如任务睡眠或退出，这时候会查看剩下的任务的vruntime是否大于min_vruntime，如果是则更新该值。
     */

    u64 min_vruntime;
#ifndef CONFIG_64BIT
    u64 min_vruntime_copy;
#endif
	/* 该红黑树的root */
    struct rb_root tasks_timeline;
     /* 下一个调度结点(红黑树最左边结点，最左边结点就是下个调度实体) */
    struct rb_node *rb_leftmost;

    /*
     * 'curr' points to currently running entity on this cfs_rq.
     * It is set to NULL otherwise (i.e when none are currently running).
	 * curr: 当前正在运行的sched_entity（对于组虽然它不会在cpu上运行，但是当它的下层有一个task在cpu上运行，那么它所在的cfs_rq就把它当做是该cfs_rq上当前正在运行的sched_entity）
     * next: 表示有些进程急需运行，即使不遵从CFS调度也必须运行它，调度时会检查是否next需要调度，有就调度next
     *
     * skip: 略过进程(不会选择skip指定的进程调度)
     */
    struct sched_entity *curr, *next, *last, *skip;

#ifdef  CONFIG_SCHED_DEBUG
    unsigned int nr_spread_over;
#endif

#ifdef CONFIG_SMP
    /*
     * CFS load tracking
     */
    struct sched_avg avg;
    u64 runnable_load_sum;
    unsigned long runnable_load_avg;
#ifdef CONFIG_FAIR_GROUP_SCHED
    unsigned long tg_load_avg_contrib;
#endif
    atomic_long_t removed_load_avg, removed_util_avg;
#ifndef CONFIG_64BIT
    u64 load_last_update_time_copy;
#endif

#ifdef CONFIG_FAIR_GROUP_SCHED
    /*
     *   h_load = weight * f(tg)
     *
     * Where f(tg) is the recursive weight fraction assigned to
     * this group.
     */
    unsigned long h_load;
    u64 last_h_load_update;
    struct sched_entity *h_load_next;
#endif /* CONFIG_FAIR_GROUP_SCHED */
#endif /* CONFIG_SMP */

#ifdef CONFIG_FAIR_GROUP_SCHED
    /* 所属于的CPU rq */
    struct rq *rq;  /* cpu runqueue to which this cfs_rq is attached */

    /*
     * leaf cfs_rqs are those that hold tasks (lowest schedulable entity in
     * a hierarchy). Non-leaf lrqs hold other higher schedulable entities
     * (like users, containers etc.)
     *
     * leaf_cfs_rq_list ties together list of leaf cfs_rq's in a cpu. This
     * list is used during load balance.
     */
    int on_list;
    struct list_head leaf_cfs_rq_list;
    /* 拥有该CFS运行队列的进程组 */
    struct task_group *tg;  /* group that "owns" this runqueue */

#ifdef CONFIG_CFS_BANDWIDTH
    int runtime_enabled;
    u64 runtime_expires;
    s64 runtime_remaining;

    u64 throttled_clock, throttled_clock_task;
    u64 throttled_clock_task_time;
    int throttled, throttle_count;
    struct list_head throttled_list;
#endif /* CONFIG_CFS_BANDWIDTH */
#endif /* CONFIG_FAIR_GROUP_SCHED */
};
```


| 成员 | 描述 |
| ------------- |:-------------:|
| nr_running | 队列上可运行进程的数目
| load | 就绪队列上可运行进程的累计负荷权重 |
| min_vruntime | 跟踪记录队列上所有进程的最小虚拟运行时间. 这个值是实现与就绪队列相关的虚拟时钟的基础 |
| tasks_timeline | 用于在按时间排序的红黑树中管理所有进程 |
| rb_leftmost | 总是设置为指向红黑树最左边的节点, 即需要被调度的进程. 该值其实可以可以通过病例红黑树获得, 但是将这个值存储下来可以减少搜索红黑树花费的平均时间 |
| curr | 当前正在运行的sched_entity（对于组虽然它不会在cpu上运行，但是当它的下层有一个task在cpu上运行，那么它所在的cfs_rq就把它当做是该cfs_rq上当前正在运行的sched_entity |
| next | 表示有些进程急需运行，即使不遵从CFS调度也必须运行它，调度时会检查是否next需要调度，有就调度next |
| skip | 略过进程(不会选择skip指定的进程调度) |

