---

title: 影响性能的那些事
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

2   **虚拟化子系统**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***



# 1 调度
-------

关注与那些引起了一些场景, benchmark 发现的引起性能劣化和优化的补丁


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/01 | Shaokun Zhang <zhangshaokun@hisilicon.com> | [fs: Optimized file struct to improve performance](https://patchwork.kernel.org/project/linux-fsdevel/patch%20/1622513557-46189-1-git-send-email-zhangshaokun@hisilicon.com) | 通过调整 struct file 中各字段的布局来提升性能. 在系统调用过程中, 经常使用 struct file 结构体中 `f_count`和 `f_mod` 两个[字段](https://patchwork.kernel.org/project/linux-fsdevel/patch/1592987548-8653-1-git-send-email-zhangshaokun@hisilicon.com), 如果我们将它们放在一起, 将能有效地共享同一 cache line, 这对性能非常有用. intel 0-day CI 工程发现该补丁可以提升 UnixBench System Call Overhead 子项的性能, 参见 [aec499039e: unixbench.score 19.2% improvement](https://lkml.org/lkml/2021/4/20/28). | v1 ☐ | [2021/04/09 Patchwork RESEND](https://patchwork.kernel.org/project/linux-fsdevel/patch/1617940057-52843-1-git-send-email-zhangshaokun@hisilicon.com)<br>*-*-*-*-*-*-*-* <br>[2021/06/01 Patchwork RESEND](https://patchwork.kernel.org/project/linux-fsdevel/patch%20/1622513557-46189-1-git-send-email-zhangshaokun@hisilicon.com) |
| 2021/08/03 | Peter Oskolkov <posk@google.com> | [5-10% increase in IO latencies with nohz balance patch](https://lkml.org/lkml/2021/11/29/1108) | [7fd7a9e0caba ("sched/fair: Trigger nohz.next_balance updates when a CPU goes NOHZ-idle")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7fd7a9e0caba) 这个补丁导致了 FIO 性能测试劣化. | v1 ☐ | [PatchWork v4,00/10](https://lore.kernel.org/patchwork/cover/1471548) |
| 2021/08/03 | Peter Oskolkov <posk@google.com> | [New Linux Scheduler Patches Can Improve AMD Zen Performance For Some Workloads](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Sched-NUMA-Imbalance-Zen) | NA | v1 ☐ | [PatchWork v4,00/10](https://lore.kernel.org/patchwork/cover/1471548) |
| 2018/11/05 | Patrick Bellasi <patrick.bellasi@arm.com> | [sched/fair: Fix cpu_util_wake() for 'execl' type workloads](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c469933e772132aad040bd6a2adc8edf9ad6f825) | TODO | v1 ☐☑✓ | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c469933e772132aad040bd6a2adc8edf9ad6f825) |
| 2019/10/22 | Vincent Guittot <vincent.guittot@linaro.org> | [sched/fair: Fix rework of find_idlest_group()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3318544b721d3072fdd1f85ee0f1f214c0b211ee) | TODO | v1 ☐☑✓ | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3318544b721d3072fdd1f85ee0f1f214c0b211ee) |


# 2 内存
-------

[commit ff042f4a9b05 ("mm: lru_cache_disable: replace work queue synchronization with synchronize_rcu")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ff042f4a9b050895a42cae893cc01fa2ca81b95c) 引起了 NUMA 系统 stress-ng 的性能回归, 被 phoronix 测试并报告, 参见 [Poking At A Big NUMA Benchmark Regression In Linux 5.18 Git](https://www.phoronix.com/scan.php?page=news_item&px=Linux-518-Stress-NUMA-Goes-Boom) 以及社区报告邮件 [Re: [patch v5] mm: lru_cache_disable: replace work queue synchronization with synchronize_rcu](https://lore.kernel.org/lkml/0ce05ce7-1a00-82df-f37a-bf7f9e216504@MichaelLarabel.com).  Stress-NG 0.14 测试项 NUMA 的分数从 v5.17 的 412.88 下降到 v5.18 的 49.33.

随后社区进行了修复, 参见 [A Fix Is On The Way For A Previously-Reported Linux 5.18 Performance Regression](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.18-NUMA-Regression-Fix).

# 3 网络
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/01 | Shaokun Zhang <zhangshaokun@hisilicon.com> | [tcp: optimizations for linux-5.17e](https://patchwork.kernel.org/project/netdevbpf/cover/20211115190249.3936899-1-eric.dumazet@gmail.com/) | 对于包含了大量数据的报文(大包), 释放也是比较耗时间的, 因此将报文释放的操作从锁中移出. 参见报道 [Linux 5.17 To Boast A Big TCP Performance Optimization](https://git.kernel.org/pub/scm/linux/kernel/git/netdev/net-next.git/commit/?id=f35f821935d8df76f9c92e2431a225bdff938169) | v1 ☐ 5.17-rc1 | [2021/04/09 Patchwork RESEND](https://patchwork.kernel.org/project/linux-fsdevel/patch/1617940057-52843-1-git-send-email-zhangshaokun@hisilicon.com)<br>*-*-*-*-*-*-*-* <br>[2021/06/01 Patchwork RESEND](https://patchwork.kernel.org/project/linux-fsdevel/patch%20/1622513557-46189-1-git-send-email-zhangshaokun@hisilicon.com) |

https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-AF_UNIX-Optimization

https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-Faster-csum_partial

https://www.phoronix.com/scan.php?page=news_item&px=UDP-IPV6-5P-Optimization

[UDP IPv6 Optimizations Queued Up For Linux 5.18](https://www.phoronix.com/scan.php?page=news_item&px=UDP-IPv6-Opts-5.18)


# 4 IO
-------

[Linux 5.17 To Continue With I/O Optimizations, 5~6% Improvement Pending For NVMe](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-Will-Continue-IO)
[Many Block Improvements Land In Linux 5.16 - Multi-Actuator Hard Drive Support](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.16-Block-Changes)
[Newest Linux Optimizations Can Achieve 10M IOPS Per-Core With IO_uring](https://www.phoronix.com/scan.php?page=news_item&px=Linux-IO_uring-10M-IOPS)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/03 | Peter Oskolkov <posk@google.com> | [thread_info: use helpers to snapshot thread flags](https://lwn.net/Articles/722293) | 参见报道 [Linux 5.17 To Continue With I/O Optimizations, 5~6% Improvement Pending For NVMe](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-Will-Continue-IO) | v1 ☐ | [PatchWork v4,00/10](https://lore.kernel.org/patchwork/cover/1471548) |


# 5 ARCH
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/12/22 | Wander Lairson Costa <wander@redhat.com> | [tty: serial: Use fifo in 8250 console driver](https://lwn.net/Articles/722293) | 参见报道 [Linux Serial Console Driver Lands Patch For Possible ~25% Performance Improvement](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Serial-16550-FIFO) | v1 ☑ 5.17-rc1 | [PatchWork v4,00/10](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5021d709b31b8a14317998a33cbc78be0de9ab30) |

# 6 LIB
-------


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/07/10 | Yu-Jen Chang <arthurchang09@gmail.com> | [Optimize memchr()](https://lore.kernel.org/all/20220710142822.52539-1-arthurchang09@gmail.com) | 优化 Linux 内核的 memchr() 实现, 以便更快地在内存块中定位字符. 这组补丁使得 memchr 在处理长字符串时可以快 4 倍左右. memchr() 的原始版本是使用按字节比较技术实现的, 该技术在 CPU 中不完全使用 64 位或 32 位寄存器. 使用全字范围的比较, 以便在 CPU 上可以同时比较 8 个字符. 参见 [Optimized memchr() Implementation For The Linux Kernel Up To ~4x Faster](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Kernel-Faster-memchr). | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20220710142822.52539-1-arthurchang09@gmail.com) |



# 10 评测
-------


## 10.1 系统评测
-------

Intel AderLake 引入大小核之后, Windows & Intel 联盟做了足够多的优化, 但是 Linux 还没有针对性的优化, 因此在 Core i9 12900K(Alder Lake) 上测试, Window 11 性能领先于 Linux. [Windows 11 Better Than Linux Right Now For Intel Alder Lake Performance](https://www.phoronix.com/scan.php?page=article&item=alderlake-windows-linux&num=1).

随后多个 Linux 版本都对性能进行了优化和修复. 参见 phoronix 报道 [Linux 5.15.35 Released With Important Performance Fix For Intel Alder Lake](https://www.phoronix.com/scan.php?page=article&item=linux-51535-adl&num=1).

[Windows 11 vs. Linux Performance For Intel Core i9 12900K In Mid-2022](https://www.phoronix.com/scan.php?page=article&item=windows-linux-mid22adl&num=1).

[Apple M2 vs. AMD Rembrandt vs. Intel Alder Lake Linux Benchmarks](https://www.phoronix.com/review/apple-m2-linux)

## 10.2 内核评测
-------

| 日期 | 版本 | 评测 |
|:---:|:----:|:---:|
| 2022/07/11 | [Linux 5.19 Looking Real Good On The HP Dev One, XanMod + Liquorix Also Tested](https://www.phoronix.com/scan.php?page=article&item=hp-devone-kernels) | 基于 HP Dev One 与 AMD Ryzen 7 PRO 5850U 测试了 Mainline 5.19 相比较 Pop_OS Stock 5.17/Mainline 5.18/XanMod 5.18/Liquorix 5.18 的性能. |


## 10.3 硬件评测
-------

[Intel Xeon Platinum 8380 Performance Is Looking Great For Linux 6.0](https://www.phoronix.com/news/Linux-6.0-Xeon-8380-Ice-Lake)

<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
