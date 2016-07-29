Linux进程调度之stop调度器类
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |


#stop调度器类
-------

stop调度器类是优先级最高的调度器类

```c
/*
 * Simple, special scheduling class for the per-CPU stop tasks:
 */
const struct sched_class stop_sched_class = {
    .next           = &dl_sched_class,

    .enqueue_task       = enqueue_task_stop,
    .dequeue_task       = dequeue_task_stop,
    .yield_task         = yield_task_stop,

    .check_preempt_curr     = check_preempt_curr_stop,

    .pick_next_task     = pick_next_task_stop,
    .put_prev_task      = put_prev_task_stop,

#ifdef CONFIG_SMP
    .select_task_rq     = select_task_rq_stop,
    .set_cpus_allowed       = set_cpus_allowed_common,
#endif

    .set_curr_task      = set_curr_task_stop,
    .task_tick          = task_tick_stop,

    .get_rr_interval    = get_rr_interval_stop,

    .prio_changed       = prio_changed_stop,
    .switched_to        = switched_to_stop,
    .update_curr        = update_curr_stop,
};
```


#队列操作
-------


```c
```



#选择进程
-------




