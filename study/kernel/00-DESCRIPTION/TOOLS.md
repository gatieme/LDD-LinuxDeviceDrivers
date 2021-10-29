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

   **工具**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***


# 1 开发工具
-------


社区一直在寻求一种非邮件列表的内核开发方式

1.  [Pulling GitHub into the kernel process](https://lwn.net/Articles/860607)

2.  [How Red Hat uses GitLab for kernel development](https://lwn.net/Articles/871237)

3.  [Moving Google toward the mainline](https://lwn.net/Articles/871195)


# 2 调试工具
-------

## 2.1 debug
-------

systrace, perfetto, drgn

## 2.2 call kernel func from userspace
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2012/06/23 | Peiyong Feng <peiyong.feng.kernel@gmail.com> | [Implement uhook(call kernel func from userspace) driver](https://lwn.net/Articles/503414) | 提供了 uhook(userspace kernel hook) 驱动和工具用于在用户态直接调用内核态的函数, 这对于调试内核非常有帮助. | v1 ☐ | [LWN](https://lwn.net/Articles/503414), [bigfeng12/uhook](https://github.com/bigfeng12/uhook)<br>*-*-*-*-*-*-*-* <br>[LKML1](https://lkml.org/lkml/2012/6/22/570), [LKML2](https://lkml.org/lkml/2012/6/22/572) |

## 2.3 stack backtrace
-------

在内核态尝试 dump 用户态的栈对调试是非常有帮助的.

[how to print stack trace in-your-user](http://kungjohan.blogspot.com/2012/06/how-to-print-stack-trace-in-your-user.html)

[how-to-dump-user-space-stack-in-kernel-space](https://stackoverflow.com/questions/38384259/how-to-dump-user-space-stack-in-kernel-space)

[](https://groups.google.com/g/linux.kernel/c/ca3lZGlbnmo)

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/06/23 | 胡俊鹏 and <dongzhiyan_linux@163.com> | [dongzhiyan-stack/user_stack_backstrace-in-kernel](https://github.com/dongzhiyan-stack/user_stack_backstrace-in-kernel) | 海康 CLK 2019 的一个 slides, 内核态回溯用户态栈. 对于一些比较难解析符号的场景也有对策 | v1 ☐ | [github](https://github.com/dongzhiyan-stack/user_stack_backstrace-in-kernel) |
| 2012/4/11 | "Tu, Xiaobing" <xiaobing.tu@intel.com> | [kernel patch for dump user space stack tool](https://lkml.org/lkml/2012/4/11/49) | 内核态回溯用户态栈. | v1 ☐ | [LKML RFC 1/2](https://lkml.org/lkml/2012/4/11/49) |


## 2.4 patchwork
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/01/10 | Joe Perches <joe@perches.com> | [Documentation: Replace lkml.org links with lore](https://patchwork.kernel.org/project/linux-omap/patch/77cdb7f32cfb087955bfc3600b86c40bed5d4104.camel@perches.com/#23968747) | 补丁将内核中 lkml 的链接转换为 lore.kernel.org 的链接. 补丁描述中提供了一个脚本来完整这个工作. | v1 ☑ 5.12-rc1 | [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=05a5f51ca566674e6a6ee9cef0af1b00bf100d67) |
| 2020/02/06 | Joe Perches <joe@perches.com> | [B4 Better tools for kernel developers](https://lwn.net/Articles/811528) | 补丁将内核中 lkml 的链接转换为 lore.kernel.org 的链接. 补丁描述中提供了一个脚本来完整这个工作. | v1 ☑ 5.12-rc1 | [GIT](https://git.kernel.org/pub/scm/utils/b4/b4.git), [elinux](https://elinux.org/images/9/93/Kernel_email_tools_elce_2020.pdf) |

patchwork 提供了 [API](https://patchwork.kernel.org/api/projects) 可以获取到各个 patchwork 子项目的 ID 等信息.
[xuliker/kde](https://github.com/xuliker/kde) 接着 patchwork 的 API, 提供了一个获取制定日期各个 patchwork 项目上邮件的脚本 [get_mm_patch_series_today.sh](https://github.com/xuliker/kde/blob/master/kernel_mailing_list/mm_community/get_mm_patch_series_today.sh)


## 2.5 Static code checking
-------

[Static code checking In the Linux kernel](https://elinux.org/images/d/d3/Bargmann.pdf)


## 2.6 topology 获取工具
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| NA | RRZE-HPC | [likwid:Performance monitoring and benchmarking suite](https://github.com/RRZE-HPC/likwid) | NA | ☐ | [github](https://github.com/RRZE-HPC/likwid) |

<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
