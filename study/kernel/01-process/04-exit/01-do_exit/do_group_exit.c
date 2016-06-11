/*
 * Take down every thread in the group.  This is called by fatal signals
 * as well as by sys_exit_group (below).
 */
void
do_group_exit(int exit_code)
{
    struct signal_struct *sig = current->signal;

    BUG_ON(exit_code & 0x80); /* core dumps don't get here */
    /*  
        检查current->sig->flags的SIGNAL_GROUP_EXIT标志是否置位
        或者current->sig->group_exit_task是否不为NULL
    */
    if (signal_group_exit(sig))
        exit_code = sig->group_exit_code;   /*  group_exit_code存放的是线程组终止代码  */
    else if (!thread_group_empty(current)) {    /*  检查线程组链表是否不为空  */
        struct sighand_struct *const sighand = current->sighand;

        spin_lock_irq(&sighand->siglock);
        if (signal_group_exit(sig))
            /* Another thread got here before we took the lock.  */
            exit_code = sig->group_exit_code;
        else {
            sig->group_exit_code = exit_code;
            sig->flags = SIGNAL_GROUP_EXIT;
            zap_other_threads(current);     /*  遍历整个线程组链表，并杀死其中的每个线程  */
        }
        spin_unlock_irq(&sighand->siglock);
    }

    do_exit(exit_code);
    /* NOTREACHED */
}