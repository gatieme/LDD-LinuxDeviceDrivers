---

title: 使用 Hexo 搭建 GitHub Page 博客(一)
date: 2018-09-02 18:40
author: gatieme
tags: hexo
categories:
        - hexo
thumbnail: 
blogexcerpt: 博文摘要

---

| CSDN | GitHub | Hexo |
|:----:|:------:|:----:|
| [Aderstep--紫夜阑珊-青伶巷草](http://blog.csdn.net/gatieme) | [`AderXCoding/system/tools`](https://github.com/gatieme/AderXCoding/tree/master/system/tools) | [gatieme.github.io](https://gatieme.github.io) |

<br>

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 也欢迎大家提供一些其他好的调试工具以供收录, 鄙人在此谢谢啦

<br>

# 1 Perfetto 概述
-------

## 1.1 为什么需要 Perfetto
-------

Perfetto 工具是 Android 下一代全新的统一的 trace 收集和分析框架, 在 Android 9.0(API级别28)或更高版本的设备上, 可以使用 System Tracing 的 System App 在设备上记录系统跟踪
, 可以抓取平台和app的 trace 信息, 是用来取代 systrace 的, 但 systrace 由于历史原因也还会一直存在, 并且 Perfetto 抓取的 trace 文件也可以同样转换成 systrace 视图.

如果习惯用 systrace 的, 可以用 Perfetto UI 的 Open with legacy UI 转换成 systrace 视图来看


## 1.2 Perfetto 优点
-------

1.  支持 Android 和 Linux 上的全系统跟踪, 可以在线抓取长时间(可达数小时)的 trace, 子系统[跟踪处理器](https://perfetto.dev/docs/analysis/trace-processor) 专门设计用于将数小时的跟踪数据有效地保存到本地中, 并基于流行的SQLite查询引擎公开SQL查询接口支持 SQL 查询. 这样就可以在后台开启, 让它一直抓取 trace 了, 特别适用于那种复现概率很低, 又比较严重的性能问题. 


2.  Perfetto 具有很好的可扩展性, 它除了支持标准的 tracepoints(例如CPU调度信息, 内存信息等)之外, 还可以监听系统的多种信息, 比如 procfs 以及 sysfs 接口等; 还可以通过 atrace HAL 层扩展, 在 Android P当中, Google新增加了一个 atrace HAL 层, atrace 进程可以调用这个HAL的接口来获取当前的扩展信息, 比如添加用于记录电池和电量使用的统计信息, 程序的执行路径等. 相关代码可见 [Google 提交](https://android-review.googlesource.com/c/platform/frameworks/native/+/770934), 这样如果需要扩展 tracepoints 的话, 就可以按照 graphic 的示例添加即可.


3.  提供全新的 [Perfetto UI 网站](https://ui.perfetto.dev/), 用于打开的跟踪, 并通过浏览器在本地处理，不需要任何服务器端交互. 可以在上面通过选取开关的方式, 自动生成抓取 trace 的命令, 同时可以打开 trace 文件. 另外还集成了几种预定义的 trace 分析统计工具, 详情可见它的 Metrics and auditors 选项



> Perfetto 本身是一个框架, 关于它的架构和模块的详细介绍, 可以参考它的 [doc 网站](https://perfetto.dev/docs/), 它的源码可以参考 Android Source Tree 的 /external/perfetto 目录, 里面有很多的tools, 配置和脚本等, 可以拿来直接使用.


# 2 Android 上使用 perfetto
-------


# 3 服务器上使用 perfetto
-------

# 4 perfetto 的一些技巧
-------



https://www.jianshu.com/p/ab22238a9ab1
https://blog.csdn.net/sinat_20059415/article/details/106307905
https://blog.csdn.net/zhendong_hu/article/details/103858660
https://medium.com/kayvan-kaseb/android-app-performance-analysis-with-perfetto-c8707d879abd
https://perfetto.dev/docs

<br>

*	本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*	采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的. 

*	基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.
