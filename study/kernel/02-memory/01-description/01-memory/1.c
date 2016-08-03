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