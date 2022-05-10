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

[Intel® 64 and IA-32 Architectures Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

[Intel Architecture Day 2021](https://www.intel.com/content/www/us/en/newsroom/resources/press-kit-architecture-day-2021.html)

## 1.1 架构新特性
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
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
| 2021/09/13 | Sohil Mehta <sohil.mehta@intel.com> | [x86 User Interrupts support](https://lwn.net/Articles/871113) | 用户中断(Uintr)是一种硬件技术, 可以将中断直接传递到用户空间.<br>如今, 几乎所有跨越特权边界的通信都是通过内核进行的. 这些包括信号、管道、远程过程调用和基于硬件中断的通知. 用户中断通过避免通过内核的转换, 为这些常见操作的更高效(低延迟和低CPU利用率)版本提供了基础. 在用户中断硬件体系结构中, 接收者总是期望是用户空间任务. 但是, 用户中断可以由另一个用户空间任务、内核或外部源(如设备)发送. 除了接收用户中断的一般基础结构之外, 这组补丁还介绍了另一个用户任务的中断(用户 IPI). 用户 IPI 的第一个实现将在代号为 Sapphire Rapids 的英特尔处理器中实现. 有关硬件架构的详细信息, 请参阅[英特尔架构指令集扩展的第11章](https://software.intel.com/content/www/us/en/develop/download/intel-architecture-instruction-set-extensions-programming-reference.html). | v1 ☐ | [LWN RFC,00/13](https://lwn.net/ml/linux-kernel/20210913200132.3396598-1-sohil.mehta@intel.com)<br>*-*-*-*-*-*-*-* <br>[LWN](https://lwn.net/Articles/869140)<br>*-*-*-*-*-*-*-* <br>[Patchwork RFC,00/13](https://lore.kernel.org/lkml/20210913200132.3396598-1-sohil.mehta@intel.com) |
| 2021/10/04 | Tvrtko Ursulin <tvrtko.ursulin@linux.intel.com> | [CPU + GPU synchronised priority scheduling](https://www.phoronix.com/scan.php?page=news_item&px=Intel-CPU-GPU-Prio-Nice-Sched) | Intel 在 CPU 和 GPU 协同调度领域进行了探索与尝试. | v1 ☐ | [Patchwork RFC,0/6](https://lists.freedesktop.org/archives/intel-gfx/2021-September/279200.html), [LWN](https://lwn.net/Articles/871467), [LKML](https://lkml.org/lkml/2021/9/30/852)<br>*-*-*-*-*-*-*-* <br>[LKML v2,0/8](https://lkml.org/lkml/2021/10/4/1004), [LWN news](https://lwn.net/Articles/873334) |


## 1.4 hybrid CPUs
-------

[该感谢 AMD 还是 ARM？：Intel Alder Lake 为什么要搞大小核？](https://zhuanlan.zhihu.com/p/350097861)

### 1.4.1 Intel hybrid CPUs
-------


phoronix 上所有与 [Alder Lake 相关的报道](https://www.phoronix.com/scan.php?page=search&q=Alder%20Lake).

#### 1.4.1.1 架构支持
-------

Intel Architecture Day 2021, 官宣了自己的服务于终端和桌面场景的异构(或者混合架构)处理器架构 [Alder Lake](https://www.anandtech.com/show/16881/a-deep-dive-into-intels-alder-lake-microarchitectures), 与 ARM 的 big.LITTLE 以及 DynamIQ 架构类似, 它包含了基于 Golden Cove 微架构的性能核(P-core/Performance cores)以及基于新的 Gracemont 架构的能效核(E-core/Efficiency cores). P-core 优先用于需要低延迟的单线程任务, 而 E-core 在功率有限或多线程情景方面更好.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/10/02 | Catalin Marinas <catalin.marinas@arm.com> | [x86: Add initial support to discover Intel hybrid CPUs](https://lore.kernel.org/lkml/20201002201931.2826-1-ricardo.neri-calderon@linux.intel.com) | 支持混合微架构的 CPU(Alder Lake CPU) | v3 ☐ | [Patchwork 0/3](https://lore.kernel.org/lkml/20201002201931.2826-1-ricardo.neri-calderon@linux.intel.com) |
| 2021/05/12  | Rafael J. Wysocki <rafael.j.wysocki@intel.com> | [cpufreq: intel_pstate: hybrid: CPU-specific scaling factors](https://www.phoronix.com/scan.php?page=news_item&px=P-State-Preps-For-Hybrid) | Hybrid CPU 的 P-state 增强 | v1 ☑ 5.14-rc1 | [Patchwork 0/3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=eb3693f0521e020dd8617c7fa3ddf5c9f0d8dea0) |
| 2021/02/08  | Kan Liang <kan.liang@linux.intel.com> | [Add Alder Lake support for perf](https://lkml.org/lkml/2021/2/8/1142) | perf 支持 Hybrid CPU.  | v1 ☑ 5.14-rc1 | [LKML 00/49](https://lkml.org/lkml/2021/2/8/1142) |
| 2021/04/05  | Kan Liang <kan.liang@linux.intel.com> | [Add Alder Lake support for perf (kernel)](https://lkml.org/lkml/2021/4/5/775) | perf 支持 Hybrid CPU(内核态). | v1 ☑ 5.13-rc1 | [LKML V5 00/25](https://lkml.org/lkml/2021/4/5/775), [LKML V3 00/25](https://lkml.org/lkml/2021/3/26/964) |
| 2021/04/23  | Kan Liang <kan.liang@linux.intel.com> | [perf tool: AlderLake hybrid support series 1](https://lkml.org/lkml/2021/4/23/52) | perf 支持 Hybrid CPU(内核态). | v1 ☑ 5.13-rc1 | [LKML v5 00/26](https://lkml.org/lkml/2021/4/23/52) |
| 2021/05/27  | Kan Liang <kan.liang@linux.intel.com> | [perf: Support perf-mem/perf-c2c for AlderLake](https://lkml.org/lkml/2021/4/5/775) | perf 支持 Hybrid CPU(内核态). | v2 ☑ 5.14-rc1 | [LKML v1 0/8](https://lkml.org/lkml/2021/4/5/775), [LKML v2 0/8](https://lkml.org/lkml/2021/5/27/191) |
| 2021/09/10 | Ricardo Neri <ricardo.neri-calderon@linux.intel.com> | [sched/fair: Fix load balancing of SMT siblings with ASYM_PACKING](https://lwn.net/Articles/880367/) | 在使用非对称封装(ASM_PACKING)时, 可能存在具有三个优先级的 CPU 拓扑, 其中只有物理核心的子集支持 SMT. 这种架构下 ASM_PACKING 和 SMT 以及 load_balance 都存在冲突.<br>这种拓扑的一个实例是 Intel Alder Lake. 在 Alder Lake 上, 应该通过首先选择 Core(酷睿) cpu, 然后选择 Atoms, 最后再选择 Core 的 SMT 兄弟 cpu 来分散工作. 然而, 当前负载均衡器的行为与使用 ASYM_PACKING 时描述的不一致. 负载平衡器将选择高优先级的 CPU (Intel Core) 而不是中优先级的 CPU (Intel Atom), 然后将负载溢出到低优先级的 SMT 同级 CPU. 这使得中等优先级的 Atoms cpu 空闲, 而低优先级的 cpu sibling 繁忙.<br>1. 首先改善了 SMT 中 sibling cpu 优先级的计算方式, 它将比单个 core 优先级更低.<br>2. 当决定目标 CPU 是否可以从最繁忙的 CPU 提取任务时, 还检查执行负载平衡的 CPU 和最繁忙的候选组的 SMT 同级 CPU 的空闲状态. | v5 ☑ 5.16-rc1 | [PatchWork v1](https://lore.kernel.org/patchwork/cover/1408312)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/cover/1413015)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3 0/6](https://lore.kernel.org/patchwork/cover/1428441)<br>*-*-*-*-*-*-*-* <br>[PatchWork v4,0/6](https://lore.kernel.org/patchwork/cover/1474500)<br>*-*-*-*-*-*-*-* <br>[LKML v5,0/6](https://lkml.org/lkml/2021/9/10/913), [LORE v5,0/6](https://lore.kernel.org/all/20210911011819.12184-1-ricardo.neri-calderon@linux.intel.com) |
| 2021/11/19 | Srinivas Pandruvada <srinivas.pandruvada@linux.intel.com> | [cpufreq: intel_pstate: ITMT support for overclocked system](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Patch-ITMT-OC-ADL) | Intel ITMT (Intel Turbo Boost Max Technology) 感知混合架构, Alder Lake CPU 上 P-Core/E-core 优先级应该有不同的值(P-core 0x40, P-core HT sibling 0x10, E-core 0x26). | v1 ☑ 5.16-rc3 | [Patchwork](https://patchwork.kernel.org/project/linux-pm/patch/20211119051801.1432724-1-srinivas.pandruvada@linux.intel.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=03c83982a0278207709143ba78c5a470179febee) |
| 2021/12/16 | Srinivas Pandruvada <srinivas.pandruvada@linux.intel.com> | [cpufreq: intel_pstate: Update EPP for AlderLake mobile](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-P-State-ADL-Mobile) | 修正 AlderLake 的 EPP. | v1 ☑ 5.17-rc1 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b6e6f8beec98ba7541213c506fe908517fdc52b8) |
| 2022/04/15 | Zhang Rui <rui.zhang@intel.com> | [intel_idle: add AlderLake support](https://lore.kernel.org/all/20220415093951.2677170-1-rui.zhang@intel.com) | 参见 [phoronix 报道](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Idle-Alder-Lake) | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20220415093951.2677170-1-rui.zhang@intel.com) |


#### 1.4.1.2 Thread Director
-------

[Thread Director](https://www.anandtech.com/show/16881/a-deep-dive-into-intels-alder-lake-microarchitectures/2) 其实是一个软硬协同优化的范畴.

早在 2018 年, 苹果就发布过自家 AMP 硬件上的软硬协同优化的调度器, 中文专利号 [CN108984282A 具有闭环性能控制器的 AMP 体系结构的调度器/Scheduler for AMP architecture with closed loop performance controller](https://www.patentguru.com/cn/CN108984282A).

[Intel Hardware Feedback Interface "HFI" Driver Submitted For Linux 5.18](https://www.phoronix.com/scan.php?page=news_item&px=Intel-HFI-Thermal-Linux-5.18)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/06 | Ricardo Neri <ricardo.neri-calderon-AT-linux.intel.com> | [Thermal: Introduce the Hardware Feedback Interface for thermal and performance management](https://lwn.net/Articles/875296) | 支持 Intel HFI.<br>英特尔硬件反馈接口(HFI) 提供系统中每个 CPU 的性能(performance)和能效(Energy efficiency)的信息. 它使用一个在硬件和操作系统之间共享的表. 该表的内容可能由于系统运行条件的变化(如达到热极限)或外部因素的作用(如热设计功率的变化)而更新.<br>HFI 提供的信息被指定为相对于系统中其他 cpu 的数字、单元较少的能力. 这些功能的范围为 [0-255], 其中更高的数字表示更高的功能. 如果 CPU 的性能效率或能量能力效率为 0, 硬件建议分别出于性能、能量效率或热原因, 不要在该 CPU 上调度任何任务.<br>内核或用户空间可以使用来自 HFI 的信息来修改任务放置或调整功率限制. 当前这个补丁集中于用户空间. 热通知框架(thermal notification framework)被扩展以支持 CPU capacity 的更新. | v1 ☐ | [2021/11/06 LWN](https://lwn.net/Articles/875296)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/7](https://lore.kernel.org/lkml/20211220151438.1196-1-ricardo.neri-calderon@linux.intel.com), [phoronix v2](https://www.phoronix.com/scan.php?page=news_item&px=Intel-HFI-Linux-v2-2021)<br>*-*-*-*-*-*-*-* <br>[PatchWork v5,0/7](https://patchwork.kernel.org/project/linux-pm/cover/20220127193454.12814-1-ricardo.neri-calderon@linux.intel.com), [phoronix v5](https://www.phoronix.com/scan.php?page=news_item&px=Intel-HFI-For-Linux-5.18) |


#### 1.4.1.2 编译器支持
-------


[Intel Updates Alder Lake Tuning For GCC, Reaffirms No Official AVX-512](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Alder-Lake-Tuning-GCC)


#### 1.4.1.3 性能评测
-------

[Intel Core I9-12900K VS I5-12600K](https://openbenchmarking.org/vs/Processor/Intel%20Core%20i5-12600K,Intel%20Core%20i9-12900K)

[The Intel 12th Gen Core i9-12900K Review: Hybrid Performance Brings Hybrid Complexity](https://www.anandtech.com/show/17047/the-intel-12th-gen-core-i912900k-review-hybrid-performance-brings-hybrid-complexity)

Intel Alder Lake CPU 支持 AVX 512

[Intel Alder Lake Question (Scheduler)](https://lkml.org/lkml/2021/11/5/79)

[Intel Core i9 12900K "Alder Lake" AVX-512 On Linux](https://www.phoronix.com/scan.php?page=article&item=alder-lake-avx512&num=1)

#### 1.4.1.4 调度器优化
-------

为了更好的发挥这种混合架构的优势, Intel 提供了一项名为 [Thread Director 的技术](https://www.anandtech.com/show/16881/a-deep-dive-into-intels-alder-lake-microarchitectures/2), 专利分析 [The Alder Lake hardware scheduler – A brief overview](https://coreteks.tech/articles/index.php/2021/07/02/the-alder-lake-hardware-scheduler-a-brief-overview/)

1.  首席按通过机器学习算法对进程在 P-core/E-core 的性能和功耗进行分析和建模, 识别进程特征, 从而可以预测出不同类型进程或者进程某段时期在 P/E core 上的能效关系.

2.  硬件将进程的运行能效换算后, 直接填写到内存中. 当进程在 P/E core 上运行时负责某种规则时(比如进程行为特征发生变化, 即能效发生变化), 直接通知 OS/Kernel 调度器进行决策, 以达到最好的能效.

### 1.4.2 AMD's version of big.LITTLE
-------

AMD 关于大小核的专利 [US20210173715A1: METHOD OF TASK TRANSITION BETWEEN HETEROGENOUS PROCESSORS](https://www.freepatentsonline.com/y2021/0173715.html), 最初于 2019 年 12 月提交, 2021 年被公布. 根据该专利, CPU 将依靠 CPU 利用率等指标来确定何时适合将工作负载从一种类型的 CPU 转移到另一种类型. 建议的指标包括 CPU 以最大速度工作的时间量、CPU 使用最大内存的时间量、一段时间内的平均利用率以及工作负载从一个 CPU 转移的更一般类别另一个基于与任务执行相关的未指定指标等. 当 CPU 确定工作负载应从 CPU A 转移到 CPU B 时, 当前执行工作的内核(在本例中为 CPU A)将进入空闲或停止状态. CPU A 的架构状态保存到内存并由 CPU B 加载, 然后继续该过程. 通过这种 Save/Restore 的方式可以实现软件无感的快速迁移, 并且支持的迁移是双向的, 小核可以将任务迁移到大核, 反之亦然.

参见相关报道:

[效仿 Arm, AMD 也在做大小核芯片](https://mp.weixin.qq.com/s/ckDRWep-ih287wIsjddRrw).

[AMD “big.LITTLE” aka heterogeneous computing in Ryzen 8000 series](https://videocardz.com/newz/amd-patents-a-task-transition-method-between-big-and-little-processors).

[AMD reportedly working on its heterogeneous CPU design](https://technosports.co.in/2021/06/14/amd-reportedly-working-on-its-heterogeneous-cpu-design).

[Recently published patent hints at AMD hybrid CPU plans](https://m.hexus.net/tech/news/cpu/147956-recently-published-patent-hints-amd-hybrid-cpu-plans).


### 1.4.3 ARM big.LITTLE & DynamIQ
-------




#### 1.4.4 理论研究
-------

2012 年 Intel 发布了一篇关于大小核的论文 [Scheduling Heterogeneous Multi-Cores through Performance Impact Estimation (PIE)](http://www.jaleels.org/ajaleel/publications/isca2012-PIE.pdf). 论文中小核使用了 In-Order 的微架构, 大核则使用了 Out-Order 的微架构, 传统的想法都是计算密集型(compute-intensive)的进程跑到大核上, 访存密集型(memory-intensive)的进程跑到小核上去. 但是论文中提出了不一样的想法. 因为内存访问的密集程序并不是一个能反应进程的负载对 CPU 需求的好指标.

1.  如果应用的指令流拥有较高水平的 ILP, 那么它在 In-Order 的小核上, 也能获得很好的性能.

2.  如果应用有较大的 MLP(访存的并发) 以及动态的 ILP, 那么它在 Out-Order 的大核上, 才能获得很好的性能.

因此在大小核上应该分析和利用进程的 ILP/MLP 等特征, 这样才能获得更好的性能.

这应该是 Intel 关于大小核混合架构(single-ISA heterogeneous multi-cores)最早的研究, 此后 Intel 发表了陆续发表了关于大小核混合架构的多项研究.



## 1.5 RAPL
-------

[RUNNING AVERAGE POWER LIMIT – RAPL](https://01.org/blogs/2014/running-average-power-limit-%E2%80%93-rapl)

[Understanding Intel's RAPL Driver On Linux](https://www.phoronix.com/scan.php?page=news_item&px=MTcxMjY)


## 1.6 安全相关
-------

### 1.6.1 SGX(Intel Software Guard eXtensions ,英特尔软件保护扩展)
-------


2013 年, Intel 推出 SGX(software guard extensions) 指令集扩展, 能创造出一个可信执行环境来保护代码和数据, 即使 root 权限也无法访问. 这样应用程序可以使用它来隔离代码和数据的特定可信区域, 防止底层OS被 compromise 以后对自己的攻击,
同时在软件的管理上也可以不用信任云供应商. 比较符合当前要解决的云计算安全问题, 比如给安全敏感服务存放密钥等.

SGX 旨在以硬件安全为强制性保障, 不依赖于固件和软件的安全状态, 提供用户空间的可信执行环境, 通过一组新的指令集扩展与访问控制机制, 实现不同程序间的隔离运行, 保障用户关键代码和数据的机密性与完整性不受恶意软件的破坏.

不同于其他安全技术, SGX 可信计算(trusted computing base, 简称TCB) 仅包括硬件, 避免了基于软件的 TCB 自身存在软件安全漏洞与威胁的缺陷, 极大地提升了系统安全保障; 此外, SGX可保障运行时的可信执行环境, 恶意代码无法访问与篡改其他程序运行时的保护内容, 进一步增强了系统的安全性; 基于指令集的扩展与独立的认证方式, 使得应用程序可以灵活调用这一安全功能并进行验证.作为系统安全领域的重大研究进展, Intel SGX是基于CPU的新一代硬件安全机制, 其健壮、可信、灵活的安全功能与硬件扩展的性能保证, 使得这项技术具有广阔的应用空间与发展前景.目前, 学术界和工业界已经对SGX技术展开了广泛的研究, Intel也在其最新的第六代CPU中加入了对SGX的支持.


[SGX技术的分析和研究](http://www.jos.org.cn/html/2018/9/5594.htm)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/11/13 | Jarkko Sakkinen <jarkko@kernel.org> | [Intel SGX foundations](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=bc4bac2ecef0e47fd5c02f9c6f9585fd477f9beb) | 20201112220135.165028-1-jarkko@kernel.org | v41 ☑✓ 5.11-rc1 | [LORE v41,0/24](https://lore.kernel.org/all/20201112220135.165028-1-jarkko@kernel.org) |
| 2021/03/19 | Kai Huang <kai.huang@intel.com> | [KVM SGX virtualization support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fe7e948837f312d87853b3fce743795d1ae3715a) | cover.1616136307.git.kai.huang@intel.com | v3 ☑✓ 5.13-rc1 | [LORE v3,0/25](https://lore.kernel.org/all/cover.1616136307.git.kai.huang@intel.com) |
| 2021/04/12 | Kai Huang <kai.huang@intel.com> | [KVM SGX virtualization support (KVM part)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fe7e948837f312d87853b3fce743795d1ae3715a) | cover.1618196135.git.kai.huang@intel.com | v5 ☑✓ 5.13-rc1 | [LORE v5,0/11](https://lore.kernel.org/all/cover.1618196135.git.kai.huang@intel.com) |
| 2021/11/02 | Catalin Marinas <catalin.marinas@arm.com> | [Basic recovery for machine checks inside SGX](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=3ad6fd77a2d62e8f4465b429b65805eaf88e1b9e) | 支持混合微架构的 CPU(Alder Lake CPU) | v11 ☑✓ 5.17-rc1 | [Patchwork v11,0/7](https://patchwork.kernel.org/project/linux-mm/cover/20211026220050.697075-1-tony.luck@intel.com) |

### 1.6.2 CET
-------

[New Linux Patch Series Provides A Fresh Take On Intel Indirect Branch Tracking](https://www.phoronix.com/scan.php?page=news_item&px=Linux-x86-IBT-New-Shot)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/30 | Yu-cheng Yu <yu-cheng.yu@intel.com> | [Control-flow Enforcement: Shadow Stack](https://patchwork.kernel.org/project/linux-mm/cover/20210830181528.1569-1-yu-cheng.yu@intel.com) | 控制流执行(CET) 是英特尔处理器的一个新特性, 它可以阻止面向返回/跳转的编程攻击. 详情见 ["Intel 64 and IA-32架构软件开发人员手册"](https://software.intel.com/content/www/us/en/develop/download/intel-64-and-ia-32-architectures-sdm-combined-volumes-1-2a-2b-2c-2d-3a-3b-3c-3d-and-4.html). CET 可以保护应用程序和内核. 这是 CET 的第一部分, 本系列仅支持应用程序级保护, 并进一步分为阴影堆栈和间接分支跟踪. | v30 ☐ | [Patchwork v30,00/32](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) |
| 2021/08/30 | Yu-cheng Yu <yu-cheng.yu@intel.com> | [Control-flow Enforcement: Indirect Branch Tracking](https://patchwork.kernel.org/project/linux-mm/cover/20210830182221.3535-1-yu-cheng.yu@intel.com) | 控制流执行(CET) 是英特尔处理器的一个新特性, 它可以阻止面向返回/跳转的编程攻击. 详情见 ["Intel 64 and IA-32架构软件开发人员手册"](https://software.intel.com/content/www/us/en/develop/download/intel-64-and-ia-32-architectures-sdm-combined-volumes-1-2a-2b-2c-2d-3a-3b-3c-3d-and-4.html). 这是 CET 的第二部分, 支持间接分支跟踪(IBT). 它是建立在阴影堆栈系列之上的. | v1 ☐ | [Patchwork v30,00/10](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) |
| 2021/11/22 | Peter Zijlstra <peterz@infradead.org> | [x86: Kernel IBT beginnings](https://lore.kernel.org/lkml/20211122170301.764232470@infradead.org) | IBT 的基础支持, 功能不完整, 但是够精简, 引入 CONFIG_X86_IBT 控制, 添加了 X86_IBT build 选项, 为编译器启用了 "-fcf-protection=branch" 支持, 并支持 objtool, 用于为可能的间接分支目标添加 ENDBR 指令以及内核入口点添加了 ENDBR 结束分支指令. | v11 ☐ | [Patchwork v11,0/7](https://lore.kernel.org/lkml/20211122170301.764232470@infradead.org) |

## 1.7 PPIN
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2020/03/21 | Wei Huang <wei.huang2@amd.com> | [x86/mce/amd: Add PPIN support for AMD MCE](https://lore.kernel.org/all/20200321193800.3666964-1-wei.huang2@amd.com) | 20200321193800.3666964-1-wei.huang2@amd.com | v3 ☑✓ | [LORE v3,0/1](https://lore.kernel.org/all/20200321193800.3666964-1-wei.huang2@amd.com) |
| 2022/01/31 | Tony Luck <tony.luck@intel.com> | [PPIN (Protected Processor Inventory Number) updates](https://lore.kernel.org/all/20220131230111.2004669-1-tony.luck@intel.com) | 受保护处理器库存编号(Protected Processor Inventory Number, PPIN), 有时也称为 "受保护处理器标识号(Protected Processor Identification Number)是处理器在制造时设置的一个编号, 用于唯一地标识给定的处理器. 可以通过 PPIN 追溯芯片生产的车间和生产以及运行环境, 这有助于帮助缺陷诊断. Intel 之前提交了 Linux 补丁, 用于在服务器管理员遇到 MCE 错误时显示 PPIN, 以帮助跟踪问题发生在哪个物理 CPU 上, 但是只有在出现错误时才会打印. 这组补丁使得服务器管理员可以更容易地访问 PPIN. (直接从 `sys/devices/system/cpu/cpu*/topology/ppin` 即可读取). 参见 phoronix 报道 [Convenient Intel PPIN Reporting To Come With Linux 5.18](https://www.phoronix.com/scan.php?page=news_item&px=Intel-PPIN-Linux-5.18) | v3 ☑✓ v5.18-rc1 | [LORE v3,0/5](https://lore.kernel.org/all/20220131230111.2004669-1-tony.luck@intel.com) |



## 1.8 AMD
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/10 | Brijesh Singh <brijesh.singh@amd.com> | [Add AMD Secure Nested Paging (SEV-SNP) Guest Support](https://patchwork.kernel.org/project/linux-mm/cover/20211110220731.2396491-1-brijesh.singh@amd.com) | Guest 支持 AMD SEV SNP.  | v7 ☐ | [Patchwork v7,00/45](https://patchwork.kernel.org/project/linux-mm/cover/20211110220731.2396491-1-brijesh.singh@amd.com) |




# 2 ARM64
-------



ARM & Linaro [Kernel versions highlights](https://developer.arm.com/tools-and-software/open-source-software/linux-kernel)

ARM64 架构文档地址下载 [](https://developer.arm.com/architectures/cpu-architecture)


[Memory Layout on AArch64 Linux](https://www.kernel.org/doc/html/latest/arm64/memory.html)

[ARM64 Instruction](https://courses.cs.washington.edu/courses/cse469/19wi/arm64.pdf)

[硬件特性列表](https://developer.arm.com/architectures/cpu-architecture/a-profile/exploration-tools/feature-names-for-a-profile)

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

[郭健： 进程切换分析之——TLB处理](https://blog.csdn.net/21cnbao/article/details/110675822)

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
[进程切换分析（2）：TLB 处理](http://www.wowotech.net/process_management/context-switch-tlb.html)

[stackoverflow: What is tlb shootdown ?](https://stackoverflow.com/questions/3748384/what-is-tlb-shootdown)

[深入理解 Linux 内核--jemalloc 引起的 TLB shootdown 及优化](https://blog.csdn.net/weixin_46837673/article/details/105379768)

在多核系统中, 进程可以调度到任何一个 CPU 上执行, 从而导致 task 处处留情. 如果 CPU 支持 PCID 并且在进程切换的时候不 flush tlb, 那么系统中各个 CPU 中的 tlb entry 则保留各种 task 的 tlb entry, 当在某个 CPU 上, 一个进程被销毁, 或者修改了自己的页表(修改了VA PA映射关系)的时候, 我们必须将该 task 的相关 tlb entry 从系统中清除出去. 这时候, 不仅仅需要 flush 本 CPU 上对应的 TLB entry, 还需要 shootdown 其他 CPU 上的和该 task 相关的 tlb 残余.

通知其他 CPU shootdown 到指定 task 的 TLB entry 有两种方式: 广播 和 IPI.

1.  广播的方式, 当进程销毁或者页表修改时, 则本地 CPU 通过广播的方式将请求发送出去. 各个 CPU 接受到请求之后, 如果 TLB 中包含了对应的 TLB entry, 则 flush, 否则则抛弃此请求. 这种方式主要由硬件参与完成, 在 shootdown 请求比较少的时候, 硬件处理及时且迅速, 开销也小.

2.  IPI 的方式, 需要先由软件(内核)识别到哪些 CPU 中包含了这些需要 flush 的 TLB entry, 然后由本地 CPU 通过 IPI 请求的方式, 通知对应的 CPU 进行 shootdown 操作. 这种方式由硬件软件交互完成, 在 CPU 核数比较多, 且shootdown 请求比较多的时候, 可能造成 TLB entry shootdown 广播风暴, 硬件(NM)处理不过来, 造成性能下降. 此时对于单个 task 的 TLB entry 来说, 残留的 CPU 往往是明确且有限的, 通过 IPI 的方式, 给硬件压力反而会小很多, 这时采用 IPI 的方式性能反而会好.



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/07/11 | David Daney <ddaney.cavm@gmail.com>/<david.daney@cavium.com> | [arm64, mm: Use IPIs for TLB invalidation.](http://lists.infradead.org/pipermail/linux-arm-kernel/2015-July/355866.html) | 在 Cavium ThunderX (ARM64) 的机器上, 某些场景下不能使用广播 TLB, TLB 广播风暴会导致严重的性能问题, 所以我们在必要时使用 IPIs. 测试发现, 它还使内核构建的速度更快. | v1 ☐ | [Patchwork 0/3](http://lists.infradead.org/pipermail/linux-arm-kernel/2015-July/355866.html) |
| 2016/08/04 | Matthias Brugger <mbrugger@suse.com> | [arm64, mm: Use IPIs for TLB invalidation.](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1470302117-32296-3-git-send-email-mbrugger@suse.com/) | NA | v1 ☐ | [Patchwork 2/4](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1470302117-32296-3-git-send-email-mbrugger@suse.com) |
| 2020/11/03 | Nianyao Tang <tangnianyao@huawei.com> | [KVM: arm64: Don't force broadcast tlbi when guest is running](https://lists.cs.columbia.edu/pipermail/kvmarm/2020-November/043071.html) | KVM 当 guest 在运行的时候, 避免 tlbi 广播. | RFC v1 ☐ | [Patchwork RTC](https://lore.kernel.org/linux-arm-kernel/1603331829-33879-1-git-send-email-zhangshaokun@hisilicon.com) |
| 2019/06/17 |  Takao Indoh <indou.takao@jp.fujitsu.com> | [arm64: Introduce boot parameter to disable TLB flush instruction within the same inner shareable domain](hhttps://lists.cs.columbia.edu/pipermail/kvmarm/2020-November/043071.html) | 富士通的开发人员发现 ARM64 TLB.IS 广播在 HPC 上造成了严重的性能下降, 因此新增一个 disable_tlbflush_is 参数来禁用 TLB.IS 广播, 使用原始的 TLB IPI 方式. | RFC v1 ☐ | [Patchwork 0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20190617143255.10462-1-indou.takao@jp.fujitsu.com), [LORE](https://lore.kernel.org/linux-arm-kernel/20190617143255.10462-1-indou.takao@jp.fujitsu.com) |
| 2016/10/24 | Marc Zyngier <marc.zyngier@arm.com> | [arm/arm64: KVM: Perform local TLB invalidation when multiplexing vcpus on a single CPU](https://patchwork.kernel.org/project/kvm/patch/1477323088-18768-1-git-send-email-marc.zyngier@arm.com) | KVM 当 guest 在运行的时候, 避免 tlbi 广播. | RFC v1 ☐ | [Patchwork RTC](https://lore.kernel.org/linux-arm-kernel/1603331829-33879-1-git-send-email-zhangshaokun@hisilicon.com) |
| 2020/02/23 | Andrea Arcangeli <aarcange@redhat.com> | [arm64: tlb: skip tlbi broadcast v2](https://lore.kernel.org/all/20200223192520.20808-1-aarcange@redhat.com) |20200223192520.20808-1-aarcange@redhat.com | v1 ☐ | [LORE](https://lore.kernel.org/all/20200223192520.20808-1-aarcange@redhat.com) |

> 注: x86 由于没有 tlb IS 方案, 因此只能采用 IPI 的方式来完成 TLB shootdown.

目前 ARM64 中 TLUSH TLB 的接口:

| 接口 | 描述 |
|:---:|:---:|
| flush_tlb_all | 无效掉所有的 TLB entry.(包括内核的和用户态的) |
| flush_tlb_mm | 无效掉 mm_struct 指向的所有 TLB entry.<br>1. mm_struct 指向的都是进程的用户态空间.<br>2. ARM64 上每个 mm_struct 有自己单独的 ASID. 当前只需要对指定的 ASID 执行 TLB.IS 即可 |
| flush_tlb_range | 无效掉用户态地址 start ~ end 区间内的所有 tlb entry |
| flush_tlb_kernel_range | 无效掉内核态 start ~ end 区间内的所有 TLB entry |
| local_flush_tlb_all | 无效掉本 CPU 上所有的 TLB entry. 无需使用 TLB.IS |

## 2.3 指令加速
-------

### 2.3.1 LSE
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/07/24 | Will Deacon <will.deacon@arm.com> | [arm64: support for 8.1 LSE atomic instructions](https://lwn.net/Articles/650900) | 为 Linux 内核添加了对[新原子指令(LSE atomic instructions)的支持](https://mysqlonarm.github.io/ARM-LSE-and-MySQL), 这是作为 ARMv8.1 中大系统扩展(LSE-LSE atomic instructions)的一部分引入的. 新的指令可以在编译时通过 CONFIG_ARM64_LSE_ATOMICS 选项配置出来.<br>之前测试发现, 核少的时候关 LSE 性能更好, 核多的时候, 开 LSE 性能更好. | v1 ☑ 4.3-rc1 | [2015/07/13 Patchwork 00/18](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20200625080314.230-1-yezhenyu2@huawei.com)<br>*-*-*-*-*-*-*-* <br>[2015/07/24 Patchwork v2,07/20](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1437734531-10698-8-git-send-email-will.deacon@arm.com/) |
| 2021/12/10 | Mark Brown <broonie@kernel.org> | [arm64: atomics: cleanups and codegen improvements](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20211027184424.166237-1-broonie@kernel.org) | SME 指令的支持. | v5 ☑ [5.17-rc1](https://lore.kernel.org/lkml/20220106185501.1480075-1-catalin.marinas@arm.com) | [Patchwork 0/5](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20211210151410.2782645-1-mark.rutland@arm.com) |


### 2.3.2 SVE
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/10/30 | Mark Brown <broonie@kernel.org> | [ARM Scalable Vector Extension (SVE)](https://patchwork.ozlabs.org/project/glibc/cover/1509101470-7881-1-git-send-email-Dave.Martin@arm.com) | SVE 指令的支持. 目前不支持 KVM guest 使用 SVE 指令. 这种将被捕获并作为未定义的指令执行反映给 guest. SVE 在 guest 可见的 CPU 功能寄存器视图中是隐藏的, 因此 guest 不会期望它工作. | v4 ☑ 4.15-rc1 | [Patchwork v4,00/28](https://patchwork.ozlabs.org/project/glibc/cover/1509101470-7881-1-git-send-email-Dave.Martin@arm.com)<br>*-*-*-*-*-*-*-* <br>[Patchwork v5,01/30](https://patchwork.ozlabs.org/project/glibc/patch/1509465082-30427-2-git-send-email-Dave.Martin@arm.com) |
| 2019/03/29 | Mark Brown <broonie@kernel.org> | [KVM: arm64: SVE guest support](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1553864452-15080-1-git-send-email-Dave.Martin@arm.com) | KVM guest SVE 指令的支持. | v7 ☑ 5.2-rc1 | [Patchwork v7,00/27](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1553864452-15080-1-git-send-email-Dave.Martin@arm.com) |
| 2019/04/18 | Mark Brown <broonie@kernel.org> | [KVM: arm64: SVE cleanups](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1555603631-8107-1-git-send-email-Dave.Martin@arm.com) | KVM guest SVE 指令的支持. | v2 ☑ 5.2-rc1 | [Patchwork v2,00/14](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1555603631-8107-1-git-send-email-Dave.Martin@arm.com) |
| 2019/04/18 | Mark Brown <broonie@kernel.org> | [arm64: Expose SVE2 features for userspace](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1555609298-10498-1-git-send-email-Dave.Martin@arm.com) | 支持向用户空间报告 SVE2 的存在及其可选功能. 同时为 KVM 虚拟化 guest 提供了 SVE2 的可见性. | v2 ☑ 5.2-rc1 | [Patchwork v2,00/14](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1555609298-10498-1-git-send-email-Dave.Martin@arm.com) |


### 2.3.3 SME
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/10/27 | Mark Brown <broonie@kernel.org> | [arm64/sme: Initial support for the Scalable Matrix Extension](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20211027184424.166237-1-broonie@kernel.org) | SME 指令的支持. v7 版本前 6 个 [prepare 的补丁](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-AArch64)先合入了 [5.17-rc1](https://lore.kernel.org/lkml/20220106185501.1480075-1-catalin.marinas@arm.com) | v5 ☐ | [LORE v3,00/42](https://lore.kernel.org/all/20211019172247.3045838-1-broonie@kernel.org)<br>*-*-*-*-*-*-*-* <br>[2021/10/27 Patchwork v5,00/38](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20211027184424.166237-1-broonie@kernel.org)<br>*-*-*-*-*-*-*-* <br>[2021/12/10 Patchwork v7,00/37](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20211210184133.320748-1-broonie@kernel.org) |

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

[Arm Chips Vulnerable to PAN Bypass – "We All Know it’s Broken"](https://techmonitor.ai/techonology/hardware/arm-pan-bypass)
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

2016 年 10 月, ARMV8.3-A 指令集中加入了[指针认证(Pointer Authentication)机制](https://www.kernel.org/doc/html/latest/arm64/pointer-authentication.html
), 在使用寄存器的值作为指针访问数据或代码之前验证其内容, 目的是为了对抗 ROP/JOP 攻击.

[ARM pointer authentication](https://lwn.net/Articles/718888)
[开源快递: ARM 指针身份认证 (arm pointer authentication)](https://www.cnblogs.com/liuhailong0112/p/14258844.html)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/12/07 | Kristina Martsenko <kristina.martsenko@arm.com> | [ARMv8.3 pointer authentication userspace support](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20181207183931.4285-1-kristina.martsenko@arm.com) | RISC-V高级核心本地中断(Advacned Core Local Interruptor-ACLINT) 支持 | v8 ☐ | [Patchwork v5,00/17](https://patchwork.kernel.org/project/linux-riscv/cover/20210830041729.237252-1-anup.patel@wdc.com)<br>*-*-*-*-*-*-*-*<br>[Patchwork v6,00/13](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20181207183931.4285-1-kristina.martsenko@arm.com) |
| 2019/03/19 | Kristina Martsenko <kristina.martsenko@arm.com> | [arm64: pac: Optimize kernel entry/exit key installation code paths](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20181207183931.4285-1-kristina.martsenko@arm.com) | PAC 合入后, svc(SYSCALL) 进入和退出以及进程切换的过程中, 会进行 key 的切换, 会存在 10+ 个 MSR 操作, 因此造成了性能劣化. | v8 ☑ 5.13-rc1 | [Patchwork v8,3/3](https://patchwork.kernel.org/project/linux-arm-kernel/patch/2d653d055f38f779937f2b92f8ddd5cf9e4af4f4.1616123271.git.pcc@google.com)<br>*-*-*-*-*-*-*-*<br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b90e483938ce387c256e03fb144f82f64551847b) |

## 2.7 Linux to Apple Silicon Macs
-------

2020 年 11 月 11 日在苹果新品发布会上发布了适用于部分Mac、iPad设备 ARM64 芯片 Apple M1, 社区开始有不少团队开始着手于 Linux 支持 Apple M1 芯片. 目前有两个团队在做这块工作 Corellium 的 [corellium/linux-m1](https://github.com/corellium/linux-m1) 以及 [AsahiLinux/linux](https://github.com/AsahiLinux/linux). 其中

*   [Corellium](https://github.com/corellium) 一直致力于为用户提供虚拟 iOS 系统桌面解决方案, 这个团队从 A10 芯片开始就开始做 Linux 的适配, 因此 M1 刚出来就做了快速的适配.

*   [AsahiLinux]() 国外一名资深操作系统移植专家 Hector Martin(网名为 Marcan) 主导发起的一个开源项目, 其目标是让用户可以在 Apple M1 上把基于 Linux 的发行版作为日常操作系统来使用.

相关报道

[Rewritten Apple Silicon CPUFreq Driver Posted For Linux](https://www.phoronix.com/scan.php?page=news_item&px=Apple-SoC-CPUFreq-Driver-v2)

## 2.8 clocksource
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/10/17 | Marc Zyngier <maz@kernel.org> | [clocksource/arm_arch_timer: Add basic ARMv8.6 support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ec8f7f3342c88780d682cc2464daf0fe43259c4f) | NA | v4 ☑ 5.16-rc1 | [LORE v4,00/17](https://lore.kernel.org/all/20211017124225.3018098-1-maz@kernel.org) |


# 3 RISC-V
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/10 | Anup Patel <anup.patel@wdc.com> | [RISC-V CPU Idle Support](https://patchwork.kernel.org/project/linux-riscv/cover/20210610052221.39958-1-anup.patel@wdc.com) | RISC-V高级核心本地中断(Advacned Core Local Interruptor-ACLINT) 支持 | v8 ☐ | [2021/06/10Patchwork v7,0/8](https://patchwork.kernel.org/project/linux-riscv/cover/20210830041729.237252-1-anup.patel@wdc.com)<br>*-*-*-*-*-*-*-*<br>[LWN v8, 0/8](https://lwn.net/Articles/872513) |
| 2021/08/30 | Anup Patel <anup.patel@wdc.com> | [Linux RISC-V ACLINT Support](https://patchwork.kernel.org/project/linux-riscv/cover/20210830041729.237252-1-anup.patel@wdc.com) | RISC-V高级核心本地中断(Advacned Core Local Interruptor-ACLINT) 支持 | v3 ☐ | [Patchwork RFC,v3,00/11](https://patchwork.kernel.org/project/linux-riscv/cover/20210610052221.39958-1-anup.patel@wdc.com) |


# 4 CPU 漏洞
-------

## 4.1 Straight Line Speculation
-------

[Blocking straight-line speculation — eventually](https://lwn.net/Articles/877845)

Google 的 SafeSide 小组发现 ARM CPU 存在新的投机执行漏洞, 可导致侧信道攻击. 研究人员在 Armv8-A(Cortex-A) CPU 体系结构中发现了一个名为直线推测( Straight-Line Speculation , SLS) 的新漏洞, 被追踪为 [CVE-2020-13844](https://nvd.nist.gov/vuln/detail/CVE-2020-13844). 该漏洞可导致攻击者对 ARM 架构处理器进行侧边信道攻击(SCA).

让目标处理器通过预先访问数据来提升性能, 然后再扔掉所有使用过的计算分支(computational branches), 而 SLS 则通过类似的侧道攻击就能让黑客从处理器直接获得(窃取)重要数据.

SLS 被认为是 Spectre 漏洞的变体, 但二者的攻击范围略有不同, SLS 漏洞仅影响 Arm Armv-A 处理器, 而 Spectre 漏洞影响所有主要芯片制造商的 CPU. 到目前为止, 该漏洞还没有在野利用.

[Arm CPUs Hit By Straight Line Speculation Vulnerability, LLVM Adds Initial Mitigation](https://www.phoronix.com/scan.php?page=news_item&px=Arm-Straight-Line-Speculation)

很快在 [GCC](https://gcc.gnu.org/pipermail/gcc-patches/2020-June/547520.html) 和 [LLVM](https://reviews.llvm.org/rG9c895aea118a2f50ca8413372363c3ff6ecc21bf) 编译器中推出了针对 SLS 的保障措施, 通过在易受 SLS 影响的指令周围插入投机障碍(SB)指令或其他 DSB + ISB 指令来减少直线预测.

[LLVM Adds Additional Protections For Arm's SLS Speculation Vulnerability Mitigation](https://www.phoronix.com/scan.php?page=news_item&px=Arm-SLS-More-In-LLVM)

11 月 17 日, 将 x86/x86_64 的 SLS 缓解选项 -mharden-SLS 合并到 GCC 12 Git 上, 预计不久将推出内核补丁, 将 -mharden-SLS 缓解选项作为对 x86 cpu 最新的安全保护. 这个选项包括 none、all、return 或 indirect-branch 四个值, x86/x86_64 架构上的原理是通过在函数返回和间接分支之后添加 INT3 断点指令, 来减少函数返回和间接分支的直线推测(SLS). 参见 [Linux + GCC/Clang Patches Coming For Straight-Line Speculation Mitigation On x86/x86_64](https://www.phoronix.com/scan.php?page=news_item&px=Straight-Line-Speculation-x86), [x86 Straight Line Speculation CPU Mitigation Appears For Linux 5.17](https://www.phoronix.com/scan.php?page=news_item&px=x86-SLS-Mitigation-5.17)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/10 | Anup Patel <anup.patel@wdc.com> | [ARM: Implement Clang's SLS mitigation](https://patchwork.kernel.org/project/linux-security-module/patch/20210212051500.943179-1-jiancai@google.com) | NA | v8 ☐ | [2021/06/10Patchwork v7,0/8](https://patchwork.kernel.org/project/linux-riscv/cover/20210830041729.237252-1-anup.patel@wdc.com)<br>*-*-*-*-*-*-*-*<br>[LWN v8, 0/8](https://lwn.net/Articles/872513) |
| 2021/12/04 | Peter Zijlstra <peterz@infradead.org> | [x86: Add stright-line-speculation mitigations](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e463a09af2f0677b9485a7e8e4e70b396b2ffb6f) | 20211204134338.760603010@infradead.org | v2 ☑✓ 5.17-rc1 | [LORE v2,0/6](https://lore.kernel.org/all/20211204134338.760603010@infradead.org) |


## 4.2  Control-Flow Enforcement Technology (CET)
-------

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



## 6.2 RANOM
-------


### 6.2.1 Arm True Random Number Generator Firmware Interface
-------

ARM 规范 [DEN0098](https://developer.arm.com/documentation/den0098/latest) 中描述的 ARM 架构的 TRNG 固件接口定义了一个基于 ARM SMCCC 的接口, 该接口由固件提供给真实随机数生成器.

Arm True Random Number Generator Firmware Interface 1.0 于去年发布, 最终由Arm TrustZone的TRNG或其他原始噪声等硬件设备
提供支持.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/12/11 | Vladimir Murzin <vladimir.murzin@arm.com> | [ARM: arm64: Add SMCCC TRNG entropy service](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=38db987316a38a3fe55ff7f5f4653fcb520a9d26) | NA | v4 ☑ 5.12-rc1 | [LORE v4,0/5](https://lore.kernel.org/lkml/20201211160005.187336-1-andre.przywara@arm.com) |
| 2021/07/26 | Vladimir Murzin <vladimir.murzin@arm.com> | [hwrng: Add Arm SMCCC TRNG based driver](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0888d04b47a165ae8c429c6fe11b3c43f5017f31) | 添加一个"arm_smccc_trng"驱动程序, 并允许将熵暴露给用户空间. ARM 规范定义了此真正的随机数生成器固件接口, 用于为随机池设定种子, 也可由 KVM Guest 使用. 使用此新驱动程序, 可以通过 `/dev/hwrng` 公开来自此固件接口的熵. 反过来, 这对于能够使用 rng-tool 的 rngtest 实用程序等来验证熵的质量非常有用. 参见 [Arm SMCCC TRNG Driver Queued Ahead Of Linux 5.15](https://www.phoronix.com/scan.php?page=news_item&px=Arm-SMCCC-TRNG-Linux-5.15). | v3 ☑ 5.15_rc1 | [LKML v3,0/2](https://lkml.org/lkml/2021/7/26/1571), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0888d04b47a165ae8c429c6fe11b3c43f5017f31) |


### 6.2.2 LRNG
-------

[FIPS-compliant random numbers for the kernel](https://lwn.net/Articles/877607)

[smuellerDD/lrng](https://github.com/smuellerDD/lrng)

[Uniting the Linux random-number devices](https://lwn.net/Articles/884875)

[Random number generator enhancements for Linux 5.17 and 5.18](https://www.zx2c4.com/projects/linux-rng-5.17-5.18)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/21 | "Stephan Müller" <smueller@chronox.de> | [/dev/random - a new approach](https://lore.kernel.org/lkml/2036923.9o76ZdvQCi@positron.chronox.de) | 随机数实现改进. | v1 ☐ | [Patchwork v43 00/15](https://lore.kernel.org/lkml/2036923.9o76ZdvQCi@positron.chronox.de) |



## 6.3 总线
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


## 6.4 CPU IDLE(C-state)
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/04/05 | Wyes Karny <wyes.karny@amd.com> | [x86: Prefer MWAIT over HALT on AMD processors](https://lore.kernel.org/all/20220405130021.557880-1-wyes.karny@amd.com) | Monitor Wait "MWAIT" 指令可用于电源管理目的, 以提示处理器在等待事件或 MONITOR 存储操作完成时可以进入指定的目标 C 状态. MWAIT 的使用旨在比 HALT 指令更有效. Intel Core 2 及更高版本已经将 MWAIT 置于 HALT 指令之上, 以进入 C1 状态. 但是 AMD CPU 此代码路径中依旧使用 HALT. 这个补丁使 AMD 使用 MWAIT 指令而不是 HALT, 当系统未使用 CPU 空闲驱动程序时, 无论是 BIOS 禁用 C 状态还是驱动程序不属于内核构建. 反过来, 这可以将受影响系统的退出延迟提高约 21%. 参见 [](https://www.phoronix.com/scan.php?page=news_item&px=AMD-MWAIT-Over-HALT-2022). | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20220405130021.557880-1-wyes.karny@amd.com) |


## 6.5 memory model
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/07/08 | Jiri Olsa <jolsa@redhat.com> | [memory barrier: adding smp_mb__after_lock](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ad46276952f1af34cd91d46d49ba13d347d56367) | TODO | v1 ☑✓ 2.6.31-rc3 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ad46276952f1af34cd91d46d49ba13d347d56367) |
| 2013/08/12 | Oleg Nesterov <oleg@redhat.com> | [sched: fix the theoretical signal_wake_up() vs schedule() race](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e0acd0a68ec7dbf6b7a81a87a867ebd7ac9b76c4) | 在 try_to_wake_up 路径引入了 smp_mb__before_spinlock(), 入口位置的 smp_wmb() 就被替换为 smp_mb__before_spinlock(). | v1 ☑✓ v3.11-rc6 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e0acd0a68ec7dbf6b7a81a87a867ebd7ac9b76c4) |
| 2017/08/02 | Peter Zijlstra <peterz@infradead.org> | [Getting rid of smp_mb__before_spinlock](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ae813308f4630642d2c1c87553929ce95f29f9ef) | 本系列删除了 smp_mb_before_spinlock() 用户, 并将调度路径下转换为使用 smp_mb_after_spinlock(), 从而在相同数量的障碍下提供更多保障. | v1 ☑✓ 4.14-rc1 | [LORE v1,0/4](https://lore.kernel.org/all/20170802113837.280183420@infradead.org) |
| 2017/08/07 | Byungchul Park <byungchul.park@lge.com> | [lockdep: Implement crossrelease feature](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ef0758dd0fd70b98b889af26e27f003656952db8) | 1502089981-21272-1-git-send-email-byungchul.park@lge.com | v8 ☑✓ 4.14-rc1 | [LORE v8,0/14](https://lore.kernel.org/all/1502089981-21272-1-git-send-email-byungchul.park@lge.com) |


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/07/16 | Paul E. McKenney <paulmck@linux.vnet.ibm.com> | [Updates to the formal memory model](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=71b7ff5ebc9b1d5aa95eb48d6388234f1304fd19) | NA | v1 ☑✓ 4.19-rc1 | [LORE v1,0/14](https://lore.kernel.org/all/20180716180540.GA14222@linux.vnet.ibm.com) |

<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
