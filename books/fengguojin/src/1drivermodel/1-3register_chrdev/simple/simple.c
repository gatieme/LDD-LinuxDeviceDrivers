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


#include "simple.h"



MODULE_AUTHOR("gatieme");
MODULE_LICENSE("Dual BSD/GPL");

static unsigned char simple_inc = 0;
static unsigned char demoBuffer[256];



/*  open打开文件操作  */
int simple_open(struct inode *inode, struct file *filp)
{
	if(simple_inc > 0)
    {
        return -ERESTARTSYS;
    }

    simple_inc++;

    return 0;
}

/*  close关闭设备文件的操作  */
int simple_release(struct inode *inode, struct file *filp)
{
	simple_inc--;

    return 0;
}



/*  read读设备文件操作  */
ssize_t simple_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	/*  把数据拷贝到用户空间  */
	if (copy_to_user(buf, demoBuffer, count))
	{
	   count = -EFAULT;
	}

	return count;
}



/*  write写设备文件操作  */
ssize_t simple_write(struct file *filp, const char __user *buf, size_t count,loff_t *f_pos)
{
	/* 把数据拷贝到内核空间 */
	if (copy_from_user(demoBuffer + *f_pos, buf, count))
	{
		count = -EFAULT;
	}

	return count;
}



struct file_operations simple_fops =
{
	.owner =    THIS_MODULE,
	.read =     simple_read,
	.write =    simple_write,
	.open =     simple_open,
	.release =  simple_release,
};



/*******************************************************
                MODULE ROUTINE
*******************************************************/
static int __init simple_setup_module(void)
{
	int ret = -1;

    /*  注册设备号, 设备名  */
	ret = register_chrdev(simple_MAJOR, "simple", &simple_fops);
	if (ret < 0)
	{
		printk("Unable to register character device %d!\n",simple_MAJOR);
		return ret;
	}
    printk("Register character device simple major(%d)\n",simple_MAJOR);

    return 0;
}


static void __exit simple_cleanup_module(void)
{
    unregister_chrdev(simple_MAJOR, "simple");
	printk("simple_cleanup_module!\n");
}


module_init(simple_setup_module);
module_exit(simple_cleanup_module);



/*  Driver Information  */
#define DRIVER_VERSION  "1.0.0"
#define DRIVER_AUTHOR   "Gatieme @ AderStep Inc..."
#define DRIVER_DESC     "Linux \"cdev_add\" module for LDD-LinuxDeviceDrivers devices"
#define DRIVER_LICENSE  "Dual BSD/GPL"

/*  Kernel Module Information   */
MODULE_VERSION(DRIVER_VERSION);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);
