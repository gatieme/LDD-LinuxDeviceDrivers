例1.7  poll接口驱动程序示例
=======

|    驱动    |    描述    | 书中对应章节 |
|:----------:|:----------:|:------------:|
|  1-7poll              |  例1.7  poll接口驱动程序示例              |   [1.2.7 poll接口](http://book.51cto.com/art/201205/337672.htm)                   |



#1.2.7  poll接口
-------

如果设备被配置成阻塞式操作，即当设备执行I/O操作时如果不能获得数据将阻塞，直到获得数据.

应用层可以使用select函数查询设备当前的状态，以便用户程序获知是否能对设备进行非阻塞的访问.

使用select函数需要在设备驱动程序中添加file_operations->poll接口支持.

一个典型的字符驱动程序的file_operations->poll函数的实现如下

```c
static unsigned int my_poll(struct file *file, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    poll_wait(file, & outq, wait);//把当前进程添加到等待列表
    if (0 != bta->read_count)//如果有数据
        mask |= (POLLIN | POLLRDNORM);
    return mask;
}
```


驱动程序中的poll函数返回的标志如下：

```c
#define POLLIN            1 //设备可以无阻塞地读取
#define POLLPRI       2//可以无阻塞地读取高优先级数据(带外数据)
#define POLLOUT       4//设备可以无阻塞地写入
#define POLLERR       8//设备发生错误
#define POLLHUP       16//当读取设备的进程到达文件尾部
#define POLLNVAL          32 //请求无效
#define POLLRDNORM    64//常规数据已经就绪
#define POLLWRNORM  POLLOUT
#define POLLRDBAND    128//可以从设备读带外数据
#define POLLWRBAND    256//可以向设备写带外数据
#define POLLMSG       0x0400
#define POLLREMOVE   0x1000
#define POLLRDHUP    0x2000
```

应用层多路I/O选择函数select的原型如下：

```c
int select(int numfds, fd_set *readfds, fd_set
*writefds, fd_set *exceptfds, struct timeval *timeout);
```

其中readfds、writefds、exceptfds分别是被select函数监视的读、写和异常处理的文件描述符集合，numfds的值是需要检查的号码最高的文件描述符加1。timeout参数是一个指向struct timeval类型的指针，它可以使select函数在等待timeout时间后若没有文件描述符准备好则返回。文件描述符集常用函数接口如下：

```c
FD_ZERO(fd_set *set)//清除一个文件描述符集；

FD_SET(int fd,fd_set *set)//将文件描述符fd加入文件描述符集中；

FD_CLR(int fd,fd_set *set)//将文件描述符fd从文件描述符集中清除；

FD_ISSET(int fd,fd_set *set)//判断文件描述符fd是否被置位。
```

#例1.7  poll接口驱动程序示例
-------

代码见光盘\src\1drivermodel\1-7poll

核心代码如下所示：

```c
FD_ZERO(fd_set *set)//清除一个文件描述符集；

FD_SET(int fd,fd_set *set)//将文件描述符fd加入文件描述符集中；

FD_CLR(int fd,fd_set *set)//将文件描述符fd从文件描述符集中清除；
FD_ISSET(int fd,fd_set *set)//判断文件描述符fd是否被置位
```

#例1.7  poll接口驱动程序示例
-------

代码见光盘\src\1drivermodel\1-7poll

核心代码如下所示


```c
ssize_t simple_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos)
{
    //printk("wait_event_interruptible before\n");
    wait_event_interruptible(read_queue, simple_flag);
    //printk("wait_event_interruptible after\n");
    if (copy_to_user(buf,demoBuffer,count))
    {
        count=-EFAULT;
    }
    return count;
}

ssize_t simple_write(struct file *filp, const char __user *buf, size_t count,loff_t *f_pos)
{
    if (copy_from_user(demoBuffer, buf, count))
    {
        count = -EFAULT;
        goto out;
    }
    simple_flag=1;
    wake_up(&read_queue);
out:
    return count;
}

//poll接口实现
unsigned int simple_poll(struct file * file, poll_table * pt)
{
    unsigned int mask = POLLIN | POLLRDNORM;
    poll_wait(file, &read_queue, pt);
    return mask;
}
struct file_operations simple_fops = {
    .owner =    THIS_MODULE,
    .poll =     simple_poll,
    .read =     simple_read,
    .write=     simple_write,
    .open =     simple_open,
    .release =  simple_release,
};
```

应用程序参考代码如下：

```c
int fd;
void *readthread(void *arg)//读数据线程
{
    char data[256];
    fd_set rfds; //读描述符集合
    fd_set wfds; //写描述符集合
    int retval=0;
    while(1)
    {
        FD_ZERO(&rfds);
         FD_SET(fd, &rfds);
        select(fd+1, &rfds, &wfds, NULL, NULL); //多路选择
        if(FD_ISSET(fd, &rfds))
        {
            retval=read(fd,data,3);
            if(retval==-1)
            {
                perror("read error\n");
                exit(-1);
            }
            data[retval]=0;
            printf("read successfully:%s\n",data);
        }
    }
    return (void *)0;
}
void main()
{
    int i;
    int retval;
    fd=open("/dev/fgj",O_RDWR);
    if(fd==-1)
    {
        perror("error open\n");
        exit(-1);
    }
    printf("open /dev/fgj successfully\n");
    pthread_t tid;
    pthread_create(&tid, NULL, readthread, NULL);//创建读线程
    while(1)
    {
        retval=write(fd,"fgj",3);//主线程负责写数据
        if(retval==-1)
        {
            perror("write error\n");
            exit(-1);
        }
    }
    close(fd);
}
```

本例运行结果如下：


```c
insmod demo.ko
mknod /dev/fgj c 224 0
./test
```
