/*
 * kdatasize.c -- print the size of common data items from kernel space
 * This runs with any Linux kernel (not any Unix, because of <linux/types.h>)
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
 */

#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/utsname.h>
#include <linux/errno.h>

static void data_cleanup(void)
{
	/* never called */
}

int data_init(void)
{
	/* print information and return an error */
	printk("arch   Size:  char  short  int  long   ptr long-long "
		" u8 u16 u32 u64\n");
	printk("%-12s  %3i   %3i   %3i   %3i   %3i   %3i      "
		"%3i %3i %3i %3i\n",
		init_uts_ns.name.machine,
		(int)sizeof(char), (int)sizeof(short), (int)sizeof(int),
		(int)sizeof(long),
		(int)sizeof(void *), (int)sizeof(long long), (int)sizeof(__u8),
		(int)sizeof(__u16), (int)sizeof(__u32), (int)sizeof(__u64));
	return -ENODEV;
}

module_init(data_init);
module_exit(data_cleanup);

MODULE_LICENSE("Dual BSD/GPL");
