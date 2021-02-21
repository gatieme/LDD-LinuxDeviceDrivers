服务器体系与共享存储器架构
=======

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6393814) |


#1	目录
-------

|  目录  |  描述  |
|:-------:|:-------:|
| 描述物理内存    		    |
| 页表管理         				|
| 初始化内存管理  			 |
| 伙伴系统物理内存管理  |
| slab分配器       		    |
| 非连续内存分配  			 |
| 高端内存管理    		   |
| --页面帧回收--       			 |
| --交换管理--         			    |
| --进程虚拟地址空间--         |
| 共享内存虚拟文件系统  |
| 内存溢出管理                |
|  进程虚拟内存 |
|  页缓存和块缓存 |
|  页面回收和页交换 |

#2	参考内容
-------



|   链接   |
|:-------:|
| [内存管理（一）内存模型之Node](http://biancheng.dnbcw.info/linux/387391.html)                             |
| [Linux 内存管理 重要结构体](http://blog.chinaunix.net/uid-26009500-id-3078986.html)                       |
| [Bootmem机制](http://blog.csdn.net/samssm/article/details/25064897)                                       |
| [Linux-2.6.32 NUMA架构之内存和调度](http://www.cnblogs.com/zhenjing/archive/2012/03/21/linux_numa.html)   |
| [Linux 用户空间与内核空间——高端内存详解](http://blog.csdn.net/tommy_wxie/article/details/17122923)        |
| [探索 Linux 内存模型](http://www.ibm.com/developerworks/cn/linux/l-memmod/)                               |
| [Linux内存管理](http://blog.chinaunix.net/uid/21718047/cid-151509-list-2.html)                            |
| [内存管理-之内核内存管理-基于linux3.10](http://blog.csdn.net/shichaog/article/details/45509917)           |
| [内存管理(一)](http://www.cnblogs.com/openix/p/3334026.html)                                              |
| [Linux内存管理原理](http://www.cnblogs.com/zhaoyl/p/3695517.html)                                         |
| [第 15 章 内存映射和 DMA](http://www.embeddedlinux.org.cn/ldd3/ch15.html)                                 |
| [ 内存管理（二）struct page ](http://blog.chinaunix.net/uid-30282771-id-5176971.html)                     |
| [Linux内存管理](http://blog.csdn.net/column/details/linux--mm.html)                                       |
