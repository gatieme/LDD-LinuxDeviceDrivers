Linux进程调度器的设计
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------- |:-------:|:-------:|:-------:|:-------:|:-------:|
| 2016-06-14 | [Linux-4.6](http://lxr.free-electrons.com/source/?v=4.6) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度](http://blog.csdn.net/gatieme/article/category/6225543) |



CFS负责处理普通非实时进程, 这类进程是我们linux中最普遍的进程, 今天我们把注意力转向CFS的虚拟时钟


#CFS虚拟时钟
-------


完全公平调度算法CFS依赖于虚拟时钟, 用以度量等待进程在完全公平系统中所能得到的CPU时间. 但是数据结构中任何地方都没有找到虚拟时钟. 这个是由于所有的必要信息都可以根据现存的实际时钟和每个进程相关的负荷权重推算出来.


##update_curr函数
-------

所有与虚拟时钟有关的计算都在update_curr中执行, 该函数在系统中各个不同地方调用, 包括周期性调度器在内.


首先, 该函数确定就绪队列的当前执行进程, 并获取主调度器就绪队列的实际时钟值, 该值在每个调度周期都会更新

```c

````
其中辅助函数[rq_of](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L248)用于确定与CFS就绪队列相关的struct rq实例, 其定义在[kernel/sched/fair.c, line 248](http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L248)

cfs_rq就绪队列中存储了指向就绪队列的实例,参见[kernel/sched/sched.h, line412](http://lxr.free-electrons.com/source/kernel/sched/sched.h#L412), 如下

```c
struct cfs_rq
{
	/*  ......  */
	struct rq *rq;  /* cpu runqueue to which this cfs_rq is attached */
	/*  ......  */
};
````

而rq_of返回了这个指向struct rq的指针

/* cpu runqueue to which this cfs_rq is attached */
static inline struct rq *rq_of(struct cfs_rq *cfs_rq)
{
        return cfs_rq->rq;
}


