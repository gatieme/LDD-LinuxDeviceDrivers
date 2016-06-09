ELF文件的加载和动态链接过程
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------------- |:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|
| 2016-06-04 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度-之-进程的描述](http://blog.csdn.net/gatieme/article/category/6225543) |



#加载和动态链接
-------

从编译/链接和运行的角度看，应用程序和库程序的连接有两种方式。
一种是固定的、静态的连接，就是把需要用到的库函数的目标代码（二进制）代码从程序库中抽取出来，链接进应用软件的目标映像中；

另一种是动态链接，是指库函数的代码并不进入应用软件的目标映像，应用软件在编译/链接阶段并不完成跟库函数的链接，而是把函数库的映像也交给用户，到启动应用软件目标映像运行时才把程序库的映像也装入用户空间（并加以定位），再完成应用软件与库函数的连接。

这样，就有了两种不同的ELF格式映像。

*	一种是静态链接的，在装入/启动其运行时无需装入函数库映像、也无需进行动态连接。

*	另一种是动态连接，需要在装入/启动其运行时同时装入函数库映像并进行动态链接。

Linux内核既支持静态链接的ELF映像，也支持动态链接的ELF映像，而且装入/启动ELF映像必需由内核完成，而动态连接的实现则既可以在内核中完成，也可在用户空间完成。

因此，GNU把对于动态链接ELF映像的支持作了分工：

把ELF映像的装入/启动入在Linux内核中；而把动态链接的实现放在用户空间（glibc），并为此提供一个称为"解释器"(ld-linux.so.2)的工具软件，而解释器的装入/启动也由内核负责，这在后面我们分析ELF文件的加载时就可以看到

 这部分主要说明ELF文件在内核空间的加载过程，下一部分对用户空间符号的动态解析过程进行说明。

#Linux可执行文件类型的注册机制
-------

在说明ELF文件的加载过程以前，我们先回答一个问题，就是：

>**为什么Linux可以运行ELF文件？**
>
>内核对所支持的每种可执行的程序类型都有个struct linux_binfmt的数据结构，这个结构我们在前面的博文中我们已经提到, 但是没有详细讲. 其定义如下

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

linux_binfmt定义在include/linux/binfmts.h中


linux支持其他不同格式的可执行程序, 在这种方式下, linux能运行其他操作系统所编译的程序, 如MS-DOS程序, 活BSD Unix的COFF可执行格式, 因此linux内核用struct linux_binfmt来描述各种可执行程序。

linux内核对所支持的每种可执行的程序类型都有个struct linux_binfmt的数据结构，
其提供了3种方法来加载和执行可执行程序

| 函数 | 描述 |
| ------------- |:-------------:|
| load_binary | 通过读存放在可执行文件中的信息为当前进程建立一个新的执行环境 |
| load_shlib | 用于动态的把一个共享库捆绑到一个已经在运行的进程, 这是由uselib()系统调用激活的 |
| core_dump | 在名为core的文件中, 存放当前进程的执行上下文. 这个文件通常是在进程接收到一个缺省操作为”dump”的信号时被创建的, 其格式取决于被执行程序的可执行类型 |

所有的linux_binfmt对象都处于一个链表中, 第一个元素的地址存放在formats变量中, 可以通过调用register_binfmt()和unregister_binfmt()函数在链表中插入和删除元素, 在系统启动期间, 为每个编译进内核的可执行格式都执行registre_fmt()函数. 当实现了一个新的可执行格式的模块正被装载时, 也执行这个函数, 当模块被卸载时, 执行unregister_binfmt()函数.

当我们执行一个可执行程序的时候, 内核会list_for_each_entry遍历所有注册的linux_binfmt对象, 对其调用load_binrary方法来尝试加载, 直到加载成功为止.


 其中的load_binary函数指针指向的就是一个可执行程序的处理函数。而我们研究的ELF文件格式的linux_binfmt结构对象elf_format, 定义如下, 在[/fs/binfmt.c](http://lxr.free-electrons.com/source/fs/binfmt_elf.c#L84)中

```c
static struct linux_binfmt elf_format = {
	.module      = THIS_MODULE,
	.load_binary = load_elf_binary,
	.load_shlib      = load_elf_library,
	.core_dump       = elf_core_dump,
	.min_coredump    = ELF_EXEC_PAGESIZE,
	.hasvdso     = 1
};
```

要支持ELF文件的运行，则必须向内核登记注册elf_format这个linux_binfmt类型的数据结构，加入到内核支持的可执行程序的队列中。内核提供两个函数来完成这个功能，一个注册，一个注销，即：

```c
int register_binfmt(struct linux_binfmt * fmt)
int unregister_binfmt(struct linux_binfmt * fmt)
```


**当需要运行一个程序时，则扫描这个队列，依次调用各个数据结构所提供的load处理程序来进行加载工作，ELF中加载程序即为`load_elf_binary`，内核中已经注册的可运行文件结构linux_binfmt会让其所属的加载程序load_binary逐一前来认领需要运行的程序binary，如果某个格式的处理程序发现相符后，便执行该格式映像的装入和启动**

#ELF文件的加载load_elf_binary
-------


#内核空间的加载过程
-------

内核中实际执行execv()或execve()系统调用的程序是do_execve()，这个函数先打开目标映像文件，并从目标文件的头部（第一个字节开始）读入若干（当前Linux内核中是128）字节（实际上就是填充ELF文件头，下面的分析可以看到），然后调用另一个函数search_binary_handler()，在此函数里面，它会搜索我们上面提到的Linux支持的可执行文件类型队列，让各种可执行程序的处理程序前来认领和处理。如果类型匹配，则调用load_binary函数指针所指向的处理函数来处理目标映像文件。

在ELF文件格式中，处理函数是load_elf_binary函数，下面主要就是分析load_elf_binary函数的执行过程（说明：因为内核中实际的加载需要涉及到很多东西，这里只关注跟ELF文件的处理相关的代码）：
 
```c
struct pt_regs *regs = current_pt_regs();
struct {
    struct elfhdr elf_ex;
    struct elfhdr interp_elf_ex;
} *loc;
struct arch_elf_state arch_state = INIT_ARCH_ELF_STATE;

loc = kmalloc(sizeof(*loc), GFP_KERNEL);
if (!loc) {
    retval = -ENOMEM;
    goto out_ret;
}

/* Get the exec-header
	使用映像文件的前128个字节对bprm->buf进行了填充  */
loc->elf_ex = *((struct elfhdr *)bprm->buf);

retval = -ENOEXEC;
/* First of all, some simple consistency checks
	比较文件头的前四个字节
    。*/
if (memcmp(loc->elf_ex.e_ident, ELFMAG, SELFMAG) != 0)
    goto out;
/*	还要看映像的类型是否ET_EXEC和ET_DYN之一；前者表示可执行映像，后者表示共享库  */
if (loc->elf_ex.e_type != ET_EXEC && loc->elf_ex.e_type != ET_DYN)
	goto out;
```
 在load_elf_binary之前，内核已经使用映像文件的前128个字节对bprm->buf进行了填充，563行就是使用这此信息填充映像的文件头（具体数据结构定义见第一部分，ELF文件头节），然后567行就是比较文件头的前四个字节，查看是否是ELF文件类型定义的“\177ELF”。除这4个字符以外，还要看映像的类型是否ET_EXEC和ET_DYN之一；前者表示可执行映像，后者表示共享库。


```c
710         elf_phdata = load_elf_phdrs(&loc->elf_ex, bprm->file);
711         if (!elf_phdata)
712                 goto out;
713
714         elf_ppnt = elf_phdata;
715         elf_bss = 0;
716         elf_brk = 0;
717 
718         start_code = ~0UL;
719         end_code = 0;
720         start_data = 0;
721         end_data = 0;
722 
723         for (i = 0; i < loc->elf_ex.e_phnum; i++) {
724                 if (elf_ppnt->p_type == PT_INTERP) {
725                         /* This is the program interpreter used for
726                          * shared libraries - for now assume that this
727                          * is an a.out format binary
728                          */
729                         retval = -ENOEXEC;
730                         if (elf_ppnt->p_filesz > PATH_MAX || 
731                             elf_ppnt->p_filesz < 2)
732                                 goto out_free_ph;
733 
734                         retval = -ENOMEM;
735                         elf_interpreter = kmalloc(elf_ppnt->p_filesz,
736                                                   GFP_KERNEL);
737                         if (!elf_interpreter)
738                                 goto out_free_ph;
739 
740                         retval = kernel_read(bprm->file, elf_ppnt->p_offset,
741                                              elf_interpreter,
742                                              elf_ppnt->p_filesz);
743                         if (retval != elf_ppnt->p_filesz) {
744                                 if (retval >= 0)
745                                         retval = -EIO;
746                                 goto out_free_interp;
747                         }

577         /* Now read in all of the header information */  
580         if (loc->elf_ex.e_phnum < 1 ||  
581                 loc->elf_ex.e_phnum > 65536U / sizeof(struct elf_phdr))  
582                 goto out;  
583         size = loc->elf_ex.e_phnum * sizeof(struct elf_phdr);  
               ……  
585         elf_phdata = kmalloc(size, GFP_KERNEL);  
               ……  
589         retval = kernel_read(bprm->file, loc->elf_ex.e_phoff,  
590                              (char *)elf_phdata, size);  
```

这块就是通过kernel_read读入整个program header table。从代码中可以看到，一个可执行程序必须至少有一个段（segment），而所有段的大小之和不能超过64K。
 
```
614 elf_ppnt = elf_phdata;  
                ……  
623 for (i = 0; i < loc->elf_ex.e_phnum; i++) {  
624     if (elf_ppnt->p_type == PT_INTERP) {  
            ……  
635         elf_interpreter = kmalloc(elf_ppnt->p_filesz, GFP_KERNEL);  
            ……  
640         retval = kernel_read(bprm->file, elf_ppnt->p_offset,  
641                          elf_interpreter,  
642                          elf_ppnt->p_filesz);  
            ……  
682         interpreter = open_exec(elf_interpreter);  
            ……  
695         retval = kernel_read(interpreter, 0, bprm->buf,  
696                          BINPRM_BUF_SIZE);  
            ……  
703         /* Get the exec headers */  
            ……  
705         loc->interp_elf_ex = *((struct elfhdr *)bprm->buf);  
706             break;  
707     }  
708     elf_ppnt++;  
709 }  
```language
```
 这个for循环的目的在于寻找和处理目标映像的“解释器”段。“解释器”段的类型为PT_INTERP，找到后就根据其位置的p_offset和大小p_filesz把整个“解释器”段的内容读入缓冲区（640~640）。事个“解释器”段实际上只是一个字符串，即解释器的文件名，如“/lib/ld-linux.so.2”。有了解释器的文件名以后，就通过open_exec()打开这个文件，再通过kernel_read()读入其开关128个字节（695~696），即解释器映像的头部。我们以Hello World程序为例，看一下这段中具体的内容：

其实从readelf程序的输出中，我们就可以看到需要解释器/lib/ld-linux.so.2，为了进一步的验证，我们用hd命令以16进制格式查看下类型为INTERP的段所在位置的内容，在上面的各个域可以看到，它位于偏移量为0x000114的位置，文件内占19个字节：

从上面红色部分可以看到，这个段中实际保存的就是“/lib/ld-linux.so.2”这个字符串。
 
C代码  收藏代码
814         for(i = 0, elf_ppnt = elf_phdata;  
815             i < loc->elf_ex.e_phnum; i++, elf_ppnt++) {  
                       ……   
819                 if (elf_ppnt->p_type != PT_LOAD)  
820                         continue;  
                       ……   
870                 error = elf_map(bprm->file, load_bias + vaddr, elf_ppnt,  
871                                 elf_prot, elf_flags);  
                       ……  
920         }  
 
 这段代码从目标映像的程序头中搜索类型为PT_LOAD的段（Segment）。在二进制映像中，只有类型为PT_LOAD的段才是需要装入的。当然在装入之前，需要确定装入的地址，只要考虑的就是页面对齐，还有该段的p_vaddr域的值（上面省略这部分内容）。确定了装入地址后，就通过elf_map()建立用户空间虚拟地址空间与目标映像文件中某个连续区间之间的映射，其返回值就是实际映射的起始地址。
 
C代码  收藏代码
946     if (elf_interpreter) {  
                ……  
951         elf_entry = load_elf_interp(&loc->interp_elf_ex,  
952                                 interpreter,  
953                                     &interp_load_addr);  
                                ……  
965     } else {  
966         elf_entry = loc->elf_ex.e_entry;  
                ……  
972     }  
 这段程序的逻辑非常简单：如果需要装入解释器，就通过load_elf_interp装入其映像（951~953），并把将来进入用户空间的入口地址设置成load_elf_interp()的返回值，即解释器映像的入口地址。而若不装入解释器，那么这个入口地址就是目标映像本身的入口地址。
 
C代码  收藏代码
991        create_elf_tables(bprm, &loc->elf_ex,  
992                           (interpreter_type == INTERPRETER_AOUT),  
993                           load_addr, interp_load_addr);  
               ……  
1028       start_thread(regs, elf_entry, bprm->p);  
 在完成装入，启动用户空间的映像运行之前，还需要为目标映像和解释器准备好一些有关的信息，这些信息包括常规的argc、envc等等，还有一些“辅助向量（Auxiliary Vector）”。这些信息需要复制到用户空间，使它们在CPU进入解释器或目标映像的程序入口时出现在用户空间堆栈上。这里的create_elf_tables()就起着这个作用。
 
       最后，start_thread()这个宏操作会将eip和esp改成新的地址，就使得CPU在返回用户空间时就进入新的程序入口。如果存在解释器映像，那么这就是解释器映像的程序入口，否则就是目标映像的程序入口。那么什么情况下有解释器映像存在，什么情况下没有呢？如果目标映像与各种库的链接是静态链接，因而无需依靠共享库、即动态链接库，那就不需要解释器映像；否则就一定要有解释器映像存在。
       以我们的Hello World为例，gcc在编译时，除非显示的使用static标签，否则所有程序的链接都是动态链接的，也就是说需要解释器。由此可见，我们的Hello World程序在被内核加载到内存，内核跳到用户空间后并不是执行Hello World的，而是先把控制权交到用户空间的解释器，由解释器加载运行用户程序所需要的动态库（Hello World需要libc），然后控制权才会转移到用户程序。
 
ELF文件中符号的动态解析过程
 
       上面一节提到，控制权是先交到解释器，由解释器加载动态库，然后控制权才会到用户程序。因为时间原因，动态库的具体加载过程，并没有进行深入分析。大致的过程就是将每一个依赖的动态库都加载到内存，并形成一个链表，后面的符号解析过程主要就是在这个链表中搜索符号的定义。
       我们后面主要就是以Hello World为例，分析程序是如何调用printf的：
查看一下gcc编译生成的Hello World程序的汇编代码（main函数部分）：
 
C代码  收藏代码
08048374 <main>:  
 8048374:       8d 4c 24 04         lea     0x4(%esp),%ecx  
                ……  
 8048385:       c7 04 24 6c 84 04 08    movl    $0x804846c,(%esp)  
 804838c:       e8 2b ff ff ff          call        80482bc <puts@plt>  
 8048391:       b8 00 00 00 00          mov     $0x0,%eax  
 从上面的代码可以看出，经过编译后，printf函数的调用已经换成了puts函数（原因读者可以想一下）。其中的call指令就是调用puts函数。但从上面的代码可以看出，它调用的是puts@plt这个标号，它代表什么意思呢？在进一步说明符号的动态解析过程以前，需要先了解两个概念，一个是global offset table，一个是procedure linkage table。
 
 
       Global Offset Table（GOT）
       在位置无关代码中，一般不能包含绝对虚拟地址（如共享库）。当在程序中引用某个共享库中的符号时，编译链接阶段并不知道这个符号的具体位置，只有等到动态链接器将所需要的共享库加载时进内存后，也就是在运行阶段，符号的地址才会最终确定。因此，需要有一个数据结构来保存符号的绝对地址，这就是GOT表的作用，GOT表中每项保存程序中引用其它符号的绝对地址。这样，程序就可以通过引用GOT表来获得某个符号的地址。
       在x86结构中，GOT表的前三项保留，用于保存特殊的数据结构地址，其它的各项保存符号的绝对地址。对于符号的动态解析过程，我们只需要了解的就是第二项和第三项，即GOT[1]和GOT[2]：GOT[1]保存的是一个地址，指向已经加载的共享库的链表地址（前面提到加载的共享库会形成一个链表）；GOT[2]保存的是一个函数的地址，定义如下：GOT[2] = &_dl_runtime_resolve，这个函数的主要作用就是找到某个符号的地址，并把它写到与此符号相关的GOT项中，然后将控制转移到目标函数，后面我们会详细分析。
       Procedure Linkage Table（PLT）
       过程链接表（PLT）的作用就是将位置无关的函数调用转移到绝对地址。在编译链接时，链接器并不能控制执行从一个可执行文件或者共享文件中转移到另一个中（如前所说，这时候函数的地址还不能确定），因此，链接器将控制转移到PLT中的某一项。而PLT通过引用GOT表中的函数的绝对地址，来把控制转移到实际的函数。
       在实际的可执行程序或者共享目标文件中，GOT表在名称为.got.plt的section中，PLT表在名称为.plt的section中。
大致的了解了GOT和PLT的内容后，我们查看一下puts@plt中到底是什么内容：
 
C代码  收藏代码
Disassembly of section .plt:  
  
0804828c <__gmon_start__@plt-0x10>:  
 804828c:       ff 35 68 95 04 08       pushl   0x8049568  
 8048292:       ff 25 6c 95 04 08       jmp     *0x804956c  
 8048298:       00 00  
        ......  
0804829c <__gmon_start__@plt>:  
 804829c:       ff 25 70 95 04 08       jmp     *0x8049570  
 80482a2:       68 00 00 00 00          push        $0x0  
 80482a7:       e9 e0 ff ff ff          jmp     804828c <_init+0x18>  
  
080482ac <__libc_start_main@plt>:  
 80482ac:       ff 25 74 95 04 08       jmp     *0x8049574  
 80482b2:       68 08 00 00 00          push        $0x8  
 80482b7:       e9 d0 ff ff ff          jmp     804828c <_init+0x18>  
080482bc <puts@plt>:  
 80482bc:       ff 25 78 95 04 08       jmp     *0x8049578  
 80482c2:       68 10 00 00 00          push    $0x10  
 80482c7:       e9 c0 ff ff ff          jmp     804828c <_init+0x18>  
 可以看到puts@plt包含三条指令，程序中所有对有puts函数的调用都要先来到这里（Hello World里只有一次）。可以看出，除PLT0以外（就是__gmon_start__@plt-0x10所标记的内容），其它的所有PLT项的形式都是一样的，而且最后的jmp指令都是0x804828c，即PLT0为目标的。所不同的只是第一条jmp指令的目标和push指令中的数据。PLT0则与之不同，但是包括PLT0在内的每个表项都占16个字节，所以整个PLT就像个数组（实际是代码段）。另外，每个PLT表项中的第一条jmp指令是间接寻址的。比如我们的puts函数是以地址0x8049578处的内容为目标地址进行中跳转的。
 
顺着这个地址，我们进一步查看此处的内容：
 
C代码  收藏代码
(gdb) x/w  0x8049578  
0x8049578 <_GLOBAL_OFFSET_TABLE_+20>:   0x080482c2  
 从上面可以看出，这个地址就是GOT表中的一项。它里面的内容是0x80482c2，即puts@plt中的第二条指令。前面我们不是提到过，GOT中这里本应该是puts函数的地址才对，那为什么会这样呢？原来链接器在把所需要的共享库加载进内存后，并没有把共享库中的函数的地址写到GOT表项中，而是延迟到函数的第一次调用时，才会对函数的地址进行定位。
 
puts@plt的第二条指令是pushl $0x10，那这个0x10代表什么呢？
 
C代码  收藏代码
Relocation section '.rel.plt' at offset 0x25c contains 3 entries:  
 Offset       Info       Type            Sym.Value  Sym. Name  
08049570  00000107 R_386_JUMP_SLOT      00000000   __gmon_start__  
08049574  00000207 R_386_JUMP_SLOT      00000000   __libc_start_main  
08049578  00000307 R_386_JUMP_SLOT  00000000   puts  
 其中的第三项就是puts函数的重定向信息，0x10即代表相对于.rel.plt这个section的偏移位置（每一项占8个字节）。其中的Offset这个域就代表的是puts函数地址在GOT表项中的位置，从上面puts@plt的第一条指令也可以验证这一点。向堆栈中压入这个偏移量的主要作用就是为了找到puts函数的符号名（即上面的Sym.Name域的“puts”这个字符串）以及puts函数地址在GOT表项中所占的位置，以便在函数定位完成后将函数的实际地址写到这个位置。
 
puts@plt的第三条指令就跳到了PLT0的位置。这条指令只是将0x8049568这个数值压入堆栈，它实际上是GOT表项的第二个元素，即GOT[1]（共享库链表的地址）。
随即PLT0的第二条指令即跳到了GOT[2]中所保存的地址（间接寻址），即_dl_runtime_resolve这个函数的入口。
_dl_runtime_resolve的定义如下：
 
C代码  收藏代码
_dl_runtime_resolve:  
    pushl %eax      # Preserve registers otherwise clobbered.  
    pushl %ecx  
    pushl %edx  
    movl 16(%esp), %edx # Copy args pushed by PLT in register.  Note  
    movl 12(%esp), %eax # that `fixup' takes its parameters in regs.  
    call _dl_fixup      # Call resolver.  
    popl %edx       # Get register content back.  
    popl %ecx  
    xchgl %eax, (%esp)  # Get %eax contents end store function address.  
    ret $8          # Jump to function address.  
 从调用puts函数到现在，总共有两次压栈操作，一次是压入puts函数的重定向信息的偏移量，一次是GOT[1]（共享库链表的地址）。上面的两次movl操作就是将这两个数据分别取到edx和eax，然后调用_dl_fixup（从寄存器取参数），此函数完成的功能就是找到puts函数的实际加载地址，并将它写到GOT中，然后通过eax将此值返回给_dl_runtime_resolve。xchagl这条指令，不仅将eax的值恢复，而且将puts函数的值压到栈顶，这样当执行ret指令后，控制就转移到puts函数内部。ret指令同时也完成了清栈动作，使栈顶为puts函数的返回地址（main函数中call指令的下一条指令），这样，当puts函数返回时，就返回到正确的位置。
 
       当然，如果是第二次调用puts函数，那么就不需要这么复杂的过程，而只要通过GOT表中已经确定的函数地址直接进行跳转即可。下图是前面过程的一个示意图，红色为第一次函数调用的顺序，蓝色为后续函数调用的顺序（第1步都要执行）。
       
http://jzhihui.iteye.com/blog/1447570


#
