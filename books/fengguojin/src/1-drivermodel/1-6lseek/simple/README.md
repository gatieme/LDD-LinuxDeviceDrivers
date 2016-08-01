例1.5  字符设备ioctl实例
=======

|    驱动    |    描述    | 书中对应章节 |
|:----------:|:----------:|:------------:|
|  1-6lseek  |  例1.6  字符设备seek实例     |   [1.2.6 seek接口](http://book.51cto.com/art/201205/337671.htm)                   |

#1.2.6  seek接口
-------


seek接口用来对设备的读写位置进行定位。file_operations结构中对应的seek接口如下：

```c
loff_t (*llseek) (struct file *filp, loff_t off, int whence) 
```

其中off是偏移量，whence参数指起点位置。

#例1.6  字符设备seek实例
-------

代码见光盘\src\1drivermodel\1-6lseek。核心代码如下所示：

```c
ssize_t simple_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos)  
{  
    loff_t pos= *f_pos;//获取文件指针  
    if(pos>=256)  
    {  
        count=0;  
        goto out;  
    }  
    if(count>(256-pos))  
    {  
        count=256-pos;  
    }  
    pos += count;  
    //复制数据到指定的地址  
    if (copy_to_user(buf,demoBuffer+*f_pos,count))  
    {  
       count=-EFAULT;   
       goto out;  
    }  
    *f_pos = pos;  
 out:  
    return count;  
}

loff_t simple_llseek(struct file *filp, loff_t off, int whence)  
{  
    loff_t pos;  
    pos = filp->f_pos;  
    switch (whence)   
    {  
    case 0:  
        pos = off;  
        break;  
    case 1:  
        pos += off;  
        break;  
    case 2:  
        pos =255+off;  
        break;  
    default:  
        return -EINVAL;  
    }  
    if ((pos>=256) || (pos<0))   
    {  
        return -EINVAL;  
    }  
    return filp->f_pos=pos;  
}  
struct file_operations simple_fops = {  
    .owner =    THIS_MODULE,  
    .llseek =   simple_llseek,  
    .read =     simple_read,  
    .open =     simple_open,  
    .release =  simple_release,  
};  
```


应用程序参考代码如下：


```c
int main()  
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
    retval=lseek(fd,5,0);  
    if(retval==-1)  
    {  
        perror("lseek error\n");  
        exit(-1);  
    }  
    retval=read(fd,data,3);  
    if(retval==-1)  
    {  
        perror("read error\n");  
        exit(-1);  
    }  
    data[retval]=0;  
    printf("read successfully:%s\n",data);  
    //文件定位  
    retval=lseek(fd,2,0);  
    if(retval==-1)  
    {  
        perror("lseek error\n");  
        exit(-1);  
    }  
    retval=read(fd,data,3);  
    if(retval==-1)  
    {  
        perror("read error\n");  
        exit(-1);  
    }  
    data[retval]=0;  
    printf("read successfully:%s\n",data);  
    close(fd);  
}  
```

本例运行结果如下：

```c
insmod demo.ko  
mknod /dev/fgj c 224 0  
./test  
```