---

title: Perfetto for linux 专题
date: 2020-11-21 22:40
author: gatieme
tags:
	- scheduler
	- linux
	- debug
categories:
        - 内核探秘
thumbnail:
blogexcerpt: <br>Perfetto 工具是 Android 下一代全新的统一的 trace 收集和分析框架, 在 Android 9.0(API级别28)或更高版本的设备上, 可以使用 System Tracing 的 System App 在设备上记录系统跟踪, 可以抓取平台和app的 trace 信息, 是用来取代 systrace 的, 但 systrace 由于历史原因也还会一直存在, 并且 Perfetto 抓取的 trace 文件也可以同样转换成 systrace 视图.

---

<br>

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 也欢迎大家提供一些其他好的调试工具以供收录, 鄙人在此谢谢啦

<br>

| 博文 | CSDN | GitHub | Blog |
|:---:|:----:|:------:|:----:|
| Perfetto for linux-使用 Perfetto 分析调度问题 | [CSDN-紫夜阑珊-青伶巷草](https://kernel.blog.csdn.net/article/details/110296324) | [`LDD-LinuxDeviceDrivers/study/debug/tools/perfetto/01-description`](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/debug/tools/perfetto/01-description) | [OS 内核实验室](https://gatieme.github.io/2020/11/21/2020/1121-0002-Perfetto_for_linux) |




<br>

*	本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*	采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*	基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.
