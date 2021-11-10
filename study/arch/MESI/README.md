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

![boom 流水线](boom-pipeline-detailed.png)



# 1 参考资料
-------

| 编号 | 链接 | 描述 |
|:---:|:----:|:---:|
| 1 | [理解Memory Barrier（内存屏障）](https://blog.csdn.net/caoshangpa/article/details/78853919) | NA |
| 2 | [内存屏障（Memory Barrier）与volatile](https://blog.csdn.net/fedorafrog/article/details/113782404) | NA |
| 3 | [OR1200处理器的Store Buffer分析](https://blog.csdn.net/leishangwen/article/details/35212811) | NA |
| 4 | [MESI 与 StoreBuffer 相互独立的猜想](https://www.cnblogs.com/lqlqlq/p/14414091.html) | NA |
| 5 | [内存屏障的来历](https://zhuanlan.zhihu.com/p/125549632) | NA |
| 6 | [内存屏障（Memory Barrier）究竟是个什么鬼？](https://www.bilibili.com/read/cv5131208) | NA |
| 7 | [处理器中的存储问题（三）：从Store Buffer到TSO存储模型](https://zhuanlan.zhihu.com/p/149302247) | NA |
| 8 | [CPU 缓存和 volatile](https://www.cnblogs.com/xmzJava/p/11417943.html) | MESI 有个图表 | NA |
| 9 | [CPU多级缓存与缓存一致性，详细的讲解](https://blog.csdn.net/weixin_43649997/article/details/108742221) | NA |
| 10 | [CPU有缓存一致性协议(MESI)，为何还需要volatile](https://blog.csdn.net/org_hjh/article/details/109626607) | NA |
| 11 | [笔记：cpu中的cache（二）](https://zhuanlan.zhihu.com/p/144836286) | NA |
| 12 | [Reducing Design Complexity of the Load/Store Queue](https://engineering.purdue.edu/~vijay/papers/2003/lsq.pdf) | NA |
| 13 | [乱序处理器中的LSQ简介](https://blog.csdn.net/baidu_35679960/article/details/79554428) | NA |
| 14 | [简述 典型处理器（如Cortex A9）中一条存储器读写指令的执行全过程](https://blog.csdn.net/baidu_35679960/article/details/78571097) | MA |
| 15 | [4.2 存储器读写指令的发射与执行1](https://blog.csdn.net/sailing_w/article/details/55003968) | NA |
| 16 | [乱序处理器中的LSQ简介](https://blog.csdn.net/baidu_35679960/article/details/79554428) | NA |
| 17 | [Volatile：内存屏障原理应该没有比这篇文章讲的更清楚了](https://www.bilibili.com/read/cv7429219/) | NA |
| 18 | [内存一致性模型](http://www.wowotech.net/memory_management/456.html) | NA |
| 19 | [从硬件层面理解memory barrier](https://zhuanlan.zhihu.com/p/184912992) | NA |
| 20 | [多处理器编程：从缓存一致性到内存模型](https://zhuanlan.zhihu.com/p/35386457) | NA |
| 21 | [聊聊原子变量、锁、内存屏障那点事（2） ](https://www.sohu.com/a/250274701_467784) | NA |
| 22 | [Spectre原理详解及分支毒化的实现](https://zhuanlan.zhihu.com/p/114680178) | NA |
| 23 | [Lecture 11: Memory Data Flow Technique](http://home.eng.iastate.edu/~zzhang/courses/cpre585_f03/slides/lecture11.pdf) | NA |
| 24 | [cpu 乱序执行与问题](https://blog.csdn.net/lizhihaoweiwei/article/details/50562732) | NA |
| 25 | [store-queue VS store-buffer](https://stackoverflow.com/questions/24975540/what-is-the-difference-between-a-store-queue-and-a-store-buffer) | NA |

<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
