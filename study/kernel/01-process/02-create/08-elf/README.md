Linux进程启动过程do_execve详解
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------------- |:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|
| 2016-06-04 | [Linux-4.5](http://lxr.free-electrons.com/source/?v=4.5) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度-之-进程的描述](http://blog.csdn.net/gatieme/article/category/6225543) |



#execve系统调用
-------

##execve系统调用
-------

我们前面提到了, fork, vfork等复制出来的进程是父进程的一个副本, 那么如何我们想加载新的程序, 可以通过execve来加载和启动新的程序。

>x86架构下, 其实还实现了一个新的exec的系统调用叫做execveat(自linux-3.19后进入内核)
>
>[syscalls,x86: Add execveat() system call](http://lwn.net/Articles/600344)

##exec()函数族
-------

exec函数一共有六个，其中execve为内核级系统调用，其他（execl，execle，execlp，execv，execvp）都是调用execve的库函数。


```c
int execl(const char *path, const char *arg, ...);
int execlp(const char *file, const char *arg, ...);
int execle(const char *path, const char *arg,
                  ..., char * const envp[]);
int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
```

#ELF文件格式以及可执行程序的表示
-------

##struct linux_bin_rpm结构描述一个可执行程序
-------

linux_binprm是定义在[include/linux/binfmts.h](http://lxr.free-electrons.com/source/include/linux/binfmts.h#L14)中, 用来保存要要执行的文件相关的信息

```c
/*
* This structure is used to hold the arguments that are used when loading binaries.
*/
struct linux_binprm {
    char buf[BINPRM_BUF_SIZE];	// 保存可执行文件的头128字节
#ifdef CONFIG_MMU
    struct vm_area_struct *vma;
    unsigned long vma_pages;
#else
# define MAX_ARG_PAGES  32
    struct page *page[MAX_ARG_PAGES];
#endif
    struct mm_struct *mm;
    unsigned long p; /* current top of mem , 当前内存页最高地址*/
    unsigned int
            cred_prepared:1,/* true if creds already prepared (multiple
                             * preps happen for interpreters) */
            cap_effective:1;/* true if has elevated effective capabilities,
                             * false if not; except for init which inherits
                             * its parent's caps anyway */
#ifdef __alpha__
    unsigned int taso:1;
#endif
    unsigned int recursion_depth; /* only for search_binary_handler() */
    struct file * file;			/*  要执行的文件  */
    struct cred *cred;      /* new credentials */
    int unsafe;             /* how unsafe this exec is (mask of LSM_UNSAFE_*) */
    unsigned int per_clear; /* bits to clear in current->personality */
    int argc, envc;		/*  命令行参数和环境变量数目  */
    const char * filename;  /* Name of binary as seen by procps, 要执行的文件的名称  */
    const char * interp;    /* Name of the binary really executed. Most
                               of the time same as filename, but could be
                               different for binfmt_{misc,script} 要执行的文件的真实名称，通常和filename相同  */
    unsigned interp_flags;
    unsigned interp_data;
    unsigned long loader, exec;
};
```

##struct linux_binfmt可执行程序的结构
-------
linux内核对所支持的每种可执行的程序类型都有个struct linux_binfmt的数据结构，定义如下

>linux_binfmt定义在[include/linux/binfmts.h](http://lxr.free-electrons.com/source/include/linux/binfmts.h#L74)中

```c
/*
  * This structure defines the functions that are used to load the binary formats that
  * linux accepts.
  */
struct linux_binfmt {
    struct list_head lh;
    struct module *module;
    int (*load_binary)(struct linux_binprm *);
    int (*load_shlib)(struct file *);
    int (*core_dump)(struct coredump_params *cprm);
    unsigned long min_coredump;     /* minimal dump size */
 };
```
这里的linux_binfmt对象包含了一个单链表，这个单链表中的第一个元素的地址存储在formats这个变量中list_for_each_entry依次应用load_binary的方法，同时我们可以看到这里会有递归调用，bprm会记录递归调用的深度。

装载ELF可执行程序的load_binary的方法叫做load_elf_binary方法，下面会进行具体分析



#execve加载可执行程序的过程
-------

内核中实际执行execv()或execve()系统调用的程序是do_execve()，这个函数先打开目标映像文件，并从目标文件的头部（第一个字节开始）读入若干（当前Linux内核中是128）字节（实际上就是填充ELF文件头，下面的分析可以看到），然后调用另一个函数search_binary_handler()，在此函数里面，它会搜索我们上面提到的Linux支持的可执行文件类型队列，让各种可执行程序的处理程序前来认领和处理。如果类型匹配，则调用load_binary函数指针所指向的处理函数来处理目标映像文件。在ELF文件格式中，处理函数是load_elf_binary函数，下面主要就是分析load_elf_binary函数的执行过程（说明：因为内核中实际的加载需要涉及到很多东西，这里只关注跟ELF文件的处理相关的代码）：

sys_execve() > do_execve() > do_execveat_common > search_binary_handler() > load_elf_binary()

##execve的入口函数sys_execve
-------

| 描述 | 定义 | 链接 |
| ------------- |:-------------:|:-------------:|
| 系统调用号(体系结构相关) | 类似与如下的形式<br>#define __NR_execve                             117<br>__SYSCALL(117, sys_execve, 3) | [arch/对应体系结构/include/uapi/asm/unistd.h, line 265](http://lxr.free-electrons.com/ident?v=4.5;i=__NR_execve) |
| 入口函数声明 |asmlinkage long sys_execve(const char __user *filename,<br>const char __user *const __user *argv,<br>const char __user *const __user *envp); | [include/linux/syscalls.h, line 843](http://lxr.free-electrons.com/source/include/linux/syscalls.h?v=4.5#L843) |
| 系统调用实现 | SYSCALL_DEFINE3(execve,<br>const char __user *, filename,<br>const char __user *const __user *, argv,<br>const char __user *const __user *, envp)<br>{<br>return do_execve(getname(filename), argv, envp);<br>} | [fs/exec.v 1710](http://lxr.free-electrons.com/source/fs/exec.c?v=4.5#L1710) |


execve系统调用的的入口点是体系结构相关的sys_execve, 该函数很快将工作委托给系统无关的do_execve函数


```c
SYSCALL_DEFINE3(execve,
                const char __user *, filename,
                const char __user *const __user *, argv,
                const char __user *const __user *, envp)
{
    return do_execve(getname(filename), argv, envp);
}
```

通过参数传递了寄存集合和可执行文件的名称(filename), 而且还传递了指向了程序的参数argv和环境变量envp的指针

| 参数 | 描述 |
| ------------- |:-------------:|
| filename | 可执行程序的名称 |
| argv | 程序的参数 |
| envp | 环境变量 |

指向程序参数argv和环境变量envp两个数组的指针以及数组中所有的指针都位于虚拟地址空间的用户空间部分。因此内核在当问用户空间内存时, 需要多加小心, 而__user注释则允许自动化工具来检测时候所有相关事宜都处理得当

##do_execve函数

do_execve的定义在fs/exec.c中，参见 http://lxr.free-electrons.com/source/fs/exec.c?v=4.5#L1628

| [更早期实现linux-2.4](http://lxr.free-electrons.com/source/fs/exec.c?v=2.4.37#L936) | [linux-3.18引入execveat之前do_execve实现](http://lxr.free-electrons.com/source/fs/exec.c?v=3.18#L1549) | [linux-3.19~至今引入execveat之后do_execve实现](http://lxr.free-electrons.com/source/fs/exec.c?v=4.5#L1628) | [do_execveat的实现](http://lxr.free-electrons.com/source/fs/exec.c?v=4.5#L1637) |
| ------------- |:-------------:|:-------------:|
| 代码过长, 没有经过do_execve_common的封装| int do_execve(struct filename *filename,<br>const char __user *const __user *__argv,<br>const char __user *const __user *__envp)<br>{<br>    struct user_arg_ptr argv = { .ptr.native = __argv };<br>struct user_arg_ptr envp = { .ptr.native = __envp };<br>    return do_execveat_common(AT_FDCWD, filename, argv, envp, 0);<br>} | int do_execve(struct filename *filename,<br>    const char __user *const __user *__argv,<br>const char __user *const __user *__envp)<br>{<br>struct user_arg_ptr argv = { .ptr.native = __argv };<br>struct user_arg_ptr envp = { .ptr.native = __envp };<br>return do_execveat_common(AT_FDCWD, filename, argv, envp, 0);<br>} | int do_execveat(int fd, struct filename *filename,<br>const char __user *const __user *__argv,<br>const char __user *const __user *__envp,<br>int flags)<br>{<br>struct user_arg_ptr argv = { .ptr.native = __argv };<br>struct user_arg_ptr envp = { .ptr.native = __envp };<br>return do_execveat_common(fd, filename, argv, envp, flags);<br>}<br> |


我们可以看到不同时期的演变, 早期的代码 do_execve就直接完成了自己的所有工作, 后来do_execve会调用更加底层的do_execve_common函数, 后来x86架构下引入了新的系统调用execveat, 为了使代码更加通用, do_execveat_common替代了原来的do_execve_common函数


![do_execve函数的流程](./images/do_execve.jpg)


##程序的加载do_execve_common和do_execveat_common
-------

>[早期linux-2.4中直接由do_execve实现程序的加载和运行](http://lxr.free-electrons.com/source/fs/exec.c?v=2.4.37#L936)
>
>[linux-3.18引入execveat之前do_execve调用do_execve_common来完成程序的加载和运行](http://lxr.free-electrons.com/source/fs/exec.c?v=3.18#L1427)
>
>[linux-3.19~至今引入execveat之后do_execve调用do_execveat_common来完成程序的加载和运行](http://lxr.free-electrons.com/source/fs/exec.c?v=4.5#L1481)

在Linux中提供了一系列的函数，这些函数能用可执行文件所描述的新上下文代替进程的上下文。这样的函数名以前缀exec开始。所有的exec函数都是调用了execve()系统调用。

sys_execve接受参数：1.可执行文件的路径  2.命令行参数字符串 3.环境变量字符串

sys_execve是调用do_execve实现的。do_execve则是调用do_execveat_common实现的，依次执行以下操作：




1、调用unshare_files()为进程复制一份文件表；

2、调用kzalloc()分配一份structlinux_binprm结构体；

3、调用open_exec()查找并打开二进制文件；

4、调用sched_exec()找到最小负载的CPU，用来执行该二进制文件；

5、根据获取的信息，填充structlinux_binprm结构体中的file、filename、interp成员；

6、调用bprm_mm_init()创建进程的内存地址空间，为新程序初始化内存管理.并调用init_new_context()检查当前进程是否使用自定义的局部描述符表；如果是，那么分配和准备一个新的LDT；

7、填充structlinux_binprm结构体中的argc、envc成员；

8、调用prepare_binprm()检查该二进制文件的可执行权限；最后，kernel_read()读取二进制文件的头128字节（这些字节用于识别二进制文件的格式及其他信息，后续会使用到）；

9、调用copy_strings_kernel()从内核空间获取二进制文件的路径名称；

10、调用copy_string()从用户空间拷贝环境变量和命令行参数；

11.	 至此，二进制文件已经被打开，struct linux_binprm结构体中也记录了重要信息, 内核开始调用exec_binprm执行可执行程序


12. 释放linux_binprm数据结构，返回从该文件可执行格式的load_binary中获得的代码

定义在[fs/exec.c](http://lxr.free-electrons.com/source/fs/exec.c#L1580)

```c
/*
 * sys_execve() executes a new program.
 */
static int do_execveat_common(int fd, struct filename *filename,
                          struct user_arg_ptr argv,
                          struct user_arg_ptr envp,
                          int flags)
{
    char *pathbuf = NULL;
    struct linux_binprm *bprm;  /* 这个结构当然是非常重要的，下文，列出了这个结构体以便查询各个成员变量的意义   */
    struct file *file;
    struct files_struct *displaced;
    int retval;

    if (IS_ERR(filename))
            return PTR_ERR(filename);

    /*
     * We move the actual failure in case of RLIMIT_NPROC excess from
     * set*uid() to execve() because too many poorly written programs
     * don't check setuid() return code.  Here we additionally recheck
     * whether NPROC limit is still exceeded.
     */
    if ((current->flags & PF_NPROC_EXCEEDED) &&
        atomic_read(&current_user()->processes) > rlimit(RLIMIT_NPROC)) {
            retval = -EAGAIN;
            goto out_ret;
    }

    /* We're below the limit (still or again), so we don't want to make
     * further execve() calls fail. */
    current->flags &= ~PF_NPROC_EXCEEDED;

    //  1.	调用unshare_files()为进程复制一份文件表；
    retval = unshare_files(&displaced);
    if (retval)
            goto out_ret;

    retval = -ENOMEM;

    //	2、调用kzalloc()在堆上分配一份structlinux_binprm结构体；
    bprm = kzalloc(sizeof(*bprm), GFP_KERNEL);
    if (!bprm)
            goto out_files;

    retval = prepare_bprm_creds(bprm);
    if (retval)
            goto out_free;

    check_unsafe_exec(bprm);
    current->in_execve = 1;

    //	3、调用open_exec()查找并打开二进制文件；
    file = do_open_execat(fd, filename, flags);
    retval = PTR_ERR(file);
    if (IS_ERR(file))
            goto out_unmark;

	//	4、调用sched_exec()找到最小负载的CPU，用来执行该二进制文件；
    sched_exec();

    //	5、根据获取的信息，填充structlinux_binprm结构体中的file、filename、interp成员；
    bprm->file = file;
    if (fd == AT_FDCWD || filename->name[0] == '/') {
            bprm->filename = filename->name;
    } else {
            if (filename->name[0] == '\0')
                    pathbuf = kasprintf(GFP_TEMPORARY, "/dev/fd/%d", fd);
            else
                    pathbuf = kasprintf(GFP_TEMPORARY, "/dev/fd/%d/%s",
                                        fd, filename->name);
            if (!pathbuf) {
                    retval = -ENOMEM;
                    goto out_unmark;
            }
            /*
             * Record that a name derived from an O_CLOEXEC fd will be
             * inaccessible after exec. Relies on having exclusive access to
             * current->files (due to unshare_files above).
             */
            if (close_on_exec(fd, rcu_dereference_raw(current->files->fdt)))
                    bprm->interp_flags |= BINPRM_FLAGS_PATH_INACCESSIBLE;
            bprm->filename = pathbuf;
    }
    bprm->interp = bprm->filename;

    //	6、调用bprm_mm_init()创建进程的内存地址空间，并调用init_new_context()检查当前进程是否使用自定义的局部描述符表；如果是，那么分配和准备一个新的LDT；
    retval = bprm_mm_init(bprm);
    if (retval)
            goto out_unmark;

    //	7、填充structlinux_binprm结构体中的命令行参数argv,环境变量envp
    bprm->argc = count(argv, MAX_ARG_STRINGS);
    if ((retval = bprm->argc) < 0)
            goto out;

    bprm->envc = count(envp, MAX_ARG_STRINGS);
    if ((retval = bprm->envc) < 0)
            goto out;

    //	8、调用prepare_binprm()检查该二进制文件的可执行权限；最后，kernel_read()读取二进制文件的头128字节（这些字节用于识别二进制文件的格式及其他信息，后续会使用到）；
    retval = prepare_binprm(bprm);
    if (retval < 0)
            goto out;

	//	9、调用copy_strings_kernel()从内核空间获取二进制文件的路径名称；
    retval = copy_strings_kernel(1, &bprm->filename, bprm);
    if (retval < 0)
            goto out;

    bprm->exec = bprm->p;

    //	10.1、调用copy_string()从用户空间拷贝环境变量
    retval = copy_strings(bprm->envc, envp, bprm);
    if (retval < 0)
            goto out;

    //	10.2、调用copy_string()从用户空间拷贝命令行参数；
    retval = copy_strings(bprm->argc, argv, bprm);
    if (retval < 0)
            goto out;
	/*
	    至此，二进制文件已经被打开，struct linux_binprm结构体中也记录了重要信息；

        下面需要识别该二进制文件的格式并最终运行该文件
    */
    retval = exec_binprm(bprm);
    if (retval < 0)
            goto out;

    /* execve succeeded */
    current->fs->in_exec = 0;
    current->in_execve = 0;
    acct_update_integrals(current);
    task_numa_free(current);
    free_bprm(bprm);
    kfree(pathbuf);
    putname(filename);
    if (displaced)
            put_files_struct(displaced);
    return retval;

out:
    if (bprm->mm) {
            acct_arg_size(bprm, 0);
            mmput(bprm->mm);
    }

out_unmark:
    current->fs->in_exec = 0;
    current->in_execve = 0;

out_free:
    free_bprm(bprm);
    kfree(pathbuf);

out_files:
    if (displaced)
            reset_files_struct(displaced);
out_ret:
    putname(filename);
    return retval;
}
http://blog.chinaunix.net/uid-23769728-id-3129443.html
http://blog.csdn.net/titer1/article/details/45008793
```




##exec_binprm识别并加载二进程程序
-------

每种格式的二进制文件对应一个struct linux_binprm结构体，load_binary成员负责识别该二进制文件的格式；

内核使用链表组织这些structlinux_binfmt结构体，链表头是formats。

接着do_execve_common()继续往下看：

1. 调用search_binary_handler()函数对linux_binprm的formats链表进行扫描，并尝试每个load_binary函数，如果成功加载了文件的执行格式，对formats的扫描终止。


```c
static int exec_binprm(struct linux_binprm *bprm)
{
    pid_t old_pid, old_vpid;
    int ret;

    /* Need to fetch pid before load_binary changes it */
    old_pid = current->pid;
    rcu_read_lock();
    old_vpid = task_pid_nr_ns(current, task_active_pid_ns(current->parent));
    rcu_read_unlock();

    ret = search_binary_handler(bprm);
    if (ret >= 0) {
            audit_bprm(bprm);
            trace_sched_process_exec(current, old_pid, bprm);
            ptrace_event(PTRACE_EVENT_EXEC, old_vpid);
            proc_exec_connector(current);
    }

    return ret;
}
```

##search_binary_handler识别二进程程序
-------

这里需要说明的是，这里的fmt变量的类型是struct linux_binfmt *, 但是这一个类型与之前在do_execveat_common()中的bprm是不一样的，


>定义在[fs/exec.c](http://lxr.free-electrons.com/source/fs/exec.c#L1502)

/*
 * cycle the list of binary formats handler, until one recognizes the image
 */
int search_binary_handler(struct linux_binprm *bprm)
{
    bool need_retry = IS_ENABLED(CONFIG_MODULES);
    struct linux_binfmt *fmt;
    int retval;

    /* This allows 4 levels of binfmt rewrites before failing hard. */
    if (bprm->recursion_depth > 5)
            return -ELOOP;

    retval = security_bprm_check(bprm);
    if (retval)
            return retval;

    retval = -ENOENT;
 retry:
    read_lock(&binfmt_lock);

    //	遍历formats链表
    list_for_each_entry(fmt, &formats, lh) {
            if (!try_module_get(fmt->module))
                    continue;
            read_unlock(&binfmt_lock);
            bprm->recursion_depth++;

            // 遍历formats链表
            retval = fmt->load_binary(bprm);
            read_lock(&binfmt_lock);
            put_binfmt(fmt);
            bprm->recursion_depth--;
            if (retval < 0 && !bprm->mm) {
                    /* we got to flush_old_exec() and failed after it */
                    read_unlock(&binfmt_lock);
                    force_sigsegv(SIGSEGV, current);
                    return retval;
            }
            if (retval != -ENOEXEC || !bprm->file) {
                    read_unlock(&binfmt_lock);
                    return retval;
            }
    }
    read_unlock(&binfmt_lock);

    if (need_retry) {
            if (printable(bprm->buf[0]) && printable(bprm->buf[1]) &&
                printable(bprm->buf[2]) && printable(bprm->buf[3]))
                    return retval;
            if (request_module("binfmt-%04x", *(ushort *)(bprm->buf + 2)) < 0)
                    return retval;
            need_retry = false;
            goto retry;
    }

    return retval;
}





