/*
*  Author: HIT CS HDMC team.
*  Create: 2010-3-12 8:50
*  Last modified: 2010-6-13 14:06:20
*  Description:
*   Memory fault injection engine running as a kernel module.
*       This module will create "/proc/memoryEngine/" directory and 9 proc nodes.
*   Write paramenters and request to these proc nodes and read the output from related proc node.
*/

#include "main.h"



/*
*   proc entries
*/
struct proc_dir_entry   *proc_dir             = NULL; // the directory of the Moudle
struct proc_dir_entry   *proc_wo = NULL; // write only
struct proc_dir_entry   *proc_ro  = NULL; // read only
struct proc_dir_entry   *proc_rw = NULL; // rw


extern const struct file_operations proc_ro_fops;  /*  RW */
extern const struct file_operations proc_rw_fops; /*  RW */
extern const struct file_operations proc_wo_fops; /*  WO */




static int __init proc_test_init(void)
{
    /*
     *  create a direntory named "memoryEngine" in /proc for the moudles
     *  as the interface between the kernel and the user program.
     *
     */
    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if(proc_dir == NULL)
    {
        printk("Can't create " PROC_ROOT "/" PROC_DIR "\n");
        return FAIL;
    }
    printk("PROC_MKDIR ");
    printk("Create " PROC_ROOT "/" PROC_DIR " success...\n");

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 32)
    /// modify by gatieme for system porting NeoKylin-linux-3.14/16
    /// error: dereferencing pointer to incomplete type
    proc_dir->owner = THIS_MODULE;
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
#if defined(CREATE_PROC_ENTRY)

    proc_ro = create_proc_entry(PROC_READ_ONLY_ENTRY, PERMISSION, proc_dir/;
    if(proc_ro == NULL)
    {
        printk("Can't create " PROC_READ_ONLY_FILE "\n");
        //ret = FAIL;

        goto create_read_only_failed;

    }

    proc_ro->write_proc = proc_read_ro;              // write only
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 32)
    proc_ro->owner = THIS_MODULE;
#endif

    printk("CREATE_PROC_ENTRY ");
#elif defined(PROC_CREATE)

    proc_ro = proc_create(PROC_READ_ONLY_ENTRY, PERMISSION, proc_dir, &proc_ro_fops);

    if(proc_ro == NULL)
    {
        printk("Can't create " PROC_READ_ONLY_FILE "\n");
        //ret = FAIL;

        goto create_read_only_failed;
    }
    printk("PROC_CREATE ");
#endif
    printk("Create " PROC_READ_ONLY_FILE " success...\n");


    ////////////////////////////////////////////////////////////////////////////
    /// create a file named "write only" in direntory
    ////////////////////////////////////////////////////////////////////////////
#ifdef CREATE_PROC_ENTRY

    proc_wo = create_proc_entry(PROC_WRITE_ONLY_ENTRY, PERMISSION, proc_dir);

    if(proc_wo == NULL)
    {
        printk("Can't create " PROC_WRITE_ONLY_FILE "\n");
        //ret = FAIL;

        goto create_write_only_failed;
    }
    proc_wo->write_proc = proc_write_wo;  /// write only
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 32)
    proc_wo->owner = THIS_MODULE;
#endif

    printk("CREATE_PROC_ENTRY ");

#elif defined PROC_CREATE

    proc_wo = proc_create(PROC_WRITE_ONLY_ENTRY, PERMISSION, proc_dir, &proc_wo_fops);

    if(proc_wo == NULL)
    {
        printk("Can't create " PROC_WRITE_ONLY_FILE "\n");
        //ret = FAIL;

        goto create_write_only_failed;
    }
    printk("PROC_CREATE ");
#endif
    printk("Create " PROC_WRITE_ONLY_FILE " success...\n");




    ////////////////////////////////////////////////////////////////////////////
    /// create a file named "read_write" in direntory
    ////////////////////////////////////////////////////////////////////////////
#ifdef CREATE_PROC_ENTRY

    proc_rw = create_proc_entry(PROC_READ_WRITE_ENTRY, PERMISSION, proc_dir);
    if(proc_rw == NULL)
    {
        printk("Can't create " PROC_READ_WRITE_FILE "\n");
        ret = FAIL;

        goto create_read_write_failed;
    }
    proc_rw->read_proc = proc_read_rw;         // can read
    proc_rw->write_proc = proc_write_rw;       // can write
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 32)
    proc_rw->owner = THIS_MODULE;
#endif

    printk("CREATE_PROC_ENTRY ");

#elif defined PROC_CREATE

    proc_rw = proc_create(PROC_READ_WRITE_ENTRY, PERMISSION, proc_dir, &proc_rw_fops);

    if(proc_rw == NULL)
    {
        printk("Can't create " PROC_READ_WRITE_FILE "\n");
        //ret = FAIL;


        goto create_read_write_failed;
    }
    printk("PROC_CREATE ");
#endif

    printk("Create " PROC_READ_WRITE_FILE " success...\n");

    printk("Memory engine module init\n");

    return OK;



create_read_write_failed    :
    remove_proc_entry(PROC_WRITE_ONLY_ENTRY, proc_dir);

create_write_only_failed    :
    remove_proc_entry(PROC_READ_ONLY_ENTRY, proc_dir);

create_read_only_failed     :
    remove_proc_entry(PROC_DIR, NULL);

    return FAIL;
}



/*
*  uninit memory fault injection module
*/
static void __exit proc_test_exit(void)
{
    remove_proc_entry(PROC_READ_WRITE_ENTRY, proc_dir);
    printk("Remove " PROC_READ_WRITE_FILE " success\n");

    remove_proc_entry(PROC_WRITE_ONLY_ENTRY, proc_dir);
    printk("Remove " PROC_WRITE_ONLY_FILE " success\n");

    remove_proc_entry(PROC_READ_ONLY_ENTRY, proc_dir);
    printk("Remove " PROC_READ_ONLY_FILE " success\n");

    proc_remove(proc_dir);
    printk("Remove " PROC_ROOT "/" PROC_DIR " success\n");

    printk("proc-test module exit\n");


}

module_init(proc_test_init);
module_exit(proc_test_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Gatieme @ HIT CS HDMC team");
MODULE_DESCRIPTION("Memory Engine Module.");

