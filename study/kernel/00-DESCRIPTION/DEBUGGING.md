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

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/11/23 | Shile Zhang <shile.zhang@linux.alibaba.com> | [Speed booting by sorting ORC unwind tables at build time](https://lore.kernel.org/patchwork/cover/1162315) | ORC unwind有两个表，.orc_unwind_ip和.orc_unwind 二分搜索需要排序。在构建时对其进行排序可以节省更多CPU周期有助于加快内核引导. 添加ORC表排序在一个独立的线程有助于避免更多的链接. | RFC v6 ☐ | [PatchWork v6](https://lore.kernel.org/patchwork/cover/1162315) |
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


80d6b0c2eed2a504f6740cd1f5ea76dc50abfc4d
commit 57efac2f7108e3255d0dfe512290c9896f4ed55f
Author: Ard Biesheuvel <ard.biesheuvel@linaro.org>
Date:   Thu Mar 3 15:10:59 2016 +0100

    arm64: enable CONFIG_DEBUG_RODATA by defaut

    commit 40982fd6b975de4a51ce5147bc1d698c3b075634
Author: Mark Rutland <mark.rutland@arm.com>
Date:   Thu Aug 25 17:23:23 2016 +0100

    arm64: always enable DEBUG_RODATA and remove the Kconfig option


commit 604c8e676e609da9f17a2abb36f2b2067bb86561
Author: Mark Rutland <mark.rutland@arm.com>
Date:   Fri May 13 12:20:36 2016 +0100

    arm64: enable CONFIG_SET_MODULE_RONX by default


| 2016/02/17 | Kees Cook <keescook@chromium.org> | [introduce post-init read-only memory](https://lore.kernel.org/patchwork/cover/648401) | 许多内容只在 `__init` 期间写入, 并且再也不会更改. 这些不能成为"const", 相反，这组补丁引入了 `__ro_after_init` 来标记这些内存的方法, 并在 x86 和 arm vDSO 上使用它来. 此外<br>*-*-*-*-*-*-*-*<br>1. 还默认使能了 [DEBUG_RODATA](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9ccaf77cf05915f51231d158abfd5448aedde758)<br>2. 添加了一个新的内核参数(rodata)来帮助调试将来的使用, 可以[使用 rodata=n 来禁用此特性](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d2aa1acad22f1bdd0cfa67b3861800e392254454).<br>3. 并添加了一个名为 [lkdtm 测试驱动来验证结果](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7cca071ccbd2a293ea69168ace6abbcdce53098e). | v3 ☑ 4.6-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/648401) |
| 2017/01/29 | Laura Abbott <labbott@redhat.com> | [mm: add arch-independent testcases for RODATA](https://lore.kernel.org/patchwork/cover/755615) | 这个补丁为 RODATA 创建了主独立的测试用例. x86和x86_64都已经有了RODATA的测试用例, 用 CONFIG_DEBUG_RODATA_TEST 宏控制. 但是它们是特定于原型的，因为它们直接使用内联汇编. 如果有人改变了CONFIG_DEBUG_RODATA_TEST的状态，它会导致内核构建的开销. 为了解决上述问题, 编写独立于拱门的测试用例, 并将其移动到共享位置. | v4 ☑ 4.9-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/755615), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2959a5f726f6510d6dd7c958f8877e08d0cf589c) |
| 2017/02/07 | Laura Abbott <labbott@redhat.com> | [Hardening configs refactor/rename](https://lore.kernel.org/patchwork/cover/758092) | 将原来的 CONFIG_DEBUG_RODATA 重命名为 CONFIG_STRICT_KERNEL_RWX, 原来的 DEBUG_SET_MODULE_RONX 重命名为 CONFIG_STRICT_MODULE_RWX. 这些特性已经不再是 DEBUG 特性, 可以安全地被启用. 同时将这些 CONFIG 移动到 arch/Kconfig 架构无关的定义, 同时引入 ARCH_HAS_KERNEL_RWX 和 ARCH_HAS_MODULE_RWX 来标记对应架构是否支持. | v3 ☑ 4.11-rc1 | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/752526)<br>*-*-*-*-*-*-*-*<br>[PatchWork v3](https://lore.kernel.org/patchwork/cover/758092) |
| 2017/06/28 | Christophe Leroy <christophe.leroy@c-s.fr> | [powerpc/Kconfig: Enable STRICT_KERNEL_RWX](https://patchwork.ozlabs.org/project/linuxppc-dev/patch/20170628170411.28864-8-bsingharora@gmail.com/) | 为 PPC64 实现 STRICT_KERNEL_RWX. | v3 ☑ 4.13-rc1 | [PatchWork v5](https://patchwork.ozlabs.org/project/linuxppc-dev/patch/20170628170411.28864-8-bsingharora@gmail.com/) |
| 2017/08/02 | Christophe Leroy <christophe.leroy@c-s.fr> | [powerpc/mm: Fix kernel protection and implement STRICT_KERNEL_RWX on PPC32](https://lore.kernel.org/patchwork/cover/816516) | 为 PPC32 实现 STRICT_KERNEL_RWX. | v3 ☑ 4.14-rc1 | [PatchWork v3 OLD](https://lore.kernel.org/patchwork/patch/782821)<br>*-*-*-*-*-*-*-*<br>[PatchWork v3](https://lore.kernel.org/patchwork/cover/816516) |


commit 25362dc496edaf17f714c0fecd8b3eb79670207b
Author: Kees Cook <keescook@chromium.org>
Date:   Tue Jan 26 01:19:36 2016 +0100

    ARM: 8501/1: mm: flip priority of CONFIG_DEBUG_RODATA


| 2017/01/29 | Jinbum Park <jinb.park7@gmail.com> | [arm64: enable CONFIG_DEBUG_RODATA by default](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1457014259-32015-1-git-send-email-ard.biesheuvel@linaro.org) | arm64 架构下默认开启 CONFIG_DEBUG_RODATA, 并不允许手动关闭. | v3 ☑ 4.9-rc1 | [PatchWork](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1457014259-32015-1-git-send-email-ard.biesheuvel@linaro.org), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=40982fd6b975de4a51ce5147bc1d698c3b075634) |

commit e1a58320a38dfa72be48a0f1a3a92273663ba6db
Author: Stephen Smalley <sds@tycho.nsa.gov>
Date:   Mon Oct 5 12:55:20 2015 -0400

    x86/mm: Warn on W^X mappings


commit 54727e6e950aacd14ec9cd4260e9fe498322828c
Author: Linus Torvalds <torvalds@linux-foundation.org>
Date:   Fri Nov 6 09:12:41 2015 -0800

    x86: don't make DEBUG_WX default to 'y' even with DEBUG_RODATA

ommit d2852a2240509e512712e25de2d0796cda435ecb
Author: Daniel Borkmann <daniel@iogearbox.net>
Date:   Tue Feb 21 16:09:33 2017 +0100

    arch: add ARCH_HAS_SET_MEMORY config



# 4 KDUMP
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/08/05 | Arjan van de Ven <arjan@infradead.org> | [Implement crashkernel=auto](https://lore.kernel.org/patchwork/cover/166256) | 实现 crashkernel=auto . | v1  ☐ | [PatchWork](https://lore.kernel.org/patchwork/cover/166256) |


# 5 REFCOUNT
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/11/21 | Will Deacon <will@kernel.org> | [Rework REFCOUNT_FULL using atomic_fetch_* operations](https://lore.kernel.org/patchwork/cover/1156945) | 通过 atomic_fetch_* 操作来降低 cmpxchg() 带来的性能开销, 从而提升 refcount 机制的性能. | v4 ☑ [5.5-rc1]() | [PatchWork](https://lore.kernel.org/patchwork/cover/1156945) |


# 6 PSI
-------


Pressure Stall Information 提供了一种评估系统资源压力的方法。系统有三个基础资源：CPU、Memory 和 IO，无论这些资源配置如何增加，似乎永远无法满足软件的需求。一旦产生资源竞争，就有可能带来延迟增大，使用户体验到卡顿。

如果没有一种相对准确的方法检测系统的资源压力程度，有两种后果。一种是资源使用者过度克制，没有充分使用系统资源；另一种是经常产生资源竞争，过度使用资源导致等待延迟过大。准确的检测方法可以帮忙资源使用者确定合适的工作量，同时也可以帮助系统制定高效的资源调度策略，最大化利用系统资源，最大化改善用户体验。

Facebook 在 2018 年开源了一套解决重要计算集群管理问题的 Linux 内核组件和相关工具，PSI 是其中重要的资源度量工具，它提供了一种实时检测系统资源竞争程度的方法，以竞争等待时间的方式呈现，简单而准确地供用户以及资源调度者进行决策。

[纯干货，PSI 原理解析与应用](https://blog.csdn.net/feelabclihu/article/details/105534140)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/08/28 | Johannes Weiner <hannes@cmpxchg.org> | [psi: pressure stall information for CPU, memory, and IO v4](https://lwn.net/Articles/759781) | 引入 PSI 评估系统 CPU, MEMORY, IO 等资源的压力. | v4 ☑ [4.20-rc1](https://kernelnewbies.org/Linux_4.20#Core_.28various.29) | [Patchwork](https://lore.kernel.org/patchwork/patch/978495) |
| 2019/03/19 | Suren Baghdasaryan <surenb@google.com> | [psi: pressure stall monitors v6](https://lwn.net/Articles/775971/) | NA | v6 ☑ [5.2-rc1](https://kernelnewbies.org/Linux_5.2#Improved_Presure_Stall_Information_for_better_resource_monitoring) | [Patchwork](https://lore.kernel.org/patchwork/patch/1052413) |
| 2020/03/03 | Suren Baghdasaryan <surenb@google.com> | [psi: Add PSI_CPU_FULL state and some code optimization](ttps://lore.kernel.org/patchwork/patch/1388805) | 1. 添加 PSI_CPU_FULL 状态标记 cgroup 中的所有非空闲任务在 cgroup 之外的 CPU 资源上被延迟, 或者 cgroup 被 throttle<br>2. 使用 ONCPU 状态和当前的 in_memstall 标志来检测回收, 删除 timer tick 中的钩子, 使代码更简洁和可维护.<br>4. 通过移除两个任务的每个公共cgroup祖先的psi_group_change()调用来优化自愿睡眠开关。 | v2 ☑ 5.13-rc1 | [Patchwork](https://lore.kernel.org/patchwork/patch/1388805) |
| 2020/03/31 | Yafang Shao <laoar.shao@gmail.com> | [psi: enhance psi with the help of ebpf](https://lwn.net/Articles/1218304) | 引入 psi_memstall_type 标记 MEMSTALL 的类别, 并在 tracepoint 输出, 从而可以被 ebpf 使用来增强工具. | v4 ☑ [4.20-rc1](https://kernelnewbies.org/Linux_4.20#Core_.28various.29) | [Patchwork](https://lore.kernel.org/patchwork/patch/1218304) |


# 7 DYNAMIC_DEBUG
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/02/12 | Rasmus Villemoes <linux@rasmusvillemoes.dk> | [various dynamic_debug patches](https://lore.kernel.org/patchwork/patch/1041363) | NA | v1 ☑ 5.1-rc1 | [Patchwork v4,00/14](https://lore.kernel.org/patchwork/patch/1041363) |
| 2019/04/09 | Rasmus Villemoes <linux@rasmusvillemoes.dk> | [implement DYNAMIC_DEBUG_RELATIVE_POINTERS](https://lore.kernel.org/patchwork/patch/1059829) | 实现 DYNAMIC_DEBUG_RELATIVE_POINTERS | v1 ☐ | [Patchwork ](https://lore.kernel.org/patchwork/patch/1059829) |



# 8 DRGN
-------


[LWN/A kernel debugger in Python: drgn(https://lwn.net/Articles/789641)
[LWN: 想用python命令来调试kernel吗？drgn就是了！](https://blog.csdn.net/Linux_Everything/article/details/93270705)


# 9 PRINTK
-------

[Enhanced printk() with Enterprise Event Logging](http://evlog.sourceforge.net/enhanced_printk.html)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/11/19 | Tobin C. Harding <me@tobin.cc> | [hash addresses printed with %p](https://lore.kernel.org/patchwork/cover/856356) | 内核中 %p 打印的地址会暴露内核态地址信息, 是极其不安全的, 因此限制 %p 的打印信息, 它将打印一个散列值, 并不是实际的地址. 如果想要打印实际地址, 需要显式指定 %px. | v11 ☑ 4.15-rc2 | [PatchWork](https://lore.kernel.org/patchwork/cover/856356)<br>*-*-*-*-*-*-*-* <br>[关键 commit 57e734423add](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=57e734423adda83f3b05505875343284efe3b39c) |
| 2021/02/14 | Tobin C. Harding <me@tobin.cc> | [add support for never printing hashed addresses](https://lore.kernel.org/patchwork/cover/1380477) | 新引入 no_hash_pointers 启动参数, 可以禁用 %p 只打印散列值不打印实际地址的限制, 让 %p 可以重新打印内核实际地址. | v4 ☑ 5.12-rc1 | [PatchWork 0/3,v4](https://lore.kernel.org/patchwork/cover/1380477)<br>*-*-*-*-*-*-*-* <br>[关键 commit 57e734423add](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5ead723a20e0447bc7db33dc3070b420e5f80aa6) |
| 2018/11/24 | Tetsuo Handa <penguin-kernel@I-love.SAKURA.ne.jp> | [printk: Add caller information to printk() output.](https://lore.kernel.org/lkml/1543045075-3008-1-git-send-email-penguin-kernel@I-love.SAKURA.ne.jp) | NA | v1 ☑ 5.1-rc1 | [PatchWork 0/3,v4](https://lore.kernel.org/lkml/1543045075-3008-1-git-send-email-penguin-kernel@I-love.SAKURA.ne.jp)<br>*-*-*-*-*-*-*-* <br>[关键 commit 15ff2069cb7f](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=15ff2069cb7f967dae6a8f8c176ba51447c75f00) |


# 10 KEXEC
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/05/07 | Anthony Yznaga <anthony.yznaga@oracle.com> | [PKRAM: Preserved-over-Kexec RAM](https://lore.kernel.org/patchwork/cover/856356) | NA | v11 ☑ 4.15-rc2 | [PatchWork RFC,00/43](https://lore.kernel.org/patchwork/cover/1237362) |
| 2021/08/02 | Pavel Tatashin <pasha.tatashin@soleen.com> | [arm64: MMU enabled kexec relocation](https://patchwork.kernel.org/project/linux-mm/cover/20210802215408.804942-1-pasha.tatashin@soleen.com) | 在 kexec 重定位期间启用 MMU, 以提高重启性能.<br>如果 kexec 功能用于快速的系统更新, 并且停机时间最少, 那么重新定位 kernel + initramfs 将占用重新引导的很大一部分时间.<br>重定位慢的原因是它在没有 MMU 的情况下进行, 因此不能从 D-Cache 中受益. | v16 ☐ | [PatchWork v16,00/15](https://patchwork.kernel.org/project/linux-mm/cover/20210802215408.804942-1-pasha.tatashin@soleen.com) |


# 11 perf
-------

## 11.1 perf-user
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/09/18 | Yafang Shao <laoar.shao@gmail.com> | [introduce new perf-script page-reclaim](https://lore.kernel.org/patchwork/cover/1128886) | 为 perf 引入了一个新的 python 脚本 page-reclaim.py 页面回收, 用于报告页面回收详细信息.<br>此脚本目前的用途如下:<br>1. 识别由直接回收引起的延迟峰值<br>2. 延迟峰值与 pageout 是否相关<br>3. 请求页面回收的原因, 即是否是内存碎片<br>4. 页面回收效率等. 将来, 我们还可以将其增强以分析 memcg 回收. | v1 ☐ | [PatchWork 0/2](https://lore.kernel.org/patchwork/cover/1128886) |


# 12 KPROBE
-------

## 12.1 OPTPROBE
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/03/21 | Rasmus Villemoes <linux@rasmusvillemoes.dk> | [arm64: Enable OPTPROBE for arm64](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20210818073336.59678-1-liuqi115@huawei.com) | 为 ARM64 引入了 optprobe, 使用分支指令替换探测指令.<br>作者在 Hip08 平台上的进行了测试, optprobe 可以将延迟降低到正常 kprobe 的 `1/4` | v4 ☐ | [Patchwork v4,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20210818073336.59678-1-liuqi115@huawei.com) |

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

3.  同样在 2020 年, 来自 Microsoft's GNU/Linux development-tools 团队的 Lan Bearman 也展示了其所在团队在内核 PGO(profile-guided optimization) 和 LTO(link-time optimization) 编译优化方面的努力和成果. 大部分的测试都是在 5.3 内核上用 Redis 数据库完成的. 如果仅使用 `-O3` 选项来构建内核的话, 会看到性能变差了. 不过，经过 LTO 和 PGO 的优化之后, 会比标准 kernel 在各方面的测试中都有 2-4% 的提升, 除了有一个测试大约性能下降了 0.5%. 这个性能提升令人印象深刻, 尤其是 Redis 实际上并没有在内核中花费很多时间. 参见 [Profile-guided optimization for the kernel](https://lwn.net/Articles/830300).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/11/27 | Andi Kleen <andi@firstfloor.org>/<ak@linux.intel.com> | [x86: Enable GCC Link Time Optimization](https://lwn.net/Articles/512548/) | NA | v1 ☐ | [2012/08/19 Patchwork 67/74](https://patchwork.kernel.org/project/linux-kbuild/patch/1345345030-22211-68-git-send-email-andi@firstfloor.org), [LKML](https://lkml.org/lkml/2012/8/18/273)<br>*-*-*-*-*-*-*-* <br>[2014/02/08 Patchwork 14/17](https://patchwork.kernel.org/project/linux-kbuild/patch/1391846481-31491-14-git-send-email-ak@linux.intel.com), [LKML](https://lkml.org/lkml/2014/2/14/662)<br>*-*-*-*-*-*-*-* <br>[2014/02/14 Patchwork 16/19](https://patchwork.kernel.org/project/linux-kbuild/patch/1392412903-25733-17-git-send-email-andi@firstfloor.org), [LKML](https://lkml.org/lkml/2014/2/14/662)<br>*-*-*-*-*-*-*-* <br>[2014/02/18 Patchwork 16/20](https://patchwork.kernel.org/project/linux-kbuild/patch/1392733738-8290-17-git-send-email-andi@firstfloor.org), [LKML](https://lkml.org/lkml/2014/2/18/1060)<br>*-*-*-*-*-*-*-* <br>[2017/11/27 Patchwork 0/21](https://patchwork.kernel.org/project/linux-kbuild/patch/20171127213423.27218-22-andi@firstfloor.org), [LKML](https://lkml.org/lkml/2017/11/27/1076) |
| 2020/12/11 | Masahiro Yamada <yamada.masahiro@socionext.com> | [Add support for Clang LTO](https://patchwork.kernel.org/project/linux-kbuild/cover/20201211184633.3213045-1-samitolvanen@google.com) | 本补丁系列增加了对使用 Clang 的链接时间优化(LTO) 构建内核的支持. 除了性能之外, LTO 的主要动机是允许在内核中使用 Clang 的控制流完整性(CFI). 自 2018 年以来, 谷歌已经发布了数百万个运行 LTO + CFI 三个主要内核版本的 Pixel 设备.<br>大多数补丁是为了处理 LLVM 位码而进行的构建系统更改, Clang 使用 LTO 而不是 ELF 对象文件生成 LLVM 位码, 将 ELF 处理推迟到后面的阶段, 并确保初始化调用顺序.<br>arm64支持依赖于 [Will 的内存排序补丁](https://git.kernel.org/pub/scm/linux/kernel/git/arm64/linux.git/log/?h=for-next/lto). 早期版本同时做了 X86_64 和 ARM64 的适配和支持. | v9 ☑ 5.12-rc1 | [Patchwork v6,00/25](https://patchwork.kernel.org/project/linux-kbuild/cover/20201211184633.3213045-1-samitolvanen@google.com)<br>*-*-*-*-*-*-*-* <br>v7 之后不再包含 X86 的使能的补丁.<br>*-*-*-*-*-*-*-* <br>[Patchwork v9,00/16](https://patchwork.kernel.org/project/linux-kbuild/cover/20201211184633.3213045-1-samitolvanen@google.com) |
| 2021/4/29 | Wende Tan <twd2.me@gmail.com> | [x86: Enable clang LTO for 32-bit as well](https://patchwork.kernel.org/project/linux-riscv/cover/20210719205208.1023221-1-twd2.me@gmail.com) | CLANG LTO 支持 X86 32 位. | v1 ☑ 5.14-rc1 | [LKML](https://lkml.org/lkml/2021/4/29/873) |
| 2021/07/19 | Wende Tan <twd2.me@gmail.com> | [RISC-V: build: Allow LTO to be selected](https://patchwork.kernel.org/project/linux-riscv/cover/20210719205208.1023221-1-twd2.me@gmail.com) | NA | v1 ☐ | [Patchwork 0/3](https://patchwork.kernel.org/project/linux-riscv/cover/20210719205208.1023221-1-twd2.me@gmail.com) |


## 13.2 Shrinking the kernel
-------


缩小内核镜像的体积(Shrinking the kernel) 是内核优化领域一个永恒不变的命题. 之前 LWN 上曾经出了一系列的文章来讨论这项工作. 参见:

1.  [Shrinking the kernel with link-time garbage collection](https://lwn.net/Articles/741494)

2.  [Shrinking the kernel with link-time optimization](https://lwn.net/Articles/744507)

3.  [Shrinking the kernel with an axe](https://lwn.net/Articles/746780)

4.  [Shrinking the kernel with a hammer](https://lwn.net/Articles/748198)

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
| 2021/08/10 | Masami Hiramatsu <mhiramat@kernel.org> | [tracing/boot: Add histogram syntax support in boot-time tracing](https://patchwork.kernel.org/project/linux-trace-devel/patch/162936876189.187130.17558311387542061930.stgit@devnote2) | 为 boot-time tracing  添加 Histogram 选项, 目前, 引导时跟踪仅支持设置触发器动作的每事件动作. 对于像 traceon, traceoff, snapshot 等短动作来说, 这就足够了. 然而, 对于 hist 触发器操作来说, 这并不好, 因为它通常太长了, 无法将其写入单个字符串, 特别是如果它有 onmatch 操作时. | v1 ☑ 5.15-rc1 | [Patchwork](https://lore.kernel.org/all/162856122550.203126.17607127017097781682.stgit@devnote2) |



# 7 OTHER
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/03/21 | Rasmus Villemoes <linux@rasmusvillemoes.dk> | [background initramfs unpacking, and CONFIG_MODPROBE_PATH](https://lore.kernel.org/patchwork/patch/1394812) | 启动阶段异步解压 initramfs. 可以加速系统启动. | v1 ☑ [5.13-rc1](https://kernelnewbies.org/Linux_5.13) | [Patchwork](https://lore.kernel.org/patchwork/patch/1394812) |




<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
