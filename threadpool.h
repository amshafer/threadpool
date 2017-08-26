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
  atomic_int pa_threads;
  atomic_bool pa_kill;
  qnl_t *p_work;
  pthread_mutex_t *p_lock;
  pthread_cond_t *p_cond;
};

// for ease of use
typedef struct pool_tag pool_t;

pool_t *pool_init(int thread_number);

int pool_exec(pool_t *in, void (*exec_f)(void *), void *arg);

void pool_destroy(pool_t *out);

#endif
