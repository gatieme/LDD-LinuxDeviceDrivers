Linux进程ELF文件格式
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------------- |:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|
| 2016-06-04 | [Linux-4.5](http://lxr.free-electrons.com/source/?v=4.5) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度-之-进程的描述](http://blog.csdn.net/gatieme/article/category/6225543) |

#对象文件格式
-------


##对象文件
-------

首先，你需要知道的是所谓对象文件(Object files)有三个种类：

-	可重定位的对象文件(Relocatable file)

     适于链接的可重定位文件(relocatable file)，包含二进制代码和数据，能与其他可重定位对象文件在编译时合并创建出一个可执行文件。

    这是由汇编器汇编生成的 .o 文件。后面的链接器(link editor)拿一个或一些 Relocatable object files 作为输入，经链接处理后，生成一个可执行的对象文件 (Executable file) 或者一个可被共享的对象文件(Shared object file)。我们可以使用 ar 工具将众多的 .o Relocatable object files 归档(archive)成 .a 静态库文件。如何产生 Relocatable file，你应该很熟悉了，请参见我们相关的基本概念文章和JulWiki。另外，可以预先告诉大家的是我们的内核可加载模块 .ko 文件也是 Relocatable object file。

-	可执行的对象文件(Executable file)

    适于执行的可执行文件(executable file)，包含可以直接拷贝进行内存执行的二进制代码和数据。用于提供程序的进程映像，加载的内存执行。

	这我们见的多了。文本编辑器vi、调式用的工具gdb、播放mp3歌曲的软件mplayer等等都是Executable object file。你应该已经知道，在我们的 Linux 系统里面，存在两种可执行的东西。除了这里说的 Executable object file，另外一种就是可执行的脚本(如shell脚本)。注意这些脚本不是 Executable object file，它们只是文本文件，但是执行这些脚本所用的解释器就是 Executable object file，比如 bash shell 程序。

-	可被共享的对象文件(Shared object file)

    共享目标文件(shared object file)，一种特殊的可重定位对象文件，能在加载时或运行时，装载进内存进行动态链接。连接器可将它与其它可重定位文件和共享目标文件连接成其它的目标文件，动态连接器又可将它与可执行文件和其它共享目标文件结合起来创建一个进程映像。

	这些就是所谓的动态库文件，也即 .so 文件。如果拿前面的静态库来生成可执行程序，那每个生成的可执行程序中都会有一份库代码的拷贝。如果在磁盘中存储这些可执行程序，那就会占用额外的磁盘空间；另外如果拿它们放到Linux系统上一起运行，也会浪费掉宝贵的物理内存。如果将静态库换成动态库，那么这些问题都不会出现。动态库在发挥作用的过程中，必须经过两个步骤：

1.	链接编辑器(link editor)拿它和其他Relocatable object file以及其他shared object file作为输入，经链接处理后，生存另外的 shared object file 或者 executable file。

2.	在运行时，动态链接器(dynamic linker)拿它和一个Executable file以及另外一些 Shared object file 来一起处理，在Linux系统里面创建一个进程映像。

##文件格式
-------

本质上，对象文件只是保存在磁盘文件中的一串字节，每个系统的文件格式都不尽相同：

*	Bell实验室的第一个Unix系统使用 a.out格式。

*	System V Unix的早期版本使用 Common Object File Format(COFF)。

*	Windows NT使用COFF的变种，叫做 Portable Executable(PE)。

*	现代Unix系统，包括Linux、新版System V、BSD变种、Solaris都使用 Executable and Linkable Format(ELF)。


##ELF对象文件格式
-------

ELF(Executable and Linking Format)是一种对象文件的格式，用于定义不同类型的对象文件(Object files)中都放了什么东西、以及都以什么样的格式去放这些东西。它自最早在 System V 系统上出现后，被 xNIX 世界所广泛接受，作为缺省的二进制文件格式来使用。可以说，ELF是构成众多xNIX系统的基础之一。


ELF代表Executable and Linkable Format。他是一种对可执行文件、目标文件和库使用的文件格式。

他在Linux下成为标准格式已经很长时间, 代替了早年的a.out格式。ELF一个特别的优点在于, 同一文件格式可以用于内核支持的几乎所有体系结构上, 这不仅简化了用户空间工具程序的创建, 也简化了内核自身的程序设计。例如, 在必须为可执行文件生成装载程序例程时。

但是文件格式相同并不意味着不同系统上的程序之间存在二进制兼容性, 例如, FreeBSD和Linux都使用ELF作为二进制格式。尽管二者在文件中组织数据的方式相同。但在系统调用机制以及系统调用的语义方面, 仍然有差别。这也是在没有中间仿真层的情况下, FreeBSD程序不能在linux下运行的原因(反过来同样是如此)。

有一点是可以理解的, 二进制程序不能在不同体系结构交换(例如, 为Alpha CPU编译的Linux二进制程序不能在Sparc Linux上执行), 因为底层的体系结构是完全不同的。但是由于ELF的存在, 对所有体系结构而言, 程序本身的相关信息以及程序的各个部分在二进制文件中编码的方式都是相同的。

Linux不仅将ELF用于用户空间程序和库, 还用于构建模块。内核本身也是ELF格式。


##ELF标准
-------

ELF是一种开放格式, 其规范可以自由获取。在ELF格式出来之后，TISC(Tool Interface Standard Committee)委员会定义了一套ELF标准。你可以从这里(http://refspecs.freestandards.org/elf/)找到[详细的标准文档](http://refspecs.freestandards.org/elf)

TISC委员会前后出了两个版本，v1.1和v1.2。两个版本内容上差不多，但就可读性上来讲，我还是推荐你读 v1.2的。因为在v1.2版本中，TISC重新组织原本在v1.1版本中的内容，将它们分成为三个部分(books)：
a) Book I
介绍了通用的适用于所有32位架构处理器的ELF相关内容
b) Book II
介绍了处理器特定的ELF相关内容，这里是以Intel x86 架构处理器作为例子介绍
c) Book III
介绍了操作系统特定的ELF相关内容，这里是以运行在x86上面的 UNIX System V.4 作为例子介绍

值得一说的是，虽然TISC是以x86为例子介绍ELF规范的，但是如果你是想知道非x86下面的ELF实现情况，那也可以在http://refspecs.freestandards.org/elf/中找到[特定处理器相关的Supplment文档](http://refspecs.freestandards.org/elf)。比方ARM相关的，或者MIPS相关的等等。另外，相比较UNIX系统的另外一个分支BSD Unix，Linux系统更靠近 System V 系统。所以关于操作系统特定的ELF内容，你可以直接参考v1.2标准中的内容。




#本文所使用的测试程序结构
-------

##add.c
-------

```c
#include <stdio.h>
#include <stdlib.h>

// 不指定寄存器实现两个整数相加
int Add(int a, int b)
{
    __asm__ __volatile__
    (
        //"lock;\n"
        "addl %1,%0;\n"
        : "=m"(a)
        : "r"(b), "m"(a)
      //  :
    );

    return a;
}
```
##sub.c
-------

```c
#include <stdio.h>
#include <stdlib.h>

// 不指定寄存器实现两个参数相减
int Sub(int a, int b)
{
    __asm__ __volatile__
    (
        "subl %1, %0;"
        : "=m"(a)
        : "r"(b), "m"(a)
 //       :
    );

    return a;
}
```

##testelf.c
-------

```c
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int a = 3, b = 5;

    printf("%d + %d = %d\n", a, b, Add(a, b));
    printf("%d - %d = %d\n", a, b, Sub(a, b));


    return EXIT_SUCCESS;
}
```


##Makefile
-------

```c

target=testelf_normal testelf_dynamic testelf_static

MAIN_OBJS=testelf.o
SUBS_OBJS=add.o sub.o

DYNA_FILE=libtestelf.so
STAT_FILE=libtestelf.a

all:$(target)

%.o : %.c
	$(CC) -c $^ -o $@

clean :
	rm -rf $(MAIN_OBJS) $(SUBS_OBJS)
	rm -rf $(DYNA_FILE) $(STAT_FILE)
	rm -rf $(target)


# Complie the execute
testelf_normal:$(MAIN_OBJS) $(SUBS_OBJS)
	gcc $^ -o $@

testelf_dynamic:$(MAIN_OBJS) $(DYNA_FILE)
	gcc  $^ -o $@ -L./ -ltestelf

testelf_static:$(MAIN_OBJS) $(STAT_FILE)
	gcc  testelf.o -o $@ -static -L./ -ltestelf



# Complie the Dynamic Link Library libtestelf.so
libtestelf.so:$(SUBS_OBJS)
	gcc -fPCI -shared $^ -o $@

# Complie the Static Link Library libtestelf.so
libtestelf.a:$(SUBS_OBJS)
	ar -r $@ $^
```

我们编写了两个库函数分别实现add和sub的功能, 然后编写了一个测试代码testelf.c调用了Add和Sub.

然后我们的Mmakefile为测试程序编写了3分程序

1.	普通的程序testelf_normal, 由add.o sub.o 和testelf.o直接链接生成

2.	动态链接程序testelf_dynamic, 将add.o和sub.o先链接成动态链接库libtestelf.so, 然后再动态链接生成testelf_dynamic

3.	静态链接程序testelf_static, 将add.o和sub.o先静态链接成静态库libtestelf.a, 然后再静态链接生成可执行程序testelf_staticke


我们在源代码目录执行make后会完成编译, 编译完成后

*	add.o, sub.o和testelf.o是可重定位的对象文件(Relocatable file)

*	libtestelf.so是可被共享的对象文件(Shared object file)

*	testelf_normal, testelf_dynamic和testelf_static是可执行的对象文件(Executable file)

如下图所示

![对象文件](./images/object_file.jpg)

#ELF可执行与链接文件格式详解
-------

##布局和结构
-------

http://blog.csdn.net/dc_726/article/details/45921979
http://blog.chinaunix.net/uid-26430381-id-3408417.html
http://www.cnblogs.com/brianhxh/archive/2009/07/04/1517020.html
http://blog.csdn.net/wu5795175/article/details/7657580
http://baike.baidu.com/link?url=WF2n6700ckIT8r3YByfvz3_hnDa5uuXvnBOLrruyQXrWEvtNCfyE6h2MubPDZ7JXxkmgz1R07v_5P4jOtatNlkSbyYtExonbBCsSnwfwVb7
http://blog.chinaunix.net/uid-9068997-id-2010376.html
http://jzhihui.iteye.com/blog/1447570
http://www.cnblogs.com/xmphoenix/archive/2011/10/23/2221879.html
http://www.360doc.com/content/13/0821/12/7377734_308735271.shtml
http://www.lxway.com/805909004.htm
ELF文件由各个部分组成。

为了方便和高效，ELF文件内容有两个平行的视角:一个是程序连接角度，另一个是程序运行角度

![elf文件的布局和结构](./images/elf_layout.png)

首先**图的左边部分，它是以链接视图来看待elf文件的**, 从左边可以看出，包含了一个ELF头部，它描绘了整个文件的组织结构。它还包括很多节区（section）。这些节有的是系统定义好的，有些是用户在文件在通过.section命令自定义的，链接器会将多个输入目标文件中的相同的节合并。节区部分包含链接视图的大量信息：指令、数据、符号表、重定位信息等等。除此之外，还包含程序头部表（可选）和节区 头部表，程序头部表，告诉系统如何创建进程映像。用来构造进程映像的目标文件必须具有程序头部表，可重定位文件不需要这个表。而节区头部表（Section Heade Table）包含了描述文件节区的信息，每个节区在表中都有一项，每一项给出诸如节区名称、节区大小这类信息。用于链接的目标文件必须包含节区头部表，其他目标文件可以有，也可以没有这个表。

需要注意地是：尽管图中显示的各个组成部分是有顺序的，实际上除了 ELF 头部表以外，其他节区和段都没有规定的顺序。

**右半图是以程序执行视图来看待的**，与左边对应，多了一个段（segment）的概念，编译器在生成目标文件时，通常使用从零开始的相对地址，而在链接过程中，链接器从一个指定的地址开始，根据输入目标文件的顺序，以段（segment）为单位将它们拼装起来。其中每个段可以包括很多个节（section）。


-	elf头部

    除了用于标识ELF文件的几个字节外, ELF头还包含了有关文件类型和大小的有关信息, 以及文件加载后程序执行的入口点信息

-	程序头表(program header table)

	程序头表向系统提供了可执行文件的数据在进程虚拟地址空间中组织文件的相关信息。他还表示了文件可能包含的段数据、段的位置和用途

-	段segment

    各个段保存了与文件爱你相关的各种形式的数据, 例如，符号表、实际的二进制码、固定值(如字符串)活程序使用的数值常数

-	节头表section

	包含了与各段相关的附加信息。

##ELF的数据类型定义
-------

在具体介绍ELF的格式之前，我们先来了解在ELF文件中都有哪些数据类型的定义：

ELF数据编码顺序与机器相关，为了使数据结构更加通用, linux内核自定义了几种通用的数据, 使得数据的表示与具体体系结构分离, 数据类型有六种


| 名称 | 大小 | 对齐 | 目的 |
| ------------- |:-------------:|:-------------:|:-------------:|
| Elf32_Addr | 4 | 4 | 无符号程序地址 |
| Elf32_Half | 2 | 2 | 无符号中等整数 |
| Elf32_Off | 4 | 4 | 无符号文件偏移 |
| Elf32_SWord | 4 | 4 | 有符号大整数 |
| Elf32_Word | 4 | 4 | 无符号大整数 |
| unsigned char | 1 | 1 | 无符号小整数 |

http://lxr.free-electrons.com/source/include/uapi/linux/elf.h?v=4.5#L218
##ELF头部
-------



























#execve系统调用
-------

##execve系统调用
-------

我们前面提到了, fork, vfork等复制出来的进程是父进程的一个副本, 那么如何我们想加载新的程序, 可以通过execve来加载和启动新的程序。

>x86架构下, 其实还实现了一个新的exec的系统调用叫做execveat(自linux-3.19后进入内核)
>
>[syscalls,x86: Add execveat() system call](http://lwn.net/Articles/600344)

##exec()函数族
-------

exec函数一共有六个，其中execve为内核级系统调用，其他（execl，execle，execlp，execv，execvp）都是调用execve的库函数。


```c
int execl(const char *path, const char *arg, ...);
int execlp(const char *file, const char *arg, ...);
int execle(const char *path, const char *arg,
                  ..., char * const envp[]);
int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
```

#ELF文件格式以及可执行程序的表示
-------

##struct linux_bin_rpm结构描述一个可执行程序
-------

linux_binprm是定义在[include/linux/binfmts.h](http://lxr.free-electrons.com/source/include/linux/binfmts.h#L14)中, 用来保存要要执行的文件相关的信息

```c
/*
* This structure is used to hold the arguments that are used when loading binaries.
*/
struct linux_binprm {
    char buf[BINPRM_BUF_SIZE];	// 保存可执行文件的头128字节
#ifdef CONFIG_MMU
    struct vm_area_struct *vma;
    unsigned long vma_pages;
#else
# define MAX_ARG_PAGES  32
    struct page *page[MAX_ARG_PAGES];
#endif
    struct mm_struct *mm;
    unsigned long p; /* current top of mem , 当前内存页最高地址*/
    unsigned int
            cred_prepared:1,/* true if creds already prepared (multiple
                             * preps happen for interpreters) */
            cap_effective:1;/* true if has elevated effective capabilities,
                             * false if not; except for init which inherits
                             * its parent's caps anyway */
#ifdef __alpha__
    unsigned int taso:1;
#endif
    unsigned int recursion_depth; /* only for search_binary_handler() */
    struct file * file;			/*  要执行的文件  */
    struct cred *cred;      /* new credentials */
    int unsafe;             /* how unsafe this exec is (mask of LSM_UNSAFE_*) */
    unsigned int per_clear; /* bits to clear in current->personality */
    int argc, envc;		/*  命令行参数和环境变量数目  */
    const char * filename;  /* Name of binary as seen by procps, 要执行的文件的名称  */
    const char * interp;    /* Name of the binary really executed. Most
                               of the time same as filename, but could be
                               different for binfmt_{misc,script} 要执行的文件的真实名称，通常和filename相同  */
    unsigned interp_flags;
    unsigned interp_data;
    unsigned long loader, exec;
};
```

##struct linux_binfmt可执行程序的结构
-------
linux内核对所支持的每种可执行的程序类型都有个struct linux_binfmt的数据结构，定义如下

>linux_binfmt定义在[include/linux/binfmts.h](http://lxr.free-electrons.com/source/include/linux/binfmts.h#L74)中

```c
/*
  * This structure defines the functions that are used to load the binary formats that
  * linux accepts.
  */
struct linux_binfmt {
    struct list_head lh;
    struct module *module;
    int (*load_binary)(struct linux_binprm *);
    int (*load_shlib)(struct file *);
    int (*core_dump)(struct coredump_params *cprm);
    unsigned long min_coredump;     /* minimal dump size */
 };
```
这里的linux_binfmt对象包含了一个单链表，这个单链表中的第一个元素的地址存储在formats这个变量中list_for_each_entry依次应用load_binary的方法，同时我们可以看到这里会有递归调用，bprm会记录递归调用的深度。

装载ELF可执行程序的load_binary的方法叫做load_elf_binary方法，下面会进行具体分析



