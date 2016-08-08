asmlinkage __visible void __init start_kernel(void)
{
    char *command_line;
    char *after_dashes;

    set_task_stack_end_magic(&init_task);
    smp_setup_processor_id();
    debug_objects_early_init();

    /*
     * Set up the the initial canary ASAP:
     */
    boot_init_stack_canary();

    cgroup_init_early();

    local_irq_disable();
    early_boot_irqs_disabled = true;

/*
 * Interrupts are still disabled. Do necessary setups, then
 * enable them
 */
    boot_cpu_init();
    page_address_init();
    pr_notice("%s", linux_banner);
    setup_arch(&command_line);
    mm_init_cpumask(&init_mm);
    setup_command_line(command_line);
    setup_nr_cpu_ids();
    setup_per_cpu_areas();
    boot_cpu_state_init();
    smp_prepare_boot_cpu(); /* arch-specific boot-cpu hooks */

    build_all_zonelists(NULL, NULL);
    page_alloc_init();

    pr_notice("Kernel command line: %s\n", boot_command_line);
    parse_early_param();
    after_dashes = parse_args("Booting kernel",
                  static_command_line, __start___param,
                  __stop___param - __start___param,
                  -1, -1, NULL, &unknown_bootoption);
    if (!IS_ERR_OR_NULL(after_dashes))
        parse_args("Setting init args", after_dashes, NULL, 0, -1, -1,
               NULL, set_init_arg);

    jump_label_init();

    /*
     * These use large bootmem allocations and must precede
     * kmem_cache_init()
     */
    setup_log_buf(0);
    pidhash_init();
    vfs_caches_init_early();
    sort_main_extable();
    trap_init();
    mm_init();

    /*
     * Set up the scheduler prior starting any interrupts (such as the
     * timer interrupt). Full topology setup happens at smp_init()
     * time - but meanwhile we still have a functioning scheduler.
     */
    sched_init();
    /*
     * Disable preemption - early bootup scheduling is extremely
     * fragile until we cpu_idle() for the first time.
     */
    preempt_disable();
    if (WARN(!irqs_disabled(),
         "Interrupts were enabled *very* early, fixing it\n"))
        local_irq_disable();
    idr_init_cache();
    rcu_init();

    /* trace_printk() and trace points may be used after this */
    trace_init();

    context_tracking_init();
    radix_tree_init();
    /* init some links before init_ISA_irqs() */
    early_irq_init();
    init_IRQ();
    tick_init();
    rcu_init_nohz();
    init_timers();
    hrtimers_init();
    softirq_init();
    timekeeping_init();
    time_init();
    sched_clock_postinit();
    printk_nmi_init();
    perf_event_init();
    profile_init();
    call_function_init();
    WARN(!irqs_disabled(), "Interrupts were enabled early\n");
    early_boot_irqs_disabled = false;
    local_irq_enable();

    kmem_cache_init_late();

    /*
     * HACK ALERT! This is early. We're enabling the console before
     * we've done PCI setups etc, and console_init() must be aware of
     * this. But we do want output early, in case something goes wrong.
     */
    console_init();
    if (panic_later)
        panic("Too many boot %s vars at `%s'", panic_later,
              panic_param);

    lockdep_info();

    /*
     * Need to run this when irqs are enabled, because it wants
     * to self-test [hard/soft]-irqs on/off lock inversion bugs
     * too:
     */
    locking_selftest();

#ifdef CONFIG_BLK_DEV_INITRD
    if (initrd_start && !initrd_below_start_ok &&
        page_to_pfn(virt_to_page((void *)initrd_start)) < min_low_pfn) {
        pr_crit("initrd overwritten (0x%08lx < 0x%08lx) - disabling it.\n",
            page_to_pfn(virt_to_page((void *)initrd_start)),
            min_low_pfn);
        initrd_start = 0;
    }
#endif
    page_ext_init();
    debug_objects_mem_init();
    kmemleak_init();
    setup_per_cpu_pageset();
    numa_policy_init();
    if (late_time_init)
        late_time_init();
    sched_clock_init();
    calibrate_delay();
    pidmap_init();
    anon_vma_init();
    acpi_early_init();
#ifdef CONFIG_X86
    if (efi_enabled(EFI_RUNTIME_SERVICES))
        efi_enter_virtual_mode();
#endif
#ifdef CONFIG_X86_ESPFIX64
    /* Should be run before the first non-init thread is created */
    init_espfix_bsp();
#endif
    thread_stack_cache_init();
    cred_init();
    fork_init();
    proc_caches_init();
    buffer_init();
    key_init();
    security_init();
    dbg_late_init();
    vfs_caches_init();
    signals_init();
    /* rootfs populating might need page-writeback */
    page_writeback_init();
    proc_root_init();
    nsfs_init();
    cpuset_init();
    cgroup_init();
    taskstats_init_early();
    delayacct_init();

    check_bugs();

    acpi_subsystem_init();
    sfi_init_late();

    if (efi_enabled(EFI_RUNTIME_SERVICES)) {
        efi_late_init();
        efi_free_boot_services();
    }

    ftrace_init();

    /* Do the rest non-__init'ed, we're now alive */
    rest_init();
}