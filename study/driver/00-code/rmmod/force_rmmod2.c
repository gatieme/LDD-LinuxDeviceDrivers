#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/list.h>
#include<linux/cpumask.h>

static int __init mymod_init(void)
{
    struct module *list_mod, *mod,*relate;
    int cpu;
    int zero = 0;

    // 打印本模块的模块名和模块状态
    printk(KERN_ALERT"[insmod mymod] name:%s state:%d\n",THIS_MODULE->name,THIS_MODULE->state);


    // 遍历模块列表，查找target模块
    list_for_each_entry(list_mod, THIS_MODULE->list.prev, list)
    {
        if(strcmp(mod->name,"hello")==0)
        {
            mod = list_mod;
        }
    }

    // 打印target的模块名、模块状态、引用计数
    printk(KERN_ALERT"name:%s state:%d refcnt:%u ",mod->name,mod->state,module_refcount(mod));


    // 打印出所有依赖target的模块名
    if(!list_empty(&mod->modules_which_use_me))
    {
        list_for_each_entry(relate,&mod->modules_which_use_me,modules_which_use_me)
        {
            printk(KERN_ALERT"%s ",relate->name);
        }
    }
    else
    {
        printk(KERN_ALERT"used by NULL\n");
    }

    // 把target的引用计数置为0
    for_each_possible_cpu(cpu)
    {
        //local_set(__module_ref_addr(mod,cpu),0);
        //mod->ref[i].count=*(local_t *)&zero;
        atomic_set(&mod->refcnt, 0);
    }

    // 再看看target的名称、状态、引用计数
    printk(KERN_ALERT"name:%s state:%d refcnt:%u\n",mod->name,mod->state,module_refcount(mod));

    return 0;
}

static void __exit mymod_exit(void)
{
        printk(KERN_ALERT"[rmmod mymod] name:%s state:%d\n",THIS_MODULE->name,THIS_MODULE->state);
}

module_init(mymod_init);
module_exit(mymod_exit);

MODULE_AUTHOR("Zhangsk");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Why module can not be removed");

