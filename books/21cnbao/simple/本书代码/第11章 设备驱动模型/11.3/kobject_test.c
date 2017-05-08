#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/stat.h>
 
 
void kobject_test_release(struct kobject *kobject);/*释放kobject结构体的函数*/
/*读属性的函数*/
ssize_t kobject_test_show(struct kobject *kobject, struct attribute *attr,char *buf);
/*写属性的函数*/
ssize_t kobject_test_store(struct kobject *kobject,struct attribute *attr,const char *buf, size_t count);
/*定义了一个名为kobject_test，可以读写的属性*/
struct attribute test_attr = {
	.name = "kobject_test_attr",/*属性名*/
	.mode = S_IRWXUGO,/*属性为可读可写*/
};
/*该kobject只有一个属性*/
static struct attribute *def_attrs[] = {
	&test_attr,
	NULL,
};
struct sysfs_ops obj_test_sysops =
{
	.show = kobject_test_show,/*属性读函数*/
	.store = kobject_test_store,/*属性写函数*/
};

struct kobj_type ktype = 
{
	.release = kobject_test_release,/*释放函数*/
	.sysfs_ops=&obj_test_sysops,/*属性的操作函数*/
	.default_attrs=def_attrs,/*默认属性*/
};
void kobject_test_release(struct kobject *kobject)
{
	printk("kobject_test: kobject_test_release() .\n");/*这只是一个测试例子，实际的代码要复杂很多*/
}
/*该函数用来读取一个属性的名字*/
ssize_t kobject_test_show(struct kobject *kobject, struct attribute *attr,char *buf)
{
	printk("call kobject_test_show().\n");/*调试信息*/
	printk("attrname:%s.\n", attr->name);/*打印属性的名字*/
	sprintf(buf,"%s\n",attr->name);/*将属性名字存放在buf中，返回用户空间*/
	return strlen(attr->name)+2;
}
/*该函数用来写入一个属性的值*/
ssize_t kobject_test_store(struct kobject *kobject,struct attribute *attr,const char *buf, size_t count)
{
	printk("call kobject_test_store().\n");/*调试信息*/
	printk("write: %s\n",buf);/*输出要存入的信息*/
	strcpy(attr->name,buf);
	return count;
}

struct kobject kobj;/*要添加的kobject结构*/
static int kobject_test_init()
{
	printk("kboject test_init().\n");
	kobject_init_and_add(&kobj,&ktype,NULL,"kobject_test");/*初始化并添加kobject到内核中*/
	return 0;
}
static int kobject_test_exit()
{
	printk("kobject test exit.\n");
	kobject_del(&kobj);/*删除kobject*/
	return 0;
}
module_init(kobject_test_init);
module_exit(kobject_test_exit);

MODULE_AUTHOR("Zheng Qiang");
MODULE_LICENSE("Dual BSD/GPL");
