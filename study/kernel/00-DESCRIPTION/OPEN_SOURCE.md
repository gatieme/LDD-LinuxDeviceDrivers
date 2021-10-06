# 1 ANDROID
-------

| 公司 | github | MAIN|
|:-----:|:----------:|:-------:|
| GOOGLE | [googlesource](https://android.googlesource.com/kernel/common) | NA |
| QCOM | [source.codeaurora](https://source.codeaurora.org/quic/la/kernel) | [codeaurora](https://www.codeaurora.org) |
| HUAWEI | [consumer-opensource](https://consumer.huawei.com/en/opensource/detail/?siteCode=worldwide&productCode=Smartphones&fileType=openSourceSoftware&pageSize=10&curPage=1) | [opensource](https://consumer.huawei.com/en/opensource) |
| XiaoMi | [Xiaomi_Kernel_OpenSource](https://github.com/MiCode/Xiaomi_Kernel_OpenSource), [mitwo-dev](https://github.com/mitwo-dev)| [micode](http://www.micode.net) |
| 魅族 | [meizuosc](https://github.com/meizuosc), [lvchaqiu](https://github.com/lvchaqiu)|  [魅族内核团队](http://kernel.meizu.com) |
| 三星 | [opensource.samsung.com](https://opensource.samsung.com/uploadSearch?searchValue=kernel) | [BogList](https://opensource.samsung.com/community/blogList) |
| OPPO | [oppo-source](https://github.com/oppo-source) | NA |
| CyanogenMod | [CyanogenMod](https://github.com/CyanogenMod) | [cyanogenmodroms](https://cyanogenmodroms.com/) |
| LineageOS-CyanogenMod 的重生 | [lineageos](https://github.com/LineageOS) | [lineageos](https://www.lineageos.org/) |
| OnePlus | [OnePlus Open Source Software](https://github.com/OnePlusOSS) | [oneplus](https://www.oneplus.com/cn)


# 2 CLOUD & SERVER
-------

| 公司 | github | MAIN|
|:-----:|:----------:|:-------:|
| 华为| [openeuler](http://gitee.com/openeuler/kernel) | [openeuler](https://openeuler.org/zh) |
| 阿里巴巴 | [alikernel](https://github.com/alibaba/alikernel) | [阿里云智能基础软件部-技术博客](https://kernel.taobao.org), [Alibaba Cloud Linux 2 DOC](https://help.aliyun.com/document_detail/154950.html?spm=a2c4g.11186623.3.3.37157594hOc6qA) |
| 腾讯 | [TencentOS-kernel](https://github.com/Tencent/TencentOS-kernel) | [腾讯开源 | Tencent Open Source](https://opensource.tencent.com/projects) |
| AMAZON | [AmazonLinux](https://github.com/amazonlinux/linux) |
| ORACLE | [linux-uek](https://github.com/oracle/linux-uek) | [linux-kernel-development](https://blogs.oracle.com/linux/linux-kernel-development) |
| openSuse | [openSUSE/kernel](https://github.com/openSUSE/kernel)<br>*-*-*-*-*-*-*-* <br>[openSUSE/kernel](https://github.com/openSUSE/kernel-source) | [opensuse.org](https://en.opensuse.org/openSUSE:Kernel_git) |
| microsoft | [微软的 linux 内部版本 CBL-Mariner](https://github.com/microsoft/CBL-Mariner-Linux-Kernel)<br>*-*-*-*-*-*-*-* <br>[WSL2-Linux-Kernel](https://github.com/microsoft/WSL2-Linux-Kernel)<br>*-*-*-*-*-*-*-* <br>[azure-linux-kernel](https://github.com/microsoft/azure-linux-kernel) | NA |


# 3 野的不能再野的野分支
-------

[Linux 内核为什么没有第三方的版本或分支？](https://www.zhihu.com/question/36367269/answer/144200260)

在 Linux 发展过程中, 存在诸多场景以及诸多特性和优化无法合入主线, 其中一些特性在特定场景往往表现极为出色, 当然也存在一些开发者出于兴趣, 自行进行了维护, 因此 Linux 衍生出了诸多(野)分支.

| 分支 | 描述 | 内核 |
|:---:|:---:|:----:|
| [linux-rt(RTLinux)](https://rt.wiki.kernel.org/index.php/Main_Page) | 提供 realtime 支持 | NA |
| [ck](http://users.tpg.com.au/ckolivas/kernel) | [CK(Con_Kolivas)](https://en.wikipedia.org/wiki/Con_Kolivas) 大佬的分支, BFS 以及后来 MuQSS 的作者, 该内核强调改进 linux desktop/laptop 的体验. |
| [zen](http://zen-kernel.org) | 使用一些尚未包含在主线内核的代码, 试图支持最新的硬件, 提供最新的特性、安全补丁和性能优化. 参见 [wiki.ubuntu](https://wiki.ubuntu.com/ZenKernel). | [github.com, zen-kernel](https://github.com/zen-kernel/zen-kernel)
| [grsecurity](https://grsecurity.net) | 基于 grsecurity/PaX 的 patch, 强调安全的 linux 内核 | NA |
| [liquorix](https://liquorix.net) | 基于 linux-zen 内核源码和倾向于 Debian 的设置, 致力于改进桌面、多媒体和游戏体验. | NA |
| [LinuxCNC](https://www.linuxcnc.org) | 针对数控机器人的系统<br>它可以驱动铣床、车床、3D 打印机、激光切割机、等离子切割机、机器人手臂、六足动物等. | [github.com, linuxcnc](https://github.com/LinuxCNC/linuxcnc) |
| [Machinekit](http://www.machinekit.io) | Machinekit 用于支持各种硬件平台和实时环境, 并以低成本提供卓越的性能. 它基于 HAL 组件架构, 这是一个直观且易于使用的电路模型, 包括 150 多个用于数字逻辑、运动、控制环路、信号处理和硬件驱动程序的构建基块. Machinekit 支持本地和联网的 UI 选项, 包括无处不在的平台, 如手机或平板电脑. | [github, Machinekit](https://github.com/machinekit/machinekit) |
| [xanmod](https://xanmod.org) | 旨在提供稳定、响应迅速、流畅的桌面体验. | [github, xanmod/linux](https://github.com/xanmod/linux) |
| [mptcp(MultiPath TCP)](https://www.multipath-tcp.org) | 支持多路径 TCP 的 Linux 内核和模块. | NA |
| [pf](https://pf.natalenko.name) | 名字取自作者 post-factum, 提供一些未加入主线内核的特性集. | NA |
| [linux-vfio](https://github.com/awilliam/linux-vfio) | 出自 Alex Williamson 之手, 启用在某些计算机上使用 KVM 进行 PCI 直通的功能. | [github.com, awilliam/linux-vfio](https://github.com/awilliam/linux-vfio) |

# 4 Linux Version
-------

|  tag  |  LWN  | 泰晓资讯 |
|:-----:|:-----:|:-------:|
| [5.14](https://lwn.net/Articles/867540) | [2021/07/12 rc1](https://lwn.net/Articles/861695)<br>*-*-*-*-*-*-*-* <br>[2021/07/12 5.14 conclusion](https://lwn.net/Articles/861695) | [8月 / 第一期 / 2021](http://tinylab.org/tinylab-weekly-8-1st-2021)<br>*-*-*-*-*-*-*-* <br>[8月 / 第二期 / 2021](http://tinylab.org/tinylab-weekly-8-2nd-2021/)<br>*-*-*-*-*-*-*-* <br>[2021 年 9 月 第 一 期](http://tinylab.org/tinylab-weekly-9-1st-2021) |
| 5.15 | [2021/07/12 rc1](https://lwn.net/Articles/861695) | [9月 / 第二期 / 2021](http://tinylab.org/tinylab-weekly-9-2nd-2021) |


# 5 业界会议
-------


## 5.1 LPC
-------

| 日期 | 录屏 |
|:---:|:----:|
| 2021/09 | [Watch Live (Free)](https://www.linuxplumbersconf.org/event/11/page/107-watch-live-free) |

