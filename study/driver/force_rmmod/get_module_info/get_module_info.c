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
static char *modname = NULL;
module_param(modname, charp, 0644);
MODULE_PARM_DESC(modname, "The name of module you want do clean or delete...\n");


static struct module *find_module_by_name(char *mod_name)
{
    struct module  *list_mod, *mod1, *mod2;

    //  方法一, 遍历内核模块树list_mod查询
    /*  遍历模块列表, 查找 del_mod_name 模块  */
    list_for_each_entry(list_mod, THIS_MODULE->list.prev, list)
    {
        if (strcmp(list_mod->name, mod_name) == 0)
        {
            mod1 = list_mod;
        }
    }

    /*  如果未找到 mod_name 则直接退出  */
    if(mod1 == NULL)
    {
        printk("[%s] module %s not found\n", THIS_MODULE->name, mod_name);
    }
    else
    {
        printk("[before] name:%s, state:%d, refcnt:%u\n",
            mod1->name ,mod1->state, module_refcount(mod1));
    }


    //  方法二, 通过find_mod函数查找
    mod2 = find_module(mod_name);

    /*  如果未找到 mod_name 则直接退出  */
    if(mod1 == NULL)
    {
        printk("[%s] module %s not found\n", THIS_MODULE->name, mod_name);
    }
    else
    {
        printk("[before] name:%s, state:%d, refcnt:%u\n",
            mod2->name ,mod2->state, module_refcount(mod2));
    }

    if(mod1 != mod2)
    {
        printk("MOD1(%p) != MOD2(%p)\n", mod1, mod2);
        return NULL;
    }
    else
    {
        printk("MOD1(%p) == MOD2(%p)\n", mod1, mod2);
        return mod1;
    }
}

static void list_depond_module(struct module *mod)
{
    struct module *relate = NULL;
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
        printk("No modules depond on %s...\n", modname);
    }
}


static void get_module_symbol_address(const char *mod_name)
{
    unsigned long symbol_addr = 0;
    //unsigned long symbol_addr2 = 0;

    /*  打印内核模块的地址  */
    if((symbol_addr = kallsyms_lookup_name(mod_name)) == NULL)
    {
        printk("[%s] %s symbol_addr : (null)\n", THIS_MODULE->name, mod_name);
        //return 0;
    }
    else
    {
        printk("[%s] %s symbol_addr : 0x%x\n", THIS_MODULE->name, modname, symbol_addr);
    }
#if 0
    if((symbol_addr2 = module_kallsyms_lookup_name(mod_name)) == NULL)
    {
        printk("[%s] %s symbol_addr : (null)\n", THIS_MODULE->name, mod_name);
        //return 0;

    }
    else
    {
        printk("[%s] %s symbol_addr : 0x%x\n", THIS_MODULE->name, modname, symbol_addr2);
    }
#endif
    //printk("%0x == %0x\n", symbol_addr1, symbol_addr2);
}


static int __init get_module_info_init(void)
{
    struct module *mod = NULL;

    if((mod = find_module_by_name(modname)) != NULL)
    {
        get_module_symbol_address(modname);
        list_depond_module(mod);
    }

    return 0;
}

static void __exit get_module_info_exit(void)
{
    printk("=======name : %s, state : %d EXIT=======\n", THIS_MODULE->name, THIS_MODULE->state);
}

module_init(get_module_info_init);
module_exit(get_module_info_exit);

MODULE_LICENSE("GPL");
