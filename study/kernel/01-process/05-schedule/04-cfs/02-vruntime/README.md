Linux进程调度器的设计
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |



CFS负责处理普通非实时进程, 这类进程是我们linux中最普遍的进程, 今天我们把注意力转向CFS的虚拟时钟


#CFS虚拟时钟
-------


完全公平调度算法CFS依赖于虚拟时钟, 用以度量等待进程在完全公平系统中所能得到的CPU时间. 但是数据结构中任何地方都没有找到虚拟时钟. 这个是由于所有的必要信息都可以根据现存的实际时钟和每个进程相关的负荷权重推算出来.

假设现在系统有A，B，C三个进程，A.weight=1,B.weight=2,C.weight=3.那么我们可以计算出整个公平调度队列的总权重是cfs_rq.weight = 6，很自然的想法就是，公平就是你在重量中占的比重的多少来拍你的重要性，那么，A的重要性就是1/6,同理，B和C的重要性分别是2/6,3/6.很显然C最重要就应改被先调度，而且占用的资源也应该最多，即假设A，B,C运行一遍的总时间假设是6个时间单位的话，A占1个单位，B占2个单位，C占三个单位。这就是CFS的公平策略.

**CFS调度算法的思想**：理想状态下每个进程都能获得相同的时间片，并且同时运行在CPU上，但实际上一个CPU同一时刻运行的进程只能有一个。也就是说，当一个进程占用CPU时，其他进程就必须等待。CFS为了实现公平，必须惩罚当前正在运行的进程，以使那些正在等待的进程下次被调度.

具体实现时，CFS通过每个进程的**虚拟运行时间(vruntime)**来衡量哪个进程最值得被调度. CFS中的就绪队列是一棵以vruntime为键值的红黑树，虚拟时间越小的进程越靠近整个红黑树的最左端。因此，调度器每次选择位于红黑树最左端的那个进程，该进程的vruntime最小.


虚拟运行时间是通过进程的实际运行时间和进程的权重(weight)计算出来的。在CFS调度器中，将进程优先级这个概念弱化，而是强调进程的权重。一个进程的权重越大，则说明这个进程更需要运行，因此它的虚拟运行时间就越小，这样被调度的机会就越大。而，CFS调度器中的权重在内核是对用户态进程的优先级nice值, 通过prio_to_weight数组进行nice值和权重的转换而计算出来的

#update_curr函数
-------

所有与虚拟时钟有关的计算都在update_curr中执行, 该函数在系统中各个不同地方调用, 包括周期性调度器在内.

update_curr的流程如下

*	首先计算进程当前时间与上次启动时间的差值

*	通过负荷权重和当前时间模拟出进程的虚拟运行时钟

*	重新设置cfs的min_vruntime保持其单调性


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
 ```

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

$$	delta =delta \times \dfrac{NICE\_0\_LOAD}{curr->se->load.weight}$$




每一个进程拥有一个vruntime, 每次需要调度的时候就选运行队列中拥有最小vruntime的那个进程来运行, vruntime在时钟中断里面被维护, 每次时钟中断都要更新当前进程的vruntime, 即vruntime以如下公式逐渐增长

那么`curr->vruntime += calc_delta_fair(delta_exec, curr);` 即相当于如下操作




| 条件 | 公式 |
|:-------:|:-------:|
| curr.nice != NICE_0_LOAD | $curr->vruntime += delta\_exec \times \dfrac{NICE\_0\_LOAD}{curr->se->load.weight}$|
| curr.nice == NICE_0_LOAD | $ curr->vruntime += delta $ |


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


#红黑树的键值entity_key和entity_before
-------


完全公平调度调度器CFS的真正关键点是, 红黑树的排序过程是进程的vruntime来进行计算的, 准确的来说同一个就绪队列所有进程(或者调度实体)依照其键值se->vruntime - cfs_rq->min_vruntime进行排序.

键值通过entity_key计算, 该函数在linux-2.6之中被定义, 但是后来的内核中移除了这个函数, 但是我们今天仍然讲解它, 因为它对我们理解CFS调度器和虚拟时钟vruntime有很多帮助, 我们也会讲到为什么这么有用的一个函数会被移除

我们可以在早期的linux-2.6.30(仅有[entity_key函数](http://lxr.linux.no/linux+v2.6.30/kernel/sched_fair.c#L269))和linux-2.6.32(定义了[entity_key](http://lxr.linux.no/linux+v2.6.32/kernel/sched_fair.c#L283)和[entity_befire函数](http://lxr.linux.no/linux+v2.6.32/kernel/sched_fair.c#L277))来查看
```c
static inline s64 entity_key(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
 	return se->vruntime - cfs_rq->min_vruntime;
}
```

键值较小的结点, 在CFS红黑树中排序的位置就越靠左, 因此也更快地被调度. 用这种方法, 内核实现了下面两种对立的机制

*	在程序运行时, 其vruntime稳定地增加, 他在红黑树中总是向右移动的.

	因为越重要的进程vruntime增加的越慢, 因此他们向右移动的速度也越慢, 这样其被调度的机会要大于次要进程, 这刚好是我们需要的

*	如果进程进入睡眠, 则其vruntime保持不变. 因为每个队列min_vruntime同时会单调增加, 那么当进程从睡眠中苏醒, 在红黑树中的位置会更靠左, 因为其键值相对来说变得更小了.


好了我们了解了entity_key计算了红黑树的键值, 他作为CFS对红黑树中结点的排序依据. 但是在新的内核中entity_key函数却早已消失不见, 这是为什么呢?

[sched: Replace use of entity_key](http://lkml.iu.edu/hypermail/linux/kernel/1107.2/01692.html)

[sched: Replace use of entity_key](http://marc.info/?l=linux-kernel&m=131127311326308)

我们在[linux-2.6.32的kernel/sched_fair.c](http://lxr.linux.no/linux+v2.6.32/kernel/sched_fair.c#L269)中搜索entity_key函数关键字, 会发现内核仅在__enqueue_entity(定义在[linux-2.6.32的kernel/sched_fair.c, line 309](http://lxr.linux.no/linux+v2.6.32/kernel/sched_fair.c#L309))函数中使用了entity_key函数用来比较两个调度实体的虚拟时钟键值的大小

即相当于如下代码

```c
if (entity_key(cfs_rq, se) < entity_key(cfs_rq, entry))

等价于
if (se->vruntime-cfs_rq->min_vruntime < entry->vruntime-cfs_rq->min_vruntime)

进一步化简为

if (se->vruntime < entry->vruntime)
````

即整个过程等价于比较两个调度实体vruntime值得大小

因此内核定义了函数entity_before来实现此功能, 函数定义在[linux+v2.6.32/kernel/sched_fair.c, line 269](http://lxr.linux.no/linux+v2.6.32/kernel/sched_fair.c#L269), 在我们新的linux-4.6内核中定义在[kernel/sched/fair.c, line 452](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L452)

```c
static inline int entity_before(struct sched_entity *a,
                                struct sched_entity *b)
{
    return (s64)(a->vruntime - b->vruntime) < 0;
}
```





