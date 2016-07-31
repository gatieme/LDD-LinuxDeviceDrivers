#register_chrdev注册字符设备实例
-------



[使用register_chrdev注册字符设备](http://book.51cto.com/art/201205/337666.htm)


注册字符设备可以使用register_chrdev函数。


```c
int register_chrdev(unsigned int major,
                    const  char *name,
                    struct file_operations*fops);
```

register_chrdev函数的major参数如果等于0，则表示采用系统动态分配的主设备号。

注销字符设备可以使用unregister_chrdev函数

```c
int unregister_chrdev(unsigned int major, const char *name);
```



