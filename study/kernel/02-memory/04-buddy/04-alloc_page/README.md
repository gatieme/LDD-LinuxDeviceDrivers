伙伴系统之页面分配
=======

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6393814) |


# 1 页面分配
-------

## 1.1 页面分配概述
-------

在内核初始化完成之后, 内存管理的责任就由伙伴系统来承担. 伙伴系统基于一种相对简单然而令人吃惊的强大算法.

Linux内核使用二进制伙伴算法来管理和分配物理内存页面, 该算法由Knowlton设计, 后来Knuth又进行了更深刻的描述.

伙伴系统是一个结合了2的方幂个分配器和空闲缓冲区合并计技术的内存分配方案, 其基本思想很简单. 内存被分成含有很多页面的大块, 每一块都是2个页面大小的方幂. 如果找不到想要的块, 一个大块会被分成两部分, 这两部分彼此就成为伙伴. 其中一半被用来分配, 而另一半则空闲. 这些块在以后分配的过程中会继续被二分直至产生一个所需大小的块. 当一个块被最终释放时, 其伙伴将被检测出来, 如果伙伴也空闲则合并两者.

*	内核如何记住哪些内存块是空闲的

*	分配空闲页面的方法

*	影响分配器行为的众多标识位

*	内存碎片的问题和分配器如何处理碎片


## 1.2 内存分配 API
-------


就伙伴系统的接口而言, NUMA 或 UMA 体系结构是没有差别的, 二者的调用语法都是相同的.

所有函数的一个共同点是 : 只能分配 2 的整数幂个页.

因此, 接口中不像C标准库的 malloc 函数或 bootmem 和 memblock 分配器那样指定了所需内存大小作为参数. 相反, 必须指定的是分配阶, 伙伴系统将在内存中分配 $2^order$ 页. 内核中细粒度的分配只能借助于 slab 分配器(或者 slub、slob 分配器), 后者基于伙伴系统


| 内存分配函数 | 功能 | 定义 |
|:----------:|:---:|:----:|
| alloc_pages(mask, order) | 分配$2^order$ 个连续的物理页面, 并返回第一个页面的 struct page 的实例, 表示分配的内存块的起始页 | [NUMA-include/linux/gfp.h, line 466](http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L466)<br>[UMA-include/linux/gfp.h?v=4.7, line 476](http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L476) |
| [`__get_free_pages(mask, order)`](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L3883)<br>[`__get_free_page(mask)`](http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L500) | 工作方式与上述函数相同, 但返回分配内存块的虚拟地址, 而不是page实例<br>32 位系统中, 该函数不会使用高端内存, 如果一定要使用高端内存, 最佳的办法是使用 alloc_pages 和 kmap 函数. |
| alloc_page(mask) | 是前者在order = 0情况下的简化形式, 只分配一页 |  [include/linux/gfp.h?v=4.7, line 483](http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L483) |
| `__get_free_page(gfp_mask)` | 是 `__get_free_pages` 在 `order = 0` 情况下的简化形式, 只分配一页 |  [include/linux/gfp.h?v=5.10, line 483](http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L483) |
| get_zeroed_page(mask) | 分配一页并返回一个page实例, 页对应的内存填充0(所有其他函数, 分配之后页的内容是未定义的) | [mm/page_alloc.c?v=5.10, line 4996](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L4996)| |
| `__get_dma_pages(gfp_mask, order)` | 用来获得适用于DMA的页. | [`include/linux/gfp.h?v=5.10, line 578`](https://elixir.bootlin.com/linux/v5.10/source/include/linux/gfp.h#578) |


在空闲内存无法满足请求以至于分配失败的情况下, 所有上述函数都返回空指针(比如alloc_pages和alloc_page)或者0(比如 `get_zeroed_page`、 `__get_free_pages` 和 `__get_free_page`).

因此内核在各次分配之后都必须检查返回的结果. 这种惯例与设计得很好的用户层应用程序没什么不同, 但在内核中忽略检查会导致严重得多的故障


内核除了伙伴系统函数之外, 还提供了其他内存管理函数. 它们以伙伴系统为基础, 但并不属于伙伴分配器自身. 这些函数包括vmalloc和vmalloc_32, 使用页表将不连续的内存映射到内核地址空间中, 使之看上去是连续的.

还有一组kmalloc类型的函数, 用于分配小于一整页的内存区. 其实现将在以后分别讨论. 

## 1.3 页面分配函数实现上之间的关系
-------

内核源代码将 `__alloc_pages_nodemask` 称之为"伙伴系统的心脏"(`the 'heart' of the zoned buddy allocator`), 因为它处理的是实质性的内存分配.

由于"心脏"的重要性, 我将在后面详细介绍该函数. `__alloc_pages_nodemask` 函数定义在 [include/linux/gfp.h?v=4.7#L428](http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L428)

通过使用标志、内存域修饰符和各个分配函数, 内核提供了一种非常灵活的内存分配体系.尽管如此, 所有接口函数都可以追溯到一个简单的基本函数(alloc_pages_node)


分配单页的函数[`alloc_page`](https://elixir.bootlin.com/linux/v5.10/source/include/linux/gfp.h#L564)和[`__get_free_page`](https://elixir.bootlin.com/linux/v5.10/source/include/linux/gfp.h#L575) 是借助于 alloc_pages 直接分配一个 order 为 0 的页面.

`__get_free_page` 同样分配一页, 但是返回了内存页的虚拟地址, 他是借助 `__get_free_pages` 直接分配一个 order 为 0 的页面.

[`__get_dma_pages`](http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L503) 也是借助于 `__get_free_pages` 使用 `GFP_DMA` 标记从 DMA 中分配页面.

```cpp
//  http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L483
#define alloc_page(gfp_mask) alloc_pages(gfp_mask, 0)

//  http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L500
#define __get_free_page(gfp_mask) \
    __get_free_pages((gfp_mask), 0)`

//  http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L503
#define __get_dma_pages(gfp_mask, order) \
    __get_free_pages((gfp_mask) | GFP_DMA, (order))
```

[`get_zeroed_page`](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L3900)的实现也没什么困难, 对`__get_free_pages`使用`__GFP_ZERO`标志, 即可分配填充字节0的页. 再返回与页关联的内存区地址即可.


```cpp
//  http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L3900
unsigned long get_zeroed_page(gfp_t gfp_mask)
{
        return __get_free_pages(gfp_mask | __GFP_ZERO, 0);
}
EXPORT_SYMBOL(get_zeroed_page);
```

可以看到 `__get_free_page`, `__get_dma_pages`, `get_zeroed_page` 都是直接借助了 `__get_free_pages` 函数来实现的, 他们都是直接返回内存块的虚拟地址.


[`__get_free_pages`](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L3883)调用 `alloc_pages` 完成内存分配. 然后通过 page_address 获取虚拟地址.

```cpp
//  http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L3883
unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order)
{
    struct page *page;

    /*
     * __get_free_pages() returns a 32-bit address, which cannot represent
     * a highmem page
     */
    VM_BUG_ON((gfp_mask & __GFP_HIGHMEM) != 0);

    page = alloc_pages(gfp_mask, order);
    if (!page)
        return 0;
    return (unsigned long) page_address(page);
}
EXPORT_SYMBOL(__get_free_pages);
```

在这种情况下,  使用了一个普通函数而不是宏,  因为 `alloc_pages` 返回的 `page` 实例需要使用辅助

函数 `page_address` 转换为内存地址. 在这里, 只要知道该函数可根据 `page` 实例计算相关页的线性内存地址即可. 对高端内存页这是有问题的


<font color = 0x00ffff>
这样, 就完成了所有分配内存的API函数到公共的基础函数`alloc_pages`的统一
</font>

![伙伴系统中各个分配函数之间的关系](../images/alloc_pages.png)


# 2	alloc_pages 函数分配页
-------


## 2.1 alloc_pages 接口实现
-------


既然所有的内存分配API函数都可以追溯掉 `alloc_page` 函数, 从某种意义上说, 该函数是伙伴系统主要实现的"发射台".


`alloc_pages`函数的定义是依赖于NUMA或者UMA架构的, 定义如下


```cpp
#ifdef CONFIG_NUMA

//  http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L465
static inline struct page *
alloc_pages(gfp_t gfp_mask, unsigned int order)
{
        return alloc_pages_current(gfp_mask, order);
}

#else

//  http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L476
#define alloc_pages(gfp_mask, order) \
                alloc_pages_node(numa_node_id(), gfp_mask, order)
#endif
```


## 2.2 UMA
-------

```cpp
|---->alloc_pages
|
     |---->alloc_pages_node
     |    
          |---->__alloc_pages_node
          |
               |---->__alloc_pages
               |
                   |---->__alloc_pages_nodemask
                   |
```



UMA结构下的`alloc_pages`是通过`alloc_pages_node`函数实现的, 下面我们看看`alloc_pages_node`函数的定义, 在[include/linux/gfp.h?v=4.7, line 448](http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L448)


```cpp
//  http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L448
/*
 * Allocate pages, preferring the node given as nid. When nid == NUMA_NO_NODE,
 * prefer the current CPU's closest node. Otherwise node must be valid and
 * online.
 */
static inline struct page *alloc_pages_node(int nid, gfp_t gfp_mask,
                        unsigned int order)
{
    if (nid == NUMA_NO_NODE)
        nid = numa_mem_id();

    return __alloc_pages_node(nid, gfp_mask, order);
}
```

它只是执行了一个简单的检查, 如果指定负的结点ID(不存在, 即[NUMA_NO_NODE = -1](http://lxr.free-electrons.com/source/include/linux/numa.h?v=4.7#L13)), 内核自动地使用当前执行CPU对应的结点nid = [numa_mem_id();](http://lxr.free-electrons.com/source/include/linux/topology.h?v=4.7#L137), 然后调用`__alloc_pages_node`函数进行了内存分配



`__alloc_pages_node`函数定义在[include/linux/gfp.h?v=4.7, line 435)](http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L435), 如下所示

```cpp
// https://elixir.bootlin.com/linux/v5.10/source/include/linux/gfp.h#L519
/*
 * Allocate pages, preferring the node given as nid. The node must be valid and
 * online. For more general interface, see alloc_pages_node().
 */
static inline struct page *
__alloc_pages_node(int nid, gfp_t gfp_mask, unsigned int order)
{
    VM_BUG_ON(nid < 0 || nid >= MAX_NUMNODES);
    VM_WARN_ON((gfp_mask & __GFP_THISNODE) && !node_online(nid));

    return __alloc_pages(gfp_mask, order, nid);
}
```

内核假定传递给改alloc_pages_node函数的结点nid是被激活, 即online的.但是为了安全它还是检查并警告内存结点不存在的情况. 接下来的工作委托给__alloc_pages, 只需传递一组适当的参数, 其中包括节点nid的备用内存域列表zonelist.


现在`__alloc_pages`函数没什么特别的, 它直接将自己的所有信息传递给`__alloc_pages_nodemask`来完成内存的分配

```cpp
// https://elixir.bootlin.com/linux/v5.10/source/include/linux/gfp.h#L509
static inline struct page *
__alloc_pages(gfp_t gfp_mask, unsigned int order, int preferred_nid)
{
    return __alloc_pages_nodemask(gfp_mask, order, preferred_nid, NULL);
}
```


## 2.3 NUMA 
-------

```cpp
|---->alloc_pages
|
     |---->alloc_pages_current
     |
          |---->alloc_page_interleave
          |
               |---->__alloc_pages_nodemask
               |
```

## 2.4 `__get_free_pages`
-------

```cpp
#  https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L4985
/*
 * Common helper functions. Never use with __GFP_HIGHMEM because the returned
 * address cannot represent highmem pages. Use alloc_pages and then kmap if
 * you need to access high mem.
 */
unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order)
{
    struct page *page;

    page = alloc_pages(gfp_mask & ~__GFP_HIGHMEM, order);
    if (!page)
        return 0;
    return (unsigned long) page_address(page);
}
EXPORT_SYMBOL(__get_free_pages);
```

## 2.5 `__get_dma_pages`
-------

```cpp
# https://elixir.bootlin.com/linux/v5.10/source/include/linux/gfp.h#578
#define __get_dma_pages(gfp_mask, order) \
        __get_free_pages((gfp_mask) | GFP_DMA, (order))
```

## 2.6 alloc_page 的流程
-------

```cpp
# https://elixir.bootlin.com/linux/v5.10/source/include/linux/gfp.h#L564
#define alloc_page(gfp_mask) alloc_pages(gfp_mask, 0)
```

# 3   伙伴系统的心脏 `__alloc_pages_nodemask`
-------


另外所有体系结构都必须实现的标准函数`clear_page`, 可帮助alloc_pages对页填充字节0, 实现如下表所示

| x86 | arm |
|:----:|:-----:|
| [arch/x86/include/asm/page_32.h?v=4.7, line 24](http://lxr.free-electrons.com/source/arch/x86/include/asm/page_32.h?v=4.7#L24) | [arch/arm/include/asm/page.h?v=4.7#L14](http://lxr.free-electrons.com/source/arch/arm/include/asm/page.h?v=4.7#L142)<br>[arch/arm/include/asm/page-nommu.h](http://lxr.free-electrons.com/source/arch/arm/include/asm/page-nommu.h?v=4.7#L20) |


## 3.1  `__alloc_pages_nodemask` 如何分配内存
-------

如前所述, `__alloc_pages_nodemask` 是伙伴系统的心脏. 我们已经处理了所有的准备工作并描述了所有可能的标志, 现在我们把注意力转向相对复杂的部分 : 函数`__alloc_pages_nodemask`的实现, 这也是内核中比较冗长的部分
之一. 特别是在可用内存太少或逐渐用完时, 函数就会比较复杂. 如果可用内存足够, 则必要的工作会很快完成, 就像下述代码

### 3.1.1   函数源代码注释
-------

`__alloc_pages_nodemask` 函数定义在[include/linux/gfp.h?v=4.7#L428](http://lxr.free-electrons.com/source/include/linux/gfp.h?v=4.7#L428)




```cpp
//  http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L3779
/*
 * This is the 'heart' of the zoned buddy allocator.
 */
struct page *
__alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order,
            struct zonelist *zonelist, nodemask_t *nodemask)
{
    struct page *page;
    unsigned int cpuset_mems_cookie;
    unsigned int alloc_flags = ALLOC_WMARK_LOW|ALLOC_FAIR;
    gfp_t alloc_mask = gfp_mask; /* The gfp_t that was actually used for allocation */
    struct alloc_context ac = {
        .high_zoneidx = gfp_zone(gfp_mask),
        .zonelist = zonelist,
        .nodemask = nodemask,
        .migratetype = gfpflags_to_migratetype(gfp_mask),
    };

    if (cpusets_enabled()) {
        alloc_mask |= __GFP_HARDWALL;
        alloc_flags |= ALLOC_CPUSET;
        if (!ac.nodemask)
            ac.nodemask = &cpuset_current_mems_allowed;
    }

    gfp_mask &= gfp_allowed_mask;

    lockdep_trace_alloc(gfp_mask);

    might_sleep_if(gfp_mask & __GFP_DIRECT_RECLAIM);

    if (should_fail_alloc_page(gfp_mask, order))
        return NULL;

    /*
     * Check the zones suitable for the gfp_mask contain at least one
     * valid zone. It's possible to have an empty zonelist as a result
     * of __GFP_THISNODE and a memoryless node
     */
    if (unlikely(!zonelist->_zonerefs->zone))
        return NULL;

    if (IS_ENABLED(CONFIG_CMA) && ac.migratetype == MIGRATE_MOVABLE)
        alloc_flags |= ALLOC_CMA;

retry_cpuset:
    cpuset_mems_cookie = read_mems_allowed_begin();

    /* Dirty zone balancing only done in the fast path */
    ac.spread_dirty_pages = (gfp_mask & __GFP_WRITE);

    /*
     * The preferred zone is used for statistics but crucially it is
     * also used as the starting point for the zonelist iterator. It
     * may get reset for allocations that ignore memory policies.
     */
    ac.preferred_zoneref = first_zones_zonelist(ac.zonelist,
                    ac.high_zoneidx, ac.nodemask);
    if (!ac.preferred_zoneref) {
        page = NULL;
        goto no_zone;
    }

    /* First allocation attempt */
    page = get_page_from_freelist(alloc_mask, order, alloc_flags, &ac);
    if (likely(page))
        goto out;

    /*
     * Runtime PM, block IO and its error handling path can deadlock
     * because I/O on the device might not complete.
     */
    alloc_mask = memalloc_noio_flags(gfp_mask);
    ac.spread_dirty_pages = false;

    /*
     * Restore the original nodemask if it was potentially replaced with
     * &cpuset_current_mems_allowed to optimize the fast-path attempt.
     */
    if (cpusets_enabled())
        ac.nodemask = nodemask;
    page = __alloc_pages_slowpath(alloc_mask, order, &ac);

no_zone:
    /*
     * When updating a task's mems_allowed, it is possible to race with
     * parallel threads in such a way that an allocation can fail while
     * the mask is being updated. If a page allocation is about to fail,
     * check if the cpuset changed during allocation and if so, retry.
     */
    if (unlikely(!page && read_mems_allowed_retry(cpuset_mems_cookie))) {
        alloc_mask = gfp_mask;
        goto retry_cpuset;
    }

out:
    if (kmemcheck_enabled && page)
        kmemcheck_pagealloc_alloc(page, order, gfp_mask);

    trace_mm_page_alloc(page, order, alloc_mask, ac.migratetype);

    return page;
}
EXPORT_SYMBOL(__alloc_pages_nodemask);
```

最简单的情形中, 分配空闲内存区只涉及调用一次 `get_page_from_freelist`, 然后返回所需数目的页(由标号got_pg处的代码处理).

第一次内存分配尝试不会特别积极. 如果在某个内存域中无法找到空闲内存, 则意味着内存没剩下多少了, 内核需要增加较多的工作量才能找到更多内存("重型武器"稍后才会出现).





## 3.2  快速路径 get_page_from_freelist 函数
-------

http://blog.csdn.net/yuzhihui_no1/article/details/50776826
http://bbs.chinaunix.net/thread-3769001-1-1.html

`get_page_from_freelist` 是伙伴系统使用的另一个重要的辅助函数. 它从伙伴系统的空闲页面链表中尝试分配物理页面. 它通过标志集和分配阶来判断是否能进行分配. 如果可以, 则发起实际的分配操作. 该函数定义在[mm/page_alloc.c?v=4.7, line 2905](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L2905)

### 3.2.1 alloc_context
-------

这个函数的参数很有意思, 之前的时候这个函数的参数只能用复杂来形容


```cpp
static struct page *  
get_page_from_freelist(gfp_t gfp_mask, nodemask_t *nodemask, unsigned int order,  
        struct zonelist *zonelist, int high_zoneidx, int alloc_flags,  
        struct zone *preferred_zone, int migratetype)  
```

但是这仍然不够, 随着内核的不段改进, 所支持的特性也越多, 分配内存时需要参照的标识也越来越多, 那难道看着这个函数的参数不断膨胀么, 这个不是内核黑客们所能容忍的, 于是大家想出了一个解决方案, 把那些相关联的参数封装成一个结构

```cpp
static struct page *
get_page_from_freelist(gfp_t gfp_mask, unsigned int order, int alloc_flags, const struct alloc_context *ac)
```


这个封装好的结构就是[`struct alloc_context`](http://lxr.free-electrons.com/source/mm/internal.h?v=4.7#L103), 定义在[mm/internal.h?v=4.7, line 103](http://lxr.free-electrons.com/source/mm/internal.h?v=4.7#L103)

```cpp
/*
 * Structure for holding the mostly immutable allocation parameters passed
 * between functions involved in allocations, including the alloc_pages*
 * family of functions.
 *
 * nodemask, migratetype and high_zoneidx are initialized only once in
 * __alloc_pages_nodemask() and then never change.
 *
 * zonelist, preferred_zone and classzone_idx are set first in
 * __alloc_pages_nodemask() for the fast path, and might be later changed
 * in __alloc_pages_slowpath(). All other functions pass the whole strucure
 * by a const pointer.
 */
struct alloc_context {
        struct zonelist *zonelist;
        nodemask_t *nodemask;
        struct zoneref *preferred_zoneref;
        int migratetype;
        enum zone_type high_zoneidx;
        bool spread_dirty_pages;
};
```

| 字段 | 描述 |
|:-----:|:-----:|
| zonelist | 当perferred_zone上没有合适的页可以分配时, 就要按zonelist中的顺序扫描该zonelist中备用zone列表, 一个个的试用 |
| nodemask | 表示节点的mask, 就是是否能在该节点上分配内存, 这是个bit位数组 |
| preferred_zone | 表示从high_zoneidx后找到的合适的zone, 一般会从该zone分配；分配失败的话, 就会在zonelist再找一个preferred_zone = 合适的zone |
| migratetype | 迁移类型, 在zone->free_area.free_list[XXX] 作为分配下标使用, 这个是用来反碎片化的, 修改了以前的free_area结构体, 在该结构体中再添加了一个数组, 该数组以迁移类型为下标, 每个数组元素都挂了对应迁移类型的页链表 |
| high_zoneidx | 是表示该分配时, 所能分配的最高zone, 一般从high-->normal-->dma 内存越来越昂贵, 所以一般从high到dma分配依次分配 |
| spread_dirty_pages | |


zonelist是指向备用列表的指针. 在预期内存域没有空闲空间的情况下, 该列表确定了扫描系统其他内存域(和结点)的顺序.

### 3.2.2 get_page_from_freelist
-------

for 循环所作的基本上与直觉一致(从高端向低端扫描), for_next_zone_zonelist_nodemask 遍历备用列表的所有内存域, 用最简单的方式查找一个适当的空闲内存块

*   首先, 如果使能了 CPUSET, 且内存分配附带了 ALLOC_CPUSET 标记, 则只能(从 CPUSET 限定的)该进程允许运行 CPU 的所属内存域去分配内存, 通过 `__cpuset_zone_allowed` 来检查.

*   `zone_watermark_fast` 检测当前 `ZONE` 的水位情况, 检查是否能够满足当前多个页面的分配请求. 如果水位不足, 则会 `zone_reclaim` 尝试去回收内存. 如果没有正常回收, 或者回收的内存不够, 都将跳过从 ZONE 上分配内存, 回收完成后, 通过 `zone_watermark_ok` 再次检查 ZONE 的水位情况以及是否满足连续大内存块的分配需求. 

*   如果没有足够的空闲页, 或者没有连续内存块可满足分配请求, 则循环进行到备用列表中的下一个内存域, 继续同样的检查. 直到找到一个合适的页面, 再进行 `try_this_node` 进行内存分配. 请注意即使一切检查都成功了, 最终依然可能会分配失败. 这种情况可能是:

    1.  外碎片化严重, 没有足够的连续内存页

    2.  也可能是无法从 ZONE 内其他迁移类型中借用内存出来.


*   如果内存域适用于当前的分配请求, 那么则通过 `rmqueue` 从伙伴系统中分配内存.

```cpp
// https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L3788
/*
 * get_page_from_freelist goes through the zonelist trying to allocate
 * a page.
 */
static struct page *
get_page_from_freelist(gfp_t gfp_mask, unsigned int order, int alloc_flags,
                        const struct alloc_context *ac)
{
    struct zoneref *z;
    struct zone *zone;
    struct pglist_data *last_pgdat_dirty_limit = NULL;
    bool no_fallback;

retry:
    /*
     * Scan zonelist, looking for a zone with enough free.
     * See also __cpuset_node_allowed() comment in kernel/cpuset.c.
     */
    no_fallback = alloc_flags & ALLOC_NOFRAGMENT;
    z = ac->preferred_zoneref;
    for_next_zone_zonelist_nodemask(zone, z, ac->highest_zoneidx,
                    ac->nodemask) {
        struct page *page;
        unsigned long mark;

        if (cpusets_enabled() &&
            (alloc_flags & ALLOC_CPUSET) &&
            !__cpuset_zone_allowed(zone, gfp_mask))
                continue;
        /*
         * When allocating a page cache page for writing, we
         * want to get it from a node that is within its dirty
         * limit, such that no single node holds more than its
         * proportional share of globally allowed dirty pages.
         * The dirty limits take into account the node's
         * lowmem reserves and high watermark so that kswapd
         * should be able to balance it without having to
         * write pages from its LRU list.
         *
         * XXX: For now, allow allocations to potentially
         * exceed the per-node dirty limit in the slowpath
         * (spread_dirty_pages unset) before going into reclaim,
         * which is important when on a NUMA setup the allowed
         * nodes are together not big enough to reach the
         * global limit.  The proper fix for these situations
         * will require awareness of nodes in the
         * dirty-throttling and the flusher threads.
         */
        if (ac->spread_dirty_pages) {
            if (last_pgdat_dirty_limit == zone->zone_pgdat)
                continue;

            if (!node_dirty_ok(zone->zone_pgdat)) {
                last_pgdat_dirty_limit = zone->zone_pgdat;
                continue;
            }
        }

        if (no_fallback && nr_online_nodes > 1 &&
            zone != ac->preferred_zoneref->zone) {
            int local_nid;

            /*
             * If moving to a remote node, retry but allow
             * fragmenting fallbacks. Locality is more important
             * than fragmentation avoidance.
             */
            local_nid = zone_to_nid(ac->preferred_zoneref->zone);
            if (zone_to_nid(zone) != local_nid) {
                alloc_flags &= ~ALLOC_NOFRAGMENT;
                goto retry;
            }
        }

        mark = wmark_pages(zone, alloc_flags & ALLOC_WMARK_MASK);
        if (!zone_watermark_fast(zone, order, mark,
                       ac->highest_zoneidx, alloc_flags,
                       gfp_mask)) {
            int ret;

#ifdef CONFIG_DEFERRED_STRUCT_PAGE_INIT
            /*
             * Watermark failed for this zone, but see if we can
             * grow this zone if it contains deferred pages.
             */
            if (static_branch_unlikely(&deferred_pages)) {
                if (_deferred_grow_zone(zone, order))
                    goto try_this_zone;
            }
#endif
            /* Checked here to keep the fast path fast */
            BUILD_BUG_ON(ALLOC_NO_WATERMARKS < NR_WMARK);
            if (alloc_flags & ALLOC_NO_WATERMARKS)
                goto try_this_zone;

            if (node_reclaim_mode == 0 ||
                !zone_allows_reclaim(ac->preferred_zoneref->zone, zone))
                continue;

            ret = node_reclaim(zone->zone_pgdat, gfp_mask, order);
            switch (ret) {
            case NODE_RECLAIM_NOSCAN:
                /* did not scan */
                continue;
            case NODE_RECLAIM_FULL:
                /* scanned but unreclaimable */
                continue;
            default:
                /* did we reclaim enough */
                if (zone_watermark_ok(zone, order, mark,
                    ac->highest_zoneidx, alloc_flags))
                    goto try_this_zone;

                continue;
            }
        }

try_this_zone:
        page = rmqueue(ac->preferred_zoneref->zone, zone, order,
                gfp_mask, alloc_flags, ac->migratetype);
        if (page) {
            prep_new_page(page, order, gfp_mask, alloc_flags);

            /*
             * If this is a high-order atomic allocation then check
             * if the pageblock should be reserved for the future
             */
            if (unlikely(order && (alloc_flags & ALLOC_HARDER)))
                reserve_highatomic_pageblock(page, zone, order);

            return page;
        } else {
#ifdef CONFIG_DEFERRED_STRUCT_PAGE_INIT
            /* Try again if zone has deferred pages */
            if (static_branch_unlikely(&deferred_pages)) {
                if (_deferred_grow_zone(zone, order))
                    goto try_this_zone;
            }
#endif
        }
    }

    /*
     * It's possible on a UMA machine to get through all zones that are
     * fragmented. If avoiding fragmentation, reset and try again.
     */
    if (no_fallback) {
        alloc_flags &= ~ALLOC_NOFRAGMENT;
        goto retry;
    }

    return NULL;
}
```


## 3.3	水位控制
-------

我们先把注意力转向页面选择是如何工作的. 

### 3.3.1	内存水印标志
-------

还记得之前讲过的内存水印么

```cpp
enum zone_watermarks {
        WMARK_MIN,
        WMARK_LOW,
        WMARK_HIGH,
        NR_WMARK
};

#define min_wmark_pages(z) (z->watermark[WMARK_MIN])
#define low_wmark_pages(z) (z->watermark[WMARK_LOW])
#define high_wmark_pages(z) (z->watermark[WMARK_HIGH])
````



内核需要定义一些函数使用的标志, 用于控制到达各个水印指定的临界状态时的行为, 这些标志用宏来定义, 定义在[mm/internal.h?v=4.7, line 453](http://lxr.free-electrons.com/source/mm/internal.h?v=4.7#L453)

```cpp
/* The ALLOC_WMARK bits are used as an index to zone->watermark */
#define ALLOC_WMARK_MIN         WMARK_MIN	/*  1 = 0x01, 使用pages_min水印  */
#define ALLOC_WMARK_LOW         WMARK_LOW	/*  2 = 0x02, 使用pages_low水印  */
#define ALLOC_WMARK_HIGH        WMARK_HIGH   /*  3 = 0x03, 使用pages_high水印  */
#define ALLOC_NO_WATERMARKS     0x04 /* don't check watermarks at all  完全不检查水印 */

/* Mask to get the watermark bits */
#define ALLOC_WMARK_MASK        (ALLOC_NO_WATERMARKS-1)

#define ALLOC_HARDER            0x10 /* try to alloc harder, 试图更努力地分配, 即放宽限制  */
#define ALLOC_HIGH              0x20 /* __GFP_HIGH set, 设置了__GFP_HIGH */
#define ALLOC_CPUSET            0x40 /* check for correct cpuset, 检查内存结点是否对应着指定的CPU集合 */
#define ALLOC_CMA               0x80 /* allow allocations from CMA areas */
#define ALLOC_FAIR              0x100 /* fair zone allocation */
```

前几个标志(`ALLOC_WMARK_MIN`, `ALLOC_WMARK_LOW`, `ALLOC_WMARK_HIGH`, `ALLOC_NO_WATERMARKS`)表示在判断页是否可分配时, 需要考虑哪些水印. 默认情况下(即没有因其他因素带来的压力而需要更多的内存), 只有内存域包含页的数目至少为zone->pages_high时, 才能分配页.这对应于`ALLOC_WMARK_HIGH`标志. 如果要使用较低(zone->pages_low)或最低(zone->pages_min)设置, 则必须相应地设置`ALLOC_WMARK_MIN`或`ALLOC_WMARK_LOW`. 而`ALLOC_NO_WATERMARKS`则通知内核在进行内存分配时不要考虑内存水印.


`ALLOC_HARDER`通知伙伴系统在急需内存时放宽分配规则. 在分配高端内存域的内存时, `ALLOC_HIGH`进一步放宽限制. 

`ALLOC_CPUSET`告知内核, 内存只能从当前进程允许运行的CPU相关联的内存结点分配, 当然该选项只对NUMA系统有意义.

`ALLOC_CMA`通知伙伴系统从CMD区域中分配内存

最后, `ALLOC_FAIR`则希望内核公平(均匀)的从内存域zone中进行内存分配



### 3.3.2	`zone_watermark_ok` 判定能否从给定域中分配内存
-------

设置的标志在`zone_watermark_ok`函数中检查, 该函数根据设置的标志判断是否能从给定的内存域分配内存. 该函数定义在[mm/page_alloc.c?v=4.7, line 2820](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L2820)

```cpp
//  http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L2820
bool zone_watermark_ok(struct zone *z, unsigned int order, unsigned long mark,
              int classzone_idx, unsigned int alloc_flags)
{
    return __zone_watermark_ok(z, order, mark, classzone_idx, alloc_flags,
                    zone_page_state(z, NR_FREE_PAGES));
}
```

而 `__zone_watermark_ok` 函数则完成了检查的工作, 该函数定义在[mm/page_alloc.c?v=4.7, line 2752](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L2752)

```cpp
// http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L2752
/*
 * Return true if free base pages are above 'mark'. For high-order checks it
 * will return true of the order-0 watermark is reached and there is at least
 * one free page of a suitable size. Checking now avoids taking the zone lock
 * to check in the allocation paths if no pages are free.
 */
bool __zone_watermark_ok(struct zone *z, unsigned int order, unsigned long mark,
             int classzone_idx, unsigned int alloc_flags,
             long free_pages)
{
    long min = mark;
    int o;
    const bool alloc_harder = (alloc_flags & ALLOC_HARDER);

    /* free_pages may go negative - that's OK
     * free_pages可能变为负值, 没有关系 */
    free_pages -= (1 << order) - 1;

    if (alloc_flags & ALLOC_HIGH)
        min -= min / 2;

    /*
     * If the caller does not have rights to ALLOC_HARDER then subtract
     * the high-atomic reserves. This will over-estimate the size of the
     * atomic reserve but it avoids a search.
     */
    if (likely(!alloc_harder))
        free_pages -= z->nr_reserved_highatomic;
    else
        min -= min / 4;

#ifdef CONFIG_CMA
    /* If allocation can't use CMA areas don't use free CMA pages */
    if (!(alloc_flags & ALLOC_CMA))
        free_pages -= zone_page_state(z, NR_FREE_CMA_PAGES);
#endif

    /*
     * Check watermarks for an order-0 allocation request. If these
     * are not met, then a high-order request also cannot go ahead
     * even if a suitable page happened to be free.
     */
    if (free_pages <= min + z->lowmem_reserve[classzone_idx])
        return false;

    /* If this is an order-0 request then the watermark is fine */
    if (!order)
        return true;

    /* For a high-order request, check at least one suitable page is free 
     * 在下一阶, 当前阶的页是不可用的  */
    for (o = order; o < MAX_ORDER; o++) {
        struct free_area *area = &z->free_area[o];
        int mt;

        if (!area->nr_free)
            continue;

        if (alloc_harder)
            return true;

        /* 所需高阶空闲页的数目相对较少 */
        for (mt = 0; mt < MIGRATE_PCPTYPES; mt++) {
            if (!list_empty(&area->free_list[mt]))
                return true;
        }

#ifdef CONFIG_CMA
        if ((alloc_flags & ALLOC_CMA) &&
            !list_empty(&area->free_list[MIGRATE_CMA])) {
            return true;
        }
#endif
    }
    return false;
}
```


我们知道 [`zone_per_state`](http://lxr.free-electrons.com/source/include/linux/vmstat.h?v=4.7#L130)用来访问每个内存域的统计量. 在上述代码中, 得到的是空闲页的数目.

```cpp
free_pages -= zone_page_state(z, NR_FREE_CMA_PAGES);
```

在解释了`ALLOC_HIGH`和`ALLOC_HARDER`标志之后(将最小值标记降低到当前值的一半或四分之一, 使得分配过程努力或更加努力), 
```cpp
if (alloc_flags & ALLOC_HIGH)
	min -= min / 2;

if (likely(!alloc_harder))
	free_pages -= z->nr_reserved_highatomic;
else
	min -= min / 4;
```


该函数会检查空闲页的数目`free_pages`是否小于最小值与[`lowmem_reserve`](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=4.7#L341)中指定的紧急分配值`min`之和.

```cpp
if (free_pages <= min + z->lowmem_reserve[classzone_idx])
	return false;
```

如果不小于, 则代码遍历所有小于当前阶的分配阶, 其中nr_free记载的是当前分配阶的空闲页块数目.

```cpp
/* For a high-order request, check at least one suitable page is free */
for (o = order; o < MAX_ORDER; o++) {
    struct free_area *area = &z->free_area[o];
    int mt;

    if (!area->nr_free)
        continue;

    if (alloc_harder)
        return true;

    for (mt = 0; mt < MIGRATE_PCPTYPES; mt++) {
        if (!list_empty(&area->free_list[mt]))
            return true;
    }

#ifdef CONFIG_CMA
    if ((alloc_flags & ALLOC_CMA) &&
        !list_empty(&area->free_list[MIGRATE_CMA])) {
        return true;
    }
#endif
}
```

如果内核遍历所有的低端内存域之后, 发现内存不足, 则不进行内存分配.


### 3.3.3 zone_watermark_fast
-------


## 3.4 rmqueue
-------

### 3.4.1 rmqueue 的基本流程

![rmqueue 流程](./rmqueue.png)


```cpp
// https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L3421
```

1.  对于分配分配单页的情况(oder == 0), 则使用 [`rmqueue_pcplist`](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L3396) 直接从冷热页缓存 PCP(Per Cpu Pages)中获取.

2.  如果设置了 ALLOC_HARDER, 则说明是一次高优先级的分配, 就[从迁移类型为 MIGRATE_HIGHATOMIC 的内存中进行分配](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#3458). MIGRATE_HIGHATOMIC 类型的页用常用于于一些紧急情况下的内存分配.

3.  常规模式下, 一般都是通过 [`__rmqueue`](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L2841) 从伙伴系统中指定迁移类型为 migratetype 的链表中获取.
    
    3.1 如果超过一半空闲内存位于 CMA 区域时, 则优先使用 [`__rmqueue_cma_fallback`](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L2348) 从 CMA 中分配

    3.2 标准流程就是 [`__rmqueue_smallest`](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L2348) 从 各个 order 的空闲链表中获取, 首先从当前 order 的空闲链表中获取页面, 如果当前 order 页面不足, 则开始向高 order 的链表中切蛋糕.

    3.3 如果切蛋糕也失败了, 说明空闲的页面已经不足以支撑此次分配, 则从 CMA 区域中预留的内存中进行分配.

    3.4 如果前面流程都失败了, 那么说明当前 MIGRATE_TYPE 中已经没有足够的连续物理页面, 那么进入 fallback 流程, 尝试从其他 MIGRATE_TYPE 中窃取内存.


4.  通过 [`check_new_pages`](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L2253) 检查获取到的内存页是否满足要求. 伙伴系统返回了所获取的的物理页面块第一个页面的 page 结构体, 而 [`check_new_pages`](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L2253) 则需要检查所有的页面.


### 3.4.2   rmqueue_pcplist
-------

### 3.4.3   ` __rmqueue`
-------


### 3.4.4   `__rmqueue_smallest`
-------


### 3.4.5 __rmqueue_fallback