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

2   **调测子系统**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***

# X CGROUP CORE
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2023/04/03 | Yosry Ahmed <yosryahmed@google.com> | [cgroup: eliminate atomic rstat](https://patchwork.kernel.org/project/linux-mm/cover/20230403220337.443510-1-yosryahmed@google.com/) | 736574 | v1 ☐☑ | [LORE v1,0/5](https://lore.kernel.org/r/20230403220337.443510-1-yosryahmed@google.com) |



# 1 CPU
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2021/10/09 | "Pratik R. Sampat" <psampat@linux.ibm.com> | [kernel: Introduce CPU Namespace](https://www.phoronix.com/scan.php?page=news_item&px=Linux-CPU-Namespace) | CPU namespace | RFC ☐ | [LWN RFC, 0/5]](https://lwn.net/Articles/872507) |
| 2022/1/7 |  Josh Don | [cgroup: add cpu.stat_percpu](https://lkml.org/lkml/2022/1/7/833) | CPU namespace | RFC ☐ | [LORE 1/2]](https://lkml.org/lkml/2022/1/7/833) |

# 2 DEBUG
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2020/02/27 | Johannes Weiner <hannes@cmpxchg.org> | [mm: memcontrol: recursive memory.low protection](https://lore.kernel.org/all/20200227195606.46212-1-hannes@cmpxchg.org) | TODO | v1 ☐☑✓ | [LORE v1,0/3](https://lore.kernel.org/all/20200227195606.46212-1-hannes@cmpxchg.org) |
| 2022/08/26 | Lu Jialin <lujialin4@huawei.com> | [Introduce cgroup.top interface](https://lore.kernel.org/all/20220@huawei.com) | Cgroup 用于组织和管理资源可用的流程. 目前, 没有方便的工具来为每个子 CGroup 收集 Reuserce 使用信息, 这使得很难检测资源中断和调试资源问题. 为了克服这个问题, 我们提出了 cgroup. 顶部界面. 就像 top 命令一样, 用户能够轻松地收集资源使用信息, 允许用户检测和响应子 cgroup 中的资源中断. | v1 ☐☑✓ | [LORE v1,0/2](https://lore.kernel.org/all/20220826011503.103894-1-lujialin4@huawei.com) |

# 3 DRM
-------

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2022/11/11 | Tvrtko Ursulin <tvrtko.ursulin@linux.intel.com> | [DRM scheduling cgroup controller](https://lore.kernel.org/all/20221111104435.3152347-1-tvrtko.ursulin@linux.intel.com) | TODO | v2 ☐☑✓ | [LORE v2,0/13](https://lore.kernel.org/all/20221111104435.3152347-1-tvrtko.ursulin@linux.intel.com) |


<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
