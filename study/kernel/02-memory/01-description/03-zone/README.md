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


##1.3	Linux如何描述物理内存
-------

Linux把物理内存划分为三个层次来管理

| 层次 | 描述 |
|:----:|:----:|
| 存储节点(Node) |  CPU被划分为多个节点(node), 内存则被分簇, 每个CPU对应一个本地物理内存, 即一个CPU-node对应一个内存簇bank，即每个内存簇被认为是一个节点 |
| 管理区(Zone)   | 每个物理内存节点node被划分为多个内存管理区域, 用于表示不同范围的内存, 内核可以使用不同的映射方式映射物理内存 |
| 页面(Page) 	   |	内存被细分为多个页面帧, 页面是最基本的页面分配的单位　｜


##1.4	用pd_data_t描述内存节点node
-------


>CPU被划分为多个节点(node), 内存则被分簇, 每个CPU对应一个本地物理内存, 即一个CPU-node对应一个内存簇bank，即每个内存簇被认为是一个节点
>
>系统的物理内存被划分为几个节点(node), 一个node对应一个内存簇bank，即每个内存簇被认为是一个节点

*	首先, 内存被划分为结点. 每个节点关联到系统中的一个处理器, 内核中表示为`pg_data_t`的实例. 系统中每个节点被链接到一个以NULL结尾的`pgdat_list`链表中<而其中的每个节点利用`pg_data_tnode_next`字段链接到下一节．而对于PC这种UMA结构的机器来说, 只使用了一个成为contig_page_data的静态pg_data_t结构.


内存中的每个节点都是由pg_data_t描述,而pg_data_t由struct pglist_data定义而来, 该数据结构定义在[include/linux/mmzone.h, line 615](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L615)


在分配一个页面时, Linux采用节点局部分配的策略, 从最靠近运行中的CPU的节点分配内存, 由于进程往往是在同一个CPU上运行, 因此从当前节点得到的内存很可能被用到


##1.5	今日内容(内存管理域zone)
-------

为了支持NUMA模型，也即CPU对不同内存单元的访问时间可能不同，此时系统的物理内存被划分为几个节点(node), 一个node对应一个内存簇bank，即每个内存簇被认为是一个节点

*	首先, 内存被划分为结点. 每个节点关联到系统中的一个处理器, 内核中表示为`pg_data_t`的实例. 系统中每个节点被链接到一个以NULL结尾的`pgdat_list`链表中<而其中的每个节点利用`pg_data_tnode_next`字段链接到下一节．而对于PC这种UMA结构的机器来说, 只使用了一个成为contig_page_data的静态pg_data_t结构.

*	接着各个节点又被划分为内存管理区域, 一个管理区域通过struct zone_struct描述, 其被定义为zone_t, 用以表示内存的某个范围, 低端范围的16MB被描述为ZONE_DMA, 某些工业标准体系结构中的(ISA)设备需要用到它, 然后是可直接映射到内核的普通内存域ZONE_NORMAL,最后是超出了内核段的物理地址域ZONE_HIGHMEM, 被称为高端内存.　是系统中预留的可用内存空间, 不能被内核直接映射.


下面我们就来详解讲讲内存管理域的内容zone


#2	为什么要将内存node分成不同的区域zone
-------


NUMA结构下, 每个处理器CPU与一个本地内存直接相连, 而不同处理器之前则通过总线进行进一步的连接, 因此相对于任何一个CPU访问本地内存的速度比访问远程内存的速度要快, 而Linux为了兼容NUMAJ结构, 把物理内存相依照CPU的不同node分成簇, 一个CPU-node对应一个本地内存pgdata_t.



这样已经很好的表示物理内存了, 在一个理想的计算机系统中, 一个页框就是一个内存的分配单元, 可用于任何事情:存放内核数据, 用户数据和缓冲磁盘数据等等. 任何种类的数据页都可以存放在任页框中, 没有任何限制.

<font color=0x00ffff>
但是Linux内核又把各个物理内存节点分成个不同的管理区域zone, 这是为什么呢?
</font>

因为实际的计算机体系结构有硬件的诸多限制, 这限制了页框可以使用的方式. 尤其是, Linux内核必须处理80x86体系结构的两种硬件约束.

*	ISA总线的直接内存存储DMA处理器有一个严格的限制 : 他们只能对RAM的前16MB进行寻址

*	在具有大容量RAM的现代32位计算机中, CPU不能直接访问所有的物理地址, 因为线性地址空间太小, 内核不可能直接映射所有物理内存到线性地址空间, 我们会在后面典型架构(x86)上内存区域划分详细讲解x86_32上的内存区域划分
</font>

因此Linux内核对不同区域的内存需要采用不同的管理方式和映射方式, 因此内核将物理地址或者成用zone_t表示的不同地址区域



#3	内存管理区类型zone_type
-------

前面我们说了由于硬件的一些约束, 低端的一些地址被用于DMA, 而在实际内存大小超过了内核所能使用的现行地址的时候, 一些高地址处的物理地址不能简单持久的直接映射到内核空间. 因此内核将内存的节点node分成了不同的内存区域方便管理和映射.

Linux使用enum zone_type来标记内核所支持的所有内存区域


##3.1	内存区域类型zone_type
-------

zone_type结构定义在[include/linux/mmzone.h](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=4.7#L267), 其基本信息如下所示

```c
enum zone_type
{
#ifdef CONFIG_ZONE_DMA
    ZONE_DMA,
#endif

#ifdef CONFIG_ZONE_DMA32

    ZONE_DMA32,
#endif

    ZONE_NORMAL,

#ifdef CONFIG_HIGHMEM
    ZONE_HIGHMEM,
#endif
    ZONE_MOVABLE,
#ifdef CONFIG_ZONE_DEVICE
    ZONE_DEVICE,
#endif
    __MAX_NR_ZONES

};

```
不同的管理区的用途是不一样的，ZONE_DMA类型的内存区域在物理内存的低端，主要是ISA设备只能用低端的地址做DMA操作。ZONE_NORMAL类型的内存区域直接被内核映射到线性地址空间上面的区域（line address space），ZONE_HIGHMEM将保留给系统使用，是系统中预留的可用内存空间，不能被内核直接映射。


##3.2	不同的内存区域的作用
-------

在内存中，每个簇所对应的node又被分成的称为管理区(zone)的块，它们各自描述在内存中的范围。一个管理区(zone)由[struct zone](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L326)结构体来描述，在linux-2.4.37之前的内核中是用[`typedef  struct zone_struct zone_t `](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=2.4.37#L47)数据结构来描述）

管理区的类型用zone_type表示, 有如下几种

| 管理内存域 | 描述 |
|:---------:|:---:|
| ZONE_DMA | 标记了适合DMA的内存域. 该区域的长度依赖于处理器类型. 这是由于古老的ISA设备强加的边界. 但是为了兼容性, 现代的计算机也可能受此影响 |
| ZONE_DMA32 | 标记了使用32位地址字可寻址, 适合DMA的内存域. 显然, 只有在53位系统中ZONE_DMA32才和ZONE_DMA有区别, 在32位系统中, 本区域是空的, 即长度为0MB, 在Alpha和AMD64系统上, 该内存的长度可能是从0到4GB |
| ZONE_NORMAL | 标记了可直接映射到内存段的普通内存域. 这是在所有体系结构上保证会存在的唯一内存区域, 但无法保证该地址范围对应了实际的物理地址. 例如, 如果AMD64系统只有两2G内存, 那么所有的内存都属于ZONE_DMA32范围, 而ZONE_NORMAL则为空 |
| ZONE_HIGHMEM | 标记了超出内核虚拟地址空间的物理内存段, 因此这段地址不能被内核直接映射 |
| ZONE_MOVABLE | 内核定义了一个伪内存域ZONE_MOVABLE, 在防止物理内存碎片的机制memory migration中需要使用该内存域. 供防止物理内存碎片的极致使用 |
| ZONE_DEVICE | 为支持热插拔设备而分配的Non Volatile Memory非易失性内存 |
| MAX_NR_ZONES | 充当结束标记, 在内核中想要迭代系统中所有内存域, 会用到该常亮 |

根据编译时候的配置, 可能无需考虑某些内存域. 例如在64位系统中, 并不需要高端内存, 因为AM64的linux采用4级页表，支持的最大物理内存为64TB, 对于虚拟地址空间的划分，将0x0000,0000,0000,0000 – 0x0000,7fff,ffff,f000这128T地址用于用户空间；而0xffff,8000,0000,0000以上的128T为系统空间地址, 这远大于当前我们系统中的内存空间, 因此所有的物理地址都可以直接映射到内核中, 不需要高端内存的特殊映射. 可以参见[Documentation/x86/x86_64/mm.txt](https://www.kernel.org/doc/Documentation/x86/x86_64/mm.txt)
	`


ZONE_MOVABLE和ZONE_DEVICE其实是和其他的ZONE的用途有异,

*	ZONE_MOVABLE在防止物理内存碎片的机制中需要使用该内存区域,

*	ZONE_DEVICE笔者也第一次知道了，理解有错的话欢迎大家批评指正, 这个应该是为支持热插拔设备而分配的Non Volatile Memory非易失性内存, 


>关于ZONE_DEVICE, 具体的信息可以参见[ATCH v2 3/9] mm: ZONE_DEVICE for "device memory"](https://lkml.org/lkml/2015/8/25/844)
>
>While pmem is usable as a block device or via DAX mappings to userspace
there are several usage scenarios that can not target pmem due to its
lack of struct page coverage. In preparation for "hot plugging" pmem
into the vmemmap add ZONE_DEVICE as a new zone to tag these pages
separately from the ones that are subject to standard page allocations.
Importantly "device memory" can be removed at will by userspace
unbinding the driver of the device.


##3.3	典型架构(x86)上内存区域划分
-------


对于x86机器，管理区(内存区域)类型如下分布

| 类型 | 区域 |
| :------- | ----: |
| ZONE_DMA | 0~15MB |
| ZONE_NORMAL | 16MB~895MB |
| ZONE_HIGHMEM | 896MB~物理内存结束 |

而由于32位系统中, Linux内核虚拟地址空间只有1G, 而0~895M这个986MB被用于DMA和直接映射, 剩余的物理内存被成为高端内存. 那内核是如何借助剩余128MB高端内存地址空间是如何实现访问可以所有物理内存？

当内核想访问高于896MB物理地址内存时，从0xF8000000 ~ 0xFFFFFFFF地址空间范围内找一段相应大小空闲的逻辑地址空间，借用一会。借用这段逻辑地址空间，建立映射到想访问的那段物理内存（即填充内核PTE页面表），临时用一会，用完后归还。这样别人也可以借用这段地址空间访问其他物理内存，实现了使用有限的地址空间，访问所有所有物理内存

>关于高端内存的内容, 我们后面会专门抽出一章进行讲解

因此, 传统和X86_32位系统中, 前16M划分给ZONE_DMA, 该区域包含的页框可以由老式的基于ISAS的设备通过DMA使用"直接内存访问(DMA)", ZONE_DMA和ZONE_NORMAL区域包含了内存的常规页框, 通过把他们线性的映射到现行地址的第4个GB, 内核就可以直接进行访问, 相反ZONE_HIGHME包含的内存页不能由内核直接访问, 尽管他们也线性地映射到了现行地址空间的第4个GB. 在64位体系结构中, 线性地址空间的大小远远好过了系统的实际物理地址, 内核可知直接将所有的物理内存映射到线性地址空间, 因此64位体系结构上ZONE_HIGHMEM区域总是空的.


#4	管理区结构zone_t
-------

>一个管理区(zone)由[`struct zone`](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L326)结构体来描述(linux-3.8~目前linux4.5)，而在linux-2.4.37之前的内核中是用[`struct zone_struct `](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=2.4.37#L47)数据结构来描述), 他们都通过typedef被重定义为zone_t类型

zone对象用于跟踪诸如页面使用情况的统计数, 空闲区域信息和锁信息

>里面保存着内存使用状态信息，如page使用统计, 未使用的内存区域，互斥访问的锁（LOCKS）等.


##4.1	struct zone管理域数据结构
-------

`struct zone`在`linux/mmzone.h`中定义, 在linux-4.7的内核中可以使用[include/linux/mmzone.h](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=4.7#L324)来查看其定义

```cpp
struct zone
{
    /* Read-mostly fields */

    /* zone watermarks, access with *_wmark_pages(zone) macros */
    unsigned long watermark[NR_WMARK];

    unsigned long nr_reserved_highatomic;

    /*
     * We don't know if the memory that we're going to allocate will be
     * freeable or/and it will be released eventually, so to avoid totally
     * wasting several GB of ram we must reserve some of the lower zone
     * memory (otherwise we risk to run OOM on the lower zones despite
     * there being tons of freeable ram on the higher zones).  This array is
     * recalculated at runtime if the sysctl_lowmem_reserve_ratio sysctl
     * changes.
     * 分别为各种内存域指定了若干页
     * 用于一些无论如何都不能失败的关键性内存分配。
     */
    long lowmem_reserve[MAX_NR_ZONES];

#ifdef CONFIG_NUMA
    int node;
#endif

    /*
     * The target ratio of ACTIVE_ANON to INACTIVE_ANON pages on
     * this zone's LRU.  Maintained by the pageout code.
     * 不活动页的比例,
     * 接着是一些很少使用或者大部分情况下是只读的字段：
     * wait_table wait_table_hash_nr_entries wait_table_bits
     * 形成等待列队，可以等待某一页可供进程使用  */
    unsigned int inactive_ratio;

    /*  指向这个zone所在的pglist_data对象  */
    struct pglist_data      *zone_pgdat;
    /*/这个数组用于实现每个CPU的热/冷页帧列表。内核使用这些列表来保存可用于满足实现的“新鲜”页。但冷热页帧对应的高速缓存状态不同：有些页帧很可能在高速缓存中，因此可以快速访问，故称之为热的；未缓存的页帧与此相对，称之为冷的。*/
    struct per_cpu_pageset __percpu *pageset;

    /*
     * This is a per-zone reserve of pages that are not available
     * to userspace allocations.
     * 每个区域保留的不能被用户空间分配的页面数目
     */
    unsigned long       totalreserve_pages;

#ifndef CONFIG_SPARSEMEM
    /*
     * Flags for a pageblock_nr_pages block. See pageblock-flags.h.
     * In SPARSEMEM, this map is stored in struct mem_section
     */
    unsigned long       *pageblock_flags;
#endif /* CONFIG_SPARSEMEM */

#ifdef CONFIG_NUMA
    /*
     * zone reclaim becomes active if more unmapped pages exist.
     */
    unsigned long       min_unmapped_pages;
    unsigned long       min_slab_pages;
#endif /* CONFIG_NUMA */

    /* zone_start_pfn == zone_start_paddr >> PAGE_SHIFT
     * 只内存域的第一个页帧 */
    unsigned long       zone_start_pfn;

    /*
     * spanned_pages is the total pages spanned by the zone, including
     * holes, which is calculated as:
     *      spanned_pages = zone_end_pfn - zone_start_pfn;
     *
     * present_pages is physical pages existing within the zone, which
     * is calculated as:
     *      present_pages = spanned_pages - absent_pages(pages in holes);
     *
     * managed_pages is present pages managed by the buddy system, which
     * is calculated as (reserved_pages includes pages allocated by the
     * bootmem allocator):
     *      managed_pages = present_pages - reserved_pages;
     *
     * So present_pages may be used by memory hotplug or memory power
     * management logic to figure out unmanaged pages by checking
     * (present_pages - managed_pages). And managed_pages should be used
     * by page allocator and vm scanner to calculate all kinds of watermarks
     * and thresholds.
     *
     * Locking rules:
     *
     * zone_start_pfn and spanned_pages are protected by span_seqlock.
     * It is a seqlock because it has to be read outside of zone->lock,
     * and it is done in the main allocator path.  But, it is written
     * quite infrequently.
     *
     * The span_seq lock is declared along with zone->lock because it is
     * frequently read in proximity to zone->lock.  It's good to
     * give them a chance of being in the same cacheline.
     *
     * Write access to present_pages at runtime should be protected by
     * mem_hotplug_begin/end(). Any reader who can't tolerant drift of
     * present_pages should get_online_mems() to get a stable value.
     *
     * Read access to managed_pages should be safe because it's unsigned
     * long. Write access to zone->managed_pages and totalram_pages are
     * protected by managed_page_count_lock at runtime. Idealy only
     * adjust_managed_page_count() should be used instead of directly
     * touching zone->managed_pages and totalram_pages.
     */
    unsigned long       managed_pages;
    unsigned long       spanned_pages;             /*  总页数，包含空洞  */
    unsigned long       present_pages;              /*  可用页数，不包哈空洞  */

    /*  指向管理区的传统名字, "DMA", "NROMAL"或"HIGHMEM" */
    const char          *name;

#ifdef CONFIG_MEMORY_ISOLATION
    /*
     * Number of isolated pageblock. It is used to solve incorrect
     * freepage counting problem due to racy retrieving migratetype
     * of pageblock. Protected by zone->lock.
     */
    unsigned long       nr_isolate_pageblock;
#endif

#ifdef CONFIG_MEMORY_HOTPLUG
    /* see spanned/present_pages for more description */
    seqlock_t           span_seqlock;
#endif

    /*
     * wait_table       -- the array holding the hash table
     * wait_table_hash_nr_entries   -- the size of the hash table array
     * wait_table_bits      -- wait_table_size == (1 << wait_table_bits)
     *
     * The purpose of all these is to keep track of the people
     * waiting for a page to become available and make them
     * runnable again when possible. The trouble is that this
     * consumes a lot of space, especially when so few things
     * wait on pages at a given time. So instead of using
     * per-page waitqueues, we use a waitqueue hash table.
     *
     * The bucket discipline is to sleep on the same queue when
     * colliding and wake all in that wait queue when removing.
     * When something wakes, it must check to be sure its page is
     * truly available, a la thundering herd. The cost of a
     * collision is great, but given the expected load of the
     * table, they should be so rare as to be outweighed by the
     * benefits from the saved space.
     *
     * __wait_on_page_locked() and unlock_page() in mm/filemap.c, are the
     * primary users of these fields, and in mm/page_alloc.c
     * free_area_init_core() performs the initialization of them.
     */
    /*  进程等待队列的散列表, 这些进程正在等待管理区中的某页  */
    wait_queue_head_t       *wait_table;
    /*  等待队列散列表中的调度实体数目  */
    unsigned long       wait_table_hash_nr_entries;
    /*  等待队列散列表数组大小, 值为2^order  */
    unsigned long       wait_table_bits;

    ZONE_PADDING(_pad1_)

    /* free areas of different sizes
       页面使用状态的信息，以每个bit标识对应的page是否可以分配
       是用于伙伴系统的，每个数组元素指向对应阶也表的数组开头
       以下是供页帧回收扫描器(page reclaim scanner)访问的字段
       scanner会跟据页帧的活动情况对内存域中使用的页进行编目
       如果页帧被频繁访问，则是活动的，相反则是不活动的，
       在需要换出页帧时，这样的信息是很重要的：   */
    struct free_area    free_area[MAX_ORDER];

    /* zone flags, see below 描述当前内存的状态, 参见下面的enum zone_flags结构 */
    unsigned long       flags;

    /* Write-intensive fields used from the page allocator, 保存该描述符的自旋锁  */
    spinlock_t          lock;

    ZONE_PADDING(_pad2_)

    /* Write-intensive fields used by page reclaim */

    /* Fields commonly accessed by the page reclaim scanner */
    spinlock_t          lru_lock;   /* LRU(最近最少使用算法)活动以及非活动链表使用的自旋锁  */
    struct lruvec       lruvec;

    /*
     * When free pages are below this point, additional steps are taken
     * when reading the number of free pages to avoid per-cpu counter
     * drift allowing watermarks to be breached
     * 在空闲页的数目少于这个点percpu_drift_mark的时候
     * 当读取和空闲页数一样的内存页时，系统会采取额外的工作，
     * 防止单CPU页数漂移，从而导致水印被破坏。
     */
    unsigned long percpu_drift_mark;

#if defined CONFIG_COMPACTION || defined CONFIG_CMA
    /* pfn where compaction free scanner should start */
    unsigned long       compact_cached_free_pfn;
    /* pfn where async and sync compaction migration scanner should start */
    unsigned long       compact_cached_migrate_pfn[2];
#endif

#ifdef CONFIG_COMPACTION
    /*
     * On compaction failure, 1<<compact_defer_shift compactions
     * are skipped before trying again. The number attempted since
     * last failure is tracked with compact_considered.
     */
    unsigned int        compact_considered;
    unsigned int        compact_defer_shift;
    int                       compact_order_failed;
#endif

#if defined CONFIG_COMPACTION || defined CONFIG_CMA
    /* Set to true when the PG_migrate_skip bits should be cleared */
    bool            compact_blockskip_flush;
#endif

    bool            contiguous;

    ZONE_PADDING(_pad3_)
    /* Zone statistics 内存域的统计信息, 参见后面的enum zone_stat_item结构 */
    atomic_long_t       vm_stat[NR_VM_ZONE_STAT_ITEMS];
} ____cacheline_internodealigned_in_smp;
```

| 字段| 描述 |
| :------- | ----: |
| lowmem_reserve[MAX_NR_ZONES] | 为了防止一些代码必须运行在低地址区域，所以事先保留一些低地址区域的内存 |
| pageset | page管理的数据结构对象，内部有一个page的列表(list)来管理。每个CPU维护一个page list，避免自旋锁的冲突。这个数组的大小和NR_CPUS(CPU的数量）有关，这个值是编译的时候确定的 |
| lock | 对zone并发访问的保护的自旋锁 |
| free_area[MAX_ORDER] | 页面使用状态的信息，以每个bit标识对应的page是否可以分配 |
| lru_lock | LRU(最近最少使用算法)的自旋锁 |
| wait_table | 待一个page释放的等待队列哈希表。它会被wait_on_page()，unlock_page()函数使用. 用哈希表，而不用一个等待队列的原因，防止进程长期等待资源 |
| wait_table_hash_nr_entries | 哈希表中的等待队列的数量 |
| zone_pgdat | 指向这个zone所在的pglist_data对象 |
| zone_start_pfn | 和node_start_pfn的含义一样。这个成员是用于表示zone中的开始那个page在物理内存中的位置的present_pages， spanned_pages: 和node中的类似的成员含义一样 |
| name | zone的名字，字符串表示： "DMA"，"Normal" 和"HighMem" |
| totalreserve_pages | 每个区域保留的不能被用户空间分配的页面数目 |
| ZONE_PADDING | 由于自旋锁频繁的被使用，因此为了性能上的考虑，将某些成员对齐到cache line中，有助于提高执行的性能。使用这个宏，可以确定zone->lock，zone->lru_lock，zone->pageset这些成员使用不同的cache line. |



##4.2	ZONE_PADDING将数据保存在高速缓冲行
-------

该结构比较特殊的地方是它由ZONE_PADDING分隔的几个部分. 这是因为堆zone结构的访问非常频繁. 在多处理器系统中, 通常会有不同的CPU试图同时访问结构成员. 因此使用锁可以防止他们彼此干扰, 避免错误和不一致的问题. 由于内核堆该结构的访问非常频繁, 因此会经常性地获取该结构的两个自旋锁zone->lock和zone->lru_lock



那么数据保存在CPU高速缓存中, 那么会处理得更快速. 高速缓冲分为行, 每一行负责不同的内存区. 内核使用ZONE_PADDING宏生成"填充"字段添加到结构中, 以确保每个自旋锁处于自身的缓存行中

ZONE_PADDING宏定义在[nclude/linux/mmzone.h?v4.7, line 105](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v4.7#L105)

```cpp
/*
 * zone->lock and zone->lru_lock are two of the hottest locks in the kernel.
 * So add a wild amount of padding here to ensure that they fall into separate
 * cachelines.  There are very few zone structures in the machine, so space
 * consumption is not a concern here.
     */
#if defined(CONFIG_SMP)
    struct zone_padding
    {
            char x[0];
    } ____cacheline_internodealigned_in_smp;
    #define ZONE_PADDING(name)      struct zone_padding name;

#else
    #define ZONE_PADDING(name)
 #endif
```


内核还用了____cacheline_internodealigned_in_smp,来实现最优的高速缓存行对其方式.

该宏定义在[include/linux/cache.h](http://lxr.free-electrons.com/source/include/linux/cache.h?v=4.7#L68)

```cpp
#if !defined(____cacheline_internodealigned_in_smp)
	#if defined(CONFIG_SMP)
		#define ____cacheline_internodealigned_in_smp \
        __attribute__((__aligned__(1 << (INTERNODE_CACHE_SHIFT))))
	#else
		#define ____cacheline_internodealigned_in_smp
	#endif
#endif
```

##4.3	水印watermark[NR_WMARK]与kswapd内核线程
-------

Zone的管理调度的一些参数watermarks水印, 水存量很小(MIN)进水量，水存量达到一个标准(LOW)减小进水量，当快要满(HIGH)的时候，可能就关闭了进水口

WMARK_LOW, WMARK_LOW, WMARK_HIGH就是这个标准


```cpp
enum zone_watermarks
{
        WMARK_MIN,
        WMARK_LOW,
        WMARK_HIGH,
        NR_WMARK
};


#define min_wmark_pages(z) (z->watermark[WMARK_MIN])
#define low_wmark_pages(z) (z->watermark[WMARK_LOW])
#define high_wmark_pages(z) (z->watermark[WMARK_HIGH])
```

在linux-2.4中, zone结构中使用如下方式表示水印, 参照[include/linux/mmzone.h?v=2.4.37, line 171](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=2.4.37#L171)

``` c
typedef struct zone_watermarks_s
{
	unsigned long min, low, high;
} zone_watermarks_t;


typedef struct zone_struct {
	zone_watermarks_t       watermarks[MAX_NR_ZONES];
```


在Linux-2.6.x中标准是直接通过成员pages_min， pages_low and pages_high定义在zone结构体中的, 参照[include/linux/mmzone.h?v=2.6.24, line 214](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=2.6.24#L214)


当系统中可用内存很少的时候，系统进程kswapd被唤醒, 开始回收释放page, 水印这些参数(WMARK_MIN, WMARK_LOW,  WMARK_HIGH)影响着这个代码的行为

每个zone有三个水平标准：watermark[WMARK_MIN], watermark[WMARK_LOW],  watermark[WMARK_HIGH]，帮助确定zone中内存分配使用的压力状态

| 标准 | 描述 |
|:----:|:---:|
| watermark[WMARK_MIN] | 当空闲页面的数量达到page_min所标定的数量的时候， 说明页面数非常紧张, 分配页面的动作和kswapd线程同步运行.<br>WMARK_MIN所表示的page的数量值，是在内存初始化的过程中调用[free_area_init_core](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L5932)中计算的。这个数值是根据zone中的page的数量除以一个>1的系数来确定的。通常是这样初始化的ZoneSizeInPages/12 |
| watermark[WMARK_LOW] | 当空闲页面的数量达到WMARK_LOW所标定的数量的时候，说明页面刚开始紧张, 则kswapd线程将被唤醒，并开始释放回收页面 |
| watermark[WMARK_HIGH] | 当空闲页面的数量达到page_high所标定的数量的时候， 说明内存页面数充足, 不需要回收, kswapd线程将重新休眠，通常这个数值是page_min的3倍 |

*	如果空闲页多于pages_high = watermark[WMARK_HIGH], 则说明内存页面充足, 内存域的状态是理想的.

*	如果空闲页的数目低于pages_low = watermark[WMARK_LOW], 则说明内存页面开始紧张, 内核开始将页患处到硬盘.

*	如果空闲页的数目低于pages_min = watermark[WMARK_MIN], 则内存页面非常紧张, 页回收工作的压力就比较大


##4.3	内存域标志
-------

[内存管理域zone_t结构中的flags字段](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v4.7#L475)描述了内存域的当前状态


```cpp
//  http://lxr.free-electrons.com/source/include/linux/mmzone.h#L475
struct zone
{
	/* zone flags, see below */
	unsigned long           flags;
}
```

它允许使用的标识用[`enum zone_flags`](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v4.7#L525)标识, 该枚举标识定义在[include/linux/mmzone.h?v4.7, line 525](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v4.7#L525), 如下所示

```cpp
enum zone_flags
{
    ZONE_RECLAIM_LOCKED,         /* prevents concurrent reclaim */
    ZONE_OOM_LOCKED,               /* zone is in OOM killer zonelist 内存域可被回收*/
    ZONE_CONGESTED,                 /* zone has many dirty pages backed by
                                                    * a congested BDI
                                                    */
    ZONE_DIRTY,                           /* reclaim scanning has recently found
                                                   * many dirty file pages at the tail
                                                   * of the LRU.
                                                   */
    ZONE_WRITEBACK,                 /* reclaim scanning has recently found
                                                   * many pages under writeback
                                                   */
    ZONE_FAIR_DEPLETED,           /* fair zone policy batch depleted */
};
```


| flag标识 |  描述   |
|:-------:|:-------:|
| ZONE_RECLAIM_LOCKED | 防止并发回收, 在SMP上系统, 多个CPU可能试图并发的回收亿i个内存域. ZONE_RECLAIM_LCOKED标志可防止这种情况: 如果一个CPU在回收某个内存域, 则设置该标识. 这防止了其他CPU的尝试 |
| ZONE_OOM_LOCKED | 用于某种不走运的情况: 如果进程消耗了大量的内存, 致使必要的操作都无法完成, 那么内核会使徒杀死消耗内存最多的进程, 以获取更多的空闲页, 该标志可以放置多个CPU同时进行这种操作 |
| ZONE_CONGESTED | 标识当前区域中有很多脏页 |
| ZONE_DIRTY | 用于标识最近的一次页面扫描中, LRU算法发现了很多脏的页面 |
| ZONE_WRITEBACK | 最近的回收扫描发现有很多页在写回 |
| ZONE_FAIR_DEPLETED | 公平区策略耗尽(没懂) |


##4.4	内存域统计信息vm_stat
-------


内存域struct zone的vm_stat维护了大量有关该内存域的统计信息. 由于其中维护的大部分信息曲面没有多大意义

```cpp
//  http://lxr.free-electrons.com/source/include/linux/mmzone.h#L522
struct zone
{
	  atomic_long_t vm_stat[NR_VM_ZONE_STAT_ITEMS];
}
```

vm_stat的统计信息由`enum zone_stat_item`枚举变量标识, 定义在[include/linux/mmzone.h?v=4.7, line 110](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=4.7#L110)

```cpp
enum zone_stat_item
{
    /* First 128 byte cacheline (assuming 64 bit words) */
    NR_FREE_PAGES,
    NR_ALLOC_BATCH,
    NR_LRU_BASE,
    NR_INACTIVE_ANON = NR_LRU_BASE, /* must match order of LRU_[IN]ACTIVE */
    NR_ACTIVE_ANON,         /*  "     "     "   "       "         */
    NR_INACTIVE_FILE,       /*  "     "     "   "       "         */
    NR_ACTIVE_FILE,         /*  "     "     "   "       "         */
    NR_UNEVICTABLE,         /*  "     "     "   "       "         */
    NR_MLOCK,               /* mlock()ed pages found and moved off LRU */
    NR_ANON_PAGES,  /* Mapped anonymous pages */
    NR_FILE_MAPPED, /* pagecache pages mapped into pagetables.
                       only modified from process context */
    NR_FILE_PAGES,
    NR_FILE_DIRTY,
    NR_WRITEBACK,
    NR_SLAB_RECLAIMABLE,
    NR_SLAB_UNRECLAIMABLE,
    NR_PAGETABLE,           /* used for pagetables */
    NR_KERNEL_STACK,
    /* Second 128 byte cacheline */
    NR_UNSTABLE_NFS,        /* NFS unstable pages */
    NR_BOUNCE,
    NR_VMSCAN_WRITE,
    NR_VMSCAN_IMMEDIATE,    /* Prioritise for reclaim when writeback ends */
    NR_WRITEBACK_TEMP,      /* Writeback using temporary buffers */
    NR_ISOLATED_ANON,       /* Temporary isolated pages from anon lru */
    NR_ISOLATED_FILE,       /* Temporary isolated pages from file lru */
    NR_SHMEM,               /* shmem pages (included tmpfs/GEM pages) */
    NR_DIRTIED,             /* page dirtyings since bootup */
    NR_WRITTEN,             /* page writings since bootup */
    NR_PAGES_SCANNED,       /* pages scanned since last reclaim */
#ifdef CONFIG_NUMA
    NUMA_HIT,               /* allocated in intended node */
    NUMA_MISS,              /* allocated in non intended node */
    NUMA_FOREIGN,           /* was intended here, hit elsewhere */
    NUMA_INTERLEAVE_HIT,    /* interleaver preferred this zone */
    NUMA_LOCAL,             /* allocation from local node */
    NUMA_OTHER,             /* allocation from other node */
#endif
    WORKINGSET_REFAULT,
    WORKINGSET_ACTIVATE,
    WORKINGSET_NODERECLAIM,
    NR_ANON_TRANSPARENT_HUGEPAGES,
    NR_FREE_CMA_PAGES,
    NR_VM_ZONE_STAT_ITEMS
};
```

内核提供了很多方式来获取当前内存域的状态信息, 这些函数大多定义在[include/linux/vmstat.h?v=4.7](http://lxr.free-electrons.com/source/include/linux/vmstat.h?v=4.7)



##4.5	等待队列
-------


struct zone中实现了一个等待队列, 可用于等待某一页的进程, 内核将进程排成一个列队, 等待某些条件. 在条件变成真时, 内核会通知进程恢复工作.

```c
struct zone
{
	wait_queue_head_t       *wait_table;
	unsigned long               wait_table_hash_nr_entries;
	unsigned long               wait_table_bits;
}
```

| 字段 | 描述 |
|:-----:|:-----:|
| wait_table | 待一个page释放的等待队列哈希表。它会被wait_on_page()，unlock_page()函数使用. 用哈希表，而不用一个等待队列的原因，防止进程长期等待资源 |
| wait_table_hash_nr_entries | 哈希表中的等待队列的数量 |
| wait_table_bits | 等待队列散列表数组大小 |


##4.6	冷热页
-------

`struct zone`的pageset成员用于实现冷热分配器(hot-n-cold allocator)

内核说页面是热的， 意味着

#5	内存域水印的计算
-------




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
