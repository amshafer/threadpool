/*
 * A personal practice program for pthreads
 * 
 * A thread pool for general purpose use. 
 *
 * Austin Shafer - 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>
#include "queue_nl.h"
#include "threadpool.h"

/* uncomment to print all debug info */
//#define WITH_LOGGING

#ifdef WITH_LOGGING
# define POOL_LOG(args) printf((args))
#else
# define POOL_LOG(args) 
#endif /* WITH_LOGGING */

/*
 * Function that all pool theads run for their lifetime.
 * When pa_kill is set the threads will kill themselves. This is
 * so that the threadpool does not have to hold references to all child
 * threads. 
 *
 * @param arg a pointer to the thread pool to operate on
 */ 
void
thread_life (void *arg)
{
	if (!arg) return; 

	pool_t *pl = (pool_t *)arg;
	pthread_mutex_lock(pl->p_lock);

	// continuously get work off of queue and run it
	while (1) {
		POOL_LOG("Waiting for condition var...\n");
		pthread_cond_wait(pl->p_cond, pl->p_lock);
		// we now hold the lock
    
		// see if we should kill ourselves
		if (!pl || pl->pa_kill) {
			pl->pa_threads--;
			POOL_LOG("Signaling condition var...\n");
			pthread_cond_signal(pl->p_cond);
			pthread_mutex_unlock(pl->p_lock);
			pthread_exit(NULL);
		}
    
		// condition signals that queue holds work. Dequeue and run.
		qnl_exec_t *task = qnl_dequeue(pl->p_work);

		if (qnl_size(pl->p_work) > 0) {
			// if more work to do then signal condition
			POOL_LOG("Signaling condition var...\n");
			pthread_cond_signal(pl->p_cond);
		} else {
			printf("skipping signal\n");
		}
		// execute function call
		if (task && task->qe_func)
			task->qe_func(task->qe_arg);
	}
}

/*
 * Spins up n number of threads.
 * NOTE: DOES NOT keep a reference to threads launched.
 * @param p the threadpool to signal
 * @param n the number of threads to start
 * @return 0 on success
 */
int
pool_spin_threads (pool_t *p, int n)
{
	if (n < 1 || !p) return TPERROR;

	for (int i = 0; i < n; i++) {
		pthread_t cur;
		pthread_create(&cur, NULL, (void *)&thread_life, p);
		p->pa_threads++;
	}
	return 0;
}

/*
 * Signals all threads to end and waits for them.
 * @param p the threadpool to signal
 * @return 0 on success
 */
int
pool_join_threads (pool_t *p)
{
	if (!p) return TPERROR;

	// set kill and signal condition
	p->pa_kill = 1;
	pthread_mutex_lock(p->p_lock);
	pthread_cond_broadcast(p->p_cond);
	pthread_mutex_unlock(p->p_lock);
  
	while (p->pa_threads != 0) {
		sleep(1);
	}
	return 0;
}

pool_t *
pool_init (int thread_number)
{
	pool_t *pl;
	pl = malloc(sizeof(pool_t));

	pl->pa_threads = 0;
	pl->pa_kill = 0;
	pl->p_work = qnl_init();

	pl->p_lock = malloc(sizeof(pthread_mutex_t));
	pl->p_cond = malloc(sizeof(pthread_cond_t));
	pthread_mutex_init(pl->p_lock, NULL);
	pthread_cond_init(pl->p_cond, NULL);

	// spin up threads
	int err = pool_spin_threads(pl, thread_number);
	if (err) return NULL;
  
	return pl;
}

void 
pool_destroy (pool_t *out)
{
	// join threads
	pool_join_threads(out);
  
	pthread_cond_destroy(out->p_cond);
	pthread_mutex_destroy(out->p_lock);
	free(out->p_cond);
	free(out->p_lock);

	qnl_destroy(out->p_work);

	free(out);
}

int
pool_exec (pool_t *in, void (*exec_f)(void *), void *arg)
{
	if (!in || !exec_f) return TPERROR;
  
	// signal condition after work is added
	qnl_exec_t *qe = qnl_exec_init(exec_f, arg);

	// we need to follow proper lock order. pool lock, then enqueue
	pthread_mutex_lock(in->p_lock);
	qnl_enqueue(in->p_work, qe);

	// If this is the first job signal work is available
	if (qnl_size(in->p_work) == 1) {
		POOL_LOG("Signaling condition var for work...\n");
		pthread_cond_signal(in->p_cond);
	}
	pthread_mutex_unlock(in->p_lock);
  
	return 0;
}
