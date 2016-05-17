/*************************************************************************
    > File Name: get_task.c
    > Author: gatieme
    > Created Time: Mon 16 May 2016 03:28:46 PM CST
 ************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>

#include "print_vmarea.h"


void print_task(struct task_struct *task)
{

	printk("process : %s, pid : %d\n", task->comm, task->pid);

    print_task_vmarea(task);
}


