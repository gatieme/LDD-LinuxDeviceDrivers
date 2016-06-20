Linux进程负荷权重
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |



前面我们纤细的了解了linux下进程优先级的表示以及其计算的方法, 我们了解到linux针对普通进程和实时进程分别使用静态优先级static_prio和实时优先级rt_priority来指定其默认的优先级别, 然后通过normal_prio函数将他们分别转换为普通优先级normal_prio, 最终换算出动态优先级prio, 动态优先级prio才是内核调度时候有限考虑的优先级字段

但是进程的重要性不仅是由优先级指定的, 而且还需要考虑保存在task_struct->se.load的负荷权重, 

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

linux把进程区分为实时进程和非实时进程, 其中非实时进程进一步划分为交互式进程和批处理进程

| 类型 | 描述 | 示例 |
| ------- |:-------:|:-------:|:-------:|
| 交互式进程(interactive process) | 此类进程经常与用户进行交互, 因此需要花费很多时间等待键盘和鼠标操作. 当接受了用户的输入后, 进程必须很快被唤醒, 否则用户会感觉系统反应迟钝 | shell, 文本编辑程序和图形应用程序 |
| 批处理进程(batch process) | 此类进程不必与用户交互, 因此经常在后台运行. 因为这样的进程不必很快相应, 因此常受到调度程序的怠慢 | 程序语言的编译程序, 数据库搜索引擎以及科学计算 |
| 实时进程(real-time process) | 这些进程由很强的调度需要, 这样的进程绝不会被低优先级的进程阻塞. 并且他们的响应时间要尽可能的短 | 视频音频应用程序, 机器人控制程序以及从物理传感器上收集数据的程序|


##不同进程采用不同的调度策略
-------

根据进程的不同分类Linux采用不同的调度策略.

对于实时进程，采用FIFO, Round Robin或者Earliest Deadline First (EDF)最早截止期限优先调度算法|的调度策略.

但是普通进程的调度策略就比较麻烦了, 因为普通进程不能简单的只看优先级, 必须公平的占有CPU, 否则很容易出现进程饥饿, 这种情况下用户会感觉操作系统很卡, 响应总是很慢，因此在linux调度器的发展历程中经过了多次重大变动, linux总是希望寻找一个最接近于完美的调度策略来公平快速的调度进程.


##linux调度器的演变
-------

一开始的调度器是复杂度为**$O(n)$的始调度算法**(实际上每次会遍历所有任务，所以复杂度为O(n)), 这个算法的缺点是当内核中有很多任务时，调度器本身就会耗费不少时间，所以，从linux2.5开始引入赫赫有名的**$O(1)$调度器**

然而，linux是集全球很多程序员的聪明才智而发展起来的超级内核，没有最好，只有更好，在$O(1)$调度器风光了没几天就又被另一个更优秀的调度器取代了，它就是**CFS调度器Completely Fair Scheduler**. 这个也是在2.6内核中引入的，具体为2.6.23，即从此版本开始，内核使用CFS作为它的默认调度器，$O(1)$调度器被抛弃了, 其实CFS的发展也是经历了很多阶段，最早期的楼梯算法(SD), 后来逐步对SD算法进行改进出RSDL(Rotating Staircase Deadline Scheduler), 这个算法已经是"完全公平"的雏形了， 直至CFS是最终被内核采纳的调度器, 它从RSDL/SD中吸取了完全公平的思想，不再跟踪进程的睡眠时间，也不再企图区分交互式进程。它将所有的进程都统一对待，这就是公平的含义。CFS的算法和实现都相当简单，众多的测试表明其性能也非常优越



| 字段 | 版本 |
| ------------- |:-------------:|:-------------:|
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

调度器类是实现了不同调度策略的实例，如 CFS、RT class等。

它们的关系如下图

![调度器的组成](./images/level.jpg)

当前的内核支持两种调度器类（sched_setscheduler系统调用可修改进程的策略）：CFS（公平）、RT（实时）；5种调度策略：SCHED_NORAML（最常见的策略）、SCHED_BATCH（除了不能抢占外与常规任务一样，允许任务运行更长时间，更好地使用高速缓存，适合于成批处理的工作）、SCHED_IDLE（它甚至比nice 19还有弱，为了避免优先级反转使用）和SCHED_RR（循环调度，拥有时间片，结束后放在队列末）、SCHED_FIFO（没有时间片，可以运行任意长的时间）；其中前面三种策略使用的是cfs调度器类，后面两种使用rt调度器类。


##优先级的内核表示
-------


内核使用一些简单的数值范围0~139表示内部优先级, 数值越低, 优先级越高。

从0~99的范围专供实时进程使用, nice的值[-20,19]则映射到范围100~139


>实时优先级范围是0到MAX_RT_PRIO-1（即99），而普通进程的静态优先级范围是从MAX_RT_PRIO到MAX_PRIO-1（即100到139）。

| 优先级范围 | 描述 |
| ------------- |:-------------:|
| 0——99 | 实时进程 |
| 100——139 | 非实时进程 |



##进程的优先级表示
-------


```c
struct task_struct
{
    /* 进程优先级
     * prio: 动态优先级，范围为100~139，与静态优先级和补偿(bonus)有关
     * static_prio: 静态优先级，static_prio = 100 + nice + 20 (nice值为-20~19,所以static_prio值为100~139)
     * normal_prio: 没有受优先级继承影响的常规优先级，具体见normal_prio函数，跟属于什么类型的进程有关
     */
    int prio, static_prio, normal_prio;
    /* 实时进程优先级 */
    unsigned int rt_priority;
}
```

**动态优先级 静态优先级 实时优先级**


其中task_struct采用了三个成员表示进程的优先级:prio和normal_prio表示动态优先级, static_prio表示进程的静态优先级.

此外还用了一个字段rt_priority保存了实时进程的优先级


| 字段 | 描述 |
| ------------- |:-------------:|
| static_prio | 用于保存静态优先级, 是进程启动时分配的优先级, ，可以通过nice和sched_setscheduler系统调用来进行修改, 否则在进程运行期间会一直保持恒定 |
| prio | 保存进程的动态优先级 |
| normal_prio | 表示基于进程的静态优先级static_prio和调度策略计算出的优先级. 因此即使普通进程和实时进程具有相同的静态优先级, 其普通优先级也是不同的, 进程分叉(fork)时, 子进程会继承父进程的普通优先级 |
| rt_priority | 用于保存实时优先级 |


实时进程的优先级用实时优先级rt_priority来表示



#负荷权重
-------


##符合权重结构struct load_weight
-------

负荷权重用struct load_weight数据结构来表示, 保存着进程权重值weight。其定义在[/include/linux/sched.h, v=4.6, L1195](http://lxr.free-electrons.com/source/include/linux/sched.h?v=4.6#L1195), 如下所示

```c
struct load_weight {
	unsigned long weight;
	u32 inv_weight;
};
```

##调度实体的负荷权重load
-------

既然struct load_weight保存着进程的权重信息, 那么作为进程调度的实体, 必须将这个权重值与特定的进程task_struct, 更一般的与通用的调度实体sched_entity相关联

struct sched_entity作为进程调度的实体信息, 其内置了load_weight结构用于保存当前调度实体的权重, 参照http://lxr.free-electrons.com/source/include/linux/sched.h?v=4.6#L1195

```c
struct sched_entity {
	struct load_weight      load;           /* for load-balancing */
	/*  ......  */
};
```

##进程的符合权重

而进程可以被作为一个调度的实时, 其内部通过存储struct sched_entity se而间接存储了其load_weight信息, 参照http://lxr.free-electrons.com/source/include/linux/sched.h?v=4.6#L1415

```c
struct task_struct
{
	/*  ......  */
	struct sched_entity se;
    /*  ......  */
}
```

因此我们就可以通过task_statuct->se.load获取负荷权重的信息, 而set_load_weight负责根据进程类型及其静态优先级计算符合权重.


内核不仅维护了负荷权重自身, 还保存另外一个数值, 用于击碎安被负荷权重重除的结果.



http://blog.sina.com.cn/s/blog_9ca3f6e70102wkwp.html
http://lxr.free-electrons.com/source/kernel/sched/core.c#L3527
http://blog.chinaunix.net/uid-20671208-id-4909620.html
http://blog.chinaunix.net/uid-20671208-id-4909623.html
http://www.linuxidc.com/Linux/2016-05/131244.htm