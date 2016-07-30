
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/ctype.h>
#include <linux/pagemap.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/signal.h>
#include <asm/uaccess.h>


#include "demo.h"

MODULE_AUTHOR("fgj");
MODULE_LICENSE("Dual BSD/GPL");


struct simple_dev *simple_devices;
static unsigned char simple_inc=0;
static struct timer_list simple_timer;
static struct fasync_struct *fasync_queue=NULL;

static void simple_timer_handler( unsigned long data)
{
    printk("simple_timer_handler...\n");
    if (fasync_queue)
    {
	  //POLL_IN¿É¶Á£¬POLL_OUTÎª¿ÉÐ´
      kill_fasync(&fasync_queue, SIGIO, POLL_IN);
      printk("kill_fasync...\n");
    }
    return ;
}

int simple_open(struct inode *inode, struct file *filp)
{
	struct simple_dev *dev;
	dev = container_of(inode->i_cdev, struct simple_dev, cdev);
	filp->private_data = dev;

    simple_timer.function = &simple_timer_handler;
	simple_timer.expires = jiffies + 2*HZ;
	add_timer (&simple_timer);
    printk("add_timer...\n");
	return 0;
}


static int simple_fasync(int fd, struct file * filp, int mode) 
{
    int retval;
    printk("simple_fasync...\n");
    retval=fasync_helper(fd,filp,mode,&fasync_queue);
    if(retval<0)
      return retval;
    return 0;
}

int simple_release(struct inode *inode, struct file *filp)
{
	simple_fasync(-1, filp, 0);
	return 0;
}

struct file_operations simple_fops = {
	.owner =    THIS_MODULE,
	.open =     simple_open,
	.release=   simple_release,
	.fasync=    simple_fasync,
	
};

/*******************************************************
                MODULE ROUTINE
*******************************************************/
void simple_cleanup_module(void)
{
	dev_t devno = MKDEV(simple_MAJOR, simple_MINOR);

	if (simple_devices) 
	{
		cdev_del(&simple_devices->cdev);
		kfree(simple_devices);
	}

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

	simple_devices = kmalloc(sizeof(struct simple_dev), GFP_KERNEL);
	if (!simple_devices)
	{
		result = -ENOMEM;
		goto fail;
	}
	memset(simple_devices, 0, sizeof(struct simple_dev));

	cdev_init(&simple_devices->cdev, &simple_fops);
	simple_devices->cdev.owner = THIS_MODULE;
	simple_devices->cdev.ops = &simple_fops;
	result = cdev_add (&simple_devices->cdev, dev, 1);
	if(result)
	{
		printk(KERN_NOTICE "Error %d adding DEMO\n", result);
		goto fail;
	}
	init_timer(&simple_timer);
	return 0;

fail:
	simple_cleanup_module();
	return result;
}

module_init(simple_init_module);
module_exit(simple_cleanup_module);
