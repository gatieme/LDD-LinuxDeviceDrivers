
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>

MODULE_LICENSE("GPL");

#define MAJOR_NUM 224

rwlock_t lock = RW_LOCK_UNLOCKED;
char demoBuffer[256];

static ssize_t simple_read(struct file *, char *, size_t, loff_t*);
static ssize_t simple_write(struct file *, const char *, size_t, loff_t*);
static int simple_open(struct inode *inode, struct file *filp);
static int simple_release(struct inode *inode, struct file *filp);

struct file_operations simple_fops =
{
	read: simple_read, 
	write: simple_write, 
	open: simple_open,
	release:simple_release,
};

static int simple_var = 0;
static int simple_count = 0;

static int simple_open(struct inode *inode, struct file *filp)
{
	simple_count++;
	return 0;
}

static int simple_release(struct inode *inode, struct file *filp)
{
	simple_count--;
	return 0;
}

static ssize_t simple_read(struct file *filp, char *buf, size_t len, loff_t*off)
{
	int count=len;
	read_lock(&lock);
    if (copy_to_user(buf,demoBuffer,count))
	{
	   count=-EFAULT;
	}
	read_unlock(&lock);
	return count;
}

static ssize_t simple_write(struct file *filp, const char *buf, size_t len,loff_t *off)
{
	int count=len;
	write_lock(&lock);
    if (copy_from_user(demoBuffer, buf, count))
	{
		count = -EFAULT;
	}
	write_unlock(&lock);
	return count;
}


static int __init simple_init(void)
{
	int ret;
	/*注册设备驱动*/
	ret = register_chrdev(MAJOR_NUM, "chardev", &simple_fops);
	if (ret)
	{
		printk("chardev register failure\n");
	}
	else
	{
		printk("chardev register success\n");
	}
	return ret;
}

static void __exit simple_exit(void)
{
	/*注销设备驱动*/
	unregister_chrdev(MAJOR_NUM, "chardev");
}

module_init(simple_init);
module_exit(simple_exit); 
