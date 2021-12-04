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


# 2 网络
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/01 | Shaokun Zhang <zhangshaokun@hisilicon.com> | [tcp: optimizations for linux-5.17e](https://patchwork.kernel.org/project/netdevbpf/cover/20211115190249.3936899-1-eric.dumazet@gmail.com/) | 参见报道 [Linux 5.17 To Boast A Big TCP Performance Optimization](https://git.kernel.org/pub/scm/linux/kernel/git/netdev/net-next.git/commit/?id=f35f821935d8df76f9c92e2431a225bdff938169) | v1 ☐ | [2021/04/09 Patchwork RESEND](https://patchwork.kernel.org/project/linux-fsdevel/patch/1617940057-52843-1-git-send-email-zhangshaokun@hisilicon.com)<br>*-*-*-*-*-*-*-* <br>[2021/06/01 Patchwork RESEND](https://patchwork.kernel.org/project/linux-fsdevel/patch%20/1622513557-46189-1-git-send-email-zhangshaokun@hisilicon.com) |


# 3 IO
-------

[Linux 5.17 To Continue With I/O Optimizations, 5~6% Improvement Pending For NVMe](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-Will-Continue-IO)
[Many Block Improvements Land In Linux 5.16 - Multi-Actuator Hard Drive Support](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.16-Block-Changes)
[Newest Linux Optimizations Can Achieve 10M IOPS Per-Core With IO_uring](https://www.phoronix.com/scan.php?page=news_item&px=Linux-IO_uring-10M-IOPS)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/03 | Peter Oskolkov <posk@google.com> | [thread_info: use helpers to snapshot thread flags](https://lwn.net/Articles/722293) | 参见报道 [Linux 5.17 To Continue With I/O Optimizations, 5~6% Improvement Pending For NVMe](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-Will-Continue-IO) | v1 ☐ | [PatchWork v4,00/10](https://lore.kernel.org/patchwork/cover/1471548) |


<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
