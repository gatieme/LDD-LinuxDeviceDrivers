

//  http://lxr.free-electrons.com/source/kernel/sched/fair.c?v=4.6#L457

static void update_min_vruntime(struct cfs_rq *cfs_rq)
{
    /*  初始化vruntime的值, 相当于如下的代码
    if (cfs_rq->curr != NULL)
        vruntime = cfs_rq->curr->vruntime;
    else
        vruntime = cfs_rq->min_vruntime;
    */
    u64 vruntime = cfs_rq->min_vruntime;

    if (cfs_rq->curr)
        vruntime = cfs_rq->curr->vruntime;


    /*  检测红黑树是都有最左的节点, 即是否有进程在树上等待调度
     *  cfs_rq->rb_leftmost(struct rb_node *)存储了进程红黑树的最左节点
     *  这个节点存储了即将要被调度的结点  
     *  */
    if (cfs_rq->rb_leftmost)
    {
        /*  获取最左结点的调度实体信息se, se中存储了其vruntime
         *  rb_leftmost的vruntime即树中所有节点的vruntiem中最小的那个  */
        struct sched_entity *se = rb_entry(cfs_rq->rb_leftmost,
                           struct sched_entity,
                           run_node);
        /*  如果就绪队列上没有curr进程
         *  则vruntime设置为树种最左结点的vruntime
         *  否则设置vruntiem值为cfs_rq->curr->vruntime和se->vruntime的最小值
         */
        if (!cfs_rq->curr)  /*  此时vruntime的原值为cfs_rq->min_vruntime*/
            vruntime = se->vruntime;
        else                /* 此时vruntime的原值为cfs_rq->curr->vruntime*/
            vruntime = min_vruntime(vruntime, se->vruntime);
    }

    /* ensure we never gain time by being placed backwards. 
     * 为了保证min_vruntime单调不减
     * 只有在vruntime超出的cfs_rq->min_vruntime的时候才更新
     */
    cfs_rq->min_vruntime = max_vruntime(cfs_rq->min_vruntime, vruntime);
#ifndef CONFIG_64BIT
    smp_wmb();
    cfs_rq->min_vruntime_copy = cfs_rq->min_vruntime;
#endif
}

以上代码等价于如下

if (cfs_rq->curr != NULL)
    vruntime = cfs_rq->curr->vruntime;
else if(cfs_rq->curr == NULL)
    vruntime = cfs_rq->min_vruntime;

if(cfs_rq->rb_leftmost != NULL)
{
    if (cfs_rq->curr == NULL)  /*  此时vruntime的原值为cfs_rq->min_vruntime*/
        vruntime = se->vruntime;
    else                /* 此时vruntime的原值为cfs_rq->curr->vruntime*/
        vruntime = min_vruntime(vruntime, se->vruntime);
}
cfs_rq->min_vruntime = max_vruntime(cfs_rq->min_vruntime, vruntime);


进一步可以等价于, 如下的代码

if (cfs_rq->curr != NULL cfs_rq->rb_leftmost == NULL)
    vruntime = cfs_rq->curr->vruntime;
else if(cfs_rq->curr == NULL && cfs_rq->rb_leftmost != NULL)
        vruntime = cfs_rq->rb_leftmost->se->vruntime;
else if (cfs_rq->curr != NULL cfs_rq->rb_leftmost != NULL)
    vruntime = min(cfs_rq->curr->vruntime, cfs_rq->rb_leftmost->se->vruntime);
else if(cfs_rq->curr == NULL cfs_rq->rb_leftmost == NULL)
    vruntime = cfs_rq->min_vruntime;
cfs_rq->min_vruntime = max_vruntime(cfs_rq->min_vruntime, vruntime);
