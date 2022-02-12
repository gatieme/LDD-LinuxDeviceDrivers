---

title: Linux 内核历史的 Git 记录
date: 2021-02-15 00:32
author: gatieme
tags:
    - linux
    - tools
categories:
        - 技术积累
thumbnail:
blogexcerpt: FZF 是目前最快的模糊搜索工具. 使用golang编写. 结合其他工具(比如 ag 和 fasd)可以完成非常多的工作. 前段时间, 有同事给鄙人推荐了 FZF, 通过简单的配置, 配合 VIM/GIT 等工具食用, 简直事半功倍, 效率指数级提升, 因此推荐给大家.

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

2   **调测子系统**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***



# 1 UNWIND
-------

[探索Android平台ARM unwind技术](https://zhuanlan.zhihu.com/p/336916116)

[The Linux x86 ORC Stack Unwinder](https://www.codeblueprint.co.uk/2017/07/31/the-orc-unwinder.html)

[Linux 内核的栈回溯与妙用](https://blog.csdn.net/qq_40732350/article/details/83649888)

[Stack unwinding](https://zhuanlan.zhihu.com/p/280929169)

[CPU Architectures » x86-specific Documentation » 9. ORC unwinderView page source](https://www.kernel.org/doc/html/latest/x86/orc-unwinder.html)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/11/23 | Shile Zhang <shile.zhang@linux.alibaba.com> | [Speed booting by sorting ORC unwind tables at build time](https://lore.kernel.org/patchwork/cover/1162315) | ORC unwind有两个表, .orc_unwind_ip和.orc_unwind 二分搜索需要排序. 在构建时对其进行排序可以节省更多CPU周期有助于加快内核引导. 添加ORC表排序在一个独立的线程有助于避免更多的链接. | RFC v6 ☐ | [PatchWork v6](https://lore.kernel.org/patchwork/cover/1162315) |
| 2021/08/12 | Shile Zhang <shile.zhang@linux.alibaba.com> | [arm64: Reorganize the unwinder and implement stack trace reliability checks](https://lwn.net/Articles/866194) | 使所有堆栈遍历函数都使用 arch_stack_walk().<br>目前, ARM64 代码中有多个函数使用 start_backtrace() 和 unwind_frame() 遍历堆栈. 将它们全部转换为使用 arch_stack_walk(). 这使得维护更容易. | RFC v8 ☐ | [PatchWork RFC,v8,0/4](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20210812190603.25326-1-madvenka@linux.microsoft.com) |


# 2 unikernel
-------


*   KML(Kernel Mode Linux : Execute user processes in kernel mode)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/04/16 | Toshiyuki Maeda | [Kernel Mode Linux : Execute user processes in kernel mode](http://web.yl.is.s.u-tokyo.ac.jp/~tosh/kml) | 内核直接执行用户态程序. | v1 ☐ [4.0](http://web.yl.is.s.u-tokyo.ac.jp/~tosh/kml/kml/for4.x) | [Patch](https://github.com/sonicyang/KML) |
| 2018/11/23 | Hsuan-Chi Kuo <hckuo2@illinois.edu> | [Lupine: Linux in Unikernel Clothes](https://github.com/hckuo/Lupine-Linux) | 通过内核配置和(KML)间接清除系统调用的影响, 从而使得 Linux 的性能达到持平甚至优于 unikernel 的性能. | ☐ | [Github](https://github.com/hckuo/Lupine-Linux), [159_kuo_slides.pdf](https://www.eurosys2020.org/wp-content/uploads/2020/04/slides/159_kuo_slides.pdf) |


# 3 RONX
-------


https://lwn.net/Articles/422487/

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/01/30 | Arjan van de Ven <arjan@infradead.org> | [mark text section read-only](https://lore.kernel.org/patchwork/cover/45845) | 新增 CONFIG_RODATA, 将内核代码段设置为只读. 分别为 i386 和 x86_64 新增了实现. | v3 ☑ 2.6.16-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/45845), [commit1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=67df197b1a07944c2e0e40ded3d4fd07d108e110), [commit2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=63aaf3086baea7b94c218053af8237f9dbac5d05), [LKML](https://lkml.org/lkml/2005/11/11/274) |
| 2007/10/24 | Randy Dunlap <rdunlap@xenotime.net> | x86: unification of i386 and x86_64 Kconfig.debug | 将 i386 和 x86_64 的 CONFIG 和实现统一 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d013a27cb79a01b324f93adb275162c244cca2de) |
| 2008/01/30 | Ingo Molnar <mingo@elte.hu> | [x86: rodata config hookup](https://lore.kernel.org/patchwork/cover/224632) | 新增 DEBUG_RODATA_TEST 和 DEBUG_NX_TEST . | v13 ☑ 2.6.25-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/224632), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=aba8391f7323294e88e3a665513434aba4042a7d) |
| 2010/12/16 | Matthieu Castet <castet.matthieu@free.fr> | [x86: Add NX protection for kernel data](https://lore.kernel.org/patchwork/cover/224616) | 为内核数据段设置只读属性, x86 上实现. | v1 ☑ 2.6.38-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/224616), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5bd5a452662bc37c54fb6828db1a3faf87e6511c) |
| 2010/12/16 | Matthieu Castet <castet.matthieu@free.fr> | [x86 : Add NX protection for kernel data on 64 bit](https://lore.kernel.org/patchwork/cover/240901) | 为内核数据段设置只读属性, x86_64 上实现. | v1 ☐ 2.6.38-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/240901), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5bd5a452662bc37c54fb6828db1a3faf87e6511c) |
| 2010/12/16 | Laura Abbott <labbott@redhat.com> | [Add RO/NX protection for loadable kernel modules](https://lore.kernel.org/patchwork/cover/224632) | 实现 CONFIG_DEBUG_SET_MODULE_RONX, 限制模块中各个不同类型页面的读写和可执行权限. | v1 ☑ 2.6.38-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/224632), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=84e1c6bb38eb318e456558b610396d9f1afaabf0) |
| 2014/04/14 | Kees Cook <keescook@chromium.org> | [ARM: mm: allow text and rodata sections to be read-only](https://lore.kernel.org/patchwork/cover/456177) | 为 ARM 实现CONFIG_DEBUG_RODATA 特性, 设置内核代码段只读和数据段不可执行的. | v2 ☑ 3.19-rc1 | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/456177), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=80d6b0c2eed2a504f6740cd1f5ea76dc50abfc4) |
| 2014/08/19 | Laura Abbott <lauraa@codeaurora.org> | arm64: Add CONFIG_DEBUG_SET_MODULE_RONX support | 为 ARM64 实现 CONFIG_DEBUG_SET_MODULE_RONX, 限制模块中各个不同类型页面的读写和可执行权限. | v2 ☑ 3.18-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=11d91a770f1fff44dafdf88d6089a3451f99c9b6) |
| 2015/01/21 | Laura Abbott <lauraa@codeaurora.org> | arm64: add better page protections to arm64 | 为 ARM64 实现CONFIG_DEBUG_RODATA 特性, 设置内核代码段只读和数据段不可执行的. | v2 ☑ 4.0-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=da141706aea52c1a9fbd28cb8d289b78819f5436) |
| 2015/11/06 | Linus Torvalds <torvalds@linux-foundation.org> | [x86: don't make DEBUG_WX default to 'y' even with DEBUG_RODATA](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=54727e6e950aacd14ec9cd4260e9fe498322828c) | NA | v1 ☑ 4.4-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=54727e6e950aacd14ec9cd4260e9fe498322828c) |
| 2016/01/26 | Stephen Smalley <sds@tycho.nsa.gov> | [x86/mm: Warn on W^X mappings](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e1a58320a38dfa72be48a0f1a3a92273663ba6db) | NA | v1 ☑ 4.9-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e1a58320a38dfa72be48a0f1a3a92273663ba6db) |
| 2016/02/17 | Kees Cook <keescook@chromium.org> | [introduce post-init read-only memory](https://lore.kernel.org/patchwork/cover/648401) | 许多内容只在 `__init` 期间写入, 并且再也不会更改. 这些不能成为"const", 相反, 这组补丁引入了 `__ro_after_init` 来标记这些内存的方法, 并在 x86 和 arm vDSO 上使用它来. 此外<br>*-*-*-*-*-*-*-*<br>1. 还默认使能了 [DEBUG_RODATA](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9ccaf77cf05915f51231d158abfd5448aedde758)<br>2. 添加了一个新的内核参数(rodata)来帮助调试将来的使用, 可以[使用 rodata=n 来禁用此特性](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d2aa1acad22f1bdd0cfa67b3861800e392254454).<br>3. 并添加了一个名为 [lkdtm 测试驱动来验证结果](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7cca071ccbd2a293ea69168ace6abbcdce53098e). | v3 ☑ 4.6-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/648401) |
| 2017/01/29 | Laura Abbott <labbott@redhat.com> | [mm: add arch-independent testcases for RODATA](https://lore.kernel.org/patchwork/cover/755615) | 这个补丁为 RODATA 创建了主独立的测试用例. x86和x86_64都已经有了RODATA的测试用例, 用 CONFIG_DEBUG_RODATA_TEST 宏控制. 但是它们是特定于原型的, 因为它们直接使用内联汇编. 如果有人改变了CONFIG_DEBUG_RODATA_TEST的状态, 它会导致内核构建的开销. 为了解决上述问题, 编写独立于拱门的测试用例, 并将其移动到共享位置. | v4 ☑ 4.9-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/755615), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2959a5f726f6510d6dd7c958f8877e08d0cf589c) |
| 2017/02/07 | Laura Abbott <labbott@redhat.com> | [Hardening configs refactor/rename](https://lore.kernel.org/patchwork/cover/758092) | 将原来的 CONFIG_DEBUG_RODATA 重命名为 CONFIG_STRICT_KERNEL_RWX, 原来的 DEBUG_SET_MODULE_RONX 重命名为 CONFIG_STRICT_MODULE_RWX. 这些特性已经不再是 DEBUG 特性, 可以安全地被启用. 同时将这些 CONFIG 移动到 arch/Kconfig 架构无关的定义, 同时引入 ARCH_HAS_KERNEL_RWX 和 ARCH_HAS_MODULE_RWX 来标记对应架构是否支持. | v3 ☑ 4.11-rc1 | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/752526)<br>*-*-*-*-*-*-*-*<br>[PatchWork v3](https://lore.kernel.org/patchwork/cover/758092) |
| 2017/06/28 | Christophe Leroy <christophe.leroy@c-s.fr> | [powerpc/Kconfig: Enable STRICT_KERNEL_RWX](https://patchwork.ozlabs.org/project/linuxppc-dev/patch/20170628170411.28864-8-bsingharora@gmail.com/) | 为 PPC64 实现 STRICT_KERNEL_RWX. | v3 ☑ 4.13-rc1 | [PatchWork v5](https://patchwork.ozlabs.org/project/linuxppc-dev/patch/20170628170411.28864-8-bsingharora@gmail.com/) |
| 2017/06/29 | Michael Ellerman <mpe@ellerman.id.au> | [Provide linux/set_memory.h](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d2852a2240509e512712e25de2d0796cda435ecb) | 引入 CONFIG_ARCH_HAS_SET_MEMORY 封装了 set_memory{ro|rw|x|nx} | v1 ☑ 4.11-rc1 | [commit](https://lore.kernel.org/all/1498717781-29151-1-git-send-email-mpe@ellerman.id.au) |
| 2017/08/02 | Christophe Leroy <christophe.leroy@c-s.fr> | [powerpc/mm: Fix kernel protection and implement STRICT_KERNEL_RWX on PPC32](https://lore.kernel.org/patchwork/cover/816516) | 为 PPC32 实现 STRICT_KERNEL_RWX. | v3 ☑ 4.14-rc1 | [PatchWork v3 OLD](https://lore.kernel.org/patchwork/patch/782821)<br>*-*-*-*-*-*-*-*<br>[PatchWork v3](https://lore.kernel.org/patchwork/cover/816516) |
| 2021/11/22 | Huangzhaoyang <huangzhaoyang@gmail.com> | [arch: arm64: introduce RODATA_FULL_USE_PTE_CONT](https://patchwork.kernel.org/project/linux-mm/patch/1637558929-22971-1-git-send-email-huangzhaoyang@gmail.com) | 为 PPC32 实现 STRICT_KERNEL_RWX. | v3 ☑ 4.14-rc1 | [PatchWork v3 OLD](https://patchwork.kernel.org/project/linux-mm/patch/1637558929-22971-1-git-send-email-huangzhaoyang@gmail.com) |
| 2016/01/26 | [ARM: 8501/1: mm: flip priority of CONFIG_DEBUG_RODATA](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=25362dc496edaf17f714c0fecd8b3eb79670207b) | NA | v1 ☑ 4.6-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=25362dc496edaf17f714c0fecd8b3eb79670207b) |
| 2018/11/07 | Ard Biesheuvel <ardb@kernel.org> | [arm64: mm: apply r/o permissions of VM areas to its linear alias as well](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c55191e96caa9d787e8f682c5e525b7f8172a3b4) | 引入 RODATA_FULL,  | v1 ☑ 5.0-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c55191e96caa9d787e8f682c5e525b7f8172a3b4) |
| 2021/11/22 | Jinbum Park <jinb.park7@gmail.com> | [arm64: enable CONFIG_DEBUG_RODATA by default](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1457014259-32015-1-git-send-email-ard.biesheuvel@linaro.org) | 当启用 RODATA_FULL 时, 内核线性映射将被分割为最小的粒度, 这可能会导致 TLB 压力. 这个补丁使用了一种在 pte 上应用 PTE_CONT 的方法. | v3 ☐ | [PatchWork](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1457014259-32015-1-git-send-email-ard.biesheuvel@linaro.org), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=40982fd6b975de4a51ce5147bc1d698c3b075634) |





# 4 KDUMP
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/08/05 | Arjan van de Ven <arjan@infradead.org> | [Implement crashkernel=auto](https://lore.kernel.org/patchwork/cover/166256) | 实现 crashkernel=auto . | v1 ☐ | [PatchWork](https://lore.kernel.org/patchwork/cover/166256) |


# 5 REFCOUNT
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/11/21 | Will Deacon <will@kernel.org> | [Rework REFCOUNT_FULL using atomic_fetch_* operations](https://lore.kernel.org/patchwork/cover/1156945) | 通过 atomic_fetch_* 操作来降低 cmpxchg() 带来的性能开销, 从而提升 refcount 机制的性能. | v4 ☑ [5.5-rc1]() | [PatchWork](https://lore.kernel.org/patchwork/cover/1156945) |

# 6 Load Average
-------


## 6.1 LTENEX Load Average
-------

[Linux Load Averages: 什么是平均负载 ?](https://zhuanlan.zhihu.com/p/75975041)

Load Averages 是一项历史悠久的指标, 在 1973 年 8 月的 RFC 546 中对 [LTENEX Load Average](https://datatracker.ietf.org/doc/html/rfc546) 有一个很好的描述. 原始的 load averages 仅显示 CPU 需求: 运行的进程数 + 等待运行的进程数.


## 6.2 Delay accounting
-------

[Delay accounting](https://www.kernel.org/doc/html/latest/accounting/delay-accounting.html)


[Utility to get per-pid delay accounting statistics from the kernel using libnl](https://github.com/andrestc/linux-delays)

[per-task-delay-accounting: add memory reclaim delay](https://lwn.net/Articles/285346)

[Per-task delay accounting](https://lwn.net/Articles/160123)

[Patch 0/6 delay accounting & taskstats fixes](http://lkml.iu.edu/hypermail/linux/kernel/0607.1/1005.html)

## 6.3 memory vmpressure
-------



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/08/28 | Johannes Weiner <hannes@cmpxchg.org> | [psi: pressure stall information for CPU, memory, and IO v4](https://lwn.net/Articles/759781) | 引入 PSI 评估系统 CPU, MEMORY, IO 等资源的压力. | v4 ☑ [4.20-rc1](https://kernelnewbies.org/Linux_4.20#Core_.28various.29) | [LWN](https://lwn.net/Articles/544652), [](https://lkml.org/lkml/2013/2/10/140), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=70ddf637eebe47e61fb2be08a59315581b6d2f38) |

## 6 PSI
-------


Pressure Stall Information 提供了一种评估系统资源压力的方法. 系统有三个基础资源: CPU、Memory 和 IO, 无论这些资源配置如何增加, 似乎永远无法满足软件的需求. 一旦产生资源竞争, 就有可能带来延迟增大, 使用户体验到卡顿.

如果没有一种相对准确的方法检测系统的资源压力程度, 有两种后果. 一种是资源使用者过度克制, 没有充分使用系统资源；另一种是经常产生资源竞争, 过度使用资源导致等待延迟过大. 准确的检测方法可以帮忙资源使用者确定合适的工作量, 同时也可以帮助系统制定高效的资源调度策略, 最大化利用系统资源, 最大化改善用户体验.

Facebook 在 2018 年开源了一套解决重要计算集群管理问题的 Linux 内核组件和相关工具, PSI 是其中重要的资源度量工具, 它提供了一种实时检测系统资源竞争程度的方法, 以竞争等待时间的方式呈现, 简单而准确地供用户以及资源调度者进行决策.

[纯干货, PSI 原理解析与应用](https://blog.csdn.net/feelabclihu/article/details/105534140)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/08/28 | Johannes Weiner <hannes@cmpxchg.org> | [psi: pressure stall information for CPU, memory, and IO v4](https://lwn.net/Articles/759781) | 引入 PSI 评估系统 CPU, MEMORY, IO 等资源的压力. | v4 ☑ [4.20-rc1](https://kernelnewbies.org/Linux_4.20#Core_.28various.29) | [Patchwork](https://lore.kernel.org/patchwork/patch/978495), [Patchwork 0/9](https://patchwork.kernel.org/project/linux-mm/cover/20180828172258.3185-1-hannes@cmpxchg.org), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=eb414681d5a07d28d2ff90dc05f69ec6b232ebd2) |
| 2019/03/19 | Suren Baghdasaryan <surenb@google.com> | [psi: pressure stall monitors v6](https://lwn.net/Articles/775971/) | NA | v6 ☑ [5.2-rc1](https://kernelnewbies.org/Linux_5.2#Improved_Presure_Stall_Information_for_better_resource_monitoring) | [Patchwork](https://lore.kernel.org/patchwork/patch/1052413) |
| 2020/03/03 | Suren Baghdasaryan <surenb@google.com> | [psi: Add PSI_CPU_FULL state and some code optimization](ttps://lore.kernel.org/patchwork/patch/1388805) | 1. 添加 PSI_CPU_FULL 状态标记 cgroup 中的所有非空闲任务在 cgroup 之外的 CPU 资源上被延迟, 或者 cgroup 被 throttle<br>2. 使用 ONCPU 状态和当前的 in_memstall 标志来检测回收, 删除 timer tick 中的钩子, 使代码更简洁和可维护.<br>4. 通过移除两个任务的每个公共cgroup祖先的psi_group_change()调用来优化自愿睡眠开关.  | v2 ☑ 5.13-rc1 | [Patchwork](https://lore.kernel.org/patchwork/patch/1388805) |
| 2020/03/31 | Yafang Shao <laoar.shao@gmail.com> | [psi: enhance psi with the help of ebpf](https://lwn.net/Articles/1218304) | 引入 psi_memstall_type 标记 MEMSTALL 的类别, 并在 tracepoint 输出, 从而可以被 ebpf 使用来增强工具. | v4 ☑ [4.20-rc1](https://kernelnewbies.org/Linux_4.20#Core_.28various.29) | [Patchwork](https://lore.kernel.org/patchwork/patch/1218304) |


# 7 DYNAMIC_DEBUG
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/02/12 | Rasmus Villemoes <linux@rasmusvillemoes.dk> | [various dynamic_debug patches](https://lore.kernel.org/patchwork/patch/1041363) | NA | v1 ☑ 5.1-rc1 | [Patchwork v4,00/14](https://lore.kernel.org/patchwork/patch/1041363) |
| 2019/04/09 | Rasmus Villemoes <linux@rasmusvillemoes.dk> | [implement DYNAMIC_DEBUG_RELATIVE_POINTERS](https://lore.kernel.org/patchwork/patch/1059829) | 实现 DYNAMIC_DEBUG_RELATIVE_POINTERS | v1 ☐ | [Patchwork ](https://lore.kernel.org/patchwork/patch/1059829) |


# 8 VDSO
-------


[Remove cached PID/TID in clone](https://sourceware.org/git/?p=glibc.git;a=commitdiff;h=c579f48edba88380635ab98cb612030e3ed8691e)
[Bug 1469670 - glibc: Implement vDSO-based getpid](https://bugzilla.redhat.com/show_bug.cgi?id=1469670)
[Bug 1469757 - kernel: Implement vDSO support for getpid](https://bugzilla.redhat.com/show_bug.cgi?id=1469757)
[Why getpid() is not implemented in x86_64's vdso?](https://stackoverflow.com/questions/65454115/why-getpid-is-not-implemented-in-x86-64s-vdso)


# 9 PRINTK
-------

[Enhanced printk() with Enterprise Event Logging](http://evlog.sourceforge.net/enhanced_printk.html)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/11/19 | Tobin C. Harding <me@tobin.cc> | [hash addresses printed with %p](https://lore.kernel.org/patchwork/cover/856356) | 内核中 %p 打印的地址会暴露内核态地址信息, 是极其不安全的, 因此限制 %p 的打印信息, 它将打印一个散列值, 并不是实际的地址. 如果想要打印实际地址, 需要显式指定 %px. | v11 ☑ 4.15-rc2 | [PatchWork](https://lore.kernel.org/patchwork/cover/856356)<br>*-*-*-*-*-*-*-* <br>[关键 commit 57e734423add](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=57e734423adda83f3b05505875343284efe3b39c) |
| 2021/02/14 | Tobin C. Harding <me@tobin.cc> | [add support for never printing hashed addresses](https://lore.kernel.org/patchwork/cover/1380477) | 新引入 no_hash_pointers 启动参数, 可以禁用 %p 只打印散列值不打印实际地址的限制, 让 %p 可以重新打印内核实际地址. | v4 ☑ 5.12-rc1 | [PatchWork 0/3,v4](https://lore.kernel.org/patchwork/cover/1380477)<br>*-*-*-*-*-*-*-* <br>[关键 commit 57e734423add](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5ead723a20e0447bc7db33dc3070b420e5f80aa6) |
| 2018/11/24 | Tetsuo Handa <penguin-kernel@I-love.SAKURA.ne.jp> | [printk: Add caller information to printk() output.](https://lore.kernel.org/lkml/1543045075-3008-1-git-send-email-penguin-kernel@I-love.SAKURA.ne.jp) | NA | v1 ☑ 5.1-rc1 | [PatchWork 0/3,v4](https://lore.kernel.org/lkml/1543045075-3008-1-git-send-email-penguin-kernel@I-love.SAKURA.ne.jp)<br>*-*-*-*-*-*-*-* <br>[关键 commit 15ff2069cb7f](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=15ff2069cb7f967dae6a8f8c176ba51447c75f00) |
| 2021/10/12 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Improvements to %pGp](https://patchwork.kernel.org/project/linux-mm/patch/20211012182647.1605095-6-willy@infradead.org) | `%pGp` 用来打印 page flag 的信息(通过 format_page_flags() 打印 `__def_pageflag_name`), 但是之前打印的方式比较麻烦, 需要使用 `%#lx(%pGp)`, 先用 hex 16 进制打印一次, 再用 `%pGp` 打印一次, 使用起来略显麻烦. 因此修改 `%pGp` 在打印 flag 的同时, 同时用 hex 打印一次. 这样单用 `%pGp` 就可以完成输出. | v1 ☐ | [PatchWork](https://patchwork.kernel.org/project/linux-mm/patch/20211012182647.1605095-6-willy@infradead.org) |
| 2017/11/08 | Linus Torvalds <torvalds@linux-foundation.org> | [stop using '%pK' for /proc/kallsyms pointer values](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c0f3ea1589394deac2d840c685f57c69e4ac4243) | commit [c0f3ea158939 stop using '%pK' for /proc/kallsyms pointer values](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c0f3ea1589394deac2d840c685f57c69e4ac4243) 4.15 之后, kallsyms 不用 %pK 打印了. 但是仍然用 kptr_restrict 控制权限. | v1 ☑ 5.1-rc1 | [ommit c0f3ea158939](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c0f3ea1589394deac2d840c685f57c69e4ac4243) |

[lockless ringbuffer](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.10-printk)

[[GIT PULL] printk for 5.10 (includes lockless ringbuffer)](http://lkml.iu.edu/hypermail/linux/kernel/2010.1/04077.html)

[printk: Why is it so complicated?, LPC2019](https://lpc.events/event/4/contributions/290/attachments/276/463/lpc2019_jogness_printk.pdf)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/07/09 | John Ogness <john.ogness@linutronix.de> | [printk: replace ringbuffer](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=896fbe20b4e2333fb55cc9b9b783ebcc49eee7c7) | 20200709132344.760-1-john.ogness@linutronix.de | v5 ☑ 5.10-rc1 | [LORE v5,0/4](https://lore.kernel.org/all/20200709132344.760-1-john.ogness@linutronix.de) |
| 2020/07/21 | John Ogness <john.ogness@linutronix.de> | [printk: ringbuffer: support dataless records](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d397820f36ffe4701343b6ee12687d60db0ed8db) | 20200721132528.9661-1-john.ogness@linutronix.de | v2 ☑ 5.10-rc1 | [LORE](https://lore.kernel.org/all/20200721132528.9661-1-john.ogness@linutronix.de) |
| 2020/09/14 | John Ogness <john.ogness@linutronix.de> | [printk: reimplement LOG_CONT handling](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f5f022e53b874f978dda23847173cbf2589b07f5) | 20200914123354.832-6-john.ogness@linutronix.de | v5 ☑ 5.10-rc1 | [LORE v5,0/6](https://lore.kernel.org/all/20200914123354.832-1-john.ogness@linutronix.de) |
| 2020/09/19 | John Ogness <john.ogness@linutronix.de> | [printk: move dictionaries to meta data](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f5f022e53b874f978dda23847173cbf2589b07f5) | 20200918223421.21621-1-john.ogness@linutronix.de | v2 ☑ 5.10-rc1 | [LORE v2,0/3](https://lore.kernel.org/all/20200918223421.21621-1-john.ogness@linutronix.de) |
| 2022/02/07 | John Ogness <john.ogness@linutronix.de> | [implement threaded console printing](https://lore.kernel.org/all/20220207194323.273637-1-john.ogness@linutronix.de) | 参见 phoronix 报道 [Linux Gets Patches For Threaded Console Printing](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Threaded-Console-Print) | v1 ☐ | [LORE v1,0/13](https://lore.kernel.org/all/20220207194323.273637-1-john.ogness@linutronix.de) |


# 10 KEXEC
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/05/07 | Anthony Yznaga <anthony.yznaga@oracle.com> | [PKRAM: Preserved-over-Kexec RAM](https://lore.kernel.org/patchwork/cover/856356) | NA | v11 ☑ 4.15-rc2 | [PatchWork RFC,00/43](https://lore.kernel.org/patchwork/cover/1237362) |
| 2021/09/16 | Pavel Tatashin <pasha.tatashin@soleen.com> | [arm64: MMU enabled kexec relocation](https://patchwork.kernel.org/project/linux-mm/cover/20210802215408.804942-1-pasha.tatashin@soleen.com) | 在 kexec 重定位期间启用 MMU, 以提高重启性能.<br>如果 kexec 功能用于快速的系统更新, 并且停机时间最少, 那么重新定位 kernel + initramfs 将占用重新引导的很大一部分时间.<br>重定位慢的原因是它在没有 MMU 的情况下进行, 因此不能从 D-Cache 中受益. | v16 ☐ | [2021/08/02 PatchWork v16,00/15](https://patchwork.kernel.org/project/linux-mm/cover/20210802215408.804942-1-pasha.tatashin@soleen.com)<br>*-*-*-*-*-*-*-* <br>[2021/09/16 PatchWork v17,00/15](https://patchwork.kernel.org/project/linux-mm/cover/20210916231325.125533-1-pasha.tatashin@soleen.com) |


# 11 perf
-------

## 11.1 perf-user
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/09/18 | Yafang Shao <laoar.shao@gmail.com> | [introduce new perf-script page-reclaim](https://lore.kernel.org/patchwork/cover/1128886) | 为 perf 引入了一个新的 python 脚本 page-reclaim.py 页面回收, 用于报告页面回收详细信息.<br>此脚本目前的用途如下:<br>1. 识别由直接回收引起的延迟峰值<br>2. 延迟峰值与 pageout 是否相关<br>3. 请求页面回收的原因, 即是否是内存碎片<br>4. 页面回收效率等. 将来, 我们还可以将其增强以分析 memcg 回收. | v1 ☐ | [PatchWork 0/2](https://lore.kernel.org/patchwork/cover/1128886) |


## 11.2 ARM SPE
-------



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/10/12 | Will Deacon <will.deacon@arm.com> | [Add support for the ARMv8.2 Statistical Profiling Extension](https://lore.kernel.org/lkml/1507811438-2267-1-git-send-email-will.deacon@arm.com) | perf PMU 驱动支持 SPE | v1 ☑ 4.15-rc1 | [PatchWork v6 0/7](https://lore.kernel.org/lkml/1507811438-2267-1-git-send-email-will.deacon@arm.com) |
| 2018/01/14 | Kim Phillips <kim.phillips@arm.com> | [perf tools: Add ARM Statistical Profiling Extensions (SPE) support](https://lore.kernel.org/lkml/1507811438-2267-1-git-send-email-will.deacon@arm.com) | perf tools 支持 SPE. 这个版本实现的功能还比较简单, 直接把 SPE 的 format 数据导出到了用户态, 由 perf 直接解析, 并没有提供更进一步的 profiling 的功能. | v1 ☑ 4.16-rc1 | [PatchWork v6](https://lore.kernel.org/patchwork/cover/1128886) |
| 2020/05/30 | Leo Yan <leo.yan@linaro.org> | [perf arm-spe: Add support for synthetic events](https://lore.kernel.org/lkml/20200530122442.490-1-leo.yan@linaro.org) | 支持将 SPE 的事件进行分类解析, 可以显示热点以及汇编等信息. | v1 ☑ 5.8-rc1 | [PatchWork v8 0/3](https://lore.kernel.org/lkml/20200530122442.490-1-leo.yan@linaro.org) |
| 2020/11/06 | Leo Yan <leo.yan@linaro.org> | [perf mem/c2c: Support AUX trace](https://lore.kernel.org/lkml/20201106094853.21082-1-leo.yan@linaro.org) | NA. | v1 ☑ 5.8-rc1 | [PatchWork v4 0/9](https://lore.kernel.org/lkml/20201106094853.21082-1-leo.yan@linaro.org) |
| 2021/02/12 | James Clark <james.clark@arm.com> | [perf arm-spe: Enable sample type PERF_SAMPLE_DATA_SRC](https://lore.kernel.org/all/20210211133856.2137-1-james.clark@arm.com) | 在 perf 数据中为 Arm SPE 支持解析 PERF_SAMPLE_DATA_SRC 数据, 当输出跟踪数据时, 它告诉 perf 它在内存事件中包含数据源. | v1 ☑ 5.8-rc1 | [PatchWork v2 1/6](https://lore.kernel.org/all/20210211133856.2137-1-james.clark@arm.com/) |
| 2020/12/13 |  Leo Yan <leo.yan@linaro.org>| [perf c2c: Sort cacheline with all loads](https://lore.kernel.org/all/20201213133850.10070-1-leo.yan@linaro.org) | 实现类似 x86 下 c2c 的功能, 由于 ARM SPE 中没有默认实现类似 X86 hitM 的 data_src, 因此不能有效地判断伪共享. 当前实现方案是将所有的 load 操作排序, 方便开发人员针对伪共享进行分析. | v1 ☐ | [PatchWork v2 00/11](https://lohttps://lore.kernel.org/all/20201213133850.10070-1-leo.yan@linaro.org) |

## 11.3 TOPDOWN
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/05/24 | Andi Kleen <andi@firstfloor.org> | [perf stat: Basic support for TopDown in perf stat](https://lore.kernel.org/all/1464119559-17203-1-git-send-email-andi@firstfloor.org) | perf stat 支持 topdown 分析. | v1 ☑ 4.8-rc1 | [PatchWork 1/4](https://lore.kernel.org/all/1464119559-17203-1-git-send-email-andi@firstfloor.org) |
| 2021/02/02 | Kan Liang <kan.liang@linux.intel.com> | [perf stat: Support L2 Topdown events](https://lore.kernel.org/lkml/1612296553-21962-9-git-send-email-kan.liang@linux.intel.com) | perf stat 支持 Level 2 级别 topdown 分析. [perf core PMU support for Sapphire Rapids (User tools)](https://lore.kernel.org/lkml/1612296553-21962-1-git-send-email-kan.liang@linux.intel.com) 系列中的其中一个补丁. | v1 ☑ 5.12-rc1 | [PatchWork 1/4](https://lore.kernel.org/lkml/1612296553-21962-9-git-send-email-kan.liang@linux.intel.com) |
| 2017/08/31 | Andi Kleen <andi@firstfloor.org> | [perf arm64 metricgroup support](https://lore.kernel.org/all/20170831194036.30146-1-andi@firstfloor.org) | 为 perf stat 添加[对 JSON 文件中指定的独立指标](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b18f3e365019de1a5b26a851e123f0aedcce881f) 的通用支持. 这些指标是一个公式, 它使用多个事件来计算更高级别的结果(例如 IPC, TOPDOWN 分析等). 添加一个新的 `-M /--metrics` 选项来添加指定的度量或度量组. 并增加了对 Intel X86 平台的支持. 通过这些 JSON 文件定义的事件组合度量, 可以很好的支持 TOPDOWN 分析. | v3 ☑ 4.15-rc1 | [PatchWork v3,00/11](https://lore.kernel.org/all/20170831194036.30146-1-andi@firstfloor.org) |
| 2020/09/11 | Kan Liang <kan.liang@linux.intel.com> | [TopDown metrics support for Ice Lake (perf tool)](https://lore.kernel.org/lkml/20200911144808.27603-1-kan.liang@linux.intel.com) | 为 perf metrics 分析增加对 Ice Lake 的支持. 将原本 group 重命名为 topdown. | v3 ☑ 5.10-rc1 | [PatchWork v3,0/4](https://lore.kernel.org/lkml/20200911144808.27603-1-kan.liang@linux.intel.com) |
| 2021/04/07 | John Garry <john.garry@huawei.com> | [perf arm64 metricgroup support](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1617791570-165223-1-git-send-email-john.garry@huawei.com) | perf 支持 HiSilicon hip08 平台的 topdown metric. 支持到 Level 3. 自此鲲鹏 920 的 ARM64 服务器上, 可以使用:<br>`sudo perf stat -M TopDownL1 sleeep 1`<br>来进行 TopDown 分析了. | v1 ☑ 5.13-rc1 | [PatchWork 0/5](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1614784938-27080-1-git-send-email-john.garry@huawei.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/6](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1616668398-144648-1-git-send-email-john.garry@huawei.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3,0/6](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1617791570-165223-1-git-send-email-john.garry@huawei.com) |


# 12 KPROBE
-------

## 12.1 OPTPROBE
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/18 | Qi Liu <liuqi115@huawei.com> | [arm64: Enable OPTPROBE for arm64](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20210818073336.59678-1-liuqi115@huawei.com) | 为 ARM64 引入了 optprobe, 使用分支指令替换探测指令.<br>作者在 Hip08 平台上的进行了测试, optprobe 可以将延迟降低到正常 kprobe 的 `1/4` | v4 ☐ | [Patchwork v4,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20210818073336.59678-1-liuqi115@huawei.com) |
| 2021/11/02 | Janet Liu <jianhua.ljh@gmail.com> | [arm64: implement KPROBES_ON_FTRACE](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1635858706-27320-1-git-send-email-jianhua.ljh@gmail.com) | 为 ARM64 引入了 optprobe. | v2 ☐ | [Patchwork v2,1/2](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1635858706-27320-1-git-send-email-jianhua.ljh@gmail.com) |



## 12.2 KPROBE_ON_FTRACE
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/12/25 | Jisheng Zhang <Jisheng.Zhang@synaptics.com> | [arm64: implement KPROBES_ON_FTRACE](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20191225172625.69811b3e@xhacker.debian) | 为 ARM64 引入了 kprobe on ftrace, 对函数开头注册 kprobe 的时候, 直接用 ftrace 来实现. | v7 ☐ | [Patchwork v7,0/3](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20191225172625.69811b3e@xhacker.debian) |
| 2021/11/02 | Janet Liu <jianhua.ljh@gmail.com> | [arm64: implement KPROBES_ON_FTRACE](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1635858706-27320-2-git-send-email-jianhua.ljh@gmail.com) | 为 ARM64 引入了 kprobe on ftrace, 对函数开头注册 kprobe 的时候, 直接用 ftrace 来实现. | v7 ☐ | [Patchwork v2,1/2](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1635858706-27320-2-git-send-email-jianhua.ljh@gmail.com) |



# 13 编译
-------


## 13.1 CONFIG_OPTIMIZE_INLINING
-------


[CONFIG_OPTIMIZE_INLINING 引发 Go运行时bug调试过程解析](https://blog.csdn.net/qq_40267706/article/details/78817614)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/03/20 | Ingo Molnar <mingo@elte.hu> | [x86: add optimized inlining](https://lore.kernel.org/patchwork/patch/1394812) | 引入 CONFIG_OPTIMIZE_INLINING, 允许 GCC 自己确定哪个 inline 函数真的需要内联, 而不是强制内允许 GCC. 之前 Linux 强制 gcc 总是通过 gcc 属性来内联这些函数, 这可以一定程度提升函数调用的性能, 但是对于一个比较大的函数, 强制将其 inline, 将不可避险增加内核映像的大小, 开启这个选项后, 即使有些函数被标记为 inline, GCC 也会将这些不再 inline. 特别是当用户已经选择了 CONFIG_OPTIMIZE_FOR_SIZE=y 时, 这可以在内核映像大小上产生巨大的差异. 使用标准的 Fedora .config 测试, vmlinux 比之前减少了 2.3% | v1 ☑ 2.6.26-rc1 | [Patchwork](https://patches.linaro.org/patch/160631), [LKML](https://lkml.org/lkml/2019/3/20/63)<br>*-*-*-*-*-*-*-* <br>[关键 commit 57e734423add](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=60a3cdd0639473c79c253bc08c8ef8f882cca107) |
| 2019/03/20 | Masahiro Yamada <yamada.masahiro@socionext.com> | [compiler: allow all arches to enable CONFIG_OPTIMIZE_INLINING](https://lore.kernel.org/patchwork/patch/1394812) | [commit 60a3cdd06394 ("x86: add optimized inline ")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=60a3cdd0639473c79c253bc08c8ef8f882cca107) 引入了 CONFIG_OPTIMIZE_INLINING, 但它只在 x86 上可用.<br>这个想法显然是不可知论的, 因此当前提交将配置项从 `arch/x86/Kconfig.debug` 移到 `lib/Kconfig.debug`, 以便所有架构都能从中受益. 这个提交[早在 2008 年就有过讨论](https://lkml.org/lkml/2008/4/27/7). | v1 ☑ 5.2-rc1 | [Patchwork](https://patches.linaro.org/patch/160631), [LKML](https://lkml.org/lkml/2019/3/20/63)<br>*-*-*-*-*-*-*-* <br>[LKML v3 00/10](https://lkml.org/lkml/2019/4/22/922)<br>*-*-*-*-*-*-*-* <br>[关键 commit 57e734423add](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9012d011660ea5cf2a623e1de207a2bc0ca6936d) |
| 2019/08/30 | Masahiro Yamada <yamada.masahiro@socionext.com> | [compiler: enable CONFIG_OPTIMIZE_INLINING forcibly](https://lore.kernel.org/patchwork/patch/1122097) | NA | v1 ☑ 5.4-rc1 | [Patchwork](https://patches.linaro.org/patch/160631), [LKML](https://lkml.org/lkml/2019/8/29/1772) |


## 13.2 LTO(Link Time Optimization)
-------


编译器一次只能对一个单元(比如一个 `.c` 文件)进行编译和优化(生成 `.o`), 因此没法跨文件范围做优化(如内联等), 只能实现局部优化, 缺乏全局观. 而 LTO, 就是在链接时通过全局视角进行优化操作, 从而对整个系统进行更加极致的优化. 跨模块优化的效果, 也即开启 LTO 主要有这几点好处:

1.  从全局上看将哪些函数內联化, 哪些函数非内敛.

2.  去除了一些无用代码

3.  对程序有全局的优化作用

实践证明, LTO 可以带来诸多好处, 不仅仅是性能的提升甚至飞跃, 也可以带来诸如二进制大小上的缩小等其他附加好处. 但是内核由于其复杂性, 为内核支持 LTO 链接优化在内核社区经历了长期的实践.

1.  早在 2012 年 Intel 的 Andi Kleen 就基于 linux 3.x 为[内核支持了 gcc LTO 编译 Link-time optimization for the kernel](https://lwn.net/Articles/512548), 测试表明 LTO 内核镜像减少了约 20% 的体积, 当然带来的代价就是编译时间增加 3 倍左右(主要是链接时间的增加), 编译过程中所消耗的内存增加(至少 4G 以上). 而由于当时 LTO 并不成熟, 因此禁用了不少模块, 以及不支持多线程编译等问题, 被社区所诟病, 最终 Linus 明确表示不会合并此提交. 但是即使如此 Andi Kleen 也依旧在自己的代码分支: [scm](https://git.kernel.org/pub/scm/linux/kernel/git/ak/linux-misc.git), [github](https://github.com/andikleen/linux-misc/tree/lto-5.12-3). 继续维护和支持着 gcc LTO kernel, 截止目前最新的版本是 lto-5.12. 邮件列表中也能断断续续看到这批补丁的身影. 但是由于为了满足内核稳定性的要求, 体积方面的收益不断减少, 根据最新的测试结果, LTO 内核体积反而增大了 0.2%. 参见 [gcc link time optimization and the Linux kernel](http://www.halobates.de/kernel-lto.pdf).

2.  在 2020 年, Google 公布了其使用 CLANG/LLVM 对内核支持 LTO 编译的工作, 并已经将此项技术应用于 Android.

3.  同样在 2020 年, 来自 Microsoft's GNU/Linux development-tools 团队的 Lan Bearman 也展示了其所在团队在内核 PGO(profile-guided optimization) 和 LTO(link-time optimization) 编译优化方面的努力和成果. 大部分的测试都是在 5.3 内核上用 Redis 数据库完成的. 如果仅使用 `-O3` 选项来构建内核的话, 会看到性能变差了. 不过, 经过 LTO 和 PGO 的优化之后, 会比标准 kernel 在各方面的测试中都有 2-4% 的提升, 除了有一个测试大约性能下降了 0.5%. 这个性能提升令人印象深刻, 尤其是 Redis 实际上并没有在内核中花费很多时间. 参见 [Profile-guided optimization for the kernel](https://lwn.net/Articles/830300).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/11/27 | Andi Kleen <andi@firstfloor.org>/<ak@linux.intel.com> | [x86: Enable GCC Link Time Optimization](https://lwn.net/Articles/512548/) | NA | v1 ☐ | [2012/08/19 Patchwork 67/74](https://patchwork.kernel.org/project/linux-kbuild/patch/1345345030-22211-68-git-send-email-andi@firstfloor.org), [LKML](https://lkml.org/lkml/2012/8/18/273)<br>*-*-*-*-*-*-*-* <br>[2014/02/08 Patchwork 14/17](https://patchwork.kernel.org/project/linux-kbuild/patch/1391846481-31491-14-git-send-email-ak@linux.intel.com), [LKML](https://lkml.org/lkml/2014/2/14/662)<br>*-*-*-*-*-*-*-* <br>[2014/02/14 Patchwork 16/19](https://patchwork.kernel.org/project/linux-kbuild/patch/1392412903-25733-17-git-send-email-andi@firstfloor.org), [LKML](https://lkml.org/lkml/2014/2/14/662)<br>*-*-*-*-*-*-*-* <br>[2014/02/18 Patchwork 16/20](https://patchwork.kernel.org/project/linux-kbuild/patch/1392733738-8290-17-git-send-email-andi@firstfloor.org), [LKML](https://lkml.org/lkml/2014/2/18/1060)<br>*-*-*-*-*-*-*-* <br>[2017/11/27 Patchwork 0/21](https://patchwork.kernel.org/project/linux-kbuild/patch/20171127213423.27218-22-andi@firstfloor.org), [LKML](https://lkml.org/lkml/2017/11/27/1076) |
| 2020/12/11 | Masahiro Yamada <yamada.masahiro@socionext.com> | [Add support for Clang LTO](https://patchwork.kernel.org/project/linux-kbuild/cover/20201211184633.3213045-1-samitolvanen@google.com) | 本补丁系列增加了对使用 Clang 的链接时间优化(LTO) 构建内核的支持. 除了性能之外, LTO 的主要动机是允许在内核中使用 Clang 的控制流完整性(CFI). 自 2018 年以来, 谷歌已经发布了数百万个运行 LTO + CFI 三个主要内核版本的 Pixel 设备.<br>大多数补丁是为了处理 LLVM 位码而进行的构建系统更改, Clang 使用 LTO 而不是 ELF 对象文件生成 LLVM 位码, 将 ELF 处理推迟到后面的阶段, 并确保初始化调用顺序.<br>arm64支持依赖于 [Will 的内存排序补丁](https://git.kernel.org/pub/scm/linux/kernel/git/arm64/linux.git/log/?h=for-next/lto). 早期版本同时做了 X86_64 和 ARM64 的适配和支持. | v9 ☑ 5.12-rc1 | [Patchwork v6,00/25](https://patchwork.kernel.org/project/linux-kbuild/cover/20201211184633.3213045-1-samitolvanen@google.com)<br>*-*-*-*-*-*-*-* <br>v7 之后不再包含 X86 的使能的补丁.<br>*-*-*-*-*-*-*-* <br>[Patchwork v9,00/16](https://patchwork.kernel.org/project/linux-kbuild/cover/20201211184633.3213045-1-samitolvanen@google.com) |
| 2021/4/29 | Wende Tan <twd2.me@gmail.com> | [x86: Enable clang LTO for 32-bit as well](https://patchwork.kernel.org/project/linux-riscv/cover/20210719205208.1023221-1-twd2.me@gmail.com) | CLANG LTO 支持 X86 32 位. | v1 ☑ 5.14-rc1 | [LKML](https://lkml.org/lkml/2021/4/29/873) |
| 2021/07/19 | Wende Tan <twd2.me@gmail.com> | [RISC-V: build: Allow LTO to be selected](https://patchwork.kernel.org/project/linux-riscv/cover/20210719205208.1023221-1-twd2.me@gmail.com) | NA | v1 ☐ | [Patchwork 0/3](https://patchwork.kernel.org/project/linux-riscv/cover/20210719205208.1023221-1-twd2.me@gmail.com) |

4.  BOLT'ing

几年来, Facebook 的工程师们一直在研究 [BOLT](https://www.phoronix.com/scan.php?page=news_item&px=Facebook-BOLT-Optimize-Binaries), 以此加速 Linux/ELF 二进制文件.

BOLT 是一个二进制优化和布局工具, 它是一个 Facebook 孵化器项目, 用于加速Linux x86-64/AArch64 ELF 二进制文件. 这个工具能够分析和重新排布的可执行程序, 以产生比编译器的 LTO 和 PGO 优化所能实现的更快的性能.

facebook 在 LPC-2021 公布了其[最新基于 BOLT 优化 Linux 内核的进展](https://www.phoronix.com/scan.php?page=news_item&px=Facebook-BOLTing-The-Kernel), 这项工作与允许 Linux 内核的配置文件引导优化(PGO)的挑战类似, 与现有的 BOLT 专注于仅优化 ELF 应用程序可执行性相比, BOLT'ing 的 Linux 内核在正确分析/采样内核和相关优化工作负载、内核的大规模代码基数、模块与内核代码等方面面临着类似的复杂障碍. 从公布的信息上看效果不错, 在 PGO + LTO 编译器优化的基础之上仍然带来了两位数的提升(double digit speedups"). 这些提速是通过优化可执行工具的代码布局来实现更高效的硬件页面使用和指令缓存. 参见 [slides](https://linuxplumbersconf.org/event/11/contributions/974/attachments/923/1793/Optimizing%20Linux%20Kernel%20with%20BOLT.pdf).

[BOLT Close To Merging Into LLVM For Optimizing Performance Of Binaries](https://www.phoronix.com/scan.php?page=news_item&px=BOLT-Inches-To-LLVM)

BOLT 代码在 [github 开源](https://github.com/facebookincubator/BOLT).

## 13.2 Shrinking the kernel
-------


缩小内核镜像的体积(Shrinking the kernel) 是内核优化领域一个永恒不变的命题. 之前 LWN 上曾经出了一系列的文章来讨论这项工作. 参见:

1.  [Shrinking the kernel with link-time garbage collection](https://lwn.net/Articles/741494)

2.  [Shrinking the kernel with link-time optimization](https://lwn.net/Articles/744507)

3.  [Shrinking the kernel with an axe](https://lwn.net/Articles/746780)

4.  [Shrinking the kernel with a hammer](https://lwn.net/Articles/748198)


## 13.4 ZERO_CALL_USED_REGS
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/05/05 | Kees Cook <keescook@chromium.org> | [Makefile: Introduce CONFIG_ZERO_CALL_USED_REGS](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.15-Hardening) | 新增 CONFIG_ZERO_CALL_USED_REGS, 启用时使用 "-fzero CALL USED REGS=USED gpr"(在 GCC 11 中)构建内核. 此选项将在函数返回之前将所有调用方使用的寄存器内容归零, 以确保临时值不会泄漏到函数边界之外. 这意味着寄存器内容不太可能用于旁道攻击和信息泄露. 此外, 这有助于将内核映像中有用的 ROP 小工具的数量减少约 20%. phoronix 对这个选项进行了[性能影响测试](https://www.phoronix.com/scan.php?page=article&item=linux515-compile-regress). | v2 ☑ 5.15-rc1 | [Patchwork](https://patchwork.kernel.org/project/linux-kbuild/patch/20210505191804.4015873-1-keescook@chromium.org)<br>*-*-*-*-*-*-*-* <br>[commit a82adfd5c7cb](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a82adfd5c7cb4b8bb37ef439aed954f9972bb618) |


## 13.5 zero initialization for stack variables
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/01/23 | Kees Cook <keescook@chromium.org> | [gcc-plugins: Introduce stackinit plugin](https://patchwork.kernel.org/project/linux-kbuild/cover/20190123110349.35882-1-keescook@chromium.org) | 使用 GCC 编译内核时, 提供插件来实现早期提议的 gcc 选项 [`-finit-local-vars`](https://gcc.gnu.org/ml/gcc-patches/2014-06/msg00615.html)(未被 GNU/gcc 社区接纳)类似的功能, 以实现[未初始化局部变量初始化为 0 的诉求](https://gcc.gnu.org/ml/gcc-patches/2014-06/msg00615.html). 通过 CONFIG_GCC_PLUGIN_STACKINIT 来开启. | v1 ☐ | [PatchWork](https://patchwork.kernel.org/project/linux-kbuild/cover/20190123110349.35882-1-keescook@chromium.org) |
| 2019/04/10 | Kees Cook <keescook@chromium.org> | [security: Implement Clang's stack initialization](https://patchwork.kernel.org/project/linux-kbuild/cover/20190423194925.32151-1-keescook@chromium.org) | clang 中提供了 `-ftrivial-auto-var-init` 来进行未初始化局部变量的默认初始化, 引入 CONFIG_INIT_STACK_ALL 来为内核开启此选项, 这比 CONFIG_GCC_PLUGINS_STRUCTLEAK_BYREF_ALL 的覆盖更广. 当启用 CONFIG_INIT_STACK_ALL 时, 当前补丁仅使用 "pattern" 模式. 开发人员可以通过使用 `__attribute__((uninitialized))` 在每个变量的基础上选择不使用该特性. | v1 ☑ 5.2-rc1 | [PatchWork v1,3/3](https://patchwork.kernel.org/project/linux-kbuild/patch/20190410161612.18545-4-keescook@chromium.org)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,3/3](https://patchwork.kernel.org/project/linux-kbuild/patch/20190411180117.27704-4-keescook@chromium.org)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3,3/3](https://patchwork.kernel.org/project/linux-kbuild/patch/20190423194925.32151-4-keescook@chromium.org), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=709a972efb01efaeb97cad1adc87fe400119c8ab) |
| 2020/06/14 | Alexander Potapenko <glider@google.com> | [security: allow using Clang's zero initialization for stack variables](https://lore.kernel.org/patchwork/cover/1255765) | 支持 clang 的局部变量零初始化. 通过 CONFIG_INIT_STACK_ALL_ZERO 来启用, clang 可以通过选项 `-ftrivial-auto-var-init=zero -enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang` 来保证未初始化局部变量初始化为 0. | RFC v2 ☑ 5.9-rc1 | [PatchWork v2,RFC](https://lore.kernel.org/patchwork/cover/1255765)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC](https://lore.kernel.org/patchwork/patch/1256566), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f0fe00d4972a8cd4b98cc2c29758615e4d51cdfe) |
| 2021/02/18 | NA | [gcc Stack variable initialization-无需合入补丁, GCC 支持即可](https://lwn.net/Articles/870045) | gcc 也引入了 [Auto Initialize Automatic Variables](https://www.phoronix.com/scan.php?page=news_item&px=GCC-12-Auto-Var-Init) 通过 `-ftrivial-auto-var-init` 选项将未初始化的[变量默认初始化为 0](https://gcc.gnu.org/pipermail/gcc-patches/2021-February/565514.html). | RFC v2 ☑ 5.9-rc1 | NA |

## 13.6 Randomizing structure layout
-------

将来自 grsecurity 的[结构布局随机化(Randomizing structure layout)](http://xuxinting.cn/2020/12/20/2020-12-20-kernel-randomize-layout) 推送到 linux 主线. 这个特性通过在编译时将所选结构的布局随机化, 作为对需要知道内核中结构布局的攻击的概率防御. 这对于"内部"内核构建非常有用, 因为攻击者既不能使用随机种子也不能使用其他构建工件.

[commit 313dd1b62921 ("gcc-plugins: Add the randstruct plugin")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=313dd1b629219db50cad532dba6a3b3b22ffe622) 通过引入一个 gcc 插件 [`scripts/gcc-plugins/randomize_layout_plugin.c`](https://elixir.bootlin.com/linux/v4.13/source/scripts/gcc-plugins/randomize_layout_plugin.c) 来完成这项工作. 这个插件通过 [Fisher-Yates shuffle 算法](https://elixir.bootlin.com/linux/v4.13/source/scripts/gcc-plugins/randomize_layout_plugin.c#L256)完成了对指定结构体(类似于数组)的乱序.

>   Fisher–Yates shuffle 是对有限序列生成一个随机排列的算法, 所有的排列是等概率的, 该算法是无偏的、高效的, 算法的时间正比于乱序的数组.
>
>   参见 [Fisher–Yates shuffle 算法详解：给定数组的乱序](https://blog.csdn.net/qikaihuting/article/details/78224690)

这个功能的大部分都是从 grsecurity 移植过来的. 该实现几乎与 PaX 团队和 Brad Spengler 编写的原始代码完全相同. 这些变化是添加了改进的指定初始值设定项标记、白名单机制、许多假阳性修复, 以及处理任务结构随机化的不同方法.


该插件通过三种方式选择结构:

1.  手动使用新的 [` __randomize_layout`](https://elixir.bootlin.com/linux/v4.13/source/include/linux/compiler-gcc.h#L231) 进行标记;

2.  或者在发现结构完全由函数指针组成时由编译器自动进行标记(可以使用 [`__no_randomize_layout`](https://elixir.bootlin.com/linux/v4.13/source/include/linux/compiler-gcc.h#L232) 通知编译器不要进行随机化).

3.  通过 `randomized_struct_fields_{start|end}` 圈定一个范围, 结构体这个范围内所有的字段都会被乱序.

    task_struct 是一种在利用漏洞时特别敏感且经常被滥用的结构, 但由于某些字段需要在开头和结尾, 因此对其进行随机化需要一些特殊处理. 为了解决这个问题, 使用一个[内部匿名结构](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=29e48ce87f1eaaa4b1fe3d9af90c586ac2d1fb74) 来标记将随机化的部分. 随机的这部分(匿名结构)字段用 [randomized_struct_fields_start](https://elixir.bootlin.com/linux/v4.13/source/include/linux/sched.h#L534) 和 [randomized_struct_fields_end](https://elixir.bootlin.com/linux/v4.13/source/include/linux/sched.h#L1094) 来标记.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/10/21 | Michael Leibowitz <michael.leibowitz@intel.com> | [Add the randstruct gcc plugin](https://patchwork.kernel.org/project/kernel-hardening/patch/1477071466-19256-1-git-send-email-michael.leibowitz@intel.com) | 这个插件在编译时随机化某些结构的布局. 这引入了两个定义 `__randomize_layout` 和 `__no_randomize_layout`. 这两个选项用于通知编译器尝试随机化或不随机化有问题的结构. 这个特性是从 grsecurity 移植过来的. 实现与 PaX 团队和 Spender 编写的原始代码几乎相同. 为了简化集成, 当前只支持显式标记结构. 但是, 它保留了对 `__no_randomize_layout` 的支持. UAPI 检查也被保留. 要随机化的结构必须使用C99指定的初始化式形式. | v1 ☐ | [PatchWork](https://lkml.org/lkml/2017/5/26/558)<br>*-*-*-*-*-*-*-* <br>[PatchWork](https://patchwork.kernel.org/project/kernel-hardening/patch/1495829844-69341-20-git-send-email-keescook@chromium.org) |
| 2017/05/26 | Kees Cook <keescook@chromium.org> | [Introduce struct layout randomization plugin](https://lore.kernel.org/patchwork/cover/1471548) | 引入结构体布局随机化插件, 通过 CONFIG_GCC_PLUGIN_RANDSTRUCT 使能. | v2 ☑ 4.13-rc1 | [2017/04/06 PatchWork 16/18](https://patchwork.kernel.org/project/kernel-hardening/patch/1491513513-84351-17-git-send-email-keescook@chromium.org)<br>*-*-*-*-*-*-*-* <br>[2017/05/26 PatchWork v2,00/20](https://lkml.org/lkml/2017/5/26/558)<br>*-*-*-*-*-*-*-* <br>[2017/05/26 PatchWork RFC,v2,19/20](https://patchwork.kernel.org/project/kernel-hardening/patch/1495829844-69341-20-git-send-email-keescook@chromium.org)<br>*-*-*-*-*-*-*-* <br>[2017/06/19 PatchWork 1/4](https://patchwork.kernel.org/project/kernel-hardening/patch/1497905801-69164-2-git-send-email-keescook@chromium.org) |
| 2017/11/09 | Sandipan Das <sandipan@linux.vnet.ibm.com> | [compiler, clang: handle randomizable anonymous structs](https://lists.archive.carbon60.com/linux/kernel/2848189) | 为 clang 支持处理 randomized_struct_fields_start 和 randomized_struct_fields_end 标记 | v1 ☑ 4.15-rc1 | [CGIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4ca59b14e588f873795a11cdc77a25c686a29d2) |


## 13.7 Fast Kernel Headers
-------


Ingo 自 2020 年底就一直投入 "Fast Kernel Headers" 项目, 在 Linux 内核中, 在 `include` 和 `arch/*/include` 层次结构中, 大约有 10000 个主要的 .h 头文件. 在过去的 30 多年里, 他们已经成长为一套复杂而痛苦的交叉依赖, 被称为 "依赖地狱"(Dependency Hell'). 这个项目就是通过对 Linux 内核的头文件层次结构和依赖关系的全面重构, 彻底解决 "依赖地狱", 该项目有两个主要的的目标:

1.  加快内核构建速度 (包括绝对构建时间和增量构建时间)

2.  解耦子系统类型和 API 定义


参见 phoronix 的报道

v1: [Massive ~2.3k Patch Series Would Improve Linux Build Times 50~80% & Fix "Dependency Hell"](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Fast-Kernel-Headers)

v2: [Fast Kernel Headers v2 Posted - Speeds Up Clang-Built Linux Kernel Build By ~88%](https://www.phoronix.com/scan.php?page=news_item&px=Fast-Kernel-Headers-v2)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/01/02 | Ingo Molnar <mingo@kernel.org> | ["Fast Kernel Headers" Tree -v1: Eliminate the Linux kernel's "Dependency Hell"](https://lore.kernel.org/lkml/YdIfz+LMewetSaEB@gmail.com) | "Fast Kernel Headers" 的补丁集, 新增 config 配置, CONFIG_FAST_HEADERS 和 CONFIG_KALLSYMS_FAST.<br>这个补丁集非常庞大, 包含了 2000+ 补丁, 这可能是内核有史以来代码量最大的一个功能. 但是效果也很不错.<br>1. 启用了 CONFIG_FAST_HEADERS 的内核每小时的内核构建次数可能比当前的库存内核多出 78%, 在支持的架构上，绝对内核构建性能可以提高 50~80%.将许多高级标头与其他标头分离, 取消不相关的函数, 类型和 API 标头的分离, 头文件的自动依赖关系处理以及各种其他更改.<br>2. CONFIG_KALLSYMS_FAST 则实现了一个基于 objtool 的未压缩符号表功能, 它避免了 vmlinux 对象文件的通常三重链接, 这是增量内核构建的主要瓶颈. 由于即使使用 distro 配置, kallsyms 表也只有几十 MB 大, 因此在内核开发人员的桌面系统上, 内存成本是可以接受的. 不过当前只在 x86_64 下实现了此功能.<br>到目前为止, 这个庞大的补丁系列已经在 x86/x86_64, SPARC, MIPS 和 ARM64 上进行了测试. | v1 ☐ | [LORE RFC, 0000/2297](https://patchwork.kernel.org/project/kernel-hardening/patch/1495829844-69341-20-git-send-email-keescook@chromium.org)[LORE v2,](https://lore.kernel.org/lkml/Ydm7ReZWQPrbIugn@gmail.com) |

## 13.8 LINK
-------


Mold 是目前 Unix 链接器的现代替代品, 已经达到了 1.0 版本. 由 LLVM lld 连接器的原创者编写, 通过提高并行性, Mold 的目标是比它的前辈快几倍.

2021 年 12 月 [Mold 1.0 发布](https://www.phoronix.com/scan.php?page=news_item&px=Mold-1.0-Released), 作为非常有前途的高性能链接器, 是当前主流编译器等(如 GNU 的 Gold 和 LLVM 的 LLD) 首选替代方案. 随即 GCC 12 宣布增加了[对 Mold 的支持](https://www.phoronix.com/scan.php?page=news_item&px=GCC-12-Mold-Linker). 紧接着 Mold 宣布 1.0.1 将[维护 1 年](https://www.phoronix.com/scan.php?page=news_item&px=Mold-1.0.1-Released), 成为事实上的 LTS 版本.



# 14 FTRACE
-------

## 14.1 dynamic event
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/11/05 | Masami Hiramatsu <mhiramat@kernel.org> | [tracing: Unifying dynamic event interface](https://lore.kernel.org/all/154140838606.17322.15294184388075458777.stgit@devbox) | 目前 ftrace 有 3 个动态事件接口, kprobes, uprobes 和 synthetic. 这组补丁将这些动态事件接口统一为 "dynamic_events", 这样我们就可以在同一个接口上轻松地添加其他动态事件. Dynamic_events 语法与 kprobe_events 和 uprobe_events 没有区别, 可以对 dynamic_events 接口使用相同的语法. | v2 ☑ 5.0-rc1 | [Patchwork](https://lore.kernel.org/all/154140838606.17322.15294184388075458777.stgit@devbox) |


## 14.2 tracepoint
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/12 | "Tzvetomir Stoyanov (VMware)" <tz.stoyanov@gmail.com> | [trace: Add kprobe on tracepoint](https://lore.kernel.org/patchwork/patch/1122097) | 引入了一种新的动态事件:事件探测, 事件可以被附加到一个现有的 tracepoint 跟踪点, 并使用它的字段作为参数. 事件探测通过在 'dynamic_events' ftrace 文件中写入配置字符串来创建. | RFC,v5 ☑ 5.15-rc1 | [Patchwork](https://patchwork.kernel.org/project/linux-trace-devel/patch/20210812145805.2292326-1-tz.stoyanov@gmail.com)<br>*-*-*-*-*-*-*-* <br>[commit 7491e2c44278](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7491e2c442781a1860181adb5ab472a52075f393) |


## 14.3 trace_probe
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/19 | "Tzvetomir Stoyanov (VMware)" <tz.stoyanov@gmail.com> | [tracing/probes: Reject events which have the same name of existing one](https://patchwork.kernel.org/project/linux-trace-devel/patch/162936876189.187130.17558311387542061930.stgit@devnote2) | NA | v1 ☑ 5.15-rc1 | [Patchwork](https://patchwork.kernel.org/project/linux-trace-devel/patch/162936876189.187130.17558311387542061930.stgit@devnote2)<br>*-*-*-*-*-*-*-* <br>[commit 8e242060c6a4](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8e242060c6a4947e8ae7d29794af6a581db08841) |
| 2021/08/20 | Steven Rostedt <rostedt@goodmis.org> | [Tracing: Creation of event probe](https://patchwork.kernel.org/project/linux-trace-devel/patch/162936876189.187130.17558311387542061930.stgit@devnote2) | NA | v9 ☐ | [Patchwork v7,00/10](https://patchwork.kernel.org/project/linux-trace-devel/cover/20210819041321.105110033@goodmis.org)<br>*-*-*-*-*-*-*-* <br>[Patchwork v9,0/6](https://patchwork.kernel.org/project/linux-trace-devel/cover/20210820204644.546662591@goodmis.org) |

## 14.4 boot-time tracing
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/10 | Masami Hiramatsu <mhiramat@kernel.org> | [tracing/boot: Add histogram syntax support in boot-time tracing](https://patchwork.kernel.org/project/linux-trace-devel/patch/162936876189.187130.17558311387542061930.stgit@devnote2) | 为 boot-time tracing  添加 Histogram 选项, 目前, 引导时跟踪仅支持设置触发器动作的每事件动作. 对于像 traceon, traceoff, snapshot 等动作来说, 这足够了. 然而, 对于 hist 触发器操作来说, 这并不好, 因为它通常太长了, 无法将其写入单个字符串, 特别是如果它有 onmatch 操作时. | v1 ☑ 5.15-rc1 | [Patchwork](https://lore.kernel.org/all/162856122550.203126.17607127017097781682.stgit@devnote2) |

## 14.5 OSNOISE & TimerLat
-------

在高性能计算 (HPC) 的上下文中, 操作系统噪声 (OSNOISE) 是指应用程序由于操作系统内部的活动而遭受的干扰. 在 Linux 的上下文中, NMI、IRQ、softirqs 和任何其他系统线程都可能对应用程序造成噪音. 此外, 与硬件相关的作业也会产生噪音, 例如, 通过 SMIs.

关心操作系统窃取的每一微秒的 HPC 用户和开发人员不仅需要一种精确的方法来测量 OSNOISE, 而且主要是找出谁在窃取 CPU 时间, 以便他们可以追求系统的完美调谐. 因此为 linux 内核设计了 [OSNOISE Tracer](https://docs.kernel.org/trace/OSNOISE-tracer.html) 来分析和发现干扰源. OSNOISE Tracer 运行内核内循环, 测量有多少时间可用. 它通过启用抢占, softirq 和 IRQ 来做到这一点, 从而在其执行期间允许所有 OSNOISE 源.

渗透噪声示踪剂会记录任何干扰源的进入点和退出点. 当噪声发生时, 操作系统级别没有任何干扰, Tracer 可以安全地指向与硬件相关的噪声. 通过这种方式, OSNOISE 可以解释任何干扰源. OSNOISE Tracer 还添加了新的内核跟踪点, 这些跟踪点可辅助用户以精确直观的方式指向噪声的罪魁祸首.

在周期结束时, 渗透噪声Tracer 打印所有噪声的总和、最大单个噪声、线程可用的 CPU 百分比以及噪声源的计数器, 用作基准测试工具.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/22 | Daniel Bristot de Oliveira <bristot@redhat.com> | [hwlat improvements and osnoise/timerlat tracers](https://patchwork.kernel.org/project/linux-trace-devel/patch/162936876189.187130.17558311387542061930.stgit@devnote2) | NA | v5 ☑ 5.14-rc1 | [Patchwork v5,09/14](https://lore.kernel.org/all/e649467042d60e7b62714c9c6751a56299d15119.1624372313.git.bristot@redhat.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bce29ac9ce0bb0b0b146b687ab978378c21e9078) |
| 2021/11/29 | Daniel Bristot de Oliveira <bristot@kernel.org> | [osnoise: Support multiple instances (for RTLA)](https://patchwork.kernel.org/project/linux-trace-devel/patch/162936876189.187130.17558311387542061930.stgit@devnote2) | 为 osnoise/timerlat 跟踪器启用多个实例. 目前, osnoise 和 timerlat 仅在单个实例上运行. 为了减少这种限制, 本系列增加了对同一跟踪程序的并行实例的支持. 也就是说, 可以使用不同的跟踪配置运行 osnoise tracer 的两个实例. 例如, 一个仅用于跟踪器输出, 另一个用于跟踪器和一组跟踪点. 这个补丁集是 RTLA 的内核依赖项. 此修补程序集与 [RTLA](https://patchwork.kernel.org/project/linux-trace-devel/cover/cover.1638182284.git.bristot@kernel.org) 一起发送, 但我们将内核和用户空间修补程序集分开. | v9 ☑ 5.16-rc1 | [Patchwork V9,0/9](https://patchwork.kernel.org/project/linux-trace-devel/cover/cover.1635702894.git.bristot@kernel.org), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2fac8d6486d5c34e2ec7028580142b8209da3f92) |
| 2021/12/10 | Daniel Bristot de Oliveira <bristot@kernel.org> | [RTLA: An interface for osnoise/timerlat tracers](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=5dce5904e3b9bc11d9635dd3458c7ff32caebb59) | [rtla](https://github.com/bristot/rtsl) 是一个旨在分析 Linux 实时属性的工具集. rtla 并没有将 Linux 作为一个黑盒来测试, 而是利用内核跟踪功能来提供关于属性和意外结果的根本原因的精确信息. 首先, 它提供了一个 osnoise 和 timerlat 示踪器的接口. 在未来, 它还将作为 rtsl 和其他延迟 / 噪声跟踪器的基础. V6 版本前五个补丁是 [osnoise: Support multiple instances](https://lore.kernel.org/lkml/cover.1628775552.git.bristot@kernel.org) 的重新发送, 它为 osnoise/timerlat 跟踪器启用多个实例. 接下来的七个补丁是 rtla, rtla osnoise 和 rtla timerlat. 后面的版本都将内核态和用户态拆开来提交. 参见 [Linux 5.17 Picks Up A Real-Time Analysis Tool](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-RTLA) | v5 ☐ 5.17-rc1 | [2021/10/27 Patchwork v6,00/20](https://lore.kernel.org/lkml/cover.1635284863.git.bristot@kernel.org)<br>*-*-*-*-*-*-*-* <br>[2021/11/29 Patchwork v8,00/14](https://patchwork.kernel.org/project/linux-trace-devel/cover/cover.1638182284.git.bristot@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v9,00/14](https://lore.kernel.org/all/cover.1639158831.git.bristot@kernel.org) |


https://patchwork.kernel.org/project/linux-trace-devel/list/?submitter=200911&state=*&archive=both&param=1&page=2

[[for-next,01/14] tracing/OSNOISE: Do not follow tracing_cpumask](https://patchwork.kernel.org/project/linux-trace-devel/patch/20211102201156.678148671@goodmis.org/)


## 14.6 MMIO register read/write tracing
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/09 | Sai Prakash Ranjan <quic_saipraka@quicinc.com> | [tracing/rwmmio/arm64: Add support to trace register reads/writes](https://patchwork.kernel.org/project/linux-arm-kernel/cover/cover.1636452784.git.quic_saipraka@quicinc.com) | MMIO register read/write tracing. | v3 ☐ | [Patchwork v3,0/3](https://patchwork.kernel.org/project/linux-arm-kernel/cover/cover.1636452784.git.quic_saipraka@quicinc.com) |

# 15 kptr_restrict
-------


# 16 Generic Kernel Image(GKI)
-------


谷歌的Android因其移动操作系统以及各种供应商/设备内核树所携带的所有下游补丁而臭名昭著, 而近年来, 更多的代码已经上游. 谷歌也一直在转向Android通用内核映像（GKI）作为其所有产品内核的基础, 以进一步减少碎片化. 展望未来, 谷歌在 2021 年北美开源峰会展示了一种["上游优先"(Moving Google toward the mainline)](https://lwn.net/Articles/871195)的方法, 以推动新的内核功能. 追求"新功能的上游优先开发模型", 以确保新代码首先进入主线Linux内核, 而不是直接在Android源代码树中停留.

谷歌的 Todd Kjos 随后在 Linux Plumbers Conference(LPC2021) 上谈到了他们的[通用内核映像(Generic Kernel Image, GKI)计划](https://linuxplumbersconf.org/event/11/contributions/1046). 通过 Android 12 和他们基于 Linux 5.10 的 GKI 映像, 进一步减少了碎片化, 以至于"几乎被消除". 在 Android 12 GKI 中, 大多数供应商/OEM 内核功能现在要么被上游到 Linux 内核中, 要么被隔离到供应商模块或者钩子中, 要么合并到 Android Common Kernel 中.

LWN 上也对此进行了[汇总报道](https://lwn.net/Kernel/Index/#Android-Generic_kernel_image)

[Google Finally Shifting To "Upstream First" Linux Kernel Approach For Android Features](https://www.phoronix.com/scan.php?page=news_item&px=Android-Linux-Upstream-First)

[Android to take an “upstream first” development model for the Linux kernel](https://arstechnica.com/gadgets/2021/09/android-to-take-an-upstream-first-development-model-for-the-linux-kernel)

[](https://stackoverflow.com/questions/65415511/android-kernel-build-flow-with-gki-introduced-from-android-11)

[LPC 2021-Generic Kernel Image](https://linuxplumbersconf.org/event/11/contributions/1046/attachments/824/1557/2021%20LPC%20GKI.pdf)

目前的计划:

1.  2020-2022: Accumulating ecosystem technical debt in Android Common Kernels (android12-5.10 and android-mainline), 汇总技术债务

2.  2023-2024: 减少技术债务(Reducing Technical Debt)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/04/24 | Todd Kjos <tkjos@google.com> | [ANDROID: add support for vendor hooks](https://lore.kernel.org/patchwork/patch/1394812) | 启动阶段异步解压 initramfs. 可以加速系统启动. | v1 ☑ [5.13-rc1](https://kernelnewbies.org/Linux_5.13) | [Patchwork 00/28](https://github.com/aosp-mirror/kernel_common/commit/67e0a3df19970176f093ff8be72f201d8c76ae81) |

# OTHER
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/03/21 | Rasmus Villemoes <linux@rasmusvillemoes.dk> | [background initramfs unpacking, and CONFIG_MODPROBE_PATH](https://lore.kernel.org/patchwork/patch/1394812) | 启动阶段异步解压 initramfs. 可以加速系统启动. | v1 ☑ [5.13-rc1](https://kernelnewbies.org/Linux_5.13) | [Patchwork](https://lore.kernel.org/patchwork/patch/1394812) |




# 17 众核
-------

## 17.1 启动加速
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/21 | David Woodhouse <dwmw2@infradead.org> | [Parallel CPU bringup for x86_64](https://lkml.org/lkml/2021/12/9/664) | 随着核数的增多, 内核的启动速度越来越慢. 这组补丁能够并行启动辅助 (x86_64) CPU 内核. | v1 ☐ | [LWN ](https://lwn.net/Articles/878161), [LKML](https://lkml.org/lkml/2021/12/9/664), [LORE 00/11](https://lkml.kernel.org/lkml/20211209150938.3518-1-dwmw2@infradead.org) |

# 18 LIB
-------

## 18.1 bitmap
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/12/18 | David Woodhouse <dwmw2@infradead.org> | [lib/bitmap: optimize bitmap_weight() usage](https://patchwork.kernel.org/project/linux-mm/cover/20211218212014.1315894-1-yury.norov@gmail.com) | NA | v1 ☐ | [Patchwork v2,00/17](https://lkml.kernel.org/lkml/20211209150938.3518-1-dwmw2@infradead.org) |


## 18.2 CONFIG
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/12/18 | David Woodhouse <dwmw2@infradead.org> | [configs: introduce debug.config for CI-like setup](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0aaa8977acbf3996d351f51b3b15295943092f63) | 参见 [Linux 5.17 Making It Easier To Build A Kernel With All The Shiny Debug Features](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-debug-config) | v5 ☑ 5.17-rc1 | [Patchwork v5](https://lore.kernel.org/all/20211115134754.7334-1-quic_qiancai@quicinc.com) |



<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
