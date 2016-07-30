
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>	
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/pagemap.h>
#include <linux/times.h>

#include "demo.h"

MODULE_AUTHOR("fgj");
MODULE_LICENSE("Dual BSD/GPL");

#define SIMPLE_TIMER_DELAY 2*HZ//2Second

struct simple_dev *simple_devices;
static unsigned char simple_inc=0;
struct timeval start,stop,diff; 
static struct timer_list simple_timer;
static void simple_timer_handler(unsigned long data);

int timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y) 
{ 
    if(x->tv_sec>y->tv_sec) 
      return -1; 
  
    if((x->tv_sec==y->tv_sec)&&(x->tv_usec>y->tv_usec)) 
      return -1; 
  
    result->tv_sec = ( y->tv_sec-x->tv_sec ); 
    result->tv_usec = ( y->tv_usec-x->tv_usec ); 
  
    if(result->tv_usec<0) 
    { 
      result->tv_sec--; 
      result->tv_usec+=1000000; 
    } 
    return 0; 
} 

static void simple_timer_handler( unsigned long data)
{
    do_gettimeofday(&stop); 
    timeval_subtract(&diff,&start,&stop); 
    printk("总计用时:%d 秒 %d 微秒\n",diff.tv_sec,diff.tv_usec); 
	return ;
}

int simple_open(struct inode *inode, struct file *filp)
{
	struct simple_dev *dev; /* device information */

	simple_inc++;

	dev = container_of(inode->i_cdev, struct simple_dev, cdev);
	filp->private_data = dev; /* for other methods */

	return 0;    /* success */
}

int simple_release(struct inode *inode, struct file *filp)
{
	simple_inc--;
	return 0;
}

struct file_operations simple_fops = {
	.owner =  THIS_MODULE,
	.open =   simple_open,
	.release =  simple_release,
};

/*******************************************************
      MODULE ROUTINE
*******************************************************/
void simple_cleanup_module(void)
{
	dev_t devno = MKDEV(simple_MAJOR, simple_MINOR);

	/* Get rid of our char dev entries */
	if (simple_devices) 
	{
		cdev_del(&simple_devices->cdev);
		kfree(simple_devices);
	}

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno,1);
}

int simple_init_module(void)
{
	int result;
	dev_t dev = 0;

	dev = MKDEV(simple_MAJOR, simple_MINOR);
	result = register_chrdev_region(dev, 1, "DEMO");
	if (result < 0) 
	{
		printk(KERN_WARNING "DEMO: can't get major %d\n", simple_MAJOR);
		return result;
	}

	//allocate the devices 
	simple_devices = kmalloc(sizeof(struct simple_dev), GFP_KERNEL);
	if (!simple_devices)
	{
		result = -ENOMEM;
		goto fail;  /* Make this more graceful */
	}
	memset(simple_devices, 0, sizeof(struct simple_dev));

	init_MUTEX(&simple_devices->sem);
	cdev_init(&simple_devices->cdev, &simple_fops);
	simple_devices->cdev.owner = THIS_MODULE;
	simple_devices->cdev.ops = &simple_fops;
	result = cdev_add (&simple_devices->cdev, dev, 1);
	if(result)
	{
		printk(KERN_NOTICE "Error %d adding DEMO\n", result);
		goto fail;
	}

	/* Register timer */
	init_timer(&simple_timer);
	simple_timer.function = &simple_timer_handler;
	simple_timer.expires = jiffies + SIMPLE_TIMER_DELAY;
	add_timer (&simple_timer);
	do_gettimeofday(&start); 
	return 0; /* succeed */
fail:
	simple_cleanup_module();
	return result;
}

module_init(simple_init_module);
module_exit(simple_cleanup_module);
