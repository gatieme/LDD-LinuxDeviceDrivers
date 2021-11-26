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


# 1 POU 和 POC
-------

随着人类不断向更快的计算机系统进发, memory hierarchy 也变得异常复杂起来, cache也形成了cache hierarchy(ARMv8最大支持7个level, L1～L7), 不同级别的cache中都包含了部分下一级cache(或者main memory)的内容. 这时候, 维护数据一致性变得复杂了, 例如：当要操作(例如clean或者invalidate)某个地址对应的cacheline的时候, 是仅仅操作L1还是覆盖L1和L2, 异或将L1～L3中对应的cacheline都设置为无效呢？PoU(Point of Unification)和PoC(Point of Coherency)这两个术语就是用来定义cache操作范围的, 它们其实都是用来描述计算机系统中memory hierarchy的一个具体的“点”, 操作范围是从PE到该点的所有的memory level.


之所以区分 PoC 和 PoU, 根本原因是为了更好的利用cache中的数据, 提高性能.


```cpp
PoC: the PoC is the point at which all blocks, for example, cores, DSPs, or DMA engines, that canaccess memory are guaranteed to see the same copy of a memory location.

PoU: the PoU for a core is the point at which the instruction and data caches of the core are guaranteed to see the same copy of a memory location.
```

| 单元 | 描述 |
|:---:|:----:|
| Point of Unification(PoU) | 所有的 PE 可以观察到一致性的观察点, 一般是从 CPU 的维度, 即 CPU 间看到的 cache 一致性. 即 CPU 中的指令 Cache, 数据 cache 还有 MMU, TLB 等看到的是同一份的内存拷贝. |
| Point of Coherency(PoC) | 系统中所有的观察者(agent) 可以观察到一致性拷贝的点, 从 SoC 的较多个 master 之间看到的一致性, 例如DSP, GPU, CPU, DMA等都能看到同一份内存拷贝. 一般就是到memory层了. |

## 1.1 PoU
-------

先看PoU, PoU是以一个特定的PE(该PE执行了cache相关的指令)为视角. PE需要透过各级cache(涉及instruction cache、data cache和translation table walk)来访问 main memory, 这些操作在 memory hierarchy 的某个点上(或者说某个level上)会访问同一个copy, 那么这个点就是该PE的 Point of Unification.

假设一个4核cpu, 每个core都有自己的 L1 instruction cache和 L1 Data cache, 所有的 core 共享L2 cache.

在这样的一个系统中, PoU就是L2 cache, 只有在该点上, 特定PE的instruction cache、data cache和translation table walk硬件单元访问memory的时候看到的是同一个copy.


| 单元 | 描述 |
|:---:|:----:|
| PoU for a PE | 保证 PE 看到的 I/D cache 和 MMU 是同一份拷贝. 大多数情况下, PoU 是站在单系统的角度来观察的. |
| PoU for inner share | 意思是说在 inner share 里面的所有 PE 都能看到相同的一份拷贝. |


所以, PoU 有两个观察点

*   一个是 poc for a PE, PE就是 cpu core.

*   另外一个是 POU for inner share.

关于 PoU 的描述, 在 armv8.6 手册的第 D4.4.7 章里有详细的描述.

```cpp
Point of Unification (PoU)

    The PoU for a PE is the point by which the instruction and data caches and the translation table walks of that PE are guaranteed to see the same copy of a memory location. In many cases, the Point of Unification is the point in a uniprocessor memory system by which the instruction and data caches and the translation table walks have merged.

    The PoU for an Inner Shareable shareability domain is the point by which the instruction and data caches and the translation table walks of all the PEs in that Inner Shareable shareability domain are guaranteed to see the same copy of a memory location.
```

(PoU for inner share 对于 self-modify code 有一个好处, 可以简化 data cache 和指令 cache 一致性的步骤, 只需要简单两步, 不需要 memory barrier 指令了)

Defining this point permits self-modifying software to ensure future instruction fetches are associated with the modified version of the software by using the standard correctness policy of:

1. Clean data cache entry by address.

2. Invalidate instruction cache entry by address

## 1.2 Point of Coherency (PoC)
-------

PoC 可以认为是 Point of System, 它和 PoU 的概念类似, 只不过 PoC 是以系统中所有的 agent(bus master, 又叫做 observer, 包括CPU、DMA engine等)为视角, 这些 agents 在进行memory access的时候看到的是同一个copy的那个“点”.

例如上一段文章中的4核cpu例子, 如果系统中还有一个 DMA controller和main memory(DRAM) 通过 bus 连接起来, 在这样的一个系统中, PoC 就是 main memory 这个 level, 因为 DMA controller 不通过 cache 访问 memory, 因此看到同一个 copy 的位置只能是 main memory 了.


```cpp
Point of Coherency (PoC)
    The point at which all agents that can access memory are guaranteed to see the same copy of a memory location for accesses of any memory type or cacheability attribute. In many cases this is effectively the main system memory, although the architecture does not prohibit the implementation of caches beyond the PoC that have no effect on the coherency between memory system agents.
```


## 1.3 PoU 和 PoC 的区别
-------

PoC是系统的一个概念, 和系统配置相关, 包含 GPU,CPU,DMA 等有能力访问内存的设备, 我们称为观察者.

例如, Cortex-A53可以配置L2 cache和没有L2 cache, 可能会影响PoU的范围.


图 1
没有集成L2 cache, 只有一个 Core 和 I/D cache 以及 TLB, 所以 PoU 等于 PoC.

图 2
站在core1上看集成了L2 cache, 包含core, I/D cache TLB L2 cache组成了PoU for inter share
站在系统上看, 包含了core1、core2和系统内存, 整体组成了 PoC.

图 3
系统架构包含了两个cluster组成, 每个cluster包含了L1、L2 cache, 可以独立看成一个PoU.
两个Cluster共享了L3 cache和系统内存, 则可以是PoC

# 参考资料
-------

| 编号 | 链接 | 描述 |
|:---:|:----:|:---:|
| 1 | [armv8 cache PoU和PoC区别](https://blog.csdn.net/dai_xiangjun/article/details/120199782) | NA |
| 2 | [什么是PoU和PoC？](https://community.arm.com/cn/f/discussions/3437/pou-poc/9674) | NA |
| 3 | [CPU体系结构-Cache](https://zhuanlan.zhihu.com/p/36119440) | NA |
| 4 | [ARMV8 datasheet学习笔记4：AArch64系统级体系结构之存储模型](https://www.cnblogs.com/smartjourneys/p/6848018.html) | NA |
| 5 | [Linux内存管理：ARM64体系结构与编程之cache(2)：cache一致性](https://www.codenong.com/cs109826779) | NA |
| 6 | [RM64的启动过程之（三）：为打开MMU而进行的CPU初始化](https://blog.csdn.net/zdy0_2004/article/details/49309133) | NA |
| 7 | [[mmu/cache]-ARM cache的学习笔记-一篇就够了](https://blog.csdn.net/weixin_42135087/article/details/107064291)

<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
