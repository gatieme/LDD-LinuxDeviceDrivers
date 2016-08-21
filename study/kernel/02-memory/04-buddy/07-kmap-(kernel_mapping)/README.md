服务器体系与共享存储器架构
=======

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6225543) |




在内核初始化完成之后, 内存管理的责任就由伙伴系统来承担. 伙伴系统基于一种相对简单然而令人吃惊的强大算法.

Linux内核使用二进制伙伴算法来管理和分配物理内存页面, 该算法由Knowlton设计, 后来Knuth又进行了更深刻的描述.

伙伴系统是一个结合了2的方幂个分配器和空闲缓冲区合并计技术的内存分配方案, 其基本思想很简单. 内存被分成含有很多页面的大块, 每一块都是2个页面大小的方幂. 如果找不到想要的块, 一个大块会被分成两部分, 这两部分彼此就成为伙伴. 其中一半被用来分配, 而另一半则空闲. 这些块在以后分配的过程中会继续被二分直至产生一个所需大小的块. 当一个块被最终释放时, 其伙伴将被检测出来, 如果伙伴也空闲则合并两者.

*	内核如何记住哪些内存块是空闲的

*	分配空闲页面的方法

*	影响分配器行为的众多标识位

*	内存碎片的问题和分配器如何处理碎片


#1	高端内存与内核映射
-------


尽管`vmalloc`函数族可用于从高端内存域向内核映射页帧(这些在内核空间中通常是无法直接看到的), 但这并不是这些函数的实际用途.

重要的是强调以下事实 : 内核提供了其他函数用于将`ZONE_HIGHMEM`页帧显式映射到内核空间, 这些函数与vmalloc机制无关. 因此, 这就造成了混乱.


而在高端内存的页不能永久地映射到内核地址空间. 因此, 通过alloc_pages()函数以\__GFP_HIGHMEM标志获得的内存页就不可能有逻辑地址.

在x86_32体系结构总, 高于896MB的所有物理内存的范围大都是高端内存, 它并不会永久地或自动映射到内核地址空间, 尽管X86处理器能够寻址物理RAM的范围达到4GB(启用PAE可以寻址64GB), 一旦这些页被分配, 就必须映射到内核的逻辑地址空间上. 在x86_32上, 高端地址的页被映射到内核地址空间(即虚拟地址空间的3GB~4GB)

内核地址空间的最后128 MiB用于何种用途呢?

该部分有3个用途。

1.	虚拟内存中连续、但物理内存中不连续的内存区，可以在vmalloc区域分配. 该机制通常用于用户过程, 内核自身会试图尽力避免非连续的物理地址。内核通常会成功，因为大部分大的内存块都在启动时分配给内核，那时内存的碎片尚不严重。但在已经运行了很长时间的系统上, 在内核需要物理内存时, 就可能出现可用空间不连续的情况. 此类情况, 主要出现在动态加载模块时.

2.	持久映射用于将高端内存域中的非持久页映射到内核中

3.	固定映射是与物理地址空间中的固定页关联的虚拟地址空间项，但具体关联的页帧可以自由选择. 它与通过固定公式与物理内存关联的直接映射页相反，虚拟固定映射地址与物理内存位置之间的关联可以自行定义，关联建立后内核总是会注意到的.

![x86_32上的地址划分](../images/x86_32_mapping.png)

在这里有两个预处理器符号很重要 \__VMALLOC_RESERVE设置了`vmalloc`区域的长度, 而`MAXMEM`则表示内核可以直接寻址的物理内存的最大可能数量.

![内核虚拟地址空间](../images/kernel_space.jpg)


内核中, 将内存划分为各个区域是通过图3-15所示的各个常数控制的。根据内核和系统配置, 这些常数可能有不同的值。直接映射的边界由high_memory指定。




1.	直接映射区
	线性空间中从3G开始最大896M的区间, 为直接内存映射区，该区域的线性地址和物理地址存在线性转换关系：线性地址=3G+物理地址。

2.	动态内存映射区
	该区域由内核函数`vmalloc`来分配, 特点是 : 线性空间连续, 但是对应的物理空间不一定连续. `vmalloc`分配的线性地址所对应的物理页可能处于低端内存, 也可能处于高端内存.

3.	永久内存映射区
	该区域可访问高端内存. 访问方法是使用`alloc_page(_GFP_HIGHMEM)`分配高端内存页或者使用`kmap`函数将分配到的高端内存映射到该区域.

4.	固定映射区
	该区域和4G的顶端只有4k的隔离带，其每个地址项都服务于特定的用途，如ACPI_BASE等。

>说明
>
>注意用户空间当然可以使用高端内存，而且是正常的使用，内核在分配那些不经常使用的内存时，都用高端内存空间(如果有)，所谓不经常使用是相对来说的，比如内核的一些数据结构就属于经常使用的，而用户的一些数据就属于不经常使用的。用户在启动一个应用程序时，是需要内存的，而每个应用程序都有3G的线性地址，给这些地址映射页表时就可以直接使用高端内存。
>
>而且还要纠正一点的是：那128M线性地址不仅仅是用在这些地方的，如果你要加载一个设备，而这个设备需要映射其内存到内核中，它也需要使用这段线性地址空间来完成，否则内核就不能访问设备上的内存空间了.
>
>总之，内核的高端线性地址是为了访问内核固定映射以外的内存资源。进程在使用内存时，触发缺页异常，具体将哪些物理页映射给用户进程是内核考虑的事情. 在用户空间中没有高端内存这个概念.


#2	持久内核映射
-------


如果需要将高端页帧长期映射(作为持久映射)到内核地址空间中, 必须使用kmap函数. 需要映射的页用指向page的指针指定，作为该函数的参数。该函数在有必要时创建一个映射(即，如果该页确实是高端页), 并返回数据的地址.

如果没有启用高端支持, 该函数的任务就比较简单. 在这种情况下, 所有页都可以直接访问, 因此只需要返回页的地址, 无需显式创建一个映射.

如果确实存在高端页, 情况会比较复杂. 类似于vmalloc, 内核首先必须建立高端页和所映射到的地址之间的关联. 还必须在虚拟地址空间中分配一个区域以映射页帧, 最后, 内核必须记录该虚拟区域的哪些部分在使用中, 哪些仍然是空闲的.


##2.1	数据结构
-------


核在IA-32平台上在`vmalloc`区域之后分配了一个区域, 从`PKMAP_BASE`到`FIXADDR_START`. 该区域用于持久映射. 不同体系结构使用的方案是类似的.

```cpp
// http://lxr.free-electrons.com/source/mm/highmem.c?v=4.7#L126
static int pkmap_count[LAST_PKMAP];
```
`pkmap_count`(在[mm/highmem.c?v=4.7, line 126](http://lxr.free-electrons.com/source/mm/highmem.c?v=4.7#L126)定义)是一容量为`LAST_PKMAP`的整数数组, 其中每个元素都对应于一个持久映射页。它实际上是被映射页的一个使用计数器，语义不太常见.

该计数器计算了内核使用该页的次数加1. 如果计数器值为2, 则内核中只有一处使用该映射页. 计数器值为5表示有4处使用. 一般地说，计数器值为n代表内核中有n-1处使用该页. 和通常的使用计数器一样, 0意味着相关的页没有使用.计数器值1有特殊语义. 这表示该位置关联的页已经映射, 但由于CPU的TLB没有更新而无法使用, 此时访问该页, 或者失败, 或者会访问到一个不正确的地址

内核利用下列数据结构, 来建立物理内存页的page实例与其在虚似内存区中位置之间的关联

```cpp
/*
 * Describes one page->virtual association
 */
struct page_address_map
{
	struct page *page;
	void *virtual;
	struct list_head list;
};
```
该结构用于建立`page-->virtual`的映射(该结构由此得名). 



| 字段 | 描述 |
|:-----:|:-----:|
| page  | 是一个指向全局`mem_map`数组中的`page`实例的指针 |
| virtual | 指定了该页在内核虚拟地址空间中分配的位置 |


为便于组织, 映射保存在散列表中, 结构中的链表元素用于建立溢出链表,以处理散列碰撞. 该散列表通过`page_address_htable`数组实现, 定义在[mm/highmem.c?v=4.7, line 392](http://lxr.free-electrons.com/source/mm/highmem.c?v=4.7#L392)



```cpp
/*
 * Hash table bucket
 */
static struct page_address_slot {
    struct list_head lh;            /* List of page_address_maps */
    spinlock_t lock;            /* Protect this bucket's list */
} ____cacheline_aligned_in_smp page_address_htable[1<<PA_HASH_ORDER];
```


散列表的散列函数是page_slot函数, 定义在[mm/highmem.c?v=4.7, line 397](http://lxr.free-electrons.com/source/mm/highmem.c?v=4.7#L397)

```cpp
static struct page_address_slot *page_slot(const struct page *page)
{
    return &page_address_htable[hash_ptr(page, PA_HASH_ORDER)];
}
```

##2.2	page_address函数
-------

`page_address`是一个前端函数, 使用上述数据结构确定给定page实例的地址, 该函数定义在[mm/highmem.c?v=4.7, line 408)](http://lxr.free-electrons.com/source/mm/highmem.c?v=4.7#L408)


```cpp
/**
 * page_address - get the mapped virtual address of a page
 * @page: &struct page to get the virtual address of
 *
 * Returns the page's virtual address.
 */
void *page_address(const struct page *page)
{
    unsigned long flags;
    void *ret;
    struct page_address_slot *pas;

    if (!PageHighMem(page))
        return lowmem_page_address(page);

    pas = page_slot(page);
    ret = NULL;
    spin_lock_irqsave(&pas->lock, flags);
    if (!list_empty(&pas->lh)) {
        struct page_address_map *pam;

        list_for_each_entry(pam, &pas->lh, list) {
            if (pam->page == page) {
                ret = pam->virtual;
                goto done;
            }
        }
    }
done:
    spin_unlock_irqrestore(&pas->lock, flags);
    return ret;
}

EXPORT_SYMBOL(page_address);
```



`page_address`首先检查传递进来的`page`实例在普通内存还是在高端内存.

*	如果是前者(普通内存区域), 页地址可以根据`page`在`mem_map`数组中的位置计算. 这个工作可以通过[lowmem_page_address](http://lxr.free-electrons.com/source/include/linux/mm.h?v=4.7#L964)调用[page_to_virt(page)](http://lxr.free-electrons.com/ident?v=4.7;i=page_to_virt)来完成 


*	对于后者, 可通过上述散列表查找虚拟地址.


##2.3	kmap创建映射
-------


###2.3.1	kmap函数
-------

为通过`page`指针建立映射, 必须使用`kmap`函数.

不同体系结构的定义可能不同, 但是大多数体系结构的定义都如下所示, 比如arm上该函数定义在[arch/arm/mm/highmem.c?v=4.7, line 37](http://lxr.free-electrons.com/source/arch/arm/mm/highmem.c?v=4.7#L37), 如下所示

```cpp
void *kmap(struct page *page)
{
    might_sleep();
    if (!PageHighMem(page))
        return page_address(page);
    return kmap_high(page);
}
EXPORT_SYMBOL(kmap);
```


`kmap`函数只是一个`page_address`的前端，用于确认指定的页是否确实在高端内存域中. 否则, 结果返回`page_address`得到的地址. 如果确实在高端内存中, 则内核将工作委托给`kmap_high`


`kmap_high`的实现在函数[mm/highmem.c?v=4.7, line 275](http://lxr.free-electrons.com/source/mm/highmem.c?v=4.7#L275)中, 定义如下


###2.3.2	kmap_high函数
-------


```cpp
/**
 * kmap_high - map a highmem page into memory
 * @page: &struct page to map
 *
 * Returns the page's virtual memory address.
 *
 * We cannot call this from interrupts, as it may block.
 */
void *kmap_high(struct page *page)
{
    unsigned long vaddr;

    /*
     * For highmem pages, we can't trust "virtual" until
     * after we have the lock.
     */
    lock_kmap();
    vaddr = (unsigned long)page_address(page);
    if (!vaddr)
        vaddr = map_new_virtual(page);
    pkmap_count[PKMAP_NR(vaddr)]++;
    BUG_ON(pkmap_count[PKMAP_NR(vaddr)] < 2);
    unlock_kmap();
    return (void*) vaddr;
}

EXPORT_SYMBOL(kmap_high);
```



###2.3.3	map_new_virtual函数
-------

上文讨论的`page_address`函数首先检查该页是否已经映射. 如果它不对应到有效地址, 则必须使用`map_new_virtual`映射该页.

该函数定义在[mm/highmem.c?v=4.7, line 213](http://lxr.free-electrons.com/source/mm/highmem.c?v=4.7#L213), 将执行下列主要的步骤.

```cpp
static inline unsigned long map_new_virtual(struct page *page)
{
    unsigned long vaddr;
    int count;
    unsigned int last_pkmap_nr;
    unsigned int color = get_pkmap_color(page);

start:
    count = get_pkmap_entries_count(color);
    /* Find an empty entry */
    for (;;) {
        last_pkmap_nr = get_next_pkmap_nr(color);
        if (no_more_pkmaps(last_pkmap_nr, color)) {
            flush_all_zero_pkmaps();
            count = get_pkmap_entries_count(color);
        }
        if (!pkmap_count[last_pkmap_nr])
            break;  /* Found a usable entry */
        if (--count)
            continue;

        /*
         * Sleep for somebody else to unmap their entries
         */
        {
            DECLARE_WAITQUEUE(wait, current);
            wait_queue_head_t *pkmap_map_wait =
                get_pkmap_wait_queue_head(color);

            __set_current_state(TASK_UNINTERRUPTIBLE);
            add_wait_queue(pkmap_map_wait, &wait);
            unlock_kmap();
            schedule();
            remove_wait_queue(pkmap_map_wait, &wait);
            lock_kmap();

            /* Somebody else might have mapped it while we slept */
            if (page_address(page))
                return (unsigned long)page_address(page);

            /* Re-start */
            goto start;
        }
    }
    vaddr = PKMAP_ADDR(last_pkmap_nr);
    set_pte_at(&init_mm, vaddr,
           &(pkmap_page_table[last_pkmap_nr]), mk_pte(page, kmap_prot));

    pkmap_count[last_pkmap_nr] = 1;
    set_page_address(page, (void *)vaddr);

    return vaddr;
}
```

1.	从最后使用的位置（保存在全局变量last_pkmap_nr中）开始，反向扫描pkmap_count数组, 直至找到一个空闲位置. 如果没有空闲位置，该函数进入睡眠状态，直至内核的另一部分执行解除映射操作腾出空位. 在到达`pkmap_count`的最大索引值时,  搜索从位置0开始. 在这种情况下,  还调用
`flush_all_zero_pkmaps`函数刷出CPU高速缓存（读者稍后会看到这一点）。

2.	修改内核的页表，将该页映射在指定位置。但尚未更新TLB.

3.	新位置的使用计数器设置为1。如上所述，这意味着该页已分配但无法使用，因为TLB项未更新.

4.	set_page_address将该页添加到持久内核映射的数据结构。
该函数返回新映射页的虚拟地址. 在不需要高端内存页的体系结构上（或没有设置CONFIG_HIGHMEM），则使用通用版本的kmap返回页的地址，且不修改虚拟内存


##2.4	kunmap解除映射
-------


用`kmap`映射的页, 如果不再需要, 必须用`kunmap`解除映射. 照例, 该函数首先检查相关的页(由`page`实例标识)是否确实在高端内存中. 倘若如此, 则实际工作委托给`mm/highmem.c`中的`kunmap_high`, 该函数的主要任务是将`pkmap_count`数组中对应位置在计数器减1


该机制永远不能将计数器值降低到小于1. 这意味着相关的页没有释放。因为对使用计数器进行了额外的加1操作, 正如前文的讨论, 这是为确保CPU高速缓存的正确处理. 

也在上文提到的`flush_all_zero_pkmaps`是最终释放映射的关键. 在`map_new_virtual`从头开始搜索空闲位置时, 总是调用该函数. 

它负责以下3个操作。

1.	`flush_cache_kmaps`在内核映射上执行刷出(在需要显式刷出的大多数体系结构上，将使用`flush_cache_all`刷出CPU的全部的高速缓存), 因为内核的全局页表已经修改.

2.	扫描整个`pkmap_count`数组. 计数器值为1的项设置为0，从页表删除相关的项, 最后删除该映射。

3.	最后, 使用`flush_tlb_kernel_range`函数刷出所有与`PKMAP`区域相关的`TLB`项.


###2.4.1	kunmap函数
-------


同kmap类似, 每个体系结构都应该实现自己的kmap函数, 大多数体系结构的定义都如下所示, 参见[arch/arm/mm/highmem.c?v=4.7, line 46](http://lxr.free-electrons.com/source/arch/arm/mm/highmem.c?v=4.7#L46)

```cpp
void kunmap(struct page *page)
{
    BUG_ON(in_interrupt());
    if (!PageHighMem(page))
        return;
    kunmap_high(page);
}
EXPORT_SYMBOL(kunmap);
```


内核首先检查待释放内存区域是不是在高端内存区域

*	如果内存区域在普通内存区, 则内核并没有通过kmap_high对其建立持久的内核映射, 当然也无需用kunmap_high释放

*	如果内存区域在高端内存区, 则内核通过kunmap_high释放该内存空间



###2.4.2	kunmap_high函数
-------

kunmap_high函数定义在[mm/highmem.c?v=4.7, line 328](http://lxr.free-electrons.com/source/mm/highmem.c?v=4.7#L328)

```cpp
#ifdef CONFIG_HIGHMEM
/**
 * kunmap_high - unmap a highmem page into memory
 * @page: &struct page to unmap
 *
 * If ARCH_NEEDS_KMAP_HIGH_GET is not defined then this may be called
 * only from user context.
 */
void kunmap_high(struct page *page)
{
    unsigned long vaddr;
    unsigned long nr;
    unsigned long flags;
    int need_wakeup;
    unsigned int color = get_pkmap_color(page);
    wait_queue_head_t *pkmap_map_wait;

    lock_kmap_any(flags);
    vaddr = (unsigned long)page_address(page);
    BUG_ON(!vaddr);
    nr = PKMAP_NR(vaddr);

    /*
     * A count must never go down to zero
     * without a TLB flush!
     */
    need_wakeup = 0;
    switch (--pkmap_count[nr]) {
    case 0:
        BUG();
    case 1:
        /*
         * Avoid an unnecessary wake_up() function call.
         * The common case is pkmap_count[] == 1, but
         * no waiters.
         * The tasks queued in the wait-queue are guarded
         * by both the lock in the wait-queue-head and by
         * the kmap_lock.  As the kmap_lock is held here,
         * no need for the wait-queue-head's lock.  Simply
         * test if the queue is empty.
         */
        pkmap_map_wait = get_pkmap_wait_queue_head(color);
        need_wakeup = waitqueue_active(pkmap_map_wait);
    }
    unlock_kmap_any(flags);

    /* do wake-up, if needed, race-free outside of the spin lock */
    if (need_wakeup)
        wake_up(pkmap_map_wait);
}

EXPORT_SYMBOL(kunmap_high);
#endif
```



#3	临时内核映射
-------


刚才描述的`kmap`函数不能用于中断处理程序, 因为它可能进入睡眠状态. 如果`pkmap`数组中没有空闲位置, 该函数会进入睡眠状态, 直至情形有所改善. 

因此内核提供了一个备选的映射函数, 其执行是原子的, 逻辑上称为`kmap_atomic`. 该函数的一个主要优点是它比普通的`kmap`快速. 但它不能用
于可能进入睡眠的代码. 因此, 它对于很快就需要一个临时页的简短代码，是非常理想的.

`kmap_atomic`的定义在IA-32, PPC, Sparc32上是[特定于体系结构的](http://lxr.free-electrons.com/ident?v=4.7;i=kmap_atomic), 但这3种实现只有非常细微的差别. 其原型是相同的.


##3.1	kmap_atomic函数
-------


```cpp
//  http://lxr.free-electrons.com/source/arch/arm/mm/highmem.c?v=4.7#L55
void *kmap_atomic(struct page *page)
```
page是一个指向高端内存页的管理结构的指针, 而早期的内核中, 增加了一个类型为[enum km_type](http://lxr.free-electrons.com/ident?v=2.6.32;i=km_type)的[type参数](http://lxr.free-electrons.com/source/arch/arm/mm/highmem.c?v=2.6.32#L39), 用于指定所需的映射类型

```cpp
//  http://lxr.free-electrons.com/source/arch/arm/mm/highmem.c?v=2.6.32#L39
void *kmap_atomic(struct page *page, enum km_type type)
```

而在新的内核中, 删除了这个标识, 但是保留了`km_type`的最大值`KM_TYPE_NR`


```cpp
void *kmap_atomic(struct page *page)
{
    unsigned int idx;
    unsigned long vaddr;
    void *kmap;
    int type;

    preempt_disable();
    pagefault_disable();
    if (!PageHighMem(page))
        return page_address(page);

#ifdef CONFIG_DEBUG_HIGHMEM
    /*
     * There is no cache coherency issue when non VIVT, so force the
     * dedicated kmap usage for better debugging purposes in that case.
     */
    if (!cache_is_vivt())
        kmap = NULL;
    else
#endif
        kmap = kmap_high_get(page);
    if (kmap)
        return kmap;

    type = kmap_atomic_idx_push();

    idx = FIX_KMAP_BEGIN + type + KM_TYPE_NR * smp_processor_id();
    vaddr = __fix_to_virt(idx);
#ifdef CONFIG_DEBUG_HIGHMEM
    /*
     * With debugging enabled, kunmap_atomic forces that entry to 0.
     * Make sure it was indeed properly unmapped.
     */
    BUG_ON(!pte_none(get_fixmap_pte(vaddr)));
#endif
    /*
     * When debugging is off, kunmap_atomic leaves the previous mapping
     * in place, so the contained TLB flush ensures the TLB is updated
     * with the new mapping.
     */
    set_fixmap_pte(idx, mk_pte(page, kmap_prot));

    return (void *)vaddr;
}
EXPORT_SYMBOL(kmap_atomic);
```


这个函数不会被阻塞, 因此可以用在中断上下文和起亚不能重新调度的地方. 它也禁止内核抢占, 这是有必要的, 因此映射对每个处理器都是唯一的(调度可能对哪个处理器执行哪个进程做变动).


##3.2	kunmap_atomic函数
-------

可以通过函数kunmap_atomic取消映射

```cpp
/*
 * Prevent people trying to call kunmap_atomic() as if it were kunmap()
 * kunmap_atomic() should get the return value of kmap_atomic, not the page.
 */
#define kunmap_atomic(addr)                     \
do {                                \
    BUILD_BUG_ON(__same_type((addr), struct page *));       \
    __kunmap_atomic(addr);                  \
} while (0)
```

这个函数也不会阻塞. 在很多体系结构中, 除非激活了内核抢占, 否则kunmap_atomic根本无事可做, 因为只有在下一个临时映射到来前上一个临时映射才有效. 因此, 内核完全可以"忘掉"kmap_atomic映射, kunmap_atomic也无需做什么实际的事情. 下一个原子映射将自动覆盖前一个映射.

