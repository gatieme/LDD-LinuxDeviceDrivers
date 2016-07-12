//  http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L701
/*
 * Update the current task's runtime statistics.
 */
static void update_curr(struct cfs_rq *cfs_rq)
{
    /*  ==1==  */
    /*  确定就绪队列的当前执行进程curr  */
    struct sched_entity *curr = cfs_rq->curr;
    /*  rq_of -=> return cfs_rq->rq 返回cfs队列所在的全局就绪队列  
     *  rq_clock_task返回了rq的clock_task  */
    u64 now = rq_clock_task(rq_of(cfs_rq));
    u64 delta_exec;

    /*   如果就队列上没有进程在执行, 则显然无事可做  */
    if (unlikely(!curr))
        return;

    /*  内核计算当前和上一次更新负荷权重时两次的时间的差值 */
    delta_exec = now - curr->exec_start;
    if (unlikely((s64)delta_exec <= 0))
        return;

    /*  重新更新启动时间exec_start为now  */
    curr->exec_start = now;

    schedstat_set(curr->statistics.exec_max,
              max(delta_exec, curr->statistics.exec_max));

    /*  将时间差加到先前统计的时间即可  */
    curr->sum_exec_runtime += delta_exec;
    schedstat_add(cfs_rq, exec_clock, delta_exec);

    /*  ==2==  */
    curr->vruntime += calc_delta_fair(delta_exec, curr);
    update_min_vruntime(cfs_rq);

    if (entity_is_task(curr)) {
        struct task_struct *curtask = task_of(curr);

        trace_sched_stat_runtime(curtask, delta_exec, curr->vruntime);
        cpuacct_charge(curtask, delta_exec);
        account_group_exec_runtime(curtask, delta_exec);
    }

    account_cfs_rq_runtime(cfs_rq, delta_exec);
}


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


//  http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L214
/*
 * delta_exec * weight / lw.weight
 *   OR
 * (delta_exec * (weight * lw->inv_weight)) >> WMULT_SHIFT
 *
 * Either weight := NICE_0_LOAD and lw \e prio_to_wmult[], in which case
 * we're guaranteed shift stays positive because inv_weight is guaranteed to
 * fit 32 bits, and NICE_0_LOAD gives another 10 bits; therefore shift >= 22.
 *
 * Or, weight =< lw.weight (because lw.weight is the runqueue weight), thus
 * weight/lw.weight <= 1, and therefore our shift will also be positive.
 */
static u64 __calc_delta(u64 delta_exec, unsigned long weight, struct load_weight *lw)
{
    u64 fact = scale_load_down(weight);
    int shift = WMULT_SHIFT;

    __update_inv_weight(lw);

    if (unlikely(fact >> 32)) {
        while (fact >> 32) {
            fact >>= 1;
            shift--;
        }
    }

    /* hint to use a 32x32->64 mul */
    fact = (u64)(u32)fact * lw->inv_weight;

    while (fact >> 32) {
        fact >>= 1;
        shift--;
    }

    return mul_u64_u32_shr(delta_exec, fact, shift);
}


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

    /* ensure we never gain time by being placed backwards. */
    cfs_rq->min_vruntime = max_vruntime(cfs_rq->min_vruntime, vruntime);
#ifndef CONFIG_64BIT
    smp_wmb();
    cfs_rq->min_vruntime_copy = cfs_rq->min_vruntime;
#endif
}


if (cfs_rq->curr != NULL)
    vruntime = cfs_rq->curr->vruntime;
else if(cfs_rq->curr == NULL)
    vruntime = cfs_rq->min_vruntime;

if(cfs_rq->rb_leftmost != NULL)
{
    if (cfs_rq->curr == NULL)  /*  此时vruntime的原值为cfs_rq->min_vruntime*/
        vruntime = se->vruntime;
    else                /* 此时vruntime的原值为cfs_rq->curr->vruntime*/
        vruntime = min_vruntime(vruntime, se->vruntime);
}
cfs_rq->min_vruntime = max_vruntime(cfs_rq->min_vruntime, vruntime);


if (cfs_rq->curr != NULL cfs_rq->rb_leftmost == NULL)
    vruntime = cfs_rq->curr->vruntime;
else if(cfs_rq->curr == NULL && cfs_rq->rb_leftmost != NULL)
        vruntime = cfs_rq->rb_leftmost->se->vruntime;
else if (cfs_rq->curr != NULL cfs_rq->rb_leftmost != NULL)
    vruntime = min(cfs_rq->curr->vruntime, cfs_rq->rb_leftmost->se->vruntime);
else if(cfs_rq->curr == NULL cfs_rq->rb_leftmost == NULL)
    vruntime = cfs_rq->min_vruntime;
cfs_rq->min_vruntime = max_vruntime(cfs_rq->min_vruntime, vruntime);
