struct net_device {
    char            name[IFNAMSIZ];
    struct hlist_node       name_hlist;
    char            *ifalias;
    /*
     *      I/O specific fields
     *      FIXME: Merge these and struct ifmap into one
     */
    unsigned long       mem_end;
    unsigned long       mem_start;
    unsigned long       base_addr;
    int             irq;

    atomic_t        carrier_changes;

    /*
     *      Some hardware also needs these fields (state,dev_list,
     *      napi_list,unreg_list,close_list) but they are not
     *      part of the usual set specified in Space.c.
     */

    unsigned long       state;

    struct list_head    dev_list;
    struct list_head    napi_list;
    struct list_head    unreg_list;
    struct list_head    close_list;
    struct list_head    ptype_all;
    struct list_head    ptype_specific;

    struct {
        struct list_head upper;
        struct list_head lower;
    } adj_list;

    struct {
        struct list_head upper;
        struct list_head lower;
    } all_adj_list;

    netdev_features_t       features;
    netdev_features_t       hw_features;
    netdev_features_t       wanted_features;
    netdev_features_t       vlan_features;
    netdev_features_t       hw_enc_features;
    netdev_features_t       mpls_features;
    netdev_features_t       gso_partial_features;

    int             ifindex;
    int             group;

    struct net_device_stats stats;

    atomic_long_t       rx_dropped;
    atomic_long_t       tx_dropped;
    atomic_long_t       rx_nohandler;

#ifdef CONFIG_WIRELESS_EXT
    const struct iw_handler_def *wireless_handlers;
    struct iw_public_data   *wireless_data;
#endif
    const struct net_device_ops *netdev_ops;
    const struct ethtool_ops *ethtool_ops;
#ifdef CONFIG_NET_SWITCHDEV
    const struct switchdev_ops *switchdev_ops;
#endif
#ifdef CONFIG_NET_L3_MASTER_DEV
    const struct l3mdev_ops *l3mdev_ops;
#endif

    const struct header_ops *header_ops;

    unsigned int        flags;
    unsigned int        priv_flags;

    unsigned short      gflags;
    unsigned short      padded;

    unsigned char       operstate;
    unsigned char       link_mode;

    unsigned char       if_port;
    unsigned char       dma;

    unsigned int        mtu;
    unsigned short      type;
    unsigned short      hard_header_len;

    unsigned short      needed_headroom;
    unsigned short      needed_tailroom;

    /* Interface address info. */
    unsigned char       perm_addr[MAX_ADDR_LEN];
    unsigned char       addr_assign_type;
    unsigned char       addr_len;
    unsigned short      neigh_priv_len;
    unsigned short      dev_id;
    unsigned short      dev_port;
    spinlock_t          addr_list_lock;
    unsigned char       name_assign_type;
    bool            uc_promisc;
    struct netdev_hw_addr_list      uc;
    struct netdev_hw_addr_list      mc;
    struct netdev_hw_addr_list      dev_addrs;

#ifdef CONFIG_SYSFS
    struct kset         *queues_kset;
#endif
    unsigned int        promiscuity;
    unsigned int        allmulti;


    /* Protocol-specific pointers */

#if IS_ENABLED(CONFIG_VLAN_8021Q)
    struct vlan_info __rcu  *vlan_info;
#endif
#if IS_ENABLED(CONFIG_NET_DSA)
    struct dsa_switch_tree  *dsa_ptr;
#endif
#if IS_ENABLED(CONFIG_TIPC)
    struct tipc_bearer __rcu *tipc_ptr;
#endif
    void            *atalk_ptr;
    struct in_device __rcu  *ip_ptr;
    struct dn_dev __rcu     *dn_ptr;
    struct inet6_dev __rcu  *ip6_ptr;
    void            *ax25_ptr;
    struct wireless_dev     *ieee80211_ptr;
    struct wpan_dev     *ieee802154_ptr;
#if IS_ENABLED(CONFIG_MPLS_ROUTING)
    struct mpls_dev __rcu   *mpls_ptr;
#endif

/*
 * Cache lines mostly used on receive path (including eth_type_trans())
 */
    unsigned long       last_rx;

    /* Interface address info used in eth_type_trans() */
    unsigned char       *dev_addr;

#ifdef CONFIG_SYSFS
    struct netdev_rx_queue  *_rx;

    unsigned int        num_rx_queues;
    unsigned int        real_num_rx_queues;
#endif

    unsigned long       gro_flush_timeout;
    rx_handler_func_t __rcu *rx_handler;
    void __rcu          *rx_handler_data;

#ifdef CONFIG_NET_CLS_ACT
    struct tcf_proto __rcu  *ingress_cl_list;
#endif
    struct netdev_queue __rcu *ingress_queue;
#ifdef CONFIG_NETFILTER_INGRESS
    struct list_head    nf_hooks_ingress;
#endif

    unsigned char       broadcast[MAX_ADDR_LEN];
#ifdef CONFIG_RFS_ACCEL
    struct cpu_rmap     *rx_cpu_rmap;
#endif
    struct hlist_node       index_hlist;

/*
 * Cache lines mostly used on transmit path
 */
    struct netdev_queue     *_tx ____cacheline_aligned_in_smp;
    unsigned int        num_tx_queues;
    unsigned int        real_num_tx_queues;
    struct Qdisc        *qdisc;
    unsigned long       tx_queue_len;
    spinlock_t          tx_global_lock;
    int             watchdog_timeo;

#ifdef CONFIG_XPS
    struct xps_dev_maps __rcu *xps_maps;
#endif
#ifdef CONFIG_NET_CLS_ACT
    struct tcf_proto __rcu  *egress_cl_list;
#endif
#ifdef CONFIG_NET_SWITCHDEV
    u32             offload_fwd_mark;
#endif

    /* These may be needed for future network-power-down code. */
    struct timer_list       watchdog_timer;

    int __percpu        *pcpu_refcnt;
    struct list_head    todo_list;

    struct list_head    link_watch_list;

    enum { NETREG_UNINITIALIZED=0,
           NETREG_REGISTERED,       /* completed register_netdevice */
           NETREG_UNREGISTERING,    /* called unregister_netdevice */
           NETREG_UNREGISTERED,     /* completed unregister todo */
           NETREG_RELEASED,     /* called free_netdev */
           NETREG_DUMMY,        /* dummy device for NAPI poll */
    } reg_state:8;

    bool dismantle;

    enum {
        RTNL_LINK_INITIALIZED,
        RTNL_LINK_INITIALIZING,
    } rtnl_link_state:16;

    void (*destructor)(struct net_device *dev);

#ifdef CONFIG_NETPOLL
    struct netpoll_info __rcu       *npinfo;
#endif

    possible_net_t          nd_net;

    /* mid-layer private */
    union {
        void                    *ml_priv;
        struct pcpu_lstats __percpu         *lstats;
        struct pcpu_sw_netstats __percpu    *tstats;
        struct pcpu_dstats __percpu         *dstats;
        struct pcpu_vstats __percpu         *vstats;
    };

    struct garp_port __rcu  *garp_port;
    struct mrp_port __rcu   *mrp_port;

    struct device       dev;
    const struct attribute_group *sysfs_groups[4];
    const struct attribute_group *sysfs_rx_queue_group;

    const struct rtnl_link_ops *rtnl_link_ops;

    /* for setting kernel sock attribute on TCP connection setup */
#define GSO_MAX_SIZE        65536
    unsigned int        gso_max_size;
#define GSO_MAX_SEGS        65535
    u16             gso_max_segs;

#ifdef CONFIG_DCB
    const struct dcbnl_rtnl_ops *dcbnl_ops;
#endif
    u8              num_tc;
    struct netdev_tc_txq    tc_to_txq[TC_MAX_QUEUE];
    u8              prio_tc_map[TC_BITMASK + 1];

#if IS_ENABLED(CONFIG_FCOE)
    unsigned int        fcoe_ddp_xid;
#endif
#if IS_ENABLED(CONFIG_CGROUP_NET_PRIO)
    struct netprio_map __rcu *priomap;
#endif
    struct phy_device       *phydev;
    struct lock_class_key   *qdisc_tx_busylock;
    bool            proto_down;
};