#1    pid namespace框架
-------


一个框架的设计会考虑很多因素, 相信分析过 `Linux` 内核的读者来说会发现, 内核的大量数据结构被哈希表和链表链接起来, 最最主要的目的就是在于查找. 可想而知一个好的框架, 应该要考虑到检索速度，还有考虑功能的划分. 那么在 `PID` 框架中，需要考虑以下几个因素.

*    如何通过task_struct快速找到对应的pid
*    如何通过pid快速找到对应的task_struct
*    如何快速的分配一个唯一的pid

这些都是 `PID` 框架设计的时候需要考虑的一些基本的因素. 也正是这些因素将 `PID` 框架设计的愈加复杂.
原始的PID框架
先考虑的简单一点，一个进程对应一个pid

```cpp
struct task_struct
{
    .....
    pid_t pid;
    .....
}
```

是不是很easy，回到上文，看看是否符合PID框架的设计原则，通过task_struct找到pid，很方便，但是通过pid找到task_struct怎么办呢?

好吧，基于现在的这种结构肯定是无法满足需求的，那就继续改进吧。 

>注: 以上的这种设计来自与linux 2.4内核的设计

##1.2    引入 `pid` 位图
-------

如何分配一个唯一的 `pId` 呢, 连续递增? 

那么前面分配的进程如果结束了, 那么分配的 `pid` 就需要回收掉, 直到分配到 `pid` 的最大值, 然后从头再继续. 好吧, 这或许是个办法, 但是是不是需要标记一下那些pid可用呢?

到此为此这看起来似乎是个解决方案,但是考虑到这个方案是要放进内核,开发 `linux` 的那帮家伙肯定会想近一切办法进行优化的,的确如此,他们使用了 `pid` 位图,但是基本思想没有变,同样需要标记 `pid` 是否可用,只不过使用 `pid` 位图的方式更加节约内存. 想象一下, 通过将每一位设置为 `0` 或者是 `1`, 可以用来表示是否可用,第 `1` 位的 `0` 和 `1` 用来表示pid为1是否可用,以此类推. 到此为此一个看似还不错的 `pid` 框架设计完成了,下图是目前整个框架的整体效果.


```cpp
//  http://elixir.free-electrons.com/linux/latest/source/include/linux/pid_namespace.h#L14
struct pidmap {
        atomic_t nr_free;  //表示当前可用的pid个数, 这个bitmap还有多少位为0，就是说对应的pid没有被分配出去
        void *page;  //用来存放位图, 表示一段连续的内存空间，每位的0或1表示对应pid是否被分配
};
```

默认情况下 `pid` 最大是 `32768`, 那么默认正好是 `1` 页能保存下的 `pid` 使用情况, `linux` 默认一页的大小是 `4k=4*1024*8bit=32768`, 如果 `pid` 的最大值超过 `32768` 那么 `pidmap` 数组就用上了, 多个 `pidmap` 就是为了 `pid` 限制大于 `32768` 来设计的.

>[commit 95846ecf9dac pid: replace pid bitmap implementation with IDR API]() 引入了一种新的pid分配方式.
>


##1.3    引入PID类型后的PID框架
------


熟悉linux的读者应该知道一个进程不光光只有一个进程 `pid`, 还会有进程组 `id` ,还有会话 `id` ,(关于进程组和会话请参考(进程之间的关系)那么引入 `pid` 类型后,框架变成了下面这个样子,

```cpp
// http://elixir.free-electrons.com/linux/v4.15-rc7/source/include/linux/sched.h#L696
struct task_struct
{
    ....
    /* PID/PID hash table linkage. */
    struct pid_link            pids[PIDTYPE_MAX];
    struct list_head        thread_group;
    struct list_head        thread_node;
    ....
}
```

对于进程组 `id` 来说, 信号需要知道这这个 `id`, 通过这个 `id`, 可以实现对一组进程进行控制, 所以这个 `id` 出现在了 `signal` 这个结构体中.


```cpp
//  http://elixir.free-electrons.com/linux/v4.15-rc7/source/include/linux/sched/signal.h#L79
struct signal
{
    ....
    struct pid *leader_pid;
    struct pid *tty_old_pgrp;
    /* boolean value for session group leader */
    int leader;
    ....
}
```

所以直到现在来说框架还不是那么复杂, 但是有一个需要明确的就是无论是 `session id` 还是 `group id` 其实都不占用 `pid` 的资源, 因为 `session id` 是和领导进程组的组 `id` 相同,而 `group id` 则是和这个进程组中的领导进程的 `pid` 相同.


##1.4   引入进程PID命名空间后的PID框架
-------

随着内核不断的添加新的内核特性, 尤其是 `PID Namespace` 机制的引入, 这导致 `PID` 存在命名空间的概念, 并且命名空间还有层级的概念存在, 高级别的可以被低级别的看到, 这就导致高级别的进程有多个 `PID` ,比如说在默认命名空间下, 创建了一个新的命名空间,占且叫做 `level1`, 默认命名空间这里称之为 `level0`, 在 `level1` 中运行了一个进程在 `level1` 中这个进程的 `pid` 为 `1`, 因为高级别的 `pid namespace` 需要被低级别的 `pid namespace` 所看见,所以这个进程在 `level0` 中会有另外一个 `pid`, 为 `xxx`. 套用上面说到的 `pid` 位图的概念, 可想而知, 对于每一个 `pid namespace` 来说都应该有一个 `pidmap`, 上文中提到的 `level1` 进程有两个 `pid` 一个是 `1` ,另一个是 `xxx`,其中 `pid` 为 `1` 是在 `level1` 中的 `pidmap` 进行分配的, `pid` 为 `xxx` 则是在 `level0` 的 `pidmap` 中分配的. 下面这幅图是整个 `pidnamespace` 的一个框架 

.引入了PID命名空间后,一个pid就不仅仅是一个数值那么简单了,还要包含这个pid所在的命名空间,父命名空间,命名空间多对应的pidmap,命名空间的pid等等.因此内核对pid做了一个封装,封装成struct pid,一个名为pid的结构体,下面是其定义:

enum pid_type
{
    PIDTYPE_PID,
    PIDTYPE_PGID,
    PIDTYPE_SID,
    PIDTYPE_MAX
};
struct pid
{
    unsigned int level; //这个pid所在的层级
    /* lists of tasks that use this pid */
    struct hlist_head tasks[PIDTYPE_MAX]; //一个hash表,又三个表头,分别是pid表头,进程组id表头,会话id表头,后面再具体介绍
    struct upid numbers[1]; //这个pid对应的命名空间,一个pid不仅要包含当前的pid,还有包含父命名空间,默认大小为1,所以就处于根命名空间中
};
struct upid {              //包装命名空间所抽象出来的一个结构体
    int nr;                //pid在该命名空间中的pid数值
    struct pid_namespace *ns;      //对应的命名空间
    struct hlist_node pid_chain;    //通过pidhash将一个pid对应的所有的命名空间连接起来.
};
struct pid_namespace {
    struct kref kref;
    struct pidmap pidmap[PIDMAP_ENTRIES];  //上文说到的,一个pid命名空间应该有其独立的pidmap
    int last_pid;              //上次分配的pid
    unsigned int nr_hashed; 
    struct task_struct *child_reaper;  //这个pid命名空间对应的init进程,因为如果父进程挂了需要找养父啊,这里指明了该去找谁
    struct kmem_cache *pid_cachep;
    unsigned int level;        //所在的命名空间层次
    struct pid_namespace *parent;    //父命名空间,构建命名空间的层次关系
    struct user_namespace *user_ns;
    struct work_struct proc_work;
    kgid_t pid_gid;
    int hide_pid;
    int reboot; /* group exit code if this pidns was rebooted */
    unsigned int proc_inum;
};

//上面还有一些复杂的成员,这里的讨论占且用不到
引入了pid namespace后,的确变得很复杂了,多了很多看不懂的数据结构.进程如何和struct pid关联起来呢,内核为了统一管理pid,进程组id,会话id,将这三类id,进行了整合,也就是现在task_struct要和三个struct pid关联,还要区分struct pid的类型.所以内核又引入了中间结构将task_struct和pid进行了1:3的关联.其结构如下:
struct pid_link
{
    struct hlist_node node;
    struct pid *pid;
};
struct task_struct
{
    .............
    pid_t pid;
    struct pid_link pids[PIDTYPE_MAX];
    .............
}
struct pid
{
    unsigned int level; //这个pid所在的层级
    /* lists of tasks that use this pid */
    struct hlist_head tasks[PIDTYPE_MAX]; //一个hash表,又三个表头,分别是pid表头,进程组id表头,会话id表头,用于和task_struct进行关联
    struct upid numbers[1]; //这个pid对应的命名空间,一个pid不仅要包含当前的pid,还有包含父命名空间,默认大小为1,所以就处于根命名空间中
};

到此为止一个看起来已经比较完善的pid框架构建完成了,整个框架的效果如下: 

其中进程Ａ，Ｂ，Ｃ是一个进程组的，Ａ是组长进程，所以B，和C的task_struct结构体中的pid_link成员的node字段就被邻接到进程A对应的struct pid中的tasks[1]．struct upid通过pid_hash和pid数值关联了起来，这样就可以通过pid数值快速的找到所有命名空间的upid结构，numbers是一个struct pid的最后一个成员，利用可变数组来表示这个pid结构当前有多少个命名空间．
注: 2016/08/28 修正上图的一个错误，pid_hash的key并不是nr，应该是pid_hashfn(upid->nr, upid->ns)]唯一确定一个upid结构(具体可以参考pid_hashfn的实现)，这样就可以通过nr和ns来找到其对应的upid结构了。
为了验证我们这个框架,下面是一些PID相关的函数,通过函数的实现来验证下这个框架.
进程PID相关的API分析
获取三种类型的pid结构

static inline struct pid *task_pid(struct task_struct *task)
{
    return task->pids[PIDTYPE_PID].pid;
}
/*
* Without tasklist or rcu lock it is not safe to dereference
* the result of task_pgrp/task_session even if task == current,
* we can race with another thread doing sys_setsid/sys_setpgid.
*/
static inline struct pid *task_pgrp(struct task_struct *task)
{
    return task->group_leader->pids[PIDTYPE_PGID].pid;
}
static inline struct pid *task_session(struct task_struct *task)
{
    return task->group_leader->pids[PIDTYPE_SID].pid;
}

获取pid结构中的某一个名字空间的pid数值

pid_t pid_nr_ns(struct pid *pid, struct pid_namespace *ns)
{
    struct upid *upid;
    pid_t nr = 0;
    //判断传入的pid namespace层级是否符合要求
    if (pid && ns->level <= pid->level) {
        upid = &pid->numbers[ns->level]; //去到对应pid namespace的strut upid结构
        if (upid->ns == ns) //判断命名空间是否一致
            nr = upid->nr; //获取pid数值
    }
    return nr;
}

看看如何分配一个pid吧

struct pid *alloc_pid(struct pid_namespace *ns) //pid分配要依赖与pid namespace,也就是说这个pid是属于哪个pid namespace
{
    struct pid *pid;
    enum pid_type type; 
    int i, nr;
    struct pid_namespace *tmp;
    struct upid *upid;
    //分配一个pid结构
    pid = kmem_cache_alloc(ns->pid_cachep, GFP_KERNEL);
    if (!pid)
        goto out;
    tmp = ns;
    pid->level = ns->level; //初始化level
    //递归到上面的层级进行pid的分配和初始化
    for (i = ns->level; i >= 0; i--) {
        nr = alloc_pidmap(tmp); //从当前pid namespace开始知道全局pid namespace,每一个层级都分配一个pid
        if (nr < 0)
            goto out_free;
        pid->numbers[i].nr = nr; //初始化upid结构
        pid->numbers[i].ns = tmp;
        tmp = tmp->parent; //递归到父亲pid namespace
    }
    if (unlikely(is_child_reaper(pid))) {  //如果是init进程需要做一些设定,为其准备proc目录
        if (pid_ns_prepare_proc(ns))
            goto out_free;
    }
    get_pid_ns(ns);
    atomic_set(&pid->count, 1);
    for (type = 0; type < PIDTYPE_MAX; ++type)  //初始化pid中的hlist结构
        INIT_HLIST_HEAD(&pid->tasks[type]);
    upid = pid->numbers + ns->level;  //定位到当前namespace的upid结构
    spin_lock_irq(&pidmap_lock);
    if (!(ns->nr_hashed & PIDNS_HASH_ADDING)) 
        goto out_unlock;
    for ( ; upid >= pid->numbers; --upid) {
        hlist_add_head_rcu(&upid->pid_chain,
                &pid_hash[pid_hashfn(upid->nr, upid->ns)]); //建立pid_hash,让pid和pid namespace关联起来
        upid->ns->nr_hashed++;
    }
    spin_unlock_irq(&pidmap_lock);
out:
    return pid;
out_unlock:
    spin_unlock_irq(&pidmap_lock);
    put_pid_ns(ns);
out_free:
    while (++i <= ns->level)
        free_pidmap(pid->numbers + i);
    kmem_cache_free(ns->pid_cachep, pid);
    pid = NULL;
    goto out;
}
