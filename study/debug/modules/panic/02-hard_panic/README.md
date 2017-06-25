Linux Kernel PANIC(二)--Hard Panic/Aieee实例分析
=======

本文信息

| CSDN | GitHub |
|:----:|:------:|
| [Linux Kernel PANIC(二)--Hard Panic/Aieee实例分析](http://blog.csdn.net/gatieme/article/details/73712595) | [`LDD-LinuxDeviceDrivers/study/debug/modules/panic/02-hard_panic`](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/debug/modules/panic/02-hard_panic) |


同类博文信息

| CSDN | GitHub |
|:----:|:------:|
| [Linux Kernel PANIC(一)--概述(Hard Panic/Aieee和Soft Panic/Oops)](http://blog.csdn.net/gatieme/article/details/73711897) | [`LDD-LinuxDeviceDrivers/study/debug/modules/panic/01-kernel_panic`](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/debug/modules/panic/01-kernel_panic) |
| [Linux Kernel PANIC(二)--Hard Panic/Aieee实例分析](http://blog.csdn.net/gatieme/article/details/73712595) | [`LDD-LinuxDeviceDrivers/study/debug/modules/panic/02-hard_panic`](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/debug/modules/panic/02-hard_panic) |

<br>
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处
<br>


对于 `hard panic` 而言, 最大的可能性是驱动模块的中断处理(`interrupt handler`)导致的, 一般是因为驱动模块在中断处理程序中访问一个空指针(`null pointer`).

一旦发生这种情况，驱动模块就无法处理新的中断请求，最终导致系统崩溃.

关于 Hard Panic 的详细信息, 请参见[Kernel PANIC概述(Hard Panic/Aieee和Soft Panic/Oops)]()



#1 实例1--Kernel panic-not syncing fatal exception
-------

##1.1 问题描述
-------

今天就遇到一个客户机器内核报错 : "Kernel panic-not syncing fatal exception"

重启后正常, 几个小时后出现同样报错，系统down了，有时重启后可恢复有时重启后仍然报同样的错误。

我先来解释一下什么是fatal exception?
“致命异常（fatal exception）表示一种例外情况，这种情况要求导致其发生的程序关闭。通常，异常（exception）可能是任何意想不到的情况（它不仅仅包括程序错误）。致命异常简单地说就是异常不能被妥善处理以至于程序不能继续运行。
软件应用程序通过几个不同的代码层与操作系统及其他应用程序相联系。当异常（exception）在某个代码层发生时，为了查找所有异常处理的代码，各个代码层都会将这个异常发送给下一层，这样就能够处理这种异常。如果在所有层都没有这种异常处理的代码，致命异常（fatal exception）错误信息就会由操作系统显示出来。这个信息可能还包含一些关于该致命异常错误发生位置的秘密信息（比如在程序存储范围中的十六进制的位置）。这些额外的信息对用户而言没有什么价值，但是可以帮助技术支持人员或开发人员调试程序。
当致命异常（fatal exception）发生时，操作系统没有其他的求助方式只能关闭应用程序，并且在有些情况下是关闭操作系统本身。当使用一种特殊的应用程序时，如果反复出现致命异常错误的话，应将这个问题报告给软件供应商。 ”
而且此时键盘无任何反应，必然使用reset键硬重启。
panic.c源文件有个方法，当panic挂起后，指定超时时间，可以重新启动机器

##1.2 解决方法
-------


>`vi /etc/sysctl.conf`  添加

```cpp
kernel.panic = 20 #panic error中自动重启，等待timeout为20秒
kernel.sysrq=1 #激活Magic SysRq  否则，键盘鼠标没有响应
```

按住 `[ALT]+[SysRq]+[COMMAND]`, 这里 SysRq` 是 `Print SCR` 键, 而 `COMMAND` 按以下来解释！

| 命令 | 描述 |
|:---:|:---:|
| b |立即重启
| e | 发送SIGTERM给init之外的系统进程 |
| o | 关机 |
| s | sync同步所有的文件系统 |
| u | 试图重新挂载文件系统 |

#2	示例2--Kernel panic-not syncing fatal exception in interrupt
-------

##2.1 问题描述
-------

很多网友安装 `Linux` 出现 `Kernel panic-not syncing fatal exception in interrupt` 是由于网卡驱动原因。

##2.2 解决方法
-------


将 `BIOS`选项 `"Onboard Lan"` 的选项 `"Disabled"`, 重启从光驱启动即可.

等安装完系统之后, 再进入 `BIOS` 将 "Onboard Lan" 的选项给 `"enable"`, 下载相应的网卡驱动安装。

如出现以下报错 :

```CPP
init() r8168 … 
          … …
         … ：Kernel panic: Fatal exception
```

`r8168` 是网卡型号.

在 `BIOS` 中禁用网卡, 从光驱启动安装系统. 再从网上下载网卡驱动安装.

```cpp
#tar  vjxf  r8168-8.014.00.tar.bz2
# make  clean  modules       (as root or with sudo)
      # make  install
      # depmod  -a
      # modprobe  r8168
```

安装好系统后 `reboot` 进入 `BIOS` 把网卡打开.

另有网友在 `Kernel panic` 出错信息中看到 `"alc880"`, 这是个声卡类型. 尝试着将声卡关闭, 重启系统, 搞定.


#3	示例3--Kernel panic-not syncing fatal exception
-------

##3.1	问题描述
-------


安装 `linux` 系统遇到安装完成之后， 无法启动系统出现 `Kernel panic-not syncing fatal exception`.

##3.2 解决方法
-------


很多情况是由于板载声卡、网卡、或是cpu 超线程功能 (`Hyper-Threading`) 引起的.


这类问题的解决办法就是先查看错误代码中的信息, 找到错误所指向的硬件, 将其禁用. 系统启动后, 安装好相应的驱动, 再启用该硬件即可.

>另外出现 `"Kernel Panic — not syncing: attempted to kill init"` 和 `"Kernel Panic — not syncing: attempted to kill idle task"`
>
>有时把内存互相换下位置或重新插拔下可以解决问题。



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
