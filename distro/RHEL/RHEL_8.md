2   *   *   OpenAnolis cloud-kernel 分析*   *
=====================

---

title: 树莓派使用资料
date: 2021-05-05 18:40
author: gatieme
tags: hexo
categories:
        - hexo
thumbnail:
blogexcerpt: 树莓派使用资料

---

| CSDN | GitHub | Hexo |
|:----:|:------:|:----:|
| [Aderstep--紫夜阑珊-青伶巷草](http://blog.csdn.net/gatieme) | [`AderXCoding/system/tools`](https://github.com/gatieme/AderXCoding/tree/master/system/tools) | [gatieme.github.io](https://gatieme.github.io) |

<br>

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作

<br>


| 发行版 | 镜像 |
|:-----:|:----:|
| [Rocky Linux](https://rockylinux.org), [Rocky Linux 中文站点](https://rockylinux.cn) |  |
| [AlmaLinux](https://almalinux.org) | [Mirrors](https://mirrors.almalinux.org/isos/x86_64/8.4-beta.html) |

# 1 RHEL 8.4
-------

[BETA 8.4_release_notes](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8-beta/html/8.4_release_notes/index)


[kernel-4.18.0-293.el8.src.rpm](https://mirror.sjtu.edu.cn/almalinux/8.4-beta/BaseOS/Source/Packages/kernel-4.18.0-293.el8.src.rpm)

## 1.1 新支持特性
-------

[8.4 new kernel feature](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8-beta/html/8.4_release_notes/new-features#enhancement_kernel)

### 1.1.1 扩展了 RHEL 8.4 对 eBPF 的支持.
-------

[bugzilla 1780124](https://bugzilla.redhat.com/show_bug.cgi?id=1780124)


扩展伯克利包过滤器(eBPF)是一个内核虚拟机, 允许代码在内核空间中执行, 在受限的沙箱环境中访问有限的功能集. 虚拟机执行一种特殊的类似于程序集的代码.

eBPF 字节码首先加载到内核, 然后进行验证, 通过即时编译将代码转换为本机代码, 然后虚拟机执行代码.

Red Hat 提供了许多利用 eBPF 虚拟机的组件. 每个组件处于不同的开发阶段, 因此目前并不是所有组件都得到完全支持.

RHEL 8.4 支持以下组件

*   BPF 编译器收集(BPF Compiler Collection, BCC)工具包, 提供使用 eBPF 对 Linux 操作系统进行 I/O 分析、组网和监控的工具. BCC 库, 可以开发类似于 BCC 工具包的工具.

*   用于流量控制(tc)的 eBPF 功能, 它允许在内核网络数据路径内进行可编程的包处理.

*   eXpress Data Path (XDP) 特性在特定条件下是受支持的, 该特性在内核网络堆栈处理接收到的数据包之前提供对它们的访问.

*   libbpf 包, 它对于 bpf 相关的应用程序(如 bpftrace 和 bpf/xdp 开发)至关重要.

*   XDP-tools 包包含用于 XDP 特性的用户空间支持实用程序, 现在在 AMD 和 Intel 64 位体系结构上得到了支持.

请注意, 所有其他 eBPF 组件都可以作为技术预览 (Technology Preview) 使用, 除非指定支持某个特定组件.

以下值得注意的 eBPF 组件目前作为技术预览可用:

bpftrace 跟踪语言 AF_XDP 套接字用于连接 eXpress Data Path (XDP) 路径到用户空间有关技术预览组件的更多信息, 请参见 [技术预览](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8-beta/html-single/8.4_release_notes/index#technology-preview_kernel).


bpf rebased to version 5.9 (BZ#1874005)

The bcc package rebased to version 0.16.0 (BZ#1879411)
bpftrace rebased to version 0.11.0 (BZ#1879413)

libbpf rebased to version 0.2.0.1 (BZ#1919345)

### 1.1.2 kmod-redhat-oracleasm
-------

(BZ#1827015)

这次更新添加了新的 kmod-redhat-oracleasm 包, 它提供了 ASMLib 实用程序的内核模块部分. ASM (Oracle Automated Storage Management) 是 Oracle 数据库的数据卷管理器. ASMLib 是一个可选的实用程序, 可以在 Linux 系统上用于管理 Oracle ASM 设备.

> 没找到源代码和 RPM 文件


### 1.1.3 SLAB cache merging disabled by default
-------

(BZ#1871214)

```cpp
- [mm] mm: slub: introduce 'slub_merge' kernel parameter (Rafael Aquini) [1871214]
```

CONFIG_SLAB_MERGE_DEFAULT 内核配置选项被默认禁用, 现在默认情况下SLAB缓存不合并. 关掉的理由是 : 增强 SLAB 分配器的可靠性和缓存使用的可跟踪性.

如果需要先前的板缓存合并行为, 用户可以通过向内核命令行添加 slub_merge 参数来重新启用它. 有关如何设置内核命令行参数的更多信息, 请参见 Customer Portal 上的配置内核命令行参数.

```cpp
$ grep -r slub_merge *

mm/slab_common.c: * a "slub_merge" kernel parameter to enable the feature on demand.
mm/slab_common.c:static int __init setup_slub_merge(char *str)
mm/slab_common.c:__setup_param("slub_merge", slub_merge, setup_slub_merge, 0);
```

之前的内核 SLAB_MERGE 是默认开启的, 可以通过 slub_nomerge 关闭.
当前 RHEL 将 SLAB_MERGE 默认关闭, 通过 slub_merge 开启


### 1.1.4 The ima-evm-utils package rebased to version 1.3.2
-------

### 1.1.5 Levelling IMA and EVM features across supported CPU architectures
-------


### 1.1.6 主动式内存规整, 不过默认情况下已被禁用.
-------

(BZ#1848427)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/02/04 | SeongJae Park <sjpark@amazon.com> | [Proactive compaction for the kernel](https://lwn.net/Articles/817905) | 主动进行内存规整, 而不是之前的按需规整. 添加了一个新的sysctl vm. compaction_pro主动性, 它规定了kcompactd试图维护提交的外部碎片的界限. | v8 ☑ 5.9 | [PatchWork v24](https://lore.kernel.org/patchwork/cover/1257280) |

> 警告:主动压缩可能导致压缩活动增加. 这可能会造成严重的系统范围的影响, 因为属于不同进程的内存页会被移动和重新映射. 因此, 启用主动压缩需要非常小心, 以避免应用程序中的延迟高峰.




### 1.1.7 RHEL8 现在支持较新的 Intel 处理器的错误检测和纠正(EDAC)内核模块功能.
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/06/10 | Marco Elver <elver@google.com> | [EDAC, ie31200: Add Intel Coffee Lake CPU support](https://lore.kernel.org/patchwork/cover/1086706) | EDAC 支持 Coffee Lake 架构支持 | v2 ☑ 5.3-rc1 | [PatchWork v24](https://lore.kernel.org/patchwork/cover/1086706) |
| 2020/11/19 | Marco Elver <elver@google.com> | [EDAC - fix for i10nm driver + upgrade to support Sapphire Rapids](https://patchwork.kernel.org/project/linux-edac/patch/20201119212219.1335-4-tony.luck@intel.com/) | EDAC 新增 Sapphire Rapids 架构 | v2 ☑ 5.3-rc1 | [PatchWork v24](https://patchwork.kernel.org/project/linux-edac/patch/20201119212219.1335-4-tony.luck@intel.com/) |



### 1.1.8 新的 "kpatch-dnf" 包已被添加为 DNF 插件, 用于使用 Kpatch 应用实时内核补丁更新.
-------

kpatch-dnf 包提供了一个 DNF 插件, 这使得订阅 RHEL 系统来进行内核实时补丁更新成为可能. 订阅将影响当前安装在系统上的所有内核, 包括将来将要安装的内核. 有关kpatch-dnf的更多详细信息, 请参见 [dnf-kpatch(8) 手册页面或管理、监控和更新内核文档](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8-beta/html-single/managing_monitoring_and_updating_the_kernel/index?lb_target=production#applying-patches-with-kernel-live-patching_managing-monitoring-and-updating-the-kernel).

### 1.1.9 A new cgroups controller implementation for slab memory
-------

(BZ#1877019)

目前, 单个 SLAB 可以包含不同 memory cgroup 的对象. slab内存控制器提高了slab的利用率(高达45%), 并能够将内存统计从页面级转移到对象级. 此外, 这一变化消除了每个 memory cgroup 每个重复的每个 cpu 和每个节点 slab 缓存集, 并为所有内存控制组建立了一个共同的每个cpu和每个节点slab缓存集.  因此, 您可以显著降低总的内核内存占用, 并观察到对内存碎片的积极影响.

> 注意, 新的更精确的内存计算需要更多的CPU时间. 然而, 在实践中, 这种差异似乎可以忽略不计.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2020/06/23 |  Roman Gushchin <guro@fb.com> | [The new cgroup slab memory controller](https://lore.kernel.org/patchwork/cover/1261793) | 将 SLAB 的统计跟踪统计从页面级别更改为到对象级别. 它允许在 memory cgroup 之间共享 SLAB 页. 这将显著提高 SLAB 利用率(最高可达45%), 并相应降低总的内核内存占用. 不可移动页面数量的减少也对内存碎片产生积极的影响. | v7 ☑ 5.9-rc1 | [PatchWork v7](https://lore.kernel.org/patchwork/cover/1261793/) |


### 1.1.10 RHEL8 内核的时间命名空间支持
-------

(BZ#1548297)

时间名称空间使系统单调时钟和启动时间时钟在 AMD64、Intel 64 和 64 位 ARM 体系结构上使用每个名称空间偏移. 该特性适用于更改 Linux 容器内的日期和时间, 以及在从检查点恢复后对容器内的时钟进行调整. 因此, 用户现在可以独立地为每个容器设置时间.

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/10/11 | Dmitry Safonov <dima@arista.com> | [kernel: Introduce Time Namespace](https://lore.kernel.org/patchwork/cover/1138393) | Time Namespace | v7 ☑ 5.9-rc1 | [PatchWork v7](https://lore.kernel.org/patchwork/cover/1138393) |

### 1.1.11 New feature: Free memory page returning
-------

(BZ#1839055)


### 1.1.12 perf
-------

*   Supports changing the sorting order in perf top

perf top现在可以按任意事件列对样本进行排序, 以防组中的多个事件被抽样, 而不是按第一列排序. 因此, 按数字键将按匹配的数据列对表进行排序.

使用 --group-sort-idx 命令行选项, 可以按列号排序.

[https://lore.kernel.org/patchwork/patch/1215447/](https://lore.kernel.org/patchwork/patch/1215447)
(BZ#1851933)

*   perf now supports adding or removing tracepoints from a running collector without having to stop or restart perf

以前, 要在 record 时从性能记录中添加或删除跟踪点, 必须停止 record 过程.

通过此更新, 您可以通过控制管道接口动态启用和禁用由perf record收集的跟踪点, 而不必停止perf记录进程.

(BZ#1844111)
https://lore.kernel.org/patchwork/cover/1356907/

*   The perf tool now supports recording and displaying absolute timestamps for trace data

通过这个更新, perf脚本现在可以用绝对时间戳记录和显示跟踪数据.

注意:要用绝对时间戳显示跟踪数据, 必须用指定的时钟ID记录数据.


要用绝对时间戳记录数据, 请指定时钟ID:

(BZ#1811839)
[perf tools: Add wallclock time conversion support](https://lore.kernel.org/patchwork/cover/1284429)

*   perf now supports circular buffers that use specified events to trigger snapshots

(BZ#1844086)


dwarves rebased to version 1.19.1

(BZ#1903566)

### 1.1.13 The kabi_whitelist package has been renamed to kabi_stablelist
--------



*   RHEL 8.4 现在正式支持突击移除 NVMe 存储设备.

*   支持 SMB Direct.

*   OpenIMPI 2.0.29 与 FreeIPMI 1.6.6 一起到位.

*   现已支持英特尔 Gen12 Tiger Lake 显卡.


<br>

*      本作品/博文 [成坚(gatieme) @ 内核干货(OSKernelLAB)-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*      采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚 gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*      基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.
