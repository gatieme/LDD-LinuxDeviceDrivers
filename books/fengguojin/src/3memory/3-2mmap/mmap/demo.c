
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
#include "demo.h"

MODULE_DESCRIPTION("mmap demo driver");
MODULE_LICENSE("Dual BSD/GPL");

struct simple_dev *simple_devices;
static unsigned char simple_inc=0;
static char*buffer=NULL;
static char*buffer_area=NULL;

volatile void *virt_to_kseg(volatile void *address)
{
    pgd_t *pgd; pmd_t *pmd; pte_t *ptep, pte;
	unsigned long va, ret = 0UL;
	
	va=(unsigned long)address;
	
	/* get the page directory. Use the kernel memory map. */
	pgd = pgd_offset_k(va);

	/* check whether we found an entry */
	if (!pgd_none(*pgd))
        {
	      /* get the page middle directory */
	      pmd = pmd_offset(pgd, va);
	      /* check whether we found an entry */
	      if (!pmd_none(*pmd))
           {
				/* get a pointer to the page table entry */
				preempt_disable();
				ptep = pte_offset_map(pmd, va);
				pte = *ptep;
				/* check for a valid page */
				if (pte_present(pte))
				{
		        /* get the address the page is refering to */
		        ret = (unsigned long)page_address(pte_page(pte));
				/* add the offset within the page to the page address */
				ret |= (va & (PAGE_SIZE -1));
				}
				pte_unmap(ptep);
				preempt_enable();
	      }
	}
	return((volatile void *)ret);
}


int simple_open(struct inode *inode, struct file *filp)
{
	struct simple_dev *dev;
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

static int simple_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret;
	ret = remap_pfn_range(vma,
		   vma->vm_start,
		   virt_to_phys((void*)((unsigned long)buffer_area)) >> PAGE_SHIFT,
		   vma->vm_end-vma->vm_start,
		   PAGE_SHARED);
	if(ret != 0) {
		return -EAGAIN;
	}
	return 0;
}

struct file_operations simple_fops = {
	.owner =    THIS_MODULE,
	.open =     simple_open,
	.mmap =    simple_mmap,
	.release =  simple_release,
};

/*******************************************************
                MODULE ROUTINE
*******************************************************/
void simple_cleanup_module(void)
{
	dev_t devno = MKDEV(simple_MAJOR, simple_MINOR);
	unsigned long virt_addr;
	if (simple_devices) 
	{
		cdev_del(&simple_devices->cdev);
		kfree(simple_devices);
	}
	for(virt_addr=(unsigned long)buffer_area; virt_addr<(unsigned long)buffer_area+4096;
	virt_addr+=PAGE_SIZE)
	{
		 SetPageReserved(virt_to_page(virt_addr));
	}
	if (buffer)
			kfree(buffer);
	unregister_chrdev_region(devno,1);
}

int simple_init_module(void)
{
	int result;
	dev_t dev = 0;
	int i;
	unsigned long virt_addr;
	
	dev = MKDEV(simple_MAJOR, simple_MINOR);
	result = register_chrdev_region(dev, 1, "DEMO");
	if (result < 0) 
	{
		printk(KERN_WARNING "DEMO: can't get major %d\n", simple_MAJOR);
		goto out_free;
	}

	simple_devices = kmalloc(sizeof(struct simple_dev), GFP_KERNEL);
	if (!simple_devices)
	{
		result = -ENOMEM;
		goto out_free;
	}
	memset(simple_devices, 0, sizeof(struct simple_dev));

	init_MUTEX(&simple_devices->sem);
	cdev_init(&simple_devices->cdev, &simple_fops);
	simple_devices->cdev.owner = THIS_MODULE;
	simple_devices->cdev.ops = &simple_fops;
	result = cdev_add (&simple_devices->cdev, dev, 1);
	if(result)
	{
		printk(KERN_NOTICE "Error %d adding DEMO\n", result);
		goto out_free;
	}
	
	buffer = kmalloc(4096,GFP_KERNEL);            
    printk(" mmap buffer = %p\n",buffer);            
	buffer_area=(int *)(((unsigned long)buffer + PAGE_SIZE -1) & PAGE_MASK);

	for (virt_addr=(unsigned long)buffer_area; virt_addr<(unsigned long)buffer_area+4096;
		virt_addr+=PAGE_SIZE)
	{
		/* reserve all pages to make them remapable */
		SetPageReserved(virt_to_page(virt_addr));
	}
	memset(buffer,'C',100);
	return 0;

out_free:
	simple_cleanup_module();
	return result;
}


module_init(simple_init_module);
module_exit(simple_cleanup_module);
