调度子系统(scheduling)
=====================


# 0 调度子系统(开篇)
-------


# 0.1 调度子系统概述
-------


**概述: **Linux 是一个遵循 POSIX 标准的类 Unix 操作系统(然而它并不是 Unix 系统[<sup>1</sup>](#refer-anchor-1)), POSIX 1003.1b 定义了调度相关的一个功能集合和 API 接口[<sup>2</sup>](#refer-anchor-2). 调度器的任务是分配 CPU 运算资源, 并以协调效率和公平为目的. **效率**可从两方面考虑: 1) 吞吐量(throughput) 2)延时(latency). 不做精确定义, 这两个有相互矛盾的衡量标准主要体现为两大类进程: 一是 CPU 密集型, 少量 IO 操作, 少量或无与用户交互操作的任务(强调吞吐量, 对延时不敏感, 如高性能计算任务 HPC), 另一则是 IO 密集型, 大量与用户交互操作的任务(强调低延时, 对吞吐量无要求, 如桌面程序). **公平**在于有区分度的公平, 多媒体任务和数值计算任务对延时和限定性的完成时间的敏感度显然是不同的.
为此,  POSIX 规定了操作系统必须实现以下**调度策略(scheduling policies),** 以针对上述任务进行区分调度:

**- SCHED\_FIFO**

**- SCHED\_RR**

这两个调度策略定义了对实时任务, 即对延时和限定性的完成时间的高敏感度的任务. 前者提

供 FIFO 语义, 相同优先级的任务先到先服务, 高优先级的任务可以抢占低优先级的任务; 后 者提供 Round-Robin 语义, 采用时间片, 相同优先级的任务当用完时间片会被放到队列尾

部, 以保证公平性, 同样, 高优先级的任务可以抢占低优先级的任务. 不同要求的实时任务可

以根据需要用 **_sched\_setscheduler()_** API 设置策略.
**- SCHED\_OTHER**

此调度策略包含除上述实时进程之外的其他进程, 亦称普通进程. 采用分时策略, 根据动态优

先级(可用 **nice()** API设置), 分配 CPU 运算资源.  **注意: 这类进程比上述两类实时进程优先级低, 换言之, 在有实时进程存在时, 实时进程优先调度**.

Linux 除了实现上述策略, 还额外支持以下策略:

- **SCHED\_IDLE** 优先级最低, **在系统空闲时才跑这类进程**(如利用闲散计算机资源跑地外文明搜索, 蛋白质结构分析等任务, 是此调度策略的适用者)

- **SCHED\_BATCH** 是 SCHED\_OTHER 策略的分化, 与 SCHED\_OTHER 策略一样, 但针对吞吐量优化

- **SCHED\_DEADLINE** 是新支持的实时进程调度策略, 针对突发型计算, 且对延迟和完成时间高度敏感的任务适用.


除了完成以上基本任务外, Linux 调度器还应提供高性能保障, 对吞吐量和延时的均衡要有好的优化; 要提供高可扩展性(scalability)保障, 保障上千节点的性能稳定; 对于广泛作为服务器领域操作系统来说, 它还提供丰富的组策略调度和节能调度的支持.


## 0.2 主线内存管理分支合并窗口
-------

Mainline Merge Window

- [x] Merge tag 'sched-core-date'

- [x] Merge tag 'sched-urgent-date'

| 版本 | 发布时间 | 合并链接 |
|:---:|:-------:|:-------:|
| 5.13 | 2021/06/28 | [Merge tag 'sched-core-2021-04-28', 5.13-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=16b3d0cf5bad844daaf436ad2e9061de0fe36e5c)<br>[Merge tag 'sched-urgent-2021-05-09', 5.13-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9819f682e48c6a7055c5d7a6746411dd3969b0e5)<br>[Merge tag 'sched-urgent-2021-05-15', 5.13-rc2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c12a29ed9094b4b9cde8965c12850460b9a79d7c)<br>[Merge tag 'sched-urgent-2021-06-12', 5.13-rc6](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=99f925947ab0fd5c17b74460d8b32f1aa1c86e3a)<br>[Merge tag 'sched_urgent_for_v5.13_rc6', 5.13-rc7, 2021/06/20](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=cba5e97280f53ec7feb656fcdf0ec00a5c6dd539)<br>[Merge tag 'sched-urgent-2021-06-24', 5.13](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=666751701b6e4b6b6ebc82186434806fa8a09cf3)<br> |
| 5.14 | 2021/08/29 | [Merge tag 'sched-core-2021-06-28', 5.14-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=54a728dc5e4feb0a9278ad62b19f34ad21ed0ee4)<br>[Merge tag 'sched-urgent-2021-06-30', 5.14-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a6eaf3850cb171c328a8b0db6d3c79286a1eba9d)<br>[Merge tag 'sched-urgent-2021-07-11', 5.14-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=877029d9216dcc842f50d37571f318cd17a30a2d)<br>[Merge tag 'sched-urgent-2021-08-08', 5.14-rc5](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=713f0f37e8128e8a0190a98f5a4be71fb32a671a)<br>[Merge tag 'sched_urgent_for_v5.14', 5.14](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=537b57bd5a202af145c266d4773971c2c9f90cd9) |
| 5.15 | NA | [Merge tag 'sched-core-2021-08-30'](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5d3c0db4598c5de511824649df2aa976259cf10a)

## 0.3 社区会议
-------


### 0.3.1 Linux Plumbers Conference
-------

Scheduler Microconference Accepted into Linux Plumbers Conference

`2010~2017` 年的内容, 可以在 [wiki](http://wiki.linuxplumbersconf.org/?do=search&id=scheduler) 检索.

| 日期 | 官网 | LKML | LWN |
|:---:|:----:|:----:|:---:|
| [2021](https://linuxplumbersconf.org/event/11/page/104-accepted-microconferences) | [LPC 2021 - Overview](https://www.linuxplumbersconf.org/event/11/page/103-lpc-2021-overview) | [[ANNOUNCE] CFP: Scheduler Microconference - Linux Plumbers Conference 2021](https://lkml.org/lkml/2021/8/9/718) | [A look forward to Linux Plumbers 2021](https://lwn.net/Articles/864016) |
| [2020](https://linuxplumbersconf.org/event/7/page/44-microconferences) | [Scheduler Microconference 2020](https://www.linuxplumbersconf.org/blog/2020/scheduler-microconference-accepted-into-2020-linux-plumbers-conference) | NA | NA |
| [2019](https://linuxplumbersconf.org/event/4/page/38-microconferences) | [Scheduler Microconference 2019](https://www.linuxplumbersconf.org/blog/2019/scheduler-microconference-accepted-into-2019-linux-plumbers-conference) | NA | NA |
| [2018](https://www.linuxplumbersconf.org/2018) | [Power Management and Energy-awareness Microconference 2018](https://blog.linuxplumbersconf.org/2018/power-management-mc)<br>[timeline](https://www.linuxplumbersconf.org/event/2/timetable/?view=lpc) | NA | [Notes from the LPC scheduler microconference](https://lwn.net/Articles/734039) |
| [2017](https://blog.linuxplumbersconf.org/2017) | [Scheduler Workloads Microconference 2017](http://wiki.linuxplumbersconf.org/2017:scheduler_workloads) | | [Notes from the LPC scheduler microconference](https://lwn.net/Articles/734039)


## 0.4 社区几个调度的大神
-------

- [x] [Mel Gorman mgorman@techsingularity.net](https://lore.kernel.org/patchwork/project/lkml/list/?submitter=19167)

- [x] [Alex Shi](https://lore.kernel.org/patchwork/project/lkml/list/?submitter=25695&state=%2A&series=&q=&delegate=&archive=both)

- [x] [Valentin Schneider](https://lore.kernel.org/patchwork/project/lkml/list/?series=&submitter=23332&state=*&q=&archive=both&delegate=)

- [x] [Vincent Guittot](https://lore.kernel.org/patchwork/project/lkml/list/?submitter=11990&archive=both&state=*)



## 0.5 目录
-------

- [x] 1. 进程调度类

- [x] 2. 组调度支持(Group Scheduling)

- [x] 3. 负载跟踪机制

- [x] 4. 基于调度域的负载均衡

- [x] 5. SELECT_TASK_RQ

- [x] 6. PICK_NEXT_TASK

- [x] 7. 调度与节能

- [x] 8. 实时性 linux PREEMPT_RT

- [x] 9. 其他

- [x] 10. 调试信息


# 1 进程调度类
-------

## 1.1 普通进程调度器(SCHED\_NORMAL)之纠极进化史
-------

Linux 一开始, 普通进程和实时进程都是基于优先级的一个调度器, 实时进程支持 100 个优先级, 普通进程是优先级小于实时进程的一个静态优先级, 所有普通进程创建时都是默认此优先级, 但可通过 **nice()** 接口调整动态优先级(共40个). 实时进程的调度器比较简单, 而普通进程的调度器, 则历经变迁[<sup>5</sup>](#refer-anchor-5):



### 1.1.1 O(1) 调度器:
-------

2.6 时代开始支持(2002年引入).

顾名思义, 此调度器为O(1)时间复杂度. 该调度器修正之前的O(n) 时间复杂度调度器, 以解决扩展性问题. 为每一个动态优先级维护队列, 从而能在常数时间内选举下一个进程来执行.


### 1.1.2 夭折的 RSDL(The Rotating Staircase Deadline Scheduler)调度器
-------

**2007 年 4 月提出, 预期进入 2.6.22, 后夭折.**



O(1) 调度器存在一个比较严重的问题: 复杂的交互进程识别启发式算法 - 为了识别交互性的和批处理型的两大类进程, 该启发式算法融入了睡眠时间作为考量的标准, 但对于一些特殊的情况, 经常判断不准, 而且是改完一种情况又发现一种情况.


Con Kolivas (八卦: 这家伙白天是个麻醉医生)为解决这个问题提出 **RSDL(The Rotating Staircase Deadline Scheduler)** 算法. 该算法的亮点是对公平概念的重新思考: **交互式(A)**和**批量式(B)**进程应该是被完全公平对待的, 对于两个动态优先级完全一样的 A, B 进程, **它们应该被同等地对待, 至于它们是交互式与否(交互式的应该被更快调度),　应该从他们对分配给他们的时间片的使用自然地表现出来, 而不是应该由调度器自作高明地根据他们的睡眠时间去猜测**. 这个算法的核心是**Rotating Staircase**, 是一种衰减式的优先级调整, 不同进程的时间片使用方式不同, 会让它们以不同的速率衰减(在优先级队列数组中一级一级下降, 这是下楼梯这名字的由来), 从而自然地区分开来进程是交互式的(间歇性的少量使用时间片)和批量式的(密集的使用时间片). 具体算法细节可看这篇文章: [The Rotating Staircase Deadline Scheduler [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/224865/)



### 1.1.3 完全公平的调度器(CFS)
-------

**2.6.23(2007年10月发布)**

Con Kolivas 的完全公平的想法启发了原 O(1) 调度器作者 Ingo Molnar, 他重新实现了一个新的调度器, 叫 CFS(Completely Fair Scheduler). 它从 RSDL/SD 中吸取了完全公平的思想, 不再跟踪进程的睡眠时间, 也不再企图区分交互式进程. 它将所有的进程都统一对待, 这就是公平的含义. CFS的算法和实现都相当简单, 众多的测试表明其性能也非常优越.

> 新的 CFS 调度器的核心同样是**完全公平性**, 即平等地看待所有普通进程, 让它们自身行为彼此区分开来, 从而指导调度器进行下一个执行进程的选举.


具体说来, 此算法基于一个理想模型. 想像你有一台无限个 相同计算力的 CPU, 那么完全公平很容易, 每个 CPU 上跑一个进程即可. 但是, 现实的机器 CPU 个数是有限的, 超过 CPU 个数的进程数不可能完全同时运行. 因此, 算法为每个进程维护一个理想的运行时间, 及实际的运行时间, 这两个时间差值大的, 说明受到了不公平待遇, 更应得到执行.


至于这种算法如何区分交互式进程和批量式进程, 很简单. 交互式的进程大部分时间在睡眠, 因此它的实际运行时间很小, 而理想运行时间是随着时间的前进而增加的, 所以这两个时间的差值会变大. 与之相反, 批量式进程大部分时间在运行, 它的实际运行时间和理想运行时间的差距就较小. 因此, 这两种进程被区分开来.


CFS的算法和实现都相当简单, 众多的测试表明其性能也非常优越. 并得到更多的开发者支持, 所以它最终替代了 RSDL 在 2.6.23 进入内核, 一直使用到现在.


### 1.1.4 CK 的 BFS 和 MuQSS
-------

可以八卦的是, Con Kolivas (因此)离开了社区, 不过他本人否认是因为此事而心生龃龉. 后来, 2009 年, 他对越来越庞杂的 CFS 不满意, 认为 CFS 过分注重对大规模机器, 而大部分人都是使用少 CPU 的小机器, 因此于 2009年8月31日发布了 BFS 调度器(Brain Fuck Scheduler)[<sup>48</sup>](#refer-anchor-48).

BFS调度器的原理十分简单, 是为桌面交互式应用专门设计, 使得用户的桌面环境更为流畅, 早期使用CFS编译内核时, 音讯视讯同时出现会出现严重的停顿(delay), 而使用 BFS 则没有这些问题. 【注意】

BFS 的原理是将所有行程被安排到103组伫列(queue)之中. BFS本身是O(n)调度器, 但大部份的时间比目前Linux上拥有O(log n)效能的主流调度器CFS还优异. [2]Con Kolivas 并没有打算将BFS应用在 mainline Linux[3]. 他再度以 -ck 的补丁来维护这套原始码.

Android 曾经在试验性的分支, 使用 BFS 作为其操作系统排程器. 但是经过测试发现对使用者并没有明显的改进, 因此并未合入之后发表的正式版本. 初次之外当时很多人和厂商都做过 BFS 和 CFS 的对比测试.

[Linux 调度器 BFS 简介](https://www.ibm.com/developerworks/cn/linux/l-cn-bfs)

[BFS vs. mainline scheduler benchmarks and measurements](https://lwn.net/Articles/351058/)

[BFS vs. CFS - Scheduler Comparison](https://www.cs.unm.edu/~eschulte/classes/cs587/data/bfs-v-cfs_groves-knockel-schulte.pdf)

[bfs-faq](http://ck.kolivas.org/patches/bfs/bfs-faq.txt)

https://lkml.org/lkml/2007/8/30/307
https://www.wikiwand.com/zh-hans/腦殘排程器


BFS 的最后版本是 2016 年 12 月发布的 v0.512, 基于 v4.8 内核.

之后 CK 发布了更现代化的 MuQSS(多队列跳过列表调度程序) [The MuQSS CPU scheduler](https://lwn.net/Articles/720227), CK 称之为原始 BFS 调度程序基于 per-CPU 运行队列改进版. 截止目前 MuQSS 都在不断维护.


### 1.1.5 批处理进程 SCHED\_BATCH
-------

从 Linux 2.6.16 开始, SCHED\_BATCH 可以用于静态优先级为 0 的线程. 该策略类似 SCHED_NORMAL, 并根据动态优先级(nice值)进行调度. 区别是使用该策略时, 调度器会假设线程是 CPU 密集型的, 因此, 该调度器会根据线程的唤醒行为施加调度惩罚, 因此这种调度策略比较不受欢迎.

该策略比较适用于非交互且不期望降低 nice 值的负载, 以及需要不因为交互而(在负载之间)造成额外抢占的调度策略的负载

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2006/01/14 | Ingo Molnar <mingo@elte.hu> | [b0a9499c3dd5 sched: add new SCHED_BATCH policy](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b0a9499c3dd5) | NA | v1 ☑ 2.6.24-rc1 | [PATCH HISTORY](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b0a9499c3dd5) |
| 2007/12/04 | Ingo Molnar <mingo@elte.hu> | [8ca0e14ffb12 sched: disable sleeper_fairness on SCHED_BATCH](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8ca0e14ffb12) | NA | v1 ☑ 2.6.24-rc1 | [PATCH HISTORY](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8ca0e14ffb12) |
| 2007/12/04 | Ingo Molnar <mingo@elte.hu> | [91c234b4e341 sched: do not wakeup-preempt with SCHED_BATCH tasks](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=91c234b4e341) | 不要用 SCHED\_BATCH 任务唤醒时抢占其他进程, 它们的抢占由 tick 驱动. | v1 ☑ 2.6.24-rc5 | [PATCH HISTORY](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=91c234b4e341) |
| 2007/12/04 | Ingo Molnar <mingo@elte.hu> | [db292ca302e8 sched: default to more agressive yield for SCHED_BATCH tasks](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=db292ca302e8) | 对 SCHED\_BATCH 调优的任务执行更积极的 yield. | v1 ☑ 2.6.24-rc5 | [PATCH HISTORY](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=db292ca302e8) |
| 2007/12/18 | Ingo Molnar <mingo@elte.hu> | [6cbf1c126cf6 sched: do not hurt SCHED_BATCH on wakeup](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6cbf1c126cf6) | Yanmin Zhang的测量表明, 如果 SCHED\_BATCH 任务运行与 SCHED\_OTHER 任务相同的 place_entity() 逻辑, 则它们将受益. 因此统一该领域的行为. SCHED\_BATCH 进程唤醒时 vruntime 也将进行补偿. | v1 ☑ 2.6.24-rc6 | [PATCH HISTORY](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6cbf1c126cf6) |
| 2011/02/22 | Darren Hart <dvhart@linux.intel.com> | [a2f5c9ab79f7 sched: Allow SCHED_BATCH to preempt SCHED_IDLE tasks](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a2f5c9ab79f7) | 非交互式任务 SCHED\_BATCH 仍然比 SCHED_BATCH 的任务重要, 因此应该优先于 SCHED\_IDLE 的任务唤醒和运行. | v1 ☑ 2.6.39-rc1 | [PATCH HISTORY](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a2f5c9ab79f7) |



### 1.1.5 不那么重要的进程 SCHED\_IDLE
-------

**2.6.23(2007年10月发布)**


~~此调度策略和 CFS 调度器在同一版本引入. 系统在空闲时, 每个 CPU 都有一个 idle 线程在跑, 它什么也不做, 就是把 CPU 放入硬件睡眠状态以节能(需要特定CPU的driver支持), 并等待新的任务到来, 以把 CPU 从睡眠状态中唤醒. 如果你有任务想在 CPU 完全 idle 时才执行, 就可以用 **sched\_setscheduler()** API 设置此策略.~~


注意上面的描述是有问题的, SCHED_IDLE 和 idle 进程是完全不同的东西

SCHED_IDLE 跟 SCHED_BATCH 一样, 是 CFS 中的一个策略, SCHED\_IDLE 的进程也是 CFS 调度类的一员, CFS内部的家务事, 意味着 SCHED\_IDLE 进程也是需要按照权重来分配CPU时间的, 只是权重很低而已, 权重默认被[设置为 WEIGHT_IDLEPRIO(3), 参见 [cce7ade80369 sched: SCHED_IDLE weight change](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=cce7ade803699463ecc62a065ca522004f7ccb3d).

另一方面, SCHED_IDLE 是用户赋予的, 的确可以用 **sched\_setscheduler()** API 设置此策略

使用了 SCHED_IDLE 策略, 这意味着这些进程是不重要的, 但是 CFS 又号称完全公平, 这体现在哪些方面呢?

- 首先设置了 SCHED_IDLE 策略的进程优先级(nice值)都很低, 这将影响到进程的时间片和负载信息
- SCHED_IDLE 调度策略只在进行抢占处理的时候有一些特殊处理, 比如 check_preempt_curr() 中, 这里当前正在运行的如果是 SCHED_IDLE task 的话, 会马上被新唤醒的 SCHED_NORMAL task抢占, 即 SCHED_NORMAL 的进程可以抢占 SCHED_IDLE 的进程.

因此 Linux 社区里面并没有多少人会使用 SCHED_IDLE 调度策略, 因此自从Linux 2.6.23 引入之后, 就没人对它进行改进.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/07/09 | Ingo Molnar <mingo@elte.hu> | [0e6aca43e08a sched: add SCHED_IDLE policy](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0e6aca43e08a) | NA | v1 ☑ 2.6.23-rc1 | [PATCH HISTORY](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0e6aca43e08a) |
| 2007/07/09 | Ingo Molnar <mingo@elte.hu> | [dd41f596cda0 sched: cfs core code](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=dd41f596cda0) | NA | v1 ☑ 2.6.23-rc1 | [PATCH HISTORY](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=dd41f596cda0) |
| 2009/01/15 | Peter Zijlstra <a.p.zijlstra@chello.nl> | [cce7ade80369 sched: SCHED_IDLE weight change](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=cce7ade80369) | 将 SCHED\_IDLE 权重从 2 增加到 3, 这将提供更稳定的 vruntime. | v1 ☑ 2.6.23-rc1 | [PATCH HISTORY](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=cce7ade80369) |


注意, 在 select_task_rq() 调用中并没有针对 SCHED\_IDLE 调度策略的相应处理, 因此我们并没能做到尽量把新唤醒的 SCHED_NORMAL task 放到当前正在运行 SCHED_IDLE task 的 CPU 上去. 之前的 select_task_rq_fair() 中更倾向于寻找一个 IDLE 的 CPU.

那么这就有一个矛盾的地方了, 那就是在选核的时候, 如果当前 CPU 上正运行 SCHED_IDLE 的进程. 那么选择这样的 CPU 更合适, 还是继续往下寻找 idle 的 CPU 更合适?

*   当前的策略是倾向于是唤醒完全 idle 的 CPU, 而保持 SCHED_IDLE 进程继续占有原来的CPU. 这显然与 SCHED_NORMAL 可以抢占 SCHED_IDLE 的初衷相违背. 这样的好处是系统会更加均衡一些, 但是另外一方面唤醒 idle 的 CPU 是有一定延迟的, 在当前的 CPU 上这样的操作往往可能耗时若干 ms 的时间, 远远比一次抢占的时间要长.

*   另外一种策略就是抢占 SCHED_IDLE 进程, 这样可以保持完全 idle 的 CPU 继续 idle, 降低功耗;

第二种策略虽然从负载均衡看来, 系统貌似不那么均衡了, 但是看起来有诸多的好处. 于是一组特性 [sched/fair: Fallback to sched-idle CPU in absence of idle CPUs](https://lwn.net/Articles/805317), [patchwork](https://lore.kernel.org/patchwork/cover/1094197), [lkml-2019-06-26](https://lkml.org/lkml/2019/6/26/16) 被提出, 并于 5.4-rc1 合入主线.

后续对此补丁还有一些细微的优化,比如 [sched/fair: Make sched-idle cpu selection consistent throughout 1143783 diffmboxseries](https://lore.kernel.org/patchwork/patch/1143783), 优化了选核的逻辑, 之前的逻辑中认为, 如果 find_idlest_group_cpu/sched_idle_core 中如果查找不到 avaliable_idle_cpu(处于 IDLE 状态的 CPU), 才寻找 sched_idle_cpu(运行 SCHED_IDLE 的 CPU), 即优先 IDLE, 其次 SCHED_IDLE. 而 select_idle_sibling 中则认为 avaliable_idle_cpu 和 sched_idle_cpu 都是可以接受的, 找到了任何一个满足要求的则返回. 可以看到不同的选核流程下, 对 avaliable_idle_cpu 和 sched_idle_cpu 的接受程度不同. 但是这其实本身就是一个没有对错的问题, 其实我们选核的时候, 不一定 IDLE 一定是最优的, 如果直接选一个 sched_idle_cpu 的 CPU 有时候是优于 avaliable_idle_cpu 的, 因为这些核是不需要(发 IPI)唤醒的. 因此这个补丁就将选核流程上做了归一, 所有的选核流程都将 avaliable_idle_cpu 和 sched_idle_cpu 同等对待, 只要找到了一个 avaliable_idle_cpu 或者 sched_idle_cpu 都是可以接受的, 没必要在 avaliable_idle_cpu 找不到的时候才去选则 sched_idle_cpu

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2019/6/26 | [sched/fair: Fallback to sched-idle CPU in absence of idle CPUs](https://lore.kernel.org/patchwork/cover/1094197) | CFS SCHED_NORMAL 进程在选核的时候, 之前优先选择 idle 的 CPU, 现在也倾向于选择只有 SCHED_IDLE 的进程在运行的 CPU | v3 ☑ 5.4-rc1 | [LWN](https://lwn.net/Articles/805317), [PatchWork](https://lore.kernel.org/patchwork/cover/1094197), [lkml](https://lkml.org/lkml/2019/6/26/16) |
| 2019/10/24 | [sched/fair: Make sched-idle cpu selection consistent throughout 1143783 diffmboxseries](https://lore.kernel.org/patchwork/patch/1143783) | 重构了 SCHED_IDLE 时的选核逻辑, 所有的选核流程都将 avaliable_idle_cpu 和 sched_idle_cpu 同等对待 | v1 ☑ 5.4-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/1143783) |
| 2020/12/23 | [sched/fair: Load balance aggressively for SCHED_IDLE CPUs](https://lkml.org/lkml/2020/1/8/112) | LOAD_BALANCE 感知 SCHED_IDLE 优化, SCHED_IDLE 的 CPU 虽然运行着进程但是在调度器看来其实也是空闲的, 应该积极地进行负载均衡 |  v1 ☑ 5.6-rc1 | [LKML](https://lkml.org/lkml/2020/1/8/112) |
| 2021/02/22 | [sched: pull tasks when CPU is about to run SCHED_IDLE tasks](https://lore.kernel.org/patchwork/patch/1382990) | 在 CPU 从 SCHED_NORMAL 进程切换到 SCHED_IDLE 任务之前, 尝试通过 load_balance 从其他核上 PULL SCHED_NORMAL 进程过来执行. | v2 | [2020/12/27 v1](https://lore.kernel.org/patchwork/patch/1356241), [2021/02/22 v2](https://lore.kernel.org/patchwork/patch/1143783) |
| 2021/06/08 | [cgroup SCHED_IDLE support](https://lore.kernel.org/patchwork/patch/1443542) | 将 SCHED_IDLE 策略扩展到进程组. | v1 ☑ 5.15 | [2021/06/08 v1](https://lore.kernel.org/patchwork/patch/1443542) |


### 1.1.6 吭哧吭哧跑计算 SCHED\_BATCH
-------

**2.6.16(2006年3月发布)**


概述中讲到 SCHED\_BATCH 并非 POSIX 标准要求的调度策略, 而是 Linux 自己额外支持的.
它是从 SCHED\_OTHER 中分化出来的, 和 SCHED\_OTHER 一样, 不过该调度策略会让采用策略的进程比 SCHED\_OTHER 更少受到 调度器的重视. 因此, 它适合非交互性的, CPU 密集运算型的任务. 如果你事先知道你的任务属于该类型, 可以用 **sched\_setscheduler()** API 设置此策略.


在引入该策略后, 原来的 SCHED\_OTHER 被改名为 SCHED\_NORMAL, 不过它的值不变, 因此保持 API 兼容, 之前的 SCHED\_OTHER 自动成为 SCHED\_NORMAL, 除非你设置 SCHED\_BATCH.

## 1.2  SCHED\_RT
-------

RT 有两种调度策略, SCHED_FIFO 先到先服务 和 SCHED_RR 时间片轮转

系统中的实时进程将比 CFS 优先得到调度, 实时进程根据实时优先级决定调度权值

RR进程和FIFO进程都采用实时优先级做为调度的权值标准, RR是FIFO的一个延伸. FIFO时, 如果两个进程的优先级一样, 则这两个优先级一样的进程具体执行哪一个是由其在队列中的位置决定的, 这样导致一些不公正性(优先级是一样的, 为什么要让你一直运行?),如果将两个优先级一样的任务的调度策略都设为RR,则保证了这两个任务可以循环执行, 保证了公平.


SHCED_RR和SCHED_FIFO的不同：

1.  当采用SHCED_RR策略的进程的时间片用完, 系统将重新分配时间片, 并置于就绪队列尾. 放在队列尾保证了所有具有相同优先级的RR任务的调度公平.

2.  SCHED_FIFO一旦占用cpu则一直运行. 一直运行直到有更高优先级任务到达或自己放弃.

3.  如果有相同优先级的实时进程(根据优先级计算的调度权值是一样的)已经准备好, FIFO时必须等待该进程主动放弃后才可以运行这个优先级相同的任务. 而RR可以让每个任务都执行一段时间.



相同点：

1.  RR和FIFO都只用于实时任务.

2.  创建时优先级大于0(1-99).

3.  按照可抢占优先级调度算法进行.

4.  就绪态的实时任务立即抢占非实时任务.


由于 RT 比 NORMAL 进程拥有较高的优先级, 因此当系统中存在长时间运行的 RT 进程时, 系统通常通过默认的 RT_BANGWIDTH 机制来限制 RT 的带宽, 默认是 95%, 因此每 1S 种, CPU 将让出 0.05s 的时间给优先级较低的 NORMAL 进程运行.

用户总是期望 RT 来完成一些非常紧急, 尽管 linux 不是一个严格意义上的实时内核, 但是也要保证 RT 的实时性, 但是总有一些场景会打破这种限制. 一个进程组中的 RT 任务或者不同用户的 RT 任务, 在一个环境下运行. 如果某些 RT 进程负载非常大, 经常在一段时间内把整个 CPU 占满了, 还有一些 RT 进程往往只是偶尔在非常紧急的时候出来运行.
由于 RT_BANDWIDTH 机制的存在, 当一个 CPU 上的 RT 配额被某个 RT 进程吃掉的时候, 如果这时候也放弃其他的 CPU 是非常不公平, 不实时的. 假如车载事实场景上, 发现系统故障了, 要紧急刹车, 这个处理任务是个 RT 进程, 当他被唤醒了, 但是却发现没有配额无法运行. 那岂不是要出大问题的. 因此内核提供了 RT_RUNTIME_SHARE 特性. 如果当前 RT 队列的运行时间超过了最大运行时间, 则可以从其他 cpu 上借用时间. 只要整个系统中其他 CPU 上有 RT 的配额, 那这个 CPU 上 RT 进程就可以继续运行. 从而可以尽可能的保证 RT 的实时性要求.

RT_RUNTIME_SHARE 这个机制本身是为了解决不同 CPU 上, 以及不同分组的 RT 进程繁忙程度不同, 从而一些运行时间不长的进程因为其他繁忙的 RT 进程占用 CPU 导致被限流的不公平性/不实时的问题. 但是却导致了上述提到的 RT_BANDWIDTH 失效. 因此当 RT 可以源源不断的从其他 CPU 上窃取时间, 将会造成本核上 CFS 进程无法得到调度而出现饿死.

内核社区对此特性多有异议, 但是却一直没有什么完美的解决方案, 之前提出的一些方案也都没有被接受:
[sched/rt: RT_RUNTIME_GREED sched feature](https://lore.kernel.org/patchwork/patch/732374) 实现了一套有环保意识的解决方案, 当发现当前 RT 可以窃取时间, 但是有其他 CFS 进程已经在等待的时候, 那么他将主动放弃窃取的机会, 让位给 CFS 进程运行.

[sched/rt: disable RT_RUNTIME_SHARE by default and document it](https://lore.kernel.org/patchwork/cover/735472) 既然解决方案不被认可, 那就接受他. 但是默认把他禁用, 同时在注释中强烈的标注这个问题, 来表达自己迫切不想再出问题的愿望.

> 通过设置/proc/sys/kernel/sched_rt_runtime_us 和/proc/sys/kernel/sched_rt_period_us配合实现; sched_rt_period_us默认值是1s(1000000us), sched_rt_runtime_us默认是0.95s(950000us); 通过此项配置的RT进程利用率, 是针对整个CPU的, 对于多核处理器, 每个CPU仍然可以跑到100%, 那么怎么让每cpu的利用率都是95%呢？可以设置
> echo NO_RT_RUNTIME_SHARE > /sys/kernel/debug/sched_features; 这样每个核不去借用别的cpu时间, 可以达到95%的限制


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/11/07 | Daniel Bristot de Oliveira <bristot@redhat.com> | [sched/rt: RT_RUNTIME_GREED sched feature](https://lore.kernel.org/patchwork/patch/732374) | 限制 RT_RUNTIME_SHARE 的生效场景, 开启了 RT_RUNTIME_GREED 的情况下, RT_RUNTIME_SHARE 将尽量的绿色无害. 即当发现当前 RT 可以窃取时间, 但是有其他 CFS 进程已经在等待的时候, 那么他将主动放弃窃取的机会, 让位给 CFS 进程运行 | v1 ☐ | [PatchWork](https://lore.kernel.org/patchwork/patch/732374) |
| 2016/11/15 | Daniel Bristot de Oliveira <bristot@redhat.com> | [sched/rt: disable RT_RUNTIME_SHARE by default and document it](https://lore.kernel.org/patchwork/cover/735472) | RT_RUNTIME_GREED 无法合入主线后, 作者尝试默认关掉 RT_RUNTIME_SHARE | v1 ☐ | [PatchWork](https://lore.kernel.org/patchwork/patch/735472) |
| 2020/09/21 | Daniel Bristot de Oliveira <bristot@redhat.com> | [sched/rt: Disable RT_RUNTIME_SHARE by default](https://lore.kernel.org/patchwork/patch/1309182) | 最后一次尝试默认关掉 RT_RUNTIME_SHARE, 终于被主线接受 | RFC ☑ 5.10-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/1309182)<br>*-*-*-*-*-*-*-* <br>[commit 2586af1ac187](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2586af1ac187f6b3a50930a4e33497074e81762d) |



## 1.3 十万火急, 限期完成 SCHED\_DEADLINE
-------

**3.14(2014年3月发布)**


此策略支持的是一种实时任务. 对于某些实时任务, 具有阵发性(sporadic),　它们阵发性地醒来执行任务, 且任务有 deadline 要求, 因此要保证在 deadline 时间到来前完成. 为了完成此目标, 采用该 SCHED\_DEADLINE 的任务是系统中最高优先级的, 它们醒来时可以抢占任何进程.


如果你有任务属于该类型, 可以用 **_sched\_setscheduler()_** 或 **_sched\_setattr()_** API 设置此策略.


更多可参看此文章: [Deadline scheduling: coming soon? [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/575497/)


## 1.4 其他一些调度类的尝试
-------


业务场景中总存在一些对时延敏感但是负载很小的在线任务, 和一些时延不敏感但是负载很大的离线任务. 单独使用 isolation 等为时延敏感的业务分配 CPU 是比较浪费 CPU 资源的, 因此这些业务往往混部在一起. 然而, 现有的实现在混部后在线业务的服务质量下降严重.

虽然内核提供了 SCHED_BATCH 和 SCHED_IDLE 两种优先级比较低的调度算法, 但它们仍然和CFS共用相同的实现, 尤其是在负载均衡时是未做区分的, 它们是没有完全的和CFS隔离开来, 所以效果上面介绍的通用方案存在类似的问题.


其实, 在大家看来, 专门为这些应用新增一个调度类也是一个不错的想法, 通过各个调度类的优先级次序, 原生可以保证在线任务直接抢占离线任务, 保证在线任务的唤醒时延等.


- 一种思路是为时延敏感的在线任务, 新增一个优先级比 CFS 高的调度类.

    暂且称作 background 调度类, 这样在选择 idle 的调度类之前, background 可以兜底了. 各个厂商也都做过类型的尝试. 比如腾讯曾经发往邮件列表的 [BT scheduling class](https://lore.kernel.org/patchwork/cover/1092086), 不过这个版本不完善, 存在诸多问题, 如果大家关注的话, 可以查考查阅 TencentOS-kernel 的 商用版本 [离线调度算法bt](https://github.com/Tencent/TencentOS-kernel#离线调度算法bt).


- 另外一种思路是为时延不敏感的离线任务, 新增一个优先级比 CFS 低的调度类.

    [sched: Add micro quanta scheduling class](https://lkml.org/lkml/2019/9/6/178) 在 RT 之后, CFS 之前实现了一个类似于 RT 的策略, 为在线任务提供服务, 来解决同样的问题.

- 当前其实很多情况下使用 SCHED_IDLE 策略也已经能满足我们的基本要求, SCHED_NORMAL 可以抢占 SCHED_IDLE, 而 SCHED_IDLE 的进程也只有在空闲时候才出来蹦跶蹦跶.

    特别是在之前提到的 [sched/fair: Fallback to sched-idle CPU in absence of idle CPUs](https://lore.kernel.org/patchwork/cover/1094197) 合入之后, SCHED_NORMAL 的时延已经很低, 其实可以理解为在 SCHED_NORMAL(CFS) 下面又添加了一层时延不敏感的离线任务, 只不过不是使用新增调度类的方式. 而是借助了 CFS 的框架和策略实现.


## 1.5 调度类的一些其他优化点
-------

调度器的演进是 linux 不断发展和完善的风向标之一. 调度器对性能的追求是无止境的, 不光调度本身对性能的影响比较大. 另外一方面, 调度器本身的性能影响也要最小. 于是内核开发者们在这条路上, 不断前行. 调度器的终极目标就是: 用最小的性能开销, 发挥 CPU 的最大能力.

### 1.5.1  lockless scheduler 调度器减少 rq->lock 的争抢
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2011/04/05 | Peter Zijlstra | [sched: Reduce runqueue lock contention -v6](https://lore.kernel.org/patchwork/cover/244310) | 优化调度器的路径, 减少对 rq->lock 的争抢, 实现 lockless. | RFC ☑ 3.0-rc1 | [PatchWork v6](https://lore.kernel.org/patchwork/cover/244310) |

这组补丁是当年亮点的补丁, 虽然做了较大的重构, 但是通过去锁和无锁话, 性能得到了提升. 虽然也引入了不少问题, 但是毋庸置疑, 这是调度器历史上的一次飞跃.


### 1.5.2 直接比较两个调度类的优先级
-------

linux 调度器定义了多个调度类, 不同调度类的调度优先级不同, 通过链表的方式进行排列, 高优先级调度类的 next 指针域指向低优先级的调度类. 从而可以使用 for_each_class 的方式进行遍历. 这其实是有缺陷的:

*   首先, 我们需要频繁的遍历这个链表, 但是 list next 的方式不像数组, 编译器无法对其做必要的优化, 他们的地址可能不集中, 因此无法使用数据预取, 严重的可能还会频繁的触发 cache-miss.

*   很多时候, 我们期望能直接比较两个调度类的优先级高低, 但是很抱歉, 目前这种链表的组织形式, 对这种方式很不友好. 如果非要比较, 我们也不得不遍历一次.

调度器中的两个热点函数 `pick_next_task()` 和 `check_preempt_curr()` 都需要使用 for_each_class, 也迫切期望能直接比较两个调度类的优先级.

那怎么解决这个问题呢? 最简单的问题, 就是在链接时, 显式按照优先级对调度类进行排布. 于是 Kirill Tkhai 发出了这组补丁 [sched: Micro optimization in pick_next_task() and in check_preempt_curr()](https://lore.kernel.org/patchwork/cover/1170249), 对于这组补丁 Steven Rostedt 给出了很多意见, 并在作者发出 [v2 之后](https://lore.kernel.org/patchwork/cover/1170294), 进行了改进, 发出了一组补丁 [sched: Optimizations to sched_class processing](https://lore.kernel.org/patchwork/cover/1170901). 保证了链接时候按照调度类优先级进行排布, 并将他们放到了一个单独的段中.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/12/19 | Kirill Tkhai | [sched: Micro optimization in pick_next_task() and in check_preempt_curr()](https://lore.kernel.org/patchwork/cover/1170294) | 在二进制中通过 xxx_sched_class 地址顺序标记调度类的优先级, 从而可以通过直接比较两个 xxx_sched_class 地址的方式, 优化调度器中两个热点函数 pick_next_task()和check_preempt_curr(). | v2 ☐ |[PatchWork RFC](https://lore.kernel.org/patchwork/cover/1170249)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/cover/1170294) |
| 2019/12/19 | Steven Rostedt | [sched: Optimizations to sched_class processing](https://lore.kernel.org/patchwork/cover/1170901) | 对上面补丁的进一步优化, 对齐数据结构保证 cache 对齐, 通过链接脚本保证数据的排布顺序. | RFC ☑ 5.9-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1170901) |

这组补丁在 5.9-rc1 时合入主线, 至此, 我们可以[直接在调度器中通过比较地址高低, 直接判断两个调度类的优先级](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=aa93cd53bc1b91b5f99c7b55e3dcc1ac98e99558). 当然这组补丁还有个附带的好处, 就是 `kernel/sched/core.o` 的二进制体积更小了.

从这组补丁可以看出来, 调度器中的算法和数据结构对性能简直到了吹毛求疵的地步, 这里也不得不佩服社区调度和性能大神的脑洞和技术能力.

### 1.5.3 SMT 适配与优化
-------

早期调度器上 SMT 的优化, 主要集中在 POWERPC.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2010/06/08 | Michael Neuling | [sched: asymmetrical packing for POWER7 SMT4](https://lore.kernel.org/patchwork/cover/202834) | 这个补丁集实现了非对称 SMT 封装(SD_ASYM_PCAKING), 在任务负载小的时候, 将进程都打包在 SMT 域内的某一个 CPU 上, 从而确保在 POWER7 上始终保持良好的性能. 如果没有这个系列, 在 POWER7 上, 任务的性能将有大约 +/-30% 的抖动. | v2 ☐ |[PatchWork RFC](https://lore.kernel.org/patchwork/cover/1408312)) |
| 2016/11/01 | Ricardo Neri | [Support Intel® Turbo Boost Max Technology 3.0](https://lore.kernel.org/patchwork/cover/722406) | 支持 Intel 超频 | RFC ☑ 5.9-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/722406) |
| 2021/08/10 | Ricardo Neri <ricardo.neri-calderon@linux.intel.com> | [sched/fair: Fix load balancing of SMT siblings with ASYM_PACKING](https://lore.kernel.org/patchwork/cover/1428441) | 修复 ASYM_PACKING 和 load_balance 的冲突. | v5 ☐ | [PatchWork v1](https://lore.kernel.org/patchwork/cover/1408312)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/cover/1413015)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3 0/6](https://lore.kernel.org/patchwork/cover/1428441)<br>*-*-*-*-*-*-*-* <br>[PatchWork v4,0/6](https://lore.kernel.org/patchwork/cover/1474500)<br>*-*-*-*-*-*-*-* <br>[LKML v5,0/6](https://lkml.org/lkml/2021/9/10/913) |
| 2011/12/15 | Peter Zijlstra <peterz@infradead.org> | [sched: Avoid SMT siblings in select_idle_sibling() if possible](https://lore.kernel.org/patchwork/cover/274702) | 如果有共享缓存的空闲核心, 避免 select_idle_sibling() 选择兄弟线程. | v1 ☐ | [PatchWork v1](https://lore.kernel.org/patchwork/cover/274702) |



core_scheduling 与 coscheduling

*   core_scheduling 是近几年(笔者当前时间 2021 年 1 月)争议颇大的一个功能.

随着 Intel CPU 漏洞的公布, 内核也开始关注之前硬件的一些设计所带来性能提升的同时, 引入的一些安全问题. SMT 超线程技术(simultaneous multi-threading, 或者"hyperthreading", 超线程), 通过将一个物理 core 虚拟成2个或者多个 CPU thread(PE), 从而充分使用 CPU 的流水线技术, 为了提升性能, CPU 上进程运行时的数据不可避免会被加载到 cache 中, 由于这些 cpu thread 物理上是一个 core, 因此不能排除某些不怀好意的用户或者进程可以通过窥测的方式, 窥测同一个 core 上其他进程的隐私数据, 虽然这较难实施, 但是目前是不可屏蔽的. 这被称为 侧信道攻击.

>  一个SMT 核包含两个或者更多的CPU(有时候也被称为"hardware threads")都共享很多底层硬件.  这里共享的包括一部分cache, 这样导致SMT可能会受基于cache进行的side-channel攻击.

Core scheduling 最开始的讨论来自 Julien Desfossez 和 Vineeth Remanan Pillai. 主要目的是为了让 SMT能更加安全, 不受 speculative-execution 类型的攻击. 它的主要思路是通过一些手段(当前实现是 cgoup cpu subsystem)标记一组进程之间的相互信任的, 这组进程通常是一个单独的用户, 或者一组单独的进程, 假设他们至少彼此任务自己不是不怀好意的, 或者即使是不怀好意的, 也是用户有意为之, 影响也只是自己. 那么 core scheduling 将只允许两个相互之间信任的进程运行在一个 SMT 的两个 thread 上. 不信任的线程, 将不能在一个 core 上协同运行.

Core scheduling 势必带来一些性能的下降, 但是理论上性能比开了 SMT 要差一些, 但是至少会比关掉 SMT 要好一些. 但是实施起来你会发现这只是大家一厢情愿了. 不同组之间相互是独占 CPU core 的, 那么不同组之间频繁争抢 CPU 资源和频繁开销所引入的影响, 在不同场景会被无限制放大. 而且不同组之间并没有优先级的概念, 因此混布场景性能下降最明显.

> 对于担心这类攻击的服务器来说, 最好现在就把SMT关掉, 当然这会让一些workload(负载)的性能有不少下降.
> 在 2019 Linux Plumbers Conference 会议上至少有三个议题都提到了它. 其中主要的收获, 可能就是在最早的“防止side-channel attack”的目标之外, 又找到了其他一些应用场景.

*   coscheduling 近年来已经很少能看到调度里面 60 个补丁的大特性了

乍看起来, 真的以为是 core_scheduling 拼错了, 但是也在 SMT 安全问题的时候再次被提出来.

coscheduling 协同调度是为了解决云服务场景, 为不同用户提供稳定的系统资源. 思路跟 core_scheduling 类似, 都是希望相互信任的一组进程可以互斥的占有 CPU 资源. 但是 coscheduling 的实现要比 core_scheduling 复杂的多. coscheduling 允许用户动态的指定互斥使用的 CPU 拓扑层级, 你可以限制 core 层次的 CPU 只能运行一组信任的进程, 也可以继续扩大限制的范围, 限制 MC 甚至是系统所有的 CPU 都只能运行一组进程. 因此他引入了 RD_RQ 的概念, 即调度域层级的 RQ 结构.

而 core_scheduling 顾名思义只是提供了 core 级别的互斥调度. 因此将 coscheduling 的层次设置到 core 级别, 就完成了 core_scheduling 一样的功能. 因此在侧信道攻击被关注的时机, 作者非常适时的把这组补丁发出来给大家探讨. 当然这组补丁目前被社区接受的概念几乎可以忽略, 但是却总是让人眼前一亮.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/12/19 | Jan H. Schönherr | [[RFC,00/60] Coscheduling for Linux](https://lore.kernel.org/patchwork/cover/983568) | 亚马逊的协同调度方案 | RFC ☑ 5.9-rc1 | [PatchWork RFC](https://lore.kernel.org/patchwork/cover/983568) |
| 2019/12/19 | Peter & Kirill Tkhai 等 | [Core scheduling (v9)](https://lore.kernel.org/patchwork/cover/1340764) | 核调度器, 限制同一个 SMT 域内的两个 CPU 只能运行同一组进程 | v9 ☐ |[PatchWork v9](https://lore.kernel.org/patchwork/cover/1340764) |
| 2021/03/25 | Joel Fernandes 等 | [Core scheduling remaining patches rebase](https://lore.kernel.org/patchwork/cover/1369931) | Core scheduling v9 的大部分补丁都已经在合入队列了, 部分未合入补丁的重构与适配. | v10 ☐ | [PatchWork v9](https://lore.kernel.org/patchwork/cover/1369931)<br>*-*-*-*-*-*-*-* <br>[PatchWork v9 resend](https://lore.kernel.org/patchwork/cover/1401863) |
| 2021/04/01 | Peter Zijlstra | [sched: Core scheduling interfaces](https://lore.kernel.org/patchwork/cover/1406301) | Peter 重新设计了 Core scheduling 的接口. | v10 ☐ |[PatchWork v9](https://lore.kernel.org/patchwork/cover/1406301) |
| 2021/04/22 | Peter Zijlstra | [sched: Core Scheduling](https://lore.kernel.org/patchwork/cover/1417028) | Peter 重构的 Core scheduling, 已经合入 TIP 分支 | v10 ☑ 5.14-rc1 |[PatchWork v10](https://lore.kernel.org/patchwork/cover/1417028) |



# 2 组调度支持(Group Scheduling)
-------

## 2.1 普通进程的组调度支持(Fair Group Scheduling)
-------

### 2.1.1 CFS 组调度
-------

**2.6.24(2008年１月发布)**


2.6.23 引入的 CFS 调度器对所有进程完全公平对待. 但这有个问题, 设想当前机器有２个用户, 有一个用户跑着 9个进程, 还都是 CPU 密集型进程; 另一个用户只跑着一个 X 进程, 这是交互性进程. 从 CFS 的角度看, 它将平等对待这 10 个进程, 结果导致的是跑 X 进程的用户受到不公平对待, 他只能得到约 10% 的 CPU 时间, 让他的体验相当差.


基于此, 组调度的概念被引入[<sup>6</sup>](#refer-anchor-6). CFS 处理的不再是一个进程的概念, 而是调度实体(sched entity), 一个调度实体可以只包含一个进程, 也可以包含多个进程. 因此, 上述例子的困境可以这么解决: 分别为每个用户建立一个组, 组里放该用户所有进程, 从而保证用户间的公平性.

该功能是基于控制组(control group, cgroup)的概念, 需要内核开启 CGROUP 的支持才可使用. 关于 CGROUP, 以后可能会写.


### 2.1.2 CFS BANDWIDTH 带宽控制
-------

内核中的 `CFS BANDWIDTH Controller` 是控制每个 CGROUP 可以使用多少 CPU 时间的一种有效方式. 它可以避免某些进程消耗过多的 CPU 时间, 并确保所有需要 CPU 的进程都能拿到足够的时间.

这个 bandwidth controller 提供了两个参数来管理针对各个 cgroup 的限制.

| 参数 | 描述 |
|:---:|:----:|
| cpu.cfs_quota_us | 是这个 cgroup 在每个统计周期(accounting period)内可用的 CPU 时间(以微秒为单位).  |
| cpu.cfs_period_us | 是统计周期的长度, 也是以微秒为单位 |

因此, 假如我们将 cpu.cfs_quota_us 设置为 50000, cpu.cfs_period_us 设置为 100000, 将使该组在每 100ms 周期内消耗 50ms 的 CPU 时间. 将这些值减半(将 cpu.cfs_quota_us 设置为 25000, cpu.cfs_period_us 设置为 50000)的话, 就是每 50ms 消耗 25ms 的 CPU 时间. 在这两种情况下, 这个 cgroup 就被授权可以占用一个 CPU 的 50% 资源, 但在后一种设置下, 可供使用的 CPU 时间会来得更频繁, 更小块.

这两种情况的区别很重要. 想象一下, 某个 cgroup 中只有一个进程, 需要运行 30ms. 在第一种情况下, 30ms 小于被授予的 50ms 时长, 所以进程能够完成任务, 不会被限制. 在第二种情况下, 进程在运行 25ms 后将被停止执行, 不得不等待下一个 50ms 周期再继续运行来完成工作. 如果 workload 对延迟敏感的话, 就需要仔细考虑 bandwidth controller 参数的设置.

可以想象, Bandwidth Controller 并未完全满足每一个 workload 的全部需求. 这种机制对于那些需要持续执行特定 CPU 时长的工作负载来说效果相当不错. 不过, 对于突发性的工作负载, 会比较尴尬. 某个进程在大多数时间段内需要使用的时间可能远远少于它的配额(quota), 但偶尔会出现一个突发的工作, 此时它所需要用到的 CPU 时间可能又比配额要多. 在延迟问题并不敏感的情况下, 当然可以让该进程等待到下一个周期来完成它的工作, 但如果延迟问题确实影响很大的话, 那么我们不应该让它再等到下个周期.


人们尝试了一些方法来解决这个问题.

1.  大家都能想到的一种方法是直接给这个进程分配足够大的配额, 就可以处理这类突发事件了, 但这样做会使该进程总体上消耗更多的 CPU 时间. 系统管理员可能不喜欢这种做法, 尤其是如果用户只付费购买了一部分 CPU 时间的情况下, 系统管理员不希望给他分配更多时间.

2.  另一种选择就是同时增加配额和周期, 但如果进程最终还是要等到下一个周期才能完成工作的话, 那也会导致延迟增加.


社区提出的一组 patch 旨在让它更好地适用于突发的(bursty)、对延迟敏感(latency-sensitive)的 workload.

Chang 的 patch set 采用了与之前不同的方法：允许 cgroup 将一些未使用的配额从一个周期转到下一个周期. 引入的新参数 cpu.cfs_burst_us 就设置了这种方式可以累积的时长上限. 举个例子, 继续讨论配额为 25ms, 周期为 50ms 的 cgroup 配置. 如果将 cpu.cfs_burst_us 设置为 40000(40ms), 那么这个 cgroup 中的进程在某个周期内最多可以运行 40ms, 但前提是它已经在之前的周期中省下了这次额外所需的 15ms. 这样一来, 这个 cgroup 就可以既满足它的突发性工作需求, 又能够在长时间的统计中保证它的 CPU 耗时仍然不超过配额上限.

可以用另一种方式来解释这个做法, 当使用了 cpu.cfs_burst_us 的时候, 配额(quota)的定义就跟从前不一样了. 配额不再是绝对的数值上限, 而是在每个周期内向此 CPU 时间账户内存入的时长, cfs_burst_us 值就是这个账户的封顶值. 需要预先为突发事件做准备的 cgroup 就可以在该账户中来储蓄一些(有限的) CPU 时间, 在需要时使用.

默认情况下, cpu.cfs_burst_us 为 0, 这就禁用了 burst 机制, 而遵照传统行为执行. 系统中有一个 sysctl 开关可以用来在整个系统中禁止使用 burst 方式. 还有另一个开关 (sysctl_sched_cfs_bw_burst_onset_percent), 用来每个周期开始时给每个组一个规定百分比的突发配额(burst quota), 不管以前的周期中是否积累出了这些时间.

这组 patch set 附带了一些 benchmark 数据, 结果显示当使用 burstable controller 时, 最坏情况延迟数据有数量级的减少. 这个想法目前已经在 mailing list 上看到过好几次, 既有当前这个版本, 也有 Cong Wang 和 Konstantin Khlebnikov 独立实现的版本. 目前看来, 最大的障碍已经被克服了, 所以这个改动可能会在 5.13 合并窗口中进入 mainline.

https://lore.kernel.org/lkml/20180522062017.5193-1-xiyou.wangcong@gmail.com/
https://lore.kernel.org/lkml/157476581065.5793.4518979877345136813.stgit@buzz/

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/21 | JHuaixin Chang | [sched/fair: Burstable CFS bandwidth controller](https://lore.kernel.org/patchwork/cover/1396878) | 突发任务的带宽控制优化, 通过临时之前剩余累计的配额, 使得突发进程在当前周期的配额突然用尽之后, 还可以临时使用之前累计的配额使用, 从而降低突发任务的时延. | v6 ☑ 5.14-rc1 | [ 2020/12/17 v1](https://lore.kernel.org/patchwork/cover/1354613)<br>*-*-*-*-*-*-*-*<br>[2021/01/20 v2](https://lore.kernel.org/patchwork/cover/1368037)<br>*-*-*-*-*-*-*-*<br>[2021/01/21 v3](https://lore.kernel.org/patchwork/cover/1368746)<br>*-*-*-*-*-*-*-*<br>[2021-02-02 v4](https://lore.kernel.org/patchwork/cover/1396878)<br>*-*-*-*-*-*-*-*<br>[2021/05/20 v5](https://lore.kernel.org/patchwork/cover/1433660)<br>*-*-*-*-*-*-*-*<br>[2021/06/21 v6](https://lore.kernel.org/patchwork/cover/1449268)<br>*-*-*-*-*-*-*-*<br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f4183717b370ad28dd0c0d74760142b20e6e7931) |
| 2021/07/30 | Huaixin Chang <changhuaixin@linux.alibaba.com> | [Add statistics and ducument for cfs bandwidth burst](https://lore.kernel.org/patchwork/cover/1396878) | 为 Burstable CFS bandwidth 添加统计信息和文档. | v1 ☐ | [ 2020/12/17 v1](https://lore.kernel.org/patchwork/cover/1396878) |


## 2.2 实时进程的组调度支持(RT Group Scheduling)
-------


**2.6.25(2008年4月发布)**

该功能同普通进程的组调度功能一样, 只不过是针对实时进程的.


## 2.3 组调度带宽控制(CFS bandwidth control)** , **3.2(2012年1月发布)**
-------


组调度的支持, 对实现多租户系统的管理是十分方便的, 在一台机器上, 可以方便对多用户进行 CPU 均分．然后, 这还不足够, 组调度只能保证用户间的公平, 但若管理员想控制一个用户使用的最大 CPU 资源, 则需要带宽控制．3.2 针对 CFS组调度, 引入了此功能[<sup>8</sup>](#refer-anchor-8), 该功能可以让管理员控制在一段时间内一个组可以使用 CPU 的最长时间．



## 2.4 极大提高体验的自动组调度(Auto Group Scheduling)
-------

**2.6.38(2011年3月发布)**

试想, 你在终端里熟练地敲击命令, 编译一个大型项目的代码, 如 Linux内核, 然后在编译的同时悠闲地看着电影等待, 结果电脑却非常卡, 体验一定很不爽．



2.6.38 引入了一个针对桌面用户体验的改进, 叫做自动组调度．短短400多行代码[<sup>9</sup>](#refer-anchor-9), 就很大地提高了上述情形中桌面使用者体验, 引起不小轰动．



其实原理不复杂, 它是基于之前支持的组调度的一个延伸．Unix 世界里, 有一个**会话(session)** 的概念, 即跟某一项任务相关的所有进程, 可以放在一个会话里, 统一管理．比如你登录一个系统, 在终端里敲入用户名, 密码, 然后执行各种操作, 这所有进程, 就被规划在一个会话里．


因此, 在上述例子里, 编译代码和终端进程在一个会话里, 你的浏览器则在另一个会话里．自动组调度的工作就是, 把这些不同会话自动分成不同的调度组, 从而利用组调度的优势, 使浏览器会话不会过多地受到终端会话的影响, 从而提高体验．



该功能可以手动关闭.


# 3 负载跟踪机制
-------

## 3.1 WALT
-------

原谅我在这里跳过了主线默认强大的 PELT, 而先讲 WALT.

[improving exynos 9810 galaxy s9](https://www.anandtech.com/show/12620/improving-the-exynos-9810-galaxy-s9-part-2)

## 3.2 PELT
-------

从Arm的资源来看, 这很像该公司意识到性能问题, 并正在积极尝试改善 PELT 的行为以使其更接近 WALT.

1.  一个重要的变化是称为 [util_est 利用率估计的特性](http://retis.santannapisa.it/~luca/ospm-summit/2017/Downloads/OSPM_PELT_DecayClampingVsUtilEst.pdf), [Utilization estimation (util_est) for FAIR tasks](https://lore.kernel.org/patchwork/cover/932237)

2.  改善PELT的另一种简单方法是[减少斜坡/衰减时间](https://lore.kernel.org/lkml/20180409165134.707-1-patrick.bellasi@arm.com/#r), 主线默认的 PELT 衰减周期为 32MS, 该补丁提供了 8MS/16MS/32MS 的可选择衰减周期. 通常的测试结果会认为 8ms 的半衰期是一种偏性能的选择, 默认的 32ms 设置, 无法满足终端场景突发的负载变化, 因此往往 16ms 的折中方案能提供最佳性能和电池折衷.



# 4 基于调度域的负载均衡
-------

## 4.1 拓扑域构建
-------

### 4.1.1 拓扑域
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:----:|:---:|:----------:|:---:|
| 2018/05/30 | Srikar Dronamraju <srikar@linux.vnet.ibm.com> | [Skip numa distance for offline nodes](https://lore.kernel.org/patchwork/patch/1433871) | NA | v1 ☐ | [select_idle_sibling rework](https://lore.kernel.org/patchwork/patch/1433871) |


### 4.1.2 3-hops 问题
-------



最早的时候是 Valentin Schneider 在 HUAWEI KunPeng 920 上发现了一个问题, 构建调度域的过程中在部分 CPU 构建时 报了 `ERROR: groups don't span domain->span`. 参见 [sched/topology: Fix overlapping sched_group build](https://lore.kernel.org/lkml/20200324125533.17447-1-valentin.schneider@arm.com). 这台机器由 2 socket(每个 scoker 由 2 个 DIE 组成, 每个 DIE 包含 24 个 CPU).

社区经过讨论发现, 这是由于内核当前调度域构建的方式存在诸多限制, 对超过 2 个 NUMA 层级的系统支持不完善. 因此我们称这个问题为 3 跳问题(3 htops)暂时没有想到好的解决方法. 因此 Valentin Schneider 决定先通过 WARN_ON 警告此问题 [sched/topology: Warn when NUMA diameter > 2](https://lore.kernel.org/lkml/20201110184300.15673-1-valentin.schneider@arm.com).



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:----:|:---:|:----------:|:---:|
| 2020/3/24 | Valentin Schneider <valentin.schneider@arm.com> | [sched/topology: Fix overlapping sched_group build](https://lore.kernel.org/lkml/20200324125533.17447-1-valentin.schneider@arm.com) | NA | v1 ☐ | [LKML](https://lkml.org/lkml/2020/3/24/615) |
| 2020/8/14 | Valentin Schneider | [sched/topology: NUMA topology limitations](https://lkml.org/lkml/2020/8/14/214) | 修复 | v1 | [LKML](https://lkml.org/lkml/2020/8/14/214) |
| 2020/11/10 | Valentin Schneider | [sched/topology: Warn when NUMA diameter > 2](https://lore.kernel.org/patchwork/patch/1336369) | WARN | v1 ☑ 5.11-rc1 | [PatchWork](https://lore.kernel.org/lkml/20201110184300.15673-1-valentin.schneider@arm.com), [LKML](https://lkml.org/lkml/2020/11/10/925), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b5b217346de85ed1b03fdecd5c5076b34fbb2f0b) |

随后告警的合入, 越来越多的人发现了这个问题, 并进行了讨论 [5.11-rc4+git: Shortest NUMA path spans too many nodes](https://lkml.org/lkml/2021/1/21/726).

| 时间  | 提问者 | 问题描述 |
|:----:|:-----:|:-------:|
| 2021/1/21 | Meelis Roos | [Shortest NUMA path spans too many nodes](https://lkml.org/lkml/2021/1/21/726) |

3-hops 目前来看存在两个问题:

1.  sched_init_numa() 中在构建 numa 距离表的时候, 假定以 NODE 0 为起点的距离阶梯包含了系统中所有节点的距离步长. 这种假定在以 NODE 0 为边缘结点的 NUMA 系统中工作的很好, 但是对于不以 NODE 0 为边缘结点的 NUMA 系统中并不适用. 这会造成以 NODE 0 构建的 NUMA 距离表缺失了一部分距离, 造成真正的边缘结点缺失了一层调度域, 并没有包含系统中所有的结点.


```cpp
以 0 为结点的 NUMA 系统 :
      2       10      2
  0 <---> 1 <---> 2 <---> 3

以 1 为结点的 NUMA 系统 :
      2       10      2
  1 <---> 0 <---> 2 <---> 3
```

2.  build_overlap_sched_groups() 中构建顶层调度域时, 为边缘节点构建高层级调度域时, 当前域中可能只想包含 3 个结点, 但是 sched_group 却意外的把另外一个结点也包含进来了. 从而造成上报了 `ERROR: groups don't span domain->span` 等错误.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:---------:|:----:|
| 2021/01/22 | Valentin Schneider | [sched/topology: NUMA distance deduplication](https://lore.kernel.org/patchwork/cover/1369363) | 修复问题 1 | v1 ☑ 5.12-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1369363), [LKML](https://lkml.org/lkml/2021/1/22/460), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=620a6dc40754dc218f5b6389b5d335e9a107fd29) |
| 2021/01/15 | Song Bao Hua (Barry Song) | [sched/fair: first try to fix the scheduling impact of NUMA diameter > 2](https://lore.kernel.org/patchwork/patch/1366256) | 修复问题 2 | RFC ☐ | [PatchWork](https://lore.kernel.org/patchwork/cover/1366256) |
| 2021/2/23 | Song Bao Hua (Barry Song) | [sched/topology: fix the issue groups don't span domain->span for NUMA diameter > 2](https://lore.kernel.org/patchwork/patch/1371875)| 修复问题 2<br>build_overlap_sched_groups() 中构建调度域的时候如果将某个 (child) sched_domain 作为 sched_group 加进 (parent) sched_domain 的时候, 如果发现其 sched_group_span 不是 parent sched_domain_span 子集的情况, 则通过 find_descended_sibling() 查找该 (child) sched_doman 的 child, 将符合要求的 child 作为 sched_group 加进来. | v4 ☑ 5.13-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1371875), [LKML](https://lkml.org/lkml/2021/2/23/1010), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=585b6d2723dc927ebc4ad884c4e879e4da8bc21f) |


## 4.2 负载均衡
-------


**2.6.7(2004年6月发布)**

计算机依靠并行度来突破性能瓶颈, CPU个数也是与日俱增. 最早的是 SMP(对称多处理), 所以 CPU共享内存, 并访问速度一致. 随着 CPU 个数的增加, 这种做法不适应了, 因为 CPU 个数的增多, 增加了总线访问冲突, 这样 CPU 增加的并行度被访问内存总线的瓶颈给抵消了, 于是引入了 NUMA(非一致性内存访问)的概念. 机器分为若干个node, 每个node(其实一般就是一个 socket)有本地可访问的内存, 也可以通过 interconnect 中介机构访问别的 node 的内存, 但是访问速度降低了, 所以叫非一致性内存访问. Linux 2.5版本时就开始了对 NUMA 的支持[<sup>7</sup>](#refer-anchor-7).


而在调度器领域, 调度器有一个重要任务就是做负载均衡. 当某个 CPU 出现空闲, 就要从别的 CPU 上调整任务过来执行; 当创建新进程时, 调度器也会根据当前负载状况分配一个最适合的 CPU 来执行. 然后, 这些概念是大大简化了实际情形.


在一个 NUMA 机器上, 存在下列层级:

> 1. 每一个 NUMA node 是一个 CPU socket(你看主板上CPU位置上那一块东西就是一个 socket).
> 2. 每一个socket上, 可能存在两个核, 甚至四个核.
> 3. 每一个核上, 可以打开硬件多纯程(HyperThread).


如果一个机器上同时存在这三人层级, 则对调度器来说, 它所见的一个逻辑 CPU其实是一人 HyperThread．处理同一个core 中的CPU , 可以共享L1, 乃至 L2 缓存, 不同的 core 间, 可以共享 L3 缓存(如果存在的话)．


基于此, 负载均衡不能简单看不同 CPU 上的任务个数, 还要考虑缓存, 内存访问速度．所以, 2.6.7 引入了**调度域(sched domain)** 的概念, 把 CPU 按上述层级划分为不同的层级, 构建成一棵树, 叶子节点是每个逻辑 CPU, 往上一层, 是属于 core 这个域, 再往上是属于 socket 这个域, 再往上是 NUMA 这个域, 包含所有 CPU.


当进行负载均衡时, 将从最低一级域往上看, 如果能在 core 这个层级进行均衡, 那最好; 否则往上一级, 能在socket 一级进行均衡也还凑合; 最后是在 NUMA node 之间进行均衡, 这是代价非常大的, 因为跨 node 的内存访问速度会降低, 也许会得不偿失, 很少在这一层进行均衡．



这种分层的做法不仅保证了均衡与性能的平衡, 还提高了负载均衡的效率．


关于这方面, 可以看这篇文章: [Scheduling domains [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/80911/)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2009/09/01 | Peter Zijlstra <a.p.zijlstra@chello.nl> | [load-balancing and cpu_power -v2](https://lore.kernel.org/patchwork/patch/169381) | load balance 感知 CPU PWOER, 对于 RT 任务, 引入了 sched_time_avg 来平均一段时间内实时任务的 CPU 消耗. 用得到的平均值来调整非实时任务的 CPU 功率, 以确保实时任务不必争抢 CPU. | v4 ☑ |[PatchWork](https://lore.kernel.org/patchwork/patch/169381) |


## 4.3 自动 NUMA 均衡(Automatic NUMA balancing)
-------

**3.8(2013年2月发布)**

NUMA 机器一个重要特性就是不同 node 之间的内存访问速度有差异, 访问本地 node 很快, 访问别的 node 则很慢．所以进程分配内存时, 总是优先分配所在 node 上的内存．然而, 前面说过, 调度器的负载均衡是可能把一个进程从一个 node 迁移到另一个 node 上的, 这样就造成了跨 node 的内存访问; Linux 支持 CPU 热插拔, 当一个 CPU 下线时, 它上面的进程会被迁移到别的 CPU 上, 也可能出现这种情况．



调度者和内存领域的开发者一直致力于解决这个问题．由于两大系统都非常复杂, 找一个通用的可靠的解决方案不容易, 开发者中提出两套解决方案, 各有优劣, 一直未能达成一致意见．3.8内核中, 内存领域的知名黑客 Mel Gorman 基于此情况, 引入一个叫自动 NUMA 均衡的框架, 以期存在的两套解决方案可以在此框架上进行整合; 同时, 他在此框架上实现了简单的策略: 每当发现有跨 node 访问内存的情况时, 就马上把该内存页面迁移到当前 node 上．



不过到 4.2 , 似乎也没发现之前的两套方案有任意一个迁移到这个框架上, 倒是, 在前述的简单策略上进行更多改进．



如果需要研究此功能的话, 可参考以下几篇文章:

－介绍 3.8 前两套竞争方案的文章: [A potential NUMA scheduling solution [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/522093/)

- 介绍 3.8 自动 NUMA 均衡 框架的文章: [NUMA in a hurry [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/524977/)

- 介绍 3.8 后进展的两篇文章, 细节较多, 建议对调度／内存代码有研究后才研读:

[NUMA scheduling progress [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/568870/)

[NUMA placement problems [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/591995/)

## 4.4 rework_load_balance
-------

2019 年的 [Vincent Guittot](https://www.youtube.com/watch?v=cfv63BMnIug) 的 [sched/fair: rework the CFS load balance](https://lwn.net/Articles/793427) 是近几年特别有亮点的补丁.
. 博主个人一直是计算机先驱"高德纳"教授"文学化编程"思想的坚定追随者, 小米创始人雷军雷布斯先生也说"写代码要有写诗一样的感觉". 这种代码才真的让人眼前一亮, 如沐春风. 这个就是我看到 [rework_load_balance 这组补丁](https://lore.kernel.org/patchwork/cover/1141687) 的感觉. 这组补丁通过重构 (CFS) load_balance 的逻辑, 将原来逻辑混乱的 load_balance 变成了内核中一抹亮丽的风景, 不光使得整个 load_balance 的框架更清晰, 可读性更好. 更带来了性能的提升.

它将系统中调度组的状态[归结于集中类型](https://lore.kernel.org/patchwork/patch/1141698), 对于其中的负载不均衡状态分别采用不同的处理方式.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2019/10/18 | | [sched/fair: rework the CFS load balance](https://linuxplumbersconf.org/event/4/contributions/480) | 重构 load balance | v4 ☑ | [LWN](https://lwn.net/Articles/793427), [PatchWork](https://lore.kernel.org/patchwork/patch/1141687), [lkml](https://lkml.org/lkml/2019/10/18/676) |
| 2019/10/22 | | [sched/fair: fix rework of find_idlest_group()](https://lore.kernel.org/patchwork/patch/1143049) | fix 补丁 | | |
| 2019/11/29 | | [sched/cfs: fix spurious active migration](https://lore.kernel.org/patchwork/patch/1160934) | fix 补丁 | | |
| 2019/12/20 | | [sched/fair : Improve update_sd_pick_busiest for spare capacity case 1171109 diffmboxseries](https://lore.kernel.org/patchwork/patch/1171109/) | | |
| 2021/01/06 | Vincent Guittot | [sched/fair: ensure tasks spreading in LLC during LB](https://lore.kernel.org/patchwork/cover/1330614) | 之前的重构导致 schbench 延迟增加95%以上, 因此将 load_balance 的行为与唤醒路径保持一致, 尝试为任务选择一个同属于LLC 的空闲 CPU. 从而在空闲 CPU 上更好地分散任务. | v1 ☑ 5.10-rc4 | [PatchWork](https://lore.kernel.org/patchwork/cover/1330614) |


在 Vincent 进行重构的基础上, Mel Gorman 也进行了 NUMA Balancing 的重构和修正 [Reconcile NUMA balancing decisions with the load balancer](https://lore.kernel.org/patchwork/cover/1199507).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2019/10/18 | Mel Gorman | [Reconcile NUMA balancing decisions with the load balancer](https://linuxplumbersconf.org/event/4/contributions/480) | NUMA Balancing 和 Load Blance 经常会做出一些相互冲突的决策(任务放置和页面迁移), 导致不必要的迁移, 这组补丁减少了冲突 | v4 ☑ | [LWN](https://lwn.net/Articles/793427), [PatchWork](https://lore.kernel.org/patchwork/cover/1199507), [lkml](https://lkml.org/lkml/2019/10/18/676) |
| 2020/2/21 | Vincent Guittot | [remove runnable_load_avg and improve group_classify](https://lore.kernel.org/patchwork/cover/1198654) | 重构组调度的 PELT 跟踪, 在每次更新平均负载的时候, 更新整个 CFS_RQ 的平均负载| V10 ☑ 5.7 | [PatchWork](https://lore.kernel.org/patchwork/cover/1198654), [lkml](https://lkml.org/lkml/2020/2/21/1386) |
| 2020/02/03 | Mel Gorman | [Accumulated fixes for Load/NUMA Balancing reconcilation series](https://lore.kernel.org/patchwork/cover/1203922) | 解决一个负载平衡问题 | | [PatchWork](https://lore.kernel.org/patchwork/cover/1203922) |
| 2020/02/03 | Mel Gorman | [Revisit NUMA imbalance tolerance and fork balancing](https://lore.kernel.org/patchwork/cover/1340184) | 解决一个负载平衡问题<br>之前解决 NUMA Balancing 和 Load Balancing 被调和时, 尝试允许在一定程度的不平衡, 但是也引入了诸多问题. 因此当时限制不平衡只允许在几乎空闲的 NUMA 域. 现在大多数问题都已经解决掉了, 现在允许不平衡扩大一定的范围. 同时该补丁还解决了 fork 时候 balance 的问题 | RFC ☑ 5.11-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1340184) |


此外 load_balance 上还有一些接口层次的变化

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2020/03/11 | Valentin Schneider <valentin.schneider@arm.com> | [sched: Streamline select_task_rq() & select_task_rq_fair()](https://lore.kernel.org/patchwork/patch/1208449) | 选核流程上的重构和优化, 当然除此之外还做了其他操作, 比如清理了 sd->flags 信息, 甚至 sysfs 接口都变成只读了 | | |
| 2020/03/11 | Valentin Schneider <valentin.schneider@arm.com> | [sched: Instrument sched domain flags](https://lore.kernel.org/patchwork/cover/1224722) | 基于上一组补丁, 重构了 SD_FLAGS 的定义 | v4 ☑ 5.10-rc1 | [PatchWork v5 00/17](https://lore.kernel.org/patchwork/cover/1224722) |

## 4.5 load_balance 的其他优化
-------

Mel Gorman 深耕与解决 load_balance 以及 wake_affine 流程中一些不合理的行为.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2017/12/18 | Mel Gorman | [Reduce scheduler migrations due to wake_affine](https://lore.kernel.org/patchwork/cover/864391) | 减少 wake_affine 机制和 load_balance 的冲突 | | [PatchWork](https://lore.kernel.org/patchwork/cover/864391) |
| 2018/01/30 | Mel Gorman | [Reduce migrations and unnecessary spreading of load to multiple CPUs](https://lore.kernel.org/patchwork/cover/878789) | 减少不合理的迁移 | v1 ☑ 4.16-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/878789) |
| 2018/02/12 | Mel Gorman | [Reduce migrations due to load imbalance and process exits](https://lore.kernel.org/patchwork/cover/886577) | 优化退出场景时的不合理迁移行为 | v1 ☑ 4.17-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/886577) |
| 2018/02/12 | Mel Gorman | [Stop wake_affine fighting with automatic NUMA balancing](https://lore.kernel.org/patchwork/cover/886622) | 处理 NUMA balancing 和 wake_affine 的冲突 | v1 | [PatchWork](https://lore.kernel.org/patchwork/cover/886622) |
| 2018/02/13 | Mel Gorman | [Reduce migrations and conflicts with automatic NUMA balancing v2](https://lore.kernel.org/patchwork/cover/886940) | 处理 NUMA balancing 与负载均衡的冲突 | v1 | [PatchWork](https://lore.kernel.org/patchwork/cover/886940) |

Vincent Guittot 深耕与解决 load_balance 各种疑难杂症和不均衡状态, 特别是优化他对终端等嵌入式场景的性能.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2017/12/18 | Vincent Guittot | [sched/fair: Improve fairness between cfs tasks](https://lore.kernel.org/patchwork/cover/1308748) | 当系统没有足够的周期用于所有任务时, 调度器必须确保在CFS任务之间公平地分配这些cpu周期. 某些用例的公平性不能通过在系统上静态分配任务来解决, 需要对系统进行周期性的再平衡但是, 这种动态行为并不总是最优的, 也不总是能够确保CPU绑定的公平分配. <br>这组补丁通过减少选择可迁移任务的限制来提高公平性. 这个更改可以降低不平衡阈值, 因为  1st LB将尝试迁移完全匹配不平衡的任务.  | v1 ☑ [5.10-rc1](https://kernelnewbies.org/Linux_5.10#Memory_management) | [PatchWork](https://lore.kernel.org/patchwork/cover/1308748) |
| 2021/01/06 | Vincent Guittot | [Reduce number of active LB](https://lore.kernel.org/patchwork/cover/1361676) | 减少 ACTIVE LOAD_BALANCE 的次数 | v2 ☑ 5.12-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1361676) |
| 2021/01/06 | Vincent Guittot | [move update blocked load outside newidle_balance](https://lore.kernel.org/patchwork/cover/1383963) |oel报告了newidle_balance中的抢占和irq关闭序列很长, 因为大量的CPU cgroup正在使用, 并且需要更新. 这个补丁集将更新移动到newidle_imblance之外. 这使我们能够在更新期间提前中止. 在选择并进入空闲状态之前, 触发CPU空闲线程中的统计数据更新, 而不是踢一个正常的ILB来唤醒已经处于空闲状态的CPU.  | v2 ☑ 5.10-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1361676) |

此外 update_blocked_averages 也是内核调度器一个非常耗时的操作.
最早的时候, update_blocked_averages 是一个非常严重的性能瓶颈, [OOM and High CPU utilization in update_blocked_averages because of too many cfs_rqs in rq->leaf_cfs_rq_list](https://bugs.launchpad.net/ubuntu/+source/linux/+bug/1747896). 原因在于 rq->leaf_cfs_rq_list 是当前 CPU 上所有处于活动状态的 cfs_rq 的有序队列, update_blocked_averages 大多数 cgroup 中启用 CPU 控制器的系统中的负载均衡路径中, CPU消耗较少, 调度延迟增加. 特别是在临时创建了多个 cgroup 分组(称为 cgroup 泄漏) 的特殊情况下, 这会导致内核在运行 update_blocked_average() 耗时过程, 每次运行都要花费数毫秒. perf top 查看, update_blocked_averages 函数的热点占比甚至达到 30% 左右.

为此主线体现了解决思路, 通过删除空的和完全衰减的 cfs_rq 来减低遍历的层次和开销.

*   在 update_blocked_averages 中如果发现当前 CFS_RQ 是 decayed 的, 则直接将其移出

*   由于在遍历的过程中, 可能会遇到删除链表的情况, 因此 for_each 需要使用 safe 接口.

修复补丁 4.13-rc1 合入主线, [sched/fair: Fix O(nr_cgroups) in load balance path](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a9e7f6544b9cebdae54d29f87a7ba2a83c0471b5).

但是后来 4.19 测试过程中触发了一个调度的 BUG. 修复补丁发到社区 [sched: fix infinity loop in update_blocked_averages](https://lkml.org/lkml/2018/12/26/541), 但是补丁的修改只是规避了问题, 并没有解决问题. Linus Torvalds 本着让大家好好过圣诞的心情, 直接回退了引起问题的优化补丁 [sched/fair: Fix infinite loop in update_blocked_averages() by reverting a9e7f6544b9c](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c40f7d74c741a907cfaeb73a7697081881c497d0).


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2017/04/24 | Tejun Heo | [sched/fair: Fix O(nr_cgroups) in load balance path](https://lore.kernel.org/patchwork/patch/783385) | 优化 update_blocked_averages 的负载  | v1 ☑ 4.13-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/783385) |
| 2019/02/06 | Vincent Guittot | [sched/fair: Fix O(nr_cgroups) in load balance path]()](https://lore.kernel.org/patchwork/cover/1039223) | 优化 update_blocked_averages 的负载 | v1 ☑ 5.1-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/1039223) |
| 2017/12/18 | Vincent Guittot | [move update blocked load outside newidle_balance](https://lore.kernel.org/patchwork/cover/1383963) | Joel报告了 newidle_balance 中的抢占和irq关闭序列很长, 因为大量的 CPU cgroup 正在使用, 并且需要更新. 这个补丁集将更新 update_blocked_average 移动到 newidle_imblance 之外. | v2 ☐ | [PatchWork](https://lore.kernel.org/patchwork/cover/1383963) |



## 4.6 idle balance
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2013/08/29 | Jason Low <jason.low2@hp.com> | [sched: Limiting idle balance](https://lore.kernel.org/patchwork/patch/403138) | 限制 idle balance  | v1 ☑ 4.13-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/403138) |
| 2021/10/04 | Vincent Guittot <vincent.guittot@linaro.org> | [sched/fair: Improve cost accounting of newidle_balance](https://lore.kernel.org/patchwork/patch/403138) | 通过考虑更新阻塞负载 update_blocked_averages() 所花费的时间, 在没有机会运行至少一个负载平衡循环的情况下完全跳过负载平衡循环. 因此在 newidle_balance()中, 当 this_rq 的第一个 sd 满足 `this_rq->avg_idle < sd->max_newidle_lb_cost` 时, 认为执行 update_blocked_averages() 是非常昂贵且没有收益的, 只会增加开销. 因此在 newidle_balance() 中尽早检查条件, 尽可能跳过 update_blocked_averages() 的执行. | v1 ☐ | [PatchWork](https://lore.kernel.org/lkml/20211004171451.24090-1-vincent.guittot@linaro.org), [LKML](https://lkml.org/lkml/2021/10/4/1188) |


## 4.7 active load_balance
-------


最早的时候, Valentin Schneider 阅读代码发现, active load_balance 和 sched_switch 之间存在一个 race, 因为当前 CPU/RQ 永远无法感知到远程的 RQ 上正在运行着什么样的进程, 因此可能会发生,  cpu_stopper 触发 migration/N 抢占高优先级进程(比如 RT) 的情况. 并提出了自己的优化方案 [sched/fair: Active balancer RT/DL preemption fix](https://lore.kernel.org/patchwork/patch/1115663).

后来 Yafang Shao 上报了一个现象, 经过分析, 就是 Valentin Schneider 之前走读代码发现的那种情况, [migration 进程抢占了一个 RT 进程](https://lkml.org/lkml/2021/6/14/204). 它尝试直接规避这种方式, [阻止抢占非 CFS 的高优先级进程](https://lore.kernel.org/patchwork/patch/1446860)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2021/06/02 | Valentin Schneider <valentin.schneider@arm.com> | [sched/fair: Active balancer RT/DL preemption fix](https://lore.kernel.org/patchwork/patch/1115663) | NA | v2 ☐ | [PatchWork v1](https://lore.kernel.org/patchwork/patch/1115663) |
| 2021/06/02 | Yafang Shao <laoar.shao@gmail.com> | [sched, fair: try to prevent migration thread from preempting non-cfs task](https://lore.kernel.org/patchwork/patch/1440172) | 规避问题 [a race between active_load_balance and sched_switch](https://lkml.org/lkml/2021/6/14/204), []() | v1 ☐ | [PatchWork v1 old](https://lore.kernel.org/patchwork/patch/1440172), [PatchWork v1](https://lore.kernel.org/patchwork/patch/1446860) |


## 4.8 WAKEUP
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2021/09/20 | Mel Gorman <mgorman@techsingularity.net> | [Scale wakeup granularity relative to nr_running](https://lore.kernel.org/lkml/20210920142614.4891-1-mgorman@techsingularity.net) | 在任务迁移或唤醒期间, 将决定是否抢占当前任务. 为了限制过度调度, 可以通过设置 sysctl_sched_wakeup_granularity 来延迟抢占, 以便在抢占之前允许至少一定的运行时间. 但是, 当任务堆叠而造成域严重过载时(例如 hackbench 测试), 过度调度的程度仍然很严重. 而且由于许多时间被调度器浪费在重新安排任务(切换等)上, 这会进一步延长过载状态. 这组补丁根据 CPU 上正在运行的任务数在 wakeup_gran() 中扩展唤醒粒度, 默认情况下最大可达 8ms. 其目的是允许任务在过载时运行更长时间, 以便某些任务可以更快地完成, 并降低域过载的程度. | v1 ☐ | [PatchWork v1](https://lore.kernel.org/lkml/20210920142614.4891-1-mgorman@techsingularity.net), [LKML](https://lkml.org/lkml/2021/9/20/478) |


# 5 select_task_rq
-------

调度器最核心的工作就两个内容 :

- [x] **选进程**: 选择下一个更合适的进程 **pick_next_task**
- [x] **选核**: 为某个进程选择更合适的 CPU 运行 **select_task_rq**

所有其他的机制都是直接或者间接服务这两个终极任务的.

1.  Linux 下各个调度类都实现了这两个接口, 各个调度类按照既定的算法和规则运行, 在选进程这方面, 各个调度类的算法很成熟, 而且经历的改动较小.

2.  调度的目标是提升系统整体的性能或者降低功耗, 从这些目标上看, 在**选进程**算法既定的情况下, 第二个任务**选核**就会显的更重要也更复杂一些, 选核选的好, 不光能及早的投入运行, 而且在运行时能充分利用 cache 等硬件优势, 让进程飞一般的运行.

3.  此外现在调度器还有一个附加功能, 为了提升系统整体的吞吐量和利用率, 大多数调度器都提供了 **load_balance 负载均衡** 的功能.

因此我们将这几个工作称为调度器的核心功能. 而调度器的性能优化也都围绕这几个方面来进行.
而其中对性能影响最大的, 也是最关键的任务就是 select_task_rq, 其次是 load_balance, 而 pick_next_task 的工作, 更多的是体现在公平性和扩展性方面.
那么下面我们就看看内核调度中, 近几年这部分的优化工作.


内核中主体的进程都是以 SCHED_NORMAL 为策略的普通 CFS 进程, 以 select_task_rq_fair 为例, 其代码就经过了不断的重构和优化.



## 5.1 WAKE_AFFINE & WAKEUP 优化
-------

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2013/07/04 | Michael wang | [sched: smart wake-affine](https://lore.kernel.org/patchwork/cover/390846) | 引入wakee 翻转次数, 通过巧妙的启发式算法, 识别系统中 1:N/N:M 等唤醒模型, 作为是否进行 wake_affine 的依据 | v3 ☑ 3.12-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/390846)<br>*-*-*-*-*-*-*-* <br>[commit 1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=62470419e993f8d9d93db0effd3af4296ecb79a5), [commit2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7d9ffa8961482232d964173cccba6e14d2d543b2) |
| 2017/12/18 | Mel Gorman | [Reduce scheduler migrations due to wake_affine](https://lore.kernel.org/patchwork/cover/864391) | 优化 wake_affine 减少迁移次数 | | [PatchWork](https://lore.kernel.org/patchwork/cover/864391) |
| 2018/01/30 | Mel Gorman | [Reduce migrations and unnecessary spreading of load to multiple CPUs](https://lore.kernel.org/patchwork/cover/878789) | 减少不合理的迁移 | v1 ☑ 4.16-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/878789) |
| 2020/05/24 | Mel Gorman | [Optimise try_to_wake_up() when wakee is descheduling](https://lore.kernel.org/patchwork/cover/1246560) | 唤醒时如果 wakee 进程正在睡眠或者调度(释放 CPU), 优化在 on_cpu 的自旋等待时间 | v1 ☑ 5.8-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1246560) |
| 2021/05/13 |  Srikar Dronamraju <srikar@linux.vnet.ibm.com> | [sched/fair: wake_affine improvements](https://lore.kernel.org/patchwork/cover/1416963) | 通过根据 LLC 域内 idle CPU 的信息, 优化 wake_affine, 如果当前待选的 LLC 域没有空闲 CPU, 尝试从之前的 LLC 域中选择. | v3 ☐ | [PatchWork](https://lore.kernel.org/patchwork/cover/1428244) |


TencentOS-kernel 回合了主线 wake_affine 中几个优化迁移的补丁, 可以 [kernel-4.14 修复wake affine进程导致性能降低的问题](https://github.com/Tencent/TencentOS-kernel/commit/985a0aad220cec1e43a35432b25dbbdb31b975ba), [kernel-5.4](https://github.com/Tencent/TencentOS-kernel/commit/822a50c9e70205cbc29fb97d72c26c7a51b58a1d)

## 5.3 SELECT_CPU 的优化是调度优化永恒不变的命题
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:----:|:---:|:----------:|:---:|
| 2016/05/06 | Peter Zijlstra | [sched: Rewrite select_idle_siblings()](https://lore.kernel.org/patchwork/cover/677017) | 通过这个补丁, 将 select_idle_siblings 中对 sched_domain 上 CPU 的单次扫描替换为 3 个显式的扫描.<br>1. select_idle_core 在 LLC 域内搜索一个空闲的 CORE<br>2. select_idle_cpu 在 LLC 域内搜索一个空闲的 CPU.<br>3. select_idle_smt 在目标 CORE 中搜索一个空闲的 CPU. | RFC ☑ 4.9-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/677017) |
| 2021/04/20 | Song Bao Hua (Barry Song) | [scheduler: expose the topology of clusters and add cluster scheduler](https://lore.kernel.org/patchwork/cover/1415806) | 增加了 cluster 层次的 CPU select. 多个架构都是有 CLUSTER 域的概念的, 比如 Kunpeng 920 一个 NODE(DIE) 24个 CPU 分为 8 个 CLUSTER, 整个 DIE 共享 L3 tag, 但是一个 CLUSTER 使用一个 L3 TAG. 这种情况下对于有数据共享的进程, 在一个 cluster 上运行, 通讯的时延更低. | RFC v6 ☐ | [PatchWork](https://lore.kernel.org/patchwork/cover/1415806) |



## 5.4 提升 CPU 的查找效率
-------

每次为进程选择一个合适的 CPU 的时候, 较好的情况可以通过 wake_affine 等走快速路径, 但是最坏的情况下, 却不得不遍历当前 SD 查找一个 IDLE CPU 或者负载较小的 CPU.
这个查找是一项大工程, 在调度里面几乎是觉难以容忍的. 因此这里一直是性能优化的战场, 炮火味十足.

2020 年 12 月 15 日, 调度的大 Maintainer Peter Zijlstra, 曾公开抨击选核的慢速流程里面[部分代码, "The thing is, the code as it exists today makes no sense what so ever. It's plain broken batshit."](https://lkml.org/lkml/2020/12/15/93).

### 5.4.1 select_idle_sibling rework
-------



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:----:|:---:|:----------:|:---:|
| 2018/05/30 | Peter Zijlstra | [select_idle_sibling rework](https://lore.kernel.org/patchwork/patch/911697) | 优化 select_idle_XXX 的性能 | RFC | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/911697) |
| 2019/7/1 | Subhra Mazumdar | [Improve scheduler scalability for fast path](https://lore.kernel.org/patchwork/cover/1094549) | select_idle_cpu 每次遍历 LLC 域查找空闲 CPU 的代价非常高, 因此通过限制搜索边界来减少搜索时间, 进一步通过保留 PER_CPU 的 next_cpu 变量来跟踪上次搜索边界, 来缓解此次优化引入的线程局部化问题  | v3 ☐ | [LWN](https://lwn.net/Articles/757379/), [PatchWork](https://lore.kernel.org/patchwork/cover/1094549/), [lkml](https://lkml.org/lkml/2019/7/1/450), [Blog](https://blogs.oracle.com/linux/linux-scheduler-scalabilty-like-a-boss) |
| 2018/01/30 | Mel Gorman | [Reduce migrations and unnecessary spreading of load to multiple CPUs](https://lore.kernel.org/patchwork/cover/878789) | 通过优化选核逻辑, 减少不必要的迁移 | | |
| 2019/1/21 | Srikar Dronamraju | [sched/fair: Optimize select_idle_core](https://lore.kernel.org/patchwork/patch/1163807) | | | |
| 2019/07/08 | Parth Shah | [Optimize the idle CPU search](https://lore.kernel.org/patchwork/patch/1098092) | 通过标记 idle_cpu 来降低 select_idle_sibling/select_idle_cpu 的搜索开销 | | |
| 2020/04/15 | Valentin Schneider | [sched: Streamline select_task_rq() & select_task_rq_fair()](https://lore.kernel.org/patchwork/patch/1224733) | 选核流程上的重构和优化, 当然除此之外还做了其他操作, 比如清理了 sd->flags 信息, 甚至 sysfs 接口都变成只读了 | v3 ☑ 5.8-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1224733) |
| 2021/03/26 | Rik van Riel | [Throttle select_idle_sibling when a target domain is overloaded](https://lore.kernel.org/patchwork/patch/1213189) | 这是 CPU/NUMA 负载平衡器协调的后续工作. 补丁中做了如下的优化<br>1. 补丁1-2 添加了 schedstats 来跟踪 select_idle_sibling() 的效率. 默认是禁用的<br>2. 补丁3 是一个细微的优化, 如果一个 CPU 已经找到, 避免清除部分 cpumask.<br>3. 补丁 4 跟踪在 select_idle_cpu() 期间域是否出现重载, 以便将来的扫描可以在必要时尽早终止. 这减少了在域超载时无用扫描的运行队列数量. | v2 ☐ | [PatchWork](https://lore.kernel.org/patchwork/patch/1213189) |
| 2021/03/26 | Rik van Riel | [sched/fair: bring back select_idle_smt, but differently](https://lore.kernel.org/patchwork/patch/1402916) | Mel Gorman 上面的补丁在 9fe1f127b913("sched/fair: Merge select_idle_core/cpu()") 中做了一些出色的工作, 从而提高了内核查找空闲cpu的效率, 可以有效地降低任务等待运行的时间. 但是较多的均衡和迁移, 减少的局部性和相应的 L2 缓存丢失的增加会带来一些负面的效应. 这个补丁重新将 `select_idle_smt` 引入回来, 并做了优化, 修复了性能回归, 在搜索所有其他 CPU 之前, 检查 prev 的兄弟 CPU 是否空闲. | v2 ☐ | [PatchWork](https://lore.kernel.org/patchwork/patch/1402916) |

### 5.4.2 SIS_AVG_CPU
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:----:|:---:|:----------:|:---:|
| 2020/12/03 | Mel Gorman | [Reduce time complexity of select_idle_sibling](https://lore.kernel.org/patchwork/cover/1348877) | 通过自己完善的 schedstat 的统计信息, 发现 select_idle_XXX 中不合理的地方(提高了p->recent_used_cpu的命中率. 以减少扫描开销, 同时如果在扫描a时发现了一个候选, 那么补丁4将返回一个空闲的候选免费的核心等), 降低搜索开销 | RFC v3 ☐  | 这组补丁其实有很多名字, 作者发了几版本之后, 不断重构, 也改了名字<br>*-*-*-*-*-*-*-* <br>2020/12/03 [Reduce time complexity of select_idle_sibling RFC,00/10](https://lore.kernel.org/patchwork/cover/1348877)<br>*-*-*-*-*-*-*-* <br>2020/12/07 RFC [Reduce worst-case scanning of runqueues in select_idle_sibling 0/4](https://lore.kernel.org/patchwork/cover/1350248)<br>*-*-*-*-*-*-*-* <br>2020/12/08 [Reduce scanning of runqueues in select_idle_sibling 0/4](https://lore.kernel.org/patchwork/patch/1350876)<br>*-*-*-*-*-*-*-* <br>后面换标题名重发 [Scan for an idle sibling in a single pass](https://lore.kernel.org/patchwork/cover/1371921) |
| 2020/12/08 | Mel Gorman | [Scan for an idle sibling in a single pass](https://lore.kernel.org/patchwork/cover/1371921) |  将上面一组补丁 [Reduce time complexity of select_idle_sibling](https://lore.kernel.org/patchwork/cover/1348877), 基于上面 Peter 的补丁的思路进行了重构, 减少 select_idle_XXX 的开销<br>1. 优化了 IDLE_CPU 扫描深度的计算方法<br>2. 减少了 CPU 的遍历次数, 重构了 sched_idle_XXX 函数<br>3. 将空闲的核心扫描调节机制转换为SIS_PROP, 删除了 SIS_PROP_CPU. | v5 ☑ 5.12-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1371921) |

### 5.4.2 SIS_PROP
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:----:|:---:|:----------:|:---:|
| 2020/12/14 | Peter Zijlstra | [select_idle_sibling() wreckage](https://lore.kernel.org/patchwork/cover/1353496) | 重构 SIS_PROP 的逻辑, 重新计算 CPU 的扫描成本, 同时归一 select_idle_XXX 中对 CPU 的遍历, 统一选核的搜索逻辑来降低开销, 提升性能 | RFC ☐ | [PatchWork](https://lore.kernel.org/patchwork/cover/1353496), [LKML](https://lkml.org/lkml/2020/12/14/560) |
| 2020/12/08 | Mel Gorman | [Scan for an idle sibling in a single pass](https://lore.kernel.org/patchwork/cover/1371921) |  基于上面 Peter 的补丁的思路进行了重构, 减少 select_idle_XXX 的开销<br>1. 优化了 IDLE_CPU 扫描深度的计算方法<br>2. 减少了 CPU 的遍历次数, 重构了 sched_idle_XXX 函数<br>3. 将空闲的核心扫描调节机制转换为SIS_PROP, 删除了 SIS_PROP_CPU | v5 ☑ 5.12-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1371921) |
| 2021/03/26 | Rik van Riel | [sched/fair: bring back select_idle_smt, but differently](https://lore.kernel.org/patchwork/patch/1402916) | Mel Gorman 上面的补丁在 9fe1f127b913("sched/fair: Merge select_idle_core/cpu()") 中做了一些出色的工作, 从而提高了内核查找空闲cpu的效率, 可以有效地降低任务等待运行的时间. 但是较多的均衡和迁移, 减少的局部性和相应的 L2 缓存丢失的增加会带来一些负面的效应. 这个补丁重新将 `select_idle_smt` 引入回来, 并做了优化, 修复了性能回归, 在搜索所有其他 CPU 之前, 检查 prev 的兄弟 CPU 是否空闲. | v2 ☐ | [PatchWork](https://lore.kernel.org/patchwork/patch/1402916) |
| 2021/07/26 | Mel Gorman <mgorman@techsingularity.net> | [Modify and/or delete SIS_PROP](https://lore.kernel.org/patchwork/cover/1467090) | NA | RFC | [PatchWork RFC,0/9](https://lore.kernel.org/patchwork/cover/1467090) |
| 2021/08/04 | Mel Gorman <mgorman@techsingularity.net> | [Reduce SIS scanning](https://lore.kernel.org/patchwork/cover/1472054) | 将 [Modify and/or delete SIS_PROP](https://lore.kernel.org/patchwork/cover/1467090) 拆开进行提交. | RFC | [PatchWork 0/2](https://lore.kernel.org/patchwork/cover/1472054) |



### 5.4.3 avg_idle bypass
-------

avg_idle 可以反应目前 RQ 进入 idle 的时间长短.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:----:|:---:|:----------:|:---:|
| 2020/03/30 | Valentin Schneider | [sched: Align rq->avg_idle and rq->avg_scan_cost](https://lore.kernel.org/patchwork/patch/1217584) | NA | RFC | [2021/06/15 v2](https://lore.kernel.org/patchwork/patch/1217584) |
| 2021/06/15 | Peter Zijlstra | [sched/fair: Age the average idle time](https://lore.kernel.org/patchwork/patch/1446838) | NA | RFC | [2021/06/15 v2](https://lore.kernel.org/patchwork/patch/1446838) |



# 6 pick_next_task
-------

5d7d605642b2 sched/core: Optimize pick_next_task()
678d5718d8d0 sched/fair: Optimize cgroup pick_next_task_fair()
27e3e60 sched: Optimize branch hint in pick_next_task_fair()
9ee576809d8 sched/core: Optimize pick_next_task() for idle_..

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2017/01/19 | Peter Zijlstra | [sched/core: Optimize pick_next_task() for idle_sched_class](https://lkml.org/lkml/2017/1/19/687) | NA | | v1 ☑ 4.11-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/755854), [LKML](https://lkml.org/lkml/2017/1/19/687), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=49ee576809d837442624ac18804b07943267cd57) |


# 7 调度与节能
-------


从节能角度讲, 如果能维持更多的 CPU 处于深睡眠状态, 仅保持必要数目的 CPU 执行任务, 就能更好地节约电量, 这对笔记本电脑来说, 尤其重要．然而这不是一个简单的工作, 这涉及到负载均衡, 调度器, 节能模块的并互, Linux 调度器中曾经有相关的代码, 但后来发现问题, 在 3.5, 3.6 版本中, 已经把相关代码删除．整个问题需要重新思考．



在前不久, 一个新的 patch 被提交到 Linux 内核开发邮件列表, 这个问题也许有了新的眉目, 到时再来更新此小节．可阅读此文章: [Steps toward power-aware scheduling [LWN.net](https://lwn.net/Articles/655479/)


## 7.1 小任务封包
-------

小任务封包(Small Task Packing) 是内核调度特性中少数几个隔几年就会被人换个马甲发出来的特性之一.

早在 2012 年, Linaro 的 Vincent Guittot 发出了一版 [Small-task packing](https://lwn.net/Articles/520857) 的补丁, 通过将负载较轻的任务打包在尽量少的 CPU 上, 从而在保证系统性能的前提下, 尽可能的降低功耗.
该补丁最终发到 v5 版本 [PatchWork](https://lore.kernel.org/patchwork/cover/414759), [lkml](https://lkml.org/lkml/2013/10/18/121), [git-sched-packing-small-tasks-v5](http://git.linaro.org/people/vincent.guittot/kernel.git/log/?h=sched-packing-small-tasks-v5) 后, 没有了下文.

但是请大家记住这个特性, 这个特性将在后面的特性中不断被提及并实现. 它作为一个先驱者, 为终端功耗场景在调度领域做了最初的一次尝试. 自此开始调度领域一场旷日持久的性能 vs 功耗的战役被打响.


## 7.2 能耗感知的调度器
-------

在 Linaro 开发小任务封包的同时, 2012 年 Intel 的 Alex Shi 发起的讨论中, 提出了更高大上的概念[**能耗感知的调度器**](https://lkml.org/lkml/2012/8/13/139), 功耗和性能本身就是一个矛盾的统一体, 因此由于此特性节省的功耗和降低的性能不成正比, 因此在发到 v7 [PatchWork](https://lore.kernel.org/patchwork/cover/370834) 之后也寿终正寝.


功耗感知的调度器是如此的新颖, 以至于 2013 年 10 月, 致力于 ANDROID 调度器优化的 Morten Rasmussen [sched: Power scheduler design proposal](https://lore.kernel.org/patchwork/cover/391530) 也提出了自己的想法, 在发到了 [v2 PatchWork](https://lore.kernel.org/patchwork/cover/412619),在这之前他刚发出了他 HMP 的补丁.

接着 2014 年, Preeti U Murthy 在总结了 Alex Shi 的经验之后, 接着完善了能耗感知调度器的设计, 但是由于缺少热度, 这个特性最终止步 v2 [Power Scheduler Design](https://lore.kernel.org/patchwork/cover/491059) 版本.

但是不可否认, 能耗感知的调度器提出了很多新颖的想法, 他将会后来的调度器知名一个新的方向.


>2014/08/27, 2014Jonathan Corbet, [The power-aware scheduling miniconference](https://lwn.net/Articles/609561)
>
>2015/08/25, By Jonathan Corbet, [Steps toward power-aware scheduling](https://lwn.net/Articles/655479)
>
>[Another attempt at power-aware scheduling](https://lwn.net/Articles/600419)

## 7.3 IKS -> HMP -> EAS
-------


[The power-aware scheduling mini-summit](https://lwn.net/Articles/571414)

ARM 的 Morten Rasmussen 一直致力于ANDROID 调度器优化的:

1.  最初版本的 IKS 调度器, [ELC: In-kernel switcher for big.LITTLE](https://lwn.net/Articles/539840)

2.  以及后来先进的 HMP 调度器 [LC-Asia: A big LITTLE MP update](https://lwn.net/Articles/541005), 虽然我们称呼他是一个调度器, 但是其实他的策略只是一个**进程放置(Task Placement, 请原谅我的这个翻译, 很抱歉我实在不知道翻译成什么好, 如果您有什么好的建议, 请联系我)** 的策略. [sched: Task placement for heterogeneous MP systems](https://lore.kernel.org/patchwork/cover/327745)

3.  直到看到了能耗感知调度器, 忽然眼前有了光, Morten 充分借鉴了能耗感知调度器的思想, 提出了自己的想法[Another attempt at power-aware scheduling](https://lwn.net/Articles/600419), 最终发布到 v2 版本 [Power-aware scheduling v2](https://lore.kernel.org/patchwork/cover/412619).

4.  在 [Power-aware scheduling v2](https://lore.kernel.org/patchwork/cover/412619) 发布之后 版本之后. Morten 有了更多的奇思妙想, 提出了能耗模型(energy cost model) 的雏形, 以及 energy diff 的思想, 从此一发不可收拾. 可以认为这个就是 EAS 最早的雏形, 并最终发到了 2015/07/07 v5 版本 [sched: Energy cost model for energy-aware scheduling](https://lore.kernel.org/patchwork/cover/576661).

5.  EAS 带来了划时代的想法, 最终 [Quentin Perret](http://www.linux-arm.org/git?p=linux-qp.git;a=summary) 接手了 Morten Rasmussen 的工作, 最终在 2018/10/03 v10 版本将 EAS 合入主线 [https://lore.kernel.org/patchwork/cover/1020432/](https://lore.kernel.org/patchwork/cover/1020432)




## 7.4 异构调度优化
-------

后续基于 EAS 还做了很多优化

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2016/10/14 | Vincent Guittot | [sched: Clean-ups and asymmetric cpu capacity support](https://lore.kernel.org/patchwork/cover/725608) | 调度程序目前在非对称计算能力的系统上没有做太多的工作来提高性能(读ARM big.LITTLE). 本系列主要通过对任务唤醒路径进行一些调整来改善这种情况, 这些调整主要考虑唤醒时的计算容量, 而不仅仅考虑cpu对这些系统是否空闲. 在部分使用的场景中, 这为我们提供了一致的、可能更高的吞吐量. SMP的行为和性能应该是不受影响. <br>注意这组补丁是分批合入的 | v1 ☑ 4.10-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/725608) |
| 2018/03/15 | Morten Rasmussen | [sched/fair: Migrate 'misfit' tasks on asymmetric capacity systems](https://lore.kernel.org/patchwork/cover/933989) | 异构 CPU 上 wakeup 路径倾向于使用 prev CPU | v1 ☑ 4.20-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/933989) |
| 2018/12/03 | Quentin Perret | [Energy Aware Scheduling](https://lore.kernel.org/patchwork/cover/1020432) | 能效感知的调度器 EAS | v10 ☑ 5.0-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1020432) |
| 2019/08/22 | Patrick Bellasi | [Add utilization clamping support (CGroups API)](https://lore.kernel.org/patchwork/cover/1118345) | TASK util clamp(Android schedtune 的主线替代方案)  | v14 ☑ 5.4-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1118345) |
| 2020/02/06 | Vincent Guittot | [sched/fair: Capacity aware wakeup rework](https://lore.kernel.org/patchwork/cover/1190300) | 异构 CPU 上 wakeup 路径优化 | v4 ☑ 5.7-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1190300) |
| 2020/05/20 | Dietmar Eggemann | [Capacity awareness for SCHED_DEADLINE](https://lore.kernel.org/patchwork/cover/1245028) | DEADLINE 感知 Capacity | v3 ☐ 5.9-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1245028) |
| 2020/12/19 | Vincent Guittot | [sched/fair: prefer prev cpu in asymmetric wakeup path](https://lore.kernel.org/patchwork/cover/1329119) | 异构 CPU 上 wakeup 路径倾向于使用 prev CPU | v1 ☑ 5.10-rc4 | [PatchWork](https://lore.kernel.org/patchwork/cover/1308748) |
| 2021/03/11 | Valentin Schneider | [sched/fair: misfit task load-balance tweaks](https://lore.kernel.org/patchwork/cover/1393531) | 优化 misfit task 的一些逻辑 | v3 ☐ 5.10-rc4 | [PatchWork](https://lore.kernel.org/patchwork/cover/1393531) |
| 2021/04/07 | Valentin Schneider | [sched/fair: load-balance vs capacity margins](https://lore.kernel.org/patchwork/cover/1409479) | misfit task load-balance tweaks 的补丁被拆分重构, 这个是 Part 1 | v3 ☐ 5.10-rc4 | [PatchWork](https://lore.kernel.org/patchwork/cover/1409479) |
| 2021/04/16 | Valentin Schneider | [sched/fair: (The return of) misfit task load-balance tweaks](https://lore.kernel.org/patchwork/cover/1414181) | misfit task load-balance tweaks 的补丁被拆分重构, 这个是 Part 2 | v1 ☐ 5.10-rc4 | [PatchWork](https://lore.kernel.org/patchwork/cover/1414181) |
| 2021/05/10 | Valentin Schneider | [Rework CPU capacity asymmetry detection](https://lore.kernel.org/patchwork/cover/1424708) | 优化 misfit task 的一些逻辑 | v7 ☑ 5.14-rc1  | [PatchWork v1](https://lore.kernel.org/patchwork/cover/1414557)<br>*-*-*-*-*-*-*-* <br>[PatchWork v7](https://lore.kernel.org/patchwork/cover/1440770) |



异构拓扑优化

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2021/05/17 | Beata Michalska <beata.michalska@arm.com> | [Rework CPU capacity asymmetry detection](https://lore.kernel.org/patchwork/cover/1429483) | NA | v3 ☐ | [PatchWork v4](https://lore.kernel.org/patchwork/cover/1429483) |



## 7.5 基于调度器的调频
-------


不管是服务器领域, 还是终端以及嵌入式 rtos 等场景, 性能与功耗都是一个永恒的话题. 因此我们经常把它们合在一起称为"性能, 功耗与热". 而这些都离不开调度器的参与. 调度器通过之前提到的两个核心功能: 选核和选进程, 提升系统的性能和吞吐量, 再或者在满足性能的前提下通过功耗感知的调度来降低功耗. 之前我们提到的手段都是围绕着调度核心功能的. 通过选核让更多的 CPU 能够处于 IDLE 状态是可以降低功耗的, EAS 的实现已经与 CPUIDLE 紧密结合. 但是别忘了, 我们还有 DVFS, 通过调压调频也可以降低功耗. 因此 EAS 也提供了一些手段能够让调度器感知 CPUFREQ 框架.


1.  传统 CPU 调频策略

CPUFREQ 调频框架主要分为 3 块: CPUFreq 驱动和 CPUFreq 核心模块、CPUFreq Governor.

CPUFreq 驱动是处理和平台相关的逻辑, Governor 中实现了具体的调频策略, 核心模块是驱动和 Governor 连接的桥梁, 它是 CPUFreq 的抽象层, 主要是一些公共的逻辑和 API, 比如设置 CPU 的频率和电压.

为了适应不同的应用场景, CPUFreq 张抽象了多个调频 Governor. 比如有些场景需要性能最优, 那么不希望调频而是永远使用最高频, 那么就使用 Performance, 反之可以使用 Powersave. 除了这两种是固定频率外, 而其他一些策略都实现了类似于 快上快下, 快上慢下, 以及慢上快下等不同类型的策略. 而包含一个共同的部分 - 负载采样, 需要每隔一定时间就计算一次 CPU 负载. 由于他们是跟调度器分离开的, 无法感知调度的信息, 获取 CPU 的负载信息, 因此只能通过采样的方式来获取.

但是对于 CPU 的负载, 没有谁比调度器还清楚的了. 所以 cpufreq governor 完全没必要自己去做负载采样, 应该从内核调度器那里获取. 因此在 EAS 设计的早期, 基于调度器的 cpufreq governor 就是这样引出来的.

2.  调度器驱动的调频

当时内核社区中, 逐渐实现了两个成形的方案.

一个是 ARM 和 Linaro 主导的项目 - cpufreq_sched, 属于 EAS 的一部分.

而另外一个 Intel 主导的项目 - schedutil.

[cpufreq_sched](https://lkml.org/lkml/2016/2/22/1037) 本身逻辑比较简单, 当调度器发现进程运行时负载发生了变化, 就通过 update_cpu_capacity_request() 来更新当前 policy 下 CPU 的频率. 通过调度器直接更新频率, 自然响应速度快, 调频间隔短, 这固然是 cpufreq_sched 的优势, 但是把整个调频动作都放到调度器里做, 无疑会增加调度器的负担. 调度器代码路径变长, 也会增加调度器的延时. 如果某个平台的 cpufreq 驱动在设置 CPU 频率的时候会导致系统睡眠, 那么 cpufreq_sched 还需要在每一个 CPU 上额外开启一个线程, 防止对调度器造成影响. [scheduler-driven cpu frequency selection](https://lwn.net/Articles/649593)

而 [schedutil 实现成一个 Governor 的形式](https://lkml.org/lkml/2016/3/29/1041), 通过 [utilization update callback 机制](https://lkml.org/lkml/2016/2/15/734), 在 CPU 使用率变化时的注册回调, 那么就调度器期望进行调频的时候, 就通过回调通知 CPUFREQ 来进行调频. 其实跟 cpufreq_sched 大同小异, 但是由于其合理的架构, 因此最终在 4.7 时合入主线.

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----------:|:---:|
| 2016/02/23 | Steve Muckle 等 | [sched: scheduler-driven CPU frequency selection](https://lore.kernel.org/patchwork/cover/649930) | 调度器驱动的调频 cpufreq_sched | RFC v7 | [PatchWork](https://lore.kernel.org/patchwork/cover/649930), [lkml](https://lkml.org/lkml/2016/2/22/1037) |
| 2016/03/22 | Rafael J. Wysocki | [cpufreq: schedutil governor](https://lore.kernel.org/patchwork/cover/660587) | 基于 utilization update callback callback 的 schedutil 的调频 governor | v6 ☑ 4.7-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/660587), (https://lkml.org/lkml/2016/3/29/1041) |


3.  schedutil 后续优化
-------

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----------:|:---:|
| 2021/08/12 | Viresh Kumar <viresh.kumar@linaro.org> | [Add callback to register with energy model](https://lore.kernel.org/patchwork/cover/1424708) | 当前许多 cpufreq 驱动程序向每个策略的注册了能耗模型, 并通过相同的操作 dev_pm_opp_of_register_em() 来完成. 但是随着  thermal-cooling 的完善, 可以在 cpufreq 层次通过新的回调 register_em 来完成这个工作. | v3 ☐ | [PatchWork V3,0/9](https://patchwork.kernel.org/project/linux-arm-kernel/cover/cover.1628742634.git.viresh.kumar@linaro.org) |


## 7.6 freezer 冻结
-------

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2016/02/23 | Steve Muckle 等 | [freezer,sched: Rewrite core freezer logic](https://lore.kernel.org/patchwork/cover/1444882) | 重写冻结的核心逻辑, 从而使得 WRT 解冻的表现更加合理. 通过将 PF_FROZEN 替换为 TASK_FROZEN (一种特殊的块状态), 可以确保冻结的任务保持冻结状态, 直到明确解冻, 并且不会像目前可能的那样过早随机唤醒. | v2 | [2021/06/01 RFC](https://lore.kernel.org/patchwork/cover/1439538)<br>*-*-*-*-*-*-*-* <br>[2021/06/01 RFC](https://lore.kernel.org/patchwork/cover/1444882) |



# 8 实时性 linux PREEMPT_RT
-------


## 8.1 抢占支持(preemption)
-------

**2.6 时代开始支持** (首次在2.5.4版本引入[<sup>37</sup>](#refer-anchor-37), 感谢知友 [@costa](https://www.zhihu.com/people/78ceb98e7947731dc06063f682cf9640) 考证! 关于 Linux 版本规则,  可看我文章[<sup>4</sup>](#refer-anchor-4).


可抢占性, 对一个系统的调度延时具有重要意义. 2.6 之前, 一个进程进入内核态后, 别的进程无法抢占, 只能等其完成或退出内核态时才能抢占, 这带来严重的延时问题, 2.6 开始支持内核态抢占.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2021/01/18 | Frederic Weisbecker & Peter Zijlstra 等 | [preempt: Tune preemption flavour on boot v4](https://lore.kernel.org/patchwork/cover/1366962) | 增加了 PREEMPT_DYNAMIC 配置选项, 允许内核启动阶段选择使用哪种抢占模式(none, voluntary, full) 等, 同时支持 debugfs 中提供开关, 在系统运行过程中动态的修改这个配置. | RFC v4 ☑ 5.12-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1366962) |



## 8.2 NO_HZ
-------

NOHZ 社区测试用例 [frederic/dynticks-testing.git](https://git.kernel.org/pub/scm/linux/kernel/git/frederic/dynticks-testing.git)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2021/01/22 | Joel Fernandes (Google)" <joel@joelfernandes.org> | [sched/fair: Rate limit calls to update_blocked_averages() for NOHZ](https://lore.kernel.org/patchwork/patch/1369598) | 在运行ChromeOS Linux kernel v5.4 的 octacore ARM64 设备上, 发现有很多对 update_blocked_average() 的调用, 导致调度的开销增大, 造成 newilde_balance 有时需要最多500微秒. 我在周期平衡器中也看到了这一点. 将 update_blocked_average() 调用速率限制为每秒 20 次 | v1 ☐ | [PatchWork](https://lore.kernel.org/patchwork/cover/1369598) |
| 2021/08/23 | Valentin Schneider <valentin.schneider@arm.com> | [sched/fair: nohz.next_balance vs newly-idle CPUs](https://lore.kernel.org/patchwork/patch/1480299) | NA | v3 ☐ | [2021/07/19 v2,0/2](https://lore.kernel.org/patchwork/cover/1462201)<br>*-*-*-*-*-*-*-* <br>[2021/08/23 PatchWork v3,0/2](https://lore.kernel.org/patchwork/cover/1480299) |


## 8.3 task/CPU 隔离
-------



### 8.3.1 降噪
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2017/11/27 | Frederic Weisbecker | [housekeeping: Move housekeeping related code to its own file](https://lore.kernel.org/patchwork/patch/845141) |  housekeepin 的代码目前与 nohz 绑定在了一起. 先把他们拆分出来, 后期计划将两个功能拆开 | v1☑ 4.15-rc4 | [PatchWork](https://lore.kernel.org/patchwork/cover/845141) |
| 2017/12/14 | Frederic Weisbecker | [sched/isolation: Make NO_HZ_FULL select CPU_ISOLATION](https://lore.kernel.org/patchwork/patch/863225) | 增加 cpu_capacity 的跟踪点 | v1☑ 4.15-rc4 | [PatchWork](https://lore.kernel.org/patchwork/cover/863225) |
| 2018/02/21 | Frederic Weisbecker | [isolation: 1Hz residual tick offloading v7](https://lore.kernel.org/patchwork/patch/888860) | 当CPU在full dynticks模式下运行时, 一个1Hz的滴答保持, 以保持调度器的统计状态活着. 然而, 对于那些根本无法忍受任何中断或想要最小化中断的裸金属任务来说, 这种残留的滴答是一种负担. 这组补丁将这个 1HZ 的 TICK 也从 CPU 上卸载掉. | v7 ☐ | [PatchWork](https://lore.kernel.org/patchwork/cover/888860) |
| 2020/12/27 | Frederic Weisbecker | [context_tracking: Flatter archs not using exception_enter/exit() v2](https://lore.kernel.org/patchwork/patch/1327311) | 为了能够在运行时打开/关闭 nohz_full 所做的准备, 需要 arch 放弃在任务堆栈上保存上下文跟踪状态, 因为这将迫使上下文跟踪在整个系统范围内运行, 即使是在没有启用 nohz_full 的 CPU 上 | v2 ☑ 5.11-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1327311) |


*   隔离 IRQD_AFFINITY_MANAGED 的中断

设置了 IRQD_AFFINITY_MANAGED 的中断, 其亲和性将由内核自动管理, 用户无法通过 `/proc/irq/*` 接口改变中断的亲和性, 这会导致亲和性同时包含隔离核和非隔离核,
有可能在隔离核触发造成干扰.

[`sched/isolation: isolate from handling managed interrupt`](https://lore.kernel.org/patchwork/patch/1182228)

1.  设置和还原中断亲和性的时候, 当设置的中断亲和性与隔离核的交集为空, 或者亲和性中的非隔离核均下线的时候, 不修改亲和性 mask. 否则与非隔离核掩码 housekeeping_mask 取交集.
2.  亲和性中的非隔离核均下线的场景, 当其中某个非隔离核上线的时候, 中断会迁移到这个核上.
3.  通过启动参数进行设置, isolcpus 中加上 managed_irq. 例如 isolacpus=domain, managed_irq, 10-19

*   隔离内核线程

内核线程默认的亲和性是所有 CPU, 有可能在隔离核上运行. 对隔离核上的进程产生干扰. 因此在配置了 nohz_full= 中, 新增了 HK_FLAG_KTHREAD.
在 kthreadd 和 `__kthread_create_on_node` 中, 设置了内核进程的亲和性为非隔离核, 对绑定了 CPU 的内线程和 per-cpu 的内核线程实例无影响.
设置方式, 启动参数配置 nohz_full=10-19.

[affine kernel threads to nohz_full= cpumask (v4)](https://lore.kernel.org/patchwork/cover/1218793), [lkml](https://lkml.org/lkml/2020/4/1/459)

*   Preventing job distribution to isolated CPUs

[Preventing job distribution to isolated CPUs](https://lore.kernel.org/patchwork/cover/1264010)

第一个补丁[lib: Restrict cpumask_local_spread to houskeeping CPUs](https://lore.kernel.org/patchwork/patch/1264008): cpumask_local_spread(int i, int node) 函数返回第 node 个 numa 节点上的第 i 个 CPU 的 CPU 号, 这个函数驱动中经常使用, 比如网卡驱动中设置中断亲和性时使用, 有可能选择到隔离核导致对隔离核上的进程有干扰, 因此修改为选择 CPU 时自动跳过隔离核. 通过启动参数打开此功能: isolcpus=domain,managed_irq,10-19

第二个补丁 [PCI: Restrict probe functions to housekeeping CPUs](), pci_call_probe 中选择 probe 的时候也跳过隔离核. 防止后面的 kworker 固定在隔离核上, 从而对隔离核造成干扰.

第三个补丁 [net: Restrict receive packets queuing to housekeeping CPUs](), 现有的 store_rps_map 机制会将报文发到其他 CPU 的 backlog 队列中接收, 无论该 COU 是否被隔离. 从而导致对隔离核造成干扰. 因此在选择 CPU 时, 只从非隔离核中选择. 使用启动参数开启此功能, isolcpus 和 nohz_full 均可.

### 8.3.2 task_isolation mode
-------

2020 年, 开发社区对 ["task_isolation" mode](https://lwn.net/Articles/816298) 进行了激烈的讨论, 这个特性允许那些对 latency 很敏感的应用程序在没有内核干扰的情况下得以在 CPU 上运行. 这项工作最终没有被 merge, 但人们显然仍然对这种模式很感兴趣. 这组补丁启发了 来自 Redhat 的 Marcelo Tosatti.

2021 年, Marcelo Tosatti 采取了一种更简单的方法来解决这个问题, 至少起初的时候是这样的. 这个 patch 着重关注的是, 哪怕 CPU 在 "nohz" 模式下运行时不会有定期的 clock tick, 也仍然会产生内核中断, 因此会引出麻烦. 具体而言, 他正在研究 "vmstat" 代码, 这部分代码是为内存管理子系统执行清理工作的. 其中一些工作是在一个单独的线程中完成的(通过一个工作队列来运行), 当 CPU 运行在 nohz 模式下时, 该线程通常会被禁用. 不过, 有些情况会导致这个线程在 nohz CPU 上被重新 reschedule 得以运行, 从而导致原来的应用程序无法再独占该处理器.

Tosatti 的 patch set 增加了一组新的 prctl() 命令来解决这个问题.

| 模式 | 描述 |
|:---:|:---:|
| PR_ISOL_SET | 设置 "isolation parameters", 这个参数可以是 PR_ISOL_MODE_NONE 或 PR_ISOL_MODE_NORMAL. 后者要求内核避免发生中断. |
| PR_ISOL_ENTER | task 进入 isolation 模式. |
| PR_ISOL_EXIT | task 退出 isolation 模式. 执行这个操作系统将变成 PR_ISOL_MODE_NONE 模式.  |
| PR_ISOL_GET | 获取 "isolation parameters" 参数 |

内核看到需要进入 isolation 模式的时候, 就会立即执行之前推迟的所有 vmstat 工作, 这样内核就不会在以后不方便清理的时候再做这个工作了. 在 isolation mode 中, 任何一次系统调用结束的时候都会触发这个 deferred-work 要完成的 cleanup 动作. 因为这些系统调用很可能总会触发一些 delayed work, 这样在应用程序代码运行时情况不会被弄得更加混乱.

这个改动的意图明显是希望使这类功能更加普遍适用, 也就是保证任何一个 delayed work 都要得以立即执行. 这导致其他人(包括 Nicolás Sáenz)的质疑, 认为这种采用单一的 mode 来控制那些各种不同的内核操作是不对的. 他说, 将各种行为分割开来, 后续就可以将一些决策动作转移到用户空间. 经过反反复复的讨论, Tosatti 同意修改接口, 让用户空间可以明确控制每个可能会用到的 isolation 功能. 因此 v2 实现该 API, 它增加了一个新的操作(PR_ISOL_FEAT), 用于查询 isolation 模式激活时可以被静默掉的那些 action.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2020/11/23 | Alex Belits | [support "task_isolation" mode](https://lwn.net/Articles/816298) | NO_HZ_FULL 的进一步优化, 进一步降低 tick 等对隔离核的影响 | v5 ☐ | [2016 Chris Metcalf v16](https://lore.kernel.org/patchwork/cover/847460)<br>*-*-*-*-*-*-*-* <br>[Alex Belits 2020 LWN](https://lwn.net/Articles/813804), [PatchWork](https://lore.kernel.org/patchwork/cover/1344134), [lkml](https://lkml.org/lkml/2020/11/23/1380) |
| 2021/07/30 | Marcelo Tosatti <mtosatti@redhat.com> | [extensible prctl task isolation interface and vmstat sync](https://lwn.net/Articles/864603) | 添加 prctl 来控制 task isolation 模式. 依赖于之前 "task isolation mode" 的基础代码实现, 内核看到需要进入 isolation 模式的时候, 就会立即执行之前推迟的所有 vmstat 工作, 这样内核就不会在以后不方便清理的时候再做这个工作了. 目前借助这个特性优化了 vmstat 的干扰. | v2 ☐ | [2021/07/27 Chris Metcalf 0/4](https://lore.kernel.org/patchwork/cover/1468441)<br>*-*-*-*-*-*-*-* <br>[2021/07/30 PatchWork v2, 0/4](https://lore.kernel.org/patchwork/cover/1470296)<br>*-*-*-*-*-*-*-* <br> |


## 8.4 更精确的调度时钟(HRTICK), 2.6.25(2008年4月发布)**
-------


CPU的周期性调度, 和基于时间片的调度, 是要基于时钟中断来触发的．一个典型的 1000 HZ 机器, 每秒钟产生 1000 次时间中断, 每次中断到来后, 调度器会看看是否需要调度．



然而, 对于调度时间粒度为微秒(10^-6)级别的精度来说, 这每秒 1000 次的粒度就显得太粗糙了．



2.6.25 引入了所谓的**高清嘀哒(High Resolution Tick)**, 以提供更精确的调度时钟中断．这个功能是基于**高精度时钟(High Resolution Timer)框架**, 这个框架让内核支持可以提供纳秒级别的精度的硬件时钟(将会在时钟子系统里讲).


## 8.5 混乱的 RT 优先级(RT 进程优先级管控)
-------

Linux 内核会将大量(并且在不断增加中)工作放置在内核线程中, 这些线程是在内核地址空间中运行的特殊进程. 大多数内核线程运行在 SCHED_NORMAL 类中, 必须与普通用户空间进程争夺CPU时间. 但是有一些内核线程它的开发者们认为它们非常特殊, 应该比用户空间进程要有更高优先级. 因此也会把这些内核线程放到 SCHED_FIFO 中去.

那么问题来了, 某个内核线程的实时优先级到底该设为多少呢?

要回答这个问题, 不仅需要判断这个线程相对于所有其他实时线程是否更加重要, 还要跟用户态的实时进程比较谁更重要. 这是一个很难回答的问题, 更何况在不同的系统和工作模式下这个答案很有可能还会各不相同.

所以无非两种可能性

*   一般来说, 内核开发人员也就是看心情直接随便选一个实时优先级;

*   另外一种情况, 内核开发人员觉得自己的线程是那么的重要, 因此给他一个比较高的实时优先级;

最近大神 Peter Zijlstra 又看到有内核开发者随便给内核线程设置优先级, 终于看不下去了,  指责这种把内核线程放入SCHED_FIFO的做法毫无意义:

"the kernel has no clue what actual priority it should use for various things, so it is useless (or worse, counter productive) to even try"

所以他发了一个系列 [[PATCH 00/23] sched: Remove FIFO priorities from modules](https://lore.kernel.org/patchwork/cover/1229354) 把设置内核线程优先级的接口干脆都给删了, 省得再有人瞎搞. 这个 PatchSet 主要做了下面几件事情:

1.  不再导出原有的sched_setschedule() / sched_setattr() 接口

2.  增加了 sched_set_fifo(p) / sched_set_fifo_low(p) / sched_set_normal(p, nice)

其中调用 sched_set_fifo() 会将指定进程放到 SCHED_FIFO 类中, 其优先级为 50 这只是 min 和 max 之间的一半位置.
对于需求不那么迫切的线程, sched_set_fifo_low()将优先级设置为最低值(1).
而调用 sched_set_normal() 会将线程返回给定好的值 SCHED_NORMAL 类.

通过只留下这三个接口可以避免开发者们再不停地去随机选取内核线程优先级, 因为这样本来毫无意义, 当然如果需要的话系统管理员还是可以按需调整不同进/线程的优先级.

这个特性最终在 5.9-rc1 的时候合入主线.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/09/17 | Peter Zijlstra | [sched: Remove FIFO priorities from modules](https://lore.kernel.org/patchwork/cover/1229354) | 显示驱动中 RT/FIFO 线程的优先级设定  | v1 ☑ 5.9-rc1 | [PatchWork](https://lwn.net/Articles/1307272) |
| 2020/09/17 | Dietmar Eggemann | [sched: Task priority related cleanups](https://lore.kernel.org/patchwork/cover/1372514) | 清理了优先级设置过程中一些老旧的接口. | v1 ☐ 5.12-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1372514) |



## 8.6 PREEMPT_RT
-------

标准的 Linux 内核中不可中断的系统调用、中断屏蔽等因素, 都会导致系统在时间上的不可预测性, 对硬实时限制没有保证. 目前, 针对 real-time Linux 的修改有两种成功的方案.

1.  直接修改 Linux 内核, 使其直接具有 real-time 能力; 其中有代表性的就是 PREEMPT-RT kernel.

2.  先运行一个 real-time 核心, 然后将 Linux 内核作为该 real-time 核心的 idle task 来运行. 称为 dual kernel (如 RTLinux 等).


其中 [PREEMPT_RT](https://rt.wiki.kernel.org/index.php/Main_Page) 是 Linux 内核的一个实时补丁. 一直由 [Thomas Gleixner](https://git.kernel.org/pub/scm/linux/kernel/git/tglx) 负责维护. 这组补丁曾经得到 Linus 的高度评价:

> Controlling a laser with Linux is crazy, but everyone in this room is crazy in his own way. So if you want to use Linux to control an industrial welding laser, I have no problem with your using PREEMPT_RT.
>    -- Linus Torvalds


PREEMPT-RT PATCH 的核心思想是最小化内核中不可抢占部分的代码, 同时将为支持抢占性必须要修改的代码量最小化. 对临界区、中断处理函数、关中断等代码序列进行抢占改进. 重点改动参见 : [Linux 优化 - Preempt RT 关键点](https://blog.csdn.net/jackailson/article/details/51045796), [PREEMPT-RT](https://blog.csdn.net/Binp0209/article/details/41241703). 目前(2021 年) PREEMPT_RT 的特性正在逐步往社区主线合并, 在完全合入之前, 所有的补丁都可以在 [PatchSet 镜像地址](https://mirrors.edge.kernel.org/pub/linux/kernel/projects/rt) 找到.

1. 可抢占支持(临界区可抢占, 中断处理函数可抢占, "关中断" 代码序列可抢占).

2. rt lock 改动, 锁支持优先级继承.

3. 降低延迟的措施.



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/7/15 | Thomas Gleixner <tglx@linutronix.de> | [locking, sched: The PREEMPT-RT locking infrastructure](https://lkml.org/lkml/2019/7/15/1386) | m在抢占菜单中添加一个新条目 PREEMPT_RT, 以支持内核的实时支持. 该选项仅在体系结构支持时启用. 它选择抢占, 因为 RT 特性依赖于它. 为了实现将现有的 PREEMPT 选项重命名为 `PREEMPT_LL`, 该选项也会选择 PREEMPT. 没有功能上的改变. | v1 ☑ 5.3-rc1 | [LKML](https://lkml.org/lkml/2019/7/15/1386) |


### 8.6.1  Migrate disable support && kmap_local
-------


内核中的 `kmap()` 接口在某种意义上是个挺奇怪的 API, 它的存在意义, 完全只是用来克服 32 位 CPU 的虚拟寻址限制的, 但是它影响了整个内核中各处的代码, 而且对 64 位机器还有副作用. 最近一次关于内核内部的 preemption(抢占)的处理的[讨论](https://lwn.net/Articles/831678/#highmem) 中, 暴露出来一些需要注意的问题, 其中之一就是 kmap() API. 现在, 人们提出了一个名为 kmap_local() 的 API , 对其进行扩展, 从而解决其中的一些问题. 它标志着内核社区在把 32 位机器从优先支持等级移除出去的过程又走出了一步, 参见 [Atomic kmaps become local](https://lwn.net/Articles/836144).



32 位的 linux 系统中可以使用 kmap 来映射高端内存, kmap() 函数本身, 会将一个 page 映射到内核的地址空间, 然后返回一个指针, 接下来就可以用这个指针来访问 page 的内容了. 不过, 用这种方式创建的映射开销是很大的. 它们会占用地址空间, 而且关于这个映射关系的改动必须要传播给系统的所有 CPU, 这个操作开销很大. 如果一个映射需要持续使用比较长的时间, 那么这项工作是必要的, 但是内核中的大部分 high memory 映射都是作为一个临时使用的映射, 短暂存在的, 并且只在一个地方来使用. 这种情况下, kmap()的开销中大部分都被浪费了.

因此, 人们加入了 kmap_atomic() API 作为避免这种开销的方法. 它也能做到将一个 high memory 的 page 映射到内核的地址空间中, 但是有一些不同. 它会从若干个 address slot 中挑一个进行映射, 而且这个映射只在创建它的 CPU 上有效. 这种设计意味着持有这种映射的代码必须在原子上下文中运行(因此叫 kmap_atomic()). 如果这部分代码休眠了, 或被移到另一个 CPU 上, 就肯定会出现混乱或者数据损坏了. 因此, 只要某一段在内核空间中运行的代码创建了一个 atomic mapping, 它就不能再被抢占或迁移, 也不允许睡眠, 直到所有的 atomic mapping 被释放. 为了支持这种功能, 内核需要提供 Migrate disable support. 在一些必要的路径上, 禁止进程进行迁移.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/09/17 | Thomas Gleixner | [sched: Migrate disable support for RT](https://lore.kernel.org/patchwork/cover/1307272) | 在启用 PREEMPT_RT 的内核上, 包括spin/rw锁持有部分在内的大部分代码都是可抢占的, 也使得任务可以迁移. 这违反了每个CPU的约束. 因此, PREEMPT_RT 需要一种独立于抢占的机制来控制迁移.  | v1 ☐ | [PatchWork](https://lwn.net/Articles/1307272) |
| 2020/10/23 | Peter Zijlstra | [sched: Migrate disable support](https://lore.kernel.org/patchwork/cover/1323936) | Peter 自己实现的 Migrate disable | v4 ☑ 5.11-rc1 | [2020/09/11 preparations](https://lore.kernel.org/patchwork/cover/1304210)<br>*-*-*-*-*-*-*-* <br>[2020/09/21 v1 PatchWork](https://lore.kernel.org/patchwork/cover/1309702)<br>*-*-*-*-*-*-*-* <br>[2020/10/23 v4 PatchWork](https://lore.kernel.org/patchwork/cover/1323936) |
| 2021/01/16 | Peter Zijlstra | [sched: Fix hot-unplug regressions](https://lore.kernel.org/patchwork/cover/1366383) | 修复 Migrate Disable 合入后导致的 CRASH 问题 | v3 ☑ 5.11-rc1 | [021/01/16 V1](https://lore.kernel.org/patchwork/cover/1366383)<br>*-*-*-*-*-*-*-*<br>[2021/01/21 V2](https://lore.kernel.org/patchwork/cover/1368710) |

接着 Thomas Gleixner 完成了 KMAP_LOCAL API.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/09/17 | Thomas Gleixner | [mm/highmem: Preemptible variant of kmap_atomic & friends](https://lore.kernel.org/patchwork/cover/1341244) | 此处填写补丁描述 | v4 ☑ 5.11-rc1 | [v3 PatchWork](https://lore.kernel.org/patchwork/cover/1331277)<br>*-*-*-*-*-*-*-* <br>[v4 PatchWork](https://lore.kernel.org/patchwork/cover/1341244) |
| 2021/01/12 | Thomas Gleixner | [mm/highmem: Fix fallout from generic kmap_local conversions](https://lore.kernel.org/patchwork/cover/1364171) | 此处填写补丁描述| v1 ☐ | [v4 PatchWork](https://lore.kernel.org/patchwork/cover/1364171) |

后来主线上 Dexuan Cui 报 Migrate Disable 合入后引入了问题, [5.10: sched_cpu_dying() hits BUG_ON during hibernation: kernel BUG at kernel/sched/core.c:7596!](https://lkml.org/lkml/2020/12/22/141). Valentin Schneider 怀疑是有些 kworker 线程在 CPU 下线后又选到了下线核上运行, 因此建议去测试这组补丁 [workqueue: break affinity initiatively](https://lkml.org/lkml/2020/12/18/406). Dexuan Cui 测试以后可以解决这个问题, 但是会有其他 WARN. Peter Zijlstra 的 解决方案如下 [sched: Fix hot-unplug regression](https://lore.kernel.org/patchwork/cover/1368710).


### 8.6.2 RT LOCK
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/15 | Thomas Gleixner <tglx@linutronix.de> | [locking, sched: The PREEMPT-RT locking infrastructure](https://lore.kernel.org/patchwork/cover/1476862) | mutex, ww_mutex, rw_semaphore, spinlock, rwlock | v5 ☑ 5.15-rc1 | [PatchWork V5,00/72](https://lore.kernel.org/all/20210815203225.710392609@linutronix.de), [LKML](https://lkml.org/lkml/2021/8/15/209) |


### 8.6.3 中断线程化
-------

[Linux RT (2)－硬实时 Linux (RT-Preempt Patch) 的中断线程化](https://blog.csdn.net/21cnbao/article/details/8090398)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/3/23 | Thomas Gleixner <tglx@linutronix.de> | [Add support for threaded interrupt handlers - V3](https://lkml.org/lkml/2009/3/23/344) | 线程化中断的支持, 这组补丁提供一个能力, 驱动可以通过 request_threaded_irq 申请一个线程化的 IRQ. kernel 会为中断的底版本创建一个名字为 irq/%d-%s 的线程, %d 对应着中断号. 其中上半部(硬中断) handler 在做完必要的处理工作之后, 会返回 IRQ_WAKE_THREAD, 之后 kernel 会唤醒 irq/%d-%s 线程, 而该 kernel 线程会调用 thread_fn 函数, 因此, 该线程处理中断下半部. 该机制目前在 kernel 中使用已经十分广泛, 可以认为是继 softirq(含 tasklet) 和 workqueue 之后的又一大中断下半部方式. | v3 ☑ 2.6.30-rc1 | [PatchWork V5,00/72](https://lkml.org/lkml/2009/3/23/344), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3aa551c9b4c40018f0e261a178e3d25478dc04a9) |
| 2009/8/15 | Thomas Gleixner <tglx@linutronix.de> | [genirq: Support nested threaded irq handlinge](https://lkml.org/lkml/2009/8/15/130) | 中断线程化支持 nested/oneshot 以及 buslock 等. | v1 ☑ [2.6.32-rc1](https://kernelnewbies.org/Linux_2_6_32#Various_core_changes) | [LKML](https://lkml.org/lkml/2009/8/15/130), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=399b5da29b9f851eb7b96e2882097127f003e87c) |
| 2011/2/23 | Thomas Gleixner <tglx@linutronix.de> | [genirq: Forced threaded interrupt handlers](https://lkml.org/lkml/2011/2/23/510) | 引入 CONFIG_IRQ_FORCED_THREADING, 增加了命令行参数 ["threadirqs"](https://elixir.bootlin.com/linux/v2.6.39/source/kernel/irq/manage.c#L28), 强制所有中断(除了标记为 IRQF_NO_THREAD 的中断)包括软中断[均以线程方式运行](https://elixir.bootlin.com/linux/v2.6.39/source/Documentation/kernel-parameters.txt#L2474), 这主要是一个调试选项, 允许从崩溃的中断处理程序中检索更好的调试数据. 如果在内核命令行上没有启用 "threadirqs", 那么对中断热路径没有影响. 架构代码需要在标记了不能被线程化的中断 IRQF_NO_THREAD 之后选择 CONFIG_IRQ_FORCED_THREADING. 所有设置了 IRQF_TIMER 的中断都是隐式标记的 IRQF_NO_THREAD. 所有的 PER_CPU 中断也被排除在外.<br>当启用它时, 可能会降低一些速度, 但对于调试中断代码中的问题, 这是一个合理的惩罚, 因为当中断处理程序有 bug 时, 它不会立即崩溃和烧毁机器. | v1 ☑ [2.6.39-rc1](https://kernelnewbies.org/Linux_2_6_39#Core) | [LKML](https://lkml.org/lkml/2011/2/23/510), [LWN](https://lwn.net/Articles/429690), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8d32a307e4faa8b123dc8a9cd56d1a7525f69ad3) |
| 2021/8/22 | Thomas Gleixner <tglx@linutronix.de> | [softirq: Introduce SOFTIRQ_FORCED_THREADING](https://lkml.org/lkml/2021/8/22/417) | CONFIG_IRQ_FORCED_THREADING 中强制软中断也做了线程化, 作者认为这不合理, 因此引入 SOFTIRQ_FORCED_THREADING 单独控制软中断的线程化.<br>1. 中断退出时是否执行 softirq 由 IRQ_FORCED_THREADING 控制, 这是不合理的. 应该将其拆分, 并允许其单独生效.<br>2. 同时, 当中断退出时, 我们应该增加 ksoftirqd 的优先级, 作者参考了 PREEMPT_RT 的实现, 认为它是合理的. | v1 ☐ | [PatchWork](https://lore.kernel.org/lkml/1629689583-25324-1-git-send-email-wangqing@vivo.com), [LKML](https://lkml.org/lkml/2021/8/22/417) |


### 8.6.x 零碎的修修补补
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/9/28 | Thomas Gleixner <tglx@linutronix.de> | [sched: Miscellaneous RT related tweaks](https://lkml.org/lkml/2021/9/28/617) | 启用 RT 的内核在调度程序的内部工作方面存在一些问题:<br>1. 远程 TTWU 队列机制导致最大延迟增加 5 倍;<br>2. 32 个任务的批处理迁移限制会导致较大的延迟.<br>3. kprobes 的清理、死任务的 vmapped 堆栈和 mmdrop() 是导致延迟增大的源头, 这些路径从禁用抢占的调度程序核心中获取常规的自旋锁. | v1 ☐ | [PatchWork 0/5](https://lkml.org/lkml/2021/9/28/617) |



# 9 进程管理
-------


## 9.1 进程创建
-------

### 9.1.1 shared page tables
-------

对 shared page tables 的研究由来已久, Dave McCracken提出了一种共享页面表的方法[Implement shared page tables](https://lore.kernel.org/patchwork/cover/42673), 可以参见 [Shared Page Tables Redux](https://www.kernel.org/doc/ols/2006/ols2006v2-pages-125-130.pdf). 但从未进入内核.

随后 2021 年《 我们相信, 随着现代应用程序和fork的现代用例（如快照）内存消耗的增加, fork上下文中的共享页表方法值得探索.

在我们的研究工作中[https://dl.acm.org/doi/10.1145/3447786.3456258], 我们已经确定了一种方法, 对于大型应用程序(即几百 MBs 及以上), 该方法可以显著加快 fork 系统调用. 目前, fork系统调用完成所需的时间与进程分配内存的大小成正比, 在我们的实验中, 我们的设计将 fork 调用的速度提高了 270 倍(50GB).<br>其设计是, 在 fork 调用期间, 我们不复制整个分页树, 而是让子进程和父进程共享同一组最后一级的页表, 这些表将被引用计数. 为了保留写时复制语义, 我们在 fork 中的 PMD 条目中禁用写权限, 并根据需要在页面错误处理程序中复制 PTE 表.

另一方面, http://lkml.iu.edu/hypermail/linux/kernel/0508.3/1623.html, https://www.kernel.org/doc/ols/2006/ols2006v2-pages-125-130.pdf], 但从未进入内核. 我们相信, 随着现代应用程序和fork的现代用例（如快照）内存消耗的增加, fork上下文中的共享页表方法值得探索.



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2006/04/10 | Dave McCracken <dmccr@us.ibm.com> | [Implement shared page tables](https://patchwork.kernel.org/project/linux-mm/patch/20210701134618.18376-1-zhao776@purdue.edu) | 这组补丁为跨越整个页表页的所有共享内存区域实现页表共享(CONFIG_PTSHARE). 它支持在多个页面级别(PTSHARE_PTE/PTSHARE_PMD/PTSHARE_PUD/PTSHARE_HUGEPAGE)上共享, 具体取决于体系结构.<br>共享页表的主要目的是提高在多个进程之间共享大内存区域的大型应用程序的性能.<br>它消除了冗余页表, 并显著减少了次要页错误的数量. 测试表明, 大型数据库应用程序(包括使用大型页面的应用程序)的性能有了显著提高. 对于小流程, 没有可测量的性能下降. | [2002/10/02 PatchWork](https://lore.kernel.org/patchwork/cover/9505)<br>*-*-*-*-*-*-*-* <br>[2005/08/30 PatchWork 1/1](https://lore.kernel.org/patchwork/cover/42673)<br>*-*-*-*-*-*-*-* <br>[2006/01/05 PatchWork RFC](https://lore.kernel.org/patchwork/cover/49324)<br>*-*-*-*-*-*-*-* <br>[2006/04/10 PatchWork RFC](https://lore.kernel.org/patchwork/cover/55396) |
| 2021/07/06 | Kaiyang Zhao <zhao776@purdue.edu> | [Shared page tables during fork](https://patchwork.kernel.org/project/linux-mm/patch/20210701134618.18376-1-zhao776@purdue.edu) | 引入 read_ti_thread_flags() 规范对 thread_info 中 flag 的访问. 其中默认使用了 READ_ONCE. 防止开发者忘记了这样做. | [PatchWork v4,00/10](https://lore.kernel.org/patchwork/cover/1471548) |

# 9 其他
-------

## 9.1 CPU HOTPLUG 中的调度处理
-------



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/03/10 | Tejun Heo <tj@kernel.org> | [sched: define and use CPU_PRI_* enums for cpu notifier priorities](https://lore.kernel.org/patchwork/cover/201980) | 重构 migration_call 的 通知方式 | v1 ☑ 2.6.36-rc1 | 注意补丁有 UPATE, [UPDATE](https://lore.kernel.org/patchwork/cover/202907) |
| 2016/03/10 | Thomas Gleixner <tglx@linutronix.de> | [sched: Migrate disable support](https://lore.kernel.org/patchwork/cover/657710) | 将 CPU 下线时 MIGRATE 的操作放到了 sched_cpu_dying() 中进行 | v1 ☑ 4.7-rc1 | [2020/09/11 preparations](https://lore.kernel.org/patchwork/cover/657710) |
| 2020/10/23 | Peter Zijlstra | [sched: Migrate disable support](https://lore.kernel.org/patchwork/cover/1323936) | 在启用 PREEMPT_RT 的内核上, 包括spin/rw锁持有部分在内的大部分代码都是可抢占的, 也使得任务可以迁移. 这违反了每个CPU的约束. 因此, PREEMPT_RT 需要一种独立于抢占的机制来控制迁移. 此特性移除了 sched_cpu_dying(), 改为 balance_push 来完成这个操作 | v4 ☑ 5.11-rc1 | [2020/09/11 preparations](https://lore.kernel.org/patchwork/cover/1304210)<br>*-*-*-*-*-*-*-* <br>[2020/09/21 v1 PatchWork](https://lore.kernel.org/patchwork/cover/1309702)<br>*-*-*-*-*-*-*-* <br>[2020/10/23 v4 PatchWork](https://lore.kernel.org/patchwork/cover/1323936) |



## 9.2 用户态调度框架
-------


### 9.2.1 Google Fibers 用户空间调度框架
-------

"Google Fibers" 是一个用户空间调度框架, 在谷歌广泛使用并成功地用于改善进程内工作负载隔离和响应延迟. 我们正在开发这个框架, UMCG(用户管理并发组)内核补丁是这个框架的基础.


[FUTEX_SWAP补丁分析-SwitchTo 如何大幅度提升切换性能？](https://mp.weixin.qq.com/s/dDg5WKb8vqo5WfArAuav9Q)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/08/03 | Peter Oskolkov <posk@google.com>/<posk@posk.io> | [FUTEX_SWAP](https://lore.kernel.org/patchwork/cover/1433967) | 通过对 futex 的魔改, 使得在用户态使用 switch_to() 指定任务切换的能力. 这就是用户模式线程的用途: 极低的切换开销, 意味着我们操作系统可以支持的数以千计的线程可以提高到 10 倍以上甚至百万级别. | [2020/06/15 PatchWork RFC,0/3](https://lore.kernel.org/patchwork/cover/1256264)<br>*-*-*-*-*-*-*-* <br>[2020/06/16 PatchWork RFC,0/3,v2](https://lore.kernel.org/patchwork/cover/1257233)<br>*-*-*-*-*-*-*-* <br>[2021/07/16 PatchWork RFC,0/3,v3](https://lore.kernel.org/patchwork/cover/1263506)<br>*-*-*-*-*-*-*-* <br>[2020/08/03 PatchWork for,5.9,v2,0/4](https://lore.kernel.org/patchwork/cover/1283798) |
| 2021/09/08 | Peter Oskolkov <posk@google.com>/<posk@posk.io> | [sched/UMCG](https://lore.kernel.org/patchwork/cover/1433967) | UMCG (User-Managed Concurrency Groups)  | [PatchWork RFC,v0.1,0/9](https://lore.kernel.org/patchwork/cover/1433967)<br>*-*-*-*-*-*-*-* <br>[2021/07/08 PatchWork RFC,0/3,v0.2](https://lore.kernel.org/patchwork/cover/1455166)<br>*-*-*-*-*-*-*-* <br>[2021/07/16 PatchWork RFC,0/4,v0.3](https://lore.kernel.org/patchwork/cover/1461708)<br>*-*-*-*-*-*-*-* <br>[2021/08/01 PatchWork 0/4,v0.4](https://lore.kernel.org/patchwork/cover/1470650)<br>*-*-*-*-*-*-*-* <br>[2021/08/01 LWN 0/4,v0.5](https://lore.kernel.org/patchwork/cover/1470650) |
| 2021/09/08 | Peter Oskolkov <posk@google.com>/<posk@posk.io> | [google ghOSt](https://github.com/google/ghost-kernel) | ghOSt 是在 Linux 内核上实现的用户态调度策略的通用代理. ghOSt 框架提供了一个丰富的 API, 该 API 从用户空间接收进程的调度决策, 并将其作为事务执行. 程序员可以使用任何语言或工具来开发策略, 这些策略可以在不重新启动机器的情况下升级. ghOSt 支持一系列调度目标的策略, 从 µs 级延迟到吞吐量, 再到能源效率, 等等, 并且调度操作的开销较低. 许多策略只是几百行代码. 总之, ghOSt 提供了一个性能框架, 用于将线程调度策略委托给用户空间进程, 从而实现策略优化、无中断升级和故障隔离. | [github kernel](https://github.com/google/ghost-kernel)<br>*-*-*-*-*-*-*-* <br>[github userspace](https://github.com/google/ghost-userspace) |


### 9.2.2 Scheduler BPF
-------


CFS 调度器为用户和开发人员提供了非常多的调试接口和参数供大家调优, 这些设置现在大多数都位于 debugfs 中. 其中一些参数可以归结为改变任务抢占的可能性, 比如通过将 wakeup_granularity_ns 设置为 latency_ns 的一半以上来禁用任务抢占.

从这点可以看出, 我们的一些工作负载受益于处理短期运行请求的任务抢占长时间运行的任务, 而一些只运行短期请求的工作负载受益于从不被抢占.


这导致了一些观察和想法:


1.  不同的工作量需要不同的策略, 能够为每个工作负载配置策略可能会很有用.

2.  一个工作负载从自己不被抢占中受益, 仍然可以从抢占(低优先级)后台系统任务中受益.

3.  在生产中快速(且安全地)试验不同的策略是有用的, 而不必关闭应用程序或重新启动系统, 以确定不同工作负载的策略应该是什么.

4.  只有少数任务的工作量足够大和敏感, 值得他们自己调整策略. CFS 本身就足以解决其他问题, 我们可能不希望政策调整取代 CFS 所做的任何事情.


BPF 钩子(它已经成功地用于各种内核子系统)为外部代码(安全地)更改一些内核决策提供了一种方法, BPF 工具使这变得非常容易, 部署 BPF 脚本的开发者已经非常习惯于为新的内核版本更新它们.


*   Facebook 的尝试

Roman Gushchin 在邮件列表发起了 BPF 对调度器的潜在应用的讨论, 它提交的 patchset 旨在为调度器提供一些非常基本的 BPF 基础设施, 以便向调度器添加新的 BPF钩子、一组最小的有用助手以及相应的 libbpf 更改等等. 他们在 CFS 中使用 BPF 的第一次实验看起来非常有希望. 虽然还处于非常早期的阶段, 但在 Facebook 的主网页工作量已经获得了不错的延迟和约 1% 的 RPS.

作者提供了一个用户空间部分的示例 [github/rgushchin/atc](https://github.com/rgushchin/atc), 它加载了一些简单的钩子. 它非常简单, 只是为了简化使用所提供的内核补丁.

*   Google 的尝试

于此同时, google 团队的 Hao Luo 和 Barret Rhoden 等也在 eBPF 在  CPU Scheduler 领域的应用进行了探索, 并在 LPC-2021 上做了分享. 当前的工作集中在几个方向:

1.  调度延迟分析以及依据 profiling 来指导调度

2.  核心调度中的强制空闲时间计算

3.  使用 BPF 加速 ghOSt 内核调度器

这与谷歌的 ghOSt 非常类似, 但是 ghOSt 比 BPF 的方式要激进很多, ghOSt 的目标是将调度代码转移到用户空间. 它们的核心动机似乎有些相似:使调度器更改更容易开发、验证和部署. 尽管他们的方法不同, 他们也使用 BPF 来加速一些热点路径. 但是作者认为使用 BPF 的方式也可以达到他们的目的. , 参见 [eBPF in CPU Scheduler](https://linuxplumbersconf.org/event/11/contributions/954/attachments/776/1463/eBPF%20in%20CPU%20Scheduler.pdf)



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/09/15 | Roman Gushchin <guro@fb.com> | [Scheduler BPF](https://www.phoronix.com/scan.php?page=news_item&px=Linux-BPF-Scheduler) | NA | RFC ☐ | [PatchWork rfc,0/6](https://patchwork.kernel.org/project/netdevbpf/cover/20210916162451.709260-1-guro@fb.com)<br>*-*-*-*-*-*-*-* <br>[LPC 2021](https://linuxplumbersconf.org/event/11/contributions/954)<br>*-*-*-*-*-*-*-* <br>[LKML](https://lkml.org/lkml/2021/9/16/1049), [LWN](https://lwn.net/Articles/869433) |


## 9.3 安全相关
-------



## 9.4 其他
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/03 | Peter Oskolkov <posk@google.com> | [thread_info: use helpers to snapshot thread flags](https://lwn.net/Articles/722293) | 引入 read_ti_thread_flags() 规范对 thread_info 中 flag 的访问. 其中默认使用了 READ_ONCE. 防止开发者忘记了这样做. | v4 ☐ | [PatchWork v4,00/10](https://lore.kernel.org/patchwork/cover/1471548) |




# 10 调试信息
-------


## 10.1 统计信息
-------

阿里的王贇 [sched/numa: introduce numa locality](https://lore.kernel.org/patchwork/cover/1190383) 提供了 per-cgroup 的 NUMASTAT 功能, 发到了 2020/02/07 v8, 但是最终还是没能合入主线.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/02/07 | 王贇 | [sched/numa: introduce numa locality](https://lore.kernel.org/patchwork/cover/1190383) | per-cgroup 的 NUMASTAT 功能 | [PatchWork v8](https://lore.kernel.org/patchwork/cover/1190383) |
| 2021/03/27 | Yafang Shao | [sched: support schedstats for RT sched class](https://lore.kernel.org/patchwork/cover/1403138) | 我们希望使用 schedstats 工具测量生产环境中 RT 任务的延迟, 但目前只支持公平调度类的 schedstats.  将 sched_statistics 修改为独立于 task_struct 或 task_group 的调度统计数据, 从而完成了 RT 的 schedstats 支持 | [PatchWork v2](https://lore.kernel.org/patchwork/cover/1403138)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3](http://patches.linaro.org/cover/502064) |


## 10.2 tracepoint
-------

[`tracepoints-helpers`](https://github.com/auldp/tracepoints-helpers.git)
[`plot-nr-running`](https://github.com/jirvoz/plot-nr-running)


| 时间  | 作者 |特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:---------:|:----:|
| 2017/03/28 | Dietmar Eggemann | [CFS load tracking trace events](https://lore.kernel.org/patchwork/cover/774154) | 增加 PELT 的跟踪点 | v1 ☐ | [PatchWork](https://lore.kernel.org/patchwork/cover/774154) |
| 2018/12/14 | Benjamin | [sched/debug: Add tracepoint for RT throttling](https://lore.kernel.org/patchwork/patch/1024902) | RT throttle 的 tracepoint | v1 ☐ | [PatchWork](https://lore.kernel.org/patchwork/patch/1024902) |
| 2019/06/04 | | [sched: Add new tracepoints required for EAS testing](https://lore.kernel.org/patchwork/patch/1296761) | 增加 PELT 的跟踪点 | v3 ☑ 5.3-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/1296761) |
| 2020/06/19 | | [Sched: Add a tracepoint to track rq->nr_running](https://lore.kernel.org/patchwork/patch/1258690) | 增加 nr_running 的跟踪点 | v1 ☑ 5.9-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/1258690)<br>*-*-*-*-*-*-*-* <br>[FixPatch](https://lore.kernel.org/patchwork/patch/1284621) |
| 2020/08/28 | | [sched/debug: Add new tracepoint to track cpu_capacity](https://lore.kernel.org/patchwork/patch/1296761) | 增加 cpu_capacity 的跟踪点 | v1 ☐ |  [PatchWork](https://lore.kernel.org/patchwork/cover/1296761) |

## 10.3 debug 接口
-------


在调度的 debug 接口和调优接口中, 在 cpu_load index 一直时争议比较大的一组.

在 cpu_load 衰减使用中, 我们混合了长期、短期负载和平衡偏差, 根据平衡随机选取一个大或小的值目的地或来源. 这种组合本身时非常奇怪的, 甚至很多人觉得是错误的, 应该基于平衡偏见和在 cpu 组之间的任务移动成本, 而不是随机历史或即时负载.

很多开发者普遍认为历史负载可能会偏离实际负载, 导致不正确的偏差.

因此开发者尝试过多次, 将这些看起来奇怪的东西从内核中移除掉.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2014/04/16 | Alex Shi | [remove cpu_load idx](https://lore.kernel.org/patchwork/cover/456546) | 调度中使用 cpu_load 来做负载比较时非常错误的, 因此移除他们. | v5 ☐ | [PatchWork](https://lore.kernel.org/patchwork/cover/456546) |
| 2017/09/29 | Peter Zijlstra | [sched: Rework task state printing](https://lore.kernel.org/patchwork/cover/834387) | 重构进程状态的打印方式 | v1 ☑ 4.14-rc3 |[PatchWork](https://lore.kernel.org/patchwork/cover/834387) |
| 2021/01/06 | Vincent Guittot | [sched: Remove per rq load array](https://lore.kernel.org/patchwork/cover/1079333) | 自 LB_BIAS 被禁用之后, 调度器只使用 rq->cpu_load[0] 作为cpu负载值, 因此 cpu_load 这个数组的其他之其实没意义了, 直接去掉了. 注意这对 load_balance 的调优是有一定影响的, 之前 sched_domain 中可以通过 sysctl 接口修改比较负载使用的 index, 这些 index 对应的 cpu_load 数组的下标. 干掉了这个数组, 那么这些 sysctl 也就没必要了 | v2 ☑ 5.10-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1079333) |
| 2021/03/26 | Peter Zijlstra | [sched: Clean up SCHED_DEBUG](https://lore.kernel.org/patchwork/cover/1402660) | 目前内核有 sysctl, procfs 和 debugfs SCHED_DEBUG 接口, 比较混乱, 将所有接口信息都转移到 debugfs 中 | v1 ☑ 5.13-rc1 |[PatchWork](https://lore.kernel.org/patchwork/cover/1402660) |



**引用: **

<div id="ref-anchor-1"></div>
- [1] [Single UNIX Specification](https://en.wikipedia.org/wiki/Single_UNIX_Specification%23Non-registered_Unix-like_systems)

<div id="ref-anchor-2"></div>
- [2] [POSIX 关于调度规范的文档](http://nicolas.navet.eu/publi/SlidesPosixKoblenz.pdf)

<div id="ref-anchor-3"></div>
- [3] [Towards Linux 2.6](https://link.zhihu.com/?target=http%3A//www.informatica.co.cr/linux-scalability/research/2003/0923.html)

<div id="ref-anchor-4"></div>
- [4] [Linux内核发布模式与开发组织模式(1)](https://link.zhihu.com/?target=http%3A//larmbr.com/2013/11/02/Linux-kernel-release-process-and-development-dictator-%26-lieutenant-system_1/)

<div id="ref-anchor-5"></div>
- [5] IBM developworks 上有一篇综述文章, 值得一读 :[Linux 调度器发展简述](https://link.zhihu.com/?target=http%3A//www.ibm.com/developerworks/cn/linux/l-cn-scheduler/)

<div id="ref-anchor-6"></div>
- [6] [CFS group scheduling [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/240474/)

<div id="ref-anchor-7"></div>
- [7] [http://lse.sourceforge.net/numa/](https://link.zhihu.com/?target=http%3A//lse.sourceforge.net/numa/)

<div id="ref-anchor-8"></div>
- [8] [CFS bandwidth control [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/428230/)

<div id="ref-anchor-9"></div>
- [9] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D5091faa449ee0b7d73bc296a93bca9540fc51d0a)

<div id="ref-anchor-10"></div>
- [10] [DMA模式\_百度百科](https://link.zhihu.com/?target=http%3A//baike.baidu.com/view/196502.htm)

<div id="ref-anchor-11"></div>
- [11] [进程的虚拟地址和内核中的虚拟地址有什么关系？ - 詹健宇的回答](http://www.zhihu.com/question/34787574/answer/60214771)

<div id="ref-anchor-12"></div>
- [12] [Physical Page Allocation](https://link.zhihu.com/?target=https%3A//www.kernel.org/doc/gorman/html/understand/understand009.html)

<div id="ref-anchor-13"></div>
- [13] [The SLUB allocator [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/229984/)

<div id="ref-anchor-14"></div>
- [14] [Lumpy Reclaim V3 [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/211199/)

<div id="ref-anchor-15"></div>
- [15] [Group pages of related mobility together to reduce external fragmentation v28 [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/224254/)

<div id="ref-anchor-16"></div>
- [16] [Memory compaction [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/368869/)

<div id="ref-anchor-17"></div>
- [17] [kernel 3.10内核源码分析--TLB相关--TLB概念、flush、TLB lazy模式-humjb\_1983-ChinaUnix博客](https://link.zhihu.com/?target=http%3A//blog.chinaunix.net/uid-14528823-id-4808877.html)

<div id="ref-anchor-18"></div>
- [18] [Toward improved page replacement[LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/226756/)

<div id="ref-anchor-19"></div>
- [19] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D4f98a2fee8acdb4ac84545df98cccecfd130f8db)

<div id="ref-anchor-20"></div>
- [20] [The state of the pageout scalability patches [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/286472/)

<div id="ref-anchor-21"></div>
- [21] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D894bc310419ac95f4fa4142dc364401a7e607f65)

<div id="ref-anchor-22"></div>
- [22] [Being nicer to executable pages [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/333742/)

<div id="ref-anchor-23"></div>
- [23] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D8cab4754d24a0f2e05920170c845bd84472814c6)

<div id="ref-anchor-24"></div>
- [24] [Better active/inactive list balancing [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/495543/)

<div id="ref-anchor-25"></div>
- [25] [Smarter write throttling [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/245600/)

<div id="ref-anchor-26"></div>
- [26] [https://zh.wikipedia.org/wiki/%E6%8C%87%E6%95%B0%E8%A1%B0%E5%87%8F](https://link.zhihu.com/?target=https%3A//zh.wikipedia.org/wiki/%25E6%258C%2587%25E6%2595%25B0%25E8%25A1%25B0%25E5%2587%258F)

<div id="ref-anchor-27"></div>
- [27] [Flushing out pdflush [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/326552/)

<div id="ref-anchor-28"></div>
- [28] [Dynamic writeback throttling [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/405076/)

<div id="ref-anchor-29"></div>
- [29] [On-demand readahead [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/235164/)

<div id="ref-anchor-30"></div>
- [30] [Transparent huge pages in 2.6.38 [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/423584/)

<div id="ref-anchor-31"></div>
- [31] [https://events.linuxfoundation.org/sites/events/files/lcjp13\_ishimatsu.pdf](https://link.zhihu.com/?target=https%3A//events.linuxfoundation.org/sites/events/files/lcjp13_ishimatsu.pdf)

<div id="ref-anchor-32"></div>
- [32] [transcendent memory for Linux [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/338098/)

<div id="ref-anchor-33"></div>
- [33] [linux kernel monkey log](https://link.zhihu.com/?target=http%3A//www.kroah.com/log/linux/linux-staging-update.html)

<div id="ref-anchor-34"></div>
- [34] [zcache: a compressed page cache [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/397574/)

<div id="ref-anchor-35"></div>
- [35] [The zswap compressed swap cache [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/537422/)

<div id="ref-anchor-36"></div>
- [36] [Linux-Kernel Archive: Linux 2.6.0](https://link.zhihu.com/?target=http%3A//lkml.iu.edu/hypermail/linux/kernel/0312.2/0348.html)

<div id="ref-anchor-37"></div>
- [37]抢占支持的引入时间: [https://www.kernel.org/pub/linux/kernel/v2.5/ChangeLog-2.5.4](https://link.zhihu.com/?target=https%3A//www.kernel.org/pub/linux/kernel/v2.5/ChangeLog-2.5.4)

<div id="ref-anchor-38"></div>
- [38] [RAM is 100 Thousand Times Faster than Disk for Database Access](https://link.zhihu.com/?target=http%3A//www.directionsmag.com/entry/ram-is-100-thousand-times-faster-than-disk-for-database-access/123964)

<div id="ref-anchor-39"></div>
- [39] [http://www.uefi.org/sites/default/files/resources/ACPI\_6.0.pdf](https://link.zhihu.com/?target=http%3A//www.uefi.org/sites/default/files/resources/ACPI_6.0.pdf)

<div id="ref-anchor-40"></div>
- [40] [Injecting faults into the kernel [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/209257/)

<div id="ref-anchor-41"></div>
- [41] [Detecting kernel memory leaks [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/187979/)

<div id="ref-anchor-42"></div>
- [42] [The kernel address sanitizer [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/612153/)

<div id="ref-anchor-43"></div>
- [43] [Linux Kernel Shared Memory 剖析](https://link.zhihu.com/?target=http%3A//www.ibm.com/developerworks/cn/linux/l-kernel-shared-memory/)

<div id="ref-anchor-44"></div>
- [44] [KSM tries again [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/330589/)

<div id="ref-anchor-45"></div>
- [45] [HWPOISON [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/348886/)

<div id="ref-anchor-46"></div>
- [46] [https://www.mcs.anl.gov/research/projects/mpi/](https://link.zhihu.com/?target=https%3A//www.mcs.anl.gov/research/projects/mpi/)

<div id="ref-anchor-47"></div>
- [47] [Fast interprocess messaging [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/405346/)



---8<---

**更新日志:**

**- 2015.9.12**

o 完成调度器子系统的初次更新, 从早上10点开始写, 写了近７小时, 比较累, 后面更新得慢的话大家不要怪我(对手指
