进程虚拟地址空间
=======

| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.7](http://lxr.free-electrons.com/source/?v=4.7) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux内存管理](http://blog.csdn.net/gatieme/article/category/6225543) |

http://blog.sina.com.cn/s/blog_4b9eab320102vme0.html

http://blog.csdn.net/geekcome/article/details/7971463

http://blog.chinaunix.net/uid-29478572-id-4119158.html

https://yq.aliyun.com/articles/5862

http://blog.sina.com.cn/s/blog_4b9eab320102v9qn.html

http://blog.csdn.net/shanshanpt/article/details/19918171

http://blog.chinaunix.net/uid-21768364-id-2936798.html

http://www.linuxidc.com/Linux/2012-09/70700.htm

http://www.oschina.net/question/234345_47845

http://xuela-net.iteye.com/blog/1899566

http://blog.chinaunix.net/uid-22359610-id-1626525.html

http://www.360doc.com/content/15/0522/07/18252487_472352496.shtml

http://www.openstack.cn/?p=4756

http://weibo.com/p/1001603837806467238143

大家都知道TCP/IP协议栈现在是世界上最流行的网络协议栈，恐怕它的普及的最重要的原因就是其清晰的层次结构以及清晰定义的原语和接口。不仅使得上层应用开发者可以无需关心下层架构或者内部机制，从而相对透明的操作网络。这个明显的层次结构也可以在Linux内核的网络协议栈中观察到。
主要的参考文献是：Linux网络栈剖析（中文版）/Anatomy of Linux networking stack（英文原版）by Tim Jones.
以及：Linux内核2.4.x的网络接口结构
另外一些参考资料可以从这个页面找到：http://www.ecsl.cs.sunysb.edu/elibrary/linux/network/ （纽约州立大学石溪分校的页面）
Linux内核网络协议栈采用了如下的层次结构：

http://www.ibm.com/developerworks/cn/linux/l-linux-networking-stack/
http://www.ibm.com/developerworks/linux/library/l-linux-networking-stack/

http://gjiwwa.blu.livefilestore.com/y1p99O_6n29_58mKHdfUgbvzgKOsD5_7unXA5noegNcJ9Y2h3QMaQ-m0Rb9JGP1v_X4zI7-n7J6aqbOm1xr-HMy56E3YJZbWajO/Linux%E5%86%85%E6%A0%B82.4.x%E7%9A%84%E7%BD%91%E7%BB%9C%E6%8E%A5%E5%8F%A3%E7%BB%93%E6%9E%84.pdf?download

http://www.ecsl.cs.sunysb.edu/elibrary/linux/network/