
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

#define S3C64XX_PA_WDC	   	(0x7E004000)
#define S3C64XX_WDCON	   	0
#define S3C64XX_WDDAT	   	4
#define S3C64XX_WDCNT	   	8
#define S3C64XX_WDLRINT	   	12

#define WDC_RESET_ENABLE      (1<<0)
#define WDC_INTERRUPT_ENABLE  (1<<2)
#define WDC_TIMER_ENABLE      (1<<5)

static void __iomem *s3c_wdc_base;

struct simple_dev *simple_devices;
static unsigned char simple_inc=0;
static int wdctimeout=0;

int simple_open(struct inode *inode, struct file *filp)
{
	struct simple_dev *dev;
	printk("simple_open\n");
	if(simple_inc>0)return -ERESTARTSYS;
	simple_inc++;
	
	dev = container_of(inode->i_cdev, struct simple_dev, cdev);
	filp->private_data = dev;
	
	int tmp=((0x67<<8)|(0x1<<4))|WDC_RESET_ENABLE|WDC_TIMER_ENABLE;
	//PCLK=66500000
	//t_watchdog = 1/( PCLK / (Prescaler value + 1) / Division_factor )=(Division_factor*(Prescaler value + 1))/PCLK
	//t_watchdog =(104*64)/66500000~=0.0001s
	wdctimeout=0xFFFF;
	writel(wdctimeout, s3c_wdc_base + S3C64XX_WDCNT);
	writel(tmp, s3c_wdc_base + S3C64XX_WDCON);
	printk("S3C64XX_WDCON 0x%x\n",readl(s3c_wdc_base + S3C64XX_WDCON));
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
	int __user *p = argp;
	if(cmd==WDIOC_KEEPALIVE)
	{
		printk("S3C64XX_WDDAT 0x%x\n",readl(s3c_wdc_base + S3C64XX_WDDAT));
		printk("S3C64XX_WDCON 0x%x\n",readl(s3c_wdc_base + S3C64XX_WDCON));
		writel(wdctimeout, s3c_wdc_base + S3C64XX_WDCNT);
		printk("S3C64XX_WDCNT 0x%x\n",readl(s3c_wdc_base + S3C64XX_WDCNT));
	}
	if(cmd==WDIOC_SETTIMEOUT)
	{
		//t_watchdog=0.0001s
		wdctimeout=(*p)*10000;
		printk("wdctimeout%d\n",wdctimeout);
		writel(wdctimeout, s3c_wdc_base + S3C64XX_WDDAT);
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
		printk(KERN_WARNING "simple: can't get major %d\n", simple_MAJOR);
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
	
	s3c_wdc_base = ioremap(S3C64XX_PA_WDC,0xff);
	if (s3c_wdc_base == NULL)
	{
		printk(KERN_NOTICE "ioremap Error\n");
		result = -EINVAL;
		goto fail;
	}
	printk("simple_init_module\n");
	return 0;
	
fail:
	simple_cleanup_module();
	return result;
}

module_init(simple_init_module);
module_exit(simple_cleanup_module);
MODULE_AUTHOR("fgj");
MODULE_LICENSE("Dual BSD/GPL");