/*
* File Name: add_sub.c
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
#include "add_sub.h"

long add_integer(long a, long b)
{
        return a+b;
}

long sub_integer(long a, long b)
{
        return a-b;
}

EXPORT_SYMBOL(add_integer);
EXPORT_SYMBOL(sub_integer);
MODULE_LICENSE("Dual BSD/GPL");