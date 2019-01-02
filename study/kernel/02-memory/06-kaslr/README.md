---

title: qemu中使用 9p virtio, 支持 host 和 guest 中共享目录
date: 2018-09-02 18:40
author: gatieme
tags: qemu
categories:
        - qemu
thumbnail: 
blogexcerpt: 在使用qemu调试内核的时候, 如果没有网络，想要部署点驱动或者程序上去都需要重新制作文件系统，本文讲解了如何通过 9p virtio fs 实现在 qemu 和 host 机器上共享文件和目录。

---

| CSDN | GitHub | Hexo |
|:----:|:------:|:----:|
| [qemu中使用 9p virtio, 支持 host 和 guest 中共享目录](https://blog.csdn.net/gatieme/article/details/82912921) | [`AderXCoding/system/tools/qemu/0001-9p_virtio`](https://github.com/gatieme/AderXCoding/tree/master/system/tools/qemu/0001-9p_virtio) | [KernelShow(gatieme.github.io)](https://gatieme.github.io/2018/09/30/2018/09/0003-qemu_use_9pnet_virtio_fs_to_share_folder/index) | 
<br>

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 也欢迎大家提供一些其他好的调试工具以供收录, 鄙人在此谢谢啦

<br>

#1	Linux 地址随机化机制
-------

##1.1	地址随机化机制
-------

*	ASLR(Address space layout  randomization)

地址空间布局随机化, 是参与保护缓冲区溢出问题的一个计算机安全技术. 是为了防止攻击者在内存中能够可靠地对跳转到特定利用函数.

`ASLR` 包括随机排列程序的关键数据区域的位置, 包括可执行的部分、堆、栈及共享库的位置.

*	历史

在 `1997` 年, `Memco` 软件公司实现了一个有限的堆栈随机化作为 `SeOS` 访问控制产品的一部分.
`Linux Pax` 项目第一次创建了 `ASLR` 这个术语. `ASLR` 的第一次设计实现是在 `2001` 年7月. 从 `2002` 年 `10`月开始提供内核栈随机化的实现.

*	作用

`ASLR` 通过制造更多让攻击者预测目标地址的困难以阻碍一些类型的安装攻击. 例如 : 攻击者试图执行返回到 `libc` 的攻击必须要找到要执行的代码, 而其他攻击者试图执行 `shellcode` 注入栈上则必须首先到栈. 在这两种情况下, 系统将模糊攻击者相关的存储器地址. 这些值被猜中，并且错误的猜测由于应用程序崩溃通常是不可恢复的.

*	有效性

地址空间布局随机化是基于攻击者猜测随机化空间位置的可能性降低. 安全是通过增加搜索空间的方式来实现的. 因此, `ASLR` 提供更多的熵存在于随机偏移中时是更有效的. 熵增加或许提高了其随机出现虚拟内存区域的空间量或减少了其随机发生的时期. 该期间通常被实现尽可能小, 因此, 大多数系统必须增加 `VMA` 空间随机化.

要打败随机化, 攻击者必须成功猜出所有他们想要攻击的区域的位置. 为数据区, 如堆和栈, 定制代码或者有用的数据可以被加载, 一个以上的状态可以通过使用NOP滑动代码或数据的重复拷贝被攻击. 如果一个区域被分配到少数值中的一个将被允许攻击成功. 与此相反, 代码区域例如: 基础库, 主要的可执行的需要准确地发现. 通常这些区域被混合, 例如堆栈桢被注入到栈和动态库中.


参考 [维基百科](http://en.wikipedia.org/wiki/Address_space_layout_randomization#Linux)

##1.2	Linux 地址随机化
-------

###1.2.1	用户态地址随机化
-------

`ASLR(Address Space Layout Randomization)` 在 `2005` 年被引入到 `Linux` 的内核 `kernel 2.6.12` 中(参见[`Address space randomization in 2.6`](https://lwn.net/Articles/121845), 当然早在 `2004` 年就以 `patch` 的形式被引入. 随着内存地址的随机化, 使得响应的应用变得随机. 这意味着同一应用多次执行所使用内存空间完全不同, 也意味着简单的缓冲区溢出攻击无法达到目的.

GDB从版本7开始，第一次在Ubuntu 9.10（Karmic）上，被调试的程序可以被关闭ASLR（通过标记位ADDR_NO_RANDOMIZE ）。

此处有坑，笔者有一个Ubuntu 9.10的虚拟机，用了下面将要介绍的全部姿势，死活关闭不了ASLR，后来换成Ubuntu 10.04就没问题了，说明Ubuntu 9.10的版本控制ASLR的方法还不成熟，需要重源码层面确认是否可以关闭开启，真是坑到家了。

可以将进程的 `mmap` 的基址, `stack` 和 `vdso` 页面地址固定下来.
可以通过设置 `kernel.randomize_va_space` 内核参数来设置内存地址随机化的行为.
目前 `randomize_va_space` 的值有三种, 分别是 `[0, 1, 2]`

*	0	表示关闭进程地址空间随机化.

*	1	表示将 `mmap` 的基址, `stack` 和 `vdso` 页面随机化。

*	2	表示在 `1` 的基础上增加栈(`heap`)的随机化。


```cpp
echo 0 >/proc/sys/kernel/randomize_va_space 
```

###1.2.3	KASLR 内核态地址随机化
-------

`Linux` 内核对用户态地址随机化的支持在 `2005` 年的 `2.6.12` 版本就合并到了 `mainline`, 但是内核态的地址随机化却很长一段时间没有动静. `2011` 年的时候, `Dan Rosenberg` 提议增加内核 `ASLR` 的功能但后来一直没有实施下去, 最近Kees Cook向社区提交内核地址随机化的补丁，社区可
能会在最近把这个补丁合并到mainline upstream repo里去。

绕过ASLR并不是一门新的技术，早在2002年的Phrack Issue 59中就已经有一篇论文详细的
描述了原理和细节，个人认为内核空间的ASLR是非常有必要的，在内存和硬盘上隐藏内核空
间地址是一个暂时的方案，比如:


通过用下面这个程序，可以检查是否修改成功(x86_64)：
// gcc -g stack.c -o stack
//
unsigned long sp(void){ asm("mov %rsp, %rax");}
int main(int argc, char **argv)
{
    unsigned long esp = sp();
    printf("Stack pointer (ESP : 0x%lx)\n",esp);
    return 0;
}


关闭前运行结果

-bash-4.1# ./stack
Stack pointer (ESP : 0x7fff50162e50)
-bash-4.1# ./stack
Stack pointer (ESP : 0x7fff5d023730)
-bash-4.1# ./stack
Stack pointer (ESP : 0x7ffff9982180)
-bash-4.1# ./stack
Stack pointer (ESP : 0x7fffb23612a0)
-bash-4.1# ./stack
Stack pointer (ESP : 0x7ffffd5a4980)
-bash-4.1# ./stack
Stack pointer (ESP : 0x7fffbac61bf0)

关闭后运行结果
-bash-4.1# ./stack
Stack pointer (ESP : 0x7fffffffeaf0)
-bash-4.1# ./stack
Stack pointer (ESP : 0x7fffffffeaf0)
-bash-4.1# ./stack
Stack pointer (ESP : 0x7fffffffeaf0)
-bash-4.1# ./stack
Stack pointer (ESP : 0x7fffffffeaf0)
-bash-4.1# ./stack
Stack pointer (ESP : 0x7fffffffeaf0)
-bash-4.1# ./stack
Stack pointer (ESP : 0x7fffffffeaf0)

参考:
http://en.wikipedia.org/wiki/Address_space_layout_randomization
http://xorl.wordpress.com/2011/01/16/linux-kernel-aslr-implementation/
--------------------- 
作者：功名半纸 
来源：CSDN 
原文：https://blog.csdn.net/force_eagle/article/details/8024502 
版权声明：本文为博主原创文章，转载请附上博文链接！

<br>

*	本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*	采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的. 

*	基于本文修改后的作品务必以相同的许可发布. 如有任何疑问，请与我联系.
