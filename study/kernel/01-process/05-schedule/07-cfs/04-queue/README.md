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

##1.2	负荷权重和虚拟时钟

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


##1.3	今日内容--CFS进程入队和出队
-------


完全公平调度器CFS中有两个函数可用来增删队列的成员:enqueue_task_fair和dequeue_task_fair分别用来向CFS就绪队列中添加或者删除进程


#2 enqueue_task_fair入队操作
-------

##2.1	enque_task_fair函数
-------

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



##2.2	enque_task_fair完全函数
-------


```c
/*
 * The enqueue_task method is called before nr_running is
 * increased. Here we update the fair scheduling stats and
 * then put the task into the rbtree:
 */
static void
enqueue_task_fair(struct rq *rq, struct task_struct *p, int flags)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se;

    for_each_sched_entity(se) {
        if (se->on_rq)
            break;
        cfs_rq = cfs_rq_of(se);
        enqueue_entity(cfs_rq, se, flags);

        /*
         * end evaluation on encountering a throttled cfs_rq
         *
         * note: in the case of encountering a throttled cfs_rq we will
         * post the final h_nr_running increment below.
        */
        if (cfs_rq_throttled(cfs_rq))
            break;
        cfs_rq->h_nr_running++;

        flags = ENQUEUE_WAKEUP;
    }

    for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
        cfs_rq->h_nr_running++;

        if (cfs_rq_throttled(cfs_rq))
            break;

        update_load_avg(se, 1);
        update_cfs_shares(cfs_rq);
    }

    if (!se)
        add_nr_running(rq, 1);

    hrtick_update(rq);
}
```


##2.3	for_each_sched_entity
-------

首先内核查找到待天机进程p所在的调度实体信息, 然后通过for_each_sched_entity循环所有调度实体,

```c
//  enqueue_task_fair函数
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se;

    for_each_sched_entity(se)
    {
    /*  ......  */
    }
}
```

但是有个疑问是, 进程p所在的调度时提就一个为嘛要循环才能遍历啊, 这是因为为了支持组调度.组调度下调度实体是有层次结构的, 我们将进程加入的时候, 同时要更新其父调度实体的调度信息, 而非组调度情况下, 就不需要调度实体的层次结构

linux对组调度的支持可以通过CONFIG_FAIR_GROUP_SCHED来启用, 在启用和不启用的条件下, 内核对很多函数的实现也会因条件而异, 这点对for_each_sched_entity函数尤为明显, 参见[启用CONFIG_FAIR_GROUP_SCHED](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L246)和[不启用CONFIG_FAIR_GROUP_SCHED](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L367)

```c
#ifdef CONFIG_FAIR_GROUP_SCHED

/* An entity is a task if it doesn't "own" a runqueue */
#define entity_is_task(se)      (!se->my_q)

//  http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L266
/* Walk up scheduling entities hierarchy */
#define for_each_sched_entity(se) \
		for (; se; se = se->parent)

 #else   /* !CONFIG_FAIR_GROUP_SCHED */

#define entity_is_task(se)      1

//  http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L381
#define for_each_sched_entity(se) \
                for (; se; se = NULL)
```

*	如果通过struct sched_entity的on_rq成员判断进程已经在就绪队列上, 则无事可做. 

*	否则, 具体的工作委托给enqueue_entity完成, 其中内核会借机用update_curr更新统计量.

```c
//  enqueue_task_fair函数
{
        /*  如果当前进程已经在就绪队列上  */
        if (se->on_rq)
            break;

        /*  获取到当前进程所在的cfs_rq就绪队列  */
        cfs_rq = cfs_rq_of(se);
        /*  内核委托enqueue_entity完成真正的插入工作  */
        enqueue_entity(cfs_rq, se, flags);
}
```


##2.4	enqueue_entity插入进程
-------

enqueue_entity完成了进程真正的入队操作, 其具体流程如下所示

*	更新一些统计统计量, update_curr, update_cfs_shares等

*	如果进程此前是在睡眠状态, 则调用place_entity中首先会调整进程的虚拟运行时间

*	最后如果进程最近在运行, 其虚拟运行时间仍然有效, 那么则直接用__enqueue_entity加入到红黑树

首先如果进程最近正在运行, 其虚拟时间时间仍然有效, 那么(除非它当前在执行中)它可以直接用__enqueue_entity插入到红黑树, 该函数徐娅萍处理一些红黑树的机制, 这可以依靠内核的标准实现, 参见[__enqueue_entity函数, kernel/sched/fair.c, line483](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#483)



```c
static void
enqueue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int flags)
{
    /*
     * Update the normalized vruntime before updating min_vruntime
     * through calling update_curr().
     *
     * 如果当前进程之前已经是可运行状态不是被唤醒的那么其虚拟运行时间要增加
     */
    if (!(flags & ENQUEUE_WAKEUP) || (flags & ENQUEUE_WAKING))
        se->vruntime += cfs_rq->min_vruntime;

    /*
     * Update run-time statistics of the 'current'.
     * 更新进程的统计量信息
     */
    update_curr(cfs_rq);
    enqueue_entity_load_avg(cfs_rq, se);
    account_entity_enqueue(cfs_rq, se);
    update_cfs_shares(cfs_rq);

    /*  如果当前进行之前在睡眠刚被唤醒  */
    if (flags & ENQUEUE_WAKEUP)
    {
        /*  调整进程的虚拟运行时间  */
        place_entity(cfs_rq, se, 0);
        if (schedstat_enabled())
            enqueue_sleeper(cfs_rq, se);
    }

    check_schedstat_required();
    if (schedstat_enabled()) {
        update_stats_enqueue(cfs_rq, se);
        check_spread(cfs_rq, se);
    }

    /*  将进程插入到红黑树中  */
    if (se != cfs_rq->curr)
        __enqueue_entity(cfs_rq, se);
    se->on_rq = 1;

    if (cfs_rq->nr_running == 1) {
        list_add_leaf_cfs_rq(cfs_rq);
        check_enqueue_throttle(cfs_rq);
    }
}
```

##2.5	place_entity处理睡眠进程
-------

如果进程此前在睡眠, 那么则调用place_entity处理其虚拟运行时间

设想一下子如果休眠进程的vruntime保持不变, 而其他运行进程的 vruntime一直在推进, 那么等到休眠进程终于唤醒的时候, 它的vruntime比别人小很多, 会使它获得长时间抢占CPU的优势, 其他进程就要饿死了. 这显然是另一种形式的不公平，因此CFS是这样做的：在休眠进程被唤醒时重新设置vruntime值，以min_vruntime值为基础，给予一定的补偿，但不能补偿太多. 这个重新设置其虚拟运行时间的工作就是就是通过place_entity来完成的, 另外新进程创建完成后, 也是通过place_entity完成其虚拟运行时间vruntime的设置的. place_entity通过其第三个参数initial来标识新进程创建和休眠进程苏醒两种不同情形的.



place_entity函数定义在[kernel/sched/fair.c, line 3135](http://lxr.free-electrons.com/source/kernel/sched/fair.c#L3135)中首先会调整进程的虚拟运行时间


```c
//  http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L3134
static void
place_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int initial)
{
    u64 vruntime = cfs_rq->min_vruntime;

    /*
     * The 'current' period is already promised to the current tasks,
     * however the extra weight of the new task will slow them down a
     * little, place the new task so that it fits in the slot that
     * stays open at the end.
     *
     * 如果是新进程第一次要入队, 那么就要初始化它的vruntime
     * 一般就把cfsq的vruntime给它就可以
     * 但是如果当前运行的所有进程被承诺了一个运行周期
     * 那么则将新进程的vruntime后推一个他自己的slice
     * 实际上新进程入队时要重新计算运行队列的总权值
     * 总权值显然是增加了，但是所有进程总的运行时期并不一定随之增加
     * 则每个进程的承诺时间相当于减小了，就是减慢了进程们的虚拟时钟步伐。 
     */
    /*  initial标识了该进程是新进程  */
    if (initial && sched_feat(START_DEBIT))
        vruntime += sched_vslice(cfs_rq, se);

    /* sleeps up to a single latency don't count. 
     * 休眠进程  */
    if (!initial)
    {
        /*  一个调度周期  */
        unsigned long thresh = sysctl_sched_latency;

        /*
         * Halve their sleep time's effect, to allow
         * for a gentler effect of sleepers:
         */
        /*  若设了GENTLE_FAIR_SLEEPERS  */
        if (sched_feat(GENTLE_FAIR_SLEEPERS))
            thresh >>= 1;   /*  补偿减为调度周期的一半  */

        vruntime -= thresh;
    }

    /* ensure we never gain time by being placed backwards.
     * 如果是唤醒已经存在的进程，则单调附值
     */
    se->vruntime = max_vruntime(se->vruntime, vruntime);
}
```

我们可以看到enqueue_task_fair调用place_entity传递的initial参数为0

```c
place_entity(cfs_rq, se, 0);
```

所以会执行if (!initial)后的语句。因为进程睡眠后，vruntime就不会增加了，当它醒来后不知道过了多长时间，可能vruntime已经比 min_vruntime小了很多，如果只是简单的将其插入到就绪队列中，它将拼命追赶min_vruntime，因为它总是在红黑树的最左面。如果这 样，它将会占用大量的CPU时间，导致红黑树右边的进程被饿死。但是我们又必须及时响应醒来的进程，因为它们可能有一些工作需要立刻处理，所以系统采取了 一种折衷的办法，将当前cfs_rq->min_vruntime时间减去sysctl_sched_latency赋给vruntime，这时它 会被插入到就绪队列的最左边。这样刚唤醒的进程在当前执行进程时间耗尽时就会被调度上处理器执行。当然如果进程没有睡眠那么多时间，我们只需保留原来的时 间vruntime = max_vruntime(se->vruntime, vruntime)。这有什么好处的，我觉得它可以将所有唤醒的进程排个队，睡眠越久的越快得到响应。


对于新进程创建时initial为1，所以它会执行`vruntime += sched_vslice(cfs_rq, se);`这句，而这里的vruntime就是当前CFS就绪队列的min_vruntime，新加进程应该在最近很快被调度，这样减少系统的响应时间，我们已经知道当前进程的vruntime越小，它在红黑树中就会越靠左，就会被很快调度到处理器上执行。但是，Linux内核需要根据新加入的进程的权重决策一下应该何时调度该进程，而不能任意进程都来抢占当前队列中靠左的进程，因为必须保证就绪队列中的所有进程尽量得到他们应得的时间响应， sched_vslice函数就将其负荷权重转换为等价的虚拟时间, 其定义在[kernel/sched/fair.c, line 626](http://lxr.free-electrons.com/source/kernel/sched/fair.c#L626)


函数就是根据initial的值来区分两种情况, 一般来说只有在新进程被加到系统中时,才会首次设置该参数,  但是这里的情况并非如此:

由于内核已经承诺在当前的延迟周期内使所有活动进程都至少运行一次, 队列的min_vruntime用作基准虚拟时间, 通过减去sysctl_sched_latency, 则可以确保新唤醒新唤醒的进程只有在当前延迟周期结束后才能运行.

但是如果进程在睡眠的过程中累积了比较大的不公平值(即se->vruntime值比较大), 则内核必须考虑这一点. 如果se->vruntime比先前的差值更大, 则将其作为进程的vruntime, 这会导致高进程在红黑树中处于靠左的位置, 而具有较小vruntime值得进程可以更早调度执行.




##2.6	__enqueue_entity完成红黑树的插入
-------

如果进程最近在运行, 其虚拟时间是有效的, 那么它可以直接通过__enqueue_entity加入到红黑树

```c
//  enqueue_entity函数解析
    /*  将进程插入到红黑树中  */
    if (se != cfs_rq->curr)
        __enqueue_entity(cfs_rq, se);
    se->on_rq = 1;
```

__enqueue_entity函数定义在[kernel/sched/fair.c, line 486](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L486)中, 其实就是一个机械性地红黑树插入操作

```c
/*
 * Enqueue an entity into the rb-tree:
 */
static void __enqueue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    struct rb_node **link = &cfs_rq->tasks_timeline.rb_node;
    struct rb_node *parent = NULL;
    struct sched_entity *entry;
    int leftmost = 1;

    /*
     * Find the right place in the rbtree:
     * 从红黑树中找到se所应该在的位置
     * 同时leftmost标识其位置是不是最左结点
     * 如果在查找结点的过程中向右走了, 则置leftmost为0
     * 否则说明一直再相左走, 最终将走到最左节点, 此时leftmost恒为1
     */
    while (*link) {
        parent = *link;
        entry = rb_entry(parent, struct sched_entity, run_node);
        /*
         * We dont care about collisions. Nodes with
         * the same key stay together.
         * 以se->vruntime值为键值进行红黑树结点的比较
         */
        if (entity_before(se, entry)) {
            link = &parent->rb_left;
        } else {
            link = &parent->rb_right;
            leftmost = 0;
        }
    }
    /*
     * Maintain a cache of leftmost tree entries (it is frequently
     * used):
     * 如果leftmost为1, 说明se是红黑树当前的最左结点, 即vruntime最小
     * 那么把这个节点保存在cfs就绪队列的rb_leftmost域中
     */
    if (leftmost)
        cfs_rq->rb_leftmost = &se->run_node;

	/*  将新进程的节点加入到红黑树中  */
    rb_link_node(&se->run_node, parent, link);
    /*  为新插入的结点进行着色  */
    rb_insert_color(&se->run_node, &cfs_rq->tasks_timeline);
}
```

#3	dequeue_task_fair出队操作
-------

dequeue_task_fair函数在完成睡眠等情况下调度, 将任务从就绪队列中移除

其执行的过程正好跟enqueue_task_fair的思路相同, 只是操作刚好相反


dequeue_task_fair的执行流程如下

*	如果通过struct sched_entity的on_rq成员判断进程已经在就绪队列上, 则无事可做. 

*	否则, 具体的工作委托给dequeue_entity完成, 其中内核会借机用update_curr更新统计量
	在enqueue_entity内部如果需要会调用__dequeue_entity将进程插入到CFS红黑树中合适的结点


dequeue_task_fair定义在[/kernel/sched/fair.c, line 4155](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v4.6#L4155), 其大致框架流程如下


##3.1  dequeue_task_fair函数
-------

```c
/*
 * The dequeue_task method is called before nr_running is
 * decreased. We remove the task from the rbtree and
 * update the fair scheduling stats:
 */
static void dequeue_task_fair(struct rq *rq, struct task_struct *p, int flags)
;

    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se;
    int task_sleep = flags & DEQUEUE_SLEEP;
		
    //   设置
    flags |= DEQUEUE_SLEEP;


    for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
        cfs_rq->h_nr_running--;

        if (cfs_rq_throttled(cfs_rq))
            break;

        update_load_avg(se, 1);
        update_cfs_shares(cfs_rq);
    }

    if (!se)
        sub_nr_running(rq, 1);

    hrtick_update(rq);
}
```

##3.2	dequeue_entity将调度实体出队
-------

```c
static void
dequeue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int flags)
{
    /*
     * Update run-time statistics of the 'current'.
     */
    update_curr(cfs_rq);
    dequeue_entity_load_avg(cfs_rq, se);

    if (schedstat_enabled())
        update_stats_dequeue(cfs_rq, se, flags);

    clear_buddies(cfs_rq, se);

    if (se != cfs_rq->curr)
        __dequeue_entity(cfs_rq, se);
    se->on_rq = 0;
    account_entity_dequeue(cfs_rq, se);

    /*
     * Normalize the entity after updating the min_vruntime because the
     * update can refer to the ->curr item and we need to reflect this
     * movement in our normalized position.
     */
    if (!(flags & DEQUEUE_SLEEP))
        se->vruntime -= cfs_rq->min_vruntime;

    /* return excess runtime on last dequeue */
    return_cfs_rq_runtime(cfs_rq);

    update_min_vruntime(cfs_rq);
    update_cfs_shares(cfs_rq);
}
```

##3.3	__dequeue_entity完成真正的出队操作
-------


```c
static void __dequeue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    if (cfs_rq->rb_leftmost == &se->run_node) {
        struct rb_node *next_node;

        next_node = rb_next(&se->run_node);
        cfs_rq->rb_leftmost = next_node;
    }

    rb_erase(&se->run_node, &cfs_rq->tasks_timeline);
}
```