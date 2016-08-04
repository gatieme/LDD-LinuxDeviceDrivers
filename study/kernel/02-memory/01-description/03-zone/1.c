struct per_cpu_pages {
       int count;          /* number of pages in the list */
       int high;           /* high watermark, emptying needed */
       int batch;          /* chunk size for buddy add/remove */

       /* Lists of pages, one per migrate type stored on the pcp-lists */
       struct list_head lists[MIGRATE_PCPTYPES];
};

struct per_cpu_pageset {
       struct per_cpu_pages pcp;
#ifdef CONFIG_NUMA
       s8 expire;
#endif
#ifdef CONFIG_SMP
       s8 stat_threshold;
       s8 vm_stat_diff[NR_VM_ZONE_STAT_ITEMS];
#endif
};