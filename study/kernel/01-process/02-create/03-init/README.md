Linux下1号进程的前世(kernel_init)今生(init进程)
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



我们下面就详解分析1号进程的前世(kernel_init)今生(init进程)

Linux系统中的init进程(pid=1)是除了idle进程(pid=0，也就是init_task)之外另一个比较特殊的进程，它是Linux内核开始建立起进程概念时第一个通过kernel_thread产生的进程，其开始在内核态执行，然后通过一个系统调用，开始执行用户空间的/sbin/init程序，期间Linux内核也经历了从内核态到用户态的特权级转变，/sbin/init极有可能产生出了shell，然后所有的用户进程都有该进程派生出来


#1号进程
-------

前面我们了解到了0号进程是系统所有进程的先祖, 它的进程描述符init_task是内核静态创建的, 而它在进行初始化的时候, 通过kernel_thread的方式创建了两个内核线程，分别是kernel_init和kthreadd，其中kernel_init进程号为1


>start_kernel在其最后一个函数rest_init的调用中，会通过kernel_thread来生成一个内核进程，后者则会在新进程环境下调 用kernel_init函数，kernel_init一个让人感兴趣的地方在于它会调用run_init_process来执行根文件系统下的 /sbin/init等程序：


##kernel_init
-------

0号进程创建1号进程的方式如下

```c
kernel_thread(kernel_init, NULL, CLONE_FS);
```

我们发现1号进程的执行函数就是kernel_init, 这个函数被定义[init/main.c](http://lxr.free-electrons.com/source/init/main.c?v=4.5#L932)中，如下所示

>kernel_init函数将完成设备驱动程序的初始化，并调用init_post函数启动用户空间的init进程。



由0号进程创建1号进程（内核态），1号内核线程负责执行内核的部分初始化工作及进行系统配置，并创建若干个用于高速缓存和虚拟主存管理的内核线程。

##init进程
-------

随后，1号进程调用do_execve运行可执行程序init，并演变成用户态1号进程，即init进程。

init进程是linux内核启动的第一个用户级进程。init有许多很重要的任务，比如像启动getty（用于用户登录）、实现运行级别、以及处理孤立进程。



它按照配置文件/etc/initab的要求，完成系统启动工作，创建编号为1号、2号...的若干终端注册进程getty。

每个getty进程设置其进程组标识号，并监视配置到系统终端的接口线路。当检测到来自终端的连接信号时，getty进程将通过函数do_execve（）执行注册程序login，此时用户就可输入注册名和密码进入登录过程，如果成功，由login程序再通过函数execv（）执行shell，该shell进程接收getty进程的pid，取代原来的getty进程。再由shell直接或间接地产生其他进程。


上述过程可描述为：0号进程->1号内核进程->1号用户进程（init进程）->getty进程->shell进程

注意，上述过程描述中提到：1号内核进程调用执行init函数并演变成1号用户态进程（init进程），这里前者是init是函数，后者是进程。两者容易混淆，区别如下：

1.	kernel_init函数在内核态运行，是内核代码

2.	init进程是内核启动并运行的第一个用户进程，运行在用户态下。

3.	一号内核进程调用execve()从文件/etc/inittab中加载可执行程序init并执行，这个过程并没有使用调用do_fork()，因此两个进程都是1号进程。


当内核启动了自己之后（已被装入内存、已经开始运行、已经初始化了所有的设备驱动程序和数据结构等等），通过启动用户级程序init来完成引导进程的内核部分。因此，init总是第一个进程（它的进程号总是1）。

当init开始运行，它通过执行一些管理任务来结束引导进程，例如检查文件系统、清理/tmp、启动各种服务以及为每个终端和虚拟控制台启动getty，在这些地方用户将登录系统。

在系统完全起来之后，init为每个用户已退出的终端重启getty（这样下一个用户就可以登录）。init同样也收集孤立的进程：当一个进程启动了一个子进程并且在子进程之前终止了，这个子进程立刻成为init的子进程。对于各种技术方面的原因来说这是很重要的，知道这些也是有好处的，因为这便于理解进程列表和进程树图。init的变种很少。绝大多数Linux发行版本使用sysinit（由Miguel van Smoorenburg著），它是基于System V的init设计。UNIX的BSD版本有一个不同的init。最主要的不同在于运行级别：System V有而BSD没有（至少是传统上说）。这种区别并不是主要的。在此我们仅讨论sysvinit。 配置init以启动getty：/etc/inittab文件


##关于init程序
-------

1号进程通过execve执行init程序来进入用户空间，成为init进程，那么这个init在哪里呢

内核在几个位置上来查寻init，这几个位置以前常用来放置init，但是init的最适当的位置（在Linux系统上）是/sbin/init。如果内核没有找到init，它就会试着运行/bin/sh，如果还是失败了，那么系统的启动就宣告失败了。

因此init程序是一个可以又用户编写的进程, 如果希望看init程序源码的朋友，可以参见

| init包 | 说明 | 学习链接 |
| ------------- |:-------------:|:-------------:|
| sysvinit | 早期一些版本使用的初始化进程工具, 目前在逐渐淡出linux历史舞台, sysvinit 就是 system V 风格的 init 系统，顾名思义，它源于 System V 系列 UNIX。它提供了比 BSD 风格 init 系统更高的灵活性。是已经风行了几十年的 UNIX init 系统，一直被各类 Linux 发行版所采用。| [浅析 Linux 初始化 init 系统（1）：sysvinit](http://blog.jobbole.com/85076)
| [upstart](http://upstart.ubuntu.com/index.html) | debian, Ubuntu等系统使用的initdaemon | [浅析 Linux 初始化 init 系统（2）： UpStart](http://blog.jobbole.com/85107/) |
| systemd | Systemd 是 Linux 系统中最新的初始化系统（init），它主要的设计目标是克服 sysvinit 固有的缺点，提高系统的启动速度 | [浅析 Linux 初始化 init 系统（3） Systemd](http://blog.jobbole.com/85070/)

Ubuntu等使用deb包的系统可以通过dpkg -S查看程序所在的包

![Ubuntu](./images/dpkg-upstart.jpg)

CentOS等使用rpm包的系统可以通过rpm -qf查看系统程序所在的包

![早期centos](./images/sysvinit.jpg)

![新版centos](./images/upstart.jpg)

>参见
>
>[ Linux下查看并下载命令源码包（根据命令/应用程序逆向获取并且安装其所属源码包）](http://blog.csdn.net/gatieme/article/details/51353648)


#附录
-------

##kernel_init_freeable流程分析
-------
```c
static noinline void __init kernel_init_freeable(void)
{
    /*
     * Wait until kthreadd is all set-up.
     */
    wait_for_completion(&kthreadd_done);

    /* Now the scheduler is fully set up and can do blocking allocations */
    gfp_allowed_mask = __GFP_BITS_MASK;

    /*
     * init can allocate pages on any node
     */
    set_mems_allowed(node_states[N_MEMORY]);
    /*
     * init can run on any cpu.
     */
    set_cpus_allowed_ptr(current, cpu_all_mask);

    cad_pid = task_pid(current);

    smp_prepare_cpus(setup_max_cpus);
 
    do_pre_smp_initcalls();
    lockup_detector_init();
 
    smp_init();
    sched_init_smp();
 
    page_alloc_init_late();
 
    do_basic_setup();
 
    /* Open the /dev/console on the rootfs, this should never fail */
    if (sys_open((const char __user *) "/dev/console", O_RDWR, 0) < 0)
            pr_err("Warning: unable to open an initial console.\n");
 
    (void) sys_dup(0);
    (void) sys_dup(0);
    /*
     * check if there is an early userspace init.  If yes, let it do all
     * the work
     */
 
    if (!ramdisk_execute_command)
            ramdisk_execute_command = "/init";
 
    if (sys_access((const char __user *) ramdisk_execute_command, 0) != 0) {
            ramdisk_execute_command = NULL;
            prepare_namespace();
    }
 
    /*
     * Ok, we have completed the initial bootup, and
     * we're essentially up and running. Get rid of the
     * initmem segments and start the user-mode stuff..
     *
     * rootfs is available now, try loading the public keys
     * and default modules
     */
 
    integrity_load_keys();
    load_default_modules();
 }
```

| 执行流程 | 说明 |
| ------------- |:-------------:|
| wait_for_completion | 实例在kernel/sched/completion.c中, 等待Kernel Thread kthreadd (PID=2)创建完毕 |
| gfp_allowed_mask| __GFP_BITS_MASK;设置bitmask, 使得init进程可以使用PM并且允许I/O阻塞操作 |
|  set_mems_allowed(node_states[N_MEMORY]); | init进程可以分配物理页面 |
| set_cpus_allowed_ptr | 通过设置cpu_bit_mask, 可以限定task只能在特定的处理器上运行, 而initcurrent进程此时必然是init进程，设置其cpu_all_mask即使得init进程可以在任意的cpu上运行 |
| task_pid | 设置到目前运行进程init的pid号给[cad_pid](http://lxr.free-electrons.com/source/kernel/reboot.c?v=4.5#L24)(cad_pid是用来接收`ctrl-alt-del` reboot signal的进程, 如果设置[C_A_D=1](http://lxr.free-electrons.com/source/kernel/reboot.c?v=4.5#L24)就表示可以处理来自`ctl-alt-del`的动作)， 最后会调用 [ctrl_alt_del(void)](http://lxr.free-electrons.com/source/kernel/reboot.c?v=4.5#L379)并确认C_A_D是否为1,确认完成后将执行cad_work=[deferred_cad](http://lxr.free-electrons.com/source/kernel/reboot.c?v=4.5#L369),执行[kernel_restart](http://lxr.free-electrons.com/source/kernel/reboot.c?v=4.5#L214) |
|  smp_prepare_cpus |体系结构相关的函数，实例在arch/arm/kernel/smp.c中,调用smp_prepare_cpus时,会以全局变量setup_max_cpus为函式参数max_cpus,以表示在编译核心时,设定支援的最大CPU数量 |
| do_pre_smp_initcalls| 实例在[init/main.c](http://lxr.free-electrons.com/source/init/main.c?v=4.5#L889)中, 会透过函式do_one_initcall,执行Symbol中 __initcall_start与__early_initcall_end之间的函数 |
| smp_init | 实例在kernel/smp.c中, 函数主要是由Bootstrap处理器,进行Active多核心架构下其它的处理器. 如果发生Online的处理器个数(from num_online_cpus)超过在核心编译时,所设定的最大处理器个数 setup_max_cpus (from NR_CPUS),就会终止流程.如果该处理器目前属於Present （也就是存在系统中),但尚未是Online的状态,就会呼叫函式cpu_up(in kernel/cpu.c)来啟动该处理器.|
| sched_init_smp | 实例在kernel/sched.c中, (1), 呼叫get_online_cpus,如果目前CPU Hotplug Active Write行程是自己,就直接返回.反之就把 cpu_hotplug.refcount加1 (表示多一个Reader)<br> (2),取得Mutex Lock “sched_domains_mutex”<br>(3),呼叫arch_init_sched_domains,设定scheduler domains与groups,参考Linux Documentation/scheduler/sched-domains.txt文件,一个Scheduling Domain会包含一个或多个CPU Groups,排程的Load-Balance就会根据Domain中的Groups来做调整.<br>(4),释放Mutex Lock “sched_domains_mutex”<br>(5),呼叫put_online_cpus,如果目前CPU Hotplug Active Writer行程是自己,就直接返回.反之就把 cpu_hotplug.refcount减1,如果 cpu_hotplug.refcount减到為0,表示没有其他Reader,此时如果有CPU Hotplug Active Writer行程在等待,就会透过wake_up_process唤醒该行程,以便让等待中的Writer可以被执行下去.(也可以参考_cpu_up中对於函式cpu_hotplug_begin的说明).<br>(6)注册CPU Notifier cpuset_cpu_active/cpuset_cpu_inactive/update_runtime <br>(7),呼叫set_cpus_allowed_ptr,透过这函式可以设定CPU bitmask,限定Task只能在特定的处理器上运作.在这会用参数”non_isolated_cpus”,也就是会把init指定给non-isolated CPU. Linux Kernel可以在啟动时,透过Boot Parameters “isolcpus=“指定CPU编号或是范围,让这些处理器不被包含在Linux Kernel SMP balancing/scheduling算法内,可以在啟动后指派给特定的Task运作.而不在 “isolcpus=“ 指定范围内的处理器就算是non-isolated CPU.<br>(8),呼叫sched_init_granularity,透过函式update_sysctl,让sysctl_sched_min_granularity=normalized_sysctl_sched_min_granularity,sysctl_sched_latency=normalized_sysctl_sched_latency,sysctl_sched_wakeup_granularity=normalized_sysctl_sched_wakeup_granularit|
| do_basic_setup | 实例在init/main.c中,<br>1,diaousermodehelper_init (in kernel/kmod.c),产生khelper workqueue.<br>2,调用init_tmpfs (in mm/shmem.c),对VFS注册Temp FileSystem.<br>3,呼叫driver_init (in drivers/base/init.c),初始化Linux Kernel Driver System Model.<br>4,呼叫init_irq_proc(in kernel/irq/proc.c),初始化 “/proc/irq”与其下的File Nodes.<br>5,呼叫do_ctors (in init/main.c),执行位於Symbol __ctors_start 到 __ctors_end间属於Section  “.ctors” 的Constructor函式.<br>6,透过函式do_initcalls,执行介於Symbol  __early_initcall_end与__initcall_end之间的函式呼叫,<br> |
| sys_open| 实例在fs/fcntl.c中,”SYSCALL_DEFINE1(dup, unsigned int, fildes)”,在这会连续执行两次sys_dup,复制两个sys_open开啟/dev/console所產生的档案描述0 (也就是会多生出两个1与2),只是都对应到”/dev/console”,我们在System V streams下的Standard Stream一般而言会有如下的对应<br>0:Standard input (stdin)<br>1:Standard output (stdout)<br>2:Standard error (stderr)<br>(為方便大家参考,附上Wiki URL http://en.wikipedia.org/wiki/Standard_streams ) |
|ramdisk_execute_command与prepare_namespace	| 1,如果ramdisk_execute_command為0,就设定ramdisk_execute_command = “/init”<br>2,如果sys_access确认档案ramdisk_execute_command 失败,就把ramdisk_execute_command 设定為0,然后呼叫prepare_namespace去mount root FileSystem. |
| integrity_load_keys | 至此我们初始化工作完成, 文件系统也已经准备好了，那么接下来加载 load integrity keys hook |
| load_default_modules | 加载基本的模块 |

##kernel_init分析
-------
```c
static int __ref kernel_init(void *unused)
{
    int ret;

    kernel_init_freeable();
    /* need to finish all async __init code before freeing the memory */
    async_synchronize_full();
    free_initmem();
    mark_rodata_ro();
    system_state = SYSTEM_RUNNING;
    numa_default_policy();

    flush_delayed_fput();

    rcu_end_inkernel_boot();

    if (ramdisk_execute_command) {
            ret = run_init_process(ramdisk_execute_command);
            if (!ret)
                    return 0;
            pr_err("Failed to execute %s (error %d)\n",
                   ramdisk_execute_command, ret);
    }

    /*
     * We try each of these until one succeeds.
     *
     * The Bourne shell can be used instead of init if we are
     * trying to recover a really broken machine.
     */
    if (execute_command) {
            ret = run_init_process(execute_command);
            if (!ret)
                    return 0;
            panic("Requested init %s failed (error %d).",
                  execute_command, ret);
    }
    if (!try_to_run_init_process("/sbin/init") ||
        !try_to_run_init_process("/etc/init") ||
        !try_to_run_init_process("/bin/init") ||
        !try_to_run_init_process("/bin/sh"))
            return 0;

    panic("No working init found.  Try passing init= option to kernel. "
          "See Linux Documentation/init.txt for guidance.");
}
```

| 执行流程 | 说明 |
| ------------- |:-------------:|
| kernel_init_freeable | 调用kernel_init_freeable完成初始化工作，准备文件系统，准备模块信息 |
| async_synchronize_full | 用以同步所有非同步函式呼叫的执行,在这函数中会等待List async_running与async_pending都清空后,才会返回. Asynchronously called functions主要设计用来加速Linux Kernel开机的效率,避免在开机流程中等待硬体反应延迟,影响到开机完成的时间 |
| free_initmem| free_initmem(in arch/arm/mm/init.c),释放Linux Kernel介於__init_begin到 __init_end属于init Section的函数的所有内存.并会把Page个数加到变量totalram_pages中,作为后续Linux Kernel在配置记忆体时可以使用的Pages. (在这也可把TCM范围(__tcm_start到__tcm_end)释放加入到总Page中,但TCM比外部记忆体有效率,适合多媒体,中断,…etc等对效能要求高的执行需求,放到总Page中,成为可供一般目的配置的存储范围 |
| system_state | 设置运行状态SYSTEM_RUNNING |
| 加载init进程，进入用户空间 | a,如果ramdisk_execute_command不為0,就执行该命令成為init User Process.<br>b,如果execute_command不為0,就执行该命令成為init User Process.<br>c,如果上述都不成立,就依序執行如下指令<br>run_init_process(“/sbin/init”);<br>run_init_process(“/etc/init”);<br>run_init_process(“/bin/init”);<br>run_init_process(“/bin/sh”);<br>也就是说会按照顺序从/sbin/init, /etc/init, /bin/init 與 /bin/sh依序执行第一个 init User Process.<br> 如果都找不到可以執行的 init Process,就會進入Kernel Panic.如下所示panic(“No init found.  Try passing init= option to kernel. ”“See Linux Documentation/init.txt for guidance.”); |