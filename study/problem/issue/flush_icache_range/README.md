---

title: flush_icache_range()
date: 2021-01-24 18:40
author: gatieme
tags:
        - debug
        - linux
        - todown
categories:
        - debug

thumbnail:
blogexcerpt: 这篇文章旨在帮助希望更好地分析其应用程序中性能瓶颈的人们. 有许多现有的方法可以进行性能分析, 但其中没有很多方法既健壮又正式. 而 TOPDOWN 则为大家进行软硬协同分析提供了无限可能. 本文通过 pmu-tools 入手帮助大家进行 TOPDOWN 分析.


---

| CSDN | GitHub | OSKernelLAB |
|:----:|:------:|:-----------:|
| [紫夜阑珊-青伶巷草](https://blog.csdn.net/gatieme/article/details/113269052) | [`LDD-LinuxDeviceDrivers`](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/debug/tools/topdown/pmu-tools) | [Intel CPU 上使用 pmu-tools 进行 TopDown 分析](https://oskernellab.com/2021/01/24/2021/0127-0001-Topdown_analysis_as_performed_on_Intel_CPU_using_pmu-tools/) |

<br>

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作.

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 鄙人在此谢谢啦

<br>


# 1 问题描述(ARM64 指令被修改后入如何刷 icache)
-------

## 1.1 CPU cache
-------

CPU 和主存之间也存在多级高速缓存, ARM64 上一般分为 3 级, 分别是 L1, L2 和 L3.

其中 :

1.      L1 Cache比较特殊, 每个 CPU 会有 2 个 L1 Cache, 分别为指令高速缓存(Instruction Cache, 简称iCache)和数据高速缓存(Data Cache, 简称dCache).

2.      L2 和 L3 一 不区分指令和数据, 可以同时缓存指令和数据.


## 1.2 为什么要区分指令和数据
-------

iCache 用于缓存指令, dCache 则用于缓存数据. 为什么我们需要区分数据和指令呢?

原因之一是出于性能的考量. CPU在执行程序时, 可以同时获取指令和数据, 做到硬件上的并行, 提升性能. 另外, 指令和数据有很大的不同. 例如, 指令一般不会被修改, 所以 iCache 在硬件设计上是可以是只读的, 这在一定程度上降低硬件设计的成本.



# 2 iCache 和 dCache 一致性
-------


一般来说指令是不会被修改的. 那自然就不会存在一致性问题. 但是, 总有些特殊情况.

内核经常会有一些改指令的需求, 比如 self-modifying 场景, 以及 static_key(jump_label), 甚至是一些调测特性 ftrace, kprobe 等, 都需要对指令进行修改, 甚至是运行时的指令修改.

## 2.1 I/D cache 一致性问题
-------

而我们需要修改指令时, 只能将指令当成数据, 通过 dcache 进行修改, 这样就存在了 icache 和 dcache 的一致性问题.

1.      icache hit 的情形:

        如果旧指令已经缓存在 iCache 中. 那么 CPU 继续执行该指令会直接命中 iCache, 这样执行的就是旧指令, 显然不是我们预期的结果。

2.      icache miss 的情形:

        如果旧指令没有缓存到 iCache, 那么指令会从更高级别的 cache 或者直接从主存中缓存到 iCache 中. 这时候 dCache 更新的新指令可能被没有写回到更高级别的 cache 或者主存中, 那么新指令依然只缓存在 dCache 中, 并不被外界其他组件感知, 因此 icache 将仍然加载了旧的指令, 也存在问题.

此外, 除了 I/D cache 的一致性问题, 由于 CPU 流水线的存在, 因此旧的指令可能已经被放置在流水线, 还没被执行.



## 2.2 PoU & PoC
------

| 单元 | 描述 | 代表层级 |
|:----|:----:|:------:|
| PoU(Point of unification) | cache 的统一性, 一般是从 cpu 的维度, 即 cpu 间看到的 cache 一致性. 架构上要求在这一层级, I/D cache 的一致性要保证. | L2(由于 L2 不区分 Icache 和 Dcache, 因此同一 PA 在 L2 的缓存时一份, 则一般来说, CPU 从这一层级命中的数据再缓存的 icache 和 dcache 可以是一致的.)|
| PoC (Point of coherency) |
cache的一致性，这个是大一统，即 SoC 较多, 多个master之间看到的一致性 | 一般就是到 L3 和 memory层了, memory 较普遍一些, 比如cpu和dma看到的一致性 |


## 2.2 硬件维护一致性
-------

I/D cache 的一致性可以直接由硬件来维护.

硬件上可以让 iCache 和 dCache 之间通信, 每一次修改 dCache 数据的时候, 硬件负责查找 iCache 是否命中, 如果命中, 也更新iCache. 当加载指令的时候, 先查找 iCache, 如果 iCache 没有命中, 再去查找 dCache 是否命中, 如果 dCache 没有命中, 从主存中读取. 这确实解决了问题, 软件基本不用维护两者一致性. 但是本身改指令只是很少的情况, 为了解决少数的情况, 却给硬件带来了很大的负担, 得不偿失. 因此, 大多数情况下由软件维护一致性.

## 2.3 软件维护一致性
-------


当操作系统发现修改的数据可能是代码时, 可以采取下面的步骤维护一致性.


| 步骤 | 描述 |
|:---:|:---:|
| 指令修改 | 将需要修改的指令数据加载到 dCache 中. 修改成新指令后, 写回 dCache. |
| IDC | 保证 icache miss 场景的一致性<br>将 dcache 修改的指令写回到 POU/POC, 这样当所有 icache miss(如果旧指令没有缓存到 iCache) 的情况下, 这样会在 POU/POC 层次 hit, 并重新加载到 icache 中, 由于 POU/POC 层次的指令时新的(与 dcache 一致), 所以 icache 缓存到的也是一致的新指令. |
| DIC | 保证 icache hit 场景的一致性<br>对于已经缓存在 icache 中的指令, 选择 invalid 掉对应的(层次低于 POU/POC 的所有) cacheline, 保证指令下次读取时将发生 icache miss, 并只能至少从 POU/POC 层次读取. |
| ISB | 前面 IDC/DIC 保证了不同情形下的 I/D cache 一致性. 但是已经放置在流水线的指令, 他们无能为力. 这种情况下只能选择通过 ISB 冲刷流水线, 要求 CPU 重新取指. |



# 3 flush_icache_range()
-------


# 4 参考资料
-------

[CPU体系结构-Cache](https://zhuanlan.zhihu.com/p/36119440)

[iCache和dCache一致性](https://zhuanlan.zhihu.com/p/112704770)

<br>

*	本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*	采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*	基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.
