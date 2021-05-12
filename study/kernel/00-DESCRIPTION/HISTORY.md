---

title: Linux 内核历史的 Git 记录
date: 2021-02-15 00:32
author: gatieme
tags:
    - linux
    - tools
categories:
        - 技术积累
thumbnail:
blogexcerpt: FZF 是目前最快的模糊搜索工具. 使用golang编写. 结合其他工具(比如 ag 和 fasd)可以完成非常多的工作. 前段时间, 有同事给鄙人推荐了 FZF, 通过简单的配置, 配合 VIM/GIT 等工具食用, 简直事半功倍, 效率指数级提升, 因此推荐给大家.

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

# 1 Linux 的历史上使用版本控制工具
-------

## 1.1
-------


最初, Linus Torvalds 根本没有使用版本控制工具. 内核贡献者会将补丁发布到 Usenet 组，然后发布到邮件列表中，Linus 在对这些补丁逐行检视之后, 将它们合并到自己的源码源树上。然后一段时间后, Linus 将发布 Linux 内核的新版本. 在这个阶段不同作者的各个补丁之间没有加以区分, 要研究这些补丁, 我们只能在这两个版本之间进行 diff.



## 1.2
-------

后来随着 Linux 内核项目规模的不断增大, 原本逐行审查变更的方式逐渐变得原来越没有效率，甚至有的时候还成为了制约项目进度的负面因素。

许多 Linux 内核开发人员对缺乏适当的版本控制工具感到不满，因此 Linus 总是面临一定程度的社区压力，在和团队新成员的冲突发生之后，Linus 意识到这个项目的规模已经成长到个人无法掌控的程度，于是 Linus 在多个版本控制工具中选择了 BitKeeper, 并果断放权，越来越多的让其他开发人员主动承担各个主要模块的开发工作。

这个成果是很明显的，整个团队的开发速度得到了提升，而 Linus 本人也能更加专注于核心模块的开发，以及架构的优化。

不过，BitKeeper 是一个由 BitMover 公司开发的封闭式商业系统，由 Larry McVoy 经营（他同时也是一位 Linux 内核开发者），而 Linux 内核是历史上最重要的开源项目。

Linus 允许了 BitMover 对 Linux 施加的一些重大限制，以换取非付费许可证。

1.  首先，Linux 开发人员在使用 BitKeeper 时将不允许参与竞争的修订控制项目。

2.  其次，BitMover 将控制与内核项目相关的某些元数据，以便注意到任何滥用许可证的行为。如果没有该元数据，内核开发人员将只能看到最新版本，无法比较过去的内核版本，但是这是其他修订控制系统的一个重要标准特征。

## 1.3 没有硝烟的战争
-------

虽然争议一直没有停歇，但是至少相安无事。直至，2005年4月，BitMover宣布将停止向社区免费提供BitKeeper版本，理由是OSDL(开放源开发实验室) 的开发人员 Andrew Tridgell 对 BitKeeper 做了逆向分析，以便创建一个免费的软件替代方案。我们相信如果不是 Andrew，那别人做这个也是迟早的事情。这只是时间问题。

战争已经发生了，虽然 TridgellBitMover 决定向一些核心开发人员提供免费的商业 BitKeeper 许可证，但它拒绝向 OSDL 雇佣的任何人发放或出售许可证。这其中就包括 Linus Torvalds 和 Andrew Morton。

Linus 花了几个星期试图在 和拉里 · 麦克沃伊之间进行调解， 但最终显然不起作用。Linus 想要放弃使用 BitKeeper，但是没人想回到之前没有版本控制器的日子，而更可悲的是， 当时， 虽然还有其他一些分布式版本控制工具，他们都达不到预期的要求。最终 Linus 决定自己写一套。

## 1.4 git 时代来临
-------

Linus 在闭关一段时间后（据说是 10 天左右），然后向全世界分享了他的 git，

1.  在2005年6月项目启动后的几天内，Linus的git修订控制系统就完全自承载了。

2.  几周内，它就准备承载Linux内核开发。

3.  在几个月内，它达到了完整的功能。

接着，Linus 将项目的维护工作移交给了最热心的贡献者 Junio C. Hamano，并再次全职返回 Linux 内核的开发。

https://blog.csdn.net/kakakaoo_/article/details/116055620

# 2 Linux History 仓库
-------


| 仓库 | 描述 |
|:---:|:---:|
| [history/history](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git) | 包含了 2.6.12-rc2 ~ v2.6.33-rc5 的 git 信息. 但是包含了自 Linux 0.10 ~ 2.6.33-rc5 的所有 tag 的代码.<br>2.6.12-rc2 之间的代码没有 git 信息 |
| [linux-history](https://github.com/tbodt/linux-history) | master: Git history from 2010 through the initial commit of 2.6.12-rc2<br>bitkeeper: BitKeeper history from 2.6.12-rc2 through 2.4.0<br>releases: One commit for each release from 2.4.0 through 0.01 |
| [tglx/history](https://git.kernel.org/pub/scm/linux/kernel/git/tglx/history.git) | master 分支包含了 v2.4.0 ~ v2.6.12-rc2 的 git 提交 |
| [davej/history](https://git.kernel.org/pub/scm/linux/kernel/git/davej/history.git) | davej 的 history tree, 包含了 Linux 0.01 ~ 2.4.0 的历史归档和提交. kernel cgit 的链接已经失效, 找到了一些镜像仓库.<br>[schwabe/davej-history](https://github.com/schwabe/davej-history)<br>[davej-history.git](https://repo.or.cz/w/davej-history.git) |
| [git-history-of-linux](https://archive.org/details/git-history-of-linux) | 这个网站归档了 0.0.1 ~ 2.6.12 的 Git 提交. 下载地址 [Files for git-history-of-linux](https://archive.org/download/git-history-of-linux),
| [remram44/linux-full-history](https://github.com/remram44/linux-full-history/tree/explanation) | 包含了 可以浏览完整历史记录, 包括在 v2.6.12-rc2 之前提交.<br>注意: 本人亲测此项目无效, 没有历史信息. |


其中 tglx 和 davej 的 history 仓库最为出名, 之前都是在 kerne 的 cgit 网站上可以下载的. 虽然 davej 的仓库连接已经失效了, 但是schwabe 这位仁兄镜像了 tglx 和 davej 的 history 信息.

```cpp
https://github.com/schwabe/davej-history
https://github.com/schwabe/tglx-history
```

其中 [linux-history](https://github.com/tbodt/linux-history) 是整理的最全最好的, 虽然她也是站在了巨人的肩膀上, 但是通过三个分支很好地把历史信息归档了, 因此鄙人非常感激作者的辛苦劳动. 同时为了防止该内容失联, 我 fork 了一份, 在[我个人仓库 gatieme/linux-history](https://github.com/gatieme/linux-history).


# 3 参考资料
-------


[Linux kernel “historical” git repository with full history](https://stackoverflow.com/questions/3264283/linux-kernel-historical-git-repository-with-full-history)


<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
