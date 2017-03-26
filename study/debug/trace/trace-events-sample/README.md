http://blog.csdn.net/arethe/article/details/6293505]
http://lxr.free-electrons.com/source/Documentation/trace/tracepoints.txt
https://lwn.net/Articles/410200/


#1  驱动安装
-------


sudo insmod trace-events-sample.ko


#2  检查驱动
-------

cat /sys/kernel/debug/tracing/available_events | grep sample-trace

cd /sys/kernel/debug/tracing/events/sample-trace


#3  列出所有的events
-------

find /sys/kernel/debug/tracing/events -type d

cat /sys/kernel/debug/tracing/available_events

trace-cmd list

perf list 2>&1 | grep Tracepoint

#3  trace-cmd
-------

sudo trace-cmd reset

sudo trace-cmd record -e sample-trace:foo_with_template_print

sudo trace-cmd report

