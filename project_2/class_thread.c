#include "class_thread.h"
#include <unistd.h>

/**
 * Added and Edited functions
 */

#define MAX 1000000
static int counter = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond;

int nodeadlock(char *action, int thread_id, int index) {
    int retval = 0;
    //printf("Calling nodeadlock with thread_id %d to do %s\n", thread_id, action);
    if (action != NULL) {
        syscall(318, action, thread_id, index, &retval); // calling the newly defined system call
        //printf("INSIDE RETVAL = %d\n", retval);
        if (retval == -1) {
            printf("CRITICAL ERROR, TERMINATING.\n");
            exit(1);
        } 
    }
    return retval;
}

int class_mutex_lock(class_mutex_ptr cmutex)
{
    // Manually stopping the execution
    if (counter >= MAX) {
        printf("bestOffer has called class_mutex_lock %d times. The execution will end\n", MAX);
//        exit(1);
    }
    else counter++;
    
    pthread_mutex_lock(&lock); // to make the checking logic atomic
    pthread_t thread_id = pthread_self();
    
    int retval = nodeadlock("lock", (int) thread_id, -1 /* Unusued arg */);
    while (retval == 0) { // another thread has locked, should not lock right away
        pthread_cond_wait(&cond, &lock); // waiting for the signal from the other thread
        retval = nodeadlock("lock", (int) thread_id, -1 /* Unusued arg */);
    }
    pthread_mutex_unlock(&lock); // to make the checking logic atomic
    
    if(pthread_mutex_lock(&cmutex->mutex))
    {
        fprintf(stdout, "Error: pthread mutex lock failed!\n");
        return -1;
    }
    //printf("THREAD ID: %d locked mutex: %p\n",thread_id, &cmutex->mutex);
    return 0;
}


int class_mutex_unlock(class_mutex_ptr cmutex)
{
    pthread_mutex_lock(&lock); // to make the checking logic atomic
    pthread_t thread_id = pthread_self();
    
    int retval = nodeadlock("unlock", (int) thread_id, -1 /* Unusued arg */);
    if (retval == 0)
        pthread_cond_signal(&cond); // when current thread has released both mutexes, signal another thread
    pthread_mutex_unlock(&lock); // to make the checking logic atomic
    
    if(pthread_mutex_unlock(&cmutex->mutex))
    {
        fprintf(stdout, "Error: pthread mutex unlock failed!\n");
        return -1;
    }
    //printf("THREAD ID: %d unlocked mutex: %p\n",thread_id, &cmutex->mutex);
    return 0;
}

/* Two threads and two mutexes are initially created */
int class_thread_create(class_thread_t * cthread, void *(*start)(void *), void * arg, int * num_of_mutexes)
{
    static int index = 0;
    pthread_t temp_pthread;
    if(pthread_create(&temp_pthread, NULL, start, arg))
    {
        fprintf(stderr, "Error: Failed to create pthread!\n");
        return -1;
    }
    
    //Hacking a bit to get everything working correctly
    memcpy(cthread, &temp_pthread, sizeof(pthread_t));
    //printf("THREAD ID CREATE thread_id = %d, index = %d\n", (int) temp_pthread,index);
    nodeadlock("init", (int) temp_pthread, index++); // initialize two thread monitors
    return 0;
}

/**
 * Below is un-edited functions
 */

int allocate_mutex(class_mutex_t *cmutex)
{
    return 0;
}

int allocate_cond(class_condit_ptr ccondit)
{
    if(!ccondit->condition)
    {
        ccondit->condition = malloc(sizeof(pthread_cond_t));
        
        if(!ccondit->condition)
        {
            fprintf(stderr, "Error: malloc failed to allocate space for condition var!\n");
            return -1;
        }
    }
}


int class_mutex_init(class_mutex_ptr cmutex)
{
    if(pthread_mutex_init(&(cmutex->mutex), NULL))
    {
        fprintf(stderr, "Error: pthread mutex initialization failed!\n");
        return -1;
    }
    return 0;
}

int class_mutex_destroy(class_mutex_ptr cmutex)
{
    if(pthread_mutex_destroy(&cmutex->mutex))
    {
        fprintf(stderr, "Error: pthread mutex destruction failed!\n");
        return -1;
    }
    
    return 0;
}

int class_cond_init(class_condit_ptr ccondit)
{
    
    if(pthread_cond_init(ccondit->condition, NULL))
    {
        fprintf(stderr, "Error: pthread condition initialization failed!\n");
        return -1;
    }
    
    return 0;
}

int class_cond_destroy(class_condit_ptr ccondit)
{
    if(pthread_cond_destroy(ccondit->condition))
    {
        fprintf(stderr, "Error: pthread condition destruction failed!\n");
        return -1;
    }
    
    return 0;
}


int class_thread_join(class_thread_t cthread, void ** retval)
{
    pthread_t temp_pthread;
    memcpy(&temp_pthread, &cthread, sizeof(pthread_t));
    
    if(pthread_join(temp_pthread, retval))
    {
        fprintf(stderr, "Error: failed to join the pthread!\n");
        return -1;
    }
    
    return 0;
}

int class_thread_cond_wait(class_condit_ptr ccondit, class_mutex_ptr cmutex)
{
    if(pthread_cond_wait(ccondit->condition, &cmutex->mutex))
    {
        fprintf(stderr, "Error: pthread cond wait failed!\n");
        return -1;
    }
    
    return 0;
}

int class_thread_cond_signal(class_condit_ptr ccondit)
{
    if(pthread_cond_signal(ccondit->condition))
    {
        fprintf(stderr, "Error: pthread cond signal failed!\n");
        return -1;
    }
    
    return 0;
}
