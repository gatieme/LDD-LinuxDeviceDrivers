Linux进程退出详解
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |

1.	调度器概述introduction
2.	调度器演变develop
3.	调度器的设计design
4.	周期性调度器periodic_scheduler
5.	主调度器main_schedulrt
6.	优先级priority
7.	抢占-内核抢占和用户抢占以及抢占的时机preempt
8.	进程切换context_switch

9.	完全公平调度器cfs-设计
10.	cfs-负荷权重
11.	cfs-虚拟时钟
12.	cfs-延迟跟踪
13.	cfs-操作pick_next_task
14.	rt实时调度器
15.	dl时调度器
16.	stop调度
17.	idle调度器
18.	SMP调度
19.	调度域和控制组

http://blog.csdn.net/b02042236/article/details/6076473
http://www.tuicool.com/articles/MjyANr
http://iamzhongyong.iteye.com/blog/1895728
http://blog.csdn.net/xiaofei0859/article/details/8113211



BFS
https://en.wikipedia.org/wiki/Brain_Fuck_Scheduler

http://blog.csdn.net/u201017971/article/details/50511511


http://baike.baidu.com/link?url=qO-044OZarVCuMDuioyhbYLswbB7MkyVwW3vPbWzHGE6j2-2X3IKIiXCUecABqkg9KCXSPCQ3Kc6IP26uCT0JK

Con Kolivas

http://www.ibm.com/developerworks/cn/linux/l-cn-bfs/


rifs进程调度比起bfs cfs好在哪差在哪

交互性极佳，特别是在大负载，那种交互性差距很明显
吞吐量比bfs和cfs低一半，但还是比windows高1/3

要测试交互性的差距请用mplayer测，
要体验交互性大可以开make -j512然后看网页，听音乐，移动窗口。


