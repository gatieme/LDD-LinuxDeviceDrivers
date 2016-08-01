#例1.5  字符设备ioctl实例
-------

|    驱动    |    描述    | 书中对应章节 |
|:----------:|:----------:|:------------:|
|  1-5ioctl  |  例1.5  字符设备ioctl实例    |   [1.2.5 ioctl接口](http://book.51cto.com/art/201205/337670.htm)                  |



#1.2.5  ioctl接口
-------

ioctl是设备驱动程序中对设备的I/O通道进行管理的函数,
程序可以通过ioctl向设备发送命令、参数配置等信息.

file_operations结构中对应的ioctl接口如下:

```c
int (*ioctl) (struct inode *inode, struct file
*filp,unsigned int cmd, unsigned long arg);
```

其中cmd是命令类型，arg是参数。

#例1.5  字符设备ioctl实例

代码见光盘\src\1drivermodel\1-5ioctl

核心代码如下所示：

```c
//内核ioctl接口
int simple_ioctl(struct inode *inode, struct file *filp,unsigned int cmd, unsigned long arg)
{
    switch(cmd)
    {
        case COMMAND1:
            memset(demoBuffer,0x31,256);
            break;
        case COMMAND2:
            memset(demoBuffer,0x32,256);
            break;
        default:
            return -EFAULT;
            break;
    }
    return 0;
}
struct file_operations simple_fops = {
    .owner =    THIS_MODULE,
    .ioctl =    simple_ioctl,
    .open =     simple_open,
    .release =  simple_release,
};
```


接下来编写一个应用程序，参考代码如下：

```c
#define COMMAND1 1
#define COMMAND2 2

int main(void)
{
    int fd;
    int i;
    char data[256];
    int retval;
    fd=open("/dev/fgj",O_RDWR);
    if(fd==-1)
    {
        perror("error open\n");
        exit(-1);
    }
    printf("open /dev/fgj successfully\n");
    //应用层IOCTL控制
    retval=ioctl(fd,COMMAND1,0);
    if(retval==-1)
    {
        perror("ioctl error\n");
        exit(-1);
    }
    printf("send command1 successfully\n");
    retval=ioctl(fd,COMMAND2,0);
    if(retval==-1)
    {
        perror("ioctl error\n");
        exit(-1);
    }
    printf("send command2 successfully\n");
    close(fd);
}
```

本例运行结果如下

```c
insmod demo.ko
mknod /dev/fgj c 224 0
./test
```

#error:unknown field 'ioctl' specified in initializer
-------

在linux-2.6.36以上的内核加载编译驱动时，出现

>error:unknown field 'ioctl' specified in initializer

原因是：在2.6.36内核上file_operations发生了重大的改变：

原先的

```c
int (*ioctl)(struct inode*, struct file*, unsigned int, unsigned long);
```

被改为了

```c
long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);

long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
```


因而在实际驱动中,
我们需要将原先的写的ioctl函数头给改成下面的unlocked_ioctl
在file_operations结构体的填充中也是一样。
