#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

MODULE_LICENSE("Dual BSD/GPL");


static void print_loglevel(void)
{
    printk("console_loglevel         = %d\n", console_loglevel);
    printk("default_message_loglevel = %d\n", default_message_loglevel);
    printk("minimum_console_loglevel = %d\n", minimum_console_loglevel);
    printk("default_console_loglevel = %d\n", default_console_loglevel);
}

static void print_loginfo(void)
{

    printk(KERN_EMERG   "EMERG  = %s\n", KERN_EMERG);
    printk(KERN_ALERT   "ALERT  = %s\n", KERN_ALERT);
    printk(KERN_CRIT    "CRIT   = %s\n", KERN_CRIT);
    printk(KERN_ERR     "ERR    = %s\n", KERN_ERR);
    printk(KERN_WARNING "WARNING= %s\n", KERN_WARNING);
    printk(KERN_NOTICE  "NOTICE = %s\n", KERN_NOTICE);
    printk(KERN_INFO    "INFO   = %s\n", KERN_INFO);
    printk(KERN_DEBUG   "DEBUG  = %s\n", KERN_DEBUG);
}


static int book_init(void)
{
    printk("Book module init\n");
    //print_loglevel();
    print_loginfo();
    return 0;
}

static void book_exit(void)
{
    printk(KERN_ALERT "Book module exit\n");
}


module_init(book_init);
module_exit(book_exit);
