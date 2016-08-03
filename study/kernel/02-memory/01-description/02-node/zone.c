enum zone_type {
#ifdef CONFIG_ZONE_DMA
    /*
     * ZONE_DMA is used when there are devices that are not able
     * to do DMA to all of addressable memory (ZONE_NORMAL). Then we
     * carve out the portion of memory that is needed for these devices.
     * The range is arch specific.
     *
     * Some examples
     *
     * Architecture     Limit
     * ---------------------------
     * parisc, ia64, sparc  <4G
     * s390         <2G
     * arm          Various
     * alpha        Unlimited or 0-16MB.
     *
     * i386, x86_64 and multiple other arches
     *              <16M.
     */
    ZONE_DMA,
#endif
#ifdef CONFIG_ZONE_DMA32
    /*
     * x86_64 needs two ZONE_DMAs because it supports devices that are
     * only able to do DMA to the lower 16M but also 32 bit devices that
     * can only do DMA areas below 4G.
     */
    ZONE_DMA32,
#endif
    /*
     * Normal addressable memory is in ZONE_NORMAL. DMA operations can be
     * performed on pages in ZONE_NORMAL if the DMA devices support
     * transfers to all addressable memory.
     */
    ZONE_NORMAL,
#ifdef CONFIG_HIGHMEM
    /*
     * A memory area that is only addressable by the kernel through
     * mapping portions into its own address space. This is for example
     * used by i386 to allow the kernel to address the memory beyond
     * 900MB. The kernel will set up special mappings (page
     * table entries on i386) for each page that the kernel needs to
     * access.
     */
    ZONE_HIGHMEM,
#endif
    ZONE_MOVABLE,
#ifdef CONFIG_ZONE_DEVICE
    ZONE_DEVICE,
#endif
    __MAX_NR_ZONES

};


struct zone {
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
     */
    unsigned int inactive_ratio;

    struct pglist_data      *zone_pgdat;
    /*/这个数组用于实现每个CPU的热/冷页帧列表。内核使用这些列表来保存可用于满足实现的“新鲜”页。但冷热页帧对应的高速缓存状态不同：有些页帧很可能在高速缓存中，因此可以快速访问，故称之为热的；未缓存的页帧与此相对，称之为冷的。*/
    struct per_cpu_pageset __percpu *pageset;

    /*
     * This is a per-zone reserve of pages that are not available
     * to userspace allocations.
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

    /* zone_start_pfn == zone_start_paddr >> PAGE_SHIFT */
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
    unsigned long       spanned_pages;
    unsigned long       present_pages;

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
    wait_queue_head_t       *wait_table;
    unsigned long       wait_table_hash_nr_entries;
    unsigned long       wait_table_bits;

    ZONE_PADDING(_pad1_)
    /* free areas of different sizes */
    struct free_area    free_area[MAX_ORDER];

    /* zone flags, see below */
    unsigned long       flags;

    /* Write-intensive fields used from the page allocator */
    spinlock_t          lock;

    ZONE_PADDING(_pad2_)

    /* Write-intensive fields used by page reclaim */

    /* Fields commonly accessed by the page reclaim scanner */
    spinlock_t          lru_lock;
    struct lruvec       lruvec;

    /*
     * When free pages are below this point, additional steps are taken
     * when reading the number of free pages to avoid per-cpu counter
     * drift allowing watermarks to be breached
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
    int             compact_order_failed;
#endif

#if defined CONFIG_COMPACTION || defined CONFIG_CMA
    /* Set to true when the PG_migrate_skip bits should be cleared */
    bool            compact_blockskip_flush;
#endif

    bool            contiguous;

    ZONE_PADDING(_pad3_)
    /* Zone statistics */
    atomic_long_t       vm_stat[NR_VM_ZONE_STAT_ITEMS];
} ____cacheline_internodealigned_in_smp;