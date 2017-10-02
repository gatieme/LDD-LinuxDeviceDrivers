// 2010年 07月 28日 星期三 13:01:43 CST
// author: 李小丹(Li Shao Dan) 字 殊恒(shuheng)
// K.I.S.S
// S.P.O.T
// linux-2.6.XX/Document/
// linux-2.6.xx/kernel/futex.c
// http://blog.csdn.net/cybertan/article/details/8096863
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <linux/futex.h>




#include <sched.h>
#include <sys/syscall.h>

#define  gettid( )    syscall(SYS_gettid)

#define futex(addr1, op, val, rel, addr2, val3)    \
    syscall(SYS_futex, addr1, op, val, rel, addr2, val3)

typedef struct futex_t {
    int wake;
    int lock;
    int wlock;
}futex_t;

inline static void futex_init(futex_t *);
inline static int futex_wake(futex_t *);
inline static int futex_wait(futex_t *);
inline static int futex_lock(futex_t *);
inline static int futex_unlock(futex_t *);


void *print_msg(void *arg);


int main()
{
	struct futex_t ftx;
	pthread_t tid[2];
	futex_init(&ftx);
	futex_wake(&ftx);


	pthread_create(&tid[0], 0, print_msg, (void *)&ftx);
	pthread_create(&tid[1], 0, print_msg, (void *)&ftx);

	pthread_join(tid[0], 0);
	pthread_join(tid[1], 0);
	return 0;
}

void *print_msg(void *arg){

	struct futex_t *ftx = (struct futex_t *)arg;
        unsigned long   count = 0;
        struct args     *curr_arg = (struct args*)arg;
        cpu_set_t       mask;

        printf("tid = %d(%ld) START\n", gettid( ), pthread_self( ));


        CPU_ZERO(&mask);
        //CPU_SET(curr_arg->cpu, &mask);
        CPU_SET(0, &mask);

        if (pthread_setaffinity_np(pthread_self( ), sizeof(mask), &mask) < 0) {
		perror("sched_setaffinity");
        }
        else
        {
                printf("tid = %d affine to CPU %d\n", gettid( ), 0);
        }

        while( 1 )
        {
                futex_wait(&ftx);

                printf("tid = %d, count = %ld\n", gettid( ), count);
                count++;
                usleep(0);

                futex_wake(&ftx);
        }
}


inline static void futex_init(futex_t *ftx)
{
	ftx->lock = 0;
	ftx->wake = 0;
	ftx->wlock = 0;
}

inline static int futex_wake(futex_t *ftx)
{
	__sync_fetch_and_add(&ftx->wake, 1);
	//__sync_lock_test_and_set(&ftx->wake, 1);
	return futex(&ftx->wake, FUTEX_WAKE, 1, 0, 0, 0);
}

inline static int futex_wait(futex_t *ftx)
{
	futex(&ftx->wlock, FUTEX_LOCK_PI, 0, 0, 0, 0);
	int ret = futex(&ftx->wake, FUTEX_WAIT, 0, 0, 0, 0);
	__sync_fetch_and_sub(&ftx->wake, 1);
	futex(&ftx->wlock, FUTEX_UNLOCK_PI, 0, 0, 0, 0);
	return (ret && errno == EWOULDBLOCK ? 1 : ret);
}

inline static int futex_lock(futex_t *ftx)
{
	return futex(&ftx->lock, FUTEX_LOCK_PI, 0, 0, 0, 0);
}

inline static int futex_unlock(futex_t *ftx)
{
	return futex(&ftx->lock, FUTEX_UNLOCK_PI, 0, 0, 0, 0);
}
