Linux用户抢占和内核抢占以及抢占发生的时机
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |


前面我们了解了linux进程调度器的设计思路和注意框架

周期调度器scheduler_tick通过linux定时器周期性的被激活, 进行程序调度

进程主动放弃CPU或者发生阻塞时, 则会调用主调度器schedule进行程序调度

在分析的过程中, 我们提到了内核抢占和用户抢占的概念, 但是并没有详细讲, 因此我们在这里详细分析一下子



#前景回顾
-------

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
```c
stop_sched_class -> dl_sched_class -> rt_sched_class -> fair_sched_class -> idle_sched_class
```

**3个调度实体**

调度器不限于调度进程, 还可以调度更大的实体, 比如实现组调度.

这种一般性要求调度器不直接操作进程, 而是处理可调度实体, 因此需要一个通用的数据结构描述这个调度实体,即seched_entity结构, 其实际上就代表了一个调度对象，可以为一个进程，也可以为一个进程组.

linux中针对当前可调度的实时和非实时进程, 定义了类型为seched_entity的3个调度实体

*	sched_dl_entity 采用EDF算法调度的实时调度实体

*	sched_rt_entity 采用Roound-Robin或者FIFO算法调度的实时调度实体 

*	sched_entity 采用CFS算法调度的普通非实时进程的调度实体


##主调度器与内核/用户抢占
-------

###调度过程中关闭内核抢占
-------

我们在上一篇linux内核主调度器schedule(文章链接, [CSDN](), [Github]())中在分析主调度器的时候, 我们会发现内核在进行调度之前都会通过preempt_disable关闭内核抢占, 而在完成调度工作后, 又会重新开启**内核抢占**

参见[主调度器函数schedule](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L3243)
```c
    do {
        preempt_disable();                                  /*  关闭内核抢占  */
        __schedule(false);                                  /*  完成调度  */
        sched_preempt_enable_no_resched();                  /*  开启内核抢占  */

    } while (need_resched());   /*  如果该进程被其他进程设置了TIF_NEED_RESCHED标志，则函数重新执行进行调度    */
```

这个很容易理解, 我们在内核完成调度器过程中, 这时候如果发生了内核抢占, 我们的调度会被中断, 而调度却还没有完成, 这样会丢失我们调度的信息.

###调度完成检查need_resched看是否需要重新调度

而同样我们可以看到, 在调度完成后, 内核会去判断need_resched条件, 如果这个时候为真, 内核会重新进程一次调度.

这个的原因, 我们在前一篇博客中, 也已经说的很明白了,

内核在thread_info的flag中设置了一个标识来标志进程是否需要重新调度, 即重新调度need_resched标识TIF_NEED_RESCHED, 内核在即将返回用户空间时会检查标识TIF_NEED_RESCHED标志进程是否需要重新调度，如果设置了，就会发生调度, 这被称为**用户抢占**



