/*********************************
模块:键盘驱动
作者:fgjnew@163.com
**********************************/
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
#include <asm/dma.h>

MODULE_AUTHOR("FGJ");
MODULE_DESCRIPTION("buttons driver");
MODULE_LICENSE("GPL");

#define DEBUG_DRIVER 1


static struct input_dev*simplekey_dev;
static unsigned long  polling_jffs=0;
static char *simplekey_name = "simplekey";
static char *simplekey_phys = "input0";
struct timer_list polling_timer;
static unsigned char simplekey_keycode[0x06] = {
	[0]	 = KEY_1,
	[1]	 = KEY_2,
	[2]	 = KEY_3,
	[3]	 = KEY_4,
	[4]	 = KEY_5,
	[5]	 = KEY_6,
};

static unsigned char offset[]={0x3E,0x3D,0x3B,0x37,0x2F,0x1F};

static int irqArray[6]=
{
	S3C_EINT(0),
		S3C_EINT(1),
		S3C_EINT(2),
		S3C_EINT(3),
		S3C_EINT(4),
		S3C_EINT(5)
};

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
			for(i=0;i<6;i++)
			{
				if(offset[i]==code)
				{
					code=i;
				}
			}

			input_report_key(simplekey_dev, simplekey_keycode[code], 1);
			input_report_key(simplekey_dev, simplekey_keycode[code], 0);
			input_sync(simplekey_dev);
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

static int __init simplekey_init(void)
{
	int i=0;
	initButton();
	simplekey_dev=input_allocate_device();

	for (i = 0; i <6; i++)
	{
		if (request_irq(irqArray[i], &simplekey_interrupt, 0, "simplekey", NULL)) 
		{
			printk("request button irq failed!\n");
			return -1;
		}
	}
	printk("initialize button ...\n");
	simplekey_dev->evbit[0] = BIT(EV_KEY);
	simplekey_dev->keycode = simplekey_keycode;
	simplekey_dev->keycodesize = sizeof(unsigned char);
	simplekey_dev->keycodemax = ARRAY_SIZE(simplekey_keycode);
	for (i = 0; i < 0x78; i++)
		if (simplekey_keycode[i])
			set_bit(simplekey_keycode[i], simplekey_dev->keybit);
	simplekey_dev->name = simplekey_name;
	simplekey_dev->phys = simplekey_phys;
	simplekey_dev->id.bustype = BUS_AMIGA;
	simplekey_dev->id.vendor = 0x0001;
	simplekey_dev->id.product = 0x0001;
	simplekey_dev->id.version = 0x0100;
	input_register_device(simplekey_dev);
#if DEBUG_DRIVER
	printk("initialize button ok!\n");
#endif

	init_timer(&polling_timer);
	polling_timer.data = (unsigned long)0;
	polling_timer.function = polling_handler;
	return 0;
}

static void __exit simplekey_exit(void)
{
	int i;
	for (i = 0; i <6; i++) 
	{
		free_irq(irqArray[i],simplekey_interrupt);
	}
	input_unregister_device(simplekey_dev);
}

module_init(simplekey_init);
module_exit(simplekey_exit);
