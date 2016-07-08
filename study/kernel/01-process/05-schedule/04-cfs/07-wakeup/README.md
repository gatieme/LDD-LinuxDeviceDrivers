Linux CFS调度器之唤醒抢占
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-07-05 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/details/51456569) |


CFS负责处理普通非实时进程, 这类进程是我们linux中最普遍的进程


#1	前景回顾
-------


##1.1	CFS调度算法
-------

**CFS调度算法的思想**

理想状态下每个进程都能获得相同的时间片，并且同时运行在CPU上，但实际上一个CPU同一时刻运行的进程只能有一个。也就是说，当一个进程占用CPU时，其他进程就必须等待。CFS为了实现公平，必须惩罚当前正在运行的进程，以使那些正在等待的进程下次被调度.

##1.2	CFS的pick_next_fair选择下一个进程
-------

前面的一节中我们讲解了CFS的pick_next操作**pick_next_task_fair函数**, 他从当前运行队列上找出一个最优的进程来抢占处理器 ,一般来说这个最优进程总是**红黑树的最左进程left结点(其vruntime值最小)**, 当然如果挑选出的进程正好是队列是上需要**被调过调度的skip**, 则可能需要进一步读取**红黑树的次左结点second**, 而同样**curr进程**可能vruntime与cfs_rq的min_vruntime小, 因此它可能更渴望得到处理器, 而**last和next进程**由于刚被唤醒也应该尽可能的补偿.

**主调度器schedule**在选择最优的进程抢占处理器的时候, 通过__schedule调用**全局的pick_next_task**函数, 在**全局的pick_next_task**函数中, 按照stop > dl > rt > cfs > idle的顺序依次从**各个调度器类中pick_next函数**, 从而选择一个最优的进程.

##1.3	CFS如何处理周期性调度
-------

周期性调度器的工作由scheduler_tick函数完成(定义在[kernel/sched/core.c, line 2910](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L2910)), 在scheduler_tick中周期性调度器通过调用curr进程所属调度器类sched_class的task_tick函数完成周期性调度的工作

周期调度的工作形式上sched_class调度器类的task_tick函数完成, CFS则对应task_tick_fair函数, 但实际上工作交给entity_tick完成.


##1.4	唤醒抢占
-------


当在try_to_wake_up/wake_up_process和wake_up_new_task中唤醒进程时, 内核使用全局check_preempt_curr看看是否进程可以抢占当前进程可以抢占当前运行的进程. 请注意该过程不涉及核心调度器.


每个调度器类都因应该实现一个check_preempt_curr函数, 在全局check_preempt_curr中会调用进程其所属调度器类check_preempt_curr进行抢占检查, 对于完全公平调度器CFS处理的进程, 则对应由check_preempt_wakeup函数执行该策略.


新唤醒的进程不必一定由完全公平调度器处理, 如果新进程是一个实时进程, 则会立即请求调度, 因为实时进程优先极高, 实时进程总会抢占CFS进程.



#2 Linux进程的睡眠

在Linux中，仅等待CPU时间的进程称为就绪进程，它们被放置在一个运行队列中，一个就绪进程的状 态标志位为TASK_RUNNING. 一旦一个运行中的进程时间片用完, Linux 内核的调度器会剥夺这个进程对CPU的控制权, 并且从运行队列中选择一个合适的进程投入运行.

当然，一个进程也可以主动释放CPU的控制权. 函数schedule()是一个调度函数, 它可以被一个进程主动调用, 从而调度其它进程占用CPU. 一旦这个主动放弃CPU的进程被重新调度占用CPU, 那么它将从上次停止执行的位置开始执行, 也就是说它将从调用schedule()的下一行代码处开始执行.

有时候,进程需要等待直到某个特定的事件发生,例如设备初始化完成、I/O 操作完成或定时器到时等. 在这种情况下, 进程则必须从运行队列移出, 加入到一个等待队列中, 这个时候进程就进入了睡眠状态.

Linux 中的进程睡眠状态有两种

*	一种是可中断的睡眠状态，其状态标志位TASK_INTERRUPTIBLE.

	**可中断的睡眠状态**的进程会睡眠直到某个条件变为真, 比如说产生一个硬件中断、释放进程正在等待的系统资源或是传递一个信号都可以是唤醒进程的条件. 

*	另一种是不可中断的睡眠状态，其状态标志位为TASK_UNINTERRUPTIBLE.

	**不可中断睡眠状态**与可中断睡眠状态类似, 但是它有一个例外, 那就是把信号传递到这种睡眠 状态的进程不能改变它的状态, 也就是说它不响应信号的唤醒. 不可中断睡眠状态一般较少用到, 但在一些特定情况下这种状态还是很有用的, 比如说: 进程必须等待, 不能被中断, 直到某个特定的事件发生.

在现代的Linux操作系统中, 进程一般都是用调用schedule的方法进入睡眠状态的, 下面的代码演示了如何让正在运行的进程进入睡眠状态。

```c
sleeping_task = current;
set_current_state(TASK_INTERRUPTIBLE);
schedule();
func1();
/* Rest of the code ... */
```

在第一个语句中, 程序存储了一份进程结构指针sleeping_task, current 是一个宏，它指向正在执行的进程结构。set_current_state()将该进程的状态从执行状态TASK_RUNNING变成睡眠状态TASK_INTERRUPTIBLE.

*	如果schedule是被一个状态为TASK_RUNNING的进程调度，那么schedule将调度另外一个进程占用CPU;

*	如果schedule是被一个状态为TASK_INTERRUPTIBLE 或TASK_UNINTERRUPTIBLE 的进程调度，那么还有一个附加的步骤将被执行：当前执行的进程在另外一个进程被调度之前会被从运行队列中移出，这将导致正在运行的那个进程进入睡眠，因为它已经不在运行队列中了.


#3	linux进程的唤醒
-------

当在try_to_wake_up/wake_up_process和wake_up_new_task中唤醒进程时, 内核使用全局check_preempt_curr看看是否进程可以抢占当前进程可以抢占当前运行的进程. 请注意该过程不涉及核心调度器.


##  wake_up_process


我们可以使用wake_up_process将刚才那个进入睡眠的进程唤醒, 该函数定义在[kernel/sched/core.c, line 2043](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L2043).


```c
int wake_up_process(struct task_struct *p)
{
	return try_to_wake_up(p, TASK_NORMAL, 0);
}
```

在调用了wake_up_process以后, 这个睡眠进程的状态会被设置为TASK_RUNNING，而且调度器会把它加入到运行队列中去. 当然， 这个进程只有在下次被调度器调度到的时候才能真正地投入运行.


##   try_to_wake_up
-------

try_to_wake_up函数通过把进程状态设置为TASK_RUNNING, 并把该进程插入本地CPU运行队列rq来达到唤醒睡眠和停止的进程的目的.

例如: 调用该函数唤醒等待队列中的进程, 或恢复执行等待信号的进程.


```c
static int
try_to_wake_up(struct task_struct *p, unsigned int state, int wake_flags)
```



该函数接受的参数有: 被唤醒进程的描述符指针(p), 可以被唤醒的进程状态掩码(state), 一个标志wake_flags，用来禁止被唤醒的进程抢占本地CPU上正在运行的进程.

try_to_wake_up函数定义在[kernel/sched/core.c, line 1906](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L1906)


## wake_up_new_task
-------

```c
void wake_up_new_task(struct task_struct *p)
```

该函数定义在[kernel/sched/core.c, line 2421
](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L2421
)


之前进入睡眠状态的可以通过try_to_wake_up和wake_up_process完成唤醒, 而我们fork新创建的进程在完成自己的创建工作后, 可以通过wake_up_new_task完成唤醒工作, 参见[Linux下进程的创建过程分析(_do_fork/do_fork详解)--Linux进程的管理与调度（八）](http://blog.csdn.net/gatieme/article/details/51569932)

使用fork创建进程的时候, 内核会调用_do_fork(早期内核对应do_fork)函数完成内核的创建, 其中在进程的信息创建完毕后, 就可以使用wake_up_new_task将进程唤醒并添加到就绪队列中等待调度. 代码参见[kernel/fork.c, line 1755](http://lxr.free-electrons.com/source/kernel/fork.c?v=4.6#L1755)


##  check_preempt_curr
-------

当在try_to_wake_up和wake_up_new_task中唤醒进程时, 内核使用全局check_preempt_curr看看是否进程可以抢占当前进程可以抢占当前运行的进程.

函数定义在[kernel/sched/core.c, line 905](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L905)

```c
	check_preempt_curr(rq, p, WF_FORK);
#ifdef CONFIG_SMP
	if (p->sched_class->task_woken) {
	/*
     * Nothing relies on rq->lock after this, so its fine to
	 * drop it.
	 */
	lockdep_unpin_lock(&rq->lock);
	p->sched_class->task_woken(rq, p);
	lockdep_pin_lock(&rq->lock);
	}
#endif
```

#	无效唤醒
-------


几乎在所有的情况下, 进程都会在检查了某些条件之后, 发现条件不满足才进入睡眠. 可是有的时候进程却会在判定条件为真后开始睡眠, 如果这样的话进程就会无限期地休眠下去, 这就是所谓的无效唤醒问题。在操作系统中，当多个进程都企图对共享数据进行某种处理，而 最后的结果又取决于进程运行的顺序时，就会发生竞争条件，这是操作系统中一个典型的问题，无效唤醒恰恰就是由于竞争条件导致的。
设想有两个进程A 和B，A 进程正在处理一个链表，它需要检查这个链表是否为空，如果不空就对链
表里面的数据进行一些操作，同时B进程也在往这个链表添加节点。当这个链表是空的时候，由于无数据可操作，这时A进程就进入睡眠，当B进程向链表里面添加了节点之后它就唤醒A 进程，其代码如下：
A进程:
1 spin_lock(&list_lock);
2 if(list_empty(&list_head)) {
3 spin_unlock(&list_lock);
4 set_current_state(TASK_INTERRUPTIBLE);
5 schedule();
6 spin_lock(&list_lock);
7 }
8
9 /* Rest of the code ... */
10 spin_unlock(&list_lock);
B进程:
100 spin_lock(&list_lock);
101 list_add_tail(&list_head, new_node);
102 spin_unlock(&list_lock);
103 wake_up_process(processa_task);
这里会出现一个问题，假如当A进程执行到第3行后第4行前的时候，B进程被另外一个处理器调度
投 入运行。在这个时间片内，B进程执行完了它所有的指令，因此它试图唤醒A进程，而此时的A进程还没有进入睡眠，所以唤醒操作无效。在这之后，A 进程继续执行，它会错误地认为这个时候链表仍然是空的，于是将自己的状态设置为TASK_INTERRUPTIBLE然后调用schedule()进入睡 眠。由于错过了B进程唤醒，它将会无限期的睡眠下去，这就是无效唤醒问题，因为即使链表中有数据需要处理，A 进程也还是睡眠了。

3 避免无效唤醒
如何避免无效唤醒问题呢？我们发现无效唤醒主要发生在检查条件之后和进程状态被设置为睡眠状
态之前， 本来B进程的wake_up_process()提供了一次将A进程状态置为TASK_RUNNING 的机会，可惜这个时候A进程的状态仍然是TASK_RUNNING，所以wake_up_process()将A进程状态从睡眠状态转变为运行状态的努力 没有起到预期的作用。要解决这个问题，必须使用一种保障机制使得判断链表为空和设置进程状态为睡眠状态成为一个不可分割的步骤才行，也就是必须消除竞争条 件产生的根源，这样在这之后出现的wake_up_process ()就可以起到唤醒状态是睡眠状态的进程的作用了。
找到了原因后，重新设计一下A进程的代码结构，就可以避免上面例子中的无效唤醒问题了。
A进程:
1 set_current_state(TASK_INTERRUPTIBLE);
2 spin_lock(&list_lock);
3 if(list_empty(&list_head)) {
4 spin_unlock(&list_lock);
5 schedule();
6 spin_lock(&list_lock);
7 }
8 set_current_state(TASK_RUNNING);
9
10 /* Rest of the code ... */
11 spin_unlock(&list_lock);
可以看到，这段代码在测试条件之前就将当前执行进程状态转设置成TASK_INTERRUPTIBLE了，并且在链表不为空的情况下又将自己置为TASK_RUNNING状态。这样一来如果B进程在A进程进程检查
了链表为空以后调用wake_up_process()，那么A进程的状态就会自动由原来TASK_INTERRUPTIBLE
变成TASK_RUNNING，此后即使进程又调用了schedule()，由于它现在的状态是TASK_RUNNING，所以仍然不会被从运行队列中移出，因而不会错误的进入睡眠，当然也就避免了无效唤醒问题。

4 Linux内核的例子
在Linux操作系统中，内核的稳定性至关重要，为了避免在Linux操作系统内核中出现无效唤醒问题，
Linux内核在需要进程睡眠的时候应该使用类似如下的操作：
/* ‘q’是我们希望睡眠的等待队列 */
DECLARE_WAITQUEUE(wait,current);
add_wait_queue(q, &wait);
set_current_state(TASK_INTERRUPTIBLE);
/* 或TASK_INTERRUPTIBLE */
while(!condition) /* ‘condition’ 是等待的条件*/
schedule();
set_current_state(TASK_RUNNING);
remove_wait_queue(q, &wait);
上面的操作，使得进程通过下面的一系列步骤安全地将自己加入到一个等待队列中进行睡眠：首先调
用DECLARE_WAITQUEUE ()创建一个等待队列的项，然后调用add_wait_queue()把自己加入到等待队列中，并且将进程的状态设置为 TASK_INTERRUPTIBLE 或者TASK_INTERRUPTIBLE。然后循环检查条件是否为真：如果是的话就没有必要睡眠，如果条件不为真，就调用schedule()。当进程 检查的条件满足后，进程又将自己设置为TASK_RUNNING 并调用remove_wait_queue()将自己移出等待队列。
从上面可以看到，Linux的内核代码维护者也是在进程检查条件之前就设置进程的状态为睡眠状态，
然后才循环检查条件。如果在进程开始睡眠之前条件就已经达成了，那么循环会退出并用set_current_state()将自己的状态设置为就绪，这样同样保证了进程不会存在错误的进入睡眠的倾向，当然也就不会导致出现无效唤醒问题。
下面让我们用linux 内核中的实例来看看Linux 内核是如何避免无效睡眠的，这段代码出自Linux2.6的内核(linux-2.6.11/kernel/sched.c: 4254):
4253 /* Wait for kthread_stop */
4254 set_current_state(TASK_INTERRUPTIBLE);
4255 while (!kthread_should_stop()) {
4256 schedule();
4257 set_current_state(TASK_INTERRUPTIBLE);
4258 }
4259 __set_current_state(TASK_RUNNING);
4260 return 0;
上面的这些代码属于迁移服务线程migration_thread，这个线程不断地检查kthread_should_stop()，
直 到kthread_should_stop()返回1它才可以退出循环，也就是说只要kthread_should_stop()返回0该进程就会一直睡 眠。从代码中我们可以看出，检查kthread_should_stop()确实是在进程的状态被置为TASK_INTERRUPTIBLE后才开始执行 的。因此，如果在条件检查之后但是在schedule()之前有其他进程试图唤醒它，那么该进程的唤醒操作不会失效。

#唤醒抢占
-------

小结
通过上面的讨论，可以发现在Linux 中避免进程的无效唤醒的关键是在进程检查条件之前就将进程的
状态置为TASK_INTERRUPTIBLE或TASK_UNINTERRUPTIBLE，并且如果检查的条件满足的话就应该
将其状态重新设置为TASK_RUNNING。这样无论进程等待的条件是否满足， 进程都不会因为被移出就绪队列而错误地进入睡眠状态，从而避免了无效唤醒问题。






