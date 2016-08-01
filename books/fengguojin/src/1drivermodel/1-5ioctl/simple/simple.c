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
#include <linux/slab.h>

#include <linux/pagemap.h>

#include "simple.h"


struct simple_dev       *simple_devices = NULL;
static unsigned char    simple_inc = 0;
static char             simple_buffer[MAX_SIZE];



int simple_open(struct inode *inode, struct file *filp)
{
	struct simple_dev *dev;

	if(simple_inc > 0)
    {
        return -ERESTARTSYS;
    }

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




#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 36)

int simple_ioctl(struct inode *inode, struct file *filp,
                unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case COMMAND1   :
        {
            memset(simple_buffer, 0x31, MAX_SIZE);
			break;
        }

		case COMMAND2   :
        {
            memset(simple_buffer, 0x32, MAX_SIZE);
			break;
        }

        default         :
        {
			return -EFAULT;
			break;
        }
	}

    return 0;
}

#else

long simple_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    printk("command = %d, ", cmd);

    switch(cmd)
    {
		case COMMAND_1  :
        {
            printk("ioctl COMMAND_1 ");
            memset(simple_buffer, 0x31, MAX_SIZE);
			//strcpy(simple_buffer, "command 1!");
            break;
        }

		case COMMAND_2  :
        {
            printk("ioctl COMMAND_2 ");
            memset(simple_buffer, 0x32, MAX_SIZE);
			//strcpy(simple_buffer, "command 2!");
			break;
        }

        default         :
        {
			return -EFAULT;
			break;
        }
    }
    printk("success\n");

    return 1;
}



#endif




struct file_operations simple_fops = {
	.owner =    THIS_MODULE,

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 36)
    .ioctl = simple_ioctl,
#else
    .unlocked_ioctl = simple_ioctl,
    .compat_ioctl = simple_ioctl,
#endif

    .open =     simple_open,
	.release =  simple_release,
};

/*******************************************************
                MODULE ROUTINE
*******************************************************/
static void __exit simple_cleanup_module(void)
{
	dev_t devno = MKDEV(SIMPLE_MAJOR, SIMPLE_MINOR);

	if (simple_devices)
	{
		cdev_del(&simple_devices->cdev);
		kfree(simple_devices);
	}
    unregister_chrdev_region(devno,1);
}



static int __init simple_setup_module(void)
{
	int result;
	dev_t dev = 0;

	dev = MKDEV(SIMPLE_MAJOR, SIMPLE_MINOR);
	result = register_chrdev_region(dev, 1, "DEMO");
	if (result < 0)
	{
		printk(KERN_WARNING "DEMO: can't get major %d\n", SIMPLE_MAJOR);
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

fail    :
	simple_cleanup_module();

    return result;
}



module_init(simple_setup_module);
module_exit(simple_cleanup_module);
