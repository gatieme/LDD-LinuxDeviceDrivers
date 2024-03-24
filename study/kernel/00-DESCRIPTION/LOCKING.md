---

title: 锁机制
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

2   **锁**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***

# 1 SPINLOCK
-------

[Linux 中的 spinlock 机制 [一] - CAS 和 ticket spinlock](https://zhuanlan.zhihu.com/p/80727111)

[Linux 中的 spinlock 机制 [二] - MCS Lock](https://zhuanlan.zhihu.com/p/89058726)

[Linux 中的 spinlock 机制 [三] - qspinlock](https://zhuanlan.zhihu.com/p/100546935)

[Non-scalable locks are dangerous](https://pdos.csail.mit.edu/papers/linux:lock.pdf)
[Non-scalable locks are dangerous](https://andreybleme.com/2021-01-24/non-scalable-locks-are-dangerous-summary)
[Non-scalable locks are dangerous](https://www.jianshu.com/p/d058fb620f89)
[Scalable   Locking](https://pdos.csail.mit.edu/6.828/2018/lec/l-mcs.pdf)

## 1.1 CAS(compare and swap) LOCK
-------

CAS 的原理是, 将旧值与一个期望值进行比较, 如果相等, 则更新旧值.

这种是实现 spinlock 用一个整形变量表示, 其初始值为 1, 表示 available 的状态(可以被 1 用户独占).

1.  当一个 CPU A 获得 spinlock 后, 会将该变量的值设为 0(不能被任何用户再占有), 之后其他 CPU 试图获取这个 spinlock 时, 会一直等待, 直到 CPU A 释放 spinlock, 并将该变量的值设为 1.

2.  等待该 spinlock 的 CPU B 不断地把 「期望的值」 1 和 「实际的值」 (1 或者 0)进行比较(compare), 当它们相等时, 说明持有 spinlock 当前未被任何人持有(持有的 CPU 已经释放了锁或者锁一直未被任何人持有), 那么试图获取 spinlock 的 CPU 就会尝试将 "new" 的值(0) 写入 "p"(swap), 以表明自己成为占有了 spinlock, 成为新的 owner.

这里只用了 0 和 1 两个值来表示 spinlock 的状态, 没有充分利用 spinlock 作为整形变量的属性, 为此还有一种衍生的方法, 可以判断当前 spinlock 的争用情况. 具体规则是: 每个 CPU 在试图获取一个 spinlock 时, 都会将这个 spinlock 的值减1, 所以这个值可以是负数, 而「负」的越多(负数的绝对值越大), 说明当前的争抢越激烈.

存在的问题
基于CAS的实现速度很快, 尤其是在没有真正竞态的情况下(事实上大部分时候就是这种情况),  但这种方法存在一个缺点：它是「不公平」的.  一旦spinlock被释放, 第一个能够成功执行CAS操作的CPU将成为新的owner, 没有办法确保在该spinlock上等待时间最长的那个CPU优先获得锁, 这将带来延迟不能确定的问题.

## 1.2 ticket LOCK
-------

[Ticket spinlocks](https://lwn.net/Articles/267968)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/11/01 | Nick Piggin <npiggin@suse.de> | [ticket spinlocks for x86](https://lore.kernel.org/patchwork/cover/95892) | X86 架构 ticket spinlocks 的实现. | v1 ☑ 2.6.25-rc1(部分合入) | [PatchWork RFC](https://lore.kernel.org/patchwork/cover/85789)<br>*-*-*-*-*-*-*-* <br>[PatchWork](https://lore.kernel.org/patchwork/cover/95892), [PatchWork](https://lore.kernel.org/patchwork/cover/95894) |
| 2021/09/19 | Guo Ren <guoren@kernel.org> | [riscv: locks: introduce ticket-based spinlock implementation](https://patchwork.kernel.org/project/linux-riscv/patch/20210919165331.224664-1-guoren@kernel.org) | riscv 架构 ticket spinlocks 的实现. | v1 ☐ |[PatchWork](https://patchwork.kernel.org/project/linux-riscv/patch/20210919165331.224664-1-guoren@kernel.org) |
| 2013/10/09 | Nick Piggin <npiggin@suse.de> | [arm64: locks: introduce ticket-based spinlock implementation](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=52ea2a560a9dba57fe5fd6b4726b1089751accf2) | ARM64 架构 ticket spinlocks 的实现. | v1 ☑ 3.13-rc1 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=52ea2a560a9dba57fe5fd6b4726b1089751accf2) |
| 2015/02/10 | Nick Piggin <npiggin@suse.de> | [arm64: locks: patch in lse instructions when supported by the CPU](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=81bb5c6420635dfd058c210bd342c29c95ccd145) | ARM64 ticket spinlocks 使用 LSE 进行优化. | v1 ☑ 4.3-rc1 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=81bb5c6420635dfd058c210bd342c29c95ccd145) |
| 2022/04/14 | Palmer Dabbelt <palmer@rivosinc.com> | [RISC-V: Generic Ticket Spinlocks](https://patchwork.kernel.org/project/linux-riscv/cover/20220414220214.24556-1-palmer@rivosinc.com/) | 632429 | v3 ☐☑ | [LORE v3,0/7](https://lore.kernel.org/all/20220414220214.24556-1-palmer@rivosinc.com) |


[Linux中的spinlock机制[一] - CAS和ticket spinlock](https://zhuanlan.zhihu.com/p/80727111)

[BAKERY ALGORITHM](https://remonstrate.wordpress.com/tag/bakery-algorithm)
[Lamport 面包店算法](https://blog.csdn.net/pizi0475/article/details/17649949)

## 1.3 MCS lock
-------

[MCS locks and qspinlocks](https://lwn.net/Articles/590243)

spinlock 的值出现变化时, 所有试图获取这个 spinlock 的 CPU 都需要读取内存, 刷新自己对应的 cache line, 而最终只有一个 CPU 可以获得锁, 也只有它的刷新才是有意义的. 锁的争抢越激烈(试图获取锁的CPU数目越多), 无谓的开销也就越大.

如果在 ticket spinlock 的基础上进行一定的修改, 让每个 CPU 不再是等待同一个 spinlock 变量, 而是基于各自不同的 per-CPU 的变量进行等待, 那么每个 CPU 平时只需要查询自己对应的这个变量所在的本地 cache line, 仅在这个变量发生变化的时候, 才需要读取内存和刷新这条 cache line, 这样就可以解决上述的这个问题.

要实现类似这样的 spinlock的 「分身」, 其中的一种方法就是使用 MCS lock. 试图获取一个 spinlock 的每个CPU, 都有一份自己的 MCS lock.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2014/01/21 | Tim Chen <tim.c.chen@linux.intel.com> | [MCS Lock: MCS lock code cleanup and optimizations](https://lore.kernel.org/patchwork/cover/435770) | MCS LOCK 重构, 增加了新的文件 `include/linux/mcs_spinlock.h` | v9 ☑ 3.15-rc1 | [LKML v6 0/6](https://lkml.org/lkml/2013/9/25/532)<br>*-*-*-*-*-*-*-* <br>[PatchWork v9 0/6](https://lore.kernel.org/patchwork/cover/435770), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e72246748ff006ab928bc774e276e6ef5542f9c5) |
| 2014/02/10 | Peter Zijlstra <peterz@infradead.org> | [locking/core patches](https://lore.kernel.org/patchwork/cover/440565) | PV SPINLOCK | v1 ☑ 4.2-rc1 | [PatchWork](https://lore.kernelorg/lkml/20140210195820.834693028@infradead.org) |


## 1.4 qspinlock
-------

### 1.4.1 X86
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/08/28 | Nick Piggin <npiggin@suse.de> | [queueing spinlocks?](https://lore.kernel.org/patchwork/cover/127444) | X86 架构 qspinlocks 的实现. | RFC ☐ | [PatchWork RFC](https://lore.kernel.org/lkml/20080828073428.GA19638@wotan.suse.de) |
| 2015/04/24 | Waiman Long <Waiman.Long@hp.com> | [qspinlock: a 4-byte queue spinlock with PV support](https://lore.kernel.org/patchwork/cover/127444) | X86 架构 qspinlocks 的实现. | v16 ☑ 4.2-rc1 | [PatchWork RFC](https://lore.kernel.org/lkml/20140310154236.038181843@infradead.org)<br>*-*-*-*-*-*-*-* <br>[LORE v16 00/14](https://lore.kernel.org/all/1429901803-29771-1-git-send-email-Waiman.Long@hp.com/) |

### 1.4.2 ARM64
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/04/10 | Yury Norov <ynorov@caviumnetworks.com> | [arm64: queued spinlocks and rw-locks](http://patches.linaro.org/cover/98492) | ARM64 架构 qspinlocks 的实现. | RFC ☐ | [PatchWork RFC](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1491860104-4103-4-git-send-email-ynorov@caviumnetworks.com)<br>*-*-*-*-*-*-*-* <br>[LORE 0/3](https://lore.kernel.org/lkml/20170503145141.4966-1-ynorov@caviumnetworks.com) |
| 2018/04/26 | Will Deacon <will.deacon@arm.com> | [kernel/locking: qspinlock improvements](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=baa8c6ddf7be33f2b0ddeb68906d668caf646baa) | 优化并实现了 qspinlocks 的通用框架. | v3 ☑ 4.18-rc1 | [LWN](https://lwn.net/Articles/751105), [LORE v3,00/14](https://lore.kernel.org/all/1524738868-31318-1-git-send-email-will.deacon@arm.com) |
| 2018/06/26 | Will Deacon <will.deacon@arm.com> | [Hook up qspinlock for arm64](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=5d168964aece0b4a41269839c613683c5d7e0fb2) | ARM64 架构 qspinlocks 的实现. | v1 ☑ 4.19-rc1 | [LORE 0/3](https://lore.kernel.org/linux-arm-kernel/1530010812-17161-1-git-send-email-will.deacon@arm.com) |


### 1.4.3 RISC-V
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/06/21 | guoren@kernel.org <guoren@kernel.org> | [riscv: Add qspinlock support](https://lore.kernel.org/all/20220621144920.2945595-1-guoren@kernel.org) | TODO | v6 ☐☑✓ | [LORE v5](https://lore.kernel.org/lkml/20220620155404.1968739-1-guoren@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v6,0/2](https://lore.kernel.org/all/20220621144920.2945595-1-guoren@kernel.org) |

### 1.4.4 powerpc
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/11/26 | Nicholas Piggin <npiggin@gmail.com> | [powerpc: alternate queued spinlock implementation](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0b2199841a7952d01a717b465df028b40b2cf3e9) | [Linux 6.2 Landing Scalability Improvement For Large IBM Power Systems](https://www.phoronix.com/news/IBM-Power-Linux-6.2) | v3 ☐☑✓ 6.1-rc2 | [LORE v3,0/17](https://lore.kernel.org/all/20221126095932.1234527-1-npiggin@gmail.com) |



## 1.5 PV_SPINLOCK
-------

[PV qspinlock 原理](https://blog.csdn.net/bemind1/article/details/118224344)

[Hook 内核之 PVOPS](https://diting0x.github.io/20170101/pvops)

spinlock 在非虚拟化的环境下, 它是可以认为 CPU 不会被抢占的, 所以 A 拿锁干活, B 死等 A, A 干完自己的活, 就释放了, 中间不会被调度.

但是在虚拟化下, CPU 对应到 vcpu, 每个 vcpu 跟之前裸机上的进程调度一样, 所以 A 拿锁干活, 并不一定不会被抢占, 很有可能被调度走了, 因为 cpu 这时候还不知道 vcpu 在干嘛. B 死等 A, 但是 A 被调度了, 运行了 C, C 也要死等 A, 在一些设计不够好的系统里面, 这样就会变得很糟糕.


为了保证spinlock unlock的公平性, 有一种队列的spinlock, ticketlock,  http://www.ibm.com/developerworks/cn/linux/l-cn-spinlock/这篇文章介绍的非常详细, 总之根据next, own, 来判断是否到自己了. 这样一种机制在裸机上是可以解决公平的问题, 但是放到虚拟化环境里, 它会使问题变得更糟. C必须等到B完成才可以, 如果中间B被调度了, 又开始循环了, 当然更糟的定义也是相对的, 如果vcpu的调度机制能够vcpu正在拿锁的话, 会怎样？


jeremy很早就写了一个pv ticketlock, 原理大概就是vcpu在拿锁了一段时间, 会放弃cpu, 并blocked, unlocked的时候会被唤醒, 这个针对PV制定的优化, 在vcpu拿不到锁的场景下, 并没有任何的性能损耗, 并且能够解决之前的问题, 但是当运行native linux的时候, 就会有性能损耗, 所以当时在config里面添加了一个编译选项CONFIG_PARAVIRT_SPINLOCK, 话说我们的系统里面, 这个是没打开的啊, 后面要再好好评估下


之后, 这个patch进行了改良, 在原有native linux的ticketlock的基础, 增加了一种模式, 通过检测cpu是否spinned一段时间, 判断是否要进入slow path, 之前的fast path的逻辑和原来保持不变, 进入slowpath后, 会在ticketlock里面置位, 并block vcpu, 等unlock的时候, 这个位会被clear, 因为占用了一个位, 所以能用的ticket少了一半.

这个方案在一些硬件(XEON x3450)上进行各种benchmark测试后, 结论是不再有任何的性能损耗.



好吧, 说了这么多理论性的东西, 再来说下, 我们实际遇到的问题.  很早以前经常有windows用户的工单投诉, 说自己的vm里面cpu没有怎么使用, 为什么cpu显示百分之百.

由于是windows系统, 加上我是小白, 很难给出一些技术细节上的分析, 只能通过简单滴一些测试实验进行调查.



最后的结果, 就是windows很多核的情况下, 比如12、16,  在一个稍微有点load的物理机上面, 跑一些cpu压力, 就很容易出现cpu百分百的问题, 后面降core之后, 情况有所缓解. 后面大致的分析结果是, windows里面很多操作是用到spinlock的, 当一个core拿到锁, 事情没有做完, 被调度了, 这时其他的core也需要拿锁, 当core越来越多的时候, 情况就越来越糟, 最后看上去就大家都很忙, 但实际什么事情也没做.



目前来看, 已经有一种较为成熟的软件方法来解决类似问题, 期待后续是否会有硬件的一些特性来支持, 或许已经有了.


PV_SPINLOCKS 的合入引起了[性能问题 Performance overhead of paravirt_ops on native identified](https://lore.kernel.org/patchwork/patch/156045), yinru


当开启了 CONFIG_PARAVIRT_SPINLOCKS 之后, queued_spin_lock_slowpath() 将作为[宏函数被展开多份](https://elixir.bootlin.com/linux/v4.2/source/kernel/locking/qspinlock.c#L281), 一份 [native_queued_spin_lock_slowpath()](https://elixir.bootlin.com/linux/v4.2/source/kernel/locking/qspinlock.c#L255) 用于传统的 spinlock 场景, 一份 [`__pv_queued_spin_lock_slowpath()`](https://elixir.bootlin.com/linux/v4.2/source/kernel/locking/qspinlock.c#L468) 用于虚拟化场景. 参见 [v4.2: commit a23db284fe0d locking/pvqspinlock: Implement simple paravirt support for the qspinlock](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a23db284fe0d1879ca2002bf31077b5efa2fe2ca). 在启动阶段, 通过 PVOPS 机制动态的的将 spinlock 替换为内核实际所需的 spinlock 处理函数. 虚拟化 guest 中将在 kvm_spinlock_init() 中被替换为虚拟化场景[所需的 `__pv_queued_spin_lock_slowpath()` ](https://elixir.bootlin.com/linux/v4.2/source/arch/x86/kernel/kvm.c#L868) 等函数.



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/07/07 | Raghavendra K T <raghavendra.kt@linux.vnet.ibm.com> | [Paravirtual spinlocks](https://lore.kernel.org/patchwork/patch/121810) | PV_SPINLOCK 实现. | RFC ☑ 2.6.27-rc1 | [PatchWork RFC](https://lore.kernel.org/patchwork/cover/121810) |
| 2009/05/15 | Raghavendra K T <raghavendra.kt@linux.vnet.ibm.com> | [x86: Fix performance regression caused by paravirt_ops on native kernels](https://lore.kernel.org/patchwork/patch/156045/) | NA | v13 ☑ 3.12-rc1 | [PatchWork v13](https://lore.kernel.org/patchwork/cover/156402) |
| 2013/08/09 | Raghavendra K T <raghavendra.kt@linux.vnet.ibm.com> | [Paravirtualized ticket spinlocks](https://lore.kernel.org/patchwork/cover/398912) | PV_SPINLOCK 的 ticket lock 实现. | v13 ☑ 3.12-rc1 | [PatchWork v13](https://lore.kernel.org/patchwork/cover/398912) |
| 2015/04/07 | Waiman Long <Waiman.Long@hp.com> | [qspinlock: a 4-byte queue spinlock with PV support](https://lore.kernel.org/patchwork/cover/558505) | PV SPINLOCK | v15 ☑ 4.2-rc1 | [PatchWork v15](https://lore.kernel.org/patchwork/cover/558505) |
| 2015/11/10 | Waiman Long <Waiman.Long@hpe.com> | [locking/qspinlock: Enhance pvqspinlock performance](https://lore.kernel.org/patchwork/cover/616398) | PV SPINLOCK | v10 ☑ 4.5-rc1 | [PatchWork v5](https://lore.kernel.org/patchwork/cover/588106)<br>*-*-*-*-*-*-*-* <br>[PatchWork v10](https://lore.kernel.org/patchwork/cover/616398) |
| 2018/10/08 | Raghavendra K T <raghavendra.kt@linux.vnet.ibm.com> | [Enable PV qspinlock for Hyper-V](https://lore.kernel.org/patchwork/cover/996494) | Hyper-V 的 PV spiclock 实现. | v2 ☑ 4.20-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/cover/996494) |
| 2019/10/23 | Zhenzhong Duan <zhenzhong.duan@oracle.com> | [Add a unified parameter "nopvspin"](https://lore.kernel.org/patchwork/cover/1143398) | PV SPINLOCK | v8 ☑ 5.9-rc1 | [PatchWork v8](https://lore.kernel.org/patchwork/cover/1143398) |
|


## 1.5 NumaAware SPINLOCK
-------

[关于多核 CPU 自旋锁 (spinlock) 的优化](https://blog.csdn.net/cpongo1/article/details/89539933)

[NUMA-aware qspinlocks](https://lwn.net/Articles/852138)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/08/24 | H.J. Lu  | [numa-spinlock](https://sanidhya.github.io/pubs/2019/shfllock.pdf) | 阿里实现的用户态 numa aware spinlock. | ☐ |[GitLab](https://gitlab.com/numa-spinlock/numa-spinlock) |
| 2019/08/24 | sanidhya | [Scalable and Practical Locking with Shuffling](https://sanidhya.github.io/pubs/2019/shfllock.pdf) | Shuffling 锁实现了洗牌技术. 将等待锁的线程更指定的策略进行重新排序. 类似于通过定义的比较功能对 waiter 进行排序. 实现 NUMA 感知的唤醒和阻塞策略. 洗牌的动作通常不会在关键路径上执行. | ☐ | [GitHub](https://github.com/sslab-gatech/shfllock) |
| 2019/08/08 | dozenow | ShortCut: Accelerating Mostly-Deterministic Code Regions | NA | ☐ | [GitHub](hhttps://github.com/shortcut-sosp19/shortcut) |
| 2021/05/14 | Alex Kogan <alex.kogan@oracle.com> | [Add NUMA-awareness to qspinlock](https://lwn.net/Articles/856387) | NUMA 感知的 spinlock, 基于 [CNA-compact-numa-aware-locks](https://deepai.org/publication/compact-numa-aware-locks). | v15 ☐ | [PatchWork v15](https://lore.kernel.org/patchwork/cover/1428910), [PatchWork v15,0/6 ARM](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20210514200743.3026725-1-alex.kogan@oracle.com) |



## 1.6 SPINlOCK DEBUG
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/05/14 | Alex Kogan <alex.kogan@oracle.com> | [x86, locking/qspinlock: Allow lock to store lock holder cpu number](https://lkml.org/lkml/2020/7/16/1066) | struct raw_spinlock 中有 owner_cpu, 但是需要开启 CONFIG_DEBUG_SPINLOCK, 这个选项对 spinlock 的性能影响较大. 这个补丁集修改 x86 的 qspinlock 和 qrwlock 代码, 允许它在可行的情况下将锁持有者的 cpu 号 (qrwlock 的锁写入器 cpu 号) 存储在锁结构本身, 这对调试和崩溃转储分析很有用. 通过定义宏 `__cpu_number_sadd1` (用于 qrwlock) 和 `__cpu_number_sadd2` (用于 qspinlock), 以达到饱和的 + 1 和 + 2 cpu 数, 可以在 qspinlock 和 qrwlock 的锁字节中使用. 可以在每个体系结构的基础上启用该功能, 当前只提供了 x86 下的实现.| v2 ☐ | [PatchWork v2,0/5](https://lkml.org/lkml/2020/7/16/1066) |


# 2 RWSEM
-------

## 2.1 RWSEM
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/11/21 | Waiman Long <longman@redhat.com> | [locking/rwsem: Rework reader optimistic spinning](https://lore.kernel.org/patchwork/cover/1342950) | 当读者的评论部分很短, 周围没有那么多读者时, 读者乐观旋转(osq_lock)是有帮助的. 它还提高了读者获得锁的机会, 因为写入器乐观旋转对写入器的好处远远大于读者. 由于提交d3681e269fff ("locking/rwsem: Wake up almost all reader in wait queue"), 所有等待的reader都会被唤醒, 这样它们就都能获得读锁并并行运行. 当竞争的读者数量很大时, 允许读者乐观自旋很可能会导致读者碎片, 多个较小的读者组可以以顺序的方式(由写入器分隔)获得读锁. 这降低了读者的并行性. 解决这个缺点的一种可能方法是限制能够进行乐观旋转的读者的数量(最好是一个). 这些读者作为等待队列中所有等待的读者的代表, 因为一旦获得锁, 它们将唤醒所有等待的读者.  | v2 ☐ | [PatchWork v2,0/5](https://lore.kernel.org/patchwork/cover/1342950) |


## 2.2 PER-CPU RWSEM
-------

percpu rw 信号量是一种新的读写信号量设计, 针对读取锁定进行了优化.

传统的读写信号量的问题在于, 当多个内核读取锁时, 包含信号量的 cache-line 在内核的 L1 缓存之间弹跳, 导致性能下降. 锁定读取速度非常快, 它使用 RCU, 并且避免了锁定和解锁路径中的任何原子指令. 另一方面, 写入锁定非常昂贵, 它调用 synchronize_rcu () 可能需要数百毫秒. 使用 RCU 优化 rw-lock 的想法是由 Eric Dumazet <eric.dumazet@gmail.com>. 代码由 Mikulas Patocka <mpatocka@redhat.com> 编写

锁是用 “struct percpu_rw_semaphore” 类型声明的. 锁被初始化为 percpu_init_rwsem, 它在成功时返回 0, 在分配失败时返回 -ENOMEM. 必须使用 percpu_free_rwsem 释放锁以避免内存泄漏.

使用 percpu_down_read 和 percpu_up_read 锁定读取, 使用 percpu_down_write 和 percpu_up_write 锁定写入.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/06/22 | Peter Zijlstra <peterz@infradead.org> | [percpu rwsem -v2](https://lore.kernel.org/all/20150622121623.291363374@infradead.org) | TODO | v2 ☐☑✓ | [LORE v2,0/13](https://lore.kernel.org/all/20150622121623.291363374@infradead.org) |
| 2012/08/31 | Mikulas Patocka <mpatocka@redhat.com> | [Fix a crash when block device is read and block size is changed at the same time](https://lore.kernel.org/patchwork/cover/323377) | NA | v2 ☑ 3.8-rc1 | [PatchWork 0/4](https://lore.kernel.org/patchwork/cover/323377) |
| 2020/11/07 | Oleg Nesterov <oleg@redhat.com> | [percpu_rw_semaphore: reimplement to not block the readers unnecessarily](https://lore.kernel.org/patchwork/cover/1342950) | NA | v2 ☑ 3.8-rc1 | [PatchWork v2,0/5](https://lore.kernel.org/patchwork/cover/339247), [PatchWork](https://lore.kernel.org/patchwork/cover/339702)<br>*-*-*-*-*-*-*-* <br>[PatchWork](https://lore.kernel.org/patchwork/patch/339064) |
| 2020/11/18 | Oleg Nesterov <oleg@redhat.com> | [percpu_rw_semaphore: lockdep + config](https://lore.kernel.org/patchwork/cover/1342950) | NA | v1 ☑ 3.8-rc1 | [PatchWork 0/3](https://lore.kernel.org/patchwork/cover/341521) |
| 2020/01/31 | Peter Zijlstra <peterz@infradead.org> | [locking: Percpu-rwsem rewrite](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=41f0e29190ac9e38099a37abd1a8a4cb1dc21233) | TODO | v1 ☐☑✓ | [LORE v1,0/7](https://lore.kernel.org/all/20200131150703.194229898@infradead.org) |


# 3 MUTEX
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2013/4/17 | Waiman Long <Waiman.Long@hp.com> | [mutex: Improve mutex performance by doing less atomic-ops & better spinning](https://lkml.org/lkml/2013/4/17/418) | NA | v4 ☑ 3.10-rc1 | [LKML 0/3 v2](https://lkml.org/lkml/2013/4/15/245)<br>*-*-*-*-*-*-*-* <br>[LKML v4 0/4](https://lkml.org/lkml/2013/4/17/418) |


# 4 membarrier
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/10/19 | Raghavendra K T <raghavendra.kt@linux.vnet.ibm.com> | [membarrier: Provide register expedited private command](https://lore.kernel.org/patchwork/cover/843003) | 引入 MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED. | v6 ☑ 4.14-rc6 | [PatchWork v5](https://lore.kernel.org/patchwork/cover/835747)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6](https://lore.kernel.org/patchwork/cover/398912) |
| 2017/11/21 | Raghavendra K T <raghavendra.kt@linux.vnet.ibm.com> | [membarrier: Provide core serializing command](https://lore.kernel.org/patchwork/cover/843003) | 引入 MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED. | v6 ☑ 4.16-rc1 | [PatchWork v5](https://lore.kernel.org/patchwork/cover/835747)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6](https://lore.kernel.org/patchwork/cover/398912) |
| 2018/01/29 | Raghavendra K T <raghavendra.kt@linux.vnet.ibm.com> | [membarrier: Provide core serializing command](https://lore.kernel.org/patchwork/cover/843003) | 引入 MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED. | v6 ☑ 4.16-rc1 | [PatchWork v5](https://lore.kernel.org/patchwork/cover/835747)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6](https://lore.kernel.org/patchwork/cover/398912) |


# 5 RCU
-------

[What is RCU, Fundamentally?](https://lwn.net/Articles/262464)

[What is RCU? Part 2: Usage](https://lwn.net/Articles/263130)

[Recent RCU changes](https://lwn.net/Articles/894379)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/06/01 | "Joel Fernandes (Google)" <joel@joelfernandes.org> | [Harden list_for_each_entry_rcu() and family](https://lore.kernel.org/patchwork/cover/1082845) | 本系列增加了一个新的内部函数rcu_read_lock_any_held(), 该函数在调用这些宏时检查reader节是否处于活动状态. 如果不存在reader section, 那么list_for_each_entry_rcu()的可选第四个参数可以是一个被计算的lockdep表达式(类似于rcu_dereference_check()的工作方式). . | RFC ☑ 5.4-rc1 | [PatchWork RFC,0/6](https://lore.kernel.org/patchwork/cover/1082845) |

Google 的 Joel Fernandes 等发现 RCU 并没有很好的节能, 在 Android 和 ChromeOS 系统的功耗方面, RCU 占据了比较大的比重. 他们在 LPC-2022 上演示了他们在延迟 RCU 处理等降低 RCU 功耗和底噪的工作. 参见 [Make RCU do less (& later) !](https://lpc.events/event/16/contributions/1204). 随后 2022 年 10 月份左右, 补丁[推送到 v6.2 版本](https://lore.kernel.org/rcu/20221019225138.GA2499943@paulmck-ThinkPad-P17-Gen-1/).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/10/19 | Paul E. McKenney <paulmck@kernel.org> | [Lazy call_rcu() updates for v6.2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9d40c84cf5bcb5b1d124921ded2056d76be7640d) | 参见 LWN 报道 [The intersection of lazy RCU and memory reclaim](https://lwn.net/Articles/931920) | v6 ☑✓ 6.2-rc1 | [LORE v6,00/14](https://lore.kernel.org/all/20221019225138.GA2499943@paulmck-ThinkPad-P17-Gen-1)<br>*-*-*-*-*-*-*-* <br>[LORE 00/16](https://lore.kernel.org/all/20221122010408.GA3799268@paulmck-ThinkPad-P17-Gen-1) |


# 6 FUTEX
-------

[FUTEX2's sys_futex_waitv() Sent In For Linux 5.16 To Help Linux Gaming](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.16-sys_futex_waitv)

[FUTEX2 Begins Sorting Out NUMA Awareness](https://www.phoronix.com/scan.php?page=news_item&px=FUTEX2-NUMA-Awareness-RFC)

[NUMA Interface For FUTEX2 Still Being Tackled For Linux](https://www.phoronix.com/news/FUTEX2-NUMA-Still-Coming-2022)

[内核工匠-futex问答](https://blog.csdn.net/feelabclihu/article/details/127020166)

# 7 Semaphores
-------

[The Little Book of Semaphores](https://www.greenteapress.com/semaphores/LittleBookOfSemaphores.pdf)

# 8 LockLess
-------


[An introduction to lockless algorithms](https://lwn.net/Articles/844224)


# 9 ATOMIC
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/08/06 | Will Deacon <will.deacon@arm.com> | [Add generic support for relaxed atomics](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0ca326de7aa9cb253db9c1a3eb3f0487c8dbf912) | ARM64 引入 relaxed atomics. | v5 ☑ 4.3-rc1 | [LORE 0/5](https://lore.kernel.org/lkml/1436790687-11984-1-git-send-email-will.deacon@arm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/7](https://lore.kernel.org/lkml/1437060758-10381-1-git-send-email-will.deacon@arm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/8](https://lore.kernel.org/all/1438880084-18856-1-git-send-email-will.deacon@arm.com) |

# 10 LOCKDEP
-------


## 10.1 LOCKDEP
-------

Lockdep 跟踪锁的获取顺序, 以检测死锁, 以及 IRQ 和 IRQ 启用/禁用状态, 并考虑故障现场分析或获取. Lockdep 在检测到并报告死锁后应立即关闭, 因为由于设计复杂, 检测后数据结构和算法不可重用.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2006/07/03 | Paul Mackerras <paulus@samba.org> | [Add generic support for relaxed atomics](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=829035fd709119d9def124a6d40b94d317573e6f) | LOCKDEP 死锁检测机制. | v5 ☑ 2.6.18-rc1 | [LORE 0/5](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=829035fd709119d9def124a6d40b94d317573e6f) |


参见 [[PATCH RFC v6 00/21] DEPT(Dependency Tracker)](https://lore.kernel.org/lkml/1651652269-15342-1-git-send-email-byungchul.park@lge.com), 分析了 Lockdep 的问题以及引入 Dependency Tracker 的背景和设计思路.

但是 Lockdep 依旧有太多问题:


1. 错误消息有时令人困惑且难以理解, 这不仅使读取死锁方案难以理解, 而且还使内部错误难以调试.

2. 一旦报告了一个问题, 所有锁定功能都将关闭. 尽管这是合理的, 因为一旦检测到锁定问题, 整个系统就会受到锁定错误的影响, 并且在修复错误之前继续运行
系统是毫无意义的. 然而, 当开发人员遇到其他子系统中发生的一些锁定问题时, 这让他们感到沮丧, 在修复现有问题之前, 他们无法测试代码是否存在锁定问题.

3. 检测需要一些时间来运行, 并且会创建比生产环境更多的同步点. lockdep 使用内部锁来保护锁定问题检测的数据结构, 这并不奇怪. 但是, 此锁定会
创建同步点, 并可能使某些问题难以检测(因为问题可能仅针对特定的偶数序列发生, 并且额外的同步点可能会阻止此类序列的发生)

针对此问题冯博群 Boqun Feng (Microsoft) 在 LPC-2022 上演示了 [Modularization for Lockdep](https://lpc.events/event/16/contributions/1210). 它提出将 LOCKDEP 进行模块化设计, 解耦为前端 - 后端的模式: 前端跟踪每个任务/上下文的当前持有的锁, 并向后端报告锁定对象, 后端维护锁依赖关系图并根据前端报告的内容检测锁定问题.

此外:

1.  对于与实际锁无关的等待和事件, 比如时间等待等机制, 如果无法完成, 最终也会导致死锁. 但是 Lockdep 只能通过分析锁的获取顺序来完成思索检测, 对于与实际锁无关的等待和事件, 无法识别和处理, 只能通过过模拟锁来完成.

2.  更糟糕的是, Lockdep 存在太多假阳性检测, 这可能阻止了本身更有价值的进一步检测.

3.  此外, 通过跟踪获取顺序, 它不能正确地处理读锁和交叉事件, 例如 wait_for_completion()/complete() 用于死锁检测. Lockdep 不再是实现这一目的的好工具.


## 10.2 Crossrelease Feature
-------

不仅是锁操作, 而且任何导致等待或旋转的操作都可能导致死锁, 除非它最终被某人“释放”. 这里重要的一点是, 等待或旋转必须由某人"释放". 但是很明显 LOCKDEP 无法检测到此类问题.

因此社区开发者 Byungchul Park 开发了交叉释放功能(Crossrelease Feature), 使得 LOCKDEP 不仅可以检查典型锁的依赖关系并检测死锁可能性, 还可以检查 lock_page()、wait_for_xxx() 等等待事件, 这些锁/等待可能在任何上下文中被释放.

这个特性最终经历了 v8 之后在 v4.14 被合入主线. 一开始的时候它报告很多有价值的隐藏死锁问题, 但随后也报告了诸多假阳性的死锁. 当然, 没有人喜欢 Lockdep 的假阳性报告, 因为它使 Lockdep 停止, 阻止报告更多的真实问题.

这造成越来越多地开发者直接关闭甚至不再使用 LOCKDEP 特性. 最终 Ingo 跟作者讨论后, 在 v4.15 回退了交叉释放功能. 参见 [locking/lockdep: Remove the cross-release locking checks](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e966eaeeb623f09975ef362c2866fae6f86844f9).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/08/07 | Byungchul Park <byungchul.park@lge.com> | [lockdep: Implement crossrelease feature](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ef0758dd0fd70b98b889af26e27f003656952db8) | 交叉释放功能(Crossrelease Feature). | v8 ☑✓ 4.14-rc1 | [LORE v8,0/14](https://lore.kernel.org/all/1502089981-21272-1-git-send-email-byungchul.park@lge.com) |
| 2017/12/13 | Ingo Molnar <mingo@kernel.org> | [locking/lockdep: Remove the cross-release locking checks](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e966eaeeb623f09975ef362c2866fae6f86844f9) | 回退交叉释放功能(Crossrelease Feature) | v1 ☑✓ 4.15-rc4 | [LORE](https://lore.kernel.org/all/20171213104617.7lffucjhaa6xb7lp@gmail.com) |


## 10.3 Dependency Tracker
-------


接着 2022 年, 原交叉释放功能(Crossrelease Feature)的作者 Byungchul Park 提出了新的解决方案 Dept(依赖跟踪器), 它专注于等待和事件本身, 跟踪等待和事件, 并在任何事件永远无法到达时报告它.

1.  以正确的方式处理读锁.

2.  适用于任何等待和事件也就是交叉事件.

3.  报告多次后仍可继续工作.

4.  提供简单直观的 api.

5.  做了依赖检查器应该做的事情.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/05/04 | Byungchul Park <byungchul.park@lge.com> | [DEPT(Dependency Tracker)](https://lore.kernel.org/all/1651652269-15342-1-git-send-email-byungchul.park@lge.com) | 一种死锁检测工具, 通过跟踪等待/事件而不是锁的获取顺序来检测死锁的可能性, 试图覆盖所有锁(spinlock, mutex, rwlock, seqlock, rwsem)以及同步机制(包括 wait_for_completion, PG_locked,  PG_writeback, swait/wakeup 等). | v6 ☐☑✓ | [RFC 00/14](https://lore.kernel.org/lkml/1643078204-12663-1-git-send-email-byungchul.park@lge.com)<br>*-*-*-*-*-*-*-* <br>[LORE v6,0/21](https://lore.kernel.org/all/1651652269-15342-1-git-send-email-byungchul.park@lge.com)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/23](https://lore.kernel.org/r/1673235231-30302-1-git-send-email-byungchul.park@lge.com)<br>*-*-*-*-*-*-*-* <br>[LORE v8,0/25](https://lore.kernel.org/r/1674782358-25542-1-git-send-email-max.byungchul.park@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v9,0/25](https://lore.kernel.org/r/1675154394-25598-1-git-send-email-max.byungchul.park@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v10,0/25](https://lore.kernel.org/r/20230626115700.13873-1-byungchul@sk.com)<br>*-*-*-*-*-*-*-* <br>[LORE v10,0/25](https://lore.kernel.org/r/20230703094752.79269-1-byungchul@sk.com)<br>*-*-*-*-*-*-*-* <br>[LORE v10,0/25](https://lore.kernel.org/r/20230821034637.34630-1-byungchul@sk.com) |
| 2022/09/15 | 刘顺 | [OSPP 2022: Add lite-lockdep as a lightweight lock validator](https://gitee.com/openeuler/kernel/issues/I5R8DS) | openEuler 开源之夏轻量级死锁检测特性. 参考了 [Low-overhead deadlock prediction](https://dl.acm.org/doi/10.1145/3377811.3380367), [PDF](https://web.cs.ucla.edu/~palsberg/paper/icse20.pdf) | ☐☑✓ | [gitee, PR](https://gitee.com/openeuler/kernel/pulls/112) |


# 11 优先级翻转
-------


## 11.1 优先级继承
-------

### 11.1.1 rt_mutex
-------

### 11.1.2 代理执行(proxy execution)
-------

代理执行(proxy execution) 被视为一种"更好"的优先级继承机制, 持有锁的小任务可以使用(继承)在同一互斥体上阻止的其他关键任务的调度上下文(属性)来运行它(避免优先级反转). 使用代理执行时, 被阻塞的关键任务不会像在常规的那样从运行队列中删除. 并且如果它是队列中优先级最高的任务, 则可以选择由调度程序以通常的方式运行. 于是, 当发生优先级反转的情况时, 持锁的任务将继承被阻塞的关键任务的上下文信息. 被阻塞的任务也会迁移到持有锁的任务的运行队列中, 从而将其利用率信息带过来, 这将导致 CPU 频率增加, 从而可以更好地帮助持锁任务尽快完成工作并释放锁.

代理执行(proxy execution) 并不是一个很新颖的概念, 它早就存在于学术界和邮件列表的讨论中.

早在 2010 年 [20th Euromicro Conference on Real-Time Systems (ECRTS2010)](http://www.artist-embedded.org/artist/Overview,1909.html), Peter Zijlstra 与 Thomas Gleixner 等进行 preemption rt 专题演讲, 讨论到优先级继承(priority inheritance) 时, 就提到了代理执行(proxy execution), 当时在 Doug Niehaus 的堪萨斯大学实时项目中就已经存在代理执行, 但不幸的是, 它缺乏 SMP 支持, 因此并没有得到广泛的推广, 但是这项技术得到了 Thomas 的赞誉. 参见当时 LWN 的报道 [Realtime Linux: academia v. reality](https://lwn.net/Articles/397422), 以及学术界的论文 [A Flexible Scheduling Framework Supporting Multiple Programming Models with Arbitrary Semantics in Linux](https://static.lwn.net/images/conf/rtlws11/papers/proc/p38.pdf)

Peter Zijlstra 在 [RT-Summit 2017](https://wiki.linuxfoundation.org/realtime/events/rt-summit2017/schedule) 时进行了主题为 [Proxy Execution (initial topic: "Migrate disable: What's wrong with that?")](https://wiki.linuxfoundation.org/realtime/events/rt-summit2017/proxy-execution) 的专题讨论, 详细介绍了 Proxy Execution 的思想. 其 Slides 参见 [proxy-execution_peter-zijlstra.pdf](https://wiki.linuxfoundation.org/_media/realtime/events/rt-summit2017/proxy-execution_peter-zijlstra.pdf).

但是彼时代理执行(proxy execution) 还停留在学术界和邮件列表以及会议的讨论中, 并没有真正在内核中被实现. 直到 2018 年 RedHat 的开发者 Juri Lelli 首次在内核实现了代理执行(Proxy execution)这一概念 [Towards implementing proxy execution, RFD/RFC, 0/8](https://lore.kernel.org/all/20181009092434.26221-1-juri.lelli@redhat.com), 即使只对 SCHED_DELINE 生效, 但是也是一次大胆的尝试. 对于 SCHED_DEADLINE 调度策略, 这转化为互斥体所有者在"内部"捐赠者(互斥服务员)带宽运行的可能性, 从而解决了一个长期存在的策略问题: 优先级提升的任务目前允许在运行时强制之外运行, 因为它们只继承了捐赠者的截止日期. 随后 Juri 在 LPC-2018 上进行了演示 [SCHED_DEADLINE desiderata and slightly crazy ideas](https://lpc.events/event/2/contributions/62). 随后 LWN 也进行了报道 [Proxy execution](https://lwn.net/Articles/793502). 由于存在诸多悬而未决的问题, 这个实现最终停留在了 RFC v2, 最后一版本的代码参见 [jlelli, github, deadline/proxy-rfc-v2-debug](https://github.com/jlelli/linux/commits/experimental/deadline/proxy-rfc-v2-debug).

随后 2020 年 5 月的 Power Management and Scheduling in the Linux Kernel summit (OSPM) 上, 来自 ARM 的开发者 Valentin Schneider 进一步在 ANDROID big.LITTLE 平台上对代理执行(proxy execution)进行了探索. 在开发 uclamp 的过程中, 他发现了 Utilization Inversion 这种与 Priority Inversion 极其相似的场景. 通过将代理执行与 uclamp 结合起来, 持有锁的小(或者不那么重要的)任务可以继承被阻塞的关键任务的同等的供给等资源, 以便它可以快速运行并释放锁, 从而防止优先级翻转. 参见 LWN 报道 [Utilization inversion and proxy execution](https://lwn.net/Articles/820575). 随后 Valentin 又在 LPC-2020 上演示了其代理执行(proxy execution) 的最新进展, 并基于 Juri 的实现, 发布了 RFC v3, 并扩展到了 CFS 和 RT 线程, 但是扩展性不足, 对 CONFIG_FAIR_GROUP_SCHED 的支持也欠佳. 参见 [Looking forward on proxy execution](https://lpc.events/event/7/contributions/758).

接着 2022 年来自 Google 的 Connor O'Brien 继续对代理执行(proxy execution) 进行了探索.

又来到 2023 年, Google 的 John Stultz 继续接受了这项工作. 参见 LWN 报道 [Addressing priority inversion with proxy execution](https://lwn.net/Articles/934114) 以及作者的 github 代码分支 [johnstultz-work/linux-dev/proxy-exec-v4-6.4-rc3](https://github.com/johnstultz-work/linux-dev/commits/proxy-exec-v4-6.4-rc3).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/10/09 | Juri Lelli <juri.lelli@redhat.com> | [Towards implementing proxy execution](https://lore.kernel.org/all/20181009092434.26221-1-juri.lelli@redhat.com) | TODO | v1 ☐☑✓ | [LORE v1,0/8](https://lore.kernel.org/all/20181009092434.26221-1-juri.lelli@redhat.com) |
| 2020/12/18 | ValenƟn Schneider <valentin.schneider@arm.com> | [Looking forward on proxy execution](https://lpc.events/event/7/contributions/758) | TODO | v1 ☐☑✓ | [GitLab, linux-arm RFC v3,00/08](https://gitlab.arm.com/linux-arm/linux-vs/-/tree/mainline/sched/proxy-rfc-v3/) |
| 2022/10/03 | Connor O'Brien <connoro@google.com> | [Reviving the Proxy Execution Series](https://lore.kernel.org/all/20221003214501.2050087-1-connoro@google.com) | TODO | v1 ☐☑✓ | [2022/10/03 LORE v1,0/11](https://lore.kernel.org/all/20221003214501.2050087-1-connoro@google.com)<br>*-*-*-*-*-*-*-* <br>[2023/03/20 LORE v2,0/12](https://lore.kernel.org/all/20230320233720.3488453-1-jstultz@google.com)<br>*-*-*-*-*-*-*-* <br>[2023/04/11 LORE v3,00/14](https://lore.kernel.org/all/20230411042511.1606592-1-jstultz@google.com) |
| 2023/06/01 | John Stultz <jstultz@google.com> | [Generalized Priority Inheritance via Proxy Execution](https://lore.kernel.org/all/20230601055846.2349566-1-jstultz@google.com) | TODO | v3 ☐☑✓ | [LORE v4,0/13](https://lore.kernel.org/all/20230601055846.2349566-1-jstultz@google.com) |


# 12 深入理解并行编程
-------


## 12.1 理论
-------

Paul McKenney's parallel programming book, [LWN](https://lwn.net/Articles/421425), [PerfBook](https://mirrors.edge.kernel.org/pub/linux/kernel/people/paulmck/perfbook/perfbook.html), [cgit, perfbook](https://git.kernel.org/pub/scm/linux/kernel/git/paulmck/perfbook.git/)

计算机体系结构基础, [第 10 章 并行编程基础](https://foxsen.github.io/archbase/并行编程基础.html)

[Is Parallel Programming Hard, And, If So, What Can You Do About It?](https://mirrors.edge.kernel.org/pub/linux/kernel/people/paulmck/perfbook/perfbook.html)

## 12.2 并行化框架
-------

[taskflow 的总结: 令人敬畏的并行计算](https://github.com/taskflow/awesome-parallel-computing)


### 12.2.1 Task/Function Flow
-------

[curated list of awesome open source workflow engines](https://github.com/meirwah/awesome-workflow-engines)

| 项目 | 描述 |
|:---:|:----:|
| [taskflow/taskflow](https://github.com/taskflow/taskflow) | Function Flow 并行化业界标杆, 犹他大学开发, 支持异构. [官网](https://taskflow.github.io). 论文<br>1. [Taskflow: A Lightweight Parallel and Heterogeneous Task Graph Computing System, TPDS 2021](https://taskflow.github.io/papers/tpds21-taskflow.pdf)<br>2. [Late Breaking Results: Efficient Timing Propagation with Simultaneous Structural and Pipeline Parallelisms, DAC 2022](https://tsung-wei-huang.github.io/papers/dac2022.pdf)<br>3. [Pipeflow: An Efficient Task-Parallel Pipeline Programming Framework using Modern C++, HPDC 2022](https://arxiv.org/abs/2202.00717)<br>4. [From RTL to CUDA: A GPU Acceleration Flow for RTL Simulation with Batch Stimulus, ICPP 2022](https://icpp22.gitlabpages.inria.fr/prog/) |
| [ChunelFeng/CGraph](https://github.com/ChunelFeng/CGraph) | ChunelFeng 的图化调度并行框架, 轻量, 快捷, 暂不支持异构 |
| [AthrunArthur/functionflow](https://github.com/AthrunArthur/functionflow) | 基于 C++11 的 FunctionFlow 并行编程库. |


### 12.2.2 WorkQueue
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2023/05/18 | Tejun Heo <tj@kernel.org> | [workqueue: Improve unbound workqueue execution locality](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=523a301e66afd1ea9856660bcf3cee3a7c84c6dd) | 重构 workqueue, 使其感知具有多个 L3 缓存的系统受益.<br>"默认行为是根据最后一级缓存边界进行软仿射. 从给定 LLC 排队的工作项由在同一 LLC 上运行的工作线程执行, 但工作线程可以跨缓存边界移动, 因为计划程序认为合适. 在具有多个 L3 缓存的机器上, 随着小芯片设计变得越来越流行, 这提高了缓存局部性, 同时不会过多地损害工作保护.<br>未绑定的工作队列现在在执行相关性方面也更加灵活. 支持不同级别的相关性作用域, 并且可以动态修改默认和按工作队列的相关性设置. 这应该有助于解决最近在非对称 ARM CPU 中观察到的次优行为. 参见 [Linux 6.6 WQ Change May Help Out AMD CPUs & Other Systems With Multiple L3 Caches](https://www.phoronix.com/news/Linux-6.6-Workqueue) | v1 ☐☑✓ [6.6-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bd30fe6a7d9b72e73c5ac9109cbc3066dde08034) | [LORE](https://lore.kernel.org/all/20230519001709.2563-1-tj@kernel.org) |


# 13 Windows NT Synchronization Primitive Driver
-------




| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2024/02/14 | Elizabeth Figura <zfigura@codeweavers.com> | [NT synchronization primitive driver](https://lore.kernel.org/all/20240214233645.9273-1-zfigura@codeweavers.com) | 参见 LWN 报道 [Windows NT synchronization primitives for Linux](https://lwn.net/Articles/961884) 以及 phoronix 报道 [Windows NT Synchronization Primitive Driver Updated For The Linux Kernel](https://www.phoronix.com/news/NTSYNC-Linux-Update-February)
 | v1 ☐☑✓ | [2024/02/14 LORE v1,00/31](https://lore.kernel.org/all/20240214233645.9273-1-zfigura@codeweavers.com)<br>*-*-*-*-*-*-*-* <br>[2024/02/19 LORE v2,00/31](https://lore.kernel.org/lkml/20240219223833.95710-1-zfigura@codeweavers.com) |


<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
