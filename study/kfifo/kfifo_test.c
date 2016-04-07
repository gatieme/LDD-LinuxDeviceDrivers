/*************************************************************************
    > File Name: 1.c
    > Author: gatieme
    > Created Time: Thu 07 Apr 2016 02:44:43 PM CST
 ************************************************************************/

#include <linux/init.h>
#include <linux/module.h>


static int kfifo_init()
{
    unsigned int i;
    unsigned int val;
    ret =
    /*  */
    for(i = 0; i < 32; i++)
    {
        kfifo_in(fifo, &i, sizeof(i));


    }
}


static void kfifo_exit( )
{

}

module_init(list_kfifo_init);
module_exit(list_kfifo_exit);

MODULE_AUTHOR("gatieme");
MODULE_LICENSE("GPL");





