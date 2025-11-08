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

   **工具**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***


# 1 开发工具
-------


社区一直在寻求一种非邮件列表的内核开发方式

1.  [Pulling GitHub into the kernel process](https://lwn.net/Articles/860607)

2.  [How Red Hat uses GitLab for kernel development](https://lwn.net/Articles/871237)

3.  [Moving Google toward the mainline](https://lwn.net/Articles/871195)


# 2 调试工具
-------

## 2.1 debug
-------

systrace, perfetto, drgn, systemtep

[All my favorite tracing tools: eBPF, QEMU, Perfetto, new ones I built and more](https://thume.ca/2023/12/02/tracing-methods) 将 eBPF 跟踪与 Perfetto 可视化相结合, 通过使用一些巧妙的技巧来诊断大量跟踪中的尾部延迟问题.

[Perfetto数据流架构故障分析：带你研究 trace 为何丢失](https://blog.csdn.net/feelabclihu/article/details/136266302)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/05/29 | Sandoval @ Facebook | [A kernel debugger in Python: drgn](https://lwn.net/Articles/789641) | 一款 kernel 调试器, 可以利用 Python 脚本来访问正在运行的 kernel 里的数据结构<br>Drgn 在 [LSFMM2019](https://lwn.net/Articles/lsfmm2019) 被公开, LWN 随即进行了报道  [A kernel debugger in Python: drgn](https://lwn.net/Articles/789641), 随后 [2021 年报道](https://lwn.net/Articles/878309/)了 [drgn: How the Linux Kernel Team at Meta Debugs the Kernel at Scale (Meta)](https://developers.facebook.com/blog/post/2021/12/09/drgn-how-linux-kernel-team-meta-debugs-kernel-scale), Meta 的内核团队使用了 Drgn 跟踪内核中出现的问题, 并贡献了非常分布的用例集合. | ☐ | [Github](https://github.com/osandov/drgn), [User Guide](https://drgn.readthedocs.io/en/latest/user_guide.html), [BPF drgn tools](https://www.kernel.org/doc/html/latest/bpf/drgn.html) |
| 2021/11/21 | David Woodhouse <dwmw2@infradead.org> | [Microsoft-Performance-Tools-Linux-Android](https://www.phoronix.com/scan.php?page=news_item&px=MS-Performance-Tools-Linux) | Microsoft [发布了](https://devblogs.microsoft.com/performance-diagnostics/new-tools-for-analyzing-android-linux-and-chromium-browser-performance)一款 Linux/Android 的性能分析工具 [Performance Tools For Linux/Android](https://github.com/microsoft/Microsoft-Performance-Tools-Linux-Android), 一组跟踪处理工具, 它提供了许多强大的功能、功能、方法和生态系统, 这些功能、功能、方法和生态系统在过去 15 年多的时间里一直用于通过不断发展和更新的工具集来推动系统的性能. 支持通过 LTTng、perf 和 Perfetto 对系统进行分析. | v1 ☐ | [LWN](https://github.com/microsoft/Microsoft-Performance-Tools-Linux-Android) |
| 2025/01/13 | Oracle OLED | [phoronix, 2025/01/13, Oracle OLED Wants To Help Improve The Debugability Of The Linux Kernel](https://www.phoronix.com/news/Oracle-Linux-OLED-Debug) | 甲骨文公司推出 Oracle Linux 增强诊断(OLED) 作为其最新项目, 旨在增强 Linux 内核的可调试性. Oracle Linux 增强诊断是 Oracle 内部开发的一组工具, 最初是针对 Oracle Linux 开发的, 可帮助增强调试体验, 尤其是在云环境中. | v1 ☐ | [Introducing Oracle Linux Enhanced Diagnostics](https://blogs.oracle.com/linux/post/oracle-linux-enhanced-diagnostics) |


*   DRGN

[LWN/A kernel debugger in Python: drgn(https://lwn.net/Articles/789641)
[LWN: 想用python命令来调试kernel吗？drgn就是了！](https://blog.csdn.net/Linux_Everything/article/details/93270705)
[pf-kernel/crush](https://codeberg.org/pf-kernel/crush) crush 是一种实验性尝试, 至少部分地使用 Python 和更现代的 drgn 后端重写 crash util.
[phoronix, 2025/01/18, Linux 6.14 To Perform Better With The Drgn Debugger Via Faster /proc/kcore Reads](https://www.phoronix.com/news/Linux-6.14-Faster-kcore-Reads)

*   ControlFlag

Intel 发布的 ControlFlag 用机器学习来发现代码中的错误, 支持 C/C++. 参见:

[Intel Makes ControlFlag Open-Source For Helping To Detect Bugs In Code](https://www.phoronix.com/scan.php?page=news_item&px=Intel-ControlFlag-Open-Source)

[Intel's ControlFlag 1.2 Released To Use AI To Provide Full Support For Spotting C++ Bugs](https://www.phoronix.com/scan.php?page=news_item&px=Intel-ControlFlag-1.2).

*  systemtap

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/05/29 | Alex Feng <lifeng1519@gmail.com> | Kernel Visualization Tool | 该工具用于分析 Linux/Solaris/BSD 内核指定流程的调用关系链, 并绘制特定函数的调用图，帮助开发者理解代码. | ☐ | [Kernel Visualization Tool](https://github.com/x2c3z4/kernel_visualization) |

*  Dtrace

[phoronix, 2024/04/08, Oracle Releases DTrace 2.0.0-1.14 For Linux Systems](https://www.phoronix.com/news/D-Trace-2.0.0-1.14)

[phoronix, 2024/10/23, Gentoo Linux Touts DTrace 2.0 Support](https://www.phoronix.com/news/Gentoo-Linux-DTrace-2.0), [DTrace 2.0 for Gentoo](https://www.gentoo.org/news/2024/10/23/DTrace-for-Gentoo.html), [DTrace-WIKI](https://wiki.gentoo.org/wiki/DTrace)

* kLLDB

[djolertrk/kLLDB](https://github.com/djolertrk/kLLDB)

* LTTng


[使用 PerfCollect 跟踪 .NET 应用程序](https://learn.microsoft.com/zh-cn/dotnet/core/diagnostics/trace-perfcollect-lttng)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2025/07/14 | Mathieu Desnoyers <mathieu.desnoyers@efficios.com> | [[RFC] LTTng upstreaming next steps](https://lore.kernel.org/lkml/b554bfa3-d710-4671-945b-5d6ec49e52cd@efficios.com) | phoronix 报道 [phoronix, 2025/07/14, New Effort To Upstream LTTng In The Linux Kernel Draws Criticism From Torvalds](https://www.phoronix.com/news/LTTng-Kernel-Modules-2025). | v7 ☐☑✓ | [2025/07/14, LORE RFC](https://lore.kernel.org/lkml/b554bfa3-d710-4671-945b-5d6ec49e52cd@efficios.com) |

## 2.2 call kernel func from userspace
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/06/23 | Peiyong Feng <peiyong.feng.kernel@gmail.com> | [Implement uhook(call kernel func from userspace) driver](https://lwn.net/Articles/503414) | 提供了 uhook(userspace kernel hook) 驱动和工具用于在用户态直接调用内核态的函数, 这对于调试内核非常有帮助. | v1 ☐ | [LWN](https://lwn.net/Articles/503414), [bigfeng12/uhook](https://github.com/bigfeng12/uhook)<br>*-*-*-*-*-*-*-* <br>[LKML1](https://lkml.org/lkml/2012/6/22/570), [LKML2](https://lkml.org/lkml/2012/6/22/572) |

## 2.3 stack backtrace
-------

在内核态尝试 dump 用户态的栈对调试是非常有帮助的.

[Dumping User and Kernel Stacks on Kernel Events](https://blog.csdn.net/devwang_com/article/details/93633488)

[how to print stack trace in-your-user](http://kungjohan.blogspot.com/2012/06/how-to-print-stack-trace-in-your-user.html)

[how-to-dump-user-space-stack-in-kernel-space](https://stackoverflow.com/questions/38384259/how-to-dump-user-space-stack-in-kernel-space)

[](https://groups.google.com/g/linux.kernel/c/ca3lZGlbnmo)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/06/23 | 胡俊鹏 and <dongzhiyan_linux@163.com> | [dongzhiyan-stack/user_stack_backstrace-in-kernel](https://github.com/dongzhiyan-stack/user_stack_backstrace-in-kernel) | 海康 CLK 2019 的一个 slides, 内核态回溯用户态栈. 对于一些比较难解析符号的场景也有对策 | v1 ☐ | [github](https://github.com/dongzhiyan-stack/user_stack_backstrace-in-kernel) |
| 2012/4/11 | "Tu, Xiaobing" <xiaobing.tu@intel.com> | [kernel patch for dump user space stack tool](https://lkml.org/lkml/2012/4/11/49) | 内核态回溯用户态栈. | v1 ☐ | [LKML RFC 1/2](https://lkml.org/lkml/2012/4/11/49) |
| 2023/05/01 | Indu Bhagat <indu.bhagat@oracle.com> | [SFrame based stack tracer for user space in the kernel](https://lore.kernel.org/all/20230501200410.3973453-1-indu.bhagat@oracle.com) | [Reliable user-space stack traces with SFrame](https://lwn.net/Articles/932209) 以及 [SFrame: fast, low-overhead stack traces](https://lwn.net/Articles/940686). | v1 ☐☑✓ | [LORE v1,0/5](https://lore.kernel.org/all/20230501200410.3973453-1-indu.bhagat@oracle.com) |


## 2.4 patchwork
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/01/10 | Joe Perches <joe@perches.com> | [Documentation: Replace lkml.org links with lore](https://patchwork.kernel.org/project/linux-omap/patch/77cdb7f32cfb087955bfc3600b86c40bed5d4104.camel@perches.com/#23968747) | 补丁将内核中 lkml 的链接转换为 lore.kernel.org 的链接. 补丁描述中提供了一个脚本来完整这个工作. | v1 ☑ 5.12-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=05a5f51ca566674e6a6ee9cef0af1b00bf100d67) |
| 2020/02/06 | Joe Perches <joe@perches.com> | [B4 Better tools for kernel developers](https://lwn.net/Articles/811528) | 补丁将内核中 lkml 的链接转换为 lore.kernel.org 的链接. 补丁描述中提供了一个脚本来完整这个工作. | v1 ☑ 5.12-rc1 | [GIT](https://git.kernel.org/pub/scm/utils/b4/b4.git), [elinux](https://elinux.org/images/9/93/Kernel_email_tools_elce_2020.pdf) |

[Sending a kernel patch with b4 (part 1)](https://people.kernel.org/monsieuricon/sending-a-kernel-patch-with-b4-part-1), Konstantin Ryabitsev 的博客展示如何使用 B4 提交内核补丁, 无需(直接)使用电子邮件.

patchwork 提供了 [API](https://patchwork.kernel.org/api/projects) 可以获取到各个 patchwork 子项目的 ID 等信息.
[xuliker/kde](https://github.com/xuliker/kde) 接着 patchwork 的 API, 提供了一个获取制定日期各个 patchwork 项目上邮件的脚本 [get_mm_patch_series_today.sh](https://github.com/xuliker/kde/blob/master/kernel_mailing_list/mm_community/get_mm_patch_series_today.sh)


## 2.5 静态扫描工具
-------

[Static code checking In the Linux kernel](https://elinux.org/images/d/d3/Bargmann.pdf)


## 2.6 topology 获取工具
-------


RRZE-HPC 的 [likwid(Like I knew what I am doing):Performance monitoring and benchmarking suite](https://github.com/RRZE-HPC/likwid/wiki) 工具集提供了多个工具辅助开发人员获取硬件的信息和性能数据. 虽然 LIKWID 的重点最初是 x86 处理器, 但后期逐渐开始支持 x86、ARM 和 POWERPC 以及 Nvidia 等处理器. 代码仓库地址 [github](https://github.com/RRZE-HPC/likwid).


| 编号 | 应用 | 功能 |
|:---:|:----:|:---:|
| 1 | likwid-topology | 获取 CPU 硬件的拓扑结构. |
| 2 | [likwid-perfCtr](https://blog.csdn.net/u010099177/article/details/108864013) | 测量CPU的硬件性能计数器. |
| 3 | [likwid-powermeter]() | . |
| 4 | [likwid-pin](https://github.com/RRZE-HPC/likwid/wiki/Likwid-Pin) | 开发多线程(pthread_create)以及 OpenMP 并发应用的时候经常希望各个线程绑定在不同的 CPU 上, pin 工具提供了一个灵活的配置来完成这个功能, 而不需要开发者修改代码. |
| 5 | [likwid-bench](https://github.com/RRZE-HPC/likwid/wiki/Likwid-Bench) | 快速创建和运行不同类型的基准测试程序 |
| 6 | [likwid-features](https://github.com/RRZE-HPC/likwid/wiki/likwid-features): 获取和修改 CPU 硬件上的一些特性/功能 |
| 7 | [likwid-mpirun](https://github.com/RRZE-HPC/likwid/wiki/Likwid-Mpirun) | NA |
| 8 | [likwid-powermeter](https://github.com/RRZE-HPC/likwid/wiki/Likwid-Powermeter) |  获取通过 MSR 寄存器读取和配置 RAPL 计数的工具, 可以进行控制配置和读取 Intel 处理器和内存的能耗信息, 查询给定时间段内包内消耗的能量并计算产生的功耗. |
| 9 | [likwid-memsweeper](https://github.com/RRZE-HPC/likwid/wiki/Likwid-Memsweeper) | 清理 ccNUMA/Cache Coherent NUMA 结构中缓存的 cache 以及 last level cache 的数据 |
| 10 | [likwid-setFrequencies](https://github.com/RRZE-HPC/likwid/wiki/likwid-setFrequencies) | 操纵频率的工具 |
| 11 | [likwid-agent](https://github.com/RRZE-HPC/likwid/wiki/likwid-agent) | likwid-agent 是一种守护应用程序, 它使用 likwid-perfctr 来测量硬件性能计数器. |
| 12 | [likwid-genTopoCfg](https://github.com/RRZE-HPC/likwid/wiki/likwid-genTopoCfg) | 获取当前系统的硬件 CPU 拓扑信息. |
| 13 | [likwid-perfscope](https://github.com/RRZE-HPC/likwid/wiki/likwid-perfscope) |使用 gnuplot 执行实时性能数据绘图的工具. |

## 2.7 eBPF
-------

[Kernel analysis with bpftrace](https://lwn.net/Articles/793749)



## 2.8 增加时延
-------


[netem](https://wiki.linuxfoundation.org/networking/netem)


## 2.9 耗时分析
-------


## 2.10 DEBUGINFO
-------

RedHat 一直领导着调试信息方面的生态系统工作, 自 Fedora 35 以来, 它一直默认使用 [debuginfod](https://sourceware.org/elfutils/Debuginfod.html) 来辅助工作, debuginfod 通过 HTTP API 提供调试信息服务, 无需在本地使用所有 debuginfo package, 只需向远程服务器查询所需的 debuginfo 信息即可. 既节省了磁盘空间, 使用起来又相当方便.

Arch Linux 在 2022 年 5 月份开始了也提供了他们的 [debuginfod 调试信息服务器](https://wiki.archlinux.org/title/Debuginfod). 参见 [wiki.archlinux--Debugging/Getting traces](https://wiki.archlinux.org/title/Debugging/Getting_traces)

接着 Ubuntu 22.10 也开始集成 debuginfod, 参见 phoronix 报道 [](https://www.phoronix.com/news/Ubuntu-22.10-Debuginfod)

# 3 安全
-------

## 3.1 Kasper
-------


[Kasper: a tool for finding speculative-execution vulnerabilities](https://lwn.net/Articles/883448)
[Scanning for Generalized Transient Execution Gadgets in the Linux Kernel](https://www.vusec.net/projects/kasper)
[GitHub](https://github.com/vusec/kasper)
[Paper](https://download.vusec.net/papers/kasper_ndss22.pdf)

[phoronix, 2025/09/05, Linux Kernel Runtime Guard 1.0 Released For Security Vulnerability Exploit Detection](https://www.phoronix.com/news/LKRG-1.0-Released)




# 4 运维工具
-------


| 工具 | 主页 | 描述 |
|:---:|:---:|:----:|
| [kmon](https://github.com/orhun/kmon) | NA | 用于管理 Linux 内核模块和监视内核活动的终端工具, 使用 Rust 编写. |
| [Sysinternals/ProcDump-for-Linux](https://github.com/Sysinternals/ProcDump-for-Linux) | [sysinternals](https://www.sysinternals.com) | ProcDump 是 windows 下一个实用维测工具, 其主要用途是监视应用程序的 CPU 峰值, 并在出现峰值期间生成故障转储. 这个为官方 LINUX 移植版本. |



# 5 性能分析工具
-------


[Intel Makes Continuous Profiler Open-Source To Help Improve CPU Performance](https://www.phoronix.com/news/Intel-Continuous-Profiler-OSS)



# 6 测试
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2024/02/28 | Helen Koike <helen.koike@collabora.com> | [kci-gitlab: Introducing GitLab-CI Pipeline for Kernel Testing](https://lore.kernel.org/all/20240228225527.1052240-1-helen.koike@collabora.com) | [GitLab CI for the kernel](https://lwn.net/Articles/972713). | v1 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20240228225527.1052240-1-helen.koike@collabora.com) |




<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
