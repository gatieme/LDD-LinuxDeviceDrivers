enum node_states {
    N_POSSIBLE,         /* The node could become online at some point */
    N_ONLINE,           /* The node is online */
    N_NORMAL_MEMORY,    /* The node has regular memory */
#ifdef CONFIG_HIGHMEM
    N_HIGH_MEMORY,      /* The node has regular or high memory */
#else
    N_HIGH_MEMORY = N_NORMAL_MEMORY,
#endif
#ifdef CONFIG_MOVABLE_NODE
    N_MEMORY,           /* The node has memory(regular, high, movable) */
#else
    N_MEMORY = N_HIGH_MEMORY,
#endif
    N_CPU,      /* The node has one or more cpus */
    NR_NODE_STATES
};