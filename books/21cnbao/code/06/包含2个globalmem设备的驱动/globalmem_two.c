/*======================================================================
    A globalmem driver as an example of char device drivers
    There are two same globalmems in this driver  
    This example is to introduce the function of file->private_data
    
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
#include <asm/system.h>
#include <asm/uaccess.h>

#define GLOBALMEM_SIZE	0x1000	/*全局内存最大4K字节*/
#define MEM_CLEAR 0x1  /*清0全局内存*/
#define GLOBALMEM_MAJOR 254    /*预设的globalmem的主设备号*/

static globalmem_major = GLOBALMEM_MAJOR;
/*globalmem设备结构体*/
struct globalmem_dev                                     
{                                                        
  struct cdev cdev; /*cdev结构体*/                       
  unsigned char mem[GLOBALMEM_SIZE]; /*全局内存*/        
};

struct globalmem_dev *globalmem_devp; /*设备结构体指针*/
/*文件打开函数*/
int globalmem_open(struct inode *inode, struct file *filp)
{
  /*将设备结构体指针赋值给文件私有数据指针*/
  struct globalmem_dev *dev;
  
  dev = container_of(inode->i_cdev,struct globalmem_dev,cdev);  
  filp->private_data = dev;  
  return 0;
}
/*文件释放函数*/
int globalmem_release(struct inode *inode, struct file *filp)
{
  return 0;
}

/* ioctl设备控制函数 */
static int globalmem_ioctl(struct inode *inodep, struct file *filp, unsigned
  int cmd, unsigned long arg)
{
  struct globalmem_dev *dev = filp->private_data;/*获得设备结构体指针*/

  switch (cmd)
  {
    case MEM_CLEAR:
      memset(dev->mem, 0, GLOBALMEM_SIZE);      
      printk(KERN_INFO "globalmem is set to zero\n");
      break;

    default:
      return  - EINVAL;
  }
  return 0;
}

/*读函数*/
static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size,
  loff_t *ppos)
{
  unsigned long p =  *ppos;
  unsigned int count = size;
  int ret = 0;
  struct globalmem_dev *dev = filp->private_data; /*获得设备结构体指针*/

  /*分析和获取有效的写长度*/
  if (p >= GLOBALMEM_SIZE)
    return count ?  - ENXIO: 0;
  if (count > GLOBALMEM_SIZE - p)
    count = GLOBALMEM_SIZE - p;

  /*内核空间->用户空间*/
  if (copy_to_user(buf, (void*)(dev->mem + p), count))
  {
    ret =  - EFAULT;
  }
  else
  {
    *ppos += count;
    ret = count;
    
    printk(KERN_INFO "read %d bytes(s) from %d\n", count, p);
  }

  return ret;
}

/*写函数*/
static ssize_t globalmem_write(struct file *filp, const char __user *buf,
  size_t size, loff_t *ppos)
{
  unsigned long p =  *ppos;
  unsigned int count = size;
  int ret = 0;
  struct globalmem_dev *dev = filp->private_data; /*获得设备结构体指针*/
  
  /*分析和获取有效的写长度*/
  if (p >= GLOBALMEM_SIZE)
    return count ?  - ENXIO: 0;
  if (count > GLOBALMEM_SIZE - p)
    count = GLOBALMEM_SIZE - p;
    
  /*用户空间->内核空间*/
  if (copy_from_user(dev->mem + p, buf, count))
    ret =  - EFAULT;
  else
  {
    *ppos += count;
    ret = count;
    
    printk(KERN_INFO "written %d bytes(s) from %d\n", count, p);
  }

  return ret;
}

/* seek文件定位函数 */
static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)
{
  loff_t ret = 0;
  switch (orig)
  {
    case 0:   /*相对文件开始位置偏移*/
      if (offset < 0)
      {
        ret =  - EINVAL;
        break;
      }
      if ((unsigned int)offset > GLOBALMEM_SIZE)
      {
        ret =  - EINVAL;
        break;
      }
      filp->f_pos = (unsigned int)offset;
      ret = filp->f_pos;
      break;
    case 1:   /*相对文件当前位置偏移*/
      if ((filp->f_pos + offset) > GLOBALMEM_SIZE)
      {
        ret =  - EINVAL;
        break;
      }
      if ((filp->f_pos + offset) < 0)
      {
        ret =  - EINVAL;
        break;
      }
      filp->f_pos += offset;
      ret = filp->f_pos;
      break;
    default:
      ret =  - EINVAL;
      break;
  }
  return ret;
}

/*文件操作结构体*/
static const struct file_operations globalmem_fops =
{
  .owner = THIS_MODULE,
  .llseek = globalmem_llseek,
  .read = globalmem_read,
  .write = globalmem_write,
  .ioctl = globalmem_ioctl,
  .open = globalmem_open,
  .release = globalmem_release,
};

/*初始化并注册cdev*/
static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
  int err, devno = MKDEV(globalmem_major, index);

  cdev_init(&dev->cdev, &globalmem_fops);
  dev->cdev.owner = THIS_MODULE;
  dev->cdev.ops = &globalmem_fops;
  err = cdev_add(&dev->cdev, devno, 1);
  if (err)
    printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}

/*设备驱动模块加载函数*/
int globalmem_init(void)
{
  int result;
  dev_t devno = MKDEV(globalmem_major, 0);

  /* 申请设备号*/
  if (globalmem_major)
    result = register_chrdev_region(devno, 2, "globalmem");
  else  /* 动态申请设备号 */
  {
    result = alloc_chrdev_region(&devno, 0, 2, "globalmem");
    globalmem_major = MAJOR(devno);
  }  
  if (result < 0)
    return result;
    
  /* 动态申请2个设备结构体的内存*/
  globalmem_devp = kmalloc(2*sizeof(struct globalmem_dev), GFP_KERNEL);
  if (!globalmem_devp)    /*申请失败*/
  {
    result =  - ENOMEM;
    goto fail_malloc;
  }
  memset(globalmem_devp, 0, 2*sizeof(struct globalmem_dev));
  
  globalmem_setup_cdev(&globalmem_devp[0], 0);
  globalmem_setup_cdev(&globalmem_devp[1], 1);
  return 0;

  fail_malloc: unregister_chrdev_region(devno, 1);
  return result;
}

/*模块卸载函数*/
void globalmem_exit(void)
{
  cdev_del(&(globalmem_devp[0].cdev));   
  cdev_del(&(globalmem_devp[1].cdev));   /*注销cdev*/
  kfree(globalmem_devp);     /*释放设备结构体内存*/
  unregister_chrdev_region(MKDEV(globalmem_major, 0), 2); /*释放设备号*/
}

MODULE_AUTHOR("Song Baohua");
MODULE_LICENSE("Dual BSD/GPL");

module_param(globalmem_major, int, S_IRUGO);

module_init(globalmem_init);
module_exit(globalmem_exit);
