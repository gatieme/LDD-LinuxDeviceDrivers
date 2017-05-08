#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
 
#include <asm/irq.h>
#include <asm/io.h>

#define BUTTON_IRQ 123

static void button_interrupt(int irq, void *dummy, struct pt_regs *fp)
{
        input_report_key(&button_dev, BTN_1, inb(BUTTON_PORT) & 1);
        input_sync(&button_dev);
}
 
static int __init button_init(void)
{
        if (request_irq(BUTTON_IRQ, button_interrupt, 0, "button", NULL)) {
                printk(KERN_ERR "button.c: Can't allocate irq %d\n", button_irq);
                return -EBUSY;
        }
 
        button_dev.evbit[0] = BIT(EV_KEY);
        button_dev.keybit[LONG(BTN_0)] = BIT(BTN_0);
 
        input_register_device(&button_dev);
}
 
static void __exit button_exit(void)
{
        input_unregister_device(&button_dev);
        free_irq(BUTTON_IRQ, button_interrupt);
}
 
module_init(button_init);
module_exit(button_exit);
