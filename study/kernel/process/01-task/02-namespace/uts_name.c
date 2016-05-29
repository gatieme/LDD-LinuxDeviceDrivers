#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>


#include <linux/kernel.h>

#include <generated/compile.h>
#include <linux/module.h>
#include <linux/uts.h>
#include <linux/utsname.h>
#include <generated/utsrelease.h>
#include <linux/version.h>
#include <linux/proc_ns.h>



/*
 *
 * init_uts_ns be defined in init/version.c
 * http://lxr.free-electrons.com/source/init/version.c?v=4.5#L25
 *
 * the struct uts_namespace be degined in linux/utsname.h
 * http://lxr.free-electrons.com/source/include/linux/utsname.h?v=4.5#L23
 *

struct uts_namespace init_uts_ns =
{
    .kref =
    {
        .refcount       = ATOMIC_INIT(2),
    },

    .name =
    {
         .sysname        = UTS_SYSNAME,
         .nodename       = UTS_NODENAME,
         .release        = UTS_RELEASE,
         .version        = UTS_VERSION,
         .machine        = UTS_MACHINE,
         .domainname     = UTS_DOMAINNAME,
    },

    .user_ns = &init_user_ns,
    .proc_inum = PROC_UTS_INIT_INO,
};
EXPORT_SYMBOL_GPL(init_uts_ns);

*/
struct uts_namespace *utsns = &init_uts_ns;


static void print_new_utsname(struct new_utsname *pnew_utsname)
{

    printk(KERN_ALERT "sysname    = %s\n", pnew_utsname->sysname);
    printk(KERN_ALERT "nodename   = %s\n", pnew_utsname->nodename);
    printk(KERN_ALERT "release    = %s\n", pnew_utsname->release);
    printk(KERN_ALERT "version    = %s\n", pnew_utsname->version);
    printk(KERN_ALERT "sysname    = %s\n", pnew_utsname->machine);
    printk(KERN_ALERT "domainname = %s\n", pnew_utsname->domainname);

}

static void print_user_namespace(struct user_namespace *puser_ns)
{
    struct ns_common ns;
}


static void print_uts_namespace(struct uts_namespace *utsns)
{
    //  struct kref kref;
    //printk(KERN_ALERT "atomic kref %d\n", utsns->kref.refcount.val);
    printk(KERN_ALERT "atomic kref %d\n", utsns->kref.refcount.counter);

    struct new_utsname *pnew_utsname = &utsns->name;
    print_new_utsname(pnew_utsname);


    struct user_namespace *puser_ns = utsns->user_ns;
    print_user_namespace(puser_ns);

    //printk(KERN_ALERT "proc_inum = %d\n", utsns->proc_inum);

}


static int __init init_uts_name(void)
{
    printk("=====================\n");
    print_uts_namespace(utsns);
    printk("=====================\n");

    return 0;
}

static void __exit exit_uts_name(void)
{

    printk("=====================\n");
    printk(KERN_ALERT "Good Bye!!--uts_name\n");
    printk("=====================\n");

}

module_init(init_uts_name);
module_exit(exit_uts_name);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Gatieme @ HIT CS HDMC team");
MODULE_DESCRIPTION("Memory Engine Module.");

