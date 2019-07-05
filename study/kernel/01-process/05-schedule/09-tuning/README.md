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
| [`sysctl_sched_latency`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L54) | `/proc/sys/kernel/sched_latency_ns` | `20000000ns` | 表示一个运行队列所有进程运行一次的周期, 当前这个与运行队列的进程数有关, 如果进程数超过 `sched_nr_latency` (这个变量不能通过 `/proc` 设置, 它是由 `(sysctl_sched_latency+ sysctl_sched_min_granularity-1)/sysctl_sched_min_granularity` 确定的), 那么调度周期就是 `sched_min_granularity_ns*运行队列里的进程数`, 与 `sysctl_sched_latency` 无关; 否则队列进程数小于sched_nr_latency, 运行周期就是sysctl_sched_latency. 显然这个数越小, 一个运行队列支持的sched_nr_latency越少, 而且当sysctl_sched_min_granularity越小时能支持的sched_nr_latency越多, 那么每个进程在这个周期内能执行的时间也就越少, 这也与上面sysctl_sched_min_granularity变量的讨论一致. 其实sched_nr_latency也可以当做我们cpu load的基准值, 如果cpu的load大于这个值, 那么说明cpu不够使用了 |
| [`sysctl_sched_wakeup_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L98) | `/proc/sys/kernel/sched_wakeup_granularity_ns` | `4000000ns` | 该变量表示进程被唤醒后至少应该运行的时间的基数, 它只是用来判断某个进程是否应该抢占当前进程, 并不代表它能够执行的最小时间(sysctl_sched_min_granularity), 如果这个数值越小, 那么发生抢占的概率也就越高(见wakeup_gran、wakeup_preempt_entity函数) |
| [`sysctl_sched_child_runs_first`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L87) | `/proc/sys/kernel/sched_child_runs_first` | 0 | 该变量表示在创建子进程的时候是否让子进程抢占父进程, 即使父进程的vruntime小于子进程, 这个会减少公平性, 但是可以降低write_on_copy, 具体要根据系统的应用情况来考量使用哪种方式（见task_fork_fair过程） |
| [`sysctl_sched_migration_cost`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L101) | `/proc/sys/kernel/sched_migration_cost` | `500000ns` | 该变量用来判断一个进程是否还是hot, 如果进程的运行时间（now - p->se.exec_start）小于它, 那么内核认为它的code还在cache里, 所以该进程还是hot, 那么在迁移的时候就不会考虑它 |
| [`sysctl_sched_tunable_scaling`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L68) | `/proc/sys/kernel/sched_tunable_scaling` | 1 | 当内核试图调整sched_min_granularity, sched_latency和sched_wakeup_granularity这三个值的时候所使用的更新方法, 0为不调整, 1为按照cpu个数以2为底的对数值进行调整, 2为按照cpu的个数进行线性比例的调整 |
| [`sysctl_sched_cfs_bandwidth_slice`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L124) | `/proc/sys/kernel/sched_cfs_bandwidth_slice_us` | 5000us | |


##1.2 RT相关
-------

| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_rt_period`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L76) | `/proc/sys/kernel/sched_rt_period_us` | `1000000us` | 该参数与下面的sysctl_sched_rt_runtime一起决定了实时进程在以sysctl_sched_rt_period为周期的时间内, 实时进程最多能够运行的总的时间不能超过sysctl_sched_rt_runtime（代码见sched_rt_global_constraints |
| [`sysctl_sched_rt_runtime`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L84) | `/proc/sys/kernel/sched_rt_runtime_us` | `950000us` | 见上 `sysctl_sched_rt_period` 变量的解释 |
| [`sysctl_sched_compat_yield`]() | `/proc/sys/kernel/sched_compat_yield` | 0 | 该参数可以让sched_yield()系统调用更加有效, 让它使用更少的cpu, 对于那些依赖sched_yield来获得更好性能的应用可以考虑设置它为1 |


##1.3  全局参数
-------

| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_features`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L52) | `/proc/sys/kernel/sched_features` | `3183d=110001101111b` | 该变量表示调度器支持的特性, 如GENTLE_FAIR_SLEEPERS(平滑的补偿睡眠进程),START_DEBIT(新进程尽量的早调度),WAKEUP_PREEMPT(是否wakeup的进程可以去抢占当前运行的进程)等, 所有的features见内核sech_features.h文件的定义 |
| [`sysctl_sched_nr_migrate`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L62) | `/proc/sys/kernel/sched_nr_migrate` | 32 | 在多CPU情况下进行负载均衡时, 一次最多移动多少个进程到另一个CPU上 |




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


*   否则队列进程数小于 `sched_nr_latency`, 运行周期就是 `sysctl_sched_latency`.




###3.2.2    进程数目阈值 `sched_nr_latency`
-------

那进程数目阈值 `sched_nr_latency` 是怎么来的呢?

这个变量不能通过 `proc` 文件系统设置, 它是由



$$ \frac{sysctl_sched_latency + sysctl_sched_min_granularity - 1}{sysctl_sched_min_granularity} $$

即初始调度周期与进程最小运行时间的比值.


`sched_nr_latency` 初值为 8, 它正好就是

$$ \frac{sysctl_sched_latency + sysctl_sched_min_granularity - 1}{sysctl_sched_min_granularity} = \frac{6000000 + 750000 - 1}{750000}/ = 8 $$

显然 `sysctl_sched_latency`越小, 初始时一个运行队列支持的 `sched_nr_latency` 越少, 而且当 `sysctl_sched_min_granularity` 越小时能支持的 `sched_nr_latency`     `sysctl_sched_min_granularity` 变量的讨论一致.

>其实 `sched_nr_latency` 也可以当做我们 `cpu load` 的基准值, 如果 `cpu` 的 `load` 大于这个值, 那么说明 `cpu` 不够使用了


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
| [`sysctl_sched_wakeup_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L98) | `/proc/sys/kernel/sched_wakeup_granularity_ns` | `4000000ns` | 该变量表示进程被唤醒后至少应该运行的时间的基数, 它只是用来判断某个进程是否应该抢占当前进程, 并不代表它能够执行的最小时间( `sysctl_sched_min_granularity`), 如果这个数值越小, 那么发生抢占的概率也就越高(见 `wakeup_gran`、`wakeup_preempt_entity`、 `check_preempt_wakeup` 函数) |






##4.1   参数背景
-------


[`sysctl_sched_min_granularity`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6087) 定义在[`kernel/sched/fair.c, version v4.14.14, line 98`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L98)


唤醒后抢占时间 `sysctl_sched_wakeup_granularity` 与 进程最小运行时间 `sysctl_sched_min_granularity` 的作用类似. 该变量表示进程被唤醒后至少应该运行的时间的基数, 它只是用来判断某个进程是否应该抢占当前进程, 并不代表它能够执行的最小时间( `sysctl_sched_min_granularity`), 如果这个数值越小, 那么发生抢占的概率也就越高.



##4.2   唤醒抢占
-------

当进程被唤醒的时候会调用 `check_preempt_wakeup` 检查被唤醒的进程是否可以抢占当前进程. 调用路径如下 :


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

同样对于新创建的进程, 内核也将对其进行唤醒操作, 调用路径如下 :

```cpp
do_fork
    -=> wake_up_new_task
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


当前进程 `curr` 的虚拟运行时间不仅要比进程　`p` 的大, 还要大过 `calc_delta_fair(sysctl_sched_wakeup_granularity, p)`


*   当前进程 `curr` 的虚拟运行时间要比进程　`p` 的大

    ```cpp
       curr->vruntime - p->vruntime > 0
    ```

    否则 `wakeup_preempt_entity` 函数返回 `-1`.


*   当前进程 `curr` 的虚拟运行时间比进程　`p` 的大 `calc_delta_fair`


    ```cpp
    curr->vruntime - p->vruntime > calc_delta_fair(sysctl_sched_wakeup_granularity, p)
    ```

    否则  `wakeup_preempt_entity` 函数返回 `0`.


我们假设进程 `p` 刚好实际运行了唤醒后抢占时间基数 `sysctl_sched_wakeup_granularity`, 则其虚拟运行时间将增长 `gran`. 被唤醒的进程 `p` 要想抢占 `curr`, 则要求其虚拟运行时间比 `curr` 小 `gran`. 则保证理想情况下(没有其他进程干预和进程 `p` 睡眠等其他外因), 进程 `p` 可以至少运行 `sysctl_sched_wakeup_granularity` 时间.


我们用一个图来表示:

假设 `curr` 的虚拟运行时间位于图中的位置, 中间区间是一个 `gran` 长度


*   那么当被唤醒的进程 `p` 虚拟运行时间位于区间 `-1` 的时候, `vdiff <= 0`, 不能被唤醒; (函数 `wakeup_preempt_entity` 返回 `-1`)


*   那么当被唤醒的进程 `p` 虚拟运行时间位于区间 `0` 的时候, `0 <= vdiff <= gran`, 同样不能被唤醒; (函数 `wakeup_preempt_entity` 返回 `0`)


*   那么当被唤醒的进程 `p` 虚拟运行时间位于区间 `1` 的时候, `vdiff > gran`, 能被唤醒; (函数 `wakeup_preempt_entity` 返回 `1`)




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




#5  `sysctl_sched_tunable_scaling`
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_tunable_scaling`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L68) | `/proc/sys/kernel/sched_tunable_scaling` | 1 | 当内核试图调整sched_min_granularity, sched_latency和sched_wakeup_granularity这三个值的时候所使用的更新方法, 0为不调整, 1为按照cpu个数以2为底的对数值进行调整, 2为按照cpu的个数进行线性比例的调整 |



##5.1   参数背景
-------

内核中有有几个调度参数都是描述的进程运行时间相关的信息.


*   调度周期 `sched_latency`


*   进程最小运行时间 `sched_min_granularity`


*   唤醒后运行时间基数 `sched_wakeup_granularity`


之前我们看到这些阈值定义的时候, 总有一个 `normalized_sysctl_XXX` 的阈值伴随着它们. 这个 `normalized_sysctl_XXX` 具体是用来做什么的呢 ?


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L841
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
```

```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L70
/*
 * Minimal preemption granularity for CPU-bound tasks:
 *
 * (default: 0.75 msec * (1 + ilog(ncpus)), units: nanoseconds)
 */
unsigned int sysctl_sched_min_granularity       = 750000ULL;
unsigned int normalized_sysctl_sched_min_granularity    = 750000ULL;

/*
 * This value is kept at sysctl_sched_latency/sysctl_sched_min_granularity
 */
static unsigned int sched_nr_latency = 8;
```


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L89
/*
 * SCHED_OTHER wake-up granularity.
 *
 * This option delays the preemption effects of decoupled workloads
 * and reduces their over-scheduling. Synchronous workloads will still
 * have immediate wakeup/sleep latencies.
 *
 * (default: 1 msec * (1 + ilog(ncpus)), units: nanoseconds)
 */
unsigned int sysctl_sched_wakeup_granularity        = 1000000UL;
unsigned int normalized_sysctl_sched_wakeup_granularity = 1000000UL;
```


通过注释我们可以看到一些端倪, 这些 `normalized_sysctl_XXX` 的变量用于计算 非 `normalized_sysctl` 的值. 默认的计算方式就是 :


```cpp
sysctl_sched_XXX = normalized_sysctl_XXX * (1 + ilog(ncpus))
```

那为什么需要这样计算呢 ?



在 `SMP` 架构下, 随着 `CPU` 数量的增加, 程序的并行化增加. 用户可见的 "有效延迟" 降低. 但是这种关系并不是线性的, 所以调度器实现了几种计算方式, 就是在基础值 (`normalized_sysctl_XXX`) 的基础上乘上一个系数 `factor`. 内核提供了系数的多种计算方法.

| 策略 | 系数 | 计算方法 |
|:---:|:----:|:------:|
| SCHED_TUNABLESCALING_NONE   | 1             | `sysctl_XXX = normalized_sysctl_XXX` |
| SCHED_TUNABLESCALING_LOG    | 1+ilog(ncpus) | `sysctl_XXX = normalized_sysctl_XXX * (1+ilog(ncpus))` |
| SCHED_TUNABLESCALING_LINEAR | ncpus         | `sysctl_XXX = normalized_sysctl_XXX * ncpus` |

内核参数 [`sysctl_sched_tunable_scaling`](https://elixir.bootlin.com/linux/v4.16-rc1/source/kernel/sched/fair.c#L58) 就是内核用来控制调度器使用那种计算方法的. 默认值`SCHED_TUNABLESCALING_LOG` 标识用 `log2` 去做 `CPU` 数量. 这个想法来自 `Con Kolivas` 的 `SD` 调度器.



##5.2   参数详解
-------


当CPU数量更多时, 内核需要增加 `sched_min_granularity`, `sched_latency` 和 `sched_wakeup_granularity` 这几个内核参数的粒度值. 内核定义了一组基础值 `normalized_sysctl_XXX` 和一套校正系数 `fctor` 的计算方法, 在基础值的基础上乘以校正因子 `factor`, 作为内核最终采用的实际值 `sysctl_XXX`.

关于校正因子的计算内核提供了多种计算方法, 由 `sched_tunable_scaling` 表示.



```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/include/linux/sched/sysctl.h#L27
enum sched_tunable_scaling {
    SCHED_TUNABLESCALING_NONE,
    SCHED_TUNABLESCALING_LOG,
    SCHED_TUNABLESCALING_LINEAR,
    SCHED_TUNABLESCALING_END,
};
```


内核参数 [`sysctl_sched_tunable_scaling`](https://elixir.bootlin.com/linux/v4.16-rc1/source/kernel/sched/fair.c#L58) 就是内核用来控制调度器使用那种计算方法的. 默认值`SCHED_TUNABLESCALING_LOG` 标识用 `log2` 去做 `CPU` 数量. 这个想法来自 `Con Kolivas` 的 `SD` 调度器.



```cpp
//  https://elixir.bootlin.com/linux/v4.16-rc1/source/kernel/sched/fair.c#L58
/*
 * The initial- and re-scaling of tunables is configurable
 *
 * Options are:
 *
 *   SCHED_TUNABLESCALING_NONE - unscaled, always *1
 *   SCHED_TUNABLESCALING_LOG - scaled logarithmical, *1+ilog(ncpus)
 *   SCHED_TUNABLESCALING_LINEAR - scaled linear, *ncpus
 *
 * (default SCHED_TUNABLESCALING_LOG = *(1+ilog(ncpus))
 */
enum sched_tunable_scaling sysctl_sched_tunable_scaling = SCHED_TUNABLESCALING_LOG;
```

函数 [`get_update_sysctl_factor`](https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L153) 用来获取当前策略 `sysctl_sched_tunable_scaling` 下的 `factor` 系数.



```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L153
/*
 * Increase the granularity value when there are more CPUs,
 * because with more CPUs the 'effective latency' as visible
 * to users decreases. But the relationship is not linear,
 * so pick a second-best guess by going with the log2 of the
 * number of CPUs.
 *
 * This idea comes from the SD scheduler of Con Kolivas:
 */
static unsigned int get_update_sysctl_factor(void)
{
    unsigned int cpus = min_t(unsigned int, num_online_cpus(), 8);
    unsigned int factor;

    switch (sysctl_sched_tunable_scaling) {
    case SCHED_TUNABLESCALING_NONE:
        factor = 1;
        break;
    case SCHED_TUNABLESCALING_LINEAR:
        factor = cpus;
        break;
    case SCHED_TUNABLESCALING_LOG:
    default:
        factor = 1 + ilog2(cpus);
        break;
    }

    return factor;
}
```

[`update_sysctl`](https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L183) 函数在系统启动和 `CPU` 上下线的时候设置 `sysctl_XXX` 的值.

即


```cpp
sysctl_sched_XXX = normalized_sysctl_XXX * factor
```


```
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L183
static void update_sysctl(void)
{
    unsigned int factor = get_update_sysctl_factor();

#define SET_SYSCTL(name) \
    (sysctl_##name = (factor) * normalized_sysctl_##name)
    SET_SYSCTL(sched_min_granularity);
    SET_SYSCTL(sched_latency);
    SET_SYSCTL(sched_wakeup_granularity);
#undef SET_SYSCTL
}


//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L195
void sched_init_granularity(void)
{
    update_sysctl();
}


//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L9024
static void rq_online_fair(struct rq *rq)
{
    update_sysctl();

    update_runtime_enabled(rq);
}



//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L9051
static void rq_offline_fair(struct rq *rq)
{
    update_sysctl();

    /* Ensure any throttled groups are reachable by pick_next_task */
    unthrottle_offline_cfs_rqs(rq);
}
```


`sched_proc_update_handler` 函数则用于在用户通过 `procfs` 接口更新了 `sysctl_XXX` 的值之后, 同步更新 `normalized_sysctl_XXX`的值, 否则下一次通过 `update_sysctl` 转换的值将会有问题.

即

```cpp
normalized_sysctl_XXX = sysctl_XXX / factor
```


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L153
/**************************************************************
 * Scheduling class statistics methods:
 */
int sched_proc_update_handler(struct ctl_table *table, int write,
        void __user *buffer, size_t *lenp,
        loff_t *ppos)
{
    int ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);
    unsigned int factor = get_update_sysctl_factor();

    if (ret || !write)
        return ret;

    sched_nr_latency = DIV_ROUND_UP(sysctl_sched_latency,
                    sysctl_sched_min_granularity);

#define WRT_SYSCTL(name) \
    (normalized_sysctl_##name = sysctl_##name / (factor))
    WRT_SYSCTL(sched_min_granularity);
    WRT_SYSCTL(sched_latency);
    WRT_SYSCTL(sched_wakeup_granularity);
#undef WRT_SYSCTL

    return 0;
}


//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sysctl.c#L313
static struct ctl_table kern_table[] = {
    //  ......
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
    {
        .procname   = "sched_latency_ns",
        .data       = &sysctl_sched_latency,
        .maxlen     = sizeof(unsigned int),
        .mode       = 0644,
        .proc_handler   = sched_proc_update_handler,
        .extra1     = &min_sched_granularity_ns,
        .extra2     = &max_sched_granularity_ns,
    },
    {
        .procname   = "sched_wakeup_granularity_ns",
        .data       = &sysctl_sched_wakeup_granularity,
        .maxlen     = sizeof(unsigned int),
        .mode       = 0644,
        .proc_handler   = sched_proc_update_handler,
        .extra1     = &min_wakeup_granularity_ns,
        .extra2     = &max_wakeup_granularity_ns,
    },
#ifdef CONFIG_SMP
    {
        .procname   = "sched_tunable_scaling",
        .data       = &sysctl_sched_tunable_scaling,
        .maxlen     = sizeof(enum sched_tunable_scaling),
        .mode       = 0644,
        .proc_handler   = sched_proc_update_handler,
        .extra1     = &min_sched_tunable_scaling,
        .extra2     = &max_sched_tunable_scaling,
    },
    // ......
#endif
```

##5.3   接口详解
-------


#6  `sysctl_sched_child_runs_first`
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_child_runs_first`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L87) | `/proc/sys/kernel/sched_child_runs_first` | 0 | 该变量表示在创建子进程的时候是否让子进程抢占父进程, 即使父进程的vruntime小于子进程, 这个会减少公平性, 但是可以降低write_on_copy, 具体要根据系统的应用情况来考量使用哪种方式（见task_fork_fair过程） |


##6.1   参数背景
-------


一般来说, 通过父进程通过 `fork` 创建子进程的时候, 内核并不保证谁先运行, 但是有时候我们更希望约定父子进之间运行的顺序. 因此 `sysctl_sched_child_runs_first` 应运而生.


该变量表示在创建子进程的时候是否让子进程抢占父进程, 即使父进程的 `vruntime` 小于子进程, 这个会减少公平性, 但是可以降低 `write_on_copy`, 具体要根据系统的应用情况来考量使用哪种方式（见 `task_fork_fair` 过程）


##6.2   `sysctl_sched_child_runs_first` 保证子进程先运行
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

唤醒后抢占时间 `sysctl_sched_wakeup_granularity` 与 进程最小运行时间 `sysctl_sched_min_granularity` 的作用类似. 该变量表示进程被唤醒后至少应该运行的时间的基数, 它只是用来判断某个进程是否应该抢占当前进程, 并不代表它能够执行的最小时间( `sysctl_sched_min_granularity`), 如果这个数值越小, 那么发生抢占的概率也就越高.


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


如果参数 `sysctl_sched_child_runs_first` 被设置, 同时 `curr` 进程的虚拟运行时间比创建的子进程 `se`, 就交换它们的虚拟运行时间. 通过这种方式来保证子进程的虚拟运行时间比父进程的小, 从而保证子进程优先运行.


##6.3 `sysctl_sched_child_runs_first` 接口
-------


#7  `sysctl_sched_migration_cost`
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_migration_cost`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L101) | `/proc/sys/kernel/sched_migration_cost` | `500000ns` | 该变量用来判断一个进程是否还是 `hot`, 如果进程的运行时间 (`now - p->se.exec_start`) 小于它, 那么内核认为它的 `code` 还在 `cache` 里, 所以该进程还是 `hot`, 那么在迁移的时候就不会考虑它 |


##7.1   参数背景
-------


`CPU` 负载平衡有两种方式 : `pull` 和 `push`, 即

*   空闲 `CPU` 从其他忙的 `CPU` 队列中拉一个进程到当前 `CPU` 队列, 通过 `idle_balance` 完成;

*   或者忙的CPU队列将一个进程推送到空闲的 `CPU` 队列中.


`idle_balance` 干的则是 `pull` 的事情, `idle_balance` 会 `for_each_domain(this_cpu, sd)` 则是遍历当前 `CPU`所在的调度域, 维持调度域内的核间平衡.

但是负载平衡有一个矛盾就是 : 负载平衡的频度和 `CPU cache` 的命中率是矛盾的, `CPU` 调度域就是将各个 `CPU` 分成层次不同的组, 低层次搞定的平衡就绝不上升到高层次处理, 避免影响 `cache` 的命中率.


*   调度器通过 `sysctl_sched_migration_cost` 这个 `proc` 阈值判断进程来判断进程是否是 `cache hot` 的, 如果是 `hot` 的则不会进行负载均衡迁移.

*   同时通过该阈值控制当前 `CPU` 是否可以 `pull`. `sysctl_sched_migration_cost` 对应 `proc` 控制文件是 `/proc/sys/kernel/sched_migration_cost`, 开关代表如果 `CPU` 队列空闲了 `500000ns = 500us`(`sysctl_sched_migration_cost` 默认值) 以上, 则进行 `pull`, 否则则返回.


##7.2   `sysctl_sched_migration_cost` 实现详解
-------


###7.2.1 判断进程是否是 `cache-hot`
-------


调度器通过 [`task_hot`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6596) 函数检查一个进程是否是 `cache-hot` 的. 如果该进程还是 `cache-hot`, 那么在迁移的时候就不会考虑它.


*   如果 `sysctl_sched_migration_cost` 被设置为 `-1`, 则 `task_hot` 永远返回 `1`, 即认为进程永远是 `cache-hot` 的.


*   如果 `sysctl_sched_migration_cost` 被设置为 `0`, 则禁用了该配置, `task_hot` 永远返回 `0`.


*   如果进程的运行时间 (`now - p->se.exec_start`) 小于 `sysctl_sched_migration_cost`, 那么内核认为它的 `code` 还在 `cache` 里.



```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L6596
/*
 * Is this task likely cache-hot:
 */
static int task_hot(struct task_struct *p, struct lb_env *env)
{
    //  ......

    if (sysctl_sched_migration_cost == -1)
        return 1;
    if (sysctl_sched_migration_cost == 0)
        return 0;

    delta = rq_clock_task(env->src_rq) - p->se.exec_start;

    return delta < (s64)sysctl_sched_migration_cost;
}
```

注意

`task_hot(cahce-hot)` 只是禁止进程迁移的充分条件, 而不是必要条件.

具体的信息请参见 [`can_migrate_task`](https://elixir.bootlin.com/linux/latest/source/kernel/sched/fair.c#L7063) 函数.


```cpp
//  (https://elixir.bootlin.com/linux/latest/source/kernel/sched/fair.c#L7063
/*
 * can_migrate_task - may task p from runqueue rq be migrated to this_cpu?
 */
static
int can_migrate_task(struct task_struct *p, struct lb_env *env)
{
    int tsk_cache_hot;

    lockdep_assert_held(&env->src_rq->lock);

    /*
     * We do not migrate tasks that are:
     * 1) throttled_lb_pair, or
     * 2) cannot be migrated to this CPU due to cpus_allowed, or
     * 3) running (obviously), or
     * 4) are cache-hot on their current CPU.
     */
    if (throttled_lb_pair(task_group(p), env->src_cpu, env->dst_cpu))
        return 0;

    if (!cpumask_test_cpu(env->dst_cpu, &p->cpus_allowed)) {
        //  ......
        return 0;
    }

    /* Record that we found atleast one task that could run on dst_cpu */
    env->flags &= ~LBF_ALL_PINNED;

    if (task_running(env->src_rq, p)) {
        //  ......
        return 0;
    }

    /*
     * Aggressive migration if:
     * 1) destination numa is preferred
     * 2) task is cache cold, or
     * 3) too many balance attempts have failed.
     */
    tsk_cache_hot = migrate_degrades_locality(p, env);
    if (tsk_cache_hot == -1)
        tsk_cache_hot = task_hot(p, env);

    if (tsk_cache_hot <= 0 ||
        env->sd->nr_balance_failed > env->sd->cache_nice_tries) {
        if (tsk_cache_hot == 1) {
            schedstat_inc(env->sd->lb_hot_gained[env->idle]);
            schedstat_inc(p->se.statistics.nr_forced_migrations);
        }
        return 1;
    }

    schedstat_inc(p->se.statistics.nr_failed_migrations_hot);
    return 0;
}
```



###7.2.2    `idle_balance` 是否需要 `pull`
-------


当一个 `CPU` 进入 `idle` 状态时, 将通过 `idle_balance` 从其他繁忙的 `CPU` 上 `pull` 一个进程下来.

如果 `CPU` 的运行队列 `rq` 空闲时间超过 `sysctl_sched_migration_cost` 以上, 才会从其他 `CPU` 运行队列上 `pull`, 否则则返回, 不会进行 `pull` 操作.


```cpp
//  http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/fair.c#L8374
/*
 * idle_balance is called by schedule() if this_cpu is about to become
 * idle. Attempts to pull tasks from other CPUs.
 */
static int idle_balance(struct rq *this_rq, struct rq_flags *rf)
{
    //  ......

    /*
     * This is OK, because current is on_cpu, which avoids it being picked
     * for load-balance and preemption/IRQs are still disabled avoiding
     * further scheduler activity on it and we're being very careful to
     * re-start the picking loop.
     */
    rq_unpin_lock(this_rq, rf);

    if (this_rq->avg_idle < sysctl_sched_migration_cost ||
        !this_rq->rd->overload) {
        rcu_read_lock();
        sd = rcu_dereference_check_sched_domain(this_rq->sd);
        if (sd)
            update_next_balance(sd, &next_balance);
        rcu_read_unlock();

        goto out;
    }

    //  ......

out:
    /* Move the next balance forward */
    if (time_after(this_rq->next_balance, next_balance))
        this_rq->next_balance = next_balance;

    /* Is there a task of a high priority class? */
    if (this_rq->nr_running != this_rq->cfs.h_nr_running)
        pulled_task = -1;

    if (pulled_task)
        this_rq->idle_stamp = 0;

    rq_repin_lock(this_rq, rf);

    return pulled_task;
}
```


##7.3   `sysctl_sched_migration_cost` 接口详解
-------



#8 `sysctl_sched_rt_period` 与 `sysctl_sched_rt_runtime`
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_rt_period`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L76) | `/proc/sys/kernel/sched_rt_period_us` | `1000000us` | 该参数与下面的 `sysctl_sched_rt_runtime` 一起决定了实时进程在以 `sysctl_sched_rt_period` 为周期的时间内, 实时进程最多能够运行的总的时间, 不能超过 `sysctl_sched_rt_runtime`(代码见 `sched_rt_global_constraints`) |
| [`sysctl_sched_rt_runtime`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L84) | `/proc/sys/kernel/sched_rt_runtime_us` | `950000us` | 见上 `sysctl_sched_rt_period` 变量的解释 |

`RT` 实时进程有两种调度策略 : `FIFO`(先进先出) 和 `RR`(时间片轮转).

相同优先级的 `FIFO` 实时进程和 `RR` 实时进程挂到同一个运行队列上, 他们的不同在于 `RR` 实时进程运行一段时间后(时间片耗尽), 重新挂到运行队列的尾部等待下一次运行. 而 `FIFO` 的实时进程一旦运行, 则会一直占据 `CPU` 直到更高优先级的 `FIFO` 或 `RR` 进程抢占, 或自己主动放弃 `CPU`. 这样子好像很不公平, 通常会把运行时间较短的进程设置为 `FIFO` 调度策略, 把运行时间较长的进程设置为 `RR` 调度策略.


`RR` 实时进程是有时间片的, `FIFO` 实时进程是没有时间片. 但也有的说 `FIFO` 进程也有时间片, 从内核代码看, `FIFO` 实时进程也是有时间片的, 是为了在很多处理中不需要使用 `if` 语句区分这两种进程, 而在时钟中断中只会减少 `RR` 进程的时间片, 不会改变 `FIFO` 进程的时间片. `FIFO` 进程的时间片永远用不完, 使其时间片失去作用.

不同优先级的实时进程间是基于优先级进行抢占的, 那实时进程也是完全抢占普通进程的吗 ?


`RT` 调度类的优先级较高, 比普通进程 `CFS` 的优先级要高, 因此如果有实时进程在运行, 那么普通进程将无法获取到 `CPU` 运行时间. 即 `RT` 进程抢占 `CFS` 进程. 但是如果 `RT` 进程完全抢占 `CFS` 进程, 将导致普通进程得不到响应.


因此内核提供了这两个 `RT` 参数用来限制 `RT` 进程的运行时间占比.



```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/core.c#L72
/*
 * period over which we measure -rt task CPU usage in us.
 * default: 1s
 */
unsigned int sysctl_sched_rt_period = 1000000;


//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/core.c#L80
/*
 * part of the period that we allow rt tasks to run in us.
 * default: 0.95s
 */
int sysctl_sched_rt_runtime = 950000;
```


`sysctl_sched_rt_period` 表示实时进程运行周期为 `1s`, `sysctl_sched_rt_runtime` 表示在运行周期内, 实时进程最多运行 `0.95` 秒, 即 `CPU` 最多 `95%` 的时间片交给 `RT` 进程运行. 内核强制实时进程为普通进程预留出一定的运行时间. 当然可以把 `sysctl_sched_rt_runtime` 和 `sysctl_sched_rt_period` 设置成相同的数值, 即实时进程可以完全抢占普通进程.


#9  sysctl_sched_rt_runtime
-------




```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/sched.h#L1264
static inline u64 global_rt_period(void)
{
    return (u64)sysctl_sched_rt_period * NSEC_PER_USEC;
}

static inline u64 global_rt_runtime(void)
{
    if (sysctl_sched_rt_runtime < 0)
        return RUNTIME_INF;

    return (u64)sysctl_sched_rt_runtime * NSEC_PER_USEC;
}
```


`RT` 调度器中使用了一个通用的结构 `rt_schedulable_data` 来表示这段限制.

通过 `to_ratio(period, runtime)` 检查 `RT` 进程的可运行时间比率.


在 `tg_rt_schedulable` 中, 计算当前 `sched_group` 下进程的运行比率 (`runtime / period`) 之和.


```cpp
//  https://elixir.bootlin.com/linux/latest/source/kernel/sched/rt.c#L2404
struct rt_schedulable_data {
    struct task_group *tg;
    u64 rt_period;
    u64 rt_runtime;
};

static int tg_rt_schedulable(struct task_group *tg, void *data)
{
    struct rt_schedulable_data *d = data;
    struct task_group *child;
    unsigned long total, sum = 0;
    u64 period, runtime;

    period = ktime_to_ns(tg->rt_bandwidth.rt_period);
    runtime = tg->rt_bandwidth.rt_runtime;

    if (tg == d->tg) {
        period = d->rt_period;
        runtime = d->rt_runtime;
    }

    /*
     * Cannot have more runtime than the period.
     */
    if (runtime > period && runtime != RUNTIME_INF)
        return -EINVAL;

    /*
     * Ensure we don't starve existing RT tasks.
     */
    if (rt_bandwidth_enabled() && !runtime && tg_has_rt_tasks(tg))
        return -EBUSY;

    total = to_ratio(period, runtime);

    /*
     * Nobody can have more than the global setting allows.
     */
    if (total > to_ratio(global_rt_period(), global_rt_runtime()))
        return -EINVAL;

    /*
     * The sum of our children's runtime should not exceed our own.
     */
    list_for_each_entry_rcu(child, &tg->children, siblings) {
        period = ktime_to_ns(child->rt_bandwidth.rt_period);
        runtime = child->rt_bandwidth.rt_runtime;

        if (child == d->tg) {
            period = d->rt_period;
            runtime = d->rt_runtime;
        }

        sum += to_ratio(period, runtime);
    }

    if (sum > total)
        return -EINVAL;

    return 0;
}

static int __rt_schedulable(struct task_group *tg, u64 period, u64 runtime)
{
    int ret;

    struct rt_schedulable_data data = {
        .tg = tg,
        .rt_period = period,
        .rt_runtime = runtime,
    };

    rcu_read_lock();
    ret = walk_tg_tree(tg_rt_schedulable, tg_nop, &data);
    rcu_read_unlock();

    return ret;
}
```


#10
-------


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/rt.c#L12
int sched_rr_timeslice = RR_TIMESLICE;
int sysctl_sched_rr_timeslice = (MSEC_PER_SEC / HZ) * RR_TIMESLICE;
```


#11  调度器特性 features
-------


| 内核参数 | 位置 | 内核默认值 | 描述 |
|:------------:|:------:|:---------------:|:------:|
| [`sysctl_sched_features`](http://elixir.free-electrons.com/linux/v4.14.14/source/kernel/sched/core.c#L52) | `/proc/sys/kernel/sched_features` | `3183d=110001101111b` | 该变量表示调度器支持的特性, 如GENTLE_FAIR_SLEEPERS(平滑的补偿睡眠进程),START_DEBIT(新进程尽量的早调度),WAKEUP_PREEMPT(是否wakeup的进程可以去抢占当前运行的进程)等, 所有的features见内核sech_features.h文件的定义 |



##11.1  `sched_features` 接口的设计
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
......
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
| `sched_feat` 的函数的定义, 该函数用于判断某个特性是否开启 |

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


##11.2    调度器特性 `sched_features`
-------


每个特性对应 `sysctl_sched_features` 中的一位, 通过 `__SCHED_FEAT_XXX` 宏定义




| 特性 | 默认值 | 描述 |
|:---:|:-----:|:---:|
| GENTLE_FAIR_SLEEPERS  | true | 对于睡眠的进程, 投入运行时虚拟运行时间只补偿一半(`vruntime -= sysctl_sched_latency >> 1`), 即保证能很快运行, 又防止睡眠后投入的进程长时间占用 `CPU` |
| START_DEBIT           | true  | 为了防止进程通过 `fork` 盗取 `CPU` 时间. 对于新创建的子进程, 则将跳过子进程当前周期的运行(`vruntime += sched_vslice(cfs_rq, se)`) |
| NEXT_BUDDY            | false |
| LAST_BUDDY            | true  |
| CACHE_HOT_BUDDY       | true  |
| WAKEUP_PREEMPTION     | true  |
| HRTICK                | false |
| DOUBLE_TICK           | false |
| LB_BIAS               | true  |
| NONTASK_CAPACITY      | true  |
| TTWU_QUEUE            | true  |
| SIS_AVG_CPU           | false |
| SIS_PROP              | true  |
| WARN_DOUBLE_CLOCK     | false |
| RT_PUSH_IPI           | true  |
| RT_RUNTIME_SHARE      | true  |
| LB_MIN                | false |
| ATTACH_AGE_LOAD       | true  |
| WA_IDLE               | true  |
| WA_WEIGHT             | true  |
| WA_BIAS               | true  |


##11.3   `place_entity` -- `GENTLE_FAIR_SLEEPERS` && `START_DEBIT`
-------


###11.3.1   `GENTLE_FAIR_SLEEPERS` 平滑的补偿睡眠进程
-------

如果休眠进程的 `vruntime` 保持不变, 而其他运行进程的 `vruntime` 一直在推进, 那么等到休眠进程终于唤醒的时候, 它的 `vruntime` 比别人小很多, 会使它获得长时间抢占 `CPU` 的优势, 其他进程就要饿死了. 这显然是另一种形式的不公平. 但是不对休眠进程的 `vruntime` 进行补偿, 又是不公平的. 因此调度器有必要对休眠进程的 `vruntime` 进行恰到好处的补偿.


`CFS` 是这样做的 : 在休眠进程被唤醒时重新设置 `vruntime` 值, 以 `min_vruntime` 值为基础, 给予一定的补偿(减去一定的阈值 `thresh`), 但不能补偿太多.


*   如果 `GENTLE_FAIR_SLEEPERS` 没有被设置, 则默认使用 `sysctl_sched_latency` 作为 `thresh`


*   如果 `GENTLE_FAIR_SLEEPERS` 被设置, 则 `thresh` 减少为默认值 `sysctl_sched_latency` 的一半.




####11.3.1.1  `GENTLE_FAIR_SLEEPERS` 睡眠唤醒补偿背景
-------


通常我们将进程划分为 `CPU` 密集型和 `I/O` 密集型, 后者的特点是基本不占用 `CPU`, 主要行为是在 `S` 状态等待响应. 典型的比如交互式进程( `vim`, `bash` 等), 以及一些压力不大的, 使用了多进程(线程)的或 `select`、`poll`、`epoll` 的网络代理程序等.


如果 `CFS` 采用默认的策略处理这些程序的话, 相比 `CPU` 消耗程序来说, 这些应用由于绝大多数时间都处在 `sleep` 状态, 它们的 `vruntime` 时间基本是不变的, 而 `CPU` 密集型的进程 `vruntime` 却持续增长, 一旦这些 `I/O` 密集型进入了调度队列, 将会很快被选择调度执行. 对比 `O1` 调度算法, 这种行为相当于自然的提高了这些 `I/O` 密集型进程的优先级, 于是就不需要特殊对它们的优先级进行 "动态调整" 了.


但这样的默认策略也是有问题的


*   有时 `CPU` 密集型和 `I/O` 密集型进程的区分不是那么明显, 有些进程可能会等一会, 然后调度之后也会长时间占用 `CPU`(`CPU` 密集)这种情况下, 如果休眠的时候进程的 `vruntime` 保持不变, 那么等到休眠被唤醒之后, 这个进程的 `vruntime` 时间就可能会比别人小很多, 从而长时间占用 `CPU`,  导致调度器不公平.


*   另外, 我们可以构造这样的进程, 创建后先睡眠一段时间, 然后开始持续执行, 在默认策略下, 也将长时间占有 `CPU`.


所以对于睡眠后醒来并投入运行的进程, `CFS` 应当对其进行时间补偿. 但是可以从补偿方式上做些调整. 保证这些进程既可以很快投入运行, 又不会超额占有 `CPU`. 因此 `GENTLE_FAIR_SLEEPERS` 特性加入到调度器中.


####11.3.1.2   睡眠唤醒补偿实现
-------


如果进程是从 `sleep` 状态被唤醒的, 则会对进程的虚拟运行时间进行补偿, 即以当前就绪队列上的最小虚拟运行时间为基础, 然后减去一个基准值 thresh.


1.  传统的补偿方式(`GENTLE_FAIR_SLEEPERS` 为 `false`)下, 基准值用 `sysctl_sched_latency`(即调度周期). 从而该进程比就绪队列的虚拟运行时间的最小值还小一个调度周期, 相当于睡眠后被唤醒的时候进程比就绪队列上其他进程落后一个调度周期. 从而保证进程的调度的优先级.


2.  但是传统的方式下, 直接将虚拟运行时间下调一个调度周期 `sysctl_sched_latency`. 这样将导致该进程与其他进程的虚拟运行时间相差太大, 长时间占有 `CPU`. 因此如果 `GENTLE_FAIR_SLEEPERS` 特性开启, 将基准值下调为原来的一半, 即 `sysctl_sched_latency >>= 1`.


```cpp
//  https://elixir.bootlin.com/linux/latest/source/kernel/sched/fair.c#L3920
static void
place_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int initial)
{
    u64 vruntime = cfs_rq->min_vruntime;

    //  ......

    /* sleeps up to a single latency don't count. */
    if (!initial) {
        unsigned long thresh = sysctl_sched_latency;

        /*
         * Halve their sleep time's effect, to allow
         * for a gentler effect of sleepers:
         */
        if (sched_feat(GENTLE_FAIR_SLEEPERS))
            thresh >>= 1;

        vruntime -= thresh;
    }

    /* ensure we never gain time by being placed backwards. */
    se->vruntime = max_vruntime(se->vruntime, vruntime);
}
```

当进程是因为被唤醒而 `enqueue_entity` 到 `CPU` 就绪队列的的(`ENQUEUE_WAKEUP` 被设置), 会调用 `place_entity` 完成睡眠补偿.


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L3715
static void
enqueue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int flags)
{
    //  ......
    if (flags & ENQUEUE_WAKEUP)
        place_entity(cfs_rq, se, 0);
    //  ......
}
```

`try_to_wake_up` 和 `try_to_wake_up_local` 的时候都会设置 `ENQUEUE_WAKEUP` 标识.


```cpp
try_to_wake_up
-=> ttwu_queue(p, cpu, wake_flags);
    -=>     ttwu_do_activate(rq, p, wake_flags, &rf);
        -=>     int en_flags = ENQUEUE_WAKEUP | ENQUEUE_NOCLOCK;
        -=>     ttwu_activate(rq, p, en_flags);
            -=> activate_task(rq, p, en_flags);
                -=> enqueue_task(rq, p, flags);
                    -=> p->sched_class->enqueue_task(rq, p, flags);
                        -=> enqueue_entity(cfs_rq, se, flags);


try_to_wake_up_local
-=> ttwu_activate(rq, p, ENQUEUE_WAKEUP | ENQUEUE_NOCLOCK);
```


因为系统中这种调度补偿的存在, `IO` 消耗型的进程总是可以更快的获得响应速度. 这是 `CFS` 处理与人交互的进程时的策略, 即 : 通过提高响应速度让人的操作感受更好. 但是有时候也会因为这样的策略导致整体性能受损. 在很多使用了多进程(线程)或 `select`, `poll`, `epoll` 的网络代理程序, 一般是由多个进程组成的进程组进行工作, 典型的如 `apche`, `nginx` 和 `php-fpm` 这样的处理程序. 它们往往都是由一个或者多个进程使用 `nanosleep()` 进行周期性的检查是否有新任务, 如果有则唤醒一个子进程进行处理, 子进程的处理可能会消耗 `CPU`, 而父进程则主要是 `sleep` 等待唤醒. 这个时候, 由于系统对 `sleep` 进程的补偿策略的存在, 新唤醒的进程就可能会打断正在处理的子进程的过程, 抢占 `CPU` 进行处理. 当这种打断很多很频繁的时候, `CPU` 处理的过程就会因为频繁的进程上下文切换而变的很低效, 从而使系统整体吞吐量下降. 此时我们可以使用开关禁止唤醒抢占的特性.


####11.3.1.3   睡眠唤醒补偿接口
-------



###11.3.2   新创建进程处理 `START_DEBIT`
-------

子进程在创建时, `vruntime` 初值首先被设置为 `min_vruntime`; 然后, 如果 `sched_features` 中设置了 `START_DEBIT` 位, `vruntime` 会在 `min_vruntime` 的基础上再增大一些.


####11.3.2.1  `START_DEBIT` 背景
-------

如果打开这个特性, 表示给新进程的 `vruntime` 初始值要设置得比默认值更大一些, 这样会推迟它的运行时间, 以防进程通过不停的 `fork` 来获得 `CPU` 时间片.


>即如果设置了 `START_DEBIT` 位, 则规定新进程的第一次运行要有延迟.



####11.3.2.2   `START_DEBIT` 实现
-------


与睡眠补偿类似, 新创建进程 `vruntime` 的设置和修正操作同样在进程入队(`CPU` 就绪队列) 时通过 `place_entity` 完成.



当进程被创建的时候, 也会调用 `place_entity` 进行补偿. 此时传入 `initial` 为 `1`. 表示此时进程刚创建好.


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L9079
static void task_fork_fair(struct task_struct *p)
{
    //  ......
    place_entity(cfs_rq, se, 1);
    //  ......
}
```


此时如果同时 `START_DEBIT` 也被设置, 则新创建子进程的虚拟运行时间 `vrunime`, 会在 `CPU` 就绪队列的虚拟运行时间的基础上增加 `sched_vslice(cfs_rq, se)`, 该时间刚好是当前进程实体 `se` 在就绪队列 `cfs_rq` 上当前周期的理想运行时间. 即相当于跳过该新创建的进程当前周期的运行, 让其从下一个周期才可以投入运行.


```cpp
//  https://elixir.bootlin.com/linux/latest/source/kernel/sched/fair.c#L3920
static void
place_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int initial)
{
    u64 vruntime = cfs_rq->min_vruntime;

    /*
     * The 'current' period is already promised to the current tasks,
     * however the extra weight of the new task will slow them down a
     * little, place the new task so that it fits in the slot that
     * stays open at the end.
     */
    if (initial && sched_feat(START_DEBIT))
        vruntime += sched_vslice(cfs_rq, se);

    //  ......

    /* ensure we never gain time by being placed backwards. */
    se->vruntime = max_vruntime(se->vruntime, vruntime);
}
```


其中


*   `sched_slice` 计算的就是进程实体 `se` 在 `CPU` 就绪队列上 `cfs_rq` 上当前周期的理想运行时间.


*   `calc_delta_fair` 函数计算的就是进程实体 `se` 运行 `delta_exec` 的时间所应该推进的虚拟运行时间.


*   则 `sched_vslice(cfs_rq, se)` 就是进程正好在当前周期运行完调度器分配给自己 `CPU` 时间(理想运行时间) 而应该增长的虚拟运行时间.


*   那么新创建的进程 `vruntime += sched_vslice(cfs_rq, se)` 就是假定该子进程当前周期已经完整的运行过了, 那么调度器根据虚拟运行时间选择进程的时候, 将跳过新创建子进程第一个运行周期.


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L698
/*
 * We calculate the vruntime slice of a to-be-inserted task.
 *
 * vs = s/w
 */
static u64 sched_vslice(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    return calc_delta_fair(sched_slice(cfs_rq, se), se);
}
```


####11.3.2.3  `START_DEBIT` 接口
-------








###11.3.3   `place_entity` 函数
-------


睡眠补偿的操作是在 `place_entity` 函数中完成. 该函数用于将进程插入到就绪队列中的时候对其虚拟运行时间进行修正. 内核在进程迁移(此时进程将从一个就绪队列到另外一个), 睡眠唤醒, 进程创建完成, 进入就绪队列的时候都需要调用 `place_entity` 对其虚拟运行时间进行修正.



```cpp
//  https://elixir.bootlin.com/linux/latest/source/kernel/sched/fair.c#L3920
static void
place_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int initial)
{
    u64 vruntime = cfs_rq->min_vruntime;

    /*
     * The 'current' period is already promised to the current tasks,
     * however the extra weight of the new task will slow them down a
     * little, place the new task so that it fits in the slot that
     * stays open at the end.
     */
    if (initial && sched_feat(START_DEBIT))
        vruntime += sched_vslice(cfs_rq, se);

    /* sleeps up to a single latency don't count. */
    if (!initial) {
        unsigned long thresh = sysctl_sched_latency;

        /*
         * Halve their sleep time's effect, to allow
         * for a gentler effect of sleepers:
         */
        if (sched_feat(GENTLE_FAIR_SLEEPERS))
            thresh >>= 1;

        vruntime -= thresh;
    }

    /* ensure we never gain time by being placed backwards. */
    se->vruntime = max_vruntime(se->vruntime, vruntime);
}
```


前面通过 `START_DEBIT` 和 `GENTLE_FAIR_SLEEPERS` 两个参数讲解了 `place_entity` 这个函数.


*   如果是新创建的子进程, 则 `initial` 为 `1`. 如果 `sched_feat(START_DEBIT)` 被设置, 则将跳过子进程当前周期的运行.


    ```cpp
    vruntime += sched_vslice(cfs_rq, se);
    ```


*   如果是进程休眠后被唤醒, 入队标识 `ENQUEUE_WAKEUP` 被设置, 则需要对进程进行补偿.

    1.  如果 `sched_feat(GENTLE_FAIR_SLEEPERS)` 没有被设置, 则 `vruntime -= sysctl_sched_latency`, 补偿进程一个调度周期

    2.  如果 `sched_feat(GENTLE_FAIR_SLEEPERS)` 被设置, 则 `vruntime -= (sysctl_sched_latency >> 1)`, 补偿进程一个调度周期的一半.


##11.4 唤醒抢占 `WAKEUP_PREEMPTION`
-------


##11.4.1  唤醒抢占标识 `WAKEUP_PREEMPTION` 背景
-------


该选项用于开启和关闭休眠进程的唤醒抢占. 如果开启了该选项则内核将在进程唤醒的时候检查是否可以进行进程抢占.


##11.4.2  唤醒抢占标识 `WAKEUP_PREEMPTION` 实现
-------


前面提到唤醒进程的运行时间基数 `sysctl_sched_wakeup_granularity` 的时候已经讲过 `check_preempt_wakeup` 函数. 该函数用于检查唤醒抢占, 并设置抢占调度标识.


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L6164
/*
 * Preempt the current task with a newly woken task if needed:
 */
static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_flags)
{
    //  ......

    /*
     * Batch and idle tasks do not preempt non-idle tasks (their preemption
     * is driven by the tick):
     */
    if (unlikely(p->policy != SCHED_NORMAL) || !sched_feat(WAKEUP_PREEMPTION))
        return;

    find_matching_se(&se, &pse);
    update_curr(cfs_rq_of(se));
    BUG_ON(!pse);
    if (wakeup_preempt_entity(se, pse) == 1) {
        // ......
        goto preempt;
    }

    return;

preempt:
    resched_curr(rq);

    // ......
}
```

可见只有开启了 `WAKEUP_PREEMPTION` 选项, 才会调用 `wakeup_preempt_entity` 检查唤醒抢占.

>另外只有普通进程才会进行唤醒抢占
>
>即
>
>p->policy != SCHED_NORMAL


##11.4.3  唤醒抢占标识 `WAKEUP_PREEMPTION` 接口
-------



##11.4  `NEXT_BUDDY` && `LAST_BUDDY`
-------


`CFS` 调度器通过虚拟运行时间来保证公平性, 不同权重和优先级的进程其虚拟运行时间按照不同的速度向前推进, 调度器每次选择虚拟运行时间最小的那个进程, 但是有时候也是需要一些外部干预的.


`CFS` 的就绪队列 `cfs_rq` 上通过 `curr` 指针记录了当前运行的进程实体, 同时还标记了其他进程实体的信息, 对应的就是 `next`, `last`, `skip` 几个指针. 他们标记了当前运行队列上的一些特殊进程, 他们通过对应的 `set_xxxx_buddy` 函数来设置. 通过 `__clear_buddies_xxxx` 和 `clear_buddies` 来清除.


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/sched.h#L432
struct cfs_rq{
    //  ......
    /*
     * 'curr' points to currently running entity on this cfs_rq.
     * It is set to NULL otherwise (i.e when none are currently running).
     */
    struct sched_entity *curr, *next, *last, *skip;
    //  ......
};
```

```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L6134
static void set_last_buddy(struct sched_entity *se)
{
    if (entity_is_task(se) && unlikely(task_of(se)->policy == SCHED_IDLE))
        return;

    for_each_sched_entity(se) {
        if (SCHED_WARN_ON(!se->on_rq))
            return;
        cfs_rq_of(se)->last = se;
    }
}

static void set_next_buddy(struct sched_entity *se)
{
    if (entity_is_task(se) && unlikely(task_of(se)->policy == SCHED_IDLE))
        return;

    for_each_sched_entity(se) {
        if (SCHED_WARN_ON(!se->on_rq))
            return;
        cfs_rq_of(se)->next = se;
    }
}

static void set_skip_buddy(struct sched_entity *se)
{
    for_each_sched_entity(se)
        cfs_rq_of(se)->skip = se;
}
```

###11.4.1 `NEXT_BUDDY` && `LAST_BUDDY` 背景
-------


###11.4.2 `NEXT_BUDDY` && `LAST_BUDDY` 实现
-------

####11.4.2.1  `LAST_BUDDY` 实现
-------

*   `xxxx_buddy` 的用途


这些标记直接影响的就是 `pick_next_task_fair` 下一个投入运行的进程的选择, 调度器通过 `pick_next_entity` 选择出下一个调度实体.



```cpp
pick_next_task_fair
    -=> pick_next_entity
```

[`pick_next_entity`](https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L3905) 的实现参见 [`kernel/sched/fair.c, version 4.14.14, line 3905`](https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L3905).


```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L3905
/*
 * Pick the next process, keeping these things in mind, in this order:
 * 1) keep things fair between processes/task groups
 * 2) pick the "next" process, since someone really wants that to run
 * 3) pick the "last" process, for cache locality
 * 4) do not run the "skip" process, if something else is available
 */
static struct sched_entity *
pick_next_entity(struct cfs_rq *cfs_rq, struct sched_entity *curr)
{
    //  首先从 left 和 curr 中选出虚拟运行时间最小的那个, 记为 left
    struct sched_entity *left = __pick_first_entity(cfs_rq);
    struct sched_entity *se;

    /*
     * If curr is set we have to see if its left of the leftmost entity
     * still in the tree, provided there was anything in the tree at all.
     */
    if (!left || (curr && entity_before(curr, left)))
        left = curr;

    se = left; /* ideally we run the leftmost entity */

    //  如果 left 正好设置了被 skip, 则继续从 second 与 curr 中选择记为 second
    /*
     * Avoid running the skip buddy, if running something else can
     * be done without getting too unfair.
     */
    if (cfs_rq->skip == se) {
        struct sched_entity *second;

        if (se == curr) {  //  被 skip 的是 curr, 则直接选择最左节点(可能是NULL)即可
            second = __pick_first_entity(cfs_rq);
        } else {  //  被 skip 的是最左节点, 则从 second(可能是NULL) 和 curr 中选择最合适的
            second = __pick_next_entity(se);
            if (!second || (curr && entity_before(curr, second)))
                second = curr;
        }

        //  如果 second 不可以唤醒 left
        //  (second.vruntime - left.vruntiem < calc_delta_fair(sysctl_sched_wakeup_granularity, left))
        if (second && wakeup_preempt_entity(second, left) < 1)
            se = second;
    }

    /*
     * Prefer last buddy, try to return the CPU to a preempted task.
     */
    if (cfs_rq->last && wakeup_preempt_entity(cfs_rq->last, left) < 1)
        se = cfs_rq->last;

    /*
     * Someone really wants this to run. If it's not unfair, run it.
     */
    if (cfs_rq->next && wakeup_preempt_entity(cfs_rq->next, left) < 1)
        se = cfs_rq->next;

    clear_buddies(cfs_rq, se);

    return se;
}
```



1.  首先选择出红黑树上的最左进程(虚拟运行时间最小) `left`, `second`, 然后和 `curr` 比较. 选择出非 `skip` 的, 且虚拟运行时间最小的那个调度实体, 记为 `second'`, 而 `left`.(为什么需要在红黑树上看两个, 如果第一个进程被标记为 `skip`, 则需要去看 `second`).


    *   首先选择出红黑树上的最左进程(虚拟运行时间最小) `left`, 然后和 `curr` 比较. 选择出虚拟运行时间最小的那个, 记为 `left'`.


    *   如果 `left` 正好被设置了 `skip`, 则继续看次左节点 `second`, 从剩余的两者中选择最恰当的 : 如果 `skip == curr`, 则只需要选择 `left` ; 如果 `skip == left`, 此时选择 `second` 与 `curr` 中虚拟运行时间最小的. 记为 `second'`. 注意如果选择的节点是次左节点 `second`, 则需要满足 `second` 不能唤醒 `left'`.

2.  如果运行队列 `cfs_rq` 的 `last` 指针被标记, 且 `last` 不能唤醒 `left'`, 则选择 `last` 调度实体,


3.  如果运行队列 `cfs_rq` 的 `next` 指针被标记, 且 `next` 不能唤醒 `left'`, 则选择 `next` 调度实体,


>  注意
>
>  如果 `last` 和 `next` 同时被设置且都满足唤醒条件, 那么将使用的是 `next` 指针指向的调度实体.
>
>  可见如果内核希望某个进程实体 `se` 下一次立马投入运行的时候, 可以通过 `set_next_buddy(se)` 将其设置为 `next`, 这样 `CFS` 调度器在选择的时候会有限选择它投入运行, 但是前提是满足唤醒条件, 即当前 `se` 不能唤醒 `left'`（红黑树中最左节点和 `curr` 中虚拟运行时间最小的那个.）


从设置中可以看出一些端倪.

1.  `last-buudy` 和 `next-buddy` 用来内核标记那些下一次调度时期望优先投入运行的进程, 通过这种方式内核将跳过通过检查虚拟运行时间最小进程的策略, 而优先选择他们(其中 `next-buddy` 的优先级最高)

2.  `last-buudy` 和 `next-buddy` 投入运行是需要条件的, 那就是当前 `buddy` 不能唤醒 `left` 即
    ```cpp
    wakeup_preempt_entity(buddy, left) < 1
    等价于
    buddy->vruntime - left.vrunime < calc_delta_fair(sysctl_sched_wakeup_granularity, left)
    其中
    left 是 curr 与红黑树最左节点中虚拟运行时间最小的那个进程实体
    右值是 left 运行 sysctl_sched_wakeup_granularity 所应该增加的虚拟运行时间.
    ```


*   `xxxx_buddy` 的设置


首先是 `last buddy` 的设置, 它标记的是当前 `CFS` 运行队列的上一个 `waker`. 因此需要在 `try_to_wake_up` 和 `try_to_wakeup_new` 唤醒抢占 `check_preempt_wakeup` 成功 (`resched_curr`) 后进行标识. 同样 `next` 在唤醒成功后也将标记


```cpp
/*
 * Preempt the current task with a newly woken task if needed:
 */
static void ·(struct rq *rq, struct task_struct *p, int wake_flags)
{
    struct task_struct *curr = rq->curr;
    struct sched_entity *se = &curr->se, *pse = &p->se;
    struct cfs_rq *cfs_rq = task_cfs_rq(curr);
    int scale = cfs_rq->nr_running >= sched_nr_latency;
    int next_buddy_marked = 0;

    /*
     * This is possible from callers such as attach_tasks(), in which we
     * unconditionally check_prempt_curr() after an enqueue (which may have
     * lead to a throttle).  This both saves work and prevents false
     * next-buddy nomination below.
     */
    if (unlikely(throttled_hierarchy(cfs_rq_of(pse))))
        return;

    //  如果 NEXT_BUDDY 开启
    //  同时运行队列上进程数超过 sched_nr_latency,
    //  wake_flags 不是 WF_FORK (对于新创建的进程wake_up_new_task时候将设置此标识)
    if (sched_feat(NEXT_BUDDY) && scale && !(wake_flags & WF_FORK)) {
        set_next_buddy(pse);
        next_buddy_marked = 1;
    }

    /*
     * We can come here with TIF_NEED_RESCHED already set from new task
     * wake up path.
     *
     * Note: this also catches the edge-case of curr being in a throttled
     * group (e.g. via set_curr_task), since update_curr() (in the
     * enqueue of curr) will have resulted in resched being set.  This
     * prevents us from potentially nominating it as a false LAST_BUDDY
     * below.
     */
    if (test_tsk_need_resched(curr))
        return;

    //  ......

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
    resched_curr(rq);
    /*
     * Only set the backward buddy when the current task is still
     * on the rq. This can happen when a wakeup gets interleaved
     * with schedule on the ->pre_schedule() or idle_balance()
     * point, either of which can * drop the rq lock.
     *
     * Also, during early boot the idle thread is in the fair class,
     * for obvious reasons its a bad idea to schedule back to it.
     */
    if (unlikely(!se->on_rq || curr == rq->idle))
        return;

    //  唤醒抢占成功后, 设置当前 waker 为 last
    if (sched_feat(LAST_BUDDY) && scale && entity_is_task(se))
        set_last_buddy(se);
}
```


设想一下, `waker` 唤醒了 `wakee` 进程, 那肯定是期望 `wakee` 能够立即投入运行的, 当然如果它不能立即投入运行的话, 那保守的策略也应该是恢复 `waker` 的运行, 因为它往往没有运行完自己的时间片, 而且总是 `cache-hot` 的.


因此 `next-buddy` 和 `last-buddy` 就为了这样一个场景而设计.


唤醒抢占成功检查后, 被唤醒的进程`wakee` 将被设置 `next-buddy`, 从而保证唤醒成功(即下次调度时只要满足要求即可被唤醒而投入运行), 同时如果 `LAST_BUDDY` 选项开启, 则 `last-buddy` 同时也被标记, 这样保证了如果 `next-buddy` 不满足要求, 不能立即运行的时候, 将选择对应的 `waker` 来运行.


**LAST_BUDDY 选项的作用**


如果可以进行唤醒抢占, 那么 `last-buddy` 将被设置为 `waker`(此时 `next-buddy` 将被设置为 `wakee`), 这样如果 `wakee` 因为某些原因没有真正被唤醒的时候(比如条件没有满足), 则将优先选择 `last-buddy`(waker) 作为下一个运行的进程.


**NEXT_BUDDY 选项的作用**


此外如果开启了 `NEXT_BUDDY` 选项, 那么无论是否可以唤醒抢占成功, 都会将 `wakee` 设置成 `next-buddy`. 但是调度标记 `TIF_NEED_RESCHED` 却只有唤醒抢占检查成功才会设置. 这样如果唤醒不成功, 只要 `next-buddy` 的设置没有被覆盖掉, 下次调度时将优先选择 `next-buddy`(wakee) 作为下一个进程.


> 如果可以进行唤醒抢占, 设置调度标记 `TIF_NEED_RESCHED`, next-buddy = wakee, 同时如果设置了 `LAST_BUDDY`, last-buddy = waker,
>
> 如果不可以进行唤醒抢占, 则只有在 NEXT_BUDDY 选项开启时, 才会设置 `next-buddy = wakee`, `last-buddy` 不必设置.


另外还有一些场景下, 也需要直接设置 `next-buddy` 的.




当进程 `CFS` 调度器通过 `dequeue_task_fair` 函数将进程 `p` 出队, 由于调度实体是一个层次结构的, 需要遍历该层次下的调度实体 `se` 将它们都出队, 在开启了组调度下, 该调度实体很有可能还有其他进程, 而且该调度实体(进程组)还没有用尽调度器分配给自己的 `CPU` 时间时, 说明该调度实体依旧是饥饿的, 则应该下一次优先调度该进程实体. 如下所示 :



```cpp
//  https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L5257
/*
 * The dequeue_task method is called before nr_running is
 * decreased. We remove the task from the rbtree and
 * update the fair scheduling stats:
 */
static void dequeue_task_fair(struct rq *rq, struct task_struct *p, int flags)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se;
    int task_sleep = flags & DEQUEUE_SLEEP;

    for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
        dequeue_entity(cfs_rq, se, flags);

        //  ......

        /* Don't dequeue parent if it has other entities besides us */
        if (cfs_rq->load.weight) {
            /* Avoid re-evaluating load for this entity: */
            se = parent_entity(se);
            /*
             * Bias pick_next to pick a task from this cfs_rq, as
             * p is sleeping when it is within its sched_slice.
             */
            if (task_sleep && se && !throttled_hierarchy(cfs_rq))
                set_next_buddy(se);
            break;
        }
        flags |= DEQUEUE_SLEEP;
    }

    // ......
}
```

前面我们已经讲解了 `last-buddy` 和 `next-buddy` 的设置, 那 `skip-buddy` 什么时候设置的呢.

而 `skip-buddy` 是啥时候设置的呢, 这就跟 `yield` 相关了. 当前运行的进程 `curr` 调用 `yield` 则尝试释放 `CPU` 给其他更饥饿的进程.


`yield()` 并没有指定当前进程要将执行权利移交给谁, 只是放弃运行权利, 至于下面由谁来运行, 完全看进程调度 `schedule`, 多用于 `I/O` 等待时, 进程短暂 `wait`, 但是并没有退出运行队列. 那么此时当前进程放弃 `CPU` 就可以通过 `set_skip_buddy` 设置为 `skip`, 调度器将自动跳过当前进程的运行周期. 参见 [`yield_task_fair`](https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L6390)

`yeild_to()` 指定了执行权将要移交的进程 `p`, 则通过将当前进程设置为 `skip-buddy`, 而将执行权移交的进程设置为 `next-buddy` 来完成. 调度器将跳过 `curr` 当前周期的执行而将执行权交给指定的进程 `p`. 参见 [`yield_to_task_fair`](https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/fair.c#L6426)


*   `xxxx_buddy` 的清除

[`clear_buddies`](https://elixir.bootlin.com/linux/v4.14.4/source/kernel/sched/fair.c#L3764) 用来清除内核对调度实体 `se` 的 `buddy` 标记信息.

```cpp
static void clear_buddies(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	if (cfs_rq->last == se)
		__clear_buddies_last(se);

	if (cfs_rq->next == se)
		__clear_buddies_next(se);
	if (cfs_rq->skip == se)
		__clear_buddies_skip(se);
}
```

*	当内核 [`pick_next_entity`](https://elixir.bootlin.com/linux/v4.14.4/source/kernel/sched/fair.c#L3958) 将调度实体 `se` 优选出来作为下一个进程的时候, 就需要清除之前对该实体的 `buddy` 标记.
因为该进程很有可能是因为自己是 `last-buddy` 或者 `next-buudy` 而优选出来的. 这时候清除 `buddy` 信息. 从而不会对下次优选
在进行影响. 有利于调度器的公平性和正常运作.参见 [`pick_next_entity`](https://elixir.bootlin.com/linux/v4.14.4/source/kernel/sched/fair.c#L3958)

*	[`yield_task_fair`](https://elixir.bootlin.com/linux/v4.14.4/source/kernel/sched/fair.c#L6395) 会强制将当前运行的调度实体 `curr->se` 让出 `CPU`.
这个是通过将其设置为 `skip_buddy` 而实现的. 但是在设置之前很有可能当前实体被设置了其它 `buddy` 标记. 因此也需要清楚. 参见 [`yield_task_fair`](https://elixir.bootlin.com/linux/v4.14.4/source/kernel/sched/fair.c#L6395)

*	[`dequeue_entity`](https://elixir.bootlin.com/linux/v4.14.4/source/kernel/sched/fair.c#L3799) 当进程入队的时候, 也是需要将进程的 `buddy` 清除掉的.
因此进程很有可能之前是带着 `buddy` 标记睡眠或者让出 `CPU`，不清除 `buddy` 标记必然对下次调度的行为造成影响. 参见 [`dequeue_entity`](https://elixir.bootlin.com/linux/v4.14.4/source/kernel/sched/fair.c#L3799)


###11.4.3 `NEXT_BUDDY` && `LAST_BUDDY` 接口
-------


内核通过 `next_buddy` 和 `last_buddy` 标记了下次调度优先希望选择的进程实体.
那么什么时候将这些 `buddys` 清除掉呢 ?

```cpp
dequeue_entity
    -=> clear_buddies(cfs_rq, se);
```


`check_preempt_tick` 函数用来检查周期性调度.
如果当前运行的进程 `curr` 的实际运行时间 `delta_exec` > 调度器分配的理想运行运行时间 `ideal_runtime`
此时说明当前进程已经运行了很久, 应该被调度出去了. 这时候如果当前进程 `curr` 依旧被设置了 `buddies`,
应该将其清除掉.


```cpp
//  https://elixir.bootlin.com/linux/v4.14.4/source/kernel/sched/fair.c#L3833
static void
check_preempt_tick(struct cfs_rq *cfs_rq, struct sched_entity *curr)
{
        unsigned long ideal_runtime, delta_exec;
        struct sched_entity *se;
        s64 delta;

        ideal_runtime = sched_slice(cfs_rq, curr);
        delta_exec = curr->sum_exec_runtime - curr->prev_sum_exec_runtime;
        if (delta_exec > ideal_runtime) {
                resched_curr(rq_of(cfs_rq));
                /*
                 * The current task ran long enough, ensure it doesn't get
                 * re-elected due to buddy favours.
                 */
                clear_buddies(cfs_rq, curr);
                return;
        }

        // ......
}
```

同样每次 `pick_next_entity` 选择完进程之后, 也应该将 `buddies` 清除掉.

```cpp
static struct sched_entity *
pick_next_entity(struct cfs_rq *cfs_rq, struct sched_entity *curr)
{
        //  ......

        clear_buddies(cfs_rq, se);

        return se;
}
```

当前如果是通过 `yield` 放弃的 `CPU`, 则在设置下一个调度实体之前也要清除之前的 `buddies` 的设置.

```cpp
//
static void yield_task_fair(struct rq *rq)
{
        // ......
        clear_buddies(cfs_rq, se);
        // ......
        set_skip_buddy(se);
}
```

##11.5	`CACHE_HOT_BUDDY`
-------

###11.5.1	`CACHE_HOT_BUDDY`
-------


调度器调度和选核的时候有一项重要的参考就是 `cache-hot`

load_balance 在做负载均衡的时候, can_migrate_task 判断一个进程是否可以从 env->src
迁移到 env->dst 的关键指标就是 task_hot.


```cpp
load_balance
-=> detach_tasks
	-=>can_migrate_task
		-=>task_hot
```


关于 cache-hot 的判断, 最关键的指标是 `sysctl_sched_migration_cost`, 参照第 7 节的描述.
但是如果开启了 CACHE_HOT_BUDDY, 那么调度器会认为 CFS_RQ 上 LAST_BUDDY 和 NEXT_BUDDY
也是 cache-hot 的.

```cpp
/*
 * Is this task likely cache-hot:
 */
static int task_hot(struct task_struct *p, struct lb_env *env)
{
	s64 delta;

	lockdep_assert_held(&env->src_rq->lock);

	if (p->sched_class != &fair_sched_class)
		return 0;

	if (unlikely(p->policy == SCHED_IDLE))
		return 0;

	/*
	 * Buddy candidates are cache hot:
	 */
	if (sched_feat(CACHE_HOT_BUDDY) && env->dst_rq->nr_running &&
			(&p->se == cfs_rq_of(&p->se)->next ||
			 &p->se == cfs_rq_of(&p->se)->last))
		return 1;

	if (sysctl_sched_migration_cost == -1)
		return 1;
	if (sysctl_sched_migration_cost == 0)
		return 0;

	delta = rq_clock_task(env->src_rq) - p->se.exec_start;

	return delta < (s64)sysctl_sched_migration_cost;
}
```

##15.6	WAKEUP_PREEMPTION
-------


在前面讲唤醒抢占的时候, 我们了解了 wakeup_preempt_entity 唤醒成功设置抢占标记的条件.
而 WAKEUP_PREEMPT 是唤醒抢占的开关, 如果不设置 WAKEUP_PREEMPTION, 那么将不进行唤醒抢占的检查.
那么唤醒将只会将 wakee 进程设置为 RUNNING, 并加入到就绪队列中.


```cpp
/*
 * Preempt the current task with a newly woken task if needed:
 */
static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_flags)
{
	// ......
	/*
	 * Batch and idle tasks do not preempt non-idle tasks (their preemption
	 * is driven by the tick):
	 */
	if (unlikely(p->policy != SCHED_NORMAL) || !sched_feat(WAKEUP_PREEMPTION))
		return;

	find_matching_se(&se, &pse);
	update_curr(cfs_rq_of(se));
	BUG_ON(!pse);
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

	// ......
}
```


##15.7	HRTICK
-------


##15.8	DOUBLE_TICK
-------


##15.9	LB_BIAS
-------


##15.10	NONTASK_CAPACITY
-------

##15.11	TTWU_QUEU
-------


##15.12	SIS_AVG_CPU
-------

##15.13	SIS_PROP
-------

##15.14	WARN_DOUBLE_CLOCK
-------

##15.15	RT_PUSH_IP
-------

##15.16	RT_RUNTIME_SHARE
-------

RT 线程的优先级是很高的, 如果 RT 线程一直运行, 则会把 COU 占满, 因此往往会通过 RT_BANDWIDTH 来限制 RT 的 CPU 使用率.

调度器在检查一个调度实体运行时间是否超额时, 实际检查的是它所在的运行队列的 rt_rq[cpu]->rt_time 是否超过 rt_rq[cpu]->rt_runtime;

但是 BANDWIDTH 都是一个整体的概念, 为了防止当前 CPU 上的线程负载很高被 throttled, 而其他核上 CPU 上线程负载却很低, 远没有
到达要被 throttled 的水平线, 这样整体的 BANDWIDTH 其实没有超过限额. 因此一般调度器的 BANDWIDTH 都会实现一种 steal 机制或者 share
机制. 就是 BANDWIDTH 的时间片是大家共享的. 如果一个线程超出了限制, 而同一个 BANDWIDTH 的其他进程有空闲的 CPU 时间,
那么超出限额的进程可以尝试从空闲的 CPU 上窃取一部分运行时间.

在 SMP系统中, 如果内核使能了 RT_RUNTIME_SHARE 特性, 如果运行队列的运行时间已经超额, 则会尝试去其他 CPU 上的 RT_RQ 队列中
"借" 时间以扩张 rt_rq[cpu]->rt_runtime.


在 RT_BANDWIDTH 中定时器处理函数中, 如果发现当前 RQ 被 throttled 了.
则会调用 balance_runtime 窃取其他 RQ 的 CPU 时间.

而 RT_RUNTIME_SHARE 就是 balance_runtime 的开关, 只有配置了 RT_RUNTIME_SHARE 才会做窃取.

```cpp
//https://elixir.bootlin.com/linux/v4.14.14/source/kernel/sched/rt.c#L795
static void balance_runtime(struct rt_rq *rt_rq)
{
	if (!sched_feat(RT_RUNTIME_SHARE))
		return;

	if (rt_rq->rt_time > rt_rq->rt_runtime) {
		raw_spin_unlock(&rt_rq->rt_runtime_lock);
		do_balance_runtime(rt_rq);
		raw_spin_lock(&rt_rq->rt_runtime_lock);
	}
}
```


#   参考资料
-------

[进程管理 | Linux Performance](http://linuxperf.com/?cat=10)

[Linux 调度器 BFS 简介](https://www.ibm.com/developerworks/cn/linux/l-cn-bfs/)

[从几个问题开始理解CFS调度器](http://ju.outofmemory.cn/entry/105407)


> 关于 `waker` 和 `wakee`
>
>*  waker : The running process which try to wakeup an un-running process
>
>*  wakee : The un-running process to be wakeup


<br>

*      本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*      采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*      基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

`
