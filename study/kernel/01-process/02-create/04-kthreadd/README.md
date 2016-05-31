Linux下1号进程的前世(kernel_init)今生(init进程)
=======
http://zhidao.baidu.com/link?url=S5YkWyeZt4EtesIashfhTP27QbzVgm844EtNNWYGIDtbbWwRwTC7kdoh_bXrUPV4XhRlkxfp3WVjfExP_LZD_K

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------------- |:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|
| 2016-05-12 | [Linux-4.5](http://lxr.free-electrons.com/source/?v=4.5) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度-之-进程的创建](http://blog.csdn.net/gatieme/article/category/6225543) |

#前言
-------

<font color=0x009966>Linux下有3个特殊的进程，idle进程($PID = 0$), init进程($PID = 1$)和kthreadd($PID = 2$)

<font color=#A52A2A>
*	idle进程由系统自动创建, 运行在内核态
</font>
	idle进程其pid=0，其前身是系统创建的第一个进程，也是唯一一个没有通过fork或者kernel_thread产生的进程。完成加载系统后，演变为进程调度、交换

<font color=#A52A2A>
*	init进程由idle通过kernel_thread创建，在内核空间完成初始化后, 加载init程序, 并最终用户空间
</font>
	由0进程创建，完成系统的初始化. 是系统中所有其它用户进程的祖先进程
	Linux中的所有进程都是有init进程创建并运行的。首先Linux内核启动，然后在用户空间中启动init进程，再启动其他系统进程。在系统启动完成完成后，init将变为守护进程监视系统其他进程。


<font color=#A52A2A>
*	kthreadd进程由idle通过kernel_thread创建，并始终运行在内核空间, 负责所有内核线程的调度和管理
</font>
   它的任务就是管理和调度其他内核线程kernel_thread, 会循环执行一个kthreadd的函数，该函数的作用就是运行kthread_create_list全局链表中维护的kthread, 当我们调用kernel_thread创建的内核线程会被加入到此链表中，因此所有的内核线程都是直接或者间接的以kthreadd为父进程
</font>



我们下面就详解分析2号进程kthreadd




#2号进程
-------

内核初始化rest_init函数中，由进程 0 (swapper 进程)创建了两个process

*	init 进程 (pid = 1, ppid = 0)

*	kthreadd (pid = 2, ppid = 0)

所有其它的内核线程的ppid 都是 2，也就是说它们都是由kthreadd thread创建的

所有的内核线程在大部分时间里都处于阻塞状态(TASK_INTERRUPTIBLE)只有在系统满足进程需要的某种资源的情况下才会运行


它的任务就是管理和调度其他内核线程kernel_thread, 会循环执行一个kthreadd的函数，该函数的作用就是运行kthread_create_list全局链表中维护的kthread, 当我们调用kernel_thread创建的内核线程会被加入到此链表中，因此所有的内核线程都是直接或者间接的以kthreadd为父进程



#2号进程的创建
-------

在rest_init函数中创建2号进程的代码如下

```c
pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
rcu_read_lock();
kthreadd_task = find_task_by_pid_ns(pid, &init_pid_ns);
rcu_read_unlock();
complete(&kthreadd_done);
```



#2号进程的事件循环
-------


```c
int kthreadd(void *unused)
{
    struct task_struct *tsk = current;

    /* Setup a clean context for our children to inherit. */
    set_task_comm(tsk, "kthreadd");
    ignore_signals(tsk);
    set_cpus_allowed_ptr(tsk, cpu_all_mask);			//  允许kthreadd在任意CPU上运行
    set_mems_allowed(node_states[N_MEMORY]);

    current->flags |= PF_NOFREEZE;

    for (;;) {
     		/*  首先将线程状态设置为 TASK_INTERRUPTIBLE, 如果当前
            没有要创建的线程则主动放弃 CPU 完成调度.此进程变为阻塞态*/
            set_current_state(TASK_INTERRUPTIBLE);
            if (list_empty(&kthread_create_list))  //  没有需要创建的内核线程
                    schedule();									//   什么也不做, 执行一次调度, 让出CPU

             /*  运行到此表示 kthreadd 线程被唤醒(就是我们当前)
            设置进程运行状态为 TASK_RUNNING */
            __set_current_state(TASK_RUNNING);

            spin_lock(&kthread_create_lock);					//  加锁,
            while (!list_empty(&kthread_create_list)) {
                    struct kthread_create_info *create;

					/*  从链表中取得 kthread_create_info 结构的地址，在上文中已经完成插入操作(将
                    kthread_create_info 结构中的 list 成员加到链表中，此时根据成员 list 的偏移
                    获得 create)  */
                    create = list_entry(kthread_create_list.next,
                                        struct kthread_create_info, list);

                    /* 完成穿件后将其从链表中删除 */
                    list_del_init(&create->list);

                    /* 完成真正线程的创建 */
                    spin_unlock(&kthread_create_lock);	

                    create_kthread(create);

                    spin_lock(&kthread_create_lock);
            }
            spin_unlock(&kthread_create_lock);
    }

    return 0;
}
```

kthreadd的核心是一for和while循环体。

在for循环中，如果发现kthread_create_list是一空链表，则调用schedule调度函数，因为此前已经将该进程的状态设置为TASK_INTERRUPTIBLE，所以schedule的调用将会使当前进程进入睡眠。

如果kthread_create_list不为空，则进入while循环，在该循环体中会遍历该kthread_create_list列表，对于该列表上的每一个entry，都会得到对应的类型为struct kthread_create_info的节点的指针create.

然后函数在kthread_create_list中删除create对应的列表entry，接下来以create指针为参数调用create_kthread(create).

create_kthread的过程如下

#create_kthread完成内核线程创建
-------

```c
static void create_kthread(struct kthread_create_info *create)
{
    int pid;

#ifdef CONFIG_NUMA
    current->pref_node_fork = create->node;
#endif
    /* We want our own signal handler (we take no signals by default). 
    其实就是调用首先构造一个假的上下文执行环境，最后调用 do_fork()
    返回进程 id, 创建后的线程执行 kthread 函数
    */
    pid = kernel_thread(kthread, create, CLONE_FS | CLONE_FILES | SIGCHLD);
    if (pid < 0) {
            /* If user was SIGKILLed, I release the structure. */
            struct completion *done = xchg(&create->done, NULL);

            if (!done) {
                    kfree(create);
                    return;
            }
            create->result = ERR_PTR(pid);
            complete(done);
    }
}
```

在create_kthread()函数中，会调用kernel_thread来生成一个新的进程，该进程的内核函数为kthread，调用参数为
```
pid = kernel_thread(kthread, create, CLONE_FS | CLONE_FILES | SIGCHLD);
```
我们可以看到，创建的内核线程执行的事件[kthread](http://lxr.free-electrons.com/source/kernel/kthread.c#L177)

此时回到 kthreadd thread,它在完成了进程的创建后继续循环，检查 kthread_create_list 链表，如果为空，则 kthreadd 内核线程昏睡过去

那么我们现在回想我们的操作
我们在内核中通过kernel_create或者其他方式创建一个内核线程, 然后kthreadd内核线程被唤醒,　来执行内核线程创建的真正工作，于是这里有三个线程

1.	kthreadd已经光荣完成使命(接手执行真正的创建工作)，睡眠

2.	唤醒kthreadd的线程由于新创建的线程还没有创建完毕而继续睡眠 (在 kthread_create函数中)

3.	新创建的线程已经正在运行kthread，但是由于还有其它工作没有做所以还没有最终创建完成.


#新创建的内核线程kthread函数
-------

```c
static int kthread(void *_create)
{
    /* Copy data: it's on kthread's stack 
     create 指向 kthread_create_info 中的 kthread_create_info */
    struct kthread_create_info *create = _create;
    
     /*  新的线程创建完毕后执行的函数 */
    int (*threadfn)(void *data) = create->threadfn;
    /*  新的线程执行的参数  */
    void *data = create->data;
    struct completion *done;
    struct kthread self;
    int ret;

    self.flags = 0;
    self.data = data;
    init_completion(&self.exited);
    init_completion(&self.parked);
    current->vfork_done = &self.exited;

    /* If user was SIGKILLed, I release the structure. */
    done = xchg(&create->done, NULL);
    if (!done) {
            kfree(create);
            do_exit(-EINTR);
    }
    /* OK, tell user we're spawned, wait for stop or wakeup
     设置运行状态为 TASK_UNINTERRUPTIBLE  */
    __set_current_state(TASK_UNINTERRUPTIBLE);

     /*  current 表示当前新创建的 thread 的 task_struct 结构  */
    create->result = current;
    complete(done);
    /*  至此线程创建完毕 ,  执行任务切换，让出 CPU  */
    schedule();

    ret = -EINTR;

    if (!test_bit(KTHREAD_SHOULD_STOP, &self.flags)) {
            __kthread_parkme(&self);
            ret = threadfn(data);
    }
    /* we can't just return, we must preserve "self" on stack */
    do_exit(ret);
}
```

线程创建完毕:

创建新 thread 的进程恢复运行 kthread_create() 并且返回新创建线程的任务描述符
新创建的线程由于执行了 schedule() 调度，此时并没有执行.

直到我们使用wake_up_process(p);唤醒新创建的线程

线程被唤醒后, 会接着执行threadfn(data)
```c
    ret = -EINTR;

    if (!test_bit(KTHREAD_SHOULD_STOP, &self.flags)) {
            __kthread_parkme(&self);
            ret = threadfn(data);
    }
    /* we can't just return, we must preserve "self" on stack */
    do_exit(ret);
```

#总结
-------
kthreadd进程由idle通过kernel_thread创建，并始终运行在内核空间, 负责所有内核线程的调度和管理，它的任务就是管理和调度其他内核线程kernel_thread, 会循环执行一个kthreadd的函数，该函数的作用就是运行kthread_create_list全局链表中维护的kthread, 当我们调用kernel_thread创建的内核线程会被加入到此链表中，因此所有的内核线程都是直接或者间接的以kthreadd为父进程

我们在内核中通过kernel_create或者其他方式创建一个内核线程, 然后kthreadd内核线程被唤醒,　来执行内核线程创建的真正工作，新的线程将执行kthread函数, 完成创建工作，创建完毕后让出CPU，因此新的内核线程不会立刻运行．需要手工 wake up, 被唤醒后将执行自己的真正工作函数

*	任何一个内核线程入口都是 kthread()

*	通过 kthread_create() 创建的内核线程不会立刻运行．需要手工 wake up.

*	通过 kthread_create() 创建的内核线程有可能不会执行相应线程函数threadfn而直接退出

