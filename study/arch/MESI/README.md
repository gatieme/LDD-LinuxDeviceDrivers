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
| 1 | [A Journey Through the CPU Pipeline](https://bryanwagstaff.com/index.php/a-journey-through-the-cpu-pipeline/) | 讲述了 CPU 流水线的前世今生(不断演进和完善), [翻译版本](https://kb.cnblogs.com/page/179578) |
| 2 | [Intel P4 CPU](https://www.cnblogs.com/linhaostudy/p/9204107.html) | |
| 3 | [The Berkeley Out-of-Order Machine (BOOM)](https://docs.boom-core.org/en/latest/sections/intro-overview/boom.html) | |
| 4 | [知乎专栏-从零开始学risc-v设计](https://www.zhihu.com/column/c_1247894081075892224) | NA |
| 5 | [开源处理器Rocket的分支预测机制研究与性能评估(一)](https://blog.csdn.net/leishangwen/article/details/72877452) | | NA |
| 6 | [简书专栏-Rocket Chip](https://www.jianshu.com/nb/41934049) | NA |
| 7 | BOOM微架构学习——[0-前端与分支预测](https://zhuanlan.zhihu.com/p/379874172), [1-取指单元与分支预测](https://zhuanlan.zhihu.com/p/168755384), [2-译码单元与寄存器重命名](https://zhuanlan.zhihu.com/p/194075590), [详解寄存器重命名技术](https://zhuanlan.zhihu.com/p/399543947) | 以 BOOM 为例分析了取值单元以及分支预测器的实现 |
| 8 | [知乎专栏-大金哥的超标量处理器学习笔记](https://www.zhihu.com/column/c_1253708282457079808) | 总结了 <超标量处理器> 一书的基本内容, 对流水线各个阶段做了一定的解读 |
| 9 | [Skylake Microarchitecture](https://zhuanlan.zhihu.com/p/419876736) | 对 Skylake 微架构做了比较详细的分析 |
| 10 | [《手把手教你设计CPU——RISC-V处理器》读书笔记](https://zhuanlan.zhihu.com/p/90104625) | NA |
| 11 | [RISC-V的“Demo”级项目——Rocket-chip](https://zhuanlan.zhihu.com/p/140360043) | NA |


<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
