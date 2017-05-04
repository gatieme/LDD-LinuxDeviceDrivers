AderXCoding
=======

| CSDN | GitHub |
|:----:|:------:|
| [Aderstep--紫夜阑珊-青伶巷草](http://blog.csdn.net/gatieme) | [`AderXCoding/system/tools`](https://github.com/gatieme/AderXCoding/tree/master/system/tools) |


<br>
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处
<br>


`Systemtap` 是内核调试的强大工具, `systemtap` 从版本 `1.0` 开始变支持 `arm` 处理器, 但是只支持 `OMAP` 的, 由于"工作"需要将 `systemtap` 移植到运行 `Android`.


`systemtap` 里面有三个重要的工具：translator，compiler，staprun。
translator负责将stp脚本翻译为c代码；
compiler根据runtime环境和内核路径来生成module；
staprun负责装载、卸载模块，输出模块探测的信息。

translator和compiler都是在宿主机上运行，staprun在目标板上运行。

官方资料 http://omappedia.org/wiki/Systemtap


为其他机器生成测量工具

当运行一个 `SystemTap` 脚本的时候, `SystemTap` 会在脚本外构建一个内核模块, `SystemTap` 然后把这个内核模块加载进内核, 允许它直接从内核提取指定的数据.


正常情况下, `SystemTap` 仅仅会运行在部署了 `SystemTap` 的系统上. 这意味着, 如果你想在 `10` 个系统上运行 `SystemTap`, 你必须把 `SystemTap` 部署到所有的系统上. 有时候, 这可能既不可行也不理想. 比如, 公司政策禁止管理员在指定的机器上安装 `RPM` 包来提供编译和 `debug` 信息, 从而防止 `SystemTap` 的部署. 为了解决这一问题, `SystemTap` 允许你使用 `Cross-instrumentation`.

`Cross-instrumentation` 是一个从一台计算机上的 `SystemTap` 脚本生成 `SystemTap` 测量模块并在另一台计算机上使用的过程. 这个过程提供了以下好处：

*	各种主机的内核信息包可以被安装在单台主机上

*	每台目标机器仅仅需要被安装一个 `RPM` 包, 为了使用生成的 `SystemTap` 测量模块 : `systemtap-runtime` 包

为了简单起见，在这一节中使用以下术语：

*	测量模块 - 从 `SystemTap` 脚本构建的内核模. `SystemTap` 模块在主机系统上被构建, 将在目标系统的内核上被加载.

*	主机系统 - `SystemTap` 脚本编译测量模块的系统, 为了在目标系统上加载它们.

*	目标系统 - SystemTap 脚本构建测量模块系统

*	目标内核 - 目标系统的内核，这个内核用于你加载或运行测量模块

为了配置一个主机系统和一个目标系统, 需要完成以下步骤 :

*	在每台目标系统安装 `systemtap-runtime` 包

*	通过在每台主机上运行 `uname -r` 命令来决定运行在每台目标系统上的内核

*	在主机系统上安装 `SystemTap`. 在主机系统上，你将可以为目标系统构建测量指令. 关于怎样安装 `SystemTap` 的指令, 可以参考"SystemTap 学习笔记 - 安装篇".

*	前期确定目标系统的内核版本, 安装目标内核和在主机系统上的相关的 `RPM` 包, 如 "SystemTap 学习笔记 - 安装篇" 中的 "手动安装必需的内核信息包" 所述. 如果多个目标系统使用不同的目标内核, 为在目标系统上使用的每个不同的内核重复这一步骤.

完成这些步骤后, 你现在可以在主机系统上构建测量模块. 为了构建测量模块, 在主机系统上运行以下命令(一定要指定适当的值) :

```cpp
stap -r kernel_version script -m module_name
```
(
这里, `kernel_version` 涉及到目标内核的版本(在目标系统上通过 `uname -r` 命令输出), `script` 涉及到转换成测量模块的脚本, `module_name` 涉及测量模块要求的名称.

>注 : 为了确定运行的内核的架构, 你可以使用以下命令
>
>```cpp
>uname -m
>```

一旦测量模块被编译完成, 拷贝它到目标系统, 然后用下面的命令加载它 :

```cpp
staprun module_name.ko
```


#示例
-------

例如， 为 `2.6.18-92.1.10.el5` (`x86_64` 架构) 的目标内核从一个名称为 `simple.stp` 的 `SystemTap` 脚本创建一个测量模块 `simple.ko`， 使用以下命令 :

```cpp
stap -r 2.6.18-92.1.10.el5 -e 'probe vfs.read {exit()}' -m simple
```

这将创建一个名为 `simple.ko` 的模块, 为了使用这个测量模块, 拷贝它到目标系统, 然后在目标系统运行以下命令 :

```cpp
staprun simple.ko
```

>注 : 重要！！！！
>主机系统必须与目标系统是相同的架构以及相同的 `Linux` 发行版, 为了使构建的测量模块能正常工作.

参考资料

    http://sourceware.org/systemtap/SystemTap_Beginners_Guide/using-systemtap.html


#参考资料
-------

[SystemTap 学习笔记 - 安装篇](https://segmentfault.com/a/1190000000671438)


[systemtap交叉编译](http://blog.chinaunix.net/uid-10705106-id-3886742.html)


[omap移植systemtap](http://blog.csdn.net/sunnybeike/article/details/7776800)


[移植安装SystemTap到Android的板子上](http://www.codeweblog.com/%E7%A7%BB%E6%A4%8D%E5%AE%89%E8%A3%85systemtap%E5%88%B0android%E7%9A%84%E6%9D%BF%E5%AD%90%E4%B8%8A/)


[systemtap在android、pandaboard上的移植](http://blog.csdn.net/lzuzhp06/article/details/7778189)


[systemtap安装&内核调试环境配置](http://blog.csdn.net/sunnybeike/article/details/7752005)


[SystemTap的使用](http://blog.chinaunix.net/uid-7585066-id-2048719.html)


<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
<br>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可


http://www.davidhill.co/2012/02/enable-esxi-shell-vsphere-5-0/


https://zhidao.baidu.com/question/583512262.html

http://blog.csdn.net/yumushui/article/details/38776603
