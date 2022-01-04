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






# 1 CPI stack
-------


Van Craeynest 提出了性能影响的评估模型（performance impact estimation PIE）

大核和小核的 MLP 和 ILP 的比值可以很好的说明核间的性能差异
当应用在某一个 core 上执行时，收集 ILP 和 MLP 信息，以及 CPI 堆栈。利用收集的信息，估计另一个类型的 core 上的应用程序的性能
简化的思想，根据程序运行时，core 利用 ILP 和 MLP 对整体性能的影响程度来决定调度方案


# 2 学术研究
-------

[Mluti-Strage CPI Stacks](https://heirman.net/papers/eyerman2018mscs.pdf)

[Extending the Performance Analysis Tool Box: Multi-Stage CPI Stacks and FLOPS Stacks](https://heirman.net/papers/eyerman2018etpatbmscsafs.pdf)

[An Efficient CPI Stack Counter Architecture for Superscalar Processors](https://users.elis.ugent.be/~leeckhou/papers/glsvlsi12.pdf)

[Using Cycle stacks to Understand Scaling Bottlenecks in Multi-Threaded workloads](https://www.cs.virginia.edu/~skadron/Papers/iiswc2011heirman.pdf)

[A performance counter architecture for computing accurate CPI components](https://xueshu.baidu.com/usercenter/paper/show?paperid=103r00p0fm2u04k0ta1j0020v2210792&site=xueshu_se)
https://jes.ece.wisc.edu/papers/Perf_Ctr_ASPLOS.pdf
https://users.elis.ugent.be/~leeckhou/papers/asplos06.pdf

[A Top-Down Approach to Architecting CPI Component Performance Counters](https://xueshu.baidu.com/usercenter/paper/show?paperid=f8386b09436480d8ed1f1d8460e29c97) https://users.elis.ugent.be/~leeckhou/papers/toppicks07.pdf


# 3 工业实践
-------


https://personals.ac.upc.edu/marc/Marc_Gonzalez_Home_page/Material_PoTra_files/POTRA%20tutorial%20-%20PART%201.pdf


https://old.hotchips.org/wp-content/uploads/hc_archives/hc26/HC26-12-day2-epub/HC26.12-8-Big-Iron-Servers-epub/HC26.12.817-POWER8-Mericas-IBM%20Revised-no-spec.pdf
https://openpowerfoundation.org/wp-content/uploads/2015/03/Sadasivam-Satish_OPFS2015_IBM_031615_final.pdf


# 4 开源工具
-------

[Sniper CPI Stacks Generate Tools](http://snipersim.org/w/CPI_Stacks)


[GitHub open-power-sdk/cpi-breakdown](https://github.com/open-power-sdk/cpi-breakdown)
[GitHub mukul297/CpiStack](https://github.com/mukul297/CpiStack)
[GitHub rajjha1997/CPI_StackP](https://github.com/rajjha1997/CPI_Stack)
[GitHub prateekmds/CPI-stack](https://github.com/prateekmds/CPI-stack)
[GitHub chetancppant/CPI_STACK](https://github.com/chetancppant/CPI_STACK)
[GitHub paras2411/cpi-stack-prediction](https://github.com/paras2411/cpi-stack-prediction)









































<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
