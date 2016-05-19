Linux下进程的创建过程
=======


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




<font color=0x009966>Linux下有两个特殊的进程，idel进程($PID = 0$)和init进程($PID = 1$)

<font color=#A52A2A>
*	idel进程由系统自动创建, 运行在内核态
</font>
	idle进程其pid=0，其前身是系统创建的第一个进程，也是唯一一个没有通过fork()产生的进程。完成加载系统后，演变为进程调度、交换

<font color=#A52A2A>
*	init进程由idel创建,
</font>
	由0进程创建，完成系统的初始化. 是系统中所有其它用户进程的祖先进程
	Linux中的所有进程都是有init进程创建并运行的。首先Linux内核启动，然后在用户空间中启动init进程，再启动其他系统进程。在系统启动完成完成后，init将变为守护进程监视系统其他进程。
</font>



系统允许一个进程创建新进程，新进程即为子进程，子进程还可以创建新的子进程，形成进程树结构模型。整个linux系统的所有进程也是一个树形结构。**树根是系统自动构造的(或者说是由内核黑客手动创建的)**，即在内核态下执行的0号进程，它是所有进程的远古先祖。



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

