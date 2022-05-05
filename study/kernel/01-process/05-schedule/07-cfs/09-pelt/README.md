

# 1 PELT 3.8@2012 per-entity load-tracking
-------

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2012/8/23 | [per-entity load-tracking](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e9c84cb8d5f1b1ea6fcbe6190d51dc84b6975938) | per-task 的负载跟踪首次引入内核 | v1 ☑ 3.8 | [LWN](https://lwn.net/Articles/531853), [LORE 00/16](https://lore.kernel.org/lkml/20120823141422.444396696@google.com) |

第一个版本的 PELT 在 3.8 的时候合入主线.


```cpp
e9c84cb8d5f1 sched: Describe CFS load-balancer
f4e26b120b9d sched: Introduce temporary FAIR_GROUP_SCHED dependency for load-tracking
5b51f2f80b3b sched: Make __update_entity_runnable_avg() fast
f269ae0469fc sched: Update_cfs_shares at period edge
48a1675323fa sched: Refactor update_shares_cpu() -> update_blocked_avgs()
82958366cfea sched: Replace update_shares weight distribution with per-entity computation
f1b17280efbd sched: Maintain runnable averages across throttled periods
bb17f65571e9 sched: Normalize tg load contributions against runnable time
8165e145ceb6 sched: Compute load contribution by a group entity
c566e8e9e44b sched: Aggregate total task_group load
aff3e4988444 sched: Account for blocked load waking back up
0a74bef8bed1 sched: Add an rq migration call-back to sched_class
9ee474f55664 sched: Maintain the load contribution of blocked entities
2dac754e10a5 sched: Aggregate load contributed by task entities on parenting cfs_rq
18bf2805d9b3 sched: Maintain per-rq runnable averages
9d85f21c94f7 sched: Track the runnable average on a per-task entity basis
```

当时整个 sched_avg 结构体如下所示:

```cpp
struct sched_avg {
       /*
        * These sums represent an infinite geometric series and so are bound
        * above by 1024/(1-y).  Thus we only need a u32 to store them for for all
        * choices of y < 1-2^(-32)*1024.
        */
       u32 runnable_avg_sum, runnable_avg_period;
       u64 last_runnable_update;
       unsigned long load_avg_contrib;
};
```

| 字段 | 描述 |
|:-------:|:------:|
| runnable_avg_sum | 调度实体 sched_entity 在就绪队列上(on_rq) 累计负载 |
| runnable_avg_period | 调度实体 sched_entity 自创建至今如果持续运行, 所应该达到的累计负载 |
| last_runnable_update | 上次更新 sched_avg 的时间 |
| load_avg_contrib | 进程的负载贡献, running_avg_sum * load_weight / avg_period, 参见 [\_\_update_entity_load_avg_contrib](https://elixir.bootlin.com/linux/v3.8/source/kernel/sched/fair.c#L1402) |


```cpp
# https://elixir.bootlin.com/linux/v3.8/source/kernel/sched/fair.c#L1402
static inline void __update_task_entity_contrib(struct sched_entity *se)
{
    u32 contrib;

    /* avoid overflowing a 32-bit type w/ SCHED_LOAD_SCALE */
    contrib = se->avg.runnable_avg_sum * scale_load_down(se->load.weight);
    contrib /= (se->avg.runnable_avg_period + 1);
    se->avg.load_avg_contrib = scale_load(contrib);
}
```

cfs_rq 上记录的负载信息, 如下所示:

```cpp
# https://elixir.bootlin.com/linux/v3.8/source/kernel/sched/sched.h#L227
#ifdef CONFIG_SMP
/*
 * Load-tracking only depends on SMP, FAIR_GROUP_SCHED dependency below may be
 * removed when useful for applications beyond shares distribution (e.g.
 * load-balance).
 */
#ifdef CONFIG_FAIR_GROUP_SCHED
    /*
     * CFS Load tracking
     * Under CFS, load is tracked on a per-entity basis and aggregated up.
     * This allows for the description of both thread and group usage (in
     * the FAIR_GROUP_SCHED case).
     */
    u64 runnable_load_avg, blocked_load_avg;
    atomic64_t decay_counter, removed_load;
    u64 last_decay;
#endif /* CONFIG_FAIR_GROUP_SCHED */
/* These always depend on CONFIG_FAIR_GROUP_SCHED */
#ifdef CONFIG_FAIR_GROUP_SCHED
    u32 tg_runnable_contrib;
    u64 tg_load_contrib;
#endif /* CONFIG_FAIR_GROUP_SCHED */

    /*
     *   h_load = weight * f(tg)
     *
     * Where f(tg) is the recursive weight fraction assigned to
     * this group.
     */
    unsigned long h_load;
#endif /* CONFIG_SMP */
```

# 2 PELT 4.1@2015 consolidation of CPU capacity and usage
-------


| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2012/8/23 | [consolidation of CPU capacity and usage](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=dfbca41f347997e57048a53755611c8e2d792924) | CPU 调频会导致 CPU capacity 的变化, 在支持 DVFS 的系统还用最大 capacity 计算负载是不合理的, 因此 PELT 感知 capacity 的变化. | v10 ☑ 4.1 | [LORE v10,00/11](https://lore.kernel.org/all/1425052454-25797-1-git-send-email-vincent.guittot@linaro.org), [LKML](https://lkml.org/lkml/2015/2/27/309) |


这组补丁为 PELT 引入了 `frequency scale invariance` 的特性, 当前的系统都是支持 DVFS 的, CPU 处于不同的频率对应的 capacity 肯定是不同的, 特别的对于 big.LITTLE 等架构, 不同 cluster 的能效也是不同的. 因此如果两个相同的进程在不同的频率运行相同的时间, 那么其利用率理论上应该是不一样的.


```cpp
dfbca41f3479 sched: Optimize freq invariant accounting
1aaf90a4b88a sched: Move CFS tasks to CPUs with higher capacity
caff37ef96ea sched: Add SD_PREFER_SIBLING for SMT level
dc7ff76eadb4 sched: Remove unused struct sched_group_capacity::capacity_orig
ea67821b9a3e sched: Replace capacity_factor by usage
8bb5b00c2f90 sched: Calculate CPU's usage statistic and put it into struct sg_lb_stats::group_usage
ca6d75e6908e sched: Add struct rq::cpu_capacity_orig
b5b4860d1d61 sched: Make scale_rt invariant with frequency
0c1dc6b27dac sched: Make sched entity usage tracking scale-invariant
a8faa8f55d48 sched: Remove frequency scaling from cpu_capacity
21f4486630b0 sched: Track group sched_entity usage contributions
36ee28e45df5 sched: Add sched_avg::utilization_avg_contrib
```



| 字段 | 描述 |
|:-------:|:------:|
| runnable_avg_sum | 调度实体 sched_entity 在就绪队列上(on_rq) 累计负载 |
| running_avg_sum | 调度实体 sched_entity 实际在 CPU 上运行(on_cpu) 的累计负载 |
| avg_period  | 调度实体 sched_entity 自创建至今如果持续运行, 所应该达到的累计负载, 等同于原来的 runnable_avg_period, 只是由于这个负载其实跟进程是不是 runnable 没关系, 因此改名 |
| last_runnable_update | 上次更新 sched_avg 的时间 |
| decay_count | 用于计算当前 SE  blocked 状态时的待衰减周期, 每次调度实体出队时, 保存当前 cfs_rq 的 decay_counter, 下次入队更新时, 两个的差值就是已经经历的周期|
| load_avg_contrib | 进程的负载贡献, running_avg_sum * load_weight / avg_period, 参见 [\_\_update_entity_load_avg_contrib](https://elixir.bootlin.com/linux/v4.1/source/kernel/sched/fair.c#L2718) |
| utilization_avg_contrib | 进程的利用率, running_avg_sum * SCHED_LOAD_SCALE / avg_period, 参见 [\_\_update_task_entity_utilization](https://elixir.bootlin.com/linux/v4.1/source/kernel/sched/fair.c#L2744) |


这组补丁为了支持了 `frequency scale invariance`, 因此引入了利用率 utilization 的概念. 它表示进程真正占用 CPU 的比率, 这是一个跟当时运行时 CPU 频率(影响 capacity) 有关的累计平均值. 之前计算的 runnable_avg_sum 以及 load_avg_contrib 都同时包含了调度实体 runnable 和 running 的负载信息. 并不能很好的体现利用率. 利用率更在意的是它真正的运行. 因此 sched_avg 中引入了 running_avg_sum 和 utilization_avg_contrib, 分别表示其占用 CPU 的累计负载和利用率.

在不支持 `frequency scale invariance` 之前, 那么每个 1MS(1024us) 窗口, 其如果处于 R 状态, 那么当前窗口贡献的负载值就是 1024. 感知了频率变化之后, `__update_entity_runnable_avg` 每次在更新负载的时候, delta 会根据当前频率对应的 capacity 进行一个缩放.

```cpp
# https://elixir.bootlin.com/linux/v4.1/source/kernel/sched/fair.c#L2587
# https://elixir.bootlin.com/linux/v4.1/source/kernel/sched/fair.c#L2597
static __always_inline int __update_entity_runnable_avg(u64 now, int cpu,
                            struct sched_avg *sa,
                            int runnable,
                            int running)
{
    // ......
    if (delta + delta_w >= 1024) {
        // ......

        /* Efficiently calculate \sum (1..n_period) 1024*y^i */
        runnable_contrib = __compute_runnable_contrib(periods);
        if (runnable)
            sa->runnable_avg_sum += runnable_contrib;
        if (running)
            sa->running_avg_sum += runnable_contrib * scale_freq
                >> SCHED_CAPACITY_SHIFT;
        sa->avg_period += runnable_contrib;
    }

    /* Remainder of delta accrued against u_0` */
    if (runnable)
        sa->runnable_avg_sum += delta;
    if (running)
        sa->running_avg_sum += delta * scale_freq
            >> SCHED_CAPACITY_SHIFT;
    sa->avg_period += delta;

    return decayed;
}
```

这组补丁的理念中 load 是一个与频率无关的概念, 但是 utilization 利用率从并不是. 如果我们在两个有不同计算能力的 CPU 上运行两个 nice 0 的死循环程序, 那么他们的负载应该是相差不多的, 因为他们的所期望的计算需求是一样的. 因此在计算的时候只有 running_avg_sum 按照 `DELTA * scale_freq / 1024` 的方式缩放了, 而 runnable_avg_sum 并没有. 这样 [\_\_update_task_entity_utilization](https://elixir.bootlin.com/linux/v4.1/source/kernel/sched/fair.c#L2744) 在根据  running_avg_sum 计算 utilization_avg_contrib 的时候, 就会感知到每次 scale_freq 的变化.

# 3 PELT 4.3@2015 Rewrite runnable load and utilization average tracking
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:----------:|:----:|
| 2012/8/23 | Yuyang Du <yuyang.du@intel.com> | [Rewrite runnable load and utilization average tracking](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=7ea241afbf4924c58d41078599f7a32ba49fb985) | 重构组调度的 PELT 跟踪, 在每次更新平均负载的时候, 更新整个 CFS_RQ 的平均负载 | v10 ☑ 4.3-rc1 | [LWN](https://lwn.net/Articles/579066), [LORE v10,0/7](https://lore.kernel.org/all/1436918682-4971-1-git-send-email-yuyang.du@intel.com/), [lkml](https://lkml.org/lkml/2015/7/15/159) |


在之前的 PELT 实现中, 一次更新只跟新一个调度实体的负载变化, 而没有更新 CFS_RQ 上所有调度实体的负载, 这就导致整个就绪队列上 runnable_load_avg 是失真的. 这组补丁对此做了优化. 在每次更新平均负载的时候, 会更新整个 CFS_RQ 的平均负载.

```cpp
7ea241afbf49 sched/fair: Clean up load average references
139622343ef3 sched/fair: Provide runnable_load_avg back to cfs_rq
1269557889b4 sched/fair: Remove task and group entity load when they are dead
540247fb5ddf sched/fair: Init cfs_rq's sched_entity load average
6c1d47c08273 sched/fair: Implement update_blocked_averages() for CONFIG_FAIR_GROUP_SCHED=n
9d89c257dfb9 sched/fair: Rewrite runnable load and utilization average tracking
cd126afe838d sched/fair: Remove rq's runnable avg
```

该补丁合入之后, sched_avg 结构体又发生了重大的变化.

```cpp
# https://elixir.bootlin.com/linux/v4.3/source/include/linux/sched.h#L1204
/*
 * The load_avg/util_avg accumulates an infinite geometric series.
 * 1) load_avg factors the amount of time that a sched_entity is
 * runnable on a rq into its weight. For cfs_rq, it is the aggregated
 * such weights of all runnable and blocked sched_entities.
 * 2) util_avg factors frequency scaling into the amount of time
 * that a sched_entity is running on a CPU, in the range [0..SCHED_LOAD_SCALE].
 * For cfs_rq, it is the aggregated such times of all runnable and
 * blocked sched_entities.
 * The 64 bit load_sum can:
 * 1) for cfs_rq, afford 4353082796 (=2^64/47742/88761) entities with
 * the highest weight (=88761) always runnable, we should not overflow
 * 2) for entity, support any load.weight always runnable
 */
struct sched_avg {
    u64 last_update_time, load_sum;
    u32 util_sum, period_contrib;
    unsigned long load_avg, util_avg;
};
```

| 字段 | 描述 |
|:-------:|:------:|
| last_update_time | 代替原来的 last_runnable_update, 记录上次更新 sched_avg 的时间 |
| load_sum | 接替原来的 runnable_avg_sum, 调度实体 sched_entity 在就绪队列上(on_rq) 累计负载之和, 带权重加权 |
| util_sum | 接替原来的 running_avg_sum, 调度实体 sched_entity 实际在 CPU 上运行(on_cpu) 的累计负载之和 |
| load_avg | 接替原来的 load_avg_contrib, 作为进程的负载贡献 sa->load_sum / LOAD_AVG_MAX, 参见 [`\_\_update_load_avg`](https://elixir.bootlin.com/linux/v4.3/source/kernel/sched/fair.c#L2518) |
| util_avg| 接替 utilization_avg_contrib, 成为进程的利用率 sa->util_sum << SCHED_LOAD_SHIFT) / LOAD_AVG_MAX; |
| period_contrib | 记录了当前进程最后运行的未满一个窗口(1024us) 的剩余时间 |
| ~~avg_period~~ | ~~调度实体 sched_entity 自创建至今如果持续运行, 所应该达到的累计负载,曾用名 runnable_avg_period~~, 用 LOAD_AVG_MAX 替代 |
| ~~decay_count~~ | ~~之前用于计算当前 SE  blocked 状态时的待衰减周期,  实现了 update_blocked_average 之后~~, 新的算法不需要此结构 |

cfs_rq 中新增了 struct sched_avg 字段, 用来记录的负载信息如下所示:

```cpp
# https://elixir.bootlin.com/linux/v4.3/source/kernel/sched/sched.h#L367
# https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9d89c257dfb9c51a532d69397f6eed75e5168c35
struct cfs_rq {
......
#ifdef CONFIG_SMP
    /*
     * CFS load tracking
     */
    struct sched_avg avg;
    u64 runnable_load_sum;
    unsigned long runnable_load_avg;
#ifdef CONFIG_FAIR_GROUP_SCHED
    unsigned long tg_load_avg_contrib;
#endif
    atomic_long_t removed_load_avg, removed_util_avg;
#ifndef CONFIG_64BIT
    u64 load_last_update_time_copy;
#endif
};
```

# 4 PELT 4.4@2015 Compute capacity invariant load/utilization tracking
-------


| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2015/8/14 | [Compute capacity invariant load/utilization tracking](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=98d8fd8126676f7ba6e133e65b2ca4b17989d32c) | PELT 支持 Capacity Invariant, 对之前, 对 frequency scale invariance 的进一步优化 | v1 ☑4.4 | [LWN](https://lwn.net/Articles/531853), [LORE 0/6](https://lore.kernel.org/patchwork/cover/590249), [LKML](https://lkml.org/lkml/2015/8/14/296) |

这组补丁 Morten Rasmussen 自打 2014 年就开始推. 当前, 每个实体的负载跟踪仅对利用率跟踪的频率缩放进行补偿. 这个补丁集也扩展了这种补偿, 并增加了计算能力(不同的微架构和/或最大频率/P-state)的不变性. 前者防止在 cpu 以不同频率运行时做出次优负载平衡决策, 而后者确保可以跨 cpu 比较利用率(sched_avg.util_avg), 并且可以直接将利用率与 cpu 容量进行比较, 以确定cpu是否过载.


```cpp
98d8fd812667 sched/fair: Initialize task load and utilization before placing task on rq
231678b768da sched/fair: Get rid of scaling utilization by capacity_orig
9e91d61d9b0c sched/fair: Name utilization related data and functions consistently
e3279a2e6d69 sched/fair: Make utilization tracking CPU scale-invariant
8cd5601c5060 sched/fair: Convert arch_scale_cpu_capacity() from weak function to #define
e0f5f3afd2cf sched/fair: Make load tracking frequency scale-invariant
```

这组补丁之前只有 util_{sum|avg} 按照 CPU freq 进行了缩放. load_{sum|avg} 还保持着原有的速率. 因此当 CPU 频率降低, 运行速度变慢的时候, 进程的负载会变大. 在负载均衡的算法决策中, 使用的都是 load_avg 的信息, 这就导致了负载均衡决策失误. 因此这组补丁将 load_{sum|avg} 也按照 CPU freq 做了缩放. 参见 [sched/fair: Make load tracking frequency scale-invariant](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e0f5f3afd2cf).

# 5 PELT 4.12@ 2017 Optimize `__update_sched_avg`
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:------:|:---:|
| 2017/10/19 | Yuyang Du | [sched/fair: Add documentation and optimize `__update_sched_avg()`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=283e2ed3990c36c00403b62b264ebfabaf931104) | 异构系统(比如 big.LITTLE) 上通过 DTS 获取 cpu capacity 等信息 | v7 ☑ 4.15 | [LORE 0/2](https://lore.kernel.org/lkml/1486935863-25251-1-git-send-email-yuyang.du@intel.com) |


```cpp
283e2ed3990c sched/fair: Move the PELT constants into a generated header
bb0bd044e65c sched/fair: Increase PELT accuracy for small tasks
3841cdc31099 sched/fair: Fix comments
05296e7535d6 sched/fair: Fix corner case in __accumulate_sum()


76d034edcf65 sched/Documentation: Add 'sched-pelt' tool
a481db34b9be sched/fair: Optimize ___update_sched_avg()
```

Yuyang Du 在 Document 下新增了一个 `sched-pelt.c` 的文件, 用来生成 PELT 需要缓存的数据. 同时 PELT 计算和更新负载的路径 `___update_load_avg` 进行了优化,

```cpp
+/*
+ * Accumulate the three separate parts of the sum; d1 the remainder
+ * of the last (incomplete) period, d2 the span of full periods and d3
+ * the remainder of the (incomplete) current period.
+ *
+ *           d1          d2           d3
+ *           ^           ^            ^
+ *           |           |            |
+ *         |<->|<----------------->|<--->|
+ * ... |---x---|------| ... |------|-----x (now)
+ *
+ *                                p
+ * u' = (u + d1) y^(p+1) + 1024 \Sum y^n + d3 y^0
+ *                               n=1
+ *
+ *    = u y^(p+1) +                            (Step 1)
+ *
+ *                          p
+ *      d1 y^(p+1) + 1024 \Sum y^n + d3 y^0    (Step 2)
+ *                         n=1
+ */
```



# 6 PELT 4.15@ 2017 FIE and CIE support for ARM/ARM64
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:---:|:------:|:---:|
| 2016/10/19 | Dietmar Eggemann | [CPUs capacity information for heterogeneous systems](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=be8f185d8af4dbd450023a42a48c6faa8cbcdfe6) | 异构系统(比如 big.LITTLE) 上通过 DTS 获取 cpu capacity 等信息 | v7 ☑ 4.15 | [LORE v7 REPOST 0/9](https://lore.kernel.org/lkml/20161017154650.18779-1-juri.lelli@arm.com/) |
| 2017/08/26 | Dietmar Eggemann | [arm, arm64, cpufreq: frequency- and cpu-invariant accounting support for task scheduler](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=431ead0ff19b440b1ba25c72d732190b44615cdb) | 为 ARM/ARM64 提供 FIE/CIE 功能 | v5 ☑ 4.15 | [PatchWork](https://lore.kernel.org/lkml/20170926164115.32367-1-dietmar.eggemann@arm.com) |


得益于 ARM big.LITTLE 架构在 ANDROID 的广泛使用, ARM64 是最早支持 FIE 和 CIE 的架构.
关于 FIE 和 CIE 的具体知识, 大家可以参照本博文最后的背景知识介绍 [FIE 和 CIE].

*   FIE 是系统感知同一架构的 CPU 在不同的频点下, 所能提供的计算能力 capacity 是不同的.

*   CIE 则使系统感知, 不同架构(比如 big.LITTLE 的小核和大核虽然都使用 ARM 架构, 但是是性能异构的) 的 CPU 即使处于相同频点下, 所能提供的计算能力 capacity 也是不同的.

# 7 PELT 4.15@2017 A bit of a cgroup/PELT overhaul
-------

## 7.1 补丁列表
-------

Peter 后来进行了一些优化

| 时间  | 作者 |特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:--:|:----------:|:----:|
| 2016/06/17 | Peter Zijlstra | [sched/fair: Fix PELT wobblies](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=7dc603c9028ea5d4354e0e317e8481df99b06d7e) | | v1 ☑ 4.8-rc1 | [PatchWork](https://lore.kernel.org/lkml/20160617120136.064100812@infradead.org) |
| 2017/05/04 | Tejun Heo | [sched/fair: Propagate runnable_load_avg independently from load_avg](https://lore.kernel.org/patchwork/patch/785393) | | v1 ☑ 4.15 | [PatchWork](https://lore.kernel.org/patchwork/patch/782955)<br>*-*-*-*-*-*-*-*<br> [PatchWork](https://lore.kernel.org/patchwork/patch/785395) |
| 2017/09/01 | Peter Zijlstra | [A bit of a cgroup/PELT overhaul](https://lore.kernel.org/patchwork/cover/827575) | | v2 ☑ 4.15 | 2017/05/12 [PatchWork RFC,00/14](https://lore.kernel.org/patchwork/cover/787364)<br>*-*-*-*-*-*-*-*<br> 2017/09/01 [PatchWork -v2,00/18](https://lore.kernel.org/patchwork/cover/827575), [LKML](https://lkml.org/lkml/2017/9/1/331) |


```cpp
17de4ee04ca9 sched/fair: Update calc_group_*() comments
2c8e4dce7963 sched/fair: Calculate runnable_weight slightly differently
9a2dd585b2c4 sched/fair: Implement more accurate async detach
f207934fb79d sched/fair: Align PELT windows between cfs_rq and its se
144d8487bc6e sched/fair: Implement synchonous PELT detach on load-balance migrate
1ea6c46a23f1 sched/fair: Propagate an effective runnable_load_avg
0e2d2aaaae52 sched/fair: Rewrite PELT migration propagation
2a2f5d4e44ed sched/fair: Rewrite cfs_rq->removed_*avg
9059393e4ec1 sched/fair: Use reweight_entity() for set_user_nice()
840c5abca499 sched/fair: More accurate reweight_entity()
8d5b9025f9b4 sched/fair: Introduce {en,de}queue_load_avg()
b5b3e35f4149 sched/fair: Rename {en,de}queue_entity_load_avg()
b382a531b9fe sched/fair: Move enqueue migrate handling
88c0616ee729 sched/fair: Change update_load_avg() arguments
c7b50216818e sched/fair: Remove se->load.weight from se->avg.load_sum
3d4b60d3e3dd sched/fair: Cure calc_cfs_shares() vs. reweight_entity()
cef27403cbe9 sched/fair: Add comment to calc_cfs_shares()
7c80cfc99b7b sched/fair: Clean up calc_cfs_shares()
```

## 7.2 结构体变更
-------

这个版本 sched_avg 结构体变更如下, 参见 [1ea6c46a23f1 sched/fair: Propagate an effective runnable_load_avg](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1ea6c46a23f1)

```cpp
# https://elixir.bootlin.com/linux/v4.15/source/include/linux/sched.h#L278
struct sched_avg {
    u64             last_update_time;
    u64             load_sum;
    u64             runnable_load_sum;
    u32             util_sum;
    u32             period_contrib;
    unsigned long           load_avg;
    unsigned long           runnable_load_avg;
    unsigned long           util_avg;
};
```

| 字段 | 描述 |
|:-------:|:------:|
| last_update_time | 代替原来的 last_runnable_update, 记录上次更新 sched_avg 的时间 |
| load_sum | 接替原来的 runnable_load_sum, 调度实体 sched_entity 在就绪队列上(on_rq) 累计负载 |
| util_sum | 接替原来的 running_avg_sum, 调度实体 sched_entity 实际在 CPU 上运行(on_cpu) 的累计负载 |
| load_avg | 接替原来的 load_avg_contrib, 作为进程的负载贡献 sa->load_sum / LOAD_AVG_MAX, 参见 [`\_\_update_load_avg`](https://elixir.bootlin.com/linux/v4.3/source/kernel/sched/fair.c#L2518) |
| util_avg| 接替 utilization_avg_contrib, 成为进程的利用率 sa->util_sum << SCHED_LOAD_SHIFT) / LOAD_AVG_MAX; |
| runnable_load_sum | 新增字段, 对于进程或者进程组记录了所有调度实体(on_rq) 的累计负载, 对于 CFS_RQ, 记录了当前就绪队列上所有可运行的任务的负载累计之和 |
| runnable_load_avg | 新增字段, 调度时期在就绪队列上的平均累计负载 |
| period_contrib | 记录了当前进程最后运行的未满一个窗口(1024us) 的剩余时间 |

## 7.3 逻辑修改
-------

### 7.3.1 修改了负载和平均负载的计算方式
-------


**首先这组补丁**修改了负载和平均负载的计算方式, 参照 [commit c7b50216818e sched/fair: Remove se->load.weight from se->avg.load_sum](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c7b50216818e) 中的修改.

```cpp
+/*
+ * sched_entity:
+ *
+ *   load_sum := runnable_sum
+ *   load_avg = se_weight(se) * runnable_avg
+ *
+ * cfq_rs:
+ *
+ *   load_sum = \Sum se_weight(se) * se->avg.load_sum
+ *   load_avg = \Sum se->avg.load_avg
+ */
```

通过这个补丁, 将原来更新负载的 `___update_load_avg` 函数拆成了 `___update_load_sum` 和 `___update_load_avg` 两部分, 计算方法也略有不同.

之前的版本中, 所有的负载都直接通过 `___update_load_avg` 中更新,

而该补丁合入后, 负载更新如下:

*   对于调度实体来说, load_sum/runnable_load_sum 等都不再附带有进程的权重 weight 信息, 而在意他是否处于 running or runnable, 直接在 `___update_load_sum` 中更新. 但是最终 load_avg 则需要附带进程的权重信息, 因此将在 `___update_load_avg` 中更新, 计算中将乘以 se_weight(se).

*   对于 CFS_RQ 来说, 我们经常需要使用 runnable_load_avg 负载值比较不同 CPU 之间的负载信息, 他其实是 RQ 上所有调度实体的 load_sum 之和, 因此需要附带 weight 信息的, 因此在 `___update_load_sum` 更新 CFS_RQ 负载的时候, 直接附带了 weight 信息, 而 `___update_load_avg` 中更新 avg 的时候, 则不应该再附带.


### 7.3.2 归一了 CFS_RQ 上 runnable_load_avg 的计算
-------

**其次归一了 CFS_RQ 上 runnable_load_avg 的计算, 并处理了组调度情形下, runnable_load_avg 的计算**, load_balance 比较 CPU 的负载时, 为了体现出等待的压力, 之前一直是使用 runnable_load_avg 的负载, 因此早期版本通过 cfs_rq 上增加了一个 runnable_load_avg 字段来完成. 参见 [139622343ef3 sched/fair: Provide runnable_load_avg back to cfs_rq](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=139622343ef3).




### 7.3.3 引入 group se 的 runnable_weight
-------

参见 [1ea6c46a23f1 sched/fair: Propagate an effective runnable_load_avg](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1ea6c46a23f1).

对于没开组调度的情况

*   runnable_load_avg 是该运行队列上所有可运行任务的直接和.

*   与 load_avg 相反, load_avg 是运行队列上所有任务(task_cpu == this_cpu)的总和, 其中包那些被阻塞的任务

```cpp
runnable_load_avg = \Sum se->avg.load_avg ; where se->on_rq
```

然而, 在开启了 cgroup 的情况下, 情况稍微有点复杂, 因为 group 实体总是可运行的, 即使它的大部分调度实体都被阻塞.

因此, [commit sched/fair: Propagate an effective runnable_load_avg](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1ea6c46a23f1213d1972bfae220db5c165e27bba) 引入 runnable_weight, 它对于任务实体与常规权重相同, 但对于组调度实体 runnable_weight 则只表示组运行队列的可运行部分.

enqueue_runnable_load_avg() 中增加 runnable_weight.
dequeue_runnable_load_avg() 中则减少 runnable_weight.

具体信息可以通过这个 [commit sched/fair: Propagate an effective runnable_load_avg](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1ea6c46a23f1213d1972bfae220db5c165e27bba) 中[注释部分](https://elixir.bootlin.com/linux/v4.15/source/kernel/sched/fair.c#L3252)的修改窥测一二. 以及后续修改注释的补丁 [17de4ee04ca9 sched/fair: Update calc_group_*() comments](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=17de4ee04ca925590df036b112c1db8a778e14bf)


```cpp
 /*
  * sched_entity:
  *
+ *   task:
+ *     se_runnable() == se_weight()
+ *
+ *   group: [ see update_cfs_group() ]
+ *     se_weight()   = tg->weight * grq->load_avg / tg->load_avg
+ *     se_runnable() = se_weight(se) * grq->runnable_load_avg / grq->load_avg
+ *
  *   load_sum := runnable_sum
  *   load_avg = se_weight(se) * runnable_avg
  *
+ *   runnable_load_sum := runnable_sum
+ *   runnable_load_avg = se_runnable(se) * runnable_avg
+ *
+ * XXX collapse load_sum and runnable_load_sum
+ *
  * cfq_rs:
  *
  *   load_sum = \Sum se_weight(se) * se->avg.load_sum
  *   load_avg = \Sum se->avg.load_avg
+ *
+ *   runnable_load_sum = \Sum se_runnable(se) * se->avg.runnable_load_sum
+ *   runnable_load_avg = \Sum se->avg.runable_load_avg
  */
```

具体实现上, 仍然沿用了上面刚提到的 cfs->runnable_load_{sum|avg} 的更新, 这个补丁做了归一, 将 RQ 的负载统计放置到了 sched_avg 上.

然后将此负载 runnable_load_sum 通过 PELT 层次结构传播, 以达到有效的可运行负载平均值, 我们不应该将其与规范的可运行负载平均值混淆.

但是这样依旧有问题, 参见 [commit 2c8e4dce7963 sched/fair: Calculate runnable_weight slightly differently](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=17de4ee04ca925590df036b112c1db8a778e14bf).


可运行权重 runnable_weight 目前是直接据可运行负载与平均负载的比率来调整组.

$runnable\_weight = shares \times  \frac{runnable\_load\_avg}{load\_avg}$

这导致了一个问题, 它让我们倾向于不睡觉的任务. 因此进入睡眠状态的任务将会使其 `runnable_load_avg` 衰减得相当厉害, 这将大大降低交互任务组的可运行权重.

为了解决这种不平衡, 作者稍微调整了一下 runnable_weight 的计算方法, 所以

1.  在理想情况下仍然是上述情况. $runnable\_weight = shares \times  \frac{runnable\_load\_avg}{load\_avg}$

2.  但在交互式情况下, 它是 $runnable\_weight = shares \times \frac{runnable\_weight}{load\_weight}$, 这将使互动性和非互动性组之间的权重分配更加公平。

这个补丁将 runnable_weight 的计算方式修正为:

$shares \times \frac{max(cfs_rq->avg.runnable\_load\_avg, cfs_rq->runnable\_weight)}{max(cfs\_rq->avg.load\_avg, cfs\_rq->load.weight)}$


### 7.3.4 reweight_entity
-------

[840c5abca499 sched/fair: More accurate reweight_entity()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=840c5abca499)

当一个调度实体改变它的权重 load.weight 时, 应该立即改变它的 load_avg, 并将这个改变传播到它所属的负载累计总和中. 因为我们用这些值来预测未来的行为, 而对其历史价值不感兴趣.

如果没有这种变化, 负载的变化将需要通过平均值传播, 到那时它可能再次发生变化.

通过这个更改, cfs_rq->load_{avg|sum} 将更准确地反映当前可运行的和预期的阻塞负载.

### 7.3.4 重写 RQ remove_avg
-------


[2a2f5d4e44ed sched/fair: Rewrite cfs_rq->removed_*avg](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2a2f5d4e44ed)

在进程迁移的时候, 我们需要将进程的负载从原来的 RQ 上移除, 添加到新的 RQ 上. 但是由于唤醒, 迁移等路径, 长期持有旧的 RQ->lock, 会引入性能开销和不稳定因素, 因此我们无法更新 RQ 的负载信息. 因此早期内核就引入了 RQ->removd 机制, 通过异步的方式, 首先通过 remove_entity_load_avg() 将[待删除的负载信息](https://elixir.bootlin.com/linux/v4.14/source/kernel/sched/fair.c#L3530) 添加到一个原子变量中, 并让 CPU 在[下一次更新收集并处理它](https://elixir.bootlin.com/linux/v4.14/source/kernel/sched/fair.c#L3349).

目前我们有两个原子变量: removed_load_avg, removed_util_avg, 但是它们存在一些问题, 即它们可能会被读取为不同步. 而且, 在一条缓存线上进行两次原子操作的开销不见得比争用的 RQ 锁更廉价.

因为这个补丁进行了处理, 将其转换为带有 `cfs_rq->removed.lock` 的实现, 并且锁被标记为 `____cacheline_aligned`.

# 8 PELT 4.17@2018 sched/fair: add util_est on top of PELT
-------

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2020/2/21 | [sched/fair: add util_est on top of PELT](https://lore.kernel.org/patchwork/patch/889504) | 重构组调度的 PELT 跟踪, 在每次更新平均负载的时候, 更新整个 CFS_RQ 的平均负载| V10 ☑ 5.7 | [PatchWork](https://lore.kernel.org/patchwork/cover/1198654), [lkml](https://lkml.org/lkml/2020/2/21/1386) |


由于 `PELT` 衰减的特质, 它并不十分适合终端等场景. 对于 `big.LITTLE` 架构, 一个重量级的任务可能在长时间的睡眠唤醒之后, 进程的利用率 `util` 几乎衰减到微乎其微. 那么它很有可能被唤醒到 `LITTLE` 核上, 对性能造成影响. 其次任务的 `util` 每个周期(1024us) 更新一次, 因此对于一个一直运行的任务, 他的 util 是一个不断变化的值, 那么一个正在运行的任务其瞬时的 `PELT` 利用率直接参与调度器的决策不太合理.

因此内核需要一个平滑的估计值, 一个更稳定的利用率估计能更好的标记 CFS 和 RQ 的负载.


```cpp
d519329f72a6 sched/fair: Update util_est only on util_avg updates
a07630b8b2c1 sched/cpufreq/schedutil: Use util_est for OPP selection
f9be3e5961c5 sched/fair: Use util_est in LB and WU paths
7f65ea42eb00 sched/fair: Add util_est on top of PELT
```

该补丁合入之后, 在 `sched_avg` 结构中新增了一个 `util_est`, 用来标记估计的利用率信息.

```cpp
# https://elixir.bootlin.com/linux/v5.7/source/include/linux/sched.h#L278
# https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7f65ea42eb00bc902f1c37a71e984e4f4064cfa9
+/**
+ * struct util_est - Estimation utilization of FAIR tasks
+ * @enqueued: instantaneous estimated utilization of a task/cpu
+ * @ewma:     the Exponential Weighted Moving Average (EWMA)
+ *            utilization of a task
+ *
+ * Support data structure to track an Exponential Weighted Moving Average
+ * (EWMA) of a FAIR task's utilization. New samples are added to the moving
+ * average each time a task completes an activation. Sample's weight is chosen
+ * so that the EWMA will be relatively insensitive to transient changes to the
+ * task's workload.
+ *
+ * The enqueued attribute has a slightly different meaning for tasks and cpus:
+ * - task:   the task's util_avg at last task dequeue time
+ * - cfs_rq: the sum of util_est.enqueued for each RUNNABLE task on that CPU
+ * Thus, the util_est.enqueued of a task represents the contribution on the
+ * estimated utilization of the CPU where that task is currently enqueued.
+ *
+ * Only for tasks we track a moving average of the past instantaneous
+ * estimated utilization. This allows to absorb sporadic drops in utilization
+ * of an otherwise almost periodic task.
+ */
+struct util_est {
+   unsigned int            enqueued;
+   unsigned int            ewma;
+#define UTIL_EST_WEIGHT_SHIFT      2
+};
+
 /*
  * The load_avg/util_avg accumulates an infinite geometric series
  * (see __update_load_avg() in kernel/sched/fair.c).
@@ -335,6 +363,7 @@ struct sched_avg {
    unsigned long           load_avg;
    unsigned long           runnable_load_avg;
    unsigned long           util_avg;
+   struct util_est         util_est;
 };
```

# 9 PELT 4.19@2018 track CPU utilization
-------

| 时间  | 作者 |特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:---:|:--:|:----:|:----------:|:---:|
| 2018/06/08 | Vincent Guittot | [track CPU utilization](https://lore.kernel.org/patchwork/cover/948961) | 增加了 RT_RQ/DL_RQ 以及 IRQ 的 PELT load tracking | v6 4.19-rc1 | [v4 sched/rt: track rt rq utilization](https://lore.kernel.org/patchwork/patch/934368)<br>*-*-*-*-*-*-*-*<br>[PatchWork v5](https://lore.kernel.org/patchwork/cover/948961) |

linux 内核中调度器提供了多种调度类, 但是到目前为止, PELT 都只有支持 CFS 和 CFS_RQ 的负载均衡. 因此这组补丁为 RT_RQ/DL_RQ 以及 IRQ 增加了 PELT 跟踪.



# 10 PELT 5.1@2019 update scale invariance of PELT
-------

| 时间  | 作者 |特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:----:|:---:|:------:|:---:|
| 2019/01/16 | Vincent Guittot | [sched/fair: update scale invariance of PELT](https://lore.kernel.org/patchwork/cover/1034952) | 将 Frequency Invariance 的计算和更新放到 RQ 上, 而不是按照每个调度实体和 RQ 去完成. | v9 5.1-rc1 | [v3](https://lore.kernel.org/patchwork/patch/784059)<br>*-*-*-*-*-*-*-*<br> [v9](https://lore.kernel.org/patchwork/cover/1034952) |


Frequency Invariance 通过按照 CPU 频率和实际 capacity 对进程运行的时间 delta 进行缩放, 以反映在运行时间内已经完成的实际工作量.

但是之前 scale_delta 的计算和更新都是按每个实体和 rt/dl/cfs_rq 完成, 现在更新了这种算法, 将计算统一放到 RQ 上去进行, 这样只需要在每个 tick 中更新一次, 因此我们不再需要维护每个 rt_rq/dl_rq/cfs_rq 的 sched_avg 和 stolen_idle_time. 确保了所有的 PELT 计算将始终在同一个 RQ 级别进行. 参见[sched/fair: Update scale invariance of PELT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=23127296889fe84b0762b191b5d041e8ba6f2599).

```cpp
10a35e6812aa sched/pelt: Skip updating util_est when utilization is higher than CPU's capacity
23127296889f sched/fair: Update scale invariance of PELT
62478d9911fa sched/fair: Move the rq_of() helper function
```


# 11 PELT 5.7@2020 Support frequency invariance for X86
-------

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2020/1/22 | [Add support for frequency invariance for (some) x86](https://lore.kernel.org/patchwork/cover/1183773) | 支持 x86 FI(frequency invariance) | v5 5.7-rc1 | [Add support for frequency invariance for (some) x86](https://lore.kernel.org/patchwork/cover/1183773) |

得益于 ARM big.LITTLE 架构和 DynamicIQ 架构在安卓的广泛使用, FIE 和 CIE 的支持, ARM64 走在了其他架构的最前面, 但是 X86_64 服务器虽然更看重吞吐量, 但是对功耗的追求也是永恒的话题, 更何况 X86_64 也有一些低端嵌入式芯片, 因此这组补丁补齐了 X86 架构下 FIE 的支持.


# 12 PELT 5.7@2020 remove runnable_load_avg and improve group_classify
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:---:|:----------:|:----:|
| 2020/2/21 | Vincent Guittot | [remove runnable_load_avg and improve group_classify](https://lore.kernel.org/patchwork/cover/1198654) | 重构组调度的 PELT 跟踪, 在每次更新平均负载的时候, 更新整个 CFS_RQ 的平均负载| V10 ☑ 5.7 | [PatchWork](https://lore.kernel.org/patchwork/cover/1198654), [lkml](https://lkml.org/lkml/2020/2/21/1386) |


```cpp
070f5e860ee2 sched/fair: Take into account runnable_avg to classify group
9f68395333ad sched/pelt: Add a new runnable average signal
0dacee1bfa70 sched/pelt: Remove unused runnable load average
6499b1b2dd1b sched/numa: Replace runnable_load_avg by load_avg
6d4d22468dae sched/fair: Reorder enqueue/dequeue_task_fair path
```

之前的负载均衡流程中, 较多的使用了 runnable_load_avg, 但是这也引入了不少问题.

后来 Vincent Guittot 对整个 load_balance 和 numa_balancing 进行了重构和优化.

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2019/10/18 | Vincent Guittot | [sched/fair: rework the CFS load balance](https://linuxplumbersconf.org/event/4/contributions/480) | 重构 load balance | v4 ☑ | [LWN](https://lwn.net/Articles/793427), [PatchWork](https://lore.kernel.org/patchwork/patch/1141687), [lkml](https://lkml.org/lkml/2019/10/18/676) |
| 2020/2/21 | Vincent Guittot | [remove runnable_load_avg and improve group_classify](https://lore.kernel.org/patchwork/cover/1198654) | 重构组调度的 PELT 跟踪, 在每次更新平均负载的时候, 更新整个 CFS_RQ 的平均负载| V10 ☑ 5.7 | [PatchWork](https://lore.kernel.org/patchwork/cover/1198654), [lkml](https://lkml.org/lkml/2020/2/21/1386) |
| 2019/2/24 | Mel Gorman | [Reconcile NUMA balancing decisions with the load balancer](https://linuxplumbersconf.org/event/4/contributions/480) | 重构 load balance | v4 ☑ | [LWN](https://lwn.net/Articles/793427), [PatchWork](https://lore.kernel.org/patchwork/cover/1199507), [lkml](https://lkml.org/lkml/2019/10/18/676) |
| | [Accumulated fixes for Load/NUMA Balancing reconcilation series](https://lore.kernel.org/patchwork/cover/1203922) | fix 补丁 | | |

其中 [rework the CFS load balance](https://lore.kernel.org/patchwork/patch/1141687) 中

[commit b0fb1eb4f04a sched/fair: Use load instead of runnable load in load_balance()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b0fb1eb4f04a) 将 load_balance 路径下原来使用 RQ runnable_load_avg 的地方都修改成了 load_avg.
[commit c63be7be59de sched/fair: Use utilization to select misfit task](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c63be7be59de) 修改了 misfit task 的路径.
[commit 11f10e5420f6 sched/fair: Use load instead of runnable load in wakeup path](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=11f10e5420f6) 修改了唤醒路径.

接着的 [remove runnable_load_avg and improve group_classify](https://lore.kernel.org/patchwork/cover/1198654), Vincent Guittot 进一步修正了 numa_balance 下使用 runnable_load_avg 的路径. 这组补丁包含了几个 RFC 的补丁, 最后的 Mel Gorman 继续完善了 Vincent Guittot 在 numa balance 上的工作. [Reconcile NUMA balancing decisions with the load balancer v6](https://lore.kernel.org/patchwork/cover/1199507)

[6499b1b2dd1b sched/numa: Replace runnable_load_avg by load_avg](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6499b1b2dd1b) 将 numa_balancing 路径下之前使用 cfs_rq_runnable_load_avg(RQ runnable_load_avg) 的地方全改成了 cfs_rq_load_avg(RQ load_avg).

[0dacee1bfa70 sched/pelt: Remove unused runnable load average](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0dacee1bfa70) 前面使用 cfs_rq_runnable_load_avg 的路径都修正以后, 内核已经没有使用 cfs_rq_runnable_load_avg 的地方了, 因此删除了 sched_avg 等结构体中的 runnable_load_{sum|avg} 字段.

[9f68395333ad sched/pelt: Add a new runnable average signal](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9f68395333ad) 前脚把 runnable_load_{sum|avg} 删掉了, 这个补丁找了一种新的 runnable 计算方式来(runnable_{sum|avg})替代旧的方式. 之前 runnable_load_{sum|avg} 的计算包含了太多作者认为不合适的信息(比如 se 和 group 的 runnable 负载 包含了太多的权重 load_weight 信息), 而作者**更希望 runnable 能体现出等待进程的数量, 这个才能突出反应示 CFS_RQ 上的可运行压力**. 新的 runnable 负载计算方式只在意有多少进程在等待, 而不关心他们的 load_weight, 因此可以理解为该信号跟踪 RQ 上任务的等待压力, 有助于更好地定义 RQ 的状态. 这种计算方式与 load_{sum|avg} 的计算方式是类似的, 这样的好处是, 我们**可以直接将 runnable 的负载和 running 的负载进行比较**. 当任务竞争同一个 RQ时, 它们的可运行平均负载将高于 util_avg, 因为它将包含等待时间(不再包含之前的 load_weight 信息), 我们可以使用这个信号更好地对 CFS_RQ 进行分类.


# 13 背景知识
-------

## 13.1 进程的最大运行负载
-------

进程投入运行至今, 如果一直运行那么能达到的负载最大值是多少呢?

最早的 PELT 3.8 版本是在 sched_avg 中存储了一个字段 runnable_avg_period, 用来表示进程自投入运行至今, 假设一直运行, 所能达到的最大负载, 每次 `__update_entity_runnable_avg` 都会更新, 参见 [9d85f21c94f7 sched: Track the runnable average on a per-task entity basis](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9d85f21c94f7f7a84d0ba686c58aa6d9da58fdbb). 此时 runnable_avg_period 的计算公式如下所示:

```cpp
runnable_avg_period(t) = \Sum 1024 * y^i

load_avg_contrib = runnable_load_sum * load.weight / (runnable_avg_period + 1);

```

接着的 PELT 4.1 版本, 引入了利用率, utilization_avg_contrib 的计算跟 load_avg_contrib 类似, 只不过只计算了 running 状态的负载. 这里由于 runnable_avg_period 不再只是跟 runnable 的负载有关系, 因此改名为 avg_period. 参见 [36ee28e45df5 sched: Add sched_avg::utilization_avg_contrib](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=36ee28e45df50c2c8624b978335516e42d84ae1f).

```cpp
# https://elixir.bootlin.com/linux/v4.1/source/include/sched/sched.h#L2718
load_avg_contrib = runnable_avg_sum * load.weight / (avg_period + 1);

# https://elixir.bootlin.com/linux/v4.1/source/include/sched/sched.h#L2744
utilization_avg_contrib = running_avg_sum * SCHED_LOAD_SCALE / (avg_period + 1);
```

后来 PELT 4.3 重构代码的时候, 删除了 avg_period, 转而使用 LOAD_AVG_MAX 替代. 参见 [9d89c257dfb9 sched/fair: Rewrite runnable load and utilization average tracking](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9d89c257dfb9).

```cpp
# https://elixir.bootlin.com/linux/v4.1/source/include/sched/sched.h#L12632
load_avg = load_sum / LOAD_AVG_MAX;

runnable_load_avg = runnable_load_sum /  LOAD_AVG_MAX;

util_avg = util_sum * SCHED_LOAD_SCALE / LOAD_AVG_MAX;
```


使用 LOAD_AVG_MAX 相当于假定最后窗口的 period_contrib 时间已经耗尽, 并且被认为是空闲的. 从而导致 CPU util 的负载永远漏掉了一个窗口的值, 对于一个负载非常重的 util, 它的值本应该保持在 1023 左右, 但是却一直维持在 [1002..1024] 的区间内. 因此在 PELT 4.13, 考虑使用 LOAD_AVG_MAX 直接作为进程的最大负载值是不合理的. 因此补丁 [625ed2bf049d sched/cfs: Make util/load_avg more stable](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=625ed2bf049d5a352c1bcca962d6e133454eaaff) 中考虑了一种最简易的计算方法.

```cpp
LOAD_AVG_MAX * y + 1024(us) = LOAD_AVG_MAX
max_value = LOAD_AVG_MAX * y + sa->period_contrib
```

那么进程如果打创建开始就一直投入运行, 那么能达到的负载最大值为:

```cpp
LOAD_AVG_MAX - 1024 + sa->period_contrib = LOAD_AVG_MAX - (1024 - sa->period_contrib)
```

语义上可以理解为, 最后一个的窗口只运行了 `sa->period_contrib`, 这个窗口不需要衰减.


## 13.2 FIE 和 CIE
-------

内核当前 PELT 在计算负载的时候, 考虑了两个跟 CPU 性能和频率相关的变量(scale_freq 和 scale_cpu
). 如下所示:

```cpp
# https://elixir.bootlin.com/linux/v4.4/source/kernel/sched/fair.c#L2587

static __always_inline int
__update_load_avg(u64 now, int cpu, struct sched_avg *sa,
          unsigned long weight, int running, struct cfs_rq *cfs_rq)
{
    // ......
    scale_freq = arch_scale_freq_capacity(NULL, cpu);
    scale_cpu = arch_scale_cpu_capacity(NULL, cpu);
    // ......
}
```

这些是 EAS 调度器提出的两个概念, 分别对应了 FIE(Frequency Invarient Engine) 和 CIE(CPU Invariant Engine).

1.      FIE 在计算 CPU 负载时要考虑 CPU 频率的变化.

2.      CIE 是考虑不同 CPU 架构(比如 ARM big.LITTLE 架构, 小核使用 A5x, 大核可能是 A7x), 即使在同样的频点下, 不同架构的 CPU 所能提供的计算能力也是不同的.


*   现在的 CPU 都是支持 DVFS 调频的, CPU 在不同的频率下能提供的计算能力是不同的, 为了量化这个值, 内核使用了 scale_freq.


`scale_freq` 表示的是当前 CPU 当前频点下的计算能力按照 1024 的归一, 就是将 CPU当前频率和最高频率的比值, 按照 1024(SCHED_CAPACITY_SCALE) 为最大值进行 scale.

假设最大频率 max_freq 下计算能力为 1024, 那么 curr_freq 下的计算能力是多少呢?

其计算方式如下所示:

```cpp
              curr_freq
scale_freq = ---------- * SCHED_CAPACITY_SCALE
              max_freq
```

举例来说, CPU0 最大频率为 2GHZ, 当前频率为 1.5GHZ, 那么 scale_freq = 1024 * 1.5 / 2 = 768;

[sched/fair: Make load tracking frequency scale-invariant](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e0f5f3afd2cffa96291cd852056d83ff4e2e99c7)


*   对于 big.LITTLE 等性能异构的系统, 不同 cluster 的 CPU 最大计算能力是不同的. scale_cpu 就是用来表示这个 CPU 实际的最大计算能力.
DTS 中可以通过 "capacity-dmips-hmz" 字段配置这个值. 参见内核启动初始化流程 [`topology_parse_cpu_capacity`](https://elixir.bootlin.com/linux/v5.10/source/drivers/base/arch_topology.c#L166).

比如我们拿到的一台手机产品, 4 个 A53 小核(最大频率 1.5G), scale_cpu = 488, 4 个 A72 大核(最大频率 2.0G), scale_cpu = 1024.

ARM 的 Dietmar Eggemann 实现了 CIE 的支持, 参见 [`sched/fair: Make utilization tracking CPU scale-invariant`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=e3279a2e6d697e00e74f905851ee7cf532f72b2d).

```cpp
scale_delta = scale(delta, scale_freq) = delta * scale_freq / SCHED_CAPACITY_SCALE;

  scale(scaled_delta, scale_cpu)
= scaled_delta * scale_cpu / SCHED_CAPACITY_SCALE
= delta * scale_freq * scale_cpu / SCHED_CAPACITY_SCALE^2
```

真正计算 util 的时候, 同时考虑了 FIE 和 CIE.

## 13.3 FI Support
-------

| 时间  | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:------:|:---:|
| 2012/8/23 | [consolidation of CPU capacity and usage](https://lore.kernel.org/patchwork/cover/322242/) | CPU 调频会导致 capacity 的变化, 在支持 DVFS 的系统还用最大 capacity 计算负载是不合理的, 因此 PELT 感知 capacity 的变化  | v1 ☑ 4.1 | [PatchWork](https://lore.kernel.org/patchwork/cover/545867), [lkml](https://lkml.org/lkml/2015/2/27/309) |
| 2015/8/14 | [Compute capacity invariant load/utilization tracking](https://lore.kernel.org/patchwork/cover/590249) | PELT 支持 Capacity Invariant, 对之前, 对 frequency scale invariance 的进一步优化 | V1 ☑4.4 | [LWN](https://lwn.net/Articles/531853), [PatchWork](https://lore.kernel.org/patchwork/cover/590249), [lkml](https://lkml.org/lkml/2015/8/14/296) |
| 2019/01/16 | [sched/fair: update scale invariance of PELT](https://lore.kernel.org/patchwork/cover/1034952) | v9 ☑  5.1-rc1 | [v3](https://lore.kernel.org/patchwork/patch/784059)<br>*-*-*-*-*-*-*-*<br> [v9](https://lore.kernel.org/patchwork/cover/1034952) |





# 14 参考资料
-------

[task 的 load_avg_contrib 的更新参考](https://www.codenong.com/cs106477101)

[CFS调度器：负载跟踪与更新](https://zhuanlan.zhihu.com/p/158185705)

