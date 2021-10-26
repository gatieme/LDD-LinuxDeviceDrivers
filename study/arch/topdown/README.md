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

# 1 Top-Down 概述
-------

自顶向下的微架构分析方法(Top-Down Microarchitecture)

# 2 背景知识解析
-------

| 类型 | 描述 |
|:---:|:----:|
| [uOps](https://www.intel.com/content/www/us/en/develop/documentation/vtune-help/top/reference/cpu-metrics-reference/uops.html) | micro-ops/micro-operations/微指令 是一种底层硬件操作, CPU 前端负责获取体系结构指令中表示的程序代码, 并将其解码为一个或多个 uops. |
| [Pipeline Slots](https://www.intel.com/content/www/us/en/develop/documentation/vtune-help/top/reference/cpu-metrics-reference/pipeline-slots.html) | Pipeline Slot(流水线槽) 代表着一个 uOps 所需的硬件资源. Top-Down 分析方法假定对于每个 CPU 核, 在每个时钟周期, 有多个可用的 Pipeline Slots. 这些 Pipeline Slots 的数量被称为 Pipeline Width. |

## 2.1 uOps
-------


## 2.2 Pipeline Slots
-------

在下图的示例中, 是用 4-wide(4-way) 的 CPU 将代码执行 10个时钟周期.

![Pipeline Slots 的示例 1](./original_pipeline_slots.png)

在这个示例中, 包含了 40 个 Pipeline Slots(4 * 10) 资源. 如果一个 Pipeline Slot 没有 retire, 则认为有一个 uOp 产生阻塞(Stall).

如下图所示, 有 20 个 slots(红色) 产生阻塞(没有使一个 uOp 达到 Retire). 这表示从该微架构的角度来看, 代码的执行效率只有 50%.

![Pipeline Slots 的示例 2](./example_pipeline_slots.png)

以 Pipeline Slots 为基础资源所观测和统计到的 Top-Down 数据(如 Front-End Bound 和 Back-End Bound 等), 则表示因各种原因(如 Front-End 问题和 Back-End 问题)导致的 Pipeline Slots 阻塞占总体的百分比.

## 2.3 为什么以 Pipeline Slots 为粒度统计
-------

我们统计 top-down 数据是以 Pipeline Slots 为基准的, 而没有采用 cycle(Clockticks) 作为基准. 理论上将我们统计每个周期内阻塞的占比岂不是更有意义 ?

我们同样用一个示例来看下这个疑问 ?

![Pipeline Slots 的示例 2](./clockticks_pipeline_slots.png)

在这里, 每个周期有两个 Pipeline Slots 被阻塞, 也就是 50% 的 Stalls 和 50% 的 Retiring. 如果使用 Clockticks 而言, 却认为是 100% 的 Stalls, 因为每个周期(cycle)都有一些 Pipeline Slots 被占用.

可见, 与以 Pipeline Slots 测量的指标相比, 以 Clockticks 测量的指标不太精确. 但是, 此类指标对于识别代码中的主要性能瓶颈仍然很有用.



# 3 分析
-------


| 类型 | 描述 |
|:---:|:----:|
| Front-End/前端   | 负责获取程序代码指令, 并将其解码为一个或者多个微操作(uOps), 这些 uOps 将分配给 Back-End 去执行.<br>Front-end 负责交付 uOps 给 Back-end 执行. Front-end 从 Icache 中提取代码字节流到流水线, 通过分支预测器预测下一个地址以进行提取. 将代码字节分割成指令, 并发送给解码器, 解码器将指令解码到 uOps(mirco-ops), 以便交给 Back-end 执行. |
| Back-end/后端    | 负责监控 uOps 的数据何时可用, 并将其安排到可用的执行单元中执行. |
| Speculation/预测 | 部分跳转指令可能需要对跳转方向和跳转地址进行预测. 通常情况下, 大多数 uOps 都会通过流水线并正常退役, 但是在预测错误的情况下, 投机执行的 uOps 可能会在退役前被取消并从流水线中清楚掉. |
| Retirement/退役  | uOps 执行完成. 这被称为退役 |




| 类型 | 描述 |
|:---:|:----:|
| Front-End Bound | 表示 pileline 不足以供应 Back-end. 也就是说当 Back-end 准备接收 uOps 时, Pipeline Slots 出现了 stall. Front-End Bound 可以进一步分为:<br>1. Fetch Latency Bound: icache/tlb miss<br>2. Fetch Bandwidth Bound: sub-optimal decoding. |
| Back-end Bound    | 表示由于缺乏 uOps 执行所需的后端资源造成的停顿. 它可以进一步细分为:<br>1. Memory Bound: 由内存子系统造成的停顿卡顿.<br>2. Core Bound: 执行单元压力 Compute Bound 或者去缺少指令集并行 LTP.<br>3. Resource Bound |
| Bad Speculation | 由于分支预测错误导致的 Pipeline Slot 被浪费. 主要包括 Front-End 最终被取消的 uOps 的 pipeline Slots, 以及 Back End 过程中由于从先前错误的猜测中恢复而阻塞造成的 Pipeline Slot. |
| Retring  | 表示运行有效的 PileSlots. |


理想情况下, 我们希望所有看到的 Pileline Slot 都归类到 Retring. 因为他与 IPC 息息相关.
如果一个 Pipeline Slot 被某个 uOps 占用, 它将被分类到 Retring 或者 Bad Speculation, 具体取决于他是否被提交.
如果 Pipeline 被某个 Back-end 阶段 Pipeline Slot 都被占用, 无法接受更多的操作. 则未被利用的 Pileline Slot 就被归类为 Back-end Bound.
同样 Front-End Bound 则表示在没有 Back-end Stall 的情况下, 没有更多的 uOps 被分配给后端去处理.

在目前比较常见的 Intel X86_64 的微架构实现上, 流水线的 Front-end 每个 cycle 可以分配 4 个 uOps, 同样 Back-end 也可以在每个 cycle 中退役 4 个 uOps.
于是 TMAM 假定对于每个 CPU 核, 在每个 cycle, 均有 4 个 Pipeline Slot 可用, 然后使用一些自定义的 PMU 事件来测量这些 pipeline slot 的使用情况.

在每个 CPU 周期中, pipeline slot 可以是空的或者被 uOp 填充. 如果在一个 CPU 周期内某个 pipeline slot 是空的, 称之为一次停顿(stall). 如果 CPU 经常停顿, 系统性能肯定是受到影响的. TMAM 的目标就是确定系统性能问题的主要瓶颈.


# 1 参考资料
-------

| 编号 | 链接 | 描述 |
|:---:|:----:|:---:|
| 1 | [A Journey Through the CPU Pipeline](https://bryanwagstaff.com/index.php/a-journey-through-the-cpu-pipeline/) | 讲述了 CPU 流水线的前世今生(不断演进和完善), [翻译版本](https://kb.cnblogs.com/page/179578) |
| 2 | [Top-down Microarchitecture Analysis Method](https://www.intel.com/content/www/us/en/develop/documentation/vtune-cookbook/top/methodologies/top-down-microarchitecture-analysis-method.html) |
| 3 | [A Top-Down method for performance analysis and counters architecture](https://www.researchgate.net/publication/269302126_A_Top-Down_method_for_performance_analysis_and_counters_architecture) | Intel 关于 topdown 分析方法的论文, 以及 [slide](https://pdfs.semanticscholar.org/b5e0/1ab1baa6640a39edfa06d556fabd882cdf64.pdf) |
| 4 | [Intel P4 CPU](https://www.cnblogs.com/linhaostudy/p/9204107.html) | |
| 5 | [The Berkeley Out-of-Order Machine (BOOM)](https://docs.boom-core.org/en/latest/sections/intro-overview/boom.html) | |
| 6 | [Top-down Microarchitecture Analysis through Linux perf and toplev tools](http://www.cs.technion.ac.il/~erangi/TMA_using_Linux_perf__Ahmad_Yasin.pdf)

<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
