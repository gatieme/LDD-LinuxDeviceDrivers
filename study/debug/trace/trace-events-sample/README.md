#驱动安装
-------
http://blog.csdn.net/arethe/article/details/6293505]
http://lxr.free-electrons.com/source/Documentation/trace/tracepoints.txt

sudo insmod trace-events-sample.ko


#检查驱动
-------

cat /sys/kernel/debug/tracing/available_events | grep sample-trace

cd /sys/kernel/debug/tracing/events/sample-trace



#trace-cmd
-------

sudo trace-cmd reset

sudo trace-cmd record -e sample-trace:foo_with_template_print

sudo trace-cmd report

