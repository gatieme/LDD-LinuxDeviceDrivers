void __init bootmem_init(void)
{
    unsigned long min, max_low, max_high;

    memblock_allow_resize();
    max_low = max_high = 0;

    find_limits(&min, &max_low, &max_high);

    early_memtest((phys_addr_t)min << PAGE_SHIFT,
              (phys_addr_t)max_low << PAGE_SHIFT);

    /*
     * Sparsemem tries to allocate bootmem in memory_present(),
     * so must be done after the fixed reservations
     */
    arm_memory_present();

    /*
     * sparse_init() needs the bootmem allocator up and running.
     */
    sparse_init();

    /*
     * Now free the memory - free_area_init_node needs
     * the sparse mem_map arrays initialized by sparse_init()
     * for memmap_init_zone(), otherwise all PFNs are invalid.
     */
    zone_sizes_init(min, max_low, max_high);

    /*
     * This doesn't seem to be used by the Linux memory manager any
     * more, but is used by ll_rw_block.  If we can get rid of it, we
     * also get rid of some of the stuff above as well.
     */
    min_low_pfn = min;
    max_low_pfn = max_low;
    max_pfn = max_high;
}