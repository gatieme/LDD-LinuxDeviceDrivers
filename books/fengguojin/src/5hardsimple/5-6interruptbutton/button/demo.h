
#ifndef _button_H_
#define _button_H_

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/********************************************************
 * Macros to help debugging
 ********************************************************/
#undef PDEBUG             /* undef it, just in case */
#ifdef button_DEBUG
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
#define button_MAJOR 224
#define button_MINOR 0
#define COMMAND1 1
#define COMMAND2 2

//设备结构
struct button_dev 
{
	struct semaphore sem; 
	wait_queue_head_t wq;
	struct cdev cdev;
	unsigned char key;
};

//函数申明
ssize_t button_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos);
unsigned int button_poll(struct file *filp, poll_table *wait);

#endif /* _button_H_ */
