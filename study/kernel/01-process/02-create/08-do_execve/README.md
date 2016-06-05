Linux进程启动过程do_execve详解
=======


| 日期 | 内核版本 | 架构| 作者 | GitHub| CSDN |
| ------------- |:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|
| 2016-06-04 | [Linux-4.5](http://lxr.free-electrons.com/source/?v=4.5) | X86 & arm | [gatieme](http://blog.csdn.net/gatieme) | [LinuxDeviceDrivers](https://github.com/gatieme/LDD-LinuxDeviceDrivers) | [Linux进程管理与调度-之-进程的描述](http://blog.csdn.net/gatieme/article/category/6225543) |


#execve启动新程序
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

#execve系统调用的实现
-------

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

    retval = unshare_files(&displaced);
    if (retval)
            goto out_ret;

    retval = -ENOMEM;
    bprm = kzalloc(sizeof(*bprm), GFP_KERNEL);
    if (!bprm)
            goto out_files;

    retval = prepare_bprm_creds(bprm);
    if (retval)
            goto out_free;

    check_unsafe_exec(bprm);
    current->in_execve = 1;

    file = do_open_execat(fd, filename, flags);
    retval = PTR_ERR(file);
    if (IS_ERR(file))
            goto out_unmark;

    sched_exec();

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

    retval = bprm_mm_init(bprm);
    if (retval)
            goto out_unmark;

    bprm->argc = count(argv, MAX_ARG_STRINGS);
    if ((retval = bprm->argc) < 0)
            goto out;

    bprm->envc = count(envp, MAX_ARG_STRINGS);
    if ((retval = bprm->envc) < 0)
            goto out;

    retval = prepare_binprm(bprm);
    if (retval < 0)
            goto out;

    retval = copy_strings_kernel(1, &bprm->filename, bprm);
    if (retval < 0)
            goto out;

    bprm->exec = bprm->p;
    retval = copy_strings(bprm->envc, envp, bprm);
    if (retval < 0)
            goto out;

    retval = copy_strings(bprm->argc, argv, bprm);
    if (retval < 0)
            goto out;

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

http://www.cnblogs.com/MarkWoo/p/4439267.html
http://blog.csdn.net/luomoweilan/article/details/23042595
http://bbs.chinaunix.net/thread-3687970-1-1.html
http://blog.csdn.net/sunnybeike/article/details/6899959
http://www.cnblogs.com/sj20082663/archive/2013/05/30/3109259.html
http://blog.chinaunix.net/uid-23769728-id-3129443.html
http://blog.csdn.net/titer1/article/details/45008793
```
