的启动过程期间的内存管理--memblock分配器
=======



| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6225543) |

在内存管理的上下文中, 初始化(initialization)可以有多种含义. 在许多CPU上, 必须显式设置适用于Linux内核的内存模型. 例如在x86_32上需要切换到保护模式, 然后内核才能检测到可用内存和寄存器.




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



##1.2	启动过程中的内存初始化

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


##1.3	今日内容(非bootmem下的memblock内存管理)
-------


在引导内核的过程中, 需要使用内存, 而这个时候内核的内存管理并没有被创建, 因此也就需要一种精简的内存管理系统先接受这个工作, 而在初始化完成后, 再将旧的接口废弃, 转而使用强大的buddy系统来进行内存管理.


前面我们讲解了引导内存管理bootmem机制, 它基于最先适配算法, 早期的Linux内核在引导阶段都是通过bootmem来完成初期的内存管理的. 但是后来的版本(笔者分析的是3.19)开始把bootmem弃用了,`__alloc_memory_core_aarly()`取代了bootmem的`__alloc_memory_core()`来完成内存分配, 而后者其实就是调用的memblock来分配内的.



memblock算法是linux内核初始化阶段的一个内存分配器,本质上是取代了原来的bootmem算法.  memblock实现比较简单,而它的作用就是在page allocator初始化之前来管理内存,完成分配和释放请求.


为了保证系统的兼容性, 内核为bootmem和memblock提供了相同的API接口.

这样在编译Kernel的时候可以选择nobootmem或者bootmem 来在buddy system起来之前管理memory.
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


Memblock是在早期引导过程中管理内存的方法之一，此时内核内存分配器还没运行. Memblock以前被定义为Logical Memory Block( 逻辑内存块), 但根据[Yinghai Lu的补丁](https://lkml.org/lkml/2010/7/13/68), 它被重命名为memblock.



>参考
>
>[Linux 内核内存管理（1）](http://blog.jobbole.com/88452)
>
>[Linux内核初期内存管理---memblock](http://www.maxwellxxx.com/linuxmemblock)
>
>[Linux kernel memory management Part 1.](https://0xax.gitbooks.io/linux-insides/content/mm/linux-mm-1.html)

#2	memblock的数据结构
-------

##2.1	struct memblock结构
-------


首先来看下memblock结构的定义,文件是[include/linux/memblock.h](http://lxr.free-electrons.com/source/include/linux/memblock.h?v=4.7)

```cpp
struct memblock {
    bool bottom_up;  /* is bottom up direction? 
    如果true, 则允许由下而上地分配内存*/
    phys_addr_t current_limit; /*指出了内存块的大小限制*/	
    /*  接下来的三个域描述了内存块的类型，即预留型，内存型和物理内存*/
    struct memblock_type memory;
    struct memblock_type reserved;
#ifdef CONFIG_HAVE_MEMBLOCK_PHYS_MAP
    struct memblock_type physmem;
#endif
};
```

该结构体包含五个域。

| 字段 | 描述 |
|:---:|:----:|
| bottom_up | 表示分配器分配内存的方式<br>true:从低地址(内核映像的尾部)向高地址分配<br>false:也就是top-down,从高地址向地址分配内存. |
| current_limit | 指出了内存块的大小限制, 用于限制通过memblock_alloc的内存申请 |
| memory | 是可用内存的集合 |
| reserved | 已分配内存的集合 |
| physmem | 物理内存的集合(需要配置CONFIG_HAVE_MEMBLOCK_PHYS_MAP参数) |

接下来的三个域描述了内存块的类型

*	预留型

*	内存型

*	物理内存型(需要配置宏CONFIG_HAVE_MEMBLOCK_PHYS_MAP)


##2.2	struct memblock_type
-------

我们现在又接触到了一个数据结构[memblock_type](http://lxr.free-electrons.com/source/include/linux/memblock.h?v=4.7#L40), 它的定义在[include/linux/memblock.h?v=4.7, line 40](http://lxr.free-electrons.com/source/include/linux/memblock.h?v=4.7#L40)

```cpp
struct memblock_type
{
    unsigned long cnt;      /* number of regions */
    unsigned long max;      /* size of the allocated array */
    phys_addr_t total_size; /* size of all regions */
    struct memblock_region *regions;
};
```


该结构体存储的是内存类型信息



| 字段 | 描述 |
|:---:|:----:|
| cnt | 当前集合(memory或者reserved)中记录的内存区域个数 |
| max | 当前集合(memory或者reserved)中可记录的内存区域的最大个数 |
| total_size | 集合记录区域信息大小 |
| regions | 内存区域结构指针 |

它包含的域分别描述了当前内存块含有的内存区域数量，

所有内存区域的总共大小，已经分配的内存区域大小和一个指向memblock_region结构体的数组指针


##2.3	内存区域memblock_region
-------


memblock_region结构体描述了内存区域，它的定义在它的定义在[include/linux/memblock.h?v=4.7, line 31](http://lxr.free-electrons.com/source/include/linux/memblock.h?v=4.7#L31)


```cpp
struct memblock_region
{
    phys_addr_t base;
    phys_addr_t size;
    unsigned long flags;
#ifdef CONFIG_HAVE_MEMBLOCK_NODE_MAP
    int nid;
#endif
};
```
| 字段 | 描述 |
|:---:|:----:|
| base | 内存区域起始地址 |
| size | 内存区域大小 |
| flags | 标记 |
| nid | node号 |


##2.4	内存区域标识
-------


memblock_region的flags字段存储了当期那内存域的标识信息, 标识用enum变量来定义, 参见[include/linux/memblock.h?v=4.7, line 23](http://lxr.free-electrons.com/source/include/linux/memblock.h?v=4.7#L23)

```cpp

/* Definition of memblock flags. */
enum {
    MEMBLOCK_NONE       = 0x0,  /* No special request */
    MEMBLOCK_HOTPLUG    = 0x1,  /* hotpluggable region */
    MEMBLOCK_MIRROR     = 0x2,  /* mirrored region */
    MEMBLOCK_NOMAP      = 0x4,  /* don't add to kernel direct mapping */
};
```

##2.5	结构总体布局
-------


图示法可以用来展示以上结构体之间的关系：

```cpp
+---------------------------+   +---------------------------+
|         memblock          |   |                           |
|  _______________________  |   |                           |
| |        memory         | |   |       Array of the        |
| |      memblock_type    |-|-->|      membock_region       |
| |_______________________| |   |                           |
|                           |   +---------------------------+
|  _______________________  |   +---------------------------+
| |       reserved        | |   |                           |
| |      memblock_type    |-|-->|       Array of the        |
| |_______________________| |   |      memblock_region      |
|                           |   |                           |
+---------------------------+   +---------------------------+



```
Memblock主要包含三个结构体：memblock, memblock_type和memblock_region。现在我们已了解了Memblock, 接下来我们将看到Memblock的初始化过程。


##2.6	初始化memblock静态变量
-------


在编译时,会分配好memblock结构所需要的内存空间, 文件是[mm/memblock.c](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L28)

结构体memblock的初始化变量名和结构体名相同memblock


```cpp

static struct memblock_region memblock_memory_init_regions[INIT_MEMBLOCK_REGIONS] __initdata_memblock;
static struct memblock_region memblock_reserved_init_regions[INIT_MEMBLOCK_REGIONS] __initdata_memblock;
#ifdef CONFIG_HAVE_MEMBLOCK_PHYS_MAP
static struct memblock_region memblock_physmem_init_regions[INIT_PHYSMEM_REGIONS] __initdata_memblock;
#endif


struct memblock memblock __initdata_memblock = {
    .memory.regions     = memblock_memory_init_regions,
    .memory.cnt         = 1,    /* empty dummy entry */
    .memory.max         = INIT_MEMBLOCK_REGIONS,

    .reserved.regions       = memblock_reserved_init_regions,
    .reserved.cnt       = 1,    /* empty dummy entry */
    .reserved.max       = INIT_MEMBLOCK_REGIONS,

#ifdef CONFIG_HAVE_MEMBLOCK_PHYS_MAP
    .physmem.regions    = memblock_physmem_init_regions,
    .physmem.cnt        = 1,    /* empty dummy entry */
    .physmem.max        = INIT_PHYSMEM_REGIONS,
#endif

    .bottom_up          = false,
    .current_limit      = MEMBLOCK_ALLOC_ANYWHERE,
};
```



**__initdata_memblock宏指定存储位置**

我们可以注意到初始化使用了__initdata_memblock宏，它的定义在[include/linux/memblock.h?v=4.7, line 64](http://lxr.free-electrons.com/source/include/linux/memblock.h?v=4.7#L64), 如下所示

```cpp
#ifdef CONFIG_ARCH_DISCARD_MEMBLOCK
#define __init_memblock __meminit
#define __initdata_memblock __meminitdata
#else
#define __init_memblock
#define __initdata_memblock
#endif
```

如果启用`CONFIG_ARCH_DISCARD_MEMBLOCK`宏配置选项，memblock代码会被放到.init代码段, 在内核启动完成后 memblock代码会从.init代码段释放。


**3个memblock_type的初始化**


接下来的是memblock结构体中3个memblock_type类型数据 **memory**, **reserved**和**physmem**的初始化



它们的**memblock_typecnt域**(当前集合中区域个数)被初始化为1.
**memblock_typemax域**(当前集合中最大区域个数)被初始化为`INIT_MEMBLOCK_REGIONS`和`INIT_PHYSMEM_REGIONS`



其中`INIT_MEMBLOCK_REGIONS`为128, 参见[include/linux/memblock.h?v=4.7, line 20](http://lxr.free-electrons.com/source/include/linux/memblock.h?v=4.7#L20)

```cpp
#define INIT_MEMBLOCK_REGIONS   128
#define INIT_PHYSMEM_REGIONS    4
```

而**memblock_type.regions**域都是通过memblock_region数组初始化的, 所有的数组定义都带有__initdata_memblock宏


memblock结构体中最后两个域**bottom_up**内存分配模式被禁用(bottom_up = false, 因此内存分配方式为top-down.), 当前 Memblock的大小限制是`MEMBLOCK_ALLOC_ANYWHERE`为~(phys_addr_t)0即为0xffffffff.

```cpp
/* Flags for memblock_alloc_base() amd __memblock_alloc_base() */
#define MEMBLOCK_ALLOC_ANYWHERE (~(phys_addr_t)0)
#define MEMBLOCK_ALLOC_ACCESSIBLE       0
```






#3	Memblock-API函数接口
-------


##3.1	Memblock-API函数接口
-------


既然内核静态创建并初始化了__initdata_memblock这个变量, 那么memblock又是怎么运作的呢?


在上文中我提到过所有关于memblock的实现都在[mm/memblock.c](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7)源文件中


抛开其他的先不谈,如果要使用memblock,最上层函数一共就4个

```cpp
/////////////////////////////////////////
//   基本接口
/////////////////////////////////////////
// 向memory区中添加内存区域.
memblock_add(phys_addr_t base, phys_addr_t size)

//  向memory区中删除区域.
memblock_remove(phys_addr_t base, phys_addr_t size)

//  申请内存
memblock_alloc(phys_addr_t size, phys_addr_t align)


// 释放内存
memblock_free(phys_addr_t base, phys_addr_t size)


/////////////////////////////////////////
//   查找 & 遍历
/////////////////////////////////////////
//  在给定的范围内找到未使用的内存
phys_addr_t memblock_find_in_range(phys_addr_t start, phys_addr_t end, phys_addr_t size, phys_addr_t align)

//  反复迭代 memblock
for_each_mem_range(i, type_a, type_b, nid, flags, p_start, p_end, p_nid)




/////////////////////////////////////////
//   获取信息
/////////////////////////////////////////
//  获取内存区域信息
phys_addr_t get_allocated_memblock_memory_regions_info(phys_addr_t *addr);
//  获取预留内存区域信息
phys_addr_t get_allocated_memblock_reserved_regions_info(phys_addr_t *addr);

/////////////////////////////////////////
//   获取信息
/////////////////////////////////////////
#define memblock_dbg(fmt, ...) \
	if (memblock_debug) printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)

```


大致翻看了一下内核代码, 发现很少使用memblock_free(),因为很多地方都是申请了内存做永久使用的. 再者,其实在内核中通过memblock_alloc来分配内存其实比较少,一般都是在调用memblock底层的一些函数来简单粗暴的分配的.


##3.2	memblock_add将内存区域加入到memblock中
-------

###3.2.1	memblock_add函数
-------


memblock_add函数负责向memory区中添加内存区域, 有两个参数：物理基址和内存区域大小，并且把该内存区域添加到memblock。

memblock_add函数本身并没有什么, 它只是调用了[memblock_add_range函数](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L609)来完成工作, 定义在[mm/memblock.c?v=4.7, line 609](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L609)

```cpp
int __init_memblock memblock_add(phys_addr_t base, phys_addr_t size)
{
    memblock_dbg("memblock_add: [%#016llx-%#016llx] flags %#02lx %pF\n",
             (unsigned long long)base,
             (unsigned long long)base + size - 1,
             0UL, (void *)_RET_IP_);
	return memblock_add_range(&memblock.memory, base, size, MAX_NUMNODES, 0);
```


memblock_add传递的参数依次是 : 内存块类型(memory), 物理基址, 内存区域大小, 最大节点数(0如果CONFIG_NODES_SHIFT没有在配置文件中设置，不然就是CONFIG_NODES_SHIFT）和标志



###3.2.2	memblock_add_range函数代码
-------



memblock_add_range函数添加新的内存区域到内存块中, 定义在[mm/memblock.c?v=4.7, line 504](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L504)




*	首先，该函数检查给定的内存区域大小, 如果是0就返回.

*	在这之后, memblock_add_range用给定的memblock_type检查memblock结构体中是否存在内存区域

*	如果没有，我们就用给定的值填充新的memory_region然后返回

*	如果memblock_type不为空，我们就把新的内存区域添加到memblock_type类型的memblock中。

```cpp
/**
 * memblock_add_range - add new memblock region
 * @type: memblock type to add new region into
 * @base: base address of the new region
 * @size: size of the new region
 * @nid: nid of the new region
 * @flags: flags of the new region
 *
 * Add new memblock region [@base,@base+@size) into @type.  The new region
 * is allowed to overlap with existing ones - overlaps don't affect already
 * existing regions.  @type is guaranteed to be minimal (all neighbouring
 * compatible regions are merged) after the addition.
 *
 * RETURNS:
 * 0 on success, -errno on failure.
 */
int __init_memblock memblock_add_range(struct memblock_type *type,
                phys_addr_t base, phys_addr_t size,
                int nid, unsigned long flags)
{
    bool insert = false;
    phys_addr_t obase = base;
    ／*  获取内存区域的结束位置,
      *  memblock_cap_size函数会设置size大小确保base + size不会溢出  */
    phys_addr_t end = base + memblock_cap_size(base, &size);
    int idx, nr_new;
    struct memblock_region *rgn;

    if (!size)
        return 0;

    /* special case for empty array */
    if (type->regions[0].size == 0) {
        WARN_ON(type->cnt != 1 || type->total_size);
        type->regions[0].base = base;
        type->regions[0].size = size;
        type->regions[0].flags = flags;
        memblock_set_region_node(&type->regions[0], nid);
        type->total_size = size;
        return 0;
    }
repeat:
    /*
     * The following is executed twice.  Once with %false @insert and
     * then with %true.  The first counts the number of regions needed
     * to accomodate the new area.  The second actually inserts them.
     */
    base = obase;
    nr_new = 0;

    for_each_memblock_type(type, rgn) {
        phys_addr_t rbase = rgn->base;
        phys_addr_t rend = rbase + rgn->size;

        if (rbase >= end)
            break;
        if (rend <= base)
            continue;
        /*
         * @rgn overlaps.  If it separates the lower part of new
         * area, insert that portion.
         */
        if (rbase > base) {
#ifdef CONFIG_HAVE_MEMBLOCK_NODE_MAP
            WARN_ON(nid != memblock_get_region_node(rgn));
#endif
            WARN_ON(flags != rgn->flags);
            nr_new++;
            if (insert)
                memblock_insert_region(type, idx++, base,
                               rbase - base, nid,
                               flags);
        }
        /* area below @rend is dealt with, forget about it */
        base = min(rend, end);
    }

    /* insert the remaining portion */
    if (base < end) {
        nr_new++;
        if (insert)
            memblock_insert_region(type, idx, base, end - base,
                           nid, flags);
    }

    /*
     * If this was the first round, resize array and repeat for actual
     * insertions; otherwise, merge and return.
     */
    if (!insert) {
        while (type->cnt + nr_new > type->max)
            if (memblock_double_array(type, obase, size) < 0)
                return -ENOMEM;
        insert = true;
        goto repeat;
    } else {
        memblock_merge_regions(type);
        return 0;
    }
}
```

###3.2.3	memblock_add_range函数流程解析
-------


首先，我们用如下代码获得内存区域的结束位置：

```cpp
phys_addr_t end = base + memblock_cap_size(base, &size);
```

memblock_cap_size函数会设置size大小确保base + size不会溢出。该函数实现相当简单, 参见[mm/memblock.c?v=4.7, line 79](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L79)


```cpp
/* adjust *@size so that (@base + *@size) doesn't overflow, return new size */
static inline phys_addr_t memblock_cap_size(phys_addr_t base, phys_addr_t *size)
{
    return *size = min(*size, (phys_addr_t)ULLONG_MAX - base);
}
```


memblock_cap_size返回size和ULLONG_MAX - base中的最小值

在那之后我们得到了新的内存区域的结束地址, 然后

*	查内存区域是否重叠

*	将新的添加到memblock, 并且看是否能和已经添加到memblock中的内存区域进行合并

首先遍历所有已经存储的内存区域并检查有没有和新的内存区域重叠


```cpp
for_each_memblock_type(type, rgn) {
    phys_addr_t rbase = rgn->base;
    phys_addr_t rend = rbase + rgn->size;

    if (rbase >= end)
        break;
    if (rend <= base)
        continue;
    /*  ......  */
     /* area below @rend is dealt with, forget about it */
	base = min(rend, end);
}
```

如果新内存区域没有和已经存储在memblock的内存区域重叠, 把该新内存区域插入到memblock中. 如果有重叠通通过一个小巧的来完成冲突处理

```cpp
base = min(rend, end);
```



重叠检查完毕后, 新的内存区域已经是一块干净的不包含重叠区域的内存, 把新的内存区域插入到memblock中包含两步：

*	把新的内存区域中非重叠的部分作为独立的区域加入到memblock

*	合并所有相邻的内存区域

这个过程分为两次循环来完成, 由一个[标识变量insert](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L582)和[report代码跳转标签](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L543)控制

*	第一次循环的时候, 检查新内存区域是否可以放入内存块中并调用memblock_double_array， 而由于[insert = false](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L524), 则执行[!insert条件语句标记的代码块](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L591), 并设置[insert = true](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L595), 然后[goto 跳转到report标签](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L596)继续开始第二次循环

*	第二次循环中, insert = true, 则执行相应的insert == true的代码块, 并且执行[memblock_insert_region将新内存区域插入](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L583), 最后执行[memblock_merge_regions(type)](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L598)合并内存区域

这是第一次循环, 我们需要检查新内存区域是否可以放入内存块中并调用memblock_double_array:

```cpp
/*
 * If this was the first round, resize array and repeat for actual
 * insertions; otherwise, merge and return.
 */
if (!insert) {	/*  第一次执行的的时候insert == false  */
    while (type->cnt + nr_new > type->max)
        if (memblock_double_array(type, obase, size) < 0)
            return -ENOMEM;
    insert = true;
    goto repeat;
} else {
	/*  ......  */
}
```
memblock_double_array函数加倍给定的内存区域大小，然后把insert设为true再转到repeat标签.

第二次循环，从repeat标签开始经过同样的循环然后用memblock_insert_region函数把当前内存区域插入到内存块：

```cpp
/* insert the remaining portion */
if (base < end) {
    nr_new++;
    if (insert)
        memblock_insert_region(type, idx, base, end - base,
                       nid, flags);
}
```

由于我们在第一次循环中把insert设为true, 现在memblock_insert_region函数将会被调用


[memblock_insert_region函数](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L476)几乎和把新内存区域插入到空的memblock_type代码块有同样的实现, 定义在[mm/memblock.c?v=4.7, line 476](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L476)该函数获得最后一个内存区域：

```cpp
struct memblock_region *rgn = &type->regions[idx];
```

然后调用memmove函数移动该内存区域：

```cpp
memmove(rgn + 1, rgn, (type->cnt - idx) * sizeof(*rgn));
```

紧接着填充新内存区域memblock_region的base域，size域等等， 然后增大memblock_type的大小。

最后memblock_add_range函数调用[memblock_merge_regions](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L444)合并所有相邻且兼容的内存区域, 定义在[mm/memblock.c?v=4.7, line 444](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L444)


```cpp
/*
 * If this was the first round, resize array and repeat for actual
 * insertions; otherwise, merge and return.
 */
if (!insert) {
    /*  ......  */
} else {
    memblock_merge_regions(type);
    return 0;
}
```

##3.3	memblock_remove删除内存区域
-------



memblock_remove用来完成删除内存区域的工作, 该函数定义在[mm/memblock.c?v=4.7, line 710](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L710)

```cpp
//  http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L710
int __init_memblock memblock_remove(phys_addr_t base, phys_addr_t size)
{
    return memblock_remove_range(&memblock.memory, base, size);
}
```


##3.4	memblock_alloc申请内存
-------


而相比来说, 申请内存的函数memblock_alloc实现方式就比较麻烦了, 如下所示

`emblock_alloc(phys_addr_t size, phys_addr_t align`)其实就是在当前NODE在内存范围0-MEMBLOCK_ALLOC_ACCESSIBLE(其实是current_limit)中分配一个大小为size的内存区域.


###3.4.1	memblock_alloc函数代码
-------


```cpp
//  http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L727
phys_addr_t __init memblock_alloc(phys_addr_t size, phys_addr_t align)
{
    return memblock_alloc_base(size, align, MEMBLOCK_ALLOC_ACCESSIBLE);
}


//  http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L1192
phys_addr_t __init memblock_alloc_base(phys_addr_t size, phys_addr_t align, phys_addr_t max_addr)
{
    phys_addr_t alloc;

    alloc = __memblock_alloc_base(size, align, max_addr);

    if (alloc == 0)
        panic("ERROR: Failed to allocate 0x%llx bytes below 0x%llx.\n",
              (unsigned long long) size, (unsigned long long) max_addr);

    return alloc;
}

//  http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L1186
phys_addr_t __init __memblock_alloc_base(phys_addr_t size, phys_addr_t align, phys_addr_t max_addr)
{
    return memblock_alloc_base_nid(size, align, max_addr, NUMA_NO_NODE,
                       MEMBLOCK_NONE);
}

//  http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L1163
phys_addr_t __init __memblock_alloc_base(phys_addr_t size, phys_addr_t align, phys_addr_t max_addr)
{
    return memblock_alloc_base_nid(size, align, max_addr, NUMA_NO_NODE,
                       MEMBLOCK_NONE);
}
```
memblock_alloc()很粗暴的从能用的内存里分配, 而有些情况下需要从特定的内存范围内分配内存. 解决方法就是通过memblock_alloc_range_nid函数或者实现类似机制的函数

最终memblock_alloc的也是通过memblock_alloc_range_nid函数来完成内存分配的



###3.4.2	memblock_alloc_range_nid函数
-------


下面我们就来看看memblock_alloc_range_nid函数的实现, 该函数定义在[mm/memblock.c?v=4.7, line 1133](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L1133)

```cpp
//  http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L1133
static phys_addr_t __init memblock_alloc_range_nid(phys_addr_t size,
                    phys_addr_t align, phys_addr_t start,
                    phys_addr_t end, int nid, ulong flags)
{
    phys_addr_t found;

    if (!align)
        align = SMP_CACHE_BYTES;

    found = memblock_find_in_range_node(size, align, start, end, nid,
                        flags);
    if (found && !memblock_reserve(found, size)) {
        /*
         * The min_count is set to 0 so that memblock allocations are
         * never reported as leaks.
         */
        kmemleak_alloc(__va(found), size, 0, 0);
        return found;
    }
    return 0;
}
```


memblock_alloc_range_nid函数的主要工作如下

*	首先使用memblock_find_in_range_node指定内存区域和大小查找内存区域

*	memblock_reserve后将其标为已经分配


###3.4.3	memblock_find_in_range_node函数
-------

该函数定义在[mm/memblock.c?v=4.7, lien 178](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L178)


```cpp
//  http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L178
phys_addr_t __init_memblock memblock_find_in_range_node(phys_addr_t size,
                    phys_addr_t align, phys_addr_t start,
                    phys_addr_t end, int nid, ulong flags)
{
    phys_addr_t kernel_end, ret;

    /* pump up @end */
    if (end == MEMBLOCK_ALLOC_ACCESSIBLE)
        end = memblock.current_limit;

    /* avoid allocating the first page */
    start = max_t(phys_addr_t, start, PAGE_SIZE);
    end = max(start, end);
    kernel_end = __pa_symbol(_end);

    /*
     * try bottom-up allocation only when bottom-up mode
     * is set and @end is above the kernel image.
     */
    if (memblock_bottom_up() && end > kernel_end) {
        phys_addr_t bottom_up_start;

        /* make sure we will allocate above the kernel */
        bottom_up_start = max(start, kernel_end);

        /* ok, try bottom-up allocation first */
        ret = __memblock_find_range_bottom_up(bottom_up_start, end,
                              size, align, nid, flags);
        if (ret)
            return ret;

        /*
         * we always limit bottom-up allocation above the kernel,
         * but top-down allocation doesn't have the limit, so
         * retrying top-down allocation may succeed when bottom-up
         * allocation failed.
         *
         * bottom-up allocation is expected to be fail very rarely,
         * so we use WARN_ONCE() here to see the stack trace if
         * fail happens.
         */
        WARN_ONCE(1, "memblock: bottom-up allocation failed, memory hotunplug may be affected\n");
    }

    return __memblock_find_range_top_down(start, end, size, align, nid,
                          flags);
}
```
*	如果从memblock_alloc过来, end就是MEMBLOCK_ALLOC_ACCESSIBLE,这个时候会设置为current_limit.

*	如果不通过memblock_alloc分配, 内存范围就是指定的范围. 紧接着对start做调整，为的是避免申请到第一个页面

memblock_bottom_up返回的是memblock.bottom_up，前面初始化的时候也知道这个值是false（在numa初始化时会设置为true），所以初始化前期应该调用的是__memblock_find_range_top_down函数去查找内存:




###3.4.4	__memblock_find_range_top_down查找内存区域
-------

最后通过[__memblock_find_range_top_down](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L140)函数去查找内存


```cpp
//  http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L140
static phys_addr_t __init_memblock
__memblock_find_range_top_down(phys_addr_t start, phys_addr_t end,
                   phys_addr_t size, phys_addr_t align, int nid,
                   ulong flags)
{
    phys_addr_t this_start, this_end, cand;
    u64 i;

    for_each_free_mem_range_reverse(i, nid, flags, &this_start, &this_end,
                    NULL) {
        this_start = clamp(this_start, start, end);
        this_end = clamp(this_end, start, end);

        if (this_end < size)
            continue;

        cand = round_down(this_end - size, align);
        if (cand >= this_start)
            return cand;
    }

    return 0;
}
```


*	函数通过使用for_each_free_mem_range_reverse宏封装调用__next_free_mem_range_rev()函数，此函数逐一将memblock.memory里面的内存块信息提取出来与memblock.reserved的各项信息进行检验，确保返回的this_start和this_end不会是分配过的内存块。

*	然后通过clamp取中间值，判断大小是否满足，满足的情况下，将自末端向前（因为这是top-down申请方式）的size大小的空间的起始地址（前提该地址不会超出this_start）返回回去

至此满足要求的内存块算是找到了。


###3.4.5	memblock_reserve标记申请的内存
-------


现在我们回到[memblock_alloc_range_nid函数](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L1133), 我们说该函数完成了两项工作

*	首先通过memblock_find_in_range_node指定内存区域和大小查找内存区域

*	找到内存区域后, 调用memblock_reserve后将其标为已经分配

现在我们已经找到了内存区域了, 那么我们继续看看memblock_reserve函数是如何堆内存进行标记的, 该函数定义在[mm/memblock.c?v=4.7, line 727](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L727)

```cpp
//  http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L727
int __init_memblock memblock_reserve(phys_addr_t base, phys_addr_t size)
{
    memblock_dbg("memblock_reserve: [%#016llx-%#016llx] flags %#02lx %pF\n",
             (unsigned long long)base,
             (unsigned long long)base + size - 1,
             0UL, (void *)_RET_IP_);

    return memblock_add_range(&memblock.reserved, base, size, MAX_NUMNODES, 0);
}

//  http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L609
int __init_memblock memblock_add(phys_addr_t base, phys_addr_t size)
{
    memblock_dbg("memblock_add: [%#016llx-%#016llx] flags %#02lx %pF\n",
             (unsigned long long)base,
             (unsigned long long)base + size - 1,
             0UL, (void *)_RET_IP_);

    return memblock_add_range(&memblock.memory, base, size, MAX_NUMNODES, 0);
}
```

我们会发现首先memblock_reserve函数也是通过memblock_add_range来实现的, 我们把memblock_add的实现贴出来进行对比, 我们会发现他们就第一个参数不一样

*	memblock_reserve使用全局变量memblock的reserved域, 最终将分配到的内存块信息添加到reserved区域中

*	emblock_add则使用了全局变量的memory域, 最终将内存块添加到了memory区域


memblock_add_range函数的流程我们前面已经将的很详细了, 这里只简单的叙述一下子

*	如果memblock算法管理内存为空的时候，则将当前空间添加进去

*	不为空的情况下，则先检查是否存在内存重叠的情况，如果有的话，则剔除重叠部分，然后将其余非重叠的部分添加进去

*	如果出现region[]数组空间不够的情况，则通过memblock_double_array()添加新的region[]空间

*	最后通过memblock_merge_regions()把紧挨着的内存合并了





##2.5	memblock_free释放内存区域
-------


```cpp
//  http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L695
static int __init_memblock memblock_remove_range(struct memblock_type *type,
                      phys_addr_t base, phys_addr_t size)
{
    int start_rgn, end_rgn;
    int i, ret;

    ret = memblock_isolate_range(type, base, size, &start_rgn, &end_rgn);
    if (ret)
        return ret;

    for (i = end_rgn - 1; i >= start_rgn; i--)
        memblock_remove_region(type, i);
}

//  http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L716
int __init_memblock memblock_free(phys_addr_t base, phys_addr_t size)
{
    memblock_dbg("   memblock_free: [%#016llx-%#016llx] %pF\n",
             (unsigned long long)base,
             (unsigned long long)base + size - 1,
             (void *)_RET_IP_);

    kmemleak_free_part(__va(base), size);
    return memblock_remove_range(&memblock.reserved, base, size);
}
```
#3	兼容bootmem的接口API
-------



##3.1	memblock与bootmem
-------

我们了解到memblock是作为bootmem的替代品而引入内核的

在编译Kernel的时候可以选择nobootmem或者bootmem 来在buddy system起来之前管理memory.
这两种机制对提供的API是一致的，因此对用户是透明的

参见[mm/Makefile](http://lxr.free-electrons.com/source/mm/Makefile#L44)


```cpp
ifdef CONFIG_NO_BOOTMEM
	obj-y           += nobootmem.o
else
	obj-y           += bootmem.o
endif
```


为了保证系统的兼容性, 内核为bootmem和memblock提供了相同的API接口.



##3.2	memblock与bootmem接口对比
-------


由于接口是一致的, 那么他们共同使用一份

| 头文件 | bootmem接口 | nobootmem接口 |
|:-------:|:----------------:|:-------------------:|
| [include/linux/bootmem.h](http://lxr.free-electrons.com/source/include/linux/bootmem.h) | [mm/bootmem.c](http://lxr.free-electrons.com/source/mm/bootmem.c) | [mm/nobootmem.c](http://lxr.free-electrons.com/source/mm/nobootmem.c) |



<br>
我们知道memblock自己的接口都在自己的头文件和源文件中

| memblock 头文件 | memblock接口 |
|:--------------:|:------------:|
| [include/linux/memblock.h](http://lxr.free-electrons.com/source/include/linux/memblock.h?v=4.7) | [mm/memblock.c](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7)


为了实现接口兼容, 内核用[mm/memblock.c](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7)定义的memblock接口, 实现了一套bootmem的接口机制, 而bootmem的接口我们在上一篇引导内存管理bootmem机制中已经讲过了, 这些实现的bootmem函数接口API, 就定义在[mm/nobootmem.c](http://lxr.free-electrons.com/source/mm/nobootmem.c)文件中, 然后内核把他们进行了封装, 然后提供了与bootmem相同功能和函数的接口, 这些接口都在[include/linux/memblock.h](http://lxr.free-electrons.com/source/include/linux/memblock.h?v=4.7).



在NUMA系统上, 基本的API是相同的, 但是函数增加了_node后缀, 与UMA系统的函数相比, 还需要一些额外的参数, 用于指定内存分配的结点.


| 函数 | bootmem定义 | nobootmem定义 |
|:-----:|:------------------:|:----------------------:|
| ZONE_NORMAL的分配函数 | 按照指定大小在ZONE_NORMAL内存域分配函数. 数据是对齐的, 这使得内存或者从可适用于L1高速缓存的理想位置开始| |
| alloc_bootmem(size) |  [alloc_bootmem](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L122)<br>[__alloc_bootmem](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L700)<br>[___alloc_bootmem](http://lxr.free-electrons.com/source/mm/bootmem.c?=4.7#L672) | [alloc_bootmem](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L122)<br>[__alloc_bootmem](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L309)<br>[___alloc_bootmem](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L281)  |
| alloc_bootmem_align(size) | [alloc_bootmem_align](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L124)<br>基于__alloc_bootmem实现 | [alloc_bootmem_align](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L124)<br>[__alloc_bootmem](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L309)<br>[___alloc_bootmem](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L281)  |
| alloc_bootmem_pages(size)) | [alloc_bootmem_pages](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L128)<br>基于__alloc_bootmem实现 | [alloc_bootmem_pages](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L128)<br>基于__alloc_bootmem实现 |
|  alloc_bootmem_nopanic(size) | [alloc_bootmem_nopanic](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L126)<br>[__alloc_bootmem_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L664)<br>[___alloc_bootmem_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L632) | [alloc_bootmem_nopanic](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L126)<br>[__alloc_bootmem_nopanic](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L261)<br>[___alloc_bootmem_nopanic](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L235) |
| ZONE_DMA区域的分配函数 |  |
| alloc_bootmem_low(size) | [alloc_bootmem_low](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L141)<br>[__alloc_bootmem_low](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L415)底层基于___alloc_bootmem | [alloc_bootmem_low](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L141)<br>底层基于___alloc_bootmem  |
| alloc_bootmem_low_pages_nopanic(size) | [alloc_bootmem_low_pages_nopanic](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L143)<br>底层基于[__alloc_bootmem_low_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L838)<br>[___alloc_bootmem_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L632) | [alloc_bootmem_low_pages_nopanic](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L143)<br>底层基于[__alloc_bootmem_low_nopanic](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L421)<br>[___alloc_bootmem_nopanic](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L235) |
| alloc_bootmem_low_pages(size) | [alloc_bootmem_low_pages](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L832)<br>底层基于[__alloc_bootmem_low_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L838) | [alloc_bootmem_low_pages](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L832)<br>底层基于[__alloc_bootmem_low_nopanic](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L421) |
| NUMA结构的分配函数 | | |
| alloc_bootmem_node(pgdat, size) |  [alloc_bootmem_node](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L132)<br>[__alloc_bootmem_node](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L777)<br>[ ___alloc_bootmem_node](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L708) | [alloc_bootmem_node](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L132)<br>[__alloc_bootmem_node](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L383)<br>[ ___alloc_bootmem_node](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L317) | 
| alloc_bootmem_node_nopanic(pgdat, size) |  [alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L134)<br>[__alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L738)<br>[___alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L708) | [alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L134)<br>[__alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L344)<br>[___alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L317) |
| alloc_bootmem_pages_node(pgdat, size) |  [alloc_bootmem_pages_node](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L136)<br>[__alloc_bootmem_node](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L747)<br>[___alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L708) | [alloc_bootmem_pages_node](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L136)<br>[__alloc_bootmem_node](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L344)<br>[___alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L317) |
| alloc_bootmem_pages_node_nopanic(pgdat, size) |  [alloc_bootmem_pages_node_nopanic](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L138)<br>[__alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L344)<br>[___alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L317) |
| alloc_bootmem_low_pages_node(pgdat, size) | [alloc_bootmem_low_pages_node](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L14)<br>[__alloc_bootmem_low_node](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L861)<be>[___alloc_bootmem_node](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L747)<br>[___alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L707) | [alloc_bootmem_low_pages_node](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L14)<br>[__alloc_bootmem_low_node](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L444)<be>[___alloc_bootmem_node](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L353)<br>[___alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L317) |


我们可以看到最基本的实现思路都是一样的, 只是最底层的实现函数有细微的区别.


##3.3	实现差异
-------


*	UMA结构下这些函数最终都是通过___alloc_bootmem_nopanic函数来实现的

*	NUMA架构下, 最终这些函数都是简介的调用___alloc_bootmem_node_nopanic函数来实现的,




| 函数 | bootmem | memblock |
|:-----:|:-----------:|:------------:|
| ___alloc_bootmem_nopanic | [mm/bootmem.c?v=4.7, line 632](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L632), 通过[alloc_bootmem_core](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L607)函数来实现 | [mm/nobootmem.c?v=4.7, line 235](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L235), 通过[__alloc_memory_core_early](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L36) |
| ___alloc_bootmem_node_nopanic | [mm/bootmem.c?v=4.7, line 708](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L708), 通过调用[alloc_bootmem_core](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L607)和[alloc_bootmem_bdata](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L500)来实现 | [mm/nobootmem.c?v=4.7, line 317](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L317), 通过调用[__alloc_memory_core_early](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L36)来实现 |


bootmem的核心函数__alloc_memory_core()的实现机制我们前一篇博文[引导分配器bootmem](待添加链接)已经讲过了, 那么memblock下nobootmem的核心函数__alloc_memory_core_early是怎么实现的呢?

前面[2.4.1节memblock_alloc函数代码](待添加链接)我们分析memblock_alloc函数的时候提到, 该函数最终通过memblock_alloc_range_nid函数粗暴粗暴的进行内存分配, 而有些情况下需要从特定的内存范围内分配内存. 解决方法就是通过memblock_alloc_range_nid函数或者实现类似机制的函数, 这里的__alloc_memory_core_early函数就是基于memblock_alloc_range_nid同样的思路实现的函数


*	首先使用memblock_find_in_range_node指定内存区域和大小查找内存区域

*	memblock_reserve后将其标为已经分配


我们列出 memblock_alloc_range_nid函数与__alloc_memory_core_early函数的实现对比

| memblock_alloc_range_nid | __alloc_memory_core_early |
|:------------------------------:|:------------------------------:|
| [mm/memblock.c?v=4.7, line 1133](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L1133) | [mm/nobootmem.c?v=4.7, line 36](http://lxr.free-electrons.com/source/mm/nobootmem.c?v=4.7#L36) |


#4	memblock初始化
-------

如果从整个linux生命周期来讲,涉及到各种初始化等,这里来详细分析,因为还没有分析完内核,所以这里是分析到哪里就记录到哪里了.

##4.1	x86架构下的memblock初始化
-------

要理解memblock是如何工作和实现的, 我们首先看一下它的用法.



在Linux内核中有几处用到了memblock, 例如 arch/x86/kernel/e820.c中的函数`memblock_x86_fill`. 该函数遍历由`e820`提供的内存映射表并且通过`memblock_add`函数把内核预留的内存区域添加到memblock。既然我们首先遇到了`memblock_add`函数，那就从它开始吧。


在内核初始化初期,物理内存会通过`Int 0x15`来被探测和整理, 存放到`e820`中.而初始化就发生在这个以后. 参见[arch/x86/kernel/setup.c?v=4.7, line 1096](http://lxr.free-electrons.com/source/arch/x86/kernel/setup.c?v=4.7#L1096)

```cpp
void __init setup_arch(char **cmdline_p)
{
    /*
     * Need to conclude brk, before memblock_x86_fill()
     *  it could use memblock_find_in_range, could overlap with
     *  brk area.
     */
    reserve_brk();

    cleanup_highmap();

    memblock_set_current_limit(ISA_END_ADDRESS);
    memblock_x86_fill();
}
```

首先内核建立内核页表需要扩展__brk, 而扩展后的brk就立即被声明为已分配. 这项工作是由reserve_brk通过调用memblock_reserve完成的， 而其实并不是正真通过memblock分配的, 因为此时memblock还没有完成初始化

[reserve_brk](http://lxr.free-electrons.com/source/arch/x86/kernel/setup.c?v=4.9#L209)函数定义在[arch/x86/kernel/setup.c?v=4.7, line 209](http://lxr.free-electrons.com/source/arch/x86/kernel/setup.c?v=4.9#L209), 此时memblock还没有初始化, 只能通过[memblock_reserve](http://lxr.free-electrons.com/source/mm/memblock.c?v=4.7#L727)来完成内存的分配

```cpp
static void __init reserve_brk(void)
{
    if (_brk_end > _brk_start)
        memblock_reserve(__pa_symbol(_brk_start),
                 _brk_end - _brk_start);

    /* Mark brk area as locked down and no longer taking any
       new allocations */
    _brk_start = 0;
}
```

设置完__brk后, 可以看到,setup_arch()函数通过memblock_x86_fill(),依据e820中的信息来初始化memblock.

```cpp
void __init memblock_x86_fill(void)
{
    int i;
    u64 end;

    /*
     * EFI may have more than 128 entries
     * We are safe to enable resizing, beause memblock_x86_fill()
     * is rather later for x86
     */
    memblock_allow_resize();

    for (i = 0; i < e820.nr_map; i++) {
        struct e820entry *ei = &e820.map[i];

        end = ei->addr + ei->size;
        if (end != (resource_size_t)end)
            continue;

        if (ei->type != E820_RAM && ei->type != E820_RESERVED_KERN)
            continue;

        memblock_add(ei->addr, ei->size);
    }

    /* throw away partial pages */
    memblock_trim_memory(PAGE_SIZE);

    memblock_dump_all();
}
```

比较简单,通过e820中的信息memblock_add(),将内存添加到memblock中的memory中,当做可分配内存.后两个函数主要是修剪内存使之对齐和输出信息.

至此, 我们的memblock就初始化好了, 简单而且粗暴



##4.2	arm架构下的memblock初始化
-------


arm下的memblock初始化也是从start_kernel()->[setup_arch()](http://lxr.free-electrons.com/source/arch/arm/kernel/setup.c#L1034)开始的, 在setup_arch()中arm架构通过arm_memblock_init完成了memblock的初始化工作.

```cpp
void __init setup_arch(char **cmdline_p)
{
	arm_memblock_init(mdesc);
}
```

arm_memblock_init定义在[arch/arm/mm/init.c](http://lxr.free-electrons.com/source/arch/arm/mm/init.c?v=4.7#L230), 如下所示


```cpp
void __init arm_memblock_init(const struct machine_desc *mdesc)
{
    /* Register the kernel text, kernel data and initrd with memblock. */
#ifdef CONFIG_XIP_KERNEL
    memblock_reserve(__pa(_sdata), _end - _sdata);
#else
    memblock_reserve(__pa(_stext), _end - _stext);
#endif
#ifdef CONFIG_BLK_DEV_INITRD
    /* FDT scan will populate initrd_start */
    if (initrd_start && !phys_initrd_size) {
        phys_initrd_start = __virt_to_phys(initrd_start);
        phys_initrd_size = initrd_end - initrd_start;
    }
    initrd_start = initrd_end = 0;
    if (phys_initrd_size &&
        !memblock_is_region_memory(phys_initrd_start, phys_initrd_size)) {
        pr_err("INITRD: 0x%08llx+0x%08lx is not a memory region - disabling initrd\n",
               (u64)phys_initrd_start, phys_initrd_size);
        phys_initrd_start = phys_initrd_size = 0;
    }
    if (phys_initrd_size &&
        memblock_is_region_reserved(phys_initrd_start, phys_initrd_size)) {
        pr_err("INITRD: 0x%08llx+0x%08lx overlaps in-use memory region - disabling initrd\n",
               (u64)phys_initrd_start, phys_initrd_size);
        phys_initrd_start = phys_initrd_size = 0;
    }
    if (phys_initrd_size) {
        memblock_reserve(phys_initrd_start, phys_initrd_size);

        /* Now convert initrd to virtual addresses */
        initrd_start = __phys_to_virt(phys_initrd_start);
        initrd_end = initrd_start + phys_initrd_size;
    }
#endif

    arm_mm_memblock_reserve();

    /* reserve any platform specific memblock areas */
    if (mdesc->reserve)
        mdesc->reserve();

    early_init_fdt_reserve_self();
    early_init_fdt_scan_reserved_mem();

    /* reserve memory for DMA contiguous allocations */
    dma_contiguous_reserve(arm_dma_limit);

    arm_memblock_steal_permitted = false;
    memblock_dump_all();
}
```

##4.3	arm64下的memblock初始化
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




其中arm64_memblock_init就完成了arm64架构下的memblock的初始化


与arm架构类似, arm64的memblock初始化没有意外, 只是初始化函数成为[arm64_memblock_init()](http://lxr.free-electrons.com/source/arch/arm64/kernel/setup.c?v=4.7#L261), 该函数定义在[arch/arm64/mm/init.c?v=4.7, line 192](http://lxr.free-electrons.com/source/arch/arm64/mm/init.c?v=4.7#L192)



#5	总结
-------


memblock内存管理是将所有的物理内存放到`memblock.memory`中作为可用内存来管理, 分配过的内存只加入到`memblock.reserved`中, 并不从`memory`中移出.

同理释放内存也会加入到`memory`中. 也就是说, `memory`在`fill`过后基本就是不动的了. 申请和分配内存仅仅修改`reserved`就达到目的. 在初始化阶段没有那么多复杂的内存操作场景, 甚至很多地方都是申请了内存做永久使用的, 所以这样的内存管理方式已经足够凑合着用了, 毕竟内核也不指望用它一辈子. 在系统完成初始化之后所有的工作会移交给强大的`buddy`系统来进行内存管理

