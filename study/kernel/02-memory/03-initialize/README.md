初始化内存管理
=======





| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6393814) |









在内存管理的上下文中, 初始化(initialization)可以有多种含义. 在许多CPU上, 必须显式设置适用于Linux内核的内存模型. 例如在x86_32上需要切换到保护模式, 然后奇偶内核才能检测到可用内存和寄存器.



在初始化过程中, 还必须建立内存管理的数据结构, 以及很多事务. 因为内核在内存管理完全初始化之前就需要使用内存. 在系统启动过程期间, 使用了额外的简化悉尼股市的内存管理模块, 然后在初始化完成后, 将旧的模块丢弃掉.



对相关数据结构的初始化是从全局启动函数start_kernel中开始的, 该函数在加载内核并激活各个子系统之后执行. 由于内存管理是内核一个非常重要的部分, 因此在特定体系结构的设置步骤中检测并确定系统中内存的分配情况后, 会立即执行内存管理的初始化.


#1	物理内存初始化
-------

从硬件角度来看内存, 随机存储器 (`Random Access Memory`, `RAM`) 是与 `CPU` 直连交换数据的内部存储器. 现在大部分计算机都使用 `DDR`(`Dual Data Rate SDRAM`) 的存储设备, `DDR` 包括 `DDR3L`, `DDR4L`, `LPDDR3/4` 等. `DDR` 的初始化一般是在 `BIOS` 或者 `bootloader` 中, `BIOS` 或 `bootloader` 把 `DDR` 的大小传递给 `Linux` 内核. 因此从 `Linux` 内核的角度来看 `DDR` 其实就是一段物理内存空间.


##1.1	内存管理概述
-------

内存管理是一个很复杂的系统, 涉及的内容很多. 如果用分层来描述, 内存管理可以分成 `3` 个层次, 分别时用户空间层, 内核空间层和硬件层.


| 层次 | 描述 |
|:---:|:----:|
| **用户空间层** |可以理解为 `Linux` 内核内存管理为用户空间暴露的系统调用接口. 例如 `brk( )`, `mmap( )` 等系统调用. 通常 `libc` 库会将系统调用封装成大家常见的 `C` 库函数, 比如 `malloc( )`, `mmap( )` 等. |
| **内核空间层** | 包含的模块相当丰富, 用户空间和内核空间的接口时系统调用, 因此内核空间层首先需要处理这些内存管理相关的系统调用, 例如 `sys_brk`, `sys_mmap`, `sys_madvise` 等. 接下来就包括 `VMA` 管理, 缺页中断管理, 匿名页面, `page cache`, 页面回收, 反向映射, `slab` 分配器, 页面管理等模块. |
| **硬件层** | 包含处理器的 `MMU`, `TLB` 和 `cache` 部件, 以及板载的物理内存, 例如 `LPDDR` 或者 `DDR` |


##1.2	内存大小
-------


###1.2.1	`DTS` 上报
-------


在 `ARM Linux` 中, 各种设备的相关属性描述都采用 `DTS` 方式来呈现. `DTS` 是 `device tree source` 的简称. 最早是由 `PowerPC` 等其他体系结构使用的 `FDT(Flattened Device Tree)` 转变过来的. `ARM Linux` 社区自 `2011` 年被 `Linus Torvalds` 公开批评之后, 开始全面支持 `DTS`, 并且删除了大量的冗余代码.


在 `ARM Vexpress` 平台中, 内存的定义在 `vexpress-v2p-ca9.dts` 文件中, 该 `DTS` 文件定义了内存的起始地址为 `0x60000000`, 大小为 `0x40000000`, 即 `1GB` 大小内存空间.


```cpp
//  http://elixir.free-electrons.com/linux/v4.13.11/source/arch/arm/boot/dts/vexpress-v2p-ca9.dts#L65
memory@60000000 {
	device_type = "memory";
	reg = <0x60000000 0x40000000>;
};
```

同样 `ARM64` 平台类似, 起始地址为 `0x80000000`, 大小 `0x80000000(2 GB)`.

```cpp
// http://elixir.free-electrons.com/linux/v4.13.11/source/arch/arm64/boot/dts/arm/vexpress-v2f-1xv7-ca53x2.dts#L61
memory@80000000 {
	device_type = "memory";
	reg = <0 0x80000000 0 0x80000000>; /* 2GB @ 2GB */
};
```

内存在启动的过程中, 需要解析这些 `DTS` 文件, 实际代码在 [`early_init_dt_scan_memory( )`](http://elixir.free-electrons.com/linux/v4.13.11/source/drivers/of/fdt.c#L1058) 函数中, 代码调用关系如下 :


[`start_kernel( )`](http://elixir.free-electrons.com/linux/v4.13.11/source/init/main.c#L536)
	-=>	[`setup_arch( )`](http://elixir.free-electrons.com/linux/v4.13.11/source/arch/arm64/kernel/setup.c#L260)
	-=>	[`setup_machine_fdt( )`](http://elixir.free-electrons.com/linux/v4.13.11/source/arch/arm64/kernel/setup.c#L185)
    	-=> [`early_init_dt_scan( )`](http://elixir.free-electrons.com/linux/v4.13.11/source/drivers/of/fdt.c#L1288)
    	-=>	[`early_init_dt_scan_nodes( )`](http://elixir.free-electrons.com/linux/v4.13.11/source/drivers/of/fdt.c#L1268)
        -=> [`of_scan_flat_dt( )`](http://elixir.free-electrons.com/linux/v4.13.11/source/drivers/of/fdt.c#L728)


```cpp
//  http://elixir.free-electrons.com/linux/v4.13.11/source/drivers/of/fdt.c#L1268
void __init early_init_dt_scan_nodes(void)
{
	/* Retrieve various information from the /chosen node */
	of_scan_flat_dt(early_init_dt_scan_chosen, boot_command_line);

	/* Initialize {size,address}-cells info */
	of_scan_flat_dt(early_init_dt_scan_root, NULL);

	/* Setup memory, calling early_init_dt_add_memory_arch */
	of_scan_flat_dt(early_init_dt_scan_memory, NULL);
}
```

最终 `early_init_dt_scan_nodes( )` 调用了 `early_init_dt_scan_memory` 函数读取 `DTS` 的信息并初始化内存信息.

```cpp
//  http://elixir.free-electrons.com/linux/v4.13.11/source/drivers/of/fdt.c#L1055
/**
 * early_init_dt_scan_memory - Look for and parse memory nodes
 */
int __init early_init_dt_scan_memory(unsigned long node, const char *uname,
				     int depth, void *data)
{
	const char *type = of_get_flat_dt_prop(node, "device_type", NULL);
	const __be32 *reg, *endp;
	int l;
	bool hotpluggable;

	/* We are scanning "memory" nodes only */
	if (type == NULL) {
		/*
		 * The longtrail doesn't have a device_type on the
		 * /memory node, so look for the node called /memory@0.
		 */
		if (!IS_ENABLED(CONFIG_PPC32) || depth != 1 || strcmp(uname, "memory@0") != 0)
			return 0;
	} else if (strcmp(type, "memory") != 0)
		return 0;

	reg = of_get_flat_dt_prop(node, "linux,usable-memory", &l);
	if (reg == NULL)
		reg = of_get_flat_dt_prop(node, "reg", &l);
	if (reg == NULL)
		return 0;

	endp = reg + (l / sizeof(__be32));
	hotpluggable = of_get_flat_dt_prop(node, "hotpluggable", NULL);

	pr_debug("memory scan node %s, reg size %d,\n", uname, l);

	while ((endp - reg) >= (dt_root_addr_cells + dt_root_size_cells)) {
		u64 base, size;

		base = dt_mem_next_cell(dt_root_addr_cells, &reg);
		size = dt_mem_next_cell(dt_root_size_cells, &reg);

		if (size == 0)
			continue;
		pr_debug(" - %llx ,  %llx\n", (unsigned long long)base,
		    (unsigned long long)size);

		early_init_dt_add_memory_arch(base, size);

		if (!hotpluggable)
			continue;

		if (early_init_dt_mark_hotplug_memory_arch(base, size))
			pr_warn("failed to mark hotplug range 0x%llx - 0x%llx\n",
				base, base + size);
	}

	return 0;
}
```

`early_init_dt_scan_memory` 函数解析 `memory` 描述的信息从而得到内存的 `base_address` 和 `size` 信息, 最后内存块信息通过 [`early_init_dt_add_memory_arch( )`](http://elixir.free-electrons.com/linux/v4.13.11/source/drivers/of/fdt.c#L1163)->[`memblock_add( )`](http://elixir.free-electrons.com/linux/v4.13.11/source/mm/memblock.c#L596)


###1.2.2	`ACPI` 上报
-------

待补充


##1.3	物理内存映射
-------


在内核使用使用内存之前, 需要初始化内核的页表, 初始化页表主要在 [`map_lowmem( )`](http://elixir.free-electrons.com/linux/v4.13.11/source/arch/arm/mm/mmu.c#L1429) 函数中.

在映射页表之前, 需要把页表的页表项清 `0`. 这部分工作主要在 [`prepare_page_table( )`](http://elixir.free-electrons.com/linux/v4.13.11/source/arch/arm/mm/mmu.c#L1246) 函数中实现.


```cpp
start_kernel( )
	-=>	setup_arch( )
		-=>	paging_init( )
		-=>	prepare_page_table( )
static inline void prepare_page_table(void)
{
	unsigned long addr;
	phys_addr_t end;

	/*
	 * Clear out all the mappings below the kernel image.
	 */
	for (addr = 0; addr < MODULES_VADDR; addr += PMD_SIZE)
		pmd_clear(pmd_off_k(addr));

#ifdef CONFIG_XIP_KERNEL
	/* The XIP kernel is mapped in the module area -- skip over it */
	addr = ((unsigned long)_exiprom + PMD_SIZE - 1) & PMD_MASK;
#endif
	for ( ; addr < PAGE_OFFSET; addr += PMD_SIZE)
		pmd_clear(pmd_off_k(addr));

	/*
	 * Find the end of the first block of lowmem.
	 */
	end = memblock.memory.regions[0].base + memblock.memory.regions[0].size;
	if (end >= arm_lowmem_limit)
		end = arm_lowmem_limit;

	/*
	 * Clear out all the kernel space mappings, except for the first
	 * memory bank, up to the vmalloc region.
	 */
	for (addr = __phys_to_virt(end);
	     addr < VMALLOC_START; addr += PMD_SIZE)
		pmd_clear(pmd_off_k(addr));
}
```

这里对如下 `3` 个地址调用 `pmd_clear( )` 函数来清除一级页表项的内容.

*	0 ~ `MODULES_VADDR`

*	`MODULES_VADDR` ~ `PMD_SIZE`

*	`arm_lowmem_limit` ~ `VMALLOC_START`

真正创建页表的函数时在 [`map_lowmem( )`](http://elixir.free-electrons.com/linux/v4.13.11/source/arch/arm/mm/mmu.c#L1429) 函数中. 定义在 [`arch/arm/mm/mmu.c, line 1429`](http://elixir.free-electrons.com/linux/v4.13.11/source/arch/arm/mm/mmu.c#L1429). 该函数会在内存开始的地方覆盖到 [`arm_lowmem_limit`](http://elixir.free-electrons.com/linux/v4.13.11/source/arch/arm/mm/mmu.c#L1160) 处.


```cpp
//  http://elixir.free-electrons.com/linux/v4.13.11/source/arch/arm/mm/mmu.c#L1429
static void __init map_lowmem(void)
{
	struct memblock_region *reg;
	phys_addr_t kernel_x_start = round_down(__pa(KERNEL_START), SECTION_SIZE);
	phys_addr_t kernel_x_end = round_up(__pa(__init_end), SECTION_SIZE);

	/* Map all the lowmem memory banks. */
	for_each_memblock(memory, reg) {
		phys_addr_t start = reg->base;
		phys_addr_t end = start + reg->size;
		struct map_desc map;

		if (memblock_is_nomap(reg))
			continue;

		if (end > arm_lowmem_limit)
			end = arm_lowmem_limit;
		if (start >= end)
			break;

		if (end < kernel_x_start) {
			map.pfn = __phys_to_pfn(start);
			map.virtual = __phys_to_virt(start);
			map.length = end - start;
			map.type = MT_MEMORY_RWX;

			create_mapping(&map);
		} else if (start >= kernel_x_end) {
			map.pfn = __phys_to_pfn(start);
			map.virtual = __phys_to_virt(start);
			map.length = end - start;
			map.type = MT_MEMORY_RW;

			create_mapping(&map);
		} else {
			/* This better cover the entire kernel */
			if (start < kernel_x_start) {
				map.pfn = __phys_to_pfn(start);
				map.virtual = __phys_to_virt(start);
				map.length = kernel_x_start - start;
				map.type = MT_MEMORY_RW;

				create_mapping(&map);
			}

			map.pfn = __phys_to_pfn(kernel_x_start);
			map.virtual = __phys_to_virt(kernel_x_start);
			map.length = kernel_x_end - kernel_x_start;
			map.type = MT_MEMORY_RWX;

			create_mapping(&map);

			if (kernel_x_end < end) {
				map.pfn = __phys_to_pfn(kernel_x_end);
				map.virtual = __phys_to_virt(kernel_x_end);
				map.length = end - kernel_x_end;
				map.type = MT_MEMORY_RW;

				create_mapping(&map);
			}
		}
	}
}
```

这里需要考虑 `kernel` 代码段的问题, `kernel` 的代码段从 `_stext` 开始, 到 `_init_end` 结束. 以 `ARM Vexpress` 平台为例.

*	内存起始地址 `0x60000000`

*	`_stext` : `0x60000000`

*	`_init_end` : `0x60800000`

*	`arm_lowmem_limit` : 0x8f800000

其中, `arm_lowmem_limit` 地址考虑高端内存的情况, 该值的计算是在 `sanity_check_meminfo` 函数中. 在 `ARM Vexpress` 平台中 `arm_lowmem_limit` 等于 [`vmalloc_min`](http://elixir.free-electrons.com/linux/v4.13.11/source/arch/arm/mm/mmu.c#L1131) 其定义如下所示 :

```cpp
//  http://elixir.free-electrons.com/linux/v4.13.11/source/arch/arm/mm/mmu.c#L1131
static void * __initdata vmalloc_min =
	(void *)(VMALLOC_END - (240 << 20) - VMALLOC_OFFSET);
```


`map_lowmem( )` 会对两个内存区域创建映射关系.

*	区域 `1`
	物理地址 : `0x60000000` ~ `0x608000000`
    虚拟地址 : `0xc0000000` ~ `0xc08000000`
    属性    : 可读, 可写并且可执行(`MT_MEMORY_RWX`).

*	区域 `2`
	物理地址 : `0x60800000` ~ `0x8f8000000`
	虚拟地址 : `0xc0800000` ~ `0xcf8000000`
	属性    : 可读, 可写(`MT_MEMORY_RW`)


`MT_MEMORY_RWX` 和 `MT_MEMORY_RW` 的区别在于 `ARM` 页表项有一个 `XN` 比特位. `XN` 比特位置 `1` 表示这段内存区域不允许执行.

内存映射的函数为 `create_mapping( )`,这里创建的映射就是物理内存直接映射, 即线性映射.


##1.4	`zone` 初始化
-------

对页表的初始化完成之后, 内核旧可以对内存进行管理了, 但是内核并不是统一对待这些页面. 而是对内存进行了层次化管理. 参见 [`深入理解 `Linux` 内存管理-之-目录导航--`1` 内存描述`](http://blog.csdn.net/gatieme/article/details/52384965), 或者 [`Linux` 内存描述之概述--`Linux` 内存管理(一)--第 `2.2` 节](http://blog.csdn.net/gatieme/article/details/52384058)


>Linux把物理内存划分为三个层次来管理
>
>| 层次 | 描述 |
|:---:|:----:|
| 存储节点(Node) | CPU被划分为多个节点(node), 内存则被分簇, 每个CPU对应一个本地物理内存, 即一个CPU-node对应一个内存簇bank，即每个内存簇被认为是一个节点 |
| 管理区(Zone) | 每个物理内存节点node被划分为多个内存管理区域, 用于表示不同范围的内存, 内核可以使用不同的映射方式映射物理内存 |
| 页面(Page) | 内存被细分为多个页面帧, 页面是最基本的页面分配的单位　｜
>
>为了支持NUMA模型，也即CPU对不同内存单元的访问时间可能不同，此时系统的物理内存被划分为几个节点(node), 一个node对应一个内存簇bank，即每个内存簇被认为是一个节点
>
>首先, 内存被划分为结点. 每个节点关联到系统中的一个处理器, 内核中表示为pg_data_t的实例. 系统中每个节点被链接到一个以NULL结尾的pgdat_list链表中<而其中的每个节点利用pg_data_tnode_next字段链接到下一节．而对于PC这种UMA结构的机器来说, 只使用了一个成为contig_page_data的静态pg_data_t结构.
>
>接着各个节点又被划分为内存管理区域, 一个管理区域通过struct zone_struct描述, 其被定义为zone_t, 用以表示内存的某个范围, 低端范围的16MB被描述为ZONE_DMA, 某些工业标准体系结构中的(ISA)设备需要用到它, 然后是可直接映射到内核的普通内存域ZONE_NORMAL,最后是超出了内核段的物理地址域ZONE_HIGHMEM, 被称为高端内存.　是系统中预留的可用内存空间, 不能被内核直接映射.
>
>最后页帧(page frame)代表了系统内存的最小单位, 堆内存中的每个页都会创建一个struct page的一个实例. 传统上，把内存视为连续的字节，即内存为字节数组，内存单元的编号(地址)可作为字节数组的索引. 分页管理时，将若干字节视为一页，比如4K byte. 此时，内存变成了连续的页，即内存为页数组，每一页物理内存叫页帧，以页为单位对内存进行编号，该编号可作为页数组的索引，又称为页帧号.
>
>在一个单独的节点内，任一给定CPU访问页面所需的时间都是相同的。然而，对不同的CPU，这个时间可能就不同。对每个CPU而言，内核都试图把耗时节点的访问次数减到最少这就要小心地选择CPU最常引用的内核数据结构的存放位置.


其中 `zone` 结构的定义用 `struct zone` 描述在之前 [`Linux` 内存描述之内存区域 `zone–Linux` 内存管理(三)](http://blog.csdn.net/gatieme/article/details/52384529) 已经详细解释过. 由于这个数据结构经常被访问到, 银子这个数据结构需求以 `L1 Cache` 对齐.

通常情况下, 内核的 `zone` 分为 `ZONE_DMA`, `ZONE_DMA32`, `ZONE_NORMAL` 和 `ZONE_HIGHMEM`. 在 `ARM Vexpress` 平台中, 没有定义 `CONFIG_ZONE_DMA` 和 `CONFIG_ZONE_DMA32`. 所以只有 `ZONE_NORMAL` 和 `ZONE_HIGHMEM` 两种. zone 类型的定义在 [`include/linux/mmzone.h`](http://elixir.free-electrons.com/linux/v4.13.11/source/include/linux/mmzone.h#L293) 文件中.

`zone` 的初始化函数集中在 `bootmem_init( )` 中完成, 所以需要确定每个 `zone` 的范围. 在 `find_limits( )` 函数中会计算出 `min_low_pfn`, `max_low_pfn` 和 `max_pfn` 这 `3` 个值. 其中, `min_low_pfn` 是内存块开始地址的页帧号 (`0x60000`), `max_low_pfn(0x8f800)` 表示 `normal` 区域的结束页帧号, 它由 `arm_lowmem_limit` 这个变量得来, `max_pfn`(`0xa0000`) 是内存块结束地址的页帧号.
如果没有配置

```cpp
Memory: 1026280K/1048576K available (6144K kernel code, 402K rwdata, 1400K rodata, 4096K init, 170K bss, 22296K reserved, 0K cma-reserved)
Virtual kernel memory layout:
    vector  : 0xffff0000 - 0xffff1000   (   4 kB)
    fixmap  : 0xffc00000 - 0xfff00000   (3072 kB)
    vmalloc : 0xc0800000 - 0xff800000   (1008 MB)
    lowmem  : 0x80000000 - 0xc0000000   (1024 MB)
    modules : 0x7f000000 - 0x80000000   (  16 MB)
      .text : 0x80008000 - 0x80700000   (7136 kB)
      .init : 0x80900000 - 0x80d00000   (4096 kB)
      .data : 0x80d00000 - 0x80d64b58   ( 403 kB)
       .bss : 0x80d6bc1c - 0x80d964f4   ( 171 kB)
SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=4, Nodes=1
```


