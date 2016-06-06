Linux进程ELF文件格式
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------------- |:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|
| 2016-06-04 | [Linux-4.5](http://lxr.free-electrons.com/source/?v=4.5) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度-之-进程的描述](http://blog.csdn.net/gatieme/article/category/6225543) |


ELF(Executable and Linking Format)是一种对象文件的格式，用于定义不同类型的对象文件(Object files)中都放了什么东西、以及都以什么样的格式去放这些东西。它自最早在 System V 系统上出现后，被 xNIX 世界所广泛接受，作为缺省的二进制文件格式来使用。可以说，ELF是构成众多xNIX系统的基础之一。


ELF代表Executable and Linkable Format。他是一种对可执行文件、目标文件和库使用的文件格式。

他在Linux下成为标准格式已经很长时间, 代替了早年的a.out格式。ELF一个特别的优点在于, 同一文件格式可以用于内核支持的几乎所有体系结构上, 这不仅简化了用户空间工具程序的创建, 也简化了内核自身的程序设计。例如, 在必须为可执行文件生成装载程序例程时。

但是文件格式相同并不意味着不同系统上的程序之间存在二进制兼容性, 例如, FreeBSD和Linux都使用ELF作为二进制格式。尽管二者在文件中组织数据的方式相同。但在系统调用机制以及系统调用的语义方面, 仍然有差别。这也是在没有中间仿真层的情况下, FreeBSD程序不能在linux下运行的原因(反过来同样是如此)。

有一点是可以理解的, 二进制程序不能在不同体系结构交换(例如, 为Alpha CPU编译的Linux二进制程序不能在Sparc Linux上执行), 因为底层的体系结构是完全不同的。但是由于ELF的存在, 对所有体系结构而言, 程序本身的相关信息以及程序的各个部分在二进制文件中编码的方式都是相同的。

Linux不仅将ELF用于用户空间程序和库, 还用于构建模块。内核本身也是ELF格式。

ELF是一种开放格式, 其规范可以自由获取。

#对象文件
-------

首先，你需要知道的是所谓对象文件(Object files)有三个种类：

-	可重定位的对象文件(Relocatable file)

这是由汇编器汇编生成的 .o 文件。后面的链接器(link editor)拿一个或一些 Relocatable object files 作为输入，经链接处理后，生成一个可执行的对象文件 (Executable file) 或者一个可被共享的对象文件(Shared object file)。我们可以使用 ar 工具将众多的 .o Relocatable object files 归档(archive)成 .a 静态库文件。如何产生 Relocatable file，你应该很熟悉了，请参见我们相关的基本概念文章和JulWiki。另外，可以预先告诉大家的是我们的内核可加载模块 .ko 文件也是 Relocatable object file。

-	可执行的对象文件(Executable file)

这我们见的多了。文本编辑器vi、调式用的工具gdb、播放mp3歌曲的软件mplayer等等都是Executable object file。你应该已经知道，在我们的 Linux 系统里面，存在两种可执行的东西。除了这里说的 Executable object file，另外一种就是可执行的脚本(如shell脚本)。注意这些脚本不是 Executable object file，它们只是文本文件，但是执行这些脚本所用的解释器就是 Executable object file，比如 bash shell 程序。

-	可被共享的对象文件(Shared object file)

这些就是所谓的动态库文件，也即 .so 文件。如果拿前面的静态库来生成可执行程序，那每个生成的可执行程序中都会有一份库代码的拷贝。如果在磁盘中存储这些可执行程序，那就会占用额外的磁盘空间；另外如果拿它们放到Linux系统上一起运行，也会浪费掉宝贵的物理内存。如果将静态库换成动态库，那么这些问题都不会出现。动态库在发挥作用的过程中，必须经过两个步骤：

1.	链接编辑器(link editor)拿它和其他Relocatable object file以及其他shared object file作为输入，经链接处理后，生存另外的 shared object file 或者 executable file。

2.	在运行时，动态链接器(dynamic linker)拿它和一个Executable file以及另外一些 Shared object file 来一起处理，在Linux系统里面创建一个进程映像。

以上所提到的 link editor 以及 dynamic linker 是什么东西，你可以参考我们基本概念中的相关文章。对于什么是编译器，汇编器等你应该也已经知道，在这里只是使用他们而不再对他们进行详细介绍。为了下面的叙述方便，你可以下载test.tar.gz包，解压缩后使用"make"进行编译。编译完成后，会在目录中生成一系列的ELF对象文件，更多描述见里面的 README 文件。我们下面的论述都基于这些产生的对象文件。

make所产生的文件，包括 sub.o/sum.o/test.o/libsub.so/test 等等都是ELF对象文件。至于要知道它们都属于上面三类中的哪一种，我们可以使用 file 命令来查看：

#布局和结构
-------

ELF文件由各个部分组成。qing 


























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



