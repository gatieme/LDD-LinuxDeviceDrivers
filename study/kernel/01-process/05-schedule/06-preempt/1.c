#include <stdio.h>
#include <stdlib.h>



struct ThreadInfo
{
    int preempt_count;   
}tiv;

struct ThreadInfo *ti = &tiv;
/*
 * Increment/decrement the preempt count.
 */
#ifdef CONFIG_PREEMPT_COUNT
    .macro  inc_preempt_count, ti, tmp
    ldr     \tmp, [\ti, #TI_PREEMPT]    @ get preempt count
    add     \tmp, \tmp, #1          @ increment it
    str     \tmp, [\ti, #TI_PREEMPT]
    .endm

    .macro  dec_preempt_count, ti, tmp
    ldr     \tmp, [\ti, #TI_PREEMPT]    @ get preempt count
    sub     \tmp, \tmp, #1          @ decrement it
    str     \tmp, [\ti, #TI_PREEMPT]
    .endm

    .macro  dec_preempt_count_ti, ti, tmp
    get_thread_info \ti
    dec_preempt_count \ti, \tmp
    .endm
#else
    .macro  inc_preempt_count, ti, tmp
    .endm

    .macro  dec_preempt_count, ti, tmp
    .endm

    .macro  dec_preempt_count_ti, ti, tmp
    .endm
#endif

int main(void)
{
    ti->preempt_count = 0;
    inc_preempt_count(ti);

}