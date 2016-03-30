/*
 * complete.c -- the writers awake the readers
 *
 * Copyright (C) 2003 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2003 O'Reilly & Associates
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

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/completion.h>

MODULE_LICENSE("Dual BSD/GPL");

static int complete_major = 0;
static int complete_minor = 0;

DECLARE_COMPLETION(comp);

ssize_t complete_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	printk(KERN_DEBUG "process %i (%s) going to sleep\n",
			current->pid, current->comm);
	wait_for_completion(&comp);
	printk(KERN_DEBUG "awoken %i (%s)\n", current->pid, current->comm);
	return 0; /* EOF */
}

ssize_t complete_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *pos)
{
	printk(KERN_DEBUG "process %i (%s) awakening the readers...\n",
			current->pid, current->comm);
	complete(&comp);
	return count;
}


struct file_operations complete_fops = {
	.owner = THIS_MODULE,
	.read =  complete_read,
	.write = complete_write,
};


static int __init complete_init(void)
{
	int result;
	dev_t dev;
	struct cdev *complete_cdev;

	/* Register dynamically. */
	result = alloc_chrdev_region(&dev, complete_minor, 1, "complete");

	if (result < 0) {
		printk(KERN_WARNING "complete: couldn't get major number assignment\n");
		return result;
	}

	complete_major = MAJOR(dev);

	complete_cdev = cdev_alloc();
	complete_cdev->ops = &complete_fops;
	cdev_add(complete_cdev, dev, 1);

	return 0;
}

static void __exit complete_cleanup(void)
{
	dev_t dev = MKDEV(complete_major, complete_minor);
	unregister_chrdev_region(dev, 1);
}

module_init(complete_init);
module_exit(complete_cleanup);

