
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>

MODULE_LICENSE("GPL");

#define MAJOR_NUM 224

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
static struct semaphore sem;
static spinlock_t spin = SPIN_LOCK_UNLOCKED;

static int simple_open(struct inode *inode, struct file *filp)
{
	/*获得自选锁*/
	spin_lock(&spin);
	
	/*临界资源访问*/
	if (simple_count)
	{
		spin_unlock(&spin);
		return - EBUSY;
	}
	simple_count++;
	
	/*释放自选锁*/
	spin_unlock(&spin);
	return 0;
}

static int simple_release(struct inode *inode, struct file *filp)
{
	simple_count--;
	return 0;
}

static ssize_t simple_read(struct file *filp, char *buf, size_t len, loff_t*off)
{
	return 0;
}

static ssize_t simple_write(struct file *filp, const char *buf, size_t len,loff_t *off)
{
	return 0;
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
		init_MUTEX(&sem);
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
