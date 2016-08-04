服务器体系与共享存储器架构
=======

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6225543) |

http://www.cnblogs.com/plinx/archive/2013/01/15/2860520.html
http://blog.chinaunix.net/uid-30282771-id-5176971.html


#好的收藏
-------

##参照
-------

>参照
>
>[内存管理（一）内存模型之Node](http://biancheng.dnbcw.info/linux/387391.html)
>
> [Linux 内存管理 重要结构体](http://blog.chinaunix.net/uid-26009500-id-3078986.html)
>
>[Bootmem机制](http://blog.csdn.net/samssm/article/details/25064897)
>
>[Linux-2.6.32 NUMA架构之内存和调度](http://www.cnblogs.com/zhenjing/archive/2012/03/21/linux_numa.html)
>
>[Linux 用户空间与内核空间——高端内存详解](http://blog.csdn.net/tommy_wxie/article/details/17122923)
>
>[探索 Linux 内存模型](http://www.ibm.com/developerworks/cn/linux/l-memmod/)
>
>[Linux内存管理](http://blog.chinaunix.net/uid/21718047/cid-151509-list-2.html)
>
>[内存管理-之内核内存管理-基于linux3.10](http://blog.csdn.net/shichaog/article/details/45509917)
>
>[内存管理(一)](http://www.cnblogs.com/openix/p/3334026.html)
>
>[Linux内存管理原理](http://www.cnblogs.com/zhaoyl/p/3695517.html)
>
>[第 15 章 内存映射和 DMA](http://www.embeddedlinux.org.cn/ldd3/ch15.html)


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



#内存节点node
-------
>参照
>
>[内存管理（一）内存模型之Node](http://biancheng.dnbcw.info/linux/387391.html)
>
> [Linux 内存管理 重要结构体](http://blog.chinaunix.net/uid-26009500-id-3078986.html)
>
>[Bootmem机制](http://blog.csdn.net/samssm/article/details/25064897)
>
>[Linux-2.6.32 NUMA架构之内存和调度](http://www.cnblogs.com/zhenjing/archive/2012/03/21/linux_numa.html)


表示node的数据结构为[`typedef struct pglist_data pg_data_t`](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L630)， 这个结构定义在[include/linux/mmzone.h, line 615](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L615)中,结构体的内容如下

```c
```c
/*
 * The pg_data_t structure is used in machines with CONFIG_DISCONTIGMEM
 * (mostly NUMA machines?) to denote a higher-level memory zone than the
 * zone denotes.
 *
 * On NUMA machines, each NUMA node would have a pg_data_t to describe
 * it's memory layout.
 *
 * Memory statistics and page replacement data structures are maintained on a
 * per-zone basis.
 */
struct bootmem_data;
typedef struct pglist_data {
    struct zone node_zones[MAX_NR_ZONES];
    struct zonelist node_zonelists[MAX_ZONELISTS];
    int nr_zones;
#ifdef CONFIG_FLAT_NODE_MEM_MAP /* means !SPARSEMEM */
    struct page *node_mem_map;
#ifdef CONFIG_PAGE_EXTENSION
    struct page_ext *node_page_ext;
#endif
#endif
#ifndef CONFIG_NO_BOOTMEM
    struct bootmem_data *bdata;
#endif
#ifdef CONFIG_MEMORY_HOTPLUG
    /*
     * Must be held any time you expect node_start_pfn, node_present_pages
     * or node_spanned_pages stay constant.  Holding this will also
     * guarantee that any pfn_valid() stays that way.
     *
     * pgdat_resize_lock() and pgdat_resize_unlock() are provided to
     * manipulate node_size_lock without checking for CONFIG_MEMORY_HOTPLUG.
     *
     * Nests above zone->lock and zone->span_seqlock
     */
    spinlock_t node_size_lock;
#endif
    unsigned long node_start_pfn;
    unsigned long node_present_pages; /* total number of physical pages */
    unsigned long node_spanned_pages; /* total size of physical page
                         range, including holes */
    int node_id;
    wait_queue_head_t kswapd_wait;
    wait_queue_head_t pfmemalloc_wait;
    struct task_struct *kswapd;     /* Protected by
                       mem_hotplug_begin/end() */
    int kswapd_max_order;
    enum zone_type classzone_idx;
#ifdef CONFIG_COMPACTION
    int kcompactd_max_order;
    enum zone_type kcompactd_classzone_idx;
    wait_queue_head_t kcompactd_wait;
    struct task_struct *kcompactd;
#endif
#ifdef CONFIG_NUMA_BALANCING
    /* Lock serializing the migrate rate limiting window */
    spinlock_t numabalancing_migrate_lock;

    /* Rate limiting time interval */
    unsigned long numabalancing_migrate_next_window;

    /* Number of pages migrated during the rate limiting time interval */
    unsigned long numabalancing_migrate_nr_pages;
#endif

#ifdef CONFIG_DEFERRED_STRUCT_PAGE_INIT
    /*
     * If memory initialisation on large machines is deferred then this
     * is the first PFN that needs to be initialised.
     */
    unsigned long first_deferred_pfn;
#endif /* CONFIG_DEFERRED_STRUCT_PAGE_INIT */

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
    spinlock_t split_queue_lock;
    struct list_head split_queue;
    unsigned long split_queue_len;
#endif
} pg_data_t;
```

| 字段| 描述 |
| :------- | ----: |
|node_zones | 每个Node划分为不同的zone，分别为ZONE_DMA，ZONE_NORMAL，ZONE_HIGHMEM |
|node_zonelists | 这个是备用节点及其内存域的列表，当当前节点的内存不够分配时，会选取访问代价最低的内存进行分配。分配内存操作时的区域顺序，当调用free_area_init_core()时，由mm/page_alloc.c文件中的build_zonelists()函数设置 |
|nr_zones | 当前节点中不同内存域zone的数量，1到3个之间。并不是所有的node都有3个zone的，比如一个CPU簇就可能没有ZONE_DMA区域 |
| node_mem_map | node中的第一个page，它可以指向mem_map中的任何一个page，指向page实例数组的指针，用于描述该节点所拥有的的物理内存页，它包含了该页面所有的内存页，被放置在全局mem_map数组中  |
| bdata | 这个仅用于引导程序boot 的内存分配，内存在启动时，也需要使用内存，在这里内存使用了自举内存分配器，这里bdata是指向内存自举分配器的数据结构的实例 |
| node_start_pfn | pfn是page frame number的缩写。这个成员是用于表示node中的开始那个page在物理内存中的位置的。是当前NUMA节点的第一个页帧的编号，系统中所有的页帧是依次进行编号的，这个字段代表的是当前节点的页帧的起始值，对于UMA系统，只有一个节点，所以该值总是0 |
|node_present_pages | node中的真正可以使用的page数量 |
|node_spanned_pages |  该节点以页帧为单位的总长度，这个不等于前面的node_present_pages,因为这里面包含空洞内存 |
|node_id | node的NODE ID 当前节点在系统中的编号，从0开始 |
| kswapd_wait | node的等待队列，交换守护列队进程的等待列表|
| kswapd_max_order | 需要释放的区域的长度，以页阶为单位 |
| classzone_idx | 这个字段暂时没弄明白，不过其中的zone_type是对ZONE_DMA,ZONE_DMA32,ZONE_NORMAL,ZONE_HIGH,ZONE_MOVABLE,__MAX_NR_ZONES的枚举 |


在新的linux3.x~linux4.x的内核中，Linux定义了一个大小为[MAX_NUMNODES](http://lxr.free-electrons.com/source/include/linux/numa.h#L11)类型为[`pgdat_list`](http://lxr.free-electrons.com/source/arch/ia64/mm/discontig.c#L50)数组，数组的大小根据[CONFIG_NODES_SHIFT](http://lxr.free-electrons.com/source/include/linux/numa.h#L6)的配置决定。对于UMA来说，NODES_SHIFT为0，所以MAX_NUMNODES的值为1。内核提供了[for_each_online_pgdat(pgdat)](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=4.7#L871)来遍历节点

而在linux-2.4.x之前的内核中所有的节点，都由一个被称为[pgdat_list](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=2.4.37#L169)的链表维护。这些节点都放在该链表中，均由函数[init_bootmem_core()](http://lxr.free-electrons.com/source/mm/bootmem.c#L96)初始化结点。内核提供了[宏for_each_pgdat(pgdat)]http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=2.4.37#L169)来遍历节点链表。

对于单一node的系统，contig_page_data 是系统唯一的node数据结构对象。查看contig_page_data的定义[linux-4.5](http://lxr.free-electrons.com/source/mm/bootmem.c#L27)，[linux-2.4.37](http://lxr.free-electrons.com/source/mm/numa.c?v=2.4.37#L15)

#管理区Zone
-------


linux系统中，内存中的每个簇所对应的node又被分成的称为管理区（zone）的块，

>一个管理区（zone）由[struct zone](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L326)结构体来描述(linux-3.8~目前linux4.5)，而在linux-2.4.37之前的内核中是用[`typedef  struct zone_struct zone_t `](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=2.4.37#L47)数据结构来描述)

zone对象用于跟踪诸如页面使用情况的统计数，空闲区域信息和锁信息

>里面保存着内存使用状态信息，如page使用统计，未使用的内存区域，互斥访问的锁（LOCKS）等。

`struct zone`在`linux/mmzone.h`中定义，在linux-4.7的内核中可以使用[include/linux/mmzone.h](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=4.7#L324)来查看其定义

```cpp

```

| 字段| 描述 |
| :------- | ----: |
| free_pages | 未分配使用的page的数量 |

| lowmem_reserve[MAX_NR_ZONES] | 为了防止一些代码必须运行在低地址区域，所以事先保留一些低地址区域的内存 |
| pageset | page管理的数据结构对象，内部有一个page的列表(list)来管理。每个CPU维护一个page list，避免自旋锁的冲突。这个数组的大小和NR_CPUS(CPU的数量）有关，这个值是编译的时候确定的 |
| lock | 对zone并发访问的保护的自旋锁 |
| free_area[MAX_ORDER] | 页面使用状态的信息，以每个bit标识对应的page是否可以分配 |
| lru_lock | LRU(最近最少使用算法)的自旋锁 |
| reclaim_in_progress | 回收操作的原子锁 |
| active_list | 活跃的page的list |
| inactive_list | 不活跃的page的list |
| refill_counter | 从活跃的page list中移除的page的数量
| nr_active | 活跃的page的数量 |
| nr_inactive | 不活跃的page的数量 |
| pressure | 检查回收page的指标 |
| all_unreclaimable | 如果检测2次还是不能回收zone的page的话，则设置为1 |
| pages_scanned | 上次回收page后，扫描过的page的数量 |
| wait_table：等待一个page释放的等待队列哈希表。它会被| wait_on_page()，unlock_page()函数使用. 用哈希表，而不用一个等待队列的原因，防止进程长期等待资源。
| wait_table_hash_nr_entries | 哈希表中的等待队列的数量
| zone_pgdat | 指向这个zone所在的pglist_data对象 |
| zone_start_pfn | 和node_start_pfn的含义一样。这个成员是用于表示zone中的开始那个page在物理内存中的位置的present_pages， spanned_pages: 和node中的类似的成员含义一样 |
| name | zone的名字，字符串表示： "DMA"，"Normal" 和"HighMem" |
| ZONE_PADDING | 由于自旋锁频繁的被使用，因此为了性能上的考虑，将某些成员对齐到cache line中，有助于提高执行的性能。使用这个宏，可以确定zone->lock，zone->lru_lock，zone->pageset这些成员使用不同的cache line. |

Zone的管理调度的一些参数： （Zone watermarks)，
英文直译为zone的水平，打个比喻，就像一个水库，水存量很小的时候加大进水量，水存量达到一个标准的时候，减小进水量，当快要满的时候，可能就关闭了进水口。

pages_min， pages_low and pages_high就类似与这个标准

当系统中可用内存很少的时候，系统代码kswapd被唤醒，开始回收释放page

pages_min， pages_low and pages_high这些参数影响着这个代码的行为。

每个zone有三个水`平标准：pages_min， pages_low and pages_high，帮助确定zone中内存分配使用的压力状态。kswapd和这3个参数的互动关系如下图：

page_min中所表示的page的数量值，是在内存初始化的过程中调用free_area_init_core()中计算的。这个数值是根据zone中的page的数量除以一个>1的系数来确定的。通常是这样初始化的ZoneSizeInPages/128。
page_low: 当空闲页面的数量达到page_low所标定的数量的时候，kswapd线程将被唤醒，并开始释放回收页面。这个值默认是page_min的2倍。
page_min: 当空闲页面的数量达到page_min所标定的数量的时候， 分配页面的动作和kswapd线程同步运行
page_high: 当空闲页面的数量达到page_high所标定的数量的时候， kswapd线程将重新休眠，通常这个数值是page_min的3倍。
zone的大小的计算
setup_memory()函数计算每个zone的大小：

PFN是物理内存以Page为单位的偏移量。系统可用的第一个PFN是min_low_pfn变量，开始与_end标号的后面，也就是kernel结束的地方。在文件mm/bootmem.c中对这个变量作初始化。系统可用的最后一个PFN是max_pfn变量，这个变量的初始化完全依赖与硬件的体系结构。x86的系统中，find_max_pfn()函数通过读取e820表获得最高的page frame的数值。同样在文件mm/bootmem.c中对这个变量作初始化。e820表是由BIOS创建的。
x86中，max_low_pfn变量是由find_max_low_pfn()函数计算并且初始化的，它被初始化成ZONE_NORMAL的最后一个page的位置。这个位置是kernel直接访问的物理内存，也是关系到kernel/userspace通过“PAGE_OFFSET宏”把线性地址内存空间分开的内存地址位置。（原文：This is the physical memory directly accessible by the kernel and is related to the kernel/userspace split in the linear address space marked by PAGE OFFSET.）我理解为这段地址kernel可以直接访问，可以通过PAGE_OFFSET宏直接将kernel所用的虚拟地址转换成物理地址的区段。在文件mm/bootmem.c中对这个变量作初始化。在内存比较小的系统中max_pfn和max_low_pfn的值相同
min_low_pfn， max_pfn和max_low_pfn这3个值，也要用于对高端内存（high memory)的起止位置的计算。在arch/i386/mm/init.c文件中会对类似的highstart_pfn和highend_pfn变量作初始化。这些变量用于对高端内存页面的分配。后面将描述。
Zone等待队列表（zone wait queue table)
当对一个page做I/O操作的时候，I/O操作需要被锁住，防止不正确的数据被访问。进程在访问page前，调用wait_on_page()函数，使进程加入一个等待队列。访问完成后，UnlockPage()函数解锁其他进程对page的访问。其他正在等待队列中的进程被唤醒。每个page都可以有一个等待队列，但是太多的分离的等待队列使得花费太多的内存访问周期。替代的解决方法，就是将所有的队列放在struct zone数据结构中。
也可以有一种可能，就是struct zone中只有一个队列，但是这就意味着，当一个page unlock的时候，访问这个zone里内存page的所有休眠的进程将都被唤醒，这样就会出现拥堵（thundering herd）的问题。建立一个哈希表管理多个等待队列，能解决这个问题，zone->wait_table就是这个哈希表。哈希表的方法可能还是会造成一些进程不必要的唤醒。但是这种事情发生的机率不是很频繁的。下面这个图就是进程及等待队列的运行关系：

等待队列的哈希表的分配和建立在free_area_init_core()函数中进行。哈希表的表项的数量在wait_table_size() 函数中计算，并且保持在zone->wait_table_size成员中。最大4096个等待队列。最小是NoPages / PAGES_PER_WAITQUEUE的2次方，NoPages是zone管理的page的数量，PAGES_PER_WAITQUEUE被定义256。（原文：For smaller tables， the size of the table is the minimum power of 2 required to store NoPages / PAGES PER WAITQUEUE number of queues， where NoPages is the number of pages in the zone and PAGE PER WAITQUEUE is defined to be 256.）
下面这个公式可以用于计算这个值：

zone->wait_table_bits用于计算：根据page 地址得到需要使用的等待队列在哈希表中的索引的算法因子。page_waitqueue()函数负责返回zone中page所对应等待队列。它用一个基于struct page虚拟地址的简单的乘法哈希算法来确定等待队列的。
page_waitqueue()函数用GOLDEN_RATIO_PRIME的地址和“右移zone→wait_table_bits一个索引值”的一个乘积来确定等待队列在哈希表中的索引的。
Zone的初始化
在kernel page table通过paging_init()函数完全建立起z来以后，zone被初始化。下面章节将描述这个。当然不同的体系结构这个过程肯定也是不一样的，但它们的目的却是相同的：确定什么参数需要传递给free_area_init()函数（对于UMA体系结构）或者free_area_init_node()函数（对于NUMA体系结构）。这里省略掉NUMA体系结构的说明。
free_area_init()函数的参数：
unsigned long *zones_sizes: 系统中每个zone所管理的page的数量的数组。这个时候，还没能确定zone中那些page是可以分配使用的（free）。这个信息知道boot memory allocator完成之前还无法知道。
来源： http://www.uml.org.cn/embeded/201208071.asp



#页面page
-------


#页表
-------
