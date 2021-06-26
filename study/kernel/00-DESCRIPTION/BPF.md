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


# 1
-------


[eBPF kernel-versions.md](https://github.com/iovisor/bcc/blob/master/docs/kernel-versions.md)

[BPF的可移植性和CO-RE (Compile Once – Run Everywhere)](https://www.cnblogs.com/charlieroro/p/14206214.html)

[BCC和libbpf的转换](https://www.cnblogs.com/charlieroro/p/14244276.html)

[Libbpf-tools —— 让 Tracing 工具身轻如燕](https://blog.csdn.net/TiDB_PingCAP/article/details/107953554)

[BCC & libbpf](https://zhuanlan.zhihu.com/p/111037013)



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


<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
