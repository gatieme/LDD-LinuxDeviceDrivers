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

周期性调度器的工作由scheduler_tick函数完成(定义在[kernel/sched/core.c, line 2910](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L2910)), 在scheduler_tick中周期性调度器通过调用curr进程所属调度器类sched_class的task_tick函数完成周期性调度的工作

周期调度的工作形式上sched_class调度器类的task_tick函数完成, CFS则对应task_tick_fair函数, 但实际上工作交给entity_tick完成.


#2	CFS的周期性调度
-------

##2.1	task_tick_fair与周期性调度
-------

CFS完全公平调度器类通过task_tick_fair函数完成周期性调度的工作, 该函数定义在[kernel/sched/fair.c?v=4.6#L8119](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L8119)

```c
/*
 * scheduler tick hitting a task of our scheduling class:
 */
static void task_tick_fair(struct rq *rq, struct task_struct *curr, int queued)
{
    struct cfs_rq *cfs_rq;
    /*  获取到当前进程curr所在的调度实体  */
    struct sched_entity *se = &curr->se;

    /* for_each_sched_entity
     * 在不支持组调度条件下, 只循环一次
     * 在组调度的条件下, 调度实体存在层次关系,
     * 更新子调度实体的同时必须更新父调度实体  */
    for_each_sched_entity(se)
    {
        /*  获取当当前运行的进程所在的CFS就绪队列  */
        cfs_rq = cfs_rq_of(se);
        /*  完成周期性调度  */
        entity_tick(cfs_rq, se, queued);
    }

    if (static_branch_unlikely(&sched_numa_balancing))
        task_tick_numa(rq, curr);
}
```

我们可以看到, CFFS周期性调度的功能实际上是委托给entity_tick函数来完成的


##2.2	entity_tick函数
-------

在task_tick_fair中, 内核将CFS周期性调度的实际工作交给了entity_tick来完成, 该函数定义在[kernel/sched/fair.c, line 3470](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L3470)中, 如下所示

```c
static void
entity_tick(struct cfs_rq *cfs_rq, struct sched_entity *curr, int queued)
{
    /*
     * Update run-time statistics of the 'current'.
     */
    update_curr(cfs_rq);

    /*
     * Ensure that runnable average is periodically updated.
     */
    update_load_avg(curr, 1);
    update_cfs_shares(cfs_rq);

#ifdef CONFIG_SCHED_HRTICK
    /*
     * queued ticks are scheduled to match the slice, so don't bother
     * validating it and just reschedule.
     */
    if (queued) {
        resched_curr(rq_of(cfs_rq));
        return;
    }
    /*
     * don't let the period tick interfere with the hrtick preemption
     */
    if (!sched_feat(DOUBLE_TICK) &&
            hrtimer_active(&rq_of(cfs_rq)->hrtick_timer))
        return;
#endif

    if (cfs_rq->nr_running > 1)
        check_preempt_tick(cfs_rq, curr);
}
```


首先, 一如既往的使用update_curr来更新统计量

接下来是hrtimer的更新, 这些由内核通过参数CONFIG_SCHED_HRTICK开启

然后如果cfs就绪队列中进程数目nr_running少于两个(< 2)则实际上无事可做. 因为如果某个进程应该被抢占, 那么至少需要有另一个进程能够抢占它(即cfs_rq->nr_running > 1)

如果进程的数目不少于两个, 则由check_preempt_tick作出决策

```c
    if (cfs_rq->nr_running > 1)
        check_preempt_tick(cfs_rq, curr);
```

##2.3	check_preempt_tick函数
-------

在entity_tick中, 如果cfs的就绪队列中进程数目不少于2, 说明至少需要有另外一个进程能够抢占当前进程, 此时内核交给check_preempt_tick作出决策. check_preempt_tick函数定义在[kernel/sched/fair.c, line 3308](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L3308)


```c
/*
 * Preempt the current task with a newly woken task if needed:
 */
static void
check_preempt_tick(struct cfs_rq *cfs_rq, struct sched_entity *curr)
{
    unsigned long ideal_runtime, delta_exec;
    struct sched_entity *se;
    s64 delta;

    /*  计算curr的理论上应该运行的时间  */
    ideal_runtime = sched_slice(cfs_rq, curr);

    /*  计算curr的实际运行时间
     *  sum_exec_runtime: 进程执行的总时间
     *  prev_sum_exec_runtime:进程在切换进CPU时的sum_exec_runtime值  */
    delta_exec = curr->sum_exec_runtime - curr->prev_sum_exec_runtime;

    /*  如果实际运行时间比理论上应该运行的时间长
     *  说明curr进程已经运行了足够长的时间
     *  应该调度新的进程抢占CPU了  */
    if (delta_exec > ideal_runtime)
    {
        resched_curr(rq_of(cfs_rq));
        /*
         * The current task ran long enough, ensure it doesn't get
         * re-elected due to buddy favours.
         */
        clear_buddies(cfs_rq, curr);
        return;
    }

    /*
     * Ensure that a task that missed wakeup preemption by a
     * narrow margin doesn't have to wait for a full slice.
     * This also mitigates buddy induced latencies under load.
     */
    if (delta_exec < sysctl_sched_min_granularity)
        return;

    se = __pick_first_entity(cfs_rq);
    delta = curr->vruntime - se->vruntime;

    if (delta < 0)
        return;

    if (delta > ideal_runtime)
        resched_curr(rq_of(cfs_rq));
}
```


check_preempt_tick函数的目的在于, 判断是否需要抢占当前进程. 确保没有哪个进程能够比延迟周期中确定的份额运行得更长. 该份额对应的实际时间长度在sched_slice中计算.

而上一节我们提到, 进程在CPU上已经运行的实际时间间隔由sum_exec_runtime - prev_sum_runtime给出.


>还记得上一节， 在set_next_entity函数的最后, 将选择出的调度实体se的sum_exec_runtime保存在了prev_sum_exec_runtime中, 因为该调度实体指向的进程, 马上将抢占处理器成为当前活动进程, 在CPU上花费的实际时间将记入sum_exec_runtime, 因此内核会在prev_sum_exec_runtime保存此前的设置. 要注意进程中的sum_exec_runtime没有重置. 因此差值sum_exec_runtime - prev_sum_runtime确实标识了在CPU上执行花费的实际时间.
>
>在处理周期性调度时, 这个差值就显得格外重要


因此抢占决策很容易做出决定, 如果检查发现当前进程运行需要被抢占, 那么通过resched_task发出重调度请求. 这会在task_struct中设置TIF_NEED_RESCHED标志, 核心调度器会在下一个适当的时机发起重调度.


其实需要抢占的条件有下面两种可能性

*	curr进程的实际运行时间delta_exec比期望的时间间隔ideal_runtime长

	此时说明curr进程已经运行了足够长的时间

*	curr进程与红黑树中最左进程left虚拟运行时间的差值大于curr的期望运行时间ideal_runtime

	此时说明红黑树中最左结点left与curr节点更渴望处理器, 已经接近于饥饿状态, 这个我们可以这样理解, 相对于curr进程来说, left进程如果参与调度, 其期望运行时间应该域curr进程的期望时间ideal_runtime相差不大, 而此时如果curr->vruntime - se->vruntime > curr.ideal_runtime, 我们可以初略的理解为curr进程已经优先于left进程多运行了一个周期, 而left又是红黑树总最饥渴的那个进程, 因此curr进程已经远远领先于队列中的其他进程, 此时应该补偿其他进程。


如果检查需要发生抢占, 则内核通过resched_curr(rq_of(cfs_rq))设置重调度标识, 从而触发延迟调度




##2.4  resched_curr设置重调度标识TIF_NEED_RESCHED
-------


周期性调度器并不显式进行调度, 而是采用了延迟调度的策略, 如果发现需要抢占, 周期性调度器就设置进程的重调度标识TIF_NEED_RESCHED, 然后由主调度器完成调度工作.

>TIF_NEED_RESCHED标识, 表明进程需要被调度, TIF前缀表明这是一个存储在进程thread_info中flag字段的一个标识信息
>
>在内核的一些关键位置, 会检查当前进程是否设置了重调度标志TLF_NEDD_RESCHED, 如果该进程被其他进程设置了TIF_NEED_RESCHED标志, 则函数重新执行进行调度

前面我们在check_preempt_tick中如果发现curr进程已经运行了足够长的时间, 其他进程已经开始饥饿, 那么我们就需要通过resched_curr来设置重调度标识TIF_NEED_RESCHED


resched_curr函数定义在[kernel/sched/core.c, line 446](http://lxr.free-electrons.com/source/kernel/sched/core.c#L446)中, 并没有什么复杂的工作, 其实就是通过set_tsk_need_resched(curr);函数设置重调度标识



#3	总结
-------


周期性调度器的工作由**scheduler_tick函数**完成(定义在[kernel/sched/core.c, line 2910](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L2910)), 在scheduler_tick中周期性调度器通过调用curr进程所属调度器类sched_class的task_tick函数完成周期性调度的工作

周期调度的工作形式上sched_class调度器类的task_tick函数完成, CFS则对应**task_tick_fair函数**, 但实际上工作交给**entity_tick**完成.

而entity_tick中则通过**check_preempt_tick**函数检查是否需要抢占当前进程curr, 如果发现curr进程已经运行了足够长的时间, 其他进程已经开始饥饿, 那么我们就需要通过**resched_curr**函数来设置重调度标识TIF_NEED_RESCHED


其中check_preempt_tick检查可抢占的条件如下

*	curr进程的实际运行时间delta_exec比期望的时间间隔ideal_runtime长, 此时说明curr进程已经运行了足够长的时间

*	curr进程与红黑树中最左进程left虚拟运行时间的差值大于curr的期望运行时间ideal_runtime, 此时我们可以理解为curr进程已经优先于left进程多运行了一个周期, 而left又是红黑树总最饥渴的那个进程, 因此curr进程已经远远领先于队列中的其他进程, 此时应该补偿其他进程