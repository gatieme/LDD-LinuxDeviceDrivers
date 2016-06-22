asmlinkage __visible void __sched schedule(void)
{

    /*  获取当前的进程  */
    struct task_struct *tsk = current;

    /*  避免死锁 */
    sched_submit_work(tsk);
    do {
        preempt_disable();                                  /*  关闭内核抢占  */
        __schedule(false);                                  /*  完成调度  */
        sched_preempt_enable_no_resched();                  /*  开启内核抢占  */
    } while (need_resched());   /*  如果该进程被其他进程设置了TIF_NEED_RESCHED标志，则函数重新执行进行调度    */
}
EXPORT_SYMBOL(schedule);
