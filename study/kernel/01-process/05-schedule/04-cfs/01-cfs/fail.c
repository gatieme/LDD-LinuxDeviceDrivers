8518 /*
8519  * All the scheduling class methods:
8520  */
8521 const struct sched_class fair_sched_class = {
8522         .next                   = &idle_sched_class,
8523         .enqueue_task           = enqueue_task_fair,
8524         .dequeue_task           = dequeue_task_fair,
8525         .yield_task             = yield_task_fair,
8526         .yield_to_task          = yield_to_task_fair,
8527 
8528         .check_preempt_curr     = check_preempt_wakeup,
8529 
8530         .pick_next_task         = pick_next_task_fair,
8531         .put_prev_task          = put_prev_task_fair,
8532 
8533 #ifdef CONFIG_SMP
8534         .select_task_rq         = select_task_rq_fair,
8535         .migrate_task_rq        = migrate_task_rq_fair,
8536 
8537         .rq_online              = rq_online_fair,
8538         .rq_offline             = rq_offline_fair,
8539 
8540         .task_waking            = task_waking_fair,
8541         .task_dead              = task_dead_fair,
8542         .set_cpus_allowed       = set_cpus_allowed_common,
8543 #endif
8544 
8545         .set_curr_task          = set_curr_task_fair,
8546         .task_tick              = task_tick_fair,
8547         .task_fork              = task_fork_fair,
8548 
8549         .prio_changed           = prio_changed_fair,
8550         .switched_from          = switched_from_fair,
8551         .switched_to            = switched_to_fair,
8552 
8553         .get_rr_interval        = get_rr_interval_fair,
8554 
8555         .update_curr            = update_curr_fair,
8556 
8557 #ifdef CONFIG_FAIR_GROUP_SCHED
8558         .task_move_group        = task_move_group_fair,
8559 #endif
8560 };