
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

#include "demo.h"

MODULE_AUTHOR("fgj");
MODULE_LICENSE("Dual BSD/GPL");

static unsigned char simple_inc=0;
static unsigned char demoBuffer[256];

int simple_open(struct inode *inode, struct file *filp)
{
	if(simple_inc>0)return -ERESTARTSYS;
	simple_inc++;
	return 0;
}

int simple_release(struct inode *inode, struct file *filp)
{
	simple_inc--;
	return 0;
}

ssize_t simple_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos)
{
	/* 把数据拷贝到应用程序空间 */
	if (copy_to_user(buf,demoBuffer,count))
	{
	   count=-EFAULT; 
	}
	return count;
}

ssize_t simple_write(struct file *filp, const char __user *buf, size_t count,loff_t *f_pos)
{
	/* 把数据拷贝到内核空间 */
	if (copy_from_user(demoBuffer+*f_pos, buf, count))
	{
		count = -EFAULT;
	}
	return count;
}

struct file_operations simple_fops = {
	.owner =    THIS_MODULE,
	.read =     simple_read,
	.write =    simple_write,
	.open =     simple_open,
	.release =  simple_release,
};

/*******************************************************
                MODULE ROUTINE
*******************************************************/
void simple_cleanup_module(void)
{
    unregister_chrdev(simple_MAJOR,  "simple"); 
	printk("simple_cleanup_module!\n");
}

int simple_init_module(void)
{
	int ret;
	ret = register_chrdev(simple_MAJOR, "simple", &simple_fops);//注册设备号231，设备名 
	if (ret < 0)
	{
		printk("Unable to register character device %d!\n",simple_MAJOR);
		return ret;
	}
	return 0;
}

module_init(simple_init_module);
module_exit(simple_cleanup_module);
