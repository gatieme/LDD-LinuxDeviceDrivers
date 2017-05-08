内核新的ioctl方式--unlocked_ioctl和compat_ioctl(解决error:unknown field 'ioctl' specified in initializer)
=======



| CSDN | GitHub |
|:----:|:------:|
| [内核新的ioctl方式--unlocked_ioctl和compat_ioctl<br>解决error:unknown field 'ioctl' specified in initializer](http://blog.csdn.net/gatieme/article/details/71437163) | [`LDD/problem/port/ioctl`](https://github.com/gatieme/LDD-LinuxDeviceDrivers/study/problem/port/ioctl) |




<br>

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作

<br>


#1	问题
-------

把早期 `2.6.32`之前的驱动移植到新的内核中, 如果驱动中定义了 `ioctl` 接口, 老是会提示如下错误

>error:unknown field 'ioctl' specified in initializer


#2	原因
-------

单从字面上看, 可以看出是目前我们驱动中定义的 `ioctl` 接口与内核中 `file_operations` 结构 `ioctl` 函数的定义接口不同.

那么内核到底中到底经历了什么呢?


去查看 `file_operations` 结构体的定义, 可以发现原因是 :

在 `2.6.36` 内核上 `file_operations` 发生了重大的改变 :

原先的, 参见[include/linux/fs.h, version 2.6.17, line 1015](http://elixir.free-electrons.com/linux/v2.6.17.14/source/include/linux/fs.h#L1015)

>int (*ioctl)(struct inode*, struct file*, unsigned int, unsigned long);

被改为了, 参见[include/linux/fs.h, version 4.11, line 1654](http://elixir.free-electrons.com/linux/v4.11/source/include/linux/fs.h#L1654),

>long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
>
>long (*compat_ioctl) (struct file *, unsigned int, unsigned long);


具体 `file_operations` 的实现可以参见[include/linux/fs.h, version 4.11, line 1654](http://elixir.free-electrons.com/linux/v4.11/source/include/linux/fs.h#L1654), 

邮件列表参见[The new way of ioctl()](https://lwn.net/Articles/119652/)

早期为了保证兼容性, `file_operations` 结构体中仍然包含 `ioctl` 函数指针成员, 但是在 `kernel 3.0` 中已经完全删除了 `struct file_operations` 中的 `ioctl` 函数指针



#3	解决方案
-------


因而在实际驱动中, 我们需要

*	将原先的写的 `ioctl` 函数声明给改成下面的 `unlocked_ioctl` 或者 `compat_ioctl`, 

*	在 `file_operations` 结构体的初始化中也是一样. 修改为`unlocked_ioctl` 或者 `compat_ioctl`, 

*	注意参数的兼容性问题, 新的`ioctl()` 接口没有 `struct inode*` 参数, 如果`ioctl` 接口中使用了 `inode`, 因此需要通过其他方式获取 `inode`

内核提供了接口 `file_inode` 来通过文件指针 `file` 来获取其 `inode` 信息, 该函数定义在[include/linux/fs.h, version 4.11, line 1213](http://elixir.free-electrons.com/linux/v4.11/source/include/linux/fs.h#L1213), 如下所示

```cpp
static inline struct inode *file_inode(const struct file *f)
{
	return f->f_inode;
}
```

因此解决方案如下 :


1.	首先是将 `ioctl` 的实现转换为 `unlock_ioctl`

```cpp
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
static int XXX_ioctl(
		struct inode *indoe,
        struct file *file,
		unsigned int cmd,
        unsigned long arg)
{
#else
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
//long (*compat_ioctl) (struct file *file, unsigned int cmd, unsigned long arg)
static long XXX_unlocked_ioctl(
		struct file *file,
        unsigned int cmd,
        unsigned long arg)
{
    //struct inode *inode = file->f_dentry->d_inode;
    //struct inode *inode = file->d_inode;
    struct inode *inode = inode = file_inode(file);
#endif
	/*  此处是ioctl() 函数结构的具体实现  */
}
```

`file_operations` 结构体初始化的过程采取同样的操作

```cpp
static struct file_operations fpga_fops = {
	.owner 	= THIS_MODULE,
	.open	= fpga_open,
	.read	= fpga_read,
	.write = fpga_write,
	.llseek = fpga_llseek,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
    .ioctl = XXX_ioctl,
#else
    .compat_ioctl = XXX_unlocked_ioctl,
#endif
};
```


#4	参照
-------


[ioctl 变成了 unlocked_ioctl](http://blog.csdn.net/ypist/article/details/6664973)

[ Linux字符设备驱动入门（二）——加入ioctl功能](http://blog.csdn.net/coolkids_2008/article/details/6982305)

[内核新的ioctl方式---- unlocked_ioctl和compat_ioctl](http://www.cnblogs.com/hfww/archive/2012/03/31/2426717.html)

[error: unknown field 'ioctl' specified in initializer](http://www.360doc.com/content/14/0127/14/14450281_348317383.shtml)

[error: unknown field 'ioctl' specified in initializer](http://blog.chinaunix.net/uid-20620288-id-3066368.html)

[error: unknown field ‘ioctl’ specified in initializer](http://blog.csdn.net/zhou1232006/article/details/6867584)

[error:unknown field 'ioctl' specified in initializer](http://www.cnblogs.com/xiaoya901109/archive/2012/07/13/2589623.html)

[error: unknown field 'ioctl' specified in initializer](http://blog.csdn.net/yusiguyuan/article/details/16829299)

[error: unknown field 'ioctl' specified in initializer问题](http://blog.csdn.net/yuhaibin168/article/details/7711994)



<br>
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作.


