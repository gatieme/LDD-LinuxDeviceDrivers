1   调度子系统(scheduling)
=====================

**概述: **Linux 是一个遵循 POSIX 标准的类 Unix 操作系统(然而它并不是 Unix 系统[<sup>1</sup>](#refer-anchor-1)), POSIX 1003.1b 定义了调度相关的一个功能集合和 API 接口[<sup>2</sup>](#refer-anchor-2). 调度器的任务是分配 CPU 运算资源, 并以协调效率和公平为目的. **效率**可从两方面考虑: 1) 吞吐量(throughput) 2)延时(latency). 不做精确定义, 这两个有相互矛盾的衡量标准主要体现为两大类进程: 一是 CPU 密集型, 少量 IO 操作, 少量或无与用户交互操作的任务（强调吞吐量, 对延时不敏感, 如高性能计算任务 HPC), 另一则是 IO 密集型, 大量与用户交互操作的任务(强调低延时, 对吞吐量无要求, 如桌面程序). **公平**在于有区分度的公平, 多媒体任务和数值计算任务对延时和限定性的完成时间的敏感度显然是不同的.
为此,  POSIX 规定了操作系统必须实现以下**调度策略(scheduling policies),** 以针对上述任务进行区分调度:

**- SCHED\_FIFO**

**- SCHED\_RR**

这两个调度策略定义了对实时任务, 即对延时和限定性的完成时间的高敏感度的任务. 前者提

供 FIFO 语义, 相同优先级的任务先到先服务, 高优先级的任务可以抢占低优先级的任务；后 者提供 Round-Robin 语义, 采用时间片, 相同优先级的任务当用完时间片会被放到队列尾

部, 以保证公平性, 同样, 高优先级的任务可以抢占低优先级的任务. 不同要求的实时任务可

以根据需要用 **_sched\_setscheduler()_** API 设置策略.
**- SCHED\_OTHER**

此调度策略包含除上述实时进程之外的其他进程, 亦称普通进程. 采用分时策略, 根据动态优

先级(可用 **nice()** API设置）, 分配 CPU 运算资源.  **注意: 这类进程比上述两类实时进程优先级低, 换言之, 在有实时进程存在时, 实时进程优先调度**.



Linux 除了实现上述策略, 还额外支持以下策略:

- **SCHED\_IDLE** 优先级最低, **在系统空闲时才跑这类进程**(如利用闲散计算机资源跑地外文明搜索, 蛋白质结构分析等任务, 是此调度策略的适用者）

- **SCHED\_BATCH** 是 SCHED\_OTHER 策略的分化, 与 SCHED\_OTHER 策略一样, 但针对吞吐量优化

- **SCHED\_DEADLINE** 是新支持的实时进程调度策略, 针对突发型计算, 且对延迟和完成时间高度敏感的任务适用.


除了完成以上基本任务外, Linux 调度器还应提供高性能保障, 对吞吐量和延时的均衡要有好的优化；要提供高可扩展性(scalability)保障, 保障上千节点的性能稳定；对于广泛作为服务器领域操作系统来说, 它还提供丰富的组策略调度和节能调度的支持.


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



**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**


# 1.1 抢占支持(preemption)
-------

**2.6 时代开始支持** (首次在2.5.4版本引入[<sup>37</sup>](#refer-anchor-37), 感谢知友 [@costa](https://www.zhihu.com/people/78ceb98e7947731dc06063f682cf9640) 考证! 关于 Linux 版本规则,  可看我文章[<sup>4</sup>](#refer-anchor-4).


可抢占性, 对一个系统的调度延时具有重要意义. 2.6 之前, 一个进程进入内核态后, 别的进程无法抢占, 只能等其完成或退出内核态时才能抢占, 这带来严重的延时问题, 2.6 开始支持内核态抢占.


# 1.2 进程调度类
-------

## 1.2.1 普通进程调度器(SCHED\_OTHER)之纠极进化史
-------

Linux 一开始, 普通进程和实时进程都是基于优先级的一个调度器, 实时进程支持 100 个优先级, 普通进程是优先级小于实时进程的一个静态优先级, 所有普通进程创建时都是默认此优先级, 但可通过 **nice()** 接口调整动态优先级(共40个). 实时进程的调度器比较简单, 而普通进程的调度器, 则历经变迁[<sup>5</sup>](#refer-anchor-5):



## 1.2.1.1 O(1) 调度器:
-------

2.6 时代开始支持(2002年引入).

顾名思义, 此调度器为O(1)时间复杂度. 该调度器修正之前的O(n) 时间复杂度调度器, 以解决扩展性问题. 为每一个动态优先级维护队列, 从而能在常数时间内选举下一个进程来执行.


## 1.2.1.2 夭折的 RSDL(The Rotating Staircase Deadline Scheduler)调度器
-------

**2007 年 4 月提出, 预期进入 2.6.22, 后夭折.**



O(1) 调度器存在一个比较严重的问题: 复杂的交互进程识别启发式算法 - 为了识别交互性的和批处理型的两大类进程, 该启发式算法融入了睡眠时间作为考量的标准, 但对于一些特殊的情况, 经常判断不准, 而且是改完一种情况又发现一种情况.


Con Kolivas (八卦: 这家伙白天是个麻醉医生)为解决这个问题提出 **RSDL（The Rotating Staircase Deadline Scheduler)** 算法. 该算法的亮点是对公平概念的重新思考: **交互式(A)**和**批量式(B)**进程应该是被完全公平对待的, 对于两个动态优先级完全一样的 A, B 进程, **它们应该被同等地对待, 至于它们是交互式与否(交互式的应该被更快调度),　应该从他们对分配给他们的时间片的使用自然地表现出来, 而不是应该由调度器自作高明地根据他们的睡眠时间去猜测**. 这个算法的核心是**Rotating Staircase**, 是一种衰减式的优先级调整, 不同进程的时间片使用方式不同, 会让它们以不同的速率衰减(在优先级队列数组中一级一级下降, 这是下楼梯这名字的由来), 从而自然地区分开来进程是交互式的(间歇性的少量使用时间片)和批量式的(密集的使用时间片). 具体算法细节可看这篇文章: [The Rotating Staircase Deadline Scheduler [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/224865/)



## 1.2.1.3 完全公平的调度器(CFS)
-------

**2.6.23(2007年10月发布)**

Con Kolivas 的完全公平的想法启发了原 O(1) 调度器作者 Ingo Molnar, 他重新实现了一个新的调度器, 叫 CFS(Completely Fair Scheduler). 新调度器的核心同样是**完全公平性,** 即平等地看待所有普通进程, 让它们自身行为彼此区分开来, 从而指导调度器进行下一个执行进程的选举.


具体说来, 此算法基于一个理想模型. 想像你有一台无限个 相同计算力的 CPU, 那么完全公平很容易, 每个 CPU 上跑一个进程即可. 但是, 现实的机器 CPU 个数是有限的, 超过 CPU 个数的进程数不可能完全同时运行. 因此, 算法为每个进程维护一个理想的运行时间, 及实际的运行时间, 这两个时间差值大的, 说明受到了不公平待遇, 更应得到执行.


至于这种算法如何区分交互式进程和批量式进程, 很简单. 交互式的进程大部分时间在睡眠, 因此它的实际运行时间很小, 而理想运行时间是随着时间的前进而增加的, 所以这两个时间的差值会变大. 与之相反, 批量式进程大部分时间在运行, 它的实际运行时间和理想运行时间的差距就较小. 因此, 这两种进程被区分开来.


CFS 的测试性能比 RSDS 好, 并得到更多的开发者支持, 所以它最终替代了 RSDL 在 2.6.23 进入内核, 一直使用到现在. 可以八卦的是, Con Kolivas 因此离开了社区, 不过他本人否认是因为此事而心生龃龉. 后来, 2009 年, 他对越来越庞杂的 CFS 不满意, 认为 CFS 过分注重对大规模机器, 而大部分人都是使用少 CPU 的小机器, 开发了 BFS 调度器[<sup>48</sup>](#refer-anchor-48), 这个在 Android 中有使用, 没进入 Linux 内核.


## 1.2.1.4 不那么重要的进程 SCHED\_IDLE
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





## 1.2.1.5 吭哧吭哧跑计算 SCHED\_BATCH
-------

**2.6.16(2006年3月发布)**


概述中讲到 SCHED\_BATCH 并非 POSIX 标准要求的调度策略, 而是 Linux 自己额外支持的.
它是从 SCHED\_OTHER 中分化出来的, 和 SCHED\_OTHER 一样, 不过该调度策略会让采用策略的进程比 SCHED\_OTHER 更少受到 调度器的重视. 因此, 它适合非交互性的, CPU 密集运算型的任务. 如果你事先知道你的任务属于该类型, 可以用 **sched\_setscheduler()** API 设置此策略.


在引入该策略后, 原来的 SCHED\_OTHER 被改名为 SCHED\_NORMAL, 不过它的值不变, 因此保持 API 兼容, 之前的 SCHED\_OTHER 自动成为 SCHED\_NORMAL, 除非你设置 SCHED\_BATCH.



## 1.2.2 十万火急, 限期完成 SCHED\_DEADLINE
-------

**3.14(2014年3月发布)**


此策略支持的是一种实时任务. 对于某些实时任务, 具有阵发性(sporadic),　它们阵发性地醒来执行任务, 且任务有 deadline 要求, 因此要保证在 deadline 时间到来前完成. 为了完成此目标, 采用该 SCHED\_DEADLINE 的任务是系统中最高优先级的, 它们醒来时可以抢占任何进程.


如果你有任务属于该类型, 可以用 **_sched\_setscheduler()_** 或 **_sched\_setattr()_** API 设置此策略.


更多可参看此文章: [Deadline scheduling: coming soon? [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/575497/)


## 1.2.3  SCHED\_RT
-------



## 1.2.4 其他一些调度类的尝试
-------


业务场景中总存在一些对时延敏感但是负载很小的在线任务, 和一些时延不敏感但是负载很大的离线任务. 单独使用 isolation 等为时延敏感的业务分配 CPU 是比较浪费 CPU 资源的, 因此这些业务往往混部在一起. 然而, 现有的实现在混部后在线业务的服务质量下降严重.

虽然内核提供了 SCHED_BATCH 和 SCHED_IDLE 两种优先级比较低的调度算法, 但它们仍然和CFS共用相同的实现, 尤其是在负载均衡时是未做区分的, 它们是没有完全的和CFS隔离开来, 所以效果上面介绍的通用方案存在类似的问题.


其实, 在大家看来, 专门为这些应用新增一个调度类也是一个不错的想法, 通过各个调度类的优先级次序, 原生可以保证在线任务直接抢占离线任务, 保证在线任务的唤醒时延等.


- 一种思路是为时延敏感的在线任务, 新增一个优先级比 CFS 高的调度类.

    暂且称作 background 调度类, 这样在选择 idle 的调度类之前, background 可以兜底了. 各个厂商也都做过类型的尝试. 比如腾讯曾经发往邮件列表的 [BT scheduling class](https://lore.kernel.org/patchwork/cover/1092086), 不过这个版本不完善, 存在诸多问题, 如果大家关注的话, 可以查考查阅 TencentOS-kernel 的 商用版本 [离线调度算法bt](https://github.com/Tencent/TencentOS-kernel#离线调度算法bt).


- 另外一种思路是为时延不敏感的离线任务, 新增一个优先级比 CFS 低的调度类.
    
    [sched: Add micro quanta scheduling class](https://lkml.org/lkml/2019/9/6/178) 在 RT 之后, CFS 之前实现了一个类似于 RT 的策略, 为在线任务提供服务, 来解决同样的问题.


# 1.3 组调度支持(Group Scheduling)
-------

## 1.3.1 普通进程的组调度支持(Fair Group Scheduling)
-------

**2.6.24(2008年１月发布)**


2.6.23 引入的 CFS 调度器对所有进程完全公平对待. 但这有个问题, 设想当前机器有２个用户, 有一个用户跑着 9个进程, 还都是 CPU 密集型进程；另一个用户只跑着一个 X 进程, 这是交互性进程. 从 CFS 的角度看, 它将平等对待这 10 个进程, 结果导致的是跑 X 进程的用户受到不公平对待, 他只能得到约 10% 的 CPU 时间, 让他的体验相当差.


基于此, 组调度的概念被引入[<sup>6</sup>](#refer-anchor-6). CFS 处理的不再是一个进程的概念, 而是调度实体(sched entity), 一个调度实体可以只包含一个进程, 也可以包含多个进程. 因此, 上述例子的困境可以这么解决: 分别为每个用户建立一个组, 组里放该用户所有进程, 从而保证用户间的公平性.


该功能是基于控制组(control group, cgroup)的概念, 需要内核开启 CGROUP 的支持才可使用. 关于 CGROUP , 以后可能会写.


## 1.3.2 实时进程的组调度支持(RT Group Scheduling)
-------


**2.6.25(2008年4月发布)**

该功能同普通进程的组调度功能一样, 只不过是针对实时进程的.


## 1.3.3 组调度带宽控制(CFS bandwidth control)** , **3.2(2012年1月发布)**
-------


组调度的支持, 对实现多租户系统的管理是十分方便的, 在一台机器上, 可以方便对多用户进行 CPU 均分．然后, 这还不足够, 组调度只能保证用户间的公平, 但若管理员想控制一个用户使用的最大 CPU 资源, 则需要带宽控制．3.2 针对 CFS组调度, 引入了此功能[<sup>8</sup>](#refer-anchor-8), 该功能可以让管理员控制在一段时间内一个组可以使用 CPU 的最长时间．



## 1.3.4 极大提高体验的自动组调度(Auto Group Scheduling)
-------

**2.6.38(2011年3月发布)**

试想, 你在终端里熟练地敲击命令, 编译一个大型项目的代码, 如 Linux内核, 然后在编译的同时悠闲地看着电影等待, 结果电脑却非常卡, 体验一定很不爽．



2.6.38 引入了一个针对桌面用户体验的改进, 叫做自动组调度．短短400多行代码[<sup>9</sup>](#refer-anchor-9), 就很大地提高了上述情形中桌面使用者体验, 引起不小轰动．



其实原理不复杂, 它是基于之前支持的组调度的一个延伸．Unix 世界里, 有一个**会话(session)** 的概念, 即跟某一项任务相关的所有进程, 可以放在一个会话里, 统一管理．比如你登录一个系统, 在终端里敲入用户名, 密码, 然后执行各种操作, 这所有进程, 就被规划在一个会话里．



因此, 在上述例子里, 编译代码和终端进程在一个会话里, 你的浏览器则在另一个会话里．自动组调度的工作就是, 把这些不同会话自动分成不同的调度组, 从而利用组调度的优势, 使浏览器会话不会过多地受到终端会话的影响, 从而提高体验．



该功能可以手动关闭.


# 1.4 负载跟踪机制
-------

## 1.4.1 PELT
-------

## 1.4.2 WALT
-------


# 1.5 select_task_rq
-------

调度器最关键的任务就是两个:

- **选进程**: 选择下一个更合适的进程 pick_next_task
- **选核**: 为某个进程选择更合适的 CPU 运行 select_task_rq

所有其他的机制都是直接或者间接服务这两个终极任务的.


Linux 下各个调度类都实现了这两个接口, 各个调度类按照既定的算法和规则运行:
在选进程这方面, 各个调度类的算法很成熟, 而且经理的改动很小;
调度的目标是提升系统整体的性能或者降低功耗, 从这些目标上看, 在**选进程**算法既定的情况下, 第二个任务**选核**就会显的更重要也更复杂一些, 选核选的好, 不光能及早的投入运行, 而且在运行时能充分利用 cache 等硬件优势, 让进程飞一般的运行.

内核中主体的进程都是以 SCHED_NORMAL 为策略的普通 CFS 进程, 以 select_task_rq_fair 为例, 其代码就经过了不断的重构和优化.


## 1.5.1 机制的 WAKE_AFFINE
-------


## 1.5.2 限制遍历的 CPU 数目
-------



# 1.6 基于调度域的负载均衡
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



当进行负载均衡时, 将从最低一级域往上看, 如果能在 core 这个层级进行均衡, 那最好；否则往上一级, 能在socket 一级进行均衡也还凑合；最后是在 NUMA node 之间进行均衡, 这是代价非常大的, 因为跨 node 的内存访问速度会降低, 也许会得不偿失, 很少在这一层进行均衡．



这种分层的做法不仅保证了均衡与性能的平衡, 还提高了负载均衡的效率．



关于这方面, 可以看这篇文章: [Scheduling domains [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/80911/)



# 1.7 更精确的调度时钟(HRTICK), 2.6.25(2008年4月发布)**
-------


CPU的周期性调度, 和基于时间片的调度, 是要基于时钟中断来触发的．一个典型的 1000 HZ 机器, 每秒钟产生 1000 次时间中断, 每次中断到来后, 调度器会看看是否需要调度．



然而, 对于调度时间粒度为微秒(10^-6)级别的精度来说, 这每秒 1000 次的粒度就显得太粗糙了．



2.6.25 引入了所谓的**高清嘀哒(High Resolution Tick)**, 以提供更精确的调度时钟中断．这个功能是基于**高精度时钟(High Resolution Timer)框架**, 这个框架让内核支持可以提供纳秒级别的精度的硬件时钟(将会在时钟子系统里讲).



# 1.8 自动 NUMA 均衡(Automatic NUMA balancing)
-------

**3.8(2013年2月发布)**

NUMA 机器一个重要特性就是不同 node 之间的内存访问速度有差异, 访问本地 node 很快, 访问别的 node 则很慢．所以进程分配内存时, 总是优先分配所在 node 上的内存．然而, 前面说过, 调度器的负载均衡是可能把一个进程从一个 node 迁移到另一个 node 上的, 这样就造成了跨 node 的内存访问；Linux 支持 CPU 热插拔, 当一个 CPU 下线时, 它上面的进程会被迁移到别的 CPU 上, 也可能出现这种情况．



调度者和内存领域的开发者一直致力于解决这个问题．由于两大系统都非常复杂, 找一个通用的可靠的解决方案不容易, 开发者中提出两套解决方案, 各有优劣, 一直未能达成一致意见．3.8内核中, 内存领域的知名黑客 Mel Gorman 基于此情况, 引入一个叫自动 NUMA 均衡的框架, 以期存在的两套解决方案可以在此框架上进行整合; 同时, 他在此框架上实现了简单的策略: 每当发现有跨 node 访问内存的情况时, 就马上把该内存页面迁移到当前 node 上．



不过到 4.2 , 似乎也没发现之前的两套方案有任意一个迁移到这个框架上, 倒是, 在前述的简单策略上进行更多改进．



如果需要研究此功能的话, 可参考以下几篇文章:

－介绍 3.8 前两套竞争方案的文章: [A potential NUMA scheduling solution [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/522093/)

- 介绍 3.8 自动 NUMA 均衡 框架的文章: [NUMA in a hurry [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/524977/)

- 介绍 3.8 后进展的两篇文章, 细节较多, 建议对调度／内存代码有研究后才研读:

[NUMA scheduling progress [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/568870/)

[NUMA placement problems [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/591995/)



# 1.9 **CPU 调度与节能**
-------


从节能角度讲, 如果能维持更多的 CPU 处于深睡眠状态, 仅保持必要数目的 CPU 执行任务, 就能更好地节约电量, 这对笔记本电脑来说, 尤其重要．然而这不是一个简单的工作, 这涉及到负载均衡, 调度器, 节能模块的并互, Linux 调度器中曾经有相关的代码, 但后来发现问题, 在 3.5, 3.6 版本中, 已经把相关代码删除．整个问题需要重新思考．



在前不久, 一个新的 patch 被提交到 Linux 内核开发邮件列表, 这个问题也许有了新的眉目, 到时再来更新此小节．可阅读此文章: [Steps toward power-aware scheduling [LWN.net]](https://lwn.net/Articles/655479/)



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
