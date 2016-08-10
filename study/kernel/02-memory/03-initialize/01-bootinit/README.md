启动过程期间的内存管理--bootmem分配器
=======



| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6225543) |



在内存管理的上下文中, 初始化(initialization)可以有多种含义. 在许多CPU上, 必须显式设置适用于Linux内核的内存模型. 例如在x86_32上需要切换到保护模式, 然后内核才能检测到可用内存和寄存器.


而我们今天要讲的boot阶段就是系统初始化阶段使用的内存分配器. 




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


#2	启动过程中的内存初始化
-------




在初始化过程中, 还必须建立内存管理的数据结构, 以及很多事务. 因为内核在内存管理完全初始化之前就需要使用内存. 在系统启动过程期间, 使用了额外的简化悉尼股市的内存管理模块, 然后在初始化完成后, 将旧的模块丢弃掉.

##2.1	系统启动过程中的内存管理
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


##2.2	引导过程中的内存初始化(bootmem和nobootmem)
-------



由于硬件配置多种多样, 所以在编译时就静态初始化所有的内核存储结构是不现实的. 在boot传递给kernel memory bank相关信息后，kernel这边会以memblcok的方式保存这些信息，当buddy system 没有起来之前，在kernel中也是要有一套机制来管理memory的申请和释放.


Kernel可以选择nobootmem 或者bootmem 来在buddy system起来之前管理memory.
这两种机制对提供的API是一致的，因此对用户是透明的

参见[mm/Makefile](http://lxr.free-electrons.com/source/mm/Makefile#L44)


```cpp
ifdef CONFIG_NO_BOOTMEM
	obj-y           += nobootmem.o
else
	obj-y           += bootmem.o
endif
```


由于接口是一致的, 那么他们共同使用一份

| 头文件 | bootmem接口 | nobootmem接口 |
|:-------:|:----------------:|:-------------------:|
| [include/linux/bootmem.h](http://lxr.free-electrons.com/source/include/linux/bootmem.h) | [mm/bootmem.c](http://lxr.free-electrons.com/source/mm/bootmem.c) | [mm/nobootmem.c](http://lxr.free-electrons.com/source/mm/nobootmem.c) |







##2.2.1	初始化阶段的引导内存分配器bootmem
-------


不管是否使用bootmem分配器是系统启动初期的内存分配方式，在耳熟能详的伙伴系统建立前内存都是利用bootmem分配器来分配的，伙伴系统框架建立起来后，bootmem会过度到伙伴系统.


在启动过程期间, 尽管内存管理尚未初始化, 但是内核仍然需要分配内存以创建各种数据结构. 因此在系统启动过程期间, 内核使用了一个额外的简化形式的内存管理模块**引导内存分配器(boot memory allocator--bootmem分配器)**, 用于在启动阶段早期分配内存, 而在系统初始化完成后, 该分配器被内核抛弃, 然后初始化了一套新的更加完善的内存分配器.

显然, 对该内存分配器的需求集中于简单性方面,　而不是性能和通用性, 它仅用于初始化阶段. 因此内核开发者决定实现一个最先适配(first-first)分配器用于在启动阶段管理内存. 这是可能想到的最简单的方式.


**引导内存分配器(boot memory allocator--bootmem分配器)**基于最先适配(first-first)分配器的原理(这儿是很多系统的内存分配所使用的原理), 使用一个位图来管理页, 以位图代替原来的空闲链表结构来表示存储空间, 位图的比特位的数目与系统中物理内存页面数目相同. 若位图中某一位是1, 则标识该页面已经被分配(已用页), 否则表示未被占有(未用页).

在需要分配内存时, 分配器逐位的扫描位图, 直至找到一个能提供足够连续页的位置, 即所谓的最先最佳(first-best)或最先适配位置.

该分配机制通过记录上一次分配的页面帧号(PFN)结束时的偏移量来实现分配大小小于一页的空间, 连续的小的空闲空间将被合并存储在一页上.



**为什么在系统运行时抛弃bootmem**

当系统运行时, 为何不继续使用bootmem分配机制呢?

*	其中一个关键原因在于 : 但它每次分配都必须从头扫描位图, 每次通过对内存域进行线性搜索来实现分配.

＊	其次首先适应算法容易在内存的起始断留下许多小的空闲碎片, 在需要分配较大的空间页时,　检查位图的成本将是非常高的.



引导内存分配器bootmem分配器简单却非常低效,　因此在内核完全初始化之后,　不能将该分配器继续欧诺个与内存管理,　而伙伴系统(连同slab, slub或者slob分配器)是一个好很多的备选方案．



如果使用bootmem机制, 那么在初始化的初期内核首先初始化了bootmem自身, 然后接着又用bootmem分配和初始化了内存结点和管理域, 因此初始化bootmem的工作主要分成两步

*	初始化bootmem自身的数据结构

*	用bootmem初始化内存结点管理域


###2.2.2	初始化阶段的nonbootmem
-------

但是bootmem也有很多问题. 最明显的就是外碎片的问题, 因此内核同时维护了nobootmem机制

bootmem是通过位图来管理，位图存在地地址段, 而memblock是在高地址管理内存, 维护两个链表, 即memory和 reserved

memory链表维护系统的内存信息(在初始化阶段通过bios获取的), 对于任何内存分配, 先去查找memory链表, 然后在reserve链表上记录(新增一个节点，或者合并)


1.	两者都可以分配小于一页的内存；

2.	两者都是就近查找可用的内存， bootmem是从低到高找， memblock是从高往低找；



#3	bootmem的初始化过程
-------


目前大多数架构下, 都已经不再使用bootmem来进行内存的初始化了, 但是由于历史的影子, 即使是nobootmem的系统中我们仍然能够看到bootmem的一些影子, 因此我们首先来讲解一下bootmem下的系统初始化





##3.1   bootmem_data描述内存引导区
-------

内核用bootmem_data表示引导内存区域

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

##3.2	初始化bootmem
-------


bootmem分配器的初始化是一个特定于体系结构的过程, 此外还取决于系统的内存布局

系统是从start_kernel开始启动的, 在启动过程中通过调用体系结构相关的setup_arch函数, 来获取初始化引导内存分配器所需的参数信息, 各种体系结构都有对应的函数来获取这些信息.

![i386上的初始化过程](../images/i386-setup_memory.jpg)


#4	memblock的初始化过程
-------


系统是从start_kernel开始启动的, 在启动过程中通过调用体系结构相关的setup_arch函数, 来获取初始化引导内存分配器所需的参数信息, 各种体系结构都有对应的函数来获取这些信息, 在获取信息完成后


| 调用层次 | 描述 | x86(已经不使用bootmem初始化) | arm | arm64 |
|:-------:|:---:|:---:|:---:|:-----:|
| setup_arch  | 设置特定体系的信息 | [arch/x86/kernel/setup.c](http://lxr.free-electrons.com/source/arch/x86/kernel/setup.c?v=4.7#L857), 但是不再调用paging_init | [arch/arm/kernel/setup.c](http://lxr.free-electrons.com/source/arch/arm/kernel/setup.c?v=4.7#L1073), 调用了[paging_init](http://lxr.free-electrons.com/source/arch/arm/kernel/setup.c?v=4.7#L1073) | [arch/arm64/kernel/setup.c](http://lxr.free-electrons.com/source/arch/arm64/kernel/setup.c?v=4.7#L266), 调用了[paging_init](http://lxr.free-electrons.com/source/arch/arm64/kernel/setup.c?v=4.7#L266)和[bootmem_init](http://lxr.free-electrons.com/source/arch/arm64/kernel/setup.c?v=4.7#L271) |
| paging_init | 初始化分页机制 | 定义了[arch/x86/mm/init_32.c](http://lxr.free-electrons.com/source/arch/x86/mm/init_32.c?v=4.7#L695)和[arch/x86/mm/init_64.c](http://lxr.free-electrons.com/source/arch/x86/mm/init_64.c?v=4.7#L579)两个版本 | 分别定义了[arch/arm/mm/nommu.c](http://lxr.free-electrons.com/source/arch/arm/mm/nommu.c?v=4.7#L311)和[arch/arm/mm/mmu.c](http://lxr.free-electrons.com/source/arch/arm/mm/mmu.c?v=4.7#L1623)两个版本, 均调用了bootmem_init | [arch/arm64/mm/mmu.c](http://lxr.free-electrons.com/source/arch/arm64/mm/mmu.c?v=4.7#L538) |
| bootmem_init | 初始化bootmem分配器 | 无定义 | [arch/arm/mm/init.c](http://lxr.free-electrons.com/source/arch/arm/mm/init.c?v=4.7#L282), 调用了zone_sizes_init | [arch/arm64/mm/init.c](http://lxr.free-electrons.com/source/arch/arm64/mm/init.c?v=4.7#L306),调用了zone_sizes_init |
|  zone_sizes_init　| 初始化节点和管理区<br>一般来说NUMA结构下会调用free_area_init_nodes完成所有内存结点的初始化, 而UMA结构下则会调用free_area_init_node完成唯一一个结点的初始化 | [arch/x86/mm/init.c](http://lxr.free-electrons.com/source/arch/x86/mm/init.c?v=4.7#L718), zone_sizes_init依据系统是NUMA还是UMA会有不同的定义 | [arch/arm/mm/init.c](http://lxr.free-electrons.com/source/arch/arm/mm/init.c?v=4.7#L137), 注意arm是非numa结构, 因此直接调用free_area_init_node完成初始化 | [arch/arm64/mm/init.c](http://lxr.free-electrons.com/source/arch/arm64/mm/init.c?v=4.7#L92) |
| [free_area_init_nodes](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L6460) | 初始化结点中所有内存区 | [mm/page_alloc.c](http://lxr.free-electrons.com/ident?i=free_area_init_nodes), 体系结构无关 | [mm/page_alloc.c](http://lxr.free-electrons.com/ident?i=free_area_init_nodes), 体系结构无关 | [mm/page_alloc.c](http://lxr.free-electrons.com/ident?i=free_area_init_nodes), 体系结构无关 |
| free_area_init_node | 初始化单个节点域  | | | |






下面我们就以标准的arm架构来分析bootmem初始化内存结点和内存域的过程, 在讲解的过程中我们会兼顾的考虑arm64架构下的异同

*	首先内核从[start_kernel](http://lxr.free-electrons.com/source/init/main.c?v=4.7#L505)开始启动


*	然后进入体系结构相关的设置部分[setup_arch](http://lxr.free-electrons.com/source/arch/arm/kernel/setup.c?v=4.7#L1073), 开始获取并设置指定体系结构的一些物理信息, 而arm64架构下则对应着[rch/arm64/kernel/setup.c](http://lxr.free-electrons.com/source/arch/arm64/kernel/setup.c?v=4.7#L229)


*	在setup_arch函数内, 通过paging_init函数初始化了分页机制和页表的信息


*	接着paging_init函数通过[bootmem_init](http://lxr.free-electrons.com/source/arch/arm/mm/mmu.c#L1642)开始进行初始化工作


arm64在整个初始化的流程上并没有什么不同, 但是有细微的差别


*	由于arm是在后期才开始加入了MMU内存管理单元的, 因此内核必须实现mmu和nonmmu两套不同的代码, 这主要是提现在分页机制的不同上, 因而paging_init分别定义了[arch/arm/mm/nommu.c](http://lxr.free-electrons.com/source/arch/arm/mm/nommu.c?v=4.7#L311)和[arch/arm/mm/mmu.c](http://lxr.free-electrons.com/source/arch/arm/mm/mmu.c?v=4.7#L1623)两个版本, 但是它们均调用了bootmem_init来完成初始化


*	也是因为上面的原因, arm上paging_init有两份代码([mmu](http://lxr.free-electrons.com/source/arch/arm/mm/mmu.c?v=4.7#L162)和[nonmmu](http://lxr.free-electrons.com/source/arch/arm/mm/nommu.c?v=4.7#L311)), 为了降低代码的耦合性, arm通过setup_arch调用paging_init函数, 后者进一步调用了bootmem_init来完成, 而arm64上不存在这样的问题, 则在[setup_arch中顺序的先用paging_init](http://lxr.free-electrons.com/source/arch/arm64/kernel/setup.c?v=4.7#L266)初始化了页表, 然后[setup_arch又调用bootmem_init](http://lxr.free-electrons.com/source/arch/arm64/kernel/setup.c?v4.7#L271)来完成了bootmem的初始化



##4.2	bootmem_init
-------

在paging_init之后, 系统的页帧已经建立起来, 然后通过bootmem_init中, 系统开始完成bootmem的初始化工作.


不同的体系结构bootmem_init的实现, 没有很大的区别, 但是在初始化的过程中, 其中的很多函数, 依据系统是NUMA还是UMA结构则有不同的定义



###4.2.1	bootmem_init函数

| 函数实现 | arm | arm64 |
|:---:|:---:|:-----:|
| bootmem_init | [arch/arm/mm/init.c, line 282](http://lxr.free-electrons.com/source/arch/arm/mm/init.c?v=4.7#L282) | [arch/arm64/mm/init.c, line 306](http://lxr.free-electrons.com/source/arch/arm64/mm/init.c?v=4.7#L306) |


```cpp
//  http://lxr.free-electrons.com/source/arch/arm/mm/init.c#L282
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

//  http://lxr.free-electrons.com/source/arch/arm64/mm/init.c#L306
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
```



###4.2.2	函数设置内存区域大小
-------



要想初始化内存区域, 首先要先获取内存域的大小(起始和结束), 内核通过min_low_pfn, max_low_pfn, max_pfn三个全局变量标识了内存页面的开始和结束. 这几个变量我们在之前的[struct zone详解中](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/kernel/02-memory/01-description/03-zone)中提到过, 内核也通过这些全局变量标记了物理内存所在页面的偏移, 这些变量定义在[mm/nobootmem.c?v4.7, line 31](http://lxr.free-electrons.com/source/mm/nobootmem.c?v4.7#L31)

| 变量 | 描述 |
|:----:|:---:|
| max_low_pfn 		|  max_low_pfn变量是由find_max_low_pfn函数计算并且初始化的，它被初始化成ZONE_NORMAL的最后一个page的位置。这个位置是kernel直接访问的物理内存, 也是关系到kernel/userspace通过“PAGE_OFFSET宏”把线性地址内存空间分开的内存地址位置 |
| min_low_pfn 		| 系统可用的第一个pfn是[min_low_pfn变量](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v4.7#L16), 开始与_end标号的后面, 也就是kernel结束的地方.在文件mm/bootmem.c中对这个变量作初始化
| max_pfn 			| 系统可用的最后一个PFN是[max_pfn变量](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v4.7#L21), 这个变量的初始化完全依赖与硬件的体系结构. |

**arm架构**下通过find_limits函数用来查找系统中可用内存区域的大小, 该函数定义在[arch/arm/mm/init.c?v=4.7, line 90](http://lxr.free-electrons.com/source/arch/arm/mm/init.c?v=4.7#L90)

```cpp
static void __init find_limits(unsigned long *min, unsigned long *max_low,
                   unsigned long *max_high)
{
    *max_low = PFN_DOWN(memblock_get_current_limit());
    *min = PFN_UP(memblock_start_of_DRAM());
    *max_high = PFN_DOWN(memblock_end_of_DRAM());
}
```

而**arm64架构**下, 则直接通过如下代码获取

```cpp
void __init bootmem_init(void)
{
	/* ......  */
    min = PFN_UP(memblock_start_of_DRAM());
    max = PFN_DOWN(memblock_end_of_DRAM());

    early_memtest(min << PAGE_SHIFT, max << PAGE_SHIFT);

    max_pfn = max_low_pfn = max;
    /*  ......  */
```




###4.2.3	zone_sizes_init初始化节点和内存域
-------


内核通过zone_sizes_init函数来初始化节点和管理区的一些数据项


| 函数实现 | arm | arm64 |
|:---:|:---:|:-----:|
| zone_sizes_init | [arch/arm/mm/init.c?v=4.7#L137](http://lxr.free-electrons.com/source/arch/arm/mm/init.c?v=4.7#L137) | [arch/arm64/mm/init.c?v=4.7, line 90](http://lxr.free-electrons.com/source/arch/arm64/mm/init.c?v=4.7#L90) |

**arm架构**下该函数定义在[arch/arm/mm/init.c?v=4.7#L137](http://lxr.free-electrons.com/source/arch/arm/mm/init.c?v=4.7#L137)中. 如下所示


```cpp
static void __init zone_sizes_init(unsigned long min, unsigned long max_low,
    unsigned long max_high)
{
    unsigned long zone_size[MAX_NR_ZONES], zhole_size[MAX_NR_ZONES];
    struct memblock_region *reg;

    /*
     * initialise the zones.
     */
    memset(zone_size, 0, sizeof(zone_size));

    /*
     * The memory size has already been determined.  If we need
     * to do anything fancy with the allocation of this memory
     * to the zones, now is the time to do it.
     */
    zone_size[0] = max_low - min;
#ifdef CONFIG_HIGHMEM
    zone_size[ZONE_HIGHMEM] = max_high - max_low;
#endif

    /*
     * Calculate the size of the holes.
     *  holes = node_size - sum(bank_sizes)
     */
    memcpy(zhole_size, zone_size, sizeof(zhole_size));
    for_each_memblock(memory, reg) {
        unsigned long start = memblock_region_memory_base_pfn(reg);
        unsigned long end = memblock_region_memory_end_pfn(reg);

        if (start < max_low) {
            unsigned long low_end = min(end, max_low);
            zhole_size[0] -= low_end - start;
        }
#ifdef CONFIG_HIGHMEM
        if (end > max_low) {
            unsigned long high_start = max(start, max_low);
            zhole_size[ZONE_HIGHMEM] -= end - high_start;
        }
#endif
    }

#ifdef CONFIG_ZONE_DMA
    /*
     * Adjust the sizes according to any special requirements for
     * this machine type.
     */
    if (arm_dma_zone_size)
        arm_adjust_dma_zone(zone_size, zhole_size,
            arm_dma_zone_size >> PAGE_SHIFT);
#endif

    free_area_init_node(0, zone_size, min, zhole_size);
}
```


内核在zone_sizes_init函数来中获取了三个管理区的页面数(即大小), 然后通过free_area_init_node函数来设置和初始化内存域


由于arm是非numa结构, 因为只需要通过
```cpp
free_area_init_node(0, zone_size, min, zhole_size);
```
设置唯一一个内存节点即可.


而**arm64架构**下则需要一句系统是NUMA还是UMA结构分别进行处理

*	如果是UMA结构则直接通过free_area_init_node初始化全局唯一的内存结点即可

```cpp
free_area_init_node(0, zone_size, min, zhole_size);

```

*	如果是NUMA结构则需要free_area_init_nodes使用初始化所有结点, 而该函数会进一步遍历所有的结点, 依次通过free_area_init_node完成内存结点的初始化

```cpp
free_area_init_nodes(max_zone_pfns);
```


arm64架构下zone_sizes_init的定义在[arch/arm64/mm/init.c?v=4.7, line 90](http://lxr.free-electrons.com/source/arch/arm64/mm/init.c?v=4.7#L90)


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


###4.2.4	free_area_init_nodes初始化内存节点
-------


内核通过zone_sizes_init函数来初始化节点和管理区的一些数据项


| 函数实现 | 体系结构无关 |
|:---:|:---:|:-----:|
| free_area_init_nodes | [mm/page_alloc.c?v=4.7, line 6460](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L6460) |
| free_area_init_node | [mm/page_alloc.c?v4.7, line 6076](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L6076) |



```cpp

/**
 * free_area_init_nodes - Initialise all pg_data_t and zone data
 * @max_zone_pfn: an array of max PFNs for each zone
 *
 * This will call free_area_init_node() for each active node in the system.
 * Using the page ranges provided by memblock_set_node(), the size of each
 * zone in each node and their holes is calculated. If the maximum PFN
 * between two adjacent zones match, it is assumed that the zone is empty.
 * For example, if arch_max_dma_pfn == arch_max_dma32_pfn, it is assumed
 * that arch_max_dma32_pfn has no pages. It is also assumed that a zone
 * starts where the previous one ended. For example, ZONE_DMA32 starts
 * at arch_max_dma_pfn.
 */
void __init free_area_init_nodes(unsigned long *max_zone_pfn)
{
    unsigned long start_pfn, end_pfn;
    int i, nid;

    /* Record where the zone boundaries are */
    memset(arch_zone_lowest_possible_pfn, 0,
                sizeof(arch_zone_lowest_possible_pfn));
    memset(arch_zone_highest_possible_pfn, 0,
                sizeof(arch_zone_highest_possible_pfn));
    arch_zone_lowest_possible_pfn[0] = find_min_pfn_with_active_regions();
    arch_zone_highest_possible_pfn[0] = max_zone_pfn[0];
    for (i = 1; i < MAX_NR_ZONES; i++) {
        if (i == ZONE_MOVABLE)
            continue;
        arch_zone_lowest_possible_pfn[i] =
            arch_zone_highest_possible_pfn[i-1];
        arch_zone_highest_possible_pfn[i] =
            max(max_zone_pfn[i], arch_zone_lowest_possible_pfn[i]);
    }
    arch_zone_lowest_possible_pfn[ZONE_MOVABLE] = 0;
    arch_zone_highest_possible_pfn[ZONE_MOVABLE] = 0;

    /* Find the PFNs that ZONE_MOVABLE begins at in each node */
    memset(zone_movable_pfn, 0, sizeof(zone_movable_pfn));
    find_zone_movable_pfns_for_nodes();

    /* Print out the zone ranges */
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
        if (zone_movable_pfn[i])
            pr_info("  Node %d: %#018Lx\n", i,
                   (u64)zone_movable_pfn[i] << PAGE_SHIFT);
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
    for_each_online_node(nid) {
        pg_data_t *pgdat = NODE_DATA(nid);
        free_area_init_node(nid, NULL,
                find_min_pfn_for_node(nid), NULL);

        /* Any memory on that node */
        if (pgdat->node_present_pages)
            node_set_state(nid, N_MEMORY);
        check_for_memory(pgdat, nid);
    }
}
```


###4.2.5	free_area_init_node初始化内存节点
-------

| 函数实现 | 体系结构无关 |
|:---:|:---:|:-----:|
| free_area_init_nodes | [mm/page_alloc.c?v=4.7, line 6460](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L6460) |
| free_area_init_node | [mm/page_alloc.c?v4.7, line 6076](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L6076) |


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
    calculate_node_totalpages(pgdat, start_pfn, end_pfn,
                  zones_size, zholes_size);

    alloc_node_mem_map(pgdat);
#ifdef CONFIG_FLAT_NODE_MEM_MAP
    printk(KERN_DEBUG "free_area_init_node: node %d, pgdat %08lx, node_mem_map %08lx\n",
        nid, (unsigned long)pgdat,
        (unsigned long)pgdat->node_mem_map);
#endif

    free_area_init_core(pgdat);
}
```


###4.2.6	free_area_init_core
-------


| 函数实现 | 体系结构无关 |
|:---:|:---:|:-----:|
| free_area_init_core | [mm/page_alloc.c?v=4.7#L5932](http://lxr.free-electrons.com/source/mm/page_alloc.c?v=4.7#L5932) |


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
    init_waitqueue_head(&pgdat->kswapd_wait);
    init_waitqueue_head(&pgdat->pfmemalloc_wait);
#ifdef CONFIG_COMPACTION
    init_waitqueue_head(&pgdat->kcompactd_wait);
#endif
    pgdat_page_ext_init(pgdat);

    for (j = 0; j < MAX_NR_ZONES; j++) {
        struct zone *zone = pgdat->node_zones + j;
        unsigned long size, realsize, freesize, memmap_pages;
        unsigned long zone_start_pfn = zone->zone_start_pfn;

        size = zone->spanned_pages;
        realsize = freesize = zone->present_pages;

        /*
         * Adjust freesize so that it accounts for how much memory
         * is used by this zone for memmap. This affects the watermark
         * and per-cpu initialisations
         */
        memmap_pages = calc_memmap_size(size, realsize);
        if (!is_highmem_idx(j)) {
            if (freesize >= memmap_pages) {
                freesize -= memmap_pages;
                if (memmap_pages)
                    printk(KERN_DEBUG
                           "  %s zone: %lu pages used for memmap\n",
                           zone_names[j], memmap_pages);
            } else
                pr_warn("  %s zone: %lu pages exceeds freesize %lu\n",
                    zone_names[j], memmap_pages, freesize);
        }

        /* Account for reserved pages */
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
        zone->managed_pages = is_highmem_idx(j) ? realsize : freesize;
#ifdef CONFIG_NUMA
        zone->node = nid;
        zone->min_unmapped_pages = (freesize*sysctl_min_unmapped_ratio)
                        / 100;
        zone->min_slab_pages = (freesize * sysctl_min_slab_ratio) / 100;
#endif
        zone->name = zone_names[j];
        spin_lock_init(&zone->lock);
        spin_lock_init(&zone->lru_lock);
        zone_seqlock_init(zone);
        zone->zone_pgdat = pgdat;
        zone_pcp_init(zone);

        /* For bootup, initialized properly in watermark setup */
        mod_zone_page_state(zone, NR_ALLOC_BATCH, zone->managed_pages);

        lruvec_init(&zone->lruvec);
        if (!size)
            continue;

        set_pageblock_order();
        setup_usemap(pgdat, zone, zone_start_pfn, size);
        ret = init_currently_empty_zone(zone, zone_start_pfn, size);
        BUG_ON(ret);
        memmap_init(size, nid, j, zone_start_pfn);
    }
}
```


#总结
-------

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">版权声明<img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br />本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可。

