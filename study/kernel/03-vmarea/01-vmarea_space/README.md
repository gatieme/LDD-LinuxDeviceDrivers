进程虚拟地址空间
=======

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6225543) |



#1	虚拟地址空间概述
-------

用户层进程的虚拟地址空间是Linux的一个重要抽象 : 它向每个运行进程提供了同样的系统视图, 这使得多个进程可以同时运行, 而不会干扰到其他进程内存中的内容. 此外, 它容许使用各种高级的程序设计技术，如内存映射

从今天开始, 我将讨论内核是如何实现这些概念的. 这同样需要考察可用物理内存中的页帧与所有的进程虚拟地址空间中的页之间的关联 : **逆向映射(reverse
mapping)* ***技术有助于从虚拟内存页跟踪到对应的物理内存页, 而**缺页处理(page fault handling)**则允许从块设备按需读取数据填充虚拟地址空间


*	每个应用程序都有自身的地址空间，与所有其他应用程序分隔开

*	通常在巨大的线性地址空间中，只有很少的段可用于各个用户空间进程，这些段彼此有一定的距离。内核需要一些数据结构，来有效地管理这些（随机）分布的段。

*	地址空间只有极小的一部分与物理内存页直接关联。不经常使用的部分，则仅当必要时与页帧关联.

*	内核信任自身，但无法信任用户进程。因此，各个操作用户地址空间的操作都伴随有各种检查，以确保程序的权限不会超出应有的限制，进而危及系统的稳定性和安全性.

*	fork-exec模型在UNIX操作系统下用于产生新进程. 如果实现得较为粗劣, 该模型的功能并不强大。因此内核必须借助于一些技巧，来尽可能高效地管理用户地址空间

#2	进程虚拟地址空间
-------

##2.1	进程虚拟地址空间
-------

各个进程的虚拟地址空间起始于地址0, 延伸到TASK_SIZE - 1, 其上是内核地址空间。 在IA-32系统上地址空间的范围可达$2^{32} = 4GB$, 总的地址空间通常按3:1比例划分,我们在下文中将关注该划分. 内核分配了1GB, 而各个用户空间进程可用的部分为3GB. 其他的划分比例也是可能的, 但正
如前文的讨论, 只能在非常特定的配置和某些工作负荷下才有用.

与系统完整性相关的非常重要的一方面是, 用户程序只能访问整个地址空间的下半部分,不能访问内核部分. 如果没有预先达成"协议", 用户进程也不可能操作另一个进程的地址空间,因为后者的地址空间对前者不可见.

无论当前哪个用户进程处于活动状态, 虚拟地址空间内核部分的内容总是同样的. 取决于具体的硬件, 这可能是通过操作各用户进程的页表, 使得虚拟地址空间的上半部看上去总是相同的. 也可能是指示处理器为内核提供一个独立的地址空间, 映射在各个用户地址空间之上. 读者可以回想一下图1-3, 其中给出了相关的图示.

虚拟地址空间由许多不同长度的段组成, 用于不同的目的, 必须分别处理.

例如在大多数情况下, 不允许修改text段, 但必须可以执行其内容. 另一方面,必须可以修改映射到地址空间中的文本文件
内容,而不能允许执行其内容. 因为这没有意义,文件的内容只是数据,并非机器代码.

##2.2	进程地址空间的布局
-------

虚拟地址空间中包含了若干区域. 其分布方式是特定于体系结构的,但所有方法都有下列共同成分.

*	当前运行代码的二进制代码. 该代码通常称之为text,所处的虚拟内存区域称之为代码段(text section).

*	可执行文件的已初始化全局变量的内存映射, 称为数据段(data section).

*	包括未初始化全局变量(也就是bss段的零页)的内存映射， 页面中的信息全部为0值, 所以可用于映射bss段等目的.

*	用于保存局部变量和实现函数/过程调用栈(不要和进程内核栈混淆, 进程的内核栈独立存在并由内核维护)的零页内存映射


*	程序使用的动态库的代码, 诸如C库或动态连接程序等共享库的代码段, 数据段和bss段.

*	存储动态产生的数据的堆

*	环境变量和命令行参数的段.

*	将文件内容映射到虚拟地址空间中的内存映射


进程的虚拟地址空间中的任何有效地址都只能位于唯一的区域, 这些内存区域不能相互覆盖. 可以看到, 在执行的进程中, 每个不同的内存片段都对应一个独立的内存区域 : 栈, 对象代码, 全局变量, 被映射的文件等.


一个进程的虚拟地址空间主要由两个数据结来描述

*	一个是最高层次的 ： `mm_struct`

*	一个是较高层次的 ： `vm_area_structs`

最高层次的`mm_struct`结构描述了一个进程的整个虚拟地址空间。较高层次的结构`vm_area_truct`描述了虚拟地址空间的一个区间(简称虚拟区).

每个进程只有一个mm_struct结构, 在每个进程的`task_struct`结构中, 有一个指向该进程的结构, 参见task_struct的定义[include/linux/sched.h?v=4.7, line 1457](http://lxr.free-electrons.com/source/include/linux/sched.h?v=4.7#L1457)


```cpp
//  http://lxr.free-electrons.com/source/include/linux/sched.h?v=4.7#L1457
struct task_struct
{
	struct mm_struct *mm, *active_mm;
};
```
可以说, `mm_struct`结构是对整个用户空间的描述. `mm_strcut`用来描述一个进程的虚拟地址空间.


##2.3	内存描述符`mm_struct`
-------

内核使用内存描述符结构体`struct mm_struct`表示进程的地址空间, 该结构体包含了和进程地址空间相关的全部信息


系统中的各个进程都具有一个`struct mm_struct`的实例,可以通过`task_struct`访问. 这个实例保存了进程的内存管理信息, 定义在[`include/linux/mm_types.h?v=4.7, line 395`](http://lxr.free-electrons.com/source/include/linux/mm_types.h?v=4.7#L395)



**可执行代码**占用的虚拟地址空间区域, 其开始和结束分别通过 `start_code`和`end_code`标记.


类似地, `start_data`和`end_data`标记了包含**已初始化数据的区域**. 请注意, 在`ELF`二进制文件映射到地址空间中之后,这些区域的长度不再改变.

**堆**的起始地址保存在`start_brk`, `brk`表示堆区域当前的结束地址. 尽管堆的起始地址在进程生命周期中是不变的, 但堆的长度会发生变化,因而`brk`的值也会变.

**参数列表**和**环境变量**的位置分别由`arg_start`和 `arg_end`、`env_start`和`env_end`描述. 两个区域
都位于栈中最高的区域.

[`mmap_base`](http://lxr.free-electrons.com/source/include/linux/mm_types.h?v=4.7#L404)表示虚拟地址空间中用于内存映射的起始地址, 可调用`get_unmapped_area`在`mmap`
区域中为新映射找到适当的位置.

`task_size`, 顾名思义, 存储了对应进程的地址空间长度. 对本机应用程序来说, 该值通常是`TASK_SIZE`. 但64位体系结构与前辈处理器通常是二进制兼容的. 如果在64位计算机上执行32位二进制代码, 则`task_size`描述了该二进制代码实际可见的地址空间长度.

各个体系结构可以通过几个配置选项影响虚拟地址空间的布局。

*	如果体系结构想要在不同`mmap`区域布局之间作出选择, 则需要设置`HAVE_ARCH_PICK_MMAP_LAYOUT`, 并提供`arch_pick_mmap_layout`函数.

*	在创建新的内存映射时, 除非用户指定了具体的地址, 否则内核需要找到一个适当的位置. 如果体系结构自身想要选择合适的位置,则必须设置预处理器符号`HAVE_ARCH_UNMAPPED_AREA`, 并相应地定义`arch_get_unmapped_area`函数。

*	在寻找新的内存映射低端内存位置时, 通常从较低的内存位置开始, 逐渐向较高的内存地址搜索. 内核提供了默认的函数`arch_get_unmapped_area_topdown`用于搜索, 但如果某个体系结构想要提供专门的实现, 则需要设置预处理器符号`HAVE_ARCH_GET_UNMAPPED_AREA`.

*	通常, 栈自顶向下增长. 具有不同处理方式的体系结构需要设置配置选项`CONFIG_STACK_GROWSUP`

最后, 我们需要考虑进程标志`PF_RANDOMIZE`. 如果设置了该标志, 则内核不会为栈和内存映射的起点选择固定位置,而是在每次新进程启动时随机改变这些值的设置. 这引入了一些复杂性, 例如, 使得攻击因缓冲区溢出导致的安全漏洞更加困难. 如果攻击者无法依靠固定地址找到栈,那么想要构
建恶意代码, 通过缓冲器溢出获得栈内存区域的访问权, 而后恶意操纵栈的内容,将会困难得多.


下图说明了前述的各个部分在大多数体系结构的虚拟地址空间中的分布情况.

`text`段如何映射到虚拟地址空间中由ELF标准确定(有关该二进制格式的更多信息,请参见), 每个体系结构都指定了一个特定的起始地址 : IA-32系统起始于0x08048000, 在text段的起始地址与最低的可用地址之间有大约128 MiB的间距,用于捕获NULL指针. 其他体系结构也有类似的缺口 : `UltraSparc`计算机使用0x100000000作为text段的起始点, 而AMD64使用0x0000000000400000. 堆紧接着text段开始, 向上增长. 栈起始于STACK_TOP, 如果设置了 `PF_RANDOMIZE`, 则起始点会减少一个小的随机量. 每个体系结构都必须定义`STACK_TOP`, 大多数都设置为 `TASK_SIZE`, 即用户地址空间中最高的可用地址. 进程
的参数列表和环境变量都是栈的初始数据.

用于内存映射的区域起始于`mm_struct->mmap_base`, 通常设置为`TASK_UNMAPPED_BASE`, 每个体系结构都需要定义. 几乎所有的情况下, 其值都是`TASK_SIZE/3`. 要注意,如果使用内核的默认配置, 则`mmap`区域的起始点不是随机的.


![进程的线性地址空间的组成]()



如果计算机提供了巨大的虚拟地址空间, 那么使用上述的地址空间布局会工作得非常好. 但在32位计算机上可能会出现问题. 考虑IA-32的情况 : 虚拟地址空间从0到0xC0000000 , 每个用户进程有3GB可用. `TASK_UNMAPPED_BASE`起始于0x4000000, 即1GB处. 糟糕的是, 这意味着堆只有1GB
空间可供使用, 继续增长则会进入到`mmap`区域, 这显然不是我们想要的.

问题在于, 内存映射区域位于虚拟地址空间的中间. 这也是在内核版本2.6.7开发期间为IA-32计算机引入一个新的虚拟地址空间布局的原因(经典布局仍然可以使用).

![mmap区域自顶向下扩展时,IA-32计算机上虚拟地址空间的布局]()


其想法在于使用固定值限制栈的最大长度. 由于栈是有界的, 因此安置内存映射的区域可以在栈末端的下方立即开始. 与经典方法相反, 该区域现在是自顶向下扩展. 由于堆仍然位于虚拟地址空间中较低的区域并向上增长, 因此`mmap`区域和堆可以相对扩展, 直至耗尽虚拟地址空间中剩余的区域.

为确保栈与`mmap`区域不发生冲突,两者之间设置了一个安全隙.


另外, 它还包括下列成员,用于管理用户进程在虚拟地址空间中的所有内存区域.

```cpp
<mm_types.h>
struct mm_struct {
	struct vm_area_struct * mmap;	/* 虚拟内存区域列表 */
	struct rb_root mm_rb;			/* 虚拟内存区域的红黑树  */
	/*  ......  */
};
```

每个区域都通过一个`vm_area_struct`实例描述, 进程的各区域按两种方法排序.

1.	在一个单链表上(开始于`mm_struct->mmap`

2.	在一个红黑树中,根结点位于`mm_struct->mm_rb`

用户虚拟地址空间中的每个区域由开始和结束地址描述. 现存的区域按起始地址以递增次序被归入链表中. 扫描链表找到与特定地址关联的区域, 在有大量区域时是非常低效的操作(数据密集型的应用程序就是这样). 因此`vm_area_struct`的各个实例还通过红黑树(由mm_struct->mm_rb来标识)管理, 可以显著加快扫描速度.

增加新区域时, 内核首先搜索红黑树, 找到刚好在新区域之前的区域. 因此, 内核可以向树和线性链表添加新的区域, 而无需扫描链表.



##2.4	虚拟内存区域`vm_area_struct`
-------

`vm_area_struct结构体描述了指定地址空间内连续区间上的一个独立内存范围. 内核将每个内存区域作为一个单独的内存对象管理, 每个内存区域都拥有一致的属性, 比如访问权限等. 另外相应的操作也都一致. 按照这样的方式, 每一个VMA就可以代表不同类型的内存区域(比如内存映射文件或者进程用户空间栈). 这种管理方式类似于使用CFS层面向对象的方法.

每个区域表示为`vm_area_struct`的一个实例, 其定义在[`include/linux/mm_types.h?v=4.7, line 299`](http://lxr.free-electrons.com/source/include/linux/mm_types.h?v=4.7#L299)


```cpp
struct vm_area_struct {
    /* The first cache line has the info for VMA tree walking. */

    unsigned long vm_start;     /* Our start address within vm_mm. */
    unsigned long vm_end;       /* The first byte after our end address
                       within vm_mm. */

    /* linked list of VM areas per task, sorted by address */
    struct vm_area_struct *vm_next, *vm_prev;

    struct rb_node vm_rb;


    struct mm_struct *vm_mm;    /* The address space we belong to. */
```


每个内存描述符都对应于进程地址空间上的唯一区间. `vm_start`指向区间的首地址(最低地址). `vm_end`指向了区域的尾地址(最高地址)之后的第一个字节. 也就是说, `vm_start`是内存区间的开始地址(它本身在区间内), 而`vm_end`是内存区间的结束地址(它本身在区间外). 因此, `vm_end - vm_start`的大小便是区间的长度. 即内存区域就在[vm_start, vm_end]之中. 注意, 在同一个地址空间内的不同内存区域不能重叠.

所有的内存域组织在链表和红黑树中, 因此`vm_next`和`vm_prev`就指向了该虚拟内存区域在链表中的后继和前驱. `vm_rb`则作为内置的红黑树节点.


`vm_mm`域指向和VMA相关的`mm_struct`结构体. 注意, 每个VMA对其相关`mm_struct`结构体都是唯一的.

*	即使两个独立的进程将同一个文件映射到各自的地址空间, 他们非别都会有一个vm_area_struct结构体来标志自己的内存区域.

*	反过来, 如果两个线程共享一个地址空间, 那么他们也会同时共享其中所有的vm_area_struct结构体.



##2.5	建立布局
-------

在使用`load_elf_binary`装载一个ELF二进制文件时,将创建进程的地址空间, 该函数定义在[fs/binfmt_elf.c?v=4.7, line 666](http://lxr.free-electrons.com/source/fs/binfmt_elf.c?v=4.7#L666)


 而`exec`系统调用刚好使用了该函数. 加载`ELF`文件涉及大量纷繁复杂的技术细节, 之前我们讲解进程调度的时候曾经专门讲解过这个函数, 因此我们现在只给出的代码流程图来主要关注建立虚拟内存区域所需的各个步骤.



![`load_elf_binary`的代码流程图]()


如果全局变量`randomize_va_space`设置为1, 则启用地址空间随机化机制. 通常情况下都是启用的, 但在`Transmeta CPU`上会停用,因为该设置会降低此类计算机的速度. 此外,用户可以通过`/proc/sys/kernel/randomize_va_space`停用该特性

![cat](./images/cat_proc_sys_kernel_randomize_va_space.png)



选择布局的工作由`arch_pick_mmap_layout`完成. 如果对应的体系结构没有提供一个具体的函数, 则使用内核的默认例程, 按如图4-1所示建立地址空间. 但我们更感兴趣的是, IA-32如何在经典布局和新的布局之间选择. 该函数定义在[`/arch/对应架构/mm/mmap.c`](http://lxr.free-electrons.com/ident?v=4.7;i=arch_pick_mmap_layout)


| 设置进程的虚拟内存布局 | x86 | arm | arm64 |
|:---------------------:|:---:|:---:|:-----:|
| arch_pick_mmap_layout | [arch/x86/mm/mmap.c?v=4.7, line 100](http://lxr.free-electrons.com/source/arch/x86/mm/mmap.c?v=4.7#L100) | [arch/arm/mm/mmap.c?v=4.7, line 181](http://lxr.free-electrons.com/source/arch/arm/mm/mmap.c?v=4.7#L181) | [arch/arm64/mm/mmap.c?v=4.7, line 79](http://lxr.free-electrons.com/source/arch/arm64/mm/mmap.c?v=4.7#L79) |


参见arm下arch_pick_mmap_layout函数的实现, 定义在[arch/arm/mm/mmap.c?v=4.7, line 181](http://lxr.free-electrons.com/source/arch/arm/mm/mmap.c?v=4.7#L181)

```cpp
void arch_pick_mmap_layout(struct mm_struct *mm)
{
    unsigned long random_factor = 0UL;

    if (current->flags & PF_RANDOMIZE)
        random_factor = arch_mmap_rnd();

    if (mmap_is_legacy()) {
        mm->mmap_base = TASK_UNMAPPED_BASE + random_factor;
        mm->get_unmapped_area = arch_get_unmapped_area;
    } else {
        mm->mmap_base = mmap_base(random_factor);
        mm->get_unmapped_area = arch_get_unmapped_area_topdown;
    }
}
```


如果用户通过`/proc/sys/vm/legacy_va_layout`给出明确的指示, 或者要执行为不同的UNIX变体编译、需要旧的布局的二进制文件, 或者栈可以无限增长(最重要的一点),则系统会选择旧的布局. 这使得很难确定栈的下界, 亦即`mmap`区域的上界.

在经典的配置下, `mmap`区域的起始点是`TASK_UNMAPPED_BASE`, 其值为0x4000000, 而标准函数`arch_get_unmapped_area`(其名称虽然带有`arch` , 但该函数不一定是特定于体系结构的, 内核也提供了一个标准实现)用于自下而上地创建新的映射.

在使用新布局时, 内存映射自顶向下增长.

标准函数`arch_get_unmapped_area_topdown`(我不会详细描述)负责该工作.

|  函数or变量 | x86 | arm | arm64 |
|:---------------:|:---:|:---:|:-----:|
| MIN_GAP/MAX_GAP | [arch/x86/mm/mmap.c?v=4.7, line 54](http://lxr.free-electrons.com/source/arch/x86/mm/mmap.c?v=4.7#L54) | [arch/arm/mm/mmap.c?v=4.7, line 19](http://lxr.free-electrons.com/source/arch/arm/mm/mmap.c?v=4.7#L19) | [arch/arm64/mm/mmap.c?v=4.7, line 36](http://lxr.free-electrons.com/source/arch/arm64/mm/mmap.c?v=4.7#L36) |
| mmap_is_legacy | [arch/x86/mm/mmap.c?v=4.7, line 57](http://lxr.free-electrons.com/source/arch/x86/mm/mmap.c?v=4.7#L57) | [arch/arm/mm/mmap.c?v=4.7, line 22](http://lxr.free-electrons.com/source/arch/arm/mm/mmap.c?v=4.7#L22) | [arch/arm64/mm/mmap.c?v=4.7, line 39](http://lxr.free-electrons.com/source/arch/arm64/mm/mmap.c?v=4.7#L39) |
| arch_mmap_rnd | [arch/x86/mm/mmap.c?v=4.7, line 68](http://lxr.free-electrons.com/source/arch/x86/mm/mmap.c?v=4.7#L68) | [arch/arm/mm/mmap.c?v=4.7, line 172](http://lxr.free-electrons.com/source/arch/arm/mm/mmap.c?v=4.7#L172) | [arch/arm64/mm/mmap.c?v=4.7, line 50](http://lxr.free-electrons.com/source/arch/arm64/mm/mmap.c?v=4.7#L50) |
|  mmap_base | [arch/x86/mm/mmap.c?v=4.7, line 84](http://lxr.free-electrons.com/source/arch/x86/mm/mmap.c?v=4.7#L84) | [arch/arm/mm/mmap.c?v=4.7, line 33](http://lxr.free-electrons.com/source/arch/arm/mm/mmap.c?v=4.7#L33) | [arch/arm64/mm/mmap.c?v=4.7, line 63](http://lxr.free-electrons.com/source/arch/arm64/mm/mmap.c?v=4.7#L63) |


```cpp
//  http://lxr.free-electrons.com/source/arch/x86/mm/mmap.c?v=4.7#L54
#define MIN_GAP (128*1024*1024)
#define MAX_GAP (TASK_SIZE/6*5)

//  http://lxr.free-electrons.com/source/arch/arm/mm/mmap.c?v=4.7#L19
/* gap between mmap and stack */
#define MIN_GAP (128*1024*1024UL)
#define MAX_GAP ((TASK_SIZE)/6*5)

//  http://lxr.free-electrons.com/source/arch/arm64/mm/mmap.c?v=4.7#L36
/*
 * Leave enough space between the mmap area and the stack to honour ulimit in
 * the face of randomisation.
 */
#define MIN_GAP (SZ_128M + ((STACK_RND_MASK << PAGE_SHIFT) + 1))
#define MAX_GAP (STACK_TOP/6*5)
```


更有趣的问题是如何选择内存映射的基地址, 该工作由`mmap_base`来完成, arm架构下该函数定义在[`arch/arm/mm/mmap.c?v=4.7, line 19`](http://lxr.free-electrons.com/source/arch/arm/mm/mmap.c?v=4.7#L19)

```cpp
static unsigned long mmap_base(unsigned long rnd)
{
    unsigned long gap = rlimit(RLIMIT_STACK);

    if (gap < MIN_GAP)
        gap = MIN_GAP;
    else if (gap > MAX_GAP)
        gap = MAX_GAP;

    return PAGE_ALIGN(TASK_SIZE - gap - rnd);
}
```


可以根据栈的最大长度, 来计算栈最低的可能位置, 用作`mmap`区域的起始点. 但内核会确保栈至少跨越128MB的空间. 另外, 如果指定的栈界限非常巨大, 那么内核会保证至少有一小部分地址空间不被栈占据.

如果要求使用地址空间随机化机制, 上述位置会减去一个随机的偏移量,最大为1MB.

另外, 内核会确保该区域对齐到页帧, 这是体系结构的要求.

初看起来, 读者可能认为64位体系结构的情况会好一点, 因为不需要在不同的地址空间布局中进行选择. 虚拟地址空间是如此巨大, 以至于堆和`mmap`区域的碰撞几乎不可能.



但从AMD64体系结构的`arch_pick_mmap_layout`定义来看,此中会出现另一个复杂情况:

```cpp
arch_pick_mmap_layout
```


如果启用对32位应用程序的二进制仿真,任何以兼容模式运行的进程都应该看到与原始计算机上相同的地址空间。因此, `ia32_pick_mmap_layout`用于为32位应用程序布置地址空间。该函数实际上是IA-32系统上`arch_pick_mmap_layout`的一个相同副本,前文已经讨论过.


AMD64系统上对虚拟地址空间总是使用经典布局,因此无需区分各种选项。如果设置了`PF_RANDOMIZE`标志,则进行地址空间随机化,变动原本固定的`mmap_base`.

我们回到`load_elf_binary`. 该函数最后需要在适当的位置创建栈:

```cpp
load_elf_binary
```

标准函数setup_arg_pages即用于该目的. 因为该函数只是技术性的,我不会详细讨论. 该函数需要栈顶的位置作为参数. 栈顶由特定于体系结构的常数STACK_TOP给出, 而后调用randomize_stack_top, 确保在启用地址空间随机化的情况下,对该地址进行随机偏移.
