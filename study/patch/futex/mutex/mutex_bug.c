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


 #include <linux/futex.h>
       #include <sys/time.h>

#define futex(addr1, op, val, rel, addr2, val3) \
        syscall(SYS_futex, addr1, op, val, rel, addr2, val3)

long lock;
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
                futex(&lock, FUTEX_WAIT, 1, NULL, NULL, 0);

                printf("tid = %d, count = %ld\n", gettid( ), count);
                count++;
                usleep(curr_arg->sleeptime);

                futex(&lock, FUTEX_WAKE, 0, NULL, NULL, 0);

        }
}

int main(int argc,char** argv){
        pthread_t id1;
        pthread_t id2;

        struct args arg1 = { .sleeptime = 1000000, .cpu = 0 };
        struct args arg2 = { .sleeptime = 1000000, .cpu = 0 };


        lock = 1;

        printf("MAIN pid = %d\n", getpid( ));


        pthread_create(&id1, NULL, print_msg, &arg1);
        pthread_create(&id2, NULL, print_msg, &arg2);

        pthread_join(id1, NULL);
        pthread_join(id2, NULL);



        return EXIT_SUCCESS;
}



