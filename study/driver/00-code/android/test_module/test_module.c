#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>

static int g_testmodule_major = 0;

struct testmodule_dev{
    struct cdev dev;
    char buf[100];
};

struct testmodule_dev* testmodule_devp;

static ssize_t testmodule_read(struct file* filp, char __user* buf, size_t buf_size, loff_t* f_pos){
    struct testmodule_dev* dev = filp->private_data;

    if(copy_to_user(buf,dev->buf, 100))
    {
        return -EFAULT;
    }

    return 100;
}

static ssize_t testmodule_write(struct file* filp, const char __user* buf, size_t buf_size, loff_t* f_pos){
    struct testmodule_dev* dev = filp->private_data;

    if(copy_from_user(dev->buf,buf,buf_size))
    {
        return -EFAULT;
    }

    return buf_size;
}

static int testmodule_open(struct inode* node, struct file* filp){
    struct testmodule_dev* dev = container_of(node->i_cdev,struct testmodule_dev, dev);
    filp->private_data = dev;
    return 0;
}

static long testmodule_ioctl(struct file* filp, unsigned int cmd, unsigned long arg){
    struct testmodule_dev* dev = filp->private_data;
    switch(cmd){
    case 0:
        memset(dev->buf,0,100);
        printk(KERN_INFO "buf set to zero!");
        break;
    case 1:
        memset(dev->buf,1,100);
        printk(KERN_INFO "buf set to 1!");
        break;
    default:
        break;
    }
    return 0;
}

static const struct file_operations testmodule_fops = {
    .owner = THIS_MODULE,
    .read = testmodule_read,
    .write = testmodule_write,
    .unlocked_ioctl = testmodule_ioctl,
    .open = testmodule_open
};

void updateDev(struct testmodule_dev* dev, int index){
    dev_t devno = MKDEV(g_testmodule_major, index);
    int error;
    cdev_init(&dev->dev, &testmodule_fops);
    dev->dev.owner = THIS_MODULE;
    error = cdev_add(&dev->dev, devno, 1);
    if(error)
        printk(KERN_NOTICE "Error %d adding testmodule %d", error, index);
}

int __init testmodule_init(void){
    dev_t device_no;
    /*Get Major Number*/
    alloc_chrdev_region(&device_no,0,1,"testmodule");
    g_testmodule_major = MAJOR(device_no);

    /*Generate device struct*/
    testmodule_devp = kmalloc(sizeof(struct testmodule_dev), GFP_KERNEL);
    if(!testmodule_devp){
        /*Generate Failed*/
        int result;
        result = -ENOMEM;
        unregister_chrdev_region(device_no, 0);
        return result;
    }
    memset(testmodule_devp,0,sizeof(struct testmodule_dev));

    /*Update device struct*/
    updateDev(testmodule_devp,MINOR(device_no));
    return 0;
}

void __exit testmodule_exit(void){
    cdev_del(&testmodule_devp->dev);
    kfree(testmodule_devp);
    unregister_chrdev_region(MKDEV(g_testmodule_major,0),1);
}

module_init(testmodule_init);
module_exit(testmodule_exit);

MODULE_LICENSE("Dual BSD/GPL");
