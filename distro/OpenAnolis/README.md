2   **OpenAnolis cloud-kernel 分析**
=====================



- [x] [Alibaba Cloud Linux 2](https://help.aliyun.com/document_detail/154950.html)
- [x] [Alibaba Cloud Linux 2功能和接口概述](https://help.aliyun.com/document_detail/177687.html)


如果您对Linux的内核系统有一定的了解，并且有需求需要使用Linux内核功能，您可以依据下表提供的Alibaba Cloud Linux 2的内核功能与接口进行相关操作. 

| 文档链接 | 说明 |
|:-------:|:----:|
| 启用cgroup writeback功能 | Alibaba Cloud Linux 2在内核版本4.19.36-12.al7中，对内核接口cgroup v1新增了控制群组回写(cgroup writeback)功能. 该功能使您在使用内核接口cgroup v1时，可以对缓存异步I/O (Buffered I/O) 进行限速.  |
| blk-iocost权重限速  | Alibaba Cloud Linux 2在内核版本4.19.81-17.al7.x86_64开始支持基于成本模型(cost model)的权重限速功能，即blk-iocost功能. 该功能是对内核中IO子系统(blkcg)基于权重的磁盘限速功能的进一步完善.  |
| 在cgroup v1接口开启PSI功能 | Alibaba Cloud Linux 2在内核版本4.19.81-17.al7中为cgroup v1接口提供了PSI功能. PSI(Pressure Stall Information)是一个可以监控CPU、内存及IO性能异常的内核功能.  |
| 修改TCP TIME-WAIT超时时间 | 在Linux的内核中，TCP/IP协议的TIME-WAIT状态持续60秒且无法修改. 但在某些场景下，例如TCP负载过高时，适当调小该值有助于提升网络性能. 因此Alibaba Cloud Linux 2在内核版本4.19.43-13.al7新增内核接口，用于修改TCP TIME-WAIT超时时间.  |
| Block IO限流增强监控接口 | 为了更方便地监控Linux block IO限流，Alibaba Cloud Linux 2在内核版本4.19.81-17.al7增加相关接口，用于增强block IO限流的监控统计能力.  |
| JBD2优化接口 | JBD2作为ext4文件系统的内核线程，在使用过程中常会遇到影子状态(BH_Shadow)，影响系统性能. 为解决使用JBD2过程中出现的异常，Alibaba Cloud Linux 2在内核版本4.19.81-17.al7对JBD2进行了优化.  |
| 跨目录配额创建硬链接 | 默认情况下，ext4文件系统中存在约束，不允许跨目录配额创建硬链接. 但在实际中，某些特定场景有创建硬链接的需求，因此Alibaba Cloud Linux 2提供定制接口，该接口能够绕过ext4文件系统中的约束，实现跨目录配额创建硬链接.  |
| 追踪IO时延 | Alibaba Cloud Linux 2优化了IO时延分析工具iostat的原始数据来源/proc/diskstats接口，增加了对设备侧的读、写及特殊IO(discard)等耗时的统计，此外还提供了一个方便追踪IO时延的工具bcc. |
| 检测文件系统和块层的IO hang | IO hang是指在系统运行过程中，因某些IO耗时过长而引起的系统不稳定甚至宕机. 为了准确检测出IO hang，Alibaba Cloud Linux 2扩展核心数据结构，增加了在较小的系统开销下，快速定位并检测IO hang的功能.  |
| Memcg全局最低水位线分级 | Alibaba Cloud Linux 2新增了memcg全局最低水位线分级功能. 在global wmark_min的基础上，将资源消耗型任务的global wmark_min上移，使其提前进入直接内存回收. 将时延敏感型业务的global wmark_min下移，使其尽量避免直接内存回收. 这样当资源消耗型任务瞬间申请大量内存的时候，会通过上移的global wmark_min将其短时间抑制，避免时延敏感型业务发生直接内存回收. 等待全局kswapd回收一定量的内存后，再解除资源消耗型任务的短时间抑制. |
| Memcg后台异步回收 | Alibaba Cloud Linux 2增加了memcg粒度的后台异步回收功能. 该功能的实现不同于全局kswapd内核线程的实现，并没有创建对应的memcg kswapd内核线程，而是采用了workqueue机制来实现.  |
| cgroup v1接口支持memcg QoS功能 | 内存子系统服务质量(memcg QoS)可以用来控制内存子系统(memcg)的内存使用量的保证(锁定)与限制. Alibaba Cloud Linux 2在4.19.91-18.al7内核版本，新增cgroup v1接口支持memcg QoS的相关功能.  |
| Memcg Exstat功能  | Alibaba Cloud Linux 2在4.19.91-18.al7内核版本开始支持的Memcg Exstat(Extend/Extra)功能.  |
| TCP-RT功能的配置说明 | Alibaba Cloud Linux 2在内核版本kernel-4.19.91-21.al7开始支持TCP层的服务监控功能(TCP-RT). |


# 1 SCHEDULER
-------

## 1 SCHED_SLI
-------


```cpp
git log --oneline | grep alinux | grep -E "sched|cpuacct"
```

```cpp
06d7286d7c8e alinux: sched: add kconfig SCHED_SLI
```

## 1.2 Latency Histograms
-------

```cpp
echo 1 > /proc/cpusli/sched_lat_enabled

mount -t tmpfs cgroup_root /sys/fs/cgroup
mkdir -p /sys/fs/cgroup/cpu,cpuacct
mount -t cgroup -o cpu,cpuacct  cpu,cpuacct /sys/fs/cgroup/cpu,cpuacct
```

cpuacct 中引入了一些调度相关的 latency 的统计信息, 并用 histograms 的方式输出.

| 接口 | 描述 |  实现思路 | commit |
|:---:|:----:|:-------:|:-------:|
| cpuacct.wait_latency | 在就绪队列里的等待时间 | 统计方式同 wait_sum/wait_start 等计数器. enqueue_entity 入队时开始计时, 出队时结束计时(set_next_entity 准备投入运行时, 也会出队.)| [alinux: sched: Introduce cfs scheduling latency histograms](https://github.com/alibaba/cloud-kernel/commit/76d98609f83244bdc5f3484945a47cc43d509f64)  |
| cpuacct.cgroup_wait_latency | 在就绪队列里的等待时间 | 统计方式同 wait_sum/wait_start 等计数器. enqueue_entity 入队时开始计时, 出队时结束计时(set_next_entity 准备投入运行时, 也会出队.)| [alinux: sched: Add cgroup's scheduling latency histograms](https://github.com/alibaba/cloud-kernel/commit/6dbaddaa480a315f1fd5d66d16c0b3a0c3dd25ea) |
| cpuacct.block_latency/cpuacct.ioblock_latency | 统计进程 D 状态阻塞的时间(其中 ioblock_latency 统计因为 I/O 阻塞的等待时间) | 更新方式 block_latency 同 block_start, ioblock_latency 同 iowait_sum.<br>进程出队 update_stats_dequeue 开始计时, 进程入队的时候 update_stats_enqueue_sleeper 停止计时 |[alinux: sched: Add cgroup-level blocked time histograms](https://github.com/alibaba/cloud-kernel/commit/a055ee2ce27b61864adf67fe2ffc426b89913b89) |


```cpp
ab81d2d9f745 alinux: sched: Add cpu_stress to show system-wide task waiting
c7462cfec178 alinux: sched: Fix wrong cpuacct_update_latency declaration

fa418988c52e alinux: sched: Finer grain of sched latency
2abfd07b1c1a alinux: sched: Add "nr" to sched latency histogram
6dbaddaa480a alinux: sched: Add cgroup's scheduling latency histograms
a055ee2ce27b alinux: sched: Add cgroup-level blocked time histograms
76d98609f832 alinux: sched: Introduce cfs scheduling latency histograms
bcaf8afd6270 alinux: sched: Add switch for scheduler_tick load tracking
bb48b716f496 alinux: sched: Add switch for update_blocked_averages
```





## 1.3 CPUACCT STAT
-------

cpuacct 中引入的 proc_stat 统计了 CGROUP 分组中进程的 CPU 占用/空闲等统计信息

| 接口 | 描述 |  实现思路 | commit |
|:---:|:----:|:-------:|:-------:|
| cpuacct.proc_stat | cpuacct 的详细统计信息 | NA | [commit 9be0ac2bcbaa ("alinux: cpuacct: export cpuacct.proc_stat interface")](https://github.com/alibaba/cloud-kernel/commit/9be0ac2bcbaa1831196a88f5e81ea162811f3418)  |

```cpp
1c5ab7a7f5d9 alinux: sched: Fix %steal in cpuacct.proc_stat in guest OS
1607a4855848 alinux: sched: Fix regression caused by nr_uninterruptible
1e0cec0b58c7 alinux: sched: make SCHED_SLI dependent on FAIR_GROUP_SCHED
5be663e3cfcf alinux: sched: Fix p->cpu build error on aarch64
435d7069244d alinux: sched: Fix nr_migrations compile errors
9e7b35d6d84e alinux: sched: Introduce per-cgroup iowait accounting
c75529800bf2 alinux: sched: Introduce per-cgroup steal accounting
61e5885959be alinux: sched: Introduce per-cgroup idle accounting
965d75d3785c alinux: cpuacct: make cpuacct record nr_migrations
9410d3142882 alinux: cpuacct: Export nr_running & nr_uninterruptible
004b3ba9ebd3 alinux: sched/cputime: Fix guest cputime of cpuacct.proc_stat
8ab45c97b581 alinux: cpuacct/proc_stat: Consider isolcpus
9be0ac2bcbaa alinux: cpuacct: export cpuacct.proc_stat interface
36da4fe9851a alinux: sched: Maintain "nr_uninterruptible" in runqueue
06d7286d7c8e alinux: sched: add kconfig SCHED_SLI
```

## 1.5 remove cpu_load array
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2021/01/06 | Vincent Guittot | [sched: Remove per rq load array](https://lore.kernel.org/patchwork/cover/1079333) | 自 LB_BIAS 被禁用之后, 调度器只使用 rq->cpu_load[0] 作为cpu负载值, 因此 cpu_load 这个数组的其他之其实没意义了, 直接去掉了. 注意这对 load_balance 的调优是有一定影响的, 之前 sched_domain 中可以通过 sysctl 接口修改比较负载使用的 index, 这些 index 对应的 cpu_load 数组的下标. 干掉了这个数组, 那么这些 sysctl 也就没必要了 | v2 ☑ 5.10-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1079333) |


```cpp
980d06cfa010 sched/fair: Remove sgs->sum_weighted_load
b21904d4a87f sched/core: Remove sd->*_idx
c3055505bb6f sched/core: Remove rq->cpu_load[]
606eca9543a2 sched/debug: Remove sd->*_idx range on sysctl
d24a239d5646 sched/fair: Replace source_load() & target_load() with weighted_cpuload()
112598d686df sched/fair: Remove the rq->cpu_load[] update code
be11b02dab58 sched/fair: Remove rq->load
800bf05db86a cpuidle: menu: Remove get_loadavg() from the performance multiplier
bae5297970bb sched/fair: Disable LB_BIAS by default
```



## 1.3 PSI
-------

### 1.3.1 PSI V1
-------

```cpp
1bd8a72b1181 alinux: psi: using cpuacct_cgrp_id under CONFIG_CGROUP_CPUACCT
dc159a61c043 alinux: introduce psi_v1 boot parameter
1f49a7385032 alinux: psi: Support PSI under cgroup v1
```

```cpp
eee77c97f8c4 sched/psi: Correct overly pessimistic size calculation
39c753733baf sched/psi: Do not require setsched permission from the trigger creator
a4a4fdd978a8 sched/psi: Reduce psimon FIFO priority
```

### 1.3.2 PSI V2
-------

```cpp
c1cc09981b82 psi: introduce psi monitor
a77db4849800 kernel: cgroup: add poll file operation
12528a20abeb fs: kernfs: add poll file operation
3eb076daa6fa include/: refactor headers to allow kthread.h inclusion in psi_types.h
a310bf88c601 psi: track changed states
8b9c8c071256 psi: split update_stats into parts
6c479dfbe22f psi: rename psi fields in preparation for psi trigger addition
b967851710f2 psi: make psi_enable static
529aca9e758a psi: introduce state_mask to represent stalled psi states
92f5c2aec4ce psi: clarify the units used in pressure files
2ab1c71c198d psi: avoid divide-by-zero crash inside virtual machines
c3a04b34efeb psi: clarify the Kconfig text for the default-disable option
70f1297d3513 psi: fix aggregation idle shut-off
bcc32a75cf6b psi: fix reference to kernel commandline enable
8570f9337114 psi: make disabling/enabling easier for vendor kernels
d09e62bab8d0 kernel/sched/psi.c: simplify cgroup_move_task()
d26da1947f48 psi: cgroup support
8b2bf0799111 psi: pressure stall information for CPU, memory, and IO
```

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/04/03 | Johannes Weiner <hannes@cmpxchg.org> | [psi: pressure stall information for CPU, memory, and IO v4](https://lwn.net/Articles/815342) | NA | v4 ☑ [4.20-rc1](https://kernelnewbies.org/Linux_5.9#Memory_management) | [Patchwork](https://lore.kernel.org/patchwork/patch/978495) |

```cpp
d26da1947f48 psi: cgroup support
8b2bf0799111 psi: pressure stall information for CPU, memory, and IO
ec035638f0c2 sched: introduce this_rq_lock_irq()
420e6dea146e sched: sched.h: make rq locking and clock functions available in stats.h
3bf93774b166 sched: loadavg: make calc_load_n() public
4ca637b41664 sched: loadavg: consolidate LOAD_INT, LOAD_FRAC, CALC_LOAD
2dde6f773e87 delayacct: track delays from thrashing cache pages
e2d3e3cb0d60 mm: workingset: tell cache transitions from workingset thrashing
b027d193c877 mm: workingset: don't drop refault information prematurely
```



```cpp
abd7f977069d alinux: sched: Fix a branch prediction error in static_key
a66f5804aa67 alinux: sched: Fix compiling error without CONFIG_FAIR_GROUP_SCHED
252315eb68be alinux: sched: get_sched_lat_count_idx optimization
```


```cpp
bcaf8afd6270 alinux: sched: Add switch for scheduler_tick load tracking
bb48b716f496 alinux: sched: Add switch for update_blocked_averages
d2440c99979d alinux: sched/fair: use static load in wake_affine_weight
```


```cpp
9b83fd88733f sched: Avoid scale real weight down to zero
70a23044f6b4 sched/fair: Fix race between runtime distribution and assignment
798cfa768c74 alinux: cgroup: Fix task_css_check rcu warnings
29846134c976 alinux: config: disable CONFIG_NFS_V3_ACL and CONFIG_NFSD_V3_ACL
ac2b5c94f26e alinux: kernel: reap zombie process by specified pid
e483e6eb1bbe alinux: Fix an potential null pointer reference in dump_header
```

```cpp
8d23e73fb3fa spi: Add missing error handling for CS GPIOs
a190de9d346a spi: cadence: Fix default polarity of native chipselect
eaf88d303c4a spi: dw: Fix default polarity of native chipselect
0eb44b65a65b spi: Support high CS when using descriptors
cefd01bcd2b8 spi: dw: Convert to use CS GPIO descriptors
9d887e907728 spi: davinci: Convert to use CS GPIO descriptors
309d178ff4c7 spi: clps711x: Convert to use CS GPIO descriptors
0ac22fd7a6b9 spi: cadence: Convert to use CS GPIO descriptors
a9d0ece2e4ab spi: atmel: Convert to use CS GPIO descriptors
c9f247fa0b08 spi: ath79: Convert to use CS GPIO descriptors
7bc17db39cdd spi: Optionally use GPIO descriptors for CS GPIOs
fcdf0c5d409e spi: spi-davinci: Don't error when SPI_CS_WORD and cs_gpio
a1ae41c5e26b iio: adc: ti-ads7950: use SPI_CS_WORD to reduce CPU usage
e24e01032e26 spi: spi-davinci: Add support for SPI_CS_WORD
78c56e85abea spi: add software implementation for SPI_CS_WORD
66f9ad76fcd8 spi: add new SPI_CS_WORD flag
1ee223919a15 spi: davinci: Remove chip select GPIO pdata
```


```cpp
0f30856944d4 sched/deadline: Fix bandwidth accounting at all levels after offline migration
d29c7b8be599 x86/apic: Make apic_pending_intr_clear() more robust
f381d3d2c39c sched/core: Fix CPU controller for !RT_GROUP_SCHED
417cf53b4b85 sched/fair: Fix imbalance due to CPU affinity
7cebdfa62f22 time/tick-broadcast: Fix tick_broadcast_offline() lockdep complaint
64f3fb5d751c media: i2c: ov5640: Check for devm_gpiod_get_optional() error
```


## 2.2 burst 等
-------


```cpp
db83ac46e3f5 alinux: cpuacct: Export nr_running & nr_uninterruptible
982358c4f1f0 alinux: sched: Maintain "nr_uninterruptible" in runqueue
3a022fb07707 alinux: sched/fair: Fix CPU burst stat
a97cbf90404c alinux: sched/fair: Introduce init buffer into CFS burst
95527c5751fa alinux: sched: Restore upper limit of cfs_b->buffer to max_cfs_runtime
a0b0376bdbdc alinux: sched: Add document for burstable CFS bandwidth control
744d0dc11559 alinux: sched: Add cfs bandwidth burst statistics
739156b53e75 alinux: sched: Make CFS bandwidth controller burstable
d9ba6d0880e3 alinux: sched: Introduce primitives for CFS bandwidth burst
9d168f216486 alinux: sched: Defend cfs and rt bandwidth against overflow
```




# 2 MEMORY
-------



## 2.2 fast_cow
-------


```cpp
56a432f556a5 alinux: mm: thp: add fast_cow switch
```


```cpp

ff29a8461033 memcg, oom: check memcg margin for parallel oom
9cef2ea5e777 mm: memcontrol: don't count limit-setting reclaim as memory pressure
5feea4f9f1ee mm, memcg: unify reclaim retry limits with page allocator
e824fc708bae mm, memcg: reclaim more aggressively before high allocator throttling
c6529f4413ae mm: memcontrol: restore proper dirty throttling when memory.high changes
488ef785fbc2 mm: memcontrol: avoid workload stalls when lowering memory.high
7697c9343a35 mm: memcontrol: try harder to set a new memory.high
6e5a060130c9 mm/swapfile.c: swap_next should increase position index
7a676578239f mm, THP, swap: fix allocating cluster for swapfile by mistake
fd465b0b447a mm: fix swap cache node allocation mask
c6580892cdf8 mm/swap_state: fix a data race in swapin_nr_pages
c5d5eab5e376 mm, compaction: make capture control handling safe wrt interrupts
```

## 2.3 workingset protection/detection on the anonymous LRU list
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/04/03 | Joonsoo Kim <iamjoonsoo.kim@lge.com> | [workingset protection/detection on the anonymous LRU list](https://lwn.net/Articles/815342) | 实现对匿名 LRU 页面列表的工作集保护和检测. 在这里我们关心的是它将新创建或交换的匿名页面放到 inactive LRU list 中, 只有当它们被足够引用时才会被提升到活动列表. | v5 ☑ [5.9-rc1](https://kernelnewbies.org/Linux_5.9#Memory_management) | [Patchwork v7](https://lore.kernel.org/patchwork/patch/1278082)<br>*-*-*-*-*-*-*-*<br>[ZhiHu](https://zhuanlan.zhihu.com/p/113220105)<br>*-*-*-*-*-*-*-*<br>[关键 commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b518154e59aab3ad0780a169c5cc84bd4ee4357e) |


```cpp
0d39b86c4866 mm/memcontrol.c: fix memory.stat item ordering
92cd0e289a6b mm: memcontrol: fix missing suffix of workingset_restore
4f4e3a58d84d mm: remove activate_page() from unuse_pte()
60cbec3171fc mm/vmscan: restore active/inactive ratio for anonymous LRU
603ef0fbc186 mm/swap: implement workingset detection for anonymous LRU
d3b586512756 mm/swapcache: support to handle the shadow entries
21ea0c7e45e6 mm/workingset: prepare the workingset detection infrastructure for anon LRU
c3ade74e23bb mm/vmscan: protect the workingset on anonymous LRU
50186fde1fde mm/vmscan: make active/inactive ratio as 1:1 for anon lru
75f61106fbd1 mm, memcg: add workingset_restore in memory.stat
```

##


```cpp
c3f32f75ac28 mm/memory: fix IO cost for anonymous page
```

## mm: balance LRU lists based on relative thrashing v2
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2020/05/20 | Johannes Weiner <hannes@cmpxchg.org> | [mm: balance LRU lists based on relative thrashing v2](https://lore.kernel.org/patchwork/cover/1245255) | 基于相对抖动平衡 LRU 列表(重新实现了页面缓存和匿名页面之间的 LRU 平衡, 以便更好地与快速随机 IO 交换设备一起工作). : 在交换和缓存回收之间平衡的回收代码试图仅基于内存引用模式预测可能的重用. 随着时间的推移, 平衡代码已经被调优到一个点, 即它主要用于页面缓存, 并推迟交换, 直到 VM 处于显著的内存压力之下. 因为 commit a528910e12ec Linux 有精确的故障 IO 跟踪-回收错误页面的最终代价. 这允许我们使用基于 IO 成本的平衡模型, 当缓存发生抖动时, 这种模型更积极地扫描匿名内存, 同时能够避免不必要的交换风暴. | v1 ☑ [5.8-rc1](https://kernelnewbies.org/Linux_5.8#Memory_management) | [PatchWork v1](https://lore.kernel.org/patchwork/cover/685701)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/cover/1245255) |

```cpp
2c43e0162ddb mm/swap: fix for "mm: workingset: age nonresident information alongside anonymous pages"
6f4e186fae1a mm: workingset: age nonresident information alongside anonymous pages

bf7023bd718a mm: vmscan: limit the range of LRU type balancing
6c3b7eb38700 mm: vmscan: reclaim writepage is IO cost
65daa60b0781 mm: vmscan: determine anon/file pressure balance at the reclaim root
4814da9abe6c mm: balance LRU lists based on relative thrashing
511d0cc6e8a6 mm: only count actual rotations as LRU reclaim cost
65316ea462b7 mm: deactivations shouldn't bias the LRU balance
2f685e021dd6 mm: base LRU balancing on an explicit cost model
c64c253a6886 mm: vmscan: drop unnecessary div0 avoidance rounding in get_scan_count()
896b7290f99e mm: remove use-once cache bias from LRU balancing
901074611aa7 mm: workingset: let cache workingset challenge anon
3dc69d3351ef mm: fold and remove lru_cache_add_anon() and lru_cache_add_file()
82a1c34bf83d mm: allow swappiness that prefers reclaiming anon over the file workingset
17915bb6dfea mm: keep separate anon and file statistics on page reclaim activity
c9fe54195999 mm: fix LRU balancing effect of new transparent huge pages
```


## task #30476527
-------


### mm: fix page aging across multiple cgroups
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2019/11/07 | Johannes Weiner <hannes@cmpxchg.org> | [mm: fix page aging across multiple cgroups](https://lore.kernel.org/patchwork/cover/1150211) | NA | v2 ☑ [5.5-rc1](https://kernelnewbies.org/Linux_5.5#Memory_management) | [PatchWork v1](https://lore.kernel.org/patchwork/cover/1150211) |

```cpp
af77f2589a48 mm: vmscan: enforce inactive:active ratio at the reclaim root
293ca5d0e0df mm: vmscan: detect file thrashing at the reclaim root
0f68ac9a7339 mm: vmscan: move file exhaustion detection to the node level
```

### mm: vmscan: cgroup-related cleanups
-------

一组 cleanup 也合入了, 我表示深刻的不理解. cleanup 没改变逻辑, 只是上代码更清晰. 合入的原因是啥

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2019/11/07 | Johannes Weiner <hannes@cmpxchg.org> | [mm: fix page aging across multiple cgroups](https://lore.kernel.org/patchwork/cover/1142997) | 对回收代码与cgroups的交互进行了一些清理. 它们不应该改变任何行为, 而只是让实现更容易理解和使用. | v1 ☑ [5.5-rc1](https://kernelnewbies.org/Linux_5.5#Memory_management) | [PatchWork v1](https://lore.kernel.org/patchwork/cover/1142997) |


```cpp
9f124d740916 mm: vmscan: harmonize writeback congestion tracking for nodes & memcgs
47b5bb94e110 mm: vmscan: turn shrink_node_memcg() into shrink_lruvec()
d144427dbe3d mm: vmscan: split shrink_node() into node part and memcgs part
ae8fe4582089 mm: vmscan: replace shrink_node() loop with a retry jump
ae1afb31b51e mm: vmscan: naming fixes: global_reclaim() and sane_reclaim()
c5e78e07aaa1 mm: vmscan: move inactive_list_is_low() swap check to the caller
70cb7ad4e25f mm: clean up and clarify lruvec lookup procedure
385269e8de19 mm: vmscan: simplify lruvec_lru_size()
```

### cleanup
-------


又一组 cleanup, 但是只合入了其中的 2 个补丁.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2019/02/28 | Andrey Ryabinin <aryabinin@virtuozzo.com> | [mm/workingset: remove unused @mapping argument in workingset_eviction()](https://lore.kernel.org/patchwork/cover/1046511) | NA | v1 ☑ [5.5-rc1](https://kernelnewbies.org/Linux_5.5#Memory_management) | [PatchWork v2](https://lore.kernel.org/patchwork/cover/1046511) |


```cpp
f2116b7f9274 mm/workingset: remove unused @mapping argument in workingset_eviction()
4f466c091b62 mm/vmscan: remove unused lru_pages argument
```

### fix
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2019/01/29 | Johannes Weiner <hannes@cmpxchg.org> | [ mm: vmscan: do not iterate all mem cgroups for global direct reclaim](https://lore.kernel.org/patchwork/cover/1036823) | NA | v1 ☑ [5.5-rc1](https://kernelnewbies.org/Linux_5.5#Memory_management) | [PatchWork v1](https://lore.kernel.org/patchwork/cover/1036823) |
| 2019/08/12 | Johannes Weiner <hannes@cmpxchg.org> | [mm: vmscan: do not share cgroup iteration between reclaimers](https://lore.kernel.org/patchwork/cover/1114038) | NA | v1 ☑ [5.5-rc1](https://kernelnewbies.org/Linux_5.5#Memory_management) | [PatchWork v1](https://lore.kernel.org/patchwork/cover/1114038) |

```cpp
91ae8e0cd768 mm: vmscan: do not share cgroup iteration between reclaimers
1a8869efe950 mm: vmscan: do not iterate all mem cgroups for global direct reclaim
54c1b36ed43a mm/memcontrol: update lruvec counters in mem_cgroup_move_account
```

## task #3047633(enhance & fix memcg)
-------



### fix
-------


```cpp
f14b2eb60240 mm, memcg: partially revert "mm/memcontrol.c: keep local VM counters in sync with the hierarchical ones"
5ca35dcb4cc1 mm/memcontrol.c: keep local VM counters in sync with the hierarchical ones
a93d486591a7 mm: memcg: get number of pages on the LRU list in memcgroup base on lru_zone_size
7ff18d6488b7 mm: memcontrol: fix NULL-ptr deref in percpu stats flush
7c2fe63d509f mm: memcontrol: fix percpu vmstats and vmevents flush
bc0030faffe9 mm: memcontrol: flush percpu vmevents before releasing memcg
2ddc019313c5 mm: memcontrol: flush percpu vmstats before releasing memcg
7f75ebe21937 mm/memcontrol: fix wrong statistics in memory.stat
e7643c241159 mm: memcontrol: don't batch updates of local VM stats and events
```

###
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2019/04/12 | Andrey Ryabinin <aryabinin@virtuozzo.com> | [mm: memcontrol: memory.stat cost & correctness](https://lore.kernel.org/patchwork/cover/1061078) | NA | v1 ☑ [5.2-rc1](https://kernelnewbies.org/Linux_5.2#Memory_management) | [PatchWork v2](https://lore.kernel.org/patchwork/cover/1061078) |


```cpp
97276f710344 mm: memcontrol: fix NUMA round-robin reclaim at intermediate level
d9d834314371 mm: memcontrol: fix recursive statistics correctness & scalabilty
b82ffa7c06b9 mm: memcontrol: move stat/event counting functions out-of-line
c24d85c9806d mm: memcontrol: make cgroup stats and events query API explicitly local
98ab53a614a mm, memcg: rename ambiguously named memory.stat counters and functions
```

###
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2019/02/28 | Johannes Weiner <hannes@cmpxchg.org> | [mm: memcontrol: track LRU counts in the vmstats array](https://lore.kernel.org/patchwork/cover/1046756) | NA | v1 ☑ [5.2-rc1](https://kernelnewbies.org/Linux_5.2#Memory_management) | [PatchWork v2](https://lore.kernel.org/patchwork/cover/1046756) |

```cpp
https://lore.kernel.org/patchwork/patch/1046756
1009eea36499 mm: memcontrol: quarantine the mem_cgroup_[node_]nr_lru_pages() API
b254f736aa06 mm: memcontrol: push down mem_cgroup_nr_lru_pages()
10add36bfcea mm: memcontrol: push down mem_cgroup_node_nr_lru_pages()
98c54943f68e mm: memcontrol: replace node summing with memcg_page_state()
22682e63f389 mm: memcontrol: replace zone summing with lruvec_page_state()
af2a43b06ab6 mm: memcontrol: track LRU counts in the vmstats array
```


## task #30476868
-------

### NA
-------

```cpp
8d6fc8aaccf4 mm/memcontrol: fix OOPS inside mem_cgroup_get_nr_swap_pages()

db1ed1833f57 ksm: reinstate memcg charge on copied pages
fd3832463ad1 mm: do_swap_page(): fix up the error code
b41e5c93ed50 mm: memcontrol: correct the NR_ANON_THPS counter of hierarchical memcg
```

### mm: memcontrol: charge swapin pages on instantiation
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2020/08/20 | Johannes Weiner <hannes@cmpxchg.org> | [mm: memcontrol: charge swapin pages on instantiation](https://lore.kernel.org/patchwork/cover/1239175) | memcg swapin 的延迟统计, 对memcg进行了修改, 使其在交换时直接对交换页统计, 而不是在出错时统计, 这可能要晚得多, 或者根本不会发生. | v2 ☑ [5.8-rc1](https://kernelnewbies.org/Linux_5.8#Memory_management) | [PatchWork v1](https://lore.kernel.org/patchwork/cover/1227833), [PatchWork v2](https://lore.kernel.org/patchwork/cover/1239175) |


```cpp
4f9bab4b3a47 mm: memcontrol: update page->mem_cgroup stability rules
5ff14606ff7e mm: memcontrol: delete unused lrucare handling
f8fed034b2fe mm: memcontrol: document the new swap control behavior
33d30bcb37fe mm: memcontrol: charge swapin pages on instantiation
8b5be32d8233 mm: memcontrol: make swap tracking an integral part of memory control
0191de877275 mm: memcontrol: prepare swap controller setup for integration
bfcc7afc681b mm: memcontrol: drop unused try/commit/cancel charge API
5c9f51f53e5b mm: memcontrol: convert anon and file-thp to new mem_cgroup_charge() API
4baf8591cf70 mm: memcontrol: switch to native NR_ANON_THPS counter
4b556f45dbc7 mm: memcontrol: switch to native NR_ANON_MAPPED counter
550ce3abf64d mm: memcontrol: switch to native NR_FILE_PAGES and NR_SHMEM counters
4acc4d03b2f1 mm: memcontrol: prepare cgroup vmstat infrastructure for native anon counters
aaf017e0abbe mm: memcontrol: prepare move_account for removal of private page type counters
c85ee7938537 mm: memcontrol: prepare uncharging for removal of private page type counters
ab4551efe9d9 mm: memcontrol: convert page cache to a new mem_cgroup_charge() API
0c1947cb3ad1 mm: memcontrol: move out cgroup swaprate throttling
ec9fe1cae515 mm: shmem: remove rare optimization when swapin races with hole punching
d57c8cd555f8 mm: memcontrol: drop @compound parameter from memcg charging API
c5cfeb3e2b18 mm: memcontrol: fix stat-corrupting race in charge moving
```

## task #31315199
-------

```cpp
737b01dfdb63 alinux: mm: fix an global-out-of-bounds in __do_proc_doulongvec_minmax
f5ac90f84be8 alinux: mm: add an interface to adjust the penalty time dynamically
f8a6ae902811 alinux: mm: support swap.high for cgroup v1
```


### memcg: Slow down swap allocation as the available space gets depleted
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2020/05/27 | Jakub Kicinski <kuba@kernel.org> | [memcg: Slow down swap allocation as the available space gets depleted](https://patchwork.kernel.org/project/linux-mm/cover/20200527195846.102707-1-kuba@kernel.org/) | NA | v6 ☑ [5.8-rc1](https://kernelnewbies.org/Linux_5.8#Memory_management) | [PatchWork v6](https://patchwork.kernel.org/project/linux-mm/cover/20200527195846.102707-1-kuba@kernel.org) |


```CPP
ad06f811534b mm/memcg: automatically penalize tasks with high swap use
e1a2c040dcd7 mm/memcg: move cgroup high memory limit setting into struct page_counter
49b0d8d106e8 mm/memcg: move penalty delay clamping out of calculate_high_delay()
408975905823 mm/memcg: prepare for swap over-high accounting and penalty calculation
```

### mm, memcg: cgroup v2 tunable load/store tearing fixes
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2020/03/12 | Jakub Kicinski <kuba@kernel.org> | [memcg: Slow down swap allocation as the available space gets depleted](https://lore.kernel.org/patchwork/cover/1208947) | NA | v1 ☑ [5.8-rc1](https://kernelnewbies.org/Linux_5.8#Memory_management) | [PatchWork v1](https://lore.kernel.org/patchwork/cover/1208947) |


```cpp
e8144debf849 mm, memcg: prevent memory.swap.max load tearing
8ef892f0023b mm, memcg: prevent memory.min load/store tearing
1793bc1dd739 mm, memcg: prevent memory.low load/store tearing
4b9fe46dcedf mm, memcg: prevent memory.max load tearing
e0a3c6cf7328 mm, memcg: prevent memory.high load/store tearing
```


```cpp
https://lore.kernel.org/patchwork/patch/1218448
1e005f35c32f mm, memcg: do not high throttle allocators based on wraparound

https://lore.kernel.org/patchwork/patch/1208907
b7671aa6d7ef mm, memcg: bypass high reclaim iteration for cgroup hierarchy root

https://lore.kernel.org/patchwork/patch/1208954/
044b9665cf59 mm, memcg: throttle allocators based on ancestral memory.high
efe4ee97dd8f mm, memcg: fix corruption on 64-bit divisor in memory.high throttling

```

## task #31072503
-------



```cpp
362c19f4f21d mm: proactive compaction
```

## to #29998112
-------


```cpp
acfc4d580d12 mm: add kiocb_wait_page_queue_init() helper
20076b974a61 btrfs: flag files as supporting buffered async reads
4001914fbf5d xfs: flag files as supporting buffered async reads
337bc770492d block: flag block devices as supporting IOCB_WAITQ
8512e751229e fs: add FMODE_BUF_RASYNC
079458e82cae mm: support async buffered reads in generic_file_buffered_read()
006b5f9cbd87 mm: add support for async page locking
9660b59fb7f5 mm: abstract out wake_page_match() from wake_page_function()
f0e3c8506f52 mm: allow read-ahead with IOCB_NOWAIT set
```


## to #29931646
-------

```cpp
63de663eb5b9 mm, page_alloc: skip ->waternark_boost for atomic order-0 allocations

b893a7d56134 alinux: mm: completely disable swapout with negative swappiness
```

## task #25182720
-------

```cpp
7e6914774bc8 mm: do not allow MADV_PAGEOUT for CoW pages
```

## to #26782094
-------

```cpp
7d6cb94f148e alinux: mm: Pin code section of process in memory
```

## MEMSLI(to #26424368)
-------

```cpp
echo 1 > /proc/memsli/enabled

mount -t tmpfs cgroup_root /sys/fs/cgroup
mkdir -p /sys/fs/cgroup/memory
mount -t cgroup -o memory memory /sys/fs/cgroup/memory
```


```cpp
start_commit : 3d5ca29dd634b4628d7dc82423b2680718e6eb2a
end_commit   : a5f32c14829c2cf52ffe8a2c25fd5e089c254d9c
3d5ca29dd634 alinux: mm, memcg: optimize division operation with memsli counters
055ed63be9b6 alinux: mm, memcg: rework memsli interfaces
a2feb0da9d27 alinux: config: enable CONFIG_MEMSLI
9e58d704f63a alinux: mm, memcg: add kconfig MEMSLI
892970b760e2 alinux: mm, memcg: add memsli procfs switch interface
77663a9d54da alinux: mm, memcg: gather memsli/exstat from all possible cpus
6c4e1cc32b45 alinux: mm, memcg: account throttle over memory.high for memcg direct reclaim
ddfd4d5e8f76 alinux: mm, memcg: record latency of swapout and swapin in every memcg
390710798062 alinux: mm, memcg: account reclaim_high for memcg direct reclaim
fe673ccf92aa alinux: mm, memcg: adjust the latency probe point for memcg direct reclaim
837e53abb954 alinux: mm, memcg: rework memory latency histogram interfaces
4bec5cfe97cf alinux: mm, memcg: record latency of direct compact in every memcg
83058e75601e alinux: mm, memcg: record latency of direct reclaim in every memcg
```


## task #29077503
-------

### mm/memory_hotplug: Export generic_online_page()
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2019/09/09 | Jakub Kicinski <kuba@kernel.org> | [mm/memory_hotplug: Export generic_online_page()](https://lore.kernel.org/patchwork/cover/1125874) | NA | v1 ☑ [5.5-rc1](https://kernelnewbies.org/Linux_5.5#Memory_management) | [PatchWork v1](https://lore.kernel.org/patchwork/cover/1125874) |




```cpp
0c6a9eb5c0d9 mm/memory_hotplug: export generic_online_page()
bd6aced3785a mm/page_alloc.c: memory hotplug: free pages as higher order
5eee472829ec mm, memory_hotplug: deobfuscate migration part of offlining
a6785cdc8d2b mm, memory_hotplug: __offline_pages fix wrong locking
86f9a7e38183 mm, memory_hotplug: print reason for the offlining failure
80aa4777f63f mm/page_isolation.c: convert SKIP_HWPOISON to MEMORY_OFFLINE
5316eb6eaeb8 mm: only report isolation failures when offlining memory
59df23d6178e mm: convert PG_balloon to PG_offline
```

```cpp
bd231c594cce mm, page_alloc: reset the zone->watermark_boost early
ab70cdb0d416 mm: limit boost_watermark on small zones
fb4da0edd612 mm, vmscan: do not special-case slab reclaim when watermarks are boosted
a274182f2638 mm/page_alloc.c: fix never set ALLOC_NOFRAGMENT flag
60eb3dfc44e1 mm/page_alloc.c: avoid potential NULL pointer dereference
d5971569761b mm: do not boost watermarks to avoid fragmentation for the DISCONTIG memory model
92bebf3cf052 mm, page_alloc: fix a division by zero error when boosting watermarks v2
ba16c9c89a82 mm, page_alloc: do not wake kswapd with zone lock held
9bcadc70ec1c mm: reclaim small amounts of memory when an external fragmentation event occurs
fd98e14a7fb4 mm: use alloc_flags to record if kswapd can wake
039531d219fb mm, page_alloc: spread allocations across zones before introducing fragmentation
0b74ae708501 mm/filemap.c: don't bother dropping mmap_sem for zero size readahead
1d42b185abd1 mm: mmu_gather: remove __tlb_reset_range() for force flush
2841653a229c filemap: drop the mmap_sem for all blocking operations
d33d61675b4d filemap: kill page_cache_read usage in filemap_fault
4023e1eba37a filemap: pass vm_fault to the mmap ra helpers
7124f1ac0284 mm: unmap VM_PFNMAP mappings with optimized path
435ce5518ba1 mm: unmap VM_HUGETLB mappings with optimized path
7027c30559b9 mm: mmap: zap pages with read mmap_sem in munmap
```


```cpp
56a432f556a5 alinux: mm: thp: add fast_cow switch
e5b2cc5d25ee khugepaged: introduce 'max_ptes_shared' tunable
33b1aabe69fb thp: change CoW semantics for anon-THP
55b41d47feb2 khugepaged: allow to collapse PTE-mapped compound pages
855921474514 khugepaged: allow to collapse a page shared across fork
733e252494b4 khugepaged: drain LRU add pagevec after swapin
4adb1303f878 khugepaged: drain all LRU caches before scanning pages
ed3a7ca76842 khugepaged: do not stop collapse if less than half PTEs are referenced
98415d0cdf26 khugepaged: add self test
```


```cpp
f84e8fa05d0b alinux: add tcprt framework to kernel
4dc24f040d81 alinux: quota: fix unused label warning in dquot_load_quota_inode()
587136851af8 alinux: mm: fix undefined reference to printk_ratelimit_state
79e7c57a1833 alinux: mm: fix undefined reference to mlock_fixup
9ed6dd628860 configs: enable multipath for kernel selftests
b68e287533d2 mm: return zero_resv_unavail optimization
f85218310ce6 mm: zero remaining unavailable struct pages
```


```cpp
087f53fc12cc mm, compaction: fully assume capture is not NULL in compact_zone_order()
61d6ab62ef24 mm/compaction: add missing annotation for compact_lock_irqsave
a5c295c0d816 mm, compaction: fix wrong pfn handling in __reset_isolation_pfn()
57405fb945e0 mm/compaction.c: remove unnecessary zone parameter in isolate_migratepages()
8539ae810097 mm/compaction.c: clear total_{migrate,free}_scanned before scanning a new zone
377f424a50b2 mm: compaction: avoid 100% CPU usage during compaction when a task is killed
89c885abf8d9 mm, compaction: make sure we isolate a valid PFN
39470173d94b mm/compaction.c: correct zone boundary handling when isolating pages from a pageblock
00bb2964457f mm/compaction.c: fix an undefined behaviour
7f458a1c7a1f mm/compaction.c: abort search if isolation fails
19a4cd3c3a68 mm, page_alloc: always use a captured page regardless of compaction result
99d1c41066f5 mm/compaction.c: correct zone boundary handling when resetting pageblock skip hints
35d915bef612 mm, compaction: capture a page under direct compaction
1edbee6126fb mm, compaction: be selective about what pageblocks to clear skip hints
2de4a76844c6 mm, compaction: sample pageblocks for free pages
0752a8acba91 mm, compaction: round-robin the order while searching the free lists for a target
c52fd1473e8e mm, compaction: reduce premature advancement of the migration target scanner
e73bb390bbf2 mm, compaction: do not consider a need to reschedule as contention
4027a83a9b22 mm, compaction: rework compact_should_abort as compact_check_resched
a9e7073514a4 mm, compaction: keep cached migration PFNs synced for unusable pageblocks
767185c9a8ee mm, compaction: check early for huge pages encountered by the migration scanner
4bfbd121c88d mm, compaction: finish pageblock scanning on contention
1586467d80d8 mm, compaction: avoid rescanning the same pageblock multiple times
8eaac3573ade mm, compaction: use free lists to quickly locate a migration target
87f9b9a34cd5 mm, compaction: keep migration source private to a single compaction instance
255de7acbb96 mm, compaction: use free lists to quickly locate a migration source
a7ff78ca2004 mm, compaction: ignore the fragmentation avoidance boost for isolation and compaction
d3a528a25601 mm, compaction: always finish scanning of a full pageblock
bed49b9825e5 mm, migrate: immediately fail migration of a page with no migration handler
47ee435b7098 mm, compaction: rename map_pages to split_map_pages
66dfb67e44d4 mm, compaction: remove unnecessary zone parameter in some instances
3e1a250cf241 mm, compaction: remove last_migrated_pfn from compact_control
7f7d6863cff1 mm, compaction: rearrange compact_control
42fbe2407a12 mm, compaction: shrink compact_control
1bb2b85e5606 mm: move zone watermark accesses behind an accessor
b725e2580207 alinux: Revert "mm/compaction.c: clear total_{migrate,free}_scanned before scanning a new zone"
```


```cpp
fc0a63d8314c mm/page_reporting: add free page reporting documentation
d2932bdb681c mm/page_reporting: add budget limit on how many pages can be reported per pass
05c415bcf6f1 mm/page_reporting: rotate reported pages to the tail of the list
671b4f344c85 virtio-balloon: add support for providing free page reports to host
8256460e42d7 virtio-balloon: pull page poisoning config out of free page hinting
2b197730c24f mm: introduce Reported pages
06df03eda8de mm: add function __putback_isolated_page
999c6c60ed70 mm: use zone and order instead of free area in free_list manipulators
54c14b30a5e6 mm: move buddy list manipulations into helpers
```

```cpp
bdfadacea5f2 mm: fix tick timer stall during deferred page init
27393b9b965a alinux: mm, memcg: export workingset counters on memcg v1
350f8ab8b835 bpf/sockmap: Read psock ingress_msg before sk_receive_queue
4a5d2b59c6eb alinux: pci/iohub-sriov: Support for Alibaba PCIe IOHub SRIOV
ec661706b1c9 alinux: mm, memcg: abort priority oom if with oom victim
2061acd6c236 alinux: mm, memcg: account number of processes in the css
7bf04cbb80f0 mm: memcontrol: use CSS_TASK_ITER_PROCS at mem_cgroup_scan_tasks()
fb4c5ea64934 alinux: mm, memcg: fix soft lockup in priority oom
```

40969475355a alinux: mm, memcg: record latency of memcg wmark reclaim

```cpp
5c1675fcd785 tools headers uapi: Sync asm-generic/mman-common.h with the kernel
88ec97eca1f9 mm: fix trying to reclaim unevictable lru page when calling madvise_pageout
b6a18a3c8d9f mm: factor out common parts between MADV_COLD and MADV_PAGEOUT
23757dccf53d mm: introduce MADV_PAGEOUT
1af766e81e59 mm: introduce MADV_COLD
a0747c913cc3 mm: change PAGEREF_RECLAIM_CLEAN with PAGE_REFRECLAIM
27a374d1d293 tools build: Check if gettid() is available before providing helper
2e38a0f2950e alinux: mm: add proc interface to control context readahead
```

```cpp
5028e358bcd4 alinux: mm: oom_kill: show killed task's cgroup info in global oom
7d41295cc97f alinux: mm: memcontrol: enable oom.group on cgroup-v1
0c8648d9554d alinux: doc: alibaba: Add priority oom descriptions
52e375fcb7a7 alinux: mm: memcontrol: introduce memcg priority oom
1e91d392d9e8 alinux: kernel: cgroup: account number of tasks in the css and its descendants
279df2ffcc1d alinux: doc: Add Documentation/alibaba/interfaces.rst
ef467b9ddbc0 alinux: memcg: Account throttled time due to memory.wmark_min_adj
60be0f545fac alinux: memcg: Introduce memory.wmark_min_adj
63442ea9f838 alinux: memcg: Provide users the ability to reap zombie memcgs
```



```cpp
ef467b9ddbc0 alinux: memcg: Account throttled time due to memory.wmark_min_adj
60be0f545fac alinux: memcg: Introduce memory.wmark_min_adj
63442ea9f838 alinux: memcg: Provide users the ability to reap zombie memcgs
9ea9e641c56a alinux: mm: remove unused variable
25f9e572a7da alinux: mm: kidled: fix frame-larger-than build warning
```

83cd9d23197d alinux: jbd2: track slow handle which is preventing transaction committing


```cpp
2e38a0f2950e alinux: mm: add proc interface to control context readahead
10be0b372cac readahead: introduce context readahead algorithm
```

```cpp
9ea9e641c56a alinux: mm: remove unused variable
a5c7cdabdd39 mm: thp: don't need care deferred split queue in memcg charge move path
```

```cpp
f8502f809fbe mm/hotplug: make remove_memory() interface usable
d2097173c4bd mm/memory_hotplug: make remove_memory() take the device_hotplug_lock
25f9e572a7da alinux: mm: kidled: fix frame-larger-than build warning
f38de7b3f896 mm: initialize MAX_ORDER_NR_PAGES at a time instead of doing larger sections
ad97e5e471a1 mm: implement new zone specific memblock iterator
b065ceca86fb mm: drop meminit_pfn_in_nid as it is redundant
e23b0cb5fbb8 mm: use mm_zero_struct_page from SPARC on all 64b architectures
```

```cpp
3e655c51650c alinux: mm,memcg: export memory.{min,low} to cgroup v1
942855175c77 alinux: mm,memcg: export memory.{events,events.local} to v1
5213c279abc9 mm: don't raise MEMCG_OOM event due to failed high-order allocation
de7ca7468c5d mm, memcg: introduce memory.events.local
0a198bb99132 mm, memcg: consider subtrees in memory.events
7d36553bd29d alinux: mm,memcg: export memory.high to v1
f72a099beb96 arm64: mm: add missing PTE_SPECIAL in pte_mkdevmap on arm64
1820ca632ed4 arm64: mm: implement pte_devmap support
d7066a918302 mm: introduce ARCH_HAS_PTE_DEVMAP
```

```cpp
a29243e2e890 alinux: mm: Support kidled
bbaee3afa992 alinux: mm: memcontrol: make distance between wmark_low and wmark_high configurable
c69c12cc10ba alinux: mm: vmscan: make memcg kswapd set memcg state to dirty or writeback
6332d4e3580b alinux: mm: memcontrol: treat memcg wmark reclaim work as kswapd
0149d7b9a6ab alinux: mm: memcontrol: add background reclaim support for cgroupv2
6967792f8d41 alinux: mm: memcontrol: support background async page reclaim
49a3b46525f3 alinux: mm: vmscan: make it sane reclaim if cgwb_v1 is enabled
```

```cpp
a9b17a5ef499 mm/resource: Let walk_system_ram_range() search child resources
567bed5755f7 mm/memory-hotplug: Allow memory resources to be children
0297fb96e113 mm/resource: Move HMM pr_debug() deeper into resource code
88e75600f8ca mm/resource: Return real error codes from walk failures
```


```cpp
6202ab24c966 mm, memcg: throttle allocators when failing reclaim over memory.high
a6f54bb03faa mm/zsmalloc.c: fix a -Wunused-function warning
4797417e5c19 x86/mm: Split vmalloc_sync_all()
```

a22c50279494 mm/swap_state.c: simplify total_swapcache_pages() with get_swap_device()
8afafd92a359 mm, swap: fix race between swapoff and some swap operations
0c22f660c7dc mm: swap: check if swap backing device is congested or not



da57ae3d8f47 x86/mm/cpa: Prevent large page split when ftrace flips RW on kernel text
9737e3ab95f7 x86/mm: Remove unused variable 'old_pte'
d5c07e58eb5b x86/mm/cpa: Avoid the 4k pages check completely
d935f3fe31bb x86/mm/cpa: Do the range check early
e4829197f967 x86/mm/cpa: Optimize same protection check
17847bcc7ae5 x86/mm/cpa: Add sanity check for existing mappings
7e40c6cc5370 x86/mm/cpa: Avoid static protection checks on unmap
e8b56c3dc056 x86/mm/cpa: Add large page preservation statistics
50298a22b4d3 x86/mm/cpa: Add debug mechanism
ea280ec727c9 x86/mm/cpa: Allow range check for static protections
fcbec5ea6634 x86/mm/cpa: Rework static_protections()
f6e4e6e88f6b x86/mm/cpa: Split, rename and clean up try_preserve_large_page()
52939d0dbbea x86/mm/init32: Mark text and rodata RO in one go



e2d3e3cb0d60 mm: workingset: tell cache transitions from workingset thrashing
b027d193c877 mm: workingset: don't drop refault information prematurely



8e358a027611 dump_stack: avoid the livelock of the dump_lock
6c944fc51f0a mm, vmstat: hide /proc/pagetypeinfo from normal users
2686f71fdcc5 mm: thp: handle page cache THP correctly in PageTransCompoundMap
7dfa51beacac mm, meminit: recalculate pcpu batch and high limits after init completes
8e6bf4bc3a88 mm: memcontrol: fix network errors from failing __GFP_ATOMIC charges


30cff8ab6ed7 mm/memory-failure: poison read receives SIGKILL instead of SIGBUS if mmaped more than once
91eec7692b94 hugetlbfs: don't access uninitialized memmaps in pfn_range_valid_gigantic()
f712e3066f75 mm/page_owner: don't access uninitialized memmaps when reading /proc/pagetypeinfo
bb6932c5a473 mm/slub: fix a deadlock in show_slab_objects()
9792afbd630b mm/memory-failure.c: don't access uninitialized memmaps in memory_failure()
01a44055a48a mmc: cqhci: Commit descriptors before setting the doorbell
6ea856efef9f fs/proc/page.c: don't access uninitialized memmaps in fs/proc/page.c


# 3 IO
-------

```cpp
48f9a1c0d29c alinux: dm: add support for IO polling
a613fd3185fb alinux: block: add back ->poll_fn in request queue
60decb718330 alinux: block/mq: add iterator for polling hw queues
91559bea7849 alinux: Revert "blk-mq: fix NULL pointer deference in case no poll implementation"
```


```cpp
83bae9307b6b blk-mq: support batching dispatch in case of io
c08303c639aa blk-mq: pass obtained budget count to blk_mq_dispatch_rq_list
25d50beddbe6 blk-mq: remove dead check from blk_mq_dispatch_rq_list
a44517bc658a blk-mq: move getting driver tag and budget into one helper
edaf05044f89 blk-mq: pass hctx to blk_mq_dispatch_rq_list
d15a1155dca7 blk-mq: pass request queue into get/put budget callback
5e19e570f3d6 Revert "scsi: core: run queue if SCSI device queue isn't ready and queue is idle"
cef99b200c42 blk-mq: Rerun dispatching in the case of budget contention
08d34ab79af1 blk-mq: Add blk_mq_delay_run_hw_queues() API call
826e0c2b5636 blk-mq: In blk_mq_dispatch_rq_list() "no budget" is a reason to kick
7764c05b0b48 blk-mq: Put driver tag in blk_mq_dispatch_rq_list() when no budget
7ba445112d91 alinux: configs: Enable SM2 asymmetric algorithm
```

```cpp
5b1e5e690da7 blk-throttle: Re-use the throtl_set_slice_end()
0728de8213ea blk-throttle: Open code __throtl_de/enqueue_tg()
6a1323009272 blk-throttle: Move service tree validation out of the throtl_rb_first()
563714f8d593 blk-throttle: Move the list operation after list validation
1a39eb05bf56 blk-throttle: Fix IO hang for a corner case
894e181d8286 blk-throttle: Avoid tracking latency if low limit is invalid
b0b82c541577 blk-throttle: Avoid getting the current time if tg->last_finish_time is 0
577d664237a9 blk-throttle: Remove a meaningless parameter for throtl_downgrade_state()
b17935d59831 blk-throttle: Avoid checking bps/iops limitation if bps or iops is unlimited
b78b5074e2b9 blk-throttle: Avoid calculating bps/iops limitation repeatedly
8a71d83e63f2 blk-throttle: Define readable macros instead of static variables
4671f1776d5d blk-throttle: Use readable READ/WRITE macros
131683786c42 blk-throttle: Fix some comments' typos
ea4a898b6aa8 Blk-throttle: update to use rbtree with leftmost node cached

8daa9640fa7c alinux: blk-throttle: fix logic error about BIO_THROTL_STATED in throtl_bio_end_io()
```





```cpp
0efb5f4b2cc fuse: reserve values for mapping protocol
4b61cdee4410 fuse: reserve byteswapped init opcodes
9ffcf1ac125b fuse: delete dentry if timeout is zero
09db4841303a fuse: Export fuse_dequeue_forget() function
f96b6dd693df fuse: export fuse_get_unique()
0213de764a40 fuse: Separate fuse device allocation and installation in fuse_conn
57f165877bfb fuse: add fuse_iqueue_ops callbacks
6769b1fd353a fuse: Export fuse_send_init_request()
609c1cf3d40b fuse: export fuse_len_args()
63b1ffab8812 fuse: export fuse_end_request()
0fdc23c2572a fuse: extract fuse_fill_super_common()
```

```cpp
1ec0deaf966d iocost: protect iocg->abs_vdebt with iocg->waitq.lock
751b3a445eea iocost_monitor: drop string wrap around numbers when outputting json
c5eb33a2cc3c iocost_monitor: exit successfully if interval is zero
8c8fb1418eca blk-iocost: account for IO size when testing latencies
9bdcaff2b9ca block: make rq sector size accessible for block stats
fc94dc726f86 blk-iocost: switch to fixed non-auto-decaying use_delay
d14b5329ce99 blk-iocost: Fix error on iocost_ioc_vrate_adj
4b3109d51f2c blk-iocost: remove duplicated lines in comments
e7c5f0283a52 blk-iocost: fix incorrect vtime comparison in iocg_is_idle()
80cefe0cc635 iocost: Fix iocost_monitor.py due to helper type mismatch
9ab225fe51e7 iocost: over-budget forced IOs should schedule async delay
```


```cpp
817f2cbf863e alinux: net/hookers: fix link error with ipv6 disabled
2307342e80bc alinux: writeback: memcg_blkcg_tree_lock can be static
2c0162e8f367 alinux: net/hookers: only enable on x86 platform
5e06ec32e8a7 alinux: fs/writeback: wrap cgroup writeback v1 logic
6f7afdf3f81a alinux: writeback: introduce cgwb_v1 boot param
67dc261c0c1d alinux: fs/writeback: Attach inode's wb to root if needed
b8016b41abf4 alinux: fs/writeback: fix double free of blkcg_css
665e22e9ec59 alinux: writeback: add debug info for memcg-blkcg link
7396367dbfa2 alinux: writeback: add memcg_blkcg_link tree
e209e3cbb13e alinux: net: kernel hookers service for toa module
0eea9fdfefb6 virtio_blk: add discard and write zeroes support
deb14433526c alinux: kconfig: Disable x86 clocksource watchdog
0fda568f60f9 alinux: Revert "x86/tsc: Prepare warp test for TSC adjustment"
a7d3a2158cb5 alinux: Revert "x86/tsc: Try to adjust TSC if sync test fails"
0430bf1d433e alinux: block-throttle: enable hierarchical throttling even on traditional hierarchy
9c4a8e9db790 alinux: drivers/virtio: add vring_force_dma_api boot param
```


# 3 CGROUP
-------

## 3.1 CONFIG_CGROUP_CACHE
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/01/15 | Yi Tao <escape@linux.alibaba.com> | [add CONFIG_CGROUP_CACHE](https://github.com/gatieme/linux/commit/021d0414e9c5c593431b6b448a73b465a0e9662b) | 自研特性, 加速 cgroup 的创建, 目前已经支持 memcg, cpu, cpuacct 等子 subsysten, 同时接管 cgroup kernfs node 的创建.
 | 自研 NA | [COMMIT](https://github.com/gatieme/linux/commit/021d0414e9c5c593431b6b448a73b465a0e9662b) |



```cpp
#31531504
021d0414e9c5 configs: add CONFIG_CGROUP_CACHE
18d7ef726d4a alinux: cgroup: fix dead lock in put_to_cache
5b71d2f57720 alinux: cgroup: introduce cgroup_limit
dc6cf8e45fc4 alinux: cgroup: use cache for kernfs node
51b3509e23bd alinux: sched: use cache when creating task_group
1435e60d7490 alinux: cpuacct: use cache when creating cpuacct
e79f31f668aa alinux: cpuacct: extract init and free
ef94292f1674 alinux: memcg: use cache when creating memcg
869bebf17144 alinux: memcg: memcg: extract init from mem_cgroup_alloc
bea1123abee6 alinux: cgroup: introduce cache struct and function
```



## 3.2 rich container
-------


```cpp
38d7e4fc9e01 alinux: virtio_net: fix wrong print format type
86fecc6dbe1f alinux: configs: Enable rich container
f3743190bb46 alinux: cpuinfo: Add cpuinfo support of cpu quota and cpu.shares
4dbc0b194539 alinux: pidns: Introduce rich container scenario
2eb706e6db6a alinux: pidns: Support rich container switch on/off
9eb3033b5452 alinux: meminfo: Add meminfo support for rich container
e6e2e084f712 alinux: sysfs/cpu: Add online cpus support for rich container
eae4ceb83bae alinux: arm64: cpuinfo: Add cpuinfo support for rich container
f8c0ce62776d alinux: x86: cpuinfo: Add cpuinfo support for rich container
178de8fd07eb checkpatch/coding-style: deprecate 80-column warning
53b844792580 alinux: net: track the pid who created socks
```


# 4 VIRT
-------


```cpp
2706f2ae1a7d alinux: configs: Enable NITRO_ENCLAVES for x86_64
3bac8a607ab0 alinux: nitro_enclaves: Split mmio region and increase EIF_LOAD_OFFSET
b491dbca9ac9 nitro_enclaves: Fixup type and simplify logic of the poll mask setup
d4a056b99d7f nitro_enclaves: Add overview documentation
dba621bf4b2b nitro_enclaves: Add sample for ioctl interface usage
3ed22df45360 nitro_enclaves: Add Makefile for the Nitro Enclaves driver
d6f7a1168c3d nitro_enclaves: Add Kconfig for the Nitro Enclaves driver
fb510a87cb33 nitro_enclaves: Add logic for terminating an enclave
e4bc049cc6aa nitro_enclaves: Add logic for starting an enclave
852ca10b82d0 nitro_enclaves: Add logic for setting an enclave memory region
64843abbe002 nitro_enclaves: Add logic for getting the enclave image load info
0cf92b6ffdfc nitro_enclaves: Add logic for setting an enclave vCPU
26480f8b795f nitro_enclaves: Add logic for creating an enclave VM
690c8c85cbea nitro_enclaves: Init misc device providing the ioctl interface
22462daa1f9a nitro_enclaves: Handle out-of-band PCI device events
91375a08585d nitro_enclaves: Handle PCI device command requests
94c84243ea68 nitro_enclaves: Init PCI device driver
f4c81ada1b8b nitro_enclaves: Define enclave info for internal bookkeeping
93198b71cf18 nitro_enclaves: Define the PCI device interface
2320f6ed0a8b nitro_enclaves: Add ioctl interface definition
```

```cpp
5f0a83af2d0 KVM x86: Extend AMD specific guest behavior to Hygon virtual CPUs
41bcadd16414 KVM: x86: Add helpers to perform CPUID-based guest vendor check
c989d9b7f1c2 kvm: x86: Use AMD CPUID semantics for AMD vCPUs
e2fb3c567a5a kvm: x86: Improve emulation of CPUID leaves 0BH and 1FH
```


```cpp
88fa38fa1f16 virtiofs: Use completions while waiting for queue to be drained
2a6ae53ed76c virtiofs: Do not send forget request "struct list_head" element
a6d9f512aea9 virtiofs: Use a common function to send forget
8270fcad0efc virtiofs: Fix old-style declaration
823286b71f24 virtiofs: Remove set but not used variable 'fc'
986957da44f8 virtiofs: Retry request submission from worker context
ab040ec83ff4 virtiofs: Count pending forgets as in_flight forgets
f5fa6847df71 virtiofs: Set FR_SENT flag only after request has been sent
da960fb237f8 virtiofs: No need to check fpq->connected state
c3694064258c virtiofs: Do not end request in submission context
5cb5b717178d virtio-fs: Change module name to virtiofs.ko
917f6dfbd24a virtio-fs: add virtiofs filesystem
3fcc16fc602c virtio-fs: add Documentation/filesystems/virtiofs.rst
```



## task #29077503(virtio-mem: paravirtualized memory)
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2020/05/07 | Johannes Weiner <hannes@cmpxchg.org> | [virtio-mem: paravirtualized memory](https://lore.kernel.org/patchwork/cover/1237689) | NA | v2 ☑ [5.8-rc1](https://kernelnewbies.org/Linux_5.8#Memory_management) | [PatchWork v4](https://lore.kernel.org/patchwork/cover/1237689) |

```cpp
start_commit : 1e5d7fb5446ca564cd5be1b0f555b8f766b3aacf
end_commit   : add0d5e477f6fe01e7ab1556590bb47fa12a13e2
1e5d7fb5446c virtio_mem: convert device block size into 64bit
64f827bfa426 mm/memory_hotplug: set node_start_pfn of hotadded pgdat to 0
99d68412c05e virtio-mem: silence a static checker warning
a640f7590c2b virtio-mem: drop unnecessary initialization
8e6d8cc8fe1c virtio-mem: Don't rely on implicit compiler padding for requests
fe30f1c1e835 virtio-mem: Try to unplug the complete online memory block first
7540dce96113 virtio-mem: Use -ETXTBSY as error code if the device is busy
c9d27e7ba7b0 virtio-mem: Unplug subblocks right-to-left
e134eeb06600 virtio-mem: Drop manual check for already present memory
94271a31a098 virtio-mem: Add parent resource for all added "System RAM"
f362154ac025 virtio-mem: Better retry handling
bf191d238e51 virtio-mem: Offline and remove completely unplugged memory blocks
c062d118b6c4 mm/memory_hotplug: Introduce offline_and_remove_memory()
97fe5a448b11 virtio-mem: Allow to offline partially unplugged memory blocks
2050082598f4 mm: Allow to offline unmovable PageOffline() pages via MEM_GOING_OFFLINE
733f2794cb8b virtio-mem: Paravirtualized memory hotunplug part 2
91f30ea2ed5c virtio-mem: Paravirtualized memory hotunplug part 1
c9f36272db0c virtio-mem: Allow to specify an ACPI PXM as nid
d3997ceb10d2 virtio-mem: Paravirtualized memory hotplug

0c6a9eb5c0d9 mm/memory_hotplug: export generic_online_page()
bd6aced3785a mm/page_alloc.c: memory hotplug: free pages as higher order
5eee472829ec mm, memory_hotplug: deobfuscate migration part of offlining
a6785cdc8d2b mm, memory_hotplug: __offline_pages fix wrong locking
86f9a7e38183 mm, memory_hotplug: print reason for the offlining failure
80aa4777f63f mm/page_isolation.c: convert SKIP_HWPOISON to MEMORY_OFFLINE
5316eb6eaeb8 mm: only report isolation failures when offlining memory
59df23d6178e mm: convert PG_balloon to PG_offline
```


```cpp
414ddd93b45c namei: LOOKUP_{IN_ROOT,BENEATH}: permit limited ".." resolution
685c60efde29 namei: LOOKUP_IN_ROOT: chroot-like scoped resolution
31084d4b6f53 namei: LOOKUP_BENEATH: O_BENEATH-like scoped resolution
0a73d1631a54 fs/namei.c: keep track of nd->root refcount status
40107f78cda6 fs/namei.c: new helper - legitimize_root()
38549ac131a1 namei: LOOKUP_NO_XDEV: block mountpoint crossing
773b3516bc65 namei: LOOKUP_NO_MAGICLINKS: block magic-link resolution
35e8a4ee5cfc namei: LOOKUP_NO_SYMLINKS: block symlink resolution
53a5271b08ae namei: allow set_root() to produce errors
b63f56c4af17 namei: allow nd_jump_link() to produce errors
39f702abc31c nsfs: clean-up ns_get_path() signature to return int
8eef64a26992 namei: only return -ECHILD from follow_dotdot_rcu()
```



```cpp
dad6cc969648 x86/kvm: Be careful not to clear KVM_VCPU_FLUSH_TLB bit
4cb032ec91b2 KVM: nVMX: Check IO instruction VM-exit conditions
f26796b00a5d KVM: nVMX: Refactor IO bitmap checks into helper function
c2868767b1eb KVM: nVMX: Don't emulate instructions in guest mode
```


```cpp
e2f72bd7314 cpuidle: haltpoll: Take 'idle=' override into account
6805e4bab22e cpuidle-haltpoll: do not set an owner to allow modunload
f9706f222c79 cpuidle-haltpoll: return -ENODEV on modinit failure
aee4b74e6fb8 cpuidle-haltpoll: set haltpoll as preferred governor
0176005ec6ae cpuidle: allow governor switch on cpuidle_register_driver()
1e18beca0aee cpuidle-haltpoll: vcpu hotplug support
7b905bca80af cpuidle: add haltpoll governor
6d2ef95fa705 add cpuidle-haltpoll driver
dc59f8b073ff governors: unify last_state_idx
5e083c5c1f94 cpuidle: menu: Do not update last_state_idx in menu_select()
2163e221226f cpuidle: add poll_limit_ns to cpuidle_device structure
a72c57d78b75 cpuidle: poll_state: Fix default time limit
4488ba366420 cpuidle: Add cpuidle.governor= command line parameter
907fd899c16a cpuidle: poll_state: Disregard disable idle states
a89bfc68bdc6 cpuidle: poll_state: Revise loop termination condition
```


```cpp
497b388250bf KVM: arm64: Add support for creating PUD hugepages at stage 2
445074ed9a0d KVM: arm64: Update age handlers to support PUD hugepages
1f37c38fae1e KVM: arm64: Support handling access faults for PUD hugepages
29c25546b0c8 KVM: arm64: Support PUD hugepage in stage2_is_exec()
f89dc7dc8c69 KVM: arm64: Support dirty page tracking for PUD hugepages
8351986b2438 KVM: arm/arm64: Introduce helpers to manipulate page table entries
4a0080eae820 KVM: arm/arm64: Re-factor setting the Stage 2 entry to exec on fault
0ca1b1abc71e KVM: arm/arm64: Share common code in user_mem_abort()
6088bea01141 KVM: arm/arm64: Log PSTATE for unhandled sysregs
02107bfd39df arm64: KVM: Consistently advance singlestep when emulating instructions
```

# 5 NET
-------


```cpp
d25693146dd configs: enable configs for ICMP PingTrace support
2ef6ebabb62e alinux: net: add pingtrace feature support
```


```cpp
54075ba4f00c alinux: tcp: introduce tunable tcp_rto_min value
11f0fcae52c6 ipvs: allow connection reuse for unconfirmed conntrack
59b08e389968 alinux: Revert "TencentOS-kernel: ipvs: avoid drop first packet by reusing conntrack"
```


```cpp
996ce642edc configs: enable config for TCP_RT module
3719abbc340b alinux: tcp_rt: add Documentation for tcp-rt
5efc57b3ea4d alinux: tcp_rt module: peer ports add more statistics
ded7aa682cd2 alinux: tcp_rt module: support pports_range
36e4a622505c alinux: tcp_rt module: change the _tcp_rt_stats item type to u64
81be6f9d9b51 alinux: tcp_rt module: use atomic64_xchg replace atomic64_read and atomic64_set
404bc1e27230 alinux: tcp_rt module: save tcp rtt when R record, change the unit to us
0c635e000428 alinux: tcp_rt module: P record add rt and tcp reorder info
13867aae1287 alinux: tcp_rt module: simplify the parameter name
8675ebe3a08f alinux: tcp_rt module: fix repeat stats for mrtt
2bac13c5873c alinux: tcp_rt module: change relay work mode
757ae9401ae1 alinux: tcp_rt module: change real to stats
f486d9b98b68 alinux: tcp_rt module: support ports_range
03fcbae2ecd7 alinux: tcp_rt module: add tcp_rt module
```

# 6 X86
-------


```cpp
b22ed0da929c x86/resctrl: Don't move a task to the same resource group
fabce5a823c5 x86/resctrl: Use an IPI instead of task_work_add() to update PQR_ASSOC MSR
100bada1805b x86/resctrl: Fix incorrect local bandwidth when mba_sc is enabled
f6500f1c2a97 x86/resctrl: Remove unused struct mbm_state::chunks_bw
b157de95fb44 x86/resctrl: Add necessary kernfs_put() calls to prevent refcount leak
6fa45e6146f6 x86/resctrl: Remove superfluous kernfs_get() calls to prevent refcount leak
e8b331c11f9e x86/resctrl: Fix a NULL vs IS_ERR() static checker warning in rdt_cdp_peer_get()
39b9015f9115 x86/resctrl: Fix invalid attempt at removing the default resource group
eaf839556c91 x86/resctrl: Preserve CDP enable over CPU hotplug
e5018c8e3705 x86/resctrl: Fix a deadlock due to inaccurate reference
d004e4e96c66 x86/resctrl: Fix use-after-free due to inaccurate refcount of rdtgroup
0164232bf0a2 x86/resctrl: Fix use-after-free when deleting resource groups
5650783cf911 x86/resctrl: Fix potential memory leak
e4f9d5318305 x86/resctrl: Fix an imbalance in domain_remove_cpu()
```

```cpp
715ef40d1a89 tools/power/x86/intel-speed-select: Fix invalid core mask
16c74c3dd215 tools/power/x86/intel-speed-select: Increase CPU count
5ef88569ce16 tools/power/x86/intel-speed-select: Fix json perf-profile output output
01d9f1fa11dc tools/power/x86/intel-speed-select: Enable clos for turbo-freq enable
45b4d20e5270 intel-speed-select: Fix speed-select-base-freq-properties output on CLX-N
0b3b149d6c4a tools/power/x86/intel-speed-select: Fix CLX-N package information output
eba01950ec28 tools/power/x86/intel-speed-select: Change debug to error
7bed6e0755d2 tools/power/x86/intel-speed-select: Check support status before enable
b18aa4d2b62c tools/power/x86/intel-speed-select: Fix a typo in error message
0043f50113c3 tools/power/x86/intel-speed-select: Avoid duplicate Package strings for json
0703f7d8ada2 tools/power/x86/intel-speed-select: Add display for enabled cpus count
ce3c227467ec tools/power/x86/intel-speed-select: Print friendly warning for bad command line
3fbee5db1ada tools/power/x86/intel-speed-select: Fix avx options for turbo-freq feature
85722af35b39 tools/power/x86/intel-speed-select: Improve CLX commands
41df16a650ea tools/power/x86/intel-speed-select: Show error for invalid CPUs in the options
8915566af7ba tools/power/x86/intel-speed-select: Improve core-power result and error display
0a59e82e7403 tools/power/x86/intel-speed-select: Kernel interface error handling
0f633cf92ad1 tools/power/x86/intel-speed-select: Improve output of perf-profile commands
c9c8dc2f2817 tools/power/x86/intel-speed-select: Enhance help for core-power assoc
5a06240a315e tools/power/x86/intel-speed-select: Display error for invalid priority type
d7a8e7355b5e tools/power/x86/intel-speed-select: Check feature status first
20da8e5acf92 tools/power/x86/intel-speed-select: Improve error display for perf-profile feature
88fbd32939b6 tools/power/x86/intel-speed-select: Add an API for error/information print
72d9569162f6 tools/power/x86/intel-speed-select: Enhance --info option
a94ce6a77fc1 tools/power/x86/intel-speed-select: Enhance help
a2424d811508 tools/power/x86/intel-speed-select: Helpful warning for missing kernel interface
3b81f5256aaf tools/power/x86/intel-speed-select: Store topology information
0285b9681281 tools/power/x86/intel-speed-select: Max CPU count calculation when CPU0 is offline
b7339039055c tools/power/x86/intel-speed-select: Special handling for CPU 0 online/offline
e69da83b241b tools/power/x86/intel-speed-select: Use more verbiage for clos information
ea379f73ea54 tools/power/x86/intel-speed-select: Enhance core-power info command
8f2c9f8d89af tools/power/x86/intel-speed-select: Make target CPU optional for core-power info
222c7a5259e1 tools/power/x86/intel-speed-select: Warn for invalid package id
82488cf32ce6 .gitignore: add SPDX License Identifier
26ba517373b6 tools/power/x86/intel-speed-select: Fix last cpu number
8d6f2ec7a9c0 tools/power/x86/intel-speed-select: Fix mailbox usage for CLOS_PM_QOS_CONFIG
f3309ca1a2ae tools/power/x86/intel-speed-select: Avoid duplicate names for json parsing
b437859257f0 tools/power/x86/intel-speed-select: Fix display for turbo-freq auto mode
56a8649013e3 tools/power/x86/intel-speed-select: Update version
72375fa374e9 tools/power/x86/intel-speed-select: Change the order for clos disable
bd1aeda2eb17 tools/power/x86/intel-speed-select: Fix result display for turbo-freq auto mode
8b415df4c63e tools/power/x86/intel-speed-select: Add support for core-power discovery
a28ee43468b4 tools/power/x86/intel-speed-select: Display TRL buckets for just base config level
d6f05680a643 tools/power/x86/intel-speed-select: Ignore missing config level
fefcc86d8034 tools/power/x86/intel-speed-select: Increment version
3b2760d14a27 tools/power/x86/intel-speed-select: Use core count for base-freq mask
baf511bfbdaa tools/power/x86/intel-speed-select: Support platform with limited Intel(R) Speed Select
04279699954c tools/power/x86/intel-speed-select: Use Frequency weight for CLOS
ddd6b2b64b4a tools/power/x86/intel-speed-select: Make CLOS frequency in MHz
d1c83ca6163b tools/power/x86/intel-speed-select: Use mailbox for CLOS_PM_QOS_CONFIG
056ec3bb4e0e tools/power/x86/intel-speed-select: Auto mode for CLX
b06a6b5fd2b9 tools/power/x86/intel-speed-select: Correct CLX-N frequency units
d80d60329f81 tools/power/x86/intel-speed-select: Change display of "avx" to "avx2"
ebdc7f946a8c tools/power/x86/intel-speed-select: Extend command set for perf-profile
c12c0e5480c6 tools/power/x86/intel-speed-select: Implement base-freq commands on CascadeLake-N
c02113c4086c tools/power/x86/intel-speed-select: Implement 'perf-profile info' on CascadeLake-N
0407dcbdb421 tools/power/x86/intel-speed-select: Implement CascadeLake-N help and command functions structures
c8b0fa520a64 tools/power/x86/intel-speed-select: Add check for CascadeLake-N models
9621418e5bbb tools/power/x86/intel-speed-select: Make process_command generic
b6f5cccd74d8 tools/power/x86/intel-speed-select: Add int argument to command functions
bcfb56a0502a tools/power/x86/intel-speed-select: Refuse to disable core-power when getting used
22fdc63c4842 tools/power/x86/intel-speed-select: Turbo-freq feature auto mode
e6e4ba9a519f tools/power/x86/intel-speed-select: Base-freq feature auto mode
53c653dd2025 tools/power/x86/intel-speed-select: Remove warning for unused result
6585c11603d2 tools/power/x86/intel-speed-select: Fix perf-profile command output
ed098c7030fb tools/power/x86/intel-speed-select: Extend core-power command set
e089cba4ad0f tools/power/x86/intel-speed-select: Fix some debug prints
53e7f05bdde5 tools/power/x86/intel-speed-select: Format get-assoc information
2c74ba62cb21 tools/power/x86/intel-speed-select: Allow online/offline based on tdp
99baa0fc29b1 tools/power/x86/intel-speed-select: Fix high priority core mask over count
2d1da1320fac tools/power/x86/intel-speed-select: Display core count for bucket
dc347816a13a tools/power/x86/intel-speed-select: Fix memory leak
88dbd1e0179c tools/power/x86/intel-speed-select: Output success/failed for command output
6dcdef594f8b tools/power/x86/intel-speed-select: Output human readable CPU list
a0ddb2f96b2d tools/power/x86/intel-speed-select: Change turbo ratio output to maximum turbo frequency
151b2530eaa4 tools/power/x86/intel-speed-select: Switch output to MHz
c59df5dc4fa1 tools/power/x86/intel-speed-select: Simplify output for turbo-freq and base-freq
f96066050651 tools/power/x86/intel-speed-select: Fix cpu-count output
9beb8b68d0af tools/power/x86/intel-speed-select: Fix help option typo
88cf267ac7bd tools/power/x86/intel-speed-select: Fix package typo
48a39768858b tools/power/x86/intel-speed-select: Fix a read overflow in isst_set_tdp_level_msr()
5e3926548799 tools/power/x86/intel-speed-select: Add .gitignore file
```


```cpp
c73335a8dc5b intel_idle: Customize IceLake server support
2a450b56cc35 Intel: Documentation: admin-guide: PM: Add intel_idle document
52387dc3315e Intel: ACPI: processor: Make ACPI_PROCESSOR_CSTATE depend on ACPI_PROCESSOR
b2502b519d34 Intel: intel_idle: Use ACPI _CST on server systems
275d4e4243e1 Intel: intel_idle: Add module parameter to prevent ACPI _CST from being used
985aa8e87db4 Intel: intel_idle: Allow ACPI _CST to be used for selected known processors
07049896fd03 Intel: cpuidle: Allow idle states to be disabled by default
104fe267a92e Intel: Documentation: admin-guide: PM: Add cpuidle document
abb7d81744f1 Intel: cpuidle: use BIT() for idle state flags and remove CPUIDLE_DRIVER_FLAGS_MASK
5887563b9925 Intel: intel_idle: Use ACPI _CST for processor models without C-state tables
7af3d4b5ce73 Intel: intel_idle: Refactor intel_idle_cpuidle_driver_init()
c8af726647cf Intel: ACPI: processor: Export acpi_processor_evaluate_cst()
2fe34238a4aa Intel: ACPI: processor: Clean up acpi_processor_evaluate_cst()
43fd4feea67f Intel: ACPI: processor: Introduce acpi_processor_evaluate_cst()
19805ef74259 Intel: ACPI: processor: Export function to claim _CST control
```



```cpp
9a1de619ce83 perf stat: Fix shadow stats for clock events
d5a3153a28c8 arm64: fix kernel stack overflow in kdump capture kernel
625b8a72f51a arm64: Relax ICC_PMR_EL1 accesses when ICC_CTLR_EL1.PMHE is clear
b534b2266e5a arm64: Lower priority mask for GIC_PRIO_IRQON
89d26aa939d5 arm64: Fix incorrect irqflag restore for priority masking for compat
dce43ff7a7b8 arm64: irqflags: Introduce explicit debugging for IRQ priorities
d53b2738b944 arm64: Fix incorrect irqflag restore for priority masking
bf4c79db12ac arm64: Fix interrupt tracing in the presence of NMIs
ea17bf4a93c7 arm64: irqflags: Add condition flags to inline asm clobber list
df76e7b7cbb0 arm64: irqflags: Pass flags as readonly operand to restore instruction
df594b47657e arm64: sysreg: Make mrs_s and msr_s macros work with Clang and LTO
20c588c5a5e2 arm64: irqflags: Fix clang build warnings
beaa4f75b742 arm64: Enable the support of pseudo-NMIs
b20a91fef74b configs: aarch64: add PSEUDO_NMI configuration item
b63f21bcb81b arm64: Skip irqflags tracing for NMI in IRQs disabled context
dce9c126c93f arm64: Skip preemption when exiting an NMI
3f44beda5245 arm64: Handle serror in NMI context
b50b9a7ba6ee irqchip/gic-v3: Allow interrupts to be set as pseudo-NMI
53fa25a9ee68 irqchip/gic-v3: Handle pseudo-NMIs
7bf70240e0dc irqchip/gic-v3: Detect if GIC can support pseudo-NMIs
e957955e218a arm64: gic-v3: Implement arch support for priority masking
7d5f80c92b0c arm64: Switch to PMR masking when starting CPUs
d90f0060b112 irqchip/gic-v3: Factor group0 detection into functions
9465c0bbe7b0 arm64: alternative: Apply alternatives early in boot process
a716ee5e5c59 arm64: alternative: Allow alternative status checking per cpufeature
fc82838523ab arm64: daifflags: Include PMR in daifflags restore operations
0302164f0c3c arm64: irqflags: Use ICC_PMR_EL1 for interrupt masking
1c2e583bfaa4 efi: Let architectures decide the flags that should be saved/restored
2869273135e4 arm64: kvm: Unmask PMR before entering guest
22382bd16fae arm64: Unmask PMR before going idle
b9282aa42464 arm64: Make PMR part of task context
260a54b23c76 arm64: ptrace: Provide definitions for PMR values
8d753c9b5f5a irqchip/gic-v3: Switch to PMR masking before calling IRQ handler
0e5b4185788a arm/arm64: gic-v3: Add PMR and RPR accessors
c5cd3dff4b9c arm64: cpufeature: Add cpufeature for IRQ priority masking
34b204a7f569 arm64: cpufeature: Set SYSREG_GIC_CPUIF as a boot system feature
d458d876e743 arm64: Remove unused daif related functions/macros
d0cd74614a25 irqdesc: Add domain handler for NMIs
3035257e3dbd genirq: Provide NMI handlers
3bd2aa7bd4cb genirq: Provide NMI management for percpu_devid interrupts
2e186fb53ca1 genirq: Provide basic NMI management for interrupt lines
eeae3040f1a1 arm64: daifflags: Use irqflags functions for daifflags
8fdd8aec22b0 irqchip/gic: Unify GIC priority definitions
21fc7f872457 irqchip/gic-v3: Remove acknowledge loop
489104636dee arm64: capabilities: Batch cpu_enable callbacks
5a0c24b3dc67 arm64: capabilities: Use linear array for detection and verification
047390ae7c7a arm64: capabilities: Optimize this_cpu_has_cap
de2783098076 arm64: capabilities: Speed up capability lookup
5fb19a4d8fe0 arm64: capabilities: Merge duplicate entries for Qualcomm erratum 1003
44498f79acd7 arm64: capabilities: Merge duplicate Cavium erratum entries
b5fe242835cf arm64: capabilities: Merge entries for ARM64_WORKAROUND_CLEAN_CACHE
ad27c9e51256 arm64/dma-mapping: Mildly optimise non-coherent IOMMU ops
```


607e86d5d3ea iommu/arm-smmu-v3: Remove unnecessary wrapper function
a1fdb30fbfb5 iommu/arm-smmu: Support non-strict mode
989011e77d59 iommu/io-pgtable-arm-v7s: Add support for non-strict mode
c35eee6e8a28 iommu/arm-smmu-v3: Add support for non-strict mode
0e8d86ed2402 iommu/io-pgtable-arm: Add support for non-strict mode
eb59e3891ef3 iommu: Add "iommu.strict" command line option
bd385efeffb6 iommu/dma: Add support for non-strict mode
f83ff7c9e7fd iommu/arm-smmu-v3: Implement flush_iotlb_all hook
57afa2225ea1 iommu/arm-smmu-v3: Avoid back-to-back CMD_SYNC operations
e4416a58c0cb iommu/arm-smmu-v3: Fix a couple of minor comment typos



74de6a39ab40 scripts/sorttable: Implement build-time ORC unwind table sorting
1fd0b7e9eb88 scripts/sorttable: Rename 'sortextable' to 'sorttable'
f2180acdcfc5 scripts/sortextable: Refactor the do_func() function
dd4b28606ec5 scripts/sortextable: Remove dead code
e3f834c364c5 scripts/sortextable: Clean up the code to meet the kernel coding style better
9999162392f9 scripts/sortextable: Rewrite error/success handling


# KDUMP
-------


https://lore.kernel.org/patchwork/patch/1266592

```cpp
8d6fc8aaccf4 mm/memcontrol: fix OOPS inside mem_cgroup_get_nr_swap_pages()
```