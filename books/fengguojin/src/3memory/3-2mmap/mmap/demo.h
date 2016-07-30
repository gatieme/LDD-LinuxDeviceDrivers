
#ifndef _simple_H_
#define _simple_H_

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

//设备号
#define simple_MAJOR 224
#define simple_MINOR 0
#define COMMAND1 1
#define COMMAND2 2

//设备结构
struct simple_dev 
{
	struct semaphore sem;     /* mutual exclusion semaphore     */
	struct cdev cdev;	  /* Char device structure		*/
};

//函数申明
ssize_t simple_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos);
ssize_t simple_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos);
loff_t  simple_llseek(struct file *filp, loff_t off, int whence);
int     simple_ioctl(struct inode *inode, struct file *filp,
                    unsigned int cmd, unsigned long arg);


#endif /* _simple_H_ */
