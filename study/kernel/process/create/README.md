| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------------- |:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|
| 2016-05-12 | [Linux-4.5](http://lxr.free-electrons.com/source/?v=4.5) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux-进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |




Unix标准的复制进程的系统调用时fork（即分叉），但是Linux，BSD等操作系统并不止实现这一个，确切的说linux实现了三个，fork,vfork,clone（确切说vfork创造出来的是轻量级进程，也叫线程，是共享资源的进程）



| 系统调用 | 描述 |
|:-------------:|:-------------:|
| fork | fork创造的子进程是父进程的完整副本，复制了父亲进程的资源，包括内存的内容task_struct内容 |
| vfork | vfork创建的子进程与父进程共享数据段,而且由vfork()创建的子进程将先于父进程运行 |
| clone | Linux上创建线程一般使用的是pthread库  实际上linux也给我们提供了创建线程的系统调用，就是clone |


>关于fork, vfork和clone的用户空间的详细知识，请参见
>
>[Linux中fork，vfork和clone详解（区别与联系）](http://blog.csdn.net/gatieme/article/details/51417488)
>
>在那里我们详细讲解了，linux系统编程创建进程的方法，写时复制技术，vfork和clone的由来，但是并未涉及内核中的实现机制，
>
>但是在这里，我们会着重关注内核中是如何实现这个三个系统调用来创建进程，或者是从原有的进程中复制出一个新的进程

#为什么叫进程复制
-------

为什么我们老是把linux进程的创建叫做进程的复制呢?



#系统调用
-------

fork, vfork和clone系统调用的入口函数分别是sys_fork, sys_vfork, sys_clone函数。

其定义依赖于具体的体系结构, 因为在用户空间和内核空间之间传递参数的方法因体系结构而异, 但是他们的完成的工作却是基本相同，他们的任务就是从处理器寄存器中提取到由用户空间提供的信息，然后调用与体系结构无关的do_fork函数, 后者才是进程的真正创建者，完成了进程复制的所有工作


