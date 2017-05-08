/*
* File Name: test.c
*
* Descriptions: 
*		Test the add and sub.
*
* Author: 
*		Zheng Qiang
* Kernel Version: 2.6.29
*
* Update:
* 		-	2009.12.20	Zheng Qiang	 Creat this file
*/

#include <linux/init.h>
#include <linux/module.h>  
#include "add_sub.h"/* 不要使用<>包含文件 否则找不到该文件 */  

static long a = 1;  
static long b = 1;  
static int AddOrSub =1;
  
static int test_init(void)  
{
		long result=0;
        printk(KERN_ALERT "test init\n");
		if(1==AddOrSub)
		{
			result=add_integer(a, b);
		}
		else
		{
			result=sub_integer(a, b);  
		}
		printk(KERN_ALERT "The %s result is %ld",AddOrSub==1?"Add":"Sub",result);
        return 0;  
}  
   
static void test_exit(void)
{  
        printk(KERN_ALERT "test exit\n");
}  
   
module_init(test_init);
module_exit(test_exit);
module_param(a, long, S_IRUGO);  
module_param(b, long, S_IRUGO);  
module_param(AddOrSub, int, S_IRUGO); 
/* 描述信息 */                               
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Zheng Qiang");
MODULE_DESCRIPTION("A Module for testing module params and EXPORT_SYMBOL");
MODULE_VERSION("V1.0");