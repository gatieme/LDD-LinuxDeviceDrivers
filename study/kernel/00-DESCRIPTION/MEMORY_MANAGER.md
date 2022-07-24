 2   **内存管理子系统(memory management)**
=====================


# 0 内存管理子系统(开篇)
-------



## 0.1 内存管理子系统概述
------

**概述: **内存管理子系统**, 作为 kernel 核心中的核心, 是承接所有系统活动的舞台, 也是 Linux kernel 中最为庞杂的子系统, 没有之一．截止 4.2 版本, 内存管理子系统(下简称 MM)所有平台独立的核心代码(C文件和头文件)达到11万6千多行, 这还不包括平台相关的 C 代码, 及一些汇编代码; 与之相比, 调度子系统的平台独立的核心代码才2万８千多行．


现代操作系统的 MM 提供的一个重要功能就是为每个进程提供独立的虚拟地址空间抽象, 为用户呈现一个平坦的进程地址空间, 提供安全高效的进程隔离, 隐藏所有细节, 使得用户可以简单可移植的库接口访问／管理内存, 大大解放程序员生产力．



在继续往下之前, 先介绍一些 Linux 内核中内存管理的基本原理和术语, 方便后文讨论．





**- 物理地址(Physical Address):** 这就是内存 DIMM 上的一个一个存储区间的物理编址, 以字节为单位．



**- 虚拟地址(Virtual Address):** 技术上来讲, 用户或内核用到的地址就是虚拟地址, 需要 MMU (内存管理单元, 一个用于支持虚拟内存的 CPU 片内机构) 翻译为物理地址．在 CPU 的技术规范中, 可能还有虚拟地址和线性地址的区别, 但在这不重要．



**- NUMA(Non-Uniform Memory Access):** 非一致性内存访问．NUMA 概念的引入是为了解决随着 CPU 个数的增长而出现的内存访问瓶颈问题, 非一致性内存意为每个 NUMA 节点都有本地内存, 提供高访问速度; 也可以访问跨节点的内存, 但要遭受较大的性能损耗．所以尽管整个系统的内存对任何进程来说都是可见的, 但却存在访问速度差异, 这一点对内存分配/内存回收都有着非常大的影响．Linux 内核于2.5版本引入对 NUMA的支持[<sup>7</sup>](#refer-anchor-7).



**- NUMA node(NUMA节点):**  NUMA 体系下, 一个 node 一般是一个CPU socket(一个 socket 里可能有多个核)及它可访问的本地内存的整体．



**- zone(内存区):** 一个 NUMA node 里的物理内存又被分为几个内存区(zone), 一个典型的 node 的内存区划分如下:

![zone 区域](https://pic3.zhimg.com/50/b53313b9ef1f062460f90f56bcf6d0b7_hd.jpg)


可以看到每个node里, 随着**物理内存地址**的增加, 典型地分为三个区:

> **1\. ZONE\_DMA**: 这个区的存在有历史原因, 古老的 ISA 总线外设, 它们进行 DMA操作[<sup>10</sup>](#refer-anchor-10) 时, 只能访问内存物理空间低 16MB 的范围．所以故有这一区, 用于给这些设备分配内存时使用．
> **2\. ZONE\_NORMAL**: 这是 32位 CPU时代产物, 很多内核态的内存分配都是在这个区间(用户态内存也可以在这部分分配, 但优先在ZONE\_HIGH中分配), 但这部分的大小一般就只有 896 MiB, 所以比较局限． 64位 CPU 情况下, 内存的访问空间增大, 这部分空间就增大了很多．关于为何这部分区间这么局限, 且内核态内存分配在这个区间, 感兴趣的可以看我[之间一个回答](http://www.zhihu.com/question/34787574/answer/60214771).
> **3\. ZONE\_HIGH**: 典型情况下, 这个区间覆盖系统所有剩余物理内存．这个区间叫做高端内存区(不是高级的意思, 是地址区间高的意思). 这部分主要是用户态和部分内核态内存分配所处的区间．





**- 内存页/页面(page):** 现代虚拟内存管理／分配的单位是一个物理内存页, 大小是 4096(4KB) 字节. 当然, 很多 CPU 提供多种尺寸的物理内存页支持(如 X86, 除了4KB, 还有 2MB, 1GB页支持), 但 Linux 内核中的默认页尺寸就是 4KB．内核初始化过程中, 会对每个物理内存页分配一个描述符(struct page), 后文描述中可能多次提到这个描述符, 它是 MM 内部, 也是 MM 与其他子系统交互的一个接口描述符．



**- 页表(page table):** 从一个虚拟地址翻译为物理地址时, 其实就是从一个稀疏哈希表中查找的过程, 这个哈希表就是页表．



**- 交换(swap):** 内存紧缺时, MM 可能会把一些暂时不用的内存页转移到访问速度较慢的次级存储设备中(如磁盘, SSD), 以腾出空间,这个操作叫交换, 相应的存储设备叫交换设备或交换空间.



**- 文件缓存页(PageCache Page):** 内核会利用空闲的内存, 事先读入一些文件页, 以期不久的将来会用到, 从而避免在要使用时再去启动缓慢的外设(如磁盘)读入操作. 这些有后备存储介质的页面, 可以在内存紧缺时轻松丢弃, 等到需要时再次从外设读入. **典型的代表有可执行代码, 文件系统里的文件.**

**- 匿名页(Anonymous Page):** 这种页面的内容都是在内存中建立的,没有后备的外设, 这些页面在回收时不能简单的丢弃, 需要写入到交换设备中. **典型的代表有进程的栈, 使用 _malloc()_ 分配的内存所在的页等 .**


一篇对数据库中内存行为进行分析的博文 [Optimizing Linux Memory Management for Low-latency / High-throughput Databases](https://engineering.linkedin.com/performance/optimizing-linux-memory-management-low-latency-high-throughput-databases#numarebalance).

## 0.2 概述
-------


内存管理的目标是提供一种方法, 为实现各种目的而在各个用户之间实现内存共享. 内存管理方法应该实现以下两个功能:

*   最小化管理内存所需的时间

*   最大化用于一般应用的可用内存(最小化管理开销)

内存管理实际上是一种关于权衡的零和游戏. 您可以开发一种使用少量内存进行管理的算法, 但是要花费更多时间来管理可用内存. 也可以开发一个算法来有效地管理内存, 但却要使用更多的内存. 最终, 特定应用程序的需求将促使对这种权衡作出选择.


| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----------:|:---:|
| 1991/01/01 | [示例 sched: Improve the scheduler]() | 此处填写描述【示例】 | ☑ ☒☐ v3/5.4-rc1 | [LWN](), [PatchWork](), [lkml]() |
| 2016/06/27 | [mm: mirrored memory support for page buddy allocations](http://lore.kernel.org/patchwork/patch/574230) | 内存镜像的功能 | RFC v2 ☐  | [PatchWork](https://lore.kernel.org/patchwork/patch/574230) |
| 2016/01/08 | [mm: Introduce kernelcore=mirror option](http://lore.kernel.org/patchwork/patch/574230) | 内存镜像的功能 | RFC v2 ☐  | [LKML](https://lkml.org/lkml/2016/1/8/88), [PatchWork](https://lore.kernel.org/lkml/1452241523-19559-1-git-send-email-izumi.taku@jp.fujitsu.com) |


## 0.3 主线内存管理分支合并窗口
-------

Andrew Morton 一般以一封 [incoming](https://lore.kernel.org/linux-mm/20211105133408.cccbb98b71a77d5e8430aba1@linux-foundation.org) 的邮件向 Linus 发起 push 请求, Linus pull 之后的 Merge 标题为 Merge branch 'akpm' (patches from Andrew).

- [x] Merge branch 'akpm' (patches from Andrew)

```cpp
git log --oneline v5.15...v5.16 | grep -E "Merge branch | Linux "  | grep -E "akpm| Linux " | less
```

| 版本 | 发布时间 | 合并链接 |
|:---:|:-------:|:-------:|
| 5.13 | [2021/06/02](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=62fb9874f5da) | [2021/04/30, 5.13-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d42f323a7df0b298c07313db00b44b78555ca8e6)<br>[2021/05/05, 5.13-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8404c9fbc84b741f66cff7d4934a25dd2c344452)<br>[2021/05/15, 5.13-rc2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a4147415bdf152748416e391dd5d6958ad0a96da)<br>[2021/05/22, 5.13-rc3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=34c5c89890d6295621b6f09b18e7ead9046634bc)<br>[2021/06/05, 5.13-rc5](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e5220dd16778fe21d234a64e36cf50b54110025f)<br>[2021/06/16, 5.13-rc7](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=70585216fe7730d9fb5453d3e2804e149d0fe201)<br>[2021/06/25, 5.13](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7ce32ac6fb2fc73584b567c73ae0c47528954ec6) |
| 5.14 | [2021/08/29](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=73f3af7b4611) | [2021/06/29, 5.14-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=65090f30ab791810a3dc840317e57df05018559c)<br>[2021/07/02, 5.14-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=71bd9341011f)<br>[2021/07/09, 5.14-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bd9c35060329)<br>[2021/07/15, 5.14-rc2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=dd9c7df94c1b)<br>[2021/07/24, 5.14-rc3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bca1d4de3981)<br>[2021/07/30, 5.14-rc4](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ad6ec09d9622)<br>[2021/08/13, 5.14-rc6](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=dfa377c35d70)<br>[2021/08/20, 5.14-rc7](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ed3bad2e4fd7)<br>[2021/08/25, 5.14](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=73f3af7b4611) |
| 5.15 |  [2021/10/31](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8bb7eca972ad) | [2021/09/08, 5.15-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2d338201d531)<br>[2021/09/08, 5.15-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a3fa7a101dcf)<br>[2021/09/25, 5.15-rc3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a3b397b4fffb)<br>[2021/10/19, 5.15-rc6](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d9abdee5fd5a)<br>[2021/10/29, 5.15](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2c04d67ec1eb) |
| 5.16 | [2021/10/31](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8bb7eca972ad) | [2021/11/06, 5.16-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=512b7931ad05)<br>[2021/11/09, 5.16-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=59a2ceeef6d6)<br>[2021/11/11, 5.16-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=dbf49896187f)<br>[2021/11/20, 5.16-rc2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=923dcc5eb0c1)<br>[2021/11/20, 5.16-rc5](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=df442a4ec740)<br>[2021/12/25, 5.16-rc7](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d0cc67b2781654ac71c73d303e0347e5e9b10ad3)<br>[2021/12/25, 5.16-rc8](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f87bcc88f3028af584b0820bdf6e0e4cdc759d26) |
| 5.17 | 2022/03/20 | [2022/01/20, 5.17-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f4484d138b31e8fa1ba410363b5b9664f68974af)<br>[2022/01/22, 5.17-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1c52283265a462a100ae63ddf58b4e5884acde86)<br>[2022/01/30, 5.17-rc2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8dd71685dcb7839f6d91417e0a9237daca363908)<br>[2022/01/04, 5.17-rc3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f9aaa5b05ea376f4917ff2c838c4641a100fd1e2)<br>[2022/02/12, 5.17-rc4](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9917ff5f319788a195c691fa19cf3e90cee59f40)<br>[2022/02/26, 5.17-rc6](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=086ee11b0384c5ee837a46fac36e38189717960b)<br>[2022/03/05, 5.17-rc7](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0014404f9c18dd360a1b8bb4243643c679ce99bf)<br>[2022/03/20, 5.17](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f443e374ae131c168a065ea1748feac6b2e76613) |


cgit 上查看 MM 所有的 log 信息 :

[GIT LOG: linux/mm](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/mm)


| 时间  | 记者 | 报道 | 描述 |
|:----:|:----:|:---:|:----:|
| 2021/11/12 | Jonathan Corbet | [Some upcoming memory-management patches](https://lwn.net/Articles/875587) | NA | 介绍了如下几个特性:<br>1. Freeing page-table pages: [Free user PTE page table pages](https://patchwork.kernel.org/project/linux-mm/cover/20210819031858.98043-1-zhengqi.arch@bytedance.com)<br>2. More flags in vmalloc(): [extend vmalloc support for constrained allocations](https://patchwork.kernel.org/project/linux-mm/cover/20211018114712.9802-1-mhocko@kernel.org)<br>3. Uncached memory clearing: [Use uncached stores while clearing huge pages](https://patchwork.kernel.org/project/linux-mm/cover/20211020170305.376118-1-ankur.a.arora@oracle.com)<br>4. Setting a home NUMA node:  |



## 0.4 社区会议
-------

### 0.4.1 Linux Plumbers Conference
-------

### 0.4.2 LSFMM(Linux Storage, Filesystem, and Memory-Management SummitScheduler Microconference Accepted into Linux Plumbers Conference

`2010~2017` 年的内容, 可以在 [wiki](http://wiki.linuxplumbersconf.org/?do=search&id=scheduler) 检索.

| 日期 | 官网 | LKML | LWN |
|:---:|:----:|:----:|:---:|
| NA | NA | NA | [The 2019 LSFMM Summit](https://lwn.net/Articles/lsfmm2019) |
| NA | NA | NA | [The 2018 LSFMM Summit](https://lwn.net/Articles/lsfmm2018) |


## 0.5 社区的内存管理领域的开发者
-------

| developer | git |
|:---------:|:---:|
| [David Rientjes <rientjes@google.com>](https://lore.kernel.org/patchwork/project/lkml/list/?submitter=6580&state=*&archive=both&param=4&page=1) | NA |
| [Mel Gorman <mgorman@techsingularity.net>](https://lore.kernel.org/patchwork/project/lkml/list/?submitter=19167&state=*&archive=both&param=3&page=1) | [git.kernel.org](https://git.kernel.org/pub/scm/linux/kernel/git/mel/linux.git) |
| [Minchan Kim <minchan@kernel.org>](https://lore.kernel.org/patchwork/project/lkml/list/?series=&submitter=13305&state=*&q=&archive=both&delegate=) | NA |
| [Joonsoo Kim](https://lore.kernel.org/patchwork/project/lkml/list/?submitter=13703&state=%2A&archive=both) | NA |
| [Kamezawa Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com>](https://lore.kernel.org/patchwork/project/lkml/list/?submitter=4430&state=%2A&archive=both) | NA |
| [Kirill A. Shutemov <kirill.shutemov@linux.intel.com>](https://lore.kernel.org/patchwork/project/lkml/list/?submitter=13419&state=%2A&archive=both) | [git.kernel.org](https://git.kernel.org/pub/scm/linux/kernel/git/kas/linux.git)
| [Naoya Horiguchi <n-horiguchi@ah.jp.nec.com>](https://github.com/nhoriguchi/linux) | [github/nhoriguchi/linu](https://github.com/nhoriguchi/linux) |

## 0.6 社区地址
-------


| 描述 | 地址 |
|:---:|:----:|
| WebSite | [linux-mm](https://www.linux-mm.org) |
| PatchWork | [PatchWork](https://patchwork.kernel.org/project/linux-mm/list/?archive=both&state=*) |
| 邮件列表 | linux-mm@kvack.org |
| maintainer branch | [Github](https://github.com/hnaz/linux-mm)

## 0.7 目录
-------

下文将按此**目录**分析 Linux 内核中 MM 的重要功能和引入版本:




- [x] 1. 页表管理

- [x] 2. 内存分配

- [x] 3. 内存去碎片化

- [x] 4. 页面回收

- [x] 5. Swappiness

- [x] 6. PageCache

- [x] 7. 大内存页支持

- [x] 8. 内存控制组(Memory Cgroup)支持

- [x] 9. 内存热插拔支持

- [x] 10. 超然内存(Transcendent Memory)支持

- [x] 11. 非易失性内存 (NVDIMM, Non-Volatile DIMM) 支持

- [x] 12. 内存管理调试支持

- [x] 13. 杂项



# 1 页表管理
-------

## 1.1 多级页表
-------

**2.6.11(2005年3月发布)**

页表实质上是一个虚拟地址到物理地址的映射表, 但由于程序的局部性, 某时刻程序的某一部分才需要被映射, 换句话说, 这个映射表是相当稀疏的, 因此在内存中维护一个一维的映射表太浪费空间, 也不现实. 因此, 硬件层次支持的页表就是一个多层次的映射表.

Linux 一开始是在一台i386上的机器开发的, i386 的硬件页表是2级的(页目录项 -> 页表项), 所以, 一开始 Linux 支持的软件页表也是2级的; 后来, 为了支持 PAE (Physical Address Extension), 扩展为3级; 后来, 64位 CPU 的引入, 3级也不够了, 于是, 2.6.11 引入了四级的通用页表.


关于四级页表是如何适配 i386 的两级页表的, 很简单, 就是虚设两级页表. 类比下, 北京市(省)北京市海淀区东升镇, 就是为了适配4级行政区规划而引入的一种表示法. 这种通用抽象的软件工程做法在内核中不乏例子.

关于四级页表演进的细节, 可看我以前文章: [Linux内核4级页表的演进](https://link.zhihu.com/?target=http%3A//larmbr.com/2014/01/19/the-evolution-of-4-level-page-talbe-in-linux)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/03/13 | "Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [x86: 5-level paging enabling for v4.12, Part 1](https://lore.kernel.org/patchwork/patch/769534) | NA| v1 ☑ 4.12-rc1 | [PatchWork 0/6](https://lore.kernel.org/patchwork/patch/769534) |
| 2017/03/17 | "Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [x86: 5-level paging enabling for v4.12, Part 2](https://lore.kernel.org/patchwork/patch/771246) | NA | v1 ☑ 4.12-rc1 | [PatchWork 0/6](https://lore.kernel.org/patchwork/patch/771246) |
| 2017/03/17 | "Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [x86: 5-level paging enabling for v4.12, Part 3](https://lore.kernel.org/patchwork/patch/775152) | NA | v3 ☑ 4.12-rc1 | [PatchWork v3,0/7](https://lore.kernel.org/patchwork/patch/775152) |
| 2017/06/06 | "Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [x86: 5-level paging enabling for v4.13, Part 4](https://lore.kernel.org/patchwork/patch/796033) | NA | v7 ☑ 4.14-rc1 | [PatchWork v7,00/14](https://lore.kernel.org/patchwork/patch/796033) |
| 2017/06/16 | "Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [5-level paging enabling for v4.14](https://lore.kernel.org/patchwork/patch/810080) | NA | v7 ☑ 4.14-rc1 | [PatchWork 0/8](https://lore.kernel.org/patchwork/patch/810080) |
| 2017/06/22 | "Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [Last bits for initial 5-level paging enabling](https://lore.kernel.org/patchwork/patch/802652) | NA | v1 ☑ 4.14-rc1 | [PatchWork 0/5](https://lore.kernel.org/patchwork/patch/802652) |
| 2017/08/07 | "Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [Boot-time switching between 4- and 5-level paging](https://lore.kernel.org/patchwork/patch/818286) | NA | v3 ☑ 4.17-rc1 | [PatchWork v3,00/13](https://lore.kernel.org/patchwork/patch/818286) |
| 2022/06/27 | Adam Sindelar <adam@wowsignal.io> | [selftests/vm: Only run 128TBswitch with 5-level paging](https://patchwork.kernel.org/project/linux-mm/patch/20220627155530.2272-1-adam@wowsignal.io/) | 654233 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220627155530.2272-1-adam@wowsignal.io)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/1](https://lore.kernel.org/r/20220627163912.5581-1-adam@wowsignal.io) |



## 1.2 VA_BITS
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/12/03 | Kristina Martsenko <kristina.martsenko@arm.com> | [arm64: enable 52-bit physical address support](https://lwn.net/Articles/849538) | 支持 ARMv8.2 的 52 bit 地址特性. | v3 ☐ 4.16-rc1 | [PatchWork 0/10](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1513184845-8711-11-git-send-email-kristina.martsenko@arm.com) |
| 2019/08/07 | Steve Capper <steve.capper@arm.com> | [52-bit kernel + user VAs](https://lwn.net/Articles/849538) | 内核支持 52 bit 虚拟地址空间. | v5 ☐ 5.4-rc1 | [PatchWork v5 52-bit userspace VAs](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20181206225042.11548-1-steve.capper@arm.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v5](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20190807155524.5112-1-steve.capper@arm.com) |
| 2019/08/07 | Steve Capper <steve.capper@arm.com> | [arm64/mm: Enable FEAT_LPA2 (52 bits PA support on 4K|16K pages)](https://lwn.net/Articles/849538) | arm64 使能 FEAT_LPA2.<br>4K/16K PAGE_SIZE 下支持 52 bit PA. | RFC,V2 ☐ 5.14 | [PatchWork v5 52-bit userspace VAs](https://patchwork.kernel.org/project/linux-arm-kernel/cover/20181206225042.11548-1-steve.capper@arm.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC,V2,00/10](https://patchwork.kernel.org/project/linux-mm/cover/1627281445-12445-1-git-send-email-anshuman.khandual@arm.com) |


## 1.3 TLB flushing
-------

### 1.3.1 延迟页表缓存冲刷 (Lazy-TLB flushing)
-------

**极早引入, 时间难考**


有个硬件机构叫 TLB, 用来缓存页表查寻结果, 根据程序局部性, 即将访问的数据或代码很可能与刚访问过的在一个页面, 有了 TLB 缓存, 页表查找很多时候就大大加快了. 但是, 内核在切换进程时, 需要切换页表, 同时 TLB 缓存也失效了, 需要冲刷掉. 内核引入的一个优化是, 当切换到内核线程时, 由于内核线程不使用用户态空间, 因此切换用户态的页表是不必要, 自然也不需要冲刷 TLB. 所以引入了 Lazy-TLB 模式, 以提高效率. 关于细节, 可参考[kernel 3.10内核源码分析--TLB相关--TLB概念、flush、TLB lazy模式](https://www.cnblogs.com/sky-heaven/p/5133747.html)


### 1.3.2 avoid unnecessary TLB flushes
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/01/31 | Nadav Amit <nadav.amit@gmail.com> | [TLB batching consolidation and enhancements](https://patchwork.kernel.org/project/linux-mm/cover/20210131001132.3368247-1-namit@vmware.com) | TLB 批处理整合和增强. 内核中目前至少有 5 种不同的 TLB 处理方案. 对这种情况做了合并和规整. | v2 ☐ | [PatchWork RFC,00/20](https://patchwork.kernel.org/project/linux-mm/cover/20210131001132.3368247-1-namit@vmware.com) |
| 2021/10/21 | Nadav Amit <nadav.amit@gmail.com> | [mm/mprotect: avoid unnecessary TLB flushes](https://patchwork.kernel.org/project/linux-mm/cover/20211021122112.592634-1-namit@vmware.com) | 用于删除不必要的 TLB 刷新. | v2 ☐ | [2021/09/25 PatchWork v1,0/2](https://patchwork.kernel.org/project/linux-mm/cover/20210925205423.168858-1-namit@vmware.com)<br>*-*-*-*-*-*-*-* <br>[2021/10/21 PatchWork v2,0/5](https://patchwork.kernel.org/project/linux-mm/cover/20211021122112.592634-1-namit@vmware.com) |


## 1.4 [Clarifying memory management with page folios](https://lwn.net/Articles/849538)
-------

### 1.4.1 page folios
-------

[LWN: 利用page folio来明确内存操作！](https://blog.csdn.net/Linux_Everything/article/details/115388078)

[带有 "memory folios" 的 Linux: 编译内核时性能提升了 7%](https://www.heikewan.com/item/27509944)


最终该特性与 5.16 合入, [Memory Folios Merged For Linux 5.16](https://www.phoronix.com/scan.php?page=news_item&px=Memory-Folios-Lands-Linux-5.16), 代码仓库 [willy/pagecache.git](http://git.infradead.org/users/willy/pagecache.git), 合入链接 [GIT,PULL Memory folios for v5.1](https://patchwork.kernel.org/project/linux-mm/patch/YX4RkYNNZtO9WL0L@casper.infradead.org), [Merge tag 'folio-5.16' of git://git.infradead.org/users/willy/pagecache](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=49f8275c7d9247cf1dd4440fc8162f784252c849)


内存管理(memory management) 一般是以 page 为单位进行的, 一个 page 通常包含 4,096 个字节, 也可能更大. 内核已经将 page 的概念扩展到所谓的 compound page(复合页), 即一组组物理连续的单独 page 的组合. 这又使得 "page" 的定义变得有些模糊了. Matthew Wilcox 提出了 "page folio" 的概念, 它实际上仍然是一个 page structure, 只是保证了它一定不是 tail page. 任何接受 folio page 参数的函数都会是对整个 compound page 进行操作(如果传入的确实是一个 compound page 的话), 这样就不会有任何歧义. 从而可以使内核里的内存管理子系统更加清晰; 也就是说, 如果某个函数被改为只接受 folio page 作为参数的话, 很明确, 它们不适用于对 tail page 的操作. 通过 folio 结构来管理内存. 它提供了一些具有自身价值的基础设施, 将内核的文本缩减了约 6kB.



#### 1.4.1.1 Memory folios core @5.16
-------

[Clarifying memory management with page folios](https://lwn.net/Articles/849538)

[The folio pull-request pushback](https://lwn.net/Articles/868598)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/07/15 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Memory folios](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=121703c1c817b3c77f61002466d0bfca7e39f25d) | 主要功能 [Memory folios](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=121703c1c817b3c77f61002466d0bfca7e39f25d) 以及 [BUGFIX](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=c035713998700e8843c7d087f55bce3c54c0e3ec). | v14 ☑ [5.16-rc1](https://kernelnewbies.org/Linux_5.16#Memory_folios_infrastructure_for_a_faster_memory_management) | [PatchWork v13,000/137](https://patchwork.kernel.org/project/linux-mm/cover/20210712030701.4000097-1-willy@infradead.org/)<br>[PatchWork v13a](https://patchwork.kernel.org/project/linux-mm/cover/20210712190204.80979-1-willy@infradead.org)<br>*-*-*-*-*-*-*-* <br>[LORE v14,000/138](https://patchwork.kernel.org/project/linux-mm/cover/20210715033704.692967-1-willy@infradead.org)<br>*-*-*-*-*-*-*-* <br>[[GIT,PULL] Memory folios for v5.16, 00/90](https://patchwork.kernel.org/project/linux-mm/patch/YX4RkYNNZtO9WL0L@casper.infradead.org), [[GIT,PULL] Folio fixes for 5.16, 0/6](https://patchwork.kernel.org/project/linux-mm/patch/YZ6enA9aRgJLL55w@casper.infradead.org) |
| 2021/11/10 | David Howells <dhowells@redhat.com> | [netfs, 9p, afs, ceph: Support folios, at least partially](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=255ed63638da190e2485d32c0f696cd04d34fbc0) | 参见 phoronix 的报道 [AFS, 9p, Netfslib Wired Up To Use Newly-Merged Folios In Linux 5.16](https://www.phoronix.com/scan.php?page=news_item&px=AFS-9p-NETFS-Folios-Linux-5.16) | v5 ☑ 5.16-rc1 | [PatchWork v4,0/5](https://patchwork.kernel.org/project/linux-mm/cover/163649323416.309189.4637503793406396694.stgit@warthog.procyon.org.uk)<br>*-*-*-*-*-*-*-* <br>[PatchWork v5,0/4](https://patchwork.kernel.org/project/linux-mm/cover/163657847613.834781.7923681076643317435.stgit@warthog.procyon.org.uk)<br>*-*-*-*-*-*-*-* <br>[Merge tag](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0f7ddea6225b9b001966bc9665924f1f8b9ac535) |

#### 1.4.1.2 Memory folios: Pagecache edition @5.17
-------

[Folio Improvements For Linux 5.17, Large Folio Patches Posted](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-Folios)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/22 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Folio-enabling the page cache](https://patchwork.kernel.org/project/linux-mm/cover/20210622121551.3398730-1-willy@infradead.org) | NA | v2 ☐ | [PatchWork v2](https://patchwork.kernel.org/project/linux-mm/cover/20210622121551.3398730-1-willy@infradead.org) |
| 2021/07/15 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Memory folios: Pagecache edition](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=b27652d935f41793c5e229a1e8b3a8bb3afe3cc1) | NA | v14c ☑ 5.16-rc1 | [PatchWork v14c,00/39](https://patchwork.kernel.org/project/linux-mm/cover/20210715200030.899216-1-willy@infradead.org) |
| 2021/10/08 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Folios for 5.17](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=6b24ca4a1a8d4ee3221d6d44ddbb99f542e4bda3) | 将大部分 Page Cache 转换为使用 folios. 其中最大的变化是[在页面缓存 XArray 中使用大型条目](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6b24ca4a1a8d4ee3221d6d44ddbb99f542e4bda3), 而不是许多小型条目. 目前, 这只会影响到 shmem, 但对 shmem 来说, 这是一个相当大的变化, 因为[它改变了需要分配内存的位置](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b9a8a4195c7d3a51235a4fc974a46ad4e9689ffd)(在分割时而不是插入时). | v1 ☑ [5.17-rc1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6020c204be997e3f5129839ff9c801800fb4336e) | [PatchWork 00/48](https://patchwork.kernel.org/project/linux-mm/cover/20211208042256.1923824-1-willy@infradead.org), [[GIT,PULL] Page cache for 5.17, 00/48](https://patchwork.kernel.org/project/linux-mm/patch/YdyuuBCe4EPmr3k2@casper.infradead.org) |
| 2022/01/16 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Enabling large folios for 5.17](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=72e725887413f031fa72d27fea5795450bab1940) | NA | v1 ☐ | [LKML 00/12](https://patchwork.kernel.org/project/linux-mm/cover/20220116121822.1727633-1-willy@infradead.org) |

#### 1.4.1.2 Memory folios @5.18
-------

[[GIT,PULL] Folio patches for 5.18 (MM part)](https://patchwork.kernel.org/project/linux-mm/patch/Yjh+EuacJURShtJI@casper.infradead.org/)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/30 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Folio conversion of memcg](https://lwn.net/Articles/1450196) | NA | v3 ☐ | [PatchWork v13b](https://patchwork.kernel.org/project/linux-mm/cover/20210712194551.91920-1-willy@infradead.org/) |
| 2021/07/19 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Folio support in block + iomap layers](https://lwn.net/Articles/1450196) | NA | v15 ☐ | [PatchWork v15,00/17](https://patchwork.kernel.org/project/linux-mm/cover/20210712194551.91920-1-willy@infradead.org/) |
| 2022/01/10 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Convert GUP to folios](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1b7f7e58decccb52d6bc454413e3298f1ab3a9c6) | NA | v1 ☑ 5.18-rc1 | [PatchWork 00/17](https://patchwork.kernel.org/project/linux-mm/cover/20220102215729.2943705-1-willy@infradead.org)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,00/28](https://patchwork.kernel.org/project/linux-mm/cover/20220110042406.499429-1-willy@infradead.org) |
| 2022/01/05 | Alex Shi <alexs@kernel.org> | [remove add/del page to lru functions](https://patchwork.kernel.org/project/linux-mm/cover/20220120131024.502877-1-alexs@kernel.org) | 使用了 folio 之后, LRU 后部分函数可以删除. | v1 ☐ | [PatchWork 0/5](https://patchwork.kernel.org/project/linux-mm/cover/20220120131024.502877-1-alexs@kernel.org) |

#### 1.4.1.3 Memory folios @5.19
-------

[A memory-folio update](https://lwn.net/Articles/893512)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/04/29 | Matthew Wilcox (Oracle) <willy@infradead.org> | [Folio patches for 5.19](https://patchwork.kernel.org/project/linux-mm/cover/20220429192329.3034378-1-willy@infradead.org/) | 637111 | v1 ☐☑ | [LORE v1,0/21](https://lore.kernel.org/r/20220429192329.3034378-1-willy@infradead.org)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/26](https://lore.kernel.org/r/20220504182857.4013401-1-willy@infradead.org) |
| 2022/05/27 | Miaohe Lin <linmiaohe@huawei.com> | [mm/vmscan: don't try to reclaim freed folios](https://patchwork.kernel.org/project/linux-mm/patch/20220527080451.48549-1-linmiaohe@huawei.com/) | 645512 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220527080451.48549-1-linmiaohe@huawei.com) |
| 2022/05/28 | FanJun Kong <bh1scw@gmail.com> | [mm/slub: replace alloc_pages with folio_alloc](https://patchwork.kernel.org/project/linux-mm/patch/20220528161157.3934825-1-bh1scw@gmail.com/) | 645771 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220528161157.3934825-1-bh1scw@gmail.com) |
| 2022/06/17 | Matthew Wilcox <willy@infradead.org> | [Convert the swap code to be more folio-based](https://patchwork.kernel.org/project/linux-mm/cover/20220617175020.717127-1-willy@infradead.org/) | 651513 | v1 ☐☑ | [LORE v1,0/22](https://lore.kernel.org/r/20220617175020.717127-1-willy@infradead.org) |
| 2022/06/08 | Miaohe Lin <linmiaohe@huawei.com> | [[v2] mm/vmscan: don't try to reclaim freed folios](https://patchwork.kernel.org/project/linux-mm/patch/20220608141432.23258-1-linmiaohe@huawei.com/) | 648484 | v2 ☐☑ | [LORE v2,0/1](https://lore.kernel.org/r/20220608141432.23258-1-linmiaohe@huawei.com) |


### 1.4.2 Pulling slabs out of struct page
-------

[Pulling slabs out of struct page](https://lwn.net/Articles/871982)

[Struct slab comes to 5.17](https://lwn.net/Articles/881039/)

受 Page Folio 启发的一个工作. slab 分配器使用的数据结构之前是内嵌在 struct page 中的, 将 struct slab 从 struct page 中分离出来. 与 struct folio 类似, 它总是一个 head page. 这带来了更好的类型安全性、将大型 kmalloc 分配与真正的 slab 分离以及清理.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/01/04 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Separate struct slab from struct page](https://patchwork.kernel.org/project/linux-mm/cover/20211004134650.4031813-1-willy@infradead.org) | struct page 结构定义中比较复杂的部分之一是 slab 分配器所使用的部分. 一般来说, 如果将 slab 的数据类型从 page 结构体中分离是有好处的, 而且它还有助于防止尾页滑落到任何地方. | v2 ☐ | [2021/07/15 PatchWork RFC,00/62](https://patchwork.kernel.org/project/linux-mm/cover/20211004134650.4031813-1-willy@infradead.org)<br>*-*-*-*-*-*-*-* <br>[2021/12/01 PatchWork v2,00/33](https://patchwork.kernel.org/project/linux-mm/cover/20211201181510.18784-1-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[2022/01/04 PatchWork v4,00/32](https://patchwork.kernel.org/project/linux-mm/cover/20220104001046.12263-1-vbabka@suse.cz) |
| 2021/10/12 | Johannes Weiner <hannes@cmpxchg.org> | [PageSlab: eliminate unnecessary compound_head() calls](https://patchwork.kernel.org/project/linux-mm/cover/20211012180148.1669685-1-hannes@cmpxchg.org) | 重构代码, 消除二义性, 使得代码更加简洁. PageSlab() 目前对所有调用站点施加一个 compound_head() 调用, 即使只有极少数情况会遇到尾页. 这组补丁气泡尾分辨率到少数需要它的网站, 并消除它在其他地方. 这个改动很独立, 它的灵感来自于 Willy 的补丁 Separate struct slab from struct page](https://patchwork.kernel.org/project/linux-mm/cover/20210715200030.899216-1-willy@infradead.org). 为了让逻辑更清晰, 代码更简洁. PageSlab() 的调用应该完全从对 compound_head() 的无限制调用中分离出来, 因为它们本身就有不必要的开销. | v1 ☐ | [PatchWork 00/11](https://patchwork.kernel.org/project/linux-mm/cover/20211012180148.1669685-1-hannes@cmpxchg.org), [LKML](https://lkml.org/lkml/2021/10/12/820) |
| 2022/01/04 | Vlastimil Babka <vbabka@suse.cz> | [Separate struct slab from struct page](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=07f910f9b7295b6a28b337fedb56e612684c5659) | NA | v4 ☑ [5.17-rc1](https://kernelnewbies.org/Linux_5.17#Memory_management) | [PatchWork RFC,00/32](https://patchwork.kernel.org/project/linux-mm/cover/20211116001628.24216-1-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[PatchWork v4,00/32](https://patchwork.kernel.org/project/linux-mm/cover/20220104001046.12263-1-vbabka@suse.cz) |

### 1.4.3 MEMCG Folio


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/03/16 | Matthew Wilcox <willy@infradead.org> | [[RFC] memcg: Convert mc_target.page to mc_target.folio](https://patchwork.kernel.org/project/linux-mm/patch/YjJJIrENYb1qFHzl@casper.infradead.org/) | 624009 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/YjJJIrENYb1qFHzl@casper.infradead.org) |


## 1.5 页面初始化
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/04/28 | Mel Gorman <mel@csn.ul.ie> | [Verification and debugging of memory initialisation V4](https://lore.kernel.org/patchwork/patch/114302) | 引导初始化非常复杂, 有大量特定于体系结构的例程、挂钩和代码排序. 虽然大量的初始化是独立于体系结构的, 但它信任从体系结构层接收的数据. 这是一个错误, 并导致了一些难以诊断的错误. 这个补丁集为内存初始化添加了一些验证和跟踪. 它还介绍了一些基本的防御措施. 对于嵌入式系统, 可以显式禁用验证代码. | v6 ☑ 5.2-rc1 | [PatchWork mm,v6,0/7](https://lore.kernel.org/patchwork/patch/114302) |
| 2018/12/30 | Alexander Duyck <alexander.h.duyck@linux.intel.com> | [Deferred page init improvements](https://lore.kernel.org/patchwork/patch/1019963) | 该补丁集本质上是页面初始化逻辑的重构, 旨在提供更好的代码重用, 同时显著提高延迟页面初始化性能.<br>在我对 x86_64 系统的测试中, 每个节点有 384GB 的 RAM 和 3TB 的持久内存<br>1. 在常规内存初始化的情况下, 初始化时间平均从 3.75s 减少到 1.06s. 对于持久内存, 初始化时间平均从 24.17s 下降到 19.12s.<br>2. 这相当于内存初始化性能提高了 253%, 持久内存初始化性能提高了 26%. | v6 ☑ 5.2-rc1 | [PatchWork mm,v6,0/7](https://patchwork.kernel.org/project/linux-mm/cover/154361452447.7497.1348692079883153517.stgit@ahduyck-desk1.amr.corp.intel.com) |



## 1.6 madvise
-------


| madvise 标记 | 描述 | commit | 关键 func |
|:-----------:|:----:|:------:|:--------:|
| MADV_NORMAL | 不做任何特殊处理, 这是默认操作 |
| MADV_RANDOM | 期望以随机的顺序访问 page, 这等价于告诉内核, 随机性强, 局部性弱, 预读机制意义不大 |
| MADV_SEQUENTIAL | 与 MADV_RANDOM 相反, 期望顺序的访问 page, 因此内核应该积极的预读给定范围内的 page, 并在访问过后快速释放. |
| MADV_DONTFORK | 在执行 fork (2) 后, 子进程不允许使用此范围的页面. 这样是为了避免 COW 机制导致父进程在写入页面时更改页面的物理位置. |
| MADV_DOFORK | 撤销 MADV_DONTFORK 的效果, 恢复默认行为. |
| MADV_HWPOISON | 使指定范围内的页面 "中毒", 即模拟硬件内存损坏的效果, 访问这些页面会引起与之相同处理. 此操作仅仅适用于特权 (CAP_SYS_ADMIN) 进程. 此操作将会导致进程收到 SIGBUS, 并且页面被取消映射.<br>此功能用于测试内存错误的处理代码；仅当内核配置为 CONFIG_MEMORY_FAILURE 时才可用. |
| MADV_MERGEABLE | 为指定范围内的页面启用 KSM (Kernel Samepage Merging). 内核会定期扫描那些被标记为可合并的用户内存区域, 寻找具有相同的内容的页面. 他们将被一个单一的具有写保护的页面取代, 并且要更新此页面时发生 COW 操作！KSM 仅仅用于合并私有匿名映射的页面.<br>KSM 功能适用于生成相同数据的多个实例的应用程序(例如, KVM 等虚拟化系统). KSM 会消耗大量的处理能力, 应该小心使用. 详细可以参阅内核源文件: Documentation/admin-guide/mm/ksm.rst.<br>MADV_MERGEABLE 和 MADV_UNMERGEABLE 操作仅在内核配置了 CONFIG_KSM 时才可用. |
| MADV_UNMERGEABLE | 撤消先前的 MADV_MERGEABLE 操作对指定地址范围的影响；同时恢复在指定的地址范围内合并的所有页面. |
| MADV_SOFT_OFFLINE | 将指定范围内的页面软脱机. 即保留指定范围内的所有页面, 使其脱离正常内存管理, 不再使用, 而原 page 的内容被迁移到新的页面, 对该区域的访问不受任何影响, 即 MADV_SOFT_OFFLINE 的操作效果对进程是不可见的, 并不会改变其语义.<br>此功能用于测试内存错误处理代码； 仅当内核配置为 CONFIG_MEMORY_FAILURE 时才可用. |
| MADV_HUGEPAGE | 对指定范围的页面启用透明大页 (THP). 目前, 透明大页仅仅适用于私有匿名页. 内核会定期扫描标记为大页候选的区域, 然后将其替换为大页. 当区域自然对齐到大页大小时, 内核也会直接分配大页 (参见 posix_memalign (2)).<br>此功能主要针对那些映射大范围区域, 且一次性访问内存大片区域的应用程序, 如 QEMU. 大页容易导致严重的内存浪费, 比如访问访问 1 字节内容需要映射 2MB 的内存页, 而不是 4KB 的内存页！有关更多详细信息, 请参阅 Linux 内核源文件 Documentation/admin-guide/mm/transhuge.rst<br>大多数常见的内核配置默认提供 MADV_HUGEPAGE 样式的行为, 因此通常不需要 MADV_HUGEPAGE. 它主要用于嵌入式系统, 其内核中默认情况下可能不会启用 MADV_HUGEPAGE 类似的行为. 在此类系统上, 可使用此标志来有选择地启用 THP. 每当使用 MADV_HUGEPAGE 时, 它应该始终位于可访问的内存区域中, 开发人员需要确保在启用透明大页面时不会增加应用程序的内存占用的风险.<br>MADV_HUGEPAGE 和 MADV_NOHUGEPAGE 操作仅在内核配置了 CONFIG_TRANSPARENT_HUGEPAGE 时才可用. |
| MADV_NOHUGEPAGE | 确保指定范围内的页面不会使用透明大页. |
| MADV_DONTDUMP | 从核心转储中排除指定范围的页面. 这在已知大内存区域无法用于核心转储的应用程序中很有用. MADV_DONTDUMP 的效果优先于通过 /proc/[pid]/coredump_filter 文件设置的位掩码, 请参阅 core . 注所谓核心转储, 指的是在进程发生错误时, 将进程地址空及其一些特定状态数据保存到磁盘文件中, 以供调试使用. |
| MADV_DODUMP | 撤销 MADV_DONTDUMP 的使用效果. |
| MADV_WIPEONFORK | 在 fork 之后, 子进程访问此区域内的内容, 将看到 0 填充数据, 这样做可以确保不会将敏感的数据, 如 PRNG seeds, cryptographic secrets 等传递给子进程. |
| MADV_WIPEONFORK | 同样只能操作私有匿名映射的页面 |
| MADV_WIPEONFORK | 设置的区域, 会在子进程中保留, 也就是说, 子进程的子进程依然是只能看到空白页. 此设置将会在 execve 期间被清除. |
| MADV_KEEPONFORK | 撤销 MADV_WIPEONFORK 的效果 |
| MADV_WILLNEED | 预计不久将会被访问, 因此提前预读几页是个不错的主意. |
| MADV_DONTNEED | 与 MADV_WILLNEED 相反, 预计未来长时间不会被访问, 可以认为应用程序完成了对这部分内容的访问, 因此内核可以释放与之相关的资源.<br>成功执行 MADV_DONTNEED 操作之后, 访问指定区域的语义将发生变化: 后续访问这些页面将会成功, 但是会导致从底层映射文件中重新填充内容(用于共享文件映射、共享匿名映射及 shmem 等)或者导致私有映射的零填充按需页面. 因此此操作是直接将 page 给回收了, 从对私有映射的处理来看, 与 swap 还是略微不同的.<br>需要注意的是, 当应用于共享映射时, MADV_DONTNEED 可能不会立即释放范围内的页面. 内核可以自由地选择合适的时机来释放页面. 然而, 调用进程的常驻集大小 (RSS) 将立即减少.<br>MADV_DONTNEED 不能应用于 locked pages、Huge TLB 页面或 VM_PFNMAP 页面.(标有内核内部 VM_PFNMAP 标志的页面是不由虚拟内存子系统管理的特殊内存区域. 此类页面通常由将页面映射到用户空间的设备驱动程序创建. |
| MADV_REMOVE | 释放给定范围的页面及其关联的后备存储. 这相当于在后备存储的相应字节范围内打一个洞. (参见 fallocate).<br>对指定范围的后续访问将看到包含 0 的字节.<br>指定的地址范围必须是共享、可写的映射. 不能应用于 locked pages、Huge TLB 页面或 VM_PFNMAP 页面.<br>在最初的实现中, 此标志仅仅支持 tmpfs. 从 linux 3.5 开始, 任何支持 fallocate FALLOC_FL_PUNCH_HOLE 模式的文件系统也支持 MADV_REMOVE 标志. |
| MADV_FREE | 意味着, 应用程序不再需要指定范围内的页面. 内核因此可以释放这些页面, 但不是立即释放, 而是当出现内存压力时释放. 这样就会出现一些特殊情况, 如果在页面被释放前被写入, 那么将取消释放操作, 一旦页面被释放, 那么后续访问将看到 0 填充的页面. 此外当内核释放页面时, 任何陈旧数据(即脏的、未写入的页面)都将丢失. 但是, 随后对该范围内的页面的写入将成功, 然后内核将不会释放这些脏页面, 因此调用者始终可以看到刚刚写入的数据.<br> MADV_FREE 操作只能应用于私有匿名页面 |
| MADV_COLD | 可以立即为将此区域的 page 标记为冷页, 更容易的被回收. | [mm: introduce MADV_COLD](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9c276cc65a58faf98be8e56962745ec99ab87636)| []() |
| MADV_PAGEOUT | 直接回收页面, 如果是匿名页, 那么将会被换出, 如果是文件页并且是脏页, 那么会被回写. | [mm: introduce MADV_PAGEOUT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1a4e58cce84ee88129d5d49c064bd2852b481357) |


### 1.6.1 MADV_DOEXEC
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/07/20 | Anthony Yznaga <anthony.yznaga@oracle.com> | [madvise MADV_DOEXEC](https://lore.kernel.org/patchwork/patch/1280469) | 这组补丁引入了 madvise MADV_DOEXEC 参数实现了跨 exec 金恒地址空间保留匿名页范围的支持. 与重新附加到命名共享内存段不同, 以这种方式共享内存的主要好处是确保新进程中的内存映射到与旧进程相同的虚拟地址. 这样做的目的是为使用 vfio 的 guest 保留 guest 的内存, 通过 qemu 使用 exec 可以生成一份其自身的更新版本. 通过确保内存保留在固定地址, vfio 映射及其相关的内核数据结构可以保持有效. | RFC ☐ | [PatchWork RFC,0/5](https://lore.kernel.org/patchwork/patch/1280469) |





### 1.6.2 MADV_DONTNEED
-------

标记为 MADV_DONTNEED 的页面由 madvise_dontneed_single_vma() 调用 zap_page_range() 把给定范围内的用户页释放掉, 页表清零.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/09/26 | Anthony Yznaga <anthony.yznaga@oracle.com> | [mm/madvise: support process_madvise(MADV_DONTNEED)](https://patchwork.kernel.org/project/linux-mm/cover/20210926161259.238054-1-namit@vmware.com) | 这些补丁的目标是添加对 process_madvise(MADV_DONTNEED) 的支持. 然而, 在这个过程中, 执行了一些(可以说)有用的清理、bug修复和性能增强. 这些补丁试图整合不同行为之间的逻辑, 并在一定程度上解决与之前[比较出彩的补丁](https://lore.kernel.org/linux-mm/CAJuCfpFDBJ_W1y2tqAT4BGtPbWrjjDud_JuKO8ZbnjYfeVNvRg@mail.gmail.com) 的冲突.<br>process_madvise(MADV_DONTNEED) 之所以有用有两个原因:<br>(a)它允许 userfaultfd 监视器从被监控的进程中取消内存映射;<br>(b) 它 比 madvise() 更有效, 因为它是矢量化的, 批处理 TLB 刷新更积极. | RFC ☐ | [PatchWork RFC,0/8](https://patchwork.kernel.org/project/linux-mm/cover/20210926161259.238054-1-namit@vmware.com) |
| 2022/03/04 | Johannes Weiner <hannes@cmpxchg.org> | [mm: madvise: MADV_DONTNEED_LOCKED](https://lore.kernel.org/all/20220304171912.305060-1-hannes@cmpxchg.org) | 20220304171912.305060-1-hannes@cmpxchg.org | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20220304171912.305060-1-hannes@cmpxchg.org) |

### 1.6.3 madvise MADV_FREE 页面延迟回收
-------

[Volatile ranges and MADV_FREE](https://lwn.net/Articles/590991)

#### 1.6.3.1 Volatile Ranges
-------

[LWN: Volatile ranges](https://lwn.net/Kernel/Index/#Volatile_ranges)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2011/11/21 | John Stultz <john.stultz@linaro.org> | [`fadvise: Add _VOLATILE,_ISVOLATILE, and _NONVOLATILE flags`](https://lore.kernel.org/all/1321932788-18043-1-git-send-email-john.stultz@linaro.org) | [POSIX_FADV_VOLATILE](https://lwn.net/Articles/468896) | v1 ☐ | [LORE](https://lore.kernel.org/all/1321932788-18043-1-git-send-email-john.stultz@linaro.org) |
| 2012/05/25 | John Stultz <john.stultz@linaro.org> | [Fallocate Volatile Ranges](https://lore.kernel.org/all/1337973456-19533-1-git-send-email-john.stultz@linaro.org) | [Volatile ranges with fallocate()](https://lwn.net/Articles/500382) | v1 ☐ | [LORE v1,0/4](https://lore.kernel.org/all/1337973456-19533-1-git-send-email-john.stultz@linaro.org) |
| 2014/03/14 | John Stultz <john.stultz@linaro.org> | [Volatile Ranges (v11)](https://lore.kernel.org/all/1394822013-23804-1-git-send-email-john.stultz@linaro.org) | 1394822013-23804-1-git-send-email-john.stultz@linaro.org | v11 ☐ | [LORE v11,0/3](https://lore.kernel.org/all/1394822013-23804-1-git-send-email-john.stultz@linaro.org) |

#### 1.6.3.2 MADV_FREE
-------

与 MADV_DONTNEED 不同, MADV_DONTNEED 会立即回收指定内存区域的的页面, 而 MADV_FREE 则实现了一种延迟回收页面的机制.

应用程序通过 madvise MADV_FREE 告诉内核这些内存不再包含有用的数据, 这样有诸多好处:

1.  如果内存压力发生, 内核可以丢弃释放的页面, 而不是交换或 OOM. 如果内核中其他地方有使用内存的需求, 可以被内核回收. 在内存不足的情况下, 内核仍然知道要回收哪些页面. 通过检查页面表的脏位, 如果发现仍然是 "干净" 的, 这意味着这是一个 "空闲页面", 内核可以直接释放页面, 而不是交换出去或者 OOM.

2.  如果没有内存压力, 这些延缓释放的页面可以被用户空间重用, 而不会产生额外的开销. 例如如下场景, 如果应用程序又重新分配了内存, 内核可以直接复用这块区域, 此时对页面进行写操作, 可以直接将新数据放入页面, 并将页面标记为 DIRTY, 而无需通过触发 Page Fault 来分配页面. 这使得应用程序先 free() 后继续使用 malloc() 处理相同数据的应用程序运行得更快, 因为避免了 Page Fault.

MADV_FREE 的合入在 linux 上经历了漫长的岁月.

2007 年 Rik van Riel 实现了最早的 MADV_FREE [MM: implement MADV_FREE lazy freeing of anonymous memory](https://lkml.org/lkml/2007/4/28/6). 这引发了关于 [MADV_FREE functionality](https://lkml.org/lkml/2007/4/30/565) 的讨论, 以及 [wrong madvise(MADV_DONTNEED) semantic](https://lkml.org/lkml/2005/6/28/188). 但是还有诸多工作要做.

随后 2014 年开始, Minchan Kim 继续了 MADV_FREE 的工作. 此时类似的特性已经被 BSD 等内核所支持, 但是 linux 仍旧不支持.

处理流程上, madvise MADV_FREE 的操作会调用 [madvise_free_single_vma()](https://elixir.bootlin.com/linux/v4.5/source/mm/madvise.c#L410) 将当前 VMA 上指定 start_addr, end_addr 区域页面标记为 lazyfree 的. 对于 lazyfree 的页面, LRU 会通过 [deactivate_page()](https://elixir.bootlin.com/linux/v4.5/source/mm/swap.c#L644) 和 [lru_deactivate_pvecs](https://elixir.bootlin.com/linux/v4.5/source/mm/swap.c#L647) 移动到 【inactive file list 上](https://elixir.bootlin.com/linux/v4.12/source/mm/swap.c#L591)(注意不管之前的页面是不是文件页, 都会将其引入 inactive file list). 其中 deactivate_page() 随后被改名为 [mark_page_lazyfree()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f7ad2a6cb9f7c4040004bedee84a70a9b985583e).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/04/28 | Rik van Riel <riel@redhat.com> | [MM: implement MADV_FREE lazy freeing of anonymous memory](https://lore.kernel.org/patchwork/patch/79624) | madvise 支持页面延迟回收(MADV_FREE)的早期尝试. 通过 MADV_FREE 延迟释放匿名页面, 在四核系统上 MySQL sysbench 的性能提高了一倍多. | v5 ☐ | [PatchWork RFC](https://lore.kernel.org/lkml/4632D0EF.9050701@redhat.com) |
| 2014/07/18 | Minchan Kim <minchan@kernel.org> | [MADV_FREE support](https://lore.kernel.org/patchwork/patch/484703) | madvise 可以用来设置页面的属性, MADV_FREE 则将这些页标识为延迟回收, 在页面用不着的时候, 可能并不会立即释放<br>1. 当内核内存紧张时, 这些页将会被优先回收, 如果应用程序在页回收后又再次访问, 内核将会返回一个新的并设置为 0 的页.<br>2. 而如果内核内存充裕时, 标识为 MADV_FREE 的页会仍然存在, 后续的访问会清掉延迟释放的标志位并正常读取原来的数据, 因此应用程序不检查页的数据, 就无法知道页的数据是否已经被丢弃. | v13 ☐ | [2014/03/14 LORE RFC,0/6](https://lore.kernel.org/lkml/1394779070-8545-1-git-send-email-minchan@kernel.org)<br>*-*-*-*-*-*-*-*<br>[2014/03/20 LORE v2,0/3](https://lore.kernel.org/lkml/1395297538-10491-1-git-send-email-minchan@kernel.org)<br>*-*-*-*-*-*-*-*<br>[2014/07/18 LORE v13,0/8](https://lore.kernel.org/lkml/1405666386-15095-1-git-send-email-minchan@kernel.org) |
| 2015/12/30 | Minchan Kim <minchan@kernel.org> | [MADV_FREE support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=05ee26d9e7e29ab026995eab79be3c6e8351908c) | madvise 支持页面延迟回收(MADV_FREE)的再一次尝试  | v5 ☑ [4.5-rc1](https://kernelnewbies.org/Linux_4.5#Add_MADV_FREE_flag_to_madvise.282.29) | [LORE v5,00/12](https://lore.kernel.org/lkml/1448865583-2446-1-git-send-email-minchan@kernel.org) |
| 2017/02/24 | MShaohua Li <shli@fb.com> | [mm: fix some MADV_FREE issues](https://lore.kernel.org/patchwork/patch/622178) | MADV_FREE 有几个问题, 使它不能在 jemalloc 这样的库中使用: 不支持系统没有交换启用, 增加了内存的压力, 另外统计也存在问题. 这个版本将 MADV_FREE 页面放到 LRU_INACTIVE_FILE 列表中, 为无交换系统启用 MADV_FREE, 并改进了统计计费.  | v5 ☑ [4.12-rc1](https://kernelnewbies.org/Linux_4.12#Memory_management) | [PatchWork v5,0/6](https://lore.kernel.org/all/cover.1487965799.git.shli@fb.com) |

### 1.6.4 MADV_COLD and MADV_PAGEOUT
-------

*   [MADV_COLD](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9c276cc65a58faf98be8e56962745ec99ab87636)

MADV_COLD 在某种程度上类似于 MADV_FREE, 它暗示内核当前不需要内存区域, 应该在内存压力上升时回收该内存区域. 这有助于帮助内核在内存紧张的情况下决定提前释放哪些页面. 但是与 MADV_FREE 不同的是, 它不会将匿名页面添加到 inactive file list 的头部. 因此它总是将匿名页面或者文件页面从 active list 移动到对应的 inactive list.

MADV_FREE 意味着在内存压力下可以丢弃, 因为页面的内容是垃圾(garbage), 不需要 SWAP, 可以直接丢弃, 所以释放这些页面的开销几乎为零. 因此, 将这些可自由使用的页面放在 inactive file LRU list 中以与其他使用过的页面竞争是有意义的. 因为在它被写入数据重新脏化之前, 它不会再被交换回内存. 甚至可以在没有开启 SWAP 的情况下使用.

然而, MADV_COLD 并不意味着垃圾, 所以回收它们最终需要换入/换出, 所以成本更大. 由于我们设计了基于成本模型的 VM LRU 老化(VM LRU aging based on cost-model), 将匿名冷页面放到不活跃的 inactive anon LRU list , 而不是 inactive file LRU list 更好. 此外, 如果系统没有 SWAP, 它将有助于避免不必要的扫描. 这样的实现简单而高效. 但是, 也要记住, 带有大量 Page Cache 的工作负载很可能会忽略匿名内存上的 MADV_COLD, 因为我们很少老化匿名 LRU 列表.


或者说, 理论上讲: 与具有相同访问频率的系统中的页面相比, 标记为 MADV_COLD 的页面将被视为最近访问较少的页面. 与 MADV_FREE 不同的是, 该区域的内容会被保留, 而不管后续如何写入页面. madvise_cold_page_range() 中通过 deactivate_page() 将页面从 active LRU 移动对对应的 inactive LRU.


*   [MADV_PAGEOUT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1a4e58cce84ee88129d5d49c064bd2852b481357)

MADV_PAGEOUT 在某种程度上类似于 MADV_DONTNEED, 它提示内核当前不需要内存区域, 应该立即回收. madvise_pageout_page_range() 会通过 isolate_lru_page() 先将页面从 LRU 中移除, 然后调用 reclaim_pages() 直接回收.


注意 MADV_COLD 和 MADV_PAGEOUT 无法应用于锁定页面、大型 TLB 页面或 VM_PFNMAP 页面, 但是支持透明大页 THP.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/07/14 | Minchan Kim <minchan@kernel.org> | [Introduce MADV_COLD and MADV_PAGEOUT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9c276cc65a58faf98be8e56962745ec99ab87636) | 为了优化 Android 中 OOM, 允许用户空间通过利用平台信息主动回收整个流程. 这允许开发人员通过自己对应用的了解绕过内核的 LRU 的不准确性, 对于那些已知从用户空间冷的页面, 并通过在应用程序进入缓存状态时立即回收它们来避免与 LMKD 竞争. 此外, 它还为平台提供了利用大量信息优化内存效率的机会. 为了实现这个目标, 补丁集为 madvise 引入了两个新选项, MADV_COLD 和 MADV_PAGEOUT, 来快速回收指定的内存页面. MADV_COLD 会把指定的 page 移到 inactive list, 基本上就是把它们标记为没人使用状态, 可以在 page reclaim 的时候释放. MADV_PAGEOUT 则更激进, 这会让这些 page 立刻被释放回收. | v5 ☑ 5.4-rc1 | [PatchWork v5,0/5](https://patchwork.kernel.org/project/linux-mm/cover/20190714233401.36909-1-minchan@kernel.org) |
| 2021/10/19 | Suren Baghdasaryan <surenb@google.com> | [mm: rearrange madvise code to allow for reuse](https://patchwork.kernel.org/project/linux-mm/patch/20211019215511.3771969-1-surenb@google.com) | 重构 madvise 系统调用, 以允许影响 vma 的 prctl 系统调用重用其中的一部分. 将遍历虚拟地址范围内 vma 的代码移动到以函数指针为参数的函数中. 目前唯一的调用者是 sys_madvise, 它使用它在每个 vma 上调用 madvise_vma_behavior. | v11 ☐ | [PatchWork v11,1/3](https://patchwork.kernel.org/project/linux-mm/cover/20190714233401.36909-1-minchan@kernel.org) |


## 1.7 page table pages
-------

### 1.7.1 Quicklist
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/03/23 | Christoph Lameter <clameter@sgi.com> | [Quicklists for page table pages V4](https://lore.kernel.org/patchwork/patch/76916) | NA | v1 ☐ | [PatchWork v1](https://lore.kernel.org/patchwork/patch/76097)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/76244)<br>*-*-*-*-*-*-*-* <br> [PatchWork v3](https://lore.kernel.org/patchwork/patch/76702)<br>*-*-*-*-*-*-*-* <br>[PatchWork v4](https://lore.kernel.org/patchwork/patch/76916)<br>*-*-*-*-*-*-*-* <br>[PatchWork v5](https://lore.kernel.org/patchwork/patch/78116) |
| 2019/08/08 | Christoph Lameter <clameter@sgi.com> | [mm: remove quicklist page table caches](https://lore.kernel.org/patchwork/patch/1112468) | 内核提前直接映射好足够的 PTE 级别的页面, 引入 `__GFP_PTE_MAPPED` 标志, 当使用此标记分配 order 为 0 页面时, 将在直接映射中的 PTE 级别进行映射.<br>目前只是分配页表时使用 `__GFP_PTE_MAPPED` 分配页表, 以便它们在直接映射中具有 4K PTE. | v1  ☐ | [PatchWork v5](https://lore.kernel.org/patchwork/patch/1112468) |


### 1.7.2 special permsissions
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/11/20 | Nadav Amit <namit@vmware.com> | [New permission vmalloc interface](https://lore.kernel.org/all/20190426001143.4983-1-namit@vmware.com) | 20201120202426.18009-1-rick.p.edgecombe@intel.com | v1 ☑✓ |[LORE v5,00/23](https://lore.kernel.org/all/20190426001143.4983-1-namit@vmware.com) |

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/11/20 | Rick Edgecombe <rick.p.edgecombe@intel.com> | [New permission vmalloc interface](https://lore.kernel.org/all/20201120202426.18009-1-rick.p.edgecombe@intel.com) | 20201120202426.18009-1-rick.p.edgecombe@intel.com | v1 ☐☑✓ |[LORE RFC,00/10](https://lore.kernel.org/all/20201120202426.18009-1-rick.p.edgecombe@intel.com) |
| 2021/08/23 | Mike Rapoport <rppt@linux.ibm.com> | [mm/page_alloc: cache pte-mapped allocations](https://lore.kernel.org/all/20210823132513.15836-1-rppt@kernel.org) | NA | v1  ☐ | [PatchWork RFC,0/4](https://lore.kernel.org/all/20210823132513.15836-1-rppt@kernel.org) |
| 2022/01/27 | Mike Rapoport <rppt@linux.ibm.com> | [Prototype for direct map awareness in page allocator](https://patchwork.kernel.org/project/linux-mm/cover/20220127085608.306306-1-rppt@kernel.org) | 让页面分配器知道 direct map 布局, 并允许在 direct map 中对必须在 PTE 级别映射的页面进行分组. | RFC ☐ | [PatchWork RFC,0/3](https://patchwork.kernel.org/project/linux-mm/cover/20220127085608.306306-1-rppt@kernel.org) |


### 1.7.3 page table check
-------

[Page Table Check Feature Merged For Linux 5.17 To Help Fight Memory Corruption](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.17-Page-Table-Check)

[Google Proposes "Page Table Check" For Fighting Some Types Of Linux Memory Corruption](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Page-Table-Check-RFC)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/01/26 | Pasha Tatashin <pasha.tatashin@soleen.com> | [Hardening page _refcount](https://patchwork.kernel.org/project/linux-mm/cover/20211026173822.502506-1-pasha.tatashin@soleen.com) | 目前很难从根本上解决 `_refcount` 问题, 因为它们通常在损坏发生后才会显现出来. 然而, 它们可能导致灾难性的故障, 如内存损坏.<br>通过添加更多的检查来提高可调试性, 确保 `page->_refcount` 永远不会变成负数(例如, 双空闲不发生, 或冻结后空闲等).<br>1. 增加了对 `_refcount` 异常值的检测.<br>2. 删除了 set_page_count(), 这样就不会无条件地用不受限制的值覆盖 `_refcount` | RFC,0/8 ☐ | [PatchWork RFC,0/8](https://patchwork.kernel.org/project/linux-mm/cover/20211026173822.502506-1-pasha.tatashin@soleen.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC,v2,00/10](https://patchwork.kernel.org/project/linux-mm/cover/20211117012059.141450-1-pasha.tatashin@soleen.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/9](https://patchwork.kernel.org/project/linux-mm/cover/20211221150140.988298-1-pasha.tatashin@soleen.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3,0/9](https://lore.kernel.org/r/20220126183429.1840447-1-pasha.tatashin@soleen.com) |
| 2021/12/21 | Pasha Tatashin <pasha.tatashin@soleen.com> | [page table check](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d283d422c6c4f0264fe8ecf5ae80036bf73f4594) | NA | v3 ☐☑✓ | [LORE v3,0/4](https://lore.kernel.org/all/20211221154650.1047963-1-pasha.tatashin@soleen.com) |
| 2022/04/21 | Tong Tiangen <tongtiangen@huawei.com> | [mm: page_table_check: add support on arm64 and riscv](https://patchwork.kernel.org/project/linux-mm/cover/20220317141203.3646253-1-tongtiangen@huawei.com) | 页面表检查通过将新页面的页面表条目(PTE, PMD 等)添加到表中, 在用户空间访问新页面时执行额外的验证 X86 支持它.<br>这个补丁集做了一些简单的更改, 使其更容易支持新的体系结构, 然后我们在 ARM64 和 RICV 上支持这个功能. | v1 ☐☑ | [LORE v1,0/4](https://lore.kernel.org/r/20220317141203.3646253-1-tongtiangen@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/4](https://lore.kernel.org/r/20220322144447.3563146-1-tongtiangen@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/4](https://lore.kernel.org/r/20220418034444.520928-1-tongtiangen@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/5](https://lore.kernel.org/r/20220421082042.1167967-1-tongtiangen@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/6](https://lore.kernel.org/r/20220507110114.4128854-1-tongtiangen@huawei.com) |
| 2022/05/31 | Matthew Wilcox <willy@infradead.org> | [Allocate and free frozen pages](https://patchwork.kernel.org/project/linux-mm/cover/20220531150611.1303156-1-willy@infradead.org/) | 我们已经有了冻结页面的能力(安全地将其引用计数减少到 0). 一些用户(如 slab)希望能够分配冻结的页面并避免触及引用计数. 它还可以避免在这些页面上进行虚假的临时引用. | v1 ☐☑ | [LORE v1,0/6](https://lore.kernel.org/r/20220531150611.1303156-1-willy@infradead.org) |

### 1.7.4 Local Page Tables
-------


#### 1.7.4.1 [Mitosis: Transparently Replicating Page Tables](https://research.vmware.com/projects/mitosis-transparently-self-replicating-page-tables)
-------

传统的 NUMA Balacning 当前策略只涉及: Memory Migration(将进程的私有内存迁移到进程所在的 NUMA Node 上) 和 Task Placement(将进程迁移到其频繁访问的内存所在的 NUMA Node 上). 但是页表本身也是在内存中存储的, 迁移的过程中, 并没有考虑对页表进行迁移, 因此页表可能在远端节点上. 这样如果 TLB 都是 Hint 的, 倒是影响不太大, 但是如果存在大量的 TLB Miss, 那么每次 Page Table Walk 都不得不去访问访问远端的页表.

那么理论上, NUMA 系统下, 页表在内存中的存储位置, 对性能也有较大的影响. Mitosis 团队测试发现, 在 4 插槽 Intel Haswell 机器上, 对于 HPCC RandomAccess 基准测试工作负载, 由于页表放置在远端的 NUMA NODE 上导致的性能下降可能高达 3.4 倍. 虽然此工作负载的设计目的是具有较高的缓存未命中率, 但在 Redis 等 Key-Value 数据库也可以看到类似的效果, 在最坏的情况下, 性能下降可能高达 2 倍.

为了减少 NUMA 机器上页表行走的远程访问开销, [Mitosis](https://www.cs.yale.edu/homes/abhishek/reto-osdi18.pdf) 设计了一种用于透明自复制页表的技术. 通过更改页表分配和管理子系统, 在 NUMA 节点上完全复制页表. 可以在每个进程的基础上启用页表复制, 从而为进程运行的每个 NUMA 节点创建和维护一个副本. 当进程计划在内核上运行时, 它会用本地 NUMA 节点的页表副本的物理地址写入内核的页表指针(x86 处理器上的 CR3 寄存器). 每次操作系统修改页面表时, 我们都会确保更新有效地传播到所有副本页面表, 并基于所有副本(包括硬件更新的脏位和访问位)返回一致的值.

测试表明, Mitosis 可以完全缓解 HPCC RandomAccess 的性能劣化, 并将其他单线程工作负载分别提高 30% 和 15%.

随后, 2021 年作者所在团队进一步扩展了 Mitosis 的设计, 以支持虚拟化环境. 通过支持 KVM 扩展, 以提高在虚拟化系统中运行的应用程序的性能. 在具有硬件支持(扩展页表)的虚拟环境中, 处理 TLB 未命中比在本机情况下的开销更高, 因为所需的 2D 页表遍历最多引入 24 次内存访问来解决单个 TLB 未命中. 通过修改虚拟机监控程序, 以便在 guest 操作系统下透明地执行复制, 从而使未修改的 guest 操作系统能够在 VM 中运行.

[Mitosis 公开地址](https://gandhijayneel.github.io/mitosis)

github 地址: [Mitosis Project](https://github.com/mitosis-project), [linux 内核](https://github.com/gandhijayneel/mitosis-linux-release), [numactl](https://github.com/gandhijayneel/mitosis-numactl-release)

| 时间线 | 相关论文 |
|:-----:|:-------:|
| 2019 | [Mitosis: Transparently Self-Replicating Page-Tables for Large-Memory Machines; October, 2019; 1910.05398.pdf](https://research.vmware.com/files/attachments/0/0/0/0/0/9/5/1910.05398.pdf) |
| 2020 | [Mitosis: Transparently Self-Replicating Page-Tables for Large-Memory Machines; March, 2020; aspl0359a-achermanna.pdf](https://research.vmware.com/files/attachments/0/0/0/0/1/0/3/aspl0359a-achermanna.pdf) |
| 2021 | [Fast Local Page-Tables for Virtualized NUMA Servers with vMitosis; April, 2021; asplos21_vmitosis.pdf](https://research.vmware.com/files/attachments/0/0/0/0/1/3/8/asplos21_vmitosis.pdf)<br>[Fast Local Page-Tables for Virtualized NUMA Servers with vMitosis; April, 2021; vmitosis_ext_abstract.pdf](https://research.vmware.com/files/attachments/0/0/0/0/1/3/1/vmitosis_ext_abstract.pdf) |


### 1.7.5 Shared Page Table
-------


Linux 进程使用不同的虚拟地址空间. 因此, 管理该地址空间状态的页表是每个进程专用的. 因此, 如果两个进程具有到物理内存中同一页的映射, 则每个进程都将具有该页的独立页表条目. 因此, PTE 的开销随着映射每个页面的进程数而线性增加. 虽然页表条目(PTE)相对较小, 在大多数系统上, 仅需要 8 个字节即可引用 4096 字节的页面. 这看起来貌似并不会浪费多少内存空间.

但总会有人在那里做古怪的事情, 在 Oracle 的一个业务场景下: 在具有 300GB SGA [Oracle 系统全局区域] 的数据库服务器上, 当 1500 多个客户端尝试共享此 SGA 时, 即使系统具有 512GB 内存, 也会出现 OOM, 从而导致系统崩溃. 在此服务器上, 在最坏的情况下, 映射来自 SGA 的每个页面的所有 1500 个进程中, 仅 PTE 就需要 878GB+. 如果可以共享这些 PTE, 则节省的内存量非常大.

在 2022 年 5 月份的 Linux 存储、文件系统、内存管理和 BPF 峰会上对此进行了讨论. 当时, Aziz 正在提议一个新的系统调用(mshare()) 来实现 PTE 页表的共享. 参见 [LWN 报道--Sharing page tables with mshare()](https://lwn.net/Articles/895217).

随后经过讨论, v2 补丁集已更改此接口, 现在不需要新的系统调用. 而是提供了一个内核虚拟文件系统(msharefs), 参见 [LWN 报道--Sharing page tables with msharefs](https://lwn.net/Articles/901059). 它应该安装在 /sys/fs/mshare 上. 通过在 `/sys/fs/mshare` 下创建一个文件, 然后使用 mmap() 将该文件映射到进程的地址空间. 传递给 mmap() 的大小将决定生成的内存共享区域的大小.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/01/18 | Khalid Aziz <khalid.aziz@oracle.com> | [Add support for shared PTEs across processes](https://patchwork.kernel.org/project/linux-mm/cover/cover.1642526745.git.khalid.aziz@oracle.com) | 内核中的页表会消耗一些内存, 只要要维护的映射数量足够小, 那么页表所消耗的空间是可以接受的. 当进程之间共享的内存页很少时, 要维护的页表条目(PTE)的数量主要受到系统中内存页的数量的限制. 但是随着共享页面的数量和共享页面的次数的增加, 页表所消耗的内存数量开始变得非常大.<br>比如在一些实际业务中, 通常会看到非常多的进程共享内存页面. 在 x86_64 上, 每个页面页面在每个进程空间都需要占用一个只有 8Byte 大小的 PTE, 共享此页面的进程数目越多, 占用的内存会非常的大. 如果这些 PTE 可以共享, 那么节省的内存数量将非常可观.<br>这组补丁在内核中实现一种机制, 允许用户空间进程选择共享 PTE. 一个进程可以通过 通过 mshare() 和 mshare_unlink() syscall 来创建一个 mshare 区域(mshare'd region), 这个区域可以被其他进程使用共享 PTE 映射相同的页面. 其他进程可以通过 mashare() 使用共享 PTE 将共享页面映射到它们的地址空间. 然后还可以通过 mshare_unlink() syscall 来结束对共享页面的访问. 当最后一个访问 mshare'd region 的进程调用 mshare_unlink() 时, mshare'd region就会被销毁, 所使用的内存也会被释放. | v1 ☐ | [LKML RFC,0/6](https://patchwork.kernel.org/project/linux-mm/cover/cover.1642526745.git.khalid.aziz@oracle.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/14](https://lore.kernel.org/all/cover.1649370874.git.khalid.aziz@oracle.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/9](https://lore.kernel.org/r/cover.1656531090.git.khalid.aziz@oracle.com) |

### 1.7.5 Reclaim unused page-table pages
-------

| 日期 | LWN | 翻译 |
|:---:|:----:|:---:|
| 2022/05/09 | [Ways to reclaim unused page-table pages](https://lwn.net/Articles/893726) | NA |

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/10 | Qi Zheng <zhengqi.arch@bytedance.com> | [Free user PTE page table pages](https://patchwork.kernel.org/project/linux-mm/cover/20210819031858.98043-1-zhengqi.arch@bytedance.com) | 这个补丁系列的目的是在所有 PTE 条目都为空时释放用户 PTE 页表页面.<br>一些malloc库(例如 jemalloc 或 tcmalloc) 通常通过 mmap() 分配 VAs 的数量, 而不取消这些 VAs 的映射. 如果需要, 他们将使用 madvise(MADV_DONTNEED) 来释放物理内存. 但是 madvise() 不会释放页表, 因此当进程接触到巨大的虚拟地址空间时, 它会生成许多页表.<br>PTE 页表占用大量内存的原因是 madvise(MADV_DONTNEED) 只清空 PTE 并释放物理内存, 但不释放 PTE 页表页. 这组补丁通过释放那些空的 PTE 页表来节省内存. | v1 ☐ | [PatchWork 0/7](https://lore.kernel.org/patchwork/patch/1461972)<br>*-*-*-*-*-*-*-* <br>[2021/08/19 PatchWork v2,0/9](https://patchwork.kernel.org/project/linux-mm/cover/20210819031858.98043-1-zhengqi.arch@bytedance.com)<br>*-*-*-*-*-*-*-* <br>[2021/11/10 PatchWork v3,00/15](https://patchwork.kernel.org/project/linux-mm/cover/20211110084057.27676-1-zhengqi.arch@bytedance.com) |
| 2022/04/29 | Qi Zheng <zhengqi.arch@bytedance.com> | [Try to free user PTE page table pages](https://patchwork.kernel.org/project/linux-mm/cover/20220429133552.33768-1-zhengqi.arch@bytedance.com/) | 636959 | v1 ☐☑ | [LORE v1,0/18](https://lore.kernel.org/r/20220429133552.33768-1-zhengqi.arch@bytedance.com) |

## 1.8 安全
-------

### 1.8.1 PKS
-------

页表是许多类型保护的基础, 因此是攻击者的攻击目标. 将它们映射为只读将使它们更难在攻击中使用. 内核开发者提出了通过 PKS 来对内核页表进行写保护. 这可以防止攻击者获得写入页表的能力. 这并不是万无一失的. 因为能够执行任意代码的攻击者可以直接禁用 PKS. 或者简单地调用内核用于合法页表写入的相同函数.

[PKS](https://lore.kernel.org/lkml/20210401225833.566238-1-ira.weiny@intel.com) 是即将推出的 CPU 功能, 它允许在不刷新 TLB 的情况下更改监控器虚拟内存权限, 就像 PKU 对用户内存所做的那样. 保护页表通常会非常昂贵, 因为您必须通过分页本身来实现. PKS 提供了一种切换页面表可写性的方法, 这只需要 MSR 操作即可完成.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/09/04 | Rick Edgecombe <rick.p.edgecombe@intel.com> | [arm64: Memory Tagging Extension user-space support](https://patchwork.kernel.org/project/linux-mm/cover/20200904103029.32083-1-catalin.marinas@arm.com) | 使用 [PKS(Protection Keys for Supervisor)]() 对页表进行写保护. 其基本思想是使页表成为只读的, 除非在需要修改页表时临时基于每个 cpu 来修改. | v1  ☐ | [PatchWork RFC,0/4](https://patchwork.kernel.org/project/linux-mm/cover/20200904103029.32083-1-catalin.marinas@arm.com) |

#### 1.8.2 内存标签扩展(Arm v8.5 memory tagging extension-MTE)
-------

[MTE技术在Android上的应用](https://zhuanlan.zhihu.com/p/353807709)

[Memory Tagging Extension (MTE) 简介(一)](https://blog.csdn.net/weixin_47569031/article/details/114694733)

[LWN: Arm64的内存标记扩展功能！](https://blog.csdn.net/Linux_Everything/article/details/109396397)


MTE(Memory Tag) 是 ARMV8.5 增加一个硬件特性, 主要用于内存安全. 通过硬件 Arch64 MTE 和 compiler 辅助, 可以增加 64bit 进程的内存安全.

MTE 实现了锁和密钥访问内存. 这样在内存访问期间, 可以在内存和密钥上设置锁. 如果钥匙与锁匹配, 则允许进入. 如果不匹配, 则报告错误. 通俗讲就是为每个分配内存都打上一个 TAG(可以认为是访问内存的密钥或者键值), 当通过指针地址访问内存的时候, 硬件会比较指针里面的 TAG 与实际内存 TAG 是否匹配, 如果不匹配则检测出错误.


通过在物理内存的每个 16 字节中添加 4 位元数据来标记内存位置. 这是标签颗粒. 标记内存实现了锁. 指针和虚拟地址都被修改为包含键. 为了实现关键位而不需要更大的指针, MTE 使用了 ARMv8-A 体系结构的 Top Byte Ignore (TBI) 特性. 当启用 TBI 时, 当将虚拟地址作为地址转换的输入时, 虚拟地址的上字节将被忽略.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/09/04 | Catalin Marinas <catalin.marinas@arm.com> | [arm64: Memory Tagging Extension user-space support](https://patchwork.kernel.org/project/linux-mm/cover/20200904103029.32083-1-catalin.marinas@arm.com) | NA | v9 ☑ 5.10-rc1 | [2019/12/11 PatchWork 00/22](https://patchwork.kernel.org/project/linux-mm/cover/20191211184027.20130-1-catalin.marinas@arm.com)<br>*-*-*-*-*-*-*-* <br>[2020/09/04 PatchWork v9,00/29](https://patchwork.kernel.org/project/linux-mm/cover/20200904103029.32083-1-catalin.marinas@arm.com) |

#### 1.8.3 Linear Address Masking
-------

[Intel Preparing Linear Address Masking Support (LAM)](https://www.phoronix.com/scan.php?page=news_item&px=Intel-LAM-Glibc)

[Intel Gets Back To Working On Linear Address Masking Support For The Linux Kernel](https://www.phoronix.com/scan.php?page=news_item&px=Intel-LAM-Linux-Kernel-May-2022)

[Intel Revs Its Linear Address Masking Patches For Linux](https://www.phoronix.com/scan.php?page=news_item&px=Intel-LAM-Linux-v5)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/02/05 | "Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [Linear Address Masking enabling](https://patchwork.kernel.org/project/linux-mm/cover/20210205151631.43511-1-kirill.shutemov@linux.intel.com) | [线性地址屏蔽(LAM)](https://software.intel.com/content/dam/develop/external/us/en/documents-tps/architecture-instruction-set-extensions-programming-reference.pdf) 修改应用于 64 位线性地址的检查, 允许软件将未翻译的地址位用于元数据. 手册参见 [ISE, Chapter 14](https://patchwork.kernel.org/project/linux-mm/cover/20210205151631.43511-1-kirill.shutemov@linux.intel.com). 代码参见 [kas/linux.git](https://git.kernel.org/pub/scm/linux/kernel/git/kas/linux.git/log/?h=lam). | RFC ☐ | [PatchWork RFC,0/9](https://patchwork.kernel.org/project/linux-mm/cover/20210205151631.43511-1-kirill.shutemov@linux.intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/8](https://lore.kernel.org/r/20220610143527.22974-1-kirill.shutemov@linux.intel.com) |


## 1.9 page attributes
-------

## 1.9.1 CPA(Change Page Attribute)
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/09/17 | Srivatsa S. Bhat <srivatsa.bhat@linux.vnet.ibm.com> | [x86/mm/cpa: Improve large page preservation handling](https://lore.kernel.org/patchwork/patch/987147) | 优化 页面属性(CPA) 代码中的 try_preserve_large_page(), 降低 CPU 消耗. | v3 ☑ 4.20-rc1 | [PatchWork RFC v3](https://lore.kernel.org/patchwork/patch/987147) |


## 1.10 其他页面页表相关
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/04/28 | Matthew Wilcox <willy@infradead.org> | [Record the mm_struct in the page table pages](https://lore.kernel.org/patchwork/patch/1232723) | NA| v1 ☐ | [PatchWork 0/6](https://lore.kernel.org/patchwork/patch/1232723) |
| 2022/02/14 | David Hildenbrand <david@redhat.com> | [mm: enforce pageblock_order < MAX_ORDER](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=b3d40a2b6d10c9d0424d2b398bf962fb6adad87e) | 20220214174132.219303-1-david@redhat.com | v1 ☑✓ 5.18-rc1 | [LORE v1,0/2](https://lore.kernel.org/all/20220214174132.219303-1-david@redhat.com) |


# 2 内存分配
-------



每个内存管理器都使用了一种基于堆的分配策略. 在这种方法中, 大块内存(称为 堆)用来为用户定义的目的提供内存. 当用户需要一块内存时, 就请求给自己分配一定大小的内存. 堆管理器会查看可用内存的情况(使用特定算法)并返回一块内存. 搜索过程中使用的一些算法有first-fit(在堆中搜索到的第一个满足请求的内存块) 和 best-fit(使用堆中满足请求的最合适的内存块). 当用户使用完内存后, 就将内存返回给堆.

这种基于堆的分配策略的根本问题是碎片(fragmentation). 当内存块被分配后, 它们会以不同的顺序在不同的时间返回. 这样会在堆中留下一些洞, 需要花一些时间才能有效地管理空闲内存. 这种算法通常具有较高的内存使用效率(分配需要的内存), 但是却需要花费更多时间来对堆进行管理.

另外一种方法称为 buddy memory allocation, 是一种更快的内存分配技术, 它将内存划分为 2 的幂次方个分区, 并使用 best-fit 方法来分配内存请求. 当用户释放内存时, 就会检查 buddy 块, 查看其相邻的内存块是否也已经被释放. 如果是的话, 将合并内存块以最小化内存碎片. 这个算法的时间效率更高, 但是由于使用 best-fit 方法的缘故, 会产生内存浪费.


## 2.1 早期内存分配器
-------

[A quick history of early-boot memory allocators](https://lwn.net/Articles/761215)


### 2.1.1 bootmem
-------

2.3.23pre3 版本的引入入了第一个版本的 bootmem 分配器. 使用一个位图来表示每个物理内存页的使用状态. 清零标志位表示该物理页可用, 而设置该位则意味着相应的物理页已被占用或者不存在. 所有原先使用 memory_start 的通用部分代码, 以及 i386 架构下的初始化代码在该版本中都转换为使用 bootmem. 其他架构暂时没有跟上, 直到版本 2.3.48 时才全部完成了转换. 与此同时, Linux 被移植到 Itanium(ia64)上, 这是第一个从一开始就使用 bootmem 的架构.

bootmem 的主要缺点在于如何初始化位图. 要创建此位图, 必须基于实际的物理内存配置. 位图的大小取决于实际物理内存的大小. 并且内核需要确保能够找到一个合适的内存 bank, 其必须要拥有足够大的、连续的物理内存来存储该位图. 系统的内存越多, bootmem 所使用的位图所占用的内存也越大. 对于一个具有 32 GB 内存的系统, 位图需要占用 1 MB 的内存.

### 2.1.2 memblock
-------

[1] - https://www.kernel.org/doc/html/latest/core-api/boot-time-mm.html
[2] - https://insecuremode.com/post/2021/12/14/getting-to-know-memblock.html

随着时间的推移, 对内存的检测已经从简单地询问 BIOS 有关扩展内存块的大小发展为处理更复杂的拓扑关系, 譬如 tables, pieces , banks 和 clusters 等. 特别地, 内核对 Power64 架构的支持也已经准备就绪, 同时还引入了逻辑内存块分配器(Logical Memory Block allocator, 下文简称 LMB)的概念. 对于 LMB, 其管理的内存区域通过两个数组来标识, 第一个数组描述系统中可用的连续的物理存储区域, 而第二个数组用于跟踪这些区域的分配情况. 在内核整合 PowerPC 的 32 位和 64 位代码过程中, LMB 分配器被 32 位 的 PowerPC 架构所采纳. 后来它又被 SPARC 架构使用. 最终, 所有的体系架构都开始使用 LMB, 现在它被叫做 memblock.


memblock 分配器提供了两个最基本原语, 其他更复杂的 API 内部都会调用它们:  一个是 memblock_add(), 用于发现并注册一个可用的物理内存范围, 还有一个是 memblock_reserve() 用于申请某段内存范围并将其标记为已使用. 这两个 API 内部都会调用同一个函数 memblock_add_range(), 由该函数将相关内存区域分别添加到前面介绍的两个数组中的一个, 从而参与管理.

memblock 的内存占用是非常小的, 它采用静态数组的方式, 数组的大小确保至少可以支持系统最开始运行时的内存需要(包括执行基本的对内存区域的注册和分配动作). 如果运行过程中出现数组大小不够用的情况, 则 memblock 处理的方法很简单, 就是直接将数组的大小增加一倍. 当然这么做有一个潜在的假设, 就是, 在扩大数组时, 总有足够的内存存在.


为了减轻从 bootmem 迁移到 memblock 的痛苦, 内核引入了一个名为 nobootmem 的适配层. nobootmem 提供了大部分与 bootmem 相同的接口, 但在接口的内部没有使用位图, 而是封装了 memblock 的调用接口. 截至 v4.17, 内核所支持的 24 个架构中只有 5 个仍然在使用 bootmem 作为唯一的内核初始化早期内存分配器; 14 个使用 memblock(以 nobootmem 封装的方式); 其余五个同时支持 memblock 和 bootmem.

从 4.20 开始, 随着 bootmem 被完全清除, nobootmem 这个适配层也不再存在.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/01/27 | Karolina Drobnik <karolinadrobnik@gmail.com> | [Introduce memblock simulator](https://patchwork.kernel.org/project/linux-mm/cover/cover.1643206612.git.karolinadrobnik@gmail.com/) | Memblock 是一个启动时内存分配器, 允许在实际内存管理初始化之前管理内存区域. 因为它在引导过程中使用得太早, 所以测试和调试非常困难. 由于 memblock 没有多少内核依赖项, 因此在删除几个结构和函数后, 可以在用户空间中模拟其运行时行为. 这一系列补丁为 memblock 添加了测试套件的初始版本, 它是 tools/testing 的一部分, 包含了检查测试 memblock 的基本功能, 即内存区域管理添加/删除可用区域, 将其标记为保留或释放. | v1 ☐ | [PatchWork v1,0/16](https://lore.kernel.org/r/cover.1643206612.git.karolinadrobnik@gmail.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/16](https://lore.kernel.org/r/cover.1643796665.git.karolinadrobnik@gmail.com) |
| 2022/02/28 | Karolina Drobnik <karolinadrobnik@gmail.com> | [Add tests for memblock allocation functions](https://patchwork.kernel.org/project/linux-mm/cover/cover.1646055639.git.karolinadrobnik@gmail.com/) | 618775 | v1 ☐☑ | [LORE v1,0/9](https://lore.kernel.org/r/cover.1646055639.git.karolinadrobnik@gmail.com) |


## 2.2 页分配器: 伙伴分配器
-------

[Document: Chapter 6  Physical Page Allocation](https://www.kernel.org/doc/gorman/html/understand/understand009.html)

### 2.2.1 BUDDY 伙伴系统
-------

古老, 具体时间难考 , 应该是生而有之. orz...

内存页分配器, 是 MM 的一个重大任务, 将内存页分配给内核或用户使用. 内核把内存页分配粒度定为 11 个层次, 叫做阶(order). 第0阶就是 2^0 个(即1个)连续物理页面, 第 1 阶就是 2^1 个(即2个)连续物理页面, ..., 以此类推, 所以最大是一次可以分配 2^10(= 1024) 个连续物理页面.



所以, MM 将所有空闲的物理页面以下列链表数组组织进来:

![](https://pic4.zhimg.com/50/1eddf633b4ec562e7bfc4b22fa5375e7_hd.jpg)


(图片来自[Physical Page Allocation](https://link.zhihu.com/?target=https%3A//www.kernel.org/doc/gorman/html/understand/understand009.html))


那伙伴(buddy)的概念从何体现?

体现在释放的时候, 当释放某个页面(组)时, MM 如果发现同一个阶中如果有某个页面(组) 跟这个要释放的页面(组) 是物理连续的, 那就把它们合并, 并升入下一阶(如: 两个 0 阶的页面, 合并后, 变为连续的2页面(组), 即一个1阶页面). 两个页面(组) 手拉手升阶, 所以叫伙伴.

关于 伙伴系统, 想了解的朋友可以参见我之前的博客.

|   日期   |   博文  |   链接   |
| ------- |:-------:|:-------:|
| 2016-06-14 | 伙伴系统之伙伴系统概述--Linux内存管理(十五) | [CSDN](https://kernel.blog.csdn.net/article/details/52420444), [GitHub](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/kernel/02-memory/04-buddy/01-buddy_system) |
| 2016-09-28 | 伙伴系统之避免碎片--Linux内存管理(十六) | [CSDN](https://blog.csdn.net/gatieme/article/details/52694362), [GitHub](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/kernel/02-memory/04-buddy/03-fragmentation) |


内存分配可以分成快速 fast path(`get_page_from_freelist()`) 和 慢速 slow path(`__alloc_pages_slowpath()`), fast path 失败会走 slow path. 慢速路径中则倾向于先通过一些内存回收和规整等方式回收一些内存出来, 然后再尝试进行分配.

一般来说 slow path 的基本操作如下(顺序分先后, 但是表中顺序不代表实际顺序, 不同版本中各操作顺序等可能存在差异):

| 操作 | 条件 | 描述 |
|:---:|:----:|:----:|
| `wake_all_kswapds()` | alloc_flags & ALLOC_KSWAPD | 唤醒 kswapd 进行异步回收. |
| `get_page_from_freelist()` | checking watermark |                 | get_page_from_freelist | ALLOC_NO_WATERMARKS | 如果继续分配失败, 则通过 `__gfp_pfmemalloc_flags()` 重设 alloc_flags. 尝试不检查水线等条件, 强制无条件99进行内存分配. |
| `__alloc_pages_direct_compact()` | NA | 尝试通过内存规整, 获得足够的空闲页面. |
| `__alloc_pages_direct_reclaim()` || NA | 尝试通过内回收, 获得足够的空闲页面. |
| `__alloc_pages_may_oom ()`| NA |


**关于 NUMA 支持:** Linux 内核中, 每个 zone 都有上述的链表数组, 从而提供精确到某个 node 的某个 zone 的伙伴分配需求.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/04/15 | Mel Gorman | [Remove zonelist cache and high-order watermark checking v4](https://lore.kernel.org/patchwork/patch/599755) | 优化调度器的路径, 减少对 rq->lock 的争抢, 实现 lockless. | v4 ☑ 4.4-rc1 | [PatchWork v6](https://lore.kernel.org/patchwork/patch/599755) |
| 2016/04/15 | Mel Gorman | [Optimise page alloc/free fast paths v3](https://lore.kernel.org/patchwork/patch/668967) | 优化调度器的路径, 减少对 rq->lock 的争抢, 实现 lockless. | v3 ☑ 4.7-rc1 | [PatchWork v6](https://lore.kernel.org/patchwork/patch/668967) |
| 2016/07/08 | Mel Gorman | [Move LRU page reclaim from zones to nodes v9](https://lore.kernel.org/patchwork/patch/696408) | 将 LRU 页面的回收从 ZONE 切换到 NODE. | v3 ☑ 4.7-rc1 | [PatchWork v6](https://lore.kernel.org/patchwork/patch/696408) |
| 2016/07/15 | Mel Gorman | [Follow-up fixes to node-lru series v2](https://lore.kernel.org/patchwork/patch/698606) | node-lru 系列补丁的另一轮修复补丁, 防止 memcg 中警告被触发. | v3 ☑ 4.8-rc1 | [PatchWork v6](https://lore.kernel.org/patchwork/patch/698606) |
| 2019/11/21 | Pengfei Li <fly@kernel.page> | [Modify zonelist to nodelist v1](https://lore.kernel.org/patchwork/patch/1157012) | 目前, 如果我们想遍历所有节点, 我们必须遍历分区列表中的所有区域. 所以为了减少遍历节点所需的循环次数, 这一系列补丁将zonelist修改为nodelist. 引入了两个新的宏: <br>1) for_each_node_nlist<br>2) for_each_node_nlist_nodemask<br>这样带来的好处是:<br>1. 对于有N个节点的NUMA系统, 每个节点有M个区域, 在遍历节点时, 循环次数从N*M次减少到N次.<br>2. pg_data_t的大小减少了很多. | RFC,v1 ☐ | [PatchWork RFC,v1,00/19] Modify zonelist to nodelist v1](https://lore.kernel.org/patchwork/patch/1157012) |


一些核心的重构和修正


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/03/01 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Rationalise `__alloc_pages` wrappers](https://patchwork.kernel.org/project/linux-mm/cover/20210225150642.2582252-1-willy@infradead.org) | NA | v3 ☑ 5.13-rc1 | [PatchWork v6](https://patchwork.kernel.org/project/linux-mm/cover/20210225150642.2582252-1-willy@infradead.org) |


### 2.2.2 ZONE 分区管理
-------

[Linux 内存描述之内存区域 zone--Linux 内存管理 (三)](https://blog.csdn.net/gatieme/article/details/52384529)

在 NUMA 架构中存在多个内存节点, 每个内存节点在 Linux 内核用 pg_data_t(实际是 struct pglist_data) 来表示表示. 各个内存节点又被划分为多个内存区(即 struct zone), 这些内存区就记录在 pg_data_t->struct zone node_zones[MAX_NR_ZONES]

而在分配内存时不仅可以从本地的内存节点分配内存, 还可从其他内存节点分配内存, 这样分配时如果本地 ZONE 中内存不足时, 选择从其他 ZONE(可能本地也可能远程)分配的次序就显得格外重要. 内核通过 pg_data_t 的成员 struct zonelist node_zonelists[MAX_ZONELISTS] 来约定了该内存节点分配内存时选择从本地以及远程内存分配内存的 ZONE 先后顺序.

在系统初始化阶段, Linux 内核会遍历系统中的各个内存节点, 并根据其他邻居节点到该节点 "距离" 的 "远", " 近" 关系安插邻居节点的 zonelist 中, "距离" 越近的邻居节点所属的 struct zone, 通常在数组中的位置越靠前, 同一个邻居节点中 zones 往往按照从高到低的顺序排列 (即 ZONE_HIGH、ZONE_NORMAL...ZONE_DMA). 这样在分配内存时, 优先从最近的邻居内存节点的最高 index 的内存 zone 分配内存, 本地或者距离近的内存的 ZONE 中内存不足时, 会尝试从远端节点进行分配.

NUMA 系统下, 内核对系统的内存定义了明确的层次结构, 即当前 Node 与系统中其他 Node 的 ZONE 域之前存在一种等级次序.(首先分配本地 Node 明显比远程要好, 其次同一个 Node 的各个 ZONE 也是分三六九等的), 因此内存总是试图优先分配 "廉价的" 内存. 如果失败, 则根据访问速度和容量, 逐渐尝试分配 "更昂贵的" 内存.

1.  高端内存是最廉价的, 因为内核没有任何部份依赖于从该内存域分配的内存. 如果高端内存域用尽, 对内核没有任何副作用, 这也是优先分配高端内存的原因.

2.  其次是普通内存域, 这种情况有所不同. 许多内核数据结构必须保存在该内存域, 而不能放置到高端内存域. 因此如果普通内存完全用尽, 那么内核会面临紧急情况. 所以只要高端内存域的内存没有用尽, 都不会从普通内存域分配内存.

3.  最昂贵的是 DMA 内存域 , 因为它用于外设和系统之间的数据传输. 因此从该内存域分配内存是最后一招.

比如内核想要分配高端内存, 它首先企图在当前结点的高端内存域找到一个大小适当的空闲段. 如果失败, 则查看该结点的普通内存域. 如果还失败, 则试图在该结点的 DMA 内存域执行分配. 如果在 3 个本地内存 ZONE 都无法找到空闲内存, 则查看其他 NUMA Node. 在这种情况下, 备选结点应该尽可能靠近主结点, 以最小化由于访问非本地内存引起的性能损失.


#### 2.2.2.1 NUMA zonelist
-------

[linux 那些事之 ZONE (zonelist)(2)](https://zhikunhuo.blog.csdn.net/article/details/122800050)

*   轮循(round robin) 的 NUMA 感知的页面分配(NUMA aware page allocation)

[Import 2.3.29](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/diff/mm/page_alloc.c?id=338322e6076c42a58e4cfcda8add4ef372d90274) 引入了 zonelist_t zonelists[NR_GFPINDEX], 启动过程 free_area_init() 中通过 build_zonelists() 按照既定的规则初始化了 zonelist. 然后 `__alloc_pages()` 分配内存的过程中. 这是为了实现了 NUMA aware page allocation 的准备工作.

最早 [Import 2.3.30pre7](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/diff/mm/numa.c?id=0c7791dbc6d086d121176e9b8c9a1ce7f3a25343) 实现了 NUMA 感知的页面分配(NUMA aware page allocation). 通过 CONFIG_DISCONTIGMEM 宏控制, 引入了结构体 pglist_data 管理 node 中的内存资源, 将原本全局(不支持 NUMA, 只有一个结点)的 zonelists[NR_GFPINDEX] 修改为 pglist_data->node_zonelists[NR_GFPINDEX]. 除了原本的 `__alloc_pages()`, 还添加了 alloc_pages_node() 来在制定的 NUMA node 上分配内存. 并增加了开启 CONFIG_DISCONTIGMEM 选项下的 [alloc_pages()](https://elixir.bootlin.com/linux/2.3.30/source/mm/numa.c#L75) 实现, 当前 alloc_pages() 试图以轮循(round robin)方式通过 alloc_pages_node() 依次从各个节点分配页面, 使用一个静态的 nextnid 存储下次分配的 NUMA node, 每次分配成功后 `nextnid++`, 从而保证以轮循的方式进行页面分配. 随后 [Import 2.3.48](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/diff/include/linux/mmzone.h?id=6de1208584565275939efb5376cdd80b05d1fb0d) 将各个 NUMA NODE 通过 pglist_data->node_next 链起来, 于是在 [Linux 2.4.0-test10pre3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/diff/mm/numa.c?id=52bb34eca7a1e109192fdef2d641f8c5cf043033) 使用了这种通过 pglist_data->node_next 遍历 pglist_data 的方式替代了直接用 numa nodeid++ 的方式. alloc_pages() 中使用 `static pg_data_t *next` 和 alloc_pages_pgdat() 替代了 nextnid 和 alloc_pages_node() 的方式, 但是逻辑没变, 还是轮循.

接着 v2.5.30 [show_free_areas() cleanup](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/diff/mm/numa.c?id=c1ab3459d0ce0820b4bcddfa55fde62bb88d13c1) 移除了 node_lock. [for_each_pgdat macro](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/diff/mm/numa.c?id=f183c478d32d866b7018a9980eb4b61975b8f1fb) 将 pglist_data->node_next 重命名为 pgdat_next.

之前的 zonelist, 都是包含了本 NODE 的内存 zone 信息, 因此 `_alloc_pages()` 分配内存的时候, 只会从本 NUMA node 的内存中分配. 而通过 `alloc_pages() -=> _alloc_pages()` 轮询所有 NUMA node 的方式, NUMA 感知的页面分配(NUMA aware page allocation). 这是非常粗糙的算法. 因为页面分配的时候, 完全不考虑 NUMA node 页面的亲和性和距离问题, 分配时可能分配到距离很远的 NUMA node 的页面, 只是考虑公平切均匀的是系统中各个 NUMA node 的内存.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/09/19 | Andrew Morton <akpm@digeo.com> | [`_alloc_pages cleanup`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ccc98a67de98c912840e0a35a24115ad64ae426d) | 重新设计了 NUMA aware page allocation 的逻辑. 移除了 `_alloc_pages()` 以及轮循(round robin)的 NUMA 内存分配器, 实现了 NUMA zonelist. | v1 ☑✓ 2.5.37 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ccc98a67de98c912840e0a35a24115ad64ae426d) |

*   NUMA-aware zonelist(Walk all node and zones in the zonelist)

v2.5.37 [commit ccc98a67de98 ("`_alloc_pages cleanup`")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ccc98a67de98c912840e0a35a24115ad64ae426d) 重新设计了 NUMA aware page allocation 的逻辑. 移除了 `_alloc_pages()` 以及轮循(round robin)的 NUMA 内存分配器, 实现了 NUMA zonelist, zonelist->zones[MAX_NUMNODES * MAX_NR_ZONES + 1] 中系统中所有的 NUMA node 的 zone [按照 [local_node, numnodes - 1, 0, local_node - 1] 的顺序](https://elixir.bootlin.com/linux/v2.5.37/source/mm/page_alloc.c#L702) 排列. 这样分配时会优先从本地 ZONE 进行分配, 本地 ZONE 不足时, 会在本地其他 ZONE 分配, 当整个本地都没有内存时, 则继续从其他 NUMA node 中进行分配. 已经有了优先本地, 其次其他 NUMA node 的思想, 但是远程的 NUMA node 依旧是[按照 numanodeid 顺序](https://elixir.bootlin.com/linux/v2.5.37/source/mm/page_alloc.c#L702)来的, 没有考虑各个 NUMA node 之间的距离. 添加了 [build_zonelists_node()](https://elixir.bootlin.com/linux/v2.5.37/source/mm/page_alloc.c#L675) 对某个 NUMA node 的 zonelist 进行构建, 然后 build_zonelists() 通过 build_zonelists_node() 完成所有节点 zonelist 的构建. 随后 v2.6.5-rc1, [commit 0eaf393b6b6e ("NUMA-aware zonelist builder")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0eaf393b6b6ec017f48aa02ba90b1dd53ad16e65) 实现了 zonelist NUMA-aware ordering of nodes, 按照近节点优先、远节点最后的顺序对区域列表 zonelists 进行排序. build_zonelists() 通过调用 [find_next_best_node()](https://elixir.bootlin.com/linux/v2.6.5/source/mm/page_alloc.c#L1140) 来选择下一个最近的节点添加到 zonelist.

上面的补丁只有 `alloc_pages()` 感知了 NUMA-aware zonelist 的变化, 其他路径还没有了解到随后对内存的各个 2.5.40 进一步对 KSWAPD 也做了处理. 参见 [commit1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=db7b0c9fcd4e93c07f16a0bfd449b49210fa523d), [commit2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=36fb7f8459cc42eca202f0ad7b2d051359406d57). 实现 per-node kswapd 的时候, `_alloc_pages()` 中相应的 wakeup_kswapd() 会遍历区域列表 zonelist 中的所有区域 zone.

此时 try_to_free_pages() 仅释放分区列表中第一个分区所属的 pgdat 中的页面, 这明显与其他路径显得格格不入. 因此在 v2.6.2 页面释放路径 try_to_free_pages() 也开始遍历 zonelist 来进行释放, 参见 [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d5d4042dd2d0352990d7ff54ea422950eba62969).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2004/03/11 | Andrew Morton <akpm@osdl.org> | [NUMA-aware zonelist builder](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0eaf393b6b6ec017f48aa02ba90b1dd53ad16e65) | TODO | v1 ☐☑✓ 2.6.5-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0eaf393b6b6ec017f48aa02ba90b1dd53ad16e65) |
| 2002/09/29 | Andrew Morton <akpm@digeo.com> | [per-node kswapd instances](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=db7b0c9fcd4e93c07f16a0bfd449b49210fa523d) | TODO | v1 ☑✓ 2.5.40 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=db7b0c9fcd4e93c07f16a0bfd449b49210fa523d) |
| 2002/11/21 | Andrew Morton <akpm@digeo.com> | [handle zones which are full of unreclaimable pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=36fb7f8459cc42eca202f0ad7b2d051359406d57) | TODO | v1 ☑✓ 2.5.49 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=36fb7f8459cc42eca202f0ad7b2d051359406d57) |
| 2004/01/18 | Andrew Morton <akpm@osdl.org> | [make try_to_free_pages walk zonelist](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d5d4042dd2d0352990d7ff54ea422950eba62969) | 如果在 /proc 中设置了 lower_zone_protection, 那么 kswapd 可能永远不会清除区域列表下方区域中的数据, 而 try_to_free_pages() 需要这样做. 但是, 在 2.6.0 中, try_to_free_pages() 只释放区域列表中第一个区域所属的 pgdat 中的页面. 这可能是错误的行为, 因为页面分配器和 kswapd 都从区域列表上的所有区域中唤醒. 下面的补丁通过将区域列表作为参数传递并从列表中的所有区域释放页面, 使 try_to_free_pages() 与分配器保持一致. | v1 ☑✓ 2.6.2-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d5d4042dd2d0352990d7ff54ea422950eba62969) |

v2.6.19 之前 GFP 到 ZONE 的转换是非常奇怪的, 内核定义了 GFP_ZONEMASK 和 GFP_ZONETYPES 来完成 GFP 到 ZONE 的映射关系, 这其实完全没必要, [commit 19655d348700 ("linearly index zone->node_zonelists")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=19655d3487001d7df0e10e9cbfc27c758b77c2b5) 实现了对 zone->node_zonelists[MAX_NR_ZONES] 直接进行线性访问的方式. zonelist 数组的定义从 node_zonelists[GFP_ZONETYPES] 变成了 node_zonelists[MAX_NR_ZONES]. 直接通过 gfp_zone(gfp)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2006/09/25 | Christoph Lameter <clameter@sgi.com> | [linearly index zone->node_zonelists](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=19655d3487001d7df0e10e9cbfc27c758b77c2b5) | 当前需要这个位掩码 MASK 索引到 pglist_data->node_zonelists[GFP_ZONETYPES]. 通常我们总是从最高的区域开始, 然后包括所有较低的区域来建立区域列表. 实现用索引对 pglist_data->node_zonelists[MAX_NR_ZONES] 进行线性访问, 这样 highest_zone() 也不用需要了. | v1 ☑✓ 2.6.19-rc1 | [LORE 00/13](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=19655d3487001d7df0e10e9cbfc27c758b77c2b5) |

*   build zonelist

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2006/01/06 | Christoph Lameter <clameter@engr.sgi.com> | [simplify build_zonelists](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=070f80326a215d8e6c4fd6f175e28eb446c492bc) | 重构了 build_zonelists 的逻辑. | v1 ☑✓ 2.6.16-rc1 | [LORE 0/4](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=070f80326a215d8e6c4fd6f175e28eb446c492bc) |
| 2017/07/14 | Michal Hocko <mhocko@kernel.org> | [cleanup zonelists initialization](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=b95046b0472f7a805fa28fbcfc7205a76ff7a7d0) | NA | v1 ☑✓ 4.14-rc1 | [LORE v1,0/9](https://lore.kernel.org/all/20170714080006.7250-1-mhocko@kernel.org)|
| 2006/08/21 | Mel Gorman <mel@csn.ul.ie> | [Sizing zones and holes in an architecture independent manner V9](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0e0b864e069c52a7b3e4a7da56e29b03a012fd75) | NA | v1 ☑ v2.6.19-rc1 | [PatchWork 0/2](https://lore.kernel.org/patchwork/patch/63170) |
| 2021/08/10 | Baoquan He <bhe@redhat.com> | [Avoid requesting page from DMA zone when no managed pages](https://lore.kernel.org/patchwork/patch/1474378) | 在当前内核的某些地方, 它假定 DMA 区域必须具有托管页面, 并在启用 CONFIG_ZONE_DMA 时尝试请求页面. 但这并不总是正确的. 例如, 在 x86_64 的 kdump 内核中, 在启动的早期阶段, 只显示并锁定低 1M, 因此在 DMA 区域中根本没有托管页面. 如果从 DMA 区域请求页面, 此异常将始终导致页面分配失败. 这造成在 x86_64 的 kdump 内核中,  使用 GFP_DMA 创建的 atomic_pool_dma 会导致页面分配失败. dma-kmalloc 初始化也会导致页面分配失败. | v2 ☐ | [PatchWork RFC,v2,0/5](https://patchwork.kernel.org/project/linux-mm/cover/20210810094835.13402-1-bhe@redhat.com) |
| 2021/08/30 | Bharata B Rao <bharata@amd.com> | [Fix NUMA nodes fallback list ordering](https://lore.kernel.org/all/20210830121603.1081-1-bharata@amd.com) | 20210830121603.1081-1-bharata@amd.com | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20210830121603.1081-1-bharata@amd.com) |


*   GFP_THISNODE 与 Two Zonelists

v2.6.19 [commit 9b819d20 ("Add `__GFP_THISNODE` to avoid fallback to other nodes and ignore cpuset/memory policy restrictions")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9b819d204cf602eab1a53a9ec4b8d2ca51e02a1d) 添加一个新的 gfp 标志 `GFP_THISNODE` 标记, 以避免内存分配时通过 zonelists FallBack 到其他 NUMA 节点. 具体实现方式是, 如果设置了 GFP_THISNODE 则, get_page_from_freelist() 被限制只能从 `zonelist->zones[0]->zone_pgdat` 的节点上去分配内存.

v2.6.24 [commit 523b945855a1 ("Memoryless nodes: Fix GFP_THISNODE behavior")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=523b945855a1427000ffc707c610abe5947ae607) 修改了 `GFP_THISNODE` 的实现方式. 为了将 GFP_THISNODE 的分配限制为单个节点的区域列表, 将 NUMA zonelists 数组大小直接加倍(double). 引入 MAX_ZONELISTS, NUMA 情况下, 被设置为 (2 * MAX_NR_ZONES), 其他情况下依旧是 MAX_NR_ZONES. NUMA node 的 zonelists 定义从 pglist_data->node_zonelists[MAX_NR_ZONES] 更改为 pglist_data->node_zonelists[MAX_ZONELISTS], 数组的前一半 zonelist 区域 [0 ... MAX_NR_ZONES - 1] 支持 Fallback 到其他 NUMA node, 后一半 zonelist 区域  [MAZ_NR_ZONES ... MAZ_ZONELISTS - 1] 用于 GFP_THISNODE, 不提供 Fallback 功能.

之前的设计中, 每个 NUMA 节点的 zonelists pglist_data->node_zonelists[MAX_NR_ZONES] 中包含了 MAX_ZONELISTS(2 * MAX_NR_ZONE) 个 zonelist. 这些 zonelist 被分成了两组, 一组用于每种分区类型, 另一组用于 GFP_THISNODE 分配. 然后根据 gfp 来确定允许的 ZONE, 在选择其中一个分区列表 zonelist. 所有这些分区列表都会消耗内存. 因此 v2.6.26 [commit 19770b32609b ("Use two zonelists per node instead of multiple zonelists v11r2")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=19770b32609b6bf97a3dece2529089494cbfc549) 进一步修正了 NUMA zonelists 的实现.

首先 [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=54a6eb5c4765aa573a030ceeba2c14e3d2ea5706) 将每个节点的多个 zonelist 替换为两个 zonelist(直接定义 MAX_ZONELISTS 为 2). 第一个包含系统中按 NUMA 距离排序的所有填充区域, 用于在目标/首选节点没有空闲页面时进行 Fallback 分配. 第二个包含节点中适合 GFP_THISNODE 分配的所有填充区域. 之前的每个 ZONE 都有一个 zonelist, 因此遍历 zonelist 的时候, 直接从前往后直到遇到 NULL 节点为止. 但是归一为 Two zonelist 后, 原本的遍历方式无法进行, 因此引入了一个[迭代器宏 for_each_zone_zonelist()](https://elixir.bootlin.com/linux/v2.6.26/source/include/linux/mmzone.h#L809), 该宏通过所选 zonelist 中 GFP 标志允许的每个区域进行交互. 通过 [first_zones_zonelist()](https://elixir.bootlin.com/linux/v2.6.26/source/include/linux/mmzone.h#L763) 和 [next_zones_zonelist()](https://elixir.bootlin.com/linux/v2.6.26/source/include/linux/mmzone.h#L746) 来辅助遍历的工作.

其次, 筛选 zonelists 需要非常频繁地使用 zone_idx(), 这非常耗时, 因为它涉及另一个结构的查找和减法操作. 为了快速访问, 如果发现访问 zone->node 很重要, 那么节点 idx 也可以保存下来, 这可能是在大量使用 nodemask 的工作负载上的情况. [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=dd1a239f6f2d4d3eedd318583ec319aa145b324c) 引入了一个 struct zoneref 来存储区域指针和区域索引. 然后, zonelist 由这些 zoneref 的数组组成, 为同时访问区域索引和节点索引提供了帮助.

v4.5 将 Two Zonelists 的 index 定义为 enum 变量. 参见 [commit c00eb15a8914 ("mm/zonelist: enumerate zonelists array index")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c00eb15a8914b8ba84032a36044a5aaf7f71709d).

| Two Zonelists | 描述 |
|:-------------:|:---:|
| ZONELIST_FALLBACK | 不管是 NUMA 或 SMP 系统中都存在, 当内存失败时从该 FALLBACK 中管理的 zone 顺序取出下一个 zone 中申请内存. SMP 系统中只有一个节点其顺序依次从 ZONE_HIGHMEM、ZONE_NORMAL、ZONE_DMA 中从高到低排序. NUMA 系统中, 首先安排本节点之后, 再根据 NUMA 系统由近到远安排其他远端节点内存, 当本节点没有内存时, 就有依次按照最近原则尽量从最近远端节点中获取内存, 决定内存分配策略. |
| ZONELIST_NOFALLBACK | NUMA 系统有时候根据需要只想从本节点中获取内存, 不想总远端中获取内存. 因此 NOFALLBACK 对应的 zonelist 只有本节点信息, zone 排序依次从高到低. ZONELIST_NOFALLBACK 主要是针对在有些特定的内存申请场景, 只希望从指定节点或者本地节点中申请内存, 当本地节点或者指定内存节点内存不足时, 能够立即知道内存不足, 而不是希望通过 FALLBACK 机制从其他节点中继续申请内存. |


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2006/09/25 | Christoph Lameter <clameter@sgi.com> | [mm: handle GFP_THISNODE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=980128f223fa3c75e3ebdde650c9f1bcabd4c0a2) | 添加一个新的 gfp 标志 `GFP_THISNODE` 标记, 以避免 FallBack 到其他 NUMA 节点来分配内存. 如果分配内存时要求内存位于某个节点上, 则可以使用此标记. alloc_pages_node() 使用此标记则说明需要在指定的节点上强制分配, alloc_pages() 使用此标记表明在当前节点上强制分配.<br>具体实现方式是, get_page_from_freelist() 中不再从其他非 `zonelist->zones[0]->zone_pgdat` 的节点上去分配内存. | v1 ☑✓ 2.6.19-rc1 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=980128f223fa3c75e3ebdde650c9f1bcabd4c0a2) |
| 2007/10/16 | Yasunori Goto <y-goto@jp.fujitsu.com> | [Memoryless nodes: Fix GFP_THISNODE behavior](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=523b945855a1427000ffc707c610abe5947ae607) | [Memoryless nodes support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=58c0a4a7864b2dad6da4090813322fcd29a11c92) 的其中一个补丁. 修改了 `GFP_THISNODE` 的实现方式. 之前是通过 get_page_from_freelist() 中不再从其他非 `zonelist->zones[0]->zone_pgdat` 的节点上去分配内存来保证的. 这个补丁为了将 GFP_THISNODE 的分配限制为单个节点的区域列表, 将 NUMA zonelists 加倍(double).  pglist_data->node_zonelists[MAX_NR_ZONES] 修改为 pglist_data->node_zonelists[MAX_ZONELISTS]. MAX_ZONELISTS 被设置为 2 * MAX_NR_ZONE. 前一半支持 Fallback 到其他 NUMA node, 后一半用于 GFP_THISNODE, 不提供 Fallback 功能, 提供了 build_thisnode_zonelists() 来构建 GFP_THISNODE 的 zonelist. 然后分配时, gfp_zone() 直接返回 zonelist 上后半数组对应 zone 的索引, 这样保证只能从本地 node 上分配. | v1 ☑✓ 2.6.24-rc1 | [CGIT 00/16](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=58c0a4a7864b2dad6da4090813322fcd29a11c92) |
| 2007/12/11 | Mel Gorman <mel@csn.ul.ie> | [Use two zonelists per node instead of multiple zonelists v11r2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=19770b32609b6bf97a3dece2529089494cbfc549) | 优化分配器处理区域列表, 区域列表指示分配目标区域的顺序. 类似地, 页面的直接回收会在区域数组上迭代. 为了保持一致性, 这组补丁将直接回收转换为使用分区列表, 并简化 zonelist 迭代器.<br>将每个节点的多个(两组)分区列表替换为两个分区列表, 一组用于系统中的每个分区类型, 另一组用于 GFP_THISNODE 分配. 根据 gfp 掩码允许的分区, 选择其中一个分区列表. 所有这些分区列表都会消耗内存并占用缓存线. | v1 ☑ v2.6.26-rc1 | [LORE 0/6](https://lore.kernel.org/lkml/20070928142326.16783.98817.sendpatchset@skynet.skynet.ie), [LORE RFC,v5,0.6](https://lore.kernel.org/lkml/20070911151939.11117.30384.sendpatchset@skynet.skynet.ie), [PatchWork v11r2,0/6](https://lore.kernel.org/all/20071211202157.1961.27940.sendpatchset@skynet.skynet.ie) |
| 2016/01/14 | Yaowei Bai <baiyaowei@cmss.chinamobile.com> | [mm/zonelist: enumerate zonelists array index](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c00eb15a8914b8ba84032a36044a5aaf7f71709d) | 使用 enum 定义了 zonelist double 的索引, ZONELIST_FALLBACK 和 ZONELIST_NOFALLBACK. | v1 ☑✓ 4.5-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c00eb15a8914b8ba84032a36044a5aaf7f71709d) |


*   Zonelist Caching

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2006/12/06 | Paul Jackson <pj@sgi.com> | [memory page_alloc zonelist caching reorder structure](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=7253f4ef04b1cd138baf2b29a95473743ac0a307) | TODO | v1 ☑✓ 2.6.20-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7253f4ef04b1cd138baf2b29a95473743ac0a307) |
| 2015/09/21 | Mel Gorman <mgorman@techsingularity.net> | [Remove zonelist cache and high-order watermark checking v4](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=dd56b046426760aa0c852ad6e4b6b07891222d65) | 引入区域列表缓存(ZLC)是为了跳过最近已知已满的区域. 这避免了一些耗时昂贵的操作, 如 cpuset 检查、水印计算和 zone_reclaim. 但是到此时的版本, 情况已经大不相同, ZLC 的复杂性更难证明. 因此将其移除. | v4 ☑✓ 4.4-rc1 | [LORE v4,0/10](https://lore.kernel.org/all/1442832762-7247-1-git-send-email-mgorman@techsingularity.net), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f77cf4e4cc9d40310a7224a1a67c733aeec78836) |

*   Numa Zonelist Order

NUMA 系统中存在多个节点, 每个节点对应一个 struct pglist_data 结构, 每个结点中可以包含多个 zone, 如: ZONE_DMA, ZONE_NORMAL, 这样就会产生几种排列顺序.

v2.6.32 [commit f0c0b2b808f2 ("change zonelist order: zonelist order selection logic")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f0c0b2b808f232741eadac272bd4bc51f18df0f4) 引入了启动参数 "numa_zonelist_order"(`/proc/sys/vm/numa_zonelist_order`) 来配置 NUMA zonelist 的次序. 当前实现了 3 种模式: Legacy 方式, Node 方式和 Zone 方式, 通过 [current_zonelist_order](https://elixir.bootlin.com/linux/v2.6.32/source/mm/page_alloc.c#L2345) 全局的 current_zonelist_order 变量标识了系统中的当前使用的内存域排列方式, 默认配置为 ZONELIST_ORDER_DEFAULT. [zonelist_order_name[current_zonelist_order]](https://elixir.bootlin.com/linux/v2.6.32/source/mm/page_alloc.c#L2346) 就标识了当前系统中所使用的 zonelist 次序的名称 "Default", "Node", "Zone", 可用于调试和输出.

build_zonelists() 的过程中, 如果[指定了 ZONELIST_ORDER_NODE](https://elixir.bootlin.com/linux/v2.6.32/source/mm/page_alloc.c#L2657), 则通过 build_zonelists_in_node_order() 来构建, 如果[是 ZONELIST_ORDER_ZONE 次序](https://elixir.bootlin.com/linux/v2.6.32/source/mm/page_alloc.c#L2663), 则通过 build_zonelists_in_zone_order() 来构建.

| Zonelist Order | 排列方式 | 描述 |
|:--:|:--------------------:|:------:|:----:|
| ZONELIST_ORDER_DEFAULT | Default | 由系统智能选择 Node 或 Zone 方式 |
| ZONELIST_ORDER_NODE    | Node 模式(Node-based order) |  按节点顺序依次排列, 先排列本地节点的所有 zone, 再排列其它节点的所有 zone |
| ZONELIST_ORDER_ZONE    | Zone 模式(Zone-based order) | 按 zone 类型从高到低依次排列各节点的同相类型 zone |

内核提供了不同的 zonlists order 模式的支持, 但测试结果发现它们没有什么效果, 真正使用的人也很少. 因此 v4.14 [commit c9bff3eebc09 ("mm, page_alloc: rip out ZONELIST_ORDER_ZONE")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c9bff3eebc09be23fbc868f5e6731666d23cbea3) 完全删除了 ZONELIST_ORDER_ZONE 模式(Zone-based order), 但是依旧保留了 numa_zonelist_order, 用于提醒用户已经不至此 ZONELIST_ORDER_ZONE 模式. 至此 build_zonelists() 只会通过 build_zonelists_in_node_order() 构建 Node-based order 的 zonelist.

内存回收的很多关键路径(比如页面回收等)都已经从等都已经从 Zone-Base 切到了 NODE-base 的方式. 而 移除了 ZONELIST_ORDER_ZONE 模式后, 内核现在只使用 ZONELIST_ORDER_NODE 模式, ZONELIST 始终处于 "Node" 由近及远的 order, 然后一个 Node 内部 zonelist order 的顺序也是固定的, 因此构建 NodeList 是可以的. 参见 [LORE 讨论](https://lore.kernel.org/all/20191123013613.566bb40a.fly@kernel.page).

另外一方面, 内存中太多的路径存在由近及远遍历所有 NUMA 节点的诉求, 而当前采用的是曲线救国策略, 使用 for_each_zone_zonelist() 和 for_each_zone_zonelist_nodemask() 遍历所有 NUMA zonelist, 由于这两个宏本质是按照 NUMA 距离由近及远的顺序遍历各个 NUMA 的各个 ZONE, 因此如果单纯只想要遍历 NUMA node, 需要[不断保存和跳过对同一个 NUMA 的遍历](https://elixir.bootlin.com/linux/v4.14/source/mm/vmscan.c#L2879). 参见 [LORE 讨论](https://lore.kernel.org/all/20191122232847.3ad94414.fly@kernel.page).

因此, 为了减少遍历节点所需的循环数, 补丁集 [Modify zonelist to nodelist v1](https://lore.kernel.org/all/20191121151811.49742-1-fly@kernel.page) 将 ZonelList 修改为 NodeList. 并引入了 for_each_node_nlist() 和 for_each_node_nlist_nodemask() 替代之前的两个宏用来辅助 NUMA node 的遍历. 而如果依旧想遍历 NUMA zonelist, 则可以通过 for_each_zone_nlist() 和 for_each_zone_nlist_nodemask() 来完成.

不过由于没有有效地测试数据证明, 以及[对 ZONE_DMA 等分配请求](https://lore.kernel.org/all/alpine.DEB.2.21.1911221551570.10063@www.lameter.com)处理地不确定性, NodeList 的方案只进行了简单的讨论后再没有下文. 但是前面提到, 当前内存管理路径下很多代码当前使用了 for_each_zone_zonelist(), 即使只需要迭代每个 NUMA Node. 因此 2022 年,  [mm/mmzone: Introduce a new macro for_each_node_zonelist()](https://patchwork.kernel.org/project/linux-mm/patch/20220416123930.5956-1-dthex5d@gmail.com/) 只提供了一个宏 for_each_node_zonelist(), 它只遍历 zonelist 中的有效 NUMA Node. 使用这个新宏, 这可以极大地简化代码, 使得逻辑更清晰.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/11/21 | Andrew Morton <akpm@digeo.com> | [change zonelist order: zonelist order selection logic](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f0c0b2b808f232741eadac272bd4bc51f18df0f4) | TODO | v1 ☑✓ 2.6.23-rc1 | [LKML v6,0/3](https://lkml.org/lkml/2007/5/10/61) |
| 2017/07/14 | Michal Hocko <mhocko@kernel.org> | [mm, page_alloc: rip out ZONELIST_ORDER_ZONE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c9bff3eebc09be23fbc868f5e6731666d23cbea3) | [cleanup zonelists initialization](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=b95046b0472f7a805fa28fbcfc7205a76ff7a7d0) 系列的其中一个补丁. | v1 ☑✓ 4.14-rc1 | [LORE v1,1/9](https://lore.kernel.org/all/20170714080006.7250-1-mhocko@kernel.org) |
| 2019/11/21 | Pengfei Li <fly@kernel.page> | [Modify zonelist to nodelist v1](https://lore.kernel.org/all/20191121151811.49742-1-fly@kernel.page) | NA | v1 ☐☑✓ | [LORE v1,00/19](https://lore.kernel.org/all/20191121151811.49742-1-fly@kernel.page) |
| 2022/04/16 | dthex5d <dthex5d@gmail.com> | [mm/mmzone: Introduce a new macro for_each_node_zonelist()](https://patchwork.kernel.org/project/linux-mm/patch/20220416123930.5956-1-dthex5d@gmail.com/) | 632783 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220416123930.5956-1-dthex5d@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20220416132037.6395-1-dthex5d@gmail.com) |


*   Print Zonelist

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/04/28 | Mel Gorman <mel@csn.ul.ie> | [mm: print out the zonelists on request for manual verification](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=68ad8df42e12037c3894c9706ab428bf5cd6426b) | [Verification and debugging of memory initialisation V4](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=68ad8df42e12037c3894c9706ab428bf5cd6426b) | v1 ☑✓ 2.6.27-rc1 | [LORE RFC,0/4](https://lore.kernel.org/all/20080416135058.1346.65546.sendpatchset@skynet.skynet.ie)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/4](https://lore.kernel.org/all/20080428192839.23649.82172.sendpatchset@skynet.skynet.ie) |
| 2021/08/30 | Bharata B Rao <bharata@amd.com> | [mm/page_alloc: Print node fallback order](https://lore.kernel.org/all/20210830121603.1081-1-bharata@amd.com) | [Fix NUMA nodes fallback list ordering](https://lore.kernel.org/all/20210830121603.1081-1-bharata@amd.com) 的第一个补丁, 在 build_zonelists() 完成构建后, 添加了Fallback order for Node 的调试信息. | v1 ☐☑✓ | [LORE v1,1/2](https://lore.kernel.org/all/20210830121603.1081-2-bharata@amd.com) |


#### 2.2.2.3 fair allocation zone policy
-------

每个拥有一个工作负载的用户空间页面的区域必须以与区域大小成比例的速度老化. 否则, 单个页面在内存中停留的时间取决于它碰巧被分配的区域. 区域老化的不对称造成相当不可预测的老化行为, 并导致错误的页面被回收, 激活等.

但 v3.12 时, 开发者发现页面分配器和 kswapd 交互的方式, 将造成页面老化不平衡. 在分配新页面时, 页面分配器使用系统中所有分区的每个节点列表(按优先顺序排列). 当第一次迭代没有产生任何结果时, kswapd 将被唤醒, 分配器将重试. 由于以下方式 kswapd 回收区高水准而带可以从以上时分配低水印,分配器可能保持kswapd运行而kswapd回收确保页分配器可以防止分配中的第一个区zonelist长时间. 与此同时, 其他区域很少看到新的分配, 因此相比之下, 老化要慢得多. 其结果是, 偶尔放置在较低区域的页面在内存中占用的时间相对较多, 甚至在其对等页面长期被逐出后被提升到活动列表. 同时, 大部分 workingset 可能在首选区域上颠簸, 即使在较低区域中可能有大量可用内存.

即使是最基本的测试--反复读取比内存稍大的文件, 也会显示区域老化的破坏程度. 在这种情况下, 没有一个页面能够在内存中停留足够长的时间来被引用两次并被激活, 但是激活是非常频繁的

通过 fair zone allocator policy 来解决这个问题. 通过一个非常简单的循环分配器. 每个分区允许一批与分区大小成比例的分配, 用 NR_ALLOC_BATCH(PatchWork 早期版本用 zone->alloc_batch) 记录, 分配后将被视为已满. 当所有区域都已尝试且分配器进入慢路径并启动 kswapd 回收时, 批处理计数器将重置. 分配和回收现在公平地分布到所有可用/允许的区域.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2013/08/02 | Mel Gorman <mgorman@techsingularity.net> | [mm: improve page aging fairness between zones/nodes](https://lore.kernel.org/patchwork/patch/397316) | 引入了区域公平分配策略来修复页面分配器与 kswapd 交互的方式上造成老化不平衡<br>在回收压力下, 用户空间页面在内存中获得的时间取决于分配器从哪个区域、哪个节点获取页面框架. 这造成了如下几个问题<br>1. NUMA 系统上错误的不同步的 kswapd 唤醒, 这导致一些节点相对于系统中的其他节点落后于一个完整的回收周期.<br>2. kswapd 和页面分配的连续流无限期地将任务的首选区域保持在高水位和低水位之间(分配成功 + kswapd不会进入休眠), 完全不充分利用较低的区域, 并在首选区域上抖动. | v2 ☑ 3.12-rc1 | [PatchWork v2,0/3](https://lore.kernel.org/patchwork/patch/397316) |
| 2013/12/18 | Mel Gorman <mgorman@suse.de> | [Configurable fair allocation zone policy v4](https://lore.kernel.org/patchwork/patch/428591) | NA | v2 ☑ 3.12-rc1 | [PatchWorkRFC,0/6](https://lore.kernel.org/patchwork/patch/428591) |
| 2013/12/18 | Mel Gorman <mgorman@techsingularity.net> | [Configurable fair allocation zone policy v4](https://lore.kernel.org/patchwork/patch/397316) | NA | v4 ☐ | [PatchWork RFC v4,0/6](https://lore.kernel.org/patchwork/patch/397316) |
| 2014/03/20 | Johannes Weiner <hannes@cmpxchg.org> | [mm: page_alloc: spill to remote nodes before waking kswapd](https://lore.kernel.org/patchwork/patch/450947) | 这个补丁引入了 ALLOC_FAIR.<br>在 NUMA 系统上, 节点可能会过早的开始回收页面, 甚至交换匿名页, 而即使远程节点上仍然有空闲页时.<br>这是 81c0a2bb515f ("mm: page_alloc: fair zone allocator policy") 和 fff4068cba48 ("mm: page_alloc: revert NUMA aspect of fair allocation policy") 合入后导致的.<br>在进行这些更改之前, 分配器将首先尝试所有允许的分区, 包括远程节点上的分区, 然后再唤醒任何 kswapds.<br>但是现在, 分配器快速路径同时作为公平通道, 它只能考虑本地节点, 以防止仅基于耗尽公平批次的远程溢出. 远程节点只在缓慢路径中考虑, 且在 kswapds 被唤醒之后.<br>是如果远程节点仍然有空闲内存, 其实不应该唤醒 kswapd 来重新平衡本地节点, 否则它可能会过早地 swap.<br>通过在 zonelist 上再添加一个不公平的传递来修复此问题, 该传递允许在本地公平传递失败后, 在进入慢路径并唤醒 KSWAPD 之前考虑远程节点. | v1 ☑ 3.15-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/450947), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3a025760fc158b3726eac89ee95d7f29599e9dfa) |
| 2014/07/09 | Mel Gorman <mgorman@suse.de> | [mm: page_alloc: Reduce cost of the fair zone allocation policy](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=4ffeaf3560a52b4a69cc7909873d08c0ef5909d4) | [Reduce sequential read overhead](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=4ffeaf3560a52b4a69cc7909873d08c0ef5909d4) 系列中的一个补丁. fair zone allocation policy 按单个区域大小的比例分配页面, 以确保页面老化公平. 在回收页之前, 分配页的区域不应影响页在内存中的时间. 启用区域回收模式后, 尝试停留在快速路径中的本地区域. 如果失败, 将进入慢路径, 该路径将从本地区域开始执行另一次传递, 但最终返回到不参与此区域列表的公平循环的远程区域. | v1 ☑ 3.17-rc1 | [PatchWork 6/6](https://lore.kernel.org/all/1404893588-21371-1-git-send-email-mgorman@suse.de/) |
| 2016/04/15 | Mel Gorman <mgorman@techsingularity.net> | [mm, page_alloc: Reduce cost of fair zone allocation policy retry](https://lore.kernel.org/patchwork/patch/668985) | [Optimise page alloc/free fast paths v3](https://lore.kernel.org/patchwork/patch/668967) 系列中的一个补丁. 降低了 fair zone 分配器的开销. | v3 ☑ 4.7-rc1 | [PatchWork v6 00/28](https://lore.kernel.org/patchwork/patch/668967) |
| 2016/07/08 | Mel Gorman <mgorman@techsingularity.net> | [mm, page_alloc: remove fair zone allocation policy](https://lore.kernel.org/patchwork/patch/696437/) | [Move LRU page reclaim from zones to nodes v9](https://lore.kernel.org/patchwork/patch/696437)系列中的一个补丁. 公平区域分配策略在区域之间交叉分配请求, 以避免年龄倒置问题, 即回收新页面来平衡区域. LRU 回收现在是基于节点的, 所以这应该不再是一个问题, 公平区域分配策略本身开销也不小, 因此这个补丁移除了它. | v9 ☑ [4.8-rc1](https://kernelnewbies.org/Linux_4.8#Memory_management) | [PatchWork v21](https://lore.kernel.org/patchwork/patch/696437), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e6cbd7f2efb433d717af72aa8510a9db6f7a7e05) |

### 2.2.3 NUMA 下的内存分配 memory policy
-------

NUMA 系统中 CPU 访问不同节点的内存速度很有大的差别. 位于本地 NUMA 节点(或附近节点)上的内存比远程节点上的内存访问速度更快. 因此如果能通过策略去控制优先在哪些节点上内存分配, 能有效地提升业务的性能.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2010/05/04 | Miao Xie <miaox@cn.fujitsu.com> | [NUMA API](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e1e71f9b6c8dd34be36573346ecbbb00f34a7a0a) | NA | v1 ☑✓ 2.6.7-rc1 | [LORE v1,0/2](https://lore.kernel.org/all/4BDFFCC8.4040205@cn.fujitsu.com) |
| 2010/05/04 | Miao Xie <miaox@cn.fujitsu.com> | [mempolicy: restructure rebinding-mempolicy functions](https://lore.kernel.org/all/4BDFFCC8.4040205@cn.fujitsu.com) | 4BDFFCC8.4040205@cn.fujitsu.com | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/4BDFFCC8.4040205@cn.fujitsu.com) |
| 2017/04/11 | Vlastimil Babka <vbabka@suse.cz> | [cpuset/mempolicies related fixes and cleanups](https://lore.kernel.org/all/20170411140609.3787-1-vbabka@suse.cz) | 20170411140609.3787-1-vbabka@suse.cz | v1 ☐☑✓ | [LORE v1,0/6](https://lore.kernel.org/all/20170411140609.3787-1-vbabka@suse.cz) |
| 2021/03/17 | Feng Tang <feng.tang@intel.com> | [Introduced multi-preference mempolicy](https://lore.kernel.org/all/1615952410-36895-1-git-send-email-feng.tang@intel.com) | 1615952410-36895-1-git-send-email-feng.tang@intel.com | v4 ☐☑✓ | [LORE v4,0/13](https://lore.kernel.org/all/1615952410-36895-1-git-send-email-feng.tang@intel.com) |
| 2021/08/03 | Feng Tang <feng.tang@intel.com> | [Introduce multi-preference mempolicy](https://lore.kernel.org/patchwork/patch/1471473) | 参见 LWN 报道 [NUMA policy and memory types](https://lwn.net/Articles/862707).<br>引入 MPOL_PREFERRED_MANY 的 policy, 该 mempolicy 模式可用于 set_mempolicy 或 mbind 接口.<br>1. 与 MPOL_PREFERRED 模式一样, 它允许应用程序为满足内存分配请求的节点设置首选项. 但是与 MPOL_PREFERRED 模式不同, 它需要一组节点.<br>2. 与 MPOL_BIND 接口一样, 它在一组节点上工作, 与 MPOL_BIND 不同, 如果首选节点不可用, 它不会导致 SIGSEGV 或调用 OOM killer. | v7 ☑ 5.15-rc1 | [LORE v4,00/13](https://lore.kernel.org/lkml/1615952410-36895-1-git-send-email-feng.tang@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/5](https://patchwork.kernel.org/project/linux-mm/cover/1627970362-61305-1-git-send-email-feng.tang@intel.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/tools/perf/builtin-record.c?id=a38a59fdfa10be55d08e4530923d950e739ac6a2) |
| 2021/11/01 | "Aneesh Kumar K.V" <aneesh.kumar@linux.ibm.com> | [mm: add new syscall set_mempolicy_home_node](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=21b084fdf2a49ca1634e8e360e9ab6f9ff0dee11) | 增加了 set_mempolicy_home_node() 来为用户空间的一段地址 [start, srart + len] 指定内存分配的主节点 home_node. 主节点 home_node 应与 MPOL_PREFERRED_MANY 或 MPOL_BIND 内存分配策略结合使用. 这些策略可以指定一组将用于新内存分配的节点, 但不能说明这些节点中的哪个节点(如果有)是首选节点. 如果设置了 home_node, 则分配内存时将优先在该节点上进行分配; 否则, 主节点 home_node 内存不足, 则将回退到有效策略允许的其他节点上分配, 首选最接近主节点的节点. 其目的是让应用程序能够更好地控制内存分配, 同时避免来自慢速节点的内存. 参见 LWN 报道 [Some upcoming memory-management patches](https://lwn.net/Articles/875587). | v1 ☑✓ 5.17-rc1 | [PatchWork v4,0/3](https://patchwork.kernel.org/project/linux-mm/cover/20211101050206.549050-1-aneesh.kumar@linux.ibm.com) |
| 2022/04/12 | Wei Yang <richard.weiyang@gmail.com> | [mm/page_alloc: add same penalty is enough to get round-robin order](https://lore.kernel.org/all/20220412001319.7462-1-richard.weiyang@gmail.com) | 20220412001319.7462-1-richard.weiyang@gmail.com | v3 ☐☑✓ | [LORE](https://lore.kernel.org/all/20220412001319.7462-1-richard.weiyang@gmail.com) |


### 2.2.4 内存水线
-------



Linux 为每个 zone 都设置了独立的 min, low 和 high 三个档位的 watermark 值, 在代码中以struct zone中的 `_watermark[NR_WMARK]` 来表示.

*   在进行内存分配的时候, 如果伙伴系统发现当前空余内存的值低于"low"但高于"min", 说明现在内存面临一定的压力, 但是并不是非常紧张, 那么在此次内存分配完成后, kswapd将被唤醒, 以执行内存回收操作. 在这种情况下, 内存分配虽然会触发内存回收, 但不存在被内存回收所阻塞的问题, 两者的执行关系是异步的.

*   如果内存分配器发现空余内存的值低于了 "min", 说明现在内存严重不足. 那么这时候就有必要等待内存回收完成后, 再进行内存的分配了, 也就是 "direct reclaim". 但是这里面有个别特例, 内核提供了 PF_MEMALLOC 标记, 如果现在空余内存的大小可以满足本次内存分配的需求, 允许设置了 PF_MEMALLOC 标记的进程在内存紧张时, 先分配, 再回收. 比如 kswapd, 由于其本身就是负责回收内存的, 只需要满足它很小的需求, 它会回收大量的内存回来. 它就像公司濒临破产时抓到的一根救命稻草, 只需要小小的付出, 就会让公司起死回生.


#### 2.2.4.1 内存水线的引入
-------

*   早期的 pages_min, pages_low, pages_high

*   zone 中的  watermark[NR_WMARK]

v2.6.31 [Cleanup and optimise the page allocator V7](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=72807a74c0172376bba6b5b27702c9f702b526e9) 的过程中, [commit 418589663d60 ("page allocator: use allocation flags as an index to the zone watermark")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=418589663d6011de9006425b6c5721e1544fb47a) 引入了 enum zone_watermarks, 将原来 struct zone 中松散的 pages_min, pages_low, pages_high 封装成了 watermark[NR_WMARK]. 可以使用 `{min|low|high}_wmark_pages()` 直接访问 zone 对应的水线.


*   分配过程中的与水线有关的分配标记

v2.6.15-rc2, [commit 7fb1d9fca5c6 ("`mm: __alloc_pages cleanup`")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7fb1d9fca5c6e3b06773b69165a73f3fb786b8ee) 将从分配流程中抽象出了 get_page_from_freelist() 函数, 并引入了水线标记 ALLOC_NO_WATERMARKS, ALLOC_HARDER, ALLOC_HARDER 来辅助 zone_watermark_ok() 工作.

v2.6.15-rc3, [commit 3148890bfa4f ("`mm: __alloc_pages cleanup fix`")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3148890bfa4f36c9949871264e06ef4d449eeff9) 引入了 ALLOC_WMARK_MIN, ALLOC_WMARK_LOW, ALLOC_WMARK_HIGH.

v3.7-rc1, [commit d95ea5d18e69 ("cma: fix watermark checking")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d95ea5d18e699515468368415c93ed49b1a3221b) 又引入了 ALLOC_CMA, 并将这些 flags 都移动到了 `mm/internal.h` 文件中.

#### 2.2.4.2 内存水线的优化
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2004/09/05 | Nick Piggin <nickpiggin@yahoo.com.au> | [beat kswapd with the proverbial clue-bat](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d4cf10128caffbe419a483894261ca8d2f72c1eb) | KSWAPD 在更高阶的分配上非常愚蠢, 非常不公平. 解决方案非常简单, 只需以一种相当简单的方式让 kswapd 感知到内存水线和高阶分配. 其中<br>1. [mm: higher order watermarks](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d4cf10128caffbe419a483894261ca8d2f72c1eb) 引入了 zone_watermark_ok(), 并添加了对不同 order 的水线的检查.<br>2. [mm: teach kswapd about higher order areas](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d4cf10128caffbe419a483894261ca8d2f72c1eb) 增加了 KSWAPD 对不同 order 页面的处理. | v1 ☑✓ v2.6.11-rc1 | [LORE v1,0/3](https://lore.kernel.org/all/413AA7B2.4000907@yahoo.com.au) |
| 2011/01/07 | Satoru Moriya <satoru.moriya@hds.com> | [Tunable watermark](https://lore.kernel.org/patchwork/patch/231713) | 引入可调的水线. 为 min/low/high 各个水线都引入了一个 sysctl 接口用于调节. | v1 ☐ | [PatchWork RFC,0/2](https://lore.kernel.org/lkml/65795E11DBF1E645A09CEC7EAEE94B9C3A30A295@USINDEVS02.corp.hds.com) |
| 2013/02/17 | dormando <dormando@rydia.net><br>Rik van Riel <riel@redhat.com> | [add extra free kbytes tunable](https://lore.kernel.org/patchwork/patch/360274) | 默认内核中 min 和 low 之间的距离太短, 造成 kswapd 的作用空间太小, 从而导致频繁出现 direct reclaim. 这个补丁引入 extra_free_kbytes, 作为计算 low 时候的加权. 从而增大 min 和 low 之间的距离. | v1 ☐ | [PatchWork v5](https://lore.kernel.org/patchwork/patch/360274) |
| 2015/07/20 | Mel Gorman <mgorman@suse.com> | [Remove zonelist cache and high-order watermark checking](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=97a16fc82a7c5b0cfce95c05dfb9561e306ca1b1) | 1437379219-9160-1-git-send-email-mgorman@suse.com | v1 ☑✓ 4.4-rc1 | [LORE v1,0/10](https://lore.kernel.org/all/1437379219-9160-1-git-send-email-mgorman@suse.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/10](https://lore.kernel.org/all/1439376335-17895-1-git-send-email-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v3,00/12](https://lore.kernel.org/all/1440418191-10894-1-git-send-email-mgorman@techsingularity.net) |
| 2016/02/22 | Johannes Weiner <hannes@cmpxchg.org> | [mm: scale kswapd watermarks in proportion to memory](https://lore.kernel.org/patchwork/patch/649909) |  | v2 ☑ 4.6-rc1 | [PatchWork v5](https://lore.kernel.org/patchwork/patch/360274) |
| 2018/11/23 | Mel Gorman | [Fragmentation avoidance improvements v5](https://lore.kernel.org/patchwork/patch/1016503) | 伙伴系统页面分配时的反碎片化 | v5 ☑ 5.0-rc1 | [PatchWork v5](https://lore.kernel.org/patchwork/patch/1016503) |
| 2020/02/25 | Mel Gorman | [Limit runaway reclaim due to watermark boosting](https://lore.kernel.org/patchwork/patch/1200172) | 优化调度器的路径, 减少对 rq->lock 的争抢, 实现 lockless. | v4 ☑ 4.4-rc1 | [PatchWork v6](https://lore.kernel.org/patchwork/patch/1200172) |
| 2020/06/11 |Charan Teja Kalla <charante@codeaurora.org> | [mm, page_alloc: skip ->waternark_boost for atomic order-0 allocations](https://lore.kernel.org/patchwork/patch/1254998) | NA | v1 ☑ 5.9-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/1244272), [](https://lore.kernel.org/patchwork/patch/1254998) |
| 2020/10/20 |Charan Teja Kalla <charante@codeaurora.org> | [mm: don't wake kswapd prematurely when watermark boosting is disabled](https://lore.kernel.org/patchwork/patch/1322999) | NA | v1 ☑ 5.11-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/1244272), [PatchWork](https://lore.kernel.org/patchwork/patch/1322999) |
| 2020/05/01 |Charan Teja Kalla <charante@codeaurora.org> | [mm: Limit boost_watermark on small zones.](https://lore.kernel.org/patchwork/patch/1234105) | NA | v1 ☑ 5.11-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/1234105) |



### 2.2.5 PCP(Per CPU Page) Allocation
-------

[说说 perf 里面的 kmem(顺便聊下 pcp)](https://zhuanlan.zhihu.com/p/479922331)

伙伴系统 buddy 是最基础的页面管理机制, 每次从 buddy 分配意味着需要持有对应的 zone lock, 这可能会导致频繁的锁竞争. 因此内核考虑为每个 CPU 增加了一个本地页面的缓存池, 就是 per-cpu page allocator(PCP).

1.  当 get_page_from_freelist() 分配页面的时候, 如果允许的话(参见 pcp_allowed_order()), 会优先从 PCP 中去获取页面.

2.  当 PCP 没有空闲页面可供分配时, 就不得不从 zone buddy 里面再捞一些页面出来, 由于此过程只是一个空闲页面的转移(从 zone buddy 的 freelist 到 pcp 的 freelist), 所以并不算是 "alloc", 而是 "refill".

3.  反过来, 如果一个 CPU 上的 PCP 的空闲页面太多了(超过上限 pcp->high), 就要还一些给 zone buddy, 该过程叫做 "drain".

#### 2.2.5.1 冷热页 [Hot and cold pages](https://lwn.net/Articles/14768)
-------

[Linux 中的冷热页机制概述](https://toutiao.io/posts/d4cz9u/preview)

[Hot and cold pages](https://lwn.net/Articles/14768)

[内存管理中的cold page和hot page,  冷页 vs 热页](https://blog.csdn.net/kickxxx/article/details/9306361)

*  冷热页的引入

v2.5.45, Martin Bligh 和 Andrew Morton 以及其他人提交了一个内核分配器 patch, 引入了 hot-n-cold pages 的概念, 这个概念本身是和现在处理器架构息息相关的. 尽量利用处理器 cache, 避免使用主存, 可以提升性能. hot-cold page 就是这样的思路. 处理器cache保存着最近访问的内存. kernel 认为最近访问的内存很有可能存在于 cache 之中. 冷页表明该空闲页大概率已经不在 CPU Cache 中了, 热页则表明该空闲页大概率仍然在 CPU Cache 中.

*   区分冷热页的好处

1.  发挥软件 Cache 高速缓存作用: Buddy Allocator 在分配 order 为 0 的空闲页的时候, 如果分配一个热页, 由于该页已经存在于 CPU Cache 缓存中, CPU 可以直接写. 如果分配一个冷页, 说明该页不在 CPU Cache 中, 需访问内存. 一般情况下, 尽可能分配热页, 但如果是 DMA 请求, 需要连续的页面, 要分配冷页.

2.  降低了 CPU 间的页面竞争: Buddy System 在给某个进程分配某个 zone 中空闲页的时候, 首先需要用自旋锁锁住该 zone, 然后分配页. 这样, 如果多个 CPU 上的进程同时进行分配页, 便会竞争. 引入了 per-cpu-set 后, 当多个 CPU 上的进程同时分配页的时候, 竞争便不会发生, 提高了效率. 另外当释放单个页面时, 空闲页面首先放回到 per-cpu-pageset 中, 以减少 zone->lock 自旋锁的争用. 当页面缓存中的页面数量超过阀值时, 再将页面放回到伙伴系统中.

3.  使用每个 CPU 独立冷热页, 能保证某个页面一直黏在 1 个 CPU 上, 这有助于提高 Cache 的命中率.

*   冷热页的设计与实现

冷热页是针对于 CPU 缓存而言的, 但是软件设计上, 每个 zone 中, 都会针对于所有的 CPU 初始化一个冷热页的 per-CPU 的页面缓存 [per-cpu-pageset](https://elixir.bootlin.com/linux/v2.5.45/source/include/linux/mmzone.h#L125), 页面缓存中包含了[ cold 和 hot 两种页面](https://elixir.bootlin.com/linux/v2.5.45/source/include/linux/mmzone.h#L59) 每个内存zone). 当 kernel 释放的 page 可能是 hot page 时([page cache 的页面往往被认为是在 cache 中的](https://elixir.bootlin.com/linux/v2.5.45/source/mm/swap.c#L87), 是 hot 的), 那么就把它[放入hot链表](https://elixir.bootlin.com/linux/v2.5.45/source/mm/page_alloc.c#L558), 否则放入 cold 链表.

1.  当 kernel 需要分配一个page时, 新分配器通常会从 per-CPU 的 hot list 获取页面, 甚至我们获得的页面马上就要写入新数据的情况下, 仍然能获得较好的速度.

2.  当然也有些情况下, 申请 hot page 不会获得性能上的提高, 只要申请 cold page 就可以了. 比如 [DMA 读操作需要的内存分配](https://elixir.bootlin.com/linux/v2.5.45/C/ident/page_cache_alloc_cold), 设备会直接修改内存并且无效相应的 cache. 所以内核分配器提供了 [GFP_COLD分配标记](https://elixir.bootlin.com/linux/v2.5.45/source/mm/page_alloc.c#L412) 来显式从 cold page 链表分配内存.

此外使用 per-CPU page 链表也削减了锁竞争, 提高了性能. Andrew Morton 测试了这个patch, 在不同环境下获得了 `%1~%12` 不等的性能提升.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/10/29 | Jan Kara <jack@suse.cz> | [hot-n-cold pages: bulk page allocator](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=8d6282a1cf812279f490875cd55cb7a85623ac89) | 区分热页面和冷页面. | v2 ☑ 2.5.45 | [CGIT, 0/5](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=8d6282a1cf812279f490875cd55cb7a85623ac89) |

后来经过测试后, 大家一致认为, 将热页面和冷页面分开列出可能没有什么意义. 因为有一种方法可以连接这两个列表: 使用单一列表, 将冷页面放在末尾, 将热页面放在开头. 这样, 一个列表就可以为这两种类型的分配服务. [Page allocator: get rid of the list of cold pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3dfa5721f12c3d5a441448086bee156887daa961).

这样 Hot Page 和 Cold Page 也已经没有那么明确的界限, 因此 free_cold_page() 和 free_hot_page() 函数已经没意义了, 随后在一些 cleanup 中被删除掉. 直接使用 free_hot_col_page() 来操作 PCP 列表. 参见 v2.6.32-rc1 [page-allocator: Remove dead function free_cold_page()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=38a398572fa2d8124f7479e40db581b5b72719c9). 以及 v2.6.34-rc1 [mm: remove free_hot_page()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=fc91668eaf9e7ba61e867fc2218b7e9fb67faa4f).



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/04/22 | Christoph Lameter <clameter@sgi.com> | [Page allocator: get rid of the list of cold pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3dfa5721f12c3d5a441448086bee156887daa961) | 清理和优化页面分配器 [Cleanup and optimise the page allocator V7](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=72807a74c0172376bba6b5b27702c9f702b526e9) 的其中也提到了类似补丁.  | v7 ☑✓ 2.6.31-rc1 | [LORE RFC,20/20](hhttps://lore.kernel.org/lkml/1235344649-18265-21-git-send-email-mel@csn.ul.ie), [LORE v1](https://marc.info/?l=linux-mm&m=119492902717327) |
| 2009/08/05 | Mel Gorman <mel@csn.ul.ie> | [page-allocator: Remove dead function free_cold_page()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=38a398572fa2d8124f7479e40db581b5b72719c9) | NA | v1 ☑✓ 2.6.32-rc1 | [LORE](https://lore.kernel.org/lkml/20090805102817.GE21950@csn.ul.ie) |
| 2017/10/17 | Li Hong <lihong.hi@gmail.com> | [mm: remove free_hot_page()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fc91668eaf9e7ba61e867fc2218b7e9fb67faa4f) | 移除了 free_hot_page(), 直接使用 [free_hot_cold_page()](https://elixir.bootlin.com/linux/v2.6.34/source/mm/page_alloc.c#L1102). | v1 ☑✓ 2.6.34-rc1 | [LORE v1,3/3](https://lore.kernel.org/lkml/3a3680031001130654q1928df60pde0e3706ea2461c@mail.gmail.com/) |
| 2017/10/17 | Jan Kara <jack@suse.cz> | [Speed up page cache truncation](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=aa65c29ce1b6e1990cd2c7d8004bbea7ff3aff38) | 20171017162120.30990-1-jack@suse.cz | v1 ☑✓ 4.15-rc1 | [LORE v1,0/7](https://lore.kernel.org/all/20171017162120.30990-2-jack@suse.cz) |



这个阶段的 PCP 只支持 order-0 的页面, 然后 pcp->lists 中存储了 PCP 的页面, 其中热页在前(队头), 冷页在后(队尾). CPU 每释放一个 order-0 的页, 如果 per-cpu-pageset 中的页数少于其指定的阈值, 便会将释放的页插入到冷热页链表的开始处. 与 LRU 类似, 之前插入的热页便会随着后续热页源源不断的插入向后移动, 其页由热变冷的几率便大大增加. 对于链表适宜长度, 参考热页变冷需要维护一个多大数量的链表.

首先是分配, 参照 v2.6.34 版本:

get_page_from_freelist() 中分配 order-0 页面的时候, 默认从 PCP->list 的队头(大概率是热页)中直接获取页面出来.

```cpp
static struct page *get_page_from_freelist(gfp_t gfp_mask, nodemask_t *nodemask, unsigned int order,
                        struct zonelist *zonelist, int high_zoneidx, int alloc_flags,
                        struct zone *preferred_zone, int migratetype)
{
    /*
     * Scan zonelist, looking for a zone with enough free.
     * See also cpuset_zone_allowed() comment in kernel/cpuset.c.
     */
     for_each_zone_zonelist_nodemask(zone, z, zonelist, high_zoneidx, nodemask) {
        if (!(alloc_flags & ALLOC_NO_WATERMARKS)) {
            ret = zone_reclaim(zone, gfp_mask, order);

        if (zone_watermark_ok(zone, order, mark, classzone_idx, alloc_flags))
            goto try_this_zone;

        if (zone_reclaim_mode == 0)
            goto this_zone_full;
        }

try_this_zone:
        page = buffered_rmqueue(preferred_zone, zone, order, gfp_mask, migratetype);

     }
}

static inline
struct page *buffered_rmqueue(struct zone *preferred_zone, struct zone *zone, int order, gfp_t gfp_flags, int migratetype)
{
    int cold = !!(gfp_flags & __GFP_COLD);
again:
    if (likely(order == 0)) {
        list = &pcp->lists[migratetype];
        if (list_empty(list)) {
            pcp->count +=  (zone, 0, pcp->batch, list, migratetype, cold);
        }

        if (cold)
            page = list_entry(list->prev, struct page, lru);
        else
            page = list_entry(list->next, struct page, lru);

        list_del(&page->lru);
        pcp->count--;
    } else {
             page = __rmqueue(zone, order, migratetype);
    }
    if (prep_new_page(page, order, gfp_flags))
        goto again;
}
```


接着来看释放:

刚释放的内存页大概率还在 CPU 的 Cache 里, 因此这些页面还是热的. 因此 free_pages() 释放单个内存页的时候会调用 free_hot_cold_page() 将页面交还给 PCP, 并且默认放在 pcp->list 的头部. 这样每次申请页面从队头申请到的就可以认为是热页缓存.


```cpp
void __free_pages(struct page *page, unsigned int order)
{
    if (put_page_testzero(page)) {
        if (order == 0)
            free_hot_cold_page(page, false);
        else
             __free_pages_ok(page, order);
    }
}

void free_hot_cold_page(struct page *page, bool cold)
{
    pcp = &this_cpu_ptr(zone->pageset)->pcp;
    if (!cold)
        list_add(&page->lru, &pcp->lists[migratetype]);
    else
        list_add_tail(&page->lru, &pcp->lists[migratetype]);
    pcp->count++;
    if (pcp->count >= pcp->high) {
        unsigned long batch = READ_ONCE(pcp->batch);
        free_pcppages_bulk(zone, batch, pcp);
        pcp->count -= batch;
    }
}
```

内核默认从 PCP 分配页面都是倾向于分配热页(队头)的, 但是有一些路径下, 可能存在冷页面分配的诉求, 因此内核提供了 `__GFP_COLD` 用来显式从 PCP 中分配冷页面. 最普遍的比如 Page Cache 等页面, page_cache_read() 以及 do_read_cache_page() 中 `__page_cache_alloc()` 中都传递了 `__GFP_COLD` 分配冷页面. 内核甚至提供了 [page_cache_alloc_cold()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=44110fe385af23ca5eee8a6ad4ff55d50339097a) 来从冷页中分配 Page Cache.

但是, 从上面的实现可以发现, PCP 中的页面其实并没有明确的冷热页面区分, 当前空闲列表中没有真正有用的页面冷热排序, 分配请求无法利用这些冷热排序, `__GFP_COLD` 其实并没有太大的意义. 因此 v4.15 直接将 `__GFP_COLD` 标记删掉 [mm: remove `__GFP_COLD`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=453f85d43fa9ee243f0fc3ac4e1be45615301e3f). 删除 `__GFP_COLD` 参数还简化了页面分配器中的一些路径.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/10/18 | Mel Gorman <mgorman@techsingularity.net> | [mm: remove `__GFP_COLD`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=453f85d43fa9ee243f0fc3ac4e1be45615301e3f) | [Follow-up for speed up page cache truncation v2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=453f85d43fa9ee243f0fc3ac4e1be45615301e3f) 的其中一个补丁. | v2 ☑ 4.15-rc1 | [PatchwWork v2](https://lore.kernel.org/lkml/20171018075952.10627-1-mgorman@techsingularity.net) |


#### 2.2.5.2 Drain/Reclaim PCP
-------

引入 PCP 的时候, 并没有考虑内存不足时回收所有 PCP 页面的情况, PCP 中的页面数量超出 high 阈值时会主动归还一些页面, 并不属于回收. 此外只有 CPU offline 和 suspend 流程会释放当前释放当前 CPU 上 PCP 的所有页面. 其中 [offline(offline_pages())](https://elixir.bootlin.com/linux/v2.6.24/source/mm/memory_hotplug.c#L566) 流程使用 drain_all_local_pages(), suspend 时直接通过 [`__drain_pages()`](https://elixir.bootlin.com/linux/v2.6.24/source/mm/page_alloc.c#L3982) 释放所有 CPU 的 PCP.

因为普遍认为这是一个百利而无一害的 CPU 高速页面缓存. 但是事实真的如此么?

PCP 页面对内存碎片可能有潜在的冲击, 它可能会意外导致碎片, 因为它们是空闲的, 但是却不属于伙伴系统直接俄管辖范围内, 这些页面可能是从一块大的连续页面块中扣掉的一块.


*   发送 IPI 去 Drain PCP

因此 v2.6.24 引入 migratetype 来缓解内存碎片化的时候, 引入了 PCP 页面的回收机制 drain_all_local_pages(). 如果内存分配器分配非 Order 0 的页面时, 没有足够的空间来完成分配, 则在[直接回收(这个时代的直接回收仅仅是直接调用了 try_to_free_pages())](https://elixir.bootlin.com/linux/v2.6.24/source/mm/page_alloc.c#L1564)之后, 触发 [drain_all_local_pages()](https://elixir.bootlin.com/linux/v2.6.24/source/mm/page_alloc.c#L1572) 来回收所有 PCP 的页面. 这是直接借用了现成的 suspend 和 hotplug 路径的代码来完成的, 通过 smp_call_function() 给每个 CPU 发送 IPI 然后执行 smp_call_function() 来完成 PCP 的回收的.

之前给所有 CPU 强制发送 IPI 去清理 PCP 页面的方式看起来不是那么智能, 因此 [commit 74046494ea68 ("mm: only IPI CPUs to drain local pages if they exist")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=74046494ea68676d29ef6501a4bd950f08112a2c) 在 drain_all_pages() 中只给所有拥有 PCP 的 CPU(cpus_with_pcps) 发送 IPI 去 drain_local_pages(), 从而减少了 IPI 发送的数量.

|  时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:---:|:---:|:----------:|:----:|
| 2007/09/10 | Mel Gorman <mel@csn.ul.ie> | [Drain per-cpu lists when high-order allocations fail](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e2c55dc87f4a398b9c4dcc702dbc23a07fe14e23) | [Reduce external fragmentation by grouping pages by mobility v30](https://lore.kernel.org/patchwork/patch/75208) | 基于页面可移动性的页面聚类来抗(外部)碎片化的其中一个补丁. 尝试直接分配非 order-0 的页面失败, 在慢速路径上直接回收之后, 尝试通过 drain_all_local_pages() 回收所有 PCP 的内存. | v30 ☑ 2.6.24-rc1 | [Patchwork v30,0/13](https://lore.kernel.org/lkml/20070910112011.3097.8438.sendpatchset@skynet.skynet.ie), [关注 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e2c55dc87f4a398b9c4dcc702dbc23a07fe14e23) |
| 2008/02/04 | Christoph Lameter <clameter@sgi.com> | [Page allocator: clean up pcp draining functions](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9f8f2172537de7af0b0fbd33502d18d52b1339bc) | 1. 重命名 drain_all_local_pages() 为 drain_all_pages(), 因为它清理的其实是所有 CPU 的 PCP 页面, 而不仅仅是 local 的.<br>2. drain_all_pages() 使用 on_each_cpu() 替代 smp_call_function(). | v1 ☑✓ 2.6.25-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9f8f2172537de7af0b0fbd33502d18d52b1339bc) |
| 2012/02/09 | Gilad Ben-Yossef <gilad@benyossef.com> | [mm: only IPI CPUs to drain local pages if they exist](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=74046494ea68676d29ef6501a4bd950f08112a2c) | [Reduce cross CPU IPI interference](https://lore.kernel.org/all/1328776585-22518-1-git-send-email-gilad@benyossef.com) 减少系统中 IPI 数量优化的其中一个补丁, drain_all_pages() 中不再直接发 IPI 给各个 CPU 去 drain_all_pages(), 而是先统计下当前拥有 PCP 页面的 CPU, 标记为 cpus_with_pcps, 只对这些 CPU 发送 IPI. | v9 ☑✓ 3.4-rc1 | [LORE v9,8/8](https://lore.kernel.org/lkml/1328776585-22518-8-git-send-email-gilad@benyossef.com) |

*   使用统一的 WQ_RECLAIM WorkQueue 来 Drain PCP

原本内存分配(慢速路径)下 Drain PCP 的操作都是通过 IPI 在中断上下文来完成的, v4.11 为了实现 Order-0 页面的批量分配(Bulk Allocator), 优化了页面分配的性能, 引入了中断安全的 Per CPU Allocator. 虽然这个特性很快因为性能回归被 [revert](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d34b0733b452ca3ef1dee36436dab08b8aa6a85c). 但是其他重构却被保留, 其中就包含 [commit 0ccce3b92421 ("mm, page_alloc: drain per-cpu pages from workqueue context")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0ccce3b924212e121503619df97cc0f17189b77b) 不再通过发送 IPI 的方式去通知其他 CPU 释放其 PCP 页面, 通过引入 per-cpu 的 drain_local_pages_wq 将释放 PCP 的页面放到 workqueue 上去执行.


紧接着 [commit bd233f538d51 ("mm, page_alloc: use static global work_struct for draining per-cpu")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bd233f538d51c2cae6f0bfc2cf7f0960e1683b8a) 使用了[静态的 per-cpu 的 work_struct pcpu_drain](https://elixir.bootlin.com/linux/v4.11/source/mm/page_alloc.c#L97) 来替代之前 drain_all_pages() 中每次动态通过 alloc_percpu_gfp() 申请的 workqueue.

此时 mm 中有 2 个特定的 WQ_MEM_RECLAIM 工作队列. [vmstat_wq](https://elixir.bootlin.com/linux/v4.10/source/mm/vmstat.c#L1550) 用于 vmstat_update() [更新 PCP 状态 refresh_cpu_vm_stats()](https://elixir.bootlin.com/linux/v4.10/source/mm/vmstat.c#L1713), 参见 [commit d1187ed21026 ("vmstat: use our own timer events")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d1187ed21026fd512b87851d0ca26d9ae16f9059), [lru_add_drain_wq](https://elixir.bootlin.com/linux/v4.10/source/mm/swap.c#L676) 用于 lru_add_drain_all() [清理每个 CPU 的 LRU 缓存](https://elixir.bootlin.com/linux/v4.10/source/mm/swap.c#L709). 但是这两个 workqueue 都可以在一个 WQ 上运行, 因为两者都不会阻塞需要分配内存的锁, 也不会执行任何分配. 但是再来看 drain_all_pages() 回收 PCP 页面时使用的 pcpu_drain, 它直接通过 schedule_work_on 使用了系统全局的 system wq, 这也是没必要的, 它也应该与 lru drain 和 vmstat 一样使用 WQ_MEM_RECLAIM. 因此 [commit ce612879ddc7 ("mm: move pcp and lru-pcp drainging into single wq")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=`ce612879ddc78ea7e4de4be80cba4ebf9caa07ee`) 引入了 mm_percpu_wq 统一管理上述三种 WORK.

|  时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:---:|:---:|:----------:|:----:|
| 2017/01/23 | Mel Gorman | [mm, page_alloc: drain per-cpu pages from workqueue context](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0ccce3b924212e121503619df97cc0f17189b77b) | [Use per-cpu allocator for !irq requests and prepare for a bulk allocator v5](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=374ad05ab64d696303cec5cc8ec3a65d457b7b1c) 重构 Per CPU Pages 分配器的其中一个补丁. 不再使用 IPI(在中断上下文)去释放 PCP, 而是 [将 drain_local_pages_wq() 将 drain_local_pages() 放到 WORK 中去完成](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0ccce3b924212e121503619df97cc0f17189b77b), 从而防止 PCP 在 IPI 等中断上下文中被释放. | v5 ☑ 4.11-rc1 | [LORE RFC](https://lore.kernel.org/all/20170207201950.20482-1-mhocko@kernel.org), [LORE v5,0/4](https://lore.kernel.org/all/20170123153906.3122-1-mgorman@techsingularity.net), [关注 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0ccce3b924212e121503619df97cc0f17189b77b) |
| 2017/02/24 | Mel Gorman <mgorman@techsingularity.net> | [mm, page_alloc: use static global work_struct for draining per-cpu](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bd233f538d51c2cae6f0bfc2cf7f0960e1683b8a) | 使用静态的 per-cpu 的 work_struct pcpu_drain 来替代之前 drain_all_pages() 中每次动态通过 alloc_percpu_gfp() 申请的 workqueue. | v1 ☑✓ 4.11-rc1 | [LORE](https://lore.kernel.org/lkml/20170125083038.rzb5f43nptmk7aed@techsingularity.net/), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bd233f538d51c2cae6f0bfc2cf7f0960e1683b8a) |
| 2017/03/07 | Michal Hocko <mhocko@kernel.org> | [mm: move pcp and lru-pcp drainging into single wq](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ce612879ddc78ea7e4de4be80cba4ebf9caa07ee) | 引入一个统一名为 mm_percpu_wq 的 WQ_MEM_RECLAIM workqueue 来接管当前 MM 下存在的 vmstat_work, lru_add_drain_per_cpu 和 drain_local_pages_wq. | v1 ☑✓ 4.11-rc6 | [LORE RFC](https://lore.kernel.org/all/20170207210908.530-1-mhocko@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v1](https://lore.kernel.org/all/20170307131751.24936-1-mhocko@kernel.org) |

*   Remote per-cpu cache access

最初释放 PCP 是通过 IPI 来完成的, 并且经历了不断的修正和优化, 参见 v2.6.24-rc1 的 [Drain per-cpu lists when high-order allocations fail](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e2c55dc87f4a398b9c4dcc702dbc23a07fe14e23), v2.6.25-rc1 的 [Page allocator: clean up pcp draining functions](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9f8f2172537de7af0b0fbd33502d18d52b1339bc), v3.4-rc1 的 [mm: only IPI CPUs to drain local pages if they exist](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=74046494ea68676d29ef6501a4bd950f08112a2c).

随后 v4.11-rc1 的 [mm, page_alloc: drain per-cpu pages from workqueue context](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0ccce3b924212e121503619df97cc0f17189b77b).

然而即使经历了这么多年的修改, 这个解决方案并不完美. 使用 workqueue 的方式来释放 PCP lists, 最好的情况是, 只需要目标 CPU 上一次上下文切换(切换到 kworker)来运行释放 PCP 列表的回调就可以了. 但是, 如果目标 CPU 处于 tickless 模式, 或者它正在运行高优先级实时任务, 那么工作队列条目可能在很长一段时间内根本不运行. 因此, CPU 上的任何空闲页面都将被锁定在其本地列表中.

想要完美的解决这个问题, 就需要要在一个 CPU 上即可完成系统中所有 CPU 的 PCP 页面, 这就需要提供一种机制, 能够让一个 CPU 可以安全地访问 其他 remote CPU 的 PCP 页面.

2021 年, Nicolas Saenz Julienne 的 [mm/page_alloc: Remote per-cpu page list drain support](https://lore.kernel.org/all/20211103170512.2745765-1-nsaenzju@redhat.com) 通过允许远程 CPU 的本地列表中获取页面来缓解这个问题. 尝试添加了自旋锁来控制对 per-CPU 列表的访问, 从根本上消除了它们的 per-CPUness; 这个解决方案是有效的, 但是它只是增加了创建 per-CPU 列表以避免的开销. 所以这些补丁没有进入内核.

这个算法使用了 RCU 的方式来完成 remote CPU 释放 PCP 页面的操作.<br>将 PCP 页面列表结构封装到 pcplists 中, 每个 CPU 现在都有两组列表来保存空闲页面: lp(local_page) 和 drain, 其中一组在任何给定的时间都在使用, 而另一组则保留在备用状态(并且是空的). 当系统想要释放所有 CPU 的 PCP 页面时, 就就通过 rcu_replace_pointer() 交换两个指针, 并通过 synchronize_rcu() 等待宽限期结束后对 PCP 页面进行释放. 通过 RCU 这种方式, 不用再通过以前 IPI 或者 workqueue 的方式在 per-cpu 上去完成, 而是通过一个 remote CPU 即可释放系统中所有 CPU 的 PCP 页面.<br>
其主要优点是它可以很好地解决问题, 避免了对基于配置的启发式方法的需求或不得不修改应用程序(即使用 Marcello Tosatti ATM 正在工作的隔离 prctrl). 参见 [Remote per-CPU page list draining](https://lwn.net/Articles/884448).

但是在经历了多个版本之后, 并没有被社区所认可. 2022 年, MM 领域的大佬 Mel Gorman 接手了这项工作. 发布了 [Drain remote per-cpu directly](https://patchwork.kernel.org/project/linux-mm/cover/20220420095906.27349-1-mgorman@techsingularity.net). 与 Nicolas 之前 RCU 的方式类似, 他们都移除了以前 WorkQueue 中释放 PCP list 的方案, 但是解决的方式略有不同, Mel 直接使用了 IRQ 安全的 local_lock() 保护 PCP lists. local_lock 本身可以防止迁移, 禁用 IRQ 可以防止在页面分配过程中由于被中断打断而造成异常. 一个自旋锁 [spinlock](https://lore.kernel.org/all/20220509130805.20335-6-mgorman@techsingularity.net) 被添加到 per_cpu_pages 结构中, 以保护列表的内容, 而 local_lock_irq 则进一步阻止迁移和 IRQ 重入. 从而允许一个 CPU 安全地清空远程 PCP lists.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/09/21 | Sebastian Andrzej Siewior <bigeasy@linutronix.de> | [mm/swap: Add static key dependent pagevec locking](https://patchwork.kernel.org/project/linux-mm/cover/20190424111208.24459-1-bigeasy@linutronix.de) | 本系列实现了 swap 的代码路径中通过禁用抢占来同步它对 per-cpu pagevec 结构体的访问. 这是可行的, 需要从中断上下文访问的结构体是通过禁用中断来保护的.<br>有一种情况下, 需要访问远程 CPU 的 per-cpu 数据, 在 v1 版本中, 试图添加每个 cpu 的自旋锁来访问结构体. 这将增加 lockdep 覆盖率和从远程 CPU 的访问, 不需要 worker.<br>在 v2 中这是通过在远程 CPU 上启动一个 worker 并等待它完成来解决的.<br>关于无争用 spin_lock () 的代价, 以及避免每个 cpu worker 的好处很少, 因为它很少被使用. 然后听从社区的建议使用 static key use_pvec_lock, 在某些情况下(如 NOHZ_FULL 情况), 它支持每个 cpu 的锁定. | v1 ☐ | [PatchWork 0/4,v2](https://patchwork.kernel.org/project/linux-mm/cover/20190424111208.24459-1-bigeasy@linutronix.de) |
| 2021/09/21 | Nicolas Saenz Julienne <nsaenzju@redhat.com> | [mm: Remote LRU per-cpu pagevec cache/per-cpu page list drain support](https://patchwork.kernel.org/project/linux-mm/cover/20210921161323.607817-1-nsaenzju@redhat.com) | 本系列介绍了 mm/swap.c 的每个 CPU LRU pagevec 缓存和 mm/page_alloc 的每个 CPU 页面列表的另一种锁定方案 remote_pcpu_cache_access, 这将允许远程 CPU 消耗它们.<br>目前, 只有一个本地 CPU 被允许更改其每个 CPU 列表, 并且当进程需要它时, 它会按需这样做 (通过在本地 CPU 上排队引流任务).<br>大多数系统会迅速处理这个问题, 但它会给 NOHZ_FULL CPU 带来问题, 这些 CPU 无法在不破坏其功能保证(延迟、带宽等) 的情况下接受任何类型的中断. 如果这些进程能够远程耗尽列表本身, 就可以与隔离的 CPU 共存, 但代价是更多的锁约束.<br>通过 static key remote_pcpu_cache_access 来控制该特性的开启与否, 对于非 nohz_full 用户来说, 默认禁用它, 这保证了最小的功能或性能退化. 而只有当 NOHZ_FULL 的初始化过程成功时, 该特性才会被启用. | v2 ☐ | [2021/09/21 PatchWork 0/6](https://patchwork.kernel.org/project/linux-mm/cover/20210921161323.607817-1-nsaenzju@redhat.com)<br>*-*-*-*-*-*-*-* <br>[2021/11/03 PatchWork v2,0/3](https://patchwork.kernel.org/project/linux-mm/cover/20211103170512.2745765-1-nsaenzju@redhat.com) |
| 2022/02/08 | Nicolas Saenz Julienne <nsaenzju@redhat.com> | [mm/page_alloc: Remote per-cpu lists drain support](https://lore.kernel.org/all/20220208100750.1189808-1-nsaenzju@redhat.com) | 参见 [Remote per-CPU page list draining](https://lwn.net/Articles/884448) | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20220208100750.1189808-1-nsaenzju@redhat.com) |
| 2022/02/24 | Suren Baghdasaryan <surenb@google.com> | [mm: page_alloc: replace mm_percpu_wq with kthreads in drain_all_pages](https://lore.kernel.org/all/20220225012819.1807147-1-surenb@google.com) | 20220225012819.1807147-1-surenb@google.com | v1 ☐☑✓ | [LORE v1,0/1](https://lore.kernel.org/all/20220225012819.1807147-1-surenb@google.com) |
| 2022/05/09 | Mel Gorman <mgorman@techsingularity.net> | [Drain remote per-cpu directly](https://patchwork.kernel.org/project/linux-mm/cover/20220420095906.27349-1-mgorman@techsingularity.net) | 直接使用了 IRQ 安全的 local_lock() 保护 PCP lists. local_lock 本身可以防止迁移, 禁用 IRQ 可以防止在页面分配过程中由于被中断打断而造成异常. 一个自旋锁 [spinlock](https://lore.kernel.org/all/20220509130805.20335-6-mgorman@techsingularity.net) 被添加到 per_cpu_pages 结构中, 以保护列表的内容, 而 local_lock_irq 则进一步阻止迁移和 IRQ 重入. 从而允许一个 CPU 安全地清空远程 PCP lists. | v1 ☐☑ | [2022/04/20 LORE v1,0/6](https://lore.kernel.org/r/20220420095906.27349-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[2022/05/09 LORE v2,0/6](https://lore.kernel.org/r/20220509130805.20335-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/6](https://lore.kernel.org/r/20220512085043.5234-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[2022/06/13 LORE v4,0/7](https://lore.kernel.org/r/20220613125622.18628-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[2022/06/24 LORE v5,0/7](https://lore.kernel.org/r/20220624125423.6126-1-mgorman@techsingularity.net) |


#### 2.2.5.3 PCP Batch-Free free_pcppages_bulk
-------

*   free_pages_bulk() 批量释放整个 list

内核最早引入冷热页的时候, [commit 44110fe385af ("hot-n-cold pages: bulk page freeing")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1d2652dd2c3e942e75dc3137b3cb1774b43ae377) 实现了 free_pages_bulk() 完成一整个 list page 的页面, 这个被用于 PCP 页面的批量释放.

*   引入 MIGRATETYPE 后, round-robin fashion 与 batch-free

由于 free_pages_bulk() 一直用来释放 PCP 的页面, 因此 v2.6.24-rc1, 优化 PCP 的 MIGRATETYPE 感知的时候, 直接将该函数[重命名为 free_pcppages_bulk()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=5f8dcc21211a3d4e3a7a5ca366b469fb88117f61). 通过 pcp->list 按照 MIGRATETYPE 拆分成了多个 list, 分配的时候按照 MIGRATETYPE 分配, 那么批量释放的时候, 由于不能明确每个 MIGRATETYPE 应该释放多少页面, 因此实现了一个 [round-robin fashion](https://elixir.bootlin.com/linux/v2.6.32/source/mm/page_alloc.c#L545) 的模式, 循环着为每种 MIGRATETYPE 依次释放页面, 其中 [COMMIT1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5f8dcc21211a3d4e3a7a5ca366b469fb88117f61) 引入了 round-robin fashion, [COMMIT2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a6f9edd65beaef24836e8934c8912c1e974dd45c) 在函数中引入了一个 batch_free 的计数, 记录遇到空 PCP MIGRATETYPE list 时需要释放的页面数量, 从而一定程度上减少了在空 PCP MIGRATETYPE list 上遍历的次数.

随后 v2.6.39-rc1, 对 batch_free 进一步做了优化, 如果当前 PCP list 上一个非空的 lists[MIGRATETYPE], 则不用再频繁地做 round-robin fashion 遍历了, 直接对这个 lists[MIGRATETYPE] 做批量释放就可以了. 参见 [commit 1d16871d8c96 ("mm: batch-free pcp list if possible")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1d16871d8c96deadc5f9753b6b096074f2cbcbe1).

*   减少 zone->lock 的持锁时间 以及 prefetch_buddy

free_pcppages_bulk() 中批量释放页面的过程可能会耗时比较久, 这个过程一直持有 zone->lock 这个热锁是非常不合适的. 因此 v4.17-rc1 [mm: improve zone->lock scalability](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=97334162e4d79f866edd7308aac0ab3ab7a103f7) 对此现状进行了优化. 其中

[COMMIT2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0a5f4e5b45625e75db85b4968fc4c232d8091143) 将 PCP 上待释放页面查找的动作放到了锁外, 引入了一个临时的 LIST_HEAD(head), 在不持有 zone->lock 的情况下将待释放的页面缓存到这个链表上, 然后再在持锁的情况下, 遍历链表对页面进行释放.

[COMMIT3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=97334162e4d79f866edd7308aac0ab3ab7a103f7) 则实现了 prefetch_buddy 机制. 当一个页面被释放回全局池时, 它的伙伴将被检查是否有可能进行合并. 这需要访问 Buddy 的页面结构, 如果是缓存冷的, 访问可能需要很长时间. 在没有持有 zone->lock 的时候增加了一个预取, 希望以后在 zone->lock 下访问 buddy 的页面结构会更快. 由于伙伴系统 "总是" 会合并临近的兄弟页面, 并检查 order-0 页面的好友, 以便在它进入主分配器时尝试合并它, cacheline 总是会进来, 也就是说, 预取的数据将总是会被用到的.

**关于预取的数量**, 通常情况下, 预取次数为 pcp->batch(默认为 31, 在 x86_64 上的上限为 (PAGE_SHIFT * 8) = 96), 但当 PCP 的页面全部耗尽时, 预取次数为 pcp->count, 上限为 pcp->high. pcp->high, 虽然有一个默认值 186 (pcp->batch = 31 * 6), 可以由用户通过 `/proc/sys/vm/percpu_pagelist_fraction` 更改, 并且没有软件上限, 所以可以很大, 比如几千. 因此, 只预取页面 buddy 结构前 pcp->batch 张页面, 避免预取过多.

**关于预取带来的收益和引入的开销**, 有两个问题:

1. 预取可能会移除现有的 cacheline, 特别是对于 L1D 缓存, 因为它不是很大.

2. 还有一些额外的指令开销, 即计算 BUDDY PFN 两次.

对于问题 1, 这很难说, 虽然通过性能测试显示了很好的结果, 但是其实实际的好处依旧取决于当前 CPU 上实际的工作负载.

对于问题 2, 因为计算是对两个局部变量的异或, 所以在许多情况下, 预期所花费的周期将会被后来减少的内存延迟所抵消. 这对于 NUMA 机器来说尤其如此, 其中多个 CPU 在 zone->lock 上竞争, zone->lock 锁下最耗时的部分是等待被释放的页面及其同伴的 "struct page" 的 cacheline.

*   Scale batch-free 的页面数量(free_factor)

v5.14 的时候, [commit 3b12e7e97938 ("mm/page_alloc: scale the number of pages that are batch freed")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3b12e7e97938424de2bb1b95ba0bd6a49bad39f9) 当任务释放大量 order-0 页面时, 可能会多次获取 zone->lock, 批量释放页面. 当释放大量页面时, 这可能会不必要地争夺 zone->lock. 这个补丁根据最近的模式调整 PCP 页面批量释放(batch-free)的页面大小, 提高了扩展性.

具体调整(Scale)的策略如下:

1.  每次在没有任何分配的情况下释放页面时, 将[释放的页面数增加一倍](https://elixir.bootlin.com/linux/v5.14/source/mm/page_alloc.c#L3358).

2.  在 rmqueue_pcplist() 进行分配时, 将批量释放的页面数[减少一半](https://elixir.bootlin.com/linux/v5.14/source/mm/page_alloc.c#L3673).

3.  释放时至少[保证有 pcp->batch 张页面](https://elixir.bootlin.com/linux/v5.14/source/mm/page_alloc.c#L3350)在 PCP list 上.

*   High-Order PCP 时代的 round-robin fashion 与 batch-free

依旧是 v5.14, [commit 44042b449872 ("mm/page_alloc: allow high-order pages to be stored on the per-cpu lists")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=44042b4498728f4376e84bae1ac8016d146d850b) 实现 High-Order PCP 页面的时候, 同样采用了 round-robin fashion 的 batch-free, 只不过当前会不光要保持 MIGRATE_PCPTYPES 的平衡, 还有考虑不同 Order 页面的平衡.

*    回退到  single pass zone->lock 与 prefetch_buddy

v5.18 的时候, free_pcppages_bulk() 又做了较多的优化.

这个时候, 从 PCP 列表中选择页面的操作已经足够简单, 选择期间的主要开销是 bulkfree_pcp_prepare(), 在通常情况下, 它是一个简单的检查和预取. 考虑到 list 操作本身有成本, 所以回到单次释放页面的流程. 参见 [commit 8b10b465d0e1 ("mm/page_alloc: free pages in a single pass during bulk free")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8b10b465d0e18b002b290b2162145abc7167e53d).

这个过程中还有一个关键的改动是移除了 prefetch_buddy(). 因为由于现在所有的遍历和释放[现在都是在 zone->lock 下进行的](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8b10b465d0e18b002b290b2162145abc7167e53d), 而预取当年也是伴随着将[页面选择放在 zone->lock 外, zone->lock 之保护页面释放所引入的]((https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0a5f4e5b45625e75db85b4968fc4c232d8091143)), 因此不太清楚预取这是否总能带来好处, 直接回退了 [prefetch_buddy() 的 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=97334162e4d79f866edd7308aac0ab3ab7a103f7) 特性. 参见 [commit 2a791f4412cb ("mm/page_alloc: do not prefetch buddies during bulk free")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2a791f4412cba41330453527a3045cf39818e72a).


|  时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:---:|:---:|:----------:|:----:|
| 2009/08/28 | Mel Gorman <mel@csn.ul.ie> | [Reduce searching in the page allocator fast-path](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a6f9edd65beaef24836e8934c8912c1e974dd45c) | [COMMIT1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=5f8dcc21211a3d4e3a7a5ca366b469fb88117f61) 新增了 PCP 对 MIGRATETYPE 的感知, pcp->list 按照 MIGRATETYPE 拆分成了多个 list, 这优化了每次查找对应 MIGRATTYPE 的开销(之前需要遍历 pcp->list 去找, 现在直接从对应 list[MIGRATETYPE] 中去取即可). 同样批量释放页面的时候, 为了保证公平性, 这个补丁为 free_pcppages_bulk() 实现了一个 [round-robin fashion](https://elixir.bootlin.com/linux/v2.6.32/source/mm/page_alloc.c#L545) 的模式, 循环着为每种 MIGRATETYPE 依次释放页面. | v1 ☑✓ 2.6.32-rc1 | [LORE RFC,0/3](https://lore.kernel.org/lkml/1250594162-17322-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE 0/3](https://lore.kernel.org/lkml/1251449067-3109-1-git-send-email-mel@csn.ul.ie) |
| 2010/09/09 | Mel Gorman <mel@csn.ul.ie> | [mm: page allocator: update free page counters after pages are placed](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=72853e2991a2702ae93aaf889ac7db743a415dd3) | [to_free 被删除](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=72853e2991a2702ae93aaf889ac7db743a415dd3e5b31ac2ca2cd0cf6bf2fcbb708ed01466c89aaa) | v1 ☑✓ 2.6.36-rc4 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=72853e2991a2702ae93aaf889ac7db743a415dd3) |
| 2011/03/22 | Namhyung Kim <namhyung@gmail.com> | [mm: batch-free pcp list if possible](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1d16871d8c96deadc5f9753b6b096074f2cbcbe1) | TODO | v1 ☑✓ 2.6.39-rc1 | [LORE](https://lore.kernel.org/lkml/1297257677-12287-1-git-send-email-namhyung@gmail.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1d16871d8c96deadc5f9753b6b096074f2cbcbe1) |
| 2018/03/01 | Aaron Lu <aaron.lu@intel.com> | [mm: improve zone->lock scalability](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=97334162e4d79f866edd7308aac0ab3ab7a103f7) | 优化了 free_pcppages_bulk() 中对 zone->lock 的持锁. | v4 ☑✓ 4.17-rc1 | [LORE v4,0/3](https://lore.kernel.org/all/20180301062845.26038-1-aaron.lu@intel.com) |
| 2021/05/25 | Mel Gorman <mgorman@techsingularity.net> | [mm/page_alloc: scale the number of pages that are batch freed](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3b12e7e97938424de2bb1b95ba0bd6a49bad39f9) | [Calculate pcp->high based on zone sizes and active CPUs](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=74f44822097c665041010994502b5971d6cd9f04) 的其中一个补丁. 整个补丁集 pcp->high 和 pcp->batch 根据 zone 内内存的大小进行调整. 当前补丁对 free_pcppages_bulk() 的页面数量做了 scale. 当任务释放大量 order-0 页面时, 可能会多次获取 zone->lock, 批量释放页面. 当释放大量页面时, 这可能会不必要地争夺 zone->lock. 这个补丁根据最近的模式调整 PCP 页面批量释放(batch-free)的页面大小, 从而提高了扩展性. | v2 ☑✓ 5.14-rc1 | [LORE v1,0/6](https://lore.kernel.org/all/20210521102826.28552-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/6](https://lore.kernel.org/lkml/20210525080119.5455-1-mgorman@techsingularity.net) |
| 2021/06/11 | Mel Gorman <mgorman@techsingularity.net> | [mm/page_alloc: allow high-order pages to be stored on the per-cpu lists](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=44042b4498728f4376e84bae1ac8016d146d850b) | 20210611135753.GC30378@techsingularity.net | v2 ☑✓ 5.14-rc1 | [LORE](https://lore.kernel.org/all/20210611135753.GC30378@techsingularity.net) |
| 2022/02/16 | Mel Gorman <mgorman@techsingularity.net> | [Follow-up on high-order PCP caching](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=2a791f4412cba41330453527a3045cf39818e72a) | commit 44042b449872 ("mm/page_alloc: allow high-order pages to storage on the per-cpu list") 的主要目的是通过两种方式降低高阶页面的 SLUB 缓存重新填充的成本. 首先, 区域锁获取减少, 其次, 好友列表修改减少. 这是一个后续系列, 修复了合并后出现的一些问题.<br>补丁 1 是一个功能补丁. 这是无害的, 但效率低下.<br>补丁 2-4 减少了大量释放 PCP 页面的开销. 虽然开销很小, 但在截断大文件时, 它是累积的, 并且是可以注意到的.<br>它可以删除带有页面缓存中的数据的大型稀疏文件, 稀疏文件用于消除文件系统开销.<br>补丁 5 解决了高阶 PCP 页面在 PCP 列表中存储时间过长的问题. CPU 上释放的页面可能无法快速重用, 在某些情况下, 这可能会增加缓存未命中率. 详细信息包含在变更日志中. | v1 ☐☑ 5.18-rc1 | [LORE v1,0/5](https://lore.kernel.org/r/20220215145111.27082-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/6](https://lore.kernel.org/r/20220217002227.5739-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE 1/1](https://patchwork.kernel.org/project/linux-mm/patch/20220221094119.15282-2-mgorman@techsingularity.net) |

*   free_pcppages_bulk Caller


[commit e7c8d5c9955a ("hot-n-cold pages: page allocator core")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a206231bbe6ffb988cdf9fcbdfd98e49abaf4819) 引入 PCP 框架的时候


1.  首先是 free_unref_page_list() 和 free_unref_page(), 早期分别是 free_hot_cold_page() 和 free_hot_cold_page_list(). 内核的 PCP 早已没有了当初 HOT COLD 的概念. 因此叫 hot_cold 已经不那么恰当了. 参见 [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=2d4894b5d2ae0fe1725ea7abd57b33bfbbe45492).

free_the_page() 是一个非常底层的页面释放回收, 用于释放任意 Order 的页面到 PCP 或者伙伴系统中. 如果可以使用 PCP, 则直接使用 free_unref_page() 将页面交给 PCP list. 否则就使用 `__free_pages_ok()` 将其归还给伙伴系统. 参见 [commit 742aa7fb52c5 ("mm/page_alloc.c: use a single function to free page")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=742aa7fb52c56fb3b307e704f93e67b698959cc2).

`put_page() -=> __put_single_page()` 是 `free_unref_page()` 的另一个用户, 最早引入冷热页的时候, 就使用 [`__page_cache_release()` 来释放](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a206231bbe6ffb988cdf9fcbdfd98e49abaf4819) Page Cache 的页面到 PCP 中. 随后引入 THP 的时候, 将 `free_unref_page()` 从 `__page_cache_release()` 中移除, 而封装了 [`__put_single_page()`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9180706344487700b40da9eca5dedd3d11cb33b4) 来完成页面的释放, 执行路径 `put_page()` -=> `__put_page()` -=> `__put_single_page()` -=> `free_unref_page()`.

[free_unref_page_list()(早期叫 free_hot_cold_page_list())](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=cc59850ef940e4ee6a765d28b439b9bafe07cf63) 用于释放一整个 list 的页面. 这也是一个非常广泛的页面释放接口. LRU 或者 PCP 释放页面的过程中, 为了将遍历查询页面和释放页面的操作分开, 减少持锁的时间. 普遍采用的方式是将页面加入到一个临时的 list 中, 然后再通过 free_unref_page_list() 释放整个 list 中所有的页面. 由于所有的页面都是 Order-0 的, 因此每张页面依旧是通过 free_unref_page_prepare() 和 free_unref_page_commit() 来归还给 PCP 的.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/10/18 | Mel Gorman <mgorman@techsingularity.net> | [mm, page_alloc: enable/disable IRQs once when freeing a list of pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=453f85d43fa9ee243f0fc3ac4e1be45615301e3f) | [Follow-up for speed up page cache truncation v2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=453f85d43fa9ee243f0fc3ac4e1be45615301e3f) 的其中一个补丁. 将原来的 free_hot_cold_page() 拆成了 free_unref_page_prepare() 和 free_unref_page_commit(). | v2 ☑ 4.15-rc1 | [LORE v2,1/8](https://lore.kernel.org/all/20171018075952.10627-2-mgorman@techsingularity.net), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9cca35d42eb61b69e108a17215756c46173a5e6f) |
| 2018/11/19 | Aaron Lu <aaron.lu@intel.com> | [free order-0 pages through PCP in page_frag_free() and cleanup](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=742aa7fb52c56fb3b307e704f93e67b698959cc2) | page_frag_free() 支持释放页面到 PCP 中. page_frag_free() 是用来释放任意 Order 的普通页或者复合页. page_frag_free() 调用 `__free_pages_ok()` 将页面释放回 Buddy, 但是却不考虑使用 PCP. 这对于高阶页面是可以的, 但是对于 Order-0 的页面, 它错过了使用 PCP 的优化机会, 如果频繁调用时可能导致 zone->lock 争用.<br>Pawel Staszewski 最近分享了他的 "Linux 内核如何处理正常流量" 的结果 [1] 和从性能数据, Jesper Dangaard Brouer 发现锁争用来自页面分配器. 通过在 page_frag_free() 中增加对 PCP 页面的支持. Pawel 和 Jesper 的场景性能提高了 7%, 锁争用消失了. 此外 Ilias 在 cortex-a53 上的 "低" 速度 1Gbit 接口上的测试显示 ~11% 的性能提升. 测试 64 字节数据包 `__free_pages_ok()` 函数的不再是 perf top 的热点.<br>函数接口上, 之前释放 PCP 页面的路径和函数太多了, 这里引入了一个统一的接口 free_the_page() 来释放 PCP 页面. 其中对于满足 PCP 要求的页面, 直接使用 free_unref_page() 回收到了 PCP list 中. | v1 ☑✓ 5.0-rc1 | [LORE v1,0/2](https://lore.kernel.org/all/20181119134834.17765-1-aaron.lu@intel.com) |


2.  接着是 drain_pages_zone(cpu, zone), 该函数用于回收指定 CPU 指定 zone 上的 PCP list 页面.

很多情况下, 直接 Drain 掉一个 CPU 上所有 zone 的 PCP 页面相对来说是非常不合适的, 因此 v3.19 实现了单个 zone 的 pcplist Draining, 引入了 drain_pages_zone(cpu, zone) 来清理掉指定 CPU 上指定 drain 的 page 页面. 参见 [commit 93481ff0e5a0 ("mm: introduce single zone pcplists drain")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=93481ff0e5a0c7636359a7ee52248856da5e7859).

内存规整时, 3.19-rc1 时, compact_zone() 中添加 check_drain 标签, 如果页面分配慢速路径下进行直接规整, 并且当前区域已经完成了页面规整, 则通过 drain_local_pages(zone)(后面修正为 lru_add_drain_cpu_zone()) 将当前 zone 的 PCP 页面释放掉, 从而让伙伴系统合并出足够的连续页面块, 来完成当前高阶页面的分配, 参见 [COMMIT1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=fdaf7f5c40f3d20690c236298418acf72eb664b5), [COMMIT2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b01b2141999936ac3e4746b7f76c0f204ae4b445). 随后 v4.17-rc1 时, 进一步做了优化. 当 kcompactd 无法规整出足够的连续内存块来完成 cc.order 的页面分配时, 会将该 zone 的所有 pcps 页面通过 [drain_all_pages(zone)](https://elixir.bootlin.com/linux/v4.17/source/mm/compaction.c#L1996) 归还到伙伴系统, 这提高了高阶页面分配成功的概率. 参见 [COMMIT3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bc3106b26cf6a6f214fd1a8538736afc39ae1b5c),

页面隔离时, 在 pageblock 上设置 MIGRATETYPE_ISOLATE 时, 也会把当前 zone 的 pcplists 清空, 以便有更好的机会将所有页面成功隔离, 而不是留在 PCP 缓存中. 由于隔离始终与单个 zone 有关, 所以可以将 pcplists 的 drain 减少到单个区域. 这一变化将使内存隔离更快, 并且不影响其他不相关的 pcplists. 参见 [COMMIT1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ec25af84b23b6862341b5b5b68d24be3f53b8d2c), [COMMIT2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7612921f2376d51d020ae2f06ffb7da40422b75b).

CPU offline 时, 同样需要释放当前 CPU 上 PCP 缓存的页面. [2014/12/10, COMMIT1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c05543293e0bf586842844c14fd8c598f494a107), [2019/03/05, COMMIT2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c52e75935f8ded2bd4a75eb08e914bd96802725b), [2020/09/18, COMMIT3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9683182612214aa5f5e709fad49444b847cd866a), [2020/12/14, COMMIT4](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ec6e8c7e03147c65380e6c04c4cf4290e96280b6)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2014/10/02 | Vlastimil Babka <vbabka@suse.cz> | [Single zone pcpclists drain](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c05543293e0bf586842844c14fd8c598f494a107) | 支持单个 zone 的 pcplist Draining. 在很多情况下, 只 drain 一个 zone 的 pcplist 就足够了, drain 掉所有 zone 的 pcplist 是浪费, 然后会导致更多的 pcplist 重新填充.<br> [COMMIT1] 为 drain_local_pages() 和 drain_all_pages() 引入了 "struct zone *" 参数, 传入 NULL 值意味着所有 zone 都像往常一样被 drain. 剩余的补丁在适当的地方将现有的调用者转换为单一 zone Draining 的操作. 注意内存规整上单个 zone 的 pcplist Draining, 将在下一个补丁集 [Further compaction tuning](https://lore.kernel.org/all/1412696019-21761-1-git-send-email-vbabka@suse.cz) | 1412696019-21761-1-git-send-email-vbabka@suse.cz) 提供. | v1 ☑✓ 3.19-rc1 | [LORE v1,0/4](https://lore.kernel.org/all/1412264940-15738-1-git-send-email-vbabka@suse.cz) |
| 2014/10/07 | Vlastimil Babka <vbabka@suse.cz> | [Further compaction tuning](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fdaf7f5c40f3d20690c236298418acf72eb664b5) | NA | v1 ☑✓ 3.19-rc1 | [LORE v1,0/5](https://lore.kernel.org/all/1412696019-21761-1-git-send-email-vbabka@suse.cz) |
| 2015/12/03 | Vlastimil Babka <vbabka@suse.cz> | [mm, compaction: reduce spurious pcplist drains](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a34753d275576896b06af9baa6f54bee258368c2) | [reduce latency of direct async compaction](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fdd048e12c9a46d058f69822cb15641adae181e1) 的其中一个补丁. | v1 ☑✓ 4.7-rc1 | [LORE v1,0/3](https://lore.kernel.org/all/1449130247-8040-1-git-send-email-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/4](https://lore.kernel.org/lkml/1459414236-9219-1-git-send-email-vbabka@suse.cz), [关注 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a34753d275576896b06af9baa6f54bee258368c2) |
| 2018/03/01 | David Rientjes <rientjes@google.com> | [mm, compaction: drain pcps for zone when kcompactd fails](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bc3106b26cf6a6f214fd1a8538736afc39ae1b5c) | 内存规整通过内存的碎片化整理在 zone 内规整出一块连续的空闲页面, 从而满足高阶页面(甚至是 MAX_ORDER) 的分配请求, 但是 PCP 对此是个冲击, 空闲页面可能会滞留在 PCP 上, 从而组织当前 zone 上的伙伴页面合并. 当 kcompactd 无法整理内存以便分配 cc.order 页面时, 将该区域的所有 pcps 页面通过 [drain_all_pages(zone)](https://elixir.bootlin.com/linux/v4.17/source/mm/compaction.c#L1996) 归还到伙伴系统, 这样就不会发生这种搁浅. 该顺序的内存规整随后将被推迟, 从而限制 Draining 的速率. 测试发现, 没有这个补丁, 就很难有 order-9 或 order-10 页面空闲. | v1 ☑✓ 4.17-rc1 | [LORE](https://lore.kernel.org/all/alpine.DEB.2.20.1803010340100.88270@chino.kir.corp.google.com) |


*   最后是 drain_zone_pages

在大型 NUMA 系统上, PCP 中可能缓存了大量内存. 在一个拥有 512 个处理器和 256 个节点的系统上, 将会有 `256*512` 个页面集. 如果每个页面集只能保存 5 个页面, 那么我们讨论的是 655360 个页面. 在 IA64 上, 如果页面大小为 16K, 这可能会导致 10 GB 的内存被困在 PCP 中. 对于较小的系统来说, 典型的情况要少得多, 但仍然存在内存被困在非节点页面集中的可能性.

v2.6.13-rc1, [commit 4ae7c03943fc ("Periodically drain non local pagesets")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4ae7c03943fca73f23bc0cdb938070f41b98101f) 引入了 PCP 页面的周期性回收. 通过将 Drain PCP 的操作添加到 SLAB 的刷新过程中, cache_reap() -=> drain_remote_pages(). SLAB 分配器每 2 秒刷新一次它的 PCP 缓存. 如果本地内存可用, offline 的节点内存可能很少使用. 没有这个补丁, 类似这种很少使用的页面集中依旧会被留有较多的内存.

随后 v2.6.16-rc6, `next_reap_node() -=> drain_node_pages()`, 参见 [COMMIT1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8fce4d8e3b9e3cf47cc8afeb6077e22ab795d989), [COMMIT2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=879336c3930ae9273ea1c45214cb8adae0ce494a), [COMMIT3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bc4ba393c007248f76c05945abb7b7b892cdd1cc).

最后, 将 drain_node_pages() 从 SLAB cache_reap() 流程中移除, 转而转换为 drain_zone_pages(), 并加入到 refresh_cpu_vm_stats() 流程中. 参见 [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4037d452202e34214e8a939fa5621b2b3bbb45b7).


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2006/06/12 | Christoph Lameter <clameter@sgi.com> | [Zoned VM counters V3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ed11d9eb2228acc483c819ab353e3c41bcb158fa) | NA | v3 ☑✓ 2.6.18-rc1 | [LORE v1,0/21](https://lore.kernel.org/all/20060612211244.20862.41106.sendpatchset@schroedinger.engr.sgi.com) |


#### 2.2.5.4 [Bulk memory allocation](https://lwn.net/Articles/711075)
-------

*   Order-0 批量页面分配器

由于存储设备和网络接口等外围设备可以处理不断增加的数据速率, 因此内核面临许多可扩展性挑战. 通常, 提高吞吐量的关键是分批工作. 在许多情况下, 执行一系列相关操作的开销不会比执行单个操作的开销高很多. 内存分配是批处理可以显着提高性能的地方, 但是到目前为止, 关于如何进行批处理社区进行了多次激烈的讨论.

举例来说, 网络接口往往需要大量内存. 毕竟, 所有这些传入的数据包都必须放在某个地方. 但是分配该内存的开销很高, 以至于它可能会限制整个系统的最大吞吐量. 之前网络驱动开发人员的做法都是采用诸如先分配(大 order 的高阶内存后)再拆分(成小 order 的低阶内存块) 的折衷方案. 但是高阶页面分配不仅分配速度慢, 开销大, 可能还会给整个系统带来压力. 参见 [Generic page-pool recycle facility ?](https://people.netfilter.org/hawk/presentations/MM-summit2016/generic_page_pool_mm_summit2016.pdf)

在 2016 年 [Linux 存储, 文件系统和内存管理峰会 (Linux Storage, Filesystem, and Memory-Management Summit)](https://lwn.net/Articles/lsfmm2016) 上, 网络开发人员 Jesper Dangaard Brouer 提议创建一个新的内存分配器, [LWN: Bulk memory-allocation APIs](https://lwn.net/Articles/684616), 该分配器从一开始就设计用于批处理操作. 驱动程序可以使用它在一个调用中分配许多页面, 从而最大程度地减少了每页的开销. 在这次会议上, 内存管理开发人员 Mel Gorman 了解了此问题, 但不同意他创建新分配器的想法. 因为这样做会降低内存管理子系统的可维护性. 另外, 随着新的分配器功能的不断完善, 新的分配器遇到现有分配器同样的问题, 比如 NUMA 上的一些处理, 并且当它想要完成具有所有必需的功能时, 它并不见得完成的更快. 参见 [LWN: Bulk memory allocation without a new allocator](https://lwn.net/Articles/711075).

*   首次尝试

Mel Gorman 认为最好基于现有的 Per CPU Allocator/PCP 针对这种情况进行优化. 那么内核中的所有用户都会受益. 他实现了 [Fast noirq bulk page allocator, RFC,0/4](https://lore.kernel.org/lkml/20170104111049.15501-1-mgorman@techsingularity.net), 在特性的场景下, 它可以将页面分配器的开销减半, 且用户不必关心 NUMA 结构.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/01/09 | Mel Gorman | [Fast noirq bulk page allocator](https://lore.kernel.org/patchwork/patch/747351) | 中断安全的批量内存分配器, RFC 补丁, 最终 Mel Gorman 为了完成这组优化做了大量的重构和准备工作. | v5 ☐ | [2017/01/04 LORE RFC,0/4](https://lore.kernel.org/lkml/20170104111049.15501-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[2017/01/09 LORE RFC,v2r7](https://lore.kernel.org/lkml/20170109163518.6001-1-mgorman@techsingularity.net) |
| 2017/01/23 | Mel Gorman | [Use per-cpu allocator for !irq requests and prepare for a bulk allocator v5](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=374ad05ab64d696303cec5cc8ec3a65d457b7b1c) | 重构了 Per CPU Allocation(PCP), 只有 IRQ 安全分配请求能够使用 Per CPU Allocation(PCP), 这将减少大约 30% 的分配/释放开销. 这是完成 Bulk memory allocation 工作的第一步. 许多分配页面的工作负载一次都不会处理中断. 由于分配请求可能来自 IRQ 上下文, 因此页面分配和释放时需要频繁的开关中断, 比如页面分配 buffered_rmqueue() 时[禁用/启用 IRQ](https://elixir.bootlin.com/linux/v4.10/source/mm/page_alloc.c#L2622), 页面释放时 Order-0 页面 [free_hot_cold_page()](https://elixir.bootlin.com/linux/v4.10/source/mm/page_alloc.c#L2456) 以及其他 Order 页面释放 [`__free_pages_ok()`](https://elixir.bootlin.com/linux/v4.10/source/mm/page_alloc.c#L1247) 中. 这一成本占释放路径的大部分, 但也占分配路径的很大一部分. 对性能的影响和开销比较大. 这组补丁先进行了一些清理操作<br>1. [commit1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=066b23935578d3913c2df9bed7addbcdf4711f1a) 从 buffered_rmqueue() 中拆解出 rmqueue_pcplist() 用于从 PCP 中分配, 并将 buffered_rmqueue() 重命名为 rmqueue().<br>2. [commit2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9cd7555875bb09dad875e89a76f41f576e11c638) 从 `__alloc_pages_nodemask()` 中拆解出 prepare_alloc_pages(), 因为这个准备工作会在批处理分配中多次调用<br>3. [commit3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0ccce3b924212e121503619df97cc0f17189b77b) 将 drain_local_pages_wq() 将 drain_local_pages() 放到 WORK 中去完成.<br>接着最重要的改动是改变了锁定和检查, 使得只有 IRQ 安全分配请求使用 Per CPU Allocation(PCP). 所有其他的分配请求从伙伴系统中器获得 irq 安全区 -> 锁定和分配. 它依靠禁用抢占来安全访问 PCP 结构. 这种修改可能会略微降低 IRQ 上下文中的分配速度, 但 Per CPU Allocation(PCP) 的主要好处是, 它可以更好地扩展来自多个上下文的分配. 有一个隐含的假设, 即从 IRQ 上下文在单个 NUMA NODE 的多个 CPU 上进行密集地分配是非常罕见的, 而且大多数扩展问题都是在 !IRQ 上下文, 例如 Page Fault 等. 其实批量页面分配器不需要此修补程序, 但它显著降低了开销.<br>不过这个最关键的 [commit d34b0733b452 ("Revert "mm, page_alloc: only use per-cpu allocator for irq-safe requests")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d34b0733b452ca3ef1dee36436dab08b8aa6a85c) 很快被 revert. | v5 ☑ 4.11-rc1 | [LORE 0/3](https://lore.kernel.org/all/20170112104300.24345-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/4](https://lore.kernel.org/all/20170117092954.15413-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/4](https://lore.kernel.org/all/20170123153906.3122-1-mgorman@techsingularity.net) |

*   再次尝试

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/03/25 | Mel Gorman | [Introduce a bulk order-0 page allocator with two in-tree users](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=be5dba25b4b27f262626ddc9079d4858a75462fd) | 批量 Order-0 页面分配器, 目前 sunrpc 和 network 页面池是这个特性的第一个用户 | v6 ☑ 5.13-rc1 | [RFC](https://lore.kernel.org/patchwork/patch/1383906)<br>*-*-*-*-*-*-*-* <br>[v1](https://lore.kernel.org/patchwork/patch/1385629)<br>*-*-*-*-*-*-*-* <br>[v2](https://lore.kernel.org/patchwork/patch/1392670)<br>*-*-*-*-*-*-*-* <br>[v3](https://lore.kernel.org/patchwork/patch/1393519)<br>*-*-*-*-*-*-*-* <br>[v4](https://lore.kernel.org/patchwork/patch/1394347)<br>*-*-*-*-*-*-*-* <br>[v5](https://lore.kernel.org/patchwork/patch/1399888)<br>*-*-*-*-*-*-*-* <br>[LORE v6,0/9](https://lore.kernel.org/all/20210325114228.27719-1-mgorman@techsingularity.net) |
| 2021/03/29 | Mel Gorman | [Use local_lock for pcp protection and reduce stat overhead](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=902499937e3a82156dcb5069b6df27640480e204) | Bulk memory allocation 的第一组修复补丁, PCP 与 vmstat 共享锁定要求, 这很不方便, 并且会导致一些问题. 可能因为这个原因, PCP 链表和 vmstat 共享相同的 Per CPU 空间, 这意味着 vmstat 可能跨 CPU 更新包含 Per CPU 列表的脏缓存行, 除非使用填充. 该补丁集拆分该结构并分离了锁. | v6 ☑ 5.14-rc1 | [LORE RFC,0/6](https://lore.kernel.org/lkml/20210329120648.19040-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v2,00/11](https://lore.kernel.org/lkml/20210407202423.16022-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v3,00/11](https://lore.kernel.org/lkml/20210414133931.4555-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v4,00/10](https://lore.kernel.org/lkml/20210419141341.26047-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v6,0/9](https://lore.kernel.org/lkml/20210512095458.30632-1-mgorman@techsingularity.net) |
| 2021/03/30 | Mel Gorman | [mm/page_alloc: Add a bulk page allocator -fix -fix](https://lore.kernel.org/patchwork/patch/1405057) | Bulk memory allocation 的第二组修复补丁 | v6 ☐ | [LORE](https://lore.kernel.org/lkml/20210330114847.GX3697@techsingularity.net) |
| 2021/07/16 | Mel Gorman | [mm/page_alloc: enable alloc bulk when page owner is on](https://lore.kernel.org/lkml/20210716081756.25419-1-link@vivo.com/) | 上一个 alloc bulk 版本有一个bug, 当 page_owner 打开时, 系统可能会由于 irq 禁用上下文中的 alloc bulk 调用 prep_new_page() 而崩溃, 这个问题是由于  set_page_owner() 在 local_irq 关闭的情况下通过 GFP_KERNEL 标志分配内存来保存栈信息导致的. 所以, 我们不能假设 alloc 标志应该与 new page 相同, prep_new_page() 应该准备/跟踪页面 gfp, 但不应该使用相同的gfp来获取内存, 这取决于调用方. 现在, 这里有两个gfp标志, alloc_gfp 用于分配内存, 取决于调用方, page_gfp 是 page 的 gfp, 用于跟踪/准备自身. 在大多数情况下, 两个 flag 相同是可以的, 在 alloc_pages_bulk() 中, 使用 GFP_ATOMIC, 因为 irq 被禁用. | v1 ☐ | [RFC](https://lore.kernel.org/lkml/20210716081756.25419-1-link@vivo.com/) |



#### 2.2.5.4 High Order PCP
-------

很长一段时间以来, SLUB 一直是默认的小型内核对象分配器, 但由于性能问题和对高阶页面的依赖, 它并不是普遍使用的. 高阶关注点有两个主要组件——高阶页面并不总是可用, 高阶页面分配可能会在 zone->lock 上发生冲突.

[mm: page_alloc: High-order per-cpu page allocator v4](https://lore.kernel.org/patchwork/patch/740275) 通过扩展 Per CPU Pages(PCP) 分配器来缓存高阶页面, 解决了一些关于区域锁争用的问题. 这个补丁做了以下修改

1.  添加了新的 Per CPU 列表来缓存高阶页面. 这会增加 Per CPU Allocation 的缓存空间和总体使用量, 但对于某些工作负载, 这将通过减少 zone->lock 上的争用而抵消. 列表中的第一个 MIGRATE_PCPTYPE 条目是每个 migratetype 的. 剩余的是高阶缓存, 直到并包括 PAGE_ALLOC_COSTLY_ORDER. 页面释放时, PCP 的计算现在被限制为 free_pcppages_bulk, 因为调用者不可能知道到底释放了多少页. 由于使用了高阶缓存, 请求耗尽的页面数量不再精确.

2.  增加 Per CPU Pages 的高水位, 以减少一次重新填充导致下一个空闲页面的溢出的概率. 这个改动的优化效果跟硬件环境和工作负载有较大的关系, 取决定因素的是当前有业务在 zone->lock 上的反弹和争用是否占主导.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/12/01 | Mel Gorman | [mm: page_alloc: High-order per-cpu page allocator v4](https://lore.kernel.org/patchwork/patch/740275) | 为高阶内存分配提供 Per CPU Pages 缓存  | v4 ☐ | [LORE v4](https://lore.kernel.org/lkml/20161201002440.5231-1-mgorman@techsingularity.net) |
| 2020/08/14 | Minchan Kim <minchan@kernel.org> | [Support high-order page bulk allocation](https://lore.kernel.org/all/20200814173131.2803002-1-minchan@kernel.org) | 20200814173131.2803002-1-minchan@kernel.org | v1 ☐☑✓ | [LORE RFC,0/7](https://lore.kernel.org/all/20200814173131.2803002-1-minchan@kernel.org) |
| 2021/06/03 | Mel Gorman <mgorman@techsingularity.net> | [Allow high order pages to be stored on PCP v2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=44042b4498728f4376e84bae1ac8016d146d850b) | PCP 支持缓存高 order 的页面. | v2 ☑ 5.14-rc1 | [OLD v6](https://lore.kernel.org/patchwork/patch/740779)<br>*-*-*-*-*-*-*-* <br>[OLD v7](https://lore.kernel.org/patchwork/patch/741937)<br>*-*-*-*-*-*-*-* <br>[LORE v2](https://lore.kernel.org/lkml/20210603142220.10851-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v2](https://lore.kernel.org/all/20210611135753.GC30378@techsingularity.net) |
| 2022/02/16 | Mel Gorman <mgorman@techsingularity.net> | [Follow-up on high-order PCP caching](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=2a791f4412cba41330453527a3045cf39818e72a) | commit 44042b449872 ("mm/page_alloc: allow high-order pages to storage on the per-cpu list") 的主要目的是通过两种方式降低高阶页面的 SLUB 缓存重新填充的成本. 首先, 区域锁获取减少, 其次, 好友列表修改减少. 这是一个后续系列, 修复了合并后出现的一些问题.<br>补丁 1 是一个功能补丁. 这是无害的, 但效率低下.<br>补丁 2-4 减少了大量释放 PCP 页面的开销. 虽然开销很小, 但在截断大文件时, 它是累积的, 并且是可以注意到的.<br>它可以删除带有页面缓存中的数据的大型稀疏文件, 稀疏文件用于消除文件系统开销.<br>补丁 5 解决了高阶 PCP 页面在 PCP 列表中存储时间过长的问题. CPU 上释放的页面可能无法快速重用, 在某些情况下, 这可能会增加缓存未命中率. 详细信息包含在变更日志中. | v1 ☐☑ | [LORE v1,0/5](https://lore.kernel.org/r/20220215145111.27082-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/6](https://lore.kernel.org/r/20220217002227.5739-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE 1/1](https://patchwork.kernel.org/project/linux-mm/patch/20220221094119.15282-2-mgorman@techsingularity.net) |
| 2022/03/10 | Mel Gorman <mgorman@techsingularity.net> | [mm/page_alloc: check high-order pages for corruption during PCP operations](https://patchwork.kernel.org/project/linux-mm/patch/20220310092456.GJ15701@techsingularity.net) | Eric Dumazet 指出, [commit 44042b449872 ("mm/page_alloc: allow high-order pages to storage to the per-cpu list")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=44042b4498728f4376e84bae1ac8016d146d850b) 仅在 PCP 重新填充和分配操作期间检查首页. 这是一个疏忽, 所有页面都应该检查. 这将导致一个小的性能损失, 但这对正确性是必要的. | v1 ☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220310092456.GJ15701@techsingularity.net) |


#### 2.2.5.5 Adjust PCP high and batch
-------

PCP 页分配器旨在减少对区域锁的争用, 但是 PCP 中的页面数量也要有限制. 因此 PCP 引入了 [low, high 和 batch](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a206231bbe6ffb988cdf9fcbdfd98e49abaf4819) 来控制 pcp->list 中的页面大小. 当 PCP 中的页面[超过了 pcp->high 的时候](https://elixir.bootlin.com/linux/v2.6.15/source/mm/page_alloc.c#L700), 则会释放 batch 的页面回到 BUDDY 中. 同样当 PCP 中的页面数量[少于 pcp->low 的时候](https://elixir.bootlin.com/linux/v2.6.15/source/mm/page_alloc.c#L744), 则会尝试再从 BUDDY 中申请 batch 张页面到 PCP 中.

v2.6.16-rc1 时, 大家发现 pcp->low 水线貌似并没有什么太大的用处, 直接被干掉. 参见 [commit e7c8d5c9955a ("a206231bbe6f ("mm: remove pcp low")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=2d92c5c9150a2a9ca3dc25da58d5042e17a96b6a).

同样还是 v2.6.16 时, 内核通过 [commit 8ad4b1fb8205 ("Making high and batch sizes of per_cpu_pagelists configurable")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=8ad4b1fb8205340dba16b63467bb23efc27264d6) 引入了一个参数 vm.percpu_pagelist_fraction 用来设置 PCP->high 的大小. 而 PCP->batch 的大小将被设置为 min(high / 4, PAGE_SHIFT * 8).

时间来到 2021 年, batch 和 high 的大小已经过时了, 既不考虑 zone 大小, 也不考虑一个区域的本地节点上 CPU 的数量. PCP 的空间往往非常有限, 随着更大的 zone 和每个节点更多的 CPU, 将导致争用情况越来越糟, 特别是通过 `release_pages()` 批量释放页面时 zone->lock 争用尤为明显. 虽然可以使用 vm.percpu_pagelist_fraction 来增加 pcp->high 来减少争用, 但是由于 vm.percpu_pagelist_fraction 同时调整 high 和 batch 两个值, 虽然可以一定程度减少 zone->lock 锁的争用, 但也会增加分配延迟. Mel Gorman 发现了这一问题, v5.14 时开发了 [Calculate pcp->high based on zone sizes and active CPUs](https://lore.kernel.org/lkml/20210525080119.5455-1-mgorman@techsingularity.net) 将 pcp->high 和 pcp->batch 的设置分离, 然后根据 local zone 的大小扩展 pcp->high, 对活动 CPU 的回收和计算影响有限, 但 PCP->batch 保持静态. 它还根据最近的释放模式调整可以在 PCP 列表上的页面数量.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2005/12/09 | Rohit Seth <rohit.seth@intel.com> | [Making high and batch sizes of per_cpu_pagelists configurable](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=8ad4b1fb8205340dba16b63467bb23efc27264d6) | 引入了 percpu_pagelist_fraction 来调整各个 zone PCP 的 high, 同时将 batch 值设置为 min(high / 4, PAGE_SHIFT * 8).  | v1 ☑ 2.6.16-rc1 | [LORE](https://lore.kernel.org/patchwork/patch/47659), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8ad4b1fb8205340dba16b63467bb23efc27264d6) |
| 2017/01/25 | Mel Gorman | [Recalculate per-cpu page allocator batch and high limits after deferred meminit](https://lore.kernel.org/patchwork/patch/1141598) | 由于 PCP(Per CPU Page) Allocation 中不正确的高限制导致的高阶区域 zone->lock 的竞争, 在初始化阶段, 但是在初始化结束之前, PCP 分配器会计算分批分配/释放的页面数量, 以及 Per CPU 列表上允许的最大页面数量. 由于 zone->managed_pages 还不是最新的, pcp 初始化计算不适当的低批量和高值. 在某些情况下, 这会严重增加区域锁争用, 严重程度取决于共享一个本地区域的cpu数量和区域的大小. 这个问题导致了构建内核的时间比预期的长得多时, AMD epyc2 机器上的系统 CPU 消耗也过多. 这组补丁修复了这个问题 | v5 ☑ 4.11-rc1 | [LORE 0/3](https://lore.kernel.org/lkml/20191018105606.3249-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/3](https://lore.kernel.org/all/20191021094808.28824-1-mgorman@techsingularity.net), [commit1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3e8fc0075e24338b1117cdff6a79477427b8dbed) |
| 2021/05/25 | Mel Gorman <mgorman@techsingularity.net> | [Calculate pcp->high based on zone sizes and active CPUs](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=74f44822097c665041010994502b5971d6cd9f04) | pcp->high 和 pcp->batch 根据 zone 内内存的大小进行调整. 移除了不适用的 vm.percpu_pagelist_fraction 参数. | v2 ☑ 5.14-rc1 | [LORE v1,0/6](https://lore.kernel.org/all/20210521102826.28552-1-mgorman@techsingularity.net)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/6](https://lore.kernel.org/lkml/20210525080119.5455-1-mgorman@techsingularity.net) |


#### 2.2.5.7 PCP 分配器框架变更与优化时间线(总结)
-------

[commit e7c8d5c9955a ("hot-n-cold pages: page allocator core")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a206231bbe6ffb988cdf9fcbdfd98e49abaf4819) 引入了 PCP 最早的框架, 其中 per_cpu_pages 维护了 PCP 的基础结构, per_cpu_pageset 包含了 per_cpu_pages 的数组 pcp[2](0: hot, 1: cold`), 每个 zone 中都包含了 per_cpu_pageset 的数组 pageset[NR_CPUS], 维护了 zone 下所有 CPU 的 PCP 页面.

之前的版本每个 zone 都有 PCP 的页面集阵列 struct per_cpu_pageset pageset[NR_CPUS]. 这样 NUMA 系统下, 任何特定的 CPU 在每个 zone 结构中都有一些属于它自己的内存, 即使 CPU 不是该区域的本地 CPU. 因此 v2.6.13-rc1 实现了 NUMA 感知的 PCP, 修改了 NUMA 系统下 struct zone 中 PCP 页面集的管理方式.这个版本, NUMA 系统下, zone->pageset 成为一个数组指针 `*pageset[NR_CPUS]`, 然后使用 process_zones() 通过 kmalloc　来动态分配指定 CPU 的 zone->pageset[cpu] 空间. 其中 boot CPU(CPU 0) 的 PCP pageset 在 start_kernel()-=>setup_per_cpu_pageset() 中完成, 并注册 register_cpu_notifier(&pageset_notifier), 而非 boot CPU 的 PCP pageset, 就通过注册 pageset_notifier 的 CallBack 函数 pageset_cpuup_callback 来完成. 但是这样依旧有一些问题, 在 zone 初始化阶段 free_area_init_core() 中, SLAB 分配器还没有初始化, 因此在这个阶段先使用了静态 boot_pageset(曾经叫 pageset_table) 完成启动阶段的初始化. 这个静态变量被标记为 `__initdata`, 在系统启动后会其空间将被释放掉. 参见 [commit e7c8d5c9955a ("node local per-cpu-pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e7c8d5c9955a4d2e88e36b640563f5d6d5aba48a) 和 [Reduce size of huge boot per_cpu_pageset, 0/2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=2caaad41e4aa8f5dd999695b4ddeaa0e7f3912a4), [commit b7c84c6ada2b ("boot_pageset must not be freed")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=b7c84c6ada2be942eca6722edb2cfaad412cd5de)

v2.6.25-rc1, PCP 不再明确用 2 个数组区分冷热页, 而是使用单一列表管理. 参见 [Page allocator: get rid of the list of cold pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3dfa5721f12c3d5a441448086bee156887daa961), 此时 struct per_cpu_pageset 结构中 struct per_cpu_pages pcp[2] 变成 pcp.

v2.6.24-rc1, 引入 migratetype 来缓解内存碎片化的时候, [commit 535131e6925b ("Choose pages from the per-cpu list based on migration type")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=535131e6925b4a95f321148ad7293f496e0e58d7) 新增了 PCP 对 migratetype 的感知, 不过此时每次需要遍历 pcp->list 查找到对应 migratetype 的页面, 如果 PCP 中没有对应 migratetype 的页面, 则通过 rmqueue_bulk() 从 Buddy 中继续缓存一些出来. 这种遍历查找的方式效率较低. 因此 v2.6.32-rc1 时 [commit 5f8dcc21211a ("page-allocator: split per-cpu list into one-list-per-migrate-type")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=5f8dcc21211a3d4e3a7a5ca366b469fb88117f61), 将 PCP 的页面列表也按照 migratetype 分开管理. 不过并不是所有的 migratetype 都会缓存在 PCP 中. 因此使用 MIGRATE_PCPTYPES 管理, 至此 per_cpu_pages 中维护 PCP 页面的 list 演变为 lists[MIGRATE_PCPTYPES]．每次要从 PCP 获取某个 migratetype 的时候, 直接从对应 migratetype 的 pcp->list[migratetype] 去获取就行, 不用再费劲地区遍历 pcp->list, 归还的时候也按照 migratetype 归还.

v2.6.34-rc1, [commit 99dcc3e5a94e ("this_cpu: Page allocator conversion")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=99dcc3e5a94ed491fbef402831d8c0bbb267f995) 开始使用了 PER CPU 变量替代 NR_CPUS 的数组. 于是 boot_pageset 被定义为 DEFINE_PER_CPU, zone->pageset 变成了一个指针, 而是用 alloc_percpu() 来分配. 然后直接使用 per_cpu_ptr(zone->pageset, cpu) 访问指定的 PCP 页面集. 以及 [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=43cf38eb5cea91245502df3fcee4dbfc1c74dd1c).



|  时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:---:|:---:|:----------:|:----:|
| 2005/06/21 | Christoph Lameter <christoph@lameter.com> | [node local per-cpu-pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e7c8d5c9955a4d2e88e36b640563f5d6d5aba48a) | TODO | v1 ☑✓ v2.6.13-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e7c8d5c9955a4d2e88e36b640563f5d6d5aba48a) |
| 2009/08/28 | Mel Gorman <mel@csn.ul.ie> | [page-allocator: split per-cpu list into one-list-per-migrate-type](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5f8dcc21211a3d4e3a7a5ca366b469fb88117f61) | [Reduce searching in the page allocator fast-path](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a6f9edd65beaef24836e8934c8912c1e974dd45c) 的其中一个补丁. | v1 ☑✓ 2.6.32-rc1 | [LORE RFC,0/3](https://lore.kernel.org/lkml/1250594162-17322-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE 0/3](https://lore.kernel.org/lkml/1251449067-3109-1-git-send-email-mel@csn.ul.ie) |
| 2007/11/19 | clameter@sgi.com <clameter@sgi.com> | [this_cpu: Page allocator conversion](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=99dcc3e5a94ed491fbef402831d8c0bbb267f995) | [CPU ops and a rework of per cpu data handling on x86_64](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=dbfc196a3cc1a2514ad0737a82f764de23bd65e6) 的其中一个补丁. PCP 代码中使用 PER CPU 变量替代 NR_CPUS 的数组. | v1 ☑✓ 2.6.34-rc | [LORE RFC,00/45](https://lore.kernel.org/lkml/20071120011132.143632442@sgi.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,00/30](https://lore.kernel.org/lkml/20071116230920.278761667@sgi.com) |

page offline 期间存在一个竞争, 可能会导致无限循环. 造成这个问题的原因是: 这会造成这些页面永远不会出现在好友列表中, offline_pages() 会不断重试, 或者直到收到终止信号.


|  时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:---:|:---:|:----------:|:----:|
| 2020/09/03 | Pavel Tatashin <pasha.tatashin@soleen.com> | [mm/memory_hotplug: drain per-cpu pages again during memory offline](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9683182612214aa5f5e709fad49444b847cd866a) | 20200903140032.380431-1-pasha.tatashin@soleen.com | v2 ☑✓ 5.9-rc6 | [LORE v1](https://lore.kernel.org/linux-mm/20200901124615.137200-1-pasha.tatashin@soleen.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2](https://lore.kernel.org/all/20200903140032.380431-1-pasha.tatashin@soleen.com) |
| 2020/09/07 | Vlastimil Babka <vbabka@suse.cz> | [disable pcplists during page isolation](https://lore.kernel.org/all/20200907163628.26495-1-vbabka@suse.cz) | 页面隔离应该禁用 pcplists, 以避免页面释放过程中的竞争. | v1 ☐☑✓ | [LORE v1,0/5](https://lore.kernel.org/all/20200907163628.26495-1-vbabka@suse.cz) |
| 2020/11/11 | Vlastimil Babka <vbabka@suse.cz> | [disable pcplists during memory offline](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ec6e8c7e03147c65380e6c04c4cf4290e96280b6) | 当内存下线的时候, 禁用 PCP | v3 ☑ [5.11-rc1](https://kernelnewbies.org/Linux_5.11#Memory_management) | [LORE RFC,0/5](https://lore.kernel.org/lkml/20200907163628.26495-1-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[LORE 0/9](https://lore.kernel.org/lkml/20200922143712.12048-1-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/7](https://lore.kernel.org/lkml/20201008114201.18824-1-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/7](https://lore.kernel.org/lkml/20201111092812.11329-1-vbabka@suse.cz) |
| 2021/05/12 | Mel Gorman <mgorman@techsingularity.net> | [Use local_lock for pcp protection and reduce stat overhead](https://lore.kernel.org/all/20210512095458.30632-1-mgorman@techsingularity.net) | 20210512095458.30632-1-mgorman@techsingularity.net | v6 ☐☑✓ | [LORE v6,0/9](https://lore.kernel.org/all/20210512095458.30632-1-mgorman@techsingularity.net) |


### 2.2.6 ALLOC_NOFRAGMENT 优化
-------

页面分配最容易出现的就是外碎片化问题, 因此主线进行了锲而不舍的优化, Mel Gorman 提出的 [Fragmentation avoidance improvements v5](https://lore.kernel.org/patchwork/patch/1016503) 是比较有特色的一组.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/11/23 | Mel Gorman | [Fragmentation avoidance improvements v5](https://lore.kernel.org/patchwork/patch/1016503) | 伙伴系统页面分配时的反碎片化. | v5 ☑ 5.0-rc1 | [PatchWork v6](https://lore.kernel.org/patchwork/patch/1016503) |



当发现某个 ZONE 上内存分配可能出现紧张的时候, 那么有两种选择:

| 方式 | 描述 | 注意 | 触发场景 |
|:---:|:----:|:---:|:-------:|
| 从当前 ZONE(比如 ZONE_NORMAL) 的低端 ZONE(比如 ZONE_DMA32) 中去分配, 这样可以防止分配内存时将当前 ZONE 做了分割, 久而久之就造成了碎片化 | 在 x86 上, 节点 0 可能有多个分区, 而其他节点只有一个分区. 这样造成的结果是, 运行在节点0上的任务可能会导致 ZONE_NORMAL 区域出现了碎片化, 但是其实 ZONE_DMA32 区域还有足够的空闲内存. 如果(此次分配采取其他方式)**将来会导致外部碎片问题**, 在分配器的快速路径, 这样它将尝试从较低的本地 ZONE (比如 ZONE_DMA32) 分配, 然后才会尝试去分割较高的区域(比如 ZONE_NORMAL). |  | 会导致外部碎片问题的事件 |
| 从同 ZONE 的其他迁移类型的空闲链表中窃取内存过来 | 而**当发生了外碎片化的时候**, 则更倾向于从 ZONE_NORMAL 区域中其他迁移类型的空闲链表中多挪用一些空闲内存过来, 这比过早地使用低端 ZONE 区域的内存要很好多. | 理想情况下, 总是希望从其他迁移类型的空闲链表中至少挪用 2^pageblock_order 个空闲页面, 参见 __rmqueue_fallback 函数 | 已经发生了外碎片化 |

怎么取舍和选择两种方式是非常微妙的.

- [x] 如果首选的 ZONE 是高端的(比如 ZONE_NORMAL), 那么过早使用较低的低层次的 ZONE(ZONE_DMA32) 可能会导致这些低层次的 ZONE 面临内存短缺的压力, 这通常比破碎化更严重. 特别的, 如果下一个区域是ZONE_DMA, 那么它可能太小了. 因此只有分散分配才能避免正常区域和DMA32区域之间的碎片化. 参见 alloc_flags_nofragment 及其函数注释.

- [x] 如果总是从当前 ZONE 的其他 MIGRATE_TYPE 窃取内存过来, 将导致两个 ZONE 的内存都被切分成多块. 长此以往, 系统中将满是零零碎碎的切片, 将导致碎片化.

之前在 `__alloc_pages_nodemas` 中. 页面分配器基于每个节点的 ZONE 迭代, 而没有考虑抗碎片化. 补丁 1 [mm, page_alloc: spread allocations across zones before introducing fragmentation](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6bb154504f8b496780ec53ec81aba957a12981fa), 因此引入了一个新的选项 ALLOC_NOFRAGMENT, 来实现页面分配时候的抗外碎片化. 在特殊的场景下, 倾向于有限分配较低 ZONE 区域的内存, 而不是对较高的区域做切片. 补丁 1 增加了 `__alloc_pages_nodemas` 的 FRAGMENT AWARE 支持后, 页面分配的行为发生了变化. 如果发现当前 ZONE 中存在碎片化倾向(开始尝试执行 `__rmqueue_fallback` 去其他分组窃取内存)的时候:

*   如果分配内存的 order < pageblock_order 时, 就认为此操作即将导致外碎片化问题, 则倾向于从低端的 ZONE 中分配.

*   如果已经发生了碎片化, 或者分配的内存超过了 pageblock_order, 则倾向于从其他 MIGRATE_TYPE 中窃取内存过来分配.


补丁 2-4 [1c30844d2dfe mm: reclaim small amounts of memory when an external fragmentation event occurs
](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1c30844d2dfe) 则引入了 boost_watermark 机制, 在外部碎片事件发生时暂时提高水印. kswapd 唤醒以回收少量旧内存, 然后在完成时唤醒 kcompactd 以稍微恢复系统. 这在slowpath中引入了一些开销. 提升的级别可以根据对碎片和分配延迟的容忍程度进行调整或禁用.

补丁5暂停了一些可移动的分配请求, 以让补丁4的 kswapd 取得一些进展. 档位的持续时间很短, 但是如果可以容忍更大的档位, 则可以调整系统以避免碎片事件.

整个补丁在测试场景下将外碎片时间减少 94% 以上, 这收益大部分来自于补丁 1-4, 但补丁 5 可以处理罕见的特例, 并为THP分配成功率提供调整系统的选项, 以换取一些档位来控制碎片化.


### 2.2.7 页面窃取 page stealing
-------


在使用伙伴系统申请内存页面时, 如果所请求的 migratetype 的空闲页面列表中没有足够的内存, 伙伴系统尝试从其他不同的页面中窃取内存.

这会造成减少永久碎片化, 因此伙伴系统使用了各种各样启发式的方法, 尽可能的使这一事件不要那么频繁地触发,  最主要的思路是尝试从拥有最多免费页面的页面块中窃取, 并可能一次窃取尽量多的页面. 但是精确地搜索这样的页面块, 并且一次窃取整个页面块, 是昂贵的, 因此启发式方法是免费的列出从MAX_ORDER到请求的顺序, 并假设拥有最高次序空闲页面的块可能也拥有总数最多的空闲页面.

很有可能, 除了最高顺序的页面, 我们还从同一块中窃取低顺序的页面. 但我们还是分走了最高订单页面. 这是一种浪费, 会导致碎片化, 而不是避免碎片化.



因此, 这个补丁将__rmqueue_fallback()更改为仅仅窃取页面并将它们放到请求的migratetype的自由列表中, 并且只报告它是否成功. 然后我们使用__rmqueue_least()选择(并最终分割)最小的页面. 这一切都是在区域锁定下发生的, 所以在这个过程中没有人能从我们这里偷走它. 这应该可以减少由于回退造成的碎片. 在最坏的情况下, 我们只是窃取了一个最高顺序的页面, 并通过在列表之间移动它, 然后删除它而浪费了一些周期, 但后退并不是真正的热门路径, 所以这不应该是一个问题. 作为附带的好处, 该补丁通过重用__rmqueue_least()删除了一些重复的代码.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/01/23 | Vlastimil Babka <vbabka@suse.cz> | [page stealing tweaks](https://lore.kernel.org/patchwork/patch/535613) |  | v1 ☑ 4.13-rc1 | [PatchWork v1](https://lore.kernel.org/patchwork/patch/535613) |
| 2017/03/07 | Vlastimil Babka <vbabka@suse.cz> | [try to reduce fragmenting fallbacks](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=baf6a9a1db5a40ebfa5d3e761428d3deb2cc3a3b) | 修复 [Regression in mobility grouping?](https://lkml.org/lkml/2016/9/28/94) 上报的碎片化问题, 通过修改 fallback 机制和 compaction 机制来减少永久随便化的可能性. 其中 fallback 修改时, 仅尝试从不同 migratetype 的 pageblock 中窃取的页面中挑选最小(但足够)的页面. | v3 ☑ [4.12-rc1](https://kernelnewbies.org/Linux_4.12#Memory_management) | [LORE v3,0/8](https://lore.kernel.org/all/20170307131545.28577-1-vbabka@suse.cz), [关键 commit 3bc48f96cf11 ("mm, page_alloc: split least stolen page in fallback")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3bc48f96cf11ce8699e419d5e47ae0d456403274) |
| 2017/05/29 | Vlastimil Babka <vbabka@suse.cz> | [mm, page_alloc: fallback to smallest page when not stealing whole pageblock](https://lore.kernel.org/patchwork/patch/793063) |  | v1 ☑ 4.13-rc1 | [PatchWork v1](https://lore.kernel.org/patchwork/patch/793063), [commit 7a8f58f39188](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7a8f58f3918869dda0d71b2e9245baedbbe7bc5e) |

commit fef903efcf0cb9721f3f2da719daec9bbc26f12b
Author: Srivatsa S. Bhat <srivatsa.bhat@linux.vnet.ibm.com>
Date:   Wed Sep 11 14:20:35 2013 -0700

    mm/page_allo.c: restructure free-page stealing code and fix a bug


### 2.2.8 页面清 0 优化
-------

将页面内容归零通常发生在分配页面时, 这是一个耗时的操作, 它会使 pin 和 mlock 操作非常慢, 特别是对于大量内存.

| 2020/04/12 | Liang Li <liliang.opensource@gmail.com> | [mm: Add PG_zero support](https://lore.kernel.org/patchwork/patch/1222960) | 这个补丁引入了一个新特性, 可以在页面分配之前将页面清空, 它可以帮助加快页面分配.<br>想法很简单, 在系统不忙时将空闲页面清 0, 并用 PG_ZERO 标记页面, 分配页面时, 如果页面需要用零填充, 则检查 struct page 中的标志,  如果标记为 PG_ZERO, 则可以跳过清 0 的操作, 从而节省 CPU 时间并加快页面分配.<br>本系列基于 Alexander Duyck 推出的"免费页面报告"功能. | RFC ☐ | [PatchWork RFC,0/4](https://patchwork.kernel.org/project/linux-mm/cover/20200412090728.GA19572@open-light-1.localdomain) |
| 2020/12/21 | Liang Li <liliang.opensource@gmail.com> | [speed up page allocation for `__GFP_ZERO`](https://patchwork.kernel.org/project/linux-mm/cover/20201221162519.GA22504@open-light-1.localdomain) | mm: Add PG_zero support](https://lore.kernel.org/patchwork/patch/1222960) 系列的再版和延续. | RFC v2 ☐ | [PatchWork RFC,v2,0/4](https://patchwork.kernel.org/project/linux-mm/cover/20201221162519.GA22504@open-light-1.localdomain) |


### 2.2.9 优化
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/04/15 | Mel Gorman <mgorman@techsingularity.net> | [Optimise page alloc/free fast paths v3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=4741526b83c5d3a3d661d1896f9e7414c5730bcb) | 优化 page 申请和释放的快速路径. 优化后<br>1. 在 free 路径中, 调试检查和页面区域/页面块仍然查找占主导地位, 目前仍没有明显的解决方案. 在 alloc 路径中, 主要的耗时操作是处理 zonelist、新页面准备和 fair zone 分配以及无数的统计更新. | v3 ☑ 4.7-rc1 | [PatchWork v6 00/28](https://lore.kernel.org/patchwork/patch/668967) |


### 2.1.9 重构
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/04/15 | Mel Gorman <mgorman@techsingularity.net> | [Rationalise `__alloc_pages` wrappers](https://patchwork.kernel.org/project/linux-mm/cover/20210225150642.2582252-1-willy@infradead.org) | NA | v3 ☑ 4.7-rc1 | [PatchWork v3,0/7](https://patchwork.kernel.org/project/linux-mm/cover/20210225150642.2582252-1-willy@infradead.org) |


### 2.2.10 其他
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/05 | Zi Yan <zi.yan@sent.com> | [Make MAX_ORDER adjustable as a kernel boot time parameter.](https://lore.kernel.org/patchwork/patch/1472787) | 这个补丁集增加了启动参数添加可调的 MAX_ORDER 的支持, 以便用户可以更改从伙伴系统获得的页面的最大大小.<br>它还消除了基于 SECTION_SIZE_BITS 对 MAX_ORDER 的限制, 这样当设置了 SPARSEMEM_VMEMMAP 时, 伙伴系统分配器可以跨内存段合并 pfn. | RFC ☐ v5.14-rc4-mmotm-2021-08-02-18-51 | [PatchWork RFC,00/15](https://patchwork.kernel.org/project/linux-mm/cover/20210805190253.2795604-1-zi.yan@sent.com) |
| 2021/07/14 | Dave Chinner <david@fromorbit.com> | [xfs, mm: memory allocation improvements](https://patchwork.kernel.org/project/linux-mm/cover/20210225150642.2582252-1-willy@infradead.org) | NA | v3 ☐ | [PatchWork v3,0/7](https://patchwork.kernel.org/project/linux-mm/cover/20210714023440.2608690-1-david@fromorbit.com) |



## 2.3 内核级别的 malloc 分配器之-对象分配器(小内存分配)
-------

伙伴系统是每次分配内存最小都是以页(4KB)为单位的, 页面分配和回收起来很方便, 但是在实际使用过程中还是有一些问题的.

1.  系统运行的时候使用的绝大部分的数据结构都是很小的, 为一个小对象分配4KB显然是不划算了.

2.  内核中常见的是经常分配某种固定大小尺寸的对象, 并且对象都需要一定的初始化操作, 这个初始化操作有时比分配操作还要费时


因此 Linux 需要一个小块内存的快速分配方案:

1.      一个解决方法是用缓存池把这些对象管理起来, 把第一次分配作初始化; 释放时析构为这个初始状态, 这样能提高效率.

2.      此外, 增加一个缓存池, 把不同大小的对象分类管理起来, 这样能更高效地使用内存. 试想用固定尺寸的页分配器来分配给对象使用, 则不可避免会出现大量内部碎片.


因此 SLAB 等对象分配器应运而生.

BUDDY 提供了页面的分配和回收机制, 而在运行时, SLAB 等对象分配器向 BUDDY 一次性"批发"一些内存, 加工切块以后"散卖"出去. 等自己"批发"的库存耗尽了, 那么就再去向 BUDDY 申请批发一些. 在这个过程中, BUDDY 是一个内存的批发商, 接受一些大的内存订单, 而 SLAB 等对象分配器像是一个二级分销商, 把申请的内存零售出去, 一段时间以后如果客户不需要使用内存了, 那么 SLAB 这些分销商再零碎的回收回去, 当自己库存足够多的时候, 会再把库存退回给 BUDDY 这个批发商.


https://lore.kernel.org/patchwork/patch/47616/
https://lore.kernel.org/patchwork/patch/46669/
https://lore.kernel.org/patchwork/patch/46671/

https://lore.kernel.org/patchwork/patch/408914


### 2.3.1 SLAB
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2005/05/14 | Christoph Lameter <clameter@engr.sgi.com> | [NUMA aware slab allocator V3](https://lore.kernel.org/patchwork/patch/38309) | SLAB 分配器感知 NUMA | v3 ☑ 2.6.22-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/38309) |
| 2005/11/18 | Christoph Lameter <clameter@engr.sgi.com> | [NUMA policies in the slab allocator V2](https://lore.kernel.org/patchwork/patch/38309) | SLAB 分配器感知 NUMA | v3 ☑ 2.6.16-rc2 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/38309) |
| 2007/02/28 | Mel Gorman | [mm/slab: reduce lock contention in alloc path](https://lore.kernel.org/patchwork/patch/667440) | 优化 SLAB 分配的路径, 减少对 lock 的争抢, 实现 lockless. | v2 ☑ 2.6.22-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/667440) |
| 2018/07/09 | [Improve shrink_slab() scalability (old complexity was O(n^2), new is O(n))](http://lore.kernel.org/patchwork/patch/960597) | 内存镜像的功能 | RFC v2 ☐  | [PatchWork](https://lore.kernel.org/patchwork/patch/960597) |
| 2007/05/04 | clameter@sgi.com <clameter@sgi.com> | [Slab Defrag / Slab Targeted Reclaim and general Slab API changes](https://lore.kernel.org/all/20070504221555.642061626@sgi.com) | 20070504221555.642061626@sgi.com | v1 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20070504221555.642061626@sgi.com) |

https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0aa817f078b655d0ae36669169d73a5c8a388016

**2.0 版本时代(1996年引入)**

这是最早引入的对象分配器, Linux 所使用的 slab 分配器的基础是 Jeff Bonwick 为 SunOS 操作系统首次引入的一种算法. Jeff 的分配器是围绕对象缓存进行的. 在内核中, 会为有限的对象集(例如文件描述符和其他常见结构)分配大量内存. Jeff 发现对内核中普通对象进行初始化所需的时间超过了对其进行分配和释放所需的时间. 因此他的结论是不应该将内存释放回一个全局的内存池, 而是将内存保持为针对特定目而初始化的状态. 例如, 如果内存被分配给了一个互斥锁, 那么只需在为互斥锁首次分配内存时执行一次互斥锁初始化函数(mutex_init)即可. 后续的内存分配不需要执行这个初始化函数, 因为从上次释放和调用析构之后, 它已经处于所需的状态中了. 这里想说的主要是我们得到一块很原始的内存, 必须要经过一定的初始化之后才能用于特定的目的. 当我们用slab时, 内存不会被释放到全局内存池中, 所以还是处于特定的初始化状态的. 这样就能加速我们的处理过程.

Linux slab 分配器使用了这种思想和其他一些思想来构建一个在空间和时间上都具有高效性的内存分配器.

1.  小对象的申请和释放通过slab分配器来管理.
    SLAB 基于页分配器分配而来的页面(组), 实现自己的对象缓存管理. 它提供预定尺寸的对象缓存, 也支持用户自定义对象缓存

2.  slab分配器有一组高速缓存, 每个高速缓存保存同一种对象类型, 如i节点缓存、PCB缓存等.
    维护着每个 CPU , 每个 NUMA node 的缓存队列层级, 可以提供高效的对象分配

3.  内核从它们各自的缓存种分配和释放对象.

4.  每种对象的缓存区由一连串slab构成, 每个slab由一个或者多个连续的物理页面组成
    *   这些页面种包含了已分配的缓存对象, 也包含了空闲对象.
    *   还支持硬件缓存对齐和着色, 所谓着色, 就是把不同对象地址, 以缓存行对单元错开, 从而使不同对象占用不同的缓存行, 从而提高缓存的利用率并获得更好的性能

与传统的内存管理模式相比,  slab 缓存分配器提供了很多优点. 首先, 内核通常依赖于对小对象的分配, 它们会在系统生命周期内进行无数次分配. slab 缓存分配器通过对类似大小的对象进行缓存而提供这种功能, 从而避免了常见的碎片问题. slab 分配器还支持通用对象的初始化, 从而避免了为同一目而对一个对象重复进行初始化. 最后, slab 分配器还可以支持硬件缓存对齐和着色, 这允许不同缓存中的对象占用相同的缓存行, 从而提高缓存的利用率并获得更好的性能.


### 2.3.2 SLUB
-------

随着大规模多处理器系统和NUMA系统的广泛应用, slab终于暴露出不足:

1.  复杂的队列管理

2.  管理数据和队列存储开销较大

3.  长时间运行 partial 队列可能会非常长

4.  对 NUMA 支持非常复杂

为了解决这些高手们开发了slub: 改造 page 结构来削减 slab 管理结构的开销、每个 CPU 都有一个本地活动的 slab(kmem_cache_cpu)等. 对于小型的嵌入式系统存在一个 slab模拟层slob, 在这种系统中它更有优势.

**2.6.22(2007年7月发布)**


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/02/28 | Christoph Lameter <clameter@engr.sgi.com> | [SLUB The unqueued slab allocator V6](https://lore.kernel.org/patchwork/patch/77393) | 优化调度器的路径, 减少对 rq->lock 的争抢, 实现 lockless. | v3 ☑ 2.6.22-rc1 | [PatchWork v3](https://lore.kernel.org/patchwork/patch/75156)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6](https://lore.kernel.org/patchwork/patch/75156), [commit 81819f0fc828 SLUB core](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=81819f0fc8285a2a5a921c019e3e3d7b6169d225) |
| 2011/08/09 | Christoph Lameter <cl@linux.com> | [slub: per cpu partial lists V4](https://lore.kernel.org/patchwork/patch/262225) | slub 的 per-cpu 页面池, 有助于避免每个节点锁定开销, 使得 SLUB 的部分工作(比如fastpath和free slowpath)可以进一步在不禁用中断的情况下工作, 可以进一步减少分配器的延迟. | v3 ☑ 2.6.22-rc1 | [PatchWork v6](https://lore.kernel.org/patchwork/patch/262225) |
| 2020/10/15 | Kees Cook <keescook@chromium.org> | [Actually fix freelist pointer vs redzoning](https://lore.kernel.org/patchwork/patch/1321332) | NA | v3 ☑ 2.6.22-rc1 | [PatchWork v3,0/3](https://lore.kernel.org/patchwork/patch/1321332) |
| 2021/08/23 | Vlastimil Babka <vbabka@suse.cz> | [SLUB: reduce irq disabled scope and make it RT compatible](https://lore.kernel.org/patchwork/patch/1469467) | 本系列最初是受到 Mel 的 pcplist local_lock 重写的启发, 同时也对更好地理解 SLUB 的锁以及新的原语、RT变体和含义感兴趣. 使 SLUB 更有利于抢占, 特别是对于RT. | v3 ☐ 5.14-rc3 | [PatchWork v3,00/35](https://patchwork.kernel.org/project/linux-mm/cover/20210729132132.19691-1-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[PatchWork v5,00/35](https://patchwork.kernel.org/project/linux-mm/cover/20210823145826.3857-1-vbabka@suse.cz) |
| 2021/10/12 | Vlastimil Babka <vbabka@suse.cz> | [mm, slub: change percpu partial accounting from objects to pages](https://patchwork.kernel.org/project/linux-mm/patch/20211012134651.11258-1-vbabka@suse.cz) | NA | v3 ☑ 2.6.22-rc1 | [PatchWork v6](https://lore.kernel.org/patchwork/patch/262225) |


SLUB 这是第二个对象分配器实现. 引入这个新的实现的原因是 SLAB 存在的一些问题. 比如 NUMA 的支持, SLAB 引入时内核还没支持 NUMA, 因此, 一开始就没把 NUMA 的需求放在设计理念里, 结果导致后来的对 NUMA 的支持比较臃肿奇怪, 一个典型的问题是, SLAB 为追踪这些缓存, 在每个 CPU, 每个 node, 上都维护着对象队列. 同时, 为了满足 NUMA 分配的局部性, 每个 node 上还维护着所有其他 node 上的队列, 这样导致 SLAB 内部为维护这些队列就得花费大量的内存空间, 并且是O(n^2) 级别的. 这在大规模的 NUMA 机器上, 浪费的内存相当可观. 同时, 还有别的一些使用上的问题, 导致开发者对其不满, 因而引入了新的实现. 参见 [The SLUB allocator](https://lwn.net/Articles/229984).

SLUB 在解决了上述的问题之上, 提供与 SLAB 完全一样的接口, 所以用户可以无缝切换, 而且, 还提供了更好的调试支持. 早在几年前, 各大发行版中的对象分配器就已经切换为 SLUB 了.

关于性能, 前阵子为公司的系统切换 SLUB, 做过一些性能测试, 在一台两个 NUMA node, 32 个逻辑 CPU , 252 GB 内存的机器上, 在相同的 workload 测试下, SLUB 综合来说, 体现出了比 SLAB 更好的性能和吞吐量.



### 2.3.3 SLOB
-------

**2.6.16(2006年3月发布)**

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2005/11/03 | Matt Mackall <mpm@selenic.com> | [slob: introduce the SLOB allocator](https://lore.kernel.org/patchwork/patch/45623) | 实现 SLOB 分配器 | v2 ☑ 2.6.16-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/45623) |
| 2021/10/18 | Matt Mackall <mpm@selenic.com> | [slob: add size header to all allocations](https://patchwork.kernel.org/project/linux-mm/patch/20211018033841.3027515-1-rkovhaev@gmail.com) | 在所有分配的 (PAGE_SIZE - align_offset) 和 less 前面加上 size 头. 这样, kfree() 和 kfree() 都可以释放 kmem_cache_alloc() 内存, 只要它们小于 (PAGE_SIZE - align_offset). 这个更改的主要原因是稍微简化了 SLOB, 使它在出现问题时更容易调试. | v1 ☐ | [PatchWork v2](https://patchwork.kernel.org/project/linux-mm/patch/20211018033841.3027515-1-rkovhaev@gmail.com)<br>*-*-*-*-*-*-*-*<br>[PatchWork v4](https://patchwork.kernel.org/project/linux-mm/patch/20211122013026.909933-1-rkovhaev@gmail.com) |


这是第三个对象分配器, 提供同样的接口, 它是为适用于嵌入式小内存小机器的环境而引入的, 所以实现上很精简, 大大减小了内存 footprint, 能在小机器上提供很不错的性能.

### 2.3.4 SLQB
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/01/23 | Nick Piggin <npiggin@suse.de> | [SLQB slab allocator](https://lwn.net/Articles/311502) | 实现 SLQB 分配器 | v2 ☐ | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/1385629)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/141837) |


### 2.3.5 kmalloc
-------


kmalloc 的 API 家族对 mm 非常关键, 但有一个缺点, 就是它的对象大小被固定为 2 次方. 当用户请求内存为 '2^n +1' 字节时, 实际上会分配 2^(n+1) 字节, 所以在最坏的情况下, 大约有 50% 的内存空间浪费. 因此内核可能会以为浪费了太多的内存, 从而导致 OOM. 参见 [Crash kernel with 256 MB reserved memory runs into OOM condition](https://lkml.org/lkml/2019/8/12/266) 和 [Re: [PATCH] iommu/iova: change IOVA_MAG_SIZE to 127 to save memory](https://lore.kernel.org/lkml/2920df89-9975-5785-f79b-257d3052dfaf@huawei.com).

为了能辅助分析 kmalloc 本身造成的浪费(内碎片), [mm/slub: enable debugging memory wasting of kmalloc](https://lore.kernel.org/all/20220701135954.45045-1-feng.tang@intel.com) 增加了 waste 信息记录 kmalloc/slub 造成的内存浪费.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/04/14 | Hyeonggon Yoo <42.hyeyoo@gmail.com> | [common kmalloc subsystem on SLAB/SLUB](https://patchwork.kernel.org/project/linux-mm/cover/20220308114142.1744229-1-42.hyeyoo@gmail.com) | 清理 slab 公共代码. 在这组补丁之后后, kmalloc 子系统在 SLAB 和 SLUB 之间得到了完美的推广. | v1 ☐☑ | [2022/03/08 LORE v1,00/15](https://lore.kernel.org/r/20220308114142.1744229-1-42.hyeyoo@gmail.com)<br>*-*-*-*-*-*-*-* <br>[2022/04/14 LORE v2,0/23](https://lore.kernel.org/r/20220414085727.643099-1-42.hyeyoo@gmail.com)<br>*-*-*-*-*-*-*-* <br>[2022/07/12 LORE v3,0/15](https://lore.kernel.org/r/20220712133946.307181-1-42.hyeyoo@gmail.com) |
| 2022/07/01 | Feng Tang <feng.tang@intel.com> | [mm/slub: enable debugging memory wasting of kmalloc](https://lore.kernel.org/all/20220701135954.45045-1-feng.tang@intel.com) | 这个补丁帮助显示了当前 kmalloc 下的浪费的空间, 信息在 `/sys/kernel/debug/slab/kmalloc-xx/alloc_traces` 中显示, 显示的格式为: waste=总共浪费的字节数目/单词请求浪费的字节数目. | v1 ☐☑✓ | [LORE RFC](https://lore.kernel.org/r/20220630014715.73330-1-feng.tang@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE](https://lore.kernel.org/all/20220701135954.45045-1-feng.tang@intel.com) |


### 2.3.6 改进与优化
-------

*   slab_merge

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2014/09/15 | Joonsoo Kim <iamjoonsoo.kim@lge.com> | [mm/slab_common: commonize slab merge logic](https://lore.kernel.org/patchwork/patch/493577) | 如果新创建的 SLAB 具有与现有 SLAB 相似的大小和属性, 该特性将重用它而不是创建一个新的. 这特性就是我们熟知的 `__kmem_cache_alias()`. SLUB 原生支持 merged. | v2 ☑ [3.18-rc1](https://kernelnewbies.org/Linux_3.18#Memory_management) | [PatchWork v1](https://lore.kernel.org/patchwork/patch/493577)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/499717) |
| 2017/06/20 | Joonsoo Kim <iamjoonsoo.kim@lge.com> | [mm: Allow slab_nomerge to be set at build time](https://lore.kernel.org/patchwork/patch/802038) | 新增 CONFIG_SLAB_MERGE_DEFAULT, 支持通过编译开关 slab_merge | v2 ☑ [3.18-rc1](https://kernelnewbies.org/Linux_3.18#Memory_management) | [PatchWork v1](https://lore.kernel.org/patchwork/patch/801532)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/802038) |
| 2021/03/29 | Joonsoo Kim <iamjoonsoo.kim@lge.com> | [mm/slab_common: provide "slab_merge" option for !IS_ENABLED(CONFIG_SLAB_MERGE_DEFAULT) builds](https://lore.kernel.org/patchwork/patch/1399240) | 如果编译未开启 CONFIG_SLAB_MERGE_DEFAULT, 可以通过 slab_merge 内核参数动态开启  | v2 ☐ | [PatchWork v1](https://lore.kernel.org/patchwork/patch/1399240)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/1399260) |


*   slab 抗碎片化

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/08/11 | Christoph Lameter <cl@linux-foundation.org> | [Slab Fragmentation Reduction V14](https://lore.kernel.org/patchwork/patch/125818) | SLAB 抗碎片化 | v14 ☐ | [PatchWork v5](https://lore.kernel.org/patchwork/patch/90742)<br>*-*-*-*-*-*-*-* <br>[PatchWork v14](https://lore.kernel.org/patchwork/patch/125818) |


*   kmalloc-reclaimable caches


内核的 dentry 缓存用于缓存文件系统查找的结果, 这些对象是可以立即被回收的. 但是使用 kmalloc() 完成的分配不能直接回收. 虽然这并不是一个大问题, 因为 dentry 收缩器可以回收这两块内存. 但是内核对真正可回收的内存的计算被这种分配模式打乱了. 内核经常会认为可回收的内存比实际的要少, 因此不必要地进入 OOM 状态.

[LSF/MM 2018](https://lwn.net/Articles/lsfmm2018) 讨论了一个有意思的话题: 可回收的 SLAB. 参见 [The slab and protected-memory allocators](https://lwn.net/Articles/753154), 这种可回收的 slab 用于分配那些可根据用户请求随时进行释放的内核对象, 比如内核的 dentry 缓存.

Roman Gushchin indirectly reclaimable memory](https://lore.kernel.org/patchwork/patch/922092) 可以认为是一种解决方案. 它创建一个新的计数器(nr_indirect_reclaimable)来跟踪可以通过收缩不同对象来释放的对象所使用的内存. 该特性于 [4.17-rc1](https://kernelnewbies.org/Linux_4.20#Memory_management) 合入主线.

不过, Vlastimil Babka 对补丁集并不完全满意. 计数器的名称迫使用户关注"间接"可回收内存, 这是他们不应该做的. Babka 认为更好的解决方案是为这些 kmalloc() 调用制作一套单独的[可回收 SLAB](https://lore.kernel.org/patchwork/patch/969264). 这将把可回收的对象放在一起, 从碎片化的角度来看, 这个方案是非常好的. 通过 GFP_RECLAIMABLE 标志, 可以从这些 slab 中分配内存. 经历了 v4 个版本后, 于 [4.20-rc1](https://kernelnewbies.org/Linux_4.20#Memory_management) 合入.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/03/05 | Roman Gushchin <guro@fb.com> | [indirectly reclaimable memory](https://lore.kernel.org/patchwork/patch/922092) | 引入间接回收内存的概念( `/proc/vmstat/nr_indirect _reclaimable`). 间接回收内存是任何类型的内存, 由内核使用(除了可回收的 slab), 这实际上是可回收的, 即将在内存压力下释放. 并[被当作 available 的内存对待](034ebf65c3c21d85b963d39f992258a64a85e3a9). | v1 ☑ [4.17-rc1](https://kernelnewbies.org/Linux_4.20#Memory_management) | [PatchWork](https://lore.kernel.org/patchwork/patch/922092)) |
| 2018/07/31 | Vlastimil Babka <vbabka@suse.cz> | [kmalloc-reclaimable caches](https://lore.kernel.org/patchwork/patch/969264) | 为 kmalloc 引入回收 SLAB, dcache external names 是 kmalloc-rcl-* 的第一个用户.  | v4 ☑ [4.20-rc1](https://kernelnewbies.org/Linux_4.20#Memory_management) |  [PatchWork v4](https://lore.kernel.org/patchwork/patch/969264)) |


https://lore.kernel.org/patchwork/patch/76916
https://lore.kernel.org/patchwork/patch/78940
https://lore.kernel.org/patchwork/patch/72119
https://lore.kernel.org/patchwork/patch/63980
https://lore.kernel.org/patchwork/patch/91223
https://lore.kernel.org/patchwork/patch/145184
https://lore.kernel.org/patchwork/patch/668967


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/09/20 | Hyeonggon Yoo <42.hyeyoo@gmail.com> | [mm, sl[au]b: Introduce lockless cache](https://patchwork.kernel.org/project/linux-mm/patch/20210920154816.31832-1-42.hyeyoo@gmail.com) | 板上无锁缓存的 RFC v2, 用于 IO 轮询等场景.<br>最近块层实现了 percpu, 在板分配器顶部实现了无锁缓存. 它可以用于 IO 轮询, 因为 IO 轮询禁用中断. | v1 ☐ | [PatchWork](https://patchwork.kernel.org/project/linux-mm/patch/20210919164239.49905-1-42.hyeyoo@gmail.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork](https://lore.kernel.org/patchwork/patch/922092)) |


## 2.4 内核级别的 malloc 分配器之-大内存分配
-------

### 2.4.1 VMALLOC 大内存分配器
-------

小内存的问题算是解决了, 但还有一个大内存的问题: 用伙伴系统分配 8 x 4KB 大小以上的的数据时, 只能从 16 x 4KB 的空闲列表里面去找(这样得到的物理内存是连续的), 但很有可能系统里面有内存, 但是伙伴系统分配不出来, 因为他们被分割成小的片段. 那么, vmalloc 就是要用这些碎片来拼凑出一个大内存, 相当于收集一些"边角料", 组装成一个成品后"出售".

2.6.28 时, Nick Piggin 重写了 vmalloc/vmap 分配器[mm: vmap rewrite](https://lwn.net/Articles/304188). 做了几项比较大的改动和优化.

1.  由于内核会频繁的进行 VMAP 区域的查找, 引入[红黑树表示的 vmap_area](https://elixir.bootlin.com/linux/v2.6.28/source/mm/vmalloc.c#L242) 来解决当查找数量非常多时效率低下的问题. 同时使用 vmap_area 的链表 vmap_area_list 替代原来的链表.

2.  由于要映射页表, 因此 TLB flush 就成了必需要做的操作. 这个过程在 vfree 流程中进行, 为了减少开销, 引入了 lazy 模式.

3.  整个 VMAP 子系统在一个全局读写锁 vmlist_lock 下工作. 虽然是 rwlock, 但是但它实际上是写在所有的快速路径上, 并且读锁可能永远不会并发运行, 所以这对于小型 VMAP 是毫无意义的. 因此实现了一个按 CPU 分配的分配器, 它可以平摊或避免全局锁定. 要使用 CPU 接口, 必须使用 vm_map_ram()/vm_unmap_ram() 接口来代替 vmap() 和 vunmap().  当前 vmalloc 目前没有使用这些接口, 所以它的可伸缩性不是很好(尽管它将使用惰性TLB刷新).



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| NA | Nick Piggin <npiggin@suse.de> | [Import 0.99.13k](https://elixir.bootlin.com/linux/0.99.13k/source/mm/vmalloc.c) | 引入 vmalloc 分配器. | ☑ 0.99.13k | [HISTORY commit](https://github.com/gatieme/linux-history/commit/537b6ff02ce317a747658a9f5dff96ed2733b28a) |
| 2002/08/06 | Nick Piggin <npiggin@suse.de> | [VM: Rework vmalloc code to support mapping of arbitray pages](https://github.com/gatieme/linux-history/commit/d24919a7fbc635bea6ecc267058dcdbadf03f565) | 引入 vmap/vunmap. 将 vmalloc 操作分为两部分: 分配支持物理地址不连续的页面(vmalloc)以及将这些页面映射到内核页表中, 以便进行实际的临时访问. 因此引入一组新的接口vmap/vunmap 允许将任意页映射到内核虚拟内存中. | ☑ 2.5.32~39 | [HISTORY commit](https://github.com/gatieme/linux-history/commit/d24919a7fbc635bea6ecc267058dcdbadf03f565) |
| 2008/07/28 | Nick Piggin <npiggin@suse.de> | [mm: vmap rewrite](https://lwn.net/Articles/304188) | 重写 vmap 分配器 | RFC ☑ 2.6.28-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/118352))<br>*-*-*-*-*-*-*-* <br>[PatchWork](https://lore.kernel.org/patchwork/patch/124065), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=db64fe02258f1507e13fe5212a989922323685ce) |
| 2009/02/18 | Tejun Heo <tj@kernel.org> | [implement new dynamic percpu allocator](https://lore.kernel.org/patchwork/patch/144750) | 实现了 [vm_area_register_early()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f0aa6617903648077dffe5cfcf7c4458f4610fa7) 以支持在启动阶段注册 vmap 区域. 基于此特性实现了可伸缩的动态 percpu 分配器(CONFIG_HAVE_DYNAMIC_PER_CPU_ARE), 可用于静态(pcpu_setup_static)和动态(percpu_modalloc) percpu 区域, 这将允许静态和动态区域共享更快的直接访问方法. | v1 ☑ 2.6.30-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/144750) |
| 2016/03/29 | Chris Wilson <chris@chris-wilson.co.uk> | [mm/vmap: Add a notifier for when we run out of vmap address space](https://lore.kernel.org/patchwork/patch/662338) | vmap是临时的内核映射, 可能持续时间很长. 对于驱动程序来说, 在对象上重用vmap是更好的选择, 因为在其他情况下, 设置vmap的成本可能会支配对象上的操作. 然而, 在32位系统上, vmap地址空间非常有限, 因此我们添加了一个vmap压力通知, 以便驱动程序释放任何缓存的 vmap 区域. 并为该通知链添加了首批用户. | v3 ☑ 4.7-rc1 | [PatchWork v3](https://lore.kernel.org/patchwork/patch/664579) |
| 2019/01/03 |  "Uladzislau Rezki (Sony)" <urezki@gmail.com> | [test driver to analyse vmalloc allocator](https://lore.kernel.org/patchwork/patch/1028793) | 实现一个驱动来帮助分析和测试 vmalloc | RFC v4 ☑ [5.1-rc1](https://kernelnewbies.org/Linux_5.1#Memory_management) | [PatchWork RFC v4](https://patchwork.kernel.org/project/linux-mm/cover/20190103142108.20744-1-urezki@gmail.com) |
| 2019/10/31 | Daniel Axtens <dja@axtens.net> | [kasan: support backing vmalloc space with real shadow memory](https://lore.kernel.org/patchwork/patch/1146684) | NA | v11 ☑ 5.5-rc1 | [PatchWork v11,0/4](https://lore.kernel.org/patchwork/patch/1146684) |
| 2021/03/24 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [vmalloc: Improve vmalloc(4MB) performance](https://lore.kernel.org/patchwork/patch/1401688) | 加速 4MB vmalloc 分配. | v2 ☑ 5.13-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/1401688) |
| 2006/04/21 | Nick Piggin <npiggin@suse.de> | [mm: introduce remap_vmalloc_range](https://lore.kernel.org/patchwork/patch/55978) | 添加 remap_vmalloc_range()、vmalloc_user() 和 vmalloc_32_user(), 这样驱动程序就可以有一个很好的接口来重新映射vmalloc内存. | v2 ☑ 2.6.18-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/55972)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/55978) |
| 2013/05/23 | HATAYAMA Daisuke <d.hatayama@jp.fujitsu.com> | [kdump, vmcore: support mmap() on /proc/vmcore](https://lore.kernel.org/patchwork/patch/381217) | NA | v8 ☑ 3.11-rc1 | [PatchWork v8,0/9](https://lore.kernel.org/patchwork/patch/381217) |
| 2017/01/02 | Michal Hocko <mhocko@suse.com> | [kvmalloc](https://lore.kernel.org/patchwork/patch/755798) | 实现 kvmalloc()/kvzalloc(). | v8 ☑ 3.11-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/381217)<br>*-*-*-*-*-*-*-* <br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a7c3e901a46ff54c016d040847eda598a9e3e653) |
| 2021/03/09 | Topi Miettinen <toiwoton@gmail.com> | [mm/vmalloc: randomize vmalloc() allocations](https://lore.kernel.org/patchwork/patch/1392280) | vmalloc() 随机分配. 内核中使用 vmalloc() 分配的内存映射是按可预测的顺序排列的, 并且紧密地向低地址填充, 除了从 vmalloc 区域的顶部开始的 per-cpu 区域. 有了新的内核引导参数 'randomize_vmalloc = 1', 整个区域被随机使用, 使得分配更难以预测, 更难让攻击者猜测. 此外, 模块和 BPF 代码位置是随机的(在它们专用的很小的区域内), 如果启用了 CONFIG_VMAP_STACK, 内核栈位置也是随机的.  | v4 ☐ | [PatchWork](https://lore.kernel.org/patchwork/patch/1392280) |
| 2022/04/18 | Waiman Long <longman@redhat.com> | [[RFC] mm/mmap: Map MAP_STACK to VM_STACK](https://patchwork.kernel.org/project/linux-mm/patch/20220418145620.788664-1-longman@redhat.com/) | 633056 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220418145620.788664-1-longman@redhat.com) |



#### 2.4.1.2 VMALLOC 中的 RBTREE 和 LIST 以及保护它们的锁
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/07/28 | Nick Piggin <npiggin@suse.de> | [mm: vmap rewrite](https://lwn.net/Articles/304188) | 重写 vmap 分配器, 为 vmap_area 首次引入了红黑树组织(使用 vmap_area_lock 来保护). | RFC ☑ 2.6.28-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/118352))<br>*-*-*-*-*-*-*-* <br>[PatchWork](https://lore.kernel.org/patchwork/patch/124065), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=db64fe02258f1507e13fe5212a989922323685ce) |
| 2011/03/22 | Nick Piggin <npiggin@suse.de> | [mm: vmap area cache](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=89699605fe7cfd8611900346f61cb6cbf179b10a) | 为 vmalloc 分配器提供一个空闲区域缓存 free_vmap_cache, 它缓存了上次搜索和分配的空闲区域的节点, 下次分配和释放时, 都将从这里开始搜索, 避免了每次从 VMALLOC_START 搜索. 之前的实现中, 如果 vmalloc/vfree 频繁, 将从 VMALLOC_START 地址建立一个区段链, 每次都必须对其进行迭代, 这将是 O(n) 的开销. 在这个补丁之后, 搜索将从它结束的地方开始, 给出更接近于 O(1) 的平摊结果. 这一定程度上减少了rbtree操作的数量. 这解决了在 [db64fe02 mm: rewrite vmap layer](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=db64fe02258f1507e13fe5212a989922323685ce) 中引入的惰性 vunmap TLB 清除导致的回归. 在 vunmapped 区段之后, 该补丁将在 vmap 分配器中保留区段, 直到可以在单个批处理中刷新的大量区段积累为止.  | v2 ☑ 3.10-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=89699605fe7cfd8611900346f61cb6cbf179b10a) |
| 2013/03/13 | Joonsoo Kim <iamjoonsoo.kim@lge.com> | [remove vm_struct list management](https://lore.kernel.org/patchwork/patch/365284) | 在 vmalloc 初始化后, 不再使用旧的 vmlist 链表管理 vm_struct. 而是直接使用由 vmap_area 组织成的红黑树 vmap_area_root和链表 vmap_area_list 管理. 这个补丁之后 vmlist 被标记为 `__initdata`, 且只在 vmalloc_init 之前, 通过  vm_area_register_early-=>vm_area_add_early 来使用, 比如[注册 percpu 区域](https://elixir.bootlin.com/linux/v3.10/source/mm/percpu.c#L1785). 注意: 很多网络的博客上说, vm_struct 通过 next 字段组成链表, 这个链表之后, 这个说法在这个补丁之后已经不严谨了. | v2 ☑ 3.10-rc1 | [PatchWork v2,0/8](https://lore.kernel.org/patchwork/patch/365284)<br>*-*-*-*-*-*-*-* <br>[关注 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4341fa454796b8a37efd5db98112524e85e7114e) |
| 2016/04/15 | "Uladzislau Rezki (Sony)" <urezki@gmail.com> | [mm/vmalloc: Keep a separate lazy-free list](https://lore.kernel.org/patchwork/patch/669083) | 将惰性释放的 vmap_area 添加到一个单独的无锁空闲 vmap_purge_list, 这样我们就不必在每次清除时遍历整个列表.  | v2 ☑ 4.7-rc1 | [PatchWork v1](https://lore.kernel.org/patchwork/patch/667471)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/669083)<br>*-*-*-*-*-*-*-* <br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=80c4bd7a5e4368b680e0aeb57050a1b06eb573d8) |
| 2019/04/06 |  "Uladzislau Rezki (Sony)" <urezki@gmail.com> | [improve vmap allocation](https://patchwork.kernel.org/project/linux-mm/cover/20181019173538.590-1-urezki@gmail.com) | [improve vmap allocation](https://lore.kernel.org/patchwork/patch/1059021) 的 RESEND 和再版. 优化 alloc_vmap_area(), 优化查找复杂度从 O(N) 下降为 O(logN). 目前新的 VA 区域的分配是在繁忙列表(vmap_area_list/root)迭代中完成的, 直到在两个繁忙区域之间找到一个合适的空洞. 因此, 每次新的分配都会导致列表增长. 查找空洞的代价为 O(N). 而通过跟踪空闲的 vmap_area 区域, 并在空闲列表搜索空洞来完成分配, 可以将分配的开销降到 O(logN). 在初始化阶段 vmalloc_init() 中, vmalloc 内存布局将[对空闲区域进行跟踪和组织](https://elixir.bootlin.com/linux/v5.2/source/mm/vmalloc.c#L1880), 并用链表 free_vmap_area_list 和 红黑书 free_vmap_area_root 组织. 为了优化查找效率, vmap_area 中 subtree_max_size 存储了其节点为根的红黑树中最大的空洞大小. | v3 ☑ [5.2-rc7](https://kernelnewbies.org/Linux_5.2#Memory_management) | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/1002038)<br>*-*-*-*-*-*-*-* <br>[PatchWork v4](https://patchwork.kernel.org/project/linux-mm/patch/20190406183508.25273-2-urezki@gmail.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3](https://lore.kernel.org/lkml/20190402162531.10888-1-urezki@gmail.com) |
| 2019/07/06 | Pengfei Li <lpf.vector@gmail.com> | [mm/vmalloc.c: improve readability and rewrite vmap_area](https://lore.kernel.org/patchwork/patch/1101062) | 为了减少结构体 vmap_area 的大小.<br>1. "忙碌"树可能相当大, 即使区域被释放或未映射, 它仍然停留在那里, 直到"清除"逻辑删除它. 优化和减少"忙碌"树的大小, 在用户触发空闲路径时立即删除一个节点. 这样做是可能的, 因为分配是使用另一个扩展树完成的;这样忙碌的树将只包含分配的区域, 不会干扰惰性空闲的节点, 引入新的函数 show_purge_info(), 转储通过"/proc/vmallocinfo" 来显示 "unpurged" 区域.<br>2. 由于结构体 vmap_area 的成员不是同时使用的, 所以可以通过将几个不同时使用的成员放入联合中来减少其大小. | v6 ☑ 5.4-rc1 | [PatchWork v1,0/5](https://lore.kernel.org/patchwork/patch/1095787)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/5](https://lore.kernel.org/patchwork/patch/1096583)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6,0/2](https://patchwork.kernel.org/project/linux-mm/cover/20190716152656.12255-1-lpf.vector@gmail.com) |
| 2020/11/16 | "Uladzislau Rezki (Sony)" <urezki@gmail.com> | [mm/vmalloc: rework the drain logic](https://lore.kernel.org/patchwork/patch/1339347) | 当前的"惰性释放"模式至少存在两个问题: <br>1. 由于只有 vmap_purge_list 链表组织, 因此 vmap 区域的未排序的, 因此为了识别要耗尽的区域的 [min:max] 范围, 它需要扫描整个 vmap_purge_list 链表. 可能会耗费较多时间.<br>2. 作为下一步是关于合并所有片段与自由空间.这也是一种耗时的操作, 因为它必须遍历包含突出惰性区域的整个列表. 这造成了极高的延迟. | v1 ☑ 5.11-rc1 | [PatchWork](https://patchwork.kernel.org/project/linux-mm/patch/20201116220033.1837-2-urezki@gmail.com) |
| 2019/10/22 | Uladzislau Rezki (Sony) <urezki@gmail.com> | [mm/vmalloc: rework vmap_area_lock](https://lore.kernel.org/patchwork/patch/1143029) | 随着 5.2 版本 [improve vmap allocation](https://lore.kernel.org/patchwork/patch/1059021) 的合入, 全局的 vmap_area_lock 可以被拆分成两个锁: 一个用于分配部分(vmap_area_lock), 另一个用于回收(free_vmap_area_lock), 因为有两个不同的实体: "空闲数据结构"和"繁忙数据结构". 这和那后的减少锁争用, 允许在不同的 CPU 上并行执行"空闲"和"忙碌"树的操作. 但是需要注意的是, 分配/释放操作仍然存在依赖. 如果在不同的 CPU 上同时运行, 分配/释放操作仍然会相互干扰. | v1 ☑ 5.5-rc1 | [PatchWork v2,0/8](https://patchwork.kernel.org/project/linux-mm/patch/20191022155800.20468-1-urezki@gmail.com)<br>*-*-*-*-*-*-*-* <br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e36176be1c3920a487681e37158849b9f50189c4) |


#### 2.4.1.3 lazy drain
-------


vmalloc 分配的内存并不是内核预先分配好的, 而是需要动态分配的, 因此需要进行 TLB flush. 在 vmalloc 分配的内存释放(vfree)时会做 TLB flush. 这是一个全局内核 TLB flush, 在大多数架构上, 它是一个广播 IPI 到所有 CPU 来刷新缓存. 这都是在全局锁下完成的, 随着 CPU 数量的增加, 扩展后的工作负载需要执行的 vunmap() 数量也会增加, 全局 TLB 刷新的成本也会增加. 这导致了糟糕的二次可伸缩性问题.

2.6.28 时, Nick Piggin 重写了 vmalloc/vmap 分配器 [mm: vmap rewrite](https://lwn.net/Articles/304188). 其中为了
为提升 TLB flush 效率, vfree 流程中, 对于 TLB flush 操作, 采用 lazy 模式, 即: 先收集, 不真正释放, 当达到[限制(lazy_max_pages)时](https://elixir.bootlin.com/linux/v2.6.28/source/mm/vmalloc.c#L552), 再一起释放.


并为小型 vmap 提供快速, 可伸缩的 percpu 前端.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/07/28 | Nick Piggin <npiggin@suse.de> | [mm: vmap rewrite](https://lwn.net/Articles/304188) | 重写 vmap 分配器. 引入减轻 TLB flush 的影响, 引入了 lazy 模式. | RFC ☑ 2.6.28-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/118352)<br>*-*-*-*-*-*-*-* <br>[PatchWork](https://lore.kernel.org/lkml/20080728123438.GA13926@wotan.suse.de), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=db64fe02258f1507e13fe5212a989922323685ce) |
| 2016/04/15 | "Uladzislau Rezki (Sony)" <urezki@gmail.com> | [mm/vmalloc: Keep a separate lazy-free list](https://lore.kernel.org/patchwork/patch/669083) | 之前惰性释放的 vmap_area 也是放到 vmap_area_list 中, 但是使用 LAZY_FREE 标记. 每次处理时, 需要遍历整个 vmap_area_list, 然后将 LAZY_FREE 的节点添加到一个临时的 purge_list 中进行释放. 当混合使用大量 vmalloc 和 set_memory_*()(它调用vm_unmap_aliases())时, 触发了由于每次调用都要遍历整个 vmap_area_list 而导致性能严重下降的情况. 因此将惰性释放的 vmap_area 添加到一个单独的无锁空闲列表, 这样我们就不必在每次清除时遍历整个列表.  | v2 ☑ 4.7-rc1 | [PatchWork v1](https://lore.kernel.org/patchwork/patch/667471)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/669083)<br>*-*-*-*-*-*-*-* <br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=80c4bd7a5e4368b680e0aeb57050a1b06eb573d8) |
| 2016/11/18 | Christoph Hellwig <hch@lst.de> | [`Reduce latency in __purge_vmap_area_lazy V2`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=763b218ddfaf56761c19923beb7e16656f66ec62) | 1479474236-4139-11-git-send-email-hch@lst.de) | `__purge_vmap_area_lazy()` 中需要持有 vmap_area_lock, 这组补丁通过使用 cond_resched_lock() 避免持有 vmap_area_lock 时间过长. | v1 ☑✓ 4.10-rc1 | [LKML RFC](https://lkml.org/lkml/2016/10/18/47)br>*-*-*-*-*-*-*-* <br>[LORE v2,00/10](https://lore.kernel.org/all/1479474236-4139-1-git-send-email-hch@lst.de) |
| 2018/08/23 | "Uladzislau Rezki (Sony)" <urezki@gmail.com> | [minor mmu_gather patches](https://lore.kernel.org/patchwork/patch/976960) | NA | v1 ☑ 5.11-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/976960) |
| 2019/04/25 | Nadav Amit <namit@vmware.com> | [x86: text_poke() fixes and executable lockdowns](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=241a1f22380646bc4d1dd18e5bc246877513da68) | 最后 7 个补丁. 为 x86 实现了 ARCH_HAS_SET_DIRECT_MAP, 并添加一个新的标志 VM_FLUSH_RESET_PERMS, 允许 vfree 操作在释放页面之前立即清除可执行 TLB 条目, 并处理 directmap 上的重置权限. 这个标志对于任何具有提升权限的内存, 或者在 directmap 上可能有相关权限更改的内存都是有用的.<br>虽然现在可以直接释放非写内存, 但非写内存不能在中断中释放, 因为分配本身被用作延迟空闲列表上的一个节点. 因此, 当需要在中断中释放 RO 内存时, 执行 vfree 的代码需要有自己的工作队列, 就像延迟 vfree 列表添加到 vmalloc 之前的情况一样.<br>对于具有 set_direct_map 实现的架构, 当这样集中时, 整个操作可以通过一次 TLB 刷新完成. 对于其他具有 directmap 权限的, 目前只有 arm64, 一个使用 set_memory 函数的备份方法被用来重置 directmap. 当 arm64 增加 set_direct_map 时, 这个备份可以被删除.<br>当刷新 TLB 以同时删除 vmalloc 范围映射和直接映射权限的 TLB 条目时, 可以执行延迟清除操作, 以便稍后尝试保存 TLB 刷新. 但是现在 vm_unmap_aliases 可以刷新不包含 directmap 的 TLB 范围. 因此, 添加了一个带有额外参数的 helper 函数, 该参数允许在此操作期间刷新 vmalloc 地址和直接映射. vm_unmap_aliases 函数的行为没有改变. | v5 ☑✓ | [LORE v5,0/23](https://lore.kernel.org/all/20190426001143.4983-1-namit@vmware.com) |
| 2020/05/08 | Joerg Roedel <joro@8bytes.org> | [mm: Get rid of vmalloc_sync_(un)mappings()](https://lore.kernel.org/patchwork/patch/381217) | vmalloc_sync_mappings() 接口相关的一直存在着一些问题之后, 这里尝试修复了这些问题, 并删除了这些接口. 通过对 vmalloc() 和 ioremap() 添加了对页表目录更改的跟踪. 根据更改的页表级别, 会调用一个新的per-arch函数:arch_sync_kernel_mappings() 来进行 sync 操作. 这还带来了其他好处: [删除 x86_64 上 vmalloc 区域上的 page fault 处理](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7f0a002b5a21302d9f4b29ba83c96cd433ff3769), 因为 vmalloc() 现在负责同步系统中所有页表的更改. | v3 ☑ 5.8-rc1 | [PatchWork RFC,0/7](https://lore.kernel.org/patchwork/patch/1239082)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/7](https://lore.kernel.org/patchwork/patch/1241594)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3,0/7](https://lore.kernel.org/patchwork/patch/1242476) |
| 2020/08/14 | Joerg Roedel <joro@8bytes.org> | [x86: Retry to remove vmalloc/ioremap synchronzation](https://lore.kernel.org/patchwork/patch/1287505) | 删除同步 x86-64 的 vmalloc 和 ioremap 上页表同步的代码 arch_sync_kernel_mappings(). 页表页现在都是预先分配的, 因此不再需要同步. | v1 ☑ 5.10-rc1 | [PatchWork 0/2](https://lore.kernel.org/patchwork/patch/1287505)<br>*-*-*-*-*-*-*-* <br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=58a18fe95e83b8396605154db04d73b08063f31b) |


#### 2.4.1.4 per cpu vmap block
-------

[mm: vmap rewrite](https://lwn.net/Articles/304188) 新增了 per-cpu vmap block 缓存. 通过 vm_map_ram()/vm_unmap_ram() 显式调用. 当分配的空闲小于 VMAP_MAX_ALLOC 的时候, 将使用 per-cpu 缓存的 vmap block 来进行 vmap 映射.

vmap_block 是内核每次预先分配的 per-cpu vmap 缓存块, vmap_block_queue 链表维护了所有空闲 vmap_block 缓存.
每次分配的时候, 遍历 vmap_block_queue 查找到第一块满足要求(大小不小于待分配大小)的 vmap_block 块, 进行分配. 当链表中无空闲块或者所有的空闲块都无法满足当前分配需求的时候, 将通过 new_vmap_block 再缓存一块空闲块.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/07/28 | Nick Piggin <npiggin@suse.de> | [mm: vmap rewrite](https://lwn.net/Articles/304188) | 重写 vmap 分配器. 引入了 percpu 的 vmap_blocke_queue 来加速小于 VMAP_MAX_ALLOC 大小的 vmap 区域分配. 必须使用新增接口 vm_map_ram()/vm_unmap_ram() 来调用. | RFC ☑ 2.6.28-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/118352)<br>*-*-*-*-*-*-*-* <br>[PatchWork](https://lore.kernel.org/patchwork/patch/124065), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=db64fe02258f1507e13fe5212a989922323685ce) |
| 2009/01/07 | MinChan Kim <minchan.kim@gmail.com> | [Remove needless lock and list in vmap](https://lore.kernel.org/patchwork/patch/139998) | vmap 的 dirty_list 未使用. 这是为了优化 TLB flush. 但尼克还没写代码. 所以, 我们在需要的时候才需要它. 这个补丁删除了 vmap_block 的 dirty_list 和相关代码. | v1 ☑ 2.6.30-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/139998)<br>*-*-*-*-*-*-*-* <br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d086817dc0d42f1be8db4138233d33e1dd16a956) |
| 2010/02/01 | Nick Piggin <npiggin@suse.de> | [mm: purge fragmented percpu vmap blocks](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=02b709df817c0db174f249cc59e5f7fd01b64d92) | 引入 purge_fragmented_blocks_allcpus() 来释放 percpu 的 vmap block 缓存(借助了 lazy 模式). 在此之前, 内核不会释放 per-cpu 映射, 直到它的所有地址都被使用和释放. 所以碎片块可以填满 vmalloc 空间, 即使它们实际上内部没有活动的 vmap 区域. 这解决了 Christoph 报告的在 XFS 中使用 percpu vmap api 时出现的一些 vmap 分配失败的问题. | v1 ☑ 2.6.33-rc7 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=02b709df817c0db174f249cc59e5f7fd01b64d92) |
| 2020/08/06 | Matthew Wilcox (Oracle) <willy@infradead.org> | [vmalloc: convert to XArray](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0f14599c607d32512a1d37e6d2a2d1a867f16177) | 把 vmap_blocks 组织结构从 radix 切换到 XArray. | v1 ☑ 5.9-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0f14599c607d32512a1d37e6d2a2d1a867f16177) |
| 2008/07/28 | Nick Piggin <npiggin@suse.de> | [mm/vmalloc: fix possible exhaustion of vmalloc space](https://lore.kernel.org/patchwork/patch/553114) | 修复 vm_map_ram 分配器引发的高度碎片化: vmap_block 有空闲空间, 但仍然会出现新的块. 频繁的使用 vm_map_ram()/vm_unmap_ram() 去映射/解映射会非常快的耗尽 vmalloc 空间. 在小型 32 位系统上, 这不是一个大问题, 在第一次分配失败(alloc_vmap_area)时将很快调用 cause 清除, 但在 64 位机器上, 例如 x86_64 有 45 位 vmalloc 空间, 这可能是一场灾难.  | RFC ☑ 2.6.28-rc1 | [PatchWork RFC,0/3](https://lore.kernel.org/patchwork/patch/550979)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC,v2,0/3](https://lore.kernel.org/patchwork/patch/553114) |
| 2008/07/28 | Nick Piggin <npiggin@suse.de> | [mm, vmalloc: cleanup for vmap block](https://lore.kernel.org/patchwork/patch/384995) | 删除了 vmap block 中的一些死代码和不用代码. 其中 vb_alloc() 中 vmap_block 中如果有足够空间是必然分配成功的, 因此删除了 bitmap_find_free_region() 相关的 purge 代码. | RFC ☑ 2.6.28-rc1 | [PatchWork 0/3](https://lore.kernel.org/patchwork/patch/384995)<br>*-*-*-*-*-*-*-* <br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3fcd76e8028e0be37b02a2002b4f56755daeda06) |


https://lore.kernel.org/patchwork/patch/616606/
https://lore.kernel.org/patchwork/patch/164572/

commit f48d97f340cbb0c323fa7a7b36bd76a108a9f49f
Author: Joonsoo Kim <iamjoonsoo.kim@lge.com>
Date:   Thu Mar 17 14:17:49 2016 -0700

    mm/vmalloc: query dynamic DEBUG_PAGEALLOC setting



#### 2.4.1.5 other vmalloc interface
-------

vread 用于读取指定地址的内存数据.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 1993/12/28 | Linus Torvalds <torvalds@linuxfoundation.org> | [Linux-0.99.14 (November 28, 1993)](https://elixir.bootlin.com/linux/0.99.14/source/mm/vmalloc.c#L168) | 引入 vread. | ☑ 0.99.14 | [HISTORY commit](https://github.com/gatieme/linux-history/commit/7e8425884852b83354ab090a07715c6c32918f37) |


vwrite 则用于对指定的 vmalloc 区域进行写操作.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| NA | Marc Boucher | [Support /dev/kmem access to vmalloc space (Marc Boucher)](https://elixir.bootlin.com/linux/2.4.17/source/mm/vmalloc.c#L168) | 引入 vwrite | ☑ 2.4.17 | [HISTORY commit](https://github.com/gatieme/linux-history/commit/a3245879f664fb42b1903bc98af670da6d783db5) |
| 2021/05/06 | David Hildenbrand <david@redhat.com> | [mm/vmalloc: remove vwrite()](https://lore.kernel.org/patchwork/patch/1401594) | 删除 vwrite | v1 ☑ 5.13-rc1 | [PatchWork v1,3/3](https://lore.kernel.org/patchwork/patch/1401594)<br>*-*-*-*-*-*-*-* <br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f7c8ce44ebb113b83135ada6e496db33d8a535e3) |

vmalloc_to_page 则提供了通过 vmalloc 地址查找到对应 page 的操作.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/02/19 | Ingo Molnar <mingo@elte.hu> | [vmalloc_to_page helper](https://github.com/gatieme/linux-history/commit/094686d30d5f58780a1efda2ade5cb0d18e25f82) | NA | ☑ 3.11-rc1 | [commit1 HISTORY](https://github.com/gatieme/linux-history/commit/094686d30d5f58780a1efda2ade5cb0d18e25f82), [commit2 HISTORY](https://github.com/gatieme/linux-history/commit/e1f40fc0cf09f591b474a0c17fc4e5ca7438c7dd) |

对于 vmap_pfn

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/05/04 | Jeremy Fitzhardinge <jeremy@goop.org> | [xen: Xen implementation for paravirt_ops](https://lore.kernel.org/patchwork/patch/80500) | NA | ☑ 2.6.22-rc1 | [PatchWork 0/11](https://lore.kernel.org/patchwork/patch/80500), [关注 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=aee16b3cee2746880e40945a9b5bff4f309cfbc4) |
| 2020/10/02 | Christoph Hellwig <hch@lst.de> | [mm: remove alloc_vm_area and add a vmap_pfn function](https://lore.kernel.org/patchwork/patch/1316291) | NA | ☑ 5.10-rc1 | [PatchWork 0/11](https://lore.kernel.org/patchwork/patch/1316291) |


#### 2.4.1.6 huge vmap
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/03/03 | Toshi Kani <toshi.kani@hp.com> | [Kernel huge I/O mapping support](https://lore.kernel.org/patchwork/patch/547056) | ioremap() 支持透明大页. 扩展了 ioremap() 接口, 尽可能透明地创建具有大页面的 I/O 映射. 当一个大页面不能满足请求范围时, ioremap() 继续使用 4KB 的普通页面映射. 使用 ioremap() 不需要改变驱动程序. 但是, 为了使用巨大的页面映射, 请求的物理地址必须以巨面大小(x86上为 2MB 或 1GB)对齐. 内核巨页的 I/O 映射将提高 NVME 和其他具有大内存的设备的性能, 并减少创建它们映射的时间. | v3 ☑ 4.1-rc1 | [PatchWork v3,0/6](https://lore.kernel.org/patchwork/patch/547056) |
| 2021/03/17 | Nicholas Piggin <npiggin@gmail.com> | [huge vmalloc mappings](https://lore.kernel.org/patchwork/patch/1397495) | 支持大页面 vmalloc 映射. 配置选项 HAVE_ARCH_HUGE_VMALLOC 支持定义 HAVE_ARCH_HUGE_VMAP 的架构, 并支持 PMD 大小的 vmap 映射. 如果分配 PMD 大小或更大的页面, vmalloc 将尝试分配 PMD 大小的页面, 如果不成功, 则返回到小页面. 架构必须确保任何需要 PAGE_SIZE 映射的 arch 特定的 vmalloc 分配(例如, 模块分配与严格的模块rwx) 使用 VM_NOHUGE 标志来禁止更大的映射. 对于给定的分配, 这可能导致更多的内部碎片和内存开销, nohugevmalloc 选项在引导时被禁用. | v13 ☑ [5.13-rc1](https://kernelnewbies.org/Linux_5.13#Memory_management) | [PatchWork v13,00/14](https://patchwork.kernel.org/project/linux-mm/cover/20210317062402.533919-1-npiggin@gmail.com) |
| 2021/05/12 | Christophe Leroy <christophe.leroy@csgroup.eu> | [Implement huge VMAP and VMALLOC on powerpc 8xx](https://lore.kernel.org/patchwork/patch/1425630) | 本系列在 powerpc 8xx 上实现了大型 VMAP 和 VMALLOC. Powerpc 8xx 有 4 个页面大小: 4k, 16k, 512k, 8M. 目前, vmalloc() 和 vmap() 只支持 PMD 级别的大页. 这里的 PMD 级别是 4M, 它不对应于任何受支持的页面大小. 现在, 实现 16k 和 512k 页的使用, 这是在PTE级别上完成的. 对 8M 页面的支持将在以后实现, 这需要使用 gepd表. 为了实现这一点, 该体系结构提供了两个功能: <br>1. arch_vmap_pte_range_map_size() 告诉 vmap_pte_range() 使用什么页面大小. <br>2. arch_vmap_pte_supported_shift() 告诉 `__vmalloc_node_range()` 对于给定的区域大小使用什么分页移位.<br>当体系结构不提供这些函数时, 将返回PAGE_SHIFT. | v2 ☑ 5.14-rc1 | [PatchWork v2,0/5](https://lore.kernel.org/patchwork/patch/1425630) |
| 2021/12/26 | Kefeng Wang <wangkefeng.wang@huawei.com> | [mm: support huge vmalloc mapping on arm64/x86](https://patchwork.kernel.org/project/linux-mm/cover/20211226083912.166512-1-wangkefeng.wang@huawei.com) | NA | v1 ☐ | [PatchWork 0/3](https://patchwork.kernel.org/project/linux-mm/cover/20211226083912.166512-1-wangkefeng.wang@huawei.com) |
| 2022/04/11 | Song Liu <song@kernel.org> | [vmalloc: bpf: introduce VM_ALLOW_HUGE_VMAP](https://lore.kernel.org/all/20220411231808.667073-1-song@kernel.org) | 在 x86_64 上启用 HAVE_ARCH_HUGE_VMALLOC 并将其用于 bpf_prog_pack 会导致[一些问题](https://lore.kernel.org/lkml/20220204185742.271030-1-song@kernel.org), 因为许多 vmalloc 的用户还没有准备好处理巨大的页面. 为了能够更平稳地过渡到使用大页面支持的 vmalloc 内存, 这个集合将 VM_NO_HUGE_VMAP 标志替换为一个新的可选标志 VM_ALLOW_HUGE_VMAP. 更多[相关的讨论可以参照](https://lore.kernel.org/linux-mm/20220330225642.1163897-1-song@kernel.org).<br>1. 删除 VM_NO_HUGE_VMAP, 引入 VM_ALLOW_HUGE_VMAP 逻辑上更简洁.<br>2. 实现了 module_alloc_huge(), <br>3. 实现使用 bpf_prog_pack 中的 VM_ALLOW_HUGE_VMAP. | v2 ☐☑✓ | [LORE v2,0/3](https://lore.kernel.org/all/20220411231808.667073-1-song@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/4](https://lore.kernel.org/r/20220413153340.326834-1-song@kernel.org), [LORE v3,0/4](https://lore.kernel.org/r/20220414195914.1648345-1-song@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/4](https://lore.kernel.org/r/20220415164413.2727220-1-song@kernel.org) |


#### 2.4.1.7 vmallocinfo
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/05/02 | Michal Hocko <mhocko@kernel.org> | [mm, vmalloc: properly track vmalloc users](https://lore.kernel.org/patchwork/patch/784732) | 在 `/proc/vmallocinfo` 中显示 lazy 释放的 vm_area. | v2 ☑ 4.13-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/784732)<br>*-*-*-*-*-*-*-* <br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1f5307b1e094bfffa83c65c40ac6e3415c108780) |
| 2017/06/05 | Yisheng Xie <xieyisheng1@huawei.com> | [vmalloc: show lazy-purged vma info in vmallocinfo](https://lore.kernel.org/patchwork/patch/795284) | 在 `/proc/vmallocinfo` 中显示 lazy 释放的 vm_area. | v2 ☑ 4.13-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/795284)<br>*-*-*-*-*-*-*-* <br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=78c72746f56b212ecf768a7e67cee3b7cf89238c) |


#### 2.4.1.8 其他 vmalloc 补丁
-------

根据最近与 Dave 和 Neil 的讨论 [congestion_wait() and GFP_NOFAIL](http://lkml.kernel.org/r/163184741778.29351.16920832234899124642.stgit@noble.brown), 需要尝试实现对 vmalloc 的 NOFS, NOIO, NOFAIL 支持,  以使 kvmalloc 用户的使用更方便.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/22 | Alistair Popple <apopple@nvidia.com> | [extend vmalloc support for constrained allocations](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a421ef303008b0ceee2cfc625c3246fa7654b0ca) | 扩展 vmalloc 对受限分配的支持.<br>第一个补丁为 vmalloc 实现 NOFS/NOIO支持. 第二个补丁增加了 NOFAIL 支持, 第三个补丁将所有支持打包到 kvmalloc 中, 并删除了现在可以直接使用 kvmalloc 的 ceph_kvmalloc. | v2 ☑ 5.17-rc1 | [2021/10/18 PatchWork RFC,0/3](https://patchwork.kernel.org/project/linux-mm/cover/20211018114712.9802-1-mhocko@kernel.org)<br>*-*-*-*-*-*-*-* <br>[2021/10/25 PatchWork 0/4](https://patchwork.kernel.org/project/linux-mm/cover/20211025150223.13621-1-mhocko@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/4](https://lore.kernel.org/all/20211122153233.9924-1-mhocko@kernel.org) |
| 2022/01/19 | "Uladzislau Rezki (Sony)" <urezki@gmail.com> | [mm/vmalloc: Move draining areas out of caller context](https://patchwork.kernel.org/project/linux-mm/patch/20220119143540.601149-1-urezki@gmail.com) | NA | v1 ☐ | [PatchWork 1/3](https://patchwork.kernel.org/project/linux-mm/patch/20220119143540.601149-1-urezki@gmail.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3,0/1](https://lore.kernel.org/r/20220131144058.35608-1-urezki@gmail.com) |
| 2022/01/27 | Christophe Leroy <christophe.leroy@csgroup.eu> | [Allocate module text and data separately](https://patchwork.kernel.org/project/linux-mm/cover/cover.1643282353.git.christophe.leroy@csgroup.eu/) | 本系列允许架构将 module 的数据放在 vmalloc 区域而不是模块区域. 在 powerpc book3s/32 的机器上, 了设置数据非可执行性, 这是必需的, 因为不可能以页面为基础设置可执行性, 这是每256 mb的段执行一次. 模块区有 exec 权, vmalloc 区则没有. 没有这个更改模块, 即使开启了 CONFIG_STRICT_MODULES_RWX, 模块数据仍然是可执行的. 这在其他 powerpc/32 上也很有用, 可以最大限度地增加代码接近内核的机会,  从而避免使用 PLT 和 trampoline 等. | v2 ☐☑ | [PatchWork v2,0/5](https://lore.kernel.org/r/cover.1643282353.git.christophe.leroy@csgroup.eu)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3,0/6](https://lore.kernel.org/r/cover.1643475473.git.christophe.leroy@csgroup.eu) |
| 2022/03/08 | Paolo Bonzini <pbonzini@redhat.com> | [mm: vmalloc: introduce array allocation functions](https://patchwork.kernel.org/project/linux-mm/cover/20220308105918.615575-1-pbonzini@redhat.com/) | 实现了四个数组分配函数来替换 vmalloc(array_size()) 和 vzalloc (array_size()), Linux  中当前有几十个这样的函数.  函数负责乘法和溢出检查, 特别混乱, 作者这样实现后这样代码更清晰, 并使开发人员更容易避免溢出错误. | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/20220308105918.615575-1-pbonzini@redhat.com) |


### 2.4.2 连续内存分配器(下·)
-------

物理上连续的大页面对于 gpu、fpga、nic 和 RDMA 控制器等设备非常重要, 因为它们在大页面上操作时通常可以获得更好的性能. 当然, CPU 的性能也是如此, 但是有一个重要的区别:

gpu 和高吞吐量设备在 TLB 丢失和随后的页表遍行情况下, 与 CPU 相比, 通常会遭受更严重的性能打击. 这种影响非常大, 以至于这些设备迫切想要一种高度可靠的方式来分配大页面, 以最小化潜在的 TLB 遗漏数量和诱导页表遍历所花费的时间.

因此各个厂商(如 Oracle、Mellanox、IBM、NVIDIA 等)对生成超过 THP 大小的物理连续内存都非常感兴趣, 并寻找解决方案.

[Improving support for large, contiguous allocations](https://lwn.net/Articles/753167)

#### 2.4.2.1 CMA
-------

**3.5(2012年7月发布)**

[Linux中的Memory Compaction [二] - CMA](https://zhuanlan.zhihu.com/p/105745299)

[Contiguous memory allocation for drivers](https://lwn.net/Articles/396702)

[A deep dive into CMA](https://lwn.net/Articles/486301)

[CMA and compaction](https://lwn.net/Articles/684611)

[A reworked contiguous memory allocator](https://lwn.net/Articles/447405)

[linux 内核CMA笔记](https://blog.csdn.net/zjy900507/article/details/105328190)


连续内存的分配需求来自形形色色的驱动. 例如现在大家的手机都有视频功能, camer 功能, 这类驱动都需要非常大块的内存, 而且有 DMA 用来进行外设和大块内存之间的数据交换. 对于嵌入式设备, 一般不会有 IOMMU, 而且 DMA 也不具备 scatter-getter 功能, 这时候, 驱动分配的大块内存(DMA buffer)必须是物理地址连续的.

通过内存管理系统分配大且物理地址空间连续的的内存, 压力可是不小. 当然, 在系统启动之处, 伙伴系统中的大块内存比较大, 也许分配大块内存 不算什么, 但是随着系统的运行, 内存不断的分配、释放, 大块内存不断的裂解, 再裂解, 这时候, 内存碎片化导致分配地址连续的大块内存变得不是那么的容易了, 怎么办?

通常大家可能有两个选择:

1.  一是在启动时分配为驱动预留足够的 DMA buffer 空间, 这种方式非常可靠的, 当设备使用 DMA buffer 时就能立即使用, 不会有延时. 但它有一个缺点, 即当设备不使用时, 预留的那些 DMA BUFFER 的内存实际上是浪费了.

2.  另外一个方案是当实际使用设备的时候分配 DMA buffer. 不会浪费内存, 但是不可靠, 随着内存碎片化, 大的、连续的内存分配变得越来越困难, 一旦内存分配失败, 设备分配内存会有极大的延迟, 甚至可能分配失败, 导致设备功能不可用, 这种情况下用户也不会答应.

这就是驱动工程师面临的困境, 为了解决这个问题, 各个驱动各出奇招, 但是都不能非常完美的解决问题. 最终来自Michal Nazarewicz 的 CMA 将可以把各个驱动工程师的烦恼"一洗了之". 对于 CMA 内存, 当前驱动没有分配使用的时候, 这些 memory 可以内核的被其他的模块使用(当然有一定的要求), 而当驱动分配 CMA 内存后, 那些被其他模块使用的内存需要吐出来, 形成物理地址连续的大块内存, 给具体的驱动来使用.

因此 DMA 设备对 CMA 区域有优先使用权, 被称为 primary client, 而其他模块的页面则是 secondary client.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/02/28 | Mel Gorman | [Introduce ZONE_CMA](https://lore.kernel.org/patchwork/patch/778794) | ZONE_CMA 的实现方案, 使用新的分区不仅可以有H/W 寻址限制, 还可以有 S/W 限制来保证页面迁移. | v7 ☐ | [PatchWork v7](https://lore.kernel.org/patchwork/patch/778794) |
| 2007/02/28 | Mel Gorman | [mm/cma: manage the memory of the CMA area by using the ZONE_MOVABLE](https://lore.kernel.org/patchwork/patch/857428) | 新增了 ZONE_CMA 区域, 使用新的分区不仅可以有H/W 寻址限制, 还可以有 S/W 限制来保证页面迁移.  | v2 ☐ | [PatchWork v7](https://lore.kernel.org/patchwork/patch/857428) |
| 2010/11/19 | Mel Gorman | [big chunk memory allocator v4](https://lore.kernel.org/patchwork/patch/224757) | 大块内存分配器 | v4 ☐ | [PatchWork v4](https://lore.kernel.org/patchwork/patch/224757) |
| 2012/04/03 | Michal Nazarewicz <m.nazarewicz@samsung.com><br>*-*-*-*-*-*-*-* <br>Marek Szyprowski <m.szyprowski@samsung.com> | [Contiguous Memory Allocator](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=58f42fd54144346898e6dc6d6ae3acd4c591b42f) | 实现 CMA, 参见 [LWN](https://lwn.net/Articles/486301) | v24 ☑ [3.5-rc1](https://kernelnewbies.org/Linux_3.5#Memory_Management) | [PatchWork v7](https://lore.kernel.org/patchwork/patch/229177)<br>*-*-*-*-*-*-*-* <br>[PatchWork v24](https://lore.kernel.org/patchwork/patch/295656) |
| 2015/02/12 | Joonsoo Kim <iamjoonsoo.kim@lge.com> | [mm/compaction: enhance compaction finish condition](https://lore.kernel.org/patchwork/patch/542063) | 同样的, 之前 NULL 指针和错误指针的输出也很混乱, 进行了归一化. | v1 ☑ 4.1-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/542063)<br>*-*-*-*-*-*-*-* <br>[关键 commit 2149cdaef6c0](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2149cdaef6c0eb59a9edf3b152027392cd66b41f) |
| 2015/02/23 | SeongJae Park <sj38.park@gmail.com> | [introduce gcma](https://lore.kernel.org/patchwork/patch/544555) | [GCMA(Guaranteed Contiguous Memory Allocator)方案](http://ceur-ws.org/Vol-1464/ewili15_12.pdf), 倾向于使用 writeback 的page cache 和 完成 swap out 的 anonymous pages 来做 seconday client, 进行迁移. 从而确保 primary client 的分配. | RFC v2 ☐ | [PatchWork](https://lore.kernel.org/patchwork/patch/544555), [GitHub](https://github.com/sjp38/linux.gcma/releases/tag/gcma/rfc/v2) |
| 2021/03/02 | Minchan Kim <minchan@kernel.org> | [mm: vmstat: add cma statistics](https://lore.kernel.org/all/20210302183346.3707237-1-minchan@kernel.org) | 将 CMA 分配统计信息输出到 vmstat, 从而可以使用户知道系统使用 CMA 分配成功 (CMA_ALLOC_SUCCESS) 和失败 (CMA_ALLOC_SUCCESS) 的次数和频率. | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20210302183346.3707237-1-minchan@kernel.org) |
| 2022/04/24 | lipeifeng@oppo.com <lipeifeng@oppo.com> | [mm/page_alloc: give priority to free cma-pages from pcplist to buddy](https://patchwork.kernel.org/project/linux-mm/patch/20220424032734.1542-1-lipeifeng@oppo.com/) | 在许多情况下, cma pages 将回退到可移动页面. 当 cma 页面被释放给 pcplist 时, 我们优先将其从 pcplist 释放给 buddy, 以避免 cma 页面在有足够的自由移动页面时很快被用作可移动页面, 这样可以在 buddy 中节省更多的 cma 页面, 从而减少 cma_alloc 时的页面迁移. | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220424032734.1542-1-lipeifeng@oppo.com) |


#### 2.4.2.2 contiguous pages
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/08/02 | Mike Kravetz <mike.kravetz@oracle.com> | [Add mmap(MAP_CONTIG) support](https://lwn.net/Articles/736170) | 当以较高的阶数应用回收时, 可能会启动大量IO. 这组补丁尝试修复这个问题, 用于在 VM 事件记录器中中将页面标记为非活动时修复, 并在直接回收连续区域时等待页面写回. | v3 ☑ 2.6.23-rc4 | [Patchwork V5](https://lore.kernel.org/patchwork/patch/87667) |
| 2019/02/15 | Zi Yan <zi.yan@sent.com> | [Generating physically contiguous memory after page allocation](https://lwn.net/Articles/779979) | 这个补丁集通过移动正在使用的页面而不分配任何新页面来产生物理上连续的内存. 与在页面分配时分配物理上连续的内存相比, 这个补丁集提供了一种替代方法, 可以在页面分配后生成物理上连续的内存. 这种方法可以避免在页面分配过程中发生的页面回收和内存压缩, 但仍然产生相当的物理上连续的内存. 使用这个补丁集, 我们可以生成比 PMD 级别的 THP 更大的页面, 而不需要在  buddy allocator 中更改MAX_ORDER. 它的目标是两个场景:<br>1. 当系统处于内存压力下时, 避免页面回收和内存压缩, 因为这个补丁集不分配任何新页面<br>2. 生成大于 2^MAX_ORDER 的页面, 而不更改 buddy allocator.<br>为了演示它的使用, 作者添加了非常基本的 1GB THP支持, 并在补丁集中将 512 2MB THP 提升为 1GB THP. 还实现了将 512 个 4KB 页面升级到到 2MB THP. 它只适用于可移动页面, 因此它也面临与内存压缩相同的碎片问题, 即, 如果不可移动页面分布在整个内存中, 那么这个补丁集只能在任何两个不可移动页面之间生成连续性. 这个补丁集有三个组件:<br>1. 一个新的页面迁移机制, 称为交换页面, 它交换两个正在使用的页面的内容, 而不是执行两个背靠背的页面迁移. 它节省了开销, 并避免了页面分配路径中的页面回收和内存压缩, 不过如果系统中有足够的空闲内存, 则并不严格要求这样做.<br>2. 一个新的机制, 它利用页面迁移和交换页面来生成物理上连续的内存/任意大小的页面, 而不需要分配任何新页面, 这与khugepage所做的不同. 它在每个VMA的基础上工作, 从每个VMA中创建物理上连续的内存, 而这些内存实际上是连续的.<br>3. 一个新的物理连续内存产生机制的用例, 通过迁移和交换页面, 并将512个连续2MB THP提升为1GB THP, 尽管可以生成更大的物理连续内存范围. 1GB THP实现是非常基本的, 当buddy allocator被修改为分配1GB页面时, 它可以处理1GB THP故障, 支持1GB THP分裂到2MB THP, 并支持从2MB THP就地提升到1GB THP, 以及PMD/ pte映射1GB THP. 这些都没有经过充分的测试. | v7 ☐ | [PatchWork RFC,00/31](https://patchwork.kernel.org/project/linux-mm/cover/20190215220856.29749-1-zi.yan@sent.com) |
| 2022/03/11 | Zi Yan <zi.yan@sent.com> | [Use pageblock_order for cma and alloc_contig_range alignment.](https://patchwork.kernel.org/project/linux-mm/cover/20220311183656.1911811-1-zi.yan@sent.com) | 622757 | v7 ☐☑ | [LORE v7,0/5](https://lore.kernel.org/r/20220311183656.1911811-1-zi.yan@sent.com) |


# 3 内存去碎片化
-------

[由 Linux 的内存碎片问题说开来](https://zhuanlan.zhihu.com/p/351780620)

[一张图读懂内存反碎片化(OPPO ColorOS 内存反碎片化引擎)](https://blog.csdn.net/21cnbao/article/details/105172435)

## 3.1 关于碎片化
-------

内存按 chunk 分配, 每个程序保留的 chunk 的大小和时间都不同. 一个程序可以多次请求和释放 `memory chunk`. 程序一开始时, 空闲内存有很多并且连续, 随后大的连续的内存区域碎片化, 变成更小的连续区域, 最终程序无法获取大的连续的 memory chunk.


| 碎片化类型 | 描述 | 示例 |
|:--------:|:----:|:---:|
| 内碎片化(Internal fragmentation) | 分给程序的内存比它实际需要的多, 多分的内存被浪费. | 比如chunk一般是4, 8或16的倍数, 请求23字节的程序实际可以获得24字节的chunk, 未被使用的内存无法再被分配, 这种分配叫fixed partitions. 一个程序无论多么小, 都要占据一个完整的partition. 通常最好的解决方法是改变设计, 比如使用动态内存分配, 把内存空间的开销分散到大量的objects上, 内存池可以大大减少internal fragmentation. |
| 外部碎片(External fragmentation) | 有足够的空闲内存, 但是没有足够的连续空闲内存供分配. | 因为都被分成了很小的pieces, 每个piece都不足以满足程序的要求. external指未使用的存储空间在已分配的区域外. 这种情况经常发生在频繁创建、更改(大小)、删除不同大小文件的文件系统中. 比起文件系统, 这种fragmentation在RAM上更是一个问题, 因为程序通常请求RAM分配一些连续的blocks, 而文件系统可以利用可用的blocks并使得文件逻辑上看上去是连续的. 所以对于文件系统来说, 有空闲空间就可以放新文件, 碎片化也没关系, 对内存来说, 程序请求连续blocks可能无法满足, 除非程序重新发出请求, 请求一些更小的分散的blocks. 解决方法一是compaction, 把所有已分配的内存blocks移到一块, 但是比较慢, 二是garbage collection, 收集所有无法访问的内存并把它们当作空闲内存, 三是paging, 把物理内存分成固定大小的frames, 用相同大小的逻辑内存pages填充, 逻辑地址不连续, 只要有可用内存, 进程就可以获得, 但是paging又会造成internal fragmentation. |
| Data fragmentation. | 当内存中的一组数据分解为不连续且彼此不紧密的许多碎片时, 会发生这种类型的碎片. 如果我们试图将一个大对象插入已经遭受的内存中, 则会发生外部碎片 .  | 通常发生在向一个已有external fragmentation的存储系统中插入一个大的object的时候, 操作系统找不到大的连续的内存区域, 就把一个文件不同的blocks分散放置, 放到可用的小的内存pieces中, 这样文件的物理存放就不连续, 读写就慢了, 这叫文件系统碎片. 消除碎片工具的主要工作就是重排block, 让每个文件的blocks都相邻. |

参考 [Fragmentation in Operating System](https://www.includehelp.com/operating-systems/fragmentation.aspx)


*   [Linux Kernel vs. Memory Fragmentation (Part I)](https://en.pingcap.com/blog/linux-kernel-vs-memory-fragmentation-1)

*   [Linux Kernel vs. Memory Fragmentation (Part II)](https://en.pingcap.com/blog/linux-kernel-vs-memory-fragmentation-2)

前面讲了运行较长时间的系统存在的内存碎片化问题, Linux 内核也不能幸免, 因此有开发者陆续提出若干种方法.



## 3.2 成块回收(Lumpy Reclaim)
-------


**2.6.23引入(2007年7月), 3.5移除(2012年7月)**

这不是一个完整的解决方案, 它只是缓解这一问题. 所谓回收是指 MM 在分配内存遇到内存紧张时, 会把一部分内存页面回收. 而[成块回收](https://lwn.net/Articles/211199), 就是尝试成块回收目标回收页相邻的页面, 以形成一块满足需求的高阶连续页块. 这种方法有其局限性, 就是成块回收时没有考虑被连带回收的页面可能是"热页", 即被高强度使用的页, 这对系统性能是损伤.


*   引入 Lumpy Reclaim


当我们没有合适大小的内存时, 我们进入回收. 当前的回收算法以 LRU 顺序定位页面, 这对于 order-0 的公平性非常好, 但如果希望分配更好的 order, 则非常不合适.

之前为了获得更高阶的页面, 我们必须回收非常高比例的内存. 因此 v2.6.23-rc1 在内存分配器添加了一个块状回收算法 [Lumpy Reclaim V4](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5ad333eb66ff1e52a87639822ae088577669dcf9). 它以指定的顺序定位在活动列表和非活动列表末尾的页面组. 这鼓励按请求顺序的页面组从活跃列表移动到不活跃列表, 并从活跃列表移动到自由列表. 这种行为只在直接回收时被触发, 当更高的订单页面已被请求. 当使用防碎片方案时, 这个方案特别有效, 将具有相似可回收性的页面分组在一起.

当块块回收与 ZONE_MOVABLE 一起使用时, 效率更高. 在一台拥有 2GB RAM 的桌面机器上进行的测试表明, 使用 ZONE_MOVABLE 自己增加巨大的页面池非常缓慢, 因为成功率非常低. 如果没有块回收, 每次尝试将池增加 100 个页面将产生 1 到 2 个巨大的页面. 使能了成块回收, 每次尝试都能获得 40 到 70 个巨大的页面.

引入了 PAGE_ALLOC_COSTLY_ORDER, 默认为 3, 内核认为超过 order 3 的分配所花费的开销都很大. 也就是说, 当一次内存申请小于或等于 2^3 = 8 个 pages 时, 通常是容易得到满足的, 而大于 8 个就是比较 "costly" 的操作. 这同时也是在提醒开发者, 最好不要一次申请超过 8 个连续的 page frames.


在 isolate_lru_pages() 中尝试获取标记页周围按顺序排列区域中的所有页面. 只取那些与标记页具有相同 LRU 活动状态的页. 我们可以安全地c从目标页面 pfn 上下获取到所请求的 order 大小的页面.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/04/20 | Andy Whitcroft <apw@shadowen.org> | [Lumpy Reclaim V4](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5ad333eb66ff1e52a87639822ae088577669dcf9) | 实现成块回收, 其中引入了 PAGE_ALLOC_COSTLY_ORDER, 该值虽然是经验值, 但是通常被认为是介于系统有/无回收页面压力的一个临界值, 一次分配低于这个 order 的页面, 通常是容易满足的. 而大于这个 order 的页面, 被认为是 costly 的. | v6 ☑ 2.6.23-rc1 | [Patchwork V5](https://lore.kernel.org/patchwork/patch/76206)<br>*-*-*-*-*-*-*-* <br>[PatchWork v8](https://lore.kernel.org/patchwork/patch/78996)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6 cleanup](https://lore.kernel.org/patchwork/patch/79316)<br>*-*-*-*-*-*-*-* <br>[COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5ad333eb66ff1e52a87639822ae088577669dcf9) |
| 2008/07/01 | Mel Gorman <mel@csn.ul.ie> | [Reclaim page capture v1](https://lore.kernel.org/lkml/1214935122-20828-1-git-send-email-apw@shadowen.org/) | 大 order 页面分配的有一次探索. 这种方法是捕获直接回收中释放的页面, 以便在空闲页面被竞争的分配程序重新分配之前增加它们被压缩的机会. | v1 ☐  | [Patchwork V5](https://lore.kernel.org/lkml/1214935122-20828-1-git-send-email-apw@shadowen.org) |
| 2007/08/02 | Andy Whitcroft <apw@shadowen.org> | [Synchronous Lumpy Reclaim V3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=c661b078fd62abe06fd11fab4ac5e4eeafe26b6d) | 当以较高的阶数应用回收时, 可能会启动大量IO. 这组补丁尝试修复这个问题, 用于在 VM 事件记录器中中将页面标记为非活动时修复, 并在直接回收连续区域时等待页面写回. | v3 ☑ 2.6.23-rc4 | [Patchwork v3,0/2](https://lore.kernel.org/lkml/exportbomb.1186077923@pinky) |
| 2010/09/06 | Mel Gorman <mel@csn.ul.ie> | [Reduce latencies and improve overall reclaim efficiency](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=08fc468f4eaf6683bae5bdb94743a09d8630cb80) | 成块回收过于激进, 会在 LRU 系统造成一定的破坏. 由于 SLUB 使用高阶分配, 块状回收产生的巨大成本将是显而易见的. 这些补丁应该可以在不禁用 Lumpy Reclaim 的情况下缓解该问题. 引入 lumpy_mode, 减少成块回收过程中的等待和延迟. | v2 ☑✓ 2.6.37-rc1 | [LORE v1,0/9](https://lore.kernel.org/all/1283770053-18833-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/8](https://lore.kernel.org/lkml/1284553671-31574-1-git-send-email-mel@csn.ul.ie) |


*   使用内存规整替代 Lumpy Reclaim

成块回收(Lumpy Reclaim) 并不是一个很好的解决问题的办法, 只能一定程度缓解页面碎片化问题.

首先它粗暴地回收目标区域附近的页面, 动作非常粗暴, 这可能耗时非常长, 造成严重的阻塞.

其次它并不考虑 LRU 上 active 和 inactive 的比例和老化问题, 这对 LRU 系统造成了严重的破坏.

而相比较, 内存规整效率更高, 是一个不错的替代的成块回收的操作. 因此 v2.6.38 [commit 3e7d34497067 ("mm: vmscan: reclaim order-0 and use compaction instead of lumpy reclaim")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3e7d344970673c5334cf7b5bb27c8c0942b06126) 引入了一个称为(先)回收(后)规整(`reclaim/compaction`) 的方法来替代成块回收. 不再像成块回收那样选择一个连续的页面范围来回收, 而是先回收大量的 order-0 页面, 然后通过[直接规整(`__alloc_pages_direct_compact()`)](https://elixir.bootlin.com/linux/v2.6.38/source/mm/page_alloc.c#L2148) 进行碎片化整理, 从而规整出足够连续页面供高阶分配.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2010/11/22 | Mel Gorman <mel@csn.ul.ie> | [Use memory compaction instead of lumpy reclaim during high-order allocations V2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f3a310bc4e5ce7e55e1c8e25c31e63af017f3e50) | 在分配大内存时, 不再使用成块回收(lumpy reclaim)策略, 而是使用内存规整(memory compaction) | v2 ☑ 2.6.38-rc1 | [2010/11/11 LORE RFC v1,0/3](https://lore.kernel.org/all/1289502424-12661-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[2010/11/22 LORE v2,0/7](https://lore.kernel.org/lkml/1290440635-30071-1-git-send-email-mel@csn.ul.ie) |

但是这个阶段并没有完全抛弃成块回收. 回收路径下 shrink_inactive_list() 回收页面时, 如果使能了 CONFIG_COMPACTION(COMPACTION_BUILD=1), 则回收模式 reclaim_mode 会被设置为 RECLAIM_MODE_COMPACTION, 而没有开启内存规整的情况下, 依旧使用 RECLAIM_MODE_LUMPYRECLAIM;

*   Lumpy Reclaim 的寿终正寝

成块回收最终在 3.5 版本废除, 被内存规整(内存碎片整理技术)取代. 成块回收不是一个完整的解决方案, 它只是缓解了碎片问题.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/03/19 | Konstantin Khlebnikov <khlebnikov@openvz.org> | [mm: forbid lumpy-reclaim in shrink_active_list()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1480de0340a8d5f094b74d7c4b902456c9a06903) | 将 shrink_active_list () 的回收模式重置为 RECLAIM_MODE_SINGLE | RECLAIM_MODE_ASYNC. 其中 SYNC/ASYNC 只在 shrink_page_list() 中使用, 不影响 shrink_active_list().<br>当前 shrink_active_list() 有时工作在成块回收 RECLAIM_MODE_LUMPYRECLAIM 模式, 然后会在 age_active_anon() 中重置回收模式 sc->reclaim_mode. 整体行为和逻辑过于复杂和混乱, 通常, shrink_active_list() 为下一次 shrink_inactive_list() 填充 inactive 列表.  Lumpy Reclaim 时 shring_inactive_list() 将所选页面周围的页面从活动列表和非活动列表中分离出来. 因此, 没必要在 shrink_active_list() 中进行块状隔离. 参见 [LKML](https://lkml.org/lkml/2012/3/15/583). | v1 ☑✓ 3.4-rc1 | [LORE](https://lore.kernel.org/all/20120319091821.17716.54031.stgit@zurg) |
| 2012/04/11 | Mel Gorman <mgorman@suse.de> | [Removal of lumpy reclaim V2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=23b9da55c5b0feb484bd5e8615f4eb1ce4169453) | 删除了消除了内存规整路径下的[成块状回收](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c53919adc045bf803252e912f23028a68525753d)和一些[引起阻塞的逻辑](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=41ac1999c3e3563e1810b14878a869c79c9368bb). 最终的结果是, 页面回收过程中脏页上的暂停现在就只剩下了 wait_iff_consugged(). | v1 ☑✓ 3.5-rc1 | [LORE RFC v1,0/2](https://lore.kernel.org/all/1332950783-31662-1-git-send-email-mgorman@suse.de)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/3](https://lore.kernel.org/all/1334162298-18942-1-git-send-email-mgorman@suse.de) |

*   关于回收模式

[commit 7d3579e8e619 ("vmscan: narrow the scenarios in whcih lumpy reclaim uses synchrounous reclaim")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7d3579e8e61937cbba268ea9b218d006b6d64221) 引入了 enum lumpy_mode.

引入直接规整替代成块回收的过程中, 移除 enum lumpy_mode, 引入了 RECLAIM_MODE_COMPACTION. 将原来的 LUMPY_MODE_CONTIGRECLAIM 重命名为 RECLAIM_MODE_LUMPYRECLAIM. 其中 [commit ee64fc9354e5 ("mm: vmscan: convert lumpy_mode into a bitmask")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ee64fc9354e515a79c7232cfde65c88ec627308b) 直接删除了 enum lumpy_mode 类型, 将他们置换为 bitmask, 接着 [commit f3a310bc4e5c ("mm: vmscan: rename lumpy_mode to reclaim_mode")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f3a310bc4e5ce7e55e1c8e25c31e63af017f3e50) LUMPY_MODE_XXXX 重命名为 RECLAIM_MODE_XXXX.

最终 [Removal of lumpy reclaim V2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=23b9da55c5b0feb484bd5e8615f4eb1ce4169453) 彻底移除了 reclaim_mode 和 Lumpy Reclaim. 其中 [commit c53919adc045 ("mm: vmscan: remove lumpy reclaim")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c53919adc045bf803252e912f23028a68525753d) 移除了 RECLAIM_MODE_LUMPYRECLAIM. [commit 23b9da55c5b0 ("mm: vmscan: do not stall on writeback during memory compaction")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=41ac1999c3e3563e1810b14878a869c79c9368bb) 移除了 RECLAIM_MODE_ASYNC 和 RECLAIM_MODE_SYNC. [commit 23b9da55c5b0 ("mm: vmscan: remove reclaim_mode_t")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=23b9da55c5b0feb484bd5e8615f4eb1ce4169453) 最终移除了剩余的 RECLAIM_MODE_XXXX 和 reclaim_mode.

至此不再通过 RECLAIM_MODE_COMPACTION 判断是否在进行回收规整, 开启了 CONFIG_COMPACTION 的情况下, 就默认使能. 参见 should_continue_reclaim()-=>in_reclaim_compaction().


## 3.3 通过迁移类型分组来实现反碎片
-------

也称为: 基于页面可移动性的页面聚类(Page Clustering by Page Mobility)


### 3.3.1 page_group_by_mobility
-------

为什么要引入迁移类型. 我们都知道伙伴系统是针对于解决外碎片的问题而提出的, 那么为什么还要引入这样一个概念来避免碎片呢?


在去碎片化时, 需要移动或回收页面, 以腾出连续的物理页面, 但可能一颗"老鼠屎就坏了整锅粥"——由于某个页面无法移动或回收, 导致整个区域无法组成一个足够大的连续页面块. 这种页面通常是内核使用的页面, 因为内核使用的页面的地址是直接映射(即物理地址加个偏移就映射到内核空间中), 这种做法不用经过页表翻译, 提高了效率, 却也在此时成了拦路虎.

我们注意到, 碎片一般是指散布在内存中的小块内存, 由于它们已经被分配并且插入在大块内存中, 而导致无法分配大块的连续内存. 而伙伴系统把内存分配出去后, 要再回收回来并且重新组成大块内存, 这样一个过程必须建立两个伙伴块必须都是空闲的这样一个基础之上, 如果其中有一个伙伴不是空闲的, 甚至是其中的一个页不是空闲的, 都会导致无法分配一块连续的大块内存. 我们引用一个例子来看这个问题:

![碎片化](1338638697_6064.png)

图中, 如果15对应的页是空闲的, 那么伙伴系统可以分配出连续的16个页框, 而由于15这一个页框被分配出去了, 导致最多只能分配出8个连续的页框. 假如这个页还会被回收回伙伴系统, 那么至少在这段时间内产生了碎片, 而如果更糟的, 如果这个页用来存储一些内核永久性的数据而不会被回收回来, 那么碎片将永远无法消除, 这意味着15这个页所处的最大内存块永远无法被连续的分配出去了. 假如上图中被分配出去的页都是不可移动的页, 那么就可以拿出一段内存, 专门用于分配不可移动页, 虽然在这段内存中有碎片, 但是避免了碎片散布到其他类型的内存中. 在系统中所有的内存都被标识为可移动的！也就是说一开始其他类型都没有属于自己的内存, 而当要分配这些类型的内存时, 就要从可移动类型的内存中夺取一部分过来, 这样可以根据实际情况来分配其他类型的内存.

Mel Gorman 观察到, 所有使用的内存页有三种情形:

| 编号 | 类型 | 描述 |
|:---:|:---:|:----:|
| 1 | 容易回收的(easily reclaimable) | 这种页面可以在系统需要时回收, 比如文件缓存页, 们可以轻易的丢弃掉而不会有问题(有需要时再从后备文件系统中读取); 又比如一些生命周期短的内核使用的页, 如DMA缓存区. |
| 2 | 难回收的(non-reclaimable) | 这种页面得内核主动释放, 很难回收, 内核使用的很多内存页就归为此类, 比如为模块分配的区域, 比如一些常驻内存的重要内核结构所占的页面. |
| 3 | 可移动的(movable) | 用户空间分配的页面都属于这种类型, 因为用户态的页地址是由页表翻译的, 移动页后只要修改页表映射就可以(这也从另一面应证了内核态的页为什么不能移动, 因为它们采取直接映射). |


长年致力于解决内存碎片化的内存领域黑客 Mel Gorman 观察到这个事实, 在经过 30 个版本的修改后, 他的解决方案 page_group_by_mobility 机制 进入内核. 他修改了伙伴分配器和分配 API, 使得在分配时告知伙伴分配器页面的可移动性: 回收时, 把相同移动性的页面聚类; 分配时, 根据移动性, 从相应的聚类中分配. 参见 [Group pages of related mobility together to reduce external fragmentation v30](https://lore.kernel.org/lkml/20070910112011.3097.8438.sendpatchset@skynet.skynet.ie).

聚类的好处是, 结合上述的**成块回收**方案, 回收页面时, 就能保证回收同一类型的; 或者在迁移页面时(migrate page), 就能移动可移动类型的页面, 从而腾出连续的页面块, 以满足高阶的连续物理页面分配.

关于细节, 可看我之前写的文章:[Linux内核中避免内存碎片的方法(1)](https://link.zhihu.com/?target=http%3A//larmbr.com/2014/04/08/avoiding-memory-fragmentation-in-linux-kernelp%281%29).


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/09/10 | Mel Gorman <mel@csn.ul.ie> | [Reduce external fragmentation by grouping pages by mobility v30](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=467c996c1e1910633fa8e7adc9b052aa3ed5f97c) | 基于页面可移动性的页面聚类来抗(外部)碎片化. 引入 CONFIG_PAGE_GROUP_BY_MOBILITY 控制 | v30 ☑ 2.6.24-rc1 | [2006/11/01 PatchWork v26,0/11](https://lore.kernel.org/patchwork/patch/67812)<br>*-*-*-*-*-*-*-* <br>[2006/11/21 PatchWork v27,0/11](https://lore.kernel.org/patchwork/patch/68969)<br>*-*-*-*-*-*-*-* <br>[Patchwork v28,0/12](https://lore.kernel.org/patchwork/patch/75208)<br>*-*-*-*-*-*-*-* <br>[Patchwork v30,0/13](https://lore.kernel.org/lkml/20070910112011.3097.8438.sendpatchset@skynet.skynet.ie) |
| 2007/05/17 | Mel Gorman <mel@csn.ul.ie> | [Annotation fixes for grouping pages by mobility v2](https://lore.kernel.org/patchwork/patch/81511) | NA | v1 ☐ | [PatchWork 0/5](https://lore.kernel.org/patchwork/patch/81511) |
| 2007/05/25 | Mel Gorman <mel@csn.ul.ie> | [Arbitrary grouping and statistics for grouping pages by mobility](https://lore.kernel.org/patchwork/patch/82099) | NA | v1 ☐ | [PatchWork 0/5](https://lore.kernel.org/patchwork/patch/82099))<br>*-*-*-*-*-*-*-* <br>[2007/06/01 Patchwork v4, 0/3](https://lore.kernel.org/patchwork/patch/82585) |
| 2015/02/12 | Joonsoo Kim <iamjoonsoo.kim@lge.com> | [mm/page_alloc: factor out fallback freepage checking](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4eb7dce62007113f1a2778213980fd6d8034ef5e) | [enhance compaction success rate](https://lore.kernel.org/lkml/1422621252-29859-1-git-send-email-iamjoonsoo.kim@lge.com) 的其中一个补丁. | v4 ☑✓ 4.1-rc1 | [LORE v4,0/3](https://lore.kernel.org/all/1423725305-3726-2-git-send-email-iamjoonsoo.kim@lge.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4eb7dce62007113f1a2778213980fd6d8034ef5e) |

内核按照 pageblock 为粒度对页面按照 MIGRATE_TYPE 划分, 因此只有[当页面总数 vm_total_pages 比 MAX_ORDER_NR_PAGES * MIGRATE_TYPES 多](https://elixir.bootlin.com/linux/v2.6.32/source/mm/page_alloc.c#L2777)时, 才能成功地对页面进行分组. 当区域中的内存不足时, 就不能使用 page_group_by_mobility 机制. 参见 [commit 9ef9acb05a74 ("Do not group pages by mobility type on low memory systems")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9ef9acb05a741ec10a5e9122717736de12adced9).



### 3.3.2 从其他 MIGRATETYPE 窃取页面
-------

v2.6.24 实现迁移类型 MIGRATETYPE 的时候, 在从伙伴系统中内存分配出来的时候,  [commit b2a0ac8875a0 ("Split the free lists for movable and unmovable allocations")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b2a0ac8875a0a3b9f0739b60526f8c5977d2200f) 引入了 [`__rmqueue_fallback()`](https://elixir.bootlin.com/linux/v2.6.31/source/mm/page_alloc.c#L864) 用来在当目标 MIGRATETYPE 中没有空闲的页面时, 从其他 MIGRATETYPE 的高阶内存块中窃取页面出来完成分配. 这个过程中, 为了防止引入内存碎片, 轻易不会修改窃取的内存块的 MIGRATETYPE. 只有[窃取了一整块 pageblock_order(`MAX_ORDER - 1`) 的页面](https://elixir.bootlin.com/linux/v2.6.31/source/mm/page_alloc.c#L840)时, 才会将这整块 pageblock 的 MIGRATETYPE 更新为窃取后的类型. 而对于不足 pageblock 的大块内存, 则通过 [move_freepages_block()](https://elixir.bootlin.com/linux/v2.6.31/source/mm/page_alloc.c#L823) 只是将整块 pageblock 页面窃取过来, 但是其实际 MIGRATETYPE 不做修改, 这样的页面再释放的时候, 依旧会归还到其原来 MIGRATETYPE 的空闲列表中去. 参见 [commit c361be55b312 ("Move free pages between lists on steal")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c361be55b3128474aa66d31092db330b07539103). 随后 [commit 46dafbca2bba ("Be more agressive about stealing when MIGRATE_RECLAIMABLE allocations fallback")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=46dafbca2bba811665b01d8cedf911204820623c) 进一步优化了此逻辑, 如果窃取的页面块, [空闲的页面空间超过了半数](https://elixir.bootlin.com/linux/v2.6.31/source/mm/page_alloc.c#L829), 则其 MIGRATETYPE 也会被修改.

因此当前只有窃取时被分割的页面 Order 正好是一个 pageblock_order 时, 整个页面块的 MIGRATETYPE 才会被更新. 当 窃取的页面 order < pageblock_order 时, MIGRATETYPE 没有改变, 页面被释放时会归还到旧的 MIGRATETYPE 列表中, 可以有效地避免页面的外碎片化. 但是如果窃取了 > pageblock_order 的页面时, 也不修改其 MIGRATETYPE 就显得不是很合理. 因此 v2.6.32 [commit 2f66a68f3fac ("`page-allocator: change migratetype for all pageblocks within a high-order page during __rmqueue_fallback`")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2f66a68f3fac2e94da360c342ff78ab45553f86c) 就修正了此策略, 当获取的页面的顺序大于 pageblock_order 时, 该高阶页面中的所有页面块都会被更改为 MIGRATETYPE. 在没有这个补丁的情况下, 在 x86-64 上, 在压力下分配大量页面的成功率约为物理内存的 59%. 合入此补丁后, 这一比例上升到 65%.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/09/01 | Mel Gorman <mel@csn.ul.ie> | [page-allocator: Always change pageblock ownership when anti-fragmentation is disabled](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=dd5d241ea955006122d76af88af87de73fec25b4) | 20090901110005.GB27393@csn.ul.ie | v1 ☐☑✓ 2.6.31-rc9 | [LORE](https://lore.kernel.org/all/20090901110005.GB27393@csn.ul.ie), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=dd5d241ea955006122d76af88af87de73fec25b4) |
| 2009/09/21 | Mel Gorman <mel@csn.ul.ie> | [`page-allocator: change migratetype for all pageblocks within a high-order page during __rmqueue_fallback`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2f66a68f3fac2e94da360c342ff78ab45553f86c) | 将被分割的高阶页面内的所有页面块更改为正确的 MIGRATETYPE. 当获取的页面的顺序大于 pageblock_order 时, 该高阶页面中的所有页面块都应该更改 MIGRATETYPE, 以便页面稍后被释放到对应的自由列表中. | v1 ☑✓ 2.6.32-rc1 | [LORE](https://lore.kernel.org/all/20090729094951.GA15102@csn.ul.ie), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2f66a68f3fac) |

### 3.3.2 从其他 MIGRATETYPE 窃取页面
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2022/06/06 | xin huanpeng <xinhuanpeng9@gmail.com> | [mm: add a new emergency page migratetype.](https://patchwork.kernel.org/project/linux-mm/patch/20220606032709.11800-1-xinhuanpeng9@gmail.com) | 添加一个新的页面 migratetype, 用于非昂贵的非 nowarn 页面分配失败. | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220606032709.11800-1-xinhuanpeng9@gmail.com) |


## 3.4 内存规整(Memory Compaction)
-------

### 3.4.1 内存规整
-------

**2.6.35(2010年8月发布)**

2.2中讲到页面迁移类型(聚类), 它把相当可移动性的页面聚集在一起: 可移动的在一起, 可回收的在一起, 不可移动的也在一起. **它作为去碎片化的基础.** 然后, 利用**成块回收**, 在回收时, 把可回收的一起回收, 把可移动的一起移动, 从而能空出大量连续物理页面. 这个**作为去碎片化的策略.**



2.6.35 里, Mel Gorman 又实现了一种新的**去碎片化的策略** 叫[**内存紧致化或者内存规整**](https://lwn.net/Articles/368869). 不同于**成块回收**回收相临页面, **内存规整**则是更彻底, 它在回收页面时被触发, 它会在一个 zone 里扫描, 把已分配的页记录下来, 然后把所有这些页移动到 zone 的一端, 这样这把一个可能已经七零八落的 zone 给紧致化成一段完全未分配的区间和一段已经分配的区间, 这样就又腾出大块连续的物理页面了.

它后来替代了成块回收, 使得后者在 3.5 中被移除.

[memory compaction 原理、实现与分析](https://blog.csdn.net/21cnbao/article/details/118687445)

通过碎片索引 [fragmentation_index()](https://elixir.bootlin.com/linux/v2.6.35/source/mm/compaction.c#L495) 可以确定分配失败是由于内存不足还是外部碎片造成的. 参见 [Linux 内存碎片化检视之 buddy_info | extfrag_index | unusable_index](https://blog.csdn.net/memory01/article/details/80958009).

| fragindex 值 | 描述 |
| -1 | 分配可能成功, 取决于水印. |
|  0 | 碎片化并不严重, 分配失败是由于缺少内存 |
| 1000 | 碎片化非常严重, 分配失败是由于碎片导致的只有紧凑的情况下失败是由于碎片.(数值越接近1000, 说明碎片化越严重). |

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2010/04/20 | Mel Gorman <mel@csn.ul.ie> | [Memory Compaction](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=4f92e2586b43a2402e116055d4edda704f911b5b) | 内存规整, 参见 [LWN: Memory compaction](https://lwn.net/Articles/368869) | v8 ☑ 2.6.35-rc1 | [PatchWork v8](https://lore.kernel.org/lkml/1271797276-31358-1-git-send-email-mel@csn.ul.ie) |
| 2010/11/22 | Mel Gorman <mel@csn.ul.ie> | [Use memory compaction instead of lumpy reclaim during high-order allocations V2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f3a310bc4e5ce7e55e1c8e25c31e63af017f3e50) | 在分配大内存时, 不再使用成块回收(lumpy reclaim)策略, 而是使用内存规整(memory compaction) | v2 ☑ 2.6.38-rc1 | [2010/11/11 LORE RFC v1,0/3](https://lore.kernel.org/all/1289502424-12661-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[2010/11/22 LORE v2,0/7](https://lore.kernel.org/lkml/1290440635-30071-1-git-send-email-mel@csn.ul.ie) |
| 2012/04/11 | Mel Gorman <mel@csn.ul.ie> | [Removal of lumpy reclaim V2](https://lore.kernel.org/patchwork/patch/296609) | 移除成块回收(lumpy reclaim) 的代码. | v2 ☑ [3.5-rc1](https://kernelnewbies.org/Linux_3.5#Memory_Management) | [PatchWork v2](https://lore.kernel.org/patchwork/patch/296609) |
| 2012/09/21 | Mel Gorman <mgorman@suse.de> | [Reduce compaction scanning and lock contention](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=62997027ca5b3d4618198ed8b1aba40b61b1137b) | 进一步优化内存规整的扫描耗时和锁开销. | v1 ☑✓ 3.7-rc1 | [LORE 0/6](https://lore.kernel.org/all/1348149875-29678-1-git-send-email-mgorman@suse.de)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/9](https://lore.kernel.org/all/1348224383-1499-1-git-send-email-mgorman@suse.de) |
| 2013/12/05 | Mel Gorman <mel@csn.ul.ie> | [Removal of lumpy reclaim V2](https://lore.kernel.org/patchwork/patch/296609) | 添加了 start 和 end 两个 tracepoint, 用于内存规整的开始和结束. 通过这两个 tracepoint 可以计算工作负载在规整过程中花费了多少时间, 并可能调试与用于扫描的缓存 pfns 相关的问题. 结合直接回收和 slab 跟踪点, 应该可以估计工作负载的大部分与分配相关的开销. | v2 ☑ 3.14-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/296609) |
| 2016/08/10 | Mel Gorman <mel@csn.ul.ie> | [make direct compaction more deterministic](https://lore.kernel.org/patchwork/patch/692460) | 更有效地直接规整(压缩迁移). 在内存分配的慢速路径 `__alloc_pages_slowpath` 中的之前一直会先尝试直接回收和规整, 直到分配成功或返回失败.<br>1. 当回收先于压缩时更有可能成功, 因为压缩需要满足某些苛刻的条件和水线要求, 并且在有更多的空闲页面时会增加压缩成功的概率.<br>2. 另一方面, 从轻异步压缩(如果水线允许的话)开始也可能更有效, 特别是对于较小 order 的申请. 因此[这个补丁])(https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a8161d1ed6098506303c65b3701dedba876df42a)将慢速路径下的尝试流程修正为将先进行 MIGRATE_ASYNC 异步迁移(规整), 再尝试内存直接回收, 接着进行 MIGRATE_SYNC_LIGHT 轻度同步迁移(规整). 并引入了[直接规整的优先级](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a5508cd83f10f663e05d212cb81f600a3af46e40). | RFC v2 ☑ 4.8-rc1 & 4.9-rc1 | [PatchWork v3](https://lore.kernel.org/patchwork/patch/692460)<br>*-*-*-*-*-*-*-* <br>[PatchWork series 1 v5](https://lore.kernel.org/patchwork/patch/700017)<br>*-*-*-*-*-*-*-* <br>[PatchWork series 2 v6](https://lore.kernel.org/patchwork/patch/705827) |


### 3.4.2 慢速路径的内存规整
-------

#### 3.4.2.1 在直接回收之前先尝试直接内存规整
-------

之前当分配高阶(high order)内存分配失败时, 首先唤醒 KSWAPD 进行异步回收, 然后 [`__alloc_pages_high_priority()`](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L2003) 尝试[忽略水线 ALLOC_NO_WATERMARKS](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L2002) 再分配一次, 如果还是分配不成功, 则直接通过 [`__alloc_pages_direct_reclaim()`](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L2032) 直接回收内存来释放内存空间.

但是这并不是高效解决问题的方法, 如果是因为内存的确不足造成的, 那直接回收可以解决问题. 但是如果是因为外部碎片造成的, 可能通过内存规整更加合适. 为内存规整只在在内存中移动页面, 这比将页面 SWAP OUT 或者 WRITE BACK 到磁盘的开销要小很多, 并且适用于有 MLOCK 锁定页面或没有 SWAP 的情况.

2.6.35-rc1, Mel Gorman 在引入内存规整 [Memory Compaction v8](https://lore.kernel.org/lkml/1271797276-31358-1-git-send-email-mel@csn.ul.ie) 的过程中. [mm: compaction: direct compact when a high-order allocation fails](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=56de7263fcf3eb10c8dcdf8d59a9cec831795f3f) 就在内存分配的回收路径引入了直接内存规整 [`__alloc_pages_direct_compact()`](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L1783).

在进行直接回收 [`__alloc_pages_direct_reclaim()`](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L2032) 之前, [`__alloc_pages_high_priority()`](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L2003) 之后, 通过[直接规整 `__alloc_pages_direct_compact()`](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L2023) 的内存 fragmentation_index() 来确认当前[高阶内存分配](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L1790)失败是[内存不足](https://elixir.bootlin.com/linux/v2.6.35/source/mm/compaction.c#L499)还是因为[外部碎片](https://elixir.bootlin.com/linux/v2.6.35/source/mm/compaction.c#L496)造成的, 如果发现是因为外部碎片造成的, 就会通过 `try_to_compact_pages() -=> compact_zone_order()` 尝试规整出足够大小的页面来[完成页面分配](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L1801). 另外如果内存规整也无法释放合适大小的页面来完成页面分配, 则依旧会进行直接回收. 由于在内存分配(慢速)路径进行, 直接规整不能耗时过长, 应该尽快返回. 因此在规整每个 ZONE 时, 都检查是否释放了合适 order 的页面, 如果释放了, 则返回.


#### 3.4.2.2 使用(order-0)页面的回收规整替代 Lumpy Reclaim
-------

成块回收(Lumpy Reclaim) 是非常粗暴的行为, 它回收大量的页面, 而且不考虑页面本身的老化, 这耗时可能非常长, 造成严重的阻塞, 并增加应用 Page Fault 的次数, 影响性能. 而相比较, 内存规整效率更高, 是一个不错的替代的成块回收的操作.

因此 v2.6.38 [commit 3e7d34497067 ("mm: vmscan: reclaim order-0 and use compaction instead of lumpy reclaim")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3e7d344970673c5334cf7b5bb27c8c0942b06126) 引入了一个称为 (先)回收(后)规整(`reclaim/compaction`) 的方法来替代成块回收. 它的基本思路非常简单, 不再是选择一个连续的页面范围来回收, 而是回收大量的 order-0 页面, 然后通过 kswapd (compact_zone_order()) 或[直接规整(`__alloc_pages_direct_compact()`)](https://elixir.bootlin.com/linux/v2.6.38/source/mm/page_alloc.c#L2148) 进行规整, 从而规整出分配所需的足够连续页面.


1.  引入了回收规整(`reclaim/compaction`) 后, `__alloc_pages_slowpath()` 中可能会进行两次直接规整.

第一次是在直接回收 `__alloc_pages_direct_reclaim()` 之前, 如果发现高阶内存分配失败不是因为内存不足, 而是因为内存碎片比较严重, 则通过 `__alloc_pages_direct_compact()` 尝试规整出足够的连续内存以供分配.

第二次是在直接回收 `__alloc_pages_direct_reclaim()` 回收了足够多的内存之后, 则会使用 `should_alloc_retry()` 检测是否可以分配出足够的内存, 如果不行, 则将再次进行直接规整 `__alloc_pages_direct_compact()` 尝试在回收之后规整出分配所需的连续页面出来.

引入了回收规整后, 内存分配慢速路径下, 倾向于释放一些 order-0 的页面后, 然后通过规整的方式进行碎片整理. 参见 [mm: vmscan: reclaim order-0 and use compaction instead of lumpy reclaim](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3e7d344970673c5334cf7b5bb27c8c0942b06126), [vmscan: reclaim at order 0 when compaction is enabled](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=fe2c2a106663130a5ab45cb0e3414b52df2fff0c) 优化了有规整情况下, 对 order-0 页面的处理, 不再积极地对高阶页面进行回收. 减少了阻塞的可能性.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2010/04/20 | Mel Gorman <mel@csn.ul.ie> | [mm: compaction: direct compact when a high-order allocation fails](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=56de7263fcf3eb10c8dcdf8d59a9cec831795f3f) | 内存规整 [Memory Compaction](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=4f92e2586b43a2402e116055d4edda704f911b5b) 系列的其中一个补丁,  | v8 ☑ 2.6.35-rc1 | [PatchWork v8](https://lore.kernel.org/lkml/1271797276-31358-1-git-send-email-mel@csn.ul.ie) |
| 2010/11/22 | Mel Gorman <mel@csn.ul.ie> | [mm: vmscan: reclaim order-0 and use compaction instead of lumpy reclaim](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3e7d344970673c5334cf7b5bb27c8c0942b06126) | [Use memory compaction instead of lumpy reclaim during high-order allocations V2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f3a310bc4e5ce7e55e1c8e25c31e63af017f3e50) 的其中一个补丁. 在分配大内存时, 不再使用成块回收(lumpy reclaim)策略, 而是使用内存规整(memory compaction) | v2 ☑ 2.6.38-rc1 | [2010/11/11 LORE RFC v1,0/3](https://lore.kernel.org/all/1289502424-12661-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[2010/11/22 LORE v2,0/7](https://lore.kernel.org/lkml/1290440635-30071-1-git-send-email-mel@csn.ul.ie), [关注 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3e7d344970673c5334cf7b5bb27c8c0942b06126) |
| 2012/01/24 | Rik van Riel <riel@redhat.com> | [vmscan: reclaim at order 0 when compaction is enabled](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=fe2c2a106663130a5ab45cb0e3414b52df2fff0c) | [kswapd vs compaction improvements](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=aff622495c9a0b56148192e53bdec539f5e147f2) 的其中一个补丁. 当开启了 CONFIG_COMPACTION 构建时, 就[不再使用成块回收(Lumpy Reclaim)](https://elixir.bootlin.com/linux/v3.4/source/mm/vmscan.c#L378), kswapd 不会尝试释放连续的页面. shrink_inactive_list() 回收页面的过程中, 很多路径只需要对 order-0 的页面回收进行积极的响应和处理<br>1. 因为它没有尝试高阶页面的回收, 所以  balance_pgdat() 中[也不应该测试它是否成功](https://elixir.bootlin.com/linux/v3.4/source/mm/vmscan.c#L2817), 否则这会导致持续的页面回收, 直到有很大一部分内存是空闲的, 造成 workingset 的大部分页面被驱逐.<br>2. 除非我们真的处于块状回收模式 RECLAIM_MODE_LUMPYRECLAIM, isolate_lru_pages() 中不应该尝试[进行更高阶(超出 LRU 顺序) 的页面隔离](https://elixir.bootlin.com/linux/v3.4/source/mm/vmscan.c#L1197),  这为所有页面在不活动列表上提供了大量的时间, 为积极使用的页面提供了被引用和避免被驱逐的机会. | v2 ☑✓ 3.4-rc1 | [LORE v2,0/3](https://lore.kernel.org/all/20120124131822.4dc03524@annuminas.surriel.com) |


#### 3.4.2.3 内存规整过程中的内存迁移(同步和异步内存迁移)
-------

不过两次直接规整的操作是有差异的. 页面的同步迁移将等待回写完成. 如果 Caller 对延迟非常敏感, 或者并不关心迁移是否完成, 我们可以使用异步迁移. 内存分配的(慢速)路径很明显就属于前者.

因此第一次直接规整就尝试使用异步页面迁移 MIGRATE_ASYNC. 而随后第二次的回收规整就使用同步迁移 MIGRATE_SYNC. 参见 [v2.6.38-rc1, commit 77f1fe6b08b1 ("mm: migration: allow migration to operate asynchronously and avoid synchronous compaction in the faster path")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=77f1fe6b08b13a87391549c8a820ddc817b6f50e). 这个补丁为 `migrate_pages()` 添加了一个[同步参数 sync](https://elixir.bootlin.com/linux/v2.6.38/source/mm/migrate.c#L896), 允许调用者指定在迁移过程中是否[等待 wait_on_page_writeback()](https://elixir.bootlin.com/linux/v2.6.38/source/mm/migrate.c#L691).

但是存在一个问题, 当将文件复制到 U 盘等设备时, 可能会有大量脏页写入到不支持 `->writepages()` 操作的文件系统, 如果这时候触发回收规整操作. 将造成长时间的阻塞. 因此 v3.3 [mm: compaction: introduce sync-light migration for use by compaction](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a6bc32b899223a877f595ef9ddc1e89ead5072b8) 实现了一种专门用于(第二次的)回收规整的轻量级回收迁移模式 MIGRATE_SYNC_LIGHT. 这种模式下允许对大多数操作进行阻塞, 但不允许 `->writepage()`, 因为潜在的暂停时间太长. 至此直接规整(第一次)使用异步页面迁移 MIGRATE_ASYNC, 回收规整(第二次)就使用同步迁移 MIGRATE_SYNC_LIGHT.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2011/12/14 | Mel Gorman <mgorman@suse.de> | [mm: compaction: introduce sync-light migration for use by compaction](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a6bc32b899223a877f595ef9ddc1e89ead5072b8) | [Reduce compaction-related stalls and improve asynchronous migration of dirty pages v6](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0cee34fd72c582b4f8ad8ce00645b75fb4168199) 的其中一个补丁, 这个补丁增加了一个轻量级的同步迁移操作 MIGRATE_SYNC_LIGHT 模式, 以避免将页面写回备份存储. 异步压缩映射到 MIGRATE_ASYNC, 而同步压缩映射到 MIGRATE_SYNC_LIGHT. 从而避免同步压缩时间过长而停滞. | v6 ☑✓ 3.3-rc1 | [LORE 0/5](https://lore.kernel.org/all/1321635524-8586-1-git-send-email-mgorman@suse.de)<br>*-*-*-*-*-*-*-* <br>[LORE RRC v4r2,0/7](https://lore.kernel.org/all/1321900608-27687-1-git-send-email-mgorman@suse.de)<br>*-*-*-*-*-*-*-* <br>[LORE v6,0/11](https://lore.kernel.org/all/1323877293-15401-1-git-send-email-mgorman@suse.de) |

#### 3.4.2.4 规整策略对 THP 等高阶内存分配的影响(分配开销和成功率)
-------

分配高阶内存(包括 Page Fault 时尝试分配大页)是一项非常耗时的操作, 如果这里进行了回收规整执行同步迁移可能会造成较大的延迟.

*   首先是如何提升高阶内存分配的成功率

v3.4 [commit fe2c2a106663 ("vmscan: reclaim at order 0 when compaction is enabled")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=fe2c2a106663130a5ab45cb0e3414b52df2fff0c) 修正在内存压缩使能后, 不再倾向于回收高 order 的页面, 而是积极地回收 order-0 的页面. 然后通过直接规整通过碎片整理的方式整理出足够的连续页面出来, 目标和期望是很美好的, 但是却造成高阶(order)的页面分配成功率一直较低. 之前成块回收以及对高阶页面的积极回收虽然可能造成较长时间的阻塞, 但是不可否认的是的确效果不错.

v3.6 [commit 7db8889ab05b ("mm: have order > 0 compaction start off where it left")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7db8889ab05b57200158432755af318fb68854a2) 和 [mm: have order > 0 compaction start near a pageblock with free pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=de74f1cc3b1e) 大大减少了扫描量, 不光实现复杂且难以理解, 还使得高阶内存分配的成功率再次受到较大的下降, 甚至在部分场景, 直接下降到 0, 参见 [Re: Windows VM slow boot](https://lore.kernel.org/all/20120912164615.GA14173@alpha.arachsys.com). 为了解决这些问题.

因此在 v3.7 Mel Gorman 设计了新的算法, 上面两个补丁统统[被 revert](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=753341a4b85ff337487b9959c71c529f522004f4). 参见 [Reduce compaction scanning and lock contention](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=62997027ca5b3d4618198ed8b1aba40b61b1137b). 但是这只是减少了扫描量, 减少了锁争抢. 并没有提高高阶内存分配成功率下降的问题, 与此同时 Mel 进一步改善了高阶内存分配的成功率 [Improve hugepage allocation success rates under load](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1fb3f8ca0e9222535a39b884cb67a34628411b9f).

*   其次是如何减少高阶内存分配的时慢速路径的阻塞

分配高阶内存(包括 Page Fault 时尝试分配大页)是一项非常耗时的操作, 如果这里进行了回收规整执行同步迁移可能会造成较大的延迟. 随后 v3.16, [commit aeef4b8380 ("mm, compaction: embed migration mode in compact_control")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=aeef4b83806f49a0c454b7d4578671b71045bee2) 慢速路径开始显式使用 enum migrate_mode 来标记两次内存规整的页面迁移, 因此 THP 分配的路径不再使用 MIGRATE_SYNC_LIGHT. 参见 [commit 75f30861a12a ("mm, thp: avoid excessive compaction latency during fault")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=75f30861a12a6b09b759dfeeb9290b681af89057).


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/08/07 | Mel Gorman <mgorman@suse.de> | [Improve hugepage allocation success rates under load](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1fb3f8ca0e9222535a39b884cb67a34628411b9f) | 1344342677-5845-1-git-send-email-mgorman@suse.de | v1 ☑✓ 3.6-rc1,3.7-rc1 | [LORE v1,0/6](https://lore.kernel.org/all/1344342677-5845-1-git-send-email-mgorman@suse.de)<br>*-*-*-*-*-*-*-* <br>[commit1-3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1fb3f8ca0e9222535a39b884cb67a34628411b9f), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7db8889ab05b57200158432755af318fb68854a2), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=de74f1cc3b1e9730d9b58580cd11361d30cd182d) |
| 2012/09/21 | Mel Gorman <mgorman@suse.de> | [Reduce compaction scanning and lock contention](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=62997027ca5b3d4618198ed8b1aba40b61b1137b) | 1348224383-1499-1-git-send-email-mgorman@suse.de | v1 ☑✓ 3.7-rc1 | [LORE 0/6](https://lore.kernel.org/all/1348149875-29678-1-git-send-email-mgorman@suse.de)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/9](https://lore.kernel.org/all/1348224383-1499-1-git-send-email-mgorman@suse.de) |
| 2014/05/06 | David Rientjes <rientjes@google.com> | [mm, compaction: embed migration mode in compact_control](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=aeef4b83806f49a0c454b7d4578671b71045bee2) | [mm, migration: add destination page freeing callback](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e0b9daeb453e602a95ea43853dc12d385558ce1f) 的其中一个补丁. 使用了 enum migrate_mode 代替原来的 bool sync_migration. 同时THP 分配的路径下的回收规整不再使用 MIGRATE_SYNC_LIGHT. | v4 ☑✓ 3.16-rc1 | [LORE v4,0/6](https://lore.kernel.org/all/alpine.DEB.2.02.1405070336200.16568@chino.kir.corp.google.com) |
| 2014/07/24 | David Rientjes <rientjes@google.com> | [mm, thp: restructure thp avoidance of light synchronous migration](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=8fe780484d2674eec27e12bb29c07d3e98a7ad21) | 不再使用 `__GFP_NO_KSWAPD` 判断 THP 的分配, 而是使用 `gfp_mask & GFP_TRANSHUGE` 来判断. | v1 ☑✓3.17-rc1 | [LORE](https://lore.kernel.org/all/alpine.DEB.2.02.1407241540190.22557@chino.kir.corp.google.com) |


#### 3.4.2.5 规整优先级 compact_priority
-------

在直接规整的上下文中, 对于某些类型的分配, 我们希望在尽可能努力的情况下, 规整要么成功, 要么肯定失败. 当前的 MIGRATE_ASYNC / MIGRATE_SYNC_LIGHT 迁移模式是不够的, 因为有一些启发式的方法, 比如缓存扫描器的位置, 标记不合适的页面块或延迟区域的规整. 至少最后的规整尝试应该能够覆盖这些试探. 为了指示规整应该如何尝试, [commit a5508cd83f10 ("mm, compaction: introduce direct compaction priority")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a5508cd83f10f663e05d212cb81f600a3af46e40) 用一个新的 enum compact_priority 替换迁移模式. 在构造 struct compact_control 的 compact_zone_order() 中, 优先级被映射到适当的控制标志(比如迁移模式等).


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/07/21 | Vlastimil Babka <vbabka@suse.cz> | [compaction-related cleanups v5](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=c3486f5376696034d0fcbef8ba70c70cfcb26f51) | 20160721073614.24395-1-vbabka@suse.cz | v5 ☑✓ 4.8-rc1 | [LORE v5,0/8](https://lore.kernel.org/all/20160721073614.24395-1-vbabka@suse.cz), [关注 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a5508cd83f10f663e05d212cb81f600a3af46e40) |
| 2016/09/26 | Vlastimil Babka <vbabka@suse.cz> | [followups to reintroduce compaction feedback for OOM decisions](https://lore.kernel.org/all/20160926162025.21555-1-vbabka@suse.cz) | 20160926162025.21555-1-vbabka@suse.cz | v1 ☑✓ | [LORE v1,0/4](https://lore.kernel.org/all/20160926162025.21555-1-vbabka@suse.cz) |


#### 3.4.2.6 CMA 与 `__perform_reclaim()`
-------

随后 v3.5 [commit bba907108710 ("mm: extract reclaim code from `__alloc_pages_direct_reclaim()`")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bba9071087108d3de70bea274e35064cc480487b) 将 `__perform_reclaim()` 函数从 `__alloc_pages_direct_reclaim()` 中分离出来. 用于 CMA 分配过程中的快速回收 CMA 内存出来, 参见 [commit 49f223a9cd96 ("mm: trigger page reclaim in alloc_contig_range() to stabilise watermarks")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=49f223a9cd96c7293d7258ff88c2bdf83065f69c). CMA 使用 alloc_contig_range() 进行分配时, 为了获取足够的页面, 采用了跟内存分配路径的 slowpath 类似的操作, 先通过 `__reclaim_pages() -=> __perform_reclaim()` 快速的回收部分内存出来, 同时保证内存始终在低水线以上. 后来在 v3.8 [commit bc357f431c83 ("mm: cma: remove watermark hacks")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bc357f431c836c6631751e3ef7dfe7882394ad67) 移除了 `__reclaim_pages()`.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/01/25 | Marek Szyprowski <m.szyprowski@samsung.com> | [`mm: extract reclaim code from __alloc_pages_direct_reclaim()`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bba9071087108d3de70bea274e35064cc480487b) | `__perform_reclaim()` 函数从 `__alloc_pages_direct_reclaim()` 中分离出来.<br>后来在 v3.8 [commit bc357f431c83 ("mm: cma: remove watermark hacks")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bc357f431c836c6631751e3ef7dfe7882394ad67) 移除了 `__reclaim_pages()`. | v1 ☑✓ 3.5-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bba9071087108d3de70bea274e35064cc480487b) |

#### 3.4.2.7 使慢速路径的回收规整逻辑更加清晰
-------

[LSF/MM 2016](https://lwn.net/Articles/lsfmm2016) 上对 Michal 的 [oom detection rework v6,00/14](https://lore.kernel.org/lkml/1461181647-8039-1-git-send-email-mhocko@kernel.org) 重构 OOM 的工作进行讨论的时候强调了直接规整的必要性, 以便在回收/规整的循环中提供更好的反馈, 以便它能够可靠地识别何时规整无法取得进一步的进展, 是否应该触发 OOM Killer 或失败. 参见 [CMA and compaction](https://lwn.net/Articles/684611)

[make direct compaction more deterministic v3,00/17](https://lore.kernel.org/lkml/20160624095437.16385-1-vbabka@suse.cz) 完成了这项工作, v3 之后这项工作被拆成两个部分, 分开合入.

首先建议将规整中使用的异步/同步迁移模式扩展到更通用的 "优先级", [Part 1, compaction-related cleanups v5 v5,0/8](https://lore.kernel.org/all/20160721073614.24395-1-vbabka@suse.cz) 增加了一个新的优先级, 它只会覆盖所有的启发式, 并使规整完全扫描所有区域. 当前的优先级映射已经足够了, 因此并没有设计更细粒度的优先级.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/08/10 | Mel Gorman <mel@csn.ul.ie> | [make direct compaction more deterministic](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fdd4c6149a71ff1da98317adb6f18c28f75a6e3f) | 更有效地直接规整(压缩迁移). 在内存分配的慢速路径 `__alloc_pages_slowpath` 中的之前一直会先尝试直接回收和规整, 直到分配成功或返回失败.<br>1. 当回收先于压缩时更有可能成功, 因为压缩需要满足某些苛刻的条件和水线要求, 并且在有更多的空闲页面时会增加压缩成功的概率.<br>2. 另一方面, 从轻异步压缩(如果水线允许的话)开始也可能更有效, 特别是对于较小 order 的申请. 因此[这个补丁])(https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a8161d1ed6098506303c65b3701dedba876df42a)将慢速路径下的尝试流程修正为将先进行 MIGRATE_ASYNC 异步迁移(规整), 再尝试内存直接回收, 接着进行 MIGRATE_SYNC_LIGHT 轻度同步迁移(规整). 并引入了[直接规整的优先级](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a5508cd83f10f663e05d212cb81f600a3af46e40). | v6 ☑ 4.8-rc1 & 4.9-rc1 | [LORE RFC,00/13](https://lore.kernel.org/lkml/1462865763-22084-1-git-send-email-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[LORE v2,00/18](https://lore.kernel.org/lkml/20160531130818.28724-1-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[LORE v3,00/17](https://lore.kernel.org/lkml/20160624095437.16385-1-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/8](https://lore.kernel.org/lkml/20160718112302.27381-1-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[LORE series 1 v5](https://lore.kernel.org/lkml/20160721073614.24395-1-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[LORE series 2 v6,00/11](https://lore.kernel.org/lkml/20160810091226.6709-1-vbabka@suse.cz) |
| 2016/07/21 | Vlastimil Babka <vbabka@suse.cz> | [part 1, compaction-related cleanups v5](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=c3486f5376696034d0fcbef8ba70c70cfcb26f51) | 20160721073614.24395-1-vbabka@suse.cz | v5 ☐☑✓ | [LORE series 1,v4,0/8](https://lore.kernel.org/lkml/20160718112302.27381-1-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[LORE series 1,v5,0/8](https://lore.kernel.org/all/20160721073614.24395-1-vbabka@suse.cz) |
| 2016/08/10 | Mel Gorman <mel@csn.ul.ie> | [part 2, make direct compaction more deterministic](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fdd4c6149a71ff1da98317adb6f18c28f75a6e3f) | NA | v6 ☑ 4.8-rc1 & 4.9-rc1 | [LORE series 2,v6,00/11](https://lore.kernel.org/lkml/20160810091226.6709-1-vbabka@suse.cz) |


#### 3.4.2.8 内存规整与内存水线
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/12/11 | Marek Szyprowski <m.szyprowski@samsung.com> | [mm: cma: remove watermark hacks](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bc357f431c836c6631751e3ef7dfe7882394ad67) | TODO | v1 ☑✓ 3.8-rc1 | [LORE](https://lore.kernel.org/lkml/1352357985-14869-1-git-send-email-m.szyprowski@samsung.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bc357f431c836c6631751e3ef7dfe7882394ad67) |


### 3.4.3 内存规整中的抗碎片化逻辑
-------

#### 3.4.3.1 内存规整的开销以及成功率
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2011/02/25 | Mel Gorman <mel@csn.ul.ie> | [Reduce the amount of time compaction disables IRQs for V2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=b2eef8c0d09101bbbff2531c097543aedde0b525) | 减少内存规整关中断的时间, 降低其开销. | v2 ☑ 2.6.39-rc1 | [PatchWork v2](https://lore.kernel.org/lkml/1298664299-10270-1-git-send-email-mel@csn.ul.ie) |
| 2014/02/14 | Joonsoo Kim <iamjoonsoo.kim@lge.com> | [compaction related commits](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=b6c750163c0d138f5041d95fcdbd1094b6928057) | 内存规整相关清理和优化. 降低了内存规整 9% 的运行时间. 而成功率没有下降. | v2 ☑ 3.15-rc1 | [LORE v1,0/5](https://lore.kernel.org/lkml/1391749726-28910-1-git-send-email-iamjoonsoo.kim@lge.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2 0/5](https://lore.kernel.org/lkml/1392360843-22261-1-git-send-email-iamjoonsoo.kim@lge.com) |
| 2014/07/28 | Vlastimil Babka <vbabka@suse.cz> | [compaction: balancing overhead and success rates](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=99c0fd5e51c447917264154cb01a967804ace745) | 优化内存规整的整体效率, 它试图同时朝着两个相互排斥的目标工作, 即减少规整的开销和提高成功率. 它包括一些清理和或多或少的琐碎(微观)优化, 希望是更智能的锁争用管理, 以及一些准备修补程序, 最终生成最后两个修补程序, 这些修补程序将提高成功率, 并将不太可能成功分配 THP 页面错误的工作降至最低. | v6 ☑✓ 3.18-rc1 | [LORE v3,00/13](https://lore.kernel.org/all/1403279383-5862-1-git-send-email-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[LORE v4,00/15](https://lore.kernel.org/all/1405518503-27687-1-git-send-email-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/14](https://lore.kernel.org/all/1406553101-29326-1-git-send-email-vbabka@suse.cz)<br>*-*-*-*-*-*-*-* <br>[LORE v6,00/13](https://lore.kernel.org/all/1407142524-2025-1-git-send-email-vbabka@suse.cz) |
| 2014/12/08 | Joonsoo Kim <iamjoonsoo.kim@lge.com> | [enhance compaction success rate](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2149cdaef6c0eb59a9edf3b152027392cd66b41f) | 1418022980-4584-1-git-send-email-iamjoonsoo.kim@lge.com | v1 ☐☑✓ | [LORE v1,0/4](https://lore.kernel.org/all/1418022980-4584-1-git-send-email-iamjoonsoo.kim@lge.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/4](https://lore.kernel.org/lkml/1422621252-29859-1-git-send-email-iamjoonsoo.kim@lge.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,1/4](https://lore.kernel.org/lkml/1422861348-5117-1-git-send-email-iamjoonsoo.kim@lge.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,1/3](https://lore.kernel.org/lkml/1423725305-3726-1-git-send-email-iamjoonsoo.kim@lge.com) |
| 2015/07/02 | Mel Gorman <mel@csn.ul.ie> | [Outsourcing compaction for THP allocations to kcompactd](https://lore.kernel.org/patchwork/patch/650051) | 实现 per node 的 kcompactd 内核线程来定期触发内存规整. | RFC v2 ☑ 4.6-rc1 | [PatchWork RFC v2](https://lore.kernel.org/patchwork/patch/650051) |
| 2016/07/21 | Vlastimil Babka <vbabka@suse.cz> | [compaction-related cleanups v5](https://lore.kernel.org/all/20160721073614.24395-1-vbabka@suse.cz) | 20160721073614.24395-1-vbabka@suse.cz | v5 ☐☑✓ | [LORE v5,0/8](https://lore.kernel.org/all/20160721073614.24395-1-vbabka@suse.cz) |
| 2017/03/07 | Vlastimil Babka <vbabka@suse.cz> | [try to reduce fragmenting fallbacks](https://lore.kernel.org/patchwork/patch/766804) | 修复 [Regression in mobility grouping?](https://lkml.org/lkml/2016/9/28/94) 上报的碎片化问题, 通过修改 fallback 机制和 compaction 机制来减少永久随便化的可能性. 其中 fallback 修改时, 仅尝试从不同 migratetype 的 pageblock 中窃取的页面中挑选最小(但足够)的页面. | v3 ☑ [4.12-rc1](https://kernelnewbies.org/Linux_4.12#Memory_management) | [PatchWork v6](https://lore.kernel.org/patchwork/patch/766804), [KernelNewbies](https://kernelnewbies.org/Linux_4.12#Memory_management), [关键 commit 3bc48f96cf11 ("mm, page_alloc: split least stolen page in fallback")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3bc48f96cf11ce8699e419d5e47ae0d456403274) |
| 2019/01/18 |Mel Gorman <mgorman@techsingularity.net> | [Increase success rates and reduce latency of compaction v3](https://lore.kernel.org/patchwork/patch/1033508) | 提高内存规整成功率并减少规整的延迟, 将用于迁移的扫描页面数减少 65%, 将用于迁移目标的可用页面数减少97%, 同时显著提高透明的hugepage分配成功率.<br>这组补丁通过使用自由列表来缩短扫描, 更好地控制跳过信息, 以及是否多个扫描可以瞄准同一块并在被并行请求窃取之前捕获页块, 从而降低了扫描率和压缩成功率.<br>使用了 THPscale 来衡量和测试这组补丁的影响. 基准测试创建一个大文件, 映射它, 使它出错, 在映射中打洞, 使虚拟地址空间碎片化, 然后试图分配THP. 对于不同数量的线程, 它将重新执行. 从碎片的角度来看, 工作负载是相对良性的, 但它会压缩压力. 为迁移而扫描的页面数量减少了65%, 空闲扫描器减少了97.5%. 更少的工作换来更低的延迟和更高的成功率.<br>这组补丁还使用了严重碎片内存的工作负载进行了评估, 但也有很大的好处. | v3 ☑ [5.1-rc1](https://kernelnewbies.org/Linux_5.1#Memory_management) | [PatchWork 00/22](https://lore.kernel.org/patchwork/patch/1033508) |

#### 3.4.3.2 内存规整的 Anti Fragmentation
-------


### 3.4.4 主动规整
-------

[主动规整, 而不是按需规整](https://lwn.net/Articles/817905).

对于正在进行的工作负载活动, 系统内存变得碎片化. 碎片可能会导致容量和性能问题. 在某些情况下, 程序错误也是可能的. 因此, 内核依赖于一种称为内存压缩的反应机制.

该机制的原始设计是保守的, 压缩活动是根据分配请求的要求发起的. 但是, 如果系统内存已经严重碎片化, 反应性行为往往会增加分配延迟. 通过在请求分配之前定期启动内存压缩工作, 主动压缩改进了设计. 这种增强增加了内存分配请求找到物理上连续的内存块的机会, 而不需要内存压缩来按需生成这些内存块. 因此, 特定内存分配请求的延迟降低了. 添加了一个新的 sysctl 接口 `vm.compaction_pro` 来调整内存规整的主动性, 它规定了 kcompactd 试图维护提交的外部碎片的界限.

> 警告 : 主动压缩可能导致压缩活动增加. 这可能会造成严重的系统范围的影响, 因为属于不同进程的内存页会被移动和重新映射. 因此, 启用主动压缩需要非常小心, 以避免应用程序中的延迟高峰.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/06/16 | Nitin Gupta <nigupta@nvidia.com> | [mm: Proactive compaction](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log?id=facdaa917c4d5a376d09d25865f5a863f906234a) | 主动进行内存规整, 而不是之前的按需规整. 新的 sysctl 接口 `vm.compaction_pro` 来调整内存规整的主动性, 它规定了 kcompactd 试图维护提交的外部碎片的界限. | v8 ☑ [5.9-rc1](https://kernelnewbies.org/Linux_5.9#Memory_management) | [PatchWork v8](https://patchwork.kernel.org/project/linux-mm/patch/20200616204527.19185-1-nigupta@nvidia.com), [LORE v8](https://lore.kernel.org/lkml/20200616204527.19185-1-nigupta@nvidia.com), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit?id=facdaa917c4d5a376d09d25865f5a863f906234a) |
| 2021/07/30 | Nitin Gupta <nigupta@nvidia.com> | [mm: compaction: support triggering of proactive compaction by user](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=65d759c8f9f57b96c199f3fe5cfb93ac7da095e9) | 主动压缩每 500ms 触发一次, 并基于设置为 sysctl.compression_proactiveness 的值, 在节点上运行压缩 COMPACTION_HPAGE_ORDER(通常为 ORDER-9) 的页面. 并非所有应用程序都需要每 500ms 触发一次压缩以搜索压缩页面, 特别是在可能只有很少 MB RAM 的嵌入式系统用例上. 这些默认设置将导致嵌入式系统中主动压缩几乎总是在运行.<br>另一方面, 主动压缩对于获取一组高阶页面仍然非常有用. 因此, 在不需要启用主动压缩的系统上, 可以在写入其 sysctl 接口时从用户空间触发主动压缩. 例如, 假设 AppLauncher 决定启动内存密集型应用程序, 如果它获得更多高阶页面, 则可以快速启动该应用程序, 这样 launcher 就可以通过从用户空间触发主动压缩来提前准备系统. | v5 ☑ 5.15-rc1 | [PatchWork v5](https://patchwork.kernel.org/project/linux-mm/patch/1627653207-12317-1-git-send-email-charante@codeaurora.org), [LORE v5](https://lore.kernel.org/all/1627653207-12317-1-git-send-email-charante@codeaurora.org) |



## 3.5 抗碎片化优化
-------



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2005/05/03 | Mel Gorman <mel@csn.ul.ie> | [Avoiding external fragmentation with a placement policy Version 10](https://lore.kernel.org/all/20050503164452.A3AB9E592@skynet.csn.ul.ie) | 20050503164452.A3AB9E592@skynet.csn.ul.ie | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20050503164452.A3AB9E592@skynet.csn.ul.ie) |
| 2005/05/11 | Wolfgang Wander <wwc@rentec.com> | [Avoiding mmap fragmentation (against 2.6.12-rc4) to](https://lore.kernel.org/all/17026.6227.225173.588629@gargle.gargle.HOWL) | 17026.6227.225173.588629@gargle.gargle.HOWL | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/17026.6227.225173.588629@gargle.gargle.HOWL) |
| 2008/08/11 | Christoph Lameter <cl@linux-foundation.org> | [Slab Fragmentation Reduction V14](https://lore.kernel.org/patchwork/patch/125818) | SLAB 抗碎片化 | v14 ☐ | [PatchWork v5](https://lore.kernel.org/patchwork/patch/90742)<br>*-*-*-*-*-*-*-* <br>[PatchWork v14](https://lore.kernel.org/patchwork/patch/125818) |
| 2017/03/07 | Vlastimil Babka <vbabka@suse.cz> | [try to reduce fragmenting fallbacks](https://lore.kernel.org/patchwork/patch/766804) | 修复 [Regression in mobility grouping?](https://lkml.org/lkml/2016/9/28/94) 上报的碎片化问题, 通过修改 fallback 机制和 compaction 机制来减少永久随便化的可能性. 其中 fallback 修改时, 仅尝试从不同 migratetype 的 pageblock 中窃取的页面中挑选最小(但足够)的页面. | v3 ☑ [4.12-rc1](https://kernelnewbies.org/Linux_4.12#Memory_management) | [PatchWork v6](https://lore.kernel.org/patchwork/patch/766804), [KernelNewbies](https://kernelnewbies.org/Linux_4.12#Memory_management), [关键 commit 3bc48f96cf11 ("mm, page_alloc: split least stolen page in fallback")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3bc48f96cf11ce8699e419d5e47ae0d456403274) |
| 2018/11/23 | Mel Gorman | [Fragmentation avoidance improvements v5](https://lore.kernel.org/patchwork/patch/1016503) | 伙伴系统页面分配时的反碎片化 | v5 ☑ 5.0-rc1 | [PatchWork v5](https://lore.kernel.org/patchwork/patch/1016503) |
| 2022/01/27 | Mike Rapoport <rppt@kernel.org> | [Prototype for direct map awareness in page allocator](https://lore.kernel.org/all/20220127085608.306306-1-rppt@kernel.org) | [LSFMM-2022/Solutions for direct-map fragmentation](https://lwn.net/Articles/894557) | v1 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20220127085608.306306-1-rppt@kernel.org) |


# 4 页面回收
-------

[Page replacement for huge memory systems](https://lwn.net/Articles/257541)

[Short topics in memory management](https://lwn.net/Articles/224829)

从用户角度来看, 这一节对了解 Linux 内核发展帮助不大, 可跳过不读; 但对于技术人员来说, 本节可以展现教材理论模型到工程实现的一些思考与折衷, 还有软件工程实践中由简单粗糙到复杂精细的演变过程

当 MM 遭遇内存分配紧张时, 会回收页面. **页框替换算法(Page Frame Replacement Algorithm, 下称PFRA)** 的实现好坏对性能影响很大: 如果选中了频繁或将马上要用的页, 将会出现 **Swap Thrashing** 现象, 即刚换出的页又要换回来, 现象就是系统响应非常慢


[Linux内存回收机制](https://zhuanlan.zhihu.com/p/348873183)

[Linux内存源码分析 - 内存回收(整体流程)](https://www.cnblogs.com/tolimit/p/5435068.html)

内核之所以要进行内存回收, 主要原因有两个:

1.  内核需要为任何时刻突发到来的内存申请提供足够的内存, 以便 cache 的使用和其他相关内存的使用不至于让系统的剩余内存长期处于很少的状态.

2.  当真的有大于空闲内存的申请到来的时候, 会触发强制内存回收.

在大多数情况下内存回收和内存分配总是紧密联系在一起的, 在通常情况下, 总是因为内存分配的过程中发现内存可能不足以分配所需的内存, 因此才需要进行内存回收.

内存分配可以分成快速 fast path(`get_page_from_freelist()`) 和 慢速 slow path(`__alloc_pages_slowpath()`), fast path 失败会走 slow path. 在不同的内存分配路径中, 会触发不同的内存回收方式.

内存回收针对的目标有两种, 一种是针对 zone 的, 另一种是针对一个 memcg 的, 把针对 zone 的内存回收方式分为三种, 分别是快速内存回收、直接内存回收、kswapd 内存回收.

| 回收机制 | 描述 |
|:-------:|:---:|
| 本地快速内存回收机制 `node_reclaim()` | get_page_from_freelist() 函数中进行内存分配时, 在遍历 zonelist 过程中, 对每个 zone 的水线进行判断, 如果分配后 zone 的空闲内存数量 < 阀值 + 保留页框数量, 那么此 zone 就会进行快速内存回收. 其中阀值可能是 min/low/high 的任何一种, 因为在快速内存分配, 慢速内存分配和 oom 分配过程中如果回收的页框足够, 都会调用到 get_page_from_freelist() 函数, 所以快速内存回收不仅仅发生在快速内存分配中, 在慢速内存分配过程中也会发生. |
| 直接内存回收 `__alloc_pages_direct_reclaim()` | 处于慢速分配过程中, 直接内存回收只有一种情况下会使用, 在慢速分配中无法从zonelist的所有zone中以min阀值分配页框, 并且进行异步内存压缩后, 还是无法分配到页框的时候, 就对zonelist中的所有zone进行一次直接内存回收. 注意, 直接内存回收是针对zonelist中的所有zone的, 它并不像快速内存回收和kswapd内存回收, 只会对zonelist中空闲页框不达标的zone进行内存回收. 在直接内存回收中, 有可能唤醒flush内核线程. |
| kswapd 内存回收 | 发生在 kswapd 内核线程中, 当前每个 node 有一个 kswapd 内核线程, 也就是kswapd内核线程中的内存回收, 是只针对所在node的, 并且只会对分配了order页框数量后空闲页框数量 < 此zone的high阀值 + 保留页框数量的zone进行内存回收, 并不会对此node的所有zone进行内存回收. |

页面回收的方式有页回写、页交换和页丢弃三种方式

| 行为 | 描述 |
|:---:|:----:|
| 页面交换 | 对于匿名页, 内存回收过程中会筛选出一些不经常使用的匿名页, 将它们写入到 swap 分区中, 然后作为空闲页框释放到伙伴系统. |
| 页面丢弃 | 对于文件页, 内存回收过程中也会筛选出一些不经常使用的文件页, 如果此文件页中保存的内容与磁盘中文件对应内容一致, 说明此文件页是一个干净的文件页, 就不需要进行回写, 直接将此页丢弃, 作为空闲页框释放到伙伴系统中. |
| 页面写回 | 相反, 如果文件页保存的数据与磁盘中文件对应的数据不一致, 则认定此文件页为脏页, 需要先将此文件页回写到磁盘中对应数据所在位置上, 然后再将此页作为空闲页框释放到伙伴系统中. 内存对匿名页和文件缓存一共用了四条链表进行组织, 回收过程主要是针对这四条链表进行扫描和操作. |


## 4.1 内存回收机制
-------


| 2021/09/20 | BVlastimil Babka <vbabka@suse.cz> @ Linux Kernel Developer, SUSE Labs | [Overview of memory reclaim in the current upstream kernel](https://lkml.org/lkml/2017/8/25/189) | LPC2021 Refereed Track 议题. 主要阐述了目前内存回收的基本思路和发展, 对了解整个内核内存回收是一个非常好的材料. | NA | [SLIDE](https://linuxplumbersconf.org/event/11/contributions/896/attachments/793/1493/slides-r2.pdf) |


### 4.1.1 内存分配的快速路径与快速内存回收机制 `node_reclaim()`
-------

本地快速内存回收机制 `node_reclaim()`, 在内存分配的关键路径 get_page_from_freelist() (内存分配的快速路径)上进行, 因此一直是优化的重点.

*   配置

内核提供了 zone_reclaim_mode 来控制内存 zone 回收模式的开启以及回收行为. zone_reclaim_mode 模式是在 2.6 版本后期开始加入引入的, 可以用来管理当一个内存区域(zone)内部的内存耗尽时, 是从其内部进行内存回收还是可以从其他 zone 进行回收的选项. 过 `/proc/sys/vm/zone_reclaim_mode` 结点来设置.

在使用 get_page_from_freelist() 申请内存时, 内核在当前 zone 内没有足够内存可用的情况下, 会根据 zone_reclaim_mode 的设置来决策是从下一个 zone 找空闲内存还是在 zone 内部进行回收. 当 NUMA 系统某个 node 内存不足的时候, 会从相邻的 node 分配内存.


> 早期的本地快速回收也是基于 zone 的, 因此控制参数名称为 zone_reclaim_mode.
> 随后 [Move LRU page reclaim from zones to nodes v9](https://lore.kernel.org/patchwork/patch/696428) 切到基于 node 的管理方式后, 仍然保留了此配置接口不变.

| zone_reclaim_mode BIT 值 | 行为 | 控制  |
|:------------------------:|:---:|:-----:|
| 0 | 禁用快速内存回收机制 `node_reclaim()`, 此时内存分配时如果某个 zone 的内存不足(不满足水线要求), 则不会尝试触发直接内存回收工作.<br>1. 早期 zone_reclaim 时期直接尝试从下一个 zone 中窃取页面.<br>2. 切到 node_reclaim 之后, 当 NUMA 系统某个 NODE 内存不足的时候, 会从相邻的 NODE 中分配内存. | 关闭时, [不触发 node reclaim](https://elixir.bootlin.com/linux/v5.17/source/mm/page_alloc.c#L4139) |
| 1(RECLAIM_ZONE) | 当前 node 没有足够内存的情况下, 通过 shrink_node() 进行 shrink_lruvec() 和 shrink_slab() 的回收. 对于 NUMA 系统某个 NODE 内存不足的时候, 会从这个 node 本地启动内存回收机制, 避免从其它 NODE 分配内存.  只有这个 node 的 unmmap 的 cache(`node_pagecache_reclaimable(pgdat)`) [大于 min_unmapped_ratio](https://elixir.bootlin.com/linux/v5.17/source/mm/vmscan.c#L4776), 才会启动内存回收. | 参见 node_pagecache_reclaimable() 统计了 zone reclaim 模式可以回收的页面数量. |
| 2(RECLAIM_WRITE) | 默认脏页不会回收, 但是设置该位之后, 可以将 page cache 中的[脏数据(NR_FILE_DIRTY)](https://elixir.bootlin.com/linux/v5.17/source/mm/vmscan.c#L4731)写回硬盘, 以回收内存. | 通过 [scan_control 的 may_writepage](https://elixir.bootlin.com/linux/v5.17/source/mm/vmscan.c#L4754) 来控制. shrink_page_list() 中发现如果 may_writepage 没有被设置, 则会忽略 Dirty 的页面, 不对其进程回收. |
| 4(RECLAIM_UNMAP) | 所有 mapping 的页面(文件页/NR_FILE_PAGES)都可被回收. | 通过 [scan_control 的 may_unmap](https://elixir.bootlin.com/linux/v5.17/source/mm/vmscan.c#L4755) 来控制. shrink_page_list() 中只有设置了发现设置了 may_unmmap 才会回收 page_mapped() 的页面(文件页). |


*   适用场景

参考 [Documentation/admin-guide/sysctl/vm](https://elixir.bootlin.com/linux/v5.12/source/Documentation/admin-guide/sysctl/vm.rst#L981).

默认情况下, zone_reclaim 是禁用的. 因为它处在 get_page_from_freelist() 关键流程中. 且可能会释放 page cache 等页面, 开启它可能增加(快速路径)内存分配的时延, 影响应用的性能. 关掉 zone_reclaim 在很多应用场景下可以提高效率, 比如文件服务器, 或者依赖内存中 page cache 比较多的应用场景. 这样的场景对内存中 page cache 速度的依赖要高于进程进程本身对内存速度的依赖, 所以我们宁可让内存从其他 zone 申请使用, 也不愿意清本地 page cache.

当然如果确定应用场景是内存需求大于缓存 page cache 的, 而且尽量要避免内存访问跨越 NUMA 节点造成的性能下降的话, 则可以打开 zone_reclaim. 此时 zone_reclaim 会优先回收容易回收的可回收内存(主要是当前不用的page cache页), 然后再回收其他内存.

打开本地回收模式的写回可能会引发其他内存节点上的大量的脏数据写回处理. 如果一个内存zone已经满了, 那么脏数据的写回也会导致进程处理速度收到影响, 产生处理瓶颈. 这会降低某个内存节点相关的进程的性能, 因为进程不再能够使用其他节点上的内存. 但是会增加节点之间的隔离性, 其他节点的相关进程运行将不会因为另一个节点上的内存回收导致性能下降.


#### 4.1.1.1 early zone reclaim
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2005/06/21 | Martin Hicks <mort@sgi.com> | [VM: early zone reclaim](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1e7e5a9048b30c57ba1ddaa6cdf59b21b65cde99) | 实现了最初版本的 zone reclaim, 在 `__alloc_pages()` 的过程中, 如果水线不满足要求 !zone_watermark_ok(), 则通过 zone_reclaim() 尝试回收本地 zone 的内存. 引入了 set_zone_reclaim syscall, 用来开启和关闭 zone reclaim. | v1 ☑ 2.6.13-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=753ee728964e5afb80c17659cc6c3a6fd0a42fe0) |
| 2005/08/01 | Mel Gorman <mel@csn.ul.ie> | [remove sys_set_zone_reclaim()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6cb54819d7b1867053e2dfd8c0ca3a8dc65a7eff) | 删除了 set_zone_reclaim syscall | v1 ☑ 2.6.13-rc5 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6cb54819d7b1867053e2dfd8c0ca3a8dc65a7eff) |
| 2005/08/01 | Mel Gorman <mel@csn.ul.ie> | [kill last zone_reclaim() bits](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7756b9e4e321c3c83c7aa5b9532d3e7fd7ddeb4a) | 删除了 set_zone_reclaim syscall. | v1 ☑ 2.6.16-rc1 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7756b9e4e321c3c83c7aa5b9532d3e7fd7ddeb4a) |


#### 4.1.1.2 zone reclaim
-------


当前版本 LRU 是在 zone 上管理的, 因此引入快速页面内存回收的时候, 也是 Zone reclaim 的.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2006/01/18 | Christoph Lameter <clameter@sgi.com> | [Zone reclaim V3: main patch](https://lore.kernel.org/patchwork/patch/47638) | 重构了快速内存回收机制 `zone_reclaim()`.<br>本地快速回收允许在空闲页面数量低于水线的情况下从本地 node/zone 区域内回收页面, 即使其他区域仍然有足够的可用页面. 区域回收对于 NUMA 机器特别重要, 与在远程区域上分配页面所带来的性能损失相比, 回收页面可能更有好处.<br>如果到另一个节点的最大距离高于 RECLAIM_DISTANCE. 如果区域回收没有成功, 那么在一定的时间段内将不会发生进一步的回收尝试(ZONE_RECLAIM_INTERVAL).<br>引入了 [zone_reclaim_mode](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1743660b911bfb849b1fb33830522254561b9f9b). 可以通过 procfs 接口控制. | v4 ☑ v2.6.16-rc2 | [PatchWork v3](https://lore.kernel.org/lkml/20051208203707.30456.57439.sendpatchset@schroedinger.engr.sgi.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v4, 0/3](https://lore.kernel.org/all/20051221210828.3354.23467.sendpatchset@schroedinger.engr.sgi.com), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9eeff2395e3cfd05c9b2e6074ff943a34b0c5c21) |
| 2006/01/18 | Christoph Lameter <clameter@sgi.com> | [mm, numa: reclaim from all nodes within reclaim distance](https://lore.kernel.org/patchwork/patch/326889) | NA | v1 ☑ v2.6.16-rc2 | [PatchWork](https://lore.kernel.org/patchwork/patch/326889)<br>*-*-*-*-*-*-*-* <br>[PatchWork fix](https://lore.kernel.org/patchwork/patch/328345)<br>*-*-*-*-*-*-*-* <br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=957f822a0ab95e88b146638bad6209bbc315bedd) |
| 2009/06/11 | Mel Gorman <mel@csn.ul.ie> | [Fix malloc() stall in zone_reclaim() and bring behaviour more in line with expectations V3](https://lore.kernel.org/patchwork/patch/159963) | NA | v1 ☑ v2.6.16-rc2 | [PatchWork](https://lore.kernel.org/patchwork/patch/159963), [LKML](https://lkml.org/lkml/2009/6/11/143) |


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/09/21 | Mel Gorman <mgorman@techsingularity.net> | [remove zonelist cache and high-order watermark checking v4](https://lore.kernel.org/patchwork/patch/599755) | 引入分区列表缓存(zonelist cache/zlc)是为了跳过最近已知已满的区域. 这避免了昂贵的操作, 如cpuset检查、水印计算和zone_reclaim. 今天的情况不同了, zlc的复杂性更难证明. <br>1. cpuset检查是no-ops, 除非一个cpuset是活动的, 而且通常是非常便宜的.<br>2. zone_reclaim在默认情况下是禁用的, 我怀疑这是zlc想要避免的成本的主要来源. 当启用该功能时, 它将成为节点满时导致暂停的主要原因, 并且让所有其他用户都承受开销是不明智的.<br>3. 对于高阶分配请求, 水印检查的计算代价是昂贵的. 本系列的后续补丁将减少水印检查的成本.<br>4. 最重要的问题是, 在当前的实现中, THP 分配失败可能会导致order-0分配的区域被填满, 并导致回退到远程节点.<br>因此这个补丁尝试删除了 zlc, 这带来了诸多好处. | v1 ☑ 4.4-rc1 | [PatchWork v1](https://lore.kernel.org/patchwork/patch/599762), [关注 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f77cf4e4cc9d40310a7224a1a67c733aeec78836) |
| 2016/04/20 | Dave Hansen <dave.hansen@linux.intel.com> | [OOM detection rework](https://lwn.net/Articles/668126) | 只要还存在可回收的页框, 内核就有充分的理由不启动 OOM Killer. 因此 zone_reclaimable 允许多次重新扫描可回收列表, 并在页面被释放时进行重试. 但是问题在于, 由于多种原因, 我们并不知道内核需要花费多长的时间才可以完成对这些理论上"可回收"的内存页框的实际回收动作. 一种可以预见的情况是, 在回收单个页框时, 分配器会进入无休止的重试, 而那个回收的页框也无法被用于当前的分配请求. 最终的结果是分配尝试一直无法成功, 这导致了内核被挂起, 而且还无法触发 OOM 的处理. 这个补丁更改了 OOM 检测逻辑, 并将其从 shrink_zone() 中提取出来, shrink_zone 太底层, 不适合任何高层决策, 这个决策更适合放在 __alloc_pages_slowpath(), 因为它知道已经做了多少次尝试, 以及到目前为止的进展情况, 因此更适合实现这个逻辑. 新的启发式是在 __alloc_pages_slowpath() 中调用的 should_reclaim_retry() 实现的. 它试图更有确定性, 更容易遵循. 它建立在一个假设上, 即只有当当前可回收的内存+空闲页面允许当前分配请求成功(按照__zone_watermark_ok)时, 重试才有意义, 至少对于可用分区列表中的一个区域. | v6 ☑ 4.7-rc1 | [PatchWork v5,1/11](https://lore.kernel.org/patchwork/patch/664978)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6,10/14](https://lore.kernel.org/patchwork/patch/670859))<br>*-*-*-*-*-*-*-* <br>[关注 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0a0337e0d1d134465778a16f5cbea95086e8e9e0) |
| 2016/06/13 | Minchan Kim <minchan@kernel.org> | [mm: per-process reclaim](https://lore.kernel.org/patchwork/patch/688097) | 这个补丁允许用户空间主动回收进程的页面, 通过把手 "/proc/PID/reclaim" 有效地管理内存, 这样平台可以在任何时候回收任何进程. 一个有用的用例是避免在android中为了获得空闲内存而杀死进程, 这是非常糟糕的体验, 因为当我在享受游戏的同时切换电话后, 我失去了我所拥有的最好的游戏分数, 以及由于冷启动而导致的缓慢启动. 因为冷启动需要加载大量资源数据, 有些游戏需要 15~20 秒, 而成功启动只需要1~5秒. 通过使用新的管理策略来回收perproc, 我们可以大大减少冷启动(即. (171-72), 从而大大减少了应用启动. 该特性的另一个有用功能是方便切换, 这对于测试切换压力和工作负载很有用.  | v2 ☑ 3.16-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/688097) |


#### 4.1.1.3 zone reclaim mode
-------

*   zone_reclaim_mode

[commit 1743660b911b zone_reclaim_mode](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1743660b911bfb849b1fb33830522254561b9f9b) 引入了 zone_reclaim_mode 作为本地快速回收 zone_reclaim() 的开关.

随后,  [commit 1b2ffb7896ad ("Zone reclaim: Allow modification of zone reclaim behavior")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1b2ffb7896ad46067f5b9ebf7de1891d74a4cdef) 修改了 zone_reclaim_mode 接口配置, 允许控制 zone_reclaim_mode 的不同 bit, 来更灵活的控制 zone_reclaim() 的行为.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2006/02/01 | Christoph Lameter <clameter@sgi.com> | [Zone reclaim: Allow modification of zone reclaim behavior](https://lore.kernel.org/patchwork/patch/48460) | 某些情况下, 人们可能希望 zone_reclaim 有不同的行为. 这个补丁允许用户设置 zone_reclaim_mode 的值, 来操纵 zone_reclaim_mode 的行为.<br>最初版引入了以下几种行为控制.<br>1. RECLAIM_ZONE  /* Run shrink_cache on the zone */<br>2. RECLAIM_WRITE /* Writeout pages during reclaim */<br>3. RECLAIM_SWAP  /* Swap pages out during reclaim */<br>在 例如, 一个写大量内存的进程将会溢出到其他节点来缓存写入, 如果一个区域中的许多页面变成脏的. 这可能会影响运行在其他节点上的进程的性能.  在回收期间允许写操作会停止这种行为, 并通过将页面限制在本地区域来限制进程. | v1 ☑ 2.6.16.28 | [PatchWork v4 0/3](https://lore.kernel.org/patchwork/patch/48460), [关键 commit1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1b2ffb7896ad46067f5b9ebf7de1891d74a4cdef) |
| 2014/04/08 | Mel Gorman <mel@csn.ul.ie> | [Disable zone_reclaim_mode by default v2](https://lore.kernel.org/patchwork/patch/454625) | NA | v2 ☑ 3.16-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/454625) |
| 2020/07/01 | Dave Hansen <dave.hansen@linux.intel.com> | [Repair and clean up vm.zone_reclaim_mode sysctl ABI](https://lore.kernel.org/all/20200701152621.D520E62B@viggo.jf.intel.com) | 修正了 zone_reclaim 的文档, 显式声明了 RECLAIM_ZONE, 同时实现了 node_reclaim_enabled() 供其他接口使用. | v3 ☑ 5.12-rc1 | [PatchWork](https://lore.kernel.org/all/20200701152621.D520E62B@viggo.jf.intel.com) |
| 2021/01/26 | Dave Hansen <dave.hansen@linux.intel.com> | [mm/vmscan: replace implicit RECLAIM_ZONE checks with explicit checks](https://lore.kernel.org/patchwork/patch/1266424) | 后来作为 [Migrate Pages in lieu of discard](https://lore.kernel.org/patchwork/patch/1370630) 的前几个补丁, 但是由于逻辑跟这组补丁无关, 被直接先合入. 引入 node_reclaim_mode() 替代了原来的 `node_reclaim_mode == 0` 判断条件. | v1 ☑ 5.13-rc1 | [PatchWork v1](https://lore.kernel.org/patchwork/patch/1266424), [关注 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=202e35db5e71) |

*   RECLAIM_SLAB

随后 zone_reclaim() 中开始支持回收 slab 的内存.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2006/02/01 | Christoph Lameter <clameter@engr.sgi.com> | [Reclaim slab during zone reclaim](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2a16e3f4b0c408b9e50297d2ec27e295d490267) | 引入 RECLAIM_SLAB 全局回收 SLAB 页面.<br>如果大量的 zone 内存被空的 slab 占用, 那么 zone_reclaim 将变得无效. 这个补丁的问题是, slab 回收不能包含到一个区域. 因此, 板坯回收可能会影响整个系统, 而且速度极慢. 这也意味着我们无法确定在这个区域中释放了多少页. 因此, 我们需要离开节点进行至少一次分配. 缺省情况下, 该功能是禁用的. | v1 ☑ 2.6.16-rc2 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2a16e3f4b0c408b9e50297d2ec27e295d490267) |
| 2006/02/01 | Christoph Lameter <clameter@engr.sgi.com> | [zone_reclaim: dynamic slab reclaim](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0ff38490c836dc379ff7ec45b10a15a662f4e5f6) | 如果释放未映射的文件备份页面不足以释放, 只能通过手动配置 RECLAIM_SLAB 来启用 slab 回收, 释放更多内存. 但是这样并不好控制. 因此这个补丁将 slab 回收修改为在 zone reclaim 过程中自动进行. 因此删除了 zone_reclaim_mode 中 RECLAIM_SLAB 选项<br>1. 如果 page cache 数量不足, 则收缩每个节点的页面缓存, 页面大于区域中页面的min_unmapped_ratio百分比.<br>2. 如果可回收 slab 页面的节点数量不足, 则收缩 slab 缓存. 引入了一个 min_slab_ratio 参数控制 slab 的比例. | v1 ☑ 2.6.19-rc1 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0ff38490c836dc379ff7ec45b10a15a662f4e5f6) |

*   RECLAIM_UNMAP

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2006/07/03  | Zhihui Zhang <zzhsuny@gmail.com> | [ZVC/zone_reclaim: Leave 1% of unmapped pagecache pages for file I/O](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9614634fe6a138fd8ae044950700d2af8d203f97) | 引入 min_unmapped_ratio, 控制只有 [NR_FILE_MAPPED 状态的文件页(所有基于文件的未映射页面, 包括 swapcache 页和 tmpfs 文件)所占的百分比](https://elixir.bootlin.com/linux/v2.6.18/source/mm/vmscan.c#L1604) 超过 min_unmapped_ratio 时, 内存域才会执行回收操作. 这样就在本地保留了一部分 page cache 等文件页面, 事实证明, 即使分区的所有页面(或几乎所有页面) 都已分配, 保留一小部分未映射的文件后置页面是有利的. 这允许最近使用的文件 I/O 缓冲区保留在节点上, 并缩短了在我们用完一个区域的内存时发生文件 I/O 时调用 zone_reclaim() 的时间. 当前默认设置 min_unmapped_ratio 为 1, 即保留约 1% 的文件页. | v1 ☑ 2.6.18-rc1 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9614634fe6a138fd8ae044950700d2af8d203f97) |
| 2009/06/16 | Mel Gorman <mel@csn.ul.ie> | [vmscan: properly account for the number of page cache pages zone_reclaim() can reclaim](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=90afa5de6f3fa89a733861e843377302479fcf7e) | [Fix malloc() stall in zone_reclaim() and bring behaviour more in line with expectations V3](https://lore.kernel.org/patchwork/patch/159963) 的其中一个补丁, 这个补丁修改了 zone_reclaim () 计算给定当前 zone_reclaim_mode 下它可能回收多少页面的方式.<br>之前 min_unmapped_ratio 的检查条件中, 使用了 NR_FILE_PAGES 以及 NR_FILE_MAPPED 的所有页面, 这样是有问题的.<br>如果一个大的 tmpfs 挂载占用了很大百分比的内存, 这会造成 malloc() 会停顿很长时间(在某些情况下甚至是几分钟), 因为页面没有被 zone_reclaim() 清理或回收, 反而频繁地扫描列表, 使得 CPU 旋转接近 100%, 但却都是无用功.<br>引入 [zone_pagecache_reclaimable()](https://elixir.bootlin.com/linux/v2.6.31/source/mm/vmscan.c#L2373) 来根据不同的 zone_reclaim_mode 来计算可回收页面数目.<br>1. 如果 RECLAIM_SWAP 被设置, 将[考虑 NR_FILE_PAGES](https://elixir.bootlin.com/linux/v2.6.31/source/mm/vmscan.c#L2385) 作为潜在的候选页进行回收, 或者使用 `NR_{IN}ACTIVE}_PAGES - NR_FILE_MAPPE` 来折价 swapcache 和其他非文件支持的页面(non-file-backed).<br>2. 如果没有设置 RECLAIM_SWAP, 则[不会回收 NR_FILE_MAPPED](https://elixir.bootlin.com/linux/v2.6.31/source/mm/vmscan.c#L2387).<br>3. 如果没有设置 RECLAIM_WRITE, 则 [NR_FILE_DIRTY 页面不会](https://elixir.bootlin.com/linux/v2.6.31/source/mm/vmscan.c#L2391)被作为回收的候选页面. | v1 ☑ 2.6.31-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/159963), [LKML](https://lkml.org/lkml/2009/6/11/143)<br>[关注 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=90afa5de6f3fa89a733861e843377302479fcf7e) |
| 2015/06/24 | Zhihui Zhang <zzhsuny@gmail.com> | [mm: rename RECLAIM_SWAP to RECLAIM_UNMAP](https://lore.kernel.org/patchwork/patch/454625) | RECLAIM_SWAP 感觉上像是我们只处理匿名页面, 但是事实上, 最初引入 min_unmapped_ratio 的逻辑的补丁 [ZVC/zone_reclaim: Leave 1% of unmapped pagecache pages for file I/O](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9614634fe6a138fd8ae044950700d2af8d203f97) 是为了解决与文件页相关的问题, 将其重命名为 RECLAIM_UNMAP 更合适. | v1 ☑ 3.16-rc1 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=95bbc0c7210a7397fec1cd219f896ca95bf29e3e) |

*   RECLAIM_WRITE


#### 4.1.1.4 node reclaim
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/07/08 | Mel Gorman <mgorman@techsingularity.net> | [Move LRU page reclaim from zones to nodes v9](https://lore.kernel.org/patchwork/patch/696428) | 将 LRU 页面的回收从 ZONE 切换到 NODE. 当前我们关注的是[快速内存回收从 zone_reclaim() 切换到了 node_reclaim()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a5f5f91da6ad647fb0cc7fce0e17343c0d1c5a9a). | v9 ☑ [4.8-rc1](https://kernelnewbies.org/Linux_4.8#Memory_management) | [PatchWork v21](https://lore.kernel.org/patchwork/patch/696428), [关注 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a5f5f91da6ad647fb0cc7fce0e17343c0d1c5a9a) |
| 2016/05/31 | Minchan Kim <minchan@kernel.org> | [Support non-lru page migration](https://lore.kernel.org/patchwork/patch/683233) | 支持非lru页面迁移, 主要解决zram和GPU驱动程序造成的碎片问题<br>到目前为止, 我们只允许对 LRU 页面进行迁移, 这足以生成高阶页面. 但是最近, 嵌入式系统以及终端等设备使用了大量不可移动的页面(如zram、GPU内存), 因此我们看到了一些关于小高阶分配问题的报道. 问题主要是由<br>1. zram 和 GPU 驱动造成的碎片. 在内存压力下, 它们的页面被分散到所有的页块中, 无法进行迁移, 内存规整也不能很好地工作, 所以收缩业务所有的页面, 使得系统非常慢.<br>2. 另一个问题是他们不能使用CMA内存空间, 所以当OOM kill发生时, 我可以在CMA区域看到很多空闲页面, 这不是内存效率. 我们的产品有很大的CMA内存, 虽然CMA中有很大的空闲空间, 但是过多的回收区域来分配GPU和zram页面, 很容易使系统变得很慢. 之前为了解决这个问题, 我们做了几项努力(例如, 增强压缩算法、SLUB回退到0阶页、保留内存、vmalloc等), 但如果系统中存在大量不可移动页, 则从长远来看, 它们的解决方案是无效的. 因此尝试了当前这种方案.  | v2 ☑ [4.8-rc1](https://kernelnewbies.org/Linux_4.8#Memory_management) | [PatchWork](https://lore.kernel.org/patchwork/patch/683233) |


#### 4.1.1.5 与 NUMA 的冲突
-------

[十年后数据库还是不敢拥抱 NUMA?](https://zhuanlan.zhihu.com/p/387117470)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2014/04/07 | Mel Gorman <mgorman@suse.de> | [Disable zone_reclaim_mode by default](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=5f7a75acdb24c7b9c436b3a0a66eec12e101d19c) | 1396910068-11637-1-git-send-email-mgorman@suse.de | v1 ☑✓ 3.16-rc1 | [LORE v1,0/2](https://lore.kernel.org/all/1396910068-11637-1-git-send-email-mgorman@suse.de)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/2](https://lore.kernel.org/lkml/1396945380-18592-1-git-send-email-mgorman@suse.de) |



### 4.1.2 内存回收的慢速路径与直接内存回收 Direct Reclaim
-------

#### 4.1.2.1 最早的直接回收机制
-------


[commit 1fc53b2209b5 ("2.4.0-test9pre1, MM balancing (Rik Riel)")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1fc53b2209b58e786c102e55ee682c12ffb4c794) 引入 active/inactive LRU 算法的时候, 实现了 [Direct Reclaim 机制](https://elixir.bootlin.com/linux/2.4.0/source/mm/page_alloc.c#L299). LRU 中将 inactive 分为 clean page 和 dirty page, 维护了一张干净的 [inactive_clean_list](https://elixir.bootlin.com/linux/2.4.0/source/mm/vmscan.c#L655), 这个列表中的页面是干净的, 可以直接会复用(回收再分配).

`__alloc_pages()` 中引入了 `__alloc_pages_limit()` 和 reclaim_page() 来完成直接回收的工作. 其中 reclaim_page() 直接从 [inactive (clean) LRU list](https://elixir.bootlin.com/linux/2.4.0/source/include/linux/mmzone.h#L38) 中回收并返回一张页面到空闲列表,  而 `__alloc_pages_limit()` 允许从空闲页面和 inactive LRU list 中直接回收并分配页面.

`__alloc_pages_limit()` 中, 如果[页面接近 min 水线](https://elixir.bootlin.com/linux/2.4.0/source/mm/page_alloc.c#L255), 则尝试通过 reclaim_page() [从 inactive LRU list 中直接回收并返回一张页面](https://elixir.bootlin.com/linux/2.4.0/source/mm/page_alloc.c#L255)出来以供使用. 反之或者分配失败, 则依旧走 rmqueue() 进行分配.

当然实际的策略其实远比我描述的复杂, 考虑到高阶分配或者内存紧缺的情况下, `__alloc_pages_limit()` 依旧会失败

对于高阶分配, 只要[没设置 PF_MEMALLOC, 并且允许 `__GFP_WAIT`](https://elixir.bootlin.com/linux/2.4.0/source/mm/page_alloc.c#L417), 只要 inactive LRU list 有[足够的干净页面](https://elixir.bootlin.com/linux/2.4.0/source/mm/page_alloc.c#L429), 就不断通过 `reclaim_page() + __free_page() +  rmqueue()` 的组合来分配一个高阶页面.

这就是最早期的 Direct Reclaim 机制, inactive LRU 分为 inactive_dirty_pages list 和 inactive_clean_pages list, reclaim_page() 尝试直接从 inactive_clean_list 中移动一张页面到空闲列表从而完成分配.


#### 4.1.2.2 内存分配的慢速路径
-------

[commit a880f45a48be ("v2.4.9.11, Andrea Arkangeli: major VM merge")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a880f45a48be2956d2c78a839c472287d54435c1) 优化了 LRU, 不再区分 inactive_dirty_pages 和 inactive_clean_pages. 引入了 balance_classzone() 替代了 `__alloc_pages_limit()` 和 `reclaim_page()` 机制. 在这里 `__alloc_pages()` 被分割为 Fast Path 和 [Slow Path](https://elixir.bootlin.com/linux/2.4.9.11/source/mm/page_alloc.c#L354). 在慢速路径中, 通过 `balance_classzone() -=> try_to_free_pages()` 不断提升优先级进行直接回收 `shrink_caches()/swap_out`, 每次尝试扫描 SWAP_CLUSTER_MAX 张页面.

```cpp
__alloc_pages()
    -=> balance_classzone()
        -=> try_to_free_pages()
            -=> for each priority
                -=> shrink_caches()
                -=> swap_out()
```

随后 2.5.45 [commit 1d2652dd2c3e ("hot-n-cold pages: bulk page freeing")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1d2652dd2c3e942e75dc3137b3cb1774b43ae377) 区分热页和冷页并进行批量回收的时候, 移除了 balance_classzone(), 慢速路径直接调用 try_to_free_pages() 完成直接回收. [commit ac12db05e309 ("vm: alloc_pages watermark fixes")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ac12db05e3093de2624d842dc2677621f49d0d74), 直接称这一时期的直接回收为 synchronous reclaim, 这已经很贴切了. 这就是现如今内核直接回收的雏形.

```cpp
__alloc_pages()
    -=> try_to_free_pages()
        -=> for each priority
            -=> shrink_caches()
            -=> shrink_slab()
```

接着 09 年 Mel Gorman 在 [2.6.31, 优化 Page Allocator 的过程](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=72807a74c0172376bba6b5b27702c9f702b526e9)中, 将内存分配的核心函数 `__alloc_pages()` 显式分解为快速路径 `get_page_from_freelist()` 和缓慢路径 `__alloc_pages_slowpath()`. 并对慢速路径的主体流程进行了拆解: `__alloc_pages_high_priority()`, `__alloc_pages_direct_reclaim()` 以及 `__alloc_pages_may_oom()`. 参见 [commit 11e33f6a55ed ("page allocator: break up the allocator entry point into fast and slow paths")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=11e33f6a55ed7847d9c8ffe185ef87faf7806abe). 这时候慢速路径的框架已经有了现在的影子.

```cpp
__alloc_pages_nodemask()
    -=> get_page_from_freelist()
    -=> __alloc_pages_slowpath()
        -=> get_page_from_freelist()
        -=> __alloc_pages_high_priority()
        -=> __alloc_pages_direct_reclaim()
        -=> __alloc_pages_may_oom()
```

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/02/04 | Linus Torvalds <torvalds@athlon.transmeta.com> | [v2.4.9.11, Andrea Arkangeli: major VM merge](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a880f45a48be2956d2c78a839c472287d54435c1) | 不再区分 inactive_dirty_pages 和 inactive_clean_pages. `__alloc_pages()` 被分割为 Fast Path 和 [Slow Path](https://elixir.bootlin.com/linux/2.4.9.11/source/mm/page_alloc.c#L354). | v1 ☑✓ 2.4.9.11 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a880f45a48be2956d2c78a839c472287d54435c1) |
| 2002/10/29 | Jan Kara <jack@suse.cz> | [hot-n-cold pages: bulk page allocator](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=8d6282a1cf812279f490875cd55cb7a85623ac89) | 区分热门页面和冷页面, 区分热页和冷页并进行批量回收的时候, 移除了 balance_classzone(), 慢速路径直接调用 try_to_free_pages() 完成直接回收. | v2 ☑ 2.5.45 | [CGIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1d2652dd2c3e942e75dc3137b3cb1774b43ae377) |
| 2004/08/23 | Nick Piggin <nickpiggin@yahoo.com.au> | [vm: alloc_pages watermark fixes](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ac12db05e3093de2624d842dc2677621f49d0d74) | 修复异步回收的逻辑. 由于 page_min 水线为 `__GFP_HIGH` 和 `PF_MEMALLOC` 分配保留的. 页面水线达到 pages_low + protection 时, 内存分配器将尝试唤醒 KSWAPD 进行异步回收, 直到达到 ->pages_high 水线为止. 在唤醒 KSWAPD 之后, 可以在不阻塞的情况下再次尝试分配. 这里我们关注的是它显式通过注释 "We now go into synchronous reclaim", 标记了直接回收的开始. | v1 ☑✓ 2.6.9-rc2 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ac12db05e3093de2624d842dc2677621f49d0d74) |
| 2009/04/22 | Mel Gorman <mel@csn.ul.ie> | [page allocator: break up the allocator entry point into fast and slow paths](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=11e33f6a55ed7847d9c8ffe185ef87faf7806abe) | 清理和优化页面分配器 [Cleanup and optimise the page allocator V7](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=72807a74c0172376bba6b5b27702c9f702b526e9) 的其中一个补丁. 将内存分配的核心函数 `__alloc_pages()` 显式分解为快速路径 `get_page_from_freelist()` 和缓慢路径 `__alloc_pages_slowpath()`. | v7 ☑✓ 2.6.31-rc1 | [LORE RFC,00/20](https://lore.kernel.org/lkml/1235344649-18265-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE RFC,v2,00/19](https://lore.kernel.org/lkml/1235477835-14500-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE RFC,v3,00/35](https://lore.kernel.org/lkml/1237196790-7268-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE RFC, v4,00/26](https://lore.kernel.org/lkml/1237196790-7268-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE RFC,v5,00/25](https://lore.kernel.org/lkml/1237543392-11797-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE v6,00/25](https://lore.kernel.org/lkml/1240266011-11140-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/22](https://lore.kernel.org/lkml/1240408407-21848-1-git-send-email-mel@csn.ul.ie) |

#### 4.1.2.3 __alloc_pages_high_priority
-------


[commit 11e33f6a55ed ("page allocator: break up the allocator entry point into fast and slow paths")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=11e33f6a55ed7847d9c8ffe185ef87faf7806abe) 从分配流程中拆解出慢速路径 `__alloc_pages_slowpath()` 时就引入了 `is_allocation_high_priority()` 和 `__alloc_pages_high_priority()`,

如果分配请求是 `__GFP_NOFAIL` 的, 那么 `__alloc_pages_high_priority()` 就循环循环通过 get_page_from_freelist 忽略水线(ALLOC_NO_WATERMARKS) 去请求分配页面. 这其实是非常脆弱的, 因为我们当前并没有足够的页面来完成分配, 所以这里基本上是依靠别人来为我们进行回收(不管是异步回收, 直接回收还是 OOM Killer). 他们可能竞争某些资源(例如锁等), 从而阻止其他回收者取得任何进展. 因此 4.5 期间, [`get rid of __alloc_pages_high_priority`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=33d5310306ec244d96533da5f9183e05a7a51106) 这组补丁[删除了 `__alloc_pages_high_priority()` 不断重试的循环](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=fde82aaa731de8a23d817971f6080041a4917d06), 直接通过 get_page_from_freelist() 请求页面, 并依赖 `__alloc_pages_slowpath()` 中的其他回收和重试操作来达成分配的请求. 有一点是需要谨慎的, 那就是 PF_MEMALLOC 上下文中的 `__GFP_NOFAIL` 分配, 它根本不能保证任何进展. 但是我们不能排除没有内核路径可能会有这种组合.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2015/11/16 | mhocko@kernel.org <mhocko@kernel.org> | [`get rid of __alloc_pages_high_priority`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=33d5310306ec244d96533da5f9183e05a7a51106) | 1447680139-16484-1-git-send-email-mhocko@kernel.org | v1 ☑✓ 4.5-rc1 | [LORE RFC](https://lore.kernel.org/all/1447343618-19696-1-git-send-email-mhocko@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/2](https://lore.kernel.org/all/1447680139-16484-1-git-send-email-mhocko@kernel.org) |

fde82aaa731de8a23d817971f6080041a4917d06

#### 4.1.2.4 慢速路径的内存规整
-------

1.  引入了回收规整(`reclaim/compaction`) 后, `__alloc_pages_slowpath()` 中可能会进行两次直接规整.

第一次是在直接回收 `__alloc_pages_direct_reclaim()` 之前, 如果发现高阶内存分配失败不是因为内存不足, 而是因为内存碎片比较严重, 则通过 `__alloc_pages_direct_compact()` 尝试规整出足够的连续内存以供分配.

2.6.35-rc1, Mel Gorman 在引入内存规整 [Memory Compaction v8](https://lore.kernel.org/lkml/1271797276-31358-1-git-send-email-mel@csn.ul.ie) 的过程中. [mm: compaction: direct compact when a high-order allocation fails](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=56de7263fcf3eb10c8dcdf8d59a9cec831795f3f) 就在内存分配的回收路径引入了直接内存规整 [`__alloc_pages_direct_compact()`](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L1783).


第二次是在直接回收 `__alloc_pages_direct_reclaim()` 回收了足够多的内存之后, 则会使用 `should_alloc_retry()` 检测是否可以分配出足够的内存, 如果不行, 则将再次进行直接规整 `__alloc_pages_direct_compact()` 尝试在回收之后规整出分配所需的连续页面出来.

在进行直接回收 [`__alloc_pages_direct_reclaim()`](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L2032) 之前, [`__alloc_pages_high_priority()`](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L2003) 之后, 通过[直接规整 `__alloc_pages_direct_compact()`](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L2023) 的内存 fragmentation_index() 来确认当前[高阶内存分配](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L1790)失败是[内存不足](https://elixir.bootlin.com/linux/v2.6.35/source/mm/compaction.c#L499)还是因为[外部碎片](https://elixir.bootlin.com/linux/v2.6.35/source/mm/compaction.c#L496)造成的, 如果发现是因为外部碎片造成的, 就会通过 `try_to_compact_pages() -=> compact_zone_order()` 尝试规整出足够大小的页面来[完成页面分配](https://elixir.bootlin.com/linux/v2.6.35/source/mm/page_alloc.c#L1801). 另外如果内存规整也无法释放合适大小的页面来完成页面分配, 则依旧会进行直接回收. 由于在内存分配(慢速)路径进行, 直接规整不能耗时过长, 应该尽快返回. 因此在规整每个 ZONE 时, 都检查是否释放了合适 order 的页面, 如果释放了, 则返回.

至此, 一直到 v4.7 期间, 慢速路径的分配流程都如下所示:

```cpp
__alloc_pages_slowpath()
{
retry:
        if (gfp_mask & __GFP_KSWAPD_RECLAIM)
                wake_all_kswapds(order, ac);

        page = get_page_from_freelist(gfp_mask, order, alloc_flags & ~ALLOC_NO_WATERMARKS, ac);

        /* Allocate without watermarks if the context allows */
        if (alloc_flags & ALLOC_NO_WATERMARKS) {
                page = get_page_from_freelist(gfp_mask, order, ALLOC_NO_WATERMARKS, ac);

        /*
         * Try direct compaction. The first pass is asynchronous. Subsequent
         * attempts after direct reclaim are synchronous
         */
        page = __alloc_pages_direct_compact(gfp_mask, order, alloc_flags, ac, migration_mode, &compact_result);

        /* Try direct reclaim and then allocating */
        page = __alloc_pages_direct_reclaim(gfp_mask, order, alloc_flags, ac, &did_some_progress);

        if (should_reclaim_retry(gfp_mask, order, ac, alloc_flags, did_some_progress > 0, no_progress_loops))
                goto retry;

        /*
         * It doesn't make any sense to retry for the compaction if the order-0
         * reclaim is not able to make any progress because the current
         * implementation of the compaction depends on the sufficient amount
         * of free memory (see __compaction_suitable)
             */
        if (did_some_progress > 0 && should_compact_retry(ac, order, alloc_flags, compact_result, &migration_mode, compaction_retries))
                goto retry;

        page = __alloc_pages_may_oom(gfp_mask, order, ac, &did_some_progress);

        if (is_thp_gfp_mask(gfp_mask) && !(current->flags & PF_KTHREAD))
                migration_mode = MIGRATE_ASYNC;
        else
                migration_mode = MIGRATE_SYNC_LIGHT;
        page = __alloc_pages_direct_compact(gfp_mask, order, alloc_flags, ac, migration_mode, &compact_result);

```

`__alloc_pages_slowpath()` 中的 retry 循环逻辑应该持续地尝试回收和压缩(和 OOM), 直到分配成功, 或返回失败. 且需要准寻两个必要的原则:

1.      首先, 当回收先于压缩时, 成功的可能性更大, 因为必须满足某些水印才能进行压缩, 而且更多的空闲页面增加了压缩成功的可能性.

2.      其次, 从轻度异步压缩开始(如果水印允许的话)可以更高效, 特别是对于 order 的分配请求, 如果是有足够的空闲内存, 但是只是碎片化稍微有点严重的情况.

但是当前的逻辑(参见 v4.7)在尝试 retry 时先[进行直接规整](https://elixir.bootlin.com/linux/v4.7/source/mm/page_alloc.c#L3667), 再[进行直接回收](https://elixir.bootlin.com/linux/v4.7/source/mm/page_alloc.c#L3697), 并且为了确保最后一次回收之后总是有一个最终的压缩, 在[循环的最后 `__alloc_pages_may_oom()` 之后还会有另一个直接的压缩调用](https://elixir.bootlin.com/linux/v4.7/source/mm/page_alloc.c#L3764). 这使得代码难以理解, 并增加了一些对 migration_mode 决策的重复处理. 即使回收或规整决定不重试, 最终的规整仍然会被尝试, 这也有点低效. 一些 gfp 标志组合也[通过 "goto noretry" 来简化重试决定](https://elixir.bootlin.com/linux/v4.7/source/mm/page_alloc.c#L3750), 这使得它更加难以遵循.

鉴于此等诸多问题, v4.9 对直接规整的效率做了进一步的提升和改进, [make direct compaction more deterministic v3,00/17](https://lore.kernel.org/lkml/201606240
95437.16385-1-vbabka@suse.cz). 其中 [commit a8161d1ed609 ("mm, page_alloc: restructure direct compaction handling in slowpath")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a8161d1ed6098506303c65b3701dedba876df42a) 对慢速路径下直接规整的顺序和逻辑做了重构, 整个 retry 操作(回收规整)持续进行: "分配-=>回收-=>规整-=>再分配" 的循环闭包, OOM 决策之后, 也不再需要一个直接规整. 整体逻辑也更加清晰.

```cpp
__alloc_pages_slowpath()
{
        /*
         * The adjusted alloc_flags might result in immediate success, so try
         * that first
         */
        page = get_page_from_freelist(gfp_mask, order, alloc_flags, ac);

        /*
         * For costly allocations, try direct compaction first, as it's likely
         * that we have enough base pages and don't need to reclaim. Don't try
         * that for allocations that are allowed to ignore watermarks, as the
         * ALLOC_NO_WATERMARKS attempt didn't yet happen.
         */
        if (can_direct_reclaim && order > PAGE_ALLOC_COSTLY_ORDER && !gfp_pfmemalloc_allowed(gfp_mask)) {
                page = __alloc_pages_direct_compact(gfp_mask, order, alloc_flags, ac, INIT_COMPACT_PRIORITY, &compact_result);

retry:
        /* Ensure kswapd doesn't accidentally go to sleep as long as we loop */
        if (gfp_mask & __GFP_KSWAPD_RECLAIM)
                wake_all_kswapds(order, ac);

        /* Attempt with potentially adjusted zonelist and alloc_flags */
        page = get_page_from_freelist(gfp_mask, order, alloc_flags, ac);

        /* Try direct reclaim and then allocating */
        page = __alloc_pages_direct_reclaim(gfp_mask, order, alloc_flags, ac, &did_some_progress);

        /* Try direct compaction and then allocating */
        page = __alloc_pages_direct_compact(gfp_mask, order, alloc_flags, ac, compact_priority, &compact_result);

        if (should_reclaim_retry(gfp_mask, order, ac, alloc_flags, did_some_progress > 0, &no_progress_loops))
                goto retry;

        /*
         * It doesn't make any sense to retry for the compaction if the order-0
         * reclaim is not able to make any progress because the current
         * implementation of the compaction depends on the sufficient amount
         * of free memory (see __compaction_suitable)
         */
        if (did_some_progress > 0 && should_compact_retry(ac, order, alloc_flags, compact_result, &compact_priority, &compaction_retries))
                goto retry;

        /* Reclaim has failed us, start killing things */
        page = __alloc_pages_may_oom(gfp_mask, order, ac, &did_some_progress);
}
```


#### 4.1.2.5 `__alloc_pages_may_oom()`
-------

[oom detection rework v6](https://lore.kernel.org/lkml/1461181647-8039-1-git-send-email-mhocko@kernel.org)

内存分配的慢速路径上, 经常要探测是要触发 OOM 还是再次尝试进行分配.

1.  通过 did_some_progress 跟踪上一轮分配或则回收的进度, 使用 no_progress_loops 记录尝试回收的次数.

2.  通过 should_alloc_retry(), [should_reclaim_retry()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0a0337e0d1d134465778a16f5cbea95086e8e9e0), [should_compact_retry()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=33c2d21438daea807947923377995c73ee8ed3fc) 在慢速路径的各个阶段判断是进行下一步操作, 还是直接尝试重新分配.

*   did_some_progress

[commit bb459c6567e4 ("mm: fix several oom killer bugs")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bb459c6567e40d45ddb220fcea82296810094f19) 引入了 did_some_progress, 在 try_to_free_pages() 回收了内存之后, 就不再倾向于触发 OOM.

随后 [commit 11e33f6a55ed ("page allocator: break up the allocator entry point into fast and slow paths")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=11e33f6a55ed7847d9c8ffe185ef87faf7806abe) 将 `__alloc_pages_slowpath()` 进行了拆解之后, did_some_progress 开始贯穿于 slowpath 的所有子路径中:

1.  直接回收 `__alloc_pages_direct_reclaim()` 中 `try_to_free_pages()` 会收到了内存, 才会重新尝试分配 get_page_from_freelist().

2.  也只有直接回收没有回收到内存的时候, 才会使用 `__alloc_pages_may_oom()` 做 OOM 前最后的挣扎.

3.  如果回收了足够的内存, 才会通过 should_alloc_retry() 检查是否需要重新尝试分配.

```cpp
__alloc_pages_direct_reclaim()
{
    *did_some_progress = try_to_free_pages(zonelist, order, gfp_mask, nodemask);
    if (likely(*did_some_progress))
        page = get_page_from_freelist(gfp_mask, nodemask, order, zonelist, high_zoneidx, alloc_flags);

}

__alloc_pages_slowpath()
{
    page = __alloc_pages_direct_reclaim(gfp_mask, order,
                                        zonelist, high_zoneidx,
                                        nodemask,
                                        alloc_flags, &did_some_progress);

    if (!did_some_progress) {
        if ((gfp_mask & __GFP_FS) && !(gfp_mask & __GFP_NORETRY)) {
            page = __alloc_pages_may_oom(gfp_mask, order, zonelist, high_zoneidx, nodemask);
    }

    pages_reclaimed += did_some_progress;

    if (should_alloc_retry(gfp_mask, order, pages_reclaimed)) {
        /* Wait for some write requests to complete then retry */
        congestion_wait(WRITE, HZ/50);
        goto rebalance;
    }
}
```

接着 [commit 9879de7373fc ("mm: page_alloc: embed OOM killing naturally into allocation slowpath")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9879de7373fcfb466ec198293b6ccc1ad7a42dd8) 在优化分配慢速路径下 OOM 的重复检查的时候, 为 `__alloc_pages_may_oom()` 也引入了 did_some_progress.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2005/02/01 | Andrea Arcangeli <andrea@suse.de> | [mm: oom-killer tunable](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bb459c6567e40d45ddb220fcea82296810094f19) | oom killer 补丁的核心, 部分借鉴了 Thomas 从退出路径获取反馈的补丁的思想, 将 oom killer 移到了 page_alloc 路径下, 因为这里应该能够在杀死更多的东西之前检查水印. 这样不再需要等待 5 秒, oom killer 也会非常理智. 这个补丁引入了 did_some_progress(try_to_free_pages() 的返回值) 和 out_of_memory(). 只要回收了定量的页面, 就不轻易进行 OOM. | v1 ☑✓ 2.6.11-rc3 | [CGIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bb459c6567e40d45ddb220fcea82296810094f19), [关注 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=86a4c6d9e2e43796bb362debd3f73c0e3b198efa) |
| 2014/12/05 | Johannes Weiner <hannes@cmpxchg.org> | [mm: page_alloc: embed OOM killing naturally into allocation slowpath](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9879de7373fcfb466ec198293b6ccc1ad7a42dd8) | 为了判断是否需要进行 OOM 的检查与调用, 在内存分配的 slowpath 行大量的重复检查, 将 `__alloc_pages_may_oom()` 的调用路径放到 should_alloc_retry() 分支下, 移除了 oom_gfp_allowed(), 将它的判断逻辑分散到了各个流程和 should_alloc_retry() 中. | v1 ☑✓ 3.19-rc7 | [LORE](https://lore.kernel.org/all/1417790893-32010-1-git-send-email-hannes@cmpxchg.org), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9879de7373fcfb466ec198293b6ccc1ad7a42dd8) |

*   no_progress_loops

commit 423b452e1553e3d19b632880bf2adf1f058ab267
Author: Vlastimil Babka <vbabka@suse.cz>
Date:   Fri Oct 7 17:00:40 2016 -0700

    mm, page_alloc: pull no_progress_loops update to should_reclaim_retry()


*   should_alloc_retry()

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2014/12/05 | Johannes Weiner <hannes@cmpxchg.org> | [mm: page_alloc: embed OOM killing naturally into allocation slowpath](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9879de7373fcfb466ec198293b6ccc1ad7a42dd8) | 为了判断是否需要进行 OOM 的检查与调用, 在内存分配的 slowpath 行大量的重复检查, 将 `__alloc_pages_may_oom()` 的调用路径放到 should_alloc_retry() 分支下, 移除了 oom_gfp_allowed(), 将它的判断逻辑分散到了各个流程和 should_alloc_retry() 中. | v1 ☑✓ 3.19-rc7 | [LORE](https://lore.kernel.org/all/1417790893-32010-1-git-send-email-hannes@cmpxchg.org), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9879de7373fcfb466ec198293b6ccc1ad7a42dd8) |
| 2015/03/25 | Johannes Weiner <hannes@cmpxchg.org> | [mm: page_alloc: inline should_alloc_retry()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9083905a2a13dec4093a9c35a9b7f60037b87672) | [mm: page_alloc: improve OOM mechanism and policy](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9083905a2a13dec4093a9c35a9b7f60037b87672) 的其中一个补丁, should_alloc_retry() 函数旨在封装内存分配器 slowpath 的重试条件, 但是主函数 `__alloc_pages_slowpath()` 中仍有剩余的检查, 重试的执行方式在很大程度上也取决于进程. 这些条件的物理分离使得代码难以理解. 因此这个补丁直接去掉了 should_alloc_retry() 函数, 将重试的逻辑分散在了主函数流程中. | v2 ☑✓ 4.2-rc1 | [LORE v1,0/12](https://lore.kernel.org/all/1427264236-17249-1-git-send-email-hannes@cmpxchg.org)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/9](https://lore.kernel.org/all/1430161555-6058-1-git-send-email-hannes@cmpxchg.org) |

*   [should_reclaim_retry()](https://elixir.bootlin.com/linux/v4.7/source/mm/page_alloc.c#L3489)

[commit 0a0337e0d1d1 ("mm, oom: rework oom detection")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0a0337e0d1d134465778a16f5cbea95086e8e9e0) 实现了回收反馈 [should_reclaim_retry()](https://elixir.bootlin.com/linux/v4.7/source/mm/page_alloc.c#L3489). 这是直接回收的 FreeBack. 用于在直接回收 `__alloc_pages_direct_reclaim()` 后检查是否需要再次尝试分配和回收.

如果持续回收内存都是失败的, 则[通过 no_progress_loops 记录其失败重试次数](https://elixir.bootlin.com/linux/v4.7/source/mm/page_alloc.c#L3721), 连续[超过 MAX_RECLAIM_RETRIES 次重试后](https://elixir.bootlin.com/linux/v4.7/source/mm/page_alloc.c#L3500), 将不再继续重试, 而是准备启动 OOM 流程.

如果前面成功回收到了内存(did_some_progress > 0), [则 no_progress_loops 重新计数](https://elixir.bootlin.com/linux/v4.7/source/mm/page_alloc.c#L3718).


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/04/20 | Michal Hocko <mhocko@kernel.org> | [mm, oom: rework oom detection](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0a0337e0d1d134465778a16f5cbea95086e8e9e0) | [oom detection rework v6](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f44666b04605d1c7fd94ab90b7ccf633e7eff228) 系列的其中一个补丁. | v1 ☑✓ 4.7-rc1 | [LORE 00/14](https://lore.kernel.org/lkml/1461181647-8039-1-git-send-email-mhocko@kernel.org) |
| 2016/04/12 | Michal Hocko <mhocko@kernel.org> | [oom: consider multi-threaded tasks in task_will_free_mem](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=98748bd722005be9de2662bd4f7e41ad8148bdbd) | 1460452756-15491-1-git-send-email-mhocko@kernel.org | v1 ☑✓ 4.7-rc1 | [LORE](https://lore.kernel.org/all/1460452756-15491-1-git-send-email-mhocko@kernel.org), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=98748bd722005be9de2662bd4f7e41ad8148bdbd) |
| 2016/04/26 | Michal Hocko <mhocko@kernel.org> | [last pile of oom_reaper patches for now](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f44666b04605d1c7fd94ab90b7ccf633e7eff228) | 1461679470-8364-1-git-send-email-mhocko@kernel.org | v1 ☑✓ v4.7-rc1 | [LORE v1,0/2](https://lore.kernel.org/all/1461679470-8364-1-git-send-email-mhocko@kernel.org) |


*   should_compact_retry()


但是我们通过了 order-0 的 watermak 检查, 如果没有符合条件的区域有任何请求或更高的订单页面可用, should_reclaim_retry() 也会放弃分配的重试.  这样做是因为不能保证可回收的和当前空闲的页面能够满足当前高阶页面的分配. 但是, 这可能会导致高阶请求(例如. fork 过程中堆栈分配所需的 order-2 页面) 失败而将过早触发 OOM.

为了防止这种情况出现, 回收再规整之后. 没有任何证据证明再次进行回收压缩会有所帮助, [commit 33c2d21438da ("mm, oom: protect !costly allocations some more")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=33c2d21438daea807947923377995c73ee8ed3fc) 引入 should_reclaim_retry() 来完成是这个判断. 并在触发 OOM 之前, 尝试 MAX_COMPACT_RETRIES 次重试. 从而保证回收再压缩尽了自己所能. 直接规整是 MIGRATE_ASYNC, 这是相当弱的, 因为它忽略回写下的页面, 在其他情况下很容易放弃. 因此回收再规整使用了 MIGRATE_SYNC_LIGHT 模式. 有了这个逻辑, 我们就不必无条件地增加迁移模式, 而只是在较弱模式的压缩失败时才这样做. 只在真正需要的时候才使用更强的迁移模式, 才使用同步迁移.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/04/20 | Michal Hocko <mhocko@kernel.org> | [mm, oom: protect !costly allocations some more](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=33c2d21438daea807947923377995c73ee8ed3fc) | [oom detection rework v6](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f44666b04605d1c7fd94ab90b7ccf633e7eff228) 的其中一个补丁. | v1 ☑✓ 4.7-rc1 | [LORE 00/14](https://lore.kernel.org/lkml/1461181647-8039-1-git-send-email-mhocko@kernel.org), [关注 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=33c2d21438daea807947923377995c73ee8ed3fc) |
| 2016/09/06 | Vlastimil Babka <vbabka@suse.cz> | [reintroduce compaction feedback for OOM decisions](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9f7e3387939b036faacf4e7f32de7bb92a6635d6) | 20160906135258.18335-1-vbabka@suse.cz | v1 ☑✓ 4.9-rc1 | [LORE v1,0/4](https://lore.kernel.org/all/20160906135258.18335-1-vbabka@suse.cz) |


### 4.1.3 KSWAPD 内核 Balancing
-------


## 4.2 LRU
-------

[A Linux Kernel Miracle Tour - 内存回收](https://blog.csdn.net/chuck_huang/article/details/80212666)

### 4.2.1 经典地 LRU 算法
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2005/06/21 | Martin Hicks <mort@sgi.com> | [Swap reorganised 29.12.95](https://github.com/gatieme/linux-history/commit/ecda87c064fea1d7322a357b78b99e04594bc6f3) | 引入 zone_reclaim syscall, 用来回收 zone 的内存. | v1 ☑ 1.3.57 | [commit HISTORY](https://github.com/gatieme/linux-history/commit/ecda87c064fea1d7322a357b78b99e04594bc6f3) |
| 2.4.0-test9pre1 | Rik van Riel <riel@redhat.com> | [MM balancing (Rik Riel)](https://github.com/gatieme/linux-history/commit/1fc53b2209b58e786c102e55ee682c12ffb4c794) | 引入 MM balancing, 其中将 LRU 拆分成了 active_list 和 inactive_dirty_list 两条链表 | 2.4.0-test9pre1 | [1fc53b2209b](https://github.com/gatieme/linux-history/commit/1fc53b2209b58e786c102e55ee682c12ffb4c794) |

内存提供了函数 isolate_lru_pages() 来将 LRU 链表中的页进行隔离, 为什么要隔离, 为了效率. 因为在做内存回收的路径了一定会各种遍历 LRU 链表, 但是这个时候系统并没有停止, 也会频繁的访问链表. 所以为了避免对锁的竞争, 内核决定将页面从 LRU 的链表上隔离出来去做内存回收的页面筛选动作. 参见 commit [ae102ac599f5 ("vmscan: move code to isolate LRU pages into separate function")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ae102ac599f557ea3544875957dda84ed5bb020e).


### 4.2.2 Split LRU 替换算法
-------


Rik van Riel, Lee Schermerhorn, Kosaki Motohiro 等众多的开发者设计了最初的 [Split LRU 替换算法](https://linux-mm.org/PageReplacementDesign). 它与 2.6.28 合入主线内核, 在经历了诸多的 bugfix 和优化后, 与 2.6.32 版本开始趋于稳定并运行良好.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/06/11 | Rik van Riel <riel@redhat.com> | [VM pageout scalability improvements (V12)](https://lore.kernel.org/patchwork/patch/118967) | 在大内存系统上, 扫描 LRU 中无法(或不应该)从内存中逐出的页面. 它不仅会占用CPU时间, 而且还会引发锁争用. 并且会使大型系统处于紧张的内存压力状态. 该补丁系列通过一系列措施提高了虚拟内存的可扩展性:<br>1. 将文件系统支持的、交换支持的和不可收回的页放到它们自己的LRUs上, 这样系统只扫描它可以/应该从内存中收回的页<br>2. 为匿名 LRUs 切换到双指针时钟替换, 因此系统开始交换时需要扫描的页面数量绑定到一个合理的数量<br>3. 将不可收回的页面完全远离LRU, 这样 VM 就不会浪费 CPU 时间扫描它们. ramfs、ramdisk、SHM_LOCKED共享内存段和mlock VMA页都保持在不可撤销列表中. | v12 ☑ [2.6.28-rc1](https://kernelnewbies.org/Linux_2_6_28#Various_core) | [PatchWork v2](https://lore.kernel.org/patchwork/patch/118967), [LWN](https://lwn.net/Articles/286472) |
| 2006/04/18 | "Rafael J. Wysocki" <rjw@sisk.pl> | [swsusp: rework memory shrinker (rev. 2)](https://lore.kernel.org/patchwork/patch/55788) | 按以下方式重新设计 swsusp 的内存收缩器: <br>1. 通过从中删除所有与 swsusp 相关的代码来简化 balance_pgdat(). <br>2. 使 shrink_all_memory() 使用 shrink_slab() 和一个新函数 shrink_all_zones(), 该函数以针对挂起优化的方式直接为每个区域调用 shrink_active_list() 和 shrink_inactive_list().<br>在 shrink_all_memory() 中, 我们尝试从更简单的目标开始释放与调用者要求的一样多的页面, 最好一次性释放. 如果slab缓存很大, 它们很可能有足够的页面来回收.  接下来是非活动列表(具有更多非活动页面的区域首先显示)等.<br>每次 shrink_all_memory() 尝试在5 次传递中缩小每个区域的活动和非活动列表. 在第一遍中, 仅考虑非活动列表. 在接下来的两次传递中, 活动列表也缩小了, 但映射的页面不会被回收. 在最后两遍中, 活动和非活动列表缩小, 映射页面也被回收. 这样做的目的是改变回收逻辑以选择最好的页面来继续恢复并提高恢复系统的响应能力. | [2.6.18](https://elixir.bootlin.com/linux/v2.6.18/source/mm/vmscan.c#L1057) | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/55598)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/55754)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/55788)<br>*-*-*-*-*-*-*-* <br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d6277db4ab271862ed599da08d78961c70f00002) |


### 4.2.3 二次机会法
-------

[Page Cache eviction and page reclaim](https://biriukov.dev/docs/page-cache/4-page-cache-eviction-and-page-reclaim)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/02/04 | Daniel Phillips | [generic use-once optimization instead of drop-behind check_used_once](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/diff/mm/filemap.c?id=6fbaac38b85e4bd3936b882392e3a9b45e8acb46) | TODO v2.4.7 -> v2.4.7.1 | v1 ☑✓ 2.5.0 | [HISTORY COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/diff/mm/filemap.c?id=6fbaac38b85e4bd3936b882392e3a9b45e8acb46) |
| 2002/02/04 | Linus Torvalds <torvalds@athlon.transmeta.com> | [me: be sane about page reference bits](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/diff/mm/filemap.c?id=c37fa164f793735b32aa3f53154ff1a7659e6442) | v2.4.9.9 -> v2.4.9.10 实现了 mark_page_accessed() | v1 ☑✓ 2.5.0 | [HISTORY COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/diff/mm/filemap.c?id=c37fa164f793735b32aa3f53154ff1a7659e6442) |
| 2002/10/31 | Andrew Morton <akpm@digeo.com> | [[PATCH] lru_add_active(): for starting pages on the active list](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=228c3d15a7020c3587a2c356657099c73f9eb76b) | 引入了 PER CPU 的 pagevec lru_add_pvecs 和 lru_add_active_pvecs. | v1 ☑✓ 2.5.46 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=228c3d15a7020c3587a2c356657099c73f9eb76b) |
| 2014/05/13 | Mel Gorman <mgorman@suse.de> | [Misc page alloc, shmem, mark_page_accessed and page_waitqueue optimisations v3r33](https://lore.kernel.org/patchwork/patch/464309) | 在页缓存分配期间非原子标记页访问方案, 为了解决 dd to tmpfs 的性能退化问题. 问题的主要原因是Kconfig的不同, 但是 Mel 找到了 tmpfs、mark_page_accessible 和  page_alloc 中不必要的开销. | v3 ☑ [3.16-rc1](https://kernelnewbies.org/Linux_3.16#Memory_management) | [PatchWork v3](https://lore.kernel.org/patchwork/patch/464309) |

### 4.2.4 pagevec 批处理
-------

为提高操作 LRU 链表的效率, 内核使用批量的操作方式进行一次性添加. 意思就是说先把 page 暂存在 pagevec 里面, 待存满的时候再一次性的刷到对应的 LRU 链表中.


#### 4.2.4.1 引入 pagevec 缓解 pagemap_lru_lock 锁竞争
-------

*   通过 pagevec 缓存来缓解 pagemap_lru_lock 锁竞争

2.5.32 合入了一组补丁优化全局 pagemap_lru_lock 锁的竞争, 做两件事:

1. 在几乎所有内核一次处理大量页面的地方, 将代码转换为一次执行16页的相同操作. 这把锁只开一次, 不要开十六次. 尽量缩短锁的使用时间.

2. 分页缓存回收函数的多线程: 在回收分页缓存页面时不要持有 pagemap_lru_lock. 这个功能非常昂贵.


这项工作的一个后果是, 我们在持有 pagemap_lru_lock 时从不使用任何其他锁. 因此, 这个锁从概念上从 VM 锁定层次结构中消失了.

所以. 这基本上都是为了提高内核可伸缩性而进行的代码调整. 它通过优化现有的设计来实现, 而不是重新设计. VM 的工作原理几乎没有概念上的改变.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2002/08/14 | Andrew Morton <akpm@zip.com.au> | [deferred and batched addition of pages to the LRU](hhttps://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=44260240ce0d1e19e84138ac775811574a9e1326) | TODO | v1 ☑✓ 2.5.32 | [HISTORY COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=44260240ce0d1e19e84138ac775811574a9e1326) |


```cpp
44260240ce0 [PATCH] deferred and batched addition of pages to the LRU
eed29d66442 [PATCH] pagemap_lru_lock wrapup
aaba9265318 [PATCH] make pagemap_lru_lock irq-safe
008f707cb94 [PATCH] batched removal of pages from the LRU
9eb76ee2a6f [PATCH] batched addition of pages to the LRU
823e0df87c0 [PATCH] batched movement of lru pages in writeback
3aa1dc77254 [PATCH] multithread page reclaim
6a952840483 [PATCH] pagevec infrastructure
```

| 编号 | 时间  | 作者 | 补丁 | 描述 |
|:---:|:----:|:----:|:---:|:---:|
| 1 | 2002/08/14 | Andrew Morton <akpm@zip.com.au> | [6a952840483 ("pagevec infrastructure")](https://github.com/gatieme/linux-history/commit/6a95284048359fb4e1c96e02c6be0be9bdc71d6c) | 引入了 struct pagevec, 这是批处理工作的基本单元 |
| 2 | 2002/08/14 | Andrew Morton <akpm@zip.com.au> | [3aa1dc77254 ("multithread page reclaim")](https://github.com/gatieme/linux-history/commit/3aa1dc772547672e6ff453117d169c47a5a7cbc5) | 借助 pagevec 完成页面回收 [shrink_cache()](https://elixir.bootlin.com/linux/v2.5.32/source/mm/vmscan.c#L278) 的并行化, 该操作需要持有 pagemap_lru_lock下运行. 借助 pagevec, 持锁后, 将 LRU 中的 32 个页面放入一个私有列表中, 然后解锁并尝试回收页面.<br>任何已成功回收的页面都将被批释放. 未回收的页面被重新添加到LRU.<br>这个补丁将 4-way 上的 pagemap_lru_lock 争用减少了 30 倍.<br>同时做的工作有: <br>对 shrink_cache() 代码进行了一些简化, 即使非活动列表比活动列表大得多, 它仍然会通过 [refill_inactive()](https://elixir.bootlin.com/linux/v2.5.32/source/mm/vmscan.c#L375) 渗透到活动列表中. |
| 3 | 2002/08/14 | Andrew Morton <akpm@zip.com.au> | [823e0df87c0 ("batched movement of lru pages in writeback")](https://github.com/gatieme/linux-history/commit/823e0df87c01883c05b3ee0f1c1d109a56d22cd3) | 借助 struct pagevec 完成 [mpage_writepages()](https://elixir.bootlin.com/linux/v2.5.32/source/fs/mpage.c#L526) 的批处理, 在 LRU 上每次移动 16 个页面, 而不是每次移动一个页面. |
| 4 | 2002/08/14 | Andrew Morton <akpm@zip.com.au> | [9eb76ee2a6f ("batched addition of pages to the LRU")](https://github.com/gatieme/linux-history/commit/9eb76ee2a6f64fe412bef315eccbb1dd63a203ae) | 通过对批量页面调用 lru_cache_add()的各个地方, 并对它们进行批量处理. 同时做了改进了系统在重回写负载下的行为. 页面分配失败减少了, 由于页面分配器在从 VM 回写时卡住而导致的交互性损失也有所减少. 原来 mpage_writepages() 无条件地将已写回的页面重新放到到非活动列表的头部. 现在对脏页做了优化. <br>1. 如果调用者(通常)是 balance_dirty_pages(), 则是脏页, 那么将页面留在它们在 LRU 上.<br>
如果调用者是 PF_MEMALLOC, 这些页面会 refiled 到 LRU 头. 因为只有 dirty_pages 才需要回写, 而且正在写的页面都位于 LRU 的尾部附近, 把它们留在那里, 页面分配器会过早地阻塞它们, 成为一个同步写操作. |
| 5 | 2002/08/14 | Andrew Morton <akpm@zip.com.au> | [008f707cb94 ("batched removal of pages from the LRU")](https://github.com/gatieme/linux-history/commit/008f707cb94696398bac6e5b5050b3bfd0ddf054) | 这个补丁在一定程度上改变了截断锁定. 从 LRU 中删除现在发生在页面从地址空间中删除并解锁之后. 所以现在有一个窗口, shrink_cache代码可以通过LRU发现要释放的页面列表. <br>1. 将所有对 lru_cache_del() 的调用转换为使用批处理 pagevec_lru_del().<br>2. 更改 truncate_complete_page() 不从 LRU 中删除页面, 改用page_cache_release() |
| 6 | 2002/08/14 | Andrew Morton <akpm@zip.com.au> | [aaba9265318 ("make pagemap_lru_lock irq-safe")](https://github.com/gatieme/linux-history/commit/aaba9265318483297267400fbfce1c399b3ac018) | 将 `spin_lock/unlock(&pagemap_lru_lock)` 替换为 `spin_lock/unlock_irq(&_pagemap_lru_lock)`. 对于 CPU 来说, 在保持页面 LRU 锁的同时进行中断是非常昂贵的, 因为在中断运行时, 其他 CPU 会在锁上堆积起来. 在持有锁时禁用中断将使 4-way 上的争用减少 30%. |
| 7 | 2002/08/14 | Andrew Morton <akpm@zip.com.au> | [eed29d66442 ("pagemap_lru_lock wrapup")](https://github.com/gatieme/linux-history/commit/eed29d66442c0e6babcea33ab03f02cdf49e62af) | 第 6 个补丁之后的一些简单的 cleanup. |
| 8 | 2002/08/14 | Andrew Morton <akpm@zip.com.au> | [44260240ce0 ("deferred and batched addition of pages to the LRU")](https://github.com/gatieme/linux-history/commit/44260240ce0d1e19e84138ac775811574a9e1326) | 1. 通过 pagevec 做缓冲和延迟, lru_cache_add 分批地将页面添加到LRU.<br>2. 在页面回收代码中, 在开始之前清除本地CPU的缓冲区. 减少页面长时间不驻留在LRU上的可能性.<br>(可以有 15 * num_cpus 页不在LRU上) |

这些补丁引入了页向量(pagevec) 数据结构来完成页面的批处理, 借助一个数组来缓存特定数量的页面, 可以在不需要持有大锁 pagemap_lru_lock 的情况下, 对这些页面进行批量操作, 比单独处理一个个页面的效率要高.

首先通过 [`pagevec_add()`](https://elixir.bootlin.com/linux/v2.5.32/source/include/linux/pagevec.h#L43) 将页面插入到页向量(pvec->pages) 中, 如果页向量满了, 则通过 [`__pagevec_lru_add()`](https://elixir.bootlin.com/linux/v2.5.32/source/mm/swap.c#L197) 将页面[添加到 LRU 链表](https://elixir.bootlin.com/linux/v2.5.32/source/mm/swap.c#L61)中.


#### 4.2.4.2 pagevec 的使用(LRU 缓存)
-------

最初没有 pagevec 的时候, lru_cache_add() 和 lru_cache_del() 直接向全局的 lru_cache 链表中添加页面, 每处理一个页面都要持有和释放一次 pagemap_lru_lock. 参见 [Import 2.3.16pre1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/diff/include/linux/swap.h?id=9aa2c66ac214f71cb051ba7c1adf313d9e160ee1).


*   首先是 lru_add_pvec @2.5.32

首次引入 pagevec 进行批量操作的时候, `lru_add_drain()-=>lru_cache_add()` 是 pagevec 的第一个用户, 在此之前 lru_cache_add() 每次 add_page_to_inactive_list() 将单个页面添加到 inactive_list 的时候, 都需要[持有和释放 `_pagemap_lru_lock`](https://elixir.bootlin.com/linux/v2.5.31/source/mm/swap.c#L55). 因此 [deferred and batched addition of pages to the LRU](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/commit/?id=44260240ce0d1e19e84138ac775811574a9e1326) 通过 pagevec 批处理进行优化. 新增了一个 `lru_add_pvecs[NR_CPUS]` 的 pagevec 数组, 用于缓存 per CPU 的 LRU page. `lru_cache_add()` 在处理的时候, 不再直接将页面加入 inactive_list. 而是先通过 pagevec_add() 将页面缓存到当前 CPU 的 lru_add_pvecs[get_cpu()] 中. 直到加入 PAGEVEC_SIZE 个页面时, 才通过 [`__pagevec_lru_add()`](https://elixir.bootlin.com/linux/v2.5.32/source/mm/swap.c#L197) 将这些页面一起加入到 inactive_list. 这样就不需要每个页面持有一次 `_pagemap_lru_lock`, 而是将 PAGEVEC_SIZE 个页面一把持有和释放一次 `_pagemap_lru_lock`.

lru_add_pvec 用于缓冲向 LRU 列表(主要是 active 和 inactive list) 中添加页面的请求, 通过批处理操作, 减少对 `_pagemap_lru_lock` 的冲突与竞争.

|  时间  | 作者 |  特性 | 描述  |  是否合入主线  | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2002/08/14 | Andrew Morton <akpm@zip.com.au> | [deferred and batched addition of pages to the LRU](hhttps://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=44260240ce0d1e19e84138ac775811574a9e1326) | 引入 pagevec, lru_cache_add() 将页面添加到 inactive_list 时, 批量将一组 PAGEVEC_SIZE 个页面先缓存到 lru_add_pvecs[get_cpu()] 中, 再一次性添加到 inactive_list 中. | v1 ☑✓ 2.5.32 | [HISTORY COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=44260240ce0d1e19e84138ac775811574a9e1326) |
| 2002/10/31 | Andrew Morton <akpm@digeo.com> | [start anon pages on the active list (properly this time)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f9a316fa9099053a299851762aedbf12881cff42) | 为了优化大压力 swap 场景下性能劣化的问题, 引入 [lru_cache_add_active()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=228c3d15a7020c3587a2c356657099c73f9eb76b), 确保[已映射或即将映射到页面的从 LRU active list 中开始](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=33709b5c8022486197ed2345eed18bbeb14a2251).<br>新增了 per-CPU 的 lru_add_active_pvecs pagevec 数组, 同时将它和 lru_add_pvecs 都定位为 per-CPU 变量. | v1 ☑✓ 2.5.46 | [HISTORY COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=33709b5c8022486197ed2345eed18bbeb14a22512) |

这个阶段处理还比较简单, 通过 [lru_cache_add()](https://elixir.bootlin.com/linux/v2.5.46/source/mm/swap.c#L71) 和 [lru_cache_add_active()](https://elixir.bootlin.com/linux/v2.5.46/source/mm/swap.c#L81) 分别将页面加入到 inactive_list 和 active_list 中.

然后 [swap: use an array for the LRU pagevecs](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f04e9ebbe4909f9a41efd55149bc353299f4e83b) 移除了 lru_cache_add() 和 lru_cache_add_active, 使用了统一的 lru_add_pvecs[NR_LRU_LISTS] 数组, 然后封装了 [lru_cache_add_lru 往对应 LRU 的 pagevec 中添加页面](https://elixir.bootlin.com/linux/v2.6.28/source/mm/swap.c#L214). 同时提供了 [lru_cache_add_active_or_unevictable()](https://elixir.bootlin.com/linux/v2.6.28/source/mm/swap.c#L259) 将页面添加到 active 或者 unevictable 列表(其中 unevictable 还未使用 pagevec). 此外 [commit 4f98a2fee8ac ("vmscan: split LRU lists into anon & file sets")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4f98a2fee8acdb4ac84545df98cccecfd130f8db) 拆分了匿名页和文件页 LRU 之后, 拆解出来的 lru_cache_add_anon(),  lru_cache_add_active_anon(),  lru_cache_add_file(),  lru_cache_add_active_file() 也是一些潜在的用户, 他们直接调用了更底层的 `__lru_cache_add()` 直接向指定类型的 LRU 中批量添加页面.

[mm: pagevec: defer deciding which LRU to add a page to until pagevec drain time](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=13f7f78981e49f288d871bb918545ef5c952e00b) 删除了 per-CPU 上 per LRU 的 PageVec 数组 lru_add_pvecs[NR_LRU_LISTS], 只留下一个 [per-CPU 的 pagevec](https://elixir.bootlin.com/linux/v3.11/source/mm/swap.c#L43).

|  时间  | 作者 |  特性 | 描述  |  是否合入主线  | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2008/10/18 | KOSAKI Motohiro <kosaki.motohiro@jp.fujitsu.com> | [swap: use an array for the LRU pagevecs](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f04e9ebbe4909f9a41efd55149bc353299f4e83b) | 将 lru_add_pvecs 和 lru_add_active_pvecs 两个 PageVec 变成一个 per LRU 的 PageVec 数组 lru_add_pvecs[NR_LRU_LISTS], 就像 LRU 一样. 在 split VM 补丁系列中进一步创建了所有 LRU 列表之后, 这显著地清理了源代码, 并将内核大小减少了约 13kB. | v1 ☑✓ 2.6.28-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f04e9ebbe4909f9a41efd55149bc353299f4e83b) |
| 2013/05/13 | Mel Gorman <mgorman@suse.de> | [Obey mark_page_accessed hint given by filesystems](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=c53954a092d07c5684d31ea1fc813d262cff08a5) | Alexey Lyahkov 和 Robin Dong 等最近(v3.10期间)报告了诸多问题, 这些问题可能是由于热页太快到达非活动列表的末尾并被回收造成的. 这个系列的目的不是在每个文件系统的基础上解决这个问题, 而是通过[推迟页面添加到 pagevec 的 LRU 时间](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=13f7f78981e49f288d871bb918545ef5c952e00b), 并[允许 mark_page_accessed() 在 pagevec 页面上调用 SetPageActive()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=059285a25f30c13ed4f5d91cecd6094b9b20bb7b).<br>当前 mark_page_access() 不能激活位于非活动 LRU 页面的非活动页面.<br>为了解决这个问题<br>a. 这个补丁删除了 per-CPU 上 per LRU 的 PageVec 数组 lru_add_pvecs[NR_LRU_LISTS], 只留下一个 per-CPU 的 pagevec. 页面将在 pagevec 满后批量被添加到的 LRU.<br>b. 当 LRU 在 LRU 消耗时间被选中, 如果它们在本地的 pagevec 上, 且被标记为 PageActive, 这样它在 LRU 消耗时间就无法被移动到正确的列表. mark_page_accessed() 过程中如果发现页面不在 LRU 上, 则使用 `__lru_cache_activate_page()` 处理全局 lru_add_pvec 上的页面. 这样修复后, 使用 git checkout 这样的工作负载进行测试, 在实践中页面从来没有添加到活动文件列表中, 但应用这个补丁后, 它们被添加到活动文件列表中.<br>由于只保留了一个 per-CPU 的 pagevec, 这意味着可用的 pagevecs 更少, 并且 LRU 锁上的争用可能更大. 然而, 这只适用于在 LRU 中添加了几乎完美的文件、匿名、活动和非活动页面的情况. 在实践中, 增加的是特定时间的页面流, 而社区所争论问题中的变化几乎无法衡量. <br>1. [补丁 1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c6286c983900c77410a951874f1589f4a41fbbae) 为 LRU 页面激活和插入添加了两个跟踪点. 以便在 LRU 中构建一个离线的页面模型.<br>2. [补丁 2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=13f7f78981e49f288d871bb918545ef5c952e00b) 推迟决定向哪个 LRU 添加页面, 直到 pagevec 耗尽.<br>3. [补丁 3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=059285a25f30c13ed4f5d91cecd6094b9b20bb7b) 在本地 pagevec 中搜索要在 mark_page_accessed() 上标记 PageActive 的页面.<br>4. 补丁 4/5 清理了 API. 延迟判断要添加的 lru 列表后, lru_cache_add() 函数中不再需要显式指定 `enum lru_list lru` 参数. | v2 ☑✓ 3.11-rc1 | [LORE v2,0/4](https://lore.kernel.org/all/1368440482-27909-1-git-send-email-mgorman@suse.de), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=13f7f78981e49f288d871bb918545ef5c952e00b) |



*   其次是 lru_rotate_pvecs @v2.6.24


|  时间  | 作者 |  特性 | 描述  |  是否合入主线  | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2007/10/16 | Hisashi Hifumi <hifumi.hisashi@oss.ntt.co.jp> | [mm: use pagevec to rotate reclaimable page](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=902aaed0d983dfd459fcb2b678608d4584782200) | [Move reclaimable pages to the tail ofthe inactive list on](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3b0db538ef6782a1e2a549c68f1605ca8d35dd7e) 使用 rotate_reclaimable_page() 将 IO 路径的脏页移动到非活动列表的尾部, 以加速这类页面的回收, 但是当时没有使用 pagevec 进行批处理. 因此后面测试遇到了一些性能问题于此有关.<br>当运行一些内存密集型负载时, 系统响应在换出启动后就恶化了. 这个问题的原因是当一个 PG_reclaim 页面在 rotate_reclaimable_page() 中被移动到不活动的 LRU 列表的尾部时, 每次回写页面都会获得 lru_lock 旋转锁. 这会导致系统性能下降, 并且在切换启动时中断保持时间变长.<br>这个补丁解决此问题. 在旋转可回收页面时使用 pagevec 来减轻 LRU 旋转锁争用和减少中断等待时间.<br>新增了 per-CPU 的 lru_rotate_pvecs pagevec, 通过 pagevec_move_tail 将缓存在 lru_rotate_pvecs 的页面批量插入到 inactive_list 中. | v1 ☑✓ 2.6.24-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=902aaed0d983dfd459fcb2b678608d4584782200) |


*   紧接着是 lru_deactivate_file_pvecs @v2.6.39

invalidate_mapping_pages() 用于清理和释放内核中映射的文件页面. 比如我们可以通过 fadvise 系统调用通过 POSIX_FADV_DONTNEED 清除文件所属的缓存 Page Cache, 从而释放这些页面. 这种情况下最终就是通过 invalidate_mapping_pages() 调用 deactivate_file_page() 强制将页面移入 inactive_list 来加速完成 page 的释放的.


|  时间  | 作者 |  特性 | 描述  |  是否合入主线  | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2011/03/22 | Minchan Kim <minchan.kim@gmail.com> | [mm: deactivate invalidated pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=315601809d124d046abd6c3ffa346d0dbd7aa29d) | 社区上报了几起[性能问题](http://marc.info/?l=rsync&m=128885034930933&w=2), 它执行了一些备份工作负载(例如, 夜间执行 rsync 等). 往往这些工作负载只使用一次页面, 而触摸两次页面. 它将页面提升到活动列表, 从而导致工作集页面被从 LRU 中剔除, 导致了业务的性能颠簸. 引入 deactivate_page()(后来改名叫 deactivate_file_page()), 将这类备份工作负载等特殊路径下识别处理的页面移动到非活动列表以加快其回收速度. 它被移到列表的顶部, 而不是尾部, 以便给刷新线程一些时间将其写出来, 因为这比从回收中写单页要有效得多.<br>为了进行批处理, 这里引入了 per-CPU 的 pagevec lru_deactivate_pvecs.<br>随后因为它们处理的其实都是文件页, 因此 [commit 315601809d12 ("mm: rename deactivate_page to deactivate_file_page")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=cc5993bd7b8cff4a3e37042ee1358d1d5eafa70c) 将这一系列接口都改名加上 file 的前缀. | v1 ☑✓ 2.6.39-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=315601809d124d046abd6c3ffa346d0dbd7aa29d) |

*   其次是 activate_page_pvecs 与 pagevec_lru_move_fn @v3.0


v2.6.38 时 [mm: simplify code of swap.c](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d8505dee1a87b8d41b9c4ee1325cd72258226fbc) 引入了 pagevec_lru_move_fn() 精简了 pagevec 的处理, 减少了冗余代码. 此时内核中已经有两个全局 per-CPU pagevec: lru_add_pvec 和 lru_rotate_pvecs 都切到了 pagevec_lru_move_fn(), 对应的 fn 分别是 `____pagevec_lru_add_fn()` 和 `pagevec_move_tail_fn()`.

在二次机会法的关键路径 mark_page_accessed()-=>activate_page() 激活一个页面时, 需要频繁地争抢 zone->lru_lock, 也可以通过 pagevec 批量执行 activate_page() 来减少锁争用.

在一个 4 socket 64 CPU 系统中, 创建一个稀疏文件和 64 个进程, 共享的进程映射到该文件. 每个进程读取访问整个文件, 然后退出. 进程退出将执行 unmap_vmas() 并导致大量 activate_page() 调用. 在这样的工作负载下, 通过 pagevec 批处理后, 发现使用 below patch 的总时间减少了 58%.

|  时间  | 作者 | 特性  | 描述  | 是否合入主线   | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2011/01/13 | Shaohua Li <shaohua.li@intel.com> | [mm: batch activate_page() to reduce lock contention](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=744ed1442757767ffede5008bb13e0805085902e) | 第一个补丁引入了 pagevec_lru_move_fn(), 第二个补丁引入了 PAGEVEC activate_page_pvecs 来批处理 active_page() 的流程. | v1 ☑✓ 2.6.38-rc1 | [LORE v1,0/2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=744ed1442757767ffede5008bb13e0805085902e) |
| 2011/01/17 | Linus Torvalds <torvalds@linux-foundation.org> | [Revert "mm: simplify code of swap.c"](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=83896fb5e515) | Chris Mason 上报了一些页面分配错误和在 IO 调度器上的页面等待, 发现是上述补丁引起的, Linus 直接进行了回退. | v1 ☑✓ 2.6.38-rc1 | [LORE v1,0/2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=83896fb5e515) |

由于第一个补丁只是重构, 未影响功能, 随后 2.6.39-rc1 期间, 又将第一个补丁重新合入. [mm: simplify code of swap.c](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3dd7ae8ec0ef399bfea347f297d2a95504d35571)

第二个对 active_page() 进行 PAGEVEC 批处理的补丁, 直接 3.0-rc1 才合入, [mm: batch activate_page() to reduce lock contention](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=eb709b0d062efd653a61183af8e27b2711c3cf5c)


*   随后是 lru_lazyfree_pvecs @v4.5

配置了 madvise MADV_FREE 的页面, 将通过 mark_page_lazyfree() 标记为 lazyfree 的, 这些页面会使用 lru_lazyfree_pvecs 作为批处理的缓冲区.  不管它是匿名页还是文件页, 都将通过 lru_lazyfree_pvecs 添加到 inactive file LRU list 的头部位置, 从而加速它的回收. 参见 [lru_lazyfree_fn()](https://elixir.bootlin.com/linux/v4.12/source/mm/swap.c#L591)

|  时间  | 作者 |  特性 | 描述  |  是否合入主线  | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2016/01/15 | Minchan Kim <minchan@kernel.org> | [mm: move lazily freed pages to inactive list](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=b8d3c4c3009d42869dc03a1da0efc2aa687d0ab4) | [MADV_FREE support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=05ee26d9e7e29ab026995eab79be3c6e8351908c) 的其中一个补丁. 引入了 lru_deactivate_pvecs( 随后改名 [lru_lazyfree_pvecs](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f7ad2a6cb9f7c4040004bedee84a70a9b985583e)) pagevec 将标记了 lazyfree 的页面, 添加到 inactive list 上. 同时又引入了 deactivate_page() 后来被改名为 mark_page_lazyfree(). | v1 ☑✓ 4.5 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=10853a039208c4afaa322a7d802456c8dca222f4), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f7ad2a6cb9f7c4040004bedee84a70a9b985583e) |


*   最后被引入的 lru_deactivate_pvecs

与 lru_lazyfree_pvecs 用于将标记 MADV_FREE 的页面移动到 inactive file LRU list 的头部类似. lru_deactivate_pvecs()

|  时间  | 作者 |  特性 | 描述  |  是否合入主线  | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2019/07/14 | Minchan Kim <minchan@kernel.org> | [mm: introduce MADV_COLD](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9c276cc65a58faf98be8e56962745ec99ab87636) | 实现 madvise MADV_COLD, 引入了 lru_deactivate_pvecs 和 deactivate_page() | v5 ☑ 5.4-rc1 | [PatchWork v7,0/5](https://patchwork.kernel.org/project/linux-mm/cover/20190714233401.36909-1-minchan@kernel.org), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9c276cc65a58faf98be8e56962745ec99ab87636) |


*   引入 local_locks 的概念

它严格针对每个 CPU, 满足 PREEMPT_RT 所需的约束.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/05/27 | Sebastian Andrzej Siewior <bigeasy@linutronix.de> | [Introduce local_lock()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=19f545b6e07f753c4dc639c2f0ab52345733b6a8) | 引入了 local_locks 保护 LRU pagevec, 实现了 `local_lock_t lock` 保护的 per-CPU 的 lru_pvecs 和 lru_rotate 结构, [替代了传统的 per-CPU 的 pagevec 的方式](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b01b2141999936ac3e4746b7f76c0f204ae4b445). 其中 lru_pvecs 结构统一管理和保护 lru_add, lru_rotate_pvecs, lru_deactivate_file, lru_deactivate, lru_lazyfree, 等 pagevec. lru_rotate 单独管理和保护了 lru_rotate_pvecs. | v3 ☑ [5.8-rc1](https://kernelnewbies.org/Linux_5.8#Memory_management) | [LORE v2](https://lore.kernel.org/lkml/20200524215739.551568-1-bigeasy@linutronix.de)<br>*-*-*-*-*-*-*-* <br>[LORE v3](https://lore.kernel.org/all/20200527201119.1692513-1-bigeasy@linutronix.de) |

*   总结

| PAGEVEC | CallChain(user...pagevec_lru_move_fn) | 详细描述(使用场景参考 user, 具体操作参考 FN) |
|:-------:|:-------------------------------------:|:---------------------------------------:|
| lru_pvecs.lru_add             | lru_cache_add_(in)active_or_unevictable()/add_page_cache_to_lru()<br>-=>lru_cache_add()<br>&emsp;-=> __pagevec_lru_add()<br>&emsp;&emsp;-=> __pagevec_lru_add_fn | 最常规的 LRU 操作, 将页面添加到对应的 LRU 中, 将一个新分配的页面添加到 LRU, 以及把 PageCache 加入到 LRU 都是走的这个路径. |
| lru_pvecs.lru_deactivate_file | invalidate_mapping_pages()<br>-=> deactivate_file_page()<br>&emsp;-=> lru_deactivate_file_fn() | NA |
| lru_pvecs.lru_deactivate      | madvise_cold_or_pageout_pte_range()<br>-=> deactivate_page()<br>&emsp;-=> lru_deactivate_fn() | 将页面从 active list 移动到对应的 inactive list |
| lru_pvecs.lru_lazyfree        | madvise_free_pte_range()/madvise_free_huge_pmd()<br>-=> mark_page_lazyfree()<br>&emsp;-=> lru_lazyfree_fn() | 将(匿名的) active 页移动到 inactive file LRU list. |
| lru_pvecs.activate_page       | mark_page_accessed()<br>-=> activate_page()<br>&emsp;-=> __activate_page() | 将页面从 inactive list 移动到对应的 active list |
| lru_rotate.pvec               | end_page_writeback()<br>-=> rotate_reclaimable_page()<br>&emsp;-=> pagevec_move_tail_fn() | NA |


*   pagevec 的动态使用

pagevec 还提供了一些 API, 供内核和驱动中动态的创建和使用 pagevec.

|        接口        |      描述      |
|:-----------------:|:--------------:|
| pagevec_init()    | 初始化 pagevec. |
| pagevec_reinit()  | 重置 pagevec. |
| pagevec_count()   | 当前占用的 pagevec 的页面数量. |
| pagevec_space()   | pagevec 可容纳剩余空间. |
| pagevec_add()     | 将 page 添加到 pagevec. |
| pagevec_release() | 将 page 的_refcount 减 1, 如果为 0, 则释放该页到伙伴系统. |

#### 4.2.4.3 添加页面到 LRU 的接口变迁
-------

*   lru_cache_add

[vmscan: split LRU lists into anon & file sets](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4f98a2fee8acdb4ac84545df98cccecfd130f8db) 分离匿名页和文件页后, add 和 del 的接口需要显式 anon 或者 file.

*   lru_cache_add_inactive_or_unevictable

在引入 Unevictable LRU 的时候, 最基础的一个场景, [commit 64d6519dda39 ("swap: cull unevictable pages in fault path")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=64d6519dda3905dfb94d3f93c07c5f263f41813f) 在 [Page Fault 分配了新的匿名页面](https://elixir.bootlin.com/linux/v2.6.28/source/mm/memory.c#L1918)后, 如果该页面是可以被驱逐的 [page_evictable()](https://elixir.bootlin.com/linux/v2.6.28/source/mm/swap.c#L262), 则通过 lru_cache_add_lru 将其添加到活动的 lru 列表中(借助了 pagevec), 否则将其添加到 [Unevictable LRU](https://www.kernel.org/doc/html/latest/vm/unevictable-lru.html). 因此将这个流程封装到 [lru_cache_add_active_or_unevictable()](https://elixir.bootlin.com/linux/v2.6.28/source/mm/swap.c#L259) 函数中.

由于 lru_cache_add_active_or_unevictable 总是与 page_add_new_anon_rmap() 成对出现, 因此 [commit b5934c531849 ("mm: add_active_or_unevictable into rmap")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b5934c531849ff4a51ce0f290141efe564290e40), 将此流程放到了 page_add_new_anon_rmap() 中, 并移除了 lru_cache_add_active_or_unevictable() 函数.

[mm: memcontrol: rewrite charge API](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=00501b531c4723972aa11d6d4ebcf8d6552007c8) 又只能再次将 lru_cache_add_active_or_unevictable() 的流程从 page_add_new_anon_rmap() 中剥离出来.

[commit b518154e59aa ("mm/vmscan: protect the workingset on anonymous LRU")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b518154e59aab3ad0780a169c5cc84bd4ee4357e) 将新创建或交换的匿名页面放到 inactive LRU list, 这样 lru_cache_add_active_or_unevictable() 也直接被 lru_cache_add_inactive_or_unevictable() 替代.

### 4.2.5 zone or node base LRU
-------

LRU 的组织形式经历了多次变迁, 从最开始全局的 LRU, 演变成 Per-Zone LRU, 后来又支持了了 per-memcg LRU, 直到 v4.8 切换到了 Per-Node LRU.

LRU 组织形式的变更和 LRU lock 的变更是无法割裂开的. 每次 LRU 整体 base 的变迁, 必然伴随着 lru_lock 的变迁.

*  Global LRU/Reclaim

最开始的 LRU 是全局的, 那么 lru_lock 也是全局的 `pagemap_lru_lock`. 随后 [make pagemap_lru_lock irq-safe](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=aaba9265318483297267400fbfce1c399b3ac018) 将 `pagemap_lru_lock` 修改为中断安全的, 其实就是之前全是 spin_lock/unlock 的, 现在改为 spin_unlock_irq/spin_lock_irq. 并将锁名字替换为 `_pagemap_lru_lock`.

*   Zone-Based LRU/Reclaim

2.5.33 合入了基于 ZONE 的 LRU, 那么自然全局的 lru_lock `_pagemap_lru_lock` 也被修改为细粒度的 zone->lru_lock. 从而减少冲突.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2002/08/27 | Andrew Morton <akpm@zip.com.au> | [per-zone LRU](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a8382cf1153689a1caac0e707e951e7869bb92e1) | [per zone LRU](https://github.com/gatieme/linux-history/commit/e6f0e61d9ed94134f57bcf6c72b81848b9d3c2fe) 替换原来的全局 LRU 链表.<br>[per zone 的 lru_lock](https://github.com/gatieme/linux-history/commit/a8382cf1153689a1caac0e707e951e7869bb92e1) 替换原来的全局 `_pagemap_lru_lock` | v1 ☐☑✓ 2.5.33 | [LORE v1,0/3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a8382cf1153689a1caac0e707e951e7869bb92e1) |

*   Node-Based LRU/Reclaim

采用 zone LRU 链表方案主要是由于历史原因: 在 32 位系统内存在 ZONE_HIGH, 内核申请内存时尽量从 ZONE_HIGH 中申请, 这对 ZONE_HIGH 回收优先级要高于 ZONE_NORMA. 而随着时代的发展, 64 位系统中已经没必要使用 ZONE_HIGH, 故采用 node 节点 LRU 效率更高, 也更加让人容易理解. 参见 [Move LRU page reclaim from zones to nodes v9](https://lore.kernel.org/all/1467970510-21195-1-git-send-email-mgorman@techsingularity.net).

> The reason we have zone-based reclaim is that we used to have large highmem zones in common configurations and it was necessary to quickly find ZONE_NORMAL pages for reclaim. Today, this is much less of a concern as machines with lots of memory will (or should) use 64-bit kernels. Combinations of 32-bit hardware and 64-bit hardware are rare. Machines that do use highmem should have relatively low highmem:lowmem ratios than we worried about in the past.
>
> Conceptually, moving to node LRUs should be easier to understand. The page allocator plays fewer tricks to game reclaim and reclaim behaves similarly on all nodes.

4.8 合入了基于 NODE 的页面回收策略, 将 LRU 的页面回收从 ZONE 迁移到了 NODE 上. 同时将 zone->lru_lock 移除, 替换成了 NODE 的 lru_lock. 并提供了封装好的接口 zone_lru_lock() 来访问.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:
| 2016/07/08 | Mel Gorman <mgorman@techsingularity.net> | [Move LRU page reclaim from zones to nodes v9](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d7f05528eedb047efe2288cff777676b028747b6) | [将 LRU 页面的回收从 ZONE 切换到 NODE](https://lore.kernel.org/all/1467970510-21195-1-git-send-email-mgorman@techsingularity.net). 后续又直接合入了诸多 bugfix, 有些不定直接合并到了之前的提交上. [Follow-up fixes to node-lru series v1, 0/4](https://lore.kernel.org/all/1468404004-5085-1-git-send-email-mgorman@techsingularity.net)<br>[Follow-up fixes to node-lru series v2,0/5](https://lore.kernel.org/all/1468588165-12461-1-git-send-email-mgorman@techsingularity.net)<br>[Follow-up fixes to node-lru series v3,0/3](https://lore.kernel.org/all/1468853426-12858-1-git-send-email-mgorman@techsingularity.net)<br>[Candidate fixes for premature OOM kills with node-lru v2, 0/5](https://lore.kernel.org/all/1469110261-7365-1-git-send-email-mgorman@techsingularity.net/) | v9 ☑ [4.8-rc1](https://kernelnewbies.org/Linux_4.8#Memory_management) | [LORE v9,00/34](https://lore.kernel.org/all/1467970510-21195-1-git-send-email-mgorman@techsingularity.net) |
| 2009/12/11 | Rik van Riel <riel@redhat.com> | [vmscan: limit concurrent reclaimers in shrink_zone](https://lore.kernel.org/patchwork/patch/181645) | 限制 shrink_zone() 中的并发数目.<br>在非常重的多进程工作负载下(如AIM7), VM可能会以各种方式陷入麻烦. 当页面回收代码中有数百甚至数千个活动进程时, 问题就开始了.<br>1. 在页面回收代码中, 由于成千上万个进程之间的锁争用(和有条件的重调度), 系统不仅会遭受巨大的减速<br>2. 每个进程都将尝试释放最多 SWAP_CLUSTER_MAX 页面, 即使系统已经有大量的空闲内存.<br>通过限制页面回收代码中同时活动的进程数量, 应该可以同时避免这两个问题.如果在一个区域中有太多活动的进程在执行页面回收, 只需在shrink_zone()中进入休眠状态. 在唤醒时, 在我们自己进入页面回收代码之前, 检查是否已经释放了足够的内存. 在这里使用与页面分配器中使用相同的阈值, 以决定是否首先调用页面回收代码, 否则一些不幸的进程可能最终会为系统的其他部分释放内存. | v2 ☐ | [PatchWork v2](https://lore.kernel.org/patchwork/patch/181645) |

*   MEMCG-Aware LRU/Reclaim

2.6.25 期间, 在 Per-Zone LRU 的基础上, 实现了 per-memcg LRU 的支持, 但是 per-memcg LRU lock 的支持却经历了不少的.

[memcg lru lock 血泪史](https://blog.csdn.net/bjchenxu/article/details/112504932)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/12/27 | Kamezawa Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com> | [per-zone and reclaim enhancements for memory controller take 3](https://lore.kernel.org/patchwork/patch/98042) | per-zone LRU for memcg, 其中引入了 mem_cgroup_per_zone, mem_cgroup_per_node 等结构 | v7 ☑ 2.6.25-rc1 | [PatchWork v7](https://lore.kernel.org/lkml/20071127115525.e9779108.kamezawa.hiroyu@jp.fujitsu.com) |
| 2011/12/08 | Johannes Weiner <jweiner@redhat.com> | [memcg naturalization -rc5](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=6b208e3f6e35aa76d254c395bdcd984b17c6b626) | 引入 per-memcg lru, 消除重复的 LRU 列表, [全局 LRU 不再存在, page 只存在于 per-memcg LRU list 中](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=925b7673cce39116ce61e7a06683a4a0dad1e72a).<br>它使传统的页面回收能够从每个memcg LRU列表中查找页面, 从而消除了双LRU模式(除了每个memcg区域外, 每个全局区域)和系统中每个页面所需的额外列表头. <br>该补丁引入了 lruvec 结构. | v5 ☑ [3.3-rc1](https://kernelnewbies.org/Linux_3.3#Memory_management) | [PatchWork v5](https://lore.kernel.org/patchwork/patch/273527), [LWN](https://lwn.net/Articles/443241) |
| 2012/02/20 | Hugh Dickins <hughd@google.com> | [mm/memcg: per-memcg per-zone lru locking](https://lore.kernel.org/patchwork/patch/288055) | per-memcg lru lock | v1 ☐ | [PatchWork v1](https://lore.kernel.org/patchwork/patch/288055) |
| 2020/12/05 | Alex Shi <alex.shi@linux.alibaba.com> | [per memcg lru lock](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=15b447361794271f4d03c04d82276a841fe06328) | per memcg LRU lock | v21 ☑ [5.11](https://kernelnewbies.org/Linux_5.11#Memory_management) | [LORE v21,00/19](https://lore.kernel.org/all/1604566549-62481-1-git-send-email-alex.shi@linux.alibaba.com) |


### 4.2.6 不同类型页面拆分管理
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/06/11 | Rik van Riel <riel@redhat.com> | [VM pageout scalability improvements (V12)](https://lore.kernel.org/patchwork/patch/118967) | 一系列完整的重构和优化补丁, 在大内存系统上, 扫描 LRU 中无法(或不应该)从内存中逐出的页面. 它不仅会占用CPU时间, 而且还会引发锁争用. 并且会使大型系统处于紧张的内存压力状态. 该补丁系列通过一系列措施提高了虚拟机的可扩展性:<br>1. 将文件系统支持的、交换支持的和不可收回的页放到它们自己的LRUs上, 这样系统只扫描它可以/应该从内存中收回的页<br>
2. 为匿名 LRUs 切换到双指针时钟替换, 因此系统开始交换时需要扫描的页面数量绑定到一个合理的数量<br>3. 将不可收回的页面完全远离LRU, 这样 VM 就不会浪费 CPU 时间扫描它们. ramfs、ramdisk、SHM_LOCKED共享内存段和mlock VMA页都保持在不可撤销列表中. | v12 ☑ [2.6.28-rc1](https://kernelnewbies.org/Linux_2_6_28#Various_core) | [PatchWork v2](https://lore.kernel.org/patchwork/patch/118966), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4f98a2fee8acdb4ac84545df98cccecfd130f8db) |

*   匿名页和文件页拆分

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/03/16 | Rik van Riel <riel@redhat.com> | [split file and anonymous page queues](https://lore.kernel.org/patchwork/patch/76770) | 将 LRU 中匿名页和文件页拆分的第一次尝试 | RFC v3 ☐ | [PatchWork RFC v1](https://lore.kernel.org/patchwork/patch/76719)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC v2](https://lore.kernel.org/patchwork/patch/76558)<br>*-*-*-*-*-*-*-*<br>[PatchWork RFC v3](https://lore.kernel.org/patchwork/patch/76558) |
| 2007/11/03 | Rik van Riel <riel@redhat.com> | [split anon and file LRUs](https://lore.kernel.org/patchwork/patch/96138) | 将 LRU 中匿名页和文件页分开管理的系列补丁 | RFC ☐ | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/96138) |
| 2008/06/11 | Rik van Riel <riel@redhat.com> | [VM pageout scalability improvements (V12)](https://lore.kernel.org/patchwork/patch/118966) | 这里我们关心的是它将 LRU 中匿名页和文件页分开成两个链表进行管理. 将LRU列表分成两组, 一组用于文件映射的页面, 另一组用于匿名页面, 后者包括tmpfs. 这样做的好处是, VM 将不必扫描大量的匿名页面(我们通常不希望换出这些匿名页面), 只需要找到它应该清除的页面缓存页面(PageCache). | v12 ☑ [2.6.28-rc1](https://kernelnewbies.org/Linux_2_6_28#Various_core) | [PatchWork v2](https://lore.kernel.org/patchwork/patch/118966), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4f98a2fee8acdb4ac84545df98cccecfd130f8db) |

*   拆分出 unevictable page 的 LRU 链表

**2.6.28(2008年12月)**

[Unevictable快取的奇怪行為(Linux內核)](https://t.codebug.vip/questions-1595575.htm)

虽然现在拆分出 4 个链表了, 但还有一个问题, 有些页被**"钉"**在内存里(比如实时算法, 或出于安全考虑, 不想含有敏感信息的内存页被交换出去等原因, 用户通过 **_mlock()_**等系统调用把内存页锁住在内存里). 当这些页很多时, 扫描这些页同样是徒劳的. 内核将这些页面成为 [unevictable page](https://stackoverflow.com/questions/30891570/what-is-specific-to-an-unevictable-page). [Documentation/vm/unevictable-lru.txt](https://www.kernel.org/doc/Documentation/vm/unevictable-lru.txt)

有很多页面都被认为是 unevictable 的, 参见 [What is specific to an unevictable page?](https://stackoverflow.com/questions/30891570/what-is-specific-to-an-unevictable-page), 主要原因如下:

比如 ramfs 或者 ram disk 的页面是虚拟硬盘的一部分, 将这些页面进行回收会破坏虚拟硬盘, 因此当 vmscan 发现它们是脏的并试图清理它们时, 而 ram 磁盘回写函数只是重脏页面, 从而使它们再回到活动列表, [commit ba9ddf493916 ("Ramfs and Ram Disk pages are unevictable")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ba9ddf49391645e6bb93219131a40446538a5e76)

被其他机制"手动"锁定到当前位置的页面也是不可回收的, [commit 89e004ea55ab ("SHM_LOCKED pages are unevictable")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=89e004ea55abe201b29e2d6e35124101f1288ef7), [commit b291f000393f ("mlock: mlocked pages are unevictable")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b291f000393f5a0b679012b39d79fbc85c018233)

所以解决办法是把这些页独立出来, 放一个独立链表, [commit 894bc310419a ("Unevictable LRU Infrastructure")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=894bc310419ac95f4fa4142dc364401a7e607f65). 现在就有5个链表了, 不过有一个链表不会被扫描. 详情参见 [The state of the pageout scalability patches](https://lwn.net/Articles/286472).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/06/11 | Rik van Riel <riel@redhat.com> | [VM pageout scalability improvements (V12)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9978ad583e100945b74e4f33e73317983ea32df9) | 新增了 CONFIG_UNEVICTABLE_LRU, 将 unevictable page 用一个单独的 LRU 链表管理, 参见 [The state of the pageout scalability patches](https://lwn.net/Articles/286472). | v12 ☑ [2.6.28-rc1](https://kernelnewbies.org/Linux_2_6_28#Various_core) | [PatchWork v2](https://lore.kernel.org/patchwork/patch/118967), [LWN](https://lwn.net/Articles/286472)[LORE v12,00/25](https://lore.kernel.org/lkml/20080606202838.390050172@redhat.com)[LORE v12,00/24](https://lore.kernel.org/lkml/20080611184214.605110868@redhat.com/) |
| 2009/05/13 | KOSAKI Motohiro <kosaki.motohiro@jp.fujitsu.com> | [Kconfig: CONFIG_UNEVICTABLE_LRU move into EMBEDDED submenu](https://lore.kernel.org/patchwork/patch/155947) | NA | v1 ☐ | [PatchWork v2](https://lore.kernel.org/patchwork/patch/155947), [LWN](https://lwn.net/Articles/286472) |
| 2009/06/16 | KOSAKI Motohiro <kosaki.motohiro@jp.fujitsu.com> | [mm: remove CONFIG_UNEVICTABLE_LRU config option](https://lore.kernel.org/patchwork/patch/156055) | 已经没有人想要关闭 CONFIG_UNEVICTABLE_LRU 了, 因此将这个宏移除. 内核永久使能 UNEVICTABLE_LRU. | v1 ☑ v2.6.31-rc1 | [PatchWork v1](https://lore.kernel.org/patchwork/patch/156055), [commit ](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6837765963f1723e80ca97b1fae660f3a60d77df) |

*   让代码文件缓存页多待一会(Being nicer to executable pages)

**2.6.31(2009年9月发布)**

试想, 当你在拷贝一个非常大的文件时, 你发现突然电脑变得反应慢了, 那么可能发生的事情是:

突然涌入的大量文件缓存页让内存告急, 于是 MM 开始扫描前面说的链表, 如果系统的设置是倾向替换文件页的话(**_swappiness_** 靠近0), 那么很有可能, 某个 C 库代码所在代码要在这个内存吃紧的时间点(意味扫描 active list 会比较凶)被挑中, 给扔掉了, 那么程序执行到了该代码, 要把该页重新换入, 这就是发生了 **Swap Thrashing** 现象了. 这体验自然就差了.

解决方法是[对可执行页面做一些保护](https://lore.kernel.org/patchwork/patch/156462), 参见 [Being nicer to executable pages](https://lwn.net/Articles/333742), 在扫描这些在使用中的代码文件缓存页时, 跳过它, 让它有多一点时间[待在 active 链表](https://elixir.bootlin.com/linux/v2.6.31/source/mm/vmscan.c#L1301)上, 从而避免上述问题.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/05/17 | Wu Fengguang <fengguang.wu@intel.com> | [vmscan: make mapped executable pages the first class citizen](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=3eb4140f0389bdada022d5e8efd88504ad30df14) | 将 VM_EXEC 的页面作为一等公民("the first class citizen") 区别对待. 在扫描这些在使用中的代码文件缓存页时, 跳过它, 让它有多一点时间待在 active 链表上, 从而防止抖动. 参见 [LWN: Being nicer to executable pages](https://lwn.net/Articles/333742) | v2 ☑ 2.6.31-rc1 | [LORE 0/3](https://lore.kernel.org/lkml/20090516090005.916779788@intel.com), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8cab4754d24a0f2e05920170c845bd84472814c6) |
| 2011/08/08 | Konstantin Khlebnikov <khlebnikov@openvz.org> | [vmscan: activate executable pages after first usage](https://lore.kernel.org/patchwork/patch/262018) | 上面补丁希望对 VM_EXEC 的页面区别对待, 但是添加的逻辑被明显削弱, 只有在第二次使用后才能成为一等公民("the first class citizen"), 由于二次机会法的存在, 这些页面将通过 page_check_references() 在第一次使用后才能进入 active LRU list, 然后才能触发前面补丁的优化, 保证 VM_EXEC 的页面有更好的机会留在内存中. 因此这个补丁 通过 page_check_references() 在页面在第一次被访问后就激活它们, 从而保护可执行代码将有更好的机会留在内存中. | v2 ☑ 3.3-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/262018), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c909e99364c8b6ca07864d752950b6b4ecf6bef4) |


*   rotate_reclaimable_page 处理 DIRTY 的文件页

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2002/12/02 | Andrew Morton <akpm@digeo.com> | [Move reclaimable pages to the tail ofthe inactive list on](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3b0db538ef6782a1e2a549c68f1605ca8d35dd7e) | 这个补丁解决了当非活动列表上有大量脏数据时出现的一些搜索复杂性故障. 通常, 我们会尝试写出这些页面, 然后将它们移动到非活动列表的开头. 但这不利于页面老化, 这意味着页面必须再次遍历整个列表才能回收.<br>因此, 我们在这个补丁中所做的是通过 SetPageReclaim() 将页面标记为需要回收, 然后启动 IO. 在 IO 完成处理程序 end_page_writeback() 中, TestClearPageReclaim() 检查页面是否仍然可能可回收, 如果是, 则使用 rotate_reclaimable_page() 将其移动到非活动列表的尾部, 在那里可以立即回收.<br>在交换密集型负载非常大的情况下, 这将页面回收效率(回收页面/扫描页面)从 10% 提高到 25%. 当前没有使用 pagevec 进行批处理操作, 因此每处理一个页面获取一次 LRU 锁. | v1 ☑✓ 2.5.51 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3b0db538ef6782a1e2a549c68f1605ca8d35dd7e) |




### 4.2.7 页面老化(active 与 inactive 链表拆分)
-------

#### 4.2.7.1 页面老化
-------


教科书式的 PFRA 会提到要用 LRU (Least-Recently-Used) 算法, 该算法思想基于 : 最近很少使用的页, 在紧接着的未来应该也很少使用, 因此, 它可以被当作替换掉的候选页.

但现实中, 要跟踪每个页的使用情况, 开销不是一般的大, 尤其是内存大的系统. 而且, 还有一个问题, LRU 考量的是近期的历史, 却没能体现页面的使用频率 - 假设有一个页面会被多次访问, 最近一次访问稍久点了, 这时涌入了很多只会使用一次的页(比如在播放电影), 那么按照 LRU 语义, 很可能被驱逐的是前者, 而不是后者这些不会再用的页面.

为此, Linux 引入了两个链表, 一个 active list, 一个 inactive list , 参见 [2.4.0-test9pre1 MM balancing (Rik Riel)](https://github.com/gatieme/linux-history/commit/1fc53b2209b58e786c102e55ee682c12ffb4c794). 这两个链表如此工作:

1.  inactive list 链表尾的页将作为候选页, 在需要时被替换出系统.

2.  对于文件缓存页, 当第一次被读入时, 将置于 inactive list 链表头. 如果它被再次访问, 就把它提升到 active list 链表尾; 否则, 随着新的页进入, 它会被慢慢推到 inactive list 尾巴; 如果它再一次访问, 就把它提升到 active list 链表头.

3.  对于匿名页, 当第一次被读入时, 将置于 active list 链表尾(对匿名页的优待是因为替换它出去要写入交换设备, 不能直接丢弃, 代价更大); 如果它被再次访问, 就把它提升到 active list 链表头.

4.  在需要换页时, MM 会从 active 链表尾开始扫描, 把足够量页面降级到 inactive 链表头, 同样, 默认文件缓存页会受到优待(用户可通过 **_swappiness_** 这个用户接口设置权重).

如上, 上述两个链表按照使用的热度构成了四个层级:

active 头(热烈使用中) > active 尾 > inactive 头 > inactive 尾(被驱逐者)

1.  这种增强版的 LRU 同时考虑了 LRU 的语义: 更近被使用的页在链表头;

2.  又考虑了使用频度: 还是考虑前面的例子, 一个频繁访问的页, 它极有可能在 active 链表头, 或者次一点, 在 active 链表尾, 此时涌入的大量一次性文件缓存页, 只会被放在 inactive 链表头, 因而它们会更优先被替换出去.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2.4.0-test9pre1 | Rik van Riel <riel@redhat.com> | [MM balancing (Rik Riel)](https://github.com/gatieme/linux-history/commit/1fc53b2209b58e786c102e55ee682c12ffb4c794) | 引入 MM balancing, 其中将 LRU 拆分成了 active_list 和 inactive_dirty_list 两条链表 | 2.4.0-test9pre1 | [1fc53b2209b](https://github.com/gatieme/linux-history/commit/1fc53b2209b58e786c102e55ee682c12ffb4c794) |



2.5.46 的时候, 设计了匿名页映射后, 优先加入到 active LRU list 中, 防止其被过早的释放. [commit 228c3d15a702 ("lru_add_active(): for starting pages on the active list")](https://github.com/gatieme/linux-history/commit/228c3d15a7020c3587a2c356657099c73f9eb76b), [commit a5bef68d6c85 ("start anon pages on the active list")](https://github.com/gatieme/linux-history/commit/a5bef68d6c85d26344dd31b4c342e5a365e68326).

但是这种改动又会导致另外一个问题, 这造成在某种场景下新申请的内存(即使被使用一次cold page)也会把在 active list 的 hot page 挤到 inactive list. 为了更好的解决这个问题, [workingset protection/detection on the anonymous LRU list](https://lwn.net/Articles/815342), 实现对匿名 LRU 页面列表的工作集保护和检测. 其中通过补丁 [commit b518154e59aa ("mm/vmscan: protect the workingset on anonymous LRU")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b518154e59aab3ad0780a169c5cc84bd4ee4357e) 将新创建或交换的匿名页面放到 inactive LRU list 中.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/10/31 | Andrew Morton <akpm@digeo.com> | [MM balancing (Rik Riel)](https://github.com/gatieme/linux-history/commit/1fc53b2209b58e786c102e55ee682c12ffb4c794) | 优化高压力 SWAP 下的性能, 为了防止匿名页过早的被释放, 在匿名页创建后先将他们加入到 active LRU list. | 2.5.46 | [HISTORY COMMIT1 228c3d15a70](https://github.com/gatieme/linux-history/commit/228c3d15a7020c3587a2c356657099c73f9eb76b)<br>*-*-*-*-*-*-*-*<br>[HISTORY COMMIT2 33709b5c802](https://github.com/gatieme/linux-history/commit/33709b5c8022486197ed2345eed18bbeb14a2251)<br>*-*-*-*-*-*-*-*<br>[HISTORY COMMIT3 a5bef68d6c8](https://github.com/gatieme/linux-history/commit/a5bef68d6c85d26344dd31b4c342e5a365e68326)<br>*-*-*-*-*-*-*-*<br> |
| 2016/07/29 | Minchan Kim <minchan@kernel.org> | [mm: move swap-in anonymous page into active list](https://lore.kernel.org/patchwork/patch/702045) | 将被换入的匿名页面(swapin)放到 active LRU list 中. | v5 ☑ [4.8-rc1](https://kernelnewbies.org/Linux_4.8#Memory_management) | [Patchwork v7](https://lore.kernel.org/patchwork/patch/702045), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1a8018fb4c6976559c3f04bcf760822381be501d) |
| 2020/04/03 | Joonsoo Kim <iamjoonsoo.kim@lge.com> | [workingset protection/detection on the anonymous LRU list](https://lwn.net/Articles/815342) | 实现对匿名 LRU 页面列表的工作集保护和检测. 在这里我们关心的是它将新创建或交换的匿名页面放到 inactive LRU list 中, 只有当它们被足够引用时才会被提升到活动列表. | v5 ☑ [5.9-rc1](https://kernelnewbies.org/Linux_5.9#Memory_management) | [Patchwork v7](https://lore.kernel.org/patchwork/patch/1278082)<br>*-*-*-*-*-*-*-*<br>[ZhiHu](https://zhuanlan.zhihu.com/p/113220105)<br>*-*-*-*-*-*-*-*<br>[关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b518154e59aab3ad0780a169c5cc84bd4ee4357e) |

2.6.31-rc1 合入了对只引用了一次的文件页面的处理, 极大地减少了一次性使用的映射文件页的生存期. 在此之前 VM 的实现中假设一个未激活的(处在 inactive LRU list 上)、映射的和引用的文件页面正在使用, 并将其提升到活动列表. 目前每个映射文件页面都是这样开始的, 因此当工作负载创建只在短时间内访问和使用的此类页面时, 就会出现问题. 这将造成 active list 中充斥大量这样的页面, 在进行页面回收时, VM 很快就会在寻找合格的回收候选对象时遇到麻烦. 结果是引起长时间的分配延迟和错误页面的回收. 补丁 [commit 645747462435 ("vmscan: detect mapped file pages used only once")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=645747462435d84c6c6a64269ed49cc3015f753d) 通过重用 PG_referenced 页面标志(用于未映射的文件页面)来实现一个使用检测, 来发现这些只引用了一次的页面. 该检测随着 LRU 列表循环的速度(即内存压力)而变化. 如果扫描程序遇到这些页面, 则设置该标志, 并在非活动列表上再次循环该页面. 只有当它返回另一个页表引用时, 它才能被激活. 否则它将被回收. 这有效地将使用过一次的映射文件页的最小生命周期从一个完整的内存周期更改为一个非活动的列表周期, 从而允许它在线性流(如文件读取)中发生, 而不会影响系统的整体性能.

但是这补丁也减少了所有共享映射文件页面的生存时间. 在这个补丁之后, 如果这个页面已经通过几个 ptes 被多次使用, page_check_references() 也会在第一次非活动列表扫描时激活这些文件映射页面.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2010/02/22 | Johannes Weiner <hannes@cmpxchg.org> | [vmscan: detect mapped file pages used only once](https://lore.kernel.org/patchwork/patch/189878) | 在文件页面线性流的场景, 源源不断的文件页面在被一次引用后, 被激活到 active LRU list 中, 将导致 active LRU list 中充斥着这样的页面. 为了解决这个问题, 通过重用 PG_referenced 页面标志检测和识别这类引用, 设置其只有在第二次引用后, 才能被激活. | v2 ☑ 2.6.31-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/189878), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=645747462435d84c6c6a64269ed49cc3015f753d) |
| 2011/08/08 | Konstantin Khlebnikov <khlebnikov@openvz.org> | [vmscan: promote shared file mapped pages](https://lore.kernel.org/patchwork/patch/262019) | 上面的补丁极大地减少了一次性使用的映射文件页的生存期. 不幸的是, 它还减少了所有共享映射文件页面的生存时间. 在这个补丁之后, 如果这个页面已经通过几个 ptes 被多次使用, page_check_references() 也会在第一次非活动列表扫描时激活文件映射页面. | v2 ☑ 3.3-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/262019), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=34dbc67a644f11ab3475d822d72e25409911e760) |


#### 4.2.7.2 多级 LRU
-------

[Linux 内核页面置换算法](https://blog.eastonman.com/blog/2021/04/linux-multi-lru/)

所有跟 MGLRU 相关的报道 [Phoronix: MGLRU](https://www.phoronix.com/scan.php?page=search&q=MGLRU)

hakavlad 在自己 github 仓库提供了 [mg-lru-helper](https://github.com/hakavlad/mg-lru-helper).

*   主体思想

原来内核只维护了 active 和 inactive 两个 LRU LIST, Yu Zhao 最近提交的 Patch 中提出了 Multigenerational LRU 算法, 旨在解决现在内核使用的两级 LRU 的问题. 这个多级 LRU 借鉴了老化算法的思路, 按照页面的生成(分配)时间将 LRU 表分为若干 Generation. 在 LRU 页面扫面的时候, 使用增量的方式扫描, 根据周期内访问过的页面对页表进行扫描, 除非这段时间内访问的内存分布非常稀疏, 通常页表相对于倒排页表有更好的局部性, 进而可以提升 CPU 的缓存命中率.

Multigenerational LRU 将 LRU 列表划分为多级. 当前实现试图增加两个中间状态, 即 likely to be active 和 likely to be unused. 这样不至于错误的回收 likely to be active 的页面, 也不至于对 likely to be unused 的页面置之不理. 设想是希望更有效的回收页面, 确保能够及时的回收内存. 参见 [Multi-generational LRU: the next generation](https://lwn.net/Articles/856931), [The multi-generational LRU](https://lwn.net/Articles/851184).


*   性能数据


最初在邮件列表中 Yu Zhao 说明了他使用更改后的算法在几个模拟场景中的情况, 参见 [Google Proposes Multi-Generational LRU For Linux To Yield Much Better Performance](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Multigen-LRU).

Android:    减少了 18% 的 low memory kill, 进而减少了 16% 的冷启动.
Borg(Google 的云资源编排系统):  活跃内存占用降低了.
ChromeOS:   非活跃页面减少了 96% 的内存占用, 减少了 59% 的 OOM Kill.

随后越来越多的人开始看好病关注到此特性, 并进行了大量的测试:

v5 测试时 [MariaDB 高并发条件](https://patchwork.kernel.org/project/linux-mm/cover/20210818063107.2696454-1-yuzhao@google.com/#24506153) 下, 在内存略微过度使用时, 每分钟 TPM 的事务数分别增加了 95% [5.24, 10.71]% 和 [20.22, 25.97]%. 在其他条件下, TPM 没有统计学上的显着变化. 参见["MGLRU" Code Updated For More Performant Linux Page Reclamation](https://www.phoronix.com/scan.php?page=news_item&px=Multigen-LRU-v5).

v6 测试时, Redis, PostgreSQL, MongoDB, Memcached, Hadoop, Spark, Cassandra, MariaDB 和其他工作负载的最新基准测试看起来都非常有希望. 参见 [MGLRU Is A Very Enticing Enhancement For Linux In 2022](https://www.phoronix.com/scan.php?page=news_item&px=Linux-MGLRU-v6-Linux).

v8 和 v9 测试时, 测试场景进一步扩大, 参见 [MGLRU Continues To Look Very Promising For Linux Kernel Performance](https://www.phoronix.com/scan.php?page=news_item&px=Linux-MGLRU-v9-Promising). 此时正处于 v5.18 开发窗口, 作者 Yu Zhao 发起了 Pull Request, 随后 [Multi-gen LRU for 5.18-rc1](https://lore.kernel.org/lkml/20220326010003.3155137-1-yuzhao@google.com), 但是并没有被直接合入. [MGLRU Could Land In Linux 5.19 For Improving Performance - Especially Low RAM Situations](https://www.phoronix.com/scan.php?page=news_item&px=MGLRU-Not-For-5.18).

v10 基本趋于稳定, 已经尝试发送合并请求 [LWN: Merging the multi-generational LRU](https://lwn.net/Articles/894859). 参见 [MGLRU Revised A 10th Time For Improving Linux Performance, Better Under Memory Pressure](https://www.phoronix.com/scan.php?page=news_item&px=MGLRU-v10).

至此, MGLRU 已经在  Cassandra, Hadooop, MySQL/MariaDB, Memcached, MongoDB, PostgreSQL, Redis 等场景都获得了较好的结果.

v11 版本时, Google 已经向数千万 Chrome OS 用户和大约 100 万 Android 用户推出了 MGLRU. Google 的 Fleetwide 分析显示, 除了其他 UX 指标的改进之外, kswapd CPU 使用率总体降低了 40%, 其中第 75 百分位的 low-memory kills 次数减少了 85%, 第 50 百分位的应用启动时间减少了 18%. [MGLRU 再次为有希望的 Linux 性能改进而加速](https://www.phoronix.com/scan.php?page=news_item&px=MGLRU-v11-Linux-Perf).

v12 版本已经没有什么特性变更, 仅仅是一些 bugfix, 参见 phoronix 报道 [Performance-Boosting MGLRU Patches Updated Against Current Linux 5.19 State](https://www.phoronix.com/scan.php?page=news_item&px=MGLRU-v12-For-Linux-5.19-rc). 随后 phoronix 也对 MGLRU v12 做了一些基础的 benchmark 测试, 在进行的数十个基准测试中, 大多数测试显示性能没有变化. 但是, 当涉及到数据库等 I/O 工作负载时, MGLRU显示出实质性的帮助.

接着 7 月份发布了 v13 版本, phoronix 继续补充了性能测试, 参见 phoronix 报道 [MGLRU v13 Published, Benchmarks Continue Looking Good For This Kernel Feature](https://www.phoronix.com/scan.php?page=news_item&px=MGLRU-v13-More-Benchmarks).

*   实现

传统的 LRU 页面回收仅仅通过 ACTIVE/INACTIVE 划分页面的冷热和老化程度, 这是一锤子买卖, 粒度非常粗, 对页面也机器不友好, 一个页面要么热页, 可以被宣判延刑, 要么是冷页, 可以立即被回收. 而 MGLRU 将页面的冷热程度做了更细粒度的划分.
因此 MGLRU 通过 generation number 来标记页面的老化程度, 只区分匿名页 LRU_GEN_ANON 和文件页 LRU_GEN_FILE. 然后使用 struct lru_gen_struct 维护了 LRU 列表. 其中 max_seq

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/04/07 | Yu Zhao <yuzhao@google.com> | [Multigenerational LRU Framework(https://lwn.net/Articles/856931) | Multi-Gen LRU Framework, 将 LRU 的列表划分为多代老化. 通过 CONFIG_LRU_GEN 来控制. | v10 ☐ | [Patchwork v1,00/14](https://lore.kernel.org/patchwork/patch/1394674)<br>*-*-*-*-*-*-*-*<br>[PatchWork v2,00/16](https://lore.kernel.org/patchwork/patch/1412560)<br>*-*-*-*-*-*-*-*<br>[2021/05/20 PatchWork v3,00/14](https://patchwork.kernel.org/project/linux-mm/cover/20210520065355.2736558-1-yuzhao@google.com)<br>*-*-*-*-*-*-*-*<br>[2021/08/18 PatchWork v4,00/11](https://patchwork.kernel.org/project/linux-mm/cover/20210818063107.2696454-1-yuzhao@google.com)<br>*-*-*-*-*-*-*-*<br>[2021/11/11 PatchWork v5,00/10](https://patchwork.kernel.org/project/linux-mm/cover/20211111041510.402534-1-yuzhao@google.com)<br>*-*-*-*-*-*-*-*<br>[2022/01/04 PatchWork v6,0/9](https://patchwork.kernel.org/project/linux-mm/cover/20220104202227.2903605-1-yuzhao@google.com)<br>*-*-*-*-*-*-*-*<br>[2022/02/08 PatchWork v7,0/12](https://lore.kernel.org/all/20220208081902.3550911-1-yuzhao@google.com)<br>*-*-*-*-*-*-*-*<br>[2022/03/08 LORE v8,0/14](https://lore.kernel.org/all/20220308234723.3834941-1-yuzhao@google.com)<br>*-*-*-*-*-*-*-*<br>[2022/03/09 LORE v9,0/14](https://lore.kernel.org/all/20220309021230.721028-1-yuzhao@google.com)<br>*-*-*-*-*-*-*-*<br>[2022/04/07 LORE,v10,00/14](https://lore.kernel.org/lkml/20220407031525.2368067-1-yuzhao@google.com)<br>*-*-*-*-*-*-*-*<br>[LORE v11,00/14](https://lore.kernel.org/lkml/20220518014632.922072-1-yuzhao@google.com)<br>*-*-*-*-*-*-*-*<br>[LORE v12,00/14](https://lore.kernel.org/lkml/20220614071650.206064-1-yuzhao@google.com)<br>*-*-*-*-*-*-*-*<br>[LORE v13,00/14](https://lore.kernel.org/lkml/20220706220022.968789-1-yuzhao@google.com) |





### 4.2.8 工作集大小的探测(Better LRU list balancing)
-------

https://lore.kernel.org/patchwork/patch/222042
https://github.com/hakavlad/le9-patch

**3.15(2014年6月发布)**

LRU 链表被分为 inactive 和 active 链表:

*   在 active 的页面如果长时间未访问, 则会被降级到 inactive 链表, inactive 链表的页面一段时间未被访问后, 则会被释放.

*   处于 inactive 链表表头的页面, 如果它没被再次访问, 它将被慢慢推到 inactive 链表表尾, 最后在回收时被回收走; 而如果有再次访问, 它会被提升到 active 链表尾, 再再次访问, 提升到 active 链表头

但是(由于内存总量有限)如果 inactive 链表较长就意味着 active 链表相对较小; 这会引起大量的 "soft page fault", 最终导致整个系统的速度减慢. 因此, 作为内存管理决策的一部分, 两个链表的相对大小也需要采取一定的策略加以调节并保持适当的平衡.

因此, 可以定义一个概念: **访问距离, 它指该页面第一次进入内存到被踢出的间隔, 显然至少是 inactive 链表的长度.**

另外一方面, 非活动的列表应该足够小, 这样 LRU 扫描的时候不必做太多的工作. 但是要足够大, 使每个非活动页面在被淘汰之前有机会再次被引用.

**那么问题来了: 这个 inactive 链表的长度得多长? 才能既控制页面回收时扫描的工作量, 又保护该页面在第二次访问前尽量不被踢出, 以避免 Swap Thrashing 现象.**

#### 4.2.8.1 早期的 inactive 和 active 的均衡
-------


早期的工作集探测非常简陋, 并没有通过 inactive/active list 的页面数量来保持 inactive/active 的平衡. 而仅仅是在页面分配等路径下如果发现页面紧缺(VM SHORTAGE), 则尝试从 LRU 中回收部分页面.

*   最原始的 balance

[commit 1742f19fa920 ("Linux 2.4.0-test9pre1/MM balancing (Rik Riel)")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1fc53b2209b58e786c102e55ee682c12ffb4c794) 的时候, 就引入了 refill_inactive(), 通过调用 refill_inactive_scan() 来将页面从 active_list 驱逐到 inactive_list.

*   VM shortage age 时期

开始是 [commit a880f45a48be ("v2.4.7 -> v2.4.7.1/Marcelo Tosatti: per-zone VM shortage/Daniel Phillips: generic use-once optimization instead of drop-behind")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6fbaac38b85e4bd3936b882392e3a9b45e8acb46) 的时候, 实现了新的 shortage 页面老化算法, 引入 zone_free_shortage(), zone_inactive_plenty(), zone_inactive_shortage() 以及 refill_inactive_zone(), refill_inactive().

接着 [commit a880f45a48be ("v2.4.7.6 -> v2.4.7.7/Linus Torvalds: sane and nice VM balancing")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=dfc05323cf886ed7faca146cbee5cdad1a62157f) 的时候, 对 shortage 老化算法的代码做了一些精简.

随后 [commit a67f1b5da2cf ("v2.4.8 -> v2.4.8.1")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a67f1b5da2cf8b14395596048c876247b894aa5c) 的时候, 对 shortage 老化算法进一步进行了完善, 移除了 refill_inactive_zone() 和 移除了 zone_free_shortage(), zone_inactive_plenty(), zone_inactive_shortage() 的老化算法.


*   active 2/3, inactive 1/3 的比例

[commit a880f45a48be ("v2.4.9.10 -> v2.4.9.11/Andrea Arkangeli: major VM merge")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a880f45a48be2956d2c78a839c472287d54435c1) 的时候, 移除了老旧的 VM shortage 思想, 开始使用 Balance. 引入 [check_classzone_need_balance()](https://elixir.bootlin.com/linux/v2.5.0/source/mm/vmscan.c#L613) 检查各个 zone 是否页面紧缺.

如果 [`classzone->free_pages > classzone->pages_high`](https://elixir.bootlin.com/linux/v2.5.0/source/mm/vmscan.c#L613) 则说明当前 zone 页面还是足够的, 否则则认为页面紧缺. 分配页面的时候, 如果发现 [classzone->need_balance](https://elixir.bootlin.com/linux/v2.5.0/source/mm/page_alloc.c#L328), 则唤醒 kswapd 来回收页面. , 则 kswapd 就通过 kswapd_balance() -=> kswapd_balance_pgdat() -=> try_to_free_pages() shrink_caches() 回收各个 zone 的页面, 直到 check_classzone_need_balance() 认为当前 zone 页面充足.

```cpp
kswapd_balance()
-=> kswapd_balance_pgdat()
    -=> try_to_free_pages()
        -=> shrink_caches()
            -=> shrink_cache()
```

[commit dfc52b82fee5 ("v2.4.9.11 -> v2.4.9.12")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=dfc52b82fee5bc6713ecce3f81767a8565c4f874) 中重新引入了 refill_inactive() 将一定数量的页面从 active_list 移动到 inactive_list 中, 此时 shrink_caches() 中每次通过 refill_inactive() 将一半页面移动到 inactive_list 上. 初始移动页面为 `SWAP_CLUSTER_MAX/2`.


在这个过程中, 内核开发者发现如果有效地控制 inactive LRU list 和 active LRU list 的页面比例, 可以有效地提升性能. 因此在这个过程中做了不断地尝试.

首先 [commit a27c6530ff12 ("v2.4.9.12 -> v2.4.9.13")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a27c6530ff12bab100e64c5b43e84f759fa353ae) 引入了 balance_inactive() 替代 refill_inactive(), 检查 nr_active_pages 的页面是否小于 nr_inactive_pages, 否则则将 active LRU list 中的部分页面迁移到 inactive LRU list. 并在 shrink_caches() 中通过 balance_inactive() 进行了 balance. 每次移动的页面数量也不再减半.

接着很长一段时间, 主要保持 nr_active_pages 不超过 nr_inactive_pages 的一半. 首先 [commit e2f6721a0a1b ("v2.4.9.14 -> v2.4.9.15")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e2f6721a0a1b07612c0682d8240d3e9bc0a445a4) 将 balance_inactive() 修改为 refill_inactive(), 并移除了 balance 条件的判断, 将 balance 的判定操作直接内置到了 shrink_caches() 中. 在 shrink_caches() 中保持 nr_active_pages 不超过 nr_inactive_pages 的一半(`nr_inactive_pages < nr_active_pages*2`), 否则则要进行 refill_inactive() 操作.

然后 [commit a356c406f36e ("v2.4.10.0.3 -> v2.4.10.0.4")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a356c406f36e65105272b1a9127c0098a113511c) 将 shrink_caches() 中保持 nr_active_pages 和 nr_active_pages 的 balance 比例, 修改为 nr_active_pages 不能超过 page cache 总数的 2/3, 否则则要进行 refill_inactive() 操作. 之后这个比例保持了很久. 并经历了诸多变更.

比如 v2.5.19 [commit ce677ce203cd ("move nr_active and nr_inactive into per-CPU page")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ce677ce203cdb7bf92e0c1babcd2322446bb735e) 将 nr_active 和 nr_inactive 的计数放到了 page_state 结构中.

[commit 3aa1dc772547 ("multithread page reclaim")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3aa1dc772547672e6ff453117d169c47a5a7cbc5), 这个补丁多线程处理主页面回收函数 shrink_cache(). 同时更改 shrink_caches() 的逻辑, 以便它仍然会缓慢地通过 refill_inactive() 处理活动列表 即使不活动列表比活动列表大得多. 此外之前 refill_inactive() 被调用得太频繁了, 多数时候通常只处理两三个页面. 修改后, 它以同样的速度处理页面, 但一次可以处理 SWAP_CLUSTER_MAX(即 32) 个页面. shrink_cache() 这个函数在 pagemap_lru_lock 下运行. 通过获取该锁, 将 LRU 中的 32 个页面放入一个私有列表中, 接着释放 pagemap_lru_lock, 然后继续尝试释放这些页面. 成功回收的任何页面都将被批处理释放. 未回收的页面会重新添加到 LRU 中. 这个补丁将 pagemap_lru_lock 争用减少了 30 倍.

v2.5.33 [commit e6f0e61d9ed9 ("per-zone-LRU")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e6f0e61d9ed94134f57bcf6c72b81848b9d3c2fe) 的时候, refill_inactive() ==> refill_inactive_zone(), 引入 shrink_zone() 回收 zone 上的页面. 不过 refill_inactive_zone 依旧一次批量处理 SWAP_CLUSTER_MAX 个页面.

一直到 [v2.6.6 版本](https://elixir.bootlin.com/linux/v2.6.6/source/mm/vmscan.c#L752), 这个阶段, active/inactive list balance 总体上依旧比较简陋, 总是通过控制 active/inactive list 的长度比例来限制扫描 ratio(扫描的页面长度). 总体预期依旧是控制 nr_active_pages 不能超过 page cache 总数的 2/3.

但是这样直接比较 active 和 inactive list 长度的方式, 如果区域中有非常少的非活动页面, 那么扫描的 ratio 可能会非常大, 这会耗时过长, 同样如果 inactive list 与 active list 的大小相比变得非常小, 那么活动列表扫描长度 nr_scan_active(这些页面将会添加到 inactive lisr)又会很小, 导致只有极少数的页面被添加到 inactive list, 回收的页面有限, 无法有效地缓解内存压力.

因此 v2.6.7-rc1 [commit 6fa1d901d520 ("Fix arithmetic in shrink_zone()")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6fa1d901d520c1414a1cb3386bbaf0c51a445e20) 对 active/inactive list 长度比例 进行了区分, 比例超过 8 倍时, 则[限制 active 扫描长度 nr_scan_active 不能超过 inactive list 的 4 倍](https://elixir.bootlin.com/linux/v2.6.7/source/mm/vmscan.c#L818), 防止扫描过长, 导致耗时过久. 其他情况又[限制扫描长度不要过短](https://elixir.bootlin.com/linux/v2.6.7/source/mm/vmscan.c#L826). 此外在考虑不同 active/inactive list 长度比例的同时, 还考虑 `zone->nr_active + zone->nr_inactive` 的大小](https://elixir.bootlin.com/linux/v2.6.7/source/mm/vmscan.c#L804). 它还可以隐藏主动和非主动平衡实现, 使其不受更高级别扫描代码的影响. 但是还是倾向于维护 active 占比 2/3 的比例.

接着 v2.6.7 [commit 72a9defa9a4f ("vmscan.c: struct scan_control")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=72a9defa9a4fda3a4e1f9ed67b74a11e756cc3cc) 引入了 struct scan_control 封装了 LRU 每次扫描的力度信息. 精简了函数的参数列表长度集合.

可见, 一直到 v2.6.7 总体预期依旧是控制 nr_active_pages 不能超过 page cache 总数的 2/3.

1.  通过判断 active list 的长度是否超过 inactive list 的长度在一定比例范围, 来决策是否对 active 页面进行驱逐, 以及 inactive 页面进行回收. 通过 refill_inactive_zone()(早期是 refill_inactive()) 将一定数量 nr_scan_active (通常是至少 SWAP_CLUSTER_MAX, 也就是 32) 的页面从 active LRU list 移动到 inactive LRU list. 以及通过 shrink_cache() 将一定数量 nr_scan_inactive 的页面进行回收.

2.  在页面分配的过程中, 如果发现内存紧缺, 分配存在压力, 则唤醒 kswapd 通过 try_to_free_pages 开始执行进行页面驱逐以及页面回收, 规则满足第一条.


*   按照 sc->priority 等比例扫描

经历了多年的运行和测试后, 大家发现保持 active 2/3, inactive 1/3 的占比貌似并没有什么实际的理论基础和效果, 因此 v2.6.8-rc1 [commit 2332dc7870b6 ("vmscan.c scan rate fixes")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2332dc7870b6f40eff03df88cbb03f4ffddbd086) 直接去掉了这种机械化的策略. 直接用 sc->priority 控制扫描的速度, 且让 active 和 inactive 两个列表上的所有页面都有相同的扫描速度. 此时 [`zone->nr_scan_active += (zone->nr_active >> sc->priority) + 1`](https://elixir.bootlin.com/linux/v2.6.8/source/mm/vmscan.c#L811), 以及 [`zone->nr_scan_inactive += (zone->nr_inactive >> sc->priority) + 1`](https://elixir.bootlin.com/linux/v2.6.8/source/mm/vmscan.c#L818). 随后这个状态持续了很久.

v2.6.17-rc1 [commit 1742f19fa920 ("vmscan: rename functions")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1742f19fa920cdd6905f0db5898524dde22ab2a4) 对原来分歧和不一致的函数命名就行了修改, refill_inactive_zone() ==> shrink_active_list(), shrink_cache() ==> shrink_inactive_list(), shrink_caches() ==> shrink_zones(). 这样所有 LRU 页面驱逐和页面回收的接口都改成了 shrink_xxx() 的形式.


[commit 5d5d36943259 ("Speed freeing memory for suspend.")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5d5d36943259171d2a90f2499541c8a8f4e90d54)


*   另外一个平衡的维持:匿名页和文件页的平衡(anon/file reclaim balancing)

前面所有的操作都在处理如何维持 active 和 inactive LRU 列表的平衡, 但是 LRU 中页面也是分类型的, 虽然这个时候还没有区分 ANON LRU 和 FILE LRU, 但是维持匿名页和文件页的平衡也是有意义的. 这个我们会在下一节展开讲.


#### 4.2.8.2 inactive_is_low() && get_scan_count() 共同维持 LRU 的平衡
-------

早期内核采取的平衡策略相对简单: 仅仅控制 active list 的长度不要超过 inactive list 的长度一定比例, 参见 [inactive_list_is_low(), v3.14, mm/vmscan.c, line 1799](https://elixir.bootlin.com/linux/v3.14/source/mm/vmscan.c#L1799), 具体的比例[与 inactive_ratio 有关](https://elixir.bootlin.com/linux/v3.14/source/mm/page_alloc.c#L5697). 其中对于文件页更是直接[要求 active list 的长度不要超过 inactive list](https://elixir.bootlin.com/linux/v3.14/source/mm/vmscan.c#L1788).

inactive LRU list 应该足够小, 这样 VM 就不需要做太多的工作, 但是应该足够大, 这样每个 inactive 页面在被交换之前都有机会再次被引用. 这本身是一个很矛盾的问题. 因此 v2.6.28-rc1 [VM pageout scalability improvements (V12)](https://lore.kernel.org/lkml/20080611184214.605110868@redhat.com) 系列中在将 LRU 拆成匿名页和文件页的时候, 这就又引入了另外一个问题. 以前我们不区分页面类型, 只需要进行 inactive 和 active 的比例均衡就可以了, 但是现在又引入了匿名页和文件页不同类型的 LRU, 他们之间的列表的长度应该保持怎样的均衡.

1.  首先是 active 和 inactive 的平衡(active/inactive balancing), 使用 inactive_is_low() 探测 active/inactive balancing.

[vmscan: second chance replacement for anonymous pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=556adecba110bf5f1db6c6b56416cfab5bcab698). 引入 zone->inactive_ratio 控制 active 和 inactive 的比例. 然后 balance_pgdat() 和 shrink_zone() 以及 shrink_list() 中都通过 [inactive_anon_is_low()](https://elixir.bootlin.com/linux/v2.6.28/source/include/linux/mm_inline.h#L88) 检查当前 zone 上的 inactive LRU 的匿名页是否小于 active LRU 一定比例, 如果是, 则说明 inactive LRU list 过短, 需要将一部分 active 的页面 deactivated 到 inactive LRU. 这个比率 zone->inactive_ratio 在初始化阶段通过 setup_per_zone_inactive_ratio() 完成初始化. 与内存的大小有直接关系, 默认被设置为 sqrt(memory in GB * 10), 加入 zone->inactive_ratio 为 3 则意味着 active 和 inactive 的比例为: 3:1, 即 25% 的匿名页面应该保留在 inactive list 中. 注意这个阶段的 zone->inactive_ratio 只控制匿名页的占比.

2.  其次是匿名页和文件页的平衡(anon/file reclaim balancing), 使用 get_scan_count() 处理 anon/file reclaim balancing.

[commit 4f98a2fee8ac ("vmscan: split LRU lists into anon & file sets")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4f98a2fee8acdb4ac84545df98cccecfd130f8db) 对 LRU 中的页面进一步针对是匿名页和文件页进行了区分, 引入了 get_scan_ratio() 替代 calc_reclaim_mappe() 来确定应以多大力度扫描 anon 和 file LRU 列表. 每一组 LRU 列表的相对值是通过查看扫描的页面的比例来确定的, 我们将其旋转回活动列表, 而不是逐出. 计算的结果放到 percent[2] 的数组中, 其中 percent[0] 指定对匿名页 LRU 施加多大压力, 而 percent[1] 则制定对文件 LRU 施加的压力.


至此, shrink_list() 中同时处理了 anon/file reclaim balancing 和 active/inactive balancing. 这套机制维持了很久.


*   inactive_is_low() 探测 inactive 和 active 的平衡性(Reclaim pressure balance between active and inactive list)


首先是 inactive_anon_is_low()

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/06/11 | Rik van Riel <riel@redhat.com> | [vmscan: second chance replacement for anonymous pages](https://lore.kernel.org/patchwork/patch/118976) | [VM pageout scalability improvements (V12)](https://lore.kernel.org/lkml/20080611184214.605110868@redhat.com) 系列中的一个补丁.<br>在大多数情况下, 我们避免了驱逐和扫描匿名页面, 但在某些工作负载下, 我们可能最终会让大部分内存充满匿名页面. 这时, 我们突然需要清除所有内存上的引用位, 这可能会耗时很长. 当取消激活匿名页面时, 我们可以通过不考虑引用状态来减少需要扫描的页面的最大数量. 毕竟, 每个匿名页面一开始都是被引用的.如果匿名页面在到达非活动列表的末尾之前再次被引用, 我们将其移回活动列表.<br>为了使所需的最大工作量保持合理, 这个补丁<br>1. 引入了 inactive_ratio 根据内存大小伸缩活动与非活动的比率, 使用公式 inactive_ratio = active/inactive = sqrt(memory in GB * 10). 注意当前只支持匿名页面的转换比率.<br>2. 引入了 inactive_anon_is_low() 检查当前 zone 上的 active LRU 的匿名页是否过多需要被 deactivated. | v12 ☑ [2.6.28-rc1](https://kernelnewbies.org/Linux_2_6_28#Various_core) | [PatchWork v2](https://lore.kernel.org/patchwork/patch/118976), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=556adecba110bf5f1db6c6b56416cfab5bcab698) |
| 2008/12/01 | KOSAKI Motohiro <kosaki.motohiro@jp.fujitsu.com> | [memcg: split-lru feature for memcg](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=7f016ee8b6a9a43f768e6252021f169abec4fa1f) | 为 MEMCG 实现了 mem_cgroup_inactive_anon_is_low(), 与全局的 inactive_anon_is_low_global() 共同服务于 [inactive_anon_is_low()](https://elixir.bootlin.com/linux/v2.6.29/source/mm/vmscan.c#L1336). 移除了 mem_cgroup_cal_reclaim(), 使用 get_scan_count() 统一处理全局和 MEMCG 的 anon/file reclaim balancing. 这样一个统一的 LRU 驱逐和回收流程就接管了全系统不管是全局还是 MEMCG 的 shrink 操作. | v1 ☑✓ 2.6.29-rc1 | [2008/11/30 LORE v1,0/9](https://lore.kernel.org/all/20081130193502.8145.KOSAKI.MOTOHIRO@jp.fujitsu.com)<br>*-*-*-*-*-*-*-*<br>[2008/12/01 LORE v2,00/11](https://lore.kernel.org/lkml/20081201205810.1CCA.KOSAKI.MOTOHIRO@jp.fujitsu.com) |


其次是 inactive_file_is_low()

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/11/18 | Rik van Riel <riel@redhat.com> | [vmscan: evict streaming IO first](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9ff473b9a72942c5ac0ad35607cae28d8d59ed7a) | 在用于驱动换页扫描代码的统计中计算新页面的插入. 这将帮助内核快速地退出流文件IO.<br>我们依赖于这样一个事实:新文件页面在非活动文件LRU上开始, 而新的匿名页面在活动的匿名列表上开始. 这意味着流式文件IO将增加最近扫描的文件统计, 而不影响最近旋转的文件统计, 驱动分页扫描到文件LRUs. Pageout活动执行它自己的列表操作. | v1 ☑ 2.6.16-rc1 | [PatchWork v2](https://lore.kernel.org/lkml/20081117190642.3aabd3ff@bree.surriel.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9ff473b9a72942c5ac0ad35607cae28d8d59ed7a) |
| 2009/04/29 | Rik van Riel <riel@redhat.com> | [vmscan: evict use-once pages first (v3)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=56e49d218890f49b0057710a4b6fef31f5ffbfec) | 优先驱逐那些仅使用了一次的页面.<br>当文件 LRU 列表由流 IO 页面主导时, 在考虑驱逐其他页面之前, 先驱逐这些页面(因为这些页面往往只使用了一次, 而且后面不会再使用).<br>该补丁引入了 inactive_file_is_low(), 当发现 inactive_file_is_low() 的时候, 尝试驱逐 LRU_ACTIVE_FILE 上的页面.<br>与 inactive_anon_is_low() 类似, 同时提供了全局的 inactive_file_is_low_global() 和针对 MEMCG 的 mem_cgroup_inactive_file_is_low() 共同服务于 inactive_file_is_low(). 其中 inactive_file_is_low_global() 当前只要 inactive 长度小于 active 就认为是 LOW 的, 而 mem_cgroup_inactive_file_is_low() 当前未实现具体策略, 永远认为是 LOW 的. | v3 ☑ 2.6.31-rc1 | [Patchwork v1](https://lore.kernel.org/patchwork/patch/153805)<br>*-*-*-*-*-*-*-*<br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/153955)<br>*-*-*-*-*-*-*-*<br>[PatchWork v3](https://lore.kernel.org/patchwork/patch/153980)<br>*-*-*-*-*-*-*-*<br>[COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=56e49d218890f49b0057710a4b6fef31f5ffbfec) |
| 2013/02/22 | Johannes Weiner <hannes@cmpxchg.org> | [mm: refactor inactive_file_is_low() to use get_lru_size()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e3790144c9091631a18564aa64db8a971da02c41) | 随后 [mm: refactor inactive_file_is_low() to use get_lru_size()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=4805b02e90187c68d8f4e3305c3482b797e35809) 移除了未实现的 mem_cgroup_inactive_file_is_low().<br>inactive_file_is_low() 中不再区分是否是全局还是 MEMCG, 统一用 get_lru_size() 获取 LRU list 长度, 只要 inactive 长度小于 active 就认为 inactive 是 LOW 的.(即对于文件页 active/inactive file list 各占 50%). | v1 ☑✓ 3.9-rc1 | [LORE v1](https://lore.kernel.org/lkml/1359699116-7277-1-git-send-email-hannes@cmpxchg.org), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e3790144c9091631a18564aa64db8a971da02c41) |


*   最后是统一的 inactive_is_low()


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/07/16 | Konstantin Khlebnikov <khlebnikov@openvz.org> | [throttle direct reclaim when too many pages are isolated already (v3)](https://lore.kernel.org/lkml/20090715235318.6d2f5247@bree.surriel.com) | 当隔离的页面过多时限制直接回收的进行.<br>当太多进程进入直接回收时, 所有页面都有可能从 LRU 上取下. 这样做的一个结果是, 页面回收代码中的下一个进程认为已经没有可回收的页面了, 并触发内存不足终止.<br>这个问题的一个解决方案是永远不要让太多进程进入页面回收路径, 从而导致整个 LRU 被清空. 将系统限制为仅隔离一半的非活动列表进行回收应该是安全的.<br>这个补丁引入了 too_many_isolated() 函数. | v3 ☑✓ 2.6.32-rc1 | [PatchWork v3](https://lore.kernel.org/lkml/20090715235318.6d2f5247@bree.surriel.com), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=35cd78156c499ef83f60605e4643d5a98fef14fd) |
| 2009/11/25 | Rik van Riel <riel@redhat.com> | [vmscan: do not evict inactive pages when skipping an active list scan](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b39415b2731d7dec5e612d2d12595da82399eedf) | AIM7运行时, 发现最近的内核提前触发了匿名页的换出(swapping out). 这是由于 shrink_list() 中[在 !inactive_anon_is_low() 的时候依旧执行了 shrink_inactive_list()](https://elixir.bootlin.com/linux/v2.6.32/source/mm/vmscan.c), 而我们真正想做的是让一些匿名页面提前老化, 以便它们在非活动列表上有额外的时间被引用. 显而易见的解决办法是, 当我们调用 shrink_list() 来扫描一个活动列表时, 确保回收时不会落入扫描/回收非活动页面的陷阱. 因此如果 shrink 的是 active LRUs, 则不要再回收 inactive LRUs. 这个更改是安全的, 因为 [shrink_zone()](https://elixir.bootlin.com/linux/v2.6.32/source/mm/vmscan.c#L1630) 中的循环确保在应该回收时仍然会收缩 anon 和文件非活动列表.<br>这个补丁引入了 [inactive_list_is_low()](https://elixir.bootlin.com/linux/v2.6.33/source/mm/vmscan.c#L1464). | v1 ☑✓ 2.6.33-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/179466) |
| 2012/04/26 | Konstantin Khlebnikov <khlebnikov@openvz.org> | [mm: replace struct mem_cgroup_zone with struct lruvec](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f9be23d6da035241b7687b25e64401171986dcef) | memcg naturalization -rc5](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=6b208e3f6e35aa76d254c395bdcd984b17c6b626) 引入了 lruvec 和 [mem_cgroup_zone](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f16015fbf2f6ac45505d6ad21455ff9f6c14473d), 这里为了[移除 mem_cgroup_zone](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f9be23d6da035241b7687b25e64401171986dcef), 添加了 [zone 到 lruvec 的联系](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7f5e86c2ccc1480946d2c869d7f7d5278e828092), 从而 MEMCG 中可以直接通过 lruvec->zone 获取到 zone, 之前 zone 中已经包含了 lruvec, 至此不管是 MEMCG 还是 global 都可以通过 lruvec_zone() 函数直接从 lruvec 查到对应的 zone.<br>这个过程中 inactive_file_is_low() 等函数的参数都从 struct mem_cgroup_zone 修改为 struct lruvec. | v1 ☑✓ 3.5-rc1 | [LORE v1,00/12](https://lore.kernel.org/all/20120426074632.18961.17803.stgit@zurg), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c56d5c7dfeb5cc754e17fa3d423086a3c551c219) |
| 2016/01/29 | Johannes Weiner <hannes@cmpxchg.org> | [mm: workingset: per-cgroup thrash detection](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=23047a96d7cfcfca1a6d026ecaec526ea4803e9e) | TODO | v2 ☑✓ 4.6-rc1 | [LORE v2,0/5](https://lore.kernel.org/all/1454090047-1790-1-git-send-email-hannes@cmpxchg.org) |
| 2016/04/04 | Johannes Weiner <hannes@cmpxchg.org> | [mm: vmscan: reduce size of inactive file list](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=59dc76b0d4dfdd7dc46a1010e4afb44f60f3e97f) | [mm: support bigger cache workingsets and protect against writes](https://lore.kernel.org/lkml/1459790018-6630-1-git-send-email-hannes@cmpxchg.org) 的其中一个补丁, 整个 patchset 修复了数据库场景的 Page Cache LRU 处理的一些问题, 之前的 2 个补丁不再允许那些只写没有读的文件页被激活到 active list 上, 从而将 active list 上一些频繁访问的文件页面反而被驱逐到 inactive list, 造成性能抖动. 这个补丁删除了要求[非活动文件列表的最小文件缓存大小为 50%](https://elixir.bootlin.com/linux/v4.6/source/mm/vmscan.c#L1929) 的强制措施. 采用通过 workingset refault() 来进行探测的方法, 可以使在较短时间间隔后再次访问的最近收回的文件页直接升级到活动列表. 有了这种机制, 我们可以(在更大的系统上)为活动文件列表提供更多的内存, 这样我们就可以在内存中缓存更多经常使用的文件页, 而不会让它们被流式写入、一旦使用流式文件读取等页面驱逐出去. 这个补丁缩小了 inactive file list 的大小(之前是各占 50%), 不再允许在匿名内存使用的相同比率上增加活动列表, 这同样将有助于数据库工作负载(只有一半的页面缓存可用于缓存数据库 workingset). | v1 ☑✓ 4.7-rc1 | [LORE v1,0/3](https://lore.kernel.org/all/1459790018-6630-1-git-send-email-hannes@cmpxchg.org), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=59dc76b0d4dfdd7dc46a1010e4afb44f60f3e97f) |
| 2016/07/21 | Mel Gorman <mgorman@techsingularity.net><br>Minchan Kim <minchan@kernel.org> | [mm: consider per-zone inactive ratio to deactivate](https://lore.kernel.org/patchwork/patch/699874) | Joonsoo Kim 和 Minchan Kim 都报告了在 32 位平台上过早地触发了 OOM. 常见的问题是区域约束的高阶分配失败. 两个问题的原因都是因为: pgdat 被过早地认为是不可回收, 并且活动/非活动列表的轮换不足.<br>这组补丁做了如下部分:<br>1. 不考虑扫描时跳过的页面. 这避免了pgdat被过早地标记为不可回收<br>2. 补丁 2-4 增加了每个区域的统计数据.    如果每个区域的LRU计数是可用的, 那么就没有必要估计是否应该根据pgdat统计信息重试回收和压缩, 因此重新实现了近似的 pgdat 统计数据.<br>3. inactive_list_is_low() 中的主动去激活的逻辑存在问题, 导致 Minchan Kim 报告的问题, 有可能当前 zone 上明明有 8M 的匿名页面, 但是通过非原子的 order-0 分配却触发了 OOM, 因为该区域中的所有页面都在活动列表中. 在补丁 5 中, 如果所有符合条件的区域的非活动/活动比例需要更正, 那么一个区域受限的全局回收将会旋转列表. 较高的区域页面在开始时可能会被提前旋转, 但这是维护总体LRU年龄的更安全的选择. | v5 ☑ [4.8-rc1](https://kernelnewbies.org/Linux_4.8#Memory_management) | [Patchwork RFC](https://lore.kernel.org/patchwork/patch/699587)<br>*-*-*-*-*-*-*-*<br>[Patchwork v1](https://lore.kernel.org/patchwork/patch/699874)<br>*-*-*-*-*-*-*-*<br>[Patchwork v2](https://lore.kernel.org/patchwork/patch/700111)<br>*-*-*-*-*-*-*-*<br>[关注 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f8d1a31163fcb3bf93f6c78770c3c1915f065bf9) |
| 2017/01/16 | Michal Hocko <mhocko@suse.com> | [mm, vmscan: cleanup lru size claculations](https://lore.kernel.org/patchwork/patch/751448) | 使用 lruvec_lru_size() 精简了 inactive_list_is_low() 的流程. | v1 ☑ 4.11-rc1 | [PatchWork v1,1/3](https://lore.kernel.org/patchwork/patch/751448) |
| 2019/11/07 | Johannes Weiner <hannes@cmpxchg.org> | [mm: vmscan: enforce inactive:active ratio at the reclaim root](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b91ac374346ba206cfd568bb0ab830af6b205cfd) | [mm: fix page aging across multiple cgroups](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=b91ac374346ba206cfd568bb0ab830af6b205cfd) 的其中一个补丁, 解决多个 cgroup 之间协同存在的问题, 我们希望 VM 回收系统中最冷的页面. 但是现在 VM 却只能回收一个 cgroup 中的热页, 而其他 cgroup 中明明有更适合回收的冷缓存.<br>这个补丁将 active/inactive 的大小平衡决定移动到回收的根路径. 它通过递归地观察 cgroup 子树的 active 和 inactive 的长度, 然后决策是扫描或跳过活动页面. 这样, VM 就会意识到在工作负载 A 和 B 的组合缓存集中有大量的不活动页面, 并且会选择 B 中的一次性缓存, 而不是 A 中的热页面.<br>这个补丁将 inactive_list_is_low() 改名为 inactive_is_low().  | v1 ☑ [5.5-rc1](https://kernelnewbies.org/Linux_5.5#Memory_management) | [LORE 3/3](https://lore.kernel.org/all/20191107205334.158354-4-hannes@cmpxchg.org)<br>[关注 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b91ac374346ba206cfd568bb0ab830af6b205cfd) |

原始 inactive_file_is_low() 的机制非常机械, 不像 inactive_anon_is_low() 按照 inactive_ratio 比率来比较, 它认为 inactive/active  文件列表的应该保持缓存大小为 50%. 这种机械的判断, 引起了不少问题. [vmscan: do not force-scan file lru if its absolute size is small](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9ee11ba4251dddf1b0e507d184b25b1bd7820773) 就尝试通过检查 inactive FILE LRU 的大小和当前优先级下可扫描的页面数来解决问题.

自 [mm: vmscan: reduce size of inactive file list](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=59dc76b0d4dfdd7dc46a1010e4afb44f60f3e97f) 这个补丁之后, 匿名页和文件页 balancing 的判断采用了一套机制, inactive_anon_is_low() 和 inactive_file_is_low() 不用再区别对待, 一律使用 inactive_ratio 来进行比较. 因此这两个函数被移除, 整体都使用 inactive_list_is_low() 来处理. 随后 [mm: vmscan: enforce inactive:active ratio at the reclaim root](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b91ac374346ba206cfd568bb0ab830af6b205cfd) 在解决多个 cgroup 之间协同问题的时候将这个函数改名为 inactive_is_low().




2.  用 get_scan_count() 处理 anon/file reclaim balancing(Reclaim pressure balance between anon and file pages).


get_scan_count() 用于在 LRU 进行页面回收时, 确定各个 LRU list 上需要扫描的页面数.

早在 v2.5.43 [commit 8f7a14042e1f ("reduced and tunable swappiness")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8f7a14042e1f5fc814fa60956e3a2dcf5744a0ed) 就对此进行了探索, 引入 `/proc/sys/vm/swappiness` 控制 VM 取消页面映射(回收文件页)和交换内容(回收匿名页)的倾向.

随后 v2.6.25-rc1 引入 memcg LRU 的过程中, [commit ("per-zone and reclaim enhancements for memory controller: modifies vmscan.c for isolate globa/cgroup lru activity")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1cfb419b394ba82745c54ff05436d598ecc2dbd5) 将这个流程封装成了 calc_reclaim_mappe() 函数. [commit 58ae83db2a40 ("per-zone and reclaim enhancements for memory controller: calculate mapper_ratio per cgroup")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=58ae83db2a40dea15d4277d499a11dadc823c388) 引入了 mem_cgroup_calc_mapped_ratio() 对 MEMCG 中的回收倾向也做了处理. shrink_active_list()(前面已经提过了, 就是替代以前的内核版本的 refill_inactive_zone()) 函数中, 会通过 calc_reclaim_mappe() 判断是否需要回收文件页.

后来在 v2.6.28 [commit 4f98a2fee8ac ("vmscan: split LRU lists into anon & file sets")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4f98a2fee8acdb4ac84545df98cccecfd130f8db) 对 LRU 中的页面进一步针对是匿名页和文件页进行了区分, 引入了 get_scan_ratio() 替代 calc_reclaim_mappe() 来确定应以多大力度扫描 anon 和 file LRU 列表. 每一组 LRU 列表的相对值是通过查看扫描的页面的比例来确定的, 我们将其旋转回活动列表, 而不是逐出. 计算的结果放到 percent[2] 的数组中, 其中 percent[0] 指定对匿名页 LRU 施加多大压力, 而 percent[1] 则制定对文件 LRU 施加的压力.

随后在 v2.6.35-rc1 [commit 76a33fc380c9 ("vmscan: prevent get_scan_ratio() rounding errors")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=76a33fc380c9a65e01eb15b3b87c05863a0d51db) 这个补丁发现 get_scan_ratio() 计算百分比, 如果百分比 < 1%, 它将舍入到 0%, 这将导致我们完全忽略扫描匿名/文件页面来回收内存, 即使总匿名/文件页面非常大. 为了避免下溢, 我们不使用百分比, 而是直接计算应该扫描多少页. 通过这种方式, 我们应该获得几个 < 1% 的扫描页面. 这有一方面提高计算精度, 同时使扫描更流畅. 否则, 如果 percent[x] 为下流, 那么 shrink_zone() 不会扫描任何页面, 当优先级为 0 时, 它会突然扫描所有页面. 这样修改后, 即使优先级不为零, shrink_zone() 也有机会扫描一些页面. 因此将 get_scan_ratio() 修改为 get_scan_count(), 它不再返回匿名页和文件页的扫描比率 percent[2], 而是通过 nr_scan_try_batch() 返回各个 enum lru_list 上应该扫描的页面个数 nr[NR_LRU_LISTS]. 这个补丁并没有修改其他算法逻辑, 在提高精度的同时, 只是将 shrink_zone() 中把 percent[2] 转换为 nr[NR_LRU_LISTS] 的逻辑合并到了 get_scan_count() 中. 这里关于函数的返回值注释是错误的, 直到 v3.6-rc1 [commit be7bd59db71d ("mm: fix page reclaim comment error")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=be7bd59db71dfb6dc011c9880fec5a659430003a) 才修正了这个补丁引入的注释错误. 这个过程 get_scan_count() 中匿名页和文件页之间的回收压力平衡是通过分子和分母的元组 fraction[2] 和 denominator 标记来计算的, v3.9-rc1 [commit 9a2651140ef7 ("mm: vmscan: clean up get_scan_count()")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9a2651140ef740b3b67ad47ea3d0af75581aacc6) 引入了 scan_balance 来精简了标记, 使代码更容易阅读.




| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/12/11 | Rik van Riel <riel@redhat.com> | [mm,vmscan: only evict file pages when we have plenty](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e9868505987a03a26a3979f27b82911ccc003752) | TODO | v1 ☐☑✓ | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e9868505987a03a26a3979f27b82911ccc003752) |
| 2014/03/14 | Johannes Weiner <hannes@cmpxchg.org> | [mm: vmscan: do not swap anon pages just because free+file is low](https://lore.kernel.org/patchwork/patch/449613) | 当文件缓存下降到一个区域的高水位以下时, 页面回收强制扫描/交换匿名页面, 以防止残留的少量缓存发生抖动.<br>然而, 在较大的机器上, 高水印值可能相当大, 当工作负载由静态匿名/shmem集控制时, 文件集可能只是一个使用过一次的缓存的小窗口. 在这种情况下, 当本应该回收不再使用的缓存时, 虚拟机却开始大量交换.<br>要解决这个问题, 不要在文件页面较低时强制立即扫描, 而是依赖扫描/旋转比率来做出正确的预测. | v1 ☑ 2.6.33-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/179466) |


关于 force scan, 如果 get_scan_count() 中发现 LRU 中页面很少, 当前优先级下能被扫描的页面非常少的时候, 将启动强制扫描, 防止出现一页 ZONE 区域内因为 LRU 页面一直很少而导致永远无法回收.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2011/05/26 | KAMEZAWA Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com> | [memcg: fix get_scan_count() for small targets](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=246e87a9393448c20873bc5dee64be68ed559e24) | get_scan_count() 中通过 nr_scan_try_batch() 获取将每个区域扫描的页面数量, 这是根据 LRU 的大小和扫确定的 `scan = (anon + file) >> priority.`, 如果 scan < SWAP_CLUSTER_MAX, 这次扫描将被跳过, 并提高优先级. 但是这种策略存在很多问题. 这个补丁移除了 nr_saved_scan[NR_LRU_LISTS], 引入了 force_scan 机制. 如果发现当前优先级下能被扫描的页面很少(scan < SWAP_CLUSTER_MAX), 则但是处于 KSWAPD balancing 和 MEMECG reclaim 等路径, 则使能 force_scan, 强制扫描 SWAP_CLUSTER_MAX 个页面. 从而防止一些 ZONE 区域内因为 LRU 页面很少而无法被回收. | v1 ☑✓ 3.0-rc1 | [LORE](https://lore.kernel.org/lkml/20110427164708.1143395e.kamezawa.hiroyu@jp.fujitsu.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=246e87a9393448c20873bc5dee64be68ed559e24) |
| 2011/08/11 | Johannes Weiner <jweiner@redhat.com> | [mm: vmscan: fix force-scanning small targets without swap](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=185efc0f9a1f2d6ad6d4782c5d9e529f3290567f) | 如果系统中没有 SWAP, 匿名页面将不会被扫描. 因此, 当考虑强制扫描一个小目标时, 如果没有 SWAP, 它们不应该被计算在内. 否则, 即使目标的有效扫描数为 0, 且适用其他条件 kswapd/memcg, 也不会强制扫描目标. 因此 force_scan 机制中, 在最开始通过 `scan = (anon + file) >> priority` 就不是非常合适的, 后续处理 force_scan 的时候, 也有判断, 因此移除了开始的这些判断. | v1 ☑✓ v3.1-rc7 | [LORE v1,1/2](https://lore.kernel.org/all/1313094715-31187-1-git-send-email-jweiner@redhat.com), [COMMIT1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a4d3e9e76337059406fcf3ead288c0df22a790e9) |
| 2011/08/11 | Johannes Weiner <jweiner@redhat.com> | [mm: vmscan: drop nr_force_scan[] from get_scan_count](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f11c0ca501af89fc07b0d9f17531ba3b68a4ef39) | nr_force_scan[] 保存了匿名和文件页的有效扫描号, 以防需要强制扫描且定期计算的扫描号为零. 但是, 有效扫描数总是可以假定为 SWAP_CLUSTER_MAX, 就在将其划分为 anon 和 file 之前. 因此删除这个数组. | v1 ☑✓ v3.2-rc1 | [LORE v1,2/2](https://lore.kernel.org/all/1313094715-31187-1-git-send-email-jweiner@redhat.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f11c0ca501af89fc07b0d9f17531ba3b68a4ef39) |
| 2014/06/04 | Suleiman Souhlal <suleiman@google.com> | [mm: only force scan in reclaim when none of the LRUs arebig enough.](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6f04f48dc9c0433e2bb687f5f7f7af1aba97b04d) | 在此之前, 如果 LRU 大小本身对于当前优先级来说太小, 我们将决定是否在回收过程中[启用 force_scan 强制扫描 LRU].<br>然而, 这可能会导致文件 LRU 被强制扫描, 即使有很多匿名页面可以回收, 依旧导致了导致热文件页面被不必要地回收.<br>为了解决这个问题, 我们[只在所有可回收 lru 都不够大的时候强制扫描](https://elixir.bootlin.com/linux/v3.16/source/mm/vmscan.c#L2008). | v1 ☑✓ 3.16-rc1 | [LORE](https://lore.kernel.org/all/alpine.LSU.2.11.1403151957160.21388@eggly.anvils) |
| 2015/01/09 | Vladimir Davydov <vdavydov@parallels.com> | [vmscan: force scan offline memory cgroups](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=90cbc2508827e1e15dca23361c33cc26dd2b9e99) | commit b2052564e66d ("mm:memcontrol:continue cache Recall from offlined groups") 之后, 在删除 cgroup 时, 不会重新分配向内存 cgroup 收费的页面. 取而代之的是, 它们应该以常规的方式被回收, 以及计入在线内存 cgroup 的页面. 然而, 脱机内存 cgroup 的 lruvec 迟早会变得非常小, 以至于只能以较低的扫描优先级进行扫描(请参见 get_scan_count()). 因此, 如果大型 LRUVEC 中有足够多的可回收页面, 那么离线内存 cgroup 中的页面将永远不会被扫描, 从而浪费内存. 通过无条件地从 kswapd 强制扫描死掉的 LRUVEC 来修复此问题. | v2 ☑✓ 4.0-rc1 | [LORE](https://lore.kernel.org/all/1420790983-20270-1-git-send-email-vdavydov@parallels.com) |
| 2015/11/23 | Vladimir Davydov <vdavydov@virtuozzo.com> | [vmscan: do not force-scan file lru if its absolute size is small](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9ee11ba4251dddf1b0e507d184b25b1bd7820773) | 如果系统中有足够的 inactive page cache, 即 !inactive_file_is_low(), 或者说当前实现下就是 inactive FILE LRU 的大小大于 acrtive FILE LRU, 则 [get_scan_count() 中会强制扫描 FILE LRU 忽略 ANON LRU](https://elixir.bootlin.com/linux/v4.4/source/mm/vmscan.c#L2052), 当有大量的 Page Cache 时, 这种逻辑工作得很好. 但如果 FILE LRU 的大小很小(比如只有几 MB), 它就会失败. 在这种情况下 (lru_size >> prio) 趋近于 0(即使使用正常扫描优先级), 此时如果 inactive FILE LRU 的大小大于 acrtive FILE LRU, cgroup 的匿名页面也永远不会被驱逐, 即使好几G的未使用匿名内存, 除非系统经历严重的内存压力. 这对于其他 cgroup 来说是不公平的, 因为它们的工作负载可能是面向 Page Cache 的. 这个补丁试图通过详细 "足够的非活动页面缓存" 检查来修复这个问题: 它不仅检查 `!inactive_file_is_low()`, 还检查当前 cgroup 扫描优先级下能扫描的大小. 如果这些条件不成立, 继续如往常一样 SCAN_FRACT. | v2 ☑✓ 4.5-rc1 | [LORE](https://lore.kernel.org/all/1448275173-10538-1-git-send-email-vdavydov@virtuozzo.com)<br>*-*-*-*-*-*-*-*<br>[LORE](https://lore.kernel.org/all/1448275173-10538-1-git-send-email-vdavydov@virtuozzo.com), [关注 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=316bda0e6cc5f36f94b4af8bded16d642c90ad75) |
| 2017/02/28 | Johannes Weiner <hannes@cmpxchg.org> | [mm: kswapd spinning on unreclaimable nodes - fixes and cleanups](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=491d79ae778f24dbf65f1f2178d4744d940d4093) | Jia 报告了一个场景, 在这个场景中, 一个节点的 kswapd 占用 CPU 100% 并且无限循环. 当前内核当前判断其回收节点的能力(或是否退出并休眠) 的方法是基于扫描的页面数量与可回收的页面数量成比例. 然而, 在 Jia 的场景中, 节点中没有可回收的页面, 并且永远不会满足后退的条件. 这组补丁提供了不基于扫描, 而是基于 kswapd 在 MAX_RECLAIM_RETRIES(16) 连续运行中是否能够实际回收页面, 重新定义了一个不可回收的节点. 这是页面分配器用于放弃直接回收和调用 OOM 杀手的相同标准. 如果它不能释放任何页面, kswapd 将进入休眠状态, 并留下进一步的直接回收调用尝试, 这将取得进展并重新启用 kswapd, 或者调用 OOM 杀死器.<br>补丁 1 修复了 Jia 所报告的即时问题, 剩下的是更小的补丁, 清理, 以及对旧方法的全面淘汰.<br>补丁 5/6 是一个例外. 清理了 get_scan_count(). | v1 ☑✓ 4.12-rc1 | [LORE v1,0/9](https://lore.kernel.org/all/20170228214007.5621-1-hannes@cmpxchg.org) |

[commit 246E87A934 ("memcg: fix get_scan_count() for small targets")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=246e87a9393448c20873bc5dee64be68ed559e24) 试图避免 memcg 的高回收优先级, 方法是在 LRU 中页面较少, 在当前优先级下不会回收任何页面时, 因此引入了 force_scan 机制, 强制其扫描至少扫描 SWAP_CLUSTER_MAX 个页面. 这是在回收决策与优先级挂钩的时候完成的.

但是内核经历了多年的发展之后, 唯一有意义的事情仍然与优先级降到 [`DEF_PRIORITY - 2`](https://elixir.bootlin.com/linux/v4.11/source/mm/vmscan.c#L2797) 以下有关, 那就是设置 [`laptop_mode = 1`](https://elixir.bootlin.com/linux/v4.11/source/kernel/sysctl.c#L1461) 是否通常允许写入. 但在那个时代, 直接回收仍然允许调用 `->writepage()`, 而内核发展至今 kswapd 在扫描系统中的[每个干净页面之前](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=515f4a037fb9ab736f8bad733fcd2ffd350cf265)都会避免写入. `sc->may_writepage` 即使触发的过于频繁也不会有太大的问题. 因此 [commit ("mm: don't avoid high-priority reclaim on memcg limit reclaim")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=688035f729dcd9a98152c827338805a061f5c6fa) 删除了 force_scan 的内容, 以及它所需要的丑陋的多遍目标计算.


#### 4.2.8.3 Refault Distance 算法
-------

如果进一步思考, 这个问题跟工作集大小相关. 所谓工作集, 就是维持系统所有活动的所需内存页面的最小量. 如果工作集小于等于 inactive 链表长度, 即访问距离, 则是安全的; 如果工作集大于 inactive 链表长度, 即访问距离, 则不可避免有些页要被踢出去.


[14.7 跟踪LRU活动情况和 Refault Distance算法](https://blog.csdn.net/dai_xiangjun/article/details/118945704)

Johannes Weiner 认为这种经验公式过于简单且不够灵活, 为此他提出了一个替代方案 [Refault Distance 算法](https://lwn.net/Articles/495543). 该算法希望通过跟踪一个页框从被回收开始到(因为访问缺页)被再次载入所经历的时间长度来采取更灵活的平衡策略. 它通过估算访问距离, 来测定工作集的大小, 从而维持 inactive 链表在一个合适长度. 最早 v3.15 合入时, 只针对页面高速缓存类型的页面生效, [mm: thrash detection-based file cache sizing v9](https://lwn.net/Articles/495543). 随后被不断优化. 参见 [Better active/inactive list balancing](https://lwn.net/Articles/495543).

最开始, Refault Distance 算法只支持对文件高速缓存进行评估. [mm: thrash detection-based file cache sizing v9](https://lwn.net/Articles/495543).

后来 linux 5.9 引入了对匿名页的支持. [workingset protection/detection on the anonymous LRU list](https://lwn.net/Articles/815342).

下面简单了解下 Refault Distance 算法的基本思想:


对于 LRU 链表来说, 有两个链表值得关注, 分别是活跃链表和不活跃链表.

1.  页面回收也总是从不活跃链表的尾部开始回收. 不活跃链表的页面第二次访问时会升级(promote)到活跃链表, 防止被回收;

2.  另一方面如果活跃链表增长太快, 那么活跃的页面会被降级(demote)到不活跃链表中.



实际上有一些场景, 某些页面经常被访问, 但是它们在下一次被访问之前就在不活跃链表中被回收并释放了, 那么又必须从存储系统中读取这些page cache页面, 这些场景下产生颠簸现象(thrashing).

1.  有一个 page cache 页面第一次访问时, 它加入到不活跃链表头, 然后慢慢从链表头向链表尾移动, 链表尾的 page cache 会被踢出 LRU 链表并且释放页面, 这个过程叫做 eviction(驱逐/移出).

2.  当第二次访问时, page cache 被升级到活跃 LRU 链表, 这样不活跃链表也空出一个位置, 这个过程叫作 activation(激活).


当我们观察文件缓存不活跃链表的行为特征时, 会发现如下有趣特征:

1. 任意两个时间点之间的驱逐和激活的总和表示在这两个时间点之间访问的不活跃页面的最小数量.

2. 将一个非活跃列表中槽位为 N 位置的页面(移向列表尾部并)释放需要至少N个非活动页面访问.

结合这些:

从宏观时间轴来看, eviction 过程处理的页面数量与 activation 过程处理的页数量的和等于不活跃链表的长度 NR_INACTIVE. 即要将新加入不活跃链表的页面释放, 需要移动 NR_INACTIVE 个页面.

记 page cache 页面第一次被踢出 LRU 链表并回收时 eviction(驱逐)和 activation(激活) 的页面总数为(E), 记第二次再访问该页时 eviction(驱逐)和 activation(激活) 的页面总数为(R), 通过这两个计数可得出页面未缓存时的最小访问次数. 将这两个时刻内里需要移动的页面个数称为 refault_distance, 则 refault_distance = (R - E).

综合上面的一些行为特征, 定义了 Refault Distance 的概念. 第一次访问 page cache 称为 fault, 第二次访问该页面称为 refault. 第一次和第二次读之间需要移动的页面个数称为 read_distance,

    read_distance =  页面从加入 LRU 到释放所需要移动的页面数量 + 页面从释放到第二次访问所需要移动的页面数量 = NR_INACTIVE + refault_distance = NR_INACTIVE + (R - E)

如果 page 想一直保持在 LRU 链表中, 那么 read_distance 不应该比内存的大小还长, 否则该 page 永远都会被踢出 LR U链表, 因此公式可以推导为:

    NR_INACTIVE + (R-E) <= NR_INACTIVE + NR_active
    (R-E) <= NR_active

换句话说, Refault Distance 可以理解为不活跃链表的"财政赤字", 如果不活跃链表长度至少再延长 Refault Distance, 那么就可以保证该 page cache 在第二次读之前不会被踢出 LRU 链表并释放内存, 否则就要把该 page cache 重新加入活跃链表加以保护, 以防内存颠簸. 在理想情况下, page cache 的平均访问距离要大于不活跃链表, 小于总的内存大小.

上述内容讨论了两次读的距离小于等于内存大小的情况, 即 NR_INACTIVE+(R-E) <= NR_INACTIVE+NR_active, 如果两次读的距离大于内存大小呢?

这种特殊情况不是 Refault Distance 算法能解决的问题, 因为它在第二次读时永远已经被踢出 LRU 链表, 因为可以假设第二次读发生在遥远的未来, 但谁都无法保证它在 LRU 链表中.

Refault Distance 算法是为了解决前者, 在第二次读时, 人为地把 page cache 添加到活跃链表从而防止该 page cache 被踢出 LRU 链表而带来的内存颠簸.


#### 4.2.8.4 Refault Distance 基本实现
-------

[refault_distance 实现思路](./images/0002-3-refault_distance.png)


如上图, T0时刻表示一个page cache第一次访问, 这时会调用 add_to_page_cache_lru() 函数来分配一个 shadow 用来存储 zone->inactive_age 值, 每当有页面被 promote 到活跃链表时, zone->inactive_age 值会加 1, 每当有页面被踢出不活跃链表时, zone->inactive_age 也会加 1. T1时刻表示该页被踢出LRU链表并从LRU链表中回收释放, 这时把当前T1时刻的zone->inactive_age的值编码存放到shadow中. T2时刻是该页第二次读, 这时要计算 Refault Distance, Refault Distance = T2 - T1, 如果Refault Distance <= NR_active, 说明该page cache极有可能在下一次读时已经被踢出LRU链表, 因此要人为地actived该页面并且加入活跃链表中.

| 操作 | 流程 | Page Cache(v3.15 支持) | 匿名页(v5.9 支持) |
|:---:|:----:|:---------------------:|:----------------:|
| zone->inactive_age | 1. 在 struct zone 数据结构中新增一个 [inactive_age](https://elixir.bootlin.com/linux/v3.15/source/include/linux/mmzone.h#L399) 原子变量成员, 用于记录(v3.15 只涉及文件缓存)不活跃链表中的 eviction 操作和 activation 操作的计数.<br>*-*-*-*-*-*-*-*<br>2.  page cache 第一次加入不活跃链表的 LRU 时, 在处理 radix tree 时会[分配一个 slot 来存放 inactive_age](https://elixir.bootlin.com/linux/v3.15/source/mm/filemap.c#L626), 这里使用 shadow 指向 slot. 因此第一次加入时 shadow 值为空, 还没有 Refault Distance, 因此[加入不活跃 LRU 链表](https://elixir.bootlin.com/linux/v3.15/source/mm/filemap.c#L640). | NA | NA |
| workingset_eviction() | 在不活跃链表末尾的页面会被踢出LRU链表并被释放, 在被踢出LRU链表时, 通过 [workingset_eviction()](https://elixir.bootlin.com/linux/v3.15/source/mm/workingset.c#L213) 把当前的 zone->inactive_age 计数保存到该页对应的 radix_tree 的 shadow 中. | 高速缓存页面 [`__remove_mapping`, v3.15](https://elixir.bootlin.com/linux/v3.15/source/mm/vmscan.c#L526) 流程中释放页面时调用. | [`__remove_mapping`, v5.9](https://elixir.bootlin.com/linux/v5.9/source/mm/vmscan.c#L901) 流程中对于 PageSwapCache(page) 释放时调用 |
| workingset_activation() | 当前 zone 中 INACTIVE LRU 中的页面在被激活到 ACTIVE LRU 时调用, 会调用 workingset_activation() 更新(增加) zone->inactive_age 的计数 | 1. 当在文件缓存不活跃链表(LRU_INACTIVE_FILE)里的页面被再一次读取时, 会被激活到活跃 LRU 链表, 这时候会通过 mark_page_accessed(page) -=> workingset_activation(page) 更新(增加) zone->inactive_age 的计数.<br>*-*-*-*-*-*-*-*<br>2. 当 page cache 第二次读取时, 在 [add_to_page_cache_lru()](https://elixir.bootlin.com/linux/v3.15/source/mm/filemap.c#L636) 中, 首先会通过 [workingset_refault()](https://elixir.bootlin.com/linux/v3.15/source/mm/workingset.c#L231) -=> [unpack_shadow](https://elixir.bootlin.com/linux/v3.15/source/mm/workingset.c#L164) 计算 Refault Distance, 并且判断是否需要把 page cache 加入到活跃链表中, 以避免下一次读之前被踢出 LRU 链表. 如果发现需要将页面激活, 则调用 [workingset_activation()](https://elixir.bootlin.com/linux/v3.15/source/mm/filemap.c#L638). 其中 unpack_shadow() 函数只把该页面之前存放的 shadow 值重新解码, 得出了图中 T1 时刻的 inactive_age 值, 然后把当前的 inactive_age 减去 T1, 得到 Refault Distance.  | 同高速缓存页面 1, mark_page_accessed(page) 激活页面时调用. |
| workingset_refault() | 计算页面的 Refault Distance, 并判断是否需要把页面加入到活跃 LRU 链表中. | add_to_page_cache_lru() 中激活高速缓存页面的时候, 通过 workingset_refault() 计算 Refault Distance. | 1. 匿名页面缺页处理时, 如果页面 pte 不为 0, 表示此时 pte 的内容所对应的页面在 swap 空间中, 缺页异常时会通过 do_swap_page() 来分配页面. 在 do_swap_page() 中 get_shadow_from_swap_cache() 获取到页面后, 通过 [workingset_refault()](https://elixir.bootlin.com/linux/v5.9/source/mm/memory.c#L3298) 判断页面是否需要加到活跃 LRU.<br>*-*-*-*-*-*-*-*<br>2.  `__read_swap_cache_async()` 预读 SWAP 页面的时候时, 同样需要通过 [workingset_refault()](https://elixir.bootlin.com/linux/v5.9/source/mm/swap_state.c#L503) 判定要将页面加入到哪个 LRU 链表中去. |
| workingset_init() | [449dd6984d0e mm: keep page cache radix tree nodes in check](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=449dd6984d0e47643c04c807f609dd56d48d5bcc) 引入, 3.15 之前, 页面缓存基数树节点在回收它们的页面指针后会被释放. 但是现在, 将 shadow 存储在它们的位置上, 只有在索引节点本身被回收时才会回收 shadow. 这对于在回收了大量缓存后仍然在使用的较大文件来说是有问题的, 这些文件中的任何页面实际上都没有再 refault. shadow 将只是停留在那里并浪费内存. 在最坏的情况下, shadow 会不断累积, 直到机器耗尽内存.<br>为了控制这一点, 跟踪每个 numa 节点列表中只包含 shadow 的基数树节点. 使用每个 numa 而不是全局的, 是因为希望基数树节点本身是在本地分配的, 减少对其他独立缓存工作负载的跨节点引用. 引入一个简单的[收缩器 workingset_shadow_shrinker](https://elixir.bootlin.com/linux/v3.15/source/mm/workingset.c#L385) 将回收这些节点上的内存压力.  | NA | NA |




#### 4.2.8.5 Refault Distance 的演进与发展
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2014/04/03 | Johannes Weiner <hannes@cmpxchg.org> | [mm: thrash detection-based file cache sizing v9](https://lwn.net/Articles/495543) | 实现基于 Refault Distance 算法的文件高速缓存页的工作集探测, 引入 WORKINGSET_REFAULT, WORKINGSET_ACTIVATE, WORKINGSET_NODERECLAIM 三种工作集. | v9 ☑ [3.15](https://kernelnewbies.org/Linux_3.15#head-dbe2430cd9e5ed1d3f2362367758cd490aba4b9d) | [PatchWork v9](https://lore.kernel.org/patchwork/patch/437949) |
| 2016/01/24 | Vladimir Davydov <vdavydov@parallels.com>/<vdavydov@virtuozzo.com> | [Make workingset detection logic memcg aware](https://lwn.net/Articles/586023) | 工作集探测感知 memcg.<br>目前, 工作集检测逻辑不是 memcg 感知的 - inactive_age 是每个 zone 域维护的. 因此, 如果使用了内存组, 则会随机激活错误的文件页. 这个补丁集使每个 lruvec 具有 inactive_age, 以便工作集检测将正确地工作在内存 cgroup 回收. | v2 ☐ | [PatchWork 0/3](https://lore.kernel.org/patchwork/patch/586023)<br>*-*-*-*-*-*-*-*<br>[2016/01/24 PatchWork v2](https://lore.kernel.org/patchwork/patch/638421/) |
| 2016/01/29 | Johannes Weiner <hannes@cmpxchg.org> | [mm: workingset: per-cgroup thrash detection](https://lore.kernel.org/patchwork/patch/641469) | 这组补丁使工作集检测具备 cgroup 感知的能力, 因此当使用 cgroup 内存控制器时, 页面回收可以正常工作.<br>缓存抖动检测目前仅在系统级别工作, 而不在 cgroup 层次工作. 更糟糕的是, 由于将 refaults 与 active LRU 的全局数量相比较, 当 cgroup 的页面比其他组的页面更热时, 它们可能会错误地激活所有 refaults 的页面.<br>1. 将 refault 的统计 inactive_age 从区域移动到 lruvec.<br>2. 然后用 memcg ID 标记逐出条目. | v2 ☑ [4.6-rc1](https://kernelnewbies.org/Linux_4.20#Memory_management) | [2016/01/26 PatchWork 0/5](https://lore.kernel.org/patchwork/patch/639567)<br>*-*-*-*-*-*-*-*<br>[2016/01/29 PatchWork v2,0/5](https://lore.kernel.org/patchwork/patch/641469) |
| 2016/02/09 | Vladimir Davydov <vdavydov@virtuozzo.com> | [mm: workingset: make shadow node shrinker memcg aware](https://lwn.net/Articles/586023) | 工作集探测已经被 memcg 识别, 但阴影节点收缩器仍然是全局的. 因此, 一个小 cgroup 可能会消耗阴影节点的所有可用内存, 通过回收某个 cgroup 的阴影节点可能会损害其他 cgroup, 即使回收这些阴影节点中存储的距离没有任何效果. 为了避免这种情况,  我们需要使阴影节点收缩器也变成 memcg 感知的.<br>实际工作在本系列的补丁6中完成. 修补程序1和2为 memcg/shrinker 基础架构的更改做好准备. 补丁3只是一个附带的清理. 补丁4使基数树节点占位, 这是使阴影节点收缩器 memcg 感知所必需的. 补丁5在工作负载主要使用匿名页面的情况下减少了阴影节点的开销. | v2 ☑ 4.6-rc1 | [2016/02/07 PatchWork 0/5](https://lore.kernel.org/patchwork/patch/644646)<br>*-*-*-*-*-*-*-*<br>[2016/02/09 PatchWork v2,0/6](https://lore.kernel.org/patchwork/patch/645395) |
| 2016/04/04 | Johannes Weiner <hannes@cmpxchg.org> | [mm: support bigger cache workingsets and protect against writes](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=59dc76b0d4dfdd7dc46a1010e4afb44f60f3e97f) | [Unhelpful caching decisions, possibly related to active/inactive sizing](https://lore.kernel.org/lkml/20160209165240.th5bx4adkyewnrf3@alap3.anarazel.de) Andres  报告说, 他的数据库测试上一些频繁写入且从来读过的的文件页占满了 active FILE LRU, 导致一些经常被读取的页面被驱逐. 因此经过讨论<br>1. 首先只有存在频繁读取的的页面被 active 才是有意义的, 除了写回缓存对部分页面写入所起的作用外, 只写入的页面不会从缓存中受益, 因此我们不应该将它们提升到活动文件列表中, 在活动文件列表中, 它们与缓存数据实际上被频繁访问的页面会存在竞争. 一种情况是用于[缓存内写访问](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bbddabe2e436aa7869b3ac5248df5c14ddde0cbf), 另一个用于[由写触发的 refaults](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f0281a00fe80f0e689dd51e68c3aed5f6ef1bf58), 这两种情况下都不应该 active page cache 的页面.<br>2. 通过 refault 检测, 可以不再需要[非活动文件列表的最小文件缓存大小为 50%](https://elixir.bootlin.com/linux/v4.6/source/mm/vmscan.c#L1929), 因为我们完全可以通过检测识别出频繁使用的页面(即使它们在访问之间被逐出), 从而[允许 active list 更大](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=59dc76b0d4dfdd7dc46a1010e4afb44f60f3e97f), 从而保护更大的 workingset.可以在内存中缓存更多经常读取的文件页面, 这些页面也不会被流写入、一次使用的流文件读取等方式将它们驱逐出去, 从而使得数据库等场景在稳定阶段获得更好的性能. | v1 ☑ [4.7-rc1](https://kernelnewbies.org/Linux_4.7#Memory_management) | [LORE v1](https://lore.kernel.org/lkml/1459790018-6630-1-git-send-email-hannes@cmpxchg.org) |
| 2016/06/24 | Johannes Weiner <hannes@cmpxchg.org> | [mm: fix vm-scalability regression in cgroup-aware workingset code](https://lore.kernel.org/patchwork/patch/692559) | commit 23047a96d7cf ("mm:workingset:per cgroup cache thrash detection") 在缓存逐出 workingset_evictio()、探测 workingset_refault() 和激活 workingset_activation() 路径中添加了 page->mem_cgroup 查找, 并[锁定到激活路径](https://elixir.bootlin.com/linux/v4.6/source/mm/workingset.c#L310), vm可伸缩性测试显示了-23%的回归.<br>虽然所讨论的测试是一种在实际工作负载中不会发生的人为最坏情况场景(并行读取两个稀疏文件, 只是为了敲碎 LRU 路径), 但仍然可以在这些路径中进行一些优化.<br>1.
内联查找函数以消除调用.<br>此外, 在计算激活次数时, 不需要持续 hold page->mem_cgroup, 我们只需要[保持 RCU 锁以防止 memcg 被释放](https://elixir.bootlin.com/linux/v4.8/source/mm/workingset.c#L316). | v1 ☑ 4.8-rc1 | [2016/06/22 PatchWork v1](https://lore.kernel.org/patchwork/patch/691734)<br>*-*-*-*-*-*-*-*<br>[2016/06/24 PatchWork rebase](https://lore.kernel.org/patchwork/patch/692559), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=55779ec759ccc3c12b917b3712a7716e1140c652) |
| 2016/07/08 | Mel Gorman <mgorman@techsingularity.net> | [Move LRU page reclaim from zones to nodes v9](https://lore.kernel.org/patchwork/patch/696408) | 将 LRU 页面的回收从 ZONE 切换到 NODE. 这里需要将 workingset 从 zone 切换到 node 上. | v9 ☑ [4.8](https://kernelnewbies.org/Linux_4.8#Memory_management) | [PatchWork v9](https://lore.kernel.org/patchwork/patch/696408), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1e6b10857f91685c60c341703ece4ae9bb775cf3) |
| 2018/08/28 | Johannes Weiner <hannes@cmpxchg.org> | [psi: pressure stall information for CPU, memory, and IO v4](https://lore.kernel.org/patchwork/patch/978495) | 发生 refault 的页面可能是 inactive, 也可能是 active 的.<br>1. 对于前者, 如果非活动缓存以合适的重构距离重构, 称为缓存**工作集转换**, 并将对当前活动列表施加压力.<br>2. 对于后者, 如果 refault 的页面最近并没有访问, 则意味着活动列表不再保护活动使用的缓存免于回收. 但是缓存不会转换到不同的工作设置, 现有的工作设置在分配给页面缓存的空间中受到冲击. 这种我们称为**原地抖动**.<br>为了有效地区分两种场景, 引入一个新的页面标志, 标记被驱逐的页面在其一生中是否处于活动状态. 然后将此位存储在阴影条目中, 以将重构分类为转换 WORKINGSET_ACTIVATE 或抖动 WORKINGSET_RESTORE.. | v1 ☑ [4.20-rc1](https://kernelnewbies.org/Linux_4.20#Memory_management) | [PatchWork](https://lore.kernel.org/patchwork/patch/978495), [关注 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1899ad18c6072d689896badafb81267b0a1092a4) |
| 2018/10/09 | Johannes Weiner <hannes@cmpxchg.org> | [mm: workingset & shrinker fixes](https://lore.kernel.org/patchwork/patch/997829) | 通过为循环中的影子节点添加一个计数器, 可以更容易地捕获影子节点收缩器中的 bug. | v1 ☑ [4.20-rc1](https://kernelnewbies.org/Linux_4.20#Memory_management) | [PatchWork](https://lore.kernel.org/patchwork/patch/997829), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=68d48e6a2df575b935edd420396c3cb8b6aa6ad3) |
| 2019/07/01 | Johannes Weiner <hannes@cmpxchg.org> | [mm: vmscan: scan anonymous pages on file refaults](https://lore.kernel.org/patchwork/patch/1095936) | NA | v1 ☑ 5.3-rc1 | [PatchWork v1](https://lore.kernel.org/patchwork/patch/1090850)<br>*-*-*-*-*-*-*-*<br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/1095351)<br>*-*-*-*-*-*-*-*<br>[PatchWork](https://lore.kernel.org/patchwork/patch/1095936), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2c012a4ad1a2cd3fb5a0f9307b9d219f84eda1fa) |
| 2019/10/22 | Johannes Weiner <hannes@cmpxchg.org> | [mm/vmscan: cgroup-related cleanups](https://lore.kernel.org/patchwork/patch/1142997) | 这里的 8 个补丁, 清理回收代码与cgroups的交互. 它们不应该改变任何行为, 只是让实现更容易理解和使用. | v1 ☑ 5.5-rc1 | [PatchWork 0/8](https://lore.kernel.org/patchwork/patch/1142997) |
| 2019/11/07 | Johannes Weiner <hannes@cmpxchg.org> | [mm: fix page aging across multiple cgroups](https://lore.kernel.org/patchwork/patch/1150211) | 我们希望VM回收系统中最冷的页面. 但是现在VM可以在一个cgroup中回收热页, 而在其他cgroup中明明有更适合回收的冷缓存. 这是因为回收算法的一部分(非活动/活动列表的平衡, 这是用来保护热缓存数据不受一次性流IO影响的部分.)并不是真正意识到 cgroup 层次结构的.<br>递归 cgroup 回收方案将以相同的速率以循环方式扫描和旋转每个符合条件的 cgroup 的物理 LRU 列表, 从而在所有这些cgroup的页面之间建立一个相对顺序. 然而, 非活动/活动的平衡决策是在每个cgroup内部本地做出的, 所以当一个cgroup冷页面运行不足时, 它的热页面将被回收——即使在相同的回收运行中, 但是同级的 cgroup 中却可能有足够的冷缓存.<br>这组补丁通过将非活动/活动平衡决策提升到回收运行的顶层来修复这个问题. 这要么是一个cgroup达到了它的极限, 要么是直接的全局回收, 如果有物理内存压力. 从那里, 它采用了一个cgroup子树的递归视图来决定是否有必要取消页面. | v1 ☑ [5.5-rc1](https://kernelnewbies.org/Linux_5.5#Memory_management) | [PatchWork](https://lore.kernel.org/patchwork/patch/1150211) |
| 2020/04/03 | Joonsoo Kim <iamjoonsoo.kim@lge.com> | [workingset protection/detection on the anonymous LRU list](https://lwn.net/Articles/815342) | 实现对匿名 LRU 页面列表的工作集保护和检测. 在之前的实现中, 新创建的或交换中的匿名页, 都是默认加入到 active LRU list, 然后逐渐降级到 inactive LRU list. 这造成在某种场景下新申请的内存(即使被使用一次cold page)也会把在a ctive list 的 hot page 挤到 inactive list. 为了解决这个的问题, 这组补丁, 将新创建或交换的匿名页面放到 inactive LRU list 中, 只有当它们被足够引用时才会被提升到活动列表. 另外,  因为这些更改可能导致新创建的匿名页面或交换中的匿名页面交换不活动列表中的现有页面, 所以工作集检测被扩展到处理匿名LRU列表. 以做出更优的决策. | v5 ☑ [5.9-rc1](https://kernelnewbies.org/Linux_5.9#Memory_management) | [PatchWork v5](https://lore.kernel.org/patchwork/patch/1219942), [Patchwork v7](https://lore.kernel.org/patchwork/patch/1278082), [ZhiHu](https://zhuanlan.zhihu.com/p/113220105) |
| 2020/05/20 | Johannes Weiner <hannes@cmpxchg.org> | [mm: balance LRU lists based on relative thrashing v2](https://lore.kernel.org/patchwork/patch/1245255) | 基于相对抖动平衡 LRU 列表(重新实现了页面缓存和匿名页面之间的 LRU 平衡, 以便更好地与快速随机 IO 交换设备一起工作). : 在交换和缓存回收之间平衡的回收代码试图仅基于内存引用模式预测可能的重用. 随着时间的推移, 平衡代码已经被调优到一个点, 即它主要用于页面缓存, 并推迟交换, 直到 VM 处于显著的内存压力之下. 因为 commit a528910e12ec Linux 有精确的故障 IO 跟踪-回收错误页面的最终代价. 这允许我们使用基于 IO 成本的平衡模型, 当缓存发生抖动时, 这种模型更积极地扫描匿名内存, 同时能够避免不必要的交换风暴. | v1 ☑ [5.8-rc1](https://kernelnewbies.org/Linux_5.8#Memory_management) | [PatchWork v1](https://lore.kernel.org/patchwork/patch/685701)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/lkml/20200520232525.798933-1-hannes@cmpxchg.org) |




### 4.2.9 LRU 的优化
-------


[KSWAPD 和页面回收的行为工作的一直不尽如意](https://lore.kernel.org/all/1368432760-21573-1-git-send-email-mgorman@suse.de), 一直有一些或多或少的 BUG. 比如大量的拷贝或者备份操作导致机器卡顿或应用程序的匿名页面被 SWAP 出去. 有时在内存不足的情况下, 会突然回收大量内存. 有时, 当应用程序启动时, KSWAPD 在很长一段时间内达到 100% 的 CPU 使用率. 因此一部分人在讨论引入一些特性, 比如一个额外的空闲 kbytes 优化(an extra free kbytes tunable), 以解决问题的各个方面, 而不是试图解决问题. 它的工作负载非常大, 而且是特定于机器的, 这会使问题变得更加复杂.


[Reduce system disruption due to kswapd V4](https://lore.kernel.org/all/1368432760-21573-1-git-send-email-mgorman@suse.de) 旨在解决其中一些最糟糕的问题, 而不试图从根本上改变页面回收的工作方式. 我们就从这个 patchset 了解下 LRU 框架和整体脉络.


#### 4.2.9.1 限制回收的数量
-------

补丁 1-2 限制了 KSWAPD 回收的页面数量, 同时仍然遵守 LRU ANON/FILE Balancing 的约定比例.

之前内核并不限制回收的页面数量, nr_to_reclaim() 中直接设置 nr_to_reclaim 为 ULONG_MAX 的. 这样的效果就是造成回收 "尖峰", 很大一部分内存突然被释放. [补丁 1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=75485363ce8552698bfb9970d901f755d5713cca) 引入了 kswapd_shrink_zone()(后期改名为 kswapd_shrink_node()) 将 nr_to_reclaim [限制为内存的高水线](https://elixir.bootlin.com/linux/v3.11/source/mm/vmscan.c#L2786), 并触发内存回收. 注意这并不是一个硬限制. 由于这种回收方式, 打破了 LRU ANON/FILE Balancing 约定的比例, 因此[补丁 2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e82e0561dae9f3ae5a21fc2d3d3ccbe69d90be46) 在 shrink_lruvec() 中引入了 scan_adjusted 根据已经扫描的页面个数, 动态地调整待扫描的页面数量.

#### 4.2.9.2 扫描优先级
-------

补丁 3-4 控制 KSWAPD 如何以及何时提高其扫描优先级, 并删除难以遵循的扫描重启逻辑.

而 DEF_PRIORITY 等扫描优先级的历史远比我们想象的要早, 早在 linux 0.9x 的年代, try_to_free_page() 就引入了 6 个优先级. 参见 [COMMIT1, 0.97.1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit?id=ce823b448e7d9186ca696cd7edbd4ba84e05ab71), [COMMIT2, 0.97.3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit?id=fdb2f0a59a1c76a7ee7b9478fae06f76bc697152), [COMMIT2, v2.0](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/diff/mm/vmscan.c?id=e2ba60b6e7071dd89c80ceb006e42479ec615baa) kswapd_free_pages().

随后 [v2.4.0-test9pre1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/diff/mm/vmscan.c?id=1fc53b2209b58e786c102e55ee682c12ffb4c794) 引入 LRU 的时候实现了 6 个优先级. 接着 [commit 3192b2dcbe00 ("VM balancing tuning")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/diff/mm/vmscan.c?id=3192b2dcbe00fdfd6a50be32c8c626cf26b66076) 优化了 LRU 的算法, 并定义了 DEF_PRIORITY.

[low-latency page reclaim](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log?id=407ee6c87e477434e3cb8be96885ed27b5539b6f) 进一步延伸出 12 个优先级.

而[补丁 3 优化 balance_pgdat()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b8e83b942a16eb73e63406592d3178207a4f07a1), 只要 kswapd_shrink_zone() 只要[扫描了足够多的页面](https://elixir.bootlin.com/linux/v3.11/source/mm/vmscan.c#L2844), 就[不再提高优先级](https://elixir.bootlin.com/linux/v3.11/source/mm/vmscan.c#L3006)继续扫描. 为了避免高阶分配请求的无限循环, 当 KSWAPD 已经回收的页面数量至少是待分配请求的两倍时, 它将[不会回收高阶分配](https://elixir.bootlin.com/linux/v3.11/source/mm/vmscan.c#L3026). 之前 KSWAPD 会在 pgdat 被认为是平衡的之后[决定是否规整内存](https://elixir.bootlin.com/linux/v3.10/source/mm/vmscan.c#L2861). 但做出这样的决定已经为时已晚, 而且现在 [KSWAPD 根据回收进度决定是否退出区域扫描循环](https://elixir.bootlin.com/linux/v3.11/source/mm/vmscan.c#L3047), 内存规整在这个位置已经不太合适. 因此[补丁 4 将内存规整的动作放到平衡地过程中进行](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2ab44f434586b8ccb11f781b4c2730492e6628f5), 如果至少从不平衡区域回收了指定优先级的请求页数, 则通过 compact_pgdat() 对[当前 pgdat 进行内存规整](https://elixir.bootlin.com/linux/v3.11/source/mm/vmscan.c#L3037). 并且只要当前 pgdat 中有任何区域当前处于平衡状态, 都[不会触发内存规整](https://elixir.bootlin.com/linux/v3.11/source/mm/vmscan.c#L2958), 因为已经有足够的页面可以使用了.

但是进一步发现 balance_pgdat() 在平衡地过程中, 优先级很容易达到 0. 优先级 0 被认为是一个接近 OOM 的条件, 他会扫描整个 LRU. 如果遇到大量无法回收的页面(比如回写中的页面). 这就造成, 明明没有分配失败或 OOM 的实际风险, KSWAPD 依然轻松地抵达了 0 有限就, 并且积极地回收大量页面. 因此[补丁 5 阻止 balance_pgdat() 达到 0 优先级](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9aa41348a8d11427feec350b21dcdd4330fd20c4). 在 OOM 情况下, 直接回收器的优先级仍然为 0. 因此这并不会有什么问题.

#### 4.2.9.3 回收过程中的脏页
-------

之前, 如果 KSWAPD 扫描的优先级提高到一定程度时将对脏页队列进行回写, 但其实 KSWAPD 扫描的优先级与遇到的未排队脏页的数量无关, 而是与 LRU 的大小和区域水线有关, 这并没有指示 KSWAPD 是否应该写页面. 如果在 LRU 末端遇到过多的未排队脏页, [补丁 6](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d43006d503ac921c7df4f94d13c17db6f13c9d26) 通过 [nr_unqueued_dirty](https://elixir.bootlin.com/linux/v3.11/source/mm/vmscan.c#L771) 跟踪这些脏页. 如果在刷新线程清理脏页之前, 有足够多的脏页正在被回收, 就[标记该区域为 ZONE_TAIL_LRU_DIRTY](https://elixir.bootlin.com/linux/v3.11/source/mm/vmscan.c#L1473), 以便 KSWAPD 将开始写页面, 直到该区域达到平衡.


#### 4.2.9.4 回收过程中节流(Reclaim Throttle)
-------

有时候 KSWAPD 会等待 IO 完成, 以减少回收干净页面或驱逐应用程序热匿名页的可能性. 很早之前, 如果回收仍然没有成效, KSWAPD 通常会在尝试更高的优先级前使用 [KSWAPD_SKIP_CONGESTION_WAIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f50de2d3811081957156b5d736778799379c29de) 和 [congestion_wait()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=92df3a723f84cdf8133560bbff950a7a99e92bc9) 等待 IO, 从而进行节流.

这其实没有任何意义, 因为回收的失败可能完全独立于 IO. congestion_wait() 后来[被 wait_iff_congested() 所取代](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=92df3a723f84cdf8133560bbff950a7a99e92bc9), 随后 [KSWAPD_SKIP_CONGESTION_WAIT 被完全删除](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=258401a60c4df39332f30ef57afbc6dbf29a7e84).

但是这样依旧有问题. wait_iff_congested() 并不适合 KSWAPD, 如果直接回收或 KSWAPD 遇到太多的回写页面, [补丁 7](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=283aba9f9e0e4882bf09bd37a2983379a6fae805) 设置了一个 ZONE_WRITEBACK 标志. 如果设置了这个标志, KSWAPD 在写回时遇到了一个 PageReclaim() 页面, 那么它会假设 LRU 列表在 IO 完成之前被回收得太快, 并阻塞等待某个 IO 完成.

随后 [Remove dependency on congestion_wait in mm/](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=66ce520bb7c22848bfdf3180d7e760a066dbcfbe) 引入了[回收节流(Reclaim Throttle)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8cd7c588decf470bf7e14f2be93b709f839a965e)替代了 congestion_wait(). 实现了 3 种不同的类型取代了 "拥塞" 节流.

1.  如果有太多脏页写回页, 则睡眠直到超时或清理足够多的页.

2.  如果隔离了太多的页, 则睡眠直到回收足够多的隔离页或将其放回 LRU.

3.  如果依旧没有进展, 请直接回收任务睡眠, 直到另一个任务以可接受的效率取得进展.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2013/05/13 | Mel Gorman <mgorman@suse.de> | [Reduce system disruption due to kswapd](https://lore.kernel.org/all/1363525456-10448-1-git-send-email-mgorman@suse.de) | 1363525456-10448-1-git-send-email-mgorman@suse.de | v1 ☑✓ 3.11-rc1 | [LORE v1,0/8](https://lore.kernel.org/all/1363525456-10448-1-git-send-email-mgorman@suse.de)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/9](https://lore.kernel.org/all/1368432760-21573-1-git-send-email-mgorman@suse.de), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e82e0561dae9f3ae5a21fc2d3d3ccbe69d90be46) |
| 2019/10/22 | Johannes Weiner <hannes@cmpxchg.org> | [mm: vmscan: harmonize writeback congestion tracking for nodes & memcgs](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1b05117df78e035afb5f66ef50bf8750d976ef08) | 清理回收代码与cgroups的交互 [mm/vmscan: cgroup-related cleanups](https://lore.kernel.org/patchwork/patch/1142997) 的其中一个补丁. | v1 ☑ 5.5-rc1 | [PatchWork 0/8](https://lore.kernel.org/patchwork/patch/1142997) |
| 2021/10/22 | Mel Gorman <mgorman@techsingularity.net> | [Remove dependency on congestion_wait in mm/](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=66ce520bb7c22848bfdf3180d7e760a066dbcfbe) | NA | v5 ☑✓ 5.16-rc1 | [LORE v5,0/8](https://lore.kernel.org/all/20211022144651.19914-1-mgorman@techsingularity.net)|



#### 4.2.9.5 LRU 中的内存水线
-------

**2.6.28(2008年12月)**

4.1 中描述过一个用户可配置的接口 : **_swappiness_**. 这是一个百分比数(取值 0-100, 默认60), 当值越靠近100, 表示更倾向替换匿名页; 当值越靠近0, 表示更倾向替换文件缓存页. 这在不同的工作负载下允许管理员手动配置.


4.1 中 规则 4)中说在需要换页时, MM 会从 active 链表尾开始扫描, 如果有一台机器的 **_swappiness_** 被设置为 0, 意为完全不替换匿名页, 则 MM 在扫描中要跳过很多匿名页, 如果有很多匿名页(在一台把 **_swappiness_** 设为0的机器上很可能是这样的), 则容易出现性能问题.


解决方法就是把链表拆分为匿名页链表和文件缓存页链表, 现在 active 链表分为 active 匿名页链表 和 active 文件缓存页链表了; inactive 链表也如此.  所以, 当 **_swappiness_** 为0时, 只要扫描 active 文件缓存页链表就够了.

#### 4.2.9.6 LRU 中的其他优化
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2010/06/29 | Mel Gorman <mel@csn.ul.ie> | [Avoid overflowing of stack during page reclaim V3](https://lore.kernel.org/patchwork/patch/204944) | NA | v3 ☐ | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/685701)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3](https://lore.kernel.org/patchwork/patch/204944) |
| 2010/09/06 | Mel Gorman <mel@csn.ul.ie> | [Reduce latencies and improve overall reclaim efficiency](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=08fc468f4eaf6683bae5bdb94743a09d8630cb80) | 成块回收过于激进, 会在 LRU 系统造成一定的破坏. 由于 SLUB 使用高阶分配, 块状回收产生的巨大成本将是显而易见的. 这些补丁应该可以在不禁用 Lumpy Reclaim 的情况下缓解该问题. 引入 lumpy_mode, 减少成块回收过程中的等待和延迟. | v2 ☑✓ 2.6.37-rc1 | [LORE v1,0/9](https://lore.kernel.org/all/1283770053-18833-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/8](https://lore.kernel.org/lkml/1284553671-31574-1-git-send-email-mel@csn.ul.ie) |
| 2010/10/28 | Mel Gorman <mel@csn.ul.ie> | [Reduce the amount of time spent in watermark-related functions V4](https://lore.kernel.org/patchwork/patch/222014) | NA | v4 ☐ | [PatchWork v4](https://lore.kernel.org/patchwork/patch/222014) |
| 2010/07/30 | Mel Gorman <mel@csn.ul.ie> | [Reduce writeback from page reclaim context V6](https://lore.kernel.org/patchwork/patch/209074) | NA | v2 ☐ | [PatchWork v2](https://lore.kernel.org/patchwork/patch/209074) |
| 2021/12/20 | Muchun Song <songmuchun@bytedance.com> | [Optimize list lru memory consumption](https://lore.kernel.org/patchwork/patch/1436887) | 优化列表lru内存消耗<br> | v3 ☐ | [2021/05/27 PatchWork v2,00/21](https://patchwork.kernel.org/project/linux-mm/cover/20210527062148.9361-1-songmuchun@bytedance.com)<br>*-*-*-*-*-*-*-* <br>[2021/09/14 PatchWork v3,00/76](https://patchwork.kernel.org/project/linux-mm/cover/20210914072938.6440-1-songmuchun@bytedance.com)<br>*-*-*-*-*-*-*-* <br>[2021/12/13 PatchWork v4,00/17](https://patchwork.kernel.org/project/linux-mm/cover/20211213165342.74704-1-songmuchun@bytedance.com)<br>*-*-*-*-*-*-*-* <br>[2021/12/20 PatchWork v5,00/16](https://patchwork.kernel.org/project/linux-mm/cover/20211220085649.8196-1-songmuchun@bytedance.com)<br>*-*-*-*-*-*-*-* <br>[LORE v6,0/16](https://lore.kernel.org/r/20220228122126.37293-1-songmuchun@bytedance.com) |




### 4.2.11 LRU 的整体框架
-------



#### 4.2.11.1 LRU 回收框架
-------

shrink_lruvec() 和 shrink_slab() 是 LRU 处理的几个最基础函数.


```cpp
shrink_lruvec()
    -=> get_scan_count()                                    # 探测各个 LRU 中需要扫描的页面个数 nr[NR_LRU_LISTS]
    -=> for_each_evictable_lru(lru) -=> shrink_list()        # 回收各个 LRU 中的页面, 至少扫描 min(nr[lru], SWAP_CLUSTER_MAX) 个.
    -=> shrink_active_list()                                # 必要时对 active/inactive 进行均衡
```

首先是 KSWAPD 路径:

```cpp
balance_pgdat()
-=> mem_cgroup_soft_limit_reclaim() // do while (sc.priority >= 1);
    -=> mem_cgroup_soft_reclaim
        -=> mem_cgroup_shrink_node()
            -=> shrink_lruvec()
-=> kswapd_shrink_node()
    -=> shrink_node()
        -=> shrink_node_memcgs()
            -=> shrink_lruvec()
            -=> shrink_slab()
```

其次是快速本地回收的路径:


```
__alloc_page()
get_page_from_freelist()
    -=> node_reclaim()
        -=> __node_reclaim()
            -=> shrink_node()
                -=> shrink_node_memcgs()
                    -=> shrink_lruvec()
                    -=> shrink_slab()
```


接着是直接回收路径:


```cpp
__alloc_pages()
__alloc_pages_slowpath()
__alloc_pages_direct_reclaim()
-=> __perform_reclaim()
    -=> try_to_free_pages()
        -=> do_try_to_free_pages()
            -=> shrink_zones()
                -=> for_each_zone_zonelist_nodemask -=> mem_cgroup_soft_limit_reclaim()
                    -=> mem_cgroup_soft_reclaim()
                        -=> mem_cgroup_shrink_node()
                            -=> shrink_lruvec()
                -=> shrink_node()
                    -=> shrink_node_memcgs()
                        -=> shrink_lruvec()
                        -=> shrink_slab()
```



#### 4.2.11.2 shrink_list
-------


#### 4.2.11.3 shrink_active_list
-------


#### 4.2.11.4 shrink_inactive_list
-------



### 4.2.12 其他页面替换算法
-------

[AdvancedPageReplacement](https://linux-mm.org/AdvancedPageReplacement)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2005/08/08 | Rik van Riel <riel@redhat.com> | [non-resident page tracking](https://lore.kernel.org/patchwork/patch/41749) | 这些补丁实现了非驻留页面跟踪, 这是高级页面替换算法(如CART和CLOCK-Pro)所需的基础功能. | RFC ☐ | [PatchWork RFC,0/3](https://lore.kernel.org/patchwork/patch/41749) |
| 2005/08/10 | Rik van Riel <riel@redhat.com> | [CLOCK-Pro page replacement](https://lore.kernel.org/patchwork/patch/41832) | CLOCK-Pr 页面替换是一种实现, 以保持那些页面在活动列表中引用"最频繁, 最近", 即, 后面两个引用之间距离最小的页面. | RFT ☐ | [PatchWork RFT,0/5](https://lore.kernel.org/patchwork/patch/41832) |



## 4.3 shrinker
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2010/11/09 | Glauber Costa <glommer@openvz.org> | [vmscan: per-node deferred work](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1d3d4437eae1bb2963faab427f65f90663c64aa1) | 实现 per node 的内存回收. | v1 ☑ 3.19-rc1 | [LORE](https://lore.kernel.org/lkml/20101109123246.GA11477@amd), [LORE v9,00/17](https://lore.kernel.org/all/153112469064.4097.2581798353485457328.stgit@localhost.localdomain), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1d3d4437eae1bb2963faab427f65f90663c64aa1) |
| 2010/11/09 | Nick Piggin <npiggin@kernel.dk> | [mm: vmscan implement per-zone shrinkers](https://lore.kernel.org/all/153063036670.1818.16010062622751502.stgit@localhost.localdomain/) | 实现 per zone 的内存回收. | v1 ☑ 3.19-rc1 | [LORE](https://lore.kernel.org/lkml/20101109123246.GA11477@amd), [LORE v9,00/17](https://lore.kernel.org/all/153112469064.4097.2581798353485457328.stgit@localhost.localdomain), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6b4f7799c6a5703ac6b8c0649f4c22f00fa07513) |
| 2012/11/28 | Dave Chinner <david@fromorbit.com> | [Numa aware LRU lists and shrinkers](https://lore.kernel.org/all/1354058086-27937-1-git-send-email-david@fromorbit.com) | 实现 SHRINKER_NUMA_AWARE | v1 ☐☑✓ | [LORE v1,0/19](https://lore.kernel.org/all/1354058086-27937-1-git-send-email-david@fromorbit.com) |
| 2022/04/22 | Roman Gushchin <roman.gushchin@linux.dev> | [mm: introduce shrinker debugfs interface](https://patchwork.kernel.org/project/linux-mm/cover/20220416002756.4087977-1-roman.gushchin@linux.dev/) | 内核中有 50 多个不同的 shrinker, 在内存压力下, 内核会按照它们在系统中创建/注册的顺序对它们施加一些压力. 其中一些只能包含很少的对象, 一些可以相当大. 有些可以有效地回收内存, 有些则不然. 但是当前却没有有效的方法对单个 shrinker 进行计数和扫描, 并对其进行分析. 现有的唯一调试机制是 do_shrink_slab() 中的两个 TRACEPOINT: mm_shrink_slab_start 和 mm_shrink_slab_end. 不过, 它们并没有涵盖所有内容: 报告 0 个对象的 shrinker永远不会出现, 也不支持支持支持 MEMCG 的 shrinker.  shrinker 通过其扫描功能进行识别, 但这并不总是足够的. 为了为内存 shrinker 提供更好的可见性和调试选项, 这个补丁集引入了 `/sys/kernel/shrinker` 接口, 在某种程度上类似于 `/sys/kernel/slab`. 为系统中注册的每个 shrinker 创建一个目录, 该目录包含两个文件结点 "count" 和 "scan" , 允许触发 count_objects() 和 scan_objects() 回调. 对于 MEMCG-Aware 和 NUMA-Aware 的 shrinker, 还额外提供了 count_memcg、scan_memcg、count_node、scan_node、count_memcg_node 和 scan_memcg_node. 它们允许获取每个 MEMCG 和每个 NUMA NODE 的对象计数, 并仅收缩特定的 MEMCG 或者 NUMA NODE. 为了使调试更加愉快, 补丁集还为所有 shrinker 命名, 以便 sysfs 条目可以有更多有意义的名称. | v2 ☐☑ | [LORE v1,0/5](https://lore.kernel.org/r/20220416002756.4087977-1-roman.gushchin@linux.dev)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/5](https://lore.kernel.org/all/20220422015853.748291-1-roman.gushchin@linux.dev)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/7](https://lore.kernel.org/r/20220422202644.799732-1-roman.gushchin@linux.dev)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/6](https://lore.kernel.org/r/20220509183820.573666-1-roman.gushchin@linux.dev)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/6](https://lore.kernel.org/r/20220601032227.4076670-1-roman.gushchin@linux.dev) |
| 2022/04/21 | Kent Overstreet <kent.overstreet@gmail.com> | [Printbufs & shrinker OOM reporting](https://patchwork.kernel.org/project/linux-mm/cover/20220419203202.2670193-1-kent.overstreet@gmail.com) | 633514 | v1 ☐☑ | [LORE v1,0/4](https://lore.kernel.org/all/20220419203202.2670193-1-kent.overstreet@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/4](https://lore.kernel.org/all/20220421234837.3629927-1-kent.overstreet@gmail.com) |


### 4.3.1 shrinker 的引入
-------

早期各个模块的回收和释放各自为政.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/02/04 | Linus Torvalds <torvalds@athlon.transmeta.com> | [v2.4.0.1 -> v2.4.0.2 VM balancing tuning](https://github.com/gatieme/linux-history/commit/3192b2dcbe00fdfd6a50be32c8c626cf26b66076) | NA | v1 ☑ 2.4.0.2 | [PATCH HISTORY](https://github.com/gatieme/linux-history/commit/3192b2dcbe00fdfd6a50be32c8c626cf26b66076) |
| 2002/02/04 | Linus Torvalds <torvalds@athlon.transmeta.com> | [v2.4.4.6 -> v2.4.5 md graceful alloc failure](https://github.com/gatieme/linux-history/commit/9c6f70be049f5a0439996107e58bf65e9a5d9a09) | NA | v1 ☑ 2.4.8.4 | [PATCH HISTORY](https://github.com/gatieme/linux-history/commit/9c6f70be049f5a0439996107e58bf65e9a5d9a09) |
| 2002/02/04 | Linus Torvalds <torvalds@athlon.transmeta.com> | [v2.4.7.3 -> v2.4.7.4](https://github.com/gatieme/linux-history/commit/70d68bd32041d22febb277038641d55c6ac7b57a) | NA | v1 ☑ 2.4.8.4 | [PATCH HISTORY](https://github.com/gatieme/linux-history/commit/70d68bd32041d22febb277038641d55c6ac7b57a) |
| 2002/02/04 | Linus Torvalds <torvalds@athlon.transmeta.com> | [v2.4.8.3 -> v2.4.8.4](https://github.com/gatieme/linux-history/commit/0b9ded43ee424791d9283cee2a33dcb4a97da57d) | NA | v1 ☑ 2.4.8.4 | [PATCH HISTORY](https://github.com/gatieme/linux-history/commit/0b9ded43ee424791d9283cee2a33dcb4a97da57d) |
| 2002/02/04 | Linus Torvalds <torvalds@athlon.transmeta.com> | [v2.4.9.10 -> v2.4.9.11](https://github.com/gatieme/linux-history/commit/a880f45a48be2956d2c78a839c472287d54435c1) | NA | v1 ☑ 2.4.13.6 | [PATCH HISTORY](https://github.com/gatieme/linux-history/commit/a880f45a48be2956d2c78a839c472287d54435c1) |
| 2002/02/04 | Linus Torvalds <torvalds@athlon.transmeta.com> | [v2.4.13.3 -> v2.4.13.4](https://github.com/gatieme/linux-history/commit/f97f22cb0b9dca2a797feccc580b3b8fdb238ac3) | NA | v1 ☑ 2.4.13.6 | [PATCH HISTORY](https://github.com/gatieme/linux-history/commit/f97f22cb0b9dca2a797feccc580b3b8fdb238ac3) |
| 2002/02/04 | Andrew Morton <akpm@digeo.com> | [v2.4.13.5 -> v2.4.13.6 me: shrink dcache/icache more aggressively](https://github.com/gatieme/linux-history/commit/857805c6bdc445542e35347e18f1f0d21353a52c) | NA | v1 ☑ 2.4.13.6 | [PATCH HISTORY](https://github.com/gatieme/linux-history/commit/857805c6bdc445542e35347e18f1f0d21353a52c) |
| 2002/09/22 | Andrew Morton <akpm@digeo.com> | [low-latency page reclaim](https://github.com/gatieme/linux-history/commit/407ee6c87e477434e3cb8be96885ed27b5539b6f) | NA | v1 ☑ 2.5.39 | [PATCH HISTORY](https://github.com/gatieme/linux-history/commit/407ee6c87e477434e3cb8be96885ed27b5539b6f) |
| 2002/09/25 | Andrew Morton <akpm@digeo.com> | [slab reclaim balancing](https://github.com/gatieme/linux-history/commit/b65bbded3935b896d55cb6b3e420a085d3089368) | NA | v1 ☑ 2.5.39 | [PATCH HISTORY](https://github.com/gatieme/linux-history/commit/b65bbded3935b896d55cb6b3e420a085d3089368) |
| 2002/10/04 | Andrew Morton <akpm@digeo.com> | [separation of direct-reclaim and kswapd function](https://github.com/gatieme/linux-history/commit/bf3f607a57d27cab30d5ddfd203d873856bc22b7) | NA | v1 ☑ 2.5.41 | [PATCH HISTORY](https://github.com/gatieme/linux-history/commit/bf3f607a57d27cab30d5ddfd203d873856bc22b7) |

v2.5 的时候引入了 shrink 机制, 并提供了 API 统一了各个模块的内存回收的接口.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/10/13 | Andrew Morton <akpm@digeo.com> | [batched slab shrink and registration API](https://github.com/gatieme/linux-history/commit/71419dc7e039a8953861df2a28fad639d12ae6b9) | 目前系统中存在 dentry、inode 和 dquot 缓存等[多个零散的收缩器](https://elixir.bootlin.com/linux/v2.5.42/source/mm/vmscan.c). 虽然可以正常工作, 但它的 cpu 效率略低. 这个补丁重写了这些收缩器. 引入了 shrinker 框架, 各个子系统可以通过 set_shrinker() 注册自己的 shrink 回调到收缩器. 这些收缩器由一个 shrinker_list 管理. 这些收缩器收缩的速度是一样的, 你们可以用相同的速率对他们进行回收. | v1 ☑ 2.5.43 | [PATCH HISTORY](https://github.com/gatieme/linux-history/commit/71419dc7e039a8953861df2a28fad639d12ae6b9) |

### 4.3.2 shrink_slab
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/07/03 | Kirill Tkhai <ktkhai@virtuozzo.com> | [Improve shrink_slab() scalability (old complexity was O(n^2), new is O(n))](https://lore.kernel.org/all/153063036670.1818.16010062622751502.stgit@localhost.localdomain/) | 降低 shrink_slab 的算法复杂度, 从 O(N^2) 降低到 O(N). | v8 ☑ 4.19-rc1 | [LORE v8,00/17](https://lore.kernel.org/all/153063036670.1818.16010062622751502.stgit@localhost.localdomain), [LORE v9,00/17](https://lore.kernel.org/all/153112469064.4097.2581798353485457328.stgit@localhost.localdomain) |
| 2018/08/07 | Kirill Tkhai <ktkhai@virtuozzo.com> | [ Introduce lockless shrink_slab()](https://patchwork.kernel.org/project/linux-mm/cover/153365347929.19074.12509495712735843805.stgit@localhost.localdomain) | NA | RFC ☐ | [LORE v8,00/17](https://patchwork.kernel.org/project/linux-mm/cover/153365347929.19074.12509495712735843805.stgit@localhost.localdomain) |
| 2022/04/02 | Hillf Danton <hdanton@sina.com> | [[RFC] mm/vmscan: add periodic slab shrinker](https://patchwork.kernel.org/project/linux-mm/patch/20220402072103.5140-1-hdanton@sina.com/) | 当一个具有大量内存的系统在一个目录中有数百万个 negative dentries 时, 在添加 inotify watch 时可能[会发生 softlookup](https://lore.kernel.org/linux-fsdevel/20220209231406.187668-1-stephen.s.brennan@oracle.com). 为了解决这个问题可以添加独立于直接回收和后台回收运行的周期性(periodic) slab shrinker, 以回收已冷却 30 秒以上的 slab 对象. 向 shrink 控件添加 periodic flag, 让缓存所有者知道这是一个周期性收缩器, 它与以最低 recalim 优先级运行的常规 shrinker 相同, 并且可以在没有一次性对象的情况下随意执行任何操作. | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220402072103.5140-1-hdanton@sina.com) |


## 4.4 主动的页面回收(Proactive Reclaim)
-------


1.  冷热页区分:  为了能识别那些可以回收的页面, 必须对那些不常用的页面有效地进行跟踪, 即 idle page tracking.

2.  进程内存的 working set size(WSS) 估计: 为了在回收了内存之后还能满足业务的需求, 保障业务性能不下降, 需要能预测出业务运行所需要的实际最小内存. brendangregg 大神对此也有描述, [Working Set Size Estimation](https://www.brendangregg.com/wss.html), 并设计了 wss 工具 [Working Set Size (WSS) Tools for Linux](https://github.com/brendangregg/wss).

Meta(原 Facebook) 开发了 [Senpai](https://github.com/facebookincubator/senpai)

### 4.4.1 Idle and stale page tracking
-------


[系统软件工程师必备技能-进程内存的working set size(WSS)测量](https://blog.csdn.net/juS3Ve/article/details/85333717)

[Idle and stale page tracking](https://lwn.net/Articles/461461)

首先是 Google 的方案, 这个特性很好的诠释了上面量两部分的内容, 参见 [V2: idle page tracking / working set estimation](https://lore.kernel.org/patchwork/patch/268228). 其主要实现思路如下:

1.  Google 最开始实现的方案是内核跟踪空闲页面的信息, 并暴露到新增的一个 procfs 的 bitmap 节点 `/sys/kernel/mm/page_idle/bitmap`,  然后 user-space 进程会频繁读取该 sysfs. 参见 [idle memory tracking](https://lore.kernel.org/lkml/cover.1437303956.git.vdavydov@parallels.com), 不过这个方案CPU占用率偏高, 并且内存浪费的也不少.

2.  所以目前 Google 尝试在此基础上进行优化, 形成一个新方案, 基于一个名为 [kstaled 的 kernel thread](https://lkml.org/lkml/2011/9/16/368), 参见 [V2, 0/9: idle page tracking / working set estimation](https://lore.kernel.org/lkml/1317170947-17074-1-git-send-email-walken@google.com). 这个 kernel thread 会利用 page 里的 flag 来跟踪 idle page, 所以不会再浪费更多系统内存了, 不过仍然需要挺多 CPU 时间的. 还有一个新加的 kreclaimd 线程来扫描内存, 回收那些空闲(没人读写访问)太久的页面. CPU 的开销并不小, 会随着需要跟踪的内存空间大小, 以及扫描的频率而线性增加. 在一个 512GB 内存的系统上, 可能会需要一个CPU完全用于做这部分工作. 大多数的时间都是在遍历 reverse-map 列表来查找 page mapping. 他们试过把 reverse-map 的查找去掉, 而是创建一个 PMD page table 链表, 可以改善这部分开销. 这样能减少 CPU 占用率到原来的 2/7. 还有另一个优化是把 kreclaimd 的扫描去掉而直接利用 kstaled 传入的一组页面, 也有明显的效果.

基于 kstaled/kreclaimd 的方案, 没有合入主线, 但是 idle memory tracking 的方案在优化后, 于 4.3 合入了主线, 命名为 [IDLE_PAGE_TRACKING](https://www.kernel.org/doc/html/latest/admin-guide/mm/idle_page_tracking.html). 作者基于这个特性进程运行所需的实际内存预测(WSS), 并提供了一[系列工具 idle_page_tracking](https://github.com/sjp38/idle_page_tracking)来完成这个工作, 参见 [Idle Page Tracking Tools](https://sjp38.github.io/post/idle_page_tracking).

Facebook 指出他们也面临过同样的问题, 所有的 workload 都需要放到 container 里去执行, 用户需要明确申明需要使用多少内存, 不过其实没人知道自己真的会用到多少内存, 因此用户申请的内存数量都太多了, 也就有了类似的overcommit和reclaim问题. Facebook的方案是采用 [PSI(pressure-stall information)](https://lwn.net/Articles/759781), 根据这个来了解内存是否变得很紧张了, 相应的会把LRU list里最久未用的page砍掉. 假如这个举动导致更多的 refault 发生. 不过通过调整内存的回收就调整的激进程度可以缓和 refault. 从而达到较合理的结果, 同时占用的CPU时间也会小得多. 描述其设计方案的论文在 ASPLOS’22 中发表, 并成为四篇 Best Paper 之一, 参见 [TMO: Transparent Memory Offloading in Datacenters](https://dl.acm.org/doi/10.1145/3503222.3507731). 网络上的分析 [知乎, 大荒落 TMO: Transparent Memory Offloading in Datacenters](https://zhuanlan.zhihu.com/p/477786756).

参见:

phoronix 报道 [Meta's Transparent Memory Offloading Saves Them 20~32% Of Memory Per Linux Server](https://www.phoronix.com/scan.php?page=news_item&px=Meta-Transparent-TMO).

| 日期 | LWN | 翻译 |
|:---:|:----:|:---:|
| 2022/06/20 | [Transparent memory offloading: more memory at a fraction of the cost and power](https://engineering.fb.com/2022/06/20/data-infrastructure/transparent-memory-offloading-more-memory-at-a-fraction-of-the-cost-and-power/) | [公众号-SegmentFault 思否--Meta “透明内存卸载” 功能亮相：可为 Linux 服务器节省 20%-32% 内存](https://mp.weixin.qq.com/s/J9hXRtEE-1x9a-I_hGwNPQ) |

Meta(原 Facebook) 博客 [Transparent memory offloading: more memory at a fraction of the cost and power](https://engineering.fb.com/2022/06/20/data-infrastructure/transparent-memory-offloading-more-memory-at-a-fraction-of-the-cost-and-power).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2011/09/28 | Rik van Riel <riel@redhat.com> | [V2: idle page tracking / working set estimation](https://lore.kernel.org/patchwork/patch/268228) | Google 的 kstaled 方案, 通过跟踪那些长期和回收未使用页面, 来减少内存使用, 同时不降低业务的性能. | v2 ☐ | [LORE v1,0/8](https://lore.kernel.org/all/1316230753-8693-1-git-send-email-walken@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/9](https://lore.kernel.org/lkml/1317170947-17074-1-git-send-email-walken@google.com) |
| 2015/07/19 | Vladimir Davydov <vdavydov@parallels.com> | [idle memory tracking](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d3691d2c6d3e72624c987bbef6f322631bbb2d5d) | Google 的 idle page 跟踪技术, CONFIG_IDLE_PAGE_TRACKING 跟踪长期未使用的页面. | v9 ☑ 4.3-rc1 | [PatchWork RFC]https://lore.kernel.org/lkml/cover.1437303956.git.vdavydov@parallels.com), [REDHAT Merge](https://lists.openvz.org/pipermail/devel/2015-October/067103.html), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=33c3fc71c8cfa3cc3a98beaa901c069c177dc295) |

### 4.4.2 [Proactively reclaiming idle memory](https://lwn.net/Articles/787611)
-------

[LWN: 主动回收较少使用的内存页面](https://blog.csdn.net/Linux_Everything/article/details/96416633)

[[RFC,-V6,0/6] NUMA balancing: optimize memory placement for memory tiering system](https://lore.kernel.org/patchwork/patch/1393431)

[Software-defined far memory in warehouse scale computers](https://blog.acolyer.org/2019/05/22/sw-far-memory)

[LSF/MM 2019](https://lwn.net/Articles/lsfmm2019) 期间, 主动回收 IDLE 页面的议题引起了开发者的关注. 通过对业务持续一段时间的页面使用进行监测, 回收掉那些不常用的或者没必要的页面, 在满足业务需求的前提下, 可以节省大量的内存. 这可能比启发式的 kswapd 更有效. 这包括两部分的内容:


后来还有一些类似的特性也达到了很好的效果.

1.  intel 在对 NVDIMM/PMEM 进行支持的时候, 为了将热页面尽量使用快速的内存设备, 而冷页面尽量使用慢速的内存设备. 因此实现了冷热页跟踪机制. 完善了 idle page tracking 功能, 实现 per process 的粒度上跟踪内存的冷热. 在 reclaim 时将冷的匿名页面迁移到 PMEM 上(只能迁移匿名页). 同时利用一个 userspace 的 daemon 和 idle page tracking, 来将热内存(在PMEM上的)迁移到 DRA M中. [ept-idle](https://github.com/intel/memory-optimizer/tree/master/kernel_module).

2.  openEuler 实现的 etmem 内存分级扩展技术, 通过 DRAM + 内存压缩/高性能存储新介质形成多级内存存储, 对内存数据进行分级, 将分级后的内存冷数据从内存介质迁移到高性能存储介质中, 达到内存容量扩展的目的, 从而实现内存成本下降.

3.  Amazon 的开发人员 SeongJae Park 基于 DAMON 分析内存的访问, 在此基础上实现了[主动的内存回收机制 DAMON-based Reclamation](https://damonitor.github.io/doc/html/v29-darc-rfc-v2/admin-guide/mm/damon/reclaim.html). 使用 DAMON 监控数据访问, 以找出特定时间段内未访问的冷页, 优先将他们回收.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/02/09 | David Rientjes <rientjes@google.com> | [smaps: extract pmd walker from smaps code](https://lore.kernel.org/patchwork/patch/73779) | Referenced Page flag, 通过在 /proc/PID/smap 中新增 pages referenced 计数的支持, 同时引入 `/proc/PID/clear_refs` 允许用户重置进程页面的 referenced 标记. 通过这种方式用户可以找出特定时间段内被访问过的页. 从而估计出进程的 WSS, 区分冷热页. | v1 ☑ 2.6.22-rc1 | [PatchWork v1 0/3](https://lore.kernel.org/patchwork/patch/73777) |
| 2007/10/09 | Matt Mackall <mpm@selenic.com> | [maps4: pagemap monitoring v4](https://lore.kernel.org/patchwork/patch/95279) | 引入 CONFIG_PROC_PAGE_MONITOR, 管理了 `/proc/pid/clear_refs`, `/proc/pid/smaps`, `/proc/pid/pagemap`, `/proc/kpagecount`, `/proc/kpageflags` 多个接口. | v1 ☑ 2.6.25-rc1 | [PatchWork v4 0/12](https://lore.kernel.org/patchwork/patch/95279) |
| 2018/12/26 | Fengguang Wu <fengguang.wu@intel.com> | [PMEM NUMA node and hotness accounting/migration](https://lore.kernel.org/patchwork/patch/1027864) | 尝试使用 NVDIMM/PMEM 作为易失性 NUMA 内存, 使其对普通应用程序和虚拟内存透明. 其中引入 `/proc/PID/idle_pages` 接口, 用于用户空间驱动的热点页面进行统计. 实现冷热页扫描, 以及页面回收路径下的被动内核冷页面迁移, 改进了用于活动用户空间热/冷页面迁移的move_pages() | RFC v2 ☐ 4.20 | PatchWork RFC,v2,00/21](https://lore.kernel.org/patchwork/patch/1027864), [LKML](https://lkml.org/lkml/2018/12/26/138), [github/intel/memory-optimizer](http://github.com/intel/memory-optimizer) |
| 2021/03/18 | liubo <liubo254@huawei.com> | [etmem: swap and scan](https://gitee.com/openeuler/kernel/issues/I3W4XW) | openEuler 实现的内存分级扩展技术. | v1 ☐ 4.19 | [etmem tools](https://gitee.com/src-openeuler/etmem) |
| 2021/10/19 | SeongJae Park <sjpark@amazon.com> | [Introduce DAMON-based Proactive Reclamation](https://lwn.net/Articles/863753) | 该补丁集改进了用于生产质量的通用数据访问模式内存管理的引擎, 并在其之上实现了主动回收. | v4 ☑ 5.16-rc1 | [PatchWork RFC,00/13](https://patchwork.kernel.org/project/linux-mm/cover/20210720131309.22073-1-sj38.park@gmail.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC,v2,00/14](https://patchwork.kernel.org/project/linux-mm/patch/20210608115254.11930-15-sj38.park@gmail.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC,v3,00/15](https://patchwork.kernel.org/project/linux-mm/cover/20210720131309.22073-1-sj38.park@gmail.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v4 00/15](https://patchwork.kernel.org/project/linux-mm/cover/20211019150731.16699-1-sj@kernel.org) |



### 4.4.3 提供用户态触发主动回收的接口(syscall or sysfs)
-------

*   process_mrelease


华为终端手机上实现了一个名为 [xreclaimer(Fast process memory reclaimer)](https://github.com/gatieme/MobileModels/blob/huawei/noh-mate40/mm/xreclaimer) 快速回收进程内存的方案, 在进程退出时快速回收.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/09 | SeongJae Park <sjpark@amazon.com> | [mm: introduce process_mrelease system call](https://lore.kernel.org/patchwork/patch/1474134) | 引入 process_mrelease() 加速进程的清理, 以更快地释放被杀死的进程的内存.<br>我们经常希望能杀死不必要的进程, 为更重要的进程释放内存. 例如 Facebook 的 OOM killer 守护程序 oomd 和 Android的低内存killer守护程序lmkd. 对于这样的系统组件, 能够快速有效地释放内存非常. 但是不幸的是, 当一个进程接收到 SIGKILL 信号的时候并不一定能及时释放自己的内存, 这可能会受到很多因素的影响, 譬如进程的状态(它可能是处于 uninterruptible sleep 态)、正在运行进程的的 core 的 OPP 级别等. 而通过调用这个新的 process_mrelease() 系统调用, 它会在调用者的上下文中释放被杀死的进程的内存. 这种方式下, 内存的释放更可控, 因为它直接在当前 CPU 上运行, 只取决于调用者任务的优先级大小. 释放内存的工作量也将由调用方承担. | v9 ☑ [5.15-rc1](https://kernelnewbies.org/LinuxChanges#Linux_5.15.Introduce_process_mrelease.282.29_system_call) | [PatchWork v9,1/2](https://lore.kernel.org/patchwork/patch/1474134) |

*   per-memcg proactive reclaim

参见 [Proactive reclaim for tiered memory and more](https://lwn.net/Articles/894849)

当前的 MG-LRU 提议[引入了一个 debugfs](https://lore.kernel.org/linux-mm/20220208081902.3550911-12-yuzhao@google.com). 可用于 MGLRU 的调试分析, 从而帮助用户出发主动回收. 这是对 MGLRU 的 lru_gen debugfs 机制的一个增量添加, 但是, 由于这是个 debug 接口, 本身独立于正在使用的回收机制(包括 CONFIG_LRU_GEN 和没有 CONFIG_LRU_GEN), 且没有直接证据表明他是直接用于进行主动回收的, 但是大家都相信通过对这些 debug 信息进行分析可以进行有效地辅助直接回收的进行.

谷歌数据中心使用了用户空间主动回收器. 此外, Meta 的[论文 TMO](https://dl.acm.org/doi/pdf/10.1145/3503222.3507731) 最近引用了一个非常类似的特性, 用于用户空间主动回收.

Google 的 Yosry Ahmed 设计的 [memcg: introduce per-memcg proactive reclaim](https://patchwork.kernel.org/project/linux-mm/cover/20220407224244.1374102-1-yosryahmed@google.com) 针对 MEMCG 提供 SYSFS 作为导出系统级分级信息的接口. 用户空间主动回收器可以持续探测 MEMCG 以回收少量内存. 随着 LRU 不断排序, 这将提供更准确和最新的工作集估计, 并可能提供更具确定性的内存过度使用行为. 内存超分配控制器可以对正在运行的应用程序不断变化的行为提供更主动的响应, 而不是被动响应. 在这种情况下, 用户空间回收器的目的不是完全替代 KSWAPD 或直接回收, 而是主动识别内存节约机会, 回收策略设置的一些冷页, 以释放内存, 用于要求更高的作业或安排新作业. 谷歌数据中心使用了这种用户空间主动回收器.

前面提到 Meta(曾经叫 Facebook) 的论文 [TMO: Transparent Memory Offloading in Datacenters](https://dl.acm.org/doi/10.1145/3503222.3507731) 讨论了一个非常类似的 per-memcg 回收机制, 用于用户空间主动回收.

此外 [Mechanism to induce memory reclaim](https://lore.kernel.org/all/5df21376-7dd1-bf81-8414-32a73cea45dd@google.com) 提出了一些新的想法: 引入一个 per-node 的 sysfs 机制来诱导内存回收, 这对于全局(非 memcg的)回收很有用, 即使在内核或挂载中没有启用 MEMCG, 也是可能的. 这可以选择一个 memcg id 来引导对 memcg 层次结构的回收. 在较低的情况下, 系统上的每个 NUMA 节点 N 将是一个 `/sys/devices/system/node/nodeN/reclaim` 机制. (这类似于现有的 per-Node 的 sysfs compact 机制, 用于从用户空间触发内存规整.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/04/07 | Yosry Ahmed <yosryahmed@google.com> | [memcg: introduce per-memcg proactive reclaim](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=eae3cb2e87ff84547e66211b81301a8f9122840f) | MEMCG 中引入 memory.reclaim 使得用户空间可以通过持续探测 MEMCG 并触发主动回收以回收少量内存. 随着 LRU 不断排序, 这将提供更准确和最新的工作集估计, 并可能提供更具确定性的内存过度使用行为. 内存超分配控制器可以对正在运行的应用程序不断变化的行为提供更主动的响应, 而不是被动响应. 在这种情况下, 用户空间回收器的目的不是完全替代 KSWAPD 或直接回收, 而是主动识别内存节约机会, 回收策略设置的一些冷页, 以释放内存, 用于要求更高的作业或安排新作业.<br>谷歌数据中心使用了此用户空间主动回收器. | v2 ☑ 5.19-rc1 | [LORE v1,0/1](https://lore.kernel.org/all/20220331084151.2600229-1-yosryahmed@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/4](https://lore.kernel.org/r/20220407224244.1374102-1-yosryahmed@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/4](https://lore.kernel.org/r/20220408045743.1432968-1-yosryahmed@google.com))<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/4](https://lore.kernel.org/r/20220421234426.3494842-1-yosryahmed@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/4](https://lore.kernel.org/r/20220425190040.2475377-1-yosryahmed@google.com) |
| 2022/04/16 | Davidlohr Bueso <dave@stgolabs.net> | [Mechanism to induce memory reclaim](https://lore.kernel.org/all/5df21376-7dd1-bf81-8414-32a73cea45dd@google.com) |  LSFMM 2022 | v1 ☐☑ | [LORE RFC v1,0/6](https://lore.kernel.org/all/5df21376-7dd1-bf81-8414-32a73cea45dd@google.com) |
| 2022/05/18 | Vaibhav Jain <vaibhav@linux.ibm.com> | [memcg: provide reclaim stats via 'memory.reclaim'](https://patchwork.kernel.org/project/linux-mm/patch/20220518223815.809858-1-vaibhav@linux.ibm.com/) | 642903 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220518223815.809858-1-vaibhav@linux.ibm.com) |


### 4.4.4 主动回收与内存分级
-------


另外一方面随着内存分级的日益普及, Davidlohr Bueso 在 LSFMM 2022 上提交了一个是引发一些 [mm: proactive reclaim and memory tiering topics](https://patchwork.kernel.org/project/linux-mm/cover/20220416053902.68517-1-dave@stgolabs.net). 主要思路和想法启发自 David 的系统级主动回收的讨论.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/04/16 | Davidlohr Bueso <dave@stgolabs.net> | [mm: proactive reclaim and memory tiering topics](https://patchwork.kernel.org/project/linux-mm/cover/20220416053902.68517-1-dave@stgolabs.net/) |  LSFMM 2022 | v1 ☐☑ | [LORE RFC v1,0/6](https://lore.kernel.org/all/20220416053902.68517-1-dave@stgolabs.net) |



# 5 Swappiness
-------


## 5.1 Swappiness 倾向
-------

Swappiness 用于设置将页面从物理内存交换到交换空间以及从页面缓存中删除页面之间的平衡. 可以通过引入 `/proc/sys/vm/swappiness` 控制系统在进行 swap 时, 内存使用的相对权重.

swappiness 参数值可设置范围在 `0~100` 之间.

1.      参数值越低, 就会让 Linux 系统尽量少用 swap 分区, 多用内存;

2.      参数值越高就是反过来, 使内核更多的去使用swap空间.

假设 swappiness 值为 60, 表示当剩余物理内存低于40%(40=100-60)时, 开始使用 swap 分区.

[swappiness参数的含义和设置](https://www.maixj.net/ict/swappiness-22576)

[Linux内核页回收swappiness参数有着什么作用](https://www.linuxprobe.com/linuxkernel-swappiness-meaning.html)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/10/12 | Andrew Morton <akpm@digeo.com> | [reduced and tunable swappiness](https://github.com/gatieme/linux-history/commit/8f7a14042e1f5fc814fa60956e3a2dcf5744a0ed) | 引入 `/proc/sys/vm/swappiness` 控制 VM 取消页面映射和交换内容的倾向. 在所有用于控制交换性的机制是: 宁愿回收 page cache, 也不愿将匿名页放到非活动列表. 对 swappiness 机制的控制如下:<br>1. 如果系统中有大量的匿名页, 我们倾向于将匿名页安置到非活动列表中.<br>2. 如果页面回收处于困境(比如更多的扫描正在发生), 然后选择将匿名页带到非活动列表. 这基本上是2.4算法.<br>3. 如果 `/proc/sys/vm/swappiness` 控制高, 则倾向于将映射的页面带到非活动列表中.<br>执行起来很简单: 将上述三件事计算成百分比并相加, 如果[超过100%, 那么开始回收匿名页](https://elixir.bootlin.com/linux/v2.5.43/source/mm/vmscan.c#L594). | v1 ☑ [2.5.43](https://elixir.bootlin.com/linux/v2.5.43/source/mm/vmscan.c#L588) | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/685701)<br>*-*-*-*-*-*-*-* <br>[COMMIT HISTORY](https://github.com/gatieme/linux-history/commit/8f7a14042e1f5fc814fa60956e3a2dcf5744a0ed) |
| 2002/10/12 | Christoph Lameter <clameter@engr.sgi.com> | [vmscan: skip reclaim_mapped determination if we do not swap](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2903fb1694dcb08a3c1d9d823cfae7ba30e66cd3) | 如果不是 may_swap, 则跳过 swappiness 机制检查回收匿名页的流程. | v1 ☑ [2.6.16](https://elixir.bootlin.com/linux/v2.6.16/source/mm/vmscan.c#L1205) | [COMMIT 1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=80e4342601abfafacb5f20571e40b56d73d10819), [COMMIT 2](https://github.com/gatieme/linux-history/commit/072eaa5d9cc3e63f567ffd9ad87b36194fdd8010), [COMMIT 3](https://github.com/gatieme/linux-history/commit/072eaa5d9cc3e63f567ffd9ad87b36194fdd8010) |
| 2007/10/16 | Andrea Arcangeli <andrea@suse.de> | [make swappiness safer to use](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4106f83a9f86afc423557d0d92ebf4b3f36728c1) | Swappiness 不是一个安全的sysctl.<br>1. 将它设置为0会挂起系统. 这是一种极端情况, 但即使将其设置为10或更低, 也会浪费大量cpu而没有取得很大进展.<br>2. 有一些客户想要使用swappiness, 但由于当前实现的原因, 他们不能使用(如果你改变它, 系统停止交换, 它真的停止交换, 如果你真的必须交换一些东西, 任何事情都不能正常工作).<br>这个补丁来自Kurt Garloff使swappiness使用更安全, 并且没有更多巨大的cpu占用或低swappiness值引起的挂起. 计算 swap_tendency 时考虑活动和非活动页面的不平衡比例 $\frac {NR_ACTIVE}{NR_INACTIVE + 1} \times \frac {vm\_swappiness + 1}{100} \times \frac {mapped\_ratio}{100}$. | v1 ☑ [2.6.24](https://kernelnewbies.org/Linux_3.16#Memory_management) | [PatchWork v3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4106f83a9f86afc423557d0d92ebf4b3f36728c1) |
| 2007/11/26 | KAMEZAWA Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com> | [per-zone and reclaim enhancements for memory controller take 3 [8/10] modifies vmscan.c for isolate globa/cgroup lru activity](https://lkml.org/lkml/2007/11/26/366) | 隔离全局和 cgroup 内存回收, 防止 cgroup 内存回收影响全局内存回收. per-zone 的页面回收感知 MEMCG 的其中一个补丁. 将原本 swap_tendency 和 reclaim_mapped 的处理流程分离新一个新函数 calc_reclaim_mapped() 中.<br>当使用内存控制器时, 有2级内存回收.<br>1. 区域内存回收, 因为系统/zone 内内存短缺.<br>2. cgroup 内存回收, 因为达到限制.<br>这两个可以通过sc->mem_cgroup参数来区分. 这个补丁试图使内存cgroup回收程序避免影响系统/区域内存回收. 这个补丁插入if (scan_global_lru()) 并钩子到 memory_cgroup回收支持函数. | v3 ☑ [2.6.25-rc1](https://elixir.bootlin.com/linux/v2.6.25/source/mm/vmscan.c#L1057) | [PatchWork v3](https://lore.kernel.org/patchwork/patch/98041), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1cfb419b394ba82745c54ff05436d598ecc2dbd5) |
| 2008/06/11 | Rik van Riel <riel@redhat.com> | [VM pageout scalability improvements (V12)](https://lore.kernel.org/patchwork/patch/118966) | 这里我们关心的是它将 LRU 中匿名页和文件页分开成两个链表进行管理时引入的平衡策略, 用于平衡我们扫描匿名列表和扫描文件列表的数量. 引入 [get_scan_ratio()](https://elixir.bootlin.com/linux/v2.6.28/source/mm/vmscan.c#L1332) 来确定确定对匿名页 LRU 列表和文件页 LRU 列表的扫描力度. 每一组 LRU
列表的相对值是通过查看我们已经旋转回活动列表而不是驱逐的页面的部分来确定的. %[0] 指定对匿名页 LRUs 施加多大压力, 而 %[1] 确定对文件 LRUs 施加多大压力. | v12 ☑ [2.6.28-rc1](https://kernelnewbies.org/Linux_2_6_28#Various_core) | [PatchWork v2](https://lore.kernel.org/patchwork/patch/118966), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4f98a2fee8acdb4ac84545df98cccecfd130f8db) |
| 2021/07/27 | Rik van Riel <riel@redhat.com> | [mm: Enable suspend-only swap spaces](https://lore.kernel.org/patchwork/patch/1468228) | 引入一个新的 SWAP_FLAG_HIBERNATE_ONLY 添加一个交换区域, 但是不允许进行通用交换, 只能在 SUSPEND/HIBERNATE 时挂起到磁盘时使用. 目前不能在不启用特定区域的通用交换的情况下启用休眠, 对此的一种半变通方法是将对 swapon() 的调用延迟到尝试休眠之前, 然后在休眠完成之后调用swapoff(). 这有点笨拙, 而且在保持交换脱离 hibernate 区域方面也不起作用. 目前 SWAP_FLAG_HIBERNATE_ONLY 设置的交换区域将不会出现在 SwapTotal 和 SwapFree 下的 /proc/meminfo 中, 因为它们不能作为常规交换使用, 但是这些区域仍然出现在 /proc/swap 中. | v4 ☐ | [PatchWork v4](https://lore.kernel.org/patchwork/patch/1468228) |
| 2022/02/17 | Peter Xu <peterx@redhat.com> | [mm: Rework zap ptes on swap entries](https://patchwork.kernel.org/project/linux-mm/cover/20220217060746.71256-1-peterx@redhat.com/) | 615245 | v5 ☐☑ | [LORE v5,0/4](https://lore.kernel.org/r/20220217060746.71256-1-peterx@redhat.com) |





## 5.2 MEMCG Swap
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/12/02 | KOSAKI Motohiro <kosaki.motohiro@jp.fujitsu.com> | [memcg: swappiness](https://lkml.org/lkml/2008/12/2/21) | 引入 per-memcg 的 swappiness, 可以用来对 per-memcg 进行精确控制. | v2 ☑ [2.6.29-rc1](https://kernelnewbies.org/Linux_2_6_29#Memory_controller_swap_management_and_other_improvements) | [LKML](https://lkml.org/lkml/2008/12/2/21), [PatchWork](https://lore.kernel.org/patchwork/patch/136809) |
| 2014/04/14 | Michal Hocko <mhocko@suse.cz> | [vmscan: memcg: Always use swappiness of the reclaimed memcg swappiness and oom_control](https://lore.kernel.org/patchwork/patch/459015) | NA | RFC ☑  | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/459015), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=688eb988d15af55c1d1b70b1ca9f6ce58f277c20) |
| 2014/04/16 | Johannes Weiner <hannes@cmpxchg.org> | [mm: memcontrol: remove hierarchy restrictions for swappiness and oom_control](https://lore.kernel.org/patchwork/patch/456852) | swappiness 和 oom_control 目前不能被调整在一个等级的一部分的memcg, 但不是那个等级的根. 当打开层次模式时, 他们无法配置 per-memcg 的 swappiness 和 oom_control. 但是这种限制并没有很好的理由. swappiness 和 oom_control的设置是从其限制触发回收和OOM调用的任何memcg获取的, 而不管它在层次结构树中的位置. 这个补丁允许<br>1. 在任何组上设置 swappiness. root memcg 上的配置读取了全局虚拟机的 swappiness, 也使它是可写的.<br>2. 允许在任何非根memcg上禁用OOM杀手.  | RFC ☑ 3.16-rc1 | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/456852), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3dae7fec5e884a4e72e5416db0894de66f586201) |


## 5.3 pagemap swap
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/07/30 | Tiberiu A Georgescu <tiberiu.georgescu@nutanix.com> | [pagemap: swap location for shared pages](https://lore.kernel.org/patchwork/patch/1470271) | NA | v2 ☐ v5.14-rc4 | [PatchWork RFC,0/4](https://lore.kernel.org/patchwork/patch/1470271) |
| 2021/08/17 | Peter Xu <peterx@redhat.com> | [mm: Enable PM_SWAP for shmem with PTE_MARKER](https://lore.kernel.org/patchwork/patch/1473423) | 这个补丁集在 shmem 上启用 pagemap 的 PM_SWAP. IOW 用户空间将能够检测 shmem 页面是否被换出, 就像匿名页面一样.<br>可以使用  CONFIG_PTE_MARKER_PAGEOUT 来启用该特性. 当启用时, 它会在 shmem 页面上带来 0.8% 的换入性能开销, 所以作者还没有将它设置为默认值. 然而, 以作者的看法, 0.8% 仍然在一个可接受的范围内, 我们甚至可以使它最终默认. | v2 ☐ v5.14-rc4 | [PatchWork RFC,0/4](https://lore.kernel.org/patchwork/patch/1473423) |

## 5.4 swap IO
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/12/16 | NeilBrown <neilb@suse.de> | [Repair SWAP-over-NFS](https://patchwork.kernel.org/project/linux-mm/cover/163969801519.20885.3977673503103544412.stgit@noble.brown) | NA | v2 ☐ | [PatchWork 00/18,V2](https://patchwork.kernel.org/project/linux-mm/cover/163969801519.20885.3977673503103544412.stgit@noble.brown) |

# 6 PageCache
-------

[The future of the page cache](https://lwn.net/Articles/712467)

[vmtouch](https://github.com/hoytech/vmtouch) 是一个 Linux 下管理和控制文件系统缓存的工具. 它可以用来, 查看一个文件(或者目录)哪些部分在内存中, 把文件调入内存, 把文件清除出内存, 把文件锁住在内存中而不被换出到磁盘上. 参见 [vmtouch——Linux 下的文件缓存管理神器播](https://blog.csdn.net/weixin_29762151/article/details/116554695), [vmtouch - the Virtual Memory Toucher](https://www.cnblogs.com/zengkefu/p/5636273.html), [vmtouch 实现原理解析](https://blog.csdn.net/DKH63671763/article/details/87990669), [ebpf 实践之 查看文件占用的缓存大小](https://www.jianshu.com/p/32aff371d6f5), [利用 vmtouch 管理文件的 page cache](http://wangxuemin.github.io/2016/02/15/利用vmtouch管理文件的page%20cache)

## 6.1 PAGE CACHE
-------


| 补丁 | 描述 |
|:---:|:---:|
| [Import 1.1.69](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/diff/mm/filemap.c?id=ea8a68b948397fa9cd1c8a1a6b61a4dc4bc99ec5) | 引入 page cache |
| [Import 1.3.21](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/diff/mm/filemap.c?id=13e539119b02e4af5daf6c55b31e6273c9a084a6) | 完善了 page cache 的功能 |
| [Import 1.3.50](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/diff/mm/filemap.c?id=22accfc2b4fe4bc6a635c70c1a05a9a80abc81ea) | 引入了 shrink_mmap() 机制来释放 page cache 的空间. |
| [Import 1.3.53](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/diff/mm/filemap.c?id=0e8625c7689bef9a38293aa927add4d9704e48be) | 引入了 page_cache_size, 统计 page cache 的大小. |
| [Linux 2.3.7pre1](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/diff/mm/filemap.c?h=2.3.7pre1&id=344971f8de0ecf3fb7ea642e319aad5865b23529) | 统一了 page cache 和 buffer 的框架. |
| [Import 2.3.16pre1](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/diff/mm/filemap.c?id=9aa2c66ac214f71cb051ba7c1adf313d9e160ee1) | 引入了 pagemap-LRU |


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/06/17 | Matthew Wilcox <willy@infradead.org> | [Convert page cache to XArray](https://lore.kernel.org/patchwork/patch/951137) | 将 page cahce 的组织结构从 radix tree 切换到 [xarray](https://lwn.net/Articles/745073). | v13 ☑ 4.20-rc1 | [PatchWork v14,00/74](https://lore.kernel.org/patchwork/patch/950304) |
| 2020/06/10 | Matthew Wilcox <willy@infradead.org> | [Large pages in the page cache](https://lore.kernel.org/patchwork/patch/1254710) | NA | v6 ☑ 5.9-rc1 | [PatchWork RFC,v6,00/51](https://patchwork.kernel.org/project/linux-mm/cover/20200610201345.13273-1-willy@infradead.org) |
| 2020/06/29 | Matthew Wilcox <willy@infradead.org> | [THP prep patches](https://patchwork.kernel.org/project/linux-mm/cover/20200629151959.15779-1-willy@infradead.org) | NA | v1 ☑ 2.5.8 | [PatchWork](https://patchwork.kernel.org/project/linux-mm/cover/20200629151959.15779-1-willy@infradead.org) |
| 2020/04/22 | Jan Kara <jack@suse.cz> | [mm: Speedup page cache truncation](https://lore.kernel.org/patchwork/patch/1229535) | 页面缓存到 xarray 的转换(关键 commit 69b6c1319b6 "mm:Convert truncate to xarray") 使页面缓存截断的性能降低了约 10%, 参见 [Truncate regression due to commit 69b6c1319b6](https://lore.kernel.org/linux-mm/20190226165628.GB24711@quack2.suse.cz). 本系列补丁旨在改进截断, 以恢复部分回归.<br>1.
第一个补丁修复了一个长期存在的错误, 我在调试补丁时发现了 xas_for_each_marked().<br>2. 剩下的补丁则致力于停止清除 xas_store() 中的标记, 从而将截断性能提高约 6%. | v2 ☐ | [PatchWork 0/23,v2](https://patchwork.kernel.org/project/linux-mm/cover/20200204142514.15826-1-jack@suse.cz) |
| 2020/10/25 | Kent Overstreet <kent.overstreet@gmail.com> | [generic_file_buffered_read() improvements](https://lore.kernel.org/patchwork/patch/1324435) | 这是一个小补丁系列, 已经在 bcachefs 树中出现了一段时间. 在缓冲读取路径中, 我们在页面缓存中查找一个页面, 然后在循环中从该页面进行复制, 即在查找每个页面之间混合数据副本. 当我们从页面缓存中进行大量读取时, 这是相当大的开销.<br>这只是重写了 generic_file_buffered_read() 以使用 find_get_pages_contig() 并处理页面数组. 对于大型缓冲读取, 这是一个非常显著的性能改进, 并且不会降低单页读取的性能.<br>generic_file_buffered_read() 被分解成多个函数, 这些函数在某种程度上更容易理解. | v2 ☑ 5.11-rc1 | [2020/06/10](https://lore.kernel.org/patchwork/patch/1254393)<br>*-*-*-*-*-*-*-* <br>[2020/06/19 v2](https://lore.kernel.org/patchwork/patch/1254398)<br>*-*-*-*-*-*-*-* <br>[2020/06/19 v3](https://lore.kernel.org/patchwork/patch/1258432)<br>*-*-*-*-*-*-*-* <br>[2020/10/17 v4](https://lore.kernel.org/patchwork/patch/1322156)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/2](https://lore.kernel.org/patchwork/patch/1324435) |
| 2021/10/20 | Yang Shi <shy828301@gmail.com> | [Solve silent data loss caused by poisoned page cache (shmem/tmpfs)](https://patchwork.kernel.org/project/linux-mm/cover/20210930215311.240774-1-shy828301@gmail.com) | 为了让文件系统意识到有毒的(poisoned)页面.<br>在讨论拆分页面缓存 THP 以脱机有毒页面的补丁时, Noaya 提到了一个[更大的问题](https://lore.kernel.org/linux-mm/CAHbLzkqNPBh_sK09qfr4yu4WTFOzRy+MKj+PA7iG-adzi9zGsg@mail.gmail.com/T/#m0e959283380156f1d064456af01ae51fdff91265), 它阻止了这一工作的进行, 因为如果发生不可纠正的错误, 页面缓存页面将被截断. 深入研究后发现, 如果页面脏, 这种方法 (截断有毒页面) 可能导致所有非只读文件系统的静默数据丢失. 对于内存中的文件系统, 例如 shmem/tmpfs, 情况可能更糟, 因为数据块实际上已经消失了. 为了解决这个问题, 我们可以将有毒的脏页面保存在页面缓存中, 然后在任何后续访问时通知用户, 例如页面错误、读 / 写等. 可以按原样截断干净的页, 因为稍后可以从磁盘重新读取它们. 结果是, 文件系统可能会发现有毒的页面, 并将其作为健康页面进行操作, 因为除了页面错误之外, 所有文件系统实际上都不会检查页面是否有毒或是否在所有相关路径中. 通常, 在将有毒页面保存在页面缓存中以解决数据丢失问题之前, 我们需要让文件系统知道有毒页面. | v3 ☐ | [2021/09/30 PatchWork RFC,v3,0/5](https://patchwork.kernel.org/project/linux-mm/cover/20210930215311.240774-1-shy828301@gmail.com)<br>*-*-*-*-*-*-*-* <br>[2021/10/14 PatchWork RFC,v4,0/6](https://patchwork.kernel.org/project/linux-mm/cover/20211014191615.6674-1-shy828301@gmail.com)<br>*-*-*-*-*-*-*-* <br>[2021/10/20 PatchWork v5,0/6](https://patchwork.kernel.org/project/linux-mm/cover/20211020210755.23964-1-shy828301@gmail.com) |
| 2022/06/24 | CGEL <cgel.zte@gmail.com> | [drop caches: skip tmpfs and ramfs](https://patchwork.kernel.org/project/linux-mm/patch/20220624182121.995294-1-ran.xiaokai@zte.com.cn/) | 653700 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220624182121.995294-1-ran.xiaokai@zte.com.cn) |
| 2022/07/01 | Jens Axboe <axboe@kernel.dk> | [mm: honor FGP_NOWAIT for page cache page allocation](https://patchwork.kernel.org/project/linux-mm/patch/fda6ea77-97c5-7235-421a-21ca5b2c48f9@kernel.dk/) | 655931 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/fda6ea77-97c5-7235-421a-21ca5b2c48f9@kernel.dk)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/dd47b882-1a18-317c-9906-e73c5487678c@kernel.dk) |


## 6.2 页面预读(readahead)
-------

[linux文件预读发展过程](https://blog.csdn.net/jinking01/article/details/106541116)

[浅谈 Linux Kernel 的预读算法](http://www.caturra.cc/2021/08/31/浅谈linux-kernel的预读算法) 基于 Linux 4.18.20 对预读进行了

[LWN: Readahead: the documentation I wanted to read](https://lwn.net/Articles/888715)

kai_ding 的 [Linux 文件系统预读](https://blog.csdn.net/kai_ding/article/details/17322787) 以三个实际读取的实例程序讲解了 linux-3.12 的预读算法. [Linux 文件系统预读 (一)](https://blog.csdn.net/kai_ding/article/details/17322787) 讲解了单进程规则顺序读时预读算法的工作. [Linux 文件系统预读 (二)](https://blog.csdn.net/kai_ding/article/details/19957763) 讲解了单进程不规则顺序读(一共进行了三次读, 顺序读, 且读的大小不定, 有超过最大预读量的, 也有低于最大预读量的)下预读的工作行为. [Linux 文件系统预读 (三)](https://blog.csdn.net/kai_ding/article/details/20112753) 讲解了多进程交织顺序读行为下的预读是如何处理的.

[2.4.18 预读算法详解](https://blog.csdn.net/liuyuanqing2010/article/details/6705338)

[Linux readahead: less tricks for more](https://www.kernel.org/doc/ols/2007/ols2007v2-pages-273-284.pdf)


从用户角度来看, 这一节对了解 Linux 内核发展帮助不大, 可跳过不读; 但对于技术人员来说, 本节可以展现教材理论模型到工程实现的一些思考与折衷, 还有软件工程实践中由简单粗糙到复杂精细的演变过程


系统在读取文件页时, 如果发现存在着顺序读取的模式时, 就会预先把后面的页也读进内存, 以期之后的访问可以快速地访问到这些页, 而不用再启动快速的磁盘读写.

### 6.2.1 原始的预读框架
-------

Linux内核的一大特色就是支持最多的文件系统, 并拥有一个虚拟文件系统(VFS)层. 早在2002年, 也就是2.5内核的开发过程中, Andrew Morton在VFS层引入了文件预读的基本框架, 以统一支持各个文件系统. 如图所示, Linux内核会将它最近访问过的文件页面缓存在内存中一段时间, 这个文件缓存被称为pagecache. 如下图所示. 一般的read()操作发生在应用程序提供的缓冲区与pagecache之间. 而预读算法则负责填充这个 pagecache. 应用程序的读缓存一般都比较小, 比如文件拷贝命令cp的读写粒度就是4KB; 内核的预读算法则会以它认为更合适的大小进行预读I/O, 比比如16-128KB.


![以pagecache为中心的读和预读](./images/0002-1-readahead_page_cache.gif)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/04/09 | Andrew Morton <akpm@digeo.com> | [readahead](https://github.com/gatieme/linux-history/commit/8fa498462272fec2c16a92a9a7f67d005225b640) | 统一的预读框架, 预读算法的雏形 | v1 ☑ 2.5.8 | [HISTORY commit](https://github.com/gatieme/linux-history/commit/8fa498462272fec2c16a92a9a7f67d005225b640) |
| 2003/02/03 | Andrew Morton <akpm@digeo.com> | [implement posix_fadvise64()](https://github.com/gatieme/linux-history/commit/fccbe3844c29beed4e665b1a5aafada44e133adc) | 引入 posix_fadvise64 | v1 ☑ 2.5.60 | [HISTORY commit](https://github.com/gatieme/linux-history/commit/fccbe3844c29beed4e665b1a5aafada44e133adc) |


### 6.2.2 早期预读算法及其优化
-------

一开始, 内核的预读方案如你所想, 很简单. 就是在内核发觉可能在做顺序读操作时, 就把后面的 128 KB 的页面也读进来.

大约一年之后, Linus Torvalds 把 mmap 缺页 I/O 的预取算法单独列出, 从而形成了 read-around/read-ahead 两个独立算法.


![Linux中的read-around, read-ahead和direct read](./images/0002-2-readahead_algorithm.gif)

1.  read-around算法适用于那些以 mmap 方式访问的程序代码和数据, 它们具有很强的局域性(locality of reference)特征. 当有缺页事件发生时, 它以当前页面为中心, 往前往后预取共计 128KB 页面.

2.  readahead 算法主要针对 read() 系统调用, 它们一般都具有很好的顺序特性. 但是随机和非典型的读取模式也大量存在, 因而 readahead 算法必须具有很好的智能和适应性.

*   read-around 与 read-ahead 算法

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2003/07/03 | Andrew Morton <akpm@zip.com.au> | [readahead optimisations](https://github.com/gatieme/linux-history/commit/82a333fa1948869322f32a67223ea8d0ae9ad8ba) | 最早的 read-around 思想实现 | v1 ☑ 2.5.27 | [HISTORY commit](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/commit/?h=v2.5.27&id=b6938a7bd23a74cfa7a81c0765ec255ea1c7e12e) |
| 2003/07/03 | Linus Torvalds <torvalds@home.osdl.org> | [Simplify and speed up mmap read-around handling](https://github.com/gatieme/linux-history/commit/82a333fa1948869322f32a67223ea8d0ae9ad8ba) | Linus Torvalds 把 mmap 缺页 I/O 的预取算法单独列出, 从而形成了 [read-around](https://elixir.bootlin.com/linux/v2.5.75/source/mm/filemap.c#L997)/read-ahead 两个独立算法 | v1 ☑ 2.5.75 | [HISTORY commit](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/commit/?h=v2.5.75&id=82a333fa1948869322f32a67223ea8d0ae9ad8ba) |

这种固定的128 KB预读方案显然不是最优的. 它没有考虑系统内存使用状况和进程读取情况. 当内存紧张时, 过度的预读其实是浪费, 预读的页面可能还没被访问就被踢出去了. 还有, 进程如果访问得凶猛的话, 且内存也足够宽裕的话, 128KB又显得太小家子气了.


*   顺序读与随机读

后续通过 Steven Pratt、Ram Pai 等人的大量工作, readahead 算法进一步完善. 其中最重要的一点是实现了对随机读的完好支持. 随机读在数据库应用中处于非常突出的地位. 在此之前, 预读算法以离散的读页面位置作为输入, 一个多页面的随机读会触发"顺序预读". 这导致了预读I/O数的增加和命中率的下降. 改进后的算法通过监控所有完整的 read() 调用, 同时得到读请求的页面偏移量和数量, 因而能够更好的区分顺序读和随机读.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2005/01/03 | Steven Pratt <slpratt@austin.ibm.com>, Ram Pai <linuxram@us.ibm.com> | [Simplified readahead](https://github.com/gatieme/linux-history/commit/6f734a1af323ab4690610ecd575198ae219b6fe8) | 引入读大小参数, 代码简化及优化; 支持随机读. | v1 ☑ 2.6.11 | [HISTORY commit 1](https://github.com/gatieme/linux-history/commit/6f734a1af323ab4690610ecd575198ae219b6fe8), [HISTORY commit 2](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/commit/?h=v2.6.11&id=250c01d06ccb125519cc9958d938f41736868be9) |
| 2005/03/07 | Oleg Nesterov <oleg@tv-sign.ru> | [readahead: improve sequential read detection](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/commit/?id=671ccb4b50a6ef21e8c0ed0ef9070098295e1e61) | 支持非对齐顺序读. | v1 ☑ [2.6.12](https://kernelnewbies.org/Linux_2_6_12) | [commit 1](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/commit/?id=671ccb4b50a6ef21e8c0ed0ef9070098295e1e61), [commit 2](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/commit/?id=577a3dd8fd68d24056075fdf479a1627586f8c46) |


*   顺序性检测

参见 [Linux 内核的文件预读机制详细详解](https://blog.csdn.net/kunyus/article/details/104620057)

为了保证预读命中率, Linux 只对顺序读 (sequential read) 进行预读. 内核通过验证如下两个条件来判定一个 read() 是否顺序读:

1.  这是文件被打开后的[第一次读](https://elixir.bootlin.com/linux/v2.6.22/source/mm/readahead.c#L476), 并且读的是[文件首部](https://elixir.bootlin.com/linux/v2.6.22/source/mm/readahead.c#L494);

2.  当前的读请求与前一 (记录的) 读请求在文件内的位置是连续的.

如果不满足上述顺序性条件, 就判定为随机读. 任何一个随机读都将终止当前的顺序序列, 从而终止预读行为 (而不是缩减预读大小). 注意这里的空间顺序性说的是文件内的偏移量, 而不是指物理磁盘扇区的连续性. 在这里 Linux 作了一种简化, 它行之有效的基本前提是文件在磁盘上是基本连续存储的, 没有严重的碎片化.

### 6.2.3 按需预读(On-demand Readahead)
-------

**2.6.23(2007年10月发布)** 的内核引入了在这个领域耕耘许久的吴峰光的一个[按需预读的算法]((https://lwn.net/Articles/235164). 所谓的按需预读, 就是内核在读取某页不在内存时, 同步把页从外设读入内存, 并且, 如果发现是顺序读取的话, 还会把后续若干页一起读进来, 这些预读的页叫预读窗口; 当内核读到预读窗口里的某一页时, 如果发现还是顺序读取的模式, 会再次启动预读, 异步地读入下一个预读窗口.

该算法关键就在于适当地决定这个预读窗口的大小,和哪一页做为异步预读的开始. 它的启发式逻辑也非常简单, 但取得不了错的效果. 此外, 对于两个进程在同一个文件上的交替预读, 2.6.24 增强了该算法, 使其能很好地侦测这一行为.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2005/10/06 | WU Fengguang <wfg@mail.ustc.edu.cn> | [Adaptive file readahead](https://lwn.net/Articles/155510) | 自适应预读算法 | v1 ☐ | [LWN](https://lwn.net/Articles/155097) |
| 2009/4/10 | Wu Fengguang <fengguang.wu@intel.com> | [filemap and readahead fixes for linux-next](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d30a11004e3411909f2448546f036a011978062e) | bugfix | v1 ☑ 2.6.31-rc1 | [LORE v1,00/14](https://lore.kernel.org/lkml/20090407115039.780820496@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/9](https://lore.kernel.org/lkml/20090410060957.442203404@intel.com), [LKML v2,0/9](https://lkml.org/lkml/2009/4/10/37) |
| 2009/4/10 | Wu Fengguang <fengguang.wu@intel.com> | [context readahead for concurrent IO](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=10be0b372cac50e2e7a477852f98bf069a97a3fa) | bugfix | v1 ☑ 2.6.31-rc1 | [LORE v1,0/3](https://lkml.kernel.org/lkml/20090410131247.764370473@intel.com) |
| 2011/05/17 | WU Fengguang <wfg@mail.ustc.edu.cn> | [512K readahead size with thrashing safe readahead](https://lwn.net/Articles/372384) | 将每次预读窗口大小的最大值从 128KB 增加到了 512KB, 其中增加了一个统计接口(tracepoint, stat 节点等). | v3 ☐ | [PatchWork](https://lore.kernel.org/patchwork/patch/190891), [LWN](https://lwn.net/Articles/234784) |
| 2011/05/17 | WU Fengguang <wfg@mail.ustc.edu.cn> | [on-demand readahead](https://lwn.net/Articles/235164) | on-demand 预读算法 | v1 ☑ [2.6.23-rc1](https://kernelnewbies.org/Linux_2_6_23#On-demand_read-ahead) | [LWN](https://lwn.net/Articles/234784) |


### 6.2.4 Page Cache Fault
-------

```cpp
do_read_fault()
-=> do_fault_around()
```


| 时间   | 作者 | 特性  | 描述  |  是否合入主线  | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2014/04/07 | Kirill A. Shutemov <kirill.shutemov@linux.intel.com> | [mm: map few pages around fault address ifthey are in page cache](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=f1820361f83d556a7f0a9f629100f3825e594328) | 文件页读取触发 page_fault 时, do_read_fault() 处理过程中, 如果发现当前文件页有 page cache 映射, 则尝试围绕缺页异常的地址预读更多的文件页面, 减少缺页异常次数. 使用 [vm_ops->map_pages()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8c6e50b0290c4c708a3e6462729e1e9151a9a7df)(默认是 filemap_map_pages()) 来完成预读. | v3 ☑✓ 3.15-rc1 | [LORE v3,0/2](https://lore.kernel.org/all/1393530827-25450-1-git-send-email-kirill.shutemov@linux.intel.com) |
| 2014/06/04 | "Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [mm: document do_fault_around() feature](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1fdb412bd825998efbced3a16f6ce7e0329728cf) | TODO | v1 ☑✓ 3.16-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1fdb412bd825998efbced3a16f6ce7e0329728cf) |
| 2014/07/23 | Konstantin Khlebnikov <koct9i@gmail.com> | [mm: do not call do_fault_around for non-linear fault](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c118678bc79e8241f9d3434d9324c6400d72f48a) | 参见 [PROBLEM: repeated remap_file_pages on tmpfs triggers bug on process exit](https://lkml.org/lkml/2014/7/14/335) | v1 ☑✓ 3.16-rc7 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c118678bc79e8241f9d3434d9324c6400d72f48a) |
| 2014/5/8 | Konstantin Khlebnikov <koct9i@gmail.com> | [mm: FAULT_AROUND_ORDER patchset performance data for powerpc](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c118678bc79e8241f9d3434d9324c6400d72f48a) | Kirill A. Shutemov 在 [commit 8c6e50b029 ("mm: introduce vm_ops->map_pages()")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1fdb412bd825998efbced3a16f6ce7e0329728cf) 尝试围绕缺页异常的地址预读更多的文件页面, 以此减少 minor page faults 的数量.<br>1. 本补丁创建基础设施, 通过 mm/Kconfig 修改 FAULT_AROUND_ORDER 的值. 这将使体系结构维护者能够基于该体系结构的性能数据来决定合适的 FAULT_AROUND_ORDER 值(默认为 4).<br>2. 列出 powerpc (平台 pseries) 的性能数字, 并对 powerpc 的 pseries 平台进行初始化. | v1 ☑✓ 3.16-rc7 | [LKML V4,0/2](https://www.lkml.org/lkml/2014/5/8/118) |


### 6.2.5 readahead support for filesystem
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:---:|:----------:|:----:|
| 2022/05/16 | Hsin-Yi Wang <hsinyi@chromium.org> | [Implement readahead for squashfs](https://patchwork.kernel.org/project/linux-mm/cover/20220516105100.1412740-1-hsinyi@chromium.org/) | 641926 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20220516105100.1412740-1-hsinyi@chromium.org)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/3](https://lore.kernel.org/r/20220601103922.1338320-1-hsinyi@chromium.org)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/3](https://lore.kernel.org/r/20220606150305.1883410-1-hsinyi@chromium.org)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/4](https://lore.kernel.org/r/20220617083810.337573-1-hsinyi@chromium.org) |



### 6.2.6 More readahead Patchset
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:---:|:----------:|:----:|
| 2022/06/07 | Alistair Popple <apopple@nvidia.com> | [mm/filemap.c: Always read one page in do_sync_mmap_readahead()](https://patchwork.kernel.org/project/linux-mm/patch/20220607083714.183788-1-apopple@nvidia.com/) | 647906 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220607083714.183788-1-apopple@nvidia.com) |


## 6.3 DirtyPage
-------


### 6.3.1 页面写回
-------


从用户角度来看, 这一节对了解 Linux 内核发展帮助不大, 可跳过不读; 但对于技术人员来说, 本节可以展现教材理论模型到工程实现的一些思考与折衷, 还有软件工程实践中由简单粗糙到复杂精细的演变过程


当进程改写了文件缓存页, 此时内存中的内容与**后备存储设备(backing device)**的内容便处于不一致状态, 此时这种页面叫做**脏页(dirty page).** 内核会把这些脏页写回到后备设备中. 这里存在一个折衷: 写得太频繁(比如每有一个脏页就写一次)会影响吞吐量; 写得太迟(比如积累了很多个脏页才写回)又可能带来不一致的问题, 假设在写之前系统崩溃, 则这些数据将丢失, 此外, 太多的脏页会占据太多的可用内存. 因此, 内核采取了几种手段来写回:

1) 设一个后台门槛(background threshold), 当系统脏页数量超过这个数值, 用后台线程写回, 这是异步写回.

2) 设一个全局门槛(global threshold), 这个数值比后台门槛高. 这是以防系统突然生成大量脏页, 写回跟不上, 此时系统将扼制(throttle)生成脏页的进程, 让其开始同步写回.



#### 6.3.1.1 由全局的脏页门槛到每设备脏页门槛
-------

**2.6.24(2008年1月发布)**


内核采取的第2个手段看起来很高明 - 扼制生成脏页的进程, 使其停止生成, 反而开始写回脏页, 这一进一退中, 就能把全局的脏页数量拉低到全局门槛下. 但是, 这存在几个微妙的问题:



> **1\.** 有可能这大量的脏页是要写回某个后备设备A的, 但被扼制的进程写的脏页则是要写回另一个后备设备B的. 这样, 一个不相干的设备的脏页影响到了另一个(可能很重要的)设备, 这是不公平的.
> **2\.** 还有一个更严重的问题出现在栈式设备上. 所谓的栈式设备(stacked device)是指多个物理设备组成的逻辑设备, 如 LVM 或 software RAID 设备上. 操作在这些逻辑设备上的进程只能感知到这些逻辑设备. 假设某个进程生成了大量脏页, 于是, 在逻辑设备这一层, 脏页到达门槛了, 进程被扼制并让其写回脏页, 由于进程只能感知到逻辑设备这一层, 所以它觉得脏页已经写下去了. 但是, 这些脏页分配到底下的物理设备这一层时, 可能每个物理设备都还没到达门槛, 那么在这一层, 是不会真正往下写脏页的. 于是, 这种极端局面造成了死锁: 逻辑设备这一层以为脏页写下去了; 而物理设备这一层还在等更多的脏页以到达写回的门槛.





2.6.24 引入了一个新的改进[Smarter write throttling](https://lwn.net/Articles/245600), 就是把全局的脏页门槛替换为每设备的门槛. 这样第1个问题自然就解决了. 第2个问题其实也解决了. 因为现在是每个设备一个门槛, 所以在物理设备这一层, 这个门槛是会比之前的全局门槛低很多的, 于是出现上述问题的可能性也不存在了.



那么问题来了, 每个设备的门槛怎么确定? 设备的写回能力有强有弱(SSD 的写回速度比硬盘快多了), 一个合理的做法是根据当前设备的写回速度分配给等比例的带宽(门槛). **这种动态根据速度调整的想法在数学上就是[指数衰减](https://zh.wikipedia.org/wiki/%E6%8C%87%E6%95%B0%E8%A1%B0%E5%87%8F)的理念:某个量的下降速度和它的值成比例.** 所以, 在这个改进里, 作者引入了一个叫"**浮动比例**"的库, 它的本质就是一个**根据写回速度进行指数衰减的级数**. (这个库跟内核具体的细节无关, 感兴趣的可以研究这个代码: [[PATCH 19/23] lib: floating proportions [LWN.net]](https://lwn.net/Articles/245603). 然后, 使用这个库, 就可以"实时地"计算出每个设备的带宽(门槛).



#### 6.4.1.2 引入更具体扩展性的回写线程
-------

**2.6.32(2009年12月发布)**


Linux 内核在脏页数量到达一定门槛时, 或者用户在命令行输入 _sync_ 命令时, 会启用后台线程来写回脏页, 线程的数量会根据写回的工作量在2个到8个之间调整. 这些写回线程是面向脏页的, 而不是面向后备设备的. 换句话说, 每个回写线程都是在认领系统全局范围内的脏页来写回, 而这些脏页是可能属于不同后备设备的, 所以回写线程不是专注于某一个设备.



不过随着时间的推移, 这种看似灵巧的方案暴露出弊端.

> **1\.** 由于每个回写线程都是可以服务所有后备设备的, 在有多个后备设备, 且回写工作量大时, 线程间的冲突就变得明显了(毕竟, 一个设备同一时间内只允许一个线程写回), 当一个设备有线程占据, 别的线程就得等, 或者去先写别的设备. 这种冲突对性能是有影响的.
> **2\.** 线程写回时, 把脏页组成一个个写回请求, 挂在设备的请求队列上, 由设备去处理. 显然,每个设备的处理请求能力是有限的, 即队列长度是有限的. 当一个设备的队列被线程A占满, 新来的线程B就得不到请求位置了. 由于线程是负责多个设备的, 线程B不能在这设备上干等, 就先去忙别的, 以期这边尽快有请求空位空出来. 但如果线程A写回压力大, 一直占着请求队列不放, 那么A就及饥饿了, 时间太久就会出问题.



针对这种情况, 2.6.32为每个后备设备引入了专属的写回线程, 参见 [Flushing out pdflush](https://lwn.net/Articles/326552), 换言之, 现在的写回线程是面向设备的. 在写回脏页时, 系统会根据其所属的后备设备, 派发给专门的线程去写回, 从而避免上述问题.





#### 6.3.1.3 动态的脏页生成扼制和写回扼制算法
-------

**3.1(2011年11月发布), 3.2(2012年1月发布)**


本节一开始说的写回扼制算法, 其核心就是**谁污染谁治理: 生成脏页多的进程会被惩罚, 让其停止生产, 责成其进行义务劳动, 把系统脏页写回.** 在5.1小节里, 已经解决了这个方案的针对于后备设备门槛的一些问题. 但还存在一些别的问题.



> **1.** 写回的脏页的**破碎性导致的性能问题**. 破碎性这个词是我造的, 大概的意思是, 由于被罚的线程是同步写回脏页到后备设备上的. 这些脏页在后备设备上的分布可能是很散乱的, 这就会造成频繁的磁盘磁头移动, 对性能影响非常大. 而 Linux 存在的一个块层(block layer, 倒数第2个子系统会讲)本来就是要解决这种问题, 而现在写回机制是相当于绕过它了.
>
> **2\.** 系统根据当前可用内存状况决定一个脏页数量门槛, 一到这个门槛值就开始扼制脏页生成. 这种做法太粗野了点. 有时启动一个占内存大的程序(比如一个 kvm), 脏页门槛就会被急剧降低, 就会导致粗暴的脏页生成扼制.
>
> **3\. 从长远看, 整个系统生成的脏页数量应该与所有后备设备的写回能力相一致.** 在这个方针指导下, 对于那些过度生产脏页的进程, 给它一些限制, 扼制其生成脏页的速度. 内核于是设置了一个**定点**, 在这个定点之下, 内核对进程生成脏页的速度不做限制; 但一超过定点就开始粗暴地限制.



3.1, 3.2 版本中, 来自 Intel 中国的吴峰光博士针对上述问题, 引入了动态的脏页生成扼制和[写回扼制算法 Dynamic writeback throttling[(https://lwn.net/Articles/405076). 其主要核心就是, 不再让受罚的进程同步写回脏页了, 而是罚它睡觉; 至于脏页写回, 会委派给专门的[写回线程](https://lwn.net/Articles/326552), 这样就能利用块层的合并机制, 尽可能把磁盘上连续的脏页合并再写回, 以减少磁头的移动时间.



至于标题中的"动态"的概念, 主要有下:

> **1\.** 决定受罚者睡多久. 他的算法中, 动态地去估算后备设备的写回速度, 再结合当前要写的脏页量, 从而动态地去决定被罚者要睡的时间.
> **2\.** 平缓地修改扼制的门槛. 之前进程被罚的门槛会随着一个重量级进程的启动而走人骤降, 在吴峰光的算法中, 增加了对全局内存压力的评估, 从而平滑地修改这一门槛.
> **3\.** 在进程生成脏页的扼制方面, 吴峰光同样采取反馈调节的做法, 针对写回工作量和写回速度, 平缓地(尽量)把系统的脏页生成控制在定点附近.


### 6.3.2 dirty limits
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2005/10/06 | WU Fengguang <wfg@mail.ustc.edu.cn> | [per-zone dirty limits v3](https://lore.kernel.org/patchwork/patch/268866) | per-zone 的脏页限制.<br>在回收期间回写单个文件页面显示了糟糕的IO模式, 但在VM有其他方法确保区域中的页面是可回收的之前, 我们不能停止这样做. 随着时间的推移, 出现了一些建议, 当在内存压力期间出现需要清理页面时, 至少要在inode-proximity中对页面进行写转. 但是即使这样也会中断来自刷新器的回写, 而不能保证附近的inode-page位于同一问题区域. 脏页面之所以会到达LRU列表的末尾, 部分原因在于脏限制是一个全局限制, 而大多数系统都有多个大小不同的LRU列表. 多个节点有多个区域, 有多个文件列表, 但与此同时, 除了在遇到脏页时回收它们之外, 没有任何东西可以平衡这些列表之间的脏页.  | v3 ☐ | [PatchWork](https://lore.kernel.org/patchwork/patch/268866) |


### 6.3.4 其他
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/07/13 | Jan Kara <jack@suse.cz> | [writeback: Fix bandwidth estimates](https://patchwork.kernel.org/project/linux-mm/cover/20210712165811.13163-1-jack@suse.cz) | NA | v2 ☐ | [PatchWork 0/5,v2](https://patchwork.kernel.org/project/linux-mm/cover/20210712165811.13163-1-jack@suse.cz) |
| 2011/08/10 | Mel Gorman <mgorman@suse.de> | [Reduce filesystem writeback from page reclaim v3](https://lore.kernel.org/all/1312973240-32576-1-git-send-email-mgorman@suse.de) | 1312973240-32576-1-git-send-email-mgorman@suse.de | v3 ☐☑✓ | [LORE v3,0/7](https://lore.kernel.org/all/1312973240-32576-1-git-send-email-mgorman@suse.de) |



# 7 大内存页支持
-------


我们知道现代操作系统都是以页面(page)的方式管理内存的. 一开始, 页面的大小就是4K, 在那个时代, 这是一个相当大的数目了. 所以众多操作系统, 包括 Linux , 深深植根其中的就是一个页面是4K大小这种认知, 尽管现代的CPU已经支持更大尺寸的页面(X86体系能支持2MB, 1GB).



我们知道虚拟地址的翻译要经过页表的翻译, CPU为了支持快速的翻译操作, 引入了TLB的概念, 它本质就是一个页表翻译地址结果的缓存, 每次页表翻译后的结果会缓存其中, 下一次翻译时会优先查看TLB, 如果存在, 则称为TLB hit; 否则称为TLB miss, 就要从访问内存, 从页表中翻译. 由于这是一个CPU内机构, 决定了它的尺寸是有限的, 好在由于程序的局部性原理, TLB 中缓存的结果很大可能又会在近期使用.



但是, 过去几十年, 物理内存的大小翻了几番, 但 TLB 空间依然局限, 4KB大小的页面就显得捉襟见肘了. 当运行内存需求量大的程序时, 这样就存在更大的机率出现 TLB miss, 从而需要访问内存进入页表翻译. 此外, 访问更多的内存, 意味着更多的缺页中断. 这两方面, 都对程序性能有着显著的影响.


LWN 上 Mel 写的关于 Huge Page 的连载.

[Huge pages part 1 (Introduction)](https://lwn.net/Articles/374424)

[Huge pages part 2: Interfaces](https://lwn.net/Articles/375096)

[Huge pages part 3: Administration](https://lwn.net/Articles/376606)

[Huge pages part 4: benchmarking with huge pages](https://lwn.net/Articles/378641)

[Huge pages part 5: A deeper look at TLBs and costs](https://lwn.net/Articles/379748)

[[v2,0/4] x86, mm: Handle large PAT bit in pud/pmd interfaces](https://lore.kernel.org/patchwork/patch/579540)

[[v12,0/10] Support Write-Through mapping on x86](https://lore.kernel.org/patchwork/patch/566256/)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/01/26 | Matthew Wilcox <willy@infradead.org><br>Dave Jiang <dave.jiang@intel.com> | [Support for transparent PUD pages](https://lwn.net/Articles/669232) | NA | ☑ 4.11-rc1 | [PatchWork RFC](https://patchwork.kernel.org/project/linux-nvdimm/patch/148545059381.17912.8602162635537598445.stgit@djiang5-desk3.ch.intel.com) |
| 2021/07/30 | Hugh Dickins <hughd@google.com> | [tmpfs: HUGEPAGE and MEM_LOCK fcntls and memfds](https://lwn.net/Articles/669232) | NA  | ☑ 4.11-rc1 | [PatchWork 00/16](https://patchwork.kernel.org/project/linux-mm/cover/2862852d-badd-7486-3a8e-c5ea9666d6fb@google.com) |




## 7.1 标准大页 HugeTLB 支持
-------


### 7.1.1 引入 HugeTLB
-------


如果能使用更大的页面, 则能很好地解决上述问题. 试想如果使用2MB的页(一个页相当于512个连续的4KB 页面), 则所需的 TLB 表项由原来的 512个变成1个, 这将大大提高 TLB hit 的机率; 缺页中断也由原来的512次变为1次, 这对性能的提升是不言而喻的.


然而, 前面也说了 Linux 对4KB大小的页面认知是根植其中的, 想要支持更大的页面, 需要对非常多的核心的代码进行大改动, 这是不现实的. 于是, 为了支持大页面, 有了一个所谓 HugeTLB 的实现.


它的实现是在系统启动时, 按照用户指定需求的最大大页的大小和个数, 预留足够的物理内存空间. 用户在程序中可以使用 **mmap()** 系统调用或共享内存的方式映射和访问这些大页, 参考官方文档: [HugeTLB Pages](https://www.kernel.org/doc/html/latest/admin-guide/mm/hugetlbpage.html).

当然, 现在也存在一些用户态工具, 可以帮助用户更便捷地使用. 具体可参考此文章: [Huge pages part 2: Interfaces [LWN.net]](https://lwn.net/Articles/375096).

同 tmpfs 类似, 基于 hugetlbfs 创建文件后, 再进行 mmap() 映射, 就可以访问这些 huge page 了, 使用方法可参考内核源码 "tools/testing/selftests/vm" 中的示例.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/09/15 | Rohit Seth/Andrew Morton <akpm@digeo.com> | [hugetlb pages](https://github.com/gatieme/linux-history/commit/c9d3808fc28fc873dcf0dc95315f644997fde1d0) | 实现 ia32 的 HugeTLB. 由 CONFIG_HUGETLB_PAGE 控制. | v1 ☑ 2.5.36 | [HISTORY COMMIT](https://github.com/gatieme/linux-history/commit/c9d3808fc28fc873dcf0dc95315f644997fde1d0) |
| 2002/09/15 | Bill Irwin/Andrew Morton <akpm@digeo.com> | [hugetlbfs file system](https://github.com/gatieme/linux-history/commit/9f3336ab7c42d631f5ed50d73e1eea7bd9268892) | 为 hugetlb page 实现了一个小巧的  ram-backed 的文件系统. 通过 CONFIG_HUGETLBFS 控制. | v1 ☑ 2.5.46 | [HISTORY COMMIT](https://github.com/gatieme/linux-history/commit/9f3336ab7c42d631f5ed50d73e1eea7bd9268892) |
| 2004/04/11 | William Lee Irwin III <wli@holomorphy.com> | [hugetlb consolidation](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/commit/?id=c8b976af1af10de3d92968bf7d4bd5415e8a3778) | 整合了各个架构下 hugetlb 实现中的冗余代码, 实现了统一的 hugetlb 框架. 注意这里将整个 HugeTLB 的代码都用 CONFIG_HUGETLBFS 选项来控制开启. | v1 ☑ 2.6.6-rc1 | [HISTORY COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/commit/?id=c8b976af1af10de3d92968bf7d4bd5415e8a3778) |

至此 [v2.6.6](https://elixir.bootlin.com/linux/v2.6.6/source) 版本, HugeTLB 的功能和框架已经趋于完善.

HugeTLB 的功能, 涉及到两个模块: HugeTLB 和 HugeTLBFS.

HugeTLB 相当于是 HugeTLB 页面管理者, 页面的分配及释放, 都由此模块负责. 由 [CONFIG_HUGETLB_PAGE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e3390f67a7267daa227380b6f1bbf13c7ddd4aff) 标记.

HugeTLBFS 则用于向用户提供一套基于文件系统的巨页使用界面, 其下层功能的实现, 则依赖于 HugeTLB. 由 [CONFIG_HUGETLBFS] 控制.

首先是 HugeTLB 模块:

1.  通过内核启动参数 [`hugepages=`](https://elixir.bootlin.com/linux/v2.6.6/source/mm/hugetlb.c#L115) 来指定预留的 HugeTLB 的数量.

2.  然后内核启动时 [`hugetlb_init()`](https://elixir.bootlin.com/linux/v2.6.6/source/mm/hugetlb.c#L87) 中会通过分配 [`alloc_fresh_huge_page()`](https://elixir.bootlin.com/linux/v2.6.6/source/mm/hugetlb.c#L45).

3.  所有分配的 HugeTLB 页面会由 [`enqueue_huge_page(struct page *page)`](https://elixir.bootlin.com/linux/v2.6.6/source/mm/hugetlb.c#L20) 添加到 [`hugepage_freelists`](https://elixir.bootlin.com/linux/v2.6.6/source/mm/hugetlb.c#L17) 链表中.

4.  提供了 `/proc/sys/vm/nr_hugepages` sysctl 接口[查看](https://elixir.bootlin.com/linux/v2.6.6/source/kernel/sysctl.c#L734)和[配置](https://elixir.bootlin.com/linux/v2.6.6/source/mm/hugetlb.c#L183)当前内核中 HugeTLB 大页数目 [max_huge_pages](https://elixir.bootlin.com/linux/v2.6.6/source/mm/hugetlb.c#L16). 在配置的过程中内核会通过 [`alloc_fresh_huge_page()`](https://elixir.bootlin.com/linux/v2.6.6/source/mm/hugetlb.c#L159) 和 [`enqueue_huge_page(page)`](https://elixir.bootlin.com/linux/v2.6.6/source/mm/hugetlb.c#L163) 扩展页面数, 已经通过 [`try_to_free_low()`](https://elixir.bootlin.com/linux/v2.6.6/source/mm/hugetlb.c#L132) 和 [`update_and_free_page()`](https://elixir.bootlin.com/linux/v2.6.6/source/mm/hugetlb.c#L176) 释放那些不需要的页面. 从而动态地提供大页的数量.

其次看 HugeTLBFS 模块:

### 7.1.2 HugeTLB Pool
-------

commit [b45b5bd65f66 ("hugepage: Strict page reservation for hugepage inodes")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b45b5bd65f668a665db40d093e4e1fe563533608)


commit [a43a8c39bbb4 ("tightening hugetlb strict accounting")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a43a8c39bbb493c9e93f6764b350de2e33e18e92)

随后 v2.6.24 HugeTLB 又引入了 dynamic pool 和 overcommit.


由于匿名映射 MAP_PRIVATE 不使用保留的大面内存, 因此分配可能由于 HugeTLB 池太小而失败. commit [7893d1d505d5 ("hugetlb: Try to grow hugetlb pool for MAP_PRIVATE mappings")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7893d1d505d59db9d4f35165c8b6d3c6dff40a32) 引入动态扩展 HugeTLB 页面池的的机制.<br>1. 尝试通过 `alloc_buddy_huge_page()` 从 buddy 分配器中获取一个剩余的大页.<br>2. 通过 surplus_huge_pages 和 surplus_huge_pages_node 来记录动态扩展的页面.<br>3. 那么这种情况下如果通过 nr_hugepages sysctl 动态修改 HugeTLB 页面的数量, 则通过 `set_max_huge_pages()-=>adjust_pool_surplus()` 来完成动态扩展.

commit [e4e574b767ba ("hugetlb: Try to grow hugetlb pool for MAP_SHARED mappings")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e4e574b767ba63101cfda2b42d72f38546319297)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/10/16 | Adam Litke <agl@us.ibm.com> | [hugetlb_dynamic_pool](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=af767cbdd78f293485c294113885d95e7f1da123) | 引入 dynamic_pool. | v1 ☑ [2.6.24-rc1](https://lwn.net/Articles/255649) | [HISTORY COMMIT 0/6](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=af767cbdd78f293485c294113885d95e7f1da123) |
| 2007/12/17 | Nishanth Aravamudan <nacc@us.ibm.com> | [hugetlb: introduce nr_overcommit_hugepages sysctl](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d5dbac87b4343d98ae509fb787efb77f8ddc484b) | 1. 移除了 [Revert "hugetlb: Add hugetlb_dynamic_pool sysctl"](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=368d2c6358c3c62b3820a8a73f9fe9c8b540cdea)<br>2. 引入了 [nr_overcommit_hugepages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d1c3fb1f8f29c41b0d098d7cfb3c32939043631f) sysctl.<br>3. [Documentation: update hugetlb information](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d5dbac87b4343d98ae509fb787efb77f8ddc484b) 更新了 hugetlb 的文档, 至此 hugetlb 的功能已经趋于完善. | v1 ☑ 2.6.24-rc6 | [HISTORY COMMIT](https://github.com/gatieme/linux-history/commit/d5dbac87b4343d98ae509fb787efb77f8ddc484b) |


### 7.1.2 SHM_HUGETLB
-------


通过 shmget()/shmat() 也可以使用 hugetlb, 调用 shmget() 申请共享内存时要加上 SHM_HUGETLB 标志即可. 具体使用可以参考 selftests 用例中的 `hugepage-shm.c`.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/10/30 | Bill Irwin/Andrew Morton <akpm@digeo.com> | [hugetlbfs backing for SYSV shared memory](https://github.com/gatieme/linux-history/commit/bba2dd58c14a371b1062e585a280059fc6e9364f) | 实现 SHM_HUGETLB, 为进程的 SHEM 支持 hugetlb .<br>对于 hugetlb 接口的用户来说, 最常见的请求之一就是使用 shm 的数据库. 这个补丁导出的功能基本上相当于 tmpfs, 将调用序列添加到 `ipc/shm.c`, 并在 `fs/hugetlbfs/inode.c` 中散列出一个小的支持函数, 这样如果用户空间将一个标志传递给 shmget(), shm 段可能是 hugetlbpage 支持的. | v1 ☑ 2.5.46 | [HISTORY COMMIT](https://github.com/gatieme/linux-history/commit/bba2dd58c14a371b1062e585a280059fc6e9364f) |
| 2002/10/30 | Rohit Seth | [hugetlbpage documentation update](https://github.com/gatieme/linux-history/commit/a2f6cc8614e920b7b86782ac8391a15165631157) | 更新 hugetlb 的文档, 同时增加了 SHM_HUGETLB 的测试用例 `Documentation/vm/hugetlbpage.txt`, 该用例随后被重命名为 [`Documentation/vm/hugepage-shm.c`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=70bace8c1edefa700c7f7af522c5374ef63860ae), 最终被[移动到了 selftests 路径下](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f0f57b2b1488). | v1 ☑ 2.5.64 | [HISTORY COMMIT](https://github.com/gatieme/linux-history/commit/a2f6cc8614e920b7b86782ac8391a15165631157) |


### 7.1.3 MAP_HUGETLB
-------


但是每次使用 huge page 都需要将 hugetlbfs 挂载到文件系统到某个节点上, 部署起来很不方便, 我们只想要点匿名页面, 要搞的那么麻烦吗?
于是 2.6.32 内核通过支持 MAP_HUGETLB 方式来使用内存, 避免了烦琐的 mount 操作, 对用户更友好.

至此, 通过 mmap(), 调用时指定 MAP_HUGETLB 标志就可以使用 hugetlb 了, 方便了很多.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/08/14 | Eric B Munson <ebmunson@us.ibm.com> | [Add pseudo-anonymous huge page mappings V3](https://lore.kernel.org/linux-man/cover.1250258125.git.ebmunson@us.ibm.com) | 这个补丁集为 mmap 添加了一个标志 [MAP_HUGETLB](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4e52780d41a741fb4861ae1df2413dd816ec11b1), 允许用户请求使用大页面支持映射.<br>这个映射借用[大页 shm 代码的功能](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6bfde05bf5c9682e255c6a2c669dc80f91af6296), 在内核内部挂载上创建一个文件, 并使用它近似于匿名映射 MAP_ANONYMOUS.<br>1. MAP_HUGETLB 标志是 MAP_ANONYMOUS 的修饰符, 如果两个标志都没有被预设, 它就不能工作.<br>2. 一个新的标志是必要的, 因为当前除了在 hugetlbfs 挂载上创建一个文件外, 没有其他方法可以映射到到大页.<br>3. 对于用户空间, 这个映射的行为就像匿名映射, 因为这个文件在内核之外是不可访问的. | RFC ☐ | [PatchWork 0/3](https://lore.kernel.org/linux-man/cover.1250258125.git.ebmunson@us.ibm.com) |


### 7.1.4 More huge page sizes
-------

在 2.6.25 版本的时候, 通过 commit [4ec161cf73bc ("Add hugepagesz boot-time parameter")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4ec161cf73bc0b4e5c36843638ef9171896fc0b9) 添加了一个启动参数 hugepagesz, 可以指定预留的 HugeTLB 大页的 pagesize.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/05/26 | Andi Kleen <ak@suse.de>/Nick Piggin <npiggin@suse.de> | [multi size, giant hugetlb support, 1GB for x86, 16GB for powerpc](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0d9ea75443dc7e37843e656b8ebc947a6d16d618) | 多种 hugepage sizes 的 HugeTLB 的支持. 引入了 [struct hstate](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a5516438959d90b071ff0a484ce4f3f523dc3152) 来管理不同 page size 的 HugeTLB. 每个 pagesize 的页面由 [hstate 自己的链表](https://elixir.bootlin.com/linux/v2.6.27/source/mm/hugetlb.c#L384)管理.<br>引入了 [PUD 级别](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ceb868796181dc95ea01a110e123afd391639873) 和 [巨页(> MAX_ORDER)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=aa888a74977a8f2120ae9332376e179c39a6b07d) 的 Huge Page. 随后对 [x86_64](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=39c11e6c05b7fedbf7ed4df3908b25f622d56204) 以及 [POWERPC](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0d9ea75443dc7e37843e656b8ebc947a6d16d618) 等架构做了支持. POWERPC 更是支持了 [16GB 级别的巨页](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ec4b2c0c8312d1118c2acd00c89988ecf955d5cc). 可以通过启动参数 hugepagesz 参数指定大页的大小. | v1 ☑ 2.6.27-rc1 | [LKML 00/23](https://lore.kernel.org/all/20080525142317.965503000@nick.local0.net) |


huge page 最开始只支持 PMD 级别(基础页 4K, 则 PMD 级别为 2MB)的大页, 自 3.8 版本加入这个 [commit 42d7395feb56 ("mm: support more pagesizes for MAP_HUGETLB/SHM_HUGETLB")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=42d7395feb56f0655cd8b68e06fc6063823449f8) 之后, 利用 shmget()/mmap() 的 flag 参数中未使用的 bits, 可以支持其他的 huge page 大小(比如 1GB).

进一步的, ARM64 MMU 支持一个连续位(Contiguous bit), 该位表示 TTE 是可缓存在单个 TLB 条目中的一组连续条目之一. 通过对该位的支持可以增加新的中间大页的大小. 可用的巨大页面大小取决于基本页面大小 PAGE_SIZE. 参见 [arm64: Add support for PTE contiguous bit.](https://lore.kernel.org/lkml/1450380686-20911-1-git-send-email-dwoods@ezchip.com)


[arm64: hugetlb: partial revert of 66b3923a1a0f](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ff7925848b50050732ac0401e0acf27e8b241d7b)

[Revert "arm64: hugetlb: partial revert of 66b3923a1a0f"](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ab2e1b89230fa80328262c91d2d0a539a2790d6f)

[arm64: Re-enable support for contiguous hugepages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5cd028b9d90403bf24c8bf7915ed61c7a9bfce6c)

在不使用连续页面的情况下, 大页面大小如下所示:

```cpp
-----------------------------
| Page Size |  PMD  |  PUD  |
-----------------------------
|     4K    |   2M  |   1G  |
|    16K    |  32M  |       |
|    64K    | 512M  |       |
-----------------------------
```

对于 4KB 的 PAGE_SIZE, 使用 Contiguous bit 的相邻位将 16 页的集合分组,
对于 64KB 的 PAGE_SIZE, 它将 32 页的集合分组. 这将在每种情况下启用两个新的巨大页面大小, 因此完整的可用大小集如下所示.
如果使用 16KB 的 PAGE_SIZE, 则连续位在 PTE 级别将 128 页分组, 在 PMD 级别将 32 页分组.

```cpp
---------------------------------------------------
| Page Size | CONT PTE |  PMD  | CONT PMD |  PUD  |
---------------------------------------------------
|     4K    |   64K    |   2M  |    32M   |   1G  |
|    16K    |    2M    |  32M  |     1G   |       |
|    64K    |    2M    | 512M  |    16G   |       |
---------------------------------------------------
```

如果基本页面大小设置为 64KB, 则默认情况下会启用 2MB 页面. 在将来, 4KB 和 64KB 的页面都可以使用 2MB 作为默认的巨大页面大小.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/12/05 | Andi Kleen <ak@linux.intel.com> | [mm: support more pagesizes for MAP_HUGETLB/SHM_HUGETLB](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=42d7395feb56f0655cd8b68e06fc6063823449f8) | Free page reporting 只支持伙伴系统中的页面, 它不能报告为 hugetlbfs 预留的页面. 这个补丁在 hugetlb 的空闲列表中增加了对报告巨大页的支持, 它可以被 virtio_balloon 驱动程序用于内存过载和预归零空闲页, 以加速内存填充和页面错误处理. | v7 ☑ 3.8-rc1 | [LWN v7](https://lwn.net/Articles/533650), [LORE v7](https://lore.kernel.org/lkml/1352157848-29473-2-git-send-email-andi@firstfloor.org), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=42d7395feb56f0655cd8b68e06fc6063823449f8) |
| 2015/12/17 | David Woods <dwoods@ezchip.com> | [arm64: Add support for PTE contiguous bit.](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=66b3923a1a0f77a563b43f43f6ad091354abbfe9) | 引入 hugetlb cgroup | v5 ☑ 4.5-rc1 | [LKML v5](https://lore.kernel.org/lkml/1450380686-20911-1-git-send-email-dwoods@ezchip.com) |
| 2017/08/22 | Punit Agrawal <punit.agrawal@arm.com> | [arm64: Enable contiguous pte hugepage support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=828f193dd62a40ade5ea8b24cb8b0a22c30df673) | 20170822104249.2189-1-punit.agrawal@arm.com | v7 ☑✓ 4.14-rc1 | [LORE v7,0/9](https://lore.kernel.org/all/20170822104249.2189-1-punit.agrawal@arm.com) |
| 2022/03/23 | Steve Capper <steve.capper@arm.com> | [tlb: hugetlb: Add arm64 contiguous hint awareness](https://patchwork.kernel.org/project/linux-mm/patch/20220323165218.35499-1-steve.capper@arm.com/) | 该补丁实现了 per-arch 的 tlb_remove_huge_tlb_entry() 能力, 并提供了一个覆盖连续提示大小的 arm64 实现. 在更新 mmu_gather 结构时, tlb_remove_huge_tlb_entry 要支持不同 level HugePage 支持. 但是当前只支持了 PMD_SIZE 和 PUD_SIZE. 而 arm64 上还有两个巨页面需要覆盖: CONT_PTE_SIZE 和 CONT_PMD_SIZE, 当最终用户试图使用大页时, 由于相关 tlb 未正确更新 tlb 结构, 可能会出现 VM_BUG_ON. | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/all/20220323165218.35499-1-steve.capper@arm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20220330112543.863-1-steve.capper@arm.com) |


### 7.1.5 HugeTLB CGROUP
-------

参见内核文档 [HugeTLB Controller](https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v1/hugetlb.html).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/07/31 | Aneesh Kumar K.V <aneesh.kumar@linux.vnet.ibm.com> | [hugetlb: Add HugeTLB controller to control HugeTLB allocation](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=75754681fe79b84dde1048470a44eeb64192fad6) | 引入 hugetlb cgroup | v9 ☑ 3.6-rc1 | [LKML v1,0/9](https://lkml.org/lkml/2012/2/20/127)<br>*-*-*-*-*-*-*-* <br>[LKML v8,00/16](https://lkml.org/lkml/2012/6/9/22)[LKML v9,00/15](https://lkml.org/lkml/2012/6/13/161), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2bc64a2046975410505bb119bba32705892b9255) |
| 2019/12/16 | Giuseppe Scrivano <gscrivan@redhat.com> | [mm: hugetlb controller for cgroups v2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=faced7e0806cf44095a2833ad53ff59c39e6748d) | cgroup v2 支持 hugetlb. | v1 ☑ 5.6-rc1 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=faced7e0806cf44095a2833ad53ff59c39e6748d) |
| 2021/09/29 | Baolin Wang <baolin.wang@linux.alibaba.com> | [Support hugetlb charge moving at task migration](https://lore.kernel.org/linux-mm/cover.1632843268.git.baolin.wang@linux.alibaba.com) | 当前, 在 hugetlb cgroup 中, 任务的统计信息不会在任务迁移时转移到新的 hugetlb cgroup 中, 这个补丁集增加了 hugetlb cgroup 计费统计迁移. | v1 ☐ | [LORE 0/2](https://lore.kernel.org/linux-mm/cover.1632843268.git.baolin.wang@linux.alibaba.com) |

当前任务试图分配更多的 hugetlb 内存, 而系统已经没有可用的预留空间时, mmap/shmget 会失败. 参见 [Hugetlbfs Reservation](https://www.kernel.org/doc/html/latest/vm/hugetlbfs_reserv.html). 然而, 如果一个任务试图分配的 hugetlb 内存只超过了它的 hugetlb_cgroup 限制, 内核却能 mmap/shmget 成功, 只有在触发 page fault 时, 系统会对该进程发送 SIGBUS 处理. 部分内核开发者对内核这种处理行为表示了不满. 更合理的操作是 hugetlb_cgroup 限制的任务在 mmap/shmget 时就该失败, 而不是当它们尝试使用多余的内存时才触发 SIGBUS.

产生这样问题的根本原因是, hugetlb_cgroup 的统计是在 page fault 的时候(hugetlb memory *fault* time), 而不是在预留的时候(*reservation* time.). 因此, 检查 hugetlb_cgroup 限制只能在 page fault 时进行, 如果发现任务超出限制了则会被 SIGBUS 处理.

建议解决方案是:

1.  一个名为 `hugetlb.xMB.reservation_[limit|usage]_in_bytes` 的新页计数器. 这个计数器的语义与 hugetlb.xMB 稍有不同.

2.  usage_in_bytes 跟踪所有已经发生 page fault 并被实际使用的 hugetlb 内存, reservation_usage_in_bytes 则跟踪所有 reserved 的 hugetlb 内存.

3.  如果一个任务试图保留超过 limit_in_bytes 允许的内存, 内核将允许它这样做. 但是, 如果一个任务试图预留比 reservation_limit_in_bytes 更多的内存, 内核将无法通过这个预留.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/02/11 | Mina Almasry <almasrymina@google.com> | [hugetlb_cgroup: Add hugetlb_cgroup reservation limits](https://lore.kernel.org/linux-mm/cover.1632843268.git.baolin.wang@linux.alibaba.com) | 当前, 在 hugetlb cgroup 中, 任务的统计信息不会在任务迁移时转移到新的 hugetlb cgroup 中, 这个补丁集增加了 hugetlb cgroup 计费统计迁移. | v1 ☐ | 2019/08/08 [PatchWork RFC](https://patchwork.kernel.org/project/linux-mm/patch/20190808194002.226688-1-almasrymina@google.com)<br>*-*-*-*-*-*-*-* <br>2019/08/08 [PatchWork RFC,v2,0/5](https://patchwork.kernel.org/project/linux-mm/cover/20190808231340.53601-1-almasrymina@google.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3,0/6](https://patchwork.kernel.org/project/linux-mm/cover/20190826233240.11524-1-almasrymina@google.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v5,0/7](https://patchwork.kernel.org/project/linux-mm/cover/20190919222421.27408-1-almasrymina@google.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6,1/9](https://patchwork.kernel.org/project/linux-mm/patch/20191013003024.215429-1-almasrymina@google.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v12,1/9](https://patchwork.kernel.org/project/linux-mm/patch/20200211213128.73302-1-almasrymina@google.com) |


### 7.1.6 HugeTLB Migration
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2010/08/10 | Naoya Horiguchi <n-horiguchi@ah.jp.nec.com> | [Hugepage migration](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=86cdb465cf3a9d81058b517af05074157fa9dcdd) | 实现 hugepage 迁移.<br>1. 将 hugepage 迁移函数与原始迁移代码分开, 这是为了避免复杂性.<br>2. 在当前版本中, 定义了一些高级迁移例程来处理 hugepage, 但同时一些基础的辅助函数与原始迁移代码共享, 以避免增加重复.<br>HWPOISION 的部分参见 [HWPOISON for hugepage](https://lwn.net/Articles/387568). | v2 ☑ 2.6.37 | [LORE v2,0/9](https://lore.kernel.org/lkml/1281432464-14833-1-git-send-email-n-horiguchi@ah.jp.nec.com), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=290408d4a25002f099efeee7b6a5778d431154d6) |
| 2013/09/11 | Michal Hocko <mhocko@suse.com> | [extend hugepage migration](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=86cdb465cf3a9d81058b517af05074157fa9dcdd) | 一步扩展 Hugepage 迁移.<br> HugePage 迁移现在仅适用于软脱机 soft offlining(将半损坏页面上的数据移动到另一个页面以保存数据). 但是它对页面迁移的其他用户也很有用, 所以这个补丁集试图扩展一些这样的用户以支持 hugepage.<br>这个补丁集并没有在内存压缩中扩展页面迁移, 因为我认为内存压缩的用户主要希望通过安排原始页面来构建 thp, 但 hugepage 迁移并没有帮助.<br>页面迁移的另一个用户 CMA 可以从 hugepage 迁移中获益, 但现在还未支持它.<br>目前还没有启用 1GB Hugepage 的 Hugepage 迁移, 因为作者不确定 1GB Hugepage 的用户是否真的需要它. | v5 ☑ 3.12-rc1 | [LORE RFC,0/9](https://lore.kernel.org/lkml/1361475708-25991-1-git-send-email-n-horiguchi@ah.jp.nec.coms)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/8](https://lore.kernel.org/lkml/1374183272-10153-1-git-send-email-n-horiguchi@ah.jp.nec.com), [LKML v3,0/8](https://lkml.org/lkml/2013/7/18/542)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/8](https://lore.kernel.org/lkml/1374728103-17468-1-git-send-email-n-horiguchi@ah.jp.nec.com)<br>*-*-*-*-*-*-*-* <br>[LKML v5,0/8](https://lkml.org/lkml/2013/8/9/21)<br>*-*-*-*-*-*-*-* <br>[COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=86cdb465cf3a9d81058b517af05074157fa9dcdd) |
| 2018/09/04 | "Aneesh Kumar K.V" <aneesh.kumar@linux.ibm.com> | [mm/hugetlb: filter out hugetlb pages if HUGEPAGE migration is not](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=464c7ffbcb164b2e5cebfa406b7fc6cdb7945344) | TODO | v1 ☐☑✓ 4.19-rc3 | [LORE](http://lkml.kernel.org/r/20180824063314.21981-1-aneesh.kumar@linux.ibm.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=464c7ffbcb164b2e5cebfa406b7fc6cdb7945344) |


### 7.1.7 HugeTLB reserve & allocations
-------


#### 7.1.7.1 分配 HugeTLB 的常规路径
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/10/07 | Michal Hocko <mhocko@kernel.org> | [mm, hugetlb: allow hugepage allocations to excessively reclaim](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3f36d8669457605910cb7a40089b485949569c41) | NA | v1 ☑✓ 5.4-rc4 | [LORE](https://lore.kernel.org/all/20191007075548.12456-1-mhocko@kernel.org) |


#### 7.1.7.2 HugeTLB from ZONE_MOVABLE
-------

[commit 396faf0303d2 ("Allow huge page allocations to use GFP_HIGH_MOVABLE")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=396faf0303d273219db5d7eb4a2879ad977ed185) 允许在 ZONE_MOVABLE 上进行 hugetlb 的分配. HugeTLB 的页面通常是不能移动的, 所以不会从 ZONE_MOVABLE 中分配. 然而, 由于 ZONE_MOVABLE 区总是有可以迁移或回收的页面, 因此即使系统已经运行了很长时间, 它也有足够的连续内存可以用来满足大页的分配. 因此这允许管理员在运行时根据 ZONE_MOVABLE 的大小调整巨大页面池的大小.

这个补丁添加了一个名为 [hugepages_treat_as_movable](https://elixir.bootlin.com/linux/v2.6.23/source/kernel/sysctl.c#L895) 的新 sysctl. 使能(该接口写入 1)后将来对这个 HugeTLB 内存池的分配将从 ZONE_MOVABLE 区域分配. 这是通过将 HugeTLB 的页面分配 flag 从默认的 GFP_HIGHUSER 修改为 GFP_HIGHUSER_MOVABLE 来完成的. 尽管大页是不可移动的, 但我们不会引入额外的外部内存碎片, 因为大页正是我们所关心的最大的连续内存块分配需求.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/03/01 | Mel Gorman <mel@csn.ul.ie> | [Allow huge page allocations to use GFP_HIGH_MOVABLE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=396faf0303d273219db5d7eb4a2879ad977ed185) | [Create optional ZONE_MOVABLE to partition memory between movable and non-movable pages v2](https://lore.kernel.org/patchwork/patch/75221) 的其中一个补丁, 提供了一个 [sysctl hugepages_treat_as_movable](https://elixir.bootlin.com/linux/v2.6.23/source/mm/hugetlb.c#L31), 允许在 ZONE_MOVABLE 上进行 hugetlb 的分配. 这意味着, 如果页面没有被 mlock 锁定, 那么在系统的生命周期内, 可以将 hugetlb 的页面池调整为 ZONE_MOVABLE 的大小(huge page pool 的尺寸包含了 ZONE_MOVABLE 的尺寸). 尽管大页是不可移动的, 但是内核开发者们认为这不会引入额外的外部内存碎片, 因为大页不正是我们所关心的最大的连续块的分配请求么. | v2 ☑ 2.6.23-rc1 | [PatchWork v2](https://lore.kernel.org/lkml/20070301100802.30048.45045.sendpatchset@skynet.skynet.ie/), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=396faf0303d273219db5d7eb4a2879ad977ed185) |

hugepages_treat_as_movable 的目的是减少内存碎片, 而 hugetlb 页面的寿命一般都很长, 且范围足够大, 因此不会造成碎片,  所以在当时使用这个区域是可以接受的. 但是随着内核不断的发展情况发生了变化, 这个 ZONE_MOVABLE 区域的主要目的变成了保证可迁移性, 从而能够很好的支持虚拟机内存的热插拔. 如果我们允许不可迁移的 hugetlb 页面在 ZONE_MOVABLE 内存中, 热插拔失败的机会会很高. 因此不太合适用一个单独的 sysctl  hugepages_treat_as_movable 来控制 HugeTLB 从 ZONE_MOVABLE 中分配. 更合理的方式应该是去看当前是否支持 [HugeTLB 的迁移](). 内核引入了 hugepage_migration_support(ed) 函数来检查这个功能.

在 [extend hugepage migration](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=86cdb465cf3a9d81058b517af05074157fa9dcdd) 之时, 内核完成了支持 HugeTLB 页面的迁移大部分工作, 已经[准备好了移除 hugepages_treat_as_movable](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=86cdb465cf3a9d81058b517af05074157fa9dcdd). 因此后面直接[移除了 hugepages_treat_as_movable](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d6cb41cc44c63492702281b1d329955ca767d399), 只要内核支持 hugepage_migration_support(ed) 允许对 HugeTLB 的页面进行迁移, 就可以支持从 ZONE_MOVABLE 中迁移.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/01/31 | Michal Hocko <mhocko@suse.com> | [mm, hugetlb: remove hugepages_treat_as_movable sysctl](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=944d9fec8d7aee3f2e16573e9b6a16634b33f403) | 移除 hugepages_treat_as_movable, 不再允许 HugeTLB 从 ZONE_MOVABLE 中分配. 而是[使用 hugepage_migration_support(ed) 来做控制](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=83467efbdb7948146581a56cbd683a22a0684bbb). | RFC ☑ 4.16-rc1 | [LORE RFC](https://lore.kernel.org/all/20171003072619.8654-1-mhocko@kernel.org), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d6cb41cc44c63492702281b1d329955ca767d399) |


但是, HugeTLB 的分配仍然有两个严重的问题:

1. 分配不是 NUMA 感知的. 在 NUMA 机器上, 内核在节点之间平均分配引导时分配的大量页面. 例如, 假设您有一个四节点 NUMA 机器, 并希望在引导时分配四个 1G 的巨大页面. 内核将为每个节点分配一个巨大的页面. 另一方面, 有些用户希望能够指定应该从哪个 NUMA 节点分配巨大的页面. 这样他们就可以在特定的 NUMA 节点上放置虚拟机.

2. 在启动时预留的大页不能被释放. 只有在运行时分配的常规大页面不会有这些问题. 这是因为在 sysfs 中用于运行时分配的 HugeTLB 接口支持 NUMA, 运行时分配的页面可以通过好友分配器释放.

#### 7.1.7.3 HugeTLB CMA(支持运行时分配大页)
-------

HugeTLB 的分配和使用多数情况下是静态的. x86_64 支持 2M 和 1G 的巨页, 但是只有 2M 的巨页可以在运行时分配和释放. 如果想要分配 1G 的巨大页面, 在系统启动阶段通过命令行参数 `hugepagesz =` 和 `hugepages =` 来预留页面完成. 这是因为当前实现下, HugeTLB 子系统在运行时使用伙伴系统来分配器分配大页. 这意味着运行时的大页的大小被限制在 MAX_ORDER order. 对于支持巨页(即 gigantic pages, 页面大小大于 MAX_ORDER) 的对象, 这意味着不能在运行时分配这些页面.


为了克服这个问题, linux 内核在 v3.16 [hugetlb: add support gigantic page allocation at runtime](https://lore.kernel.org/lkml/1397152725-20990-1-git-send-email-lcapitulino@redhat.com) 完成了[在运行时分配 HugeTLB 巨页的支持](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=944d9fec8d7aee3f2e16573e9b6a16634b33f403). 它通过 CMA 而不是伙伴分配器分配巨页来实现这一点. 内核中[把 order 大于 MAX_ORDER 的称为巨页](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=bae7f4ae14d47008a11b4358b167cb0ae186c06a), 通过 [hstate_is_gigantic()](https://elixir.bootlin.com/linux/v3.16/source/include/linux/hugetlb.h#L347) 来判定待分配的请求是不是巨页. 如果是大页的请求则通过 [alloc_fresh_huge_page()](https://elixir.bootlin.com/linux/v3.16/source/mm/hugetlb.c#L1652) 分配, 如果是巨页请求则通过 [alloc_fresh_gigantic_page()](https://elixir.bootlin.com/linux/v3.16/source/mm/hugetlb.c#L1650) 扫描[节点中的所有区域](https://elixir.bootlin.com/linux/v3.16/source/mm/hugetlb.c#L752), 以查找足够大的连续区域. [找到一个满足要求的之后](https://elixir.bootlin.com/linux/v3.16/source/mm/hugetlb.c#L757), 就使用 CMA 进行分配, 即调用 [alloc_contig_range()](https://elixir.bootlin.com/linux/v3.16/source/mm/hugetlb.c#L710) 进行实际分配. 需要注意的是通过 `hugepages =` 命令行选项在引导时分配的巨页也可以在运行时释放.

但是这个方案并不完善, 仍然有些悬而未决的问题, 比如:

*   随着运行时间的推移, 巨页分配所期望的大块连续区域往往会被逐渐消耗, 导致运行时分配巨页出现失败, 作者提到的目前避免这种情况的最好方法是, 在系统启动的早期进行巨大的页面分配. 其他可能的优化包括使用内存规整等 CMA 所支持的手段, 但这个补丁集未明确使用.

*   没有添加对 hugepage over commimit 的支持, 即在 `/proc/sys/vm/nr_overcommit_hugepages > 0` 时按需分配一个巨大的页面. 因为作者不确定在这种情况下分配一个巨页是否合理, 毕竟巨页的分配存在一定的开销.

因为问题 1 的存在, 导致运行时的 HugeTLB 分配机制并没有起到实质的作用. 因为它实际上只在系统加载的早期阶段能真正起作用, 此时大部分内存是空闲的. 一段时间后, 内存被不可移动的页面分割, 已经有了太多的内存外部碎片, 因此找到连续 1GB 块的机会接近于零. 而在大规模情况下, 重新启动服务器以分配巨页是非常昂贵和复杂的. 同时, 解决方案选择也比较困难. 因为即使当前系统中的工作负载没有使用, 那么在预留的 CMA 内存中再保留一定比例的内存用作 HugeTLB 的分配也是一种巨大的浪费: 因为并非所有工作负载都能从使用 1GB 的页面中获益.

随后 v5.7 版本, FaceBook 的开发者 Roman Gushchin [mm: using CMA for 1 GB hugepages allocation](https://lore.kernel.org/all/20200407163840.92263-1-guro@fb.com) 对问题 1 进行妥善地处理. 在启动时通过 [hugetlb_cma_reserve()](https://elixir.bootlin.com/linux/v5.7/source/mm/hugetlb.c#L5562) 预留一个专用的 CMA 区域(通过启动参数 [`hugetlb_cma =`](https://elixir.bootlin.com/linux/v5.7/source/mm/hugetlb.c#L5556) 来指定预留的大小), 运行时使用 CMA 分配器和专用 CMA 区域来分配巨大的页面. 在这种情况下, 可以以很高的概率成功分配巨大的页面, 但是如果没有人使用 1GB 的巨大页面, 内存不会完全浪费. 因为它可以用于页面缓存、匿名内存、THP 等. 这个补丁同时适配了 [x86](https://elixir.bootlin.com/linux/v5.7/source/arch/x86/kernel/setup.c#L1162) 和 [arm64](https://elixir.bootlin.com/linux/v5.7/source/arch/arm64/mm/init.c#L463), 但是当前只支持 4K PAGE_SIZE 的情况. 由于事先不确定会在哪个 NUMA 节点上分配巨页内存, 因此预留时会尝试在[每个 NUMA 节点上都预留](https://elixir.bootlin.com/linux/v5.7/source/mm/hugetlb.c#L5587)一定量的内存, 为 HugeTLB 预留的 CMA 内存专区存储在 [hugetlb_cma](https://elixir.bootlin.com/linux/v5.7/source/mm/hugetlb.c#L49) 数组中, 需要时通过 [cma_alloc()](https://elixir.bootlin.com/linux/v5.7/source/mm/hugetlb.c#L1260) 从预留区域 hugetlb_cma 中分配. 因此在未使用时这块内存完全可以用作他用,

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2014/06/04 | Luiz Capitulino <lcapitulino@redhat.com> | [hugetlb: add support gigantic page allocation at runtime](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=944d9fec8d7aee3f2e16573e9b6a16634b33f403) | 通过在 CMA 区域中分配 HugeTLB 来支持 HugeTLB 的运行时分配. | v1 ☑ 3.16-rc1 | [LORE v3,0/5](https://lore.kernel.org/lkml/1397152725-20990-1-git-send-email-lcapitulino@redhat.com), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=944d9fec8d7aee3f2e16573e9b6a16634b33f403) |
| 2016/02/03 | Vlastimil Babka <vbabka@suse.cz> | [mm, hugetlb: don't require CMA for runtime gigantic pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=080fe2068e1c7f19f565b30b78baf78edf16a980) | commit 944d9fec8d7a ("hugetlb: hugetlb: add support for gigantic page allocation at runtime") 通过 alloc_contig_range() 支持了运行时巨页的分配, 但是这项支持仅在启用 CONFIG_CMA 时可用. 但是其实它不依赖于 MIGRATE_CMA 页面块和相关的基础设施, 只需要一些简单的调整就可以不依赖 CONFIG_CMA, 只需要 CONFIG_MEMORY_ISOLATION 而不是完全的 CONFIG_CMA.<br>在这个补丁之后, 只需要启用了 CONFIG_MEMORY_ISOLATION, alloc_contig_range() 就支持运行时分配巨页, 而无需在页面分配器快速路径中进行特定于 cma 的检查. 注意 CONFIG_CMA slect 了 CONFIG_MEMORY_ISOLATION. | v1 ☑✓ 4.5-rc3 | [LORE](https://lore.kernel.org/all/1454521811-11409-1-git-send-email-vbabka@suse.cz) |
| 2019/03/27 | Alexandre Ghiti <alex@ghiti.fr> | [Fix free/allocation of runtime gigantic pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4eb0716e868eed963967adb0b1b11d9bd8ca1d01) | NA | v8 ☑✓ 5.2-rc1 | [LORE v8,0/4](https://lore.kernel.org/all/20190327063626.18421-1-alex@ghiti.fr) |
| 2020/04/10 | Roman Gushchin <guro@fb.com> | [mm: using CMA for 1 GB hugepages allocation](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=cf11e85fc08cc6a4fe3ac2ba2e610c962bf20bc3) | 引入 hugetlb_cma 参数, 预留一块指定大小的 CMA 区域用来做 HugeTLB 的分配. 对于有多个 NUMA 节点的机器, 每个 NUMA 节点上都会进行预留. | v1 ☑ 5.7-rc1 | [LORE v5,0/2](https://lore.kernel.org/all/20200407163840.92263-1-guro@fb.com) |
| 2020/06/18 | Barry Song <song.bao.hua@hisilicon.com> | [arm64: mm: reserve hugetlb CMA after numa_init](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=618e07865b7453d02410c1f3407c2d78a670eabb) | ARM64 中 HugeTLB CMA 的预留应该在 numa_init 之后. | v1 ☑ 5.8-rc2 | [LORE v3](https://lore.kernel.org/all/20200617215828.25296-1-song.bao.hua@hisilicon.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=618e07865b7453d02410c1f3407c2d78a670eabb) |
| 2020/07/01 | Roman Gushchin <guro@fb.com> | [arm64/hugetlb: Reserve CMA areas for gigantic pages on 16K and 64K configs](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=abb7962adc80ab4f4313e8a065302525b6a9c2dc) | ARM64 下不光有[多种 PAGE_SIZE, 还有各类 CONT PAGE](https://elixir.bootlin.com/linux/v5.9/source/arch/arm64/mm/hugetlbpage.c#L25) 的情况. HugeTLB CMA 当前不支持 ARM64 下其他 PAGE_SIZE 情况下的预留, 因此引入 arm64_hugetlb_cma_reserve() 增加了 ARM64 下多种 PAGE_SIZE 的支持. | v1 ☑ 5.9-rc1 | [LORE v3](https://lore.kernel.org/all/1593578521-24672-1-git-send-email-anshuman.khandual@arm.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=abb7962adc80ab4f4313e8a065302525b6a9c2dc) |
| 2020/07/13 | Roman Gushchin <guro@fb.com> | [powerpc/hugetlb/cma: Allocate gigantic hugetlb pages using CMA](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ef26b76d1af61b90eb0dd3da58ad4f97d8e028f8) | POWERPC 支持从 CMA 中分配 HugeTLB. | v5 ☑ 5.9-rc1 | [LORE v3](https://lore.kernel.org/all/20200407163840.92263-1-guro@fb.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ef26b76d1af61b90eb0dd3da58ad4f97d8e028f8) |
| 2022/04/13 | liupeng (DM) <liupeng256@huawei.com> | [hugetlb: Fix some incorrect behavior](https://patchwork.kernel.org/project/linux-mm/cover/20220413032915.251254-1-liupeng256@huawei.com/) | 631696 | v3 ☐☑ | [LORE v3,0/4](https://lore.kernel.org/r/20220413032915.251254-1-liupeng256@huawei.com) |
| 2022/04/13 | Anshuman Khandual <anshuman.khandual@arm.com> | [tlb/hugetlb: Add framework to handle PGDIR_SIZE HugeTLB pages](https://patchwork.kernel.org/project/linux-mm/patch/20220413100714.509888-1-anshuman.khandual@arm.com) | 改变 tlb_remove_huge_tlb_entry() 来适应更大的 PGDIR_SIZE HugeTLB 页面, 通过添加一个新的助手 tlb_flush_pgd_range(). 而这里也更新 struct mmu_gather 需要, 即添加一个新成员 cleared_pgds. | v1 ☐☑ | [LORE v1](https://lore.kernel.org/r/20220413100714.509888-1-anshuman.khandual@arm.com) |


#### 7.1.7.4 Numa Aware HugeTLB reserve
-------

HugeTLB CMA 在设计的时候, 已经考虑了 NUMA 的存在.

1.  在 [mm: using CMA for 1 GB hugepages allocation](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=cf11e85fc08cc6a4fe3ac2ba2e610c962bf20bc3) 引入 hugetlb_cma 参数的过程中, Roman Gushchin 发现 CMA 并没有一个在指定 NUMA 节点上预留和分配内存的能力, 因此 [mm: cma: NUMA node interface](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8676af1ff2d28e64e5636147821bda7524cf007d) 增加了尝试在特定节点上[预留](https://elixir.bootlin.com/linux/v5.7/source/include/linux/cma.h#L33)和分配连续内存的能力. 注意如果指定的节点无法分配, 它将回退到其他节点进行分配.

2.  此外 [hugetlb: add support for gigantic page allocation at runtime](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=944d9fec8d7aee3f2e16573e9b6a16634b33f403) 在运行时动态分配 HugeTLB 内存时, 可以在指定节点下 `/sys/devices/system/node/node1/hugepages/hugepages-1048576kB/nr_hugepages` 的 HugeTLB 接口中进行动态分配.

随后内核开发者考虑预留 HugeTLB 空间时也可以精细化控制不同 NUMA NODE 上预留的空间, 通过扩展 HugeTLB 的启动参数解析工作, 增加各个节点上的精细化预留.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/10/05 | Zhenguo Yao <yaozhenguo1@gmail.com> | [hugetlbfs: Extend the definition of hugepages parameter to support node allocation](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=b5389086ad7be0453c55e0069a89856d1fbdf605) | 当前内核允许指定启动时要分配的 hugepages 数目. 但目前所有节点的 hugepages 都是平衡的. 在某些场景中, 我们只需要在一个节点中使用 hugepags.<br>例如: DPDK 需要与 NIC 位于同一节点的 hugepages. 如果 DPDK 在 node1 中需要 4 个 1G 大小的 HugePage, 并且系统有 16 个 numa 节点. 我们必须在内核 cmdline 中保留 64 个 HugePage. 但是, 只使用了 4 个 hugepages. 其他的应该在引导后释放. 如果系统内存不足(例如: 64G), 这将是一项不可能完成的任务. 因此, 添加 hugepages_node 内核参数以指定启动时要分配的 hugepages 的节点数. | v1 ☑ 5.16-rc1 | [2021/08/20 PatchWork RFC](https://patchwork.kernel.org/project/linux-mm/patch/20210820030536.25737-1-yaozhenguo1@gmail.com)<br>*-*-*-*-*-*-*-* <br>[2021/08/23 PatchWork v1]](https://patchwork.kernel.org/project/linux-mm/patch/20210823130154.75070-1-yaozhenguo1@gmail.com)<br>*-*-*-*-*-*-*-* <br>[2021/10/05 PatchWork  v8](https://patchwork.kernel.org/project/linux-mm/patch/20211005054729.86457-1-yaozhenguo1@gmail.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b5389086ad7be0453c55e0069a89856d1fbdf605) |
| 2021/10/15 | Baolin Wang <baolin.wang@linux.alibaba.com> | [hugetlb: Support node specified when using cma for gigantic hugepages](https://patchwork.kernel.org/project/linux-mm/patch/bb790775ca60bb8f4b26956bb3f6988f74e075c7.1634261144.git.baolin.wang@linux.alibaba.com) | 所有在线节点的 hugepages 运行时分配的 CMA 区域大小是平衡的, 但我们还希望指定每个节点的 CMA 大小, 或者在某些情况下仅指定一个节点, 这与 [hugetlb 的分 numa 节点指定大小](https://patchwork.kernel.org/project/linux-mm/patch/20211005054729.86457-1-yaozhenguo1@gmail.com)类似. | v3 ☐ | [2021/10/10 PatchWork v1](https://patchwork.kernel.org/project/linux-mm/patch/bb790775ca60bb8f4b26956bb3f6988f74e075c7.1634261144.git.baolin.wang@linux.alibaba.com)<br>*-*-*-*-*-*-*-* <br>[2021/10/15 PatchWork v3](https://patchwork.kernel.org/project/linux-mm/patch/bb790775ca60bb8f4b26956bb3f6988f74e075c7.1634261144.git.baolin.wang@linux.alibaba.com) |
| 2021/11/17 | Mina Almasry <almasrymina@google.com> | [hugetlb: Add `hugetlb.*.numa_stat` file](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f47761999052b1cc987dd3e3d3adf47997358fc0) | 添加 `hugetlb.*.numa_stat`, 它显示 hugetlb 在 cgroup 的 numa 使用信息. 类似于 memory.numa_stat. | v8 ☑ 5.17-rc1 | [PatchWork v1](https://patchwork.kernel.org/project/linux-mm/patch/20211019215437.2348421-1-almasrymina@google.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://patchwork.kernel.org/project/linux-mm/patch/20211020190952.2658759-1-almasrymina@google.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v7](https://patchwork.kernel.org/project/linux-mm/patch/20211117201825.429650-1-almasrymina@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v8](https://lore.kernel.org/all/20211123001020.4083653-1-almasrymina@google.com) |


### 7.1.8 代码段大页
-------

由于应用程序的代码段 text 和数据段 data 都是直接映射到程序二进制文件本身的, 因此如果不通过修改程序运行时库等方式, 很难把这些段使用 HugeTLB 来映射. 但是将这些段映射成大页可以有效地减少 iTLB-miss. 网上也经常会看到相关的一些讨论. 参见 [stackoverflow: usage of hugepages for text and data segments](https://stackoverflow.com/questions/36576462/usage-of-hugepages-for-text-and-data-segments).

Google 的工程师 Mina Almasry 提出了一种新的思路, 通过 [mremap 的方式重新映射程序的的 ELF 到大页上](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=550a7d60bd5e35a56942dba6d8a26752beb26c9f), 来支持代码段大页. 作者提供了一个用户态工具库 [chromium/hugepage_text](https://chromium.googlesource.com/chromium/src/+/refs/heads/main/chromeos/hugepage_text) 来辅助完成这项工作. 可以在尽可能不修改应用程序代码的情况下完成代码段大页的映射, 可以显著提升应用程序的性能. 具体使用也可以参考作者提交的测试用例 [commit 12b613206474 ("mm, hugepages: add hugetlb vma mremap() test")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=12b613206474cea36671d6e3a7be7d1db7eb8741).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/10/14 | Mina Almasry <almasrymina@google.com> | [mm, hugepages: add mremap() support for hugepage backed vma](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=12b613206474cea36671d6e3a7be7d1db7eb8741) | 通过简单地重新定位页表项, 使得 mremap() 支持 hugepage 的 vma 段. 页表条目被重新定位到 mremap() 上的新虚拟地址.<br>作者验证的测试场景是一个简单的 bench: 它在 hugepages 中重新加载可执行文件的 ELF 文本, 这大大提高了上述可执行文件的执行性能.<br>将 hugepages 上的 mremap 操作限制为原始映射的大小, 因为底层 hugetlb 保留还不能处理到更大的大小的重映射.<br>在 mremap () 操作期间, 我们检测 pmd_shared 的映射, 并在 mremap () 期间取消这些映射的共享. 在访问和故障时, 再次建立共享. | v1 ☑ 5.16-rc1 | [PatchWork v1](https://patchwork.kernel.org/project/linux-mm/patch/20210730221522.524256-1-almasrymina@google.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v4,1/2](https://patchwork.kernel.org/project/linux-mm/patch/20211006194515.423539-1-almasrymina@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v7,1/2](https://lore.kernel.org/all/20211013195825.3058275-1-almasrymina@google.com), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=550a7d60bd5e35a56942dba6d8a26752beb26c9f)<br>*-*-*-*-*-*-*-* <br>[PatchWork v8,1/2](https://patchwork.kernel.org/project/linux-mm/patch/20211014200542.4126947-1-almasrymina@google.com) |
| 2022/02/02 | Mike Kravetz <mike.kravetz@oracle.com> | [Add hugetlb MADV_DONTNEED support](https://patchwork.kernel.org/project/linux-mm/cover/20220128222605.66828-1-mike.kravetz@oracle.com/) | 609660 | v1 ☐☑ | [PatchWork v1,0/3](https://lore.kernel.org/all/20220128222605.66828-1-mike.kravetz@oracle.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/3](https://lore.kernel.org/r/20220202014034.182008-1-mike.kravetz@oracle.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/3](https://lore.kernel.org/r/20220215002348.128823-1-mike.kravetz@oracle.com) |


### 7.1.9 HugeTLBFS
--------


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2015/06/22 | Mike Kravetz <mike.kravetz@oracle.com> | [hugetlbfs: add fallocate support](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log?id=72079ba0dfefc1444b4ef98a2fa3d040838a775f) | 为 HugeTLBFS 增加了 fallocate 功能. 目前, Hugetlbfs 用于那些想要对巨大页面使用进行高度控制的应用程序. 通常, 大型 HugeTLBFS 文件用于将大量巨面映射到应用程序进程中. 应用程序知道何时不再使用这些大文件中的页面范围, 理想情况下希望将它们释放回子池或全局池以作其他用途. fallocate() 系统调用提供了一个用于预分配和在文件中打孔的接口. 这是基于 shmem 版本的, 但它有相当大的分歧. 我们既不用担心交换, 也不用担心新文件的密封. 这允许我们在 HugeTLBFS 文件中移动物理内存, 而不需要映射它. 这也使我们能够支持 MADV_REMOVE, 因为它目前是使用 fallocate () 实现的. MADV_REMOVE 允许我们从 HugeTLBFS 文件中间删除数据, 这在以前是不可能的. | v5 ☐☑✓ | [LORE RFC,0/4](https://lore.kernel.org/lkml/1429225378-22965-1-git-send-email-mike.kravetz@oracle.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,00/10](https://lore.kernel.org/lkml/1437502184-14269-1-git-send-email-mike.kravetz@oracle.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/9](https://lore.kernel.org/all/1435019919-29225-1-git-send-email-mike.kravetz@oracle.com) |
| 2022/06/13 | Mike Kravetz <mike.kravetz@oracle.com> | [hugeTLBfs: zero partial pages during fallocate hole punch](https://patchwork.kernel.org/project/linux-mm/patch/20220613180858.15933-1-mike.kravetz@oracle.com/) | NA | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220613180858.15933-1-mike.kravetz@oracle.com) |


### 7.1.x More HugeTLB Patchset
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/12/22 | Liang Li <liliang.opensource@gmail.com> | [add support for free hugepage reporting](https://lore.kernel.org/patchwork/patch/1355899) | Free page reporting 只支持伙伴系统中的页面, 它不能报告为 hugetlbfs 预留的页面. 这个补丁在 hugetlb 的空闲列表中增加了对报告巨大页的支持, 它可以被 virtio_balloon 驱动程序用于内存过载和预归零空闲页, 以加速内存填充和页面错误处理. | RFC ☐ | [PatchWork RFC,0/3](https://patchwork.kernel.org/project/linux-mm/cover/20201222074538.GA30029@open-light-1.localdomain) |
| 2021/10/07 | Mike Kravetz <mike.kravetz@oracle.com> | [hugetlb: add demote/split page functionality](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=8531fc6f52f5fc201f43d8c36c2606e25748b1c4) | 实现了 hugetlb 降低策略. 提供了一种"就地"将 hugetlb 页面分割为较小的页面的方法. | v4 ☑ 5.16-rc1 | [2021/07/21 PatchWork 0/8](https://patchwork.kernel.org/project/linux-mm/cover/20210721230511.201823-1-mike.kravetz@oracle.com)<br>*-*-*-*-*-*-*-* <br>[2021/08/16 PatchWork RESEND,0/8](https://patchwork.kernel.org/project/linux-mm/cover/20210816224953.157796-1-mike.kravetz@oracle.com)<br>*-*-*-*-*-*-*-* <br>[2021/09/23 PatchWork v2,0/4](https://patchwork.kernel.org/project/linux-mm/cover/20210923175347.10727-1-mike.kravetz@oracle.com)<br>*-*-*-*-*-*-*-* <br>[2021/10/07 PatchWork v4,0/5](https://patchwork.kernel.org/project/linux-mm/cover/20211007181918.136982-1-mike.kravetz@oracle.com) |
| 2022/02/01 | Anshuman Khandual <anshuman.khandual@arm.com> | [mm/hugetlb: Generalize ARCH_WANT_GENERAL_HUGETLB](https://patchwork.kernel.org/project/linux-mm/patch/1643718465-4324-1-git-send-email-anshuman.khandual@arm.com/) | 610328 |v1 ☐☑ | [PatchWork v1,0/1](https://lore.kernel.org/r/1643718465-4324-1-git-send-email-anshuman.khandual@arm.com) |
| 2022/03/07 | Mike Kravetz <mike.kravetz@oracle.com> | [hugetlb: do not demote poisoned hugetlb pages](https://patchwork.kernel.org/project/linux-mm/patch/20220307215707.50916-1-mike.kravetz@oracle.com/) | 621194 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220307215707.50916-1-mike.kravetz@oracle.com) |
| 2022/03/18 | Muchun Song <songmuchun@bytedance.com> | [add hugetlb_free_vmemmap sysctl](https://patchwork.kernel.org/project/linux-mm/cover/20220307130708.58771-1-songmuchun@bytedance.com/) | 621009 | v3 ☐☑ | [2022/03/07 LORE v3,0/4](https://lore.kernel.org/r/20220307130708.58771-1-songmuchun@bytedance.com)<br>*-*-*-*-*-*-*-* <br>[2022/03/18 LORE v4,0/4](https://lore.kernel.org/r/20220318100720.14524-1-songmuchun@bytedance.com) |
| 2022/04/12 | Muchun Song <songmuchun@bytedance.com> | [add hugetlb_optimize_vmemmap sysctl](https://patchwork.kernel.org/project/linux-mm/cover/20220412111434.96498-1-songmuchun@bytedance.com/) | 631440 | v7 ☐☑ | [LORE v7,0/4](https://lore.kernel.org/r/20220412111434.96498-1-songmuchun@bytedance.com)<br>*-*-*-*-*-*-*-* <br>[LORE v8,0/4](https://lore.kernel.org/r/20220413144748.84106-1-songmuchun@bytedance.com)<br>*-*-*-*-*-*-*-* <br>[LORE v9,0/4](https://lore.kernel.org/r/20220429121816.37541-1-songmuchun@bytedance.com)<br>*-*-*-*-*-*-*-* <br>[LORE v10,0/4](https://lore.kernel.org/r/20220509062703.64249-1-songmuchun@bytedance.com) |
| 2013/03/20 | David Rientjes <rientjes@google.com> | [mm, hugetlb: include hugepages in meminfo](https://lore.kernel.org/all/alpine.DEB.2.02.1303201207110.28997@chino.kir.corp.google.com) | alpine.DEB.2.02.1303201207110.28997@chino.kir.corp.google.com | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/alpine.DEB.2.02.1303201207110.28997@chino.kir.corp.google.com) |
| 2022/04/15 | Christophe Leroy <christophe.leroy@csgroup.eu> | [mm, hugetlbfs: Allow for "high" userspace addresses](https://patchwork.kernel.org/project/linux-mm/patch/ab847b6edb197bffdfe189e70fb4ac76bfe79e0d.1650033747.git.christophe.leroy@csgroup.eu/) | 632590 | v10 ☐☑ | [LORE v10](https://lore.kernel.org/r/ab847b6edb197bffdfe189e70fb4ac76bfe79e0d.1650033747.git.christophe.leroy@csgroup.eu) |
| 2022/05/08 | Mike Kravetz <mike.kravetz@oracle.com> | [hugetlb: Change huge pmd sharing synchronization again](https://patchwork.kernel.org/project/linux-mm/cover/20220420223753.386645-1-mike.kravetz@oracle.com/) | 之前 [v5.7 中添加了](https://lore.kernel.org/linux-mm/43faf292-245b-5db5-cce9-369d8fb6bd21@infradead.org) commit c0d0381ade79 ("hugetlbfs: use i_mmap_rwsem for more pmd sharing synchronization") 时, 社区就[报告了性能回归](https://lore.kernel.org/lkml/20200622005551.GK5535@shao2-debian). 当时,  Mike Kravetz 提出了一个[解决回归的建议](https://lore.kernel.org/linux-mm/20200706202615.32111-1-mike.kravetz@oracle.com), 但没有被合入. 这个补丁集使用一个新的 hugetlb 特定的 per-vma rw 信号量来同步 PMD 共享. 为了更好的演示这个性能回归, 作者构造了一个简单的测试用例. | v2 ☐☑ | [2022/04/20 LORE v2,0/6](https://lore.kernel.org/r/20220420223753.386645-1-mike.kravetz@oracle.com)<br>*-*-*-*-*-*-*-* <br>[2022/05/08 LORE v3,0/8](https://lore.kernel.org/r/20220508183420.18488-1-mike.kravetz@oracle.com) |
| 2022/05/27 | Mike Kravetz <mike.kravetz@oracle.com> | [hugetlb: speed up linear address scanning](https://patchwork.kernel.org/project/linux-mm/cover/20220527225849.284839-1-mike.kravetz@oracle.com/) | 645704 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/20220527225849.284839-1-mike.kravetz@oracle.com)<br>*-*-*-*-*-*-*-*<br>[LORE v1,0/4](https://lore.kernel.org/r/20220616210518.125287-1-mike.kravetz@oracle.com)<br>*-*-*-*-*-*-*-*<br>[[LORE v2,0/4](https://lore.kernel.org/r/20220621235620.291305-1-mike.kravetz@oracle.com) |
| 2022/06/24 | James Houghton <jthoughton@google.com> | [hugetlb: Introduce HugeTLB high-granularity mapping](https://patchwork.kernel.org/project/linux-mm/cover/20220624173656.2033256-1-jthoughton@google.com/) | 引入了HugeTLB高粒度映射(HGM)(曾经叫做 HugeTLB double mapping)的概念. 高粒度映射不需要分解大页面本身;它只影响它们的映射方式. 从广义上讲, 它主要探索如何在不同粒度上映射 HugeTLB 页面, 更重要的是, 部分映射 HugeTLB 页面. | v1 ☐☑ | [LORE v1,0/26](https://lore.kernel.org/r/20220624173656.2033256-1-jthoughton@google.com) |
| 2022/06/28 | Muchun Song <songmuchun@bytedance.com> | [Simplify hugetlb vmemmap and improve its readability](https://patchwork.kernel.org/project/linux-mm/cover/20220628092235.91270-1-songmuchun@bytedance.com/) | 654499 | v2 ☐☑ | [LORE v2,0/8](https://lore.kernel.org/r/20220628092235.91270-1-songmuchun@bytedance.com) |


## 7.2 透明大页的支持
-------

hugetlb 的使用依赖于用户主动预留并使用, 适用于用户明确需要使用大量连续大块页面或者使用大页可以带来明显性能提升的场景. 但是多数情况下, 开发者并不知道自己是否需要 huge page. 这时候如果内核通过某种机制自动把普通的页面合并为 huge page, 或者在用户分配时就直接分配成 huge page, 让用户丝毫不感知 huge page 的存在, 但是却能享受到 huge page 带来的性能提升. 那自然是极好的. 鉴于此 THP(Transparent huge page) 孕育而生.

因为该过程不会被用户和应用感知, 所以被称为 "transparent", 也被称为动态大页. 与之相对的, 之前的 hugetlb 则被称为静态大页(static huge page), 那么自然. 一个是动态的, 一个是静态的. 两者的关系, 就类似于 CMA 和预留 DMA 的关系.

**2.6.38(2011年3月发布)**

| LWN 归档 | Index entries for this article |
|:--------:|:------------------------------:|
| 0 | [Huge pages](https://lwn.net/Kernel/Index/#Huge_pages) |
| 1 | [Memory management/Huge pages](https://lwn.net/Kernel/Index/#Memory_management-Huge_pages) |

扩展阅读:

[卢钧轶: Huge Page 是否是拯救性能的万能良药？](https://www.cnblogs.com/cenalulu/p/4394695.html)

### 7.2.1 Transparent Hugepage Support
-------

2011 年 v2.6.38 期间 Andrea Arcangeli 为 linux 引入了 THP(Transparent huge page), 在应用需要 huge page 的时候, 可通过内存规整等操作, 为当前 VMA 分配一个 huge page, 因为该过程不会被应用感知到, 所以被称为 "transparent". 参见 [Transparent huge pages in 2.6.38](https://lwn.net/Articles/423584).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2010/12/15 | Andrea Arcangeli <aarcange@redhat.com> | [Transparent Hugepage Support #33](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=a664b2d8555c659127bf8fe049a58449d394a707) | 实现透明大页. | v33 ☑ [2.6.38-rc1](https://kernelnewbies.org/Linux_2_6_38#Transparent_huge_pages) | [LORE](https://lore.kernel.org/lkml/20101215051540.GP5638@random.random), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=71e3aac0724ffe8918992d76acfe3aad7d872) |

1.      当发生 page fault 时, 内核将尝试使用 [do_huge_pmd_anonymous_page()](https://elixir.bootlin.com/linux/v2.6.38/source/mm/memory.c#L3305) 分配一个 huge page 来满足它. 如果分配成功, huge page 的页表就会被设置, 而该地址范围内的[任何现有的普通页面都将被释放](https://elixir.bootlin.com/linux/v2.6.38/source/mm/memory.c#L3313), 同时将 huge page 插入到进程 VMA 中. 如果没有 huge page 可用, 内核就会通过 [do_huge_pmd_wp_page_fallback()](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L910) 退回到普通页面, 应用程序永远不会感知到这些差异. [commit ("71e3aac0724f thp: transparent hugepage core")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=71e3aac0724ffe8918992d76acfe3aad7d872).

2.      但是分配时能够分配出 huge page 取决于物理上连续的大的内存块, 这是 Linux 内核程序员永远不能指望的. 因此在一个进程出现若干次小页面的 pge fault 之后, THP 补丁试图通过添加 "khugepaged" 内核线程来改善这种情况. 这个线程偶尔会尝试分配一个巨大的页面; 如果成功, 它会扫描内存, 寻找一个巨大的页面可以替代一堆较小的页面的位置. 因此, 可用的巨大页面应该快速放置到服务中, 最大限度地利用整个系统中的 hugepage. 内核还提供了一整套参数控制 khugepaged 线程的操作. 参见 [commit ("ba76149f47d8 thp: khugepaged")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ba76149f47d8).

3.      同时引入了 defrag 功能, 控制内核是否应该积极地使用内存规整/压缩(compaction) 以使更多的 hugepage 可用. 内存规整通过移动页面, 以创建空闲内存中大的、物理上连续的区域. 这些区域通常可以用来支持大量的分配, 对于创建 huge page 来说特别有用.


最早的实现只能用于匿名页面, 将 huge page 与 page cache 集成的工作尚未完成. 它也只处理一个页面大小为(2 MB) 的 PMD level 的大页. 当时 Mel Gorman 运行了一些[基准测试](https://lwn.net/Articles/423590), 在某些情况下可以提高 10% 左右. 一般来说, 结果不如 hugetlbfs 那样好, 但是 THP 更有可能被普遍的运用在生产环境中.


7.1 介绍的这种使用大页的方式看起来是挺怪异的, 需要用户程序做出很多修改. 而且, 内部实现中, 也需要系统预留一大部分内存. 基于此, 2.6.38 引入了一种叫[透明大页 Transparent huge pages](https://lwn.net/Articles/423584) 的实现. 如其名字所示, 这种大页的支持对用户程序来说是透明的.



它的实现原理如下. 在缺页中断中, 内核会**尝试**分配一个大页, 如果失败(比如找不到这么大一片连续物理页面), 就还是回退到之前的做法: 分配一个小页. 在系统内存紧张需要交换出页面时, 由于前面所说的根植内核的4KB页面大小的因, MM 会透明地把大页分割成小页再交换出去.



用户态程序现在可以完成无修改就使用大页支持了. 用户还可以通过 **madvice()** 系统调用给予内核指示, 优化内核对大页的使用. 比如, 指示内核告知其你希望进程空间的某部分要使用大页支持, 内核会尽可能地满足你.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/09/28 | Zi Yan <ziy@nvidia.com> | [1GB PUD THP support on x86_64](https://lkml.org/lkml/2020/9/28/973) | X86_64 支持 PUD 级别(1G)的匿名大页 | RFC,v2 ☐ | [2020/09/02 PatchWork RFC,00/16](https://patchwork.kernel.org/project/linux-mm/cover/20200902180628.4052244-1-zi.yan@sent.com)<br>*-*-*-*-*-*-*-* <br>[2020/09/28 PatchWork v2 00/30](https://patchwork.kernel.org/project/linux-mm/cover/20200928175428.4110504-1-zi.yan@sent.com) |
| 2021/05/10 | Muchun Song <songmuchun@bytedance.com> | [Overhaul multi-page lookups for THP](https://lore.kernel.org/patchwork/patch/1337675) | 提升大量页面查找时的效率 | v4 ☑ [5.12-rc1](https://kernelnewbies.org/Linux_5.12#Memory_management) | [PatchWork RFC](https://patchwork.kernel.org/project/linux-mm/cover/20201112212641.27837-1-willy@infradead.org) |
| 2021/05/10 | Ankur Arora <ankur.a.arora@oracle.com> | [Use uncached stores while clearing huge pages](https://patchwork.kernel.org/project/linux-mm/cover/20211020170305.376118-1-ankur.a.arora@oracle.com) | 本系列增加了对大页的非缓存页面清除的支持. [清除大页内存](https://patchwork.kernel.org/project/linux-mm/patch/20211020170305.376118-11-ankur.a.arora@oracle.com)时, 使用基于 [MOVNTI 指令](https://www.felixcloutier.com/x86/movnti) 的 [uncached clear page 接口](https://patchwork.kernel.org/project/linux-mm/patch/20211020170305.376118-4-ankur.a.arora@oracle.com).<br>其动机是加快大型预分配虚拟机的创建, 并支持巨大的页面.<br>支持非缓存页面清除有两种帮助:<br>1. 对于小于 LLC 大小的数据块, 未缓存的存储通常比缓存的存储慢, 而对于较大的数据块, 则更快.2. 避免用无用的零替换潜在有用的缓存行.<br>性能测试: 虚拟机创建(对于预分配 2MB 后台页面的虚拟机)在运行时有了显著的改进. | v2 ☐ | [PatchWork v2,00/14](https://patchwork.kernel.org/project/linux-mm/cover/20211020170305.376118-1-ankur.a.arora@oracle.com) |

THP 虽然实现了, 但是依旧存在着不少问题. 在 LSFMM 2015 进行了讨论, 参见 [Improving huge page handling](https://lwn.net/Articles/636162)

1.  Kirill Shutemov 首先描述了他提出的如何处理 THP 的引用计数的改变, 这个补丁集在 LWN 2014 年 11 月的文章 [Transparent huge page reference counting](https://lwn.net/Articles/619738) 中有详细的描述. 该补丁的关键部分是, 它允许在 PMD (巨大页面)和 PTE (常规页面)模式下同时映射一个巨大的页面. 正如作者所言, 这是一个大的补丁集, 而且仍然存在一些错误, 所以这组补丁并没有被合入主线. 参见 [THP 和 mapcount 之间的恩恩怨怨](https://richardweiyang-2.gitbook.io/kernel-exploring/00-index/02-thp_mapcount).

2.  剩下的一个问题是关于部分取消巨大页面的映射. 当一个进程取消映射一个 huge page 的一部分页面时, 有多种想法可以执行这种操作.

| 分割行为 | 描述 |
|:------:|:----:|
| 直接分割 | 将该页面拆分, 并将与释放的区域对应的单个页面返回给系统. |
| 延迟分割 | 也可以将这个 huge page 拆解成普通页面, 但是并不释放它们, 而是内核仍然维护组成这个 huge page 的普通页面, 将它们保持在一起. 这使得 huge page 在需要时允许它快速重新构建. 但是, 这也意味着没有任何内存实际上将被释放, 同时有必要将这个 huge page 及组成它的普通页面添加到一个特殊的列表中, 如果系统遇到内存压力, 可以在该列表中真正地进行分割. |


### 7.2.2 THP reference counting
-------

首先来看 THP 的引用计数的问题, 参见:

[Transparent huge page reference counting](https://lwn.net/Articles/619738)

[THP 和 mapcount 之间的恩恩怨怨](https://richardweiyang-2.gitbook.io/kernel-exploring/00-index/02-thp_mapcount).


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/10/06 | Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [THP refcounting redesign](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ddc58f27f9ee) | NA | v12 ☑ 4.5-rc1 | [LWN RFC 00/10](https://lwn.net/Articles/601781)<br>*-*-*-*-*-*-*-* <br>[LORE RFC v2,00/19](https://lore.kernel.org/lkml/1415198994-15252-1-git-send-email-kirill.shutemov@linux.intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE RFC v12,00/37](https://lore.kernel.org/lkml/1444145044-72349-1-git-send-email-kirill.shutemov@linux.intel.com) |
| 2016/05/06 | Andrea Arcangeli <aarcange@redhat.com> | [mm: thp: mapcount updates](https://lore.kernel.org/all/1462547040-1737-1-git-send-email-aarcange@redhat.com) | NA | v1 ☑ 4.6 | [LORE 0/3](https://lore.kernel.org/all/1462547040-1737-1-git-send-email-aarcange@redhat.com), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6d0a07edd17c) |

### 7.2.3 THP allocations latencies
-------


LSFMM 2015 讨论了 THP 的诸多问题, 参见 [Improving huge page handling](https://lwn.net/Articles/636162)

最关键的问题在于, 我们分配出的 huge page 是否能带来性能的提升, 不管是处理 page fault 时分配出 huge page, 还是通过 khugepaged 合并出 huge page, 以及通过内存规整整出 huge page. 都是存在开销的. 首先在处理页面 page fault 上进行相当激进的 THP 分配尝试是否是一种良好的性能权衡. 参见 [LSF/MM TOPIC ATTEND](https://marc.info/?l=linux-mm&m=142056088232484&w=2)

    *   THP 分配增加了页面错误延迟, 因为高阶分配是出了名的昂贵. 页面分配 slowpath 现在会对 gfp_transshuge && !PF_KTHREAD 进行额外的检查, 以避免对用户页面错误进行更昂贵的同步压缩. 但即使是异步压缩也可能代价高昂.

    *   在 2MB 范围内的第一个 page fault 期间, 我们无法预测该范围内有多少实际将被访问, 理论上我们可以浪费[多达 511 个页面](https://www.spinics.net/lists/kernel/msg1928252.html). 或者, 范围内的页面可以从不同 NUMA 节点的 CPU 访问, 虽然基本页面可以都是本地的, 但 THP 可以远程到除一个 CPU 以外的所有 CPU. 由于这个错误的共享, 远程访问的成本将高于 huge page 省出来的 TLB 的开销.

因此很多时候, THP 的效果可能远不如想象中那么美好. 比如 SAP 建议为他们的应用程序[禁用 THPs](https://blogs.sap.com/2014/05/22/sap-iq-and-linux-hugepagestransparent-hugepages), 以提高性能.


#### 7.2.3.1 Improving huge page handling
-------

在 page fault 时分配 huge page 会带来较大的延迟.

Vlastimil 在 [LSFMM 2015 对 THP 的讨论中](https://lwn.net/Articles/636162) 提到, 由于不可能预测在 page fault 时提供 huge page 的好处, 所以最好少做一些. 相反, 应该主要在 khugepaged 守护进程中创建透明的巨大页面, 该守护进程可以在后台查看内存使用情况和折叠页面. 这样做需要重新设计 khugepaged, 这主要是为了在其他方法失败时填充巨大的页面. 但是它扫描速度很慢, 不能确定一个进程是否会从巨大的页面中受益; 特别是, 它不知道该进程是否会花费大量时间运行. 这可能是因为这个过程大多潜伏着等待外部事件的发生, 也可能是因为它即将退出.

Vlastimil 方法是通过将寻找 huge page 机会的扫描工作移动到进程上下文中来改进 khugepaged. 在某些时候, 例如从系统调用返回时, 每个进程都会扫描一部分内存, 或许还会将某些页折叠成 huge page. 它会部分基于成功率自动调整自身, 但也仅仅基于一个进程运行更频繁将做更多的扫描这一事实. 因为不涉及守护进程, 所以不会有额外的唤醒; 如果一个系统完全空闲, 就不会有页面扫描. 不过在 khugepaged 中折叠页面比在 page fault 时分配 huge page 要昂贵得多.

之前的想法太激进了, Andrea 等人有不同意见.

*   Andrea 认为在 khugepaged 中折叠页面比在 page fault 时分配 huge page 要昂贵得多. 要折叠一个页面, 内核必须将所有单独的小页面迁移(复制)到包含它们的新的巨大页面, 这需要花费较多的时间. 在需要之前, 进程上下文扫描可能有地方创建 huge page, 所以最好尽可能避免折叠页面.

*   Andi Kleen 认为, 在进程上下文中运行内存规整是一个糟糕的主意, 它会夺走并行性的机会. 规整扫描就应该在守护进程中完成, 这样它就可以在单独的核心上运行; 否则就会为受影响的进程创建过多的延迟.


Andrea 建议将工作放入工作队列中.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2014/10/22 |  Alex Thorlton | [Convert khugepaged to a task_work function](https://lwn.net/Articles/634384) | 将 huge page 的折叠扫描从 khugepaged 移动到 task_work 上下文. 在这个实现中, 扫描是由 `__khugepaged_enter()` 驱动的, 它在 vma_merge、fork 和 THP page fault 的尝试等事件中被调用, 例如不是完全周期性的事件. | RFC ☐ | [LKML 0/4](https://lkml.org/lkml/2014/10/22/931) |
| 2015/02/23 | Vlastimil Babka <vbabka@suse.cz> | [the big khugepaged redesign](https://lwn.net/Articles/634384) | 解决 huge page<br>1. 停止在 khugepaged 中预先分配大页面.<br>2. 只有在我们预期它们会成功的情况下才尝试错误分配.<br>3. 将折叠从 khugepaged 移动到 task_work 上下文.<br>4. thp 分配失败时唤醒 khugepaged. | RFC ☐ | [LORE RFC,0/6](https://lore.kernel.org/lkml/1424696322-21952-1-git-send-email-vbabka@suse.cz) |

但是上面的手段还是太激进了, 因此 Vlastimil 又进一步做了调整.

#### 7.2.3.2 Outsourcing THP allocations
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/05/11 | Vlastimil Babka <vbabka@suse.cz> | [Outsourcing page fault THP allocations to khugepaged](https://lwn.net/Articles/643891) | 由于 [the big khugepaged redesign](https://lwn.net/Articles/634384) 的改动过于激进, 社区建议被拆开重新适配, 将争议最小的一部分先提交到社区. 这组补丁不会将折叠扫描移动到 task_work 上下文, 而是专注于减少回收和压缩在 page fault 上下文中完成的工作, 通过将这一工作转移到 khugepaged. 这有两个好处:<br>1. 在 page fault 上下文中回收和规整会增加 page fault 的处理延迟, 这可能会抵消 THP 带来的收益, 特别是对于短期的分配, 这在 page fault 时都无法区分和甄别.<br>2. THP 分配在 page fault 仅使用异步规整(asynchronous compaction), 这减少了延迟, 也提高了成功的概率, 失败不会导致延迟规整(deferred compaction). khugepaged 则使用更彻底的同步规整(synchronous compaction), 不会因为 need_resched() 而中途退出, 并将正确地配合延迟规整(deferred compaction)机制. | RFC ☐ | [PatchWork RFC,0/4](https://lore.kernel.org/lkml/1431354940-30740-1-git-send-email-vbabka@suse.cz) |
| 2015/07/02 | Vlastimil Babka <vbabka@suse.cz> | [Outsourcing compaction for THP allocations to kcompactd](https://lwn.net/Articles/650051) | 本 RFC 系列是处理 THP 分配延迟尝试的另一个演进. 与前一个版本 [Outsourcing page fault THP allocations to khugepaged](https://lwn.net/Articles/643891) 的主要区别是借用了每个节点的 kcompactd. 试着把所有东西都放进 khugepaged 太笨拙了, 而 kcompactd 可以有更多的好处. 作者用 mmtests/thpscale 对它进行了简单的测试, 但是目前效果并不明显. | RFC ☐ | [PatchWork RFC v2,0/4](https://lore.kernel.org/lkml/1435826795-13777-1-git-send-email-vbabka@suse.cz) |



### 7.2.4 improve THP collapse rate
-------


*   khugepaged_max_ptes_none (`/sys/kernel/mm/transparent_hugepage/khugepaged/max_ptes_none`)

khugepaged 中如果发现当前连续的映射区间内有[超过 `khugepaged_max_ptes_none` 个页面是 pte_none 的](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L1643), 则将其合并为大页. 参见 [linux v2.6.38, mm/huge_memory.c#L1643](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L1643).


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/01/29 | Ebru Akagunduz <ebru.akagunduz@gmail.com> | [mm: incorporate read-only pages into transparent huge pages](https://lore.kernel.org/patchwork/patch/538503) | 允许 THP 转换只读 pte, 就像 do_swap_page 在读取错误后留下的那些pte. 当在 2MB 范围内存在最多数量为 `khugepaged_max_ptes_none` 的 pte_none ptes 时, THP可以将4kB的页面压缩为一个 THP. 这个补丁增加了对只读页面的大页支持. 该补丁使用一个程序进行测试, 该程序分配了800MB的内存, 对其进行写入, 然后休眠. 迫使系统通过触摸其他内存来交换除190MB外的所有程序. 然后, 测试程序对其内存进行混合的读写操作, 并将内存交换回来. 没有补丁的情况下, 只有没有被换出的内存保留在 THP 中, 这相当于程序内存的 24%, 这个百分比并没有随着时间的推移而增加. 有了这个补丁, 经过 5分钟的等待, khugepageage 将 60% 的程序内存还原为 THP. | v4 ☑ [4.0-rc1](https://kernelnewbies.org/Linux_4.0#Memory_management) | [PatchWork v4](https://lore.kernel.org/patchwork/patch/538503), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=10359213d05acf804558bda7cc9b8422a828d1cd) |
| 2015/04/14 | Ebru Akagunduz <ebru.akagunduz@gmail.com> | [mm: incorporate zero pages into transparent huge pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ca0984caa8235762dc4e22c1c47ae6719dcc4064) | 通过大页允许零页面, 该补丁提高了 THP 的大页转换率. 目前, 当在 2MB 范围内存在最多数量为 khugepaged_max_ptes_none 的 pte_none ptes 时, THP可以将4kB的页面压缩为一个 THP. 这个补丁支持了将零页映射为大页. 该补丁使用一个程序进行测试, 该程序分配了800MB的内存, 并执行交错读写操作, 其模式导致大约2MB的区域首先看到读访问, 从而导致影射了较多的零 pfn 映射. 没有补丁的情况下, 只有 50% 的程序被压缩成THP, 并且百分比不会随着时间的推移而增加. 有了这个补丁, 等待10分钟后, khugepage 转换了 99% 的程序内存.  | v2 ☑ [4.1-rc1](https://kernelnewbies.org/Linux_4.1#Memory_management) | [LORE v2](https://lore.kernel.org/lkml/1423688635-4306-1-git-send-email-ebru.akagunduz@gmail.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ca0984caa8235762dc4e22c1c47ae6719dcc4064) |
| 2015/09/14 | Ebru Akagunduz <ebru.akagunduz@gmail.com> | [mm: make swapin readahead to gain more THP performance](https://lore.kernel.org/patchwork/patch/597392) | 支持在对匿名页 swapin 的时候 readahead 及逆行大页的转换.<br>当 khugepaged 扫描页面时, 交换区中可能有一些页面. 有了这个补丁, 当 2MB 范围内 swap_ptes 的数目达到 max_ptes_swap 时, THP 可以将 4kB 页面转换成一个 THP 的大页.<br>这个补丁用来处理那些在被调出内存后访问大部分(但不是全部)内存的程序的. 补丁合入后, 这些程序不会在内存换出(swapout)后将内存转换成到 THPs 中, 而会在内存从交换分区读入(swapin)时进行转换.<br>测试使用了用一个测试程序, 该程序分配了 400B 的内存, 写入内存, 然后休眠. 然后强制将所有页面都换出. 之后, 测试程序通过对该内存区域进行写曹祖, 但是它在该区域的每 20 页中跳过一页.<br>1. 如果没有补丁, 系统就不能在 readahead 中交换. THP率为程序内存的65%, 不随时间变化.<br>2. 有了这个补丁, 经过10分钟的等待, khugepaged已经崩溃了程序99%的内存. | v2 ☑ [4.8-rc1](https://kernelnewbies.org/Linux_4.8#Memory_management) | [PatchWork RFC,v5,0/3](https://lore.kernel.org/patchwork/patch/597392)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC,v5,0/3](https://lore.kernel.org/lkml/1442259105-4420-1-git-send-email-ebru.akagunduz@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LKML](https://lkml.org/lkml/2015/9/14/610) |
| 2022/05/10 | Yang Shi <shy828301@gmail.com> | [Make khugepaged collapse readonly FS THP more consistent](https://patchwork.kernel.org/project/linux-mm/cover/20220228235741.102941-1-shy828301@gmail.com/) | 618921 | v1 ☐☑ | [2022/02/28 LORE v1,0/8](https://lore.kernel.org/r/20220228235741.102941-1-shy828301@gmail.com)<br>*-*-*-*-*-*-*-* <br>[2022/05/10 LORE v4,0/8](https://lore.kernel.org/all/20220510203222.24246-1-shy828301@gmail.com) |

*   userspace hugepage collapse

khugepaged 默认速度较慢, 每 10 秒最多扫描 4096 页. 作为一个系统范围的设置, 这通常最通用的, 但是也可以有一些激进的方法, 可以让程序受益于. 除了为符合条件的内存范围添加优先级, 暂时加快整个系统的 khugepaged, 或者为属于某个进程的内存分片工作, 还有一种方法是允许用户空间进行 HugePage Collapse. 这种方法的好处是, 这是在进程上下文中完成的, 因此它的 CPU 被占用到执行 HugePage Collapse 的进程上. khugepaged 并不参与.

David Rientjes 率先提出了这种想法 [Hugepage collapse in process context](https://lore.kernel.org/all/d098c392-273a-36a4-1a29-59731cdf5d3d@google.com), 它的思路是允许用户空间通过新的 process_madvise() 调用来诱导 HugePage Collapse. 这允许我们为当前或另一个进程折叠大页. 当完成时, madvise() 将在正确的节点上分配一个大页, 并尝试在进程上下文中进行折叠.

对于已经使用 MADV_DONTNEED 将内存释放回系统并随后将内存故障的 malloc() 实现, 这将立即非常有用. 因为, 与其等待 khugepaged 在 30min 以后出现, 然后将这个内存折叠成一个大页(在一个非常大的系统上, 这可能会花费更长的时间), 还不如使用这个 process_madvise() 模式提前诱导这个动作. 换句话说, "我将这个内存返回给应用程序, 它将是热的, 所以现在返回一个大页面, 而不是等到以后". 它还可以用于用于文本段的只读文件支持映射. 还有一些额外的好处就是, khugepaged 要做的工作也减少了.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/02/16 | David Rientjes <rientjes@google.com> | [Hugepage collapse in process context](https://lore.kernel.org/all/d098c392-273a-36a4-1a29-59731cdf5d3d@google.com) | 允许用户空间通过新的 process_madvise() 调用来诱导 HugePage Collapse | RFC,v1 ☐ | [LORE RFC v1,00/14](https://lore.kernel.org/all/d098c392-273a-36a4-1a29-59731cdf5d3d@google.com) |
| 2022/06/04 | Zach O'Keefe <zokeefe@google.com> | [mm: userspace hugepage collapse](https://patchwork.kernel.org/project/linux-mm/cover/20220308213417.1407042-1-zokeefe@google.com/) | 这组补丁为用户空间提供了一种机制, 可以在进程上下文中将符合条件的内存范围压缩为 THP, 从而允许用户精细化地控制自己的 HugePages 使用策略. 借鉴了 David Rientjes 的想法. | v1 ☐☑ | [LORE v1,00/14](https://lore.kernel.org/r/20220308213417.1407042-1-zokeefe@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/12](https://lore.kernel.org/r/20220414180612.3844426-1-zokeefe@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/12](https://lore.kernel.org/r/20220426144412.742113-1-zokeefe@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/12](https://lore.kernel.org/r/20220502181714.3483177-1-zokeefe@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/12](https://lore.kernel.org/r/20220504214437.2850685-1-zokeefe@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v6,0/15](https://lore.kernel.org/r/20220604004004.954674-1-zokeefe@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/18](https://lore.kernel.org/r/20220706235936.2197195-1-zokeefe@google.com) |


*   其他

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/03/11 | maobibo <maobibo@loongson.cn> | [mm/khugepaged: sched to numa node when collapse huge page](https://patchwork.kernel.org/project/linux-mm/patch/20220311090119.2412738-1-maobibo@loongson.cn/) | 622550 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220311090119.2412738-1-maobibo@loongson.cn)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20220315040549.4122396-1-maobibo@loongson.cn) |
| 2022/05/27 | Jiaqi Yan <jiaqiyan@google.com> | [Memory poison recovery in khugepaged collapsing](https://patchwork.kernel.org/project/linux-mm/cover/20220527190731.322722-1-jiaqiyan@google.com/) | 645671 | v4 ☐☑ | [LORE v4,0/2](https://lore.kernel.org/r/20220527190731.322722-1-jiaqiyan@google.com) |


### 7.2.5 THP splitting/reclaim/migration
-------

#### 7.2.5.1 splitting
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/07/31 | Yu Zhao <yuzhao@google.com> | [mm: optimize thp for reclaim and migration](https://lore.kernel.org/patchwork/patch/1470432) | 对 THP 的回收和迁移进行优化.<br>使用配置 THP 为 always 的时候, 大量 THP 的内部碎片会造成不小的内存压力. 用户空间可以保留许多子页面不变, 但页面回收不能简单的根据 dirty bit 来识别他们.<brr>但是, 仍然可以通过检查子页面的内容来确定它是否等同于干净. 当拆分 THP 用于回收或迁移时, 我们可以删除只包含0的子页面, 从而避免将它们写回或复制它们. | v1 ☐ | [PatchWork 0/3](https://patchwork.kernel.org/project/linux-mm/cover/20210731063938.1391602-1-yuzhao@google.com) |
| 2021/05/29 | Ning Zhang <ningzhang@linux.alibaba.com> | [mm, THP: introduce a controller to trigger THP reclaim](https://github.com/gatieme/linux/commit/57456d1625ba9036968fa0be70a6036b88f2b2f4) | 实现 MEMCG 的 THP 回收. [THP reclaim功能](https://www.alibabacloud.com/help/zh/elastic-compute-service/latest/thp-reclaim). | v1 ☐ | [PatchWork 0/3](https://patchwork.kernel.org/project/linux-mm/cover/20210731063938.1391602-1-yuzhao@google.com) |
| 2018/01/03 | Michal Hocko <mhocko@kernel.org> | [unclutter thp migration](https://lore.kernel.org/patchwork/patch/869414) | THP 迁移以令人惊讶的语义侵入了通用迁移. 迁移分配回调应该检查 THP 是否可以立即迁移, 如果不是这样, 则分配一个简单的页面进行迁移. 取消映射和移动, 然后通过将 THP 拆分为小页面, 同时将标题页面移动到新分配的 order-0 页面来修复此问题. 剩余页面通过拆分页面移动到 LRU 列表. 如果 THP 分配失败, 也会发生同样的情况. 这真的很难看而且容易出错. | v1 ☐ | [PatchWork 0/3](https://lore.kernel.org/patchwork/patch/869414) |
| 2019/08/22 | Yang Shi <yang.shi@linux.alibaba.com> | [Make deferred split shrinker memcg aware](https://lore.kernel.org/patchwork/patch/869414) | 目前, THP 延迟分割收缩器不了解 memcg, 这可能会导致某些配置过早地处罚 OOM.<br>通过引入每个 memcg 延迟拆分队列, 转换延迟拆分收缩器 memcg aware. THP 应位于每个节点或每个 memcg 延迟拆分队列上(如果它属于 memcg). 当页面迁移到另一个 memcg 时, 它也将迁移到目标 memcg 的延迟分割队列.<br>对于每个 memcg 列表, 重复使用第二个尾页的延迟列表, 因为同一个 THP 不能位于多个延迟拆分队列上.<br>使延迟分割收缩器不依赖于 memcg kmem, 因为它不是 slab. 过上述更改, 即使禁用了 memcg kmem(配置 cgroup.memory=nokmem), 测试也不会触发OOM. | v6 ☑ 5.4-rc1 | [PatchWork v6,0/4](https://patchwork.kernel.org/project/linux-mm/cover/1566496227-84952-1-git-send-email-yang.shi@linux.alibaba.com) |
| 2018/01/03 | Michal Hocko <mhocko@kernel.org> | [unclutter thp migration](https://lore.kernel.org/patchwork/patch/869414) | THP 迁移以令人惊讶的语义侵入了通用迁移. 迁移分配回调应该检查 THP 是否可以立即迁移, 如果不是这样, 则分配一个简单的页面进行迁移. 取消映射和移动, 然后通过将 THP 拆分为小页面, 同时将标题页面移动到新分配的 order-0 页面来修复此问题. 剩余页面通过拆分页面移动到 LRU 列表. 如果 THP 分配失败, 也会发生同样的情况. 这真的很难看而且容易出错. | v1 ☐ | [PatchWork 0/3](https://lore.kernel.org/patchwork/patch/869414) |
| 2020/11/19 | Zi Yan <ziy@nvidia.com> | [Split huge pages to any lower order pages and selftests.](https://patchwork.kernel.org/project/linux-mm/cover/20201119160605.1272425-1-zi.yan@sent.com) | NA | v1 ☐ | [PatchWork 0/7](https://patchwork.kernel.org/project/linux-mm/cover/20201119160605.1272425-1-zi.yan@sent.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/5](https://lore.kernel.org/r/20220321142128.2471199-1-zi.yan@sent.com) |


#### 7.2.5.2 splitting test
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/01/15 | Kirill A. Shutemov <kirill.shutemov@linux.intel.com> | [thp: add debugfs handle to split all huge pages](https://www.spinics.net/lists/linux-mm/msg99353.html) | 将 1 写入 "split_huge_pages" 接口将尝试查找并拆分系统中的所有 THP. 这对于调试非常有用. | v1 ☑ 4.5-rc1 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=49071d436b51b58aeaf7abcd1877f38ca0146e31) |
| 2020/11/19 | Zi Yan <ziy@nvidia.com> | [mm: huge_memory: a new debugfs interface for splitting THP tests.](https://patchwork.kernel.org/project/linux-mm/cover/20201119160605.1272425-1-zi.yan@sent.com) | 提供了一个直接的用户接口来拆分 THP. 使用  `<debugfs>/split` 接受一个新命令来执行此操作. 通过将 "<pid>,<vaddr_start>,<vaddr_end>" 写入 "<debugfs>/split_huge_pages"将 分割具有给定 pid 的进程中给定虚拟地址范围内的 thp. 用于测试拆分页面功能. 此外, 还向 `tools/testing/selftests/vm` 添加了一个自检程序, 通过拆分 PMD THP 和 PTE 映射 THP 来利用该接口. 这不会改变旧的行为, 即向接口写入 1 仍然会拆分系统中的所有 THP. | v8 ☑ 5.13-rc1 | [PatchWork v8,1/2](https://patchwork.kernel.org/project/linux-mm/patch/20210331235309.332292-1-zi.yan@sent.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=fa6c02315f745f00b62c634b220c3fb5c3310258) |



### 7.2.6 Reduce memory bloat
-------

首先是 HZP(huge zero page) 零页的支持.

[huge zero page vs FOLL_DUMP](https://linux-mm.kvack.narkive.com/Jrwys6ZE/huge-zero-page-vs-foll-dump)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/12/15 | Nitin Gupta <nitin.m.gupta@oracle.com> | [Introduce huge zero page](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=79da5407eeadc740fbf4b45d6df7d7f8e6adaf2c) | 参见 [Adding a huge zero page](https://lwn.net/Articles/517465). 在测试期间, 我注意到如果启用 THP, 某些工作负载(例如 NPB 的 ft.A)的内存消耗开销很大(高达2.5倍). 造成这种巨大差异的主要原因是 THP 的情况下缺少零页面. 即使是读操作触发的 page-fault 也必须分配一个真正的页. 该补丁集通过引入巨页的零页面(hzp) 来解决这个问题, HZP 是一个不可移动的巨页(x86-64上的2M), 里面都是零.<br>1. 如果是读 fault 且 fault 的地址周围的区域适合使用 THP, 则在 do_huge_pmd_anonymous_page() 中设置它.<br>2. 如果设置 hzp (ENOMEM)失败, 就回退到 handle_pte_fault(), 正常分配 THP.<br>3. 在对 hzp 的进行写操作触发 wp 时, 则为 THP 分配实际页面.<br>4. 如果是 ENOMEM, 则有一个优雅的回退: 创建一个新的 pmd 表, 并围绕故障地址将 pte 设置为新分配的正常(4k)页面. pmd中的所有其他 pte 设置为正常零页.<br>5. 当前不能分割 hzp, 但可以分割指向它的 pmd. 在分割 pmd 时, 创建了一个所有 ptes 设置为普通零页的表. | v6 ☑ 3.8-rc1 | [PatchWork v6,00/12](https://lore.kernel.org/lkml/1353007622-18393-1-git-send-email-kirill.shutemov@linux.intel.com) |


当前, 如果启用THP的策略为 "always", 或模式为 "madvise" 且某个区域标记为 MADV_HUGEPAGE 时, 内核都会优先分配大页, 而即使 pud 或 pmd 为空. 这会产生最佳的 VA 翻译性能, 减少 TLB 冲突, 但是却增加了内存的消耗. 特别是但如果仅仅访问一些分散地零碎的小页面范围, 却仍然分配了大页, 造成了内存的浪费.


Oracle 的 Nitin Gupta 进行了最早的尝试, 参见 [mm: Reduce memory bloat with THP](https://lkml.org/lkml/2018/1/19/252). 这组补丁发到了 RFC v2 但是未被社区所认可.

Mel Gorman 提供了[另外一种新的思路](https://lkml.org/lkml/2018/1/25/571) :

1.  当在足够大的范围内出现第一个 page fault 时, 分配一个巨大的页面大小和对齐的基页块, 但是只映射与 page fault 地址对应的基本页, 并保留其余页.

2.  后续在这个范围内其他的页面出现 page fault 时, 则继续映射之前保留的页面, 同样只映射触发 page fault 的基本页.

3.  当映射了足够多的页面后, 将映射的页面和保留中的剩余页面升级为一个 THP.

4.  当内存压力较大时, 将 THP 拆分, 从而将未使用的页面从保留中释放出来.

Anthony Yznaga 接替了之前同事 Nitin Gupta 的工作, 并基于 Mel 的思路, 进行了尝试 [mm: THP: implement THP reservations for anonymous memory](https://lore.kernel.org/patchwork/patch/1009090).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/01/19 | Nitin Gupta <nitin.m.gupta@oracle.com> | [mm: Reduce memory bloat with THP](https://marc.info/?l=linux-mm&m=151631857310828&w=2) | NA | RFC,v2 ☐ | [PatchWork RFC,v2](https://lkml.org/lkml/2018/1/19/252) |
| 2018/11/09 | Anthony Yznaga <anthony.yznaga@oracle.com> | [mm: THP: implement THP reservations for anonymous memory](https://lore.kernel.org/patchwork/patch/1009090) | NA | RFC ☐ | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/1009090) |
| 2021/07/31 | Yu Zhao <yuzhao@google.com> | [mm: optimize thp for reclaim and migration](https://lore.kernel.org/patchwork/patch/1470432) | 对 THP 的回收和迁移进行优化.<br>使用配置 THP 为 always 的时候, 大量 THP 的内部碎片会造成不小的内存压力. 用户空间可以保留许多子页面不变, 但页面回收不能简单的根据 dirty bit 来识别他们.<brr>但是, 仍然可以通过检查子页面的内容来确定它是否等同于干净. 当拆分 THP 用于回收或迁移时, 我们可以删除只包含0的子页面, 从而避免将它们写回或复制它们. | v1 ☐ | [PatchWork 0/3](https://patchwork.kernel.org/project/linux-mm/cover/20210731063938.1391602-1-yuzhao@google.com) |
| 2021/10/28 | Ning Zhang <ningzhang@linux.alibaba.com> | [Reclaim zero subpages of thp to avoid memory bloat](https://patchwork.kernel.org/project/linux-mm/cover/1635422215-99394-1-git-send-email-ningzhang@linux.alibaba.com) | 这个补丁集引入了一种新的机制来分割没有子页面的大页并回收这些子页面. thp 可能会导致内存膨胀, 从而导致 OOM. 通过对一些应用程序的测试, 作者发现内存膨胀的原因是一个巨大的页面可能包含一些零子页面(可能被访问或没有). 而且大多数零子页面集中在几个大页面中.<br>通过将匿名大页面添加到列表中, 以减少查找大页面的成本. 当内存回收被触发时, 列表将被遍历, 包含足够的零子页面的大页面可能会被回收. 同时, 用 ZERO_PAGE(0) 替换零子页面. 之前 Yu Zhao 已经做了一些类似的工作 [PatchWork 0/3](https://patchwork.kernel.org/project/linux-mm/cover/20210731063938.1391602-1-yuzhao@google.com), 当巨大的页面被交换或迁移来加速. 当我们在 swap 场景的正常内存收缩路径中这样做时, 以避免 OOM. 未来, 作者可能将尝试主动回收 "冷" 大页面. 为了尽可能在保持 thp 性能的同时. 使得开启了 thp 的内存使用量等于使用普通页的内存使用量. | RFC ☐ | [PatchWork RFC,0/6](https://patchwork.kernel.org/project/linux-mm/cover/1635422215-99394-1-git-send-email-ningzhang@linux.alibaba.com) |


### 7.2.7 THP Page Cache
-------

参见 [Linux 中的 Memory Compaction [三] - THP](https://zhuanlan.zhihu.com/p/117239320)

早期的 THP 还只支持匿名页(Anonymous Pages), 而不支持 Page Cache, 这是因为:

1. 匿名页(Anonymous Pages)通常只能通过 [mmap 映射](https://zhuanlan.zhihu.com/p/71517406)访问, 而 Page Cache 还可以通过直接调用 read() 和 write() 访问, huge page 区别于 normal page 的体现就是少了一级(或者几级)页表转换, 不通过映射访问的话, 对 huge page 的使用就比较困难.

2.  如果使用 THP 的话, 需要文件本身足够大, 才能充分利用 huge page 带来的好处, 而现实中大部分的文件都是比较小的, 参见 [Transparent huge pages in the page cache](https://lwn.net/Articles/686690).

不过在某些场景下, 让 THP 支持 page cache 的需求还是存在的.

[dhowells/linux-fs: fscache-thp](https://git.kernel.org/pub/scm/linux/kernel/git/dhowells/linux-fs.git/log/?h=fscache-thp)

[Transparent huge pages for filesystems](https://lwn.net/Articles/789159), 译文 [LWN 回顾: facebook 利用 transparent huge page 来优化代码执行性能](https://blog.csdn.net/Linux_Everything/article/details/103790440).

#### 7.2.7.1 THP RAMFS
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2013/05/12 | "Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [Transparent huge page cache](https://lore.kernel.org/patchwork/patch/378803) | 这组补丁集为内核页面缓存增加了巨页的支持. 为了证明建议的更改是功能性的, 为最简单的文件系统 ramfs 启用了该特性. 每个 THP 在页面缓存基数树中由 HPAGE_PMD_NR(x86-64 上的 512)条目表示: 一个条目用于首页, HPAGE_PMD_NR-1条目用于尾页. 可以通过三种方式将大型页面添加到页面缓存:<br>1. 向文件或页面写入;<br>2. 从稀疏文件中读取;<br>3. 稀疏文件上触发 page_fault.<br>当然可能存在第三种方法可能是折叠小页面, 但它超出了最初的实现范围. | v4 ☐ | [PatchWork v4,00/39](https://lore.kernel.org/patchwork/patch/378803) |
| 2013/09/23 | "Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [Transparent huge page cache: phase 1, everything but mmap()](https://lore.kernel.org/patchwork/patch/408000) | 这组补丁集为内核页面缓存增加了巨页的支持. 为了证明建议的更改是功能性的, 为最简单的文件系统 ramfs 启用了该特性, 但是 mmap 除外, 这将在后续的合入中修复. | v6 ☐ | [PatchWork v6,00/22](https://lore.kernel.org/patchwork/patch/408000) |


#### 7.2.7.2 THP TMPFS/SHMEM
-------


随后社区不少开发者在做 (TMPFS) Page Cache 的 THP 支持. 参见 [Two transparent huge page cache implementations](https://lwn.net/Articles/684300) 所述:

1.  一个解决方案是 Kirill Shutemov 提出的, 他完成了 THP 启用 tmpfs 补丁集. Kirill 的工作基于社区已有的[复合页面 compound pages](https://lwn.net/Articles/619514), 这是一种相当精细的机制, 可以将单个内存页面绑定到一个更大的页面上. 作者个人仓库 [`kas/linux.git`](https://git.kernel.org/pub/scm/linux/kernel/git/kas/linux.git).

2.  另一个竞争者是由 Hugh Dickins 实现的团队页面(team pages)方案. "团队页面"可以被认为是一种新的、更轻量级的页面分组方式. 它的工作有一个优势, 据作者所言, 这个方案已经在谷歌的产品部署了一年多, 效果和稳定性都有保障.

社区开发者争相选择 tmpfs 入手是因为作为一个文件系统, 它很特殊, 从某种意义上说, 它不算一个 "货真价实" 的文件系统. 但它这种模棱两可的特性正好是一个绝佳的过渡, 实现了 THP 对 tmpfs 的支持之后, 可以进一步[推广到 ext4 这种标准的磁盘文件系统](https://lwn.net/Articles/718102)中去, 但......, 还很多问题要解决, 比如磁盘文件系统的 readahead 机制, [预读窗口通常是 128KB](https://zhuanlan.zhihu.com/p/71217136), 这远小于一个 huge page 的大小. 参见 LSFMM 2017 的报道 [Huge pages in the ext4 filesystem](https://lwn.net/Articles/718102)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/02/20 | Hugh Dickins <hughd@google.com> | [huge tmpfs: an alternative approach to THPageCache](https://lwn.net/Articles/634334) | 为 tmpfs 文件系统增加了透明的巨大页面支持.<br>这项工作包含了页面缓存中完全支持大页面所需的许多元素(这也是 Kirill 补丁的最终目标). 但是 Hugh 的方法相当不同, 这引起了用户社区的一些担忧; 最终, 这些补丁集中只有一个可能被合并.<br>参见 [Improving huge page handling](https://lwn.net/Articles/636162) | v4 ☐ | [PatchWork 00/24](https://lore.kernel.org/lkml/alpine.LSU.2.11.1502201941340.14414@eggly.anvils) |
| 2016/02/20 | Hugh Dickins <hughd@google.com> | [huge tmpfs: THPagecache implemented by teams](https://lwn.net/Articles/682623) | 为 tmpfs 文件系统增加了透明的巨大页面支持.<br>这项工作包含了页面缓存中完全支持大页面所需的许多元素(这也是 Kirill 补丁的最终目标). 但是 Hugh 的方法相当不同, 这引起了用户社区的一些担忧; 最终, 这些补丁集中只有一个可能被合并.<br>参见 [Improving huge page handling](https://lwn.net/Articles/636162) | v4 ☐ | [PatchWork 00/24](https://lore.kernel.org/lkml/alpine.LSU.2.11.1502201941340.14414@eggly.anvils) |

经过多次改进和优化后, 最终 Kirill Shutemov 的基于复合页的方案 [THP-enabled tmpfs/shmem (using compound pages)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1b5946a84d6eb096158e535bdb9bda06e7cdd941) 被合入主线 [4.8](https://kernelnewbies.org/Linux_4.8#Support_for_using_Transparent_Huge_Pages_in_the_page_cache) 版本.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/06/15 | "Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [THP-enabled tmpfs/shmem using compound pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1b5946a84d6eb096158e535bdb9bda06e7cdd941) | 支持 tmpfs 和 shmem page cache 的透明大页支持.<br>1. 引入了 [CONFIG_TRANSPARENT_HUGE_PAGECACHE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e496cf3d782135c1cca0d154d4b924517ff58de0).<br>2. 实现了 [shmem page cache 大页](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=800d8c63b2e989c2e349632d1648119bf5862f01)的支持.<br>3. 完成了 khugepaged [对 tmpfs/shmem page cache 页面合并](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f3f0e1d2150b2b99da2cbdfaad000089efe9bf30)的支持. | v9 ☑ [4.8-rc1](https://kernelnewbies.org/Linux_4.8#Support_for_using_Transparent_Huge_Pages_in_the_page_cache) | [PatchWork v4,00/25](https://lore.kernel.org/patchwork/patch/658181)<br>*-*-*-*-*-*-*-* <br>[PatchWork v9,00/32](https://lore.kernel.org/lkml/1465222029-45942-1-git-send-email-kirill.shutemov@linux.intel.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v9-rebased2,00/37](https://lore.kernel.org/lkml/1466021202-61880-1-git-send-email-kirill.shutemov@linux.intel.com)  |


#### 7.2.7.3 THP EXT4
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/07/26 | "Kirill A. Shutemov" <kirill.shutemov@linux.intel.com> | [ext4: support of huge pages](https://lore.kernel.org/patchwork/patch/701302) | 这是我的补丁集的第一个版本, 它旨在将 THO 带到 ext4, 它还没有准备好应用或正式使用, 但足以展示该方法.<br>基本原理与 tmpfs 相同, 并构建在它的基础上. 主要区别在于, 我们需要处理从备份存储器的读取和回写. | RFC,v1 ☐ | [PatchWork v1,RFC,00/33](https://lore.kernel.org/patchwork/patch/701302) |
| 2019/07/17 | Yang Shi <yang.shi@linux.alibaba.com> | [Fix false negative of shmem vma's THP eligibility](https://lore.kernel.org/patchwork/patch/1101422) | 提交7635d9cbe832 ("mm, THP, proc: report THP qualified for each vma") 为进程的 map 引入了 THPeligible bit. 但是, 当检查 shmem vma 的资格时, `__transparent_hugepage_enabled()` 被调用以覆盖来自 shmem_huge_enabled() 的结果. 它可能导致匿名 vma 的 THP 标志覆盖 shmem 的.<br>通过使用 transhuge_vma_suitable() 来检查 vma, 来修复此问题 | v4 ☑ 5.3-rc1 | [PatchWork v4,0/2](https://lore.kernel.org/patchwork/patch/1101422) |
| 2021/07/30 | Hugh Dickins <hughd@google.com> | [tmpfs: HUGEPAGE and MEM_LOCK fcntls and memfds](https://patchwork.kernel.org/project/linux-mm/cover/2862852d-badd-7486-3a8e-c5ea9666d6fb@google.com) | 一系列 HUGEPAGE 和 MEM_LOCK tmpfs fcntls 和 memfd_create 标志的清理和修复工作. | v1 ☐ | [PatchWork 00/16](https://patchwork.kernel.org/project/linux-mm/cover/2862852d-badd-7486-3a8e-c5ea9666d6fb@google.com) |

#### 7.2.7.4 non-tmpfs filesystems
-------

Matthew Wilcox 在这方面也做了很多工作.
代码仓库参见 : http://git.infradead.org/users/willy/pagecache.git

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/06/10 | Matthew Wilcox <willy@infradead.org> | [Large pages in the page cache](https://lore.kernel.org/patchwork/patch/1254710) | NA | RFC v6 ☐  | [PatchWork RFC,v6,00/51](https://patchwork.kernel.org/project/linux-mm/cover/20200610201345.13273-1-willy@infradead.org) |
| 2020/06/29 | Matthew Wilcox <willy@infradead.org> | [THP prep patches](https://patchwork.kernel.org/project/linux-mm/cover/20200629151959.15779-1-willy@infradead.org) | NA | v1 ☑ 5.9-rc1 | [PatchWork 0/7](https://patchwork.kernel.org/project/linux-mm/cover/20200629151959.15779-1-willy@infradead.org) |
| 2020/09/10 | Matthew Wilcox <willy@infradead.org> | [THP iomap patches for 5.10](https://lore.kernel.org/patchwork/patch/1304097) | NA | v2 ☑ 5.10-rc1 | [PatchWork v2,0/9](https://lore.kernel.org/patchwork/patch/1304097) |
| 2020/09/16 | Matthew Wilcox <willy@infradead.org> | [fs: Add a filesystem flag for THPs](https://lore.kernel.org/patchwork/patch/1306507) | NA | v1 ☑ 5.10-rc1 | [PatchWork 1/2](https://lore.kernel.org/patchwork/patch/1306507) |
| 2020/08/24 | Matthew Wilcox <willy@infradead.org> | [iomap/fs/block patches for 5.11](https://lore.kernel.org/patchwork/patch/1294511) | NA | v1 ☑ 5.11-rc1 | [PatchWork 00/11](https://lore.kernel.org/patchwork/patch/1294511) |
| 2020/10/26 | Matthew Wilcox <willy@infradead.org> | [Remove nrexceptional tracking](https://lore.kernel.org/patchwork/patch/1294511) | NA | v1 ☐ | [PatchWork 00/11](https://patchwork.kernel.org/project/linux-mm/cover/20201026151849.24232-1-willy@infradead.org) |
| 2020/11/12 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Overhaul multi-page lookups for THP](https://lore.kernel.org/patchwork/patch/1337675) | 提升大量页面查找时的效率 | v4 ☐ | [PatchWork v4,00/16](https://patchwork.kernel.org/project/linux-mm/cover/20201112212641.27837-1-willy@infradead.org) |
| 2020/10/29 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Transparent Hugepages for non-tmpfs filesystems](https://patchwork.kernel.org/project/linux-mm/cover/2862852d-badd-7486-3a8e-c5ea9666d6fb@google.com) | 一系列 HUGEPAGE 和 MEM_LOCK tmpfs fcntls 和 memfd_create 标志的清理和修复工作. | v1 ☐ | [PatchWork 00/16](https://patchwork.kernel.org/project/linux-mm/cover/20201029193405.29125-1-willy@infradead.org) |


### 7.2.8 Huge Page(THP) SWAP
-------


THP 和 hugetlb 看起来样子差不多, 但在 Linux 中的归属和行为却完全不同. 后者一体成型, 而前者更像是焊接起来的. THP 虽然勉强拼凑成了 huge page 的模样, 但骨子里还是 normal page, 还和 normal page 同处一个世界.

在它诞生之初, 面对这个庞然大物, 既有的内存管理子系统的机制还没做好充分的应对准备, 比如 THP 要 swap 的时候怎么办啊? 这个时候, 只能调用 split_huge_page(), 将 THP 重新打散成 normal page.

swap out 的时候打散, swap in 的时候可能又需要重新聚合回来, 这对性能的影响是不言而喻的. 一点一点地找到空闲的 pages, 然后辛辛苦苦地把它们组合起来, 现在到好, 一切都白费了(路修了又挖, 挖了又修……). 虽然动态地生成 huge page 确实能更充分利用物理内存, 但其带来的收益, 有时还真不见得能平衡掉这一来一去的损耗.

不过呢, 内核开发者也在积极努力, 希望能够实现 THP 作为一个整体被 swap out 和 swap in(参考这篇文章), 但这算是对 “牵一发而动全身” 的内存子系统的一次重大调整, 所以更多的 regression 测试还在进行中.

可见啊, THP 并没有想象的那么美好, 用还是不用, 怎么用, 就成了一个需要思考和选择的问题.

[The final step for huge-page swapping](https://lwn.net/Articles/758677)

#### 7.2.8.1 optimize the performance of Transparent
-------

后来随着存储设备的性能提升, 普通页面换入换出已无法使磁盘带宽达到饱和. 而另一方面内存容量不断增加, THP 的使用已经越来越广泛. 因此有必要对 THP 交换性能进行优化.


引入 THP SWAP 是有诸多好处的:

1. 批量处理 THP 的交换操作以减少锁的获取/释放, 包括分配/释放交换空间, 增加/删除交换缓存, 读写交换空间等. 这将有助于提高 THP 交换的性能.

2. THP 交换空间的读写最小也是 2M 的顺序 IO(sequential IO). 而 swap read 交换读取通常是 4k 随机 IO. 这也将提高 THP 交换的性能.

3. 使用 THP swap 也有助于减少内存碎片, 特别是对 THP 被大量使用的应用程序. 在 THP 被 swap out 之后, 至少可以释放 2M 的连续页面.

4. 使能 swap 也能有效地将提高系统的 THP 利用率. 因为 khugepaged 将普通页面折叠成 THP 的速度相当慢. 如果在换出时对 THP 进行拆分, 那么再次换入后普通页面需要相当长的时间才能再次折叠成 THP. 较高 THP 利用率也有助于提高基于页面的内存管理的效率.

但是引入 THP SWAP 后, 必然增大换入换出时读/写的 IO 大小, 这可能会给存储设备带来更多的开销. 因此为了解决这个问题, THP 交换应该只在必要的时候打开. 例如, 它可以通过 "always/never/madvise" 逻辑来选择, 全局打开, 全局关闭, 或者仅对 VMA 使用 MADV_HUGEPAGE 打开, 等等.


*   Delay splitting THP during swapping out


支持 THP SWAP 的第一步是逐步延迟拆分 THP, 最终避免在 THP 交换期间拆分 THP, 并在整个 THP 中进行交换. [LWN 717707: 页交换（swap）的改进计划](https://tinylab.org/lwn-717707).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/05/15 | "Huang, Ying" <ying.huang@intel.com> | [THP swap: Delay splitting THP during swapping out/STEP 1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=747552b1e71b400fa32a221e072be2c0b7661f14) | 在这个补丁集完成了延迟拆分 THP, 将拆分大页面的时间几乎从交换的第一步推迟到为 THP 分配交换空间并将 THP 添加到交换缓存之后. 这将减少交换缓存管理中使用的锁的获取/释放.<br>在 8 个进程的 vm-scalability swap-w-seq 测试用例(测试用例创建了 8 个进程, 它们依次分配和写入匿名页面, 直到 RAM 和交换设备的一部分用完)中, 使用补丁集, 换出的吞吐量提高了 15.5%(从 3.73GB/s 提高到 4.31GB/s). | v11 ☑ 4.13-rc1 | [2016/08/09 LORE RFC,00/11](https://lore.kernel.org/lkml/1470760673-12420-1-git-send-email-ying.huang@intel.com)[2016/09/01 LORE v2,00/10](https://lore.kernel.org/lkml/1472743023-4116-1-git-send-email-ying.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,00/12](https://lore.kernel.org/all/20170724051840.2309-1-ying.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[2017/05/15 LORE v11,0/5](https://lore.kernel.org/lkml/20170515112522.32457-1-ying.huang@intel.com) |
| 2017/07/14 | "Huang, Ying" <ying.huang@intel.com> | [THP swap: Delay splitting THP during swapping out/STEP 2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fe490cc0fe9e6ee48cc48bb5dc463bc5f0f1428f) | 这是 THP (透明大页) 交换优化的 STEP 2. 在 STEP 1, 拆分大页面的时机从交换的第一步延迟到为 THP 分配交换空间并将 THP 添加到交换缓存之后. 在 STEP 2 中, 拆分被进一步延迟到交换完成之后. 在这个补丁集中, 对匿名 THP 回收的更多操作(如 TLB 刷新、将 THP 写入交换设备、将 THP 从交换缓存中删除) 被批处理. 从而提高了匿名 THP 交换的性能. | v3 ☑ 4.14-rc1 | [2017/06/23 LORE v2,00/12](https://lore.kernel.org/lkml/20170623071303.13469-1-ying.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[2017/07/24 LORE v3,00/12](https://lore.kernel.org/all/20170724051840.2309-1-ying.huang@intel.com) |
| 2022/05/27 | Barry Song <21cnbao@gmail.com> | [[v2] arm64: enable THP_SWAP for arm64](https://patchwork.kernel.org/project/linux-mm/patch/20220527100644.293717-1-21cnbao@gmail.com/) | 645555 | v2 ☐☑ | [LORE v2](https://lore.kernel.org/r/20220527100644.293717-1-21cnbao@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3](https://lore.kernel.org/r/20220706072707.114376-1-21cnbao@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/1](https://lore.kernel.org/r/20220720093737.133375-1-21cnbao@gmail.com) |


*   Swapout/swapin THP in one piece

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/12/14 | "Huang, Ying" <ying.huang@intel.com> | [swap: Swapout/swapin THP in one piece](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fe490cc0fe9e6ee48cc48bb5dc463bc5f0f1428f) | NA | v9 ☐ 4.14-rc1 | [2018/05/09 LORE v2,00,21](https://lore.kernel.org/lkml/20180509083846.14823-3-ying.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[2018/12/14 LORE v9,00/21](https://lore.kernel.org/lkml/20181214062754.13723-1-ying.huang@intel.com), [PatchWork v9,00/21](https://patchwork.kernel.org/project/linux-mm/cover/20181214062754.13723-1-ying.huang@intel.com) |


### 7.2.9 khugepaged
-------

khugepaged 是透明巨页的守护进程, 它会被定时唤醒, 并根据配置尝试将 page_size(比如 4K) 大小的普通 page 转成巨页, 减少 TLB 压力, 提高内存使用效率.

khugepaged 的处理过程中需要各种内存锁, 会在一定程度上影响系统性能.

khugepaged 主要的调用链如下:

```cpp
khugepaged
-=> khugepaged_do_scan()
    -=> khugepaged_scan_mm_slot()
        -=> khugepaged_scan_pmd()
            -=> collapse_huge_page()
                -=> __collapse_huge_page_copy()
```


khugepaged 控制参数, 位于 `/sys/kernel/mm/transparent_hugepage/khugepaged` 目录下:

| 节点 | 描述 |
|:---:|:----:|
| pages_collapsed | 记录了 khugepaged 已经[合并的大页的数目](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L1902) |
| pages_to_scan | 每次 khugepaged 尝试[处理的页面数目](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L2146), khugepage 扫描是有一定开销的, 因此对扫描的范围做一些限制 |
| full_scans | 完整扫描的轮数. 每次如果 khugepaged 如果完整的扫描了某个 mm_struct 的所有 VMA, 则[该计数增加 1](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L2118).  |
| scan_sleep_millisecs | 扫描间隔(毫秒) |
| alloc_sleep_millisecs | 当分配大内存失败等待多少毫秒来压制下一个大内存页的分配尝试. |

khugepaged 处理流程

| 关键流程 | 描述 |
|:-------:|:---:|
| khugepaged_do_scan | khugepaged 被唤醒后, 尝试处理 khugepaged_pages_to_scan 个页面. 其中每一次页面的循环这是通过调用 khugepaged_scan_mm_slot() 完成. 在 khugepaged_do_scan() 每次循环的最开始, 会尝试[调用 prealloc page 预分配一个巨页](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L2151), 需要注意. 这主要针对非 numa 系统, 对于 numa 系统, 这里不会真正分配. |
| khugepaged_scan_mm_slot | 扫描 struct mm_struct , 找到可以用巨页代替的普通页面. 这里需要扫描的 struct mm_struct 是放到[全局变量 khugepaged_scan 中的 mm_slot](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L93). 而这些 mm_slot 中的这些 mm 是在 vma 结构体发生[合并 merge](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L1556)和[扩展](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L678)的时候调用 [khugepaged_enter()](https://elixir.bootlin.com/linux/v2.6.38/source/include/linux/khugepaged.h#L38) 加入的. 这里找到需要扫描的 mm_struct 之后, 扫描 mm_struct 中包含的[各个 vma](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L2039), 注意, 扫描的范围是一个巨页包括的范围. |
| [khugepaged_scan_pmd](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L1915) | 完成普通页面搬运到巨页的工作. 具体来说, khugepaged_scan_pmd() 通过访问 vma 中的每一个页面, 然后判断该页面是否可以用巨页代替, 可以的话[调用 collapse_huge_page() 处理](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L1982). |
| collapse_huge_page | 首先[通过 `__collapse_huge_page_copy` 将普通页面拷贝到巨页中](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L1872), 然后释放普通页面. 不过释放之前会将原来页面的 tlb 刷出去, 然后将原来的页面从 LRU 链表移除, 最后拷贝到巨页中后, 还会更新内存相关信息. 如果是 numa 系统, 这里才会[真正分配巨页](https://elixir.bootlin.com/linux/v2.6.38/source/mm/huge_memory.c#L2151). |



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2011/01/13 | Andrea Arcangeli <aarcange@redhat.com> | [thp: khugepaged](https://lkml.org/lkml/2010/12/23/306) | 实现了一个 khugepaged 的内核线程, 用来完成将标准的小页面合并成大页的操作. | v1 ☑ 2.6.38-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ba76149f47d8c939efa0acc07a191237af900471) |
| 2015/09/14 | Ebru Akagunduz <ebru.akagunduz@gmail.com> | [mm: add tracepoint for scanning pages](https://lkml.org/lkml/2015/9/14/611) | [mm: make swapin readahead to gain more THP performance](https://lore.kernel.org/lkml/1442259105-4420-1-git-send-email-ebru.akagunduz@gmail.com) 系列的其中一个补丁, 为 khuagepaged 引入了 tracepoint 跟踪点. 用 scan_result 标记了 khugepaged 扫描的结果. | v5 ☑ 4.5-rc1 | [PatchWork RFC,v5,0/3](https://lore.kernel.org/lkml/1442259105-4420-2-git-send-email-ebru.akagunduz@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LKML](https://lkml.org/lkml/2015/9/14/611)<br>*-*-*-*-*-*-*-* <br>[commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7d2eba0557c18f7522b98befed98799990dd4fdb) |
| 2011/01/13 | Peter Xu <peterx@redhat.com> | [mm/khugepaged: Detecting uffd-wp vma more efficiently](https://patchwork.kernel.org/project/linux-mm/patch/20210922175156.130228-1-peterx@redhat.com) | 实现了一个 khugepaged 的内核线程, 用来完成将标准的小页面合并成大页的操作. | ☑ 2.6.38-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ba76149f47d8c939efa0acc07a191237af900471) |

### 7.2.9 优化 struct page 的内存占用
--------

*   DMEMFS


针对 kernel 中元数据对内存资源占用过高的问题, 腾讯云设计了全新的文件系统 Dmemfs(Direct Memory File System), 可以直接管理部分系统预留的虚拟机内存服务, 提高系统的资源利用率降低平台成本. 这个方案不仅提高了系统的资源利用率, 能够降低平台成本并最终让利于用户, 同时也给系统开销降低提供了一种新的思路.



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/12/07 | Yulei Zhang <yulei.kernel@gmail.com>/<yuleixzhang@tencent.com> | [Enhance memory utilization with DMEMFS](https://lwn.net/ml/linux-kernel/cover.1607332046.git.yuleixzhang@tencent.com) | DMEMFS 目的非常明确, 减少云场景下 struct page 结构体本身的内存消耗. 效果也是非常明显, 320G 可以省出 5G 的内存. | RFC, v2 ☐ | [2020/10/8 LKML 00/35](https://lkml.org/lkml/2020/10/8/139)<br>*-*-*-*-*-*-*-* <br>[2020/12/07 PatchWork RFC,V2,00/37](https://patchwork.kernel.org/project/linux-mm/cover/cover.1607332046.git.yuleixzhang@tencent.com) |



*   Free some vmemmap pages of HugeTLB page

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/05/10 | Muchun Song <songmuchun@bytedance.com> | [Free some vmemmap pages of HugeTLB page](https://patchwork.kernel.org/project/linux-mm/cover/20210510030027.56044-1-songmuchun@bytedance.com) | HugeTLB 使用的大量 struct page 里面有一部分没啥用可以省略. 复合页中只有头页面(compound_head)中的信息, 剩余尾页面(tail pages)中填充的信息是一样的. 因此一个 2M 的 HugeTLB 在 4K page size 的 x86_64 上会用到 512 个 struct page, 这 512 个 struct page 结构体本身占用了 sizeof(struct page) * 512 / PAGE_SIZE = 8 pages. 所以, 其实可以将最后 7 个 tail pages 的信息有点冗余, 将他们合并为一个页面(将 page 2~7 全部映射到 page 1), 这样只需要实际占用 2 个 page 就完成了映射. 这组补丁节省了大量的内存空间, 当然负面作用是分配和释放的时候会慢个 2 倍, 不过都比较小, MS 级别. | v23 ☑ 5.14-rc1 | [PatchWork v23,0/9](https://patchwork.kernel.org/project/linux-mm/cover/20210510030027.56044-1-songmuchun@bytedance.com) |
| 2021/10/18 | Muchun Song <songmuchun@bytedance.com> | [Free the 2nd vmemmap page associated with each HugeTLB page](https://lore.kernel.org/patchwork/patch/1459641) | NA  | v2 ☐ | [2021/07/14 PatchWork RFC](https://patchwork.kernel.org/project/linux-mm/cover/20210714091800.42645-1-songmuchun@bytedance.com)<br>*-*-*-*-*-*-*-* <br>[2021/10/18 PatchWork v6,0/5](https://patchwork.kernel.org/project/linux-mm/cover/20211018102043.78685-1-songmuchun@bytedance.com) |
| 2022/02/08 | Muchun Song <songmuchun@bytedance.com> | [arm64: mm: hugetlb: add support for free vmemmap pages of HugeTLB](https://patchwork.kernel.org/project/linux-mm/patch/20220208054632.66534-1-songmuchun@bytedance.com/) | 本系列文章可以极大地减少 2MB HugeTLB 页面的结构页面开销. 与之前的方法相比, 它进一步减少了 2MB HugeTLB 的结构页面开销 12.5%, 这意味着每 1TB HugeTLB 的结构页面开销为 2GB. | v2 ☑ 5.18-rc1 | [PatchWork v2,2/2](https://lore.kernel.org/all/20220208054632.66534-2-songmuchun@bytedance.com)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/5](https://lore.kernel.org/all/20211101031651.75851-1-songmuchun@bytedance.com) |
| 2022/02/10 | Joao Martins <joao.m.martins@oracle.com> | [sparse-vmemmap: memory savings for compound devmaps (device-dax)](https://patchwork.kernel.org/project/linux-mm/cover/20220210193345.23628-1-joao.m.martins@oracle.com/) | 这个系列, 通过追求类似于 Muchun Song [Free some vmemmap pages of HugeTLB page](https://patchwork.kernel.org/project/linux-mm/cover/20210510030027.56044-1-songmuchun@bytedance.com) 的方法, 针对带有 compound_pages 的 devmap, 最小化了 struct page 的开销. | v5 ☐☑ | [PatchWork v5,0/5](https://lore.kernel.org/r/20220210193345.23628-1-joao.m.martins@oracle.com) |


## 7.3 复合页 Compound Page
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2003/02/05 | Andrew Morton <akpm@digeo.com> | [Infrastructure for correct hugepage refcounting](https://github.com/gatieme/linux-history/commit/eefb08ee7da81e1548ffd5b664682dc5b229ddc2) | 实现了复合页(Compound Page), 如果用户的地址在一个大页面中, 设置页面的 refcount 是一个有歧义的事情, 我们应该设置的是组成这个大页的 4K 普通页, 而不是高阶分配单元的头页. 为了方便处理, 实现了一种处理高阶页面的通用方式:<br>1. 高阶页面称为 "复合页面", 复合页面的第一个 (控制) 4k 页面称为 "head" 页面, 剩余页面为尾页.<br>2. 所有复合页面都有 PG_compound 集合, 所有页面都有自己的 lru, 尾页面都指向头页面.<br>3. 紧接着就完成了 [convert hugetlb code to use compound pages](https://github.com/gatieme/linux-history/commit/b3a656b6d36622e628974bad5cf19c006c395efe) | v1 ☑ 2.5.60~33 | [COMMIT](https://github.com/gatieme/linux-history/commit/eefb08ee7da81e1548ffd5b664682dc5b229ddc2) |
| 2007/10/04 | Christoph Lameter <clameter@sgi.com> | [Virtual Compound Page Support V2](https://lore.kernel.org/patchwork/patch/93090) | NA | ☑ 4.11-rc1 | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/93090) |
| 2008/10/23 | Andy Whitcroft <apw@shadowen.org> | [Fixes for gigantic compounds pages V3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=18229df5b613ed0732a766fc37850de2e7988e43) | NA | v1 ☑✓ 2.6.28-rc4 | [LORE v1](https://lore.kernel.org/all/1223458499-12752-1-git-send-email-apw@shadowen.org)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/2](https://lore.kernel.org/all/1224771559-19363-1-git-send-email-apw@shadowen.org) |


# 8 进程虚拟地址空间(VMA)
-------

## 8.1 VMA
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/12/19 | Colin Cross | [mm: add a field to store names for private anonymous memory](https://lore.kernel.org/patchwork/patch/416962) | 在二进制中通过 xxx_sched_class 地址顺序标记调度类的优先级, 从而可以通过直接比较两个 xxx_sched_class 地址的方式, 优化调度器中两个热点函数 pick_next_task()和check_preempt_curr(). | v2 ☐ | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/416962)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/416962) |
| 2022/03/10 | Alex Sierra <alex.sierra@amd.com> | [split vm_normal_pages for LRU and non-LRU handling](https://patnux-mm/cover/20220310172633.9151-1-alex.sierra@amd.com/) | DEVICE_COHERENT 页面在 "普通" 页面可以被内核中的各种调用者使用的方式上引入了微妙的区别.<br>为了在 CPU 页表中进行映射和 COW, 它们的行为就像普通页面一样. 但它们不支持 LRU 列表、NUMA 迁移或 THP. 因此, 我们将 vm_normal_page 拆分为两个函数 vm_normal_any_page 和 vm_normal_lru_page. 后者将只返回那些可以放在 LRU 列表中, 并且支持 NUMA 迁移、KSM 和 THP 的页面.<br>在自测试中添加了 HMM 测试, 以使用设备连贯页面来执行这些更改. 名为 hmm_cow_in_device 的新测试将测试标记为 COW 的页面, 并在设备区域中分配. 此外, 还向 hmm_gup_test 中添加了更多的配置, 以测试基本的获取用户页面和设备区域页面中的获取用户页面的快速路径. | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/rsierra@amd.com) |
| 2022/05/19 | Chih-En Lin <shiyn.lin@gmail.com> | [Introduce Copy-On-Write to Page Table](https://patchwork.kernel.org/project/linux-mm/cover/20220519183127.3909598-1-shiyn.lin@gmail.com/) | 643289 | v1 ☐☑ | [LORE v1,0/6](https://lore.kernel.org/r/20220519183127.3909598-1-shiyn.lin@gmail.com) |
| 2022/05/27 | Jakub Matěna <matenajakub@gmail.com> | [Refactor of vma_merge and new merge call](https://patchwork.kernel.org/project/linux-mm/cover/20220527104810.24736-1-matenajakub@gmail.com/) | 645578 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20220527104810.24736-1-matenajakub@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/2](https://lore.kernel.org/r/20220527211708.839033-1-matenajakub@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/2](https://lore.kernel.org/r/20220602145642.16948-1-matenajakub@gmail.com) |


### 8.1.1 具名匿名页 Anonymous VMA naming
-------

用户空间进程通常有多个分配器, 每个分配器执行匿名 MMAP 以获取内存. 在整体检查单个进程或系统的内存使用情况时, 如果能够分解每个层分配的各种堆并检查它们的大小、RSS和物理内存使用情况对分析内核的内存分配和管理非常有用.

因此内核开发者提出匿名 VMA 命名方案, 可以有效地对进程不同名称的 VMA 的进行统计, 比较和区分. 以前有很多匿名VMA 命名的尝试.

最初 [Colin Cross](https://lore.kernel.org/linux-mm/1372901537-31033-1-git-send-email-ccross@android.com) 实现了一个重计数名称的字典, 以重用相同的名称字符串. 当时 Dave Hansen [建议改用用户空间指针](https://lore.kernel.org/linux-mm/51DDFA02.9040707@intel.com), 补丁也这样重写了.

后续 Sumit Semwal 发现这组补丁并没有被主线接受, 而一个非常相似的补丁已经存在 Android 中用于命名匿名 vma 很多年了, 参见 [ANDROID: mm: add a field to store names for private anonymous memory](https://github.com/aosp-mirror/kernel_common/commit/533e4ed309748abb89ae53548af7afe1649ac48a). 因此他[接替了 Colin Cross 的工作, 并继续发到了 v7 版本](https://lore.kernel.org/linux-mm/20200901161459.11772-1-sumit.semwal@linaro.org), [Kees Cook](https://lore.kernel.org/linux-mm/5d0358ab-8c47-2f5f-8e43-23b89d6a8e95@intel.com/) 对这个补丁提出了担忧, 他注意到这个补丁缺少字符串清理功能, 并且使用了来自内核的用户空间指针. 社区建议 strndup_user() 来自 userspace 的字符串, 执行适当的检查并将副本存储为 vm_area_struct 成员. fork() 期间额外 strdup() 的性能影响应该通过分配大量(64k)具有最长名称和定时 fork() 的 vma 来衡量.

接着, Suren Baghdasaryan 继续着这项工作. 实现了社区之间的 review 意见, 并优化了简单的重计数, 以避免在 fork() 期间 strdup() 名称所带来的性能开销. 最终于 v5.17 合入主线. 参见 LWN 报道 [Not-so-anonymous virtual memory areas](https://lwn.net/Articles/867818).

使用 CONFIG_ANON_VMA_NAME 开启这个功能. [用户空间](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9a10064f5625d5572c3626c1516e0bebc6c9fe9b)可以通过调用 [prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, start, len, (unsigned long) name) 来设置内存区域的名称](https://elixir.bootlin.com/linux/v5.17/source/kernel/sys.c#L2294). 使用 struct anon_vma_name 记录具名匿名页的名字, 记录在 [struct vm_area_struct->anon_name](https://elixir.bootlin.com/linux/v5.17/source/include/linux/mm_types.h#L423) 中. [其次](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9a10064f5625d5572c3626c1516e0bebc6c9fe9b)在 `/proc/pid/maps` 和 `/proc/pid/smaps` 中添加一个[字段](https://elixir.bootlin.com/linux/v5.17/source/fs/proc/task_mmu.c#L333), 以显示用户空间为匿名 vmas 提供的名称. 已命名匿名 vmas 的名称显示为 `[anon:anon_name]`.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2013/07/03 | Suren Baghdasaryan <surenb@google.com> | [mm: add sys_madvise2 and MADV_NAME to name vmas](https://lore.kernel.org/linux-mm/1372901537-31033-1-git-send-email-ccross@android.com) | 这组通过添加新的 MADVESE2 系统调用, 可以使用 MADV_NAME 来将名称附加到现有的 VMA 上.<br>1. 向每个 vma 添加一个包含名称字符串的 vma_name 结构, 系统中每个具有相同名称的 vma 都保证指向相同的 vma_name 结构. 可以直接通过比较指针进行名称相等比较.<br>2. 匿名 VMA 的名称在 `/proc/pid/maps` 中显示为 `[anon:<name>]`. 所有命名 VMA 的名称显示在 `/proc/pid/smap` 中的 Name 字段中用于命名 VMA.<br>3. 此修补程序添加的未命名 vma 的唯一成本是检查 vm_name 指针. 对于命名 vma, 它会将 refcount 更新添加到拆分/合并/复制 vma 中, 如果命名 vma 是具有该名称的最后一个vma, 则取消映射可能需要使用全局锁. | v2 ☐ | [PatchWork RFC](https://lore.kernel.org/linux-mm/1372901537-31033-1-git-send-email-ccross@android.com) |
| 2020/09/01 | Sumit Semwal <sumit.semwal@linaro.org> | [Anonymous VMA naming patches](https://lore.kernel.org/linux-mm/20200901161459.11772-1-sumit.semwal@linaro.org) | NA | v7 ☐ 5.9-rc3 | [PatchWork v7,0/3](https://lore.kernel.org/linux-mm/20200901161459.11772-1-sumit.semwal@linaro.org) |
| 2021/10/19 | Suren Baghdasaryan <surenb@google.com> | [Anonymous VMA naming patches](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=78db3412833dc9c479cd17412035f216cfd01a29) | NA | v11 ☑✓ 5.17-rc1 | [2021/08/27 PatchWork v8,0/3](https://patchwork.kernel.org/project/linux-mm/cover/20210827191858.2037087-1-surenb@google.com)<br>*-*-*-*-*-*-*-* <br>[2021/10/19 PatchWork v11,1/3](https://patchwork.kernel.org/project/linux-mm/patch/20211019215511.3771969-1-surenb@google.com) |


### 8.1.2 其他
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/05/16 | Jakub Matěna <matenajakub@gmail.com> | [Removing limitations of merging anonymous VMAs](https://patchwork.kernel.org/project/linux-mm/cover/20220218122019.130274-1-matenajakub@gmail.com/) | [Anonymous VMA merging improvements WIP](https://people.kernel.org/vbabka/anonymous-vma-merging-improvements-wip) | v1 ☐☑ | [2022/02/18 LORE v1,0/4](https://lore.kernel.org/r/20220218122019.130274-1-matenajakub@gmail.com)<br>*-*-*-*-*-*-*-* <br>[2022/05/16 LORE v3,0/6](https://lore.kernel.org/r/20220516125405.1675-1-matenajakub@gmail.com) |


## 8.2 Page Fault
-------

[linux那些事之gup_flags](https://zhikunhuo.blog.csdn.net/article/details/121712408) linux gup 子系统在处理各种 pin memory 中, 为了方便处理各种使用场景同时减少代码冗余, 使用了 gup_flags 标识位用于标记处理内存时各种场景.

[HZero 的内存管理专栏](https://blog.csdn.net/jasonactions/category_10652690.html) 其中有几篇详细讲解了 Page Fault 流程.

[Linux 内存管理 (10) 缺页中断处理](https://www.cnblogs.com/arnoldlu/p/8335475.html) 对各个处理函数有个简单的注解, 同时按照几个关键点(比如页面是否在内存中, pte 内容是否存在, 有无 vm_ops)区分 Page Fault 处理函数的流程. [Linux 内存管理：缺页异常 (一)](https://zhuanlan.zhihu.com/p/195580742) 则将这个框架流程绘制成了流程图的形式.

[ARM64 内存管理十五：do_page_fault 缺页中断](https://pzh2386034.github.io/Black-Jack/linux-memory/2019/09/15/ARM64内存管理十五-do_page_fault缺页中断)

[[原创](十四) Linux 内存管理之 page fault 处理](https://www.cnblogs.com/LoyenWang/p/12116570.html) 系统性地罗列了各个不同情形下 Page Fault 的处理框架.


| handler | 描述 |
|:-------:|:---:|
| do_anonymous_page | 用于[处理匿名页的缺页](https://elixir.bootlin.com/linux/v5.15/source/mm/memory.c#L4566)异常, 在以下情况下会触发:<br>1. malloc()/mmap() 分配了进程地址空间区域, 但是没有进行映射处理, 在首次访问时触发;<br>2. 用户栈不够的情况下, 进行栈区的扩大处理; |
| do_fault | 用于处理文件页异常, 包括以下三种情况:<br>1. [do_read_fault() 处理读文件页](https://elixir.bootlin.com/linux/v5.15/source/mm/memory.c#L4310)缺页<br>2. [do_cow_fault() 处理写私有文件页](https://elixir.bootlin.com/linux/v5.15/source/mm/memory.c#L4312)缺页;<br>3. [do_share_faults() 写共享文件页](https://elixir.bootlin.com/linux/v5.15/source/mm/memory.c#L4314)缺页; |
| do_swap_page | 如果访问 swap 页面出错(页面不在内存中), 则从 swap cache 或 swap file 中读取该页面. |
| do_numa_page | Numa Balancing 处理时是通过 fault driven 的方式统计页面访问的 NUMA 和 TASK 信息的, 这时候就需要把进程 VMA 中的页面设置为 PTE_PROT_NONE, 从而触发 Page Fault; |
| do_wp_page | 用于[处理写时复制(COW/Copy On Write)](https://elixir.bootlin.com/linux/v5.15/source/mm/memory.c#L4586), 会在以下两种情况处理:<br>1. 创建子进程时, 父子进程会以只读方式共享私有的匿名页和文件页, 当试图写的时候, 触发页错误异常, 从而复制物理页, 并创建映射;<br>2. 进程创建私有文件映射, 读访问后触发异常, 将文件页读入到 page cache 中, 并以只读模式创建映射, 之后发生写访问后, 触发 COW; |



|  时间  | 作者 | 特性  | 描述  |  是否合入主线  | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2022/05/05 | Peter Xu <peterx@redhat.com> | [mm: Avoid unnecessary page fault retires on shared memory types](https://patchwork.kernel.org/project/linux-mm/patch/20220505211748.41127-1-peterx@redhat.com/) | 638888 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220505211748.41127-1-peterx@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20220506135205.46810-1-peterx@redhat.com) |


### 8.2.1 零页
-------

|  时间  | 作者 | 特性  | 描述  |  是否合入主线  | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2007/10/16 | Nick Piggin <npiggin@suse.de> | [remove ZERO_PAGE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=557ed1fa2620dc119adb86b34c614e152a629a80) | TODO | v1 ☑✓ 2.6.24-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=557ed1fa2620dc119adb86b34c614e152a629a80) |
| 2008/06/20 | Linus Torvalds <torvalds@linux-foundation.org> | [Reinstate ZERO_PAGE optimization in 'get_user_pages()' and fix XIP](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=89f5b7da2a6bad2e84670422ab8192382a5aeb9f) | TODO | v1 ☐☑✓ | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=89f5b7da2a6bad2e84670422ab8192382a5aeb9f) |
| 2008/06/23 | Linus Torvalds <torvalds@linux-foundation.org> | [Fix ZERO_PAGE breakage with vmware](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=672ca28e300c17bf8d792a2a7a8631193e580c74) | TODO | v1 ☐☑✓ | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=672ca28e300c17bf8d792a2a7a8631193e580c74) |
| 2009/09/21 | Hugh Dickins <hugh.dickins@tiscali.co.uk> | [mm: reinstate ZERO_PAGE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=03f6462a3ae78f36eb1f0ee8b4d5ae2f7859c1d5) | 主要清理 get_user_pages() 标志, 但修复 munlock 的 OOM, 整理 "FOLL_ANON 优化", 并恢复零页面. <br>KAMEZAWA Hiroyuki 观察到早期内核的使能了 ZERO_PAGE, 但是引起了诸多错误, 因而在 2.6.24 时禁止了 do_anonymous_page() 中使用. 这组补丁恢复了 do_anonymous_page() 使用 ZERO_PAGE; 但是这一次不要用(map)计数更新来破坏它的结构页 cacheline. 让 vm_normal_page() 将其视为异常. | v1 ☑✓ 2.6.32-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=03f6462a3ae78f36eb1f0ee8b4d5ae2f7859c1d5) |



### 8.2.2 COW(写时拷贝)
-------

在 fork 进程的时候, 并不会为子进程直接分配物理页面, 而是使用 COW 的机制. 在 fork 之后, 父子进程都共享原来父进程的页面, 同时将父子进程的 COW mapping 都设置为只读. 这样当这两个进程触发写操作的时候, 触发缺页中断, 在处理缺页的过程中再为该进程分配出一个新的页面出来.

```cpp
// https://www.cnblogs.com/pengdonglin137/p/16131785.html
sys_fork
    -> kernel_clone
        -> copy_process
            -> copy_mm
                -> dup_mm
                    -> dup_mmap
                        -> copy_page_range
                            -> copy_p4d_range
                                -> 如果时PUD巨型页：copy_huge_pud: 分别将父子的PUD页表项设置为写保护
                                    -> pudp_set_wrprotect(src_mm, addr, src_pud);
                                    -> set_pud_at(dst_mm, addr, dst_pud, pud_mkold(pud_wrprotect(pud)));
                                -> copy_pmd_range
                                    -> 如果是PMD巨型页：copy_huge_pmd: 分别将父子的PMD页表项设置为写保护
                                        -> pmdp_set_wrprotect(src_mm, addr, src_pmd);
                                        -> set_pmd_at(dst_mm, addr, dst_pmd, pmd_mkold(pmd_wrprotect(pmd)));
                                    -> copy_pte_range
                                        -> copy_present_pte
                                            -> 如果：is_cow_mapping(vm_flags) && pte_write(pte)
                                                -> ptep_set_wrprotect(src_mm, addr, src_pte);
                                                -> set_pte_at(dst_vma->vm_mm, addr, dst_pte, pte_wrprotect(pte));
```

执行 fork 完毕后, 原先父进程中可写的区域在父子进程中都被设置了 CoW, 即 pte 中可写的属性被清除. 那么下次对此地址进行写操作时(不管是父进程还是子进程), 都会触发 PageFault. 然后新分配一个页面出来公其使用, 同时原来 pte 的可写属性也会被重新设置.


### 8.2.2.1 匿名页的写时拷贝
-------


### 8.2.2.2 文件页的写时拷贝
-------


### 8.2.2.3 写时拷贝的问题
-------


Dirty COW(CVE-2016-5195) 是近几年影响比较严重的问题, 参见 [Dirty COW and clean commit messages](https://lwn.net/Articles/704231). 也让内核开发者开始关注 COW 机制可能引起的安全问题.

随后在 2019 年Andrew Baumann, Jonathan Appavoo, Orran Krieger 和 Timothy Roscoe 在 Microsoft Research 网站上发表的一篇研究论文 [A fork() in the road](https://www.microsoft.com/en-us/research/uploads/prod/2019/04/fork-hotos19.pdf), 认为 fork() 系统调用是一个基本的设计错误. 他们认为 fork 作为一流的 OS 原语的持续存在阻碍了系统研究, 应该弃用它. 我们应该把 fork 当作历史文物来研究, 而不是作为进程的第一道工序创造机制. [LWN 上当时也对此进行了讨论](https://lwn.net/Articles/785430). 以及知乎上一些[(赵俊民)大佬的分析](https://zhuanlan.zhihu.com/p/272675052). 当时掀起了轩然大波, 讨论了很多方面, 包括性能, 安全等. 甚至有人质疑这篇论文是微软对 linux 发起的攻击.

2021 年 Redhat 的开发者 David Hildenbrand 总结了上游社区中存在的 COW 问题. 并发布在邮件列表中. [Summary of COW (Copy On Write) Related Issues in Upstream Linux](https://lore.kernel.org/all/3ae33b08-d9ef-f846-56fb-645e3b9b4c66@redhat.com). 随后在某个阳光明媚的周三进行的 linux-mm alignment session 中对这些问题进行了讨论:

| [CVE-2020-29374](https://nvd.nist.gov/vuln/detail/CVE-2020-29374)  | Observing Memory Modifications of Private Pages From A Child Process |
|:---------------:|:--------------------------------------------------------------------:|
| 参考资料 | 摘要: [Patching until the COWs come home (part 1)](https://lwn.net/Articles/849638).<br>详情: [Patching until the COWs come home (part 2)](https://lwn.net/Articles/849876). |
| 影响 | 一旦 fork(), 进程私有内存可能不像你想的那样私有, 子进程仍然可以观察到父进程中私有内存区域的连续修改, 例如, 通过使用 vmsplice() + munmap(). |
| 核心问题 | 将可读页面固定在子进程中(比如通过 vmsplice 系统调用), 可能会导致子进程观察父进程所做的内存修改, 而子进程不应该观察父进程. |

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/01/26 | David Hildenbrand <david@redhat.com> | [mm: COW fixes part 1: fix the COW security issue for THP and hugetlb](https://patchwork.kernel.org/project/linux-mm/cover/20211217113049.23850-1-david@redhat.com) | NA | v1 ☐ | [PatchWork v1,00/11](https://patchwork.kernel.org/project/linux-mm/cover/20211217113049.23850-1-david@redhat.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/9](https://lore.kernel.org/r/20220126095557.32392-1-david@redhat.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3,0/9](https://lore.kernel.org/r/20220131162940.210846-1-david@redhat.com) |
| 2022/03/15 | David Hildenbrand <david@redhat.com> | [mm: COW fixes part 2: reliable GUP pins of anonymous pages](https://patchwork.kernel.org/project/linux-mm/cover/20220224122614.94921-1-david@redhat.com/) | 617540 | v1 ☐☑ | [2022/02/24 LORE v1,00/13](https://lore.kernel.org/r/20220224122614.94921-1-david@redhat.com))<br>*-*-*-*-*-*-*-* <br>[2022/03/08 LORE v1,00/15](https://lore.kernel.org/r/20220308141437.144919-1-david@redhat.com)<br>*-*-*-*-*-*-*-* <br>[2022/03/15 LORE v2,00/15](https://lore.kernel.org/r/20220315104741.63071-1-david@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,00/16](https://lore.kernel.org/r/20220329160440.193848-1-david@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,00/17](https://lore.kernel.org/r/20220428083441.37290-1-david@redhat.com) |
| 2022/03/15 | David Hildenbrand <david@redhat.com> | [mm: COW fixes part 3: reliable GUP R/W FOLL_GET of anonymous pages](https://patchwork.kernel.org/project/linux-mm/cover/20220315141837.137118-1-david@redhat.com/) | 623540 | v1 ☐☑ | [LORE v1,0/7](https://lore.kernel.org/r/20220315141837.137118-1-david@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/8](https://lore.kernel.org/r/20220329164329.208407-1-david@redhat.com) |


| [CVE-2020-29374](https://nvd.nist.gov/vuln/detail/CVE-2020-29374)  | Intra Process Memory Corruptions due to Wrong COW (FOLL_GET) |
|:---------------:|:--------------------------------------------------------------------:|
| 参考资料 | NA |
| 影响 | NA |
| 根本问题 | NA |



### 8.2.3 enhance
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/09/10 | Catalin Marinas <catalin.marinas@arm.com> | [arm64: Add support for hardware updates of the access and dirty pte bits](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1436545468-1549-1-git-send-email-catalin.marinas@arm.com) | ARMv8.1 体系结构扩展引入了对页表项中访问和脏信息的硬件更新的支持, 用于支持硬件自动原子地完成页表更新("读-修改-回写").<br>TCR_EL1.HA 为 1, 则使能硬件的自动更新访问位. 当处理器访问内存地址时, 硬件自动设置 PTE_AF 位, 而不是再触发访问位标志错误.<br>TCR_EL1.HD 为 1, 则使能硬件的脏位管理.  | v1 ☑ [4.3-rc1](https://kernelnewbies.org/Linux_4.3#Architectures) | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/344775)<br>*-*-*-*-*-*-*-* <br>[PatchWork](https://lore.kernel.org/patchwork/patch/344816), [commit 2f4b829c625e](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2f4b829c625ec36c2d80bef6395c7b74cea8aac0) |
| 2017/07/25 | Catalin Marinas <catalin.marinas@arm.com> | [arm64: Fix potential race with hardware DBM in ptep_set_access_flags()](https://patchwork.kernel.org/project/linux-arm-kernel/patch/20170725135308.18173-2-catalin.marinas@arm.com) | 修复前面支持 TCP_EL1.HA/HD 引入的硬件和软件的竞争问题 | RFC ☐ | [PatchWork RFC](https://patchwork.kernel.org/project/linux-mm/cover/20200224203057.162467-1-walken@google.com) |
| 2022/03/15 | Bibo Mao <maobibo@loongson.cn> | [[2/2] mm: add access/dirty bit on numa page fault](https://patchwork.kernel.org/project/linux-mm/patch/20220315092323.620610-1-maobibo@loongson.cn/) | 在 x86/arm 等支持 hw 页面行走的平台上, access 和 dirty bit 由 hw 设置, 而在一些没有这些 hw 功能的平台上, access 和 dirty bit 由 next trap 中的软件设置.<br>在 numa 页面故障期间, 如果写入故障时迁移失败, 可以为旧 pte 添加脏位. 如果迁移成功, 可以为迁移后的新 pte 增加访问位, 也可以为写错误增加脏位. | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20220315092323.620610-1-maobibo@loongson.cn)<br>*-*-*-*-*-*-*-* <br>[LORE v2](https://lore.kernel.org/r/20220317065033.2635123-1-maobibo@loongson.cn) |

### 8.2.4 userfaultfd
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/06/19 | Nadav Amit <nadav.amit@gmail.com> | [userfaultfd: support access/write hints](https://patchwork.kernel.org/project/linux-mm/cover/20220619233449.181323-1-namit@vmware.com/) | 651866 | v2 ☐☑ | [LORE v2,0/5](https://lore.kernel.org/r/20220619233449.181323-1-namit@vmware.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/5](https://lore.kernel.org/r/20220622185038.71740-1-namit@vmware.com) |



### 8.2.5 MMAP locking
-------

2013 年, [LSFMM: Problems with mmap_sem](https://lwn.net/Articles/548098)

2015 年, [Topics of interest from the MM summit](https://events.static.linuxfound.org/sites/events/files/slides/mm.pdf)

2017 年, [Another attempt at speculative page-fault handling](https://lwn.net/Articles/730531)

2018 年, [Zone-lock and mmap_sem scalability](https://lwn.net/Articles/753269), [The LRU lock and mmap_sem](https://lwn.net/Articles/753058)

2019 年, [How to get rid of mmap_sem](https://lwn.net/Articles/787629)

2021 年, [Introducing maple trees](https://lwn.net/Articles/845507)

2021 年, [[LSF/MM TOPIC] mmap locking topics](https://www.spinics.net/lists/linux-mm/msg258803.html)

2022 年 [The ongoing search for mmap_lock scalability](https://lwn.net/Articles/893906)


*   mmap_sem Scalability


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/03/21 | Yang Shi <yang.shi@linux.alibaba.com> | [Drop mmap_sem during unmapping large map](https://lore.kernel.org/all/1521581486-99134-1-git-send-email-yang.shi@linux.alibaba.com) | 1521581486-99134-1-git-send-email-yang.shi@linux.alibaba.com | v1 ☐☑✓ | [LORE v1,0/8](https://lore.kernel.org/all/1521581486-99134-1-git-send-email-yang.shi@linux.alibaba.com) |


*   SPF(Speculative page faults)



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2010/01/04 | Peter Zijlstra <a.p.zijlstra@chello.nl> | [Speculative pagefault -v3](http://lore.kernel.org/patchwork/patch/183997) | SPF | RFC v3 ☐  | [PatchWork](https://lore.kernel.org/patchwork/patch/183997) |
| 2019/04/16 | Laurent Dufour <ldufour@linux.vnet.ibm.com> | [Speculative page faults](http://lore.kernel.org/patchwork/patch/1062659) | SPF | v12 ☐  | [PatchWork v12,00/31](https://lore.kernel.org/patchwork/patch/1062659) |
| 2022/01/28 | Michel Lespinasse <michel@lespinasse.org> | [Speculative page faults (anon vmas only)](http://lore.kernel.org/patchwork/patch/1420569) | SPF | v11 ☐  | [PatchWork RFC,00/37](https://lore.kernel.org/patchwork/patch/1408784)<br>*-*-*-*-*-*-*-* <br>[PatchWork v1](https://lore.kernel.org/patchwork/patch/1420569)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,00/35](https://patchwork.kernel.org/project/linux-mm/cover/20220128131006.67712-1-michel@lespinasse.org) |

* Fine grained MM locking


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2013/01/31 | Michel Lespinasse <walken@google.com> | [Mapping range lock](https://lore.kernel.org/patchwork/patch/356467) | 文件映射的 mapping lock | RFC ☐  | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/356467) |
| 2020/02/24 | Michel Lespinasse <walken@google.com> | [Fine grained MM locking](https://patchwork.kernel.org/project/linux-mm/cover/20200224203057.162467-1-walken@google.com) | 细粒度 MM MMAP lock | RFC ☐  | [PatchWork RFC](https://patchwork.kernel.org/project/linux-mm/cover/20200224203057.162467-1-walken@google.com), [fine_grained_mm.pdf](https://linuxplumbersconf.org/event/4/contributions/556/attachments/304/509/fine_grained_mm.pdf) |

* Maple Tree


[Maple Tree "RFC" Patches Sent Out As New Data Structure To Help With Linux Performance](https://www.phoronix.com/scan.php?page=news_item&px=Maple-Tree-Linux-RFC)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/04/26 | Liam Howlett <liam.howlett@oracle.com> | [Introducing the Maple Tree](https://lore.kernel.org/patchwork/patch/1477973) | Maple Tree 是一种基于 RCU 安全范围的 B树, 旨在高效使用现代处理器缓存. 在内核中有许多地方, 基于范围的非重叠树是有益的, 尤其是具有简单接口的树. Maple Tree 的第一个用户是 vm_area_struct, 当前替换了三个结构: 增强 rbtree、vma 缓存和 mm_struct 中的 vma linked 链表. 长期目标是减少或消除 mmap_sem 争用. | v9 ☐ | [2021/08/17 PatchWork v2,00/61](https://patchwork.kernel.org/project/linux-mm/cover/20210817154651.1570984-1-Liam.Howlett@oracle.com)<br>*-*-*-*-*-*-*-* <br>[2021/10/05 PatchWork v3](https://patchwork.kernel.org/project/linux-mm/cover/20211005012959.1110504-1-Liam.Howlett@oracle.com)<br>*-*-*-*-*-*-*-* <br>[2021/12/01 PatchWork v4,00/66](https://patchwork.kernel.org/project/linux-mm/cover/20211201142918.921493-1-Liam.Howlett@oracle.com)<br>*-*-*-*-*-*-*-* <br>[2022/02/02 PatchWork v5,00/70](https://patchwork.kernel.org/project/linux-mm/cover/20220202024137.2516438-1-Liam.Howlett@oracle.com)<br>*-*-*-*-*-*-*-* <br>[2022/04/04 LORE v7,00/70](https://lore.kernel.org/r/20220404143501.2016403-1-Liam.Howlett@oracle.com)<br>*-*-*-*-*-*-*-* <br>[2022/04/26 LORE v8,0/70](https://lore.kernel.org/r/20220426150616.3937571-1-Liam.Howlett@oracle.com)<br>*-*-*-*-*-*-*-* <br>[LORE v9,0/69](https://lore.kernel.org/r/20220504010716.661115-1-Liam.Howlett@oracle.com)<br>*-*-*-*-*-*-*-* <br>[2022/06/21 LORE v10,0/69](https://lore.kernel.org/all/20220621204632.3370049-1-Liam.Howlett@oracle.com)<br>*-*-*-*-*-*-*-* <br>[2022/07/17 LORE v11,0/69](https://lore.kernel.org/r/20220717024615.2106835-1-Liam.Howlett@oracle.com) |
| 2022/05/04 | Liam Howlett <Liam.Howlett@Oracle.com> | [Prepare for maple tree](https://patchwork.kernel.org/project/linux-mm/cover/20220504002554.654642-1-Liam.Howlett@oracle.com/) | 638130 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220504002554.654642-1-Liam.Howlett@oracle.com) |


## 8.3 反向映射 RMAP(Reverse Mapping)
-------

[郭健:  Linux内存逆向映射(reverse mapping) 技术的前世今生](https://blog.51cto.com/u_15015138/2557286)

[Reverse-Mapping VM Performance](https://www.usenix.org/conference/2003-linux-kernel-developers-summit/presentation/reverse-mapping-vm-performance)

[Virtual Memory Management Techniques in 2.6 Linux kernel and challenges](https://www.researchgate.net/publication/348620478_Virtual_Memory_Management_Techniques_in_26_Linux_kernel_and_challenges)

[OLS2003](https://www.kernel.org/doc/ols/2003)

[linux内存管理-反向映射](https://blog.csdn.net/faxiang1230/article/details/106609834)

[如何知道物理内存中的某个页帧属于某个进程, 或者说进程的某个页表项?](https://www.zhihu.com/question/446137543)

RMAP 反向映射是一种物理地址反向映射虚拟地址的方法.

正向映射是通过虚拟地址根据页表找到物理内存, 而反向映射就是通过物理地址或者物理页面找到哪些进程或者哪些虚拟地址使用它.

那内核在哪些路径下需要进行反向映射呢?

反向映射的典型应用场景:

1.  kswapd 进行页面回收时, 需要断开所有映射了该匿名页面的PTE表项;

2.  页面迁移时, 需要断开所有映射了该匿名页面的PTE表项;

即一个物理页面是可以被多个进程映射到自己的虚拟地址空间. 在整个内存管理的过程中, 有些页面可能被迁移, 有些则可能要被释放、写回到磁盘或者 交换到 SWAP 空间(磁盘) 上, 那么这时候经常要找到哪些进程映射并使用了这个页面.

*   2.4 的 reverse mapping


在 Linux 2.4 的时代, 为了确定映射了某个页面的所以进程, 我们不得不遍历每个进程的页表, 这是一项费时费力的工作.

*   PTE-based reverse mapping

于是在 2002 年 [Linux 2.5.27](https://kernelnewbies.org/LinuxVersions) 内核开发者实现了一种低开销的[基于 PTE 的的反向映射方方案(low overhead pte-based reverse mapping scheme)](http://lastweek.io/notes/rmap) 来解决这一问题. 经过了不断的发展, 到 2.5.33 趋于完善.

这个版本的实现相当直接, 在每个物理页(包括匿名页和文件映射页)(的通过结构体类型 struct page ) 中维护了一个名为 [pte_chain 的链表](https://elixir.bootlin.com/linux/v2.5.27/source/include/linux/mm.h#L161), 里面包含所有引用该页的页表项, 链表的每一项都直接指向映射该物理页面的页表项 PTE 的指针.

该机制在大多数情况下都很高效, 但其自身也存在一些问题. 链表中保存反向映射信息的节点很多, 占用了大量内存, 并且 pte_chain 链表本身的维护的工作量也十分繁重, 并且处理速度随着物理页面的增多而变得缓慢. 以 fork() 系统调用为例, 由于需要为进程地址空间中的每个页面都添加新的反向映射项, 所以处理较慢, 影响了进程创建的速度. 关于这个方案的详细实现可以参考 [PATCH 2.5.26-rmap-1-core](http://loke.as.arizona.edu/~ckulesa/kernel/rmap-vm/2.5.26/2.5.26-rmap-1-core) 或者 [CODE mm/rmap.c, v2.5.27](https://elixir.bootlin.com/linux/v2.5.27/source/mm/rmap.c). 论文参见 [Object-based Reverse Mapping](https://landley.net/kdocs/ols/2004/ols2004v2-pages-71-74.pdf)

社区一直在努力试图优化 (PTE-based) RMAP 的整体效率.

*   object-based reverse mapping(objrmap)

在 2003 年, IBM 的 Dave McCracken 提出了一种新的解决方法 [**基于对象的反向映射**("object-based reverse mapping")](https://lwn.net/Articles/23732), 简称 objrmap. 虽然这组补丁当时未合入主线, 但是向社区证实了, 从 struct page 找到映射该物理页的页表项 PTE. 该方法虽然存在一些问题, 但是测试证明可以显著解决 RMAP 在部分场景的巨大开销问题. 参见 [Performance of partial object-based rmap](https://lkml.org/lkml/2003/2/19/235).

用户空间进程的页面主要有两种, 一种是 file mapped page, 另外一种是 anonymous mapped page. Dave McCracken 的 objrmap 方案虽然性能不错(保证逆向映射功能的基础上, 同时又能修正 rmap 带来的各种问题). 但是只是适用于 file mapped page, 对于匿名映射页面, 这个方案无能为力.

因此 Hugh Dickins 在 [2003 年](https://lore.kernel.org/patchwork/patch/14844) ~ [2004 年](https://lore.kernel.org/patchwork/patch/22938)的提交了[一系列补丁](https://lore.kernel.org/patchwork/patch/23565), 试图在 Dave McCracken 补丁的基础上, 进一步优化 RMAP 在匿名映射页面上的问题.

与此同时, SUSE 的 Andrea Arcangeli 当时正在做的工作就是让 32-bit 的 Linux 运行在配置超过 32G 内存的公司服务器上. 在这些服务器上往往启动大量的进程, 共享了大量的物理页帧, 消耗了大量的内存. 对于 Andrea Arcangeli 来说, 内存消耗的罪魁祸首是非常明确的 : 反向映射模块, 这个模块消耗了太多的 low memory, 从而导致了系统的各种 crash. 为了让自己的工作继续推进, 他必须解决 rmap 引入的内存扩展性 (memory scalability) 问题, 最直接有效的思路同样是在 Dave McCracken's objrmap 的基础上, 为匿名映射页面设计一种基于对象的逆向映射机制. 最后 Andrea Arcangeli [设计了 full objrmap 方案](https://lwn.net/Articles/77106). 并在与 Hugh Dickins 类似方案的 PK 中胜出. 并最终在 [v2.6.7](https://elixir.bootlin.com/linux/v2.6.7/source/mm/rmap.c#322) 合入主线.

> 小故事:
>
> 关于 32-bit 的 Linux 内核是否支持 4G 以上的 memory. 在 1999 年, Linus 的决定是 :
>
> 1999年, linus说:32位linux永远不会, 也不需要支持大于2GB的内存, 没啥好商量的.
>
> 不过历史的洪流不可阻挡, 处理器厂商设计了扩展模块以便寻址更多的内存, 高端的服务器也配置了越来越多的内存.
>
> 这也迫使 Linus 改变之前的思路, 让 Linux 内核支持更大的内存.


通过新增结构 anon_vma, 类似于重用 address_space 的想法, 即拥有一个数据结构 trampoline.
一个 VMA 中的所有页面只共享一个 anon_vma. vma->anon_vma 表示 vma 是否映射了页面. 相关的处理在 do_anonymous_fault()-=>anon_vma_prepare().

相对于基本的基于PTE-chain的解决方案, 基于对象的 RMAP :

| 比较 | 描述 |
|:---:|:----:|
| 优势 |  在页面错误期间, 我们只需要设置page->映射为指向anon_vma结构, 而不是分配一个新结构并插入.
| 不足 | 在rmap遍历期间, 我们需要额外的计算来遍历每个VMA的页表, 以确保该页实际上被映射到这个特定的VMA中. |


*    anon_vma_chain(AVC-per process anon_vma)

旧的机制下, 多个进程共享一个 AV(anon_vma), 这可能导致大量分支工作负载的可伸缩性问题. 具体来说, 由于每个 anon_vma 将在父进程和它的所有子进程之间共享.

1.  这样在进程 fork 子进程的场景下, 如果进行 COW, 父子进程原本共享的 page frame 已经不再共享, 然而, 这两个 page 却仍然指向同一个 anon_vma.

2.  即使一开始就没有在父子进程之间共享的页面, 当首次访问的时候(无论是父进程还是子进程), 通过 do_anonymous_page 函数分配的 page frame 也是同样的指向一个 anon_vma.


在有些网路服务器中, 系统非常依赖fork, 某个服务程序可能会fork巨大数量的子进程来处理服务请求, 在这种情况下, 系统性能严重下降. Rik van Riel给出了一个具体的示例: 系统中有1000进程, 都是通过fork生成的, 每个进程的VMA有 1000个匿名页. 根据目前的软件架构, anon_vma链表中会有1000个vma 的节点, 而系统中有一百万个匿名页面属于同一个anon_vma.


这可能导致这样的系统 : 一个 CPU 遍历 page_referenced_one 中的 1000 个进程的页表, 而其他所有 CPU 都被 anon_vma 锁卡住. 这将导致像 AIM7 这样的基准测试出现灾难性的故障, 在那里进程的总数可能达到数万个. 实际工作量仍然比AIM7少10倍, 但它们正在迎头赶上.

这个补丁改变了 anon_vmas 和 VMA 的链接方式, 它允许我们将多个anon_vmas与一个VMA关联起来. 在分叉时, 每个子进程都获得自己的anon_vmas, 它的 COW 页面将在其中实例化. 父类的anon_vma也链接到VMA, 因为非cowed页面可以出现在任何子类中. 这将 1000 个子进程的页面的 RMAP 扫描复杂度降低到 O(1), 系统中最多 1/N 个页面的 RMAP 扫描复杂度为 O(N). 这将把繁重的工作负载从O(N)减少到很小的平均扫描成本.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2002/07/20 | Rik van Riel <riel@redhat.com> | [VM with reverse mappings](https://www.cs.helsinki.fi/linux/linux-kernel/2002-46/0281.html) | 反向映射 RMAP 机制 | ☑ 2.5.27 | [Patch Archive](http://loke.as.arizona.edu/~ckulesa/kernel/rmap-vm/2.5.31/readme.html), [LWN](https://lwn.net/Articles/2925) |
| 2003/02/20 | Dave McCracken <dmccr@us.ibm.com> | [The object-based reverse-mapping VM](https://lwn.net/Articles/23732) | 基于对象的反向映射技术 | ☐ 2.5.62 in Andrew Morton's tree | [LWN](https://lwn.net/Articles/23584) |
| 2003/04/02 | Dave McCracken <dmccr@us.ibm.com> | [Optimizaction for object-based rmap](https://lore.kernel.org/patchwork/patch/15415) | 优化基于对象的反向映射 | ☐ 2.5.66 | [PatchWork](https://lore.kernel.org/patchwork/patch/15415) |
| 2003/03/20 | Hugh Dickins <hugh@veritas.com> | [anobjrmap 0/6](https://lore.kernel.org/patchwork/patch/14844) | 扩展了Dave McCracken的objrmap来处理匿名内存 | ☐ 2.5.66 | [PatchWork for 6 patches 2003/03/20](https://lore.kernel.org/patchwork/patch/14844)<br>*-*-*-*-*-*-*-* <br>[PatchWork for 2.6.5-rc1 8 patches 2004/03/18](https://lore.kernel.org/patchwork/patch/22938)<br>*-*-*-*-*-*-*-* <br>[PatchWork for 2.6.5-mc2 40 patches 2004/04/08](https://lore.kernel.org/patchwork/patch/23565) |
| 2004/03/10 | Andrea Arcangeli <andrea@suse.de> | [Virtual Memory II: full objrmap](https://lwn.net/Articles/75198) | 基于对象的反向映射技术的有一次尝试 | v1 ☑ [2.6.7](https://elixir.bootlin.com/linux/v2.6.7/source/mm/rmap.c#322) | [230-objrmap fixes for 2.6.3-mjb2](https://lore.kernel.org/patchwork/patch/22524)<br>*-*-*-*-*-*-*-* <br>[objrmap-core-1](https://lore.kernel.org/patchwork/patch/22623)<br>*-*-*-*-*-*-*-* <br>[RFC anon_vma previous (i.e. full objrmap)](https://lore.kernel.org/patchwork/patch/22674)<br>*-*-*-*-*-*-*-* <br>[anon_vma RFC2](https://lore.kernel.org/patchwork/patch/22694)<br>*-*-*-*-*-*-*-* <br>[anon-vma](https://lore.kernel.org/patchwork/patch/23307) |
| 2009/11/24 | Hugh Dickins <hugh.dickins@tiscali.co.uk> | [ksm: swapping](https://lore.kernel.org/patchwork/patch/179157) | 内核Samepage归并(KSM)是Linux 2.6.32 中合并的一个特性, 它对虚拟客户机的内存进行重复数据删除. 但是, 该实现不允许交换共享的页面. 这个版本带来了对KSM页面的交换支持.<br>这个对 RMAP 的影响是, RMAP 支持 KASM 页面. 同时用 rmap_walk 的方式 | v1 ☑ [2.6.33-rc1](https://kernelnewbies.org/Linux_2_6_33#Swappable_KSM_pages) | [PatchWork v1](https://lore.kernel.org/patchwork/patch/179157) |
| 2010/01/14 | Rik van Riel <riel@redhat.com> | [change anon_vma linking to fix multi-process server scalability issue](https://lwn.net/Articles/75198) | 引入 AVC, 实现了 per process anon_vma, 每一个进程的 page 都指向自己特有的 anon_vma 对象. | v1 ☑ [2.6.34](https://kernelnewbies.org/Linux_2_6_34#Various_core_changes) | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/185210)<br>*-*-*-*-*-*-*-* <br>[PatchWork v1](https://lore.kernel.org/patchwork/patch/186566) |
| 2013/12/4 | Rik van Riel <riel@redhat.com> | [mm/rmap: unify rmap traversing functions through rmap_walk](https://lwn.net/Articles/424318) | 引入 AVC, 实现了 per process anon_vma, 每一个进程的 page 都指向自己特有的 anon_vma 对象. | v1 ☑ 3.4-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/424318) |
| 2012/12/01 | Davidlohr Bueso <davidlohr@hp.com> | [mm/rmap: Convert the struct anon_vma::mutex to an rwsem](https://lore.kernel.org/patchwork/patch/344816) | 将struct anon_vma::mutex 转换为rwsem, 这将有效地改善页面迁移过程中对 VMA 的访问的竞争问题. | v1 ☑ 3.8-rc1 | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/344775)<br>*-*-*-*-*-*-*-* <br>[PatchWork](https://lore.kernel.org/patchwork/patch/344816) |
| 2014/05/23 | Davidlohr Bueso <davidlohr@hp.com> | [mm: i_mmap_mutex to rwsem](https://lore.kernel.org/patchwork/patch/466974) | 2012 年 Ingo 将 anon-vma lock 从 mutex 转换到了 rwsem 锁. i_mmap_mutex 与 anon-vma 有类似的职责, 保护文件支持的页面. 因此, 我们可以使用类似的锁定技术 : 将互斥锁转换为rwsem, 并在可能的情况下共享锁. | RFC ☐ | [PatchWork](https://lore.kernel.org/patchwork/patch/466974) |
| 2014/05/23 | Davidlohr Bueso <davidlohr@hp.com> | [mm/rmap.c: Avoid double faults migrating device private pages](https://patchwork.kernel.org/project/linux-mm/patch/20211018045247.3128058-1-apopple@nvidia.com) | 在迁移过程中, 将为要迁移的每个页面安装特殊的页面表条目. 这些条目存储 pfn 和映射被混合页面的 PTE 的相关权限.<br>设备专用页使用特殊的交换 pte 项来区分只读页和可写页, 迁移代码在创建迁移项时会检查这些页. 通常, 这遵循 migrate_vma_collect_pmd() 中的快速路径, 该路径在将页面迁移回CPU时将设备专用页面的权限正确复制到迁移条目.<br>但是, 慢的路径是使用 try_to_migrate(), 它无条件地为设备专用页创建只读迁移条目. 这会导致 CPU 上出现不必要的双重错误, 因为新页面始终以只读方式映射, 即使它们可以被映射为可写. 通过在 try_to_migrate_one() 中正确复制设备私有权限来修复此问题. | v1 ☐ | [PatchWork](https://patchwork.kernel.org/project/linux-mm/patch/20211018045247.3128058-1-apopple@nvidia.com) |


## 8.4 mremap 重新映射虚拟地址
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| NA | Linus Torvalds <torvalds@linuxfoundation.org> | [Import 1.3.78](https://lore.kernel.org/patchwork/patch/1324435) | filemap 的批量内存分配和拷贝. | v4 ☑ 1.3.78 | [COMMIT HISTOR](http://github.com/gatieme/linux-history/commit/24747a70c81a376046a1ad00422142a6bb8ed7e9) |
| 2020/11/08 | Linus Torvalds <torvalds@linuxfoundation.org> | [Add support for fast mremap](https://patchwork.kernel.org/project/linux-mm/cover/20181108181201.88826-1-joelaf@google.com) | mremap PMD 映射优化. 在内存管理相关的操作中, Android 需要 mremap 大区域的内存. 如果不启用 THP, mremap 系统调用会非常慢. 瓶颈是 move_page_tables, 它一次复制每个 pte, 在一个大的映射中可能非常慢. 开启 THP 可能不是一个可行的选择, 也不适合我们. 因此实现了 fast remap 通过尽可能在PMD级别进行复制来加快非THP系统的性能. 速度在 x86 上是提升了一个数量级(~20倍). 在 1GB 的mremap上, mremap完成时间从 3.4-3.6 毫秒下降到 144-160 微秒. | v4 ☑ 1.3.78 | [PatchWork v1,1/2](https://patchwork.kernel.org/project/linux-mm/patch/20181012013756.11285-1-joel@joelfernandes.org)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/3](https://patchwork.kernel.org/project/linux-mm/cover/20181013013200.206928-1-joel@joelfernandes.org)<br>*-*-*-*-*-*-*-* <br>[PatchWork v5,0/3](https://patchwork.kernel.org/project/linux-mm/cover/20181108181201.88826-1-joelaf@google.com)<br>*-*-*-*-*-*-*-* <br>[COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2c91bd4a4e2e530582d6fd643ea7b86b27907151) |
| 2020/10/14 | Kalesh Singh <kaleshsingh@google.com> | [Speed up mremap on large regions](https://lore.kernel.org/patchwork/patch/1321018) | mremap PUD 映射优化, 可以加速映射大块地址时的速度. 如果源地址和目的地址是 PMD/PUD 对齐和 PMD/PUD 大小, mremap 的耗时时间可以通过移动 PMD/PUD 级别的表项来优化. 允许在 arm64 和 x86 上的 PMD 和 PUD 级别移动. 其他支持这种类型移动且已知安全的体系结构也可以通过启用 `HAVE_MOVE_PMD` 和 `HAVE_MOVE_PUD` 来选择这些优化. | v4 ☑ | [PatchWork v4,0/5](https://patchwork.kernel.org/project/linux-mm/cover/20201014005320.2233162-1-kaleshsingh@google.com) |
| 2020/10/14 | Kalesh Singh <kaleshsingh@google.com> | [Speedup mremap on ppc64](https://patchwork.kernel.org/project/linux-mm/cover/20210616045735.374532-1-aneesh.kumar@linux.ibm.com) | NA | v8 ☑ 5.14-rc1 | [PatchWork v8,0/3](https://patchwork.kernel.org/project/linux-mm/cover/20201014005320.2233162-1-kaleshsingh@google.com) |


## 8.5 ioremap
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2006/08/10 | Haavard Skinnemoen <hskinnemoen@atmel.com> | [Generic ioremap_page_range: introduction](https://lore.kernel.org/patchwork/patch/62430) | 基于i386实现的 ioremap_page_range() 的通用实现, 将 I/O 地址空间映射到内核虚拟地址空间. | v1 ☑ 2.6.19-rc1 | [PatchWork 0/14](https://lore.kernel.org/patchwork/patch/62430) |
| 2015/03/03 | Toshi Kani <toshi.kani@hp.com> | [Kernel huge I/O mapping support](https://lore.kernel.org/patchwork/patch/547056) | ioremap() 支持透明大页. 扩展了 ioremap() 接口, 尽可能透明地创建具有大页面的 I/O 映射. 当一个大页面不能满足请求范围时, ioremap() 继续使用 4KB 的普通页面映射. 使用 ioremap() 不需要改变驱动程序. 但是, 为了使用巨大的页面映射, 请求的物理地址必须以巨面大小(x86上为 2MB 或 1GB)对齐. 内核巨页的 I/O 映射将提高 NVME 和其他具有大内存的设备的性能, 并减少创建它们映射的时间. | v3 ☑ 4.1-rc1 | [PatchWork v3,0/6](https://lore.kernel.org/patchwork/patch/547056) |
| 2015/05/15 | Haavard Skinnemoen <hskinnemoen@atmel.com> | [mtrr, mm, x86: Enhance MTRR checks for huge I/O mapping](https://lore.kernel.org/patchwork/patch/943736) | 增强了对巨页 I/O 映射的 MTRR 检查.<br>1. 允许 pud_set_huge() 和 pmd_set_huge() 创建一个巨页映射, 当范围被任何内存类型的单个MTRR条目覆盖时. <br>2. 当指定的 PMD 映射范围超过一个 MTRR 条目时, 记录 pr_warn_once() 消息. 当这个范围被 MTRR 覆盖时, 驱动程序应该发出一个与单个 MTRR 条目对齐的映射请求. | v5 ☐ | [PatchWork v5,0/6](https://lore.kernel.org/patchwork/patch/943736) |




# 9 内存控制组 MEMCG (Memory Cgroup)支持
-------

[KS2012: The memcg/mm minisummit](https://lwn.net/Articles/516439)

[Controlling memory use in containers](https://lwn.net/Articles/243795)

**2.6.25(2008年4月发布)**

在Linux轻量级虚拟化的实现 container 中(比如现在挺火的Docker, 就是基于container), 一个重要的功能就是做资源隔离. Linux 在 2.6.24中引入了cgroup(control group, 控制组)的资源隔离基础框架(将在最后一个部分详述), 提供了资源隔离的基础.

在2.6.25 中, 内核在此基础上支持了内存资源隔离, 叫内存控制组. 它使用可以在不同的控制组中, 实施内存资源控制, 如分配, 内存用量, 交换等方面的控制.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/08/24 | Balbir Singh <balbir@linux.vnet.ibm.com> | [Memory controller containers setup (v7)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e1a1cd590e3fcb0d2e230128daf2337ea55387dc) | 实现 MEMCG | v7 ☑ [2.6.25-rc1](https://kernelnewbies.org/Linux_2_6_25) | [PatchWork v7](https://lore.kernel.org/lkml/20070824152009.16582.78904.sendpatchset@balbir-laptop) |
| 2010/09/01 | KAMEZAWA Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com> | [memcg: towards I/O aware memcg v7.](https://lore.kernel.org/patchwork/patch/213968) | IO 感知的 MEMCG. | v7 ☐ | [PatchWork v7](https://lore.kernel.org/patchwork/patch/213968) |
| 2009/09/25 | KAMEZAWA Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com> | [memcg updates v5](https://lore.kernel.org/patchwork/patch/129608) | IO 感知的 MEMCG. | v7 ☐ | [PatchWork 0/12](https://lore.kernel.org/patchwork/patch/129608) |
| 2009/06/15 | Balbir Singh <balbir@linux.vnet.ibm.com>| [Remove the overhead associated with the root cgroup](https://lore.kernel.org/patchwork/patch/160500) | 通过删除与 root cgroup 有关的开销来降低 mem cgroup 的开销<br>1. 删除了与计算 root cgroup中所有页面相关的开销. 作为一个副作用, 我们不能再在 root cgroup中设置内存硬限制.<br>2. 添加了一个新的标记 PCG_ACCT_LRU, 用于跟踪页面是否已被计入. page_cgroup的标记现在被原子地设置, pcg_default_flags 现在已经过时并被删除. | v5 ☑ 2.6.32-rc1 | [PatchWork v5](https://lore.kernel.org/patchwork/patch/160500) |
| 2009/07/10 | Balbir Singh <balbir@linux.vnet.ibm.com>| [Memory controller soft limit patches (v9)](https://lore.kernel.org/patchwork/patch/163652) | 实现内存资源控制器的软限制.<br>软限制是内存资源控制器的一个新特性, 类似的东西已经以共享的形式存在于组调度程序中. CPU控制器对共享的解释是非常不同的. 对于管理员希望过度使用系统的环境, 软限制是最有用的特性, 这样只有在内存争用时, 限制才会生效. 当前的软限制实现为内存控制器提供了 soft_limit_in_bytes 接口, 而不是为内存+交换控制器提供的接口. 该实现维护一个 RB-Tree, 其中的组超过了其软限制, 并开始从超出该限制的组中回收最大数量的组. | v9 ☑ 2.6.32-rc1 | [PatchWork RFC,0/5](https://lore.kernel.org/patchwork/patch/163652) |
| 2022/03/31 | zhaoyang.huang <zhaoyang.huang@unisoc.com> | [[RFC] cgroup: introduce dynamic protection for memcg](https://patchwork.kernel.org/project/linux-mm/patch/1648713656-24254-1-git-send-email-zhaoyang.huang@unisoc.com) | 动态限制的 MEMCG.<br>对于某些类型的 MEMCG, 使用情况因场景而异. 例如, 多媒体应用的使用范围可能在 50MB 到 500MB 之间, 这是通过在其虚拟地址空间中加载特殊算法生成的, 如果没有用户空间的交互, 很难保护扩展的使用. 此外, 固定的 mem.low 有点违背了它的软保护作用, 因为它会以同样的方式响应任何系统的内存压力. 在此基础上, 提出了一种于组水印和系统内存压力的动态保护方案. 目标是: 1. 动态保护, 无固定设置限值. 2. 正确的内存保护值. 3. 基于时间的衰变保护 4. 记忆压力相关保护.<br>引入 elow, emin<br>1. 在保护方面, elow 是根据水印的比例在适当的范围内.<br>2. 经过时间通过 decayed_watermark 对 elow 有积极的影响.<br>3. 内存压力对 low 有负面影响, 当系统压力较小时, low 可以保持更多的使用. | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/all/1648713656-24254-1-git-send-email-zhaoyang.huang@unisoc.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/all/1649040193-10073-1-git-send-email-zhaoyang.huang@unisoc.com) |



## 9.1 PageCacheLimit
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/01/15 | Roy Huang <royhuang9@gmail.com> | [Provide an interface to limit total page cache.](https://lore.kernel.org/patchwork/patch/72078) | 限制 page cache 的内存占用. | RFC ☐ | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/72078) |
| 2007/01/24 | Christoph Lameter <clameter@sgi.com> | [Limit the size of the pagecache](https://lore.kernel.org/patchwork/patch/72581) | 限制 page cache 的内存占用. | RFC ☐ | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/72581) |
| 2014/06/16 | Xishi Qiu <qiuxishi@huawei.com> | [mm: add page cache limit and reclaim feature](https://lore.kernel.org/patchwork/patch/473535) | openEuler 上限制 page cache 的内存占用. | v1 ☐ | [PatchWork](https://lore.kernel.org/patchwork/patch/416962)<br>*-*-*-*-*-*-*-* <br>[openEuler 4.19](https://gitee.com/openeuler/kernel/commit/6174ecb523613c8ed8dcdc889d46f4c02f65b9e4) |
| 2019/02/23 | Chunguang Xu <brookxu@tencent.com> | [pagecachelimit: limit the pagecache ratio of totalram](http://github.com/tencent/TencentOS-kernel/commit/6711b34671bc658c3a395d99aafedd04a4ebbd41) | TencentOS 限制 page cache 的内存占用. | NA |  [pagecachelimit: limit the pagecache ratio of totalram](http://github.com/tencent/TencentOS-kernel/commit/6711b34671bc658c3a395d99aafedd04a4ebbd41) |

## 9.2 Cgroup-Aware OOM killer
-------

[Taming the OOM killer](https://lwn.net/Articles/317814)

[Teaching the OOM killer about control groups](https://lwn.net/Articles/761118)


*   MEMCG Priority

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/03/17 | Roman Gushchin <guro@fb.com> | [add support for reclaiming priorities per mem cgroup](https://lore.kernel.org/all/20170317231636.142311-1-timmurray@google.com) | 设置 CGROUP 的内存优先级, Kswapd 等优先扫描低优先级 CGROUP 来回收内存. 用于优化 Android 上的内存回收和 OOM 等机制 | v13 ☐ | [PatchWork RFC v1](https://lore.kernel.org/all/20170317231636.142311-1-timmurray@google.com) |
| 2021/04/14 | Yulei Zhang <yuleixzhang@tencent.com> | [introduce new attribute "priority" to control group](https://lore.kernel.org/patchwork/patch/828043) | Cgroup 引入优先级. | v13 ☐ | [PatchWork v1](https://lwn.net/Articles/851649)<br>*-*-*-*-*-*-*-* <br>[LWN](https://lwn.net/Articles/852378) |

*   OOM Killer

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/09/04 | Tim Murray <timmurray@google.com> | [cgroup-aware OOM killer](https://lore.kernel.org/patchwork/patch/828043) | Cgroup 感知的 OOM, 通过优先级限定 OOM 时杀进程的次序 | v13 ☐ | [PatchWork v7](https://lore.kernel.org/patchwork/patch/828043) 带 oom_priority<br>*-*-*-*-*-*-*-* <br>[PatchWork v13](https://lore.kernel.org/patchwork/patch/828043) |
| 2018/03/16 | David Rientjes <rientjes@google.com> | [rewrite cgroup aware oom killer for general use](https://lore.kernel.org/patchwork/patch/828043) | Cgroup 感知的 OOM, 通过优先级限定 OOM 时杀进程的次序 | v13 ☐ | [PatchWork v1](https://lore.kernel.org/patchwork/patch/934536) |
| 2021/03/25 | Ybrookxu | [bfq: introduce bfq.ioprio for cgroup](https://lore.kernel.org/patchwork/patch/828043) | Cgroup 感知的 bfq.ioprio | v3 ☐ | [LKML](https://lkml.org/lkml/2021/3/25/93) |
| 2022/04/21 | Nico Pache <npache@redhat.com> | [Slight improvements for OOM/Futex](https://patchwork.kernel.org/project/linux-mm/cover/20220421190533.1601879-1-npache@redhat.com/) | 634339 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/20220421190533.1601879-1-npache@redhat.com) |


*   Killed Threads

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/03/25 | Tetsuo Handa <penguin-kernel@I-love.SAKURA.ne.jp> | [memcg: killed threads should not invoke memcg OOM killer](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7775face207922ea62a4e96b9cd45abfdc7b9840) | 如果当前线程在等待 oom_lock 时被杀死, 我们不需要调用 memcg OOM Killer | v3 ☐ | [PatchWork](https://patchwork.kernel.org/project/linux-mm/patch/01370f70-e1f6-ebe4-b95e-0df21a0bc15e@i-love.sakura.ne.jp), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7775face207922ea62a4e96b9cd45abfdc7b9840) |
| 2021/10/23 | Vasily Averin <vvs@virtuozzo.com> | [memcg: prohibit unconditional exceeding the limit of dying tasks](https://patchwork.kernel.org/project/linux-mm/patch/b89715b5-6df7-34a3-f7b9-efa8e0eefd3e@virtuozzo.com) | 内存 cgroup charge 的过程中允许杀死或退出的任务超过硬限制. 假设这些任务占用的内存数量是绑定的, 并且在任务退出时, 大部分内存将被释放. 这类似于任务访问内存储备时的全局 OOM 情况的启发式策略. 在 memcg 水平上没有全局内存短缺, 所以 memcg 启发式比较轻松.<br>不过, 上述假设过于乐观. 例如:<br>1. vmalloc 可以扩展到真正大的请求, 启发式将允许这一点. 我们曾经在 vmalloc 分配器中有一个被杀死的任务的早期中断, 但这已经被提交 b8c8a338f75e ("Revert vmalloc: back off when the current task is killed"). 所恢复. 可能还有其他类似的代码路径在分配和充电循环中不检查致命信号.<br>2. 还有一些内核对象统计到给 memcg, 但是它们没有绑定到进程生命周期. 据观察, 触发这些旁路并造成全球 OOM 情况并非真的很难.<br>解决这些问题的一种潜在方法是限制过剩的数量(类似于有限房间储备的全局 OOM). 这当然是可能的, 但我们并不清楚需要多少额外的内存, 并且仍然可以防止全局 oom, 因为这将不得不考虑整个 memcg 配置.<br>这个补丁通过完全删除启发式的策略来解决这个问题. 绕过只允许请求, 要么不能失败, 或失败是不可取的, 而超出应该仍然受到限制. 实现一个被杀死或即将死亡的任务, 如果它已经通过 OOM 杀手阶段, 就不能再被 charge. | v3 ☐ | [2021/10/20 PatchWork RFC,0/2](https://patchwork.kernel.org/project/linux-mm/cover/fe1d45a1-276d-b0f4-fb71-8f5c1a9e8872@virtuozzo.com/)<br>*-*-*-*-*-*-*-* <br>[2021/10/05 PatchWork v2,0/2](https://patchwork.kernel.org/project/linux-mm/cover/21d79b86-476c-a8f2-e950-2339606f1253@virtuozzo.com)<br>*-*-*-*-*-*-*-* <br>[2021/10/20 PatchWork v3,0/2](https://patchwork.kernel.org/project/linux-mm/cover/20e50917-3589-bcb7-0174-b6fccfd15c66@virtuozzo.com) |


## 9.3 KMEMCG
-------

[memcg kernel memory tracking](https://lwn.net/Articles/482777)

```cpp
git://git.kernel.org/pub/scm/linux/kernel/git/glommer/memcg.git kmemcg-stac
```

2012-04-20 [slab+slub accounting for memcg 00/23](https://lore.kernel.org/patchwork/patch/298625)
2012-05-11 [kmem limitation for memcg (v2,00/29)](https://lore.kernel.org/patchwork/patch/302291)
2012-05-25 [kmem limitation for memcg (v3,00/28)](https://lore.kernel.org/patchwork/patch/304994)
2012-06-18 [kmem limitation for memcg (v4,00/25)](https://lore.kernel.org/patchwork/patch/308973)
2012-06-25 [kmem controller for memcg: stripped down version](https://lore.kernel.org/patchwork/patch/310278)
2012-09-18 [ kmem controller for memcg (v3,00/13)](https://lore.kernel.org/patchwork/patch/326975)
2012-10-08 [kmem controller for memcg (v4,00/14)](https://lore.kernel.org/patchwork/patch/331578)
2012-10-16 [kmem controller for memcg (v5,00/14](https://lore.kernel.org/patchwork/patch/333535)

```cpp
git://github.com/glommer/linux.git kmemcg-slab
```

2021-07-25 [memcg kmem limitation - slab.](https://lore.kernel.org/patchwork/patch/315827)
2012-08-09 [Request for Inclusion: kmem controller for memcg (v2, 00/11)](https://lore.kernel.org/patchwork/patch/318976)
2012-09-18 [slab accounting for memcg (v3,00/16)](https://lore.kernel.org/patchwork/patch/326976)
2012-10-19 [slab accounting for memcg (v5,00/18)](https://lore.kernel.org/patchwork/patch/334479)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/03/09 |  Suleiman Souhlal <ssouhlal@FreeBSD.org> | [Memcg Kernel Memory Tracking](https://lwn.net/Articles/485593) | memcg 支持对内核 STACK 进行统计 | v2 ☐ | [PatchWork v2](https://lore.kernel.org/patchwork/patch/291219) |
| 2012/10/16 | Glauber Costa <glommer@parallels.com> | [kmemcg-stack](https://lwn.net/Articles/485593) | memcg 支持对内核 STACK 进行统计 | v5 ☐ | [PatchWork v5](https://lore.kernel.org/patchwork/patch/333535) |
| 2012/10/19 | Glauber Costa <glommer@parallels.com> | [kmemcg-stack](https://lwn.net/Articles/485593) | memcg 支持对内核 SLAB 进行统计 | v5 ☐ | [PatchWork v5](https://lore.kernel.org/patchwork/patch/334479) |
| 2015/11/10 | Vladimir Davydov <vdavydov@virtuozzo.com> | [memcg/kmem: switch to white list policy](https://lore.kernel.org/patchwork/patch/616606/) | NA | v2 ☑ 4.5-rc1 | [PatchWork v2,0/6](https://lore.kernel.org/patchwork/patch/616606) |
| 2020/06/23 | Glauber Costa <glommer@parallels.com> | [kmem controller for memcg](https://lwn.net/Articles/516529) | memcg 支持对内核内存(kmem)进行统计, memcg 开始支持统计两种类型的内核内存使用 : 内核栈 和 slab. 这些限制对于防止 fork 炸弹(bombs)等事件很有用. | v6 ☑ 3.8-rc1 | [PatchWork v5 kmem(stack) controller for memcg](https://lore.kernel.org/patchwork/patch/333535), [PatchWork v5 slab accounting for memcg](https://lore.kernel.org/patchwork/patch/334479)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6](https://lore.kernel.org/patchwork/patch/337780) |
| 2020/06/23 |  Roman Gushchin <guro@fb.com> | [The new cgroup slab memory controller](https://lore.kernel.org/patchwork/patch/1261793) | 将 SLAB 的统计跟踪统计从页面级别更改为到对象级别. 它允许在 memory cgroup 之间共享 SLAB 页. 这一变化消除了每个 memory cgroup 每个重复的每个 cpu 和每个节点 slab 缓存集, 并为所有内存控制组建立了一个共同的每个cpu和每个节点slab缓存集. 这将显著提高 SLAB 利用率(最高可达45%), 并相应降低总的内核内存占用. 测试发现不可移动页面数量的减少也对内存碎片产生积极的影响. | v7 ☑ 5.9-rc1 | [PatchWork v7](https://lore.kernel.org/patchwork/patch/1261793) |
| 2015/11/10 | Roman Gushchin <guro@fb.com> | [memcg/kmem: switch to white list policy](https://lore.kernel.org/patchwork/patch/616606) | 所有的 kmem 分配(即每次 kmem_cache_alloc、kmalloc、alloc_kmem_pages调用)都会自动计入内存 cgroup. 如果出于某些原因, 呼叫者必须明确选择退出. 这样的设计决策会导致以下许多个问题, 因此该补丁切换为白名单策略. 现在 kmalloc 用户必须通过传递 __GFP_ACCOUNT 标志来显式标记. | v7 ☑ 5.9-rc1 | [PatchWork v7](https://lore.kernel.org/patchwork/patch/616606) |
| 2021/05/06 |  Waiman Long <longman@redhat.com> | [The new cgroup slab memory controller](https://lore.kernel.org/patchwork/patch/1422112) | [The new cgroup slab memory controller](https://lore.kernel.org/patchwork/patch/1261793) 合入后, 不再需要为每个 MEMCG 使用单独的 kmemcache, 从而减少了总体的内核内存使用. 但是, 我们还为每次调用 kmem_cache_alloc() 和 kmem_cache_free() 增加了额外的内存开销. 参见 [10befea91b:  hackbench.throughput -62.4% regression](https://lore.kernel.org/lkml/20210114025151.GA22932@xsang-OptiPlex-9020) 和 [memcg: performance degradation since v5.9](https://lore.kernel.org/linux-mm/20210408193948.vfktg3azh2wrt56t@gabell/T/#u). 这组补丁通过降低了 kmemcache 统计的开销. 缓解了这个问题. | v7 ☑ 5.9-rc1 | [PatchWork v7](https://lore.kernel.org/patchwork/patch/1422112) |
| 2020/12/20 | Shakeel Butt <shakeelb@google.com> | [inotify, memcg: account inotify instances to kmemcg](https://lore.kernel.org/patchwork/patch/1355497) | 目前 sysctl inotify/max_user_instances 用于限制系统的 inotify 实例数量. 对于运行多个工作负载的系统, 每个用户的命名空间 sysctl max_inotify_instances 可以用于进一步划分 inotify 实例. 但是, 没有简单的方法可以对 inotify 实例设置合理的系统级别最大限制, 并在工作负载之间对其进行进一步分区. 该补丁通过将 inotify 实例分配给 memcg, 管理员可以简单地将 max_user_instances 设置为 INT_MAX, 并让作业的 memcg 限制它们的 inotify 实例. | v2 ☑ [5.12-rc1](https://kernelnewbies.org/Linux_5.12#Memory_management) | [PatchWork v7](https://lore.kernel.org/patchwork/patch/1355497), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ac7b79fd190b02e7151bc7d2b9da692f537657f3) |
| 2014/02/15 | Vladimir Davydov <vdavydov@parallels.com> | [kmemcg shrinkers](https://lore.kernel.org/patchwork/patch/438717) | NA | v15 ☐ | [PatchWork -mm,v15,00/13](https://lore.kernel.org/patchwork/patch/438717) |
| 2013/04/08 | Glauber Costa <glommer@parallels.com> | [memcg-aware slab shrinking with lasers and numbers](https://lore.kernel.org/all/1365429659-22108-1-git-send-email-glommer@parallels.com) | SHRINKER_MEMCG_AWARE 的一次尝试. | v3 ☐☑✓ | [LORE v3,0/32](https://lore.kernel.org/all/1365429659-22108-1-git-send-email-glommer@parallels.com) |
| 2015/01/08 | Vladimir Davydov <vdavydov@parallels.com> | [Per memcg slab shrinkers](https://lore.kernel.org/patchwork/patch/438717) | 实现 SHRINKER_MEMCG_AWARE, memcg 的 kmem 统计现在无法使用, 因为它缺少 slab 的 shrink 支持. 这意味着当达到极限时, 将获得 ENOMEM, 而没有任何恢复的机会. 然后我们应该做的是调用 shrink_slab(), 它将从这个 cgroup 中回收旧的 inode/dentry 缓存, 这组补丁就完成了这个功能. 基本上, 它做两件事.<br>1. 首先, 引入了 per memcg slab shrinkers. 按 cgroup 回收对象的 shrinker 应将自身标记为 MEMCG AWARE. 接着在 shrink_control->memcg 中递归地对 memcg 进行扫描. 迭代目标 cgroup 下的整个 cgroup 子树, 并为每个 kmem 活动 memcg 调用 shrinker.<br>2. 其次, 该补丁集为每个 memcg 生成  list_lru. 这对 list_lru 是透明的, 他们所要做的一切就是告诉 list_lru_init() 他们想要有 memcg aware 的 list_lru. 然后, list_lru 将根据对象所属的 cgroup 自动在每个 memcg list 中分配对象. 为了让 FS 收缩器(icache、dcache)能够识别 memcg, 我们只需要让它们使用 memcg 识别 list_lru.<br>与以前一样, 此修补程序集仅在压力来自 memory.limit 而不是 memory.kmem.limit 时启用 per-memcg kmem reclaim. 由于 GFP_NOFS 分配, 处理 memory.kmem.limit 是一个棘手的问题, 目前还不清楚我们是否会在统一的层次结构中使用这个旋钮. | v3 ☑ 4.0-rc1 | [PatchWork -mm,v3,0/9](https://lore.kernel.org/all/cover.1420711973.git.vdavydov@parallels.com), [关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=cb731d6c62bbc2f890b08ea3d0386d5dad887326) |
| 2022/02/11 | Shakeel Butt <shakeelb@google.com> | [memcg: robust enforcement of memory.high](https://patchwork.kernel.org/project/linux-mm/cover/20220210081437.1884008-1-shakeelb@google.com) | 612932 | v1 ☐ | [PatchWork v1,0/4](https://lore.kernel.org/all/20220210081437.1884008-1-shakeelb@google.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/4](https://lore.kernel.org/r/20220211064917.2028469-1-shakeelb@google.com) |


弃用 kmem.limit_in_bytes

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/10/19 | Shakeel Butt <shakeelb@google.com> | [memcg, kmem: further deprecate kmem.limit_in_bytes](https://patchwork.kernel.org/project/linux-mm/patch/20211019153408.2916808-1-shakeelb@google.com) | kmem.limit_in_bytes 的弃用过程始于提交 commit 0158115f702("memcg, kmem:deprecate kmem.limit_in_bytes"), 这也详细解释了弃用背后的动机. 总而言之, 这是达到 kmem 极限时的意外行为. 此修补程序通过不允许设置 kmem 限制, 将弃用过程移动到下一阶段. 将来我们可能会完全删除 kmem.limit_in_bytes 文件. | v3 ☐ | [PatchWork v3](https://patchwork.kernel.org/project/linux-mm/patch/20211019153408.2916808-1-shakeelb@google.com) |

其他

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/02/01 | Yosry Ahmed <yosryahmed@google.com> | [memcg: add per-memcg total kernel memory stat](https://patchwork.kernel.org/project/linux-mm/patch/20220201200823.3283171-1-yosryahmed@google.com/) | 610482 | v1 ☐☑ | [PatchWork v1,0/1](https://lore.kernel.org/r/20220201200823.3283171-1-yosryahmed@google.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/1](https://lore.kernel.org/r/20220203193856.972500-1-yosryahmed@google.com) |
| 2022/05/21 | Vasily Averin <vvs@openvz.org> | [memcg: accounting for objects allocated by mkdir cgroup](https://lore.kernel.org/all/06505918-3b8a-0ad5-5951-89ecb510138e@openvz.org) | TODO | v2 ☐☑✓ | [LORE v2,0/9](https://lore.kernel.org/all/06505918-3b8a-0ad5-5951-89ecb510138e@openvz.org) |
| 2022/06/28 | Yosry Ahmed <yosryahmed@google.com> | [KVM: mm: count KVM mmu usage in memory stats](https://patchwork.kernel.org/project/linux-mm/cover/20220628220938.3657876-1-yosryahmed@google.com/) | 654790 | v6 ☐☑ | [LORE v6,0/4](https://lore.kernel.org/r/20220628220938.3657876-1-yosryahmed@google.com) |


## 9.4 MEMCG LRU
-------

### 9.4.1 双 LRU 方案(per-memcg 的 per-zone LRU + 全局 LRU)
-------

自 2008 年 v2.6.25-rc1, Balbir Singh 实现 MEMCG 时, 就支持了 per-memcg 的 LRU,  [Memory controller: add per cgroup LRU and reclaim](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=66e1707bc34609f626e2e7b4fe7e454c9748bad5). 不过当时每个 MEMECG 只有一个唯一的 LRU, 且区分 active_list 和 inactive_list(Per cgroup active and inactive list). 引入了 [page_cgroup](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8cdea7c05454260c0d4d83503949c358eb131d17) 维护了 mem_cgroup 和 lru 的联系,. 并标记了 [TODO: Consider making these lists per zone](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=78fb74669e80883323391090e4d26d17fe29488f). 此时, per-memcg 的 lru_lock 也是 mem_cgroup 级别的.

而当时正是 per-Zone LRU 的时代, 随即 KAMEZAWA Hiroyuki 就为 per-memcg 的唯一 LRU 也完成了 per-Zone 的支持, 参见 [per-zone and reclaim enhancements for memory controller take 3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=072c56c13e1302fcdc39961dc64e76485731ad6). 为了完成这个功能, 引入了 mem_cgroup_per_zone 等结构, 通过 `__mem_cgroup_add_list()` 和 `__mem_cgroup_remove_list()` 将每个 page_cgroup.lru 节点添加或者移除其对应 [mem_cgroup_per_zone 的 active_list 和 inactive_list](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1ecaab2bd221251a3fd148abb08e8b877f1e93c8), 同样完成了 [mem_cgroup_per_zone 的 lru_lock](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=072c56c13e1302fcdc39961dc64e76485731ad67).

这个阶段, root memcg 的 LRU 其实并没有用处, 因此可以不用把页面添加进去, [memcg: remove the overhead associated with the root cgroup](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4b3bde4c983de36c59e6c1a24701f6fe816f9f55).

至此内核为 per-memcg 实现了 per-Zone 的 LRU. 不过这时候的 LRU 是 双 LRU 方案(double-LRU scheme). 即除了 per-memcg 的 LRU 以外, 系统中还有全局的 LRU, 而且他们都是 Per-Zone 的.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/08/24 | Balbir Singh <balbir@linux.vnet.ibm.com> | [Memory controller: add per cgroup LRU and reclaim](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=66e1707bc34609f626e2e7b4fe7e454c9748bad5) | 实现 [Memory controller containers setup (v7)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e1a1cd590e3fcb0d2e230128daf2337ea55387dc) 时引入了 [per cgroup 的 LRU 和 页面 reclaim](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=66e1707bc34609f626e2e7b4fe7e454c9748bad5) 和  | v7 ☑ [2.6.25-rc1](https://kernelnewbies.org/Linux_2_6_25) | [PatchWork v7](https://lore.kernel.org/lkml/20070824152009.16582.78904.sendpatchset@balbir-laptop), [关注 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=66e1707bc34609f626e2e7b4fe7e454c9748bad5). |
| 2007/11/26 | KAMEZAWA Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com> | [per-zone and reclaim enhancements for memory controller take 3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=072c56c13e1302fcdc39961dc64e76485731ad6) | per-zone LRU for memcg, per-zone 的页面回收感知 MEMCG. 其中引入了 mem_cgroup_per_zone, mem_cgroup_per_node 等结构. | v3 ☑ 2.6.25-rc1 | [LORE v3,00/10](https://lore.kernel.org/lkml/20071127115525.e9779108.kamezawa.hiroyu@jp.fujitsu.com) |


### 9.4.2 per-memecg 的 LRU lock 回退到全局的 zone lru_lock
-------

v2.6.29-rc1 [memcg: synchronized LRU](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=08e552c69c6930d64722de3ec18c51844d06ee28) 发现了当前 memcg per-Zone LRU 的诸多问题. 当前通过 page_cgroup.lru 将 mem_cgroup 链接到自己 per-Zone LRU 的 active_list 和 inactive_list 上, 但是

1.  page_cgroup 的 LRU 与 global LRU 不同步.

2.  page 和 page_cgroup 一对一, 静态分配.

3.  要找到 page_cgroup(pc) 是什么 LRU, 你必须检查 pc->mem_cgroup as - lru = page_cgroup_zoneinfo(pc, nid_of_pc, zid_of_pc);

4.  复杂的 mem_cgroup_per_zone lru_lock 和 zone lru_lock 的嵌套处理, 非常容易导致死锁.

因此为了简化处理

1.  接口层次上实现了 mem_cgroup_add_lru_list(), mem_cgroup_del_lru_list(), mem_cgroup_rotate_lru_list(), mem_cgroup_del_lru(), mem_cgroup_move_lists() 来辅助 mem_cgroup_move_lists() 完成工作.

2.  移除了 mem_cgroup_per_zone 的 lru(即 MEMCG 的 Per-Zone LRU) lock, 全部采用 zone->lru_lock 来处理. 这样 MEMCG 的 Per-Zone LRU lock 就又变成了全局的 zone->lru_lock. 简而言之, 就是这个补丁合入后, 内核没有了 per memcg lru lock, 依旧使用旧的全局 lru lock 来管理全部 memcg lru lists. 这就造成了本来可以自治的 MEMCG, 却要等待其他 MEMCG 释放使用的 lru lock, 在一些场景往往会造成严重的性能问题. 由此而引发了 memcg lru lock 漫长的合入, 参见 [memcg lru lock 血泪史](https://blog.csdn.net/bjchenxu/article/details/112504932).


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/11/21 | KAMEZAWA Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com> | [memcg updates (14/Nov/2008)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=08e552c69c6930d64722de3ec18c51844d06ee28) | 关键 [commit 08e552c69c69 ("memcg: synchronized LRU")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=08e552c69c6930d64722de3ec18c51844d06ee28) 移除了 mem_cgroup_per_zone 的 lru(即 MEMCG 的 Per-Zone LRU) lock, 再次回退到 MEMCG 也使用 zone->lru_lock 来处理的时代. | v1 ☐☑✓ | [LORE v1,0/9](https://lkml.kernel.org/lkml/20081114191246.4f69ff31.kamezawa.hiroyu@jp.fujitsu.com) |


### 9.4.3 归一化的 per-memcg LRU
-------

双 LRU 方案(double-LRU scheme) 方案管理混乱, 运行复杂, 因此 v3.3 的时候, Johannes Weiner 终于看不下去了, 决定对两个 LRU 统一管理, 提出 [memcg naturalization -rc5,00/10](https://lore.kernel.org/lkml/1320787408-22866-1-git-send-email-jweiner@redhat.com). 它使传统的页面回收能够从每个 memcg LRU 列表中查找页面, 从而摆脱了双 LRU 方案和系统中每个页面所需的额外列表头. 参见 [Integrating memory control groups](https://lwn.net/Articles/443241). 归一后, 全局 LRU 不再存在, 所有的页面仅存在于每个组的 LRU 列表中. 未记入特定控制组的页面进入层次结构顶部 root 分组的 LRU 列表. 从本质上讲, 每组的页面回收接管旧的全局回收代码, 即使是禁用控制组的系统也被视为只有一个控制组包含所有正在运行的进程的系统.

实现上:

1.  MEMCG 页面回收上, 引入了 [struct mem_cgroup_zone](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f16015fbf2f6ac45505d6ad21455ff9f6c14473d), 它包含内存 cgroup 和被扫描区域的组合, 作为 [shrink 回收页面时的参数传递](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=5660048ccac8735d9bc0a46325a02e6a6518b5b2). 使用 mem_cgroup_reclaim() 和 mem_cgroup_soft_reclaim() 替代了之前 mem_cgroup_hierarchical_reclaim() 来接管 MEMCG 的页面回收.

2.  MEMCG LRU 管理上, 引入了 [lruvec 封装了 LRU list](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6290df545814990ca2663baf6e894669132d5f73), 所有的 LRU 都[被转换为在 per memcg 的 LRU](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b95a2f2d486d0d768a92879c023a03757b9c7e58), 因此[没有理由再保留双 LRU 方案](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=925b7673cce39116ce61e7a06683a4a0dad1e72a). 并且清理了 memcg LRU 操作的接口, 提供了 mem_cgroup_zone_lruvec(), mem_cgroup_lru_add_list(), mem_cgroup_lru_del_list(), mem_cgroup_lru_del(), mem_cgroup_lru_move_lists() 来操作 memcg LRU.

核心算法上:

1.  现在在控件组层次结构中执行深度优先遍历, 试图从每个层次结构中回收一些页. 不存在页面的全局老化问题.

2.  不管其他组中发生了什么, 每个组都有其最老的页面被考虑回收. 当然, 在设定回收目标时, 每个分组的硬性和软性限制都会被考虑.

最终的结果是, 全局回收自然地将痛苦传播到所有控制组, 并在过程中实现每个组的策略. 控制组软限制的实施与该机制相结合, 使软限制的实施更加公平地分布在系统中的所有控制组中.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2011/12/08 | Johannes Weiner <jweiner@redhat.com> | [memcg naturalization -rc5](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=6b208e3f6e35aa76d254c395bdcd984b17c6b626) | 参见 [LWN: Integrating memory control groups](https://lwn.net/Articles/443241). 引入 per-memcg lru, 消除重复的 LRU 列表, [全局 LRU 不再存在, page 只存在于 per-memcg LRU list 中](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=925b7673cce39116ce61e7a06683a4a0dad1e72a).<br>它使传统的页面回收能够从每个memcg LRU列表中查找页面, 从而消除了双LRU模式(除了每个memcg区域外, 每个全局区域)和系统中每个页面所需的额外列表头. <br>该补丁引入了 lruvec 结构. | v5 ☑ [3.3-rc1](https://kernelnewbies.org/Linux_3.3#Memory_management) | [LORE v5,00/10](https://lore.kernel.org/lkml/1320787408-22866-1-git-send-email-jweiner@redhat.com) |


### 9.4.4 per-memcg LRU lock 的血泪史
-------

zone->lru_锁是一个竞争激烈的锁, 因此 2012 年左右 Konstantin Khlebnikov 希望在多核系统上, 通过[在多个 MEMCG 之间拆分它来减少冲突](https://lore.kernel.org/lkml/20120223133728.12988.5432.stgit@zurg).

这引起了 Google 开发人员 Hugh Dickins 的关注, 很快他就发布了 [mm/memcg: per-memcg per-zone lru locking v1, 00/10](https://lore.kernel.org/lkml/alpine.LSU.2.00.1202201518560.23274@eggly.anvils), 通过重新引入 per memcg 的 LRU lock 来解决问题. 但是当时引起的关注比较少, 也缺乏 benchmark 来展示补丁的效果, 所以很快被社区遗忘了. 不过 Google 内部则一直在维护这组补丁, 随他们内核版本升级.

随后 2020 年, 阿里的大佬 Alex Shi 重新设计了 [per memcg lru lock v21,00/19](https://lore.kernel.org/all/1604566549-62481-1-git-send-email-alex.shi@linux.alibaba.com), 并于 [v5.11](https://kernelnewbies.org/Linux_5.11#Memory_management) 版本合入主线. 参见 [memcg lru lock 血泪史](https://blog.csdn.net/bjchenxu/article/details/112504932).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/02/16 | Konstantin Khlebnikov <khlebnikov@openvz.org> | [mm: lru_lock splitting](https://lore.kernel.org/all/20120215224221.22050.80605.stgit@zurg) | 20120215224221.22050.80605.stgit@zurg | v1 ☐☑✓ | [LORE v1,0/15](https://lore.kernel.org/all/20120215224221.22050.80605.stgit@zurg)<br>*-*-*-*-*-*-*-* <br>[LORE v2,00/22](https://lore.kernel.org/lkml/20120220171138.22196.65847.stgit@zurg)<br>*-*-*-*-*-*-*-* <br>[LORE v3,00/21](https://lore.kernel.org/lkml/20120223133728.12988.5432.stgit@zurg) |
| 2012/02/20 | Hugh Dickins <hughd@google.com> | [mm/memcg: per-memcg per-zone lru locking](https://lore.kernel.org/patchwork/patch/288055) | per-memcg lru lock | v1 ☐ 3.4 | [PatchWork v1, 00/10](https://lore.kernel.org/lkml/alpine.LSU.2.00.1202201518560.23274@eggly.anvils) |
| 2018/01/31 | Daniel Jordan <daniel.m.jordan@oracle.com> | [lru_lock scalability](https://lore.kernel.org/all/20180131230413.27653-1-daniel.m.jordan@oracle.com) | lru_lock 是内核中争抢非常严重的一把锁, 它保护 per-node lru list. 提高 lru_lock 可伸缩性的一种方法就是使用细粒度锁, 引入一系列锁, 每个锁保护特定批次的 lru 页面. | v1 ☐☑✓ | [LORE v1,0/13](https://lore.kernel.org/all/20180131230413.27653-1-daniel.m.jordan@oracle.com) |
| 2020/12/05 | Alex Shi <alex.shi@linux.alibaba.com> | [per memcg lru lock](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=15b447361794271f4d03c04d82276a841fe06328) | per memcg LRU lock | v21 ☑ [5.11](https://kernelnewbies.org/Linux_5.11#Memory_management) | [LORE v4,0/9](https://lore.kernel.org/lkml/1574166203-151975-1-git-send-email-alex.shi@linux.alibaba.com)<br>*-*-*-*-*-*-*-* <br>[LORE v21,00/19](https://lore.kernel.org/all/1604566549-62481-1-git-send-email-alex.shi@linux.alibaba.com) |


### 9.4.5 MEMCG Reclaim
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2011/05/26 | KAMEZAWA Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com> | [memcg async reclaim](https://lore.kernel.org/patchwork/patch/251835) | 实现 MEMCG 的异步回收机制(Asynchronous memory reclaim)<br>1. 当使用 memc g时, 应用程序可以看到由 memcg 限制引起的内存回收延迟. 一般来说, 这是不可避免的. 有一类应用程序, 它使用许多干净的文件缓存并执行一些交互式工作.<br>2. 如果内核能够帮助后台回收内存, 那么应用程序的延迟就会在一定程度上被隐藏(这取决于应用程序的睡眠方式). 这组补丁程序添加了控制开关 memory.async_control 启用异步回收. 采用动态计算的方法计算了边缘的大小. 该值被确定为减少应用程序达到限制的机会.<br>使用了新引入的 WQ_IDLEPRI 类型(使用 SCHED_IDLE 调度策略)的 kworker(memcg_async_shrinker) 来完整回收的操作. 通过使用 SCHED_IDLE, 系统繁忙的时候异步内存回收只能消耗 0.3% 的 CPU, 但如果cpu空闲, 可以使用很多 CPU. | v3 ☐ | [LORE FRC,0/7](https://lkml.org/lkml/2011/5/10/204)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC,v3,0/10](https://lore.kernel.org/lkml/20110526141047.dc828124.kamezawa.hiroyu@jp.fujitsu.com) |
| 2017/05/30 | Johannes Weiner <hannes@cmpxchg.org> | [mm: per-lruvec slab stats](https://lore.kernel.org/patchwork/patch/793422) | Josef 正在研究一种平衡 slab 缓存和 page cache 的新方法. 为此, 他需要 lruvec 级别的 slab 缓存统计信息. 这些补丁通过添加基础设施来实现这一点, 该基础设施允许每个 lruvec 更新和读取通用 VM 统计项, 然后将一些现有VM记帐站点(包括slab记帐站点)切换到这个新的 cgroup 感知 API. | v1 ☑ 4.13-rc1 | [PatchWork 0/6](https://lore.kernel.org/patchwork/patch/793422) |
| 2021/02/17 | Yang Shi <shy828301@gmail.com> | [Make shrinker's nr_deferred memcg aware](https://lore.kernel.org/patchwork/patch/1393744) | 最近, 在一些 vfs 元数据繁重的工作负载上看到了大量的 one-off slab drop, 造成收缩器了大量累积的 nr_deferred 对象.<br>这个是由多种原因导致的.<br>这组补丁集使 nr_deferred 变成 per-memcg 来解决问题. | v10 ☑ 5.13-rc1 | [PatchWork v10,00/13](https://patchwork.kernel.org/project/linux-mm/cover/20210217001322.2226796-1-shy828301@gmail.com) |
| 2021/10/19 | Huangzhaoyang <huangzhaoyang@gmail.com> | [mm: skip current when memcg reclaim](https://patchwork.kernel.org/project/linux-mm/patch/1634628576-27448-1-git-send-email-huangzhaoyang@gmail.com) | NA | v2 ☐ | [PatchWork v1](https://patchwork.kernel.org/project/linux-mm/patch/1634278529-16983-1-git-send-email-huangzhaoyang@gmail.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://patchwork.kernel.org/project/linux-mm/patch/1634628576-27448-1-git-send-email-huangzhaoyang@gmail.com) |
| 2022/04/07 | Yosry Ahmed <yosryahmed@google.com> | [memcg: introduce per-memcg proactive reclaim](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=eae3cb2e87ff84547e66211b81301a8f9122840f) | MEMCG 中引入 memory.reclaim 使得用户空间可以通过持续探测 MEMCG 并触发主动回收以回收少量内存. 随着 LRU 不断排序, 这将提供更准确和最新的工作集估计, 并可能提供更具确定性的内存过度使用行为. 内存超分配控制器可以对正在运行的应用程序不断变化的行为提供更主动的响应, 而不是被动响应. 在这种情况下, 用户空间回收器的目的不是完全替代 KSWAPD 或直接回收, 而是主动识别内存节约机会, 回收策略设置的一些冷页, 以释放内存, 用于要求更高的作业或安排新作业.<br>谷歌数据中心使用了此用户空间主动回收器. | v2 ☑ 5.19-rc1 | [LORE v1,0/1](https://lore.kernel.org/all/20220331084151.2600229-1-yosryahmed@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/4](https://lore.kernel.org/r/20220407224244.1374102-1-yosryahmed@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/4](https://lore.kernel.org/r/20220408045743.1432968-1-yosryahmed@google.com))<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/4](https://lore.kernel.org/r/20220421234426.3494842-1-yosryahmed@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/4](https://lore.kernel.org/r/20220425190040.2475377-1-yosryahmed@google.com) |




## 9.5 MEMCG DRITY Page
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2011/04/17 | Greg Thelen <gthelen@google.com> | [memcg: per cgroup dirty page limiting](https://lore.kernel.org/patchwork/patch/263368) | per-memcg 的脏页带宽限制 | v9 ☐ | [PatchWork v9](https://lore.kernel.org/patchwork/patch/263368) |
| 2015/12/30 | Tejun Heo <tj@kernel.org> | [memcg: add per cgroup dirty page accounting](https://lore.kernel.org/patchwork/patch/558382) | madvise 支持页面延迟回收(MADV_FREE)的早期尝试  | v5 ☑ [4.2-rc1](https://kernelnewbies.org/Linux_4.2#Memory_management) | [PatchWork v1](https://lore.kernel.org/patchwork/patch/558382), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c4843a7593a9df3ff5b1806084cefdfa81dd7c79) |


## 9.6 MEMCG Swapin
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/07/04 | KAMEZAWA Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com> | [memcg: shmem swap cache](https://lore.kernel.org/patchwork/patch/121475) | memcg swapin 的延迟统计, 对memcg进行了修改, 使其在交换时直接对交换页统计, 而不是在出错时统计, 这可能要晚得多, 或者根本不会发生. | v1 ☑ 2.6.26-rc8-mm1 | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/121270)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/121475)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/https://lore.kernel.org/patchwork/patch/121475), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d13d144309d2e5a3e6ad978b16c1d0226ddc9231) |
| 2008/07/14 | KAMEZAWA Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com> | [memcg: handle tmpfs' swapcache](https://lore.kernel.org/patchwork/patch/122556) | memcg swapin 的延迟统计, 对memcg进行了修改, 使其在交换时直接对交换页统计, 而不是在出错时统计, 这可能要晚得多, 或者根本不会发生. | v1 ☐ | [PatchWork v2](https://lore.kernel.org/patchwork/patch/122556)) |
| 2008/11/14 | KAMEZAWA Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com> | [memcg : add swap controller](https://lore.kernel.org/patchwork/patch/134928) | 实现 CONFIG_MEMCG_SWAP | v1 ☑ 2.6.29-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/134928)) |
| 2008/12/01 | KOSAKI Motohiro <kosaki.motohiro@jp.fujitsu.com> | [memcg: split-lru feature for memcg take2](https://lkml.org/lkml/2008/12/1/99) | NA | v2 ☑ [2.6.29-rc1](https://kernelnewbies.org/Linux_2_6_29#Memory_controller_swap_management_and_other_improvements) | [LKML](https://lkml.org/lkml/2008/12/1/99), [PatchWork](https://lore.kernel.org/patchwork/patch/136809) |
| 2008/12/02 | KOSAKI Motohiro <kosaki.motohiro@jp.fujitsu.com> | [memcg: swappiness](https://lkml.org/lkml/2008/12/2/21) | 引入 per-memcg 的 swappiness, 可以用来对 per-memcg 进行精确控制. | v2 ☑ [2.6.29-rc1](https://kernelnewbies.org/Linux_2_6_29#Memory_controller_swap_management_and_other_improvements) | [LKML](https://lkml.org/lkml/2008/12/2/21), [PatchWork](https://lore.kernel.org/patchwork/patch/136809) |
| 2009/06/02 | KAMEZAWA Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com> | [memcg fix swap accounting](https://lore.kernel.org/patchwork/patch/158520) | NA | v1 ☑ 2.6.29-rc1 | [PatchWork RFC](https://lore.kernel.org/patchwork/patch/157997)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/patch/158520)) |
| 2009/06/02 | KAMEZAWA Hiroyuki <kamezawa.hiroyu@jp.fujitsu.com> | [mm rss counting updates](https://lore.kernel.org/patchwork/patch/182191) | NA | v1 ☑ 2.6.29-rc1 | [PatchWork v1](https://lore.kernel.org/patchwork/patch/182191)) |
| 2015/12/17 | Vladimir Davydov <vdavydov@virtuozzo.com> | [Add swap accounting to cgroup2](https://lore.kernel.org/patchwork/patch/628754) | NA | v2 ☑ 2.6.29-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/628754)) |
| 2020/08/20 | Johannes Weiner <hannes@cmpxchg.org> | [mm: memcontrol: charge swapin pages on instantiation](https://lore.kernel.org/patchwork/patch/1239175) | memcg swapin 的延迟统计, 对memcg进行了修改, 使其在交换时直接对交换页统计, 而不是在出错时统计, 这可能要晚得多, 或者根本不会发生. | v2 ☑ [5.8-rc1](https://kernelnewbies.org/Linux_5.8#Memory_management) | [PatchWork v1](https://lore.kernel.org/patchwork/patch/1227833), [PatchWork v2](https://lore.kernel.org/patchwork/patch/1239175) |


## 9.7 MEMCG THP
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/05/05 | CGEL <cgel.zte@gmail.com> | [mm/memcg: support control THP behaviour in cgroup](https://patchwork.kernel.org/project/linux-mm/patch/20220505033814.103256-1-xu.xin16@zte.com.cn) | 使用 THP 可以提高内存的性能, 但会增加内存占用. 应用程序可能会使用 madvise 来减少占用空间, 但并非所有应用程序都支持使用 madvise, 而且重新编码所有应用程序需要花费大量成本. 但是当前容器越来越流行于管理一组任务. 因此, 添加对 cgroup 的支持以控制 THP 行为将提供很大的便利, 管理员可能只对重要容器启用 THP, 而对其他容器禁用它. 这样我们就可以享受 THP 的高性能, 同时最大限度地减少内存占用, 而无需重新编写任何应用程序. | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220505033814.103256-1-xu.xin16@zte.com.cn)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20220506031804.437642-1-yang.yang29@zte.com.cn) |
| 2022/06/22 | Baolin Wang <baolin.wang@linux.alibaba.com> | [Add PUD and kernel PTE level pagetable account](https://patchwork.kernel.org/project/linux-mm/cover/cover.1655887440.git.baolin.wang@linux.alibaba.com/) | 652684 | v2 ☐☑ | [LORE v2,0/3](https://lore.kernel.org/r/cover.1655887440.git.baolin.wang@linux.alibaba.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/3](https://lore.kernel.org/r/cover.1656586863.git.baolin.wang@linux.alibaba.com) |

## 9.8 Memory vmpressure
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2022/06/23 | Yosry Ahmed <yosryahmed@google.com> | [mm: vmpressure: don't count userspace-induced reclaim as memory pressure](https://patchwork.kernel.org/project/linux-mm/patch/20220623000530.1194226-1-yosryahmed@google.com/) | 652966 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220623000530.1194226-1-yosryahmed@google.com) |


# 10 内存热插拔支持
-------




内存热插拔, 也许对于第一次听说的人来说, 觉得不可思议: 一个系统的核心组件, 为何要支持热插拔? 用处有以下几点.



> 1\. 大规模集群中, 动态的物理容量增减, 可以实现更好地支持资源集约和均衡.
> 2\. 大规模集群中, 物理内存出错的机会大大增多, 内存热插拔技术对提高高可用性至关重要.
> 3\. 在虚拟化环境中, 客户机(Guest OS)间的高效内存使用也对热插拔技术提出要求



当然, 作为一个核心组件, 内存的热插拔对从系统固件,到软件(操作系统)的要求, 跟普通的外设热插拔的要求, 不可同日而语. 这也是为什么 Linux 内核对内存热插拔的完全支持一直到近两年才基本完成.



总的来说, 内存热插拔分为两个阶段, 即**物理热插拔阶段**和**逻辑热插拔阶段:**

> **物理热插拔阶段:** 这一阶段是内存条插入/拔出主板的过程. 这一过程必须要涉及到固件的支持(如 ACPI 的支持), 以及内核的相关支持, 如为新插入的内存分配管理元数据进行管理. 我们可以把这一阶段分别称为 hot-add / hot-remove.
> **逻辑热插拔阶段:** 这一阶段是从使用者视角, 启用/关闭这部分内存. 这部分的主要从内存分配器方面做的一些准备工作. 我们可以把这一阶段分别称为 online / offline.



逻辑上来讲, 内存插和拔是一个互为逆操作, 内核应该做的事是对称的, 但是, 拔的过程需要关注的技术难点却比插的过程多, 因为, 从无到有容易, 从有到无麻烦:在使用的内存页应该被妥善安置, 就如同安置拆迁户一样, 这是一件棘手的事情. 所以内核对 hot-remove 的完全支持一直推迟到 2013 年.



## 10.1 内存热插入支持
-------

**2.6.15(2006年1月发布)**


这提供了最基本的内存热插入支持(包括物理/逻辑阶段). 注意, **此时热拔除还不支持.**



## 10.2 初步的内存逻辑热拔除支持
-------

**2.6.24(2008年1月发布)**




此版本提供了**部分的逻辑热拔除阶段的支持, 即 offline.** Offline 时, 内核会把相关的部分内存隔离开来, 使得该部分内存不可被其他任何人使用, 然后再把此部分内存页, 用前面章节说过的内存页迁移功能转移到别的内存上. 之所以说**部分支持**, 是因为该工作只提供了一个 offline 的功能. 但是, 不是所有的内存页都可以迁移的. 考虑"迁移"二字的含义, 这意味着**物理内存地址**会变化, 而内存热拔除应该是对使用者透明的, 这意味着, 用户见到的**虚拟内存地址**不能变化, 所以这中间必须存在一种机制, 可以修改这种因迁移而引起的映射变化. 所有通过页表访问的内存页就没问题了, 只要修改页表映射即可; 但是, 内核自己用的内存, 由于是绕过寻常的逐级页表机制, 采用直接映射(提高了效率), 内核内存页的虚拟地址会随着物理内存地址变动, 因此, 这部分内存页是无法轻易迁移的. 所以说, 此版本的逻辑热拔除功能只是部分完成.



注意, 此版本中, **物理热拔除是还完全未实现. 一句话, 此版本的热拔除功能还不能用.**





## 10.3 完善的内存逻辑热拔除支持
-------

**3.8(2013年2月发布)**




针对9.2中的问题, 此版本引入了一个解决方案. 9.2 中的核心问题在于**不可迁移的页会导致内存无法被拔除.** 解决问题的思路是使可能被热拔除的内存不包含这种不可迁移的页. 这种信息应该在内存初始化/内存插入时被传达, 所以, 此版本中, 引入一个 movable\_node 的概念. 在此概念中, 一个被 movable\_node 节点的所有内存, 在初始化/插入后, 内核确保它们之上不会被分配有不可迁移的页, 所以当热拔除需求到来时, 上面的内存页都可以被迁移, 从而使内存可以被拔除.





## 10.4 物理热拔除的支持
-------


**3.9(2013年4月支持)**

此版本支持了**物理热拔除,** 这包括对内存管理元数据的删除, 跟固件(如ACPI) 相关功能的实现等比较底层琐碎的细节, 不详谈.



在完成这一步之后, 内核已经可以提供基本的内存热插拔支持. 值得一提的是, 内存热插拔的工作, Fujitsu 中国这边的内核开放者贡献了很多 patch. 谢谢他们!


## 10.5  improving in-kernel auto-online support
-------

*   虚拟机内存热缩优化


新上线的页面通常会被放置在 freelist 的前面, 这意味着它们很快会在下一次内存分配时就被使用, 这将造成添加的内存通常立即无法删除.

使用 virtio-mem 和 dimm 可以经常观察到这个现象: 当通过 `add_memory*()` 添加单独的内存块并立即联机使用它们时, 内核通常会把下一个块的元数据(特别是 memmap 的数据)被放置到刚刚添加并上线的内存块上. 这是一个不可移动的内存分配链, 如果最后一个内存块不能脱机并 remove(), 那么所有依赖的内存块也会脱机.

其实新页面通常是冷的, 尝试分配其他页面(可能是热的)本身是一件非常自然的事情. 因此一个非常简单有效的尝试是: 新上线的内存其空闲页面不要放置到 freelist 的前面, 尝试放置到 freelist 的最后即可.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/09/16 | David Hildenbrand <david@redhat.com> | [mm: place pages to the freelist tail when onling and undoing isolation](https://patchwork.kernel.org/project/linux-mm/cover/20200916183411.64756-1-david@redhat.com) | 新上线的内存其空闲页面或者隔离页放置到 freelist 的最后 | RFC,v1 ☑ 5.10-rc1 | [PatchWork RFC,0/4](https://patchwork.kernel.org/project/linux-mm/cover/20200916183411.64756-1-david@redhat.com) |

*   "auto-movable" online policy


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/06/02 | David Hildenbrand <david@redhat.com> | [virtio-mem: prioritize unplug from ZONE_MOVABLE](https://patchwork.kernel.org/project/linux-mm/cover/20210602185720.31821-1-david@redhat.com) | NA | v1  ☑ 5.14-rc1 | [PatchWork v1,0/7](https://patchwork.kernel.org/project/linux-mm/cover/20210602185720.31821-1-david@redhat.com) |
| 2021/08/06 | David Hildenbrand <david@redhat.com> | [mm/memory_hotplug: "auto-movable" online policy and memory groups](https://patchwork.kernel.org/project/linux-mm/cover/20210806124715.17090-1-david@redhat.com) | NA | v3 ☑ 5.15-rc1 | [PatchWork v3,0/9](https://patchwork.kernel.org/project/linux-mm/cover/20210806124715.17090-1-david@redhat.com) |

# 11 超然内存(Transcendent Memory)支持
-------


## 11.1 tmem 简介
-------

[超然内存(Transcendent Memory)](https://lwn.net/Articles/338098), 对很多第一次听见这个概念的人来说, 是如此的奇怪和陌生. 这个概念是在 Linux 内核开发者社区中首次被提出的. 超然内存(后文一律简称为[**tmem**](https://lore.kernel.org/patchwork/patch/161314))之于普通内存, 不同之处在于以下几点:


1.  tmem 的大小对内核来说是未知的, 它甚至是可变的; 与之相比, 普通内存在系统初始化就可被内核探测并枚举, 并且大小是固定的(不考虑热插拔情形).

2.  tmem 可以是稳定的, 也可以是不稳定的, 对于后者, 这意味着, 放入其中的数据, 在之后的访问中可能发现不见了; 与之相比, 普通内存在系统加电状态下一直是稳定的, 你不用担心写入的数据之后访问不到(不考虑内存硬件故障问题).

3.  基于以上两个原因, tmem 无法被内核直接访问, 必须通过定义良好的 API 来访问 tmem 中的内存; 与之相比, 普通内存可以通过内存地址被内核直接访问.


初看之下, [tmem](https://lore.kernel.org/patchwork/patch/163284) 这三点奇异的特性, 似乎增加了不必要的复杂性, 尤其是第二条, 更是诡异无用处.

计算机界有句名言: 计算机科学领域的任何问题都可以通过增加一个间接的中间层来解决. tmem 增加的这些间接特性正是为了解决某些问题.


考虑虚拟化环境下, 虚拟机管理器(hypervisor) 需要管理维护各个虚拟客户机(Guest OS)的内存使用. 在常见的使用环境中, 我们新建一台虚拟机时, 总要事先配置其可用的内存大小. 然而这并不十分明智, 我们很难事先确切知道每台虚拟机的内存需求情况, 这难免造成有些虚拟机内存富余, 而有些则捉襟见肘. 如果能平衡这些资源就好了. 事实上, 存在这样的技术, hypervisor 维护一个内存池子, 其中存放的就是这些所谓的 tmem, 它能动态地平衡各个虚拟机的盈余内存, 更高效地使用. 这是 tmem 概念提出的最初来由.



再考虑另一种情形, 当一台机器内存使用紧张时, 它会把一些精心选择的内存页面写到交换外设中以腾出空间. 然而外设与内存存在着显著的读写速度差异, 这对性能是一个不利的影响. 如果在一个超高速网络相连的集群中, 网络访问速度比外设访问速度快, 可不可以有别的想法? tmem 又可以扮演一个重要的中间角色, 它可以把集群中所有节点的内存管理在一个池子里, 从而动态平衡各节点的内存使用, 当某一节点内存紧张时, 写到交换设备页其实是被写到另一节点的内存里...



还有一种情形, 旨在提高内存使用效率.. 比如大量内容相同的页面(全0页面)在池子里可以只保留一份; 或者, tmem 可以考虑对这些页面进行压缩, 从而增加有效内存的使用.



Linux 内核 从 3.X 系列开始陆续加入 tmem 相关的基础设施支持, 并逐步加入了关于内存压缩的功能. 进一步讨论内核中的实现前, 需要对这一问题再进一步细化, 以方便讨论细节.



前文说了内核需要通过 API 访问 tmem, 那么进一步, 可以细化为两个问题.

1.  内核的哪些部分内存可以被 tmem 管理呢? 有两大类, 即前面提到过的文件缓存页和匿名页.

2.  tmem 如何管理其池子中的内存. 针对前述三种情形, 有不同的策略. Linux 现在的主要解决方案是针对内存压缩, 提高内存使用效率.



针对这两个问题, 可以把内核对于 tmem 的支持分别分为**前端**和**后端**. 前端是内核与 tmem 通讯的接口; 而后端则实现 tmem 的管理策略.


## 11.1 tmem 前端
-------

### 11.1.1 前端接口之 CLEANCACHE
-------

**3.0(2011年7月发布)**


前面章节提到过, 内核会利用空闲内存缓存后备外设中的内容, 以期在近期将要使用时不用从缓慢的外设读取. 这些内容叫文件缓存页. 它的特点就是只要是页是干净的(没有被写过), 那么在系统需要内存时, 随时可以直接丢弃这些页面以腾出空间(因为它随时可以从后备文件系统中读取).



然而, 下次内核需要这些文件缓存页时, 又得从外设读取. 这时, tmem 可以起作用了. 内核丢弃时,假如这些页面被 tmem 接收并管理起来, 等内核需要的时候, tmem 把它归还, 这样就省去了读写磁盘的操作, 提高了性能. 3.0 引入的 CLEANCACHE, 作为 tmem 前端接口, hook 进了内核丢弃这些干净文件页的地方, 把这些页面截获了, 放进 tmem 后端管理(后方讲). 从内核角度看, 这个 CLEANCACHE 就像一个魔盒的入口, 它丢弃的页面被吸入这个魔盒, 在它需要时, 内核尝试从这个魔盒中找, 如果找得到, 搞定; 否则, 它再去外设读取. 至于为何被吸入魔盒的页面为什么会找不到, 这跟前文说过的 tmem 的特性有关. 后文讲 tmem 后端时再说.



### 11.1.2 前端接口之 FRONTSWAP
-------

**3.5(2012年7月发布)**


除了文件缓存页, 另一大类内存页面就是匿名页. 在系统内存紧张时, 内核必须要把这些页面写出到外设的交换设备或交换分区中, 而不能简单丢弃(因为这些页面没有后备文件系统). 同样, 涉及到读写外设, 又有性能考量了, 此时又是 tmem 起作用的时候了.



同样, FRONTSWAP 这个前端接口, 正如其名字一样, 在 内核 swap 路径的前面, 截获了这些页面, 放入 tmem 后端管理. 如果后端还有空闲资源的话, 这些页面被接收, 在内核需要这些页面时, 再把它们吐出来; 如果后端没有空闲资源了, 那么内核还是会把这些页面按原来的走 swap 路径写到交换设备中.



## 11.2 后端
-------

[In-kernel memory compression](https://lwn.net/Articles/545244)

### 11.2.1 后端之 ZCACHE
-------


**(没能进入内核主线)**

讲完了两个前端接口, 接下来说后端的管理策略. 对应于 CLEANCACHE, 一开始是有一个专门的后端叫 [zcache](https://lwn.net/Articles/397574), **不过最后被删除了.** 它的做法就是把这些被内核逐出的文件缓存页压缩, 并存放在内存中. 所以, zcache 相当于把内存页从内存中的一个地方移到另一个地方, 这乍一看, 感觉很奇怪, 但这正是 tmem 的灵活性所在. 它允许后端有不同的管理策略, 比如在这个情况下, 它把内存页压缩后仍然放在内存中, 这提高了内存的使用. 当然, 毕竟 zcache 会占用一部分物理内存, 导致可用的内存减小. 因此, 这需要有一个权衡. 高效(压缩比, 压缩时间)的压缩算法的使用, 从而使更多的文件页待在内存中, 使得其带来的避免磁盘读写的优势大于减少的这部分内存的代价. 不过, 也因为如此, 它的实现过于复杂, **以至最终没能进入内核主线.** 开发者在开始重新实现一个新的替代品, 不过截止至 4.2 , 还没有看到成果.



### 11.2.2 后端之 ZRAM
-------

**3.14(2014年3月发布)**



FRONTSWAP 对应的一个后端叫 ZRAM. 前身为 compcache, 它早在 2.6.33(2010年2月发布) 时就已经进入内核的 staging 分支经历了 4 年的 staging 后, 于内核 3.14 版本进入 mainline(2014年3月), 目前是谷歌 Android 系统(自4.4版本)和 Chrome OS(自2013年)使用的默认方案.

> Staging 分支是在内核源码中的一个子目录, 它是一个独立的分支, 主要维护着独立的 driver 或文件系统, 这些代码未来可能也可能不进入主线.


ZRAM 是一个在内存中的块设备(块设备相对于字符设备而言, 信息存放于固定大小的块中, 支持随机访问, 磁盘就是典型的块设备, 更多将在块层子系统中讲), 因此, 内核可以复用已有的 swap 设备设施, 把这个块设备格式化为 swap 设备. 因此, 被交换出去的页面, 将通过 FRONTSWAP 前端进入到 ZRAM 这个伪 swap 设备中, 并被压缩存放! 当然, 这个ZRAM 空间有限, 因此, 页面可能不被 ZRAM 接受. 如果这种情形发生, 内核就回退到用真正的磁盘交换设备.



### 11.2.3 后端之 ZSWAP
-------

**3.11(2013年9月发布)**


FRONTSWAP 对应的另一个后端叫 [ZSWAP](https://lwn.net/Articles/537422). ZSWAP 的做法其实也是尝试把内核交换出去的页面压缩存放到一个内存池子中. 当然, ZSWAP 空间也是有限的. 但同 ZRAM 不同的是, ZSWAP 会智能地把其中一些它认为近期不会使用的页面解压缩, 写回到真正的磁盘外设中. 因此, 大部分情况下, 它能避免磁盘写操作, 这比 ZRAM 不知高明到哪去了.

### 11.2.3.1 ZSWAP
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2021/08/19 | Johannes Weiner <hannes@cmpxchg.org> | [mm: Kconfig: simplify zswap configuration](https://lore.kernel.org/patchwork/patch/1479229) | 重构 CONFIG_ZSWAP. | v1 ☐ | [PatchWork](https://lore.kernel.org/patchwork/patch/1479229) |
| 2022/04/27 | Johannes Weiner <hannes@cmpxchg.org> | [zswap: cgroup accounting & control](https://patchwork.kernel.org/project/linux-mm/cover/20220427160016.144237-1-hannes@cmpxchg.org/) | 636247 | v1 ☐☑ | [LORE v1,0/5](https://lore.kernel.org/r/20220427160016.144237-1-hannes@cmpxchg.org)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/6](https://lore.kernel.org/all/20220510152847.230957-1-hannes@cmpxchg.org) |


### 11.2.3.2 ZTREE
-------

| 时间   | 作者  | 特性 | 描述  |  是否合入主线  | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2022/02/28 | Ananda <a.badmaev@clicknet.pro> | [[PATCH/RESEND] mm: add ztree - new allocator for use via zpool API](https://patchwork.kernel.org/project/linux-mm/patch/20220228110546.151513-1-a.badmaev@clicknet.pro/) | 用于压缩页面的专用分配器 Ztree. 在大多数情况下, Ztree 提供了快速写入、有限的最坏情况操作时间和良好的压缩比.<br>每个 Ztree 块存储整数个压缩对象. 这些块由几个物理页面(从1到8)组成, 并使用红黑树用于高效的区块组织.<br>从 0 到 PAGE_SIZE 的范围被划分为与树的数量相对应的区间数, 每棵树只操作其区间中的大小对象.<br>1. 块树彼此隔离, 这使得同时对来自不同树的多个对象执行操作成为可能. 块可以密集地排列各种大小的对象, 从而降低内部碎片.<br>2. 此外, 这个分配器试图填充不完整的块, 而不是添加新的块, 因此在许多情况下, 它提供的压缩比大大高于 z3fold 和 zbud.<br>3. 除了更大的灵活性, Ztree 在最糟糕的执行时间方面明显优于其他 ZPOOL 后端, 从而允许更好的响应. | v1 ☐☑ |[LORE v1,0/1](https://lore.kernel.org/r/20220228110546.151513-1-a.badmaev@clicknet.pro)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/r/20220301092503.44444-1-a.badmaev@clicknet.pro)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/1](https://lore.kernel.org/r/20220307142724.14519-1-a.badmaev@clicknet.pro)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/1](https://lore.kernel.org/r/20220311085807.27038-1-a.badmaev@clicknet.pro) |

### 11.2.3.3 ZBUD
-------

| 时间   | 作者  | 特性 | 描述  |  是否合入主线  | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2013/09/11 | Krzysztof Kozlowski <k.kozlowski@samsung.com> | [mm: migrate zbud pages](https://lore.kernel.org/all/1378889944-23192-1-git-send-email-k.kozlowski@samsung.com) | 1378889944-23192-1-git-send-email-k.kozlowski@samsung.com | v2 ☐☑✓ | [LORE v2,0/5](https://lore.kernel.org/all/1378889944-23192-1-git-send-email-k.kozlowski@samsung.com) |
| 2015/09/22 | Vitaly Wool <vitalywool@gmail.com> | [zbud: allow up to PAGE_SIZE allocations](https://lore.kernel.org/all/20150922141733.d7d97f59f207d0655c3b881d@gmail.com) | 20150922141733.d7d97f59f207d0655c3b881d@gmail.com | v2 ☐☑✓ | [LORE](https://lore.kernel.org/all/20150922141733.d7d97f59f207d0655c3b881d@gmail.com) |



## 11.3 一些细节
-------


这一章基本说完了, 但牵涉到后端, 其实还有一些细节可以谈, 比如对于压缩的效率的考量, 会影响到后端实现的选择, 比如不同的内存页面的压缩效果不同(全0页和某种压缩文件占据的内存页的压缩效果显然差距很大)对压缩算法的选择; 压缩后页面的存放策略也很重要, 因为以上后端都存在特殊情况要把页面解压缩写回到磁盘外设, 写回页面的选择与页面的存放策略关系很大. 但从用户角度讲, 以上内容足以, 就不多写了.



关于 tmem, lwn 上的两篇文章值得关注技术细节的人一读:

[Transcendent memory in a nutshell [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/454795)

[In-kernel memory compression [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/545244)



# 12 分级内存支持
-------

## 12.1 分级内存
-------

### 12.1.1 不同介质的内存的支持
-------

#### 12.1.1.1 非易失性内存 (NVDIMM, Non-Volatile DIMM) 支持
-------

[Linux Kernel中AEP的现状和发展](https://kernel.taobao.org/2019/05/NVDIMM-in-Linux-Kernel)


计算机的存储层级是一个金字塔体系, 从塔尖到塔基, 访问速度递减, 而存储容量递增. 从访问速度考量, 内存(DRAM)与磁盘(HHD)之间, 存在着[显著的差异(可达到10^5级别)](https://www.directionsmag.com/article/3794). 因此, 基于内存的缓存技术一直都是系统软件或数据库软件的重中之重. 即使近些年出现的新兴的最快的基于PCIe总线的SSD, 这中间依然存在着鸿沟.


![](https://pic4.zhimg.com/50/0c0850cde43c84764e65bc24942bc6d3_hd.jpg)


另一方面, 非可易失性内存也并不是新鲜产物. 然而实质要么是一块DRAM, 后端加上一块 NAND FLASH 闪存, 以及一个超级电容, 以在系统断电时的提供保护; 要么就是一块简单的 NAND FLASH, 提供类似 SSD 一样的存储特性. 所有这些, 从访问速度上看, 都谈不上真正的内存, 并且, NAND FLASH 的物理特性, 使其免不了磨损(wear out); 并且在长时间使用后, 存在写性能下降的问题.



2015 年算得上闪存技术革命年. 3D NAND FLASH 技术的创新, 以及 Intel 在酝酿的完全不同于NAND 闪存技术的 3D XPoint 内存, 都将预示着填充上图这个性能鸿沟的时刻的临近. 它们不仅能提供更在的容量(TB级别), 更快的访问速度(3D XPoint 按 Intel 说法能提供 `~1000` 倍快于传统的 NAND FLASH, 5 - 8 倍慢于 DRAM 的访问速度), 更持久的寿命.



相应的, Linux 内核也在进行相应的功能支持.


#### 12.1.1.2 NVDIMM 支持框架
-------

** libnvdimm 4.2(2015年8月30日发布)**

2015 年 4 月发布的 [ACPI 6.0 规范](https://uefi.org/sites/default/files/resources/ACPI/_6.0.pdf](https://link.zhihu.com/?target=http%3A//www.uefi.org/sites/default/files/resources/ACPI_6.0.pdf), 定义了NVDIMM Firmware Interface Table (NFIT), 详细地规定了 NVDIMM 的访问模式, 接口数据规范等细节. 在 Linux 4.2 中, 内核开始支持一个叫 libnvdimm 的子系统, 它实现了 NFIT 的语义, 提供了对 NVDIMM 两种基本访问模式的支持, 一种即内核所称之的 PMEM 模式, 即把 NVDIMM 设备当作持久性的内存来访问; 另一种则提供了块设备模式的访问. 开始奠定 Linux 内核对这一新兴技术的支持.

#### 12.1.1.3 DAX
-------

**4.0(2015年4月发布)**

与这一技术相关的还有另外一个特性值得一提, 那就是 DAX(Direct Access, 直接访问, X 无实义, 只是为了酷).



传统的基于磁盘的文件系统, 在被访问时, 内核总会把页面通过前面所提的文件缓存页(page cache)的缓存机制, 把文件系统页从磁盘中预先加载到内存中, 以提速访问. 然后, 对于新兴的 NVDIMM 设备, 基于它的非易失特性, 内核应该能直接访问基于此设备之上的文件系统的内容, 它使得这一拷贝到内存的操作变得不必要. 4.0 开始引入的 DAX 就是提供这一支持. 截至 4.3, 内核中已经有 XFS, EXT2, EXT4 这几个文件系统实现这一特性.


#### 12.1.1.4 Compute Express Link(CXL)
-------

[LWN: LSFMM-2022/CXL 1: Management and tiering](https://lwn.net/Articles/894598)

[](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.18-NUMA-Regression-Fix)

### 12.1.2 多级内存(Top-tier memory management)/内存分级(memory tiering) 支持
-------

#### 12.1.2.1 NUMA nodes for persistent-memory management
-------

多亏了 Dave Hansen 的补丁 [Allow persistent memory to be used like normal RAM](https://lore.kernel.org/patchwork/patch/1045596), 它使 PMEM 作为 NUMA 节点的一部分当普通内存一样来使用.

如何同时使用 PMEM 和普通 DRAM 仍然是一个悬而未决的问题. 各家厂商提出了不同的思路和想法.

Intel 的吴峰光 [PMEM NUMA node and hotness accounting/migration](https://lore.kernel.org/linux-mm/20181226131446.330864849@intel.com)

[Page demotion for memory reclaim](https://lore.kernel.org/linux-mm/20190321200157.29678-1-keith.busch@intel.com)

阿里巴巴的 Yang Shi [Another Approach to Use PMEM as NUMA Node](https://lore.kernel.org/linux-mm/1553316275-21985-1-git-send-email-yang.shi@linux.alibaba.com), [mm: vmscan: demote anon DRAM pages to PMEM node](https://lore.kernel.org/linux-mm/6A903D34-A293-4056-B135-6FA227DE1828@nvidia.com)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/12/26 | Fengguang Wu <fengguang.wu@intel.com> | [PMEM NUMA node and hotness accounting/migration](https://lore.kernel.org/patchwork/patch/1027864) | 1. 隔离DRAM和PMEM. 为PMEM单独构造了一个zonelist, 这样一般的内存分配是不会分配到PMEM上的<br>2. 跟踪内存的冷热. 利用内核中已经有的 idle page tracking 功能(目前主线内核只支持系统全局的tracking), 在per process的粒度上跟踪内存的冷热 <br>3. 利用现有的page reclaim, 在 reclai m时将冷内存迁移到 PMEM 上(只能迁移匿名页).  <br>4. 利用一个 userspace 的 daemon 和 idle page tracking, 来将热内存(在PMEM上的)迁移到 DRA M中. | RFC v2 ☐ 4.20 | PatchWork RFC,v2,00/21](https://lore.kernel.org/patchwork/patch/1027864), [LKML](https://lkml.org/lkml/2018/12/26/138), [github/intel/memory-optimizer](http://github.com/intel/memory-optimizer) |
| 2019/02/25 | Dave Hansen <dave.hansen@linux.intel.com><br>Huang Ying <ying.huang@intel.com> | [Allow persistent memory to be used like normal RAM](https://lore.kernel.org/patchwork/patch/1045596) | 通过memory hotplug的方式把PMEM添加到Linux的buddy allocator里面. 新添加的PMEM会以一个或多个NUMA node的形式出现, Linux Kernel就可以分配PMEM上的memory, 这样和使用一般DRAM没什么区别 | v5 ☑ 5.1-rc1 | [PatchWork v5,0/5](https://lore.kernel.org/patchwork/patch/1045596) |
| 2019/04/11 | Fengguang Wu <fengguang.wu@intel.com> | [Another Approach to Use PMEM as NUMA Node](https://patchwork.kernel.org/project/linux-mm/cover/1554955019-29472-1-git-send-email-yang.shi@linux.alibaba.com) | 通过 memory reclaim 把"冷" 内存迁移到慢速的 PMEM node 中, NUMA Balancing 访问到这些冷 page 的时候可以选择是否把这个页迁移回 DRAM, 相当于是一种比较粗粒度的"热"内存识别. | RFC v2 ☐ 4.20 | [PatchWork v2,RFC,0/9](https://patchwork.kernel.org/project/linux-mm/cover/1554955019-29472-1-git-send-email-yang.shi@linux.alibaba.com) |
| 2019/04/04 | Zi Yan <zi.yan@sent.com> | [Accelerate page migration and use memcg for PMEM management](https://patchwork.kernel.org/project/linux-mm/cover/20190404020046.32741-1-zi.yan@sent.com) | TODO | RFC ☐ | [PatchWork RFC,00/25](https://patchwork.kernel.org/project/linux-mm/cover/20190404020046.32741-1-zi.yan@sent.com) |


#### 12.1.2.2 memory tiering
-------


[Top-tier memory management](https://lwn.net/Articles/857133)

[LWN: Top-tier memory management!](https://blog.csdn.net/Linux_Everything/article/details/117970246)

[NUMA rebalancing on tiered-memory systems](https://lwn.net/Articles/893024)

[Linux Developers Discuss Improvements To Memory Tiering](https://www.phoronix.com/scan.php?page=news_item&px=Linux-Better-Memory-Tiering)

[RFC: Memory Tiering Kernel Interfaces (v2)](https://lore.kernel.org/lkml/CAAPL-u-DGLcKRVDnChN9ZhxPkfxQvz9Sb93kVoX_4J2oiJSkUw@mail.gmail.com)

[Two memory-tiering patch sets](https://lwn.net/Articles/898766)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/04/15 | Tim Chen <tim.c.chen@linux.intel.com> | [Manage the top tier memory in a tiered memory](https://lore.kernel.org/patchwork/patch/1408180) |  memory tiers 的配置管理. 监控系统和每个 cgroup 中各自使用的 top-tier 内存的数量. 当前使用了 soft limit, 用 kswapd 来把某个 cgroup 中超过 soft limit 限制的 page 迁移到较慢的 memory 类型上去. 这里所说的 soft limit, 是指如果 top-tier memory 很充足的话, cgroup 可以拥有超过此限制的 page 数量, 但如果资源紧张的话则会被迅速削减从而满足这个 limit 值. 后期可用于对于不同的任务划分快、慢内存(fast and slow memory). 即让高优先级的任务获得更多 top-tier memory 访问优先, 而低优先级的任务则要受到更严格的限制 | v1 ☐ 5.13 | [PatchWork RFC,v1,00/11](https://patchwork.kernel.org/project/linux-mm/cover/cover.1617642417.git.tim.c.chen@linux.intel.com/) |
| 2021/07/15 | Dave Hansen <dave.hansen@linux.intel.com><br>Huang Ying <ying.huang@intel.com> | [Migrate Pages in lieu of discard](https://lore.kernel.org/patchwork/patch/1393431) | 页面回收阶段引入页面降级(demote pages)策略. 在一个具备了持久性内存的系统中(这些系统具有多种类型的内存, 具有不同的性能特征, 而不是普通 NUMA 系统, 在普通 NUMA 系统中, 相同类型的内存存在于不同的距离), 在回收期间允许页面迁移使这些系统能够在快速层承受压力时将页面从快速层迁移到慢速层. 具体做法是可以把一些需要回收的 page 从 DRAM 迁移到较慢的 memory 中, 后面如果再次需要这些数据了, 也是仍然可以直接访问到, 只是速度稍微慢一些. 目前的版本还不完善, 被迁移的 page 将被永远困在慢速内存区域中, 没有机制使其回到更快的 DRAM.(Huang Ying 后续工作, 重新调整 autonuma 的用途, 将热门页面推广回 DRAM). 这个降级策略可以通过 sysctl 的 ~~vm.zone_reclaim_mode 中把 bitmask 设置为 8 从而启用这个功能~~ `/sys/kernel/mm/numa/demotion_enabled` 开关来控制. 参见报道 [Linux 5.15 Has A Critical Improvement For Tiered Memory Servers](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.15-Demote-During-Reclai). | v10 ☑ 5.15-rc1 | [PatchWork -V10,0/9](https://patchwork.kernel.org/project/linux-mm/cover/20210715055145.195411-1-ying.huang@intel.com), [PatchWork -V10,0/9](https://lore.kernel.org/all/20210715055145.195411-1-ying.huang@intel.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=26aa2d199d6f2cfa6f2ef2a5dfe891f2250e71a0) |
| 2022/02/21 | Huang Ying <ying.huang@intel.com> | [NUMA balancing: optimize memory placement for memory tiering system](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Optimize-PMEM-Place-v9) | 将经常使用的 page 从慢速内存迁移到快速内存的方案. 对 Linux 内核的 NUMA balancing 行为进行了调整, 以优化内存分层系统的内存位置. 这些补丁进一步优化了内核在持久内存存在的情况下对页面的处理, 同时将最重要的页面保留在 DRAM 中.<br>优化 numa balancing 的页面迁移策略, 利用了这些 numa fault 来对哪些 page 属于常用 page 进行更准确地估算. 新的策略依据从 page unmmap 到发生 page fault 之间的时间差来判断 page 是否常用, 并提供了一个 sysctl 开关来定义阈值: kernel.numa_balancing_hot_threshold_ms. 所有 page fault 时间差低于阈值的 page 都被判定是常用 page. 由于对于系统管理员来说可能很难决定这个阈值应该设置成什么值, 所以这组 patch 中的实现了自动调整的方法. 为了实现这个自动调整, kernel 会根据用户设置的平衡速率限制(balancing rate limit). 内核会对迁移的 page 数量进行监控, 通过增加或减少阈值来使 balancing rate 更接近该目标值. 仓库地址 [vishal/tiering.git](https://git.kernel.org/pub/scm/linux/kernel/git/vishal/tiering.git/log/?h=tiering-0.72). 参见报道 [Intel Continues Optimizing Linux For Optane DC Persistent Memory Servers](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Optimize-PMEM-Place-v9) | v13 ☑ 5.18-rc1 | [PatchWork RFC,-V6,0/6](https://lore.kernel.org/patchwork/patch/1393431)<br>*-*-*-*-*-*-*-* <br>[PatchWork -V8,0/6](https://patchwork.kernel.org/proje3t/linux-mm/cover/20210914013701.344956-1-ying.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork -V9,0/6](https://patchwork.kernel.org/project/linux-mm/cover/20211008083938.1702663-1-ying.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork -V10,0/6](https://patchwork.kernel.org/project/linux-mm/cover/20211116013522.140575-1-ying.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v13,0/3](https://lore.kernel.org/r/20220221084529.1052339-1-ying.huang@intel.com) |
| 2021/10/14 | Yang Shi <shy828301@gmail.com> | [mm: migrate: make demotion knob depend on migration](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=20f9ba4f995247bb79e243741b8fdddbd76dd923) | NA | v1 ☑✓ 5.16-rc1 | [LORE](https://lore.kernel.org/all/20211015005559.246709-1-shy828301@gmail.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=20f9ba4f995247bb79e243741b8fdddbd76dd923) |
| 2021/11/24 | Hasan Al Maruf <hasan3050@gmail.com> | [Transparent Page Placement for Tiered-Memory](https://patchwork.kernel.org/project/linux-mm/cover/cover.1637778851.git.hasanalmaruf@fb.com) | 由于不同类型的内存对性能的影响程度不同, 因此如何跨 NUMA 节点管理页面应该是一个值得关注的问题.  Dave Hansen 的补丁集 ["Migrate Pages in replace of discard"](https://lwn.net/Articles/860215) 在回收过程中将顶级页面降级到慢级节点. 然而, 他的补丁集不包括将慢层内存节点上的页面提升到顶级内存节点的功能. 因此, 在慢层节点上降级或新分配的页面将经历 NUMA 延迟, 并损害应用程序性能. 在这个补丁集中,<br>1. 通过增强现有的 AutoNUMA 机制, 将页面从慢级节点提升到顶级节点.<br>2. 将顶级节点的回收和分配逻辑解耦, 以便回收在更高的水印处触发, 并将较冷的页面降级到慢层内存中. 因此, 顶级节点可以保留一些空闲空间, 以接受来自慢级节点的新分配和提升.<br>在对页面升级期间, 添加滞后性, 只升级那些在短时间内不太可能被降级页面. 这减少了页面在短时间内频繁降级和提升而在 NUMA 节点之间来回跳转的机会. 作者在支持 cxl 的 DRAM 和 PMEM 层的系统上测试了这个补丁集. 可以将较热的页面转移到顶级节点, 而将较冷的页面移动到慢层节点, 从而产生大量的 Meta 生产工作负载和实时流量. 因此, 顶级节点提供更多的热页面, 应用程序的性能也得到了提高. 将 80% 的匿名者带到顶级节点. 慢层内存中的匿名页大多是冷页. 由于顶级节点不能承载所有热内存, 一些热文件仍然留在慢级节点上. 尽管如此, 远程 NUMA 读带宽从 80% 减少到 40%. 与服务于整个工作集的顶级节点的基线相比, 使用这个补丁集的吞吐量回归仅为 5%. 参见报道 [Facebook/Meta Tackling Transparent Page Placement For Tiered-Memory Linux Systems](https://www.phoronix.com/scan.php?page=news_item&px=Meta-Hot-Pages-High-Tiers). | v1 ☐ | [PatchWork 0/5]https://patchwork.kernel.org/project/linux-mm/cover/cover.1637778851.git.hasanalmaruf@fb.com) |
| 2021/12/11 | Baolin Wang <baolin.wang@linux.alibaba.com> | [Add speculative numa fault support](https://lore.kernel.org/patchwork/patch/1479229) | 这个 RFC 补丁集为分级内存等场景添加了推测性的 numa 错误支持. 在分层内存系统上, 它将依靠 numa 平衡将慢内存和热内存提升为快内存, 以提高性能. 因此, 我们可以根据某些工作负载的数据局部性, 提前在低速内存中提升多个顺序页面, 以提高性能. 那么现在有多少页面需要提升到最快的内存是最好的? 现在这个 RFC 补丁集只实现了一个基本而简单的机制来推测每个 VMA 的 numa 故障窗口. 它将为每个 VMA 引入一个新的原子成员来记录 numa 故障窗口信息, 该信息用于确定它是扩展还是减少 numa 故障窗口的顺序流. 在分层内存系统中测试 mysql 可以看到大约 6% 的改进. 注意: 这个补丁集是[基于实现分级内存提升的补丁集 NUMA balancing: optimize page placement for memory tiering system](https://lore.kernel.org/lkml/87bl2gsnrd.fsf@yhuang6-desk2.ccr.corp.intel.com). 参见报道 [Speculative NUMA Fault Support Proposed For Improving Tiered Memory Linux Performance](https://www.phoronix.com/scan.php?page=news_item&px=Speculative-NUMA-Fault) | v1 ☐ | [LORE](https://lore.kernel.org/lkml/cover.1639306956.git.baolin.wang@linux.alibaba.com), [PatchWork](https://patchwork.kernel.org/project/linux-mm/cover/cover.1639306956.git.baolin.wang@linux.alibaba.com) |
| 2022/07/13 | Huang Ying <ying.huang@intel.com> | [memory tiering: hot page selection](https://patchwork.kernel.org/project/linux-mm/cover/20220408071222.219689-1-ying.huang@intel.com/) | 在分级内存的系统中, 需要使用 NUMA 来优化页面在内存中的位置. 基本上, 最初的 NUMA Balancing 实现选择并提升最近访问最多(mostly recently accessed/MRU)的页面. 但最近访问的页面可能很冷. 因此, 在这个补丁集中, 我们基于 NUMA 平衡页表扫描和页面 Page Fault & Hint 之间的延迟实现了一个新的热页识别算法. 另外一方面热页升级可能会在系统中产生一些开销. 为了控制开销, 实现了一种简单的提升速率限制机制. 用于识别热页的热阈值通常取决于工作负载. 因此, 我们还实现了一个热页阈值自动调整算法. 基本思想是增加/减少热页阈值, 使通过热页阈值提升到快速内存的页面数接近速率限制. | v1 ☐☑ | [2022/04/08 LORE v1,0/3](https://lore.kernel.org/r/20220408071222.219689-1-ying.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[2022/06/22 LORE v1,0/3](https://lore.kernel.org/r/20220622083519.708236-1-ying.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[2022/07/13 LORE v1,0/3](https://lore.kernel.org/r/20220713083954.34196-1-ying.huang@intel.com) |
| 2022/04/13 | Jagdish Gediya <jvgediya@linux.ibm.com> | [mm: demotion: Introduce new node state N_DEMOTION_TARGETS](https://patchwork.kernel.org/project/linux-mm/cover/20220413092206.73974-1-jvgediya@linux.ibm.com/) | 631805 | v2 ☐☑ | [LORE v2,0/5](https://lore.kernel.org/r/20220413092206.73974-1-jvgediya@linux.ibm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/7](https://lore.kernel.org/r/20220422195516.10769-1-jvgediya@linux.ibm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/3](https://lore.kernel.org/r/20220426085105.60822-1-ying.huang@intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/3](https://lore.kernel.org/r/20220510063958.86985-1-ying.huang@intel.com) |
| 2022/07/14 | Aneesh Kumar K.V <aneesh.kumar@linux.ibm.com> | [mm/demotion: Memory tiers and demotion](https://patchwork.kernel.org/project/linux-mm/cover/20220527122528.129445-1-aneesh.kumar@linux.ibm.com/) | 645595 | v4 ☐☑ | [LORE v4,0/7](https://lore.kernel.org/r/20220527122528.129445-1-aneesh.kumar@linux.ibm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/9](https://lore.kernel.org/r/20220603134237.131362-1-aneesh.kumar@linux.ibm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v6,0/13](https://lore.kernel.org/r/20220610135229.182859-1-aneesh.kumar@linux.ibm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/12](https://lore.kernel.org/r/20220622082513.467538-1-aneesh.kumar@linux.ibm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v9,0/8](https://lore.kernel.org/r/20220714045351.434957-1-aneesh.kumar@linux.ibm.com)<br>*-*-*-*-*-*-*-* <br>[LORE v10,0/8](https://lore.kernel.org/r/20220720025920.1373558-1-aneesh.kumar@linux.ibm.com) |
| 2022/06/06 | Aneesh Kumar K V <aneesh.kumar@linux.ibm.com> | [mm/demotion: Add sysfs ABI documentation](https://patchwork.kernel.org/project/linux-mm/patch/87r1428k9n.fsf@linux.ibm.com/) | 647508 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/87r1428k9n.fsf@linux.ibm.com) |
| 2022/06/07 | Johannes Weiner <hannes@cmpxchg.org> | [mm: mempolicy: N:M interleave policy for tiered memory nodes](https://patchwork.kernel.org/project/linux-mm/patch/20220607171949.85796-1-hannes@cmpxchg.org/) | 648110 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220607171949.85796-1-hannes@cmpxchg.org) |
| 2022/06/14 | Tim Chen <tim.c.chen@linux.intel.com> | [Cgroup accounting of memory tier usage](https://patchwork.kernel.org/project/linux-mm/cover/cover.1655242024.git.tim.c.chen@linux.intel.com/) | 650358 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/cover.1655242024.git.tim.c.chen@linux.intel.com) |


## 12.2 远端内存
-------

[持久内存 - RDMA 让远程数据不再远](http://blog.itpub.net/31493717/viewspace-2731431)

[详谈 RDMA（远程直接内存访问）技术原理和三种实现方式](https://blog.csdn.net/Rong_Toa/article/details/114747763)

[关于 RDMA 技术原理、三种主流实现技术对比](https://www.idcbest.com/idcnews/11004565.html)

## 12.3 异构内存(CPU & GPU)
-------


AMD 正在研究一种异构超级计算机架构, 该架构在 CPU 和 GPU 之间具有一致的互连. 这种硬件架构允许 CPU 连贯地访问 GPU 设备内存. Alex Sierra 所在的实验室正在与合作伙伴 HPE 合作, 开发 BIOS、固件和软件, 以交付给 DOE.

系统 BIOS 在 UEFI 系统地址映射中将 GPU 设备内存(也称为VRAM)作为 SPM(专用内存)播发. AMD GPU 驱动程序可以通过 lookup_resource() 查找到这些内存资源, 并使用  devm_memremap_pages() 将其注册为通用内存设备(MEMORY_DEVICE_GENERIC)使用.

补丁集 [Support DEVICE_GENERIC memory in migrate_vma_*](https://lore.kernel.org/patchwork/patch/1472581) 进行了一些更改, 尝试通过使用 `migrate_vma_*()` 辅助函数将数据迁移到该内存中, 以便在统一内存分配中支持基于页面的迁移, 同时还支持 CPU 直接访问对这些页面.

这项工作基于 HMM 和 SVM 内存管理器, 该内存管理器最近被上传到 Dave Airlie 的 [drm-next](https://cgit.freedesktop.org/drm/drm/log/?h=drm-next), 其中对用于迁移的 VRAM 管理进行了一些返工, 以消除一些不正确的假设, 允许在 VRAM 和系统内存中混合页面的部分成功迁移和 GPU 内存映射, 参见 [drm/amdkfd: add owner ref param to get hmm pages Felix](https://lore.kernel.org/dri-devel/20210527205606.2660-6-Felix.Kuehling@amd.com/T/#r996356015e295780eb50453e7dbd5d0d68b47cbc)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/05 | Alex Sierra <alex.sierra@amd.com> | [Support DEVICE_GENERIC memory in migrate_vma_*](https://lore.kernel.org/patchwork/patch/1472581) | NA | v1 ☐ | [PatchWork v6,00/13](https://patchwork.kernel.org/project/linux-mm/cover/20210813063150.2938-1-alex.sierra@amd.com) |





# 13 内存管理调试支持
-------




由前面所述, 内存管理相当复杂, 代码量巨大, 而它又是如此重要的一个的子系统, 所以代码质量也要求非常高. 另一方面, 系统各个组件都是内存管理子系统的使用者, 而如果缺乏合理有效的约束, 不正当的内存使用(如内存泄露, 内存覆写)都将引起系统的崩溃, 以至于数据损坏. 基于此, 内存管理子系统引入了一些调试支持工具, 方便开发者/用户追踪,调试内存管理及内存使用中的问题. 本章介绍内存管理子系统中几个重要的调试工具.



## 13.1 页分配的调试支持
-------


**2.5(2003年7月之后发布)**

前面提到过, 内核自己用的内存, 由于是绕过寻常的逐级页表机制, 采用直接映射(提高了效率), 即虚拟地址与页面实际的物理地址存在着一一线性映射的关系. 另一方面, 内核使用的内存又是出于各种重要管理目的, 比如驱动, 模块, 文件系统, 甚至 SLAB 子系统也是构建于页分配器之上. 以上二个事实意味着, 相邻的页, 可能被用于完全不同的目的, 而这两个页由于是直接映射, 它们的虚拟地址也是连续的. 如果某个使用者子系统的编码有 bug, 那么它的对其内存页的写操作造成对相邻页的覆写可能性相当大, 又或者不小心读了一个相邻页的数据. 这些操作可能不一定马上引起问题, 而是在之后的某个地方才触发, 导致数据损坏乃至系统崩溃.



为此, 2.5中, 针对 Intel 的 i386 平台, 内核引入了一个 **CONFIG\_DEBUG\_PAGEALLOC** 开关, 它在页分配的路径上插入钩子, 并利用 i386 CPU 可以对页属性进行修改的特性, 通过修改未分配的页的页表项的属性, 把该页置为"隐藏". 因此, 一旦不小心访问该页(读或写), 都将引起处理器的缺页异常, 内核将进入缺页处理过程, 因而有了一个可以检查捕捉这种内存破坏问题的机会.



在2.6.30中, 又增加了对无法作处理器级别的页属性修改的体系的支持. 对于这种体系, 该特性是将未分配的页**毒化(POISON),** 写入特定模式的值, 因而一旦被无意地访问(读或写), 都将可能在之后的某个时间点被发现. **注意, 这种通用的方法就无法像前面的有处理器级别支持的方法有立刻捕捉的机会.**





**当然, 这个特性是有性能代价的, 所以生产系统中可别用哦.**



## 13.2 SLAB 子系统的调试支持
-------


SLAB 作为一个相对独立的子模块, 一直有自己完善的调试支持, 包括有:



- 对已分配对象写的边界超出的检查

- 对未初始化对象写的检查

- 对内存泄漏或多次释放的检查

- 对上一次分配者进行记录的支持等

## 13.3 内存检测工具
-------


### 13.3.1 错误注入机制
-------

**2.6.20(2007年2月发布)**


内核有着极强的健壮性, 能对各种错误异常情况进行合理的处理. 然而, 毕竟有些错误实在是极小可能发生, 测试对这种小概率异常情况的处理的代码实在是不方便. 所以, 2.6.20中内核引入了错误注入机制, 其中跟 MM 相关的有两个, 一个是对页分配器的失败注入, 一个是对 SLAB 对象分配器的失败注入. 这两个注入机制, 可以触发内存分配失败, 以测试极端情况下(如内存不足)系统的处理情况.

### 13.3.2 KMEMCHECK - 内存非法访问检测工具
-------

**2.6.31(2009年9月发布)**

对内存的非法访问, 如访问未分配的内存, 或访问分配了但未初始化的内存, 或访问了已释放了的内存, 会引起很多让人头痛的问题, 比如程序因数据损坏而在某个地方莫名崩溃, 排查非常困难. 在用户态内存检测工具 valgrind 中, 有一个 Memcheck 插件可以检测此类问题. 2.6.31, Linux 内核也引进了内核态的对应工具, 叫 KMEMCHECK.



它是一个内核态工具, 检测的是内核态的内存访问. 主要针对以下问题:

> 1. 对已分配的但未初始化的页面的访问
> 2. 对 SLAB 系统中未分配的对象的访问
> 3. 对 SLAB 系统中已释放的对象的访问

为了实现该功能, 内核引入一个叫**影子页(shadow page)**的概念, 与被检测的正常**数据页**一一相对. 这也意味着启用该功能, 不仅有速度开销, 还有很大的内存开销.



在分配要被追踪的数据页的同时, 内核还会分配等量的影子页, 并通过数据页的管理数据结构中的一个 **shadow** 字段指向该影子页. 分配后, 数据页的页表中的 **present** 标记会被清除, 并且标记为被 KMEMCHECK 跟踪. 在第一次访问时, 由于 **present** 标记被清除, 将触发缺页异常. 在缺页异常处理程序中, 内核将会检查此次访问是不是正常. 如果发生上述的非法访问, 内核将会记录下该地址, 错误类型, 寄存器, 和栈的回溯, 并根据配置的值大小, 把该地址附近的数据页和影子页的对应内容, 一并保存到一个缓冲区中. 并设定一个稍后处理的软中断任务, 把这些内容报告给上层. 所有这些之后, 将 CPU 标志寄存器 TF (Trap Flag)置位, 于是在缺页异常后, CPU 又能重新访问这条指令, 走正常的执行流程.



这里面有个问题是, **present** 标记在第一次缺页异常后将被置位, 之后如果再次访问, KMEMCHECK 如何再次利用该机制来检测呢? 答案是内核在上述处理后还**打开了单步调试功能,** 所以 CPU 接着执行下一条指令前, 又陷入了调试陷阱(debug trap, 详情请查看 CPU 文档), 在处理程序中, 内核又会把该页的 **present** 标记会清除.


## 13.3.3 KMEMLEAK - 内存泄漏检测工具
-------

**2.6.31(2009年9月发布)**

内存漏洞一直是 C 语言用户面临的一个问题, 内核开发也不例外. 2.6.31 中, 内核引入了 [KMEMLEAK 工具](https://lwn.net/Articles/187979), 用以检测内存泄漏. 它采用了标记-清除的垃圾收集算法, 对通过 SLAB 子系统分配的对象, 或通过 _vmalloc_ 接口分配的连续虚拟地址的对象, 或分配的per-CPU对象(per-CPU对象是指每个 CPU 有一份拷贝的全局对象, 每个 CPU 访问修改本地拷贝, 以提高性能)进行追踪, 把指向对象起始的指针, 对象大小, 分配时的栈踪迹(stack trace) 保存在一个红黑树里(便于之后的查找, 同时还会把对象加入一个全局链表中). 之后, KMEMLEAK 会启动一个每10分钟运行一次的内核线程, 或在用户的指令下, 对整个内存进行扫描. 如果某个对象**从其起始地址到终末地址**内没有别的指针指向它, 那么该对象就被当成是泄漏了. KMEMLEAK 会把相关信息报告给用户.



扫描的大致算法如下:



> 1. 首先会把全局链表中的对象加入一个所谓的**白名单**中, 这是所有待查对象. 然后 , 依次扫描数据区(data段, bss段), per-CPU区, 还有针对每个 NUMA 节点的所有页. 另外, 如果用户有指定, 还会描扫所有线程的栈区(之所以这个不是强制扫描, 是因为栈区是函数的活动记录, 变动迅速, 引用可能稍纵即逝). 扫描过程中, 与之前存的红黑树中的对象进行比对, 一旦发现有指针指向红黑树中的对象, 说明该对象仍有人引用 , 没被泄漏, 把它加入一个所谓的**灰名单**中.
> 2. 然后, 再扫描一遍灰名单, 即已经被确认有引用的对象, 找出这些对象可能引用的别的所有对象, 也加入灰名单中.
> 3. 最后剩下的, 在白名单中的, 就是被 KMEMLEAK 认为是泄漏了的对象.



由于内存的引用情况各异, 存在很多特殊情况, 可能存在误报或漏报的情况, 所以 KMEMLEAK 还提供了一些接口, 方便使用者告知 KMEMLEAK 某些对象不是泄露, 某些对象不用检查,等等.



这个工具当然也存在着显著的影响系统性能的问题, 所以也只是作为调试使用.

### 13.3.4 KASAN - 内核地址净化器
-------

**4.0(2015年4月发布)**

[ASAN 和 HWASAN 原理解析](https://blog.csdn.net/21cnbao/article/details/107096665)

[浅谈 ARM64 基于硬件 tag 的 KASAN](http://news.eeworld.com.cn/mp/ymc/a133074.jspx)

#### 13.3.4.1 Generic KASAN
-------


4.0 引入的 [KASan (The kernel address sanitizer)](https://lwn.net/Articles/612153) 可以看作是 KMEMCHECK 工具的替代器, 它的目的也是为了检测诸如**释放后访问(use-after-free), 访问越界(out-of-bouds)**等非法访问问题. 它比后者更快, 因为它利用了编译器的 **Instrument** 功能, 也即编译器会在访问内存前插入探针, 执行用户指定的操作, 这通常用在性能剖析中. 在内存的使用上, KASan 也比 KMEMCHECK 有优势: 相比后者1:1的内存使用 , 它只要1:1/8.


总的来说, 它利用了 GCC 5.0的新特性, 可对内核内存进行 Instrumentaion, 编译器可以在访问内存前插入指令, 从而检测该次访问是否合法. 对比前述的 KMEMCHECK 要用到 CPU 的陷阱指令处理和单步调试功能, KASan 是在编译时加入了探针, 因此它的性能更快.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/08/04 | Andrey Konovalov <andreyknvl@google.com> | [kasan: support stack instrumentation for tag-based mode](https://lore.kernel.org/patchwork/patch/1284062) | NA | v11 ☑ 5.9-rc1 | [PatchWork v2,0/5](https://lore.kernel.org/patchwork/patch/1284062), [Clang](https://clang.llvm.org/docs/HardwareAssistedAddressSanitizerDesign.html) |
| 2022/03/23 | andrey.konovalov@linux.dev <andrey.konovalov@linux.dev> | [kasan, arm64, scs, stacktrace: collect stack traces from Shadow Call Stack](https://patchwork.kernel.org/project/linux-mm/cover/cover.1648049113.git.andreyknvl@google.com) | 目前, 当保存 alloc 和 free 堆栈跟踪时, kasan 总是使用正常的堆栈跟踪收集例程, 它依赖于 unwind. 通过复制帧从阴影调用堆栈收集堆栈跟踪, 每当它被启用. 这减少了 30% 的启动时间, 所有的 KASAN 模式时, 影子呼叫堆栈被启用. 堆栈栈位通过新的 stack_trace_save_shadow () 接口从 Shadow Call Stack 中收集. | v2 ☐☑ | [LORE v2,0/4](https://lore.kernel.org/r/cover.1648049113.git.andreyknvl@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/3](https://lore.kernel.org/r/cover.1649877511.git.andreyknvl@google.com) |


#### 13.3.4.2 Software tag-based KASAN
-------

基于软件 tag 的 KASAN 使用软件内存标记方法来检查访问的内存是否有效, 目前仅在 arm64 架构上实现. 基于软件 tag 的 KASAN 使用 arm64 CPU 的 Top Byte Ignore(TBI) 功能将指针 tag 存储在指针的顶部字节中. 它使用影子内存来存储与每个 16 字节内存单元关联的内存 tag(因此, 它将内核内存的 1/16 专用于影子内存).

1.  在每次内存分配时, 生成一个随机 tag, 用这个 tag 标记分配的内存, 并将相同的 tag 嵌入到返回的指针中.

2.  编译时, 在每次内存访问之前插入检测指令, 这些检测指令会比较正在访问的内存的 tag(影子区)和用于访问该内存的指针的 tag 是否匹配.

3.  如果不匹配, 基于软件 tag 的 KASAN 会打印错误报告.

Software tag-based KASAN 使用 0xFF 作为匹配所有指针 tag(不检查通过带有 0xFF 指针 tag 的指针进行的访问). 0xFE 用于标记释放的内存区域.

目前 Software tag-based KASAN 只支持 slab 和 page_alloc 内存检测.

简单总结下 Software tag-based KASAN 的特点:

1.  编译时插入用于检测内存访问指令.
2.  专用的影子内存大小为检测内存大小的 1/16.
3.  通过指令对比内存 tag (影子区域) 和指针 tag.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2018/12/06 | Andrey Konovalov <andreyknvl@google.com> | [kasan: add software tag-based mode for arm64](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e886bf9d9abedf8236464bfd21bc5707748b4a02) | 该补丁集为 [KASAN](https://www.kernel.org/doc/html/latest/dev-tools/kasan.html) 添加了一种新的基于软件标签的模式. 最初这种模式被称为 KHWASAN, 但后来被重新命名. 通过使用 arm64 CPU 的 Top Byte Ignore(TBI) 特性, 可以在每个内核指针的顶端字节中存储指针标记. 将现有的 KASAN 模式被重命名为 generic KASAN(generic 意味着它是纯软件实现, 可以在任何架构上轻易支持). | v13 ☑✓ 5.0-rc1 | [LORE v13,0/25](https://lore.kernel.org/all/cover.1544099024.git.andreyknvl@google.com) |


#### 13.3.4.2 Hardware tag-based KASAN
-------

ARM 引入了一个[内存标签扩展](https://community.arm.com/developer/ip-products/processors/b/processors-ip-blog/posts/enhancing-memory-safety)的硬件特性. 然后 Google 的 [Andrey Konovalov](https://github.com/xairy/linux)基于此硬件特性重写了 KASan. 实现了 [hardware tag-based mode 的 KASan](https://lore.kernel.org/patchwork/patch/1344197). 参见 [The Arm64 memory tagging extension in Linux](https://lwn.net/Articles/834289), 译文 [LWN：Arm64 的内存标记扩展功能!](https://blog.csdn.net/Linux_Everything/article/details/109396397).

基于硬件 tag 的 KASAN 和基于软件 tag 的 KASAN 原理非常类似, Software tag-based KASAN 的内存的 tag(影子区)和用于访问该内存的指针的 tag 的比较是由插入的汇编指令完成的, 而 Hardware tag-based KASAN 是由硬件自动完成比较的, 对于软件是透明的. Hardware tag-based KASAN 目前也是仅支持 arm64 架构, 并且它的实现是基于 ARMv8.5 指令集架构中引入的内存标记扩展 (MTE) 和 Top Byte Ignore(TBI).

在每次内存访问时, 硬件都会比较正在访问的内存的 tag 和用于访问该内存指针的 tag. 如果 tag 不匹配, 则会触发一个异常.

基于硬件 tag 的 KASAN 使用 0xFF 作为匹配所有指针 tag(不检查通过带有 0xFF 指针 tag 的指针进行的访问). 0xFE 用于标记释放的内存区域. 同和 Software tag-based KASAN 一样, 当前只支持 slab 和 page_alloc 检测. 这些特性. 如果硬件不支持 MTE(ARMv8.5 之前), 则不会启用 Hardware tag-based KASAN.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/11/23 | Andrey Konovalov <andreyknvl@google.com> | [kasan: add hardware tag-based mode for arm64](https://lore.kernel.org/patchwork/patch/1344197) | NA | v11 ☑ [5.11-rc1](https://kernelnewbies.org/Linux_5.11#Memory_management) | [PatchWork mm,v11,00/42](https://patchwork.kernel.org/project/linux-mm/cover/cover.1606161801.git.andreyknvl@google.com) |
| 2020/11/23 | Andrey Konovalov <andreyknvl@google.com> | [kasan: boot parameters for hardware tag-based mode](https://lore.kernel.org/patchwork/patch/1344258) | NA | v4 ☑ [5.11-rc1](https://kernelnewbies.org/Linux_5.11#Memory_management) | [PatchWork mm,v4,00/19](https://patchwork.kernel.org/project/linux-mm/patch/748daf013e17d925b0fe00c1c3b5dce726dd2430.1606162397.git.andreyknvl@google.com) |
| 2021/01/15 | Andrey Konovalov <andreyknvl@google.com> | [kasan: HW_TAGS tests support and fixes](https://lore.kernel.org/patchwork/patch/1366086) | NA | v4 ☑ [5.12-rc1](https://kernelnewbies.org/Linux_5.11#Memory_management) | [PatchWork mm,v4,00/15](https://patchwork.kernel.org/project/linux-mm/cover/cover.1610733117.git.andreyknvl@google.com) |
| 2021/02/05 | Andrey Konovalov <andreyknvl@google.com> | [kasan: optimizations and fixes for HW_TAGS](https://lore.kernel.org/patchwork/patch/1376340) | NA | v3 ☑ [5.12-rc1](https://kernelnewbies.org/Linux_5.11#Memory_management) | [PatchWork mm,v3,mm,00/13](https://patchwork.kernel.org/project/linux-mm/cover/cover.1612546384.git.andreyknvl@google.com) |
| 2021/12/30 | andrey.konovalov@linux.dev | [kasan, vmalloc, arm64: add vmalloc tagging support for SW/HW_TAGS](https://patchwork.kernel.org/project/linux-mm/cover/cover.1638308023.git.andreyknvl@google.com/) | NA | v1 ☐ | [2021/11/30 PatchWork 00/31](https://patchwork.kernel.org/project/linux-mm/cover/cover.1638308023.git.andreyknvl@google.com)<br>*-*-*-*-*-*-*-* <br>[2021/12/13 PatchWork v3,00/38](https://patchwork.kernel.org/project/linux-mm/cover/cover.1639432170.git.andreyknvl@google.com)<br>*-*-*-*-*-*-*-* <br>[2021/12/20 PatchWork v4,00/39](https://patchwork.kernel.org/project/linux-mm/cover/cover.1640036051.git.andreyknvl@google.com))<br>*-*-*-*-*-*-*-* <br>[2021/12/30 PatchWork v5,00/39](https://patchwork.kernel.org/project/linux-mm/cover/cover.1640891329.git.andreyknvl@google.com) |
| 2022/02/02 | Christophe Leroy <christophe.leroy@csgroup.eu> | [mm/kasan: Add CONFIG_KASAN_SOFTWARE](https://patchwork.kernel.org/project/linux-mm/patch/a480ac6f31eece520564afd0230c277c78169aa5.1643791473.git.christophe.leroy@csgroup.eu/) | 610616 | v1 ☐ | [PatchWork v1,0/4](https://lore.kernel.org/r/a480ac6f31eece520564afd0230c277c78169aa5.1643791473.git.christophe.leroy@csgroup.eu) |
| 2022/06/13 | andrey.konovalov@linux.dev <andrey.konovalov@linux.dev> | [kasan: switch tag-based modes to stack ring from per-object metadata](https://patchwork.kernel.org/project/linux-mm/cover/cover.1655150842.git.andreyknvl@google.com/) | 649958 | v1 ☐☑ | [LORE v1,0/32](https://lore.kernel.org/r/cover.1655150842.git.andreyknvl@google.com) |

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/07/23 | Andrey Konovalov <andreyknvl@google.com> | [arm64: untag user pointers passed to the kernel](https://git.kernel.org/pub/scm/linux/kernel/git/tip/tip.git/log/?id=9ce1263033cd2ad393e2ff0df4a1c4ab4992c9df) | 扩展内核 ABI 以允许将标记的用户指针(顶部字节设置为 0x00 以外的其他值)作为系统调用参数传递的系列的一部分. 通过 CONFIG_TAGGED_ADDR_ABI 选项控制. | v19 ☐☑✓ | [LORE v19,0/15](https://lore.kernel.org/all/cover.1563904656.git.andreyknvl@google.com) |


### 13.3.5 KCSAN
-------


[KernelDOC: The Kernel Concurrency Sanitizer (KCSAN)](https://www.kernel.org/doc/html/latest/dev-tools/kcsan.html)

开源书籍 [PerfBook](https://mirrors.edge.kernel.org/pub/linux/kernel/people/paulmck/perfbook/perfbook.html), 由前 IBM 开发者, RCU 的作者和 Maintainer Paul 撰写. 国内的书籍 <<深入理解并行编程>> 就是其翻译版本.

[An introduction to lockless algorithms](https://lwn.net/Articles/844224)

[Detecting missing memory barriers with KCSAN](https://lwn.net/Articles/877200)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/30 | Andrey Konovalov <andreyknvl@google.com> | [kcsan: Support detecting a subset of missing memory barriers](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=bd3d5bd1a0ad386475ea7a3de8a91e7d8a600536) | KCSAN 增加对 LKMM 定义的弱内存子集建模的支持, 它支持检测由于丢失内存障碍而导致的数据竞争子集.<br>当内存操作的结果应该由 barrier 来排序时, KCSAN 可以检测数据竞争, 在这种情况下, 冲突只发生在由于重新排序访问而丢失 barrier 的情况下.<br>KCSAN 检测内存障碍缺失的方法是基于对访问重新排序的建模, 设置了观察点检测对每个内存访问, 也选择在其功能范围内进行模拟重排序. 由于运行时不能"预取"访问, 我们只能对延迟访问效果进行建模, 一旦选择了某个访问进行重新排序, 就会在每次其他访问中检查它, 直到函数范围结束. 如果遇到适当的内存障碍, 访问将不再考虑重新排序. | v3 ☑ 5.17-rc1 | [2021/10/05 PatchWork 00/23](https://patchwork.kernel.org/project/linux-mm/cover/20211005105905.1994700-1-elver@google.com)<br>*-*-*-*-*-*-*-* <br>[2021/11/18 PatchWork v2,00/23](https://patchwork.kernel.org/project/linux-mm/cover/20211118081027.3175699-1-elver@google.com)<br>*-*-*-*-*-*-*-* <br>[2021/11/30 PatchWork v3,00/25](https://patchwork.kernel.org/project/linux-mm/cover/20211130114433.2580590-1-elver@google.com)<br>*-*-*-*-*-*-*-* <br>[2021/12/24 LORE kcsan for 5.17, 00/29](https://lore.kernel.org/all/20211214220356.GA2236323@paulmck-ThinkPad-P17-Gen-1) |

### 13.3.6 KMSAN
-------


KernelMemorySanitizer (KMSAN) 是一个检测与未初始化内存使用相关的错误的检测器. 它依赖于 Clang 工具实现, 类似于用户空间 [MemorySanitizer](https://clang.llvm.org/docs/MemorySanitizer.html), 并跟踪内核内存的每一位状态, 如果在条件(condition)、解引用(dereferenced)或转译(escapes)到用户空间 或者 DMA时使用了未初始化的值, 则能够报告错误.

KMSAN 最早的 github 仓库位于 : https://github.com/google/kmsan

v3 的报道 [KMSAN Patches For The Linux Kernel Updated For Catching Uninitialized Memory Problems](https://www.phoronix.com/scan.php?page=news_item&px=KernelMemorySanitizer-2022).

v4 的报道 [KernelMemorySanitizer v4 Published While Already Having Found 300+ Kernel Bugs](https://www.phoronix.com/scan.php?page=news_item&px=KernelMemorySanitizer-v4).

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/12/14 | Alexander Potapenko <glider@google.com> | [Add KernelMemorySanitizer infrastructure](https://patchwork.kernel.org/project/linux-mm/cover/20211214162050.660953-1-glider@google.com) | 该补丁集包含 KMSAN 运行时实现, 以及使 KMSAN 工作所需的对其他子系统的修改.<br>1. 添加KMSAN所需的现有代码的更改和重构.<br>2. 通用代码中的 KMSAN 相关声明、KMSAN 运行时库.<br>3. 添加来自不同子系统的钩子, 以通知 KMSAN 有关内存的信息.<br>4. 通过显式初始化内存、禁用可能欺骗KMSAN的优化代码、有选择地跳过检测来防止错误报告的更改.<br>5. Noinstr 的处理.<br>这个补丁集允许在 QEMU 上启动并运行 defconfig + KMSAN 内核, 而不会出现已知的误报. 然而, 它不能保证在某些设备或测试较少的子系统的驱动程序中没有误报, 尽管 KMSAN 在 syzbot 上使用较大的配置进行了积极的测试. | v1 ☐ | [PatchWork 00/43](https://patchwork.kernel.org/project/linux-mm/cover/20211214162050.660953-1-glider@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,00/46](https://lore.kernel.org/lkml/20220426164315.625149-1-glider@google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,00/45](https://lore.kernel.org/all/20220701142310.2188015-1-glider@google.com) |






补丁集是相对于Linux v5.16-rc5生成的.

### 13.3.7 KFENCE 一个新的内存安全检测工具
-------

5.12 引入一个[新的内存错误检测工具 : KFENCE (Kernel Electric-Fence, 内核电子栅栏)](https://mp.weixin.qq.com/s/62Oht7WRnKPUBdOJfQ9cBg), 是一个低开销的基于采样的内存安全错误检测器, 用于检测堆后释放、无效释放和越界访问错误. 该系列使 KFENCE 适用于 x86 和 arm64 体系结构, 并将 KFENCE 钩子添加到 SLAB 和 SLUB 分配器.

KFENCE 被设计为在生产内核中启用, 它的性能开销几乎为零. 与 KASAN 相比, KFENCE 以性能换取精度. KFENCE 设计背后的主要动机是, 如果总正常运行时间足够长, KFENCE 将检测非生产测试工作负载通常不会执行的代码路径中的 BUG.

KFENCE 的灵感来自于 [GWP-ASan](http://llvm.org/docs/GwpAsan.html), 这是一个具有类似属性的用户空间工具. "KFENCE" 这个名字是对 [Electric Fence Malloc Debugger](https://linux.die.net/man/3/efence) 的致敬.

[利器解读！Linux 内核调测中最最让开发者头疼的 bug 有解了｜龙蜥技术](https://openanolis.cn/blog/detail/553167069912907824)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/11/03 | Marco Elver <elver@google.com> | [KFENCE: A low-overhead sampling-based memory safety error detector](https://lore.kernel.org/patchwork/patch/1331483) | 轻量级基于采样的内存安全错误检测器 | v7 ☑ 5.12-rc1 | [PatchWork v24](https://lore.kernel.org/patchwork/patch/1331483) |
| 2020/04/21 | Marco Elver <elver@google.com> | [kfence: optimize timer scheduling](https://lore.kernel.org/patchwork/patch/1416384) | ARM 支持 PTDUMP | RFC ☑ 3.19-rc1 | [PatchWork v24](https://lore.kernel.org/patchwork/patch/1416384) |
| 2020/09/17 | Marco Elver <elver@google.com> | [kfence: count unexpectedly skipped allocations](https://patchwork.kernel.org/project/linux-mm/patch/20210917110756.1121272-1-elver@google.com) | ARM 支持 PTDUMP | RFC ☑ 3.19-rc1 | [PatchWork 1/3](https://patchwork.kernel.org/project/linux-mm/patch/20210917110756.1121272-1-elver@google.com) |
| 2022/03/07 | Tianchen Ding <dtcccc@linux.alibaba.com> | [provide the flexibility to enable KFENCE](https://patchwork.kernel.org/project/linux-mm/cover/20220307074516.6920-1-dtcccc@linux.alibaba.com/) | 620847 | v3 ☐☑ | [LORE v3,0/2](https://lore.kernel.org/r/20220307074516.6920-1-dtcccc@linux.alibaba.com) |
| 2022/03/08 | Marco Elver <elver@google.com> | [kfence: allow use of a deferrable timer](https://patchwork.kernel.org/project/linux-mm/patch/20220308141415.3168078-1-elver@google.com/) | 采样间隔控制设置 KFENCE 分配的计时器. 默认情况下, 为了保持真实采样间隔的可预测性, 正常计时器会在系统完全空闲时导致 CPU 唤醒. 这在功率受限的系统上可能是不可取的. 因此这个补丁允许 KFENCE 使用可延迟 timer, 当系统空闲时, 它不会强制 CPU 唤醒. 引入一个选项 CONFIG_KFENCE_DEFERRABLE 来开启, 同时提供启动参数 ``kfence. deferrable=1`` 来切换到一个 "deferrable timer" 计时器, 该计时器不会强制在空闲系统上唤醒 CPU, 从而冒着采样间隔不可预测的风险. 使用延迟 timer 是样本间隔变得非常不可预测, 以至于无法保证 KFENCE-KUnit 测试仍然通过. 然而, 在功率受限的系统上, 这可能更可取, 因此, 如果用户接受上述权衡. | v2 ☐☑ | [LORE v2,0/1](https://lore.kernel.org/r/20220308141415.3168078-1-elver@google.com) |
| 2022/06/09 | Jason A. Donenfeld <Jason@zx2c4.com> | [mm/kfence: select random number before taking raw lock](https://patchwork.kernel.org/project/linux-mm/patch/20220609121709.12939-1-Jason@zx2c4.com/) | 648846 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220609121709.12939-1-Jason@zx2c4.com)<br>*-*-*-*-*-*-*-*<br>[LORE v2,0/1](https://lore.kernel.org/r/20220609123319.17576-1-Jason@zx2c4.com) |
| 2022/06/28 | Yee Lee <yee.lee@mediatek.com> | [[v2,1/1] mm: kfence: apply kmemleak_ignore_phys on early allocated pool](https://patchwork.kernel.org/project/linux-mm/patch/20220628113714.7792-2-yee.lee@mediatek.com/) | 654579 | v2 ☐☑ | [LORE v2,0/1](https://lore.kernel.org/r/20220628113714.7792-2-yee.lee@mediatek.com) |


## 13.4 Debugging
-------

### 13.4.1 ptdump
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2014/11/26 | Russell King - ARM Linux | [ARM: add support to dump the kernel page tables](https://patchwork.kernel.org/project/linux-arm-kernel/patch/20131024071600.GC16735@n2100.arm.linux.org.uk) | ARM 支持 PTDUMP | RFC ☑ 3.19-rc1 | [PatchWork](https://patchwork.kernel.org/project/linux-arm-kernel/patch/20131024071600.GC16735@n2100.arm.linux.org.uk), [LWN](https://lwn.net/Articles/572320) |
| 2014/11/26 | Laura Abbott | [arm64: add support to dump the kernel page tables](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1416961719-6644-1-git-send-email-lauraa@codeaurora.org) | ARM64 支持 PTDUMP | RFC ☑ 3.19-rc1 | [PatchWork](https://patchwork.kernel.org/project/linux-arm-kernel/patch/1416961719-6644-1-git-send-email-lauraa@codeaurora.org) |
| 2019/12/19 | Colin Cross | [Generic page walk and ptdump](https://lore.kernel.org/patchwork/patch/1169746) | 重构了 ARM64/X86_64 的 PTDUMP, 实现了通用的 PTDUMP 框架. 目前许多体系结构都有一个 debugfs 文件用于转储内核页表. 目前, 每个体系结构都必须为此实现自定义函数, 因为内核使用的页表的遍历细节在不同的体系结构之间是不同的. 本系列扩展了walk_page_range()的功能, 使其能够处理内核的页表(内核没有vma, 可以包含比用户空间现有页面更大的页面). 通用的 PTDUMP 实现是利用walk_page_range()的新功能实现的, 最终arm64和x86将转而使用它, 删除了自定义的表行器. | v17 ☑ 5.6-rc1 |[PatchWork RFC](https://lore.kernel.org/patchwork/patch/1169746) |
| 2020/02/10 | Zong Li | [RISC-V page table dumper](https://patchwork.kernel.org/project/linux-arm-kernel/patch/20131024071600.GC16735@n2100.arm.linux.org.uk) | ARM 支持 PTDUMP | RFC ☑ 3.19-rc1 | [LKML](https://lkml.org/lkml/2020/2/10/100) |
| 2021/08/02 | Gavin Shan <gshan@redhat.com> | [mm/debug_vm_pgtable: Enhancements](https://patchwork.kernel.org/project/linux-mm/cover/20210802060419.1360913-1-gshan@redhat.com) | 目前的实现存在一些问题, 本系列试图解决这些问题:<br>1. 所有需要的信息分散在变量中, 传递给各种测试函数. 代码以非常轻松的方式组织.<br>2. 在页面表条目修改测试期间, 页面没有从 buddy 分配. 该页面可能是无效的, 与 ARM64 上的 set_xxx_at() 的实现冲突. 访问目标页面, 以便在 ARM64 上授予执行权限时刷新 iCache.<br>3. 此外, 目标页面可以被取消映射, 访问它会导致内核崩溃.<br>引入"struct pgtable_debug_args"来解决问题(1).<br>对于问题(2), 使用的页面是从页表条目修改测试中的 buddy 分配的. 如果我们没有分配(巨大的)页面, 则跳过. 对于其他测试用例, 仍然使用到内核符号(@start_kernel)的原始页面. | RFC ☐ 5.14-rc4 | [PatchWork v5,00/12](https://patchwork.kernel.org/project/linux-mm/cover/20210802060419.1360913-1-gshan@redhat.com) |


### 13.4.2 Page Owner
-------

早在 2.6.11 的时代就通过 [Page owner tracking leak detector](https://lwn.net/Articles/121271) 给大家展示了页面所有者跟踪 [Page Owner](https://lwn.net/Articles/121656). 但是它一直停留在 Andrew 的源代码树上, 然而, 没有人试图 upstream 到 mainline, 虽然已经有不少公司使用这个特性来调试内存泄漏或寻找内存占用者. 最终在 v3.19, Joonsoo Kim 在一个重构中, 将这个特性推到主线.

这个功能帮助我们知道谁分配了页面. 当分配一个页面时, 我们将一些关于分配的信息存储在额外的内存中. 之后, 如果我们需要知道所有页面的状态, 我们可以从这些存储的信息中获取并分析它.

虽然我们已经有了跟踪页分配/空闲的跟踪点, 使用它来分析页面所有者是相当复杂的. 我们需要扩大跟踪缓冲区, 以防止重叠, 直到用户空间程序启动. 而且, 启动的程序不断地将跟踪缓冲区转储出来供以后分析, 它将更有可能改变系统行为, 而不是仅仅将其保存在内存中, 因此不利于调试.

此外, 我们还可以将 page_owner 特性用于各种目的. 例如, 我们可以借助 page_owner 实现的碎片统计以及一些 CMA 故障调试特性.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2013/11/24 | Joonsoo Kim <iamjoonsoo.kim@lge.com> | [Resurrect and use struct page extension for some debugging features](https://lore.kernel.org/patchwork/patch/520462) | 1. 引入 page_ext 作为 page 的扩展来存储调试相关的变量<br>2. upstream page_owner | v7 ☑ 3.19-rc1 | [PatchWork v3](https://lore.kernel.org/patchwork/patch/520462), [Kernel Newbies](https://kernelnewbies.org/Linux_3.19#Memory_management) |
| 2020/12/10 | Liam Mark <lmark@codeaurora.org><br>Georgi Djakov <georgi.djakov@linaro.org> | [mm/page_owner: Record timestamp and pid](https://lore.kernel.org/patchwork/patch/1352029) | 收集页面所有者中记录的每个分配的时间和 PID, 以便可以测量分配"激增"及其来源. | v3 ☑ 5.11-rc1 | [PatchWork v3](https://lore.kernel.org/patchwork/patch/1352029), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9cc7e96aa846f9086431d6c2d33ff9ab42d72b2d) |
| 2021/02/03 | Georgi Djakov <georgi.djakov@linaro.org> | [mm/page_owner: Record the timestamp of all pages during free](https://lore.kernel.org/patchwork/patch/1375264) | 收集每个分配被释放的时间, 以帮助kdump/ramdump进行内存分析. 在page_owner debugfs文件中添加时间戳, 并将其打印到 dump_page() 中. 当我们释放页面时, 拥有另一个时间戳有助于调试页面迁移问题. 例如, 如果alloc时间戳和free时间戳相同, 则可能提示迁移内存存在问题, 而不是迁移过程中刚刚删除的页面. | v2 ☑ 5.13-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/1375264), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=866b485262173a2b873386162b2ddcfbcb542b4a) |
| 2021/04/02 | Sergei Trofimovich <slyfox@gentoo.org> | [mm: page_owner: detect page_owner recursion via task_struct](https://lore.kernel.org/patchwork/patch/1406950) | 通过记录在'struct task_struct'中使用 page_owner 递归标志来防止递归. 在此之前防止递归是通过获取 backtrace 并检查当前的指令指针来检测 page_owner 递归, 这种方式效率是比较低的. 因为它需要额外的 backtrace, 以及对结果的线性堆栈扫描. | v1 ☑ 5.13-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/patch/1406950), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8e9b16c47680f6e7d6e5864a37f313f905a91cf5) |
| 2021/12/28 | Minchan Kim <minchan@kernel.org> | [mm: introduce page pin owner](https://patchwork.kernel.org/project/linux-mm/patch/20211228175904.3739751-1-minchan@kernel.org) | NA | RFC,v2 ☐ | [PatchWork RFC,v2](https://patchwork.kernel.org/project/linux-mm/patch/20211228175904.3739751-1-minchan@kernel.org) |
| 2022/02/08 | Waiman Long <longman@redhat.com> | [mm/page_owner: Extend page_owner to show memcg information](https://patchwork.kernel.org/project/linux-mm/cover/20220128195642.416743-1-longman@redhat.com) | 609625 | v1 ☐ | [PatchWork v1,0/2](https://lore.kernel.org/r/20220128195642.416743-1-longman@redhat.com)<br>*-*-*-*-*-*-*-* <br>[PatchWorkv2,0/3](https://lore.kernel.org/r/20220129205315.478628-1-longman@redhat.com)<br>*-*-*-*-*-*-*-* <br>[PatchWorkv3,0/4](https://lore.kernel.org/r/20220131192308.608837-1-longman@redhat.com)<br>*-*-*-*-*-*-*-* <br>[PatchWorkv4,0/4](https://lore.kernel.org/r/20220202203036.744010-1-longman@redhat.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v5,0/4](https://lore.kernel.org/r/20220208000532.1054311-1-longman@redhat.com) |
| 2022/02/19 | Yixuan Cao <caoyixuan2019@email.szu.edu.cn> | [mm/page_owner.c: record tgid](https://patchwork.kernel.org/project/linux-mm/patch/20220219180450.2399-1-caoyixuan2019@email.szu.edu.cn/) | 615993 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220219180450.2399-1-caoyixuan2019@email.szu.edu.cn) |
| 2022/03/22 | Yinan Zhang <zhangyinan2019@email.szu.edu.cn> | [[1/2] mm/page_owner.c: introduce vmalloc allocator for page_owner](https://patchwork.kernel.org/project/linux-mm/patch/20220322032225.1402992-1-zhangyinan2019@email.szu.edu.cn/) | 625342 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20220322032225.1402992-1-zhangyinan2019@email.szu.edu.cn) |


### 13.4.3 PROC_PAGE_MONITOR
-------

[深入理解Linux内存回收 - Round One](https://zhuanlan.zhihu.com/p/320688306)

| 文件 | 描述 |
|:---:|:----:|
| `/proc/pid/smaps` | 使用 `/proc/pid/maps` 可以高效的确定映射的内存区域、跳过未映射的区域. |
| `/proc/kpagecount` | 这个文件包含64位计数 ,  表示每一页被映射的次数, 按照PFN值固定索引. |
| [`/proc/kpageflags`](https://www.kernel.org/doc/html/latest/admin-guide/mm/pagemap.html?highlight=kpageflags) | 此文件包含为64位的标志集, 表示该页的属性, 按照PFN索引. |

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/10/09 | Matt Mackall <mpm@selenic.com> | [maps4: pagemap monitoring v4](https://lore.kernel.org/patchwork/patch/95279) | 引入 CONFIG_PROC_PAGE_MONITOR, 管理了 `/proc/pid/clear_refs`, `/proc/pid/smaps`, `/proc/pid/pagemap`, `/proc/kpagecount`, `/proc/kpageflags` 多个接口. | v1 ☑ 2.6.25-rc1 | [PatchWork v4 0/12](https://lore.kernel.org/patchwork/patch/95279) |
| 201=09/05/08 | Joonsoo Kim <iamjoonsoo.kim@lge.com> | [export more page flags in /proc/kpageflags (take 6)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=35efa5e993a7a00a50b87d2b7725c3eafc80b083) | 在 kpageflags 中导出了更多的 type. 同时新增了一个用户态工具 page-types 可以调试进程和内核的 page-types. 该工具后期[移到了 tools/vm 目录下](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c6dd897f3bfc54a44942d742d6dfa842e33d88e0) | v6 ☑ 3.4-rc1 | [PatchWork v3](https://lore.kernel.org/patchwork/patch/520462), [Kernel Newbies](https://kernelnewbies.org/Linux_3.19#Memory_management) |

### 13.4.4 数据访问监视器 DAMON
-------

[Software Visualizations to Analyze Memory Consumption: A Literature Review](https://dl.acm.org/doi/pdf/10.1145/3485134)

[linux data access monitor (DAMON)](https://blog.csdn.net/zqh1630/article/details/109954910)

[LWN: 用DAMON来优化memory-management!](https://blog.csdn.net/Linux_Everything/article/details/104707923)

[知乎-DAMON: Linux内存数据访问监控框架](https://zhuanlan.zhihu.com/p/446677951)

对指定的程序进行内存相关优化, 了解业务给定工作负载的数据访问模式至关重要. 但是, 从庞大和复杂的工作量中手动提取此类模式非常详尽. 更糟糕的是, 现有的内存访问分析工具会为不必要的详细分析结果带来不可接受的高开销.

内存管理在很大程度上基于预测 : 给定进程在不久的将来将需要哪些内存页?

不幸的是, 事实证明, 预测是困难的, 尤其是对于未来事件的预测. 在没有从未来发送回的有用信息的情况下, 内存管理子系统被迫依赖于对最近行为的观察, 并假设该行为可能会继续. 但是, 内核的内存管理决策对于用户空间是不透明的, 并且常常导致性能不佳. SeongJae Park 实现的 [DAMON](https://lwn.net/Articles/812707) 试图使内存使用模式对用户空间可见, 并让用户空间作为响应来更改内存管理决策.

亚马逊工程师公开了他们关于 ["DAMON"](https://github.com/awslabs/damo) 的工作, 这是一个Linux内核模块, 用于监控特定用户空间过程的数据访问.

[DAMON](https://damonitor.github.io/_index) 允许用户监控特定用户空间过程的实际内存访问模式. 从概念上讲, 它的操作很简单;

1.  首先, 将进程的地址空间划分为多个大小相等的区域.

2.  然后, 它监视对每个区域的访问, 并提供对每个区域的访问次数的直方图作为其输出.

由此, 该信息的使用者(在用户空间或内核中)可以请求更改以优化进程对内存的使用.

它的目标是 :

1.  足够准确, 可用于实际性能分析和优化;

2.  足够轻量, 以便它可以在线使用;

DAMON 利用两个核心机制 : **基于区域的采样**和**自适应区域调整**, 允许用户将跟踪开销限制在有界范围内, 而与目标工作负载的大小和复杂性无关, 同时保留结果的质量.

1.  基于区域的抽样允许用户在监控质量和开销之间做出自己的权衡, 并限制监控开销的上限.

2.  自适应区域调整机制使 DAMON 在保持用户配置的权衡的同时, 最大限度地提高精度, 减少开销.


[DAMON-Based Memory Reclamation Merged For Linux 5.16](https://www.phoronix.com/scan.php?page=news_item&px=DAMON-Reclamation-Linux-5.16)

作者 SeongJae Park
> github :https://github.com/sjp38/linux   /
> 后期切到了 git.kernel.org https://git.kernel.org/pub/scm/linux/kernel/git/sj/linux.git

主页 : [damonitor](https://github.com/damonitor)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/07/16 | SeongJae Park <sjpark@amazon.com>/<sj38.park@gmail.com> | [Introduce Data Access MONitor (DAMON)](https://damonitor.github.io) | 数据访问监视器 DAMON | v34 ☑ [5.15-rc1](https://kernelnewbies.org/LinuxChanges#Linux_5.15.DAMON.2C_a_data_access_monitor) | [PatchWork v24,00/14](https://lore.kernel.org/patchwork/patch/1375732), [LWN](https://lwn.net/Articles/1461471)<br>*-*-*-*-*-*-*-* <br>[PatchWork v34,00/13](https://patchwork.kernel.org/project/linux-mm/cover/20210716081449.22187-1-sj38.park@gmail.com) |
| 2021/10/19 | SeongJae Park <sjpark@amazon.com> | [Introduce DAMON-based Proactive Reclamation](https://lwn.net/Articles/863753) | 该补丁集改进了用于生产质量的通用数据访问模式内存管理的引擎, 并在其之上实现了主动回收. | v4 ☑ 5.16-rc1 | [PatchWork RFC,00/13](https://patchwork.kernel.org/project/linux-mm/cover/20210720131309.22073-1-sj38.park@gmail.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC,v2,00/14](https://patchwork.kernel.org/project/linux-mm/patch/20210608115254.11930-15-sj38.park@gmail.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC,v3,00/15](https://patchwork.kernel.org/project/linux-mm/cover/20210720131309.22073-1-sj38.park@gmail.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v4 00/15](https://patchwork.kernel.org/project/linux-mm/cover/20211019150731.16699-1-sj@kernel.org) |
| 2021/08/31 | SeongJae Park <sjpark@amazon.com>/<sj38.park@gmail.com>/<sjpark@amazon.de> | [mm/damon/vaddr: Safely walk page table](https://patchwork.kernel.org/project/linux-mm/patch/20210831161800.29419-1-sj38.park@gmail.com) | 提交 linux-mm 的 [3f49584b262c ("mm/damon: implement primitives for the virtual memory address spaces")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3f49584b262c)  尝试使用 follow_invalidate_PTE() 查找任意虚拟地址的 PTE 或 PMD, 但没有正确锁定. 此提交通过在正确的锁定(保持 mmap 读取锁定)下使用另一个页面表遍历函数来解决此问题, 并使用了更通用的 walk_page_range(). | v1 ☐ | [PatchWork v2](https://patchwork.kernel.org/project/linux-mm/patch/20210831161800.29419-1-sj38.park@gmail.com) |
| 2021/09/17 | SeongJae Park <sjpark@amazon.com> | [mm, mm/damon: Trivial fixes](https://patchwork.kernel.org/project/linux-mm/cover/20210917123958.3819-1-sj@kernel.org) | DAMON 的 fix 补丁. | v1 ☐ | [PatchWork 0/5](https://patchwork.kernel.org/project/linux-mm/cover/20210917123958.3819-1-sj@kernel.org) |
| 2021/10/01 | SeongJae Park <sjpark@amazon.com> | [Implement Data Access Monitoring-based Memory Operation Schemes](https://patchwork.kernel.org/project/linux-mm/cover/20211001125604.29660-1-sj@kernel.org) | 实现了 DAMON 的基于数据访问监视的操作方案 (DAMOS).<br>DAMON 可以用作支持数据访问的内存管理优化的原语. 因此, 希望进行此类优化的用户应该运行 DAMON, 读取监视结果, 分析并优化内存管理方案.<br>然而, 在许多其他情况下, 用户只是希望系统对具有特定时间的特定访问频率的特定大小的内存区域应用内存管理操作. 例如, "page out a memory region than 100mib only rare visits more than 2 minutes", 或者 "Do not use THP For a memory region than 2mib rarely visits more than 1 seconds". 为了使工作更容易且不冗余, 这个补丁集实现了 DAMON 的一个新特性, 称为基于数据访问监视的操作方案 (DAMOS). 使用该特性, 用户可以以简单的方式描述常规方案, 并要求 DAMON 自行执行这些方案.<br>DAMOS 对于内存管理优化是准确和有用的.<br>1. THP 的一个实验性的基于 damon 的操作方案 'ethp' 减少了 76.15% 的 THP 内存开销, 同时保留了 51.25% 的 THP 加速.<br>2. 另一个实验性的基于 damon 的 "主动回收" 实现 "prcl", 减少了 93.38% 的  residential sets 和 23.63% 的内存占用, 而在最好的情况下只产生 1.22% 的运行时开销 (parsec3/freqmine). | v1 ☑ 5.16-rc1 | [2020/12/16 PatchWork RFC,v15.1,0/8](https://patchwork.kernel.org/project/linux-mm/cover/20201216084404.23183-1-sjpark@amazon.com)<br>*-*-*-*-*-*-*-* <br>[2021/10/01 PatchWork 0/7](https://patchwork.kernel.org/project/linux-mm/cover/20211001125604.29660-1-sj@kernel.org) |
| 2021/10/08 | SeongJae Park <sjpark@amazon.com> | [mm/damon/dbgfs: Implement recording feature](https://patchwork.kernel.org/project/linux-mm/patch/20211008094509.16179-1-sj@kernel.org) | 为 'damon-dbgfs' 实现 'recording' 特性<br>用户空间可以通过 'damon_aggregate' 跟踪点事件获得监视结果. 为了简单起见, 跟踪点事件有一些重复的信息, 比如 'target_id' 和 'nr_regions'. 这造成它的大小比实际需要的要大. 另外, 对于一些简单的用例, 处理跟踪点可能会很复杂. 为了给用户空间提供一种更有效和简单的监控结果的方法, 这个提交实现了 'damon-dbgfs' 中的 'recording' 特性. 该特性通过一个名为 'record' 的新 debugfs 文件导出到用户空间, 该文件位于 '/damon/' 目录下. 该文件允许用户以简单格式在常规二进制文件中记录监视的访问模式. 记录的结果首先写入内存缓冲区并批处理刷新到文件中. 用户可以通过读取和写入 record 文件来获取和设置缓冲区的大小和结果文件的路径. | v1 ☐ | [PatchWork 1/4](https://patchwork.kernel.org/project/linux-mm/patch/20211008094509.16179-1-sj@kernel.org) |
| 2021/10/12 | SeongJae Park <sjpark@amazon.com> | [DAMON: Support Physical Memory Address Space Monitoring](https://www.phoronix.com/scan.php?page=news_item&px=DAMON-Physical-Monitoring) | 允许物理地址空间监控. | v1 ☑ 5.16-rc1 | [PatchWork RFC,v9,00/10](https://patchwork.kernel.org/project/linux-mm/cover/20201007071409.12174-1-sjpark@amazon.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC,v10,00/13](https://patchwork.kernel.org/project/linux-mm/cover/20201216094221.11898-1-sjpark@amazon.com))<br>*-*-*-*-*-*-*-* <br>[PatchWork 0/7](https://patchwork.kernel.org/project/linux-mm/cover/20211012205711.29216-1-sj@kernel.org)|
| 2021/10/12 | Xin Hao <xhao@linux.alibaba.com> | [mm/damon/dbgfs: add region_stat interface](https://patchwork.kernel.org/project/linux-mm/patch/20211012054948.90381-1-xhao@linux.alibaba.com) | DAMON 中使用 damon-dbgfs 操作带来了很大的遍历, 有时候如果我希望能够查看任务的划分区域 nr_access 等值, 当前这不能直接通过 dbgfs 接口查看, 所以添加一个接口 "region_stat" 来显示. | v1 ☐ | [PatchWork](https://patchwork.kernel.org/project/linux-mm/patch/20211012054948.90381-1-xhao@linux.alibaba.com/) |
| 2021/10/13 | Xin Hao <xhao@linux.alibaba.com> | [mm/damon: Adjust the size of kbuf array to avoid overflow](https://patchwork.kernel.org/project/linux-mm/patch/20211013114854.15705-1-xhao@linux.alibaba.com) | NA | v1 ☐ | [PatchWork](https://patchwork.kernel.org/project/linux-mm/patch/20211013114854.15705-1-xhao@linux.alibaba.com) |
| 2021/10/16 | Xin Hao <xhao@linux.alibaba.com> | [mm/damon/core: Optimize kdamod.%d thread creation code](https://patchwork.kernel.org/project/linux-mm/patch/20211016165914.96049-1-xhao@linux.alibaba.com) | 当 ctx->adaptive_targets 列表为空, 无需创建并调用kdamond. 只有当 ctx->adaptive_targets 列表不为空, 且 ctx->kdamond 指针为 NULL 时, 才调用__damon_start函数. | v1 ☐ | [PatchWork v1](https://patchwork.kernel.org/project/linux-mm/patch/20211016165616.95849-1-xhao@linux.alibaba.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://patchwork.kernel.org/project/linux-mm/patch/20211016165914.96049-1-xhao@linux.alibaba.com) |
| 2021/10/21 | Xin Hao <xhao@linux.alibaba.com> | [mm/damon/dbgfs: Optimize target_ids interface write operation](https://patchwork.kernel.org/project/linux-mm/patch/bc341f48b5558f6816dcef22eca4f4a590efdc67.1634834628.git.xhao@linux.alibaba.com) | NA | v2 ☐ | [PatchWork v1](https://patchwork.kernel.org/project/linux-mm/patch/20211021085611.81211-1-xhao@linux.alibaba.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://patchwork.kernel.org/project/linux-mm/patch/bc341f48b5558f6816dcef22eca4f4a590efdc67.1634834628.git.xhao@linux.alibaba.com) |
| 2021/10/27 | Changbin Du <changbin.du@gmail.com> | [mm/damon: simplify stop mechanism](https://patchwork.kernel.org/project/linux-mm/patch/20211027130517.4404-1-changbin.du@gmail.com) | NA | v2 ☐ | [PatchWork v2](https://patchwork.kernel.org/project/linux-mm/patch/20211027130517.4404-1-changbin.du@gmail.com) |
| 2021/12/21 | Changbin Du <changbin.du@gmail.com> | [Add a new scheme to support demotion on tiered memory system](https://patchwork.kernel.org/project/linux-mm/cover/cover.1640077468.git.baolin.wang@linux.alibaba.com) | 现在, 在具有不同内存类型的分级内存系统上, shrink_page_list() 中的回收路径已经支持将页面降级以减慢内存节点的速度, 而不是丢弃页面. 然而, 此时快速内存节点的内存水印已经紧张, 这将增加页面降级期间的内存分配延迟. 因此, 从用户空间主动降格冷页面的新方法将更有帮助. 我们可以依靠用户空间中的 DAMON 来帮助监控快速内存节点上的冷内存, 并主动降级冷页面来降低内存节点的速度, 以保持快速内存节点处于健康状态. 这个补丁集引入了一个名为 DAMOS_DEMOTE 的新模式来支持这个特性. | v2 ☐ | [PatchWork 0/2](https://patchwork.kernel.org/project/linux-mm/cover/cover.1640077468.git.baolin.wang@linux.alibaba.com) |
| 2022/02/04 | Jonghyeon Kim <tome01@ajou.ac.kr> | [mm/damon: Rebase DAMON_RECALIM watermarks for NUMA nodes](https://patchwork.kernel.org/project/linux-mm/patch/20220204064059.6244-1-tome01@ajou.ac.kr/) | 611199 | v1 ☐☑ | [PatchWork v1,0/1](https://lore.kernel.org/r/20220204064059.6244-1-tome01@ajou.ac.kr) |
| 2022/02/15 | SeongJae Park <sj@kernel.org> | [Allow DAMON user code independent of monitoring primitives](https://patchwork.kernel.org/project/linux-mm/cover/20220215184603.1479-1-sj@kernel.org/) | 614655 | v1 ☐☑ | [PatchWork v1,0/8](https://lore.kernel.org/r/20220215184603.1479-1-sj@kernel.org) |
| 2022/02/16 | Xin Hao <xhao@linux.alibaba.com> | [mm/damon: Add NUMA access statistics function support](https://patchwork.kernel.org/project/linux-mm/cover/cover.1645024354.git.xhao@linux.alibaba.com/) | 614856 | v1 ☐☑ | [LORE v1,0/5](https://lore.kernel.org/r/cover.1645024354.git.xhao@linux.alibaba.com) |
| 2022/02/17 | SeongJae Park <sj@kernel.org> | [Introduce DAMON sysfs interface](https://patchwork.kernel.org/project/linux-mm/cover/20220217161938.8874-1-sj@kernel.org/) | 615483 | v1 ☐☑ | [LORE v1,0/4](https://lore.kernel.org/r/20220217161938.8874-1-sj@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE, v1,00/12](https://patchwork.kernel.org/project/linux-mm/cover/20220223152051.22936-1-sj@kernel.org) |
| 2022/02/18 | Jonghyeon Kim <tome01@ajou.ac.kr> | [Rebase DAMON_RECALIM for NUMA system](https://patchwork.kernel.org/project/linux-mm/cover/20220218102611.31895-1-tome01@ajou.ac.kr/) | 615730 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/20220218102611.31895-1-tome01@ajou.ac.kr)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/13](https://lore.kernel.org/r/20220228081314.5770-1-sj@kernel.org) |
| 2022/03/15 | Xin Hao <xhao@linux.alibaba.com> | [mm/damon: Add CMA minotor support](https://patchwork.kernel.org/project/linux-mm/cover/cover.1647378112.git.xhao@linux.alibaba.com/) | 为 DAMON 增加 CMA 内存监控功能. 在某些内存紧张的情况下, 通过监控 CMA 内存释放更多内存将是一个不错的选择. | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/cover.1647378112.git.xhao@linux.alibaba.com) |
| 2021/01/07 | SeongJae Park <sjpark@amazon.com> | [tools/perf: Integrate DAMON in perf](https://lore.kernel.org/all/20210107120729.22328-1-sjpark@amazon.com) | perf 支持 DAMON 监控. | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20210107120729.22328-1-sjpark@amazon.com) |
| 2022/04/18 | Yuanchu Xie <yuanchu@google.com> | [[RESEND] selftests/damon: add damon to selftests root Makefile](https://patchwork.kernel.org/project/linux-mm/patch/20220418202017.3583638-1-yuanchu@google.com/) | 633136 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220418202017.3583638-1-yuanchu@google.com) |
| 2021/12/10 | SeongJae Park <sj@kernel.org> | [mm/damon/schemes: Extend stats for better online analysis and tuning](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=dbcb9b9f954f71fb46be34af624c9edaaa171414) | 20211210150016.35349-1-sj@kernel.org | v1 ☑✓ 5.17-rc1 | [LORE v1,0/6](https://lore.kernel.org/all/20211210150016.35349-1-sj@kernel.org) |
| 2022/01/14 | Baolin Wang <baolin.wang@linux.alibaba.com> | [mm/damon: add access checking for hugetlb pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=49f4203aae06ba9d67b500c90339b262b0a52637) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/6afcbd1fda5f9c7c24f320d26a98188c727ceec3.1639623751.git.baolin.wang@linux.alibaba.com), [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=49f4203aae06ba9d67b500c90339b262b0a52637) |
| 2022/04/29 | SeongJae Park <sj@kernel.org> | [mm/damon: Support online tuning](https://patchwork.kernel.org/project/linux-mm/cover/20220429160606.127307-1-sj@kernel.org/) | 637059 | v1 ☐☑ | [LORE v1,0/14](https://lore.kernel.org/r/20220429160606.127307-1-sj@kernel.org) |
| 2022/05/07 | Gautam Menghani <gautammenghani201@gmail.com> | [Add documentation for Enum value 'NR_DAMON_OPS' in](https://patchwork.kernel.org/project/linux-mm/patch/20220507165620.110706-1-gautammenghani201@gmail.com/) | 639422 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220507165620.110706-1-gautammenghani201@gmail.com) |
| 2022/05/13 | SeongJae Park <sj@kernel.org> | [mm/damon/schemes: Extend DAMOS for Proactive LRU-lists Sorting](https://patchwork.kernel.org/project/linux-mm/cover/20220513150000.25797-1-sj@kernel.org/) | 641448 | v1 ☐☑ | [LORE v1,0/3](https://lore.kernel.org/r/20220513150000.25797-1-sj@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/8](https://lore.kernel.org/r/20220613192301.8817-1-sj@kernel.org) |


### 13.4.5 vmstat
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/05 | Mel Gorman <mgorman@techsingularity.net> | [Protect vmstats on PREEMPT_RT](https://lore.kernel.org/patchwork/patch/1472709) | NA | v2 ☐ | [PatchWork 0/1,v2](https://patchwork.kernel.org/project/linux-mm/cover/20210723100034.13353-1-mgorman@techsingularity.net) |
| 2021/12/22 | Shakeel Butt <shakeelb@google.com> | [memcg: add per-memcg vmalloc stat](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4e5aa1f4c2b489bc6f3ab5ca54747b18a847289d) | NA | v1 ☑✓ 5.17-rc1 | [PatchWork v1](https://patchwork.kernel.org/project/linux-mm/patch/20211221215336.1922823-1-shakeelb@google.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://patchwork.kernel.org/project/linux-mm/patch/20211222052457.1960701-1-shakeelb@google.com) |


### 13.4.6 meminfo
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/12/16 | Qi Zheng <zhengqi.arch@bytedance.com> | [add MemAvailable to per-node meminfo](https://patchwork.kernel.org/project/linux-mm/cover/20211216124655.32247-1-zhengqi.arch@bytedance.com) | 在 `/proc/meminfo` 中, 展示了所有可用内存的总和显示为 "MemAvailable". 将相同的计数器也添加到 `/sys` 下的每个节点 `meminfo` 中. | v1 ☐ | [PatchWork 0/1,v2](https://patchwork.kernel.org/project/linux-mm/cover/20211216124655.32247-1-zhengqi.arch@bytedance.com/) |




### 13.4.7 DEBUG
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/02/03 | Christian Borntraeger <borntraeger@de.ibm.com> | [Optimize CONFIG_DEBUG_PAGEALLOC (x86 and s390)](https://damonitor.github.io) | 优化 CONFIG_DEBUG_PAGEALLOC, 提供了 debug_pagealloc_enabled(), 可以动态的开启 DEBUG_PAGEALLOC. | v4 ☑ 4.6-rc1 | [PatchWork v4,0/4](https://lore.kernel.org/patchwork/patch/642851) |


## 13.5 tracepoint
-------

内核提供的 MM 相关的 tracepoint, 介绍参见 Documentation/trace/events-kmem.rst, 使用和调试方法参见 Documentation/trace/tracepoint-analysis.rst.


### 13.5.1 内存分配路径 tracepoint
-------


| tracepoint | 提供版本 | 用途 | COMMIT |
|:----------:|:-------:|:---:|:------:|
| mm_page_alloc | v2.6.32 | NA | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4b4f278c030aa4b6ee0915f396e9a9478d92d610) |
| mm_page_alloc | v2.6.32 | NA | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4b4f278c030aa4b6ee0915f396e9a9478d92d610) |
| mm_page_free_direct/mm_page_free | v2.6.32,v3.3 | NA | [COMMIT1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4b4f278c030aa4b6ee0915f396e9a9478d92d610), [COMMIT2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=90a5d5af74f6570af063fb6bff33c6b2f8361bbc) |
| mm_pagevec_free/mm_page_free_batched | v2.6.32,v3.3 | NA | [COMMIT1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4b4f278c030aa4b6ee0915f396e9a9478d92d610), [COMMIT2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=90a5d5af74f6570af063fb6bff33c6b2f8361bbc) |
| mm_page_alloc_extfrag | v2.6.32 | 内核通过迁移类型分组来避免外碎片化. 在 `__rmqueue_smallest()` 无法获取指定 MIGRATETYPE 的情况下, 通过 `__rmqueue_fallback()` 来从其他 MIGRATETYPE 的空闲列表上窃取页面出来, 某些情况下甚至会更改整个 pageblock 的 MIGRATETYPE. 那么, 这个动作发生的次数越多, 碎片化就会越严重. TRACEPOINT mm_page_alloc_extfrag 用于跟踪 `__rmqueue_fallback()` 中页面用于跟踪窃取动作的发生以及其相关行为, 比如 order 的大小, 期望的 MIGRATETYPE 以及所使用的 MIGRATETYPE 列表, 更关键的是是否整个 pageblock 改变类型和这个事件是否对避免分裂与否是很重要的. 此信息可用于帮助分析碎片化, 并帮助决定是否应该增加 min_free_kbytes. |
| mm_page_alloc_zone_locked | v2.6.32 | 页面分配的 TRACEPOINT mm_page_alloc 只报告页面已成功分配, 但不指定它来自何处. 在分析性能时, 区分来自 PCP 还是 BUDDY 可能是很重要的, 因为后者需要持有 zone->lock, 并可能经历了页面窃取等诸多操作. 通过 TRACEPOINT mm_page_alloc_zone_locked 表明这个页面是从 BUDDY 中获取到的, 它经历了 LOCKED zone->lock. | [COMMIT1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0d3d062a6e289e065bd0aa537a6806a1806bf8aa),[COMMIT2](https://lkml.kernel.org/r/20201228132901.41523-1-carver4lio@163.com) |
| mm_page_pcpu_drain | v2.6.32 | 跟踪归还 PCP 的页面到伙伴系统的事件. | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0d3d062a6e289e065bd0aa537a6806a1806bf8aa) |

针对页面分配器相关的 TRACEPOINT 事件添加了一个简单的后处理脚本 trace-pagealloc-postprocess.pl. 它可以用来指示谁是分配程序最密集的进程, 以及在跟踪期间执行区域锁定的频率. 参见 [commit c9d05cfc001f ("tracing, page-allocator: add a postprocessing script for page-allocator-related ftrace events")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c9d05cfc001fef3d6d37651e19ab9227a32b71f5)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2009/08/10 | Mel Gorman <mel@csn.ul.ie> | [Add some trace events for the page allocator v6](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=8fbb398f5c78832ee61e0d5ed0793fa8857bd853) | NA | v6 ☑✓ 2.6.32-rc1 | [LORE v3,0/4](https://lore.kernel.org/all/1249409546-6343-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE v6,0/6](https://lore.kernel.org/all/1249918915-16061-1-git-send-email-mel@csn.ul.ie) |
| 2011/11/11 | Konstantin Khlebnikov <khlebnikov@openvz.org> | [mm-tracepoint: rename page-free events](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=90a5d5af74f6570af063fb6bff33c6b2f8361bbc) | 从 [v2.6.33-5426-gc475dab](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c475dab63ae798d81fb597a6a1859986b296d9d0)内核为所有被释放的页面都添加了 tracepoint mm_page_free_direct, 而不仅仅是直接被释放的页面. 此外对于通过 page-list 释放的页面, 我们也会触发 mm_page_free_batch 事件. 所以, 对 free 相关的 tracepoint 的命名进行修正. 将 mm_page_free_direct 重命名为 mm_page_free, mm_pagevec_free 重命名为 mm_page_free_batch. | v3 ☑✓ 3.3-rc1 | [LORE v3,0/4](https://lore.kernel.org/all/20111111123957.7371.72792.stgit@zurg) |
| 2020/12/28 | Hailong liu <carver4lio@163.com> | [mm/page_alloc:add a missing mm_page_alloc_zone_locked tracepoint](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ce8f86ee94fabcc98537ddccd7e82cfd360a4dc5) | 20201228132901.41523-1-carver4lio@163.com | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20201228132901.41523-1-carver4lio@163.com) |


### 13.5.2 LRU tracepoint
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/04/21 | Larry Woodman <lwoodman@redhat.com> | [mm tracepoints update](https://lwn.net/Articles/329577/) | 清理 mm 的 tracepoint, 以跟踪页面分配和释放、各种类型的页面错误和取消映射, 以及页面回收等路径. 这对在高内存压力下调试和分析内存分配问题和系统性能问题非常有用. | v1 ☐ | [LORE v1,0/9](https://lore.kernel.org/all/1283770053-18833-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/8](https://lore.kernel.org/lkml/1284553671-31574-1-git-send-email-mel@csn.ul.ie) |
| 2010/09/06 | Mel Gorman <mel@csn.ul.ie> | [tracing, vmscan: add trace events for LRU list shrinking](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e11da5b4fdf01d71d73c21cb92b00595b917d7fd) | [Reduce latencies and improve overall reclaim efficiency](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=08fc468f4eaf6683bae5bdb94743a09d8630cb80) 引入 lumpy_mode, 减少成块回收过程中的等待和延迟系列补丁集的其中一个补丁. 为了方便跟踪和计算扫描nr_scanned/回收nr_scanned比率, 作为页面回收所做工作量的度量. 在 shrink_inactive_list() 中添加了 tracepoint mm_vmscan_lru_shrink_inactive. | v2 ☑✓ 2.6.37-rc1 | [LORE v1,0/9](https://lore.kernel.org/all/1283770053-18833-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/8](https://lore.kernel.org/lkml/1284553671-31574-1-git-send-email-mel@csn.ul.ie) |
| 2010/06/14 | Mel Gorman <mel@csn.ul.ie> | [Avoid overflowing of stack during page reclaim V2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=1489fa14cb757b496c8fa2b63097dbcee6690695) | 这是两个补丁集的合并. 第一个减少了页面回收中的堆栈使用, 第二个在回收过程中写入连续页面, 并避免了直接回收器中的写回. 其中前几个补丁引入了一些 LRU 相关的 tracepoint, 可以用来评估在回收中发生了什么, 以及事情是变得更好还是更糟.<br>1. [commit1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=33906bc5c87b50028364405ec425de9638afc719) 添加了[直接回收](https://elixir.bootlin.com/linux/v2.6.36/source/mm/vmscan.c), [唤醒 KSWAPD](https://elixir.bootlin.com/linux/v2.6.36/source/mm/vmscan.c#L2403), 以及 [KSWAPD 睡眠](https://elixir.bootlin.com/linux/v2.6.36/source/mm/vmscan.c#L2380) 的 tracepoint.<br>2. [commit2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a8a94d151521b248727c1f88756174e15260815a) 添加了 [isolate_lru_pages() 的 tracepoint](https://elixir.bootlin.com/linux/v2.6.36/source/mm/vmscan.c#L1038).<br>3. [commit3](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=755f0225e8347b23a33ee6e3fb14a35310f95766) 中在 pageout 中添加了 [writepage 的 tracepoint](https://elixir.bootlin.com/linux/v2.6.36/source/mm/vmscan.c#L404).<br>4. [commit4](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b898cc70019ce1835bbf6c47bdf978adc36faa42) 为回收相关跟踪事件添加一个简单的[后处理脚本 trace-vmscan-postprocess.pl](https://elixir.bootlin.com/linux/v2.6.36/source/Documentation/trace/postprocess/trace-vmscan-postprocess.pl). 它可以用来指示 LRU 列表上有多少流量, 以及回收造成的延迟有多严重.<br>之后的 6 个 commit 通过将较大的分配移出主调用路径, 减少了页面回收的堆栈占用空间. 如果 KSWAPD 要直接回写页面, 那么这是为了给文件系统提供尽可能多的堆栈. [补丁 11](https://lore.kernel.org/all/1276514273-27693-12-git-send-email-mel@csn.ul.ie) 在找到脏页面时将它们放在一个临时列表中, 然后使用一个助手函数将它们全部写出来. [补丁 12](https://lore.kernel.org/all/1276514273-27693-13-git-send-email-mel@csn.ul.ie) 完全阻止直接回收写出来的页面, 取而代之的是脏页面被放回 LRU. 但是需要注意的是补丁 11/12 未合入主线. | v1 ☑✓ 2.6.36-rc1 | [LORE v1,00/10](https://lore.kernel.org/lkml/1271352103-2280-1-git-send-email-mel@csn.ul.ie)<br>*-*-*-*-*-*-*-* <br>[LORE v2,00/12](https://lore.kernel.org/all/1276514273-27693-1-git-send-email-mel@csn.ul.ie) |
| 2013/05/17 | Mel Gorman <mgorman@suse.de> | [mm: add tracepoints for LRU activation and insertions](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c6286c983900c77410a951874f1589f4a41fbbae) | [Obey mark_page_accessed hint given by filesystems v3r1](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=c53954a092d07c5684d31ea1fc813d262cff08a5) 补丁集的其中一个补丁. Alexey Lyahkov 和 Robin Dong 等最近(v3.10期间)报告了[诸多问题](https://www.spinics.net/lists/linux-ext4/msg37340.html), 这些问题可能是由于热页太快到达非活动列表的末尾并被回收造成的. 这个补丁集解决了这个问题. [当前补丁](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c6286c983900c77410a951874f1589f4a41fbbae)为 LRU 页面激活和插入添加了两个跟踪点 mm_lru_insertion 和 mm_lru_activate. 使用这些 tracepoint, 可以在 LRU 中构建一个可以脱机处理的页面模型, 用于离线检查 LRU 上不同页面类型的平均使用时间(ms). | v3 ☑✓ 3.11-rc1 | [LORE RFC,0/3](https://lore.kernel.org/lkml/1367253119-6461-1-git-send-email-mgorman@suse.de)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/4](https://lore.kernel.org/lkml/1368440482-27909-1-git-send-email-mgorman@suse.de)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/5](https://lore.kernel.org/all/1368784087-956-1-git-send-email-mgorman@suse.de) |
| 2017/01/04 | Michal Hocko <mhocko@kernel.org> | [vm, vmscan: enahance vmscan tracepoints](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=93607e5a554a6408a1b56cdd146edc91e5da9983) | 在调试问题 [OOM: Better, but still there on 4.9](http://lkml.kernel.org/r/20161215225702.GA27944@boerne.fritz.box) 时, 作者意识到当前提供的跟踪点集还有一些改进的空间. 新增了 tracepoint [mm_vmscan_lru_shrink_active](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9d998b4f1e39abd69441d29a1ef3250514479267) 和 [mm_vmscan_inactive_list_is_low](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=dcec0b60a8213aeb876823a15d834009fce3b36e). 原来 tracepoint 中[输出 LRU 的名字](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=32b3f2974adca13f8a4a610c396e88c6f81eb10e). 封装了 [reclaim_stat](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3c710c1ad11b4a856a396b181911568f3851a5d8), 并在 mm_vmscan_lru_shrink_inactive 中进行了输出. 同时将这些新增的改进同步到了 [后处理脚本 trace-vmscan-postprocess.pl](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=93607e5a554a6408a1b56cdd146edc91e5da9983) | v1 ☑✓ 4.11-rc1 | [LORE v1,0/7](https://lore.kernel.org/all/20161228153032.10821-1-mhocko@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/7](https://lore.kernel.org/all/20170104101942.4860-1-mhocko@kernel.org) |


### 13.5.3 其他 tracepoint
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/02/19 | Paul Gortmaker <paul.gortmaker@windriver.com> | [mmap_lock: add tracepoints around lock acquisition](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=abedf8e2419fb873d919dd74de2e84b510259339) | mmap lock tracepoint | v9 ☑ 4.6-rc1 | [LKML v8,0/5](https://lore.kernel.org/all/20201105211739.568279-1-axelrasmussen@google.com) |
| 2022/01/07 | Anshuman Khandual <anshuman.khandual@arm.com> | [mm/migration: Add trace events for THP migrations](https://patchwork.kernel.org/project/linux-mm/patch/1641531575-28524-1-git-send-email-anshuman.khandual@arm.com) | THP migrations tracepoint | v1 ☐ | [LORE v1](https://patchwork.kernel.org/project/linux-mm/patch/1641531575-28524-1-git-send-email-anshuman.khandual@arm.com) |


# 14 杂项
-------


这是最后一章, 讲几个 Linux 内存管理方面的属于锦上添花性质的功能, 它们使 Linux 成为一个更强大, 更好用的操作系统.



## 14.1 KSM - 内存去重
-------


**2.6.32(2009年12月发布)**

现代操作系统已经使用了不少共享内存的技术, 比如共享库, 创建新进程时子进程共享父进程地址空间. 而 KSM(Kernel SamePage Merging, 内存同页合并, 又称内存去重), 可以看作是存储领域去重(de-duplication)技术在内存使用上的延伸, 它是为了解决服务器虚拟化领域的内存去重方案. 想像在一个数据中心, 一台物理服务器上可能同时跑着多个虚拟客户机(Guest OS), 并且这些虚拟机运行着很多相同的程序, 如果在物理内存上, 这些程序文本(text)只有一份拷贝, 将会节省相当可观的内存. 而客户机间是相对独立的, 缺乏相互的认知, 所以 KSM 运作在监管机(hypervisor)上.



原理上简单地说, KSM 依赖一个内核线程, 定期地或可手动启动地, 扫描物理页面(通常稳定不修改的页面是合并的候选者, 比如包含执行程序的页面. 用户也可通过一个系统调用给予指导, 告知 KSM 进程的某部份区间适合合并, 见 [Kernel Samepage Merging](https://www.kernel.org/doc/html/latest/admin-guide/mm/ksm.html), 寻找相同的页面并合并, 多余的页面即可释放回系统另为它用. 而剩下的唯一的页面, 会被标为只读, 当有进程要写该页面, 该会为其分配新的页面.



值得一提的是, 在匹配相同页面时, 一种常规的算法是对页面进行哈希, 放入哈希列表, 用哈希值来进行匹配. 最开始 KSM 确定是用这种方法, 不过 VMWare 公司拥有跟该做法很相近的算法专利, 所以后来采用了另一种算法, 用红黑树代替哈希表, 把页面内容当成一个字符串来做内容比对, 以代替哈希比对. 由于在红黑树中也是以该"字符串值"大小作为键, 因此查找两个匹配的页面速度并不慢, 因为大部分比较只要比较开始若干位即可. 关于算法细节, 感兴趣者可以参考这两篇文章:

1.  [Linux内存管理:  Linux Kernel Shared Memory 剖析 Linux 内核中的内存去耦合](https://www.cnblogs.com/ajian005/archive/2012/12/18/2841108.html).

2.  [KSM tries again](https://lwn.net/Articles/330589)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/07/14 | Zhansaya Bagdauletkyzy <zhansayabagdaulet@gmail.com> | [add KSM selftests](https://lore.kernel.org/patchwork/patch/1459608) | 新增 KSM 相关的 selftest. | v1 ☑ NA | [PatchWork v2,0/4](https://patchwork.kernel.org/project/linux-mm/cover/cover.1626252248.git.zhansayabagdaulet@gmail.com) |
| 2021/08/19 | Zhansaya Bagdauletkyzy <zhansayabagdaulet@gmail.com> | [add KSM performance tests](https://lore.kernel.org/patchwork/patch/1470603) | 新增 KSM 性能相关的 selftest. | v1 ☑ NA | [PatchWork 0/2](https://patchwork.kernel.org/project/linux-mm/cover/cover.1627828548.git.zhansayabagdaulet@gmail.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3,0/2](https://patchwork.kernel.org/project/linux-mm/cover/cover.1629386192.git.zhansayabagdaulet@gmail.com) |
| 2022/03/14 | Lv Ruyi <cgel.zte@gmail.com> | [ksm: Count ksm-merging pages for each process](https://patchwork.kernel.org/project/linux-mm/patch/20220314015355.2111696-1-xu.xin16@zte.com.cn/) | 由于当前 KSM 只计算 KSM 合并页面的数量(例如, ksm_pages_sharing 和 ksm_pages_shared), 无法看到更细粒度的 KSM 合并, 对于上层应用程序优化, 无法根据每个进程的 KSM 页面合并概率轻松设置合并区域. 因此, 有必要添加额外的统计手段, 以便上层用户能够了解每个进程的详细 KSM 合并信息.<br>这个补丁在 proc 下添加了一个名为 ksm_merging_pages 的新文件, 以表示该进程中涉及的 KSM 合并页面. | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/all/20220314015355.2111696-1-xu.xin16@zte.com.cn)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/all/20220315114849.2119443-1-xu.xin16@zte.com.cn) |
| 2022/03/23 | CGEL <cgel.zte@gmail.com> | [mm/vmstat: add events for ksm cow](https://patchwork.kernel.org/project/linux-mm/patch/20220323031730.2342930-1-yang.yang29@zte.com.cn/) | 当用户想要节省内存时, 可以通过调用 madvise MADV_MERGEABLE 来使用 KSM, 但是这会导致 KSM COW 延迟. 用户可以通过读取 `/sys/kernel/mm/ksm/pages_sharing` 来了解 KSM 节省了多少内存, 但他们不知道 KSM COW 的成本, 这对于一些延迟敏感的任务来说是很重要的. 因此, 添加 KSM COW 事件(KSM_COW_SUCCESS 和 KSM_COW_FAIL)来帮助用户评估是否或如何使用 KSM. | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/all/20220323031730.2342930-1-yang.yang29@zte.com.cn)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/1](https://lore.kernel.org/all/20220323075714.2345743-1-yang.yang29@zte.com.cn)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/1](https://lore.kernel.org/all/20220324104332.2350482-1-yang.yang29@zte.com.cn) |
| 2022/05/08 | CGEL <cgel.zte@gmail.com> | [mm/ksm: introduce ksm_force for each process](https://patchwork.kernel.org/project/linux-mm/patch/20220508091426.928094-1-xu.xin16@zte.com.cn) | 要使用 KSM, 我们必须在应用程序代码中显式调用 madvise(), 这意味着在操作系统上安装的应用程序需要卸载, 源代码需要修改. 很不方便. 为了改变这种情况, 我们在 `/proc/<pid>` 下添加一个 proc 文件 ksm_force, 以支持动态地打开/关闭进程的 MM 的 KSM 扫描.<br>1. 如果 ksm_force 设置为 1, 强制该 mm 的所有匿名和 "合格" 的 VMA 参与 KSM 扫描, 而不显式调用 madvise() 将 VMA 标记为 MADV_MERGEABLE. 但仅当 "/sys/kernel/mm/ksm/run" 的 klob 设置为 1 时有效.<br>2. 如果 ksm_force 设置为 0, 则取消该进程的 ksm_force 特性, 并取消属于 vma 的合并页面, 这些 vma 不再被合并, 但是依旧保留合并的 MADV_MERGEABLE 区域的行为. | v4 ☐☑ | [LORE v4,0/1](https://lore.kernel.org/r/20220508091426.928094-1-xu.xin16@zte.com.cn)<br>*-*-*-*-*-*-*-* <br>[LORE v6,0/1](https://lore.kernel.org/r/20220510122242.1380536-1-xu.xin16@zte.com.cn)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/1](https://lore.kernel.org/all/20220512070347.1628163-1-xu.xin16@zte.com.cn) |
| 2022/06/09 | CGEL <cgel.zte@gmail.com> | [mm/ksm: provide global_force to see maximum potential merging](https://patchwork.kernel.org/project/linux-mm/patch/20220609055658.703472-1-xu.xin16@zte.com.cn/) | 648704 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220609055658.703472-1-xu.xin16@zte.com.cn) |
| 2022/07/01 | CGEL <cgel.zte@gmail.com> | [[linux-next] mm/madvise: allow KSM hints for process_madvise](https://patchwork.kernel.org/project/linux-mm/patch/20220701084323.1261361-1-xu.xin16@zte.com.cn/) | 655733 | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220701084323.1261361-1-xu.xin16@zte.com.cn) |



## 14.2 HWPoison - 内存页错误的处理
-------

**2.6.32(2009年12月发布)**


[一开始想把这节放在第12章"**内存管理调试支持**"中, 不过后来觉得这并非用于主动调试的功能, 所以还是放在此章. ]



随着内存颗粒密度的增大和内存大小的增加, 内存出错的概率也随之增大. 尤其是数据中心或云服务商, 系统内存大(几百 GB 甚至上 TB 级别), 又要提供高可靠的服务(RAS), 不能随随便便宕机; 然而, 内存出错时, 特别是出现多于 ECC(Error Correcting Codes) 内存条所支持的可修复 bit 位数的错误时, 此时硬件也爱莫能助, 将会触发一个 MCE(Machine Check Error) 异常, 而通常操作系统对于这种情况的做法就是 panic (操作系统选择 go die). 但是, 这种粗暴的做法显然是 over kill, 比如出错的页面是一个**文件缓存页(page cache),** 那么操作系统完全可以把它废弃掉(随后它可以从后备文件系统重新把该页内容读出), 把该页隔离开来不用即是.



这种需求在 Intel 的 Xeon 处理器中得到实现. Intel Xeon 处理器引入了一个所谓 MCA(Machine Check Abort)架构, 它支持包括**内存出错的的毒化(Poisoning)**在内的硬件错误恢复机制. 当硬件检测到一个无法修复的内存错误时,会把该数据标志为损坏(poisoned); 当之后该数据被读或消费时, 将会触发机器检查(Machine Check), 不同的时, 不再是简单地产生 MCE 异常, 而是调用操作系统定义的处理程序, 针对不同的情况进行细致的处理.



2.6.32 引入的 HWPoison 的 patch, 就是这个操作系统定义的处理程序, 它对错误数据的处理是以页为单位, 针对该错误页是匿名页还是文件缓存页, 是系统页还是进程页, 等等, 多种细致情况采取不同的措施. 关于此类细节, 可看此文章: [HWPOISON](https://lwn.net/Articles/348886)

[How to cope with hardware-poisoned page-cache pages](https://lwn.net/Articles/893565)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:---:|:---:|:----:|:---------:|:----:|
| 2022/06/30 | Naoya Horiguchi <naoya.horiguchi@linux.dev> | [mm, hwpoison: enable 1GB hugepage support (v3)](https://patchwork.kernel.org/project/linux-mm/cover/20220630022755.3362349-1-naoya.horiguchi@linux.dev/) | 655232 | v3 ☐☑ | [LORE v3,0/9](https://lore.kernel.org/r/20220630022755.3362349-1-naoya.horiguchi@linux.dev)[LORE v4,0/9](https://lore.kernel.org/r/20220704013312.2415700-1-naoya.horiguchi@linux.dev)<br>*-*-*-*-*-*-*-* <br>[LORE v7,0/8](https://lore.kernel.org/r/20220714042420.1847125-1-naoya.horiguchi@linux.dev) |


## 14.3 Cross Memory Attach - 进程间快速消息传递
-------

**3.2(2012年1月发布)**

这一节相对于其他本章内容是独立的. MPI(Message Passing Interface, 消息传递接口) [The Message Passing Interface (MPI) standard](https://www.mcs.anl.gov/research/projects/mpi) 是一个定义并行编程模型下用于进程间消息传递的一个高性能, 可扩展, 可移植的接口规范(注意这只是一个标准, 有多个实现). 之前的 MPI 程序在进程间共享信息是用到共享内存(shared memory)方式, 进程间的消息传递需要 2 次内存拷贝. 而 3.2 版本引入的 "Cross Memory Attach" 的 patch, 引入两个新的系统调用接口. 借用这两个接口, MPI 程序可以只使用一次拷贝, 从而提升性能.

## 14.4 OOM
-------

[Better tools for out-of-memory debugging](https://lwn.net/Articles/894546)

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/07/08 | Gang Li <ligang.bdlg@bytedance.com> | [mm, oom: Introduce per numa node oom for CONSTRAINT_{MEMORY_POLICY,CPUSET}](https://lore.kernel.org/all/20220708082129.80115-1-ligang.bdlg@bytedance.com) | 在本系列补丁之前, OOM 只会通过选择整个系统上错误率最高的进程来杀死内存使用率最高(最富裕)的进程. 这在 UMA 系统上运行良好, 但在 NUMA 系统上可能会发生一些意外死亡. 比如, 如果进程 c.out 绑定到 Node1, 并继续从 Node1 分配页面, 而 OOM-Killer 将首先选择 a.out, 但是杀死 a.out 并没有释放 Node1 上的任何内存, 所以 c.out 将被杀死. 如果 mempolicy 或 cpuset 有效, out_of_memory() 将选择特定节点上的受害者进行杀死. 这样内核就可以避免 NUMA 系统上的意外杀戮. | v2 ☐☑✓ | [LORE v2,0/5](https://lore.kernel.org/all/20220708082129.80115-1-ligang.bdlg@bytedance.com) |


## 14.5 能耗感知(EAMM)
-------

[Energy-Aware Memory Management for Embedded Multimedia Systems: A Computer-Aided Design Approach: 24](https://www.amazon.com.mx/Energy-Aware-Management-Embedded-Multimedia-Systems/dp/1439814007)

[HpMC: An Energy-aware Management System of Multi-level Memory Architectures](https://www.researchgate.net/publication/280580086_HpMC_An_Energy-aware_Management_System_of_Multi-level_Memory_Architectures)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2013/09/26 | Srivatsa S. Bhat <srivatsa.bhat@linux.vnet.ibm.com> | [mm: Memory Power Management](https://lore.kernel.org/all/52437128.7030402@linux.vnet.ibm.com) | 20130925231250.26184.31438.stgit@srivatsabhat.in.ibm.com | v4 ☐☑✓ | [LORE v2,00/15](https://lore.kernel.org/lkml/20130409214443.4500.44168.stgit@srivatsabhat.in.ibm.com)<br>*-*-*-*-*-*-*-* <br>[LORE RRC v4,0/40](https://lore.kernel.org/all/52437128.7030402@linux.vnet.ibm.com) |


## 14.6 PER_CPU
-------

[PERCPU 变量实现](https://zhuanlan.zhihu.com/p/260986194)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2009/02/18 | Tejun Heo <tj@kernel.org> | [implement new dynamic percpu allocator](https://lore.kernel.org/patchwork/patch/144750) | 实现了 [vm_area_register_early()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f0aa6617903648077dffe5cfcf7c4458f4610fa7) 以支持在启动阶段注册 vmap 区域. 基于此特性实现了可伸缩的动态 percpu 分配器(CONFIG_HAVE_DYNAMIC_PER_CPU_ARE), 可用于静态(pcpu_setup_static/pcpu_setup_first_chunk)和动态(`__alloc_percpu`) percpu 区域, 这将允许静态和动态区域共享更快的直接访问方法. | v1 ☑ 2.6.30-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/144750) |
| 2009/02/24 | Tejun Heo <tj@kernel.org> | [percpu: fix pcpu_chunk_struct_size](https://lore.kernel.org/patchwork/patch/145272) | 1. 为对 static per_cpu(第一个 percpu 块)分配增加更多的自由度: 引入 PERCPU_DYNAMIC_RESERVE 用于预留 percpu 空间, 修改 pcpu_setup_static() 为 pcpu_setup_first_chunk(), 并让其更灵活.<br>2. 实现嵌入式的 percpu 分配器, 在 !NUMA 模式下, 可以简单地分配连续内存并将其用于第一个块, 而无需将其映射到 vmalloc 区域. 由于内存区域是由大页物理内存映射覆盖的, 所以它允许在基于可伸缩的动态 percpu 分配器分配静态 percpu 区域不引入任何 TLB 开销, 而且实现也非常简单.  | v1 ☑ 2.6.30-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/145272) |
| 2009/03/26 | Tejun Heo <tj@kernel.org> | [percpu: clean up percpu constants](https://lore.kernel.org/patchwork/patch/146513) | NA | v1 ☑ 2.6.30-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/146513) |
| 2009/07/21 | Tejun Heo <tj@kernel.org> | [percpu:  implement pcpu_get_vm_areas() and update embedding first chunk allocator](https://lore.kernel.org/patchwork/patch/164588) | NA | v1 ☑ 2.6.32-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/164588) |
| 2008/11/18 | Tejun Heo <tj@kernel.org> | [Improve alloc_percpu: expose percpu_modalloc and percpu_modfree](https://lore.kernel.org/patchwork/patch/135207) | NA | v1 ☐ | [PatchWork 0/7](https://lore.kernel.org/patchwork/patch/135207) |
| 2021/09/10 | Kefeng Wang <wangkefeng.wang@huawei.com> | [arm64: support page mapping percpu first chunk allocator](https://lore.kernel.org/patchwork/patch/1464663) | NA | v2 ☐ v5.15-rc1 | [PatchWork v4,0/3](https://patchwork.kernel.org/project/linux-mm/cover/20210910053354.26721-1-wangkefeng.wang@huawei.com) |


## 14.7 ASLR
-------

### 14.7.1 ASLR(User Space)
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/01/24 | Topi Miettinen <toiwoton@gmail.com> | [mm: Optional full ASLR for mmap(), vdso, stack and heap](https://lore.kernel.org/patchwork/patch/1370134) | NA | v1 ☑ 2.6.30-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/1370134) |
| 2021/01/24 | Topi Miettinen <toiwoton@gmail.com> | [mm: Optional full ASLR for mmap(), vdso, stack and heap](https://lore.kernel.org/patchwork/patch/1370134) | NA | v1 ☑ 2.6.30-rc1 | [PatchWork](https://lore.kernel.org/patchwork/patch/1370134) |


### 14.7.2 KASLR
-------

[Kernel Address Space Layout Randomization](https://www.phoronix.com/scan.php?page=search&q=Kernel%20Address%20Space%20Layout%20Randomization)


*   内核地址随机化 KASLR

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/13 | Xiongwei Song <sxwjean@gmail.com> | [Use generic code for randomization of virtual address of x86](https://patchwork.kernel.org/project/linux-mm/cover/20211011143150.318239-1-sxwjean@me.com) | ASLR 的重构. | v1 ☐ | [PatchWork v2,0/6](https://patchwork.kernel.org/project/linux-mm/cover/20211011143150.318239-1-sxwjean@me.com) |
| 2017/09/03 | Ard Biesheuvel <ard.biesheuvel@linaro.org> | [implement KASLR for ARM](https://lwn.net/Articles/732891) | ARM 支持 KASLR. | v1 ☐ | [LWN](https://lwn.net/Articles/732891)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/6](https://git.kernel.org/pub/scm/linux/kernel/git/ardb/linux.git/log/?h=arm-kaslr-latest) |
| 2016/01/26 | Ard Biesheuvel <ard.biesheuvel@linaro.org> | [arm64: implement support for KASLR](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=2b5fe07a78a09a32002642b8a823428ade611f16) | 支持 ARM64 KASLR | v4 ☐☑✓ 4.6-rc1 | [LORE v4,0/22](https://lore.kernel.org/all/1453828249-14467-1-git-send-email-ard.biesheuvel@linaro.org) |


*   随机函数偏移(FGKASLR)

[Intel Open-Source Developer Has Been Working On "FGKASLR" For Better Kernel Security](https://www.phoronix.com/scan.php?page=news_item&px=Intel-Linux-FGKASLR-Proposal)

[Linux 5.16 Has Early Preparations For Supporting FGKASLR](https://www.phoronix.com/scan.php?page=news_item&px=Linux-5.16-Preps-For-FGKASLR)

[FGKASLR Is An Exciting Linux Kernel Improvement To Look Forward To In 2022](https://www.phoronix.com/scan.php?page=news_item&px=Linux-FGKASLR-2022)

[FGKASLR Patches Revised A 10th Time For Improving Linux Kernel Security](https://www.phoronix.com/scan.php?page=news_item&px=FGKASLR-Linux-v10)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/10/13 | Kees Cook <keescook@chromium.org> | [x86: Various clean-ups in support of FGKASLR](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ca136cac37eb51649d52d5bc4271c55e30ed354c) | FGKASLR 的一些的准备工作, 都是一些独立的改动. 比如:<br>1. 在 relocs 工具中支持[超过 64K 的节头](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=a54c401ae66fc78f3f0002938b3465ebd6379009).<br>2. KASLR 中通过  kaslr_get_random_long() 提取随机种子时候, 通过 earlyprintk debug_putstr 输出了很多无用的调试信息, 允许将[参数 purpose 置 NULL](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0d054d4e82072bcfd5eb961536b09a9b3f5613fb)来使输出静默.<br>4. 修改 [ORC 查找表的大小]](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ca136cac37eb51649d52d5bc4271c55e30ed354c)以覆盖整个内核的代码段. | v1 ☑ [5.16-rc1](https://lkml.org/lkml/2021/11/2/26) | [LORE 0/4](https://lore.kernel.org/all/20211013175742.1197608-1-keescook@chromium.org) |
| 2020/07/17 | Topi Miettinen <toiwoton@gmail.com> | [Function Granular KASLR](https://patchwork.kernel.org/project/kernel-hardening/cover/20200717170008.5949-1-kristen@linux.intel.com/#23528683) | NA | v1 ☑ v4 | [PatchWork v4,00/10](https://patchwork.kernel.org/project/kernel-hardening/cover/20200717170008.5949-1-kristen@linux.intel.com/#23528683)<br>*-*-*-*-*-*-*-* <br>[LORE v9,00/15](https://lore.kernel.org/lkml/20211223002209.1092165-1-alexandr.lobakin@intel.com) |


### 14.7.3 Randomize Offset
-------


*   随机栈偏移

黑客们经常利用堆栈的信息, 窥测进程运行时的指令和数据. 为了保护堆栈数据, 增加攻击的难度, Linux 内核堆栈保护不断改进, 比如基于 vmap 的堆栈分配和保护页面(VMAP_STACK)、 删除 thread_info()、[STACKLEAK](https://a13xp0p0v.github.io/img/Alexander_Popov-stackleak-LinuxPiter2017.pdf), 攻击者必须找到新的方法来利用它们的攻击.

这里面很大一部分都是 Pax 的功劳, PaX 团队在内核安全方面进行了很多实践. [Documentation for the PaX project](https://pax.grsecurity.net/docs).


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/09/13 | Elena Reshetova <elena.reshetova@intel.com> | [thread_info cleanups and stack caching](https://lore.kernel.org/all/a0898196f0476195ca02713691a5037a14f2aac5.1473801993.git.luto@kernel.org) | 引入 CONFIG_THREAD_INFO_IN_TASK 将 thread_info 保存到 task_struct 中.<br>之前 thread_info 与内核栈放在一起, 如果不慎踩了栈会同时破坏 thread_info, 极其不可靠. 因此把 thread_info 保存到 task_struct 中, 跟内核栈分离, 提高可靠性. | v1 ☑ 4.9-rc1 | [PatchWork](https://lore.kernel.org/kernel-hardening/20190329081358.30497-1-elena.reshetova@intel.com/) |
| 2017/12/06 | Alexander Popov <alex.popov@...ux.com> | [Introduce the STACKLEAK feature and a test for it](https://www.openwall.com/lists/kernel-hardening/2017/12/05/18) | STACKLEAK 是由 Grsecurity/PaX 开发的一种安全功能, 它:<br>1. 减少了内核堆栈泄漏 bug 可能泄露的信息;<br>2. 阻止一些未初始化的堆栈变量攻击(例如CVE-2010-2963);<br>3. 引入一些内核堆栈溢出检测的运行时检查. | v6 ☑ 4.20-rc1 | [PatchWork v6,0/6](https://lwn.net/Articles/725287)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6,0/6](https://www.openwall.com/lists/kernel-hardening/2017/12/05/19) |

最早 PaX 团队的 [RANDKSTACK 特性](https://pax.grsecurity.net/docs/randkstack.txt) 提出了内核栈随机偏移的最早思想. 这个特性旨在大大增加依赖确定性堆栈结构的各种基于堆栈的攻击的难度.


其主要思想是:

1.  由于堆栈偏移量在每次系统调用时都是随机的, 因此在执行攻击时, 攻击者很难可靠地落在线程堆栈上的任何特定位置.

2.  此外, 由于随机化是在确定的 pt_regs 之后执行的, 因此在长时间运行的系统调用期间, 不应该使用基于 ptrace 的方法来发现随机化偏移量.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/03/29 | Elena Reshetova <elena.reshetova@intel.com> | [x86/entry/64: randomize kernel stack offset upon syscall](https://lore.kernel.org/kernel-hardening/20190329081358.30497-1-elena.reshetova@intel.com) | 引入了 CONFIG_RANDOMIZE_KSTACK_OFFSET, 在 pt_regs 的固定位置之后, 系统调用的每个条目都会随机化内核堆栈偏移量. | v1 ☐ | [PatchWork](https://lore.kernel.org/kernel-hardening/20190329081358.30497-1-elena.reshetova@intel.com/) |
| 2021/04/06 | Kees Cook <keescook@chromium.org> | [Optionally randomize kernel stack offset each syscall](https://patchwork.kernel.org/project/linux-mm/cover/20200406231606.37619-1-keescook@chromium.org) | Elena 先前添加内核堆栈基偏移随机化的工作的延续和重构. | v4 ☐ | [PatchWork v3,0/5](https://patchwork.kernel.org/project/linux-mm/cover/20200406231606.37619-1-keescook@chromium.org) |
| 2021/08/13 | Kefeng Wang <wangkefeng.wang@huawei.com> | [riscv: Improve stack randomisation on RV64](https://www.phoronix.com/scan.php?page=news_item&px=RISC-V-Better-Stack-Rand) | NA | v1 ☑ 5.15-rc1 | [PatchWork](https://patchwork.kernel.org/project/linux-riscv/patch/20210812114702.44936-1-wangkefeng.wang@huawei.com), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d5935537c8256fc63c77d5f4914dfd6e3ef43241) |

### 14.7.4 Shadow stacks
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2022/01/30 | Edgecombe, Rick P <rick.p.edgecombe@intel.com> | [Shadow stacks for userspace](https://patchwork.kernel.org/project/linux-mm/cover/20220130211838.8382-1-rick.p.edgecombe@intel.com/) | 609893 | v1 ☐☑ | [PatchWork v1,0/35](https://lore.kernel.org/r/20220130211838.8382-1-rick.p.edgecombe@intel.com) |



## 14.8 页面迁移
-------

*   migration

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2005/11/01 | Christoph Lameter <clameter@sgi.com> | [Swap Migration V5: Overview](https://lwn.net/Articles/157066) | 交换迁移允许在进程运行时通过交换在 NUMA 系统中的节点之间移动页的物理位置.<br>1. 这组补丁不会自动迁移已移动的进程的内存, 相反, 它把迁移决策留给了用户空间. 因此并没有直接解决进程跨接点迁移后内存不能跟随迁移的问题, 但它确实试图建立了迁移的通用框架, 以便最终能够发展出完整的迁移解决方案.<br>2. 引入个新的系统调用 migrate_pages 尝试将属于给定进程的任何页面从 old_nodes 移动到 new_nodes.<br>3. 新的 MPOL_MF_MOVE 选项, 在 set_mempolicy() 系统调用中使用, 可用于相同的效果. | v5 ☑ 2.6.16-rc1 | [PatchWork v5,0/5](https://lore.kernel.org/patchwork/patch/45422) |
| 2006/01/10 | Christoph Lameter <clameter@sgi.com> | [Direct Migration V9: Overview](https://lore.kernel.org/patchwork/patch/49754) | NA | v9 ☑ 2.6.16-rc2 | [PatchWork v9,0/5](https://lore.kernel.org/patchwork/patch/49754) |
| 2011/12/14 | Mel Gorman <mgorman@suse.de> | [Reduce compaction-related stalls and improve asynchronous migration of dirty pages v6](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=0cee34fd72c582b4f8ad8ce00645b75fb4168199) | 使用 VFAT 的 U 盘在启用 THP 的情况下使用时会出现严重的暂停, 这一系列会减少暂停. 这是由于内存分配慢速路径下同步内存迁移(回收规整时进行)时进行脏页写入造成的. 这组补丁试图缓解因为内存规整或回收过多页面而导致用户可见的方式停滞, 从而进一步扩展 THP 的使用场景.  | v6 ☑✓ 3.3-rc1 | [LORE 0/5](https://lore.kernel.org/all/1321635524-8586-1-git-send-email-mgorman@suse.de)<br>*-*-*-*-*-*-*-* <br>[LORE RRC v4r2,0/7](https://lore.kernel.org/all/1321900608-27687-1-git-send-email-mgorman@suse.de)<br>*-*-*-*-*-*-*-* <br>[LORE v6,0/11](https://lore.kernel.org/all/1323877293-15401-1-git-send-email-mgorman@suse.de) |
| 2014/05/06 | David Rientjes <rientjes@google.com> | [mm, migration: add destination page freeing callback](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=aeef4b83806f49a0c454b7d4578671b71045bee2) | alpine.DEB.2.02.1405070336200.16568@chino.kir.corp.google.com | v4 ☑✓ 3.16-rc1 | [LORE 1/2](https://lore.kernel.org/all/alpine.DEB.2.02.1404301744110.8415@chino.kir.corp.google.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/6](https://lore.kernel.org/all/alpine.DEB.2.02.1405061920470.18635@chino.kir.corp.google.com) |
| 2021/08/05 | Christoph Lameter <clameter@sgi.com> | [Some cleanup for page migration](https://lore.kernel.org/patchwork/patch/1472581) | NA | v1 ☐ | [PatchWork 0/5](https://lore.kernel.org/patchwork/patch/49754) |
| 2021/09/22 | John Hubbard <jhubbard@nvidia.com> | [mm/migrate: de-duplicate migrate_reason strings](https://patchwork.kernel.org/project/linux-mm/patch/20210922041755.141817-2-jhubbard@nvidia.com/) | NA | v1 ☐ | [PatchWork 0/5](https://patchwork.kernel.org/project/linux-mm/patch/20210922041755.141817-2-jhubbard@nvidia.com/) |
| 2021/11/03 | Baolin Wang <baolin.wang@linux.alibaba.com> | [Improve the migration stats](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=84b328aa81216e08804d8875d63f26bda1298788) | 根据与 [Zi Yan](https://lore.kernel.org/linux-mm/7E44019D-2A5D-4BA7-B4D5-00D4712F1687@nvidia.com) 的谈话, 这个补丁集改变了 migrate_pages() 的返回值, 以避免返回的数字大于用户通过 move_pages() 系统调用尝试迁移的页面数. 还修复了 trace_mm_compaction_migratepages() 中的 hugetlb 迁移统计和迁移统计. | v1 ☑✓ 5.17-rc1 | [PatchWork](https://lore.kernel.org/linux-mm/b35e54802a9a82d03d24845b463e9d9a68f7fd6b.1635491660.git.baolin.wang@linux.alibaba.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC,0/3](https://patchwork.kernel.org/project/linux-mm/cover/cover.1635936218.git.baolin.wang@linux.alibaba.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/3](https://lore.kernel.org/all/cover.1636275127.git.baolin.wang@linux.alibaba.com) |
| 2021/11/11 | Baolin Wang <baolin.wang@linux.alibaba.com> | [Support multiple target nodes demotion](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ac16ec835314677dd7405dfb5a5e007c3ca424c7) | 对于同时有1个快速(DRAM)内存节点和多个慢速(持久内存)内存节点的系统, 根据当前节点降级策略不感知节点之间的距离, 速度和带宽等. 该补丁修改了 node_demotion 数据结构, 以支持多个目标节点, 并建立迁移路径以支持多个目标节点验证节点距离是否是最好的. | v1 ☑✓ 5.17-rc1 | [PatchWork v1](https://patchwork.kernel.org/project/linux-mm/patch/8850612186ea23eb5d328d84e4008a6b60f418e1.1636514506.git.baolin.wang@linux.alibaba.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://patchwork.kernel.org/project/linux-mm/patch/f6d68800ff2efcb0720599ae092d30765a640232.1636428988.git.baolin.wang@linux.alibaba.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,0/2](hhttps://patchwork.kernel.org/project/linux-mm/cover/cover.1636616548.git.baolin.wang@linux.alibaba.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3](https://patchwork.kernel.org/project/linux-mm/patch/a31dc065a7901bcdca0d9642d0def0f57e865e20.1636683991.git.baolin.wang@linux.alibaba.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4](https://lore.kernel.org/all/00728da107789bb4ed9e0d28b1d08fd8056af2ef.1636697263.git.baolin.wang@linux.alibaba.com) |
| 2022/01/28 | Anshuman Khandual <anshuman.khandual@arm.com> | [mm/migration: Add trace events](https://patchwork.kernel.org/project/linux-mm/cover/1643368182-9588-1-git-send-email-anshuman.khandual@arm.com) | 增加进程迁移的 tracepoint. | v3 ☐ | [PatchWork V3,0/2](https://patchwork.kernel.org/project/linux-mm/cover/1643368182-9588-1-git-send-email-anshuman.khandual@arm.com) |


* numa balancing


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/09/22 | John Hubbard <jhubbard@nvidia.com> | [Mitosis: Transparently Self-Replicating Page-Tables for Large-Memory Machines](https://research.vmware.com/files/attachments/0/0/0/0/1/0/3/aspl0359a-achermanna.pdf) | 迁移的时候考虑迁移 page-table. | v1 ☐ | [GitHub](https://github.com/mitosis-project/mitosis-linux) |




## 14.9 [Free Page Reporting](https://www.kernel.org/doc/html/latest/vm/free_page_reporting.html)
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/02/11 | Alexander Duyck <alexander.h.duyck@linux.intel.com> | [mm / virtio: Provide support for free page reporting](https://patchwork.kernel.org/project/linux-mm/cover/20200211224416.29318.44077.stgit@localhost.localdomain) | 本系列提供了一种异步方法, 用于向虚拟机监控程序报告空闲的 guest 页面, 以便主机上的其他进程和/或 guest 可以删除和重用与这些页面关联的内存.<br>使用此功能, 可以避免对磁盘进行不必要的I/O, 并在主机内存过度使用的情况下大大提高性能.<br>启用时, 将每2秒扫描一次可用内存, 同时释放足够高阶的页面. | v17 ☑ 5.7-rc1 | [PatchWork v17,0/9](https://lore.kernel.org/patchwork/patch/49754) |
| 2021/03/23 | Alexander Duyck <alexander.h.duyck@linux.intel.com> | [x86/Hyper-V: Support for free page reporting](https://lore.kernel.org/patchwork/patch/1401238) | Linux现在支持虚拟化环境的免费页面报告(36e66c554b5c "mm: introduce Reported pages". 在 Hyper-V 上, 当配置了虚拟备份虚拟机时, Hyper-V 将在支持时公布冷内存丢弃功能. 此修补程序添加了对挂接免费页面报告基础结构的支持, 并利用 Hyper-V 冷内存丢弃提示 hypercall 将这些页面报告/释放回主机. | v5 ☑ 5.13-rc1 | [PatchWork v5](https://lore.kernel.org/patchwork/patch/1401238) |



## 14.10 PKRAM
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/05/07 | Anthony Yznaga <anthony.yznaga@oracle.com> | [PKRAM: Preserved-over-Kexec RAM](https://lore.kernel.org/patchwork/patch/856356) | NA | v11 ☑ 4.15-rc2 | [PatchWork RFC,00/43](https://lore.kernel.org/patchwork/patch/1237362) |


## 14.11 unaccepted memory
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/08/10 | Anthony Yznaga <anthony.yznaga@oracle.com> | [x86: Impplement support for unaccepted memory](https://patchwork.kernel.org/project/linux-mm/cover/20210810062626.1012-1-kirill.shutemov@linux.intel.com) | UEFI规范 v2.9 引入了内存接受的概念, 一些虚拟机平台, 如Intel TDX或AMD SEV-SNP, 要求在来宾使用内存之前先接受内存, 并通过特定于虚拟机平台的协议进行接受.<br>接受内存成本很高, 这会使 VMM 为接受的来宾物理地址范围分配内存, 最好等到需要使用内存时再接受内存, 这可以减少启动时间并减少内存开销.<br>支持这种内存需要对核心 mm 代码进行少量更改:<br>1. memblock 必须在分配时接受内存;<br>2. 页面分配器必须在第一次分配页面时接受内存;<br>3. Memblock更改是微不足道的.<br>4. 页面分配器被修改为在第一次分配时接受页面.<br>5. PageOffline() 用于指示页面需要接受.<br>6. 热插拔和引出序号当前使用的标志, 这样的页面对页面分配器不可用.<br>如果一个体系结构想要支持不可接受的内存, 它必须提供三个助手:<br>1. accept_memory() 使一系列物理地址被接受;<br>2. 如果页面需要接受, 则通过 maybe_set_page_offline() 将页面标记为 PageOffline(), 在引导期间用于将页面放在空闲列表上.<br>3. clear_page_offline() 清除使页面被接受并清除 PageOffline(). | v1 ☐ | [PatchWork 0/5](https://patchwork.kernel.org/project/linux-mm/cover/20210810062626.1012-1-kirill.shutemov@linux.intel.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,00/12](https://lore.kernel.org/r/20220425033934.68551-1-kirill.shutemov@linux.intel.com) |


## 14.12 USERCOPY
-------

如果内核开启 [CONFIG_HARDENED_USERCOPY](https://blog.csdn.net/tiantao2012/article/details/108997191), 则在从 user space copy 数据到 kernel space 时做一些检查, 如果不是有效数据, 则 assert. 可以通过设置 `hardened_usercopy=0|1` 来开启和关闭 usercopy.

[一例 hardened_usercopy 的 bug 解析](http://www.wowotech.net/linux_kenrel/480.html)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/10/04 | Kees Cook <keescook@chromium.org> | [mm: Hardened usercopy](https://lore.kernel.org/all/1467843928-29351-1-git-send-email-keescook@chromium.org) | 将 PAX_USERCOPY 推到主线. PAX_USERCOPY 的设计是为了发现在使用 copy_to_user()/copy_from_user() 时存在的几类缺陷. | v1 ☑ 4.8-rc2 | [PatchWork 0/9](https://patchwork.kernel.org/project/linux-mm/cover/20211004224224.4137992-1-willy@infradead.org)<br>*-*-*-*-*-*-*-* <br>[LWN v2](https://lwn.net/Articles/691012/) |
| 2021/12/16 | "Matthew Wilcox (Oracle)" <willy@infradead.org> | [Assorted improvements to usercopy](https://lore.kernel.org/patchwork/patch/856356) | usercopy 的各种改进 | v1 ☐ | [2021/10/04 PatchWork 0/3](https://patchwork.kernel.org/project/linux-mm/cover/20211004224224.4137992-1-willy@infradead.org)<br>*-*-*-*-*-*-*-* <br>[2021/12/16 PatchWork v4,0/4](https://patchwork.kernel.org/project/linux-mm/cover/20211216215351.3811471-1-willy@infradead.org) |


## 14.13 ZONE_MOVABLE
-------

减少内存的碎片化是永恒的主题, 但是很多情况下事与愿违. 想象一下这个场景, 当我们需要一块大的连续内存时向伙伴系统申请, 虽然对应的内存区剩余内存还很多, 但是却发现对应的内存区由于碎片化过多, 并无法满足连续的内存申请需求, 那么此时是可以通过内存迁移来完成连续内存的申请, 但是这个过程并不一定能够成功, 因为中间往往有一些页面可能是不允许迁移的.

因此内核在 v2.6.23 引入 ZONE_MOVABLE 来减少内存碎片, 提高内存迁移的成功率. 内存管理子系统本身就把内存划分为不同 zone 来管理, ZONE_MOVABLE 与其他 zone 区域不同, 它是一个 pseudo zone, 即(虚拟的)伪的内存区域呢. 因此它实际上它是从平台中最高内存区中(比如 `ZONE_HIHGMEM`)单独划出了一部分内存, 作为 ZONE_MOVABLE.

ZONE_MOVABLE 的作用:


1.  引入 ZONE_MOVABLE 的主要目的是想要把 Non-Movable 和 Movable 的内存区分管理, 当我们划分出该区域后, 那么只有可迁移的页才能够从该区域申请, 这样当我们后面回收内存时, 针对该区域就都可以执行迁移, 而不用担心有一些不能被迁移的页面混在其中, 从而保证能够获取到足够大的连续内存.

2.  除了这个用途之外, 该区域还有一种使用场景, 那就是 memory hotplug 场景, 内存热插拔场景, 当我们对内存区执行 remove 时, 必须保证其中的内容都是可以被迁移走的, 因此热插拔的内存区必须位于 ZONE_MOVABLE 区域. 这在虚拟化等场景很有用.


>一般来说, 可迁移的页面不一定都在 ZONE_MOVABLE 中, 但是 ZONE_MOVABLE 中的页面必须尽可能都是可迁移的.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/07/17 |  Mel Gorman <mel@csn.ul.ie> | [Create the ZONE_MOVABLE zone](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=7e63efef857575320fb413fbc3d0ee704b72845f) | 614354 | v1 ☑ 2.6.23-rc1 | [CGIT v1,0/5](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=7e63efef857575320fb413fbc3d0ee704b72845f) |


### 14.13.1 ZONE_MOVABLE
-------


2.6.23 的时候, Mel Gorman 提交补丁集 [reate ZONE_MOVABLE to partition memory between movable and non-movable pages](https://lwn.net/Articles/224255) 创建了一个名为 ZONE_MOVABLE 的分区. 引入这个 pseudo zone 的目的是为了防止 zone 内存碎片化. ZONE_MOVABLE 分区只能通过分配时指定了 `__GFP_HIGHMEM` 和 `__GFP_MOVABLE` 参数才能使用. 这样做的效果是将所有不可移动(non-movable/unmovabled)的页面都被限制在一个内存分区中, 而可移动(movable)页面的分配则由其他的内存区满足. 这组补丁可以与基于列表的抗碎片(list-based anti-fragmentation)补丁(TODO-待确认该补丁是指什么)一起应用, 后者根据移动性将页面分组在一起.

ZONE_MOVABLE 区域的大小可通过启动参数 "kernelcore=" 来制定, 这个参数指定了多少内存分配给 ZONE_MOVALBE, 其余的用于 ZONE_MOVABLE. ZONE_MOVABLE 中的任何页面都可以通过迁移页面或回收页面来释放.

ZONE_MOVABLE 一个 pseudo zone, 它实际是从内核划分的某个 zone 中单独划出了一部分内存来是使用的, 因此当我们选择某一个 zone 的页面用于 ZONE_MOVABLE 来使用时, 需要考虑两点:

1.  首先, 仅仅位于最高内存 zone 的内存可用于 ZONE_MOVABLE. 对于 32 位的平台(比如 x86, arm 等), 是 ZONE_HIGHMEM. 对于 PPC64 则是 ZONE_DMA; 对于 x86_64 则是 ZONE_DMA32.

2.  其次, 内核可用的内存数量将在可能的情况下均匀分布在 NUMA 节点上. 如果节点的大小不一, 就会导致内核在不同节点上可使用的内存数量不同.

内核启动的时候会通过 [find_usable_zone_for_movable()](https://elixir.bootlin.com/linux/v2.6.23/source/mm/page_alloc.c#L2697) 查找一块合适的 zone index 供 ZONE_MOVABLE 使用.

默认情况下, 这个 zone 不会被用作 hugetlb 的分配, 因为 hugetlb 的分配是固定的且不可迁移的(至少当时 2.6.23 版本是这样). 但是, 由于 ZONE_MOVABLE 始终具有可以迁移或回收的页面, 因此即使系统已运行很长时间, 也可以在 ZONE_MOVABLE 区域轻松地满足庞大的连续页面(比如 hugepage 等)的分配. 这更将允许管理员根据区域的大小在运行时调整 hugepage 池的大小. 因此内核仍然通过补丁集中的 [commit 396faf0303d2 ("Allow huge page allocations to use GFP_HIGH_MOVABLE")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=396faf0303d273219db5d7eb4a2879ad977ed185) 提供了一个 [sysctl hugepages_treat_as_movable](https://elixir.bootlin.com/linux/v2.6.23/source/mm/hugetlb.c#L31), 允许在 ZONE_MOVABLE 上进行 hugetlb 的分配. 这意味着, 如果页面没有被 mlock 锁定, 那么在系统的生命周期内, 可以将 hugetlb 的页面池调整为 ZONE_MOVABLE 的大小(huge page pool 的尺寸包含了 ZONE_MOVABLE 的尺寸). 尽管大页是不可移动的, 但是内核开发者们认为这不会引入额外的外部内存碎片, 因为大页不正是我们所关心的最大的连续块的分配请求么.

因此除了 hughtlb 可以是 non-movable, Mel Gorman 不再引入其他可能会导致 ZONE_MOVABLE 外碎片的分配.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/01/25 | Mel Gorman <mel@csn.ul.ie> | [Create ZONE_MOVABLE to partition memory between movable and non-movable pages](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=7e63efef857575320fb413fbc3d0ee704b72845f) | NA | v2 ☑ 2.6.23-rc1 |  [2007/01/25 LORE v1,0/8](https://lore.kernel.org/lkml/20070125234458.28809.5412.sendpatchset@skynet.skynet.ie)<br>*-*-*-*-*-*-*-* <br>[2007/03/01 LORE v2,0/8](https://lore.kernel.org/lkml/20070301100802.30048.45045.sendpatchset@skynet.skynet.ie), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ed7ed365172e27b0efe9d43cc962723c7193e34e) |

> 内核通过将已经将内存划分为许多具有不同特征的 ZONE, 来防止出现较严重的内存碎片, 特别是通过引入 ZONE_MOVABLE 专区, 通过隔离可移动和不可移动的分配请求(ZONE_MOVABLE 中区域, 只满足应该可以移动页面(或者简单地回收它们)来根据需要创建更大的区域).

理想很美好, 现实很残酷, 仍然会有一些情况会破坏 ZONE_MOVABLE 想要的美好. 经常会有一些零碎的、位置不好的、当前无法被移动移动的页面固定在 ZONE_MOVABLE 中, 而导致 ZONE_MOVABLE 的整块内存无法被移动. 参见 [Preserving the mobility of ZONE_MOVABLE](https://lwn.net/Articles/843326)

例如, 在设备执行 I/O 时就往往会 PIN 住一些页面, 此时移动页面往往会产生不幸的副作用. 通常, 这种固定持续时间很短, 往往不是一个大问题. 不过, 有时候这种固定可以持续很长时间ーー例如, 数周. 用于 RDMA 的内存缓冲区常常牵涉到这种反社会行为, 但是也有其他的例子. 当然, 一个实际上连续的用户空间缓冲区可能广泛分散在物理内存中, 因此长时间固定其中一个缓冲区可能会产生同样广泛分散的问题.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2013/01/14 | Tang Chen <tangchen@cn.fujitsu.com> | [Add movablecore_map boot option](https://lore.kernel.org/all/1358154925-21537-1-git-send-email-tangchen@cn.fujitsu.com) | 1358154925-21537-1-git-send-email-tangchen@cn.fujitsu.com | v5 ☐☑✓ | [LORE v5,0/5](https://lore.kernel.org/all/1358154925-21537-1-git-send-email-tangchen@cn.fujitsu.com) |
| 2021/02/15 | Pavel Tatashin <pasha.tatashin@soleen.com> | [prohibit pinning pages in ZONE_MOVABLE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e44605a8b1aa13d892addc59ec3d416cb186c77b) | 参见 [Preserving the mobility of ZONE_MOVABLE](https://lwn.net/Articles/843326) | v11 ☑ 5.13-rc1 | [LWN v7 00/14](https://lwn.net/ml/linux-kernel/20210122033748.924330-1-pasha.tatashin@soleen.com/), [LORE v7,0/4](https://lore.kernel.org/lkml/20210122033748.924330-1-pasha.tatashin@soleen.com)<br>*-*-*-*-*-*-*-* <br>[LORE v11,00/14](https://lore.kernel.org/all/20210215161349.246722-1-pasha.tatashin@soleen.com), [关键 COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d1e153fea2a8940273174fc17733c44323d35cd5) |



### 14.13.2 配置参数
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2007/07/17  | Mel Gorman <mel@csn.ul.ie> | [Add a movablecore= parameter for sizing ZONE_MOVABLE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7e63efef857575320fb413fbc3d0ee704b72845f) | NA | v1 ☑ 2.6.23-rc1 | [LWN v7,00/14](https://lwn.net/ml/linux-kernel/20210122033748.924330-1-pasha.tatashin@soleen.com/), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7e63efef857575320fb413fbc3d0ee704b72845f) |
| 2013/01/14 | Tang Chen <tangchen@cn.fujitsu.com> | [Add movablecore_map boot option](https://lore.kernel.org/all/1358154925-21537-1-git-send-email-tangchen@cn.fujitsu.com) | 1358154925-21537-1-git-send-email-tangchen@cn.fujitsu.com | v5 ☑✓ 3.9-rc1 | [LORE v5,0/5](https://lore.kernel.org/all/1358154925-21537-1-git-send-email-tangchen@cn.fujitsu.com) |
| 2016/01/08 | Taku Izumi <izumi.taku@jp.fujitsu.com> | [handle kernelcore=: generic](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ed7ed365172e27b0efe9d43cc962723c7193e34e) | 之前 kernelcore 启动参数的处理是架构相关的, 可能导致某些架构出现问题. 这个补丁使用通用代码来处理 `kernelcore` 参数. 适用于支持独立于 arch 的区域大小调整(即定义 CONFIG_ARCH_POPULATES_NODE_MAP) 的架构, 其他架构将忽略引导参数. | v1 ☑ 2.6.23-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ed7ed365172e27b0efe9d43cc962723c7193e34e) |
| 2018/04/02 | Dou Liyang | [x86/boot/KASLR: Extend movable_node option for KASLR](https://lkml.org/lkml/2018/4/2/600) | NA | v1 ☐ | [LKML v5,0/5](https://lkml.org/lkml/2018/4/2/600) |



### 14.13.3 内存镜像
-------

内核 v4.6 通过扩展现有的 "kernelcore" 选项, 增加 "kernelcore=mirror" 选项, 引入了内存可靠性分级的功能. 通过 BIOS 上报镜像内存(可靠内存) 的地址范围等信息, 区分镜像内存(可靠内存)和非镜像内存(不可靠内存).

当前的实现比较简单:

1. 从镜像区域分配内核内存.

2. 从非镜像区域分配用户内存. 通过将非镜像范围安排到可移动区域(ZONE_MOVABLE).


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/01/08 | Taku Izumi <izumi.taku@jp.fujitsu.com> | [mm: Introduce kernelcore=mirror option](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=342332e6a925e9ed015e5465062c38d2b86ec8f9) | 内存镜像, 提供内存可靠性分级的功能.<br>通过 UEFI BIOS(规范 2.5) 上报镜像内存(即高可靠内存 mirrored/reliable)的范围和非镜像内存(即低可靠内存)的两种内存. 内核默认使用高可靠的内存, 用 NORMAL ZONE 管理, 用户态优先使用普通(低可靠的)内存, 使用 MOVABLE ZONE 管理. | v4 ☑ 4.6-rc1 | [LKML RFC](https://lkml.org/lkml/2015/10/9/24)<br>*-*-*-*-*-*-*-* <br>[LKML v1](https://lkml.org/lkml/2015/10/15/9)<br>*-*-*-*-*-*-*-* <br>[LKML v2,0/2](https://lkml.org/lkml/2015/11/27/18)<br>*-*-*-*-*-*-*-* <br>[LKML v3,0/2](https://lkml.org/lkml/2015/12/8/836)<br>*-*-*-*-*-*-*-* <br>[LKML v4,0/2](https://lkml.org/lkml/2016/1/8/88), [PatchWork v4,0/2](https://lore.kernel.org/lkml/1452241523-19559-1-git-send-email-izumi.taku@jp.fujitsu.com) |
| 2016/06/27 | Xishi Qiu <qiuxishi@huawei.com> | [mm: mirrored memory support for page buddy allocations](https://lore.kernel.org/lkml/558E084A.60900@huawei.com) | 内存镜像的功能 | RFC v2 ☐  | [PatchWork RFC,v2,0/8](https://lore.kernel.org/lkml/558E084A.60900@huawei.com) |
| 2018/2/12 | David Rientjes <rientjes@google.com> | [mm: Introduce kernelcore=mirror option](http://lore.kernel.org/patchwork/patch/574230) | `kernelcore=` 和 `movablecore=` 都可以分别用于定义系统上 ZONE_NORMAL 和 ZONE_MOVABLE 的数量. 然而, 这需要在指定命令行时知道系统内存容量. 这个补丁引入了将 `kernelcore` 和 `movablecore` 定义为系统总内存的百分比的能力. 这对于希望将 ZONE_MOVABLE 的数量定义为系统内存的比例(而不是硬编码的字节值)的系统软件来说是很方便的. 要定义百分比, 参数的最后一个字符应该是 '%'. | v4 ☑ 4.17-rc1 | [LKML 1/2](https://lkml.org/lkml/2018/2/12/1024) |
| 2022/03/26 | mawupeng <mawupeng1@huawei.com> | [introduce mirrored memory support for arm64](https://patchwork.kernel.org/project/linux-mm/cover/20220326064632.131637-1-mawupeng1@huawei.com) | 镜像内存支持 arm64. 参见 phoronix 报道 [Huawei Working On UEFI Mirrored Memory Support For Linux AArch64](https://www.phoronix.com/scan.php?page=news_item&px=Linux-AArch64-Mirrored-Memory). | v1 ☐☑ | [LORE v1,0/9](https://lore.kernel.org/all/20220326064632.131637-1-mawupeng1@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/9](https://lore.kernel.org/r/20220414101314.1250667-1-mawupeng1@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/6](https://lore.kernel.org/r/20220607093805.1354256-1-mawupeng1@huawei.com) |
| 2022/04/19 | mawupeng <mawupeng1@huawei.com> | [Add support to relocate kernel image to mirrored region](https://patchwork.kernel.org/project/linux-mm/cover/20220419070150.254377-1-mawupeng1@huawei.com/) | 633242 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20220419070150.254377-1-mawupeng1@huawei.com) |


### 14.13.4 其他 ZONE_MOVABLE 相关
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2022/02/15 | Alistair Popple <apopple@nvidia.com> | [mm/pages_alloc.c: Don't create ZONE_MOVABLE beyond the end of anode](https://patchwork.kernel.org/project/linux-mm/patch/20220215025831.2113067-1-apopple@nvidia.com/) | 614354 | v1 ☐☑ | [PatchWork v1,0/1](https://lore.kernel.org/r/20220215025831.2113067-1-apopple@nvidia.com) |


## 14.14 SHMEM
-------

### 14.14.1 SHMEM
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/11/11  | Mina Almasry <almasrymina@google.com> | [mm/shmem: support deterministic charging of tmpfs](https://patchwork.kernel.org/project/linux-mm/patch/20211110211951.3730787-2-almasrymina@google.com) | NA | v1 ☐ | [PatchWork v2,1/4](https://patchwork.kernel.org/project/linux-mm/patch/20211110211951.3730787-2-almasrymina@google.com)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3,1/4](https://patchwork.kernel.org/project/linux-mm/patch/20211111234203.1824138-2-almasrymina@google.com) |
| 2022/01/18 | Khalid Aziz <khalid.aziz@oracle.com> | [Add support for shared PTEs across processes](https://patchwork.kernel.org/project/linux-mm/cover/cover.1642526745.git.khalid.aziz@oracle.com) | NA| v1 ☐ | [LKML RFC,0/6](https://patchwork.kernel.org/project/linux-mm/cover/cover.1642526745.git.khalid.aziz@oracle.com)<br>*-*-*-*-*-*-*-* <br>[LORE v1,0/14](https://lore.kernel.org/r/cover.1649370874.git.khalid.aziz@oracle.com) |
| 2022/02/11 | Charan Teja Kalla <quic_charante@quicinc.com> | [mm: shmem: implement POSIX_FADV_[WILL|DONT]NEED for shmem](https://patchwork.kernel.org/project/linux-mm/patch/1644572051-24091-1-git-send-email-quic_charante@quicinc.com/) | 613418 | v4 ☐☑ | [PatchWork v4,0/1](https://lore.kernel.org/r/1644572051-24091-1-git-send-email-quic_charante@quicinc.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,0/2](https://lore.kernel.org/r/cover.1646987674.git.quic_charante@quicinc.com) |
| 2022/03/14 | Xavier Roche <xavier.roche@algolia.com> | [[v4] tmpfs: support for file creation time](https://patchwork.kernel.org/project/linux-mm/patch/20220314211150.GA123458@xavier-xps/) | 623317 | v4 ☐☑ | [LORE v4,0/1](https://lore.kernel.org/r/20220314211150.GA123458@xavier-xps)|
| 2022/04/18 | Gabriel Krisman Bertazi <krisman@collabora.com> | [shmem: Allow userspace monitoring of tmpfs for lack of space.](https://patchwork.kernel.org/project/linux-mm/cover/20220322222738.182974-1-krisman@collabora.com/) | 625589 | v1 ☐☑ | [2022/03/22 LORE v1,0/3](https://lore.kernel.org/r/20220322222738.182974-1-krisman@collabora.com)<br>*-*-*-*-*-*-*-* <br>[2022/03/28 LORE v2,0/3](https://lore.kernel.org/r/20220328020443.820797-1-krisman@collabora.com)<br>*-*-*-*-*-*-*-* <br>[2022/04/18 LORE v3,0/3](https://lore.kernel.org/r/20220418213713.273050-1-krisman@collabora.com) |


### 14.14.2 Anonymous Shared Memory-Ashmem
-------

[Bringing Android closer to the mainline](https://lwn.net/Articles/472984)


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2011/12/20 | Robert Love <rlove@google.com> | [ashmem: Anonymous shared memory subsystem](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=11980c2ac4ccfad21a5f8ee9e12059f1e687bb40) | Android匿名共享内存(Ashmem) | v1 ☐☑✓ | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=11980c2ac4ccfad21a5f8ee9e12059f1e687bb40) |
| 2022/03/15 | Christoph Hellwig <hch@lst.de> | [staging: remove ashmem](https://patchwork.kernel.org/project/linux-mm/patch/20220315123457.2354812-1-hch@lst.de) | ashmem 的主线替代品是 memfd, 因此从 `drivers/staging` 中删除遗留代码. | v1 ☐☑ | [LORE v1,0/1](https://lore.kernel.org/r/20220315123457.2354812-1-hch@lst.de) |


## 14.15 Document
-------

[Improving memory-management documentation](https://lwn.net/Articles/894374)

[都 2022 年了, 这 20 篇 Linux 内存管理的期刊论文, 你读了吗？](https://zhuanlan.zhihu.com/p/450826949)


# 15 测试
-------

## 15.1 Intel® Memory Latency Checker (Intel® MLC)
-------

INTEL MLC 可以测量出机器的内存访问延迟和带宽, 并且可以观察出它们是如何随着机器负载的增加而变化的. Intel 的处理器有一些内存预取功能, 可能会影响测试结果.

[INTEL MLC (Memory Latency Checker) 介绍](https://zhuanlan.zhihu.com/p/359823092)

[MLC 内存测试结果解读到 CPU 架构设计分析](https://zhuanlan.zhihu.com/p/447936509)

[Intel® Memory Latency Checker](https://www.intel.com/content/www/us/en/developer/articles/tool/intelr-memory-latency-checker.html)

---

**引用:**

<div id="ref-anchor-1"></div>
- [1] [Single UNIX Specification](https://en.wikipedia.org/wiki/Single_UNIX_Specification%23Non-registered_Unix-like_systems)

<div id="ref-anchor-2"></div>
- [2] [POSIX 关于调度规范的文档](http://nicolas.navet.eu/publi/SlidesPosixKoblenz.pdf)

<div id="ref-anchor-3"></div>
- [3] [Towards Linux 2.6](https://link.zhihu.com/?target=http%3A//www.informatica.co.cr/linux-scalability/research/2003/0923.html)

<div id="ref-anchor-4"></div>
- [4] [Linux内核发布模式与开发组织模式(1)](https://link.zhihu.com/?target=http%3A//larmbr.com/2013/11/02/Linux-kernel-release-process-and-development-dictator-%26-lieutenant-system_1)

<div id="ref-anchor-5"></div>
- [5] IBM developworks 上有一篇综述文章, 值得一读 :[Linux 调度器发展简述](https://link.zhihu.com/?target=http%3A//www.ibm.com/developerworks/cn/linux/l-cn-scheduler)

<div id="ref-anchor-6"></div>
- [6] [CFS group scheduling [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/240474)

<div id="ref-anchor-7"></div>
- [7] [http://lse.sourceforge.net/numa/](https://link.zhihu.com/?target=http%3A//lse.sourceforge.net/numa)

<div id="ref-anchor-8"></div>
- [8] [CFS bandwidth control [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/428230)

<div id="ref-anchor-9"></div>
- [9] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D5091faa449ee0b7d73bc296a93bca9540fc51d0a)

<div id="ref-anchor-10"></div>
- [10] [DMA模式\_百度百科](https://link.zhihu.com/?target=http%3A//baike.baidu.com/view/196502.htm)

<div id="ref-anchor-11"></div>
- [11] [进程的虚拟地址和内核中的虚拟地址有什么关系? - 詹健宇的回答](http://www.zhihu.com/question/34787574/answer/60214771)

<div id="ref-anchor-17"></div>
- [17] [kernel 3.10内核源码分析--TLB相关--TLB概念、flush、TLB lazy模式-humjb\_1983-ChinaUnix博客](https://link.zhihu.com/?target=http%3A//blog.chinaunix.net/uid-14528823-id-4808877.html)

<div id="ref-anchor-18"></div>
- [18] [Toward improved page replacement[LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/226756)

<div id="ref-anchor-20"></div>
- [20] [The state of the pageout scalability patches [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/286472)

<div id="ref-anchor-21"></div>
- [21] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D894bc310419ac95f4fa4142dc364401a7e607f65)

<div id="ref-anchor-22"></div>
- [22] [Being nicer to executable pages [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/333742)

<div id="ref-anchor-23"></div>
- [23] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D8cab4754d24a0f2e05920170c845bd84472814c6)

<div id="ref-anchor-24"></div>
- [24] [Better active/inactive list balancing [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/495543)

<div id="ref-anchor-29"></div>
- [29] [On-demand readahead [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/235164)

<div id="ref-anchor-30"></div>
- [30] [Transparent huge pages in 2.6.38 [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/423584)

<div id="ref-anchor-32"></div>
- [32] [transcendent memory for Linux [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/338098)

<div id="ref-anchor-33"></div>
- [33] [linux kernel monkey log](https://link.zhihu.com/?target=http%3A//www.kroah.com/log/linux/linux-staging-update.html)

<div id="ref-anchor-34"></div>
- [34] [zcache: a compressed page cache [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/397574)

<div id="ref-anchor-36"></div>
- [36] [Linux-Kernel Archive: Linux 2.6.0](https://link.zhihu.com/?target=http%3A//lkml.iu.edu/hypermail/linux/kernel/0312.2/0348.html)

<div id="ref-anchor-37"></div>
- [37]抢占支持的引入时间: [https://www.kernel.org/pub/linux/kernel/v2.5/ChangeLog-2.5.4](https://link.zhihu.com/?target=https%3A//www.kernel.org/pub/linux/kernel/v2.5/ChangeLog-2.5.4)

<div id="ref-anchor-38"></div>
- [38] [RAM is 100 Thousand Times Faster than Disk for Database Access](https://link.zhihu.com/?target=http%3A//www.directionsmag.com/entry/ram-is-100-thousand-times-faster-than-disk-for-database-access/123964)

<div id="ref-anchor-39"></div>
- [39] [http://www.uefi.org/sites/default/files/resources/ACPI\_6.0.pdf](https://link.zhihu.com/?target=http%3A//www.uefi.org/sites/default/files/resources/ACPI_6.0.pdf)

<div id="ref-anchor-40"></div>
- [40] [Injecting faults into the kernel [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/209257)

<div id="ref-anchor-47"></div>
- [47] [Fast interprocess messaging [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/405346)



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
