Linux进程调度器的设计
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |



CFS负责处理普通非实时进程, 这类进程是我们linux中最普遍的进程, 今天我们把注意力转向CFS的虚拟时钟


#CFS虚拟时钟
-------


完全公平调度算法CFS依赖于虚拟时钟, 用以度量等待进程在完全公平系统中所能得到的CPU时间. 但是数据结构中任何地方都没有找到虚拟时钟. 这个是由于所有的必要信息都可以根据现存的实际时钟和每个进程相关的负荷权重推算出来.


#update_curr函数
-------

所有与虚拟时钟有关的计算都在update_curr中执行, 该函数在系统中各个不同地方调用, 包括周期性调度器在内.

##计算时间差

首先, 该函数确定就绪队列的当前执行进程, 并获取主调度器就绪队列的实际时钟值, 该值在每个调度周期都会更新

```c
	/*  确定就绪队列的当前执行进程curr  */
	struct sched_entity *curr = cfs_rq->curr;
```

其中辅助函数[rq_of](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L248)用于确定与CFS就绪队列相关的struct rq实例, 其定义在[kernel/sched/fair.c, line 248](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L248)

cfs_rq就绪队列中存储了指向就绪队列的实例,参见[kernel/sched/sched.h, line412](http://lxr.free-electrons.com/source/kernel/sched/sched.h#L412), 而rq_of就返回了这个指向rq的指针, rq_of定义在[kernel/sched/fair.c, line 249](http://lxr.free-electrons.com/source/kernel/sched/fair.c#L249)


[rq_clock_task](http://lxr.free-electrons.com/source/kernel/sched/sched.h#L735)函数返回了运行队列的clock_task成员.

```c
    /*  rq_of -=> return cfs_rq->rq 返回cfs队列所在的全局就绪队列  
     *  rq_clock_task返回了rq的clock_task  */
    u64 now = rq_clock_task(rq_of(cfs_rq));
    u64 delta_exec;
```
 如果就队列上没有进程在执行, 则显然无事可做, 否则内核计算当前和上一次更新负荷权重时两次的时间的差值

```c
    /*   如果就队列上没有进程在执行, 则显然无事可做  */
    if (unlikely(!curr))
        return;

    /*  内核计算当前和上一次更新负荷权重时两次的时间的差值 */
    delta_exec = now - curr->exec_start;
    if (unlikely((s64)delta_exec <= 0))
        return;
```

然后重新更新更新启动时间exec_start为now, 以备下次计算时使用

最后将计算出的时间差, 加到了先前的统计时间上

```c
    /*  重新更新启动时间exec_start为now  */
    curr->exec_start = now;

    schedstat_set(curr->statistics.exec_max,
              max(delta_exec, curr->statistics.exec_max));

    /*  将时间差加到先前统计的时间即可  */
    curr->sum_exec_runtime += delta_exec;
    schedstat_add(cfs_rq, exec_clock, delta_exec);
 ````

##模拟虚拟时钟
-------


有趣的事情是如何使用给出的信息来模拟不存在的虚拟时钟. 这一次内核的实现仍然是非常巧妙地, 针对最普通的情形节省了一些时间. 对于运行在nice级别0的进程来说, 根据定义虚拟时钟和物理时间相等. 在使用不同的优先级时, 必须根据进程的负荷权重重新衡定时间

```c
    curr->vruntime += calc_delta_fair(delta_exec, curr);
```

其中calc_delta_fair函数是计算的关键

```c
//  http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L596
/*
 * delta /= w
 */
static inline u64 calc_delta_fair(u64 delta, struct sched_entity *se)
{
    if (unlikely(se->load.weight != NICE_0_LOAD))
        delta = __calc_delta(delta, NICE_0_LOAD, &se->load);

    return delta;
}
```

忽略舍入和溢出检查, calc_delta_fair所做的就是根据下列公式计算:

$$	delta\_exec\_weighted =delta\_exec \times \dfrac{NICE\_TO\_LOAD}{curr->se->load.weight} $$

$$curr->vruntime = $$


在该计算中可以派上用场了, 回想一下子,　可知越重要的进程会有越高的优先级(即, 越低的nice值), 会得到更大的权重, 因此累加的虚拟运行时间会小一点, 





