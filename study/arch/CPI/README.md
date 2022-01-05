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



# 1 CPI
-------


指令周期数(CPI)是一个很重要的高级指标, 用来描述 CPU 如何使用它的时钟周期, 同时也可以用来理解 CPU 使用率的本质.

这个指标也可以被表示为每周期指令数(instructions per cycle, IPC), 即 CPI 的倒数.

CPI 较高代表 CPU 经常陷入停滞, 通常都是在访问内存. 而较低的 CPI 则代表 CPU 基本没有停滞, 指令吞吐量较高.

通常情况下, 通过 CPI 的取值, 我们可以大致判断一个计算密集型任务, 到底是 CPU bound 还是 Memory Bound:

*   CPI 小于 1, 程序通常是 CPU Bound;

*   CPI 大于 1, 程序通常是 Memory Bound;

这些指标指明了性能调优的主要工作方向. 内存访问密集的负载, 可以通过下面的方法提高性能, 如使用更快的内存(DRAM)、提高内存本地性(软件配置), 或者减少内存 I/O 数量.

使用更高时钟频率的 CPU 并不能达到预期的性能目标, 因为 CPU 还是需要为等待内存 I/O 完成而花费同样的时间. 换句话说, 更快的 CPU 意味着更多的停滞周期, 而指令完成速率不变. CPI 的高低与否实际上和处理器以及处理器功能有关, 可以通过实验方法运行已知的负载得出.

例如, 你会发现高 CPI 的负载可以使 CPI 达到 10 或者更高, 而在低 CPI 的负载下, CPI 低于 1(受益于前述的指令流水线和宽度技术, 这是可以达到的).

值得注意的是, CPI 代表了指令处理的效率, 但并不代表指令本身的效率. 假设有一个软件改动, 加入了一个低效率的循环, 这个循环主要在操作 CPU 寄存器(没有停滞周期): 这种改动可能会降低总体 CPI, 但会提高 CPU 的使用和利用度


# 2 CPI stack
-------



## 2.2 学术研究
-------

[Mluti-Strage CPI Stacks](https://heirman.net/papers/eyerman2018mscs.pdf)

[Extending the Performance Analysis Tool Box: Multi-Stage CPI Stacks and FLOPS Stacks](https://heirman.net/papers/eyerman2018etpatbmscsafs.pdf)

[An Efficient CPI Stack Counter Architecture for Superscalar Processors](https://users.elis.ugent.be/~leeckhou/papers/glsvlsi12.pdf)

[Using Cycle stacks to Understand Scaling Bottlenecks in Multi-Threaded workloads](https://www.cs.virginia.edu/~skadron/Papers/iiswc2011heirman.pdf)

[A performance counter architecture for computing accurate CPI components](https://xueshu.baidu.com/usercenter/paper/show?paperid=103r00p0fm2u04k0ta1j0020v2210792&site=xueshu_se)
https://jes.ece.wisc.edu/papers/Perf_Ctr_ASPLOS.pdf
https://users.elis.ugent.be/~leeckhou/papers/asplos06.pdf

[A Top-Down Approach to Architecting CPI Component Performance Counters](https://xueshu.baidu.com/usercenter/paper/show?paperid=f8386b09436480d8ed1f1d8460e29c97) https://users.elis.ugent.be/~leeckhou/papers/toppicks07.pdf

[CPCI Stack: Metric for Accurate Bottleneck Analysis on OoO Microprocessors](https://www.semanticscholar.org/paper/CPCI-Stack%3A-Metric-for-Accurate-Bottleneck-Analysis-Tanimoto-Ono/3a7420ec4f5369e67f658af17563d98efe19733e)

## 2.3 开源工具
-------

[Sniper CPI Stacks Generate Tools](http://snipersim.org/w/CPI_Stacks)


[GitHub open-power-sdk/cpi-breakdown](https://github.com/open-power-sdk/cpi-breakdown)
[GitHub mukul297/CpiStack](https://github.com/mukul297/CpiStack)
[GitHub rajjha1997/CPI_StackP](https://github.com/rajjha1997/CPI_Stack)
[GitHub prateekmds/CPI-stack](https://github.com/prateekmds/CPI-stack)
[GitHub chetancppant/CPI_STACK](https://github.com/chetancppant/CPI_STACK)
[GitHub paras2411/cpi-stack-prediction](https://github.com/paras2411/cpi-stack-prediction)



## 2.3 工业实践
-------

IBM 的 [POWER5](http://www.ee.unlv.edu/~meiyang/ecg700/readings/ibmpower5chipadualcore.pdf) 是最早支持 CPI stack 跟踪的乱序(Out-of-Order)处理器, 随后在 POWER7, [POWER8](https://old.hotchips.org/wp-content/uploads/hc_archives/hc26/HC26-12-day2-epub/HC26.12-8-Big-Iron-Servers-epub/HC26.12.817-POWER8-Mericas-IBM%20Revised-no-spec.pdf) 中不断优化, 大放异彩. 参见 [POWER8 Performance Analysis](https://openpowerfoundation.org/wp-content/uploads/2015/03/Sadasivam-Satish_OPFS2015_IBM_031615_final.pdf).

Intel 随后在 [A Top-Down method for performance analysis and counters architecture](https://www.semanticscholar.org/paper/A-Top-Down-method-for-performance-analysis-and-Yasin/6776ff919597ca4feccd413208dedc401f6e655d#extracted) 中进一步扩展了 CPI stack, 进一步对微架构的事件进行了拆解, 并引入了 TOPDOWN(自顶向下) 的分析方法.

TOPDOWN 可以看作是 CPI Stack 的分析的扩展实现, 自此替代传统的 CPI stack 分析成为业界目前主流的微架构分析方法.










































<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
