Linux进程退出详解
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------------- |:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|
| 2016-05-12 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |


#Linux进程的退出
-------



##linux下进程退出的方式
-------

**正常退出**

*	从main函数返回return

*	调用exit

*	调用_exit

**异常退出**

*	调用abort

*	由信号终止

##_exit, exit和_Exit的区别和联系
-------

_exit是linux系统调用，关闭所有文件描述符，然后退出进程。

exit是c语言的库函数，他最终调用_exit。在此之前，先清洗标准输出的缓存，调用用atexit注册的函数等, 在c语言的main函数中调用return就等价于调用exit。

_Exit是c语言的库函数，自c99后加入，等价于_exit，即可以认为它直接调用_Exit。


基本来说，_Exit（或 _exit，建议使用大写版本）是为 fork 之后的子进程准备的特殊 API。功能见[POSIX 标准：_Exit](https://link.zhihu.com/?target=http%3A//pubs.opengroup.org/onlinepubs/9699919799/functions/_Exit.html%23)，讨论见 [c - how to exit a child process](https://link.zhihu.com/?target=http%3A//stackoverflow.com/questions/2329640/how-to-exit-a-child-process-exit-vs-exit)

>由fork()函数创建的子进程分支里，正常情况下使用函数exit()是不正确的，这是因为使用它会导致标准输入输出的缓冲区被清空两次，而且临时文件可能被意外删除。”
>
>因为在 fork 之后，exec 之前，很多资源还是共享的（如某些文件描述符），如果使用 exit 会关闭这些资源，导致某些非预期的副作用（如删除临时文件等）。

「刷新」是对应 flush，意思是把内容从内存缓存写出到文件里，而不仅仅是清空（所以常见的对 stdin 调用 flush 的方法是耍流氓而已）。如果在 fork 的时候父进程内存有缓冲内容，则这个缓冲会带到子进程，并且两个进程会分别 flush （写出）一次，造成数据重复。参见[c - How does fork() work with buffered streams like stdout?](https://link.zhihu.com/?target=http%3A//stackoverflow.com/questions/18671525/how-does-fork-work-with-buffered-streams-like-stdout)


#进程退出的系统调用
-------

**_exit系统调用**

进程退出由exit系统调用来完成, 这使得内核有机会将该进程所使用的资源释放回系统中

进程终止时，一般是调用exit库函数（无论是程序员显式调用还是编译器自动地把exit库函数插入到main函数的最后一条语句之后）来释放进程所拥有的资源。

exit系统调用的入口点是sys_exit()函数, 需要一个错误码作为参数, 以便退出进程。

其定义是体系结构无关的, 见kernel/exit.c

而我们用户空间的多线程应用程序, 对应内核中就有多个进程, 这些进程共享虚拟地址空间和资源, 他们有各自的进程id(pid), 但是他们的组进程id(tpid)是相同的, 都等于组长(领头进程)的pid

>在linux内核中对线程并没有做特殊的处理，还是由task_struct来管理。所以从内核的角度看， 用户态的线程本质上还是一个进程。对于同一个进程（用户态角度）中不同的线程其tgid是相同的，但是pid各不相同。 主线程即group_leader（主线程会创建其他所有的子线程）。如果是单线程进程（用户态角度），它的pid等于tgid。
>
>这个信息我们已经讨论过很多次了
>
>参见
>
>[Linux进程ID号--Linux进程的管理与调度（三）](http://blog.csdn.net/gatieme/article/details/51383377#t10)
>
>[Linux进程描述符task_struct结构体详解--Linux进程的管理与调度（一）](http://blog.csdn.net/gatieme/article/details/51383272#t5)

**为什么还需要exit_group**

我们如果了解linux的线程实现机制的话, 会知道所有的线程是属于一个线程组的, 同时即使不是线程, linux也允许多个进程组成进程组, 多个进程组组成一个会话, 因此我们本质上了解到不管是多线程, 还是进程组起本质都是多个进程组成的一个集合, 那么我们的应用程序在退出的时候, 自然希望一次性的退出组内所有的进程。

因此exit_group就诞生了

group_exit函数会杀死属于当前进程所在线程组的所有进程。它接受进程终止代号作为参数，进程终止代号可能是系统调用exit_group（正常结束）指定的一个值，也可能是内核提供的一个错误码（异常结束）。 



因此C语言的库函数exit使用系统调用exit_group来终止整个线程组，库函数pthread_exit使用系统调用_exit来终止某一个线程

_exit和exit_group这两个系统调用在Linux内核中的入口点函数分别为sys_exit和sys_exit_group。



##_exit和exit_group系统调用

###系统调用声明
-------

声明见[include/linux/syscalls.h, line 326](http://lxr.free-electrons.com/source/include/linux/syscalls.h#L326)
```c
asmlinkage long sys_exit(int error_code);
asmlinkage long sys_exit_group(int error_code);

asmlinkage long sys_wait4(pid_t pid, int __user *stat_addr,
                                int options, struct rusage __user *ru);
asmlinkage long sys_waitid(int which, pid_t pid,
                           struct siginfo __user *infop,
                           int options, struct rusage __user *ru);
asmlinkage long sys_waitpid(pid_t pid, int __user *stat_addr, int options);
```


###系统调用号
-------

其系统调用号是一个体系结构相关的定义, 但是多数体系结构的定义如下, 在[include/uapi/asm-generic/unistd.h, line 294](http://lxr.free-electrons.com/source/include/uapi/asm-generic/unistd.h?v=4.6#L294)文件中
```c
/* kernel/exit.c */
#define __NR_exit 93
__SYSCALL(__NR_exit, sys_exit)
#define __NR_exit_group 94
__SYSCALL(__NR_exit_group, sys_exit_group)
#define __NR_waitid 95
__SC_COMP(__NR_waitid, sys_waitid, compat_sys_waitid)
```
只有少数体系结构, 重新定义了系统调用号


| 体系 | 定义 |
| ------------- |:-------------:|
| xtensa | [rch/xtensa/include/uapi/asm/unistd.h, line 267](http://lxr.free-electrons.com/source/arch/xtensa/include/uapi/asm/unistd.h?v=4.6#L267) |
| arm64 | [rch/arm64/include/asm/unistd32.h, line 27](http://lxr.free-electrons.com/source/arch/arm64/include/asm/unistd32.h?v=4.6#L27) |
| 通用 | [include/uapi/asm-generic/unistd.h, line 294](http://lxr.free-electrons.com/source/include/uapi/asm-generic/unistd.h?v=4.6#L294) |


###系统调用实现
-------

然后系统调用的实现在[kernel/exit.c](http://lxr.free-electrons.com/source/kernel/exit.c?v=4.6) 中

```c
SYSCALL_DEFINE1(exit, int, error_code)
{
        do_exit((error_code&0xff)<<8);
}


/*
 * this kills every thread in the thread group. Note that any externally
 * wait4()-ing process will get the correct exit code - even if this
 * thread is not the thread group leader.
 */
SYSCALL_DEFINE1(exit_group, int, error_code)
{
        do_group_exit((error_code & 0xff) << 8);
        /* NOTREACHED */
        return 0;
}
```
##do_exit_group流程
-------



do_group_exit()函数杀死属于current线程组的所有进程。它接受进程终止代码作为参数，进程终止代号可能是系统调用exit_group()指定的一个值，也可能是内核提供的一个错误代号。
该函数执行下述操作

1.	检查退出进程的SIGNAL＿GROUP_EXIT标志是否不为0，如果不为0，说明内核已经开始为线性组执行退出的过程。在这种情况下，就把存放在current->signal->group_exit_code的值当作退出码，然后跳转到第4步。

2.	否则，设置进程的SIGNAL＿GROUP_EXIT标志并把终止代号放到current->signal->group_exit_code字段。

3.	调用zap_other_threads()函数杀死current线程组中的其它进程。为了完成这个步骤，函数扫描与current->tgid对应的PIDTYPE＿TGID类型的散列表中的每PID链表，向表中所有不同于current的进程发送SIGKILL信号，结果，所有这样的进程都将执行do_exit()函数，从而被杀死。

4.	调用do_exit()函数，把进程的终止代码传递给它。正如我们将在下面看到的，do_exit()杀死进程而且不再返回。


```c
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
```

##do_exit流程
-------

进程终止所要完成的任务都是由do_exit函数来处理。

该函数定义在[kernel/exit.c](http://lxr.free-electrons.com/source/kernel/exit.c#L652)中

###触发task_exit_nb通知链实例的处理函数

```c
profile_task_exit(tsk);

```
该函数会定义在触发[kernel/profile.c](http://lxr.free-electrons.com/source/kernel/profile.c?v=4.6#L138)

```c
void profile_task_exit(struct task_struct *task)
{
    blocking_notifier_call_chain(&task_exit_notifier, 0, task);
}
```

会触发task_exit_notifier通知, 从而触发对应的处理函数


其中task_exit_notifier被定义如下
```c
//  http://lxr.free-electrons.com/source/kernel/profile.c?v=4.6#L134
static BLOCKING_NOTIFIER_HEAD(task_exit_notifier);


// http://lxr.free-electrons.com/source/include/linux/notifier.h?v=4.6#L111
#define BLOCKING_NOTIFIER_INIT(name) {                      \
                .rwsem = __RWSEM_INITIALIZER((name).rwsem),     \
                .head = NULL }

// http://lxr.free-electrons.com/source/include/linux/rwsem.h?v4.6#L74
#define __RWSEM_INITIALIZER(name)                               \
        { .count = RWSEM_UNLOCKED_VALUE,                        \
          .wait_list = LIST_HEAD_INIT((name).wait_list),        \
          .wait_lock = __RAW_SPIN_LOCK_UNLOCKED(name.wait_lock) \
          __RWSEM_OPT_INIT(name)                                \
          __RWSEM_DEP_MAP_INIT(name) }


```

###检查进程的blk_plug是否为空

保证task_struct中的plug字段是空的，或者plug字段指向的队列是空的。plug字段的意义是stack plugging

```c
//  http://lxr.free-electrons.com/source/include/linux/blkdev.h?v=4.6#L1095
WARN_ON(blk_needs_flush_plug(tsk));
```

其中blk_needs_flush_plug函数定义在[include/linux/blkdev.h](http://lxr.free-electrons.com/source/include/linux/blkdev.h?v=4.6#L1095), 如下
```c
static inline bool blk_needs_flush_plug(struct task_struct *tsk)
{
    struct blk_plug *plug = tsk->plug;

    return plug &&
		(!list_empty(&plug->list) ||
        !list_empty(&plug->mq_list) ||
        !list_empty(&plug->cb_list));
}
```

###OOPS消息
-------

中断上下文不能执行do_exit函数, 也不能终止PID为0的进程。

```c
if (unlikely(in_interrupt()))
	panic("Aiee, killing interrupt handler!");
if (unlikely(!tsk->pid))
	panic("Attempted to kill the idle task!");
```

###设定进程可以使用的虚拟地址的上限（用户空间）
-------

```c
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

这个是一个体系结构相关的代码, 其定义如下

```

其定义在arch/对应体系/include/asm/uaccess.h中

| 体系 | 定义 |
| ------------- |:-------------:|
| arm | [arch/arm/include/asm/uaccess.h, line 99](http://lxr.free-electrons.com/source/arch/arm/include/asm/uaccess.h?v4.6#L99) |
| arm64 | [arch/arm64/include/asm/uaccess.h, line 66](http://lxr.free-electrons.com/source/arch/arm64/include/asm/uaccess.h?v4.6#L66) |
| x86 | [arch/x86/include/asm/uaccess.h, line 32](http://lxr.free-electrons.com/source/arch/x86/include/asm/uaccess.h?v4.6#L32) |
| 通用 | [include/asm-generic/uaccess.h, line 28](http://lxr.free-electrons.com/source/include/asm-generic/uaccess.h?v=4.6#L28)

arm64的定义如下
```c
static inline void set_fs(mm_segment_t fs)
{
	current_thread_info()->addr_limit = fs;

	/*
	 * Enable/disable UAO so that copy_to_user() etc can access
	 * kernel memory with the unprivileged instructions.
	*/
	if (IS_ENABLED(CONFIG_ARM64_UAO) && fs == KERNEL_DS)
		asm(ALTERNATIVE("nop", SET_PSTATE_UAO(1), ARM64_HAS_UAO));
	else
		asm(ALTERNATIVE("nop", SET_PSTATE_UAO(0), ARM64_HAS_UAO,
		CONFIG_ARM64_UAO));
}
```

###检查进病设置进程程PF_EXITING
-------


首先是检查PF_EXITING标识, 此标识表示进程正在退出,　

如果此标识已被设置, 则进一步设置PF_EXITPIDONE标识,　并将进程的状态设置为不可中断状态TASK_UNINTERRUPTIBLE,　并进程一次进程调度


```c
    /*current->flags的PF_EXITING标志表示进程正在被删除  */
    if (unlikely(tsk->flags & PF_EXITING)) {  /*  检查PF_EXITING标志是否未被设置  */
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
        /*  设置进程状态为不可中断的等待状态 */
        set_current_state(TASK_UNINTERRUPTIBLE);
        /*  调度其它进程  */
        schedule();
    }
```

如果此标识未被设置, 则通过exit_signals来设置

```c
    /*
        tsk->flags |= PF_EXITING;
        http://lxr.free-electrons.com/source/kernel/signal.c#L2383
    */
    exit_signals(tsk);  /* sets tsk->flags PF_EXITING  设置PF_EXITING标志
```


###内存屏障
-------

```c
    /*
     * tsk->flags are checked in the futex code to protect against
     * an exiting task cleaning up the robust pi futexes.
     */
    /*  内存屏障，用于确保在它之后的操作开始执行之前，它之前的操作已经完成  */
    smp_mb();
    /*  一直等待，直到获得current->pi_lock自旋锁  */
    raw_spin_unlock_wait(&tsk->pi_lock);
```

###同步进程的mm的rss_stat
```c
    /* sync mm's RSS info before statistics gathering */
    if (tsk->mm)
        sync_mm_rss(tsk->mm);
```

###获取current->mm->rss_stat.count[member]计数 
-------

```c

    /*
        cct_update_integrals - update mm integral fields in task_struct
        更新进程的运行时间, 获取current->mm->rss_stat.count[member]计数 
        http://lxr.free-electrons.com/source/kernel/tsacct.c?v=4.6#L152
    */
    acct_update_integrals(tsk);
```

函数的实现如下, 参见 http://lxr.free-electrons.com/source/kernel/tsacct.c?v=4.6#L156

```c
void acct_update_integrals(struct task_struct *tsk)
{
	cputime_t utime, stime;
	unsigned long flags;

	local_irq_save(flags);
	task_cputime(tsk, &utime, &stime);
	__acct_update_integrals(tsk, utime, stime);
	local_irq_restore(flags);
}
```

其中task_cputime获取了进程的cpu时间
__acct_update_integr定义如下

参照http://lxr.free-electrons.com/source/kernel/tsacct.c#L125

```c
static void __acct_update_integrals(struct task_struct *tsk,
                    cputime_t utime, cputime_t stime)
{
    cputime_t time, dtime;
    u64 delta;

    if (!likely(tsk->mm))
        return;

    time = stime + utime;
    dtime = time - tsk->acct_timexpd;
    /* Avoid division: cputime_t is often in nanoseconds already. */
    delta = cputime_to_nsecs(dtime);

    if (delta < TICK_NSEC)
        return;

    tsk->acct_timexpd = time;
    /*
     * Divide by 1024 to avoid overflow, and to avoid division.
     * The final unit reported to userspace is Mbyte-usecs,
     * the rest of the math is done in xacct_add_tsk.
     */
    tsk->acct_rss_mem1 += delta * get_mm_rss(tsk->mm) >> 10;
    tsk->acct_vm_mem1 += delta * tsk->mm->total_vm >> 10;
}
```
###清除定时器
-------

```c
    group_dead = atomic_dec_and_test(&tsk->signal->live);
    if (group_dead) {
        hrtimer_cancel(&tsk->signal->real_timer);
        exit_itimers(tsk->signal);
        if (tsk->mm)
            setmax_mm_hiwater_rss(&tsk->signal->maxrss, tsk->mm);
    }
```


###收集进程会计信息
-------

```c
    acct_collect(code, group_dead);
```

###审计
-------

```c
    if (group_dead)
        tty_audit_exit();	//记录审计事件
    audit_free(tsk);	//  释放struct audit_context结构体
```


###释放进程占用的资源
-------

**释放线性区描述符和页表**

```c
    /*  释放存储空间
    放弃进程占用的mm,如果没有其他进程使用该mm，则释放它。
     */
    exit_mm(tsk);
```


**输出进程会计信息**

```c
    if (group_dead)
        acct_process();
    trace_sched_process_exit(tsk);
```

**释放用户空间的“信号量”**

```c
exit_sem(tsk);   /*  释放用户空间的“信号量”  */
```
遍历current->sysvsem.undo_list链表，并清除进程所涉及的每个IPC信号量的操作痕迹

**释放锁**

```c
exit_shm(tsk);  /* 释放锁  */
```

**释放文件对象相关资源**

```c
exit_files(tsk); /*  释放已经打开的文件   */
````

**释放struct fs_struct结构体**

```c
exit_fs(tsk);   /*  释放用于表示工作目录等结构  */
```

**脱离控制终端**
```c
    if (group_dead)
        disassociate_ctty(1);
```
**释放命名空间**

```c
exit_task_namespaces(tsk);  /*  释放命名空间  */
exit_task_work(tsk);
```

**释放task_struct中的thread_struct结构**

```c
    exit_thread();      /*     */
```

触发thread_notify_head链表中所有通知链实例的处理函数，用于处理struct thread_info结构体



**Performance Event功能相关资源的释放**

```c
perf_event_exit_task(tsk);
```

**Performance Event功能相关资源的释放**

```c
cgroup_exit(tsk);
```

**注销断点**

```c
    /*
     * FIXME: do that only when needed, using sched_exit tracepoint
     */
    flush_ptrace_hw_breakpoint(tsk);
```

**更新所有子进程的父进程**

```c
    exit_notify(tsk, group_dead);
```

**进程事件连接器（通过它来报告进程fork、exec、exit以及进程用户ID与组ID的变化）**

```c
    proc_exit_connector(tsk);
```


**用于NUMA，当引用计数为0时，释放struct mempolicy结构体所占用的内存**

```c
#ifdef CONFIG_NUMA
    task_lock(tsk);
    mpol_put(tsk->mempolicy);
    tsk->mempolicy = NULL;
    task_unlock(tsk);
#endif
```

**释放struct futex_pi_state结构体所占用的内存**
```c
#ifdef CONFIG_FUTEX
    if (unlikely(current->pi_state_cache))
        kfree(current->pi_state_cache);
#endif
```

**释放struct io_context结构体所占用的内存**

```c
    if (tsk->io_context)
        exit_io_context(tsk);
```


**释放与进程描述符splice_pipe字段相关的资源**


```c
    if (tsk->splice_pipe)
        free_pipe_info(tsk->splice_pipe);
```

```c
    if (tsk->task_frag.page)
        put_page(tsk->task_frag.page);
```

##检查有多少未使用的进程内核栈
-------

```c
    check_stack_usage();
```

##调度其它进程
-------

```c
    /* causes final put_task_struct in finish_task_switch(). */
    tsk->state = TASK_DEAD;
    tsk->flags |= PF_NOFREEZE;      /* tell freezer to ignore us */
    /*
        重新调度，因为该进程已经被设置成了僵死状态，因此永远都不会再把它调度回来运行了，也就实现了do_exit不会有返回的目标    */
    schedule();
```

在设置了进程状态为TASK_DEAD后, 进程进入**僵死状态**, 进程已经无法被再次调度, 因为对应用程序或者用户空间来说此进程已经死了, 但是尽管进程已经不能再被调度，但系统还是保留了它的进程描述符，这样做是为了让系统有办法在进程终止后仍能获得它的信息。

在父进程获得已终止子进程的信息后，子进程的task_struct结构体才被释放（包括此进程的内核栈）。


