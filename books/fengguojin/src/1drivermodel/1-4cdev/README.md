#例1-4  使用cdev_add注册字符设备
=======

[1.2.3 使用cdev_add注册字符设备](http://book.51cto.com/art/201205/337667.htm)

|    驱动    |    描述    | 书中对应章节 |
|:----------:|:----------:|:------------:|
|  1-4cdev   |  例1.4  cdev_add注册字符设备实例 | [1.2.3 使用cdev_add注册字符设备](http://book.51cto.com/art/201205/337667.htm) |

在Linux 2.6内核中的字符设备用cdev结构来描述，其定义如下：

```c
struct cdev
{
    struct kobject kobj;
    struct module *owner; //所属模块
    const struct file_operations *ops; //文件操作结构
    struct list_head list;
    dev_t dev; //设备号，int 类型，高12位为主设备号，低20位为次设备号
    unsigned int count;
};
```

下面一组函数用来对cdev结构进行操作：


```c
struct cdev *cdev_alloc(void);//分配一个cdev

void cdev_init(struct cdev *, const struct file_operations *);//初始化cdev的file_operation

void cdev_put(struct cdev *p);// //减少使用计数

//注册设备，通常发生在驱动模块的加载函数中

int cdev_add(struct cdev *, dev_t, unsigned);

//注销设备，通常发生在驱动模块的卸载函数中
void cdev_del(struct cdev *);
```


使用cdev_add注册字符设备前
应该先调用register_chrdev_region或alloc_chrdev_region分配设备号

*   register_chrdev_region函数用于指定设备号的情况,

*   alloc_chrdev_region函数用于动态申请设备号, 系统自动返回没有占用的设备号。

```c
int register_chrdev_region(dev_t from, unsigned count, const char *name) ;

int alloc_chrdev_region(dev_t *dev,unsigned baseminor,unsigned count,const char *name);
```


*   alloc_chrdev_region申请一个动态主设备号, 并申请一系列次设备号.

*   baseminor为起始次设备号,

*   count为次设备号的数量.


注销设备号(cdev_del)后使用unregister_chrdev_region：


```c
void unregister_chrdev_region(dev_t from,unsigned count);
```
