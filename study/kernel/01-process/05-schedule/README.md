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

http://blog.csdn.net/li4850729/article/details/28136643
http://blog.csdn.net/xiaofei0859/article/details/8113211
http://blog.sina.com.cn/s/blog_502c8cc401012pxj.html
http://www.cnblogs.com/hustcat/archive/2009/08/31/1557507.html
http://carlsama.blog.163.com/blog/static/19491207420132199327108/