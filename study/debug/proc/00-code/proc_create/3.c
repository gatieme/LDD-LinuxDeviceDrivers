/*************************************************
 使用seq_file接口实现可读写proc文件的例子
 适用于3.10以后的内核
 Author: ZhangN
 Date: 2015-5-17
 *************************************************/
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>

static char *str = NULL;

/*5,实现show函数
  作用是将内核数据输出到用户空间
  将在proc file输出时被调用*/
static int my_proc_show(struct seq_file *m, void *v)
{
    /*这里不能使用printfk之类的函数
      要使用seq_file输出的一组特殊函数
      详见ldd3的91页*/
    seq_printf(m, "current kernel time is %ld\n", jiffies);
    seq_printf(m, "str is %s\n", str);
    return 0;
}



/*3,实现open和write函数*/
static ssize_t my_proc_write(struct file *file, const char __user *buffer,
                             size_t count, loff_t *f_pos)
{
    char *tmp = kzalloc((count+1), GFP_KERNEL);
    if(!tmp)
        return -ENOMEM;
    if(copy_from_user(tmp, buffer, count))
    {
        kfree(tmp);
        return EFAULT;
    }
    kfree(str);
    str = tmp;
    return count;
}

static int my_proc_open(struct inode *inode, struct file *file)
{
    /*4,在open函数中调用single_open绑定seq_show函数指针
      需要说明的是，ldd3中介绍的seq接口用该调用seq_open函数
      其调用形式如下：
      return sep_open(file, &scull_seq_ops);
      scull_seq_ops为struct seq_operations结构体
      在该结构体中绑定show函数指针
      需要准备seq_operations结构体
      而调用single_open函数只需直接指定show的函数指针即可
      个人猜测可能是在single_open函数中实现了seq_operations结构体
      至于是不是就不知道了，没有查看具体实现
      有兴趣的同学可以参考文档：Documentation\filesystems\seq_file.txt
      关于第三个参数，其类型应为viod*,
      内核中有些地方传入的NULL，有些地方传入的inode->i_private，也有传入其他值的
      来看看data在single_open函数中如何被使用的：
        if (!res)
         ((struct seq_file *)file->private_data)->private = data;
      data是seq_file结构体的private成员。
      那么data如何真正被使用的呢？
      发现show函数的第一个参数为seq_file类型，在show函数中，
      可以将seq_file的private成员转换成对应的类型进行使用。
      也就是说，可以通过seq_file的private成员将data参数传递到show函数中*/
    return single_open(file, my_proc_show, NULL);
}

/*2,填充proc_create函数中调用的flie_operations结构体
  其中my开头的函数为自己实现的函数，
  seq和single开头为内核实现好的函数，直接填充上就行
  open为必须填充函数
  这里详见ldd3的93页*/
static struct file_operations my_fops = {
    .owner   = THIS_MODULE,
    .open    = my_proc_open,
    .release = single_release,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .write   = my_proc_write,
};

static int __init my_init(void)
{
    struct proc_dri_entry *file;
    /*3.10以后内核的proc文件的新接口
      需要关联file_operations*/
    /*1,首先要调用创建proc文件的函数，需要绑定flie_operations*/
    file = proc_create("abc_proc", 0644, NULL, &my_fops);
    if(!file)
        return -ENOMEM;
    return 0;
}

/*6,删除proc文件*/
static void __exit my_exit(void)
{
    remove_proc_entry("abc_proc", NULL);
    kfree(str);
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZhangN");
