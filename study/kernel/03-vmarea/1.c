```cpp
struct mm_struct {
    struct vm_area_struct *mmap;        /* list of VMAs  内存区域链表  */
    struct rb_root mm_rb;               /*  VMA形成的红黑树  */
    u32 vmacache_seqnum;           /* per-thread vmacache */
#ifdef CONFIG_MMU
    unsigned long (*get_unmapped_area) (struct file *filp,
                unsigned long addr, unsigned long len,
                unsigned long pgoff, unsigned long flags);
#endif
    unsigned long mmap_base;        /* base of mmap area mmap的基地址  */
    unsigned long mmap_legacy_base;     /* base of mmap area in bottom-up allocations */
    unsigned long task_size;        /* size of task vm space */
    unsigned long highest_vm_end;       /* highest vma end address */
    pgd_t * pgd;
    atomic_t mm_users;              /* How many users with user space? */
    atomic_t mm_count;              /* How many references to "struct mm_struct" (users count as 1) */
    atomic_long_t nr_ptes;          /* PTE page table pages */
#if CONFIG_PGTABLE_LEVELS > 2
    atomic_long_t nr_pmds;          /* PMD page table pages */
#endif
    int map_count;              /* number of VMAs */

    spinlock_t page_table_lock;         /* Protects page tables and some counters */
    struct rw_semaphore mmap_sem;

    struct list_head mmlist;        /* List of maybe swapped mm's.  These are globally strung
                         * together off init_mm.mmlist, and are protected
                         * by mmlist_lock
                         */


    unsigned long hiwater_rss;      /* High-watermark of RSS usage */
    unsigned long hiwater_vm;       /* High-water virtual memory usage */

    unsigned long total_vm;     /* Total pages mapped */
    unsigned long locked_vm;    /* Pages that have PG_mlocked set */
    unsigned long pinned_vm;    /* Refcount permanently increased */
    unsigned long data_vm;      /* VM_WRITE & ~VM_SHARED & ~VM_STACK */
    unsigned long exec_vm;      /* VM_EXEC & ~VM_WRITE & ~VM_STACK */
    unsigned long stack_vm;     /* VM_STACK */
    unsigned long def_flags;
    unsigned long start_code, end_code, start_data, end_data;
    unsigned long start_brk, brk, start_stack;
    unsigned long arg_start, arg_end, env_start, env_end;

    unsigned long saved_auxv[AT_VECTOR_SIZE]; /* for /proc/PID/auxv */

    /*
     * Special counters, in some configurations protected by the
     * page_table_lock, in other configurations by being atomic.
     */
    struct mm_rss_stat rss_stat;

    struct linux_binfmt *binfmt;

    cpumask_var_t cpu_vm_mask_var;

    /* Architecture-specific MM context */
    mm_context_t context;

    unsigned long flags; /* Must use atomic bitops to access the bits */

    struct core_state *core_state; /* coredumping support */
#ifdef CONFIG_AIO
    spinlock_t              ioctx_lock;
    struct kioctx_table __rcu       *ioctx_table;
#endif
#ifdef CONFIG_MEMCG
    /*
     * "owner" points to a task that is regarded as the canonical
     * user/owner of this mm. All of the following must be true in
     * order for it to be changed:
     *
     * current == mm->owner
     * current->mm != mm
     * new_owner->mm == mm
     * new_owner->alloc_lock is held
     */
    struct task_struct __rcu *owner;
#endif

    /* store ref to file /proc/<pid>/exe symlink points to */
    struct file __rcu *exe_file;
#ifdef CONFIG_MMU_NOTIFIER
    struct mmu_notifier_mm *mmu_notifier_mm;
#endif
#if defined(CONFIG_TRANSPARENT_HUGEPAGE) && !USE_SPLIT_PMD_PTLOCKS
    pgtable_t pmd_huge_pte; /* protected by page_table_lock */
#endif
#ifdef CONFIG_CPUMASK_OFFSTACK
    struct cpumask cpumask_allocation;
#endif
#ifdef CONFIG_NUMA_BALANCING
    /*
     * numa_next_scan is the next time that the PTEs will be marked
     * pte_numa. NUMA hinting faults will gather statistics and migrate
     * pages to new nodes if necessary.
     */
    unsigned long numa_next_scan;

    /* Restart point for scanning and setting pte_numa */
    unsigned long numa_scan_offset;

    /* numa_scan_seq prevents two threads setting pte_numa */
    int numa_scan_seq;
#endif
#if defined(CONFIG_NUMA_BALANCING) || defined(CONFIG_COMPACTION)
    /*
     * An operation with batched TLB flushing is going on. Anything that
     * can move process memory needs to flush the TLB when moving a
     * PROT_NONE or PROT_NUMA mapped page.
     */
    bool tlb_flush_pending;
#endif
    struct uprobes_state uprobes_state;
#ifdef CONFIG_X86_INTEL_MPX
    /* address of the bounds directory */
    void __user *bd_addr;
#endif
#ifdef CONFIG_HUGETLB_PAGE
    atomic_long_t hugetlb_usage;
#endif
#ifdef CONFIG_MMU
    struct work_struct async_put_work;
#endif
};
```