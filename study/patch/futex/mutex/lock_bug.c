/*************************************************************************
    > File Name: mutex_bug.c
    > Author: GatieMe
    > Mail: gatieme@163.com
    > Created Time: Sat 09 Sep 2017 11:39:26 AM CST
 ************************************************************************/

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>

#include <pthread.h>            /* For pthread  */

#include <unistd.h>             /* */

#include <stdio.h>
#include <stdlib.h>

#if defined(MUTEX)
        pthread_mutex_t                 lock;
        #define LOCK_INIT( )            pthread_mutex_init(&lock, NULL)
        #define LOCK( )                 pthread_mutex_lock(&lock)
        #define UNLOCK( )               pthread_mutex_unlock(&lock)
        #define LOCK_DESTROY( )         pthread_mutex_destroy(&lock)
#elif defined(SEM)
        #include <semaphore.h>
        sem_t                           lock;
        #define LOCK_INIT( )            sem_init(&lock, 0, 1)
        #define LOCK( )                 sem_wait(&lock)
        #define UNLOCK( )               sem_post(&lock)
        #define LOCK_DESTROY( )         sem_destroy(&lock)
#else
        #define LOCK_INIT( )
        #define LOCK( )
        #define UNLOCK( )
        #define LOCK_DESTROY( )
#endif


#if 0
#include <linux/unistd.h>
pid_t gettid(void)
{
    return syscall(__NR_gettid);
}
#endif
#include <sys/syscall.h>

pid_t gettid(void)
{
    return syscall(SYS_gettid);
}
struct args
{
        unsigned long   sleeptime;
        int             cpu;
};

void *print_msg(void *arg){

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
                printf("tid = %d affine to CPU %d\n", gettid( ), curr_arg->cpu);
        }

        while( 1 )
        {
                LOCK( );

                printf("tid = %d, count = %ld\n", gettid( ), count);
                count++;
                usleep(curr_arg->sleeptime);

                UNLOCK( );
        }
}

int main(int argc,char** argv){
        pthread_t id1;
        pthread_t id2;

        struct args arg1 = { .sleeptime = 10000000, .cpu = 0 };
        struct args arg2 = { .sleeptime = 10000000, .cpu = 0 };

        LOCK_INIT( );


        printf("MAIN pid = %d\n", getpid( ));
	getchar( );

        pthread_create(&id1, NULL, print_msg, &arg1);
        pthread_create(&id2, NULL, print_msg, &arg2);

	pthread_join(id1, NULL);
        pthread_join(id2, NULL);

        LOCK_DESTROY( );

        return EXIT_SUCCESS;
}



