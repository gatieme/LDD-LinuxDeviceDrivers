/*
 * faulty.c -- a module which generates an oops when read
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 */

#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

int faulty_major = 0;
int faulty_minor = 0;

ssize_t faulty_read(struct file *filp, char __user *buf,
		    size_t count, loff_t *pos)
{
	int ret;
	char stack_buf[4];

	/* Let's try a buffer overflow. */
	memset(stack_buf, 0xff, 20);
	if (count > 4)
		count = 4; /* Copy 4 bytes to the user. */
	ret = copy_to_user(buf, stack_buf, count);
	if (!ret)
		return count;
	return ret;
}

ssize_t faulty_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *pos)
{
	/* Make a simple fault by dereferencing a NULL pointer. */
	*(int *)0 = 0;
	return 0;
}


struct file_operations faulty_fops = {
	.read =  faulty_read,
	.write = faulty_write,
	.owner = THIS_MODULE
};


static int __init faulty_init(void)
{
	int result;
	dev_t dev;
	struct cdev *faulty_cdev;

	/* Register dynamically. */
	result = alloc_chrdev_region(&dev, faulty_minor, 1, "faulty");

	if (result < 0) {
		printk(KERN_WARNING "faulty: couldn't get major number assignment\n");
		return result;
	}

	faulty_major = MAJOR(dev);

	faulty_cdev = cdev_alloc();
	faulty_cdev->ops = &faulty_fops;
	cdev_add(faulty_cdev, dev, 1);

	return 0;
}

static void __exit faulty_cleanup(void)
{
	dev_t dev = MKDEV(faulty_major, faulty_minor);
	unregister_chrdev_region(dev, 1);
}

module_init(faulty_init);
module_exit(faulty_cleanup);
