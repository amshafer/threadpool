/*
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
 * A convenient free function for queue nodes.
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
	qnode_t *dummy;
	
	ret = malloc(sizeof(qnl_t));
	/* 
	 * create a dummy node so that the queue will always contain something
	 */
	dummy = qnode_init(NULL, NULL);

	ret->q_size ret->q_hcount = ret->q_tcount = 0;
	ret->q_head = ret->q_tail = dummy;
  
	return ret;
}

int
qnl_destroy (qnl_t *q)
{
	if (!q) return QERROR;

	/*
	 * NOTE:
	 * The caller is responsible for emptying the queue and freeing
	 * all nodes, as the data in the nodes could be static.
	 */

	free(q);
	return 0;
}

/*
 * q_size is atomic so we don't use locks
 */
int
qnl_size (qnl_t *q)
{
	int ret = q->q_size;
	return ret;
}

int
qnl_enqueue (qnl_t *q, QDATA_T in)
{
	qnode_t tail;
	atomic_int tcount;
	
	if (!q || !in) return QERROR;
  
	qnode_t *desired = qnode_init(in, NULL);

	while (1) {
		tail = *q->q_tail;
		tcount = q->q_tcount;

		/* CAS: swap q->q_tail->next, NULL is expected, desired is the new tail */
		if (tcount == q->q_tcount
		    && atomic_compare_exchange(q->q_tail->qn_next, NULL, desired)) {
			break;
		}
	}
	/* update tail pointer */
	atomic_compare_exchange(q->q_tail, tail, desired);
	q->q_tcount++;
	return 0;
}

QDATA_T
qnl_dequeue (qnl_t *q)
{
	if (!q) return NULL;
	qnode_t *r = NULL;
  
	pthread_mutex_lock(q->q_out_lock);
	// 0th and 1st element cases
	if (q->q_size == 0) {
		return NULL;
	} else if (q->q_size == 1) {
		r = q->q_head;
		q->q_head = q->q_tail = NULL;
	} else {
		r = q->q_head;
		q->q_head = q->q_head->qn_next;
	}

	q->q_size--;
	pthread_mutex_unlock(q->q_out_lock);
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

	return q->q_head->qn_data;
}

/*
 * THREADPOOL SPECIFIC
 *
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
