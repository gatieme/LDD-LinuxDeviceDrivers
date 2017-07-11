#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cpumask.h>
#include <linux/list.h>
#include <asm-generic/local.h>
#include <linux/platform_device.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>


/*
 *  加载模块的时候, 传递字符串到模块的一个全局字符数组里面
 *
 *  module_param_string(name, string, len, perm)
 *
 *  @name   在加载模块时，参数的名字
 *  @string 模块内部的字符数组的名字
 *  @len    模块内部的字符数组的大小
 *  #perm   访问权限
 *
 * */
static char *modname = NULL;
module_param(modname, charp, 0644);
MODULE_PARM_DESC(modname, "The name of module you want do clean or delete...\n");


//#define CONFIG_REPLACE_EXIT_FUNCTION

#ifdef CONFIG_REPLACE_EXIT_FUNCTION
//  此处为外部注册的待卸载模块的exit函数
//  用于替代模块原来的exit函数
//  注意--此函数由于需要被待删除模块引用, 因此不能声明为static
/* static */ void force_replace_exit_module_function(void)
{
    /////////////////////
    //  此处完善待卸载驱动的 exit/cleanup 函数
    /////////////////////

    printk("module %s exit SUCCESS...\n", modname);
//    platform_device_unregister((struct platform_device*)(*(int*)symbol_addr));
}
#endif  //  CONFIG_REPLACE_EXIT_FUNCTION


static int force_cleanup_module(char *del_mod_name)
{
    struct module   *mod = NULL, *relate = NULL;
    int              cpu;
#ifdef CONFIG_REPLACE_EXIT_FUNCTION
    void            *origin_exit_addr = NULL;
#endif

    /////////////////////
    //  找到待删除模块的内核module信息
    /////////////////////
#if 0
    //  方法一, 遍历内核模块树list_mod查询
    struct module *list_mod = NULL;
    /*  遍历模块列表, 查找 del_mod_name 模块  */
    list_for_each_entry(list_mod, THIS_MODULE->list.prev, list)
    {
        if (strcmp(list_mod->name, del_mod_name) == 0)
        {
            mod = list_mod;
        }
    }

    /*  如果未找到 del_mod_name 则直接退出  */
    if(mod == NULL)
    {
        printk("[%s] module %s not found\n", THIS_MODULE->name, modname);
        return -1;
    }
#endif

    //  方法二, 通过find_mod函数查找
    if((mod = find_module(del_mod_name)) == NULL)
    {
        printk("[%s] module %s not found\n", THIS_MODULE->name, del_mod_name);
        return -1;
    }
    else
    {
        printk("[before] name:%s, state:%d, refcnt:%u\n",
                mod->name ,mod->state, module_refcount(mod));
    }

    /////////////////////
    //  如果有其他驱动依赖于当前驱动, 则不能强制卸载, 立刻退出
    /////////////////////
    /*  如果有其他模块依赖于 del_mod  */
    if (!list_empty(&mod->source_list))
    {
        /*  打印出所有依赖target的模块名  */
        list_for_each_entry(relate, &mod->source_list, source_list)
        {
            printk("[relate]:%s\n", relate->name);
        }
    }
    else
    {
        printk("No modules depond on %s...\n", del_mod_name);
    }

    /////////////////////
    //  清除驱动的状态和引用计数
    /////////////////////
    //  修正驱动的状态为LIVE
    mod->state = MODULE_STATE_LIVE;

    //  清除驱动的引用计数
    for_each_possible_cpu(cpu)
    {
        local_set((local_t*)per_cpu_ptr(&(mod->refcnt), cpu), 0);
        //local_set(__module_ref_addr(mod, cpu), 0);
        //per_cpu_ptr(mod->refptr, cpu)->decs;
        //module_put(mod);
    }
    atomic_set(&mod->refcnt, 1);

#ifdef CONFIG_REPLACE_EXIT_FUNCTION
    /////////////////////
    //  重新注册驱动的exit函数
    /////////////////////
    origin_exit_addr = mod->exit;
    if (origin_exit_addr == NULL)
    {
        printk("module %s don't have exit function...\n", mod->name);
    }
    else
    {
        printk("module %s exit function address %p\n", mod->name, origin_exit_addr);
    }

    mod->exit = force_replace_exit_module_function;
    printk("replace module %s exit function address (%p -=> %p)\n", mod->name, origin_exit_addr, mod->exit);
#endif

    printk("[after] name:%s, state:%d, refcnt:%u\n",
            mod->name, mod->state, module_refcount(mod));

    return 0;
}


static int __init force_rmmod_init(void)
{
    return force_cleanup_module(modname);
}


static void __exit force_rmmod_exit(void)
{
    printk("=======name : %s, state : %d EXIT=======\n", THIS_MODULE->name, THIS_MODULE->state);
}

module_init(force_rmmod_init);
module_exit(force_rmmod_exit);

MODULE_LICENSE("GPL");
