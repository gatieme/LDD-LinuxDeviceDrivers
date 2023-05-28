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

本作品采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"> 知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议 </a> 进行许可, 转载请注明出处, 谢谢合作

<a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a>

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 鄙人在此谢谢啦

** 转载请务必注明出处, 谢谢, 不胜感激 **

<br>

| 日期 | 作者 | GitHub| CSDN | BLOG |
| ------- |:-------:|:-------:|:-------:|:-------:|
| 2021-02-15 | [成坚 - gatieme](https://kernel.blog.csdn.net) | [`AderXCoding/system/tools/fzf`](https://github.com/gatieme/AderXCoding/tree/master/system/tools/fzf) | [使用模糊搜索神器 FZF 来提升办公体验](https://blog.csdn.net/gatieme/article/details/113828826) | [Using FZF to Improve Productivit](https://oskernellab.com/2021/02/15/2021/0215-0001-Using_FZF_to_Improve_Productivity)|


<br>

2   **BPF 系统 **
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 eBPF 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***


# 1 概述
-------


[eBPF kernel-versions.md](https://github.com/iovisor/bcc/blob/master/docs/kernel-versions.md)

[BPF 的可移植性和 CO-RE (Compile Once – Run Everywhere)](https://www.cnblogs.com/charlieroro/p/14206214.html)

[BCC 和 libbpf 的转换](https://www.cnblogs.com/charlieroro/p/14244276.html)

[Libbpf-tools —— 让 Tracing 工具身轻如燕](https://blog.csdn.net/TiDB_PingCAP/article/details/107953554)

[BCC & libbpf](https://zhuanlan.zhihu.com/p/111037013)

[使用 bpftrace 分析内核](https://developer.aliyun.com/article/741492)

[狄卫华 --eBPF 技术全面介绍 -- 历史、现在、未来 (阅码场 -- 直播回放)](https://shop.yomocode.com/live_pc/l_627e47efe4b0cedf38b1850f), 直播视频解读 [一文读懂 eBPF 的前世今生](https://mp.weixin.qq.com/s/ww510TUdLG8jd6VzfQnjxw)

[bpftrace Cheat Sheet](https://www.brendangregg.com/BPF/bpftrace-cheat-sheet.html)

[Standardizing BPF](https://lwn.net/Articles/926882)

# 2 工作流程
-------


## 2.1 编译器
-------

自 2019 年 5 月以来, Oracle 一直计划在 GCC 10 中引入 eBPF 后端, 以使 GNU 编译器成为通用内核虚拟机. Oracle 为 GCC 引入 eBPF 支持的倾向是该公司在 Linux 上改进 DTrace 的努力的一部分. 作为编译目标, eBPF 因内核验证器施加的限制而不同, 并且由于架构的安全驱动设计. 目前, 只要违反 eBPF 限制, 后端就会发出错误. 这增加了内核验证器可以接受结果对象的机会, 从而缩短了开发周期. 参见 [](https://www.phoronix.com/scan.php?page=news_item&px=GCC-10-eBPF-Port-Lands).

GCC 的支持 eBPF 经过了 3 个阶段.

1.  首先在 toolchain 里添加 BPF 的最基本的支持, 将 BPF target 添加到 GNU toolchain, 包括 binutils 支持 BPF, 以及让 GCC 能支持一个新的 bpf-unknown-none 的 target.

2.  确保生成的程序能够被内核里的 BPF verifier 验证通过, 从而允许加载到内核中.

3.  最后一个阶段是为 BPF 开发者提供额外工具. 除了编译器和汇编器 (compiler and assembler) 之外, 还需要调试器和模拟器 (debuggers and simulators). 例如 BPF 的 simulator, 用来在 user space 运行, 可以通过 GDB 来调试 BPF program. BPF 就像是某种类型的嵌入式平台一样, 需要针对这种平台创建各种工具才能让普通开发者正常进行开发.


| 日期 | LWN | 翻译 |
|:---:|:---:|:---:|
| 2019/09/27 | [Compiling to BPF with GCC](https://lwn.net/Articles/800606) | [LWN：使用 GCC 编译生成 BPF 程序](https://blog.csdn.net/Linux_Everything/article/details/102513211) |
| 2020/09/25 | [BPF in GCC](https://lwn.net/Articles/831402) | [LWN：GCC 也支持 BPF 了！](https://blog.csdn.net/Linux_Everything/article/details/108806492) |


[GCC-DOC 3.19.14 eBPF Options](https://gcc.gnu.org/onlinedocs/gcc/eBPF-pthttps://lwn.net/Articles/800606/ions.html)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/08/14 | "Jose E. Marchesi" <jose.marchesi-AT-oracle.com> | [eBPF support for GCC](https://lwn.net/Articles/796317) | GCC 支持 eBPF | v1 ☐ | [LWN](https://lwn.net/Articles/796317/) |

## 2.2 加载器
-------

### 2.2.1 mini eBPF library
-------

Alexei Starovoitov 在 v3.18 [BPF syscall, maps, verifier, samples, llvm](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=3c731eba48e1b0650decfc91a839b80f0e05ce8f) 实现最早的 BPF 支持的时候, 引入了一个 [mini eBPF library](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3c731eba48e1b0650decfc91a839b80f0e05ce8f).

紧接着 v3.19 就基于 mini eBPF library 为 `samples/bpf` 样例实现了一个简单的 BPF 加载器 bpf_load. 参见 [samples: bpf: elf_bpf file loader](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=249b812d8005ec38e351ee763ceb85d56b155064). 随后 [Add eBPF hooks for cgroups](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d8c5b17f2bc0de09fbbfa14d90e8168163a579e7) 为 mini eBPF library 实现了 bpf_prog_attach 和 bpf_prog_detach.


```cpp
git log --oneline -- samples/bpf/libbpf.c samples/bpf/libbpf.h
git log --oneline -- samples/bpf/bpf_load.c samples/bpf/bpf_load.h
```

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2014/08/13 | Alexei Starovoitov <ast@plumgrid.com> | [bpf: mini eBPF library, test stubs and verifier testsuite](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3c731eba48e1b0650decfc91a839b80f0e05ce8f) | [BPF syscall, maps, verifier, samples, llvm](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=3c731eba48e1b0650decfc91a839b80f0e05ce8f) 的其中一个补丁, 引入了 [mini eBPF library](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3c731eba48e1b0650decfc91a839b80f0e05ce8f). | v1 ☑✓ 3.18-rc1 | [LORE v4,00/26](https://lore.kernel.org/netdev/1407916658-8731-1-git-send-email-ast@plumgrid.com)<br>*-*-*-*-*-*-*-* <br>[LORE v11,00/12](https://lkml.kernel.org/netdev/1410325808-3657-1-git-send-email-ast@plumgrid.com) |
| 2014/11/26 | Alexei Starovoitov <ast@plumgrid.com> | [samples: bpf: elf_bpf file loader](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=249b812d8005ec38e351ee763ceb85d56b155064) | [allow eBPF programs to be attached to sockets](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fbe3310840c65f3cf97dd90d23e177d061c376f2) 的其中一个补丁. 基于 mini bpf library, 实现了一个简易的加载器 bpf_load, 其主要接口为 `load_bpf_file() -=> load_and_attach()`. | v1 ☑✓ 3.19-rc1 | [LORE v1,0/6](https://lore.kernel.org/all/1417066951-1999-1-git-send-email-ast@plumgrid.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/6](https://lore.kernel.org/lkml/1417475199-15950-1-git-send-email-ast@plumgrid.com) |


### 2.2.2 libbpf
-------

v4.3 的时候, HUAWEI 的王楠为 perf 提供了加载和执行 eBPF 程序的能力, 引入了 `perf bpf` 命令和 `perf-bpf-record` 等子命令. 为了架构更解耦, 将常见的 eBPF 和 eBPF 对象操作放入工具 libbpf 中, 而不是 perf 本身. 其他程序, 如 iproute2, 也可以直接借助 libbpf 来完成自己的工作. libbpf 封装和隐藏了所有对 eBPF 的操作和数据结构, 而 `perf bpf` 则通过调用 libbpf 的 API 来处理 eBPF 程序和访问对象文件的数据.

libbpf 将被编译为 libbpf.a 和 libbpf.so. 它可以分为两部分:

1. 用户内核接口. API 由 `bpf_xxx.h` 定义, 封装映射和程序加载操作. 在 bpf_load_program() 中, 为了提高性能, 它在第一次尝试时不使用日志缓冲区, 并在失败时启用日志缓冲区重试.

2. ELF 操作. 此处定义了 eBPF 对象文件的结构. 此部分的 API 可以在 `libbpf_xxx.h` 中找到.

struct bpf_object 是整个对象文件的处理程序.
structbpf_prog_handler 是程序的处理程序和迭代器. 一些访问者被定义为使调用者能够检索程序的节名和文件描述符. 可以附加更多的访问器.

libpf 明确地将整个过程分为打开和加载阶段. 数据是在 "打开" 阶段收集的. 在 "加载" 阶段调用 BPF 系统调用.


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2015/07/01 | Wang Nan <wangnan0@huawei.com> | [bpf tools: Introduce'bpf'library and add bpf feature check](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9a208effd1832e50e1f7ea002f400f8b9ca8b1ed) | perf 支持 eBPF 系列 [perf tools: filtering events using eBPF programs](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9a208effd1832e50e1f7ea002f400f8b9ca8b1ed) 的 [部分补丁](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9a208effd1832e50e1f7ea002f400f8b9ca8b1ed), 引入了一个 libbpf 的用户态工具, 可用于加载 eBPF 程序. | v10 ☑✓ 4.3-rc1 | [LORE RFC,v1,00/22](https://lkml.org/lkml/2015/4/30/264)<br>*-*-*-*-*-*-*-* <br>[LORE v2,00/37](https://lore.kernel.org/all/1431676290-1230-1-git-send-email-wangnan0@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,00/37](https://lore.kernel.org/all/1431860222-61636-1-git-send-email-wangnan0@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,00/29](https://lore.kernel.org/all/1432704004-171454-1-git-send-email-wangnan0@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v5,00/30](https://lore.kernel.org/all/1433144296-74992-1-git-send-email-wangnan0@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v6,00/32](https://lore.kernel.org/all/1433829036-23687-1-git-send-email-wangnan0@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v7,00/37](https://lore.kernel.org/all/1434087345-127225-1-git-send-email-wangnan0@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v8,00/49](https://lore.kernel.org/all/1435149113-51142-1-git-send-email-wangnan0@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v10,0/50](https://lore.kernel.org/all/1435716878-189507-1-git-send-email-wangnan0@huawei.com)<br>*-*-*-*-*-*-*-* <br>[LORE v11,00/39](https://lore.kernel.org/all/1436361268-234530-1-git-send-email-wangnan0@huawei.com)<br>*-*-*-*-*-*-*-* <br>[PULL, 00/31](https://lore.kernel.org/all/1444826502-49291-1-git-send-email-wangnan0@huawei.com) |
| 2016/12/14 | Joe Stringer <joe@ovn.org> | [Reuse libbpf from samples/bpf](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9899694a7f67714216665b87318eb367e2c5c901) | 内核主线中同时实现了两套 libbpf 的用户态库 libbpf(位于 `tools/lib/bpf`) 和 mini bpf lib(用于 samples 样例, 位于 `samples/bpf/libbpf.c`), 这是非常冗余的, 因此为 `tools/lib/bpf` 下的 libbpf 实现了 `samples/bpf` 所需的 bpf wrapper function, 从而使 samples 可以直接使用 libbpf. | v1 ☑✓ 4.10-rc1 | [LORE v1,0/5](https://lore.kernel.org/all/20161214224342.12858-1-joe@ovn.org) |
| 2018/05/14 | Jakub Kicinski <jakub.kicinski@netronome.com> | [samples: bpf: fix build after move to full libbpf](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=787360f8c2b87d4ae4858bb8736a19c289904885) | mini bpf lib 最终寿终正寝, 只包含了一个 `filter.h` 的 instruction helpers, 因此被重命名为 `bpf_insn.h`. | v2 ☐☑✓ | [LORE v2,0/5](https://lore.kernel.org/all/20180515053506.4345-1-jakub.kicinski@netronome.com) |
| 2020/11/24 | Daniel T. Lee <danieltimlee@gmail.com> | [bpf: remove bpf_load loader completely](https://lore.kernel.org/all/20201124090310.24374-1-danieltimlee@gmail.com) | 将使用 bpf_load 编写的 BPF 程序重写为使用 libbpf 加载器. 使用 libbpf 重构剩余的 bpf 程序, 并完全删除 bpf_load 这个过时的 bpf 加载器, 它已经很难跟上最新的内核 bpf. | v3 ☐☑✓ | [LORE v3,0/7](https://lore.kernel.org/all/20201124090310.24374-1-danieltimlee@gmail.com) |


## 2.3 验证器
-------


## 2.4 执行器
-------


## 2.5 JIT
-------


[bcc/docs/kernel-versions.md--BPF Features by Linux Kernel Version](https://github.com/iovisor/bcc/blob/master/docs/kernel-versions.md)

| 标记 | 描述 |
|:---:|:---:|
| HAVE_CBPF_JIT | 架构下是否实现了  classic BPF JIT, 参考 [Documentation/features/core/cBPF-JIT/arch-support.txt](https://www.kernel.org/doc/Documentation/features/core/cBPF-JIT/arch-support.txt) |
| HAVE_EBPF_JIT | 架构下是否实现了 extended BPF JIT, 参考 [Documentation/features/core/eBPF-JIT/arch-support.txt](https://www.kernel.org/doc/Documentation/features/core/eBPF-JIT/arch-support.txt) |

> 注意, 支持 eBPF JIT 的一定支持 cBPF JIT.

| 架构 | cBPF | eBPF |
|:---:|:-----:|:----:|
| x86 | 3.0 | 3.16 |
| ARM | 3.4-rc1 | 4.14 |
| ARM64 | NA | 3.18 |
| PPC32 | NA | 5.13 |
| PPC64 | 3.1 | 4.8 |
| sparc | 3.5 | 4.7 |
| s390 | 3.7 | 4.1 |
| MIPS | 3.16 | 5.16 |
| RISC-V 32 | NA | 5.7 |
| RISC-V 64 | NA | 5.1 |
| LoongArch | NA | 6.1 |

### 2.5.1 classic BPF JIT
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2011/04/03 | Eric Dumazet <eric.dumazet@gmail.com> | [net: filter: Just In Time compiler](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0a14842f5a3c0e88a1e59fac5c3025db39721f74) | 为了加快数据包过滤, 为 x86_64 实现了 BPF 的 JIT 编译器, 通过 `/proc/sys/net/core/bpf_jit_enable` 开启. 引入了 HAVE_BPF_JIT, 并为 X86_64 做了标记. 参见 LWN 报道 [A JIT for packet filters](https://lwn.net/Articles/437981). | v2 ☐☑✓ 3.0-rc1 | [LORE](https://lore.kernel.org/all/1301838968.2837.200.camel@edumazet-laptop) |
| 2012/07/20 | Matt Evans <matt@ozlabs.org> | [net: filter: BPF 'JIT' compiler for PPC64](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0ca87f05ba8bdc6791c14878464efc901ad71e99) | PPC64 上 BPF JIT 实现, 以加快 PPC64 上的数据包过滤, 其灵感来自 Eric Dumazet 的 x86-64 版本. | v1 ☐☑✓ 3.1-rc1 | [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0ca87f05ba8bdc6791c14878464efc901ad71e99) |
| 2012/04/15 | Eric Dumazet <eric.dumazet@gmail.com> | [net: filter: Just In Time compiler for sparc](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2809a2087cc44b55e4377d7b9be3f7f5d2569091) | 为 sparc 架构实现 BPF JIT. | v2 ☐☑✓ 3.5-rc1 |  [COMMIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2809a2087cc44b55e4377d7b9be3f7f5d2569091) |
| 2012/03/16 | Mircea Gherzan <mgherzan@gmail.com> | [ARM: 7259/3: net: JIT compiler for packet filters](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ddecdfcea0ae891f782ae853771c867ab51024c2) | ARM 支持 BPF JIT. | v1 ☐☑✓ 3.4-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ddecdfcea0ae891f782ae853771c867ab51024c2) |
| 2012/07/31 | Martin Schwidefsky <schwidefsky@de.ibm.com> | [s390/bpf,jit: BPF Just In Time compiler for s390](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c10302efe569bfd646b4c22df29577a4595b4580) | s390 架构的 BPF JIT 支持. | v1 ☐☑✓ 3.7-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c10302efe569bfd646b4c22df29577a4595b4580) |
| 2012/07/31 | Martin Schwidefsky <schwidefsky@de.ibm.com> | [MIPS: net: Add BPF JIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=3f5fdb4bd193091fd30db52c609c4b304151d656) | MIPS 架构的 BPF JIT 支持. | v1 ☐☑✓ 3.16-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=3f5fdb4bd193091fd30db52c609c4b304151d656) |


### 2.5.2 extended BPF JIT
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2014/05/13 | Alexei Starovoitov <ast@plumgrid.com> | [seccomp: JIT compile seccomp filter](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=8f577cadf7181243d336be9aba40c1bcc02c4c54) | x86_64 支持 eBPF JIT, 同时将 seccomp 转换为使用 eBPF 实现. | v1 ☑✓ 3.16-rc1 | [LORE v10,1/3](https://lore.kernel.org/all/1394660614-4436-1-git-send-email-ast@plumgrid.com) |
| 2014/07/08 | Zi Shen Lim <zlim.lnx@gmail.com> | [arm64: eBPF JIT compiler](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e54bcde3d69d40023ae77727213d14f920eb264a) | ARM64 架构的 eBPF JIT. | v1 ☑✓ 3.18-rc1 | [LORE](https://lore.kernel.org/all/1404803213-10653-1-git-send-email-zlim.lnx@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2](https://lore.kernel.org/all/1495754003-21099-1-git-send-email-illusionist.neo@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE 00/14](https://lore.kernel.org/all/1405708100-13604-1-git-send-email-zlim.lnx@gmail.com) |
| 2015/04/01 | Michael Holzheu <holzheu@linux.vnet.ibm.com> | [s390/bpf: Add s390x eBPF JIT compiler backend](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=054623105728b06852f077299e2bf1bf3d5f2b0b) | s390 支持 eBPF JIT. | v1 ☐☑✓ 4.1-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=054623105728b06852f077299e2bf1bf3d5f2b0b) |
| 2016/05/13 | Daniel Borkmann <daniel@iogearbox.net> | [bpf: add generic constant blinding for use in jits](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d93a47f735f3455a896e46b18d0ac26fa19639e6) | eBPF 增加了 "Constant blinding", 这个机制本质上会对 program 里面的常量值同一个随机数进行特定的 OR 操作, 这个操作在运行时每次用到这个常量值的时候都会进行一次, 可以避免攻击者把 BPF 代码假扮成常量值来进行攻击. 其中 [commit 6077776b5908 ("bpf: split HAVE_BPF_JIT into cBPF and eBPF variant")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=6077776b5908e0493a3946f7d3bc63871b201e87) 将原来 HAVE_BPF_JIT 细化成了 HAVE_CBPF_JIT 和 HAVE_EBPF_JIT. 而当前支持 eBPF JIT 的架构只有 x86_64, ARM64, s390, 只支持 cBPF JIT 的架构有 X86(_32), ARM, MIPS, POWERPC, sparc. | v1 ☐☑✓ 4.7-rc1 | [LORE v1,0/10](https://lore.kernel.org/all/cover.1463158442.git.daniel@iogearbox.net) |
| 2018/05/03 | Wang YanQing <udknight@gmail.com> | [bpf, x86_32: add eBPF JIT compiler for ia32](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=03f5781be2c7b7e728d724ac70ba10799cc710d7) | x86_32 支持 eBPF JIT. | v1 ☐☑✓ 4.18-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=03f5781be2c7b7e728d724ac70ba10799cc710d7) |
| 2016/06/22 | Naveen N. Rao <naveen.n.rao@linux.vnet.ibm.com> | [eBPF JIT for PPC64](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=156d0e290e969caba25f1851c52417c14d141b24) | PPC64 支持 eBPF JIT. | v2 ☐☑✓ 4.8-rc1 | [LORE v2,0/7](https://lore.kernel.org/all/cover.1466612260.git.naveen.n.rao@linux.vnet.ibm.com) |
| 2017/08/22 | Shubham Bansal <illusionist.neo@gmail.com> | [arm: eBPF JIT compiler](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=39c13c204bb1150d401e27d41a9d8b332be47c49) | ARM 支持 eBPF JIT. | v1 ☐☑✓ 4.14-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=39c13c204bb1150d401e27d41a9d8b332be47c49) |
| 2019/02/05 | bjorn.topel@gmail.com <bjorn.topel@gmail.com> | [Add RISC-V (RV64G) BPF JIT](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e2c6f50e48849298bed694de03cceb537d95cdc4) | RISC-V 64 支持 eBPF JIT. | v2 ☐☑✓ 5.1-rc1 | [LORE v2,0/4](https://lore.kernel.org/all/20190205124125.5553-1-bjorn.topel@gmail.com) |
| 2020/03/04 | Luke Nelson <lukenels@cs.washington.edu> | [eBPF JIT for RV32G](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=dad737c926b5b286b9c91abbda66811a8a6618e9) | RISC-V 32 支持 eBPF JIT. | v5 ☐☑✓ 5.7-rc1 | [LORE v5,0/4](https://lore.kernel.org/all/20200305050207.4159-1-luke.r.nels@gmail.com) |
| 2021/03/22 | Christophe Leroy <christophe.leroy@csgroup.eu> | [Implement EBPF on powerpc32](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=40272035e1d0edcd515ad45be297c4cce044536d) | PPC32 架构的 eBPF JIT. | v2 ☐☑✓5.13-rc1 | [LORE v2,0/8](https://lore.kernel.org/all/cover.1616430991.git.christophe.leroy@csgroup.eu) |
| 2021/10/05 | Tony Ambardar <tony.ambardar@gmail.com> | [MIPS: eBPF: refactor code, add MIPS32 JIT](https://lore.kernel.org/all/cover.1633392335.git.Tony.Ambardar@gmail.com) | MIPS32 架构的 eBPF JIT | v2 ☐☑✓ |[LORE v2,0/16](https://lore.kernel.org/all/cover.1633392335.git.Tony.Ambardar@gmail.com) |
| 2021/10/05 | Johan Almbladh <johan.almbladh@anyfinetworks.com> | [A new eBPF JIT implementation for MIPS](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=ebcbacfa50ecd7a828f40fefbb58d641f265da0d) | 用于 MIPS I-V 和 MIPS32/64 r1-r6 的 eBPF JIT 的实现. 使用了与其他 eBPF JIT 相同的架构. | v1 ☐☑✓ 5.16-rc1 | [LORE v1,0/7](https://lore.kernel.org/all/20211005165408.2305108-1-johan.almbladh@anyfinetworks.com) |
| 2020/06/29 | Tobias Klauser <tklauser@distanz.ch> | [Factor common x86 JIT code](https://lore.kernel.org/all/20200629093336.20963-1-tklauser@distanz.ch) | TODO | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20200629093336.20963-1-tklauser@distanz.ch) |
| 2022/09/01 | Tiezhu Yang <yangtiezhu@loongson.cn> | [Add BPF JIT support for LoongArch](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=5dc615520c4dfb358245680f1904bad61116648e) | 龙芯支持 eBPF JIT. | v3 ☐☑✓ 6.1-rc1 | [LORE v3,0/4](https://lore.kernel.org/all/1661999249-10258-1-git-send-email-yangtiezhu@loongson.cn) |


# 3 BPF Core
-------

## 3.1 eBPF support
-------

v3.15 对 BPF 进行了升级扩展, 参见 [BPF updates](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9a985cdc5ccb0d557720221d01bd70c19f04bb8c).

eBPF 已经是一个独立的模块了, 因此后来 3.18 直接将 eBPF 从 NET 子系统中分离出来. 参见 [bpf: split eBPF out of NET](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f89b7755f517cdbb755d7543eef986ee9d54e654).

[BPF 数据传递的桥梁 ——BPF Map(一)](https://blog.csdn.net/alex_yangchuansheng/article/details/108332511)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2014/03/28 | Daniel Borkmann <dborkman@redhat.com> | [BPF updates](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9a985cdc5ccb0d557720221d01bd70c19f04bb8c) | 支持 eBPF | v1 ☑✓ 3.15-rc1 | [LORE 0/9](https://lore.kernel.org/netdev/1395404418-25376-1-git-send-email-dborkman@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/9](https:/lhttps://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=99c55f7d47c0dc6fc64729f37bf435abf43f4c60ore.kernel.org/netdev/1395749457-16178-1-git-send-email-dborkman@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v3,0/9](https://lore.kernel.org/netdev/1395867970-1338-1-git-send-email-dborkman@redhat.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4,0/9](https://lore.kernel.org/netdev/1396029506-16776-1-git-send-email-dborkman@redhat.com) |
| 2014/09/04 | Alexei Starovoitov <ast@plumgrid.com> | [net: filter: add"load 64-bit immediate"eBPF instruction](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=daedfb22451dd02b35c0549566cbb7cc06bdd53b) | 添加 BPF_LD_IMM64 指令, 将 64 位立即数加载到寄存器中. | v1 ☑✓ 3.18-rc1 | [LORE 0/2](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=daedfb22451dd02b35c0549566cbb7cc06bdd53b) |
| 2014/10/23 | Alexei Starovoitov <ast@plumgrid.com> | [bpf: split eBPF out of NET](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f89b7755f517cdbb755d7543eef986ee9d54e654) | TODO | v1 ☐☑✓ 3.18-rc3 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f89b7755f517cdbb755d7543eef986ee9d54e654) |

随后 Alexei Starovoitov 实现了 v3.18 [BPF syscall, maps, verifier, samples, llvm](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=3c731eba48e1b0650decfc91a839b80f0e05ce8f). 这是一组比较大的补丁, 最终在 v3.18, v3.19 分批合入.


v3.18 合入了 bpf 系统调用 [bpf: introduce BPF syscall and maps](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=99c55f7d47c0dc6fc64729f37bf435abf43f4c60), 并为 eBPF 程序设计了 Verifier [bpf: verifier (add verifier core)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=17a5267067f3c372fec9ffb798d6eaba6b5e6a4c).

紧接着就增加了一些 sample 样例程序供大家参考. [BPF samples](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fbe3310840c65f3cf97dd90d23e177d061c376f2)

v3.19 支持了 MAP, [BPF maps](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9a9f9dd7c4653daf3f183f35c9a44d97ce9a91f1).


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2014/08/13 | Alexei Starovoitov <ast@plumgrid.com> | [BPF syscall, maps, verifier, samples, llvm](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=3c731eba48e1b0650decfc91a839b80f0e05ce8f) | NA | v1 ☑✓ 3.18-rc1 | [LORE v4,00/26](https://lore.kernel.org/netdev/1407916658-8731-1-git-send-email-ast@plumgrid.com)<br>*-*-*-*-*-*-*-* <br>[LORE v11,00/12](https://lkml.kernel.org/netdev/1410325808-3657-1-git-send-email-ast@plumgrid.com) |
| 2014/08/13 | Alexei Starovoitov <ast@plumgrid.com> | [BPF samples](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fbe3310840c65f3cf97dd90d23e177d061c376f2) | NA | v1 ☑✓ 3.18-rc1 | [LORE v4,00/26](https://lore.kernel.org/netdev/1407916658-8731-1-git-send-email-ast@plumgrid.com) |
| 2014/08/13 | Alexei Starovoitov <ast@plumgrid.com> | [BPF maps](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9a9f9dd7c4653daf3f183f35c9a44d97ce9a91f1) | NA | v1 ☑✓ 3.19-rc1 | [LORE v4,00/26](https://lore.kernel.org/netdev/1407916658-8731-1-git-send-email-ast@plumgrid.com) |


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/03/31 | Cong Wang <xiyou.wangcong@gmail.com> | [bpf: introduce bpf timer](https://lore.kernel.org/all/20210401042635.19768-1-xiyou.wangcong@gmail.com) | TODO | v1 ☐☑✓ | [LORE](https://lore.kernel.org/all/20210401042635.19768-1-xiyou.wangcong@gmail.com) |
| 2021/11/02 | Joe Burton <jevburton.kernel@gmail.com> | [Introduce BPF map tracing capability](https://lore.kernel.org/all/20211102021432.2807760-1-jevburton.kernel@gmail.com) | TODO | v3 ☐☑✓ | [LORE v3,0/3](https://lore.kernel.org/all/20211102021432.2807760-1-jevburton.kernel@gmail.com) |
| 2022/05/18 | Benjamin Tissoires <benjamin.tissoires@redhat.com> | [Introduce eBPF support for HID devices](https://lore.kernel.org/all/20220518205924.399291-1-benjamin.tissoires@redhat.com) | TODO | v5 ☐☑✓ | [LORE v5,0/17](https://lore.kernel.org/all/20220518205924.399291-1-benjamin.tissoires@redhat.com) |


## 3.2 BPF_CMD
-------

[commit 3007098494be ("cgroup: add support for eBPF programs")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3007098494bec614fb55dee7bc0410bb7db5ad18)


[commit f4324551489e ("bpf: add BPF_PROG_ATTACH and BPF_PROG_DETACH commands")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f4324551489e8781d838f941b7aee4208e52e8bf)

## 3.3 BPF_PROG_TYPE AND BPF_ATTACH_TYPE
-------

```cpp
git grep -W 'bpf_prog_type {' include/uapi/linux/bpf.h
```

[BPF 程序类型及其原理 s](https://blog.csdn.net/weixin_41036447/article/details/107817340)

### 3.3.1 SOCKET
-------


| enum bpf_prog_type | VERSION | COMMIT |
|:------------------:|:-------:|:------:|
| BPF_PROG_TYPE_SOCKET_FILTER |


| enum bpf_prog_type | enum bpf_attach_type | VERSION | COMMIT |
|:------------------:|:--------------------:|:-------:|:------:|
| BPF_PROG_TYPE_SOCKET_FILTER | NA | v3.19 | [allow eBPF programs to be attached to sockets](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fbe3310840c65f3cf97dd90d23e177d061c376f2) |
| BPF_PROG_TYPE_CGROUP_SKB | BPF_CGROUP_INET_INGRESS<br>BPF_CGROUP_INET_EGRESS | v4.10 | [bpf: add new prog type for cgroup socket filtering](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=0e33661de493db325435d565a4a722120ae4cbf3) |
| BPF_PROG_TYPE_CGROUP_SOCK | BPF_CGROUP_INET_SOCK_CREATE | v4.10 | [bpf: Add new cgroup attach type to enable sock modifications](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=61023658760032e97869b07d54be9681d2529e77) |
| BPF_PROG_TYPE_CGROUP_SOCK_ADDR | BPF_CGROUP_INET4_BIND<br>BPF_CGROUP_INET6_BIND | v4.17 | [bpf: Hooks for sys_bind](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4fbac77d2d092b475dda9eea66da674369665427) |
| BPF_PROG_TYPE_CGROUP_SOCK_ADDR | BPF_CGROUP_INET4_CONNECT<br>BPF_CGROUP_INET4_CONNECT | v4.17 | [bpf: Hooks for sys_connect](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d74bad4e74ee373787a9ae24197c17b7cdc428d5) |
| BPF_PROG_TYPE_CGROUP_SOCK | BPF_CGROUP_INET4_POST_BIND<br>BPF_CGROUP_INET6_POST_BIND | v4.17 | [bpf: Post-hooks for sys_bind](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=aac3fc320d9404f2665a8b1249dc3170d5fa3caf) |
| BPF_PROG_TYPE_CGROUP_SOCK_ADDR | BPF_CGROUP_UDP4_SENDMSG<br>BPF_CGROUP_UDP6_SENDMSG | v4.18 | [bpf: Hooks for sys_sendmsg](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1cedee13d25ab118d325f95588c1a084e9317229) |
| BPF_PROG_TYPE_CGROUP_SOCK_ADDR | BPF_CGROUP_UDP4_RECVMSG<br>BPF_CGROUP_UDP6_RECVMSG | v5.6 | [bpf: fix unconnected udp hooks](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=983695fa676568fc0fe5ddd995c7267aabc24632) |
| BPF_PROG_TYPE_CGROUP_SOCK_ADDR | BPF_CGROUP_INET4_GETPEERNAME<br>BPF_CGROUP_INET6_GETPEERNAME<br>BPF_CGROUP_INET4_GETSOCKNAME<br>BPF_CGROUP_INET6_GETSOCKNAME | v5.8 | [bpf: Add get{peer, sock}name attach types for sock_addr](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1b66d253610c7f8f257103808a9460223a087469) |
| BPF_PROG_TYPE_CGROUP_SOCK | BPF_CGROUP_INET_SOCK_RELEASE | v5.9 | [bpf: Add BPF_CGROUP_INET_SOCK_RELEASE hook](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f5836749c9c04a10decd2742845ad4870965fdef) |


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2014/11/26 | Alexei Starovoitov <ast@plumgrid.com> | [allow eBPF programs to be attached to sockets](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=fbe3310840c65f3cf97dd90d23e177d061c376f2) | 引入可以通过 setsockopt() 附加到套接字的 BPF_PROG_TYPE_SOCKET_FILTER 类型的 eBPF 程序. 允许这些程序通过查找 / 更新 / 删除助手访问 MAPs.  | v1 ☑✓ 3.19-rc1 | [LORE v1,0/6](https://lore.kernel.org/all/1417066951-1999-1-git-send-email-ast@plumgrid.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/6](https://lore.kernel.org/lkml/1417475199-15950-1-git-send-email-ast@plumgrid.com) |
| 2016/11/23 | Daniel Mack <daniel@zonque.org> | [Add eBPF hooks for cgroups](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d8c5b17f2bc0de09fbbfa14d90e8168163a579e7) | eBPF 程序支持 attach 到 cgroup | v9 ☑✓ 4.10-rc1 | [LORE v9,0/6](https://lore.kernel.org/all/1479916350-28462-1-git-send-email-daniel@zonque.org) |
| 2016/12/01 | David Ahern <dsa@cumulusnetworks.com> | [net: Add bpf support for sockets](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=554ae6e792ef38020b80b4d5127c51d510c0918f) | eBPF 程序支持 attach 到 socket | v7 ☑✓ 4.10-rc1 | [LORE v7,0/6](https://lore.kernel.org/all/1480610888-31082-1-git-send-email-dsa@cumulusnetworks.com) |
| 2018/03/13 | Alexei Starovoitov <ast@kernel.org> | [bpf: introduce cgroup-bpf bind, connect, post-bind hooks](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=aac3fc320d9404f2665a8b1249dc3170d5fa3caf) | eBPF 程序支持 attach 到 syscal bind 和 conntect. | v1 ☑✓ 4.17-rc1 | [LORE v1,0/6](https://lore.kernel.org/all/20180314033934.3502167-1-ast@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/9](https://lore.kernel.org/netdev/20180328034140.291484-1-ast@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v3](https://lore.kernel.org/netdev/20180330220808.763556-1-ast@kernel.org) |
| 2018/05/25 | Andrey Ignatov <rdna@fb.com> | [bpf: Hooks for sys_sendmsg](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=04b6ab731209eac1e130fa00281a29278eca2f57) | eBPF 程序支持附加到 sendmsg SYSCALL. | v4 ☑✓ 4.18-rc1 | [LORE v4,0/6](https://lore.kernel.org/all/cover.1527263217.git.rdna@fb.com) |
| 2019/06/07 | Daniel Borkmann <daniel@iogearbox.net> | [Fix unconnected bpf cgroup hooks](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=b714560f7b38de9f03b8670890ba130d4cc5604e) | eBPF 程序支持附加到 recvmsg SYSCALL. | v3 ☑✓ 5.2-rc6 | [LORE v3,0/6](https://lore.kernel.org/all/20190606234902.4300-1-daniel@iogearbox.net) |
| 2020/05/19 | Daniel Borkmann <daniel@iogearbox.net> | [Add get{peer,sock}name cgroup attach types](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=566fc3f5d1c641b510ec487cf274a047f8a1e849) | 为 BPF sock_addr 程序添加 get {peer,sock} name cgroup 类型, 以便从两个调用以及 libbpf 和 bpftool 支持以及自检中重写 sockaddr structs. | v2 ☑✓ 5.8-rc1 | [LORE v2,0/4](https://lore.kernel.org/all/cover.1589841594.git.daniel@iogearbox.net) |
| 2020/07/06 | Stanislav Fomichev <sdf@google.com> | [bpf: add BPF_CGROUP_INET_SOCK_RELEASE hook](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=65ffd797861a44ff97081de1db01e4aef716ed46) | 添加 BPF_CGROUP_INET_SOCK_RELEASE. | v4 ☑✓ 5.9-rc1 | [LORE v4,0/4](https://lore.kernel.org/all/20200706230128.4073544-1-sdf@google.com) |

### 3.3.2 CGROUP
-------


[commit 3007098494be ("cgroup: add support for eBPF programs")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=3007098494bec614fb55dee7bc0410bb7db5ad18)


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:---:|:----------:|:----:|
| 2016/11/23 | Daniel Mack <daniel@zonque.org> | [Add eBPF hooks for cgroups](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=d8c5b17f2bc0de09fbbfa14d90e8168163a579e7) | TODO | v9 ☑✓ 4.10-rc1 | [LORE v9,0/6](https://lore.kernel.org/all/1479916350-28462-1-git-send-email-daniel@zonque.org) |


### 3.3.3 BPF_PROG_TYPE_KPROBE
-------

| enum bpf_prog_type | enum bpf_attach_type | VERSION | COMMIT |
|:------------------:|:--------------------:|:-------:|:------:|
| BPF_PROG_TYPE_KPROBE | NA | v4.1 | [commit 2541517c32be ("tracing, perf: Implement BPF programs attached to kprobes")](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=2541517c32be2531e0da59dfd7efc1ce844644f5) |


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2015/03/25 | Alexei Starovoitov <ast@plumgrid.com> | [tracing: attach eBPF programs to kprobes](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9811e35359d4b18baf5bb603b225e957255b9c46) | NA | v11 ☑ 4.1-rc1 | [PatchWork v11 0/9](https://lore.kernel.org/lkml/1427312966-8434-1-git-send-email-ast@plumgrid.com) |


### 3.3.4 Traffic Control Subsystem
-------

[eBPF: Traffic Control Subsystem](https://blog.csdn.net/weixin_43705457/article/details/123388130)

[eBPF 的发展历史和核心设计](https://blog.csdn.net/alex_yangchuansheng/article/details/114558126)

[linux-next: manual merge of the tip tree with the net-next tree](https://lore.kernel.org/lkml/20150407171148.7a41ee90@canb.auug.org.au)

| enum bpf_prog_type | enum bpf_attach_type | VERSION | COMMIT |
|:------------------:|:--------------------:|:-------:|:------:|
| BPF_PROG_TYPE_SCHED_CLS | NA | v4.1 | [ebpf: add sched_cls_type and map it to sk_filter's verifier ops](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=96be4325f443dbbfeb37d2a157675ac0736531a1) |
| BPF_PROG_TYPE_SCHED_ACT | NA | v4.1 | [ebpf: add sched_act_type and map it to sk_filter's verifier ops](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=96be4325f443dbbfeb37d2a157675ac0736531a1) |


| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2015/02/27 | Daniel Borkmann <daniel@iogearbox.net> | [eBPF support for cls_bpf](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e2e9b6541dd4b31848079da80fe2253daaafb549) | TODO | v1 ☑✓ 4.1-rc1 | [LORE v1,0/10](https://lore.kernel.org/all/cover.1425040939.git.daniel@iogearbox.net) |


### 3.3.5 tracepoint
-------

#### 3.3.5.1 tracepoint
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2016/04/07 | Alexei Starovoitov <ast@fb.com> | [allow bpf attach to tracepoints](https://lore.kernel.org/patchwork/cover/665728) | BPF 支持 tracepoint 跟踪 | v2 ☑ 4.7-rc1 | [PatchWork v2 00/10](https://lore.kernel.org/patchwork/cover/665728) |

#### 3.3.5.2 raw tracepoint
-------

raw_tracepoint 相比 tracepoint

1.  效率更高, 跳过了参数的处理, 不必向 tracepoint 那样根据 format 解析了所有字段, 节约了不必要的开销.

2.  自由度更大, 提供了对参数的原始访问, 可以直接操作传入 tracepoint 时的指针, 获取某个成员.

3.  可以获得原始结构体指针. 用于做 key 比较方便, tracepoint 只能拿到里面的各个字段, 有些时候没法做 key.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/02/28 | Alexei Starovoitov <ast@kernel.org> | [bpf, tracing: introduce bpf raw tracepoints](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=4662a4e53890badf4da17e441606a2885f29d56d) | BPF 支持 raw tracepoint 跟踪, 提供了对 tracepoint 参数的访问. | v1 ☐☑✓ | [LORE v1,0/5](https://lore.kernel.org/all/20180301041957.399230-1-ast@kernel.org)<br>*-*-*-*-*-*-*-* <br>[LORE v4,00/10](https://lore.kernel.org/all/20180323003907.3113756-1-ast@fb.com) |
| 2018/12/13 | Matt Mullins <mmullins@fb.com> | [bpf: support raw tracepoints in modules](https://lore.kernel.org/patchwork/cover/1024475) | NA | v2 ☑ 5.0-rc1 | [PatchWork v2](https://lore.kernel.org/patchwork/cover/1024475) |
| 2019/04/26 | Matt Mullins <mmullins@fb.com> | [writable contexts for bpf raw tracepoints](https://lore.kernel.org/patchwork/cover/1067299) | 引入 BPF_PROG_TYPE_RAW_TRACEPOINT_WRITABLE, 允许 bpfs 程序修改 tracepoint 中的变量. | v5 ☑ 4.17-rc1 | [PatchWork v5](https://lore.kernel.org/patchwork/cover/1067299) |


## 3.4 Helper Function
-------

| enum bpf_func_id | VERSION | COMMIT |
|:----------------:|:-------:|:------:|
| NA | NA | NA |


| 2016/10/27 | Thomas Graf <tgraf@suug.ch> | [bpf: Print function name in addition to function id](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ebb676daa1a340ccef25eb769aefc09b79c01f8a) | TODO | v1 ☐☑✓ 4.10-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ebb676daa1a340ccef25eb769aefc09b79c01f8a) |
| 2018/04/25 | Quentin Monnet <quentin.monnet@netronome.com> | [bpf: add documentation for eBPF helpers](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=9cde0c8892b9bba07b27f2317d8d02707dc6ff92) | 提供 `scripts/bpf_helpers_doc.py` 脚本根据 `include/uapi/linux/bpf.h` 生成文档 `bpf-helpers.rst`. | v1 ☑✓ 4.18-rc1 | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=9cde0c8892b9) |
| 2022/05/29 | Lorenzo Bianconi <lorenzo@kernel.org> | [introduce support for XDP programs in CPUMAP](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=cover.1594734381.git.lorenzo@kernel.org) | TODO | v ☐☑✓ | [LORE v7,0/9](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=cover.1594734381.git.lorenzo@kernel.org) |
| 2022/05/29 | Tycho Andersen <tycho.andersen@canonical.com> | [c/r of seccomp filters via underlying eBPF](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1441382664-17437-1-git-send-email-tycho.andersen@canonical.com) | TODO | v ☐☑✓ | [LORE](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=1441382664-17437-1-git-send-email-tycho.andersen@canonical.com) |



### 3.1.1 dump kernel data
-------

[Dumping kernel data structures with BPF](https://lwn.net/Articles/818714)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/03/01 | Jan H. Schönherr | [bpf: implement bpf based dumping of kernel data structures](http://patches.linaro.org/cover/221148) | BPF 支持 raw tracepoint 跟踪, 提供了对 tracepoint 参数的访问. | v1 ☐ | [PatchWork v2](http://patches.linaro.org/cover/221148) |
| 2020/06/23 | Alan Maguire <alan.maguire@oracle.com> | [bpf, printk: add BTF-based type printing](https://lore.kernel.org/patchwork/cover/1261591/) | 在 BPF 程序中通过 printk 打印内核结构体   | v3 ☐ | [PatchWork v3,bpf-next,0/8](https://lore.kernel.org/patchwork/cover/1261591/) |
| 2020/08/06 | Alan Maguire <alan.maguire@oracle.com> | [bpf: add bpf-based bpf_trace_printk()-like support](https://lore.kernel.org/patchwork/cover/1285027) | 在 BPF 程序中通过 trace_printk 打印信息   | RFC ☐ | [PatchWork RFC,bpf-next,0/4](https://lore.kernel.org/patchwork/cover/1285027) |
| 2020/09/28 | Alan Maguire <alan.maguire@oracle.com> | [bpf: add helpers to support BTF-based kernel data display](https://lore.kernel.org/patchwork/cover/1312835) | 在 BPF 程序中通过 printk 打印内核结构体   | v7 ☐ | [PatchWork v7,bpf-next,0/8](https://lore.kernel.org/patchwork/cover/1312835) |
| 2021/06/19 | Alan Maguire <alan.maguire@oracle.com> | [libbpf: BTF dumper support for typed data](https://lore.kernel.org/patchwork/cover/1448989) | 引入 btf_dump__dump_type_data, 可以用来输出结构体的信息. | v5 ☐ | [PatchWork v5,bpf-next,0/3](https://lore.kernel.org/patchwork/cover/1448989) |


## 3.1 5 function call
-------

[Calling kernel functions from BPF](https://lwn.net/Articles/856005)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/03/01 | Jan H. Schönherr | [bpf: Support calling kernel function](https://patchwork.kernel.org/project/netdevbpf/cover/20210325015124.1543397-1-kafai@fb.com) | BPF 支持直接调用内核函数 | v2 ☑ 5.13-rc1 | [PatchWork v2,bpf-next,00/14](http://patches.linaro.org/cover/221148) |
| 2021/08/30 | Kumar Kartikeya Dwivedi <memxor@gmail.com> | [Support kernel module function calls from eBPF](https://patchwork.kernel.org/project/netdevbpf/cover/20210830173424.1385796-1-memxor@gmail.com) | 这组补丁允许 BPF 程序调用内核模块的函数, 并修改验证器逻辑以允许无效的内核函数调用, 只要它们作为死代码消除的一部分被修剪. 这样做是为了为 BPF 对象提供更好的运行时可移植性, 它可以有条件地禁用稍后被验证器修剪的部分代码 (例如 const volatile vars, kconfig 选项). libbpf 的修改与内核的修改一起进行, 以支持模块函数调用.
它还将 TCP 拥塞控制对象转换为使用模块 kfunc 支持, 而不是依赖于 IS_BUILTIN ifdef. | v1 ☐ | [PatchWork bpf-next,RFC,v1,0/8](https://patchwork.kernel.org/project/netdevbpf/cover/20210830173424.1385796-1-memxor@gmail.com) |


## 3.2 printk
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/03/01 | Jan H. Schönherr | [bpf: implement variadic printk helper](https://patchwork.kernel.org/project/netdevbpf/cover/20210913035609.160722-1-davemarchevsky@fb.com) | 这组补丁引入了一个新的接口 bpf_trace_vprintk(), 它的功能与 bpf_trace_printk() 类似, 但通过伪 vararg u64 的数组支持 > 3 个参数, 使用与 bpf_trace_printk() 相同的机制写入 `/sys/kernel/debug/tracing/trace_pipe`, 帮助程序的功能是在 libbpf 问题跟踪程序中请求的. libbpf 的 bpf_printk() 宏被修改为在传递 > 3 个变量时使用 bpf_trace_vprintk(), 否则保留之前的行为, 使用 bpf_trace_printk().<br>
在实现 bpf_seq_printf() 和 bpf_snprintf() 期间添加的 helper 函数和宏为 bpf_trace_vprintk() 完成了大部分繁重的工作. 用例很简单: 为 BPF 开发人员提供一个更强大的 printk 将简化 BPF 程序的开发, 特别是在调试和测试期间, 这里往往使用 printk. 这个特性是由 [Andrii 在 libbpf 的 issue 中提出](https://github.com/libbpf/libbpf/issues/315) 的. | v2 ☑ 5.13-rc1 | [PatchWork v5,bpf-next,0/9](https://patchwork.kernel.org/project/netdevbpf/cover/20210913035609.160722-1-davemarchevsky@fb.com) |


## 3.3 MAP
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2018/08/26 | Kumar Kartikeya Dwivedi <memxor@gmail.com> | [Implement file local storage](https://patchwork.kernel.org/project/netdevbpf/cover/20210826133913.627361-1-memxor@gmail.com/) | 本系列实现了 eBPF LSM 程序的文件本地存储映射. 这允许将映射中数据的生存期与打开的文件描述联系起来. 与其他本地存储映射类型一样, 数据的生存期与结构文件实例相关联. 主要用途是:<br>1. 用于将与 eBPF 程序中打开的文件 (而非 fd) 关联的数据绑定在一起, 以便在文件消失时释放数据 (例如, 检查点 checkpoint / 恢复用例 restore usecase).<br>2. 使用 eBPF LSM 在用户空间中实现 [辣椒 (Capsicum) 风格的功能沙盒](https://www.usenix.org/legacy/event/sec10/tech/full_papers/Watson), 使用此机制在文件级别强制执行权限. | v2 ☐ | [PatchWork bpf-next,v2,0/5](https://patchwork.kernel.org/project/netdevbpf/cover/20210826133913.627361-1-memxor@gmail.com) |
| 2022/04/08 | Song Liu <song@kernel.org> | [vmalloc: bpf: introduce VM_ALLOW_HUGE_VMAP](https://patchwork.kernel.org/project/linux-mm/cover/20220408223443.3303509-1-song@kernel.org/) | 630581 | v1 ☐☑ | [LORE v1,0/2](https://lore.kernel.org/r/20220408223443.3303509-1-song@kernel.org) |




## 3.5 JIT
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:-----:|:----:|:----:|:----:|:------------:|:----:|
| 2016/06/22 | Naveen N. Rao <naveen.n.rao@linux.vnet.ibm.com> | [eBPF JIT for PPC64](https://lore.kernel.org/all/cover.1466612260.git.naveen.n.rao@linux.vnet.ibm.com) | cover.1466612260.git.naveen.n.rao@linux.vnet.ibm.com | v2 ☐☑✓ | [LORE v2,0/7](https://lore.kernel.org/all/cover.1466612260.git.naveen.n.rao@linux.vnet.ibm.com) |
| 2017/05/26 | Shubham Bansal <illusionist.neo@gmail.com> | [arm: eBPF JIT compiler](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=39c13c204bb1150d401e27d41a9d8b332be47c49) | 1495754003-21099-1-git-send-email-illusionist.neo@gmail.com | v2 ☑✓ 4.14-rc1 | [LORE](https://lore.kernel.org/all/1495754003-21099-1-git-send-email-illusionist.neo@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v4](https://lore.kernel.org/lkml/1503383553-5650-1-git-send-email-illusionist.neo@gmail.com) |
| 2014/08/26 | Zi Shen Lim <zlim.lnx@gmail.com> | [arm64: eBPF JIT compiler](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=e54bcde3d69d40023ae77727213d14f920eb264a) | 1409112930-25677-1-git-send-email-zlim.lnx@gmail.com | v2 ☐☑✓ | [LORE v1,00/14](https://lore.kernel.org/lkml/1405708100-13604-1-git-send-email-zlim.lnx@gmail.com)<br>*-*-*-*-*-*-*-* <br>[LORE v2,0/14](https://lore.kernel.org/all/1409112930-25677-1-git-send-email-zlim.lnx@gmail.com) |


## 3.6 10 BPF allocator
-------

[The BPF allocator runs into trouble](https://lwn.net/Articles/892743)

[Toward signed BPF programs](https://lwn.net/Articles/853489)


# 4 子系统支持
-------

[Innovative HID-BPF Expected To Land In Linux 6.2](https://www.phoronix.com/news/Linux-6.2-HID-BPF-Expected)

[](https://www.phoronix.com/news/Linux-eBPF-For-HID)

# 5 网络场景
-------


# 6 安全场景
-------


# 7 内核可编程
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/09/09 | Kumar Kartikeya Dwivedi <memxor@gmail.com> | [bmc-cache](https://www.phoronix.com/scan.php?page=news_item&px=Orange-BPF-Memory-Cache-BMC) | 法国电信巨头奥兰治公司 (Orange) 已经发布了 "BMC", 作为 (e)BPF 内存缓存缓存, 提供了一个专注于 Linux 内核中模棱两两的用法的缓存. Orange 的开源 BPF 内存缓存允许在标准网络堆栈之前处理模组请求, 据说防撞安全, 并且此模块不需要其他内核模块. 此外, 模组用户空间软件本身可以在 BMC 上运行未经修改的软件. 这种内核 eBPF 缓存可将 Memcached 的吞吐量提高 18 倍, 而没有此内核缓存的夹层则提高了 18 倍. 参考 [GitHub](https://github.com/Orange-OpenSource/bmc-cache) | v2 ☐ | [github Orange-OpenSource/bmc-cache](https://github.com/Orange-OpenSource/bmc-cache) |
| 2021/09/15 | Roman Gushchin <guro@fb.com> | [Scheduler BPF](https://www.phoronix.com/scan.php?page=news_item&px=Orange-BPF-Memory-Cache-BMC) | NA | RFC ☐ | [PatchWork rfc,0/6](https://lore.kernel.org/bpf/CA+khW7i460ey-UFzpMSJ8AP9QeD8ufa4FzLA4PQckNP00ShQSw@mail.gmail.com)<br>*-*-*-*-*-*-*-* <br>[LPC 2021](https://linuxplumbersconf.org/event/11/contributions/954)<br>*-*-*-*-*-*-*-* <br>[LKML](https://lkml.org/lkml/2021/9/16/1049), [LWN](https://lwn.net/Articles/869433) |


# 8 用户态工具
-------

## 8.1 libbpf-tools
-------

[[BPF CO-RE clarification] Use CO-RE on older kernel versions.](https://www.spinics.net/lists/bpf/msg32659.html)

[Help using libbpf with kernel 4.14](https://www.spinics.net/lists/bpf/msg27267.html)

[How to use libbpf-tools with ubuntu 18.04 4.15 kernel #3232](https://github.com/iovisor/bcc/issues/3232)

[How to use libbpf-tools with ubuntu 18.04 4.15 kernel](https://www.gitmemory.com/issue/iovisor/bcc/3232/761744003)


## 8.2 perf
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2015/10/14 | Wang Nan <wangnan0@huawei.com> | [perf tools: filtering events using eBPF programs](https://lore.kernel.org/all/1444826502-49291-1-git-send-email-wangnan0@huawei.com) | TODO | v1 ☐☑✓ | [LORE v1,0/31](https://lore.kernel.org/all/1444826502-49291-1-git-send-email-wangnan0@huawei.com) |
| 2023/03/14 | Namhyung Kim <namhyung@kernel.org> | [perf record: Implement BPF sample filter (v5)](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/log/?id=c46bf3bd00167e09729f884dd479b0a8d1a63f95) | 社区有诉求基于 sample 数据进行更复杂的性能事件示例过滤. 最近内核增加了 BPF 程序可以访问性能样本数据, 这是用户空间部分启用这样的过滤.<br>perf record 具有 `--filter` 选项, 用于在命令行中对最后一个指定的事件设置筛选器. 到目前为止, 它只适用于跟踪点和英特尔 PT 事件. 这个补丁集将其扩展为使用 BPF, 以便为任何事件启用通用样本过滤器. 添加了一个新的过滤器表达式解析器 (使用 flex/bison) 来处理过滤器字符串. 当前它只接受用逗号分隔的非常简单的表达式. | v5 ☑✓ 6.4-rc1 | [LORE v5,0/10](https://lore.kernel.org/all/20230314234237.3008956-1-namhyung@kernel.org) |


## 8.3 coolbpf(surtrace & pyLCC)
-------

[龙蜥社区开源 coolbpf, BPF 程序开发效率提升百倍 | 龙蜥技术](https://openanolis.cn/blog/detail/601601711630939267) coolbpf 项目, 以 CO-RE(Compile Once-Run Everywhere) 为基础实现, 保留了资源占用低、可移植性强等优点, 还融合了 BCC 动态编译的特性, 适合在生产环境批量部署所开发的应用.

coolbpf 开创了一个新的思路, 利用远程编译的思想, 把用户的 BPF 程序推送到远端的服务器并返回给用户. o 或. so, 提供高级语言如 `Python/Rust/Go/C` 等进行加载, 然后在全量内核版本安全运行. 用户只需专注自己的功能开发, 不用关心底层库 (如 LLVM、python 等) 安装、环境搭建, 给广大 BPF 爱好者提供一种新的探索和实践.

coolbpf 本质上是一个 eBPF 开发平台, 通过 pylcc、rlcc、golcc 和 clcc 等实现了高级语言的支持, 同时支持远程编译.

| feature | 描述 |
|:-------:|:----:|
| clcc  | 基于 C 的 LCC |
| plcc  | 基于 Python 的 LCC |
| rlcc  | 基于 Rust 的 LCC   |
| golcc | 基于 Rust 的 LCC   |
| glcc(generic LCC | 高版本特性移植到低版本, (g 代表 generic) 是通过将高版本的 BPF 特性移植到低版本, 通过 kernel module 的方式在低版本上运行. 驱动源代码位于 [`lcc/glcc`](https://gitee.com/anolis/coolbpf/tree/master/lcc/glcc) |

glcc 则实现了 eBPF 驱动和 libbpf 的支持, 允许 eBPF 程序无需修改即可在低版本内核上运行.

1.  通过 eBPF 驱动将 BPF SYSCALL 转换为 IOCTL 系统调用, 为低版本内核提供 eBPF 的能力;

2.  通过 libbpf 则屏蔽了不同内核版本的差异.

[微信公众号 - Linux 内核之旅 -- 内核 trace 三板斧 - surtrace-cmd](https://mp.weixin.qq.com/s/XanaxrLDwkqTqcfq9B2eOw)

[龙蜥开源内核追踪利器 Surftrace：协议包解析效率提升 10 倍！ | 龙蜥技术](https://mp.weixin.qq.com/s/o3Q-spZmBbs4Gbhv-3U91g)

[iofsstat：帮你轻松定位 IO 突高, 前因后果一目了然 | 龙蜥技术](https://developer.aliyun.com/article/867067)

[libbpf 编译平台 LCC——eBPF 从入门到享受 | 龙蜥大讲堂第 20 期](https://www.bilibili.com/video/BV1Ar4y1G7R8)

[eBPF 在低版本内核如何跑起来？| 龙蜥大讲堂第 46 期](https://www.bilibili.com/video/BV1Se411u7zD)

## 8.4 eunomia-bpf
-------

[eunomia-bpf: A dynamic loader to run CO-RE eBPF as a service](https://github.com/eunomia-bpf/eunomia-bpf)

[OpenAnolis/eBPF 技术探索 SIG/Eunomia 项目介绍](https://openanolis.cn/sig/ebpfresearch/doc/640013458629853191)


## 8.5 ubpf
--------

[0](https://github.com/iovisor/ubpf)

[1](https://github.com/rlane/ubpf)

[2](https://github.com/p-quic/ubpf)

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/06/17 | Zhang Chen <chen.zhang@intel.com> | [Introduce QEMU userspace ebpf support](https://lore.kernel.org/all/20220617073630.535914-1-chen.zhang@intel.com) | TODO | v1 ☐☑✓ | [LORE v1,0/12](https://lore.kernel.org/all/20220617073630.535914-1-chen.zhang@intel.com) |

## 8.6 BCC
-------

[An introduction to the BPF Compiler Collection](https://lwn.net/Articles/742082)

[bcc/ebpf 安装及示例（2019）](http://arthurchiao.art/blog/bcc-ebpf-tutorial-zh)

## 8.7 continuous profiling
-------

| 博客 | 描述 |
|:---:|:----:|
| [Introduction to Parca - Part 1](https://www.polarsignals.com/blog/posts/2023/01/19/introduction-to-parca-agent) | Parca 是一个基于 eBPF 在线采样工具. |
| [grafana/phlare](https://github.com/grafana/phlare) |


# 9 WASM(WebAssembly)
-------

[WebAssembly 中文网 | Wasm 中文文档](https://www.wasm.com.cn)

## 9.1 Wasmtime
-------

2019 年, 英特尔、Mozilla 和红帽就启动了字节码联盟, 作为一项旨在促进 "无处不在" 运行 WebAssembly, 并将 WASM 的范围扩展到 Web 浏览器之外的倡议. 开发三年后 (2022 年), WebAssembly 运行时 Wasmtime 1.0 发布了. 参见 phoronix 报道 [Wasmtime 1.0 Released - Bytecode Alliance Declares It Production Ready](https://www.phoronix.com/news/Wasmtime-1.0-Released).


Wasmtime 完全开源, 使用 Rust 编程语言, 是的, 并且符合 WASI 标准. Wasmtime 还支持与 C/C++, Python, .NET, Go 和其他编程语言的集成, 同时在 Windows / Linux / macOS 平台上运行等等.



# 10 云原生
-------

[Microsoft and Isovalent partner to bring next generation eBPF dataplane for cloud-native applications in Azure](https://azure.microsoft.com/zh-cn/blog/microsoft-and-isovalent-partner-to-bring-next-generation-ebpf-dataplane-for-cloudnative-applications-in-azure)



# X 学习参考
-------

## X.1 业界工具
-------

| 工具 | 描述 |
|:---:|:----:|
| [nyrahul/ebpf-guidee](https://github.com/nyrahul/ebpf-guide) | eBPF 常见问题, 样例以及工具汇总 |
| [apache/skywalking-rover](https://github.com/apache/skywalking-rover) | |
| [GroundCover](https://github.com/groundcover-com) | [GroundCover](https://www.groundcover.com) 轻松、大规模地监控 K8s 应用 |
| [Exein-io/pulsar](https://github.com/Exein-io/pulsar) | [Tracing the Linux kernel using Exein Pulsar: a 5 Minute Tutorial](https://blog.exein.io/pulsar-tutorial)


## X.2 业界博客
-------

| 博客 | 描述 |
|:---:|:----:|
| [Introduction to Parca - Part 1](https://www.polarsignals.com/blog/posts/2023/01/19/introduction-to-parca-agent) | Parca 是一个基于 eBPF 在线采样工具. |
| [DWARF-based Stack Walking Using eBPF](https://www.polarsignals.com/blog/posts/2022/11/29/profiling-without-frame-pointers) | NA |

## X.3 eBPF 月报
-------


[ebpf 月报 - 2023 年 1 月](https://segmentfault.com/a/1190000043355631)


## X.4 业界论文
-------

| 论文 | 描述 |
|:---:|:----:|
| [Flexible and Low-Overhead System-Call Aggregation using BPF](https://www4.cs.fau.de/Ausarbeitung/MA-I4-2021-12-Gerhorst.pdf)<br>[AnyCall: Fast and Flexible System-Call Aggregation](https://dl.acm.org/doi/10.1145/3477113.3487267) | 通过 eBPF 实现的 ANYCALL, 它使用内核内字节码编译器在内核模式下执行安全检查的用户代码. 这允许多个快速的系统调用与错误检查和处理逻辑交织聚合在一起, 只使用一次用户态 / 内核态转换. 并将其扩展为支持系统调用和用户内存访问. 由于内核已经支持灵活的事件处理和调试, 重用 BPF 来实现系统调用聚合, 这表明软件隔离进程对于现代通用操作系统是可行的. 为了证明将真实的用户应用程序移植到 ANYCALL 既实用又直接, 我移植了两个真实的工具, 并记录了所需的代码更改. 最后, 我评估了 ANYCALL 在具有操作系统级缓解措施的系统上的性能, 以对抗活跃或不活跃的瞬时执行漏洞, 包括例如针对 Meltdown 的内核页表隔离 (KPTI). 在 KPTI 不活跃的系统上, 我演示了在计算受限的实际应用程序中高达 10% 的加速. 另一方面, 当 KPTI 处于活动状态时, 我的评估表明, 使用 ANYCALL 可以使系统调用爆发速度提高 98%, 而实际应用程序的速度提高了 32% 到 40%. 参见 [【欧拉多咖 | OS 每周快讯】2022.12.06~2022.12.12](https://www.chaspark.com/#/hotspots/821172234535870464) |
| [XRP: In-Kernel Storage Functions with eBPF【OSDI’22】](https://www.usenix.org/system/files/osdi22-zhong_1.pdf) | XRP 框架允许应用程序从 NVMe 驱动程序中的 eBPF 探针执行用户定义的存储功能, 如索引查找或聚合, 从而安全地绕过内核的大部分存储堆栈. 为了保持文件系统语义, XRP 将少量内核态转移到其 NVMe 驱动程序探针, 并在探针处调用用户注册的 eBPF 函数. |

<br>

*   本作品 / 博文 ([AderStep - 紫夜阑珊 - 青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚 (gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用 <a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt=" 知识共享许可协议 "style="border-width:0"src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"/></a><a rel="license"href="http://creativecommons.org/licenses/by-nc-sa/4.0/"> 知识共享署名 - 非商业性使用 - 相同方式共享 4.0 国际许可协议 </a> 进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名 [成坚 gatieme](http://blog.csdn.net/gatieme) (包含链接: http://blog.csdn.net/gatieme), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   ** 转载请务必注明出处, 谢谢, 不胜感激 **
<br>
