#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/fs.h>		// for basic filesystem
#include <linux/proc_fs.h>	// for the proc filesystem
#include <linux/seq_file.h>	// for sequence files
#include <linux/jiffies.h>	// for jiffies
#include <linux/slab.h>		// for kzalloc, kfree
#include <linux/uaccess.h>	// for copy_from_user

#define BUF_SIZE 128

// global var
static char *str = NULL;

// seq_operations -> show
static int jif_show(struct seq_file *m, void *v)
{
	char buf[BUF_SIZE];
	int ret = 0;
	ret = sprintf(buf, "current kernel time is %llu\n", (unsigned long long) get_jiffies_64());
	ret += sprintf(buf + ret, "str is %s\n", str);

	seq_printf(m, "%s", buf);

	return 0; //!! must be 0, or will show nothing T.T
}

// file_operations -> write
static ssize_t jif_write(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos)
{
	//分配临时缓冲区
	char *tmp = kzalloc((count+1), GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;
    printk(KERN_DEBUG "user buffer : 0x%lx %s\n", buffer, buffer);
	//将用户态write的字符串拷贝到内核空间
	//copy_to|from_user(to,from,cnt)
	if (copy_from_user(tmp, buffer, count)) {
		kfree(tmp);
		return -EFAULT;
	}

	//将str的旧空间释放，然后将tmp赋值给str
	kfree(str);
	str = tmp;

	return count;
}

// seq_operations -> open
static int jif_open(struct inode *inode, struct file *file)
{
	return single_open(file, jif_show, NULL);
}

static const struct file_operations jif_fops =
{
	.owner		= THIS_MODULE,
	.open		= jif_open,
	.read		= seq_read,
	.write 		= jif_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

// module init
static int __init jif_init(void)
{
	struct proc_dir_entry* jif_file;

	jif_file = proc_create("jif", 0666, NULL, &jif_fops);
	if (NULL == jif_file)
	{
	    return -ENOMEM;
	}

	return 0;
}

// module exit
static void __exit jif_exit(void)
{
	remove_proc_entry("jif", NULL);
	kfree(str);
}

module_init(jif_init);
module_exit(jif_exit);

MODULE_AUTHOR("aran");
MODULE_LICENSE("GPL");
