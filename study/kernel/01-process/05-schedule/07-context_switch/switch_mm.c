//  http://lxr.free-electrons.com/source/arch/x86/include/asm/mmu_context.h?v=4.6#L118
static inline void switch_mm(struct mm_struct *prev, struct mm_struct *next,
                 struct task_struct *tsk)
{
    unsigned cpu = smp_processor_id();

    if (likely(prev != next))
    {
#ifdef CONFIG_SMP
        this_cpu_write(cpu_tlbstate.state, TLBSTATE_OK);
        this_cpu_write(cpu_tlbstate.active_mm, next);
#endif
        //  
        cpumask_set_cpu(cpu, mm_cpumask(next));

        /*
         * Re-load page tables.
         *
         * This logic has an ordering constraint:
         *
         *  CPU 0: Write to a PTE for 'next'
         *  CPU 0: load bit 1 in mm_cpumask.  if nonzero, send IPI.
         *  CPU 1: set bit 1 in next's mm_cpumask
         *  CPU 1: load from the PTE that CPU 0 writes (implicit)
         *
         * We need to prevent an outcome in which CPU 1 observes
         * the new PTE value and CPU 0 observes bit 1 clear in
         * mm_cpumask.  (If that occurs, then the IPI will never
         * be sent, and CPU 0's TLB will contain a stale entry.)
         *
         * The bad outcome can occur if either CPU's load is
         * reordered before that CPU's store, so both CPUs must
         * execute full barriers to prevent this from happening.
         *
         * Thus, switch_mm needs a full barrier between the
         * store to mm_cpumask and any operation that could load
         * from next->pgd.  TLB fills are special and can happen
         * due to instruction fetches or for no reason at all,
         * and neither LOCK nor MFENCE orders them.
         * Fortunately, load_cr3() is serializing and gives the
         * ordering guarantee we need.
         *
         */
        load_cr3(next->pgd);

        trace_tlb_flush(TLB_FLUSH_ON_TASK_SWITCH, TLB_FLUSH_ALL);

        /* Stop flush ipis for the previous mm */
        cpumask_clear_cpu(cpu, mm_cpumask(prev));

        /* Load per-mm CR4 state */
        load_mm_cr4(next);

#ifdef CONFIG_MODIFY_LDT_SYSCALL
        /*
         * Load the LDT, if the LDT is different.
         *
         * It's possible that prev->context.ldt doesn't match
         * the LDT register.  This can happen if leave_mm(prev)
         * was called and then modify_ldt changed
         * prev->context.ldt but suppressed an IPI to this CPU.
         * In this case, prev->context.ldt != NULL, because we
         * never set context.ldt to NULL while the mm still
         * exists.  That means that next->context.ldt !=
         * prev->context.ldt, because mms never share an LDT.
         */
        if (unlikely(prev->context.ldt != next->context.ldt))
            load_mm_ldt(next);
#endif
    }
#ifdef CONFIG_SMP
      else {
        this_cpu_write(cpu_tlbstate.state, TLBSTATE_OK);
        BUG_ON(this_cpu_read(cpu_tlbstate.active_mm) != next);

        if (!cpumask_test_cpu(cpu, mm_cpumask(next))) {
            /*
             * On established mms, the mm_cpumask is only changed
             * from irq context, from ptep_clear_flush() while in
             * lazy tlb mode, and here. Irqs are blocked during
             * schedule, protecting us from simultaneous changes.
             */
            cpumask_set_cpu(cpu, mm_cpumask(next));

            /*
             * We were in lazy tlb mode and leave_mm disabled
             * tlb flush IPI delivery. We must reload CR3
             * to make sure to use no freed page tables.
             *
             * As above, load_cr3() is serializing and orders TLB
             * fills with respect to the mm_cpumask write.
             */
            load_cr3(next->pgd);
            trace_tlb_flush(TLB_FLUSH_ON_TASK_SWITCH, TLB_FLUSH_ALL);
            load_mm_cr4(next);
            load_mm_ldt(next);
        }
    }
#endif
}


//  http://lxr.free-electrons.com/source/arch/arm/include/asm/mmu_context.h?v=4.6#L126
/*
 * This is the actual mm switch as far as the scheduler
 * is concerned.  No registers are touched.  We avoid
 * calling the CPU specific function when the mm hasn't
 * actually changed.
 */
static inline void
switch_mm(struct mm_struct *prev, struct mm_struct *next,
      struct task_struct *tsk)
{
#ifdef CONFIG_MMU
    unsigned int cpu = smp_processor_id();

    /*
     * __sync_icache_dcache doesn't broadcast the I-cache invalidation,
     * so check for possible thread migration and invalidate the I-cache
     * if we're new to this CPU.
     */
    if (cache_ops_need_broadcast() &&
        !cpumask_empty(mm_cpumask(next)) &&
        !cpumask_test_cpu(cpu, mm_cpumask(next)))
        __flush_icache_all();

    if (!cpumask_test_and_set_cpu(cpu, mm_cpumask(next)) || prev != next) {
        check_and_switch_context(next, tsk);
        if (cache_is_vivt())
            cpumask_clear_cpu(cpu, mm_cpumask(prev));
    }
#endif
}



//  http://lxr.free-electrons.com/source/arch/arm64/include/asm/mmu_context.h?v=4.6#L183

/*
 * This is the actual mm switch as far as the scheduler
 * is concerned.  No registers are touched.  We avoid
 * calling the CPU specific function when the mm hasn't
 * actually changed.
 */
static inline void
switch_mm(struct mm_struct *prev, struct mm_struct *next,
      struct task_struct *tsk)
{
    unsigned int cpu = smp_processor_id();

    if (prev == next)
        return;

    /*
     * init_mm.pgd does not contain any user mappings and it is always
     * active for kernel addresses in TTBR1. Just set the reserved TTBR0.
     */
    if (next == &init_mm) {
        cpu_set_reserved_ttbr0();
        return;
    }

    check_and_switch_context(next, cpu);
}