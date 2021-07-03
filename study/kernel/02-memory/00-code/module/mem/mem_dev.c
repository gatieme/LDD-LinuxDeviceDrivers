/*************************************************************************
    > File Name: process.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: 2016年04月01日 星期五 21时09分29秒
 ************************************************************************/



#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/sched.h>



//#define METHOD 2
static unsigned int METHOD = 1;
module_param(METHOD, uint,0400);

long readPhysicsAddress(unsigned long pa, int *pStatus);

long writePhysicsAddress(unsigned long pa, unsigned long data, int *pStatus);



static int mem_dev_init(void)
{
    readPhysicsAddress(0x12345, NULL);
    return 0;
}

static void mem_dev_exit(void)
{
    printk( KERN_ALERT "GOOD BYE!!\n");
}



/**  add by gatieme  */
/*
 *
 */
long readPhysicsAddress(unsigned long pa, int *pStatus)
{
    long data = -1;
    data = *(long *)pa;
    printk(KERN_INFO "physics address : 0x%lx, data : 0x%lx", pa, data);

    return data;
}

long writePhysicsAddress(unsigned long pa, unsigned long data, int *pStatus)
{
    long oldData = -1;

    oldData = *(long *)pa;
    printk(KERN_INFO "physics address : 0x%lx, old data : 0x%lx", pa, *(long *)pa);

    *(long *)pa = data;
    printk(KERN_INFO "physics address : 0x%lx, new data : 0x%lx", pa, *(long *)pa);

    return oldData;
}

module_init(mem_dev_init);
module_exit(mem_dev_exit);

MODULE_AUTHOR("gatieme");
MODULE_LICENSE("GPL");
