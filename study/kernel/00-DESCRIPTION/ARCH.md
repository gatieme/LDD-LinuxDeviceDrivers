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


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/18 | Tianjia Zhang <tianjia.zhang@linux.alibaba.com> | [add AES-NI/AVX2/x86_64 implementation](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) | 这个补丁集导出了 SM4 AESNI/AVX 算法实现的一些常用函数, 并用这些函数实现 AESNI/AVX2 的加速.<br>主要算法实现来自 libgcrypt 和 [Markku Juhani 的 SM4 AES-NI 工作](https://github.com/mjosaarinen/sm4ni). | v1 ☐ | [Patchwork 0/2](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) |
| 2021/07/30 | "Chang S. Bae" <chang.seok.bae@intel.com> | [x86: Support Intel Advanced Matrix Extensions](https://lore.kernel.org/lkml/20210730145957.7927-1-chang.seok.bae@intel.com) | 支持即将发布的英特尔[高级矩阵扩展(AMX)](https://software.intel.com/content/dam/develop/external/us/en/documents-tps/architecture-instruction-set-extensions-programming-reference.pdf), [AMX](https://software.intel.com/content/www/us/en/develop/documentation/cpp-compiler-developer-guide-and-reference/top/compiler-reference/intrinsics/intrinsics-for-intel-advanced-matrix-extensions-intel-amx-instructions.html) 由可配置的 TMM"TILE" 寄存器和操作它们的新 CPU 指令组成. TMUL (Tile matrix MULtiply)是第一个利用新寄存器的运算符, 我们预计将来会有更多的指令. | v9 ☐ | [Patchwork v9,00/26](https://lore.kernel.org/lkml/20210730145957.7927-1-chang.seok.bae@intel.com) |
| 2021/08/30 | Yu-cheng Yu <yu-cheng.yu@intel.com> | [Control-flow Enforcement: Shadow Stack](https://patchwork.kernel.org/project/linux-mm/cover/20210830181528.1569-1-yu-cheng.yu@intel.com) | 控制流执行(CET) 是英特尔处理器的一个新特性, 它可以阻止面向返回/跳转的编程攻击. 详情见 ["Intel 64 and IA-32架构软件开发人员手册"](https://software.intel.com/content/www/us/en/develop/download/intel-64-and-ia-32-architectures-sdm-combined-volumes-1-2a-2b-2c-2d-3a-3b-3c-3d-and-4.html). CET 可以保护应用程序和内核. 这是 CET 的第一部分, 本系列仅支持应用程序级保护, 并进一步分为阴影堆栈和间接分支跟踪. | v30 ☐ | [Patchwork v30,00/32](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) |
| 2021/08/30 | Yu-cheng Yu <yu-cheng.yu@intel.com> | [Control-flow Enforcement: Indirect Branch Tracking](https://patchwork.kernel.org/project/linux-mm/cover/20210830182221.3535-1-yu-cheng.yu@intel.com) | 控制流执行(CET) 是英特尔处理器的一个新特性, 它可以阻止面向返回/跳转的编程攻击. 详情见 ["Intel 64 and IA-32架构软件开发人员手册"](https://software.intel.com/content/www/us/en/develop/download/intel-64-and-ia-32-architectures-sdm-combined-volumes-1-2a-2b-2c-2d-3a-3b-3c-3d-and-4.html). 这是 CET 的第二部分, 支持间接分支跟踪(IBT). 它是建立在阴影堆栈系列之上的. | v1 ☐ | [Patchwork v30,00/10](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) |



# 2 ARM64
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/18 | Tianjia Zhang <tianjia.zhang@linux.alibaba.com> | [arm64: Enable OPTPROBE for arm64](https://lore.kernel.org/linux-crypto/20210818033117.91717-1-tianjia.zhang@linux.alibaba.com) | 这个补丁集导出了 SM4 AESNI/AVX 算法实现的一些常用函数, 并用这些函数实现 AESNI/AVX2 的加速.<br>主要算法实现来自 libgcrypt 和 [Markku Juhani 的 SM4 AES-NI 工作](https://github.com/mjosaarinen/sm4ni). | v4 ☐ | [Patchwork v4,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20210818073336.59678-1-liuqi115@huawei.com) |


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
