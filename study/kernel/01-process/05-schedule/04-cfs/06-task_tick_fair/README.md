Linux CFS调度器之task_tick_fair处理周期性调度器
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-07-4 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/details/51456569) |


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

##1.3	今日看点--(CFS如何处理周期性调度器)
-------

在处理周期调度器的时候
