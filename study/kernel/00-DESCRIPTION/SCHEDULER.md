1   调度子系统(scheduling)
=====================

**概述: **Linux 是一个遵循 POSIX 标准的类 Unix 操作系统(然而它并不是 Unix 系统[<sup>1</sup>](#refer-anchor-1)), POSIX 1003.1b 定义了调度相关的一个功能集合和 API 接口[<sup>2</sup>](#refer-anchor-2). 调度器的任务是分配 CPU 运算资源, 并以协调效率和公平为目的. **效率**可从两方面考虑: 1) 吞吐量(throughput) 2)延时(latency). 不做精确定义, 这两个有相互矛盾的衡量标准主要体现为两大类进程: 一是 CPU 密集型, 少量 IO 操作, 少量或无与用户交互操作的任务（强调吞吐量, 对延时不敏感, 如高性能计算任务 HPC), 另一则是 IO 密集型, 大量与用户交互操作的任务(强调低延时, 对吞吐量无要求, 如桌面程序). **公平**在于有区分度的公平, 多媒体任务和数值计算任务对延时和限定性的完成时间的敏感度显然是不同的.
为此,  POSIX 规定了操作系统必须实现以下**调度策略(scheduling policies),** 以针对上述任务进行区分调度:

**- SCHED\_FIFO**

**- SCHED\_RR**

这两个调度策略定义了对实时任务, 即对延时和限定性的完成时间的高敏感度的任务. 前者提

供 FIFO 语义, 相同优先级的任务先到先服务, 高优先级的任务可以抢占低优先级的任务; 后 者提供 Round-Robin 语义, 采用时间片, 相同优先级的任务当用完时间片会被放到队列尾

部, 以保证公平性, 同样, 高优先级的任务可以抢占低优先级的任务. 不同要求的实时任务可

以根据需要用 **_sched\_setscheduler()_** API 设置策略.
**- SCHED\_OTHER**

此调度策略包含除上述实时进程之外的其他进程, 亦称普通进程. 采用分时策略, 根据动态优

先级(可用 **nice()** API设置）, 分配 CPU 运算资源.  **注意: 这类进程比上述两类实时进程优先级低, 换言之, 在有实时进程存在时, 实时进程优先调度**.



Linux 除了实现上述策略, 还额外支持以下策略:

- **SCHED\_IDLE** 优先级最低, **在系统空闲时才跑这类进程**(如利用闲散计算机资源跑地外文明搜索, 蛋白质结构分析等任务, 是此调度策略的适用者）

- **SCHED\_BATCH** 是 SCHED\_OTHER 策略的分化, 与 SCHED\_OTHER 策略一样, 但针对吞吐量优化

- **SCHED\_DEADLINE** 是新支持的实时进程调度策略, 针对突发型计算, 且对延迟和完成时间高度敏感的任务适用.


除了完成以上基本任务外, Linux 调度器还应提供高性能保障, 对吞吐量和延时的均衡要有好的优化; 要提供高可扩展性(scalability)保障, 保障上千节点的性能稳定; 对于广泛作为服务器领域操作系统来说, 它还提供丰富的组策略调度和节能调度的支持.


**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**


**目录:**

**1 抢占支持(preemption)**

**2 普通进程调度器(SCHED\_OTHER)之纠极进化史**

**3 有空时再跑 SCHED\_IDLE**

**4 吭哧吭哧跑计算 SCHED\_BATCH**

**5 十万火急, 限期完成 SCHED\_DEADLINE**

**6 普通进程的组调度支持(Fair Group Scheduling)**

**7 实时进程的组调度支持(RT Group Scheduling)**

**8 组调度带宽控制(CFS bandwidth control)**

**9 极大提高体验的自动组调度(Auto Group Scheduling)**

**10 基于调度域的负载均衡**

**11 更精确的调度时钟(HRTICK)**

**12 自动 NUMA 均衡(Automatic NUMA balancing)**

**13 CPU 调度与节能**


调度特性时间线

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 1991/01/01 | [示例 sched: Improve the scheduler]() | 此处填写描述【示例】 | ☑ ☒☐ v3/5.4-rc1 | [LWN](), [PatchWork](), [lkml]() |
| 2016/11/07 | [sched/rt: RT_RUNTIME_GREED sched feature](https://lore.kernel.org/patchwork/patch/732374) | 限制只有当当前 RQ 上没有 CFS 任务的时候, RT 任务才可以通过 RT_RUNTIME_SHARE 特性从其他 CPU 上窃取运行时间 | v1 ☐  | [LWN](https://lwn.net/Articles/705849/), [PatchWork](https://lore.kernel.org/patchwork/patch/732374), [lkml](https://lkml.org/lkml/2016/11/7/55) |
| 2019/6/26 | [sched/fair: Fallback to sched-idle CPU in absence of idle CPUs](https://lore.kernel.org/patchwork/cover/1094197) | CFS SCHED_NORMAL 进程在选核的时候, 之前优先选择 idle 的 CPU, 现在也倾向于选择只有 SCHED_IDLE 的进程在运行的 CPU | v3 ☑ 5.4-rc1 | [LWN](https://lwn.net/Articles/805317), [PatchWork](https://lore.kernel.org/patchwork/cover/1094197), [lkml](https://lkml.org/lkml/2019/6/26/16) |
| 2019/6/26 | [BT scheduling class](https://lore.kernel.org/patchwork/cover/1092086) | 离线调度策略, 腾讯为低时延业务实现的调度类, 调度类优先级在 CFS 之下. |  v1 ☐ | [LWN](https://lwn.net/Articles/791681), [PatchWork](https://lore.kernel.org/patchwork/cover/1092086), [lkml](https://lkml.org/lkml/2019/6/21/77) |
| 2018/12/6 | [steal tasks to improve CPU utilization](https://lore.kernel.org/patchwork/cover/1022417) | idle_balance 的补偿策略, 在 CPU 即将进入 idle 的时候, 通过从其他 CPU 上偷取线程来提升系统整体的吞吐量 | v4 ☐ | [LWN](), [PatchWork](https://lore.kernel.org/patchwork/cover/1022417), [lkml](https://lkml.org/lkml/2018/12/6/1253), [coding](http://www.linux-arm.org/git?p=linux-vs.git;a=shortlog;h=refs/heads/mainline/cfs-stealing/v4-rebase) |
| 2019/6/26 | [Scheduler soft affinity](https://lwn.net/Articles/793492) | CPU 软亲和力特性, 对进程指定一组软亲和的 CPU, 当 CPU 并不都很忙的时候, 则可以使用它们, 即使这组 CPU 不在它的 cpus_allowed 列表中 | RFC v1 ☐ | [LWN](https://lwn.net/Articles/793492), [PatchWork](https://lore.kernel.org/patchwork/cover/1094525), [lkml](https://lkml.org/lkml/2019/6/26/1044) |
| 2019/7/1 | [Improve scheduler scalability for fast path](https://lore.kernel.org/patchwork/cover/1094549) | select_idle_cpu 每次遍历 LLC 域查找空闲 CPU 的代价非常高, 因此通过限制搜索边界来减少搜索时间, 进一步通过保留 PER_CPU 的 next_cpu 变量来跟踪上次搜索边界, 来缓解此次优化引入的线程局部化问题  | v3 ☐ | [LWN](https://lwn.net/Articles/757379/), [PatchWork](https://lore.kernel.org/patchwork/cover/1094549/), [lkml](https://lkml.org/lkml/2019/7/1/450), [Blog](https://blogs.oracle.com/linux/linux-scheduler-scalabilty-like-a-boss) |
| 2019/9/6 | [sched: Add micro quanta scheduling class](https://lkml.org/lkml/2019/9/6/178) | 支持微秒级调度间隔的轻量级低时延调度类 | v1 ☐ | [PatchWork](https://lore.kernel.org/patchwork/cover/1125037/), [lkml](https://lkml.org/lkml/2019/9/6/178) |
| 2019/9/6 | [sched,fair: flatten CPU controller runqueues](https://lore.kernel.org/patchwork/cover/1125295/)) | 当前组调度 RQ 层次结构较深, 增加了每秒大量唤醒时的工作开销, 因此通过将所有进程放在同一层次的 RQ 上来解决此问题. | RFC v5 ☐  | [LWN](https://lwn.net/Articles/791072), [PatchWork](https://lore.kernel.org/patchwork/cover/1125295), [lkml](https://lkml.org/lkml/2019/9/6/853) |
| 2020/1/21 | [TurboSched: A scheduler for sustaining Turbo Frequencies for longer durations](https://lwn.net/Articles/793498)) | 小任务封包的又一次场景尝试, 将一组任务打包在一起之后, 然后把这个 CPU 超频运行 | v6 ☐ | [LWN](https://lwn.net/Articles/793498), [PatchWork](https://lore.kernel.org/patchwork/cover/1182559), [lkml](https://lkml.org/lkml/2020/1/21/39) |
| 2020/1/22 | [Add support for frequency invariance for (some) x86](https://lore.kernel.org/patchwork/cover/1183773) | 为 X86_64 实现频率标度不变性, PELT 算法计算利用率时感知频率的变化, 之前只有 ARM64 实现了此特性 | v5 ☑ 5.7-rc1 | [LWN](https://lwn.net/Articles/793393), [PatchWork](https://lore.kernel.org/patchwork/cover/1183773), [lkml](https://lkml.org/lkml/2020/1/22/1038) |
| 2020/2/21 | [Introduce Thermal Pressure](https://lore.kernel.org/patchwork/cover/1198915) | 温控会限制 CPU 的最大频率, 进而影响 CPU capacity, 因此 PELT 负载跟踪时需要感知温控 | v10 ☑ 5.7-rc1 | [LWN](https://lwn.net/Articles/807428/), [PatchWork](https://lore.kernel.org/patchwork/cover/1198915), [lkml](https://lkml.org/lkml/2020/2/21/2138) |
| 2020/2/24 | [Task latency-nice](https://lwn.net/Articles/820659) | 告诉调度器 per-task 的 latency 需求, 这个进程必须在预期 latency 之内赶快运行起来. | v4 ☐ | [Subhra Mazumdar](https://lore.kernel.org/patchwork/cover/1122405)<br>*-*-*-*-*-*-*-* <br>[LWN](https://lwn.net/Articles/798194), [PatchWork](https://lore.kernel.org/patchwork/cover/1199395), [lkml](https://lkml.org/lkml/2020/2/24/216) |
| 2020/5/7 | [IDLE gating in presence of latency-sensitive tasks]() | 为 ltency-nice 所做的优化, 在 latency-nice 的基础上, 与 CPU_IDLE 结合, 当 CPU 上存在 latency-nice 的进程时, 则阻止 CPU 陷入更深层次的睡眠, 从而降低唤醒延迟. | RFC v1 ☐ | [LWN](https://lwn.net/Articles/819784), [PatchWork](https://lore.kernel.org/patchwork/cover/1237681), [lkml](https://lkml.org/lkml/2020/5/7/575) |
| 2020/11/23 | [support "task_isolation" mode](https://lwn.net/Articles/816298) | NO_HZ_FULL 的进一步优化, 进一步降低 tick 等对隔离核的影响 | v5 ☐ | [2016 Chris Metcalf v16](https://lore.kernel.org/patchwork/cover/847460)<br>*-*-*-*-*-*-*-* <br>Alex Belits 2020 [LWN](https://lwn.net/Articles/813804), [PatchWork](https://lore.kernel.org/patchwork/cover/1344134), [lkml](https://lkml.org/lkml/2020/11/23/1380) |
| 2020/12/14 | [select_idle_sibling() wreckage](https://lore.kernel.org/patchwork/cover/1353496) | 重构 SIS_PROP 的逻辑, 重新计算 CPU 的扫描成本, 同时归一 select_idle_XXX 中对 CPU 的遍历, 统一选核的搜索逻辑来降低开销, 提升性能 | RFC | [PatchWork](https://lore.kernel.org/patchwork/cover/1353496), [lkml](https://lkml.org/lkml/2020/12/14/560) |



社区几个调度的大神
[Mel Gorman mgorman@techsingularity.net](https://lore.kernel.org/patchwork/project/lkml/list/?submitter=19167)
[Alex Shi](https://lore.kernel.org/patchwork/project/lkml/list/?submitter=25695&state=%2A&series=&q=&delegate=&archive=both)

**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**




# 1.1 进程调度类
-------

## 1.1.1 普通进程调度器(SCHED\_OTHER)之纠极进化史
-------

Linux 一开始, 普通进程和实时进程都是基于优先级的一个调度器, 实时进程支持 100 个优先级, 普通进程是优先级小于实时进程的一个静态优先级, 所有普通进程创建时都是默认此优先级, 但可通过 **nice()** 接口调整动态优先级(共40个). 实时进程的调度器比较简单, 而普通进程的调度器, 则历经变迁[<sup>5</sup>](#refer-anchor-5):



## 1.1.1.1 O(1) 调度器:
-------

2.6 时代开始支持(2002年引入).

顾名思义, 此调度器为O(1)时间复杂度. 该调度器修正之前的O(n) 时间复杂度调度器, 以解决扩展性问题. 为每一个动态优先级维护队列, 从而能在常数时间内选举下一个进程来执行.


## 1.1.1.2 夭折的 RSDL(The Rotating Staircase Deadline Scheduler)调度器
-------

**2007 年 4 月提出, 预期进入 2.6.22, 后夭折.**



O(1) 调度器存在一个比较严重的问题: 复杂的交互进程识别启发式算法 - 为了识别交互性的和批处理型的两大类进程, 该启发式算法融入了睡眠时间作为考量的标准, 但对于一些特殊的情况, 经常判断不准, 而且是改完一种情况又发现一种情况.


Con Kolivas (八卦: 这家伙白天是个麻醉医生)为解决这个问题提出 **RSDL（The Rotating Staircase Deadline Scheduler)** 算法. 该算法的亮点是对公平概念的重新思考: **交互式(A)**和**批量式(B)**进程应该是被完全公平对待的, 对于两个动态优先级完全一样的 A, B 进程, **它们应该被同等地对待, 至于它们是交互式与否(交互式的应该被更快调度),　应该从他们对分配给他们的时间片的使用自然地表现出来, 而不是应该由调度器自作高明地根据他们的睡眠时间去猜测**. 这个算法的核心是**Rotating Staircase**, 是一种衰减式的优先级调整, 不同进程的时间片使用方式不同, 会让它们以不同的速率衰减(在优先级队列数组中一级一级下降, 这是下楼梯这名字的由来), 从而自然地区分开来进程是交互式的(间歇性的少量使用时间片)和批量式的(密集的使用时间片). 具体算法细节可看这篇文章: [The Rotating Staircase Deadline Scheduler [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/224865/)



## 1.1.1.3 完全公平的调度器(CFS)
-------

**2.6.23(2007年10月发布)**

Con Kolivas 的完全公平的想法启发了原 O(1) 调度器作者 Ingo Molnar, 他重新实现了一个新的调度器, 叫 CFS(Completely Fair Scheduler). 新调度器的核心同样是**完全公平性,** 即平等地看待所有普通进程, 让它们自身行为彼此区分开来, 从而指导调度器进行下一个执行进程的选举.


具体说来, 此算法基于一个理想模型. 想像你有一台无限个 相同计算力的 CPU, 那么完全公平很容易, 每个 CPU 上跑一个进程即可. 但是, 现实的机器 CPU 个数是有限的, 超过 CPU 个数的进程数不可能完全同时运行. 因此, 算法为每个进程维护一个理想的运行时间, 及实际的运行时间, 这两个时间差值大的, 说明受到了不公平待遇, 更应得到执行.


至于这种算法如何区分交互式进程和批量式进程, 很简单. 交互式的进程大部分时间在睡眠, 因此它的实际运行时间很小, 而理想运行时间是随着时间的前进而增加的, 所以这两个时间的差值会变大. 与之相反, 批量式进程大部分时间在运行, 它的实际运行时间和理想运行时间的差距就较小. 因此, 这两种进程被区分开来.


CFS 的测试性能比 RSDS 好, 并得到更多的开发者支持, 所以它最终替代了 RSDL 在 2.6.23 进入内核, 一直使用到现在.


## 1.1.1.4 CK 的 BFS 和 MuQSS
-------

可以八卦的是, Con Kolivas 因此离开了社区, 不过他本人否认是因为此事而心生龃龉. 后来, 2009 年, 他对越来越庞杂的 CFS 不满意, 认为 CFS 过分注重对大规模机器, 而大部分人都是使用少 CPU 的小机器, 因此于 2009年8月31日发布了 BFS 调度器(Brain Fuck Scheduler)[<sup>48</sup>](#refer-anchor-48).

BFS调度器的原理十分简单, 是为桌面交互式应用专门设计, 使得用户的桌面环境更为流畅, 早期使用CFS编译内核时, 音讯视讯同时出现会出现严重的停顿（delay）, 而使用 BFS 则没有这些问题. 【注意】

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

## 1.1.1.4 不那么重要的进程 SCHED\_IDLE
-------

**2.6.23(2007年10月发布)**


~~此调度策略和 CFS 调度器在同一版本引入. 系统在空闲时, 每个 CPU 都有一个 idle 线程在跑, 它什么也不做, 就是把 CPU 放入硬件睡眠状态以节能(需要特定CPU的driver支持), 并等待新的任务到来, 以把 CPU 从睡眠状态中唤醒. 如果你有任务想在 CPU 完全 idle 时才执行, 就可以用 **sched\_setscheduler()** API 设置此策略.~~


注意上面的描述是有问题的, SCHED_IDLE 和 idle 进程是完全不同的东西

SCHED_IDLE 跟 SCHED_BATCH 一样, 是 CFS 中的一个策略, SCHED\_IDLE 的进程也是 CFS 调度类的一员, CFS内部的家务事, 意味着 SCHED\_IDLE 进程也是需要按照权重来分配CPU时间的, 只是权重很低而已.

另一方面, SCHED_IDLE 是用户赋予的, 的确可以用 **sched\_setscheduler()** API 设置此策略

使用了 SCHED_IDLE 策略, 这意味着这些进程是不重要的, 但是 CFS 又号称完全公平, 这体现在哪些方面呢?

- 首先设置了 SCHED_IDLE 策略的进程优先级(nice值)都很低, 这将影响到进程的时间片和负载信息
- SCHED_IDLE 调度策略只在进行抢占处理的时候有一些特殊处理, 比如 check_preempt_curr() 中, 这里当前正在运行的如果是 SCHED_IDLE task 的话, 会马上被新唤醒的 SCHED_NORMAL task抢占, 即 SCHED_NORMAL 的进程可以抢占 SCHED_IDLE 的进程.

因此 Linux 社区里面并没有多少人会使用 SCHED_IDLE 调度策略, 因此自从Linux 2.6.23 引入之后, 就没人对它进行改进.


注意, 在 select_task_rq() 调用中并没有针对 SCHED\_IDLE 调度策略的相应处理, 因此我们并没能做到尽量把新唤醒的 SCHED_NORMAL task 放到当前正在运行 SCHED_IDLE task 的 CPU 上去. 之前的 select_task_rq_fair() 中更倾向于寻找一个 IDLE 的 CPU.

那么这就有一个矛盾的地方了, 那就是在选核的时候, 如果当前 CPU 上正运行 SCHED_IDLE 的进程. 那么选择这样的 CPU 更合适, 还是继续往下寻找 idle 的 CPU 更合适?

*   当前的策略是倾向于是唤醒完全 idle 的 CPU, 而保持 SCHED_IDLE 进程继续占有原来的CPU. 这显然与 SCHED_NORMAL 可以抢占 SCHED_IDLE 的初衷相违背. 这样的好处是系统会更加均衡一些, 但是另外一方面唤醒 idle 的 CPU 是有一定延迟的, 在当前的 CPU 上这样的操作往往可能耗时若干 ms 的时间, 远远比一次抢占的时间要长.

*   另外一种策略就是抢占 SCHED_IDLE 进程, 这样可以保持完全 idle 的 CPU 继续 idle, 降低功耗;

第二种策略虽然从负载均衡看来, 系统貌似不那么均衡了, 但是看起来有诸多的好处. 于是一组特性 [sched/fair: Fallback to sched-idle CPU in absence of idle CPUs](https://lwn.net/Articles/805317), [patchwork](https://lore.kernel.org/patchwork/cover/1094197), [lkml-2019-06-26](https://lkml.org/lkml/2019/6/26/16) 被提出, 并于 5.4-rc1 合入主线.





## 1.1.1.5 吭哧吭哧跑计算 SCHED\_BATCH
-------

**2.6.16(2006年3月发布)**


概述中讲到 SCHED\_BATCH 并非 POSIX 标准要求的调度策略, 而是 Linux 自己额外支持的.
它是从 SCHED\_OTHER 中分化出来的, 和 SCHED\_OTHER 一样, 不过该调度策略会让采用策略的进程比 SCHED\_OTHER 更少受到 调度器的重视. 因此, 它适合非交互性的, CPU 密集运算型的任务. 如果你事先知道你的任务属于该类型, 可以用 **sched\_setscheduler()** API 设置此策略.


在引入该策略后, 原来的 SCHED\_OTHER 被改名为 SCHED\_NORMAL, 不过它的值不变, 因此保持 API 兼容, 之前的 SCHED\_OTHER 自动成为 SCHED\_NORMAL, 除非你设置 SCHED\_BATCH.

## 1.1.2  SCHED\_RT
-------

RT 有两种调度策略, SCHED_FIFO 先到先服务 和 SCHED_RR 时间片轮转

系统中的实时进程将比 CFS 优先得到调度, 实时进程根据实时优先级决定调度权值

RR进程和FIFO进程都采用实时优先级做为调度的权值标准, RR是FIFO的一个延伸. FIFO时, 如果两个进程的优先级一样, 则这两个优先级一样的进程具体执行哪一个是由其在队列中的位置决定的, 这样导致一些不公正性(优先级是一样的, 为什么要让你一直运行?),如果将两个优先级一样的任务的调度策略都设为RR,则保证了这两个任务可以循环执行, 保证了公平.


SHCED_RR和SCHED_FIFO的不同：

1.  当采用SHCED_RR策略的进程的时间片用完, 系统将重新分配时间片, 并置于就绪队列尾. 放在队列尾保证了所有具有相同优先级的RR任务的调度公平.

2.  SCHED_FIFO一旦占用cpu则一直运行. 一直运行直到有更高优先级任务到达或自己放弃.

3.  如果有相同优先级的实时进程（根据优先级计算的调度权值是一样的）已经准备好, FIFO时必须等待该进程主动放弃后才可以运行这个优先级相同的任务. 而RR可以让每个任务都执行一段时间.



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

> 通过设置/proc/sys/kernel/sched_rt_runtime_us 和/proc/sys/kernel/sched_rt_period_us配合实现; sched_rt_period_us默认值是1s(1000000us), sched_rt_runtime_us默认是0.95s（950000us）; 通过此项配置的RT进程利用率, 是针对整个CPU的, 对于多核处理器, 每个CPU仍然可以跑到100%, 那么怎么让每cpu的利用率都是95%呢？可以设置
> echo NO_RT_RUNTIME_SHARE > /sys/kernel/debug/sched_features; 这样每个核不去借用别的cpu时间, 可以达到95%的限制




## 1.1.3 十万火急, 限期完成 SCHED\_DEADLINE
-------

**3.14(2014年3月发布)**


此策略支持的是一种实时任务. 对于某些实时任务, 具有阵发性(sporadic),　它们阵发性地醒来执行任务, 且任务有 deadline 要求, 因此要保证在 deadline 时间到来前完成. 为了完成此目标, 采用该 SCHED\_DEADLINE 的任务是系统中最高优先级的, 它们醒来时可以抢占任何进程.


如果你有任务属于该类型, 可以用 **_sched\_setscheduler()_** 或 **_sched\_setattr()_** API 设置此策略.


更多可参看此文章: [Deadline scheduling: coming soon? [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/575497/)


## 1.1.4 其他一些调度类的尝试
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


# 1.2 组调度支持(Group Scheduling)
-------

## 1.2.1 普通进程的组调度支持(Fair Group Scheduling)
-------

**2.6.24(2008年１月发布)**


2.6.23 引入的 CFS 调度器对所有进程完全公平对待. 但这有个问题, 设想当前机器有２个用户, 有一个用户跑着 9个进程, 还都是 CPU 密集型进程; 另一个用户只跑着一个 X 进程, 这是交互性进程. 从 CFS 的角度看, 它将平等对待这 10 个进程, 结果导致的是跑 X 进程的用户受到不公平对待, 他只能得到约 10% 的 CPU 时间, 让他的体验相当差.


基于此, 组调度的概念被引入[<sup>6</sup>](#refer-anchor-6). CFS 处理的不再是一个进程的概念, 而是调度实体(sched entity), 一个调度实体可以只包含一个进程, 也可以包含多个进程. 因此, 上述例子的困境可以这么解决: 分别为每个用户建立一个组, 组里放该用户所有进程, 从而保证用户间的公平性.


该功能是基于控制组(control group, cgroup)的概念, 需要内核开启 CGROUP 的支持才可使用. 关于 CGROUP , 以后可能会写.


## 1.2.2 实时进程的组调度支持(RT Group Scheduling)
-------


**2.6.25(2008年4月发布)**

该功能同普通进程的组调度功能一样, 只不过是针对实时进程的.


## 1.2.3 组调度带宽控制(CFS bandwidth control)** , **3.2(2012年1月发布)**
-------


组调度的支持, 对实现多租户系统的管理是十分方便的, 在一台机器上, 可以方便对多用户进行 CPU 均分．然后, 这还不足够, 组调度只能保证用户间的公平, 但若管理员想控制一个用户使用的最大 CPU 资源, 则需要带宽控制．3.2 针对 CFS组调度, 引入了此功能[<sup>8</sup>](#refer-anchor-8), 该功能可以让管理员控制在一段时间内一个组可以使用 CPU 的最长时间．



## 1.2.4 极大提高体验的自动组调度(Auto Group Scheduling)
-------

**2.6.38(2011年3月发布)**

试想, 你在终端里熟练地敲击命令, 编译一个大型项目的代码, 如 Linux内核, 然后在编译的同时悠闲地看着电影等待, 结果电脑却非常卡, 体验一定很不爽．



2.6.38 引入了一个针对桌面用户体验的改进, 叫做自动组调度．短短400多行代码[<sup>9</sup>](#refer-anchor-9), 就很大地提高了上述情形中桌面使用者体验, 引起不小轰动．



其实原理不复杂, 它是基于之前支持的组调度的一个延伸．Unix 世界里, 有一个**会话(session)** 的概念, 即跟某一项任务相关的所有进程, 可以放在一个会话里, 统一管理．比如你登录一个系统, 在终端里敲入用户名, 密码, 然后执行各种操作, 这所有进程, 就被规划在一个会话里．



因此, 在上述例子里, 编译代码和终端进程在一个会话里, 你的浏览器则在另一个会话里．自动组调度的工作就是, 把这些不同会话自动分成不同的调度组, 从而利用组调度的优势, 使浏览器会话不会过多地受到终端会话的影响, 从而提高体验．



该功能可以手动关闭.


# 1.3 负载跟踪机制
-------

## 1.3.1 WALT
-------

原谅我在这里跳过了主线默认强大的 PELT, 而先讲 WALT.

[improving exynos 9810 galaxy s9](https://www.anandtech.com/show/12620/improving-the-exynos-9810-galaxy-s9-part-2)

## 1.3.2 PELT
-------

从Arm的资源来看, 这很像该公司意识到性能问题, 并正在积极尝试改善 PELT 的行为以使其更接近 WALT.

1.  一个重要的变化是称为 [util_est 利用率估计的特性](http://retis.santannapisa.it/~luca/ospm-summit/2017/Downloads/OSPM_PELT_DecayClampingVsUtilEst.pdf), [Utilization estimation (util_est) for FAIR tasks](https://lore.kernel.org/patchwork/cover/932237)

2.  改善PELT的另一种简单方法是[减少斜坡/衰减时间](https://lore.kernel.org/lkml/20180409165134.707-1-patrick.bellasi@arm.com/#r), 主线默认的 PELT 衰减周期为 32MS, 该补丁提供了 8MS/16MS/32MS 的可选择衰减周期. 通常的测试结果会认为 8ms 的半衰期是一种偏性能的选择, 默认的 32ms 设置, 无法满足终端场景突发的负载变化, 因此往往 16ms 的折中方案能提供最佳性能和电池折衷.




# 1.4 select_task_rq
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



## 1.4.1 机制的 WAKE_AFFINE
-------

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2013/07/04 | [sched: smart wake-affine](https://lore.kernel.org/patchwork/cover/390846) | 引入wakee 翻转次数, 通过巧妙的启发式算法, 识别系统中 1:N/N:M 等唤醒模型, 作为是否进行 wake_affine 的依据 | V3 3.12-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/390846)<br>*-*-*-*-*-*-*-* <br>[commit 1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=62470419e993f8d9d93db0effd3af4296ecb79a5), [commit2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7d9ffa8961482232d964173cccba6e14d2d543b2) |
| | [Reduce scheduler migrations due to wake_affine](https://lore.kernel.org/patchwork/cover/864391) | 优化 wake_affine 减少迁移次数 | | | |

## 1.4.2 提升 CPU 的查找效率
-------

每次为进程选择一个合适的 CPU 的时候, 较好的情况可以通过 wake_affine 等走快速路径, 但是最坏的情况下, 却不得不遍历当前 SD 查找一个 IDLE CPU 或者负载较小的 CPU.
这个查找是一项大工程, 在调度里面几乎是觉难以容忍的. 因此这里一直是性能优化的战场, 炮火味十足.

2020 年 12 月 15 日, 调度的大 Maintainer Peter Zijlstra, 曾公开抨击选核的慢速流程里面[部分代码, "The thing is, the code as it exists today makes no sense what so ever. It's plain broken batshit."](https://lkml.org/lkml/2020/12/15/93).

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2018/05/30 | [select_idle_sibling rework](https://lore.kernel.org/patchwork/patch/911697) | 优化 select_idle_XXX 的性能 | RFC | [select_idle_sibling rework](https://lore.kernel.org/patchwork/patch/911697) |
| 2019/7/1 | [Improve scheduler scalability for fast path](https://lore.kernel.org/patchwork/cover/1094549) | select_idle_cpu 每次遍历 LLC 域查找空闲 CPU 的代价非常高, 因此通过限制搜索边界来减少搜索时间, 进一步通过保留 PER_CPU 的 next_cpu 变量来跟踪上次搜索边界, 来缓解此次优化引入的线程局部化问题  | v3 ☐ | [LWN](https://lwn.net/Articles/757379/), [PatchWork](https://lore.kernel.org/patchwork/cover/1094549/), [lkml](https://lkml.org/lkml/2019/7/1/450), [Blog](https://blogs.oracle.com/linux/linux-scheduler-scalabilty-like-a-boss) |
| 2018/01/30 | [Reduce migrations and unnecessary spreading of load to multiple CPUs](https://lore.kernel.org/patchwork/cover/878789) | 通过优化选核逻辑, 减少不必要的迁移 | | |
| 2019/1/21 | [sched/fair: Optimize select_idle_core](https://lore.kernel.org/patchwork/patch/1163807) | | | |
| 2020/03/11 | [sched: Streamline select_task_rq() & select_task_rq_fair()](https://lore.kernel.org/patchwork/patch/1208449) | 选核流程上的重构和优化, 当然除此之外还做了其他操作, 比如清理了 sd->flags 信息, 甚至 sysfs 接口都变成只读了 | | |
| 2019/07/08 | [Optimize the idle CPU search](https://lore.kernel.org/patchwork/patch/1098092) | 通过标记 idle_cpu 来降低 select_idle_sibling/select_idle_cpu 的搜索开销 | | |
| 2020/12/03 | [Reduce time complexity of select_idle_sibling](https://lore.kernel.org/patchwork/cover/1348877) | 通过自己完善的 schedstat 的统计信息, 发现 select_idle_XXX 中不合理的地方(提高了p->recent_used_cpu的命中率. 以减少扫描开销, 同时如果在扫描a时发现了一个候选, 那么补丁4将返回一个空闲的候选免费的核心等), 降低搜索开销 | RFC v3 | 这组补丁其实有很多名字, 作者发了几版本之后, 不断重构, 也改了名字<br>*-*-*-*-*-*-*-* <br>2020/12/07 [Reduce scanning of runqueues in select_idle_sibling](https://lore.kernel.org/patchwork/cover/1350876)<br>*-*-*-*-*-*-*-* <br>2020/12/07 [Reduce worst-case scanning of runqueues in select_idle_sibling](https://lore.kernel.org/patchwork/patch/1350249)<br>*-*-*-*-*-*-*-* <br>2020/12/08 [Reduce scanning of runqueues in select_idle_sibling](https://lore.kernel.org/patchwork/patch/1350877)  |
| 2020/12/08 | | 跟上一个其实是一组补丁, 重构后改了名字. | | |
| 2020/12/14 | [select_idle_sibling() wreckage](https://lore.kernel.org/patchwork/cover/1353496) | 重构 SIS_PROP 的逻辑, 重新计算 CPU 的扫描成本, 同时归一 select_idle_XXX 中对 CPU 的遍历, 统一选核的搜索逻辑来降低开销, 提升性能 | RFC | [PatchWork](https://lore.kernel.org/patchwork/cover/1353496), [LKML](https://lkml.org/lkml/2020/12/14/560) |

# 1.5 基于调度域的负载均衡
-------

## 1.5.1 负载均衡
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


## 1.5.2 自动 NUMA 均衡(Automatic NUMA balancing)
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

## 1.5.3 rework_load_balance
-------

2019 年的 [Vincent Guittot](https://www.youtube.com/watch?v=cfv63BMnIug) 的 [sched/fair: rework the CFS load balance](https://lwn.net/Articles/793427) 是近几年特别有亮点的补丁.
. 博主个人一直是计算机先驱"高德纳"教授"文学化编程"思想的坚定追随者, 小米创始人雷军雷布斯先生也说"写代码要有写诗一样的感觉". 这种代码才真的让人眼前一亮, 如沐春风. 这个就是我看到 [rework_load_balance 这组补丁](https://lore.kernel.org/patchwork/cover/1141687) 的感觉. 这组补丁通过重构 (CFS) load_balance 的逻辑, 将原来逻辑混乱的 load_balance 变成了内核中一抹亮丽的风景, 不光使得整个 load_balance 的框架更清晰, 可读性更好. 更带来了性能的提升.

它将系统中调度组的状态[归结于集中类型](https://lore.kernel.org/patchwork/patch/1141698), 对于其中的负载不均衡状态分别采用不同的处理方式.


| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2019/10/18 | [sched/fair: rework the CFS load balance](https://linuxplumbersconf.org/event/4/contributions/480) | 重构 load balance | v4 ☑ | [LWN](https://lwn.net/Articles/793427), [PatchWork](https://lore.kernel.org/patchwork/patch/1141687), [lkml](https://lkml.org/lkml/2019/10/18/676) |
| 2019/10/22 | [sched/fair: fix rework of find_idlest_group()](https://lore.kernel.org/patchwork/patch/1143049) | fix 补丁 | | |
| 2019/11/29 | [sched/cfs: fix spurious active migration](https://lore.kernel.org/patchwork/patch/1160934) | fix 补丁 | | |
| 2019/12/20 | [sched/fair : Improve update_sd_pick_busiest for spare capacity case 1171109 diffmboxseries](https://lore.kernel.org/patchwork/patch/1171109/) | | |

在 Vincent 进行重构的基础上, Mel Gorman 也进行了 NUMA Balancing 的重构和修正 [Reconcile NUMA balancing decisions with the load balancer](https://lore.kernel.org/patchwork/cover/1199507).

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2019/10/18 | [Reconcile NUMA balancing decisions with the load balancer](https://linuxplumbersconf.org/event/4/contributions/480) | 重构 load balance | v4 ☑ | [LWN](https://lwn.net/Articles/793427), [PatchWork](https://lore.kernel.org/patchwork/cover/1199507), [lkml](https://lkml.org/lkml/2019/10/18/676) |
| 2020/02/11 | [sched/numa: replace runnable_load_avg by load_avg](https://lore.kernel.org/patchwork/patch/1192634) | 优化补丁 | | |
| | [Accumulated fixes for Load/NUMA Balancing reconcilation series](https://lore.kernel.org/patchwork/cover/1203922) | fix 补丁 | | |


此外 load_balance 上还有一些接口层次的变化

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2020/03/11 | [sched: Streamline select_task_rq() & select_task_rq_fair()](https://lore.kernel.org/patchwork/patch/1208449) | 选核流程上的重构和优化, 当然除此之外还做了其他操作, 比如清理了 sd->flags 信息, 甚至 sysfs 接口都变成只读了 | | |
| 2020/03/11 | [sched: Instrument sched domain flags](https://lore.kernel.org/patchwork/cover/1208463) | 基于上一组补丁, 重构了 SD_FLAGS 的定义 | | |



# 1.6 pick_next_task
-------



# 1.7 **调度与节能**
-------


从节能角度讲, 如果能维持更多的 CPU 处于深睡眠状态, 仅保持必要数目的 CPU 执行任务, 就能更好地节约电量, 这对笔记本电脑来说, 尤其重要．然而这不是一个简单的工作, 这涉及到负载均衡, 调度器, 节能模块的并互, Linux 调度器中曾经有相关的代码, 但后来发现问题, 在 3.5, 3.6 版本中, 已经把相关代码删除．整个问题需要重新思考．



在前不久, 一个新的 patch 被提交到 Linux 内核开发邮件列表, 这个问题也许有了新的眉目, 到时再来更新此小节．可阅读此文章: [Steps toward power-aware scheduling [LWN.net]](https://lwn.net/Articles/655479/)


## 1.7.1 小任务封包
-------

小任务封包(Small Task Packing) 是内核调度特性中少数几个隔几年就会被人换个马甲发出来的特性之一.

早在 2012 年, Linaro 的 Vincent Guittot 发出了一版 [Small-task packing](https://lwn.net/Articles/520857) 的补丁, 通过将负载较轻的任务打包在尽量少的 CPU 上, 从而在保证系统性能的前提下, 尽可能的降低功耗.
该补丁最终发到 v5 版本 [PatchWork](https://lore.kernel.org/patchwork/cover/414759), [lkml](https://lkml.org/lkml/2013/10/18/121), [git-sched-packing-small-tasks-v5](http://git.linaro.org/people/vincent.guittot/kernel.git/log/?h=sched-packing-small-tasks-v5) 后, 没有了下文.

但是请大家记住这个特性, 这个特性将在后面的特性中不断被提及并实现. 它作为一个先驱者, 为终端功耗场景在调度领域做了最初的一次尝试. 自此开始调度领域一场旷日持久的性能 vs 功耗的战役被打响.


## 1.7.2 能耗感知的调度器
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

## 1.7.3 IKS -> HMP -> EAS
-------


[The power-aware scheduling mini-summit](https://lwn.net/Articles/571414)

ARM 的 Morten Rasmussen 一直致力于ANDROID 调度器优化的:

1.  最初版本的 IKS 调度器, [ELC: In-kernel switcher for big.LITTLE](https://lwn.net/Articles/539840)

2.  以及后来先进的 HMP 调度器 [LC-Asia: A big LITTLE MP update](https://lwn.net/Articles/541005), 虽然我们称呼他是一个调度器, 但是其实他的策略只是一个**进程放置(Task Placement, 请原谅我的这个翻译, 很抱歉我实在不知道翻译成什么好, 如果您有什么好的建议, 请联系我)** 的策略. [sched: Task placement for heterogeneous MP systems](https://lore.kernel.org/patchwork/cover/327745)

3.  直到看到了能耗感知调度器, 忽然眼前有了光, Morten 充分借鉴了能耗感知调度器的思想, 提出了自己的想法[Another attempt at power-aware scheduling](https://lwn.net/Articles/600419), 最终发布到 v2 版本 [Power-aware scheduling v2](https://lore.kernel.org/patchwork/cover/412619).

4.  在 [Power-aware scheduling v2](https://lore.kernel.org/patchwork/cover/412619) 发布之后 版本之后. Morten 有了更多的奇思妙想, 提出了能耗模型(energy cost model) 的雏形, 以及 energy diff 的思想, 从此一发不可收拾. 可以认为这个就是 EAS 最早的雏形, 并最终发到了 2015/07/07 v5 版本 [sched: Energy cost model for energy-aware scheduling](https://lore.kernel.org/patchwork/cover/576661).

5.  EAS 带来了划时代的想法, 最终 [Quentin Perret](http://www.linux-arm.org/git?p=linux-qp.git;a=summary) 接手了 Morten Rasmussen 的工作, 最终在 2018/10/03 v10 版本将 EAS 合入主线 [https://lore.kernel.org/patchwork/cover/1020432/](https://lore.kernel.org/patchwork/cover/1020432)


## 1.7.4
[scheduler-driven cpu frequency selection](https://lwn.net/Articles/649593)


# 1.8 实时性 linux PREEMPT_RT
-------


## 1.8.1 抢占支持(preemption)
-------

**2.6 时代开始支持** (首次在2.5.4版本引入[<sup>37</sup>](#refer-anchor-37), 感谢知友 [@costa](https://www.zhihu.com/people/78ceb98e7947731dc06063f682cf9640) 考证! 关于 Linux 版本规则,  可看我文章[<sup>4</sup>](#refer-anchor-4).


可抢占性, 对一个系统的调度延时具有重要意义. 2.6 之前, 一个进程进入内核态后, 别的进程无法抢占, 只能等其完成或退出内核态时才能抢占, 这带来严重的延时问题, 2.6 开始支持内核态抢占.


## 1.8.2 NO_HZ
-------


## 1.8.3 task/CPU 隔离
-------

| 2020/11/23 | [support "task_isolation" mode](https://lwn.net/Articles/816298) | NO_HZ_FULL 的进一步优化, 进一步降低 tick 等对隔离核的影响 | v5 ☐ | [2016 Chris Metcalf v16](https://lore.kernel.org/patchwork/cover/847460)<br>*-*-*-*-*-*-*-* <br>Alex Belits 2020 [LWN](https://lwn.net/Articles/813804), [PatchWork](https://lore.kernel.org/patchwork/cover/1344134), [lkml](https://lkml.org/lkml/2020/11/23/1380) |


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




## 1.8.4 更精确的调度时钟(HRTICK), 2.6.25(2008年4月发布)**
-------


CPU的周期性调度, 和基于时间片的调度, 是要基于时钟中断来触发的．一个典型的 1000 HZ 机器, 每秒钟产生 1000 次时间中断, 每次中断到来后, 调度器会看看是否需要调度．



然而, 对于调度时间粒度为微秒(10^-6)级别的精度来说, 这每秒 1000 次的粒度就显得太粗糙了．



2.6.25 引入了所谓的**高清嘀哒(High Resolution Tick)**, 以提供更精确的调度时钟中断．这个功能是基于**高精度时钟(High Resolution Timer)框架**, 这个框架让内核支持可以提供纳秒级别的精度的硬件时钟(将会在时钟子系统里讲).


## 1.8.5 混乱的 RT 优先级(RT 进程优先级管控)
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



# 1.9 调试信息
-------

## 1.9.1 统计信息
-------

阿里的王贇 [sched/numa: introduce numa locality](https://lore.kernel.org/patchwork/cover/1190383) 提供了 per-cgroup 的 NUMASTAT 功能, 发到了 2020/02/07 v8, 但是最终还是没能合入主线.


## 1.9.2 tracepoint
-------

[`tracepoints-helpers`](https://github.com/auldp/tracepoints-helpers.git)
[`plot-nr-running`](https://github.com/jirvoz/plot-nr-running)


| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2019/06/04 | [sched: Add new tracepoints required for EAS testing](https://lore.kernel.org/patchwork/patch/1296761) | 增加 PELT 的跟踪点 | v3, 5.3-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/1296761) |
| 2020/06/19 | [Sched: Add a tracepoint to track rq->nr_running](https://lore.kernel.org/patchwork/patch/1258690) | 增加 nr_running 的跟踪点 | v1, 5.9-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/1258690)<br>*-*-*-*-*-*-*-* <br>[FixPatch](https://lore.kernel.org/patchwork/patch/1284621) |
| 2020/08/28 | [sched/debug: Add new tracepoint to track cpu_capacity](https://lore.kernel.org/patchwork/patch/1296761) | 增加 cpu_capacity 的跟踪点 |






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
