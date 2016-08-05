服务器体系与共享存储器架构
=======

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6225543) |





##参照
-------

|   链接   |
|:-------:|
| [内存管理（一）内存模型之Node](http://biancheng.dnbcw.info/linux/387391.html) |
| [Linux 内存管理 重要结构体](http://blog.chinaunix.net/uid-26009500-id-3078986.html) |
| [Bootmem机制](http://blog.csdn.net/samssm/article/details/25064897) |
| [Linux-2.6.32 NUMA架构之内存和调度](http://www.cnblogs.com/zhenjing/archive/2012/03/21/linux_numa.html) |
| [Linux 用户空间与内核空间——高端内存详解](http://blog.csdn.net/tommy_wxie/article/details/17122923) |
| [探索 Linux 内存模型](http://www.ibm.com/developerworks/cn/linux/l-memmod/) |
| [Linux内存管理](http://blog.chinaunix.net/uid/21718047/cid-151509-list-2.html) |
| [内存管理-之内核内存管理-基于linux3.10](http://blog.csdn.net/shichaog/article/details/45509917) |
| [内存管理(一)](http://www.cnblogs.com/openix/p/3334026.html) |
| [Linux内存管理原理](http://www.cnblogs.com/zhaoyl/p/3695517.html) |
| [第 15 章 内存映射和 DMA](http://www.embeddedlinux.org.cn/ldd3/ch15.html) |
| [ 内存管理（二）struct page ](http://blog.chinaunix.net/uid-30282771-id-5176971.html) |


#1	前景回顾
-------

前面我们讲到[服务器体系(SMP, NUMA, MPP)与共享存储器架构(UMA和NUMA)](http://blog.csdn.net/gatieme/article/details/52098615)

#1.1	UMA和NUMA两种模型
-------

共享存储型多处理机有两种模型

*	均匀存储器存取（Uniform-Memory-Access，简称UMA）模型
	
    将可用内存以连续方式组织起来,  
*	非均匀存储器存取（Nonuniform-Memory-Access，简称NUMA）模型

##1.2	UMA模型
-------

传统的多核运算是使用SMP(Symmetric Multi-Processor )模式：将多个处理器与一个集中的存储器和I/O总线相连。所有处理器只能访问同一个物理存储器，因此SMP系统有时也被称为一致存储器访问（UMA）结构体系，一致性意指无论在什么时候，处理器只能为内存的每个数据保持或共享唯一一个数值。



>物理存储器被所有处理机均匀共享。所有处理机对所有存储字具有相同的存取时间，这就是为什么称它为均匀存储器存取的原因。每台处理机可以有私用高速缓存,外围设备也以一定形式共享。

很显然，SMP的缺点是可伸缩性有限，因为在存储器和I/O接口达到饱和的时候，增加处理器并不能获得更高的性能，与之相对应的有AMP架构，不同核之间有主从关系，如一个核控制另外一个核的业务，可以理解为多核系统中控制平面和数据平面。

##1.3	NUMA模型
-------


NUMA模式是一种分布式存储器访问方式，处理器可以同时访问不同的存储器地址，大幅度提高并行性。 NUMA总是多处理器计算机,系统的哪个CPU都有本地内存, 可支持快速的访问, 各个处理器之前通过总线链接起来, 以支持堆其他CPU的本地内存的访问, 当然访问要比本地内存慢.


NUMA模式下，处理器被划分成多个"节点"（node）， 每个节点被分配有的本地存储器空间。 所有节点中的处理器都可以访问全部的系统物理存储器，但是访问本节点内的存储器所需要的时间，比访问某些远程节点内的存储器所花的时间要少得多。

>其访问时间随存储字的位置不同而变化。其共享存储器物理上是分布在所有处理机的本地存储器上。所有本地存储器的集合组成了全局地址空间，可被所有的处理机访问。处理机访问本地存储器是比较快的，但访问属于另一台处理机的远程存储器则比较慢，因为通过互连网络会产生附加时延。

NUMA 的主要优点是伸缩性。NUMA 体系结构在设计上已超越了 SMP 体系结构在伸缩性上的限制。通过 SMP，所有的内存访问都传递到相同的共享内存总线。这种方式非常适用于 CPU 数量相对较少的情况，但不适用于具有几十个甚至几百个 CPU 的情况，因为这些 CPU 会相互竞争对共享内存总线的访问。NUMA 通过限制任何一条内存总线上的 CPU 数量并依靠高速互连来连接各个节点，从而缓解了这些瓶颈状况。



#2	(N)UMA模型中linux内存的机构
-------


Linux适用于各种不同的体系结构, 而不同体系结构在内存管理方面的差别很大. 因此linux内核需要用一种体系结构无关的方式来表示内存.

Linux内核通过插入一些兼容层, 使得不同体系结构的差异很好的被隐藏起来, 内核对一致和非一致内存访问使用相同的数据结构


#2.1	(N)UMA模型中linux内存的机构
-------



非一致存储器访问(NUMA)模式下

*	处理器被划分成多个"节点"(node), 每个节点被分配有的本地存储器空间. 所有节点中的处理器都可以访问全部的系统物理存储器，但是访问本节点内的存储器所需要的时间，比访问某些远程节点内的存储器所花的时间要少得多


*	内存被分割成多个区域（BANK，也叫"簇"），依据簇与处理器的"距离"不同, 访问不同簇的代码也会不同. 比如，可能把内存的一个簇指派给每个处理器，或则某个簇和设备卡很近，很适合DMA，那么就指派给该设备。因此当前的多数系统会把内存系统分割成2块区域，一块是专门给CPU去访问，一块是给外围设备板卡的DMA去访问

>在UMA系统中, 内存就相当于一个只使用一个NUMA节点来管理整个系统的内存. 而内存管理的其他地方则认为他们就是在处理一个(伪)NUMA系统.



#2.2	Linux物理内存的组织形式
-------

Linux把物理内存划分为三个层次来管理

| 层次 | 描述 |
|:----:|:----:|
| 存储节点(Node) |  CPU被划分为多个节点(node), 内存则被分簇, 每个CPU对应一个本地物理内存, 即一个CPU-node对应一个内存簇bank，即每个内存簇被认为是一个节点 |
| 管理区(Zone)   | 每个物理内存节点node被划分为多个内存管理区域, 用于表示不同范围的内存, 内核可以使用不同的映射方式映射物理内存 |
| 页面(Page) 	   |	内存被细分为多个页面帧, 页面是最基本的页面分配的单位　｜

为了支持NUMA模型，也即CPU对不同内存单元的访问时间可能不同，此时系统的物理内存被划分为几个节点(node), 一个node对应一个内存簇bank，即每个内存簇被认为是一个节点

*	首先, 内存被划分为结点. 每个节点关联到系统中的一个处理器, 内核中表示为`pg_data_t`的实例. 系统中每个节点被链接到一个以NULL结尾的`pgdat_list`链表中<而其中的每个节点利用`pg_data_tnode_next`字段链接到下一节．而对于PC这种UMA结构的机器来说, 只使用了一个成为contig_page_data的静态pg_data_t结构.

*	接着各个节点又被划分为内存管理区域, 一个管理区域通过struct zone_struct描述, 其被定义为zone_t, 用以表示内存的某个范围, 低端范围的16MB被描述为ZONE_DMA, 某些工业标准体系结构中的(ISA)设备需要用到它, 然后是可直接映射到内核的普通内存域ZONE_NORMAL,最后是超出了内核段的物理地址域ZONE_HIGHMEM, 被称为高端内存.　是系统中预留的可用内存空间, 不能被内核直接映射.


在一个单独的节点内，任一给定CPU访问页面所需的时间都是相同的。然而，对不同的CPU，这个时间可能就不同。对每个CPU而言，内核都试图把耗时节点的访问次数减到最少这就要小心地选择CPU最常引用的内核数据结构的存放位置.

对于UMA体系的，系统中只有一个node

在LINUX中引入一个数据结构`struct pglist_data` ，来描述一个node，定义在[`include/linux/mmzone.h`](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L630) 文件中。（这个结构被typedef pg_data_t）。


*    对于NUMA系统来讲， 整个系统的内存由一个[node_data](http://lxr.free-electrons.com/source/arch/s390/numa/numa.c?v=4.7#L23)的pg_data_t指针数组来管理。(因为可能有多个node)，系统中的每个节点链接到一个以NULL结尾的[pgdat_list](http://lxr.free-electrons.com/source/arch/ia64/include/asm/numa.h#L27)链表中，而其中的每个节点利用pd_data_tnode_next字段链接到下一个节点。


*    对于PC这样的UMA系统，使用struct pglist_datacontig_page_data ，作为系统唯一的node管理所有的内存区域。（UMA系统中中只有一个node）




##内存节点node
-------

>CPU被划分为多个节点(node), 内存则被分簇, 每个CPU对应一个本地物理内存, 即一个CPU-node对应一个内存簇bank，即每个内存簇被认为是一个节点
>
>系统的物理内存被划分为几个节点(node), 一个node对应一个内存簇bank，即每个内存簇被认为是一个节点

*	首先, 内存被划分为结点. 每个节点关联到系统中的一个处理器, 内核中表示为`pg_data_t`的实例. 系统中每个节点被链接到一个以NULL结尾的`pgdat_list`链表中<而其中的每个节点利用`pg_data_tnode_next`字段链接到下一节．而对于PC这种UMA结构的机器来说, 只使用了一个成为contig_page_data的静态pg_data_t结构.


内存中的每个节点都是由pg_data_t描述,而pg_data_t由struct pglist_data定义而来, 该数据结构定义在[include/linux/mmzone.h, line 615](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L615)


在分配一个页面时, Linux采用节点局部分配的策略, 从最靠近运行中的CPU的节点分配内存, 由于进程往往是在同一个CPU上运行, 因此从当前节点得到的内存很可能被用到


在内存中，每个簇所对应的node又被分成的称为管理区（zone）的块，它们各自描述在内存中的范围。一个管理区（zone）由[struct zone](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L326)结构体来描述，在linux-2.4.37之前的内核中是用[`typedef  struct zone_struct zone_t `](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=2.4.37#L47)数据结构来描述）

管理区的类型有如下几种
*    ZONE_DMA
*    ZONE_NORMAL
*    ZONE_HIGHMEM这三种类型

不同的管理区的用途是不一样的，ZONE_DMA类型的内存区域在物理内存的低端，主要是ISA设备只能用低端的地址做DMA操作。ZONE_NORMAL类型的内存区域直接被内核映射到线性地址空间上面的区域（line address space），ZONE_HIGHMEM将保留给系统使用，是系统中预留的可用内存空间，不能被内核直接映射。

对于x86机器，管理区（内存区域）类型如下分布

| 类型 | 区域 |
| :------- | ----: |
| ZONE_DMA | 0~16MB |
| ZONE_NORMAL | 16MB~896MB |
| ZONE_HIGHMEM | 896MB~物理内存结束 |

##内存页page
-------

大多数内核（kernel）的操作只使用ZONE_NORMAL区域，系统内存由很多固定大小的内存块组成的，这样的内存块称作为“页”（PAGE），

x86体系结构中，page的大小为4096个字节。

每个物理的页由一个`struct page`的数据结构对象来描述。页的数据结构对象都保存在`mem_map`全局数组中，该数组通常被存放在ZONE_NORMAL的首部，或者就在小内存系统中为装入内核映像而预留的区域之后。从载入内核的低地址内存区域的后面内存区域，也就是ZONE_NORMAL开始的地方的内存的页的数据结构对象，都保存在这个全局数组中。



##高端内存
-------

由于能够被Linux内核直接访问的ZONE_NORMAL区域的内存空间也是有限的，所以LINUX提出了高端内存（High memory）的概念，并且允许对高端内存的访问
