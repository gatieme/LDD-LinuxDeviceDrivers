#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>

//  http://blog.csdn.net/wealoong/article/details/7992071

struct dentry   *pcie_file = NULL;
static int      pcie0_linked = 1;

static int pcie_debugfs_read(struct seq_file *s, void *data)
{
    seq_printf(s, "pcie0_link status : %s\n", pcie0_linked == 1 ? "Enable": "Disable");
    return 0;
}

static int pcie_single_open(struct inode *inode, struct file *file)
{
    //return single_open(file, pcie_reg_open, NULL);
    return single_open(file, pcie_debugfs_read, inode->i_private);
}

static ssize_t pcie_debugfs_write(struct file *file,
                                  const char __user *userbuf,
                                  size_t count, loff_t *ppos)
{
    char    buff[20];
    int     iRet;

    if(count <= 1)
    {
        return -EFAULT;
    }

    memset(buff, '\0', sizeof(buff));
    if (copy_from_user(buff, userbuf, count))
    {
        return -EFAULT;
    }

    printk("[%s, %d] : %s \n", __FUNCTION__, __LINE__, buff);

    iRet = sscanf(buff, "%d", &pcie0_linked);        //  将读出来的数据sPid赋值给模块的全局变量pid
    if(iRet != 1)
    {
        return -EFAULT;
    }

    pcie0_linked = !!pcie0_linked;
    printk("[%s, %d] : pcie0_linked = %d\n", __FUNCTION__, __LINE__, pcie0_linked);

    return 1;
}

static const struct file_operations pcie_ios_fops = {
    .open    = pcie_single_open,
    .read    = seq_read,
    .write   = pcie_debugfs_write,
    .lseek  = seq_lseek,
    .release = single_release,
};


static int __init pcie_debugfs_init(void)
{
    pcie_file = debugfs_create_file("gatieme_debugfs_file3", 0644, NULL, NULL, &pcie_ios_fops);
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

