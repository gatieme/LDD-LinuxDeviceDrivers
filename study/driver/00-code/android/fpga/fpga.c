/*
FPGA test
byTX
2013.1.31：create
*/
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
//for file opration
#include <linux/fs.h>
//for cdev create
#include <linux/cdev.h>
//copy_to_user
#include <linux/uaccess.h>
//class_create
#include <linux/device.h>
//ioremap defined here
#include <asm/io.h>


#include <linux/version.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Gatieme");
MODULE_DESCRIPTION("hello world");

#define FPGA_SIZE 1024
#define FPGA_BASE 0x80010000

#define CMD_FPGA_MODULE_RST 0x20
#define REG_RST_OFFSET 0x0006
static void __iomem *io_base;
struct cdev *fpga_cdev;
struct class *fpga_class;
dev_t fpga_devid;

static int fpga_open(struct inode *inode, struct file *file)
{
	printk("fpga open\n");
	return 0;
}
ssize_t fpga_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	*(unsigned short int*)buf=ioread16(io_base+(*f_pos));
	return 2;
}
/*fpga write*/
ssize_t fpga_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	if(count==2)
	{
		iowrite16(*(unsigned short int*)buf,io_base+(*f_pos));
		return 2;
	}
	else
		return 0;
}

loff_t fpga_llseek (struct file *filp, loff_t offset, int whence)
{
	loff_t new_pos;
	if(offset>=0 && offset<FPGA_SIZE)
		new_pos=offset;
	else
		new_pos=0;
	filp->f_pos = new_pos;
	return new_pos;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
static int fpga_ioctl(struct inode *indoe, struct file *file,
		unsigned int cmd, unsigned long arg)
{
#else
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
//long (*compat_ioctl) (struct file *file, unsigned int cmd, unsigned long arg)
static long fpga_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    //struct inode *inode = file->f_dentry->d_inode;
    struct inode *inode = inode = file_inode(file);
#endif
	switch(cmd)
	{
		case CMD_FPGA_MODULE_RST:
			iowrite16(0, io_base+REG_RST_OFFSET);
			iowrite16(1, io_base+REG_RST_OFFSET);
		break;
	}
	return 0;
}

static struct file_operations fpga_fops = {
	.owner 	= THIS_MODULE,
	.open	= fpga_open,
	.read	= fpga_read,
	.write = fpga_write,
	.llseek = fpga_llseek,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
    .ioctl = fpga_ioctl,
#else
    .compat_ioctl = fpga_compat_ioctl,
#endif
};
static int __init fpga_init(void)
{

	//创建字符设备
	fpga_cdev=cdev_alloc();
	cdev_init(fpga_cdev, &fpga_fops);
	fpga_cdev->owner = THIS_MODULE;

	alloc_chrdev_region(&fpga_devid, 0, 1, "fpga");
	cdev_add(fpga_cdev,fpga_devid,1);

	fpga_class = class_create(THIS_MODULE,"fpga_class");
	device_create(fpga_class, NULL, fpga_devid, NULL,"fpga");
	//申请内存
	io_base=ioremap(FPGA_BASE,FPGA_SIZE);
	if(io_base==0)
		printk("failed to ioremap fpga\n");
	printk("fpga initialized.\n");
	return 0;
}
static void __exit fpga_exit(void)
{
 	unregister_chrdev_region(fpga_devid,1);
	device_destroy(fpga_class , fpga_devid);
	class_destroy(fpga_class);
	cdev_del(fpga_cdev);
	iounmap(io_base);
	printk("fpga exit\n");
}

module_init(fpga_init);
module_exit(fpga_exit);

