/*
 * File Name: multi_div_test.c
 *
 * Descriptions: 
 *		Test the multi and div.
 *
 * Author: 
 *		Zheng Qiang
 * Kernel Version: 2.6.29
 *
 * Update:
 * 		-	2009.12.20	Zheng Qiang	 Creat this file
 */                                                   
#include <linux/init.h>				/* 定义了一些相关的宏 */
#include <linux/module.h>			/* 定义了模块需要的*/
#include <multi_div.h>

static long a = 1;  
static long b = 1;  
  
static int init_test(void)
{
	long c=0;
	c=multiply_test(a,b);
	printk(KERN_ALERT "c=%ld*%ld=%ld",a,b,c);	/*  */
	c=divide_test(a,b);
	printk(KERN_ALERT "c=%ld/%ld=%ld",a,b,c);
	return 0;
}

static void exit_test(void)
{
	printk(KERN_ALERT "exit modules");	/* 打印Goodbye,world */
}

module_init(init_test);   /* 指定模块加载函数 */
module_exit(exit_test);   /* 指定模块卸载函数 */
module_param(a, long, S_IRUGO);  
module_param(b, long, S_IRUGO);  
/* 描述信息 */                               
MODULE_LICENSE("Dual BSD/GPL");      
MODULE_AUTHOR("Zheng Qiang");
MODULE_DESCRIPTION("A Module for testing module params and EXPORT_SYMBOL");
MODULE_VERSION("V1.0");
