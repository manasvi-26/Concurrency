#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <wait.h>
#include <sys/shm.h>
#include<string.h>

struct stage
{
    int type;
    int index;
    int state;
    int sing;

}*stages;

struct performer
{
    int index;
    int state; //0 wait -1 leave 1 tshirt
    char name[50];
    int stage;   //can be of 3 types : (0 for acoustic),(1 for elctric),(2 can use both stages),(3 for singer)
    int arrival_time;
    int flag;
    char instrument;

}*performers;

pthread_mutex_t *mutex_stage,*flag_lock;
pthread_cond_t *cond_stage;

pthread_t *performer_newthread;

sem_t acoustic_musician,electric_musician,total_singer,coordinators;

int k,a,e,t1,t2,t,tot,c;


void type(int index,char c)
{
    switch (c)
    {
        case 'p': performers[index].stage = 2;break;
        case 'g': performers[index].stage = 2;break;
        case 'v': performers[index].stage = 0;break;
        case 'b': performers[index].stage = 1;break;
        case 's': performers[index].stage = 3;break;
    }
    return;
}

void Error(int index)
{
    printf( "\033[1;31m\n"
            "%s leaves Srujana due to impatience\n",
            performers[index].name);
    
    fflush(stdout);
    performers[index].state = -1;
}

void Tshirt(int index)
{
    if(c == 0)return;
    sem_wait(&coordinators);

    printf( "\033[1;34m\n"
            "%s is collecting a Tshirt\n",performers[index].name);
    fflush(stdout);

    sem_post(&coordinators);
}

void perform_musician(int index, int type)
{
    int performing_time = t1 + rand()%(t2 + 1 - t1);

    for(int i=0;i<tot;i++)
    {
        if(stages[i].state == 0 && stages[i].type == type)
        {
            if(pthread_mutex_trylock(&mutex_stage[i]))continue;
            
            stages[i].state = 1;

            pthread_mutex_unlock(&mutex_stage[i]);

            char string[20];
            if(type == 0)strcpy(string,"acoustic");
            else strcpy(string,"electric");


            printf( "\033[1;36m\n"
                    "%s is performing %c at %s stage %d for %d seconds\n",
                    performers[index].name,performers[index].instrument,string,stages[i].index,performing_time);


            fflush(stdout);
            sleep(performing_time);
            if(stages[i].state == 3)
            {
                printf("\033[1;33m\n"
                        "%s joined %s, peformance extended by 2 seconds\n",
                        performers[stages[i].sing].name,performers[index].name);
                
                fflush(stdout);
                sleep(2);
                
                pthread_mutex_lock(&mutex_stage[i]);

                stages[i].state = 0;
                stages[i].sing = -1;

                pthread_cond_signal(&cond_stage[i]);
                pthread_mutex_unlock(&mutex_stage[i]);

                printf( "\033[1;32m\n"
                "%s's performance at %s stage %d has ended\n",
                performers[index].name, string,stages[i].index);
        
                fflush(stdout);
                performers[index].state = 1;
                
                break;
            }
            
            pthread_mutex_lock(&mutex_stage[i]);
            stages[i].state = 0;
            pthread_mutex_unlock(&mutex_stage[i]);

            performers[index].state = 1; 
            
            printf( "\033[1;32m\n"
                "%s's performance at %s stage %d has ended\n",
                performers[index].name, string,stages[i].index);
        
            fflush(stdout);
            break;
        }           
    }  
}

void* musician_newthread(void* arg)
{
    int index = *((int*)arg);
    struct timespec ts;

    if(clock_gettime(CLOCK_REALTIME,&ts) == -1)return NULL;

    ts.tv_sec += t;
    int s;
    while ((s = sem_timedwait(&electric_musician, &ts)) == -1 && errno == EINTR)continue;


    if(performers[index].flag)
    {
        if(s == -1)return NULL;
    
        sem_post(&electric_musician);
        return NULL;
    }

    pthread_mutex_lock(&flag_lock[index]);
    performers[index].flag = 1;
    pthread_mutex_unlock(&flag_lock[index]);

    perform_musician(index,1);

    return NULL;
}

void* musician_init(void *arg)
{
    int index = *((int*)arg);

    sleep(performers[index].arrival_time);
    printf( "\033[1;35m\n"
            "Musician %s has arrived with instrument %c\n",
            performers[index].name,performers[index].instrument);
    fflush(stdout);
    

    struct timespec ts;

    if(clock_gettime(CLOCK_REALTIME,&ts) == -1)return NULL;

    ts.tv_sec +=t;
    int s;
    
    if(performers[index].stage == 0)
    {
        while ((s = sem_timedwait(&acoustic_musician, &ts)) == -1 && errno == EINTR)continue;
        if(s == -1)
        {
            if(errno == ETIMEDOUT)Error(index);
            else perror("sem_timedwait");
        }
        else
        {
            perform_musician(index,0);
            sem_post(&acoustic_musician);
        }
    }

    else if(performers[index].stage == 1)
    {
        while ((s = sem_timedwait(&electric_musician, &ts)) == -1 && errno == EINTR)continue;
        if(s == -1)
        {
            if(errno == ETIMEDOUT)Error(index);
            else perror("sem_timedwait");
        }
        else
        {
            perform_musician(index,1);
            sem_post(&electric_musician);
        }
    }

    else if(performers[index].stage == 2)
    {
           
        pthread_create(&performer_newthread[index],NULL,musician_newthread,(void*)(&index));

        while ((s = sem_timedwait(&acoustic_musician, &ts)) == -1 && errno == EINTR)continue;

        if(performers[index].flag == 0) 
        {
            if(s != -1)
            {   
                pthread_mutex_lock(&flag_lock[index]);
                performers[index].flag = 1;
                pthread_mutex_unlock(&flag_lock[index]);

                perform_musician(index,0);

                sem_post(&acoustic_musician);
            }
            else
            {
                if(errno == ETIMEDOUT)Error(index);
            }
        }
        else
        {
            if(s != -1)sem_post(&acoustic_musician);
        }

        pthread_join(performer_newthread[index],NULL);
    }

    if(performers[index].state == 1)
    {
        Tshirt(index);
    }
    return NULL;
}

void* singer_init(void * arg)
{
    int index = *((int *)arg);
    
    sleep(performers[index].arrival_time);
    printf( "\033[1;35m\n"
            "Singer %s has arrived\n",
            performers[index].name);

    fflush(stdout);

    
    struct timespec ts;

    if(clock_gettime(CLOCK_REALTIME,&ts) == -1)return NULL;

    ts.tv_sec += t;
    int s;

    while ((s = sem_timedwait(&total_singer, &ts)) == -1 && errno == EINTR)continue;

    if(s == -1)Error(index);
    else
    {
        int performing_time = t1 + rand()%(t2 + 1 - t1);

        for(int i=0;i<tot;i++)
        {
            if(stages[i].state == 0)
            {
                if(pthread_mutex_trylock(&mutex_stage[i]))continue;
                stages[i].state = 2;
                pthread_mutex_unlock(&mutex_stage[i]);

                char string[20];
                if(stages[i].type == 0)strcpy(string,"acoustic");
                else strcpy(string,"electric");
                
                if(stages[i].type == 0)sem_wait(&acoustic_musician);
                else sem_wait(&electric_musician);

                printf( "\033[1;36m\n"
                        "Singer %s is performing at %s stage %d for %d seconds\n",
                        performers[index].name,string,stages[i].index,performing_time);
                
                fflush(stdout);

                sleep( performing_time);
                
                if(stages[i].type == 0)sem_post(&acoustic_musician);
                else sem_post(&electric_musician);

                if(pthread_mutex_lock(&mutex_stage[i]))
                stages[i].state = 0;
                pthread_mutex_unlock(&mutex_stage[i]);

                printf( "\033[1;32m\n"
                "%s's performance at %s stage %d has ended\n",
                performers[index].name, string,stages[i].index);
        
                fflush(stdout);
                performers[index].state = 1;

                break;
            }
            else if(stages[i].state == 1)
            {
                if(pthread_mutex_trylock(&mutex_stage[i]))continue;
                
                stages[i].state = 3;
                stages[i].sing = index;

                pthread_cond_wait(&cond_stage[i],&mutex_stage[i]);
                pthread_mutex_unlock(&mutex_stage[i]);

                char string[20];
                if(stages[i].type == 0)strcpy(string,"acoustic");
                else strcpy(string,"electric");


                printf( "\033[1;32m\n"
                "%s's performance at %s stage %d has ended\n",
                performers[index].name, string,stages[i].index);
                
                fflush(stdout);
                performers[index].state = 1;
                break;
            }
        }

        sem_post(&total_singer);
    }

    if(performers[index].state == 1)
    {
        Tshirt(index);
    }
    return NULL;

}

int main()
{
    scanf("%d %d %d %d %d %d %d",&k,&a,&e,&c,&t1,&t2,&t);  

    tot = a + e;

    performers  = (struct performer*)malloc(sizeof(struct performer)*k);
    stages = (struct stage*)malloc(sizeof(struct stage)* tot);

    performer_newthread = (pthread_t*)malloc(sizeof(pthread_t)*k);

    mutex_stage=(pthread_mutex_t *)malloc((sizeof(pthread_mutex_t) * tot));
    cond_stage = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * tot);
    flag_lock = (pthread_mutex_t *)malloc((sizeof(pthread_mutex_t) *k));


    //Semaphores
    sem_init(&acoustic_musician, 0, a);
    sem_init(&electric_musician,0,e);
    sem_init(&total_singer,0,tot);
    sem_init(&coordinators,0,c);


    for(int i=0;i<tot;i++)
    {
        if(i < a)stages[i].index = i;
        else stages[i].index = i - a;
        stages[i].sing = -1;
        stages[i].state = 0;
        if(i < a)stages[i].type = 0;
        else stages[i].type = 1;
    }

    for(int i=0;i<k;i++)
    {
        
        scanf("%s",performers[i].name);
        scanf(" %c",&performers[i].instrument);
        type(i,performers[i].instrument);

        scanf("%d",&performers[i].arrival_time);

        performers[i].index = i;
        performers[i].flag = 0;
        performers[i].state = 0;
    
    }

    pthread_t performer_tid[k];

    for(int i=0;i<k;i++)
    {
        if(performers[i].stage != 3)
        {
            pthread_create(&performer_tid[i],NULL,musician_init,(void*)&performers[i].index);
        }
        else
        {
            pthread_create(&performer_tid[i],NULL,singer_init,(void*)&performers[i].index);                   
        }
    }
    for(int i=0;i<k;i++)
    {
        pthread_join(performer_tid[i],NULL);
    }

    printf("\033[1;35m\n\n***SIMULATION OVER***\n\n");
}



