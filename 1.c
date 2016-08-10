在NUMA系统上, 基本的API是相同的, 但是函数增加了_node后缀, 与UMA系统的函数相比, 还需要一些额外的参数, 用于指定内存分配的结点.


| 函数 | 定义 |
|:---:|:---:|

| alloc_bootmem(size) | 按照指定大小在ZONE_NORMAL内存域分配函数. 数据是对齐的, 这使得内存或者从可适用于L1高速缓存的理想位置开始| [alloc_bootmem](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L122)<br>[__alloc_bootmem](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L700)<br>[___alloc_bootmem](http://lxr.free-electrons.com/source/mm/bootmem.c?=4.7#L672) |
| alloc_bootmem_align(x, align) | 同alloc_bootmem函数, 按照指定大小在ZONE_NORMAL内存域分配函数, 并按照align进行数据对齐 | [alloc_bootmem_align](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L124)<br>基于__alloc_bootmem实现 |
| alloc_bootmem_pages(size)) | 同alloc_bootmem函数, 按照指定大小在ZONE_NORMAL内存域分配函数, 其中_page只是指定数据的对其方式从页边界(__pages)开始  |  [alloc_bootmem_pages](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L128)<br>基于__alloc_bootmem实现 |
|  alloc_bootmem_nopanic(size) | alloc_bootmem_nopanic是最基础的通用的，一个用来尽力而为分配内存的函数，它通过list_for_each_entry在全局链表bdata_list中分配内存. alloc_bootmem和alloc_bootmem_nopanic类似，它的底层实现首先通过alloc_bootmem_nopanic函数分配内存，但是一旦内存分配失败，系统将通过panic("Out of memory")抛出信息，并停止运行 | [alloc_bootmem_nopanic](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L126)<br>[__alloc_bootmem_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L664)<br>[___alloc_bootmem_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L632) |

| alloc_bootmem_low(size) | 按照指定大小在ZONE_DMA内存域分配函数. 类似于alloc_bootmem, 数据是对齐的 | [alloc_bootmem_low_pages_nopanic](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L141)<br>底层基于___alloc_bootmem |
| alloc_bootmem_low_pages_nopanic(size) | 按照指定大小在ZONE_DMA内存域分配函数. 类似于alloc_bootmem_pages, 数据在页边界对齐, 并且错误后不输出panic | [alloc_bootmem_low_pages_nopanic](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L143)<br>底层基于[__alloc_bootmem_low_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L838)
| alloc_bootmem_low_pages(size) | 按照指定大小在ZONE_DMA内存域分配函数. 类似于alloc_bootmem_pages, 数据在页边界对齐 | [alloc_bootmem_low_pages](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L832)<br>底层基于[__alloc_bootmem_low_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L838) |


| alloc_bootmem_node(pgdat, size) |  [alloc_bootmem_node](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L132)<br>[__alloc_bootmem_node](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L777)<br>[ ___alloc_bootmem_node](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L708)|
| alloc_bootmem_node_nopanic(pgdat, size) |  [alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L134)<br>[__alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L738)<br>[___alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L708) |
| alloc_bootmem_pages_node(pgdat, size) |  [alloc_bootmem_pages_node](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L136)<br>[__alloc_bootmem_node](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L747)<br>[___alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L708) |
| alloc_bootmem_pages_node_nopanic(pgdat, size) |  [alloc_bootmem_pages_node_nopanic](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L138)<br>[__alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L738)<br>[___alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L708) |
| alloc_bootmem_low_pages_node(pgdat, size) | [alloc_bootmem_low_pages_node](http://lxr.free-electrons.com/source/include/linux/bootmem.h?v=4.7#L14)<br>[__alloc_bootmem_low_node](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L861)<be>[___alloc_bootmem_node](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L747)<br>[___alloc_bootmem_node_nopanic](http://lxr.free-electrons.com/source/mm/bootmem.c?v=4.7#L707) |




    