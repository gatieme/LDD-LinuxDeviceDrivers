/*
CPLD test
byTX
2013.1.31：add cpld_ioctl
*/
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
//#include <linux/serial_core.h>
//for file opration
#include <linux/fs.h>
//for cdev create
#include <linux/cdev.h>
//#include <linux/gpio.h>

//#include <linux/clk.h>
//#include <linux/delay.h>
//#include <linux/usb/ch9.h>
//#include <linux/pwm_backlight.h>
//#include <linux/spi/spi.h>
//#include <linux/gpio_keys.h>
//copy_to_user
#include <linux/uaccess.h>
//class_create
#include <linux/device.h>
//#include <asm/mach/arch.h>
//#include <asm/mach/map.h>
//#include <asm/setup.h>
//#include <asm/mach-types.h>
//ioremap defined here
#include <asm/io.h>
#include "slaveSerial.h"
//#include <plat/s5pv210.h>

#include <linux/version.h>
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Gatieme");
MODULE_DESCRIPTION("hello world");


static void __iomem *io_base;
struct cdev *cpld_cdev;
struct class *cpld_class;
dev_t devid;

static int cpld_open(struct inode *inode, struct file *file)
{
	//可配置eint18，并复位cpld
	printk("cpld open\n");
	return 0;
}
//read ,do not change f_pos
ssize_t cpld_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned short int tmp;
#if DEBUG_PRINT
	printk("f_pos=0x%x\n",*(unsigned int*)f_pos);
#endif
	tmp=ioread16(io_base+(*f_pos));
#if DEBUG_PRINT
	printk("cpld read =0x%x\n",tmp);
#endif
	//return copy_to_user(buf,(char *)&tmp,sizeof(unsigned short int));
	*(unsigned short int*)buf=tmp;
	return 2;//always read 2bytes
}
//write ,do not change f_pos
ssize_t cpld_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
#if DEBUG_PRINT
	printk("f_pos=0x%x  count=%d\n",*(unsigned int*)f_pos,count);
#endif
	if(count==2)
	{
		iowrite16(*(unsigned short int*)buf,io_base+(*f_pos));
#if DEBUG_PRINT
		printk("write data=0x%x\n",*(unsigned short int*)buf);
#endif
		return count;
	}
/*
	else if(count==FPGA_BURST_SIZE)//多字节连续写入
	{
		for(i=0;i<count;i++)
		{
#if DEBUG_PRINT
			printk("write data=0x%x,addr=0x%x\n",*(unsigned short int*)(buf+2*i),i*2);
#endif
			iowrite16(*(unsigned short int*)(buf+2*i),io_base+i*2);
		}
		return count;
	}
*/
	else
		return 0;
}
//set the offset
loff_t cpld_llseek (struct file *filp, loff_t offset, int whence)
{
	loff_t new_pos;
	if(offset>=0 && offset<CPLD_SIZE)
		new_pos=offset;
	else
		new_pos=0;
#if DEBUG_PRINT
	printk("set cpld offset to 0x%x\n",(unsigned int)new_pos);
#endif
	filp->f_pos = new_pos;
	return new_pos;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
static int cpld_ioctl(struct inode *indoe, struct file *file,
		unsigned int cmd, unsigned long arg)
{
#else
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
//long (*compat_ioctl) (struct file *file, unsigned int cmd, unsigned long arg)
static long cpld_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    //struct inode *inode = file->f_dentry->d_inode;
    struct inode *inode = inode = file_inode(file);
#endif
	switch(cmd)
	{
		case CMD_START_SHIFT:
			shiftWordOut((unsigned short int)arg,io_base);
			break;
		case CMD_START_PROG:
			set_prog_b(io_base,0);  //?????? 这函数功能-设置boot值，io_base+REG_PROG_B_OFFSET
			set_prog_b(io_base,0);
			set_prog_b(io_base,1);
			break;
		case CMD_CHECK_DONE:
			return check_done(io_base);  //这check_done在slaveSerial.c中，检查是否下载完成
		case CMD_SET_LED:
			iowrite16((arg>0)?SET_LED_ON:SET_LED_OFF, io_base+REG_LED_OFFSET);
			break;
		case CMD_SEND_LOW_BYTE:
			shiftByteOut((char)arg,io_base);
			break;
		case CMD_SEND_HIGH_BYTE:
			shiftByteOut((char)(arg>>8),io_base);
			break;
		case CMD_CHECK_INIT:
			return get_init_bit(io_base);
			break;
		case CMD_SET_CONTROL:
			iowrite16((unsigned short int)arg,io_base+REG_CONTROL_OFFSET); ///?????REG_CONTROL_OFFSET在哪里定义的？arg是要设置的值，第二个参数是地址
			break;
		case CMD_SET_DATA:
			iowrite16((unsigned short int)arg,io_base+REG_DATA_OFFSET);
			break;
		default:
			break;
	}
	return 0;
}


static struct file_operations cpld_fops = {
	.owner 	= THIS_MODULE,
	.open	= cpld_open,
	.read	= cpld_read,
	.write = cpld_write,
	.llseek = cpld_llseek,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
    .ioctl = cpld_ioctl,
#else
    .compat_ioctl = cpld_compat_ioctl,
#endif
};
static int __init cpld_init(void)
{

	//创建字符设备
	cpld_cdev=cdev_alloc();
	cdev_init(cpld_cdev, &cpld_fops);
	cpld_cdev->owner = THIS_MODULE;

	alloc_chrdev_region(&devid, 0, 1, "cpld");
	cdev_add(cpld_cdev,devid,1);

	cpld_class = class_create(THIS_MODULE,"cpld_class");
	device_create(cpld_class, NULL, devid, NULL,"cpld");
	//申请内存
	io_base=ioremap(CPLD_BASE,CPLD_SIZE);
	if(io_base==0)
		printk("failed to ioremap cpld\n");
	else
		printk("cpld test reg.=0x%x\n",ioread16(io_base+REG_TEST_OFFSET));
	printk("CPLD initialized.\n");
	return 0;
}
static void __exit cpld_exit(void)
{
	unregister_chrdev_region(devid,1);
	device_destroy(cpld_class , devid);
	class_destroy(cpld_class);
	cdev_del(cpld_cdev);
	iounmap(io_base);
	printk("cpld exit\n");
}

module_init(cpld_init);
module_exit(cpld_exit);

