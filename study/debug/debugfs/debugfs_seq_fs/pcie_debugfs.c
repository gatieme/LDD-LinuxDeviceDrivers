#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>

//  http://blog.csdn.net/wealoong/article/details/7992071

struct dentry   *pcie_file;
static int      pcie0_linked = 1;

static int pcie_reg_open(struct seq_file *s, void *data)
{
    seq_printf(s, "pcie0_link status : %s\n", pcie0_linked == 1 ? "Enable": "Disable");
    return 0;
}

static int pcie_single_open(struct inode *inode, struct file *file)
{
    return single_open(file, pcie_reg_open, NULL);
    //return single_open(file, pcie_reg_open, inode->i_private);
}

static ssize_t pcie_debug_write(struct file *file,
                                const char __user *userbuf,
                                size_t count, loff_t *ppos)
{
    char buf[20];

    if (copy_from_user(buf, userbuf, min(count, sizeof(buf))))
        return -EFAULT;

    printk("%s: %s \n", __FUNCTION__, buf);

    return count;
}

static const struct file_operations pcie_ios_fops = {
    .open = pcie_single_open,
    .read = seq_read,
    .write = pcie_debug_write,
    .llseek = seq_lseek,
    .release = single_release,

};


static int __init pcie_debugfs_init(void)
{
    pcie_file = debugfs_create_file("gatieme_debugfs_file", 0644, NULL, NULL, &pcie_ios_fops);
    if(pcie_file == NULL)
    {
		return -ENOENT;
    }
    return 0;
}


static void __exit pcie_debugfs_exit(void)
{
    printk(KERN_INFO "mydebugfs_exit\n");
	debugfs_remove_recursive(pcie_file);
    //debugfs_remove(pcie_dir);
    return;
}

module_init(pcie_debugfs_init);
module_exit(pcie_debugfs_exit);

MODULE_LICENSE("GPL");

