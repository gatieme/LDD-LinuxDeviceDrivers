/*======================================================================
    A globalmem driver as an example of char device drivers

    The initial developer of the original code is Baohua Song
    <author@linuxdriver.cn>. All Rights Reserved.
======================================================================*/



#include <linux/version.h>

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>

//  error: implicit declaration of function `kfree`
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 19, 0)
#include <asm/system.h>
#else
#include <linux/slab.h>
#endif

#include <asm/uaccess.h>

#define GLOBALMEM_SIZE      0x1000            /*  全局内存最大4K字节          */
#define MEM_CLEAR           0x1               /*  清0全局内存                 */
#define GLOBALMEM_MAJOR     300               /*  预设的globalmem的主设备号   */


/*  globalmem的设备号   */
static int globalmem_major = GLOBALMEM_MAJOR;
module_param(globalmem_major, int, S_IRUGO);


/*  globalmem设备结构体 */
struct globalmem_dev
{

    struct cdev cdev;                         /*  cdev结构体  */
    unsigned char mem[GLOBALMEM_SIZE];        /*  全局内存    */

};

struct globalmem_dev *globalmem_devp;         /*  设备结构体指针    */


/*  文件打开函数    */
int globalmem_open(struct inode *inode, struct file *filp)
{
    /*    将设备结构体指针赋值给文件私有数据指针  */
    filp->private_data = globalmem_devp;

    return 0;
}

/*  文件释放函数    */
int globalmem_release(struct inode *inode, struct file *filp)
{
    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
/* ioctl设备控制函数 */
static int globalmem_ioctl(
        struct inode *inodep,
        struct file *filp,
        unsigned int cmd,
        unsigned long arg)
{
#else
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
//long (*compat_ioctl) (struct file *file, unsigned int cmd, unsigned long arg)
static long globalmem_compat_ioctl(
        struct file *filp,
        unsigned int cmd,
        unsigned long arg)
{
    //struct inode *inode = file->f_dentry->d_inode;
    struct inode *inode = inode = file_inode(filp);
#endif

    struct globalmem_dev *dev = filp->private_data;/*获得设备结构体指针*/

    switch (cmd)
    {
        case MEM_CLEAR:
        {
            memset(dev->mem, 0, GLOBALMEM_SIZE);
            printk(KERN_INFO "globalmem is set to zero\n");
            break;
        }

        default:
        {
            return  - EINVAL;
        }
    }

    return 0;
}

/*读函数*/
static ssize_t globalmem_read(
        struct file *filp,
        char __user *buf,
        size_t      size,
        loff_t      *ppos)
{
    unsigned long           p =  *ppos;                /*   文件指针偏移 once = 0, twice = GLOBALMEM_SIZE  */
    unsigned int            count = size;              /*   GLOBALMEM_SIZE */
    int                     ret = 0;
    struct globalmem_dev    *dev = filp->private_data; /*获得设备结构体指针*/

    /*  分析和获取有效的写长度  */
    if (p >= GLOBALMEM_SIZE)
    {
        printk("*ppos = %d\n", p);
        /*
         * https://zhidao.baidu.com/question/136829890072623445.html
         * http://bbs.chinaunix.net/thread-1961861-1-1.html
         * http://blog.csdn.net/qiaoliang328/article/details/4874238
         * Fix bug when `cat /dev/ XXX`
         *
         * return count ? 0 : -ENXIO;  条件满足返回-ENXIO, 即No such device or address
         *
         * 首先cat命令read设备内容, 当返回值是大于0的时候,
         * cat会继续在open设备, 然后进行read操作(每次读4096字节)
         * 程序例子中第一次
         */
        return count ? -ENXIO : 0;
    }

    if (count > GLOBALMEM_SIZE - p)
    {
        count = GLOBALMEM_SIZE - p;
    }

    /*  内核空间->用户空间
     *
     *  将dev->mem中偏移p为起始的内存区域的数据, 拷贝count个字节到用户的buf中
     *  */
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
static ssize_t globalmem_write(
        struct file         *filp,
        const char __user   *buf,
        size_t              size,
        loff_t              *ppos)
{
    unsigned long p =  *ppos;
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev *dev = filp->private_data; /*获得设备结构体指针*/

    /*  分析和获取有效的写长度  */
    if (p >= GLOBALMEM_SIZE)
    {
        return count ?  - ENXIO: 0;
    }

    if (count > GLOBALMEM_SIZE - p)
    {
        count = GLOBALMEM_SIZE - p;
    }

    /*  用户空间->内核空间*/
    if (copy_from_user(dev->mem + p, buf, count) != 0)
    {
        ret =  - EFAULT;
    }
    else
    {
        *ppos += count;
        ret = count;

        printk(KERN_INFO "written %d bytes(s) from %d\n", count, p);
    }

    return ret;
}

/* seek文件定位函数 */
static loff_t globalmem_llseek(
        struct file *filp,
        loff_t      offset,
        int         orig)
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

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
    .ioctl = globalmem_ioctl,
#else
    //.unlocked_ioctl = globalmem_ioctl,
    .compat_ioctl = globalmem_compat_ioctl,
#endif

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
    {
        printk(KERN_NOTICE "Error %d adding LED%d", err, index);
    }
}

/*  设备驱动模块加载函数    */
int globalmem_init(void)
{
    int result;
    dev_t devno = MKDEV(globalmem_major, 0);

    /*  申请设备号  */
    if (globalmem_major != 0)
    {
        result = register_chrdev_region(devno, 1, "globalmem");
        printk(KERN_INFO "register char device drivers [globalmem], MAJOR = %d\n", globalmem_major);
    }
    else  /* 动态申请设备号 */
    {
        result = alloc_chrdev_region(&devno, 0, 1, "globalmem");
        globalmem_major = MAJOR(devno);
        printk(KERN_INFO "alloc char device drivers [globalmem], MAJOR = %d\n", globalmem_major);
    }

    if (result < 0)
    {
        return result;
    }

    /* 动态申请设备结构体的内存*/
    globalmem_devp = kmalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
    if (!globalmem_devp)    /*  申请失败    */
    {
        result =  - ENOMEM;
        unregister_chrdev_region(devno, 1);

        return result;
    }
    memset(globalmem_devp, 0, sizeof(struct globalmem_dev));

    globalmem_setup_cdev(globalmem_devp, 0);

    return 0;
}

/*模块卸载函数*/
void globalmem_exit(void)
{
    cdev_del(&globalmem_devp->cdev);   /*注销cdev*/

    kfree(globalmem_devp);     /*释放设备结构体内存*/

    unregister_chrdev_region(MKDEV(globalmem_major, 0), 1); /*释放设备号*/
}


//Driver Information
#define DRIVER_VERSION  "1.0.0"
#define DRIVER_AUTHOR   "Gatieme @ AderStep Inc..."
#define DRIVER_DESC     "Linux input module for Elo MultiTouch(MT) devices"
#define DRIVER_LICENSE  "Dual BSD/GPL"

// Kernel Module Information
MODULE_VERSION(DRIVER_VERSION);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);


module_init(globalmem_init);
module_exit(globalmem_exit);
