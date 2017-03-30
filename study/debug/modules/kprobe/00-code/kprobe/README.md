https://www.ibm.com/developerworks/cn/linux/l-cn-systemtap1/


kprobe使用实例
本文附带的包包含了三个示例模块，kprobe-exam.c是kprobes使用示例，jprobe-exam.c是jprobes使用示例，kretprobe-exam.c是kretprobes使用示例，读者可以下载该包并执行如下指令来实验这些模块：
$ tar -jxvf kprobes-examples.tar.bz2
$ cd kprobes-examples
$ make
…
$ su -
…
$ insmod kprobe-example.ko
$ dmesg
…
$ rmmod kprobe-example
$ dmesg
…
$ insmod jprobe-example.ko
$ cat kprobe-example.c
$dmesg
…
$ rmmod jprobe-example
$ dmesg
…
$ insmod kretprobe-example.ko
$ dmesg
…
$ ls -Rla / > /dev/null & 
$ dmesg
…
$ rmmod kretprobe-example
$ dmesg
…
$
示例模块kprobe-exame.c探测schedule()函数，在探测点执行前后分别输出当前正在运行的进程、所在的CPU以及preempt_count()，当卸载该模块时将输出该模块运行时间以及发生的调度次数。这是该模块在作者系统上的输出：
kprobe registered
current task on CPU#1: swapper (before scheduling), preempt_count = 0
current task on CPU#1: swapper (after scheduling), preempt_count = 0
current task on CPU#0: insmod (before scheduling), preempt_count = 0
current task on CPU#0: insmod (after scheduling), preempt_count = 0
current task on CPU#1: klogd (before scheduling), preempt_count = 0
current task on CPU#1: klogd (after scheduling), preempt_count = 0
current task on CPU#1: klogd (before scheduling), preempt_count = 0
current task on CPU#1: klogd (after scheduling), preempt_count = 0
current task on CPU#1: klogd (before scheduling), preempt_count = 0
…
Scheduling times is 5918 during of 7655 milliseconds.
kprobe unregistered
示例模块jprobe-exam.c是一个jprobes探测例子，它示例了获取系统调用open的参数，但读者不要试图在实际的应用中这么使用，因为copy_from_user可能导致睡眠，而kprobe并不允许在探测点处理函数中这么做（请参看前面内容了解详细描述）。

这是该模块在作者系统上的输出：
Registered a jprobe.
process 'cat' call open('/etc/ld.so.cache', 0, 0)
process 'cat' call open('/lib/libc.so.6', 0, -524289)
process 'cat' call open('/usr/lib/locale/locale-archive', 32768, 1)
process 'cat' call open('/usr/share/locale/locale.alias', 0, 438)
process 'cat' call open('/usr/lib/locale/en_US.UTF-8/LC_CTYPE', 0, 0)
process 'cat' call open('/usr/lib/locale/en_US.utf8/LC_CTYPE', 0, 0)
process 'cat' call open('/usr/lib/gconv/gconv-modules.cache', 0, 0)
process 'cat' call open('kprobe-exam.c', 32768, 0)
…
process 'rmmod' call open('/etc/ld.so.cache', 0, 0)
process 'rmmod' call open('/lib/libc.so.6', 0, -524289)
process 'rmmod' call open('/proc/modules', 0, 438)
jprobe unregistered
示例模块kretprobe-exam.c是一个返回探测例子，它探测系统调用open并输出返回值小于0的情况。它也有意设置maxactive为1，以便示例丢失探测运行的情况，当然，只有系统并发运行多个sys_open才可能导致这种情况，因此，读者需要有SMP的系统或者有超线程支持才能看到这种情况。如果读者比较仔细，会看到在前面的命令有”ls -Rla / > /dev/null & ，那是专门为了导致出现丢失探测运行的。

这是该模块在作者系统上的输出：
Registered a return probe.
sys_open returns -2
sys_open returns -2
sys_open returns -2
sys_open returns -2
sys_open returns -2
sys_open returns -2
sys_open returns -2
sys_open returns -2
sys_open returns -2
sys_open returns -2
sys_open returns -2
sys_open returns -2
…
kretprobe unregistered
Missed 11 sys_open probe instances.
回页首
小结
本文详细地讲解了kprobe的方方面面并给出实际的例子代码帮助读者学习和使用kprobe。本文是系列文章“Linux下的一个全新的性能测量和调式诊断工具 -- Systemtap”之一，有兴趣的读者可以阅读该系列文章之二和三。