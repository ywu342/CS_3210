#include "class_thread.h"
#include <unistd.h>

/**
 * This is the pure pthread script for performance analysis
 * For details, please refer to the report
 */

#define MAX 1000000
static int counter = 0;

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
    printf("mutex init ok\n");
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


int class_mutex_lock(class_mutex_ptr cmutex)
{
    // stopping the execution once this function has been called MAX times
    if (counter >= MAX) {
        printf("bestOffer has called class_mutex_lock %d times. The execution will end\n",MAX);
        exit(1);
    }
    else counter++;
    if(pthread_mutex_trylock(&cmutex->mutex))
    {
        fprintf(stdout, "Error: pthread mutex lock failed!\n");
        return -1;
    }
    return 0;
}


int class_mutex_unlock(class_mutex_ptr cmutex)
{
    if(pthread_mutex_unlock(&cmutex->mutex))
    {
        fprintf(stdout, "Error: pthread mutex unlock failed!\n");
        return -1;
    }
    return 0;
}

int class_thread_create(class_thread_t * cthread, void *(*start)(void *), void * arg, int * num_of_mutexes)
{
    pthread_t temp_pthread;
    if(pthread_create(&temp_pthread, NULL, start, arg))
    {
        fprintf(stderr, "Error: Failed to create pthread!\n");
        return -1;
    }
    //Hacking a bit to get everything working correctly
    memcpy(cthread, &temp_pthread, sizeof(pthread_t));
    
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
