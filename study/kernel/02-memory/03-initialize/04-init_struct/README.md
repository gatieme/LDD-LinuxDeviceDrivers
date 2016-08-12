初始化内存管理
=======



| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6225543) |


在内存管理的上下文中, 初始化(initialization)可以有多种含义. 在许多CPU上, 必须显式设置适用于Linux内核的内存模型. 例如在x86_32上需要切换到保护模式, 然后奇偶内核才能检测到可用内存和寄存器.



#1	前景回顾
-------


##1.1	Linux内存管理的层次结构
-------


Linux把物理内存划分为三个层次来管理

| 层次 | 描述 |
|:----:|:----:|
| 存储节点(Node) |  CPU被划分为多个节点(node), 内存则被分簇, 每个CPU对应一个本地物理内存, 即一个CPU-node对应一个内存簇bank，即每个内存簇被认为是一个节点 |
| 管理区(Zone)   | 每个物理内存节点node被划分为多个内存管理区域, 用于表示不同范围的内存, 内核可以使用不同的映射方式映射物理内存 |
| 页面(Page) 	   |	内存被细分为多个页面帧, 页面是最基本的页面分配的单位　｜

为了支持NUMA模型，也即CPU对不同内存单元的访问时间可能不同，此时系统的物理内存被划分为几个节点(node), 一个node对应一个内存簇bank，即每个内存簇被认为是一个节点


*	首先, 内存被划分为**结点**. 每个节点关联到系统中的一个处理器, 内核中表示为`pg_data_t`的实例. 系统中每个节点被链接到一个以NULL结尾的`pgdat_list`链表中<而其中的每个节点利用`pg_data_tnode_next`字段链接到下一节．而对于PC这种UMA结构的机器来说, 只使用了一个成为contig_page_data的静态pg_data_t结构.

*	接着各个节点又被划分为内存管理区域, 一个**管理区域**通过struct zone_struct描述, 其被定义为zone_t, 用以表示内存的某个范围, 低端范围的16MB被描述为ZONE_DMA, 某些工业标准体系结构中的(ISA)设备需要用到它, 然后是可直接映射到内核的普通内存域ZONE_NORMAL,最后是超出了内核段的物理地址域ZONE_HIGHMEM, 被称为高端内存.　是系统中预留的可用内存空间, 不能被内核直接映射.


*	最后**页帧(page frame)**代表了系统内存的最小单位, 堆内存中的每个页都会创建一个struct page的一个实例. 传统上，把内存视为连续的字节，即内存为字节数组，内存单元的编号(地址)可作为字节数组的索引. 分页管理时，将若干字节视为一页，比如4K byte. 此时，内存变成了连续的页，即内存为页数组，每一页物理内存叫页帧，以页为单位对内存进行编号，该编号可作为页数组的索引，又称为页帧号.


##1.2	今日内容(启动过程中的内存初始化)
-------


在初始化过程中, 还必须建立内存管理的数据结构, 以及很多事务. 因为内核在内存管理完全初始化之前就需要使用内存. 在系统启动过程期间, 使用了额外的简化悉尼股市的内存管理模块, 然后在初始化完成后, 将旧的模块丢弃掉.


因此我们可以把linux内核的内存管理分三个阶段。

| 阶段 | 起点 | 终点 | 描述 |
|:-----:|:-----:|:-----:|
| 第一阶段 | 系统启动 | bootmem或者memblock初始化完成 | 此阶段只能使用memblock_reserve函数分配内存， 早期内核中使用init_bootmem_done = 1标识此阶段结束 |
| 第二阶段 | bootmem或者memblock初始化完 | buddy完成前 | 引导内存分配器bootmem或者memblock接受内存的管理工作, 早期内核中使用mem_init_done = 1标记此阶段的结束 |
| 第三阶段 | buddy初始化完成 | 系统停止运行 | 可以用cache和buddy分配内存 |



##1.3	start_kernel系统启动阶段的内存初始化过程
-------

首先我们来看看start_kernel是如何初始化系统的, start_kerne定义在[init/main.c?v=4.7, line 479](http://lxr.free-electrons.com/source/init/main.c?v=4.7#L479)

其代码很复杂, 我们只截取出其中与内存管理初始化相关的部分, 如下所示


```cpp
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
```


| 函数  | 功能 |
|:----:|:----:|
| [setup_arch](http://lxr.free-electrons.com/ident?v=4.7;i=setup_arch) | 是一个特定于体系结构的设置函数, 其中一项任务是负责初始化自举分配器 |
| [mm_init_cpumask](http://lxr.free-electrons.com/source/include/linux/mm_types.h?v=4.7#L522) | 初始化CPU屏蔽字 |
| [setup_per_cpu_areas](http://lxr.free-electrons.com/ident?v=4.7;i=setup_per_cpu_areas) | 函数[(查看定义)](http://lxr.free-electrons.com/source/mm/percpu.c?v4.7#L2205])给每个CPU分配内存，并拷贝.data.percpu段的数据. 为系统中的每个CPU的per_cpu变量申请空间.<br>在SMP系统中, setup_per_cpu_areas初始化源代码中(使用[per_cpu宏](http://lxr.free-electrons.com/source/include/linux/percpu-defs.h#L256))定义的静态per-cpu变量, 这种变量对系统中每个CPU都有一个独立的副本. <br>此类变量保存在内核二进制影像的一个独立的段中, setup_per_cpu_areas的目的就是为系统中各个CPU分别创建一份这些数据的副本<br>在非SMP系统中这是一个空操作 |
| [build_all_zonelists](http://lxr.free-electrons.com/source/mm/page_alloc.c?v4.7#L5029) | 建立并初始化结点和内存域的数据结构 |
| [mm_init](http://lxr.free-electrons.com/source/init/main.c?v4.7#L464) | 建立了内核的内存分配器, <br>其中通过[mem_init](http://lxr.free-electrons.com/ident?v=4.7&i=mem_init)停用bootmem分配器并迁移到实际的内存管理器(比如伙伴系统)<br>然后调用kmem_cache_init函数初始化内核内部用于小块内存区的分配器 |
| [kmem_cache_init_late](http://lxr.free-electrons.com/source/mm/slab.c?v4.7#L1378) | 在kmem_cache_init之后, 完善分配器的缓存机制,　当前3个可用的内核内存分配器[slab](http://lxr.free-electrons.com/source/mm/slab.c?v4.7#L1378), [slob](http://lxr.free-electrons.com/source/mm/slob.c?v4.7#L655), [slub](http://lxr.free-electrons.com/source/mm/slub.c?v=4.7#L3960)都会定义此函数　|
| [kmemleak_init](http://lxr.free-electrons.com/source/mm/kmemleak.c?v=4.7#L1857) | Kmemleak工作于内核态，Kmemleak 提供了一种可选的内核泄漏检测，其方法类似于跟踪内存收集器。当独立的对象没有被释放时，其报告记录在 [/sys/kernel/debug/kmemleak](http://lxr.free-electrons.com/source/mm/kmemleak.c?v=4.7#L1467)中, Kmemcheck能够帮助定位大多数内存错误的上下文 |
| [setup_per_cpu_pageset](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L5392) | 初始化CPU高速缓存行, 为pagesets的第一个数组元素分配内存, 换句话说, 其实就是第一个系统处理器分配<br>由于在分页情况下，每次存储器访问都要存取多级页表，这就大大降低了访问速度。所以，为了提高速度，在CPU中设置一个最近存取页面的高速缓存硬件机制，当进行存储器访问时，先检查要访问的页面是否在高速缓存中. |




##1.4	setup_arch函数初始化内存流程
-------


前面我们的内核从start_kernel开始, 进入setup_arch(), 并完成了早期内存分配器的初始化和设置工作.

```cpp
void __init setup_arch(char **cmdline_p)
{
	/*  初始化memblock  */
	arm64_memblock_init( );

	/*  分页机制初始化  */
	paging_init();

	bootmem_init();
}
```

| 流程 | 描述 |
|:---:|:----:|
| [arm64_memblock_init](http://lxr.free-electrons.com/source/arch/arm64/kernel/setup.c?v=4.7#L229) | 初始化memblock内存分配器 |
| [paging_init](http://lxr.free-electrons.com/source/arch/arm64/mm/mmu.c?v=4.7#L538) | 初始化分页机制 |
| [bootmem_init](http://lxr.free-electrons.com/source/arch/arm64/mm/init.c?v=4.7#L306) | 初始化内存管理 |


该函数主要执行了如下操作


1.	使用arm64_memblock_init来完成memblock机制的初始化工作, 至此memblock分配器接受系统中系统中内存的分配工作

2.	调用paging_init来完成系统分页机制的初始化工作, 建立页表, 从而内核可以完成虚拟内存的映射和转换工作

3.	最后调用bootmem_init来完成实现buddy内存管理所需要的工作



##1.5	(第一阶段)启动过程中的内存分配器
-------


在初始化过程中, 还必须建立内存管理的数据结构, 以及很多事务. 因为内核在内存管理完全初始化之前就需要使用内存. 在系统启动过程期间, 使用了额外的简化悉尼股市的内存管理模块, 然后在初始化完成后, 将旧的模块丢弃掉.

这个阶段的内存分配其实很简单, 因此我们往往称之为内存分配器(而不是内存管理器), 早期的内核中内存分配器使用的**bootmem引导分配器**, 它基于一个内存位图bitmap, 使用最优适配算法来查找内存, 但是这个分配器有很大的缺陷, 最严重的就是内存碎片的问题, 因此在后来的内核中将其舍弃《而使用了**新的memblock机制**. memblock机制的初始化在arm64上是通过[arm64_memblock_init](http://lxr.free-electrons.com/source/arch/arm64/kernel/setup.c?v=4.7#L229)函数来实现的




##1.6	今日内容(第二阶段--初始化buddy内存管理)
-------


**初始化内存分页机制**


在初始化内存的结点和内存区域之前, 内核先通过pagging_init初始化了内核的分页机制, 这样我们的虚拟运行空间就初步建立, 并可以完成物理地址到虚拟地址空间的映射工作.


在arm64架构下, 内核在start_kernel()->setup_arch()中通过arm64_memblock_init( )完成了memblock的初始化之后, 接着通过setup_arch()->paging_init()开始初始化分页机制


paging_init负责建立只能用于内核的页表, 用户空间是无法访问的. 这对管理普通应用程序和内核访问内存的方式，有深远的影响



**特定于体系结构的设置**


在完成了基础的内存结点和内存域的初始化工作以后, 我们必须克服一些硬件的特殊设置


**建立内存管理的数据结构**


对相关数据结构的初始化是从全局启动函数start_kernel中开始的, 该函数在加载内核并激活各个子系统之后执行. 由于内存管理是内核一个非常重要的部分, 因此在特定体系结构的设置步骤中检测并确定系统中内存的分配情况后, 会立即执行内存管理的初始化.



**移交早期的分配器到内存管理器**



最后我们的内存管理器已经初始化并设置完成, 可以投入运行了, 因此内核将内存管理的工作从早期的内存分配器(bootmem或者memblock)移交到我们的buddy伙伴系统.




#2	初始化前的准备工作
-------


##2.1	回到setup_arch函数(当前已经完成的工作)
-------


现在我们回到start_kernel()->setup_arch()函数


```cpp
void __init setup_arch(char **cmdline_p)
{
	/*  初始化memblock  */
	arm64_memblock_init( );

	/*  分页机制初始化  */
	paging_init();

	bootmem_init();
}
```


到目前位置我们已经完成了如下工作

*	memblock已经通过arm64_memblock_init完成了初始化, 至此系统中的内存可以通过memblock分配了

*	paging_init完成了分页机制的初始化, 至此内核已经布局了一套完整的虚拟内存空间


至此我们所有的内存都可以通过memblock机制来分配和释放, 尽管它实现的笨拙而简易, 但是已经足够我们初始化阶段使用了, 反正内核页不可能指着它过一辈子, 而我们也通过pagging_init创建了页表, 为内核提供了一套可供内核和进程运行的虚拟运行空间, 我们可以安全的进行内存的分配了

因此该是时候初始化我们强大的buddy系统了.

内核接着setup_arch()->bootmem_init()函数开始执行


##2.2	bootmem_init函数初始化内存结点和管理域
-------


arm64架构下, 在setup_arch中通过paging_init函数初始化内核分页机制之后, 内核通过`bootmem_init()`开始完成内存结点和内存区域的初始化工作, 该函数定义在[arch/arm64/mm/init.c, line 306](http://lxr.free-electrons.com/source/arch/arm64/mm/init.c?v=4.7#L306)

```cpp
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
}
```


##2.3	zone_sizes_init函数
-------


在初始化内存结点和内存域之前, 内核首先通过setup_arch()-->bootmem_init()-->zone_sizes_init()来初始化节点和管理区的一些数据项



[zone_sizes_init](http://lxr.free-electrons.com/source/arch/arm64/mm/init.c?v=4.7#L92)函数定义在[arch/arm64/mm/init.c?v=4.7, line 92](http://lxr.free-electrons.com/source/arch/arm64/mm/init.c?v=4.7#L92), 由于arm64支持NUMA和UMA两种存储器架构, 因此该函数依照NUMA和UMA, 有两种不同的实现.

```cpp
#ifdef CONFIG_NUMA

static void __init zone_sizes_init(unsigned long min, unsigned long max)
{
    unsigned long max_zone_pfns[MAX_NR_ZONES]  = {0};

    if (IS_ENABLED(CONFIG_ZONE_DMA))
        max_zone_pfns[ZONE_DMA] = PFN_DOWN(max_zone_dma_phys());
    max_zone_pfns[ZONE_NORMAL] = max;

    free_area_init_nodes(max_zone_pfns);
}

#else

static void __init zone_sizes_init(unsigned long min, unsigned long max)
{
    struct memblock_region *reg;
    unsigned long zone_size[MAX_NR_ZONES], zhole_size[MAX_NR_ZONES];
    unsigned long max_dma = min;

    memset(zone_size, 0, sizeof(zone_size));

    /* 4GB maximum for 32-bit only capable devices */
#ifdef CONFIG_ZONE_DMA
    max_dma = PFN_DOWN(arm64_dma_phys_limit);
    zone_size[ZONE_DMA] = max_dma - min;
#endif
    zone_size[ZONE_NORMAL] = max - max_dma;

    memcpy(zhole_size, zone_size, sizeof(zhole_size));

    for_each_memblock(memory, reg) {
        unsigned long start = memblock_region_memory_base_pfn(reg);
        unsigned long end = memblock_region_memory_end_pfn(reg);

        if (start >= max)
            continue;

#ifdef CONFIG_ZONE_DMA
        if (start < max_dma) {
            unsigned long dma_end = min(end, max_dma);
            zhole_size[ZONE_DMA] -= dma_end - start;
        }
#endif
        if (end > max_dma) {
            unsigned long normal_end = min(end, max);
            unsigned long normal_start = max(start, max_dma);
            zhole_size[ZONE_NORMAL] -= normal_end - normal_start;
        }
    }

    free_area_init_node(0, zone_size, min, zhole_size);
}

#endif /* CONFIG_NUMA */
```

在获取了三个管理区的页面数后，通过free_area_init_nodes()来完成后续工作, 其中核心函数为free_area_init_node(),用来针对特定的节点进行初始化

至此，节点和管理区的关键数据已完成初始化，内核在后面为内存管理做得一个准备工作就是将所有节点的管理区都链入到zonelist中，便于后面内存分配工作的进行

内核在start_kernel()-->build_all_zonelist()中完成zonelist的初始化


##2.4	free_area_init_node(s)初始化NUMA内存结点
-------

>注意
>
>此部分内容参照
>
>[Linux内存管理伙伴算法](http://www.linuxidc.com/Linux/2012-09/70711p3.htm)
>
>[linux 内存管理 - paging_init 函数](http://blog.csdn.net/decload/article/details/8080126)


[free_area_init_nodes](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L6460)初始化了NUMA系统中所有结点的pg_data_t和zone、page的数据, 并打印了管理区信息, 该函数定义在[mm/page_alloc.c?v=4.7, line 6460](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L6460)


```cpp
//  初始化各个节点的所有pg_data_t和zone、page的数据
void __init free_area_init_nodes(unsigned long *max_zone_pfn)
{
    unsigned long start_pfn, end_pfn;
    int i, nid;

    /* Record where the zone boundaries are
     * 全局数组arch_zone_lowest_possible_pfn
     * 用来存储各个内存域可使用的最低内存页帧编号   */
    memset(arch_zone_lowest_possible_pfn, 0,
                sizeof(arch_zone_lowest_possible_pfn));

    /* 全局数组arch_zone_highest_possible_pfn
     * 用来存储各个内存域可使用的最高内存页帧编号   */
	memset(arch_zone_highest_possible_pfn, 0,
                sizeof(arch_zone_highest_possible_pfn));

	/* 辅助函数find_min_pfn_with_active_regions
     * 用于找到注册的最低内存域中可用的编号最小的页帧 */
    arch_zone_lowest_possible_pfn[0] = find_min_pfn_with_active_regions();

    /*  max_zone_pfn记录了各个内存域包含的最大页帧号  */
    arch_zone_highest_possible_pfn[0] = max_zone_pfn[0];

    /*  依次遍历，确定各个内存域的边界    */
    for (i = 1; i < MAX_NR_ZONES; i++) {
    	/*  由于ZONE_MOVABLE是一个虚拟内存域
         *  不与真正的硬件内存域关联
         *  该内存域的边界总是设置为0 */
        if (i == ZONE_MOVABLE)
            continue;
        /*  第n个内存域的最小页帧
         *  即前一个（第n-1个）内存域的最大页帧  */
        arch_zone_lowest_possible_pfn[i] =
            arch_zone_highest_possible_pfn[i-1];
        /*  不出意外，当前内存域的最大页帧
         *  由max_zone_pfn给出  */
        arch_zone_highest_possible_pfn[i] =
            max(max_zone_pfn[i], arch_zone_lowest_possible_pfn[i]);
    }
    arch_zone_lowest_possible_pfn[ZONE_MOVABLE] = 0;
    arch_zone_highest_possible_pfn[ZONE_MOVABLE] = 0;

    /* Find the PFNs that ZONE_MOVABLE begins at in each node */
    memset(zone_movable_pfn, 0, sizeof(zone_movable_pfn));
    /*  用于计算进入ZONE_MOVABLE的内存数量  */
    find_zone_movable_pfns_for_nodes();

    /* Print out the zone ranges
     * 将各个内存域的最大、最小页帧号显示出来  */
    pr_info("Zone ranges:\n");
    for (i = 0; i < MAX_NR_ZONES; i++) {
        if (i == ZONE_MOVABLE)
            continue;
        pr_info("  %-8s ", zone_names[i]);
        if (arch_zone_lowest_possible_pfn[i] ==
                arch_zone_highest_possible_pfn[i])
            pr_cont("empty\n");
        else
            pr_cont("[mem %#018Lx-%#018Lx]\n",
                (u64)arch_zone_lowest_possible_pfn[i]
                    << PAGE_SHIFT,
                ((u64)arch_zone_highest_possible_pfn[i]
                    << PAGE_SHIFT) - 1);
    }

    /* Print out the PFNs ZONE_MOVABLE begins at in each node */
    pr_info("Movable zone start for each node\n");
    for (i = 0; i < MAX_NUMNODES; i++) {
    	/*  对每个结点来说，zone_movable_pfn[node_id]
         *  表示ZONE_MOVABLE在movable_zone内存域中所取得内存的起始地址
         *  内核确保这些页将用于满足符合ZONE_MOVABLE职责的内存分配 */
        if (zone_movable_pfn[i])
        {
        	/*  显示各个内存域的分配情况  */
            pr_info("  Node %d: %#018Lx\n", i,
                   (u64)zone_movable_pfn[i] << PAGE_SHIFT);
        }
    }

    /* Print out the early node map */
    pr_info("Early memory node ranges\n");
    for_each_mem_pfn_range(i, MAX_NUMNODES, &start_pfn, &end_pfn, &nid)
        pr_info("  node %3d: [mem %#018Lx-%#018Lx]\n", nid,
            (u64)start_pfn << PAGE_SHIFT,
            ((u64)end_pfn << PAGE_SHIFT) - 1);

    /* Initialise every node */
    mminit_verify_pageflags_layout();
    setup_nr_node_ids();

    /*  代码遍历所有的活动结点，
     *  并分别对各个结点调用free_area_init_node建立数据结构，
     *  该函数需要结点第一个可用的页帧作为一个参数，
     *  而find_min_pfn_for_node则从early_node_map数组提取该信息   */
    for_each_online_node(nid) {
        pg_data_t *pgdat = NODE_DATA(nid);
        free_area_init_node(nid, NULL,
                find_min_pfn_for_node(nid), NULL);

        /* Any memory on that node
         * 根据node_present_pages字段判断结点具有内存
         * 则在结点位图中设置N_HIGH_MEMORY标志
         * 该标志只表示结点上存在普通或高端内存
         * 因此check_for_regular_memory
         * 进一步检查低于ZONE_HIGHMEM的内存域中是否有内存
         * 并据此在结点位图中相应地设置N_NORMAL_MEMORY   */
        if (pgdat->node_present_pages)
            node_set_state(nid, N_MEMORY);
        check_for_memory(pgdat, nid);
    }
}
```

free_area_init_nodes函数中通过循环遍历各个节点，循环中调用了free_area_init_node函数初始化该节点对应的pg_data_t和zone、page的数据.


##2.5	free_area_init_node初始化UMA内存结点
-------

[free_area_init_nodes](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L6076)函数初始化所有结点的pg_data_t和zone、page的数据，并打印了管理区信息：

```cpp
void __paginginit free_area_init_node(int nid, unsigned long *zones_size,
        unsigned long node_start_pfn, unsigned long *zholes_size)
{
    pg_data_t *pgdat = NODE_DATA(nid);
    unsigned long start_pfn = 0;
    unsigned long end_pfn = 0;

    /* pg_data_t should be reset to zero when it's allocated */
    WARN_ON(pgdat->nr_zones || pgdat->classzone_idx);

    reset_deferred_meminit(pgdat);
    pgdat->node_id = nid;
    pgdat->node_start_pfn = node_start_pfn;
#ifdef CONFIG_HAVE_MEMBLOCK_NODE_MAP
    get_pfn_range_for_nid(nid, &start_pfn, &end_pfn);
    pr_info("Initmem setup node %d [mem %#018Lx-%#018Lx]\n", nid,
        (u64)start_pfn << PAGE_SHIFT,
        end_pfn ? ((u64)end_pfn << PAGE_SHIFT) - 1 : 0);
#else
    start_pfn = node_start_pfn;
#endif
	/*  首先累计各个内存域的页数
     *	计算结点中页的总数
     *	对连续内存模型而言
     *	这可以通过zone_sizes_init完成
     *	但calculate_node_totalpages还考虑了内存空洞 */
    calculate_node_totalpages(pgdat, start_pfn, end_pfn,
                  zones_size, zholes_size);
	/*  分配了该节点的页面描述符数组
     *  [pgdat->node_mem_map数组的内存分配  */
    alloc_node_mem_map(pgdat);
#ifdef CONFIG_FLAT_NODE_MEM_MAP
    printk(KERN_DEBUG "free_area_init_node: node %d, pgdat %08lx, node_mem_map %08lx\n",
        nid, (unsigned long)pgdat,
        (unsigned long)pgdat->node_mem_map);
#endif

    /*  对该节点的每个区[DMA,NORMAL,HIGH]的的结构进行初始化  */
    free_area_init_core(pgdat);
}
```

*	[calculate_node_totalpages](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L5789)函数累计各个内存域的页数，计算结点中页的总数。对连续内存模型而言，这可以通过zone_sizes_init完成，但calculate_node_totalpages还考虑了内存空洞,该函数定义在[mm/page_alloc.c, line 5789](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L5789)


*	[alloc_node_mem_map(pgdat)](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L6030)函数分配了该节点的页面描述符数组[pgdat->node_mem_map数组的内存分配.


*	继续调用[free_area_init_core](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L5932)函数，继续初始化该节点的pg_data_t结构，初始化zone以及page结构 ，##2.6	free_area_init_core函数是初始化zone的核心



##2.6	free_area_init_core初始化zone
-------

```cpp
/*
 * Set up the zone data structures:
 *   - mark all pages reserved
 *   - mark all memory queues empty
 *   - clear the memory bitmaps
 *
 * NOTE: pgdat should get zeroed by caller.
 */
static void __paginginit free_area_init_core(struct pglist_data *pgdat)
{
    enum zone_type j;
    int nid = pgdat->node_id;
    int ret;

    /*  初始化pgdat->node_size_lock自旋锁  */
    pgdat_resize_init(pgdat);
#ifdef CONFIG_NUMA_BALANCING
    spin_lock_init(&pgdat->numabalancing_migrate_lock);
    pgdat->numabalancing_migrate_nr_pages = 0;
    pgdat->numabalancing_migrate_next_window = jiffies;
#endif
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
    spin_lock_init(&pgdat->split_queue_lock);
    INIT_LIST_HEAD(&pgdat->split_queue);
    pgdat->split_queue_len = 0;
#endif

    /*  初始化pgdat->kswapd_wait等待队列  */
    init_waitqueue_head(&pgdat->kswapd_wait);
    /*  初始化页换出守护进程创建空闲块的大小
     *  为2^kswapd_max_order  */
    init_waitqueue_head(&pgdat->pfmemalloc_wait);
#ifdef CONFIG_COMPACTION
    init_waitqueue_head(&pgdat->kcompactd_wait);
#endif
    pgdat_page_ext_init(pgdat);

	/* 遍历每个管理区 */
    for (j = 0; j < MAX_NR_ZONES; j++) {
        struct zone *zone = pgdat->node_zones + j;
        unsigned long size, realsize, freesize, memmap_pages;
        unsigned long zone_start_pfn = zone->zone_start_pfn;

        /*  size为该管理区中的页框数，包括洞 */
        size = zone->spanned_pages;
         /* realsize为管理区中的页框数，不包括洞  /
        realsize = freesize = zone->present_pages;

        /*
         * Adjust freesize so that it accounts for how much memory
         * is used by this zone for memmap. This affects the watermark
         * and per-cpu initialisations
         * 调整realsize的大小，即减去page结构体占用的内存大小  */
        /*  memmap_pags为包括洞的所有页框的page结构体所占的大小  */
        memmap_pages = calc_memmap_size(size, realsize);
        if (!is_highmem_idx(j)) {
            if (freesize >= memmap_pages) {
                freesize -= memmap_pages;
                if (memmap_pages)
                    printk(KERN_DEBUG
                           "  %s zone: %lu pages used for memmap\n",
                           zone_names[j], memmap_pages);
            } else  /*  内存不够存放page结构体  */
                pr_warn("  %s zone: %lu pages exceeds freesize %lu\n",
                    zone_names[j], memmap_pages, freesize);
        }

        /* Account for reserved pages
         * 调整realsize的大小，即减去DMA保留页的大小  */
        if (j == 0 && freesize > dma_reserve) {
            freesize -= dma_reserve;
            printk(KERN_DEBUG "  %s zone: %lu pages reserved\n",
                    zone_names[0], dma_reserve);
        }

        if (!is_highmem_idx(j))
            nr_kernel_pages += freesize;
        /* Charge for highmem memmap if there are enough kernel pages */
        else if (nr_kernel_pages > memmap_pages * 2)
            nr_kernel_pages -= memmap_pages;
        nr_all_pages += freesize;

        /*
         * Set an approximate value for lowmem here, it will be adjusted
         * when the bootmem allocator frees pages into the buddy system.
         * And all highmem pages will be managed by the buddy system.
         */
        /* 设置zone->spanned_pages为包括洞的页框数  */
        zone->managed_pages = is_highmem_idx(j) ? realsize : freesize;
#ifdef CONFIG_NUMA
		/* 设置zone中的节点标识符 */
        zone->node = nid;
        /* 设置可回收页面比率 */
        zone->min_unmapped_pages = (freesize*sysctl_min_unmapped_ratio)
                        / 100;
        /* 设置slab回收缓存页的比率 */
        zone->min_slab_pages = (freesize * sysctl_min_slab_ratio) / 100;
#endif
		/*  设置zone的名称  */
        zone->name = zone_names[j];

        /* 初始化各种锁 */
		spin_lock_init(&zone->lock);
        spin_lock_init(&zone->lru_lock);
        zone_seqlock_init(zone);
        /* 设置管理区属于的节点对应的pg_data_t结构 */
        zone->zone_pgdat = pgdat;
        /* 初始化cpu的页面缓存 */
        zone_pcp_init(zone);

        /* For bootup, initialized properly in watermark setup */
        mod_zone_page_state(zone, NR_ALLOC_BATCH, zone->managed_pages);

        /* 初始化lru相关成员 */
        lruvec_init(&zone->lruvec);
        if (!size)
            continue;

        set_pageblock_order();
        /* 定义了CONFIG_SPARSEMEM该函数为空 */
        setup_usemap(pgdat, zone, zone_start_pfn, size);
		/* 设置pgdat->nr_zones和zone->zone_start_pfn成员
         * 初始化zone->free_area成员
         * 初始化zone->wait_table相关成员
         */
         ret = init_currently_empty_zone(zone, zone_start_pfn, size);
        BUG_ON(ret);
        /* 初始化该zone对应的page结构 */
        memmap_init(size, nid, j, zone_start_pfn);
    }
}
```

##2.7	 memmap_init
-------

在free_area_init_core初始化内存管理区zone的过程中, 通过memmap_init函数对每个内存管理区zone的page内存进行了初始化


memmap_init函数定义在[mm/page_alloc.c?v=4.7, line ](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L5241)

```cpp
#ifndef __HAVE_ARCH_MEMMAP_INIT
#define memmap_init(size, nid, zone, start_pfn) \
	memmap_init_zone((size), (nid), (zone), (start_pfn), MEMMAP_EARLY)
#endif
```
memmap_init_zone函数完成了page的初始化工作, 该函数定义在[mm/page_alloc.c?v=4.7, line 5139](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L5139)



#3	初始化内存结点和内存域
-------



##3.1	回到start_kernel函数(已经完成的工作)
-------



##3.2	build_all_zonelists
-------



内核在start_kernel中通过build_all_zonelists完成了内存结点及其管理内存域的初始化工作, 调用如下


```cpp
  build_all_zonelists(NULL, NULL);
```

[build_all_zonelists](http://lxr.free-electrons.com/source/mm/page_alloc.c?v4.7#L5029)建立内存管理结点及其内存域所需的数据结构.


##2.3	设置结点初始化顺序
-------

在build_all_zonelists开始, 首先内核通过set_zonelist_order函数设置了`zonelist_order`,如下所示, 参见[mm/page_alloc.c?v=4.7, line 5031](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L5031)

```cpp
void __ref build_all_zonelists(pg_data_t *pgdat, struct zone *zone)
{
	set_zonelist_order();
	/* .......  */
}
```


##2.3.1	zone table
-------


前面我们讲解内存管理域时候讲解到, 系统中的所有管理域都存储在一个多维的数组zone_table. 内核在初始化内存管理区时, 必须要建立管理区表zone_table. 参见[mm/page_alloc.c?v=2.4.37, line 38](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=2.4.37#L38)


```cpp
/*
 *
 * The zone_table array is used to look up the address of the
 * struct zone corresponding to a given zone number (ZONE_DMA,
 * ZONE_NORMAL, or ZONE_HIGHMEM).
 */
zone_t *zone_table[MAX_NR_ZONES*MAX_NR_NODES];
EXPORT_SYMBOL(zone_table);
```

*	MAX_NR_NODES为系统中内存结点的数目

*	MAX_NR_ZONES为系统中单个内存结点所拥有的最大内存区域数目



##2.3.2	内存域初始化顺序zonelist_order
-------


NUMA系统中存在多个节点, 每个节点对应一个`struct pglist_data`结构, 每个结点中可以包含多个zone, 如: ZONE_DMA, ZONE_NORMAL, 这样就产生几种排列顺序, 以2个节点2个zone为例(zone从高到低排列, ZONE_DMA0表示节点0的ZONE_DMA，其它类似).

*	Legacy方式, 每个节点只排列自己的zone；

![Legacy方式](../images/legacy-order.jpg)

*	Node方式, 按节点顺序依次排列，先排列本地节点的所有zone，再排列其它节点的所有zone。


![Node方式](../images/node-order.jpg)


*	Zone方式, 按zone类型从高到低依次排列各节点的同相类型zone



![Zone方式](../images/zone-order.jpg)



可通过启动参数"numa_zonelist_order"来配置zonelist order，内核定义了3种配置, 这些顺序定义在[mm/page_alloc.c?v=4.7, line 4551](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L4551)

```cpp
// http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L4551
/*
 *  zonelist_order:
 *  0 = automatic detection of better ordering.
 *  1 = order by ([node] distance, -zonetype)
 *  2 = order by (-zonetype, [node] distance)
 *
 *  If not NUMA, ZONELIST_ORDER_ZONE and ZONELIST_ORDER_NODE will create
 *  the same zonelist. So only NUMA can configure this param.
 */
#define ZONELIST_ORDER_DEFAULT  0 /* 智能选择Node或Zone方式 */

#define ZONELIST_ORDER_NODE     1 /* 对应Node方式 */

#define ZONELIST_ORDER_ZONE     2 /* 对应Zone方式 */
```

>注意
>
>在非NUMA系统中(比如UMA), 由于只有一个内存结点, 因此ZONELIST_ORDER_ZONE和ZONELIST_ORDER_NODE选项会配置相同的内存域排列方式, 因此, 只有NUMA可以配置这几个参数






全局的current_zonelist_order变量标识了系统中的当前使用的内存域排列方式, 默认配置为ZONELIST_ORDER_DEFAULT, 参见[mm/page_alloc.c?v=4.7, line 4564](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L4564)


```cpp
//  http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L4564
/* zonelist order in the kernel.
 * set_zonelist_order() will set this to NODE or ZONE.
 */
static int current_zonelist_order = ZONELIST_ORDER_DEFAULT;
static char zonelist_order_name[3][8] = {"Default", "Node", "Zone"};
```





而zonelist_order_name方式分别对应了Legacy方式, Node方式和Zone方式. 其zonelist_order_name[current_zonelist_order]就标识了当前系统中所使用的内存域排列方式的名称"Default", "Node", "Zone".


| 宏 | zonelist_order_name[宏](排列名称) | 排列方式 | 描述 |
|:--:|:-------------------:|:------:|:----:|
| ZONELIST_ORDER_DEFAULT | Default |  | 由系统智能选择Node或Zone方式 |
| ZONELIST_ORDER_NODE | Node | Node方式 | 按节点顺序依次排列，先排列本地节点的所有zone，再排列其它节点的所有zone |
| ZONELIST_ORDER_ZONE | Zone | Zone方式 | 按zone类型从高到低依次排列各节点的同相类型zone |



##2.3.3	set_zonelist_order设置排列方式
-------

内核就通过通过set_zonelist_order函数设置当前系统的内存域排列方式current_zonelist_order, 其定义依据系统的NUMA结构还是UMA结构有很大的不同.

```cpp
// http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L4571
#ifdef CONFIG_NUMA
/* The value user specified ....changed by config */
static int user_zonelist_order = ZONELIST_ORDER_DEFAULT;
/* string for sysctl */
#define NUMA_ZONELIST_ORDER_LEN 16
char numa_zonelist_order[16] = "default";


//  http://lxr.free-electrons.com/source/mm/page_alloc.c#L4571
static void set_zonelist_order(void)
{
    if (user_zonelist_order == ZONELIST_ORDER_DEFAULT)
        current_zonelist_order = default_zonelist_order();
    else
        current_zonelist_order = user_zonelist_order;
}


#else   /* CONFIG_NUMA */

//  http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L4892
static void set_zonelist_order(void)
{
	current_zonelist_order = ZONELIST_ORDER_ZONE;
}
```


其设置的基本流程如下

*	如果系统当前系统是非NUMA结构的, 则系统中只有一个结点, 配置ZONELIST_ORDER_NODE和ZONELIST_ORDER_ZONE结果相同. 那么set_zonelist_order函数被定义为直接配置当前系统的内存域排列方式`current_zonelist_order`为ZONE方式(与NODE效果相同)

*	如果系统是NUMA结构, 则设置为系统指定的方式即可
	1.	当前的排列方式为ZONELIST_ORDER_DEFAULT, 即系统默认方式, 则current_zonelist_order则由内核交给default_zonelist_order采用一定的算法选择一个最优的分配策略,　目前的系统中如果是32位则配置为ZONE方式, 而如果是６４位系统则设置为NODE方式

	2.	当前的排列方式不是默认方式, 则设置为user_zonelist_order指定的内存域排列方式

##2.3.4	default_zonelist_order函数选择最优的配置
-------

在UMA结构下, 内存域使用NODE和ZONE两个排列方式会产生相同的效果, 因此系统不用特殊指定, 直接通过set_zonelist_order函数, 将当前系统的内存域排列方式`current_zonelist_order`配置为为ZONE方式(与NODE效果相同)即可


但是NUMA结构下, 默认情况下(当配置了ZONELIST_ORDER_DEFAULT), 系统需要根据系统自身的环境信息选择一个最优的配置(NODE或者ZONE方式), 这个工作就由**default_zonelist_order函数**了来完成. 其定义在[mm/page_alloc.c?v=4.7, line 4789](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L4789)


```cpp
#if defined(CONFIG_64BIT)
/*
 * Devices that require DMA32/DMA are relatively rare and do not justify a
 * penalty to every machine in case the specialised case applies. Default
 * to Node-ordering on 64-bit NUMA machines
 */
static int default_zonelist_order(void)
{
    return ZONELIST_ORDER_NODE;
}
#else
/*
 * On 32-bit, the Normal zone needs to be preserved for allocations accessible
 * by the kernel. If processes running on node 0 deplete the low memory zone
 * then reclaim will occur more frequency increasing stalls and potentially
 * be easier to OOM if a large percentage of the zone is under writeback or
 * dirty. The problem is significantly worse if CONFIG_HIGHPTE is not set.
 * Hence, default to zone ordering on 32-bit.
 */
static int default_zonelist_order(void)
{
    return ZONELIST_ORDER_ZONE;
}
#endif /* CONFIG_64BIT */
```



###2.3.5	user_zonelist_order用户指定排列方式
-------


在NUMA结构下, 系统支持用户指定内存域的排列方式, 用户以字符串的形式操作numa_zonelist_order(default, node和zone), 最终被内核转换为user_zonelist_order, 这个变量被指定为字符串numa_zonelist_order指定的排列方式, 他们定义在[mm/page_alloc.c?v4.7, line 4573](http://lxr.free-electrons.com/source/mm/page_alloc.c?v4.7#L4573), 注意只有在NUMA结构中才需要这个配置信息.


```cpp
#ifdef CONFIG_NUMA
/* The value user specified ....changed by config */
static int user_zonelist_order = ZONELIST_ORDER_DEFAULT;
/* string for sysctl */
#define NUMA_ZONELIST_ORDER_LEN 16
char numa_zonelist_order[16] = "default";

#else
/* ......*/
#endif
```

而接受和处理用户配置的工作, 自然是交给我们强大的proc文件系统来完成的, 可以通过/proc/sys/vm/numa_zonelist_order动态改变zonelist order的分配方式。




![/proc/sys/vm/numa_zonelist_order`](../images/proc-numa_zonelist_order.png)



内核通过setup_numa_zonelist_order读取并处理用户写入的配置信息

*	接收到用户的信息后用__parse_numa_zonelist_order处理接收的参数

*	如果前面用__parse_numa_zonelist_order处理的信息串成功, 则将对用的设置信息写入到字符串numa_zonelist_order中


参见[mm/page_alloc.c?v=4.7, line 4578](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L4578)


```cpp
/*
 * interface for configure zonelist ordering.
 * command line option "numa_zonelist_order"
 *      = "[dD]efault   - default, automatic configuration.
 *      = "[nN]ode      - order by node locality, then by zone within node
 *      = "[zZ]one      - order by zone, then by locality within zone
 */

static int __parse_numa_zonelist_order(char *s)
{
    if (*s == 'd' || *s == 'D') {
        user_zonelist_order = ZONELIST_ORDER_DEFAULT;
    } else if (*s == 'n' || *s == 'N') {
        user_zonelist_order = ZONELIST_ORDER_NODE;
    } else if (*s == 'z' || *s == 'Z') {
        user_zonelist_order = ZONELIST_ORDER_ZONE;
    } else {
        pr_warn("Ignoring invalid numa_zonelist_order value:  %s\n", s);
        return -EINVAL;
    }
    return 0;
}

static __init int setup_numa_zonelist_order(char *s)
{
    int ret;

    if (!s)
        return 0;

    ret = __parse_numa_zonelist_order(s);
    if (ret == 0)
        strlcpy(numa_zonelist_order, s, NUMA_ZONELIST_ORDER_LEN);

    return ret;
}
early_param("numa_zonelist_order", setup_numa_zonelist_order);
```

##2.4	build_all_zonelists_init
-------


###2.4.1	system_state系统状态标识

其中`system_state`变量是一个系统全局定义的用来表示系统当前运行状态的枚举变量, 其定义在[include/linux/kernel.h?v=4.7, line 487](http://lxr.free-electrons.com/source/include/linux/kernel.h?v=4.7#L487)


```cpp
/* Values used for system_state */
extern enum system_states
{
	SYSTEM_BOOTING,
	SYSTEM_RUNNING,
	SYSTEM_HALT,
	SYSTEM_POWER_OFF,
	SYSTEM_RESTART,
} system_state;
```

*	如果系统system_state是SYSTEM_BOOTING, 则调用`build_all_zonelists_init`初始化所有的内存结点

*	否则的话如果定义了冷热页`CONFIG_MEMORY_HOTPLUG`且参数zone(待初始化的内存管理域zone)不为NULL, 则调用setup_zone_pageset设置冷热页



```cpp
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
```

#3	特定于体系结构的设置
-------

##2.1	内核在内存中的布局
-------

##2.2	初始化过程
-------


##2.3	分页机制初始化
-------

##2.4	注册活动内存区
-------

##2.5	系统的地址空间设置
-------

