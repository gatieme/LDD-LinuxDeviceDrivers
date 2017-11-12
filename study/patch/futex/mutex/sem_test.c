// http://blog.csdn.net/jianchaolv/article/details/7544316
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>


sem_t sem_a;
void *task1();

int main(void){
        int ret = 0;
        pthread_t thrd1;
        pthread_t thrd2;

	printf("pid = %d\n", getpid( ));
	sem_init(&sem_a,0,1);
	ret=pthread_create(&thrd1,NULL,task1,NULL); //创建子线程
	ret=pthread_create(&thrd2,NULL,task1,NULL); //创建子线程

	pthread_join(thrd1,NULL); //等待子线程结束
	pthread_join(thrd2,NULL); //等待子线程结束

}

void *task1()
{
	int sval = 0;
	while(1)
	{
		sem_wait(&sem_a); //持有信号量
		sleep(1); //do_nothing
		sem_getvalue(&sem_a,&sval);
		printf("tid = %ld, sem value = %d\n", pthread_self( ), sval);
		sem_post(&sem_a); //释放信号量
	}
}
