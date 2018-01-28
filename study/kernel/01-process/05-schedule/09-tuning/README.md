=======

| CSDN | GitHub |
|:----:|:------:|
| [Aderstep--紫夜阑珊-青伶巷草](http://blog.csdn.net/gatieme) | [`AderXCoding/system/tools`](https://github.com/gatieme/AderXCoding/tree/master/system/tools) |

<br>

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 也欢迎大家提供一些其他好的调试工具以供收录, 鄙人在此谢谢啦

<br>


#1    调度器调优参数
-------



##1.1   CFS相关
-------

| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_min_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L75) | `/proc/sys/kernel/sched_min_granularity_ns` | `4000000ns` | 表示进程最少运行时间, 防止频繁的切换, 对于交互系统(如桌面), 该值可以设置得较小, 这样可以保证交互得到更快的响应(见周期调度器的 `check_preempt_tick` 过程) |
| [`sysctl_sched_latency`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L54) | `/proc/sys/kernel/sched_latency_ns` | `20000000ns` | 表示一个运行队列所有进程运行一次的周期, 当前这个与运行队列的进程数有关, 如果进程数超过 `sched_nr_latency` (这个变量不能通过 `/proc` 设置, 它是由 `(sysctl_sched_latency+ sysctl_sched_min_granularity-1)/sysctl_sched_min_granularity` 确定的), 那么调度周期就是 `sched_min_granularity_ns*运行队列里的进程数`, 与 `sysctl_sched_latency` 无关; 否则队列进程数小于sched_nr_latency，运行周期就是sysctl_sched_latency。显然这个数越小，一个运行队列支持的sched_nr_latency越少，而且当sysctl_sched_min_granularity越小时能支持的sched_nr_latency越多，那么每个进程在这个周期内能执行的时间也就越少，这也与上面sysctl_sched_min_granularity变量的讨论一致。其实sched_nr_latency也可以当做我们cpu load的基准值，如果cpu的load大于这个值，那么说明cpu不够使用了 |
| [`sysctl_sched_wakeup_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L98) | `/proc/sys/kernel/sched_wakeup_granularity_ns` | `4000000ns` | 该变量表示进程被唤醒后至少应该运行的时间的基数, 它只是用来判断某个进程是否应该抢占当前进程，并不代表它能够执行的最小时间(sysctl_sched_min_granularity), 如果这个数值越小, 那么发生抢占的概率也就越高(见wakeup_gran、wakeup_preempt_entity函数) |
| [`sysctl_sched_child_runs_first`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L87) | `/proc/sys/kernel/sched_child_runs_first` | 0 | 该变量表示在创建子进程的时候是否让子进程抢占父进程，即使父进程的vruntime小于子进程，这个会减少公平性，但是可以降低write_on_copy，具体要根据系统的应用情况来考量使用哪种方式（见task_fork_fair过程） |
| [`sysctl_sched_cfs_bandwidth_slice`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L124) | `/proc/sys/kernel/sched_cfs_bandwidth_slice_us` | 5000us | |
| [`sysctl_sched_migration_cost`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L101) | `/proc/sys/kernel/sched_migration_cost` | `500000ns` | 该变量用来判断一个进程是否还是hot，如果进程的运行时间（now - p->se.exec_start）小于它，那么内核认为它的code还在cache里，所以该进程还是hot，那么在迁移的时候就不会考虑它 |
| [`sysctl_sched_tunable_scaling`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L68) | `/proc/sys/kernel/sched_tunable_scaling` | 1 | 当内核试图调整sched_min_granularity，sched_latency和sched_wakeup_granularity这三个值的时候所使用的更新方法，0为不调整，1为按照cpu个数以2为底的对数值进行调整，2为按照cpu的个数进行线性比例的调整 |


##1.2 RT相关
-------

| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_rt_period`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L76) | `/proc/sys/kernel/sched_rt_period_us` | `1000000us` | 该参数与下面的sysctl_sched_rt_runtime一起决定了实时进程在以sysctl_sched_rt_period为周期的时间内，实时进程最多能够运行的总的时间不能超过sysctl_sched_rt_runtime（代码见sched_rt_global_constraints |
| [`sysctl_sched_rt_runtime`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L84) | `/proc/sys/kernel/sched_rt_runtime_us` | `950000us` | 见上 `sysctl_sched_rt_period` 变量的解释 |
| [`sysctl_sched_compat_yield`]() | `/proc/sys/kernel/sched_compat_yield` | 0 | 该参数可以让sched_yield()系统调用更加有效，让它使用更少的cpu，对于那些依赖sched_yield来获得更好性能的应用可以考虑设置它为1 | 


##1.3  全局参数
-------

| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_features`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L52) | `/proc/sys/kernel/sched_features` | `3183d=110001101111b` | 该变量表示调度器支持的特性，如GENTLE_FAIR_SLEEPERS(平滑的补偿睡眠进程),START_DEBIT(新进程尽量的早调度),WAKEUP_PREEMPT(是否wakeup的进程可以去抢占当前运行的进程)等，所有的features见内核sech_features.h文件的定义 | 
| [`sysctl_sched_nr_migrate`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L62) | `/proc/sys/kernel/sched_nr_migrate` | 32 | 在多CPU情况下进行负载均衡时，一次最多移动多少个进程到另一个CPU上 |




#2   进程最少运行时间 `sysctl_sched_min_granularity`
-------

| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_min_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L75) | `/proc/sys/kernel/sched_min_granularity_ns` | `4000000ns` | 表示进程最少运行时间, 防止频繁的切换, 对于交互系统(如桌面), 该值可以设置得较小, 这样可以保证交互得到更快的响应(见周期调度器的 `check_preempt_tick` 过程) |


##2.1   参数背景
-------

[`sysctl_sched_min_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L75) 定义在 [](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L70). 如下所示:


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L70
/*
 * Minimal preemption granularity for CPU-bound tasks:
 *
 * (default: 0.75 msec * (1 + ilog(ncpus)), units: nanoseconds)
 */
unsigned int sysctl_sched_min_granularity       = 750000ULL;
unsigned int normalized_sysctl_sched_min_granularity    = 750000ULL;
```



##2.2   `check_preempt_wakeup` 检查周期性抢占
-------


在每个时钟中断中都会触发周期性调度, 调用周期性调度器主函数 `scheduler_tick`, 但是并不是每次触发都会导致调度, 调度也是有开销的, 如果频繁的调度反而降低系统的性能. 因此内核设计了 [`sysctl_sched_min_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L75) 参数, 表示进程的最少运行时间, 只有进程时间超过这个时间才会设置调度标识, 这样内核将在下一次调度时机, 调度 scheduler 完成调度, 让当前进程让出 `CPU` 了,

```cpp
scheduler_tick
    -=> task_tick_fair
        -=> entity_tick
            -=> check_preempt_tick
```

调度器完成调度和选择进程的时候, 需要检查当前进程是否需要调度, 通过 `check_preempt_tick` 来完成

>对应的唤醒抢占的时候, 检查是否可以抢占当前进程的操作, 由函数 `check_preempt_wakeup` 来完成.

```cpp
/*
 * Preempt the current task with a newly woken task if needed:
 */
static void
check_preempt_tick(struct cfs_rq *cfs_rq, struct sched_entity *curr)
{
    unsigned long ideal_runtime, delta_exec;
    struct sched_entity *se;
    s64 delta;

    //  首先 sched_slice 计算进程的理想运行时间(实际时间ns)
    ideal_runtime = sched_slice(cfs_rq, curr);
    //  计算当前进程的已实际运行时间 delta_exec
    delta_exec = curr->sum_exec_runtime - curr->prev_sum_exec_runtime;
    //  如果实际运行时间 delta_exec 比理想运行时间要大
    //  说明进程已经运行了很久了, 那么应该让出 CPU
    if (delta_exec > ideal_runtime) {
        //  设置当前进程的TIF的抢占标识
        resched_curr(rq_of(cfs_rq));
        /*
         * The current task ran long enough, ensure it doesn't get
         * re-elected due to buddy favours.
         */
        clear_buddies(cfs_rq, curr);
        return;
    }

    /*
     * Ensure that a task that missed wakeup preemption by a
     * narrow margin doesn't have to wait for a full slice.
     * This also mitigates buddy induced latencies under load.
     */
    //  如果实际运行时间小于 进程最少运行时间就直接返回, 不会设置调度标记
    if (delta_exec < sysctl_sched_min_granularity)
        return;

    //  如果进程实际运行时间超过了最少运行时间, 则取出红黑树上最左(虚拟运行时间最小的)节点
    //  进一步比较当前进程和运行队列上虚拟运行时间最小的进程
    se = __pick_first_entity(cfs_rq);
    delta = curr->vruntime - se->vruntime;

    if (delta < 0)
        return;

    //  只有当前进程与最左节点运行时间的差值超过当前进程的理想运行时间, 才会设置调度标记
    if (delta > ideal_runtime)
        resched_curr(rq_of(cfs_rq));
}
```

从`check_preempt_tick` 可以看出, 周期性调度的时候, 如果进程当前运行周期的运行时间足够长, 就需要设置调度标记, 调度器会可以在合适的调度时机完成抢占调度, 评价当前进程运行时间足够长的标准就是满足如下条件任意一条

*   当前进程已经用尽了 CFS 给予的时间片, 即实际运行时间 `delta_exec` 超过 CFS 分配的理想运行时间 `ideal_runtime`.

*   当前进程没有用尽 CFS 给予的时间片, 但是其当前周期的运行时间超过了程序的最小运行时间阈值 `sysctl_sched_min_granularity`, 同时其运行时间超过运行队列上最饥饿进程(红黑数的最左结点)一个运行周期, 即虚拟运行时间比后者还大 `ideal_runtime`.



##2.3   `sysctl_sched_min_granularity` 接口实现
-------

`sysctl_sched_min_granularity` 接口位于 `/proc/sys/kernel/sched_min_granularity_ns`, 内核中的定义在 [`kernel/sysctl.c, line 316`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sysctl.c#L316). 可以看到其默认权限为 `0644`(`root` 用户可读写, 其他用户读), 值范围从 `min_sched_granularity_ns(默认0.1ms) ~ max_sched_granularity_ns(默认1s)`.


```cpp
http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sysctl.c#L290
static int min_sched_granularity_ns = 100000;       /* 100 usecs */
static int max_sched_granularity_ns = NSEC_PER_SEC; /* 1 second */

//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sysctl.c#L316
#ifdef CONFIG_SCHED_DEBUG
    {
        .procname   = "sched_min_granularity_ns",
        .data       = &sysctl_sched_min_granularity,
        .maxlen     = sizeof(unsigned int),
        .mode       = 0644,
        .proc_handler   = sched_proc_update_handler,
        .extra1     = &min_sched_granularity_ns,
        .extra2     = &max_sched_granularity_ns,
    },
    //......
#endif
```


待插入的测试的内容和图片


#3  调度周期 `sysctl_sched_latency`
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_latency`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L54) | `/proc/sys/kernel/sched_latency_ns` | `20000000ns` | 表示一个运行队列所有进程运行一次的周期 |


##3.1   参数背景
-------


[`sysctl_sched_latency`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L54) 定义在[`kernel/sched/fair.c, version 4.14.14, line 41`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L54)


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L41
/*
 * Targeted preemption latency for CPU-bound tasks:
 *
 * NOTE: this latency value is not the same as the concept of
 * 'timeslice length' - timeslices in CFS are of variable length
 * and have no persistent notion like in traditional, time-slice
 * based scheduling concepts.
 *
 * (to see the precise effective timeslice length of your workload,
 *  run vmstat and monitor the context-switches (cs) field)
 *
 * (default: 6ms * (1 + ilog(ncpus)), units: nanoseconds)
 */
unsigned int sysctl_sched_latency           = 6000000ULL;
unsigned int normalized_sysctl_sched_latency        = 6000000ULL;


//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L78
/*
 * This value is kept at sysctl_sched_latency/sysctl_sched_min_granularity
 */
static unsigned int sched_nr_latency = 8;
```

##3.2   参数详解
-------

##3.2.1   调度周期
-------

调度器实际的调度周期由 `__sched_period` 函数完成. `CFS` 保证在一个调度周期内每个进程至少能够运行一次.


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L654
/*
 * The idea is to set a period in which each task runs once.
 *
 * When there are too many tasks (sched_nr_latency) we have to stretch
 * this period because otherwise the slices get too small.
 *
 * p = (nr <= nl) ? l : l*nr/nl
 */
static u64 __sched_period(unsigned long nr_running)
{
    if (unlikely(nr_running > sched_nr_latency))
        return nr_running * sysctl_sched_min_granularity;
    else
        return sysctl_sched_latency;
}
```



调度周期与运行队列的进程数 `nr_running` 有关


*   如果进程数超过 `sched_nr_latency`那么调度周期就是进程的最小运行时间 `sched_min_granularity_ns` * 运行队列里的进程数 `nr_running`,与 `sysctl_sched_latency` 无关; 此时调度周期就是假定每个进程刚好运行最小运行时间的总和. 


*   否则队列进程数小于 `sched_nr_latency`，运行周期就是 `sysctl_sched_latency`.




###3.2.2    进程数目阈值 `sched_nr_latency`
-------

那进程数目阈值 `sched_nr_latency` 是怎么来的呢?

这个变量不能通过 `proc` 文件系统设置, 它是由 



$$ \frac{sysctl_sched_latency + sysctl_sched_min_granularity - 1}{sysctl_sched_min_granularity} $$

即初始调度周期与进程最小运行时间的比值.


`sched_nr_latency` 初值为 8, 它正好就是

$$ \frac{sysctl_sched_latency + sysctl_sched_min_granularity - 1}{sysctl_sched_min_granularity} = \frac{6000000 + 750000 - 1}{750000}/ = 8 $$

显然 `sysctl_sched_latency`越小，初始时一个运行队列支持的 `sched_nr_latency` 越少，而且当 `sysctl_sched_min_granularity` 越小时能支持的 `sched_nr_latency`     `sysctl_sched_min_granularity` 变量的讨论一致.

>其实 `sched_nr_latency` 也可以当做我们 `cpu load` 的基准值，如果 `cpu` 的 `load` 大于这个值，那么说明 `cpu` 不够使用了


###3.2.3    调度周期 `sysctl_sched_latency` 的影响
-------

很明显, 只有当运行队列中的进程数目小于进程数目阈值 `sched_nr_latency` 的时候, 调度周期才刚好是 `sysctl_sched_latency`.

那么这个参数直接影响的就是当进程数目较少时, 初始的调度周期.


##3.3   `sysctl_sched_latency` 接口
-------

待插入的测试的内容和图片



#4  `sysctl_sched_wakeup_granularity` 唤醒后抢占
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_wakeup_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L98) | `/proc/sys/kernel/sched_wakeup_granularity_ns` | `4000000ns` | 该变量表示进程被唤醒后至少应该运行的时间的基数, 它只是用来判断某个进程是否应该抢占当前进程，并不代表它能够执行的最小时间( `sysctl_sched_min_granularity`), 如果这个数值越小, 那么发生抢占的概率也就越高(见 `wakeup_gran`、`wakeup_preempt_entity`、 `check_preempt_wakeup` 函数) |






##4.1   参数背景
-------


[`sysctl_sched_min_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6087) 定义在[`kernel/sched/fair.c, version v4.14.14, line 98`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L98)


唤醒后抢占时间 `sysctl_sched_wakeup_granularity` 与 进程最小运行时间 `sysctl_sched_min_granularity` 的作用类似. 该变量表示进程被唤醒后至少应该运行的时间的基数, 它只是用来判断某个进程是否应该抢占当前进程，并不代表它能够执行的最小时间( `sysctl_sched_min_granularity`), 如果这个数值越小, 那么发生抢占的概率也就越高.



##4.2   唤醒抢占
-------

当进程被唤醒的时候会调用 `check_preempt_wakeup` 检查被唤醒的进程是否可以抢占当前进程. 调用路径如下:


```cpp
try_to_wake_up
    -=> ttwu_queue
        -=> ttwu_do_activate
            -=> ttwu_do_wakeup
                -=> check_preempt_curr
                    -=> check_preempt_wakeup
                        -=> wakeup_preempt_entity
                            -=> wakeup_gran
```


[`check_preempt_wakeup`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6164) 函数被定义在 [`kernel/sched/fair.c, verion 4.14.14, line 6164`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6164), 如下所示:


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6164
/*
 * Preempt the current task with a newly woken task if needed:
 */
static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_flags)
{
    struct sched_entity *se = &curr->se, *pse = &p->se;
    // ......
    //  检查当前被唤醒的进程进程p是否可以抢占当前进程curr
    if (wakeup_preempt_entity(se, pse) == 1) {
        /*
         * Bias pick_next to pick the sched entity that is
         * triggering this preemption.
         */
        if (!next_buddy_marked)
            set_next_buddy(pse);
        goto preempt;
    }

    return;

preempt:
    //  设置调度标记
    resched_curr(rq);
    // ......
}
```


[`check_preempt_wakeup`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6164) 函数中通过 `wakeup_preempt_entity` 检查当前被唤醒的进程实体 `pse` 能否抢占当前进程的调度实体 `curr->se`. 如果发现可以抢占, 就通过 `resched_curr` 设置调度标记.


`wakeup_preempt_entity` 函数定义在 [`kernel/sched/fair.c, version v4.14.14, line 6127`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6127). 如下所示:


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6127
/*
 * Should 'se' preempt 'curr'.
 *
 *             |s1
 *        |s2
 *   |s3
 *         g
 *      |<--->|c
 *
 *  w(c, s1) = -1
 *  w(c, s2) =  0
 *  w(c, s3) =  1
 *
 */
static int
wakeup_preempt_entity(struct sched_entity *curr, struct sched_entity *se)
{
    s64 gran, vdiff = curr->vruntime - se->vruntime;

    //  se要想抢占curr, 最基本的条件是虚拟运行时间比curr的小
    if (vdiff <= 0)
        return -1;

    //  计算当前进程 se 以 sysctl_sched_min_granularity 为基数的虚拟运行时间 gran
    gran = wakeup_gran(curr, se);
    // 只有两者虚拟运行时间的差值大于 gran
    if (vdiff > gran)
        return 1;

    return 0;
}

//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6087
//  计算 `sysctl_sched_wakeup_granularity` 基数的虚拟运行时间值
static unsigned long
wakeup_gran(struct sched_entity *curr, struct sched_entity *se)
{
    unsigned long gran = sysctl_sched_wakeup_granularity;

    /*
     * Since its curr running now, convert the gran from real-time
     * to virtual-time in his units.
     *
     * By using 'se' instead of 'curr' we penalize light tasks, so
     * they get preempted easier. That is, if 'se' < 'curr' then
     * the resulting gran will be larger, therefore penalizing the
     * lighter, if otoh 'se' > 'curr' then the resulting gran will
     * be smaller, again penalizing the lighter task.
     *
     * This is especially important for buddies when the leftmost
     * task is higher priority than the buddy.
     */
    return calc_delta_fair(gran, se);
}


//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L646
//  计算进程实际运行 delta 时间的虚拟运行时间
/*
 * delta /= w
 */
static inline u64 calc_delta_fair(u64 delta, struct sched_entity *se)
{
    if (unlikely(se->load.weight != NICE_0_LOAD))
        delta = __calc_delta(delta, NICE_0_LOAD, &se->load);

    return delta;
}
```

可以看出 `wakeup_preempt_entity` 函数中:



`curr` 唤醒进程 `p`, 如果 `p` 可以抢占 `curr`, 则要求满足

 
*   当前进程 `curr` 的虚拟运行时间不仅要比进程　`p` 的大, 还要大过 `calc_delta_fair(sysctl_sched_wakeup_granularity, p)`


```cpp
   curr->vruntime - p->vruntime > 0
```

否则 `wakeup_preempt_entity` 函数返回 `-1`.


```cpp
curr->vruntime - p->vruntime > calc_delta_fair(sysctl_sched_wakeup_granularity, p)
```

否则  `wakeup_preempt_entity` 函数返回 `0`.


我们假设进程 `p` 刚好实际运行了唤醒后抢占时间基数 `sysctl_sched_wakeup_granularity`, 则其虚拟运行时间将增长 `gran`. 被唤醒的进程 `p` 要想抢占 `curr`, 则要求其虚拟运行时间比 `curr` 小 `gran`. 则保证理想情况下(没有其他进程干预和进程 `p` 睡眠等其他外因), 进程 `p` 可以至少运行 `sysctl_sched_wakeup_granularity` 时间.


我们用一个图来表示:

假设 `curr` 的虚拟运行时间位于图中的位置, 中间区间是一个 `gran` 长度


*   那么当被唤醒的进程 `p` 虚拟运行时间位于区间 `-1` 的时候, `vdiff <= 0`, 不能被唤醒; (函数 `wakeup_preempt_entity` 返回 `-1`)


*   那么当被唤醒的进程 `p` 虚拟运行时间位于区间 `0` 的时候, `0 <= vdiff <= gran`, 同样不能被唤醒; (函数 `wakeup_preempt_entity` 返回 `0`)


*   那么当被唤醒的进程 `p` 虚拟运行时间位于区间 `1` 的时候, `vdiff > gran`, 不能被唤醒; (函数 `wakeup_preempt_entity` 返回 `1`)




```cpp
|----区间 1----|----区间 0----|----区间 -1----|
|             -             curr   +
--------------|--------------|--------------
              |<----gran---->|
              |              |                 
 vdiff > gran |  vdiff > 0   |   vdiff < 0  
      1       |       0      |      -1      
              |              |              
--------------|--------------|--------------
```


##4.3   `sysctl_sched_wakeup_granularity` 接口
-------

待插入的测试的内容和图片



#5  `sysctl_sched_child_runs_first` 
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_child_runs_first`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L87) | `/proc/sys/kernel/sched_child_runs_first` | 0 | 该变量表示在创建子进程的时候是否让子进程抢占父进程，即使父进程的vruntime小于子进程，这个会减少公平性，但是可以降低write_on_copy，具体要根据系统的应用情况来考量使用哪种方式（见task_fork_fair过程） |


##5.1   参数背景
-------


一般来说, 通过父进程通过 `fork` 创建子进程的时候, 内核并不保证谁先运行, 但是有时候我们更希望约定父子进之间运行的顺序. 因此 `sysctl_sched_child_runs_first` 应运而生.


该变量表示在创建子进程的时候是否让子进程抢占父进程，即使父进程的 `vruntime` 小于子进程，这个会减少公平性，但是可以降低 `write_on_copy`，具体要根据系统的应用情况来考量使用哪种方式（见 `task_fork_fair` 过程）


##5.2   `sysctl_sched_child_runs_first` 保证子进程先运行
-------


[`sysctl_sched_child_runs_first`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L83) 定义在[`kernel/sched/fair.c, version v4.14.14, line 83`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L83)


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L83
/*
 * After fork, child runs first. If set to 0 (default) then
 * parent will (try to) run first.
 */
unsigned int sysctl_sched_child_runs_first __read_mostly;
```

唤醒后抢占时间 `sysctl_sched_wakeup_granularity` 与 进程最小运行时间 `sysctl_sched_min_granularity` 的作用类似. 该变量表示进程被唤醒后至少应该运行的时间的基数, 它只是用来判断某个进程是否应该抢占当前进程，并不代表它能够执行的最小时间( `sysctl_sched_min_granularity`), 如果这个数值越小, 那么发生抢占的概率也就越高.


当内核通过 `fork/clone` 创建子进程的时候, 在 `sched_fork` 中完成了调度信息的初始化, 此时会调用对应调度器类 `task_fork` 的初始化.


```cpp
_do_fork
    -=> copy_process
        -=> sched_fork
            -=> p->sched_class->task_fork(p);
```


`CFS` 调度器的 `task_fork` 函数就是 `task_fork_fair`.

```cpp
/*
 * called on fork with the child task as argument from the parent's context
 *  - child not yet on the tasklist
 *  - preemption disabled
 */
static void task_fork_fair(struct task_struct *p)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se, *curr;
    struct rq *rq = this_rq();
    struct rq_flags rf;

    rq_lock(rq, &rf);
    update_rq_clock(rq);

    cfs_rq = task_cfs_rq(current);
    curr = cfs_rq->curr;
    if (curr) {
        update_curr(cfs_rq);
        se->vruntime = curr->vruntime;
    }
    place_entity(cfs_rq, se, 1);

    if (sysctl_sched_child_runs_first && curr && entity_before(curr, se)) {
        /*
         * Upon rescheduling, sched_class::put_prev_task() will place
         * 'current' within the tree based on its new key value.
         */
        swap(curr->vruntime, se->vruntime);
        resched_curr(rq);
    }

    se->vruntime -= cfs_rq->min_vruntime;
    rq_unlock(rq, &rf);
}
```

#2  调度器特性 features
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_features`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L52) | `/proc/sys/kernel/sched_features` | `3183d=110001101111b` | 该变量表示调度器支持的特性，如GENTLE_FAIR_SLEEPERS(平滑的补偿睡眠进程),START_DEBIT(新进程尽量的早调度),WAKEUP_PREEMPT(是否wakeup的进程可以去抢占当前运行的进程)等，所有的features见内核sech_features.h文件的定义 | 


##2.1  `sched_features` 接口的设计
-------


> 我们其实更加关注的是这些特性的设计, 以及它们对内核调度器的影响.
>
> 但是这些接口本身的设计实在是太精巧, 让我不得不想要说它们.


*   `sched_features` 的初始化


内核的调度器支持很多特性, 这些特性可以在内核运行时动态的开启, 因此内核提供了 `sched_features` 接口, 该接口位于内核 `/proc/sys/kernel/sched_features`, 对应的内核控制数据为 `sysctl_sched_features`.

```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L45
/*
 * Debugging: various feature bits
 */

#define SCHED_FEAT(name, enabled)   \
    (1UL << __SCHED_FEAT_##name) * enabled |

const_debug unsigned int sysctl_sched_features =
#include "features.h"
    0;

#undef SCHED_FEAT
```
首先定义了一个 `SCHED_FEAT` 宏, 然后定义并初始化 `sysctl_sched_features` 的时候通过包含 `features.h` 完成. 该头文件的定义在 [`kernel/sched/features.h, v4.14.14`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/features.h).

```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/features.h
SCHED_FEAT(GENTLE_FAIR_SLEEPERS, true)
......
SCHED_FEAT(WA_BIAS, true)
```

通过这种方式将 `sysctl_sched_features` 的每一位 `(1UL << __SCHED_FEAT_##name)` 初始化为 `true`(特性开启) 或者 `false`(关闭).


这个 `features.h` 头文件就是实现 `sysctl_sched_features` 的精妙所在, 这个文件中 `SCHED_FEAT` 宏将所有的特性宏展开, 通过修改 `SCHED_FEAT` 宏的定义可以将其展开成不同的表达式或者代码段.

内核中用 `SCHED_FEAT` 宏完成了

| `SCHED_FEAT` 宏功能 | 定义方式 | 定义位置 |
|:------------------:|:-------:|:------:|
| \__SCHED_FEAT_XXX 特性宏的枚举定义 | `__SCHED_FEAT_##name ,` | |
| `sysctl_sched_features` 的定义和初始化 | `(1UL << __SCHED_FEAT_##name) * enabled` | 
| `sched_feat` 的函数的定义，该函数用于判断某个特性是否开启 | 

*   定义 `__SCHED_FEAT_XXX` 特性宏 和 `sched_feat` 函数


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/sched.h#L1222
/*
 * Tunables that become constants when CONFIG_SCHED_DEBUG is off:
 */
#ifdef CONFIG_SCHED_DEBUG
# include <linux/static_key.h>
# define const_debug __read_mostly
#else
# define const_debug const
#endif

extern const_debug unsigned int sysctl_sched_features;

#define SCHED_FEAT(name, enabled)   \
    __SCHED_FEAT_##name ,

enum {
#include "features.h"
    __SCHED_FEAT_NR,
};

#undef SCHED_FEAT

#if defined(CONFIG_SCHED_DEBUG) && defined(HAVE_JUMP_LABEL)
#define SCHED_FEAT(name, enabled)                   \
static __always_inline bool static_branch_##name(struct static_key *key) \
{                                   \
    return static_key_##enabled(key);               \
}

#include "features.h"

#undef SCHED_FEAT

extern struct static_key sched_feat_keys[__SCHED_FEAT_NR];
#define sched_feat(x) (static_branch_##x(&sched_feat_keys[__SCHED_FEAT_##x]))
#else /* !(SCHED_DEBUG && HAVE_JUMP_LABEL) */
#define sched_feat(x) (sysctl_sched_features & (1UL << __SCHED_FEAT_##x))
#endif /* SCHED_DEBUG && HAVE_JUMP_LABEL */
```


##2.2    调度器特性 sched_features
-------

每个特性对应 `sysctl_sched_features` 中的一位, 通过 `__SCHED_FEAT_XXX` 宏定义




| 特性 | 默认值 | 描述 |
|:---:|:-----:|:---:|
| GENTLE_FAIR_SLEEPERS | true | 对于睡眠的进程, 投入运行时虚拟运行时间只补偿一半, 即保证能很快运行, 又防止睡眠后投入的进程长时间占用 CPU |
| START_DEBIT | true | 
| NEXT_BUDDY | false |


##2.3   GENTLE_FAIR_SLEEPERS
-------


通常我们将进程划分为 `CPU` 密集型和 `I/O` 密集型, 后者的特点是基本不占用 `CPU`，主要行为是在 `S` 状态等待响应. 典型的比如交互式进程( `vim`，`bash` 等)，以及一些压力不大的，使用了多进程(线程)的或 `select`、`poll`、`epoll` 的网络代理程序等. 

如果 `CFS` 采用默认的策略处理这些程序的话，相比 `CPU` 消耗程序来说，这些应用由于绝大多数时间都处在 `sleep` 状态，它们的 `vruntime` 时间基本是不变的，而 `CPU` 密集型的进程 `vruntime` 却持续增长, 一旦这些 `I/O` 密集型进入了调度队列，将会很快被选择调度执行. 对比 `O1` 调度算法，这种行为相当于自然的提高了这些 `I/O` 密集型进程的优先级，于是就不需要特殊对它们的优先级进行 "动态调整" 了.

但这样的默认策略也是有问题的

*   有时 `CPU` 密集型和 `I/O` 密集型进程的区分不是那么明显，有些进程可能会等一会，然后调度之后也会长时间占用 `CPU`(`CPU` 密集)这种情况下，如果休眠的时候进程的 `vruntime` 保持不变，那么等到休眠被唤醒之后，这个进程的 `vruntime` 时间就可能会比别人小很多，从而长时间占用 `CPU`， 导致调度器不公平.

*   另外, 我们可以构造这样的进程, 创建后先睡眠一段时间, 然后开始持续执行, 在默认策略下, 也将长时间占有 `CPU`.

所以对于睡眠后醒来并投入运行的进程，CFS 应当对其进行时间补偿. 但是可以从补偿方式上做些调整. 保证这些进程既可以很快投入运行, 又不会超额占有 `CPU`. 因此 `GENTLE_FAIR_SLEEPERS` 特性加入到调度器中.

如果进程是从 `sleep` 状态被唤醒的，而且 `GENTLE_FAIR_SLEEPERS` 属性的值为 `true`，则 `vruntime` 被设置为 `sched_latency_ns` 的一半和当前进程的 `vruntime` 值中比较大的那个.


因为系统中这种调度补偿的存在，IO消耗型的进程总是可以更快的获得响应速度。这是CFS处理与人交互的进程时的策略，即：通过提高响应速度让人的操作感受更好。但是有时候也会因为这样的策略导致整体性能受损。在很多使用了多进程（线程）或select、poll、epoll的网络代理程序，一般是由多个进程组成的进程组进行工作，典型的如apche、nginx和php-fpm这样的处理程序。它们往往都是由一个或者多个进程使用nanosleep()进行周期性的检查是否有新任务，如果有责唤醒一个子进程进行处理，子进程的处理可能会消耗CPU，而父进程则主要是sleep等待唤醒。这个时候，由于系统对sleep进程的补偿策略的存在，新唤醒的进程就可能会打断正在处理的子进程的过程，抢占CPU进行处理。当这种打断很多很频繁的时候，CPU处理的过程就会因为频繁的进程上下文切换而变的很低效，从而使系统整体吞吐量下降。此时我们可以使用开关禁止唤醒抢占的特性。



<br>

*      本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*      采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*      基于本文修改后的作品务必以相同的许可发布. 如有任何疑问，请与我联系.

