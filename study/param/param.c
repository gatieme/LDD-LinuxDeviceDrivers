/*************************************************************************
    > File Name: param.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: 2016年04月01日 星期五 21时37分37秒
 ************************************************************************/


#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>


/*
 *  在模块里面, 声明一个变量（全局变量）,
 *  用来接收用户加载模块哦时传递的参数
 *
 *  module_param(name, type, perm)
 **/
static int value = 0;
module_param(value, int, 0644);
MODULE_PARM_DESC(value_int, "Get an value from user...\n");

/*
 *  在模块内部变量的名字和加载模块时传递的参数名字不同
 *
 *  module_param_named(name_out, name_in, type, perm)
 *
 *  @name_out    加载模块时，参数的名字
 *  @name_in     模块内部变量的名字
 *  @type        参数类型
 *  @perm        访问权限
 * */
static int  value_in = 0;
module_param_named(value_out, value_in, int, 0644);
MODULE_PARM_DESC(value_in, "value_in named var_out...\n");


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


static char buffer[20] = "gatieme";
module_param_string(buffer, buffer, sizeof(buffer), 0644);
MODULE_PARM_DESC(value_charp, "Get an string buffer from user...\n");


/*
 *  加载模块的时候, 传递参数到模块的数组中
 *
 *  module_param_array(name, type, num_point, perm)
 *
 *  @name       模块的数组名，也是外部制定的数组名
 *  @type       模块数组的数据类型
 *  @num_point  用来获取用户在加载模块时传递的参数个数,
 *              为NULL时，表示不关心用户传递的参数个数
 *  @perm       访问权限
 *
 * */
static int  array[3];
int         num;
module_param_array(array, int, &num, 0644);
MODULE_PARM_DESC(array, "Get an array from user...\n");




int __init  param_module_init(void)
{
    int index = 0;

    printk("\n---------------------\n");
    printk("value       : %d\n", value);
    printk("value_in    : %d\n", value_in);
    printk("string      : %s\n", string);
    printk("buffer      : %s\n", buffer);

    for(index = 0; index < num; index++)
    {
    printk("array[%2d]   :   %d\n", index, array[index]);
    }
    printk("---------------------\n");

    return 0;
}

void __exit param_module_exit(void)
{
    printk("\n---------------------\n");
    printk("exit param dobule\n");
    printk("---------------------\n");
}


module_init(param_module_init);
module_exit(param_module_exit);


