2   **LIVE_PATCH 内核热补丁**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***


下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***


# 1 热补丁实现方案
-------

## 1.1 热补丁方案需要解决的问题
-------

1.  函数热替换, 热补丁要完成的最本质的工作, 就是不重启而更新执行的函数, 怎么完成函数的替换. 修改函数的前几条指令, 将指令修改为跳转到新函数的指令可以完成这样的工作. 那有没有其他方法呢 ?

2.  重定位问题, 正常来说驱动只能使用内核或者驱动所 EXPORT_SYMBOL 的符号, 这类符号在驱动插入的过程中, 进行重定位, 将符号指向了所引用符号的实际地址. 而热补丁则要求更宽松一些, 它需要能够直接访问被 patched 的函数中的为导出的符号信息, 这些符号正常内核 load_module 是不会进行重定位的. 只能热补丁自己来完成重定位. 这些符号的信息需要热补丁自己去甄别和重定位.

3.  一致性问题, 内核不重启而执行新函数, 那么在系统运行过程中, 势必存在一个时间点, 之前执行了旧函数, 之后将执行新函数. 那么这个时间点怎么确认, 即怎么的模型下, 执行函数的热替换操作, 那么可以认为内核的前后是一致的, 不会出现逻辑错误, 更不会引入安全问题. 更重要的是, 如果多个被 patched 的函数之间有逻辑依赖, 一致性怎么保证. 举例来说, 假设两个函数一个执行 A++, 一个执行 A--, 每次必然成对执行. 最终 A 计数将能够正常归 0. 但是如果 patched 之后, 变成 A+=2, A-=2, 那么怎么保证不会出现, A++, A-=2 或者 A+=2, A-- 之类的执行序列.


## 1.2 已知热补丁方案
-------

在主线内核实现热补丁特性之前, 各个厂商就在社区进行了激烈的讨论, 并各自形成了自己的一套方案.

| 方案 | 作者 | 实现 | 一致性模型 | 限制与约束 |
|:---:|:----:|:---:|:--------:|:---------:|
| KGraft | SUSE 的 Jiri Kosina(JK) 和 Jiri Slaby(JS) 合作开发 | 基于 ftrace 方案实现, 采用类 RCU 更新机制 | 只要不会出现同一个 universe 里既调用了旧函数又调用了新函数的情况, 那么就是一致的<br>对于用户进程, 同一次系统调用是一次 universe<br>对于内核线程, 每次被唤醒, 判断是否被 stop 条件之后算是一个新的universe<br>对于中断, 同一次中断处理算是一次 universe | 仅支持 X86_64/s390, 未公开自动化热补丁制作工具 |
| Kpatch | REDHAT | 基于 ftrace 方案, 使用 stop_machine + 栈检查来保证一致性 | 使用 stop_machine 机制停下所有 CPU, 然后对所有的进程进行栈检查, 只有在没有任何进程执行被 patched 的函数的情况下, 才认为是安全的 | stop_machine 机制对业务有中断影响, 大概带来 1ms-40ms 的延迟<br>仅支持 X86_64 |
| kpatch without stop_machine | HITACHI 的 Masami Hiramatsu | 基于 kpatch 方案, 取消了 stop_machine一致性检查 | 不再需要 stop_machine, 引入全局的院子计数器 refcounter, 跟踪目标函数的调用情况, 只有在目标函数没有被执行时(refcounter == 0), 才可以安全的进行替换 | 基于 kpatch<br>一致性检查存在安全问题 |
| LIVEPATCH Without Ftrace | HUAWEI | 基于直接跳转实现 livepatch, 不再基于 ftrace, 通过修改函数的前几条指令, 直接跳转到新函数 | 同 kpatch 类似, 采用 stop_machine + 栈检查来保证安全性 | 与 ftrace/kprobe 等同样修改函数指令的特性存在冲突 |

## 1.3 热补丁方案总结
-------

总体来看:

1.  实现思路主要有两类:

| 实现方法 | 优点 | 缺点 |
|:------:|:----:|:---:|
| 基于 ftrace 的方案, 通过注册 ftrace, 修改函数的返回地址, ftrace 返回后, 直接跳转到新函数执行 | 跟 ftrace/kprobe 等同样需要修改函数指令的特性不冲突 | 依赖与 ftrace_regs 特性, 支持架构有限<br>ftrace 方案有性能问题, 虽然使用 x86_64 等架构的 ftrace trampoline 机制可以当前函数只注册了一个 ftrace 的时候直接使用 tracepoline 来优化性能, 但是使用场景受限, 且并不能解决 ftrace 所有的性能问题<br>基于的特性 ftrace 本身是一个调测特性, 稳定性和安全性都存在一定问题, 商用有一定风险 |
| 基于直接跳转的方案, 通过修改函数的前几条指令, 直接跳转到新函数 | 无性能问题 | 跟 ftrace/kprobe 等同样需要修改指令的方案存在冲突<br>涉及长跳转时, 可能需要改多条指令, 对函数的长度有要求限制<br>必须有可靠的栈回溯机制, 修改多条指令, 必须要求被 patched 的函数不能正在执行, 否则执行的指令前后不一致, 将出现严重问题 |

2.  一致性模型主要有三种思路

| 实现方法 | 优点 | 缺点 |
|:------:|:----:|:---:|
| stop_machine consistency 通过 stop_machine 停住所有核进行一致性检查, 通过栈回溯检查被 patched 函数是否被执行 | 安全可靠, stop_machine 会强制所有核停下原来的工作, 在非抢占式内核, 必然发生进程切换, 则总能找到一个安全的时刻去执行热补丁操作 | stop_machine 会中断原来的业务, 对性能影响较大 | 
| per-task consistency 通过标记或者引用技术, 标记出被 patched 的函数都没有执行, 或者进程处于安全上下文的状态 | 不中断原来业务的执行 | 一致性检查并不是完全可信的 |


## 1.4 内核社区的讨论
-------

最终经过了激烈的讨论, 大家认为各家的方案都存在一些问题, 社区对于使用哪家的方案也一直犹豫不决. 最终 Redhat 主张先搞一个动态热补丁的通用框架, 基于 ftrace 方案, 完成函数热替换的最基本功能, 不保证安全性, 不使用任何安全性机制, 只通过通用的热补丁制作/注册和使用的接口. 作为一个过渡方案. 而各家 KGraft/Kpatch 基于这个框架完成自己的方案. 一致性模型待后期充分讨论和检验后再做决定. 这个提议的最大依据就是, 大部分的 CVE 补丁即使没有一致性保证, 也可以安全的以动态补丁的方式打上. 因此社区可以先合入这样一个通用的记住方案. 然后接下来在考虑各种一致性方案的问题, 可以选择一种最优的一致性方案, 也可以将各种方案都集成进来, 然后用户根据自己 patch 的特点, 选择不同的一致性模型. 这个提议得到了大多数人的支持, 并由 Redhat 的 Seth Jennings 负责实现这个通用的框架. 

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:---------:|:----:|
| 2014/12/16 | Seth Jennings | Kernel Live Patching | 内核热补丁的基础框架 | v7 ☑ 4.0-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/527257), [LKML](https://www.lkml.org/lkml/2017/2/13/831) |



# 3 一致性模型 consistency model
-------

一致性模型是非常复杂的, 在第一版的 LIVE_PATCH 支持的时候, 邮件列表中分析了几种一致性模型, 但是却没有结论, 参见 [`Re: [PATCH 0/2] Kernel Live Patching`](https://lkml.org/lkml/2014/11/7/354).

在经过漫长的讨论之后, 基于 PER-TASK 的混合一致性模型合入了主线.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:---------:|:-----:|
| 2017/2/13 | Josh Poimboeuf | [livepatch: hybrid consistency model](https://lore.kernel.org/patchwork/cover/760164) | 基于 Kgraft 的PER TASK 的状态检查和基于 Redhat 可靠的栈检查的混合一致性模型 | v4 ☑ 4.12-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/760164), [LKML](https://www.lkml.org/lkml/2017/2/13/831) |


这个一致性模型被称为 **混合一致性模型 "hybrid consistency model"**, 因为他融合了

*   Kgraft 的 per-task 一致性模型, 保证同一个 universe 里不同时调用了旧函数又调用了新函数的情况

*   reliable stacktrace 可靠的栈检查, 检查被 patched 的函数不被执行.


将这几种互补的一致性方法结合, 来确定什么时候可以安全地修补任务

1.  第一个也是最有效的方法是对睡眠任务进行堆栈检查。如果给定任务的堆栈中没有受影响的函数，则该任务将被修补。在大多数情况下，这将在第一次尝试时修补大部分或所有的任务。否则它会周期性地不断尝试。这个选项只有在体系结构有可靠的堆栈时才可用(HAVE_RELIABLE_STACKTRACE)。

2. 第二种方法(如果需要的话)是内核退出切换。当任务从系统调用、用户空间IRQ或信号返回到用户空间时，它就被切换。它在以下情况下是有用的:
    *   a) 修补I/O 绑定的用户任务，这些任务在受影响的函数上处于休眠状态。在这种情况下，您必须发送SIGSTOP和SIGCONT来强制它退出内核并进行修补。

    *   b) 修补cpu绑定的用户任务。如果任务是高度cpu限制的，那么它将在下次被IRQ中断时得到修补。

    *   c) 在将来，它可以用于为还没有HAVE_RELIABLE_STACKTRACE的架构应用补丁。在这种情况下，您必须向系统上的大多数任务发出信号。然而，这还不支持，因为目前没有办法在没有HAVE_RELIABLE_STACKTRACE的情况下修补线程。

3.  对于空闲任务 "swapper"(IDLE) ，因为它们永远不会退出内核，所以它们在空闲循环中有一个 klp_update_patch_state()调用，允许它们在CPU进入空闲状态之前被修补。

## 3.1 per-task consistency model
-------

混合一致性模型中使用了 per-task 的一致性模型, 参见 [livepatch: change to a per-task consistency model](https://lore.kernel.org/patchwork/patch/760175).


## 3.2 reliable stacktrace
-------

可靠栈检查则进一步对 per-task 的一致性模型做了补充. 他检查函数是否正在运行, 


# 4 重定位信息
-------

## 4.1 重定位段
-------

## 4.2 归一的重定位信息
-------

## 4.3 JUMP_LABEL 支持
-------

kpatch 的实现一直是根据内核的进展而演进的, 对 JUMP_LABEL 的不断重试也体现除了这个问题的难搞.

1.  最开始 kpatch-build 不支持 JUMP_LABEL, [Livepatch does not handle static keys #931](https://github.com/dynup/kpatch/pull/931)

2. 为 kpatch-build 引入了 JUMP_LABEL 支持 [Add jump label support #937](https://github.com/dynup/kpatch/pull/937)

3.  发现内核热补丁对 JUMP_LABEL 的支持存在问题, 因此 kpatch 回退了之前的提交 [Revert "create-diff-object: add jump label support"](https://github.com/dynup/kpatch/pull/944)

4.  内核社区正在解决这个问题, kpatch-build 对 JUMP_LABEL 报告 WARN, 待内核解决后, 再行支持 [kpatch-build should warn about static keys #946](https://github.com/dynup/kpatch/issues/946)

对 JUMP_LABEL 的支持主要存在如下几点问题:

1.  如果 JUMP_LABEL 所使能的符号是属于内核或者被 patched 的驱动的, 那么

最终主线内核通过这个补丁解决了对 JUMP_LABEL 的支持问题 [livepatch: Apply vmlinux-specific KLP relocations early](https://lore.kernel.org/patchwork/cover/1233223).

| 时间  | 作者 |特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:---------:|:----:|
| 2017/03/28 | Dietmar Eggemann | [livepatch,module: Remove .klp.arch and module_disable_ro()](https://lore.kernel.org/patchwork/cover/1233213) | 增加 PELT 的跟踪点 | v1 | [PatchWork](https://lore.kernel.org/patchwork/cover/774154) |


# 5 引入 shadow 支持结构体中扩充字段
-------

| 时间  | 作者 |特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:---------:|:----:|
| 2017/08/31 | Joe Lawrence | [livepatch: introduce shadow variable API](https://lore.kernel.org/patchwork/cover/827343) | 增加 SHADOW API 支持结构体中引入新的变量 | v6 ☑ 4.15-rc1  | [PatchWork](https://lore.kernel.org/patchwork/cover/827343) |


# 6 架构支持
-------



相关的文章介绍: [47].




---

**引用:**

<div id="ref-anchor-1"></div>
- [1] [Single UNIX Specification](https://en.wikipedia.org/wiki/Single_UNIX_Specification%23Non-registered_Unix-like_systems)

<div id="ref-anchor-2"></div>
- [2] [POSIX 关于调度规范的文档](http://nicolas.navet.eu/publi/SlidesPosixKoblenz.pdf)

<div id="ref-anchor-3"></div>
- [3] [Towards Linux 2.6](https://link.zhihu.com/?target=http%3A//www.informatica.co.cr/linux-scalability/research/2003/0923.html)

<div id="ref-anchor-4"></div>
- [4] [Linux内核发布模式与开发组织模式(1)](https://link.zhihu.com/?target=http%3A//larmbr.com/2013/11/02/Linux-kernel-release-process-and-development-dictator-%26-lieutenant-system_1/)

<div id="ref-anchor-5"></div>
- [5] IBM developworks 上有一篇综述文章, 值得一读 :[Linux 调度器发展简述](https://link.zhihu.com/?target=http%3A//www.ibm.com/developerworks/cn/linux/l-cn-scheduler/)

<div id="ref-anchor-6"></div>
- [6] [CFS group scheduling [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/240474/)

<div id="ref-anchor-7"></div>
- [7] [http://lse.sourceforge.net/numa/](https://link.zhihu.com/?target=http%3A//lse.sourceforge.net/numa/)

<div id="ref-anchor-8"></div>
- [8] [CFS bandwidth control [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/428230/)

<div id="ref-anchor-9"></div>
- [9] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D5091faa449ee0b7d73bc296a93bca9540fc51d0a)

<div id="ref-anchor-10"></div>
- [10] [DMA模式\_百度百科](https://link.zhihu.com/?target=http%3A//baike.baidu.com/view/196502.htm)

<div id="ref-anchor-11"></div>
- [11] [进程的虚拟地址和内核中的虚拟地址有什么关系? - 詹健宇的回答](http://www.zhihu.com/question/34787574/answer/60214771)

<div id="ref-anchor-12"></div>
- [12] [Physical Page Allocation](https://link.zhihu.com/?target=https%3A//www.kernel.org/doc/gorman/html/understand/understand009.html)

<div id="ref-anchor-13"></div>
- [13] [The SLUB allocator [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/229984/)

<div id="ref-anchor-14"></div>
- [14] [Lumpy Reclaim V3 [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/211199/)

<div id="ref-anchor-15"></div>
- [15] [Group pages of related mobility together to reduce external fragmentation v28 [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/224254/)

<div id="ref-anchor-16"></div>
- [16] [Memory compaction [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/368869/)

<div id="ref-anchor-17"></div>
- [17] [kernel 3.10内核源码分析--TLB相关--TLB概念、flush、TLB lazy模式-humjb\_1983-ChinaUnix博客](https://link.zhihu.com/?target=http%3A//blog.chinaunix.net/uid-14528823-id-4808877.html)

<div id="ref-anchor-18"></div>
- [18] [Toward improved page replacement[LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/226756/)

<div id="ref-anchor-19"></div>
- [19] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D4f98a2fee8acdb4ac84545df98cccecfd130f8db)

<div id="ref-anchor-20"></div>
- [20] [The state of the pageout scalability patches [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/286472/)

<div id="ref-anchor-21"></div>
- [21] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D894bc310419ac95f4fa4142dc364401a7e607f65)

<div id="ref-anchor-22"></div>
- [22] [Being nicer to executable pages [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/333742/)

<div id="ref-anchor-23"></div>
- [23] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D8cab4754d24a0f2e05920170c845bd84472814c6)

<div id="ref-anchor-24"></div>
- [24] [Better active/inactive list balancing [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/495543/)

<div id="ref-anchor-25"></div>
- [25] [Smarter write throttling [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/245600/)

<div id="ref-anchor-26"></div>
- [26] [https://zh.wikipedia.org/wiki/%E6%8C%87%E6%95%B0%E8%A1%B0%E5%87%8F](https://link.zhihu.com/?target=https%3A//zh.wikipedia.org/wiki/%25E6%258C%2587%25E6%2595%25B0%25E8%25A1%25B0%25E5%2587%258F)

<div id="ref-anchor-27"></div>
- [27] [Flushing out pdflush [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/326552/)

<div id="ref-anchor-28"></div>
- [28] [Dynamic writeback throttling [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/405076/)

<div id="ref-anchor-29"></div>
- [29] [On-demand readahead [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/235164/)

<div id="ref-anchor-30"></div>
- [30] [Transparent huge pages in 2.6.38 [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/423584/)

<div id="ref-anchor-31"></div>
- [31] [https://events.linuxfoundation.org/sites/events/files/lcjp13\_ishimatsu.pdf](https://link.zhihu.com/?target=https%3A//events.linuxfoundation.org/sites/events/files/lcjp13_ishimatsu.pdf)

<div id="ref-anchor-32"></div>
- [32] [transcendent memory for Linux [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/338098/)

<div id="ref-anchor-33"></div>
- [33] [linux kernel monkey log](https://link.zhihu.com/?target=http%3A//www.kroah.com/log/linux/linux-staging-update.html)

<div id="ref-anchor-34"></div>
- [34] [zcache: a compressed page cache [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/397574/)

<div id="ref-anchor-35"></div>
- [35] [The zswap compressed swap cache [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/537422/)

<div id="ref-anchor-36"></div>
- [36] [Linux-Kernel Archive: Linux 2.6.0](https://link.zhihu.com/?target=http%3A//lkml.iu.edu/hypermail/linux/kernel/0312.2/0348.html)

<div id="ref-anchor-37"></div>
- [37]抢占支持的引入时间: [https://www.kernel.org/pub/linux/kernel/v2.5/ChangeLog-2.5.4](https://link.zhihu.com/?target=https%3A//www.kernel.org/pub/linux/kernel/v2.5/ChangeLog-2.5.4)

<div id="ref-anchor-38"></div>
- [38] [RAM is 100 Thousand Times Faster than Disk for Database Access](https://link.zhihu.com/?target=http%3A//www.directionsmag.com/entry/ram-is-100-thousand-times-faster-than-disk-for-database-access/123964)

<div id="ref-anchor-39"></div>
- [39] [http://www.uefi.org/sites/default/files/resources/ACPI\_6.0.pdf](https://link.zhihu.com/?target=http%3A//www.uefi.org/sites/default/files/resources/ACPI_6.0.pdf)

<div id="ref-anchor-40"></div>
- [40] [Injecting faults into the kernel [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/209257/)

<div id="ref-anchor-41"></div>
- [41] [Detecting kernel memory leaks [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/187979/)

<div id="ref-anchor-42"></div>
- [42] [The kernel address sanitizer [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/612153/)

<div id="ref-anchor-43"></div>
- [43] [Linux Kernel Shared Memory 剖析](https://link.zhihu.com/?target=http%3A//www.ibm.com/developerworks/cn/linux/l-kernel-shared-memory/)

<div id="ref-anchor-44"></div>
- [44] [KSM tries again [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/330589/)

<div id="ref-anchor-45"></div>
- [45] [HWPOISON [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/348886/)

<div id="ref-anchor-46"></div>
- [46] [https://www.mcs.anl.gov/research/projects/mpi/](https://link.zhihu.com/?target=https%3A//www.mcs.anl.gov/research/projects/mpi/)

<div id="ref-anchor-47"></div>
- [47] [Fast interprocess messaging [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/405346/)



---8<---

**更新日志:**

**- 2015.9.12**

o 完成调度器子系统的初次更新, 从早上10点开始写, 写了近７小时, 比较累, 后面更新得慢的话大家不要怪我(对手指

**- 2015.9.19**

o 完成内存管理子系统的前4章更新. 同样是写了一天, 内容太多, 没能写完......

**- 2015.9.21**

o 完成内存管理子系统的第5章"页面写回"的第1小节的更新.
**- 2015.9.25**

o 更改一些排版和个别文字描述. 接下来周末两天继续.
**- 2015.9.26**

o 完成内存管理子系统的第5, 6, 7, 8章的更新.
**- 2015.10.14**

o 国庆离网10来天, 未更新.  今天完成了内存管理子系统的第9章的更新.
**- 2015.10.16**

o 完成内存管理子系统的第10章的更新.
**- 2015.11.22**

o 这个月在出差和休假, 一直未更新.抱歉! 根据知友 [@costa](https://www.zhihu.com/people/78ceb98e7947731dc06063f682cf9640) 提供的无水印图片和考证资料, 进行了一些小更新和修正. 特此感谢 !

o 完成内存管理子系统的第11章关于 NVDIMM 内容的更新.
**- 2016.1.2**

o 中断许久, 今天完成了内存管理子系统的第11章关于调试支持内容的更新.
**- 2016.2.23**

o 又中断许久, 因为懒癌发作Orz... 完成了第二个子系统的所有章节.
[编辑于 06-27](https://www.zhihu.com/question/35484429/answer/62964898)
