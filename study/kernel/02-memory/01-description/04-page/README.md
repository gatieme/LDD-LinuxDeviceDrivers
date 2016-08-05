 服务器体系与共享存储器架构
=======

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6225543) |


http://blog.chinaunix.net/uid-30282771-id-5176971.html
http://www.cnblogs.com/hanyan225/archive/2011/07/28/2119628.html
#1	前景回顾
-------

#1.1	UMA和NUMA两种模型
-------

共享存储型多处理机有两种模型

*	均匀存储器存取（Uniform-Memory-Access，简称UMA）模型


*	非均匀存储器存取（Nonuniform-Memory-Access，简称NUMA）模型

#1.2	(N)UMA模型中linux内存的机构
-------

非一致存储器访问(NUMA)模式下

*	处理器被划分成多个"节点"(node), 每个节点被分配有的本地存储器空间. 所有节点中的处理器都可以访问全部的系统物理存储器，但是访问本节点内的存储器所需要的时间，比访问某些远程节点内的存储器所花的时间要少得多


*	内存被分割成多个区域（BANK，也叫"簇"），依据簇与处理器的"距离"不同, 访问不同簇的代码也会不同.


##1.3	Linux如何描述物理内存
-------

Linux把物理内存划分为三个层次来管理

| 层次 | 描述 |
|:----:|:----:|
| 存储节点(Node) |  CPU被划分为多个节点(node), 内存则被分簇, 每个CPU对应一个本地物理内存, 即一个CPU-node对应一个内存簇bank，即每个内存簇被认为是一个节点 |
| 管理区(Zone)   | 每个物理内存节点node被划分为多个内存管理区域, 用于表示不同范围的内存, 内核可以使用不同的映射方式映射物理内存 |
| 页面(Page) 	   |	内存被细分为多个页面帧, 页面是最基本的页面分配的单位　｜


*	首先内存被划分为结点. 内存中的每个节点都是由pg_data_t描述,而pg_data_t由struct pglist_data定义而来, 该数据结构定义在[include/linux/mmzone.h, line 615](http://lxr.free-electrons.com/source/include/linux/mmzone.h#L615), 每个结点关联到系统中的一个处理器, 内核中表示为`pg_data_t`的实例. 系统中每个节点被链接到一个以NULL结尾的`pgdat_list`链表中<而其中的每个节点利用`pg_data_tnode_next`字段链接到下一节．而对于PC这种UMA结构的机器来说, 只使用了一个成为contig_page_data的静态pg_data_t结构.


*	接着各个节点又被划分为内存管理区域, 一个管理区域通过struct zone_struct描述, 其被定义为zone_t, 用以表示内存的某个范围, 低端范围的16MB被描述为ZONE_DMA, 某些工业标准体系结构中的(ISA)设备需要用到它, 然后是可直接映射到内核的普通内存域ZONE_NORMAL,最后是超出了内核段的物理地址域ZONE_HIGHMEM, 被称为高端内存.　是系统中预留的可用内存空间, 不能被内核直接映射.

*	最后页帧(page frame)代表了系统内存的最小单位, 堆内存中的每个页都会创建一个struct page的一个实例. 传统上，把内存视为连续的字节，即内存为字节数组，内存单元的编号(地址)可作为字节数组的索引. 分页管理时，将若干字节视为一页，比如4K byte. 此时，内存变成了连续的页，即内存为页数组，每一页物理内存叫页帧，以页为单位对内存进行编号，该编号可作为页数组的索引，又称为页帧号.


##1.4	今日内容(页帧struct page)
-------


页帧代表了系统内存的最小单位, 对内存中的每个页都会创建struct page的一个实例. 内核必须要保证page结构体足够的小，否则仅struct page就要占用大量的内存.

因为即使在中等程序的内存配置下, 系统的内存同样会分解为大量的页. 例如, IA-32系统中标准页长度为4KB, 在内存大小为384MB时, 大约有100000页. 就当今的标准而言, 这个容量算不上很大, 但页的数目已经非常可观了

因而出于节省内存的考虑，内核要尽力保持struct page尽可能的小. 在典型的系统中, 由于页的数目巨大, 因此对page结构的小改动, 也可能导致保存所有page实例所需的物理内存暴涨.

页的广泛使用, 增加了保持结构长度的难度 : 内存管理的许多部分都使用页, 用于各种不同的用途. 内核的一部分可能完全依赖于struct page提供的特定信息, 而这部分信息堆内核的其他部分页可能是完全无用的. 等等.


我们今天就来详细讲解一下linux下物理页帧的描述

#2	页帧
-------

内核把物理页作为内存管理的基本单位. 尽管处理器的最小可寻址单位通常是字, 但是, 内存管理单元MMU通常以页为单位进行处理. 因此，从虚拟内存的上来看，页就是最小单位. 内核用[struct  page(include/linux/mm_types.h?v=4.7, line 45)](http://lxr.free-electrons.com/source/include/linux/mm_types.h?v4.7#L45)结构表示系统中的每个物理页.


出于节省内存的考虑，struct page中使用了大量的联合体union.

##2.1	struct page结构
-------



| 字段 | 描述 |
|:---:|:----:|
| flag | 用来存放页的状态，每一位代表一种状态，所以至少可以同时表示出32中不同的状态,这些状态定义在linux/page-flags.h中 |
| count | 记录了该页被引用了多少次 |
| mapping | 指向与该页相关的address_space对象
| virtual | 页的虚拟地址，它就是页在虚拟内存中的地址。要理解的一点是page结构与物理页相关，而并非与虚拟页相关。因此，该结构对页的描述是短暂的。内核仅仅用这个结构来描述当前时刻在相关的物理页中存放的东西。这种数据结构的目的在于描述物理内存本身，而不是描述包含在其中的数据 |
|





#页面的状态flags
-------


page->flags描述page的状态和其他信息


```cpp
struct page {
    /* First double word block */
    unsigned long flags;        /* Atomic flags,
    some possibly updated asynchronously, 描述page的状态和其他信息  */
```

如下如所示

![page的flags标识](./images/flags.png)


主要分为4部分，其中标志位flag向高位增长, 其余位字段向低位增长，中间存在空闲位

| 字段 | 描述 |
|:----:|:---:|
| section | 主要用于稀疏内存模型SPARSEMEM，可忽略 |
| node | NUMA节点号, 标识该page属于哪一个节点 |
| zone | 内存域标志，标识该page属于哪一个zone |
| flag | page的状态标识 |

其中最后一个flag用于标识page的状态, 这些状态由枚举常量[`enum pageflags`](http://lxr.free-electrons.com/source/include/linux/page-flags.h?v=4.7#L74)定义, 定义在[include/linux/page-flags.h?v=4.7, line 74](http://lxr.free-electrons.com/source/include/linux/page-flags.h?v=4.7#L74). 常用的有如下状态

| 页面状态 | 描述 |
|:-------:|:----:|
| PG_locked | page被锁定，说明有使用者正在操作该page |
| PG_error | 状态标志，表示涉及该page的IO操作发生了错误 |
| PG_referenced | 表示page刚刚被访问过 |
| PG_uptodate | 表示page的数据已经与后备存储器是同步的，是最新的 |
| PG_dirty | 与后备存储器中的数据相比，该page的内容已经被修改 |
| PG_lru | 表示该page处于LRU链表上 |
| PG_active | page处于inactive LRU链表, PG_active和PG_referenced一起控制该page的活跃程度，这在内存回收时将会非常有用 |
| PG_slab | 该page属于slab分配器 |
| PG_onwer_priv_1 | |
| PG_arch_1	      | |
| PG_reserved | 设置该标志，防止该page被交换到swap  |
| PG_private | 如果page中的private成员非空，则需要设置该标志 |
| PG_private_2 | |
| PG_writeback | page中的数据正在被回写到后备存储器 |
| PG_head | |
| PG_swapcache | 表示该page处于swap cache中 |
| PG_mappedtodisk | 表示page中的数据在后备存储器中有对应 |
| PG_reclaim | 表示该page要被回收。当PFRA决定要回收某个page后，需要设置该标志 |
| PG_swapbacked | 该page的后备存储器是swap |
| PG_unevictable | 该page被锁住，不能交换，并会出现在LRU_UNEVICTABLE链表中，它包括的几种page：ramdisk或ramfs使用的页, shm_locked、mlock锁定的页 |
| PG_mlocked | 该page在vma中被锁定，一般是通过系统调用mlock()锁定了一段内存 |
| PG_uncached | |
| PG_hwpoison | |
| PG_young | |
| PG_idle  | |

内核中提供了一些标准宏，用来检查、操作某些特定的比特位，如：
        -> PageXXX(page)：检查page是否设置了PG_XXX位
        -> SetPageXXX(page)：设置page的PG_XXX位
        -> ClearPageXXX(page)：清除page的PG_XXX位
        -> TestSetPageXXX(page)：设置page的PG_XXX位，并返回原值
        -> TestClearPageXXX(page)：清除page的PG_XXX位，并返回原值

    2) _count：引用计数，表示内核中引用该page的次数，如果要操作该page，引用计数会+1，操作完成-1。当该值为0时，表示没有引用该page的位置，所以该page可以被解除映射，这往往在内存回收时是有用的。

    3) _mapcount：被页表映射的次数，也就是说该page同时被多少个进程共享。初始值为-1，如果只被一个进程的页表映射了，该值为0 。如果该page处于伙伴系统中，该值为PAGE_BUDDY_MAPCOUNT_VALUE（-128），内核通过判断该值是否为PAGE_BUDDY_MAPCOUNT_VALUE来确定该page是否属于伙伴系统。
    注意区分_count和_mapcount，_mapcount表示的是映射次数，而_count表示的是使用次数；被映射了不一定在使用，但要使用必须先映射。

    4) mapping：有三种含义
        a: 如果mapping = 0，说明该page属于交换缓存（swap cache）；当需要使用地址空间时会指定交换分区的地址空间swapper_space。
        b: 如果mapping != 0，bit[0] = 0，说明该page属于页缓存或文件映射，mapping指向文件的地址空间address_space。
        c: 如果mapping != 0，bit[0] != 0，说明该page为匿名映射，mapping指向struct anon_vma对象。
        通过mapping恢复anon_vma的方法：anon_vma = (struct anon_vma *)(mapping - PAGE_MAPPING_ANON)。

    5) index：在映射的虚拟空间（vma_area）内的偏移；一个文件可能只映射一部分，假设映射了1M的空间，index指的是在1M空间内的偏移，而不是在整个文件内的偏移。

    6) private：私有数据指针，由应用场景确定其具体的含义：
        a：如果设置了PG_private标志，表示buffer_heads；
        b：如果设置了PG_swapcache标志，private存储了该page在交换分区中对应的位置信息swp_entry_t。
        c：如果_mapcount = PAGE_BUDDY_MAPCOUNT_VALUE，说明该page位于伙伴系统，private存储该伙伴的阶。

    7) lru：链表头，主要有3个用途：
        a：page处于伙伴系统中时，用于链接相同阶的伙伴（只使用伙伴中的第一个page的lru即可达到目的）。
        b：page属于slab时，page->lru.next指向page驻留的的缓存的管理结构，page->lru.prec指向保存该page的slab的管理结构。
        c：page被用户态使用或被当做页缓存使用时，用于将该page连入zone中相应的lru链表，供内存回收时使用。







