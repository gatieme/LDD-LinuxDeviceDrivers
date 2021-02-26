伙伴系统的初始化
=======

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6393814) |



#1	前景回顾
------


##1.1	启动阶段的物理内存初始化
-------



之前我们讲解了系统内存管理初始化的时候, 有简单提过第二阶段(buddy的初始化), 但是我们讲解的很粗糙, 我们仅仅讲解了内存管理的主要流程创建


```cpp
start_kernel()
    |---->page_address_init()
    |     考虑支持高端内存
    |     业务：初始化page_address_pool链表；
    |          将page_address_maps数组元素按索引降序插入
    |          page_address_pool链表; 
    |          初始化page_address_htable数组.
    | 
    |---->setup_arch(&command_line);
    |     初始化特定体系结构的内容
    	  |
    	  |---->arm64_memblock_init( );
          |     初始化引导阶段的内存分配器memblock
          |
          |---->paging_init();
          |     分页机制初始化
          |
          |---->bootmem_init();
          |     始化内存数据结构包括内存节点, 内存域和页帧page
                |
                |---->arm64_numa_init();
                |     支持numa架构
                |
                |---->zone_sizes_init(min, max);
                    来初始化节点和管理区的一些数据项
                    |
                    |---->free_area_init
                    |   初始系统的所有内存节点(依次遍历各个 MEM NODE)
                    |
                        ---->free_area_init_node(int nid)
                            |	初始化当前 nid 的内存节点 
                            |
                            |---->free_area_init_core(pgdat);
                                 初始化当前节点的所有 ZONE(以此遍历各个 ZONE)
                                 |
                                 |---->usemap_size
                                 |     分配当前 ZONE 的 pageblock_flags
                                 |
                                 |---->memmap_init
                                 |	   初始化page页面
                |
                |---->memblock_dump_all();
                |   初始化完成, 显示memblock的保留的所有内存信息
         	   |
    |---->build_all_zonelist()
    |     为系统中的zone建立后备zone的列表.
    |     所有zone的后备列表都在
    |     pglist_data->node_zonelists[0]中;
    |
    |     期间也对per-CPU变量boot_pageset做了初始化. 
    |
```


```cpp
    |---->page_alloc_init()
         |---->hotcpu_notifier(page_alloc_cpu_notifier, 0);
         |     不考虑热插拔CPU 
         |
    |---->pidhash_init()
    |     详见下文.
    |     根据低端内存页数和散列度，分配hash空间，并赋予pid_hash
    |
    |---->vfs_caches_init_early()
          |---->dcache_init_early()
          |     dentry_hashtable空间，d_hash_shift, h_hash_mask赋值；
          |     同pidhash_init();
          |     区别:
          |         散列度变化了（13 - PAGE_SHIFT）;
          |         传入alloc_large_system_hash的最后参数值为0;
          |
          |---->inode_init_early()
          |     inode_hashtable空间，i_hash_shift, i_hash_mask赋值；
          |     同pidhash_init();
          |     区别:
          |         散列度变化了（14 - PAGE_SHIFT）;
          |         传入alloc_large_system_hash的最后参数值为0;
          |解buddy的内部机理,

```



我们只在特定于体系结构的代码中看到了内核如何检测系统中的可用内存。与高层数据结构(如内存域和结点)的关联, 则需要根据该信息构建。我们知道，体系结构相关代码需要在启动期间建立以下信息:

*	系统中各个内存域的页帧边界，保存在max_zone_pfn数组

*	各结点页帧的分配情况，保存在全局变量early_node_map中




##1.2	内存节点的初始化
-------



```cpp
|---->free_area_init
|   初始系统的所有内存节点(依次遍历各个 MEM NODE)
|
    ---->free_area_init_node(int nid)
        | 初始化当前 nid 的内存节点 
        |
        |---->free_area_init_core(pgdat);
             初始化当前节点的所有 ZONE(以此遍历各个 ZONE)
             |
             |---->usemap_size
             |     分配当前 ZONE 的 pageblock_flags
             |
             |---->memmap_init
             |     初始化page页面
```

## 1.3 ZONE 初始化
-------

free_area_init 中完成了系统内存的初始化操作.

其中 zone 的初始化由 [`free_area_init_core()`, v5.10](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L6834) 完成.

```cpp
|---->free_area_init_core
|     初始化当前节点的所有 ZONE(以此遍历各个 ZONE)
    |
    |---->memmap_pages = calc_memmap_size(size, freesize);
    |       初始化当前 nid 的内存节点 
    |
    |---->zone_init_internals(zone, j, nid, freesize);
    |       初始化当前节点的所有 ZONE(以此遍历各个 ZONE)
    |
    |---->set_pageblock_order();
    |
    |
    |---->setup_usemap(pgdat, zone, zone_start_pfn, size);
    |       初始化page页面
    |
    |---->init_currently_empty_zone(zone, zone_start_pfn, size);
    |
    |---->memmap_init(size, nid, j, zone_start_pfn);
```

* for (j = 0; j < MAX_NR_ZONES; j++) 依次遍历当前内存节点 pglist_data 的所有 ZONE

* [`setup_usemap()`](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L6688) 函数中通过 usemap_size() 计算存储 pageblock_flags 所需的内存大小 usemapsize, 并通过 memblock_alloc_node 为其分配空间. 其中 [`usemap_size()`, v5.10](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L6675)

* [`init_currently_empty_zone()`, v5.10](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L6396) 函数完成了 ZONE 初始化的最后操作, 并将 zone->initialized 标记置为 1, 至此 ZONE 的初始化完成了, 其中通过 zone_init_free_lists() 将各个 ORDER 的各个 MIGRATE_TYPES 类型的 BUDDY 都进行了初始化, 将 `free_area->free_list[]` 置为空列表数组, nr_free 置为 0. 

* [`memmap_init()`, v5.10](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L6188) 中通过 [`memmap_init_zone()`](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L6051)->[`set_pageblock_migratetype()`](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L572) 设置了每个页面的 MIGRATE_TYPE 类型为 MIGRATE_MOVABLE 可迁移类型.




##1.3	今日内容(buddy的初始化)
-------

至此, 内存节点, ZONE, BUDDY 都已经初始化好了, 但是页面还没有加到伙伴系统 BUDDY 中, 此外可以发现初始化的时候系统所有的内存都是 MIGRATE_MOVABLE 可以迁移的.

那么我们就很好奇了 :

1.  BUDDY 伙伴系统的 `free_area->free_list[]` 被初始化为 `[]` 数组了, nr_free 也是 0, 那么物理页面怎么加进来呢 ?

2.  系统初始化所有物理内存的迁移类型都是 MIGRATE_MOVABLE, 那如果想要分配 MIGRATE_UNMOVABLE 不可迁移的或者其他类型的内存, 该怎么搞 ?

真正完成伙伴系统中 `free_list` 初始化工作的是在 `free_low_memory_core_early()` 中，其代码调用图如下 :


```cpp
arch_call_rest_init
  rest_init()
    kernel_init_freeable()
      page_alloc_init_late
        memblock_discard
          __memblock_free_late
            memblock_free_pages
              __free_pages_core
                __free_pages_ok
free_one_page
__free_one_page
```

```cpp
mem_init
memblock_free_all
free_low_memory_core_early
free_memory_core
__free_pages_memory
memblock_free_pages
free_pages_core
__free_pages_ok
free_one_page
__free_one_page
```

[`free_low_memory_core_early()`, v5.10](https://elixir.bootlin.com/linux/v5.10/source/mm/memblock.c#L1960) 就完成了两个工作:

1.  [`reserve_bootmem_region()`](https://elixir.bootlin.com/linux/v5.10/source/mm/page_alloc.c#L1488) 把内核预留的内存 reserve 掉.

2.  [`__free_memory_core()`](https://elixir.bootlin.com/linux/v5.10/source/mm/memblock.c#L1945) 通过释放页面的方式把页面归还到 BUDDY 伙伴系统中.