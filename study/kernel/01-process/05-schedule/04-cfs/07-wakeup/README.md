Linux CFS调度器之唤醒抢占
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-07-05 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/details/51456569) |


CFS负责处理普通非实时进程, 这类进程是我们linux中最普遍的进程


#1	前景回顾
-------


##1.1	CFS调度算法
-------

**CFS调度算法的思想**

理想状态下每个进程都能获得相同的时间片，并且同时运行在CPU上，但实际上一个CPU同一时刻运行的进程只能有一个。也就是说，当一个进程占用CPU时，其他进程就必须等待。CFS为了实现公平，必须惩罚当前正在运行的进程，以使那些正在等待的进程下次被调度.

##1.2	CFS的pick_next_fair选择下一个进程
-------

前面的一节中我们讲解了CFS的pick_next操作**pick_next_task_fair函数**, 他从当前运行队列上找出一个最优的进程来抢占处理器 ,一般来说这个最优进程总是**红黑树的最左进程left结点(其vruntime值最小)**, 当然如果挑选出的进程正好是队列是上需要**被调过调度的skip**, 则可能需要进一步读取**红黑树的次左结点second**, 而同样**curr进程**可能vruntime与cfs_rq的min_vruntime小, 因此它可能更渴望得到处理器, 而**last和next进程**由于刚被唤醒也应该尽可能的补偿.

**主调度器schedule**在选择最优的进程抢占处理器的时候, 通过__schedule调用**全局的pick_next_task**函数, 在**全局的pick_next_task**函数中, 按照stop > dl > rt > cfs > idle的顺序依次从**各个调度器类中pick_next函数**, 从而选择一个最优的进程.

##1.3	CFS如何处理周期性调度
-------

周期性调度器的工作由scheduler_tick函数完成(定义在[kernel/sched/core.c, line 2910](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L2910)), 在scheduler_tick中周期性调度器通过调用curr进程所属调度器类sched_class的task_tick函数完成周期性调度的工作

周期调度的工作形式上sched_class调度器类的task_tick函数完成, CFS则对应task_tick_fair函数, 但实际上工作交给entity_tick完成.

##1.4	唤醒抢占
-------


当在try_to_wake_up和wake_up_new_task中唤醒进程时, 内核使用全局check_preempt_curr看看是否进程可以抢占当前进程可以抢占当前运行的进程. 请注意该过程不涉及核心调度器.


每个调度器类都因应该实现一个check_preempt_curr函数, 在全局check_preempt_curr中会调用进程其所属调度器类check_preempt_curr进行抢占检查, 对于完全公平调度器CFS处理的进程, 则对应由check_preempt_wakeup函数执行该策略.

新唤醒的进程不必一定由完全公平调度器处理, 如果新进程是一个实时进程, 则会立即请求调度, 因为实时进程优先极高, 实时进程总会抢占CFS进程.

