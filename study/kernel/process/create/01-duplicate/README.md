Linux内核线程、轻量级进程和用户进程以及其创建方式
=======

**本文声明**

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------------- |:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|
| 2016-05-12 | [Linux-4.5](http://lxr.free-electrons.com/source/?v=4.5) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度-之-进程的创建](http://blog.csdn.net/gatieme/article/category/6225543) |


**本章链接**

|链接地址 | 上一节 | 本章目录 | 下一节 |
| ------------- |:-------------:|:-------------:|:-------------:|
| CSDN   | 已是第一篇  | 无 | [Linux下0号进程的前世(init_task进程)今生(idle进程)]()|
| GitHub | 已是第一篇  | [进程的创建](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/kernel/process/create) | [Linux下0号进程的前世(init_task进程)今生(idle进程)](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/kernel/process/create/02-idel)|

**章节链接**

|链接地址 | 上一章 | 总目录 | 下一章 |
| ------------- |:-------------:|:-------------:|:-------------:|
| CSDN   | [进程的描述](http://blog.csdn.net/gatieme/article/details/51383377)  | [目录](http://blog.csdn.net/gatieme/article/details/51456569) |[进程的调度]() |
| GitHub | [进程的描述](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/kernel/process/task)  | [目录](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/kernel/process) | [进程的调度]()|




#Linux进程类别
-------
虽然我们在区分Linux进程类别, 但是我还是想说Linux下只有一种类型的进程，那就是task_struct，当然我也想说linux其实也没有线程的概念, 只是将那些与其他进程共享资源的进程称之为线程。

1.	一个进程由于其运行空间的不同, 从而有**内核线程**和**用户进程**的区分, 内核线程运行在内核空间, 之所以称之为线程是因为它没有虚拟地址空间, 只能访问内核的代码和数据, 而用户进程则运行在用户空间, 但是可以通过中断, 系统调用等方式从用户态陷入内核态。

2.	用户进程运行在用户空间上, 而一些通过共享资源实现的一组进程我们称之为线程组, Linux下内核其实本质上没有线程的概念, Linux下线程其实上是与其他进程共享某些资源的进程而已。但是我们习惯上还是称他们为**线程**或者**轻量级进程**

因此, Linux上进程分3种，内核线程（或者叫核心进程）、用户进程、用户线程, 当然如果更严谨的，你也可以认为用户进程和用户线程都是用户进程。


>关于**轻量级进程**这个概念, 其实并不等价于**线程**
>
>不同的操作系统中依据其实现的不同, 轻量级进程其实是一个不一样的概念
>
>详细信息参见 [维基百科-LWP轻量级进程](https://en.wikipedia.org/wiki/Light-weight_process#See_also)
>
>或者本人的另外一篇博客[内核线程、轻量级进程、用户线程三种线程概念解惑（线程≠轻量级进程）](http://blog.csdn.net/gatieme/article/details/51481863)
>
>In computer operating systems, a light-weight process (LWP) is a means of achieving multitasking. In the traditional meaning of the term, as used in Unix System V and Solaris, a LWP runs in user space on top of a single kernel thread and shares its address space and system resources with other LWPs within the same process. Multiple user level threads, managed by a thread library, can be placed on top of one or many LWPs - allowing multitasking to be done at the user level, which can have some performance benefits.[1]
>
>In some operating systems there is no separate LWP layer between kernel threads and user threads. This means that user threads are implemented directly on top of kernel threads. In those contexts, the term "light-weight process" typically refers to kernel threads and the term "threads" can refer to user threads.[2] On Linux, user threads are implemented by allowing certain processes to share resources, which sometimes leads to these processes to be called "light weight processes".[3][4] Similarly, in SunOS version 4 onwards (prior to Solaris) "light weight process" referred to user threads.
>


#linux进程
-------

##Linux下进程和线程的区别

线程机制式现代编程技术中常用的一种抽象概念。该机制提供了同一个程序内共享内存地址空间，打开文件和资源的一组线程。

###专门线程支持的系统-LWP机制
-------

线程更好的支持了并发程序设计技术, 在多处理器系统上, 他能保证真正的并行处理。Microsoft Windows或是Sun Solaris等操作系统都对线程进行了支持。

这些系统中都在内核中提供了专门支持线程的机制, Unix System V和Sun Solaris将线程称作为轻量级进程(LWP-Light-weight process),在这些系统中, 相比较重量级进程, 线程被抽象成一种耗费较少资源, 运行迅速的执行单元。



###Linux下线程的实现机制
-------

但是Linux实现线程的机制非常独特。从内核的角度来说, 他并没有线程这个概念。Linux把所有的进程都当做进程来实现。内核中并没有准备特别的调度算法或者定义特别的数据结构来表示线程。相反, 线程仅仅被视为一个与其他进程共享某些资源的进程。每个线程都拥有唯一隶属于自己的task_struct, 所以在内核看来, 它看起来就像式一个普通的进程(只是线程和同组的其他进程共享某些资源)

在之前[Linux进程描述符task_struct结构体详解–Linux进程的管理与调度（一）](http://blog.csdn.net/gatieme/article/details/51383272)和[Linux进程ID号–Linux进程的管理与调度（三）](http://blog.csdn.net/gatieme/article/details/51383377)中讲解进程的pid号的时候我们就提到了, 进程task_struct中**pid存储的是内核对该进程的唯一标示**, 即对进程则标示进程号, 对线程来说就是其线程号, 那么**对于线程来说一个线程组所有线程与领头线程具有相同的进程号，存入tgid字段**

因此**getpid()返回当前进程的进程号，返回的应该是tgid值而不是pid的值, 对于用户空间来说同组的线程拥有相同进程号即tpid, 而对于内核来说, 某种成都上来说不存在线程的概念, 那么pid就是内核唯一区分每个进程的标示。 **

> 正是linux下组管理, 写时复制等这些巧妙的实现方式
>
>*	linux下进程或者线程的创建开销很小
>
>*	既然不管是线程或者进程内核都是不加区分的，一组共享地址空间或者资源的线程可以组成一个线程组, 那么其他进程即使不共享资源也可以组成进程组, 甚至来说一组进程组也可以组成会话组, 进程组可以简化向所有组内进程发送信号的操作, 一组会话也更能适应多道程序环境

###区别
-------


总而言之, <font color=#A52A2A>Linux中线程与专门线程支持系统是完全不同的</font>

Unix System V和Sun Solaris将用户线程称作为轻量级进程(LWP-Light-weight process), 相比较重量级进程, 线程被抽象成一种耗费较少资源, 运行迅速的执行单元。

而对于linux来说, 用户线程只是一种进程间共享资源的手段, 相比较其他系统的进程来说, linux系统的进程本身已经很轻量级了

举个例子来说, 假如我们有一个包括了四个线程的进程,

在提供专门线程支持的系统中, 通常会有一个包含只想四个不同线程的指针的进程描述符。该描述符复制描述像地址空间, 打开的文件这样的共享资源。线程本身再去描述它独占的资源。

相反, Linux仅仅创建了四个进程, 并分配四个普通的task_struct结构, 然后建立这四个进程时制定他们共享某些资源。



##Linux下进程的创建流程
-------

##进程的复制fork和加载execve
-------

我们在Linux下进行进行编程，往往都是通过fork出来一个新的程序，fork从化字面意义上理解就是说"分叉", 这其实就意味着我们的fork进程并不是真正从无到有被创建出来的。


一个进程，包括代码、数据和分配给进程的资源，它其实是从现有的进程（父进程）复制出的一个副本（子进程），fork（）函数通过系统调用创建一个与原来进程几乎完全相同的进程，也就是两个进程可以做完全相同的事，然后如果我们通过execve为子进程加载新的应用程序后，那么新的进程将开始执行新的应用


简单来说，<font color = 0x00ffff>新的进程是通过fork和execve创建的，首先通过fork从父进程分叉出一个基本一致的副本，然后通过execve来加载新的应用程序镜像</font>



*	fork生成当前进程的的一个相同副本，该副本成为子进程

>    原进程（父进程）的所有资源都以适当的方法复制给新的进程（子进程）。因此该系统调用之后，原来的进程就有了两个独立的实例，这两个实例的联系包括：同一组打开文件, 同样的工作目录, 进程虚拟空间（内存）中同样的数据（当然两个进程各有一份副本, 也就是说他们的虚拟地址相同, 但是所对应的物理地址不同）等等。

*	execve从一个可执行的二进制程序镜像加载应用程序, 来代替当前运行的进程

>	>	换句话说, 加载了一个新的应用程序。因此execv并不是创建新进程

所以<font color = 0x00ffff>我们在linux要创建一个应用程序的时候，其实执行的操作就是



1.	首先使用fork复制一个旧的进程



2.	然后调用execve在为新的进程加载一个新的应用程序

</font>


##写时复制技术
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

>关于进程创建的
>
>参见 [Linux中fork，vfork和clone详解（区别与联系）](http://blog.csdn.net/gatieme/article/details/51417488)

#内核线程
-------

Linux内核可以看作一个服务进程(管理软硬件资源，响应用户进程的种种合理以及不合理的请求)。内核需要多个执行流并行，为了防止可能的阻塞，多线程化是必要的。

内核线程就是内核的分身，一个分身可以处理一件特定事情。Linux内核使用内核线程来将内核分成几个功能模块，像kswapd、kflushd等，这在处理异步事件如异步IO时特别有用。内核线程的使用是廉价的，唯一使用的资源就是内核栈和上下文切换时保存寄存器的空间。支持多线程的内核叫做多线程内核(Multi-Threads kernel )。内核线程的调度由内核负责，一个内核线程处于阻塞状态时不影响其他的内核线程，因为其是调度的基本单位。这与用户线程是不一样的。
 

内核线程只运行在内核态，不受用户态上下文的拖累。

*	处理器竞争：可以在全系统范围内竞争处理器资源；

*	使用资源：唯一使用的资源是内核栈和上下文切换时保持寄存器的空间

*	调度：调度的开销可能和进程自身差不多昂贵

*	同步效率：资源的同步和数据共享比整个进程的数据同步和共享要低一些。

##内核线程与普通进程的异同
-------

1.	跟普通进程一样，内核线程也有优先级和被调度。
	当和用户进程拥有相同的static_prio 时，内核线程有机会得到更多的cpu资源

2.	内核线程的bug直接影响内核，很容易搞死整个系统, 但是用户进程处在内核的管理下，其bug最严重的情况也只会把自己整崩溃

3.	内核线程没有自己的地址空间，所以它们的"current->mm"都是空的；

4.	内核线程只能在内核空间操作，不能与用户空间交互；

内核线程不需要访问用户空间内存，这是再好不过了。所以内核线程的task_struct的mm域为空

但是刚才说过，内核线程还有核心堆栈，没有mm怎么访问它的核心堆栈呢？这个核心堆栈跟task_struct的thread_info共享8k的空间，所以不用mm描述。

但是内核线程总要访问内核空间的其他内核啊，没有mm域毕竟是不行的。
所以内核线程被调用时, 内核会将其task_strcut的active_mm指向前一个被调度出的进程的mm域, 在需要的时候，内核线程可以使用前一个进程的内存描述符。

因为内核线程不访问用户空间，只操作内核空间内存，而所有进程的内核空间都是一样的。这样就省下了一个mm域的内存。


##内核线程创建
-------

在内核中，有两种方法可以生成内核线程，一种是使用kernel_thread()接口，另一种是用kthread_create()接口

###kernel_thread
-------

先说kernel_thread接口，使用该接口创建的线程，必须在该线程中调用daemonize()函数，这是因为只有当线程的父进程指向"Kthreadd"时，该线程才算是内核线程，而恰好daemonize()函数主要工作便是将该线程的父进程改成“kthreadd"内核线程；默认情况下，调用deamonize()后，会阻塞所有信号，如果想操作某个信号可以调用allow_signal()函数。

```c
int kernel_thread(int (*fn)(void *), void *arg, unsigned long flags); 
            // fn为线程函数，arg为线程函数参数，flags为标记
void daemonize(const char * name,...); // name为内核线程的名称
```
      /* Clone io context */
```

###kthread_create
-------
而kthread_create接口，则是标准的内核线程创建接口，只须调用该接口便可创建内核线程；默认创建的线程是存于不可运行的状态，所以需要在父进程中通过调用wake_up_process()函数来启动该线程。

```c
struct task_struct *kthread_create(int (*threadfn)(void *data),void *data,
                                  const char namefmt[], ...);
 //threadfn为线程函数;data为线程函数参数;namefmt为线程名称，可被格式化的, 类似printk一样传入某种格式的线程名
```

线程创建后，不会马上运行，而是需要将kthread_create() 返回的task_struct指针传给wake_up_process()，然后通过此函数运行线程。

###kthread_run
-------

当然，还有一个创建并启动线程的函数：kthread_run
```c
struct task_struct *kthread_run(int (*threadfn)(void *data),
                                    void *data,
                                    const char *namefmt, ...);
```

线程一旦启动起来后，会一直运行，除非该线程主动调用do_exit函数，或者其他的进程调用kthread_stop函数，结束线程的运行。

int kthread_stop(struct task_struct *thread);
kthread_stop() 通过发送信号给线程。
如果线程函数正在处理一个非常重要的任务，它不会被中断的。当然如果线程函数永远不返回并且不检查信号，它将永远都不会停止。

```c
int wake_up_process(struct task_struct *p); //唤醒线程
struct task_struct *kthread_run(int (*threadfn)(void *data),void *data,
                                const char namefmt[], ...);//是以上两个函数的功能的总和
```

注：因为线程也是进程，所以其结构体也是使用进程的结构体"struct task_struct"。



##内核线程的退出
-------

 当线程执行到函数末尾时会自动调用内核中do_exit()函数来退出或其他线程调用kthread_stop()来指定线程退出。


#总结
-------

Linux使用task_struct来描述进程

1.  一个进程由于其运行空间的不同, 从而有**内核线程**和**用户进程**的区分, 内核线程运行在内核空间, 之所以称之为线程是因为它没有虚拟地址空间, 只能访问内核的代码和数据, 而用户进程则运行在用户空间, 不能直接访问内核的数据但是可以通过中断, 系统调用等方式从用户态陷入内核态，但是内核态只是进程的一种状态, 与内核线程有本质区别

2.  用户进程运行在用户空间上, 而一些通过共享资源实现的一组进程我们称之为线程组, Linux下内核其实本质上没有线程的概念, Linux下线程其实上是与其他进程共享某些资源的进程而已。但是我们习惯上还是称他们为**线程**或者**轻量级进程**

因此, Linux上进程分3种，内核线程（或者叫核心进程）、用户进程、用户线程, 当然如果更严谨的，你也可以认为用户进程和用户线程都是用户进程。

**内核线程拥有 进程描述符、PID、进程正文段、核心堆栈**

**用户进程拥有 进程描述符、PID、进程正文段、核心堆栈 、用户空间的数据段和堆栈**

**用户线程拥有 进程描述符、PID、进程正文段、核心堆栈，同父进程共享用户空间的数据段和堆栈**

>用户线程也可以通过exec函数族拥有自己的用户空间的数据段和堆栈，成为用户进程。