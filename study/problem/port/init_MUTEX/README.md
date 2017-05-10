init_MUTEX被废除(解决rror: implicit declaration of function ‘init_MUTEX’)
=======

| CSDN | GitHub |
|:----:|:------:|
| [init_MUTEX被废除<br>解决rror: implicit declaration of function ‘init_MUTEX’](http://blog.csdn.net/gatieme/article/details/71598127) | [`LDD/problem/port/init_MUTEX`](https://github.com/gatieme/LDD-LinuxDeviceDrivers/tree/master/study/problem/port/init_MUTEX)  |


<br>
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处
<br>



#1  问题
-------


近期在移植驱动的时候, 提示了如下错误

>error: implicit declaration of function ‘init_MUTEX’ [-Werror=implicit-function-declaration]


#2  原因分析
-------

在 `2.6.37` 之后的 `Linux` 内核中, `init_mutex` 已经被废除了, 新版本使用 `sema_init` 函数



查了一下早期版本的定义, 参见[include/linux/semaphore.h, version 2.6.36.4, line 42](http://elixir.free-electrons.com/linux/v2.6.36.4/source/include/linux/semaphore.h#L42)

```cpp
static inline void sema_init(struct semaphore *sem, int val)
{
	static struct lock_class_key __key;
	*sem = (struct semaphore) __SEMAPHORE_INITIALIZER(*sem, val);
	lockdep_init_map(&sem->lock.dep_map, "semaphore->lock", &__key, 0);
}

#define init_MUTEX(sem)		sema_init(sem, 1)
#define init_MUTEX_LOCKED(sem)	sema_init(sem, 0)
```




#3  解决方案
-------

修改 `init_MUTEX` 为 `sema_init`即可, 也可以在驱动中定义 `init_MUTEX`

*   修改 `init_MUTEX` 为 `sema_init`


```cpp
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36) && !defined(init_MUTEX)
    sema_init(&sem, 1);
#else
    init_MUTEX(&sem);
#endif
```

*   定义 `init_MUTEX` 为 `sema_init`

```cpp
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36) && !defined(init_MUTEX)
#define init_MUTEX(sem)		sema_init(sem, 1)
#endif
```

其实早期的内核中, 定义了 `sema_init`, 因此其实可以不需要添加 `#if #endif` 宏, 直接修改 `init_MUTEX` 为 `sema_init` 是没有什么问题的.

<br>

*	本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作, 

*	采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme/article/details/71598127 ), 不得用于商业目的, 

*	基于本文修改后的作品务必以相同的许可发布. 如有任何疑问，请与我联系.


