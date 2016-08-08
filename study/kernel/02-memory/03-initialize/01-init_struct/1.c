
/*
 * http://lxr.free-electrons.com/source/init/main.c?v=4.7#L479
*/
asmlinkage __visible void __init start_kernel(void)
{

    setup_arch(&command_line);
    mm_init_cpumask(&init_mm);

    setup_per_cpu_areas();


    build_all_zonelists(NULL, NULL);
    page_alloc_init();


    /*
     * These use large bootmem allocations and must precede
     * mem_init();
     * kmem_cache_init();
     */
    mm_init();

    kmem_cache_init_late();

    kmemleak_init();
    setup_per_cpu_pageset();

    rest_init();
}


/*
 * http://lxr.free-electrons.com/source/mm/page_alloc.c?v4.7#L5029
 * Called with zonelists_mutex held always
 * unless system_state == SYSTEM_BOOTING.
 *
 * __ref due to (1) call of __meminit annotated setup_zone_pageset
 * [we're only called with non-NULL zone through __meminit paths] and
 * (2) call of __init annotated helper build_all_zonelists_init
 * [protected by SYSTEM_BOOTING].
 */
void __ref build_all_zonelists(pg_data_t *pgdat, struct zone *zone)
{
     /*  设定zonelist的顺序，是按节点还是按管理区排序，只对NUMA有意义  */  
    set_zonelist_order();

    if (system_state == SYSTEM_BOOTING)
    {
        build_all_zonelists_init();
    }
    else
    {
#ifdef CONFIG_MEMORY_HOTPLUG
        if (zone)
            setup_zone_pageset(zone);
#endif
        /* we have to stop all cpus to guarantee there is no user
           of zonelist */
        stop_machine(__build_all_zonelists, pgdat, NULL);
        /* cpuset refresh routine should be here */
    }

     /*  得到所有管理区可分配的空闲页面数  */ 
    vm_total_pages = nr_free_pagecache_pages();
    /*
     * Disable grouping by mobility if the number of pages in the
     * system is too low to allow the mechanism to work. It would be
     * more accurate, but expensive to check per-zone. This check is
     * made on memory-hotadd so a system can start with mobility
     * disabled and enable it later
     */
    if (vm_total_pages < (pageblock_nr_pages * MIGRATE_TYPES))
        page_group_by_mobility_disabled = 1;
    else
        page_group_by_mobility_disabled = 0;

    pr_info("Built %i zonelists in %s order, mobility grouping %s.  Total pages: %ld\n",
        nr_online_nodes,
        zonelist_order_name[current_zonelist_order],
        page_group_by_mobility_disabled ? "off" : "on",
        vm_total_pages);
#ifdef CONFIG_NUMA
    pr_info("Policy zone: %s\n", zone_names[policy_zone]);
#endif
}