 ---

title: 工具
date: 2021-06-26 09:40
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

# 1 Cache 一致性
-------


# 2 缓存一致性协议--MESI协议
-------

由于现在一般是多核处理器, 每个处理器都有自己的高速缓存, 那么会导致一些问题:

当某一个数据在多个处于 "运行" 状态的线程中进行读写共享时(例如 ThreadA、ThreadB 和 ThreadC)

1.  第一个问题是多个线程可能在多个独立的 CPU 内核中"同时" 修改数据 A, 导致系统不知应该以哪个数据为准;

2.  第二个问题是由于ThreadA进行数据A的修改后没有即时写会内存, ThreadB和ThreadC也没有即时拿到新的数据A, 导致ThreadB和ThreadC对于修改后的数据不可见.

这就是缓存一致性问题.

为了解决这个问题, 处理器之间需要一种通信机制----缓存一致性协议.


## 2.1 MESI 状态
-------

MESI(Modified-Exclusive-Shared-Invalid) 协议是一种广为使用的缓存一致性协议. MESI 协议对内存数据访问的控制类似于读写锁, 它使得针对同一地址的读内存操作是并发的, 而针对同一地址的写内存操作是独占的.

之所以叫 MESI, 是因为这套方案把一个缓存行(cache line)区分出四种不同的状态标记, 他们分别是 Modified、Exclusive、Shared 和 Invalid. 这四种状态分别具备一定的意义:

| 状态 | 描述 | 监听任务 | 状态转换 |
|:---:|:----:|:------:|:-------:|
| M 修改 (Modified) | 该 Cache line 有效, 但是数据被修改了(和内存中的数据不一致), 并且该数据只存在于本 Cache 中.<br>在未来的某个时刻该数据会被写入到内存中(一般在其他 CPU 要读取该缓存行的内容时. 或者其他 CPU 要修改该缓存对应的内存中的内容时 | 缓存行必须时刻监听所有试图读该缓存行相对就主存的操作, 这种操作必须在缓存将该缓存行写回主存并将状态变成S(共享)状态之前被延迟执行. | 当被写回主存之后, 该缓存行的状态会变成独享(exclusive)状态. |
| E 独享、互斥 (Exclusive) | 该 Cache line 有效, 数据和内存中的数据一致, 并且数据只存在于本Cache中. | 缓存行也必须监听其它缓存读主存中该缓存行的操作, 一旦有这种操作, 该缓存行需要变成S(共享)状态.| 该缓存可以在任何其他 CPU 读取该缓存对应内存中的内容时变成 S 状态. 或者本地处理器写该缓存就会变成 M 状态. |
| S 共享 (Shared) | 该 Cache line 有效, 数据和内存中的数据一致, 数据存在于很多 Cache 中. | 当前缓存行必须监听其它缓存使该缓存行无效或者独享该缓存行的请求, 并将该缓存行变成无效(Invalid). | 当其他 CPU 修改该缓存行对应的内存时会使该缓存行变成 I 状态. |
| I 无效 (Invalid) | 该 Cache line 无效. | 无 | 无 |

## 2.2 请求
-------


首先不同 CPU 之间也是需要沟通的, 这里的沟通是通过在消息总线上传递 message 实现的. 这些在总线上传递的消息有如下几种:

| 请求 | 描述 |
|:---:|:----:|
| Read | 带上数据的物理内存地址发起的读请求消息 |
| Read Response | Read 请求的响应信息, 内部包含了读请求指向的数据 |
| Invalidate | 该消息包含数据的内存物理地址, 意思是要让其他如果持有该数据缓存行的 CPU 直接失效对应的缓存行 |
| Invalidate Acknowledge | CPU 对Invalidate 消息的响应, 目的是告知发起 Invalidate 消息的CPU, 这边已经失效了这个缓存行啦 |
| Read Invalidate | 这个消息其实是 Read 和 Invalidate 的组合消息, 与之对应的响应自然就是一个Read Response 和 一系列的 Invalidate Acknowledge |
| Writeback | 该消息包含一个物理内存地址和数据内容, 目的是把这块数据通过总线写回内存里. |


| 消息 | 类型 | 描述 |
|:---:|:----:|:---:|
| Read | 请求 | 通知其他处理器和内存, 当前 CPU 准备读取某个数据. 该消息内包含待读取数据的内存地址. |
| Read Response | 响应 | Read 请求的响应信息, 该消息内包含了被请求读取的数据. 该消息可能是主内存返回的, 也可能是其他高速缓存嗅探到 Read 消息返回的. |
| Invalidate | 请求 | 通知其他处理器删除(无效)指定内存地址的数据副本(缓存行中的数据). 该消息包含数据的内存物理地址. |
| Invalidate Acknowledge | 响应 | 这是 CPU 对 Invalidate 消息的响应, 接收到 Invalidate 消息的处理器必须回复此消息, 表示已经无效掉了其高速缓存内对应的数据副本. |
| Read Invalidate | 请求 | 这个消息其实是 Read 和 Invalidate 消息组成的复合消息, 主要是用于通知其他处理器当前处理器准备更新一个数据了, 并请求其他处理器删除其高速缓存内对应的数据副本. 接收到该消息的处理器必须回复 Read Response 和 Invalidate Acknowledge 消息. |
| Writeback | 响应 | 消息包含了需要写入内存的数据和其对应的内存地址, 目的是把这块数据通过总线写回内存里. |


## 2.3 状态转换
-------

那么这些请求如何动态的转换呢 ?

local read 和 local write 分别代表本地CPU读写.
remote read 和 remote write 分别代表其他CPU读写.

| 当前状态 | 事件 | 行为 | 下一个状态 |
|:------:|:----:|:---:|:---------:|
| I(invalid) | local read | 1. 如果其他处理器中没有这份数据, 本缓存从内存中取该数据, 状态变为E<br>2. 如果其他处理器中有这份数据, 且缓存行状态为M, 则先把缓存行中的内容写回到内存. 本地cache再从内存读取数据, 这时两个cache的状态都变为S<br>3. 如果其他缓存行中有这份数据, 并且其他缓存行的状态为S或E, 则本地cache从内存中取数据, 并且这些缓存行的状态变为S | E 或 S |
| I(invalid) | local write | 1. 先从内存中取数据, 如果其他缓存中有这份数据, 且状态为M, 则先将数据更新到内存再读取(个人认为顺序是这样的, 其他 CPU 的缓存内容更新到内存中并且被本地 cache 读取时, 两个cache状态都变为S, 然后再写时把其他CPU的状态变为I, 自己的变为M)<br>2. 如果其他缓存中有这份数据, 且状态为E或S, 那么其他缓存行的状态变为I | M |
| I(invalid) | remote read | remote read 不影响本地 cache 的状态 | I |
| I(invalid) | remote write | remote read 不影响本地 cache 的状态 | I |
| <br>*-* <br> | <br>*-* <br> | <br>*-* <br> |<br>*-*-*-* <br> |
| E(exclusive) | local read | 状态不变 | E |
| E(exclusive) | local write | 状态变为 M | M |
| E(exclusive) | remote read | 数据和其他核共享, 状态变为 S | S |
| E(exclusive) | remote write | 其他 CPU 修改了数据, 状态变为 I | I |
| <br>*-* <br> | <br>*-* <br> | <br>*-* <br> |<br>*-*-*-* <br> |
| S(shared) | local read | 不影响状态 | S |
| S(shared) | local write | 其他 CPU 的 cache 状态变为I, 本地 cache 状态变为 M | M |
| S(shared) | remote read | 不影响状态 | S |
| S(shared) | remote write | 本地 cache 状态变为 I, 修改内容的 CPU 的 cache 状态变为 M | I |
| <br>*-* <br> | <br>*-* <br> | <br>*-* <br> |<br>*-*-*-* <br> |
| M(modified) | local read | 状态不变 | M |
| M(modified) | local write | 状态不变 | M |
| M(modified) | remote read | 先把 cache 中的数据写到内存中, 其他 CPU 的 cache 再读取, 状态都变为 S | S |
| M(modified) | remote write | 先把 cache 中的数据写到内存中, 其他 CPU 的 cache 再读取并修改后, 本地 cache 状态变为I. 修改的那个cache状态变为 M | I |

# 1 参考资料
-------

| 编号 | 链接 | 描述 |
|:---:|:----:|:---:|
| 1 | [理解Memory Barrier(内存屏障)](https://blog.csdn.net/caoshangpa/article/details/78853919) | NA |
| 2 | [内存屏障(Memory Barrier)与volatile](https://blog.csdn.net/fedorafrog/article/details/113782404) | NA |
| 3 | [OR1200处理器的Store Buffer分析](https://blog.csdn.net/leishangwen/article/details/35212811) | NA |
| 4 | [MESI 与 StoreBuffer 相互独立的猜想](https://www.cnblogs.com/lqlqlq/p/14414091.html) | NA |
| 5 | [内存屏障的来历](https://zhuanlan.zhihu.com/p/125549632) | NA |
| 6 | [内存屏障(Memory Barrier)究竟是个什么鬼？](https://www.bilibili.com/read/cv5131208) | NA |
| 7 | [处理器中的存储问题(三):从Store Buffer到TSO存储模型](https://zhuanlan.zhihu.com/p/149302247) | NA |
| 8 | [CPU 缓存和 volatile](https://www.cnblogs.com/xmzJava/p/11417943.html) | MESI 有个图表 | NA |
| 9 | [CPU多级缓存与缓存一致性, 详细的讲解](https://blog.csdn.net/weixin_43649997/article/details/108742221) | NA |
| 10 | [CPU有缓存一致性协议(MESI), 为何还需要volatile](https://blog.csdn.net/org_hjh/article/details/109626607) | NA |
| 11 | [笔记:cpu中的cache(二)](https://zhuanlan.zhihu.com/p/144836286) | NA |
| 12 | [Reducing Design Complexity of the Load/Store Queue](https://engineering.purdue.edu/~vijay/papers/2003/lsq.pdf) | NA |
| 13 | [乱序处理器中的LSQ简介](https://blog.csdn.net/baidu_35679960/article/details/79554428) | NA |
| 14 | [简述 典型处理器(如Cortex A9)中一条存储器读写指令的执行全过程](https://blog.csdn.net/baidu_35679960/article/details/78571097) | MA |
| 15 | [4.2 存储器读写指令的发射与执行1](https://blog.csdn.net/sailing_w/article/details/55003968) | NA |
| 16 | [乱序处理器中的LSQ简介](https://blog.csdn.net/baidu_35679960/article/details/79554428) | NA |
| 17 | [Volatile:内存屏障原理应该没有比这篇文章讲的更清楚了](https://www.bilibili.com/read/cv7429219/) | NA |
| 18 | [内存一致性模型](http://www.wowotech.net/memory_management/456.html) | NA |
| 19 | [从硬件层面理解memory barrier](https://zhuanlan.zhihu.com/p/184912992) | NA |
| 20 | [多处理器编程:从缓存一致性到内存模型](https://zhuanlan.zhihu.com/p/35386457) | NA |
| 21 | [聊聊原子变量、锁、内存屏障那点事(2) ](https://www.sohu.com/a/250274701_467784) | NA |
| 22 | [Spectre原理详解及分支毒化的实现](https://zhuanlan.zhihu.com/p/114680178) | NA |
| 23 | [Lecture 11: Memory Data Flow Technique](http://home.eng.iastate.edu/~zzhang/courses/cpre585_f03/slides/lecture11.pdf) | NA |
| 24 | [cpu 乱序执行与问题](https://blog.csdn.net/lizhihaoweiwei/article/details/50562732) | NA |
| 25 | [store-queue VS store-buffer](https://stackoverflow.com/questions/24975540/what-is-the-difference-between-a-store-queue-and-a-store-buffer) | NA |
| 26 | [《大话处理器》Cache一致性协议之MESI](https://blog.csdn.net/muxiqingyang/article/details/6615199) | NA |


<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
