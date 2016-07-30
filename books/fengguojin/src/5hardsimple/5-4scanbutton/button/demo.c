
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
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

#include <plat/regs-gpio.h>
#include <mach/gpio.h>
#include <mach/hardware.h>
#include <mach/map.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/signal.h>
#include <asm/uaccess.h>
#include <plat/regs-timer.h>
#include <plat/gpio-cfg.h>
#include <plat/gpio-bank-m.h>
#include "demo.h"

MODULE_AUTHOR("fgj");
MODULE_LICENSE("Dual BSD/GPL");

struct button_dev *button_devices;
static unsigned char button_inc=0;
static int flag = 0;

void initButton(void)
{
	s3c_gpio_cfgpin(S3C64XX_GPN(0),0);
	s3c_gpio_cfgpin(S3C64XX_GPN(1),0);
	s3c_gpio_cfgpin(S3C64XX_GPN(2),0);
	s3c_gpio_cfgpin(S3C64XX_GPN(3),0);
	s3c_gpio_cfgpin(S3C64XX_GPN(4),0);
	s3c_gpio_cfgpin(S3C64XX_GPN(5),0);
}


int button_open(struct inode *inode, struct file *filp)
{
	struct button_dev *dev;

	if(button_inc>0)return -ERESTARTSYS;
	button_inc++;

	dev = container_of(inode->i_cdev, struct button_dev, cdev);
	filp->private_data = dev;
	return 0;
}

int button_release(struct inode *inode, struct file *filp)
{
	button_inc--;
	return 0;
}

ssize_t button_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos)
{
	int sum=1;
	struct button_dev *dev = filp->private_data; 
	int value=__raw_readl(S3C64XX_GPNDAT);
	value=value&0x3F;
	
	if (copy_to_user(buf,&value,sizeof(int)))
	{
		sum=-EFAULT; 
	}
	return sum;
}

struct file_operations button_fops = {
	.owner =    THIS_MODULE,
	.read =     button_read,
	.open =     button_open,
	.release =  button_release,
};

void button_cleanup_module(void)
{
	dev_t devno = MKDEV(button_MAJOR, button_MINOR);
	if (button_devices) 
	{
		cdev_del(&button_devices->cdev);
		kfree(button_devices);
	}
	unregister_chrdev_region(devno,1);
}

int button_init_module(void)
{
	int result;
	dev_t dev = 0;
	int i=0;
	initButton();
	dev = MKDEV(button_MAJOR, button_MINOR);
	result = register_chrdev_region(dev, 1, "DEMO");
	if (result < 0) 
	{
		printk(KERN_WARNING "DEMO: can't get major %d\n", button_MAJOR);
		return result;
	}

	button_devices = kmalloc(sizeof(struct button_dev), GFP_KERNEL);
	if (!button_devices)
	{
		result = -ENOMEM;
		goto fail;
	}
	memset(button_devices, 0, sizeof(struct button_dev));

	cdev_init(&button_devices->cdev, &button_fops);
	button_devices->cdev.owner = THIS_MODULE;
	button_devices->cdev.ops = &button_fops;
	result = cdev_add (&button_devices->cdev, dev, 1);
	if(result)
	{
		printk(KERN_NOTICE "Error %d adding DEMO\n", result);
		goto fail;
	}

	return 0;

fail:
	button_cleanup_module();
	return result;
}

module_init(button_init_module);
module_exit(button_cleanup_module);
