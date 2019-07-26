#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/irq_work.h>

DEFINE_SPINLOCK(lock);

static void scheduling(struct irq_work *work)
{
	schedule();
}


DEFINE_IRQ_WORK(irq_work, scheduling);
static int __init hello_init(void)
{
	irq_work_queue(&irq_work);

	return 0;
}

module_init(hello_init);

static void __exit hello_exit(void)
{
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
