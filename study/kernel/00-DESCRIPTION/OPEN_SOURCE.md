# 1 ANDROID
-------

[Android Kernel Features](https://elinux.org/Android_Kernel_Features)

[各 Android 手机厂商 Bootloader 解锁 / 内核开源 / 解锁后保修情况](https://github.com/gatieme/MobileModels/blob/master/misc/bootloader-kernel-source.md)

[移动芯片性能排行榜-天梯图](https://www.socpk.com)

[LIST OF ALL MANUFACTURERS OF CHIPSETS FOR SMARTPHONES](https://phonesdata.com/en/chipset)

| 公司 | github | MAIN |
|:---:|:------:|:----:|
| GOOGLE | [googlesource](https://android.googlesource.com/kernel/common) | NA |
| QCOM | [source.codeaurora](https://source.codeaurora.org/quic/la/kernel), [codelinaro](https://git.codelinaro.org/clo/la/kernel) | Code Aurora 是 Linux 基金会的一个协作项目, 为 QCOM 等 SoC 提供和维护 Linux 内核和 Android 等.<br>自 2022 年 4 月 1 日起, 高通创新中心公司已将其开源版本移至 [CodeLinaro](https://www.codeaurora.org)  |
| HUAWEI | [consumer-opensource](https://consumer.huawei.com/en/opensource/detail/?siteCode=worldwide&productCode=Smartphones&fileType=openSourceSoftware&pageSize=10&curPage=1) | [opensource](https://consumer.huawei.com/en/opensource) |
| OnePlus | [OnePlus Open Source Software](https://github.com/OnePlusOSS) | [oneplus](https://www.oneplus.com/cn)
| HONOR | [HONOR-OpenSource](https://www.hihonor.com/global/opensource/detail.html) | [hihonor](https://www.hihonor.com/global/opensource) |
| XiaoMi | [Xiaomi_Kernel_OpenSource](https://github.com/MiCode/Xiaomi_Kernel_OpenSource), [mitwo-dev](https://github.com/mitwo-dev)| [micode](https://www.micode.net) |
| RealMe | [realme-kernel-opensource](https://github.com/realme-kernel-opensource) | NA |
| 魅族 | [meizuosc](https://github.com/meizuosc), [lvchaqiu](https://github.com/lvchaqiu)|  [魅族内核团队](https://kernel.meizu.com) |
| 三星 | [opensource.samsung.com](https://opensource.samsung.com/uploadSearch?searchValue=kernel) | [BogList](https://opensource.samsung.com/community/blogList) |
| OPPO | [oppo-source](https://github.com/oppo-source) | NA |
| VIVO | [VIVO OpenSource](https://opensource.vivo.com/Project) | NA |
| CyanogenMod | [CyanogenMod](https://github.com/CyanogenMod) | [cyanogenmodroms](https://cyanogenmodroms.com/) |
| LineageOS-CyanogenMod 的重生 | [lineageos](https://github.com/LineageOS) | [lineageos](https://www.lineageos.org/) |

# 2 CLOUD & SERVER
-------

| 公司 | github | MAIN|
|:-----:|:----------:|:-------:|
| 华为| [openeuler](https://gitee.com/openeuler/kernel) | [openeuler](https://openeuler.org/zh) |
| 阿里巴巴 | [alikernel](https://github.com/alibaba/alikernel) | [阿里云智能基础软件部-技术博客](https://kernel.taobao.org), [Alibaba Cloud Linux 2 DOC](https://help.aliyun.com/document_detail/154950.html?spm=a2c4g.11186623.3.3.37157594hOc6qA) |
| 腾讯 | [TencentOS-kernel](https://github.com/Tencent/TencentOS-kernel) | [腾讯开源 |
| AMAZON | [AmazonLinux](https://github.com/amazonlinux/linux) ||
| ORACLE | [linux-uek](https://github.com/oracle/linux-uek) | [linux-kernel-development](https://blogs.oracle.com/linux/linux-kernel-development), [2](https://blogs.oracle.com/linux/category/lnx-linux-kernel-development) |
| openSuse | [openSUSE/kernel](https://github.com/openSUSE/kernel)<br>*-*-*-*-*-*-*-* <br>[openSUSE/kernel](https://github.com/openSUSE/kernel-source) | [opensuse.org](https://en.opensuse.org/openSUSE:Kernel_git) |
| microsoft | [微软的 linux 内部版本 CBL-Mariner](https://github.com/microsoft/CBL-Mariner-Linux-Kernel)<br>*-*-*-*-*-*-*-* <br>[WSL2-Linux-Kernel](https://github.com/microsoft/WSL2-Linux-Kernel)<br>*-*-*-*-*-*-*-* <br>[azure-linux-kernel](https://github.com/microsoft/azure-linux-kernel) | NA |


# 3 野的不能再野的野分支
-------

[Linux 内核为什么没有第三方的版本或分支？](https://www.zhihu.com/question/36367269/answer/144200260)

在 Linux 发展过程中, 存在诸多场景以及诸多特性和优化无法合入主线, 其中一些特性在特定场景往往表现极为出色, 当然也存在一些开发者出于兴趣, 自行进行了维护, 因此 Linux 衍生出了诸多(野)分支.

## 3.1 那些没有进主线的演进特性
-------

| 分支 | 描述 | 内核 |
|:---:|:---:|:----:|
| [linux-rt(RTLinux)](https://rt.wiki.kernel.org/index.php/Main_Page) | 提供 realtime 支持 | NA |
| [ck](https://users.tpg.com.au/ckolivas/kernel) | [CK(Con_Kolivas)](https://en.wikipedia.org/wiki/Con_Kolivas) 大佬的分支, BFS 以及后来 MuQSS 的作者, 该内核强调改进 linux desktop/laptop 的体验. |
| [zen](https://zen-kernel.org) | 使用一些尚未包含在主线内核的代码, 试图支持最新的硬件, 提供最新的特性、安全补丁和性能优化. 参见 [wiki.ubuntu](https://wiki.ubuntu.com/ZenKernel). | [github.com, zen-kernel](https://github.com/zen-kernel/zen-kernel)
| [grsecurity](https://grsecurity.net) | 基于 grsecurity/PaX 的 patch, 强调安全的 linux 内核 | NA |
| [liquorix](https://liquorix.net) | 基于 linux-zen 内核源码和倾向于 Debian 的设置, 致力于改进桌面、多媒体和游戏体验. | NA |
| [LinuxCNC](https://www.linuxcnc.org) | 针对数控机器人的系统<br>它可以驱动铣床、车床、3D 打印机、激光切割机、等离子切割机、机器人手臂、六足动物等. | [github.com, linuxcnc](https://github.com/LinuxCNC/linuxcnc) |
| [Machinekit](https://www.machinekit.io) | Machinekit 用于支持各种硬件平台和实时环境, 并以低成本提供卓越的性能. 它基于 HAL 组件架构, 这是一个直观且易于使用的电路模型, 包括 150 多个用于数字逻辑、运动、控制环路、信号处理和硬件驱动程序的构建基块. Machinekit 支持本地和联网的 UI 选项, 包括无处不在的平台, 如手机或平板电脑. | [github, Machinekit](https://github.com/machinekit/machinekit) |
| [xanmod](https://xanmod.org) | 旨在提供稳定、响应迅速、流畅的桌面体验. | [github, xanmod/linux](https://github.com/xanmod/linux) |
| [mptcp(MultiPath TCP)](https://www.multipath-tcp.org) | 支持多路径 TCP 的 Linux 内核和模块. | NA |
| [pf](https://pf.natalenko.name) | 名字取自作者 post-factum, 提供一些未加入主线内核的特性集. | NA |
| [linux-vfio](https://github.com/awilliam/linux-vfio) | 出自 Alex Williamson 之手, 启用在某些计算机上使用 KVM 进行 PCI 直通的功能. | [github.com, awilliam/linux-vfio](https://github.com/awilliam/linux-vfio) |



## 3.2 微内核
-------


| 分支 | 描述 | 内核 |
|:---:|:---:|:----:|
| [L4Linux](https://www.l4linux.org) | L4Linux 是[基于 Fiasco 微内核的跟普通 Linux 二进制完全兼容的 Linux 内核](https://blog.csdn.net/gracioushe/article/details/6023340). L4Linux 是运行 Fiasco 用户空间的一个进程, 由很多线程组成, 而 Native Linux 则是运行在内核空间, 如果不考虑 kernel thread 的话, 那么 Native Linux 就相当于一个运行在内核空间的大进程, 它实现了从处理器调度、内存管理到设备管理等所有一个操作系统的必备的功能. 参见 [Adam 的 diploma thesis](https://os.inf.tu-dresden.de/papers_ps/adam-diplom.pdf) | NA |
| [darwin-xnu](https://github.com/apple/darwin-xnu) | 苹果的微内核 | [opensource](https://opensource.apple.com/source/xnu), [cbowser/xnu](https://fergofrog.com/code/cbowser/xnu) |


# 4 Linux Version
-------

[LWN 上各版本归档](https://lwn.net/Kernel/Index)

|  tag  |  LWN  | 泰晓资讯 | phoronix |
|:-----:|:-----:|:-------:|:--------:|
| [5.14](https://lwn.net/Articles/867540) | [2021/07/12 rc1](https://lwn.net/Articles/861695)<br>*-*-*-*-*-*-*-* <br>[2021/07/12 5.14 conclusion](https://lwn.net/Articles/861695) | [8月 / 第一期 / 2021](https://tinylab.org/tinylab-weekly-8-1st-2021)<br>*-*-*-*-*-*-*-* <br>[8月 / 第二期 / 2021](https://tinylab.org/tinylab-weekly-8-2nd-2021/)<br>*-*-*-*-*-*-*-* <br>[2021 年 9 月 第 一 期](https://tinylab.org/tinylab-weekly-9-1st-2021) |
| [5.15](https://lwn.net/Articles/874283) | [2021/09/02 5.15 Merge window, part 1](https://lwn.net/Articles/867821)<br>*-*-*-*-*-*-*-* <br>[2021/09/13 The rest of the 5.15 merge window](https://lwn.net/Articles/868221) | [9月 / 第二期 / 2021](https://tinylab.org/tinylab-weekly-9-2nd-2021) | [Linux 5.15 Delivers Many Features](https://www.phoronix.com/scan.php?page=article&item=linux-515-features&num=1) |
| [5.16](https://lwn.net/Articles/874283) | [2021/11/04 5.16 Merge window, part 1](https://lwn.net/Articles/875135)<br>*-*-*-*-*-*-*-* <br>[2021/11/15 5.16 Merge window, part 2](https://lwn.net/Articles/868221) | [2021/07/12 rc1](https://lwn.net/Articles/875135) | [9月 / 第二期 / 2021](https://tinylab.org/tinylab-weekly-9-2nd-2021) | [Linux 5.16-rc1 ](https://www.phoronix.com/scan.php?page=article&item=linux-516-features&num=1) |
| [5.17](https://lwn.net/Articles/887559) | [The first half of the 5.17 merge window](https://lwn.net/Articles/880909)<br>*-*-*-*-*-*-*-* <br>[The rest of the 5.17 merge window](https://lwn.net/Articles/881597) | [3 月 / 第一期 / 2022](https://tinylab.org/tinylab-weekly-3-1st-2022)<br>*-*-*-*-*-*-*-* <br>[3 月 / 第三期 / 2022](https://tinylab.org/tinylab-weekly-3-3rd-2022) | NA |
| 5.18 | NA | NA | NA |
| 5.19 | NA | NA | NA |
|  6.0 | NA | NA | [Linux 6.0 Supporting New Intel/AMD Hardware, Performance Improvements & Much More](https://www.phoronix.com/review/linux-60-features), [6.0-rc1](https://www.phoronix.com/news/Linux-6.0-rc1-Released) |

# 5 业界会议
-------


2022 年, ChinaSys 学术开源创新平台已经为计算机系统研究人员提供了学术开源索引, 旨在汇聚系统研究相关的优秀开源学术项目, 方便系统研究人员检索和学习. 平台提供的学术开源索引整理了近年来学术会议论文的开源代码链接, 支持基于发表年份、研究领域及关键词的检索, 为系统研究人员提供便利. 截至目前, 平台共收集了 SOSP、OSDI、EuroSys 等 10 个著名学术会议中 652 篇论文的开源代码, 同时提炼出 131 个关键词用于索引. 同时, 学术开源索引还提供了基于合并请求(PR)的自添加功能, 欢迎系统研究人员提交更多的优秀学术开源项目.

参见 [【重磅】ChinaSys 学术开源创新平台开始试运行！](https://zhuanlan.zhihu.com/p/548714548)

[ChinaSys 学术开源创新平台](https://chinasys.org/opensource/program-index.html)


其中 SOSP 与 OSDI 是系统领域的圣殿, 无数研究者的梦想.


| 会议 | 描述 |
|:---:|:----:|
| [LPC](https://www.linuxplumbersconf.org) | 一年一度的 Linux Plumbers Conference(LPC) 是从事 Linux 系统底层(就像房屋的管道 plumbing 类似)细节工作的开发人员中小一部分人的聚会. 它的主题涵盖了从内核之下一直到用户空间的组件, 这些组件是大多数 Linux 用户交互界面和应用程序的基础. |
| [LSFMM](https://events.linuxfoundation.org/lsfmm) | [Linux Storage, Filesystem, Memory-management and BPF Summit(LSFMM)](https://events.linuxfoundation.org/lsfmm) |
| [ASPLOS](https://asplos-conference.org) | International Conference on Architectural Support for Programming Languages and Operating Systems Explanation<br>国际编程语言和操作系统架构支持会议 |
| [SOSP](https://sosp.org) | ACM 操作系统原理大会(SOSP: ACM Symposium on Operating Systems Principles) |
| [OSDI](https://www.usenix.org/conferences/all) | OSDI 的全称是 USENIX Symposium on Operating Systems Design and Implementation, 但随着时代的发展, 它早已不局限在操作系统领域. |
| [USENIX](https://www.usenix.org/conferences/all) | NA |


## 5.1 LPC
-------

| 日期 | 录屏 |
|:---:|:----:|
| 2021/09 | [Watch Live (Free)](https://www.linuxplumbersconf.org/event/11/page/107-watch-live-free) |

## 5.2 ASPLOS
-------


| 日期 | 链接 |
|:---:|:----:|
| 2021/04/12 ~ 2021/04/23 | [ASPLOS 2021](https://asplos-conference.org/2021/index.html) | [ASPLOS 2021 论文选读](https://zhuanlan.zhihu.com/p/366849275)

## 5.3 SOSP
-------

[SOSP](https://sosp.org)

## 5.4 USENIX's OSDI
-------

OSDI 的全称是 USENIX Symposium on Operating Systems Design and Implementation, 但随着时代的发展, 它早已不局限在操作系统领域.


| 日期 | 官网 | 链接 |
|:---:|:----:|:----:|
| 2021/04/12 ~ 2021/04/23 | [ASPLOS 2021](https://asplos-conference.org/2021/index.html) | [OSDI2021 论文选读](https://zhuanlan.zhihu.com/p/393380577) |


## 5.5 LSFMM
-------

| 日期 | 链接 |
|:---:|:----:|
| 2022/05/02 ~ 2022/05/04 | [LSFMM-2022](https://lwn.net/Articles/893733) |

# 6 统计信息
-------

## 6.1 版本
-------

| 网站 | 描述 |
|:---:|:---:|
| [LWN](https://lwn.net/Kernel/Index/#Releases) | LWN 每个版本都会在合并窗口发布 Merge Window, 在版本发布之后还会发布 [development statistics](https://lwn.net/Articles/867540) 信息 |


## 6.2 测试
-------


漏洞扫描器 Coverity Scan : https://scan.coverity.com/projects/linux
