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
    update_min_vruntime(cfs_rq);

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

忽略舍入和溢出检查, calc_delta_fair函数所做的就是根据下列公式计算:

$$	delta =delta \times \dfrac{NICE\_TO\_LOAD}{curr->se->load.weight} $$

那么`curr->vruntime += calc_delta_fair(delta_exec, curr);` 即相当于如下操作

$$curr->vruntime = curr->vruntime +delta\_exec \times \dfrac{NICE\_TO\_LOAD}{curr->se->load.weight} $$


在该计算中可以派上用场了, 回想一下子,　可知越重要的进程会有越高的优先级(即, 越低的nice值), 会得到更大的权重, 因此累加的虚拟运行时间会小一点, 

根据公式可知, nice = 0的进程(优先级120), 则虚拟时间和物理时间是相等的, 即current->se->load.weight等于NICE_0_LAD的情况.

##重新设置cfs_rq->min_vruntime

接着内核需要重新设置`min_vruntime`. 必须小心保证该值是单调递增的, 通过update_min_vruntime函数来设置

```c
//  http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L457

static void update_min_vruntime(struct cfs_rq *cfs_rq)
{
    /*  初始化vruntime的值, 相当于如下的代码
    if (cfs_rq->curr != NULL)
        vruntime = cfs_rq->curr->vruntime;
    else
        vruntime = cfs_rq->min_vruntime;
    */
    u64 vruntime = cfs_rq->min_vruntime;

    if (cfs_rq->curr)
        vruntime = cfs_rq->curr->vruntime;


    /*  检测红黑树是都有最左的节点, 即是否有进程在树上等待调度
     *  cfs_rq->rb_leftmost(struct rb_node *)存储了进程红黑树的最左节点
     *  这个节点存储了即将要被调度的结点  
     *  */
    if (cfs_rq->rb_leftmost)
    {
        /*  获取最左结点的调度实体信息se, se中存储了其vruntime
         *  rb_leftmost的vruntime即树中所有节点的vruntiem中最小的那个  */
        struct sched_entity *se = rb_entry(cfs_rq->rb_leftmost,
                           struct sched_entity,
                           run_node);
        /*  如果就绪队列上没有curr进程
         *  则vruntime设置为树种最左结点的vruntime
         *  否则设置vruntiem值为cfs_rq->curr->vruntime和se->vruntime的最小值
         */
        if (!cfs_rq->curr)  /*  此时vruntime的原值为cfs_rq->min_vruntime*/
            vruntime = se->vruntime;
        else                /* 此时vruntime的原值为cfs_rq->curr->vruntime*/
            vruntime = min_vruntime(vruntime, se->vruntime);
    }

    /* ensure we never gain time by being placed backwards. 
     * 为了保证min_vruntime单调不减
     * 只有在vruntime超出的cfs_rq->min_vruntime的时候才更新
     */
    cfs_rq->min_vruntime = max_vruntime(cfs_rq->min_vruntime, vruntime);
#ifndef CONFIG_64BIT
    smp_wmb();
    cfs_rq->min_vruntime_copy = cfs_rq->min_vruntime;
#endif
}
```
我们通过分析update_min_vruntime函数设置cfs_rq->min_vruntime的流程如下

*	首先检测cfs就绪队列上是否有活动进程curr, 以此设置vruntime的值
	如果cfs就绪队列上没有活动进程curr, 就设置vruntime为curr->vruntime;
	否则又活动进程就设置为vruntime为cfs_rq的原min_vruntime;

*	接着检测cfs的红黑树上是否有最左节点, 即等待被调度的节点, 重新设置vruntime的值为curr进程和最左进程rb_leftmost的vruntime较小者的值

*	为了保证min_vruntime单调不减, 只有在vruntime超出的cfs_rq->min_vruntime的时候才更新

update_min_vruntime依据当前进程和待调度的进程的vruntime值, 设置出一个可能的vruntime值, 但是只有在这个可能的vruntime值大于就绪队列原来的min_vruntime的时候, 才更新就绪队列的min_vruntime, 利用该策略, 内核确保min_vruntime只能增加, 不能减少.

update_min_vruntime函数的流程等价于如下的代码

```c
//  依据curr进程和待调度进程rb_leftmost找到一个可能的最小vruntime值
if (cfs_rq->curr != NULL cfs_rq->rb_leftmost == NULL)
    vruntime = cfs_rq->curr->vruntime;
else if(cfs_rq->curr == NULL && cfs_rq->rb_leftmost != NULL)
        vruntime = cfs_rq->rb_leftmost->se->vruntime;
else if (cfs_rq->curr != NULL cfs_rq->rb_leftmost != NULL)
    vruntime = min(cfs_rq->curr->vruntime, cfs_rq->rb_leftmost->se->vruntime);
else if(cfs_rq->curr == NULL cfs_rq->rb_leftmost == NULL)
    vruntime = cfs_rq->min_vruntime;

//  每个队列的min_vruntime只有被树上某个节点的vruntime((curr和程rb_leftmost两者vruntime的较小值)超出时才更新
cfs_rq->min_vruntime = max_vruntime(cfs_rq->min_vruntime, vruntime);
```

其中寻找可能vruntime的策略我们采用表格的形式可能更加直接


| 活动进程curr | 待调度进程rb_leftmost | 可能的vruntime值 | cfs_rq |
| ------- |:-------:|:-------:|
| NULL | NULL | cfs_rq->min_vruntime | 维持原值 |
| NULL | 非NULL | rb_leftmost->se->vruntime | max(可能值vruntime, 原值) |
| 非NULL | NULL | curr->vruntime | max(可能值vruntime, 原值) |
| 非NULL | 非NULL | min(curr->vruntime, rb_leftmost->se->vruntime) | max(可能值vruntime, 原值) |

##
-------





