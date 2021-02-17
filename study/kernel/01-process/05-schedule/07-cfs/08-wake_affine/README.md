title: Linux CFS调度器之唤醒WAKE_AFFINE 机制--Linux进程的管理与调度(三十一)
date: 2020-05-24 15:58
author: gatieme
tags:
	- linux
	- kernel
	- scheduler
categories:
	- 内核探秘
thumbnail:
blogexcerpt: 在进程唤醒的过程中为进程选核时, wake_affine 倾向于将被唤醒进程尽可能安排在 waking CPU 上, 这样考虑的原因是, 有唤醒关系的进程是相互关联的, 尽可能地运行在具有 cache 共享的调度域中, 这样可以获得一些 chache-hit 带来的性能提升. 这时 wake_affine 的初衷, 但是这也是一把双刃剑.

---

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-0729 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](https://blog.csdn.net/gatieme/article/details/106315848) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/kernel/01-process/05-schedule/07-cfs/08-wake_affine) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/details/51456569) |


> 本文更新记录
> 20200513 更新了前言背景内容, 引入 WAKE_AFFINE 机制. 让大家对 WAKE_AFFINE 的目的有一个清楚认识.




# 1 wake_affine 机制
-------

## 1.1    引入 WAKE_AFFINE 的背景
-------


当进程被唤醒的时候（try_to_wake_up）, 需要用 select_task_rq_fair为该 task 选择一个合适的CPU(runqueue), 接着会通过 check_preempt_wakeup 去看被唤醒的进程是否要抢占所在 CPU 的当前进程.


> 关于唤醒抢占的内容, 请参考 [Linux唤醒抢占----Linux进程的管理与调度(二十三）](https://blog.csdn.net/gatieme/article/details/51872831)
>
> 调度器对之前 SLEEP 的进程唤醒后重新入 RUNQ 的时候, 会对进程做一些补偿, 请参考 [Linux CFS调度器之唤醒补偿--Linux进程的管理与调度(三十）](https://blog.csdn.net/gatieme/article/details/52068061)


这个选核的过程我们一般称之为 BALANCE_WAKE. 为了能清楚的描述这个场景, 我们定义

*    执行唤醒的那个进程是 waker
*    而被唤醒的进程是 wakee

Wakeup有两种, 一种是sync wakeup, 另外一种是non-sync wakeup。

*    所谓 sync wakeup 就是 waker 在唤醒 wakee 的时候就已经知道自己很快就进入 sleep 状态, 而在调用 try_to_wake_up 的时候最好不要进行抢占, 因为 waker 很快就主动发起调度了。此外, 一般而言, waker和wakee会有一定的亲和性（例如它们通过share memory进行通信）, 在SMP场景下, waker和wakee调度在一个CPU上执行的时候往往可以获取较佳的性能。而如果在try_to_wake_up的时候就进行调度, 这时候wakee往往会调度到系统中其他空闲的CPU上去。这时候, 通过sync wakeup, 我们往往可以避免不必要的CPU bouncing。
*    对于non-sync wakeup而言, waker和wakee没有上面描述的同步关系, waker在唤醒wakee之后, 它们之间是独立运作, 因此在唤醒的时候就可以尝试去触发一次调度。

当然, 也不是说sync wakeup就一定不调度, 假设waker在CPU A上唤醒wakee, 而根据wakee进程的cpus_allowed成员发现它根本不能在CPU A上调度执行, 那么管他sync不sync, 这时候都需要去尝试调度(调用reschedule_idle函数), 反正waker和wakee命中注定是天各一方(在不同的CPU上执行).


select_task_rq_fair 的原型如下:

```cpp
int select_task_rq_fair(struct task_struct *p, int prev_cpu, int sd_flag, int wake_flags)
```

在 try_to_wake_up 场景其中 p 是待唤醒进程, prev_cpu 是进程上次运行的 CPU, 一般 sd_flag 是 BALANCE_WAKE, 因此其实wakeup 的过程也可以理解为一次主动 BALANCE 的过程, 成为 WAKEUP BALANCE, 只不过只是为一个进程选择唤醒到的 CPU. wake_flags 用于表示是 sync wakeup 还是 non-sync wakeup.


我们首先看看UP上的情况。这时候waker和wakee在同一个CPU上运行（当然系统中也只有一个CPU, 哈哈）, 这时候谁能抢占CPU资源完全取决于waker和wakee的动态优先级(调度类优先级, 或者 CFS 的 vruntime 等, 依照进程的调度类而定), 如果wakee的动态优先级大于waker, 那么就标记waker的need_resched标志, 并在调度点到来的时候调用schedule函数进行调度。

SMP情况下, 由于系统的CPU资源比较多, waker和wakee没有必要争个你死我活, wakee其实也可以选择去其他CPU执行, 但是这时候要做决策:

*    因为跑到 prev_cpu 上, 那么之前如果 cache 还是 hot 的是很有意义的
*    同时按照之前的假设 waker 和 wakee 之间有资源共享, 那么唤醒到 waker CPU 上也有好处
*    如果 prev_cpu, waker cpu 都很忙, 那放上来可以并不一定好, 唤醒延迟之类的都是一个考量.



那么这些都是一个综合权衡的过程, 我们要考虑的东西比较多

*    wake_cpu, prev_cpu 到底该不该选择？
*    选择的话选择哪个?
*    它们都不合适的时候又要怎么去选择一个更合适的?


内核需要一个简单有效的机制去做这个事情, 因此 WAKE_AFFINE 出现在内核中.


## 1.2	WAKE_AFFINE 机制简介
-------

[`select_task_rq_fair`]() 选核其实是一个优选的过程, 通常会有限选择一个 cache-miss 等开销最小的一个

1. 根据 wake_affine 选择调度域并确定 new_cpu

2. 根据调度域及其调度域参数选择兄弟 idle cpu 根据调度域及其调度域参数选择兄弟 idle cpu

3. 根据调度域选择最深idle的cpu根据调度域选择最深idle的cpu find_idest_cpu

在进程唤醒的过程中为进程选核时, wake_affine 倾向于将被唤醒进程尽可能安排在 waking CPU 上, 这样考虑的原因是: 有唤醒关系的进程是相互关联的, 尽可能地运行在具有 cache 共享的调度域中, 这样可以获得一些 chache-hit 带来的性能提升. 这时 wake_affine 的初衷, 但是这也是一把双刃剑.

将 wakee 都唤醒在 waker CPU 上, 必然造成 waker 和 wakee 的资源竞争. 特别是对于 1:N 的任务模型, wake_affine 会导致 waker 进程饥饿. 

62470419e993f8d9d93db0effd3af4296ecb79a5    sched: Implement smarter wake-affine logic

因此后来 (COMMIT 62470419e993 "sched: Implement smarter wake-affine logic"), 实现了一种智能 wake-affine 的优化机制. 用于 wake_flips 的巧妙方式, 识别出 1:N 等复杂唤醒模型, 只有在认为 wake_affine 能提升性能时(want_affine)才进行 wake_affine.


# 2    wake_affine 机制分析
-------

根据 want_affine 变量选择调度域并确定 new_cpu


我们知道如下的事实 :

* 进程p的调度域参数设置了SD_BALANCE_WAKE

* 当前cpu的唤醒次数没有超标

* 当前task p消耗的capacity * 1138小于min_cap * 1024

* 当前cpu在task p的cpu亲和数里面的一个


```cpp
// https://elixir.bootlin.com/linux/v5.1.15/source/kernel/sched/fair.c#L6674
static int
select_task_rq_fair(struct task_struct *p, int prev_cpu, int sd_flag, int wake_flags)
{
 struct sched_domain *tmp, *sd = NULL;
 int cpu = smp_processor_id();
 int new_cpu = prev_cpu;
 int want_affine = 0;
 int sync = (wake_flags & WF_SYNC) && !(current->flags & PF_EXITING);

 if (sd_flag & SD_BALANCE_WAKE) {
  record_wakee(p);

  if (sched_energy_enabled()) {
   new_cpu = find_energy_efficient_cpu(p, prev_cpu);
   if (new_cpu >= 0)
    return new_cpu;
   new_cpu = prev_cpu;
  }

  want_affine = !wake_wide(p) && !wake_cap(p, cpu, prev_cpu) &&
         cpumask_test_cpu(cpu, &p->cpus_allowed);
 }

 rcu_read_lock();
 for_each_domain(cpu, tmp) {
  if (!(tmp->flags & SD_LOAD_BALANCE))
   break;

  /*
   * If both 'cpu' and 'prev_cpu' are part of this domain,
   * cpu is a valid SD_WAKE_AFFINE target.
   */
  if (want_affine && (tmp->flags & SD_WAKE_AFFINE) &&
      cpumask_test_cpu(prev_cpu, sched_domain_span(tmp))) {
   if (cpu != prev_cpu)
    new_cpu = wake_affine(tmp, p, cpu, prev_cpu, sync);

   sd = NULL; /* Prefer wake_affine over balance flags */
   break;
  }

  if (tmp->flags & sd_flag)
   sd = tmp;
  else if (!want_affine)
   break;
 }
```

*    wake_wide 和 wake_cap 为调度器提供决策, 当前进程是否符合 wake_affine 的决策模型. 如果他们返回 1, 则说明如果采用 wake_affine 进行决策, 大概率是无效的或者会降低性能, 则调度器就不会 want_affine 了.

```cpp
want_affine = !wake_wide(p) && !wake_cap(p, cpu, prev_cpu) &&
         cpumask_test_cpu(cpu, &p->cpus_allowed);
```

wake_wide 检查当前cpu的唤醒关系符合 wake_affine 模型.
wake_cap 检查当前 task p 消耗的 CPU capacity 没有超出当前 CPU 的限制.
task p 可以在当前 CPU 上运行.

*    wake_affine 则为目标进程选择最合适运行的 wake CPU.

## 2.1    want_affine
-------

有 wakeup 关系的进程都是相互关联的进程, 那么大概率 waker 和 wakee 之间有一些数据共享, 这些数据可能是 waker 进程刚刚准备好的, 还在 cache 里面, 那么把它唤醒到 waking CPU, 就能充分利用这些在 cache 中的数据. 但是另外一方面, waker 之前在  prev CPU 上运行, 那么也是 cache-hot 的, 把它迁移到 waking CPU 上, 那么 prev CPU 上那些 cache 就有可能失效, 因此如果 waker 和 wakee 之间没有数据共享或者共享的数据没那么多, 那么wake_affine 直接迁移到 waking CPU 上反而是不合适的.

内核引入 wake_affine 的初衷就是识别什么时候要将 wakee 唤醒到 waking CPU, 什么时候不需要. 这个判断由 want_affine 通过 wake_cap() 和 wake_wide() 来完成.


### 2.2.1    record_wakee 与 wakee_flips
-------

>通过在 struct task_struct 中增加两个成员: 上次唤醒的进程 last_wakee, 和累积唤醒翻转计数器. 每当 waker 尝试唤醒 wakee 的时候, 就通过 record_wakee 来更新统计计数.

在 select_task_rq_fair 开始的时候, 如果发现是 SD_BALANCE_WAKE,  则先会 record_wakee 统计 current 的 wakee_flips.

```cpp
static int
select_task_rq_fair(struct task_struct *p, int prev_cpu, int sd_flag, int wake_flags)
{
        if (sd_flag & SD_BALANCE_WAKE) {
                record_wakee(p);
```

wakee_flips 表示了当前进程作为 waker 时翻转(切换)其唤醒目标的次数, 所以高 wakee_flips 值意味着任务不止一个唤醒, 数字越大, 说明当前进程又不止一个 wakee, 而且唤醒频率越比较高. 且当前进程的 wakerr 数目 < wakee_flips.

比如一个进程 P 连续一段时间的唤醒序列为: A, A, A, A, 那么由于没有翻转, 那么他的 wakee_flips 就始终为 1.

```cpp
static void record_wakee(struct task_struct *p)
{
        /*
         * Only decay a single time; tasks that have less then 1 wakeup per
         * jiffy will not have built up many flips.
         */
        if (time_after(jiffies, current->wakee_flip_decay_ts + HZ)) {
                current->wakee_flips >>= 1;
                current->wakee_flip_decay_ts = jiffies;
        }

        if (current->last_wakee != p) {
                current->last_wakee = p;
                current->wakee_flips++;
        }
}
```

wakee_flips 有一定的衰减期, 如果过了 1S (即 1 个 HZ 的时间), 那么 wakee_flips 就衰减为原来的 1/2, 这就类似于 PELT 的指数衰减, Ns 前的 wakee_flips 的占比大概是当前这一个窗口的 1 / 2^N;


>全局变量jiffies用来记录自系统启动以来产生的节拍的总数(经过了多少tick). 启动时, 内核将该变量初始化为0, 此后, 每次时钟中断处理程序都会增加该变量的值.一秒内时钟中断的次数等于Hz, 所以jiffies一秒内增加的值也就是Hz.系统运行时间以秒为单位, 等于jiffies/Hz.
>将以秒为单位的时间转化为jiffies：
>seconds * Hz
>将jiffies转化为以秒为单位的时间：
>jiffies / Hz
>
>jiffies记录了系统启动以来, .
>
>一个tick代表多长时间, 在内核的CONFIG_HZ中定义.比如CONFIG_HZ=200, 则一个jiffies对应5ms时间.所以内核基于jiffies的定时器精度也是5ms



### 2.2.2    wake_wide
-------

当前 current 正在为 wakeup p, 并为 p 选择一个合适的 CPU. 那么 wake_wide 就用来检查 current 和 p 之间是否适合 wake_affine 所关心的 waker/wakee 模型.

wake_wide 返回 0, 表示 wake_affine 是有效的. 否则返回 1, 表示这两个进程不适合用 wake_affine.

那么什么时候, wake_wide 返回 1 ?

```cpp
/*
 * Detect M:N waker/wakee relationships via a switching-frequency heuristic.
 *
 * A waker of many should wake a different task than the one last awakened
 * at a frequency roughly N times higher than one of its wakees.
 *
 * In order to determine whether we should let the load spread vs consolidating
 * to shared cache, we look for a minimum 'flip' frequency of llc_size in one
 * partner, and a factor of lls_size higher frequency in the other.
 *
 * With both conditions met, we can be relatively sure that the relationship is
 * non-monogamous, with partner count exceeding socket size.
 *
 * Waker/wakee being client/server, worker/dispatcher, interrupt source or
 * whatever is irrelevant, spread criteria is apparent partner count exceeds
 * socket size.
 */
static int wake_wide(struct task_struct *p)
{
        unsigned int master = current->wakee_flips;
        unsigned int slave = p->wakee_flips;
        int factor = this_cpu_read(sd_llc_size);

        if (master < slave)
                swap(master, slave);
        if (slave < factor || master < slave * factor)
                return 0;
        return 1;
}
```



wake_affine 在决策的时候,  要参考 wakee_flips

1.    将 wakee_flips 值大的 wakee 唤醒到临近的 CPU,  可能有利于系统其他进程的唤醒,  同样这也意味着,  waker 将面临残酷的竞争.
2.    此外,  如果 waker 也有一个很高的 wakee_flips,  那意味着多个任务依赖它去唤醒,  然后 1 中造成的 waker 的更高延迟会对这些唤醒造成负面影响,  因此一个高 wakee_flips 的 waker 再去将另外一个高 wakee_flips 的 wakee 唤醒到本地的 CPU 上,  是非常不明智的决策. 因此,  当 waker-> wakee_flips / wakee-> wakee_flips 变得越来越高时,  进行 wake_affine 操作的成本会很高.


理解了这层含义, 那我们 wake_wide 的算法就明晰了. 如下情况我们认为决策是有效的 wake_affine 

> factor = this_cpu_read(sd_llc_size); 这个因子表示了在当前 NODE 上能够共享 cache 的 CPU 数目(或者说当前sched_domain 中 CPU 的数目),   一个 sched_domain 中,   共享 chache 的 CPU 越多(比如 X86 上一个物理 CPU 上包含多个逻辑 CPU),   factor 就越大. 那么在 wake_affine 中的影响就是 wake_wide 返回 0 的概率更大,   那么 wake_affine 的结果有效的概率就更大. 因为有跟多的临近 CPU 可以选择,   这些 CPU 之间 cache 共享有优势.

| 条件 | 描述 |
|:----:|:---:|
| slave  < factor | 即如果 wakee->wakee_flips < factor,   则说明当前进程的唤醒切换不那么频繁,   即使当前进程有 wakee_flips 个 wakee,   当前 sched_domain 也完全能装的下他们. |
| master < slave * factor | 即 master/slave < factor,   两个 waker wakee_flips 的比值小于 factor, 那么这种情况下, 进行 wake_affine 的成本可控. |

| commit | patchwork | lkml |
|:------:|:---------:|:----:|
| [63b0e9edceec sched/fair: Beef up wake_wide](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=63b0e9edceec10fa41ec33393a1515a5ff444277) | https://lore.kernel.org/patchwork/patch/576823 | https://lkml.org/lkml/2015/7/8/40 |

### 2.2.3    wake_cap
-------

由于目前有一些 CPU 都是属于性能异构的 CPU(比如 ARM64 的 big.LITTLE 等), 不同的核 capacity 会差很多. wake_cap 会先看待选择的进程是否

```cpp
https://elixir.bootlin.com/linux/v5.6.13/source/kernel/sched/fair.c#L6128
/*
 * Disable WAKE_AFFINE in the case where task @p doesn't fit in the
 * capacity of either the waking CPU @cpu or the previous CPU @prev_cpu.
 *
 * In that case WAKE_AFFINE doesn't make sense and we'll let
 * BALANCE_WAKE sort things out.
 */
static int wake_cap(struct task_struct *p, int cpu, int prev_cpu)
{
 long min_cap, max_cap;

 if (!static_branch_unlikely(&sched_asym_cpucapacity))
  return 0;

 min_cap = min(capacity_orig_of(prev_cpu), capacity_orig_of(cpu));
 max_cap = cpu_rq(cpu)->rd->max_cpu_capacity;

 /* Minimum capacity is close to max, no need to abort wake_affine */
 if (max_cap - min_cap < max_cap >> 3)
  return 0;

 /* Bring task utilization in sync with prev_cpu */
 sync_entity_load_avg(&p->se);

 return !task_fits_capacity(p, min_cap);
}
```


注意在 [sched/fair: Capacity aware wakeup rework](https://lkml.org/lkml/2020/2/6/680) 合入之后, 通过 select_idle_sibling-=>elect_idle_capacity 让 wakeup 感知了 capacity, 因此 原生的 wakeup 路径无需再做 capacity 相关的处理, 因此 wake_cap 就被干掉了. 参见[sched/fair: Remove wake_cap()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/kernel/sched/fair.c?id=000619680c3714020ce9db17eef6a4a7ce2dc28b)

## 2.3    wake_affine
-------

如果 want_affine 发现对当前 wakee 进行 wake_affine 是有意义的, 那么就会为当前进程选择一个能尽快运行的 CPU. 它总是倾向于选择 waking CPU(this_cpu) 以及 prev_cpu.

其中

*    wake_affine_idle 则看 prev_cpu 以及 this_cpu 是不是处于 cache 亲和的以及是不是idle 状态, 这样的 CPU
往往是最佳的.

*    wake_affine_weight 则进一步考虑进程的负载信息以及调度的延迟信息.

```cpp
/*
 * The purpose of wake_affine() is to quickly determine on which CPU we can run
 * soonest. For the purpose of speed we only consider the waking and previous
 * CPU.
 *
 * wake_affine_idle() - only considers 'now', it check if the waking CPU is
 * cache-affine and is (or will be) idle.
 *
 * wake_affine_weight() - considers the weight to reflect the average
 * scheduling latency of the CPUs. This seems to work
 * for the overloaded case.
 */
static int wake_affine(struct sched_domain *sd, struct task_struct *p,
                       int this_cpu, int prev_cpu, int sync)
{
        int target = nr_cpumask_bits;

        if (sched_feat(WA_IDLE))
                target = wake_affine_idle(this_cpu, prev_cpu, sync);

        if (sched_feat(WA_WEIGHT) && target == nr_cpumask_bits)
                target = wake_affine_weight(sd, p, this_cpu, prev_cpu, sync);

        schedstat_inc(p->se.statistics.nr_wakeups_affine_attempts);
        if (target == nr_cpumask_bits)
                return prev_cpu;

        schedstat_inc(sd->ttwu_move_affine);
        schedstat_inc(p->se.statistics.nr_wakeups_affine);
        return target;
}
```


### 2.3.1    负载计算方式
-------

wake_affine 函数源码分析之前, 需要先知道三个load的计算方式如下:

```cpp
source_load(int cpu, int type)
target_load(int cpu, int type)target_load(int cpu, int type)
effective_load(struct task_group *tg, int cpu, long wl, long wg)
```

根据调度类和 "nice" 值, 对迁移源 CPU 和目的 CPU 的负载 source_load 和 target_load 进行估计.
对于 source_load 我们采用保守的方式进行估计, 对于 target_load 则倾向于偏激. 因此当 type 传入的值非 0 时, source_load 返回最小值, 而 target_load 返回最大值. 当 type == 0 时, 将直接返回 weighted_cpuload

```cpp
#https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#5258
/*
 * Return a low guess at the load of a migration-source CPU weighted
 * according to the scheduling class and "nice" value.
 *
 * We want to under-estimate the load of migration sources, to
 * balance conservatively.
 */
static unsigned long source_load(int cpu, int type)
{
        struct rq *rq = cpu_rq(cpu);
        unsigned long total = weighted_cpuload(rq);

        if (type == 0 || !sched_feat(LB_BIAS))
                return total;

        return min(rq->cpu_load[type-1], total);
}
```

```cpp
#https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#5280
/*
 * Return a high guess at the load of a migration-target CPU weighted
 * according to the scheduling class and "nice" value.
 */
static unsigned long target_load(int cpu, int type)
{
        struct rq *rq = cpu_rq(cpu);
        unsigned long total = weighted_cpuload(rq);

        if (type == 0 || !sched_feat(LB_BIAS))
                return total;

        return max(rq->cpu_load[type-1], total);
}
```


```cpp
#https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#5139
/* Used instead of source_load when we know the type == 0 */
static unsigned long weighted_cpuload(struct rq *rq)
{
        return cfs_rq_runnable_load_avg(&rq->cfs);
}
```

### 2.3.2    wake_affine_idle
-------

```cpp
static int
wake_affine_idle(int this_cpu, int prev_cpu, int sync)
{
        /*
         * If this_cpu is idle, it implies the wakeup is from interrupt
         * context. Only allow the move if cache is shared. Otherwise an
         * interrupt intensive workload could force all tasks onto one
         * node depending on the IO topology or IRQ affinity settings.
         *
         * If the prev_cpu is idle and cache affine then avoid a migration.
         * There is no guarantee that the cache hot data from an interrupt
         * is more important than cache hot data on the prev_cpu and from
         * a cpufreq perspective, it's better to have higher utilisation
         * on one CPU.
         */
        if (available_idle_cpu(this_cpu) && cpus_share_cache(this_cpu, prev_cpu))
                return available_idle_cpu(prev_cpu) ? prev_cpu : this_cpu;

        if (sync && cpu_rq(this_cpu)->nr_running == 1)
                return this_cpu;

        return nr_cpumask_bits;
}
```

如果 this_cpu 空闲, 则意味着唤醒来自中断上下文. 仅在 this_cpu 和 prev_cpu 有共享缓存时允许移动. 否则, 中断密集型工作负载可能会将所有任务强制到一个节点, 具体取决于IO拓扑或IRQ亲缘关系设置. 同时如果 prev_cpu 也是空闲的, 优先 prev_cpu.

另外没有证据保证来自中断的缓存热数据比 prev_cpu 上的缓存热数据更重要, 并且从cpufreq的角度来看, 最好在一个CPU上获得更高的利用率.




### 2.3.3    wake_affine_weight
-------

`wake_affine_weight` 会重新计算 `wakeup CPU` 和 `prev CPU` 的负载情况, 如果 `wakeup CPU` 的负载加上唤醒进程的负载比 `prev CPU` 的负载小, 那么 `wakeup CPU` 是可以唤醒进程.

```cpp
static int
wake_affine_weight(struct sched_domain *sd, struct task_struct *p,
                   int this_cpu, int prev_cpu, int sync)
{
        s64 this_eff_load, prev_eff_load;
        unsigned long task_load;

        this_eff_load = target_load(this_cpu, sd->wake_idx);

        if (sync) {
                unsigned long current_load = task_h_load(current);

                if (current_load > this_eff_load)
                        return this_cpu;

                this_eff_load -= current_load;
        }

        task_load = task_h_load(p);

        this_eff_load += task_load;
        if (sched_feat(WA_BIAS))
                this_eff_load *= 100;
        this_eff_load *= capacity_of(prev_cpu);

        prev_eff_load = source_load(prev_cpu, sd->wake_idx);
        prev_eff_load -= task_load;
        if (sched_feat(WA_BIAS))
                prev_eff_load *= 100 + (sd->imbalance_pct - 100) / 2;
        prev_eff_load *= capacity_of(this_cpu);

        /*
         * If sync, adjust the weight of prev_eff_load such that if
         * prev_eff == this_eff that select_idle_sibling() will consider
         * stacking the wakee on top of the waker if no other CPU is
         * idle.
         */
        if (sync)
                prev_eff_load += 1;

        return this_eff_load < prev_eff_load ? this_cpu : nr_cpumask_bits;
}
```

我们假设将进程从 prev CPU 迁移到了 wakeup CPU, 那么 this_eff_load  记录了迁移后 wakeup CPU 的负载, 那么 prev_eff_load 则是迁移后 prev CPU 的负载.

eff_load 的计算方式为:

$$
x={[cpu\_load\pm\task\_h\_load(p)]\times{capacity\_of(cpu)}}
$$

注意使用 wake_affine_weight 需要开启 WA_WEIGHT 开关

| 描述 | commit | 分析 |
|:---:|:------:|:---:|
| smart wake-affine([lkml](https://lkml.org/lkml/2017/10/14/172),patchwork) |

wake_affine_weight 中负载比较的部分经历了很多次的修改.
[eeb603986391 sched/fair: Defer calculation of 'prev_eff_load' in wake_affine_weight() until needed](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=eeb60398639143c11ff2c8b509e3a471411bb5d3)
[082f764a2f3f sched/fair: Do not migrate on wake_affine_weight() if weights are equal](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=082f764a2f3f2968afa1a0b04a1ccb1b70633844)
[1c1b8a7b03ef sched/fair: Replace source_load() & target_load() with weighted_cpuload()](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1c1b8a7b03ef50f80f5d0c871ee261c04a6c967e), 这个是 [sched: remove cpu_loads](https://lore.kernel.org/patchwork/patch/456549/) 中的一个补丁, 该补丁集删除了 cpu_load idx 干掉了 LB_BIAS 特性, 它指出 LB_BIAS 的设计本身是有问题的, 在负载均衡迁移时平滑两个 cpu_load 的过程中, 用 source_load/target_load 的方式在源 CPU 和目的 CPU 上用一个随机偏差的方式是错误的, 这个平衡偏差应该取决于cpu组之间的任务转移成本, 而不是随机历史记录或即时负载。因为历史负载可能与实际负载相差很大, 从而导致不正确的偏差.
[11f10e5420f6c sched/fair: Use load instead of runnable load in wakeup path](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=11f10e5420f6cecac7d4823638bff040c257aba9) https://lore.kernel.org/patchwork/patch/1141693, 该补丁是 rework load balancce 的一个补丁, 之前唤醒路径用下的是 cpu_runnable_load, 现在修正为 cpu_load. cpu_load 对应的是 rq 的 load_avg, 代表就绪队列平均负载, 其包含睡眠进程的负载贡献, cpu_runnable_load 则是 runnable_load_avg只包含就绪队列上所有可运行进程的负载贡献,  wakeup 的时候如果使用 cpu_runnable_load 则可能造成选核的时候选择到一个有很多 runnable 线程的 overloaded 的 CPU, 而不是一个有很多 blocked 线程, 但是还有很大空闲的 CPU. 因此使用 cpu_load 在 wakeup 的时候可能更好.
当前内核版本 5.6.13 中 wake_affine_weight 的实现[参见](https://elixir.bootlin.com/linux/v5.6.13/source/kernel/sched/fair.c#L5556), 跟我们前面将的思路没有太大变化, 但是没有了 LB_BIAS, 同时比较负载使用的是 cpu_load().

## 2.4    wake_affine 演进
-------

Michael Wang 实现了 Smart wake affine, 引入 wakee_flips 来识别 wake-affine 的场景. 然后 Peter 做了一个简单的优化, factor 使用了 sd->sd_llc_size 而不是直接获取所在NODE 的 CPU 数目. nr_cpus_node(cpu_to_node(smp_processor_id()));

| commit | lkml | patchwork |
|:------:|:----:|:---------:|
| [62470419e993 sched: Implement smarter wake-affine logic](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=62470419e993f8d9d93db0effd3af4296ecb79a5)<br>[7d9ffa896148 sched: Micro-optimize the smart wake-affine logic](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=7d9ffa8961482232d964173cccba6e14d2d543b2) | https://lkml.org/lkml/2013/7/4/18 | https://lore.kernel.org/patchwork/cover/390846 |

接着 Vincent Guittot 和 Rik van Riel 做了不少功耗优化的工作. 这时候 wake-affne 中开始考虑 CPU capacity 的信息.

| commit | lkml | patchwork |
|:------:|:----:|:---------:|
| [05bfb65f52cb sched: Remove a wake_affine() condition]()<br>[bd61c98f9b3f sched: Test the CPU's capacity in wake_affine()]() | https://lkml.org/lkml/2014/5/23/458 | XXXX |

然后 Rik van Riel 在 NUMA 层次支持了 wake_affine 

| commit | lkml | patchwork |
|:------:|:----:|:---------:|
| 739294fb03f5 sched/numa: Override part of migrate_degrades_locality() when idle balancing<br>7d894e6e34a5 sched/fair: Simplify wake_affine() for the single socket case<br>3fed382b46ba sched/numa: Implement NUMA node level wake_affine()<br>815abf5af45f sched/fair: Remove effective_load()<br> | https://lkml.org/lkml/2017/6/23/496 | https://lore.kernel.org/patchwork/cover/803114/ |


紧接着是 Peter Zijlstra 的一堆 FIX, 为了解决支持了 NUMA 之后一系列性能问题.

| commit | lkml | 描述 |
|:------:|:----:|:---------:|
| 90001d67be2f sched/fair: Fix wake_affine() for !NUMA_BALANCING | https://lkml.org/lkml/2017/8/1/377 | XXXX |
| a731ebe6f17b sched/fair: Fix wake_affine_llc() balancing rules | https://lkml.org/lkml/2017/9/6/196 | XXXX |
| d153b153446f sched/core: Fix wake_affine() performance regression<br>f2cdd9cc6c97 sched/core: Address more wake_affine() regressions | https://lkml.org/lkml/2017/10/14/172 | 该补丁引入了 WA_IDLE, WA_BIAS+WA_WEIGHT |

目前最新 5.2 的内核中, 
Dietmar Eggemann 删除了 LB_BIAS 特性, 因此 wake-affine 的代码做了部分精简.(仅仅是代码重构, 没有逻辑变更)

| commit | lkml | 描述 |
|:------:|:----:|:---------:|
| fdf5f315d5cf sched/fair: Disable LB_BIAS by default | https://lkml.org/lkml/2018/8/9/493 | 默认 LB_BIAS 为 false |
| 1c1b8a7b03ef sched/fair: Replace source_load() & target_load() with weighted_cpuload() | 没有 LB_BIAS 之后, source_load/target_load 不再需要, 直接使用 weighted_cpuload 代替 |
| a3df067974c5 sched/fair: Rename weighted_cpuload() to cpu_runnable_load() | weighted_cpuload 函数更名为 cpu_runnable_load, [patchwork](https://lore.kernel.org/patchwork/cover/1079333/)  |

# 3    wake_affine 对 select_task_rq_fair 的影响.
-------

在唤醒CFS 进程的时候通过 select_task_rq_fair 来为进程选择一个最适合的 CPU.
```cpp
try_to_wake_up
cpu = select_task_rq(p, p->wake_cpu, SD_BALANCE_WAKE, wake_flags);
```
那么在 wake_affine 机制的参与下, 选核流程是一个什么样的逻辑呢?
[代码参见](https://elixir.bootlin.com/linux/v5.1.15/source/kernel/sched/fair.c#L6661), 选用早期  v5.1.15 版本的内核.

*    首先 sd_flag 必须配置 SD_BALANCE_WAKE 才会去做 wake_affine, 如果是 energy aware, EAS 会先通过 find_energy_efficient_cpu 选核, 不过这个是 EAS 的范畴, 不是我们今天的重点.
*    先 record_wakee 更新 wake affine 统计信息, 接着通过 wake_cap 和 wake_wide 看这次对进程的唤醒是不是 want_affine 的.
*    接着从 waker CPU 开始向上遍历调度域, 
    1.    如果是 want_affine, 则先通过 wake_affine 在当前调度域 tmp 中是从 prev_cpu 和 waker CPU 以及上次的 waker CPU( recent_used_cpu) 中优选一个合适的 new CPU, 待会选核的时候, 就会从走快速路径 select_idle_sibling 中从 prev_cpu 和 new cpu 中优选一个 CPU. 同时设置 recent_used_cpu 为当前 waker CPU
    2.    否则, 如果是 want_affine, 但是 tmp 中没找到满足要求的 CPU,  则最终循环结束条件为 !(tmp->flag & SD_LOAD_BALANCE), 同样如果不是 want_affine 的, 则最终循环结束条件为 !(tmp->flag & SD_LOAD_BALANCE)  或者 !tmp->flags & sd_flag,则 sd 指向了配置了 SD_LOAD_BALANCE 和 sd_flag 的最高那个层次的 sd, 这个时候会通过 find_idlest_cpu 慢速路径选择, 从这个 sd 中选择一个 idle 或者负载最小的 CPU. 

只要 wakeup 的时候, 会通过 wake_affine, 然后通过 select_idle_sibling 来选核.
其他情况下, 都是找到满足 sd_flags 的最高层次 sd, 然后通过 find_idlest_cpu 在这个调度域 sd 中去选择一个最空闲的 CPU.

# 4    参考资料
-------

[`Reduce scheduler migrations due to wake_affine`](https://lwn.net/Articles/741726/)

[[scheduler]十. 传统的负载均衡是如何为task选择合适的cpu？](http://www.pianshen.com/article/267721013//)

[wukongmingjing 的调度器专栏](https://blog.csdn.net/wuming_422103632/column/info/26430)

[Linux Kernel- task_h_load](https://stackoverflow.com/questions/34442691/linux-kernel-task-h-load)
