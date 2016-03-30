/*
 * 2006 OURS TECH 
 *
 *                     <liduoli@sina.com>
 *
	 This file for explain how to use a simple driver 

 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/init.h>

#include <linux/module.h>

#include <asm/hardware.h>

// GPIO_LED DEVICE MAJOR
#define SIMPLE_GPIO_LED_MAJOR	97

#define OURS_GPIO_LED_DEBUG 

// define LED
#define OURS_GPIO_LED	(GPIO96)

//define LED STATUS
#define LED_ON  0
#define LED_OFF 1

// define cmd
#define ctl_GPIO_LED1 1

#define VERSION         "PXA2700EP-gpio-V1.00-060607"

void showversion(void)
{
        printk("*********************************************\n");
        printk("\t %s \t\n", VERSION);
        printk("*********************************************\n\n");

}


// ------------------- READ ------------------------
ssize_t SIMPLE_GPIO_LED_read (struct file * file ,char * buf, size_t count, loff_t * f_ops)
{
	#ifdef OURS_GPIO_LED_DEBUG
		printk ("SIMPLE_GPIO_LED_read [ --kernel--]\n");
	#endif              

	return count;
}	

// ------------------- WRITE -----------------------
ssize_t SIMPLE_GPIO_LED_write (struct file * file ,const char * buf, size_t count, loff_t * f_ops)
{
	#ifdef OURS_GPIO_LED_DEBUG
        	printk ("SIMPLE_GPIO_LED_write [ --kernel--]\n");
        #endif

	return count;
}	

// ------------------- IOCTL -----------------------
ssize_t SIMPLE_GPIO_LED_ioctl (struct inode * inode ,struct file * file, unsigned int cmd, long data)
{
	#ifdef OURS_GPIO_LED_DEBUG
        	printk ("SIMPLE_GPIO_LED_ioctl [ --kernel--]\n");
        #endif
	
	switch (cmd)
        {
		case LED_ON : {GPCR3 |= 0x1;break;}
		case LED_OFF: {GPSR3 |= 0x1;break;}
                default :
                        {printk ("lcd control : no cmd run  [ --kernel-- ]\n"); return (-EINVAL);}
        }

	return 0;
	
}

// ------------------- OPEN ------------------------
ssize_t SIMPLE_GPIO_LED_open (struct inode * inode ,struct file * file)
{
	#ifdef OURS_GPIO_LED_DEBUG
        	printk ("SIMPLE_GPIO_LED_open [ --kernel--]\n");
        #endif
	
	MOD_INC_USE_COUNT;
	
	return 0;
}	

// ------------------- RELEASE/CLOSE ---------------
ssize_t SIMPLE_GPIO_LED_release (struct inode  * inode ,struct file * file)
{
	#ifdef OURS_GPIO_LED_DEBUG
	        printk ("SIMPLE_GPIO_LED_release [ --kernel--]\n");
        #endif

	MOD_DEC_USE_COUNT;

	return 0;
}

// -------------------------------------------------
struct file_operations GPIO_LED_ctl_ops ={

	
	open:		SIMPLE_GPIO_LED_open,
	
	read:		SIMPLE_GPIO_LED_read,
	
	write:		SIMPLE_GPIO_LED_write,
	
	ioctl:		SIMPLE_GPIO_LED_ioctl,
	
	release:	SIMPLE_GPIO_LED_release,
	
};



// ------------------- INIT ------------------------
static int __init HW_GPIO_LED_CTL_init(void)
{
    int ret = -ENODEV;
	

	printk("hhhhhhhhhhhhhhhhhhhhhhhhhhhhh\n\n");
	showversion();
	// init GPIO
	
	GPDR3 |= 0x00000001; // SET GPIO96 OUTPUT MODE
	GPSR3 |= 0x00000001; // OFF THE LED

	#ifdef OURS_GPIO_LED_DEBUG
                printk (" GPLR3=%x \n",GPLR3);
		printk (" GPDR3=%x \n",GPDR3);
        #endif


	ret = devfs_register_chrdev(SIMPLE_GPIO_LED_MAJOR, "gpio_led_ctl", &GPIO_LED_ctl_ops);

	if( ret < 0 )
	{
		printk (" pxa270: init_module failed with %d\n [ --kernel--]", ret);	
		return ret;
	}
	else
	{
		printk(" pxa270 gpio_led_driver register success!!! [ --kernel--]\n");
	}


	return ret;
}


static int __init pxa270_GPIO_LED_CTL_init(void) 
{
    int  ret = -ENODEV;

	#ifdef OURS_GPIO_LED_DEBUG
                printk ("pxa270_GPIO_LED_CTL_init [ --kernel--]\n");
        #endif


    ret = HW_GPIO_LED_CTL_init();
    if (ret)
      return ret;
    return 0;
}

static void __exit cleanup_GPIO_LED_ctl(void)
{
	#ifdef OURS_GPIO_LED_DEBUG
        	printk ("cleanup_GPIO_LED_ctl [ --kernel--]\n");
        #endif
	
	devfs_unregister_chrdev (SIMPLE_GPIO_LED_MAJOR, "gpio_led_ctl" );
	
}


MODULE_DESCRIPTION("simple gpio_led driver module");
MODULE_AUTHOR("liduo");
MODULE_LICENSE("GPL");

module_init(pxa270_GPIO_LED_CTL_init);
module_exit(cleanup_GPIO_LED_ctl);

