#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/string.h>

/**
 * This is the source code of the new system call
 */

typedef struct {
    int count;
    int thread_id;
    int ready;
} pthread_monitor;

static pthread_monitor first_monitor;
static pthread_monitor second_monitor;

/**
 * function to initialize two thread monitors
 */
static long nodeadlock_init(int thread_id, int index) {
    if (index == 0) {
        first_monitor.count = 0;
        first_monitor.thread_id = thread_id;
        first_monitor.ready = 1;
        second_monitor.count = 0;
        second_monitor.thread_id = -1;
        second_monitor.ready = 0;
        return 0;
    } else if (index == 1) {
        second_monitor.count = 0;
        second_monitor.thread_id = thread_id;
        second_monitor.ready = 1;
        return 0;
    } 
    return -1;
}

/**
 * Two helper functions to return the requested thread monitor
 */
pthread_monitor *get_thread_by_id(int thread_id) {
    if (first_monitor.thread_id == thread_id) {
        return &first_monitor;
    } else if (second_monitor.thread_id == thread_id) {
        return &second_monitor;
    }
    return NULL;
}
pthread_monitor *get_other_thread_by_id(int thread_id) {
    if (first_monitor.thread_id == thread_id) {
        return &second_monitor;
    } else if (second_monitor.thread_id == thread_id) {
        return &first_monitor;
    } else {
        return NULL;
    }
}

/**
 * function to indicate whether or not to lock if class_mutex_lock is called
 */
static long nodeadlock_mutex_lock(int this_thread_id) {
    pthread_monitor *this_monitor, *other_monitor;
    this_monitor = NULL;
    other_monitor = NULL;
    // getting two monitors according to the thread ids
    this_monitor = get_thread_by_id(this_thread_id);
    other_monitor = get_other_thread_by_id(this_thread_id);
    if (this_monitor == NULL || other_monitor == NULL) {
        return -1;
    }
    // only allow locking for the current thread,
    // only if the other thread has not locked any and has been initialized
    if (other_monitor->count == 0 && other_monitor->ready == 1) {
        this_monitor->count += 1;
    }
    return this_monitor->count; // when current thread count is zero, it means lock was not successful, so current thread should wait
}

/**
 * function to indicate whether or not to unlock if class_mutex_unlock is called
 */
static long nodeadlock_mutex_unlock(int this_thread_id) {
    pthread_monitor *this_monitor;
    this_monitor = NULL;
    this_monitor = get_thread_by_id(this_thread_id);
    if (this_monitor == NULL) {
        return -1;
    }
    this_monitor->count -= 1; // decrement the locks current thread holds
    return this_monitor->count;
}


asmlinkage long sys_nodeadlock(const char *action, int thread_id, int index, int* retval)
{
    if (retval != NULL) {
        *retval = 0;
        // Direct to the corresponding functions according to the action passed in
        if (action == NULL) {
            *retval = -1;
        } else if (strcmp(action, "init") == 0) {
            *retval = nodeadlock_init(thread_id, index);
        } else if (strcmp(action, "lock") == 0) {
            *retval = nodeadlock_mutex_lock(thread_id);
        } else if (strcmp(action, "unlock") == 0) {
            *retval = nodeadlock_mutex_unlock(thread_id);
        } else {
            *retval = -1;
        }
    }
    return 0;
}

