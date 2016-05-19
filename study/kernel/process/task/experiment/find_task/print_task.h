#ifndef __PRINT_TASK_H_INCLUDE__
#define __PRINT_TASK_H_INCLUDE__


#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>

#include "print_vmarea.h"


void print_task(struct task_struct *task);




#endif      //  #define __PRINT_TASK_H_INCLUDE__
