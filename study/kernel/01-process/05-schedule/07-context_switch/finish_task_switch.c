/**
 * finish_task_switch - clean up after a task-switch
 * @prev: the thread we just switched away from.
 *
 * finish_task_switch must be called after the context switch, paired
 * with a prepare_task_switch call before the context switch.
 * finish_task_switch will reconcile locking set up by prepare_task_switch,
 * and do any other architecture-specific cleanup actions.
 *
 * Note that we may have delayed dropping an mm in context_switch(). If
 * so, we finish that here outside of the runqueue lock. (Doing it
 * with the lock held can cause deadlocks; see schedule() for
 * details.)
 *
 * The context switch have flipped the stack from under us and restored the
 * local variables which were saved when this task called schedule() in the
 * past. prev == current is still correct but we need to recalculate this_rq
 * because prev may have moved to another CPU.
 */
static struct rq *finish_task_switch(struct task_struct *prev)
        __releases(rq->lock)
{
        struct rq *rq = this_rq();
        struct mm_struct *mm = rq->prev_mm;
        long prev_state;

        /*
         * The previous task will have left us with a preempt_count of 2
         * because it left us after:
         *
         *      schedule()
         *        preempt_disable();                    // 1
         *        __schedule()
         *          raw_spin_lock_irq(&rq->lock)        // 2
         *
         * Also, see FORK_PREEMPT_COUNT.
         */
        if (WARN_ONCE(preempt_count() != 2*PREEMPT_DISABLE_OFFSET,
                      "corrupted preempt_count: %s/%d/0x%x\n",
                      current->comm, current->pid, preempt_count()))
                preempt_count_set(FORK_PREEMPT_COUNT);

        rq->prev_mm = NULL;

        /*
         * A task struct has one reference for the use as "current".
         * If a task dies, then it sets TASK_DEAD in tsk->state and calls
         * schedule one last time. The schedule call will never return, and
         * the scheduled task must drop that reference.
         *
         * We must observe prev->state before clearing prev->on_cpu (in
         * finish_lock_switch), otherwise a concurrent wakeup can get prev
         * running on another CPU and we could rave with its RUNNING -> DEAD
         * transition, resulting in a double drop.
         */
        prev_state = prev->state;
        vtime_task_switch(prev);
        perf_event_task_sched_in(prev, current);
        finish_lock_switch(rq, prev);
        finish_arch_post_lock_switch();

        fire_sched_in_preempt_notifiers(current);
        if (mm)
                mmdrop(mm);
        if (unlikely(prev_state == TASK_DEAD))  /*  如果上一个进程已经终止，释放其task_struct 结构  */
        {
                if (prev->sched_class->task_dead)
                        prev->sched_class->task_dead(prev);

                /*
                 * Remove function-return probe instances associated with this
                 * task and put them back on the free list.
                 */
                kprobe_flush_task(prev);
                put_task_struct(prev);
        }

        tick_nohz_task_switch();
        return rq;
}