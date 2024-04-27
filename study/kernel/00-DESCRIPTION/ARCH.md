---

title: ARCH 架构相关
date: 2021-02-15 00:32
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

本作品采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议</a> 进行许可, 转载请注明出处, 谢谢合作

<a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a>

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 鄙人在此谢谢啦

** 转载请务必注明出处, 谢谢, 不胜感激 **

<br>

| 日期 | 作者 | GitHub| CSDN | BLOG |
| ------- |:-------:|:-------:|:-------:|:-------:|
| 2021-02-15 | [成坚 - gatieme](https://kernel.blog.csdn.net) | [`AderXCoding/system/tools/fzf`](https://github.com/gatieme/AderXCoding/tree/master/system/tools/fzf) | [使用模糊搜索神器 FZF 来提升办公体验](https://blog.csdn.net/gatieme/article/details/113828826) | [Using FZF to Improve Productivit](https://oskernellab.com/2021/02/15/2021/0215-0001-Using_FZF_to_Improve_Productivity)|


<br>

2   ** 架构子系统 **
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***


# 1 X86_64
-------

[Intel® 64 and IA-32 Architectures Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

[Intel Architecture Day 2021](https://www.intel.com/content/www/us/en/newsroom/resources/press-kit-architecture-day-2021.html)

## 1.1 架构新特性
-------

### 1.1.1 split lock detect
-------

| 日期 | LWN | 翻译 |
|:---:|:----:|:---:|
| 2021/02/08 | [Detecting and handling split locks](https://lwn.net/Articles/790464) | [LWN：检测 Intel CPU 的 split locks 以及阻止攻击](https://blog.csdn.net/Linux_Everything/article/details/93270786) |
| 2019/12/06 | [Developers split over split-lock detection](https://lwn.net/Articles/806466) | [LWN：开发者争论 split-lock 检测机制！](https://blog.csdn.net/Linux_Everything/article/details/103640683) |


[字节跳动技术团队的博客 -- 深入剖析 split locks, i++ 可能导致的灾难](https://blog.csdn.net/ByteDanceTech/article/details/124701175)

拆分锁是指原子指令对跨越多个高速缓存行的数据进行操作. 由于原子性质, 在两条高速缓存行上工作时需要全局总线锁, 这反过来又会对整体系统性能造成很大的性能影响.

当原子指令跨越多个 cache line, 并且需要确保原子性所需的总线锁时, 就会发生拆分总线锁. 这些拆分锁总线至少比单个 cacheline 内的原子操作多需要 1000 个 cycles. 在锁定总线期间, 其他 CPU 或 BUS 代理要求控制 BUS 的请求被阻止, 阻止其他 CPU 的 BUS 访问, 加上配置总线锁定协议的开销不仅会降低一个 CPU 的性能, 还会降低整体系统性能.

除了拆分锁的性能影响之外, 利用该行为还可能导致无特权的拒绝服务漏洞. 因此现代英特尔 CPU 可以在处理拆分锁时生成对齐检查异常, 使用此技术这组补丁为内核实现了拆分锁检测(split_lock_detect), 以警告或杀死违规应用.

v5.7 引入了拆分锁检测的支持, 这依赖于 x86_64 intel CPU 遇到拆分锁时生成对齐检查异常的硬件特性. 参见 [The Linux Kernel Will Be Able To Detect Split-Locks To Then Warn Or Kill Offending Apps](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Split-Locks-Detection) 以及 [Split Lock Detection Sent In For Linux 5.7 To Spot Performance Issues, Unprivileged DoS](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.7-Split-Lock-Detection).

通过配置 "split_lock_detect=fatal" 内核参数, 可以配置检测出 split_lock 后的处理.  默认行为是警告有问题的用户空间应用程序, 而如果配置了 `split_lock_detect=fatal` 内核参数将发送 SIGBUS 信号杀死该应用程序.

在拆分锁检测代码合并后, 英特尔工程师将重点转向 Linux 的总线锁检测. 同样, 由于性能损失和可能的拒绝服务影响, 这很重要. 总线锁可能会破坏其他 CPU 内核的性能, 并且比缓存行内发生的原子操作慢得多. 与拆分锁检测一样, 总线锁检测依赖于 CPU 能够在用户指令获取总线锁时通知内核的硬件特性 X86_FEATURE_BUS_LOCK_DETECT, 参见 [commit ebb1064e7c2e ("x86/cpufeatures: Enumerate #DB for bus lock detection](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ebb1064e7c2e90b56e4d40ab154ef9796060a1c3). 给定的 CPU 是否支持总线锁定检测将可以通过 `/proc/cpuinfo` 中 bus_lock_detect 标志.

目前主流的缓解措施都是杀死有问题的进程, 但在某些情况下, 需要识别并限制有问题的应用程序. 因此 v5.14 [x86/bus_lock: Set rate limit for bus lock](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d28397eaf4c27947a1ffc720d42e8b3a33ae1e2a) 为总线锁添加系统范围的速率限制. 当系统检测到总线锁速度高于 N / 秒 (其中 N 可以由内核启动参数设置范围 (1...1000)), 就会让任何触发总线锁任务强制睡眠至少 20 ms, 直到整个系统总线锁率低于阈值.

随后 Intel 工程师 Tony Luck 提出来更激进的方案, 试图减慢违规应用程序的速度, 以便开发人员有望解决这个问题. 通过强制用户空间对拆分锁进行顺序访问. 在解决问题的同时, 也确保了在这些条件下整体系统性能更好. 参见 [Linux 5.19 To"Make Life Miserable"In Slowing Down Bad Behaving Split-Lock Apps](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.19-Split-Lock).

| 配置 | 描述 |
|:---:|:----:|
| off | 关闭 split_lock 检测. |
| warn | 默认情况, 当发生拆分锁定和有问题的进程时, Linux 内核将通过 dmesg 发出警告. 参见 handle_user_split_lock(). |
| fatal | 它将向应用程序发送致命的 SIGBUS 信号并杀死进程. |
| ratelimit:N | 限制总线锁的速率在每秒 N 次以内. |

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/01/26 | Luck, Tony <tony.luck@intel.com> | [x86/split_lock: Enable split lock detection by kernel](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6650cdd9a8ccf00555dbbe743d58541ad8feb6a7) | 支持 拆分锁检测(split_lock_detect). | v17 ☑✓ v5.7-rc1| [LORE](https://lore.kernel.org/all/20200126200535.GB30377@agluck-desk2.amr.corp.intel.com) |
| 2020/11/06 | Chenyi Qiang <chenyi.qiang@intel.com> | [Add bus lock VM exit support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=c32b1b896d2ab30ac30bc39194bac47a09f7f497) | 虚拟化支持 bus lock 检测. | v5 ☑✓ 5.12-rc1 | [LORE v5,0/4](https://lore.kernel.org/all/20201106090315.18606-1-chenyi.qiang@intel.com) |
| 2021/03/22 | Fenghua Yu <fenghua.yu@intel.com> | [x86/bus_lock: Enable bus lock detection](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ebca17707e38f2050b188d837bd4646b29a1b0c2) | 拆分锁检测支持 Bus Lock. 参见 [Intel's Bus Lock Detection Might Be Ready For The Mainline Linux Kernel](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Bus-Lock-Detection-2021) | v6 ☑✓ 5.13-rc1 | [LORE v6,0/3](https://lore.kernel.org/all/20210322135325.682257-1-fenghua.yu@intel.com) |
| 2021/04/19 | Fenghua Yu <fenghua.yuintel.com> | [x86/bus_lock: Set rate limit for bus lock](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d28397eaf4c27947a1ffc720d42e8b3a33ae1e2a) | 通过限制总线锁的速率而不是杀死进程来缓解拆分锁带来的问题. | v1 ☑✓ 5.14-rc1 | [Patchwork 0/4](https://lore.kernel.org/all/20210419214958.4035512-1-fenghua.yu@intel.com) |
| 2022/03/10 | Tony Luck <tony.luck@intel.com> | [Make life miserable for split lockers](https://lore.kernel.org/all/20220310204854.31752-1-tony.luck@intel.com) | 通过强制用户空间对拆分锁进行顺序访问. 在解决问题的同时, 也确保了在这些条件下整体系统性能更好. 参见 [Linux 5.19 To"Make Life Miserable"In Slowing Down Bad Behaving Split-Lock Apps](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.19-Split-Lock). | v2 ☐☑✓ | [LORE v2,0/2](https://lore.kernel.org/all/20220310204854.31752-1-tony.luck@intel.com) |
| 2022/10/24 | Guilherme G. Piccoli <gpiccoli@igalia.com> | [x86/split_lock: Add sysctl to control the misery mode](https://lore.kernel.org/all/20221024200254.635256-1-gpiccoli@igalia.com) | Splitlock Detector 机制通过人为限制触发了 Splitlock 锁的软件的性能, 从而保证整机的吞吐量和性能. 然后推动触发了对应软件整改和优化自己的代码, 但是一些专用软件特别是闭源软件改起来相对麻烦, 比如 Linux 上 window 游戏等. 这引发了这些 window 游戏性能的大幅度下降, 参见 [Linux Adding New Control Since Its Splitlock Detector Is Wrecking Some Steam Play Games](https://www.phoronix.com/news/Linux-Splitlock-Hurts-Gaming). 现在为 Linux 6.2 添加了一个新的 sysctl 可调参数 "split_lock_reduce", 以便轻松禁用 "痛苦模式" 行为. 当将 split_lock_reduce 设置为 0 时, 只会在内核锁中出现警告, 而不会带来循序存取损失. 默认情况下, 故意的性能降低模式仍然处于活动状态, 但是我们可能会看到 SteamOS 之类的操作系统自动关闭 split_lock_reduce 可调参数. 也有可能我们会看到这个自动调整喜欢野性的游戏模式. 由于它是一个 sysctl 选项, 现在在运行时管理起来比在启动时不得不为内核使用劈开锁检测选项更容易, 但是内核开发人员到目前为止还想保持默认的 “糟糕性能” 模式, 以激励用户空间软件围绕减少劈开锁进行改进. | v3 ☐☑✓ | [LORE](https://lore.kernel.org/all/20221024200254.635256-1-gpiccoli@igalia.com) |


### 1.1.2 Sub-Page Write Protection
-------

| 2020/01/19 | Yu-cheng Yu <yu-cheng.yu@intel.com> | [Enable Sub-Page Write Protection Support](https://lwn.net/Articles/810033) | 基于 EPT 的子页写保护 (SPP) 允许虚拟机监视器 (VMM) 以子页 (128 字节) 粒度为客户物理内存指定写权限. 当 SPP 工作时, 硬件强制对受保护的 4KB 页面中的子页面进行写访问检查. 该特性的目标是为内存保护和虚拟机内省等使用提供细粒度的内存保护. 当 "子页面写保护"(第 23 位) 在 Secondary VM-Execution Controls 中为 1 时, SPP 被启用. 该特性支持子页权限表 (SPPT), 子页权限向量存储在 SPPT 的叶条目中. 根页面是通过 VMCS 中的子页面权限表指针(SPPTP) 引用的.<br> 要为 guest 内存启用 SPP, guest 页面应该首先映射到一个 4KB 的 EPT 条目, 然后设置相应条目的 SPP 的 61bit. 当硬件遍历 EPT 时, 它使用 gpa 遍历 SPPT 以查找 SPPT 叶子条目中的子页面权限向量. 如果设置了对应位, 则允许写子页, 否则产生 SPP 触发的 EPT 冲突. | v30 ☐ | [Patchwork v30,00/32](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) |


## 1.2 指令加速
-------

### 1.2.1 向量运算
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/07/30 | "Chang S. Bae" <chang.seok.bae@intel.com> | [x86: Support Intel Advanced Matrix Extensions](https://lore.kernel.org/lkml/20210730145957.7927-1-chang.seok.bae@intel.com) | 支持即将发布的英特尔 [高级矩阵扩展(AMX)](https://software.intel.com/content/dam/develop/external/us/en/documents-tps/architecture-instruction-set-extensions-programming-reference.pdf), [AMX](https://software.intel.com/content/www/us/en/develop/documentation/cpp-compiler-developer-guide-and-reference/top/compiler-reference/intrinsics/intrinsics-for-intel-advanced-matrix-extensions-intel-amx-instructions.html) 由可配置的 TMM"TILE" 寄存器和操作它们的新 CPU 指令组成. TMUL (Tile matrix MULtiply) 是第一个利用新寄存器的运算符, 我们预计将来会有更多的指令. | v9 ☐ | [Patchwork v9,00/26](https://lore.kernel.org/lkml/20210730145957.7927-1-chang.seok.bae@intel.com) |
| 2021/07/20 | Tianjia Zhang <tianjia.zhang@linux.alibaba.com> | [Introduce x86 assembler accelerated implementation for SM4 algorithm](https://lwn.net/Articles/863574/) | 该补丁集提取了公有 SM4 算法作为一个单独的库, 同时对 arm64 中 SM4 的加速实现进行了调整, 以适应该 SM4 库. 然后在 x86_64 上的进行了实现. 该优化支持 SM4、ECB、CBC、CFB 和 CTR 四种模式. 由于 CBC 和 CFB 不支持多块并行加密, 优化效果不明显. 主要算法实现来自 libgcrypt 和 Markku-Juhani O. Saarinen 的 [SM4 AES-NI 工作](https://github.com/mjosaarinen/sm4ni). | v1 ☐ | [LWN 0/2](https://lwn.net/Articles/863574/) |
| 2021/08/12 | Tianjia Zhang <tianjia.zhang@linux.alibaba.com> | [support test GCM/CCM mode for SM4](https://lore.kernel.org/lkml/20210812131748.81620-1-tianjia.zhang@linux.alibaba.com) | NA | v1 ☐ | [Patchwork 0/3](https://lore.kernel.org/lkml/20210812131748.81620-1-tianjia.zhang@linux.alibaba.com), [LKML](https://lkml.org/lkml/2021/8/13/161) |
| 2021/08/18 | Tianjia Zhang <tianjia.zhang@linux.alibaba.com> | [add AES-NI/AVX2/x86_64 implementation](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) | 这个补丁集导出了 SM4 AESNI/AVX 算法实现的一些常用函数, 并用这些函数实现 AESNI/AVX2 的加速.<br> 主要算法实现来自 libgcrypt 和 [Markku Juhani 的 SM4 AES-NI 工作](https://github.com/mjosaarinen/sm4ni). | v1 ☐ | [Patchwork 0/2](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) |

### 1.2.2 Flexible Return and Event Delivery (FRED)
-------

[Linux 6.3 To Support Making Use Of Intel's New LKGS Instruction (Part Of FRED)](https://www.phoronix.com/news/Intel-LKGS-Linux-6.3)

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/10/06 | Xin Li <xin3.li@intel.com> | [Enable LKGS instruction](https://lore.kernel.org/all/20221006154041.13001-1-xin3.li@intel.com) | [Intel Preps The Linux Kernel For LKGS - Part Of FRED](https://www.phoronix.com/news/Intel-LKGS-Linux-Patches) | v1 ☐☑✓ | [LORE v1,0/6](https://lore.kernel.org/all/20221006154041.13001-1-xin3.li@intel.com) |
| 2022/12/19 | Xin Li <xin3.li@intel.com> | [x86: enable FRED for x86-64](https://lore.kernel.org/all/20221220063658.19271-1-xin3.li@intel.com) | [Intel Sends Out Initial Linux Kernel Patches For FRED](https://www.phoronix.com/news/Intel-FRED-Linux-Patches) 和 [Intel FRED Merged For Linux 6.9 As An Important Improvement With Future CPUs](https://www.phoronix.com/news/Intel-FRED-Merged-Linux-6.9). | v1 ☐☑✓ | [LORE v1,0/32](https://lore.kernel.org/all/20221220063658.19271-1-xin3.li@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v13,0/35](https://lore.kernel.org/all/20231205105030.8698-1-xin3.li@intel.com) |



## 1.3 架构上新想法
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/04/19 | Fenghua Yu <fenghua.yu@intel.com> | [x86/bus_lock: Set rate limit for bus lock](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Bus-Lock-Detection-2021) | 当原子指令跨越多个 cache line, 并且需要确保原子性所需的总线锁时, 就会发生拆分锁. 这些拆分锁至少比单个 cacheline 内的原子操作多需要 1000 个 cycles. 在锁定总线期间, 其他 CPU 或 BUS 代理要求控制 BUS 的请求被阻止, 阻止其他 CPU 的 BUS 访问, 加上配置总线锁定协议的开销不仅会降低一个 CPU 的性能, 还会降低整体系统性能. 除了拆分锁的性能影响之外, 利用该行为还可能导致无特权的拒绝服务漏洞. 现代英特尔 CPU 可以在处理拆分锁时生成对齐检查异常, 使用此技术这组补丁为内核实现了拆分锁检测(split_lock_detect), 以警告或杀死违规应用. 默认行为是警告有问题的用户空间应用程序, 而如果配置了 `split_lock_detect=fatal` 内核参数将杀死使用 SIGBUS 的应用程序. 参考 [phoronix1](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.7-Split-Lock-Detection), [phoronix2](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Split-Locks-Detection), [phoronix3](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Bus-Lock-Detection-2021) | v1 ☐ | [Patchwork 0/4](https://lore.kernel.org/all/20210419214958.4035512-1-fenghua.yu@intel.com) |
| 2021/09/13 | Sohil Mehta <sohil.mehta@intel.com> | [x86 User Interrupts support](https://lwn.net/Articles/871113) | 用户中断 (Uintr) 是一种硬件技术, 可以将中断直接传递到用户空间.<br> 如今, 几乎所有跨越特权边界的通信都是通过内核进行的. 这些包括信号、管道、远程过程调用和基于硬件中断的通知. 用户中断通过避免通过内核的转换, 为这些常见操作的更高效 (低延迟和低 CPU 利用率) 版本提供了基础. 在用户中断硬件体系结构中, 接收者总是期望是用户空间任务. 但是, 用户中断可以由另一个用户空间任务、内核或外部源 (如设备) 发送. 除了接收用户中断的一般基础结构之外, 这组补丁还介绍了另一个用户任务的中断(用户 IPI). 用户 IPI 的第一个实现将在代号为 Sapphire Rapids 的英特尔处理器中实现. 有关硬件架构的详细信息, 请参阅[英特尔架构指令集扩展的第 11 章](https://software.intel.com/content/www/us/en/develop/download/intel-architecture-instruction-set-extensions-programming-reference.html). | v1 ☐ | [LWN RFC,00/13](https://lwn.net/ml/linux-kernel/20210913200132.3396598-1-sohil.mehta@intel.com)<br>*-*-*-*-*-*-*-* <br>[LWN](https://lwn.net/Articles/869140)<br>*-*-*-*-*-*-*-* <br>[Patchwork RFC,00/13](https://lore.kernel.org/lkml/20210913200132.3396598-1-sohil.mehta@intel.com) |
| 2021/10/04 | Tvrtko Ursulin <tvrtko.ursulin@linux.intel.com> | [CPU + GPU synchronised priority scheduling](https://www.phoronix.com/scan.php?page=news_item&px=Intel-CPU-GPU-Prio-Nice-Sched) | Intel 在 CPU 和 GPU 协同调度领域进行了探索与尝试. | v1 ☐ | [Patchwork RFC,0/6](https://lists.freedesktop.org/archives/intel-gfx/2021-September/279200.html), [LWN](https://lwn.net/Articles/871467), [LKML](https://lkml.org/lkml/2021/9/30/852)<br>*-*-*-*-*-*-*-* <br>[LKML v2,0/8](https://lkml.org/lkml/2021/10/4/1004), [LWN news](https://lwn.net/Articles/873334) |
| 2023/05/15 | Muralidhara M K <muralimk@amd.com> | [AMD64 EDAC GPU Updates](https://lore.kernel.org/all/20230515113537.1052146-1-muralimk@amd.com) | [More AMD Heterogeneous System Patches Queued Ahead Of Linux 6.5](https://www.phoronix.com/news/AMD-Heterogeneous-Linux-6.5). | v1 ☐☑✓ | [LORE v1,0/5](https://lore.kernel.org/all/20230515113537.1052146-1-muralimk@amd.com) |



## 1.4 hybrid CPUs
-------

[该感谢 AMD 还是 ARM？：Intel Alder Lake 为什么要搞大小核？](https://zhuanlan.zhihu.com/p/350097861)

### 1.4.1 Intel hybrid CPUs
-------

WikiChip 上关于 AlderLake 的介绍 [Alder Lake - Microarchitectures - Intel](https://en.wikichip.org/wiki/intel/microarchitectures/alder_lake).

phoronix 上所有与 [Alder Lake 相关的报道](https://www.phoronix.com/scan.php?page=search&q=Alder%20Lake).

#### 1.4.1.1 架构支持
-------

Intel Architecture Day 2021, 官宣了自己的服务于终端和桌面场景的异构 (或者混合架构) 处理器架构 [Alder Lake](https://www.anandtech.com/show/16881/a-deep-dive-into-intels-alder-lake-microarchitectures), 与 ARM 的 big.LITTLE 以及 DynamIQ 架构类似, 它包含了基于 Golden Cove 微架构的性能核 (P-core/Performance cores) 以及基于新的 Gracemont 架构的能效核(E-core/Efficiency cores). P-core 优先用于需要低延迟的单线程任务, 而 E-core 在功率有限或多线程情景方面更好.

[What Is Performance Hybrid Architecture?](https://www.intel.com/content/www/us/en/support/articles/000091896/processors.html)

[Heterogeneous_computing](https://en.wikipedia.org/wiki/Heterogeneous_computing)

[Processing Architecture for Power Efficiency and Performance](https://www.arm.com/technologies/big-little)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/10/02 | Catalin Marinas <catalin.marinas@arm.com> | [x86: Add initial support to discover Intel hybrid CPUs](https://lore.kernel.org/lkml/20201002201931.2826-1-ricardo.neri-calderon@linux.intel.com) | 支持混合微架构的 CPU(Alder Lake CPU) | v3 ☐ | [Patchwork 0/3](https://lore.kernel.org/lkml/20201002201931.2826-1-ricardo.neri-calderon@linux.intel.com) |
| 2021/02/08  | Kan Liang <kan.liang@linux.intel.com> | [Add Alder Lake support for perf](https://lkml.org/lkml/2021/2/8/1142) | perf 支持 Hybrid CPU.  | v1 ☑ 5.14-rc1 | [LKML 00/49](https://lkml.org/lkml/2021/2/8/1142) |
| 2021/04/05  | Kan Liang <kan.liang@linux.intel.com> | [Add Alder Lake support for perf (kernel)](https://lkml.org/lkml/2021/4/5/775) | perf 支持 Hybrid CPU(内核态). | v1 ☑ 5.13-rc1 | [LKML V5 00/25](https://lkml.org/lkml/2021/4/5/775), [LKML V3 00/25](https://lkml.org/lkml/2021/3/26/964) |
| 2021/04/23  | Kan Liang <kan.liang@linux.intel.com> | [perf tool: AlderLake hybrid support series 1](https://lkml.org/lkml/2021/4/23/52) | perf 支持 Hybrid CPU(内核态). | v1 ☑ 5.13-rc1 | [LKML v5 00/26](https://lkml.org/lkml/2021/4/23/52) |
| 2021/05/27  | Kan Liang <kan.liang@linux.intel.com> | [perf: Support perf-mem/perf-c2c for AlderLake](https://lkml.org/lkml/2021/4/5/775) | perf 支持 Hybrid CPU(内核态). | v2 ☑ 5.14-rc1 | [LKML v1 0/8](https://lkml.org/lkml/2021/4/5/775), [LKML v2 0/8](https://lkml.org/lkml/2021/5/27/191) |
| 2023/02/13 | Zhao Liu <zhao1.liu@linux.intel.com> | [Introduce hybrid CPU topology](https://lore.kernel.org/all/20230213095035.158240-1-zhao1.liu@linux.intel.com) | 在 QEMU 中引入混合拓扑, 依赖于 QEMU 支持 [Support smp.clusters for x86](https://lists.gnu.org/archive/html/qemu-devel/2023-02/msg03184.html) | v1 ☐☑✓ | [LORE v1,0/52](https://lore.kernel.org/all/20230213095035.158240-1-zhao1.liu@linux.intel.com) |
| 2023/11/30 | Zhao Liu <zhao1.liu@linux.intel.com> | [qom-topo: Abstract Everything about CPU Topology](https://lore.kernel.org/all/20231130144203.2307629-1-zhao1.liu@linux.intel.com) | 关于混合拓扑支持 [Introduce hybrid CPU topology](https://lore.kernel.org/all/20230213095035.158240-1-zhao1.liu@linux.intel.com) 之后的最新尝试, 使用现代 QOM 方法来定义 CPU 拓扑, 基于这种方式, 通过 cli 定义混合拓扑. 列出了详细的 Reference. | v1 ☐☑✓ | [LORE v1,0/41](https://lore.kernel.org/all/20231130144203.2307629-1-zhao1.liu@linux.intel.com) |


#### 1.4.1.2 P-State(HWP)
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/05/12  | Rafael J. Wysocki <rafael.j.wysocki@intel.com> | [cpufreq: intel_pstate: hybrid: CPU-specific scaling factors](https://www.phoronix.com/scan.php?page=news_item&px=P-State-Preps-For-Hybrid) | Hybrid CPU 的 P-state 增强 | v1 ☑ 5.14-rc1 | [Patchwork 0/3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=eb3693f0521e020dd8617c7fa3ddf5c9f0d8dea0) |
| 2021/11/19 | Srinivas Pandruvada <srinivas.pandruvada@linux.intel.com> | [cpufreq: intel_pstate: ITMT support for overclocked system](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Patch-ITMT-OC-ADL) | Intel ITMT (Intel Turbo Boost Max Technology) 感知混合架构, Alder Lake CPU 上 P-Core/E-core 优先级应该有不同的值(P-core 0x40, P-core HT sibling 0x10, E-core 0x26). | v1 ☑ 5.16-rc3 | [Patchwork](https://patchwork.kernel.org/project/linux-pm/patch/20211119051801.1432724-1-srinivas.pandruvada@linux.intel.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=03c83982a0278207709143ba78c5a470179febee) |
| 2021/12/16 | Srinivas Pandruvada <srinivas.pandruvada@linux.intel.com> | [cpufreq: intel_pstate: Update EPP for AlderLake mobile](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-P-State-ADL-Mobile) | 修正 AlderLake 的 EPP. | v1 ☑ 5.17-rc1 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b6e6f8beec98ba7541213c506fe908517fdc52b8) |
| 2022/04/15 | Zhang Rui <rui.zhang@intel.com> | [intel_idle: add AlderLake support](https://lore.kernel.org/all/20220415093951.2677170-1-rui.zhang@intel.com) | 参见 [phoronix 报道](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Idle-Alder-Lake) | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20220415093951.2677170-1-rui.zhang@intel.com) |
| 2022/10/24 | Rafael J. Wysocki <rjw@rjwysocki.net> | [cpufreq: intel_pstate: Make HWP calibration work on all hybrid platforms](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f5c8cf2a4992dd929fa0c2f25c09ee69b8dcbce1) | 修复硬件 P-State(HWP) 校准程序在 Intel 混合 CPU 平台的一些问题.<br>1. 以前进行这项工作的尝试是基于使用 CPPC, 但事实证明, CPPC 信息对于此目的来说不够可靠, 唯一的方法是对 P 核和 E 核使用硬编码的比例因子(幸运的是, 这与非混合情况下相同). 幸运的是, 迄今为止, P-core 的相同缩放因子适用于所有混合平台. 这个补丁集的第一个补丁通过避免在一个 CPU 上读取的 MSR 值将用于另一个 CPU 的性能扩展的情况, 确保所有 CPU 都将使用来自 MSR 的正确信息. 第二个补丁完善了 hybrid_get_cpu_scaling() 的实现, 用已知的缩放因子作为 cpu->pstate.scaling. | v1 ☑✓ 6.1-rc3 | [LORE v1,0/2](https://lore.kernel.org/all/2258064.ElGaqSPkdT@kreacher) |



#### 1.4.1.3 ITMT SMT migration Improvement
-------

ASYM_PACKING 用于平衡物理核心与 SMT 之间的负载均衡处理 (例如, 支持 Intel ITMT 3.0 和混合处理器的英特尔处理器) 以及物理核心的 SMT 兄弟 (例如, Power7). 这项机制对于后者来说工作地不错, 但是对于前者的支持, 不慎友好, 特别是在混合了高性能的 P-core 以及高能效的 E-core 的混合处理器(比如 Alder Lake) 上, 这引发了 CPU 之间不必要甚至是错误的迁移.

自 v4.10 开始, 支持 ITMT 的 Intel 处理器使用 ASYM_PACKING 将更高的优先级分配给可以 Boost 的 CPU. 它通过将较低的优先级分配给编号较高的 SMT 兄弟节点, 以确保它们最后使用.

* 错误的优先级标记, 导致错误的迁移.

首先发现 ITMT 标记 HT/SMT CPU 优先级的算法存在问题, 它使得调度程序更喜欢 P-core 而不是 E-cores. 调度器的本意是在可能的情况下将进程放在更快更强劲的 P-core 上的效果. 但是事实情况是: 如果 CPU 的一个或多个 SMT 兄弟 CPU 都很繁忙, 那么 CPU 的吞吐量就会降低. 因此, 完全空闲的低优先级 CPU 比拥有繁忙 SMT 兄弟节点的高优先级 CPU 更受欢迎. 对于 Alder Lake 等混合架构的 CPU, P-core 支持 SMT, E-core 不支持 SMT. 这样 CPU 的选择顺序应该倾向于 P-core(ST) > E-core > p-core(HT/SMT), 即调度器应该先尝试 P-core, 其次是 E-core, 最后才是 P-core/E-core 的 SMT 兄弟 CPU, 但是调度器并没有意识到这点, 它也会在 P-core 不满足要求时, 优先加载了 P-core 同级的 SMT 兄弟 CPU, 而不是尝试 E-cores. 从而导致整体性能的下降.

因此 v5.16 [commit 4006a72bdd93 ("sched/fair: Fix load balancing of SMT siblings with ASYM_PACKING")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=4006a72bdd93b1ffedc2bd8646dee18c822a2c26) 修复. 其解决方案是修正 SMT 兄弟 CPU 的优先级分配, 使得 P-core(SMT) 比 E-core 的优先级更低.

1. [commit 183b8ec38f1e ("x86/sched: Decrease further the priorities of SMT siblings")"](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=183b8ec38f1ec6c1f8419375303bf1d09a2b8369) 修改了 ITMT 下 sched_core_priority 中 smt_prio 的计算方式, HT 的优先级永远比 ST 的 core 要低, 从而保证 P-core 的 HT 优先级比 E-core 要低. 这样负载平衡器将选择高优先级的 P-core (Intel Core) 而不是中优先级的 E-core (Intel Atom), 最后才将负载溢出到低优先级的 SMT 同级 CPU.

2. [commit 4006a72bdd93 ("sched/fair: Consider SMT in ASYM_PACKING load balance")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4006a72bdd93b1ffedc2bd8646dee18c822a2c26) 当决定在 ASYM_PACKING 中提取任务时, 不仅需要检查 dst CPU 的空闲状态, 还需要检查其同级 SMT CPU 的空闲状态. 如果 dst CPU 处于空闲状态, 但其同级 SMT CPU 处于繁忙状态, 则如果将任务从没有 SMT 中等优先级 CPU(比如 AderLake 的 E-core)中 PULL 过来, 性能必然会受到影响. 实现 [asym_smt_can_pull_tasks()](https://elixir.bootlin.com/linux/v5.16/source/kernel/sched/fair.c#L8492) 以检查候选最忙组中 dst CPU 和 CPU 的同级 SMT 的状态.

2. 不感知 SMT 兄弟 CPU 的状态, 导致不必要的迁移.

但是测试发现, v5.16 优先级的修复, 只是一定程度缓解了问题, 修正了 HT 和 SMT CPU 的次序, 优先 P-core(HT) -=> E-core(HT, 不支持 SMT) -=> P-core(SMT). 但是 E-core 和 P-core(SMT) 在判断是否进行迁移时, 并不感知 SMT CORE 上其他兄弟 CPU 实际的工作状态. 系统中依旧存在异常的进程迁移. 这就造成混合 CPU 系统上(由高频 SMT 内核和低频非 SMT 内核混合而成的处理器), 根据旧代码, 如果有多个 SMT 同级繁忙, 则低优先级 CPU 会从高优先级内核中提取任务, 从而导致许多不必要的任务迁移.

现在 ASYM_PACKING 的实现, x86 初始化 ITMT 时通过 [sched_set_itmt_core_prio()](https://elixir.bootlin.com/linux/v6.0/source/arch/x86/kernel/itmt.c#L189) 为编号较高的 SMT 兄弟节点分配较低的优先级 [arch_asym_cpu_priority()](https://elixir.bootlin.com/linux/v6.0/source/arch/x86/kernel/itmt.c#L199). 但是实际上, CPU Core 的任何 SMT 兄弟之间没有区别.

因此其实为每个 SMT 兄弟分配不同的优先级是非常不合理的. 相反, 应该调整 ASYM_PACKING 的负载均衡逻辑, 标记出 SMT 兄弟的状态, 如果有多个繁忙兄弟的 SMT CPU, 则[低优先级 CPU 的 E-core 将积极地从高优先级的 P-core 中提取任务](https://lore.kernel.org/lkml/20220825225529.26465-4-ricardo.neri-calderon@linux.intel.com). 随后 Ricardo Neri 向社区发送了修复方案 [sched/fair: Avoid unnecessary migrations within SMT domains v1,0/4](https://lore.kernel.org/all/20220825225529.26465-1-ricardo.neri-calderon@linux.intel.com).

在 Peter 的建议下, [v2, 0/4](https://lore.kernel.org/all/20220825225529.26465-1-ricardo.neri-calderon@linux.intel.com) 采用了开始跟踪 SMT CPU 的状态, 通过调整 sym_pack 负载均衡逻辑, arch_asym_cpu_priority() 中[通过 sched_smt_siblings_idle() 考虑 CPU 的 SMT 兄弟节点的空闲状态](https://lore.kernel.org/lkml/20221122203532.15013-8-ricardo.neri-calderon@linux.intel.com). 参见 phoronix 报道 [Intel Posts Reworked Linux Patches To Improve Hybrid CPU + HT/SMT Kernel Behavior](https://www.phoronix.com/news/Intel-SMT-Hybrid-Avoid-Migrate).

不再对 SMT 的兄弟 CPU [标记不同的优先级](https://lore.kernel.org/lkml/20221122203532.15013-8-ricardo.neri-calderon@linux.intel.com), 也不再通过 ASYM_PACKING 指导 SMT 之间的负载均衡, 这可以避免多余的迁移.

通过 [find_busiest_group()](https://lore.kernel.org/lkml/20221122203532.15013-2-ricardo.neri-calderon@linux.intel.com) 让低优先级的核检查所有 SMT 兄弟节点以找到最繁忙的队列. 这对于支持 Intel Thread Director 的 IPC Classes 也是必需的, 因为目标 CPU 将需要检查在相同优先级 CPU 上运行的任务.


这样通过改进负载均衡器, 使其能够识别具有多个繁忙同级的 SMT 内核, 并允许优先级较低的 CPU 提取任务, 这只进行必要的迁移, 避免了多余的迁移, 并允许低优先级内核检查所有 SMT 同级中最繁忙的队列. 当然这组补丁集不会影响原来 Power7 SMT8 的 ASYM_PACKING 逻辑. 对于没有实现 sched_ferences_asym() 的 新 check_smt 参数的架构, 功能不会改变.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/09/10 | Ricardo Neri <ricardo.neri-calderon@linux.intel.com> | [sched/fair: Fix load balancing of SMT siblings with ASYM_PACKING](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=4006a72bdd93b1ffedc2bd8646dee18c822a2c26) | 参见 [Fixing a corner case in asymmetric CPU packing](https://lwn.net/Articles/880367), 在使用非对称封装 (ASM_PACKING) 时, 可能存在具有三个优先级的 CPU 拓扑, 其中只有物理核心的子集支持 SMT. 这种架构下 ASM_PACKING 和 SMT 以及 load_balance 都存在冲突.<br> 这种拓扑的一个实例是 Intel Alder Lake. 在 Alder Lake 上, 应该通过首先选择 Core(酷睿) cpu, 然后选择 Atoms, 最后再选择 Core 的 SMT 兄弟 cpu 来分散工作. 然而, 当前负载均衡器的行为与使用 ASYM_PACKING 时描述的不一致. 负载平衡器将选择高优先级的 CPU (Intel Core) 而不是中优先级的 CPU (Intel Atom), 然后将负载溢出到低优先级的 SMT 同级 CPU. 这使得中等优先级的 Atoms cpu 空闲, 而低优先级的 cpu sibling 繁忙.<br>1. 首先改善了 SMT 中 sibling cpu 优先级的计算方式, 它将比单个 core 优先级更低.<br>2. 当决定目标 CPU 是否可以从最繁忙的 CPU 提取任务时, 还检查执行负载平衡的 CPU 和最繁忙的候选组的 SMT 同级 CPU 的空闲状态. | v5 ☑ 5.16-rc1 | [PatchWork v1](https://lore.kernel.org/patchwork/cover/1408312)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/cover/1413015)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3 0/6](https://lore.kernel.org/patchwork/cover/1428441)<br>*-*-*-*-*-*-*-* <br>[PatchWork v4,0/6](https://lore.kernel.org/patchwork/cover/1474500)<br>*-*-*-*-*-*-*-* <br>[LKML v5,0/6](https://lkml.org/lkml/2021/9/10/913), [LORE v5,0/6](https://lore.kernel.org/all/20210911011819.12184-1-ricardo.neri-calderon@linux.intel.com) |
| 2022/08/25 | Ricardo Neri <ricardo.neri-calderon@linux.intel.com> | [sched/fair: Avoid unnecessary migrations within SMT domains](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=046a5a95c3b0425cfe79e43021d8ee90c1c4f8c9) | 参见 phoronix 报道 [Linux 6.5 To Boast Improved Handling For Intel Hybrid CPUs With Hyper Threading](https://www.phoronix.com/news/Linux-6.4-Avoid-Unnecessary-SMT). | v1 ☐☑✓ | [2022/08/25 LORE v1,0/4](https://lore.kernel.org/all/20220825225529.26465-1-ricardo.neri-calderon@linux.intel.com)<br>*-*-*-*-*-*-*-* <br>[2022/11/22 LORE v2,0/7](https://lore.kernel.org/lkml/20221122203532.15013-1-ricardo.neri-calderon@linux.intel.com)<br>*-*-*-*-*-*-*-* <br>[2023/02/07 LORE v3,0/10](https://lore.kernel.org/all/20230207045838.11243-1-ricardo.neri-calderon@linux.intel.com) |


#### 1.4.1.3 Intel Thread Director (ITD)
-------

[Intel 支持混合内核架构的硬件线程调度器是怎么工作的？](https://mp.weixin.qq.com/s/9Zl-h61hm0kDhq68bsBEJQ)

[Thread Director](https://www.anandtech.com/show/16881/a-deep-dive-into-intels-alder-lake-microarchitectures/2) 其实是一个软硬协同优化的范畴.

早在 2018 年, 苹果就发布过自家 AMP 硬件上的软硬协同优化的调度器, 中文专利号 [CN108984282A 具有闭环性能控制器的 AMP 体系结构的调度器 / Scheduler for AMP architecture with closed loop performance controller](https://www.patentguru.com/cn/CN108984282A).

首先 v5.18, Intel 先完成了对 HFI 硬件的支持. [Intel Hardware Feedback Interface"HFI"Driver Submitted For Linux 5.18](https://www.phoronix.com/news/Intel-HFI-Thermal-Linux-5.18).

随后 Intel 发布了 Linux 上 Thread-Driector 的支持补丁. [Intel Posts Big Linux Patch Set For"Classes of Tasks"On Hybrid CPUs, Thread Director](https://www.phoronix.com/news/Intel-Linux-Classes-Of-Tasks-TD). 并随后在 LPC-2022 做了主题为 [Bringing Energy-Aware Scheduling to x86](https://lpc.events/event/16/contributions/1275) 的演示. phoronix 随即进行了报道 [Intel Working On Energy Aware Scheduling For x86 Hybrid CPUs](https://www.phoronix.com/news/Intel-x86-EAS-To-Come). 随后 LWN 对此进行了讨论 [Hybrid scheduling gets more complicated](https://lwn.net/Articles/909611).

随后发布了 v2, 参见 phoronix 报道 [Intel Advances Linux"IPC Classes"Design To Improve Load Balancing For Hybrid CPUs](https://www.phoronix.com/news/Intel-IPC-Classes-Post-RFC).


*       混合拓扑结构支持

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/06 | Ricardo Neri <ricardo.neri-calderon-AT-linux.intel.com> | [Thermal: Introduce the Hardware Feedback Interface for thermal and performance management](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=bd30cdfd9bd73b68e4977ce7c5540aa7b14c25cd) | 支持 Intel HFI.<br> 英特尔硬件反馈接口 (HFI) 提供系统中每个 CPU 的性能(performance) 和能效 (Energy efficiency) 的信息. 它使用一个在硬件和操作系统之间共享的表. 该表的内容可能由于系统运行条件的变化 (如达到热极限) 或外部因素的作用 (如热设计功率的变化) 而更新.<br>HFI 提供的信息被指定为相对于系统中其他 cpu 的数字、单元较少的能力. 这些功能的范围为 [0-255], 其中更高的数字表示更高的功能. 如果 CPU 的性能效率或能量能力效率为 0, 硬件建议分别出于性能、能量效率或热原因, 不要在该 CPU 上调度任何任务.<br> 内核或用户空间可以使用来自 HFI 的信息来修改任务放置或调整功率限制. 当前这个补丁集中于用户空间. 热通知框架 (thermal notification framework) 被扩展以支持 CPU capacity 的更新. | v1 ☑ 5.18-rc1 | [2021/11/06 LWN](https://lwn.net/Articles/875296)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/7](https://lore.kernel.org/lkml/20211220151438.1196-1-ricardo.neri-calderon@linux.intel.com), [phoronix v2](https://www.phoronix.com/scan.php?page=news_item&px=Intel-HFI-Linux-v2-2021)<br>*-*-*-*-*-*-*-* <br>[PatchWork v5,0/7](https://patchwork.kernel.org/project/linux-pm/cover/20220127193454.12814-1-ricardo.neri-calderon@linux.intel.com), [phoronix v5](https://www.phoronix.com/scan.php?page=news_item&px=Intel-HFI-For-Linux-5.18) |
| 2022/09/09 | Ricardo Neri <ricardo.neri-calderon@linux.intel.com> | [sched: Introduce classes of tasks for load balance](https://lore.kernel.org/all/20220909231205.14009-1-ricardo.neri-calderon@linux.intel.com) | 实现 Thread-Director, 根据应用程序的类型 classes 实现选核和负载均衡. | v1 ☐☑✓ | [LORE v1,00/23](https://lore.kernel.org/all/20220909231205.14009-1-ricardo.neri-calderon@linux.intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,00/22](https://lore.kernel.org/all/20221128132100.30253-1-ricardo.neri-calderon@linux.intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/24](https://lore.kernel.org/all/20230207051105.11575-1-ricardo.neri-calderon@linux.intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,00/24](https://lore.kernel.org/all/20230613042422.5344-1-ricardo.neri-calderon@linux.intel.com) |
| 2024/01/31 | Stanislaw Gruszka <stanislaw.gruszka@linux.intel.com> | [thermal/netlink/intel_hfi: Enable HFI feature only when required](https://lore.kernel.org/all/20240131120535.933424-1-stanislaw.gruszka@linux.intel.com) | 该补丁集仅在有用户空间实体侦听热网络链接事件时才启用 HFI. 引入了一个 netlink 通知, 它与 netlink_has_listners()检查一起, 允许驱动程序根据实际用户空间消费者的存在发送 netlink 多播事件. 此功能通过允许在不需要时禁用功能来优化资源使用. 然后在 intel_hif 驱动程序中实现通知机制, 用于动态禁用硬件反馈接口 (HFI). 通过实现 netlink 通知回调, 驱动程序现在可以根据实际需求启用或禁用 HFI, 特别是当用户空间应用程序(如英特尔速度选择或英特尔低功耗守护进程) 利用与性能和能效功能相关的事件时. 在有 Intel HFI 但没有安装用户空间组件的机器上, 我们可以节省大量的 CPU 周期. | v1 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20240131120535.933424-1-stanislaw.gruszka@linux.intel.com) |
| 2024/02/02 | Ricardo Neri <ricardo.neri-calderon@linux.intel.com> | [thermal: intel: hfi: Prework for the virtualization of HFI](https://lore.kernel.org/all/20240203040515.23947-1-ricardo.neri-calderon@linux.intel.com) | 用于 [支持 IPC 类任务的调度](https://lore.kernel.org/all/20230613042422.5344-1-ricardo.neri-calderon@linux.intel.com) 的基础 <br>1. 重新组织 HFI 驱动程序的部分, 以便于实现虚拟 HFI 表.<br>2. 引入了 ITD 类的概念并启用了 ITD.<br>3. 增加了对重置当前任务的 ITD 分类历史的支持, 以便在上下文切换期间使用. HFI 的虚拟化要求在系统中无条件启用 HFI 和 ITD. 这与 Stanislaw 的补丁集 [thermal/netlink/intel_hfi: Enable HFI feature only when required](https://lore.kernel.org/all/20240131120535.933424-1-stanislaw.gruszka@linux.intel.com) 存在冲突. | v1 ☐☑✓ | [LORE v1,0/9](https://lore.kernel.org/all/20240203040515.23947-1-ricardo.neri-calderon@linux.intel.com) |
| 2024/02/03 | Zhao Liu <zhao1.liu@linux.intel.com> | [Intel Thread Director Virtualization](https://lore.kernel.org/all/20240203091214.411862-1-zhao1.liu@linux.intel.com) | 以虚拟化硬件反馈接口 (HFI) 和英特尔线程控制器(ITD), 从而为使用 ITD 进行调度的虚拟机带来好处. 他的实验表明, 在某些工作负载和配置中, 性能提高了 14%. [Intel Thread Director Virtualization Patches Boost Some Workloads By ~14%](https://www.phoronix.com/news/Intel-Thread-Director-Virt) | v1 ☐☑✓ | [LORE v1,0/26](https://lore.kernel.org/all/20240203091214.411862-1-zhao1.liu@linux.intel.com)|
| 2024/02/23 | Stanislaw Gruszka <stanislaw.gruszka@linux.intel.com> | [thermal/netlink/intel_hfi: Enable HFI feature only when required](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=b33f3d2677b8ddd7a3aba2b02497422a1d2c2a01) | 英特尔 HFI 驱动程序可以在需要时启用它, 从而"节省大量 CPU 周期". 仅在有用户空间使用者处于活动状态时启用它. 如果英特尔 Speed Select 或英特尔低功耗守护程序正在运行, 英特尔 HFI 接口将被激活, 但如果不是, 它将被禁用, 从而显然可以节省大量 CPU 资源. 参见 phoronix 报道 [英特尔 HFI 驱动程序可以“节省大量 CPU 周期”，只需在需要时才启用自身](https://www.phoronix.com/news/Intel-HFI-Enable-Disable) 以及 [](https://www.phoronix.com/news/Intel-HFI-Efficient-Linux-6.10) | v4 ☐☑✓ v6.10-rc1 | [LORE v4,0/3](https://lore.kernel.org/all/20240223155942.60813-1-stanislaw.gruszka@linux.intel.com) |




HFI 中断处理, 流程如下所示:

```cpp
intel_thermal_interrupt()
    -=> intel_hfi_process_event(msr_val & PACKAGE_THERM_STATUS_HFI_UPDATED);    // if (this_cpu_has(X86_FEATURE_HFI))
        -=> queue_delayed_work(hfi_updates_wq, &hfi_instance->update_work, HFI_UPDATE_INTERVAL);
```

HFI 创建了一个名为 hfi-updates 的 workqueue, 最终通过 hfi_instance->update_work 即 hfi_update_work_fn 来更新信息.

```cpp
hfi_update_work_fn
    -=> update_capabilities
```


#### 1.4.1.2 编译器支持
-------


[Intel Updates Alder Lake Tuning For GCC, Reaffirms No Official AVX-512](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Alder-Lake-Tuning-GCC)


#### 1.4.1.3 性能评测
-------

[Intel Core I9-12900K VS I5-12600K](https://openbenchmarking.org/vs/Processor/Intel%20Core%20i5-12600K,Intel%20Core%20i9-12900K)

[The Intel 12th Gen Core i9-12900K Review: Hybrid Performance Brings Hybrid Complexity](https://www.anandtech.com/show/17047/the-intel-12th-gen-core-i912900k-review-hybrid-performance-brings-hybrid-complexity)

Intel Alder Lake CPU 支持 AVX 512

[Intel Alder Lake Question (Scheduler)](https://lkml.org/lkml/2021/11/5/79)

[Intel Core i9 12900K"Alder Lake"AVX-512 On Linux](https://www.phoronix.com/scan.php?page=article&item=alder-lake-avx512&num=1)

[Intel Core i7 1280P"Alder Lake P"Linux Laptop Performance](https://www.phoronix.com/scan.php?page=article&item=intel-corei7-1280p)

[An Important Note On The Alder Lake Mobile Power/Performance With Linux 5.19](https://www.phoronix.com/scan.php?page=article&item=linux-519-alderlakep) 针对 AdlerLake 笔记本测试了 v5.18 和 v5.19 电源管理等带来的能效差异.

#### 1.4.1.4 调度器优化
-------

为了更好的发挥这种混合架构的优势, Intel 提供了一项名为 [Thread Director 的技术](https://www.anandtech.com/show/16881/a-deep-dive-into-intels-alder-lake-microarchitectures/2), 专利分析 [The Alder Lake hardware scheduler – A brief overview](https://coreteks.tech/articles/index.php/2021/07/02/the-alder-lake-hardware-scheduler-a-brief-overview/)

1.  首席按通过机器学习算法对进程在 P-core/E-core 的性能和功耗进行分析和建模, 识别进程特征, 从而可以预测出不同类型进程或者进程某段时期在 P/E core 上的能效关系.

2.  硬件将进程的运行能效换算后, 直接填写到内存中. 当进程在 P/E core 上运行时负责某种规则时(比如进程行为特征发生变化, 即能效发生变化), 直接通知 OS/Kernel 调度器进行决策, 以达到最好的能效.

### 1.4.2 AMD's version of big.LITTLE
-------

AMD 关于大小核的专利 [US20210173715A1: METHOD OF TASK TRANSITION BETWEEN HETEROGENOUS PROCESSORS](https://www.freepatentsonline.com/y2021/0173715.html), 最初于 2019 年 12 月提交, 2021 年被公布. 根据该专利, CPU 将依靠 CPU 利用率等指标来确定何时适合将工作负载从一种类型的 CPU 转移到另一种类型. 建议的指标包括 CPU 以最大速度工作的时间量、CPU 使用最大内存的时间量、一段时间内的平均利用率以及工作负载从一个 CPU 转移的更一般类别另一个基于与任务执行相关的未指定指标等. 当 CPU 确定工作负载应从 CPU A 转移到 CPU B 时, 当前执行工作的内核 (在本例中为 CPU A) 将进入空闲或停止状态. CPU A 的架构状态保存到内存并由 CPU B 加载, 然后继续该过程. 通过这种 Save/Restore 的方式可以实现软件无感的快速迁移, 并且支持的迁移是双向的, 小核可以将任务迁移到大核, 反之亦然.

参见相关报道:

[效仿 Arm, AMD 也在做大小核芯片](https://mp.weixin.qq.com/s/ckDRWep-ih287wIsjddRrw).

[AMD “big.LITTLE” aka heterogeneous computing in Ryzen 8000 series](https://videocardz.com/newz/amd-patents-a-task-transition-method-between-big-and-little-processors).

[AMD reportedly working on its heterogeneous CPU design](https://technosports.co.in/2021/06/14/amd-reportedly-working-on-its-heterogeneous-cpu-design).

[Recently published patent hints at AMD hybrid CPU plans](https://m.hexus.net/tech/news/cpu/147956-recently-published-patent-hints-amd-hybrid-cpu-plans).


### 1.4.3 ARM big.LITTLE & DynamIQ
-------




#### 1.4.4 理论研究
-------

2012 年 Intel 发布了一篇关于大小核的论文 [Scheduling Heterogeneous Multi-Cores through Performance Impact Estimation (PIE)](http://www.jaleels.org/ajaleel/publications/isca2012-PIE.pdf). 论文中小核使用了 In-Order 的微架构, 大核则使用了 Out-Order 的微架构, 传统的想法都是计算密集型 (compute-intensive) 的进程跑到大核上, 访存密集型 (memory-intensive) 的进程跑到小核上去. 但是论文中提出了不一样的想法. 因为内存访问的密集程序并不是一个能反应进程的负载对 CPU 需求的好指标.

1.  如果应用的指令流拥有较高水平的 ILP, 那么它在 In-Order 的小核上, 也能获得很好的性能.

2.  如果应用有较大的 MLP(访存的并发) 以及动态的 ILP, 那么它在 Out-Order 的大核上, 才能获得很好的性能.

因此在大小核上应该分析和利用进程的 ILP/MLP 等特征, 这样才能获得更好的性能.

这应该是 Intel 关于大小核混合架构 (single-ISA heterogeneous multi-cores) 最早的研究, 此后 Intel 发表了陆续发表了关于大小核混合架构的多项研究.



## 1.5 RAPL
-------

[RUNNING AVERAGE POWER LIMIT – RAPL](https://01.org/blogs/2014/running-average-power-limit-%E2%80%93-rapl)

[Understanding Intel's RAPL Driver On Linux](https://www.phoronix.com/scan.php?page=news_item&px=MTcxMjY)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2013/11/12 | Stephane Eranian <eranian@google.com> | [perf/x86: add Intel RAPL PMU support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=65661f96d3b32f4b28fef26d21be81d7e173b965) | TODO | v7 ☑✓ 3.14-rc1 | [LORE v7,0/4](https://lore.kernel.org/all/1384275531-10892-1-git-send-email-eranian@google.com) |
| 2023/02/17 | Wyes Karny <wyes.karny@amd.com> | [perf/x86/rapl: Enable Core RAPL for AMD](https://lore.kernel.org/all/20230217161354.129442-1-wyes.karny@amd.com) | AMD 处理器支持通过 RAPL 计数器对每个包和每个核心的能量进行监控, 用户可以在监控模式下访问 RAPL 计数器.<br> 核心 RAPL 计数器提供每个核心的功耗信息. 对于 AMD 处理器, 包级 RAPL 计数器已暴露给 perf. 通过 `perf stat -a --per-core -C 0-127 -e power/energy-cores` 查看. 参见 phoronix 报道 [Tiny Patch Gets AMD Per-Core Energy Monitoring For Linux's Perf](https://www.phoronix.com/news/Linux-Perf-AMD-Per-Core-Energy) | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20230217161354.129442-1-wyes.karny@amd.com) |



## 1.6 安全相关
-------

### 1.6.1 SGX(Intel Software Guard eXtensions , 英特尔软件保护扩展)
-------


2013 年, Intel 推出 SGX(software guard extensions) 指令集扩展, 能创造出一个可信执行环境来保护代码和数据, 即使 root 权限也无法访问. 这样应用程序可以使用它来隔离代码和数据的特定可信区域, 防止底层 OS 被 compromise 以后对自己的攻击,
同时在软件的管理上也可以不用信任云供应商. 比较符合当前要解决的云计算安全问题, 比如给安全敏感服务存放密钥等.

SGX 旨在以硬件安全为强制性保障, 不依赖于固件和软件的安全状态, 提供用户空间的可信执行环境, 通过一组新的指令集扩展与访问控制机制, 实现不同程序间的隔离运行, 保障用户关键代码和数据的机密性与完整性不受恶意软件的破坏.

不同于其他安全技术, SGX 可信计算(trusted computing base, 简称 TCB) 仅包括硬件, 避免了基于软件的 TCB 自身存在软件安全漏洞与威胁的缺陷, 极大地提升了系统安全保障; 此外, SGX 可保障运行时的可信执行环境, 恶意代码无法访问与篡改其他程序运行时的保护内容, 进一步增强了系统的安全性; 基于指令集的扩展与独立的认证方式, 使得应用程序可以灵活调用这一安全功能并进行验证. 作为系统安全领域的重大研究进展, Intel SGX 是基于 CPU 的新一代硬件安全机制, 其健壮、可信、灵活的安全功能与硬件扩展的性能保证, 使得这项技术具有广阔的应用空间与发展前景. 目前, 学术界和工业界已经对 SGX 技术展开了广泛的研究, Intel 也在其最新的第六代 CPU 中加入了对 SGX 的支持.


[SGX 技术的分析和研究](http://www.jos.org.cn/html/2018/9/5594.htm)


| 版本 | 特性 | PatchSet |
|:----:|:---:|---------:|
| v5.11 | Linux 支持 Intel SGX | [Intel SGX foundations](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=bc4bac2ecef0e47fd5c02f9c6f9585fd477f9beb) |
| v5.13 | Linux 虚拟化支持 Intel SGX |  [KVM SGX virtualization support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fe7e948837f312d87853b3fce743795d1ae3715a), [KVM SGX virtualization support (KVM part)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fe7e948837f312d87853b3fce743795d1ae3715a) |
|

#### 1.6.1.1 SGX support
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/11/13 | Jarkko Sakkinen <jarkko@kernel.org> | [Intel SGX foundations](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=bc4bac2ecef0e47fd5c02f9c6f9585fd477f9beb) | Intel SGX 支持. 参见 phoronix 报道 [After Years Of Work With 40+ Revisions, Intel SGX Looks Like It Will Land In Linux 5.11](https://www.phoronix.com/scan.php?page=news_item&px=Intel-SGX-Linux-5.11) | v41 ☑✓ [5.11-rc1](https://lore.kernel.org/lkml/20201214114200.GD26358@zn.tnic/) | [LORE v41,0/24](https://lore.kernel.org/all/20201112220135.165028-1-jarkko@kernel.org) |
| 2021/03/19 | Kai Huang <kai.huang@intel.com> | [KVM SGX virtualization support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fe7e948837f312d87853b3fce743795d1ae3715a) | Linux 虚拟化支持 Intel SGX | v3 ☑✓ 5.13-rc1 | [LORE v3,0/25](https://lore.kernel.org/all/cover.1616136307.git.kai.huang@intel.com) |
| 2021/04/12 | Kai Huang <kai.huang@intel.com> | [KVM SGX virtualization support (KVM part)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fe7e948837f312d87853b3fce743795d1ae3715a) |  Linux 虚拟化支持 Intel SGX, 参见 phoronix 报道 [Linux 5.13 Bringing Code For Intel SX Within KVM Guests](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.13-SGX-KVM-Guests) | v5 ☑✓ 5.13-rc1 | [LORE v5,0/11](https://lore.kernel.org/all/cover.1618196135.git.kai.huang@intel.com) |
| 2021/11/02 | Catalin Marinas <catalin.marinas@arm.com> | [Basic recovery for machine checks inside SGX](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=3ad6fd77a2d62e8f4465b429b65805eaf88e1b9e) | 支持混合微架构的 CPU(Alder Lake CPU) | v11 ☑✓ 5.17-rc1 | [Patchwork v11,0/7](https://patchwork.kernel.org/project/linux-mm/cover/20211026220050.697075-1-tony.luck@intel.com) |

#### 1.6.1.1 SGX2 support
-------


[Which Platforms Support Intel® Software Guard Extensions (Intel® SGX) SGX2?](https://www.intel.com/content/www/us/en/support/articles/000058764/software/intel-security-products.html)

[Intel SGX2 / Enclave Dynamic Memory Management Patches Posted For Linux](https://www.phoronix.com/scan.php?page=news_item&px=Intel-SGX2-Linux-Patches)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/12/01 | Reinette Chatre <reinette.chatre@intel.com> | [x86/sgx and selftests/sgx: Support SGX2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e0a5915f1cca21da8ffc0563aea9fa1df5d16fb4) | 参见 phoronix 报道 [Intel SGX2 Support Poised To Land In Linux 5.20](https://www.phoronix.com/scan.php?page=news_item&px=Intel-SGX2-Landing-Linux-5.20) | v1 ☐☑✓ | [LORE v1,0/25](https://lore.kernel.org/all/cover.1638381245.git.reinette.chatre@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,00/31](https://lore.kernel.org/lkml/cover.1652137848.git.reinette.chatre@intel.com) |

### 1.6.2 CET
-------

[New Linux Patch Series Provides A Fresh Take On Intel Indirect Branch Tracking](https://www.phoronix.com/scan.php?page=news_item&px=Linux-x86-IBT-New-Shot)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/30 | Yu-cheng Yu <yu-cheng.yu@intel.com> | [Control-flow Enforcement: Shadow Stack](https://patchwork.kernel.org/project/linux-mm/cover/20210830181528.1569-1-yu-cheng.yu@intel.com) | 控制流执行(CET) 是英特尔处理器的一个新特性, 它可以阻止面向返回 / 跳转的编程攻击. 详情见 ["Intel 64 and IA-32 架构软件开发人员手册"](https://software.intel.com/content/www/us/en/develop/download/intel-64-and-ia-32-architectures-sdm-combined-volumes-1-2a-2b-2c-2d-3a-3b-3c-3d-and-4.html). CET 可以保护应用程序和内核. 这是 CET 的第一部分, 本系列仅支持应用程序级保护, 并进一步分为阴影堆栈和间接分支跟踪. | v30 ☐ | [Patchwork v30,00/32](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) |
| 2021/08/30 | Yu-cheng Yu <yu-cheng.yu@intel.com> | [Control-flow Enforcement: Indirect Branch Tracking](https://patchwork.kernel.org/project/linux-mm/cover/20210830182221.3535-1-yu-cheng.yu@intel.com) | 控制流执行(CET) 是英特尔处理器的一个新特性, 它可以阻止面向返回 / 跳转的编程攻击. 详情见 ["Intel 64 and IA-32 架构软件开发人员手册"](https://software.intel.com/content/www/us/en/develop/download/intel-64-and-ia-32-architectures-sdm-combined-volumes-1-2a-2b-2c-2d-3a-3b-3c-3d-and-4.html). 这是 CET 的第二部分, 支持间接分支跟踪(IBT). 它是建立在阴影堆栈系列之上的. | v1 ☐ | [Patchwork v30,00/10](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) |
| 2021/11/22 | Peter Zijlstra <peterz@infradead.org> | [x86: Kernel IBT beginnings](https://lore.kernel.org/lkml/20211122170301.764232470@infradead.org) | IBT 的基础支持, 功能不完整, 但是够精简, 引入 CONFIG_X86_IBT 控制, 添加了 X86_IBT build 选项, 为编译器启用了 "-fcf-protection=branch" 支持, 并支持 objtool, 用于为可能的间接分支目标添加 ENDBR 指令以及内核入口点添加了 ENDBR 结束分支指令. | v11 ☐ | [Patchwork v11,0/7](https://lore.kernel.org/lkml/20211122170301.764232470@infradead.org) |
| 2023/05/11 | Yang Weijiang <weijiang.yang@intel.com> | [Enable CET Virtualization](https://lore.kernel.org/all/20230511040857.6094-1-weijiang.yang@intel.com) | [Intel Sends Out Latest Patches Preparing Linux CET Virtualization](https://www.phoronix.com/news/Intel-CET-Virtualization-v3) | v3 ☐☑✓ | [LORE v3,0/21](https://lore.kernel.org/all/20230511040857.6094-1-weijiang.yang@intel.com) |


## 1.7 PPIN
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2020/03/21 | Wei Huang <wei.huang2@amd.com> | [x86/mce/amd: Add PPIN support for AMD MCE](https://lore.kernel.org/all/20200321193800.3666964-1-wei.huang2@amd.com) | 20200321193800.3666964-1-wei.huang2@amd.com | v3 ☑✓ | [LORE v3,0/1](https://lore.kernel.org/all/20200321193800.3666964-1-wei.huang2@amd.com) |
| 2022/01/31 | Tony Luck <tony.luck@intel.com> | [PPIN (Protected Processor Inventory Number) updates](https://lore.kernel.org/all/20220131230111.2004669-1-tony.luck@intel.com) | 受保护处理器库存编号 (Protected Processor Inventory Number, PPIN), 有时也称为 "受保护处理器标识号(Protected Processor Identification Number) 是处理器在制造时设置的一个编号, 用于唯一地标识给定的处理器. 可以通过 PPIN 追溯芯片生产的车间和生产以及运行环境, 这有助于帮助缺陷诊断. Intel 之前提交了 Linux 补丁, 用于在服务器管理员遇到 MCE 错误时显示 PPIN, 以帮助跟踪问题发生在哪个物理 CPU 上, 但是只有在出现错误时才会打印. 这组补丁使得服务器管理员可以更容易地访问 PPIN. (直接从 `sys/devices/system/cpu/cpu*/topology/ppin` 即可读取). 参见 phoronix 报道 [Convenient Intel PPIN Reporting To Come With Linux 5.18](https://www.phoronix.com/scan.php?page=news_item&px=Intel-PPIN-Linux-5.18) | v3 ☑✓ v5.18-rc1 | [LORE v3,0/5](https://lore.kernel.org/all/20220131230111.2004669-1-tony.luck@intel.com) |



## 1.8 AMD
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/10 | Brijesh Singh <brijesh.singh@amd.com> | [Add AMD Secure Nested Paging (SEV-SNP) Guest Support](https://patchwork.kernel.org/project/linux-mm/cover/20211110220731.2396491-1-brijesh.singh@amd.com) | Guest 支持 AMD SEV SNP.  | v7 ☐ | [Patchwork v7,00/45](https://patchwork.kernel.org/project/linux-mm/cover/20211110220731.2396491-1-brijesh.singh@amd.com) |
| 2022/09/29 | Quan Nguyen <quan@os.amperecomputing.com> | [Add Ampere's Altra SMPro MFD and its child drivers](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=4e6104b1e70020ad500f0fab7238898dd2ea2a38) | [Hardware Monitoring Driver Updates Land In Linux 6.2](https://www.phoronix.com/news/Linux-6.2-HWMON) | v9 ☐☑✓ 6.2 | [LORE v9,0/9](https://lore.kernel.org/all/20220929094321.770125-1-quan@os.amperecomputing.com) |

## 1.9 Software Defined Silicon
-------


[Intel On Demand Driver Ready To Activate Your Licensed CPU Features With Linux 6.2](https://www.phoronix.com/news/Intel-On-Demand-Linux-6.2)

[Intel Details The Accelerators & Security Features For On Demand / Software Defined Silicon](https://www.phoronix.com/news/Intel-On-Demand-Details)

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/02/11 | David E. Box <david.e.box@linux.intel.com> | [Intel Software Defined Silicon](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=2546c60004309ede8e2d1d5341e0decd90e057bf) | TODO | v7 ☐☑✓ | [LORE v7,0/3](https://lore.kernel.org/all/20220212013252.1293396-1-david.e.box@linux.intel.com) |
| 2022/02/24 | David E. Box <david.e.box@linux.intel.com> | [tools arch x86: Add Intel SDSi provisiong tool](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f6d92cfc79e830d9976c69e80f97d80bae7c9c6c) | TODO | v9 ☐☑✓ | [LORE v9,0/2](https://lore.kernel.org/all/20220225012457.1661574-1-david.e.box@linux.intel.com) |
| 2022/11/18 | David E. Box <david.e.box@linux.intel.com> | [Extend Intel On Demand (SDSi) support](https://lore.kernel.org/all/20221119002343.1281885-1-david.e.box@linux.intel.com) | TODO | v2 ☐☑✓ | [LORE v2,0/9](https://lore.kernel.org/all/20221119002343.1281885-1-david.e.box@linux.intel.com) |
| 2023/06/23 | Mario Limonciello <mario.limonciello@amd.com> | [Add dynamic boost control support](https://lore.kernel.org/all/20230623135001.18672-1-mario.limonciello@amd.com) | 动态增强控制是一些 SOC 的一个特征, 它允许经过验证的实体向安全处理器发送命令, 以控制某些 SOC 特性, 从而提高性能. 这是通过一种机制实现的, 用户空间应用程序将通过 IOCTL 接口使用随机数和密钥交换进行身份验证. 身份验证完成后, 应用程序可以与安全处理器交换签名消息, 并且两端都可以验证传输的数据. 该系列包括一个可以在实际硬件上运行的测试套件, 以确保通信按预期工作. 这也可以用于对通信路径进行建模的应用程序. 介绍了两个用于读取 PSP 引导加载程序版本和 TEE 版本的 sysfs 文件, 这两个文件可以作为调试通信问题的有用数据点. 参见 phoronix 报道 [AMD Dynamic Boost Control Feature Set For Introduction In Linux 6.6](https://www.phoronix.com/news/AMD-Dynamic-Boost-Control-66) | v5 ☐☑✓ | [LORE v5,0/11](https://lore.kernel.org/all/20230623135001.18672-1-mario.limonciello@amd.com)|


## 1.10 x86_32
-------


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2023/06/07 | Nikolay Borisov <nik.borisov@suse.com> | [Add ability to disable ia32 at boot time](https://lore.kernel.org/all/20230607072936.3766231-1-nik.borisov@suse.com) | [Linux Patches Posted That Would Allow Boot-Time Disabling Of x86 32-bit Processes](https://www.phoronix.com/news/Linux-Boot-Disable-32-bit) | v1 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20230607072936.3766231-1-nik.borisov@suse.com) |


## 1.11 X86S
-------

[Intel Continues Prepping The Linux Kernel For X86S](https://www.phoronix.com/news/Linux-6.9-More-X86S)


# 2 ARM64
-------



ARM & Linaro [Kernel versions highlights](https://developer.arm.com/tools-and-software/open-source-software/linux-kernel)

ARM64 架构文档地址下载 [cpu-architecture](https://developer.arm.com/architectures/cpu-architecture)

[ARM Processors 网站](https://developer.arm.com/Processors/Cortex-A710) 列出了 ARM 公版的所有 CPU 架构.

[Memory Layout on AArch64 Linux](https://www.kernel.org/doc/html/latest/arm64/memory.html)

[ARM64 Instruction](https://courses.cs.washington.edu/courses/cse469/19wi/arm64.pdf)

[硬件特性列表](https://developer.arm.com/architectures/cpu-architecture/a-profile/exploration-tools/feature-names-for-a-profile)


[Armv8/armv9 架构入门指南(中文)](http://hehezhou.cn/arm_doc)

[arm8/armv9 寄存器速查](http://hehezhou.cn/arm)

[arm8/armv9 指令集](http://hehezhou.cn/isa)

[代码在线阅读(含有 Linux Kernel5.14、TF-A2.5、optee3.16、ffa-spmc 等)](http://hehezhou.cn:8080/optee316)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/09/17 | Catalin Marinas <catalin.marinas@arm.com> | [AArch64 Linux kernel port](https://lore.kernel.org/lkml/1347035226-18649-1-git-send-email-catalin.marinas@arm.com) | 支持 ARM64 | v3 ☑ 3.7-rc1 | [Patchwork v4,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1347035226-18649-8-git-send-email-catalin.marinas@arm.com) |

## 2.1 DEBUG
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/18 | Tianjia Zhang <tianjia.zhang@linux.alibaba.com> | [arm64: Enable OPTPROBE for arm64](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) | 这个补丁集导出了 SM4 AESNI/AVX 算法实现的一些常用函数, 并用这些函数实现 AESNI/AVX2 的加速.<br> 主要算法实现来自 libgcrypt 和 [Markku Juhani 的 SM4 AES-NI 工作](https://github.com/mjosaarinen/sm4ni). | v4 ☐ | [Patchwork v4,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20210818073336.59678-1-liuqi115@huawei.com) |

## 2.2 TLB
-------

[郭健： 进程切换分析之——TLB 处理](https://blog.csdn.net/21cnbao/article/details/110675822)

### 2.2.1 TLB range
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/06/25 | Tianjia Zhang <tianjia.zhang@linux.alibaba.com> | [arm64: tlb: add support for TTL feature](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200625080314.230-1-yezhenyu2@huawei.com) | 为了降低 TLB 失效的成本, ARMv8.4 在 TLBI 指令中提供了 TTL 字段. TTL 字段表示保存被失效地址的叶条目的页表遍历级别. 这组补丁实现了对 TTL 的支持. | v4 ☑ 5.9-rc1 | [Patchwork RESEND,v5,0/6](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200625080314.230-1-yezhenyu2@huawei.com) |
| 2020/07/15 | Zhenyu Ye <yezhenyu2@huawei.com> | [arm64: tlb: Use the TLBI RANGE feature in arm64](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) | 为 ARM64 实现 CONFIG_ARM64_TLB_RANGE, 在实现了 ARM64_HAS_TLB_RANGE 的机器上使用此 feature 实现了 `__flush_tlb_range()` | v3 ☑ 5.9-rc1 | [2020/07/08 PatchWork RFC,v5,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200708124031.1414-1-yezhenyu2@huawei.com)<br>*-*-*-*-*-*-*-* <br>[2020/07/09 PatchWork v1,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200710094420.517-1-yezhenyu2@huawei.com)<br>*-*-*-*-*-*-*-* <br>[2020/07/10 PatchWork v2,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200710094420.517-1-yezhenyu2@huawei.com)<br>*-*-*-*-*-*-*-* <br>[2020/07/15 Patchwork v3,0/3](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200715071945.897-1-yezhenyu2@huawei.com) |

### 2.2.2 ASID
-------

[arm linux 的 ASID (Address Space ID)](https://blog.csdn.net/adaptiver/article/details/70228767)

[ARMv8 ARM64 架构中 ASID](https://zhuanlan.zhihu.com/p/55265099)

[多核 MMU 和 ASID 管理逻辑](https://zhuanlan.zhihu.com/p/118244515)

[TLB 中 ASID 和 nG bit 的关系](https://blog.csdn.net/rockrockwu/article/details/81090883)

### 2.2.3 TLB Shootdown
-------

[arm64 中的 TLB 失效指令](https://blog.csdn.net/choumin/article/details/108936006)

arm64 架构提供了一条 TLB 失效指令:

```cpp
TLBI <type><level>{IS}, {, <Xt>}
```

| 字段 | 描述 |
|:----:|:---:|
| type | 指定了刷新规则, 即只刷新满足特定条件的 tlb 表项, 例如 all 表示所有表项, vmall 表示当前虚拟机的阶段 1 1 的所有表项, asid 表示匹配寄存器 Xt 指定的 ASID 的表项, va 匹配寄存器 Xt 指定的虚拟地址和 ASID 的表项, 等等. |
| level | 异常级别, 取值有: E1、E2、E3. |
| IS | 表示内部共享, 即多个核共享, 如果不使用 IS 字段, 则表示非共享, 只被一个核使用. |
| Xt | 表示 X0 ~ X31 中的任何一个寄存器. |


TLB entry shootdown 常常或多或少的带来一些性能问题.

[Ptlbmalloc2: Reducing TLB Shootdowns with High Memory Efficiency](https://web.njit.edu/~dingxn/papers/ispa20.pdf)

[Torwards a more Scalable KVM Hypervisor](https://events19.linuxfoundation.org/wp-content/uploads/2017/12/Update_Wanpeng-LI_Torwards-a-more-Scalable-KVM-Hypervisor.pdf)

[TLB flush 操作](http://www.wowotech.net/memory_management/tlb-flush.html)
[进程切换分析(2)：TLB 处理](http://www.wowotech.net/process_management/context-switch-tlb.html)

[stackoverflow: What is tlb shootdown ?](https://stackoverflow.com/questions/3748384/what-is-tlb-shootdown)

[深入理解 Linux 内核 --jemalloc 引起的 TLB shootdown 及优化](https://blog.csdn.net/weixin_46837673/article/details/105379768)

在多核系统中, 进程可以调度到任何一个 CPU 上执行, 从而导致 task 处处留情. 如果 CPU 支持 PCID 并且在进程切换的时候不 flush tlb, 那么系统中各个 CPU 中的 tlb entry 则保留各种 task 的 tlb entry, 当在某个 CPU 上, 一个进程被销毁, 或者修改了自己的页表 (修改了 VA PA 映射关系) 的时候, 我们必须将该 task 的相关 tlb entry 从系统中清除出去. 这时候, 不仅仅需要 flush 本 CPU 上对应的 TLB entry, 还需要 shootdown 其他 CPU 上的和该 task 相关的 tlb 残余.

通知其他 CPU shootdown 到指定 task 的 TLB entry 有两种方式: 广播 和 IPI.

1.  广播的方式, 当进程销毁或者页表修改时, 则本地 CPU 通过广播的方式将请求发送出去. 各个 CPU 接受到请求之后, 如果 TLB 中包含了对应的 TLB entry, 则 flush, 否则则抛弃此请求. 这种方式主要由硬件参与完成, 在 shootdown 请求比较少的时候, 硬件处理及时且迅速, 开销也小.

2.  IPI 的方式, 需要先由软件 (内核) 识别到哪些 CPU 中包含了这些需要 flush 的 TLB entry, 然后由本地 CPU 通过 IPI 请求的方式, 通知对应的 CPU 进行 shootdown 操作. 这种方式由硬件软件交互完成, 在 CPU 核数比较多, 且 shootdown 请求比较多的时候, 可能造成 TLB entry shootdown 广播风暴, 硬件 (NM) 处理不过来, 造成性能下降. 此时对于单个 task 的 TLB entry 来说, 残留的 CPU 往往是明确且有限的, 通过 IPI 的方式, 给硬件压力反而会小很多, 这时采用 IPI 的方式性能反而会好.



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/07/11 | David Daney <ddaney.cavm@gmail.com>/<david.daney@cavium.com> | [arm64, mm: Use IPIs for TLB invalidation.](http://lists.infradead.org/pipermail/linux-arm-kernel/2015-July/355866.html) | 在 Cavium ThunderX (ARM64) 的机器上, 某些场景下不能使用广播 TLB, TLB 广播风暴会导致严重的性能问题, 所以我们在必要时使用 IPIs. 测试发现, 它还使内核构建的速度更快. | v1 ☐ | [Patchwork 0/3](http://lists.infradead.org/pipermail/linux-arm-kernel/2015-July/355866.html) |
| 2016/08/04 | Matthias Brugger <mbrugger@suse.com> | [arm64, mm: Use IPIs for TLB invalidation.](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1470302117-32296-3-git-send-email-mbrugger@suse.com/) | NA | v1 ☐ | [Patchwork 2/4](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1470302117-32296-3-git-send-email-mbrugger@suse.com) |
| 2020/11/03 | Nianyao Tang <tangnianyao@huawei.com> | [KVM: arm64: Don't force broadcast tlbi when guest is running](https://lists.cs.columbia.edu/pipermail/kvmarm/2020-November/043071.html) | KVM 当 guest 在运行的时候, 避免 tlbi 广播. | RFC v1 ☐ | [Patchwork RTC](https://lore.kernel.org/linux-arm-kernel/1603331829-33879-1-git-send-email-zhangshaokun@hisilicon.com) |
| 2019/06/17 |  Takao Indoh <indou.takao@jp.fujitsu.com> | [arm64: Introduce boot parameter to disable TLB flush instruction within the same inner shareable domain](hhttps://lists.cs.columbia.edu/pipermail/kvmarm/2020-November/043071.html) | 富士通的开发人员发现 ARM64 TLB.IS 广播在 HPC 上造成了严重的性能下降, 因此新增一个 disable_tlbflush_is 参数来禁用 TLB.IS 广播, 使用原始的 TLB IPI 方式. | RFC v1 ☐ | [Patchwork 0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20190617143255.10462-1-indou.takao@jp.fujitsu.com), [LORE](https://lore.kernel.org/linux-arm-kernel/20190617143255.10462-1-indou.takao@jp.fujitsu.com) |
| 2016/10/24 | Marc Zyngier <marc.zyngier@arm.com> | [arm/arm64: KVM: Perform local TLB invalidation when multiplexing vcpus on a single CPU](https://patchwork.kernel.org/project/kvm/patch/1477323088-18768-1-git-send-email-marc.zyngier@arm.com) | KVM 当 guest 在运行的时候, 避免 tlbi 广播. | RFC v1 ☐ | [Patchwork RTC](https://lore.kernel.org/linux-arm-kernel/1603331829-33879-1-git-send-email-zhangshaokun@hisilicon.com) |
| 2020/02/23 | Andrea Arcangeli <aarcange@redhat.com> | [arm64: tlb: skip tlbi broadcast v2](https://lore.kernel.org/all/20200223192520.20808-1-aarcange@redhat.com) |20200223192520.20808-1-aarcange@redhat.com | v1 ☐ | [LORE](https://lore.kernel.org/all/20200223192520.20808-1-aarcange@redhat.com) |
| 2022/09/13 | Joe Damato <jdamato@fastly.com> | [mm: Track per-task tlb events](https://lore.kernel.org/all/1663120270-2673-1-git-send-email-jdamato@fastly.com) | 通过检查 `/proc/interrupts`, 可以在每个 CPU 的基础上测量 TLB shootdown 事件. 如果 CONFIG_DEBUG_TLBFLUSH 被启用, 关于 TLB 事件的进一步信息可以从 `/proc/vmstat` 中获取, 但是这些信息是系统范围的. 这些信息是有用的, 但是在一个有许多任务的繁忙系统上, 很难消除 TLB shootdown 事件的来源的模糊性. 这组补丁跟踪每个任务的这些信息可以使开发人员修复或调整用户空间分配器, 以减少 IPI 的数量并提高应用程序性能. 为 task_struct 和 signal_struct 添加了两个新字段, 以帮助跟踪 TLB 事件:<br>1. ngtlbflush: 生成 TLB flush 的数量.<br>2. nrtlbflush: 收到 TLB flush 的数量.<br> 这些统计数据被导出到 `/proc/[pid]/stat` 中, 与类似的指标 (如 min_flt 和 maj_flt) 一起进行分析. | v1 ☐☑✓ | [LORE v1,0/1](https://lore.kernel.org/all/1663120270-2673-1-git-send-email-jdamato@fastly.com) |
| 2023/03/12 | Yair Podemsky <ypodemsk@redhat.com> | [send tlb_remove_table_smp_sync IPI only to necessary CPUs](https://lore.kernel.org/all/20230312080945.14171-1-ypodemsk@redhat.com) | 目前, tlb_remove_table_smp_sync() 将 IPI 被不分青红皂白地发送到所有 CPU, 这会导致不必要的工作和延迟, 在实时用例和隔离的 CPU 中尤为值得注意, 此补丁将限制此 IPI 仅发送到引用受影响 mm 的 cpu, 并且当前在内核空间中.  | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20230312080945.14171-1-ypodemsk@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/3](https://lore.kernel.org/r/20230404134224.137038-1-ypodemsk@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/2](https://lore.kernel.org/r/20230620144618.125703-1-ypodemsk@redhat.com) |


> 注: x86 由于没有 tlb IS 方案, 因此只能采用 IPI 的方式来完成 TLB shootdown.

目前 ARM64 中 TLUSH TLB 的接口:

| 接口 | 描述 |
|:---:|:---:|
| flush_tlb_all | 无效掉所有的 TLB entry.(包括内核的和用户态的) |
| flush_tlb_mm | 无效掉 mm_struct 指向的所有 TLB entry.<br>1. mm_struct 指向的都是进程的用户态空间.<br>2. ARM64 上每个 mm_struct 有自己单独的 ASID. 当前只需要对指定的 ASID 执行 TLB.IS 即可 |
| flush_tlb_range | 无效掉用户态地址 start ~ end 区间内的所有 tlb entry |
| flush_tlb_kernel_range | 无效掉内核态 start ~ end 区间内的所有 TLB entry |
| local_flush_tlb_all | 无效掉本 CPU 上所有的 TLB entry. 无需使用 TLB.IS |

### 2.2.4 BATCHED_UNMAP_TLB_FLUSH
-------

取消映射页时, 需要刷新 TLB. 由于该页面可能被其他 CPU 访问过, 则使用 IPI 刷新远程 CPU 的 TLB. 如果 kswapd 每秒扫描和取消映射 >100K 页面, IPI 数量非常庞大. 因此在 v4.3 s[TLB flush multiple pages per IPI v7](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=c7e1e3ccfbd153c890240a391f258efaedfa94d0) 引入了 BATCHED_UNMAP_TLB_FLUSH 用于 TLB 批处理, 在批量解除页面映射后, 发送[一个 IPI 来刷新多个 TLB range 的条目](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=72b252aed506b8f1a03f7abd29caef4cdf6a043b), 而不是每个 TLB entry 刷新都发送一个 IPI.

1.  try_to_unmap_one() 批量接触页面映射后, 通过 should_defer_flush() 判断是否应该延迟刷新, 即整体触发一次批量刷新.

2.  shrink_page_list() 释放页面并解除映射的时候, 则使用 try_to_unmap_flush() 刷新最近未映射页面的 TLB 条目.

3.  如果在取消映射时 PTE 是脏的, 那么在页上发起任何 IO 之前刷新它以防止写操作丢失是很重要的. 类似地, 它必须在释放之前刷新, 以防止数据泄漏. 参见 [mm: defer flush of writable TLB entries](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d950c9477d51f0cefc2ed3cf76e695d46af0d9c1)

但是, ARM64 架构下 TLB shootdown 是由硬件通过 tlbi 指令完成的. 刷新指令是内部共享的. 本地刷新限制在启动 (每个 CPU 1 次) 和任务获得新的 ASID 时. 像 ARM64 这样的平台有硬件 TLB shootdown 广播. 它们不维护 mm_cpumask, 只是发送 tlbi 和相关的同步指令用于 TLB 刷新. 从这个角度上讲, 可以认为 ARM64 其实是不需要 BATCHED_UNMAP_TLB_FLUSH, 因此 v5.13 [Documentation/features: mark BATCHED_UNMAP_TLB_FLUSH doesn't apply to ARM64](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6bfef171d0d74cb050112e0e49feb20bfddf7f42) 将 ARM64 架构下 BATCHED_UNMAP_TLB_FLUSH 标记为 N/A. 在这种架构下, Task 的 mm_cpumask 通常是空的.


在 x86 上, 批处理和延迟 TLB shootdown 的解决方案使 TLB shootdown 的性能提高了 90%. 在 ARM64 上, 硬件可以在没有软件 IPI 的情况下执行 TLB shootdown. 但前面讲解 TLB Shootdown 的时候, 我们已经提到了同步 tlbi 的开销同样相当昂贵. 因此这类平台上, 如果我们也允许延迟 TLB 刷新(即使用批量 TLB 刷新), 将会获得诸多好处.


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2015/07/06 | Mel Gorman <mgorman@suse.de> | [TLB flush multiple pages per IPI v7](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=c7e1e3ccfbd153c890240a391f258efaedfa94d0) | BATCHED_UNMAP_TLB_FLUSH 完成了 TLB 批处理, 在批量解除页面映射后, 发送一个 IPI 来刷新多个 TLB range 的条目, 而不是每个 TLB entry 刷新都发送一个 IPI. | v7 ☑✓ 4.3-rc1 | [LORE v5,0/3](https://lore.kernel.org/all/1433767854-24408-1-git-send-email-mgorman@suse.de)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/4](https://lore.kernel.org/all/1436189996-7220-1-git-send-email-mgorman@suse.de) |
| 2017/05/07 | Andy Lutomirski <luto@kernel.org> | [x86 TLB flush cleanups, moving toward PCID support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d6e41f1151feeb118eee776c09323aceb4a415d9) | TODO | v1 ☑✓ 4.13-rc1 | [LORE v1,0/10](https://lore.kernel.org/all/cover.1494160201.git.luto@kernel.org) |
| 2021/02/23 | Barry Song <song.bao.hua@hisilicon.com> | [Documentation/features: mark BATCHED_UNMAP_TLB_FLUSH doesn't apply to ARM64](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6bfef171d0d74cb050112e0e49feb20bfddf7f42) | 在 x86 上, BATCHED_UNMAP_TLB_FLUSH 用于批处理 TLB, 在解除页面映射后, 发送一个 IPI 到 TLB 刷新所有条目. 在 arm64 上, TLB shootdown 是由硬件完成的. 刷新指令是内部共享的. 本地刷新限制在启动 (每个 CPU 1 次) 和任务获得新的 ASID 时. 因此, 将该特性标记为 "TODO" 是不恰当的. 所以这个补丁对某些架构上不需要的这类功能标记为 "N/A". | v1 ☑✓ 5.13-rc1 | [LORE](https://lore.kernel.org/all/20210223003230.11976-1-song.bao.hua@hisilicon.com) |
| 2022/07/11 | Barry Song <21cnbao@gmail.com> | [mm: arm64: bring up BATCHED_UNMAP_TLB_FLUSH](https://lore.kernel.org/all/20220711034615.482895-1-21cnbao@gmail.com) | 虽然 ARM64 有硬件来完成 TLB shootdown, 但硬件广播的开销并不小. 最简单的微基准测试表明, 即使在只有 8 核的 snapdragon 888 上, ptep_clear_flush() 的开销也是巨大的, 即使只分页一个进程映射的一个页面, perf top 显示这造成 5.36% 的 CPU 消耗. 当页面由多个进程映射或硬件有更多 CPU 时, 由于 TLB 分解的可扩展性较差, 成本应该会更高. 在这种场景下同样的基准测试可能会导致大约 100 核的 ARM64 服务器上 16.99% 的 CPU 消耗. 该补丁集利用了现有的 BATCHED_UNMAP_TLB_FLUSH 进行了优化.<br>1. 仅在第一阶段 arch_tlbbatch_add_mm() 中发送 tlbi 指令.<br>2. 等待 dsb 完成 tlbi, 同时在 arch_tlbbatch_flush() 中执行 tlbbatch sync. 在 snapdragon 上的测试表明, ptep_clear_flush() 的开销已被该补丁集优化掉. 即使在 snapdragon 888 上通过单个进程映射一个页面, 微基准也能提升 5% 的性能. | v2 ☐☑✓ | [LORE v1,0/4](https://lore.kernel.org/lkml/20220707125242.425242-1-21cnbao@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/4](https://lore.kernel.org/all/20220711034615.482895-1-21cnbao@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/4](https://lore.kernel.org/r/20220822082120.8347-1-yangyicong@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/2](https://lore.kernel.org/r/20220921084302.43631-1-yangyicong@huawei.com) |


### 2.2.5 relaxed TLB flushes
-------

[VMware Is Exploring Reducing Meltdown/PTI Overhead With Deferred Flushes](https://www.phoronix.com/scan.php?page=news_item&px=VMware-RFC-Defer-PTI-Flushes)

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2019/08/23 | Nadav Amit <namit@vmware.com> | [x86/mm/tlb: Defer TLB flushes with PTI](https://lore.kernel.org/all/20190823225248.15597-1-namit@vmware.com) | TODO | v2 ☐☑✓ | [LORE v2,0/3](https://lore.kernel.org/all/20190823225248.15597-1-namit@vmware.com) |


[Relaxed TLB Flushes Being Worked On For Linux As Another Performance Optimization](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Relaxed-TLB-Flushes).


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/07/18 | Nadav Amit <nadav.amit@gmail.com> | [mm: relaxed TLB flushes and other optimi.](https://lore.kernel.org/all/20220718120212.3180-1-namit@vmware.com) | TODO | v1 ☐☑✓ | [LORE v1,0/14](https://lore.kernel.org/all/20220718120212.3180-1-namit@vmware.com) |


## 2.3 指令加速
-------

### 2.3.1 LSE
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/07/24 | Will Deacon <will.deacon@arm.com> | [arm64: support for 8.1 LSE atomic instructions](https://lwn.net/Articles/650900) | 为 Linux 内核添加了对 [新原子指令(LSE atomic instructions) 的支持](https://mysqlonarm.github.io/ARM-LSE-and-MySQL), 这是作为 ARMv8.1 中大系统扩展 (LSE-LSE atomic instructions) 的一部分引入的. 新的指令可以在编译时通过 CONFIG_ARM64_LSE_ATOMICS 选项配置出来.<br> 之前测试发现, 核少的时候关 LSE 性能更好, 核多的时候, 开 LSE 性能更好. | v1 ☑ 4.3-rc1 | [2015/07/13 Patchwork 00/18](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200625080314.230-1-yezhenyu2@huawei.com)<br>*-*-*-*-*-*-*-* <br>[2015/07/24 Patchwork v2,07/20](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1437734531-10698-8-git-send-email-will.deacon@arm.com/) |
| 2021/12/10 | Mark Brown <broonie@kernel.org> | [arm64: atomics: cleanups and codegen improvements](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20211027184424.166237-1-broonie@kernel.org) | SME 指令的支持. | v5 ☑ [5.17-rc1](https://lore.kernel.org/lkml/20220106185501.1480075-1-catalin.marinas@arm.com) | [Patchwork 0/5](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20211210151410.2782645-1-mark.rutland@arm.com) |


### 2.3.2 SVE
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/10/30 | Mark Brown <broonie@kernel.org> | [ARM Scalable Vector Extension (SVE)](https://patchwork.ozlabs.org/project/glibc/cover/1509101470-7881-1-git-send-email-Dave.Martin@arm.com) | SVE 指令的支持. 目前不支持 KVM guest 使用 SVE 指令. 这种将被捕获并作为未定义的指令执行反映给 guest. SVE 在 guest 可见的 CPU 功能寄存器视图中是隐藏的, 因此 guest 不会期望它工作. | v4 ☑ 4.15-rc1 | [Patchwork v4,00/28](https://patchwork.ozlabs.org/project/glibc/cover/1509101470-7881-1-git-send-email-Dave.Martin@arm.com)<br>*-*-*-*-*-*-*-* <br>[Patchwork v5,01/30](https://patchwork.ozlabs.org/project/glibc/patch/1509465082-30427-2-git-send-email-Dave.Martin@arm.com) |
| 2019/03/29 | Mark Brown <broonie@kernel.org> | [KVM: arm64: SVE guest support](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1553864452-15080-1-git-send-email-Dave.Martin@arm.com) | KVM guest SVE 指令的支持. | v7 ☑ 5.2-rc1 | [Patchwork v7,00/27](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1553864452-15080-1-git-send-email-Dave.Martin@arm.com) |
| 2019/04/18 | Mark Brown <broonie@kernel.org> | [KVM: arm64: SVE cleanups](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1555603631-8107-1-git-send-email-Dave.Martin@arm.com) | KVM guest SVE 指令的支持. | v2 ☑ 5.2-rc1 | [Patchwork v2,00/14](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1555603631-8107-1-git-send-email-Dave.Martin@arm.com) |
| 2019/04/18 | Mark Brown <broonie@kernel.org> | [arm64: Expose SVE2 features for userspace](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1555609298-10498-1-git-send-email-Dave.Martin@arm.com) | 支持向用户空间报告 SVE2 的存在及其可选功能. 同时为 KVM 虚拟化 guest 提供了 SVE2 的可见性. | v2 ☑ 5.2-rc1 | [Patchwork v2,00/14](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1555609298-10498-1-git-send-email-Dave.Martin@arm.com) |

2022 年 6 月, Arm 工程师 Wilco Dijkstra 为 Glibc 提供了 SVE 优化的 memcpy 实现, 超过 32 字节的 memcpy 使用 SVE 实现, 这显着改善了随机 memcpy 测试的性能. 参见 [glibc-commit](https://sourceware.org/git/?p=glibc.git;a=commit;h=9f298bfe1f183804bb54b54ff9071afc0494906c), 以及 phoronix 报道 --[Glibc Adds Arm SVE-Optimized Memory Copy - Can"Significantly"Help Performance](https://www.phoronix.com/scan.php?page=news_item&px=Glibc-Arm-SVE-Memcpy-Optimize).



### 2.3.3 SME
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/10/27 | Mark Brown <broonie@kernel.org> | [arm64/sme: Initial support for the Scalable Matrix Extension](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20211027184424.166237-1-broonie@kernel.org) | SME 指令的支持. v7 版本前 6 个 [prepare 的补丁](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-AArch64)先合入了 [5.17-rc1](https://lore.kernel.org/lkml/20220106185501.1480075-1-catalin.marinas@arm.com) | v5 ☐ | [LORE v3,00/42](https://lore.kernel.org/all/20211019172247.3045838-1-broonie@kernel.org)<br>*-*-*-*-*-*-*-* <br>[2021/10/27 Patchwork v5,00/38](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20211027184424.166237-1-broonie@kernel.org)<br>*-*-*-*-*-*-*-* <br>[2021/12/10 Patchwork v7,00/37](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20211210184133.320748-1-broonie@kernel.org) |
| 2023/01/16 | Mark Brown <broonie@kernel.org> | [arm64/sme: Support SME 2 and SME 2.1](https://lore.kernel.org/all/20221208-arm64-sme2-v4-0-f2fa0aef982f@kernel.org) | Arm 最近发布了 SME 扩展的版本 2 和 2.1. SME 2 引入的特征之一是一些新的体系结构状态, 即 ZT0 寄存器. 本系列增加了对这一功能以及新 SME 版本的所有其他功能的支持. | v4 ☐☑✓ | [LORE v4,0/21](https://lore.kernel.org/all/20221208-arm64-sme2-v4-0-f2fa0aef982f@kernel.org) |


## 2.4 pseudo-NMI
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/01/31 | Julien Thierry <julien.thierry@arm.com> | [arm64: provide pseudo NMI with GICv3](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1548946743-38979-1-git-send-email-julien.thierry@arm.com) | ARM64 通过中断优先级实现(伪)NMI | v10 ☑ [5.1-rc1](https://kernelnewbies.org/Linux_5.1#ARM) | [Patchwork v10,00/25](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1548946743-38979-1-git-send-email-julien.thierry@arm.com), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bc3c03ccb4641fb940b27a0d369431876923a8fe) |
| 2019/06/11 | Julien Thierry <julien.thierry@arm.com> | [arm64: IRQ priority masking and Pseudo-NMI fixes](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1560245893-46998-1-git-send-email-julien.thierry@arm.com) | 修复伪 NMI 的诸多问题 | v4 ☑ 5.10-rc1 | [Patchwork v4,0/8](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1560245893-46998-1-git-send-email-julien.thierry@arm.com) |
| 2019/07/17 | Julien Thierry <julien.thierry@arm.com> | [arm_pmu: Use NMI for perf interrupt](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1563351432-55652-1-git-send-email-julien.thierry@arm.com) | ARM64 perf 的中断通过伪 NMI 上报, 这样会使 perf 的热点采样更加精准. | v4 ☑ 5.10-rc1 | [Patchwork v4,0/9](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1563351432-55652-1-git-send-email-julien.thierry@arm.com) |


## 2.5 PAN
-------


[armv8/arm64 PAN 深入分析](https://cloud.tencent.com/developer/article/1413360)

[Arm64 架构安全 -- PAN](https://zhuanlan.zhihu.com/p/365701044)

[Learn the architecture: AArch64 memory model/Permissions attributes](https://developer.arm.com/documentation/102376/0100/Permissions-attributes)

### 2.5.1 ARMv8 页表权限控制
-------

一般控制一个内存的属性, 如 RWX 权限, 简单的想 3 个 bit 即可, 但是当前操作系统的设计 RWX 权限除了要表示内核的权限以外还要包括用户态的权限, 那么需要就需要 6 个 bit. 但是在 ARM v8 的设计中, 为了节省相应的页表设计 ARM 仅用了 4 个 bit.

| 控制位 | 描述 |
|:-----:|:---:|
| UXN(BIT [54]) |  设置为 1 的时候, 用户态没有执行权限 |
| PXN(BIT [53]) |  设置为 1 的时候, 内核态没有执行权限 |
| AP [2](BIT [7]) | 1 表示内核态是 readonly, 0 表示内核态是 RW 权限 |
| AP [1](BIT [6]) | 1 表示用户态跟内核态的权限一样, 0 表示用户态没有任何权限 |

这就导致了一个问题, 那就是无论怎么设置页表的权限, 只要用户态有权限, 此时内核态的权限跟用户态的权限一致.

这原本貌似也没什么问题, 但是后来安全研究人员发现, 由于内核态可以直接执行相应的用户态程序, 这样攻击者就可以在用户态准备好相应执行的代码, 如果内核里边有一个很小的漏洞, 比如 ROP, 攻击者通过 RetToUser Attrack 把相应的栈中的 ret 值修改为用户态准备好的地址, 那么就可以轻松做到任意代码执行.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/01/06 | James Morse <james.morse@arm.com> | [arm64: Revert support for execute-only user mappings](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=24cecc37746393432d994c0dbc251fb9ac7c5d72) | 实现 ARMv8.1-PAN, Privileged access never. | v1 ☑ 5.5-rc6 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=24cecc37746393432d994c0dbc251fb9ac7c5d72) |


### 2.5.2 ARMv8.1 引入 PAN 解决任意代码执行的漏洞
-------

为了解决这个问题, ARM v8.1 在修复这个问题的时候, 不得不在 pstate 中抠出来一个 bit 来设置 PAN(Privileged Access Never), 如果 PAN 为 1 的时候, 那么就限制在 EL1 里边不允许访问 EL0 的内存. 被称为 ARMv8.1-PAN, Privileged access never, 它的主要工作就是限制内核态不能访问用户态的数据. 如果启用了 CONFIG_ARM64_PAN, 内核试图访问用户空间的内存时, 则会报权限错误, 相反, [copy_from_user()](https://elixir.bootlin.com/linux/v4.3/source/arch/arm64/lib/copy_from_user.S#L34) 以及 [copy_to_user()](https://elixir.bootlin.com/linux/v4.3/source/arch/arm64/lib/copy_to_user.S#L35) 等接口中访问用户空间内存时必须清除 PAN 位(或使用 `ldt*/stt*` 指令), 在完成后则必须恢复.

[Arm Chips Vulnerable to PAN Bypass –"We All Know it’s Broken"](https://techmonitor.ai/techonology/hardware/arm-pan-bypass)
[PAN](https://blog.siguza.net/PAN)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/07/16 | James Morse <james.morse@arm.com> | [arm64: kernel: Add support for Privileged Access Never](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=338d4f49d6f7114a017d294ccf7374df4f998edc) | 实现 ARMv8.1-PAN, Privileged access never. | v4 ☑ 4.3-rc1 | [LORE 0/5](https://lore.kernel.org/linux-arm-kernel/1437062519-18883-1-git-send-email-james.morse@arm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/6](https://lore.kernel.org/all/1437481411-1595-1-git-send-email-james.morse@arm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,rebase](https://lore.kernel.org/all/1437588354-31278-1-git-send-email-james.morse@arm.com), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=338d4f49d6f7114a017d294ccf7374df4f998edc) |
| 2016/10/18 | James Morse <james.morse@arm.com> | [PAN Fixes](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d08544127d9fb4505635e3cb6871fd50a42947bd) | 修复 PAN 代码上的一些问题. | v1 ☑ 4.9-rc2 | [LORE 0/3](https://lore.kernel.org/all/1476786468-2173-1-git-send-email-james.morse@arm.com) |

ARMv8.2-ATS1E1, AT S1E1R and AT S1E1W instruction variants, taking account of PSTATE.PAN

### 2.5.2 ARMv8.2 引入 UAO 解决任意代码执行的漏洞
-------

本来一切看起来貌似恢复平静了, 但是总有一些意外.


开启了 PAN 之后, 内核每次 copy_from_user/copy_to_user 需要访问用户态地址的时候, 不得不动态的禁用和使能 PAN. 于是, 在 ARM v8.2 又引入了 UAO(User Access Override). 与 LDR/STR 指令不同, UAO 提供了 LDTR/STTR 等非特权 load/store 指令, 不管在哪个 ELx 态(即使是 EL1 或 EL2) 运行, 它们都也会根据 EL0 权限检查进行检查. 并且不会被 PAN 阻止.

如果发现当前 CPU ARM64_HAS_UAO, 则会通过 alternative 机制将 [copy_from_user](https://elixir.bootlin.com/linux/v4.6/source/arch/arm64/lib/copy_from_user.S#L70) 以及 copy_to_user 等函数中访问用户态的指令[替换为 ldtr/sttr](https://elixir.bootlin.com/linux/v4.6/source/arch/arm64/include/asm/alternative.h#L148) 等. 而把[使能和禁用 PAN 的操作替换为 NOP](https://elixir.bootlin.com/linux/v4.6/source/arch/arm64/include/asm/uaccess.h#L75) 操作.


[UAO (User Access Override) as a mitigation against addr_limit overwrites](https://duasynt.com/blog/android-uao-kernel-expl-mitigation)

ARM v8.2 引入了 [UAO](https://community.arm.com/arm-community-blogs/b/architectures-and-processors-blog/posts/armv8-a-architecture-evolution)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/02/05 | James Morse <james.morse@arm.com> | [arm64: kernel: Add support for User Access Override](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=705441960033e66b63524521f153fbb28c99ddbd) | 引入 hugetlb cgroup | v2 ☑ 4.6-rc1 | [LORE v1,0/5](https://lore.kernel.org/linux-arm-kernel/1454432611-21333-1-git-send-email-james.morse@arm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/5](https://lore.kernel.org/linux-arm-kernel/1454684330-892-1-git-send-email-james.morse@arm.com) |

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/10/28 | Catalin Marinas <catalin.marinas@arm.com> | [arm64: Privileged Access Never using TTBR0_EL1 switching](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ba42822af1c287f038aa550f3578c61c212a892e) | 引入 CONFIG_ARM64_SW_TTBR0_PAN, 通过软件模拟实现 PAN. 通过将 TTBR0_EL1 指向保留的归零区域和保留的 ASID, 防止内核直接访问用户空间内存. 用户访问例程临时恢复有效的 TTBR0_EL1. | v4 ☑ 4.10-rc1 | [LORE v1,0/7](https://lore.kernel.org/linux-arm-kernel/1471015666-23125-1-git-send-email-catalin.marinas@arm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/7](https://lore.kernel.org/linux-arm-kernel/1473788797-10879-1-git-send-email-catalin.marinas@arm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/8](https://lore.kernel.org/linux-arm-kernel/1477675636-3957-1-git-send-email-catalin.marinas@arm.com) |


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/12/02 | Paul Gortmaker <paul.gortmaker@windriver.com> | [arm64: remove set_fs() and friends](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1517c4facf2e66401394998dba1ee236fd261310) | 引入 hugetlb cgroup | v5 ☑ 5.11-rc1 | [LKML v5,00/12](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20201202131558.39270-1-mark.rutland@arm.com), [LORE](https://lore.kernel.org/r/20201202131558.39270-13-mark.rutland@arm.com) |
| 2021/03/12 | Vladimir Murzin <vladimir.murzin@arm.com> | [arm64: Support Enhanced PAN](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=18107f8a2df6bf1c6cac8d0713f757f866d5af51) | NA | v4 ☑ 5.13-rc1 | [LORE v4,0/2](https://lore.kernel.org/all/20210312173811.58284-1-vladimir.murzin@arm.com) |






## 2.6 PAC
-------

2016 年 10 月, ARMV8.3-A 指令集中加入了 [指针认证(Pointer Authentication) 机制](https://www.kernel.org/doc/html/latest/arm64/pointer-authentication.html), 在使用寄存器的值作为指针访问数据或代码之前验证其内容, 目的是为了对抗 ROP/JOP 攻击.

[ARM pointer authentication](https://lwn.net/Articles/718888)
[开源快递: ARM 指针身份认证 (arm pointer authentication)](https://www.cnblogs.com/liuhailong0112/p/14258844.html)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/12/07 | Kristina Martsenko <kristina.martsenko@arm.com> | [ARMv8.3 pointer authentication userspace support](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20181207183931.4285-1-kristina.martsenko@arm.com) | RISC-V 高级核心本地中断(Advacned Core Local Interruptor-ACLINT) 支持 | v8 ☐ | [Patchwork v5,00/17](https://patchwork.kernel.org/project/linux-riscv/cover/20210830041729.237252-1-anup.patel@wdc.com)<br>*-*-*-*-*-*-*-*<br>[Patchwork v6,00/13](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20181207183931.4285-1-kristina.martsenko@arm.com) |
| 2019/03/19 | Kristina Martsenko <kristina.martsenko@arm.com> | [arm64: pac: Optimize kernel entry/exit key installation code paths](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20181207183931.4285-1-kristina.martsenko@arm.com) | PAC 合入后, svc(SYSCALL) 进入和退出以及进程切换的过程中, 会进行 key 的切换, 会存在 10+ 个 MSR 操作, 因此造成了性能劣化. | v8 ☑ 5.13-rc1 | [Patchwork v8,3/3](https://patchwork.kernel.org/project/linux-arm-kernel/patch/2d653d055f38f779937f2b92f8ddd5cf9e4af4f4.1616123271.git.pcc@google.com)<br>*-*-*-*-*-*-*-*<br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b90e483938ce387c256e03fb144f82f64551847b) |

## 2.7 Linux to Apple Silicon Macs
-------

2020 年 11 月 11 日在苹果新品发布会上发布了适用于部分 Mac、iPad 设备 ARM64 芯片 Apple M1, 社区开始有不少团队开始着手于 Linux 支持 Apple M1 芯片. 目前有两个团队在做这块工作 Corellium 的 [corellium/linux-m1](https://github.com/corellium/linux-m1) 以及 [AsahiLinux/linux](https://github.com/AsahiLinux/linux). 其中

*   [Corellium](https://github.com/corellium) 一直致力于为用户提供虚拟 iOS 系统桌面解决方案, 这个团队从 A10 芯片开始就开始做 Linux 的适配, 因此 M1 刚出来就做了快速的适配.

*   [AsahiLinux]() 国外一名资深操作系统移植专家 Hector Martin(网名为 Marcan) 主导发起的一个开源项目, 其目标是让用户可以在 Apple M1 上把基于 Linux 的发行版作为日常操作系统来使用.

相关报道

[Rewritten Apple Silicon CPUFreq Driver Posted For Linux](https://www.phoronix.com/scan.php?page=news_item&px=Apple-SoC-CPUFreq-Driver-v2)

| 时间   | 作者 | 特性  | 描述  | 是否合入主线  | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2022/05/04 | Hector Martin <marcan@marcan.st> | [Apple SoC cpufreq driver](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20220504075153.185208-1-marcan@marcan.st/) | 638189 | v2 ☐☑ | [LORE v2,0/4](https://lore.kernel.org/r/20220504075153.185208-1-marcan@marcan.st) |




## 2.8 clocksource
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/10/17 | Marc Zyngier <maz@kernel.org> | [clocksource/arm_arch_timer: Add basic ARMv8.6 support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ec8f7f3342c88780d682cc2464daf0fe43259c4f) | NA | v4 ☑ 5.16-rc1 | [LORE v4,00/17](https://lore.kernel.org/all/20211017124225.3018098-1-maz@kernel.org) |


# 3 RISC-V
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/10 | Anup Patel <anup.patel@wdc.com> | [RISC-V CPU Idle Support](https://patchwork.kernel.org/project/linux-riscv/cover/20210610052221.39958-1-anup.patel@wdc.com) | RISC-V 高级核心本地中断(Advacned Core Local Interruptor-ACLINT) 支持 | v8 ☐ | [2021/06/10Patchwork v7,0/8](https://patchwork.kernel.org/project/linux-riscv/cover/20210830041729.237252-1-anup.patel@wdc.com)<br>*-*-*-*-*-*-*-*<br>[LWN v8, 0/8](https://lwn.net/Articles/872513) |
| 2021/08/30 | Anup Patel <anup.patel@wdc.com> | [Linux RISC-V ACLINT Support](https://patchwork.kernel.org/project/linux-riscv/cover/20210830041729.237252-1-anup.patel@wdc.com) | RISC-V 高级核心本地中断(Advacned Core Local Interruptor-ACLINT) 支持 | v3 ☐ | [Patchwork RFC,v3,00/11](https://patchwork.kernel.org/project/linux-riscv/cover/20210610052221.39958-1-anup.patel@wdc.com) |


# 4 CPU 漏洞
-------

[GhostRace Detailed - Speculative Race Conditions Affecting All Major CPUs / ISAs](https://www.phoronix.com/news/CPU-Speculative-GhostRace)

## 4.1 Straight Line Speculation
-------

[Blocking straight-line speculation — eventually](https://lwn.net/Articles/877845)

Google 的 SafeSide 小组发现 ARM CPU 存在新的投机执行漏洞, 可导致侧信道攻击. 研究人员在 Armv8-A(Cortex-A) CPU 体系结构中发现了一个名为直线推测(Straight-Line Speculation , SLS) 的新漏洞, 被追踪为 [CVE-2020-13844](https://nvd.nist.gov/vuln/detail/CVE-2020-13844). 该漏洞可导致攻击者对 ARM 架构处理器进行侧边信道攻击(SCA).

让目标处理器通过预先访问数据来提升性能, 然后再扔掉所有使用过的计算分支 (computational branches), 而 SLS 则通过类似的侧道攻击就能让黑客从处理器直接获得(窃取) 重要数据.

SLS 被认为是 Spectre 漏洞的变体, 但二者的攻击范围略有不同, SLS 漏洞仅影响 Arm Armv-A 处理器, 而 Spectre 漏洞影响所有主要芯片制造商的 CPU. 到目前为止, 该漏洞还没有在野利用.

[Arm CPUs Hit By Straight Line Speculation Vulnerability, LLVM Adds Initial Mitigation](https://www.phoronix.com/scan.php?page=news_item&px=Arm-Straight-Line-Speculation)

很快在 [GCC](https://gcc.gnu.org/pipermail/gcc-patches/2020-June/547520.html) 和 [LLVM](https://reviews.llvm.org/rG9c895aea118a2f50ca8413372363c3ff6ecc21bf) 编译器中推出了针对 SLS 的保障措施, 通过在易受 SLS 影响的指令周围插入投机障碍 (SB) 指令或其他 DSB + ISB 指令来减少直线预测.

[LLVM Adds Additional Protections For Arm's SLS Speculation Vulnerability Mitigation](https://www.phoronix.com/scan.php?page=news_item&px=Arm-SLS-More-In-LLVM)

11 月 17 日, 将 x86/x86_64 的 SLS 缓解选项 -mharden-SLS 合并到 GCC 12 Git 上, 预计不久将推出内核补丁, 将 -mharden-SLS 缓解选项作为对 x86 cpu 最新的安全保护. 这个选项包括 none、all、return 或 indirect-branch 四个值, x86/x86_64 架构上的原理是通过在函数返回和间接分支之后添加 INT3 断点指令, 来减少函数返回和间接分支的直线推测(SLS). 参见 [Linux + GCC/Clang Patches Coming For Straight-Line Speculation Mitigation On x86/x86_64](https://www.phoronix.com/scan.php?page=news_item&px=Straight-Line-Speculation-x86), [x86 Straight Line Speculation CPU Mitigation Appears For Linux 5.17](https://www.phoronix.com/scan.php?page=news_item&px=x86-SLS-Mitigation-5.17)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/10 | Anup Patel <anup.patel@wdc.com> | [ARM: Implement Clang's SLS mitigation](https://patchwork.kernel.org/project/linux-security-module/patch/20210212051500.943179-1-jiancai@google.com) | NA | v8 ☐ | [2021/06/10Patchwork v7,0/8](https://patchwork.kernel.org/project/linux-riscv/cover/20210830041729.237252-1-anup.patel@wdc.com)<br>*-*-*-*-*-*-*-*<br>[LWN v8, 0/8](https://lwn.net/Articles/872513) |
| 2021/12/04 | Peter Zijlstra <peterz@infradead.org> | [x86: Add stright-line-speculation mitigations](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e463a09af2f0677b9485a7e8e4e70b396b2ffb6f) | 20211204134338.760603010@infradead.org | v2 ☑✓ 5.17-rc1 | [LORE v2,0/6](https://lore.kernel.org/all/20211204134338.760603010@infradead.org) |


## 4.2 Control-Flow Enforcement Technology (CET)
-------


## 4.3 branch history injection
-------

[The "branch history injection" hardware vulnerability](https://lwn.net/Articles/969210)

[Linux Kernel Patched For Branch History Injection "BHI" Intel CPU Vulnerability](https://www.phoronix.com/news/Linux-BHI-Branch-History-Inject)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2024/04/05 | Josh Poimboeuf <jpoimboe@kernel.org> | [x86/bugs: Change commas to semicolons in 'spectre_v2' sysfs file](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=2bb69f5fc72183e1c62547d900f560d0e9334925) | TODO | v2 ☐☑✓ | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2bb69f5fc72183e1c62547d900f560d0e9334925) |


## 4.X 安全框架层
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2023/11/21 | Breno Leitao <leitao@debian.org> | [x86/bugs: Add a separate config for each mitigation](https://lore.kernel.org/all/20231121160740.1249350-1-leitao@debian.org) | CONFIG_SPECULATION_MITIGATIONS 相关的 MITIGATION 特性进行了细粒度控制. <br>1. 用户可以只选择对其工作负载很重要的缓解措施. <br>2. 用户和开发人员可以选择禁用破坏程序集代码生成、使其难以读取的缓解措施.<br>3. 可读性更强, 所有特性都整改为带明显的 MITIGATION 前缀. 参见 [Linux 6.9 Making It Easier Managing Security Mitigation Options](https://www.phoronix.com/news/Linux-6.9-CONFIG-MITIGATIONS). | v6 ☐☑✓ | [LORE v6,0/13](https://lore.kernel.org/all/20231121160740.1249350-1-leitao@debian.org) |

# 5 benchmark
-------


用于评价 CPU 性能指标的标准主要有三种: Dhrystone、MIPS、CoreMark.

20 世纪 70-80 年代开始流行的几个性能测试标准, 它们分别是 Livermore、Whetstone、Linpack、Dhrystone, 这四个性能测试标准也被合称为 Classic Benchmark. 这个网址简单介绍了四大经典性能测试标准历史 http://www.roylongbottom.org.uk/classic.htm.

[Magisk-Modules-Repo/benchkit](https://github.com/Magisk-Modules-Repo/benchkit) 项目汇总整理了 Android/Linux 上非常多的 benchmarks

[Linux Benchmark Suite Homepage](http://lbs.sourceforge.net)


# 6 通用
-------

## 6.1 SYSCALL
-------

[remove in-kernel calls to syscalls 000/109](https://lore.kernel.org/all/20180329112426.23043-1-linux@dominikbrodowski.net)


基于 pt_regs 传递  syscall 的参数, 防止在 syscall 的调用链中泄漏随机的用户提供的寄存器内容.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/04/05 | Dominik Brodowski <linux@dominikbrodowski.net> | [use struct pt_regs based syscall calling for x86-64](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d5a00528b58cdb2c71206e18bd021e34c4eab878) | NA | v1 ☑ 4.17-rc1 | [LORE 0/7](https://lore.kernel.org/all/20180330093720.6780-1-linux@dominikbrodowski.net), [LORE v3,0/8](https://lore.kernel.org/all/20180405095307.3730-1-linux@dominikbrodowski.net) |
| 2018/07/11 | Mark Rutland <mark.rutland@arm.com> | [arm64: invoke syscalls with pt_regs](https://patchwork.kernel.org/project/linux-security-module/patch/20210212051500.943179-1-jiancai@google.com) | NA | v7 ☐ | [2021/06/10Patchwork v5,00/21](https://lore.kernel.org/lkml/20180711135656.20670-1-mark.rutland@arm.com) |

FlexSC 实现异步 syscall.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/02/14 | foxhoundsk <dongzhiyan_linux@163.com> | [Implementation of FlexSC on Linux Kernel v5.0+ and Performance Analysis](https://github.com/foxhoundsk/FlexSC) | , 参见 [FlexSC: Flexible System Call Scheduling with Exceptio[n-Less System Calls](https://www.usenix.org/conference/osdi10/flexsc-flexible-system-call-scheduling-exception-less-system-calls). [Paper](http://www.usenix.org/events/osdi10/tech/full_papers/Soares.pdf), [Slides](http://www.usenix.org/events/osdi10/tech/slides/soares.pdf) | v1 ☐ | [github](https://github.com/foxhoundsk/FlexSC) |
| 2020/06/13 | c-blake | [batch: Generic Linux System Call Batching](https://github.com/c-blake/batch) | [一种以减少处理器模式切换的发生次数来提升 syscall 执行效率的方法](https://hackmd.io/@foxhoundsk/B1SgjQ8hw) | [github](https://github.com/c-blake/batch) |


## 6.2 RANOM
-------


### 6.2.1 Arm True Random Number Generator Firmware Interface
-------

ARM 规范 [DEN0098](https://developer.arm.com/documentation/den0098/latest) 中描述的 ARM 架构的 TRNG 固件接口定义了一个基于 ARM SMCCC 的接口, 该接口由固件提供给真实随机数生成器.

Arm True Random Number Generator Firmware Interface 1.0 于去年发布, 最终由 Arm TrustZone 的 TRNG 或其他原始噪声等硬件设备
提供支持.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/12/11 | Vladimir Murzin <vladimir.murzin@arm.com> | [ARM: arm64: Add SMCCC TRNG entropy service](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=38db987316a38a3fe55ff7f5f4653fcb520a9d26) | NA | v4 ☑ 5.12-rc1 | [LORE v4,0/5](https://lore.kernel.org/lkml/20201211160005.187336-1-andre.przywara@arm.com) |
| 2021/07/26 | Vladimir Murzin <vladimir.murzin@arm.com> | [hwrng: Add Arm SMCCC TRNG based driver](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0888d04b47a165ae8c429c6fe11b3c43f5017f31) | 添加一个 "arm_smccc_trng" 驱动程序, 并允许将熵暴露给用户空间. ARM 规范定义了此真正的随机数生成器固件接口, 用于为随机池设定种子, 也可由 KVM Guest 使用. 使用此新驱动程序, 可以通过 `/dev/hwrng` 公开来自此固件接口的熵. 反过来, 这对于能够使用 rng-tool 的 rngtest 实用程序等来验证熵的质量非常有用. 参见 [Arm SMCCC TRNG Driver Queued Ahead Of Linux 5.15](https://www.phoronix.com/scan.php?page=news_item&px=Arm-SMCCC-TRNG-Linux-5.15). | v3 ☑ 5.15_rc1 | [LKML v3,0/2](https://lkml.org/lkml/2021/7/26/1571), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0888d04b47a165ae8c429c6fe11b3c43f5017f31) |


### 6.2.2 LRNG
-------

[FIPS-compliant random numbers for the kernel](https://lwn.net/Articles/877607)

[smuellerDD/lrng](https://github.com/smuellerDD/lrng)

[Uniting the Linux random-number devices](https://lwn.net/Articles/884875)

[Random number generator enhancements for Linux 5.17 and 5.18](https://www.zx2c4.com/projects/linux-rng-5.17-5.18)

[Linux's RNG Code Continues Modernization Effort With v5.19](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.19-RNG)

[Linux To Try To Opportunistically Initialize /dev/urandom](https://www.phoronix.com/scan.php?page=news_item&px=Linux-RNG-Opportunistic-urandom)

[Linux's getrandom() Sees A 8450% Improvement With Latest Code](https://www.phoronix.com/scan.php?page=news_item&px=Linux-getrandom-8450p)

[Linux 6.0 To Continue Advancing Its Random Number Generator (RNG)](https://www.phoronix.com/news/Linux-6.0-RNG)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/21 | "Stephan Müller" <smueller@chronox.de> | [/dev/random - a new approach](https://lore.kernel.org/lkml/2036923.9o76ZdvQCi@positron.chronox.de) | 随机数实现改进. | v1 ☐ | [Patchwork v43 00/15](https://lore.kernel.org/lkml/2036923.9o76ZdvQCi@positron.chronox.de) |
| 2022/02/23 | Jason A. Donenfeld <Jason@zx2c4.com> | [VM fork detection for RNG](https://lore.kernel.org/all/20220223220456.666193-1-Jason@zx2c4.com) | [Linux RNG Improvements Aim For Better VM Security](https://www.phoronix.com/scan.php?page=news_item&px=Linux-RNG-VM-Forks) | v2 ☐☑✓ | [LORE v2,0/2](https://lore.kernel.org/all/20220223220456.666193-1-Jason@zx2c4.com) |

### 6.2.3 A RDRAND followup
-------

[A RDRAND followup](https://lwn.net/Articles/963281)


## 6.3 总线
-------

### 6.3.1 Compute Express Link
-------

FireBox: Warehouse-Scale Computers
[FireBox: A Hardware Building Block for 2020 Warehouse-Scale Computers](https://www.usenix.org/conference/fast14/technical-sessions/presentation/keynote)

[Flash and Chips" The FireBox Warehouse-Scale Compute](https://www.fujitsu.com/us/imagesgig5/Panel4_Krste_Asanovic.pdf)


[FireBox](https://bar.eecs.berkeley.edu/projects/firebox.html)


The Machine:HPE Memory driven compute
https://www.hc32.hotchips.org/assets/program/conference/day1/HotChips2020_Server_Processors_IBM_Starke_POWER10_v33.pdf
https://www.nextplatform.com/2020/09/03/the-memory-area-network-at-the-heart-of-ibms-power10/

Memory Cluster IBM

VMware Project Capitola
https://www.servethehome.com/vmware-project-capitola-enables-local-and-cluster-memory-tiering/#:~:text=First%20off%2C%20Capitola%20is%20a%20beach%20town%20just,other%20side%20of%20the%20Monterey%20bay%20from%20Capitola.

https://blogs.vmware.com/vsphere/2021/10/introducing-project-capitola.html

[Dbus-Broker 30 Released For High Performance Linux Message Bus](https://www.phoronix.com/scan.php?page=news_item&px=Dbus-Broker-30), [bus1/dbus-broker](https://github.com/bus1/dbus-broker)


### 6.3.2 CXL
-------

[phoronix 上关于 CXL(Compute Express Link) 的所有相关报道](https://www.phoronix.com/scan.php?page=search&q=Compute%20Express%20Link)


英特尔工程师 Ben Widawsky 已经开始发布一个关于 Linux 上 CXL 的博客文章系列, 参见 [Compute Express Link Overview](https://bwidawsk.net/blog/2022/6/compute-express-link-intro)

[公众号 - 半导体行业观察 - 越来越热的 CXL](https://mp.weixin.qq.com/s/sB2bmFcEaYsH1Jg19E0-eg)

[CXL 3.0 Specification Released - Doubles The Data Rate Of CXL 2.0](https://www.phoronix.com/news/CXL-3.0-Specification-Released)

[kernel cxl.git](https://git.kernel.org/pub/scm/linux/kernel/git/cxl/cxl.git)

#### 6.3.2.1 CXL Support
-------

[Linux 6.0 Continues Plumbing For Compute Express Link (CXL)](https://www.phoronix.com/news/Linux-6.0-CXL)


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2021/02/16 | Ben Widawsky <ben.widawsky@intel.com> | [CXL 2.0 Support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=68a5a9a243354ed52f7b37b057bd5e98cba870c8) | TODO | v5 ☐☑✓ | [LORE v3,00/16](https://lore.kernel.org/lkml/20210111225121.820014-1-ben.widawsky@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/9](https://lore.kernel.org/all/20210217040958.1354670-1-ben.widawsky@intel.com) |
| 2022/08/12 | Jonathan Cameron <Jonathan.Cameron@huawei.com> | [CXL 3.0 Performance Monitoring Unit support](https://lore.kernel.org/all/20220812151214.2025-1-Jonathan.Cameron@huawei.com) | CXL rev 3.0 规范引入了一个 CXL 性能监控单元定义. CXL 组件可以有任意数量的这样的块. 定义是高度灵活的, 但这也给驱动程序带来了复杂性. | v1 ☐☑✓ | [LORE v1,0/4](https://lore.kernel.org/all/20220812151214.2025-1-Jonathan.Cameron@huawei.com) |


#### 6.3.2.2 CXL RAM
-------

[CXL RAM Regions Being Worked On For Linux 6.3](https://www.phoronix.com/news/CXL-RAM-Region-Linux-6.3)

[Intel Has More CXL Improvements Ready For Linux 6.3](https://www.phoronix.com/news/Linux-6.3-CXL)

[Memory-management changes for CXL](https://lwn.net/Articles/931416)


#### 6.3.2.3 CXL Devices
-------


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2024/03/24 | ira.weiny@intel.com <ira.weiny@intel.com> | [DCD: Add support for Dynamic Capacity Devices (DCD)](https://lore.kernel.org/all/20240324-dcd-type2-upstream-v1-0-b7b00d623625@intel.com) | 动态容量设备 (DCD)(CXL 3.1 sec 9.13.3) 是一种 CXL 存储器设备, 它允许存储器容量动态变化, 而无需重置设备、重新配置 HDM 解码器或重新配置软件 DAX 区域. 动态容量最大的使用案例之一是允许主机在数据中心内动态共享内存, 而不增加每台主机连接的内存. 添加或删除内存的一般流程是让协调器协调内存的使用. 通常, 在这样的系统中有 5 个参与者, 即编排器、结构管理器、主机看到的设备、主机内核和主机用户. | v1 ☐☑✓ | [LORE v1,0/26](https://lore.kernel.org/all/20240324-dcd-type2-upstream-v1-0-b7b00d623625@intel.com) |


## 6.4 CPU IDLE(C-state)
-------

[AMD Updates Linux Patches For Lowering Idle Exit Latency](https://www.phoronix.com/scan.php?page=news_item&px=AMD-Prefer-MWAIT-v3)

[Linux 5.20 With AMD Zen Will Prefer MWAIT Over HALT As An HPC Optimization](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.20-AMD-MWAIT)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/04/05 | Wyes Karny <wyes.karny@amd.com> | [x86: Prefer MWAIT over HALT on AMD processors](https://lore.kernel.org/all/20220405130021.557880-1-wyes.karny@amd.com) | Monitor Wait "MWAIT" 指令可用于电源管理目的, 以提示处理器在等待事件或 MONITOR 存储操作完成时可以进入指定的目标 C 状态. MWAIT 的使用旨在比 HALT 指令更有效. Intel Core 2 及更高版本已经将 MWAIT 置于 HALT 指令之上, 以进入 C1 状态. 但是 AMD CPU 此代码路径中依旧使用 HALT. 这个补丁使 AMD 使用 MWAIT 指令而不是 HALT, 当系统未使用 CPU 空闲驱动程序时, 无论是 BIOS 禁用 C 状态还是驱动程序不属于内核构建. 反过来, 这可以将受影响系统的退出延迟提高约 21%. 参见 [](https://www.phoronix.com/scan.php?page=news_item&px=AMD-MWAIT-Over-HALT-2022). | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20220405130021.557880-1-wyes.karny@amd.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/3](https://lore.kernel.org/lkml/cover.fba143c82098dffab6bbf0a2f3c4be8bae07ccf1.1652176835.git-series.wyes.karny@amd.com/) |


## 6.5 memory model
-------


### 6.5.1 barrier
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/07/08 | Jiri Olsa <jolsa@redhat.com> | [memory barrier: adding smp_mb__after_lock](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ad46276952f1af34cd91d46d49ba13d347d56367) | TODO | v1 ☑✓ 2.6.31-rc3 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ad46276952f1af34cd91d46d49ba13d347d56367) |
| 2013/08/12 | Oleg Nesterov <oleg@redhat.com> | [sched: fix the theoretical signal_wake_up() vs schedule() race](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e0acd0a68ec7dbf6b7a81a87a867ebd7ac9b76c4) | 在 try_to_wake_up 路径引入了 smp_mb__before_spinlock(), 入口位置的 smp_wmb() 就被替换为 smp_mb__before_spinlock(). | v1 ☑✓ v3.11-rc6 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e0acd0a68ec7dbf6b7a81a87a867ebd7ac9b76c4) |
| 2017/08/02 | Peter Zijlstra <peterz@infradead.org> | [Getting rid of smp_mb__before_spinlock](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ae813308f4630642d2c1c87553929ce95f29f9ef) | 本系列删除了 smp_mb_before_spinlock() 用户, 并将调度路径下转换为使用 smp_mb_after_spinlock(), 从而在相同数量的障碍下提供更多保障. | v1 ☑✓ 4.14-rc1 | [LORE v1,0/4](https://lore.kernel.org/all/20170802113837.280183420@infradead.org) |
| 2017/08/07 | Byungchul Park <byungchul.park@lge.com> | [lockdep: Implement crossrelease feature](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ef0758dd0fd70b98b889af26e27f003656952db8) | 1502089981-21272-1-git-send-email-byungchul.park@lge.com | v8 ☑✓ 4.14-rc1 | [LORE v8,0/14](https://lore.kernel.org/all/1502089981-21272-1-git-send-email-byungchul.park@lge.com) |
| 2018/07/16 | Paul E. McKenney <paulmck@linux.vnet.ibm.com> | [Updates to the formal memory model](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=71b7ff5ebc9b1d5aa95eb48d6388234f1304fd19) | NA | v1 ☑✓ 4.19-rc1 | [LORE v1,0/14](https://lore.kernel.org/all/20180716180540.GA14222@linux.vnet.ibm.com) |


### 6.5.3 内存一致性问题(memory consistency)
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2024/04/10 | Zayd Qumsieh <zayd_qumsieh@apple.com> | [tso: aarch64: Expose TSO for virtualized linux on Apple Silicon](https://lore.kernel.org/all/20240410211652.16640-1-zayd_qumsieh@apple.com) | x86 CPU 使用 TSO 内存模型. Apple Silicon CPU 能够选择性地使用 TSO 内存模型. 这可以通过设置 ACTLR 来完成. TSOEN 位为 1. 此功能对 x86 仿真器非常有用, 因为它消除了仿真器插入内存屏障以遵守的需要. 通过 TSO 存储器模型. 此补丁系列将添加 ACTLR. TSOEN 支持 Apple Silicon 机器上的虚拟化 linux. 用户空间将能够使用 prctl 将 CPU 的内存模型从默认的 ARM64 内存模型更改为 TSO 内存模型. 可以使用一个简单的测试来确定 TSO 内存模型是否正在使用中. 这必须在 Apple Silicon MacOS Sonoma 14.4 版或更高版本上完成, 因为早期版本不支持修改 TSOEN 位. 用例程序参见 [TSOEnabler](https://github.com/saagarjha/TSOEnabler/blob/master/testtso/main.c), 如果 TSO 正在使用, 此程序将无限期挂起, 如果不使用, 则几乎立即崩溃. | v1 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20240410211652.16640-1-zayd_qumsieh@apple.com) |


## 6.6 objtool
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/04/29 | Chen Zhongjin <chenzhongjin@huawei.com> | [objtool: add base support for arm64](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20220429094355.122389-1-chenzhongjin@huawei.com/) | 636883 | v4 ☐☑ | [LORE v4,0/37](https://lore.kernel.org/r/20220429094355.122389-1-chenzhongjin@huawei.com) |


## 6.7 指令转译
-------

Rosetta 是一个转译过程, 允许用户在 Apple Silicon 上运行包含 x86_64 指令的应用程序. 在 macOS 中, 这允许为基于英特尔的 Mac 电脑构建的应用程序在 Apple Silicon 上无缝运行; Rosetta 可以在 ARM Linux 虚拟机中为英特尔 Linux 应用程序提供同样的功能.

[macOS 13 Adding Ability To Use Rosetta In ARM Linux VMs For Speedy x86_64 Linux Binaries](https://www.phoronix.com/scan.php?page=news_item&px=macOS-13-Rosetta-Linux-Binaries)





## 6.8 原子操作
-------

[ARMV8 datasheet 学习笔记 3：AArch64 应用级体系结构之 Atomicity](https://www.cnblogs.com/smartjourneys/p/6843978.html)

[ARMv8 之 Atomicity](http://www.wowotech.net/?post=295)

## 6.10 芯片设计
-------

Tachyum 宣布其设计一款完全通用的处理器 Prodigy T16128, 预计 2023 年发布, [Tachyum's Monster 128 Core 5.7GHz 'Universal Processor' Does Everything](https://www.tomshardware.com/news/tachyum-128-core-all-purpose-cpu), 号称一款芯片上可以同时运行通用计算, 高性能计算以及 AI 等业务和负载, 原生支持 x86, ARM, RISC-V 和 ISA 的二进制.

Google Google 推出 [芯片设计门户网站](https://developers.google.com/silicon), 计划名为 Open MPW Shuttle Program, 允许任何人利用开源 PDK 和其他开源 EDA 工具来提交开源集成电路设计, Google 会为他们免费制造, 不会收取任何费用. 虽然芯片制造是在 130 纳米工艺(SKY130) 上完成的, 但这一计划对资金有限的开源硬件项目具有巨大的推动作用.

[GlobalFoundries Partners With Google's Open-Source Silicon Effort To Provide 180nm Tech](https://www.phoronix.com/news/Google-GloFo-GF180MCU)


中国科学院大学 ("国科大") 的 ["一生一芯" 计划](https://ysyx.org).


### 6.10.1 预取
-------

富士通添加了 sysfs 接口来控制 CPU L2 Cache/DCU 等硬件的预取行为, 以便从用户空间对 A64FX 处理器和 x86 行性能调优.

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/06/07 | Kohei Tarumizu <tarumizu.kohei@fujitsu.com> | [Add hardware prefetch control driver for A64FX and x86](https://lore.kernel.org/all/20220607120530.2447112-1-tarumizu.kohei@fujitsu.com) | TODO | v5 ☐☑✓ | [LORE v5,0/6](https://lore.kernel.org/all/20220607120530.2447112-1-tarumizu.kohei@fujitsu.com) |


openEuler 提供了 [openEuler/prefetch_tuning](https://gitee.com/openeuler/prefetch_tuning) 提供了鲲鹏芯片设计的渔区相关寄存器读写接口, 用于读取和配置在 CPU 的硬件层面的芯片性能调优参数. 内核中更是提供了 [CONFIG_HISILICON_ERRATUM_HIP08_RU_PREFETCH](https://gitee.com/openeuler/kernel/commit/13ab4b7fa6f92eb9819a01129c4e4a0a9c401ee8) 来在启动时配置预期.


### 6.10.2 Software Branch Hinting
-------

[Software Branch Hinting](https://labs.engineering.asu.edu/mps-lab/research-themes/low-power-computing/sbh)


### 6.10.3 指令集架构
-------


| 指令集架构 | 描述 | 代表架构 |
|:--------:|:----:|:------:|
| CISC (Complex instruction set computer) 复杂指令集计算机 | NA | HP 的 PA-RISC, IBM 的 PowerPC, Compaq(被并入 HP)的 Alpha, MIPS 公司的 MIPS, SUN 公司的 SPARC 等. |
| RISC (Reduced instruction set computer) 精简指令集计算机 | NA | NA |
| [MISC (Minimal instruction set computer), 最小指令集计算机](http://en.wikipedia.org/wiki/Minimal_instruction_set_computer) | [从零开始手敲自举编译器(一): MISC 概览](https://zhuanlan.zhihu.com/p/412201989) 和 [github-whoiscc/miniboot/](https://github.com/whoiscc/miniboot) |
| [OISC (One instruction set computer, 单指令集计算机)](http://en.wikipedia.org/wiki/One_instruction_set_computer) | NA | NA |
| [ZISC (Zero instruction set computer)](https://en.wikipedia.org/wiki/Zero_instruction_set_computer) | NA | NA |
| VLIM (Very long instruction word) 超长指令字架构 | 通过将多条指令放入一个指令字, 有效的提高了 CPU 各个计算功能部件的利用效率, 提高了程序的性能. | NA |
| EPIC (Explicity parallel instruction computing) 显示并行指令集计算 | NA |  Intel 的 IA-64 |
| EDGE | 显式数据图执行 (Explicit Data Graph Execution) 的指令集体系结构, 也被称为 EDGE 架构. | [微软处理器架构是新瓶装酒 历史早已证明是死路一条](http://www.360doc.com/content/18/0623/10/22587800_764605025.shtml), [与高通联手打造全新处理器架构, 微软计算芯片界的至尊魔戒终浮现](https://zhuanlan.zhihu.com/p/38340253) | Microsoft 的 E2 |


## 6.11 bad CPUs
-------


[In-Field Scan](https://www.phoronix.com/search/In-Field%20Scan)

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/03/01 | Jithu Joseph <jithu.joseph@intel.com> | [Introduce In Field Scan driver](https://lore.kernel.org/all/20220301195457.21152-1-jithu.joseph@intel.com) | [Intel"In-Field Scan"Coming With Sapphire Rapids As New Silicon Failure Testing Feature](https://www.phoronix.com/news/Intel-In-Field-Scan) | v1 ☐☑✓ | [LORE v1,0/10](https://lore.kernel.org/all/20220301195457.21152-1-jithu.joseph@intel.com) |


[Linux 6.1 Will Make It A Bit Easier To Help Spot Faulty CPUs](https://www.phoronix.com/news/Linux-6.1-Seg-Fault-Report-CPU)

[Linux 6.1 Will Try To Print The CPU Core Where A Seg Fault Occurs](https://www.phoronix.com/news/Linux-6.1-Seg-Fault-Print-CPU)

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/08/05 | Rik van Riel <riel@surriel.com> | [x86,mm: print likely CPU at segfault time](https://lore.kernel.org/all/20220805101644.2e674553@imladris.surriel.com) | 内核将尝试打印发生 SEG 错误的 CPU 核, 通过在发生 SEG 故障的地方打印 CPU 核, 这些信息可能有助于发现故障 CPU 而成为现实. | v3 ☐☑✓ | [LORE](https://lore.kernel.org/all/20220805101644.2e674553@imladris.surriel.com) |


## 6.12 P-State
-------


[phoronix-P-State EPP Linux & Open-Source News](https://www.phoronix.com/search/P-State%20EPP)

### 6.12.1 Intel
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2023/02/01 | Srinivas Pandruvada <srinivas.pandruvada@linux.intel.com> | [Add TPMI support](https://lore.kernel.org/all/20230202010738.2186174-1-srinivas.pandruvada@linux.intel.com) | Intel Topology Aware Register and PM Capsule Interface | v1 ☐☑✓ | [LORE v1,0/7](https://lore.kernel.org/all/20230202010738.2186174-1-srinivas.pandruvada@linux.intel.com) |

### 6.12.2 AMD
-------

#### 6.12.2.1 AMD P-State Driver
-------

[How To Use The New AMD P-State Driver With Linux 5.17](https://www.phoronix.com/news/AMD-P-State-How-To)
[AMD Making It Easier To Switch To Their New P-State CPU Frequency Scaling Driver](https://www.phoronix.com/news/AMD-Easier-P-State-Usage)
[AMD P-State EPP Driver Updated For More Power/Performance Control On Linux](https://www.phoronix.com/news/AMD-P-State-EPP-v4)
[New Patches Allow More Easily Managing The AMD P-State Linux Driver](https://www.phoronix.com/news/AMD-P-State-Built-In-Options)
[Linux 6.1-rc7 Makes It Easier To Manage The AMD P-State Driver](https://www.phoronix.com/news/Linux-6.1-rc7-Easier-AMD-Pstate)
[AMD P-State EPP Submitted For Linux 6.3 To Improve CPU Performance/Power](https://www.phoronix.com/news/Linux-6.3-Power-Management)

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2021/12/24 | Huang Rui <ray.huang@amd.com> | [cpufreq: Introduce a new AMD CPU frequency control mechanism](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=38fec059bb69793f38cfa7a671d4bdbfe2a647aa) | TODO |v7 ☐☑✓ | [LORE v7,0/14](https://lore.kernel.org/all/20211224010508.110159-1-ray.huang@amd.com) |
| 2022/11/11 | Perry Yuan <Perry.Yuan@amd.com> | [Implement AMD Pstate EPP Driver](tps://lore.kernel.org/lkml/20221219064042.661122-1-perry.yuan@amd.com) | [AMD P-State EPP Patches Spun An 8th Time For Helping Out Linux Performance & Efficiency](https://www.phoronix.com/news/AMD-P-State-EPP-v8) | v4 ☐☑✓ 5.17-rc1 | [LORE v4,0/9](https://lore.kernel.org/all/20221110175847.3098728-1-Perry.Yuan@amd.com)<br>*-*-*-*-*-*-*-* <br>[LORE v8,00/13](https://lore.kernel.org/lkml/20221219064042.661122-1-perry.yuan@amd.com) |
| 2022/03/25 | Mario Limonciello <mario.limonciello@amd.com> | [Improve usability for amd-pstate](https://lore.kernel.org/all/20220325054228.5247-1-mario.limonciello@amd.com) | TODO | v1 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20220325054228.5247-1-mario.limonciello@amd.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/6](https://lore.kernel.org/linux-pm/20220414164801.1051-1-mario.limonciello@amd.com) |
| 2023/01/13 | Wyes Karny <wyes.karny@amd.com> | [amd_pstate: Add guided autonomous mode support](https://lore.kernel.org/all/20230113052141.2874296-1-wyes.karny@amd.com) | [AMD Updates P-State"Guided Autonomous Mode"Support For Linux](https://www.phoronix.com/news/AMD-Guided-Auto-Mode-v2) | v2 ☐☑✓ | [LORE v2,0/6](https://lore.kernel.org/all/20230113052141.2874296-1-wyes.karny@amd.com) |
| 2024/01/30 | Perry Yuan <perry.yuan@amd.com> | [enable x86_energy_perf_policy for AMD CPU](https://lore.kernel.org/all/cover.1706583551.git.perry.yuan@amd.com) | 对 AMD 处理器上 x86_energy_perf_policy 实用程序的支持, 这些处理器使用 CPPC(Collaborative Processor Performance Control) 接口进行频率缩放, 并使用 AMD_state 驱动程序模块. AMD 处理器已经支持各种能源性能偏好 EPP(Energy Performance Preference) 配置文件. 有了这个实用程序, 用户现在可以使用提供的命令在这些 EPP 配置文件之间无缝切换. 并使用此工具检查 CPPC 的功能. 此增强旨在提高 AMD 处理器的电源效率和性能管理, 为用户提供对系统能源性能行为的更多控制. 参见 [Linux's x86_energy_perf_policy Utility Being Extended To AMD CPUs](https://www.phoronix.com/news/AMD-x86_energy_perf_policy) | v1 ☐☑✓ | [LORE v1,0/8](https://lore.kernel.org/all/cover.1706583551.git.perry.yuan@amd.com) |


#### 6.12.2.2 AMD P-State Preferred Core
-------

[AMD P-State Preferred Core Patches For Linux Updated, Will Be Enabled By Default](https://www.phoronix.com/news/AMD-Preferred-Core-Linux-v2)

[AMD P-State Preferred Core Support For Linux Tried A 13th Time](https://www.phoronix.com/news/AMD-P-State-Preferred-Core-13)

[AMD P-State Preferred Core Support Coming With Linux 6.9](https://www.phoronix.com/news/Linux-6.9-AMD-P-State-Preferred)


磁芯频率受半导体工艺变化的影响. 并非所有内核都能够达到最大频率, 具体取决于基础架构限制. 因此, AMD 重新定义了硬件最大频率的概念. 这意味着一小部分内核可以达到最大频率. 为了找到给定场景的最佳进程调度策略, 操作系统需要通过 CPPC 接口的最高性能功能寄存器来了解平台通知的核心排序.

AMD-pstate 首选核心的早期实现仅支持静态核心排名和目标性能. 现在, 它能够根据工作负载和平台条件动态更改首选核心, 并考虑到热量和老化.

AMD-pstate 驱动程序利用 ITMT 体系结构提供的功能和数据结构, 使调度器倾向于在内核上进行调度, 从而可以以较低的电压获得更高的频率. 我们称它为 "首选核心(AMD-pstate preferred core)".

1. 通过调用 sched_set_itmt_core_prio() 来设置优先级, 调用 sched_set_itmt_support() 来启用 ITMT 特性. AMD-pstate 驱动使用最高的性能值来表示 CPU 的优先级. 数值越高优先级越高.

2. AMD-pstate 驱动程序将在引导时提供初始核心排序. 它依靠 CPPC 接口将内核排名传递给操作系统和调度器, 以确保操作系统首先选择性能最高的内核来调度进程. 当 AMD-pstate 驱动程序接收到具有最高性能变化的消息时, 它将更新核心排名.


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2023/08/15 | Meng Li <li.meng@amd.com> | [amd-pstate preferred core](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=dfddf34a3f0d45483f5b3e46c2e7bda173796f1b) | [AMD Pstate Preferred Core](https://lore.kernel.org/all/20230815061546.3556083-1-li.meng@amd.com) 以及 [AMD P-State Preferred Core Submitted For Linux 6.9 While Intel Meteor Lake Gets Tuned](https://www.phoronix.com/news/AMD-P-State-Preferred-Core-69) | v2 ☐☑✓ 6.9-rc1 | [LORE v2,0/7](https://lore.kernel.org/all/20230815061546.3556083-1-li.meng@amd.com)<br>*-*-*-*-*-*-*-* <br>[LORE v13,0/7](https://lore.kernel.org/all/20240112092531.789841-1-li.meng@amd.com)<br>*-*-*-*-*-*-*-* <br>[LORE v14,0/7](https://lore.kernel.org/lkml/20240119090502.3869695-1-li.meng@amd.com) |
| 2024/03/18 | Perry Yuan <perry.yuan@amd.com> | [AMD Pstate Driver Core Performance Boost](https://lore.kernel.org/all/cover.1710754236.git.perry.yuan@amd.com) | 该补丁集系列为 AMD pstate 驱动程序增加了核心性能提升 (core performance boost) 功能, 包括被动, 引导和主动模式支持. 用户可以使用新的 sysfs 条目: "/sys/devices/system/cpu/amd_pstate/cpb_boost" 更改核心频率 boost 控制. 由于与支持所有模式的新 cpb_boost 的功能冲突, 传统的 boost 接口已被删除. 参见 [AMD Posts Updated Linux Patches For P-State Core Performance Boost](https://www.phoronix.com/news/AMD-Core-Performance-Boost-6). | v6 ☐☑✓ | [LORE v6,0/6](https://lore.kernel.org/all/cover.1710754236.git.perry.yuan@amd.com) |
| 2024/03/08 | Sibi Sankar <quic_sibis@quicinc.com> | [cpufreq: scmi: Add boost frequency support](https://lore.kernel.org/all/20240308104410.385631-1-quic_sibis@quicinc.com) | [ARM SCMI CPUFreq Driver Enabling Boost Support By Default With Linux 6.9](https://www.phoronix.com/news/ARM-SCMI-CPUFreq-Boost-Linux-69). | v3 ☐☑✓ | [LORE v3,0/2](https://lore.kernel.org/all/20240308104410.385631-1-quic_sibis@quicinc.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/2](https://www.phoronix.com/news/Linux-69-RAM-Bandwidth-Throttle) |


## 6.13 Device
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2023/07/10 | Mina Almasry <almasrymina@google.com> | [Device Memory TCP](https://lore.kernel.org/all/20230710223304.1174642-1-almasrymina@google.com) | 通过该方案, 我们能够通过直接从设备存储器发送和接收数据, 达到约 96.6% 的线路速率. 参见 [Google Posts Experimental Linux Code For"Device Memory TCP"- Network To/From Accelerator RAM](https://www.phoronix.com/news/Linux-Device-Memory-TCP). | v1 ☐☑✓ | [LORE v1,0/10](https://lore.kernel.org/all/20230710223304.1174642-1-almasrymina@google.com) |


## 6.14 Cache 带宽管控
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2023/08/29 | Tony Luck <tony.luck@intel.com> | [Add support for Sub-NUMA cluster (SNC) systems](https://lore.kernel.org/all/20230829234426.64421-1-tony.luck@intel.com) | [Intel Fixing Up Sub-NUMA Clustering For Linux So That It Behaves With RDT](https://www.phoronix.com/news/Intel-SNT-RDT-Fixing-Up) | v5 ☐☑✓ | [LORE v5,0/8](https://lore.kernel.org/all/20230829234426.64421-1-tony.luck@intel.com) |
| 2023/12/07 | Tony Luck <tony.luck@intel.com> | [x86/resctrl: mba_MBps enhancements](https://lore.kernel.org/all/20231207195613.153980-1-tony.luck@intel.com) | TODO | v6 ☐☑✓ | [LORE v6,0/3](https://lore.kernel.org/all/20231207195613.153980-1-tony.luck@intel.com) |
| 2024/02/13 | James Morse <james.morse@arm.com> | [x86/resctrl: monitored closid+rmid together, separate arch/fs locking](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/log/?id=fb700810d30b9eb333a7bf447012e1158e35c62f) | [Improved Memory Bandwidth Throttling Behavior For Linux 6.9](https://www.phoronix.com/news/Linux-69-RAM-Bandwidth-Throttle) | v9 ☐☑✓ 6.9-rc1 | [LORE v9,0/24](https://lore.kernel.org/all/20240213184438.16675-1-james.morse@arm.com) |
| 2024/03/21 | James Morse <james.morse@arm.com> | [x86/resctrl: Move the resctrl filesystem code to /fs/resctrl](https://lore.kernel.org/all/20240321165106.31602-1-james.morse@arm.com) | TODO | v1 ☐☑✓ | [LORE v1,0/31](https://lore.kernel.org/all/20240321165106.31602-1-james.morse@arm.com) |



<br>

*   本作品 / 博文 ([AderStep - 紫夜阑珊 - 青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a><a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议</a> 进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚 gatieme](http://blog.csdn.net/gatieme) (包含链接: http://blog.csdn.net/gatieme), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   ** 转载请务必注明出处, 谢谢, 不胜感激 **
<br>
