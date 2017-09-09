#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/sysinfo.h>
#include<unistd.h>
 
#define __USE_GNU
#include<sched.h>
#include<ctype.h>
#include<string.h>
#include<pthread.h>
#define THREAD_MAX_NUM 100  //1个CPU内的最多进程数
 
int num=0;  //cpu中核数
void* threadFun(void* arg)  //arg  传递线程标号（自己定义）
{
         cpu_set_t mask;  //CPU核的集合
         cpu_set_t get;   //获取在集合中的CPU
         int *a = (int *)arg; 
         printf("the a is:%d\n",*a);  //显示是第几个线程
         CPU_ZERO(&mask);    //置空
         CPU_SET(*a,&mask);   //设置亲和力值
         if (sched_setaffinity(0, sizeof(mask), &mask) == -1)//设置线程CPU亲和力
         {
                   printf("warning: could not set CPU affinity, continuing...\n");
         }
         while (1)
         {
                   CPU_ZERO(&get);
                   if (sched_getaffinity(0, sizeof(get), &get) == -1)//获取线程CPU亲和力
                   {
                            printf("warning: cound not get thread affinity, continuing...\n");
                   }
                   int i;
                   for (i = 0; i < num; i++)
                   {
                            if (CPU_ISSET(i, &get))//判断线程与哪个CPU有亲和力
                            {
                                     printf("this thread %d is running processor : %d\n", i,i);
                            }
                   }
         }
 
         return NULL;
}
 
int main(int argc, char* argv[])
{
         num = sysconf(_SC_NPROCESSORS_CONF);  //获取核数
         pthread_t thread[THREAD_MAX_NUM];
         printf("system has %i processor(s). \n", num);
         int tid[THREAD_MAX_NUM];
         int i;
         for(i=0;i<num;i++)
         {
                   tid[i] = i;  //每个线程必须有个tid[i]
                   pthread_create(&thread[0],NULL,threadFun,(void*)&tid[i]);
         }
         for(i=0; i< num; i++)
         {
                   pthread_join(thread[i],NULL);//等待所有的线程结束，线程为死循环所以CTRL+C结束
         }
         return 0;
}