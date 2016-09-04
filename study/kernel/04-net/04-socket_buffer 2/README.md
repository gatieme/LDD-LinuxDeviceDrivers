进程虚拟地址空间
=======

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6225543) |


#1	网络访问层
-------

前面讲述了Linux内核中网络子系统的结构, 现在我们把注意力转向**网络实现的第一层, 即网络访问层**. 该层主要负责在计算机之间传输信息，与网卡的设备驱动程序直接协作.

本节不会讨论网卡驱动程序的实现和相关的问题, 因为其中采用的方法与第6章的描述仅稍有不同. 本节将详细介绍由各个网卡驱动程序提供、由网络实现代码使用的接口，它们提供了硬件的抽象视图.

这里根据以太网帧来解释如何在"线上"(on the cable)表示数据，并描述接收到一个分组之后, 将该分组传递到更高层之前，需要完成哪些步骤.

#2	网络设备的表示
-------

在内核中, 每个网络设备都表示为net_device结构的一个实例。在分配并填充该结构的一个实例之后，必须用net/core/dev.c中的register_netdev函数将其注册到内核。该函数完成一些初始化任务，并将该设备注册到通用设备机制内。这会创建一个sysfs项（参见10.3节）/sys/class/net/<device>，关联到该设备对应的目录。如果系统包含一个PCI网卡和一个环回接口设备，则在