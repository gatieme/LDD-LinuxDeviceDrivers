 ---

title: 工具
date: 2021-06-26 09:40
author: gatieme
tags:
    - linux
    - tools
categories:
        - 技术积累
thumbnail:
blogexcerpt: 虚拟化 & KVM 子系统

---

<br>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 鄙人在此谢谢啦

**转载请务必注明出处, 谢谢, 不胜感激**

<br>

| 日期 | 作者 | GitHub| CSDN | BLOG |
| ------- |:-------:|:-------:|:-------:|:-------:|
| 2021-02-15 | [成坚-gatieme](https://kernel.blog.csdn.net) | [`AderXCoding/system/tools/fzf`](https://github.com/gatieme/AderXCoding/tree/master/system/tools/fzf) | [使用模糊搜索神器 FZF 来提升办公体验](https://blog.csdn.net/gatieme/article/details/113828826) | [Using FZF to Improve Productivit](https://oskernellab.com/2021/02/15/2021/0215-0001-Using_FZF_to_Improve_Productivity)|


<br>


2012 年 Intel 发布了一篇关于大小核的论文 [Scheduling Heterogeneous Multi-Cores through Performance Impact Estimation (PIE)](http://www.jaleels.org/ajaleel/publications/isca2012-PIE.pdf). 论文中小核使用了 In-Order 的微架构, 大核则使用了 Out-Order 的微架构, 传统的想法都是计算密集型(compute-intensive)的进程跑到大核上, 访存密集型(memory-intensive)的进程跑到小核上去. 但是论文中提出了不一样的想法. 因为内存访问的密集程序并不是一个能反应进程的负载对 CPU 需求的好指标.

1.  如果应用的指令流拥有较高水平的 ILP, 那么它在 In-Order 的小核上, 也能获得很好的性能.

2.  如果应用有较大的 MLP(访存的并发) 以及动态的 ILP, 那么它在 Out-Order 的大核上, 才能获得很好的性能.

因此在大小核上应该分析和利用进程的 ILP/MLP 等特征, 这样才能获得更好的性能.

该论文是 [ACM Computing Surveys](https://dl.acm.org/journal/csur) 第 48 卷, [第 3 期](https://dl.acm.org/doi/10.1145/2856125)(2016 年 2 月) 中 [A Survey of Techniques for Architecting and Managing Asymmetric Multicore Processors](https://dl.acm.org/doi/pdf/10.1145/2856125) 部分第 5.3 节 Use of the CPI Stack 的第二篇.

这应该是 Intel 关于大小核混合架构(single-ISA heterogeneous multi-cores)最早的研究, 此后 Intel 发表了陆续发表了关于大小核混合架构的多项研究.

# 1 Introduction
-------

> Van Craeynest et al. [2012] present a model called performance impact estimation (PIE) to predict the best application-to-core mapping. They note that the ratios of MLP and ILP on big and small cores provide a good indication of the performance difference between those cores. Based on this, while an application is running on one core type, their technique collects ILP and MLP profiling information and the CPI stack, and uses this to estimate the ILP and MLP and hence application performance on the other core type. Overall, PIE tracks how a core exploits ILP and MLP, and using the CPI stack, it finds its overall performance impact. They make some simplifying assumptions such as presence of identical cache hierarchy on both core types. In addition, some HW counters (e.g., for recording interinstruction dependency distance distribution) required by this model may not be available in existing processors. They show that by making application-to-core scheduling decisions based on the PIE model, significant performance improvement can be achieved.

该论文中 Van Craeynest 等人提出了性能影响的评估模型(PIE/Performance Impact Estimation), 论文中的关注 AMP 处理器, 由顺序(In-Order)的小核和乱序(Out-of-Order)的大核构成, 论文中发现大核和小核的 MLP 和 ILP 的比值可以很好的说明核间的性能差异. 通过 PIE 模型, 当应用在某一个 core 上执行时, 收集 CPI stacks 以及 ILP 和 MLP 信息, 估计另一个类型的 core 上的应用程序的性能, 然后选择对整体性能最优的核来运行.

论文第 2 章用实验证明了传统的计算密度(compute-intensive)和访存密度(memory intensity)并不足以反应进程特征. 相反, 在当前由顺序(In-Order)的小核和乱序(Out-of-Order)的大核 AMP 系统中, ILP 和 MLP 的特征与进程实际运行的表现拟合, 可以恰到好处的反映进程的特征.

论文第 3 章描述了 PIE 的原理, PIE 的核心思想是: 当应用在一个类型的核上运行时, 预测其在另外一个类型的核上的 ILP 以及 MLP, 从而推测出其在另外一个类型的核上运行的性能. 正是大小核微架构的差异导致了同一个应用在不同类型的核上运行时 ILP 和 MLP 表现出明显的差异. 因此论文正是分析硬件实现上的差异, 实现了 PIE 预测的模型. 并通过试验(手动推测后, 将进程静态绑核)进行了验证. 由于大小核的差异比较明显(小核是 In-Order 的, 大核是 Out-of-Order 的), 因此在小核上预测大核的 ILP 和 MLP 就相对容易一些, 大核的 ILP 与其 issue-width $W_{big}$ 相关, 大核的 MLP 则与其重排序缓冲区 ROB 的大小相关. 但是由于大核的乱序执行, 掩盖了原有指令之间的依赖关系, 因此在大核上预测小核的 ILP 和 MLP 就显得稍微有点复杂. ILP 的预测使用了一个简化的概率论模型, 指令能否并行的发射与执行与这几条指令是否存在依赖强相关. 同样 MLP 的预测也与 load/store 指令之间的依赖距离强相关.

第 3 章的实验只是通过静态绑核的方式验证了 PIE 模型的准确度和效果, 如果需要在实际场景中使用, 必须要设计一套 Dynamic Scheduling 算法. 第 4 章就阐述了这个 Dynamic Scheduling 的软硬件实现, 为了进行 Dynamic Scheduling, 需要在金衡运行时, 收集进程的 ILP 和 MLP 以及 CPI stack　等信息, 然后预测进程在另外一个类型的核上的 ILP 和 MLP 等信息, 进而预测其性能, 如果发现当前核不是最优, 则将进程迁移到目标核上. 为了实现 Dynamic Scheduling, 则需要 ① 迁移开销要小, 否则迁移后引入的开销将抵消其在目标核上运行带来的收益; ② 硬件支持收集ILP 和 MLP 以及 CPI stack　等 PIE 预测所需的信息. 首先为了分析和验证迁移的开销和影响, 需要实现一套最优的 cache 系统, 因此论文中通过实验测试了不同 cache 实现方案下的迁移开销. 其次 ILP 以及 CPI stack 等信息的收集现有硬件已经支持, 而要收集 MLP 需要统计指令之间的依赖距离, 这需要单独的硬件实现, 论文中借助了一个与逻辑寄存器行数相同的表来监测依赖距离.

第 5 章描述了验证 PIE Dynamic Scheduling 所需的模拟仿真环境以及测试模型.

第 6 章则对 PIE Dynamic Scheduling 进行了测试验证. 实验表明, PIE 的调度的性能比仿存强度主导的调度(memdom)以及基于采样的调度(sampling)都要好. 这里还验证了多种 cache 系统下的性能. 发现不同的 Cache 组织(Private VS Share), 以及不同的 Cache 替换算法(LRU VS RRIP)都对整体性能和效果有一定影响. 在 Private LLCs 的实现下, PIE 远优于其他方案(> memdom 5.5%, > sampling 8.7%), 在 LRU-managed share LLC 的实现下, PIE 带来的性能依然最优, 但是由于 share cache 本身带来的收益, 因此 PIE 相较于其他调度算法的优势略有下降(> memdom 3.7% 和 > sampling 6.4%). 而如果替换算法修改为智能共享缓存管理策略 RRIP, 在 RRIP-managed shared LLC 的实现下, PIE 重新获得了远超其他调度算法的性能优势(> memdom 2.4% 和 > sampling 7.8%).

第 7 章对现有的 AMP 异构多核调度技术做了分析和对比.

第 8 章对 PIE 做了整体的总结和展望.


# 2 Motivation
-------

## 2.1 计算密度和访存密度不能很好的体现进程特征
-------

单独的内存强度(或计算强度)并不是指导异构多核上的应用程序调度的良好指标. 为了证实这个结论, 论文中对 SPEC CPU 2006 的应用进行了测试和分析.

为了标记进程计算密度和访存密度, 将应用的 CPI 按照所请求的资源强度划分为: CPI_base, CPI_L2, CPI_L3, CPI_mem

| 标记 | 描述 |
|:---:|:----:|
| CPI_mem  | 表示当前当前指令执行的周期内, 对内存的请求非常密集 |
| CPI_L3   | 当前周期内对 L3 Cache 的请求较多, 造成了较大的压力 |
| CPI_L3   | 当前周期内对 L2 Cache 的请求较多, 造成了较大的压力 |
| CPI_BASE | 对 cache 和 mem 的其他执行单元有较多的访问和请求, 可以立即为强计算密度的 |

然后将 SPEC CPU 的测试在大核上和小核上分别运行, 测得其在 small-core slowdown.

*   slowdown 越小, 说明应用对大小核不敏感, 从大核迁移到小核上, 性能下降不严重

*   slowdown 越大, 说明应用对大小核敏感, 从大核迁移到小核上, 性能下降比较严重.


从图中可以看到:

1.  虽然多数情况下, 内存密集型的应用在小核上性能都没有明显下降, 但是依旧存在一些内存密集型的应用(例如 milc.su3, soplex, soplex.pds 等)在小核上 也极低.

2.  类似地, 一些计算密集型工作负载(例如 star.r) 在小核上观察到显著的性能下降(slowdown 接近 0.9), 而其他计算密集型工作负载(例如
gcc.s, calculix 等) 在小核上执行时性能下降地就不那么明显(slowdown 只有不到 0.4 左右).

这种行为说明, 单独的内存强度(或计算强度)并不是指导异构多核上的应用程序调度的良好指标.

## 2.2 ILP/MLP 才是当前混合架构下好的特征
-------

大核特别适合于需要动态提取 ILP 或具有大量 MLP 的工作负载, 小核适用于具有大量固有 ILP 的工作负载. 这意味着不同核心类型上的性能可以直接与工作负载中普遍存在的 MLP 和 ILP 的数量相关. 一个大量 MLP 的内存密集型工作负载, 在小核上执行可能会导致显著的速度下降. 而具有大量 ILP 的计算密集型工作负载在小内核上性能下降也不会太大, 不需要大核就能运行的很好.

为了分析其中更本质的细节, 论文中又做了一组分析. 将测试的应用按照计算密度从大到小, 访存密度从小到大排序, 然后将测试的 slowdown, ILP ratio, MLP ratio 按照这个顺序展示.

$MLP_{ratio} = MLP_{big} / MLP_{small}$

$ILP_{ratio} = CPI_{base\_big}/CPI_{base\_small}$

> MLP 定义为未完成的内存请求的平均数量，如果至少有一个未完成的 [4].
>
> CPI~base~ 作为 CPI stack 的基础(非阻塞)组件.

如论文中图 2 所示. 可以看到, 对于计算密集型的应用(图左侧)其 slowdown 与 ILP ratio 的曲线相拟合. 而对于访存密集型的应用(图右侧)其 slowdown 与 MLP ratio 的曲线趋于拟合.

# 3 Performance Impact Estimation (PIE)
-------

因此仅密集的内存访问并不是反映进程负载特征的好指标. 异构多核上的调度策略必须要考虑计算不同微架构的核可以利用的 MLP 和 ILP 的数量. 此外, 当在不同类型的核之间移动时, 减速(或加速)可以直接与目标核心上的 MLP 和 ILP 实际的数量相关. 这表明, 可以通过预测当前核上的 MLP 和 ILP 来估计其他类型核上的性能.

$CPI \approx CPI_{base} + CPI_{mem}$

> 这里忽略了 L1 cache, L2 cache 等对 CPI 的影响, 从图 2 中的信息可以看出, 这部分 CPI 信息在当前 AMP 平台上对进程的特征提取影响比较小.

如果进程在大核上运行, 那么通过它在大核上的性能指标, 预测它在小核上运行的性能的方法如下:

$\widetilde{CPI}_{small} = \widetilde{CPI}_{base\_small} + \widetilde{CPI}_{mem\_small} = \widetilde{CPI}_{base\_small} + \widetilde{CPI}_{mem\_big} \times MLP_{ratio}$

反之, 如果进程在小核上运行, 那么根据它在小核上的性能指标, 预测它在大核上运行的性能的方法类似:

$\widetilde{CPI}_{big} = \widetilde{CPI}_{base\_big} + \widetilde{CPI}_{mem\_big} = \widetilde{CPI}_{base\_big} + \widetilde{CPI}_{mem\_small} / MLP_{ratio}$


> 在上面的公式中
>
> CPI~base\_big~ 是指从在小核的执行的数据中中估算出的对大核的基数 CPI~base~ 分量; CPI~base_small~ 类似.
>
> 大(小)核上的 CPI~mem~ 的计算方法是将小(大)核上的 CPI~mem~ 与 MLP~ratio~ 相除(乘)得到的


然后剩下的关键就是如何预测基 CPI~base~ 和 MLP~ratio~.

## 3.1 Predicting MLP
-------


PIE 的核心思想是: 根据进程在当前 CPU 核上的性能以及 CPI stack 等信息, 预测该进程在其他不同类型核心上的的性能.

论文中关注 CPI stack 中的两个主要组件: 基本组件 CPI~base~ 和内存组件 CPI~mem~, 前者将所有与内存无关的组件集中在一起.

$CPI = CPI_{base} + CPI_{mem}$

### 3.1.1 Predicting big-core MLP on small core
-------

大核是乱序(Out-of-Order)的, 因此实现了重排序缓存(ROB), 非阻塞 cache 以及 MSHR 等. 从而可以并行的执行指令, 特别是并行执行那些没有数据依赖的访存(load/store)指令. 一个大核可以利用的最大 MLP 是受重排序缓存(ROB)的大小限制, 因为只有当 ROB 满了, 无法容纳新的指令, 才会导致一条与 ROB 中已有指令没有数据依赖的指令阻塞.


$MLP_{big} = MPI_{small} \times ROB_{size}$

其中 $MPI_{small}$ 是小核上的 MPI 值, 它近似等于为小核上观察到的 LLC misses per instruction. 由于预取等操作提前进入流水线的指令, 也会造成 LLC Misses, 但是论文注意到了这一点, 但是为了使得设计更简化, 因此对此不做区分.

$\widetilde{CPI}_{small} = \widetilde{CPI}_{base \_small} + \widetilde{CPI}_{mem\_small} = \widetilde{CPI}_{base\_small} + \widetilde{CPI}_{mem_big} \times MLP_{ratio}$

### 3.1.2 Predicting small-core MLP on big core
-------


小核是 In-Order 的, 那么在大核上预测小核就相对复杂一些. `Out-of-Order` 的实现有两种方式.

1.  stall-on-miss 的实现, CPU 会等着当前 miss 处理完之后, 才会执行其他访存操作. 这种情况下, 可以认为跟 In-Order 类似, 是不存在 MLP 的.

2.  stall-on-use 的设计, 只要两个访存操作之间没有相关依赖, 那么就可以并行执行, 那么这种情况下, 就存在一定的 MLP. 举例来说, 一个 load 操作是给后续一个 store 操作用的, 那么这个 store 可以理解为 load 操作的消费者. 那么一个 stall-on-use 的 CPU 其 MLP 等于 long-latency 的 load 与其消费者之间的平均内存访问数.



$MLP_{small} = {{MPI_{big}} \times {D}}$


其中 $MPI_{big} 为大核上 LLC miss per instruction 的平均数量, D 为 LLC miss 与其消费者之间的平均依赖距离, 它的值为生产者和消费者之间动态执行指令的数量.



## 3.2  Predicting ILP
-------


### 3.2.1 Predicting big-core ILP on small core
-------


大核的基础 CPI 部分 $\widetilde{CPI}_{base\_big}$ 与其发射宽度(issue width)有关.


$\widetilde{CPI}_{base\_big} = 1 / W_{big}$

一个平衡的乱序的大核应该能够在没有 miss 的情况下, 每个周期中分发大约 $W_{big}$ 条 指令. 平衡的 CPU 核设计可以通过使重排序缓存(ROB)和相关结构, 如  issue queues, rename register file 等, 使得流水线能够以接近 issue width 的宽度发射指令.


### 3.2.2 Predicting small-core ILP on big core
-------


在一个乱序的大核上运行时, 估计一个顺序小核的基础 CPI 部分就要复杂得多. 为了便于推理，我们估计了平均 IPC，并取估计 IPC 的倒数得出估计 CPI。

一个 issue width 为 $W_{small}$ 的小核上的平均基础 IPC 如下:

$\widetilde{CPI}_{base\_small} = \sum^{W_{small}}_{i=1}{i} \times P_{IPC = i}$


其中 W_{small} 为小核的 issue-width, $P_{IPC = i}$ 为在一个给定的周期内执行 i 条指令的概率.

这里使用了一个概率论的方法来计算.

| i | 公式 | 描述 |
|:-:|:---:|:---:|
| 1 | $P_{IPC = 1} = P_{D = 1}$ | 在给定的循环中, 只执行一条指令的概率等于一条指令产生的值被动态指令流中的下一条指令消耗的概率(即依赖距离为 1) |
| 2 | $P_{IPC = 2} = (1 - P_{D = 1}) \times (P_{D = 1} + P_{D = 2})$ | 在一个给定的循环中, 执行只能执行两条指令的概率等于第二个指令不依赖于第一个指令的情况下, 第三个指令依赖于第一个或第二个指令的概率 |
| 3 | $P_{IP C = 3} = (1 - P_{D = 1}) \times (1 - P_{D = 1} - P_{D = 2}) \times (P_{D = 1} + P_{D = 2} + P_{D = 3})$ |依次类推|
| ... | ... | ... |

> 这个概率论模型没有考虑不同指令或者 uOps 的执行周期可能不同, 因此对实际的 CPU 指令做了简化, 但是已经足以达到预测的目的.


## 3.3 Evaluating the PIE Model
-------

论文中这个部分对 PIE 模型的预测效果做了验证和分析. 不过验证的方式很简单, 通过 PIE 预测为应用选择最高系统吞吐量的核, 然后将应用绑定在该核上(static schedule；
), 然后将测试结果同随机执行(random)和内存支配(memdom)的方式以及最优的方式(optimal)来对比. 同时由于 PIE 同时使用了 PIE 中 ILP 预测和 MLP 预测, 为了辨别两者对测试效果的影响程度, 增加了一个测试项 MLP-ratio, 这种情况下, 没有进行 ILP 预测, 仅使用了 MLP 预测, 且


先基于一个大核和一个小核的系统验证.

1.  首先分别在验证了 PIE 预测的准确度. 在小核上执行工作负载后预测其在大核上的性能,以及在大核上执行后预测其在小核上性能, 将这两组预测的数据与实际的 slowdown 进行比较, 实验发现虽然存在一些误差, 但是 PIE 可以准确预测大核和小核之间的相对性能差异. 参见论文中图 4.


2.  接着验证了 PIE 调度带来的性能收益, 同时与仅使用 MLP-ratio 预测的简化模型对比后发现, 对于第 i 类和第 III 类工作负载, MLP-ratio 调度将最坏情况下的调度提高了 12.7%, 而 PIE 为 14.2%, 两者相差不多, 且都已经接近最优的(静态地绑在大核上)的效果 16%+, 这说明在 PIE 中, 对 MLP 的预测比对 ILP 的预测更为重要. 参见图 5 和图 6.

接着基于一个大核和多个小核以及多个大核和一个小核(例如，NVidia 的 Kal-El)的异构多核系统验证 PIE 调度, 参见图 7.


# 4 Dynamic Scheduling
-------

第 3 章重点在于评估 PIE 模型的准确性, 因此使用的测试是 static scheduling, 进程绑定在固定的 CPU 核上运行. 但是这并不是一个完整的解决方案, 因此需要研究动态的调度模型.


## 4.1 Quantifying migration overhead
-------

动态调度会导致在不同核之间迁移工作造成额外的开销, 迁移不仅会导致上下文切换, 还会造成 cache misses 等影响应用的性能.

其中上下文切换的成本接近是固定. 为了更好地分析 cache warming 带来的开销, 论文中针对如下场景进行了测试:

每 x 毫秒迁移一个任务到不同类型的核上, x 从 1ms 到 50ms 不等.

通过对迁移后任务的性能进行对比评估, 目的是找到一套低迁移成本( low migration overheads)的缓存层次结构(cache hierarchy designs), 使其能更好的使用在细粒度动态调度方案(fine-grained dynamic scheduling)中.



| 测试场景 | 环境配置 | 测试结果 |
|:-------:|:------:|:-------:|
| Shared LLC | 两个相同的 CPU 核共享一个 4MB LLC | 如图 9 所示, 对于 2.5ms 的迁移频率, 迁移带来的性能开销很小, 对于所有基准测试来说都不到 0.6%. 这时候的性能损失是由于私有的 L1 和 L2 cache warming 造成的, 因此开销极小. |
| Private powered-off LLCs | 每个核有私有的 LLC, 并在进程迁出后, 下电 LLC 来省电 | 如图 10 所示, 如果想要关闭整个核心和它的私有缓存层级来省电, 那么设计私有 LLC 是有意义的. 但是如果迁移频率很高(例如 2.5ms), 性能抖动会非常大. 巨大的性能损耗是因为 cache 在关闭电源时丢失了数据, 因此迁移到新的 core 之后必须从主内存中重新获取数据. |
| Private powered-on LLCs | 不关闭私有 LLC, 这样即使进程迁出后, 私有 LLC 依旧保持上电, 且保留缓存中的数据. |  图 11 显示, 频繁迁移带来的性能开销要小得多, 实际上甚至对相当一部分基准测试带来了实质性的性能好处. 性能好处来自拥有大且高效的 LLC: 当迁移后, 新核的私有 LLC miss 时, 数据很可能会在旧核的私有 LLC 中找到, 因此通过硬件的缓存一致性可以快速地从旧核的 LLC 中获取数据, 这比从主存获取数据快多了. |


## 4.2 Dynamic PIE Scheduling
-------

为了实现最优调度方案, PIE 调度需要硬件支持, 以收集每个核上的 CPI stacks、 LLC cache miss 数、 instruction 数以及大核上的指令间依赖距离分布(inter-instruction dependency distance distribution).


在 x 毫秒的时间间隔内, 对于系统中的每个工作负载, PIE 通过硬件获取到当前核心类型上的 CPI stacks、MLP 和 ILP, 并预测其他核心类型上相同工作负载的 MLP 和 ILP. 然后将这些预测输入 PIE 模型, 以估计每个工作负载在其他核心类型上的性能. 对于给定的性能指标, PIE 调度使用这些预测值来确定与当前调度决策相比, 另一个调度决策(比如迁移至另外一类核上)是否可能提高整体系统性能. 如果是这样, 任务将被重新调度到预测的核心类型上. 如果没有, 则工作负载时间表保持当前决策, 并在下一个时间间隔中重复该过程.




## 4.3 Hardware support
-------

首先是收集 CPI stack.

*   在 in-order 的核上收集 CPI stack 非常简单, 且已经有成熟的商业实践, 参见 Intel ATOM 核[11].

*   在 out-of-order 的核上收集 CPI stack 就显得非常麻烦了, 由于 miss 事件之间的会有各种重叠效应. 例如, 一个较长的延迟负载可能隐藏另一个独立的长延迟负载错过或预测错误的分支的延迟, 等等. 最近的商业处理器如 IBM Power5 [23] 和 Intel Sandy Bridge [12] 提供了对计算内存休眠(computing memory stall components)的支持.


其次 PIE 调度还需要统计 LLC cache miss 数、 instruction 数, 这些都可以使用现有的 PMU 来统计.

以上信息都是现有硬件中可以获取到的. 但是 PIE 也需要一些无法在现有硬件上收集的信息.


1.  当在一个大核上运行时, PIE 要求能够评估确定指令间依赖距离分布, 以便估计小核的 MLP 和 ILP. PIE 模型要求依赖距离分布仅为 $W_{small}$ 的最大依赖距离 (其中 $W_{small}$ 是小核的 issue-width). 对于 4 发射的内核, 这涉及到 4 + 1 计数器, 4 个计数器用于计算 4 条指令的依赖距离分布, 1 个计数器用于计算平均距离, 这些计数器使用 10bit 就足够了.

2.  PIE 模型要求在动态指令流上计算平均依赖距离 D. 可以通过定义一张与架构定义的逻辑寄存器数量相同的行数的表来实现. 这个表记录了最后写入该逻辑寄存器的指令, 那么使用一个计数器统计针对该寄存器写和随后的读之间的增量就是依赖距离. 表计数器不需要很宽, 因为依赖关系距离通常是很短的, 只需要 4bit 的计数器就可以正确捕捉 90% 的距离.

因此, 跟踪依赖距离分布的总硬件成本大约是 15byte(4bit × 架构逻辑寄存器的数量(x86_64 为 16 个) 的计数器 + 5 个 10bit 计数器)的存储.


# 5 Experimental Setup
-------


本章讲述了最终测试的仿真环境, 论文使用了 CMP$im [13] 搭建的模拟实验:


1.  大核是一个 4 发射的乱序处理器, 小核是一个 4 发射的 stall-on-use 的处理器核. 频率均为 2GHz.


2.  拥有三级缓存, 32 KB L1 I-cache 和 D-cache、256 KB L2, 以及 4MB L3(LLC).  其中 L1 和 L2 缓存是每个核心私有的. 同时评估了共享和私有的 LLC 配置, 所有高速缓存中首选 LRU 替换策略, 然后还考虑了 ART ARRIP 共享缓存替换策略的[15]. 后续都会对这些不同的 cache 设计和实现进行测试和验证.


3.  最后, 还假设有一个非常积极的基于流的硬件预取器, 防止流水线前端预取等操作对测试的影响, 整个仿真环境都关注 In-Order 和 Out-of-Order 的差异和影响.


# 6 Results and Analysis


## 6.1 Private LLCs
-------



每个核心都有自己的 Private LLCs 的情况, 图 12 量化了基于采样的随机调度、内存支配(memory-dominance) 和 PIE 调度的相对性能. PIE 调度明显优于其他调度策略. 对于 I 型和 III 型进程组成的混合应用, PIE 分别比内存优势和基于采样的调度平均提高 5.5% 和 8.7%. 相对于对内存支配调度的改进来自两个方面:

1.  PIE 能够更准确地确定更好的工作负载到核心的映射. 与内存支配调度不同, PIE 可以利用细粒度阶段行为.

2.  PIE 还改进了基于采样的调度, 借助了硬件实现, 不会因采样而产生任何开销, 并且它准确地预测了工作负载重新调度后的性能影响, 可以适应细粒度阶段行为.


## 6.2 Shared LLC
-------

### 6.2.1 LRU-managed share LLC

对于 Shared LLC 的情况, 对比数据如图 13 所示. PIE 依然有性能优化. 对于 I 型和 III 型进程组成的混合应用, 分别比内存支配和基于采样的调度获得了平均 3.7% 和 6.4% 的性能提升.

1.  没有 Private LLCs 性能提升的明显, 原因是所有调度策略都没有预见到共享 LLC 中的冲突行为, 因此, 部分调度决策可能会被共享 LLC 中的冲突行为抵消.

2.  对于基于采样的调度, 任务在不同类型的 CPU 核上运行, cache 的访问模式会发生变化, 那么 LLC 对进程的影响不同, 这反过来会导致整体性能的差异, 采样的数据也有差异. 因此, 在 Shared LLC 的情况下, 采样的数据可能失效, 从而基于采样的方式效果不那么好.


### 6.2.2 RRIP-managed shared LLC(论文中 6.3)
-------


到目前为止, 我们使用的都是 LRU 缓存替换策略. 然而, 研究表明, LRU 并不是最有效的共享缓存管理策略; RRIP [15] 是一种最先进的缓存替换策略, 它通过预测缓存块的重新引用行为显著提高了 LLC 的性能. 在 使用了 RRIP 管理的 Share LLC 上, 测试结果对比如图 14 所示. 对于 I 型和 III 型的混合应用, PIE 调度分别比内存支配和基于采样的调度提高性能 2.4% 和 7.8%.

> 从图 14 中可以观察到一个有趣的现象: 以 RRIP 为代表的智能共享缓存管理策略能够降低由于调度而观察到的某些工作负载的性能命中. 对使用 LRU 管理的 Share LLC, 采样调度下大部分应用(以及内存支配调度下的少数应用)观察到显著的性能下降, 见图 13 左下角. 这些性能影响是通过 RRIP 消除的, 参见图 14.
>
> 即, 调度策略可以从智能缓存替换策略中受益, 调度策略的错误决策可以通过缓存管理策略(在一定程度上)得到缓解.




# 7 Related Work
-------

第 7 章讨论了一些当时(2012 年)一些前沿的异构多核调度上的技术研究, 并将这些研究与 PIE 做了简单对比.

# 8 Conclusions
-------

第 8 章对论文进行了总结.


1.  论文中用实验证明了, 仅使用内存强度作为指导工作负载调度决策的指标可能导致次优性能. 相反, 调度策略必须考虑核心类型如何利用工作负载的 ILP 和 MLP 特征.


2.  提出了 PIE 模型来指导任务调度, PIE 模型使用给定核心类型上工作负载的 CPI stack、ILP 和 MLP 信息来估计不同核心类型上的性能. 提出了用于小核(In-Order)和大核(Out-of-Order)的 PIE 模型.


3.  利用 PIE 模型, 设计了动态 PIE 调度, 在运行时收集 CPI stack、ILP 和 MLP 信息, 以指导调度器进行决策.


4.  用实现证明了, 使用 Share LLCs 可以实现高频、低开销、细粒度调度, 可以应对任务的实时负载变化. 还说明了, 只要在核心迁移中不刷新缓存, 使用 Share LLC 就可以提供类似的功能.

5.  评估了各种具有不同核心计数和缓存配置的系统的 PIE, PIE 调度可扩展到任何核心数量的系统中, 并显著优于之前的研究工作.

论文将重点放在使用 PIE 调度来提升异构多核系统的加权加速度量(weighted speedup metric). 这些指标主要是针对多进程应用的. 但是, PIE 调度也可以应用于多线程应用. 此外, 当多个工作负载竞争相同的核心类型时, 可以扩展 PIE 调度以优化公平性.




<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
