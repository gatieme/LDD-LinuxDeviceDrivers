
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
#include <linux/mm.h>
#include <asm/io.h>
#include <linux/slab.h>

static int mem_start = 101, mem_size = 10;
static char *reserve_virt_addr;
static int major;

int mmapdrv_open(struct inode *inode, struct file *file)
{
	return (0);
}

int mmapdrv_release(struct inode *inode, struct file *file)
{
	return (0);
}

static struct file_operations mmapdrv_fops =
{
	owner: THIS_MODULE, 
	open: mmapdrv_open, 
	release:mmapdrv_release,
};

int init_moduleiomap(void)
{
	if ((major = register_chrdev(0, "mmapdrv", &mmapdrv_fops)) < 0)
	{
		printk("mmapdrv: unable to register character device\n");
		return ( - EIO);
	}
	printk("mmap device major = %d\n", major);
	printk("high memory physical address 0x%ldM\n", virt_to_phys(high_memory)/1024/1024);
	
	reserve_virt_addr = ioremap(mem_start *1024 * 1024, mem_size *1024 * 1024);
	printk("reserve_virt_addr = 0x%lx\n", (unsigned long)reserve_virt_addr);
	if (reserve_virt_addr)
	{
		int i;
		for (i = 0; i < mem_size *1024 * 1024; i += 4)
		{
			reserve_virt_addr[i] = 'a';
			reserve_virt_addr[i + 1] = 'b';
			reserve_virt_addr[i + 2] = 'c';
			reserve_virt_addr[i + 3] = 'd';
		}
	}
	else
	{
		unregister_chrdev(major, "mmapdrv");
		return - ENODEV;
	}
	return 0;
}

/* remove the module */
void cleanup_moduleiomap(void)
{
	if (reserve_virt_addr)
		iounmap(reserve_virt_addr);
	unregister_chrdev(major, "mmapdrv");
	return ;
}

module_init(init_moduleiomap);
module_exit(cleanup_moduleiomap);

MODULE_LICENSE("Dual BSD/GPL");