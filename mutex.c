// gcc mutex.c -o mutex.o -lpthread
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
typedef struct list_node_s{
    int sa;
    int sz;
    int data;
    struct list_node_s* next;
}list_node_s;
typedef struct record_node_s{
    int sa;
    int sz;
    int data;
}record_node_s;
void *thread_function(void *);
void *threaded_task(void *);
int mAlloc(record_node_s* tmp);
int mDalloc(record_node_s* tmp);
int n;                 //initial memory size 
int m;                 //number of operations in each execution
float m_malloc_frac;   //member fraction
pthread_mutex_t mutex, mutex1;
int count, count_m, count_i, count_d = 0;
int m_malloc,m_malloc_thread;
int thread_count;   //number of threads
struct list_node_s* head_p = NULL;
struct record_node_s * record = NULL;

int main(int argc, char* argv[]){
    int thread;
    pthread_t* thread_handles;
    clock_t start, end;
    double cpu_time_used;
    srand (time(NULL));
    int i, ins_value;
    if (argc != 6){
        //error in input values
        fprintf(stderr, "error in input format. correct format: <thread_count> <n> <m> <m_malloc_frac> <m_mdalloc_frac> \n");
        exit(0);
    }
    thread_count = strtol(argv[1], NULL, 10);
    if (thread_count <= 0){
        fprintf(stderr, "error. thread count should be greater than 0\n");
        exit(0);
    }
    n = (int) strtol(argv[2], NULL, 10);
    m = (int) strtol(argv[3], NULL, 10);
    m_malloc_frac = (float) atof(argv[4]);
    m_malloc = m * m_malloc_frac;
    m_malloc_thread = m_malloc/ thread_count;
    printf("Total num: %i,thread: %i,malloc num: %i\n",m,thread_count,m_malloc);
    /*
    if(n < 0 || m < 0 || m_member_frac < 0 || m_insert_frac < 0 || m_delete_frac < 0 || (int)(m_member_frac+m_insert_frac+m_delete_frac)!= 1){
        //error
        fprintf(stderr, "error in input values.\n");
        fprintf(stderr, "n: no. of initial values in the linked list.\n");
        fprintf(stderr, "m: no. of values for carrying out operations.\n");
        fprintf(stderr, "m_member_frac: fraction of operations for member function\n");
        fprintf(stderr, "m_insert_frac: fraction of operations for insert function\n");
        fprintf(stderr, "m_delete_frac: fraction of operations for delete function\n");
        exit(0);
    }
    */
    record = (record_node_s*)malloc(m*sizeof(record_node_s));
    struct list_node_s* new_node_s = (struct list_node_s*)malloc(sizeof(struct list_node_s));
    new_node_s->sa = 0;
    new_node_s->sz = n;
    new_node_s->next = NULL;
    head_p = new_node_s;
    for (i = 0; i < m_malloc; i++){
        record[i].sa = -1;
        record[i].sz = 1 << rand() % 3; //value should be between 2^16 - 1
    }
    thread_handles =  malloc(thread_count*sizeof(pthread_t));
    start = clock();
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex1, NULL);
    int result;
    for (thread = 0; thread < thread_count;thread++){
        //printf("Create num %i thread\n",(int)thread);
        result = pthread_create(&thread_handles[thread], NULL, thread_function, &thread);
        if (result) {
            printf("ERROR: return code from pthread_create() is %d\n", result);
            exit(-1);
        }
    }
    for (thread = 0; thread < thread_count; thread++){
        //printf("Wait %i. %p\n",thread,thread_handles[thread]);
        void *retval;
        pthread_join(thread_handles[thread], &retval);
    }
    end = clock();
    
    struct list_node_s* node = head_p;
    printf("List after dealloc!\n");
    while(node!=NULL){
        printf("SA: %i, SZ: %i\n",node->sa,node->sz);
        node = node->next;
    }    
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("execution time is : %f\n", cpu_time_used);
    free(thread_handles);
    return 0;
}
void *task(int id) {
  printf("Task %d started\n", id);
  int i;
  double result = 0.0;
  for (i = 0; i < 1000000; i++) {
    result = result + sin(i) * tan(i);
  }
  printf("Task %d completed with result %e\n", id, result);
}

void *threaded_task(void *t) {
	int rank;
    pthread_mutex_lock( &mutex1 ); 
    rank = count++;
    pthread_mutex_unlock( &mutex1 ); 
    long id = *(long*) t;
    printf("Thread %ld started\n", id);

	int i = rank*m_malloc_thread;
    for (;i<(rank+1)*m_malloc_thread;i++){
        pthread_mutex_lock( &mutex );    
        mAlloc(&record[i]);
        pthread_mutex_unlock( &mutex );
	}
    task(id);
	i = rank*m_malloc_thread;
    for (;i<(rank+1)*m_malloc_thread;i++){
        pthread_mutex_lock( &mutex );    
        mDalloc(&record[i]);
        pthread_mutex_unlock( &mutex ); 
	}
    printf("Thread %ld done\n", id);
    pthread_exit(0);
}

void *thread_function(void * tmp){
    int i;    
    pthread_mutex_lock( &mutex1 ); 
    i = count++;
    pthread_mutex_unlock( &mutex1 ); 
    //printf("Thread %i in function (%i,%i)\n",rank,rank*m_malloc_thread,(rank+1)*m_malloc_thread);
    while(i < m){
        //only one thread can get access to the linked-list at a time
        if(i < m_malloc){
            //printf("Malloc #%i/%i\n",i,m_malloc);
            pthread_mutex_lock( &mutex );    
            mAlloc(&record[i]);
            pthread_mutex_unlock( &mutex ); 
        }
        else{
            //printf("Dealloc #%i/%i\n",i-m_malloc,m-m_malloc);
            pthread_mutex_lock( &mutex );
            mDalloc(&record[i-m_malloc]);
            pthread_mutex_unlock( &mutex );
        }
        
        pthread_mutex_lock( &mutex1 ); 
        i = count++;
        pthread_mutex_unlock( &mutex1 ); 
       
    }
    return NULL;
}
//mem alloc: in critical section
int mAlloc(record_node_s* tmp){
    //printf("Mem %i Alloc with size\n",tmp->sz);
    struct list_node_s* curr_p = head_p;
    struct list_node_s* pred_p = head_p;
    while(curr_p != NULL && curr_p->sz < tmp->sz){
        pred_p = curr_p;
        curr_p = curr_p->next;
    }
    if(curr_p==NULL) {
        //printf("Not available!\n");
        return 2;
    }
    if(curr_p->sz > tmp->sz){
        //printf("Mem Malloc case 1 with sz: %i\n",tmp->sz);
        curr_p->sz -= tmp->sz;
        tmp->sa = curr_p->sa + curr_p->sz;
        return 0;
    }else if (curr_p->sz == tmp->sz){
        //printf("Mem Malloc case 2 with sz: %i\n",tmp->sz);
        pred_p->next = NULL;
        tmp->sa = curr_p->sa;
        if (curr_p->next == NULL) head_p =NULL;
        free(curr_p);
    }
    else{
        return 1;
    }

}
//mem dealloc function: in critical section
int mDalloc(record_node_s* tmp){
    //printf("Mem Dealloc with sa: %i and sz: %i\n",tmp->sa,tmp->sz);
    struct list_node_s** head_pp = &head_p;
    struct list_node_s* curr_p = *head_pp;
    struct list_node_s* pred_p = NULL;
    struct list_node_s* temp_p;
    if (tmp->sa==-1) return 1;
    if (curr_p==NULL){
        //printf("Delete case 0\n");
        struct list_node_s* new_node_s = (struct list_node_s*)malloc(sizeof(struct list_node_s));
        new_node_s->sa = tmp->sa;
        new_node_s->sz = tmp->sz;
        tmp->sa = -1;
        new_node_s->next = curr_p;
        head_p = new_node_s;
        return 0;
    }
    if (curr_p->sa>(tmp->sa+tmp->sz)){
        //printf("Delete case 1 current pos:%i\n",curr_p->sa);
        struct list_node_s* new_node_s = (struct list_node_s*)malloc(sizeof(struct list_node_s));
        new_node_s->sa = tmp->sa;
        new_node_s->sz = tmp->sz;
        tmp->sa = -1;
        new_node_s->next = curr_p;
        head_p = new_node_s;
        return 0;
    }else if (curr_p->sa==(tmp->sz+tmp->sa)){
        //printf("Delete case 2\n");
        curr_p->sa = tmp->sa;
        curr_p->sz += tmp->sz;
        tmp->sa = -1;
        return 0;
    }
    while(curr_p != NULL){
        //printf("current node with sa: %i and sz: %i\n",curr_p->sa,curr_p->sz);
        if (curr_p->next != NULL && curr_p->sa < tmp->sa){
            if (curr_p->next->sa > tmp->sz+tmp->sa){
                //printf("Delete case 3\n");
                struct list_node_s* new_node_s = (struct list_node_s*)malloc(sizeof(struct list_node_s));
                new_node_s->sa = tmp->sa;
                new_node_s->sz = tmp->sz;
                tmp->sa = -1;
                new_node_s->next = curr_p->next;
                pred_p->next = new_node_s;
                return 0;
            }else if (curr_p->next->sa == tmp->sz+tmp->sa){
                //printf("Delete case 4\n");
                curr_p->next->sa = tmp->sa;
                curr_p->next->sz += tmp->sz;
                tmp->sa = -1;
                return 0;
            }
        }else if (curr_p->next== NULL){
            //printf("Delete case 5\n");
            struct list_node_s* new_node_s = (struct list_node_s*)malloc(sizeof(struct list_node_s));
            new_node_s->sa = tmp->sa;
            new_node_s->sz = tmp->sz;
            tmp->sa = -1;
            //printf("New node added to the last with sa:%i and sz:%i\n",new_node_s->sa,new_node_s->sz);
            new_node_s->next = NULL;
            curr_p->next = new_node_s;
            struct list_node_s* node = head_p;
            /*
            while(node!=NULL){
                printf("SA: %i, SZ: %i\n",node->sa,node->sz);
                node = node->next;
            }
            */
            return 0;
        }
        pred_p = curr_p;
        curr_p = curr_p->next;
    }
    return 1;
}
