Linux的Cgroup
=======



`CGroup`是`Control Groups`的缩写， 是`Linux`内核提供的一种可以限制、记录、隔离进程组(process groups)所使用的物力资源(如cpu memory i/o 等等)的机制. 2007年进入`Linux` 2.6.24内核, `CGroups`不是全新创造的, 它将进程管理从`cpuset`中剥离出来, 作者是`Google`的`Paul Menage.

`CGroups`也是`LXC`为实现虚拟化所使用的资源管理手段






#1	cgroup概述
-------


##1.1	cgroup的诞生
-------

cgroups（Control Groups）最初叫Process Container，由Google工程师（Paul Menage和Rohit Seth）于2006年提出，后来因为Container有多重含义容易引起误解，就在2007年更名为Control Groups，并被整合进Linux内核。顾名思义就是把进程放到一个组里面统一加以控制。官方的定义如下{![引自：https://www.kernel.org/doc/Documentation/cgroups/cgroups.txt]}

>cgroups是Linux内核提供的一种机制，这种机制可以根据特定的行为，把一系列系统任务及其子任务整合（或分隔）到按资源划分等级的不同组内，从而为系统资源管理提供一个统一的框架

通俗的来说，cgroups可以限制、记录、隔离进程组所使用的物理资源（包括：CPU、memory、IO等），为容器实现虚拟化提供了基本保证，是构建Docker等一系列虚拟化管理工具的基石。

对开发者来说，cgroups有如下四个有趣的特点：

*	cgroups的API以一个伪文件系统的方式实现，即用户可以通过文件操作实现cgroups的组织管理。

*	cgroups的组织管理操作单元可以细粒度到线程级别，用户态代码也可以针对系统分配的资源创建和销毁cgroups，从而实现资源再分配和管理。

*	所有资源管理的功能都以“subsystem（子系统）”的方式实现，接口统一。

*	子进程创建之初与其父进程处于同一个cgroups的控制组

本质上来说，cgroups是内核附加在程序上的一系列钩子（hooks），通过程序运行时对资源的调度触发相应的钩子以达到资源追踪和限制的目的。


##1.2	为什么要有cgroup
-------

`Linux`系统中经常有个需求就是希望能限制某个或者某些进程的分配资源. 也就是能完成一组容器的概念, 在这个容器中, 有分配好的特定比例的`cpu`时间, `I/O`时间, 可用内存大小等. 于是就出现了`cgroup`的概念, `cgroup`就是`controller group`, 最初由`google`的工程师提出, 后来被整合进`Linux`内核中。

`Cgroup`是将任意进程进行分组化管理的`Linux`内核功能。`cgroup`本身提供将进程进行分组化管理的功能和接口的基础结构.

而后的`Android`操作系统也就凭借着这个技术, 为每个应用程序分配不同的`cgroup`, 将每个程序进行隔离, 达到了一个应用程序不会影响其他应用程序环境的目的.

*	cgroup是control group的简称，它为Linux内核提供了一种任务聚集和划分的机制，通过一组参数集合将一些任务组织成一个或多个子系统

*	Cgroups最初由Google工程师提出，后来编进linux内核。

*	Cgroups是实现IaaS虚拟化(kvm、lxc等)，PaaS容器沙箱(Docker等)的资源管理控制部分的底层基础


##1.3	CGroup 支持的文件种类
-------

| 文件名 | R/W | 用途 |
|:-----:|:---:|:----:|
| Release_agent | RW | 删除分组时执行的命令，这个文件只存在于根分组 |
| Notify_on_release | RW | 设置是否执行 release_agent。为 1 时执行 |
| Tasks | RW | 属于分组的线程 TID 列表 |
| Cgroup.procs | R | 属于分组的进程 PID 列表。仅包括多线程进程的线程 leader 的 TID，这点与 tasks 不同 |
| Cgroup.event_control | RW | 监视状态变化和分组删除事件的配置文件 |



#2	CGroup的架构
-------

##2.1	cgroup的作用
-------

实现cgroups的主要目的是为不同用户层面的资源管理，提供一个统一化的接口。从单个进程的资源控制到操作系统层面的虚拟化。Cgroups提供了以下四大功能{![参照自：http://en.wikipedia.org/wiki/Cgroups]}。

| 功能 | 描述 |
| 资源限制（Resource Limitation） | cgroups可以对进程组使用的资源总额进行限制。如设定应用运行时使用内存的上限，一旦超过这个配额就发出OOM（Out of Memory） |
| 优先级分配（Prioritization） | 通过分配的CPU时间片数量及硬盘IO带宽大小，实际上就相当于控制了进程运行的优先级 |
| 资源统计（Accounting） | cgroups可以统计系统的资源使用量，如CPU使用时长、内存用量等等，这个功能非常适用于计费 |
| 进程控制（Control） | cgroups可以对进程组执行挂起、恢复等操作 |



##2.2	CGroup功能及组成
-------

`CGroup`是将任意进程进行分组化管理的`Linux`内核功能.

`CGroup`本身是提供将进程进行分组化管理的功能和接口的基础结构, `I/O`或内存的分配控制等具体的资源管理功能是通过这个功能来实现的. 这些具体的资源管理功能称为`CGroup`子系统或控制器.

`CGroup`子系统有控制内存的`Memory`控制器、控制进程调度的 `CPU`控制器等. 运行中的内核可以使用的`Cgroup`子系统由`/proc/cgroup`来确认.

`CGroup`提供了一个`CGroup`虚拟文件系统, 作为进行分组管理和各子系统设置的用户接口. 要使用`CGroup`, 必须挂载`CGroup`文件系统. 这时通过挂载选项指定使用哪个子系统.



*	task, 一个进程

*	control group控制族群，按照某种标准划分的进程组

*	hierarchy层级, control group可以形成树形的结构，有父节点，子节点，每个节点都是一个control group，子节点继承父节点的特定属性

*	subsystem子系统, 子系统就是资源控制器，每种子系统就是一个资源的分配器，比如`cpu`子系统是控制`cpu`时间分配的.


| 概念 | 名称 | 描述 |
|:---:|:---:|:----:|
| 任务 | task | 在 cgroups中, 任务就是系统的一个进程 |
| 控制族群 | control group | 控制族群就是一组按照某种标准划分的进程. Cgroups 中的资源控制都是以控制族群为单位实现。一个进程可以加入到某个控制族群，也从一个进程组迁移到另一个控制族群。一个进程组的进程可以使用 cgroups 以控制族群为单位分配的资源，同时受到 cgroups 以控制族群为单位设定的限制； |
| 层级 | hierarchy | 控制族群可以组织成hierarchical 的形式，既一颗控制族群树。控制族群树上的子节点控制族群是父节点控制族群的孩子，继承父控制族群的特定的属性；
| 子系统 | subsystem | 一个子系统就是一个资源控制器，比如 cpu 子系统就是控制 cpu 时间分配的一个控制器。子系统必须附加（attach）到一个层级上才能起作用，一个子系统附加到某个层级以后，这个层级上的所有控制族群都受到这个子系统的控制 |


##2.3	cgroup子系统介绍
-------


子系统`subsystem`是根据`cgroup`对任务的划分功能将任务按照一种指定的属性划分成的一个组, 主要用来实现资源的控制.

在`cgroup`中, 划分成的任务组以层次结构的形式组织, 多个子系统形成一个数据结构中类似多根树的结构.

`cgroup`包含了多个孤立的子系统, 每一个子系统代表单一的资源, 目前, `redhat`默认支持10个子系统, 但默认只挂载了8个子系统, `ubuntu 12.04`默认支持8个子系统，但默认只挂载了5个子系统, 当然也用户可以自定义子系统并进行挂载.

下面对每一个子系统进行简单的介绍：




| 子系统 | 描述 |
|:-----:|:---:|
| blkio | 设置限制每个块设备的输入输出控制。例如:磁盘，光盘以及usb等等 |
| cpu | 使用调度程序为cgroup任务提供cpu的访问 |
| cpuacct | 产生cgroup任务的cpu资源报告 |
| cpuset | 如果是多核心的cpu，这个子系统会为cgroup任务分配单独的cpu和内存 |
| devices | 允许或拒绝cgroup任务对设备的访问 |
| freezer | 暂停和恢复cgroup任务 |
| memory | 设置每个cgroup的内存限制以及产生内存资源报告 |
| net_cls | 标记每个网络包以供cgroup方便使用 |
| ns | 名称空间子系统 |
| perf_event | 增加了对每group的监测跟踪的能力，即可以监测属于某个特定的group的所有线程以及运行在特定CPU上的线程，此功能对于监测整个group非常有用，具体参见[perf_events: add support for per-cpu per-cgroup monitoring (v7)](http://lwn.net/Articles/421574) |


可以使用`lssubsys --all`来列出系统支持多少种子系统, `Ubuntu`系统中页可以使用`ls /sys/fs/cgroup/`来显示已经挂载的子系统

*	`lssubsys --all`来列出系统支持多少种子系统

![lssubsys --all](./images/lssubsys--all.png)


*	`Ubuntu`中可以使用`ls /sys/fs/cgroup/`来显示已经挂载的子系统, CentOS中位于`/cgroup/`中


![ls /sys/fs/cgroup](./images/ls_sys_fs_cgroup.png)



可以看到这里的几个子系统, 比如`cpu`是控制`cpu`时间片的, `memory`是控制内存使用的


过去有一段时间，内核开发者甚至把namespace也作为一个cgroups的subsystem加入进来，也就是说cgroups曾经甚至还包含了资源隔离的能力。但是资源隔离会给cgroups带来许多问题，如PID在循环出现的时候cgroup却出现了命名冲突、cgroup创建后进入新的namespace导致脱离了控制等等{![详见：https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/?id=a77aea92010acf54ad785047234418d5d68772e2]}，所以在2011年就被移除了。



##2.4	相互关系
-------


1.	每次在系统中创建新层级时, 该系统中的所有任务都是那个层级的默认`cgroup`(我们称之为`root cgroup`, 此`cgroup`在创建层级时自动创建，后面在该层级中创建的 `cgroup`都是此`cgroup`的后代)的初始成员;

2.	一个子系统最多只能附加到一个层级；

3.	一个层级可以附加多个子系统；

4.	一个任务可以是多个`cgroup`的成员，但是这些 `cgroup`必须在不同的层级；

5.	系统中的进程(任务)创建子进程(任务)时, 该子任务自动成为其父进程所在`cgroup`的成员。然后可根据需要将该子任务移动到不同的`cgroup`中，但开始时它总是继承其父任务的`cgroup`


![cgroup的层次结构](./images/cgroup_level.png)

#3	CGroup应用架构
-------

##3.1	CGroup应用架构
-------


CGroup 技术可以被用来在操作系统底层限制物理资源，起到 Container 的作用。图中每一个 JVM 进程对应一个 Container Cgroup 层级，通过 CGroup 提供的各类子系统，可以对每一个 JVM 进程对应的线程级别进行物理限制，这些限制包括 CPU、内存等等许多种类的资源。下一部分会具体对应用程序进行 CPU 资源隔离进行演示。

![CGroup应用架构](./images/cgroup_application.png)


##3.2	组织结构
-------

大家在namespace技术的讲解中已经了解到，传统的Unix进程管理，实际上是先启动init进程作为根节点，再由init节点创建子进程作为子节点，而每个子节点由可以创建新的子节点，如此往复，形成一个树状结构。而cgroups也是类似的树状结构，子节点都从父节点继承属性。

它们最大的不同在于，系统中cgroup构成的hierarchy可以允许存在多个。如果进程模型是由init作为根节点构成的一棵树的话，那么cgroups的模型则是由多个hierarchy构成的森林。这样做的目的也很好理解，如果只有一个hierarchy，那么所有的task都要受到绑定其上的subsystem的限制，会给那些不需要这些限制的task造成麻烦。

##3.3	基本规则
-------

了解了cgroups的组织结构，我们再来了解cgroup、task、subsystem以及hierarchy四者间的相互关系及其基本规则{![参照自：https://access.redhat.com/documentation/en-US/RedHatEnterpriseLinux/6/html/ResourceManagementGuide/sec-RelationshipsBetweenSubsystemsHierarchiesControlGroupsandTasks.html]}。

*	**规则1**
	同一个hierarchy可以附加一个或多个subsystem。如下图1，cpu和memory的subsystem附加到了一个hierarchy

![同一个hierarchy可以附加一个或多个subsystem](./images/guize1.png)

*	**规则2**
	一个subsystem可以附加到多个hierarchy，当且仅当这些hierarchy只有这唯一一个subsystem。如下图2，小圈中的数字表示subsystem附加的时间顺序，CPU subsystem附加到hierarchy A的同时不能再附加到hierarchy B，因为hierarchy B已经附加了memory subsystem。如果hierarchy B与hierarchy A状态相同，没有附加过memory subsystem，那么CPU subsystem同时附加到两个hierarchy是可以的。

![图2 一个已经附加在某个hierarchy上的subsystem不能附加到其他含有别的subsystem的hierarchy上](./images/guize2.png)


*	**规则3**
系统每次新建一个hierarchy时，该系统上的所有task默认构成了这个新建的hierarchy的初始化cgroup，这个cgroup也称为root cgroup。对于你创建的每个hierarchy，task只能存在于其中一个cgroup中，即一个task不能存在于同一个hierarchy的不同cgroup中，但是一个task可以存在在不同hierarchy中的多个cgroup中。如果操作时把一个task添加到同一个hierarchy中的另一个cgroup中，则会从第一个cgroup中移除。在下图3中可以看到，httpd进程已经加入到hierarchy A中的/cg1而不能加入同一个hierarchy中的/cg2，但是可以加入hierarchy B中的/cg3。实际上不允许加入同一个hierarchy中的其他cgroup野生为了防止出现矛盾，如CPU subsystem为/cg1分配了30%，而为/cg2分配了50%，此时如果httpd在这两个cgroup中，就会出现矛盾

![图3 一个task不能属于同一个hierarchy的不同cgroup](./images/guize3.png)


*	**规则4**
进程（task）在fork自身时创建的子任务（child task）默认与原task在同一个cgroup中，但是child task允许被移动到不同的cgroup中。即fork完成后，父子进程间是完全独立的。如下图4中，小圈中的数字表示task 出现的时间顺序，当httpd刚fork出另一个httpd时，在同一个hierarchy中的同一个cgroup中。但是随后如果PID为4840的httpd需要移动到其他cgroup也是可以的，因为父子任务间已经独立。总结起来就是：初始化时子任务与父任务在同一个cgroup，但是这种关系随后可以改变

![图4 刚fork出的子进程在初始状态与其父进程处于同一个cgroup](./images/guize4.png)



#4	cgroups实现方式及工作原理简介
-------


##4.1	cgroups实现结构讲解
-------


cgroups的实现本质上是给系统进程挂上钩子（hooks），当task运行的过程中涉及到某个资源时就会触发钩子上所附带的subsystem进行检测，最终根据资源类别的不同使用对应的技术进行资源限制和优先级分配。那么这些钩子又是怎样附加到进程上的呢？下面我们将对照结构体的图表一步步分析，请放心，描述代码的内容并不多。


![图5 cgroups相关结构体一览](./images/struct.png)

Linux中管理task进程的数据结构为task_struct（包含所有进程管理的信息），其中与cgroup相关的字段主要有两个，一个是css_set *cgroups，表示指向css_set（包含进程相关的cgroups信息）的指针，一个task只对应一个css_set结构，但是一个css_set可以被多个task使用。另一个字段是list_head cg_list，是一个链表的头指针，这个链表包含了所有的链到同一个css_set的task进程（在图中使用的回环箭头，均表示可以通过该字段找到所有同类结构，获得信息）

每个css_set结构中都包含了一个指向cgroup_subsys_state（包含进程与一个特定子系统相关的信息）的指针数组。cgroup_subsys_state则指向了cgroup结构（包含一个cgroup的所有信息），通过这种方式间接的把一个进程和cgroup联系了起来

![图6 从task结构开始找到cgroup结构](./images/cgroup_subsys_state.png)


另一方面，cgroup结构体中有一个list_head css_sets字段，它是一个头指针，指向由cg_cgroup_link（包含cgroup与task之间多对多关系的信息，后文还会再解释）形成的链表。由此获得的每一个cg_cgroup_link都包含了一个指向css_set *cg字段，指向了每一个task的css_set。css_set结构中则包含tasks头指针，指向所有链到此css_set的task进程构成的链表。至此，我们就明白如何查看在同一个cgroup中的task有哪些了，如下图

![图7 cglink多对多双向查询](./images/css_sets.png)


细心的读者可能已经发现，css_set中也有指向所有cg_cgroup_link构成链表的头指针，通过这种方式也能定位到所有的cgroup，这种方式与图1中所示的方式得到的结果是相同的。

那么为什么要使用cg_cgroup_link结构体呢？因为task与cgroup之间是多对多的关系。熟悉数据库的读者很容易理解，在数据库中，如果两张表是多对多的关系，那么如果不加入第三张关系表，就必须为一个字段的不同添加许多行记录，导致大量冗余。通过从主表和副表各拿一个主键新建一张关系表，可以提高数据查询的灵活性和效率。

而一个task可能处于不同的cgroup，只要这些cgroup在不同的hierarchy中，并且每个hierarchy挂载的子系统不同；另一方面，一个cgroup中可以有多个task，这是显而易见的，但是这些task因为可能还存在在别的cgroup中，所以它们对应的css_set也不尽相同，所以一个cgroup也可以对应多个·css_set。

在系统运行之初，内核的主函数就会对root cgroups和css_set进行初始化，每次task进行fork/exit时，都会附加（attach）/分离（detach）对应的css_set。

综上所述，添加cg_cgroup_link主要是出于性能方面的考虑，一是节省了task_struct结构体占用的内存，二是提升了进程fork()/exit()的速度。


![图8 css_set与hashtable关系](./images/cg_cgroup_link.png)

当task从一个cgroup中移动到另一个时，它会得到一个新的css_set指针。如果所要加入的cgroup与现有的cgroup子系统相同，那么就重复使用现有的css_set，否则就分配一个新css_set。所有的css_set通过一个哈希表进行存放和查询，如上图8中所示，hlist_node hlist就指向了css_set_table这个hash表。

同时，为了让cgroups便于用户理解和使用，也为了用精简的内核代码为cgroup提供熟悉的权限和命名空间管理，内核开发者们按照Linux 虚拟文件系统转换器（VFS：Virtual Filesystem Switch）的接口实现了一套名为cgroup的文件系统，非常巧妙地用来表示cgroups的hierarchy概念，把各个subsystem的实现都封装到文件系统的各项操作中。有兴趣的读者可以在网上搜索并阅读VFS的相关内容，在此就不赘述了。

定义子系统的结构体是cgroup_subsys，在图9中可以看到，cgroup_subsys中定义了一组函数的接口，让各个子系统自己去实现，类似的思想还被用在了cgroup_subsys_state中，cgroup_subsys_state并没有定义控制信息，只是定义了各个子系统都需要用到的公共信息，由各个子系统各自按需去定义自己的控制信息结构体，最终在自定义的结构体中把cgroup_subsys_state包含进去，然后内核通过container_of（这个宏可以通过一个结构体的成员找到结构体自身）等宏定义来获取对应的结构体

![图9 cgroup子系统结构体](./images/cgroup_subsys.png)

##4.2	基于cgroups实现结构的用户层体现
-------


了解了cgroups实现的代码结构以后，再来看用户层在使用cgroups时的限制，会更加清晰。

在实际的使用过程中，你需要通过挂载（mount）cgroup文件系统新建一个层级结构，挂载时指定要绑定的子系统，缺省情况下默认绑定系统所有子系统。把cgroup文件系统挂载（mount）上以后，你就可以像操作文件一样对cgroups的hierarchy层级进行浏览和操作管理（包括权限管理、子文件管理等等）。除了cgroup文件系统以外，内核没有为cgroups的访问和操作添加任何系统调用。

如果新建的层级结构要绑定的子系统与目前已经存在的层级结构完全相同，那么新的挂载会重用原来已经存在的那一套（指向相同的css_set）。否则如果要绑定的子系统已经被别的层级绑定，就会返回挂载失败的错误。如果一切顺利，挂载完成后层级就被激活并与相应子系统关联起来，可以开始使用了。

目前无法将一个新的子系统绑定到激活的层级上，或者从一个激活的层级中解除某个子系统的绑定。

当一个顶层的cgroup文件系统被卸载（umount）时，如果其中创建后代cgroup目录，那么就算上层的cgroup被卸载了，层级也是激活状态，其后代cgoup中的配置依旧有效。只有递归式的卸载层级中的所有cgoup，那个层级才会被真正删除。

层级激活后，/proc目录下的每个task PID文件夹下都会新添加一个名为cgroup的文件，列出task所在的层级，对其进行控制的子系统及对应cgroup文件系统的路径。

一个cgroup创建完成，不管绑定了何种子系统，其目录下都会生成以下几个文件，用来描述cgroup的相应信息。同样，把相应信息写入这些配置文件就可以生效，内容如下。

| 配置文件 | 描述 |
|:-------:|:---:|
| tasks | 这个文件中罗列了所有在该cgroup中task的PID。该文件并不保证task的PID有序，把一个task的PID写到这个文件中就意味着把这个task加入这个cgroup中 |
| cgroup.procs | 这个文件罗列所有在该cgroup中的线程组ID。该文件并不保证线程组ID有序和无重复。写一个线程组ID到这个文件就意味着把这个组中所有的线程加到这个cgroup中 |
| notify_on_release | 填0或1，表示是否在cgroup中最后一个task退出时通知运行release agent，默认情况下是0，表示不运行 |
| release_agent | 指定release agent执行脚本的文件路径（该文件在最顶层cgroup目录中存在），在这个脚本通常用于自动化umount无用的cgroup |

除了上述几个通用的文件以外，绑定特定子系统的目录下也会有其他的文件进行子系统的参数配置。

在创建的hierarchy中创建文件夹，就类似于fork中一个后代cgroup，后代cgroup中默认继承原有cgroup中的配置属性，但是你可以根据需求对配置参数进行调整。这样就把一个大的cgroup系统分割成一个个嵌套的、可动态变化的“软分区”。


#5	cgroups的使用方法简介
-------


#5.1	如何安装cgroup
-------


`Ubuntu`中安装`cgroup`, 对应的包为`cgroup-bin`


```cpp
sudo apt-get install cgroup-bin
```


![`sudo apt-get install cgroup-bin`](./images/install_cgroup-bin.png)


CentOS中对应的包是`libcgroup`

```cpp
yum install libcgroup    #  安装libcgroup
service cgconfig start   #  开启cgroups服务
chkconfig cgconfig on    #  开机启动
```

如何为`cgroup`分配限制的资源

首先明白下, 是先挂载子系统, 然后才有`control group`的. 意思就是比如想限制某些进程的资源, 那么, 我会先挂载`memory`子系统, 然后在`memory`子系统中创建一个cgroup节点, 在这个节点中, 将需要控制的进程`id`写入, 并且将控制的属性写入.



>说明：也许你在其他文章中看到的cgroups工具库教程，会在/etc目录下生成一些初始化脚本和配置文件，默认的cgroup配置文件为/etc/cgconfig.conf，但是因为存在使LXC无法运行的bug，所以在新版本中把这个配置移除了，详见 https://bugs.launchpad.net/ubuntu/+source/libcgroup/+bug/1096771


##5.2	查询cgroup及子系统挂载状态
-------


在挂载子系统之前，可能你要先检查下目前子系统的挂载状态，如果子系统已经挂载，根据第4节中讲的规则2，你就无法把子系统挂载到新的hierarchy，此时就需要先删除相应hierarchy或卸载对应子系统后再挂载。

*	查看所有的cgroup：lscgroup

*	查看所有支持的子系统：lssubsys -a

*	查看所有子系统挂载的位置： lssubsys –m

*	查看单个子系统（如memory）挂载位置：lssubsys –m memory


##5.3	创建hierarchy层级并挂载子系统
-------

在组织结构与规则一节中我们提到了hierarchy层级和subsystem子系统的关系，我们知道使用cgroup的最佳方式是：为想要管理的每个或每组资源创建单独的cgroup层级结构。而创建hierarchy并不神秘，实际上就是做一个标记，通过挂载一个tmpfs{![基于内存的临时文件系统，详见：http://en.wikipedia.org/wiki/Tmpfs]}文件系统，并给一个好的名字就可以了，系统默认挂载的cgroup就会进行如下操作

>mount -t tmpfs cgroups /sys/fs/cgroup

其中-t即指定挂载的文件系统类型，其后的cgroups是会出现在mount展示的结果中用于标识，可以选择一个有用的名字命名，最后的目录则表示文件的挂载点位置。

挂载完成tmpfs后就可以通过mkdir命令创建相应的文件夹。

>mkdir /sys/fs/cgroup/cg1

再把子系统挂载到相应层级上，挂载子系统也使用mount命令，语法如下。

>mount -t cgroup -o subsystems name /cgroup/name

其中subsystems是使用,(逗号)分开的子系统列表, name是层级名称. 具体我们以挂载cpu和memory的子系统为例，命令如下

>mount –t cgroup –o cpu,memory cpu_and_mem /sys/fs/cgroup/cg1

从mount命令开始，-t后面跟的是挂载的文件系统类型，即cgroup文件系统。-o后面跟要挂载的子系统种类如cpu、memory，用逗号隔开，其后的cpu_and_mem不被cgroup代码的解释，但会出现在/proc/mounts里，可以使用任何有用的标识字符串。最后的参数则表示挂载点的目录位置。

>说明：如果挂载时提示mount: agent already mounted or /cgroup busy，则表示子系统已经挂载，需要先卸载原先的挂载点，通过第二条中描述的命令可以定位挂载点。

##5.4	卸载cgroup
-------

目前cgroup文件系统虽然支持重新挂载，但是官方不建议使用，重新挂载虽然可以改变绑定的子系统和release agent，但是它要求对应的hierarchy是空的并且release_agent会被传统的fsnotify（内核默认的文件系统通知）代替，这就导致重新挂载很难生效，未来重新挂载的功能可能会移除。你可以通过卸载，再挂载的方式处理这样的需求。

卸载cgroup非常简单，你可以通过cgdelete命令，也可以通过rmdir，以刚挂载的cg1为例，命令如下。

>rmdir /sys/fs/cgroup/cg1

rmdir执行成功的必要条件是cg1下层没有创建其它cgroup，cg1中没有添加任何task，并且它也没有被别的cgroup所引用。

cgdelete cpu,memory:/ 使用cgdelete命令可以递归的删除cgroup及其命令下的后代cgroup，并且如果cgroup中有task，那么task会自动移到上一层没有被删除的cgroup中，如果所有的cgroup都被删除了，那task就不被cgroups控制。但是一旦再次创建一个新的cgroup，所有进程都会被放进新的cgroup中


##5.5	设置cgroups参数
-------

设置cgroups参数非常简单，直接对之前创建的cgroup对应文件夹下的文件写入即可，举例如下。

设置task允许使用的cpu为0和1. echo 0-1 > /sys/fs/cgroup/cg1/cpuset.cpus
使用cgset命令也可以进行参数设置，对应上述允许使用0和1cpu的命令为：

cgset -r cpuset.cpus=0-1 cpu,memory:/

##5.6	添加task到cgroup
-------

通过文件操作进行添加 echo [PID] > /path/to/cgroup/tasks 上述命令就是把进程ID打印到tasks中，如果tasks文件中已经有进程，需要使用">>"向后添加。

通过cgclassify将进程添加到cgroup cgclassify -g subsystems:path_to_cgroup pidlist 这个命令中，subsystems指的就是子系统（如果使用man命令查看，可能也会使用controllers表示）​​​，如果mount了多个，就是用","隔开的子系统名字作为名称，类似cgset命令。

通过cgexec直接在cgroup中启动并执行进程 cgexec -g subsystems:path_to_cgroup command arguments command和arguments就表示要在cgroup中执行的命令和参数。cgexec常用于执行临时的任务。

##5.7	权限管理
-------

与文件的权限管理类似，通过chown就可以对cgroup文件系统进行权限管理。

>chown uid:gid /path/to/cgroup

uid和gid分别表示所属的用户和用户组。


#6	subsystem配置参数用法
-------

##6.1	blkio - BLOCK IO资源控制
-------


**限额类**

限额类是主要有两种策略，一种是基于完全公平队列调度(CFQ：Completely Fair Queuing)的按权重分配各个cgroup所能占用总体资源的百分比，好处是当资源空闲时可以充分利用，但只能用于最底层节点cgroup的配置；另一种则是设定资源使用上限，这种限额在各个层次的cgroup都可以配置，但这种限制较为生硬，并且容器之间依然会出现资源的竞争。

*	按比例分配块设备IO资源

| 参数 | 功能 |
|:---:|:----:|
| blkio.weight | 填写100-1000的一个整数值，作为相对权重比率，作为通用的设备分配比 |
| blkio.weight_device | 针对特定设备的权重比，写入格式为device_types:node_numbers weight，空格前的参数段指定设备，weight参数与blkio.weight相同并覆盖原有的通用分配比 |


>查看一个设备的device_types:node_numbers可以使用：ls -l /dev/DEV，看到的用逗号分隔的两个数字就是
>
>有的文章也称之为major_number:minor_number

*	控制IO读写速度上限


| 参数 | 功能 |
|:---:|:----:|
| blkio.throttle.read_bps_device | 按每秒读取块设备的数据量设定上限，格式device_types:node_numbers bytes_per_second |
| blkio.throttle.write_bps_device | 按每秒写入块设备的数据量设定上限，格式device_types:node_numbers bytes_per_second |
| blkio.throttle.read_iops_device | 按每秒读操作次数设定上限，格式device_types:node_numbers operations_per_second |
| blkio.throttle.write_iops_device | 按每秒写操作次数设定上限，格式device_types:node_numbers operations_per_second |

*	针对特定操作(read, write, sync, 或async)设定读写速度上限

| 参数 | 功能 |
|:---:|:----:|
| blkio.throttle.io_serviced | 针对特定操作按每秒操作次数设定上限，格式device_types:node_numbers operation operations_per_second |
| blkio.throttle.io_service_bytes | 针对特定操作按每秒数据量设定上限，格式device_types:node_numbers operation bytes_per_second |

**统计与监控**

以下内容都是只读的状态报告，通过这些统计项更好地统计、监控进程的io情况

| 参数 | 功能 |
|:---:|:----:|
| blkio.reset_stats | 重置统计信息，写入一个int值即可 |
| blkio.time | 统计cgroup对设备的访问时间，按格式device_types:node_numbers milliseconds读取信息即可，以下类似 |
| blkio.io_serviced | 统计cgroup对特定设备的IO操作（包括read、write、sync及async）次数，格式device_types:node_numbers operation number |
| blkio.sectors | 统计cgroup对设备扇区访问次数，格式 device_types:node_numbers sector_count |
| blkio.io_service_bytes | 统计cgroup对特定设备IO操作（包括read、write、sync及async）的数据量，格式device_types:node_numbers operation bytes |
| blkio.io_queued | 统计cgroup的队列中对IO操作（包括read、write、sync及async）的请求次数，格式number operation |
| blkio.io_service_time | 统计cgroup对特定设备的IO操作（包括read、write、sync及async）时间(单位为ns)，格式device_types:node_numbers operation time |
| blkio.io_merged | 统计cgroup 将 BIOS 请求合并到IO操作（包括read、write、sync及async）请求的次数，格式number operation |
| blkio.io_wait_time | 统计cgroup在各设备中各类型IO操作（包括read、write、sync及async）在队列中的等待时间​(单位ns)，格式 |
| device_types:node_numbers operation time __blkio.__recursive_* | 各类型的统计都有一个递归版本，Docker中使用的都是这个版本。获取的数据与非递归版本是一样的，但是包括cgroup所有层级的监控数据 |


##6.2	cpu - CPU资源控制

CPU资源的控制也有两种策略，一种是完全公平调度 （CFS：Completely Fair Scheduler）策略，提供了限额和按比例分配两种方式进行资源控制；另一种是实时调度（Real-Time Scheduler）策略，针对实时进程按周期分配固定的运行时间。配置时间都以微秒（µs）为单位，文件名中用us表示。

**CFS调度策略下的配置**

*	设定CPU使用周期使用时间上限


| 参数 | 功能 |
|:---:|:----:|
| cpu.cfs_period_us | 设定周期时间，必须与cfs_quota_us配合使用 |
| cpu.cfs_quota_us | 设定周期内最多可使用的时间。这里的配置指task对单个cpu的使用上限，若cfs_quota_us是cfs_period_us的两倍，就表示在两个核上完全使用。数值范围为1000 - 1000,000（微秒） |
| cpu.stat | 统计信息，包含nr_periods（表示经历了几个cfs_period_us周期）、nr_throttled（表示task被限制的次数）及throttled_time（表示task被限制的总时长） |


**按权重比例设定CPU的分配**


| 参数 | 功能 |
|:---:|:----:|
| cpu.shares | 设定一个整数（必须大于等于2）表示相对权重，最后除以权重总和算出相对比例，按比例分配CPU时间。（如cgroup A设置100，cgroup B设置300，那么cgroup A中的task运行25%的CPU时间。对于一个4核CPU的系统来说，cgroup A 中的task可以100%占有某一个CPU，这个比例是相对整体的一个值 |

**RT调度策略下的配置**

实时调度策略与公平调度策略中的按周期分配时间的方法类似，也是在周期内分配一个固定的运行时间。

| 参数 | 功能 |
|:---:|:----:|
| cpu.rt_period_us | 设定周期时间 |
| cpu.rt_runtime_us | 设定周期中的运行时间 |

##6.3	cpuacct - CPU资源报告
-------


这个子系统的配置是cpu子系统的补充，提供CPU资源用量的统计，时间单位都是纳秒。

| 参数 | 功能 |
|:---:|:----:|
| cpuacct.usage | 统计cgroup中所有task的cpu使用时长 |
| cpuacct.stat | 统计cgroup中所有task的用户态和内核态分别使用cpu的时长 |
| cpuacct.usage_percpu | 统计cgroup中所有task使用每个cpu的时长 |


##6.4	cpuset - CPU绑定
-------

为task分配独立CPU资源的子系统，参数较多，这里只选讲两个必须配置的参数，同时Docker中目前也只用到这两个。

| 参数 | 功能 |
|:---:|:----:|
| cpuset.cpus | 在这个文件中填写cgroup可使用的CPU编号，如0-2,16代表 0、1、2和16这4个CPU |
| cpuset.mems | 与CPU类似，表示cgroup可使用的memory node，格式同上 |



##6.5	device - 限制task对device的使用
-------

**设备黑/白名单过滤 **

| 参数 | 功能 |
|:---:|:----:|
| devices.allow | 允许名单，语法type device_types:node_numbers access type ；type有三种类型：b（块设备）、c（字符设备）、a（全部设备）；access也有三种方式：r（读）、w（写）、m（创建） |
| devices.deny | 禁止名单，语法格式同上 |


**统计报告**


| 参数 | 功能 |
|:---:|:----:|
| devices.list | 报告为这个cgroup中的task设定访问控制的设备 |

##6.6	freezer - 暂停/恢复cgroup中的task
-------

只有一个属性，表示进程的状态，把task放到freezer所在的cgroup，再把state改为FROZEN，就可以暂停进程。不允许在cgroup处于FROZEN状态时加入进程。 * **freezer.state **，包括如下三种状态： - FROZEN 停止 - FREEZING 正在停止，这个是只读状态，不能写入这个值。 - THAWED 恢复

##6.7	 memory - 内存资源管理
-------


**限额类**


| 参数 | 功能 |
|:---:|:----:|
| memory.limit_bytes|  强制限制最大内存使用量，单位有k、m、g三种，填-1则代表无限制 |
| memory.soft_limit_bytes | 软限制，只有比强制限制设置的值小时才有意义。填写格式同上。当整体内存紧张的情况下，task获取的内存就被限制在软限制额度之内，以保证不会有太多进程因内存挨饿。可以看到，加入了内存的资源限制并不代表没有资源竞争 |
| memory.memsw.limit_bytes | 设定最大内存与swap区内存之和的用量限制。填写格式同上 |


**报警与自动控制**

| 参数 | 功能 |
|:---:|:----:|
| memory.oom_control | 改参数填0或1， 0表示开启，当cgroup中的进程使用资源超过界限时立即杀死进程，1表示不启用。默认情况下，包含memory子系统的cgroup都启用。当oom_control不启用时，实际使用内存超过界限时进程会被暂停直到有空闲的内存资源 |


**统计与监控类**



| 参数 | 功能 |
|:---:|:----:|
| memory.usage_bytes | 报告该cgroup中进程使用的当前总内存用量（以字节为单位） |
| memory.max_usage_bytes | 报告该cgroup中进程使用的最大内存用量 |
| memory.failcnt | 报告内存达到在 |
| memory.limit_in_bytes | 设定的限制值的次数 |
| memory.stat | 包含大量的内存统计数据 |
| cache | 页缓存, 包括tmpfs(shmem), 单位为字节 |
| rss | 匿名和swap缓存, 不包括tmpfs(shmem), 单位为字节 |
| mapped_file：memory-mapped | 映射的文件大小, 包括tmpfs(shmem), 单位为字节 |
| pgpgin | 存入内存中的页数 |
| pgpgout | 从内存中读出的页数 |
| swap | swap 用量, 单位为字节 |
| active_anon | 在活跃的最近最少使用(least-recently-used, LRU)列表中的匿名和swap缓存, 包括 tmpfs(shmem), 单位为字节 |
| inactive_anon | 不活跃的LRU列表中的匿名和swap缓存, 包括tmpfs(shmem), 单位为字节 |
| active_file | 活跃LRU列表中的file-backed内存, 以字节为单位 |
| inactive_file | 不活跃LRU列表中的file-backed内存, 以字节为单位 |
| unevictable | 无法再生的内存, 以字节为单位 |
| hierarchical_memory_limit | 包含memory cgroup的层级的内存限制, 单位为字节 |
| hierarchical_memsw_limit | 包含memory cgroup的层级的内存加swap限制, 单位为字节 |

#7	总结
-------

本文由浅入深的讲解了cgroups的方方面面，从cgroups是什么，到cgroups该怎么用，最后对大量的cgroup子系统配置参数进行了梳理。可以看到，内核对cgroups的支持已经较为完善，但是依旧有许多工作需要完善。如网络方面目前是通过TC（Traffic Controller）来控制，未来需要统一整合；资源限制并没有解决资源竞争，在各自限制之内的进程依旧存在资源竞争，优先级调度方面依旧有很大的改进空间。希望通过本文帮助大家了解cgroups，让更多人参与到社区的贡献中。

9. 作者简介

孙健波，浙江大学SEL实验室硕士研究生，目前在云平台团队从事科研和开发工作。浙大团队对PaaS、Docker、大数据和主流开源云计算技术有深入的研究和二次开发经验，团队现将部分技术文章贡献出来，希望能对读者有所帮助。

参考资料

https://sysadmincasts.com/episodes/14-introduction-to-linux-control-groups-cgroups
https://access.redhat.com/documentation/en-US/RedHatEnterprise_Linux/6/html/ResourceManagementGuide/index.html
http://www.cnblogs.com/lisperl/archive/2013/01/14/2860353.html
https://www.kernel.org/doc/Documentation/cgroups





















##2.2	使用cgroup分配资源
-------

拿`memory`子系统为例：

通过查看`cgroup的`配置文件：

```cpp
cat /etc/cgconfig.conf
```

Image(7)

看到memory子系统是挂载在目录`/sys/fs/cgroup/memory`下

进入这个目录

创建一个文件夹，就创建了一个`control group`了

`mkdir test`

发现test目录下自动创建了许多文件：

Image(8)

这些文件的含义如下：

Image(9)

于是，限制内存使用我们就可以设置memory.limit_in_bytes

Image(10)

将一个进程加入到这个test中

echo $$ > tasks

这样就将当前这个终端进程加入到了内存限制的cgroup中了。

将这个cgroup删除只要删除目录就行了

rmdir test

参考文章

http://www.cnblogs.com/lisperl/archive/2012/04/17/2453838.html

http://www.cnblogs.com/lisperl/archive/2012/04/15/2450183.html

http://www.cnblogs.com/lisperl/archive/2012/04/18/2455027.html

http://blog.chinaunix.net/uid-20940095-id-3294134.html


http://www.infoq.com/cn/articles/docker-kernel-knowledge-cgroups-resource-isolation


https://my.oschina.net/cloudcoder/blog/424418?p=1

http://www.ibm.com/developerworks/cn/linux/1506_cgroup/index.html

http://www.cnblogs.com/yjf512/p/3298582.html


