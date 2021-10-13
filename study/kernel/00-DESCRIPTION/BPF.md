---

title: BPF 子系统
date: 2021-02-15 00:32
author: gatieme
tags:
    - linux
    - tools
categories:
        - 技术积累
thumbnail:
blogexcerpt: 虚拟化 & KVM 子系统

---

<br>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 鄙人在此谢谢啦

**转载请务必注明出处, 谢谢, 不胜感激**

<br>

| 日期 | 作者 | GitHub| CSDN | BLOG |
| ------- |:-------:|:-------:|:-------:|:-------:|
| 2021-02-15 | [成坚-gatieme](https://kernel.blog.csdn.net) | [`AderXCoding/system/tools/fzf`](https://github.com/gatieme/AderXCoding/tree/master/system/tools/fzf) | [使用模糊搜索神器 FZF 来提升办公体验](https://blog.csdn.net/gatieme/article/details/113828826) | [Using FZF to Improve Productivit](https://oskernellab.com/2021/02/15/2021/0215-0001-Using_FZF_to_Improve_Productivity)|


<br>

2   **BPF 系统**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***


# 1 概述
-------


[eBPF kernel-versions.md](https://github.com/iovisor/bcc/blob/master/docs/kernel-versions.md)

[BPF的可移植性和CO-RE (Compile Once – Run Everywhere)](https://www.cnblogs.com/charlieroro/p/14206214.html)

[BCC和libbpf的转换](https://www.cnblogs.com/charlieroro/p/14244276.html)

[Libbpf-tools —— 让 Tracing 工具身轻如燕](https://blog.csdn.net/TiDB_PingCAP/article/details/107953554)

[BCC & libbpf](https://zhuanlan.zhihu.com/p/111037013)

[使用 bpftrace 分析内核](https://developer.aliyun.com/article/741492)

# 2 kprobe
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2015/03/25 | Alexei Starovoitov <ast@plumgrid.com> | [tracing: attach eBPF programs to kprobes](https://lore.kernel.org/patchwork/cover/555274) | NA | v11 ☑ 4.1-rc1 | [PatchWork v11 0/9](https://lore.kernel.org/patchwork/cover/555274) |

# 2 tracepoint
-------

## 2.1 tracepoint
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/04/07 | Alexei Starovoitov <ast@fb.com> | [allow bpf attach to tracepoints](https://lore.kernel.org/patchwork/cover/665728) | BPF 支持 tracepoint 跟踪 | v2 ☑ 4.7-rc1 | [PatchWork v2 00/10](https://lore.kernel.org/patchwork/cover/665728) |

## 2.2 raw tracepoint
-------

raw_tracepoint 相比 tracepoint

1.  效率更高, 跳过了参数的处理, 不必向 tracepoint 那样根据 format 解析了所有字段, 节约了不必要的开销.

2.  自由度更大, 提供了对参数的原始访问, 可以直接操作传入 tracepoint 时的指针, 获取某个成员.

3.  可以获得原始结构体指针. 用于做 key 比较方便, tracepoint 只能拿到里面的各个字段, 有些时候没法做 key.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/03/01 | Jan H. Schönherr | [bpf, tracing: introduce bpf raw tracepoints](https://lore.kernel.org/patchwork/cover/879278) | BPF 支持 raw tracepoint 跟踪, 提供了对 tracepoint 参数的访问. | v1 ☑ 4.17-rc1 | [PatchWork v1](https://lore.kernel.org/patchwork/cover/879278) |
| 2018/12/13 | Matt Mullins <mmullins@fb.com> | [bpf: support raw tracepoints in modules](https://lore.kernel.org/patchwork/cover/1024475) | NA | v2 ☑ 5.0-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/cover/1024475) |
| 2019/04/26 | Matt Mullins <mmullins@fb.com> | [writable contexts for bpf raw tracepoints](https://lore.kernel.org/patchwork/cover/1067299) | 引入 BPF_PROG_TYPE_RAW_TRACEPOINT_WRITABLE, 允许 bpfs 程序修改 tracepoint 中的变量. | v5 ☑ 4.17-rc1 | [PatchWork v5](https://lore.kernel.org/patchwork/cover/1067299) |



# 3 libbpf-tools
-------


[[BPF CO-RE clarification] Use CO-RE on older kernel versions.](https://www.spinics.net/lists/bpf/msg32659.html)

[Help using libbpf with kernel 4.14](https://www.spinics.net/lists/bpf/msg27267.html)

[How to use libbpf-tools with ubuntu 18.04 4.15 kernel #3232](https://github.com/iovisor/bcc/issues/3232)

[How to use libbpf-tools with ubuntu 18.04 4.15 kernel](https://www.gitmemory.com/issue/iovisor/bcc/3232/761744003)


# 4 debuging
-------

[Dumping kernel data structures with BPF](https://lwn.net/Articles/818714)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/03/01 | Jan H. Schönherr | [bpf: implement bpf based dumping of kernel data structures](http://patches.linaro.org/cover/221148) | BPF 支持 raw tracepoint 跟踪, 提供了对 tracepoint 参数的访问. | v1 ☐ | [PatchWork v2](http://patches.linaro.org/cover/221148) |
| 2020/06/23 | Alan Maguire <alan.maguire@oracle.com> | [bpf, printk: add BTF-based type printing](https://lore.kernel.org/patchwork/cover/1261591/) | 在 BPF 程序中通过 printk 打印内核结构体   | v3 ☐ | [PatchWork v3,bpf-next,0/8](https://lore.kernel.org/patchwork/cover/1261591/) |
| 2020/08/06 | Alan Maguire <alan.maguire@oracle.com> | [bpf: add bpf-based bpf_trace_printk()-like support](https://lore.kernel.org/patchwork/cover/1285027) | 在 BPF 程序中通过 trace_printk 打印信息   | RFC ☐ | [PatchWork RFC,bpf-next,0/4](https://lore.kernel.org/patchwork/cover/1285027) |
| 2020/09/28 | Alan Maguire <alan.maguire@oracle.com> | [bpf: add helpers to support BTF-based kernel data display](https://lore.kernel.org/patchwork/cover/1312835) | 在 BPF 程序中通过 printk 打印内核结构体   | v7 ☐ | [PatchWork v7,bpf-next,0/8](https://lore.kernel.org/patchwork/cover/1312835) |
| 2021/06/19 | Alan Maguire <alan.maguire@oracle.com> | [libbpf: BTF dumper support for typed data](https://lore.kernel.org/patchwork/cover/1448989) | 引入 btf_dump__dump_type_data, 可以用来输出结构体的信息. | v5 ☐ | [PatchWork v5,bpf-next,0/3](https://lore.kernel.org/patchwork/cover/1448989) |



# 5 function call
-------

[Calling kernel functions from BPF](https://lwn.net/Articles/856005)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/03/01 | Jan H. Schönherr | [bpf: Support calling kernel function](https://patchwork.kernel.org/project/netdevbpf/cover/20210325015124.1543397-1-kafai@fb.com) | BPF 支持直接调用内核函数 | v2 ☑ 5.13-rc1 | [PatchWork v2,bpf-next,00/14](http://patches.linaro.org/cover/221148) |
| 2021/08/30 | Kumar Kartikeya Dwivedi <memxor@gmail.com> | [Support kernel module function calls from eBPF](https://patchwork.kernel.org/project/netdevbpf/cover/20210830173424.1385796-1-memxor@gmail.com) | 这组补丁允许 BPF 程序调用内核模块的函数, 并修改验证器逻辑以允许无效的内核函数调用, 只要它们作为死代码消除的一部分被修剪. 这样做是为了为 BPF 对象提供更好的运行时可移植性, 它可以有条件地禁用稍后被验证器修剪的部分代码(例如const volatile vars, kconfig选项). libbpf 的修改与内核的修改一起进行, 以支持模块函数调用.
它还将 TCP 拥塞控制对象转换为使用模块 kfunc 支持, 而不是依赖于 IS_BUILTIN ifdef. | v1 ☐ | [PatchWork bpf-next,RFC,v1,0/8](https://patchwork.kernel.org/project/netdevbpf/cover/20210830173424.1385796-1-memxor@gmail.com) |


# 6 printk
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/03/01 | Jan H. Schönherr | [bpf: implement variadic printk helper](https://patchwork.kernel.org/project/netdevbpf/cover/20210913035609.160722-1-davemarchevsky@fb.com) | 这组补丁引入了一个新的接口 bpf_trace_vprintk(), 它的功能与 bpf_trace_printk() 类似, 但通过伪 vararg u64 的数组支持 > 3 个参数, 使用与 bpf_trace_printk() 相同的机制写入 `/sys/kernel/debug/tracing/trace_pipe`, 帮助程序的功能是在 libbpf 问题跟踪程序中请求的. libbpf 的 bpf_printk() 宏被修改为在传递 > 3 个变量时使用 bpf_trace_vprintk(), 否则保留之前的行为, 使用 bpf_trace_printk().<br>
在实现 bpf_seq_printf() 和 bpf_snprintf() 期间添加的 helper函数和宏为 bpf_trace_vprintk() 完成了大部分繁重的工作. 用例很简单: 为 BPF 开发人员提供一个更强大的 printk 将简化 BPF 程序的开发, 特别是在调试和测试期间, 这里往往使用 printk. 这个特性是由 [Andrii 在 libbpf 的 issue 中提出](https://github.com/libbpf/libbpf/issues/315)的. | v2 ☑ 5.13-rc1 | [PatchWork v5,bpf-next,0/9](https://patchwork.kernel.org/project/netdevbpf/cover/20210913035609.160722-1-davemarchevsky@fb.com) |


# 7 MAP
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/08/26 | Kumar Kartikeya Dwivedi <memxor@gmail.com> | [Implement file local storage](https://patchwork.kernel.org/project/netdevbpf/cover/20210826133913.627361-1-memxor@gmail.com/) | 本系列实现了 eBPF LSM 程序的文件本地存储映射. 这允许将映射中数据的生存期与打开的文件描述联系起来. 与其他本地存储映射类型一样, 数据的生存期与结构文件实例相关联. 主要用途是:<br>1. 用于将与 eBPF 程序中打开的文件(而非 fd)关联的数据绑定在一起, 以便在文件消失时释放数据(例如, 检查点checkpoint/恢复用例restore usecase).<br>2. 使用eBPF LSM 在用户空间中实现[辣椒(Capsicum)风格的功能沙盒](https://www.usenix.org/legacy/event/sec10/tech/full_papers/Watson), 使用此机制在文件级别强制执行权限. | v2 ☐ | [PatchWork bpf-next,v2,0/5](https://patchwork.kernel.org/project/netdevbpf/cover/20210826133913.627361-1-memxor@gmail.com) |



# 8 Kernel
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/09/09 | Kumar Kartikeya Dwivedi <memxor@gmail.com> | [bmc-cache](https://www.phoronix.com/scan.php?page=news_item&px=Orange-BPF-Memory-Cache-BMC) | 法国电信巨头奥兰治公司(Orange)已经发布了 "BMC", 作为 (e)BPF 内存缓存缓存, 提供了一个专注于 Linux 内核中模棱两两的用法的缓存. Orange 的开源 BPF 内存缓存允许在标准网络堆栈之前处理模组请求, 据说防撞安全, 并且此模块不需要其他内核模块. 此外, 模组用户空间软件本身可以在 BMC 上运行未经修改的软件. 这种内核 eBPF 缓存可将 Memcached 的吞吐量提高 18 倍, 而没有此内核缓存的夹层则提高了 18 倍. 参考 [GitHub](https://github.com/Orange-OpenSource/bmc-cache) | v2 ☐ | [github Orange-OpenSource/bmc-cache](https://github.com/Orange-OpenSource/bmc-cache) |
| 2021/09/15 | Roman Gushchin <guro@fb.com> | [Scheduler BPF](https://www.phoronix.com/scan.php?page=news_item&px=Orange-BPF-Memory-Cache-BMC) | NA | RFC ☐ | [PatchWork rfc,0/6](https://lore.kernel.org/bpf/CA+khW7i460ey-UFzpMSJ8AP9QeD8ufa4FzLA4PQckNP00ShQSw@mail.gmail.com)<br>*-*-*-*-*-*-*-* <br>[LPC 2021](https://linuxplumbersconf.org/event/11/contributions/954)<br>*-*-*-*-*-*-*-* <br>[LKML](https://lkml.org/lkml/2021/9/16/1049), [LWN](https://lwn.net/Articles/869433) |

<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
