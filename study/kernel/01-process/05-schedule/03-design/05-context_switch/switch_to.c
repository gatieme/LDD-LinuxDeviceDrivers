 //  http://lxr.free-electrons.com/source/arch/x86/include/asm/switch_to.h?v=4.6#L27
 
 //  http://lxr.free-electrons.com/source/arch/x86/include/asm/switch_to.h?v=4.6#L102

#ifdef CONFIG_X86_32

#ifdef CONFIG_CC_STACKPROTECTOR
#define __switch_canary                                                 \
        "movl %P[task_canary](%[next]), %%ebx\n\t"                      \
        "movl %%ebx, "__percpu_arg([stack_canary])"\n\t"
#define __switch_canary_oparam                                          \
        , [stack_canary] "=m" (stack_canary.canary)
#define __switch_canary_iparam                                          \
        , [task_canary] "i" (offsetof(struct task_struct, stack_canary))
#else   /* CC_STACKPROTECTOR */
#define __switch_canary
#define __switch_canary_oparam
#define __switch_canary_iparam
#endif  /* CC_STACKPROTECTOR */

/*
 * Saving eflags is important. It switches not only IOPL between tasks,
 * it also protects other tasks from NT leaking through sysenter etc.
 */
#define switch_to(prev, next, last)                                     \
do {                                                                    \
        /*                                                              \
         * Context-switching clobbers all registers, so we clobber      \
         * them explicitly, via unused output variables.                \
         * (EAX and EBP is not listed because EBP is saved/restored     \
         * explicitly for wchan access and EAX is the return value of   \
         * __switch_to())                                               \
         */                                                             \
        unsigned long ebx, ecx, edx, esi, edi;                          \
                                                                        \
        asm volatile("pushfl\n\t" /* save flags 保存就的ebp、和flags寄存器到旧进程的内核栈中*/   \
                     "pushl %%ebp\n\t"          /* save    EBP   */     \
                     "movl %%esp,%[prev_sp]\n\t"        /* save ESP  将旧进程esp保存到thread_info结构中 */ \
                     "movl %[next_sp],%%esp\n\t"        /* restore ESP 用新进程esp填写esp寄存器，此时内核栈已切换  */ \
                     "movl $1f,%[prev_ip]\n\t"  /* save EIP 将该进程恢复执行时的下条地址保存到旧进程的thread中*/     \
                     "pushl %[next_ip]\n\t"     /* restore EIP 将新进程的ip值压入到新进程的内核栈中 */     \
                     __switch_canary                                    \
                     "jmp __switch_to\n"        /* regparm call  */     \
                     "1:\t"                                             \
                     "popl %%ebp\n\t"           /* restore EBP 该进程执行，恢复ebp寄存器*/     \
                     "popfl\n"                  /* restore flags */     \
                                                                        \
                     /* output parameters */                            \
                     : [prev_sp] "=m" (prev->thread.sp),                \
                       [prev_ip] "=m" (prev->thread.ip),                \
                       "=a" (last),                                     \
                                                                        \
                       /* clobbered output registers: */                \
                       "=b" (ebx), "=c" (ecx), "=d" (edx),              \
                       "=S" (esi), "=D" (edi)                           \
                                                                        \
                       __switch_canary_oparam                           \
                                                                        \
                       /* input parameters: */                          \
                     : [next_sp]  "m" (next->thread.sp),                \
                       [next_ip]  "m" (next->thread.ip),                \
                                                                        \
                       /* regparm parameters for __switch_to(): */      \
                       [prev]     "a" (prev),                           \
                       [next]     "d" (next)                            \
                                                                        \
                       __switch_canary_iparam                           \
                                                                        \
                     : /* reloaded segment registers */                 \
                        "memory");                                      \
} while (0)

#else /* CONFIG_X86_32 */

/* frame pointer must be last for get_wchan */
#define SAVE_CONTEXT    "pushq %%rbp ; movq %%rsi,%%rbp\n\t"
#define RESTORE_CONTEXT "movq %%rbp,%%rsi ; popq %%rbp\t"

#define __EXTRA_CLOBBER  \
        , "rcx", "rbx", "rdx", "r8", "r9", "r10", "r11", \
          "r12", "r13", "r14", "r15", "flags"

#ifdef CONFIG_CC_STACKPROTECTOR
#define __switch_canary                                                   \
        "movq %P[task_canary](%%rsi),%%r8\n\t"                            \
        "movq %%r8,"__percpu_arg([gs_canary])"\n\t"
#define __switch_canary_oparam                                            \
        , [gs_canary] "=m" (irq_stack_union.stack_canary)
#define __switch_canary_iparam                                            \
        , [task_canary] "i" (offsetof(struct task_struct, stack_canary))
#else   /* CC_STACKPROTECTOR */
#define __switch_canary
#define __switch_canary_oparam
#define __switch_canary_iparam
#endif  /* CC_STACKPROTECTOR */

/*
 * There is no need to save or restore flags, because flags are always
 * clean in kernel mode, with the possible exception of IOPL.  Kernel IOPL
 * has no effect.
 */
#define switch_to(prev, next, last) \
        asm volatile(SAVE_CONTEXT                                         \
             "movq %%rsp,%P[threadrsp](%[prev])\n\t" /* save RSP */       \
             "movq %P[threadrsp](%[next]),%%rsp\n\t" /* restore RSP */    \
             "call __switch_to\n\t"                                       \
             "movq "__percpu_arg([current_task])",%%rsi\n\t"              \
             __switch_canary                                              \
             "movq %P[thread_info](%%rsi),%%r8\n\t"                       \
             "movq %%rax,%%rdi\n\t"                                       \
             "testl  %[_tif_fork],%P[ti_flags](%%r8)\n\t"                 \
             "jnz   ret_from_fork\n\t"                                    \
             RESTORE_CONTEXT                                              \
             : "=a" (last)                                                \
               __switch_canary_oparam                                     \
             : [next] "S" (next), [prev] "D" (prev),                      \
               [threadrsp] "i" (offsetof(struct task_struct, thread.sp)), \
               [ti_flags] "i" (offsetof(struct thread_info, flags)),      \
               [_tif_fork] "i" (_TIF_FORK),                               \
               [thread_info] "i" (offsetof(struct task_struct, stack)),   \
               [current_task] "m" (current_task)                          \
               __switch_canary_iparam                                     \
             : "memory", "cc" __EXTRA_CLOBBER)

#endif /* CONFIG_X86_32 */


//  http://lxr.free-electrons.com/source/arch/arm/include/asm/switch_to.h?v=4.6#L25

/*
 * For v7 SMP cores running a preemptible kernel we may be pre-empted
 * during a TLB maintenance operation, so execute an inner-shareable dsb
 * to ensure that the maintenance completes in case we migrate to another
 * CPU.
 */
#if defined(CONFIG_PREEMPT) && defined(CONFIG_SMP) && defined(CONFIG_CPU_V7)
#define __complete_pending_tlbi()       dsb(ish)
#else
#define __complete_pending_tlbi()
#endif

/*
 * switch_to(prev, next) should switch from task `prev' to `next'
 * `prev' will never be the same as `next'.  schedule() itself
 * contains the memory barrier to tell GCC not to cache `current'.
 */
extern struct task_struct *__switch_to(struct task_struct *, struct thread_info *, struct thread_info *);

#define switch_to(prev,next,last)                                       \
do {                                                                    \
        __complete_pending_tlbi();                                      \
        last = __switch_to(prev,task_thread_info(prev), task_thread_info(next));        \
} while (0)