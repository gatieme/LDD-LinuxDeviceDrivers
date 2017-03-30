#include <linux/module.h>
#include <linux/sched.h> //jiffies
#include <linux/proc_fs.h>
#include <linux/uaccess.h> //copy_to|from_user()

static char *str = NULL;

//proc文件的读函数
static int my_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int ret = 0;
	ret = sprintf(page, "current kernel time is %ld\n", jiffies);
	ret += sprintf(page+ret, "str is %s\n", str);

	return ret;
}

//proc文件的写函数
static int my_proc_write(struct file *filp, const char __user *buf, unsigned long count, void *data)
{
	//分配临时缓冲区
	char *tmp = kzalloc((count+1), GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	//将用户态write的字符串拷贝到内核空间
	//copy_to|from_user(to,from,cnt)
	if (copy_from_user(tmp, buf, count)) {
		kfree(tmp);
		return -EFAULT;
	}

	//将str的旧空间释放，然后将tmp赋值给str
	kfree(str);
	str = tmp;

	return count;
}


static int __init my_init(void)
{
	struct proc_dir_entry *file;

	//创建proc文件
	file = create_proc_entry("jif", 0666, NULL);
	if (!file) {
		printk("Cannot create /proc/jif\n");
		return -1;
	}

	//将创建好的文件和读写函数关联在一起
	file->read_proc = my_proc_read;
	file->write_proc = my_proc_write;

	return 0;
}

static void __exit my_exit(void)
{
	//删除proc文件
	remove_proc_entry("jif", NULL);
	kfree(str);
}

module_init(my_init);
module_exit(my_exit);
MODULE_AUTHOR("aran");
MODULE_LICENSE("GPL");
