#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
//#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <asm/desc.h>


// [《深入理解Linux内核》内存寻址学习心得](http://blog.chinaunix.net/uid-27776472-id-4304663.html)
// [lxr-get_cpu_gdt_table-Identifier Search](http://lxr.free-electrons.com/source/arch/x86/include/asm/desc.h#L48)
// [lxr-desc_struct-Identifier Search](http://lxr.free-electrons.com/source/arch/x86/include/asm/desc_defs.h#L22)


MODULE_LICENSE("Dual BSD/GPL");


#define bit_get(number, pos) ((number) >> (pos) & 1)     /// 用宏得到某数的某位
#define bit_set(number, pos) ((number) |= 1 << (pos))    /// 把某位置1
#define bit_clr(number, pos) ((number) &= ~(1 << (pos))) /// 把某位清0
#define bit_cpl(number, pos) ((number) ^= 1 << (pos))    /// 把number的POS位取反



/*
 * 打印段描述符的数据
 * desc 指向GDT的指针
 * http://lxr.free-electrons.com/source/arch/x86/include/asm/desc.h?v=4.10#L226
 */
static void print_desc_struct(struct desc_struct *desc)
{
#if 0
    printk("base = 0x%0x", get_desc_base(desc);
    printk("limit = 0x%0x", get_desc_limit(desc);


    printk("limit0 = %0x, ", desc->limit0);
    printk("base0 = %0x, ", desc->base0);

    printk("base1 = %0x, ", desc->base1);
    printk("type = %0x, ", desc->type);
    printk("s = %0x, ", desc->s);
    printk("dpl = %0x, ", desc->dpl);
    printk("p = %0x, ", desc->p);

    printk("limit = %0x, ", desc->limit);
    printk("avl = %0x, ", desc->avl);
    printk("l = %0x, ", desc->l);
    printk("d = %0x, ", desc->d);
    printk("g = %0x, ", desc->g);

    printk("base2 = %0x\n", desc->base2);
#endif
/*
段描述符
----------------------------------------------------------------------------------------
63      56/55        52/51        48/47                  40/39     32/31    16/15      0
----------------------------------------------------------------------------------------
| B31-B24 | G D/B O AV | L19-L16    | P DPL S E ED/C R/W A | B23-B16 | B15-B0 | L15-L0 |
----------------------------------------------------------------------------------------
            |  |  |  |                |  |  | |   |   |  |
            |  |  |  |                |  |  | ---------  |
            |  |  |  |                |  |  |     |      |
            |  |  |  |                |  |  |     |      ------------ = 1, 已被访问过
            |  |  |  |                |  |  |     |
            |  |  |  |                |  |  |     |               |-- E  = 0, 数据段
            |  |  |  |                |  |  |     |         ------|-- ED = 0, 向上延伸(数据段); ED = 1, 向下延伸(堆栈段)
            |  |  |  |                |  |  |     |         |     |-- W  = 0, 不可写入; W = 1, 可写入
            |  |  |  |                |  |  |     ----------|
            |  |  |  |                |  |  |               |     |-- E = 1, 代码段
            |  |  |  |                |  |  |               ------|-- C = 0, 忽视特权级别; C = 1, 依照特权级别
            |  |  |  |                |  |  |                     |-- R = 0, 不能读; R = 1, 可读
            |  |  |  |                |  |  |
            |  |  |  |                |  |  |                     |-- = 0, 表示专用于系统管理的系统段, 如各类描述表
            |  |  |  |                |  |  |---------------------|
            |  |  |  |                |  |                        |-- = 1, 表示用于一般的代码段或者数据段
            |  |  |  |                |  |
            |  |  |  |                |  ---------------------------- 特权级别
            |  |  |  |                ------------------------------- = 1, 表示该段在内存中
            |  |  |  |
            |  |  |  |
            |  |  |  --------   可由软件使用, CPU忽略该位
            |  |  -----------   永远为0
            |  --------------   = 1, 表示对该段的访问为32位指令; = 0, 为16位指令
            -----------------   = 1, 段长以4K字节为单位; = 0, 以字节为单位

*/

#define get_desc_struct_BASE0(gdt)   (((gdt) >> 16) & 0xffff)
#define get_desc_struct_BASE1(gdt)   (((gdt) >> 32) & 0xff)
#define get_desc_struct_BASE2(gdt)   (((gdt) >> 56) & 0xff)
#define get_desc_struct_BASE(gdt)                  \
            ((get_desc_struct_BASE2(gdt) << 24)    \
        |   (get_desc_struct_BASE1(gdt) << 16)     \
        |   (get_desc_struct_BASE0(gdt)))


#define get_desc_struct_LIMIT0(gdt)  ((gdt) & 0xffff)
#define get_desc_struct_LIMIT1(gdt)  (((gdt) >> 48) & 0x0f)
#define get_desc_struct_LIMIT(gdt)                 \
            ((get_desc_struct_LIMIT1(gdt) << 16)   \
        |   (get_desc_struct_LIMIT0(gdt)))

#define get_desc_struct_G(gdt)           (((gdt) >> 55) & 0x01)     /*  55  */
#define get_desc_struct_D_or_B(gdt)      (((gdt) >> 54) & 0x01)     /*  54  */
#define get_desc_struct_O(gdt)           (((gdt) >> 53) & 0x01)     /*  53  */
#define get_desc_struct_AV(gdt)          (((gdt) >> 52) & 0x01)     /*  52  */
#define get_desc_struct_P(gdt)           (((gdt) >> 47) & 0x01)     /*  47  */
#define get_desc_struct_DPL(gdt)         (((gdt) >> 45) & 0x03)     /* 46-45 */
#define get_desc_struct_S(gdt)           (((gdt) >> 44) & 0x01)     /*  44  */
#define get_desc_struct_E(gdt)           (((gdt) >> 43) & 0x01)     /*  43  */
#define get_desc_struct_ED_or_C(gdt)     (((gdt) >> 42) & 0x01)     /*  42  */
#define get_desc_struct_R_or_W(gdt)      (((gdt) >> 41) & 0x01)     /*  41  */
#define get_desc_struct_A(gdt)           (((gdt) >> 40) & 0x01)     /*  40  */


    printk("0x%08x, 0x%08x | 0x%x  |  %d  | %d |\n",
            desc->b, desc->a, desc->type, desc->dpl, desc->s);
    unsigned long data = *((unsigned long *)desc);
    printk("base = 0x%0x, limit = 0x%0x\n"
           "G = %d, D/B = %d, O = %d, AV = %d\n"
           "P = %d, DPL = %d, S = %d, type = %d%d%d%d\n",
            get_desc_struct_BASE(data), get_desc_struct_LIMIT(data),

            get_desc_struct_G(data), get_desc_struct_D_or_B(data),
            get_desc_struct_O(data), get_desc_struct_AV(data),

            get_desc_struct_P(data), get_desc_struct_DPL(data),
            get_desc_struct_S(data),
            get_desc_struct_E(data), get_desc_struct_ED_or_C(data),
            get_desc_struct_R_or_W(data), get_desc_struct_A(data));
}


/*
 * 打印 CPU 的 GDT 的数据
 * 参数
 *  cpu : CPU 的编号
 */
static void print_cpu_gdt_table(unsigned int cpu)
{
    int                 i           = 0;
    struct desc_struct *gdt_table   = get_cpu_gdt_table(cpu);

    printk("GDT_ENTRIES = %d\n", GDT_ENTRIES);
    printk("GDT_ENTRY_TLS_MIN  = %d\n", GDT_ENTRY_TLS_MIN);
    printk("GDT_ENTRY_TLS_ENTRIES = %d\n", GDT_ENTRY_TLS_ENTRIES);

    printk("-----------------------------------\n");
    printk("|\t\t\t\t| type | dpl | s |\n");
    for(i = 0; i < GDT_ENTRIES; i++)
    {
        printk("| [%0x] = ", i);
        print_desc_struct(gdt_table + i);
    }
    printk("-----------------------------------\n");

}

/*
 * 打印段选择符和段描述符号的信息
 *
 * 段标识符(段选择符)(存放在段寄存器中)字段如下 :
 * index    用来得到段描述符在GDT或LDT中的偏移（位置）
 * TI       指明段描述符是在GDT中(TI=0)或在LDT中(TI=1)
 * RPL      请求特权级，
 *
 * 由于一个段描述符是8字节,
 * 因此它的地址=GDT或LDT内的首地址+(index*8).
 * 能够保存在GDT中的段描述符的最大数目是8191(213-1)，其中13是index的位数.
 * 不同段的段描述符构成段描述符表,
 * 段描述符(8字节)存放在全局描述符表(GDT)或局部描述符表(LDT)中
 * GDT在主存中的地址和大小存放在gdtr控制寄存器中,
 * 当前正被使用的LDT地址和大小存放在ldtr控制寄存器中。
 */
static void print_segment_desc(unsigned long seg)
{
#define get_segment_INDEX(n)    ((n) >> 3)
#define get_segment_TI(n)       (((n) >> 2) & 0x01)
#define get_segment_RPL(n)      ((n) & 0x03)
    printk("0x%0x, index = %d, TI = %d, RPL = %d\n", seg,
            get_segment_INDEX(seg),
            get_segment_TI(seg),
            get_segment_RPL(seg));
}


// http://lxr.free-electrons.com/source/arch/x86/include/asm/segment.h#L123
/*
 *  打印 Linux 内核中分段信息,
 *  打印段描述
 */
static void print_segment(void)
{
    struct desc_struct *gdt_table   = get_cpu_gdt_table(1);


    /*
        ---------------------------------------------------------------------------------------------
        |                         |       INDEX               | TI| RPL |
        ---------------------------------------------------------------------------------------------
        __KERNEL_CS = 0x0010 =    B 0 0 0 0 0 0 0 0 0 0 0 1 0 | 0 | 0 0 | index = 2, TI = 0, RPL = 0
        ---------------------------------------------------------------------------------------------
        __KERNEL_DS = 0x0018 =    B 0 0 0 0 0 0 0 0 0 0 0 1 1 | 0 | 0 0 | index = 3, TI = 0, RPL = 0
        ---------------------------------------------------------------------------------------------
        __USER_DS   = 0x0033 =    B 0 0 0 0 0 0 0 0 0 0 1 1 0 | 0 | 1 1 | index = 6, TI = 0, RPL = 3
        ---------------------------------------------------------------------------------------------
        __USER_DS   = 0x002B =    B 0 0 0 0 0 0 0 0 0 0 1 0 1 | 0 | 1 1 | index = 5, TI = 0, RPL = 3
        ---------------------------------------------------------------------------------------------

        |   LE little-endian   |  低字节 -=>  高字节  |
        ---------------------------------------------------------------------------------------------
        __KERNEL_CS = 0x0010 = | 00010000    00000000 |
        __KERNEL_DS = 0x0018 = | 00011000    00000000 |
        __USER_DS   = 0x0033 = | 00110011    00000000 |
        __USER_DS   = 0x002B = | 00101011    00000000 |
     */
    printk("=====================\n");
    // 打印__KERNEL_CS的段描述符
    printk("__KERNEL_CS = ");
    print_segment_desc(__KERNEL_CS);
    //  打印__KERNEL_CS在gdt_table中的具体值
    printk("__KERNEL_CS @ index(%d) in cpu_gdt_table\n", get_segment_INDEX(__KERNEL_CS));
    printk("GDT[%d] = ", get_segment_INDEX(__KERNEL_CS));
    print_desc_struct(gdt_table + get_segment_INDEX(__KERNEL_CS));
    printk("=====================\n\n");

    printk("=====================\n");
    // 打印__KERNEL_DS的段描述符
    printk("__KERNEL_DS = ");
    print_segment_desc(__KERNEL_DS);
    //  打印__KERNEL_DS在gdt_table中的具体值
    printk("__KERNEL_DS @ index(%d) in cpu_gdt_table\n", get_segment_INDEX(__KERNEL_DS));
    printk("GDT[%d] = ", get_segment_INDEX(__KERNEL_DS));
    print_desc_struct(gdt_table + get_segment_INDEX(__KERNEL_DS));
    printk("=====================\n\n");

    printk("=====================\n");
    // 打印__USER_CS的段描述符
    printk("__USER_CS = ");
    print_segment_desc(__USER_CS);
    //  打印__USER_CS在gdt_table中的具体值
    printk("__USER_CS @ index(%d) in cpu_gdt_table\n", get_segment_INDEX(__USER_CS));
    printk("GDT[%d] = ", get_segment_INDEX(__USER_CS));
    print_desc_struct(gdt_table + get_segment_INDEX(__USER_CS));
    printk("=====================\n\n");

    printk("=====================\n");
    // 打印__USER_DS的段描述符
    printk("__USER_DS = ");
    print_segment_desc(__USER_DS);
    //  打印__USER_DS在gdt_table中的具体值
    printk("__USER_DS @ index(%d) in cpu_gdt_table\n", get_segment_INDEX(__USER_DS));
    printk("GDT[%d] = ", get_segment_INDEX(__USER_DS));
    print_desc_struct(gdt_table + get_segment_INDEX(__USER_DS));
    printk("=====================\n\n");
}

static int get_cpu_gdt_table_init(void)
{

    print_cpu_gdt_table(0);

    print_segment();


    return 0;
}



static void get_cpu_gdt_table_exit(void)
{
}


module_init(get_cpu_gdt_table_init);
module_exit(get_cpu_gdt_table_exit);
