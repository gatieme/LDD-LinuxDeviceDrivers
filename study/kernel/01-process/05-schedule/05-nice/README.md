Linux nice系统调用的实现
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |



linux优先级操作的系统调用有nice和getpriority, setpriority
提供的命令有nice和renice

| 函数 | 描述 |
| ------- |:-------:|
| nice | 调整进程的优先级 |
| getpriority | 可用来取得进程、进程组和用户的进程执行优先权 |
| setpriority |  可用来设置进程、进程组和用户的进程执行优先权 |


| 函数 | 描述 |
| ------- |:-------:|
| nice | 可以改变程序执行的优先权等级 |
| renice | 可以改变程序执行的优先权等级, 亦可以指定程序群组或用户名称调整优先权等级，并修改所有隶属于该程序群组或用户的程序的优先权 |





#前言回顾
-------


##优先级的内核表示
-------


**linux优先级概述**

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


**内核的优先级表示**

内核表示优先级的所有信息基本都放在[include/linux/sched/prio.h](http://lxr.free-electrons.com/source/include/linux/sched/prio.h?v=4.6)中, 其中定义了一些表示优先级的宏和函数, 

优先级数值通过宏来定义, 如下所示,

其中MAX_NICE和MIN_NICE定义了nice的最大最小值

而MAX_RT_PRIO指定了实时进程的最大优先级, 而MAX_PRIO则是普通进程的最大优先级数值

```c
/*  http://lxr.free-electrons.com/source/include/linux/sched/prio.h?v=4.6#L4 */
#define MAX_NICE        19
#define MIN_NICE        -20
#define NICE_WIDTH      (MAX_NICE - MIN_NICE + 1)

/* http://lxr.free-electrons.com/source/include/linux/sched/prio.h?v=4.6#L24  */
#define MAX_PRIO        (MAX_RT_PRIO + 40)
#define DEFAULT_PRIO        (MAX_RT_PRIO + 20)
```


| 宏 | 值 | 描述 |
| ------------- |:-------------:|:-------------:|
| MIN_NICE | -20 | 对应于优先级100, 可以使用NICE_TO_PRIO和PRIO_TO_NICE转换 |
| MAX_NICE |  19 | 对应于优先级139, 可以使用NICE_TO_PRIO和PRIO_TO_NICE转换 |
| NICE_WIDTH | 40 | nice值得范围宽度, 即[-20, 19]共40个数字的宽度 |
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

还有一些nice值和rlimit值之间相互转换的函数nice_to_rlimit和rlimit_to_nice, 这在nice系统调用进行检查的时候很有用, 他们定义在[include/linux/sched/prio.h, L47](http://lxr.free-electrons.com/source/include/linux/sched/prio.h#L47)中, 如下所示


```c
/*
 * Convert nice value [19,-20] to rlimit style value [1,40].
 */
static inline long nice_to_rlimit(long nice)
{
    return (MAX_NICE - nice + 1);
}

/*
 * Convert rlimit style value [1,40] to nice value [-20, 19].
 */
static inline long rlimit_to_nice(long prio)
{
    return (MAX_NICE - prio + 1);
}
```

**DEF最早截至时间优先实时调度算法的优先级描述**

此外新版本的内核还引入了EDF实时调度算法, 它的优先级比RT进程和NORMAL/BATCH进程的优先级都要高, 关于EDF的优先级的设置信息都早内核头文件[include/linux/sched/deadline.h](http://lxr.free-electrons.com/source/include/linux/sched/deadline.h?v=4.6#L10)


因此内核将MAX_DL_PRIO设置为0, 可以参见内核文件[include/linux/sched/deadline.h](http://lxr.free-electrons.com/source/include/linux/sched/deadline.h?v=4.6#L10)

```c
#define MAX_DL_PRIO             0
```

此外也提供了一些EDF优先级处理所需的函数, 如下所示, 可以参见内核文件[include/linux/sched/deadline.h](http://lxr.free-electrons.com/source/include/linux/sched/deadline.h?v=4.6#L12)

```c
static inline int dl_prio(int prio)
{
    if (unlikely(prio < MAX_DL_PRIO))
            return 1;
    return 0;
}

static inline int dl_task(struct task_struct *p)
{
    return dl_prio(p->prio);
}

static inline bool dl_time_before(u64 a, u64 b)
{
    return (s64)(a - b) < 0;
}
````

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


#nice系统调用的实现
-------

##系统调用号和入口函数
-------

| 系统调用号 | 入口函数声明 | 具体实现 |
| ------- |:-------:|
| [34](http://lxr.free-electrons.com/source/arch/arm64/include/asm/unistd32.h?v=4.6#L93) | [sys_nice](http://lxr.free-electrons.com/source/include/linux/syscalls.h?v=4.6#L287) | [SYSCALL_DEFINE1(nice, int, increment)](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L3560)

**nice系统调用号**

```c
//  http://lxr.free-electrons.com/source/arch/arm64/include/asm/unistd32.h?v=4.6#L93
#define __NR_nice 34
__SYSCALL(__NR_nice, sys_nice)
```

**nice系统调用入口函数sys_nice**

```c
//  http://lxr.free-electrons.com/source/include/linux/syscalls.h?v=4.6#L287
asmlinkage long sys_nice(int increment);
```

**nice系统调用的实现**

其具体实现在[kernel/sched/core.c](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L3560)中

##sys_nice实现
-------

```c
/*
 * sys_nice - change the priority of the current process.
 * @increment: priority increment
 *
 * sys_setpriority is a more generic, but much slower function that
 * does similar things.
 */
SYSCALL_DEFINE1(nice, int, increment)
{
    long nice, retval;

    /*
     * Setpriority might change our priority at the same moment.
     * We don't have to worry. Conceptually one call occurs first
     * and we have a single winner.
     */
    increment = clamp(increment, -NICE_WIDTH, NICE_WIDTH);
    nice = task_nice(current) + increment;

    nice = clamp_val(nice, MIN_NICE, MAX_NICE);
    if (increment < 0 && !can_nice(current, nice))
            return -EPERM;

    retval = security_task_setnice(current, nice);
    if (retval)
            return retval;

    set_user_nice(current, nice);
    return 0;
}
```

##clamp
------


clamp宏定义在[include/linux/kernel.h, L757](http://lxr.free-electrons.com/source/include/linux/kernel.h?v=4.6#L757), 其定义如下

```c
/**
* clamp - return a value clamped to a given range with strict typechecking
* @val: current value
* @lo: lowest allowable value
* @hi: highest allowable value
*
* This macro does strict typechecking of lo/hi to make sure they are of the
* same type as val.  See the unnecessary pointer comparisons.

    先求val和lo的最大值, 再求与hi的最小值
    首先这里的参数要求lo < hi
    *   如果val在[low,high]范围内, 即lo < val < hi, 则返回val
    *   如果val小于lo, 即val < lo < hi, 则返回lo
    *   如果val大于hi, 即val > hi > lo, 则返回hi
*/
#define clamp(val, lo, hi) min((typeof(val))max(val, lo), hi)
```

其中min和max是linux内核自己实现的函数宏, linux内核不能依赖于任何库, 因此必须实现一些基本的函数, 其中就包括最小值最大值以及字符串处理的一些函数.

其中min和max函数定义在[linux-4.6/include/linux/kernel.h, L727](http://lxr.free-electrons.com/source/include/linux/kernel.h?v=4.6#L727)

```c
/*
 * min()/max()/clamp() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x, y) ({                            \
    typeof(x) _min1 = (x);                  \
    typeof(y) _min2 = (y);                  \
    (void) (&_min1 == &_min2);              \
    _min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({                            \
    typeof(x) _max1 = (x);                  \
    typeof(y) _max2 = (y);                  \
    (void) (&_max1 == &_max2);              \
    _max1 > _max2 ? _max1 : _max2; })
```

其中typeof关键字是C语言中的一个新扩展, 用于获取变量或者表达式的类型, 这个特性在linux内核中应用非常广泛. typeof的参数可以是两种形式：表达式或类型

因此min和max这两个函数就通过简单的转换实现了求最小最大值的函数

我们回过去看clamp(val, lo, hi). 这个函数会检查val是不是在[lo, hi]的范围内, 参数要求lo < hi, 这个函数会先求val和lo的最大值, 再求与hi的最小值

*	如果val在[low,high]范围内, 即lo < val < hi, 则返回val

*   如果val小于lo, 即val < lo < hi, 则返回lo

*   如果val大于hi, 即val > hi > lo, 则返回hi


##task_nice
-------

[task_nice](http://lxr.free-electrons.com/source/include/linux/sched.h?=4.6#L2409)定义在[include/linux/sched.h, L2409](http://lxr.free-electrons.com/source/include/linux/sched.h?=4.6#L2409), 如下所示

其实就是将进程的静态优先级static_prio通过PRIO_TO_NICE转换成nice值

```c
/**
 * task_nice - return the nice value of a given task.
 * @p: the task in question.
 *
 * Return: The nice value [ -20 ... 0 ... 19 ].
 */
static inline int task_nice(const struct task_struct *p)
{
    return PRIO_TO_NICE((p)->static_prio);
}
```


##clamp_val
-------

clamp_val宏定义在[include/linux/kernel.h, L757](http://lxr.free-electrons.com/source/include/linux/kernel.h?v=4.6#L768), 其定义如下


```c
/*
 * ..and if you can't take the strict
 * types, you can specify one yourself.
 *
 * Or not use min/max/clamp at all, of course.
 */
#define min_t(type, x, y) ({                    \
    type __min1 = (x);                      \
    type __min2 = (y);                      \
    __min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({                    \
    type __max1 = (x);                      \
    type __max2 = (y);                      \
    __max1 > __max2 ? __max1: __max2; })

/**
 * clamp_t - return a value clamped to a given range using a given type
 * @type: the type of variable to use
 * @val: current value
 * @lo: minimum allowable value
 * @hi: maximum allowable value
 *
 * This macro does no typechecking and uses temporary variables of type
 * 'type' to make all the comparisons.
 */
#define clamp_t(type, val, lo, hi) min_t(type, max_t(type, val, lo), hi)

/**
 * clamp_val - return a value clamped to a given range using val's type
 * @val: current value
 * @lo: minimum allowable value
 * @hi: maximum allowable value
 *
 * This macro does no typechecking and uses temporary variables of whatever
 * type the input argument 'val' is.  This is useful when val is an unsigned
 * type and min and max are literals that will otherwise be assigned a signed
 * integer type.
 */
#define clamp_val(val, lo, hi) clamp_t(typeof(val), val, lo, hi)
```

##can_nice
-------

can_nice可检查有没有nice的权限, 该函数定义在[kernel/sched/core.c, #L3544](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L3544)中, 如下所示

```c
/*
 * can_nice - check if a task can reduce its nice value
 * @p: task
 * @nice: nice value
 */
int can_nice(const struct task_struct *p, const int nice)
{
    /* convert nice value [19,-20] to rlimit style value [1,40] */
    int nice_rlim = nice_to_rlimit(nice);

    return (nice_rlim <= task_rlimit(p, RLIMIT_NICE) ||
            capable(CAP_SYS_NICE));
}
```

##security_task_setnice
-------

##set_user_nice
-------



```c
//  http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L3497
```



http://blog.sina.com.cn/s/blog_9ca3f6e70102wkwp.html
http://lxr.free-electrons.com/source/kernel/sched/core.c#L3527
http://blog.chinaunix.net/uid-20671208-id-4909620.html
http://blog.chinaunix.net/uid-20671208-id-4909623.html
http://www.linuxidc.com/Linux/2016-05/131244.htm