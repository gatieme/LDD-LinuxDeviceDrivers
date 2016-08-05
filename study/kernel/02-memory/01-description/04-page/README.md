 服务器体系与共享存储器架构
=======

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6225543) |


http://blog.chinaunix.net/uid-30282771-id-5176971.html
http://www.cnblogs.com/hanyan225/archive/2011/07/28/2119628.html
#1	前景回顾
-------

#1.1	UMA和NUMA两种模型
-------

共享存储型多处理机有两种模型

*	均匀存储器存取（Uniform-Memory-Access，简称UMA）模型


*	非均匀存储器存取（Nonuniform-Memory-Access，简称NUMA）模型

#1.2	(N)UMA模型中linux内存的机构
-------

非一致存储器访问(NUMA)模式下

*	处理器被划分成多个"节点"(node), 每个节点被分配有的本地存储器空间. 所有节点中的处理器都可以访问全部的系统物理存储器，但是访问本节点内的存储器所需要的时间，比访问某些远程节点内的存储器所花的时间要少得多


*	内存被分割成多个区域（BANK，也叫"簇"），依据簇与处理器的"距离"不同, 访问不同簇的代码也会不同.


##1.3	Linux如何描述物理内存
-------

Linux把物理内存划分为三个层次来管理

| 层次 | 描述 |
|:----:|:----:|
| 存储节点(Node) |  CPU被划分为多个节点(node), 内存则被分簇, 每个CPU对应一个本地物理内存, 即一个CPU-node对应一个内存簇bank，即每个内存簇被认为是一个节点 |
| 管理区(Zone)   | 每个物理内存节点node被划分为多个内存管理区域, 用于表示不同范围的内存, 内核可以使用不同的映射方式映射物理内存 |
| 页面(Page) 	   |	内存被细分为多个页面帧, 页面是最基本的页面分配的单位　｜


*	首先内存被划分为结点. 内存中的每个节点都是由pg_data_t描述,而pg_data_t由struct pglist_data定义而来, 该数据结构定义在[include/linux/mmzone.h, line 615](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L615), 每个结点关联到系统中的一个处理器, 内核中表示为`pg_data_t`的实例. 系统中每个节点被链接到一个以NULL结尾的`pgdat_list`链表中<而其中的每个节点利用`pg_data_tnode_next`字段链接到下一节．而对于PC这种UMA结构的机器来说, 只使用了一个成为contig_page_data的静态pg_data_t结构.


*	接着各个节点又被划分为内存管理区域, 一个管理区域通过struct zone_struct描述, 其被定义为zone_t, 用以表示内存的某个范围, 低端范围的16MB被描述为ZONE_DMA, 某些工业标准体系结构中的(ISA)设备需要用到它, 然后是可直接映射到内核的普通内存域ZONE_NORMAL,最后是超出了内核段的物理地址域ZONE_HIGHMEM, 被称为高端内存.　是系统中预留的可用内存空间, 不能被内核直接映射.

*	最后页帧(page frame)代表了系统内存的最小单位, 堆内存中的每个页都会创建一个struct page的一个实例. 传统上，把内存视为连续的字节，即内存为字节数组，内存单元的编号(地址)可作为字节数组的索引. 分页管理时，将若干字节视为一页，比如4K byte. 此时，内存变成了连续的页，即内存为页数组，每一页物理内存叫页帧，以页为单位对内存进行编号，该编号可作为页数组的索引，又称为页帧号.


##1.4	今日内容(页帧struct page)
-------


页帧代表了系统内存的最小单位, 对内存中的每个页都会创建struct page的一个实例. 内核必须要保证page结构体足够的小，否则仅struct page就要占用大量的内存.

因为即使在中等程序的内存配置下, 系统的内存同样会分解为大量的页. 例如, IA-32系统中标准页长度为4KB, 在内存大小为384MB时, 大约有100000页. 就当今的标准而言, 这个容量算不上很大, 但页的数目已经非常可观了

因而出于节省内存的考虑，内核要尽力保持struct page尽可能的小. 在典型的系统中, 由于页的数目巨大, 因此对page结构的小改动, 也可能导致保存所有page实例所需的物理内存暴涨.

页的广泛使用, 增加了保持结构长度的难度 : 内存管理的许多部分都使用页, 用于各种不同的用途. 内核的一部分可能完全依赖于struct page提供的特定信息, 而这部分信息堆内核的其他部分页可能是完全无用的. 等等.


我们今天就来详细讲解一下linux下物理页帧的描述

#2	页帧
-------

内核把物理页作为内存管理的基本单位. 尽管处理器的最小可寻址单位通常是字, 但是, 内存管理单元MMU通常以页为单位进行处理. 因此，从虚拟内存的上来看，页就是最小单位.

##2.1	struct page结构
-------

 内核用[struct  page(include/linux/mm_types.h?v=4.7, line 45)](http://lxr.free-electrons.com/source/include/linux/mm_types.h?v4.7#L45)结构表示系统中的每个物理页.

出于节省内存的考虑，struct page中使用了大量的联合体union.

```cpp
/*
 * Each physical page in the system has a struct page associated with
 * it to keep track of whatever it is we are using the page for at the
 * moment. Note that we have no way to track which tasks are using
 * a page, though if it is a pagecache page, rmap structures can tell us
 * who is mapping it.
 *
 * The objects in struct page are organized in double word blocks in
 * order to allows us to use atomic double word operations on portions
 * of struct page. That is currently only used by slub but the arrangement
 * allows the use of atomic double word operations on the flags/mapping
 * and lru list pointers also.
 */
struct page {
    /* First double word block */
    unsigned long flags;        /* Atomic flags, some possibly updated asynchronously
                                              描述page的状态和其他信息  */
    union
    {
        struct address_space *mapping;  /* If low bit clear, points to
                         * inode address_space, or NULL.
                         * If page mapped as anonymous
                         * memory, low bit is set, and
                         * it points to anon_vma object:
                         * see PAGE_MAPPING_ANON below.
                         */
        void *s_mem;            /* slab first object */
        atomic_t compound_mapcount;     /* first tail page */
        /* page_deferred_list().next     -- second tail page */
    };

    /* Second double word */
    struct {
        union {
            pgoff_t index;      /* Our offset within mapping.
            在映射的虚拟空间（vma_area）内的偏移；
            一个文件可能只映射一部分，假设映射了1M的空间，
            index指的是在1M空间内的偏移，而不是在整个文件内的偏移。 */
            void *freelist;     /* sl[aou]b first free object */
            /* page_deferred_list().prev    -- second tail page */
        };

        union {
#if defined(CONFIG_HAVE_CMPXCHG_DOUBLE) && \
    defined(CONFIG_HAVE_ALIGNED_STRUCT_PAGE)
            /* Used for cmpxchg_double in slub */
            unsigned long counters;
#else
            /*
             * Keep _refcount separate from slub cmpxchg_double
             * data.  As the rest of the double word is protected by
             * slab_lock but _refcount is not.
             */
            unsigned counters;
#endif

            struct {

                union {
                    /*
                     * Count of ptes mapped in mms, to show
                     * when page is mapped & limit reverse
                     * map searches.
                     * 页映射计数器
                     */
                    atomic_t _mapcount;

                    struct { /* SLUB */
                        unsigned inuse:16;
                        unsigned objects:15;
                        unsigned frozen:1;
                    };
                    int units;      /* SLOB */
                };
                /*
                 * Usage count, *USE WRAPPER FUNCTION*
                 * when manual accounting. See page_ref.h
                 * 页引用计数器
                 */
                atomic_t _refcount;
            };
            unsigned int active;    /* SLAB */
        };
    };

    /*
     * Third double word block
     *
     * WARNING: bit 0 of the first word encode PageTail(). That means
     * the rest users of the storage space MUST NOT use the bit to
     * avoid collision and false-positive PageTail().
     */
    union {
        struct list_head lru;   /* Pageout list, eg. active_list
                     * protected by zone->lru_lock !
                     * Can be used as a generic list
                     * by the page owner.
                     */
        struct dev_pagemap *pgmap; /* ZONE_DEVICE pages are never on an
                        * lru or handled by a slab
                        * allocator, this points to the
                        * hosting device page map.
                        */
        struct {        /* slub per cpu partial pages */
            struct page *next;      /* Next partial slab */
#ifdef CONFIG_64BIT
            int pages;      /* Nr of partial slabs left */
            int pobjects;   /* Approximate # of objects */
#else
            short int pages;
            short int pobjects;
#endif
        };

        struct rcu_head rcu_head;       /* Used by SLAB
                         * when destroying via RCU
                         */
        /* Tail pages of compound page */
        struct {
            unsigned long compound_head; /* If bit zero is set */

            /* First tail page only */
#ifdef CONFIG_64BIT
            /*
             * On 64 bit system we have enough space in struct page
             * to encode compound_dtor and compound_order with
             * unsigned int. It can help compiler generate better or
             * smaller code on some archtectures.
             */
            unsigned int compound_dtor;
            unsigned int compound_order;
#else
            unsigned short int compound_dtor;
            unsigned short int compound_order;
#endif
        };

#if defined(CONFIG_TRANSPARENT_HUGEPAGE) && USE_SPLIT_PMD_PTLOCKS
        struct {
            unsigned long __pad;    /* do not overlay pmd_huge_pte
                         * with compound_head to avoid
                         * possible bit 0 collision.
                         */
            pgtable_t pmd_huge_pte; /* protected by page->ptl */
        };
#endif
    };

    /* Remainder is not double word aligned */
    union {
        unsigned long private;      /* Mapping-private opaque data:
                         * usually used for buffer_heads
                         * if PagePrivate set; used for
                         * swp_entry_t if PageSwapCache;
                         * indicates order in the buddy
                         * system if PG_buddy is set.
                         * 私有数据指针，由应用场景确定其具体的含义
                         */
#if USE_SPLIT_PTE_PTLOCKS
#if ALLOC_SPLIT_PTLOCKS
        spinlock_t *ptl;
#else
        spinlock_t ptl;
#endif
#endif
        struct kmem_cache *slab_cache;  /* SL[AU]B: Pointer to slab */
    };

#ifdef CONFIG_MEMCG
    struct mem_cgroup *mem_cgroup;
#endif

    /*
     * On machines where all RAM is mapped into kernel address space,
     * we can simply calculate the virtual address. On machines with
     * highmem some memory is mapped into kernel virtual memory
     * dynamically, so we need a place to store that address.
     * Note that this field could be 16 bits on x86 ... ;)
     *
     * Architectures with slow multiplication can define
     * WANT_PAGE_VIRTUAL in asm/page.h
     */
#if defined(WANT_PAGE_VIRTUAL)
    void *virtual;          /* Kernel virtual address (NULL if
                       not kmapped, ie. highmem) */
#endif /* WANT_PAGE_VIRTUAL */

#ifdef CONFIG_KMEMCHECK
    /*
     * kmemcheck wants to track the status of each byte in a page; this
     * is a pointer to such a status block. NULL if not tracked.
     */
    void *shadow;
#endif

#ifdef LAST_CPUPID_NOT_IN_PAGE_FLAGS
    int _last_cpupid;
#endif
}
/*
 * The struct page can be forced to be double word aligned so that atomic ops
 * on double words work. The SLUB allocator can make use of such a feature.
 */
#ifdef CONFIG_HAVE_ALIGNED_STRUCT_PAGE
    __aligned(2 * sizeof(unsigned long))
#endif
;
```

| 字段 | 描述 |
|:---:|:----:|
| flag | 用来存放页的状态，每一位代表一种状态，所以至少可以同时表示出32中不同的状态,这些状态定义在linux/page-flags.h中 |
| virtual | 对于如果物理内存可以直接映射内核的系统, 我们可以之间映射出虚拟地址与物理地址的管理, 但是对于需要使用高端内存区域的页, 即无法直接映射到内核的虚拟地址空间, 因此需要用virtual保存该页的虚拟地址 |
|  _refcount | 引用计数，表示内核中引用该page的次数, 如果要操作该page, 引用计数会+1, 操作完成-1. 当该值为0时, 表示没有引用该page的位置，所以该page可以被解除映射，这往往在内存回收时是有用的 |
| _mapcount | 被页表映射的次数，也就是说该page同时被多少个进程共享。初始值为-1，如果只被一个进程的页表映射了，该值为0. 如果该page处于伙伴系统中，该值为PAGE_BUDDY_MAPCOUNT_VALUE（-128），内核通过判断该值是否为PAGE_BUDDY_MAPCOUNT_VALUE来确定该page是否属于伙伴系统 |
| index | 在映射的虚拟空间（vma_area）内的偏移；一个文件可能只映射一部分，假设映射了1M的空间，index指的是在1M空间内的偏移，而不是在整个文件内的偏移 |
| private | 私有数据指针，由应用场景确定其具体的含义 |
| lru |链表头，用于在各种链表上维护该页, 以便于按页将不同类别分组, 主要有3个用途: 伙伴算法, slab分配器, 被用户态使用或被当做页缓存使用 |
| mapping | 指向与该页相关的address_space对象 |
| index | 页帧在映射内部的偏移量

>注意区分_count和_mapcount，_mapcount表示的是映射次数，而_count表示的是使用次数；被映射了不一定在使用，但要使用必须先映射。





##2.2	mapping & index
-------


mapping指定了页帧所在的地址空间, index是页帧在映射内部的偏移量. 地址空间是一个非常一般的概念. 例如, 可以用在向内存读取文件时. 地址空间用于将文件的内容与装载数据的内存区关联起来. mapping不仅能够保存一个指针, 而且还能包含一些额外的信息, 用于判断页是否属于未关联到地址空间的某个匿名内存区.


1.	如果mapping = 0，说明该page属于交换高速缓存页（swap cache）；当需要使用地址空间时会指定交换分区的地址空间swapper_space。

2.	如果mapping != 0，第0位bit[0] = 0，说明该page属于页缓存或文件映射，mapping指向文件的地址空间address_space。

3.	如果mapping != 0，第0位bit[0] != 0，说明该page为匿名映射，mapping指向struct anon_vma对象。


通过mapping恢复anon_vma的方法：anon_vma = (struct anon_vma *)(mapping - PAGE_MAPPING_ANON)。



pgoff_t index是该页描述结构在地址空间radix树page_tree中的对象索引号即页号, 表示该页在vm_file中的偏移页数, 其类型pgoff_t被定义为unsigned long即一个机器字长.


```cpp
/*
 * The type of an index into the pagecache.
 */
#define pgoff_t unsigned long
```

##2.3	private私有数据指针
-------

private私有数据指针, 由应用场景确定其具体的含义：


1.	如果设置了PG_private标志，则private字段指向struct buffer_head

2.	如果设置了PG_compound，则指向struct page


3.	如果设置了PG_swapcache标志，private存储了该page在交换分区中对应的位置信息swp_entry_t。

4.	如果_mapcount = PAGE_BUDDY_MAPCOUNT_VALUE，说明该page位于伙伴系统，private存储该伙伴的阶



##2.4	lru链表头
-------

最近、最久未使用struct slab结构指针变量

lru：链表头，主要有3个用途：

1.	则page处于伙伴系统中时，用于链接相同阶的伙伴（只使用伙伴中的第一个page的lru即可达到目的）。

2.	设置PG_slab, 则page属于slab，page->lru.next指向page驻留的的缓存的管理结构，page->lru.prec指向保存该page的slab的管理结构。

3.	page被用户态使用或被当做页缓存使用时，用于将该page连入zone中相应的lru链表，供内存回收时使用。




#3	体系结构无关的页面的状态flags
-------

页的不同属性通过一系列页标志描述, 存储在struct page的flag成员中的各个比特位.

```cpp
struct page {
    /* First double word block */
    unsigned long flags;        /* Atomic flags,
    some possibly updated asynchronously, 描述page的状态和其他信息  */
```


这些标识是独立于体系结构的, 因而无法通过特定于CPU或计算机的信息(该信息保存在页表中)

如下如所示

![page的flags标识](./images/flags.png)


主要分为4部分，其中标志位flag向高位增长, 其余位字段向低位增长，中间存在空闲位

| 字段 | 描述 |
|:----:|:---:|
| section | 主要用于稀疏内存模型SPARSEMEM，可忽略 |
| node | NUMA节点号, 标识该page属于哪一个节点 |
| zone | 内存域标志，标识该page属于哪一个zone |
| flag | page的状态标识 |

其中最后一个flag用于标识page的状态, 这些状态由枚举常量[`enum pageflags`](http://lxr.free-electrons.com/source/include/linux/page-flags.h?v=4.7#L74)定义, 定义在[include/linux/page-flags.h?v=4.7, line 74](http://lxr.free-electrons.com/source/include/linux/page-flags.h?v=4.7#L74). 常用的有如下状态

| 页面状态 | 描述 |
|:-------:|:----:|
| PG_locked | 指定了页是否被锁定, 如果该比特未被置位, 说明有使用者正在操作该page, 则内核的其他部分不允许访问该页， 这可以防止内存管理出现竞态条件 |
| PG_error | 如果涉及该page的I/O操作发生了错误, 则该位被设置 |
| PG_referenced | 表示page刚刚被访问过 |
| PG_uptodate | 表示page的数据已经与后备存储器是同步的, 即页的数据已经从块设备读取，且没有出错,数据是最新的 |
| PG_dirty | 与后备存储器中的数据相比，该page的内容已经被修改. 出于性能能的考虑，页并不在每次改变后立即回写, 因此内核需要使用该标识来表明页面中的数据已经改变, 应该在稍后刷出 |
| PG_lru | 表示该page处于LRU链表上， 这有助于实现页面的回收和切换. 内核使用两个最近最少使用(least recently used-LRU)链表来区别活动和不活动页. 如果页在其中一个链表中, 则该位被设置 |
| PG_active | page处于inactive LRU链表, PG_active和PG_referenced一起控制该page的活跃程度，这在内存回收时将会非常有用 |
| PG_slab | 该page属于slab分配器 |
| PG_onwer_priv_1 | |
| PG_arch_1	      | |
| PG_reserved | 设置该标志，防止该page被交换到swap  |
| PG_private | 如果page中的private成员非空，则需要设置该标志, 用于I/O的页可使用该字段将页细分为多核缓冲区 |
| PG_private_2 | |
| PG_writeback | page中的数据正在被回写到后备存储器 |
| PG_head | |
| PG_swapcache | 表示该page处于swap cache中 |
| PG_mappedtodisk | 表示page中的数据在后备存储器中有对应 |
| PG_reclaim | 表示该page要被回收。当PFRA决定要回收某个page后，需要设置该标志 |
| PG_swapbacked | 该page的后备存储器是swap |
| PG_unevictable | 该page被锁住，不能交换，并会出现在LRU_UNEVICTABLE链表中，它包括的几种page：ramdisk或ramfs使用的页, shm_locked、mlock锁定的页 |
| PG_mlocked | 该page在vma中被锁定，一般是通过系统调用mlock()锁定了一段内存 |
| PG_uncached | |
| PG_hwpoison | |
| PG_young | |
| PG_idle  | |



内核中提供了一些标准宏，用来检查、操作某些特定的比特位，这些宏定义在[include/linux/page-flags.h?v=4.7, line 183](http://lxr.free-electrons.com/source/include/linux/page-flags.h?v=4.7#L183)


```c
#define TESTPAGEFLAG(uname, lname, policy)
#define SETPAGEFLAG(uname, lname, policy)
#define CLEARPAGEFLAG(uname, lname, policy)
```

**关于page flags的早期实现**


*	linux-2.6以后的内核中, 很少出现直接用宏定义的标识, 这些标识大多通过enum枚举常量来定义, 然后__NR_XXXX的形式结束, 正好可以标记出宏参数的个数, 但是在早期的实现中, 这些变量都通过宏来标识

例如我们的page->flags用enum pageflags来定义, 内存管理区类型通过zone_type来定义, 但是这些内容在早期的内核中都是通过宏定义来实现的.

*	其次标识的函数接口也变了, 早期的内核中, 针对每个宏标识都设置了一组test/set/clear, 参见[/include/linux/mm.h?v=2.4.37, line 324](http://lxr.free-electrons.com/source/include/linux/mm.h?v=2.4.37#L324)

形式如下
```c
PageXXX(page)：检查page是否设置了PG_XXX位
SetPageXXX(page)：设置page的PG_XXX位
ClearPageXXX(page)：清除page的PG_XXX位
TestSetPageXXX(page)：设置page的PG_XXX位，并返回原值
TestClearPageXXX(page)：清除page的PG_XXX位，并返回原值
```





