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

##1,2	进程的创建
-------


fork, vfork和clone的系统调用的入口地址分别是sys_fork, sys_vfork和sys_clone, 而他们的定义是依赖于体系结构的, 而他们最终都调用了_do_fork（linux-4.2之前的内核中是do_fork），在_do_fork中通过copy_process复制进程的信息，调用wake_up_new_task将子进程加入调度器中

1.	dup_task_struct中为其分配了新的堆栈

2.	调用了sched_fork，将其置为TASK_RUNNING

3.	copy_thread(_tls)中将父进程的寄存器上下文复制给子进程，保证了父子进程的堆栈信息是一致的, 

4.	将ret_from_fork的地址设置为eip寄存器的值

5.	为新进程分配并设置新的pid

6.	最终子进程从ret_from_fork开始执行

##1.3	处理新进程
-------

前面讲解了CFS的很多信息

| 信息 | 描述 |
|:-------:|:-------:|
| 负荷权重 load_weight | CFS进程的负荷权重, 与进程的优先级相关, 优先级越高的进程, 负荷权重越高 |
| 虚拟运行时间 vruntime | 虚拟运行时间是通过进程的实际运行时间和进程的权重(weight)计算出来的。在CFS调度器中，将进程优先级这个概念弱化，而是强调进程的权重。一个进程的权重越大，则说明这个进程更需要运行，因此它的虚拟运行时间就越小，这样被调度的机会就越大。而，CFS调度器中的权重在内核是对用户态进程的优先级nice值, 通过prio_to_weight数组进行nice值和权重的转换而计算出来的 |

我们也讲解了CFS的很多进程操作

| 信息 | 函数 | 描述 |
|:-------:|:-------:|:-------:|
| 进程入队/出队 | enqueue_task_fair/dequeue_task_fair | 向CFS的就读队列中添加删除进程 |
| 选择最优进程(主调度器) | pick_next_task_fair | 主调度器会按照如下顺序调度 schedule -> __schedule -> 全局pick_next_task<br><br>全局的pick_next_task函数会从按照优先级遍历所有调度器类的pick_next_task函数, 去查找最优的那个进程, 当然因为大多数情况下, 系统中全是CFS调度的非实时进程, 因而linux内核也有一些优化的策略<br><br>一般情况下选择红黑树中的最左进程left作为最优进程完成调度, 如果选出的进程正好是cfs_rq->skip需要跳过调度的那个进程, 则可能需要再检查红黑树的次左进程second, 同时由于curr进程不在红黑树中, 它可能比较饥渴, 将选择出进程的与curr进程进行择优选取, 同样last进程和next进程由于刚被唤醒, 可能比较饥饿, 优先调度他们能提高系统缓存的命中率 |
| 周期性调度 | task_tick_fair |周期性调度器的工作由**scheduler_tick函数**完成, 在scheduler_tick中周期性调度器通过调用curr进程所属调度器类sched_class的task_tick函数完成周期性调度的工作<br><br>而entity_tick中则通过**check_preempt_tick**函数检查是否需要抢占当前进程curr, 如果发现curr进程已经运行了足够长的时间, 其他进程已经开始饥饿, 那么我们就需要通过**resched_curr**函数来设置重调度标识TIF_NEED_RESCHED, 此标志会提示系统在合适的时间进行调度 |

下面我们到了最后一道工序, 完全公平调度器如何处理一个新创建的进程, 



#处理新进程
-------


我们对完全公平调度器需要考虑的最后一个操作, 创建新进程时的处理函数:task_new_fair.

该函数的行为可使用参数sysctl_sched_child_runs_first控制. 该参数用于判断新建子进程是否应该在父进程之前运行. 这通常是有益的, 特别在子进程随后会执行exec系统调用的情况下. 该参数的默认设置是1, 但可以通过/proc/sys/kernel/sched_child_first修改

该函数先用update_curr进行通常的统计量更新, 然后调用此前讨论过的place_entity.

