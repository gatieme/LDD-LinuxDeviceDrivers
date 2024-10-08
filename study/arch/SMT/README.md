 ---

title: SMT
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
| 2022/01/03 | [成坚-gatieme](https://kernel.blog.csdn.net) | [`AderXCoding/system/tools/fzf`](https://github.com/gatieme/AderXCoding/tree/master/system/tools/fzf) | [AMP 异构多核系统研究](https://blog.csdn.net/gatieme/article/details/113828826) | [Using FZF to Improve Productivit](https://oskernellab.com/2021/02/15/2021/0215-0001-Using_FZF_to_Improve_Productivity)|


<br>



# 1 Rentable Units
-------


英特尔近日申请了一项新的技术专利 [METHODS AND APPARATUS TO SCHEDULE PARALLEL INSTRUCTIONS USING HYBRID CORES - Intel Corporation (freepatentsonline.com)](https://www.freepatentsonline.com/y2023/0168898.html), 介绍了跨混合架构的并行调度方案, 正式确认了 "Rentable Unit" 方案. 这项技术作为一个超线程 SMT 技术的继任者叫作, 相比较传统同步多线程(SMT)技术, 灵活性和可行性更高.

| 编号 | 资料 | 描述 |
|:---:|:----:|:---:|
| 1 | [Intel 多线程秘密武器: 详解「Rentable Units」](https://mp.weixin.qq.com/s/sR9aKrkAlDTWLWVV7bJWxg) |  头发实验室公众号, 介绍 "Rentable Units" |
| 2 | [英特尔专利曝光超线程技术继任者, 更灵活调度、高效处理任务](https://ngabbs.com/read.php?&tid=37385627) | 精英玩家俱乐部论坛对 "Rentable Units" 的介绍 |
| 3 | [Intel 17th Gen CPUs to Get Rentable Units: Why Hyper-Threading is Going Away](https://www.hardwaretimes.com/intel-15th-gen-cpus-to-get-rentable-units-why-hyper-threading-is-going-away) | 介绍了 SMT 以及 "Rentable Unit" 的基本原理. |
| 4 | [Intel Patent Reveals the ‘Rentable Unit’, Marking the End for Hyperthreading](https://appuals.com/intel-rentable-unit-patent) | 简单分析了下 "Rentable Unit" 的基本原理. |

英特尔在 Alder Lake(第 12 代) 处理器上, 采用了大小核 (性能 + 效率) 的混合架构设计, 通过动态算法, 将复杂的任务交由性能核心 (P 核) 处理, 后台任务交由效率 (E 核) 处理. P 核启用了 SMT, 物理内核即使具有 2 个逻辑线程, 1 次也仅能执行 1 条指令, 无法并行工作. 英特尔为了克服并行性问题, 引入了 E 核, 而 "Rentable Unit" 就是超线程技术的继任者.

"Rentable Unit" 工作原理如下:

Rentable Unit 接收指令, 并根据指令的复杂性、可用资源和 CPU 结构对其进行划分.

Rentable Unit 根据任务调度 P 核和 E 核, 确保所有核心都处于活跃状态.

该原理就是确保没有核心处于闲置状态, 如果 P 核处于闲置状态下, 也可以处理 E 核的任务.

它会尝试预测任务在 P/E 核上的完成时间然后试图分割任务以更小粒度跨核调度, 调度会更复杂, 上下文切换会更多, 优化目标可能会倾向吞吐量所以 P 核上高优任务延迟可能会被影响: P 核高优任务可能分一部分扔去 E 核, 而 E 核低优任务可能分一部分拿来 P 核跑.


<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
