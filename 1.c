void __init bootmem_init(void)
{
    unsigned long min, max;

    min = PFN_UP(memblock_start_of_DRAM());
    max = PFN_DOWN(memblock_end_of_DRAM());

    early_memtest(min << PAGE_SHIFT, max << PAGE_SHIFT);

    max_pfn = max_low_pfn = max;

    arm64_numa_init();
    /*
     * Sparsemem tries to allocate bootmem in memory_present(), so must be
     * done after the fixed reservations.
     */
    arm64_memory_present();

    sparse_init();
    zone_sizes_init(min, max);

    high_memory = __va((max << PAGE_SHIFT) - 1) + 1;
    memblock_dump_all();