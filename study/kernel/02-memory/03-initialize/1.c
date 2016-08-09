#ifndef CONFIG_NO_BOOTMEM
/*
* node_bootmem_map is a map pointer - the bits represent all physical 
* memory pages (including holes) on the node.
*/
typedef struct bootmem_data {
       unsigned long node_min_pfn;
       unsigned long node_low_pfn;
       void *node_bootmem_map;
       unsigned long last_end_off;
       unsigned long hint_idx;
       struct list_head list;
} bootmem_data_t;