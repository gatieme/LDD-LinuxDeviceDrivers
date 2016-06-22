/*
 * Pick up the highest-prio task:
 */
static inline struct task_struct *
pick_next_task(struct rq *rq, struct task_struct *prev)
{
    const struct sched_class *class = &fair_sched_class;
    struct task_struct *p;

    /*
     * Optimization: we know that if all tasks are in
     * the fair class we can call that function directly:
     *
     * 如果待被调度的进程prev是隶属于CFS的普通非实时进程
     * 而当前cpu的全局就绪队列rq中的进程数与cfs_rq的进程数相等
     * 则说明当前cpu上的所有进程都是由cfs调度的普通非实时进程
     *
     * 那么我们选择最优进程的时候
     * 就只需要调用cfs调度器类fair_sched_class的选择函数pick_next_task
     * 就可以找到最优的那个进程p
     */
    /*  如果当前所有的进程都被cfs调度, 没有实时进程  */
    if (likely(prev->sched_class == class &&
           rq->nr_running == rq->cfs.h_nr_running))
    {
        /*  调用cfs的选择函数pick_next_task找到最优的那个进程p*/
        p = fair_sched_class.pick_next_task(rq, prev);
        /*  #define RETRY_TASK ((void *)-1UL)有被其他调度气找到合适的进程  */
        if (unlikely(p == RETRY_TASK))
            goto again; /*  则遍历所有的调度器类找到最优的进程 */

        /* assumes fair_sched_class->next == idle_sched_class */
        if (unlikely(!p))   /*  如果没有进程可被调度  */
            p = idle_sched_class.pick_next_task(rq, prev); /*  则调度idle进程  */

        return p;
    }

/*  进程中所有的调度器类, 是通过next域链接域链接在一起的
 *  调度的顺序为stop -> dl -> rt -> fair -> idle 
 *  again出的循环代码会遍历他们找到一个最优的进程  */
again:
    for_each_class(class)
    {
        p = class->pick_next_task(rq, prev);
        if (p)
        {
            if (unlikely(p == RETRY_TASK))
                goto again;
            return p;
        }
    }

    BUG(); /* the idle class will always have a runnable task */
}