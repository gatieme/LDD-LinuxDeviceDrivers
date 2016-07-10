Linux进程上下文切换过程context_switch详解
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |


前面我们了解了linux进程调度器的设计思路和注意框架

周期调度器scheduler_tick通过linux定时器周期性的被激活, 进行程序调度

进程主动放弃CPU或者发生阻塞时, 则会调用主调度器schedule进行程序调度

在分析的过程中, 我们提到了内核抢占和用户抢占的概念, 但是并没有详细讲, 因此我们在这里详细分析一下子



CPU抢占分两种情况, **用户抢占**, **内核抢占**

其中内核抢占是在Linux2.5.4版本发布时加入, 同SMP(Symmetrical Multi-Processing, 对称多处理器), 作为内核的可选配置。



#1  前景回顾
-------

##1.1  Linux的调度器组成
-------


**2个调度器**

可以用两种方法来激活调度

*	一种是直接的, 比如进程打算睡眠或出于其他原因放弃CPU

*	另一种是通过周期性的机制, 以固定的频率运行, 不时的检测是否有必要

因此当前linux的调度程序由两个调度器组成：**主调度器**，**周期性调度器**(两者又统称为**通用调度器(generic scheduler)**或**核心调度器(core scheduler)**)

并且每个调度器包括两个内容：**调度框架**(其实质就是两个函数框架)及**调度器类**



**6种调度策略**

linux内核目前实现了6中调度策略(即调度算法), 用于对不同类型的进程进行调度, 或者支持某些特殊的功能

*	SCHED_NORMAL和SCHED_BATCH调度普通的非实时进程

*	SCHED_FIFO和SCHED_RR和SCHED_DEADLINE则采用不同的调度策略调度实时进程

*	SCHED_IDLE则在系统空闲时调用idle进程.



**5个调度器类**

而依据其调度策略的不同实现了5个调度器类, 一个调度器类可以用一种种或者多种调度策略调度某一类进程, 也可以用于特殊情况或者调度特殊功能的进程.


其所属进程的优先级顺序为
```c
stop_sched_class -> dl_sched_class -> rt_sched_class -> fair_sched_class -> idle_sched_class
```


**3个调度实体**

调度器不限于调度进程, 还可以调度更大的实体, 比如实现组调度.

这种一般性要求调度器不直接操作进程, 而是处理可调度实体, 因此需要一个通用的数据结构描述这个调度实体,即seched_entity结构, 其实际上就代表了一个调度对象，可以为一个进程，也可以为一个进程组.

linux中针对当前可调度的实时和非实时进程, 定义了类型为seched_entity的3个调度实体

*	sched_dl_entity 采用EDF算法调度的实时调度实体

*	sched_rt_entity 采用Roound-Robin或者FIFO算法调度的实时调度实体 

*	sched_entity 采用CFS算法调度的普通非实时进程的调度实体



##1.2	调度工作
-------

周期性调度器通过调用各个调度器类的task_tick函数完成周期性调度工作

*	如果当前进程是**完全公平队列**中的进程, 则首先根据当前就绪队列中的进程数算出一个延迟时间间隔，大概每个进程分配2ms时间，然后按照该进程在队列中的总权重中占得比例，算出它该执行的时间X，如果该进程执行物理时间超过了X，则激发延迟调度；如果没有超过X，但是红黑树就绪队列中下一个进程优先级更高，即curr->vruntime-leftmost->vruntime > X,也将延迟调度

*	如果当前进程是实时调度类中的进程：则如果该进程是SCHED_RR，则递减时间片[为HZ/10]，到期，插入到队列尾部，并激发延迟调度，如果是SCHED_FIFO，则什么也不做，直到该进程执行完成

延迟调度**的真正调度过程在：schedule中实现，会按照调度类顺序和优先级挑选出一个最高优先级的进程执行

而对于主调度器则直接关闭内核抢占后, 通过调用schedule来完成进程的调度


可见不管是周期性调度器还是主调度器, 内核中的许多地方, 如果要将CPU分配给与当前活动进程不同的另外一个进程(即抢占)，都会直接或者调用调度函数, 包括schedule或者其子函数__schedule, 其中schedule在关闭内核抢占后调用__schedule完成了抢占.

而__schedule则执行了如下操作


**__schedule如何完成内核抢占**

<font color=0x00ffff>

1.	完成一些必要的检查, 并设置进程状态, 处理进程所在的就绪队列

2.	调度全局的pick_next_task选择抢占的进程

	*	如果当前cpu上所有的进程都是cfs调度的普通非实时进程, 则直接用cfs调度, 如果无程序可调度则调度idle进程

	*	否则从优先级最高的调度器类sched_class_highest(目前是stop_sched_class)开始依次遍历所有调度器类的pick_next_task函数, 选择最优的那个进程执行

3.	context_switch完成进程上下文切换

即进程的抢占或者切换工作是由context_switch完成的

那么我们今天就详细讲解一下context_switch完成进程上下文切换的原理

</font>


#2	进程上下文
-------

##2.1	进程上下文的概念
-------

操作系统管理很多进程的执行. 有些进程是来自各种程序、系统和应用程序的单独进程，而某些进程来自被分解为很多进程的应用或程序。当一个进程从内核中移出，另一个进程成为活动的, 这些进程之间便发生了上下文切换. 操作系统必须记录重启进程和启动新进程使之活动所需要的所有信息. 这些信息被称作**上下文, 它描述了进程的现有状态, 进程上下文是可执行程序代码是进程的重要组成部分, 实际上是进程执行活动全过程的静态描述, 可以看作是用户进程传递给内核的这些参数以及内核要保存的那一整套的变量和寄存器值和当时的环境等**

进程的上下文信息包括， 指向可执行文件的指针, 栈, 内存(数据段和堆), 进程状态, 优先级, 程序I/O的状态, 授予权限, 调度信息, 审计信息, 有关资源的信息(文件描述符和读/写指针), 关事件和信号的信息, 寄存器组(栈指针, 指令计数器)等等, 诸如此类.


处理器总处于以下三种状态之一

１.	内核态，运行于进程上下文，内核代表进程运行于内核空间；

２.	内核态，运行于中断上下文，内核代表硬件运行于内核空间；

３.	用户态，运行于用户空间。

用户空间的应用程序，通过系统调用，进入内核空间。这个时候用户空间的进程要传递 很多变量、参数的值给内核，内核态运行的时候也要保存用户进程的一些寄存器值、变量等。所谓的"进程上下文"

硬件通过触发信号，导致内核调用中断处理程序，进入内核空间。这个过程中，硬件的 一些变量和参数也要传递给内核，内核通过这些参数进行中断处理。所谓的"中断上下文"，其实也可以看作就是硬件传递过来的这些参数和内核需要保存的一些其他环境（主要是当前被打断执行的进程环境）。

>LINUX完全注释中的一段话
>
>当一个进程在执行时,CPU的所有寄存器中的值、进程的状态以及堆栈中的内容被称 为该进程的上下文。当内核需要切换到另一个进程时，它需要保存当前进程的 所有状态，即保存当前进程的上下文，以便在再次执行该进程时，能够必得到切换时的状态执行下去。在LINUX中，当前进程上下文均保存在进程的任务数据结 构中。在发生中断时,内核就在被中断进程的上下文中，在内核态下执行中断服务例程。但同时会保留所有需要用到的资源，以便中继服务结束时能恢复被中断进程 的执行.

##2.2	上下文切换

进程被抢占CPU时候, 操作系统保存其上下文信息, 同时将新的活动进程的上下文信息加载进来, 这个过程其实就是**上下文切换**, 而当一个被抢占的进程再次成为活动的, 它可以恢复自己的上下文继续从被抢占的位置开始执行. 参见维基百科-[context](https://en.wikipedia.org/wiki/Context_(computing), [context switch](https://en.wikipedia.org/wiki/Context_switch)

**上下文切换**(有时也称做**进程切换**或**任务切换**)是指CPU从一个进程或线程切换到另一个进程或线程

稍微详细描述一下，上下文切换可以认为是内核（操作系统的核心）在 CPU 上对于进程（包括线程）进行以下的活动：

1.	挂起一个进程，将这个进程在 CPU 中的状态（上下文）存储于内存中的某处，

2.	在内存中检索下一个进程的上下文并将其在 CPU 的寄存器中恢复

3.	跳转到程序计数器所指向的位置（即跳转到进程被中断时的代码行），以恢复该进程


因此上下文是指某一时间点CPU寄存器和程序计数器的内容, 广义上还包括内存中进程的虚拟地址映射信息.

上下文切换只能发生在内核态中, 上下文切换通常是计算密集型的。也就是说，它需要相当可观的处理器时间，在每秒几十上百次的切换中，每次切换都需要纳秒量级的时间。所以，上下文切换对系统来说意味着消耗大量的 CPU 时间，事实上，可能是操作系统中时间消耗最大的操作。
Linux相比与其他操作系统（包括其他类 Unix 系统）有很多的优点，其中有一项就是，其上下文切换和模式切换的时间消耗非常少.




#3	context_switch进程上下文切换
-------

linux中进程调度时, 内核在选择新进程之后进行抢占时, 通过context_switch完成进程上下文切换.

>**注意**		进程调度与抢占的区别
>
>进程调度不一定发生抢占, 但是抢占时却一定发生了调度
>
>在进程发生调度时, 只有当前内核发生当前进程因为主动或者被动需要放弃CPU时, 内核才会选择一个与当前活动进程不同的进程来抢占CPU



context_switch其实是一个分配器, 他会调用所需的特定体系结构的方法

*	调用switch_mm(), 把虚拟内存从一个进程映射切换到新进程中

	switch_mm更换通过task_struct->mm描述的内存管理上下文, 该工作的细节取决于处理器, 主要包括加载页表, 刷出地址转换后备缓冲器(部分或者全部), 向内存管理单元(MMU)提供新的信息

*	调用switch_to(),从上一个进程的处理器状态切换到新进程的处理器状态。这包括保存、恢复栈信息和寄存器信息

	switch_to切换处理器寄存器的呢内容和内核栈(虚拟地址空间的用户部分已经通过switch_mm变更, 其中也包括了用户状态下的栈, 因此switch_to不需要变更用户栈, 只需变更内核栈), 此段代码严重依赖于体系结构, 且代码通常都是用汇编语言编写.

<font color=0x00ffff>

context_switch函数建立next进程的地址空间。进程描述符的active_mm字段指向进程所使用的内存描述符，而mm字段指向进程所拥有的内存描述符。对于一般的进程，这两个字段有相同的地址，但是，内核线程没有它自己的地址空间而且它的 mm字段总是被设置为 NULL

context_switch( )函数保证：如果next是一个内核线程, 它使用prev所使用的地址空间
</font>

由于不同架构下地址映射的机制有所区别, 而寄存器等信息弊病也是依赖于架构的, 因此switch_mm和switch_to两个函数均是体系结构相关的



##3.1	context_switch完全注释
-------

context_switch定义在[kernel/sched/core.c#L2711](http://lxr.free-electrons.com/source/kernel/sched/core.c#L2711), 如下所示

```c
/*
 * context_switch - switch to the new MM and the new thread's register state.
 */
static __always_inline struct rq *
context_switch(struct rq *rq, struct task_struct *prev,
           struct task_struct *next)
{
    struct mm_struct *mm, *oldmm;

    /*  完成进程切换的准备工作  */
    prepare_task_switch(rq, prev, next);

    mm = next->mm;
    oldmm = prev->active_mm;
    /*
     * For paravirt, this is coupled with an exit in switch_to to
     * combine the page table reload and the switch backend into
     * one hypercall.
     */
    arch_start_context_switch(prev);

    /*  如果next是内核线程，则线程使用prev所使用的地址空间
     *  schedule( )函数把该线程设置为懒惰TLB模式
     *  内核线程并不拥有自己的页表集(task_struct->mm = NULL)
     *  它使用一个普通进程的页表集
     *  不过，没有必要使一个用户态线性地址对应的TLB表项无效
     *  因为内核线程不访问用户态地址空间。
    */
    if (!mm)        /*  内核线程无虚拟地址空间, mm = NULL*/
    {
        /*  内核线程的active_mm为上一个进程的mm
         *  注意此时如果prev也是内核线程,
         *  则oldmm为NULL, 即next->active_mm也为NULL  */
        next->active_mm = oldmm;
        /*  增加mm的引用计数  */
        atomic_inc(&oldmm->mm_count);
        /*  通知底层体系结构不需要切换虚拟地址空间的用户部分
         *  这种加速上下文切换的技术称为惰性TBL  */
        enter_lazy_tlb(oldmm, next);
    }
    else            /*  不是内核线程, 则需要切切换虚拟地址空间  */
        switch_mm(oldmm, mm, next);

    /*  如果prev是内核线程或正在退出的进程
     *  就重新设置prev->active_mm
     *  然后把指向prev内存描述符的指针保存到运行队列的prev_mm字段中
     */
    if (!prev->mm)
    {
        /*  将prev的active_mm赋值和为空  */
        prev->active_mm = NULL;
        /*  更新运行队列的prev_mm成员  */
        rq->prev_mm = oldmm;
    }
    /*
     * Since the runqueue lock will be released by the next
     * task (which is an invalid locking op but in the case
     * of the scheduler it's an obvious special-case), so we
     * do an early lockdep release here:
     */
    lockdep_unpin_lock(&rq->lock);
    spin_release(&rq->lock.dep_map, 1, _THIS_IP_);

    /* Here we just switch the register state and the stack. 
     * 切换进程的执行环境, 包括堆栈和寄存器
     * 同时返回上一个执行的程序
     * 相当于prev = witch_to(prev, next)  */
    switch_to(prev, next, prev);
    
    /*  switch_to之后的代码只有在
     *  当前进程再次被选择运行(恢复执行)时才会运行
     *  而此时当前进程恢复执行时的上一个进程可能跟参数传入时的prev不同
     *  甚至可能是系统中任意一个随机的进程
     *  因此switch_to通过第三个参数将此进程返回
     */
    

    /*  路障同步, 一般用编译器指令实现
     *  确保了switch_to和finish_task_switch的执行顺序
     *  不会因为任何可能的优化而改变  */
    barrier();  

    /*  进程切换之后的处理工作  */
    return finish_task_switch(prev);
}
````

##3.2	prepare_arch_switch切换前的准备工作
-------

在进程切换之前, 首先执行调用每个体系结构都必须定义的prepare_task_switch挂钩, 这使得内核执行特定于体系结构的代码, 为切换做事先准备. 大多数支持的体系结构都不需要该选项


```c
struct mm_struct *mm, *oldmm;

prepare_task_switch(rq, prev, next);	/*  完成进程切换的准备工作  */
```

prepare_task_switch函数定义在[kernel/sched/core.c, line 2558](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L2558), 如下所示

```c
/**
 * prepare_task_switch - prepare to switch tasks
 * @rq: the runqueue preparing to switch
 * @prev: the current task that is being switched out
 * @next: the task we are going to switch to.
 *
 * This is called with the rq lock held and interrupts off. It must
 * be paired with a subsequent finish_task_switch after the context
 * switch.
 *
 * prepare_task_switch sets up locking and calls architecture specific
 * hooks.
 */
static inline void
prepare_task_switch(struct rq *rq, struct task_struct *prev,
            struct task_struct *next)
{
    sched_info_switch(rq, prev, next);
    perf_event_task_sched_out(prev, next);
    fire_sched_out_preempt_notifiers(prev, next);
    prepare_lock_switch(rq, next);
    prepare_arch_switch(next);
}
````

##3.3	next是内核线程时的处理
-------

由于用户空间进程的寄存器内容在进入核心态时保存在内核栈中, 在上下文切换期间无需显式操作. 而因为每个进程首先都是从核心态开始执行(在调度期间控制权传递给新进程), 在返回用户空间时, 会使用内核栈上保存的值自动恢复寄存器数据.


另外需要注意, 内核线程没有自身的用户空间上下文, 其task_struct->mm为NULL, 参见[Linux内核线程kernel thread详解--Linux进程的管理与调度（十）](http://blog.csdn.net/gatieme/article/details/51589205#t3), 从当前进程"借来"的地址空间记录在active_mm中

```c
/*  如果next是内核线程，则线程使用prev所使用的地址空间
 *  schedule( )函数把该线程设置为懒惰TLB模式
 *  内核线程并不拥有自己的页表集(task_struct->mm = NULL)
 *  它使用一个普通进程的页表集
 *  不过，没有必要使一个用户态线性地址对应的TLB表项无效
 *  因为内核线程不访问用户态地址空间。
*/
if (!mm)        /*  内核线程无虚拟地址空间, mm = NULL*/
{
    /*  内核线程的active_mm为上一个进程的mm
     *  注意此时如果prev也是内核线程,
     *  则oldmm为NULL, 即next->active_mm也为NULL  */
    next->active_mm = oldmm;
    /*  增加mm的引用计数  */
    atomic_inc(&oldmm->mm_count);
    /*  通知底层体系结构不需要切换虚拟地址空间的用户部分
     *  这种加速上下文切换的技术称为惰性TBL  */
    enter_lazy_tlb(oldmm, next);
}
else            /*  不是内核线程, 则需要切切换虚拟地址空间  */
    switch_mm(oldmm, mm, next);
````

qizhongenter_lazy_tlb通知底层体系结构不需要切换虚拟地址空间的用户空间部分, 这种加速上下文切换的技术称之为惰性TLB

##3.4	switch_mm切换进程虚拟地址空间
-------

###3.4.1	switch_mm函数
-------

switch_mm主要完成了进程prev到next虚拟地址空间的映射, 由于内核虚拟地址空间是不许呀切换的, 因此切换的主要是用户态的虚拟地址空间

这个是一个体系结构相关的函数, 其实现在对应体系结构下的[arch/对应体系结构/include/asm/mmu_context.h](http://lxr.free-electrons.com/ident?v=4.6;i=switch_mm)文件中, 我们下面列出了几个常见体系结构的实现

| 体系结构 | switch_mm实现 |
| ------- |:-------:|
| x86 | [arch/x86/include/asm/mmu_context.h, line 118](http://lxr.free-electrons.com/source/arch/x86/include/asm/mmu_context.h?v=4.6#L118) |
| arm | [arch/arm/include/asm/mmu_context.h, line 126](http://lxr.free-electrons.com/source/arch/arm/include/asm/mmu_context.h?v=4.6#L126) |
| arm64 | [arch/arm64/include/asm/mmu_context.h, line 183](http://lxr.free-electrons.com/source/arch/arm64/include/asm/mmu_context.h?v=4.6#L183)

其主要工作就是切换了进程的CR3

###3.4.2  CPU-CR0~CR4寄存器
-------

控制寄存器（CR0～CR3）用于控制和确定处理器的操作模式以及当前执行任务的特性


| 控制寄存器 | 描述 |
| ------- |:-------:|
| CR0 | 含有控制处理器操作模式和状态的系统控制标志 |
| CR1 | 保留不用, 未定义的控制寄存器，供将来的处理器使用 |
| CR3 | 含有页目录表物理内存基地址，因此该寄存器也被称为页目录基地址寄存器PDBR（Page-Directory Base address Register）, 保存页目录表的物理地址，页目录表总是放在以4K字节为单位的存储器边界上，因此，它的地址的低12位总为0，不起作用，即使写上内容，也不会被理会 |
| CR4 | 在Pentium系列（包括486的后期版本）处理器中才实现，它处理的事务包括诸如何时启用虚拟8086模式等 |

### 3.4.3	保护模式下的GDT、LDT和IDT
-------

保护模式下三个重要的系统表——GDT、LDT和IDT

这三个表是在内存中由操作系统或系统程序员所建，并不是固化在哪里，所以从理论上是可以被读写的。

这三个表都是描述符表. 描述符表是由若干个描述符组成, 每个描述符占用8个字节的内存空间, 每个描述符表内最多可以有8129个描述符. 描述符是描述一个段的大小，地址及各种状态的。

描述符表有三种,分别为**全局描述符表GDT**、**局部描述符表LDT**和**中断描述符表IDT** 


| 描述符表 | 描述 |
| ------- |:-------:|
| 全局描述符表GDT | 全局描述符表在系统中只能有一个,且可以被每一个任务所共享.
任何描述符都可以放在GDT中,但中断门和陷阱门放在GDT中是不会起作用的. 能被多个任务共享的内存区就是通过GDT完成的 |
| 局部描述符表LDT | 局部描述符表在系统中可以有多个,通常情况下是与任务的数量保持对等,但任务可以没有局部描述符表.<br><br>任务间不相干的部分也是通过LDT实现的.这里涉及到地址映射的问题.<br><br>和GDT一样,中断门和陷阱门放在LDT中是不会起作用的. |
| 中断描述符表IDT | 和GDT一样,中断描述符表在系统最多只能有一个,中断描述符表内可以存放256个描述符,分别对应256个中断.因为每个描述符占用8个字节,所以IDT的长度可达2K.<br><br>中断描述符表中可以有任务门、中断门、陷阱门三个门描述符，其它的描述符在中断描述符表中无意义 |

**段选择子**

在保护模式下,段寄存器的内容已不是段值,而称其为选择子.
该选择子指示描述符在上面这三个表中的位置,所以说选择子即是索引值。
当我们把段选择子装入寄存器时不仅使该寄存器值，同时CPU将该选择子所对应的GDT或LDT中的描述符装入了不可见部分。
这样只要我们不进行代码切换（不重新装入新的选择子）CPU就不会对不可见部分存储的描述符进行更新，可以直接进行访问，加快了访问速度。
一旦寄存器被重新赋值，不可见部分也将被重新赋值。

**关于选择子的值是否连续**

关于选择子的值，我认为不一定要连续。
但是每个描述符的起始地址相对于第一个描述符（即空描述符）的首地址的偏移必须是8的倍数，即二进制最后三位为0。这样通过全局描述符表寄存器GDTR找到全局描述符表的首地址后，使用段选择子的高13位索引到正确的描述符表项（段选择子的高13位左移3位加上GDTR的值即为段选择子指定的段描述符的逻辑首地址）

也就是说在两个段选择符之间可以填充能被8整除个字节值。当然，如果有选择子指向了这些填充的字节，一般会出错，除非你有意填充一些恰当的数值，呵呵。

**关于为什么LDT要放在GDT中 -LDT中的描述符和GDT中的描述符**

除了选择子的bit3一个为0一个为1用于区分该描述符是在GDT中还是在LDT中外，描述符本身的结构完全一样。
开始我考虑既然是这样，为什么要将LDT放在GDT中而不是像GDT那样找一个GDTR寄存器呢？

后来终于明白了原因——很简单，
GDT表只有一个，是固定的；而LDT表每个任务就可以有一个，因此有多个，并且由于任务的个数在不断变化其数量也在不断变化。

如果只有一个LDTR寄存器显然不能满足多个LDT的要求。因此INTEL的做法是把它放在放在GDT中。

##3.5	prev是内核线程时的处理
-------

如果前一个进程prev四内核线程(即prev->mm为NULL), 则其active_mm指针必须重置为NULL, 已断开其于之前借用的地址空间的联系, 而当prev重新被调度的时候, 此时它成为next会在前面[next是内核线程时的处理](未填写网址)处重新用`next->active_mm = oldmm;`赋值, 这个我们刚讲过


```c
/*  如果prev是内核线程或正在退出的进程
 *  就重新设置prev->active_mm
 *  然后把指向prev内存描述符的指针保存到运行队列的prev_mm字段中
 */
if (!prev->mm)
{
    /*  将prev的active_mm赋值和为空  */
    prev->active_mm = NULL;
    /*  更新运行队列的prev_mm成员  */
    rq->prev_mm = oldmm;
}
```

###3.4.4	switch_mm函数注释
-------


下面我们提取了x86架构下的switch_mm函数, 其定义在[arch/x86/include/asm/mmu_context.h, line 118](http://lxr.free-electrons.com/source/arch/x86/include/asm/mmu_context.h?v=4.6#L118)

```c
//  http://lxr.free-electrons.com/source/arch/x86/include/asm/mmu_context.h?v=4.6#L118
static inline void switch_mm(struct mm_struct *prev, struct mm_struct *next,
                 struct task_struct *tsk)
{
    unsigned cpu = smp_processor_id();


    /*  确保prev和next不是同一进程  */
    if (likely(prev != next))
    {
#ifdef CONFIG_SMP
        /*  刷新cpu地址转换后备缓冲器TLB  */
        this_cpu_write(cpu_tlbstate.state, TLBSTATE_OK);
        this_cpu_write(cpu_tlbstate.active_mm, next);
#endif
        /*  设置当前进程的mm->cpu_vm_mask表示其占用cpu  */
        cpumask_set_cpu(cpu, mm_cpumask(next));

        /*
         * Re-load page tables.
         *
         * This logic has an ordering constraint:
         *
         *  CPU 0: Write to a PTE for 'next'
         *  CPU 0: load bit 1 in mm_cpumask.  if nonzero, send IPI.
         *  CPU 1: set bit 1 in next's mm_cpumask
         *  CPU 1: load from the PTE that CPU 0 writes (implicit)
         *
         * We need to prevent an outcome in which CPU 1 observes
         * the new PTE value and CPU 0 observes bit 1 clear in
         * mm_cpumask.  (If that occurs, then the IPI will never
         * be sent, and CPU 0's TLB will contain a stale entry.)
         *
         * The bad outcome can occur if either CPU's load is
         * reordered before that CPU's store, so both CPUs must
         * execute full barriers to prevent this from happening.
         *
         * Thus, switch_mm needs a full barrier between the
         * store to mm_cpumask and any operation that could load
         * from next->pgd.  TLB fills are special and can happen
         * due to instruction fetches or for no reason at all,
         * and neither LOCK nor MFENCE orders them.
         * Fortunately, load_cr3() is serializing and gives the
         * ordering guarantee we need.
         * 
         * 将新进程的pgd页目录表填写到cpu的cr3寄存器中
         */
        load_cr3(next->pgd);

        trace_tlb_flush(TLB_FLUSH_ON_TASK_SWITCH, TLB_FLUSH_ALL);

        /* Stop flush ipis for the previous mm 
         * 除prev的cpu_vm_mask，表示prev放弃使用cpu  */
        cpumask_clear_cpu(cpu, mm_cpumask(prev));

        /* Load per-mm CR4 state
         */
        load_mm_cr4(next);

#ifdef CONFIG_MODIFY_LDT_SYSCALL
        /*
         * Load the LDT, if the LDT is different.
         *
         * It's possible that prev->context.ldt doesn't match
         * the LDT register.  This can happen if leave_mm(prev)
         * was called and then modify_ldt changed
         * prev->context.ldt but suppressed an IPI to this CPU.
         * In this case, prev->context.ldt != NULL, because we
         * never set context.ldt to NULL while the mm still
         * exists.  That means that next->context.ldt !=
         * prev->context.ldt, because mms never share an LDT.
         *
         * 
         */
        if (unlikely(prev->context.ldt != next->context.ldt))
            load_mm_ldt(next);
#endif
    }
#ifdef CONFIG_SMP
    else
    {
        this_cpu_write(cpu_tlbstate.state, TLBSTATE_OK);
        BUG_ON(this_cpu_read(cpu_tlbstate.active_mm) != next);

        if (!cpumask_test_cpu(cpu, mm_cpumask(next)))
        {
            /*
             * On established mms, the mm_cpumask is only changed
             * from irq context, from ptep_clear_flush() while in
             * lazy tlb mode, and here. Irqs are blocked during
             * schedule, protecting us from simultaneous changes.
             */
            cpumask_set_cpu(cpu, mm_cpumask(next));

            /*
             * We were in lazy tlb mode and leave_mm disabled
             * tlb flush IPI delivery. We must reload CR3
             * to make sure to use no freed page tables.
             *
             * As above, load_cr3() is serializing and orders TLB
             * fills with respect to the mm_cpumask write.
             */
            load_cr3(next->pgd);
            trace_tlb_flush(TLB_FLUSH_ON_TASK_SWITCH, TLB_FLUSH_ALL);
            load_mm_cr4(next);
            load_mm_ldt(next);
        }
    }
#endif
}
```

##3.6	switch_to完成进程切换
-------

###3.6.1	switch_to函数
-------

最后用switch_to完成了进程的切换, 该函数切换了寄存器状态和栈, 新进程在该调用后开始执行, 而switch_to之后的代码只有在当前进程下一次被选择运行时才会执行

执行环境的切换是在switch_to()中完成的, switch_to完成最终的进程切换，它保存原进程的所有寄存器信息，恢复新进程的所有寄存器信息，并执行新的进程

该函数往往通过宏来实现, 其原型声明如下

```c
/*
 * Saving eflags is important. It switches not only IOPL between tasks,
 * it also protects other tasks from NT leaking through sysenter etc.
*/
#define switch_to(prev, next, last)
```

| 体系结构 | switch_to实现 |
| ------- |:-------:|
| x86 | arch/x86/include/asm/switch_to.h中两种实现<br><br> [定义CONFIG_X86_32宏](http://lxr.free-electrons.com/source/arch/x86/include/asm/switch_to.h?v=4.6#L27)<br><br>[未定义CONFIG_X86_32宏](http://lxr.free-electrons.com/source/arch/x86/include/asm/switch_to.h?v=4.6#L103) |
| arm | [arch/arm/include/asm/switch_to.h, line 25](http://lxr.free-electrons.com/source/arch/arm/include/asm/switch_to.h?v=4.6#L18) |
| 通用 | [include/asm-generic/switch_to.h, line 25](http://lxr.free-electrons.com/source/include/asm-generic/switch_to.h?v=4.6#L25) |

内核在switch_to中执行如下操作

1.	进程切换, 即esp的切换, 由于从esp可以找到进程的描述符

2.	硬件上下文切换, 设置ip寄存器的值, 并jmp到__switch_to函数

3.	堆栈的切换, 即ebp的切换, ebp是栈底指针, 它确定了当前用户空间属于哪个进程


__switch_to函数

| 体系结构 | __switch_to实现 |
| ------- |:-------:|
| x86 | [arch/x86/kernel/process_32.c, line 242](http://lxr.free-electrons.com/source/arch/x86/kernel/process_32.c?v=4.6#L242) |
| x86_64 | [arch/x86/kernel/process_64.c, line 277](http://lxr.free-electrons.com/source/arch/x86/kernel/process_64.c?v=4.6#L277) |
| arm64 | [arch/arm64/kernel/process.c, line 329](http://lxr.free-electrons.com/source/arch/arm64/kernel/process.c?v=4.6#L329)






###3.6.2	为什么switch_to需要3个参数
-------

调度过程可能选择了一个新的进程, 而清理工作则是针对此前的活动进程, 请注意, 这不是发起上下文切换的那个进程, 而是系统中随机的某个其他进程, 内核必须想办法使得进程能够与context_switch例程通信, 这就可以通过switch_to宏实现. 因此switch_to函数通过3个参数提供2个变量.

在新进程被选中时, 底层的进程切换冽程必须将此前执行的进程提供给context_switch, 由于控制流会回到陔函数的中间, 这无法用普通的函数返回值来做到, 因此提供了3个参数的宏

我们考虑这个样一个例子, 假定多个进程A, B, C...在系统上运行, 在某个时间点, 内核决定从进程A切换到进程B, 此时prev = A, next = B, 即执行了switch_to(A, B), 而后当被抢占的进程A再次被选择执行的时候, 系统可能进行了多次进程切换/抢占(至少会经历一次即再次从B到A),假设A再次被选择执行时时当前活动进程是C, 即此时prev = C. next = A.

在每个switch_to被调用的时候, prev和next指针位于各个进程的内核栈中, prev指向了当前运行的进程, 而next指向了将要运行的下一个进程, 那么为了执行从prev到next的切换, switcth_to使用前两个参数prev和next就够了.

在进程A被选中再次执行的时候, 会出现一个问题, 此时控制权即将回到A, switch_to函数返回, 内核开始执行switch_to之后的点, 此时内核栈准确的恢复到切换之前的状态, 即进程A上次被切换出去时的状态, prev = A, next = B. 此时, 内核无法知道实际上在进程A之前运行的是进程C.


<font color=0x00ffff>

因此, 在新进程被选中执行时, 内核恢复到进程被切换出去的点继续执行, 此时内核只知道谁之前将新进程抢占了, 但是却不知道新进程再次执行是抢占了谁, 因此底层的进程切换机制必须将此前执行的进程(即新进程抢占的那个进程)提供给context_switch. 由于控制流会回到函数的该中间, 因此无法通过普通函数的返回值来完成. 因此使用了一个3个参数, 但是逻辑效果是相同的, 仿佛是switch_to是带有两个参数的函数, 而且返回了一个指向此前运行的进程的指针.
</font>

>switch_to(prev, next, last);
>
>即
>
>prev = last = switch_to(prev, next);

其中返回的prev值并不是做参数的prev值, 而是prev被再次调度的时候抢占掉的那个进程last.

在上个例子中, 进程A提供给switch_to的参数是prev = A, next = B, 然后控制权从A交给了B, 但是恢复执行的时候是通过prev = C, next = A完成了再次调度, 而后内核恢复了进程A被切换之前的内核栈信息, 即prev = A, next = B. 内核为了通知调度机制A抢占了C的处理器, 就通过last参数传递回来, prev = last = C.

内核实现该行为特性的方式依赖于底层的体系结构, 但内核显然可以通过考虑两个进程的内核栈来重建所需要的信息

###3.6.3	switch_to函数注释
-------

switch_mm()进行用户空间的切换, 更确切地说, 是切换地址转换表(pgd)， 由于pgd包括内核虚拟地址空间和用户虚拟地址空间地址映射, linux内核把进程的整个虚拟地址空间分成两个部分, 一部分是内核虚拟地址空间, 另外一部分是内核虚拟地址空间, 各个进程的虚拟地址空间各不相同, 但是却共用了同样的内核地址空间, 这样在进程切换的时候, 就只需要切换虚拟地址空间的用户空间部分.

每个进程都有其自身的页目录表pgd

进程本身尚未切换, 而存储管理机制的页目录指针cr3却已经切换了，这样不会造成问题吗?不会的，因为这个时候CPU在系统空间运行，而所有进程的页目录表中与系统空间对应的目录项都指向相同的页表，所以，不管切换到哪一个进程的页目录表都一样，受影响的只是用户空间，系统空间的映射则永远不变

我们下面来分析一下子, x86_32位下的switch_to函数, 其定义在[arch/x86/include/asm/switch_to.h, line 27](http://lxr.free-electrons.com/source/arch/x86/include/asm/switch_to.h?v=4.6#L27)

先对flags寄存器和ebp压入旧进程内核栈，并将确定旧进程恢复执行的下一跳地址，并将旧进程ip，esp保存到task_struct->thread_info中，这样旧进程保存完毕；然后用新进程的thread_info->esp恢复新进程的内核堆栈，用thread->info的ip恢复新进程地址执行。
关键点：内核寄存器[eflags、ebp保存到内核栈；内核栈esp地址、ip地址保存到thread_info中，task_struct在生命期中始终是全局的，所以肯定能根据该结构恢复出其所有执行场景来]

```c
/*
 * Saving eflags is important. It switches not only IOPL between tasks,
 * it also protects other tasks from NT leaking through sysenter etc.
 */
#define switch_to(prev, next, last)                                     \
do {                                                                    \
        /*                                                              \
         * Context-switching clobbers all registers, so we clobber      \
         * them explicitly, via unused output variables.                \
         * (EAX and EBP is not listed because EBP is saved/restored     \
         * explicitly for wchan access and EAX is the return value of   \
         * __switch_to())                                               \
         */                                                             \
        unsigned long ebx, ecx, edx, esi, edi;                          \
                                                                        \
        asm volatile("pushfl\n\t" /* save flags 保存就的ebp、和flags寄存器到旧进程的内核栈中*/   \
                     "pushl %%ebp\n\t"          /* save    EBP   */     \
                     "movl %%esp,%[prev_sp]\n\t"        /* save ESP  将旧进程esp保存到thread_info结构中 */ \
                     "movl %[next_sp],%%esp\n\t"        /* restore ESP 用新进程esp填写esp寄存器，此时内核栈已切换  */ \
                     "movl $1f,%[prev_ip]\n\t"  /* save EIP 将该进程恢复执行时的下条地址保存到旧进程的thread中*/     \
                     "pushl %[next_ip]\n\t"     /* restore EIP 将新进程的ip值压入到新进程的内核栈中 */     \
                     __switch_canary                                    \
                     "jmp __switch_to\n"        /* regparm call  */     \
                     "1:\t"                                             \
                     "popl %%ebp\n\t"           /* restore EBP 该进程执行，恢复ebp寄存器*/     \
                     "popfl\n"                  /* restore flags  恢复flags寄存器*/     \
                                                                        \
                     /* output parameters */                            \
                     : [prev_sp] "=m" (prev->thread.sp),                \
                       [prev_ip] "=m" (prev->thread.ip),                \
                       "=a" (last),                                     \
                                                                        \
                       /* clobbered output registers: */                \
                       "=b" (ebx), "=c" (ecx), "=d" (edx),              \
                       "=S" (esi), "=D" (edi)                           \
                                                                        \
                       __switch_canary_oparam                           \
                                                                        \
                       /* input parameters: */                          \
                     : [next_sp]  "m" (next->thread.sp),                \
                       [next_ip]  "m" (next->thread.ip),                \
                                                                        \
                       /* regparm parameters for __switch_to(): */      \
                       [prev]     "a" (prev),                           \
                       [next]     "d" (next)                            \
                                                                        \
                       __switch_canary_iparam                           \
                                                                        \
                     : /* reloaded segment registers */                 \
                        "memory");                                      \
} while (0)
```

##	3.7	barrier路障同步
-------


witch_to完成了进程的切换, 新进程在该调用后开始执行, 而switch_to之后的代码只有在当前进程下一次被选择运行时才会执行.

```c
/*  switch_to之后的代码只有在
 *  当前进程再次被选择运行(恢复执行)时才会运行
 *  而此时当前进程恢复执行时的上一个进程可能跟参数传入时的prev不同
 *  甚至可能是系统中任意一个随机的进程
 *  因此switch_to通过第三个参数将此进程返回
*/


/*  路障同步, 一般用编译器指令实现
 *  确保了switch_to和finish_task_switch的执行顺序
 *  不会因为任何可能的优化而改变  */
barrier();

/*  进程切换之后的处理工作  */
return finish_task_switch(prev);
```


而为了程序编译后指令的执行顺序不会因为编译器的优化而改变, 因此内核提供了路障同步barrier来保证程序的执行顺序.


barrier往往通过编译器指令来实现, 内核中多处都实现了barrier, 形式如下

```c
// http://lxr.free-electrons.com/source/include/linux/compiler-gcc.h?v=4.6#L15
/* Copied from linux/compiler-gcc.h since we can't include it directly 
 * 采用内敛汇编实现
 *  __asm__用于指示编译器在此插入汇编语句
 *  __volatile__用于告诉编译器，严禁将此处的汇编语句与其它的语句重组合优化。
 *  即：原原本本按原来的样子处理这这里的汇编。
 *  memory强制gcc编译器假设RAM所有内存单元均被汇编指令修改，这样cpu中的registers和cache中已缓存的内存单元中的数据将作废。cpu将不得不在需要的时候重新读取内存中的数据。这就阻止了cpu又将registers，cache中的数据用于去优化指令，而避免去访问内存。
 *  "":::表示这是个空指令。barrier()不用在此插入一条串行化汇编指令。在后文将讨论什么叫串行化指令。
*/
#define barrier() __asm__ __volatile__("": : :"memory")
```


关于内存屏障的详细信息, 可以参见 [Linux内核同步机制之（三）：memory barrier](http://www.wowotech.net/kernel_synchronization/memory-barrier.html)




##3.8	finish_task_switch完成清理工作
-------

finish_task_switch完成一些清理工作, 使得能够正确的释放锁, 但我们不会详细讨论这些. 他会向各个体系结构提供了另一个挂钩上下切换过程的可能性, 当然这只在少数计算机上需要.


前面我们谅解switch_to函数的3个参数时, 讲到
注：A进程切换到B, A被切换, 而当A再次被选择执行, C再次切换到A，此时A执行，但是系统为了告知调度器A再次执行前的进程是C, 通过switch_to的last参数返回的prev指向C,在A调度时候需要把调用A的进程的信息清除掉

由于从C切换到A时候, A内核栈中保存的实际上是A切换出时的状态信息, 即prev=A, next=B，但是在A执行时, 其位于context_switch上下文中, 该函数的last参数返回的prev应该是切换到A的进程C, A负责对C进程信息进行切换后处理，比如，如果切换到A后，A发现C进程已经处于TASK_DEAD状态，则将释放C进程的TASK_STRUCT结构


函数定义在[kernel/sched/core.c, line 2715](http://lxr.free-electrons.com/source/kernel/sched/core.c#?v=4.6L2715)中, 如下所示

```c
/**
 * finish_task_switch - clean up after a task-switch
 * @prev: the thread we just switched away from.
 *
 * finish_task_switch must be called after the context switch, paired
 * with a prepare_task_switch call before the context switch.
 * finish_task_switch will reconcile locking set up by prepare_task_switch,
 * and do any other architecture-specific cleanup actions.
 *
 * Note that we may have delayed dropping an mm in context_switch(). If
 * so, we finish that here outside of the runqueue lock. (Doing it
 * with the lock held can cause deadlocks; see schedule() for
 * details.)
 *
 * The context switch have flipped the stack from under us and restored the
 * local variables which were saved when this task called schedule() in the
 * past. prev == current is still correct but we need to recalculate this_rq
 * because prev may have moved to another CPU.
 */
static struct rq *finish_task_switch(struct task_struct *prev)
        __releases(rq->lock)
{
        struct rq *rq = this_rq();
        struct mm_struct *mm = rq->prev_mm;
        long prev_state;

        /*
         * The previous task will have left us with a preempt_count of 2
         * because it left us after:
         *
         *      schedule()
         *        preempt_disable();                    // 1
         *        __schedule()
         *          raw_spin_lock_irq(&rq->lock)        // 2
         *
         * Also, see FORK_PREEMPT_COUNT.
         */
        if (WARN_ONCE(preempt_count() != 2*PREEMPT_DISABLE_OFFSET,
                      "corrupted preempt_count: %s/%d/0x%x\n",
                      current->comm, current->pid, preempt_count()))
                preempt_count_set(FORK_PREEMPT_COUNT);

        rq->prev_mm = NULL;

        /*
         * A task struct has one reference for the use as "current".
         * If a task dies, then it sets TASK_DEAD in tsk->state and calls
         * schedule one last time. The schedule call will never return, and
         * the scheduled task must drop that reference.
         *
         * We must observe prev->state before clearing prev->on_cpu (in
         * finish_lock_switch), otherwise a concurrent wakeup can get prev
         * running on another CPU and we could rave with its RUNNING -> DEAD
         * transition, resulting in a double drop.
         */
        prev_state = prev->state;
        vtime_task_switch(prev);
        perf_event_task_sched_in(prev, current);
        finish_lock_switch(rq, prev);
        finish_arch_post_lock_switch();

        fire_sched_in_preempt_notifiers(current);
        if (mm)
                mmdrop(mm);
        if (unlikely(prev_state == TASK_DEAD))  /*  如果上一个进程已经终止，释放其task_struct 结构  */
        {
                if (prev->sched_class->task_dead)
                        prev->sched_class->task_dead(prev);

                /*
                 * Remove function-return probe instances associated with this
                 * task and put them back on the free list.
                 */
                kprobe_flush_task(prev);
                put_task_struct(prev);
        }

        tick_nohz_task_switch();
        return rq;
}
```




