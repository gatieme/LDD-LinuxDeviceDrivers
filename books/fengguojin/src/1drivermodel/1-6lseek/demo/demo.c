
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

#include "demo.h"

MODULE_AUTHOR("fgj");
MODULE_LICENSE("Dual BSD/GPL");

struct simple_dev *simple_devices;
static unsigned char simple_inc=0;
static unsigned char demoBuffer[256];

int simple_open(struct inode *inode, struct file *filp)
{
	struct simple_dev *dev;

	if(simple_inc>0)return -ERESTARTSYS;
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
	loff_t pos= *f_pos;
	if(pos>=256)
	{
		count=0;
		goto out;
	}
	if(count>(256-pos))
	{
		count=256-pos;
	}
	pos += count;
	
	if (copy_to_user(buf,demoBuffer+*f_pos,count))
	{
	   count=-EFAULT; 
	   goto out;
	}
	*f_pos = pos;
 out:
	return count;
}

loff_t simple_llseek(struct file *filp, loff_t off, int whence)
{
	loff_t pos;
	pos = filp->f_pos;
	switch (whence) 
	{
	case 0:
		pos = off;
		break;
	case 1:
		pos += off;
		break;
	case 2:
		pos =255+off;
		break;
	default:
		return -EINVAL;
	}
	
	if ((pos>=256) || (pos<0)) 
	{
		return -EINVAL;
	}
	
	return filp->f_pos=pos;
}

struct file_operations simple_fops = {
	.owner =    THIS_MODULE,
	.llseek =   simple_llseek,
	.read =     simple_read,
	.open =     simple_open,
	.release =  simple_release,
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
    memcpy(demoBuffer,"ABCDEFGHIJKLMN",14);
	return 0;

fail:
	simple_cleanup_module();
	return result;
}

module_init(simple_init_module);
module_exit(simple_cleanup_module);
