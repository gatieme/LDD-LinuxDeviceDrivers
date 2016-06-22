Linux进程核心调度器之主调度器
=======





| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |



我们前面提到linux有两种方法激活调度器：核心调度器和

*	一种是直接的, 比如进程打算睡眠或出于其他原因放弃CPU

*	另一种是通过周期性的机制, 以固定的频率运行, 不时的检测是否有必要


因而内核提供了两个调度器**主调度器**，**周期性调度器**，分别实现如上工作, 两者合在一起就组成了**核心调度器(core scheduler)**, 也叫**通用调度器(generic scheduler)**.



他们都根据进程的优先级分配CPU时间, 因此这个过程就叫做优先调度, 我们将在本节主要讲解周期调度的设计和实现方式


在内核中的许多地方, 如果要将CPU分配给与当前活动进程不同的另一个进程, 都会直接调用主调度器函数schedule, 从系统调用返回后, 内核也会检查当前进程是否设置了重调度标志TLF_NEDD_RESCHED


#前景回顾
-------


##进程调度
-------


内存中保存了对每个进程的唯一描述, 并通过若干结构与其他进程连接起来.


**调度器**面对的情形就是这样, 其任务是在程序之间共享CPU时间, 创造并行执行的错觉, 该任务分为两个不同的部分, 其中一个涉及**调度策略**, 另外一个涉及**上下文切换**.




##进程的分类
-------

linux把进程区分为**实时进程**和**非实时进程**, 其中非实时进程进一步划分为交互式进程和批处理进程

根据进程的不同分类Linux采用不同的调度策略.

对于实时进程，采用FIFO, Round Robin或者Earliest Deadline First (EDF)最早截止期限优先调度算法|的调度策略.

但是普通进程的调度策略就比较麻烦了, 因为普通进程不能简单的只看优先级, 必须公平的占有CPU, 否则很容易出现进程饥饿, 这种情况下用户会感觉操作系统很卡, 响应总是很慢，因此在linux调度器的发展历程中经过了多次重大变动, linux总是希望寻找一个最接近于完美的调度策略来公平快速的调度进程.


##linux调度器的演变
-------



一开始的调度器是复杂度为**$O(n)$的始调度算法**(实际上每次会遍历所有任务，所以复杂度为O(n)), 这个算法的缺点是当内核中有很多任务时，调度器本身就会耗费不少时间，所以，从linux2.5开始引入赫赫有名的**$O(1)$调度器**

然而，linux是集全球很多程序员的聪明才智而发展起来的超级内核，没有最好，只有更好，在$O(1)$调度器风光了没几天就又被另一个更优秀的调度器取代了，它就是**CFS调度器Completely Fair Scheduler**.

| 字段 | 版本 |
| ------------- |:-------------:|
| O(n)的始调度算法 | linux-0.11~2.4 |
| O(1)调度器 | linux-2.5 |
| CFS调度器 | linux-2.6~至今 |


##Linux的调度器组成
-------


**2个调度器**

可以用两种方法来激活调度

*	一种是直接的, 比如进程打算睡眠或出于其他原因放弃CPU

*	另一种是通过周期性的机制, 以固定的频率运行, 不时的检测是否有必要

因此当前linux的调度程序由两个调度器组成：**主调度器**，**周期性调度器**(两者又统称为**通用调度器(generic scheduler)**或**核心调度器(core scheduler)**)

并且每个调度器包括两个内容：**调度框架**(其实质就是两个函数框架)及**调度器类**



**6种调度策略**

linux内核目前实现了6中调度策略(即调度算法), 用于对不同类型的进程进行调度, 或者支持某些特殊的功能

*	SCHED_NORMAL和SCHED_BATCH调度普通的非实时进程

*	SCHED_FIFO和SCHED_RR和SCHED_DEADLINE则采用不同的调度策略调度实时进程

*	SCHED_IDLE则在系统空闲时调用idle进程.



**5个调度器类**

而依据其调度策略的不同实现了5个调度器类, 一个调度器类可以用一种种或者多种调度策略调度某一类进程, 也可以用于特殊情况或者调度特殊功能的进程.


其所属进程的优先级顺序为
````c
stop_sched_class -> dl_sched_class -> rt_sched_class -> fair_sched_class -> idle_sched_class
```

**3个调度实体**

调度器不限于调度进程, 还可以调度更大的实体, 比如实现组调度.

这种一般性要求调度器不直接操作进程, 而是处理可调度实体, 因此需要一个通用的数据结构描述这个调度实体,即seched_entity结构, 其实际上就代表了一个调度对象，可以为一个进程，也可以为一个进程组.

linux中针对当前可调度的实时和非实时进程, 定义了类型为seched_entity的3个调度实体

*	sched_dl_entity 采用EDF算法调度的实时调度实体

*	sched_rt_entity 采用Roound-Robin或者FIFO算法调度的实时调度实体 rt_sched_class

*	sched_entity 采用CFS算法调度的普通非实时进程的调度实体



**调度器类的就绪队列**

另外，对于调度框架及调度器类，它们都有自己管理的运行队列，调度框架只识别rq（其实它也不能算是运行队列），而对于cfs调度器类它的运行队列则是cfs_rq（内部使用红黑树组织调度实体），实时rt的运行队列则为rt_rq（内部使用优先级bitmap+双向链表组织调度实体）, 此外内核对新增的dl实时调度策略也提供了运行队列dl_rq


**调度器整体框架**

每个进程都属于某个调度器类(由字段task_struct->sched_class标识), 由调度器类采用进程对应的调度策略调度(由task_struct->policy )进行调度, task_struct也存储了其对应的调度实体标识

linux实现了6种调度策略, 依据其调度策略的不同实现了5个调度器类, 一个调度器类可以用一种或者多种调度策略调度某一类进程, 也可以用于特殊情况或者调度特殊功能的进程.


| 调度器类 | 调度策略 |  调度策略对应的调度算法 | 调度实体 | 调度实体对应的调度对象 |
| ------- |:-------:|:-------:|:-------:||:-------:|
| stop_sched_class | 无 | 无 | 无 | 特殊情况, 发生在cpu_stop_cpu_callback 进行cpu之间任务迁移migration或者HOTPLUG_CPU的情况下关闭任务 |
| dl_sched_class | SCHED_DEADLINE | Earliest-Deadline-First最早截至时间有限算法 | sched_dl_entity | 采用DEF最早截至时间有限算法调度实时进程 |
| rt_sched_class | SCHED_RR<br><br>SCHED_FIFO | Roound-Robin时间片轮转算法<br><br>FIFO先进先出算法 | sched_rt_entity | 采用Roound-Robin或者FIFO算法调度的实时调度实体 |
| fair_sched_class | SCHED_NORMAL<br><br>SCHED_BATCH | CFS完全公平懂调度算法 |sched_entity | 采用CFS算法普通非实时进程 |
| idle_sched_class | SCHED_IDLE | 无 | 无 |特殊进程, 用于cpu空闲时调度空闲进程idle |

它们的关系如下图

![调度器的组成](../images/level.jpg)


#主调度器
-------

在内核中的许多地方, 如果要将CPU分配给与当前活动进程不同的另一个进程, 都会直接调用主调度器函数schedule, 从系统调用返回后, 内核也会检查当前进程是否设置了重调度标志TLF_NEDD_RESCHED

例如, 前述的周期性调度器的scheduler_tick就会设置该标志, 如果是这样则内核会调用schedule, 该函数假定当前活动进程一定会被另一个进程取代.

##调度函数的__sched前缀

在详细论述schedule之前, 需要说明一下__sched前缀, 该前缀可能用于调用schedule的函数, 包括schedule本身.

__sched前缀的声明, 在[include/linux/sched.h, L416](http://lxr.free-electrons.com/source/include/linux/sched.h?v=4.6#L416), 如下所示

``` c
/* Attach to any functions which should be ignored in wchan output. */
#define __sched         __attribute__((__section__(".sched.text")))
```

__attribute__((__section_("...")))是一个gcc的编译属性, 其目的在于将相关的函数的代码编译之后, 放到目标文件的以恶搞特定的段内, 即.sched.text中. 该信息使得内核在显示栈转储活类似信息时, 忽略所有与调度相关的调用. 由于调度哈书调用不是普通代码流程的一部分, 因此在这种情况下是没有意义的.



用它修饰函数的方式如下

```c
void __sched some_function(args, ...)
{
	......
	schedule();
    ......
}
```

##schedule函数
-------


schedule就是主调度器的函数, 在内核中的许多地方, 如果要将CPU分配给与当前活动进程不同的另一个进程, 都会直接调用主调度器函数schedule

该函数完成如下工作

1.	确定当前就绪队列, 并在prev中保存一个指向(仍然)活动进程的task_struct指针


该函数定义在[kernel/sched/core.c, L3243](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L3243), 如下所示

```c
asmlinkage __visible void __sched schedule(void)
{

	/*  获取当前的进程  */
    struct task_struct *tsk = current;

    /*  避免死锁 */
    sched_submit_work(tsk);
    do {
        preempt_disable();										/*  关闭内核抢占  */
        __schedule(false);										  /*  完成调度  */
        sched_preempt_enable_no_resched();	     /*  开启内核抢占  */
    } while (need_resched());
}
EXPORT_SYMBOL(schedule);
```

##sched_submit_work避免死锁
-------


该函数定义在[kernel/sched/core.c, L3231](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L3231), 如下所示


```c
static inline void sched_submit_work(struct task_struct *tsk)
{
	/*  检测tsk->state是否为0 （runnable）, 若为运行态时则返回，
	 *	 tsk_is_pi_blocked(tsk),检测tsk的死锁检测器是否为空，若非空的话就return
    if (!tsk->state || tsk_is_pi_blocked(tsk))
        return;
    /*
     * If we are going to sleep and we have plugged IO queued,
     * make sure to submit it to avoid deadlocks.
     */
    if (blk_needs_flush_plug(tsk))  /*  然后检测是否需要刷新plug队列，用来避免死锁  */
        blk_schedule_flush_plug(tsk);
}
```

##preempt_disable和sched_preempt_enable_no_resched开关内核抢占
-------

**内核抢占**

Linux除了内核态外还有用户态。用户程序的上下文属于用户态，系统调用和中断处理例程上下文属于内核态. 如果一个进程在用户态时被其他进程抢占了COU则成发生了用户态抢占, 而如果此时进程进入了内核态, 则内核此时代替进程执行, 如果此时发了抢占, 我们就说发生了内核抢占.


内核抢占是Linux 2.6以后引入的一个重要的概念

我们说：如果进程正执行内核函数时，即它在内核态运行时，允许发生内核切换(被替换的进程是正执行内核函数的进程)，这个内核就是抢占的。

抢占内核的主要特点是：一个在内核态运行的进程，当且仅当在执行内核函数期间被另外一个进程取代。

这与用户态的抢占有本质区别.


内核为了支撑内核抢占, 提供了很多机制和结构, 必要时候开关内核抢占也是必须的, 这些函数定义在[include/linux/preempt.h, L145](http://lxr.free-electrons.com/source/include/linux/preempt.h?v=4.6#L145)


```c
#define preempt_disable() \
do { \
    preempt_count_inc(); \
    barrier(); \
} while (0)

#define sched_preempt_enable_no_resched() \
do { \
    barrier(); \
    preempt_count_dec(); \
} while (0)
```

## __schedule开始进程调度
-------

__schedule完成了真正的调度工作, 其定义在[kernel/sched/core.c, L3103](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L3103), 如下所示

```c

```

