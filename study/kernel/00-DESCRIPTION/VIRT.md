---

title: 虚拟化 & KVM 子系统
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

本作品采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"> 知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议 </a> 进行许可, 转载请注明出处, 谢谢合作

<a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a>

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 鄙人在此谢谢啦

** 转载请务必注明出处, 谢谢, 不胜感激 **

<br>

| 日期 | 作者 | GitHub| CSDN | BLOG |
| ------- |:-------:|:-------:|:-------:|:-------:|
| 2021-02-15 | [成坚 - gatieme](https://kernel.blog.csdn.net) | [`AderXCoding/system/tools/fzf`](https://github.com/gatieme/AderXCoding/tree/master/system/tools/fzf) | [使用模糊搜索神器 FZF 来提升办公体验](https://blog.csdn.net/gatieme/article/details/113828826) | [Using FZF to Improve Productivit](https://oskernellab.com/2021/02/15/2021/0215-0001-Using_FZF_to_Improve_Productivity)|


<br>

2   ** 虚拟化子系统 **
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***




# 1 VIRT LOCK
-------

## 1.1 PV_SPINLOCK
-------

spinlock 在非虚拟化的环境下, 它是可以认为 CPU 不会被抢占的, 所以 A 拿锁干活, B 死等 A, A 干完自己的活, 就释放了, 中间不会被调度.

但是在虚拟化下, CPU 对应到 vcpu, 每个 vcpu 跟之前裸机上的进程调度一样, 所以 A 拿锁干活, 并不一定不会被抢占, 很有可能被调度走了, 因为 cpu 这时候还不知道 vcpu 在干嘛. B 死等 A, 但是 A 被调度了, 运行了 C, C 也要死等 A, 在一些设计不够好的系统里面, 这样就会变得很糟糕.


为了保证 spinlock unlock 的公平性, 有一种队列的 spinlock, ticketlock,  http://www.ibm.com/developerworks/cn/linux/l-cn-spinlock / 这篇文章介绍的非常详细, 总之根据 next, own, 来判断是否到自己了. 这样一种机制在裸机上是可以解决公平的问题, 但是放到虚拟化环境里, 它会使问题变得更糟. C 必须等到 B 完成才可以, 如果中间 B 被调度了, 又开始循环了, 当然更糟的定义也是相对的, 如果 vcpu 的调度机制能够 vcpu 正在拿锁的话, 会怎样？


jeremy 很早就写了一个 pv ticketlock, 原理大概就是 vcpu 在拿锁了一段时间, 会放弃 cpu, 并 blocked, unlocked 的时候会被唤醒, 这个针对 PV 制定的优化, 在 vcpu 拿不到锁的场景下, 并没有任何的性能损耗, 并且能够解决之前的问题, 但是当运行 native linux 的时候, 就会有性能损耗, 所以当时在 config 里面添加了一个编译选项 CONFIG_PARAVIRT_SPINLOCK, 话说我们的系统里面, 这个是没打开的啊, 后面要再好好评估下



之后, 这个 patch 进行了改良, 在原有 native linux 的 ticketlock 的基础, 增加了一种模式, 通过检测 cpu 是否 spinned 一段时间, 判断是否要进入 slow path, 之前的 fast path 的逻辑和原来保持不变, 进入 slowpath 后, 会在 ticketlock 里面置位, 并 block vcpu, 等 unlock 的时候, 这个位会被 clear, 因为占用了一个位, 所以能用的 ticket 少了一半.



这个方案在一些硬件 (XEON x3450) 上进行各种 benchmark 测试后, 结论是不再有任何的性能损耗.



好吧, 说了这么多理论性的东西, 再来说下, 我们实际遇到的问题. 很早以前经常有 windows 用户的工单投诉, 说自己的 vm 里面 cpu 没有怎么使用, 为什么 cpu 显示百分之百.

由于是 windows 系统, 加上我是小白, 很难给出一些技术细节上的分析, 只能通过简单滴一些测试实验进行调查.



最后的结果, 就是 windows 很多核的情况下, 比如 12、16,  在一个稍微有点 load 的物理机上面, 跑一些 cpu 压力, 就很容易出现 cpu 百分百的问题, 后面降 core 之后, 情况有所缓解. 后面大致的分析结果是, windows 里面很多操作是用到 spinlock 的, 当一个 core 拿到锁, 事情没有做完, 被调度了, 这时其他的 core 也需要拿锁, 当 core 越来越多的时候, 情况就越来越糟, 最后看上去就大家都很忙, 但实际什么事情也没做.



目前来看, 已经有一种较为成熟的软件方法来解决类似问题, 期待后续是否会有硬件的一些特性来支持, 或许已经有了.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2013/08/09 | Raghavendra K T <raghavendra.kt@linux.vnet.ibm.com> | [Paravirtualized ticket spinlocks](https://lore.kernel.org/patchwork/cover/398912) | PV_SPINLOCK 的 ticket lock 实现. | v13 ☑ 3.12-rc1 | [PatchWork v13](https://lore.kernel.org/patchwork/cover/398912) |
| 2015/04/07 | Waiman Long <Waiman.Long@hp.com> | [qspinlock: a 4-byte queue spinlock with PV support](https://lore.kernel.org/patchwork/cover/558505) | PV SPINLOCK | v15 ☑ 4.2-rc1 | [PatchWork v15](https://lore.kernel.org/patchwork/cover/558505) |
| 2015/11/10 | Waiman Long <Waiman.Long@hpe.com> | [locking/qspinlock: Enhance pvqspinlock performance](https://lore.kernel.org/patchwork/cover/616398) | PV SPINLOCK | v10 ☑ 4.5-rc1 | [PatchWork v5](https://lore.kernel.org/patchwork/cover/588106)<br>*-*-*-*-*-*-*-* <br>[PatchWork v10](https://lore.kernel.org/patchwork/cover/616398) |
| 2018/10/08 | Raghavendra K T <raghavendra.kt@linux.vnet.ibm.com> | [Enable PV qspinlock for Hyper-V](https://lore.kernel.org/patchwork/cover/996494) | Hyper-V 的 PV spiclock 实现. | v2 ☑ 4.20-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/cover/996494) |
| 2019/10/23 | Zhenzhong Duan <zhenzhong.duan@oracle.com> | [Add a unified parameter"nopvspin"](https://lore.kernel.org/patchwork/cover/1143398) | PV SPINLOCK | v8 ☑ 5.9-rc1 | [PatchWork v8](https://lore.kernel.org/patchwork/cover/1143398) |



# 2 VDPA
-------

[Introduction to vDPA kernel framework](https://www.redhat.com/zh/blog/introduction-vdpa-kernel-framework)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/12/22 | Xie Yongji <xieyongji@bytedance.com> | [Introduce VDUSE - vDPA Device in Userspace](https://lore.kernel.org/patchwork/cover/398912) | 本系列介绍了一个框架, 可用于在用户空间程序中实现 vDPA 设备. 该工作包括两部分: 控制路径转发和数据路径卸载.<br>1. 在控制路径中, VDUSE 驱动程序将利用消息机制将配置操作从 vdpa 总线驱动程序转发到用户空间. 用户空间可以使用 read()/write() 来接收 / 回复这些控制消息.<br>2. 在数据路径中, 核心是将 dma 缓冲区映射到 VDUSE 守护进程的地址空间, 这可以根据 vdpa 设备所连接的 vdpa 总线以不同的方式实现.<br> 在 virtio-vdpa 的情况下, 我们实现了一个基于 mmu 的片上 IOMMU 驱动程序, 该驱动程序带有弹跳缓冲机制. 在 vhost-vdpa 的情况下, dma 缓冲区驻留在用户空间内存区域, 可以通过传输 shmfd 共享给 VDUSE 用户空间进程. | v2 ☐ | [PatchWork RFC,v2,00/13](https://patchwork.kernel.org/project/linux-mm/cover/20201222145221.711-1-xieyongji@bytedance.com) |


# 3 IPA
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/12/22 | Xie Yongji <xieyongji@bytedance.com> | [kvm: arm64: Dynamic IPA and 52bit IPA](https://lore.kernel.org/patchwork/cover/992057) | arm/arm64 上 VM 的物理地址空间大小 (IPA 大小) 被限制为 40 位的静态限制. 这组补丁增加了对使用特定于 VM 的 IPA 大小的支持, 允许使用主机支持的大小 (基于主机内核配置和 CPU 支持). | v6 ☐ | [PatchWork v6,00/18](https://lore.kernel.org/patchwork/cover/992057) |



# 4 LAPIC
-------

## 4.1 VMExit less
-------

[Using perf-probe To Analyse VM Exits](https://www.codeblueprint.co.uk/2017/02/26/using-perf-probe-to-analyse-vmexits.html)



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/07/23 | Wanpeng Li <kernellwp@gmail.com>/<wanpengli@tencent.com> | [KVM: X86: Implement Exit-less IPIs support](https://patchwork.kernel.org/project/kvm/cover/1532327996-17619-1-git-send-email-wanpengli@tencent.com) | 对于 xAPIC/x2APIC 物理模式, 使用 hypercall 通过一个 vmexit 发送 ip, 而不是逐个发送, 对于 x2APIC 集群模式, 每个集群使用一个 vmexit. 在 qemu 中启用中断重映射时, Intel guest 可以进入 x2apic 集群模式, 而最新的 AMD EPYC 仍然只支持 xapic 模式, 而 xapic 模式可以通过无 exit IPIs 得到很大的改进. 这个补丁集允许 guest 发送多播 ip, 64 位模式下每个超级调用最多有 128 个目的地, 32 位模式下每个超级调用最多有 64 个 vcpu. | v5 ☑ 4.19-rc1 | [PatchWork v5,0/6](https://patchwork.kernel.org/project/kvm/cover/1562376411-3533-1-git-send-email-wanpengli@tencent.com) |
| 2019/07/06 | Wanpeng Li <kernellwp@gmail.com>/<wanpengli@tencent.com> | [KVM: LAPIC: Implement Exitless Timer](https://patchwork.kernel.org/project/kvm/cover/1562376411-3533-1-git-send-email-wanpengli@tencent.com) | KVM 实现 pi_inject_timer. 由于模拟的 lapic 定时器以及虚拟中断等总是在在 VCPU 驻留的 PCPU 上触发, 这会造成 pcpu 上的 vcpu 退出虚拟化 (VMExit), 而执行 VMExit 的开销是非常大的. 这组补丁优化了虚拟 timer 中断的模拟流程, 把虚拟集中放到某些固定的 pcpu 上, 这样其他 pcpu 上的 vcpu 就不需要退出了. | v7 ☑ 5.3-rc1 | [PatchWork v7,0/2](https://patchwork.kernel.org/project/kvm/cover/1562376411-3533-1-git-send-email-wanpengli@tencent.com) |



# 5 VIRTIO
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/10/05 | David Hildenbrand <david@redhat.com> | [proc/vmcore: sanitize access to virtio-mem memory](https://patchwork.kernel.org/project/linux-mm/cover/20211005121430.30136-1-david@redhat.com) | NA | v2 ☑ 4.19-rc1 | [PatchWork v2,0/9](https://patchwork.kernel.org/project/linux-mm/cover/20211005121430.30136-1-david@redhat.com) |
| 2021/11/11 | Chao Peng <chao.p.peng@linux.intel.com> | [KVM: mm: fd-based approach for supporting KVM guest private memory](https://patchwork.kernel.org/project/linux-mm/cover/20211111141352.26311-1-chao.p.peng@linux.intel.com) | [Private memory for KVM guests](https://lwn.net/Articles/890224) | RFC ☐ | [LORE RFC,0/6](https://patchwork.kernel.org/project/linux-mm/cover/20211111141352.26311-1-chao.p.peng@linux.intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,00/13](https://lore.kernel.org/lkml/20220310140911.50924-1-chao.p.peng@linux.intel.com) |



# 6 Guest Memory
-------


## 6.1 Guest page hinting
-------

[Guest page hinting](https://lwn.net/Articles/198380)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/03/06 | Nitesh Narayan Lal <nitesh@redhat.com> | [mm: Support for page hinting](https://lkml.org/lkml/2019/7/10/742) | NA | v11 ☐  | [LWN RFC v9,0/6](https://lwn.net/Articles/782470), [LKML v11,0/2](https://lkml.org/lkml/2019/7/10/742) |
| 2019/07/24 | Alexander Duyck <alexander.duyck-AT-gmail.com> | [mm / virtio: Provide support for page hinting](https://lwn.net/Articles/794540) | NA | v2 ☐ | [LKML v2,0/5](https://lwn.net/Articles/794540) |

## 6.2 TDX and guest memory
-------

[Intel TDX Guest Attestation Support Merged For Linux 6.2](https://www.phoronix.com/news/Intel-TDX-Guest-In-Linux-6.2)

如果 TDX 开启, Host 内核不得允许对 TD 专用内存进行任何写入. 这一要求与 KVM 设计相冲突: KVM 希望 Guest 内存映射到 Host 的用户空间 (例如 QEMU). 这时如果 TDX 主机访问受 TDX 保护的客户内存, 可能会发生 Machine Check, 从而进一步导致运行中的 Host 系统 PANIC, 这对于多租户配置来说是非常糟糕的. 主机访问包括来自 KVM 用户空间 (如 QEMU) 的访问.



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/10/26 | Huang, Kai <kai.huang@intel.com> | [TDX host kernel support](https://patchwork.kernel.org/project/linux-mm/cover/cover.1666824663.git.kai.huang@intel.com/) | 689199 | v6 ☐☑ | [LORE v6,0/21](https://lore.kernel.org/r/cover.1666824663.git.kai.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/20](https://lore.kernel.org/r/cover.1668988357.git.kai.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v8,0/16](https://lore.kernel.org/r/cover.1670566861.git.kai.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v10,0/16](https://lore.kernel.org/all/cover.1678111292.git.kai.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v12,0/22](https://lore.kernel.org/r/cover.1687784645.git.kai.huang@intel.com) |
| 2021/04/16 | Kirill A. Shutemov <kirill.shutemov@linux.intel.com> | [TDX and guest memory unmapping](https://lore.kernel.org/all/20210416154106.23721-1-kirill.shutemov@linux.intel.com) | 作者有意不考虑 TDX, 并尝试找到一种通用的方法来从主机用户空间取消 KVM 客户内存的映射. 借助页表中 PG_hwpoison 的页和 SWP_HWPOISON 的交换项:<br>1. 如果应用程序接触到 SWP_HWPOISON 映射的页面, 它将得到 SIGBUS.<br>2. 当我们将页面设置为 TD-private 时, 我们可以将页面标记为有毒的, 并用 SWP_HWPOISON 替换映射该页的所有 pte. | v1 ☐☑✓ | [LORE v1,0/13](https://lore.kernel.org/all/20210416154106.23721-1-kirill.shutemov@linux.intel.com) |
| 2021/08/23 | Sean Christopherson <seanjc@google.com> | [KVM: mm: fd-based approach for supporting KVM guest private memory](https://lore.kernel.org/all/20210824005248.200037-1-seanjc@google.com) | 这是 Kirill 的 RFC 的延续, 通过在 `struct page` 级别跟踪客户内存来支持 TDX 客户私有内存. 这个提案是由 Andy Lutomirksi 对通过 `struct page` 进行跟踪的担忧引发的几次离线讨论的结果. | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20210824005248.200037-1-seanjc@google.com) |
| 2022/10/25 | Chao Peng <chao.p.peng@linux.intel.com> | [KVM: mm: fd-based approach for supporting KVM](https://patchwork.kernel.org/project/linux-mm/cover/20221025151344.3784230-1-chao.p.peng@linux.intel.com/) | 用于机密计算场景 (如 [Intel TDX](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-trust-domain-extensions.html)) 的 KVM 客户私有内存. 本系列通过引入新的 mm 和 KVM 接口来解决 KVM 用户空间导致的崩溃问题, 因此 KVM 用户空间仍然可以通过基于 fd 的方法管理来宾内存, 但它永远不能访问来宾内存内容. | v9 ☐☑ | [LORE v9,0/8](https://lore.kernel.org/r/20221025151344.3784230-1-chao.p.peng@linux.intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v10,0/9](https://lore.kernel.org/r/20221202061347.1070246-1-chao.p.peng@linux.intel.com) |


## 6.3 Secure Nested Paging
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/12/14 | Michael Roth <michael.roth@amd.com> | [Add AMD Secure Nested Paging (SEV-SNP) Hypervisor Support](https://patchwork.kernel.org/project/linux-mm/cover/20221214194056.161492-1-michael.roth@amd.com/) | 704568 | v7 ☐☑ | [LORE v7,0/64](https://lore.kernel.org/r/20221214194056.161492-1-michael.roth@amd.com) |


# 7 ANDROID
-------


## 7.1 AnBox
-------


Anbox 可让你在任何 GNU/Linux 操作系统上运行 Android 应用程序.

Anbox 使用 Linux 命名空间 (user, pid, uts, net, mount, ipc) 在容器中运行完整的 Android 系统, 并在任何基于 GNU Linux 的平台上提供 Android 应用程序.

容器内的 Android 无法直接访问任何硬件. 所有硬件访问都经过主机上的 anbox 进程. 我们正在重用 Android 中实现的基于 QEMU 的模拟器, 用于 Open GL ES 加速渲染, 容器内的 Android 系统使用不同的管道与主机系统进行通信, 并通过这些管道发送所有硬件访问命令.

## 7.2 Waydroid
-------

[Android apps on Linux with Waydroid](https://lwn.net/Articles/901459)



# 8 MEMORY
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/08/23 | Yosry Ahmed <yosryahmed@google.com> | [KVM: mm: count KVM mmu usage in memory stats](https://patchwork.kernel.org/project/linux-mm/cover/20220823004639.2387269-1-yosryahmed@google.com/) | 669960 | v7 ☐☑ | [LORE v7,0/4](https://lore.kernel.org/r/20220823004639.2387269-1-yosryahmed@google.com) |
| 2022/09/15 | Chao Peng <chao.p.peng@linux.intel.com> | [KVM: mm: fd-based approach for supporting KVM](https://patchwork.kernel.org/project/linux-mm/cover/20220915142913.2213336-1-chao.p.peng@linux.intel.com/) | 677360 | v8 ☐☑ | [LORE v8,0/8](https://lore.kernel.org/r/20220915142913.2213336-1-chao.p.peng@linux.intel.com) |
| 2023/02/16 | Yu Zhao <yuzhao@google.com> | [mm/kvm: lockless accessed bit harvest](https://lore.kernel.org/all/20230217041230.2417228-1-yuzhao@google.com) | [Linux Patches Improve KVM Guest Performance For Hosts Under Memory Pressure](https://www.phoronix.com/news/Linux-KVM-Lockless-Accessed-Bit) | v1 ☐☑✓ | [LORE v1,0/5](https://lore.kernel.org/all/20230217041230.2417228-1-yuzhao@google.com) |


# 9 Migrate
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/07/07 | Peter Xu <peterx@redhat.com> | [migration: Postcopy Preemption](https://lore.kernel.org/all/20220707185342.26794-1-peterx@redhat.com) | TODO | v9 ☐☑✓ | [LORE v7,00/14](https://lore.kernel.org/all/20220524221151.18225-1-peterx@redhat.com)<br>*-*-*-*-*-*-*-*<br>[LORE v8,00/15](https://lore.kernel.org/all/20220622204920.79061-1-peterx@redhat.com)<br>*-*-*-*-*-*-*-*<br>[LORE v9,00/14](https://lore.kernel.org/all/20220707185342.26794-1-peterx@redhat.com) |

# 10 Nested Hypervisor
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2023/01/02 | Jinank Jain <jinankjain@linux.microsoft.com> | [x86/hyperv: Add support for detecting nested hypervisor](https://lore.kernel.org/all/8e3e7112806e81d2292a66a56fe547162754ecea.1672639707.git.jinankjain@linux.microsoft.com) | [Microsoft Hyper-V Nested Hypervisor Support Comes For Linux 6.3](https://www.phoronix.com/news/Linux-6.3-Nested-Hyper-V) | v10 ☐☑✓ | [LORE v10,0/5](https://lore.kernel.org/all/8e3e7112806e81d2292a66a56fe547162754ecea.1672639707.git.jinankjain@linux.microsoft.com) |

# 11 Hypervisor
-------


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2023/04/24 | Elliot Berman <quic_eberman@quicinc.com> | [Drivers for Gunyah hypervisor](https://lore.kernel.org/all/20230424231558.70911-1-quic_eberman@quicinc.com) | [Qualcomm Continues Working To Upstream Gunyah Hypervisor Support In Linux](https://www.phoronix.com/news/Qualcomm-Gunyah-Linux-v12) | v12 ☐☑✓ | [LORE v12,0/25](https://lore.kernel.org/all/20230424231558.70911-1-quic_eberman@quicinc.com) |
| 2023/05/12 | Yi-De Wu <yi-de.wu@mediatek.com> | [GenieZone hypervisor drivers](https://lore.kernel.org/all/20230512080405.12043-1-yi-de.wu@mediatek.com) | TODO | v3 ☐☑✓ | [LORE v1,0/6](https://lore.kernel.org/lkml/20230413090735.4182-1-yi-de.wu@mediatek.com)<br>*-*-*-*-*-*-*-*<br>[LORE v2,0/7](https://lore.kernel.org/lkml/20230428103622.18291-1-yi-de.wu@mediatek.com)<br>*-*-*-*-*-*-*-*<br>[LORE v3,0/7](https://lore.kernel.org/all/20230512080405.12043-1-yi-de.wu@mediatek.com) |
| 2024/02/26 | Lai Jiangshan <jiangshanlai@gmail.com> | [KVM: x86/PVM: Introduce a new hypervisor](https://lore.kernel.org/all/20240226143630.33643-1-jiangshanlai@gmail.com) | [PVM Virtualization Framework Proposed For Linux - Built Atop The KVM Hypervisor](https://www.phoronix.com/news/PVM-Hypervisor-Linux-RFC) | v1 ☐☑✓ | [LORE v1,0/73](https://lore.kernel.org/all/20240226143630.33643-1-jiangshanlai@gmail.com) |
| 2024/06/13 | Alexey Makhalov <alexey.makhalov@broadcom.com> | [VMware hypercalls enhancements](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=57b7b6acb41b51087ceb40c562efe392ec8c9677) | Broadcom 一直在开发适用于 Linux 内核的 VMware Hypercall API. 这个 PATCHSET 提供了 "vmware_hyperscall" 是一系列新的功能, 供 VMware 客户机代码和虚拟设备驱动程序以独立于体系结构的方式使用. 参见 phoronix 报道 [VMware Hypercall API To Likely Land In Linux 6.11](https://www.phoronix.com/news/VMware-Hypercall-API-Linux-6.11) 和 [VMware Hypercall API Makes It Into Linux 6.11 For Basis To Allow Confidential Computing](https://www.phoronix.com/news/VMware-Hypercall-Linux-6.11). | v11 ☐☑✓ v6.11-rc1 | [LORE v11,0/8](https://lore.kernel.org/all/20240613191650.9913-1-alexey.makhalov@broadcom.com) |


# 12 HaltPolling
-------


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2024/02/15 | Mihai Carabas <mihai.carabas@oracle.com> | [Enable haltpoll for arm64](https://lore.kernel.org/all/1707982910-27680-1-git-send-email-mihai.carabas@oracle.com) | 这个补丁集允许在arm64上使用haltpoll-governer. 通过减少IPC延迟. openEuler 有类似的方案, 参见 [最佳实践](https://docs.openeuler.org/zh/docs/20.09/docs/Virtualization/最佳实践.html#halt-polling) 以及 [漫话Linux之“躺平”: IDLE 子系统](https://mp.weixin.qq.com/s?__biz=Mzg2OTc0ODAzMw==&mid=2247502695&idx=1&sn=60f6132f5f4f1d0f515cad74e83b3c63&source=41). | v4 ☐☑✓ | [LORE](https://lore.kernel.org/all/1707982910-27680-1-git-send-email-mihai.carabas@oracle.com) |


# 13 能效
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2023/03/30 | David Dai <davidai@google.com> | [Improve VM DVFS and task placement behavior](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=4fd06a5358e0d888d1bf23d274971ea7d1f45aad) | 这组补丁的主要目的是改善虚拟机(VM)中的动态电压频率调节(DVFS)和任务调度行为, 使其表现接近于直接在主机上运行. 旨在解决虚拟机中工作负载的任务调度和 DVFS 行为不如直接在主机上运行时那么理想的问题. 具体来说, 当工作负载在虚拟机中运行时, 其任务调度和频率调节策略不如直接在主机上运行时有效, 导致性能和功耗表现不佳. 通过改进这些方面, 可以显著提升虚拟环境中工作负载的性能和能效.<br>1. 准确的负载跟踪: 让虚拟机内核能够更准确地跟踪主机 CPU 的架构类型、频率等信息, 以进行更精确的负载评估.<br>2. 共享 vCPU 运行队列利用率信息: 将虚拟 CPU(vCPU)的运行队列利用率信息共享给主机, 主机能够更好地了解虚拟机内部的工作负载情况, 并据此调整调度策略.<br>3. 频率调节优化(改进 DVFS 行为): 确保虚拟机中的工作负载能够享受到与主机相同级别的频率调节优化, 从而提高性能和能效.<br>参见  [LPC 2022 演讲 "CPUfreq/sched and VM guest workload problems](https://lpc.events/event/16/contributions/1195), 以及 phoronix 报道 [phoronix, 2024/11/24, Virtual CPUFreq Driver Coming With Linux 6.13 For Better Power/Performance Within VMs](https://www.phoronix.com/news/Linux-6.13-Virtual-CPUFreq) 和 [phoronix, 2023/03/31, Google Posts KVM-CPUFreq Driver To Dramatically Boost VM Performance, Power Efficiency](https://www.phoronix.com/news/KVM-CPUFreq-RFC-Patches). | v1 ☐☑✓ v6.13-rc1 | [2023/03/30, LORE v1,0/6](https://lore.kernel.org/all/20230330224348.1006691-1-davidai@google.com)<br>*-*-*-*-*-*-*-* <br>[2023/03/30, LORE v2,0/6](https://lore.kernel.org/all/20230331014356.1033759-1-davidai@google.com)<br>*-*-*-*-*-*-*-* <br>[2023/07/31, LORE v3,0/2](https://lore.kernel.org/all/20230731174613.4133167-1-davidai@google.com)<br>*-*-*-*-*-*-*-* <br>[2023/11/10, LORE v4,0/2](https://lore.kernel.org/all/20231111014933.1934562-1-davidai@google.com)<br>*-*-*-*-*-*-*-* <br>[2024/01/26, LORE v5,0/2](https://lore.kernel.org/all/20240127004321.1902477-1-davidai@google.com)<br>*-*-*-*-*-*-*-* <br>[2024/05/20, LORE V6,0/2](https://lore.kernel.org/all/20240521043102.2786284-1-davidai@google.com)<br>*-*-*-*-*-*-*-* <br>[2024/09/18, LORE v7,0/2](https://lore.kernel.org/all/20240919000837.1004642-1-davidai@google.com) |



# 14 Multi-Kernel Architecture
-------

这个比较想法比较老了, 早在 2002 年, Larry McVoy 就在邮件列表中提出了用于 Linux 的 cache-coherent clusters, 在一台机器上运行 multiple "OSlets", 参见 [LKML, 2002/06/19](https://lwn.net/Articles/4536).

2013 年, Virginia Tech 的研究团队首次在 64 位 x86 机器上支持 Linux 内核的多个实例并支持 4 个以上内核同时运行. 参见 [Popcorn Linux Distributed Thread Execution](https://lore.kernel.org/all/cover.1588127445.git.javier.malave@narfindustries.com).

2025 年, Multikernel Technologies Inc 向社区提交了 [Multikernel Goes Open Source: Community-First Innovation](https://multikernel.io/2025/09/18/multikernel-goes-open-source) 方案, 几天后, 字节立即公布了 [parker: PARtitioned KERnel](https://lore.kernel.org/all/20250923153146.365015-1-fam.zheng@bytedance.com) 方案. 两者相比较, Parker 更倾向于静态分区方案, 启动时旧分配好了 CPU/内存/设备, 运行中不再变化. 这样简化了实现和隔离, 但是可能导致资源利用率不高.



| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2020/04/29 | Javier Malave <javier.malave@narfindustries.com> | [Popcorn Linux Distributed Thread Execution](https://lore.kernel.org/all/cover.1588127445.git.javier.malave@narfindustries.com) | 在 Linux 内核中引入 **Popcorn Linux 分布式线程执行** 支持. Popcorn Linux 是由 Virginia Tech 的研究团队开发, 基于 David Katz 的原始论文 [2013, Popcorn: a replicated-kernel OS based on Linux](https://www.kernel.org/doc/ols/2014/ols2014-barbalace.pdf) 和 [2013/05/31, Popcorn Linux: enabling efficient inter-core communication in a Linux-based multikernel operating system](https://vtechworks.lib.vt.edu/items/3c7e76a9-7be5-463c-8a86-79db7e900a63), 支持应用程序在多个主机上运行并动态迁移线程, 具备跨节点共享一致虚拟内存的能力.<br>补丁集基于 Linux 5.2 版本, 目前仅支持 x86 架构, 实现了一个软件级的分布式共享内存系统, 采用读复制/写失效机制维护内存一致性. 当前版本暂时关闭了 L1TF 缓解机制, 后续将予以支持. 同时, 补丁包含新的系统调用、消息层(基于 TCP 的 socket 实现) 、页面与虚拟内存管理服务等. 参见 LWN 报道 [LWN：引人眼球的Popcorn Linux能合入kernel吗？](https://blog.csdn.net/Linux_Everything/article/details/106247623), 仓库地址 [ssrg-vt/popcorn-kernel](https://github.com/ssrg-vt/popcorn-kernel) | v1 ☐☑✓ | [2020/04/29, LORE v1, 0/9](https://lore.kernel.org/all/cover.1588127445.git.javier.malave@narfindustries.com) |
| 2025/09/18 | Cong Wang <xiyou.wangcong@gmail.com><br>Multikernel Technologies Inc  | [kernel: Introduce multikernel architecture support](https://lore.kernel.org/all/20250918222607.186488-1-xiyou.wangcong@gmail.com) | 引入"多内核架构(multikernel architecture)"支持的补丁集, 旨在使多个独立内核实例能够在同一台物理机上共存并通信. 每个多内核实例可绑定至特定 CPU 核心, 共享硬件资源, 通过增强的 kexec 子系统加载和管理.<br>主要特性包括: 改进工作负载间的故障隔离、增强安全性、比传统虚拟机更优的资源利用率, 以及支持通过 KHO 实现无宕机内核更新. 补丁集核心内容涵盖 kexec 支持多内核加载、x86 引导机制、通用 IPI 通信框架、物理 CPU ID 获取、动态 kimage 追踪及 `/proc` 接口. 参见 [phoronix, 2025/09/21, Multi-Kernel Architecture Proposed For The Linux Kernel](https://www.phoronix.com/news/Linux-Multi-Kernel-Patches) 和 [LWN, 20250923, Multiple kernels on a single system](https://lwn.net/Articles/1038847), [Multikernel Goes Open Source: Community-First Innovation](https://multikernel.io/2025/09/18/multikernel-goes-open-source/) | v1 ☐☑✓ | [2025/09/18, LORE v1, 0/7](https://lore.kernel.org/all/20250918222607.186488-1-xiyou.wangcong@gmail.com) |
| 2025/09/23 | Fam Zheng <fam.zheng@bytedance.com> | [parker: PARtitioned KERnel](https://lore.kernel.org/all/20250923153146.365015-1-fam.zheng@bytedance.com) | 字节提出了一项名为 **Parker( Partitioned Kernel)** 的新特性, 旨在支持在单一机器上同时运行多个 Linux 内核实例, 无需传统 KVM 虚拟化. 通过将 CPU、内存和设备资源分区, 由一个"启动内核"( Boot Kernel) 分配资源, 其他"应用内核"(Application Kernel) 各自使用专属资源, 彼此无共享、无通信, 提升可扩展性.<br>主要特点包括:<br>1. 使用 **kexec** 加载内核镜像至预留内存;<br>2. 通过 **kernfs** 提供配置接口;<br>3. 各内核可独立配置优化, 适配不同工作负载; <br>4. 目前缺乏硬件级隔离, 存在安全与容错风险. <br>Parker 适用于高核心数机器, 解决扩展性问题. 参见 [phoronix, 2025/09/23, Bytedance Proposes "Parker" For Linux: Multiple Kernels Running Simultaneously](https://www.phoronix.com/news/Linux-Parker-Proposal). | v1 ☐☑✓ | [2025/09/23, LORE v1, 0/5](https://lore.kernel.org/all/20250923153146.365015-1-fam.zheng@bytedance.com) |




<br>

*   本作品 / 博文 ([AderStep - 紫夜阑珊 - 青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚 (gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a><a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"> 知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议 </a> 进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名 [成坚 gatieme](http://blog.csdn.net/gatieme) (包含链接: http://blog.csdn.net/gatieme), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   ** 转载请务必注明出处, 谢谢, 不胜感激 **
<br>
