/*
 * context_switch - switch to the new MM and the new thread's register state.
 */
static __always_inline struct rq *
context_switch(struct rq *rq, struct task_struct *prev,
           struct task_struct *next)
{
    struct mm_struct *mm, *oldmm;

    prepare_task_switch(rq, prev, next);

    mm = next->mm;
    oldmm = prev->active_mm;
    /*
     * For paravirt, this is coupled with an exit in switch_to to
     * combine the page table reload and the switch backend into
     * one hypercall.
     */
    arch_start_context_switch(prev);

    /*  如果next是内核线程，则线程使用prev所使用的地址空间
     *  schedule( )函数把该线程设置为懒惰TLB模式
     *  内核线程并不拥有自己的页表集(task_struct->mm = NULL)
     *  它使用一个普通进程的页表集
     *  不过，没有必要使一个用户态线性地址对应的TLB表项无效
     *  因为内核线程不访问用户态地址空间。
    */
    if (!mm)        /*  内核线程无虚拟地址空间, mm = NULL*/
    {
        /*  内核线程的active_mm为上一个进程的mm
         *  注意此时如果prev也是内核线程, 
         *  则oldmm为NULL, 即next->active_mm也为NULL  */
        next->active_mm = oldmm;    
        /*  增加mm的引用计数  */
        atomic_inc(&oldmm->mm_count);
        /*  通知底层体系结构不需要切换虚拟地址空间的用户部分
         *  这种加速上下文切换的技术称为惰性TBL  */
        enter_lazy_tlb(oldmm, next);
    }
    else            /*  不是内核线程, 则需要切切换虚拟地址空间  */
        switch_mm(oldmm, mm, next);

    /*  如果prev是内核线程或正在退出的进程
     *  就重新设置prev->active_mm
     *  然后把指向prev内存描述符的指针保存到运行队列的prev_mm字段中
     */
    if (!prev->mm)
    {
        /*  将prev的active_mm赋值和为空  */
        prev->active_mm = NULL;
        /*  更新运行队列的prev_mm成员  */
        rq->prev_mm = oldmm;
    }
    /*
     * Since the runqueue lock will be released by the next
     * task (which is an invalid locking op but in the case
     * of the scheduler it's an obvious special-case), so we
     * do an early lockdep release here:
     */
    lockdep_unpin_lock(&rq->lock);
    spin_release(&rq->lock.dep_map, 1, _THIS_IP_);

    /* Here we just switch the register state and the stack. 
     * 切换进程的执行环境, 包括堆栈和寄存器
     * 同时返回上一个执行的程序
     * 相当于prev = witch_to(prev, next)  */
    switch_to(prev, next, prev);
    
    /*  switch_to之后的代码只有在
     *  当前进程再次被选择运行(恢复执行)时才会运行
     *  而此时当前进程恢复执行时的上一个进程可能跟参数传入时的prev不同
     *  甚至可能是系统中任意一个随机的进程
     *  因此switch_to通过第三个参数将此进程返回
     */
    

    /*  路障同步, 一般用编译器指令实现
     *  确保了switch_to和finish_task_switch的执行顺序
     *  不会因为任何可能的优化而改变  */
    barrier();  

    /*  进程切换之后的处理工作  */
    return finish_task_switch(prev);
}