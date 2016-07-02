Linux CFS调度器之队列操作
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-29 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/details/51456569) |



CFS负责处理普通非实时进程, 这类进程是我们linux中最普遍的进程


#1	前景回顾
-------

##1.1	CFS调度算法
-------

**CFS调度算法的思想**

理想状态下每个进程都能获得相同的时间片，并且同时运行在CPU上，但实际上一个CPU同一时刻运行的进程只能有一个。也就是说，当一个进程占用CPU时，其他进程就必须等待。CFS为了实现公平，必须惩罚当前正在运行的进程，以使那些正在等待的进程下次被调度.

##  负荷权重和虚拟时钟

**虚拟时钟是红黑树排序的依据**

具体实现时，CFS通过每个进程的**虚拟运行时间(vruntime)**来衡量哪个进程最值得被调度. CFS中的就绪队列是一棵以vruntime为键值的红黑树，虚拟时间越小的进程越靠近整个红黑树的最左端。因此，调度器每次选择位于红黑树最左端的那个进程，该进程的vruntime最小.

**优先级计算负荷权重, 负荷权重和当前时间计算出虚拟运行时间**

虚拟运行时间是通过进程的实际运行时间和进程的权重(weight)计算出来的。在CFS调度器中，将进程优先级这个概念弱化，而是强调进程的权重。一个进程的权重越大，则说明这个进程更需要运行，因此它的虚拟运行时间就越小，这样被调度的机会就越大。而，CFS调度器中的权重在内核是对用户态进程的优先级nice值, 通过prio_to_weight数组进行nice值和权重的转换而计算出来的


**虚拟时钟相关公式**

 linux内核采用了计算公式：

| 属性 | 公式 | 描述 |
|:-------:|:-------:|
| ideal_time | sum_runtime *se.weight/cfs_rq.weight | 每个进程应该运行的时间 |
| sum_exec_runtime |  | 运行队列中所有任务运行完一遍的时间 |
| se.weight |  | 当前进程的权重 |
| cfs.weight |  | 整个cfs_rq的总权重 |

这里se.weight和cfs.weight根据上面讲解我们可以算出, sum_runtime是怎们计算的呢，linux内核中这是个经验值，其经验公式是

| 条件 | 公式 |
|:-------:|:-------:|
| 进程数 > sched_nr_latency | sum_runtime=sysctl_sched_min_granularity *nr_running |
| 进程数 <=sched_nr_latency | sum_runtime=sysctl_sched_latency = 20ms |

>注：sysctl_sched_min_granularity =4ms
>
>sched_nr_latency是内核在一个延迟周期中处理的最大活动进程数目

linux内核代码中是通过一个叫vruntime的变量来实现上面的原理的，即：

每一个进程拥有一个vruntime,每次需要调度的时候就选运行队列中拥有最小vruntime的那个进程来运行，vruntime在时钟中断里面被维护，每次时钟中断都要更新当前进程的vruntime,即vruntime以如下公式逐渐增长：


| 条件 | 公式 |
|:-------:|:-------:|
| curr.nice!=NICE_0_LOAD | vruntime +=  delta* NICE_0_LOAD/se.weight; |
| curr.nice=NICE_0_LOAD | vruntime += delta; |


##1.2	CFS进程入队和出队
-------

enqueue_task_fair和dequeue_task_fair分别用来向CFS就绪队列中添加或者删除进程

完全公平调度器CFS中有两个函数可用来增删队列的成员:enqueue_task_fair和dequeue_task_fair分别用来向CFS就绪队列中添加或者删除进程


**enqueue_task_fair进程入队**

向就绪队列中放置新进程的工作由函数enqueue_task_fair函数完成, 该函数定义在[kernel/sched/fair.c, line 5442](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L5442), 其函数原型如下

该函数将task_struct *p所指向的进程插入到rq所在的就绪队列中, 除了指向所述的就绪队列rq和task_struct的指针外, 该函数还有另外一个参数wakeup. 这使得可以指定入队的进程是否最近才被唤醒并转换为运行状态(此时需指定wakeup = 1), 还是此前就是可运行的(那么wakeup = 0).

```c
static void
enqueue_task_fair(struct rq *rq, struct task_struct *p, int flags)
```
enqueue_task_fair的执行流程如下

*	如果通过struct sched_entity的on_rq成员判断进程已经在就绪队列上, 则无事可做. 

*	否则, 具体的工作委托给enqueue_entity完成, 其中内核会借机用update_curr更新统计量
	在enqueue_entity内部如果需要会调用__enqueue_entity将进程插入到CFS红黑树中合适的结点


**dequeue_task_fair进程出队操作**

dequeue_task_fair函数在完成睡眠等情况下调度, 将任务从就绪队列中移除

其执行的过程正好跟enqueue_task_fair的思路相同, 只是操作刚好相反


enqueue_task_fair的执行流程如下

*	如果通过struct sched_entity的on_rq成员判断进程已经在就绪队列上, 则无事可做. 

*	否则, 具体的工作委托给dequeue_entity完成, 其中内核会借机用update_curr更新统计量
	在enqueue_entity内部如果需要会调用__dequeue_entity将进程插入到CFS红黑树中合适的结点


#2	今日看点(CFS如何选择最合适的进程)
-------

每个调度器类sched_class都必须提供一个pick_next_task函数用以在就绪队列中选择一个最优的进程来等待调度, 而我们的CFS调度器类中, 选择下一个将要运行的进程由pick_next_task_fair函数来完成



# CFS如何选择一个进程
-------

##  pick_next_task_fair
-------

选择下一个将要运行的进程pick_next_task_fair执行. 其代码执行流程如下

如果nr_running计数器为0, 即当前队列上没有可运行进程, 则无事可做, 函数可以立即返回. 否则将具体工作委托给pick

##  put_prev_task
-------

在选中了下一个将被调度执行的进程之后，回到pick_next_task_fair中，执行set_next_entity



##  pick_next_entity
-------


##  set_next_entity
-------



http://blog.csdn.net/sunnybeike/article/details/6918586