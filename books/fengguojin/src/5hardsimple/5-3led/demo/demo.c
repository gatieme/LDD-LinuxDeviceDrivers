
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

#define S3C64XX_GPM0_OUTPUT      (0x01 << 0)
#define S3C64XX_GPM1_OUTPUT      (0x01 << 4)
#define S3C64XX_GPM2_OUTPUT      (0x01 << 8)
#define S3C64XX_GPM3_OUTPUT      (0x01 << 12)

#define LED_SI_OUT1	 s3c_gpio_cfgpin(S3C64XX_GPM(0),S3C64XX_GPM0_OUTPUT)
#define LED_SI_OUT2	 s3c_gpio_cfgpin(S3C64XX_GPM(1),S3C64XX_GPM1_OUTPUT)
#define LED_SI_OUT3	 s3c_gpio_cfgpin(S3C64XX_GPM(2),S3C64XX_GPM2_OUTPUT)
#define LED_SI_OUT4	 s3c_gpio_cfgpin(S3C64XX_GPM(3),S3C64XX_GPM3_OUTPUT)

#define LED_SI_H(i)	 __raw_writel(__raw_readl(S3C64XX_GPMDAT)|(1<<i),S3C64XX_GPMDAT)
#define LED_SI_L(i)	 __raw_writel(__raw_readl(S3C64XX_GPMDAT)&(~(1<<i)),S3C64XX_GPMDAT)


struct simple_dev *simple_devices;
static unsigned char simple_inc=0;


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

int simple_ioctl(struct inode *inode, struct file *filp,unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int *p=(int*)argp;

	if(cmd==COMMAND_LEDON)
	{
		LED_SI_L(*p);
		printk("__raw_readl(S3C64XX_GPMDAT)=0x%x\n",__raw_readl(S3C64XX_GPMDAT));
		return 0;
	}
	if(cmd==COMMAND_LEDOFF)
	{
		LED_SI_H(*p);
		printk("__raw_readl(S3C64XX_GPMDAT)=0x%x\n",__raw_readl(S3C64XX_GPMDAT));
		return 0;
	}
	return -EFAULT;
}

struct file_operations simple_fops = {
	.owner =    THIS_MODULE,
	.open=  simple_open,
	.ioctl =    simple_ioctl,
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
	LED_SI_OUT1;
	LED_SI_OUT2;
	LED_SI_OUT3;
	LED_SI_OUT4;
	return 0;
	
fail:
	simple_cleanup_module();
	return result;
}

module_init(simple_init_module);
module_exit(simple_cleanup_module);
