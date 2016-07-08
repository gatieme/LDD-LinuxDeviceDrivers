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

下面我们到了最后一道工序, 完全公平调度器如何处理一个新创建的进程, 该工作由task_fork_fair函数来完成



#处理新进程
-------


我们对完全公平调度器需要考虑的最后一个操作, 创建新进程时的处理函数:task_fork_fair(早期的内核中对应是task_new_fair, 参见[LKML-sched: Sanitize fork() handling](https://lkml.org/lkml/2009/12/9/138)

##place_entity设置新进程的虚拟运行时间
-------

该函数先用update_curr进行通常的统计量更新, 然后调用此前讨论过的place_entity设置调度实体se的虚拟运行时间


```c
    /*  更新统计量  */
    update_curr(cfs_rq);

    if (curr)
        se->vruntime = curr->vruntime;
    /*  调整调度实体se的虚拟运行时间  */
    place_entity(cfs_rq, se, 1);
```

我们可以看到, 此时调用place_entity时的initial参数设置为1, 以便用sched_vslice_add计算初始的虚拟运行时间vruntime, 内核以这种方式确定了进程在延迟周期中所占的时间份额, 并转换成虚拟运行时间. 这个是调度器最初向进程欠下的债务.

>关于place_entity函数, 我们之前在讲解CFS队列操作的时候已经讲的很详细了
>
>
>参见[linux进程管理与调度之CFS入队出队操作](未添加网址)
>
>设想一下子如果休眠进程的vruntime保持不变, 而其他运行进程的 vruntime一直在推进, 那么等到休眠进程终于唤醒的时候, 它的vruntime比别人小很多, 会使它获得长时间抢占CPU的优势, 其他进程就要饿死了. 这显然是另一种形式的不公平，因此CFS是这样做的：在休眠进程被唤醒时重新设置vruntime值，以min_vruntime值为基础，给予一定的补偿，但不能补偿太多. 这个重新设置其虚拟运行时间的工作就是就是通过place_entity来完成的, 另外新进程创建完成后, 也是通过place_entity完成其虚拟运行时间vruntime的设置的.

>其中place_entity函数通过第三个参数initial参数来标识新进程创建和进程睡眠后苏醒两种情况的
>
>在进程入队时enqueue_entity设置的initial参数为0, 参见[kernel/sched/fair.c, line 3207](http://lxr.free-electrons.com/source/kernel/sched/fair.c#L3207)
>
>在task_fork_fair时设置的initial参数为1, 参见[kernel/sched/fair.c, line 8167](http://lxr.free-electrons.com/source/kernel/sched/fair.c#L8167)


##sysctl_sched_child_runs_first控制子进程运行时机
-------


接下来可使用参数sysctl_sched_child_runs_first控制新建子进程是否应该在父进程之前运行. 这通常是有益的, 特别在子进程随后会执行exec系统调用的情况下. 该参数的默认设置是1, 但可以通过/proc/sys/kernel/sched_child_first修改, 代码如下所示

```c
    /*  如果设置了sysctl_sched_child_runs_first期望se进程先运行
     *  但是se进行的虚拟运行时间却大于当前进程curr
     *  此时我们需要保证se的entity_key小于curr, 才能保证se先运行
     *  内核此处是通过swap(curr, se)的虚拟运行时间来完成的  */
    if (sysctl_sched_child_runs_first && curr && entity_before(curr, se))
    {
        /*
         * Upon rescheduling, sched_class::put_prev_task() will place
         * 'current' within the tree based on its new key value.
         */
        /*  由于curr的vruntime较小, 为了使se先运行, 交换两者的vruntime  */
        swap(curr->vruntime, se->vruntime);
        /*  设置重调度标识, 通知内核在合适的时间进行进程调度  */
        resched_curr(rq);
    }
```

如果entity_before(curr, se), 则父进程curr的虚拟运行时间vruntime小于子进程se的虚拟运行时间, 即在红黑树中父进程curr更靠左(前), 这就意味着父进程将在子进程之前被调度. 这种情况下如果设置了sysctl_sched_child_runs_first标识, 这时候我们必须采取策略保证子进程先运行, 可以通过交换curlr和se的vruntime值, 来保证se进程(子进程)的vruntime小于curr.


## 适应迁移的vruntime值
-------


在task_fork_fair函数的最后, 使用了一个小技巧, 通过place_entity计算出的基准虚拟运行时间, 减去了运行队列的min_vruntime.


```c
    se->vruntime -= cfs_rq->min_vruntime;
```

我们前面讲解place_entity的时候说到, 新创建的进程和睡眠后苏醒的进程为了保证他们的vruntime与系统中进程的vruntime差距不会太大, 会使用place_entity来设置其虚拟运行时间vruntime, 在place_entity中重新设置vruntime值，以cfs_rq->min_vruntime值为基础，给予一定的补偿，但不能补偿太多.这样由于休眠进程在唤醒时或者新进程创建完成后会获得vruntime的补偿，所以它在醒来和创建后有能力抢占CPU是大概率事件，这也是CFS调度算法的本意，即保证交互式进程的响应速度，因为交互式进程等待用户输入会频繁休眠

但是这样子也会有一个问题, 我们是以某个cfs就绪队列的min_vruntime值为基础来设定的, 在多CPU的系统上，不同的CPU的负载不一样，有的CPU更忙一些，而每个CPU都有自己的运行队列，每个队列中的进程的vruntime也走得有快有慢，比如我们对比每个运行队列的min_vruntime值，都会有不同, 如果一个进程从min_vruntime更小的CPU (A) 上迁移到min_vruntime更大的CPU (B) 上，可能就会占便宜了，因为CPU (B) 的运行队列中进程的vruntime普遍比较大，迁移过来的进程就会获得更多的CPU时间片。这显然不太公平

同样的问题出现在刚创建的进程上, 还没有投入运行, 没有加入到某个就绪队列中, 它以某个就绪队列的min_vruntime为基准设置了虚拟运行时间, 但是进程不一定在当前CPU上运行, 即新创建的进程应该是可以被迁移的.



CFS是这样做的：

*	当进程从一个CPU的运行队列中出来 (dequeue_entity) 的时候，它的vruntime要减去队列的min_vruntime值

*	而当进程加入另一个CPU的运行队列 ( enqueue_entiry) 时，它的vruntime要加上该队列的min_vruntime值

*	当进程刚刚创建以某个cfs_rq的min_vruntime为基准设置其虚拟运行时间后，也要减去队列的min_vruntime值

这样，进程从一个CPU迁移到另一个CPU之后，vruntime保持相对公平。

> 参照[sched: Remove the cfs_rq dependency
from set_task_cpu()](http://osdir.com/ml/linux-kernel/2009-12/msg07613.html)
>
>To prevent boost or penalty in the new cfs_rq caused by delta min_vruntime between the two cfs_rqs, we skip vruntime adjustment.

减去min_vruntime的情况如下

```c
dequeue_entity()：

	if (!(flags & DEQUEUE_SLEEP))
		se->vruntime -= cfs_rq->min_vruntime;

task_fork_fair():

	se->vruntime -= cfs_rq->min_vruntime;

switched_from_fair():
    if (!se->on_rq && p->state != TASK_RUNNING) 
    {
    	/*
         * Fix up our vruntime so that the current sleep doesn't
         * cause 'unlimited' sleep bonus.
         */
		place_entity(cfs_rq, se, 0);
        se->vruntime -= cfs_rq->min_vruntime;
	}
```


加上min_vruntime的情形
```c
enqueue_entity:
// http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L3196

	if (!(flags & ENQUEUE_WAKEUP) || (flags & ENQUEUE_WAKING))
		se->vruntime += cfs_rq->min_vruntime;

attach_task_cfs_rq:
// http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L8267

if (!vruntime_normalized(p))
		se->vruntime += cfs_rq->min_vruntime;
```
