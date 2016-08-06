#include <linux/module.h>
#include <linux/init.h>

#include <linux/sched.h>

#include <linux/mm.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <linux/mmzone.h>


MODULE_LICENSE("GPL");

void print_node(struct pglist_data   *node)
{
    printk("address = %p\n", node);
    printk("node id = %d\n", node->node_id);
    printk("nr_zones = %d\n", node->nr_zones);
    printk("pgdat_next = %p\n", node->pgdat_next);

    printk("nod present pages = %d\n", node->node_present_pages);
    printk("nod spanned pages = %d\n", node->node_spanned_pages);
}



extern pg_data_t    *pgdat_list[MAX_NUMNODES];
static int __init print_node_setup(void)
{
    printk(KERN_INFO "MAX_NUMNODES = %d\n", MAX_NUMNODES);
    printk(KERN_INFO "PAGE_OFFSET = 0x%lx\n",
            PAGE_OFFSET);


    int         i;
    pg_data_t   *node = NULL;
#if 0
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
#endif

#ifndef CONFIG_NEED_MULTIPLE_NODES
    struct pglist_data *pgdata = &contig_page_data;
    printk("%d\n", pgdata->node_id);
#else
    printk("NODES_SHIFT = %d, MAX_NUMNODES = %d\n", NODES_SHIFT, MAX_NUMNODES);
    for(i = 0; i < MAX_NUMNODES; i++)
    {
        node = NODE_DATA(i);
        if(node != NULL)
        {
            print_node(node);
        }
        else
        {
            break;
        }
    }


    return 0;
#endif
}




static void __exit print_node_cleanup(void)
{
    printk(KERN_INFO"Goodbye!\n");
}

module_init(print_node_setup);
module_exit(print_node_cleanup);
