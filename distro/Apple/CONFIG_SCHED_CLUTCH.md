2   **CONFIG_SCHED_CLUTCH**
=====================

---

title: 苹果内核 XNU
date: 2021-05-05 18:40
author: gatieme
tags: hexo
categories:
        - hexo
thumbnail:
blogexcerpt: 树莓派使用资料

---

| CSDN | GitHub | Hexo |
|:----:|:------:|:----:|
| [Aderstep-- 紫夜阑珊 - 青伶巷草](http://blog.csdn.net/gatieme) | [`AderXCoding/system/tools`](https://github.com/gatieme/AderXCoding/tree/master/system/tools) | [gatieme.github.io](https://gatieme.github.io) |

<br>

<a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a>

本作品采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"> 知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议 </a> 进行许可, 转载请注明出处, 谢谢合作

<br>

# 2.3 CONFIG_SCHED_CLUTCH
-------

## 2.3.1 Clutch 概述
-------

XNU 内核运行在各种平台上, 对动态和高效有很强的要求. 它需要满足广泛的要求; 从快速访问 CPU 以处理延迟敏感型工作负载 (例如. UI 交互, 多媒体录制 / 播放) 以避免低优先级批处理工作负载 (例如照片同步, 源编译) 的饥饿. 传统的 Mach 调度程序试图通过期望系统中的所有线程都标记为优先级编号并将高优先级线程视为交互式线程和将低优先级线程视为批处理线程来实现这些目标. 然后, 它使用基于优先级衰减的分时模型来惩罚线程, 因为它们使用 CPU 来实现公平份额和饥饿避免. 但是, 这种方法失去了线程和更高级别用户工作负载之间的关系, 使得调度程序无法对最终用户关心的整个工作负载进行推理. 这种基于线程的分时方法的一个工件是, 无论它们正在处理哪个用户工作负载, 同一优先级的线程都会得到类似的处理, 这通常会导致非最佳决策. 它最终导致整个平台的优先级膨胀, 各个子系统提高其优先级以避免与其他不相关线程的饥饿和分时. 传统的线程级调度模型还存在以下问题:


| 问题 | 描述 |
|:---:|:---:|
| 不准确的记帐 | 线程级别的 CPU 记帐激励在系统上创建更多线程. 同样在快速创建和销毁线程的 GCD 和工作队列世界中, 线程级别记帐不准确, 并且允许过多的 CPU 使用率. |
| 隔离差      | 在 Mach 调度程序中, 分时是通过根据全局系统负载衰减线程的优先级来实现的. 此属性可能会导致相同或较低优先级频段的活动突发, 从而导致应用 / UI 线程衰减, 从而导致性能和响应能力不佳. 计划程序在处理延迟敏感型 UI 工作负载的线程和执行批量非延迟敏感操作的线程之间提供非常有限的隔离. |


xnu-6153.11.26
> The clutch scheduler is enabled only on non-AMP platforms for now

[Mach Overview](https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/KernelProgramming/Mach/Mach.html#//apple_ref/doc/uid/TP30000905-CH209-TPXREF101)



为了推理更高级别的用户工作负载, Clutch 调度程序调度线程组而不是单个线程. 它打破了传统的单层调度模型, 实现了分层调度程序, 可在各种线程分组级别做出最佳决策. 分层调度程序, 正如它今天实现的那样, 有 3 个级别:

| No | Level | 描述 |
|:--:|:-----:|:---:|
| 1 | 调度桶级别 (Scheduling Bucket Level) | 最高级别是调度存储桶级别, 它决定应选取哪类线程执行. 内核维护每个线程调度存储桶的概念, 该概念是根据线程的基本 / 调度优先级定义的. 通过 `sched_bucket_t` 描述, 这些调度存储桶大致映射到操作系统运行时使用的 QoS 类, 以定义各种工作的性能预期. |
| 2 | 线程组级别 (Thread Group Level)      | 第二个级别是线程组级别的, 实现了 FreeBSD ULE 调度器的变体, 它决定接下来应该选择存储桶中的哪个线程组 (用 `clutch buckets` 表示) 来执行. 线程组是 AMP 调度程序引入的一种机制, 它代表代表特定工作负载工作的线程集合. 存储桶中具有可运行线程的每个线程组都表示为此级别的条目. 这些条目在整个实现过程中称为 `clutch buckets`. 此级别的目标是在各种用户工作负载之间共享 CPU, 优先于交互式应用程序, 而不是计算密集型批处理工作负载. |
| 3 | 线程级别 (Thread Level)              | 线程级调度程序实现 Mach 分时算法, 以决定接下来应选择离合器桶中的哪个线程执行. `clutch buckets` 中的所有可运行线程都根据 schedpri 插入到运行队列中. 调度程序根据 `clutch buckets` 中可运行的线程数和各个线程的 CPU 使用率计算 `clutch buckets` 中线程的调度. 负载信息在每个调度程序时钟周期更新, 线程使用此信息进行优先级衰减计算, 因为它们使用 CPU. 优先级衰减算法尝试奖励突发交互式线程并惩罚 CPU 密集型线程. |



## 2.3.2 第一级桶调度 Scheduling Bucket Level
-------

存储桶级别 (scheduling bucket level) 调度器使用最早截止时间优先 (EDF) 算法来决定接下来应选择哪个根存储桶执行. 具有可运行线程的每个根存储桶 (root bucket) 都表示为优先级队列中的一个条目, 该队列按存储桶的截止时间排序.

存储桶选择算法 (bucket selection algorithm) 每次选择优先级队列中截止日期最早的根存储桶 (root bucket). 根存储桶 (root bucket) 的截止时间是根据其首次运行的时间戳及其为每个存储桶预定义的最坏情况执行延迟 (Worst Case Execution Latency/WCEL) 值计算的. WCEL 值是根据衰减曲线选择的, 然后是马赫分时算法, 以允许系统从更高层次的角度运行类似于现有调度程序的功能.


```
static uint32_t sched_clutch_root_bucket_wcel_us[TH_BUCKET_SCHED_MAX] = {
        SCHED_CLUTCH_INVALID_TIME_32,                   /* FIXPRI */
        0,                                              /* FG */
        37500,                                          /* IN (37.5ms) */
        75000,                                          /* DF (75ms) */
        150000,                                         /* UT (150ms) */
        250000                                          /* BG (250ms) */
};
```

### 2.3.3 第二级组调度 Thread Group Level
-------

### 2.3.4 第三级线程调度 Thread Level
-------



*   本作品 / 博文 [成坚 (gatieme) @ 内核干货 (OSKernelLAB)- 紫夜阑珊 - 青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚 (gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a><a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"> 知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议 </a> 进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名 [成坚 gatieme](http://blog.csdn.net/gatieme) (包含链接: http://blog.csdn.net/gatieme), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.
