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

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/11/19 | Tobin C. Harding <me@tobin.cc> | [hash addresses printed with %p](https://lore.kernel.org/patchwork/cover/856356) | 内核中 %p 打印的地址会暴露内核态地址信息, 是极其不安全的, 因此限制 %p 的打印信息, 它将打印一个散列值, 并不是实际的地址. 如果想要打印实际地址, 需要显式指定 %px. | v11 ☑ 4.15-rc2 | [PatchWork](https://lore.kernel.org/patchwork/cover/856356)<br>*-*-*-*-*-*-*-* <br>[关键 commit 57e734423add](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=57e734423adda83f3b05505875343284efe3b39c) |
| 2021/02/14 | Tobin C. Harding <me@tobin.cc> | [add support for never printing hashed addresses](https://lore.kernel.org/patchwork/cover/1380477) | 新引入 no_hash_pointers 启动参数, 可以禁用 %p 只打印散列值不打印实际地址的限制, 让 %p 可以重新打印内核实际地址. | v4 ☑ 5.12-rc1 | [PatchWork 0/3,v4](https://lore.kernel.org/patchwork/cover/1380477)<br>*-*-*-*-*-*-*-* <br>[关键 commit 57e734423add](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5ead723a20e0447bc7db33dc3070b420e5f80aa6) |


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
