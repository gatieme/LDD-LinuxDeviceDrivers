Linux nice系统调用的实现
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |

#前言回顾
-------

##优先级的内核表示
-------

>在用户空间通过nice命令设置进程的静态优先级, 这在内部会调用nice系统调用, 进程的nice值在-20~+19之间. 值越低优先级越高.
>
>setpriority系统调用也可以用来设置进程的优先级. 它不仅能够修改单个线程的优先级, 还能修改进程组中所有进程的优先级, 或者通过制定UID来修改特定用户的所有进程的优先级


内核使用一些简单的数值范围0~139表示内部优先级, 数值越低, 优先级越高。

从0~99的范围专供实时进程使用, nice的值[-20,19]则映射到范围100~139


>linux2.6内核将任务优先级进行了一个划分, 实时优先级范围是0到MAX_RT_PRIO-1（即99），而普通进程的静态优先级范围是从MAX_RT_PRIO到MAX_PRIO-1（即100到139）。

| 优先级范围 | 描述 |
| ------------- |:-------------:|
| 0——99 | 实时进程 |
| 100——139 | 非实时进程 |

![内核的优先级标度](./images/priority.jpg)

优先级数值通过宏来定义, 如下所示, 其中MAX_RT_PRIO指定了实时进程的最大优先级, 而MAX_PRIO则是普通进程的最大优先级数值

```c
/*  http://lxr.free-electrons.com/source/include/linux/sched/prio.h?v=4.6#L21  */
#define MAX_USER_RT_PRIO    100
#define MAX_RT_PRIO     MAX_USER_RT_PRIO

/* http://lxr.free-electrons.com/source/include/linux/sched/prio.h?v=4.6#L24  */
#define MAX_PRIO        (MAX_RT_PRIO + 40)
#define DEFAULT_PRIO        (MAX_RT_PRIO + 20)
```

此外新版本的内核还引入了EDF实时调度算法, 它的优先级比RT进程和NORMAL/BATCH进程的优先级都要高, 因此内核将MAX_DL_PRIO设置为0, 可以参见内核文件[include/linux/sched/deadline.h](http://lxr.free-electrons.com/source/include/linux/sched/deadline.h)

```c
#define MAX_DL_PRIO             0
````

| 宏 | 值 | 描述 |
| ------------- |:-------------:|
| MAX_RT_PRIO, MAX_USER_RT_PRIO | 100 | 实时进程的最大优先级 |
| MAX_PRIO | 140 | 普通进程的最大优先级 |
| DEFAULT_PRIO | 120 | 进程的默认优先级, 对应于nice=0 |
| MAX_DL_PRIO | 0 | 使用EDF最早截止时间优先调度算法的实时进程最大的优先级 |

而内核提供了一组宏将优先级在各种不同的表示形之间转移

```c
//  http://lxr.free-electrons.com/source/include/linux/sched/prio.h?v=4.6#L27
/*
 * Convert user-nice values [ -20 ... 0 ... 19 ]
 * to static priority [ MAX_RT_PRIO..MAX_PRIO-1 ],
 * and back.
 */
#define NICE_TO_PRIO(nice)      ((nice) + DEFAULT_PRIO)
#define PRIO_TO_NICE(prio)      ((prio) - DEFAULT_PRIO)

/*
 * 'User priority' is the nice value converted to something we
 * can work with better when scaling various scheduler parameters,
 * it's a [ 0 ... 39 ] range.
 */
#define USER_PRIO(p)            ((p)-MAX_RT_PRIO)
#define TASK_USER_PRIO(p)       USER_PRIO((p)->static_prio)
#define MAX_USER_PRIO           (USER_PRIO(MAX_PRIO))
```

此外也提供了一些EDF调度算法的函数, 如下所示, 可以参见内核文件[include/linux/sched/deadline.h](http://lxr.free-electrons.com/source/include/linux/sched/deadline.h)


##进程的优先级表示
-------


```c
struct task_struct
{
    /* 进程优先级
     * prio: 动态优先级，范围为100~139，与静态优先级和补偿(bonus)有关
     * static_prio: 静态优先级，static_prio = 100 + nice + 20 (nice值为-20~19,所以static_prio值为100~139)
     * normal_prio: 没有受优先级继承影响的常规优先级，具体见normal_prio函数，跟属于什么类型的进程有关
     */
    int prio, static_prio, normal_prio;
    /* 实时进程优先级 */
    unsigned int rt_priority;
}
```


**动态优先级 静态优先级 实时优先级**


其中task_struct采用了三个成员表示进程的优先级:prio和normal_prio表示动态优先级, static_prio表示进程的静态优先级.

>为什么表示动态优先级需要两个值prio和normal_prio
>
>调度器会考虑的优先级则保存在prio. 由于在某些情况下内核需要暂时提高进程的优先级, 因此需要用prio表示. 由于这些改变不是持久的, 因此静态优先级static_prio和普通优先级normal_prio不受影响.

此外还用了一个字段rt_priority保存了实时进程的优先级

| 字段 | 描述 |
| ------------- |:-------------:|
| static_prio | 用于保存静态优先级, 是进程启动时分配的优先级, ，可以通过nice和sched_setscheduler系统调用来进行修改, 否则在进程运行期间会一直保持恒定 |
| prio | 保存进程的动态优先级 |
| normal_prio | 表示基于进程的静态优先级static_prio和调度策略计算出的优先级. 因此即使普通进程和实时进程具有相同的静态优先级, 其普通优先级也是不同的, 进程分叉(fork)时, 子进程会继承父进程的普通优先级 |
| rt_priority | 用于保存实时优先级 |


实时进程的优先级用实时优先级rt_priority来表示


#nice系统调用
-------

nice系统调用用来改变进程的静态优先级

##函数原型
-------

```c
#include <unistd.h>
int nice(int inc);
```

##功能描述
-------
改变进程优先级。在调用进程的nice值上添加参数指定的值。较高的nice值意味值较低的优先级，只有超级用户才可指定负增量--即提升优先级

nice的取值范围可参考getpriority的描述


##返回说明
-------

成功执行时，返回新的nice值。失败返回-1，errno被设为以下值
EPERM：调用者试着提高其优先级，但权能不足。


| 字段 | 描述 |
| ------------- |:-------------:|
| EPERM | 调用进程试图通过提供一个负值, 增加其优先但没有足够特权。Linux下的 cap_sys_nice 能力。可以通过setrlimit的rlimit_nice查看限制 |


另请参阅
nice (1)

fork (2)

getpriority (2)

setpriority (2)

renice (8)

http://blog.sina.com.cn/s/blog_9ca3f6e70102wkwp.html
http://lxr.free-electrons.com/source/kernel/sched/core.c#L3527
http://blog.chinaunix.net/uid-20671208-id-4909620.html
http://blog.chinaunix.net/uid-20671208-id-4909623.html
http://www.linuxidc.com/Linux/2016-05/131244.htm