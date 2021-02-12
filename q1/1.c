#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>



int *threaded_arr;

int tot = 8;
int each;
int rem;
int n;


pthread_mutex_t lock;

int *shareMem(size_t size)
{
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int *)shmat(shm_id, NULL, 0);
}

void swap(int *xp, int *yp) 
{ 
    int temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
} 



void selection_sort(int arr[], int n)
{
    int i, j, min_idx;  

    for (i = 0; i < n-1; i++)  
    {    
        min_idx = i;  
        for (j = i+1; j < n; j++)  
        if (arr[j] < arr[min_idx])  
            min_idx = j;  
            
        swap(&arr[min_idx], &arr[i]);  
    }  
}

void merge(int a[], int l1, int h1, int h2) 
{ 
    int count=h2-l1+1; 
    int sorted[count]; 
    int i=l1, k=h1+1, m=0; 
    while (i<=h1 && k<=h2) 
    { 
        if (a[i]<a[k]) 
            sorted[m++]=a[i++]; 
        else if (a[k]<a[i]) 
            sorted[m++]=a[k++]; 
        else if (a[i]==a[k]) 
        { 
            sorted[m++]=a[i++]; 
            sorted[m++]=a[k++]; 
        } 
    } 
  
    while (i<=h1) 
        sorted[m++]=a[i++]; 
  
    while (k<=h2) 
        sorted[m++]=a[k++]; 
  
    int arr_count = l1; 
    for (i=0; i<count; i++,l1++) 
        a[l1] = sorted[i]; 

} 

void concurrent_mergesort(int a[], int l , int h)
{
    int len = (h - l + 1);

    if(len < 5)
    {
        selection_sort(a + l, len);
        return;
    }

    pid_t lpid,rpid; 
    lpid = fork(); 
    
    if (lpid==0) 
    { 
        concurrent_mergesort(a,l,l+len/2-1); 
        exit(0); 
    } 
    else
    {
        rpid = fork();     
        if(rpid==0) 
        { 
            concurrent_mergesort(a,l+len/2,h); 
            exit(0); 
        } 
    }

    int status; 
  
    // Wait for child processes to finish 
    waitpid(lpid, &status, 0); 
    waitpid(rpid, &status, 0); 
  
    merge(a, l, l+len/2-1, h);

}

void normal_mergesort(int a[], int l, int h)
{
    int len = (h - l + 1);

    if(len < 5)
    {
        selection_sort(a+l,len);
        return ;
    }

    int mid = (l + h) / 2;
    if(l < h)
    {
        normal_mergesort(a,l,mid);
        normal_mergesort(a,mid+1,h);

        merge(a,l,mid,h);
    }
}

void* threaded_mergesort(void* arg)
{
    int index = *((int *)arg);

    int l = (index) * (each);
    int r = (index + 1) * (each) - 1;

    if(index == (tot - 1)) r += rem;

    if(l > r) return NULL;

    int len = (r - l + 1);

    if(len < 5)
    {
        selection_sort(threaded_arr+l,len);
        return NULL;
    }

    normal_mergesort(threaded_arr,l,r);
    
}

void merge_final(int arr[] , int num , int ele)
{
	for(int i=0;i<num;i = i+2)
	{
		int l = i*(each * ele);
		int r = (i+2)*(each * ele) - 1;
		int mid = l + (each * ele) - 1;
		
        if(r >= n)r = n-1;

		merge(arr , l , mid , r);
	}

	if((num /2) >= 1) merge_final(arr , num/2 , ele*2);
}

void runsorts(int n)
{

    // concurrent  
    int *concurrent_arr = shareMem(sizeof(int) * (n + 1));

    int normal_arr[n];

    threaded_arr = (int *)malloc(sizeof(int) * n);
    

    for(int i=0;i<n;i++)
    {
        scanf("%d",&concurrent_arr[i]);
        normal_arr[i] = concurrent_arr[i];
        threaded_arr[i] = concurrent_arr[i];
    }

    struct timespec ts;
    long double start,end;

    //CONCURRENT : 

    printf("Concurrent Merge Sort:\n");
    printf("Array before sort:\n");
    //for(int i=0;i<n;i++)printf("%d ",concurrent_arr[i]);
    //printf("\n\n");



    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    start = ts.tv_nsec / (1e9) + ts.tv_sec;


    concurrent_mergesort(concurrent_arr,0,n-1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    end = ts.tv_nsec / (1e9) + ts.tv_sec;

    printf("Concurrent time = %Lf\n\n", end - start);

    printf("Array after sort:\n");
    //for (int i = 0; i < n; i++)printf("%d ", concurrent_arr[i]);
    //printf("\n\n");

    //NORMAL :

    printf("Normal Merge sort\n");
    printf("Array before sort:\n");
    //for(int i=0;i<n;i++)printf("%d ",normal_arr[i]);
    //printf("\n\n");

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    start = ts.tv_nsec / (1e9) + ts.tv_sec;

    normal_mergesort(normal_arr,0,n-1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    end = ts.tv_nsec / (1e9) + ts.tv_sec;

    printf("Normal time = %Lf\n\n", end - start);

    printf("Array after sort:\n");
    //for (int i = 0; i < n; i++)printf("%d ", normal_arr[i]);
    //printf("\n\n");


    //THREADED:

    each = n / tot;
    rem = n % tot;

    pthread_t threads[tot];

    printf("Threaded Merge sort\n");
    printf("Array before sort:\n");
    //for(int i=0;i<n;i++)printf("%d ",threaded_arr[i]);
    //printf("\n\n");

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    start = ts.tv_nsec / (1e9) + ts.tv_sec;


    for (int i = 0; i < tot; i++) 
    {
        pthread_create(&threads[i], NULL, threaded_mergesort,(void*)(&i));  
    }

    for (int i = 0; i < tot; i++) 
    {    
        pthread_join(threads[i], NULL); 
    }

    merge_final(threaded_arr,tot,1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    end = ts.tv_nsec / (1e9) + ts.tv_sec;

    printf("Threaded time = %Lf\n\n", end - start);

    printf("Array after sort:\n");
    //for (int i = 0; i < n; i++)printf("%d ", threaded_arr[i]);
    //printf("\n\n");

}

int main()
{
    scanf("%d",&n);
    runsorts(n);
}