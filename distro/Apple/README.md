2   **OpenAnolis cloud-kernel 分析**
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
| [Aderstep--紫夜阑珊-青伶巷草](http://blog.csdn.net/gatieme) | [`AderXCoding/system/tools`](https://github.com/gatieme/AderXCoding/tree/master/system/tools) | [gatieme.github.io](https://gatieme.github.io) |

<br>

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作

<br>

# 1 总体架构
-------

首先一图解百忧:

[Apple IOS 总体架构](./Apple_iOS_Architecture.jpg)

[iOS&mac 系统内核](https://www.cnblogs.com/qiyer/p/13252630.html)

[Kernel Architecture Overview](https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/KernelProgramming/Architecture/Architecture.html)

[XNU 简介](https://www.jianshu.com/p/cab1bfab4fb7)

Apple 的内核叫做 XNU(XNU's Not UNIX), 这是个混合内核, XNU 包含两个部分, BSD 和 Mach

1.      BSD 实现了 POSIX, Networking, Filesystem 等

2.      Mach 实现了最为基本的多处理器支持, 虚拟内存, 分页, 调度, IPC, RPC 等.

3.      在 Mach 的基础上, 就是 IOKit 和各类 Kext 了.


# 2 SCHED
-------

## 2.1 CONFIG_SCHED_EDGE
-------


sched_update_pset_load_average


## 2.2 CONFIG_SCHED_MULTIQ
-------


## 2.3 CONFIG_SCHED_CLUTCH
-------

xnu-6153.11.26
> The clutch scheduler is enabled only on non-AMP platforms for now

[Mach Overview](https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/KernelProgramming/Mach/Mach.html#//apple_ref/doc/uid/TP30000905-CH209-TPXREF101)


# 3 IPC
-------

[Using Continuations to Implement Thread Management and Communication in Operating Systems](https://xueshu.baidu.com/usercenter/paper/show?paperid=f90d3d724dd390723bcfaffa88edcfda)

[Interface and Execution Models in the Fluke Kernel](http://staff.ustc.edu.cn/~bjhua/courses/ats/2014/ref/ford99interface.pdf)

在 [Mach3.0 中对系统线程所作的一项改进即称为 continuation](https://www.gnu.org/software/hurd/microkernel/mach/gnumach/continuation.html), 其动因恰在于避免保留线程堆栈, 希望使用完全无状态的 continuation 函数.(参见 Uresh Vahalia 的经典著作 "UNIX Internals" http://www.china-pub.com/computers/common/info.asp?id=12731).

<br>

*   本作品/博文 [成坚(gatieme) @ 内核干货(OSKernelLAB)-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.
