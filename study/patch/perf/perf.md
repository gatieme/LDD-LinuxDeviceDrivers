
1	perf record sched:sched_wakeup_new ./bug_fork

perf/ftrace : Fix repetitious traces when specify a target task



When use perf to trace the sched_wakeup and sched_wakeup_new tracepoint, there is a bug that output the same event repetitiously, It can be reproduced by :


```cpp
perf record -e sched:sched_wakeup ./bug_fork
OR
./bug_fork # pid 1052
perf record -e sched:sched_wakeup -p 1052
```

```cpp
swapper     0 [000]  3854.085452:     sched:sched_wakeup: comm=perf pid=1101 prio=120 target_c
pu=000
swapper     0 [000]  3854.085473:     sched:sched_wakeup: comm=perf pid=1101 prio=120 target_c
pu=000
bug_fork  1101 [000]  3854.089933: sched:sched_wakeup_new: comm=bug_fork pid=1102 prio=120 targ
et_cpu=001
bug_fork  1101 [000]  3854.089950: sched:sched_wakeup_new: comm=bug_fork pid=1102 prio=120 targ
et_cpu=001
bug_fork  1101 [000]  3854.089959: sched:sched_wakeup_new: comm=bug_fork pid=1102 prio=120 targ
et_cpu=001
bug_fork  1102 [001]  3854.092185:     sched:sched_wakeup: comm=rcu_sched pid=8 prio=120 target
_cpu=001
swapper     0 [000]  3854.665213:     sched:sched_wakeup: comm=bug_fork pid=1101 prio=120 targ
et_cpu=000
swapper     0 [000]  3854.665253:     sched:sched_wakeup: comm=bug_fork pid=1101 prio=120 targ
et_cpu=000

```

Some events not only monitor the current task, but also specify a target task. For example, the sched_wakeup/sched_wakeup_new tracepoint will be caught when the current task wakeup the target task which we traced on.

the commit bed5b2 has designed a method to trace these events which specify a target task. Due to these events are registered for each CPU(percpu), so perf_swevent_event will match the event multiple times, the number of repetitions is just the number of CPUs.

On the other hand perf_tp_event will only match an event event at a time, so we will return after an event matched.



2	perf sched record 支持 -p 1 但是不支持 task

3	perf/trace record 显示current有时候不正常, 只显示pid, 不显示comm
	stat_runtime	current进程不正确

4	pr_debug 不支持 arm64 和 arm32
