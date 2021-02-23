# 1 Linux 内核特性演进史
-------

本文最早是 [知乎大神 larmbr宇](https://www.zhihu.com/people/zhan-jian-yu) 2015 年对知乎提问 [现在的 Linux 内核和 Linux 2.6 的内核有多大区别](https://www.zhihu.com/question/35484429) 的回答, 鄙人当时看到后, 叹为观止, 这正是我一直希望做的, 但是作者已经不再维护, 甚是惋惜, 因此接手维护, 希望能尽自己的一份微薄之力.

![为什么会有这篇文章](https://raw.githubusercontent.com/gatieme/LDD-LinuxDeviceDrivers/master/study/kernel/00-DESCRIPTION/images/0000-1-why.png)


> 注: 
>
> 鄙人不是什么大神, 只是一个对 Linux 内核非常感兴趣的人肉转码机, 在自己工作之余, 不希望原作者的神作, 因为时间的推移, 随着 Linux 社区的不断壮大而蒙尘.
>
> 接手更新原作者的这篇神作, 我只是站在巨人的肩膀上, 让这篇文章不断的与时俱进.
> 
> 因个人能力有限, 文章内容有错误再所难免, 请您一定要批评指正. 欢迎随时叨扰, 联系方式我稍后奉上.

> gatieme 记于 2020/10/07 22:51.
>
> 联系方式:
>
>   WeChat  :   gatieme
>
>   EMAIL   :   gatieme@163.com




这个问题挺大的.

2.6 时代跨度非常大, 从2.6.0 (2003年12月发布[36]) 到 2.6.39(2011年5月发布), 跨越了 40 个大版本.
3.0(原计划的 2.6.40, 2011年7月发布) 到 3.19（2015年2月发布）.
4.0（2015年4月发布）到4.2（2015年8月底发布）.


总的来说, 从进入2.6之后, 每个大版本跨度开发时间大概是 2 - 3 个月. 2.6.x , 3.x, 4.x, 数字的递进并没有非常根本性, 非常非常非常引人注目的大变化, 但每个大版本中都有一些或大或小的功能改变. 主版本号只是一个数字而已. 不过要直接从 2.6.x 升级 到 3.x, 乃至 4.x, 随着时间间隔增大, 出问题的机率当然大很多.


个人觉得 Linux 真正走入严肃级别的高稳定性, 高可用性, 高可伸缩性的工业级别内核大概是在 2003 年后吧. 一是随着互联网的更迅速普及, 更多的人使用、参与开发. 二也是社区经过11年发展, 已经慢慢摸索出一套很稳定的协同开发模式, 一个重要的特点是 社区开始使用版本管理工具进入管理, 脱离了之前纯粹手工（或一些辅助的简陋工具）处理代码邮件的方式, 大大加快了开发的速度和力度.


因此, 我汇总分析一下从 2.6.12 (2005年6月发布, 也就是社区开始使用 git 进行管理后的第一个大版本）, 到 4.2 (2015年8月发布)这中间共 **51个大版本**, 时间跨度**10年**的主要大模块的一些重要的变革.


![Linux and 企鹅](https://raw.githubusercontent.com/gatieme/LDD-LinuxDeviceDrivers/master/study/kernel/00-DESCRIPTION/images/0000-2-linux.jpg)



> 感谢知友 [@costa](https://www.zhihu.com/people/78ceb98e7947731dc06063f682cf9640) 提供无水印题图)


# 2 内容目录
-------


| 内容 | GITHUB |
|:---:|:------:|
| **调度子系统(scheduling) [已完成]** | [`SCHEDULER`](https://github.com/gatieme/LDD-LinuxDeviceDrivers/blob/master/study/kernel/00-DESCRIPTION/SCHEDULER.md) |
| **内存管理子系统(memory management) [已完成]** | [`MEMORY_MANAGER`](https://github.com/gatieme/LDD-LinuxDeviceDrivers/blob/master/study/kernel/00-DESCRIPTION/MEMORY_MANAGER.md) |
| **中断与异常子系统(interrupt & exception)[填坑中]** | |
| **时间子系统(timer & timekeeping)** | |
| **同步机制子系统(synchronization)** | |
| **块层(block layer)** | |
| **文件子系统(Linux 通用文件系统层VFS, various fs)** | |
| **网络子系统(networking)** | |
| **调试和追踪子系统(debugging, tracing)** | |
| **虚拟化子系统(kvm)** | |
| **控制组(cgroup)** | |
