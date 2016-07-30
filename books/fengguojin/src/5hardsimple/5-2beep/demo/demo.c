
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

#include <mach/hardware.h>
#include <mach/map.h>
#include <mach/gpio.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/signal.h>
#include <asm/uaccess.h>
#include <plat/regs-timer.h>
#include <plat/gpio-cfg.h>
#include <plat/gpio-bank-f.h>
#include "demo.h"

#define PWM_TIMER1_AUTO_RELOAD    (1<<11)
#define PWM_TIMER1_MANUAL_UPDATE  (1<<9)
#define PWM_TIMER1_START          (1<<8)

struct simple_dev *simple_devices;
static unsigned char simple_inc=0;
static int wdctimeout=0;

static void startBeep()
{
	unsigned long tcon,tcon1,tcon2,tcmp,tcnt,pwm_PCLK;

	s3c_gpio_cfgpin(S3C64XX_GPF(15),S3C64XX_GPF15_PWM_TOUT1);	

	tcon = __raw_readl(S3C2410_TCON);
	//tcon |= 0xA00;
	tcon |= PWM_TIMER1_MANUAL_UPDATE|PWM_TIMER1_AUTO_RELOAD;
	__raw_writel(tcon, S3C2410_TCON);

	tcon1 = __raw_readl(S3C2410_TCFG0);
	printk("S3C2410_TCFG0=%d\n",tcon1);
	tcon1=tcon1|0x0000000F;

	tcon2 = __raw_readl(S3C2410_TCFG1);
	printk("S3C2410_TCFG1=%d\n",tcon2);
	tcon2=tcon2|0x000000F0;

	pwm_PCLK=66500000/(tcon1*tcon2);
	printk("pwm_PCLK=%d\n",pwm_PCLK);

	tcnt=pwm_PCLK/2000;//2000Hz
	tcmp=tcnt/3;
	__raw_writel(tcnt, S3C2410_TCNTB(1));
	__raw_writel(tcmp, S3C2410_TCMPB(1));
	
	tcon = __raw_readl(S3C2410_TCON);
	//tcon |= 0x900;
	tcon |= PWM_TIMER1_START|PWM_TIMER1_AUTO_RELOAD;
	__raw_writel(tcon, S3C2410_TCON);
}

static void stopBeep()
{
	unsigned long tcon;
	tcon = __raw_readl(S3C2410_TCON);
	tcon &= ~(PWM_TIMER1_AUTO_RELOAD);
	tcon &= ~(PWM_TIMER1_START);
	__raw_writel(tcon, S3C2410_TCON);

	tcon = __raw_readl(S3C2410_TCON);
	printk("S3C2410_TCON=0x%x\n",tcon);

	s3c_gpio_cfgpin(S3C64XX_GPF(15),0);	
}

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
	switch (cmd)
	{
	case BEEP_S3C6410_ON:
		startBeep();
		break;
	case BEEP_S3C6410_OFF:
		stopBeep();
		break;
	default:
		break;
	}
	return 1;
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
	
	return 0;
	
fail:
	simple_cleanup_module();
	return result;
}

module_init(simple_init_module);
module_exit(simple_cleanup_module);
MODULE_AUTHOR("fgj");
MODULE_LICENSE("Dual BSD/GPL");