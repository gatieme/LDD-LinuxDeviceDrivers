Linux内核调试的方式以及工具集锦
=======

| CSDN | GitHub |
|:----:|:------:|
| [Linux内核调试的方式以及工具集锦](http://blog.csdn.net/gatieme/article/details/68948080) | [`LDD-LinuxDeviceDrivers/study/debug`](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/debug) |

<br>
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作
因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 也欢迎大家提供一些其他好的调试工具以供收录, 鄙人在此谢谢啦
<br>


摘要 : `SYSTEMTAP` 是内核开发者必备的工具, 通过自动化的 `kprobe`, 而这个软件依赖于**内核符号表/调试信息/内核头文件**等等, 如果我们想测试最新的内核特性, 就需要安装带有 `debuginfo` 的内核和相关头文件. 而我们源仓库中自带的`systemtap` 很多时候, 因为版本和内核符号表的变动, 在新的内核中无法运行, 因此很多情况下我们需要自己编译 `systemtap`.

本文主要讲解了 `ubuntu` 上如果在自己编译的内核下编译安装使用 `systemtap`.


`systemtap` 依赖于 `elfutils`, 因此我们需要下载其源码包, 最好选择最新版下载.

*	最近的 `elfultils` 的下载地址: https://fedorahosted.org/releases/e/l/elfutils/

*	最新systemtap 的下载地址:http://sourceware.org/systemtap/ftp/releases/


>参考
>
>[发行版本内核上system的安装与使用](http://sourceware.org/systemtap/wiki/SystemtapOnUbuntu)
>
>[unbuntu下自定义内核上systemtap的安装](http://blog.chinaunix.net/uid-7427553-id-2627651.html)
>
>[如何在ubuntu上基于自定义内核安装systemtap](http://blog.csdn.net/trochiluses/article/details/9698449)
>
>[Optional: Installing on a custom kernel](https://www.ibm.com/support/knowledgecenter/linuxonibm/liaai.systemTap/liaaisystapcustom.htm)


如果您需要使用`systemtap`, 则系统中必须包含以下包或者内核配置

*	内核支持并配置了kprobe(2.6.11和以上)调试内核需要,在进行内核编译之前的配置

*	内核模块编译环境(即编译内核模块所需的内核头文件以及模块配置信息，对于Fedora core或Redhat指kernel-devel或kernel-smp-devel RPM包, Uuuntu的),对应自己编译的内核就是内核头文件。

*	内核调试信息debuginfo(对于Fedora core或Redhat指kernel-debuginfo RPM包)


*	C编译环境(即libc库头文件和编译工具链), 对于自己编译的内核, 这一点比较好满足，因为运行环境和我们编译内核的环境是一致的, 不用特殊处理


*	有 `libdwfl` 的 `elfutils`(只有支持 `libwdfl` 的 `elfutils`, `systemtap`才能正常工作


#1	构建和安装内核debuginfo
-------

`SystemTap` 需要通过内核调试信息来定位内核函数和变量的位置. 这些信息由`kernel-debuginfo`包来提供

如果是系统自带的内核, 可以从源中下载对应的`kernel-debuginfo`包

对于通常的发行版, 并没有安装`kernel-debuginfo`包我们可以到发行版的下载站点下载

安装`debuginfo`包有如下几种方式

*	从发行版的 `debuginfo` 站点, 下载对应的 `kernel-debuginfo` 包并安装

*	直接用`yum/apt`等包管理器安装, 如果仓库中没有`debuginfo`源, 添加`debuginfo`的源即可

*	使用一些辅助工具下载, 比如 `RedHat` 系列的 `debuinfo-install`和 `debian` 系列的`get-dbgsym`

*	下载内核源码包, 构建出 `debuginfo`


##1.1	CentOS安装 `kernel-debuginfo`
-------


###1.1.1  方法一  下载 `CentOS` 的 `debuginfo` 包
-------

一般的方式从软件仓库中是下载`debuginfo` 软件包, 然后直接安装

去站点 `http://debuginfo.centos.org`, 寻找和你架构和内核完全匹配的 `debuginfo` 包

```cpp
kernel-debuginfo-common-xxxxx
kernel-debuginfo-xxxx
```

>注意 
>
>`kernel-debug-debuginfo` 和 `kernel-debuginfo-common`, 这个版本必须和你的系统的内核版本一模一样, 一个字符不一样都不行, 查看版本的方法是 :
>
>`uname -r`
>2.6.18-308.el5
>
>所以我对应的版本是：
>
>kernel-debug-debuginfo-2.6.18-308.el5.x86_64.rpm
>kernel-debuginfo-common-2.6.18-308.el5.x86_64.rpm


如果你找不到对应的内核版本的 `rpm`, 你可以去 `google1`去寻找, 可以使用 `RedHat` 的 `debuginfo rpm` 包, 或者去其他站点[下载](http://rpm.pbone.net)


###1.1.2	方法二  添加debuginfo源
-------


如果提示没有源, 可以为`debuginfo`包增加repository, 然后直接用源安装

如果`Centos yum repository`上没有`kernel-debuginfo`包, 方法一把这个包及依赖包 `rpm`文件下载到本地安装. 我们也可以手工添加有这个包的`repository`, 然后直接用`yum`安装
在 `/etc/yum.repos.d`新建一个文件，我把他命令为debuginfo 
然后在debuginfo中加入

```cpp
[debuginfo]
name=CentOS-$releasever - debuginfo
baseurl=http://debuginfo.centos.org/5/
gpgcheck=0
enabled=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-5


其他版本的CentOS请自行修改, 版本号和源地址```


##1.1.3	方法三  使用 `debuginfo-install` 工具
-------


当然也可以使用 `debuginfo-install`工具来安装 kernel
```cpp
debuinfo-install kernel-debuginfo-`uname -r`


##1.2	Ubuntu安装 `kernel-debuginfo`
-------

`Ubuntu` 桌面版默认没有安装 `kernel-debug-info`, 因此需要手动安装 `kernel-debug-info` 包.


我的系统是 `Ubuntu 14.04.5`, 内核`4.4.0-72-generic`

查看当前内核版本`uname -r`，

###1.2.1	直接从源中安装
-------


在 `Ubuntu 14.04` 中, `debuginfo`包已经在源中包含了, 因此直接安装即可

```
sudo apt-get install linux-image-`uname -r`-dbgsym
```

如果提示`Unable to locate packag`, 说明软件包不在源中, 可以手动添加源, 或者直接下载deb包安装

添加源的方法如下

```cpp
sudo tee /etc/apt/sources.list.d/ddebs.list << EOF 
deb http://ddebs.ubuntu.com/ trusty          main restricted universe multiverse
deb http://ddebs.ubuntu.com/ trusty-security main restricted universe multiverse
deb http://ddebs.ubuntu.com/ trusty-updates  main restricted universe multiverse
deb http://ddebs.ubuntu.com/ trusty-proposed main restricted universe multiverse
EOF
```
对于不同的发行版, 请自行替换发行版代号, 可以使用`lsb_release -a`, 其中codename即是代号

```cpp
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys ECDCAD72428D7C01
sudo apt-get update
```


添加后, 直接回到方法一, 安装即可

```
sudo apt-get install linux-image-`uname -r`-dbgsym
```




###1.2.3	下载 `debuginfo` 包, 安装
-------



```

http://www.domaigne.com/blog/random/getting-debug-kernel-on-ubuntu/
`

下载对应内核版本的 `debug-info package`

http://ddebs.ubuntu.com/pool/main/l/linux/

这里下载linux-image-4.4.0-72-generic-dbgsym_4.4.0-72.93_amd64.ddeb

```cpp
wget http://ddebs.ubuntu.com/pool/main/l/linux/linux-image-4.4.0-72-generic-dbgsym_4.4.0-72.93_amd64.ddeb

sudo dpkg -i linux-image-4.4.0-72-generic-dbgsym_4.4.0-72.93_amd64.ddeb
```




##1.3	自定义内核安装`kernel-debuginfo`
-------
对应自己编译的内核, 我们需要开启下列选项 :

```cpp
CONFIG_DEBUG_INFO
CONFIG_KPROBES
CONFIG_RELAY
CONFIG_DEBUG_FS
CONFIG_MODULES
CONFIG_MODULE_UNLOAD
CONFIG_UTRACE
```







经过以上安装配置后，就可以跟踪监控内核信息了


2.内核方面的准备：（我们假设要编译的内核源码位置是/usr/src/linux）

    如果你是发行版的内核，没有自己进行编译，请参考这里： http://sourceware.org/systemtap/wiki/SystemtapOnUbuntu
    如果你需要自己编译内核，那么意味着apt-get install的systemtap是不能直接使用的，内核的准备过程如下：
1）首先，编译生成带有内核符号表和调试信息的内核镜像和相关内核头文件。

这个过程中，我们不使用传统的config，make，makeinstall命令(传统命令是make modules_install install headers_install)，建议使用kernel-package工具，可以把内核和内核头文件都打包安装，这样比较方便。具体可以参考这里：http://blog.chinaunix.net/uid-7427553-id-2627651.html
（1）进入源代码目录：$: cd /usr/src/linux
（2）进行相关配置
（3）安装内核源码编译的集成命令： $:sudo apt-get install kernel-package
（4）编译内核源码 $:make-kpkg  --initrd --revision 3.11 --append-to-version -zswap kernel_image kernel_headers

2）安装相应的内核和头文件并设置相关的连接

具体可以参考这篇博文：http://blog.chinaunix.net/uid-7427553-id-2627651.html
上一步生成的内核安装包和头文件的安装包，在运行make-kpkg命令的父目录之中，进入
（1）进入父目录：$cd ..
（2）安装都内核镜像包和头文件包：
        $:dpkg -i [thekernel.deb]
        $:dpkg -i [thekernelheaders.deb]
（3）将编译内核时的build树链接到  /lib/modules/`uname -r`/build(
         内核编译的时候，这一步一般自动完成了，可以使用



#2	systemtap与elfutils的安装
-------


 如果您的系统的 `elfutils` 较旧, 您必须下载 `elfutils` 源码包来编译, `systemtap`能够和 `elfutils` 一块编译, 使用命令

```cpp
./configure  --with-elfutils=/path/to/elfutils/dir  --prefix=yourpatch)
```

    2）elfutils RPM：SystemTap需要elfutils软件包提供的库函数来分析调试信息。目前的SystemTap要求安装elfutils-0.123以上版本。目前最新的版本是0.156，我们尽量选择最新的elfutils，这里是官方下载地址：https://fedorahosted.org/releases/e/l/elfutils/

    运行Systemtap的前提条件是：


1）卸载系统已经安装的systemtap（发行版对应的）：
     首先，如果你的系统已经使用apt-get安装了systemtap，尽量在这一步先卸载已经安装的systemtap。如果不进行卸载，在mmtests等软件运行的时候调用的命令stap来自于原来已经安装的systemtap，是无法在新内核上运行的。
2）安装systemtap和elfutils：
    下载最新的systemtap和elfutils源码：注意，此处一定要下载最新的源码自己安装，否则可能会造成编译以后的软件无法运行
    最近的elfultils的下载地址：https://fedorahosted.org/releases/e/l/elfutils/
    最新systemtap 的下载地址:http://sourceware.org/systemtap/ftp/releases/
     一块编译：
将2个包解压到2个不同的目录，然后进入systemtap包目录，执行
   ./configure  --with-elfutils=/path/to/elfutils/dir  --prefix=yourpatch
    make
    make install
总结：这一步的关键是软件版本一定要是最新的（如果测试最新内核），环境变量要进行设置。

注意：如果我们设置了安装路径，一定要把对应systemtap安装路径下面的bin添加到环境变量PATH之中，否则，mmtests调用stap的时候会发现找不到这个程序。

   安装完成以后，我们使用下面的shell命令进行测试，如果输出hello world，说明systemtap安装成功。
   $:stap -ve 'probe begin { log("hello world") exit() }'

本文来源：谁不小心的CSDN博客 如何在ubuntu上基于自定义内核安装systemtap



<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作
因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 也欢迎大家提供一些其他好的调试工具以供收录, 鄙人在此谢谢啦
<br>
