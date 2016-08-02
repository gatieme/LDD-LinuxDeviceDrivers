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
#include <linux/poll.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
#include <linux/slab.h>
#endif

#include "simple.h"



MODULE_AUTHOR("fgj & gatieme");
MODULE_LICENSE("Dual BSD/GPL");

struct simple_dev       *simple_devices;
static unsigned char    simple_inc=0;
static unsigned char    simple_flag=0;
static unsigned char    demoBuffer[256];
wait_queue_head_t       read_queue;




int simple_open(struct inode *inode, struct file *filp)
{
	struct simple_dev *dev;

	if(simple_inc >0 )
        return -ERESTARTSYS;
	simple_inc++;

	dev = container_of(inode->i_cdev, struct simple_dev, cdev);
	filp->private_data = dev;

	return 0;
}

int simple_release(struct inode *inode, struct file *filp)
{
	simple_inc--;
	return 0;
}


ssize_t simple_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos)
{
	//printk("wait_event_interruptible before\n");
	wait_event_interruptible(read_queue, simple_flag);
	//printk("wait_event_interruptible after\n");
	if (copy_to_user( buf, demoBuffer, count))
	{
		count=-EFAULT;
	}

    return count;
}

ssize_t simple_write(struct file *filp, const char __user *buf, size_t count,loff_t *f_pos)
{
	if (copy_from_user(demoBuffer, buf, count))
	{
		count = -EFAULT;
		goto out;
	}
	simple_flag=1;
    wake_up(&read_queue);
out:
	return count;
}

unsigned int simple_poll(struct file * file, poll_table * pt)
{
	unsigned int mask = POLLIN | POLLRDNORM;

    printk("poll_wait before\n");
	poll_wait(file, &read_queue, pt);
	printk("poll_wait after\n");

    return mask;
}

struct file_operations simple_fops = {
	.owner =    THIS_MODULE,
	.poll =     simple_poll,
	.read =     simple_read,
	.write=	    simple_write,
	.open =     simple_open,
	.release =  simple_release,
};




/*******************************************************
MODULE ROUTINE
*******************************************************/
static void __exit simple_cleanup_module(void)
{
	dev_t devno = MKDEV(simple_MAJOR, simple_MINOR);

	if (simple_devices)
	{
		cdev_del(&simple_devices->cdev);
		kfree(simple_devices);
	}

	unregister_chrdev_region(devno,1);
}


static int __init simple_setup_module(void)
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
    init_waitqueue_head(&read_queue);

	return 0;

fail:
	simple_cleanup_module();
	return result;
}

module_init(simple_setup_module);
module_exit(simple_cleanup_module);
