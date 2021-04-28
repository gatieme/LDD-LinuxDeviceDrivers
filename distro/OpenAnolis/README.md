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


# 1 SCHED_SLI 
-------


```cpp
git log --oneline | grep alinux | grep -E "sched|cpuacct"
```

```cpp
06d7286d7c8e alinux: sched: add kconfig SCHED_SLI
```

## 1.1 Latency Histograms
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
```

## 1.2 CPUACCT STAT
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


## 1.3 PSI V1
-------

```cpp
1bd8a72b1181 alinux: psi: using cpuacct_cgrp_id under CONFIG_CGROUP_CPUACCT
1f49a7385032 alinux: psi: Support PSI under cgroup v1
dc159a61c043 alinux: introduce psi_v1 boot parameter
```

# 2 Scheduler
-------


## 2.1

```cpp
bcaf8afd6270 alinux: sched: Add switch for scheduler_tick load tracking
bb48b716f496 alinux: sched: Add switch for update_blocked_averages
d2440c99979d alinux: sched/fair: use static load in wake_affine_weight
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