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
#include <linux/rcupdate.h>
#include <linux/spinlock.h>
#include "demo.h"

MODULE_AUTHOR("fgj");
MODULE_LICENSE("Dual BSD/GPL");

struct protectRcu
{
    int protect;
    struct rcu_head rcu;
};

struct protectRcu*global_pr=NULL;
//一般用来释放老的数据
void callback_function(struct rcu_head *r)
{
	struct protectRcu *t;
	t = container_of (r, struct protectRcu, rcu );
	kfree (t);
	printk("callback_function kfree\n");
}

struct DEMO_dev *DEMO_devices;
static unsigned char demo_inc=0;
static u8 demoBuffer[256];
static spinlock_t foo_spinlock = SPIN_LOCK_UNLOCKED;

int DEMO_open(struct inode *inode, struct file *filp)
{
	struct DEMO_dev *dev;
	demo_inc++;

	dev = container_of(inode->i_cdev, struct DEMO_dev, cdev);
	filp->private_data = dev;
	return 0;
}

int DEMO_release(struct inode *inode, struct file *filp)
{
	demo_inc--;
	return 0;
}

ssize_t DEMO_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos)
{
	rcu_read_lock ();
	if (copy_to_user(buf,demoBuffer,count))
	{
	   count=-EFAULT; /* 把数据写到应用程序空间 */
	   goto out;
	}
  out:
	rcu_read_unlock ();
	return count;
}

ssize_t DEMO_write(struct file *filp, const char __user *buf, size_t count,loff_t *f_pos)
{
	struct protectRcu *t,*old;
        if(copy_from_user(demoBuffer, buf,count))return -EFAULT;
	t = kmalloc (sizeof (*t), GFP_KERNEL );
	spin_lock (&foo_spinlock);
	t-> protect= demoBuffer[0];
	old= global_pr;
	global_pr=t;
	spin_unlock (&foo_spinlock);
	if(old!=NULL)call_rcu (&old->rcu , callback_function);
	return 0;
}

struct file_operations DEMO_fops = {
	.owner =    THIS_MODULE,
	.read =     DEMO_read,
	.write =    DEMO_write,
	.open =     DEMO_open,
	.release =  DEMO_release,
};

/*******************************************************
                MODULE ROUTINE
*******************************************************/
void DEMO_cleanup_module(void)
{
	dev_t devno = MKDEV(DEMO_MAJOR, DEMO_MINOR);

	if (DEMO_devices) 
	{
		cdev_del(&DEMO_devices->cdev);
		kfree(DEMO_devices);
	}
	unregister_chrdev_region(devno,1);
}

int DEMO_init_module(void)
{
	int result;
	dev_t dev = 0;

	dev = MKDEV(DEMO_MAJOR, DEMO_MINOR);
	result = register_chrdev_region(dev, 1, "DEMO");
	if (result < 0) 
	{
		printk(KERN_WARNING "DEMO: can't get major %d\n", DEMO_MAJOR);
		return result;
	}

	DEMO_devices = kmalloc(sizeof(struct DEMO_dev), GFP_KERNEL);
	if (!DEMO_devices)
	{
		result = -ENOMEM;
		goto fail;
	}
	memset(DEMO_devices, 0, sizeof(struct DEMO_dev));

	cdev_init(&DEMO_devices->cdev, &DEMO_fops);
	DEMO_devices->cdev.owner = THIS_MODULE;
	DEMO_devices->cdev.ops = &DEMO_fops;
	result = cdev_add (&DEMO_devices->cdev, dev, 1);
	if(result)
	{
		printk(KERN_NOTICE "Error %d adding DEMO\n", result);
		goto fail;
	}
	return 0;

fail:
	DEMO_cleanup_module();
	return result;
}
module_init(DEMO_init_module);
module_exit(DEMO_cleanup_module);
