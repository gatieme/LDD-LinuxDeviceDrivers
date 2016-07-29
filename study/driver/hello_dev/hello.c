#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <asm/uaccess.h>


static ssize_t hello_read(
        struct file * file,
        char * buf,
        size_t count,
        loff_t *ppos)
{
    char *hello_str = "Hello, world!\n";

    int len = strlen(hello_str); /* Don't include the null byte. */

    /*     * We only support reading the whole string at once.     */
    if (count < len)
        return -EINVAL;

    /*
    * If file position is non-zero, then assume the string has
    * been read and indicate there is no more data to be read.
    */
    if (*ppos != 0)
    {
        return 0;
    }

    /*
    * Besides copying the string to the user provided buffer,
    * this function also checks that the user has permission to
    * write to the buffer, that it is mapped, etc.
    */
    if (copy_to_user(buf, hello_str, len))
    {
        return -EINVAL;
    }

    /*
    * Tell the user how much data we wrote.
    */
    *ppos = len;
    return len;
}


static const struct file_operations hello_fops =
{
    .owner        = THIS_MODULE,
    .read        = hello_read,
};


static struct miscdevice hello_dev =
{
    /*
    * We don't care what minor number we end up with, so tell the
    * kernel to just pick one.
    */
    MISC_DYNAMIC_MINOR,
    /*
    * Name ourselves /dev/hello.
    */
    "hello",
    /*
    * What functions to call when a program performs file
    * operations on the device.
    */
    &hello_fops,
};


static int __init hello_init(void)
{
    int ret;
    /*
    * Create the "hello" device in the /sys/class/misc directory.
    * Udev will automatically create the /dev/hello device using
    * the default rules.
    */
    ret = misc_register(&hello_dev);
    if (ret != 0)
    {
        printk(KERN_ERR "Unable to register \"Hello, world!\" misc devicen");
    }
    else
    {
        printk(KERN_INFO "register \"Hello World!\" misc devicen...\n");
    }

    return ret;
}
module_init(hello_init);


static void __exit hello_exit(void)
{
    misc_deregister(&hello_dev);
    printk(KERN_INFO "deregister \"Hello World!\" misc devicen...\n");
}
module_exit(hello_exit);


/*  Driver Information  */
#define DRIVER_VERSION  "1.0.0"
#define DRIVER_AUTHOR   "Gatieme @ AderStep Inc..."
#define DRIVER_DESC     "Linux input module for Elo MultiTouch(MT) devices"
#define DRIVER_LICENSE  "Dual BSD/GPL"

/*  Kernel Module Information   */
MODULE_VERSION(DRIVER_VERSION);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);
