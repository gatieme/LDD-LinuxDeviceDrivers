---

title: TIMER 系统
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

2   ** TIMER 子系统**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***



# The hierarchical timer pull model
-------



| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2024/02/21 | Anna-Maria Behnsen <anna-maria@linutronix.de> | [timers: Move from a push remote at enqueue to a pull at expiry model](https://git.kernel.org/pub/scm/linux/kernel/git/history/history.git/log/?id=b2cf7507e18649a30512515ec0ca89f26b2c2d0f) | [Linux 6.9 Has A Big Rework To CPU Timers - Some Power/Performance Benefits](https://www.phoronix.com/news/Linux-6.9-Timers-Rework) | v11 ☐☑✓ 6.9-rc1 | [2024/01/15, LORE v10,00/20](https://lore.kernel.org/all/20240115143743.27827-1-anna-maria@linutronix.de/)<br>*-*-*-*-*-*-*-* <br>[2024/02/21, LORE v11,0/20](https://lore.kernel.org/all/20240221090548.36600-1-anna-maria@linutronix.de) |
| 2023/06/06 | Thomas Gleixner <tglx@linutronix.de> | [posix-timers: Cure inconsistencies and the SIG_IGN mess](https://lore.kernel.org/all/20230606132949.068951363@linutronix.de) | TODO | v1 ☐☑✓ | [LORE v1,0/45](https://lore.kernel.org/all/20230606132949.068951363@linutronix.de)<br>*-*-*-*-*-*-*-* <br>[LORE v2,00/50](https://lore.kernel.org/lkml/20240410164558.316665885@linutronix.de) |


[hrtimers: Consolidate hrtimer initialization - Part 1](https://lore.kernel.org/all/20241031-hrtimer_setup_p1_v2-v2-0-23400656575a@linutronix.de)

| 时间 | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:---:|:----:|:---:|:----:|:---------:|:----:|
| 2024/10/28 | Nam Cao <namcao@linutronix.de> | [hrtimers: Switch to new hrtimer interface functions (1/5)](https://lore.kernel.org/all/cover.1729864615.git.namcao@linutronix.de) | TODO | v1 ☐☑✓ | [LORE v1,0/21](https://lore.kernel.org/all/cover.1729864615.git.namcao@linutronix.de) |
| 2024/10/28 | Nam Cao <namcao@linutronix.de> | [hrtimers: Switch to new hrtimer interface functions (2/5)](https://lore.kernel.org/all/cover.1729864823.git.namcao@linutronix.de) | TODO | v1 ☐☑✓ | [LORE v1,0/31](https://lore.kernel.org/all/cover.1729864823.git.namcao@linutronix.de) |
| 2024/10/28 | Nam Cao <namcao@linutronix.de> | [hrtimers: Switch to new hrtimer interface functions (3/5)](https://lore.kernel.org/all/cover.1729865232.git.namcao@linutronix.de) | TODO | v1 ☐☑✓ | [LORE v1,0/24](https://lore.kernel.org/all/cover.1729865232.git.namcao@linutronix.de) |
| 2024/10/28 | Nam Cao <namcao@linutronix.de> | [hrtimers: Switch to new hrtimer interface functions (4/5)](https://lore.kernel.org/all/cover.1729865485.git.namcao@linutronix.de) | TODO | v1 ☐☑✓ | [LORE v1,0/44](https://lore.kernel.org/all/cover.1729865485.git.namcao@linutronix.de) |
| 2024/10/28 | Nam Cao <namcao@linutronix.de> | [hrtimers: Switch to new hrtimer interface functions (5/5)](https://lore.kernel.org/all/cover.1729865740.git.namcao@linutronix.de) | TODO | v1 ☐☑✓ | [LORE v1,0/12](https://lore.kernel.org/all/cover.1729865740.git.namcao@linutronix.de) |







<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
