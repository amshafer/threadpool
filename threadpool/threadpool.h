/*
 * A personal practice program for pthreads
 * 
 * A thread pool for general purpose use. 
 * This MUST use a version of queue_nl compiled for 
 * use with function pointers. Further notes in queue_nl.h
 *
 * Austin Shafer - 2017
 */

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

/*
 * The threadpool structure
 * 
 *
 */
struct pool_tag {
  atomic_int pa_threads;     // the number of current threads
  atomic_bool pa_kill;       // Flag to kill all threads
  qnl_t *p_work;             // the queue of available work
  pthread_mutex_t *p_lock;   // the mutex used to wait for work
  pthread_cond_t *p_cond;    // signaled when there is work in the queue
};

// for ease of use
typedef struct pool_tag pool_t;

/*
 * Initializes a thread pool.
 * @param thread_number the number of threads
 * @return a pointer to the new threadpool
 */
pool_t *pool_init(int thread_number);


/*
 * Hands a function and its parameter to the threadpool for execution
 * @param in the threadpool to execute the work
 * @param exec_f the pointer to the function to be run
 * @param arg structure containing the arguments for exec_f to run
 * @return 0 on success
 */
int pool_exec(pool_t *in, void (*exec_f)(void *), void *arg);


/*
 * Destroys a thread pool and its sub structures.
 * @param out the threadpool to be destroyed
 */
void pool_destroy(pool_t *out);

#endif
