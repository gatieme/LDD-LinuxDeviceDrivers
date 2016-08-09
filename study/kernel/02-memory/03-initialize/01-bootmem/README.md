启动过程期间的内存管理--bootmem分配器
=======



| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6225543) |

http://blog.csdn.net/vanbreaker/article/details/7554977


http://blog.csdn.net/yuzhihui_no1/article/details/50759567

http://www.cnblogs.com/zhenjing/archive/2012/03/21/linux_numa.html

http://www.linuxidc.com/Linux/2012-05/60230.htm

[[内存管理] bootmem没了？](http://bbs.chinaunix.net/thread-4141073-1-1.html)

http://blog.chinaunix.net/uid-26009923-id-3860465.html

http://www.linuxidc.com/Linux/2012-02/53139.htm

http://blog.chinaunix.net/uid-7588746-id-1629805.html


http://blog.csdn.net/xxxxxlllllxl/article/details/12091667


http://blog.csdn.net/xxxxxlllllxl/article/details/12091667

http://lib.csdn.net/article/embeddeddevelopment/29997#focustext

在内存管理的上下文中, 初始化(initialization)可以有多种含义. 在许多CPU上, 必须显式设置适用于Linux内核的内存模型. 例如在x86_32上需要切换到保护模式, 然后内核才能检测到可用内存和寄存器.


而我们今天要讲的bootmem分配器就是系统初始化阶段使用的内存分配器. 

为什么要使用bootmem分配器，内存管理不是有buddy系统和slab分配器吗？由于在系统初始化的时候需要执行一些内存管理，内存分配的任务，这个时候buddy系统，slab分配器等并没有被初始化好，此时就引入了一种内存管理器bootmem分配器在系统初始化的时候进行内存管理与分配，当buddy系统和slab分配器初始化好后，在mem_init()中对bootmem分配器进行释放，内存管理与分配由buddy系统，slab分配器等进行接管。

bootmem分配器使用一个bitmap来标记物理页是否被占用，分配的时候按照第一适应的原则，从bitmap中进行查找，如果这位为1，表示已经被占用，否则表示未被占用。为什么系统运行的时候不使用bootmem分配器呢？bootmem分配器每次在bitmap中进行线性搜索，效率非常低，而且在内存的起始端留下许多小的空闲碎片，在需要非常大的内存块的时候，检查位图这一过程就显得代价很高。bootmem分配器是用于在启动阶段分配内存的，对该分配器的需求集中于简单性方面，而不是性能和通用性。

bootmem allocator 核心数据结构
bootmem allocator 的初始化
bootmem allocator 分配内存
bootmem allocator 保留内存
bootmem allocator 释放内存
bootmem allocator的销毁


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


##1.2	内存结点pg_data_t
-------

在LINUX中引入一个数据结构`struct pglist_data` ，来描述一个node，定义在[`include/linux/mmzone.h`](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L630) 文件中。（这个结构被typedef pg_data_t）。


*    对于NUMA系统来讲， 整个系统的内存由一个[node_data](http://lxr.free-electrons.com/source/arch/s390/numa/numa.c?v=4.7#L23)的pg_data_t指针数组来管理

*    对于PC这样的UMA系统，使用struct pglist_data contig_page_data ，作为系统唯一的node管理所有的内存区域。（UMA系统中中只有一个node）

可以使用NODE_DATA(node_id)来查找系统中编号为node_id的结点, 而UMA结构下由于只有一个结点, 因此该宏总是返回全局的contig_page_data, 而与参数node_id无关.

**NODE_DATA(node_id)查找编号node_id的结点pg_data_t信息** 参见[NODE_DATA的定义](http://lxr.free-electrons.com/ident?v=4.7;i=NODE_DATA)

```cpp
extern struct pglist_data *node_data[];
#define NODE_DATA(nid)          (node_data[(nid)])
```


在UMA结构的机器中, 只有一个node结点即contig_page_data, 此时NODE_DATA直接指向了全局的contig_page_data, 而与node的编号nid无关, 参照[include/linux/mmzone.h?v=4.7, line 858](http://lxr.free-electrons.com/source/include/linux/mmzone.h?v=4.7#L858)


```cpp
extern struct pglist_data contig_page_data;
#define NODE_DATA(nid)          (&contig_page_data)

```

##1.2	物理内存区域
-------

因为实际的计算机体系结构有硬件的诸多限制, 这限制了页框可以使用的方式. 尤其是, Linux内核必须处理80x86体系结构的两种硬件约束.

*	ISA总线的直接内存存储DMA处理器有一个严格的限制 : 他们只能对RAM的前16MB进行寻址

*	在具有大容量RAM的现代32位计算机中, CPU不能直接访问所有的物理地址, 因为线性地址空间太小, 内核不可能直接映射所有物理内存到线性地址空间, 我们会在后面典型架构(x86)上内存区域划分详细讲解x86_32上的内存区域划分


因此Linux内核对不同区域的内存需要采用不同的管理方式和映射方式, 因此内核将物理地址或者成用zone_t表示的不同地址区域

对于x86_32的机器，管理区(内存区域)类型如下分布


| 类型 | 区域 |
| :------- | ----: |
| ZONE_DMA | 0~15MB |
| ZONE_NORMAL | 16MB~895MB |
| ZONE_HIGHMEM | 896MB~物理内存结束 |


##1.3	物理页帧
-------

内核把物理页作为内存管理的基本单位. 尽管处理器的最小可寻址单位通常是字, 但是, 内存管理单元MMU通常以页为单位进行处理. 因此，从虚拟内存的上来看，页就是最小单位.

页帧代表了系统内存的最小单位, 对内存中的每个页都会创建struct page的一个实例. 内核必须要保证page结构体足够的小，否则仅struct page就要占用大量的内存.


 内核用[struct  page(include/linux/mm_types.h?v=4.7, line 45)](http://lxr.free-electrons.com/source/include/linux/mm_types.h?v4.7#L45)结构表示系统中的每个物理页.

出于节省内存的考虑，struct page中使用了大量的联合体union.


`mem_map`是一个struct page的数组，管理着系统中所有的物理内存页面。在系统启动的过程中，创建和分配mem_map的内存区域, mem_map定义在[mm/page_alloc.c?v=4.7, line 6691](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L6691)


UMA体系结构中，free_area_init函数在系统唯一的struct node对象contig_page_data中node_mem_map成员赋值给全局的mem_map变量

##1.4	今日内容(启动过程中的内存初始化)
-------


**启动过程中的内存初始化**

在初始化过程中, 还必须建立内存管理的数据结构, 以及很多事务. 因为内核在内存管理完全初始化之前就需要使用内存. 在系统启动过程期间, 使用了额外的简化悉尼股市的内存管理模块, 然后在初始化完成后, 将旧的模块丢弃掉.

**系统启动**

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




#2	引导内存分配器bootmem概述
-------



由于硬件配置多种多样, 所以在编译时就静态初始化所有的内核存储结构是不现实的.


bootmem分配器是系统启动初期的内存分配方式，在耳熟能详的伙伴系统建立前内存都是利用bootmem分配器来分配的，伙伴系统框架建立起来后，bootmem会过度到伙伴系统.


##2.1	初始化阶段的引导内存分配器bootmem
-------

在启动过程期间, 尽管内存管理尚未初始化, 但是内核仍然需要分配内存以创建各种数据结构. 因此在系统启动过程期间, 内核使用了一个额外的简化形式的内存管理模块**引导内存分配器(boot memory allocator--bootmem分配器)**, 用于在启动阶段早期分配内存, 而在系统初始化完成后, 该分配器被内核抛弃, 然后初始化了一套新的更加完善的内存分配器.

显然, 对该内存分配器的需求集中于简单性方面,　而不是性能和通用性, 它仅用于初始化阶段. 因此内核开发者决定实现一个最先适配(first-first)分配器用于在启动阶段管理内存. 这是可能想到的最简单的方式.


**引导内存分配器(boot memory allocator--bootmem分配器)**基于最先适配(first-first)分配器的原理(这儿是很多系统的内存分配所使用的原理), 使用一个位图来管理页, 以位图代替原来的空闲链表结构来表示存储空间, 位图的比特位的数目与系统中物理内存页面数目相同. 若位图中某一位是1, 则标识该页面已经被分配(已用页), 否则表示未被占有(未用页).

在需要分配内存时, 分配器逐位的扫描位图, 直至找到一个能提供足够连续页的位置, 即所谓的最先最佳(first-best)或最先适配位置.

该分配机制通过记录上一次分配的页面帧号(PFN)结束时的偏移量来实现分配大小小于一页的空间, 连续的小的空闲空间将被合并存储在一页上.


##2.2	为什么需要bootmem
-------


##2.3	为什么在系统运行时抛弃bootmem

当系统运行时, 为何不继续使用bootmem分配机制呢?

*	其中一个关键原因在于 : 但它每次分配都必须从头扫描位图, 每次通过对内存域进行线性搜索来实现分配.

＊	其次首先适应算法容易在内存的起始断留下许多小的空闲碎片, 在需要分配较大的空间页时,　检查位图的成本将是非常高的.



引导内存分配器bootmem分配器简单却非常低效,　因此在内核完全初始化之后,　不能将该分配器继续欧诺个与内存管理,　而伙伴系统(连同slab, slub或者slob分配器)是一个好很多的备选方案．



#3	引导内存分配器数据结构
-------

##3.1	bootmem_data表示引导内存区域

即使是初始化用的最先适配分配器也必须使用一些数据结构存, 内核为系统中每一个结点都提供了一个struct bootmem_data结构的实例, 用于bootmem的内存管理. 它含有引导内存分配器给结点分配内存时所需的信息. 当然, 这时候内存管理还没有初始化, 因而该结构所需的内存是无法动态分配的, 必须在编译时分配给内核.

在UMA系统上该分配的实现与CPU无关, 而NUMA系统内存结点与CPU相关联, 因此采用了特定体系结构的解决方法.


bootmem_data的结构定义在[include/linux/bootmem.h?v=4.7, line 28](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L28), 其定义如下所示

```cpp
#ifndef CONFIG_NO_BOOTMEM
/*
* node_bootmem_map is a map pointer - the bits represent all physical 
* memory pages (including holes) on the node.
*/
typedef struct bootmem_data {
       unsigned long node_min_pfn;
       unsigned long node_low_pfn;
       void *node_bootmem_map;
       unsigned long last_end_off;
       unsigned long hint_idx;
       struct list_head list;
} bootmem_data_t;

extern bootmem_data_t bootmem_node_data[];

#endif
```



|  字段  |  描述  |
|:-----:|:------:|
| node_min_pfn | 节点起始地址 |
| node_low_pfn | 低端内存最后一个page的页帧号 |
| node_bootmem_map | 指向内存中位图bitmap所在的位置 |
| last_end_off | 分配的最后一个页内的偏移，如果该页完全使用，则offset为0 |
| hint_idx | |
| list | |


bootmem的位图建立在从start_pfn开始的地方, 也就是说, 内核映像终点_end上方的地方. 这个位图用来管理低区（例如小于 896MB), 因为在0到896MB的范围内, 有些页面可能保留, 有些页面可能有空洞, 因此, 建立这个位图的目的就是要搞清楚哪一些物理页面是可以动态分配的

*	node_bootmem_map就是一个指向位图的指针. node_min_pfn表示存放bootmem位图的第一个页面(即内核映像结束处的第一个页面)

*	node_low_pfn 表示物理内存的顶点, 最高不超过896MB


##3.2	初始化引导分配器
-------


每一个体系结构都有一个setup_arch函数, 用于获取初始化引导内存分配器所需的参数信息

各种体系结构都有其函数来获取这些信息, 在x86体系结构中


##3.3	初始化内存结点与内存域
-------


###3.3.1	初始化过程
-------

| 调用层次 | 描述 | x86(已经不使用bootmem初始化) | arm | arm64 |
|:-------:|:---:|:---:|:---:|:-----:|
| setup_arch  | 设置特定体系的信息 | [arch/x86/kernel/setup.c](http://lxr.free-electrons.com/source/arch/x86/kernel/setup.c?v=4.7#L857), 但是不再调用paging_init | [arch/arm/kernel/setup.c](http://lxr.free-electrons.com/source/arch/arm/kernel/setup.c?v=4.7#L1073), 调用了[paging_init](http://lxr.free-electrons.com/source/arch/arm/kernel/setup.c?v=4.7#L1073) | [arch/arm64/kernel/setup.c](http://lxr.free-electrons.com/source/arch/arm64/kernel/setup.c?v=4.7#L266), 调用了[paging_init](http://lxr.free-electrons.com/source/arch/arm64/kernel/setup.c?v=4.7#L266)和[bootmem_init](http://lxr.free-electrons.com/source/arch/arm64/kernel/setup.c?v=4.7#L271) |
| paging_init | 初始化分页机制 | 定义了[arch/x86/mm/init_32.c](http://lxr.free-electrons.com/source/arch/x86/mm/init_32.c?v=4.7#L695)和[arch/x86/mm/init_64.c](http://lxr.free-electrons.com/source/arch/x86/mm/init_64.c?v=4.7#L579)两个版本 | 分别定义了[arch/arm/mm/nommu.c](http://lxr.free-electrons.com/source/arch/arm/mm/nommu.c?v=4.7#L311)和[arch/arm/mm/mmu.c](http://lxr.free-electrons.com/source/arch/arm/mm/mmu.c?v=4.7#L1623)两个版本, 均调用了bootmem_init | [arch/arm64/mm/mmu.c](http://lxr.free-electrons.com/source/arch/arm64/mm/mmu.c?v=4.7#L538) |
| bootmem_init | 初始化bootmem分配器 | 无定义 | [arch/arm/mm/init.c](http://lxr.free-electrons.com/source/arch/arm/mm/init.c?v=4.7#L282), 调用了zone_sizes_init | [arch/arm64/mm/init.c](http://lxr.free-electrons.com/source/arch/arm64/mm/init.c?v=4.7#L306),调用了zone_sizes_init |
|  zone_sizes_init　| 初始化节点和管理区 | [arch/x86/mm/init.c](http://lxr.free-electrons.com/source/arch/x86/mm/init.c?v=4.7#L718)  | [arch/arm/mm/init.c](http://lxr.free-electrons.com/source/arch/arm/mm/init.c?v=4.7#L137)| [arch/arm64/mm/init.c](http://lxr.free-electrons.com/source/arch/arm64/mm/init.c?v=4.7#L92) |
| [free_area_init_nodes](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L6460) | 初始化结点中所有内存区 | [mm/page_alloc.c](http://lxr.free-electrons.com/ident?i=free_area_init_nodes), 体系结构无关 | [mm/page_alloc.c](http://lxr.free-electrons.com/ident?i=free_area_init_nodes), 体系结构无关 | [mm/page_alloc.c](http://lxr.free-electrons.com/ident?i=free_area_init_nodes), 体系结构无关 |




下面我们就以标准的arm架构来分析bootmem初始化内存结点和内存域的过程

*	首先内核从[start_kernel](http://lxr.free-electrons.com/source/init/main.c?v=4.7#L505)开始启动

*	然后进入体系结构相关的设置部分[setup_arch](http://lxr.free-electrons.com/source/arch/arm/kernel/setup.c?v=4.7#L1073), 开始获取并设置指定体系结构的一些物理信息

*	在setup_arch函数内, 通过[paging_init函数]()初始化了分页机制和页表的细心

*	接着paging_init函数通过[bootmem_init](http://lxr.free-electrons.com/source/arch/arm/mm/mmu.c#L1642)开始进行bootmem初始化的工作

###3.3.2	bootmem_init
-------

```cpp
void __init bootmem_init(void)
{
    unsigned long min, max_low, max_high;

    memblock_allow_resize();
    max_low = max_high = 0;

    /* 找到内存区域大小，
     * max_low低端内存上界限
     * max_high 总内存上界
     */
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
```