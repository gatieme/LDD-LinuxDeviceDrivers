/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2007, 2010 fengGuojin(fgjnew@163.com)
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/signal.h>

#include <asm/uaccess.h>
#include <mach/map.h>
#include <mach/gpio.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/signal.h>
#include <asm/uaccess.h>
#include <plat/regs-timer.h>
#include <plat/gpio-cfg.h>
#include <plat/gpio-bank-f.h>

MODULE_AUTHOR("fgj");
MODULE_DESCRIPTION("s3c6410 BEEP driver");
MODULE_LICENSE("GPL");

#define PWM_TIMER1_AUTO_RELOAD    (1<<11)
#define PWM_TIMER1_MANUAL_UPDATE  (1<<9)
#define PWM_TIMER1_START          (1<<8)

static char s3c6410_BEEP_name[] = "s3c6410BEEP";
static char s3c6410_BEEP_phys[] = "s3c6410BEEP";
static struct input_dev* s3c6410_BEEP_dev;

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

static int s3c6410_BEEP_event(struct input_dev *dev, unsigned int type, unsigned int code, int value)
{
	printk("s3c6410_BEEP_event\n");
	if (type != EV_LED)return -1;

	switch(value)
	{
		case 0:
			startBeep();
			break;
		case 1:
			stopBeep();
			break;
	}
	return 0;
}

static int __init s3c6410_BEEP_init(void)
{
	s3c6410_BEEP_dev=input_allocate_device();
	s3c6410_BEEP_dev->evbit[0] = BIT(EV_LED);
	s3c6410_BEEP_dev->ledbit[0] = BIT(LED_NUML);
	s3c6410_BEEP_dev->event = s3c6410_BEEP_event;

	s3c6410_BEEP_dev->name = s3c6410_BEEP_name;
	s3c6410_BEEP_dev->phys = s3c6410_BEEP_phys;
	s3c6410_BEEP_dev->id.bustype = BUS_HOST;
	s3c6410_BEEP_dev->id.vendor = 0x001f;
	s3c6410_BEEP_dev->id.product = 0x0001;
	s3c6410_BEEP_dev->id.version = 0x0100;

	input_register_device(s3c6410_BEEP_dev);
	printk(KERN_INFO "input: %s\n", s3c6410_BEEP_name);
	return 0;
}

static void __exit s3c6410_BEEP_exit(void)
{
    input_unregister_device(s3c6410_BEEP_dev);
}

module_init(s3c6410_BEEP_init);
module_exit(s3c6410_BEEP_exit);
