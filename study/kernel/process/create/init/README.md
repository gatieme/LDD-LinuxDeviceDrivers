| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------------- |:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|
| 2016-05-12 | [Linux-4.5](http://lxr.free-electrons.com/source/?v=4.5) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux-进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |


http://www.linuxidc.com/Linux/2013-07/87011.htm
http://www.linuxeye.com/Linux/1827.html
http://bbs.csdn.net/topics/390872515
http://blog.csdn.net/yjzl1911/article/details/5613569
http://blog.csdn.net/dagouaofei/article/details/5644119
http://blog.chinaunix.net/uid-23769728-id-3129443.html
http://baike.baidu.com/link?url=sCsQDvMUaAikV5W_eKrEL3RVijNHJtOJk8nsCnjlxtnU7yoJ9svp6cwaerQ6Dqc0I-kdoAYrOtMcocCUnzyggK
http://blog.chinaunix.net/uid-21718047-id-3070635.html
http://blog.163.com/boneshunter_1234/blog/static/340762320084472122207/
http://blog.sina.com.cn/s/blog_626aed8b0100hws6.html

#Linux下进程的创建流程
-------

##进程的复制fork和加载execve
-------

我们在Linux下进行进行编程，往往都是通过fork出来一个新的程序，fork从化字面意义上理解就是说"分叉", 这其实就意味着我们的fork进程并不是真正从无到有被创建出来的。


一个进程，包括代码、数据和分配给进程的资源，它其实是从现有的进程（父进程）复制出的一个副本（子进程），fork（）函数通过系统调用创建一个与原来进程几乎完全相同的进程，也就是两个进程可以做完全相同的事，然后如果我们通过execve为子进程加载新的应用程序后，那么新的进程将开始执行新的应用

简单来说，<font color = 0x00ffff>新的进程是通过fork和execve创建的，首先通过fork从父进程分叉出一个基本一致的副本，然后通过execve来加载新的应用程序镜像</font>

*	fork生成当前进程的的一个相同副本，该副本成为子进程
	
>    原进程（父进程）的所有资源都以适当的方法复制给新的进程（子进程）。因此该系统调用之后，原来的进程就有了两个独立的实例，这两个实例的联系包括：同一组打开文件, 同样的工作目录, 进程虚拟空间（内存）中同样的数据（当然两个进程各有一份副本, 也就是说他们的虚拟地址相同, 但是所对应的物理地址不同）等等。

*	execve从一个可执行的二进制程序镜像加载应用程序, 来代替当前运行的进程

>	换句话说, 加载了一个新的应用程序。因此execv并不是创建新进程

所以<font color = 0x00ffff>我们在linux要创建一个应用程序的时候，其实执行的操作就是

1.	首先使用fork复制一个旧的进程

2.	然后调用execve在为新的进程加载一个新的应用程序
</font>


#写时复制技术
-------

有人认为这样大批量的复制会导致执行效率过低。其实在复制过程中，linux采用了写时复制的策略。


写入时复制(Copy-on-write)是一个被使用在程式设计领域的最佳化策略。其基础的观念是，如果有多个呼叫者(callers)同时要求相同资源，他们会共同取得相同的指标指向相同的资源，直到某个呼叫者(caller)尝试修改资源时，系统才会真正复制一个副本(private copy)给该呼叫者，以避免被修改的资源被直接察觉到，这过程对其他的呼叫只都是通透的(transparently)。此作法主要的优点是如果呼叫者并没有修改该资源，就不会有副本(private copy)被建立。

第一代Unix系统实现了一种傻瓜式的进程创建：当发出fork()系统调用时，内核原样复制父进程的整个地址空间并把复制的那一份分配给子进程。这种行为是非常耗时的，这种创建地址空间的方法涉及许多内存访问，消耗许多CPU周期，并且完全破坏了高速缓存中的内容。在大多数情况下，这样做常常是毫无意义的，因为许多子进程通过装入一个新的程序开始它们的执行，这样就完全丢弃了所继承的地址空间。

现在的Linux内核采用一种更为有效的方法，称之为写时复制（Copy On Write，COW）。这种思想相当简单：父进程和子进程共享页帧而不是复制页帧。然而，只要页帧被共享，它们就不能被修改，即页帧被保护。无论父进程还是子进程何时试图写一个共享的页帧，就产生一个异常，这时内核就把这个页复制到一个新的页帧中并标记为可写。原来的页帧仍然是写保护的：当其他进程试图写入时，内核检查写进程是否是这个页帧的唯一属主，如果是，就把这个页帧标记为对这个进程是可写的。

当进程A使用系统调用fork创建一个子进程B时,由于子进程B实际上是父进程A的一个拷贝,

因此会拥有与父进程相同的物理页面.为了节约内存和加快创建速度的目标,fork()函数会让子进程B以只读方式共享父进程A的物理页面.同时将父进程A对这些物理页面的访问权限也设成只读.

这样,当父进程A或子进程B任何一方对这些已共享的物理页面执行写操作时,都会产生页面出错异常(page_fault int14)中断,此时CPU会执行系统提供的异常处理函数do_wp_page()来解决这个异常.

do_wp_page()会对这块导致写入异常中断的物理页面进行取消共享操作,为写进程复制一新的物理页面,使父进程A和子进程B各自拥有一块内容相同的物理页面.最后,从异常处理函数中返回时,CPU就会重新执行刚才导致异常的写入操作指令,使进程继续执行下去.

一个进程调用fork（）函数后，系统先给新的进程分配资源，例如存储数据和代码的空间。然后把原来的进程的所有值都复制到新的新进程中，只有少数值与原来的进程的值（比如PID）不同。相当于克隆了一个自己。



#0号进程与1号进程
-------

前面我们了解到linux下的进程创建式通过父进程复制自身分叉出的一个副本, 那么我们的系统中就必然存在一个进程是所有进程的祖先，要不然我们从哪里fork分叉出一个个子进程呢，linux下这个进程就是init进程

<font color=0x009966>但是问题也来了，我们的祖先进程init进程，是从哪里来的?

他总不能也是被分叉来的吧？

如果是，那么分叉它的进程是谁，那么它为什么没有成为祖先进程

如果不是, 那么好了, 它是怎么创建出来的（真正的从无到有）
</font>


Linux下有两个特殊的进程，idel进程($PID = 0$)和init进程($PID = 1$)

*	idel进程由系统自动创建, 运行在内核态
	
    idle进程其pid=0，其前身是系统创建的第一个进程，也是唯一一个没有通过fork()产生的进程。

*	init进程由idel创建, 
	
    Linux中的所有进程都是有init进程创建并运行的。首先Linux内核启动，然后在用户空间中启动init进程，再启动其他系统进程。在系统启动完成完成后，init将变为守护进程监视系统其他进程。


系统允许一个进程创建新进程，新进程即为子进程，子进程还可以创建新的子进程，形成进程树结构模型。整个linux系统的所有进程也是一个树形结构。**树根是系统自动构造的(或者说是由内核黑客手动创建的)**，即在内核态下执行的0号进程，它是所有进程的远古先祖。

由0号进程创建1号进程（内核态），1号负责执行内核的部分初始化工作及进行系统配置，并创建若干个用于高速缓存和虚拟主存管理的内核线程。随后，1号进程调用execve（）运行可执行程序init，并演变成用户态1号进程，即init进程。它按照配置文件/etc/initab的要求，完成系统启动工作，创建编号为1号、2号...的若干终端注册进程getty。

每个getty进程设置其进程组标识号，并监视配置到系统终端的接口线路。当检测到来自终端的连接信号时，getty进程将通过函数execve（）执行注册程序login，此时用户就可输入注册名和密码进入登录过程，如果成功，由login程序再通过函数execv（）执行shell，该shell进程接收getty进程的pid，取代原来的getty进程。再由shell直接或间接地产生其他进程。

 

  上述过程可描述为：0号进程->1号内核进程->1号用户进程（init进程）->getty进程->shell进程

   注意，上述过程描述中提到：1号内核进程调用执行init并演变成1号用户态进程（init进程），这里前者是init是函数，后者是进程。两者容易混淆，区别如下：

   1.init（）函数在内核态运行，是内核代码

   2.init进程是内核启动并运行的第一个用户进程，运行在用户态下。

   3.一号内核进程调用execve（）从文件/etc/inittab中加载可执行程序init并执行，这个过程并没有使用调用do_fork()，因此两个进程都是1号进程。

 

转自：http://qhwang.blogbus.com/logs/46874366.html，本人根据自己的理解做了一定修改

#init进程
-------
init进程是linux内核启动的第一个用户级进程。init有许多很重要的任务，比如像启动getty（用于用户登录）、实现运行级别、以及处理孤立进程。


Linux系统中的init进程(pid=1)是除了idle进程(pid=0，也就是init_task)之外另一个比较特殊的进程，它是Linux内核开始建立起进程概念时第一个通过kernel_thread产生的进程，其开始在内核态执行，然后通过一个系统调用，开始执行用户空间的/sbin/init程序，期间Linux内核也经历了从内核态到用户态的特权级转变，/sbin/init极有可能产生出了shell，然后所有的用户进程都有该进程派生出来(目前尚未阅读过/sbin/init的源码)...


对于Linux系统的运行来说，init程序是最基本的程序之一。但你仍可以大部分的忽略它。一个好的Linux发行版本通常随带有一个init的配置，这个配置适合于绝大多数系统的工作，在这样一些系统上不需要对init做任何事。通常，只有你在碰到诸如串行终端挂住了、拨入（不是拨出）调制解调器、或者你希望改变缺省的运行级别时你才需要关心init。

当内核启动了自己之后（已被装入内存、已经开始运行、已经初始化了所有的设备驱动程序和数据结构等等），通过启动用户级程序init来完成引导进程的内核部分。因此，init总是第一个进程（它的进程号总是1）。

内核在几个位置上来查寻init，这几个位置以前常用来放置init，但是init的最适当的位置（在Linux系统上）是/sbin/init。如果内核没有找到init，它就会试着运行/bin/sh，如果还是失败了，那么系统的启动就宣告失败了。

当init开始运行，它通过执行一些管理任务来结束引导进程，例如检查文件系统、清理/tmp、启动各种服务以及为每个终端和虚拟控制台启动getty，在这些地方用户将登录系统。

在系统完全起来之后，init为每个用户已退出的终端重启getty（这样下一个用户就可以登录）。init同样也收集孤立的进程：当一个进程启动了一个子进程并且在子进程之前终止了，这个子进程立刻成为init的子进程。对于各种技术方面的原因来说这是很重要的，知道这些也是有好处的，因为这便于理解进程列表和进程树图。init的变种很少。绝大多数Linux发行版本使用sysinit（由Miguel van Smoorenburg著），它是基于System V的init设计。UNIX的BSD版本有一个不同的init。最主要的不同在于运行级别：System V有而BSD没有（至少是传统上说）。这种区别并不是主要的。在此我们仅讨论sysvinit。 配置init以启动getty：/etc/inittab文件
当init启动后，init读取/etc/inittab配置文件。当系统正在运行时，如果发出HUP信号，init会重读它；这个特性就使得对init的配置文件作过的更改不需要再重新启动系统就能起作用了。 /etc/inittab文件有点复杂。我们将从配置getty行的简单情况说起。
etc/inittab中的行由四个冒号限定的域组成：
id:runlevels:action:process
下面对各个域进行了描述。另外，/etc/inittab可以包含空行以及以数字符号（’#’）开始的行；这些行均被忽略。
id 这确定文件中的一行。对于getty行来说，指定了它在其上运行的终端（设备文件名/dev/tty后面的字符）。对于别的行来说，是没有意义的（除了有长度的限制），但它必须是唯一的。
runlevels 该行应考虑的运行级别。运行级别以单个数字给出，没有分隔符。
action 对于该行应采取的动作，也即，respawn再次运行下一个域中的命令，当它存在时，或者仅运行一次。


kasflaskflaskflaskflas;kfas;
asfsafasf

目前我们至少知道在内核空间执行用户空间的一段应用程序有两种方法：
1. call_usermodehelper
2. kernel_execve

它们最终都通过int $0x80在内核空间发起一个系统调用来完成，这个过程我在《深入Linux设备驱动程序内核机制》第9章有过详细的描述，对它的讨论最终结束在 sys_execve函数那里，后者被用来执行一个新的程序。现在一个有趣的问题是，在内核空间发起的系统调用，最终通过sys_execve来执行用户 空间的一个程序，比如/sbin/myhotplug，那么该应用程序执行时是在内核态呢还是用户态呢？直觉上肯定是用户态，不过因为cpu在执行 sys_execve时cs寄存器还是__KERNEL_CS，如果前面我们的猜测是真的话，必然会有个cs寄存器的值从__KERNEL_CS到 __USER_CS的转变过程，这个过程是如何发生的呢？下面我以kernel_execve为例，来具体讨论一下其间所发生的一些有趣的事情。 

start_kernel在其最后一个函数rest_init的调用中，会通过kernel_thread来生成一个内核进程，后者则会在新进程环境下调 用kernel_init函数，kernel_init一个让人感兴趣的地方在于它会调用run_init_process来执行根文件系统下的 /sbin/init等程序： 


#idel的创建
-------

>idle进程其pid=0，其前身是系统创建的第一个进程，也是唯一一个没有通过fork()产生的进程。在smp系统中，每个处理器单元有独立的一个运行队列，而每个运行队列上又有一个idle进程，即有多少处理器单元，就有多少idle进程。系统的空闲时间，其实就是指idle进程的"运行时间"。既然是idle是进程，那我们来看看idle是如何被创建，又具体做了哪些事情？

　　我们知道系统是从BIOS加电自检，载入MBR中的引导程序(LILO/GRUB),再加载linux内核开始运行的，一直到指定shell开始运行告一段落，这时用户开始操作Linux。而大致是在vmlinux的入口startup_32(head.S)中为pid号为0的原始进程设置了执行环境，然后原是进程开始执行start_kernel()完成Linux内核的初始化工作。包括初始化页表，初始化中断向量表，初始化系统时间等。继而调用 fork(),创建第一个用户进程:

　　kernel_thread(kernel_init, NULL, CLONE_FS | CLONE_SIGHAND);

　　这个进程就是着名的pid为1的init进程，它会继续完成剩下的初始化工作，然后execve(/sbin/init), 成为系统中的其他所有进程的祖先。关于init我们这次先不研究，回过头来看pid=0的进程，在创建了init进程后，pid=0的进程调用 cpu_idle()演变成了idle进程。

　　current_thread_info()->status |= TS_POLLING;

　　在 smp系统中，除了上面刚才我们讲的主处理器(执行初始化工作的处理器)上idle进程的创建，还有从处理器(被主处理器activate的处理器)上的 idle进程，他们又是怎么创建的呢？接着看init进程，init在演变成/sbin/init之前，会执行一部分初始化工作，其中一个就是 smp_prepare_cpus()，初始化SMP处理器，在这过程中会在处理每个从处理器时调用

　　task = copy_process(CLONE_VM, 0, idle_regs(&regs), 0, NULL, NULL, 0);

　　init_idle(task, cpu);

　　即从init中复制出一个进程，并把它初始化为idle进程(pid仍然为0)。从处理器上的idle进程会进行一些Activate工作，然后执行cpu_idle()。

　　整个过程简单的说就是，原始进程(pid=0)创建init进程(pid=1),然后演化成idle进程(pid=0)。init进程为每个从处理器(运行队列)创建出一个idle进程(pid=0)，然后演化成/sbin/init。

　　3. idle的运行时机

　　idle 进程优先级为MAX_PRIO，即最低优先级。早先版本中，idle是参与调度的，所以将其优先级设为最低，当没有其他进程可以运行时，才会调度执行 idle。而目前的版本中idle并不在运行队列中参与调度，而是在运行队列结构中含idle指针，指向idle进程，在调度器发现运行队列为空的时候运行，调入运行。

　　4. idle的workload

　　从上面的分析我们可以看出，idle在系统没有其他就绪的进程可执行的时候才会被调度。不管是主处理器，还是从处理器，最后都是执行的cpu_idle()函数。所以我们来看看cpu_idle做了什么事情。

　　因为idle进程中并不执行什么有意义的任务，所以通常考虑的是两点：1.节能，2.低退出延迟。

　　其核心代码如下：

　　void cpu_idle(void)  {   int cpu = smp_processor_id();    current_thread_info()->status |= TS_POLLING;    /* endless idle loop with no priority at all */   while (1) {     tick_nohz_stop_sched_tick(1);     while (!need_resched()) {       check_pgt_cache();      rmb();       if (rcu_pending(cpu))       rcu_check_callbacks(cpu, 0);       if (cpu_is_offline(cpu))       play_dead();       local_irq_disable();      __get_cpu_var(irq_stat).idle_timestamp = jiffies;      /* Don't trace irqs off for idle */      stop_critical_timings();      pm_idle();      start_critical_timings();     }     tick_nohz_restart_sched_tick();     preempt_enable_no_resched();     schedule();     preempt_disable();   }  }

　　循环判断need_resched以降低退出延迟，用idle()来节能。

　　默认的idle实现是hlt指令，hlt指令使CPU处于暂停状态，等待硬件中断发生的时候恢复，从而达到节能的目的。即从处理器C0态变到 C1态(见 ACPI标准)。这也是早些年windows平台上各种"处理器降温"工具的主要手段。当然idle也可以是在别的ACPI或者APM模块中定义的，甚至是自定义的一个idle(比如说nop)。

　　小结:

　　1.idle是一个进程，其pid为0。

　　2.主处理器上的idle由原始进程(pid=0)演变而来。从处理器上的idle由init进程fork得到，但是它们的pid都为0。

　　3.Idle进程为最低优先级，且不参与调度，只是在运行队列为空的时候才被调度。

　　4.Idle循环等待need_resched置位。默认使用hlt节能。

　　希望通过本文你能全面了解linux内核中idle知识。

