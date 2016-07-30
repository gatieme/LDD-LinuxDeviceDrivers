
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
#include <linux/irq.h>

#include <plat/regs-irqtype.h>
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
#include <plat/gpio-bank-n.h>

#include "demo.h"

MODULE_AUTHOR("fgj");
MODULE_LICENSE("Dual BSD/GPL");
static int irqArray[6]=
{
	S3C_EINT(0),
		S3C_EINT(1),
		S3C_EINT(2),
		S3C_EINT(3),
		S3C_EINT(4),
		S3C_EINT(5)
};
struct button_dev *button_devices;
static unsigned char button_inc=0;
static int flag = 0;
struct timer_list polling_timer;
static unsigned long  polling_jffs=0;

#define eint_offset(irq)	((irq) - IRQ_EINT(0))
#define eint_irq_to_bit(irq)	(1 << eint_offset(irq))

static void s3c_irq_eint_unmask(unsigned int irq)
{
	u32 mask;
	
	mask = __raw_readl(S3C64XX_EINT0MASK);
	mask &= ~(eint_irq_to_bit(irq));
	__raw_writel(mask, S3C64XX_EINT0MASK);
}

static int s3c_irq_eint_set_type(unsigned int irq, unsigned int type)
{
	int offs = eint_offset(irq);
	int shift;
	u32 ctrl, mask;
	u32 newvalue = 0;
	void __iomem *reg;
	
	if (offs > 27)
		return -EINVAL;
	
	if (offs > 15)
		reg = S3C64XX_EINT0CON1; /* org: reg = S3C64XX_EINT0CON0; */
	else
		reg = S3C64XX_EINT0CON0; /* org: reg = S3C64XX_EINT0CON1; */
	
	
	switch (type) {
	case IRQ_TYPE_NONE:
		printk(KERN_WARNING "No edge setting!\n");
		break;
		
	case IRQ_TYPE_EDGE_RISING:
		newvalue = S3C2410_EXTINT_RISEEDGE;
		break;
		
	case IRQ_TYPE_EDGE_FALLING:
		newvalue = S3C2410_EXTINT_FALLEDGE;
		break;
		
	case IRQ_TYPE_EDGE_BOTH:
		newvalue = S3C2410_EXTINT_BOTHEDGE;
		break;
		
	case IRQ_TYPE_LEVEL_LOW:
		newvalue = S3C2410_EXTINT_LOWLEV;
		break;
		
	case IRQ_TYPE_LEVEL_HIGH:
		newvalue = S3C2410_EXTINT_HILEV;
		break;
		
	default:
		printk(KERN_ERR "No such irq type %d", type);
		return -1;
	}
	
	shift = ((offs % 16) / 2) * 4;	/* org: shift = (offs / 2) * 4; */
	mask = 0x7 << shift;
	
	ctrl = __raw_readl(reg);
	ctrl &= ~mask;
	ctrl |= newvalue << shift;
	__raw_writel(ctrl, reg);
	
	if (offs < 16)
		s3c_gpio_cfgpin(S3C64XX_GPN(offs), 0x2 << (offs * 2));
	else if (offs < 23)
		s3c_gpio_cfgpin(S3C64XX_GPL(offs - 8), S3C_GPIO_SFN(3));
	else
		s3c_gpio_cfgpin(S3C64XX_GPM(offs - 23), S3C_GPIO_SFN(3));
	
	return 0;
}

void initButton(void)
{
	s3c_gpio_cfgpin(S3C64XX_GPN(0),S3C64XX_GPN0_EINT0);
	s3c_gpio_cfgpin(S3C64XX_GPN(1),S3C64XX_GPN1_EINT1);
	s3c_gpio_cfgpin(S3C64XX_GPN(2),S3C64XX_GPN2_EINT2);
	s3c_gpio_cfgpin(S3C64XX_GPN(3),S3C64XX_GPN3_EINT3);
	s3c_gpio_cfgpin(S3C64XX_GPN(4),S3C64XX_GPN4_EINT4);
	s3c_gpio_cfgpin(S3C64XX_GPN(5),S3C64XX_GPN5_EINT5);
	
	s3c_gpio_setpull(S3C64XX_GPN(0), S3C_GPIO_PULL_NONE); 
	s3c_gpio_setpull(S3C64XX_GPN(1), S3C_GPIO_PULL_NONE); 
	s3c_gpio_setpull(S3C64XX_GPN(2), S3C_GPIO_PULL_NONE); 
	s3c_gpio_setpull(S3C64XX_GPN(3), S3C_GPIO_PULL_NONE); 
	s3c_gpio_setpull(S3C64XX_GPN(4), S3C_GPIO_PULL_NONE); 
	s3c_gpio_setpull(S3C64XX_GPN(5), S3C_GPIO_PULL_NONE); 
	
	s3c_irq_eint_set_type(0, IRQ_TYPE_EDGE_FALLING);
	s3c_irq_eint_set_type(1, IRQ_TYPE_EDGE_FALLING);
	s3c_irq_eint_set_type(2, IRQ_TYPE_EDGE_FALLING);
	s3c_irq_eint_set_type(3, IRQ_TYPE_EDGE_FALLING);
	s3c_irq_eint_set_type(4, IRQ_TYPE_EDGE_FALLING);
	s3c_irq_eint_set_type(5, IRQ_TYPE_EDGE_FALLING);
	
	s3c_irq_eint_unmask(0);
	s3c_irq_eint_unmask(1);
	s3c_irq_eint_unmask(2);
	s3c_irq_eint_unmask(3);
	s3c_irq_eint_unmask(4);
	s3c_irq_eint_unmask(5);
}

void polling_handler(unsigned long data)
{
	int code=-1;
	int i;
	code=__raw_readl(S3C64XX_GPNDAT);
	code=code&0x3F;
	
	for (i = 0; i <6; i++) 
	{
		enable_irq(irqArray[i]);
	}
	
	if(code>=0)
	{
		//避免中断连续出现
		if((jiffies-polling_jffs)>100)
		{
			polling_jffs=jiffies;
			//获取键盘值
			button_devices->key=(unsigned char)code;
			printk("get key %u\n",button_devices->key);
			flag = 1;
			wake_up_interruptible(&(button_devices->wq));
		}
	}
}

static irqreturn_t simplekey_interrupt(int irq, void *dummy, struct pt_regs *fp)
{
	int i;
	for (i = 0; i <6; i++) 
	{
		disable_irq(irqArray[i]);
	}
	polling_timer.expires = jiffies + HZ/5;
	add_timer(&polling_timer);
	return IRQ_HANDLED;
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
	struct button_dev *dev = filp->private_data; 
	int sum=0;
	if(flag==1)
	{
		flag = 0;
		sum=1;
		if (copy_to_user(buf,&dev->key,1))
		{
			sum=-EFAULT; 
		}
	}
	else
	{
		if (filp->f_flags & O_NONBLOCK)
		{
			return -EAGAIN;
		}
		else
		{
			if(wait_event_interruptible(dev->wq, flag != 0))
			{
				return - ERESTARTSYS;
			}	
			flag = 0;
			sum=1;
			if (copy_to_user(buf,&dev->key,1))
			{
				sum=-EFAULT; 
			}
		}
	}
	return sum;
}
unsigned int button_poll(struct file *filp, poll_table *wait)
{
	struct button_dev *dev = filp->private_data; 
	poll_wait(filp, &dev->wq, wait);
	if (flag==1)//数据准备好
		return  POLLIN | POLLRDNORM;
	return 0;
}

struct file_operations button_fops = {
	.owner =    THIS_MODULE,
		.read =     button_read,
		.open =     button_open,
		.poll=      button_poll,
		.release =  button_release,
};

void button_cleanup_module(void)
{
	dev_t devno = MKDEV(button_MAJOR, button_MINOR);
	int i;
	for (i = 0; i <6; i++) 
	{
		free_irq(irqArray[i],simplekey_interrupt);
	}
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
	
	init_MUTEX(&button_devices->sem);
	cdev_init(&button_devices->cdev, &button_fops);
	button_devices->cdev.owner = THIS_MODULE;
	button_devices->cdev.ops = &button_fops;
	result = cdev_add (&button_devices->cdev, dev, 1);
	if(result)
	{
		printk(KERN_NOTICE "Error %d adding DEMO\n", result);
		goto fail;
	}
	
	for (i = 0; i <6; i++)
	{
		if (request_irq(irqArray[i], &simplekey_interrupt, 0, "simplekey", NULL)) 
		{
			printk("request button irq failed!\n");
			return -1;
		}
	}
	init_waitqueue_head(&button_devices->wq);
	init_timer(&polling_timer);
	polling_timer.data = (unsigned long)0;
	polling_timer.function = polling_handler;
	return 0;
	
fail:
	button_cleanup_module();
	return result;
}

module_init(button_init_module);
module_exit(button_cleanup_module);
