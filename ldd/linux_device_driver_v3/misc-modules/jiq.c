/*
 * jiq.c -- the just-in-queue module
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
 * $Id: jiq.c,v 1.7 2004/09/26 07:02:43 gregkh Exp $
 */
 
#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>     /* everything... */
#include <linux/proc_fs.h>
#include <linux/errno.h>  /* error codes */
#include <linux/workqueue.h>
#include <linux/preempt.h>
#include <linux/interrupt.h> /* tasklets */

MODULE_LICENSE("Dual BSD/GPL");

/*
 * The delay for the delayed workqueue timer file.
 */
static long delay = 1;
module_param(delay, long, 0);


/*
 * This module is a silly one: it only embeds short code fragments
 * that show how enqueued tasks `feel' the environment
 */

#define LIMIT	(PAGE_SIZE-128)	/* don't print any more after this size */

/*
 * Print information about the current environment. This is called from
 * within the task queues. If the limit is reched, awake the reading
 * process.
 */
static DECLARE_WAIT_QUEUE_HEAD (jiq_wait);


static struct work_struct jiq_work;



/*
 * Keep track of info we need between task queue runs.
 */
static struct clientdata {
	int len;
	char *buf;
	unsigned long jiffies;
	long delay;
} jiq_data;

#define SCHEDULER_QUEUE ((task_queue *) 1)



static void jiq_print_tasklet(unsigned long);
static DECLARE_TASKLET(jiq_tasklet, jiq_print_tasklet, (unsigned long)&jiq_data);


/*
 * Do the printing; return non-zero if the task should be rescheduled.
 */
static int jiq_print(void *ptr)
{
	struct clientdata *data = ptr;
	int len = data->len;
	char *buf = data->buf;
	unsigned long j = jiffies;

	if (len > LIMIT) { 
		wake_up_interruptible(&jiq_wait);
		return 0;
	}

	if (len == 0)
		len = sprintf(buf,"    time  delta preempt   pid cpu command\n");
	else
		len =0;

  	/* intr_count is only exported since 1.3.5, but 1.99.4 is needed anyways */
	len += sprintf(buf+len, "%9li  %4li     %3i %5i %3i %s\n",
			j, j - data->jiffies,
			preempt_count(), current->pid, smp_processor_id(),
			current->comm);

	data->len += len;
	data->buf += len;
	data->jiffies = j;
	return 1;
}


/*
 * Call jiq_print from a work queue
 */
static void jiq_print_wq(void *ptr)
{
	struct clientdata *data = (struct clientdata *) ptr;
    
	if (! jiq_print (ptr))
		return;
    
	if (data->delay)
		schedule_delayed_work(&jiq_work, data->delay);
	else
		schedule_work(&jiq_work);
}



static int jiq_read_wq(char *buf, char **start, off_t offset,
                   int len, int *eof, void *data)
{
	DEFINE_WAIT(wait);
	
	jiq_data.len = 0;                /* nothing printed, yet */
	jiq_data.buf = buf;              /* print in this place */
	jiq_data.jiffies = jiffies;      /* initial time */
	jiq_data.delay = 0;
    
	prepare_to_wait(&jiq_wait, &wait, TASK_INTERRUPTIBLE);
	schedule_work(&jiq_work);
	schedule();
	finish_wait(&jiq_wait, &wait);

	*eof = 1;
	return jiq_data.len;
}


static int jiq_read_wq_delayed(char *buf, char **start, off_t offset,
                   int len, int *eof, void *data)
{
	DEFINE_WAIT(wait);
	
	jiq_data.len = 0;                /* nothing printed, yet */
	jiq_data.buf = buf;              /* print in this place */
	jiq_data.jiffies = jiffies;      /* initial time */
	jiq_data.delay = delay;
    
	prepare_to_wait(&jiq_wait, &wait, TASK_INTERRUPTIBLE);
	schedule_delayed_work(&jiq_work, delay);
	schedule();
	finish_wait(&jiq_wait, &wait);

	*eof = 1;
	return jiq_data.len;
}




/*
 * Call jiq_print from a tasklet
 */
static void jiq_print_tasklet(unsigned long ptr)
{
	if (jiq_print ((void *) ptr))
		tasklet_schedule (&jiq_tasklet);
}



static int jiq_read_tasklet(char *buf, char **start, off_t offset, int len,
                int *eof, void *data)
{
	jiq_data.len = 0;                /* nothing printed, yet */
	jiq_data.buf = buf;              /* print in this place */
	jiq_data.jiffies = jiffies;      /* initial time */

	tasklet_schedule(&jiq_tasklet);
	interruptible_sleep_on(&jiq_wait);    /* sleep till completion */

	*eof = 1;
	return jiq_data.len;
}




/*
 * This one, instead, tests out the timers.
 */

static struct timer_list jiq_timer;

static void jiq_timedout(unsigned long ptr)
{
	jiq_print((void *)ptr);            /* print a line */
	wake_up_interruptible(&jiq_wait);  /* awake the process */
}


static int jiq_read_run_timer(char *buf, char **start, off_t offset,
                   int len, int *eof, void *data)
{

	jiq_data.len = 0;           /* prepare the argument for jiq_print() */
	jiq_data.buf = buf;
	jiq_data.jiffies = jiffies;

	init_timer(&jiq_timer);              /* init the timer structure */
	jiq_timer.function = jiq_timedout;
	jiq_timer.data = (unsigned long)&jiq_data;
	jiq_timer.expires = jiffies + HZ; /* one second */

	jiq_print(&jiq_data);   /* print and go to sleep */
	add_timer(&jiq_timer);
	interruptible_sleep_on(&jiq_wait);  /* RACE */
	del_timer_sync(&jiq_timer);  /* in case a signal woke us up */
    
	*eof = 1;
	return jiq_data.len;
}



/*
 * the init/clean material
 */

static int jiq_init(void)
{

	/* this line is in jiq_init() */
	INIT_WORK(&jiq_work, jiq_print_wq, &jiq_data);

	create_proc_read_entry("jiqwq", 0, NULL, jiq_read_wq, NULL);
	create_proc_read_entry("jiqwqdelay", 0, NULL, jiq_read_wq_delayed, NULL);
	create_proc_read_entry("jitimer", 0, NULL, jiq_read_run_timer, NULL);
	create_proc_read_entry("jiqtasklet", 0, NULL, jiq_read_tasklet, NULL);

	return 0; /* succeed */
}

static void jiq_cleanup(void)
{
	remove_proc_entry("jiqwq", NULL);
	remove_proc_entry("jiqwqdelay", NULL);
	remove_proc_entry("jitimer", NULL);
	remove_proc_entry("jiqtasklet", NULL);
}


module_init(jiq_init);
module_exit(jiq_cleanup);
