Linux进程核心调度器之周期性调度器
=======





| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |



我们前面提到linux有两种方法激活调度器：核心调度器和

*	一种是直接的, 比如进程打算睡眠或出于其他原因放弃CPU

*	另一种是通过周期性的机制, 以固定的频率运行, 不时的检测是否有必要


因而内核提供了两个调度器**主调度器**，**周期性调度器**，分别实现如上工作, 两者合在一起就组成了**核心调度器(core scheduler)**, 也叫**通用调度器(generic scheduler)**.



他们都根据进程的优先级分配CPU时间, 因此这个过程就叫做优先调度, 我们将在本节主要讲解核心调度器的设计和优先调度的实现方式.


而我们的周期性调度器以固定的频率激活负责当前进程调度类的周期性调度方法, 以保证系统的并发性

首先还是让我们简单回顾一下子之前的的内容



#前景回顾
-------


##进程调度
-------


内存中保存了对每个进程的唯一描述, 并通过若干结构与其他进程连接起来.


**调度器**面对的情形就是这样, 其任务是在程序之间共享CPU时间, 创造并行执行的错觉, 该任务分为两个不同的部分, 其中一个涉及**调度策略**, 另外一个涉及**上下文切换**.


内核必须提供一种方法, 在各个进程之间尽可能公平地共享CPU时间, 而同时又要考虑不同的任务优先级.


调度器的一般原理是, 按所需分配的计算能力, 向系统中每个进程提供最大的公正性, 或者从另外一个角度上说, 他试图确保没有进程被亏待.



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


#周期性调度器
-------





周期性调度器在scheduler_tick中实现. 如果系统正在活动中, 内核会按照频率HZ自动调用该函数. 如果没有近曾在等待调度, 那么在计算机电力供应不足的情况下, 内核将关闭该调度器以减少能耗. 这对于我们的嵌入式设备或者手机终端设备的电源管理是很重要的.



scheduler_tick函数定义在[kernel/sched/core.c, L2910](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L2910)中, 它有两个主要任务



1.	更新相关统计量

	管理内核中的与整个系统和各个进程的调度相关的统计量. 其间执行的主要操作是对各种计数器+1

2.	激活负责当前进程调度类的周期性调度方法

	检查进程执行的时间是否超过了它对应的ideal_runtime，如果超过了，则告诉系统，需要启动主调度器(schedule)进行进程切换。(注意thread_info:preempt_count、thread_info:flags (TIF_NEED_RESCHED))





```c

/*
 * This function gets called by the timer code, with HZ frequency.
 * We call it with interrupts disabled.
 */

void scheduler_tick(void)
{
    /*  1.  获取当前cpu上的全局就绪队列rq和当前运行的进程curr  */

    /*  1.1 在于SMP的情况下，获得当前CPU的ID。如果不是SMP，那么就返回0  */
    int cpu = smp_processor_id();

    /*  1.2 获取cpu的全局就绪队列rq, 每个CPU都有一个就绪队列rq  */
    struct rq *rq = cpu_rq(cpu);

    /*  1.3 获取就绪队列上正在运行的进程curr  */
    struct task_struct *curr = rq->curr;





    sched_clock_tick();



    /*  2 更新rq上的统计信息, 并执行进程对应调度类的周期性的调度  */

    /*  加锁 */
    raw_spin_lock(&rq->lock);

    /*  2.1 更新rq的当前时间戳.即使rq->clock变为当前时间戳  */
    update_rq_clock(rq);

    /*  2.2 执行当前运行进程所在调度类的task_tick函数进行周期性调度  */
    curr->sched_class->task_tick(rq, curr, 0);

    /*  2.3 更新rq的负载信息,  即就绪队列的cpu_load[]数据
     *  本质是讲数组中先前存储的负荷值向后移动一个位置，
     *  将当前负荷记入数组的第一个位置 */
    update_cpu_load_active(rq);



    /*  2.4 更新cpu的active count活动计数
     *  主要是更新全局cpu就绪队列的calc_load_update*/
    calc_global_load_tick(rq);

    /* 解锁 */
    raw_spin_unlock(&rq->lock);

    /* 与perf计数事件相关 */
    perf_event_task_tick();

#ifdef CONFIG_SMP

     /* 当前CPU是否空闲 */
    rq->idle_balance = idle_cpu(cpu);

    /* 如果到是时候进行周期性负载平衡则触发SCHED_SOFTIRQ */
    trigger_load_balance(rq);

#endif

    rq_last_tick_reset(rq);
}
```



##更新统计量
-------


| 函数 | 描述 | 定义 |
| ------------- |:-------------:||:-------------:|
| update_rq_clock | 处理就绪队列时钟的更新, 本质上就是增加struct rq当前实例的时钟时间戳 | [sched/core.c, L98](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L98)
| update_cpu_load_active | 负责更新就绪队列的cpu_load数组, 其本质上相当于将数组中先前存储的负荷值向后移动一个位置, 将当前就绪队列的符合记入数组的第一个位置. 另外该函数还引入一些取平均值的技巧, 以确保符合数组的内容不会呈现太多的不联系跳读. | [kernel/sched/fair.c, L4641](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L4641) |
| calc_global_load_tick | 跟新cpu的活动计数, 主要是更新全局cpu就绪队列的calc_load_update |  [kernel/sched/loadavg.c, L382](http://lxr.free-electrons.com/source/kernel/sched/loadavg.c?v=4.6#L378) |


##激活进程所属调度类的周期性调度器
-------



由于调度器的模块化结构, 主体工程其实很简单, 在更新统计信息的同时, 内核将真正的调度工作委托给了特定的调度类方法

内核先找到了就绪队列上当前运行的进程curr, 然后调用curr所属调度类sched_class的周期性调度方法task_tick


即

```c
curr->sched_class->task_tick(rq, curr, 0);
```



task_tick的实现方法取决于底层的调度器类, 例如完全公平调度器会在该方法中检测是否进程已经运行了太长的时间, 以避免过长的延迟, 注意此处的做法与之前就的基于时间片的调度方法有本质区别, 旧的方法我们称之为到期的时间片, 而完全公平调度器CFS中则不存在所谓的时间片概念.

目前我们的内核中的3个调度器类`struct sched_entity`,  `struct sched_rt_entity`, 和`struct sched_dl_entity` dl, 我们针对当前内核中实现的调度器类分别列出其周期性调度函数task_tick

| 调度器类 | 描述 | 周期性调度函数 | 定义 |
| ------------- |:-------------:||:-------------:|


如果当前进程希望被重新调度, 那么调度类方法会在task_struct中设置TIF_NEED_RESCHED标志, 以表示该请求, 而内核将会在接下来的适当实际完成此请求.