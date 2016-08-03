#include <linux/module.h>
#include <linux/init.h>

#include <linux/sched.h>

#include <linux/mm.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <linux/mmzone.h>


MODULE_LICENSE("GPL");

extern pg_data_t    *pgdat_list[MAX_NUMNODES];
static int __init print_node_setup(void)
{
    printk(KERN_INFO "MAX_NUMNODES = %d\n", MAX_NUMNODES);
    printk(KERN_INFO "PAGE_OFFSET = 0x%lx\n",
            PAGE_OFFSET);

    /*
    int         i;
    pg_data_t   *pgdat = NULL;
    pg_data_t   **pgdata_list = pgdat_list;
    void        *high_memory = NULL;

    for(i = 0; i < MAX_NUMNODES; i++)
    {
        printk(KERN_INFO "%p\n", (void *)pgdata_list[i]);
    }

    for_each_online_pgdat(pgdat)
    {
        high_memory = max_t(void *, high_memory, __va(pgdat_end_pfn(pgdat) << PAGE_SHIFT));
        printk(KERN_INFO "%p\n", high_memory);
        printk(KERN_INFO "");
    }
    */
    return 0;
}

static void __exit print_node_cleanup(void)
{
    printk(KERN_INFO"Goodbye!\n");
}

module_init(print_node_setup);
module_exit(print_node_cleanup);
