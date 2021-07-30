---

title: 虚拟化 & KVM 子系统
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

2   **虚拟化子系统**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***

# 1 membarrier
-------




| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2017/10/19 | Raghavendra K T <raghavendra.kt@linux.vnet.ibm.com> | [membarrier: Provide register expedited private command](https://lore.kernel.org/patchwork/cover/843003) | 引入 MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED. | v6 ☑ 4.14-rc6 | [PatchWork v5](https://lore.kernel.org/patchwork/cover/835747)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6](https://lore.kernel.org/patchwork/cover/398912) |
| 2017/11/21 | Raghavendra K T <raghavendra.kt@linux.vnet.ibm.com> | [membarrier: Provide core serializing command](https://lore.kernel.org/patchwork/cover/843003) | 引入 MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED. | v6 ☑ 4.16-rc1 | [PatchWork v5](https://lore.kernel.org/patchwork/cover/835747)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6](https://lore.kernel.org/patchwork/cover/398912) |
| 2018/01/29 | Raghavendra K T <raghavendra.kt@linux.vnet.ibm.com> | [membarrier: Provide core serializing command](https://lore.kernel.org/patchwork/cover/843003) | 引入 MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED. | v6 ☑ 4.16-rc1 | [PatchWork v5](https://lore.kernel.org/patchwork/cover/835747)<br>*-*-*-*-*-*-*-* <br>[PatchWork v6](https://lore.kernel.org/patchwork/cover/398912) |


相关的文章介绍: [47].



# 2 RCU
-------


| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/06/01 | "Joel Fernandes (Google)" <joel@joelfernandes.org> | [Harden list_for_each_entry_rcu() and family](https://lore.kernel.org/patchwork/cover/1082845) | 本系列增加了一个新的内部函数rcu_read_lock_any_held(), 该函数在调用这些宏时检查reader节是否处于活动状态. 如果不存在reader section, 那么list_for_each_entry_rcu()的可选第四个参数可以是一个被计算的lockdep表达式(类似于rcu_dereference_check()的工作方式). . | RFC ☑ 5.4-rc1 | [PatchWork RFC,0/6](https://lore.kernel.org/patchwork/cover/1082845) |

<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
