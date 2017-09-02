/*************************************************************************
    > File Name: test_container_of.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: Tue 18 Jul 2017 09:06:55 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>

/////////////////////
// container_of
/////////////////////
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)


#define container_of(ptr, type, member)                                 \
({                                                                      \
        const typeof( ((type *)0)->member) *m_ptr = (ptr);              \
        (type *)( (char *)m_ptr - offsetof(type, member) );             \
})


typedef struct CTest
{
        int     m_a;
        int     m_b;
}CTest;


/////////////////////
// test main
/////////////////////
int main(void)
{
    CTest cc = {
        .m_a = 10,
        .m_b = 20,
    };

    int *pa = &(cc.m_a);
    CTest *pc = container_of(pa, CTest, m_a);
    printf("%p %p\n", &cc, pc);
    printf("%d %d\n", pc->m_a, pc->m_b);

    return 0;
}
