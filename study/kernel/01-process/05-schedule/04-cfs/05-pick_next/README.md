Linux CFS调度器之pick_next_task_fair选择下一个被调度的进程
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


之前我们在将主调度器的时候, 主调度器schedule函数在进程调度抢占时, 会通过__schedule函数调用全局pick_next_task选择一个最优的进程, 在pick_next_task中我们就按照优先级依次调用不同调度器类提供的pick_next_task方法

今天就让我们窥探一下完全公平调度器类CFS的pick_next_task方法pick_next_fair


# CFS如何选择一个进程
-------


##  pick_next_task_fair
-------

选择下一个将要运行的进程pick_next_task_fair执行. 其代码执行流程如下

对于pick_next_task_fair函数的讲解, 我们从simple标签开始, 这个是常规状态下pick_next的思路, 简单的来说pick_next_task_fair的函数框架如下

```c
again:
	控制循环来读取最优进程

#ifdef CONFIG_FAIR_GROUP_SCHED
	完成组调度下的pick_next选择
    返回被选择的调度时实体的指针
#endif

simple:
	最基础的pick_next函数
   返回被选择的调度时实体的指针

idle :
	如果系统中没有可运行的进行, 则需要调度idle进程
```

可见我们会发现,

*	simple标签是CFS中最基础的pick_next操作

*	idle则使得在没有进程被调度时, 调度idle进程

*	again标签用于循环的进行pick_next操作

*	CONFIG_FAIR_GROUP_SCHED宏指定了组调度情况下的pick_next操作, 如果不支持组调度, 则pick_next_task_fair将直接从simple开始执行

如果nr_running计数器为0, 即当前队列上没有可运行进程, 则无事可做, 函数可以立即返回. 否则将具体工作委托给pick


#simple无组调度最简单的pick_next_task_fair
-------

在不支持组调度情况下(选项CONFIG_FAIR_GROUP_SCHED), CFS的pick_next_task_fair函数会直接执行simple标签, 优选下一个函数, 这个流程清晰而且简单, 但是已经足够我们理解cfs的pick_next了

##simple流程
-------

pick_next_task_fair函数的simple标签定义在[kernel/sched/fair.c, line 5526)](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L5526), 代码如下所示


```c
simple:
    cfs_rq = &rq->cfs;
#endif
    /*  如果nr_running计数器为0,
     *  当前队列上没有可运行进程,
     *  则需要调度idle进程  */
    if (!cfs_rq->nr_running)
        goto idle;
    /*  将当前进程放入运行队列的合适位置  */
    put_prev_task(rq, prev);

    do
    {
        /*  选出下一个可执行调度实体(进程)  */
        se = pick_next_entity(cfs_rq, NULL);
        /*  把选中的进程从红黑树移除，更新红黑树  
         *  set_next_entity会调用__dequeue_entity完成此工作  */
        set_next_entity(cfs_rq, se);
        /*  group_cfs_rq return NULL when !CONFIG_FAIR_GROUP_SCHED
         *  在非组调度情况下, group_cfs_rq返回了NULL  */
        cfs_rq = group_cfs_rq(se);
    } while (cfs_rq);  /*  在没有配置组调度选项(CONFIG_FAIR_GROUP_SCHED)的情况下.group_cfs_rq()返回NULL.因此,上函数中的循环只会循环一次  */


    /*  获取到调度实体指代的进程信息  */
    p = task_of(se);

    if (hrtick_enabled(rq))
        hrtick_start_fair(rq, p);

    return p;
```

其基本流程如下

| 流程 | 描述 |
|:-------:|:-------:|
| !cfs_rq->nr_running -=>  goto idle; | 如果nr_running计数器为0, 当前队列上没有可运行进程, 则需要调度idle进程 |
| put_prev_task(rq, prev); | 将当前进程放入运行队列的合适位置, 每次当进程被调度后都会使用set_next_entity从红黑树中移除, 因此被抢占时需要重新加如红黑树中等待被调度 |
| se = pick_next_entity(cfs_rq, NULL); | 选出下一个可执行调度实体 |
| set_next_entity(cfs_rq, se); | set_next_entity会调用__dequeue_entity把选中的进程从红黑树移除，并更新红黑树 |


##  put_prev_task
-------


**全局put_prev_task函数**


put_prev_task用来将前一个进程prev放回到就绪队列中, 这是一个全局的函数, 而每个调度器类也必须实现一个自己的put_prev_task函数(比如CFS的put_prev_task_fair), 

由于CFS调度的时候, prev进程不一定是一个CFS调度的进程, 因此必须调用全局的put_prev_task来调用prev进程所属调度器类sched_class的对应put_prev_task方法, 完成将进程放回到就绪队列中


全局的put_prev_task函数定义在[kernel/sched/sched.h, line 1245](http://lxr.free-electrons.com/source/kernel/sched/sched.h?v=4.6#L1245), 代码如下所示
```c
static inline void put_prev_task(struct rq *rq, struct task_struct *prev)
{
	prev->sched_class->put_prev_task(rq, prev);
}
```

**CFS的put_prev_task_fair函数**


然后我们来分析一下CFS的put_prev_task_fair函数, 其定义在[kernel/sched/fair.c, line 5572](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L5572)

在选中了下一个将被调度执行的进程之后，回到pick_next_task_fair中，执行set_next_entity

```c
/*
 * Account for a descheduled task:
 */
static void put_prev_task_fair(struct rq *rq, struct task_struct *prev)
{
    struct sched_entity *se = &prev->se;
    struct cfs_rq *cfs_rq;

    for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
        put_prev_entity(cfs_rq, se);
    }
}
```

前面我们说到过函数在组策略情况下, 调度实体之间存在父子的层次, for_each_sched_entity会从当前调度实体开始,　然后循环向其父调度实体进行更新, 非组调度情况下则只执行一次

而put_prev_task_fair函数最终会调用put_prev_entity函数将prev的调度时提se放回到就绪队列中等待下次调度


**put_prev_entity函数**

[put_prev_entity](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L3443)函数定义在[kernel/sched/fair.c, line 3443](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L3443), 他在更新了虚拟运行时间等信息后, 最终通过__enqueue_entity函数将prev进程(即current进程)放回就绪队列rq上



##  pick_next_entity
-------

**pick_next_entity函数完全注释**

```c
/*
 * Pick the next process, keeping these things in mind, in this order:
 * 1) keep things fair between processes/task groups
 * 2) pick the "next" process, since someone really wants that to run
 * 3) pick the "last" process, for cache locality
 * 4) do not run the "skip" process, if something else is available
 *
 *  1. 首先要确保任务组之间的公平, 这也是设置组的原因之一
 *  2. 其次, 挑选下一个合适的（优先级比较高的）进程
 *     因为它确实需要马上运行 
 *  3. 如果没有找到条件2中的进程
 *     那么为了保持良好的局部性
 *     则选中上一次执行的进程 
 *  4. 只要有任务存在, 就不要让CPU空转, 
 *     只有在没有进程的情况下才会让CPU运行idle进程
 */
static struct sched_entity *
pick_next_entity(struct cfs_rq *cfs_rq, struct sched_entity *curr)
{
    /*  摘取红黑树最左边的进程  */
    struct sched_entity *left = __pick_first_entity(cfs_rq);
    struct sched_entity *se;

    /*
     * If curr is set we have to see if its left of the leftmost entity
     * still in the tree, provided there was anything in the tree at all.
     *
     * 如果
     * left == NULL  或者
     * curr != NULL curr进程比left进程更优(即curr的虚拟运行时间更小) 
     * 说明curr进程是自动放弃运行权利, 且其比最左进程更优
     * 因此将left指向了curr, 即curr是最优的进程
     */
    if (!left || (curr && entity_before(curr, left)))
    {
        left = curr;
    }

    /* se = left存储了cfs_rq队列中最优的那个进程  
     * 如果进程curr是一个自愿放弃CPU的进程(其比最左进程更优), 则取se = curr
     * 否则进程se就取红黑树中最左的进程left, 它必然是当前就绪队列上最优的
     */
    se = left; /* ideally we run the leftmost entity */

    /*
     * Avoid running the skip buddy, if running something else can
     * be done without getting too unfair.
     *
     * cfs_rq->skip存储了需要调过不参与调度的进程调度实体
     * 如果我们挑选出来的最优调度实体se正好是skip
     * 那么我们需要选择次优的调度实体se来进行调度
     * 由于之前的se = left = (curr before left) curr left
     * 则如果 se == curr == skip, 则选择left = __pick_first_entity进行即可
     * 否则则se == left == skip, 则选择次优的那个调度实体second
     */
    if (cfs_rq->skip == se)
    {
        struct sched_entity *second;

        if (se == curr) /* se == curr == skip选择最左的那个调度实体left  */
        {
            second = __pick_first_entity(cfs_rq);
        }
        else    /*  否则se == left == skip, 选择次优的调度实体second  */
        {
            /*  摘取红黑树上第二左的进程节点  */
            second = __pick_next_entity(se);
            /*  同时与left进程一样, 
             * 如果
             * second == NULL 没有次优的进程  或者
             * curr != NULL curr进程比left进程更优(即curr的虚拟运行时间更小) 
             * 说明curr进程比最second进程更优
             * 因此将second指向了curr, 即curr是最优的进程*/
            if (!second || (curr && entity_before(curr, second)))
                second = curr;
        }

        /* 判断left和second的vruntime的差距是否小于sysctl_sched_wakeup_granularity
         * 即如果second能抢占left */
        if (second && wakeup_preempt_entity(second, left) < 1)
            se = second;
    }

    /*
     * Prefer last buddy, try to return the CPU to a preempted task.
     *
     * 
     */
    if (cfs_rq->last && wakeup_preempt_entity(cfs_rq->last, left) < 1)
        se = cfs_rq->last;

    /*
     * Someone really wants this to run. If it's not unfair, run it.
     */
    if (cfs_rq->next && wakeup_preempt_entity(cfs_rq->next, left) < 1)
        se = cfs_rq->next;

    /* 用过一次任何一个next或者last
     * 都需要清除掉这个指针
     * 以免影响到下次pick next sched_entity  */
    clear_buddies(cfs_rq, se);

    return se;
}
```

**从left, second和curr进程中选择最优的进程**


pick_next_entity则从CFS的红黑树中摘取一个最优的进程, 这个进程往往在红黑树的最左端, 即vruntime最小, 但是也有例外, 但是不外乎这几个进程

| 调度实体 | 描述 |
|:-------:|:-------:|
| left = __pick_first_entity(cfs_rq) | **红黑树的最左节点**, 这个节点拥有当前队列中vruntime最小的特性, 即应该优先被调度 |
| second = __pick_first_entity(left) | **红黑树的次左节点**, 为什么这个节点也可能呢, 因为内核支持skip跳过某个进程的抢占权力的, 如果left被标记为skip(由cfs_rq->skip域指定), 那么可能就需要找到次优的那个进程 |
| curr结点 | curr节点的vruntime可能比left和second更小, 但是由于它正在运行, 因此它不在红黑树中(进程抢占物理机的时候对应节点同时会从红黑树中删除), 但是如果其vruntime足够小, 意味着cfs调度器应该尽可能的补偿curr进程, 让它再次被调度 |

其中__pick_first_entity会返回cfs_rq红黑树中的最左节点rb_leftmost所属的调度实体信息, 该函数定义在[kernel/sched/fair.c, line 543](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L3443)

而__pick_next_entity(se)函数则返回se在红黑树中中序遍历的下一个节点信息, 该函数定义在[kernel/sched/fair.c, line 544](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L3443), 获取下一个节点的工作可以通过内核红黑树的标准操作rb_next完成



**cfs_rq的last和next指针域**

在pick_next_entity的最后, 要把红黑树最左下角的进程和另外两个进程(即next和last)做比较, next是抢占失败的进程, 而last则是抢占成功后被抢占的进程, 这三个进程到底哪一个是最优的next进程呢?

Linux CFS实现的判决条件是：

1.	尽可能满足需要刚被唤醒的进程抢占其它进程的需求

2.	尽可能减少以上这种抢占带来的缓存刷新的影响


**cfs_rq的last和next指针，last表示最后一个执行wakeup的sched_entity,next表示最后一个被wakeup的sched_entity。他们在进程wakeup的时候会赋值，在pick新sched_entity的时候，会优先选择这些last或者next指针的sched_entity,有利于提高缓存的命中率** **

因此我们优选出来的进程必须同last和next指针域进行对比, 其实就是检查就绪队列中的最优进程, 即红黑树中最左节点last是否可以抢占last和next指针域, 检查是否可以抢占是通过wakeup_preempt_entity函数来完成的.


**wakeup_preempt_entity检查是否可以被抢占**


```c
//  http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L5317
/*
 * Should 'se' preempt 'curr'.
 *
 *         |s1
 *    |s2
 *   |s3
 *     g
 *      |<--->|c
 *
 *  w(c, s1) = -1
 *  w(c, s2) =  0
 *  w(c, s3) =  1
 *
 */
static int
wakeup_preempt_entity(struct sched_entity *curr, struct sched_entity *se)
{
    /*  vdiff为curr和se vruntime的差值*/
    s64 gran, vdiff = curr->vruntime - se->vruntime;

    /*  cfs_rq的vruntime是单调递增的，也就是一个基准
     *  各个进程的vruntime追赶竞争cfsq的vruntime
     *  如果curr的vruntime比较小, 说明curr更加需要补偿, 
     *  即se无法抢占curr */
    if (vdiff <= 0)
        return -1;

    /*  计算curr的最小抢占期限粒度   */
    gran = wakeup_gran(curr, se);
    /*  当差值大于这个最小粒度的时候才抢占，这可以避免频繁抢占  */
    if (vdiff > gran)
        return 1;

    return 0;
}


//  http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L5282
static unsigned long
wakeup_gran(struct sched_entity *curr, struct sched_entity *se)
{
    /*  NICE_0_LOAD的基准最小运行期限  */
    unsigned long gran = sysctl_sched_wakeup_granularity;

    /*
     * Since its curr running now, convert the gran from real-time
     * to virtual-time in his units.
     *
     * By using 'se' instead of 'curr' we penalize light tasks, so
     * they get preempted easier. That is, if 'se' < 'curr' then
     * the resulting gran will be larger, therefore penalizing the
     * lighter, if otoh 'se' > 'curr' then the resulting gran will
     * be smaller, again penalizing the lighter task.
     *
     * This is especially important for buddies when the leftmost
     * task is higher priority than the buddy.
     *
     * 计算进程运行的期限，即抢占的粒度
     */
    return calc_delta_fair(gran, se);
}
```

到底能不能选择last和next两个进程, 则是wakeup_preempt_entity函数来决定的, 看下面的图解即可：

![last进程next进程和left进程的比较](./images/last_next_left.png)

*	如果S3是left，curr是next或者last，left的vruntime值小于curr和next, 函数wakeup_preempt_entity肯定返回1，那么就说明next和last指针的vruntime和left差距过大，这个时候没有必要选择这个last或者next指针，而是应该优先补偿left

*	如果next或者last是S2，S1，那么vruntime和left差距并不大，并没有超过sysctl_sched_wakeup_granularity ，那么这个next或者last就可以被优先选择，而代替了left

而清除last和next这两个指针的时机有这么几个：

*	sched_tick的时候, 如果一个进程的运行时间超过理论时间（这个时间是根据load和cfs_rq的load, 平均分割sysctl_sched_latency的时间）, 那么如果next或者last指针指向这个正在运行的进程, 需要清除这个指针, 使得pick sched_entity不会因为next或者last指针再次选择到这个sched_entity

*	当一个sched_entity调度实体dequeue出运行队列，那么如果有next或者last指针指向这个sched_entity, 那么需要删除这个next或者last指针。

*	刚才说的那种case，如果next，last指针在pick的时候被使用了一次，那么这次用完了指针，需要清除相应的指针，避免使用过的next，last指针影响到下次pick

*	当进程yield操作的时候，进程主动放弃了调度机会，那么如果next，last指针指向了这个sched_entity，那么需要清除相应指针。


**pick_next_entity流程总结**

pick_next_entity函数选择出下一个最渴望被公平调度器调度的进程, 函数的执行流程其实很简单

1.	先从最左节点left和当前节点curr中选择出最渴望被调度(即虚拟运行vruntime最小)的那个调度实体色

2.	判断第一步优选出的调度实体se是不是cfs_rq中被跳过调度的那个进程skip, 如果是则可能需要继续优选红黑树次左节点

	*	如果se == curr == skip则需要跳过curr选择最左的那个调度实体second = left = __pick_first_entity(cfs_rq);

    *	否则se == left == skip, 则从次优的调度实体second和curr中选择最优的那个进程

3.	检查left是否可以抢占last和next调度实体, 此项有助于提高缓存的命中率

*	cfs_rq的last和next指针, last表示最后一个执行wakeup的sched_entity, next表示最后一个被wakeup的sched_entity, 在pick新sched_entity的时候，会优先选择这些last或者next指针的sched_entity,有利于提高缓存的命中率



于是我们会发现, 下一个将要被调度的调度实体或者进程, 总是下列几个调度实体之一

| 调度实体 | 描述 |
|:-------:|:-------:|
| left = __pick_first_entity(cfs_rq) | **红黑树的最左节点**, 这个节点拥有当前队列中vruntime最小的特性, 即应该优先被调度 |
| second = __pick_first_entity(left) | **红黑树的次左节点**, 为什么这个节点也可能呢, 因为内核支持skip跳过某个进程的抢占权力的, 如果left被标记为skip(由cfs_rq->skip域指定), 那么可能就需要找到次优的那个进程 |
| cfs_rq的curr结点 | curr节点的vruntime可能比left和second更小, 但是由于它正在运行, 因此它不在红黑树中(进程抢占物理机的时候对应节点同时会从红黑树中删除), 但是如果其vruntime足够小, 意味着cfs调度器应该尽可能的补偿curr进程, 让它再次被调度, 同样这种优化也有助于提高缓存的命中率 |
|cfs_rq的last或者next |  last表示最后一个执行wakeup的sched_entity, next表示最后一个被wakeup的sched_entity, 在pick新sched_entity的时候，会优先选择这些last或者next指针的sched_entity,有利于提高缓存的命中率 |

即红黑树中的最左结点left和次左结点second(检查两个节点是因为cfs_rq的skip指针域标识了内核需要跳过不调度的实体信息, 如果left被跳过, 则需要检查second)

以及cfs_rq的调度实体curr, last和next, curr是当前正在运行的进程, 它虽然已经运行, 但是可能仍然很饥渴, 那么我们应该继续补偿它, 而last表示最后一个执行wakeup的sched_entity, next表示最后一个被wakeup的sched_entity, 刚被唤醒的进程可能更希望得到CPU, 因此在pick新sched_entity的时候，会优先选择这些last或者next指针的sched_entity,有利于提高缓存的命中率

##  set_next_entity
-------


http://blog.csdn.net/sunnybeike/article/details/6918586