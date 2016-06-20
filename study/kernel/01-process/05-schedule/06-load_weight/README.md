Linux进程负荷权重
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |



前面我们纤细的了解了linux下进程优先级的表示以及其计算的方法, 我们了解到linux针对普通进程和实时进程分别使用静态优先级static_prio和实时优先级rt_priority来指定其默认的优先级别, 然后通过normal_prio函数将他们分别转换为普通优先级normal_prio, 最终换算出动态优先级prio, 动态优先级prio才是内核调度时候有限考虑的优先级字段

但是进程的重要性不仅是由优先级指定的, 而且还需要考虑保存在task_struct->se.load的负荷权重, 

#前景回顾
-------


##进程调度
-------

内存中保存了对每个进程的唯一描述, 并通过若干结构与其他进程连接起来.

**调度器**面对的情形就是这样, 其任务是在程序之间共享CPU时间, 创造并行执行的错觉, 该任务分为两个不同的部分, 其中一个涉及**调度策略**, 另外一个涉及**上下文切换**.


内核必须提供一种方法, 在各个进程之间尽可能公平地共享CPU时间, 而同时又要考虑不同的任务优先级.

调度器的一般原理是, 按所需分配的计算能力, 向系统中每个进程提供最大的公正性, 或者从另外一个角度上说, 他试图确保没有进程被亏待.


##进程的分类
-------

linux把进程区分为实时进程和非实时进程, 其中非实时进程进一步划分为交互式进程和批处理进程

| 类型 | 描述 | 示例 |
| ------- |:-------:|:-------:|:-------:|
| 交互式进程(interactive process) | 此类进程经常与用户进行交互, 因此需要花费很多时间等待键盘和鼠标操作. 当接受了用户的输入后, 进程必须很快被唤醒, 否则用户会感觉系统反应迟钝 | shell, 文本编辑程序和图形应用程序 |
| 批处理进程(batch process) | 此类进程不必与用户交互, 因此经常在后台运行. 因为这样的进程不必很快相应, 因此常受到调度程序的怠慢 | 程序语言的编译程序, 数据库搜索引擎以及科学计算 |
| 实时进程(real-time process) | 这些进程由很强的调度需要, 这样的进程绝不会被低优先级的进程阻塞. 并且他们的响应时间要尽可能的短 | 视频音频应用程序, 机器人控制程序以及从物理传感器上收集数据的程序|


##不同进程采用不同的调度策略
-------

根据进程的不同分类Linux采用不同的调度策略.

对于实时进程，采用FIFO, Round Robin或者Earliest Deadline First (EDF)最早截止期限优先调度算法|的调度策略.

但是普通进程的调度策略就比较麻烦了, 因为普通进程不能简单的只看优先级, 必须公平的占有CPU, 否则很容易出现进程饥饿, 这种情况下用户会感觉操作系统很卡, 响应总是很慢，因此在linux调度器的发展历程中经过了多次重大变动, linux总是希望寻找一个最接近于完美的调度策略来公平快速的调度进程.


##linux调度器的演变
-------

一开始的调度器是复杂度为**$O(n)$的始调度算法**(实际上每次会遍历所有任务，所以复杂度为O(n)), 这个算法的缺点是当内核中有很多任务时，调度器本身就会耗费不少时间，所以，从linux2.5开始引入赫赫有名的**$O(1)$调度器**

然而，linux是集全球很多程序员的聪明才智而发展起来的超级内核，没有最好，只有更好，在$O(1)$调度器风光了没几天就又被另一个更优秀的调度器取代了，它就是**CFS调度器Completely Fair Scheduler**. 这个也是在2.6内核中引入的，具体为2.6.23，即从此版本开始，内核使用CFS作为它的默认调度器，$O(1)$调度器被抛弃了, 其实CFS的发展也是经历了很多阶段，最早期的楼梯算法(SD), 后来逐步对SD算法进行改进出RSDL(Rotating Staircase Deadline Scheduler), 这个算法已经是"完全公平"的雏形了， 直至CFS是最终被内核采纳的调度器, 它从RSDL/SD中吸取了完全公平的思想，不再跟踪进程的睡眠时间，也不再企图区分交互式进程。它将所有的进程都统一对待，这就是公平的含义。CFS的算法和实现都相当简单，众多的测试表明其性能也非常优越



| 字段 | 版本 |
| ------------- |:-------------:|:-------------:|
| O(n)的始调度算法 | linux-0.11~2.4 |
| O(1)调度器 | linux-2.5 |
| CFS调度器 | linux-2.6~至今 |

##Linux的调度器组成
-------

**2个调度器**

可以用两种方法来激活调度

*	一种是直接的, 比如进程打算睡眠或出于其他原因放弃CPU

*	另一种是通过周期性的机制, 以固定的频率运行, 不时的检测是否有必要

因此当前linux的调度程序由两个调度器组成：**主调度器**，**周期性调度器**(两者又统称为**通用调度器(generic scheduler)**或**核心调度器(core scheduler)**)

并且每个调度器包括两个内容：**调度框架**(其实质就是两个函数框架)及**调度器类**

调度器类是实现了不同调度策略的实例，如 CFS、RT class等。

它们的关系如下图

![调度器的组成](./images/level.jpg)

当前的内核支持两种调度器类（sched_setscheduler系统调用可修改进程的策略）：CFS（公平）、RT（实时）；5种调度策略：SCHED_NORAML（最常见的策略）、SCHED_BATCH（除了不能抢占外与常规任务一样，允许任务运行更长时间，更好地使用高速缓存，适合于成批处理的工作）、SCHED_IDLE（它甚至比nice 19还有弱，为了避免优先级反转使用）和SCHED_RR（循环调度，拥有时间片，结束后放在队列末）、SCHED_FIFO（没有时间片，可以运行任意长的时间）；其中前面三种策略使用的是cfs调度器类，后面两种使用rt调度器类。


##优先级的内核表示
-------


内核使用一些简单的数值范围0~139表示内部优先级, 数值越低, 优先级越高。

从0~99的范围专供实时进程使用, nice的值[-20,19]则映射到范围100~139


>实时优先级范围是0到MAX_RT_PRIO-1（即99），而普通进程的静态优先级范围是从MAX_RT_PRIO到MAX_PRIO-1（即100到139）。

| 优先级范围 | 描述 |
| ------------- |:-------------:|
| 0——99 | 实时进程 |
| 100——139 | 非实时进程 |



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

此外还用了一个字段rt_priority保存了实时进程的优先级


| 字段 | 描述 |
| ------------- |:-------------:|
| static_prio | 用于保存静态优先级, 是进程启动时分配的优先级, ，可以通过nice和sched_setscheduler系统调用来进行修改, 否则在进程运行期间会一直保持恒定 |
| prio | 保存进程的动态优先级 |
| normal_prio | 表示基于进程的静态优先级static_prio和调度策略计算出的优先级. 因此即使普通进程和实时进程具有相同的静态优先级, 其普通优先级也是不同的, 进程分叉(fork)时, 子进程会继承父进程的普通优先级 |
| rt_priority | 用于保存实时优先级 |


实时进程的优先级用实时优先级rt_priority来表示



#负荷权重
-------


##符合权重结构struct load_weight
-------

负荷权重用struct load_weight数据结构来表示, 保存着进程权重值weight。其定义在[/include/linux/sched.h, v=4.6, L1195](http://lxr.free-electrons.com/source/include/linux/sched.h?v=4.6#L1195), 如下所示

```c
struct load_weight {
	unsigned long weight;
	u32 inv_weight;
};
```

##调度实体的负荷权重load
-------

既然struct load_weight保存着进程的权重信息, 那么作为进程调度的实体, 必须将这个权重值与特定的进程task_struct, 更一般的与通用的调度实体sched_entity相关联

struct sched_entity作为进程调度的实体信息, 其内置了load_weight结构用于保存当前调度实体的权重, 参照http://lxr.free-electrons.com/source/include/linux/sched.h?v=4.6#L1195

```c
struct sched_entity {
	struct load_weight      load;           /* for load-balancing */
	/*  ......  */
};
```

##进程的符合权重

而进程可以被作为一个调度的实时, 其内部通过存储struct sched_entity se而间接存储了其load_weight信息, 参照http://lxr.free-electrons.com/source/include/linux/sched.h?v=4.6#L1415

```c
struct task_struct
{
	/*  ......  */
	struct sched_entity se;
    /*  ......  */
}
```

因此我们就可以通过task_statuct->se.load获取负荷权重的信息, 而set_load_weight负责根据进程类型及其静态优先级计算符合权重.


#优先级和权重转换表
-------


##优先级->权重转换表
-------


内核不仅维护了负荷权重自身, 还保存另外一个数值, 用于击碎安被负荷权重重除的结果.

一般这个概念是这样的, 进程每降低一个nice值(优先级提升), 则多获得10%的CPU时间, 没升高一个nice值(优先级降低), 则放弃10%的CPU时间.

为执行该策略, 内核需要将优先级转换为权重值, 并提供了一张优先级->权重转换表sched_prio_to_weight

```c
//   http://lxr.free-electrons.com/source/kernel/sched/sched.h?v=4.6#L1132
/*
 * To aid in avoiding the subversion of "niceness" due to uneven distribution
 * of tasks with abnormal "nice" values across CPUs the contribution that
 * each task makes to its run queue's load is weighted according to its
 * scheduling class and "nice" value. For SCHED_NORMAL tasks this is just a
 * scaled version of the new time slice allocation that they receive on time
 * slice expiry etc.
 */

#define WEIGHT_IDLEPRIO                3
#define WMULT_IDLEPRIO         1431655765


extern const int sched_prio_to_weight[40];
extern const u32 sched_prio_to_wmult[40];


// http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L8484
/*
* Nice levels are multiplicative, with a gentle 10% change for every
* nice level changed. I.e. when a CPU-bound task goes from nice 0 to
* nice 1, it will get ~10% less CPU time than another CPU-bound task
* that remained on nice 0.
*
* The "10% effect" is relative and cumulative: from _any_ nice level,
* if you go up 1 level, it's -10% CPU usage, if you go down 1 level
* it's +10% CPU usage. (to achieve that we use a multiplier of 1.25.
* If a task goes up by ~10% and another task goes down by ~10% then
* the relative distance between them is ~25%.)
*/
const int sched_prio_to_weight[40] = {
/* -20 */     88761,     71755,     56483,     46273,     36291,
/* -15 */     29154,     23254,     18705,     14949,     11916,
/* -10 */      9548,      7620,      6100,      4904,      3906,
/*  -5 */      3121,      2501,      1991,      1586,      1277,
/*   0 */      1024,       820,       655,       526,       423,
/*   5 */       335,       272,       215,       172,       137,
/*  10 */       110,        87,        70,        56,        45,
/*  15 */        36,        29,        23,        18,        15,
};


/*
* Inverse (2^32/x) values of the sched_prio_to_weight[] array, precalculated.
*
* In cases where the weight does not change often, we can use the
* precalculated inverse to speed up arithmetics by turning divisions
* into multiplications:
*/
const u32 sched_prio_to_wmult[40] = {
/* -20 */     48388,     59856,     76040,     92818,    118348,
/* -15 */    147320,    184698,    229616,    287308,    360437,
/* -10 */    449829,    563644,    704093,    875809,   1099582,
/*  -5 */   1376151,   1717300,   2157191,   2708050,   3363326,
/*   0 */   4194304,   5237765,   6557202,   8165337,  10153587,
/*   5 */  12820798,  15790321,  19976592,  24970740,  31350126,
/*  10 */  39045157,  49367440,  61356676,  76695844,  95443717,
/*  15 */ 119304647, 148102320, 186737708, 238609294, 286331153,
};
```

对内核使用的范围[-20, 19]中的每个nice级别, sched_prio_to_weight数组都有一个对应项

nice [-20, 19] -=> 下标 [0, 39]

而由于权重`weight` 用`unsigned long` 表示, 因此内核无法直接存储1/weight, 而必须借助于乘法和位移来执行除法的技术.
值, sched_prio_to_wmult数组就存储了这些值, 即sched_prio_to_wmult每个元素的值是$2^{32}/prio_to_weight$每个元素的值.

可以验证$sched\_prio\_to\_wmult[i] = \frac{2^{32}}{sched\_prio\_to\_weight[i]}$



##linux-4.4之前的shced_prio_to_weight和sched_prio_to_wmult
-------

>关于优先级->权重转换表sched_prio_to_weight
>
>在linux-4.4之前的内核中, 优先级->权重转换表用prio_to_weight表示, 定义在[kernel/sched/sched.h, line 1116](http://lxr.free-electrons.com/source/kernel/sched/sched.h?v=4.4#L1116), 与它一同定义的还有prio_to_wmult, 在[kernel/sched/sched.h, line 1139](http://lxr.free-electrons.com/source/kernel/sched/sched.h?v=4.4#L1139)
>均被定义为static const
>
>但是其实这种能够方式不太符合规范的编码风格, 因此常规来说, 我们的头文件中不应该存储结构的定义, 即为了是程序的模块结构更加清晰, 头文件中尽量只包含宏或者声明, 而将具体的定义， 需要分配存储空间的代码放在源文件中
>
>否则如果在头文件中定义全局变量，并且将此全局变量赋初值，那么在多个引用此头文件的C文件中同样存在相同变量名的拷贝，关键是此变量被赋了初值，所以编译器就会将此变量放入DATA段，最终在连接阶段，会在DATA段中存在多个相同的变量，它无法将这些变量统一成一个变量，也就是仅为此变量分配一个空间，而不是多份空间，假定这个变量在头文件没有赋初值，编译器就会将之放入BSS段，连接器会对BSS段的多个同名变量仅分配一个存储空间
>
>因此在新的内核中, 内核黑客们将这两个变量存放在了[kernel/sched/core.c](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L8472), 并加上了sched_前缀, 以表明这些变量是在进程调度的过程中使用的, 而在[kernel/sched/sched.h, line 1144](http://lxr.free-electrons.com/source/kernel/sched/sched.h?v=4.6#L1144)中则只包含了他们的声明.


下面我们列出其对比项

| 内核版本 | 实现 | 地址 |
| ------------- |:-------------:|:-------------:|
| <= linux-4.4 | static const int prio_to_weight[40] |  [kernel/sched/sched.h, line 1116](http://lxr.free-electrons.com/source/kernel/sched/sched.h?v=4.4#L1116) |
| >=linux-4.5 | const int sched_prio_to_weight[40] | 声明在[kernel/sched/sched.h, line 1144](http://lxr.free-electrons.com/source/kernel/sched/sched.h?v=4.6#L1144), 定义在[kernel/sched/core.c](http://lxr.free-electrons.com/source/kernel/sched/core.c?v=4.6#L8472)

其定义并没有发生变化, 依然是一个一对一NICE to WEIGHT的转换表

##1.25的乘积因子
-------

各数组之间的乘积因子是1.25. 要知道为何使用该因子, 可考虑下面的例子

两个进程A和B在nice级别0, 即静态优先级120运行, 因此两个进程的CPU份额相同, 都是50%, nice级别为0的进程, 查其权重表可知是1024. 每个进程的份额是1024/(1024+1024)=0.5, 即50%

如果进程B的优先级+1(优先级降低), 成为nice=1, 那么其CPU份额应该减少10%, 换句话说进程A得到的总的CPU应该是55%, 而进程B应该是45%. 优先级增加1导致权重减少, 即1024/1.25=820, 而进程A仍旧是1024, 则进程A现在将得到的CPU份额是1024/(1024+820=0.55, 而进程B的CPU份额则是820/(1024+820)=0.45. 这样就正好产生了10%的差值.

#

由于权重`weight` 用`unsigned long` 表示, 因此内核无法直接存储1/weight, 而必须借助于乘法和位移来执行除法的技术.

内核用sched_prio_to_wmult存储了用于除法的值.


#进程负荷权重的计算
-------

set_load_weight负责根据进程类型极其静态优先级计算符合权重

执行转换的代码也需要实时进程. 实时进程的权重是普通进程的两倍, 另一方面, SCHED_IDLE进程的权值总是非常小

```c

```



http://blog.chinaunix.net/uid-20671208-id-4909620.html
http://blog.chinaunix.net/uid-20671208-id-4909623.html
http://www.linuxidc.com/Linux/2016-05/131244.htm