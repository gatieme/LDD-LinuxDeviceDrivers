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
blogexcerpt: FZF 是目前最快的模糊搜索工具. 使用 golang 编写. 结合其他工具 (比如 ag 和 fasd) 可以完成非常多的工作. 前段时间, 有同事给鄙人推荐了 FZF, 通过简单的配置, 配合 VIM/GIT 等工具食用, 简直事半功倍, 效率指数级提升, 因此推荐给大家.

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

2   ** 调测子系统 **
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***



# 1 UNWIND
-------

[探索 Android 平台 ARM unwind 技术](https://zhuanlan.zhihu.com/p/336916116)

[The Linux x86 ORC Stack Unwinder](https://www.codeblueprint.co.uk/2017/07/31/the-orc-unwinder.html)

[Linux 内核的栈回溯与妙用](https://blog.csdn.net/qq_40732350/article/details/83649888)

[Stack unwinding](https://zhuanlan.zhihu.com/p/280929169)

[CPU Architectures » x86-specific Documentation » 9. ORC unwinderView page source](https://www.kernel.org/doc/html/latest/x86/orc-unwinder.html)

[内核栈回溯原理应用](https://www.cnblogs.com/mysky007/p/12539754.<!DOCTYPE html>

[Android中基于DWARF的stack unwind实现原理](https://blog.csdn.net/feelabclihu/article/details/137703180)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/11/23 | Shile Zhang <shile.zhang@linux.alibaba.com> | [Speed booting by sorting ORC unwind tables at build time](https://lore.kernel.org/patchwork/cover/1162315) | ORC unwind 有两个表, .orc_unwind_ip 和. orc_unwind 二分搜索需要排序. 在构建时对其进行排序可以节省更多 CPU 周期有助于加快内核引导. 添加 ORC 表排序在一个独立的线程有助于避免更多的链接. | RFC v6 ☐ | [PatchWork v6](https://lore.kernel.org/patchwork/cover/1162315) |
| 2021/08/12 | Shile Zhang <shile.zhang@linux.alibaba.com> | [arm64: Reorganize the unwinder and implement stack trace reliability checks](https://lwn.net/Articles/866194) | 使所有堆栈遍历函数都使用 arch_stack_walk().<br> 目前, ARM64 代码中有多个函数使用 start_backtrace() 和 unwind_frame() 遍历堆栈. 将它们全部转换为使用 arch_stack_walk(). 这使得维护更容易. | RFC v8 ☐ | [PatchWork RFC,v8,0/4](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20210812190603.25326-1-madvenka@linux.microsoft.com) |

虽然不省略帧指针对性能有已知的影响, 因此之前各发行版都默认不开启栈帧, 但是 2022 年各大发行版,  Fedora/Ubuntu 24.04 LTS 等均启用了帧指针以实现更好的调试/分析. 为了两全其美, 2024 年 Red Hat 开发 eu-stracktrace 作为一种新的分析方法, 而无需依赖帧指针. 参见 phoronix 报道 [phoronix, 2024/06/11, Red Hat Developing "eu-stacktrace" For Profiling Without Frame Pointers](https://www.phoronix.com/news/Red-Hat-eu-stacktrace), [redhat 博客--Get system-wide profiles of binaries without frame pointers](https://developers.redhat.com/articles/2024/06/11/get-system-wide-profiles-binaries-without-frame-pointers#), 以及 [elfutils, eu-stacktrace development branch](https://sourceware.org/cgit/elfutils/tree/README.eu-stacktrace?h=users/serhei/eu-stacktrace).

# 2 unikernel
-------

Unikernel 是一种专门的操作系统, 其中应用程序直接与内核链接, 并以管理模式运行. 这允许开发人员对内核实现特定于应用程序的优化, 应用程序可以直接调用这些优化(无需经过系统调用路径). 应用程序可以控制调度和资源管理, 并直接访问硬件. 应用程序和内核可以联合优化, 例如通过 LTO、PGO 等. 所有这些优化和其他优化都为应用程序提供了比通用操作系统更大的性能优势.

[An overview of approaches-PROJECTS/Open source work on unikernels](http://unikernel.org/projects)


*   KML(Kernel Mode Linux : Execute user processes in kernel mode)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/04/16 | Toshiyuki Maeda | [Kernel Mode Linux : Execute user processes in kernel mode](http://web.yl.is.s.u-tokyo.ac.jp/~tosh/kml) | 内核直接执行用户态程序. | v1 ☐ [4.0](http://web.yl.is.s.u-tokyo.ac.jp/~tosh/kml/kml/for4.x) | [Patch](https://github.com/sonicyang/KML) |
| 2018/11/23 | Hsuan-Chi Kuo <hckuo2@illinois.edu> | [Lupine: Linux in Unikernel Clothes](https://github.com/hckuo/Lupine-Linux) | 通过内核配置和 (KML) 间接清除系统调用的影响, 从而使得 Linux 的性能达到持平甚至优于 unikernel 的性能. | ☐ | [Github](https://github.com/hckuo/Lupine-Linux), [159_kuo_slides.pdf](https://www.eurosys2020.org/wp-content/uploads/2020/04/slides/159_kuo_slides.pdf) |
| 2022/10/03 | Ali Raza <aliraza@bu.edu> | [Unikernel Linux (UKL)](https://lore.kernel.org/all/20221003222133.20948-1-aliraza@bu.edu) | 参见 [Experimental Patches Adapt Linux For A Unikernel Design](https://www.phoronix.com/news/Linux-Unikernel-RFC). | v1 ☐☑✓ | [LORE v1,0/10](https://lore.kernel.org/all/20221003222133.20948-1-aliraza@bu.edu), [Github](https://github.com/unikernelLinux/ukl) |
| 2024/02/07 | Huaiyu Yan | [ndss2024_LDR: Secure and Efficient Linux Driver Runtime for Embedded TEE Systems](https://zhuanlan.zhihu.com/p/681195447) |  复用现有 LINUX 驱动, 在嵌入式TEE系统下更高效的 LKM runtime. | ☐ | [ndss-paper](https://www.ndss-symposium.org/ndss-paper/ldr-secure-and-efficient-linux-driver-runtime-for-embedded-tee-systems), [github, SparkYHY/Linux-Driver-Runtime](https://github.com/SparkYHY/Linux-Driver-Runtime) |

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
| 2014/04/14 | Kees Cook <keescook@chromium.org> | [ARM: mm: allow text and rodata sections to be read-only](https://lore.kernel.org/patchwork/cover/456177) | 为 ARM 实现 CONFIG_DEBUG_RODATA 特性, 设置内核代码段只读和数据段不可执行的. | v2 ☑ 3.19-rc1 | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/456177), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=80d6b0c2eed2a504f6740cd1f5ea76dc50abfc4) |
| 2014/08/19 | Laura Abbott <lauraa@codeaurora.org> | arm64: Add CONFIG_DEBUG_SET_MODULE_RONX support | 为 ARM64 实现 CONFIG_DEBUG_SET_MODULE_RONX, 限制模块中各个不同类型页面的读写和可执行权限. | v2 ☑ 3.18-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=11d91a770f1fff44dafdf88d6089a3451f99c9b6) |
| 2015/01/21 | Laura Abbott <lauraa@codeaurora.org> | arm64: add better page protections to arm64 | 为 ARM64 实现 CONFIG_DEBUG_RODATA 特性, 设置内核代码段只读和数据段不可执行的. | v2 ☑ 4.0-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=da141706aea52c1a9fbd28cb8d289b78819f5436) |
| 2015/11/06 | Linus Torvalds <torvalds@linux-foundation.org> | [x86: don't make DEBUG_WX default to 'y' even with DEBUG_RODATA](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=54727e6e950aacd14ec9cd4260e9fe498322828c) | NA | v1 ☑ 4.4-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=54727e6e950aacd14ec9cd4260e9fe498322828c) |
| 2016/01/26 | Stephen Smalley <sds@tycho.nsa.gov> | [x86/mm: Warn on W^X mappings](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e1a58320a38dfa72be48a0f1a3a92273663ba6db) | NA | v1 ☑ 4.9-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e1a58320a38dfa72be48a0f1a3a92273663ba6db) |
| 2016/02/17 | Kees Cook <keescook@chromium.org> | [introduce post-init read-only memory](https://lore.kernel.org/patchwork/cover/648401) | 许多内容只在 `__init` 期间写入, 并且再也不会更改. 这些不能成为 "const", 相反, 这组补丁引入了 `__ro_after_init` 来标记这些内存的方法, 并在 x86 和 arm vDSO 上使用它来. 此外 <br>*-*-*-*-*-*-*-*<br>1. 还默认使能了 [DEBUG_RODATA](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9ccaf77cf05915f51231d158abfd5448aedde758)<br>2. 添加了一个新的内核参数 (rodata) 来帮助调试将来的使用, 可以[使用 rodata=n 来禁用此特性](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d2aa1acad22f1bdd0cfa67b3861800e392254454).<br>3. 并添加了一个名为 [lkdtm 测试驱动来验证结果](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7cca071ccbd2a293ea69168ace6abbcdce53098e). | v3 ☑ 4.6-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/648401) |
| 2017/01/29 | Laura Abbott <labbott@redhat.com> | [mm: add arch-independent testcases for RODATA](https://lore.kernel.org/patchwork/cover/755615) | 这个补丁为 RODATA 创建了主独立的测试用例. x86 和 x86_64 都已经有了 RODATA 的测试用例, 用 CONFIG_DEBUG_RODATA_TEST 宏控制. 但是它们是特定于原型的, 因为它们直接使用内联汇编. 如果有人改变了 CONFIG_DEBUG_RODATA_TEST 的状态, 它会导致内核构建的开销. 为了解决上述问题, 编写独立于拱门的测试用例, 并将其移动到共享位置. | v4 ☑ 4.9-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/755615), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2959a5f726f6510d6dd7c958f8877e08d0cf589c) |
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
| 2022/08/28 | Baoquan He <bhe@redhat.com> | [arm64, kdump: enforce to take 4G as the crashkernel low memory end](https://patchwork.kernel.org/project/linux-mm/cover/20220828005545.94389-1-bhe@redhat.com/) | 671768 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20220828005545.94389-1-bhe@redhat.com) |
| 2024/03/05 | Steven Rostedt <rostedt@goodmis.org> | [tracing: Persistent traces across a reboot or crash](https://lore.kernel.org/all/20240306015910.766510873@goodmis.org) | [Experimental Linux Patches Allow Kernel Tracing To Work Past Reboots/Crashes](https://www.phoronix.com/news/Linux-Tracing-Post-Reboots). | v1 ☐☑✓ | [LORE v1,0/8](https://lore.kernel.org/all/20240306015910.766510873@goodmis.org) |


[crash extension modules](https://crash-utility.github.io/extensions.html)


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

[docs/scheduler: Introduce the doc of load average](https://lore.kernel.org/lkml/20220314084759.9849-1-juitse.huang@gmail.com)

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
| 2022/02/19 | Suren Baghdasaryan <surenb@google.com> | [[1/1] mm: count time in drain_all_pages during direct reclaimas memory pressure](https://patchwork.kernel.org/project/linux-mm/patch/20220219174940.2570901-1-surenb@google.com/) |615990 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220219174940.2570901-1-surenb@google.com) |
| 2022/09/10 | Christoph Hellwig <hch@lst.de> | [improve pagecache PSI annotations](https://lore.kernel.org/all/20220910065058.3303831-1-hch@lst.de) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20220910065058.3303831-1-hch@lst.de)<br>*-*-*-*-*-*-*-* <br>[LORE v2](https://lore.kernel.org/all/20220915094200.139713-1-hch@lst.de) |

## 6.4 PSI Core
-------

[知乎 - 兰新宇 --Linux 的资源控制监测 - PSI [上]](https://zhuanlan.zhihu.com/p/523716850)

[知乎 - 兰新宇 --Linux 的资源控制监测 - PSI [下]](https://zhuanlan.zhihu.com/p/523554299)

Pressure Stall Information 提供了一种评估系统资源压力的方法. 系统有三个基础资源: CPU、Memory 和 IO, 无论这些资源配置如何增加, 似乎永远无法满足软件的需求. 一旦产生资源竞争, 就有可能带来延迟增大, 使用户体验到卡顿.

如果没有一种相对准确的方法检测系统的资源压力程度, 有两种后果. 一种是资源使用者过度克制, 没有充分使用系统资源；另一种是经常产生资源竞争, 过度使用资源导致等待延迟过大. 准确的检测方法可以帮忙资源使用者确定合适的工作量, 同时也可以帮助系统制定高效的资源调度策略, 最大化利用系统资源, 最大化改善用户体验.

Facebook 在 2018 年开源了一套解决重要计算集群管理问题的 Linux 内核组件和相关工具, PSI 是其中重要的资源度量工具, 它提供了一种实时检测系统资源竞争程度的方法, 以竞争等待时间的方式呈现, 简单而准确地供用户以及资源调度者进行决策.

内核文档参见 [Documentation/accounting/psi.rst](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/accounting/psi.rst), 翻译参见 [`Documentation/translations/zh_CN/accounting/psi.rst`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/translations/zh_CN/accounting/psi.rst).

[CSDN - 内核工匠 -- 纯干货, PSI 原理解析与应用](https://blog.csdn.net/feelabclihu/article/details/105534140)

Meta(原 Facebook) 开发的 [Senpai](https://github.com/facebookincubator/senpai), 就是通过 PSI 来确定容器化应用程序的实际内存需求. 根据 PSI 提供的 Memory Pressure 信息, 不断地调整 memory.high, 从而找到指定 memcg 所承载的 workload 真正需要的内存大小. Senpai 是一个用户空间代理, 它通过施加轻微的主动式内存压力, 跨不同的工作负载和异构硬件有效地卸载内存, 对应用程序性能的影响最小.

根据与压力目标的偏差, Senpai 每隔 interval 时间 (默认 6s) 重新确定要回收的页面数:

$reclaim = current\_mem \times reclaim\_ratio \times max(0,1 – \frac{psi_some}{psi_threshold})$

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/08/28 | Johannes Weiner <hannes@cmpxchg.org> | [psi: pressure stall information for CPU, memory, and IO v4](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=2ce7135adc9ad081aa3c49744144376ac74fea60) | 引入 PSI 评估系统 CPU, MEMORY, IO 等资源的压力. 参见 [LWN-Tracking pressure-stall information](https://lwn.net/Articles/759781) | v4 ☑ [4.20-rc1](https://kernelnewbies.org/Linux_4.20#Core_.28various.29) | [Patchwork](https://lore.kernel.org/patchwork/patch/978495), [Patchwork 0/9](https://patchwork.kernel.org/project/linux-mm/cover/20180828172258.3185-1-hannes@cmpxchg.org), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=eb414681d5a07d28d2ff90dc05f69ec6b232ebd2) |
| 2019/03/19 | Suren Baghdasaryan <surenb@google.com> | [psi: pressure stall monitors v6](https://lwn.net/Articles/775971/) | NA | v6 ☑ [5.2-rc1](https://kernelnewbies.org/Linux_5.2#Improved_Presure_Stall_Information_for_better_resource_monitoring) | [Patchwork](https://lore.kernel.org/patchwork/patch/1052413) |
| 2020/03/03 | Suren Baghdasaryan <surenb@google.com> | [psi: Add PSI_CPU_FULL state and some code optimization](ttps://lore.kernel.org/patchwork/patch/1388805) | 1. 添加 PSI_CPU_FULL 状态标记 cgroup 中的所有非空闲任务在 cgroup 之外的 CPU 资源上被延迟, 或者 cgroup 被 throttle<br>2. 使用 ONCPU 状态和当前的 in_memstall 标志来检测回收, 删除 timer tick 中的钩子, 使代码更简洁和可维护.<br>4. 通过移除两个任务的每个公共 cgroup 祖先的 psi_group_change()调用来优化自愿睡眠开关.  | v2 ☑ 5.13-rc1 | [Patchwork](https://lore.kernel.org/patchwork/patch/1388805) |
| 2020/03/31 | Yafang Shao <laoar.shao@gmail.com> | [psi: enhance psi with the help of ebpf](https://lwn.net/Articles/1218304) | 引入 psi_memstall_type 标记 MEMSTALL 的类别, 并在 tracepoint 输出, 从而可以被 ebpf 使用来增强工具. | v4 ☑ [4.20-rc1](https://kernelnewbies.org/Linux_4.20#Core_.28various.29) | [Patchwork](https://lore.kernel.org/patchwork/patch/1218304) |
| 2022/07/21 | Chengming Zhou <zhouchengming@bytedance.com> | [sched/psi: some optimization and extension](https://lore.kernel.org/all/20220721040439.2651-1-zhouchengming@bytedance.com) | 优化 PSI 的性能, 同时增加对 IRQ/SOFTIRQ 的负载压力跟踪. | v1 ☐☑✓ | [LORE v1,0/9](https://lore.kernel.org/all/20220721040439.2651-1-zhouchengming@bytedance.com) |
| 2022/08/01 | CGEL <cgel.zte@gmail.com> | [[RFC,1/2] psi: introduce memory.pressure.stat](https://patchwork.kernel.org/project/linux-mm/patch/20220801004205.1593100-1-ran.xiaokai@zte.com.cn/) | 664363 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20220801004205.1593100-1-ran.xiaokai@zte.com.cn) |
| 2022/09/10 | Christoph Hellwig <hch@lst.de> | [[1/5] mm: add PSI accounting around ->read_folio and ->readahead calls](https://patchwork.kernel.org/project/linux-mm/patch/20220910065058.3303831-2-hch@lst.de/) | 675899 | v1 ☐☑ | [LORE v1,0/5](https://lore.kernel.org/r/20220910065058.3303831-2-hch@lst.de) |
| 2022/09/15 | Christoph Hellwig <hch@lst.de> | [improve pagecache PSI annotations v2](https://lore.kernel.org/all/20220915094200.139713-1-hch@lst.de) | TODO | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20220915094200.139713-1-hch@lst.de) |
| 2023/02/10 | Sudarshan Rajagopalan <quic_sudaraja@quicinc.com> | [psi: reduce min window size to 50ms](https://patchwork.kernel.org/project/linux-mm/patch/8b7a3270fe253de1cd2b71473e29394409b2a0f7.1676067791.git.quic_sudaraja@quicinc.com/) | 720854 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/8b7a3270fe253de1cd2b71473e29394409b2a0f7.1676067791.git.quic_sudaraja@quicinc.com) |


# 7 DYNAMIC_DEBUG
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/02/12 | Rasmus Villemoes <linux@rasmusvillemoes.dk> | [various dynamic_debug patches](https://lore.kernel.org/patchwork/patch/1041363) | NA | v1 ☑ 5.1-rc1 | [Patchwork v4,00/14](https://lore.kernel.org/patchwork/patch/1041363) |
| 2019/04/09 | Rasmus Villemoes <linux@rasmusvillemoes.dk> | [implement DYNAMIC_DEBUG_RELATIVE_POINTERS](https://lore.kernel.org/patchwork/patch/1059829) | 实现 DYNAMIC_DEBUG_RELATIVE_POINTERS | v1 ☐ | [Patchwork](https://lore.kernel.org/patchwork/patch/1059829) |


# 8 VDSO
-------

## 8.1 getpid() 性能劣化
-------

[Remove cached PID/TID in clone](https://sourceware.org/git/?p=glibc.git;a=commitdiff;h=c579f48edba88380635ab98cb612030e3ed8691e)

[Bug 1469670 - glibc: Implement vDSO-based getpid](https://bugzilla.redhat.com/show_bug.cgi?id=1469670)

[Bug 1469757 - kernel: Implement vDSO support for getpid](https://bugzilla.redhat.com/show_bug.cgi?id=1469757)

[Why getpid() is not implemented in x86_64's vdso?](https://stackoverflow.com/questions/65454115/why-getpid-is-not-implemented-in-x86-64s-vdso)


## 8.2 getrandom vDSO
-------

[Implementing virtual system calls](https://lwn.net/Articles/615809)

[Linux Proposal Adding getrandom() To The vDSO For Better Performance](https://www.phoronix.com/news/Linux-getrandom-vDSO)

[A vDSO implementation of getrandom()](https://lwn.net/Articles/919008)

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/07/29 | Jason A. Donenfeld <Jason@zx2c4.com> | [random: implement getrandom() in vDSO](https://lore.kernel.org/all/20220729145525.1729066-1-Jason@zx2c4.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20220729145525.1729066-1-Jason@zx2c4.com)<br>*-*-*-*-*-*-*-* <br>[2023/01/01 LORE v14,0/7](https://lore.kernel.org/all/20230101162910.710293-1-Jason@zx2c4.com) |


# 9 PRINTK
-------


## 9.1 Printk Improve
-------

[Enhanced printk() with Enterprise Event Logging](http://evlog.sourceforge.net/enhanced_printk.html)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/11/19 | Tobin C. Harding <me@tobin.cc> | [hash addresses printed with %p](https://lore.kernel.org/patchwork/cover/856356) | 内核中 %p 打印的地址会暴露内核态地址信息, 是极其不安全的, 因此限制 %p 的打印信息, 它将打印一个散列值, 并不是实际的地址. 如果想要打印实际地址, 需要显式指定 %px. | v11 ☑ 4.15-rc2 | [PatchWork](https://lore.kernel.org/patchwork/cover/856356)<br>*-*-*-*-*-*-*-* <br>[关键 commit 57e734423add](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=57e734423adda83f3b05505875343284efe3b39c) |
| 2021/02/14 | Tobin C. Harding <me@tobin.cc> | [add support for never printing hashed addresses](https://lore.kernel.org/patchwork/cover/1380477) | 新引入 no_hash_pointers 启动参数, 可以禁用 %p 只打印散列值不打印实际地址的限制, 让 %p 可以重新打印内核实际地址. | v4 ☑ 5.12-rc1 | [PatchWork 0/3,v4](https://lore.kernel.org/patchwork/cover/1380477)<br>*-*-*-*-*-*-*-* <br>[关键 commit 57e734423add](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5ead723a20e0447bc7db33dc3070b420e5f80aa6) |
| 2018/11/24 | Tetsuo Handa <penguin-kernel@I-love.SAKURA.ne.jp> | [printk: Add caller information to printk() output.](https://lore.kernel.org/lkml/1543045075-3008-1-git-send-email-penguin-kernel@I-love.SAKURA.ne.jp) | NA | v1 ☑ 5.1-rc1 | [PatchWork 0/3,v4](https://lore.kernel.org/lkml/1543045075-3008-1-git-send-email-penguin-kernel@I-love.SAKURA.ne.jp)<br>*-*-*-*-*-*-*-* <br>[关键 commit 15ff2069cb7f](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=15ff2069cb7f967dae6a8f8c176ba51447c75f00) |
| 2021/10/12 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Improvements to %pGp](https://patchwork.kernel.org/project/linux-mm/patch/20211012182647.1605095-6-willy@infradead.org) | `%pGp` 用来打印 page flag 的信息(通过 format_page_flags() 打印 `__def_pageflag_name`), 但是之前打印的方式比较麻烦, 需要使用 `%#lx(%pGp)`, 先用 hex 16 进制打印一次, 再用 `%pGp` 打印一次, 使用起来略显麻烦. 因此修改 `%pGp` 在打印 flag 的同时, 同时用 hex 打印一次. 这样单用 `%pGp` 就可以完成输出. | v1 ☐ | [PatchWork](https://patchwork.kernel.org/project/linux-mm/patch/20211012182647.1605095-6-willy@infradead.org) |
| 2017/11/08 | Linus Torvalds <torvalds@linux-foundation.org> | [stop using'%pK'for /proc/kallsyms pointer values](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c0f3ea1589394deac2d840c685f57c69e4ac4243) | commit [c0f3ea158939 stop using'%pK'for /proc/kallsyms pointer values](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c0f3ea1589394deac2d840c685f57c69e4ac4243) 4.15 之后, kallsyms 不用 %pK 打印了. 但是仍然用 kptr_restrict 控制权限. | v1 ☑ 5.1-rc1 | [ommit c0f3ea158939](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c0f3ea1589394deac2d840c685f57c69e4ac4243) |
| 2022/04/27 | Guilherme G. Piccoli <gpiccoli@igalia.com> | [The panic notifiers refactor](https://lore.kernel.org/all/20220427224924.592546-1-gpiccoli@igalia.com) | 20220427224924.592546-1-gpiccoli@igalia.com | v1 ☐☑✓ | [LORE v1,0/30](https://lore.kernel.org/all/20220427224924.592546-1-gpiccoli@igalia.com) |
| 2023/01/30 | Hyeonggon Yoo <42.hyeyoo@gmail.com> | [mm, printk: introduce new format for page_type](https://patchwork.kernel.org/project/linux-mm/cover/20230130042514.2418-1-42.hyeyoo@gmail.com/) | 716768 | v4 ☐☑ | [LORE v4,0/3](https://lore.kernel.org/r/20230130042514.2418-1-42.hyeyoo@gmail.com) |
| 2023/04/13 | Pavan Kondeti <quic_pkondeti@quicinc.com> | [printk: Export console trace point for kcsan/kasan/kfence/kmsan](https://patchwork.kernel.org/project/linux-mm/patch/20230413100859.1492323-1-quic_pkondeti@quicinc.com/) | 739441 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20230413100859.1492323-1-quic_pkondeti@quicinc.com) |

## 9.2 threaded/atomic console support
-------

[Linux 5.10 Begins Landing The Long Overdue Revamp Of printk()](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.10-printk)

[lockless ringbuffer](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.10-printk)

[[GIT PULL] printk for 5.10 (includes lockless ringbuffer)](http://lkml.iu.edu/hypermail/linux/kernel/2010.1/04077.html)

[printk: Why is it so complicated?, LPC2019](https://lpc.events/event/4/contributions/290/attachments/276/463/lpc2019_jogness_printk.pdf)

[Printbuf rebuffed for now](https://lwn.net/Articles/892611)

[Linux 6.3 printk Changes Prepare For Threaded/Atomic Consoles](https://www.phoronix.com/news/Linux-6.3-printk)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/07/09 | John Ogness <john.ogness@linutronix.de> | [printk: replace ringbuffer](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=896fbe20b4e2333fb55cc9b9b783ebcc49eee7c7) | 20200709132344.760-1-john.ogness@linutronix.de | v5 ☑ 5.10-rc1 | [LORE v5,0/4](https://lore.kernel.org/all/20200709132344.760-1-john.ogness@linutronix.de) |
| 2020/07/21 | John Ogness <john.ogness@linutronix.de> | [printk: ringbuffer: support dataless records](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d397820f36ffe4701343b6ee12687d60db0ed8db) | 20200721132528.9661-1-john.ogness@linutronix.de | v2 ☑ 5.10-rc1 | [LORE](https://lore.kernel.org/all/20200721132528.9661-1-john.ogness@linutronix.de) |
| 2020/09/14 | John Ogness <john.ogness@linutronix.de> | [printk: reimplement LOG_CONT handling](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f5f022e53b874f978dda23847173cbf2589b07f5) | 20200914123354.832-6-john.ogness@linutronix.de | v5 ☑ 5.10-rc1 | [LORE v5,0/6](https://lore.kernel.org/all/20200914123354.832-1-john.ogness@linutronix.de) |
| 2020/09/19 | John Ogness <john.ogness@linutronix.de> | [printk: move dictionaries to meta data](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f5f022e53b874f978dda23847173cbf2589b07f5) | 20200918223421.21621-1-john.ogness@linutronix.de | v2 ☑ 5.10-rc1 | [LORE v2,0/3](https://lore.kernel.org/all/20200918223421.21621-1-john.ogness@linutronix.de) |


[Linux 5.19's Printk To Offload Messages To Per-Console KThreads](https://www.phoronix.com/news/Linux-Threaded-Console-Print)

[Linux Sees A New Attempt At Threaded Console Printing](https://www.phoronix.com/news/New-Linux-printk-Threaded-Work).

[A discussion on printk()](https://lwn.net/Articles/909980)

[Printk Cleanups Ready For Linux 6.6 - Stepping Towards Threaded/Atomic Console Printing](https://www.phoronix.com/news/Linux-6.6-printk)

[Linux 6.7 Continues Work On printk Threaded Printing](https://www.phoronix.com/news/Linux-6.7-printk)

[Third Version Of Linux Atomic Console Support Posted](https://www.phoronix.com/news/Linux-Threaded-Atomic-Console-3)

[Linux 6.9 Cleans Up Printk Code While Preparing For Atomic Consoles](https://www.phoronix.com/news/Linux-6.9-Printk-Cleanup)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/02/07 | John Ogness <john.ogness@linutronix.de> | [implement threaded console printing](https://lore.kernel.org/all/20220207194323.273637-1-john.ogness@linutronix.de) | 参见 phoronix 报道 [Linux Gets Patches For Threaded Console Printing](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Threaded-Console-Print) 和 [Patches Updated For Linux To Enjoy Consoles Running At Full-Speed](https://www.phoronix.com/scan.php?page=news_item&px=Printk-v3-Consoles-Full-Speed) | v1 ☐ | [LORE v1,0/13](https://lore.kernel.org/all/20220207194323.273637-1-john.ogness@linutronix.de) |
| 2023/03/02 | John Ogness <john.ogness@linutronix.de> | [threaded/atomic console support](https://lore.kernel.org/all/87wn3zsz5x.fsf@jogness.linutronix.de) | TODO | v1 ☐☑✓ | [LORE v1,0/18](https://lore.kernel.org/all/87wn3zsz5x.fsf@jogness.linutronix.de) |
| 2024/02/18 | John Ogness <john.ogness@linutronix.de> | [wire up write_atomic() printing](https://lore.kernel.org/all/20240218185726.1994771-1-john.ogness@linutronix.de) | TODO | v2 ☐☑✓ | [LORE v2,0/26](https://lore.kernel.org/all/20240218185726.1994771-1-john.ogness@linutronix.de) |
| 2024/06/04 | John Ogness <john.ogness@linutronix.de> | [add threaded printing + the rest](https://lore.kernel.org/all/20240603232453.33992-1-john.ogness@linutronix.de) | [Updated Printk Rework Patches - Necessary Step For Mainlining Linux Real-Time "RT"](https://www.phoronix.com/news/Printk-Rework-v2-Linux) | v2 ☐☑✓ | [LORE v2,0/18](https://lore.kernel.org/all/20240603232453.33992-1-john.ogness@linutronix.de) |
| 2022/09/11 | Thomas Gleixner <tglx@linutronix.de> | [printk: A new approach - WIP](https://lore.kernel.org/all/20220910221947.171557773@linutronix.de) |  | v1 ☐☑✓ | [LORE v1,0/29](https://lore.kernel.org/all/20220910221947.171557773@linutronix.de) |
| 2023/09/16 | John Ogness <john.ogness@linutronix.de> | [provide nbcon base](https://lore.kernel.org/all/20230916192007.608398-1-john.ogness@linutronix.de) | [NBCON Console Patches Updated For Eventually Unblocking Real-Time Linux Kernel](https://www.phoronix.com/news/Linux-NCON-Consoles-v5) | v5 ☐☑✓ | [LORE v5,0/8](https://lore.kernel.org/all/20230916192007.608398-1-john.ogness@linutronix.de) |




# 10 KEXEC
-------

[ByteDance Working To Make It Faster Kexec Booting The Linux Kernel](https://www.phoronix.com/news/Bytedance-Faster-Kexec-Reboot)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/05/07 | Anthony Yznaga <anthony.yznaga@oracle.com> | [PKRAM: Preserved-over-Kexec RAM](https://lore.kernel.org/patchwork/cover/856356) | NA | v11 ☑ 4.15-rc2 | [PatchWork RFC,00/43](https://lore.kernel.org/patchwork/cover/1237362) |
| 2021/09/16 | Pavel Tatashin <pasha.tatashin@soleen.com> | [arm64: MMU enabled kexec relocation](https://patchwork.kernel.org/project/linux-mm/cover/20210802215408.804942-1-pasha.tatashin@soleen.com) | 在 kexec 重定位期间启用 MMU, 以提高重启性能.<br> 如果 kexec 功能用于快速的系统更新, 并且停机时间最少, 那么重新定位 kernel + initramfs 将占用重新引导的很大一部分时间.<br> 重定位慢的原因是它在没有 MMU 的情况下进行, 因此不能从 D-Cache 中受益. | v16 ☐ | [2021/08/02 PatchWork v16,00/15](https://patchwork.kernel.org/project/linux-mm/cover/20210802215408.804942-1-pasha.tatashin@soleen.com)<br>*-*-*-*-*-*-*-* <br>[2021/09/16 PatchWork v17,00/15](https://patchwork.kernel.org/project/linux-mm/cover/20210916231325.125533-1-pasha.tatashin@soleen.com) |
| 2022/07/25 | Albert Huang <huangjie.albert@bytedance.com> | [faster kexec reboot](https://lore.kernel.org/all/20220725083904.56552-1-huangjie.albert@bytedance.com) | TODO | v1 ☐☑✓ | [LORE v1,0/4](https://lore.kernel.org/all/20220725083904.56552-1-huangjie.albert@bytedance.com) |


# 11 perf
-------

## 11.1 perf-user
-------

### 11.1.1 perf script
-------

`struct scripting_ops` 封装了 perf 支持的高级语言解析框架. 可通过 script_spec_register() 注册, 当前支持 perl, python.

支持对 Python 使用 Linux perf 进行性能分析 [Python support for the Linux perf profiler](https://docs.python.org/3/howto/perf_profiling.html).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/11/25 | Tom Zanussi <tzanussi@gmail.com> | [perf trace: general-purpose scripting support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1ae4a971250c55e473ca53c78011fcf73809885d) | 为解析 ascii 跟踪输出的流行方法提供一种更高效、更强大的替代方法, 以便从中提取有用的信息. 为了避免所有这些的开销和复杂性, 这个补丁集提供了一个直接到脚本的解释器路径来做同样的事情, 但以一种更规则化的方式, 它利用了跟踪基础结构提供的所有事件元信息, 例如为此目的设计的 "格式文件" 中包含的事件 / 字段信息. 它允许将通用脚本语言的全部功能应用于跟踪流, 以进行非琐碎的分析, 并突然提供了大量有用的工具和模块库 (例如, 用于 Perl 的 CPAN), 以应用于创建新的、有趣的跟踪应用程序的问题. 当前只实现了一个 Perl 接口, 但其目的是使添加对其他语言(如 Python、Ruby 等) 的支持相对容易——他们所需要做的就是以 Perl 实现为例, 提供自己的 trace_scripting_ops 实现和支持函数. | v2 ☑✓ 2.6.33-rc1 | [LORE v2,0/7](https://lore.kernel.org/all/1259133352-23685-1-git-send-email-tzanussi@gmail.com) |
| 2010/01/27 | Tom Zanussi <tzanussi@gmail.com> | [perf trace: Python scripting support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=cff68e582237cae3cf456f01153202175961dfbe) | [Scripting support for perf](https://lwn.net/Articles/373842) 以及 [Python support for the Linux perf profiler](https://docs.python.org/zh-cn/dev/howto/perf_profiling.html) | v1 ☑✓ 2.6.34-rc1 | [LORE v1,0/12](https://lore.kernel.org/all/1264580883-15324-1-git-send-email-tzanussi@gmail.com) |
| 2019/09/18 | Yafang Shao <laoar.shao@gmail.com> | [introduce new perf-script page-reclaim](https://lore.kernel.org/patchwork/cover/1128886) | 为 perf 引入了一个新的 python 脚本 page-reclaim.py 页面回收脚本, 用于报告页面回收详细信息.<br> 此脚本目前的用途如下: <br>1. 识别由直接回收引起的延迟峰值 <br>2. 延迟峰值与 pageout 是否相关 <br>3. 请求页面回收的原因, 即是否是内存碎片 <br>4. 页面回收效率等. 将来, 我们还可以将其增强以分析 memcg 回收. | v1 ☐ | [PatchWork 0/2](https://lore.kernel.org/lkml/1568817522-8754-1-git-send-email-laoar.shao@gmail.com) |
| 2020/04/09 | Andreas Gerstmayr <agerstmayr@redhat.com> | [perf script: Add flamegraph.py script](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5287f926920688e1151741d49da37a533ccf1960) | 这个脚本与 d3-flame-graph 协同工作, 从 perf 生成火焰图. 它支持两种输出格式：JSON 和 HTML(默认). HTML 格式将在 `/usr/share/d3-flame-graph/d3-flamegraph-base.html` 中查找一个独立的 d3-flame-graph template, 并填充收集的堆栈. | v1 ☑✓ 5.8-rc1 | [LORE](https://lore.kernel.org/lkml/20200320151355.66302-1-agerstmayr@redhat.com) |
| 2022/12/06 | Petar Gligoric <petar.gligor@gmail.com> | [perf: introduce perf based task analyzer](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e8478b84d6ba9ccfac15dfce103062c4fa7ded2f) | 此补丁系列引入了任务分析器, 并添加了输出 csv 文件的功能, 以便在第三方脚本 (例如 pandas 和 friends) 中进行进一步分析. 任务分析器根据 sched:sched_switch 事件解析记录的性能数据文件. 它为每个任务的用户输出有用的信息, 用户可以根据自己的喜好和需要修改输出. | v2 ☑✓ 6.2-rc1 | [LORE v2,0/3](https://lore.kernel.org/all/20221206154406.41941-1-petar.gligor@gmail.com) |


### 11.1.2 show-lost-events
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/03/25 | Peter Zijlstra <a.p.zijlstra@chello.nl> | [perf_counter: Add event overlow handling](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=43a21ea81a2400992561146327c4785ce7f7be38) | 使用 mmap() 提供更好的溢出管理和更可靠的数据流. 之前方法没有任何 user-> 内核反馈, 并依赖于用户空间保持更新, 与之不同的是, 此方法依赖于用户空间将其最后一次读位置写入到控件页. 它将确保新输出不会覆盖尚未读取的事件, 同时允许丢失没有剩余空间的新事件, 并增加溢出计数器, 提供确切的事件丢失数字. 丢失事件用 PERF_EVENT_LOST(后来被改名为 [PERF_RECORD_LOST](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=cdd6c482c9ff9c55475ee7392ec8f672eddb7be6)) 标记. | v1 ☑✓ 2.6.31-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=43a21ea81a2400992561146327c4785ce7f7be38) |
| 2015/05/10 | Kan Liang <kan.liang@intel.com> | [large PEBS interrupt threshold](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=c4937a91ea56b546234b0608a413ebad90536d26) | 其中 [perf/x86/intel: Introduce PERF_RECORD_LOST_SAMPLES](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f38b0dbb491a6987e198aa6b428db8692a6480f8) 引入 PERF_RECORD_LOST_SAMPLES. | v9 ☑✓ 4.2-rc1 | [LORE v9,0/8](https://lore.kernel.org/all/1431285195-14269-1-git-send-email-kan.liang@intel.com) |
| 2011/01/29 | Arnaldo Carvalho de Melo <acme@redhat.com> | [perf top: Switch to non overwrite mode](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=93fc64f14472ae24fd640bf3834a178f59142842) | perf top 发现 PERF_RECORD_LOST 丢失事件时上报 WARN. | v1 ☑✓ 2.6.39-rc1  | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=93fc64f14472ae24fd640bf3834a178f59142842) |
| 2011/10/29 | Arnaldo Carvalho de Melo <acme@redhat.com> | [perf hists browser: Warn about lost events](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7b27509fc62686c53e9301560034e6b0b001174d) | 发现 PERF_RECORD_LOST 丢失事件时上报 WARN. | v1 ☑✓ 3.2-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7b27509fc62686c53e9301560034e6b0b001174d) |
| 2018/01/07 | Jiri Olsa <jolsa@kernel.org> | [perf script: Add support to display lost events](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3d7c27b6dbca4c90e7d921b45c2240e7c3cb92a2) | 用户态 perf 提供 `perf script --show-lost-events`, 显示 PERF_RECORD_LOST 信息. | v1 ☑✓ 4.16-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3d7c27b6dbca4c90e7d921b45c2240e7c3cb92a2) |
| 2022/06/16 | Namhyung Kim <namhyung@kernel.org> | [perf/core: Add a new read format to get a number of lost samples](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=119a784c81270eb88e573174ed2209225d646656) | 有时候用户想知道样本的准确数量, 即使存在一定数量的样本丢失. 当因为环缓冲区冲突造成的事件丢失使用 PERF_RECORD_LOST 标记记录, 虽然很难知道每个事件丢失的次数, 但是聊胜于无. 因此定义 PERF_FORMAT_LOST, 通过 event->lost_samples 记录丢失的次数, 以便从用户空间获取到它. | v4 ☑✓ 6.0-rc1 | [LORE](https://lore.kernel.org/all/20220616180623.1358843-1-namhyung@kernel.org) |
| 2022/08/16 | Namhyung Kim <namhyung@kernel.org> | [perf tools: Support reading PERF_FORMAT_LOST (v2)](https://lore.kernel.org/all/20220816221747.275828-1-namhyung@kernel.org) | TODO | v2 ☐☑✓ | [LORE 0/4](https://lore.kernel.org/lkml/20220815190106.1293082-1-namhyung@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/4](https://lore.kernel.org/all/20220816221747.275828-1-namhyung@kernel.org) |

## 11.2 ARM SPE
-------



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/10/12 | Will Deacon <will.deacon@arm.com> | [Add support for the ARMv8.2 Statistical Profiling Extension](https://lore.kernel.org/lkml/1507811438-2267-1-git-send-email-will.deacon@arm.com) | perf PMU 驱动支持 SPE | v1 ☑ 4.15-rc1 | [PatchWork v6 0/7](https://lore.kernel.org/lkml/1507811438-2267-1-git-send-email-will.deacon@arm.com) |
| 2018/01/14 | Kim Phillips <kim.phillips@arm.com> | [perf tools: Add ARM Statistical Profiling Extensions (SPE) support](https://lore.kernel.org/lkml/1507811438-2267-1-git-send-email-will.deacon@arm.com) | perf tools 支持 SPE. 这个版本实现的功能还比较简单, 直接把 SPE 的 format 数据导出到了用户态, 由 perf 直接解析, 并没有提供更进一步的 profiling 的功能. | v1 ☑ 4.16-rc1 | [PatchWork v6](https://lore.kernel.org/patchwork/cover/1128886) |
| 2020/05/30 | Leo Yan <leo.yan@linaro.org> | [perf arm-spe: Add support for synthetic events](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a54ca194981be3707213437a67792b88e08264fe) | 支持将 SPE 的事件进行分类解析, 可以显示热点以及汇编等信息. | v1 ☑ 5.8-rc1 | [PatchWork v8 0/3](https://lore.kernel.org/lkml/20200530122442.490-1-leo.yan@linaro.org) |
| 2020/11/06 | Leo Yan <leo.yan@linaro.org> | [perf mem/c2c: Support AUX trace](https://lore.kernel.org/lkml/20201106094853.21082-1-leo.yan@linaro.org) | NA. | v1 ☑ 5.8-rc1 | [PatchWork v4 0/9](https://lore.kernel.org/lkml/20201106094853.21082-1-leo.yan@linaro.org) |
| 2021/02/12 | James Clark <james.clark@arm.com> | [perf arm-spe: Enable sample type PERF_SAMPLE_DATA_SRC](https://lore.kernel.org/all/20210211133856.2137-1-james.clark@arm.com) | 在 perf 数据中为 Arm SPE 支持解析 PERF_SAMPLE_DATA_SRC 数据, 当输出跟踪数据时, 它告诉 perf 它在内存事件中包含数据源. | v1 ☑ 5.8-rc1 | [PatchWork v2 1/6](https://lore.kernel.org/all/20210211133856.2137-1-james.clark@arm.com/) |
| 2020/12/13 |  Leo Yan <leo.yan@linaro.org>| [perf c2c: Sort cacheline with all loads](https://lore.kernel.org/all/20201213133850.10070-1-leo.yan@linaro.org) | 实现类似 x86 下 c2c 的功能, 由于 ARM SPE 中没有默认实现类似 X86 hitM 的 data_src, 因此不能有效地判断伪共享. 当前实现方案是将所有的 load 操作排序, 方便开发人员针对伪共享进行分析. | v1 ☐ | [PatchWork v2 00/11](https://lohttps://lore.kernel.org/all/20201213133850.10070-1-leo.yan@linaro.org) |
| 2021/12/16 | German Gomez <german.gomez@arm.com> | [perf arm-spe: Synthesize SPE instruction events](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ff8752d7617da301ad3b7ef18caa58d135ee8c3c) | 20211216152404.52474-1-german.gomez@arm.com | v3 ☐☑✓ 5.17-rc1 | [LORE](https://lore.kernel.org/all/20211216152404.52474-1-german.gomez@arm.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ff8752d7617da301ad3b7ef18caa58d135ee8c3c) |
| 2021/12/01 | Namhyung Kim <namhyung@kernel.org> | [perf tools: Add SPE total latency as PERF_SAMPLE_WEIGHT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b0fde9c6e291e528878ea3713997777713fe44c6) | 20211201220855.1260688-1-namhyung@kernel.org | v2 ☐☑✓ 5.17-rc1 | [LORE](https://lore.kernel.org/all/20211201220855.1260688-1-namhyung@kernel.org), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b0fde9c6e291e528878ea3713997777713fe44c6) |
| 2022/04/29 | Shaokun Zhang <zhangshaokun@hisilicon.com> | [drivers/perf: arm_spe: Expose saturating counter to 16-bit](https://patchwork.kernel.org/project/linux-arm-kernel/patch/20220429063307.63251-1-zhangshaokun@hisilicon.com/) | 636823 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220429063307.63251-1-zhangshaokun@hisilicon.com) |



## 11.3 Userspace counter access
-------

x86 和 arm64 都支持直接访问用户空间中的事件计数器. 访问序列并不简单, 目前存在于 perf 测试代码 (tools/perf/arch/x86/tests/rdpmc.c) 中, 在 PAPI 和 libpfm4 等项目中有类似的用例程序.

为了支持 usersapce 访问, 必须首先使用 perf_evsel__mmap() 映射事件. 然后, 对 perf_evsel__read() 对 PMU 进行读取.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2011/12/21 | Peter Zijlstra <a.p.zijlstra@chello.nl> | [perf, x86: Implement user-space RDPMC support, to allow fast, user-space access to self-monitoring counters](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=fe4a330885aee20f233de36085fb15c38094e635) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/tip-mwxab34dibqgzk5zywutfnha@git.kernel.org) |
| 2021/12/08 | Rob Herring <robh@kernel.org> | [arm64 userspace counter support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=aa1005d15d2aee10e5b93a25db076c47e05c4efa) | ARM64 的 PMU 是支持用户态直接访问的. 与 x86 上 rdpmc 机制类似, 为了防止出现安全问题, 当前只允许用户态访问 thread bound events, 在启用时, 为了防止从其他任务泄漏禁用的 PMU 数据, 还会有一些额外的开销来清除脏计数器. | v13 ☑✓ 5.17-rc1 | [LORE v13,0/5](https://lore.kernel.org/all/20211208201124.310740-1-robh@kernel.org) |
| 2021/04/14 | Rob Herring <robh@kernel.org> | [libperf userspace counter access](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=47d01e7b9999b9591077a59a1ecec11c6ce570de) | libperf 支持用户态读取 PMU, 当前只支持了 X86 | v8 ☑✓ 5.13-rc1 | [LORE v8,0/4](https://lore.kernel.org/all/20210414155412.3697605-1-robh@kernel.org) |
| 2022/02/01 | Rob Herring <robh@kernel.org> | [libperf: Add arm64 support to perf_mmap__read_self()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=407eb43ae87c969d98746c3274ae5d0f977b102e) | libperf 支持 ARM64 用户态读取 PMU. | v9 ☑✓ 5.17-rc3 | [LORE](https://lore.kernel.org/all/20220201214056.702854-1-robh@kernel.org) |

## 11.4 分支预测
-------

### 11.4.1 AMD Branch Samling "BRS" Feature
-------

[AMD Branch Sampling Support Being Worked On For Linux](https://www.phoronix.com/scan.php?page=news_item&px=AMD-BRS-Branch-Sampling)

[AMD Branch Sampling"BRS"Feature To Land With Linux 5.19](https://www.phoronix.com/scan.php?page=news_item&px=AMD-Branch-Sampling-v5.19)

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/03/22 | Stephane Eranian <eranian@google.com> | [perf/x86/amd: Add AMD Fam19h Branch Sampling support](https://lore.kernel.org/all/20220322221517.2510440-1-eranian@google.com) | 引入 CONFIG_PERF_EVENTS_AMD_BRS. perf 支持 BRS. AMD 系列 19h "Zen 3" 处理器新增了分支采样功能 BRS, 用于收集代码执行期间所采用分支的详细信息. 该功能可用于 AMD 处理器上的 AutoFDO 样式优化, 编译器利用收集的硬件数据来做出更明智和准确的优化决策. | v7 ☑✓ 5.19-rc1 | [LORE v7,0/13](https://lore.kernel.org/all/20220322221517.2510440-1-eranian@google.com) |


## 11.5 perf-KWork
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/07/09 | Yang Jihong <yangjihong1@huawei.com> | [perf: Add perf kwork](https://lore.kernel.org/all/20220709015033.38326-1-yangjihong1@huawei.com) | 开发者经常需要分析内核工作的时间属性, 例如 irq、softirq 和工作队列, 包括特定中断的延迟和运行时间. 目前, 这些事件具有内核跟踪点, 但 perf 工具不直接分析这些事件的延迟. perf kwork 工具用于跟踪内核工作的时间属性(如 irq、softirq 和 workqueue), 包括运行时、延迟和时间历史, 使用 perf 工具中的基础设施来允许跟踪额外的目标, 我们还使用 bpf 跟踪来收集和过滤内核中的数据, 以解决大 perf 数据量和额外文件系统中断的问题. | v3 ☐☑✓ | [LORE v3,0/17](https://lore.kernel.org/all/20220709015033.38326-1-yangjihong1@huawei.com) |

## 11.6 perf-lock
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/06/15 | Namhyung Kim <namhyung@kernel.org> | [perf lock: New lock contention tracepoints support (v4)](https://lore.kernel.org/all/20220615163222.1275500-1-namhyung@kernel.org) | TODO | v4 ☐☑✓ | [LORE v4,0/7](https://lore.kernel.org/all/20220615163222.1275500-1-namhyung@kernel.org) |

## 11.7 perf bench
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/07/04 | Marco Elver <elver@google.com> | [perf/hw_breakpoint: Optimize for thousands of tasks](https://lore.kernel.org/all/20220704150514.48816-1-elver@google.com) | TODO | v3 ☐☑✓ | [LORE v3,0/14](https://lore.kernel.org/all/20220704150514.48816-1-elver@google.com) |
| 2023/10/17 | Namhyung Kim <namhyung@kernel.org> | [perf bench sched pipe: Add -G/--cgroups option](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=79a3371bdf453bedb9d2c80df5adc201dddc11b5) | TODO | v4 ☐☑✓ 6.7-rc1 | [LORE](https://lore.kernel.org/all/20231017202342.1353124-1-namhyung@kernel.org) |


## 11.8 TOPDOWN
-------

[Support standalone metrics and metric groups for perf](https://lore.kernel.org/all/20170831194036.30146-1-andi@firstfloor.org)


### 11.8.1 stat topdown
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/05/24 | Andi Kleen <andi@firstfloor.org> | [perf stat: Basic support for TopDown in perf stat](https://lore.kernel.org/all/1464119559-17203-1-git-send-email-andi@firstfloor.org) | perf stat 支持 topdown 分析. | v1 ☑ 4.8-rc1 | [PatchWork 1/4](https://lore.kernel.org/all/1464119559-17203-1-git-send-email-andi@firstfloor.org) |
| 2021/02/02 | Kan Liang <kan.liang@linux.intel.com> | [perf stat: Support L2 Topdown events](https://lore.kernel.org/lkml/1612296553-21962-9-git-send-email-kan.liang@linux.intel.com) | perf stat 支持 Level 2 级别 topdown 分析. [perf core PMU support for Sapphire Rapids (User tools)](https://lore.kernel.org/lkml/1612296553-21962-1-git-send-email-kan.liang@linux.intel.com) 系列中的其中一个补丁. | v1 ☑ 5.12-rc1 | [PatchWork 1/4](https://lore.kernel.org/lkml/1612296553-21962-9-git-send-email-kan.liang@linux.intel.com) |


### 11.8.2 metricsgroup topdown
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/08/31 | Andi Kleen <andi@firstfloor.org> | [Support standalone metrics and metric groups for perf](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=b90f1333ef08d2a497ae239798868b046f4e3a97) | 向 perf-stat 添加对 JSON 文件中指定的独立度量 metrics 的支持. 度量 metrics 使用多个事件来计算更高级别结果 (例如 IPC, TOPDOWN 等) 的公式. 对于更复杂的度量(比如 pipeline, TOPDOWN), 需要具有特定于微架构的配置, 因此将度量 metrics 与 JSON 事件列表联系起来是有意义的. 以前的度量始终与事件关联, 并随该事件自动启用. 但现在改变它, 可以有独立的指标. 它们与事件处于相同的 JSON 数据结构中, 但没有事件名称, 只有度量名称. 同时允许在度量组中组织度量, 这允许一次选择几个相关度量的快捷方式. | v3 ☑✓ 4.15-rc1 | [LORE v3,0/11](https://lore.kernel.org/all/20170831194036.30146-2-andi@firstfloor.org) |
| 2017/08/31 | Andi Kleen <andi@firstfloor.org> | [perf arm64 metricgroup support](https://lore.kernel.org/all/20170831194036.30146-1-andi@firstfloor.org) | 为 perf stat 添加[对 JSON 文件中指定的独立指标](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b18f3e365019de1a5b26a851e123f0aedcce881f) 的通用支持. 这些指标是一个公式, 它使用多个事件来计算更高级别的结果(例如 IPC, TOPDOWN 分析等). 添加一个新的 `-M /--metrics` 选项来添加指定的度量或度量组. 并增加了对 Intel X86 平台的支持. 通过这些 JSON 文件定义的事件组合度量, 可以很好的支持 TOPDOWN 分析. | v3 ☑ 4.15-rc1 | [PatchWork v3,00/11](https://lore.kernel.org/all/20170831194036.30146-1-andi@firstfloor.org) |
| 2020/09/11 | Kan Liang <kan.liang@linux.intel.com> | [TopDown metrics support for Ice Lake (perf tool)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=328781df86fa8f9299219c74ffae876bd625c6d1) | 为 perf metrics 分析增加对 Ice Lake 的支持. 将原本 group 重命名为 topdown. | v3 ☑ 5.10-rc1 | [PatchWork v3,0/4](https://lore.kernel.org/lkml/20200911144808.27603-1-kan.liang@linux.intel.com) |
| 2021/04/07 | John Garry <john.garry@huawei.com> | [perf arm64 metricgroup support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0cc177cfc95d565e1a458136a592b0bd6d487db0) | perf 支持 HiSilicon hip08 平台的 topdown metric. 支持到 Level 3. 自此鲲鹏 920 的 ARM64 服务器上, 可以使用:<br>`sudo perf stat -M TopDownL1 sleeep 1`<br> 来进行 TopDown 分析了. | v1 ☑ 5.13-rc1 | [PatchWork 0/5](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1614784938-27080-1-git-send-email-john.garry@huawei.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/6](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1616668398-144648-1-git-send-email-john.garry@huawei.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3,0/6](https://patchwork.kernel.org/project/linux-arm-kernel/cover/1617791570-165223-1-git-send-email-john.garry@huawei.com) |
| 2022/05/28 | zhengjun <zhengjun.xing@linux.intel.com> | [perf vendor events intel: Add metrics for Sapphirerapids](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1bcca2b1bd67f3c0e5c3a88ed16c6389f01a5b31) | TODO | v2 ☑✓ 5.19-rc1 | [LORE v2,0/2](https://lore.kernel.org/all/20220528095933.1784141-1-zhengjun.xing@linux.intel.com) |
| 2022/08/25 | zhengjun.xing@linux.intel.com <zhengjun.xing@linux.intel.com> | [perf stat: Capitalize topdown metricsnel.org/all/20220825015458.3252239-1-zhengjun.xing@linux.intel.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/9-1-zhengjun.xing@linux.intel.com) |
| 2022/10/21 | Shang XiaoJing <shangxiaojing@huawei.com> | [perf vendor events arm64: Fix incorrect Hisi hip08 L3 metrics](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e9229d5b6254a75291536f582652c599957344d2) | TODO | v2 ☑✓ 6.1-rc3 | [LORE v2,0/3](https://lore.kernel.org/all/20221021105035.10000-1-shangxiaojing@huawei.com) |
| 2022/12/14 | Sandipan Das <sandipan.das@amd.com> | [perf vendor events amd: Add Zen 4 events and metrics](https://lore.kernel.org/all/20221214082652.419965-1-sandipan.das@amd.com) | [Linux 6.2 Adds AMD Zen 4 Pipeline Utilization Data To Help Find Performance Bottlenecks](https://www.phoronix.com/news/LInux-6.2-AMD-Zen-4-Events) | v2 ☐☑✓ 6.2-rc1 | [LORE v2,0/4](https://lore.kernel.org/all/20221214082652.419965-1-sandipan.das@amd.com)|
| 2023/06/07 | kan.liang@linux.intel.com <kan.liang@linux.intel.com> | [New metricgroup output in perf stat default mode](https://lore.kernel.org/all/20230607162700.3234712-1-kan.liang@linux.intel.com) | 在默认模式下, metricgroup 的当前输出包括事件和度量, 这是不必要的, 并且使输出难以读取. 此外, 由于度量中的事件不同, 不同的 ARCH(甚至不同代的 ARCH)可能具有不同的输出格式. 该补丁提出了一种新的输出格式, 只输出每个度量的值和度量组名称. 它可以在 ARCH 和各代之间带来干净一致的输出格式. | v1 ☐☑✓ | [LORE v1,0/8](https://lore.kernel.org/all/20230607162700.3234712-1-kan.liang@linux.intel.com) |
| 2024/03/29 | weilin.wang@intel.com <weilin.wang@intel.com> | [TPEBS counting mode support](https://lore.kernel.org/all/20240329191224.1046866-1-weilin.wang@intel.com) | TODO | v6 ☐☑✓ | [LORE v6,0/5](https://lore.kernel.org/all/20240329191224.1046866-1-weilin.wang@intel.com) |



## 11.9 perf bperf
-------


perf 使用 PMC (performance monitoring counters)来监控系统的性能. PMC 是有限的硬件资源. 例如, Intel cpu 每个 cpu 有 3 个固定 PMC 和 4 个可编程 PMC.

现代数据中心系统以许多不同的方式使用这些 PMC: 系统级监控、(可能是嵌套的)容器级监控、每个进程监控、分析 (在示例模式下) 等等. 在某些情况下, 活动的 perf_event 比可用的硬件 PMC 还多. 为了允许所有 perf_events 都有机会运行, 有必要对事件进行昂贵的时间复用.

另一方面, 许多监视工具计算公共度量 (周期、指令). 让多个工具创建多个 "循环" 的 perf_event 并占用多个 PMC 是一种浪费.

bperf 试图通过允许多个 "周期" 或 "指令" 的 perf_event (在不同作用域) 共享 PMU 来减少这种浪费. bperf 没有让每个 perf-stat 会话读取自己的 perf_events, 而是使用 BPF 程序读取 perf_events 并将读取的数据聚合到 BPF 映射中. 然后, perf-stat 会话从这些 BPF 映射中读取值.

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2020/12/28 | Song Liu <songliubraving@fb.com> | [Introduce perf-stat -b for BPF programs](https://lore.kernel.org/all/20201228174054.907740-1-songliubraving@fb.com) | 引入了 perf-stat-b 选项来统计 BPF 程序的事件. 这与 bpftool 程序配置文件类似. 但是 perf-stat 使它更加灵活. | v6 ☐☑✓ | [LORE v6,0/4](https://lore.kernel.org/all/20201228174054.907740-1-songliubraving@fb.com) |
| 2021/03/16 | Song Liu <songliubraving@fb.com> | [perf-stat: share hardware PMCs with BPF](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7fac83aaf2eecc9e7e7b72da694c49bb4ce7fdfc) | TODO | v2 ☐☑✓ 5.13-rc1 | [LORE](https://lore.kernel.org/all/20210312020257.197137-1-songliubraving@fb.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/3](https://lore.kernel.org/all/20210316211837.910506-1-songliubraving@fb.com)|
| 2021/06/25 | Namhyung Kim <namhyung@kernel.org> | [perf stat: Enable BPF counters with --for-each-cgroup](https://lore.kernel.org/all/20210625071826.608504-1-namhyung@kernel.org) | bperf 被添加到使用 BPF 来计算各种性能事件目的, 对其扩展 cgroup 的支持. 与其他 bperf 不同, 它不与其他进程共享事件, 但它可以减少 perf 会话中每个被监视的 GROUP 组的不必要事件 (以及多路复用的开销). 当 `for-each-cgroup` 与 `BPF -counters` 一起使用时, 它将在每个 cpu 内部打开 cgroup-switches 事件, 并附加新的 BPF 程序来读取给定的 perf_events 并聚合 cgroups 的结果. 只有当 task 切换到另一个 group 组中的 task 时才会调用它. | v4 ☐☑✓ | [LORE v4,0/4](https://lore.kernel.org/all/20210625071826.608504-1-namhyung@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v5](https://lore.kernel.org/all/20210701211227.1403788-1-namhyung@kernel.org)|

## 11.10 perf other
-------

| 2021/07/20 | kan.liang@linux.intel.com <kan.liang@linux.intel.com> | [perf: Save PMU specific data in task_struct](https://lore.kernel.org/all/1626788420-121610-1-git-send-email-kan.liang@linux.intel.com) | 某些特定于 PMU 的数据必须在上下文切换期间保存 / 恢复, 例如 LBR 调用堆栈数据. 目前, 数据保存在事件上下文结构中, 但仅针对每个流程的事件. 对于系统范围的事件, 由于上下文切换后缺少 LBR 调用堆栈数据, 与按进程模式相比, LBR 调用栈总是更短. | v6 ☐☑✓ | [LORE v6,0/6](https://lore.kernel.org/all/1626788420-121610-1-git-send-email-kan.liang@linux.intel.com) |



# 12 KPROBE
-------

## 12.1 OPTPROBE
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/18 | Qi Liu <liuqi115@huawei.com> | [arm64: Enable OPTPROBE for arm64](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20210818073336.59678-1-liuqi115@huawei.com) | 为 ARM64 引入了 optprobe, 使用分支指令替换探测指令.<br> 作者在 Hip08 平台上的进行了测试, optprobe 可以将延迟降低到正常 kprobe 的 `1/4` | v4 ☐ | [Patchwork v4,0/2](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20210818073336.59678-1-liuqi115@huawei.com) |
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


[CONFIG_OPTIMIZE_INLINING 引发 Go 运行时 bug 调试过程解析](https://blog.csdn.net/qq_40267706/article/details/78817614)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/03/20 | Ingo Molnar <mingo@elte.hu> | [x86: add optimized inlining](https://lore.kernel.org/patchwork/patch/1394812) | 引入 CONFIG_OPTIMIZE_INLINING, 允许 GCC 自己确定哪个 inline 函数真的需要内联, 而不是强制内允许 GCC. 之前 Linux 强制 gcc 总是通过 gcc 属性来内联这些函数, 这可以一定程度提升函数调用的性能, 但是对于一个比较大的函数, 强制将其 inline, 将不可避险增加内核映像的大小, 开启这个选项后, 即使有些函数被标记为 inline, GCC 也会将这些不再 inline. 特别是当用户已经选择了 CONFIG_OPTIMIZE_FOR_SIZE=y 时, 这可以在内核映像大小上产生巨大的差异. 使用标准的 Fedora .config 测试, vmlinux 比之前减少了 2.3% | v1 ☑ 2.6.26-rc1 | [Patchwork](https://patches.linaro.org/patch/160631), [LKML](https://lkml.org/lkml/2019/3/20/63)<br>*-*-*-*-*-*-*-* <br>[关键 commit 57e734423add](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=60a3cdd0639473c79c253bc08c8ef8f882cca107) |
| 2019/03/20 | Masahiro Yamada <yamada.masahiro@socionext.com> | [compiler: allow all arches to enable CONFIG_OPTIMIZE_INLINING](https://lore.kernel.org/patchwork/patch/1394812) | [commit 60a3cdd06394 ("x86: add optimized inline ")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=60a3cdd0639473c79c253bc08c8ef8f882cca107) 引入了 CONFIG_OPTIMIZE_INLINING, 但它只在 x86 上可用.<br> 这个想法显然是不可知论的, 因此当前提交将配置项从 `arch/x86/Kconfig.debug` 移到 `lib/Kconfig.debug`, 以便所有架构都能从中受益. 这个提交[早在 2008 年就有过讨论](https://lkml.org/lkml/2008/4/27/7). | v1 ☑ 5.2-rc1 | [Patchwork](https://patches.linaro.org/patch/160631), [LKML](https://lkml.org/lkml/2019/3/20/63)<br>*-*-*-*-*-*-*-* <br>[LKML v3 00/10](https://lkml.org/lkml/2019/4/22/922)<br>*-*-*-*-*-*-*-* <br>[关键 commit 57e734423add](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9012d011660ea5cf2a623e1de207a2bc0ca6936d) |
| 2019/08/30 | Masahiro Yamada <yamada.masahiro@socionext.com> | [compiler: enable CONFIG_OPTIMIZE_INLINING forcibly](https://lore.kernel.org/patchwork/patch/1122097) | NA | v1 ☑ 5.4-rc1 | [Patchwork](https://patches.linaro.org/patch/160631), [LKML](https://lkml.org/lkml/2019/8/29/1772) |


## 13.2 Layout Optimizer
-------

### 13.2.1 PGO
-------

2020 年 GNU Tools Cauldron 大会受到了疫情影响取消, 在 8 月份的 LPC2020 会议上, 组委会特别增加了一个 GNU Tools 分会场来讨论工具链相关的议题. 来自微软的 Ian Bearman 展示了他的团队使用 Profile Guided Optimization(PGO) 和 Link-time Optimization(LTO) 来优化 Linux 内核取得的一些成果. 参见 [Exploring Profile Guided Optimization of the Linux Kernel](https://lpc.events/event/7/contributions/771/), [微软技术团队：用这种方式编译 Linux，可直接改善系统性能](https://zhuanlan.zhihu.com/p/235032245)[Profile-guided optimization for the kernel](https://lwn.net/Articles/830300)


[使用 GCC 的 PGO(Profile-guided Optimizztion) 优化整个系统](https://blog.csdn.net/tianya_lu/article/details/125235253)

[Profile Guided Optimizations (PGO) Likely Coming To Linux 5.14 For Clang](https://www.phoronix.com/news/Clang-PGO-For-Linux-Next)

[Following LTO, Linux Kernel Patches Updated For PGO To Yield Faster Performance](https://www.phoronix.com/news/Clang-PGO-Linux-Kernel)

[meugur/kernel-pgo](https://github.com/meugur/kernel-pgo)
[h0tc0d3/linux_pgo](https://github.com/h0tc0d3/linux_pgo)

[基于 PGO 的安卓手机内核编译优化——需求分析](https://bbs.csdn.net/topics/604049200)

[Rethinking Compiler Optimizations for the Linux Kernel:An Explorative Stud](https://yaoguopku.github.io/papers/Yuan-APSys-15.pdf)

[Experiences in Profile-Guided Operating System Kernel Optimization](https://yaoguopku.github.io/papers/Yuan-ApSys-14.pdf)


### 13.2.2 LTO(Link Time Optimization)
-------


编译器一次只能对一个单元 (比如一个 `.c` 文件) 进行编译和优化(生成 `.o`), 因此没法跨文件范围做优化(如内联等), 只能实现局部优化, 缺乏全局观. 而 LTO, 就是在链接时通过全局视角进行优化操作, 从而对整个系统进行更加极致的优化. 跨模块优化的效果, 也即开启 LTO 主要有这几点好处:

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
| 2020/12/11 | Masahiro Yamada <yamada.masahiro@socionext.com> | [Add support for Clang LTO](https://patchwork.kernel.org/project/linux-kbuild/cover/20201211184633.3213045-1-samitolvanen@google.com) | 本补丁系列增加了对使用 Clang 的链接时间优化(LTO) 构建内核的支持. 除了性能之外, LTO 的主要动机是允许在内核中使用 Clang 的控制流完整性(CFI). 自 2018 年以来, 谷歌已经发布了数百万个运行 LTO + CFI 三个主要内核版本的 Pixel 设备.<br> 大多数补丁是为了处理 LLVM 位码而进行的构建系统更改, Clang 使用 LTO 而不是 ELF 对象文件生成 LLVM 位码, 将 ELF 处理推迟到后面的阶段, 并确保初始化调用顺序.<br>arm64 支持依赖于 [Will 的内存排序补丁](https://git.kernel.org/pub/scm/linux/kernel/git/arm64/linux.git/log/?h=for-next/lto). 早期版本同时做了 X86_64 和 ARM64 的适配和支持. | v9 ☑ 5.12-rc1 | [Patchwork v6,00/25](https://patchwork.kernel.org/project/linux-kbuild/cover/20201211184633.3213045-1-samitolvanen@google.com)<br>*-*-*-*-*-*-*-* <br>v7 之后不再包含 X86 的使能的补丁.<br>*-*-*-*-*-*-*-* <br>[Patchwork v9,00/16](https://patchwork.kernel.org/project/linux-kbuild/cover/20201211184633.3213045-1-samitolvanen@google.com) |
| 2021/04/07 | Bill Wendling <morbo@google.com> | [pgo: add clang's Profile Guided Optimization infrastructure](https://lore.kernel.org/all/20210407211704.367039-1-morbo@google.com) | 20210116094357.3620352-1-morbo@google.com | v9 ☐☑✓ | [LORE](https://lore.kernel.org/all/20210407211704.367039-1-morbo@google.com) |
| 2021/4/29 | Wende Tan <twd2.me@gmail.com> | [x86: Enable clang LTO for 32-bit as well](https://patchwork.kernel.org/project/linux-riscv/cover/20210719205208.1023221-1-twd2.me@gmail.com) | CLANG LTO 支持 X86 32 位. | v1 ☑ 5.14-rc1 | [LKML](https://lkml.org/lkml/2021/4/29/873) |
| 2021/07/19 | Wende Tan <twd2.me@gmail.com> | [RISC-V: build: Allow LTO to be selected](https://patchwork.kernel.org/project/linux-riscv/cover/20210719205208.1023221-1-twd2.me@gmail.com) | NA | v1 ☐ | [Patchwork 0/3](https://patchwork.kernel.org/project/linux-riscv/cover/20210719205208.1023221-1-twd2.me@gmail.com) |
| 2022/11/14 | Jiri Slaby (SUSE) <jirislaby@kernel.org> | [gcc-LTO support for the kernel](https://lore.kernel.org/all/20221114114344.18650-1-jirislaby@kernel.org) | [Patches Posted For GCC LTO Optimizing The Linux Kernel](https://www.phoronix.com/news/GCC-LTO-Linux-2022) | v1 ☐☑✓ | [LORE v1,00/46](https://lore.kernel.org/all/20221114114344.18650-1-jirislaby@kernel.org) |


其他相关

[Unified LTO Bitcode Front-End Comes Together For LLV](https://www.phoronix.com/news/LLVM-Unified-LTO-Front-End).

### 13.2.3 BOLT'ing
-------

[[RFC] BOLT: A Framework for Binary Analysis, Transformation, and Optimization](https://discourse.llvm.org/t/rfc-bolt-a-framework-for-binary-analysis-transformation-and-optimization/56722)

[BOLT: A Practical Binary Optimizer for Data Centers and Beyond](https://arxiv.org/pdf/1807.06735.pdf)

几年来, Facebook 的工程师们一直在研究 [BOLT](https://www.phoronix.com/scan.php?page=news_item&px=Facebook-BOLT-Optimize-Binaries), 以此加速 Linux/ELF 二进制文件.

BOLT 是一个二进制优化和布局工具, 它是一个 Facebook 孵化器项目, 用于加速 Linux x86-64/AArch64 ELF 二进制文件. 这个工具能够分析和重新排布的可执行程序, 以产生比编译器的 LTO 和 PGO 优化所能实现的更快的性能. 2022 年 1 月, [BOLT Merged Into LLVM To Optimize Binaries For Faster Performance](https://www.phoronix.com/news/LLVM-Lands-BOLT). 允许优化二进制文件的布局, 作为链接后的步骤, 以提高性能. 与配置文件引导优化 (PGO) 一样, BOLT 首先需要分析步骤来生成性能记录以反馈优化过程, 但收益可能很大.

facebook 在 LPC-2021 公布了其 [最新基于 BOLT 优化 Linux 内核的进展](https://www.phoronix.com/news/Facebook-BOLTing-The-Kernel), 这项工作与允许 Linux 内核的配置文件引导优化(PGO) 的挑战类似, 与现有的 BOLT 专注于仅优化 ELF 应用程序可执行性相比, BOLT'ing 的 Linux 内核在正确分析 / 采样内核和相关优化工作负载、内核的大规模代码基数、模块与内核代码等方面面临着类似的复杂障碍. 从公布的信息上看效果不错, 在 PGO + LTO 编译器优化的基础之上仍然带来了两位数的提升(double digit speedups"). 这些提速是通过优化可执行工具的代码布局来实现更高效的硬件页面使用和指令缓存. 参见 [slides](https://linuxplumbersconf.org/event/11/contributions/974/attachments/923/1793/Optimizing%20Linux%20Kernel%20with%20BOLT.pdf).

BOLT 之前代码在 [github 开源](https://github.com/facebookincubator/BOLT), 随后在 2022 合并到[主线 LLVM](https://github.com/llvm/llvm-project/tree/main/bolt), 参见 [BOLT Close To Merging Into LLVM For Optimizing Performance Of Binaries](https://www.phoronix.com/scan.php?page=news_item&px=BOLT-Inches-To-LLVM). 并默认情况下在 Linux x86_64 和 AArch64 测试版本中被打开, [LLVM's BOLT Flipped On By Default For Linux x86/AArch64 Test Releases](https://www.phoronix.com/news/LLVM-BOLT-Default-Test-Releases).

Facebook 一直致力于 BOLT 的 Linux 内核以获得更高的性能, [Meta Continues Working On BOLT'ing The Linux Kernel For Greater Performance](https://www.phoronix.com/news/LLVM-BOLT-Linux-Kernel-2024), 经过 2 年的努力, 这项工作终于于 2024 年 2 合入 LLVM 主线. 参见 [BOLT: Add writing support for Linux kernel ORC #80950](https://github.com/llvm/llvm-project/pull/80950).

LLVM BOLT 优化 GNOME 的 Pango 净改进 ~6%, 参见 phoronix 报道 [LLVM BOLT Optimizations Net ~6% Improvement For GNOME's Pango](https://www.phoronix.com/news/LLVM-BOLT-Faster-Pango).

ARM 的编译器工程师利用 BOLT 来创建二进制分析工具, 以审查安全强化选项的正确性. 该工具旨在验证堆栈保护器、堆栈冲突保护、分支保护、控制流保护等功能. 二进制分析工具将验证整个程序的强化功能, 并且比今天使用的有限安全强化测试要详细得多. 参见 phoronix 报道 [LLVM's BOLT Being Adapted To Analyze Security Hardening Of Binaries](https://www.phoronix.com/news/LLVM-BOLT-Security-Hardening).

### 13.2.4 Propeller
-------

[Propeller: A Profile Guided, Relinking Optimizer for Warehouse Scale Applications](https://github.com/google/llvm-propeller)
[Google's Propeller Is Beginning To Be Upstreamed For Spinning Faster Program Binaries](https://www.phoronix.com/news/Google-Propeller-Some-Upstream)
[[RFC] Propeller: A frame work for Post Link Optimizations](https://discourse.llvm.org/t/rfc-propeller-a-frame-work-for-post-link-optimizations/53161)


### 13.2.5 Thin Layout Optimizer
-------

Thin-Layout-Optimizer 是一种新的代码布局优化器, 主要强调易用性和易采用性, 同时与 BOLT/Propeller 相比在性能上保持竞争力. 与 BOLT/Propeller 一样, Thin-Layout-Optimizer 在使用 Linux perf 和 LBR 生成的配置文件上运行. Thin-Layout-Optimizer 不会反汇编二进制文件, 而是通过类似于 Propeller 的链接器脚本重新排序的部分. 然而, 与 Propeller 不同的是, 它不需要基本块部分, 并且适用于任何部分粒度. 有效的粒度是函数节 (-ffunction-sections), 它几乎得到普遍支持, 并为重新排序优化提供了合理的基础. 此外, 它不需要对链接器命令进行任何更改, 而是通过使用环境变量透明地运行. 最后, 它可以透明地扩展到任意数量的包, 并且几乎不需要增量更改. 参见 phoronix 报道 [Intel's Newest Software Effort For Achieving Greater Performance: Thin Layout Optimizer](https://www.phoronix.com/news/Intel-Thin-Layout-Optimizer).

[github, intel/thin-layout-optimizer](https://github.com/intel/thin-layout-optimizer)

[thin-layout-optimizer/wiki](https://github.com/intel/thin-layout-optimizer/wiki)


## 13.3 Shrinking the kernel
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
| 2021/02/18 | NA | [gcc Stack variable initialization - 无需合入补丁, GCC 支持即可](https://lwn.net/Articles/870045) | gcc 也引入了 [Auto Initialize Automatic Variables](https://www.phoronix.com/scan.php?page=news_item&px=GCC-12-Auto-Var-Init) 通过 `-ftrivial-auto-var-init` 选项将未初始化的[变量默认初始化为 0](https://gcc.gnu.org/pipermail/gcc-patches/2021-February/565514.html). | RFC v2 ☑ 5.9-rc1 | NA |

## 13.6 Randomize Structure Layout
-------

### 13.6.1 GCC Plugin 的支持
-------

将来自 grsecurity 的[结构布局随机化(Randomizing structure layout)](http://xuxinting.cn/2020/12/20/2020-12-20-kernel-randomize-layout) 推送到 linux 主线. 这个特性通过在编译时将所选结构的布局随机化, 作为对需要知道内核中结构布局的攻击的概率防御. 这对于 "内部" 内核构建非常有用, 因为攻击者既不能使用随机种子也不能使用其他构建工件.

[commit 313dd1b62921 ("gcc-plugins: Add the randstruct plugin")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=313dd1b629219db50cad532dba6a3b3b22ffe622) 通过引入一个 gcc 插件 [`scripts/gcc-plugins/randomize_layout_plugin.c`](https://elixir.bootlin.com/linux/v4.13/source/scripts/gcc-plugins/randomize_layout_plugin.c) 来完成这项工作. 这个插件通过 [Fisher-Yates shuffle 算法](https://elixir.bootlin.com/linux/v4.13/source/scripts/gcc-plugins/randomize_layout_plugin.c#L256)完成了对指定结构体 (类似于数组) 的乱序.

>   Fisher–Yates shuffle 是对有限序列生成一个随机排列的算法, 所有的排列是等概率的, 该算法是无偏的、高效的, 算法的时间正比于乱序的数组.
>
>   参见 [Fisher–Yates shuffle 算法详解：给定数组的乱序](https://blog.csdn.net/qikaihuting/article/details/78224690)

这个功能的大部分都是从 grsecurity 移植过来的. 该实现几乎与 PaX 团队和 Brad Spengler 编写的原始代码完全相同. 这些变化是添加了改进的指定初始值设定项标记、白名单机制、许多假阳性修复, 以及处理任务结构随机化的不同方法.


该插件通过三种方式选择结构:

1.  手动使用新的 [` __randomize_layout`](https://elixir.bootlin.com/linux/v4.13/source/include/linux/compiler-gcc.h#L231) 进行标记;

2.  或者在发现结构完全由函数指针组成时由编译器自动进行标记(可以使用 [`__no_randomize_layout`](https://elixir.bootlin.com/linux/v4.13/source/include/linux/compiler-gcc.h#L232) 通知编译器不要进行随机化).

3.  通过 `randomized_struct_fields_{start|end}` 圈定一个范围, 结构体这个范围内所有的字段都会被乱序.

    task_struct 是一种在利用漏洞时特别敏感且经常被滥用的结构, 但由于某些字段需要在开头和结尾, 因此对其进行随机化需要一些特殊处理. 为了解决这个问题, 使用一个 [内部匿名结构](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=29e48ce87f1eaaa4b1fe3d9af90c586ac2d1fb74) 来标记将随机化的部分. 随机的这部分(匿名结构) 字段用 [randomized_struct_fields_start](https://elixir.bootlin.com/linux/v4.13/source/include/linux/sched.h#L534) 和 [randomized_struct_fields_end](https://elixir.bootlin.com/linux/v4.13/source/include/linux/sched.h#L1094) 来标记.

### 13.6.1 LLVM/CLANG 支持
-------


到目前为止, Linux 内核 Randomize Structure Layout 的支持还只支持 GCC, 以更高的安全性的名义随机化 Linux 内核的敏感结构. 随后 LLVM/Clang 编译器开始支持这一特性, 实现方式与 GCC 类似, 旨在作为编译时强化, 使攻击者更难从程序结构中检索数据. 可以通过 "-frandomize-layout-seed=" 或 "-frandomize-layout-seed-file=" 选项启用支持, 以提供确定性随机种子以允许可重现的构建. CLANG  支持的补丁已经合入 LLVM/Clang 15. 与此同时, Google 的 Kees Cook 也已经准备了 Linux 内核对 CLANG Randomize Structure Layout 的支持. 参见 phoronix 报道: [Clang 15 Lands Support To Randomize Structure Layout, Linux Prepares To Use It](https://www.phoronix.com/scan.php?page=news_item&px=Clang-Linux-RandStruct).

利用 Linux 内核的 Randomize Structure Layout 强化可能会引起一些性能影响, 但尝试将结构布局的随机化限制为缓存行大小的成员组以降低性能成本(尽管随机化减少了), 但构建时间也无法调整.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/10/21 | Michael Leibowitz <michael.leibowitz@intel.com> | [Add the randstruct gcc plugin](https://patchwork.kernel.org/project/kernel-hardening/patch/1477071466-19256-1-git-send-email-michael.leibowitz@intel.com) | 这个插件在编译时随机化某些结构的布局. 这引入了两个定义 `__randomize_layout` 和 `__no_randomize_layout`. 这两个选项用于通知编译器尝试随机化或不随机化有问题的结构. 这个特性是从 grsecurity 移植过来的. 实现与 PaX 团队和 Spender 编写的原始代码几乎相同. 为了简化集成, 当前只支持显式标记结构. 但是, 它保留了对 `__no_randomize_layout` 的支持. UAPI 检查也被保留. 要随机化的结构必须使用 C99 指定的初始化式形式. | v1 ☐ | [PatchWork](https://lkml.org/lkml/2017/5/26/558)<br>*-*-*-*-*-*-*-* <br>[PatchWork](https://patchwork.kernel.org/project/kernel-hardening/patch/1495829844-69341-20-git-send-email-keescook@chromium.org) |
| 2017/05/26 | Kees Cook <keescook@chromium.org> | [Introduce struct layout randomization plugin](https://lore.kernel.org/patchwork/cover/1471548) | 引入结构体布局随机化插件, 通过 CONFIG_GCC_PLUGIN_RANDSTRUCT 使能. | v2 ☑ 4.13-rc1 | [2017/04/06 PatchWork 16/18](https://patchwork.kernel.org/project/kernel-hardening/patch/1491513513-84351-17-git-send-email-keescook@chromium.org)<br>*-*-*-*-*-*-*-* <br>[2017/05/26 PatchWork v2,00/20](https://lkml.org/lkml/2017/5/26/558)<br>*-*-*-*-*-*-*-* <br>[2017/05/26 PatchWork RFC,v2,19/20](https://patchwork.kernel.org/project/kernel-hardening/patch/1495829844-69341-20-git-send-email-keescook@chromium.org)<br>*-*-*-*-*-*-*-* <br>[2017/06/19 PatchWork 1/4](https://patchwork.kernel.org/project/kernel-hardening/patch/1497905801-69164-2-git-send-email-keescook@chromium.org) |
| 2017/11/09 | Sandipan Das <sandipan@linux.vnet.ibm.com> | [compiler, clang: handle randomizable anonymous structs](https://lists.archive.carbon60.com/linux/kernel/2848189) | 为 clang 支持处理 randomized_struct_fields_start 和 randomized_struct_fields_end 标记 | v1 ☑ 4.15-rc1 | [CGIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4ca59b14e588f873795a11cdc77a25c686a29d2) |


## 13.7 Fast Kernel Headers
-------


Ingo 自 2020 年底就一直投入 "Fast Kernel Headers" 项目, 在 Linux 内核中, 在 `include` 和 `arch/*/include` 层次结构中, 大约有 10000 个主要的 .h 头文件. 在过去的 30 多年里, 他们已经成长为一套复杂而痛苦的交叉依赖, 被称为 "依赖地狱"(Dependency Hell'). 这个项目就是通过对 Linux 内核的头文件层次结构和依赖关系的全面重构, 彻底解决 "依赖地狱", 该项目有两个主要的的目标:

1.  加快内核构建速度 (包括绝对构建时间和增量构建时间)

2.  解耦子系统类型和 API 定义


参见 phoronix 的报道

v1: [Massive ~2.3k Patch Series Would Improve Linux Build Times 50~80% & Fix"Dependency Hell"](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Fast-Kernel-Headers)

v2: [Fast Kernel Headers v2 Posted - Speeds Up Clang-Built Linux Kernel Build By ~88%](https://www.phoronix.com/scan.php?page=news_item&px=Fast-Kernel-Headers-v2)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/01/02 | Ingo Molnar <mingo@kernel.org> | ["Fast Kernel Headers" Tree -v1: Eliminate the Linux kernel's"Dependency Hell"](https://lore.kernel.org/lkml/YdIfz+LMewetSaEB@gmail.com) | "Fast Kernel Headers" 的补丁集, 新增 config 配置, CONFIG_FAST_HEADERS 和 CONFIG_KALLSYMS_FAST.<br> 这个补丁集非常庞大, 包含了 2000+ 补丁, 这可能是内核有史以来代码量最大的一个功能. 但是效果也很不错.<br>1. 启用了 CONFIG_FAST_HEADERS 的内核每小时的内核构建次数可能比当前的库存内核多出 78%, 在支持的架构上, 绝对内核构建性能可以提高 50~80%. 将许多高级标头与其他标头分离, 取消不相关的函数, 类型和 API 标头的分离, 头文件的自动依赖关系处理以及各种其他更改.<br>2. CONFIG_KALLSYMS_FAST 则实现了一个基于 objtool 的未压缩符号表功能, 它避免了 vmlinux 对象文件的通常三重链接, 这是增量内核构建的主要瓶颈. 由于即使使用 distro 配置, kallsyms 表也只有几十 MB 大, 因此在内核开发人员的桌面系统上, 内存成本是可以接受的. 不过当前只在 x86_64 下实现了此功能.<br> 到目前为止, 这个庞大的补丁系列已经在 x86/x86_64, SPARC, MIPS 和 ARM64 上进行了测试. | v1 ☐ | [LORE RFC, 0000/2297](https://patchwork.kernel.org/project/kernel-hardening/patch/1495829844-69341-20-git-send-email-keescook@chromium.org)[LORE v2,](https://lore.kernel.org/lkml/Ydm7ReZWQPrbIugn@gmail.com), [LORE -v3, 0000/2300](https://lore.kernel.org/lkml/YjBr10JXLGHfEFfi@gmail.com) |

由于这组补丁如此庞大, 涉及的模块也如此多, 因此它不可能在短时间内合入, 因此 Ingo 决定先从调度入手, [Merge branch'sched/fast-headers'into sched/core](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/kernel/sched?id=ccacfe56d7ecdd2922256b87e9ea46f13bb03b55), 先行合并到 TIP 分支, 这些补丁将构建内核的调度程序部分所需的 CPU 时间减少了 60.9%. 挂钟时间下降了 3.9%. 参见 [Linux Scheduler Build Improvements From"Fast Kernel Headers"Queued, FKH v3 Posted](https://www.phoronix.com/scan.php?page=news_item&px=Sched-Core-Fast-Kernel-Headers). [补丁集](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=4ff8f2ca6ccd9e0cc5665d09f86d631b3ae3a14c) 于 v5.17-rc5 合入主线.



| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2024/01/31 | Max Kellermann <max.kellermann@ionos.com> | [Fast kernel headers: reduce header dependencies](https://lore.kernel.org/all/20240131145008.1345531-1-max.kellermann@ionos.com) | [Fast Kernel Headers Work Restarted For Linux To Ultimately Speed Up Build Times](https://www.phoronix.com/news/Fast-Kernel-Headers-2024) | v1 ☐☑✓ | [LORE v1,0/28](https://lore.kernel.org/all/20240131145008.1345531-1-max.kellermann@ionos.com) |



## 13.8 LINK
-------


### 13.8.1 Mold
-------

Mold 是目前 Unix 链接器的现代替代品, 已经达到了 1.0 版本. 由 LLVM lld 连接器的原创者编写, 通过提高并行性, Mold 的目标是比它的前辈快几倍.

2021 年 12 月 [Mold 1.0 发布](https://www.phoronix.com/scan.php?page=news_item&px=Mold-1.0-Released), 作为非常有前途的高性能链接器, 是当前主流编译器等(如 GNU 的 Gold 和 LLVM 的 LLD) 首选替代方案. 随即 GCC 12 宣布增加了[对 Mold 的支持](https://www.phoronix.com/scan.php?page=news_item&px=GCC-12-Mold-Linker). 紧接着 Mold 宣布 1.0.1 将[维护 1 年](https://www.phoronix.com/scan.php?page=news_item&px=Mold-1.0.1-Released), 成为事实上的 LTS 版本.

2022 年 6 月 Mold 1.3 继续对 LTO 进行了优化. 参见 phoronix 报道 [Mold 1.3 High Speed Linker Released With LTO Improvements](https://www.phoronix.com/scan.php?page=news_item&px=Mold-1.3-Released).

Mesa CI 开始使用 Mold 作为其 x86_64 和 AArch64 上的默认链接器, 从而提高其 CI 的工作效率, 最终也可能帮助他们解决以前不断膨胀的云 CI 成本. [Mesa CI Begins Making Use Of Mold Linker For"Substantial"Performance Improvement](https://www.phoronix.com/news/Mesa-CI-Begins-Mold).

[Mold 1.6 High Speed Linker Adds PPC64 and s390x, Smaller Output Files](https://www.phoronix.com/news/Mold-1.6-Linker).

[Mold 2.0 High Speed Linker Released: Moves From AGPL To MIT License](https://www.phoronix.com/news/Mold-2.0-Linker).

[Mold Linker Performance Remains Very Compelling In 2024 Over GNU Gold/ld, LLVM lld](https://www.phoronix.com/news/Mold-Linker-2024-Performance).


### 13.8.2 dynamic linking
-------

[A look at dynamic linking](https://lwn.net/Articles/961117)


## 13.9 Compiler Optimization
-------

随后 2022 年, 开发者 Miko 建议所有架构都开启 `-O3` 编译内核 [Experimental -O3 Optimizing The Linux Kernel For Better Performance Brought Up Again](https://www.phoronix.com/news/O3-Optimize-Kernel-2022-Patches), 但是遭到了 Linus 的强烈反对, [Linus Torvalds' Latest Commentary Against -O3'ing The Linux Kernel](https://www.phoronix.com/scan.php?page=news_item&px=Linus-Against-O3-Kernel), [LKML 回复](https://lore.kernel.org/lkml/CA+55aFz2sNBbZyg-_i8_Ldr2e8o9dfvdSfHHuRzVtP2VMAUWPg@mail.gmail.com).

随后 对内核使用 `-O3` 进行了较为详细的性能测试, 参照 phoronix 报道

[Benchmarking The Linux Kernel With An"-O3"Optimized Build](https://www.phoronix.com/review/linux-kernel-o3).

[Benchmarking The Linux 5.19 Kernel Built With"-O3 -march=native"](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.19-O3-March-Native)


随后 Intel 尝试为 ClearLinux 进行了 `-O3` 构建, [Ensure -O3 is default option for all archs in 5.19 kernel #2693](https://github.com/clearlinux/distribution/issues/2693), [use O3 (github #2693)](https://github.com/clearlinux-pkgs/linux/commit/71fbe0406686178c6a209a515c174df7ef77e4e4). 参见 [Intel's Clear Linux Taps -O3 For Its Kernel Builds](https://www.phoronix.com/news/Clear-Linux-O3-Kernel).

["CC_OPTIMIZE_FOR_PERFORMANCE_O3" Performance Tunable Dropped In Linux 6.0](https://www.phoronix.com/news/Linux-6.0-Drops-O3-Kconfig)

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/06/21 | Miko Larsson <mikoxyzzz@gmail.com> | [Kconfig: -O3 enablement](https://lore.kernel.org/all/20220621133526.29662-1-mikoxyzzz@gmail.com) | 允许所有架构支持 -O3 编译. | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20220621133526.29662-1-mikoxyzzz@gmail.com) |
| 2022/06/28 | Nick Desaulniers <ndesaulniers@google.com> | [kbuild: drop support for CONFIG_CC_OPTIMIZE_FOR_PERFORMANCE_O3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a6036a41bffba3d5007e377483b425d470ad8042) | 移除 CONFIG_CC_OPTIMIZE_FOR_PERFORMANCE_O3. | v1 ☑✓ 6.0-rc1 | [LORE](https://lore.kernel.org/all/20220628210407.3343118-1-ndesaulniers@google.com) |


Intel 编译器随后也切到 LLVM 框架, 参见 [Intel Fully Embracing LLVM For Their C/C++ Compilers](https://www.phoronix.com/news/Intel-LLVM-Adoption-C-CPP) 和 [Intel C/C++ compilers complete adoption of LLVM](https://www.intel.com/content/www/us/en/developer/articles/technical/adoption-of-llvm-complete-icx.html).

由于 ICC 编译 linux 缺乏维护, 随后内核有人提议删除 ICC 编译器编译内核的支持.

| 2022/10/17 | Masahiro Yamada <masahiroy@kernel.org> | [Remove Intel compiler support](https://lore.kernel.org/all/20221016182349.49308-1-masahiroy@kernel.org) | TODO | v3 ☐☑✓ | [LORE](https://lore.kernel.org/all/20221016182349.49308-1-masahiroy@kernel.org) |


## 13.10 编译选项
-------


### 13.10.1 -funsigned-char
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/10/19 | Jason A. Donenfeld <Jason@zx2c4.com> | [kbuild: treat char as always unsigned](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3bc753c06dd02a3517c9b498e3846ebfc94ac3ee) | [Linux 6.2 Looks To Enable"-funsigned-char"To Better Deal With Buggy Code](https://www.phoronix.com/news/Linux-6.2-funsigned-char) | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20221019203034.3795710-1-Jason@zx2c4.com) |

#### 13.10.2 arch 相关编译选项
-------

[graysky2/kernel_compiler_patch](https://github.com/graysky2/kernel_compiler_patch) 此补丁通过添加更多可在以下位置访问的微架构选项, 为内核构建添加了额外的优化 / 调优.

## 13.11 Reduce Memory Usage
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/12/27 | Thomas Weißschuh <linux@weissschuh.net> | [reduce maximum memory usage](https://lore.kernel.org/all/20221216-objtool-memory-v2-0-17968f85a464@weissschuh.net) | 使用 objtool 处理 vmlinux.o 是内核构建中最占用内存的步骤. 通过减少最大内存使用量, 我们可以减少整个内核构建的最大内存使用.<br> 因此, 在内核构建过程中, 内存不足的机器上的内存压力得到了缓解, 而且构建速度更快, 因为需要的交换更少. 参见 phoronix 报道 [New Patches Aim To Reduce Memory Use While Compiling The Linux Kernel](https://www.phoronix.com/news/Linux-Kernel-Build-Less-RAM). | v2 ☐☑✓ | [LORE v2,0/8](https://lore.kernel.org/all/20221216-objtool-memory-v2-0-17968f85a464@weissschuh.net) |


## 13.12 PIE
-------


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2023/04/28 | Hou Wenlong <houwenlong.hwl@antgroup.com> | [x86/pie: Make kernel image's virtual address flexible](https://lore.kernel.org/all/cover.1682673542.git.houwenlong.hwl@antgroup.com) | 这些补丁允许 x86_64 上将内核构建为位置独立可执行文件(PIE). PIE 内核可以被重新定位在虚拟地址空间的顶部 2G 之下. 这个补丁集提供了一个例子, 允许内核映像在地址空间的顶部 512G 中重新定位. PIE 内核的最终目的是提高内核的安全性, 以及内核映像的虚拟地址的可扩展性, 甚至可以在地址空间的下半部分. 内核可以容纳更多的位置, 这意味着攻击者可以更难猜测. 参见 phoronix 报道 [New Patches Aim To Tackle Linux x86_64 PIE Support](https://www.phoronix.com/news/Linux-x86_64-PIE-2023). | v1 ☐☑✓ | [LORE v1,0/43](https://lore.kernel.org/all/cover.1682673542.git.houwenlong.hwl@antgroup.com) |


## 13.13 Scope Guard
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2023/06/12 | Peter Zijlstra <peterz@infradead.org> | [Scope-based Resource Management](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=7170509cadbb76e5fa7d7b090d2cbdb93d56a2de) | [Scope-based resource management for the kernel](https://lwn.net/Articles/934679) 以及 [Scope-Based Resource Management Infrastructure Merged For Linux 6.5](https://www.phoronix.com/news/Linux-6.5-Scope-Resource-Manage). | v3 ☐☑✓ 6.6-rc1 | [LORE v3,0/57](https://lore.kernel.org/all/20230612090713.652690195@infradead.org) |


## 13.14 Compiler
-------


| 编号 | 应用 | 功能 |
|:---:|:----:|:---:|
| 1 | [Cling](https://github.com/vgvassilev/cling) | 用于这个基于 LLVM/Clang 构建的开源交互式 C++ 解释器. Cling 是作为 LLVM/Clang 的扩展实现的, 以用作利用读取 - 求值 - 打印循环 (REPL) 概念的中间人, 并依赖于实时 (JIT) 编译. [Cling 1.0 发布用于交互式 C++ 解释器](https://www.phoronix.com/news/Cling-1.0-Released) |



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
| 2021/08/12 | "Tzvetomir Stoyanov (VMware)" <tz.stoyanov@gmail.com> | [trace: Add kprobe on tracepoint](https://lore.kernel.org/patchwork/patch/1122097) | 引入了一种新的动态事件: 事件探测, 事件可以被附加到一个现有的 tracepoint 跟踪点, 并使用它的字段作为参数. 事件探测通过在 'dynamic_events' ftrace 文件中写入配置字符串来创建. | RFC,v5 ☑ 5.15-rc1 | [Patchwork](https://patchwork.kernel.org/project/linux-trace-devel/patch/20210812145805.2292326-1-tz.stoyanov@gmail.com)<br>*-*-*-*-*-*-*-* <br>[commit 7491e2c44278](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7491e2c442781a1860181adb5ab472a52075f393) |


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

OSNOISE & TimerLat Tracer 用于跟踪系统内部活动的噪音, 分析那些对业务应用可能造成性能干扰的因素. 是将 PREEMPT_RT 合入 mainline 的一部分工作.

### 14.5.1 OSNOISE & TimerLat Tracer
-------

在高性能计算 (HPC) 的上下文中, 操作系统噪声 (OSNOISE) 是指应用程序由于操作系统内部的活动而遭受的干扰. 在 Linux 的上下文中, NMI、IRQ、softirqs 和任何其他系统线程都可能对应用程序造成噪音. 此外, 与硬件相关的作业也会产生噪音, 例如, 通过 SMIs.

关心操作系统窃取的每一微秒的 HPC 用户和开发人员不仅需要一种精确的方法来测量 OSNOISE, 而且主要是找出谁在窃取 CPU 时间, 以便他们可以追求系统的完美调谐. 因此为 linux 内核设计了 [OSNOISE Tracer](https://docs.kernel.org/trace/OSNOISE-tracer.html) 来分析和发现干扰源. OSNOISE Tracer 运行内核内循环, 测量有多少时间可用. 它通过启用抢占, softirq 和 IRQ 来做到这一点, 从而在其执行期间允许所有 OSNOISE 源.

渗透噪声示踪剂会记录任何干扰源的进入点和退出点. 当噪声发生时, 操作系统级别没有任何干扰, Tracer 可以安全地指向与硬件相关的噪声. 通过这种方式, OSNOISE 可以解释任何干扰源. OSNOISE Tracer 还添加了新的内核跟踪点, 这些跟踪点可辅助用户以精确直观的方式指向噪声的罪魁祸首.

在周期结束时, 渗透噪声 Tracer 打印所有噪声的总和、最大单个噪声、线程可用的 CPU 百分比以及噪声源的计数器, 用作基准测试工具.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/22 | Daniel Bristot de Oliveira <bristot@redhat.com> | [hwlat improvements and osnoise/timerlat tracers](https://patchwork.kernel.org/project/linux-trace-devel/patch/162936876189.187130.17558311387542061930.stgit@devnote2) | NA | v5 ☑ 5.14-rc1 | [Patchwork v5,09/14](https://lore.kernel.org/all/e649467042d60e7b62714c9c6751a56299d15119.1624372313.git.bristot@redhat.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bce29ac9ce0bb0b0b146b687ab978378c21e9078) |
| 2021/11/29 | Daniel Bristot de Oliveira <bristot@kernel.org> | [osnoise: Support multiple instances (for RTLA)](https://patchwork.kernel.org/project/linux-trace-devel/patch/162936876189.187130.17558311387542061930.stgit@devnote2) | 为 osnoise/timerlat 跟踪器启用多个实例. 目前, osnoise 和 timerlat 仅在单个实例上运行. 为了减少这种限制, 本系列增加了对同一跟踪程序的并行实例的支持. 也就是说, 可以使用不同的跟踪配置运行 osnoise tracer 的两个实例. 例如, 一个仅用于跟踪器输出, 另一个用于跟踪器和一组跟踪点. 这个补丁集是 RTLA 的内核依赖项. 此修补程序集与 [RTLA](https://patchwork.kernel.org/project/linux-trace-devel/cover/cover.1638182284.git.bristot@kernel.org) 一起发送, 但我们将内核和用户空间修补程序集分开. | v9 ☑ 5.16-rc1 | [Patchwork V9,0/9](https://patchwork.kernel.org/project/linux-trace-devel/cover/cover.1635702894.git.bristot@kernel.org), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2fac8d6486d5c34e2ec7028580142b8209da3f92) |

### 14.5.2 New Real-Time Linux Analysis (RTLA) tool
-------

在 OSNOISE & TimerLat Tracer 合入后, 开发者又向 linux 社区提交了一个用于实时性 (噪音) 分析的工具集(RTLA/Real-Time Linux Analysis), 它包含一组旨在分析 Linux 实时性的工具(命令). 它并没有将 Linux 作为一个黑盒进行测试, 而是利用内核 OSNOISE & TimerLat Tracer 提供关于意外结果的属性和根源的精确信息, 使用户和开发人员更容易收集性能和跟踪数据, 并帮助微调他们的系统或者算法.


参见作者 [Daniel's 关于 RTLA 的 Blog](https://bristot.me/and-now-linux-has-a-real-time-linux-analysis-rtla-tool), RTLA 可以理解为 OSNOISE & TimerLat TRACER 的用户界面, 提供了一个直观的界面来使用和处理数据. 对于那些不熟悉 PREEMPT_RT, 但是必须却想要评估内核实时性的开发人员来说, 这也是很有帮助的.


#### 14.5.2.1 timerlat tool
-------

比如通过如下命令:

```cpp
rtla timerlat top-p f: 95-t 150-t trace_output.txt
```

用户可以得到高优先级任务所经历的延迟的信息(-pf: 95), 如果延迟高于 150us (-t 150), 并且将带有调试信息的跟踪日志保存在 trace_output.txt 中.

其他评估调度延迟的方法, 可以参照 [Demystifying the Real-Time Linux Scheduling Latency](https://bristot.me/demystifying-the-real-time-linux-latency).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/12/10 | Daniel Bristot de Oliveira <bristot@kernel.org> | [RTLA: An interface for osnoise/timerlat tracers](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=5dce5904e3b9bc11d9635dd3458c7ff32caebb59) | [rtla](https://github.com/bristot/rtsl) 是一个旨在分析 Linux 实时属性的工具集. rtla 并没有将 Linux 作为一个黑盒来测试, 而是利用内核跟踪功能来提供关于属性和意外结果的根本原因的精确信息. 首先, 它提供了一个 osnoise 和 timerlat 示踪器的接口. 在未来, 它还将作为 rtsl 和其他延迟 / 噪声跟踪器的基础. V6 版本前五个补丁是 [osnoise: Support multiple instances](https://lore.kernel.org/lkml/cover.1628775552.git.bristot@kernel.org) 的重新发送, 它为 osnoise/timerlat 跟踪器启用多个实例. 接下来的七个补丁是 rtla, rtla osnoise 和 rtla timerlat. 后面的版本都将内核态和用户态拆开来提交. 参见 [Linux 5.17 Picks Up A Real-Time Analysis Tool](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-RTLA) | v5 ☐ 5.17-rc1 | [2021/10/27 Patchwork v6,00/20](https://lore.kernel.org/lkml/cover.1635284863.git.bristot@kernel.org)<br>*-*-*-*-*-*-*-* <br>[2021/11/29 Patchwork v8,00/14](https://patchwork.kernel.org/project/linux-trace-devel/cover/cover.1638182284.git.bristot@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v9,00/14](https://lore.kernel.org/all/cover.1639158831.git.bristot@kernel.org) |


https://patchwork.kernel.org/project/linux-trace-devel/list/?submitter=200911&state=*&archive=both&param=1&page=2

[[for-next,01/14] tracing/OSNOISE: Do not follow tracing_cpumask](https://patchwork.kernel.org/project/linux-trace-devel/patch/20211102201156.678148671@goodmis.org/)

#### 14.5.2.2 hwnoise tool
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2023/01/10 | Daniel Bristot de Oliveira <bristot@kernel.org> | [rtla: Add hwnoise tool](https://lore.kernel.org/all/cover.1673380089.git.bristot@kernel.org) | 参见 phoronix 报道 [Linux 6.3 Introducing Hardware Noise"hwnoise"Tool](https://www.phoronix.com/news/Linux-6.3-hwnoise). | v1 ☐☑✓ | [LORE v1,0/6](https://lore.kernel.org/all/cover.1673380089.git.bristot@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/6](https://lore.kernel.org/lkml/cover.1675181734.git.bristot@kernel.org) |



## 14.6 MMIO register read/write tracing
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/09 | Sai Prakash Ranjan <quic_saipraka@quicinc.com> | [tracing/rwmmio/arm64: Add support to trace register reads/writes](https://patchwork.kernel.org/project/linux-arm-kernel/cover/cover.1636452784.git.quic_saipraka@quicinc.com) | MMIO register read/write tracing. | v3 ☐ | [Patchwork v3,0/3](https://patchwork.kernel.org/project/linux-arm-kernel/cover/cover.1636452784.git.quic_saipraka@quicinc.com) |

## 14.7 ftrace_direct
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/11/8 | Steven Rostedt | [ftrace: Add register_ftrace_direct()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a3ad1a7e39689005cb04a4f2adb82f9d55b4724f) | 1615872606-56087-1-git-send-email-aubrey.li@intel.com | v1 ☑✓ 5.5-rc1 | [LORE 00/10](https://lkml.org/lkml/headers/2019/11/8/1675) |
| 2019/08/27 | Peter Zijlstra <peterz@infradead.org> | [Rewrite x86/ftrace to use text_poke()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=768ae4406a5cab7e8702550f2446dbeb377b798d) | 615872606-56087-1-git-send-email-aubrey.li@intel.com | v3 ☑✓ 5.6-rc1 | [LORE v3,0/3](https://lore.kernel.org/all/20190827180622.159326993@infradead.org) |
| 2021/06/05 | Jiri Olsa <jolsa@kernel.org> | [x86/ftrace/bpf: Add batch support for direct/tracing attach](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a3ad1a7e39689005cb04a4f2adb82f9d55b4724f) | 1615872606-56087-1-git-send-email-aubrey.li@intel.com | RRC,v3 ☐ | [LORE RRC,v3,00/19](https://lkml.kernel.org/netdev/20210605111034.1810858-1-jolsa@kernel.org) |
| 2021/10/8 | Jiri Olsa <jolsa@kernel.org> | [x86/ftrace: Add direct batch interface](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=5fae941b9a6f95773df644e7cf304bf199707876) | 1615872606-56087-1-git-send-email-aubrey.li@intel.com | v2 ☑✓ 5.16-rc1 | [LKML v2,0/8](https://lkml.org/lkml/2021/10/8/186), [LORE v2,0/8](https://lore.kernel.org/all/20211008091336.33616-1-jolsa@kernel.org) |


## 14.8 uprobe
-------

### 14.8.1 user events
-------

User Events 于 [5.18-rc1](https://kernelnewbies.org/Linux_5.18#User_events) 合入主线, 参见 Document [user_events: User-based Event Tracing](https://www.kernel.org/doc/html/latest/trace/user_events.html) 以及 example [`samples/user_events/example.c`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/samples/user_events/example.c?id=c57eb4781509)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/01/18 | Beau Belgrave <beaub@linux.microsoft.com> | [user_events: Enable user processes to create and write to trace events](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=864ea0e10cc90416a01b46f0d47a6f26dc020820) | Linux 已经有一种方法允许使用内核跟踪工具 (uprobes) 跟踪用户空间进程. 此版本添加了一个新的 ABI, 允许进程创建和写入与内核跟踪级别事件隔离的跟踪事件. 这样可以更快地跟踪用户模式数据, 并打开托管代码以参与跟踪事件, 其中存根位置为 dynamic.<br> 进程通常只希望仅在有用时才进行跟踪. 进程可以将描述事件格式的事件注册到内核. 内核将创建事件. 然后, 该进程将从映射的 tracefs 文件中接收页面映射中的一个字节, 它可以检查该文件. 特权任务可以启用该事件, 这会将映射的字节更改为 true. 然后, 进程可以开始将事件写入跟踪 buffer. 参见 [User events — but not quite yet](https://lwn.net/Articles/889607). | v10 ☑✓ [5.18-rc1](https://kernelnewbies.org/Linux_5.18#User_events) | [LORE v10,0/12](https://lore.kernel.org/all/20220118204326.2169-1-beaub@linux.microsoft.com) |
| 2023/03/28 | Beau Belgrave <beaub@linux.microsoft.com> | [tracing/user_events: Remote write ABI](https://lore.kernel.org/all/20230328235219.203-1-beaub@linux.microsoft.com) | 之前的 API 使用起来并不方便, 因此进行了重构. 参见 [User trace events, one year later](https://lwn.net/Articles/927595). 当前正在使用一个需要 mmap() 的共享页面. 删除共享页面实现并移动到用户注册地址实现. 在这个新模型中, 从用户程序注册事件期间指定了 3 个新值. 第一个是启用或禁用事件时要更新的地址. 第二个是要设置 / 清除的位, 以反映正在启用的事件. 第三个是指定地址的值的大小. 这允许在用户程序中使用本地 32/64 位值来支持内核跟踪程序和用户跟踪程序. 例如, 当事件启用时, 为内核跟踪程序设置第 31 位允许用户跟踪程序使用其他位作为引用计数或其他标志. 内核端原子地更新位, 用户程序也需要原子地更新这些值. 用户提供的地址必须在自然边界上对齐, 这允许单页检查, 并防止奇怪的行为, 如跨 2 页而不是单页的启用值. 当遇到页面错误时, 它们将通过工作队列异步完成. 如果页面错误返回, 则再次尝试写更新. 如果页面不能故障进入, 那么我们将登录并等待下一次事件被启用 / 禁用. 这是为了防止由于不良用户进程在注册地址后取消映射或更改保护值而导致的无限循环. | v10 ☐☑✓ | [LORE v10,0/12](https://lore.kernel.org/all/20230328235219.203-1-beaub@linux.microsoft.com)|



# 15 kptr_restrict
-------


# 16 Generic Kernel Image(GKI)
-------


谷歌的 Android 因其移动操作系统以及各种供应商 / 设备内核树所携带的所有下游补丁而臭名昭著, 而近年来, 更多的代码已经上游. 谷歌也一直在转向 Android 通用内核映像 (GKI) 作为其所有产品内核的基础, 以进一步减少碎片化. 展望未来, 谷歌在 2021 年北美开源峰会展示了一种 ["上游优先"(Moving Google toward the mainline)](https://lwn.net/Articles/871195) 的方法, 以推动新的内核功能. 追求 "新功能的上游优先开发模型", 以确保新代码首先进入主线 Linux 内核, 而不是直接在 Android 源代码树中停留.

谷歌的 Todd Kjos 随后在 Linux Plumbers Conference(LPC2021) 上谈到了他们的 [通用内核映像(Generic Kernel Image, GKI) 计划](https://linuxplumbersconf.org/event/11/contributions/1046). 通过 Android 12 和他们基于 Linux 5.10 的 GKI 映像, 进一步减少了碎片化, 以至于 "几乎被消除". 在 Android 12 GKI 中, 大多数供应商 / OEM 内核功能现在要么被上游到 Linux 内核中, 要么被隔离到供应商模块或者钩子中, 要么合并到 Android Common Kernel 中.

LWN 上也对此进行了[汇总报道](https://lwn.net/Kernel/Index/#Android-Generic_kernel_image)

[Google Finally Shifting To"Upstream First"Linux Kernel Approach For Android Features](https://www.phoronix.com/scan.php?page=news_item&px=Android-Linux-Upstream-First)

[Android to take an “upstream first” development model for the Linux kernel](https://arstechnica.com/gadgets/2021/09/android-to-take-an-upstream-first-development-model-for-the-linux-kernel)

[ANDROID GTI](https://stackoverflow.com/questions/65415511/android-kernel-build-flow-with-gki-introduced-from-android-11)

[LPC 2021-Generic Kernel Image](https://linuxplumbersconf.org/event/11/contributions/1046/attachments/824/1557/2021%20LPC%20GKI.pdf)

目前的计划:

1.  2020-2022: Accumulating ecosystem technical debt in Android Common Kernels (android12-5.10 and android-mainline), 汇总技术债务

2.  2023-2024: 减少技术债务(Reducing Technical Debt)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/04/24 | Todd Kjos <tkjos@google.com> | [ANDROID: add support for vendor hooks](https://lore.kernel.org/patchwork/patch/1394812) | 启动阶段异步解压 initramfs. 可以加速系统启动. | v1 ☑ [5.13-rc1](https://kernelnewbies.org/Linux_5.13) | [Patchwork 00/28](https://github.com/aosp-mirror/kernel_common/commit/67e0a3df19970176f093ff8be72f201d8c76ae81) |



# 17 众核
-------

## 17.1 启动加速
-------

Fedora 尝试优化 systemd 开机以及重启的时间, 参见 phoronix 报道 [Fedora 38 Wants To Make Sure Shutdowns & Reboots Are Faster](https://www.phoronix.com/news/Fedora-38-Faster-Reboots) 以及 [Fedora wiki--Changes/Shorter Shutdown Timer](https://fedoraproject.org/wiki/Changes/Shorter_Shutdown_Timer).

### 17.1.1 启动优化
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/03/21 | Rasmus Villemoes <linux@rasmusvillemoes.dk> | [background initramfs unpacking, and CONFIG_MODPROBE_PATH](https://lore.kernel.org/patchwork/patch/1394812) | 启动阶段异步解压 initramfs. 可以加速系统启动. | v1 ☑ [5.13-rc1](https://kernelnewbies.org/Linux_5.13) | [Patchwork](https://lore.kernel.org/patchwork/patch/1394812) |
| 2022/11/02 | Stuart Hayes <stuart.w.hayes@gmail.com> | [cpufreq: acpi: Defer setting boost MSRs](https://lore.kernel.org/all/20221102195957.82871-1-stuart.w.hayes@gmail.com) | [Deferred Enabling Of ACPI CPUFreq Boost Support Can Help Boot Times For Large Servers](https://www.phoronix.com/news/CPUFreq-Defer-Boost-MSRs) | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20221102195957.82871-1-stuart.w.hayes@gmail.com) |
| 2024/02/22 | Gang Li <gang.li@linux.dev> | [hugetlb: parallelize hugetlb page init on boot](https://lore.kernel.org/all/20240222140422.393911-1-gang.li@linux.dev) | 用于在启动时并行化 HugeTLB 页面初始化. 对于具有大量 HugeTLB 页面的系统, 这可以大大缩短启动时间. 参见 phoronix 报道 [Linux 6.9 Will Boot Much Faster For Systems With Large Amounts Of RAM](https://www.phoronix.com/news/Linux-6.9-RAM-HugeTLB-Boot-Fast). | v6 ☐☑✓ | [LORE v6,0/8](https://lore.kernel.org/all/20240222140422.393911-1-gang.li@linux.dev) |
| 2024/04/04 | Ryan Roberts <ryan.roberts@arm.com> | [Speed up boot with faster linear map creation](https://lore.kernel.org/all/20240404143308.2224141-1-ryan.roberts@arm.com) | 创建线性映射可能会占用总启动时间的很大一部分, 尤其是当 rodata=full 时. 大部分时间都花在等待多余的 tlb 失效和内存障碍上. 本系列重新编写内核 pgtable 生成代码, 以显著减少 TLBI、ISB 和 DSB 的数量. | v2 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20240326101448.3453626-1-ryan.roberts@arm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/4](https://lore.kernel.org/all/20240404143308.2224141-1-ryan.roberts@arm.com) |


### 17.1.2 并行 CPU BringUp
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/21 | David Woodhouse <dwmw2@infradead.org> | [Parallel CPU bringup for x86_64](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0c7ffa32dbd6b09a87fea4ad1de8b27145dfd9a6) | 随着核数的增多, 内核的启动速度越来越慢. 这组补丁能够并行启动辅助 (x86_64) CPU 内核. 对 v6 测 hi 发现可以显著改善 Sapphire Rapids CPU 系统的启动时间, 从 71s 降低到 14s. 随后 v7 增加了对 AMD CPU 的支持. 参见 [Parallel CPU Bring-Up Poised For Linux 6.5](https://www.phoronix.com/news/Parallel-CPU-Bringup-TIP-Linux). [Parallel CPU Bringup Lands For Linux 6.5 To Shorten Boot/Reboot Time On Large Servers](https://www.phoronix.com/news/Parallel-CPU-Bringup-Linux-6.5). | v1 ☐ | [LWN](https://lwn.net/Articles/878161), [LKML](https://lkml.org/lkml/2021/12/9/664), [LORE 00/11](https://lkml.kernel.org/lkml/20211209150938.3518-1-dwmw2@infradead.org),  [Phoronix 报道 v1](https://www.phoronix.com/news/Linux-x86_64-Parallel-CPU-Boot)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/9](https://lore.kernel.org/lkml/20211215145633.5238-1-dwmw2@infradead.org), [Phoronix 报道 v3](https://www.phoronix.com/news/Parallel-CPU-Bringup-AMD-Snag)<br>*-*-*-*-*-*-*-* <br>[LORE v6](https://lore.kernel.org/lkml/20230202215625.3248306-1-usama.arif@bytedance.com), [Phoronix 报道 v6](https://www.phoronix.com/news/Linux-CPU-Parallel-Bringup-2023)<br>*-*-*-*-*-*-*-* <br>[LORE v7](20230207230436.2690891-1-usama.arif@bytedance.com)<br>*-*-*-*-*-*-*-* <br>[LORE v17,0/8](https://lore.kernel.org/lkml/20230328195758.1049469-1-usama.arif@bytedance.com) |
| 2023/04/15 | Thomas Gleixner <tglx@linutronix.de> | [cpu/hotplug, x86: Reworked parallel CPU bringup](https://lore.kernel.org/all/20230414225551.858160935@linutronix.de) | [Reworked x86_64 Parallel Boot Support Posted For The Linux Kernel](https://www.phoronix.com/news/Linux-Parallel-Boot-x86-Rework) | v1 ☐☑✓ | [LORE v1,0/37](https://lore.kernel.org/all/20230414225551.858160935@linutronix.de) |



# 18 LIB 与 数据结构
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


## 18.3 Rosebush
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2024/02/22 | Matthew Wilcox (Oracle) <willy@infradead.org> | [Rosebush, a new hash table](https://lore.kernel.org/all/20240222203726.1101861-1-willy@infradead.org) | Rosebush 被描述为内核的调整大小、可扩展、缓存感知的 RCU 优化哈希表. Rosebush 适合替换哈希表, 同时开销低于枫树或哈希表. 但它不是枫树的替代品, 因为它不支持范围. Rosebush 的另一个优势是具有每个存储桶的锁, 因此它对于写入密集型工作负载更具可扩展性. [Rosebush Proposed As A New Data Structure For The Linux Kernel](https://www.phoronix.com/news/Rosebush-Linux-Proposal). | v1 ☐☑✓ | [LORE v1,0/1](https://lore.kernel.org/all/20240222203726.1101861-1-willy@infradead.org) |



# 19 启动加速
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/03/21 | Rasmus Villemoes <linux@rasmusvillemoes.dk> | [background initramfs unpacking, and CONFIG_MODPROBE_PATH](https://lore.kernel.org/patchwork/patch/1394812) | 启动阶段异步解压 initramfs. 可以加速系统启动. | v1 ☑ [5.13-rc1](https://kernelnewbies.org/Linux_5.13) | [Patchwork](https://lore.kernel.org/patchwork/patch/1394812) |

# 20 形式化验证
-------

[Runtime Verification - 技术调查](https://zhuanlan.zhihu.com/p/434664665)

[Runtime Verification - 技术调查 2](https://zhuanlan.zhihu.com/p/468369055)

[Runtime Verification-Linux 内核验证(调查 3)](https://zhuanlan.zhihu.com/p/439660334)


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/06/16 | Daniel Bristot de Oliveira <bristot@kernel.org> | [The Runtime Verification (RV) interface](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e88043c0ac16f19960048372dcffc6df7c05c5b8) | 运行时验证 Linux 内核的行为. 运行时验证(RV) 是一种轻量级 (但严格) 的方法, 它补充了经典的穷举验证技术 (如模型检查和定理证明), 并为复杂系统提供了一种更实用的方法. RV 不依赖于系统的细粒度模型 (例如, 重新实现指令级别), 而是通过分析系统实际执行的轨迹, 并将其与系统行为的形式化规范进行比较来工作. RV 使用确定性自动机是一种成熟的方法. | v4 ☐☑✓ 6.0-rc1 | [LORE v4,0/20](https://lore.kernel.org/all/cover.1655368610.git.bristot@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v9,00/16](https://lore.kernel.org/all/cover.1659052063.git.bristot@kernel.org) |
| 2022/11/11 | Daniel Bristot de Oliveira <bristot@kernel.org> | [verification/rv: Add rv tool](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=afc70ccb962861e068e04c6089827493f5160a0a) | (用户空间)运行时验证工具 rv. 该工具旨在成为内核 rv 监视器的接口, 以及用户空间控制监视器. 该工具接收命令作为第一个参数 <br>1. list 列出所有可用的监视器 <br>2. mon 运行给定的监视器 <br> 每个监视器都是工具内的一个独立软件, 可以有自己的参数. | v2 ☐☑✓ 6.2-rc1 | [LORE v2,0/3](https://lore.kernel.org/all/cover.1668180100.git.bristot@kernel.org) |


# 21 新语言支持
-------

## 21.1 RUST 支持
-------

[Arm Helping With AArch64 Rust Linux Kernel Enablement](https://www.phoronix.com/news/AArch64-Rust-Linux-Kernel)


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/09/27 | Miguel Ojeda <ojeda@kernel.org> | [Rust support](https://lore.kernel.org/all/20220927131518.30000-1-ojeda@kernel.org) | TODO| v10 ☐☑✓ | [LORE 00/13](https://lore.kernel.org/all/20210414184604.23473-1-ojeda@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v10,0/27](https://lore.kernel.org/all/20220927131518.30000-1-ojeda@kernel.org) |
| 2024/02/27 | Daniel Almeida <daniel.almeida@collabora.com> | [Rewrite the VP9 codec library in Rust](https://lore.kernel.org/all/20240227215146.46487-1-daniel.almeida@collabora.com) | Collabora 的 Daniel Almeida 发布了 Linux 内核的 Video 4 Linux 2 (V4L2) 子系统中 VP9 编解码器库代码的重写. 在使用 Rust 而不是现有的 C 代码时, 这应该会产生更好的内存安全性, 并更好地抵御现有代码中的潜在问题. 参见 phoronix 报道 [Linux's V4L2 VP9 Codec Kernel Code Rewritten In Rust For Better Memory Safety](https://www.phoronix.com/news/VP9-Linux-Kernel-Rust-V4L2-RFC). | v1 ☐☑✓ | [LORE v1,0/1](https://lore.kernel.org/all/20240227215146.46487-1-daniel.almeida@collabora.com) |
| 2024/03/27 | Wedson Almeida Filho <wedsonaf@gmail.com> | [In-place module initialisation](https://lore.kernel.org/all/20240327032337.188938-1-wedsonaf@gmail.com) | [Microsoft Engineer Sends Rust Linux Kernel Patches For In-Place Module Initialization](https://www.phoronix.com/news/Linux-Rust-In-Place-Module-Init) | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20240327032337.188938-1-wedsonaf@gmail.com) |
| 2024/03/22 | Boqun Feng <boqun.feng@gmail.com> | [Memory model and atomic API in Rust](https://lore.kernel.org/all/20240322233838.868874-1-boqun.feng@gmail.com) | [A memory model for Rust code in the kernel](https://lwn.net/Articles/967049). | v1 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20240322233838.868874-1-boqun.feng@gmail.com) |
| 2024/05/14 | Wedson Almeida Filho <wedsonaf@gmail.com> | [Rust abstractions for VFS](https://lore.kernel.org/all/20240514131711.379322-1-wedsonaf@gmail.com) | 参见 phoronix 报道 [Microsoft Engineer Ports EXT2 File-System Driver To Rust](https://www.phoronix.com/news/Rust-VFS-Linux-V2-Now-With-EXT2). | v2 ☐☑✓ | [LORE v2,0/30](https://lore.kernel.org/all/20240514131711.379322-1-wedsonaf@gmail.com) |
| 2024/05/20 | Danilo Krummrich <dakr@redhat.com> | [DRM Rust abstractions and Nova](https://lore.kernel.org/all/20240520172059.181256-1-dakr@redhat.com) | [RFC Patches Posted For Rust-Written NVIDIA "Nova" GPU Driver](https://www.phoronix.com/news/RFC-Rust-Nova-NVIDIA-Driver). | v1 ☐☑✓ | [LORE v1,0/8](https://lore.kernel.org/all/20240520172059.181256-1-dakr@redhat.com) |


## 22.2 C++
-------

[A 2024 Discussion Whether To Convert The Linux Kernel From C To Modern C++](https://www.phoronix.com/news/CPP-Linux-Kernel-2024-Discuss)


# 22 kallsyms
-------

[Linux 6.2 Speeds Up A Function By 715x - kallsyms_lookup_name()](https://www.phoronix.com/news/Linux-6.2-Modules)

# 23 Auto Tune
-------


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/09/27 | Oracle | [bpftune For BPF-Based](https://lore.kernel.org/all/20220927131518.30000-1-ojeda@kernel.org) | [Oracle Developing"bpftune"For BPF-Based, Automatic Tuning Of Linux Systems](https://www.phoronix.com/news/Oracle-bpftune)<br>*-*-*-*-*-*-*-* <br>[https://blogs.oracle.com/linux/post/introducing-bpftune](https://blogs.oracle.com/linux/post/introducing-bpftune)<br>*-*-*-*-*-*-*-* <br>[bpftune - Using Reinforcement Learning in BPF](https://blogs.oracle.com/linux/post/bpftune-using-reinforcement-learning-in-bpf) | v10 ☐☑✓ | [GitHub](https://github.com/oracle-samples/bpftune) |
| 2022/09/27 | Atune | NA | NA | NA | NA |
| 2022/09/27 | Ktune | NA | NA | NA | NA |

# 24 ELF
-------

[When ELF notes reveal too much](https://lwn.net/Articles/962782)

[A look at dynamic linking](https://lwn.net/Articles/961117/)


# X 学习参考
-------

## X.1 业界论文
-------

| 论文 | 描述 |
|:---:|:----:|
| [Hubble: Performance Debugging with In-Production, Just-In-Time Method Tracing on Android](https://www.usenix.org/system/files/osdi22-luo.pdf) | Hubble 是一个 method-tracing 系统, 在所有支持的和即将由华为制造的安卓设备上运行, 以帮助调试性能问题. Hubble instruments every non-inlined bytecode method’s entry and exit, 以记录 method 的名称和时间戳. trace points 被记录到一个内存中的环形缓冲器中, 旧的数据会不断被覆盖. 这些数据只有在检测到性能问题时才会被保存下来, 使工程师在检测到异常情况之前就能及时获得宝贵的、详细的运行时间数据. 参见 [datawine - 论文阅读](https://datawine.github.io/paper-reading.html), 参见 [【欧拉多咖 | OS 每周快讯】2022.12.06~2022.12.12](https://www.chaspark.com/#/hotspots/821172234535870464) |

<br>

*   本作品 / 博文 ([AderStep - 紫夜阑珊 - 青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a><a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议</a> 进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚 gatieme](http://blog.csdn.net/gatieme) (包含链接: http://blog.csdn.net/gatieme), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   ** 转载请务必注明出处, 谢谢, 不胜感激 **
<br>
