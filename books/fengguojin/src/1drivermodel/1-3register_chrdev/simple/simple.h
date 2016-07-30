#ifndef __SIMPLE_H_INCLUDE__
#define __SIMPLE_H_INCLUDE__

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/********************************************************
 * Macros to help debugging
 ********************************************************/
#undef PDEBUG             /* undef it, just in case */
#ifdef simple_DEBUG
#ifdef __KERNEL__
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "DEMO: " fmt, ## args)
#else//usr space
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

//  设备号
//  error inserting 'globalmem.ko': -1 Device or resource busy
//  出现这个原因是定义的设备号有冲突,
//  可以通过cat /proc/devices查看有哪些设备号还没被使用
//  或者直接由系统动态分配主设备号
#define simple_MAJOR 224




//  设备结构
struct simple_dev
{
	struct cdev cdev;	  /* Char device structure		*/
};





/////////////////////
//函数申明
/////////////////////

/*  open打开设备文件  */
int simple_open(struct inode *inode, struct file *filp);

/*  close释放文件操作  */
int simple_release(struct inode *inode, struct file *filp);


/*  read读设备文件操作  */
ssize_t simple_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos);

/*  write写设备文件写操作  */
ssize_t simple_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos);

/*  文件定位操作  */
loff_t  simple_llseek(struct file *filp, loff_t off, int whence);

/*  设备控制操作  */
int     simple_ioctl(struct inode *inode, struct file *filp,
                    unsigned int cmd, unsigned long arg);


#endif  //  #endif  __SIMPLE_H_INCLUDE__
