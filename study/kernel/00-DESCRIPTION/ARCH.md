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

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 鄙人在此谢谢啦

**转载请务必注明出处, 谢谢, 不胜感激**

<br>

| 日期 | 作者 | GitHub| CSDN | BLOG |
| ------- |:-------:|:-------:|:-------:|:-------:|
| 2021-02-15 | [成坚-gatieme](https://kernel.blog.csdn.net) | [`AderXCoding/system/tools/fzf`](https://github.com/gatieme/AderXCoding/tree/master/system/tools/fzf) | [使用模糊搜索神器 FZF 来提升办公体验](https://blog.csdn.net/gatieme/article/details/113828826) | [Using FZF to Improve Productivit](https://oskernellab.com/2021/02/15/2021/0215-0001-Using_FZF_to_Improve_Productivity)|


<br>

2   **架构子系统**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***


# 1 X86_64
-------


## 1.1 架构新特性
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/30 | Yu-cheng Yu <yu-cheng.yu@intel.com> | [Control-flow Enforcement: Shadow Stack](https://patchwork.kernel.org/project/linux-mm/cover/20210830181528.1569-1-yu-cheng.yu@intel.com) | 控制流执行(CET) 是英特尔处理器的一个新特性, 它可以阻止面向返回/跳转的编程攻击. 详情见 ["Intel 64 and IA-32架构软件开发人员手册"](https://software.intel.com/content/www/us/en/develop/download/intel-64-and-ia-32-architectures-sdm-combined-volumes-1-2a-2b-2c-2d-3a-3b-3c-3d-and-4.html). CET 可以保护应用程序和内核. 这是 CET 的第一部分, 本系列仅支持应用程序级保护, 并进一步分为阴影堆栈和间接分支跟踪. | v30 ☐ | [Patchwork v30,00/32](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) |
| 2021/08/30 | Yu-cheng Yu <yu-cheng.yu@intel.com> | [Control-flow Enforcement: Indirect Branch Tracking](https://patchwork.kernel.org/project/linux-mm/cover/20210830182221.3535-1-yu-cheng.yu@intel.com) | 控制流执行(CET) 是英特尔处理器的一个新特性, 它可以阻止面向返回/跳转的编程攻击. 详情见 ["Intel 64 and IA-32架构软件开发人员手册"](https://software.intel.com/content/www/us/en/develop/download/intel-64-and-ia-32-architectures-sdm-combined-volumes-1-2a-2b-2c-2d-3a-3b-3c-3d-and-4.html). 这是 CET 的第二部分, 支持间接分支跟踪(IBT). 它是建立在阴影堆栈系列之上的. | v1 ☐ | [Patchwork v30,00/10](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) |
| 2021/04/19 | Fenghua Yu <fenghua.yu@intel.com> | [x86/bus_lock: Set rate limit for bus lock](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Bus-Lock-Detection-2021) | 当原子指令跨越多个 cache line, 并且需要确保原子性所需的总线锁时, 就会发生拆分锁. 这些拆分锁至少比单个 cacheline 内的原子操作多需要 1000 个 cycles. 在锁定总线期间, 其他 CPU 或 BUS 代理要求控制 BUS 的请求被阻止, 阻止其他 CPU 的 BUS 访问, 加上配置总线锁定协议的开销不仅会降低一个 CPU 的性能, 还会降低整体系统性能. 除了拆分锁的性能影响之外, 利用该行为还可能导致无特权的拒绝服务漏洞. 现代英特尔 CPU 可以在处理拆分锁时生成对齐检查异常, 使用此技术这组补丁为内核实现了拆分锁检测(split_lock_detect), 以警告或杀死违规应用. 默认行为是警告有问题的用户空间应用程序, 而如果配置了 `split_lock_detect=fatal` 内核参数将杀死使用 SIGBUS 的应用程序. 参考 [phoronix1](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.7-Split-Lock-Detection), [phoronix2](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Split-Locks-Detection), [phoronix3](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Bus-Lock-Detection-2021) | v1 ☐ | [Patchwork 0/4](https://lore.kernel.org/all/20210419214958.4035512-1-fenghua.yu@intel.com) |
| 2020/01/19 | Yu-cheng Yu <yu-cheng.yu@intel.com> | [Enable Sub-Page Write Protection Support](https://lwn.net/Articles/810033) | 基于 EPT 的子页写保护(SPP) 允许虚拟机监视器(VMM)以子页(128字节)粒度为客户物理内存指定写权限. 当 SPP 工作时, 硬件强制对受保护的 4KB 页面中的子页面进行写访问检查. 该特性的目标是为内存保护和虚拟机内省等使用提供细粒度的内存保护. 当"子页面写保护"(第23位)在 Secondary VM-Execution Controls 中为1时, SPP 被启用. 该特性支持子页权限表(SPPT), 子页权限向量存储在SPPT的叶条目中. 根页面是通过VMCS中的子页面权限表指针(SPPTP)引用的.<br>要为 guest 内存启用 SPP, guest 页面应该首先映射到一个 4KB 的 EPT 条目, 然后设置相应条目的 SPP 的 61bit. 当硬件遍历 EPT 时, 它使用 gpa 遍历 SPPT 以查找 SPPT 叶子条目中的子页面权限向量. 如果设置了对应位, 则允许写子页, 否则产生 SPP 触发的 EPT 冲突. | v30 ☐ | [Patchwork v30,00/32](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) |


## 1.2 指令加速
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/07/30 | "Chang S. Bae" <chang.seok.bae@intel.com> | [x86: Support Intel Advanced Matrix Extensions](https://lore.kernel.org/lkml/20210730145957.7927-1-chang.seok.bae@intel.com) | 支持即将发布的英特尔[高级矩阵扩展(AMX)](https://software.intel.com/content/dam/develop/external/us/en/documents-tps/architecture-instruction-set-extensions-programming-reference.pdf), [AMX](https://software.intel.com/content/www/us/en/develop/documentation/cpp-compiler-developer-guide-and-reference/top/compiler-reference/intrinsics/intrinsics-for-intel-advanced-matrix-extensions-intel-amx-instructions.html) 由可配置的 TMM"TILE" 寄存器和操作它们的新 CPU 指令组成. TMUL (Tile matrix MULtiply)是第一个利用新寄存器的运算符, 我们预计将来会有更多的指令. | v9 ☐ | [Patchwork v9,00/26](https://lore.kernel.org/lkml/20210730145957.7927-1-chang.seok.bae@intel.com) |
| 2021/07/20 | Tianjia Zhang <tianjia.zhang@linux.alibaba.com> | [Introduce x86 assembler accelerated implementation for SM4 algorithm](https://lwn.net/Articles/863574/) | 该补丁集提取了公有 SM4 算法作为一个单独的库, 同时对 arm64 中 SM4 的加速实现进行了调整, 以适应该 SM4 库. 然后在 x86_64 上的进行了实现. 该优化支持 SM4、ECB、CBC、CFB 和 CTR 四种模式. 由于 CBC 和 CFB 不支持多块并行加密, 优化效果不明显. 主要算法实现来自 libgcrypt 和 Markku-Juhani O. Saarinen 的 [SM4 AES-NI 工作](https://github.com/mjosaarinen/sm4ni). | v1 ☐ | [LWN 0/2](https://lwn.net/Articles/863574/) |
| 2021/08/12 | Tianjia Zhang <tianjia.zhang@linux.alibaba.com> | [support test GCM/CCM mode for SM4](https://lore.kernel.org/lkml/20210812131748.81620-1-tianjia.zhang@linux.alibaba.com) | NA | v1 ☐ | [Patchwork 0/3](https://lore.kernel.org/lkml/20210812131748.81620-1-tianjia.zhang@linux.alibaba.com), [LKML](https://lkml.org/lkml/2021/8/13/161) |
| 2021/08/18 | Tianjia Zhang <tianjia.zhang@linux.alibaba.com> | [add AES-NI/AVX2/x86_64 implementation](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) | 这个补丁集导出了 SM4 AESNI/AVX 算法实现的一些常用函数, 并用这些函数实现 AESNI/AVX2 的加速.<br>主要算法实现来自 libgcrypt 和 [Markku Juhani 的 SM4 AES-NI 工作](https://github.com/mjosaarinen/sm4ni). | v1 ☐ | [Patchwork 0/2](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) |


## 1.3 架构上新想法
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/04/19 | Fenghua Yu <fenghua.yu@intel.com> | [x86/bus_lock: Set rate limit for bus lock](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Bus-Lock-Detection-2021) | 当原子指令跨越多个 cache line, 并且需要确保原子性所需的总线锁时, 就会发生拆分锁. 这些拆分锁至少比单个 cacheline 内的原子操作多需要 1000 个 cycles. 在锁定总线期间, 其他 CPU 或 BUS 代理要求控制 BUS 的请求被阻止, 阻止其他 CPU 的 BUS 访问, 加上配置总线锁定协议的开销不仅会降低一个 CPU 的性能, 还会降低整体系统性能. 除了拆分锁的性能影响之外, 利用该行为还可能导致无特权的拒绝服务漏洞. 现代英特尔 CPU 可以在处理拆分锁时生成对齐检查异常, 使用此技术这组补丁为内核实现了拆分锁检测(split_lock_detect), 以警告或杀死违规应用. 默认行为是警告有问题的用户空间应用程序, 而如果配置了 `split_lock_detect=fatal` 内核参数将杀死使用 SIGBUS 的应用程序. 参考 [phoronix1](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.7-Split-Lock-Detection), [phoronix2](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Split-Locks-Detection), [phoronix3](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Bus-Lock-Detection-2021) | v1 ☐ | [Patchwork 0/4](https://lore.kernel.org/all/20210419214958.4035512-1-fenghua.yu@intel.com) |
| 2021/09/13 | Sohil Mehta <sohil.mehta@intel.com> | [x86 User Interrupts support](https://lwn.net/Articles/869140) | 用户中断(Uintr)是一种硬件技术, 可以将中断直接传递到用户空间.<br>如今, 几乎所有跨越特权边界的通信都是通过内核进行的. 这些包括信号、管道、远程过程调用和基于硬件中断的通知. 用户中断通过避免通过内核的转换, 为这些常见操作的更高效(低延迟和低CPU利用率)版本提供了基础. 在用户中断硬件体系结构中, 接收者总是期望是用户空间任务. 但是, 用户中断可以由另一个用户空间任务、内核或外部源(如设备)发送. 除了接收用户中断的一般基础结构之外, 这组补丁还介绍了另一个用户任务的中断(用户 IPI). 用户 IPI 的第一个实现将在代号为 Sapphire Rapids 的英特尔处理器中实现. 有关硬件架构的详细信息, 请参阅[英特尔架构指令集扩展的第11章](https://software.intel.com/content/www/us/en/develop/download/intel-architecture-instruction-set-extensions-programming-reference.html). | v1 ☐ | [Patchwork RFC,00/13](https://lwn.net/ml/linux-kernel/20210913200132.3396598-1-sohil.mehta@intel.com) |

## 1.4 让人眼前一亮的新架构
-------

### 1.4.1 hybrid CPUs
-------

Intel Architecture Day 2021, 官宣了自己的服务于终端和桌面场景的异构(或者混合架构)处理器架构 [Alder Lake](https://www.anandtech.com/show/16881/a-deep-dive-into-intels-alder-lake-microarchitectures), 与 ARM 的 big.LITTLE 以及 DynamIQ 架构, 包含了基于 Golden Cove 微架构的性能核(P-core/Performance cores)以及基于新的 Gracemont 架构的能效核(E-core/Efficiency cores). P-core 优先用于需要低延迟的单线程任务, 而 E-core 在功率有限或多线程情景方面更好.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/09/17 | Catalin Marinas <catalin.marinas@arm.com> | [x86: Add initial support to discover Intel hybrid CPUs](https://lore.kernel.org/lkml/20201002201931.2826-1-ricardo.neri-calderon@linux.intel.com) | 支持 ARM64 | v3 ☑ 3.7-rc1 | [Patchwork 0/3](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1347035226-18649-8-git-send-email-catalin.marinas@arm.com) |


为了更好的发挥这种混合架构的优势, Intel 提供了一项名为 [Thread Director 的技术](https://www.anandtech.com/show/16881/a-deep-dive-into-intels-alder-lake-microarchitectures/2).

1.  首席按通过机器学习算法对进程在 P-core/E-core 的性能和功耗进行分析和建模, 识别进程特征, 从而可以预测出不同类型进程或者进程某段时期在 P/E core 上的能效关系.

2.  硬件将进程的运行能效换算后, 直接填写到内存中. 当进程在 P/E core 上运行时负责某种规则时(比如进程行为特征发生变化, 即能效发生变化), 直接通知 OS/Kernel 调度器进行决策, 以达到最好的能效.




# 2 ARM64
-------


[Memory Layout on AArch64 Linux](https://www.kernel.org/doc/html/latest/arm64/memory.html)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/09/17 | Catalin Marinas <catalin.marinas@arm.com> | [AArch64 Linux kernel port](https://lore.kernel.org/lkml/1347035226-18649-1-git-send-email-catalin.marinas@arm.com) | 支持 ARM64 | v3 ☑ 3.7-rc1 | [Patchwork v4,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1347035226-18649-8-git-send-email-catalin.marinas@arm.com) |

## 2.1 DEBUG
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/18 | Tianjia Zhang <tianjia.zhang@linux.alibaba.com> | [arm64: Enable OPTPROBE for arm64](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) | 这个补丁集导出了 SM4 AESNI/AVX 算法实现的一些常用函数, 并用这些函数实现 AESNI/AVX2 的加速.<br>主要算法实现来自 libgcrypt 和 [Markku Juhani 的 SM4 AES-NI 工作](https://github.com/mjosaarinen/sm4ni). | v4 ☐ | [Patchwork v4,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20210818073336.59678-1-liuqi115@huawei.com) |

## 2.2 TLB
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/06/25 | Tianjia Zhang <tianjia.zhang@linux.alibaba.com> | [arm64: tlb: add support for TTL feature](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200625080314.230-1-yezhenyu2@huawei.com) | 为了降低 TLB 失效的成本, ARMv8.4 在 TLBI 指令中提供了 TTL 字段. TTL 字段表示保存被失效地址的叶条目的页表遍历级别. 这组补丁实现了对 TTL 的支持. | v4 ☑ 5.9-rc1 | [Patchwork RESEND,v5,0/6](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200625080314.230-1-yezhenyu2@huawei.com) |
| 2020/07/15 | Zhenyu Ye <yezhenyu2@huawei.com> | [arm64: tlb: Use the TLBI RANGE feature in arm64](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) | 为 ARM64 实现 CONFIG_ARM64_TLB_RANGE, 在实现了 ARM64_HAS_TLB_RANGE 的机器上使用此 feature 实现了 `__flush_tlb_range()` | v3 ☑ 5.9-rc1 | [2020/07/08 PatchWork RFC,v5,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200708124031.1414-1-yezhenyu2@huawei.com)<br>*-*-*-*-*-*-*-* <br>[2020/07/09 PatchWork v1,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200710094420.517-1-yezhenyu2@huawei.com)<br>*-*-*-*-*-*-*-* <br>[2020/07/10 PatchWork v2,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200710094420.517-1-yezhenyu2@huawei.com)<br>*-*-*-*-*-*-*-* <br>[2020/07/15 Patchwork v3,0/3](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200715071945.897-1-yezhenyu2@huawei.com) |


## 2.3 指令加速
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/07/24 | Will Deacon <will.deacon@arm.com> | [arm64: support for 8.1 LSE atomic instructions](https://lwn.net/Articles/650900) | 为 Linux 内核添加了对[新原子指令(LSE atomic instructions)的支持](https://mysqlonarm.github.io/ARM-LSE-and-MySQL), 这是作为 ARMv8.1 中大系统扩展(LSE-LSE atomic instructions)的一部分引入的. 新的指令可以在编译时通过 CONFIG_ARM64_LSE_ATOMICS 选项配置出来.<br>之前测试发现, 核少的时候关 LSE 性能更好, 核多的时候, 开 LSE 性能更好. | v1 ☑ 4.3-rc1 | [2015/07/13 Patchwork 00/18](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200625080314.230-1-yezhenyu2@huawei.com)<br>*-*-*-*-*-*-*-* <br>[2015/07/24 Patchwork v2,07/20](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1437734531-10698-8-git-send-email-will.deacon@arm.com/) |



# 3 RISC-V
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/10 | Anup Patel <anup.patel@wdc.com> | [RISC-V CPU Idle Support](https://patchwork.kernel.org/project/linux-riscv/cover/20210610052221.39958-1-anup.patel@wdc.com) | RISC-V高级核心本地中断(Advacned Core Local Interruptor-ACLINT) 支持 | v3 ☐ | [Patchwork v7,0/8](https://patchwork.kernel.org/project/linux-riscv/cover/20210830041729.237252-1-anup.patel@wdc.com) |

| 2021/08/30 | Anup Patel <anup.patel@wdc.com> | [Linux RISC-V ACLINT Support](https://patchwork.kernel.org/project/linux-riscv/cover/20210830041729.237252-1-anup.patel@wdc.com) | RISC-V高级核心本地中断(Advacned Core Local Interruptor-ACLINT) 支持 | v3 ☐ | [Patchwork RFC,v3,00/11](https://patchwork.kernel.org/project/linux-riscv/cover/20210610052221.39958-1-anup.patel@wdc.com) |



相关的文章介绍: [47].



<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
