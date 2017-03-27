#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <asm/uaccess.h>

//  http://files.cnblogs.com/wwang/debugfs.zip

struct dentry *my_debugfs_root;

u8                          a           = 0;
char                        hello[32]   = "Hello world!\n";
struct debugfs_blob_wrapper b;

static int c_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

static ssize_t c_read(struct file *filp, char __user *buffer,
		size_t count, loff_t *ppos)
{
	if (*ppos >= 32)
		return 0;
	if (*ppos + count > 32)
		count = 32 - *ppos;

	if (copy_to_user(buffer, hello + *ppos, count))
		return -EFAULT;

	*ppos += count;

	return count;
}

static ssize_t c_write(struct file *filp, const char __user *buffer,
		size_t count, loff_t *ppos)
{
	if (*ppos >= 32)
		return 0;
	if (*ppos + count > 32)
		count = 32 - *ppos;

	if (copy_from_user(hello + *ppos, buffer, count))
		return -EFAULT;

	*ppos += count;

	return count;
}

struct file_operations c_fops = {
	.owner = THIS_MODULE,
	.open = c_open,
	.read = c_read,
	.write = c_write,
};

static int __init mydebugfs_init(void)
{
	struct dentry *sub_dir, *r_a, *r_b, *s_c;

        printk(KERN_INFO "mydebugfs_init\n");

	my_debugfs_root = debugfs_create_dir("gatieme_mydebug", NULL);
	if (!my_debugfs_root)
		return -ENOENT;

	r_a = debugfs_create_u8("a", 0644, my_debugfs_root, &a);
	if (!r_a)
		goto Fail;

	b.data = (void *)hello;
	b.size = strlen(hello) + 1;
	r_b = debugfs_create_blob("b", 0644, my_debugfs_root, &b);
	if (!r_b)
		goto Fail;

	sub_dir = debugfs_create_dir("subdir", my_debugfs_root);
	if (!sub_dir)
		goto Fail;

	s_c = debugfs_create_file("c", 0644, sub_dir, NULL, &c_fops);
	if (!s_c)
		goto Fail;

        return 0;

Fail:
	debugfs_remove_recursive(my_debugfs_root);
	my_debugfs_root = NULL;
	return -ENOENT;
}

static void __exit mydebugfs_exit(void)
{
    printk(KERN_INFO "mydebugfs_exit\n");
	debugfs_remove_recursive(my_debugfs_root);

    return;
}

module_init(mydebugfs_init);
module_exit(mydebugfs_exit);

MODULE_LICENSE("GPL");

