Linux用户抢占和内核抢占详解(概念, 实现和触发时机)
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

1.	完成一些必要的检查, 并设置进程状态, 处理进程所在的就绪队列

2.	调度全局的pick_next_task选择抢占的进程

	*	如果当前cpu上所有的进程都是cfs调度的普通非实时进程, 则直接用cfs调度, 如果无程序可调度则调度idle进程

	*	否则从优先级最高的调度器类sched_class_highest(目前是stop_sched_class)开始依次遍历所有调度器类的pick_next_task函数, 选择最优的那个进程执行

3.	context_switch完成进程上下文切换

即进程的抢占或者切换工作是由context_switch完成的

那么我们今天就详细讲解一下context_switch完成进程上下文切换的原理


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

*	调用switch_to(),从上一个进程的处理器状态切换到新进程的处理器状态。这包括保存、恢复栈信息和寄存器信息

context_switch函数建立next进程的地址空间。进程描述符的active_mm字段指向进程所使用的内存描述符，而mm字段指向进程所拥有的内存描述符。对于一般的进程，这两个字段有相同的地址，但是，内核线程没有它自己的地址空间而且它的 mm字段总是被设置为 NULL

context_switch( )函数保证：如果next是一个内核线程, 它使用prev所使用的地址空间

由于不同架构下地址映射的机制有所区别, 而寄存器等信息弊病也是依赖于架构的, 因此switch_mm和switch_to两个函数均是体系结构相关的


##3.1	context_switch流程
-------

context_switch定义在[kernel/sched/core.c#L2711](http://lxr.free-electrons.com/source/kernel/sched/core.c#L2711), 如下所示

```c

````

###switch_mm切换进程虚拟地址空间
-------

switch_mm主要完成了进程prev到next虚拟地址空间的映射, 由于内核虚拟地址空间是不许呀切换的, 因此切换的主要是用户态的虚拟地址空间

这个是一个体系结构相关的函数, 其实现在对应体系结构下的[arch/对应体系结构/include/asm/mmu_context.h](http://lxr.free-electrons.com/ident?v=4.6;i=switch_mm)文件中, 我们下面列出了几个常见体系结构的实现

| 体系结构 | switch_mm实现 |
| ------- |:-------:|
| x86 | [arch/x86/include/asm/mmu_context.h, line 118](http://lxr.free-electrons.com/source/arch/x86/include/asm/mmu_context.h?v=4.6#L118) |
| arm | [arch/arm/include/asm/mmu_context.h, line 126](http://lxr.free-electrons.com/source/arch/arm/include/asm/mmu_context.h?v=4.6#L126) |
| arm64 | [arch/arm64/include/asm/mmu_context.h, line 183](http://lxr.free-electrons.com/source/arch/arm64/include/asm/mmu_context.h?v=4.6#L183)

其主要工作就是切换了进程的CR3

>控制寄存器（CR0～CR3）用于控制和确定处理器的操作模式以及当前执行任务的特性
>
>CR0中含有控制处理器操作模式和状态的系统控制标志；
>
>CR1保留不用；
>
>CR2含有导致页错误的线性地址；
>
>CR3中含有页目录表物理内存基地址，因此该寄存器也被称为页目录基地址寄存器PDBR（Page-Directory Base address Register）。


###switch_to切换进程堆栈和寄存器
-------

执行环境的切换是在switch_to()中完成的, switch_to完成最终的进程切换，它保存原进程的所有寄存器信息，恢复新进程的所有寄存器信息，并执行新的进程

调度过程可能选择了一个新的进程, 而清理工作则是针对此前的活动进程, 请注意, 这不是发起上下文切换的那个进程, 而是系统中随机的某个其他进程, 内核必须想办法使得进程能够与context_switch例程通信, 这就可以通过switch_to宏实现. 因此switch_to函数通过3个参数提供2个变量, 

在新进程被选中时, 底层的进程切换冽程必须将此前执行的进程提供给context_switch, 由于控制流会回到陔函数的中间, 这无法用普通的函数返回值来做到, 因此提供了3个参数的宏

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


