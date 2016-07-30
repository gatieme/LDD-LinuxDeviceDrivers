/*
 * A version of the "short" driver which drives a parallel printer directly,
 * with a lot of simplifying assumptions.
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 * $Id: shortprint.c,v 1.4 2004/09/26 08:01:04 gregkh Exp $
 */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/sched.h>
#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>	  /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/delay.h>  /* udelay */
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/poll.h>

#include <asm/io.h>
#include <asm/semaphore.h>
#include <asm/atomic.h>

#include "shortprint.h"

#define SHORTP_NR_PORTS 3

/*
 * all of the parameters have no "shortp_" prefix, to save typing when
 * specifying them at load time
 */
static int major = 0; /* dynamic by default */
module_param(major, int, 0);

/* default is the first printer port on PC's. "shortp_base" is there too
   because it's what we want to use in the code */
static unsigned long base = 0x378;
unsigned long shortp_base = 0;
module_param(base, long, 0);

/* The interrupt line is undefined by default. "shortp_irq" is as above */
static int irq = -1;
static int shortp_irq = -1;
module_param(irq, int, 0);

/* Microsecond delay around strobe. */
static int delay = 0;
static int shortp_delay;
module_param(delay, int, 0);

MODULE_AUTHOR ("Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");

/*
 * Forwards.
 */
static void shortp_cleanup(void);
static void shortp_timeout(unsigned long unused);

/*
 * Input is managed through a simple circular buffer which, among other things,
 * is allowed to overrun if the reader isn't fast enough.  That makes life simple
 * on the "read" interrupt side, where we don't want to block.
 */
static unsigned long shortp_in_buffer = 0;
static unsigned long volatile shortp_in_head;
static volatile unsigned long shortp_in_tail;
DECLARE_WAIT_QUEUE_HEAD(shortp_in_queue);
static struct timeval shortp_tv;  /* When the interrupt happened. */

/*
 * Atomicly increment an index into shortp_in_buffer
 */
static inline void shortp_incr_bp(volatile unsigned long *index, int delta)
{
	unsigned long new = *index + delta;
	barrier ();  /* Don't optimize these two together */
	*index = (new >= (shortp_in_buffer + PAGE_SIZE)) ? shortp_in_buffer : new;
}


/*
 * On the write side we have to be more careful, since we don't want to drop
 * data.  The semaphore is used to serialize write-side access to the buffer;
 * there is only one consumer, so read-side access is unregulated.  The
 * wait queue will be awakened when space becomes available in the buffer.
 */
static unsigned char *shortp_out_buffer = NULL;
static volatile unsigned char *shortp_out_head, *shortp_out_tail;
static struct semaphore shortp_out_sem;
static DECLARE_WAIT_QUEUE_HEAD(shortp_out_queue);

/*
 * Feeding the output queue to the device is handled by way of a
 * workqueue.
 */
static void shortp_do_work(void *);
static DECLARE_WORK(shortp_work, shortp_do_work, NULL);
static struct workqueue_struct *shortp_workqueue;

/*
 * Available space in the output buffer; should be called with the semaphore
 * held.  Returns contiguous space, so caller need not worry about wraps.
 */
static inline int shortp_out_space(void)
{
	if (shortp_out_head >= shortp_out_tail) {
		int space = PAGE_SIZE - (shortp_out_head - shortp_out_buffer);
		return (shortp_out_tail == shortp_out_buffer) ? space - 1 : space;
	} else
		return (shortp_out_tail - shortp_out_head) - 1;
}

static inline void shortp_incr_out_bp(volatile unsigned char **bp, int incr)
{
	unsigned char *new = (unsigned char *) *bp + incr;
	if (new >= (shortp_out_buffer + PAGE_SIZE))
		new -= PAGE_SIZE;
	*bp = new;
}

/*
 * The output "process" is controlled by a spin lock; decisions on
 * shortp_output_active or manipulation of shortp_out_tail require
 * that this lock be held.
 */
static spinlock_t shortp_out_lock;
volatile static int shortp_output_active;
DECLARE_WAIT_QUEUE_HEAD(shortp_empty_queue); /* waked when queue empties */

/*
 * When output is active, the timer is too, in case we miss interrupts.	 Hold
 * shortp_out_lock if you mess with the timer.
 */
static struct timer_list shortp_timer;
#define TIMEOUT 5*HZ  /* Wait a long time */


/*
 * Open the device.
 */
static int shortp_open(struct inode *inode, struct file *filp)
{
	return 0;
}


static int shortp_release(struct inode *inode, struct file *filp)
{
	/* Wait for any pending output to complete */
	wait_event_interruptible(shortp_empty_queue, shortp_output_active==0);

	return 0;
}



static unsigned int shortp_poll(struct file *filp, poll_table *wait)
{
    return POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM;
}



/*
 * The read routine, which doesn't return data from the device; instead, it
 * returns timing information just like the "short" device.
 */
static ssize_t shortp_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	int count0;
	DEFINE_WAIT(wait);

	while (shortp_in_head == shortp_in_tail) {
		prepare_to_wait(&shortp_in_queue, &wait, TASK_INTERRUPTIBLE);
		if (shortp_in_head == shortp_in_tail)
			schedule();
		finish_wait(&shortp_in_queue, &wait);
		if (signal_pending (current))  /* a signal arrived */
			return -ERESTARTSYS; /* tell the fs layer to handle it */
	}

	/* count0 is the number of readable data bytes */
	count0 = shortp_in_head - shortp_in_tail;
	if (count0 < 0) /* wrapped */
		count0 = shortp_in_buffer + PAGE_SIZE - shortp_in_tail;
	if (count0 < count)
		count = count0;

	if (copy_to_user(buf, (char *)shortp_in_tail, count))
		return -EFAULT;
	shortp_incr_bp(&shortp_in_tail, count);
	return count;
}


/*
 * Wait for the printer to be ready; this can sleep.
 */
static void shortp_wait(void)
{
	if ((inb(shortp_base + SP_STATUS) & SP_SR_BUSY) == 0) {
		printk(KERN_INFO "shortprint: waiting for printer busy\n");
		printk(KERN_INFO "Status is 0x%x\n", inb(shortp_base + SP_STATUS));
		while ((inb(shortp_base + SP_STATUS) & SP_SR_BUSY) == 0) {
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(10*HZ); 
		}
	}
}


/*
 * Write the next character from the buffer.  There should *be* a next
 * character...	 The spinlock should be held when this routine is called.
 */
static void shortp_do_write(void)
{
	unsigned char cr = inb(shortp_base + SP_CONTROL);

	/* Something happened; reset the timer */
	mod_timer(&shortp_timer, jiffies + TIMEOUT);

	/* Strobe a byte out to the device */
	outb_p(*shortp_out_tail, shortp_base+SP_DATA);
	shortp_incr_out_bp(&shortp_out_tail, 1);
	if (shortp_delay)
		udelay(shortp_delay);
	outb_p(cr | SP_CR_STROBE, shortp_base+SP_CONTROL);
	if (shortp_delay)
		udelay(shortp_delay);
	outb_p(cr & ~SP_CR_STROBE, shortp_base+SP_CONTROL);
}


/*
 * Start output; call under lock.
 */
static void shortp_start_output(void)
{
	if (shortp_output_active) /* Should never happen */
		return;

	/* Set up our 'missed interrupt' timer */
	shortp_output_active = 1;
	shortp_timer.expires = jiffies + TIMEOUT;
	add_timer(&shortp_timer);

	/*  And get the process going. */
	queue_work(shortp_workqueue, &shortp_work);
}


/*
 * Write to the device.
 */
static ssize_t shortp_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	int space, written = 0;
	unsigned long flags;
	/*
	 * Take and hold the semaphore for the entire duration of the operation.  The
	 * consumer side ignores it, and it will keep other data from interleaving
	 * with ours.
	 */
	if (down_interruptible(&shortp_out_sem))
		return -ERESTARTSYS;
	/*
	 * Out with the data.
	 */
	while (written < count) {
		/* Hang out until some buffer space is available. */
		space = shortp_out_space();
		if (space <= 0) {
			if (wait_event_interruptible(shortp_out_queue,
					    (space = shortp_out_space()) > 0))
				goto out;
		}

		/* Move data into the buffer. */
		if ((space + written) > count)
			space = count - written;
		if (copy_from_user((char *) shortp_out_head, buf, space)) {
			up(&shortp_out_sem);
			return -EFAULT;
		}
		shortp_incr_out_bp(&shortp_out_head, space);
		buf += space;
		written += space;

		/* If no output is active, make it active. */
		spin_lock_irqsave(&shortp_out_lock, flags);
		if (! shortp_output_active)
			shortp_start_output();
		spin_unlock_irqrestore(&shortp_out_lock, flags);
	}

out:
	*f_pos += written;
	up(&shortp_out_sem);
	return written;
}


/*
 * The bottom-half handler.
 */


static void shortp_do_work(void *unused)
{
	int written;
	unsigned long flags;

	/* Wait until the device is ready */
	shortp_wait();
	
	spin_lock_irqsave(&shortp_out_lock, flags);

	/* Have we written everything? */
	if (shortp_out_head == shortp_out_tail) { /* empty */
		shortp_output_active = 0;
		wake_up_interruptible(&shortp_empty_queue);
		del_timer(&shortp_timer);  
	}
	/* Nope, write another byte */
	else
		shortp_do_write();

	/* If somebody's waiting, maybe wake them up. */
	if (((PAGE_SIZE + shortp_out_tail - shortp_out_head) % PAGE_SIZE) > SP_MIN_SPACE) {
		wake_up_interruptible(&shortp_out_queue);
	}
	spin_unlock_irqrestore(&shortp_out_lock, flags);

	/* Handle the "read" side operation */
	written = sprintf((char *)shortp_in_head, "%08u.%06u\n",
			(int)(shortp_tv.tv_sec % 100000000),
			(int)(shortp_tv.tv_usec));
	shortp_incr_bp(&shortp_in_head, written);
	wake_up_interruptible(&shortp_in_queue); /* awake any reading process */
}


/*
 * The top-half interrupt handler.
 */
static irqreturn_t shortp_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	if (! shortp_output_active) 
		return IRQ_NONE;

	/* Remember the time, and farm off the rest to the workqueue function */ 
	do_gettimeofday(&shortp_tv);
	queue_work(shortp_workqueue, &shortp_work);
	return IRQ_HANDLED;
}

/*
 * Interrupt timeouts.	Just because we got a timeout doesn't mean that
 * things have gone wrong, however; printers can spend an awful long time
 * just thinking about things.
 */
static void shortp_timeout(unsigned long unused)
{
	unsigned long flags;
	unsigned char status;
   
	if (! shortp_output_active)
		return;
	spin_lock_irqsave(&shortp_out_lock, flags);
	status = inb(shortp_base + SP_STATUS);

	/* If the printer is still busy we just reset the timer */
	if ((status & SP_SR_BUSY) == 0 || (status & SP_SR_ACK)) {
		shortp_timer.expires = jiffies + TIMEOUT;
		add_timer(&shortp_timer);
		spin_unlock_irqrestore(&shortp_out_lock, flags);
		return;
	}

	/* Otherwise we must have dropped an interrupt. */
	spin_unlock_irqrestore(&shortp_out_lock, flags);
	shortp_interrupt(shortp_irq, NULL, NULL);
}
    




static struct file_operations shortp_fops = {
	.read =	   shortp_read,
	.write =   shortp_write,
	.open =	   shortp_open,
	.release = shortp_release,
	.poll =	   shortp_poll,
	.owner	 = THIS_MODULE
};




/*
 * Module initialization
 */

static int shortp_init(void)
{
	int result;

	/*
	 * first, sort out the base/shortp_base ambiguity: we'd better
	 * use shortp_base in the code, for clarity, but allow setting
	 * just "base" at load time. Same for "irq".
	 */
	shortp_base = base;
	shortp_irq = irq;
	shortp_delay = delay;

	/* Get our needed resources. */
	if (! request_region(shortp_base, SHORTP_NR_PORTS, "shortprint")) {
		printk(KERN_INFO "shortprint: can't get I/O port address 0x%lx\n",
				shortp_base);
		return -ENODEV;
	}	

	/* Register the device */
	result = register_chrdev(major, "shortprint", &shortp_fops);
	if (result < 0) {
		printk(KERN_INFO "shortp: can't get major number\n");
		release_region(shortp_base, SHORTP_NR_PORTS);
		return result;
	}
	if (major == 0)
		major = result; /* dynamic */

	/* Initialize the input buffer. */
	shortp_in_buffer = __get_free_pages(GFP_KERNEL, 0); /* never fails */
	shortp_in_head = shortp_in_tail = shortp_in_buffer;

	/* And the output buffer. */
	shortp_out_buffer = (unsigned char *) __get_free_pages(GFP_KERNEL, 0);
	shortp_out_head = shortp_out_tail = shortp_out_buffer;
	sema_init(&shortp_out_sem, 1);
    
	/* And the output info */
	shortp_output_active = 0;
	spin_lock_init(&shortp_out_lock);
	init_timer(&shortp_timer);
	shortp_timer.function = shortp_timeout;
	shortp_timer.data = 0;
    
	/* Set up our workqueue. */
	shortp_workqueue = create_singlethread_workqueue("shortprint");

	/* If no IRQ was explicitly requested, pick a default */
	if (shortp_irq < 0)
		switch(shortp_base) {
		    case 0x378: shortp_irq = 7; break;
		    case 0x278: shortp_irq = 2; break;
		    case 0x3bc: shortp_irq = 5; break;
		}

	/* Request the IRQ */
	result = request_irq(shortp_irq, shortp_interrupt, 0, "shortprint", NULL);
	if (result) {
		printk(KERN_INFO "shortprint: can't get assigned irq %i\n",
				shortp_irq);
		shortp_irq = -1;
		shortp_cleanup ();
		return result;
	}

	/* Initialize the control register, turning on interrupts. */
	outb(SP_CR_IRQ | SP_CR_SELECT | SP_CR_INIT, shortp_base + SP_CONTROL);

	return 0;
}

static void shortp_cleanup(void)
{
	/* Return the IRQ if we have one */
	if (shortp_irq >= 0) {
		outb(0x0, shortp_base + SP_CONTROL);   /* disable the interrupt */
		free_irq(shortp_irq, NULL);
	}

	/* All done with the device */
	unregister_chrdev(major, "shortprint");
	release_region(shortp_base,SHORTP_NR_PORTS);

	/* Don't leave any timers floating around.  Note that any active output
	   is effectively stopped by turning off the interrupt */
	if (shortp_output_active)
		del_timer_sync (&shortp_timer);
	flush_workqueue(shortp_workqueue);
	destroy_workqueue(shortp_workqueue);

	if (shortp_in_buffer)
		free_page(shortp_in_buffer);
	if (shortp_out_buffer)
		free_page((unsigned long) shortp_out_buffer);
}

module_init(shortp_init);
module_exit(shortp_cleanup);
