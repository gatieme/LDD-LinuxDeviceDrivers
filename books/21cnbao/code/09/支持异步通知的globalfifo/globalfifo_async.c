/*======================================================================
    A globalfifo driver as an example of char device drivers
    This example is to introduce asynchronous notifier

    The initial developer of the original code is Baohua Song
    <author@linuxdriver.cn>. All Rights Reserved.
======================================================================*/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>

//#include <asm/system.h>
#include <linux/slab.h>

#include <asm/uaccess.h>
#include <linux/poll.h>

#define GLOBALFIFO_SIZE	0x1000	/*全局fifo最大4K字节*/
#define FIFO_CLEAR 0x1  /*清0全局内存的长度*/
#define GLOBALFIFO_MAJOR 253    /*预设的globalfifo的主设备号*/

static int globalfifo_major = GLOBALFIFO_MAJOR;
/*globalfifo设备结构体*/
struct globalfifo_dev
{
  struct cdev cdev; /*cdev结构体*/
  unsigned int current_len;    /*fifo有效数据长度*/
  unsigned char mem[GLOBALFIFO_SIZE]; /*全局内存*/
  struct semaphore sem; /*并发控制用的信号量*/
  wait_queue_head_t r_wait; /*阻塞读用的等待队列头*/
  wait_queue_head_t w_wait; /*阻塞写用的等待队列头*/
  struct fasync_struct *async_queue; /* 异步结构体指针，用于读 */
};

struct globalfifo_dev *globalfifo_devp; /*设备结构体指针*/
/*文件打开函数*/
int globalfifo_open(struct inode *inode, struct file *filp)
{
  /*将设备结构体指针赋值给文件私有数据指针*/
  filp->private_data = globalfifo_devp;
  return 0;
}
/*文件释放函数*/
int globalfifo_release(struct inode *inode, struct file *filp)
{
	/* 将文件从异步通知列表中删除 */
  globalmem_fasync( - 1, filp, 0);

  return 0;
}

/* ioctl设备控制函数 */
static int globalfifo_ioctl(struct inode *inodep, struct file *filp, unsigned
  int cmd, unsigned long arg)
{
  struct globalfifo_dev *dev = filp->private_data;/*获得设备结构体指针*/

  switch (cmd)
  {
    case FIFO_CLEAR:
    	down(&dev->sem); //获得信号量
      dev->current_len = 0;
      memset(dev->mem,0,GLOBALFIFO_SIZE);
      up(&dev->sem); //释放信号量

      printk(KERN_INFO "globalfifo is set to zero\n");
      break;

    default:
      return  - EINVAL;
  }
  return 0;
}

static unsigned int globalfifo_poll(struct file *filp, poll_table *wait)
{
  unsigned int mask = 0;
  struct globalfifo_dev *dev = filp->private_data; /*获得设备结构体指针*/

  down(&dev->sem);

  poll_wait(filp, &dev->r_wait, wait);
  poll_wait(filp, &dev->w_wait, wait);
  /*fifo非空*/
  if (dev->current_len != 0)
  {
    mask |= POLLIN | POLLRDNORM; /*标示数据可获得*/
  }
  /*fifo非满*/
  if (dev->current_len != GLOBALFIFO_SIZE)
  {
    mask |= POLLOUT | POLLWRNORM; /*标示数据可写入*/
  }

  up(&dev->sem);
  return mask;
}

/* globalfifo fasync函数*/
static int globalfifo_fasync(int fd, struct file *filp, int mode)
{
	struct globalfifo_dev *dev = filp->private_data;
	return fasync_helper(fd, filp, mode, &dev->async_queue);
}


/*globalfifo读函数*/
static ssize_t globalfifo_read(struct file *filp, char __user *buf, size_t count,
  loff_t *ppos)
{
  int ret;
  struct globalfifo_dev *dev = filp->private_data; //获得设备结构体指针
  DECLARE_WAITQUEUE(wait, current); //定义等待队列

  down(&dev->sem); //获得信号量
  add_wait_queue(&dev->r_wait, &wait); //进入读等待队列头

  /* 等待FIFO非空 */
  if (dev->current_len == 0)
  {
    if (filp->f_flags &O_NONBLOCK)
    {
      ret =  - EAGAIN;
      goto out;
    }
    __set_current_state(TASK_INTERRUPTIBLE); //改变进程状态为睡眠
    up(&dev->sem);

    schedule(); //调度其他进程执行
    if (signal_pending(current))
    //如果是因为信号唤醒
    {
      ret =  - ERESTARTSYS;
      goto out2;
    }

    down(&dev->sem);
  }

  /* 拷贝到用户空间 */
  if (count > dev->current_len)
    count = dev->current_len;

  if (copy_to_user(buf, dev->mem, count))
  {
    ret =  - EFAULT;
    goto out;
  }
  else
  {
    memcpy(dev->mem, dev->mem + count, dev->current_len - count); //fifo数据前移
    dev->current_len -= count; //有效数据长度减少
    printk(KERN_INFO "read %d bytes(s),current_len:%d\n", count, dev->current_len);

    wake_up_interruptible(&dev->w_wait); //唤醒写等待队列

    ret = count;
  }
  out: up(&dev->sem); //释放信号量
  out2:remove_wait_queue(&dev->w_wait, &wait); //从附属的等待队列头移除
  set_current_state(TASK_RUNNING);
  return ret;
}


/*globalfifo写操作*/
static ssize_t globalfifo_write(struct file *filp, const char __user *buf,
  size_t count, loff_t *ppos)
{
  struct globalfifo_dev *dev = filp->private_data; //获得设备结构体指针
  int ret;
  DECLARE_WAITQUEUE(wait, current); //定义等待队列

  down(&dev->sem); //获取信号量
  add_wait_queue(&dev->w_wait, &wait); //进入写等待队列头

  /* 等待FIFO非满 */
  if (dev->current_len == GLOBALFIFO_SIZE)
  {
    if (filp->f_flags &O_NONBLOCK)
    //如果是非阻塞访问
    {
      ret =  - EAGAIN;
      goto out;
    }
    __set_current_state(TASK_INTERRUPTIBLE); //改变进程状态为睡眠
    up(&dev->sem);

    schedule(); //调度其他进程执行
    if (signal_pending(current))
    //如果是因为信号唤醒
    {
      ret =  - ERESTARTSYS;
      goto out2;
    }

    down(&dev->sem); //获得信号量
  }

  /*从用户空间拷贝到内核空间*/
  if (count > GLOBALFIFO_SIZE - dev->current_len)
    count = GLOBALFIFO_SIZE - dev->current_len;

  if (copy_from_user(dev->mem + dev->current_len, buf, count))
  {
    ret =  - EFAULT;
    goto out;
  }
  else
  {
    dev->current_len += count;
    printk(KERN_INFO "written %d bytes(s),current_len:%d\n", count, dev
      ->current_len);

    wake_up_interruptible(&dev->r_wait); //唤醒读等待队列
    /* 产生异步读信号 */
    if (dev->async_queue)
       kill_fasync(&dev->async_queue, SIGIO, POLL_IN);

    ret = count;
  }

  out: up(&dev->sem); //释放信号量
  out2:remove_wait_queue(&dev->w_wait, &wait); //从附属的等待队列头移除
  set_current_state(TASK_RUNNING);
  return ret;
}


/*文件操作结构体*/
static const struct file_operations globalfifo_fops =
{
  .owner = THIS_MODULE,
  .read = globalfifo_read,
  .write = globalfifo_write,
  .ioctl = globalfifo_ioctl,
  .poll = globalfifo_poll,
  .open = globalfifo_open,
  .release = globalfifo_release,
  .fasync = globalfifo_fasync,
};

/*初始化并注册cdev*/
static void globalfifo_setup_cdev(struct globalfifo_dev *dev, int index)
{
  int err, devno = MKDEV(globalfifo_major, index);

  cdev_init(&dev->cdev, &globalfifo_fops);
  dev->cdev.owner = THIS_MODULE;
  dev->cdev.ops = &globalfifo_fops;
  err = cdev_add(&dev->cdev, devno, 1);
  if (err)
    printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}

/*设备驱动模块加载函数*/
int globalfifo_init(void)
{
  int ret;
  dev_t devno = MKDEV(globalfifo_major, 0);

  /* 申请设备号*/
  if (globalfifo_major)
    ret = register_chrdev_region(devno, 1, "globalfifo");
  else  /* 动态申请设备号 */
  {
    ret = alloc_chrdev_region(&devno, 0, 1, "globalfifo");
    globalfifo_major = MAJOR(devno);
  }
  if (ret < 0)
    return ret;
  /* 动态申请设备结构体的内存*/
  globalfifo_devp = kmalloc(sizeof(struct globalfifo_dev), GFP_KERNEL);
  if (!globalfifo_devp)    /*申请失败*/
  {
    ret =  - ENOMEM;
    goto fail_malloc;
  }

  memset(globalfifo_devp, 0, sizeof(struct globalfifo_dev));

  globalfifo_setup_cdev(globalfifo_devp, 0);

  init_MUTEX(&globalfifo_devp->sem);   /*初始化信号量*/
  init_waitqueue_head(&globalfifo_devp->r_wait); /*初始化读等待队列头*/
  init_waitqueue_head(&globalfifo_devp->w_wait); /*初始化写等待队列头*/

  return 0;

  fail_malloc: unregister_chrdev_region(devno, 1);
  return ret;
}


/*模块卸载函数*/
void globalfifo_exit(void)
{
  cdev_del(&globalfifo_devp->cdev);   /*注销cdev*/
  kfree(globalfifo_devp);     /*释放设备结构体内存*/
  unregister_chrdev_region(MKDEV(globalfifo_major, 0), 1); /*释放设备号*/
}

MODULE_AUTHOR("Song Baohua");
MODULE_LICENSE("Dual BSD/GPL");

module_param(globalfifo_major, int, S_IRUGO);

module_init(globalfifo_init);
module_exit(globalfifo_exit);
