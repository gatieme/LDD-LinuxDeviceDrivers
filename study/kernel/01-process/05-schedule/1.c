

您好，需要报到证原件、存档机构出具同意改派涵、和新单位当地人社局接收函。 

4006165567

1.报到证原件； 
2.原就业单位放行（解约）材料原件； 
3.接收单位出具的接收材料原件。




    您好
    我是哈尔滨学院2015年的毕业生, 籍贯"湖北荆州", 户口也在家里, 目前在北京工作, 想把户口落到武汉去

    目前在北京工作,  当时派遣证派遣到了荆州, 
    但是我想先在北京工作几年, 然后去武汉工作, 因此最近想先把户口落到武汉, 
    打电话问武汉那边, 人家说需要派遣证, 而且是到武汉人才市场

    因此我想问下您这边, 重新开派遣证需要什么材料




static void __sched notrace __schedule(bool preempt)
{
    struct task_struct *prev, *next;
    unsigned long *switch_count;
    struct rq *rq;
    int cpu;

    /*  找到当前cpu上的就绪队列rq
        并将正在运行的进程curr保存到prev中  */
    cpu = smp_processor_id();
    rq = cpu_rq(cpu);
    prev = rq->curr;

    /*
     * do_exit() calls schedule() with preemption disabled as an exception;
     * however we must fix that up, otherwise the next task will see an
     * inconsistent (higher) preempt count.
     *
     * It also avoids the below schedule_debug() test from complaining
     * about this.
     */
    if (unlikely(prev->state == TASK_DEAD))
        preempt_enable_no_resched_notrace();

    schedule_debug(prev);

    if (sched_feat(HRTICK))
        hrtick_clear(rq);

    local_irq_disable();
    rcu_note_context_switch();

    /*
     * Make sure that signal_pending_state()->signal_pending() below
     * can't be reordered with __set_current_state(TASK_INTERRUPTIBLE)
     * done by the caller to avoid the race with signal_wake_up().
     */
    smp_mb__before_spinlock();
    raw_spin_lock(&rq->lock);
    lockdep_pin_lock(&rq->lock);

    rq->clock_skip_update <<= 1; /* promote REQ to ACT */

    switch_count = &prev->nivcsw;
    /*
     *  scheduler检查prev的状态state和内核抢占表示
     *  如果prev是不可运行的, 并且在内核态没有被抢占
     *  那么我们就应该将进程prev从就绪队列rq中删除, 
     *  如果当前进程prev有非阻塞等待信号，并且它的状态是TASK_INTERRUPTIBLE  
     *  则配置其状态为TASK_RUNNING, 并且把他留在rq中
     */

    /*  如果内核态没有被抢占，并且内核抢占有效 */
    if (!preempt && prev->state)
    {

        /*  如果当前进程有非阻塞等待信号，并且它的状态是TASK_INTERRUPTIBLE  */
        if (unlikely(signal_pending_state(prev->state, prev)))
        {
            /*  将当前进程的状态设为：TASK_RUNNING  */
            prev->state = TASK_RUNNING;
        }
        else   /*  否则需要将prev进程从就绪队列中删除*/
        {
            /*  将当前进程从runqueue(运行队列)中删除  */
            deactivate_task(rq, prev, DEQUEUE_SLEEP);

            /*  标识当前进程不在runqueue中  */
            prev->on_rq = 0;

            /*
             * If a worker went to sleep, notify and ask workqueue
             * whether it wants to wake up a task to maintain
             * concurrency.
             */
            if (prev->flags & PF_WQ_WORKER) {
                struct task_struct *to_wakeup;

                to_wakeup = wq_worker_sleeping(prev);
                if (to_wakeup)
                    try_to_wake_up_local(to_wakeup);
            }
        }
        switch_count = &prev->nvcsw;
    }

    if (task_on_rq_queued(prev))
        update_rq_clock(rq);

    next = pick_next_task(rq, prev);
    clear_tsk_need_resched(prev);
    clear_preempt_need_resched();
    rq->clock_skip_update = 0;

    if (likely(prev != next)) {
        rq->nr_switches++;
        rq->curr = next;
        ++*switch_count;

        trace_sched_switch(preempt, prev, next);
        rq = context_switch(rq, prev, next); /* unlocks the rq */
    }
    else
    {
        lockdep_unpin_lock(&rq->lock);
        raw_spin_unlock_irq(&rq->lock);
    }

    balance_callback(rq);
}
STACK_FRAME_NON_STANDARD(__schedule); /* switch_to() */