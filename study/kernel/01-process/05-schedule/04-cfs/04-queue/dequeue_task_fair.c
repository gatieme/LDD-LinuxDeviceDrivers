/*
 * The dequeue_task method is called before nr_running is
 * decreased. We remove the task from the rbtree and
 * update the fair scheduling stats:
 */
static void dequeue_task_fair(struct rq *rq, struct task_struct *p, int flags)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se;
    int task_sleep = flags & DEQUEUE_SLEEP;

    for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
        dequeue_entity(cfs_rq, se, flags);

        /*
         * end evaluation on encountering a throttled cfs_rq
         *
         * note: in the case of encountering a throttled cfs_rq we will
         * post the final h_nr_running decrement below.
        */
        if (cfs_rq_throttled(cfs_rq))
            break;
        cfs_rq->h_nr_running--;

        /* Don't dequeue parent if it has other entities besides us */
        if (cfs_rq->load.weight) {
            /*
             * Bias pick_next to pick a task from this cfs_rq, as
             * p is sleeping when it is within its sched_slice.
             */
            if (task_sleep && parent_entity(se))
                set_next_buddy(parent_entity(se));

            /* avoid re-evaluating load for this entity */
            se = parent_entity(se);
            break;
        }
        flags |= DEQUEUE_SLEEP;
    }

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
