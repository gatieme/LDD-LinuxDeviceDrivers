例1.3  register_chrdev注册字符设备实例
=======


|    驱动    |    描述    | 书中对应章节 |
|:----------:|:----------:|:------------:|
|  1-3register_chrdev | 例1.3  register_chrdev注册字符设备实例  | 1.2.2 使用register_chrdev注册字符设备](http://book.51cto.com/art/201205/337666.htm) |



#1.2.2  使用register_chrdev注册字符设备
-------


注册字符设备可以使用register_chrdev函数。


```c
int register_chrdev (unsigned int major, const  char *name, struct file_operations*fops);
```


register_chrdev函数的major参数如果等于0，则表示采用系统动态分配的主设备号。



注销字符设备可以使用unregister_chrdev函数。


```c
int unregister_chrdev(unsigned int major, const char *name);
```
