/*
 * File Name: button.c
 *
 * Descriptions: 
 *		The example of button-interrupt module driver code.
 *
 * Author: 
 *		Zheng Qiang
 * Kernel Version: 2.6.29
 *
 * Update:
 * 		-	2009.12.16	Zheng Qiang	 Creat this file
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm-arm/arch-s3c2410/regs-gpio.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <asm-arm/arch-s3c2410/irqs.h>
#include <asm-arm/irq.h>
#include <linux/interrupt.h>

#define     BUTTON_IRQ1 IRQ_EINT0
#define     DEVICE_NAME "button-interrupt"

static irqreturn_t isr_button(int irq,void *dev_id,struct pt_regs *regs)
{
     unsigned long GPGDAT;

     GPGDAT=(unsigned long)ioremap(0x56000064,4);
	 if(irq==BUTTON_IRQ1)
	 {
	  	if((*(volatile unsigned long *)GPGDAT) & 1==0)
	  	{
	  		printk("K1 is pressed\n");
	  	}
	 }

    return 0;
}

static int __init s3c2440_buttons_init(void)
{
        int ret;
        set_irq_type(BUTTON_IRQ1,IRQT_FALLING);
		ret=request_irq(BUTTON_IRQ1,isr_button,SA_INTERRUPT,DEVICE_NAME,NULL);

		if(ret)
        {
                printk("K1_IRQ: could not register interrupt\n");
                return ret;
        }
       
		printk(DEVICE_NAME "initialized\n");
        return 0;
}
 
static void __exit s3c2440_buttons_exit(void)
{
	   free_irq(BUTTON_IRQ1,NULL);
	   printk(DEVICE_NAME "exit\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gery");
MODULE_DESCRIPTION ("button interrupt");

module_init(s3c2440_buttons_init);
module_exit(s3c2440_buttons_exit(void));