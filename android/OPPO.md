---

title: OPPO ANDROID 优化
date: 2021-02-15 00:32
author: gatieme
tags:
    - linux
    - tools
categories:
        - 技术积累
thumbnail:
blogexcerpt: ANDROID

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

2   **OPPO**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***

# 1 SCHED
-------

## 1.1 OPPO oplus_scheduler
-------

| feature | sysfs interface |
|:-------:|:---------------:|
| oplus_scheduler | OPLUS_SCHEDASSIST_PROC_DIR |
| slide_boost | OPLUS_SLIDEBOOST_PROC_DIR |
| sched_assist | OPLUS_SCHEDASSIST_PROC_DIR |

### 1.1.1 Sched Assist
-------

OPPO 通过 global_sched_assist_scene 进行场景标记和感知. 然后通过 sched_assist_scene() 获取到当前所属的场景.

| CONFIG | 描述 | 源代码路径 | 代表手机 | 芯片平台 |
|:------:|:---:|:---------:|:------:|:-------:|
| CONFIG_OPLUS_FEATURE_SCHED_ASSIST | |
| CONFIG_OPLUS_FEATURE_SCHED_SPREAD | nr-balance to spread tasks | `kernel/sched/oem_sched/oem_fair`<br>`vendor/oplus/kernel/oplus_performance_5.10/misc/sched_assist`<br>`kernel/oplus_perf_sched/sched_assist` | OPPO FindX5 Pro | Snapdragon 8 Gen 1, QCOM-5.10, ColorOS 12.1 PFEM10_11_A.18 |


#### 1.1.1.1 SCHED_ASSIST
-------

#### 1.1.1.2 SCHED_SPREAD
-------

CONFIG_OPLUS_FEATURE_SCHED_SPREAD 实现了一个仅通过 nr_running 数量来进行分级选核和负载均衡的策略, 无需跟踪 TASK 和 RQ 的实际负载信息, 因此更快速, overhead 更低.

> PS.
> 这种算法我们之前在服务器场景也曾经尝试过, 当时是为了 BYPASS 负载均衡和选核的 overload.
> 不过当时的数据库业务场景, 并没有看到明显的收益.

oplus_task_struct->lb_state 记录了进程的负载状态 high 或者 low. 每次 WALT 更新进程的负载信息时, 根据进程的负载信息动态更新.

```cpp
walt_update_task_ravg()
update_task_demand()
update_history()
-=> update_load_flag()
```

PER_CPU(task_lb_count) 记录了各个 cpu cgroup 分组 high 负载和 low 负载的任务数量. 每次进程入队和出队的时候, 根据进程的 cgroup 信息 `task_lb_sched_type()` 和其负载信息(high 或者 low), 通过 inc_ld_stats()/dec_ld_stats() 维护 task_lb_count 计数.

```cpp
enqueue_task()
-=> trace_android_rvh_after_enqueue_task(rq, p) =======> android_rvh_enqueue_task()
    -=> inc_rq_walt_stats(rq, p)
        -=> dec_ld_stats(p, rq)

dequeue_task()
-=> trace_android_rvh_after_dequeue_task(rq, p) =======> android_rvh_dequeue_task()
    -=> dec_rq_walt_stats(rq, p)
        -=> dec_ld_stats(p, rq)
```

选核时候, 通过 sched_assist_spread_tasks() 按照 nr_running(也就是 PER_CPU(task_lb_count)) 来选核, 寻找到 rq_nr/lowest_nr 最小的 CPU. lowest_nr 相同时则选择 NR 加权负载 rq_nr_load 最小的.

```cpp
walt_find_energy_efficient_cpu()
-=> walt_find_best_target()
    -=> sched_assist_spread_tasks(p, new_allowed_cpus, fbt_env->order_index, fbt_env->end_index, fbt_env->skip_cpu, candidates, fbt_env->strict_max)
        -=> find_spread_lowest_nr_cpu(p, &visit_cpus, sched_type, prev_cpu, skip_cpu, &lowest_nr, &lowest_nr_load, &lowest_nr_cpu)
-=> if (fbt_env.fastpath == NR_WAKEUP_SELECT) best_energy_cpu = cpumask_first(candidates)
```

其中 rq_nr_load 直接已 nr_running 做加权来预估 CPU 的负载, 不同 sched_type 的 nr_running 权重不同.

$rq_nr_load = 1000 \times ux_nr + 100 \times top_nr + 10 \times fg_nr + bg_nr$


# 2 MM
-------

| 编号 | 特性 | CONFIG | 描述 | 链接 |
|:---:|:----:|:------:|:----:|:---:|
| 1 | Multi Freearea | CONFIG_PHYSICAL_ANTI_FRAGMENTATION | 物理内存反碎片机制. | [mm/multi_freearea.c](https://github.com/oppo-source/android_kernel_modules_and_devicetree_oppo_sm8250/tree/oppo/sm8250_s_12.1/vendor/oplus/kernel/oplus_performance/multi_freearea) |
| 2 | VM Anti Fragment | NA | 虚拟内存反碎片化机制. | [mm/vm_anti_fragment.c](https://github.com/oppo-source/android_kernel_modules_and_devicetree_oppo_sm8250/tree/oppo/sm8250_s_12.1/vendor/oplus/kernel/oplus_performance/vm_anti_fragment) |
| 3 | Multi Kswapd | NA | 多线程 KSWAPD. 加速内存回收. | [vendor/oplus/kernel/oplus_performance/multi_kswapd](https://github.com/oppo-source/android_kernel_modules_and_devicetree_oppo_sm8250/tree/oppo/sm8250_s_12.1/vendor/oplus/kernel/oplus_performance/multi_kswapd)
| 4 | Health Info | CONFIG_OPLUS_MEM_MONITOR | NA | [mm/healthinfo](https://github.com/oppo-source/android_kernel_modules_and_devicetree_oppo_sm8250/tree/oppo/sm8250_s_12.1/vendor/oplus/kernel/oplus_performance/healthinfo) |
| 5 | MemoryLeak Detect |

## 2.1 Anti Fragmentation
-------

第 17 届中国 Linux 内核开发者大会(CLK-2022) ["内存管理与异构计算" 分论坛](https://live.csdn.net/v/247710)的第三个议题 "[手机平台] Linux 内存反碎片化" 来自 OPPO 的开发者李培锋春就对 OPPO ColorOS 上内存反碎片化机制做了深入的讲解.

### 2.1.1 MF(Multi Freearea) 物理内存反碎片化机制
-------

ColorOS 提供了 [MF(Multi Freearea)](https://github.com/oppo-source/android_kernel_modules_and_devicetree_oppo_sm8250/tree/oppo/sm8250_s_12.1/vendor/oplus/kernel/oplus_performance/multi_freearea) 提供了物理内存反碎片化的能力.



### 2.1.2 CSVM(Centralize Small Virtual Mem) 虚拟内存反碎片化机制
-------

ColorOS 提供了 [vm_anti_fragment](https://github.com/oppo-source/android_kernel_modules_and_devicetree_oppo_sm8250/tree/oppo/sm8250_s_12.1/vendor/oplus/kernel/oplus_performance/vm_anti_fragment) 提供了物理内存反碎片化的能力.



## 2.2 Multi Kswapd
-------


## 2.3 Memleak Detect
-------


### 2.3.1 DUMP_TASKS_MEM
-------

[task_mem](https://github.com/oppo-source/android_kernel_modules_and_devicetree_oppo_sm8250/tree/oppo/sm8250_s_12.1/vendor/oplus/kernel/oplus_performance/memleak_detect/task_mem) 提供了 Dump Task Mem 的能力.

[ion_track](https://github.com/oppo-source/android_kernel_modules_and_devicetree_oppo_sm8250/blob/oppo/sm8250_s_12.1/vendor/oplus/kernel/oplus_performance/memleak_detect/ion_track) 提供了 dump_ion_info 的能力.

[malloc_track](https://github.com/oppo-source/android_kernel_modules_and_devicetree_oppo_sm8250/tree/oppo/sm8250_s_12.1/vendor/oplus/kernel/oplus_performance/memleak_detect/malloc_track)





# 3 帧感知
-------


| 编号 | 特性 | CONFIG | 描述 | 链接 |
|:---:|:----:|:------:|:----:|:---:|
| 1 | Input Boost | [PLUS_FEATURE_INPUT_BOOST](https://github.com/oppo-source/android_kernel_modules_and_devicetree_oppo_sm8250/blob/oppo/sm8250_s_12.1/vendor/oplus/kernel/oplus_performance/input_boost/Kconfig) | NA | [vendor/oplus/kernel/oplus_performance/input_boost](https://github.com/oppo-source/android_kernel_modules_and_devicetree_oppo_sm8250/tree/oppo/sm8250_s_12.1/vendor/oplus/kernel/oplus_performance/input_boost) |
| 2 | Identify Task Itself(IM)) | [OPLUS_FEATURE_IM](https://github.com/oppo-source/android_kernel_modules_and_devicetree_oppo_sm8250/blob/oppo/sm8250_s_12.1/vendor/oplus/kernel/oplus_performance/im/Kconfig) | 通过 task->im_flag 标记 ANDROID 下的关键线程类型. 在许多情况下, 我们需要识别关键任务, 虽然可以通过字符串比较(比较进程的 COMM 是否是我们关心的任务), 但在一些性能敏感的上下文中, 如调度器路径下, 这样做不是一个好的选择. 这个 IM 工具可以让我们只通过一个比较来检查任务. | [vendor/oplus/kernel/oplus_performance/im](https://github.com/oppo-source/android_kernel_modules_and_devicetree_oppo_sm8250/blob/oppo/sm8250_s_12.1/vendor/oplus/kernel/oplus_performance/im/im.h) |









<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
