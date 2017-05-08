/*======================================================================
    A VirtualDisk driver as an example of char device drivers

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
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/version.h>


#define VIRTUALDISK_SIZE	0x2000	/*全局内存最大8K字节*/
#define MEM_CLEAR 0x1  /*全局内存清零*/
#define PORT1_SET 0x2  /*将port1端口清零*/
#define PORT2_SET 0x3  /*将port2端口清零*/
/*
#define VirtualDisk_MAGIC 100
#define MEM_CLEAR _IO(VirtualDisk_MAGIC,0)
#define PORT1_SET _IO(VirtualDisk_MAGIC,1)
#define PORT2_SET _IO(VirtualDisk_MAGIC,2)
*/
#define VIRTUALDISK_MAJOR 200    /*预设的VirtualDisk的主设备号为200*/

static int VirtualDisk_major = VIRTUALDISK_MAJOR;
/*VirtualDisk设备结构体*/
struct VirtualDisk
{
  struct cdev cdev; /*cdev结构体*/
  unsigned char mem[VIRTUALDISK_SIZE]; /*全局内存8K*/
  int port1; /*两个不同类型的端口*/
  long port2;
  long count;  /*记录设备目前被多少设备打开*/
};

struct VirtualDisk *Virtualdisk_devp; /*设备结构体指针*/
/*文件打开函数*/
int VirtualDisk_open(struct inode *inode, struct file *filp)
{
  struct VirtualDisk *devp = NULL;

  /*将设备结构体指针赋值给文件私有数据指针*/
  filp->private_data = Virtualdisk_devp;
  devp = filp->private_data;/*获得设备结构体指针*/
  devp->count++;/*增加设备打开次数*/

  return 0;
}
/*文件释放函数*/
int VirtualDisk_release(struct inode *inode, struct file *filp)
{
  struct VirtualDisk *devp = filp->private_data;/*获得设备结构体指针*/
  devp->count--;/*减少设备打开次数*/
  return 0;
}


/* ioctl设备控制函数 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
static int VirtualDisk_ioctl(
        struct inode *inodep,
        struct file *filp,
        unsigned int cmd,
        unsigned long arg)
{
#else
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
//long (*compat_ioctl) (struct file *file, unsigned int cmd, unsigned long arg)
static long VirtualDisk_unlocked_ioctl(
        struct file *filp,
        unsigned int cmd,
        unsigned long arg)
{
  //struct inode *inode = filp->f_dentry->d_inode;
  struct inode *inode = inode = file_inode(filp);
#endif
  struct VirtualDisk *devp = filp->private_data;/*获得设备结构体指针*/

  switch (cmd)
  {
    case MEM_CLEAR:/*设备内存清零*/
      memset(devp->mem, 0, VIRTUALDISK_SIZE);
      printk(KERN_INFO "VirtualDisk is set to zero\n");
      break;
	case PORT1_SET:/*将端口1置0*/
	  devp->port1=0;
	  break;
	case PORT2_SET:/*将端口2置0*/
	  devp->port2=0;
	  break;
    default:
      return  - EINVAL;
  }
  return 0;
}

/*读函数*/
static ssize_t VirtualDisk_read(struct file *filp, char __user *buf, size_t size,
  loff_t *ppos)
{
  unsigned long p =  *ppos; /*记录文件指针偏移位置*/
  unsigned int count = size;/*记录需要读取的字节数*/
  int ret = 0;/*返回值*/
  struct VirtualDisk *devp = filp->private_data; /*获得设备结构体指针*/

  printk("p = %ld\n", p);
  /*分析和获取有效的读长度*/
  if (p >= VIRTUALDISK_SIZE)  /*要读取的偏移大于设备的内存空间*/
    return count ?  - ENXIO: 0;/*读取地址错误*/
  if (count > VIRTUALDISK_SIZE - p)/*要读取的字节大于设备的内存空间*/
    count = VIRTUALDISK_SIZE - p;/*将要读取的字节数设为剩余的字节数*/
  /*内核空间->用户空间交换数据*/
  if (copy_to_user(buf, (void*)(devp->mem + p), count))
  {
    ret =  - EFAULT;
  }
  else
  {
    *ppos += count;
    ret = count;
    printk(KERN_INFO "read %d bytes(s) from %ld\n", count, p);
  }
  return ret;
}

/*写函数*/
static ssize_t VirtualDisk_write(struct file *filp, const char __user *buf,
  size_t size, loff_t *ppos)
{
  unsigned long p =  *ppos; /*记录文件指针偏移位置*/
  int ret = 0;  /*返回值*/
  unsigned int count = size;/*记录需要写入的字节数*/
  struct VirtualDisk *devp = filp->private_data; /*获得设备结构体指针*/

  /*分析和获取有效的写长度*/
  if (p >= VIRTUALDISK_SIZE)/*要写入的偏移大于设备的内存空间*/
    return count ?  - ENXIO: 0;/*写入地址错误*/
  if (count > VIRTUALDISK_SIZE - p)/*要写入的字节大于设备的内存空间*/
    count = VIRTUALDISK_SIZE - p;/*将要写入的字节数设为剩余的字节数*/

  /*用户空间->内核空间*/
  if (copy_from_user(devp->mem + p, buf, count))
    ret =  - EFAULT;
  else
  {
    *ppos += count;/*增加偏移位置*/
    ret = count;/*返回实际的写入字节数*/
    printk(KERN_INFO "written %d bytes(s) from %ld\n", count, p);
  }
  return ret;
}

/* seek文件定位函数 */
static loff_t VirtualDisk_llseek(struct file *filp, loff_t offset, int orig)
{
  loff_t ret = 0;/*返回的位置偏移*/
  switch (orig)
  {
    case SEEK_SET:   /*相对文件开始位置偏移*/
      if (offset < 0)/*offset不合法*/
      {
        ret =  - EINVAL;		/*无效的指针*/
        break;
      }
      if ((unsigned int)offset > VIRTUALDISK_SIZE)/*偏移大于设备内存*/
      {
        ret =  - EINVAL;		/*无效的指针*/
        break;
      }
      filp->f_pos = (unsigned int)offset;	/*更新文件指针位置*/
      ret = filp->f_pos;/*返回的位置偏移*/
      break;
    case SEEK_CUR:   /*相对文件当前位置偏移*/
      if ((filp->f_pos + offset) > VIRTUALDISK_SIZE)/*偏移大于设备内存*/
      {
        ret =  - EINVAL;/*无效的指针*/
        break;
      }
      if ((filp->f_pos + offset) < 0)/*指针不合法*/
      {
        ret =  - EINVAL;/*无效的指针*/
        break;
      }
      filp->f_pos += offset;/*更新文件指针位置*/
      ret = filp->f_pos;/*返回的位置偏移*/
      break;
    default:
      ret =  - EINVAL;/*无效的指针*/
      break;
  }
  return ret;
}

/*文件操作结构体*/
static const struct file_operations VirtualDisk_fops =
{
  .owner = THIS_MODULE,
  .llseek = VirtualDisk_llseek,/*定位偏移量函数*/
  .read = VirtualDisk_read,/*读设备函数*/
  .write = VirtualDisk_write,/*写设备函数*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
  .ioctl = VirtualDisk_ioctl,/*控制函数*/
#else
  .compat_ioctl = VirtualDisk_unlocked_ioctl,
#endif
  .open = VirtualDisk_open,/*打开设备函数*/
  .release = VirtualDisk_release,/*释放设备函数*/
};

/*初始化并注册cdev*/
static void VirtualDisk_setup_cdev(struct VirtualDisk *dev, int minor)
{
  int err;
  dev_t devno = MKDEV(VirtualDisk_major, minor);/*构造设备号*/

  cdev_init(&dev->cdev, &VirtualDisk_fops);/*初始化cdev设备*/
  dev->cdev.owner = THIS_MODULE;/*使驱动程序属于该模块*/
  dev->cdev.ops = &VirtualDisk_fops;/*cdev连接file_operations指针*/

  err = cdev_add(&dev->cdev, devno, 1);/*将cdev注册到系统中*/

  if (err)
    printk(KERN_NOTICE "Error in cdev_add()\n");
}
/*设备驱动模块加载函数*/
int VirtualDisk_init(void)
{
  int result;
  dev_t devno = MKDEV(VirtualDisk_major, 0); /*构建设备号*/

  /* 申请设备号*/
  if (VirtualDisk_major)  /* 如果不为0，则静态申请*/
    result = register_chrdev_region(devno, 1, "VirtualDisk");
  else  /* 动态申请设备号 */
  {
    result = alloc_chrdev_region(&devno, 0, 1, "VirtualDisk");
    VirtualDisk_major = MAJOR(devno);/* 从申请设备号中得到主设备号 */
  }
  if (result < 0)
    return result;

  /* 动态申请设备结构体的内存*/
  Virtualdisk_devp = kmalloc(sizeof(struct VirtualDisk), GFP_KERNEL);
  if (!Virtualdisk_devp)    /*申请失败*/
  {
    result =  - ENOMEM;
    goto fail_kmalloc;
  }
  memset(Virtualdisk_devp, 0, sizeof(struct VirtualDisk));/*将内存清零*/
  /*初始化并且添加cdev结构体*/
  VirtualDisk_setup_cdev(Virtualdisk_devp, 0);
  return 0;

  fail_kmalloc:
  unregister_chrdev_region(devno, 1);
  return result;
}

/*模块卸载函数*/
void VirtualDisk_exit(void)
{
  cdev_del(&Virtualdisk_devp->cdev);   /*注销cdev*/
  kfree(Virtualdisk_devp);     /*释放设备结构体内存*/
  unregister_chrdev_region(MKDEV(VirtualDisk_major, 0), 1); /*释放设备号*/
}

MODULE_AUTHOR("Zheng Qiang");
MODULE_LICENSE("Dual BSD/GPL");

module_param(VirtualDisk_major, int, S_IRUGO);

module_init(VirtualDisk_init);
module_exit(VirtualDisk_exit);
