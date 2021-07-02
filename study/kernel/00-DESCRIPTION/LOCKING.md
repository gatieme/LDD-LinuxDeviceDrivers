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


## 1.1 CAS LOCK
-------


## 1.2 ticket LOCK
-------

https://lwn.net/Articles/267968


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/11/01 | Nick Piggin <npiggin@suse.de> | [ticket spinlocks for x86](https://lore.kernel.org/patchwork/cover/95892) | X86 架构 ticket spinlocks 的实现. | v1 ☑ 2.6.25-rc1(部分合入) | [PatchWork RFC](https://lore.kernel.org/patchwork/cover/85789)<br>*-*-*-*-*-*-*-* <br>[PatchWork](https://lore.kernel.org/patchwork/cover/95892), [PatchWork](https://lore.kernel.org/patchwork/cover/95894) |


[Linux中的spinlock机制[一] - CAS和ticket spinlock](https://zhuanlan.zhihu.com/p/80727111)


## 1.3 MCS lock
-------


spinlock 的值出现变化时, 所有试图获取这个 spinlock 的 CPU 都需要读取内存, 刷新自己对应的 cache line, 而最终只有一个 CPU 可以获得锁, 也只有它的刷新才是有意义的. 锁的争抢越激烈(试图获取锁的CPU数目越多), 无谓的开销也就越大.

如果在 ticket spinlock 的基础上进行一定的修改, 让每个 CPU 不再是等待同一个 spinlock 变量, 而是基于各自不同的 per-CPU 的变量进行等待, 那么每个 CPU 平时只需要查询自己对应的这个变量所在的本地 cache line, 仅在这个变量发生变化的时候, 才需要读取内存和刷新这条 cache line, 这样就可以解决上述的这个问题.

要实现类似这样的 spinlock的 「分身」, 其中的一种方法就是使用 MCS lock. 试图获取一个 spinlock 的每个CPU, 都有一份自己的 MCS lock.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/08/28 | Nick Piggin <npiggin@suse.de> | [queueing spinlocks?](https://lore.kernel.org/patchwork/cover/127444) | X86 架构 qspinlocks 的实现. | RFC ☐ | [PatchWork RFC](https://lore.kernel.org/patchwork/cover/127444) |
| 2015/04/07 | Waiman Long <Waiman.Long@hp.com> | [qspinlock: a 4-byte queue spinlock with PV support](https://lore.kernel.org/patchwork/cover/558505) | PV SPINLOCK | v15 ☑ 4.2-rc1 | [PatchWork v15](https://lore.kernel.org/patchwork/cover/558505) |
| 2014/01/21 | Tim Chen <tim.c.chen@linux.intel.com> | [MCS Lock: MCS lock code cleanup and optimizations](https://lore.kernel.org/patchwork/cover/435770) | MCS LOCK 优化 | v9 ☑ 4.2-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/435770) |
| 2014/02/10 | Peter Zijlstra <peterz@infradead.org> | [locking/core patches](https://lore.kernel.org/patchwork/cover/440565) | PV SPINLOCK | v1 ☑ 4.2-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/440565) |



## 1.4 PV_SPINLOCK
-------

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


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/05/14 | Alex Kogan <alex.kogan@oracle.com> | [Add NUMA-awareness to qspinlock](https://lore.kernel.org/patchwork/cover/1428910) | NUMA 感知的 spinlock, 基于 CNA. | v15 ☐ | [PatchWork v15](https://lore.kernel.org/patchwork/cover/1428910) |


# 2 RWSEM
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/11/21 | Waiman Long <longman@redhat.com> | [locking/rwsem: Rework reader optimistic spinning](https://lore.kernel.org/patchwork/cover/1342950) | 当读者的评论部分很短, 周围没有那么多读者时, 读者乐观旋转(osq_lock)是有帮助的. 它还提高了读者获得锁的机会, 因为写入器乐观旋转对写入器的好处远远大于读者. 由于提交d3681e269fff ("locking/rwsem: Wake up almost all reader in wait queue"), 所有等待的reader都会被唤醒, 这样它们就都能获得读锁并并行运行. 当竞争的读者数量很大时, 允许读者乐观自旋很可能会导致读者碎片, 多个较小的读者组可以以顺序的方式(由写入器分隔)获得读锁. 这降低了读者的并行性. 解决这个缺点的一种可能方法是限制能够进行乐观旋转的读者的数量(最好是一个). 这些读者作为等待队列中所有等待的读者的代表, 因为一旦获得锁, 它们将唤醒所有等待的读者.  | v2 ☐ | [PatchWork v2,0/5](https://lore.kernel.org/patchwork/cover/1342950) |

<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
