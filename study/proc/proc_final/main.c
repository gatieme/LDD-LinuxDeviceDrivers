/*
*  Author: HIT CS HDMC team.
*  Create: 2010-3-12 8:50
*  Last modified: 2010-6-13 14:06:20
*  Description:
*  	Memory fault injection engine running as a kernel module.
*		This module will create "/proc/memoryEngine/" directory and 9 proc nodes.
*   Write paramenters and request to these proc nodes and read the output from related proc node.
*/

#include "main.h"



/*
*	proc entries
*/
struct proc_dir_entry   *dir             = NULL; // the directory of the Moudle
struct proc_dir_entry   *proc_write_only = NULL; // write only
struct proc_dir_entry   *proc_read_only  = NULL; // read only
struct proc_dir_entry   *proc_read_write = NULL; // rw

unsigned long           write_only;
unsigned long           read_only;
unsigned long           read_write;



EXPORT_SYMBOL(write_only);
EXPORT_SYMBOL(read_only);
EXPORT_SYMBOL(read_write);





extern const struct file_operations proc_read_only_fops;  /*  RW */
extern const struct file_operations proc_read_write_fops; /*  RW */
extern const struct file_operations proc_write_only_fops; /*  WO */

static int __init proc_test_init(void)
{
	/*
     *  create a direntory named "memoryEngine" in /proc for the moudles
     *  as the interface between the kernel and the user program.
     *
     */
    dir = proc_mkdir("proc_test", NULL);
	if(dir == NULL)
	{
		dbginfo("Can't create "PROC_DIR"\n");
		return FAIL;
	}
    dbginfo("PROC_MKDIR ");
    printk("Create /proc/memoryEngine success...\n");

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 32)
    /// modify by gatieme for system porting NeoKylin-linux-3.14/16
    /// error: dereferencing pointer to incomplete type
	dir->owner = THIS_MODULE;
#endif


    /// modify by gatieme for system porting NeoKylin-linux-3.14/16 @ 2016--03-28 20:08
    /*
     *  ==
     *  write in STACK_OVER_FLOW http://stackoverflow.com/questions/26808325/implicit-declaration-of-function-create-proc-entry
     *  ==
     *
     *  proc filesystem has been refactored in 3.10,
     *  the function `create_proc_entry` has been removed,
     *  you should use the full featured `proc_create function` family.
     *
     *  Note that the signatures are different, you can find them in LXR
     *  3.10 version: http://lxr.free-electrons.com/source/include/linux/proc_fs.h?v=3.10
     *  3.9 version: http://lxr.free-electrons.com/source/include/linux/proc_fs.h?v=3.9
     *
     *  You can find greater explanation of using full featured /proc functions in the book Linux Device Drivers 4,
     *  or, if you want shorter solution, check this link (https://github.com/jesstess/ldd4/blob/master/scull/main.c)
     *  where you can see how the struct file_operations has been used. You do not have to setup to all fields of the struct.
     *
     *  but the function  remove_proc_remove, you can do nothing for it, becase there are tow function for it
     *  static inline void proc_remove(struct proc_dir_entry *de) {}
     *  #define remove_proc_entry(name, parent) do {} while (0)
     */


    ////////////////////////////////////////////////////////////////////////////
    ///  create a file named "read only" in direntory
    ////////////////////////////////////////////////////////////////////////////
#ifdef CREATE_PROC_ENTRY

    proc_read_only = create_proc_entry("ctl", PERMISSION, dir);
	if(proc_read_only == NULL)
	{
		dbginfo("Can't create "PROC_DIR PROC_READ_ONLY_ENTRY "\n");
        ret = FAIL;

        goto create_read_only_failed;

	}

    proc_read_only->write_proc = proc_read_only;              // write only

    //proc_read_only->owner = THIS_MODULE;

    dbginfo("CREATE_PROC_ENTRY ");
#elif defined PROC_CREATE

    proc_read_only = proc_create("ctl", PERMISSION, dir, &proc_read_only_fops);

    if(proc_read_only == NULL)
	{
		dbginfo("Can't create "PROC_DIR PROC_READ_ONLY_ENTRY "\n");
        ret = FAIL;

	    goto create_read_only_failed;
    }
    dbginfo("PROC_CREATE ");
#endif
    printk("Create "PROC_DIR PROC_READ_ONLY_ENTRY " success...\n");

    ////////////////////////////////////////////////////////////////////////////
    /// create a file named "write only" in direntory
    ////////////////////////////////////////////////////////////////////////////
#ifdef CREATE_PROC_ENTRY

    proc_write_only = create_proc_entry(PROC_DIR, PERMISSION, dir);

    if(proc_write_only == NULL)
	{
		dbginfo("Can't create " PROC_DIR PROC_WRITE_ONLY_ENTRY"\n");
        ret = FAIL;

        goto create_write_only_failed;
	}
	proc_write_only->write_proc = proc_write_pid;  /// write only
	//proc_write_only->owner = THIS_MODULE;

    dbginfo("CREATE_PROC_ENTRY ");

#elif defined PROC_CREATE

    proc_write_only = proc_create("pid", PERMISSION, dir, &proc_write_only_fops);

    if(proc_write_only == NULL)
	{
		dbginfo("Can't create " PRPC_DIR PROC_WRITE_ONLY_ENTRY "\n");
        ret = FAIL;

        goto create_write_only_failed;
	}
    dbginfo("PROC_CREATE ");
#endif
    printk("Create " PRPC_DIR PROC_WRITE_ONLY_ENTRY " success...\n");




    ////////////////////////////////////////////////////////////////////////////
    /// create a file named "read_write" in direntory
    ////////////////////////////////////////////////////////////////////////////
#ifdef CREATE_PROC_ENTRY

	proc_read_write = create_proc_entry("virtualAddr", PERMISSION, dir);
	if(proc_read_write == NULL)
	{
		dbginfo("Can't create " PRPC_DIR PROC_READ_WRITE_ENTRY "\n");
        ret = FAIL;

        goto create_read_write_failed;
	}
	proc_read_write->read_proc = proc_read_va;         // can read
	proc_read_write->write_proc = proc_write_va;       // can write
	//proc_read_write->owner = THIS_MODULE;

    dbginfo("CREATE_PROC_ENTRY ");
#elif defined PROC_CREATE

    proc_read_write = proc_create("virtualAddr", PERMISSION, dir, &proc_read_write_fops);

    if(proc_read_write == NULL)
	{
		dbginfo("Can't create " PRPC_DIR PROC_READ_WRITE_ENTRY "\n");
        ret = FAIL;

        goto create_va_failed;
    }
    dbginfo("PROC_CREATE ");
#endif

    printk("Create " PRPC_DIR PROC_READ_WRITE_ENTRY " success...\n");

	dbginfo("Memory engine module init\n");

    return OK;



create_read_write_failed    :
    remove_proc_entry(PROC_WRITE_ONLY_ENTRY, dir);

create_write_only_failed    :
    remove_proc_entry(PROC_READ_ONLY_ENTRY, dir);

create_read_only_failed     :
    remove_proc_entry(PROC_DIR, NULL);

    return ret;
}



/*
*  uninit memory fault injection module
*/
static void __exit exitME(void)
{
	remove_proc_entry("read_write", dir);
    printk("Remove /proc/proc_test/read_write success\n");

    remove_proc_entry("write_only", dir);
    printk("Remove /proc/proc_test/write_only success\n");

	remove_proc_entry("read_only", dir);
    printk("Remove /proc/proc_test/read_only success\n");

	dbginfo("Memory proc_test module exit\n");
}

module_init(initPROC);
module_exit(exitPROC);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Gatieme @ HIT CS HDMC team");
MODULE_DESCRIPTION("Memory Engine Module.");

