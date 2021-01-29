---

title: 从 NUMA 3 跳(3-HOPS)问题入手--窥--内核负载均衡
date: 2021-01-24 18:40
author: gatieme
tags:
        - linux
        - scheduler
        - numa
categories:
        - scheduler

thumbnail:
blogexcerpt: 这篇文章旨在帮助希望更好地分析其应用程序中性能瓶颈的人们. 有许多现有的方法可以进行性能分析, 但其中没有很多方法既健壮又正式. 而 TOPDOWN 则为大家进行软硬协同分析提供了无限可能. 本文通过 pmu-tools 入手帮助大家进行 TOPDOWN 分析.


---

| CSDN | GitHub | OSKernelLAB |
|:----:|:------:|:-----------:|
| [紫夜阑珊-青伶巷草](https://blog.csdn.net/gatieme/article/details/113269052) | [`LDD-LinuxDeviceDrivers`](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/debug/tools/topdown/pmu-tools) | [Intel CPU 上使用 pmu-tools 进行 TopDown 分析](https://oskernellab.com/2021/01/24/2021/0127-0001-Topdown_analysis_as_performed_on_Intel_CPU_using_pmu-tools/) |

<br>

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作.

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 鄙人在此谢谢啦

<br>




时间线

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:---------:|:----:|
| 2020/03/24 | Valentin Schneider | [sched/topology: Fix overlapping sched_group build](https://lore.kernel.org/patchwork/patch/1214752) | 修复 | v1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1214752) |
| 2020/8/14 | Valentin Schneider | [sched/topology: NUMA topology limitations](https://lkml.org/lkml/2020/8/14/214) | 修复 | v1 | [LKML](https://lkml.org/lkml/2020/8/14/214) |
| 2020/11/10 | Valentin Schneider | [sched/topology: Warn when NUMA diameter > 2](https://lore.kernel.org/patchwork/patch/1336369) | WARN | v1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1336369) |
| 2021/01/22 | Valentin Schneider | [sched/topology: NUMA distance deduplication](https://lore.kernel.org/patchwork/cover/1369363) | 修复 | v1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1369363), [LKML](https://lkml.org/lkml/2021/1/22/460) |

| 时间  | 提问者 | 问题描述 |
|:----:|:-----:|:-------:|
| 2021/1/21 | Meelis Roos | [Shortest NUMA path spans too many nodes](https://lkml.org/lkml/2021/1/21/726) |


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:---------:|:----:|
| 2021/01/15 | Song Bao Hua (Barry Song) | [sched/fair: first try to fix the scheduling impact of NUMA diameter > 2 1366256 diffmboxseries](https://lore.kernel.org/patchwork/patch/1366256) | 尝试修复 3 跳问题导致的性能蜕化 | RFC | [PatchWork](https://lore.kernel.org/patchwork/cover/1366256) |
| 2021/01/27 | Song Bao Hua (Barry Song) | [sched/topology: fix the issue groups don't span domain->span for NUMA diameter > 2](https://lore.kernel.org/patchwork/patch/1371875)| 修复 | v2 | [PatchWork](https://lore.kernel.org/patchwork/cover/1371875), [LKML]() |




# 1 背景描述
-------




# 2 3 跳问题的影响
-------


## 2.1 SCHED_DOMAIN 对负载均衡的影响
-------

那么 3 跳问题生成的这样的调度域结构, 对系统有什么影响, 究竟会引发什么实质的问题呢 ?

SCHED_DOMAIN 最直接的影响就是负载均衡, 在几个负载均衡的路径下都会进行:

1.  周期性负载均衡 load_balance 会周期性的从最忙的 group 中 pull task 到空闲 group 的 CPU 上, 这个是我们通常说的狭义负载均衡的概念, 主要在空闲的 CPU 或者调度组的第一个 CPU 上进行, 将该 CPU 作为 dst CPU, 然后 find_busiest_group -=> find_busiest_queue 查找到最繁忙的分组中负载最大的 CPU 作为 SRC, 从 SRC 迁移指定量的进程到 DST.

```cpp
load_balance
	-=> should_we_balance(&env);
	-=> group = find_busiest_group(&env);
	-=> busiest = find_busiest_queue(&env, group);
```

2.  其他路径, 在唤醒时(try_to_wake_up 和 wake_up_new_task)总是倾向通过 find_idlest_cpu 去寻找最空闲的 GROUP 去投送 task. 先 find_idlest_group 查找到当前待搜索 SD 中负载最小的 GROUP, 接着 find_idlest_cpu 搜索到负载最小的 CPU, 最后将进程唤醒到这个 CPU 上. 唤醒流程的均衡过程我们作为广义负载均衡流程, 他并不是显式 load_balance, 但是也是通过负载的比较希望系统在既定 SCHED_DOMAIN 内更均衡一些.

> 注意唤醒路有些场景是有一些优化的, 比如 try_to_wake_up 的时候, 会先看是否可以使用 WAKE_AFFINE 先优选一个 CPU, 这个是唤醒的快速路径.

可以看到每次负载均衡时, 都是在既定的 SCHED_DOMAIN 范围内进行的, 先找到域内负载最大或者最小的 SCHED_GROUP, 然后再从 SCHED_GROUP 内搜索满足要求的 CPU. 由于调度域内的各个 SCHED_GROUP 其实就是所包含的 CPU 子 SCHED_DOMAIN 的集合, 因此各个 SCHED_DOMAIN 的均衡, 其实就是 子 SCHED_GROUP/SCHED_DOMAIN 之间的均衡.


## 2.2 以 0 为边缘节点的情况
-------

### 2.2.1 NUMA 拓扑信息
-------

首先以 0 为边缘节点的时候, 由此所构建的 sched_domain 和 sched_group 信息如下所示.

```cpp
拓扑结构
      2       10      2
  0 <---> 1 <---> 2 <---> 3

NUMA 距离层级(阶梯)
10 -=> 12 -=> 22 -=> 24
```

| SCHED_DOAMIN |   NODE  0   |   NODE  1   |   NODE  2   |   NODE  3   |
|:------------:|:-----------:|:-----------:|:-----------:|:-----------:|
|   DOMAIN 0   |     0-1     |     2-3     |     4-5     |     6-7     |
|   GROUP  0   |   {0},{1}   |   {2},{3}   |   {4},{5}   |   {6},{7}   |
|--------------|-------------|-------------|-------------|-------------|
|   DOMAIN 1   |     0-3     |     0-3     |     4-7     |     4-7     |
|   GROUP  1   | {0-1},{2-3} | {2-3},{0-1} | {4-5},{6-7} | {6-7},{4-5} |
|--------------|-------------|-------------|-------------|-------------|
|   DOMAIN 2   |     0-5     |     0-7     |     0-7     |     2-7     |
|   GROUP  2   | {0-3},{4-7} | {0-3},{4-7} | {4-7},{0-3} | {4-7},{0-3} |
|--------------|-------------|-------------|-------------|-------------|
|   DOMAIN 3   |     0-7     |     NA      |     NA      |     0-7     |
|   GROUP  3   | {0-5},{2-7} |     NA      |     NA      | {2-7},{0-5} |
|   MASK   3   | {0-1},{6-7} |     NA      |     NA      | {6-7},{0-1} |


在 NODE 0 的 DOMAIN-2 那层次 sched_domain_span 为 {0-5}, 但是两个 sched_group 的 span 域分别为 {0-3},{4-7}. 此时第二个 sched_group 中 CPU 6/7 这两个 CPU 超出了 sched_domain {0-5} 的范围.


### 2.2.2 find_busiest_group 的情形
-------

那么在这个 SCHED_DOMAIN 上进行负载均衡 load_balance 的时候, 就会出现 sched_group {4-7} 这个 GROUP 的负载 load 被算轻的情况. 具体可以参照 [update_sg_lb_stats](https://elixir.bootlin.com/linux/v5.10/source/kernel/sched/fair.c#L8370)

被算轻的原因是:

* [update_sg_lb_stats](https://elixir.bootlin.com/linux/v5.10/source/kernel/sched/fair.c#L8370) 中统计 load 的时候, 只统计了在当前 SCHED_DOMAIN 2 中的 CPU.

* 在计算 avg_load 和 group_type 的时候, 用的 group_capacity 却是 {4-7} 这 4 个 CPU 的总和.

这样可能出现的情况是, 比如 4-7 所在的 GROUP 其实很本身忙, 但是统计 load 的只算了 {4-5} 的, 因此会被认为是一个比较空闲的 GROUP.

那么实际运行时, {0-3} 在做负载均衡的时候, 本身可能应该有机会从 4-5 这 2 个 CPU 上 pull 一些任务过来, 但是由于负载均衡的误判, 我们 MISS 掉了这次机会. 

当然我们还是有机会来完成这个 load_balance 的, 在 PARENT 调度域 DOMAIN-3 层次, 这个域已经包含了所有的 CPU, 如果系统中真的存在一些不均衡, 这时候还是有机会把进程迁移到 {0-1} 上的. 也可以满足负载均衡的要求, 只是响应上要慢一些. 

> 同样的情况也发生在 NODE 3 的 DOMAIN 2, sched_domain_span 为 {2-7}, 但是两个 sched_group 的 span 域分别为 {4-7},{0-3}. 此时 {0-3} 这个 GROUP 同样计算时会漏掉 {0-1} 的负载, 被算轻了. 这样这个 GROUP 如果负载本身够大, 但是 {4-7} 在做负载均衡的时候, 可能错失了将 {2-3} 的进程 pull 过来的机会.

### 2.2.3 find_idlest_group 的情形
-------

我们继续看 WAKEUP 的情形, 此时我们想要在 NODE 0 的 DOMAIN-2 层次调度域里面找一个最空闲的 GROUP 和 CPU.

这个流程下是：

* 通过 update_sg_wakeup_stats, 来算每个 SCHED_GROUP 的 group_type 和 avg_load. 但是整个环节, 基本无视 SCHED_DOMAIN 可能无法覆盖所有的 SCHED_GROUP 的情况.

```cpp
// https://elixir.bootlin.com/linux/v5.10/source/kernel/sched/fair.c#L8666
static inline void update_sg_wakeup_stats(struct sched_domain *sd,
            struct sched_group *group,
            struct sg_lb_stats *sgs,
            struct task_struct *p)
{
  // ......

  for_each_cpu(i, sched_group_span(group)) {
  }

  // ......
}
```

* 后面通过 找 GROUP 中搜索最闲的 CPU 的过程, 也同样直接遍历了 SCHED_GROUP 中所有的 CPU, 因此可能搜索到的 CPU 在 SCHED_GROUP 范围内, 但是却不在 SCHED_DOMAIN 的范围内.

```cpp
// https://elixir.bootlin.com/linux/v5.10/source/kernel/sched/fair.c#L5895
/*
 * find_idlest_group_cpu - find the idlest CPU among the CPUs in the group.
 */
static int
find_idlest_group_cpu(struct sched_group *group, struct task_struct *p, int this_cpu)
{
  // ......
  for_each_cpu_and(i, sched_group_span(group), p->cpus_ptr) {
  // ...... 
}
```

也就是说: 明明在 NODE 0 的 DOMAIN-2 层级(span {0-5}) 去搜索 CPU, 但是却搜索到了 SCHED_GROUP {4-7} 中最繁忙的 CPU 6/7(所在节点为 NODE 3).

这个带来的最直接影响就是, 如果 NODE 0 和 NODE 3 之间的 NUMA 距离是超过了 30(node_reclaim_distance = RECLAIM_DISTANCE) 的话, 内核本意其实是不想再同时包含 NODE 0 和 NODE 3 这么远的 DOMAIN 上进行 FORK BALANCE 的, 参见 [`sd_init`](https://elixir.bootlin.com/linux/v5.10/source/kernel/sched/topology.c#L1403), 但是却被异常执行了. 而内核却还不知道这个事情.

> RECLAIM_DISTANCE 的值不同架构可能有不同的实现, 但是多数架构都被定为为 30

可以看到, 对于 NUMA distance 大于 node_reclaim_distance 的层级, SD_BALANCE_EXEC/SD_BALANCE_FORK/SD_WAKE_AFFINE 都是被取消了的.

```cpp
// https://elixir.bootlin.com/linux/v5.10/source/kernel/sched/topology.c#L1403
static struct sched_domain *
sd_init(struct sched_domain_topology_level *tl,
  const struct cpumask *cpu_map,
  struct sched_domain *child, int dflags, int cpu)
{
    // ......

#ifdef CONFIG_NUMA
  } else if (sd->flags & SD_NUMA) {
    sd->cache_nice_tries = 2;

    sd->flags &= ~SD_PREFER_SIBLING;
    sd->flags |= SD_SERIALIZE;
    if (sched_domains_numa_distance[tl->numa_level] > node_reclaim_distance) {
      sd->flags &= ~(SD_BALANCE_EXEC |
               SD_BALANCE_FORK |
               SD_WAKE_AFFINE);
    }

    // ......
}
```

### 2.2.4 总结
-------

这个场景可能存在的问题:


## 2.1 以 1 为边缘节点的情况
-------

如果以 1 为边缘节点的时候,

```cpp
拓扑结构
      2       10      2
  1 <---> 0 <---> 2 <---> 3
```

于是我们的构建的阶梯就是如下层级:

```
NUMA 距离层级(阶梯)
10  -=> 12  -=> 20 -=> 22 -=> 24
```


| SCHED_DOAMIN |   NODE  0   |     NODE  1     |     NODE  2     |     NODE  3     |
|:------------:|:-----------:|:---------------:|:---------------:|:---------------:|
|   DOMAIN  0  |     0-1     |       2-3       |       4-5       |       6-7       |
|   GROUP   0  |   {0},{1}   |     {2},{3}     |     {4},{5}     |     {6},{7}     |
|--------------|-------------|-----------------|-----------------|-----------------|
|   DOMAIN  1  |     0-3     |       0-3       |       4-7       |       4-7       |
|   GROUP   1  | {0-1},{2-3} |   {2-3},{0-1}   |   {4-5},{6-7}   |   {6-7},{4-5}   |
|--------------|-------------|-----------------|-----------------|-----------------|
|   DOMAIN  2  |     0-5     |       0-5       |     0-1,4-7     |     0-1,4-7     |
|   GROUP   2  | {0-3},{4-7} | {0-3},{0-1,4-7} |   {4-7},{0-3}   |   {4-7},{0-5}   |
|   BALANCE 2  |     XXX     | {2-3},{4-----5} |       XXX       |   {6-7},{0-1}   |
|--------------|-------------|-----------------|-----------------|-----------------|
|   DOMAIN  3  |     0-7     |       0-7       |       0-7       |       0-7       |
|   GROUP   3  | {0-5},{4-7} | {0-5},{0-1,4-7} | {0-1,4-7},{0-3} | {0-1,4-7},{0-5} |
|   BALANCE 3  | {0-1},{6-7} | {2-3},{6-----7} | {4-----5},{2-3} | {6-----7},{2-3} |


DOMAIN 2 这个层次, 所有的 NUMA 都有问题. 除了 SCHED_GROUP_SPAN 的并集超出了 SCHED_DOMAIN_SPAN 的范围, 还有一个问题:

部分 SCHED_DOMAIN 两个子 SCHED_GROUP 的 CPU 还存在交集, 这些 SCHED_DOMAIN 都包含了一个 {0-1,4-7} 集合的 SCHED_GROUP.


# 4 问题修复
-------

## 4.1 这个问题该怎么修复 ?
-------

我们在第一篇文章中详细描述了, 3 跳问题的深层次触发原因, 本文又着重讲解了它的影响以及可能导致的问题.
现在是我们考虑如何修复这个问题的时候了.

大致有两种思路:


1.  我们让调度域 SCHED_DOMAIN 以及 调度组 SCHED_GROUP 就这么错下去, 然后修改 LOAD_BALANCE 以及 WAKEUP 的负载均衡处理流程.
这种方法改动教小, 更像是单点修复(规避)性能问题, 但是会引入 sched_domain_span 和 sched_group 做 CPUMASK 交集的额外开销. 需要修改如下几个点: ① load_balance/select_task_rq_fair 时候 SCHED_GROUP 的 group_type 和 avg_load 的更新和计算方法, 分子和分母要协调一致, ② pull 和 push task 的时候, 要把 CPU 限制在 SCHED_DOMAIN_SPAN 和 SCHED_GROUP_SPAN 的交集范围内.

2.  修复调度域的构建, 让内核对 3 跳的 NUMA 拓扑, 构建更适合的调度域结构. 比如以 0 为边缘节点时, 让 CPU 0 的 DOMAIN-2 层次第二个 GROUP 不包含 CPU {6-7}. 这样做其实就是把 1 所需要的 MASK 固化到调度域上, 从而从根本上解决问题, 只影响拓扑域的构建按流程, 负载均衡流程不需要做处理和规避, 但是会打破内核之前高层级的 SCHED_DOMAIN 的 SCHED_GROUP 是由低层次 SCHED_DOMAIN 整体构成的基本原则. 而且改动较大, 将引发整个调度域构建流程的链式修改, 如果处理不慎不甚, 可能引入问题.


## 4.1 Valentin Schneider 第一次尝试修复
-------

`Valentin Schneider` 在发现这个问题之后给出的解决方案 [`sched/topology: Fix overlapping sched_group build`](https://lore.kernel.org/patchwork/patch/1214752), 就是针对方法二的一次尝试.

还记得前面根据 NUMA 距离阶梯, 算出来的各个 `NUMA NODE` 各个层级上的 `cpumask` 信息么

| 距离 | CPU0/1 | CPU2/3 |  CPU4/5   |  CPU6/7   |
|:---:|:------:|:------:|:---------:|:---------:|
| 10 |  {0-1}  |  {2-3} |   {4-5}   |   {6-7}   |
| 12 |  {0-3}  |  {0-3} |   {4-7}   |   {4-7}   |
| 20 |  {0-5}  |  {0-3} | {0-1,4-7} |   {4-7}   |
| 22 |  {0-7}  |  {0-5} |   {0-7}   | {0-1,4-7} |
| 24 |         |  {0-7} |           |   {0-7}   |


最终将依照这个来将各个 `CPU` (或者说各个 `NUMA NODE`) 跟调度域 `sched_domain` 绑定(`attach`).


| SCHED_DOAMIN |   NODE  0   |     NODE  1     |     NODE  2     |     NODE  3     |
|:------------:|:-----------:|:---------------:|:---------------:|:---------------:|
|   DOMAIN 0   |     0-1     |       2-3       |       4-5       |       6-7       |
|   GROUP  0   |   {0},{1}   |     {2},{3}     |     {4},{5}     |     {6},{7}     |
|--------------|-------------|-----------------|-----------------|-----------------|
|   DOMAIN 1   |     0-3     |       0-3       |       4-7       |       4-7       |
|   GROUP  1   | {0-1},{2-3} |   {2-3},{0-1}   |   {4-5},{6-7}   |   {6-7},{4-5}   |
|--------------|-------------|-----------------|-----------------|-----------------|
|   DOMAIN 2   |     0-5     |       0-5       |     0-1,4-7     |     0-1,4-7     |
|   GROUP  2   | {0-3},{4-5} | {0-3},{0-1,4-5} |   {4-7},{0-1}   | {4-7},{0-1,4-5} |
|   MASK   2   |     XXX     | {0-1},{0-----1} |   {4-5},{0-3}   | {XXX},{0-----1} |
|--------------|-------------|-----------------|-----------------|-----------------|
|   DOMAIN 3   |     0-7     |       0-7       |       0-7       |       0-7       |
|   GROUP  3   | {0-5},{4-7} | {0-5},{0-1,4-7} | {0-1,4-7},{0-3} | {0-1,4-7},{0-5} |
|   MASK   3   | {0-1},{XXX} | {2-3},{6-----7} | {4-----7},{0-1} | {6-----7},{2-3} |



## 4.2 Valentin Schneider 发起讨论并添加主动告警
-------

但是上面的补丁并没有被合入, 于是后面 Valentin Schneider 发了添加告警信息的补丁, 让更多的人发现问题以后, 参与到讨论


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:----------:|:----:|
| 2020/8/14 | Valentin Schneider | [sched/topology: NUMA topology limitations](https://lkml.org/lkml/2020/8/14/214) | 发出来讨论此问题 | v1 | [LKML](https://lkml.org/lkml/2020/8/14/214) |
| 2020/11/10 | Valentin Schneider | [sched/topology: Warn when NUMA diameter > 2](https://lore.kernel.org/patchwork/patch/1336369) | 在发现 NUMA 层级超过 2 以后, 提示一个告警 "Shortest NUMA path spans too many nodes" | v1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1336369) |

这个方法很奏效, 越来越多的人发现问题, 并参与讨论. 其中 [sched/topology: NUMA topology limitations](https://lkml.org/lkml/2020/8/14/214) 进行了较多的讨论, 在这次讨论中详细的讲解 numa diameter 的概念以及 3 跳问题触发的根本原因.

*   numa diameter 被解释为 NUMA 域中最远两个节点之间的最短路径上的跳数(边数). 邮件中举了比较多的例子, 这里不一一讨论. 我们只讲解下我们的用例.

如果我们的拓扑结构是以 NODE 1 为边缘(起点)的:


```cpp
      2       10      2
  1 <---> 0 <---> 2 <---> 3
```

最远距离为 1-=> 3, 路径为 1->0->2->3, numa diameter = 3.

如果我们的拓扑结构是以 NODE 0 为边缘(起点)的, numa diameter 同样是 3.

```cpp
拓扑结构
      2       10      2
  0 <---> 1 <---> 2 <---> 3
```



| 时间  | 提问者 | 问题描述 |
|:----:|:-----:|:-------:|
| 2021/1/21 | Meelis Roos | [Shortest NUMA path spans too many nodes](https://lkml.org/lkml/2021/1/21/726) |


## 4.3 性能问题修复
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:---------:|:----:|
| 2021/01/15 | Song Bao Hua (Barry Song) | [sched/fair: first try to fix the scheduling impact of NUMA diameter > 2 1366256 diffmboxseries](https://lore.kernel.org/patchwork/patch/1366256) | 尝试修复 3 跳问题导致的性能蜕化 | RFC | [PatchWork](https://lore.kernel.org/patchwork/cover/1366256) |
| 2021/01/22 | Valentin Schneider | [sched/topology: NUMA distance deduplication](https://lore.kernel.org/patchwork/cover/1369363) | 修复 | v1 | [PatchWork](https://lore.kernel.org/patchwork/cover/1369363), [LKML](https://lkml.org/lkml/2021/1/22/460) |
| 2021/01/27 | Song Bao Hua (Barry Song) | [sched/topology: fix the issue groups don't span domain->span for NUMA diameter > 2](https://lore.kernel.org/patchwork/patch/1378144)| 修复 3 跳问题的拓扑构建 | v2 | [RFC v1](https://lore.kernel.org/patchwork/patch/1366256), [RFC v2](https://lore.kernel.org/patchwork/patch/1371875), [v1](https://lore.kernel.org/patchwork/cover/1373442), [v2](https://lore.kernel.org/patchwork/cover/1375012), [v3](https://lore.kernel.org/patchwork/cover/1378144) |
| 2021/02/03 | Valentin Schneider | [sched/topology: Get rid of overlapping groups](https://lore.kernel.org/patchwork/cover/1375188) | 修复 3 跳问题 | RFC | [PatchWork](https://lore.kernel.org/patchwork/cover/1375188) |

 
宋宝华老师(Barry Song) 提出的 [sched/fair: first try to fix the scheduling impact of NUMA diameter > 2 1366256 diffmboxseries](https://lore.kernel.org/patchwork/patch/1366256) 尝试使用方法一的方式简单规避 3 跳问题引入的性能劣化.

随着讨论的不算继续 Valentin Schneider 完善了自己的




<br>

*	本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*	采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*	基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.
