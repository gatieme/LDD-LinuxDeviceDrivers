
#include
 <linux/init.h>

#include
 <linux/time.h>

#include
 <linux/errno.h>

#include
 <linux/module.h>

#include
 <linux/sched.h>

#include
 <linux/kthread.h>

#include
 <linux/cpumask.h>

 

#define
 sleep_millisecs 1000*60

 

static

int 
thread(void

*arg)

{

    long

ns1, ns2, delta;

    unsigned
int

cpu;

    struct

timespec ts1, ts2;

 

    cpu
 = *((unsigned int

*)arg);

 

    printk(KERN_INFO
"###
 [thread/%d] test start \n",
 cpu);

 

    while

(!kthread_should_stop()) {

                /*

                 *
 Do What you want

                 */

        schedule_timeout_interruptible(

                msecs_to_jiffies(1));

    }

 

    printk(KERN_INFO
"###
 [thread/%d] test end \n",
 cpu);

 

    return

0;

}

 

static

int 
__init XXXX(void)

{

    int

cpu;

    unsigned
int

cpu_count = num_online_cpus();

    unsigned
int

parameter[cpu_count];

    struct

task_struct *t_thread[cpu_count];

 

    for_each_present_cpu(cpu){

        parameter[cpu]
 = cpu;

 

        t_thread[cpu]
 = kthread_create(thread,
 (void

*) (parameter+cpu), "thread/%d",
 cpu);

 

        if

(IS_ERR(t_thread[cpu])) {

            printk(KERN_ERR
"[thread/%d]:
 creating kthread failed\n",
 cpu);

 

            goto

out;

        }

 

        kthread_bind(t_thread[cpu],
 cpu);

        wake_up_process(t_thread[cpu]);

    }

 

    schedule_timeout_interruptible(

            msecs_to_jiffies(sleep_millisecs));

 

    for

(cpu = 0; cpu < cpu_count; cpu++) {

        kthread_stop(t_thread[cpu]);

    }

 

out:

    return

0;

}

 

static

void 
__exit XXXX_exit(void)

{

}

 

module_init(XXXX);

module_exit(XXXX_exit);

 

MODULE_LICENSE("GPL");

MODULE_AUTHOR("bluezd");

MODULE_DESCRIPTION("Kernel
 study");