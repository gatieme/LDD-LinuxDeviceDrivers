/*
*  Author: HIT CS HDMC team.
*  Create: 2010-3-12 8:50
*  Last modified: 2010-6-13 14:06:20
*  Description:
*  	Memory fault injection engine running as a kernel module.
*		This module will create "/proc/memoryEngine/" directory and 9 proc nodes.
*   Write paramenters and request to these proc nodes and read the output from related proc node.
*/

#include "memoryEngine.h"

/*
*	proc entries
*/
struct proc_dir_entry *dir = NULL;                  /// the directory of the MEMORY INJECT Moudle
struct proc_dir_entry *proc_pid = NULL;				/// write only
struct proc_dir_entry *proc_va = NULL;				/// write only
struct proc_dir_entry *proc_ctl = NULL;				/// write only
struct proc_dir_entry *proc_kFuncName = NULL;	    /// write only
struct proc_dir_entry *proc_val = NULL;				/// rw
struct proc_dir_entry *proc_signal = NULL;		    /// rw
struct proc_dir_entry *proc_pa = NULL;				/// read only
struct proc_dir_entry *proc_taskINfo = NULL;    	/// read only

/*
* proc node values
*/
int             pid;								/// pid
unsigned long   va;					                /// virtual  Address
unsigned long   pa;					                /// physical Addreess
int             ctl;								/// ctl
int             signal;								/// signal
char            kFuncName[MAX_LINE];                /// kFuncName
long            memVal;							    /// memVal

unsigned long   ack_pa;			                    /// physical Address
unsigned long   ack_va;			                    /// virtual  Address
int             ack_signal;						    /// signal
int             ret;                                /// return value
char            taskInfo[PAGE_SIZE];	            /// taskInfo

///////////////////////////////////////////////

unsigned long   userspace_phy_mem;                  /// user space physical memory
long            orig_pa_data;                       /// origin data of the physics memory
long            new_pa_data;                        /// the new data you want write to physics memory

int             faultInterval;

/*
*	kprobe
*/
static struct kprobe kp_kFunc;

struct jprobe jprobe1 =
{
    .entry				           = jforce_sig_info,
    .kp =
    {
        .symbol_name = "force_sig_info",
    },
};

//时钟中断数计数
static int count = 0;

//保存原始代码
static long orig_code = 0;

/*
*  process the request
*/
void do_request(void)
{
	struct task_struct *task = NULL;
	unsigned long pa = 0;
	long kernel_va = 0;
	int status;

	/// get a task's memory map information
	if(ctl == REQUEST_TASK_INFO)
	{
		dbginfo("Rcv request:Get task info\n");

        memset(taskInfo,'\0',sizeof(taskInfo));

        if(pid <= 0)
		{
			ack_signal = ACK_TASK_INFO;
			return;
		}
		task = findTaskByPid(pid);

        if( task != NULL )
		{
			getTaskInfo(task, taskInfo, sizeof(taskInfo));
		}
		ack_signal = ACK_TASK_INFO;

		return;
	}
	/// convert a process's linear address to physical address
	else if(ctl == REQUEST_V2P)
	{
		task = findTaskByPid(pid);
		if( task == NULL )
		{
			dbginfo("No such process\n");
			ack_pa = -1;
			ack_signal = ACK_V2P;
			return;
		}
		if( task->mm == NULL )
		{
			ack_pa = -1;
			ack_signal = ACK_V2P;
			return;
		}
		ack_pa = v2p(task->mm,va,&status);
		if(ack_pa == FAIL)
		{
			dbginfo("No physical address\n");
		}
		ack_signal = ACK_V2P;
		return;
	}
	/// convert kernel virtual address to physical address
	else if(ctl == REQUEST_KV2P)
	{
		ack_pa = kv2p(va,&status);
		if(pa == FAIL)
		{
			dbginfo("No physical address\n");
		}
		ack_signal = ACK_KV2P;
		return;
	}
	/// get kernel function's addr(kernel virtual address)
	else if(ctl == REQUEST_KFUNC_VA)
	{
		ack_va = kFunc2v(kFuncName);
		ack_signal = ACK_KFUNC_VA;
		return;
	}
	/// 请求读取内核函数起始地址内容
	else if(ctl == REQUEST_READ_KFUNC)
	{
		kernel_va = kFunc2v(kFuncName);
		memVal = *((long *)kernel_va);
		ack_signal = ACK_READ_KFUNC;
	}
	/// 请求改写内核函数起始地址内容
	else if(ctl == REQUEST_WRITE_KFUNC)
	{
		//利用kprobe，在第一次调用do_timer()时，注入故障
		int ret;
		count = 0;
		if(strlen(kFuncName) > 0)
		{
			faultInterval = 1;	//故障仅持续一个时钟周期
			kp_kFunc.addr = 0;
			kp_kFunc.symbol_name = kFuncName;
			kp_kFunc.pre_handler = handler_pre_kFunc;
			ret = register_kprobe(&kp_kFunc);
			if(ret < 0)
			{
				dbginfo("Fained to register kprobe\n");
				ack_signal = ACK_WRITE_KFUNC;
				return;
			}

			//等待故障注入结束
			dbginfo("start count\n");
			int temp=0;
			while(1)
			{
				if(count == -1)
				{
					unregister_kprobe(&kp_kFunc);
					dbginfo("recovery\n");
					break;
				}
				if(temp == -1)
				{
					break;
				}
				temp++;
				//dbginfo("count:%d\n",count);
			}
		}
		ack_signal = ACK_WRITE_KFUNC;
		dbginfo("Success to inject MTTR fault\n");
		return;
	}

}

/*
*  get a task's memory map information
*/
int getTaskInfo(struct task_struct *pTask, char *pData, int length)
{
	struct mm_struct *pMM;
	struct vm_area_struct *pVMA;
	struct vm_area_struct *p;
	char file[MAX_LINE];
	struct dentry *pPath = NULL;
	char *end, *start;
	char *info = pData;

	long phy_addr;
	unsigned long start_va,end_va;
	int status;

	if(pTask == NULL) { return FAIL; }
	if((pMM = pTask->mm) == NULL) { return FAIL; }

	memset(pData, '\0', length);
	//前19个字段是关于进程内存信息的总体信息
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->total_vm, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->locked_vm, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->shared_vm, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->exec_vm, DELIMITER);

	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->stack_vm, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->reserved_vm, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->def_flags, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->nr_ptes, DELIMITER);

	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->start_code, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->end_code, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->start_data, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->end_data, DELIMITER);

	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->start_brk, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->brk, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->start_stack, DELIMITER);

	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->arg_start, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->arg_end, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->env_start, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->env_end, DELIMITER);

	pVMA = pMM->mmap;
	if(pVMA == NULL) { return OK; }
	for(p=pVMA; p!=NULL; p=p->vm_next)
	{
		//起始地址
		safe_sprintf(pData, length, info+strlen(info), "%lx %lx ", p->vm_start, p->vm_end);
		//属性
		if(p->vm_flags & VM_READ)
		{	safe_sprintf(pData, length, info+strlen(info), "r"); }
		else
		{	safe_sprintf(pData, length, info+strlen(info), "-"); }

		if(p->vm_flags & VM_WRITE)
		{	safe_sprintf(pData, length, info+strlen(info), "w"); }
		else
		{	safe_sprintf(pData, length, info+strlen(info), "-"); }

		if(p->vm_flags & VM_EXEC)
		{	safe_sprintf(pData, length, info+strlen(info), "x"); }
		else
		{	safe_sprintf(pData, length, info+strlen(info), "-"); }

		if(p->vm_flags & VM_SHARED)
		{	safe_sprintf(pData, length, info+strlen(info), "s"); }
		else
		{	safe_sprintf(pData, length, info+strlen(info), "p"); }

		//对应文件名
		if(p->vm_file != NULL)
		{
			if(p->vm_file->f_dentry != NULL)
			{
				safe_sprintf(pData, length, info+strlen(info), " ");
				memset(file,'\0',sizeof(file));
				for(pPath = p->vm_file->f_dentry; pPath != NULL; pPath = pPath->d_parent)
				{
					if(strcmp(pPath->d_name.name,"/") != 0)
					{
						strcpy(file + strlen(file), pPath->d_name.name);
						strcpy(file + strlen(file), "/");
						continue;
					}
					break;
				}
				do
				{
					end = file + strlen(file) - 1;
					for(start = end - 1; *start != '/' && start > file; start--);
					if(*start == '/')	{start++;}
					*end = '\0';

					safe_sprintf(pData, length, info+strlen(info), "/%s", start);
					*start = '\0';
				} while(start > file);
			}
		}
		safe_sprintf(pData, length, info+strlen(info), "%c", DELIMITER);

		//对应物理地址页
		start_va = p->vm_start;
		end_va = p->vm_end;
		while(end_va > start_va)
		{
			safe_sprintf(pData, length, info+strlen(info), "%lx-%lx\t", start_va, start_va + PAGE_SIZE);
			phy_addr = v2p(pMM, start_va, &status);
			if(phy_addr != FAIL)
			{
				safe_sprintf(pData, length, info+strlen(info), "va:0x%lx <--> pa:0x%lx", start_va, phy_addr);
			}
			start_va += PAGE_SIZE;
			safe_sprintf(pData, length, info+strlen(info), "%c", DELIMITER);
		}

		safe_sprintf(pData, length, info+strlen(info), "%c", DELIMITER);
	}
	return OK;
}

/*
*
*/
static int handler_pre_kFunc(struct kprobe *p, struct pt_regs *regs)
{
	unsigned long va;
	va = (unsigned long)p->addr;
	if(va <= 0) { return OK; }

	//第一次触发
	if(count == 0)
	{
		//读取前64字节，注入故障
		orig_code = *((long *)va);
		//_inject_fault(va,memVal);
		*((long *)va) = memVal;	//故障
		return OK;
	}
	count ++;
	//到达结束时间
	if(count == faultInterval + 1)
	{
		//恢复code
		*((long *)va) = orig_code;
		count = -1;
	}
	return OK;
}

/*
*	find_task_by_pid maybe not supported
*	O(n) is fine :)
*/
struct task_struct * findTaskByPid(pid_t pid)
{
	struct task_struct *task = NULL;

    //  Traversing the process in the system to find the PID
    //  add by gatieme @2016-03-20
    for_each_process(task)
	{
		if(task->pid == pid)
        {
			return task;
	    }
    }
	return NULL;
}

/*
* convert a process's linear address to physical address
*/
long v2p(struct mm_struct *pMM,unsigned long va,int *pStatus)
{
	pte_t *pte = NULL;
	unsigned long phyaddress = FAIL;

	pte = getPte(pMM, va);
	if(pte != NULL)
	{
		phyaddress = (pte_val(*pte) & PAGE_MASK) | (va & ~PAGE_MASK);
	}
	return phyaddress;
}

/*
*  convert kernel virtual address to physical address
*/
long kv2p(unsigned long va,int *pStatus)
{
	if(va < 0)
		return FAIL;
	if(__pa(va) >= 0)
		return __pa(va);
	return FAIL;
}

/*
*  get kernel function's addr(kernel virtual address)
*	 the kernel function should be looked up in the System.map
*/
static struct kprobe kp;
long kFunc2v(char *funcName)
{
	int ret;
	unsigned long va;

	kp.addr = 0;
	kp.symbol_name = funcName;
	ret = register_kprobe(&kp);
	if(ret < 0)
	{
		dbginfo("Fained to register kprobe\n");
		return FAIL;
	}
	va = (unsigned long)kp.addr;
	unregister_kprobe(&kp);
	if(va == 0)
		return FAIL;
	return va;
}



/*
*
*/
struct vm_area_struct * getVMA(struct mm_struct *pMM,unsigned long va)
{
	struct vm_area_struct *p;
	if(pMM == NULL) return NULL;
	p = pMM->mmap;
	if(p == NULL) return NULL;

	for(; p != NULL; p = p->vm_next)
	{
		if( va >= p->vm_start && va < p->vm_end )
		{
			return p;
		}
	}
	return NULL;
}

/*
*
*/
pte_t * getPte(struct mm_struct *pMM, unsigned long va)
{
	pgd_t *pgd = NULL;
	pmd_t *pmd = NULL;
	pud_t *pud = NULL;
	pte_t *pte = NULL;

	///get the pdg entry pointer
	pgd =  pgd_offset(pMM, va);
	if(pgd_none(*pgd)) { return NULL; }

	pud = pud_offset(pgd,va);
	if(pud_none(*pud)) { return NULL; }

	pmd = pmd_offset(pud,va);
	if(pmd_none(*pmd)) { return NULL; }

	pte = pte_offset_kernel(pmd,va);
	if(pte_none(*pte)) { return NULL; }
	if(!pte_present(*pte)) { return NULL; }
	return pte;
}

/*
*
*/
int setVMAFlags(struct mm_struct *pMM,unsigned long va,int *pStatus,int flags)
{
	struct vm_area_struct *p;
	p = getVMA(pMM,va);
	if(p == NULL) return FAIL;

	if(flags > 0)
	{
		p->vm_flags |= VM_WRITE;
		p->vm_flags |= VM_SHARED;
	}
	if(flags == 0)
	{
		p->vm_flags &= ~VM_WRITE;
		p->vm_flags &= ~VM_SHARED;
	}
	else { return FAIL; }
	return OK;
}

/*
*
*/
int setPageFlags(struct mm_struct *pMM,unsigned long va,int *pStatus,int flags)
{
	pte_t *pte = NULL;
	pte_t ret;
	pte = getPte(pMM, va);
	if( pte == NULL ) { return FAIL; }
	if(flags > 0)
	{
		ret = pte_mkwrite(*pte);
	}
	else if(flags == 0)
	{
		ret = pte_wrprotect(*pte);
	}
	else { return FAIL;	}
	return OK;
}

/*
*
*/
int proc_write_pid( struct file *file,
                    const char __user *buffer,
                    unsigned long count,
                    void * data)
{
	int iRet;
	char sPid[MAX_LINE];

	if(count <= 0)
    {
        return FAIL;
    }

    memset(sPid, '\0', sizeof(sPid));
    /////////////////////////////////////////////////////////////////////
    //
    //  copy_from_user函数的目的是从用户空间拷贝数据到内核空间，
    //  失败返回没有被拷贝的字节数，成功返回0.
    //  这么简单的一个函数却含盖了许多关于内核方面的知识,
    //  比如内核关于异常出错的处理.
    //  从用户空间拷贝数据到内核中时必须很小心,
    //  假如用户空间的数据地址是个非法的地址,或是超出用户空间的范围，
    //  或是那些地址还没有被映射到，都可能对内核产生很大的影响，
    //  如oops，或被造成系统安全的影响.
    //  所以copy_from_user函数的功能就不只是从用户空间拷贝数据那样简单了，
    //  他还要做一些指针检查连同处理这些问题的方法.
    //
    //  函数原型在[arch/i386/lib/usercopy.c]中
    //  unsigned long
    //  copy_from_user( void *to,
    //                  const void __user *from,
    //                  unsigned long n)
    //
    /////////////////////////////////////////////////////////////////////
    //
    //  将用户空间中, 地址buffr指向的count个数据拷贝到内核空间地址sPid中
    iRet = copy_from_user(sPid, buffer, count);
	if(iRet != 0)
    {
        dbginfo("Error when copy_from_user...\n");
        return FAIL;
    }

    iRet = sscanf(sPid, "%d", &pid);        //  将读出来的数据sPid赋值给模块的全局变量pid
	if(iRet != 1)
    {
        return FAIL;
    }
	dbginfo("Rcv pid:%d\n",pid);

    return count;
}

/*
*
*/
int proc_read_virtualAddr(  char * page,
                            char **start,
                            off_t off,
                            int count,
                            int * eof,
                            void * data)
{
	int iLen;

	iLen = sprintf(page, "%lx", ack_va);


    return iLen;
}

/*
*
*/
int proc_write_virtualAddr( struct file *file,
                            const char *buffer,
                            unsigned long count,
                            void * data)
{
	int iRet;
	char sVa[MAX_LINE];

	if(count <= 0)
    {
        return FAIL;
    }

    memset(sVa, '\0', sizeof(sVa));

    iRet = copy_from_user(sVa, buffer, count);
	if(iRet)
    {
        return FAIL;
    }

    iRet = sscanf(sVa,"%lx",&va);
	if(iRet != 1)
    {
        return FAIL;
    }

	dbginfo("Rcv virtual addr:0x%lx\n",va);

    return count;
}

/*
*
*/
int proc_write_ctl( struct file *file,
                    const char *buffer,
                    unsigned long count,
                    void * data)
{
	int iRet;
	char sCtl[MAX_LINE];

	if(count <= 0)
    {
        return FAIL;
    }

    memset(sCtl, '\0', sizeof(sCtl));

    iRet = copy_from_user(sCtl, buffer, count);
	if(iRet)
    {
        return FAIL;
    }

    iRet = sscanf(sCtl,"%d",&ctl);
	if(iRet != 1)
    {
        return FAIL;
    }

    do_request();

    return count;
}

/*
*
*/
int proc_read_signal(   char * page,
                        char **start,
                        off_t off,
                        int count,
                        int * eof,
                        void * data)
{
	int iLen;
	iLen = sprintf(page, "%d", ack_signal);
	return iLen;
}

/*
*
*/
int proc_write_signal(  struct file *file,
                        const char *buffer,
                        unsigned long count,
                        void * data)
{
	int iRet;
	char sSignal[MAX_LINE];

	if(count <= 0)
    {
        return FAIL;
    }

    memset(sSignal, '\0', sizeof(sSignal));
	iRet = copy_from_user(sSignal, buffer, count);
	if(iRet)
    {
        return FAIL;
    }

    iRet = sscanf(sSignal,"%d",&signal);
	if(iRet != 1)
    {
        return FAIL;
    }

    dbginfo("Rcv signal:%d\n",signal);

    return count;
}

/*
*
*/
int proc_read_pa(char * page,char **start, off_t off, int count, int * eof,void * data)
{
	int iLen;
	iLen = sprintf(page, "%lx", ack_pa);
	return iLen;
}

/*
*
*/
int proc_write_pa(struct file *file,const char *buffer,unsigned long count,void * data)
{
	int iRet;
	char sPa[MAX_LINE];

	if(count <= 0) { return FAIL; }
	memset(sPa, '\0', sizeof(sPa));
	iRet = copy_from_user(sPa, buffer, count);
	if(iRet) { return FAIL; }
	iRet = sscanf(sPa,"%lx",&pa);
	if(iRet != 1) { return FAIL; }
	dbginfo("Rcv pa:0x%lx\n",pa);
	return count;
}

/*
*
*/
int proc_write_kFuncName(struct file *file,const char *buffer,unsigned long count,void * data)
{
	int iRet;
	if(count <= 0) { return FAIL; }
	memset(kFuncName, '\0', sizeof(kFuncName));
	iRet = copy_from_user(kFuncName, buffer, count);
	if(iRet) { return FAIL; }
	//remove '\n'
	if(kFuncName[strlen(kFuncName) - 1] == '\n')
	{
		kFuncName[strlen(kFuncName) - 1] = '\0';
	}
	dbginfo("Rcv kernel func name:%s\n",kFuncName);
	return count;
}

/*
*
*/
int proc_read_taskInfo(char * page,char **start, off_t off, int count, int * eof,void * data)
{
	int iLen;
	iLen = sprintf(page, "%s", taskInfo);
	return iLen;
}

/*
*
*/
int proc_write_memVal(struct file *file,const char *buffer,unsigned long count,void * data)
{
	int iRet;
	char sMemVal[MAX_LINE];

	if(count <= 0)
    {
        return FAIL;
    }

    memset(sMemVal, '\0', sizeof(sMemVal));
	iRet = copy_from_user(sMemVal, buffer, count);
	if(iRet)
    {
        return FAIL;
    }

    iRet = sscanf(sMemVal,"%lx",&memVal);
	if(iRet != 1)
    {
        return FAIL;
    }

    dbginfo("Rcv memVal:0x%lx\n",memVal);
	return count;
}

/*
*
*/
int proc_read_memVal(   char * page,
                        char **start,
                        off_t off,
                        int count,
                        int * eof,
                        void * data)
{
	int iLen;
	iLen = sprintf(page, "%lx", memVal);
	return iLen;
}

/*
*  init memory fault injection module
*  初始化内存注入故障模块
*
*  使用proc_mkdir()创建一个dir = /proc/memoryEngine
*  再利用create_proc_read_entry()函数创建一个processinfo文件。
*  我们从模块里面获取的信息都将写入到这些文件中。
*
*/
static int __init initME(void)
{
	/*
     *  create a direntory named "memoryEngine" in /proc for the moudles
     *  as the interface between the kernel and the user program.
     *
     */
    dir = proc_mkdir("memoryEngine", NULL);
	if(dir == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/\n");
		return FAIL;
	}
	dir->owner = THIS_MODULE;

    ///  create a file named "pid" in direntory
	proc_pid = create_proc_entry("pid", PERMISSION, dir);
	if(proc_pid == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/pid\n");

        remove_proc_entry("memoryEngine", NULL);

        return FAIL;
	}
	proc_pid->write_proc = proc_write_pid;  /// write only
	proc_pid->owner = THIS_MODULE;


    ///  create a file named "virtualAddr" in direntory
	proc_va = create_proc_entry("virtualAddr", PERMISSION, dir);
	if(proc_va == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/virtualAddr\n");

        remove_proc_entry("pid", dir);
		remove_proc_entry("memoryEngine", NULL);

        return FAIL;
	}
	proc_va->read_proc = proc_read_virtualAddr;         // can read
	proc_va->write_proc = proc_write_virtualAddr;       // can write
	proc_va->owner = THIS_MODULE;

    ///  create a file named "ctl" in direntory
	proc_ctl = create_proc_entry("ctl", PERMISSION, dir);
	if(proc_ctl == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/ctl\n");

        remove_proc_entry("pid", dir);
		remove_proc_entry("virtualAddr", dir);
		remove_proc_entry("memoryEngine", NULL);

        return FAIL;
	}
	proc_ctl->write_proc = proc_write_ctl;              // write only
	proc_ctl->owner = THIS_MODULE;

    ///  create a file named "signal" in direntory
	proc_signal = create_proc_entry("signal", PERMISSION, dir);
	if(proc_signal == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/signal\n");

        remove_proc_entry("pid", dir);
		remove_proc_entry("virtualAddr", dir);
		remove_proc_entry("ctl", dir);
		remove_proc_entry("memoryEngine", NULL);

        return FAIL;
	}
	proc_signal->read_proc = proc_read_signal;          //  can read
	proc_signal->write_proc = proc_write_signal;        //  can write
	proc_signal->owner = THIS_MODULE;

    ///  create a file named "physicalAddr" in direntory
	proc_pa = create_proc_entry("physicalAddr", PERMISSION, dir);
	if(proc_pa == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/physicalAddr\n");

        remove_proc_entry("pid", dir);
		remove_proc_entry("virtualAddr", dir);
		remove_proc_entry("ctl", dir);
		remove_proc_entry("memoryEngine", NULL);

        return FAIL;
	}

	proc_pa->read_proc = proc_read_pa;                  //  can read
	proc_pa->write_proc = proc_write_pa;                //  can write


    ///  create a file named "kFuncName" in direntory
	proc_kFuncName = create_proc_entry("kFuncName", PERMISSION, dir);
	if(proc_kFuncName == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/kFuncName\n");

        remove_proc_entry("pid", dir);
		remove_proc_entry("virtualAddr", dir);
		remove_proc_entry("ctl", dir);
		remove_proc_entry("physicalAddr", dir);
		remove_proc_entry("memoryEngine", NULL);

        return FAIL;
	}
	proc_kFuncName->write_proc = proc_write_kFuncName;  // write only


    ///  create a file named "taskInfo" in direntory
	proc_taskINfo = create_proc_entry("taskInfo", PERMISSION, dir);
	if(proc_taskINfo == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/taskInfo\n");

        remove_proc_entry("pid", dir);
		remove_proc_entry("virtualAddr", dir);
		remove_proc_entry("ctl", dir);
		remove_proc_entry("physicalAddr", dir);
		remove_proc_entry("kFuncName", dir);
		remove_proc_entry("memoryEngine", NULL);

        return FAIL;
	}
	proc_taskINfo->read_proc = proc_read_taskInfo;      // read only

    ///  create a file named "memVal" in direntory
	proc_val = create_proc_entry("memVal", PERMISSION, dir);
	if(proc_val == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/memVal\n");

		remove_proc_entry("pid", dir);
		remove_proc_entry("virtualAddr", dir);
		remove_proc_entry("ctl", dir);
		remove_proc_entry("physicalAddr", dir);
		remove_proc_entry("kFuncName", dir);
		remove_proc_entry("taskInfo", dir);
		remove_proc_entry("memoryEngine", NULL);

        return FAIL;
	}
	proc_val->write_proc = proc_write_memVal;           // can write
	proc_val->read_proc = proc_read_memVal;             // can read


	ret = register_jprobe(&jprobe1);
	if (ret < 0)
	{
		printk("register_jprobe jprobe1 failed, returned %d\n", ret);
		return ret;
	}
	printk("Planted jprobe at force_sig_info: %p\n", jprobe1.kp.addr);

	dbginfo("Memory engine module init\n");
	return OK;
}


static int jforce_sig_info(int sig,struct siginfo *info,struct task_struct *t)
{
    printk("MemSysFI: kernel is sending signal %d to process pid: %d, comm: %s\n",sig,t->pid,t->comm);
/*
    if (f_inject == 'N')
    {
        jprobe_return();
        return 0;
    }

    down_interruptible(&sem);
    if ( addone(addone(inj_info.rear))==inj_info.front )
    {
		*/
        /*error:队列满*/
       /*
	   sprintf(inj_info.inj_log[inj_info.rear].msg,"caution : buf is full, messages have been dropped\n");
    }
    else
    {
        inj_info.rear = addone(inj_info.rear);
        sprintf(inj_info.inj_log[inj_info.rear].msg,"warning : kernel is sending signal %d to process pid: %d, comm: %s\n",sig,current->pid,current->comm);
        //inj_info->inj_log[inj_info->rear ] = x ;
    }
    up(&sem);
	*/
    /*
    	if(message!=NULL)
    	{
    		for(i=0;i<256;i++)
    			message[i]='\0';

    		sprintf(message,"warning : kernel is sending signal %d to process pid: %d, comm: %s\n\0",sig,current->pid,current->comm);
    	}
    	else
    		return -1;


    	struct bufferList *buflist;
    	buflist = (struct bufferList *)kmalloc(sizeof(struct bufferList),GFP_KERNEL);
    	buflist->pNext = NULL;
    	buflist->buffer = message;

    	down_interruptible(&sem);
    	if(Head==NULL)
    	{
    		Head = buflist;
    		Tail = Head;
    	}
    	else
    	{
    	  Tail->pNext = buflist;
    	  Tail = buflist;
    	}
    	flag = 1;
    	up(&sem);
    	*/

//	wake_up_interruptible(&wq2);

    jprobe_return();
    return 0;
}

/*
*  uninit memory fault injection module
*/
static void __exit exitME(void)
{
	remove_proc_entry("pid", dir);
	remove_proc_entry("virtualAddr", dir);
	remove_proc_entry("ctl", dir);
	remove_proc_entry("signal", dir);
	remove_proc_entry("physicalAddr", dir);
	remove_proc_entry("kFuncName", dir);
	remove_proc_entry("taskInfo", dir);
	remove_proc_entry("memVal", dir);
	remove_proc_entry("memoryEngine", NULL);
	unregister_jprobe(&jprobe1);
	printk("jprobe at %p unregistered.\n",	jprobe1.kp.addr);
	dbginfo("Memory engine module exit\n");
}

module_init(initME);
module_exit(exitME);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIT CS HDMC team");
MODULE_DESCRIPTION("Memory Engine Module.");

