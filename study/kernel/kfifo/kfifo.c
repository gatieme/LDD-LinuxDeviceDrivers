#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zl");
MODULE_VERSION("V1.0");
MODULE_DESCRIPTION("kfifo test");

struct KfifoNode
{
    unsigned int num;
    char *string;
};

char *array[] =
{
    "abcdefg",
    "gfedcba",
    "aaaaa",
    "bbbb",
    "ccc",
    "dd",
    "e",
    "12345",
    "1234",
    "123",
    "12",
    "1",
    "1111",
};

#define TAB_SIZE(array) (sizeof(array)/sizeof(array[0]))

static struct __kfifo *pkfifoSpace = NULL;
static spinlock_t fifoLock;

static void kfifo_check(char* str, int line, struct kfifo* pkfifo)
{
    if(pkfifo != NULL)
    {
        printk("[%s-%d]: pkfifo->size = %d\t pkfifo->in = %d\t pkfifo->out = %d\t \n",
                str, line, kfifo_size(pkfifo), pkfifo->in, pkfifo->out);
    }
}

static int __init mykfifo_init(void)
{
    int i;
    struct KfifoNode *pstNode;

    pkfifoSpace = kfifo_alloc((sizeof(struct KfifoNode) << 4), GFP_KERNEL, &fifoLock);
    if (pkfifoSpace == NULL)
    {
        printk("kfifo_alloc failed !\n");
        return -EFAULT;
    }
    spin_lock_init(&fifoLock); //Initial fifo spinlock
    pstNode = kzalloc(sizeof(struct KfifoNode), GFP_KERNEL);

    /****************************************************************/
    printk("*****************************************************\n");
    kfifo_check((char *)__func__, __LINE__, pkfifoSpace);
    for(i = 0; i < TAB_SIZE(array); i++)
    {
        pstNode->num = i;
        pstNode->string = (char *)array[i];
        kfifo_put(pkfifoSpace, (unsigned char *)pstNode, sizeof(struct KfifoNode)); //将数据写入缓冲区
        kfifo_check((char *)__func__, __LINE__, pkfifoSpace);
        printk("[%s-%d]:Num is: %d, Message is: %s\n", __func__, __LINE__, pstNode->num, pstNode->string);
    }
    /***************************************************************/

    printk("-----------------------------------------------------\n");
    if(!kfifo_len(pkfifoSpace))
    {
        printk("[%s-%d]: kfifo_len return 0, test failed !!! \n", __func__, __LINE__);
        kfifo_reset(pkfifoSpace);
        kfifo_free(pkfifoSpace);

        return -1;
    }

    for(i = 0; i < TAB_SIZE(array); i++)
    {
        kfifo_get(pkfifoSpace, (unsigned char *)pstNode, sizeof(struct KfifoNode));
        kfifo_check((char *)__func__, __LINE__, pkfifoSpace);
        printk("[%s-%d]: Num is: %d, fifoMessage is: %s\n", __func__, __LINE__, pstNode->num, pstNode->string);
    }

    /***************************************************************/
    printk("-----------------------------------------------------\n");
    kfifo_check((char *)__func__, __LINE__, pkfifoSpace);

    kfree(pstNode);
    kfifo_reset(pkfifoSpace);
    kfifo_free(pkfifoSpace);

    return 0;

}

static void __exit mykfifo_exit(void)
{
    printk("exit !\n");
}

module_init(mykfifo_init);
module_exit(mykfifo_exit);
