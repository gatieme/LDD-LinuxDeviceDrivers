Linux下0号进程的前世(init_task进程)今生(idle进程)
=======


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
   它的任务就是管理和调度其他内核线程kernel_thread, 会循环执行一个kthread的函数，该函数的作用就是运行kthread_create_list全局链表中维护的kthread, 当我们调用kernel_thread创建的内核线程会被加入到此链表中，因此所有的内核线程都是直接或者间接的以kthreadd为父进程
</font>


我们下面就详解分析0号进程的前世(init_task)今生(idle)

#idle的创建
-------

在smp系统中，每个处理器单元有独立的一个运行队列，而每个运行队列上又有一个idle进程，即有多少处理器单元，就有多少idle进程。

>idle进程其pid=0，其前身是系统创建的第一个进程，也是唯一一个没有通过fork()产生的进程。在smp系统中，每个处理器单元有独立的一个运行队列，而每个运行队列上又有一个idle进程，即有多少处理器单元，就有多少idle进程。系统的空闲时间，其实就是指idle进程的"运行时间"。既然是idle是进程，那我们来看看idle是如何被创建，又具体做了哪些事情？


我们知道系统是从BIOS加电自检，载入MBR中的引导程序(LILO/GRUB),再加载linux内核开始运行的，一直到指定shell开始运行告一段落，这时用户开始操作Linux。



##0号进程上下文信息--init_task描述符
-------


**init_task**是内核中所有进程、线程的task_struct雏形，在内核初始化过程中，通过静态定义构造出了一个task_struct接口，取名为init_task，然后在内核初始化的后期，通过rest_init（）函数新建了**内核init线程，kthreadd内核线程**

*	**内核init线程**，最终执行/sbin/init进程，变为所有用户态程序的根进程（pstree命令显示）,即用户空间的init进程

    开始的init是有kthread_thread创建的内核线程, 他在完成初始化工作后, 转向用户空间, 并且生成所有用户进程的祖先

*	**内核kthreadd内核线程**，变为所有内核态其他守护线程的父线程。

    它的任务就是管理和调度其他内核线程kernel_thread, 会循环执行一个kthread的函数，该函数的作用就是运行kthread_create_list全局链表中维护的kthread, 当我们调用kernel_thread创建的内核线程会被加入到此链表中，因此所有的内核线程都是直接或者间接的以kthreadd为父进程


![pa-aux](./images/ps-aux.jpg)


所以<font color=0x009966>**init_task决定了系统所有进程、线程的基因, 它完成初始化后, 最终演变为0号进程idle, 并且运行在内核态**</font>

内核在初始化过程中，当创建完init和kthreadd内核线程后，内核会发生调度执行，此时内核将使用该init_task作为其task_struct结构体描述符，当系统无事可做时，会调度其执行， 此时该内核会变为idle进程，让出CPU，自己进入睡眠，不停的循环，查看init_task结构体，其comm字段为swapper，作为idle进程的描述符。

>idle的运行时机
>
>idle 进程优先级为MAX_PRIO-20。早先版本中，idle是参与调度的，所以将其优先级设低点，当没有其他进程可以运行时，才会调度执行 idle。而目前的版本中idle并不在运行队列中参与调度，而是在运行队列结构中含idle指针，指向idle进程，在调度器发现运行队列为空的时候运行，调入运行

简言之, **内核中init_task变量就是是进程0使用的进程描述符**，也是Linux系统中第一个进程描述符，init_task并不是系统通过kernel_thread的方式（当然更不可能是fork）创建的, 而是由内核黑客静态创建的.


该进程的描述符在[init/init_task](http://lxr.free-electrons.com/source/init/init_task.c?v=4.5#L17
)中定义，代码片段如下


```c
/* Initial task structure */
struct task_struct init_task = INIT_TASK(init_task);
EXPORT_SYMBOL(init_task);
```

init_task描述符使用宏INIT_TASK对init_task的进程描述符进行初始化，宏INIT_TASK在[include/linux/init_task.h](http://lxr.free-electrons.com/source/include/linux/init_task.h?v=4.5#L186)文件中

init_task是Linux内核中的第一个线程，它贯穿于整个Linux系统的初始化过程中，该进程也是Linux系统中唯一一个没有用kernel_thread()函数创建的内核态进程(内核线程)

在init_task进程执行后期，它会调用kernel_thread()函数创建第一个核心进程kernel_init，同时init_task进程继续对Linux系统初始化。在完成初始化后，init_task会退化为cpu_idle进程，当Core 0的就绪队列中没有其它进程时，该进程将会获得CPU运行。新创建的1号进程kernel_init将会逐个启动次CPU,并最终创建用户进程！

>备注：core0上的idle进程由init_task进程退化而来，而AP的idle进程则是BSP在后面调用fork()函数逐个创建的


###进程堆栈init_thread_union
-------

init_task进程使用init_thread_union数据结构描述的内存区域作为该进程的堆栈空间，并且和自身的thread_info参数公用这一内存空间空间，

>请参见 http://lxr.free-electrons.com/source/include/linux/init_task.h?v=4.5#L193
>
>         .stack          = &init_thread_info,


而init_thread_info则是一段体系结构相关的定义，被定义在[/arch/对应体系/include/asm/thread_info.h]中，但是他们大多数为如下定义

```c
#define init_thread_info        (init_thread_union.thread_info)
#define init_stack              (init_thread_union.stack)
```

其中init_thread_union被定义在[init/init_task.c](http://lxr.free-electrons.com/source/init/init_task.c?v=4.5#L21), 紧跟着前面[init_task](http://lxr.free-electrons.com/source/init/init_task.c?v=4.5#L17)的定义

```c
/*
 * Initial thread structure. Alignment of this is handled by a special
 * linker map entry.
 */
union thread_union init_thread_union __init_task_data =
        { INIT_THREAD_INFO(init_task) };
```

我们可以发现init_task是用INIT_THREAD_INFO宏进行初始化的, 这个才是我们真正体系结构相关的部分, 他与init_thread_info定义在一起，被定义在[/arch/对应体系/include/asm/thread_info.h](http://lxr.free-electrons.com/ident?v=4.5;i=INIT_THREAD_INFO)中，以下为[x86架构的定义](http://lxr.free-electrons.com/source/arch/x86/include/asm/thread_info.h?v=4.5#L65)

> 参见 
> 
> http://lxr.free-electrons.com/source/arch/x86/include/asm/thread_info.h?v=4.5#L65

```c
#define INIT_THREAD_INFO(tsk)                   \
{                                               \
    .task           = &tsk,                 \
    .flags          = 0,                    \
    .cpu            = 0,                    \
    .addr_limit     = KERNEL_DS,            \
}
```

>其他体系结构的定义请参见
>
>[/arch/对应体系/include/asm/thread_info.h](http://lxr.free-electrons.com/ident?v=4.5;i=INIT_THREAD_INFO)中

| 架构 | 定义 |
| ------------- |:-------------:|
| x86 | [arch/x86/include/asm/thread_info.h](http://lxr.free-electrons.com/source/arch/x86/include/asm/thread_info.h?v=4.5#L65) |
| arm64 | [arch/arm64/include/asm/thread_info.h](http://lxr.free-electrons.com/source/arch/arm64/include/asm/thread_info.h?v=4.5#L55) |

init_thread_info定义中的__init_task_data表明该内核栈所在的区域位于内核映像的init data区，我们可以通过编译完内核后所产生的System.map来看到该变量及其对应的逻辑地址
```
cat System.map-3.1.6 | grep init_thread_union
```

![init_thread_union](./images/init_thread_union.png)

###进程内存空间
-------
init_task的虚拟地址空间，也采用同样的方法被定义

由于init_task是一个运行在内核空间的内核线程, 因此其虚地址段mm为NULL, 但是必要时他还是需要使用虚拟地址的，因此avtive_mm被设置为init_mm

> 参见
>
> http://lxr.free-electrons.com/source/include/linux/init_task.h?v=4.5#L202

```c
.mm             = NULL,                                         \
  .active_mm      = &init_mm,                                     \
```
其中init_mm被定义为init-mm.c中，参见 http://lxr.free-electrons.com/source/mm/init-mm.c?v=4.5#L16

```c
struct mm_struct init_mm = {
    .mm_rb          = RB_ROOT,
    .pgd            = swapper_pg_dir,
    .mm_users       = ATOMIC_INIT(2),
    .mm_count       = ATOMIC_INIT(1),
    .mmap_sem       = __RWSEM_INITIALIZER(init_mm.mmap_sem),
    .page_table_lock =  __SPIN_LOCK_UNLOCKED(init_mm.page_table_lock),
    .mmlist         = LIST_HEAD_INIT(init_mm.mmlist),
    INIT_MM_CONTEXT(init_mm)
};
```

#0号进程的演化
-------

##rest_init创建init进程(PID =1)和kthread进程(PID=2)
-------

Linux在无进程概念的情况下将一直从初始化部分的代码执行到start_kernel，然后再到其最后一个函数调用rest_init

>大致是在vmlinux的入口startup_32(head.S)中为pid号为0的原始进程设置了执行环境，然后原是进程开始执行start_kernel()完成Linux内核的初始化工作。包括初始化页表，初始化中断向量表，初始化系统时间等。

从rest_init开始，Linux开始产生进程，因为init_task是静态制造出来的，pid=0，它试图将从最早的汇编代码一直到start_kernel的执行都纳入到init_task进程上下文中。

这个函数其实是由0号进程执行的, 他就是在这个函数中, 创建了init进程和kthreadd进程

这部分代码如下：

>参见	
>
>http://lxr.free-electrons.com/source/init/main.c?v=4.5#L386

```c
static noinline void __init_refok rest_init(void)
{
	int pid;

	rcu_scheduler_starting();
	smpboot_thread_init();

    /*
 	* We need to spawn init first so that it obtains pid 1, however
 	* the init task will end up wanting to create kthreads, which, if
 	* we schedule it before we create kthreadd, will OOPS.
 	*/
	kernel_thread(kernel_init, NULL, CLONE_FS);
	numa_default_policy();
	pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
	rcu_read_lock();
	kthreadd_task = find_task_by_pid_ns(pid, &init_pid_ns);
	rcu_read_unlock();
	complete(&kthreadd_done);

	/*
 	* The boot idle thread must execute schedule()
 	* at least once to get things moving:
 	*/
	init_idle_bootup_task(current);
	schedule_preempt_disabled();
	/* Call into cpu_idle with preempt disabled */
	cpu_startup_entry(CPUHP_ONLINE);
}
```

1.	调用kernel_thread()创建1号内核线程, 该线程随后转向用户空间, 演变为init进程

2.	调用kernel_thread()创建kthreadd内核线程。

3.	init_idle_bootup_task()：当前0号进程init_task最终会退化成idle进程，所以这里调用init_idle_bootup_task()函数，让init_task进程隶属到idle调度类中。即选择idle的调度相关函数。

4.	调用schedule()函数切换当前进程，在调用该函数之前，Linux系统中只有两个进程，即0号进程init_task和1号进程kernel_init，其中kernel_init进程也是刚刚被创建的。调用该函数后，1号进程kernel_init将会运行！

5.	调用cpu_idle()，0号线程进入idle函数的循环，在该循环中会周期性地检查。

###创建kernel_init
-------

在rest_init函数中，内核将通过下面的代码产生第一个真正的进程(pid=1):

```c
kernel_thread(kernel_init, NULL, CLONE_FS);
```

这个进程就是着名的pid为1的init进程，它会继续完成剩下的初始化工作，然后execve(/sbin/init), 成为系统中的其他所有进程的祖先。

>但是这里我们发现一个问题, init进程应该是一个用户空间的进程, 但是这里却是通过kernel_thread的方式创建的, 哪岂不是式一个永远运行在内核态的内核线程么, 它是怎么演变为真正意义上用户空间的init进程的？
>
>1号kernel_init进程完成linux的各项配置(包括启动AP)后，就会在/sbin,/etc,/bin寻找init程序来运行。该init程序会替换kernel_init进程（注意：并不是创建一个新的进程来运行init程序，而是一次变身，使用sys_execve函数改变核心进程的正文段，将核心进程kernel_init转换成用户进程init），此时处于内核态的1号kernel_init进程将会转换为用户空间内的1号进程init。户进程init将根据/etc/inittab中提供的信息完成应用程序的初始化调用。然后init进程会执行/bin/sh产生shell界面提供给用户来与Linux系统进行交互。
>
>调用init_post()创建用户模式1号进程。

关于init其他的信息我们这次先不研究，因为我们这篇旨在探究0号进程的详细过程，

###创建kthreadd
-------
在rest_init函数中，内核将通过下面的代码产生第一个kthreadd(pid=2)

```c
pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
```

它的任务就是管理和调度其他内核线程kernel_thread, 会循环执行一个kthread的函数，该函数的作用就是运行kthread_create_list全局链表中维护的kthread, 当我们调用kernel_thread创建的内核线程会被加入到此链表中，因此所有的内核线程都是直接或者间接的以kthreadd为父进程


##0号进程演变为idle
-------

```c
	/*
 	* The boot idle thread must execute schedule()
 	* at least once to get things moving:
 	*/
	init_idle_bootup_task(current);
	schedule_preempt_disabled();
	/* Call into cpu_idle with preempt disabled */
	cpu_startup_entry(CPUHP_ONLINE);
```

因此我们回过头来看pid=0的进程，在创建了init进程后，pid=0的进程调用 cpu_idle()演变成了idle进程。

0号进程首先执行init_idle_bootup_task, **让init_task进程隶属到idle调度类中**。即选择idle的调度相关函数。

这个函数被定义在[kernel/sched/core.c](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.5#L5075)中，如下

```c
void init_idle_bootup_task(struct task_struct *idle)
{
	idle->sched_class = &idle_sched_class;
}
```

接着通过schedule_preempt_disabled来**执行调用schedule()函数切换当前进程**，在调用该函数之前，Linux系统中只有两个进程，即0号进程init_task和1号进程kernel_init，其中kernel_init进程也是刚刚被创建的。调用该函数后，1号进程kernel_init将会运行

这个函数被定义在[kernel/sched/core.c](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.5#L3342)中，如下

```c
/**
* schedule_preempt_disabled - called with preemption disabled
*
* Returns with preemption disabled. Note: preempt_count must be 1
*/
void __sched schedule_preempt_disabled(void)
{
	sched_preempt_enable_no_resched();
	schedule();
	preempt_disable();
}
```


最后cpu_startup_entry**调用cpu_idle_loop()，0号线程进入idle函数的循环，在该循环中会周期性地检查**

cpu_startup_entry定义在[kernel/sched/idle.c](http://lxr.free-electrons.com/source/kernel/sched/idle.c?v=4.5#L276)

```c
 void cpu_startup_entry(enum cpuhp_state state)
{
	/*
	* This #ifdef needs to die, but it's too late in the cycle to
	* make this generic (arm and sh have never invoked the canary
	* init for the non boot cpus!). Will be fixed in 3.11
	*/
#ifdef CONFIG_X86
    /*
    * If we're the non-boot CPU, nothing set the stack canary up
    * for us. The boot CPU already has it initialized but no harm
    * in doing it again. This is a good place for updating it, as
    * we wont ever return from this function (so the invalid
    * canaries already on the stack wont ever trigger).
    */
    boot_init_stack_canary();
#endif
    arch_cpu_idle_prepare();
    cpu_idle_loop();
}
```

其中cpu_idle_loop就是idle进程的事件循环，定义在[kernel/sched/idle.c](http://lxr.free-electrons.com/source/kernel/sched/idle.c?v=4.5#L203)




整个过程简单的说就是，原始进程(pid=0)创建init进程(pid=1),然后演化成idle进程(pid=0)。init进程为每个从处理器(运行队列)创建出一个idle进程(pid=0)，然后演化成/sbin/init。


#idle的运行与调度
-------


##idle的workload--cpu_idle_loop
-------

从上面的分析我们知道，idle在系统没有其他就绪的进程可执行的时候才会被调度。不管是主处理器，还是从处理器，最后都是执行的cpu_idle_loop()函数

其中cpu_idle_loop就是idle进程的事件循环，定义在[kernel/sched/idle.c](http://lxr.free-electrons.com/source/kernel/sched/idle.c?v=4.5#L203)，早期的版本中提供的是[cpu_idle](http://lxr.free-electrons.com/ident?v=3.9;i=cpu_idle)，但是这个函数是完全依赖于体系结构的，不利用架构的分层，因此在新的内核中更新为更加通用的cpu_idle_loop，由他来调用体系结构相关的代码

所以我们来看看cpu_idle_loop做了什么事情。

因为idle进程中并不执行什么有意义的任务，所以通常考虑的是两点

1.	节能

2.	低退出延迟。

其代码如下

```c
/*
 * Generic idle loop implementation
 *
 * Called with polling cleared.
 */
static void cpu_idle_loop(void)
{
        while (1) {
                /*
                 * If the arch has a polling bit, we maintain an invariant:
                 *
                 * Our polling bit is clear if we're not scheduled (i.e. if
                 * rq->curr != rq->idle).  This means that, if rq->idle has
                 * the polling bit set, then setting need_resched is
                 * guaranteed to cause the cpu to reschedule.
                 */

                __current_set_polling();
                quiet_vmstat();
                tick_nohz_idle_enter();

                while (!need_resched()) {
                        check_pgt_cache();
                        rmb();

                        if (cpu_is_offline(smp_processor_id())) {
                                rcu_cpu_notify(NULL, CPU_DYING_IDLE,
                                               (void *)(long)smp_processor_id());
                                smp_mb(); /* all activity before dead. */
                                this_cpu_write(cpu_dead_idle, true);
                                arch_cpu_idle_dead();
                        }

                        local_irq_disable();
                        arch_cpu_idle_enter();

                        /*
                         * In poll mode we reenable interrupts and spin.
                         *
                         * Also if we detected in the wakeup from idle
                         * path that the tick broadcast device expired
                         * for us, we don't want to go deep idle as we
                         * know that the IPI is going to arrive right
                         * away
                         */
                        if (cpu_idle_force_poll || tick_check_broadcast_expired())
                                cpu_idle_poll();
                        else
                                cpuidle_idle_call();

                        arch_cpu_idle_exit();
                }

                /*
                 * Since we fell out of the loop above, we know
                 * TIF_NEED_RESCHED must be set, propagate it into
                 * PREEMPT_NEED_RESCHED.
                 *
                 * This is required because for polling idle loops we will
                 * not have had an IPI to fold the state for us.
                 */
                preempt_set_need_resched();
                tick_nohz_idle_exit();
                __current_clr_polling();

                /*
                 * We promise to call sched_ttwu_pending and reschedule
                 * if need_resched is set while polling is set.  That
                 * means that clearing polling needs to be visible
                 * before doing these things.
                 */
                smp_mb__after_atomic();

                sched_ttwu_pending();
                schedule_preempt_disabled();
        }
}
```

循环判断need_resched以降低退出延迟，用idle()来节能。

默认的idle实现是hlt指令，hlt指令使CPU处于暂停状态，等待硬件中断发生的时候恢复，从而达到节能的目的。即从处理器C0态变到 C1态(见 ACPI标准)。这也是早些年windows平台上各种"处理器降温"工具的主要手段。当然idle也可以是在别的ACPI或者APM模块中定义的，甚至是自定义的一个idle(比如说nop)。



　　1.idle是一个进程，其pid为0。

　　2.主处理器上的idle由原始进程(pid=0)演变而来。从处理器上的idle由init进程fork得到，但是它们的pid都为0。

　　3.Idle进程为最低优先级，且不参与调度，只是在运行队列为空的时候才被调度。

　　4.Idle循环等待need_resched置位。默认使用hlt节能。

　　希望通过本文你能全面了解linux内核中idle知识。

##idle的调度和运行时机
-------

我们知道, linux进程的调度顺序是按照 rt实时进程(rt调度器), normal普通进程(cfs调度器)，和idel的顺序来调度的

那么可以试想如果rt和cfs都没有可以运行的任务，那么idle才可以被调度，那么他是通过怎样的方式实现的呢？


由于我们还没有讲解调度器的知识, 所有我们只是简单讲解一下

在normal的调度类,cfs公平调度器[sched_fair.c](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.5#L8408)中, 我们可以看到

```c
static const struct sched_class fair_sched_class = {
.next = &idle_sched_class,
```
也就是说，如果系统中没有普通进程，那么会选择下个调度类优先级的进程，即使用idle_sched_class调度类进行调度的进程

当系统空闲的时候，最后就是调用idle的pick_next_task函数，被定义在/kernel/sched/idle_task.c中

>参见
>
>http://lxr.free-electrons.com/source/kernel/sched/idle_task.c?v=4.5#L27

```c
static struct task_struct *pick_next_task_idle(struct rq *rq)
{
        schedstat_inc(rq, sched_goidle);
        calc_load_account_idle(rq);
        return rq->idle;    //可以看到就是返回rq中idle进程。
}
```

这idle进程在启动start_kernel函数的时候调用init_idle函数的时候，把当前进程（0号进程）置为每个rq运行队列的的idle上。
```
rq->curr = rq->idle = idle;
```
这里idle就是调用start_kernel函数的进程，就是0号进程。



#idle进程总结
-------


系统允许一个进程创建新进程，新进程即为子进程，子进程还可以创建新的子进程，形成进程树结构模型。整个linux系统的所有进程也是一个树形结构。**树根是系统自动构造的(或者说是由内核黑客手动创建的)**，即在内核态下执行的0号进程，它是所有进程的远古先祖。

>在smp系统中，每个处理器单元有独立的一个运行队列，而每个运行队列上又有一个idle进程，即有多少处理器单元，就有多少idle进程。

<font color=#A52A2A>
1.	idle进程其pid=0，其前身是系统创建的第一个进程(我们称之为init_task)，也是唯一一个没有通过fork或者kernel_thread产生的进程。

2.	**init_task**是内核中所有进程、线程的task_struct雏形，它是在内核初始化过程中，通过静态定义构造出了一个task_struct接口，取名为init_task，然后在内核初始化的后期，在rest_init()函数中通过kernel_thread创建了两个内核线程**内核init线程，kthreadd内核线程**, 前者后来通过演变，进入用户空间，成为所有用户进程的先祖,  而后者则成为所有内核态其他守护线程的父线程, 负责接手内核线程的创建工作

3.	然后init_task通过变更调度类为sched_idle等操作演变成为**idle进程**, 此时系统中只有0(idle), 1(init), 2(kthreadd)3个进程, 然后执行一次进程调度, 必然切换当前进程到到init
</font>


#附录--rest_init的执行解析
-------

| rest_init 流程 | 说明 |
| ------------- |:-------------:|
| rcu_scheduler_starting	| 启动Read-Copy Update,会调用num_online_cpus确认目前只有bootstrap处理器在运作,以及调用nr_context_switches确认在启动RCU前,没有进行过Contex-Switch,最后就是设定rcu_scheduler_active=1启动RCU机制. RCU在多核心架构下,不同的行程要读取同一笔资料内容/结构,可以提供高效率的同步与正确性. 在这之后就可以使用 rcu_read_lock/rcu_read_unlock了 |
| 产生Kernel Thread kernel_init | Kernel Thread函式 kernel_init实例在init/main.c中, init Task PID=1,是内核第一个产生的Task. 产生后,会阻塞在wait_for_completion处,等待kthreadd_done Signal,以便往后继续执行下去. |
|产生Kernel Thread kthreadd | Kernel Thread函式 kthreadd实例在kernel/kthread.c中, kthreadd Task PID=2,是内核第二个产生的Task. |
| find_task_by_pid_ns	| 实例在kernel/pid.c中, 调用函数find_task_by_pid_ns,并传入参数kthreadd的PID 2与PID NameSpace (struct pid_namespace init_pid_ns)取回PID 2的Task Struct. |
| complete	| 实例在kernel/sched.c中, 会发送kthreadd_done Signal,让 kernel_init(也就是 init task)可以往后继续执行. |
| init_idle_bootup_task	| 实例在kernel/sched.c中, 设定目前启动的Task为IDLE Task. (idle->sched_class = &idle_sched_class), 而struct sched_class idle_sched_class的定义在kernel/sched_idletask.c中. 在Linux下IDLE Task并不占用PID(也可以把它当作是PID 0),每个处理器都会有这洋的IDLE Task,用来在没有行程排成时,让处理器掉入执行的.而最基础的省电机制,也可透过IDLE Task来进行. (包括让系统可以关闭必要的周边电源与Clock Gating). |
| schedule_preempt_disabled(); | 启动一次Linux Kernel Process的排成Context-Switch调度机制, 从而使得kernel_init即1号进程获得处理机 |
| cpu_startup_entry | 完成工作后, 调用cpu_idle_loop()使得idle进程进入自己的事件处理循环 |


