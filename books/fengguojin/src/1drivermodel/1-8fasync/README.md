例1.8  异步通知实例
=======


|    驱动    |    描述    | 书中对应章节 |
|:----------:|:----------:|:------------:|
|  1-8fasync            |  例1.8  异步通知实例                      |   [例1.8  异步通知实例](http://book.51cto.com/art/201205/337673.htm)                      |



#1.2.8  异步通知
-------

驱动程序与应用程序不能直接通信, 如果设备已经准备好数据，可以采用异步通知的方式通知应用层来读取，这样应用程序就不需要一直查询设备的状态。要支持异步通知，需要实现设备驱动程序的fasync接口。当一个打开的文件的FASYNC标志变化时file_operations ->fasync()接口将被调用.

file_operations ->fasync函数会调用fasync_helper从相关的进程列表中添加或去除异步通知关联.

```c
int fasync_helper(int fd, struct file *filp, int mode, struct fasync_struct **fa); 
```

当数据到达时 kill_fasync函数将被用来通知相关的进程：

```c
void kill_fasync(struct fasync_struct **fa, int sig, int band); 
```

#例1.8  异步通知实例
-------

代码见光盘\src\1drivermodel\1-8fasync。驱动层代码如下：

```c
struct simple_dev *simple_devices;  
static unsigned char simple_inc=0;  
static struct timer_list simple_timer;  
static struct fasync_struct *fasync_queue=NULL;  
int simple_open(struct inode *inode, struct file *filp)  
{  
    struct simple_dev *dev;  
    dev = container_of(inode->i_cdev, struct simple_dev, cdev);  
    filp->private_data = dev;  
    simple_timer.function = &simple_timer_handler;  
    simple_timer.expires = jiffies + 2*HZ;  
    add_timer (&simple_timer);  
    printk("add_timer...\n");  
    return 0;  
}

//异步通知处理函数  
static int simple_fasync(int fd, struct file * filp, int mode)   
{  
    int retval;  
    printk("simple_fasync...\n");  
    retval=fasync_helper(fd,filp,mode,&fasync_queue);  
    if(retval<0)  
      return retval;  
    return 0;  
}

int simple_release(struct inode *inode, struct file *filp)  
{  
    simple_fasync(-1, filp, 0);  
    return 0;  
}  

struct file_operations simple_fops = {  
    .owner =    THIS_MODULE,  
    .open =     simple_open,  
    .release=   simple_release,  
    .fasync=    simple_fasync,  
};  
```

当数据来临时通知应用层：

```c
static void simple_timer_handler( unsigned long data)  
{  
    printk("simple_timer_handler...\n");  
    if (fasync_queue)  
    {  
      //POLL_IN为可读，POLL_OUT为可写  
      kill_fasync(&fasync_queue, SIGIO, POLL_IN);  
      printk("kill_fasync...\n");  
    }  
    return ;  
}  
```

*   POLL_IN表示设备可读

*   POLL_OUT表示设备可写

应用层参考代码如下

```c
int fd;  
void fasync_handler(int num)  
{  
   printf("fasync_handler entering\n");  
}  
void main()  
{  
  int i=2;  
  char data[256];  
  int oflags=0;  
  int retval;  
  signal(SIGIO, fasync_handler);//注册信号处理函数  
  fd=open("/dev/fcn",O_RDWR);  
  if(fd==-1)  
  {  
     perror("error open\n");  
     exit(-1);  
  }  
  printf("open /dev/fcn successfully\n");  
  //使能了异步的通知到当前进程  
  fcntl(fd, F_SETOWN, getpid());  
  oflags=fcntl(fd, F_GETFL);  
  fcntl(fd, F_SETFL, oflags | FASYNC);//修改文件标志  
  while(1);  
  close(fd);  
}
```


运行结果如下：

```c
insmod demo.ko  
mknod /dev/gatieme c 224 0  
./test 
```