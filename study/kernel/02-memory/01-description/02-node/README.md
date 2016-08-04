服务器体系与共享存储器架构
=======

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6225543) |



#1	前景回顾
-------


前面我们讲到[服务器体系(SMP, NUMA, MPP)与共享存储器架构(UMA和NUMA)](http://blog.csdn.net/gatieme/article/details/52098615)


#1.1	UMA和NUMA两种模型
-------


共享存储型多处理机有两种模型

*	均匀存储器存取（Uniform-Memory-Access，简称UMA）模型


*	非均匀存储器存取（Nonuniform-Memory-Access，简称NUMA）模型


**UMA模型**

物理存储器被所有处理机均匀共享。所有处理机对所有存储字具有相同的存取时间，这就是为什么称它为均匀存储器存取的原因。每台处理机可以有私用高速缓存,外围设备也以一定形式共享。

**NUMA模型**

NUMA模式下，处理器被划分成多个"节点"（node）， 每个节点被分配有的本地存储器空间。 所有节点中的处理器都可以访问全部的系统物理存储器，但是访问本节点内的存储器所需要的时间，比访问某些远程节点内的存储器所花的时间要少得多。


#1.2	(N)UMA模型中linux内存的机构
-------



非一致存储器访问(NUMA)模式下

*	处理器被划分成多个"节点"(node), 每个节点被分配有的本地存储器空间. 所有节点中的处理器都可以访问全部的系统物理存储器，但是访问本节点内的存储器所需要的时间，比访问某些远程节点内的存储器所花的时间要少得多


*	内存被分割成多个区域（BANK，也叫"簇"），依据簇与处理器的"距离"不同, 访问不同簇的代码也会不同. 比如，可能把内存的一个簇指派给每个处理器，或则某个簇和设备卡很近，很适合DMA，那么就指派给该设备。因此当前的多数系统会把内存系统分割成2块区域，一块是专门给CPU去访问，一块是给外围设备板卡的DMA去访问

>在UMA系统中, 内存就相当于一个只使用一个NUMA节点来管理整个系统的内存. 而内存管理的其他地方则认为他们就是在处理一个(伪)NUMA系统.


Linux把物理内存划分为三个层次来管理

| 层次 | 描述 |
|:----:|:----:|
| 存储节点(Node) |  CPU被划分为多个节点(node), 内存则被分簇, 每个CPU对应一个本地物理内存, 即一个CPU-node对应一个内存簇bank，即每个内存簇被认为是一个节点 |
| 管理区(Zone)   | 每个物理内存节点node被划分为多个内存管理区域, 用于表示不同范围的内存, 内核可以使用不同的映射方式映射物理内存 |
| 页面(Page) 	   |	内存被细分为多个页面帧, 页面是最基本的页面分配的单位　｜





#2	内存节点node
-------


##2.1   为什么要用node来描述内存
-------


这点前面是说的很明白了, NUMA结构下, 每个处理器CPU与一个本地内存直接相连, 而不同处理器之前则通过总线进行进一步的连接, 因此相对于任何一个CPU访问本地内存的速度比访问远程内存的速度要快

Linux适用于各种不同的体系结构, 而不同体系结构在内存管理方面的差别很大. 因此linux内核需要用一种体系结构无关的方式来表示内存.

因此linux内核把物理内存按照CPU节点划分为不同的node, 每个node作为某个cpu结点的本地内存, 而作为其他CPU节点的远程内存, 而UMA结构下, 则任务系统中只存在一个内存node, 这样对于UMA结构来说, 内核把内存当成只有一个内存node节点的伪NUMA


##2.2	内存结点的概念
-------

>CPU被划分为多个节点(node), 内存则被分簇, 每个CPU对应一个本地物理内存, 即一个CPU-node对应一个内存簇bank，即每个内存簇被认为是一个节点
>
>系统的物理内存被划分为几个节点(node), 一个node对应一个内存簇bank，即每个内存簇被认为是一个节点

内存被划分为结点. 每个节点关联到系统中的一个处理器, 内核中表示为`pg_data_t`的实例. 系统中每个节点被链接到一个以NULL结尾的`pgdat_list`链表中<而其中的每个节点利用`pg_data_tnode_next`字段链接到下一节．而对于PC这种UMA结构的机器来说, 只使用了一个成为contig_page_data的静态pg_data_t结构.


内存中的每个节点都是由pg_data_t描述,而pg_data_t由struct pglist_data定义而来, 该数据结构定义在[include/linux/mmzone.h, line 615](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L615)


在分配一个页面时, Linux采用节点局部分配的策略, 从最靠近运行中的CPU的节点分配内存, 由于进程往往是在同一个CPU上运行, 因此从当前节点得到的内存很可能被用到




##2.3	pg_data_t描述内存节点
-------

表示node的数据结构为[`typedef struct pglist_data pg_data_t`](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L630)， 这个结构定义在[include/linux/mmzone.h, line 615](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L615)中,结构体的内容如下

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
	/*  包含了结点中各内存域的数据结构 , 可能的区域类型用zone_type表示*/
    struct zone node_zones[MAX_NR_ZONES];
    /*  指点了备用结点及其内存域的列表，以便在当前结点没有可用空间时，在备用结点分配内存   */
    struct zonelist node_zonelists[MAX_ZONELISTS];
    int nr_zones;									/*  保存结点中不同内存域的数目    */
#ifdef CONFIG_FLAT_NODE_MEM_MAP /* means !SPARSEMEM */
    struct page *node_mem_map;		/*  指向page实例数组的指针，用于描述结点的所有物理内存页，它包含了结点中所有内存域的页。    */
#ifdef CONFIG_PAGE_EXTENSION
    struct page_ext *node_page_ext;
#endif
#endif
#ifndef CONFIG_NO_BOOTMEM
       /*  在系统启动boot期间，内存管理子系统初始化之前，
       内核页需要使用内存（另外，还需要保留部分内存用于初始化内存管理子系统）
       为解决这个问题，内核使用了自举内存分配器 
       此结构用于这个阶段的内存管理  */
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
	 * 当系统支持内存热插拨时，用于保护本结构中的与节点大小相关的字段。
     * 哪调用node_start_pfn，node_present_pages，node_spanned_pages相关的代码时，需要使用该锁。
     */
    spinlock_t node_size_lock;
#endif
	/* /*起始页面帧号，指出该节点在全局mem_map中的偏移
    系统中所有的页帧是依次编号的，每个页帧的号码都是全局唯一的（不只是结点内唯一）  */
    unsigned long node_start_pfn;
    unsigned long node_present_pages; /* total number of physical pages 结点中页帧的数目 */
    unsigned long node_spanned_pages; /* total size of physical page range, including holes  					该结点以页帧为单位计算的长度，包含内存空洞 */
    int node_id;		/*  全局结点ID，系统中的NUMA结点都从0开始编号  */
    wait_queue_head_t kswapd_wait;		/*  交换守护进程的等待队列，
    在将页帧换出结点时会用到。后面的文章会详细讨论。    */
    wait_queue_head_t pfmemalloc_wait;
    struct task_struct *kswapd;     /* Protected by  mem_hotplug_begin/end() 指向负责该结点的交换守护进程的task_struct。   */
    int kswapd_max_order;						/*  定义需要释放的区域的长度  */
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

>而在linux-2.4.x之前的内核中所有的节点，都由一个被称为[pgdat_list](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=2.4.37#L169)的链表维护。这些节点都放在该链表中，均由函数[init_bootmem_core()](http://lxr.free-electrons.com/source/mm/bootmem.c#L96)初始化结点。内核提供了[宏for_each_pgdat(pgdat)]http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=2.4.37#L169)来遍历节点链表。

对于单一node的系统，contig_page_data 是系统唯一的node数据结构对象。查看contig_page_data的定义[linux-4.5](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L27)，[linux-2.4.37](http://lxr.free-electrons.com/source/mm/numa.c?v=2.4.37#L15)


