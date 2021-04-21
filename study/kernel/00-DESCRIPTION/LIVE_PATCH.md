2   **LIVE_PATCH 内核热补丁**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***


下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***

https://ruby-china.org/topics/20680
https://www.open-open.com/news/view/1d7445f
https://www.infoworld.com/article/2851028/four-ways-linux-is-headed-for-no-downtime-kernel-patching.html

# 1 热补丁的历史
-------

第 1 节内容参照 [History of Linux Kernel Live Patching](https://www.howtoforge.com/history-of-linux-kernel-live-patching)



| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2008/12/22 | Jeff Arnold <jbarnold@MIT.EDU> | [Ksplice: Rebootless kernel updates](https://lore.kernel.org/patchwork/cover/137199) | KSplice 的实现方案 | v1 ☐ | [PatchWork RFC v3](https://lore.kernel.org/patchwork/cover/135799)<br>*-*-*-*-*-*-*-* <br>[PatchWork v1](https://lore.kernel.org/patchwork/cover/137199) |
| 2014/04/30 |  Jiri Slaby <jslaby@suse.cz> | [kGraft](https://lore.kernel.org/patchwork/cover/460811) | SUSE 的 Kgraft 方案 | RFC v1 ☐ | [PatchWork RFC](https://lore.kernel.org/patchwork/cover/460811), [LWN](https://lwn.net/Articles/596776) |
| 2014/07/15 | Josh Poimboeuf <jpoimboe@redhat.com> | [kpatch: dynamic kernel patching](https://lore.kernel.org/patchwork/cover/482999) | Redhat 的实现方案 | v1 ☐ | [PatchWork RFC](https://lore.kernel.org/patchwork/cover/461063)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2](https://lore.kernel.org/patchwork/cover/482999) |
| 2014/12/16 | Seth Jennings <sjenning@redhat.com> | [Kernel Live Patching](https://lore.kernel.org/patchwork/cover/527257) | 内核热补丁的基础框架 | v7 ☑ 4.0-rc1 | [PatchWork v7](https://lore.kernel.org/patchwork/cover/527257), [LKML](https://www.lkml.org/lkml/2017/2/13/831)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6](https://lore.kernel.org/patchwork/cover/525706/) |



## 1.1 2001–2010: The Patent Trail(专利追溯)
-------

如果您使用热修补或实时系统更新等关键字浏览专利档案, 您将挖掘许多应用程序和拒绝, 表明更新计算机系统而不停止它的想法并不是什么新鲜事. 追踪从一般到具体的重要日期如下 : 

2001年 : 惠普公司申请了一种动态更新软件以避免丢失硬件功能的方法. 
2002年 : 微软加入游戏的方法是在不中断系统的情况下更新系统(Windows). (他们的初始申请因HP的“现有技术”而被驳回. )
2008年 : Jeff Arnold 宣布推出 Ksplice软件, 用于更新(修补)Linux内核而不会中断(即无需重启1). 
2010年 : 微软的专利最终获准上诉. 
关于这些的有趣之处在于, 他们共同希望通过软件更新来纠正系统核心软件或硬件中的故障, 而不会影响系统的持续运行并且不会改变硬件. 听起来很熟悉？(线索 : 熔化, 幽灵. )

## 1.2 2009年 : KSplice 的诞生
-------

Jeff Arnold 是一名麻省理工学院的学生, 正在照看他们的一台[服务器](http://news.mit.edu/2014/bringing-the-world-reboot-less-updates-0124). 它需要一个安全补丁, 但他推迟了它, 因为重新启动会给他的用户带来不便. 在系统更新之前, 它被黑了. 耻辱和(具有讽刺意味的)不便使杰夫在没有延迟而不重新启动系统更新的问题上找到了他的硕士论文的主题.

这个故事可能是杜撰的, 但它提醒我们, 现场补丁技术的出现不仅仅是为了方便而是为了安全, 它应该受到赞赏. 

Jeff Arnold 与其他三名学生同事一起研究了如何更新 Linux 服务器内核的问题, 没有延迟, 也没有中断系统的进程. 该解决方案采用名为 Ksplice 的软件形式, 其技术基础在2009年的学术论文中提出. 该文章的标题包括"重新启动"这个词, 现在熟悉的"不间断更新"的Linux简写, 但是 2005 年首次由微软创造, 用于 Windows 驱动程序更新. 

image.png
补丁和不打补丁进行了二进制的比较, 找出不同的函数, 源代码patch守护程序提取出不同的函数. 内核源码优化这些不同的单元)

毕业后, 杰夫和他的麻省理工学院同事创办了 Ksplice Inc., 并于 2009 年 5 月获得麻省理工学院 10 万美元创业大赛奖. 该公司于 2010 年推出了商业服务; 事情进展顺利. 

Ksplice 在替换新内核时, 不需要预先修改;只需要一个diff文件, 列出内核即将接受的修改即可. Ksplice公司免费提供软件, 但技术支持是需要收费的, 这个特性是如此的优秀, 以至于这家新创的公司短时间内就积累了大量的用户. 

参见:

[Ksplice: kernel patches without reboots](https://lwn.net/Articles/280058)

[Ksplice and kreplace](https://lwn.net/Articles/308409)

[Jeff Arnold 的 Ksplice 的补丁提交历史](https://lore.kernel.org/patchwork/project/lkml/list/?series=&submitter=9109&state=*&q=&archive=both&delegate=)



## 1.3 2011-2016 Oracle  
-------


但在 2011年7月21日 [Oracle 收购了 Ksplice Inc.](https://www.infoworld.com/article/2622437/oracle-buys-ksplice-for-linux--zero-downtime--tech.html), 情况发生了变化. 这项功能被整合到 Oracle 自己的 Linux 发行版本中, 并且只对 Oralcle 自己提供技术更新.

> 即使 Linux本身就是Red Hat 3 的衍生产品. 尽管有这种传统, 甲骨文仍然停止支持红帽.


Ksplice Inc. 在收购前后拥有着大量热补丁技术方面的专利, 而众所周知, Oracle 是一家 [patent troll](https://lwn.net/Articles/584016).



这就导致, 其他 linux 发行版开始寻找替代 Ksplice 的方法, 以避免被 Oracle 收取巨额的专利费.

[Ksplice](http://ksplice.oracle.com/legacy)


## 1.4 百家争鸣
-------

在 2011 年和 2014 年之间, SUSE 和 Red Hat 分开工作(并且不了解彼此的目标)来发布他们自己的实时内核更新解决方案, 他们分别在 Kgraft 和 Kpatch 中进行了这些解决方案.


### 1.4.1 Kpatch @ Redhat
-------

2014 年 Red Hat 分享了他们的 Kpatch 代码, 并将其集成为 Red Hat Enterprise Linux 的支持功能. 


### 1.4.1 Kgraft @ Suse
-------

[kGraft — live kernel patching from SUSE](https://lwn.net/Articles/584016)

https://git.kernel.org/pub/scm/linux/kernel/git/jirislaby/kgraft.git/

[The initial kGraft submission](https://lwn.net/Articles/596854)

[kGraft — live kernel patching from SUSE](https://lwn.net/Articles/584016)




### 1.4.2 KernelCare @ CloudLinux
-------

由于主要供应商争相成为第一个推出可行的实时补丁解决方案, CloudLinux 是基于 Linux 的网络托管操作系统的主要参与者, 在 3 月成功测试后, 于 2014年5 月推出了KernelCare. 

他们通过在大多数Linux平台上提供最广泛的功能集来为市场感到惊讶, 并在 Linux 内核开发和客户支持方面享有盛誉. 另一个令人震惊的是负担能力, 吸引了网站托管商, 他们发现 KernelCare 的每服务器成本比主要竞争对手的每站点成本更易于管理和扩展. 


## 1.5 Kgraft + Kpatch 的混合体 Livepatch
-------

SUSE 和 Red Hat 都尝试将自家的解决方案推向 Linux Mainline, 社区经过激烈的讨论, 最终融合了两家方案的创意形成 livepatch 方案, 合入 v4.0.
随后在并在2016年10月, Canonical 宣布他们正基于 livepatch 推出自己的的商业内核更新服务 Canonical Livepatch服务.


## 1.6 总结
-------

# 2 热补丁实现方案
-------


## 2.1 热补丁方案需要解决的问题
-------

1.  函数热替换, 热补丁要完成的最本质的工作, 就是不重启而更新执行的函数, 怎么完成函数的替换. 修改函数的前几条指令, 将指令修改为跳转到新函数的指令可以完成这样的工作. 那有没有其他方法呢 ?

2.  重定位问题, 正常来说驱动只能使用内核或者驱动所 EXPORT_SYMBOL 的符号, 这类符号在驱动插入的过程中, 进行重定位, 将符号指向了所引用符号的实际地址. 而热补丁则要求更宽松一些, 它需要能够直接访问被 patched 的函数中的未导出的符号信息, 这些符号正常内核 load_module 是不会进行重定位的. 只能热补丁自己来完成重定位. 这些符号的信息需要热补丁自己去甄别和重定位.

3.  一致性问题, 内核不重启而执行新函数, 那么在系统运行过程中, 势必存在一个时间点, 之前执行了旧函数, 之后将执行新函数. 那么这个时间点怎么确认, 即怎么的模型下, 执行函数的热替换操作, 那么可以认为内核的前后是一致的, 不会出现逻辑错误, 更不会引入安全问题. 更重要的是, 如果多个被 patched 的函数之间有逻辑依赖, 一致性怎么保证. 举例来说, 假设两个函数一个执行 A++, 一个执行 A--, 每次必然成对执行. 最终 A 计数将能够正常归 0. 但是如果 patched 之后, 变成 A+=2, A-=2, 那么怎么保证不会出现, A++, A-=2 或者 A+=2, A-- 之类的执行序列.

4.  自动制作热补丁, 我们制作热补丁看到的信息只有修改的补丁, 因此热补丁的制作, 需要程序员对整个编译和链接的过程高度理解, 了解那些符号可能发生了什么样变化, 并能将这些信息组织到热补丁中. 那有没有程序式的工具, 能够自动化完成这些工作, 生成热补丁 KO.


## 2.2 已知热补丁方案
-------

在主线内核实现热补丁特性之前, 各个厂商就在社区进行了激烈的讨论, 并各自形成了自己的一套方案.

| 方案 | 作者 | 实现 | 一致性模型 | 限制与约束 |
|:---:|:----:|:---:|:--------:|:---------:|
| Ksplice | Oracle | [Ksplice: Rebootless kernel updates](https://lore.kernel.org/patchwork/cover/137199) 基于直接跳转方式 | stop_machine | 
| KGraft | SUSE 的 Jiri Kosina(JK) 和 Jiri Slaby(JS) 合作开发 | 基于 ftrace 方案实现, 采用类 RCU 更新机制 | 只要不会出现同一个 universe 里既调用了旧函数又调用了新函数的情况, 那么就是一致的<br>对于用户进程, 同一次系统调用是一次 universe<br>对于内核线程, 每次被唤醒, 判断是否被 stop 条件之后算是一个新的universe<br>对于中断, 同一次中断处理算是一次 universe | 仅支持 X86_64/s390, 未公开自动化热补丁制作工具 |
| Kpatch | REDHAT | 基于 ftrace 方案, 使用 stop_machine + 栈检查来保证一致性 | 使用 stop_machine 机制停下所有 CPU, 然后对所有的进程进行栈检查, 只有在没有任何进程执行被 patched 的函数的情况下, 才认为是安全的 | stop_machine 机制对业务有中断影响, 大概带来 1ms-40ms 的延迟<br>仅支持 X86_64 |
| kpatch without stop_machine | HITACHI 的 Masami Hiramatsu | 基于 kpatch 方案, 取消了 stop_machine一致性检查 | 不再需要 stop_machine, 引入全局的院子计数器 refcounter, 跟踪目标函数的调用情况, 只有在目标函数没有被执行时(refcounter == 0), 才可以安全的进行替换 | 基于 kpatch<br>一致性检查存在安全问题, 参见 [LinuxConNA-kpatch-without-stopmachine_fixed](https://events.static.linuxfound.org/sites/events/files/slides/LinuxConNA-kpatch-without-stopmachine_fixed.pdf), 以及 [kmod/core: Remove stop_machine from kpatch](https://github.com/mhiramathitachi/kpatch/commit/54e86e251ead4fd4db2fcf0f0157b6a41a541d17) |
| LIVEPATCH Without Ftrace | HUAWEI | 基于直接跳转实现 livepatch, 不再基于 ftrace, 通过修改函数的前几条指令, 直接跳转到新函数 | 同 kpatch 类似, 采用 stop_machine + 栈检查来保证安全性 | 与 ftrace/kprobe 等同样修改函数指令的特性存在冲突 |

## 2.3 热补丁方案总结
-------

总体来看:

1.  实现思路主要有两类:

| 实现方法 | 优点 | 缺点 |
|:------:|:----:|:---:|
| 基于 ftrace 的方案, 通过注册 ftrace, 修改函数的返回地址, ftrace 返回后, 直接跳转到新函数执行 | 跟 ftrace/kprobe 等同样需要修改函数指令的特性不冲突 | 依赖与 ftrace_regs 特性, 支持架构有限<br>ftrace 方案有性能问题, 虽然使用 x86_64 等架构的 ftrace trampoline 机制可以当前函数只注册了一个 ftrace 的时候直接使用 tracepoline 来优化性能, 但是使用场景受限, 且并不能解决 ftrace 所有的性能问题<br>基于的特性 ftrace 本身是一个调测特性, 稳定性和安全性都存在一定问题, 商用有一定风险 |
| 基于直接跳转的方案, 通过修改函数的前几条指令, 直接跳转到新函数 | 无性能问题 | 跟 ftrace/kprobe 等同样需要修改指令的方案存在冲突<br>涉及长跳转时, 可能需要改多条指令, 对函数的长度有要求限制<br>必须有可靠的栈回溯机制, 修改多条指令, 必须要求被 patched 的函数不能正在执行, 否则执行的指令前后不一致, 将出现严重问题 |



## 2.4 内核社区的讨论
-------

最终经过了激烈的讨论, 大家认为各家的方案都存在一些问题, 社区对于使用哪家的方案也一直犹豫不决 

最终 Redhat 主张先搞一个动态热补丁的通用框架, 基于 ftrace 方案, 完成函数热替换的最基本功能, 不保证安全性, 不使用任何安全性机制, 只通过通用的热补丁制作/注册和使用的接口. 作为一个过渡方案. 而各家 KGraft/Kpatch 基于这个框架完成自己的方案. 一致性模型待后期充分讨论和检验后再做决定. 参见 [Re: [PATCH 0/2] Kernel Live Patching](https://lkml.org/lkml/2014/11/7/306). 这个过程分为如下几个步骤:

1.  实现一套通用的核心框架, 这三种方法都可以使用(manual、kpatch generator、kGraft generator).

2.  添加一致性模型(例如 kpatch stop_machine, kGraft per task consistency, Masami's per task ref-count).

3.  添加组合补丁模块生成工具.


这个提议的最大依据就是, 大部分的 CVE 补丁即使没有一致性保证, 也可以安全的以动态补丁的方式打上. 因此社区可以先合入这样一个通用的基础方案. 然后接下来在考虑各种一致性方案的问题, 可以选择一种最优的一致性方案, 也可以将各种方案都集成进来, 然后用户根据自己 patch 的特点, 选择不同的一致性模型. 这个提议得到了大多数人的支持, 并由 Redhat 的 Seth Jennings 负责实现这个通用的框架. 

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:---------:|:----:|
| 2014/12/16 | Seth Jennings | Kernel Live Patching | 内核热补丁的基础框架 | v7 ☑ 4.0-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/527257), [LKML](https://www.lkml.org/lkml/2017/2/13/831) |



# 3 一致性模型 consistency model
-------


一致性模型主要有三种思路

| 实现方法 | 优点 | 缺点 |
|:------:|:----:|:---:|
| stop_machine consistency 通过 stop_machine 停住所有核进行一致性检查, 通过栈回溯检查被 patched 函数是否被执行 | 安全可靠, stop_machine 会强制所有核停下原来的工作, 在非抢占式内核, 必然发生进程切换, 则总能找到一个安全的时刻去执行热补丁操作 | stop_machine 会中断原来的业务, 对性能影响较大 | 
| per-task consistency 通过标记或者引用技术, 标记出被 patched 的函数都没有执行, 或者进程处于安全上下文的状态 | 不中断原来业务的执行 | 一致性检查并不是完全可信的 |


## 3.1 per-task consitency
-------


## 3.2 stop_machine consistency
-------


## 3.3 per task ref counting
-------



## 3.2 关于一致性模型的探讨
------


### 3.2.1 per-task VS stop_machine
-------

kgraft 的 per-task consistency 和 Kpatch 的 stop_machine consistency 是两种截然不同的一致性模型, 可以参照 Kpatch 的补丁的 [changelog](https://lore.kernel.org/patchwork/cover/482999).


### 3.2.2 consistency model 总结
-------

一致性模型是非常复杂的, 在第一版的 LIVE_PATCH 开发的时候, 内核开发者们就一致性mixing进行了激烈的讨论.

最初开发者们建议大家基于 livepatch 的核心框架, 把目前所有的一致性模型都实现了, 然后供用户根据场景自己去选择. 但是随着讨论的不断深入, 邮件列表中分析了几种一致性模型, 参见 [`Re: [PATCH 0/2] Kernel Live Patching`](https://lkml.org/lkml/2014/11/7/354) 或者 [`[PATCH 0/2] Kernel Live Patching`](https://lore.kernel.org/patchwork/cover/514589/).

在这次讨论中, Vojtech Pavlik 从 Masami Hiramatsu  [kpatch without stopmachine](https://events.static.linuxfound.org/sites/events/files/slides/LinuxConNA-kpatch-without-stopmachine_fixed.pdf) 的[实现 kmod/core: Remove stop_machine from kpatch](https://github.com/mhiramathitachi/kpatch/commit/54e86e251ead4fd4db2fcf0f0157b6a41a541d17) 中获得了启发, 发现了 per-task 和 stop_machine 两种一致性模型的相通之处. 然后对一致性模型进行了概括.


#### 3.2.2.1 一致性模型
-------

首先, 对新旧函数的更新时机做了分类, 即执行必须在哪个实体之外才能进行转换, 从最弱到最强:

| 转换时机 | 描述 |
|:---:|:----:|
| LEAVE_FUNCTION | 执行必须离开一个修补过的函数切换到新的实现 |
| LEAVE_PATCHED_SET | 执行必须离开补丁函数集, 以切换到新的实现 |
| LEAVE_KERNEL | 执行必须离开整个内核切换到新的实现 |


然后, 是什么实体发生了转换. 同样, 从最弱到最强:

| 转换实体 | 描述 |
|:---:|:----:|
| SWITCH_FUNCTION | 切换到新实现是基于每个函数的 |
| SWITCH_THREAD | 切换到新实现是每线程的 |
| SWITCH_KERNEL | 整个内核同时切换到新的实现 |

最后我们对目前所有的一致性模型进行对号入座.

| 一致性模型 | 转换时机 | 转换实体 |
| livepatch (null模型) | LEAVE_FUNCTION | SWITCH_FUNCTION |
| kpatch | LEAVE_PATCHED_SET | SWITCH_KERNEL |
| masami-refcounting | LEAVE_PATCHED_SET | SWITCH_KERNEL |
| Ksplice | LEAVE_PATCHED_SET | SWITCH_KERNEL |
| kGraft | LEAVE_KERNEL | SWITCH_THREAD |
| CRIU/kexec | LEAVE_KERNEL | SWITCH_KERNEL |


目前公认最安全的模型是 LEAVE_PATCHED_SET 和 SWITCH_KERNEL, 因为它可靠、快速收敛、不需要注释内核线程. 它提供了所需的最严格的一致性. 

通过混合 kGraft 和 masami-refcounting, 可以创建一个一致性引擎, 它几乎可以包含这些属性的任何组合, 以及所有的一致性模型. 


> PS.: Livepatch 的 NULL 模型实际上并不是最弱的, 因为它仍然保证执行完整完整的函数, 这要感谢 ftrace. 这比直接在内存中重写函数所达到的效果要大得多. 
> 这也是为什么 Ksplice 被锁定在一个非常特定的一致性模型上的原因. Ksplice 只能在内核停止并以此为基础构建模型时进行修补, 因此他不得不使用 stop_machine.
> masami-refcounting, kpatch, kGraft, livepatch 因为是基于 ftrace 的, 在一致性模型中在某些方面有了更多的自由.

后来 Vojtech Pavlik [Re: [PATCH 0/2] Kernel Live Patching](https://lkml.org/lkml/2014/11/8/15) 又提了一些有趣的场景和例子, 用来说明不同一致性模型的影响.

#### 3.2.2.2 SWITCH_THREAD VS SWITCH_KERNEL
-------

SWITCH_THREAD 存在一个问题, 就是它允许旧函数可以与新函数同时运行. 因此当补丁改变数据或数据语义时, 它产生了一些严重的头痛(经过评估 CVE 安全补丁中有 10% 是这样的补丁). 在这种模型下, 这使得补丁的安全分析变得更加困难, 因为你需要考虑的场景的排列加倍了. 除了考虑 newfunc/olddata 和 newfunc/newdata 之外, 还必须考虑 oldfunc/olddata 和 oldfunc/newdata. 为了这种场景引入的问题, 它需要把修复补丁拆成两个. 第一个补丁需要修改旧的功能, 以便能够处理新的数据(oldfunc/newdata). 在完全应用了第一个补丁之后(此时不再出现 oldfunc/newdata), 您可以应用第二个补丁, 它可以开始创建新版本的数据(newfunc/newdata).

另一方面, SWITCH_KERNEL没有这些问题. 它保证了在同一时刻内核全部由 oldfunc 切换到 newfunc, 但是它也不是完美的, 它需要中断内核中的业务来完成更新, 并检查系统中所有进程的堆栈, 确保没有进程正在执行待更新的路径. 因此它不能修补内核中的热点函数和一直在使用的功能. 但在这种情况下, 我们可以在90%的情况下跳过回溯检查. 所以这真的是一种可能. 经过分析大概有 0.2% 的补丁不能用 SWITCH_KERNEL 打补丁. 但即便如此, 我认为我们也可以通过创造性地解决这个问题, 例如使用多个补丁方法. 

Josh Poimboeuf 的观点是 SWITCH_THREAD 引起头痛的概率是10%, 而 SWITCH_KERNEL 引起头痛的概率是 1.8%. 从这个角度上讲就有一个有趣的模型: LEAVE_PATCHED_SET 和 SWITCH_THREAD, 它提供了最少的更改所需的一致性, 函数的调用约定仍然允许语义依赖关系.

Josh Poimboeuf 从 Masami kpatch without stop_machine 方案中得到灵感, 认为没有必要把所有的一致性模型都实现, 通过混合 kGraft 和 masami-refcounting, 可以创建一个一致性引擎, 它几乎可以包含这些属性的任何组合, 以及所有的一致性模型.


## 3.4 hybrid consistency model
-------

在经过漫长的讨论之后, 基于 PER-TASK 的混合一致性模型合入了主线.


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:---------:|:-----:|
| 2017/2/13 | Josh Poimboeuf | [livepatch: hybrid consistency model](https://lore.kernel.org/patchwork/cover/760164) | 基于 Kgraft 的PER TASK 的状态检查和基于 Redhat 可靠的栈检查的混合一致性模型 | v5 ☑ 4.12-rc1 | [PatchWork](https://lore.kernel.org/patchwork/cover/760164), [LKML](https://www.lkml.org/lkml/2017/2/13/831) |


这个一致性模型被称为 **混合一致性模型 "hybrid consistency model"**, 因为他融合了

*   Kgraft 的 per-task 一致性模型, 保证同一个 universe 里不同时调用了旧函数又调用了新函数的情况

*   reliable stacktrace 可靠的栈检查, 检查被 patched 的函数不被执行.


将这几种互补的一致性方法结合, 来确定什么时候可以安全地修补任务

1.  第一个也是最有效的方法是对睡眠任务进行堆栈检查. 如果给定任务的堆栈中没有受影响的函数, 则该任务将被修补. 在大多数情况下, 这将在第一次尝试时修补大部分或所有的任务. 否则它会周期性地不断尝试. 这个选项只有在体系结构有可靠的堆栈时才可用(HAVE_RELIABLE_STACKTRACE). 

2. 第二种方法(如果需要的话)是内核退出切换. 当任务从系统调用、用户空间IRQ或信号返回到用户空间时, 它就被切换. 它在以下情况下是有用的:
    *   a) 修补I/O 绑定的用户任务, 这些任务在受影响的函数上处于休眠状态. 在这种情况下, 您必须发送SIGSTOP和SIGCONT来强制它退出内核并进行修补. 

    *   b) 修补cpu绑定的用户任务. 如果任务是高度cpu限制的, 那么它将在下次被IRQ中断时得到修补. 

    *   c) 在将来, 它可以用于为还没有HAVE_RELIABLE_STACKTRACE的架构应用补丁. 在这种情况下, 您必须向系统上的大多数任务发出信号. 然而, 这还不支持, 因为目前没有办法在没有HAVE_RELIABLE_STACKTRACE的情况下修补线程. 

3.  对于空闲任务 "swapper"(IDLE) , 因为它们永远不会退出内核, 所以它们在空闲循环中有一个 klp_update_patch_state()调用, 允许它们在CPU进入空闲状态之前被修补. 

### 3.4.1 per-task consistency model
-------

混合一致性模型中使用了 per-task 的一致性模型, 参见 [livepatch: change to a per-task consistency model](https://lore.kernel.org/patchwork/patch/760175).


### 3.4.2 reliable stacktrace
-------

可靠栈检查则进一步对 per-task 的一致性模型做了补充. 他检查函数是否正在运行, [livepatch: change to a per-task consistency model](https://lore.kernel.org/patchwork/patch/760175)


## 3.3 一致性总结
-------


# 4 重定位信息
-------

## 4.1 重定位段
-------

## 4.2 归一的重定位信息
-------

## 4.3 JUMP_LABEL 支持
-------

kpatch 的实现一直是根据内核的进展而演进的, 对 JUMP_LABEL 的不断重试也体现除了这个问题的难搞.

1.  最开始 kpatch-build 不支持 JUMP_LABEL, [Livepatch does not handle static keys #931](https://github.com/dynup/kpatch/pull/931)

2. 为 kpatch-build 引入了 JUMP_LABEL 支持 [Add jump label support #937](https://github.com/dynup/kpatch/pull/937)

3.  发现内核热补丁对 JUMP_LABEL 的支持存在问题, 因此 kpatch 回退了之前的提交 [Revert "create-diff-object: add jump label support"](https://github.com/dynup/kpatch/pull/944)

4.  内核社区正在解决这个问题, kpatch-build 对 JUMP_LABEL 报告 WARN, 待内核解决后, 再行支持 [kpatch-build should warn about static keys #946](https://github.com/dynup/kpatch/issues/946)

对 JUMP_LABEL 的支持主要存在如下几点问题:

1.  如果 JUMP_LABEL 所使能的符号是属于内核或者被 patched 的驱动的, 那么

最终主线内核通过这个补丁解决了对 JUMP_LABEL 的支持问题 [livepatch: Apply vmlinux-specific KLP relocations early](https://lore.kernel.org/patchwork/cover/1233223).

| 时间  | 作者 |特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:---------:|:----:|
| 2017/03/28 | Dietmar Eggemann | [livepatch,module: Remove .klp.arch and module_disable_ro()](https://lore.kernel.org/patchwork/cover/1233213) | 增加 PELT 的跟踪点 | v1 | [PatchWork](https://lore.kernel.org/patchwork/cover/774154) |


# 5 其他改进措施
-------

## 5.1 引入 shadow 支持结构体中扩充字段
-------

| 时间  | 作者 |特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:--:|:----:|:---------:|:----:|
| 2017/08/31 | Joe Lawrence | [livepatch: introduce shadow variable API](https://lore.kernel.org/patchwork/cover/827343) | 增加 SHADOW API 支持结构体中引入新的变量 | v6 ☑ 4.15-rc1  | [PatchWork](https://lore.kernel.org/patchwork/cover/827343) |

## 5.2 atomic replace
-------

# 6 架构支持
-------




# 7 Kpatch 自动化工具
-------


相关的文章介绍: [47].




---

**引用:**

<div id="ref-anchor-1"></div>
- [1] [Single UNIX Specification](https://en.wikipedia.org/wiki/Single_UNIX_Specification%23Non-registered_Unix-like_systems)

<div id="ref-anchor-2"></div>
- [2] [POSIX 关于调度规范的文档](http://nicolas.navet.eu/publi/SlidesPosixKoblenz.pdf)

<div id="ref-anchor-3"></div>
- [3] [Towards Linux 2.6](https://link.zhihu.com/?target=http%3A//www.informatica.co.cr/linux-scalability/research/2003/0923.html)

<div id="ref-anchor-4"></div>
- [4] [Linux内核发布模式与开发组织模式(1)](https://link.zhihu.com/?target=http%3A//larmbr.com/2013/11/02/Linux-kernel-release-process-and-development-dictator-%26-lieutenant-system_1/)

<div id="ref-anchor-5"></div>
- [5] IBM developworks 上有一篇综述文章, 值得一读 :[Linux 调度器发展简述](https://link.zhihu.com/?target=http%3A//www.ibm.com/developerworks/cn/linux/l-cn-scheduler/)

<div id="ref-anchor-6"></div>
- [6] [CFS group scheduling [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/240474/)

<div id="ref-anchor-7"></div>
- [7] [http://lse.sourceforge.net/numa/](https://link.zhihu.com/?target=http%3A//lse.sourceforge.net/numa/)

<div id="ref-anchor-8"></div>
- [8] [CFS bandwidth control [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/428230/)

<div id="ref-anchor-9"></div>
- [9] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D5091faa449ee0b7d73bc296a93bca9540fc51d0a)

<div id="ref-anchor-10"></div>
- [10] [DMA模式\_百度百科](https://link.zhihu.com/?target=http%3A//baike.baidu.com/view/196502.htm)

<div id="ref-anchor-11"></div>
- [11] [进程的虚拟地址和内核中的虚拟地址有什么关系? - 詹健宇的回答](http://www.zhihu.com/question/34787574/answer/60214771)

<div id="ref-anchor-12"></div>
- [12] [Physical Page Allocation](https://link.zhihu.com/?target=https%3A//www.kernel.org/doc/gorman/html/understand/understand009.html)

<div id="ref-anchor-13"></div>
- [13] [The SLUB allocator [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/229984/)

<div id="ref-anchor-14"></div>
- [14] [Lumpy Reclaim V3 [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/211199/)

<div id="ref-anchor-15"></div>
- [15] [Group pages of related mobility together to reduce external fragmentation v28 [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/224254/)

<div id="ref-anchor-16"></div>
- [16] [Memory compaction [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/368869/)

<div id="ref-anchor-17"></div>
- [17] [kernel 3.10内核源码分析--TLB相关--TLB概念、flush、TLB lazy模式-humjb\_1983-ChinaUnix博客](https://link.zhihu.com/?target=http%3A//blog.chinaunix.net/uid-14528823-id-4808877.html)

<div id="ref-anchor-18"></div>
- [18] [Toward improved page replacement[LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/226756/)

<div id="ref-anchor-19"></div>
- [19] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D4f98a2fee8acdb4ac84545df98cccecfd130f8db)

<div id="ref-anchor-20"></div>
- [20] [The state of the pageout scalability patches [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/286472/)

<div id="ref-anchor-21"></div>
- [21] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D894bc310419ac95f4fa4142dc364401a7e607f65)

<div id="ref-anchor-22"></div>
- [22] [Being nicer to executable pages [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/333742/)

<div id="ref-anchor-23"></div>
- [23] [kernel/git/torvalds/linux.git](https://link.zhihu.com/?target=https%3A//git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/%3Fid%3D8cab4754d24a0f2e05920170c845bd84472814c6)

<div id="ref-anchor-24"></div>
- [24] [Better active/inactive list balancing [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/495543/)

<div id="ref-anchor-25"></div>
- [25] [Smarter write throttling [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/245600/)

<div id="ref-anchor-26"></div>
- [26] [https://zh.wikipedia.org/wiki/%E6%8C%87%E6%95%B0%E8%A1%B0%E5%87%8F](https://link.zhihu.com/?target=https%3A//zh.wikipedia.org/wiki/%25E6%258C%2587%25E6%2595%25B0%25E8%25A1%25B0%25E5%2587%258F)

<div id="ref-anchor-27"></div>
- [27] [Flushing out pdflush [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/326552/)

<div id="ref-anchor-28"></div>
- [28] [Dynamic writeback throttling [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/405076/)

<div id="ref-anchor-29"></div>
- [29] [On-demand readahead [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/235164/)

<div id="ref-anchor-30"></div>
- [30] [Transparent huge pages in 2.6.38 [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/423584/)

<div id="ref-anchor-31"></div>
- [31] [https://events.linuxfoundation.org/sites/events/files/lcjp13\_ishimatsu.pdf](https://link.zhihu.com/?target=https%3A//events.linuxfoundation.org/sites/events/files/lcjp13_ishimatsu.pdf)

<div id="ref-anchor-32"></div>
- [32] [transcendent memory for Linux [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/338098/)

<div id="ref-anchor-33"></div>
- [33] [linux kernel monkey log](https://link.zhihu.com/?target=http%3A//www.kroah.com/log/linux/linux-staging-update.html)

<div id="ref-anchor-34"></div>
- [34] [zcache: a compressed page cache [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/397574/)

<div id="ref-anchor-35"></div>
- [35] [The zswap compressed swap cache [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/537422/)

<div id="ref-anchor-36"></div>
- [36] [Linux-Kernel Archive: Linux 2.6.0](https://link.zhihu.com/?target=http%3A//lkml.iu.edu/hypermail/linux/kernel/0312.2/0348.html)

<div id="ref-anchor-37"></div>
- [37]抢占支持的引入时间: [https://www.kernel.org/pub/linux/kernel/v2.5/ChangeLog-2.5.4](https://link.zhihu.com/?target=https%3A//www.kernel.org/pub/linux/kernel/v2.5/ChangeLog-2.5.4)

<div id="ref-anchor-38"></div>
- [38] [RAM is 100 Thousand Times Faster than Disk for Database Access](https://link.zhihu.com/?target=http%3A//www.directionsmag.com/entry/ram-is-100-thousand-times-faster-than-disk-for-database-access/123964)

<div id="ref-anchor-39"></div>
- [39] [http://www.uefi.org/sites/default/files/resources/ACPI\_6.0.pdf](https://link.zhihu.com/?target=http%3A//www.uefi.org/sites/default/files/resources/ACPI_6.0.pdf)

<div id="ref-anchor-40"></div>
- [40] [Injecting faults into the kernel [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/209257/)

<div id="ref-anchor-41"></div>
- [41] [Detecting kernel memory leaks [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/187979/)

<div id="ref-anchor-42"></div>
- [42] [The kernel address sanitizer [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/612153/)

<div id="ref-anchor-43"></div>
- [43] [Linux Kernel Shared Memory 剖析](https://link.zhihu.com/?target=http%3A//www.ibm.com/developerworks/cn/linux/l-kernel-shared-memory/)

<div id="ref-anchor-44"></div>
- [44] [KSM tries again [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/330589/)

<div id="ref-anchor-45"></div>
- [45] [HWPOISON [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/348886/)

<div id="ref-anchor-46"></div>
- [46] [https://www.mcs.anl.gov/research/projects/mpi/](https://link.zhihu.com/?target=https%3A//www.mcs.anl.gov/research/projects/mpi/)

<div id="ref-anchor-47"></div>
- [47] [Fast interprocess messaging [LWN.net]](https://link.zhihu.com/?target=https%3A//lwn.net/Articles/405346/)



---8<---

**更新日志:**

**- 2015.9.12**

o 完成调度器子系统的初次更新, 从早上10点开始写, 写了近７小时, 比较累, 后面更新得慢的话大家不要怪我(对手指

**- 2015.9.19**

o 完成内存管理子系统的前4章更新. 同样是写了一天, 内容太多, 没能写完......

**- 2015.9.21**

o 完成内存管理子系统的第5章"页面写回"的第1小节的更新.
**- 2015.9.25**

o 更改一些排版和个别文字描述. 接下来周末两天继续.
**- 2015.9.26**

o 完成内存管理子系统的第5, 6, 7, 8章的更新.
**- 2015.10.14**

o 国庆离网10来天, 未更新.  今天完成了内存管理子系统的第9章的更新.
**- 2015.10.16**

o 完成内存管理子系统的第10章的更新.
**- 2015.11.22**

o 这个月在出差和休假, 一直未更新.抱歉! 根据知友 [@costa](https://www.zhihu.com/people/78ceb98e7947731dc06063f682cf9640) 提供的无水印图片和考证资料, 进行了一些小更新和修正. 特此感谢 !

o 完成内存管理子系统的第11章关于 NVDIMM 内容的更新.
**- 2016.1.2**

o 中断许久, 今天完成了内存管理子系统的第11章关于调试支持内容的更新.
**- 2016.2.23**

o 又中断许久, 因为懒癌发作Orz... 完成了第二个子系统的所有章节.
[编辑于 06-27](https://www.zhihu.com/question/35484429/answer/62964898)
