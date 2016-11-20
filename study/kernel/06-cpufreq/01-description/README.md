AderXCoding
=======

| CSDN | GitHub |
|:----:|:------:|
| [Aderstep--紫夜阑珊-青伶巷草](http://blog.csdn.net/gatieme) | [`AderXCoding/system/tools`](https://github.com/gatieme/AderXCoding/tree/master/system/tools) |


<br>
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作.
<br>


随着技术的发展, 我们对CPU的处理能力提出了越来越高的需求, 芯片厂家也对制造工艺不断地提升. 现在的主流PC处理器的主频已经在3GHz左右, 就算是智能手机的处理器也已经可以工作在1.5GHz以上, 可是我们并不是时时刻刻都需要让CPU工作在最高的主频上, 尤其是移动设备和笔记本电脑, 大部分时间里, CPU其实工作在轻负载状态下, 我们知道：主频越高, 功耗也越高. 为了节省CPU的功耗和减少发热, 我们有必要根据当前CPU的负载状态, 动态地提供刚好足够的主频给CPU. 在Linux中, 内核的开发者定义了一套框架模型来完成这一目的, 它就是CPUFreq系统

http://blog.csdn.net/droidphone/article/details/9346981

#1	cpufreq调频系统
-------


##1.1	cpufreq调频系统介绍
-------

现在的 `CPU` 耗电很大, 按需调节 `CPU` 频率对普通桌面及移动设备节能有重要的意义, 目前多数Linux发行版都已经默认启用了这个功能, 但在一些像数据库, 集群系统等特别需要 `CPU` 高性能的服务器环境中, `Linux` 提供的这种对 `CPU` 频率调节的功能对 `CPU` 性能使用受到一些限制, 不利于系统性能的更好发挥.

为此需要 `Linux` 系统管理员进行必要相关参数优化及设置来确保`CPU` 性能最大化. 本文将针对应用中常见的 `RedHat` 系列及`Debian/Ubuntu `系列中 `CPU` 频率调整工具的使用进行描述, 以方便 `Linux` 用户进行该方面工作的优化.

在 `Linux` 中, 内核的开发者定义了一套框架模型来完成 `CPU` 频率动态调整这一目的, 它就是 `CPU Freq` 系统. 尽管在各个 `Linux` 发行版中, 前端软件稍有差异, 但其最终都会通过 `Linux` 内核的 `CPU Freq` 系统来实现 `CPU` 频率动态调整的功能.



##1.2	cpu节能模式
-------


这些软件都会提供如下 `CPU` 模式(`governor`参数)


| 模式 | 名称 | 描述 |
|:---:|:---:|
| `ondemand` | 系统默认的超频模式, 按需调节 | 内核提供的功能, 不是很强大, 但有效实现了动态频率调节, 平时以低速方式运行, 当系统负载提高时候自动提高频率. 以这种模式运行不会因为降频造成性能降低, 同时也能节约电能和降低温度. 一般官方内核默认的方式都是 `ondemand` |
| `interactive` | 交互模式 | 直接上最高频率, 然后看 `CPU` 负荷慢慢降低, 比较耗电. `Interactive` 是以 `CPU` 排程数量而调整频率, 从而实现省电. `InteractiveX` 是以 `CPU` 负载来调整 `CPU` 频率, 不会过度把频率调低. 所以比 `Interactive` 反应好些, 但是省电的效果一般 |
| `conservative` | 保守模式 | 类似于 `ondemand`, 但调整相对较缓, 想省电就用他吧. `Google`官方内核, `kang`内核默认模式 |
| `smartass` | 聪明模式 | 是 `I` 和 `C` 模式的升级, 该模式在比 `interactive` 模式不差的响应的前提下会做到了更加省电 |
| `performance` | 性能模式 | 只有最高频率, 从来不考虑消耗的电量, 性能没得说, 但是耗电量 |
| powersave | 省电模式 | 通常以最低频率运行 |
| userspace | 用户自定义模式 | 系统将变频策略的决策权交给了用户态应用程序, 并提供了相应的接口供用户态应用程序调节CPU 运行频率使用. 也就是长期以来都在用的那个模式. 可以通过手动编辑配置文件进行配置 |
| Hotplug | 热插拔模式 | 类似于ondemand, 但是cpu会在关屏下尝试关掉一个cpu, 并且带有deep sleep, 比较省电. |


#2	使用接口
-------

##2.1	sysfs接口
-------


我们先从 `CPUFreq` 提供的 `sysfs` 接口入手, 直观地看看它提供了那些功能.

*	`/sys/devices/system/cpu` 总目录

所有与 `CPUFreq` 相关的 `sysfs` 接口都位于 : `/sys/devices/system/cpu` 下面

以下是我的电脑输出的结果

```cpp
cd /sys/devices/system/cpu
ls
```

![接口都位于 : `/sys/devices/system/cpu` 下面](ls-sys-devices-system-cpu.png)




我们可以看到, 4个 `cpu` 分别建立了一个自己的目录, 从 `cpu0` 到 `cpu3`, 我们再看看 `offline` 和 `online` 以及present的内容：

![内核对 `cpu` 的限制](online-offline-present.png)

| 接口文件 | 描述 | 我的输出 |
|:------:|:----:|:-------:|
| possible | 代表主板支持的cpu | 0-7, 表示主板有8个cpu插槽,  支持8个cpu, 编号0-7 |
| present | 则表示主板上已经安装的cpu | 0-3, 表示当前主板上安装了4个cpu, 编号0-3 |
| online | 代表目前正在工作的cpu | 0-3, 表示系统中有4个cpu在工作, 编号0~3 |
| offline | 代表目前被关掉的cpu | 并停掉没有工作的 `cpu`, 包括 `hotplug` 的 `cpu` 和未安装的 `cpu` |
| kernel_max | 代表内核支持的最大cpu数目 | 255, 系统最大支持255个cpu |


我们接着往下看 :

*	cpufreq/policyx 与 cpux/cpufreq

子目录 `cpufreq` 就是调频子系统的接口目录, 对应每个cpu单独有一个 `policyx` 的子目录, 其对应了编号为 `x` 的 `cpu` 的变频设置

```cpp
#-------
ls cpufreq
ls cpufreq/policy*

#-------
tree cpufreq
```

![`cpufreq` 子系统](ls-cpufreq.png)

同时每个安装的 `cpu` 都有一个单独的目录 `cpux`, `cpux`中存储了编号为 `x` 的 `cpu` 的所有信息. 其中其子目录 `cpufreq` 存储了其 `CPU-Freq` 变频的信息, 它是一个链接文件, 指向了我们之前提到的 `../cpufreq/policyx`


![`cpux` 的变频子系统](ls-cpux.png)


下面我们选择一个 `cpu` 的 `cpufreq` 目录进入, 比如 `cpu0`, 其目录路径如下
```cpp
/sys/devices/system/cpu/cpufreq/policy0

OR

/sys/devices/system/cpu/cpu0/cpufreq
```

这个目录下通常应该存在的文件, 以及他们在我的电脑上的值如下表所示

| 变量 | 值 |
|:---:|:--:|
| affected_cpus |  需要软件协调频率的 `CPU` 列表 | 0 |
| related_cpus | 需要软件或者硬件来协调频率的 `CPU` 列表 | 我的笔记本无此文件 |
| `scaling_available_frequencies` | 当前软件支持的频率值, 看看当前 `cpu` 支持那些频率 | 我的笔记本无此文件 |
| scaling_driver | 该 `CPU` 正在使用何种 `cpufreq driver`, 完成 `cpufreq` 控制的硬件驱动 |intel_pstate |
| scaling_setspeed |  如果用户选择了 `userspace` 作为当前的 `cpufreq` 策略 `governor`, 那么可以设置 `cpu` 工作主频率到某一个指定值. 只需要这个值在 `scaling_min_freq` 和 `scaling_max_freq` 之间即可 | _<unsupported_> |
| scaling_governor | 当前处理器的 变频策略 `governor` 类型 | powersave |
| scaling_available_governors | 当前内核中支持的所有 `cpufreq governor` 类型 | performance powersave |
| scaling_min_freq | 被 `governor` 和 `cpufreq` 核决定的当前 `CPU` 的最低工作频率, 该频率是内核认定的该 `CPU` 理论上的最低运行频率 | 800000 |
| scaling_curr_freq |  被 `governor` 和 `cpufreq` 核决定的当前 `CPU` 工作频率, 该频率是内核认定的该 `CPU` 理论上运行的主频率 |837656 |
| scaling_max_freq | 被 `governor` 和 `cpufreq` 核决定的当前 `CPU` 工作频率, 该频率是内核认定的该 `CPU` 理论上的最大运行频率 | 3000000 |
| cpuinfo_transition_latency | 定义了处理器在两个不同频率之间切换时所需要的时间(单位 : 纳秒) | 4294967295 |
| cpuinfo_min_freq | 通过硬件实际读取到的`cpu` 的最低运行频率 | 800000 |
| cpuinfo_cur_freq | 通过硬件实际读取到的`cpu` 的当前运行频率 | 837656 |
| cpuinfo_max_freq | 通过硬件实际读取到的`cpu` 的最高运行频率 | 3000000 |


![`cpu0` 的 `CPU-FREQ` 策略信息](cat-cpu0-freq.png)

所以, 我的 `cpu0` 的最低运行频率是$800000 = 800MHz$, 最高是$3000000 = 3.0GHz$, 目前正在运行的频率是$837656=837.656MHz$


*	前缀 `cpuinfo` 代表的是cpu硬件上支持的频率, 而 `scaling` 前缀代表的是可以通过 `CPUFreq` 系统用软件进行调节时所支持的频率.

*	`cpuinfo_cur_freq` 代表通过硬件实际上读到的频率值, 而 `scaling_cur_freq` 则是软件当前的设置值, 多数情况下这两个值是一致的, 但是也有可能因为硬件的原因, 有微小的差异. 因为`cat scaling_xxx_freq` 需要通过系统软件读取硬件的信息, 因此需要 `root` 权限

![ `cpuinfo_xxx_freq` 和 `scaling_xxx_freq` 的信息](cpu0-cpuinfo-scaling-freq.png)


*	`scaling_driver` 则会输出当前使用哪一个驱动来设置 `cpu` 的工作频率.

*	`scaling_available_governors` 则会输出当前可供选择的频率调节策略

*	当前系统选用 `CPU-FREQ` 策略用 `scaling_governor` 来表示

![`cpu0` 的频率调整策略 ](cpu-scaling_governor.png)


我们的系统中当前仅支持两个变频策略 高性能 `performance` 和 节能 `powersave`. 而当前使用的策略是 节能 `powersave`.



*	当我们选择 `userspace` 作为我们的调频 `governor` 时, 我们可以通过 `scaling_setspeed` 手工设置需要的频率. 

`powersave` 则简单地使用较低的工作频率进行运行 


![`powersave` 下 `cpu` 保持低频率运行 ](powersave-cpufreq.png)

`performance` 则一直选择较高的频率进行运行.


![`performance` 下 `cpu` 一直选择较高的频率运行](performance-cpufreq.png)

2.  软件架构
通过上一节的介绍, 我们可以大致梳理出CPUFreq系统的构成和工作方式. 首先, CPU的硬件特性决定了这个CPU的最高和最低工作频率, 所有的频率调整数值都必须在这个范围内, 它们用cpuinfo_xxx_freq来表示. 然后, 我们可以在这个范围内再次定义出一个软件的调节范围, 它们用scaling_xxx_freq来表示, 同时, 根据具体的硬件平台的不同, 我们还需要提供一个频率表, 这个频率表规定了cpu可以工作的频率值, 当然这些频率值必须要在cpuinfo_xxx_freq的范围内. 有了这些频率信息, CPUFreq系统就可以根据当前cpu的负载轻重状况, 合理地从频率表中选择一个合适的频率供cpu使用, 已达到节能的目的. 至于如何选择频率表中的频率, 这个要由不同的governor来实现, 目前的内核版本提供了5种governor供我们选择. 选择好适当的频率以后, 具体的频率调节工作就交由scaling_driver来完成. CPUFreq系统把一些公共的逻辑和接口代码抽象出来, 这些代码与平台无关, 也与具体的调频策略无关, 内核的文档把它称为CPUFreq Core（/Documents/cpufreq/core.txt）. 另外一部分, 与实际的调频策略相关的部分被称作cpufreq_policy, cpufreq_policy又是由频率信息和具体的governor组成, governor才是具体策略的实现者, 当然governor需要我们提供必要的频率信息, governor的实现最好能做到平台无关, 与平台相关的代码用cpufreq_driver表述, 它完成实际的频率调节工作. 最后, 如果其他内核模块需要在频率调节的过程中得到通知消息, 则可以通过cpufreq notifiers来完成. 由此, 我们可以总结出CPUFreq系统的软件结构如下：

3.  cpufreq_policy
一种调频策略的各种限制条件的组合称之为policy, 代码中用cpufreq_policy这一数据结构来表示：

[cpp] view plain copy
struct cpufreq_policy {  
          
        cpumask_var_t           cpus;     
        cpumask_var_t           related_cpus;   
  
        unsigned int            shared_type;   
                                                  
        unsigned int            cpu;      
        unsigned int            last_cpu;   
                                            
        struct cpufreq_cpuinfo  cpuinfo;  
  
        unsigned int            min;    /* in kHz */  
        unsigned int            max;    /* in kHz */  
        unsigned int            cur;      
                                           
        unsigned int            policy;   
        struct cpufreq_governor *governor;   
        void                    *governor_data;  
  
        struct work_struct      update;   
                                           
  
        struct cpufreq_real_policy      user_policy;  
  
        struct kobject          kobj;  
        struct completion       kobj_unregister;  
};  
其中的各个字段的解释如下：
cpus和related_cpus    这两个都是cpumask_var_t变量, cpus表示的是这一policy控制之下的所有还出于online状态的cpu, 而related_cpus则是online和offline两者的合集. 主要是用于多个cpu使用同一种policy的情况, 实际上, 我们平常见到的大多数系统中都是这种情况：所有的cpu同时使用同一种policy. 我们需要related_cpus变量指出这个policy所管理的所有cpu编号. 
cpu和last_cpu    虽然一种policy可以同时用于多个cpu, 但是通常一种policy只会由其中的一个cpu进行管理, cpu变量用于记录用于管理该policy的cpu编号, 而last_cpu则是上一次管理该policy的cpu编号（因为管理policy的cpu可能会被plug out, 这时候就要把管理工作迁移到另一个cpu上）. 
cpuinfo    保存cpu硬件所能支持的最大和最小的频率以及切换延迟信息. 
min/max/cur  该policy下的可使用的最小频率, 最大频率和当前频率. 
policy    该变量可以取以下两个值：CPUFREQ_POLICY_POWERSAVE和CPUFREQ_POLICY_PERFORMANCE, 该变量只有当调频驱动支持setpolicy回调函数的时候有效, 这时候由驱动根据policy变量的值来决定系统的工作频率或状态. 如果调频驱动（cpufreq_driver）支持target回调, 则频率由相应的governor来决定. 
governor和governor_data    指向该policy当前使用的cpufreq_governor结构和它的上下文数据. governor是实现该policy的关键所在, 调频策略的逻辑由governor实现. 
update    有时在中断上下文中需要更新policy, 需要利用该工作队列把实际的工作移到稍后的进程上下文中执行. 
user_policy    有时候因为特殊的原因需要修改policy的参数, 比如溫度过高时, 最大可允许的运行频率可能会被降低, 为了在适当的时候恢复原有的运行参数, 需要使用user_policy保存原始的参数（min, max, policy, governor）. 
kobj    该policy在sysfs中对应的kobj的对象. 
4.  cpufreq_governor
所谓的governor, 我把它翻译成：调节器. governor负责检测cpu的使用状况, 从而在可用的范围中选择一个合适的频率, 代码中它用cpufreq_governor结构来表示：

[cpp] view plain copy
struct cpufreq_governor {  
        char    name[CPUFREQ_NAME_LEN];  
        int     initialized;  
        int     (*governor)     (struct cpufreq_policy *policy,  
                                 unsigned int event);  
        ssize_t (*show_setspeed)        (struct cpufreq_policy *policy,  
                                         char *buf);  
        int     (*store_setspeed)       (struct cpufreq_policy *policy,  
                                         unsigned int freq);  
        unsigned int max_transition_latency; /* HW must be able to switch to 
                        next freq faster than this value in nano secs or we 
                        will fallback to performance governor */  
        struct list_head        governor_list;  
        struct module           *owner;  
};  

其中的各个字段的解释如下：
name    该governor的名字. 
initialized    初始化标志. 
governor    指向一个回调函数, CPUFreq Core会在不同的阶段调用该回调函数, 用于该governor的启动、停止、初始化、退出动作. 
list_head    所有注册的governor都会利用该字段链接在一个全局链表中, 以供系统查询和使用. 
5.  cpufreq_driver
上一节提到的gonvernor只是负责计算并提出合适的频率, 但是频率的设定工作是平台相关的, 这需要cpufreq_driver驱动来完成, cpufreq_driver的结构如下：

[cpp] view plain copy
struct cpufreq_driver {  
        struct module           *owner;  
        char                    name[CPUFREQ_NAME_LEN];  
        u8                      flags;  
       
        bool                    have_governor_per_policy;  
  
        /* needed by all drivers */  
        int     (*init)         (struct cpufreq_policy *policy);  
        int     (*verify)       (struct cpufreq_policy *policy);  
  
        /* define one out of two */  
        int     (*setpolicy)    (struct cpufreq_policy *policy);  
        int     (*target)       (struct cpufreq_policy *policy,  
                                 unsigned int target_freq,  
                                 unsigned int relation);  
  
        /* should be defined, if possible */  
        unsigned int    (*get)  (unsigned int cpu);  
  
        /* optional */  
        unsigned int (*getavg)  (struct cpufreq_policy *policy,  
                                 unsigned int cpu);  
        int     (*bios_limit)   (int cpu, unsigned int *limit);  
  
        int     (*exit)         (struct cpufreq_policy *policy);  
        int     (*suspend)      (struct cpufreq_policy *policy);  
        int     (*resume)       (struct cpufreq_policy *policy);  
        struct freq_attr        **attr;  
};  

相关的字段的意义解释如下：
name    该频率驱动的名字. 
init    回调函数, 该回调函数必须实现, CPUFreq Core会通过该回调函数对该驱动进行必要的初始化工作. 
verify    回调函数, 该回调函数必须实现, CPUFreq Core会通过该回调函数检查policy的参数是否被驱动支持. 
setpolicy/target    回调函数, 驱动必须实现这两个函数中的其中一个, 如果不支持通过governor选择合适的运行频率, 则实现setpolicy回调函数, 这样系统只能支持CPUFREQ_POLICY_POWERSAVE和CPUFREQ_POLICY_PERFORMANCE这两种工作策略. 反之, 实现target回调函数, 通过target回调设定governor所需要的频率. 
get    回调函数, 用于获取cpu当前的工作频率. 
getavg    回调函数, 用于获取cpu当前的平均工作频率. 
6.  cpufreq notifiers
CPUFreq的通知系统使用了内核的标准通知接口. 它对外提供了两个通知事件：policy通知和transition通知. 

policy通知用于通知其它模块cpu的policy需要改变, 每次policy改变时, 该通知链上的回调将会用不同的事件参数被调用3次, 分别是：

CPUFREQ_ADJUST    只要有需要, 所有的被通知者可以在此时修改policy的限制信息, 比如温控系统可能会修改在大允许运行的频率. 
CPUFREQ_INCOMPATIBLE    只是为了避免硬件错误的情况下, 可以在该通知中修改policy的限制信息. 
CPUFREQ_NOTIFY    真正切换policy前, 该通知会发往所有的被通知者. 
transition通知链用于在驱动实施调整cpu的频率时, 用于通知相关的注册者. 每次调整频率时, 该通知会发出两次通知事件：
CPUFREQ_PRECHANGE    调整前的通知. 
CPUFREQ_POSTCHANGE    完成调整后的通知. 
当检测到因系统进入suspend而造成频率被改变时, 以下通知消息会被发出：
CPUFREQ_RESUMECHANGE

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
<br>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作.
