---

title: HUAWEI ANDROID 优化
date: 2021-02-15 00:32
author: gatieme
tags:
    - linux
    - tools
categories:
        - 技术积累
thumbnail:
blogexcerpt: ANDROID

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

2   **HUAWEI**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***

# 1 SCHED
-------



# 2 MM
-------


## 2.1 Memory Leak Detect
-------

| 编号 | 特性 | CONFIG | 描述 | 链接 |
|:---:|:----:|:------:|:----:|:---:|
| 1 | mem_track | CONFIG_MM_PAGE_TRACE | 跟踪进程内存的分配, 通过 user_memory_dump() DUMP 系统的内存使用情况, 其中 mm_dump_tasks() DUMP 所有进程的 Memory 占用, mm_ion_memory_info() DUMP 系统的 ION 信息. mm_mem_stats_show() 则用于显示系统中各类内存(NR_TRACK)使用情况, mm_vmalloc_detail_show() 则显示了 vmalloc 的使用情况. | [mm/mem_track, P50_PRO](https://github.com/gatieme/MobileModels/tree/huawei/QCOM-5.4.86-JAD_AL00-HMOS2-snapdragon888-P50_PRO/mm/mem_track) |
| 2 | reserved_memory_debug | CONFIG_MEMORY_STATIC_FOOTPRINT | NA | [mm/mem_track/reserved_memory_debug.c](https://github.com/gatieme/MobileModels/blob/huawei/QCOM-5.4.86-JAD_AL00-HMOS2-snapdragon888-P50_PRO/mm/mem_track/reserved_memory_debug.c) |






<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
