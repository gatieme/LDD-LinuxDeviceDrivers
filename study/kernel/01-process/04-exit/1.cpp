void do_exit(long code)
{
    struct task_struct *tsk = current;
    int group_dead;
    TASKS_RCU(int tasks_rcu_i);

    /*  1.    触发task_exit_nb通知链实例的处理函数
        http://lxr.free-electrons.com/source/kernel/profile.c?v=4.6#L138
      */
    profile_task_exit(tsk);

    /*
        http://lxr.free-electrons.com/source/kernel/kcov.c?v=4.6;#L104  
        https://lwn.net/Articles/671640
    */
    kcov_task_exit(tsk);

    /*  http://lxr.free-electrons.com/source/include/linux/blkdev.h?v=4.6;#L1095  */
    WARN_ON(blk_needs_flush_plug(tsk));

    /*  oops消息  
        中断上下文不能执行do_exit函数
        也不能终止PID为0的进程。
    */
    if (unlikely(in_interrupt()))
        panic("Aiee, killing interrupt handler!");
    if (unlikely(!tsk->pid))
        panic("Attempted to kill the idle task!");

    /*
     * If do_exit is called because this processes oopsed, it's possible
     * that get_fs() was left as KERNEL_DS, so reset it to USER_DS before
     * continuing. Amongst other possible reasons, this is to prevent
     * mm_release()->clear_child_tid() from writing to a user-controlled
     * kernel address.
     * 
     * 设定进程可以使用的虚拟地址的上限（用户空间） 
     * http://lxr.free-electrons.com/ident?v=4.6;i=set_fs
     */
    set_fs(USER_DS);
    /*  
        possibly stop for a ptrace event notification
        由于进程如果是在ptrace调试事件中终止的
        因此需要将信息发送给ptrace的进程
        函数定义在 http://lxr.free-electrons.com/source/include/linux/ptrace.h?v=4.6#L133  */
    ptrace_event(PTRACE_EVENT_EXIT, code);
    
    /*
        heck creds for do_exit()
        http://lxr.free-electrons.com/source/kernel/cred.c?v=4.6#L805  
    */
    validate_creds_for_do_exit(tsk);
    /*
     * We're taking recursive faults here in do_exit. Safest is to just
     * leave this task alone and wait for reboot.
     */
    if (unlikely(tsk->flags & PF_EXITING)) {
        pr_alert("Fixing recursive fault but reboot is needed!\n");
        /*
         * We can do this unlocked here. The futex code uses
         * this flag just to verify whether the pi state
         * cleanup has been done or not. In the worst case it
         * loops once more. We pretend that the cleanup was
         * done as there is no way to return. Either the
         * OWNER_DIED bit is set by now or we push the blocked
         * task into the wait for ever nirwana as well.
         */
        /*  设置进程标识为PF_EXITPIDONE*/
        tsk->flags |= PF_EXITPIDONE;
        set_current_state(TASK_UNINTERRUPTIBLE);
        schedule();
    }
    /*
        tsk->flags |= PF_EXITING;
        http://lxr.free-electrons.com/source/kernel/signal.c#L2383
    */
    exit_signals(tsk);  /* sets tsk->flags PF_EXITING */
    /*
     * tsk->flags are checked in the futex code to protect against
     * an exiting task cleaning up the robust pi futexes.
     */
    smp_mb();
    raw_spin_unlock_wait(&tsk->pi_lock);

    if (unlikely(in_atomic())) {
        pr_info("note: %s[%d] exited with preempt_count %d\n",
            current->comm, task_pid_nr(current),
            preempt_count());
        preempt_count_set(PREEMPT_ENABLED);
    }

    /* sync mm's RSS info before statistics gathering */
    if (tsk->mm)
        sync_mm_rss(tsk->mm);

    /*
        cct_update_integrals - update mm integral fields in task_struct
        更新进程的运行时间
        http://lxr.free-electrons.com/source/kernel/tsacct.c?v=4.6#L152
    */
    acct_update_integrals(tsk);
    group_dead = atomic_dec_and_test(&tsk->signal->live);
    if (group_dead) {
        hrtimer_cancel(&tsk->signal->real_timer);
        exit_itimers(tsk->signal);
        if (tsk->mm)
            setmax_mm_hiwater_rss(&tsk->signal->maxrss, tsk->mm);
    }
    acct_collect(code, group_dead);
    if (group_dead)
        tty_audit_exit();
    audit_free(tsk);

    tsk->exit_code = code;
    taskstats_exit(tsk, group_dead);

    /*  释放存储空间 
    放弃进程占用的mm,如果没有其他进程使用该mm，则释放它。  
     */
    exit_mm(tsk);

    if (group_dead)
        acct_process();
    trace_sched_process_exit(tsk);

    exit_sem(tsk);   /*  释放用户空间的“信号量”  */
    exit_shm(tsk);  /* 释放锁  */
    exit_files(tsk); /*  释放已经打开的文件   */
    exit_fs(tsk);   /*  释放用于表示工作目录等结构  */
    if (group_dead)
        disassociate_ctty(1);
    exit_task_namespaces(tsk);  /*  释放命名空间  */
    exit_task_work(tsk);
    exit_thread();      /*  释放task_struct中的thread_struct结构    */

    /*
     * Flush inherited counters to the parent - before the parent
     * gets woken up by child-exit notifications.
     *
     * because of cgroup mode, must be called before cgroup_exit()
     */
    /*  When a child task exits, feed back event values to parent events.  
    http://lxr.free-electrons.com/source/kernel/events/core.c#L8987  
    */ 
    perf_event_exit_task(tsk);

    cgroup_exit(tsk);

    /*
     * FIXME: do that only when needed, using sched_exit tracepoint
     */
    flush_ptrace_hw_breakpoint(tsk);

    TASKS_RCU(preempt_disable());
    TASKS_RCU(tasks_rcu_i = __srcu_read_lock(&tasks_rcu_exit_srcu));
    TASKS_RCU(preempt_enable());
    exit_notify(tsk, group_dead);
    proc_exit_connector(tsk);
#ifdef CONFIG_NUMA
    task_lock(tsk);
    mpol_put(tsk->mempolicy);
    tsk->mempolicy = NULL;
    task_unlock(tsk);
#endif
#ifdef CONFIG_FUTEX
    if (unlikely(current->pi_state_cache))
        kfree(current->pi_state_cache);
#endif
    /*
     * Make sure we are holding no locks:
     */
    debug_check_no_locks_held();
    /*
     * We can do this unlocked here. The futex code uses this flag
     * just to verify whether the pi state cleanup has been done
     * or not. In the worst case it loops once more.
     */
    tsk->flags |= PF_EXITPIDONE;

    if (tsk->io_context)
        exit_io_context(tsk);

    if (tsk->splice_pipe)
        free_pipe_info(tsk->splice_pipe);

    if (tsk->task_frag.page)
        put_page(tsk->task_frag.page);

    validate_creds_for_do_exit(tsk);

    check_stack_usage();
    preempt_disable();
    if (tsk->nr_dirtied)
        __this_cpu_add(dirty_throttle_leaks, tsk->nr_dirtied);
    exit_rcu();
    TASKS_RCU(__srcu_read_unlock(&tasks_rcu_exit_srcu, tasks_rcu_i));

    /*
     * The setting of TASK_RUNNING by try_to_wake_up() may be delayed
     * when the following two conditions become true.
     *   - There is race condition of mmap_sem (It is acquired by
     *     exit_mm()), and
     *   - SMI occurs before setting TASK_RUNINNG.
     *     (or hypervisor of virtual machine switches to other guest)
     *  As a result, we may become TASK_RUNNING after becoming TASK_DEAD
     *
     * To avoid it, we have to wait for releasing tsk->pi_lock which
     * is held by try_to_wake_up()
     */
    smp_mb();
    raw_spin_unlock_wait(&tsk->pi_lock);

    /* causes final put_task_struct in finish_task_switch(). */
    tsk->state = TASK_DEAD;
    tsk->flags |= PF_NOFREEZE;      /* tell freezer to ignore us */
    /*
        重新调度，因为该进程已经被设置成了僵死状态，因此永远都不会再把它调度回来运行了，也就实现了do_exit不会有返回的目标    */
    schedule();
    BUG();
    /* Avoid "noreturn function does return".  */
    for (;;)
        cpu_relax();    /* For when BUG is null */
}
EXPORT_SYMBOL_GPL(do_exit);