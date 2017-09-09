// http://blog.csdn.net/bgylde/article/details/53096435
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_COUNT 64

typedef struct tagMute
{
    pthread_mutex_t  mutex;
    char file[128];
    int  line;
    int  mute_status;
    struct tagPLAYER * tag_player;
} T_MUTE;

typedef struct tagPLAYER
{
    T_MUTE *player_mux;
}T_PLAYER;

typedef struct tagLIST
{
    int     list_count;
    T_MUTE *list[MAX_COUNT];
}T_LIST;

typedef struct tagARG
{

    int shareA, shareB;
    int count;
    T_PLAYER *player_A, *player_B;
}T_ARG;

T_LIST *list;

#define PLAYER_MUXTEX_INIT(x) do {\
T_PLAYER *player = (T_PLAYER *)x;\
pthread_mutex_init(&(player->player_mux->mutex), NULL);\
memset(player->player_mux->file, 0, sizeof(player->player_mux->file));\
player->player_mux->line = 0;\
player->player_mux->mute_status = 0;\
player->player_mux->tag_player = player;\
list->list[list->list_count++] = player->player_mux;\
}while(0)

#define PLAYER_MUXTEX_LOCK(x) do {\
    T_PLAYER *player = (T_PLAYER *)x;\
    pthread_mutex_lock(&(player->player_mux->mutex));\
    strncpy(player->player_mux->file, __FILE__, sizeof(player->player_mux->file));\
    player->player_mux->line =  __LINE__;\
    player->player_mux->mute_status = 1;\
}while (0)

#define PLAYER_MUXTEX_UNLOCK(x) do {\
    T_PLAYER *player = (T_PLAYER *)x;\
    player->player_mux->mute_status = 0;\
    pthread_mutex_unlock(&player->player_mux->mutex);\
}while(0)

#define PLAYER_MUXTEX_DESTROY do {\
    while(list->list_count){\
        pthread_mutex_destroy(&list->list[list->list_count-1]->mutex);\
        free(list->list[list->list_count-1]);\
        list->list_count--;\
    }\
}while(0)

void handle_dead_lock(void)
{
    int index;
flag:
    printf("Please input unlock index: ");
    scanf("%d", &index);

    if((index >= 0) &&(index < list->list_count))
    {
        PLAYER_MUXTEX_UNLOCK(list->list[index]->tag_player);
    }
    else
    {
        printf("Input error!\n");
        goto flag;
    }
}

void *loop_detect_deadlock_thread(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    T_ARG *argDetect = (T_ARG *)arg;
    int i = 0;
    while(1)
    {
        T_MUTE *mutex = list->list[i];

        pthread_mutex_lock(&mutex->mutex);
        argDetect->count ++;
        pthread_mutex_unlock(&mutex->mutex);


        i = (++i) % list->list_count;
        pthread_testcancel();
        sleep(1);
    }
}

void *loop_detect_count_thread(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

    T_ARG *argCount = (T_ARG *)arg;
    do
    {
        int temp = argCount->count;
        sleep(5);
        if(temp == argCount->count)
        {
            printf("dead lock!\n");
            int i = 0;
            for(i = 0; i < list->list_count; i++)
            {
                if(list->list[i]->mute_status)
                {
                    printf("%d\tlock at %s:%d\n",i, list->list[i]->file, list->list[i]->line);
                }
            }
            handle_dead_lock();
        }
    }while(1);
}

void * thread_fun_A(void * arg)
{
    T_ARG *argA = (T_ARG *)arg;
    PLAYER_MUXTEX_LOCK(argA->player_A);
    printf("threadA lock   playerA, shareA = %d, shareB = %d\n", argA->shareA, argA->shareB);

//    printf("shareA = shareA(%d) + 2\n", shareA);
    argA->shareA += 2;


    sleep(1);
    PLAYER_MUXTEX_LOCK(argA->player_B);
    printf("threadA lock   playerB, shareA = %d, shareB = %d\n", argA->shareA, argA->shareB);

//    printf("shareA = shareB(%d) + 2\n", shareB);
    argA->shareA = argA->shareB + 2;


    PLAYER_MUXTEX_UNLOCK(argA->player_B);
    printf("threadA unlock playerB, shareA = %d, shareB = %d\n", argA->shareA, argA->shareB);

    PLAYER_MUXTEX_UNLOCK(argA->player_A);
    printf("threadA unlock playerA, shareA = %d, shareB = %d\n", argA->shareA, argA->shareB);
}

void * thread_fun_B(void * arg)
{
    T_ARG *argB = (T_ARG *)arg;

    PLAYER_MUXTEX_LOCK(argB->player_B);
    printf("threadB lock   playerB, shareA = %d, shareB = %d\n", argB->shareA, argB->shareB);

//    printf("shareB = shareB(%d) + 3\n", shareB);
    argB->shareB += 3;


    sleep(1);
    PLAYER_MUXTEX_LOCK(argB->player_A);
    printf("threadB lock   playerA, shareA = %d, shareB = %d\n", argB->shareA, argB->shareB);

//    printf("shareB = shareA(%d) + 3\n", shareA);
    argB->shareB = argB->shareA + 3;


    PLAYER_MUXTEX_UNLOCK(argB->player_A);
    printf("threadB unlock playerA, shareA = %d, shareB = %d\n", argB->shareA, argB->shareB);

    PLAYER_MUXTEX_UNLOCK(argB->player_B);
    printf("threadB unlock playerB, shareA = %d, shareB = %d\n", argB->shareA, argB->shareB);
}

void init(T_ARG *arg)
{
    arg->shareA = 0;
    arg->shareB = 0;
    arg->count = 0;

    list = (T_LIST *)malloc(sizeof(T_LIST));
    list->list_count = 0;

    arg->player_A = (T_PLAYER *)malloc(sizeof(T_PLAYER));
    arg->player_B = (T_PLAYER *)malloc(sizeof(T_PLAYER));

    arg->player_A->player_mux = (T_MUTE *)malloc(sizeof(T_MUTE));
    arg->player_B->player_mux = (T_MUTE *)malloc(sizeof(T_MUTE));

    PLAYER_MUXTEX_INIT(arg->player_A);
    PLAYER_MUXTEX_INIT(arg->player_B);
}

void free_player(T_ARG *arg)
{
    free(arg->player_A);
    free(arg->player_B);

    free(arg);

    free(list);
}
int main(int argc, char * argv[])
{
    T_ARG *arg = (T_ARG *)malloc(sizeof(T_ARG));

    init(arg);

    int ret = -1;

    pthread_t threadA, threadB, thread_detect_deadlock, thread_detect_count;
    ret = pthread_create(&threadA, NULL, thread_fun_A, (void *)arg);
    if(ret != 0)
    {
        printf("Pthread create error!\n");
    }

    ret = pthread_create(&threadB, NULL, thread_fun_B, (void *)arg);
    if(ret != 0)
    {
        printf("Pthread create error!\n");
    }

    ret = pthread_create(&thread_detect_deadlock, NULL, loop_detect_deadlock_thread, (void *)arg);
    if(ret != 0)
    {
        printf("Pthread create error!\n");
    }

    ret = pthread_create(&thread_detect_count, NULL, loop_detect_count_thread, (void *)arg);
    if(ret != 0)
    {
        printf("Pthread create error!\n");
    }

    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);

    pthread_cancel(thread_detect_count);
    pthread_cancel(thread_detect_deadlock);
    pthread_join(thread_detect_count, NULL);
    pthread_join(thread_detect_deadlock, NULL);

    PLAYER_MUXTEX_DESTROY;

    printf("shareA: %d, shareB: %d\n", arg->shareA, arg->shareB);
    free_player(arg);
}
