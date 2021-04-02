
```cpp
#sh test.sh `pidof yes`
```

```cpp
PID: 6390
COMM: yes
threads: 1
POLICY: NORMAL CFS(0)
BASE-------------------------------------------------------------------
prio                                         :                  125
se.nr_migrations                             :                   47
nr_switches                                  :                  741
nr_voluntary_switches                        :                    0
nr_involuntary_switches                      :                  741
se.load.weight                               :               343040
TIME(s)-------------------------------------------------------------------
now                                          :      14956139.657933187
se.exec_start                                :      14955774.229852083
se.vruntime                                  :       6789957.542831753
se.sum_exec_runtime                          :           161.523601661
start_time                                   :      14955978.66683773
real_start_time                              :      14955978.66683873
delta_time                                   :           161.591249414
```

| 字段 | 描述 |
|:----:|:----:|
| prio | 优先级 |
| nr_migrations | 迁移次数 |
| nr_switches | 总切换次数 |
| nr_voluntary_switches | 主动上下文切换次数, 主动睡眠导致的上下文切换, 一般是因为等待资源而退出  |
| nr_involuntary_switches | 被动上下文切换次数, 线程被抢占导致的调度 |
| load.weight | 进程的负载权重 |
| now | 当前的系统时间 |
| exec_start | 当前周期内, 进程开始执行的时间 |
| vruntime | CFS 进程的 vruntime |
| sum_exec_runtime | 进程的累计运行时间 |
| start_time | 进程启动(创建)的时间  |
| real_start_time/start_boottime | 以 bootime 为基准的, 进程启动(创建)的时间 |
| delta_time | 进程创建至今, 经过的时间(不需要一定是 on_cpu/on_rq) 的 |
