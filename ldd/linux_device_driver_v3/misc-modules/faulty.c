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
 * $Id: faulty.c,v 1.3 2004/09/26 07:02:43 gregkh Exp $
 */


#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/types.h>  /* size_t */
#include <asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");


int faulty_major = 0;

ssize_t faulty_read(struct file *filp, char __user *buf,
		    size_t count, loff_t *pos)
{
	int ret;
	char stack_buf[4];

	/* Let's try a buffer overflow  */
	memset(stack_buf, 0xff, 20);
	if (count > 4)
		count = 4; /* copy 4 bytes to the user */
	ret = copy_to_user(buf, stack_buf, count);
	if (!ret)
		return count;
	return ret;
}

ssize_t faulty_write (struct file *filp, const char __user *buf, size_t count,
		loff_t *pos)
{
	/* make a simple fault by dereferencing a NULL pointer */
	*(int *)0 = 0;
	return 0;
}



struct file_operations faulty_fops = {
	.read =  faulty_read,
	.write = faulty_write,
	.owner = THIS_MODULE
};


int faulty_init(void)
{
	int result;

	/*
	 * Register your major, and accept a dynamic number
	 */
	result = register_chrdev(faulty_major, "faulty", &faulty_fops);
	if (result < 0)
		return result;
	if (faulty_major == 0)
		faulty_major = result; /* dynamic */

	return 0;
}

void faulty_cleanup(void)
{
	unregister_chrdev(faulty_major, "faulty");
}

module_init(faulty_init);
module_exit(faulty_cleanup);

