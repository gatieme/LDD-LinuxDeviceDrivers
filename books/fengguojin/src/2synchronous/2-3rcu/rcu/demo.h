
#ifndef _DEMO_H_
#define _DEMO_H_

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/********************************************************
 * Macros to help debugging
 ********************************************************/
#undef PDEBUG             /* undef it, just in case */
#ifdef DEMO_DEBUG
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

//设备号
#define DEMO_MAJOR 224
#define DEMO_MINOR 0
#define COMMAND_LEDON 1
#define COMMAND_LEDOFF 2

//设备结构
struct DEMO_dev 
{
	struct cdev cdev;	  /* Char device structure		*/
};

//函数申明
ssize_t DEMO_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos);
ssize_t DEMO_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos);
loff_t  DEMO_llseek(struct file *filp, loff_t off, int whence);
int     DEMO_ioctl(struct inode *inode, struct file *filp,
                    unsigned int cmd, unsigned long arg);


#endif /* _DEMO_H_ */
