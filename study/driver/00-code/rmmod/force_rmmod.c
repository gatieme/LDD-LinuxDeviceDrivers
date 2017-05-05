#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cpumask.h>
#include <linux/list.h>
#include <asm-generic/local.h>
#include <linux/platform_device.h>
#include <linux/kallsyms.h>


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
static char *string = NULL;
module_param(string, charp, 0644);
MODULE_PARM_DESC(string, "Get an string(char *) value from user...\n");


/*
static char buffer[20] = "gatieme";
module_param_string(buffer, buffer, sizeof(buffer), 0644);
MODULE_PARM_DESC(value_charp, "Get an string buffer from user...\n");
*/

static void force_exit_module(void)
{
    int symbol_addr;

    printk("XXXXXX, force!\n");
    symbol_addr = kallsyms_lookup_name(string);

    platform_device_unregister((struct platform_device*)(*(int*)symbol_addr));
}

static int force_cleanup_module(char *del_mod_name)
{
    struct module *list_mod, *mod, *relate;
    int cpu;

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
        printk("[%s] module %s not found\n", THIS_MODULE->name, string);
        return -1;
    }

    printk("[before] name:%s, state:%d, refcnt:%u\n",
            mod->name ,mod->state, module_refcount(mod));

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

    mod->state = 0;
    //mod->exit = force;

    for_each_possible_cpu(cpu)
    {
        local_set((local_t*)per_cpu_ptr(&(mod->refcnt), cpu), 0);
        //local_set(__module_ref_addr(mod, cpu), 0);
        //per_cpu_ptr(mod->refptr, cpu)->decs;
        //module_put(mod);
    }
    atomic_set(&mod->refcnt, 1);

    printk("[after] name:%s, state:%d, refcnt:%u\n",
            mod->name, mod->state, module_refcount(mod));

    return 0;
}


static int __init force_rmmod_init(void)
{
    int symbol_addr;
    int ret;

    /*  打印内核模块的信息*/
    printk("=======name : %s, state : %d INIT=======\n", THIS_MODULE->name, THIS_MODULE->state);

    if(string == NULL)
    {
        printk("[%s] module_name (NULL)\n", THIS_MODULE->name);
        return 0;
    }

    /*  打印内核模块的地址  */
    if((symbol_addr = kallsyms_lookup_name(string)) == NULL)
    {
        printk("[%s] %s symbol_addr : (null)\n", THIS_MODULE->name, string);
        return 0;
    }
    printk("[%s] %s symbol_addr : 0x%x\n", THIS_MODULE->name, string, symbol_addr);


    return force_cleanup_module(string);
}


static void __exit force_rmmod_exit(void)
{
    printk("=======name : %s, state : %d EXIT=======\n", THIS_MODULE->name, THIS_MODULE->state);
}

module_init(force_rmmod_init);
module_exit(force_rmmod_exit);

MODULE_LICENSE("GPL");
