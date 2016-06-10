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


##do_exit
-------

##do_exit_group
-------
http://blog.csdn.net/npy_lp/article/details/7402305
http://www.myexception.cn/linux-unix/1867899.html
http://blog.csdn.net/goodlixueyong/article/details/6411810



do_group_exit()函数杀死属于current线程组的所有进程。它接受进程终止代码作为参数，进程终止代号可能是系统调用exit_group()指定的一个值，也可能是内核提供的一个错误代号。该函数执行下述操作：
检查退出进程的SIGNAL＿GROUP_EXIT标志是否不为0，如果不为0，说明内核已经开始为线性组执行退出的过程。在这种情况下，就把存放在current->signal->group_exit_code的值当作退出码，然后跳转到第4步。
否则，设置进程的SIGNAL＿GROUP_EXIT标志并把终止代号放到current->signal->group_exit_code字段。
调用zap_other_threads()函数杀死current线程组中的其它进程。为了完成这个步骤，函数扫描与current->tgid对应的PIDTYPE＿TGID类型的散列表中的每PID链表，向表中所有不同于current的进程发送SIGKILL信号，结果，所有这样的进程都将执行do_exit()函数，从而被杀死。
调用do_exit()函数，把进程的终止代码传递给它。正如我们将在下面看到的，do_exit()杀死进程而且不再返回。
