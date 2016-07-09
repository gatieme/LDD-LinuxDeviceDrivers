/*
 * called on fork with the child task as argument from the parent's context
 *  - child not yet on the tasklist
 *  - preemption disabled
 */
static void task_fork_fair(struct task_struct *p)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se, *curr;
    int this_cpu = smp_processor_id();
    struct rq *rq = this_rq();
    unsigned long flags;

    raw_spin_lock_irqsave(&rq->lock, flags);

    update_rq_clock(rq);

    cfs_rq = task_cfs_rq(current);
    curr = cfs_rq->curr;

    /*
     * Not only the cpu but also the task_group of the parent might have
     * been changed after parent->se.parent,cfs_rq were copied to
     * child->se.parent,cfs_rq. So call __set_task_cpu() to make those
     * of child point to valid ones.
     */
    rcu_read_lock();
    __set_task_cpu(p, this_cpu);
    rcu_read_unlock();

    /*  更新统计量  */
    update_curr(cfs_rq);

    if (curr)
        se->vruntime = curr->vruntime;
    /*  调整调度实体se的虚拟运行时间  */
    place_entity(cfs_rq, se, 1);

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

    se->vruntime -= cfs_rq->min_vruntime;

    raw_spin_unlock_irqrestore(&rq->lock, flags);
}