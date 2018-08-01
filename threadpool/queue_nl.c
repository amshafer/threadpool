/*
 * A personal practice program for pthreads
 * 
 * A lockless queue.
 *
 * Austin Shafer - 2017
 */

#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include "queue_nl.h"

/*
 *  NOTE: more elegant comments in header file
 */

/*
 * A simple init function for queue nodes.
 * @param d the data to add.
 * @param n the next node.
 * @return the new node.
 */
qnode_t *
qnode_init (QDATA_T d, qnode_t *n)
{
	qnode_t *ret;
	ret = malloc(sizeof(qnode_t));
	ret->qn_data = d;
	ret->qn_next = n;

	return ret;
}

/*
 * A simple free function for queue nodes.
 * @param out the node to destroy.
 * @return 0 on success.
 */
int
qnode_destroy (qnode_t *out)
{
	if (!out) return QERROR;

	if (out->qn_data) {
		Q_FREE(out->qn_data);
	}
	free(out);
	return 0;
}

qnl_t *
qnl_init ()
{
	qnl_t *ret;
	ret = malloc(sizeof(qnl_t));
	ret->q_lock = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(ret->q_lock, NULL);

	ret->qa_size = 0;
	ret->q_next = NULL;
	ret->q_tail = NULL;
  
	return ret;
}

int
qnl_destroy (qnl_t *q)
{
	if (!q) return QERROR;

	pthread_mutex_destroy(q->q_lock);
	free(q->q_lock);
	// free items in list if we are able
	for (qnode_t *c = q->q_next; c != NULL;) {
		qnode_t *t = c;
		c = c->qn_next;
		// use custom free function since we dont know the type
		qnode_destroy(t);
	}

	free(q);
	return 0;
}

int
qnl_size (qnl_t *q)
{
	pthread_mutex_lock(q->q_lock);
	int ret = q->qa_size;
	pthread_mutex_unlock(q->q_lock);
	return ret;
}

int
qnl_enqueue (qnl_t *q, QDATA_T in)
{
	if (!q || !in) return QERROR;
  
	qnode_t *qn = qnode_init(in, NULL);
  
	pthread_mutex_lock(q->q_lock); 
	// first elemenet case
	if (!q->q_next) {
		q->q_next = qn;
		q->q_tail = q->q_next;
	} else {

		// add to end of queue
		q->q_tail->qn_next = qn;
		q->q_tail = qn;
	}
	q->qa_size++;
	pthread_mutex_unlock(q->q_lock);
	return 0;
}

QDATA_T
qnl_dequeue (qnl_t *q)
{
	if (!q) return NULL;
	qnode_t *r = NULL;
  
	pthread_mutex_lock(q->q_lock);
	// 0th and 1st element cases
	if (q->qa_size == 0) {
		return NULL;
	} else if (q->qa_size == 1) {
		r = q->q_next;
		q->q_next = q->q_tail = NULL;
	} else {
		r = q->q_next;
		q->q_next = q->q_next->qn_next;
	}

	q->qa_size--;
	pthread_mutex_unlock(q->q_lock);
	QDATA_T ret = r->qn_data;
	// change qn_data so we dont accidently delete it
	r->qn_data = NULL;
	qnode_destroy(r);
	return ret;
}

QDATA_T
qnl_peek (qnl_t *q)
{
	if (!q) return NULL;

	return q->q_next->qn_data;
}

/*
 * Initializes a struct holding a function and its parameter struct.
 * Used to hold a queue of work to execute.
 * @param exec_f pointer to function to work with
 * @param a pointer to exec_f's argument structure
 * @return a new qnl_exec_t struct
 */
qnl_exec_t *
qnl_exec_init (void (*exec_f)(void *), void *a)
{
	qnl_exec_t *qe;
	qe = malloc(sizeof(qnl_exec_t));
	qe->qe_func = exec_f;
	qe->qe_arg = a;
	return qe;
}
