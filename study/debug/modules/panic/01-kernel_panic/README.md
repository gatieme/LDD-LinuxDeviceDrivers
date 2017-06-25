Linux Kernel PANIC(一)--概述(Hard Panic/Aieee和Soft Panic/Oops)
=======


本文信息

| CSDN | GitHub |
|:----:|:------:|
| [Linux Kernel PANIC(一)--概述(Hard Panic/Aieee和Soft Panic/Oops)](http://blog.csdn.net/gatieme/article/details/73711897) | [`LDD-LinuxDeviceDrivers/study/debug/modules/panic/01-kernel_panic`](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/debug/modules/panic/01-kernel_panic) |


同类博文信息

| CSDN | GitHub |
|:----:|:------:|
| [Linux Kernel PANIC(一)--概述(Hard Panic/Aieee和Soft Panic/Oops)](http://blog.csdn.net/gatieme/article/details/73711897) | [`LDD-LinuxDeviceDrivers/study/debug/modules/panic/01-kernel_panic`](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/debug/modules/panic/01-kernel_panic) |

<br>
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处
<br>

#1	Kernel PANIC/OOPS
-------


>wiki:A kernel panic is an action taken by an operating system upon detecting an internal fatal error from which it cannot safely recover. The term is largely specific to Unix and Unix-like systems; for Microsoft Windowsoperating systems the equivalent term is “Bug check” (or, colloquially, “Blue Screen of Death“).
The kernel routines that handle panics (in AT&T-derived and BSD Unix source code, a routine known as panic()) are generally designed to output an error message to the console, dump an image of kernel memory to disk for post-mortemdebugging and then either wait for the system to be manually rebooted, or initiate an automatic reboot. The information provided is of highly technical nature and aims to assist a system administrator or software developer in diagnosing the problem.
Attempts by the operating system to read an invalid or non-permitted memory address are a common source of kernel panics. A panic may also occur as a result of a hardware failure or a bug in the operating system. In many cases, the operating system could continue operation after memory violations have occurred. However, the system is in an unstable state and rather than risking security breaches and data corruption, the operating system stops to prevent further damage and facilitate diagnosis of the error.
          The kernel panic was introduced in an early version of Unix and demonstrated a major difference between the design philosophies of Unix and its predecessor Multics. Multics developer Tom van Vleck recalls a discussion of this change with Unix developer Dennis Ritchie:
I remarked to Dennis that easily half the code I was writing in Multics was error recovery code. He said, “We left all that stuff out. If there’s an error, we have this routine called panic, and when it is called, the machine crashes, and you holler down the hall, ‘Hey, reboot it.’”[1]
The original panic() function was essentially unchanged from Fifth Edition UNIX to the VAX-based UNIX 32V and output only an error message with no other information, then dropped the system into an endless idle loop. As the Unixcodebase was enhanced, the panic() function was also enhanced to dump various forms of debugging information to the console.

##1.1 什么是 `Kernel PANIC`
-------

`panic` 是英文中是**惊慌**的意思, `Linux Kernel panic` 正如其名, `Linux Kernel` 不知道如何走了, 它会尽可能把它此时能获取的全部信息都打印出来, 为开发人员调试提供帮助.

有两种主要类型 `Kernel panic`

*	Hard Panic(也就是Aieee信息输出)

*	Soft Panic(也就是Oops信息输出)

##1.2	什么会导致Linux Kernel Panic
-------

只有加载到内核空间的驱动模块才能直接导致 `kernel panic`, 你可以在系统正常的情况下, 使用 `lsmod` 查看当前系统加载了哪些模块. 除此之外, 内建在内核里的组件(比如 `memory map`等)也能导致panic.

常见Linux Kernel Panic报错内容

```CPP
Kernel panic-not syncing fatal exception in interrupt
kernel panic – not syncing: Attempted to kill the idle task!
kernel panic – not syncing: killing interrupt handler!
Kernel Panic – not syncing：Attempted to kill init !
```





一般出现下面的情况，就认为是发生了 `kernel panic` :

*	机器彻底被锁定，不能使用

*	数字键(Num Lock)，大写锁定键(Caps Lock)，滚动锁定键(Scroll Lock)不停闪烁。

*	如果在终端下，应该可以看到内核dump出来的信息（包括一段"Aieee"信息或者"Oops"信息）

*	和Windows蓝屏相似




>因为 `hard panic` 和 `soft panic` 本质上不同，因此我们分别讨论.

#2	hard panic
-------

对于 `hard panic` 而言, 最大的可能性是驱动模块的中断处理(`interrupt handler`)导致的, 一般是因为驱动模块在中断处理程序中访问一个空指针(`null pointer`).

一旦发生这种情况，驱动模块就无法处理新的中断请求，最终导致系统崩溃.

##2.1	原因
-------

对于 `hard panic` 而言，最大的可能性是驱动模块的中断处理(`interrupt handler`)导致的，一般是因为驱动模块在中断处理程序中访问一个空指针(`null pointre`).

一旦发生这种情况, 驱动模块就无法处理新的中断请求, 最终导致系统崩溃.

##2.2	信息收集
-------

根据 `panic` 的状态不同, 内核将记录所有在系统锁定之前的信息. 因为 `kenrel panic` 是一种很严重的错误, 不能确定系统能记录多少信息, 下面是一些需要收集的关键信息, 他们非常重要，因此尽可能收集全，当然如果系统启动的时候就kernel panic，那就无法只知道能收集到多少有用的信息了。


1.	`/var/log/messages`, 幸运的时候, 整个 `kernel panic` 栈跟踪信息都能记录在这里. 要确认是否有一个足够的栈跟踪信息，你只要查找包含"EIP"的一行, 它显示了是什么函数和模块调用时导致`panic`.

2.	应用程序/库 日志: 可能可以从这些日志信息里能看到发生 `panic` 之前发生了什么。

3.	其他发生 `panic` 之前的信息, 或者知道如何重现panic那一刻的状态

4.	终端屏幕 `dump`信息, 一般 `OS`被锁定后, 复制, 粘贴肯定是没戏了, 因此这类信息, 你可以需要借助数码相机或者原始的纸笔工具了.

如果 `kernel dump` 信息既没有在 `/var/log/message` 里, 也没有在屏幕上, 那么尝试下面的方法来获取(当然是在还没有死机的情况下).

1.	如果在图形界面，切换到终端界面，dump信息是不会出现在图形界面的，甚至都不会在图形模式下的虚拟终端里.

2.	确保屏幕不黑屏，可以使用下面的几个方法：

    ```cpp
    setterm -blank 0
    setterm -powerdown 0
    setvesablank off
    ```
3.	从终端，拷贝屏幕信息

##2.3	排查方法
-------
栈跟踪信息(`stack trace`)是排查 `kernel panic` 最重要的信息, 该信息如果在/var/log/messages日志里当然最好，因为可以看到全部的信息，如果仅仅只是在屏幕上，那么最上面的信息可能因为滚屏消失了，只剩下栈跟踪信息的一部分。如果你有一个完整栈跟踪信息的话，那么就可能根据这些充分的信息来定位panic的根本原因。要确认是否有一个足够的栈跟踪信息，你只要查找包含”EIP”的一行，它显示了是什么函数和模块调用时导致panic


使用内核调试工具(`kenrel debugger ,aka KDB`)
如果跟踪信息只有一部分且不足以用来定位问题的根本原因时, `kernel debugger(KDB)`就需要请出来了。

KDB编译到内核里，panic发生时，他将内核引导到一个shell环境而不是锁定。这样，我们就可以收集一些与panic相关的信息了，这对我们定位问题的根本原因有很大的帮助。




#3	soft panic
-------

1.	症状没有 `hard panic`严重

2.	通常导致段错误(`segmentation fault`)

3.	以看到一个 `oops` 信息, `/var/log/message`s里可以搜索到'Oops'.

4.	机器稍微还能用(但是收集信息后，应该会重启系统).

##3.1	原因
-------

凡是非中断处理引发的模块崩溃都将导致 `soft panic`

在这种情况下, 驱动本身会崩溃, 但是还不至于让系统出现致命性失败, 因为它没有锁定中断处理例程. 导致 `hard panic`的原因同样对`soft panic`也有用(比如在运行时访问一个空指针).

##3.2	信息收集
-------

当 `soft panic`发生时, 内核将产生一个包含内核符号(`kernel symbols`)信息的 `dump`数据, 这个将记录在`/var/log/messages`里.

##3.3	排查方法
-------

为了开始排查故障, 可以使用 `ksymoops` 工具来把内核符号信息转成有意义的数据.

为了生成ksymoops文件,需要

*	从 `/var/log/messages` 里找到的堆栈跟踪文本信息保存为一个新文件。确保删除了时间戳(`timestamp`)，否则 `ksymoops` 会失败.

*	运行 `ksymoops` 程序（如果没有，请安装）
详细的`ksymoops`执行用法，可以参考`ksymoops(8)`手册。

`Oops` 可以看成是内核级的 `Segmentation Fault`.

*	应用程序如果进行了非法内存访问或执行了非法指令, 会得到 `Segfault` 信号, 一般的行为是 `coredump`, 应用程序也可以自己截获`Segfault` 信号，自行处理

*	如果内核自己犯了这样的错误, 则会打出 `Oops` 信息.

处理器使用的所有地址几乎都是通过一个复杂的页表结构对物理地址映射而得到的虚拟地址(除了内存管理子系统自己所使用的物理地址)。当一个非法的指针被废弃时，内存分页机制将不能为指针映射一个物理地址，处理器就会向操作系统发出一个页故障信号。如果地址不合法，那么内核将不能在该地址“布页”；这时如果处理器处于超级用户模式，内核就会生成一条oops消息。


#4	参考资料
-------

[根据内核Oops 定位代码工具使用— addr2line 、gdb、objdump](http://blog.csdn.net/u012719256/article/details/53365155)

[转载_Linux内核OOPS调试](http://blog.csdn.net/tommy_wxie/article/details/12521535)


[kernel panic/kernel oops分析](http://blog.chinaunix.net/uid-20651662-id-1906954.html)


[DebuggingKernelOops](https://wiki.ubuntu.com/DebuggingKernelOops)


[kerneloops package in Ubuntu](https://launchpad.net/ubuntu/+source/kerneloops)


[Understanding a Kernel Oops!](http://opensourceforu.com/2011/01/understanding-a-kernel-oops/)


[Kernel oops错误](http://blog.163.com/prodigal_s/blog/static/204537164201411611432884/)


[Kernel Oops Howto](http://madwifi-project.org/wiki/DevDocs/KernelOops)


[Kernel Panics](https://wiki.archlinux.org/index.php/Kernel_Panics)

[WiKipedia](https://en.wikipedia.org/wiki/Linux_kernel_oops)

[Oops中的error code解释](http://blog.csdn.net/mozun1/article/details/53306714)

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
<br>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可
