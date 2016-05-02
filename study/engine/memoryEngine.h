/*
*  Author: HIT CS HDMC team.
*  Create: 2010-3-12 8:20:01
*  Last modified: 2010-6-13 14:13:47
*/

#ifndef _MEMORY_ENGINE_H
#define _MEMORY_ENGINE_H

/*
*  common include header files
*/


#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/kprobes.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/dcache.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/kprobes.h>
#include <asm/pgtable.h>
#include <asm/page.h>

#include "common.h"


#define MAX_LINE		        256						/// max length of a line
#define DELIMITER		        '\n'					/// the char is used to split the taskinfo
#define PERMISSION	            0644					/// proc node permission
/*
*	return values
*/
#define OK					     0
#define FAIL					-1
#define PGD_NONE 			    -2
#define PUD_NONE			    -3
#define PMD_NONE 			    -4
#define PTE_NONE 			    -5
#define PTE_NOT_PRESENT         -6



/*
*	request command
*/
#define REQUEST_TASK_INFO		1		/// get a task's memory map information
#define REQUEST_V2P				2		/// convert a process's linear address to physical address
#define REQUEST_KV2P			3		/// convert kernel virtual address to physical address
#define REQUEST_KFUNC_VA		4		/// get kernel function's addr(kernel virtual address)
#define REQUEST_READ_KFUNC	    5		/// 请求读取内核函数起始地址内容
#define REQUEST_WRITE_KFUNC	    6		/// 请求改写内核函数起始地址内容
///#define REQUEST_WRITE				10 	/// 请求改写指定物理地址内容，改为用户态实现此功能
///#define REQUEST_MEM					11	/// 请求获取全部物理内存信息
///#define REQUEST_ADDR_STOP		12	///

/*
*	ack signals
*/
#define ACK_TASK_INFO			REQUEST_TASK_INFO
#define ACK_V2P					REQUEST_V2P
#define ACK_KV2P				REQUEST_KV2P
#define ACK_KFUNC_VA			REQUEST_KFUNC_VA
#define ACK_READ_KFUNC		    REQUEST_READ_KFUNC
#define ACK_WRITE_KFUNC		    REQUEST_WRITE_KFUNC
///#define REQUEST_WRITE		REQUEST_WRITE
///#define ACK_MEM				REQUEST_MEM
///#define ACK_ADDR_STOP		REQUEST_ADDR_STOP

/*
*	utility functions
*/
static int handler_pre_kFunc(struct kprobe *p, struct pt_regs *regs);

struct task_struct *findTaskByPid(pid_t pid);

long v2p(struct mm_struct *pMM,unsigned long va,int *pStatus);

long kv2p(unsigned long va,int *pStatus);

long kFunc2v(char *funcName);

struct vm_area_struct * getVMA(struct mm_struct *pMM,unsigned long va);

pte_t * getPte(struct mm_struct *pMM,unsigned long va);

int setVMAFlags(struct mm_struct *pMM,unsigned long va,int *pStatus,int flags);

int setPageFlags(struct mm_struct *pMM,unsigned long va,int *pStatus,int flags);

int getTaskInfo(struct task_struct *pTask, char *pData, int length);

/*
*	proc entry function
*/
int proc_write_pid(struct file *file,const char __user *buffer,unsigned long count,void * data);

int proc_read_virtualAddr(char * page,char **start, off_t off, int count, int * eof,void * data);
int proc_write_virtualAddr(struct file *file,const char *buffer,unsigned long count,void * data);

int proc_write_ctl(struct file *file,const char *buffer,unsigned long count,void * data);

int proc_read_signal(char * page,char **start, off_t off, int count, int * eof,void * data);
int proc_write_signal(struct file *file,const char *buffer,unsigned long count,void * data);

int proc_read_pa(char * page,char **start, off_t off, int count, int * eof,void * data);

static int jforce_sig_info(int sig,struct siginfo *info,struct task_struct *t);


#endif

