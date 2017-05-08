/*
 * File Name: Hello.c
 *
 * Descriptions: 
 *		This is the first module.
 *
 * Author: 
 *		Zheng Qiang
 * Kernel Version: 2.6.29
 *
 * Update:
 * 		-	2009.12.19	Zheng Qiang	 Creat this file
 */                                                   
#include <linux/init.h>				/* 定义了一些相关的宏 */
#include <linux/module.h>			/* 定义了模块需要的*/

static int hello_init(void)
{
	printk(KERN_ALERT "Hello, world\n");	/* 打印hello World */
	return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT "Goodbye, world\n");	/* 打印Goodbye,world */
}

module_init(hello_init);   /* 指定模块加载函数 */
module_exit(hello_exit);   /* 指定模块卸载函数 */
MODULE_LICENSE("Dual BSD/GPL");