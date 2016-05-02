#define _VERSION__

#include <linux/kernel.h>
#include <linux/module.h>

#if CONFIG_MODVERSIOINS==1

    #define MODVERSIONS
    #include <linux/modversions.h>

#endif

#include<linux/fs.h>
#include<linux/string.h>
#include<linux/errno.h>

#include<linux/mm.h>
#include<linux/vmalloc.h>
//#include<linux/wrapper.h>
#include<linux/slab.h>
#include<asm/io.h>
#include<linux/mman.h>

#define MAPLEN (4096*10)


/*
 * 将代码从linux2.4移植到linux2.6.*
 *
 * ==A==
 *  2.4内核中，模块自身通过 MOD_INC_USE_COUNT, MOD_DEC_USE_COUNT宏来管理自己被使用的计数。
 *  2.6内核提供了更健壮、灵活的模块计数管理接口 try_module_get(&module), module_put(&module)
 *  取代2.4中的模块使用计数管理宏；模块的使用计数不必由自身管理，
 *  而且在管理模块使用计数时考虑到 SMP与PREEMPT机制的影响。
 *
 * ==B==
 * em_map_reserve是2.4的函数  而到2.6被SetPageReserved取代
 * */


/* device open */
int mapdrv_open(struct inode *inode,struct file *file);

/* device close */
int mapdrv_release(struct inode *inode,struct file *file);

/*device mmap */
int mapdrv_mmap(struct file *file,struct vm_area_struct *vma);

/* vm area open */
void map_vopen(struct vm_area_struct *vma);

/* vm area close */
void map_vclose(struct vm_area_struct *vma);

/* vm area nopage */
struct page *map_nopage(struct vm_area_struct *vma,unsigned long address,int write_access);

static struct file_operations mapdrv_fops=
{
	owner  : THIS_MODULE,
	mmap   : mapdrv_mmap,
	open   : mapdrv_open,
	release:mapdrv_release,
};

static struct vm_operations_struct map_vm_ops=
{
	open  : map_vopen,
	close : map_vclose,
	nopage: map_nopage,
};

static int *vmalloc_area = NULL;

static int major; // major number of device

#define VMALLOC_VMADDR(x)       ((unsigned long)(x))

volatile void *vaddr_to_kaddr(volatile void *address)
{
    pgd_t           *pgd;
    pud_t           *pud;
    pmd_t           *pmd;
    pte_t           *ptep, pte;

    unsigned long   va;
    unsigned long   ret = 0UL;

    va = VMALLOC_VMADDR((unsigned long)address);

    /* get the page directory. Use the kernel memory map. */
    pgd = pgd_offset_k(va);
    /* check whether we found an entry */
    if (pgd_none(*pgd))
    {
        return NULL;
    }

    /* get the page upper directory */
    pud = pud_offset(pgd, va);
    /* check whether we found an entry */
    if (pud_none(*pud))
    {
        return NULL;
    }


    /* get the page middle directory */
    pmd = pmd_offset(pud, va);
    /* check whether we found an entry */
    if (pmd_none(*pmd))
    {
        return NULL;
    }

    /* get a pointer to the page table entry */
    /*ptep = pte_offset(pmd, va);  implicit declaration of function ‘pte_offset’`. */
    ptep = pte_offset_kernel(pmd, va);
    pte = *ptep;

   /* check for a valid page */
   if (pte_present(pte))
   {

       /* get the address the page is refering to */
       ret = (unsigned long)page_address(pte_page(pte));

       /* add the offset within the page to the page address */
       ret |= (va & (PAGE_SIZE -1));
    }

    return((volatile void *)ret);
}



static int  __init mapdrv_init(void)
{
    unsigned long virt_addr;
    if ((major = register_chrdev(0, "mapdrv", &mapdrv_fops)) < 0)
    {
        printk("mapdrv: unable to register character device\n");

        return (-EIO);
    }

    /* get a memory area that is only virtual contigous. */
    vmalloc_area = vmalloc(MAPLEN/* + 2 * PAGE_SIZE*/);


    for (virt_addr = (unsigned long)vmalloc_area;
         virt_addr < (unsigned long)(&(vmalloc_area[MAPLEN / sizeof(int)]));
         virt_addr += PAGE_SIZE)
    {
        /*
            error: implicit declaration of function ‘mem_map_reserve’
            `mem_map_reserve` can be found in linux-2.4, but not define in linux-2.6
            you can use `SetPageReserved` instead...

            #define SetPageReserved(page)   set_bit(PG_reserved, &(page)->flags)
            #define mem_map_reserve(p)      set_bit(PG_reserved, &((p)->flags))

        so the code

            mem_map_reserve(virt_to_page(vaddr_to_kaddr((void *)virt_addr)));

        use the next instead

            SetPageReserved(virt_to_page(vaddr_to_kaddr((void *)virt_addr)));

            it's the same to mem_map_unreserve
        */
        SetPageReserved(virt_to_page(vaddr_to_kaddr((void *)virt_addr)));
    }

    /* set a hello message to kernel space for read by user */
    strcpy((char*)vmalloc_area, "hello world from kernel space !");


    printk("vmalloc_area at 0x%p (phys 0x%lx)\n",
            vmalloc_area,
            virt_to_phys((void *)vaddr_to_kaddr(vmalloc_area)));


    return (0);
}


static void __exit mapdrv_exit(void)
{
    unsigned long   virt_addr;

    /* unreserve all pages */
    for (virt_addr = (unsigned long)vmalloc_area;
         virt_addr < (unsigned long)(&(vmalloc_area[MAPLEN / sizeof(int)]));
         virt_addr += PAGE_SIZE)
    {
        /*
         *  error: implicit declaration of function ‘mem_map_unreserve’`.
         *  it's the same to mem_map_reserve

        so the code

            mem_map_unreserve(virt_to_page(vaddr_to_kaddr((void *)virt_addr)));

        use the next code instead...

            ClearPageReserved(virt_to_page(vaddr_to_kaddr((void *)virt_addr)));
        */
        ClearPageReserved(virt_to_page(vaddr_to_kaddr((void *)virt_addr)));
    }
    /* and free the two areas */
    if (vmalloc_area)
    {
        vfree(vmalloc_area);
    }

    /* unregister the device */
    unregister_chrdev(major, "mapdrv");


    return;

}
/* device open method */
int mapdrv_open(struct inode *inode, struct file *file)
{
/*
    the code in linux-2.4
        MOD_INC_USE_COUNT;
    use the next instead...
        try_module_get(THIS_MODULE);
*/
    try_module_get(THIS_MODULE);
    return(0);
}

/* device close method */
int mapdrv_release(struct inode *inode, struct file *file)
{
    /*
    the code in linux-2.4
        MOD_DEC_USE_COUNT;
    use the next instead
        module_put(THIS_MODULE);
    */
    module_put(THIS_MODULE);

    return(0);
}

int mapdrv_mmap(struct file *file, struct vm_area_struct *vma)
{
   unsigned long offset = vma->vm_pgoff<<PAGE_SHIFT;

   unsigned long size = vma->vm_end - vma->vm_start;
   if (offset & ~PAGE_MASK)
   {
    printk("offset not aligned: %ld\n", offset);
    return -ENXIO;
   }
   if (size>MAPLEN)
   {
    printk("size too big\n");
    return(-ENXIO);
   }
  /*  only support shared mappings.*/
   if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED))
   {
    printk("writeable mappings must be shared, rejecting\n");
    return(-EINVAL);
   }
  /* do not want to have this area swapped out, lock it */
    vma->vm_flags |= VM_LOCKED;
   if (offset == 0)
   {
    vma->vm_ops = &map_vm_ops;
   /* call the open routine to increment the usage count */
    map_vopen(vma);
   }else
   {
     printk("offset out of range\n");
     return -ENXIO;
   }
   return(0);
}
/* open handler for vm area */
void map_vopen(struct vm_area_struct *vma)
{
  /* needed to prevent the unloading of the module while
  somebody still has memory mapped */
//     MOD_INC_USE_COUNT;

/*
    the code in linux-2.4
        MOD_INC_USE_COUNT;
    use the next instead...
        try_module_get(THIS_MODULE);
*/
    try_module_get(THIS_MODULE);
}

/* close handler form vm area */
void map_vclose(struct vm_area_struct *vma)
{

    /*
    the code in linux-2.4
        MOD_DEC_USE_COUNT;
    use the next instead
        module_put(THIS_MODULE);
    */
    module_put(THIS_MODULE);
}

/* page fault handler */
struct page *map_nopage(struct vm_area_struct *vma, unsigned long address, int write_access)
{
    unsigned long offset;
    unsigned long virt_addr;

    /* determine the offset within the vmalloc'd area  */
    offset = address - vma->vm_start + (vma->vm_pgoff << PAGE_SHIFT);

    /* translate the vmalloc address to kmalloc address  */
    virt_addr = (unsigned long)vaddr_to_kaddr(&vmalloc_area[offset / sizeof(int)]);
    if (virt_addr == 0UL)
    {
        return((struct page *)0UL);
    }

    /* increment the usage count of the page */
    atomic_inc(&(virt_to_page(virt_addr)->_count));
    printk("map_drv: page fault for offset 0x%lx (kseg x%lx)\n",offset, virt_addr);

    return(virt_to_page(virt_addr));

}




module_init(mapdrv_init);
module_exit(mapdrv_exit);
