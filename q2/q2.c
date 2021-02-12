#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>
#include<sys/shm.h>

//Global Variables:
int n,m,o;
int number_of_students;

struct company
{
    int batches_produced;
    int capacity;
    float probability;
    int batches_left;
    int index;
}*companies;
 
struct zone
{
    int company_index;
    int vaccines_left;
    int slots;
    int index;
}*zones;

struct student
{
    int zone;
    int state;  // waiting:0 ;assigned:1;vaccination phase:2;done:3;
    int shots_count;
    int index;
}*students;

pthread_mutex_t *mutex_zones,*mutex_students,students_remaining;

int min(int a,int b){return(a < b ? a : b);}

void distribute(int index)
{
    while(companies[index].batches_produced > 0  && number_of_students > 0)
    {
        for(int i=0;i<m;i++)
        {
            if(zones[i].company_index == -1)
            {
                if(pthread_mutex_trylock(&mutex_zones[i]))continue;

                //Cyan:
                printf( "\033[1;33m\n"
                        "Company %d is delivering a batch of capacity %d to zone"
                        "%d\033[0m\n",
                        index,companies[index].capacity,i);
                
                fflush(stdout);
                sleep(2);
                
                int capacity = companies[index].capacity;
                zones[i].company_index = index;

                printf( "\033[1;32m\n"
                        "Zone %d has just recieved a batch of %d vaccines from Company"
                        "%d\033[0m\n",
                        i,companies[index].capacity,index);
                
                fflush(stdout);

                pthread_mutex_unlock(&mutex_zones[i]);
                
                companies[index].batches_produced--;
                //sleep(1);  

            }

            if(!companies[index].batches_produced)break;
        }
    }
}

void* Company_init(void *arg)
{

    int index = *((int *)arg);
    while(number_of_students > 0)
    {
        
        int time = 1 + rand()%4;
        int batch = (1 + rand()%5);
        companies[index].batches_left = batch;
        int capacity = (10 + rand()%10);
        companies[index].capacity = capacity;
        companies[index].batches_produced = batch;
        

        //printf("%d %d %d\n",time,batch,capacity);

        printf( "\033[1;35m\n"
                "Company %d is preparing %d batches of vaccines each of capacity %d which have success probability %f\n",
                index,batch,capacity,companies[index].probability);
        
        fflush(stdout);
        sleep(time);
        //Magenta
        printf("\033[1;35m\nCompany %d has finished production.\033[0m\n",index);
        fflush(stdout);
        
        distribute(index);

        while(1)
        {
            if(number_of_students == 0)
            {
                break;
            }
            else if(companies[index].batches_left == 0)
            {
                //Magenta
                printf( "\033[1;35m\n"
                        "All vaccines produced by Company %d are emptied. Resuming production now."
                        "\033[0m\n",index);

                fflush(stdout);
                //sleep(1);
                break;
            }
        }
    }
}

void vaccinate(int index)
{
    sleep(3);
    
    int k = min(1 + rand()%8, min(number_of_students,zones[index].vaccines_left));

    zones[index].slots = k;

    printf("\033[1;34m\nZone %d is ready to vaccinate with %d slots open\n",index,k);

    sleep(1);
    zones[index].vaccines_left -= k;


    while(zones[index].slots > 0 && number_of_students > 0)
    {
        for(int i=0;i<o;i++)
        {
            if(students[i].state == 0)
            {      
                if(pthread_mutex_trylock(&mutex_students[i]))continue;
                
                students[i].state = 1;
                students[i].zone = index;
                
                printf( "\033[1;36m\n"
                        "Student %d has been assigned to Vaccination Zone %d and is waiting to get vaccinated"
                        "\033[0m\n",i,index);

                fflush(stdout);

                pthread_mutex_unlock(&mutex_students[i]);
                
                zones[index].slots--;
            }

            if(!zones[index].slots)break;
        }
    }
}

void* Zone_init(void* arg)
{
    int index = *((int *)arg);

    while(number_of_students > 0)
    {
        if(zones[index].company_index == -1)continue;

        int x = zones[index].company_index;

        zones[index].vaccines_left = companies[x].capacity;

        while(number_of_students > 0 && zones[index].vaccines_left >0)
        {  
            //green color
            sleep(1); 
            printf("\033[1;31m\nVaccination zone %d is entering vaccination phase\033[0m\n",index);
            fflush(stdout);
            vaccinate(index);   
        }

        //Restoring
        if(number_of_students)
        {
            printf("\033[1;34m\nVaccination Zone %d has run out of vaccines\033[0m\n",index);
            fflush(stdout);

            companies[x].batches_left--;
            zones[index].company_index = -1;
        }
    }
        
}

void * Student_init(void* arg)
{
    int index = *((int *)arg);
    printf("\033[1;34m\nStudent %d has arrived for his 1st round of vaccination\033[0m\n",index);
    
    while(students[index].state != 2)
    {
        if(students[index].state == 0)continue;
        
        
        int z = students[index].zone;
        int c = zones[z].company_index;
        float p = companies[c].probability;

        printf( "\033[1;33m\n"
                "Student %d on Vaccination Zone %d has been vaccinated which has success probability %f"
                "\033[0m\n",index,z,p);

        fflush(stdout);
        sleep(3);
        //check prob:

        p *=100;p = 100 -p;
        int x = rand()%100;

        if(x < p)
        {
            printf("\033[1;31m\nStudent %d has tested negative for antibodies\033[0m\n",index);
            fflush(stdout);
            students[index].shots_count++;

            //printf("shots : %d \n",students[index].shots_count);
            if(students[index].shots_count == 3)
            {
                students[index].state = 2;
                printf("\033[1;31m\nStudent %d has to go back Home:(\n",index);
                fflush(stdout);
                pthread_mutex_lock(&students_remaining);
                number_of_students--;
                pthread_mutex_unlock(&students_remaining);
            }
            else 
            {
                sleep(2);
                printf( "\033[1;34m\n"
                        "Student %d has arrived for his %d round of vaccination"
                        "\033[0m\n",index,students[index].shots_count+1);
                
                students[index].state = 0;
                fflush(stdout);
            }
        }
        else
        {
            students[index].state = 2;
            printf("\033[1;32m\nStudent %d has tested positive for antibodies\033[0m\n",index);
            fflush(stdout);

            sleep(2);
            printf("\033[1;32m\nStudent %d is ready to go back to college:)\033[0m\n",index);
            fflush(stdout);

            pthread_mutex_lock(&students_remaining);
            number_of_students--;
            pthread_mutex_unlock(&students_remaining);
        }
    }
}

int main()
{
    scanf("%d %d %d",&n,&m,&o);

    float prob[n];
    for(int i=0;i<n;i++)scanf("%f",&prob[i]);

    number_of_students = o;

    companies  = (struct company*)malloc(sizeof(struct company)*n);
    zones  = (struct zone*)malloc(sizeof(struct zone)*m);
    students  = (struct student*)malloc(sizeof(struct student)*o);

    mutex_zones=(pthread_mutex_t *)malloc((sizeof(pthread_mutex_t) * m));
    mutex_students=(pthread_mutex_t *)malloc((sizeof(pthread_mutex_t) * o));

    pthread_mutex_init(&students_remaining,NULL);


    pthread_t company_tid[n];
    for(int i=0;i<n;i++)
    {
        companies[i].index = i;
        companies[i].batches_produced = 0;
        companies[i].batches_left = 0;
        companies[i].capacity = 0;
        companies[i].probability = prob[i];

        pthread_create(&company_tid[i], NULL, Company_init,(void*)&companies[i].index);
    }
    
    pthread_t zone_tid[m];
    for(int i=0;i<m;i++)
    {
        zones[i].index = i;
        zones[i].company_index = -1;
        zones[i].slots = 0;
        zones[i].vaccines_left = 0;
        pthread_create(&zone_tid[i], NULL, Zone_init,(void*)&zones[i].index);
    }


    pthread_t student_tid[o];
    for(int i=0;i<o;i++)
    {
        students[i].index = i;
        students[i].shots_count = 0;
        students[i].state = 0;
        students[i].zone = -1;
        pthread_create(&student_tid[i], NULL, Student_init,(void*)&students[i].index);
    }
    
    for(int i=0;i<o;i++)
    {
        pthread_join(student_tid[i],NULL);
    }
    for(int i=0;i<n;i++)
    {
        pthread_join(company_tid[i],NULL);
    }
    for(int i=0;i<m;i++)
    {
        //printf(" zone company index %d\n",zones[i].company_index);
        pthread_join(zone_tid[i],NULL);
    }
    sleep(2);

    printf("\033[1;36m\nSIMULATION OVER:)\n\n");
}