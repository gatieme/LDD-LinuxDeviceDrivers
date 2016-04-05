#模块参数
-------
很多情况下，我们期望通过参数来控制我们的驱动的行为，比如由于系统的不同，而为了保证我们驱动有较好的移植性，我们有时候期望通过传递参数来控制我们驱动的行为，这样不同的系统中，驱动可能有不同的行为控制。

为了满足这种需求，内核允许对驱动程序指定参数，而这些参数可在加载驱动的过程中动态的改变

参数的来源主要有两个

*	使用`insmod/modprobe ./xxx.ko`时候在命令行后直接给出参数；

*	modprobe命令装载模块时可以从它的配置文件`/etc/modprobe.conf`文件中读取参数值

**这个宏必须放在任何函数之外**，通常实在源文件的头部

#模块参数传递的方式
-------
对于如何向模块传递参数，Linux kernel 提供了一个简单的框架。其允许驱动程序声明参数，并且用户在系统启动或模块装载时为参数指定相应值，在驱动程序里，参数的用法如同全局变量。

使用下面的宏时需要包含头文件`<linux/moduleparam.h>`

##宏
-------
```
module_param(name, type, perm);
module_param_array(name, type, num_point, perm);
module_param_named(name_out, name_in, type, perm);
module_param_string(name, string, len, perm);
MODULE_PARM_DESC(name, describe);

```

##参数类型
-------
内核支持的模块参数类型如下
| 参数 | 描述 |
| ------------- |:-------------:|
| bool | 布尔类型(true/false)，关联的变量类型应该死int
| intvbool | bool的反值，例如赋值位true，但是实际值位false |
| int | 整型 |
| long | 长整型 |
| short | 短整型 |
| uint | 无符号整型 |
| ulong | 无符号长整形型 |
| ushort | 无符号短整型|
| charp | 字符指针类型，内核会为用户提供的字符串分配内存，并设置相应指针 |

关于数组类型怎么传递，我们后面会谈到

>**注意**
>
>如果我们需要的类型不在上面的清单中，模块代码中的钩子可让我们来指定这些类型。
>
>具体的细节请参阅moduleparam.h文件。所有的模块参数都应该给定一个默认值；
>
>insmod只会在用户明确设定了参数值的情况下才会改变参数的值，模块可以根据默认值来判断是否一个显示给定的值

##访问权限
-------
perm访问权限与linux文件爱你访问权限相同的方式管理，

如0644，或使用stat.h中的宏如S_IRUGO表示。

我们鼓励使用stat.h中存在的定义。这个值用来控制谁能够访问sysfs中对模块参数的表述。
如果制定0表示完全关闭在sysfs中相对应的项，否则的话，模块参数会在/sys/module中出现，并设置为给定的访问许可。

如果指定S_IRUGO，则任何人均可读取该参数，但不能修改
如果指定S_IRUGO | S_IWUSR 则允许root修改该值

>注意
>
>如果一个参数通过sysfs而被修改，则如果模块修改了这个参数的值一样，但是内核不会以任何方式通知模块，大多数情况下，我们不应该让模块参数是可写的，除非我们打算检测这种修改并做出相应的动作。



如果你只有ko文件却没有源码，想知道模块中到底有哪些模块参数，不着急，只需要用
 
 ```
 modinfo -p ${modulename}
 ```
就可以看到个究竟啦。

对于已经加载到内核里的模块，如果想改变这些模块的模块参数该咋办呢？简单，只需要输入
```
echo -n ${value} > /sys/module/${modulename}/parameters/${param}
```
来修改即可。


#示例
-------
##传递全局参数
-------
在模块里面, 声明一个变量（全局变量）,用来接收用户加载模块时传递的参数



```c
module_param(name, type, perm);
```


| 参数 | 描述 |
| ------------- |:-------------:|
| name | 用来接收参数的变量名 |
| type | 参数的数据类型 |
| perm | 用于sysfs入口项系的访问可见性掩码 |




###示例--传递int
-------
这些宏不会声明变量，因此在使用宏之前，必须声明变量，典型地用法如下：

```
static int value = 0;
module_param(value, int, 0644);
MODULE_PARM_DESC(value_int, "Get an value from user...\n");
```


使用
```
sudo insmod param.ko value=100 
```
来进行加载


###示例--传递charp
-------


```c
static char *string = "gatieme";
module_param(string, charp, 0644);
MODULE_PARM_DESC(string, "Get an string(char *) value from user...\n");
```
使用
```
sudo insmod param.ko string="hello" 
```



##在模块内部变量的名字和加载模块时传递的参数名字不同
-------

前面那种情况下，外部参数的名字和模块内部的名字必须一致，那么有没有其他的绑定方法，可以是我们的参数传递更加灵活呢？

使模块源文件内部的变 量名与外部的参数名有不同的名字，通过module_param_named()定义。 

```c
module_param_named(name_out, name_in, type, perm);
```

| 参数 | 描述 |
| ------------- |:-------------:|
| name_out | 加载模块时，参数的名字 |
| name_in | 模块内部变量的名字 |
| type | 参数类型 |
| perm | 访问权限 |

使用

 
```
static int  value_in = 0;
module_param_named(value_out, value_in, int, 0644);
MODULE_PARM_DESC(value_in, "value_in named var_out...\n");
```

加载

```
sudo insmod param.ko value_out=200
```

##传递字符串
-------

 加载模块的时候, 传递字符串到模块的一个全局字符数组里面
```
module_param_string(name, string, len, perm);
```
| 参数 | 描述 |
| ------------- |:-------------:|
| name   | 在加载模块时，参数的名字 |
| string | 模块内部的字符数组的名字 |
| len    | 模块内部的字符数组的大小 |
| perm   | 访问权限 |


```
static char buffer[20] = "gatieme";
module_param_string(buffer, buffer, sizeof(buffer), 0644);
MODULE_PARM_DESC(value_charp, "Get an string buffer from user...\n");
```

##传递数组
-------
加载模块的时候, 传递参数到模块的数组中

```
module_param_array(name, type, num_point, perm);
```
| 参数 | 描述 |
| ------------- |:-------------:|
| name | 模块的数组名，也是外部制定的数组名|
| type | 模块数组的数据类型|
| num_point | 用来获取用户在加载模块时传递的参数个数,为NULL时，表示不关心用户传递的参数个数 |
| perm | 访问权限|

使用

```
static int  array[3];
int         num;
module_param_array(array, int, &num, 0644);
MODULE_PARM_DESC(array, "Get an array from user...\n");
```

##指定描述信息
-------

```c
MODULE_PARM_DESC(name, describe);
```

| 参数 | 描述 |
| ------------- |:-------------:|
| name | 参数变量名 |
| describe | 描述信息的字符串 |

使用modinfo查看参数
```sh
modinfo -p param.ko
```


#param驱动源码
-------

##驱动源码param.c
-------
```c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>


/*
 *  在模块里面, 声明一个变量（全局变量）,
 *  用来接收用户加载模块哦时传递的参数
 *
 *  module_param(name, type, perm)
 **/
static int value = 0;
module_param(value, int, 0644);
MODULE_PARM_DESC(value_int, "Get an value from user...\n");

/*
 *  在模块内部变量的名字和加载模块时传递的参数名字不同
 *
 *  module_param_named(name_out, name_in, type, perm)
 *
 *  @name_out    加载模块时，参数的名字
 *  @name_in     模块内部变量的名字
 *  @type        参数类型
 *  @perm        访问权限
 * */
static int  value_in = 0;
module_param_named(value_out, value_in, int, 0644);
MODULE_PARM_DESC(value_in, "value_in named var_out...\n");


/*
 *  加载模块的时候, 传递字符串到模块的一个全局字符数组里面
 *
 *  module_param_string(name, string, len, perm)
 *
 *  @name   在加载模块时，参数的名字
 *  @string 模块内部的字符数组的名字
 *  @len    模块内部的字符数组的大小
 *  #perm   访问权限
 *
 * */
static char *string = NULL;
module_param(string, charp, 0644);
MODULE_PARM_DESC(string, "Get an string(char *) value from user...\n");


static char buffer[20] = "gatieme";
module_param_string(buffer, buffer, sizeof(buffer), 0644);
MODULE_PARM_DESC(value_charp, "Get an string buffer from user...\n");


/*
 *  加载模块的时候, 传递参数到模块的数组中
 *
 *  module_param_array(name, type, num_point, perm)
 *
 *  @name       模块的数组名，也是外部制定的数组名
 *  @type       模块数组的数据类型
 *  @num_point  用来获取用户在加载模块时传递的参数个数,
 *              为NULL时，表示不关心用户传递的参数个数
 *  @perm       访问权限
 *
 * */
static int  array[3];
int         num;
module_param_array(array, int, &num, 0644);
MODULE_PARM_DESC(array, "Get an array from user...\n");




int __init  param_module_init(void)
{
    int index = 0;

    printk("\n---------------------\n");
    printk("value       : %d\n", value);
    printk("value_in    : %d\n", value_in);
    printk("string      : %s\n", string);
    printk("buffer      : %s\n", buffer);

    for(index = 0; index < num; index++)
    {
        printk("array[%2d]   :   %d\n", index, array[index]);
    }
    printk("---------------------\n");

    return 0;
}

void __exit param_module_exit(void)
{
    printk("\n---------------------\n");
    printk("exit param dobule\n");
    printk("---------------------\n");
}


module_init(param_module_init);
module_exit(param_module_exit);



```


##Makefile
-------
```


obj-m := param.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
```

##参数传递过程
-------
```
sudo insmod param.ko value=100 value_out=200 string="gatieme" buffer="Hello-World" array=100,200,300
```

![这里写图片描述](http://img.blog.csdn.net/20160401234127296)

dmesg查看

![这里写图片描述](http://img.blog.csdn.net/20160401235023283)

```
sudo rmmod param
```

![这里写图片描述](http://img.blog.csdn.net/20160401234335031)

##使用modinfo查看参数
```
modinfo -p param.ko

```

![这里写图片描述](http://img.blog.csdn.net/20160401235514098)

##动态修改模块参数
-------
首先查看一下sysfs目录下的本模块参数信息
```
ls /sys/module/param/parameters
```

![这里写图片描述](http://img.blog.csdn.net/20160401235926818)

动态修改
![这里写图片描述](http://img.blog.csdn.net/20160402000300569)