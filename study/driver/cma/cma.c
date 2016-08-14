/* 
 * kernel module helper for testing CMA 
 * 
 * Licensed under GPLv2 or later. 
 */  
  
#include <linux/module.h>   
#include <linux/device.h>   
#include <linux/fs.h>   
#include <linux/miscdevice.h>   
#include <linux/dma-mapping.h>   
  
#define CMA_NUM  10   
static struct device *cma_dev;  
static dma_addr_t dma_phys[CMA_NUM];  
static void *dma_virt[CMA_NUM];  
  
/* any read request will free coherent memory, eg. 
 * cat /dev/cma_test 
 */  
static ssize_t  
cma_test_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)  
{  
    int i;  
  
    for (i = 0; i < CMA_NUM; i++) {  
        if (dma_virt[i]) {  
            dma_free_coherent(cma_dev, (i + 1) * SZ_1M, dma_virt[i], dma_phys[i]);  
            _dev_info(cma_dev, "free virt: %p phys: %p\n", dma_virt[i], (void *)dma_phys[i]);  
            dma_virt[i] = NULL;  
            break;  
        }  
    }  
    return 0;  
}  
  
/* 
 * any write request will alloc coherent memory, eg. 
 * echo 0 > /dev/cma_test 
 */  
static ssize_t  
cma_test_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)  
{  
    int i;  
    int ret;  
  
    for (i = 0; i < CMA_NUM; i++) {  
        if (!dma_virt[i]) {  
            dma_virt[i] = dma_alloc_coherent(cma_dev, (i + 1) * SZ_1M, &dma_phys[i], GFP_KERNEL);  
  
            if (dma_virt[i]) {  
                void *p;  
                /* touch every page in the allocated memory */  
                for (p = dma_virt[i]; p <  dma_virt[i] + (i + 1) * SZ_1M; p += PAGE_SIZE)  
                    *(u32 *)p = 0;  
  
                _dev_info(cma_dev, "alloc virt: %p phys: %p\n", dma_virt[i], (void *)dma_phys[i]);  
            } else {  
                dev_err(cma_dev, "no mem in CMA area\n");  
                ret = -ENOMEM;  
            }  
            break;  
        }  
    }  
  
    return count;  
}  
  
static const struct file_operations cma_test_fops = {  
    .owner =    THIS_MODULE,  
    .read  =    cma_test_read,  
    .write =    cma_test_write,  
};  
  
static struct miscdevice cma_test_misc = {  
    .name = "cma_test",  
    .fops = &cma_test_fops,  
};  
  
static int __init cma_test_init(void)  
{  
    int ret = 0;  
  
    ret = misc_register(&cma_test_misc);  
    if (unlikely(ret)) {  
        pr_err("failed to register cma test misc device!\n");  
        return ret;  
    }  
    cma_dev = cma_test_misc.this_device;  
    cma_dev->coherent_dma_mask = ~0;  
    _dev_info(cma_dev, "registered.\n");  
  
    return ret;  
}  
module_init(cma_test_init);  
  
static void __exit cma_test_exit(void)  
{  
    misc_deregister(&cma_test_misc);  
}  
module_exit(cma_test_exit);  
  
MODULE_LICENSE("GPL");  
MODULE_AUTHOR("Barry Song <21cnbao@gmail.com>");  
MODULE_DESCRIPTION("kernel module to help the test of CMA");  
MODULE_ALIAS("CMA test");  